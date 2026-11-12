import { rightGroundWalks } from './world-click-dispatch.mjs';
import { corpseLootable, deathSendAllowed } from './realm-death.mjs';
// What the player's input sends in a joined Battle.net game. The same clicks and keys a local game
// reads become the retail 1.13c D2GS client packets (layouts: D2MOO D2PacketDef.h / PlrMsg.cpp, and the
// size table in src/core/realm/d2gs_session.cpp); nothing here moves a unit. The server walks the
// player to a target, runs the skill and answers with its own packets, and the position the screen
// shows comes back from those packets only.

export const UNIT = Object.freeze({ PLAYER: 0, MONSTER: 1, OBJECT: 2, MISSILE: 3, ITEM: 4, TILE: 5 });
export const INV_PAGE = Object.freeze({ INVENTORY: 0, TRADE: 2, CUBE: 3, STASH: 4 });

const le16 = (v) => [v & 255, (v >>> 8) & 255];
const le32 = (v) => [v & 255, (v >>> 8) & 255, (v >>> 16) & 255, (v >>> 24) & 255];
const make = (op, ...parts) => Uint8Array.from([op, ...parts.flat()]);
const at = (op, x, y) => make(op, le16(x), le16(y));
const unit = (op, type, guid) => make(op, le32(type), le32(guid));

/** Every client packet the input sends, by what it does.  The comment names the retail opcode's size. */
export const packets = Object.freeze({
  walkTo: (x, y) => at(0x01, x, y),                                 // 5
  runTo: (x, y) => at(0x03, x, y),                                  // 5
  walkToUnit: (type, guid) => unit(0x02, type, guid),               // 9
  runToUnit: (type, guid) => unit(0x04, type, guid),                // 9
  leftSkillAt: (x, y, held = false) => at(held ? 0x08 : 0x05, x, y),          // 5
  leftSkillOnUnit: (type, guid, held = false) => unit(held ? 0x09 : 0x06, type, guid), // 9
  rightSkillAt: (x, y, held = false) => at(held ? 0x0f : 0x0c, x, y),         // 5
  rightSkillOnUnit: (type, guid, held = false) => unit(held ? 0x10 : 0x0d, type, guid), // 9
  interact: (type, guid) => unit(0x13, type, guid),                 // 9
  pickUp: (type, guid, toInventory = false) => make(0x16, le32(type), le32(guid), le32(toInventory ? 1 : 0)), // 13
  dropCursor: (guid) => make(0x17, le32(guid)),                     // 5
  toGrid: (guid, x, y, page) => make(0x18, le32(guid), le32(x), le32(y), le32(page)), // 17
  fromGrid: (guid) => make(0x19, le32(guid)),                       // 5
  equip: (guid, bodyLoc) => make(0x1a, le32(guid), le32(bodyLoc)),  // 9
  swapOneForTwoHanded: (guid, bodyLoc) => make(0x1b, le32(guid), le32(bodyLoc)), // 9
  unequip: (bodyLoc) => make(0x1c, le16(bodyLoc)),                  // 3
  swapWithBody: (guid, bodyLoc) => make(0x1d, le32(guid), le32(bodyLoc)),   // 9
  swapCursorBuffer: (cursorGuid, itemGuid, x, y) => make(0x1f, le32(cursorGuid), le32(itemGuid), le32(x), le32(y)), // 17
  stackItems: (cursorGuid, itemGuid) => make(0x21, le32(cursorGuid), le32(itemGuid)),   // 9
  unstackItem: (guid) => make(0x22, le32(guid)),                    // 5
  socketItem: (cursorGuid, itemGuid) => make(0x28, le32(cursorGuid), le32(itemGuid)),   // 9
  scrollToBook: (scrollGuid, bookGuid) => make(0x29, le32(scrollGuid), le32(bookGuid)), // 9
  itemToCube: (itemGuid, cubeGuid) => make(0x2a, le32(itemGuid), le32(cubeGuid)),       // 9
  useOnItem: (targetGuid, useGuid) => make(0x27, le32(targetGuid), le32(useGuid)),      // 9: D2Client 0x95FE0 -> 0x14910 [0x27][clicked item][armed item]; D2GSPacketClt27
  useItem: (guid, x, y) => make(0x20, le32(guid), le32(x), le32(y)),        // 13
  useBeltItem: (guid, onMerc = false) => make(0x26, le32(guid), le32(onMerc ? 1 : 0), le32(0)), // 13
  npcChatStart: (type, guid) => unit(0x2f, type, guid),             // 9
  npcChatEnd: (type, guid) => unit(0x30, type, guid),               // 9
  npcAction: (action, npcGuid, itemGuid = 0) => make(0x38, le32(action), le32(npcGuid), le32(itemGuid)), // 13
  addStat: (stat, count = 1) => make(0x3a, le16((stat & 255) | (((count - 1) & 255) << 8))),   // 3
  addSkill: (skill) => make(0x3b, le16(skill)),                     // 3
  respawn: () => make(0x41),                                        // 1
  bindHotkey: (slot, skill, left, ownerGuid = 0xffffffff) => make(0x51, le32((((slot & 15) << 16) | (left ? 0x8000 : 0) | (skill & 0x7fff)) >>> 0), le32(ownerGuid)), // 9
  selectSkill: (skill, left, ownerGuid = 0xffffffff) => make(0x3c, le32(((left ? 0xffff8000 : 0) | (skill & 0x7fff)) >>> 0), le32(ownerGuid)), // 9 (the left flag is bit 15 of the short, sign-extended to a dword by movsx at 0x796F0)
  enterPortal: (portalGuid, levelId) => make(0x45, le32(portalGuid), le32(levelId)),    // 9
  waypoint: (waypointGuid, levelId) => make(0x49, le32(waypointGuid), le32(levelId)),   // 9
  swapWeapons: () => make(0x60),                                    // 1
  beltPlace: (itemGuid, slot) => make(0x23, le32(itemGuid), le32(slot)),    // 9
  beltLift: (itemGuid) => make(0x24, le32(itemGuid)),               // 5
  beltSwap: (cursorGuid, beltGuid) => make(0x25, le32(cursorGuid), le32(beltGuid)),   // 9
  dropGold: (playerGuid, amount) => make(0x50, le32(playerGuid), le32(amount)),   // 9: D2Client 0x14910's [0x50][player guid][amount]
  filmEnd: () => make(0x48),                                        // 1
});

// D2Client+0x143E0: the one outgoing-packet gate.  A packet equal to the last one sent, inside its
// opcode's window, is dropped; the last-packet store is shared by every opcode.  0x3A has no window,
// 5..0x0A and 0x0C..0x11 wait 50 ms, everything else 200 ms.
export function sendWindow(opcode) {
  const t = (opcode - 5) >>> 0;
  if (t > 0x35) return 200;
  if (t === 0x35) return 0;
  return t === 6 || t >= 13 ? 200 : 50;
}
export function createSendGate(transmit) {
  let lastTick = 0, last = null;
  return {
    /** Send `bytes` unless the gate drops it; returns true when it went out. */
    send(bytes, now) {
      const time = Number(now) >>> 0, win = sendWindow(bytes[0]);
      if (win && last && ((time - lastTick) >>> 0) < win && last.length === bytes.length && last.every((b, i) => b === bytes[i])) return false;
      last = Uint8Array.from(bytes); lastTick = time;
      return transmit(bytes) !== false;
    },
  };
}

const MONSTER_DEATH = 0, MONSTER_DEAD = 12, PLAYER_DEAD = 17;
const near = (dx, dy, w, up, down) => Math.abs(dx) <= w && dy >= -up && dy <= down;
// Objects whose use depends on what the cursor holds: the Compelling Orb wants the Khalim's Will flail,
// the Hell Forge in its open state wants the Hellforge Hammer. Without it the click sends nothing.
const COMPELLING_ORB = 404, HELL_FORGE = 376;
const CODE_QF2 = 0x20326671, CODE_HFH = 0x20686668;

/**
 * Turn the player's input into packets.
 *   transmit(bytes)      -> false when the connection is closed
 *   scene()              -> { ready, player:{x,y}, monsters:[{guid,classId,mode,x,y,anchorX,anchorY,drawn}], objects, items,
 *                             players:[{guid,x,y,anchorX,anchorY,drawn}] (the other players),
 *                             warps:[{id,x,y}] and warpAt(px,py) -> the level exit under the pointer (optional),
 *                             isNpc(classId), hoverBox(dx,dy), toScreen(wx,wy) -> {x,y} in pointer space, pointerWorld(px,py),
 *                             run, selected:{left,right}, cursorId, cursorCode() -> the held item's 4-byte code,
 *                             corpses:[{guid,owner,own,mode,x,y}], the player corpses the server has named (optional) }
 * The engine hooks below each answer the local player's own prediction of an action: 1 accepted, 0 refused
 * (the packet is not sent, as the client sends only what it could start itself), -1 or absent unknown (sent).
 *   move(x, y, run)               -> starts the local route to a spot. The game server sends the walk/run
 *                                    broadcast only to the OTHER clients, so the owner moves on its click.
 *   approachUnit(type, guid, run) -> starts the local route to a unit (falls back to move() to its spot)
 *   reach(type, guid)             -> true when the unit is close enough to use (falls back to a tile distance)
 *   actionDone()                  -> true once the local player has finished its current action: a waiting
 *                                    interaction then re-checks its reach and re-approaches if short
 *   skillPlan(hand, type, guid)   -> what a click with a hand's skill (0 left, 1 right) does. On a unit: 0 walk to
 *                                    it first and use the skill on arrival, 1 use it now. On the ground
 *                                    (type -1): 2 walk there instead (the right button with a hand-to-hand
 *                                    skill), 1 use it at the spot.
 *   skillAt(hand, x, y), skillOnUnit(hand, type, guid) -> the local start of a hand's skill
 *   refusalSound() -> the Sounds event the last refused start speaks (0 none); onRefused(event) plays it
 *   clickPlayer(guid, run)        -> the engine's click on a player unit, a live player or a corpse (D2Client 0x1AB20
 *                                    type 0, its waiting use gated on arrival by 0x2AFB0): 1 started, 0 refused, -1 no
 *                                    client unit. Its packets are read with takeSends. Without it a player click sends nothing.
 */
/** D2Client+0x29920 on an Items.txt row: true only for a plain useable row (useable byte 1, unique and quest bytes not 1). */
export const canUseItemRow = (row) => row.useable === 1 && row.unique !== 1 && row.quest !== 1;
/** The 0x81CE0 stat list: flag 2, a 12-frame timeout; the game runs 25 frames a second. */
export const ACTION_LOCK_FRAMES = 12, GAME_FRAME_MS = 40;
export function createRealmInput({ transmit, scene, now = () => 0, move = null, approachUnit = null, reach = null,
  actionDone = null, skillPlan = null, skillAt = null, skillOnUnit = null, onTalk = null, questFlags = null, leaveWait = null,
  queueUnit = null, clickPlayer = null, fireUnit = null, takeSends = null, refusalSound = null, onRefused = null }) {
  const gate = createSendGate(transmit);
  const evidence = { attempts: 0, sent: 0, last: null };
  // The inventory's item actions pass the leave wait's gate first (1.13c 0x923F0): an armed wait is cancelled and the action refused.
  const itemLocks = new Map();   // item guid -> tick the 12-frame action lock (0x81CE0) ends
  const itemGate = () => (leaveWait ? leaveWait.itemAction() : true);
  function emit(bytes, what) {
    evidence.attempts++;
    // Dead, the hero sends nothing but the restart (realm-death.mjs).
    if (!deathSendAllowed(scene?.()?.playerMode, what)) { evidence.last = { what, opcode: bytes[0], sent: false, dead: true, packet: Array.from(bytes) }; return false; }
    const sent = gate.send(bytes, now());
    if (sent) evidence.sent++;
    evidence.last = { what, opcode: bytes[0], sent, packet: Array.from(bytes) };
    return sent;
  }
  const open = () => { const s = scene(); return s?.ready ? s : null; };

  // What is under the pointer, nearest the player first: a unit, then another player, then a ground item, then an object.
  function pick(s, px, py) {
    const me = s.player, d2 = (u) => (u.x - me.x) ** 2 + (u.y - me.y) ** 2;
    const units = s.monsters.filter((m) => m.drawn && m.mode !== MONSTER_DEATH && m.mode !== MONSTER_DEAD &&
      s.hoverBox(px - m.anchorX, py - m.anchorY)).sort((a, b) => d2(a) - d2(b));
    if (units[0]) return s.isNpc(units[0].classId) ? { kind: 'npc', unit: units[0] } : { kind: 'monster', unit: units[0] };
    const lying = new Set((s.corpses ?? []).map((c) => c.guid));   // a player unit the server named a corpse is picked as a corpse below
    const other = (s.players || []).filter((p) => p.drawn && !lying.has(p.guid) && s.hoverBox(px - p.anchorX, py - p.anchorY)).sort((a, b) => d2(a) - d2(b))[0];
    if (other) return { kind: 'player', unit: other };
    const spot = (u) => { const p = s.toScreen(u.x, u.y); return near(px - p.x, py - p.y, 22, 40, 12); };
    // A dead player unit is a target only when it is the hero's own corpse or its owner lets the hero loot it
    // (1.13c D2Client 0x2AFB0; realm-death.mjs shapes the answer). Any other corpse is ground.
    const corpse = (s.corpses ?? []).filter((c) => corpseLootable(c) && c.mode === PLAYER_DEAD && spot(c)).sort((a, b) => d2(a) - d2(b))[0];
    if (corpse) return { kind: 'corpse', unit: corpse };
    const item = s.items.filter((i) => i.mode === 3 && spot(i)).sort((a, b) => d2(a) - d2(b))[0];
    if (item) return { kind: 'item', unit: item };
    const object = s.objects.filter((o) => o.selectable !== false && spot(o)).sort((a, b) => d2(a) - d2(b))[0];
    if (object) return { kind: 'object', unit: object };
    // A level exit is a tile unit: the scene's own hit test (the one a local game hovers with) names it.
    const warp = s.warpAt?.(px, py);
    if (warp && warp.id != null) return { kind: 'warp', unit: warp };
    return { kind: 'ground' };
  }
  const clamp16 = (v) => Math.max(0, Math.min(0xffff, v | 0));
  // Without the engine's reach test: five tiles each way is inside the server's talk range (fewer than 9 by
  // its size-aware distance) from every side; an item or object is used from two tiles. A player unit never
  // comes here: its click and arrival are the engine's (clickPlayer).
  const inReach = (s, type, u) => {
    if (pending?.hand != null) return plan(pending.hand, type, idOf(type, u)) !== 0;
    if (reach) { const r = reach(type, idOf(type, u)); if (r != null && r !== -1) return Boolean(r); }
    const d = Math.max(Math.abs(u.x - s.player.x), Math.abs(u.y - s.player.y));
    return d <= (type === UNIT.MONSTER ? 5 : type === UNIT.TILE ? 4 : 2);
  };
  const idOf = (type, u) => (type === UNIT.MONSTER || type === UNIT.PLAYER ? u.guid : u.id);
  // A corpse is a player unit lying in mode DEAD; the server opens it only in that mode. Any other player unit is one of the other players.
  const corpseOf = (s, guid) => (s.corpses ?? []).find((c) => c.guid === guid) ?? null;
  const find = (s, type, guid) => (type === UNIT.PLAYER ? ((s.corpses ?? []).find((c) => c.guid === guid && c.mode === PLAYER_DEAD && corpseLootable(c)) ?? (corpseOf(s, guid) ? null : (s.players || []).find((p) => p.guid === guid)))
    : type === UNIT.MONSTER ? s.monsters.find((m) => m.guid === guid && m.mode !== MONSTER_DEATH && m.mode !== MONSTER_DEAD)
    : type === UNIT.OBJECT ? s.objects.find((o) => o.id === guid) : type === UNIT.ITEM ? s.items.find((i) => i.id === guid && i.mode === 3)
    : type === UNIT.TILE ? (s.warps ?? []).find((w) => w.id === guid) : null);
  // The local start comes first: the client sends an action only when its own player could start it.
  const predicted = (result) => { evidence.localMove = result; return result !== 0; };
  const startRoute = (x, y, run) => predicted(move ? move(x, y, Boolean(run)) : null);

  // A hand's skill: started locally, then sent. Left is 0x05/0x06 (held 0x08/0x09), right 0x0C/0x0D (held 0x0F/0x10).
  const plan = (hand, type, guid) => { const r = skillPlan ? skillPlan(hand, type, guid) : null; return r == null ? -1 : r; };
  // A right-hand cast the client's own start refused speaks the class's line (D2Client+0x18F90); the client answers, the shell plays.
  const refused = (hand) => { if (hand === 1 && !held_) { const ev = refusalSound ? refusalSound() : 0; if (ev && onRefused) onRefused(ev); } return false; };
  let held_ = false;
  function skillOn(hand, guid, held) {
    held_ = held;
    if (!predicted(skillOnUnit ? skillOnUnit(hand, UNIT.MONSTER, guid) : null)) return refused(hand);
    return hand ? emit(packets.rightSkillOnUnit(UNIT.MONSTER, guid, held), 'right-skill-unit')
      : emit(packets.leftSkillOnUnit(UNIT.MONSTER, guid, held), 'left-skill-unit');
  }
  function skillGround(hand, x, y, held) {
    held_ = held;
    if (!predicted(skillAt ? skillAt(hand, x, y) : null)) return refused(hand);
    return hand ? emit(packets.rightSkillAt(x, y, held), 'right-skill-ground') : emit(packets.leftSkillAt(x, y, held), 'left-skill-ground');
  }
  // A click with a hand's skill on a unit. A hand-to-hand skill out of melee range walks to the unit and is
  // used on arrival (the waiting use is the plain packet, never the held one); with Stand Still it is used at
  // the pointer instead. Anything else is used on the unit now.
  function skillClick(s, hand, u, world, held, stand) {
    pending = null;
    if (plan(hand, UNIT.MONSTER, u.guid) !== 0) return skillOn(hand, u.guid, held);
    if (stand) return skillGround(hand, clamp16(world.x), clamp16(world.y), held);
    pending = { type: UNIT.MONSTER, guid: u.guid, hand };
    const sent = approach(s, UNIT.MONSTER, u);
    return sent === null ? false : sent;
  }

  // The interaction waiting on the walk: the click stores it, and it is used on arrival, never on the click.
  let pending = null;
  // Returns null when the local start refused the route (nothing sent), else whether it went out. A refused
  // route still leaves the interaction waiting: 2AA40 stores the pending action whether or not it sent.
  function approach(s, type, u) {
    const run = Boolean(s.run), guid = idOf(type, u);
    if (queueUnit && takeSends && pending) {
      // The engine walks to the unit, plans the walk again each frame and starts the waiting action in reach.
      const kind = pending.hand == null ? 0x13 : pending.hand ? 0x0d : 0x06;
      const started = queueUnit(kind, type, guid, run);
      if (started >= 0) { pending = null; evidence.localMove = started; return flush(s) ? true : started ? false : null; }
    }
    // The client keeps no unit for a ground item, so the engine cannot plan a walk to it (-1): the walk is to the item's spot.
    let started = approachUnit ? approachUnit(type, guid, run) : null;
    if ((started === -1 || !approachUnit) && move) started = move(clamp16(u.x), clamp16(u.y), run);
    if (!predicted(started)) return null;
    return emit(run ? packets.runToUnit(type, guid) : packets.walkToUnit(type, guid), 'approach');
  }
  // The engine's own packets: a walk it planned again, or the waiting action it started in reach.
  function flush(s) {
    let sent = false;
    for (const { op, type, guid } of takeSends()) {
      if (op === 0x02 || op === 0x04) sent = emit(op === 0x04 ? packets.runToUnit(type, guid) : packets.walkToUnit(type, guid), 'approach') || sent;
      else if (op === 0x13) { const u = find(s, type, guid); if (u) sent = use(s, type, u, true) || sent; }
      else if (op >= 0x05 && op <= 0x0b) sent = emit(packets.leftSkillOnUnit(type, guid, false), 'left-skill-unit') || sent;
      else sent = emit(packets.rightSkillOnUnit(type, guid, false), 'right-skill-unit') || sent;
    }
    return sent;
  }
  // The packet that uses a unit once the player has reached it.
  function use(s, type, u, started = false) {
    const guid = idOf(type, u);
    if (!started && fireUnit && takeSends) { fireUnit(0x13, type, guid); takeSends(); }   // the engine's own start; the packet goes out below
    if (type === UNIT.ITEM) return emit(packets.pickUp(UNIT.ITEM, guid), 'pickup');
    if (type === UNIT.PLAYER) return emit(packets.interact(UNIT.PLAYER, guid), corpseOf(s, guid) ? 'corpse' : 'trade');   // a corpse is opened; a live player: the server's answer (a trade request, or a refusal) decides
    if (type === UNIT.TILE) return emit(packets.interact(UNIT.TILE, guid), 'warp');   // the server moves the player through the exit
    if (type === UNIT.MONSTER) {
      const sent = emit(packets.interact(UNIT.MONSTER, guid), 'talk');
      if (sent && onTalk) onTalk(guid);
      return sent;
    }
    if (u.selectable !== false && u.mode === 0 && u.classId === COMPELLING_ORB && (s.cursorCode?.() >>> 0) !== CODE_QF2) return false;
    if (u.selectable !== false && u.mode === 2 && u.classId === HELL_FORGE && (s.cursorCode?.() >>> 0) !== CODE_HFH) return false;
    return emit(packets.interact(UNIT.OBJECT, guid), 'object');
  }
  // In reach, use it now; otherwise walk to it and keep it waiting.
  function arrive(s, reapproach) {
    const u = find(s, pending.type, pending.guid);
    if (!u) { pending = null; return null; }
    if (inReach(s, pending.type, u)) {
      const { type, hand } = pending; pending = null;
      return hand != null ? skillOn(hand, u.guid, false) : use(s, type, u);
    }
    if (!reapproach) return null;
    const sent = approach(s, pending.type, u);
    return sent === null ? false : sent;
  }
  function interactWith(s, type, u) {
    pending = { type, guid: idOf(type, u) };
    return arrive(s, true);
  }

  return {
    evidence,
    get pending() { return pending ? { ...pending } : null; },
    pick: (px, py) => { const s = open(); return s ? pick(s, px, py) : null; },
    /** The left button on the world: act on what is under it, otherwise walk or run there. */
    leftClick(px, py, { held = false, stand = false } = {}) {
      const s = open(); if (!s) return null;
      const hit = pick(s, px, py);
      const world = s.pointerWorld(px, py);
      if (hit.kind === 'monster') return skillClick(s, 0, hit.unit, world, held, stand) && hit;
      // Stand Still held: the left skill is used at the spot under the pointer and the player does not walk.
      if (stand) {
        pending = null;
        return skillGround(0, clamp16(world.x), clamp16(world.y), held) && hit;
      }
      if (held && hit.kind !== 'ground') return hit;               // a held button does not repeat a pickup or a talk
      pending = null;
      // A player unit, live or a corpse: the engine's click (1.13c D2Client 0x1AB20 type 0) decides the use or the
      // walk, and the arrival gate (0x2AFB0) decides the waiting use; the page sends what the engine started.
      if (hit.kind === 'player' || hit.kind === 'corpse') {
        if (!clickPlayer || !takeSends) return false;
        const started = clickPlayer(hit.unit.guid, Boolean(s.run));
        evidence.localMove = started;
        return flush(s) ? hit : false;
      }
      const type = hit.kind === 'npc' ? UNIT.MONSTER : hit.kind === 'item' ? UNIT.ITEM : hit.kind === 'object' ? UNIT.OBJECT : hit.kind === 'warp' ? UNIT.TILE : null;
      if (type != null) return interactWith(s, type, hit.unit) === false ? false : hit;
      const x = clamp16(world.x), y = clamp16(world.y);
      if (!startRoute(x, y, s.run)) return false;
      return emit(s.run ? packets.runTo(x, y) : packets.walkTo(x, y), s.run ? 'run' : 'walk') && hit;
    },
    /** Once per frame: an interaction waiting on the walk is used once the unit is in reach. */
    tick() {
      if (takeSends) { const s = open(); if (s) flush(s); }
      if (!pending) return null;
      const s = open(); if (!s) return null;
      if (actionDone && !actionDone()) return null;
      return arrive(s, Boolean(actionDone));
    },
    /** Arrow keys: walk (or run) a step offset from where the player stands. */
    step(dx, dy) {
      const s = open(); if (!s) return null;
      pending = null;
      const x = clamp16(s.player.x + dx), y = clamp16(s.player.y + dy);
      if (!startRoute(x, y, s.run)) return false;
      return emit(s.run ? packets.runTo(x, y) : packets.walkTo(x, y), s.run ? 'run' : 'walk');
    },
    /** The interact key: talk to the nearest person, else operate the nearest object, within 7 tiles. */
    interactNearest() {
      const s = open(); if (!s) return null;
      const d = (u) => Math.max(Math.abs(u.x - s.player.x), Math.abs(u.y - s.player.y));
      const npc = s.monsters.filter((m) => s.isNpc(m.classId) && m.mode !== MONSTER_DEATH && m.mode !== MONSTER_DEAD && d(m) <= 7)
        .sort((a, b) => d(a) - d(b) || a.guid - b.guid)[0];
      if (npc) return interactWith(s, UNIT.MONSTER, npc) !== false;
      const object = s.objects.filter((o) => o.selectable !== false && d(o) <= 7).sort((a, b) => d(a) - d(b) || a.id - b.id)[0];
      if (!object) return false;
      return interactWith(s, UNIT.OBJECT, object) !== false;
    },
    /** The right button on the world: the right-hand skill at the unit under it, otherwise at the ground. */
    rightClick(px, py, { held = false, stand = false } = {}) {
      const s = open(); if (!s) return null;
      const hit = pick(s, px, py), world = s.pointerWorld(px, py);
      if (hit.kind === 'monster' || hit.kind === 'npc') return skillClick(s, 1, hit.unit, world, held, stand) && hit;
      pending = null;
      const x = clamp16(world.x), y = clamp16(world.y);
      // A hand-to-hand right skill on open ground walks there, unless Stand Still is held.
      if (rightGroundWalks(plan(1, -1, 0), stand)) {
        if (!startRoute(x, y, s.run)) return false;
        return emit(s.run ? packets.runTo(x, y) : packets.walkTo(x, y), s.run ? 'run' : 'walk') && hit;
      }
      return skillGround(1, x, y, held) && hit;
    },
    /**
     * The server's answer to a talk (its 0x28 packet: type, guid, and a third field): the client opens the
     * conversation with 0x2F for that unit, or, when it no longer knows the unit, ends it with 0x30.
     */
    npcInteraction(bytes) {
      const type = bytes[1], guid = (bytes[2] | (bytes[3] << 8) | (bytes[4] << 16) | (bytes[5] << 24)) >>> 0, third = bytes[6];
      // The 96 quest-flag bytes at +7 replace the client's quest record: always for type 6, otherwise only when the unit is known.
      const flags = () => { if (questFlags && bytes.length >= 103) questFlags(bytes.subarray(7, 103)); };
      if (type === 6) { flags(); return null; }
      const s = open();
      if (!s || !find(s, type, guid)) { pending = null; return emit(make(0x30, le32(third), le32(guid)), 'npc-end'); }
      flags();
      return emit(packets.npcChatStart(type, guid), 'npc-start');
    },
    /** Belt key 1-4: the item in belt slot `column` exactly (the key handler passes the column as the slot). Nothing happens
     *  while the player is dead (mode 17), when the cell is empty, or while the cursor holds an item. The retail use routine
     *  (D2Client+0x29990) then refuses, in order: an item row that is not a plain useable one (0x29920: Items.txt unique byte
     *  +0x129 or quest byte +0x12A equal to 1 opens the reading panel instead, useable byte +0x11D not 1 plays the refusal
     *  voice; the scene's `itemRow(classId)` answers, a missing row is fatal in retail), and an item still action-locked
     *  (0x81CB0: a stat list with flag 2, STATLIST_NEWLENGTH, the 12-frame timer 0x81CE0 attaches after every sent
     *  use; the lock below) or a hero in state 0x36 (the scene's optional `heroLocked()`). The game menu (flag 0xFADA4,
     *  the Esc menu panel) never gets here: the shell drains input first. */
    useBelt(column, onMerc = false) {
      const s = open(); if (!s) return false;
      if (s.playerMode === PLAYER_DEAD || s.cursorId) return false;
      const slot = s.items.find((i) => i.mode === 2 && i.gridX === column);
      if (!slot) return false;
      if (s.itemRow) {
        const row = s.itemRow(slot.classId);
        if (!row || !canUseItemRow(row)) return false;
      }
      const t = now();
      if ((itemLocks.get(slot.id) ?? -1) > t || s.heroLocked?.()) return false;
      const sent = emit(packets.useBeltItem(slot.id, onMerc), 'belt');
      if (sent) itemLocks.set(slot.id, t + ACTION_LOCK_FRAMES * GAME_FRAME_MS);   // 0x81CE0 after the 0x26 went out
      return sent;
    },
    /** The skill popup / hotkey choice. */
    selectSkill: (skill, left, owner = 0xffffffff) => emit(packets.selectSkill(skill, left, owner), 'select-skill'),
    /** The skill popup's F-key assign: slot (0-15) takes the skill in that hand; the server binds it (0x51). */
    bindHotkey: (slot, skill, left, owner = 0xffffffff) => (Number.isInteger(slot) && slot >= 0 && slot < 16 && Number.isInteger(skill) && skill >= 0 && skill <= 0x7fff
      ? emit(packets.bindHotkey(slot, skill, Boolean(left), owner), 'bind-hotkey') : false),
    /** Retail's popup assign: unbind the slot's old binding, bind the new one, then clear every other slot holding the same skill and owner. `table` is the hotkey table before the assign. */
    assignHotkey: (slot, skill, left, owner = 0xffffffff, table = []) => {
      if (!Number.isInteger(slot) || slot < 0 || slot >= 16 || !Number.isInteger(skill) || skill < 0 || skill > 0x7fff) return false;
      if (table[slot] && table[slot].skill >= 0) emit(packets.bindHotkey(slot, 0x7fff, true, 0xffffffff), 'unbind-hotkey');
      const sent = emit(packets.bindHotkey(slot, skill, Boolean(left), owner), 'bind-hotkey');
      for (let i = 0; i < 16; i++) if (i !== slot && table[i] && table[i].skill === skill && table[i].owner === (owner >>> 0)) emit(packets.bindHotkey(i, 0x7fff, true, 0xffffffff), 'unbind-hotkey');
      return sent;
    },
    /** An F-key's select-skill packet, composed from the server's hotkey table (d2gsHotkeySelect); null when the key is unbound. */
    hotkey: (bytes) => (bytes ? emit(bytes, 'hotkey-select') : false),
    /** The character screen's +: `count` points (1..100) into a stat, one packet; the server spends them or refuses. */
    addStat: (stat, count = 1) => (Number.isInteger(stat) && stat >= 0 && stat < 16 && count >= 1 && count <= 100
      ? emit(packets.addStat(stat, count), 'add-stat') : false),
    /** The skill tree's +: one point into a skill. */
    addSkill: (skill) => (Number.isInteger(skill) && skill >= 0 && skill <= 0xffff ? emit(packets.addSkill(skill), 'add-skill') : false),
    /** The death screen's continue. */
    respawn: () => emit(packets.respawn(), 'respawn'),
    /** The hero fell: the interaction waiting on a walk is dropped (1.13c D2Client 0x16A0). */
    heroDied() { pending = null; },
    useItem(guid) { const s = open(); return s && itemGate() ? emit(packets.useItem(guid, s.player.x, s.player.y), 'use-item') : false; },
    /** Inventory panel: lift a carried item (0x19) or a worn one (0x1C). */
    lift: (guid) => itemGate() && emit(packets.fromGrid(guid), 'lift'),
    unequip: (bodyLoc) => itemGate() && emit(packets.unequip(bodyLoc), 'unequip'),
    /** With an item on the cursor: into a grid cell (0x18), onto an empty body slot (0x1A), swapping a worn one (0x1D). */
    putInGrid: (guid, x, y, page = INV_PAGE.INVENTORY) => itemGate() && emit(packets.toGrid(guid, x, y, page), 'to-grid'),
    equip: (guid, bodyLoc) => itemGate() && emit(packets.equip(guid, bodyLoc), 'equip'),
    swapGrid: (cursorGuid, itemGuid, x, y) => itemGate() && emit(packets.swapCursorBuffer(cursorGuid, itemGuid, x, y), 'swap-grid'),
    /** With an item held, dropped on one carried item it combines with: stack (0x21), socket (0x28), scroll into tome (0x29), item into the cube (0x2A); shift+click on a stack takes one off (0x22). */
    combine: (action, cursorGuid, itemGuid) => { const b = { stack: packets.stackItems, socket: packets.socketItem, scroll: packets.scrollToBook, cube: packets.itemToCube }[action];
      return Boolean(b) && itemGate() && emit(b(cursorGuid, itemGuid), `combine-${action}`); },
    unstack: (guid) => itemGate() && emit(packets.unstackItem(guid), 'unstack'),
    /** The identify cursor's press on an item (0x27): the item under the click, then the armed scroll or tome; the same id twice cancels. */
    useOnItem: (targetGuid, useGuid) => itemGate() && emit(packets.useOnItem(targetGuid, useGuid), 'use-on-item'),
    swapBody: (guid, bodyLoc) => itemGate() && emit(packets.swapWithBody(guid, bodyLoc), 'swap-body'),
    /** A click on a ground item by id: walk to it and pick it up (0x16) once in reach, as leftClick does for the item under the pointer. */
    pickUp(guid) { const s = open(), u = s?.items.find((i) => i.id === guid && i.mode === 3); return Boolean(u) && itemGate() && interactWith(s, UNIT.ITEM, u) !== false; },
    dropCursor: (guid) => itemGate() && emit(packets.dropCursor(guid), 'drop'),
    swapWeapons: () => emit(packets.swapWeapons(), 'swap-weapons'),
    /** The belt click's lift, place and swap (the same bytes the local engine is handed); `use` is useBelt. */
    beltClick(act) {
      if (act.op === 'lift') return emit(packets.beltLift(act.a), 'belt-lift');
      if (act.op === 'place') return emit(packets.beltPlace(act.a, act.b), 'belt-place');
      if (act.op === 'swap') return emit(packets.beltSwap(act.a, act.b), 'belt-swap');
      return false;
    },
    /** The drop-gold box's OK: nothing for 0, else the 0x50 with the hero's unit id. */
    dropGold: (playerGuid, amount) => (amount > 0 ? emit(packets.dropGold(playerGuid >>> 0, amount >>> 0), 'drop-gold') : false),
    /** The film the server asked for (0x61) is over or was not shown: the server's busy mark for the player is lifted (1.13c D2Client 0x899A0 ends with the 1-byte 0x48). */
    filmEnd: () => emit(packets.filmEnd(), 'film-end'),
    /** After the server has opened a menu: the waypoint list and a town-portal choice. */
    waypoint: (guid, levelId) => emit(packets.waypoint(guid, levelId), 'waypoint'),
    enterPortal: (guid, levelId) => emit(packets.enterPortal(guid, levelId), 'portal'),
    npcChatEnd: (guid) => emit(packets.npcChatEnd(UNIT.MONSTER, guid), 'npc-end'),
  };
}
