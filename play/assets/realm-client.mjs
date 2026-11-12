import { parseMonsterAnimations } from './monster-renderer.mjs';
import { readD2gsDeny, d2gsDenyError } from './d2gs-deny.mjs';
import { parseRealmMissiles } from './realm-missiles.mjs';
import { parseRealmUnitInfo } from './realm-combat.mjs';
import { parseRealmMissileSoundEvents } from './realm-missile-sounds.mjs';
import { OBJECT_UNIT_BYTES, parseRealmObjectUnits, parseRealmObjectEvents } from './realm-objects.mjs';
import { parseRealmUnitFx } from './realm-unit-fx.mjs';
import { parseSessionRooms, parseSessionSnapshot, parseSessionWarps } from '../gameplay-model.mjs';
// The realm CLIENT half: it drives our protocol (BNCS/MCP/D2GS) over a byte-pipe transport.
//
// Everything here is ORCHESTRATION and PARSING. The message BODIES are composed by the engine's own
// realm code (the same `src/core/realm` sources the server side and realmd use), reached through
// `protocol` -- so this file adds no second copy of a wire layout to drift. What it owns is the
// ORDER a client sends and expects them in, and the client-side read offsets (which the server never
// sees and so cannot check): serverToken at body offset 4, the MPQ name at 20, the JOIN ticket's
// network-order IP, the char-select statstring's class byte 13 / level byte 25.
//
// A `transport` is the byte pipe. A browser has only one usable one -- a WebSocket -- and a browser
// cannot host the realm itself (the locked decision: point at one, never host it), so the page dials
// a project-configured endpoint through `webSocketTransport` and the `tools/realmd/ws-bridge.mjs`
// bridge copies its bytes to the realm's TCP sockets verbatim. `loopbackTransport` is the same pipe
// over an in-process server session, which is how the test runner proves the client against the engine's
// own server with no socket and no mock.
//
// Prior art: the request/reply ORDER and read offsets are cross-checked against
// tristramd, d2-clientless and the diablo2-protocol schemas; this file re-uses the same references
// and the engine exports s113-s125 already prove byte-for-byte.

// Reply waits, from the client libraries' own loops. Battle.net: each logon step waits for its reply
// flag for 45 s (error 0x11 on expiry). Realm: the startup reply wait is a 10 ms poll bounded at 60 s.
// Every other realm reply is awaited by the screen's event pump with no deadline of its own (the shell
// cancels it), so CONNECT_TIMEOUT_MS is only a transport guard for those and the game-server join, not
// a retail value.
export const BNCS_REPLY_MS = 45000;
export const MCP_STARTUP_MS = 60000;
const CONNECT_TIMEOUT_MS = 8000;

const cat = (a, b) => (a.length ? Uint8Array.from([...a, ...b]) : (b instanceof Uint8Array ? b : Uint8Array.from(b)));
const hex = (u8) => [...u8].map((b) => b.toString(16).padStart(2, '0')).join('');
const fourcc = (value) => { let s = ''; for (let i = 0; i < 4; i++) { const c = (value >>> (i * 8)) & 0xff; if (c) s += String.fromCharCode(c); } return s.trim(); };

/**
 * A reader over a transport that ACCUMULATES. The obvious per-call reader is wrong on a socket: one
 * read can deliver a whole reply, so a caller that reads the header then starts over waiting for the
 * body blocks on bytes already in hand. `want` returns the first n bytes WITHOUT consuming, so a
 * caller can read a frame header and then the whole frame it describes, then `skip` it.
 */
function makeReader(transport) {
  let buf = new Uint8Array(0), closed = false, cancelled = false;
  const waiters = [];
  const pump = () => {
    for (let i = waiters.length - 1; i >= 0; i--) {
      const w = waiters[i];
      if (buf.length >= w.n) { clearTimeout(w.t); waiters.splice(i, 1); w.resolve(buf.subarray(0, w.n)); }
      else if (closed) { clearTimeout(w.t); waiters.splice(i, 1); w.reject(new Error(`transport closed after ${buf.length}/${w.n} bytes`)); }
    }
  };
  transport.onData((d) => { buf = cat(buf, d); pump(); });
  transport.onClose(() => { closed = true; pump(); });
  return {
    echo: (bytes) => transport.send(bytes),
    want: (n, ms = CONNECT_TIMEOUT_MS) => new Promise((resolve, reject) => {
      const w = { n, resolve, reject, t: null };
      if (ms !== null) w.t = setTimeout(() => { const i = waiters.indexOf(w); if (i >= 0) waiters.splice(i, 1); reject(Object.assign(new Error(`timed out after ${buf.length}/${n} bytes`), { timedOut: true })); }, ms);
      waiters.push(w);
      pump();
    }),
    skip: (n) => { buf = buf.subarray(n); },
    buffered: () => buf,
    get closed() { return closed; },
    get cancelled() { return cancelled; },
    cancel: () => {
      if (cancelled) return;
      cancelled = true;
      const error = new Error('D2GS reader cancelled');
      while (waiters.length) {
        const w = waiters.pop(); clearTimeout(w.t); w.reject(error);
      }
    },
  };
}

// BNClient+0xF430: once the 0x50 reply is in, a link already marked lost raises failure event 0xB (over any earlier event) and the
// connect returns as done, so the version step is never sent.
function linkLostAfterAuthInfo(reader) {
  if (reader.closed) throw Object.assign(new Error('link lost after the AUTH_INFO reply'), { failureEvent: 0xb });
}

/**
 * One BNCS frame: `[0xFF][id][len u16 LE]` + body. A server may send SID_PING (0x25, a 4-byte cookie)
 * at any time, and Battle.net servers send it right after the selector, before the AUTH_INFO reply;
 * it is never an answer to a call, so it is echoed back as the protocol asks and skipped.
 */
// `wantId`: the client library files every frame by its id and a wait watches only its own reply's flag, so a frame with another
// id (a keep-alive, a server notice, an optional-work or news packet) never ends or fails the wait; it is consumed and the wait goes on.
async function readBncsFrame(reader, timeoutMs, packetWait = false, wantId = null) {
  // Retail waits 45 s for the version, key and realm replies and raises failure event 0x11 when none comes; the logon reply has no such wait.
  // One deadline for the whole wait (the reply flag is polled from the start, skipped pings and a split frame do not restart it) and a
  // strict one: a reply at exactly the limit still counts, the wait ends only past it.
  const started = Date.now();
  const left = () => (packetWait && timeoutMs ? Math.max(0, timeoutMs - (Date.now() - started)) + 1 : timeoutMs);
  const want = async (n) => { try { return await reader.want(n, left()); } catch (error) { if (packetWait && error?.timedOut) error.failureEvent = 0x11; throw error; } };
  for (;;) {
    // The client library's splitter starts every frame at the next 0xFF byte of what it has received: anything ahead of it is passed over, never an error.
    const lead = await want(1);
    if (lead[0] !== 0xff) { const at = reader.buffered().indexOf(0xff); reader.skip(at < 0 ? reader.buffered().length : at); continue; }
    const head = await want(4);
    const total = head[2] | (head[3] << 8);
    // Retail steps on by the declared length whatever it is (and never moves again on a zero). A length under the header's own four bytes
    // cannot hold a frame, so the marker is dropped and the scan for the next one goes on: a choice of ours where retail would stall or misread.
    if (total < 4) { reader.skip(1); continue; }
    const frame = await want(total);
    const copy = Uint8Array.from(frame);
    reader.skip(total);
    if (copy[1] === 0x25) { if (reader.echo) reader.echo(copy); continue; }
    if (wantId !== null && copy[1] !== wantId) continue;
    return { id: copy[1], body: copy.subarray(4) };
  }
}

/** One MCP frame: `[total:u16 LE][op:u8][body]`; total includes its own 3-byte header. */
// The realm client reads each frame into a 0x400-byte buffer (opcode and body, the 2-byte length excluded); a larger frame is
// consumed and dropped without a dispatch, and the session carries on.
export const MCP_RECV_BUFFER = 0x400;
export async function readMcpFrame(reader, timeoutMs) {
  for (;;) {
    const head = await reader.want(3, timeoutMs);
    const total = head[0] | (head[1] << 8);
    if (total < 3) throw new Error(`MCP frame length ${total} is shorter than its header`);
    const frame = await reader.want(total, timeoutMs);
    reader.skip(total);
    if (total - 2 > MCP_RECV_BUFFER) continue;
    // The reply to an idle-time MOTD request (the lobby's keep-alive) is never an answer to a call.
    if (frame[2] === MCP_MOTD) continue;
    // Likewise the character-list reply to a keep-alive sent before the character is logged on.
    if (frame[2] === 0x19 && reader.keepAliveLists > 0) { reader.keepAliveLists--; continue; }
    return { op: frame[2], body: frame.subarray(3) };
  }
}

/** The realm list reply (0x40) body: `[unknown u32][count u32]` then per realm `[unknown u32][title\0][description\0]`. */
export function parseRealmList(body) {
  const count = body.length >= 8 ? u32(body, 4) : 0, list = [], dec = new TextDecoder();
  let o = 8;
  for (let i = 0; i < count; i++) {
    o += 4;
    const titleEnd = body.indexOf(0, o), descriptionEnd = titleEnd < 0 ? -1 : body.indexOf(0, titleEnd + 1);
    if (descriptionEnd < 0) break;
    list.push({ title: dec.decode(body.subarray(o, titleEnd)), description: dec.decode(body.subarray(titleEnd + 1, descriptionEnd)) });
    o = descriptionEnd + 1;
  }
  return list;
}
/**
 * The realm named in the realm logon (0x3e). Retail (D2Launch 0x4C430 / 0x4D5D0 set the name up, 0x5A320 sends it through the BNClient
 * realm-logon routine) sorts the listed realms by title (a byte compare), then takes the saved "Preferred Realm" when a listed realm
 * matches it ignoring case, else the current gateway's realm name when a listed realm matches that, else the first of the sorted list;
 * the name sent is the saved string as stored, not the listed spelling. A list of none is a failure screen there and no logon; ours
 * falls back to the configured name for a server that lists none (a deliberate difference).
 */
export function realmLogonTitle(realmList, configured, preferred = '') {
  if (!realmList.length) return configured;
  const listed = (name) => !!name && realmList.some((r) => r.title.toLowerCase() === name.toLowerCase());
  if (listed(preferred)) return preferred;
  if (listed(configured)) return configured;
  return realmList.map((r) => r.title).sort((x, y) => (x < y ? -1 : x > y ? 1 : 0))[0];
}

/**
 * What the character-create screen says when the realm refuses the new character: result 0x14 is the name already in use, any other
 * non-zero result the name rejected. `string(id)` reads the game's string table.
 */
export function charCreateRefusalText(result, name, string) {
  return result === 0x14 ? string(11059).replace('%s', name) : `${string(5214)}${name}${string(5215)}`;
}

/**
 * One reader for a realm session. The lobby has several things waiting on the realm socket at once (a create or join, a ladder, the
 * game list that arrives a frame at a time and unasked-for queue notices), so the session's frames are read in one place: `onFrame`
 * (the game list) sees each first and may take it, the rest go to whoever waits next, in order.
 */
function sessionInbox(session) {
  if (session.inbox) return session.inbox;
  session.reader ??= makeReader(session.transport);
  const queue = [], waiters = [];
  let failure = null, reading = false;
  const drop = (w) => { const i = waiters.indexOf(w); if (i >= 0) waiters.splice(i, 1); clearTimeout(w.t); w.signal?.removeEventListener('abort', w.abort); };
  session.inbox = {
    next: (ms, signal = null) => new Promise((resolve, reject) => {
      if (signal?.aborted) { reject(cancelledError()); return; }
      if (queue.length) { resolve(queue.shift()); return; }
      if (failure) { reject(failure); return; }
      const w = { resolve, reject, t: null, signal, abort: () => { drop(w); reject(cancelledError()); } };
      if (ms !== null && ms !== undefined) w.t = setTimeout(() => { drop(w); reject(Object.assign(new Error('timed out waiting for a realm reply'), { timedOut: true })); }, ms);
      signal?.addEventListener('abort', w.abort, { once: true });
      waiters.push(w); pump();
    }),
    listen: () => pump(),
  };
  // Frames are read only while someone wants them: a wait, or the listener.
  async function pump() {
    if (reading || failure) return;
    reading = true;
    try {
      while (waiters.length || session.onFrame) {
        const read = await readMcpFrame(session.reader, null), frame = { op: read.op, body: Uint8Array.from(read.body) };
        let taken = false;
        try { taken = session.onFrame?.(frame) === true; } catch { /* a listener's fault is not the socket's */ }
        if (taken) continue;
        const w = waiters[0];
        if (w) { drop(w); w.resolve(frame); }
        // With nobody waiting a frame is kept for the next wait (retail files each reply under its opcode until it is looked at); the oldest go first.
        else { queue.push(frame); if (queue.length > 64) queue.shift(); }
      }
    } catch (error) { failure = error; for (const w of waiters.splice(0)) { clearTimeout(w.t); w.signal?.removeEventListener('abort', w.abort); w.reject(error); } }
    reading = false;
  }
  return session.inbox;
}
const cancelledError = () => Object.assign(new Error('cancelled'), { cancelled: true });
/**
 * Wait for the reply with opcode `op`. The realm client files every frame under its own opcode and each wait watches only its own, so a
 * frame with another opcode never ends or fails a wait: it is passed over, under the one deadline the wait started with.
 */
async function mcpReply(read, op, ms, received = null) {
  const started = Date.now();
  for (;;) {
    const frame = await read(ms === null ? null : Math.max(1, ms - (Date.now() - started)));
    received?.push(frame.op);
    if (frame.op === op) return frame;
  }
}
// Taken before the request is sent, so the session is being read by the time its answer can arrive.
const sessionRead = (session) => { const inbox = sessionInbox(session); return (ms, signal) => inbox.next(ms, signal); };

/** Route the session's game-list frames (and anything else `handler` returns true for) away from the reply waits. */
export function mcpListen(session, handler) { session.onFrame = handler; if (handler) sessionInbox(session).listen(); }
/** The session's request counter: create, join, list and info requests share it, and it never hands out 0. */
export function mcpNextRequestId(session) { const id = session.nextRequestId; session.nextRequestId = (id + 1) & 0xffff || 1; return id; }
/** Put one request on the session's realm socket. */
export function mcpSend(session, op, body) { session.transport.send(session.protocol.mcpFrame(op, body)); }

/**
 * MCP CHARLOGON (0x07) for the character the player picked. Retail sends it when the character is chosen, before the lobby opens: the
 * realm answers list, info, ladder and create requests only for a logged-on character. Asking again for the one already on is free.
 */
export function mcpLogonCharacter(session, name, { timeoutMs = CONNECT_TIMEOUT_MS } = {}) {
  if (!session || session.joined) return Promise.reject(new Error('MCP character session is not reusable'));
  const selected = session.list.characters.find((c) => c.name.toLowerCase() === String(name).toLowerCase());
  if (!selected) return Promise.reject(new Error(`character "${name}" is not in the MCP character list`));
  if (session.characterLoggedOn && session.characterLoggedOn.toLowerCase() === selected.name.toLowerCase()) return Promise.resolve(selected);
  if (session.logon?.name === selected.name) return session.logon.promise;
  const promise = (async () => {
    try {
      const read = sessionRead(session);
      mcpSend(session, 0x07, session.protocol.mcpCharlogon(selected.name));
      const reply = await mcpReply(read, 0x07, timeoutMs, session.received);
      if (reply.body.length < 4 || u32(reply.body, 0) !== 0) throw charlogonRefusal(reply.body);
      session.characterLoggedOn = selected.name;
      return selected;
    } finally { session.logon = null; }
  })();
  session.logon = { name: selected.name, promise };
  return promise;
}

// Retail has no idle keep-alive on the realm socket: the message of the day is asked once at lobby entry,
// and the only periodic realm traffic is the game-list refresh every 15 s on the game list. This timer
// exists for the WebSocket proxies in front of the realm (they drop a silent socket after about 100 s),
// so its period is a transport choice, not a retail value.
const MCP_MOTD = 0x12, MCP_KEEPALIVE_MS = 30000;
// D2Multi's reply pump (1.13c D2Multi+0xDAE0, GetTickCount against DAT_6fa09f2c) gives a create or join 30 s from the Please Wait popup, then clears the wait and returns to the lobby.
const GAME_REPLY_MS = 30000;
/**
 * An idle MCP socket is dropped by the proxies in front of the realm (about 100 s with no traffic), and
 * the lobby sits idle that long. Ask for the message of the day on a timer; the reply is skipped by
 * readMcpFrame. Stops when the transport closes or the session has joined its game.
 */
function startMcpKeepAlive(session, ms = MCP_KEEPALIVE_MS) {
  const { transport, protocol } = session;
  const timer = setInterval(() => {
    if (session.joined || transport.isOpen === false) { clearInterval(timer); return; }
    // The realm answers MOTD only once the character is logged on; before that the lobby asks for the
    // character list again (valid at either stage) and the reply is skipped by readMcpFrame.
    try {
      if (session.characterLoggedOn) transport.send(protocol.mcpFrame(MCP_MOTD, []));
      else { session.reader.keepAliveLists = (session.reader.keepAliveLists || 0) + 1; transport.send(protocol.mcpFrame(0x19, protocol.mcpCharlist(8))); }
    } catch { clearInterval(timer); }
  }, ms);
  timer.unref?.();
  transport.onClose?.(() => clearInterval(timer));
  return timer;
}

function decodeMcpCharacters(body) {
  if (body.length < 8) throw new Error(`MCP CHARLIST2 body is short (${body.length} bytes)`);
  const total = u32(body, 2), returned = body[6] | (body[7] << 8);
  if (returned > total) throw new Error(`MCP CHARLIST2 returned ${returned} entries but declares ${total} total`);
  const characters = [];
  let at = 8;
  const readCString = (field) => {
    const end = body.indexOf(0, at);
    if (end < 0) throw new Error(`MCP CHARLIST2 ${field} is not terminated`);
    const value = body.subarray(at, end);
    at = end + 1;
    return value;
  };
  for (let i = 0; i < returned; i++) {
    if (at + 4 > body.length) throw new Error(`MCP CHARLIST2 entry ${i} has no expiry field`);
    const expiry = u32(body, at); at += 4;
    const rawName = readCString(`entry ${i} name`);
    const statstring = readCString(`entry ${i} statstring`);
    if (statstring.length < 26) throw new Error(`MCP CHARLIST2 entry ${i} statstring is short (${statstring.length} bytes)`);
    if (statstring[13] < 1 || statstring[13] > 7) throw new Error(`MCP CHARLIST2 entry ${i} has invalid class byte ${statstring[13]}`);
    characters.push({ name: new TextDecoder().decode(rawName), expiry,
      charClass: statstring[13] - 1, level: statstring[25], flags: statstring[26] ?? 0, progression: statstring[27] ?? 0 });
  }
  return { total, returned, characters };
}

/**
 * The MCP realm leg: selector -> STARTUP -> CHARLIST2 -> CHARLOGON -> CREATEGAME -> JOINGAME.
 * The startup bytes are the exact 64-byte block BNCS hands back. Wire bodies are composed through
 * the engine exports, as with BNCS and D2GS; this function owns only the sequence and read offsets.
 */
export async function mcpCreateAndJoin(transport, protocol, { realmReply, character, gameName, gamePassword = '',
  difficulty = 0, maxPlayers = 8, levelDifference = 0xff, description = '', characterCount = 8, session = null, join = false, timeoutMs = CONNECT_TIMEOUT_MS, startupMs = MCP_STARTUP_MS, gameReplyMs = GAME_REPLY_MS, onQueue = () => {}, signal = null } = {}) {
  if (!Array.isArray(realmReply) && !(realmReply instanceof Uint8Array)) throw new Error('MCP requires the BNCS realm handoff bytes');
  if (!character || (!gameName && !join)) throw new Error('MCP requires a character and game name');
  const sent = [], received = session ? [...session.received] : [];
  const send = (op, body) => {
    const frame = protocol.mcpFrame(op, body); sent.push({ op, bytes: frame.length }); transport.send(frame);
  };
  let list;
  if (session) {
    if (session.transport !== transport || session.protocol !== protocol || session.joined) throw new Error('MCP character session is not reusable');
    list = session.list;
  }
  let reader = null;
  const read = session ? sessionRead(session) : (ms) => readMcpFrame(reader, ms);
  if (!session) {
    reader = makeReader(transport);
    transport.send(protocol.mcpSelector());
    send(0x01, protocol.mcpStartup(realmReply));
    const startup = await mcpReply(read, 0x01, startupMs, received);
    if (startup.body.length < 4 || u32(startup.body, 0) !== 0) throw new Error('MCP STARTUP was rejected');
    send(0x19, protocol.mcpCharlist(characterCount));
    list = decodeMcpCharacters((await mcpReply(read, 0x19, timeoutMs, received)).body);
  }
  const selected = list.characters.find((c) => c.name.toLowerCase() === character.toLowerCase());
  if (!selected) throw new Error(`character "${character}" is not in the MCP character list`);

  // The character is normally logged on already (the lobby does it on entry); a session that is on another character logs this one on.
  if (session) { if ((session.characterLoggedOn || '').toLowerCase() !== selected.name.toLowerCase()) { sent.push({ op: 0x07 }); await mcpLogonCharacter(session, selected.name, { timeoutMs }); received.push(0x07); } }
  else {
    send(0x07, protocol.mcpCharlogon(selected.name));
    const charlogon = await mcpReply(read, 0x07, timeoutMs, received);
    if (charlogon.body.length < 4 || u32(charlogon.body, 0) !== 0) throw charlogonRefusal(charlogon.body);
  }

  const requestState = session || { nextRequestId: 2 };
  // `join` is the lobby's JOIN GAME: only MCP_JOINGAME is sent. Otherwise CREATE GAME sends
  // MCP_CREATEGAME and then joins the game it made. The create form's difficulty is the radio index
  // shifted into the request's flags (D2Multi's CREATE GAME callback, `shl 0xc`); D2GS takes the index.
  let gameToken = null;
  // Retail D2MCPClient 0x6540 / 0x64D0 drop a reply whose request id is not the pending one and keep waiting, and the
  // id counter skips 0. D2Multi's 30 s limit (0xDAE0) is checked only while the JOIN is pending, from the popup start.
  const nextId = () => mcpNextRequestId(requestState);
  // D2Multi's pump has no clock on a pending CREATE, but a server that never answers must not hang the player
  // for good, so the create wait is bounded too (same limit, counted from its own start).
  const gameReply = async (op, id, ms) => {
    let startedAt = Date.now(); const stage = op === 3 ? 'create' : 'join';
    for (;;) {
      // Cancel on the create wait (the Please Wait popup's, or the queue panel's) sends the realm the one-byte cancel request and ends the wait
      // with no notice. The join wait's Cancel has no handler in retail, so only a create can be cancelled.
      const frame = await read(Math.max(1, ms - (Date.now() - startedAt)), op === 3 ? signal : null).catch((error) => {
        if (error?.cancelled) send(0x13, []);
        throw error?.timedOut ? Object.assign(error, { mcpStage: stage, mcpResult: -1 }) : error;
      });
      received.push(frame.op);
      // The realm client files each reply by its opcode, so a reply to something else never ends this wait. A busy realm answers
      // CREATEGAME with its queue notice first (opcode 0x14, the place in line) and the real reply when the game is made:
      // the place is handed on and the wait starts over, since the realm has just shown it is alive.
      if (frame.op === 0x14 && frame.body.length >= 4) { onQueue(u32(frame.body, 0)); startedAt = Date.now(); }
      if (frame.op !== op) continue;
      if (frame.body.length >= 2 && u16(frame.body, 0) === id) return frame;
    }
  };
  if (!join) {
    const createId = nextId();
    send(0x03, protocol.mcpCreateGame({ requestId: createId, difficulty: (difficulty & 3) << 12, maxPlayers, levelDifference,
      name: titleCaseGameText(gameName), password: titleCaseGameText(gamePassword), description }));
    const created = await gameReply(0x03, createId, gameReplyMs);
    if (created.body.length < 10) throw new Error('MCP CREATEGAME reply is truncated');
    const result = u32(created.body, 6);
    if (result !== 0) throw Object.assign(new Error(`MCP CREATEGAME was rejected (0x${result.toString(16)})`), { mcpStage: 'create', mcpResult: result });
    gameToken = created.body[2] | (created.body[3] << 8);
  }

  const joinId = nextId();
  send(0x04, protocol.mcpJoinGame({ requestId: joinId, name: titleCaseGameText(gameName), password: titleCaseGameText(gamePassword) }));
  const joined = await gameReply(0x04, joinId, gameReplyMs);
  if (joined.body.length < 18) throw new Error('MCP JOINGAME reply is truncated');
  // Retail (D2MCPClient 0x64D0): the join failed only when the IP dword is zero; the code is then the result dword, or 0x28 when that is zero too.
  const joinResult = u32(joined.body, 6) === 0 ? (u32(joined.body, 14) || 0x28) : 0;
  if (joinResult !== 0) throw Object.assign(new Error(`MCP JOINGAME was rejected (0x${joinResult.toString(16)})`), { mcpStage: 'join', mcpResult: joinResult });
  const token = joined.body[2] | (joined.body[3] << 8);
  const ip = [...joined.body.subarray(6, 10)].join('.');
  const hash = u32(joined.body, 10);
  if (session) session.joined = true;
  return { character: selected, game: { name: titleCaseGameText(gameName), createToken: gameToken ?? token, difficulty }, ticket: { ip, token, hash }, sent, received };
}

const u16 = (b, o) => (b[o] | (b[o + 1] << 8)) >>> 0;
// Spaces stay; the first other character of each word is upper-cased and every later one lower-cased (not trimmed).
export function titleCaseGameText(value) {
  let start = true;
  return Array.from(String(value), (c) => {
    if (c === ' ') { start = true; return c; }
    const r = c.length > 1 || c > '\x7f' ? c : start ? c.toUpperCase() : c.toLowerCase();
    start = false;
    return r;
  }).join('');
}

/** Open the MCP login leg far enough to populate the character picker. */
export async function mcpListCharacters(transport, protocol, { realmReply, characterCount = 8, timeoutMs = CONNECT_TIMEOUT_MS, startupMs = MCP_STARTUP_MS, keepAliveMs = MCP_KEEPALIVE_MS } = {}) {
  if (!Array.isArray(realmReply) && !(realmReply instanceof Uint8Array)) throw new Error('MCP requires the BNCS realm handoff bytes');
  const reader = makeReader(transport), received = [];
  const send = (op, body) => transport.send(protocol.mcpFrame(op, body));
  transport.send(protocol.mcpSelector());
  send(0x01, protocol.mcpStartup(realmReply));
  const read = (ms) => readMcpFrame(reader, ms);
  const startup = await mcpReply(read, 0x01, startupMs, received);
  if (startup.body.length < 4 || u32(startup.body, 0) !== 0) throw new Error('MCP STARTUP was rejected');
  send(0x19, protocol.mcpCharlist(characterCount));
  const list = decodeMcpCharacters((await mcpReply(read, 0x19, timeoutMs, received)).body);
  const session = { reader, transport, protocol, list, nextRequestId: 2,
    characterLoggedOn: false, joined: false,
    received: [...received] };
  session.keepAlive = startMcpKeepAlive(session, keepAliveMs);
  return { ...list, received, session };
}

/**
 * Back to the lobby after a game: a fresh MCP connection on the realm handoff the BNCS logon already
 * gave (the realm accepts the same session on every STARTUP), the character list, then MCP_CHARLOGON
 * for the character that left the game. Returns the list plus a session that create and join reuse.
 */
export async function mcpReturnToLobby(transport, protocol, { realmReply, character, characters = [], timeoutMs = CONNECT_TIMEOUT_MS, startupMs = MCP_STARTUP_MS, keepAliveMs = MCP_KEEPALIVE_MS } = {}) {
  // Retail sends the selector, STARTUP and CHARLOGON only: no character-list request on the way back, so the roster is the one
  // the lobby already holds.
  if (!Array.isArray(realmReply) && !(realmReply instanceof Uint8Array)) throw new Error('MCP requires the BNCS realm handoff bytes');
  const reader = makeReader(transport), received = [];
  transport.send(protocol.mcpSelector());
  transport.send(protocol.mcpFrame(0x01, protocol.mcpStartup(realmReply)));
  const read = (ms) => readMcpFrame(reader, ms);
  const startup = await mcpReply(read, 0x01, startupMs, received);
  if (startup.body.length < 4 || u32(startup.body, 0) !== 0) throw new Error('MCP STARTUP was rejected');
  const found = characters.find((c) => c.name.toLowerCase() === String(character).toLowerCase());
  if (!found) throw new Error(`character "${character}" is not in the MCP character list`);
  transport.send(protocol.mcpFrame(0x07, protocol.mcpCharlogon(found.name)));
  const reply = await mcpReply(read, 0x07, timeoutMs, received);
  if (reply.body.length < 4 || u32(reply.body, 0) !== 0) throw charlogonRefusal(reply.body);
  const list = { characters, total: characters.length, returned: characters.length };
  const session = { reader, transport, protocol, list, nextRequestId: 2, characterLoggedOn: found.name, joined: false, received };
  session.keepAlive = startMcpKeepAlive(session, keepAliveMs);
  return { ...list, received, session };
}

// A refused MCP_CHARLOGON carries the realm's result, so the lobby can raise the notice that result has (gameErrorText).
const charlogonRefusal = (body) => Object.assign(new Error('MCP CHARLOGON was rejected'),
  { mcpStage: 'charlogon', mcpResult: body.length < 4 ? -1 : u32(body, 0) });
const u32 = (b, o) => (b[o] | (b[o + 1] << 8) | (b[o + 2] << 16) | (b[o + 3] << 24)) >>> 0;

/** MCP CHARCREATE (0x02): class, flags, name. On success the new character joins the session's roster. */
export async function mcpCreateCharacter(session, { name, charClass, expansion = true, hardcore = false, ladder = false, timeoutMs = CONNECT_TIMEOUT_MS } = {}) {
  if (!session || session.joined) throw new Error('MCP character session is not reusable');
  // Retail sends whatever its edit box took (letters, digits, punctuation, Latin-1) as UTF-8 and lets the realm decide: a name
  // it will not have comes back as a refusal, not as a client-side error. The box's 0x91 and 0x92 are the curly single quotes.
  const text = String(name || '').replace(/\u0091/g, '\u2018').replace(/\u0092/g, '\u2019');
  const encoded = new TextEncoder().encode(text);
  // Druids and Assassins exist only in the expansion, so retail always creates them as expansion characters.
  const flags = (expansion || charClass >= 5 ? 0x20 : 0) | (hardcore ? 0x04 : 0) | (ladder ? 0x40 : 0);
  const body = new Uint8Array(4 + 2 + encoded.length + 1);
  body[0] = charClass & 0xff; body[4] = flags & 0xff; body[5] = flags >> 8;
  body.set(encoded, 6);
  session.transport.send(session.protocol.mcpFrame(0x02, body));
  const reply = await mcpReply(sessionRead(session), 0x02, timeoutMs);
  const result = reply.body.length >= 4 ? u32(reply.body, 0) : 0;
  if (result !== 0) throw Object.assign(new Error(`MCP CHARCREATE was refused (0x${result.toString(16)})`), { result });
  const character = { name, expiry: 0, charClass, level: 1, flags: flags & 0xff };
  session.list.characters.push(character); session.list.total++; session.list.returned++;
  return character;
}

const MCP_LADDERDATA = 0x11;
/** MCP LADDERDATA (0x11): ask for one ladder by mode and read its answer (see realm-ladder.mjs for the layout). */
export async function mcpLadderRequest(session, mode, { timeoutMs = CONNECT_TIMEOUT_MS } = {}) {
  if (!session || session.joined) throw new Error('MCP character session is not reusable');
  const { parseLadderReply, ladderRequestBody } = await import('./realm-ladder.mjs');
  session.transport.send(session.protocol.mcpFrame(MCP_LADDERDATA, ladderRequestBody(mode)));
  for (;;) {
    const reply = await sessionInbox(session).next(timeoutMs);
    if (reply.op === MCP_LADDERDATA) return parseLadderReply(reply.body);
  }
}

const MCP_CHARDELETE = 0x0a, MCP_DELETE_COOKIE = 1;
/** MCP CHARDELETE request (0x0A): `[cookie:u16 LE][name\0]`; retail's reply handler keeps only the answer whose cookie it sent. */
export function mcpCharDeleteBody(name, cookie = MCP_DELETE_COOKIE) {
  const encoded = new TextEncoder().encode(String(name || '').slice(0, 1023));
  const body = new Uint8Array(2 + encoded.length + 1);
  body[0] = cookie & 0xff; body[1] = (cookie >> 8) & 0xff; body.set(encoded, 2);
  return body;
}
/** MCP CHARDELETE reply (0x0A): `[cookie:u16 LE][result:u32 LE]`; null when it is not the answer to `cookie`. */
export function parseMcpCharDeleteReply(body, cookie = MCP_DELETE_COOKIE) {
  if (body.length < 6 || (body[0] | (body[1] << 8)) !== cookie) return null;
  return { result: u32(body, 2) };
}
/**
 * Delete a character on the realm. A zero result removes it from the session's roster; any other result is a refusal
 * (the roster is untouched, the session stays usable) thrown as an Error carrying `.result`.
 */
export async function mcpDeleteCharacter(session, name, { timeoutMs = CONNECT_TIMEOUT_MS } = {}) {
  if (!session || session.joined) throw new Error('MCP character session is not reusable');
  session.transport.send(session.protocol.mcpFrame(MCP_CHARDELETE, mcpCharDeleteBody(name)));
  for (;;) {
    const reply = await sessionInbox(session).next(timeoutMs);
    if (reply.op !== MCP_CHARDELETE) continue;   // MOTD and friends can arrive in between
    const parsed = parseMcpCharDeleteReply(reply.body);
    if (!parsed) continue;
    if (parsed.result !== 0) throw Object.assign(new Error(`MCP CHARDELETE was refused (0x${parsed.result.toString(16)})`), { result: parsed.result });
    break;
  }
  const list = session.list, at = list.characters.findIndex((c) => c.name === name);
  if (at >= 0) { list.characters.splice(at, 1); list.total = Math.max(0, list.total - 1); list.returned = Math.max(0, list.returned - 1); }
  return list.characters;
}

const rol32 = (v, n) => ((v << n) | (v >>> (32 - n))) >>> 0;

/**
 * Battle.net's SHA-1 variant (the schedule rotates by one bit less): the account password hash. Retail walks the input in
 * 64-byte steps over a state that carries from step to step but never advances the input: every step hashes the first bytes
 * again (the last one zero padded), and an empty input leaves the initial state. Callers bound the string to the 255 bytes
 * retail's string copy keeps.
 */
export function bsha1(data) {
  let h = [0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0];
  for (let off = 0; off < data.length; off += 64) {
    const block = new Uint8Array(64); block.set(data.subarray(0, Math.min(data.length - off, 64)));
    const w = new Array(80);
    for (let i = 0; i < 16; i++) w[i] = u32(block, i * 4);
    for (let i = 16; i < 80; i++) w[i] = rol32(1, (w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16]) & 0x1f);
    let [a, b, c, d, e] = h;
    for (let i = 0; i < 80; i++) {
      let f, k;
      if (i < 20) { f = (b & c) | (~b & d); k = 0x5a827999; }
      else if (i < 40) { f = b ^ c ^ d; k = 0x6ed9eba1; }
      else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8f1bbcdc; }
      else { f = b ^ c ^ d; k = 0xca62c1d6; }
      const t = (rol32(a, 5) + (f >>> 0) + e + k + w[i]) >>> 0;
      e = d; d = c; c = rol32(b, 30); b = a; a = t;
    }
    h = [a, b, c, d, e].map((v, i) => (h[i] + v) >>> 0);
  }
  const out = new Uint8Array(20), dv = new DataView(out.buffer);
  h.forEach((v, i) => dv.setUint32(i * 4, v, true));
  return out;
}

/**
 * The version check every account request makes before its own packet (Battle.net refuses SID_CREATEACCOUNT,
 * SID_CHANGEPASSWORD and the rest until AUTH_CHECK has passed): selector, AUTH_INFO, AUTH_CHECK.
 * Resolves to the AUTH_INFO reply.
 */
async function bncsPrelude(transport, protocol, reader, { product, auth, timeoutMs }) {
  transport.send(protocol.bncsSelector());
  transport.send(protocol.bncsFrame(0x50, protocol.bncsAuthInfo(product)));
  const authInfo = await readBncsFrame(reader, timeoutMs, true, 0x50);
  if (authInfo.id !== 0x50) throw new Error(`expected AUTH_INFO reply 0x50, got 0x${authInfo.id.toString(16)}`);
  linkLostAfterAuthInfo(reader);
  const mpqEnd = authInfo.body.indexOf(0, 20);
  const mpq = authInfo.body.subarray(20, mpqEnd < 0 ? authInfo.body.length : mpqEnd);
  const { body } = protocol.bncsAuthCheck({ serverToken: u32(authInfo.body, 4), mpq: new TextDecoder().decode(mpq),
    challenge: authInfo.body.subarray(mpqEnd + 1), product, auth });
  transport.send(protocol.bncsFrame(0x51, body));
  const check = await readBncsFrame(reader, timeoutMs, true, 0x51);
  if (check.id !== 0x51) throw new Error(`expected AUTH_CHECK reply 0x51, got 0x${check.id.toString(16)}`);
  if (u32(check.body, 0) !== 0) throw new Error(`AUTH_CHECK rejected the version (result 0x${u32(check.body, 0).toString(16)})`);
  return authInfo;
}

/** SID_CREATEACCOUNT (0x3d): the password hash then the account name. Resolves on success. */
export async function bncsCreateAccount(transport, protocol, { account, password, product = 'D2XP', auth = null, timeoutMs = BNCS_REPLY_MS } = {}) {
  const reader = makeReader(transport);
  const authInfo = await bncsPrelude(transport, protocol, reader, { product, auth, timeoutMs });
  const enc = new TextEncoder();
  const hash = bsha1(enc.encode(password.toLowerCase()).subarray(0, 255));
  const name = enc.encode(account).subarray(0, 255);
  const body = new Uint8Array(hash.length + name.length + 1);
  body.set(hash); body.set(name, hash.length);
  transport.send(protocol.bncsFrame(0x3d, body));
  const reply = await readBncsFrame(reader, timeoutMs, false, 0x3d);
  if (reply.id !== 0x3d) throw new Error(`expected CREATEACCOUNT reply 0x3d, got 0x${reply.id.toString(16)}`);
  const result = u32(reply.body, 0);
  if (result !== 0) {
    const error = new Error(`This account name could not be created (result ${result})`);
    error.createResult = result;
    throw error;
  }
  return { result };
}

const cString = (text) => { const b = new TextEncoder().encode(text); const out = new Uint8Array(b.length + 1); out.set(b); return out; };
const joinBytes = (...parts) => { const out = new Uint8Array(parts.reduce((n, p) => n + p.length, 0)); let at = 0; for (const p of parts) { out.set(p, at); at += p.length; } return out; };

/**
 * SID_CHANGEPASSWORD (0x31): { u32 clientToken, u32 serverToken, u8[20] old proof, u8[20] new hash, STRING account }.
 * The old proof is the LOGONRESPONSE2 proof (the tokens and double hash the engine builds), the new hash the
 * single hash of the lowercased new password. Resolves to the reply's u32: 1 = changed, anything else is refused (D2Launch's pump tests the reply against 1).
 */
export async function bncsChangePassword(transport, protocol, { account, password, newPassword, product = 'D2XP', auth = null, timeoutMs = BNCS_REPLY_MS } = {}) {
  const reader = makeReader(transport);
  const authInfo = await bncsPrelude(transport, protocol, reader, { product, auth, timeoutMs });
  const logon = Uint8Array.from(protocol.bncsLogon({ clientToken: protocol.tickCount(), serverToken: u32(authInfo.body, 4), account, password }));
  const newHash = bsha1(new TextEncoder().encode(newPassword.toLowerCase()).subarray(0, 255));
  transport.send(protocol.bncsFrame(0x31, joinBytes(logon.subarray(0, 28), newHash, logon.subarray(28))));
  const reply = await readBncsFrame(reader, timeoutMs, false, 0x31);
  if (reply.id !== 0x31) throw new Error(`expected CHANGEPASSWORD reply 0x31, got 0x${reply.id.toString(16)}`);
  return u32(reply.body, 0);
}

/** SID_RESETPASSWORD (0x5A): { STRING account, STRING email }. Battle.net sends no reply. */
export async function bncsResetPassword(transport, protocol, { account, email, product = 'D2XP', auth = null, timeoutMs = BNCS_REPLY_MS } = {}) {
  if (!account || !email) throw new Error('retail sends no reset request without an account and an email');
  const reader = makeReader(transport);
  const authInfo = await bncsPrelude(transport, protocol, reader, { product, auth, timeoutMs });
  transport.send(protocol.bncsFrame(0x5a, joinBytes(cString(account), cString(email))));
}

/** SID_CHANGEEMAIL (0x5B): { STRING account, STRING old email, STRING new email }. No reply. */
export async function bncsChangeEmail(transport, protocol, { account, oldEmail, newEmail, product = 'D2XP', auth = null, timeoutMs = BNCS_REPLY_MS } = {}) {
  if (!account || !oldEmail || !newEmail) throw new Error('retail sends no email change without an account and both emails');
  const reader = makeReader(transport);
  const authInfo = await bncsPrelude(transport, protocol, reader, { product, auth, timeoutMs });
  transport.send(protocol.bncsFrame(0x5b, joinBytes(cString(account), cString(oldEmail), cString(newEmail))));
}

/**
 * The BNCS login: selector, AUTH_INFO, AUTH_CHECK, LOGONRESPONSE2, QUERYREALMS2, LOGONREALMEX.
 * Returns the challenge the server issued, the realm list it advertised, and the LOGONREALMEX
 * handoff (the MCP ticket) -- the read offsets the server cannot check for us.
 */
export async function bncsLogin(transport, protocol, { account, password, realm = 'd2-web', preferredRealm = '', product = 'D2XP', auth = null, timeoutMs = BNCS_REPLY_MS } = {}) {
  const reader = makeReader(transport);
  const sent = [], received = [];
  const send = (id, body) => { const frame = protocol.bncsFrame(id, body); sent.push({ id, bytes: frame.length }); transport.send(frame); };

  // A refused logon keeps the connection (retail's does), and the next LOG IN goes straight to the logon:
  // the selector and AUTH_INFO/AUTH_CHECK are once per connection, and a second 0x01 makes the server hang up.
  let serverToken, clientToken, mpqName, challenge, versionResult;
  if (transport.bncsAuth) {
    ({ serverToken, clientToken, mpqName, challenge, versionResult } = transport.bncsAuth);
  } else {
    transport.send(protocol.bncsSelector());                       // single 0x01 protocol selector
    send(0x50, protocol.bncsAuthInfo(product));

    const authInfo = await readBncsFrame(reader, timeoutMs, true, 0x50); received.push(authInfo.id);
    if (authInfo.id !== 0x50) throw new Error(`expected AUTH_INFO reply 0x50, got 0x${authInfo.id.toString(16)}`);
    linkLostAfterAuthInfo(reader);
    serverToken = u32(authInfo.body, 4);
    const mpqEnd = authInfo.body.indexOf(0, 20);
    const mpq = authInfo.body.subarray(20, mpqEnd < 0 ? authInfo.body.length : mpqEnd);
    challenge = authInfo.body.subarray(mpqEnd + 1);
    mpqName = new TextDecoder().decode(mpq);

    // BNClient+0x12060: the token is its running seed (seed += tick + time); the version, hash and info
    // come from the server's revision formula over the configured files, the key blocks from `auth`.
    const checked = protocol.bncsAuthCheck({ serverToken, mpq: mpqName, challenge, product, auth });
    const authCheckBody = checked.body;
    clientToken = checked.clientToken;
    send(0x51, authCheckBody);
    const authCheck = await readBncsFrame(reader, timeoutMs, true, 0x51); received.push(authCheck.id);
    if (authCheck.id !== 0x51) throw new Error(`expected AUTH_CHECK reply 0x51, got 0x${authCheck.id.toString(16)}`);
    versionResult = u32(authCheck.body, 0);
    if (versionResult !== 0) {
      // Bnclient+0x11330: the reply is a result and a C string (the key owner's name for 0x201); a body whose
      // last byte is not NUL, or with a NUL before it, is not handled at all, so no refusal result is kept.
      const error = new Error(`AUTH_CHECK rejected the version (result 0x${versionResult.toString(16)})`);
      const info = authCheck.body.subarray(4);
      if (authCheck.body.length >= 5 && info[info.length - 1] === 0 && !info.subarray(0, -1).includes(0)) {
        error.authResult = versionResult;
        error.authInfo = new TextDecoder().decode(info.subarray(0, -1));
      }
      throw error;
    }

    transport.bncsAuth = { serverToken, clientToken, mpqName, challenge, versionResult };
  }

  // BNClient+0xC9E0 tokens the logon with GetTickCount; +0xBFD0 with its own counter (1, 2, ...).
  send(0x3a, protocol.bncsLogon({ clientToken: protocol.tickCount(), serverToken, account, password }));
  // PvPGN prompts an account with no email address (SID_SETEMAIL 0x59) ahead of the logon result; like any other id it is passed over.
  const logon = await readBncsFrame(reader, timeoutMs, false, 0x3a); received.push(logon.id);
  if (logon.id !== 0x3a) throw new Error(`expected LOGONRESPONSE2 reply 0x3a, got 0x${logon.id.toString(16)}`);
  const logonResult = u32(logon.body, 0);
  if (logonResult !== 0) {
    // The result picks retail's message box (Bnclient+0x109A0): 1 no such account, 2 wrong password,
    // 3 account closed, 6 the server's own text, which follows the result as a C string.
    const error = new Error(`LOGONRESPONSE2 rejected the account (result ${logonResult})`);
    error.logonResult = logonResult;
    if (logonResult === 6) { const end = logon.body.indexOf(0, 4); error.logonReason = new TextDecoder().decode(logon.body.subarray(4, end < 0 ? logon.body.length : end)); }
    throw error;
  }

  send(0x40, protocol.bncsQueryRealms());                         // QUERYREALMS2, empty body
  const realms = await readBncsFrame(reader, timeoutMs, true, 0x40); received.push(realms.id);
  if (realms.id !== 0x40) throw new Error(`expected QUERYREALMS2 reply 0x40, got 0x${realms.id.toString(16)}`);
  const realmList = parseRealmList(realms.body);

  send(0x3e, protocol.bncsRealmEx({ clientToken: protocol.realmToken(), serverToken, realm: realmLogonTitle(realmList, realm, preferredRealm), password }));
  const realmEx = await readBncsFrame(reader, timeoutMs, true, 0x3e); received.push(realmEx.id);
  if (realmEx.id !== 0x3e) throw new Error(`expected LOGONREALMEX reply 0x3e, got 0x${realmEx.id.toString(16)}`);
  // MCPStatus is 0 or 1 on success: the engine's own server sends 1, a stock Battle.net realm sends 0,
  // and a failure sets the high bit (0x80000001...). Either way a real hand-off carries a non-zero IP.
  const status = u32(realmEx.body, 4);
  if (status >= 0x80000000 || ![0, 1].includes(status) || !realmEx.body.subarray(16, 20).some(Boolean)) {
    throw new Error(`LOGONREALMEX did not hand off the realm (MCPStatus 0x${status.toString(16)})`);
  }
  // The 1.13 schema lays the body out as MCPCookie(0) MCPStatus(4) MCPChunk1 x2(8,12)
  // IP[4] bytes(16) port u16 BE(20) zero(22) MCPChunk2 x12(24..72) name; so the IP is the four BYTES
  // at 16, not the u32 at 8, and the port is a big-endian u16 at 20.
  const ip = [...realmEx.body.subarray(16, 20)].join('.');
  const port = ((realmEx.body[20] << 8) | realmEx.body[21]) & 0xffff;
  const handoff = { ip, port, cookie: u32(realmEx.body, 0), uuid: u32(realmEx.body, 12),
    realmReply: [...realmEx.body],
    // MCP_STARTUP forwards bytes [0,16) then [24,72) of this reply -- the block the realm hands over.
    // A plain array, so the value survives the CDP hop a scenario result crosses (a typed array does not).
    startup: [...cat(realmEx.body.subarray(0, 16), realmEx.body.subarray(24, 72))] };

  // `reader` stays attached to the open socket: retail keeps the BNCS chat connection for the session.
  return { clientToken, serverToken, mpq: mpqName, challenge, versionResult, logonResult, realmList, handoff, sent, received, reader };
}

/**
 * The D2GS direct-connect join -- the TCP/IP HOST/JOIN leg. The server greets UNPROMPTED (`AF 01`)
 * before the client says anything; the client then sends GAMELOGON and reads the game-entry burst,
 * which arrives Huffman-compressed and length-framed. The engine's 1.13c packet table frames the
 * decoded stream; this routine waits until a complete player assignment is available.
 */
export async function d2gsJoin(transport, protocol, opts = {}) {
  const sent = { logonSent: false };
  try { return await d2gsJoinRun(transport, protocol, opts, sent); }
  catch (error) { if (error && typeof error === 'object') error.logonSent = sent.logonSent; throw error; }
}

/** A wasm trap that ends the engine instance ("RuntimeError: Aborted(...)"): nothing in it can be called again. */
export function isEngineAbort(error) {
  const text = `${error?.name ?? ''} ${error?.message ?? error ?? ''}`;
  return /\bAborted\(/.test(text) || (/RuntimeError/.test(text) && /abort|unreachable|out of bounds/i.test(text));
}

/** Call an engine function that may trap. Returns true when it ran; an abort is reported through onDead and
 *  returns false, any other error is the caller's to ignore (it is rethrown only when `strict`). */
export function callEngine(fn, { onDead = () => {}, strict = false } = {}) {
  try { fn(); return true; }
  catch (error) {
    if (isEngineAbort(error)) { onDead(error); return false; }
    if (strict) throw error;
    return true;
  }
}

/** Whether a failed game-server connect may be tried again. The game ticket is single use: once the 0x68 logon
 *  has gone out the server has consumed it, so another attempt can only fail (or load the character twice). */
export function canRetryJoin(error, { attempt = 0, maxRetries = 2, expired = false } = {}) {
  if (expired || attempt >= maxRetries) return false;
  if (isEngineAbort(error)) return false;
  return error?.logonSent !== true;
}

/** Save and Exit teardown: the 0x69 byte is out, so let the game server finish saving and close the socket
 *  itself. Resolves when the transport reports closed or after `ms`, then closes it from this side. */
export function closeAfterSave(transport, { ms = 3000, setTimer = setTimeout, clearTimer = clearTimeout } = {}) {
  return new Promise((resolve) => {
    if (!transport || transport.isOpen === false) { resolve('closed'); return; }
    let done = false;
    const finish = (why) => {
      if (done) return; done = true; clearTimer(timer);
      try { transport.close(); } catch { /* already closed */ }
      resolve(why);
    };
    const timer = setTimer(() => finish('timeout'), ms);
    try { transport.onClose(() => finish('closed')); } catch { finish('timeout'); }
  });
}

async function d2gsJoinRun(transport, protocol, { character, charClass, gameToken, gameHash, difficulty = 0, expansion = false, timeoutMs = CONNECT_TIMEOUT_MS } = {}, sent) {
  const reader = makeReader(transport);
  // The greeting's size is decided by its first two bytes (`AF 00`/`AF 01` are 2 bytes, `AF 81`
  // carries a 128-byte inline table), so read the flag pair first and then the whole thing.
  const flag = await reader.want(2, timeoutMs);
  const greeting = protocol.greetingParse(flag);
  const greetingSize = greeting < 0 ? 0 : greeting >> 8, mode = greeting & 0xff;
  if (!greetingSize) throw new Error(`not a D2GS greeting: ${hex(flag)}`);
  await reader.want(greetingSize, timeoutMs);
  reader.skip(greetingSize);

  const gamelogon = protocol.d2gsGamelogon({ character, charClass, gameToken, gameHash, expansion });
  sent.logonSent = transport.send(gamelogon) !== false;   // a send on a closed socket never reached the server

  const decoded = [];
  let frameCount = 0, assignOffset = -1, handshakeEnd = -1, playerAssignment = null;
  let receiptScanAt = 0, fed = null, tail = null, applied = 0;
  const joinReassignments = [];
  // `AF 00` is the unframed dialect the stock 1.13c game server speaks: after the greeting the stream is a bare
  // run of packets that only the size table delimits. Reading it as Huffman frames drops the join on the
  // server's first bytes (01 .. 00 02), so the client would hang up before ever asking to enter the game.
  const raw = mode === 0;
  const nextFrame = () => (raw ? readD2gsRawPackets : readD2gsFrame)(reader, protocol, timeoutMs);
  let enterSent = false;
  if (typeof protocol.d2gsFindPlayerAssign !== 'function') throw new Error('engine D2GS packet framing is unavailable');
  if (typeof protocol.d2gsClientPlayerFeed !== 'function') throw new Error('engine D2GS client state is unavailable');
  protocol.d2gsClientPlayerReset?.();
  protocol.d2gsClientSetDifficulty?.(difficulty);
  // The game server repeats its 2-byte AF greeting once the logon is accepted; it is not part of the frame stream.
  if (!raw) {
    const head = await reader.want(2, timeoutMs);
    if (head[0] === 0xaf && head[1] <= 1) reader.skip(2);
  }
  while (handshakeEnd < 0) {
    const frame = await nextFrame();
    decoded.push(...frame);
    frameCount++;
    if (decoded.length > 65536 || frameCount > 32) throw new Error('D2GS join stream exceeded the player selection scan limit');
    const response = Uint8Array.from(decoded);
    // Observe complete packets as soon as the decoded receive stream contains them. The engine
    // roster is installed only after the matching 0x0B below, so a selected-player 0x15 during
    // join cannot be inferred later from the already-updated roster.
    while (receiptScanAt < response.length) {
      const packet = response.subarray(receiptScanAt);
      const packetLength = protocol.d2gsPacketSize(packet);
      if (packetLength < 0) throw new Error('D2GS join stream contains an invalid 1.13c server packet');
      if (packetLength === 0) break;
      const deny = readD2gsDeny(packet);
      if (deny) throw d2gsDenyError(deny.code, (charClass ?? 0) >= 5 || expansion);
      // The server's 0x02 (game load success) is answered with the one-byte 0x6B before it sends the world.
      if (packet[0] === 0x02 && !enterSent) { enterSent = true; transport.send(Uint8Array.of(0x6b)); }
      if (packet[0] === 0x15) {
        const reassignment = protocol.d2gsPlayerReassign?.(packet.subarray(0, packetLength));
        if (reassignment) joinReassignments.push({ offset: receiptScanAt, update: reassignment });
      }
      receiptScanAt += packetLength;
    }
    const scan = protocol.d2gsFindPlayerAssign(response);
    if (scan.status === -2) throw new Error('D2GS join stream has an unknown or invalid 1.13c server packet');
    if (scan.status !== 1) continue;
    assignOffset = scan.offset;
    if (assignOffset + 26 > response.length) throw new Error('D2GS assignment scan returned a truncated packet');
    playerAssignment = protocol.d2gsPlayerAssign?.(response.subarray(assignOffset, assignOffset + 26)) ?? null;
    if (!playerAssignment) throw new Error('D2GS player assignment 0x59 failed engine validation');
    if (playerAssignment.name.toLowerCase() !== String(character).toLowerCase())
      throw new Error(`D2GS assigned player "${playerAssignment.name}" but MCP selected "${character}"`);
    if (playerAssignment.classId !== charClass)
      throw new Error(`D2GS assigned class ${playerAssignment.classId} but MCP selected class ${charClass}`);

    // The native join sender constructs the player's 0x59 CUnit, then selects that already
    // created unit with the type/GUID 0x0B Handshake. Walk complete engine-framed packets until
    // that exact handshake is present; it may arrive in a later Huffman frame than the assignment.
    for (let at = 0; at < response.length;) {
      const packetLength = protocol.d2gsPacketSize(response.subarray(at));
      if (packetLength < 0) throw new Error('D2GS join stream contains an invalid 1.13c server packet');
      if (packetLength === 0) break;
      if (at > assignOffset && response[at] === 0x0b && packetLength === 6 && response[at + 1] === 0) {
        const guid = (response[at + 2] | (response[at + 3] << 8) | (response[at + 4] << 16) | (response[at + 5] << 24)) >>> 0;
        if (guid === playerAssignment.guid) {
          handshakeEnd = at + packetLength;
          break;
        }
      }
      at += packetLength;
    }
    // The stock game server names the player (0x59) BEFORE it sends the act (0x03 LoadAct), and the engine's
    // client state refuses a player assigned with no act. Feed the act first when it is here; when the engine
    // still refuses (the act or its rooms are not all here yet) read on and try again instead of failing a join
    // that would then retry against a spent ticket.
    if (handshakeEnd >= 0) {
      // The act, then every room reveal (0x07) of the burst, go ahead of the player: the player's room must be
      // active when the engine places it, and the stock server sends the reveals after the assignment.
      const act = findPacket(response, protocol, 0x03);
      const early = [];
      if (act && act.at > assignOffset) {
        early.push(act);
        for (let at = 0; at < response.length;) {
          const size = protocol.d2gsPacketSize(response.subarray(at));
          if (size <= 0) break;
          if (at > assignOffset && at > act.at && response[at] === 0x07) early.push({ at, size });
          at += size;
        }
      }
      const taken = (i) => early.some((p) => i >= p.at && i < p.at + p.size);
      const cut = (from, to) => [...response.subarray(from, to)].filter((_, i) => !taken(from + i));
      const lead = early.flatMap((p) => [...response.subarray(p.at, p.at + p.size)]);
      fed = Uint8Array.from(early.length ? [...response.subarray(0, assignOffset), ...lead, ...cut(assignOffset, handshakeEnd)] : response.subarray(0, handshakeEnd));
      tail = Uint8Array.from(cut(handshakeEnd, response.length));
      applied = protocol.d2gsClientPlayerFeed(fed);
      if (applied === -5) throw new Error('D2GS join needs the game data archives mounted before the act can load');
      if (applied < 1) { handshakeEnd = -1; applied = 0; fed = null; }   // the engine resets itself on a refusal
    }
  }
  const response = Uint8Array.from(decoded);
  // Keep packet offsets from the engine table for transport evidence, without assuming any order.
  const opcodes = [];
  for (let at = 0; at < decoded.length;) {
    const size = protocol.d2gsPacketSize(response.subarray(at));
    if (size < 0) throw new Error('D2GS join stream contains an invalid 1.13c server packet');
    if (size === 0) break;
    opcodes.push(at);
    at += size;
  }
  if (!fed) { fed = response.subarray(0, handshakeEnd); tail = response.subarray(handshakeEnd); applied = protocol.d2gsClientPlayerFeed(fed); }
  if (applied < 1) throw new Error(`D2GS engine did not parse the assigned player before selection (applied ${applied})`);
  const playerState = protocol.d2gsClientPlayerState?.();
  if (!playerState?.active || playerState.guid !== playerAssignment.guid)
    throw new Error('D2GS engine did not select the assigned player after the 0x0B Handshake');
  const worldState = protocol.d2gsClientWorldState?.() ?? null;
  const transitionFade = joinReassignments.some(({ offset, update }) => offset < handshakeEnd &&
    update.unitType === 0 && update.guid === playerAssignment.guid && (update.value & 0xff) !== 0);

  const result = { greeting: { size: greetingSize, mode }, burstBytes: [...response], headBytes: Array.from(fed), opcodes, bytes: response.length,
    playerAssignment, playerState, worldState, transitionFade, gamelogon: gamelogon.length, gamelogonBytes: Array.from(gamelogon) };
  const pending = tail.length ? [tail] : [];
  const readFrame = async () => pending.length ? pending.shift() : (raw ? readD2gsRawPackets : readD2gsFrame)(reader, protocol, null);
  readFrame.cancel = () => reader.cancel();
  // The same stream read without waiting: the join's tail, then the complete packets already received, or null when there
  // are none. The in-process game hands its client every packet of a server frame before the client's frame runs.
  const readFrameNow = () => {
    if (pending.length) return pending.shift();
    if (!raw) return null;
    const buf = reader.buffered();
    let at = 0;
    for (;;) {
      const n = at < buf.length ? protocol.d2gsPacketSize(buf.subarray(at)) : 0;
      if (n < 0) throw new Error('D2GS stream contains an invalid 1.13c server packet');
      if (n === 0) break;
      at += n;
    }
    if (!at) return null;
    const out = buf.slice(0, at); reader.skip(at); return out;
  };
  // D2GS remains a continuous stream after the entry burst; reuse this reader for later frames.
  // Keep the method non-enumerable so browser-harness evidence stays plain JSON data.
  Object.defineProperties(result, {
    readFrame: { value: readFrame },
    readFrameNow: { value: readFrameNow },
  });
  return result;
}

// The first complete packet with `opcode` at or after byte `from` of an unframed stream, as { at, size }.
function findPacket(stream, protocol, opcode, from = 0) {
  for (let at = 0; at < stream.length;) {
    const size = protocol.d2gsPacketSize(stream.subarray(at));
    if (size <= 0) return null;
    if (at >= from && stream[at] === opcode) return { at, size };
    at += size;
  }
  return null;
}

/** Retail's 13-byte D2GS keep-alive, sent every five seconds while the game socket is retained. */
export function startD2gsKeepAlive(transport, protocol, { intervalMs = 5000, now = () => Date.now(),
  setIntervalFn = setInterval, clearIntervalFn = clearInterval, onSent = () => {} } = {}) {
  const started = now();
  let active = true, sent = 0;
  let timer;
  const stop = () => {
    if (!active) return;
    active = false;
    clearIntervalFn(timer);
  };
  timer = setIntervalFn(() => {
    if (!active) return;
    const tick = (now() - started) >>> 0;
    const frame = protocol.d2gsPing(tick);              // null: D2Client+0x14B00's five-second gate held it
    if (!frame) return;
    transport.send(frame);
    sent++;
    onSent(sent, tick);
  }, intervalMs);
  transport.onClose?.(stop);
  // D2Client+0x422E0: feed each 0x8F pong back so the next ping carries the averaged round trip.
  const pong = (payload) => protocol.d2gsPong?.(payload, (now() - started) >>> 0);
  return { stop, pong, get active() { return active; }, get sent() { return sent; } };
}

/** Decode later server frames continuously so an idle socket cannot grow the reader buffer. */
export function startD2gsReceiveLoop(readFrame, { onFrame = () => {}, onClose = () => {} } = {}) {
  let active = true, frames = 0;
  const done = (async () => {
    while (active) {
      try {
        const rawPayload = await readFrame();
        if (!active) break;
        const payload = rawPayload instanceof Uint8Array ? rawPayload : Uint8Array.from(rawPayload);
        frames++;
        onFrame({ count: frames, byteLength: payload.length, prefix: Array.from(payload.subarray(0, 8)) }, payload);
      } catch (error) {
        if (active) { active = false; onClose(error); }
        break;
      }
    }
  })();
  return { stop() { if (!active) return; active = false; readFrame.cancel?.(); }, done, get active() { return active; }, get frames() { return frames; } };
}

/**
 * The receive loop of a game whose server runs in the same process: every packet already received is dispatched when
 * `drain()` is called, in arrival order, with no promise hop. Single player's main loop receives and dispatches what
 * the server frame flushed before the client frame runs, all in one pass (D2Client+0x43920 inside +0x44B50).
 */
export function startD2gsSyncReceive(readFrameNow, transport, { onFrame = () => {}, onClose = () => {} } = {}) {
  let active = true, frames = 0;
  const fail = (error) => { if (!active) return; active = false; onClose(error); };
  transport?.onClose?.(() => fail(new Error('D2GS transport closed')));
  const drain = () => {
    while (active) {
      let payload;
      try { payload = readFrameNow(); } catch (error) { fail(error); break; }
      if (!payload) break;
      frames++;
      onFrame({ count: frames, byteLength: payload.length, prefix: Array.from(payload.subarray(0, 8)) }, payload);
    }
    return frames;
  };
  return { stop() { active = false; }, drain, done: Promise.resolve(), get active() { return active; }, get frames() { return frames; } };
}

/**
 * Bind the `protocol` primitives to the engine's realm exports. Every message BODY is composed by
 * `src/core/realm` through `Module.ccall` -- the same sources the server and realmd compile -- so the
 * client never re-implements a wire layout. Common to the test runner scenario and the page, so neither
 * can drift from the other. Call `protocol.free()` when done.
 */
export function bindEngineProtocol(Module, scratch = 65536, host = {}) {
  const P = Module._malloc(scratch), O = Module._malloc(scratch), T = Module._malloc(4096), E = Module._malloc(1);
  // The host values retail reads from Windows, supplied explicitly so a test pins every byte. A browser
  // has no socket address (+0x14 stays 0) and no Windows modules (the ping's module word is "not found").
  const env = { tick: () => Math.floor(globalThis.performance?.now?.() ?? Date.now()) >>> 0,
    time: () => Math.floor(Date.now() / 1000) >>> 0, language: 0, localIp: 0,
    timezoneBias: () => new Date().getTimezoneOffset(), lcid: 0x409, uiLanguage: 0x409,
    countryAbbr: 'USA', country: 'United States', pingModule: () => null, ...host };
  const SEED = Module._malloc(4), PING = Module._malloc(64), PN = Module._malloc(16);
  Module.HEAPU8.fill(0, Number(SEED), Number(SEED) + 4);
  Module.HEAPU8.fill(0, Number(PING), Number(PING) + 64);
  let realmCounter = 0;
  const u32at = (p) => { const b = Module.HEAPU8, o = Number(p); return (b[o] | (b[o + 1] << 8) | (b[o + 2] << 16) | (b[o + 3] << 24)) >>> 0; };
  const put32 = (p, v) => { const b = Module.HEAPU8, o = Number(p); b[o] = v & 0xff; b[o + 1] = (v >>> 8) & 0xff; b[o + 2] = (v >>> 16) & 0xff; b[o + 3] = (v >>> 24) & 0xff; };
  const call = (n, rt, a, v) => Module.ccall(n, rt, a, v);
  let weaponSet = 0;
  const hotkeys = Array.from({ length: 16 }, () => ({ skill: -1, left: false, owner: 0xffffffff }));
  const rd = (p, n) => Array.from(Module.HEAPU8.subarray(Number(p), Number(p) + n));
  // The skills table's row count; retail's 0x7B handler stores -1 for a skill above it. Without the export (a bare test module) nothing clamps.
  const engineSkillCount = () => (typeof Module._d2_realm_skill_count === 'function' ? call('d2_realm_skill_count', 'number', [], []) : 0xfff);
  const str = (bytes) => String.fromCharCode(...bytes.filter((b) => b !== 0));
  // The unit's path position at 16.16 precision, so an online unit glides between subtiles as it
  // does offline instead of stepping a whole subtile at a time. Null before its room is active.
  const unitPrecise = (type, guid) => {
    if (!Module._d2_realm_client_unit_precise) return null;
    if (!call('d2_realm_client_unit_precise', 'number', ['number', 'number', 'bigint'], [type, guid, BigInt(T)])) return null;
    return { preciseX: u32at(T) / 65536, preciseY: u32at(Number(T) + 4) / 65536 };
  };
  // A unit's life as the server's records left it on the client's own unit: the hit points and their maximum
  // (both in 1/256ths; a monster's maximum is the fixed 0x8000 its assignment sets) and the last hit class.
  const withLife = (record, type) => {
    if (!record || !Module._d2_realm_client_unit_life) return record;
    if (call('d2_realm_client_unit_life', 'number', ['number', 'number', 'bigint'], [type, record.guid, BigInt(T)]))
      Object.assign(record, { hp: u32at(T), hpMax: u32at(Number(T) + 4), lastHitClass: u32at(Number(T) + 8) });
    return record;
  };
  const withPrecise = (record, type) => {
    const precise = record ? unitPrecise(type, record.guid) : null;
    if (precise) Object.assign(record, precise);
    return record;
  };
  const readPlayerPointer = (ptr) => {
    if (!ptr) return null;
    const p = Number(ptr), view = new DataView(Module.HEAPU8.buffer, p, 92);
    if (view.getUint32(0, true) !== 5 || view.getUint32(4, true) !== 92) return null;
    const rawName = Module.HEAPU8.slice(p + 32, p + 48), end = rawName.indexOf(0);
    return { active: Boolean(view.getUint32(8, true)), guid: view.getUint32(12, true),
      x: view.getUint32(16, true), y: view.getUint32(20, true), classId: view.getUint8(24),
      reassignValue: view.getUint8(25), reassignments: view.getUint32(28, true),
      name: new TextDecoder().decode(rawName.subarray(0, end < 0 ? rawName.length : end)),
      clientRoomActive: Boolean(view.getUint32(48, true)), clientRoomLevel: view.getUint32(52, true),
      clientRoomTileX: view.getUint32(56, true), clientRoomTileY: view.getUint32(60, true),
      cunitInitSeed: view.getUint32(64, true), cunitInitSeedValid: Boolean(view.getUint32(68, true)),
      cunitRngLow: view.getUint32(72, true), cunitRngHigh: view.getUint32(76, true),
      cunitRngValid: Boolean(view.getUint32(80, true)), initialMode: view.getUint32(84, true),
      initialDirection: view.getUint32(88, true) };
  };
  const protocol = {
    bncsSelector: () => [0x01],
    tickCount: () => env.tick() >>> 0,
    realmToken: () => (realmCounter = (realmCounter + 1) >>> 0),
    bncsFrame: (id, body) => { call('d2_realm_bncs_header', 'number', ['number', 'number', 'bigint'], [id, body.length, BigInt(T)]); return [...rd(T, 4), ...body]; },
    // AUTH_INFO and AUTH_CHECK carry the host-filled words; LOGONRESPONSE2, QUERYREALMS2, LOGONREALMEX and the MCP
    // requests are composed by the 1.13c BNClient/D2MCPClient ports (src/core/realm/realm_client_retail.cpp).
    // BNClient+0x11960 with the host-filled words: language, local IP, timezone bias, LCIDs, country.
    bncsAuthInfo: (product) => {
      const n = call('d2_realm_bncs_auth_info_env', 'number',
        ['number', 'number', 'number', 'number', 'number', 'number', 'string', 'string', 'bigint'],
        [product === 'D2XP' ? 1 : 0, env.language, env.localIp, env.timezoneBias() | 0, env.lcid, env.uiLanguage, env.countryAbbr, env.country, BigInt(T)]);
      return rd(T, n);
    },
    // BNClient+0x12060. A classic formula ("A=.. B=.. C=.. 4 ...") runs the 1.13c path over the configured
    // revision files and CD keys; our own realmd's base64 challenge keeps the project's keyless form.
    bncsAuthCheck: ({ serverToken, mpq = '', challenge, product, auth }) => {
      const challengeStr = str(challenge), expansion = product === 'D2XP' ? 1 : 0;
      const keyless = !challengeStr.includes('=') || /^[A-Za-z0-9+/]*=*$/.test(challengeStr);
      // With no revision files configured, a formula challenge gets the keyless request: the project's own realm does not verify it.
      // A server that checks a fixed Game.exe version and checksum has them in its config (auth.exeVersion, auth.exeHash, auth.exeInfo).
      if (keyless || !auth?.revisionFiles?.length) {
        const clientToken = (u32at(SEED) + env.tick() + env.time()) >>> 0;
        put32(SEED, clientToken);
        const revLen = keyless ? call('d2_realm_bncs_checkrev', 'number', ['string', 'string', 'number', 'bigint'], [challengeStr, '1.13.2.0', 0, BigInt(O)]) : 0;
        const checkrev = keyless ? str(rd(O, revLen)) : '';
        const n = call('d2_realm_bncs_auth_check_request', 'number', ['number', 'number', 'number', 'number', 'bigint', 'number', 'string', 'string', 'bigint'], [clientToken, auth?.exeVersion ?? 0x0e, auth?.exeHash ?? 0, 0, BigInt(E), 0, auth?.exeInfo ?? product + ';' + checkrev, auth?.keyOwner ?? 'd2-web', BigInt(T)]);
        return { body: rd(T, n), clientToken };
      }
      const files = auth?.revisionFiles, keys = auth?.cdKeys ?? [];
      if (!files || files.length !== 3 || !auth.exeTime || !keys[0] || (expansion && !keys[1]))
        throw new Error('AUTH_CHECK: this server asks for a classic revision check; its config needs auth.revisionFiles (3), auth.exeTime and auth.cdKeys');
      const total = files.reduce((a, f) => a + f.length, 0);
      const F = Module._malloc(total + 16), Z = Module._malloc(16);
      let off = 0;
      for (const [i, f] of files.entries()) { Module.HEAPU8.set(f, Number(F) + off); put32(Number(Z) + 4 * i, f.length); off += f.length; }
      try {
        const n = call('d2_realm_bncs_auth_check_env', 'number',
          ['bigint', 'number', 'number', 'number', 'number', 'string', 'string', 'bigint', 'bigint', 'string', 'string', 'string', 'string', 'bigint'],
          [BigInt(SEED), env.time(), env.tick(), serverToken, expansion, challengeStr, mpq, BigInt(F), BigInt(Z), auth.exeTime,
            keys[0], keys[1] ?? '', auth.keyOwner ?? '', BigInt(O)]);
        if (n === -1) throw new Error('AUTH_CHECK: a configured CD key does not decode');
        if (n <= 0) throw new Error(`AUTH_CHECK: the revision check failed for ${mpq}`);
        return { body: rd(O, n), clientToken: u32at(SEED) };
      } finally { Module._free(F); Module._free(Z); }
    },
    bncsLogon: ({ clientToken, serverToken, account, password }) => { const n = call('d2_realm_client_bncs_logon', 'number', ['number', 'number', 'string', 'string', 'bigint'], [clientToken, serverToken, account, password, BigInt(T)]); return rd(T, n); },
    bncsQueryRealms: () => { const n = call('d2_realm_client_bncs_query_realms', 'number', ['bigint'], [BigInt(T)]); if (n < 0) throw new Error('QUERYREALMS2 composer failed'); return rd(T, n); },
    // D2Launch passes the literal realm password "password" to BNClient+0x0BFD0 (slot +0x44), never the account's.
    bncsRealmEx: ({ clientToken, serverToken, realm }) => { const n = call('d2_realm_client_bncs_logon_realm', 'number', ['number', 'number', 'string', 'string', 'bigint'], [clientToken, serverToken, realm, 'password', BigInt(T)]); return rd(T, n); },
    mcpSelector: () => { const n = call('d2_realm_mcp_protocol_selector', 'number', ['bigint'], [BigInt(T)]); return rd(T, n); },
    mcpFrame: (op, body) => { const n = call('d2_realm_mcp_header', 'number', ['number', 'number', 'bigint'], [op, body.length, BigInt(T)]); if (n < 3) throw new Error('MCP frame composer rejected the body'); return [...rd(T, 3), ...body]; },
    mcpStartup: (realmReply) => { Module.HEAPU8.set(realmReply, Number(P)); const n = call('d2_realm_client_mcp_startup', 'number', ['bigint', 'number', 'bigint'], [BigInt(P), realmReply.length, BigInt(T)]); if (!n) throw new Error('MCP STARTUP composer rejected the realm handoff'); return rd(T, n); },
    // Retail always asks for eight characters; the count argument is kept for the call sites only.
    mcpCharlist: (_count) => { const n = call('d2_realm_client_mcp_charlist', 'number', ['bigint'], [BigInt(T)]); return rd(T, n); },
    mcpCharlogon: (name) => { const n = call('d2_realm_client_mcp_charlogon', 'number', ['string', 'bigint'], [name, BigInt(T)]); if (!n) throw new Error('MCP CHARLOGON composer rejected the name'); return rd(T, n); },
    mcpCreateGame: ({ requestId, difficulty, maxPlayers, levelDifference = 0xff, name, password, description }) => { const n = call('d2_realm_client_mcp_creategame', 'number', ['number', 'number', 'number', 'number', 'string', 'string', 'string', 'bigint'], [requestId, difficulty, maxPlayers, levelDifference, name, password, description, BigInt(T)]); if (!n) throw new Error('MCP CREATEGAME composer failed'); return rd(T, n); },
    mcpJoinGame: ({ requestId, name, password }) => { const n = call('d2_realm_client_mcp_joingame', 'number', ['number', 'string', 'string', 'bigint'], [requestId, name, password, BigInt(T)]); if (!n) throw new Error('MCP JOINGAME composer failed'); return rd(T, n); },
    greetingParse: (bytes) => { Module.HEAPU8.set(bytes, Number(P)); return call('d2_realm_greeting_parse', 'number', ['bigint', 'number'], [BigInt(P), bytes.length]); },
    d2gsFrameSize: (bytes) => { Module.HEAPU8.set(bytes, Number(P)); return call('d2_realm_d2gs_frame_size', 'number', ['bigint', 'number'], [BigInt(P), bytes.length]); },
    d2gsPacketSize: (bytes) => { Module.HEAPU8.set(bytes, Number(P)); return call('d2_realm_d2gs_packet_size', 'number', ['bigint', 'number'], [BigInt(P), bytes.length]); },
    d2gsFindPlayerAssign: (bytes) => {
      Module.HEAPU8.set(bytes, Number(P));
      const status = call('d2_realm_d2gs_find_player_assign', 'number', ['bigint', 'number', 'bigint'], [BigInt(P), bytes.length, BigInt(O)]);
      return { status, offset: status === 1 ? Module.HEAP32[Number(O) >> 2] : -1 };
    },
    d2gsPlayerAssign: (bytes) => {
      Module.HEAPU8.set(bytes, Number(P));
      const n = call('d2_realm_d2gs_parse_player_assign', 'number', ['bigint', 'number', 'bigint'], [BigInt(P), bytes.length, BigInt(O)]);
      if (n !== 26) return null;
      const record = rd(O, n), rawName = record.slice(5, 21), end = rawName.indexOf(0);
      return { guid: (record[0] | (record[1] << 8) | (record[2] << 16) | (record[3] << 24)) >>> 0,
        classId: record[4], name: new TextDecoder().decode(Uint8Array.from(rawName.slice(0, end < 0 ? rawName.length : end))),
        x: record[22] | (record[23] << 8), y: record[24] | (record[25] << 8) };
    },
    d2gsPlayerReassign: (bytes) => {
      Module.HEAPU8.set(bytes, Number(P));
      const n = call('d2_realm_d2gs_parse_player_reassign', 'number', ['bigint', 'number', 'bigint'], [BigInt(P), bytes.length, BigInt(O)]);
      if (n !== 11) return null;
      const record = rd(O, 12);
      return { guid: (record[0] | (record[1] << 8) | (record[2] << 16) | (record[3] << 24)) >>> 0,
        x: record[4] | (record[5] << 8), y: record[6] | (record[7] << 8),
        unitType: record[8], value: record[9] };
    },
    d2gsClientPlayerReset: () => { weaponSet = 0; hotkeys.forEach((_, i) => { hotkeys[i] = { skill: -1, left: false, owner: 0xffffffff }; }); return call('d2_realm_client_player_reset', null, [], []); },
    // Server 0x97 (1 byte): flips the weapon-set flag when the game is an expansion game. The flag starts at 0.
    d2gsApplyWeaponSwitch: (packet, expansion) => {
      if (!(packet instanceof Uint8Array) || packet.length < 1 || packet[0] !== 0x97) throw new RangeError('not a D2GS weapon-set packet');
      if (!expansion) return false;
      weaponSet = weaponSet ? 0 : 1;
      return true;
    },
    d2gsWeaponSet: () => weaponSet,
    // Which weapon boxes show their empty-arm silhouettes and two-hander ghosts for the selected player's worn arms (-1 = no player).
    d2gsClientWeaponHands: () => call('d2_realm_client_weapon_hands', 'number', [], []),
    d2gsClientSetDifficulty: (difficulty) => call('d2_realm_client_set_difficulty', null, ['number'], [difficulty]),
    // Retail transaction price (-1 when unknown): an owned item by guid, or a shop stock item's bitstream.
    // The player's 96 quest-flag bytes from a server 0x28 (read by the vendor price rule).
    d2gsClientSetQuestFlags: (bytes) => {
      if (!(bytes instanceof Uint8Array) || bytes.length !== 96) throw new RangeError('quest flags are 96 bytes');
      Module.HEAPU8.set(bytes, Number(P));
      return call('d2_realm_client_set_quest_flags', 'number', ['bigint', 'number'], [BigInt(P), bytes.length]);
    },
    // The stats of a hire offer (S2C 0x4E: name id, seed) for the selected player, or null: { level, life, def, cost, id, exp }.
    // The roll is the server's. The game rolls each hire at purchase, with the game's difficulty and the hirer's level, in the
    // shared monster-roll routine; the client does not roll offers. The offer rows draw only the portrait, the name and the
    // owned count. The seed and the difficulty are the server's inputs, as the server sends them.
    // Open: the Lvl/Life/Def/Cost header is drawn by this file, and the hirer's level term has not been pinned against a
    // player unit with a level stat.
    d2gsClientHireStats: (mercName, seed, expansion) => {
      const at = Number(P);
      if (!call('d2_realm_client_hire_stats', 'number', ['number', 'number', 'number', 'bigint'], [mercName, seed, expansion ? 1 : 0, BigInt(P)])) return null;
      const w = new Int32Array(Module.HEAPU8.buffer, at, 6);
      return { level: w[0], life: w[1], def: w[2], cost: w[3], id: w[4], exp: w[5] };
    },
    // The game flags of the server's 0x01, or null before it arrives. `flags` is in the layout a local game's client flags use.
    d2gsClientGameInfo: () => {
      if (!Module._d2_realm_client_game_info || !call('d2_realm_client_game_info', 'number', ['bigint'], [BigInt(P)])) return null;
      const w = new Uint32Array(Module.HEAPU8.buffer, Number(P), 5);
      return { difficulty: w[0], gameFlags: w[1], expansion: w[2] !== 0, ladder: w[3] !== 0, flags: w[4] };
    },
    // The 0x9B offer to raise the dead mercenary, or null when none stands: { nameId, cost }.
    d2gsClientMercResurrect: () => {
      if (!Module._d2_realm_client_merc_resurrect || !call('d2_realm_client_merc_resurrect', 'number', ['bigint'], [BigInt(P)])) return null;
      const w = new Uint32Array(Module.HEAPU8.buffer, Number(P), 2);
      return { nameId: w[0], cost: w[1] };
    },
    // The next film index the server's 0x61 queued, or -1.
    d2gsClientTakeMovie: () => (Module._d2_realm_client_take_movie ? call('d2_realm_client_take_movie', 'number', [], []) : -1),
    // The guids of the NPCs carrying the speech mark (0x8A sets it, the 0x28 reply to a talk clears it).
    d2gsClientNpcAlerts: () => {
      if (!Module._d2_realm_client_npc_alerts) return [];
      const n = call('d2_realm_client_npc_alerts', 'number', ['bigint', 'number'], [BigInt(P), 256]);
      return Array.from(new Uint32Array(Module.HEAPU8.buffer, Number(P), n));
    },
    // The units other than monsters carrying the same mark (0x8A names any unit, the player's own hero included): [{ type, guid }].
    d2gsClientUnitMarks: () => {
      if (!Module._d2_realm_client_unit_marks) return [];
      const n = call('d2_realm_client_unit_marks', 'number', ['bigint', 'number'], [BigInt(P), 128]);
      const w = new Uint32Array(Module.HEAPU8.buffer, Number(P), n * 2);
      return Array.from({ length: n }, (_, i) => ({ type: w[i * 2], guid: w[i * 2 + 1] }));
    },
    // The panel flags the server's 0x01 left (by the client's panel number) and how many 0x01 arrived; null before the first.
    d2gsClientGameUi: () => {
      if (!Module._d2_realm_client_game_ui) return null;
      const starts = call('d2_realm_client_game_ui', 'number', ['bigint'], [BigInt(P)]);
      return starts ? { starts, panels: Array.from(new Uint32Array(Module.HEAPU8.buffer, Number(P) + 4, 38)) } : null;
    },
    d2gsClientSetActiveNpc: (guid) => { if (Module._d2_realm_client_set_active_npc) call('d2_realm_client_set_active_npc', null, ['number', 'number'], [guid == null ? 0 : 1, guid ?? 0]); },
    // The pet list of the server's 0x81 (newest first) and a stat its 0x9E..0xA2 set on a pet unit (null when never sent).
    d2gsClientPets: () => {
      if (!Module._d2_realm_client_pets) return [];
      const n = call('d2_realm_client_pets', 'number', ['bigint', 'number'], [BigInt(P), 64]);
      const w = new Uint32Array(Module.HEAPU8.buffer, Number(P), n * 9);
      return Array.from({ length: n }, (_, i) => ({ classId: w[i * 9], kind: w[i * 9 + 1], guid: w[i * 9 + 2], owner: w[i * 9 + 3], area: w[i * 9 + 4],
        lifePct: w[i * 9 + 5], seed: w[i * 9 + 6], nameId: w[i * 9 + 7], unitPresent: w[i * 9 + 8] !== 0 }));
    },
    d2gsClientPetStat: (guid, stat) => {
      if (!Module._d2_realm_client_pet_stat) return null;
      const v = call('d2_realm_client_pet_stat', 'number', ['number', 'number'], [guid, stat]);
      return v === -2147483648 ? null : v;
    },
    d2gsClientItemPrice: (itemGuid, vendorId, transType) => call('d2_realm_client_item_price', 'number', ['number', 'number', 'number'], [itemGuid, vendorId, transType]),
    // A carried item's sell line with NPC class `npcClass` trading (D2Client 0x46230), null for no line.
    d2gsClientSellValue: (itemGuid, npcClass) => {
      if (!Module._d2_realm_client_item_sell_value) return null;
      const v = call('d2_realm_client_item_sell_value', 'number', ['number', 'number'], [itemGuid >>> 0, npcClass]);
      return v < 0 ? null : v;
    },
    // A carried item's repair: needs (D2Client 0x45B40), and the mode and durability words the 0x35 carries; null when unknown.
    d2gsClientRepairInfo: (itemGuid) => {
      if (!Module._d2_realm_client_item_repair_field) return null;
      const mode = call('d2_realm_client_item_repair_field', 'number', ['number', 'number'], [itemGuid >>> 0, 0]);
      if (mode < 0) return null;
      return { needs: call('d2_realm_client_item_needs_repair', 'number', ['number'], [itemGuid >>> 0]) !== 0, mode,
        durability: call('d2_realm_client_item_repair_field', 'number', ['number', 'number'], [itemGuid >>> 0, 1]) };
    },
    d2gsClientStockPrice: (bits, vendorId, transType) => {
      Module.HEAPU8.set(bits, Number(P));
      return call('d2_realm_client_stock_price', 'number', ['bigint', 'number', 'number', 'number'], [BigInt(P), bits.length, vendorId, transType]);
    },
    d2gsClientPlayerFeed: (bytes) => {
      Module.HEAPU8.set(bytes, Number(P));
      return call('d2_realm_client_player_feed', 'number', ['bigint', 'number'], [BigInt(P), bytes.length]);
    },
    d2gsClientPlayerState: () => {
      const ptr = call('d2_realm_client_player_state', 'bigint', [], []);
      const player = readPlayerPointer(ptr);
      // The selected player is one of the roster entries. The 92-byte assignment view stays as it
      // is, but the same live motion the roster exposes must travel with it so a server mode change
      // reaches the selected player's draw instead of the assignment's fixed initialMode.
      if (player?.active) {
        const count = call('d2_realm_client_player_count', 'number', [], []);
        for (let index = 0; index < count; index++) {
          const other = readPlayerPointer(call('d2_realm_client_player_at', 'bigint', ['number'], [index]));
          if (other?.guid === player.guid) { player.motion = protocol.d2gsClientPlayerMotionAt(index); player.form = protocol.d2gsClientPlayerFormAt(index); break; }
        }
        withPrecise(player, 0);
      }
      return player;
    },
    d2gsClientGameChat: () => {
      const ptr = call('d2_realm_client_game_chat', 'bigint', [], []);
      if (!ptr) return null;
      const at = Number(ptr), view = new DataView(Module.HEAPU8.buffer, at, 312);
      if (view.getUint32(0, true) !== 1 || view.getUint32(4, true) !== 312) return null;
      const nameLength = view.getUint32(32, true), messageLength = view.getUint32(36, true);
      if (nameLength > 15 || messageLength > 255) return null;
      return { messageType: view.getUint32(8, true), unknown2: view.getUint32(12, true),
        subType: view.getUint32(16, true), unitGuid: view.getUint32(20, true),
        unknown8: view.getUint32(24, true), color: view.getUint32(28, true),
        name: Array.from(Module.HEAPU8.subarray(at + 40, at + 40 + nameLength)),
        message: Array.from(Module.HEAPU8.subarray(at + 56, at + 56 + messageLength)) };
    },
    d2gsClientGameChatCount: () => call('d2_realm_client_game_chat_count', 'number', [], []),
    // The players of the game as the roster packets left them: 52-byte records.
    d2gsClientRoster: () => {
      if (!Module._d2_realm_client_roster) return [];
      const n = call('d2_realm_client_roster_count', 'number', [], []), at = Number(call('d2_realm_client_roster', 'bigint', [], []));
      const view = new DataView(Module.HEAPU8.buffer, at, n * 52), out = [];
      for (let i = 0, o = 0; i < n; i++, o += 52) {
        const name = Module.HEAPU8.subarray(at + o + 36, at + o + 52);
        out.push({ guid: view.getUint32(o, true), classId: view.getUint32(o + 4, true), level: view.getUint32(o + 8, true), party: view.getUint32(o + 12, true),
          relation: view.getUint32(o + 16, true), portalA: view.getUint32(o + 20, true), portalB: view.getUint32(o + 24, true), corpses: view.getUint32(o + 28, true),
          kills: view.getInt32(o + 32, true), name: String.fromCharCode(...name.subarray(0, Math.max(0, name.indexOf(0)))) });
      }
      return out;
    },
    // Town portals: the destination area, the open flags and the owner's name, by object id.
    d2gsClientPortals: () => {
      if (!Module._d2_realm_client_portals) return [];
      const n = call('d2_realm_client_portal_count', 'number', [], []), at = Number(call('d2_realm_client_portals', 'bigint', [], []));
      const view = new DataView(Module.HEAPU8.buffer, at, n * 28), out = [];
      for (let i = 0, o = 0; i < n; i++, o += 28) {
        const name = Module.HEAPU8.subarray(at + o + 12, at + o + 28);
        out.push({ guid: view.getUint32(o, true), flags: view.getUint32(o + 4, true), area: view.getUint32(o + 8, true), owner: String.fromCharCode(...name.subarray(0, Math.max(0, name.indexOf(0)))) });
      }
      return out;
    },
    // The one-time things the packets asked for since the last call (a message line, a unit's sound, an
    // overlay, a cleared overhead text, a corpse tie, a worn-item recheck), oldest first; the queue is emptied.
    d2gsClientTakeSessionEvents: () => {
      if (!Module._d2_realm_client_session_events) return [];
      const n = call('d2_realm_client_session_event_count', 'number', [], []);
      if (!n) return [];
      const at = Number(call('d2_realm_client_session_events', 'bigint', [], [])), view = new DataView(Module.HEAPU8.buffer, at, n * 52), out = [];
      const text = (from) => { const b = Module.HEAPU8.subarray(from, from + 16); return String.fromCharCode(...b.subarray(0, Math.max(0, b.indexOf(0)))); };
      for (let i = 0, o = 0; i < n; i++, o += 52) {
        const event = { op: view.getUint32(o, true), a: view.getUint32(o + 4, true), b: view.getUint32(o + 8, true), c: view.getUint32(o + 12, true), d: view.getUint32(o + 16, true), name1: text(at + o + 20), name2: text(at + o + 36) };
        // A message line carries its finished text, composed from the string table as the game composes it.
        if (event.op === 0x5a && Module._d2_realm_client_session_event_text) event.text = call('d2_realm_client_session_event_text', 'string', ['number'], [i]);
        out.push(event);
      }
      call('d2_realm_client_session_events_clear', null, [], []);
      return out;
    },
    // What the game-wide packets left: the game's flags, the item the cursor is using, the cursor clears, the
    // value kept on the player's data, the last item whose use ended and the act whose animation data is current.
    d2gsClientGameSession: () => {
      if (!Module._d2_realm_client_game_session) return null;
      const at = Number(call('d2_realm_client_game_session', 'bigint', [], [])), v = new DataView(Module.HEAPU8.buffer, at, 84), u = (i) => v.getUint32(i * 4, true);
      return { gameFlagsSeen: u(0), difficulty: u(1), gameFlags: u(2), expansion: u(3), ladder: u(4),
        useItemSeen: u(5), useItemGuid: u(6), useItemKind: u(7), useCursorMode: u(8), useCursorPhase: u(9), useItemWord: u(10),
        cursorClears: u(11), cursorCleared: u(12), heroDwordSeen: u(13), heroDword: u(14),
        usedItemSeen: u(15), usedItemType: u(16), usedItemGuid: u(17), usedItemEnded: u(18), actDataSeen: u(19), actDataAct: u(20) };
    },
    d2gsClientPlayerCount: () => call('d2_realm_client_player_count', 'number', [], []),
    d2gsClientPlayerAt: (index) => {
      const ptr = call('d2_realm_client_player_at', 'bigint', ['number'], [index]);
      const player = readPlayerPointer(ptr);
      if (player?.active) { player.motion = protocol.d2gsClientPlayerMotionAt(index); player.form = protocol.d2gsClientPlayerFormAt(index); }
      return player;
    },
    // The monster a Disguise state (Werewolf, Werebear) turns this roster player into, as the engine's animation record; null otherwise.
    d2gsClientPlayerFormAt: (index) => {
      Module.HEAPU8.fill(0, Number(P), Number(P) + 96);
      if (!call('d2_realm_client_player_form_at', 'number', ['number', 'bigint'], [index, BigInt(P)])) return null;
      return parseMonsterAnimations(Module.HEAPU8.slice(Number(P), Number(P) + 96), 1)[0];
    },
    d2gsClientPlayerMotionAt: (index) => {
      const ptr = call('d2_realm_client_player_motion_at', 'bigint', ['number'], [index]);
      if (!ptr) return null;
      const view = new DataView(Module.HEAPU8.buffer, Number(ptr), 48);
      if (view.getUint32(0, true) !== 1 || view.getUint32(4, true) !== 48) return null;
      return { active: Boolean(view.getUint32(8, true)), mode: view.getUint32(12, true),
        direction: view.getUint32(16, true), framePrecise: view.getUint32(20, true),
        frameCountPrecise: view.getUint32(24, true), animSpeed: view.getUint32(28, true),
        positionHintX: view.getUint32(32, true), positionHintY: view.getUint32(36, true),
        seqMode: view.getUint32(40, true), seqFrame: view.getUint32(44, true),
        usedSkill: call('d2_realm_client_player_used_skill_at', 'number', ['number'], [index]) };
    },
    d2gsClientPlayers: () => {
      const count = call('d2_realm_client_player_count', 'number', [], []);
      const players = [];
      for (let index = 0; index < count; index++) {
        const ptr = call('d2_realm_client_player_at', 'bigint', ['number'], [index]);
        const player = readPlayerPointer(ptr);
        if (player?.active) {
          player.motion = protocol.d2gsClientPlayerMotionAt(index);
          player.form = protocol.d2gsClientPlayerFormAt(index);
          players.push(withPrecise(player, 0));
        }
      }
      return players;
    },
    // The quest state the server's packets built, or null before any arrived: the player's record (0x28) and the
    // game's (0x29) as 48 words each, the quest log's 41 state bytes (0x52, 0x5D), and the log button's state.
    // `logOpen` tells the state-change rule whether the quest log is on screen.
    d2gsClientQuests: (logOpen = false) => {
      if (call('d2_realm_client_quests', 'number', ['bigint', 'number', 'number'], [BigInt(O), scratch, logOpen ? 1 : 0]) !== 260) return null;
      const b = Module.HEAPU8.slice(Number(O), Number(O) + 260), have = b[0];
      if (!have) return null;
      const words = (at) => new Uint16Array(b.buffer.slice(at, at + 96));
      const v = new DataView(b.buffer);
      return { player: have & 1 ? words(4) : null, game: have & 2 ? words(100) : null, states: have & 4 ? b.slice(196, 237) : null,
        button: b[1], questOneCount: v.getInt32(240, true), quest32Count: v.getInt32(244, true), selected: b.slice(248, 258) };
    },
    // The client's active rooms, in the single-player room record's shape (same parser as d2_session_rooms).
    d2gsClientRooms: () => {
      const count = call('d2_realm_client_rooms', 'number', ['bigint', 'number'], [BigInt(O), scratch]);
      return parseSessionRooms(Module.HEAPU8.slice(Number(O), Number(O) + count * 36), count);
    },
    // The level exits the server assigned, in the single-player exit record's shape (same parser as
    // d2_session_warps); `id` is the exit's tile-unit guid, the one a click on it names to the server.
    d2gsClientWarps: () => {
      const count = call('d2_realm_client_warps', 'number', ['bigint', 'number'], [BigInt(O), scratch]);
      const b = Module.HEAPU8.slice(Number(O), Number(O) + count * 72), v = new DataView(b.buffer);
      return parseSessionWarps(b.subarray(0, count * 68), count).map((warp, i) => ({ ...warp, id: v.getUint32(count * 68 + i * 4, true) }));
    },
    // The bnet client's state in the single-player snapshot's shape (same parser as d2_session_snapshot).
    d2gsClientSnapshot: () => {
      const ptr = call('d2_realm_client_snapshot', 'bigint', [], []);
      return ptr ? parseSessionSnapshot(Module.HEAPU8.subarray(Number(ptr), Number(ptr) + 68)) : null;
    },
    // The selected player's life, mana and stamina (8.8 fixed point stat values), with their maxima.
    d2gsClientResources: () => {
      const ptr = call('d2_realm_client_resources', 'bigint', [], []);
      if (!ptr) return null;
      const v = new Int32Array(Module.HEAPU8.buffer.slice(Number(ptr), Number(ptr) + 32));
      return v[0] === 1 && v[1] === 32 ? { hp: v[2], hpMax: v[3], mana: v[4], manaMax: v[5], stamina: v[6], staminaMax: v[7] } : null;
    },
    // The corpses the server has named (0x8E): [{ owner, corpse }], both player-unit GUIDs.
    d2gsClientCorpses: () => {
      if (!Module._d2_realm_client_corpses) return [];
      const ptr = Number(call('d2_realm_client_corpses', 'bigint', [], []));
      if (!ptr) return [];
      const head = new DataView(Module.HEAPU8.buffer, ptr, 8);
      if (head.getUint32(0, true) !== 1) return [];
      const count = head.getUint32(4, true), view = new DataView(Module.HEAPU8.buffer, ptr + 8, count * 8);
      return Array.from({ length: count }, (_, i) => ({ owner: view.getUint32(i * 8, true), corpse: view.getUint32(i * 8 + 4, true) }));
    },
    // The selected player's character-screen numbers, computed in the wasm client from the hero unit's stat list
    // (the worn items' lists merged in) by the function the single-player panel reads: 16 values in the
    // d2_session_character_screen order, then [16] the base stamina maximum. Null before a player is selected.
    d2gsClientCharacterScreen: (expansion) => {
      if (!Module._d2_realm_client_character_screen) return null;
      const p = Module._malloc(64);
      try {
        if (!call('d2_realm_client_character_screen', 'number', ['number', 'bigint'], [expansion ? 1 : 0, BigInt(p)])) return null;
        const sc = Array.from(new Int32Array(Module.HEAPU8.buffer.slice(Number(p), Number(p) + 64)));
        sc[16] = call('d2_realm_client_hero_stat', 'number', ['number', 'number', 'number'], [11, 0, 1]) >> 8;
        return sc;
      } finally { Module._free(p); }
    },
    // The items a player's unit holds in the wasm client (0: the selected player): grids, body, belt and, first, the
    // cursor item. Null when the engine build has no such read.
    d2gsClientItems: (ownerGuid = 0) => {
      if (!Module._d2_realm_client_items) return null;
      const CAP = 512, p = Module._malloc(CAP * 48);
      try {
        const n = call('d2_realm_client_items', 'number', ['number', 'bigint', 'number'], [ownerGuid >>> 0, BigInt(p), CAP]);
        const w = new Uint32Array(Module.HEAPU8.buffer.slice(Number(p), Number(p) + n * 48));
        return Array.from({ length: n }, (_, i) => { const r = w.subarray(i * 12, i * 12 + 12);
          return { guid: r[0], classId: r[1], code: r[2], mode: r[3], x: r[4], y: r[5], page: r[6], bodyLoc: r[7], quality: r[8],
            flags: r[9], holder: r[10], fillers: r[11] & 0xff, applied: (r[11] & 0x100) !== 0 }; });
      } finally { Module._free(p); }
    },
    // The item-box colour tests of a held item against the selected player (the bits of the local game's read); 0 = not found.
    d2gsClientItemBoxState: (guid) => (Module._d2_realm_client_item_box_state
      ? call('d2_realm_client_item_box_state', 'number', ['number'], [guid >>> 0]) >>> 0 : 0),
    // A stat of the selected player: the total (worn items and states included), or the character's own with `base`.
    d2gsClientHeroStat: (stat, layer = 0, base = false) => (Module._d2_realm_client_hero_stat
      ? call('d2_realm_client_hero_stat', 'number', ['number', 'number', 'number'], [stat, layer, base ? 1 : 0]) : 0),
    d2gsClientWorldState: () => {
      const ptr = call('d2_realm_client_world_state', 'bigint', [], []);
      if (!ptr) return null;
      const view = new DataView(Module.HEAPU8.buffer, Number(ptr), 72);
      if (view.getUint32(0, true) !== 2 || view.getUint32(4, true) !== 72) return null;
      return { hasLoadAct: Boolean(view.getUint32(8, true)), act: view.getUint32(12, true),
        mapSeed: view.getUint32(16, true), area: view.getUint32(20, true),
        rawLoadActTail: view.getUint32(24, true), loadCompleteEvent: Boolean(view.getUint32(28, true)),
        loadActCount: view.getUint32(32, true), unloadCompleteEvent: Boolean(view.getUint32(36, true)),
        hasMapReveal: Boolean(view.getUint32(40, true)), mapLevel: view.getUint32(44, true),
        mapX: view.getUint32(48, true), mapY: view.getUint32(52, true), mapRevealCount: view.getUint32(56, true),
        difficulty: view.getUint32(60, true), mapHideCount: view.getUint32(64, true), mapRevision: view.getUint32(68, true) };
    },
    d2gsClientMonsters: () => {
      const count = call('d2_realm_client_monster_count', 'number', [], []);
      let modeEvents = [];
      if (Module._d2_realm_client_monster_death_mode_event_count && Module._d2_realm_client_monster_death_mode_events) {
        const eventCount = call('d2_realm_client_monster_death_mode_event_count', 'number', [], []);
        if (eventCount) {
          const eventPtr = Number(call('d2_realm_client_monster_death_mode_events', 'bigint', [], []));
          if (!eventPtr) throw new Error('engine returned no monster death-mode events');
          const eventView = new DataView(Module.HEAPU8.buffer, eventPtr, eventCount * 32);
          modeEvents = Array.from({ length: eventCount }, (_, row) => {
            const at = row * 32;
            if (eventView.getUint32(at, true) !== 1 || eventView.getUint32(at + 4, true) !== 32)
              throw new Error(`invalid monster death-mode event ${row}`);
            return { sequence: eventView.getUint32(at + 8, true), guid: eventView.getUint32(at + 12, true),
              packetOpcode: eventView.getUint32(at + 16, true), wireState: eventView.getUint32(at + 20, true),
              fromMode: eventView.getUint32(at + 24, true), toMode: eventView.getUint32(at + 28, true) };
          });
        }
      }
      if (!count) return [];
      const ptr = call('d2_realm_client_monsters', 'bigint', [], []);
      if (!ptr) throw new Error('engine returned no remote-monster records');
      const p = Number(ptr), view = new DataView(Module.HEAPU8.buffer, p, count * 116);
      const eventsByGuid = new Map();
      for (const event of modeEvents) {
        const events = eventsByGuid.get(event.guid) ?? [];
        events.push(event);
        eventsByGuid.set(event.guid, events);
      }
      return Array.from({ length: count }, (_, row) => {
        const at = row * 116;
        if (view.getUint32(at, true) !== 1 || view.getUint32(at + 4, true) !== 116)
          throw new Error(`invalid remote-monster record ${row}`);
        return { guid: view.getUint32(at + 8, true), classId: view.getUint32(at + 12, true),
          code: fourcc(view.getUint32(at + 16, true)), mode: view.getUint32(at + 20, true),
          framePrecise: view.getUint32(at + 24, true), frameCountPrecise: view.getUint32(at + 28, true),
          components: Array.from({ length: 16 }, (_, i) => fourcc(view.getUint32(at + 32 + i * 4, true))),
          animSpeed: view.getUint16(at + 96, true), direction: view.getUint8(at + 98),
          animationFlags: view.getUint8(at + 99), modeEvents: eventsByGuid.get(view.getUint32(at + 8, true)) ?? [],
          x: view.getUint32(at + 100, true), y: view.getUint32(at + 104, true),
          hpPct: view.getUint32(at + 108, true), typeFlags: view.getUint32(at + 112, true) };
      }).map((monster) => withLife(withPrecise(monster, 1), 1));
    },
    // One unit's relation, pet entry, plate name and second line (realm-combat.mjs parseRealmUnitInfo); null when the engine holds no such unit.
    d2gsClientUnitInfo: (type, guid) => {
      if (!Module._d2_realm_client_unit_info) return null;
      const ptr = Number(call('d2_realm_client_unit_info', 'bigint', ['number', 'number'], [type, guid >>> 0]));
      return ptr ? parseRealmUnitInfo(Module.HEAPU8.slice(ptr, ptr + 468)) : null;
    },
    d2gsClientMissiles: () => {
      const count = call('d2_realm_client_missile_count', 'number', [], []);
      if (!count) return [];
      const ptr = Number(call('d2_realm_client_missiles', 'bigint', [], []));
      if (!ptr) throw new Error('engine returned no client missile records');
      return parseRealmMissiles(Module.HEAPU8.subarray(ptr, ptr + count * 76), count);
    },
    d2gsClientTakeMissileSounds: () => {
      const cap = 128;
      const count = call('d2_realm_client_take_missile_sounds', 'number', ['bigint','number'], [BigInt(T),cap]);
      if (count < 0 || count > cap) throw new Error(`invalid client missile sound count ${count}`);
      return parseRealmMissileSoundEvents(Module.HEAPU8.subarray(T, T + count * 24), count);
    },
    // The object units of the joined game, and the activations (a shrine used) not yet taken. The engine's units
    // are the only model of an object, so an engine without the read-back is an error, never an empty scene.
    d2gsClientObjects: () => {
      if (typeof Module._d2_realm_client_object_units !== 'function') throw new Error('the engine does not export d2_realm_client_object_units');
      const cap = Math.floor(scratch / OBJECT_UNIT_BYTES);
      const count = call('d2_realm_client_object_units', 'number', ['bigint', 'number'], [BigInt(O), cap]);
      if (count < 0 || count > cap) throw new Error(`invalid client object count ${count}`);
      return parseRealmObjectUnits(Module.HEAPU8.slice(Number(O), Number(O) + count * OBJECT_UNIT_BYTES), count);
    },
    d2gsClientTakeObjectEvents: () => {
      const count = call('d2_realm_client_take_object_events', 'number', ['bigint', 'number'], [BigInt(T), 64]);
      if (count < 0 || count > 64) throw new Error(`invalid client object event count ${count}`);
      return parseRealmObjectEvents(new Int32Array(Module.HEAPU8.buffer.slice(T, T + count * 24)), count);
    },
    // The unit effects the engine's skill start and hit rules queued, with the engine tick they are timed against.
    d2gsClientTakeUnitFx: () => {
      const cap = 96;
      const count = call('d2_realm_client_take_unit_fx', 'number', ['bigint','number'], [BigInt(T),cap]);
      if (count < 0 || count > cap) throw new Error(`invalid client unit effect count ${count}`);
      return { tick: call('d2_realm_client_unit_fx_tick', 'number', [], []) >>> 0, events: parseRealmUnitFx(Module.HEAPU8.subarray(T, T + count * 32), count) };
    },
    d2gsClientUnitHitClass: (type, guid) => call('d2_realm_client_unit_hit_class', 'number', ['number','number'], [type, guid >>> 0]) >>> 0,
    d2gsClientMotionTick: () => call('d2_realm_client_motion_tick', 'number', [], []),
    // The own player's waiting action on a unit: the click, and the packets the engine's own steps want sent.
    d2gsClientLocalActOnUnit: Module._d2_realm_client_local_act_on_unit ? (kind, type, guid, run) => call('d2_realm_client_local_act_on_unit', 'number', ['number','number','number','number'], [kind, type, guid >>> 0, run ? 1 : 0]) : null,
    // The own player's left click on a player unit (D2Client 0x1AB20): the use in reach, else the walk with the use waiting.
    d2gsClientLocalClickPlayer: Module._d2_realm_client_local_click_player ? (guid, run) => call('d2_realm_client_local_click_player', 'number', ['number','number'], [guid >>> 0, run ? 1 : 0]) : null,
    d2gsClientLocalFire: Module._d2_realm_client_local_fire ? (kind, type, guid) => call('d2_realm_client_local_fire', 'number', ['number','number','number'], [kind, type, guid >>> 0]) : null,
    d2gsClientLocalSends: Module._d2_realm_client_local_sends ? () => {
      const n = call('d2_realm_client_local_sends', 'number', ['bigint'], [0n]), at = Number(call('d2_realm_client_local_send_view', 'bigint', [], [])) >>> 2;
      return Array.from({ length: n }, (_, i) => ({ op: Module.HEAPU32[at + i * 3], type: Module.HEAPU32[at + i * 3 + 1], guid: Module.HEAPU32[at + i * 3 + 2] }));
    } : null,
    d2gsGamelogon: ({ character, charClass, gameToken, gameHash, expansion = true }) => {
      if (Module._d2_realm_client_items_set_expansion) Module._d2_realm_client_items_set_expansion(expansion ? 1 : 0);   // picks the Inventory.txt rows of the hero's grids
      const n = call('d2_realm_d2gs_gamelogon', 'number', ['number', 'number', 'number', 'number', 'string', 'bigint'], [gameHash, gameToken, charClass, 0x0d, character, BigInt(T)]);
      const b = rd(T, n);
      // Retail D2Client+0x14A50: the version dword at +12 is 0x2185EDD6, plus 0xCBD7DE7A for an expansion game (the export writes the LoD sum).
      if (!expansion) b.splice(12, 4, 0xD6, 0xED, 0x85, 0x21);
      return b;
    },
    // D2Client+0x14B00: [6D][tick][avg rtt >> 1][module word * 0xB640DE41]; null while the 5 s gate holds.
    d2gsPing: (tick) => {
      const offset = call('d2_realm_d2gs_ping_module', 'number', ['bigint', 'bigint'], [BigInt(PING), BigInt(PN)]);
      const module = rd(PN, 8), name = str(module);
      const found = name ? env.pingModule(name, offset) : null;
      const n = call('d2_realm_d2gs_ping', 'number', ['bigint', 'number', 'number', 'number', 'number', 'bigint'],
        [BigInt(PING), tick >>> 0, found ? 1 : 0, found?.w20 ?? 0, found?.w24 ?? 0, BigInt(T)]);
      if (n === 0) return null;
      if (n !== 13) throw new Error(`D2GS ping composer returned ${n} bytes`);
      return rd(T, n);
    },
    // D2Client+0x422E0: the 0x8F pong's eight words land in the ping state and the round trip is averaged.
    d2gsPong: (payload, tick) => {
      Module.HEAPU8.set(payload.slice(0, Math.min(payload.length, 4096)), Number(T));
      return call('d2_realm_d2gs_pong', 'number', ['bigint', 'bigint', 'number', 'number'], [BigInt(PING), BigInt(T), Math.min(payload.length, 4096), tick >>> 0]);
    },
    // The pending position resync as the client packet {0x5F, x u16, y u16}, or null (realm-resync.mjs).
    d2gsTakeResync: () => {
      if (!Module._d2_realm_client_take_resync) return null;
      if (!call('d2_realm_client_take_resync', 'number', ['bigint'], [BigInt(T)])) return null;
      return rd(T, 5);
    },
    d2gsSelectedSkills: () => {
      const p = Number(call('d2_realm_client_selected_skills', 'bigint', [], []));
      if (!p) throw new Error('engine returned no selected skill record');
      const view = new DataView(Module.HEAPU8.buffer, p, 16);
      return {left:view.getInt32(0,true), right:view.getInt32(4,true),
        leftOwner:view.getInt32(8,true), rightOwner:view.getInt32(12,true)};
    },
    d2gsSkillChoices: (guid, cap = 128) => {
      if (!Number.isInteger(guid) || guid < 0 || guid > 0xffffffff) throw new RangeError('invalid D2GS player GUID');
      if (!Number.isInteger(cap) || cap < 0 || cap > 128) throw new RangeError('invalid D2GS skill choice capacity');
      const count = call('d2_realm_client_skill_choices', 'number', ['number','bigint','number'], [guid >>> 0, BigInt(T), cap]);
      if (!Number.isInteger(count) || count < 0 || count > cap) throw new Error(`engine returned invalid D2GS skill choice count ${count}`);
      return Uint8Array.from(rd(T, count * 8));
    },
    d2gsSelectSkill: (skill, left, owner = 0xffffffff) => {
      if (!Number.isInteger(skill) || skill < 0 || skill > 0xffff || typeof left !== 'boolean' ||
          !Number.isInteger(owner) || owner < -1 || owner > 0xffffffff)
        throw new RangeError('invalid D2GS skill selection');
      const n = call('d2_realm_d2gs_select_skill', 'number', ['number','number','number','bigint'], [skill,left?1:0,owner>>>0,BigInt(T)]);
      if (n !== 9) throw new Error(`D2GS skill selection composer returned ${n} bytes`);
      return Uint8Array.from(rd(T,n));
    },
    // Server hotkey table (0x7B, 8 bytes: op, slot, u16 skill|0x8000 left, u32 owner GUID): sixteen slots,
    // skill -1 = unbound, as the client's three parallel tables hold them.  Skill ids above `maxSkill`
    // (the game info's skill count) store -1.  Nothing but the server fills it.
    d2gsApplyHotkey: (packet, maxSkill = host.maxSkill ?? engineSkillCount()) => {
      if (!(packet instanceof Uint8Array) || packet.length < 8 || packet[0] !== 0x7b) throw new RangeError('not a D2GS hotkey packet');
      const slot = packet[1];
      if (slot >= hotkeys.length) return false;
      const word = packet[2] | (packet[3] << 8), skill = word & 0xfff;
      hotkeys[slot] = { skill: skill > maxSkill ? -1 : skill, left: (word & 0x8000) !== 0,
        owner: (packet[4] | (packet[5] << 8) | (packet[6] << 16) | (packet[7] << 24)) >>> 0 };
      return true;
    },
    d2gsHotkeys: () => hotkeys.map((h) => ({ ...h })),
    /** F-key press: the select-skill packet for that slot (hand and owner from the table), or null when unbound. */
    d2gsHotkeySelect: (slot) => {
      const h = hotkeys[slot];
      if (!h || h.skill < 0) return null;
      return protocol.d2gsSelectSkill(h.skill, h.left, h.owner);
    },
    // Local assign of one hotkey slot, the 1.13c assign path's table write: the slot takes the skill, hand
    // and owner; any other slot already holding that same skill and owner is cleared. No packet is sent.
    // The skill is bound-checked against maxSkill as d2gsApplyHotkey does. Unbound skill stores -1.
    d2gsSetHotkey: (slot, skill, left, owner = 0xffffffff, maxSkill = host.maxSkill ?? 0xfff) => {
      if (!Number.isInteger(slot) || slot < 0 || slot >= hotkeys.length) throw new RangeError('hotkey slot out of range');
      if (!Number.isInteger(skill) || skill < -1 || skill > 0xfff) throw new RangeError('invalid hotkey skill');
      if (typeof left !== 'boolean') throw new RangeError('hotkey hand must be a boolean');
      const bound = skill < 0 || skill > maxSkill ? -1 : skill;
      const own = owner >>> 0;
      if (bound >= 0) hotkeys.forEach((h, i) => { if (i !== slot && h.skill === bound && h.owner === own) hotkeys[i] = { skill: -1, left: false, owner: 0xffffffff }; });
      hotkeys[slot] = { skill: bound, left, owner: own };
      return true;
    },
    d2gsMoveTo: (opcode, x, y) => {
      if (!Number.isInteger(x) || !Number.isInteger(y) || x < 0 || y < 0 || x > 0xffff || y > 0xffff)
        throw new RangeError('D2GS walk destination must fit unsigned 16-bit subtiles');
      const n = call('d2_realm_d2gs_move_to', 'number', ['number', 'number', 'number', 'bigint'], [opcode, x, y, BigInt(T)]);
      if (n !== 5) throw new Error(`D2GS movement composer rejected opcode ${opcode}`);
      return Uint8Array.from(rd(T, n));
    },
    d2gsRightSkillAt: (held, x, y) => {
      if (!Number.isInteger(x) || !Number.isInteger(y) || x < 0 || y < 0 || x > 0xffff || y > 0xffff)
        throw new RangeError('D2GS skill destination must fit unsigned 16-bit subtiles');
      const opcode = held ? 0x0f : 0x0c;
      const n = call('d2_realm_d2gs_right_skill_at', 'number', ['number', 'number', 'number', 'bigint'], [opcode, x, y, BigInt(T)]);
      if (n !== 5) throw new Error(`D2GS skill composer rejected opcode ${opcode}`);
      return Uint8Array.from(rd(T, n));
    },
    d2gsRightSkillOnEntity: (held, unitType, guid) => {
      if (unitType !== 1 || !Number.isInteger(guid) || guid < 0 || guid > 0xffffffff)
        throw new RangeError('D2GS unit-target skill requires a monster GUID');
      const opcode = held ? 0x10 : 0x0d;
      const n = call('d2_realm_d2gs_right_skill_on_entity', 'number',
        ['number', 'number', 'number', 'bigint'], [opcode, unitType, guid, BigInt(T)]);
      if (n !== 9) throw new Error(`D2GS unit-target skill composer rejected opcode ${opcode}`);
      return Uint8Array.from(rd(T, n));
    },
    d2gsDecompress: (bytes) => { Module.HEAPU8.set(bytes, Number(P)); const n = call('d2_realm_huffman_decompress', 'number', ['bigint', 'number', 'bigint', 'number'], [BigInt(P), bytes.length, BigInt(O), scratch]); if (n < 0) throw new Error('huffman decompress failed'); return rd(O, n); },
    free: () => { for (const p of [P, O, T, E, SEED, PING, PN]) Module._free(p); },
  };
  return protocol;
}

/** Read one server->client D2GS frame: length header, then the Huffman-compressed body. */
export async function readD2gsFrame(reader, protocol, timeoutMs = CONNECT_TIMEOUT_MS) {
  await reader.want(2, timeoutMs);
  if (reader.cancelled) throw new Error('D2GS reader cancelled');
  // 0 means the buffer does not hold a whole frame yet (a frame can arrive split across socket messages).
  let size = protocol.d2gsFrameSize(reader.buffered());
  while (!size) {
    await reader.want(reader.buffered().length + 1, timeoutMs);
    if (reader.cancelled) throw new Error('D2GS reader cancelled');
    size = protocol.d2gsFrameSize(reader.buffered());
  }
  const frame = await reader.want(size, timeoutMs);
  if (reader.cancelled) throw new Error('D2GS reader cancelled');
  reader.skip(size);
  return protocol.d2gsDecompress(frame);
}

/**
 * One read of the unframed (`AF 00`) server stream: every whole packet buffered so far, delimited by the
 * engine's 1.13c size table (0 = the packet is not all here yet, negative = an opcode the build does not use).
 */
export async function readD2gsRawPackets(reader, protocol, timeoutMs = CONNECT_TIMEOUT_MS) {
  let have = 1;
  for (;;) {
    await reader.want(have, timeoutMs);
    if (reader.cancelled) throw new Error('D2GS reader cancelled');
    const buf = reader.buffered();
    let at = 0;
    for (;;) {
      const n = at < buf.length ? protocol.d2gsPacketSize(buf.subarray(at)) : 0;
      if (n < 0) throw new Error('D2GS stream contains an invalid 1.13c server packet');
      if (n === 0) break;
      at += n;
    }
    if (at > 0) { const out = buf.slice(0, at); reader.skip(at); return out; }
    have = buf.length + 1;
  }
}

// --- transports ---------------------------------------------------------------------------------

/**
 * The browser's only usable transport: a WebSocket whose bytes the ws-bridge copies to the realm.
 * Binary frames both ways; a byte is a byte.
 */
export function webSocketTransport(url, WebSocketImpl = globalThis.WebSocket) {
  const ws = new WebSocketImpl(url);
  ws.binaryType = 'arraybuffer';
  const dataCbs = [], closeCbs = [], pending = [];
  ws.onmessage = (e) => {
    const d = new Uint8Array(e.data);
    if (!dataCbs.length) { pending.push(d); return; }
    for (const cb of dataCbs) cb(d);
  };
  ws.onclose = () => { for (const cb of closeCbs) cb(); };
  ws.onerror = () => { for (const cb of closeCbs) cb(); };
  return {
    // A plain JS array must become bytes: `ws.send([1])` stringifies to "1" (0x31 on the wire), which
    // desyncs the frame and is exactly the sort of quiet corruption this transport cannot afford.
    send: (bytes) => { if (ws.readyState !== 1) return false; ws.send(bytes instanceof Uint8Array ? bytes : Uint8Array.from(bytes)); return true; },
    get isOpen() { return ws.readyState === 1; },
    onData: (cb) => { dataCbs.push(cb); while (pending.length) cb(pending.shift()); },
    onClose: (cb) => closeCbs.push(cb),
    close: () => { try { ws.close(); } catch { /* already closing */ } },
    ready: () => new Promise((resolve, reject) => {
      // A TCP refusal is commonly delivered as `close` without a useful `error` event. Observe all
      // terminal pre-open paths and remove these temporary listeners once the socket settles.
      const cleanup = () => {
        ws.removeEventListener('open', opened);
        ws.removeEventListener('error', failed);
        ws.removeEventListener('close', closed);
      };
      const opened = () => { cleanup(); resolve(); };
      const failed = () => { cleanup(); reject(new Error('websocket failed to open')); };
      const closed = () => { cleanup(); reject(new Error('websocket closed before opening')); };
      ws.addEventListener('open', opened);
      ws.addEventListener('error', failed);
      ws.addEventListener('close', closed);
      // Handle already-open/already-closed sockets too (for injected implementations and races).
      if (ws.readyState === 1) opened();
      else if (ws.readyState >= 2) closed();
    }),
  };
}

/**
 * The same pipe over an in-process server session -- no socket, no mock: `feed` is the engine's own
 * server (`d2_realm_*_session_feed`), so the client's bytes meet the real server behaviour. `serverSay`
 * injects what a server sends unprompted (the D2GS greeting).
 */
export function loopbackTransport(feed) {
  const dataCbs = [], closeCbs = [];
  // Buffer anything the server says before the client subscribes -- the D2GS greeting is emitted the
  // instant the socket opens, which on a real socket is before the client's own reader is attached.
  const pending = [];
  const emit = (bytes) => {
    if (!bytes || !bytes.length) return;
    const u = bytes instanceof Uint8Array ? bytes : Uint8Array.from(bytes);
    if (!dataCbs.length) { pending.push(u); return; }
    for (const cb of dataCbs) cb(u);
  };
  return {
    send: (bytes) => emit(feed(bytes instanceof Uint8Array ? bytes : Uint8Array.from(bytes))),
    onData: (cb) => { dataCbs.push(cb); while (pending.length) cb(pending.shift()); },
    onClose: (cb) => closeCbs.push(cb),
    close: () => { for (const cb of closeCbs) cb(); },
    serverSay: emit,
    ready: async () => {},
  };
}
