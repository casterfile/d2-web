// The menu retail shows when a town NPC is clicked: the NPC's name, then Talk / Trade / Gamble / Hire /
// ..., then Cancel. Which entries an NPC has is retail's own table, not ours:
//
//   D2Client 1.13c .data, file offset 0xF3BA0: 48 records of d2bs's `NPCMenu` (reference/d2bs
//   D2Structs.h -- dwNPCClassId, dwEntryAmount, wEntryId[4], pad, dwEntryFunc[4], 5 bytes), found by
//   the string ids it holds. dwEntryAmount counts the Cancel every menu ends with (string 3400).
//   The handlers the entries point at name what they do: 0x6FAFA2F0 talk, 0x6FAFAEE0 trade (also
//   behind 3334 Trade/Repair), 0x6FAFABE0 gamble, 0x6FAF9440 hire, 0x6FAF8750 Warriv's Go West (3383),
//   0x6FAF87B0 Meshif's Sail West (3385), 0x6FAF6AF0 the act 5 entry 4020.
//
// Layout from retail's own DRAW LIST (the retail captures: Warriv's
// menu, his unit anchored at (304, 292)). Retail draws, in order:
//   1. a box (D2GFX DrawBox, colour 0, DrawMode 1 = 50%) over rect (x1,y1)-(x2,y2): x2 - x1 = widest
//      line's width (.tbl advances) + 20, centred on the NPC's x; y2 = NPC y - 148 + 10 * lines;
//      y1 = y2 - (15 * lines + 21);
//   2. ui\menu\boxpieces.dc6 around it (menuFramePieces below); the count grows with the box.
//   3. the lines through D2Win's text draw in font16, anchored (bottom row) at y1 + 21, then every
//      15px, each from the NPC's x - width / 2 + 1: the name colour 4 (gold), the entries colour 0 (white).
// The blue entry is the SELECTED one: the first on opening (retail/frames/s3-s4), then whichever the
// pointer is over, none when it is elsewhere (the draw list, pointer parked at 799,300).
// TODO(client-parity F18): Warriv's 3-line and Hratli's 4-line retail lists establish this rule.
// Jerhyn's committed 2-line retail list is awaiting its Act II branch's merge onto main, when s221
// will take it as the third size rather than extrapolating from two examples.
import { createFont, decodeFontTbl, FRONTEND } from './frontend.mjs';
import { recordDc6, recordPrim } from './drawlist.mjs';
import { questOfMessage } from './npc-topics.mjs';
import { hudOrigin } from './hud-origin.mjs';

/** Scroll thumb's pixel drop down the hire window's track (149..388) for `top` of `n` offers (7 rows show). */
export function hireThumbOffset(top, n, thumb, rows = 7) {
  const maxTop = n - rows;
  return maxTop > 0 ? Math.round((388 - 149 - 12 * (thumb - 1)) * Math.min(top, maxTop) / maxTop) : 0;
}
const TALK = 'talk', TRADE = 'trade', GAMBLE = 'gamble', HIRE = 'hire', IDENTIFY = 'identify', OTHER = 'other';
const S_INTRO = 3399, S_GOSSIP = 3395, S_TALK = 3381, S_TRADE = 3396, S_TRADE_REPAIR = 3334, S_GAMBLE = 3398, S_HIRE = 3397, S_IDENTIFY = 4020, S_CANCEL = 3400;
const ACTION = { [S_TALK]: TALK, [S_TRADE]: TRADE, [S_TRADE_REPAIR]: TRADE, [S_GAMBLE]: GAMBLE, [S_HIRE]: HIRE, [S_IDENTIFY]: IDENTIFY };

// [NPC class, entry string ids] in the table's order (the Cancel each ends with is implied).
export const NPC_MENU_TABLE = [
  [148, [S_TALK, S_TRADE]], [176, [S_TALK]], [146, [S_TALK]], [154, [S_TALK, S_TRADE_REPAIR]],
  [177, [S_TALK, S_TRADE]], [199, [S_TALK, S_TRADE, S_GAMBLE]], [200, [S_TALK]], [147, [S_TALK, S_TRADE, S_GAMBLE]],
  [198, [S_TALK, S_HIRE]], [201, [S_TALK]], [202, [S_TALK, S_TRADE]], [150, [S_TALK]],
  [178, [S_TALK, S_TRADE_REPAIR]], [155, [S_TALK]], [175, [S_TALK, 3383]], [210, [S_TALK]],
  [244, [S_TALK, S_IDENTIFY]], [265, [S_TALK, S_IDENTIFY]], [245, [S_TALK, S_IDENTIFY]], [246, [S_TALK, S_IDENTIFY]],
  [251, [S_TALK]], [367, [S_TALK]], [257, [S_TRADE_REPAIR]], [405, [S_TRADE, S_GAMBLE]],
  [252, [S_TALK, S_HIRE, S_TRADE]], [253, [S_TALK, S_TRADE_REPAIR]], [254, [S_TALK, S_TRADE, S_GAMBLE]], [255, [S_TALK, S_TRADE]],
  [406, [S_TALK]], [257, [S_TALK]], [264, [S_TALK, 3385]], [297, [S_TALK]],
  [266, [S_TALK]], [331, [S_TALK]], [408, [S_TALK]], [378, [S_TALK]],
  [377, [S_TALK]], [521, [S_TALK]], [520, [S_TALK, S_IDENTIFY]], [511, [S_TALK, S_TRADE_REPAIR]],
  [513, [S_TALK, S_TRADE]], [512, [S_TALK, S_TRADE, S_GAMBLE]], [514, [S_TALK, S_GAMBLE]], [515, [S_TALK]],
  [527, [S_TALK]], [537, [S_TALK]], [538, [S_TALK]], [539, [S_TALK]],
];

/** After a speech the menu comes back only for the NPC it was opened on and one with entries (D2Client's menu state), else Esc ends the interaction. */
// Retail keeps one menu-open flag (D2Client .data 0x6FBC9663, set by 0x4A270, cleared by the 0x4A3D0 teardown): the menu comes back
// only while the NPC unit is still in the room (`alive`), else the interaction is over and the pending menu is dropped.
export const menuReturnsAfterSpeech = (pending, npcId, alive = true) => !!pending && alive && pending.npcId === npcId && Array.isArray(pending.entries) && pending.entries.length > 0;

/** The menu that returns after speech `spoken`: a quest topic just heard is gone from the engine's list (0x31 consumed it), so its row is not offered again. */
export const entriesAfterSpeech = (entries, spoken) => entries.filter((e) => !(e.action === 'topic' && e.stringId === spoken));

/** The client's speech-end rule: the first message of the list other than the one just spoken, else null (then the Talk box reopens). */
export const nextSpeechAfterTopic = (messages, spoken) => { const m = messages.find((x) => x.stringId !== spoken); return m ? m.stringId : null; };

/** The retail entries for an NPC class (first matching record, as D2Client walks the table), or null. */
// P7b: the Talk box (kashya-talk-r14: talk / introduction / gossip / Den of Evil / cancel). Introduction and Gossip are always there;
// then one topic per menu-2 message of the engine's 0x27 list (D2Text nMenu 2), labelled with its quest's own name (npc-topics.mjs).
// `messages` = [{ stringId, menu }] from the engine; `questName(act, n)` = the quest's name string.
export function talkEntries(messages, questName) {
  const seen = new Set(), topics = [];
  for (const m of messages) {
    if (m.menu !== 2) continue;
    const q = questOfMessage(m.stringId), label = q ? questName(q.act, q.quest) : '';
    if (!label || seen.has(label)) continue;
    seen.add(label); topics.push({ stringId: m.stringId, label, action: 'topic' });
  }
  return [{ stringId: S_INTRO, action: 'intro' }, { stringId: S_GOSSIP, action: 'gossip' }, ...topics, { stringId: S_CANCEL, action: 'cancel' }];
}
/**
 * The travel row the table cannot hold. D2Client 1.13c's menu open (0x4A4F0, through 0x45CC0) walks a quest-conditioned table
 * (.data 0x6FBA2330): Warriv (Act I) gets Go East (3382) when bit 0 of quest word 6 (A1Q6) is set, Meshif (Act II) Sail East (3384)
 * on word 14 (A2Q6), Tyrael (Act IV) his Harrogath row (22738) on word 28 (the Act IV completed word) in an expansion game only
 * (0x49060 / 0x49030 / 0x49000). The test is the exact bit (Ordinal 10174). Warriv (Act II, 3383) and Meshif (Act III, 3385) carry
 * theirs in the table. `questBit(word)` answers bit 0 of the hero's quest word.
 */
export const TRAVEL_ROWS = { 155: { stringId: 3382, word: 6 }, 210: { stringId: 3384, word: 14 }, 367: { stringId: 22738, word: 28, expansion: true } };
/** The level each travel row's handler puts in its 0x38 {0, npc, level}: 0x488D0 40, 0x48750 1, 0x48870 75, 0x487B0 40, 0x48810 109. */
export const TRAVEL_LEVEL = { 3382: 40, 3383: 1, 3384: 75, 3385: 40, 22738: 109 };
export function withTravelRow(entries, classId, questBit, expansion = true) {
  const row = TRAVEL_ROWS[classId];
  if (!entries || !row || (row.expansion && !expansion) || !questBit(row.word)) return entries;
  return [...entries.slice(0, -1), { stringId: row.stringId, action: OTHER }, entries[entries.length - 1]];
}
/**
 * Hire is a dynamic row for the two mercenary NPCs whose table record is Talk alone: Kashya lists it once Sisters' Burial Grounds (A1Q2)
 * is done, Qual-Kehk once Siege on Harrogath (A5Q1) is. The other hirers (Greiz, Asheara) have it in the table.
 */
export const HIRE_ROWS = { 150: [1, 2], 515: [5, 1] };
export function withHireRow(entries, classId, questDone) {
  const row = HIRE_ROWS[classId];
  if (!entries || !row || !questDone(row[0], row[1]) || entries.some((e) => e.action === HIRE)) return entries;
  return [entries[0], { stringId: S_HIRE, action: HIRE }, ...entries.slice(1)];
}
/** Picking Hire while the hero's mercenary is dead raises it (retail's resurrect, packet 0x62 {npc id}) instead of opening the offers. Pet flag bit 0 = dead. */
export const deadHireling = (roster) => roster.find((h) => h && (h.flags & 1)) ?? null;
/**
 * The Resurrect row (string 0x1507, D2Client+0x4A060): while the server's 0x9B names a dead mercenary (name id not 0xFFFF) every
 * mercenary NPC's menu lists it ahead of the Hire row, or ahead of Cancel when there is no Hire row. D2Client+0x48B20 builds its
 * text as sprintf(string 0x58A8, the mercenary's name string, the price); `label` is that text, made by the caller.
 */
export const S_RESURRECT = 5383, S_RESURRECT_FORMAT = 22696;
export const RESURRECT_CLASSES = [252, 198, 515, 367, 150];
export function withResurrectRow(entries, classId, label) {
  if (!entries || label === null || !RESURRECT_CLASSES.includes(classId) || entries.some((e) => e.action === 'resurrect')) return entries;
  const row = { stringId: S_RESURRECT, action: 'resurrect', label };
  const at = entries.findIndex((e) => e.action === HIRE);
  return at >= 0 ? [...entries.slice(0, at), row, ...entries.slice(at)] : [...entries.slice(0, -1), row, entries[entries.length - 1]];
}
/** The 0x58A8 text: the format's two conversions take the name string and the price in order. */
export const resurrectLabel = (format, name, cost) => format.replace('%s', name).replace('%d', String(cost));
export const resurrectPacket = (npcId) => new Uint8Array([0x62, npcId & 255, (npcId >>> 8) & 255, (npcId >>> 16) & 255, (npcId >>> 24) & 255]);
export function npcMenuEntries(classId) {
  const row = NPC_MENU_TABLE.find(([cls]) => cls === classId);
  if (!row) return null;
  return [...row[1].map((stringId) => ({ stringId, action: ACTION[stringId] ?? OTHER })), { stringId: S_CANCEL, action: 'cancel' }];
}

/** Retail invokes an NPC menu callback on release over the row currently selected by the menu. */
export function npcMenuChoiceOnRelease({ entryAtRelease, selectedEntry }) {
  // draw() rebuilds the row objects every frame, so a redraw between press and release must not break the match: compare the row, not the object.
  const same = entryAtRelease && selectedEntry && (entryAtRelease === selectedEntry ||
    (entryAtRelease.action === selectedEntry.action && entryAtRelease.stringId === selectedEntry.stringId));
  if (!same || !entryAtRelease.action) return null;
  return entryAtRelease;
}

/** Literal 1.13c DWORD deadline test: uint32(now) < uint32(creationTick + 80). */
export function npcMenuInputCoolingDown({ nowTick, creationTick }) {
  const now = Math.trunc(nowTick) >>> 0;
  const created = Math.trunc(creationTick) >>> 0;
  return now < ((created + 80) >>> 0);
}

export const NPC_MENU_ASSETS = { palette: FRONTEND.palette, font: 'data\\local\\font\\latin\\font16.dc6', fontTbl: 'data\\local\\font\\latin\\font16.tbl',
  pieces: 'data\\global\\ui\\menu\\boxpieces.dc6' };
export const NPC_MENU_LAYOUT = Object.freeze({ padX: 20, firstLine: 21, pitch: 15, bottomBaseAboveFeet: 148, bottomPerLine: 10 });

/**
 * The boxpieces cels retail draws around a menu rect, in its order: [frame, anchor x, anchor y]
 * (anchor = bottom row, inclusive). Retail lists establish these cycles for the 3-line Warriv and
 * 4-line Hratli boxes: top frames 2..7 begin at 3; bottom frames 16..21 begin after the top's
 * extra pieces; left frames 12,10,11 repeat; right frames 13..15 advance for each extra left piece.
 * Each edge starts inside the rect, followed by corners 0,1,8,9.
 */
export function menuFramePieces(x1, y1, x2, y2) {
  const out = [];
  // Pieces per horizontal edge: retail's lists give 4 for w 69 (Jerhyn) and 73 (Kashya), 5 for 78 (Warriv),
  // 8 for 119 (Kashya's Talk box) -- floor((w - 14) / 12); 125 (Cain) gives 9.
  const n = Math.floor((x2 - x1 - 14) / 12), top = [];
  for (let i = 0; i < n; i++) top.push([2 + ((i + 1) % 6), x1 + 12 + 12 * i, y1 + 12]);
  out.push(...top);
  for (let i = 0, x = x1 + 13; x < x2 - 12; i++, x += 12) out.push([16 + ((i + top.length + 1) % 6), x, y2 + 10]);
  // The vertical strips continue the edge cycle: the left strip starts at 10 + (top + bottom count + 1) % 3
  // and runs 10,11,12 down; the right strip carries on from where the left ended (13..15). Fitted to all
  // five retail boxes (Warriv, Hratli, Jerhyn's 69px, Cain with and without Identify) -- the earlier
  // "narrow" special case was this rule's phase for a 4-piece top edge.
  const leftStart = (2 * top.length + 1) % 3;
  let leftCount = 0;
  for (let i = 0, y = y1 + 22; y < y2; i++, y += 12) { out.push([10 + ((i + leftStart) % 3), x1 - 4, y]); leftCount++; }
  for (let i = 0, y = y1 + 24; y < y2; i++, y += 12) out.push([13 + ((i + leftStart + leftCount) % 3), x2 - 7, y]);
  out.push([0, x1, y1 + 12], [1, x2 - 12, y1 + 12], [8, x1, y2 + 1], [9, x2 - 12, y2 + 1]);
  return out;
}
const GOLD = [0xc7, 0xb3, 0x77], BLUE = [0x69, 0x69, 0xff], RED = [0xff, 0x4d, 0x4d];

/** A hire offer's first line, built the way retail joins it: name, space, dash, space, then Lvl / Life / Def / Cost
 *  (string.tbl 3368, 3366, 3367, 3369; each label cut to 20 chars) as `label`, colon, space, a 2-wide value,
 *  two spaces between fields. The space, dash and colon are string.tbl 3995, 3996 and 3997. */
export function hireOfferLine(o, string) {
  const t = (id, d) => string(id) || d;
  const sp = t(3995, ' '), dash = t(3996, '-'), colon = t(3997, ':');
  const field = (id, d, v) => `${t(id, d).slice(0, 20)}${colon}${sp}${String(v).padStart(2)}`;
  return [`${o.name}${sp}${dash}${sp}${field(3368, 'Lvl', o.level)}`, field(3366, 'Life', o.life), field(3367, 'Def', o.def), field(3369, 'Cost', o.cost)].join(sp + sp);
}
/** The skill line of a hire offer: the engine's description-id table, indexed by the offer's record field (0..8; anything else, and slot 0, has none). */
export const HIRE_SKILL_STRINGS = [0, 3371, 3372, 11090, 11088, 11089, 3377, 3376, 3375];
export function hireSkillText(index, string) {
  const id = Number.isInteger(index) && index >= 0 && index < HIRE_SKILL_STRINGS.length ? HIRE_SKILL_STRINGS[index] : 0;
  return id ? string(id) : '';
}
/** The hire window's box in a screen of the given size: 490 x 350, centred horizontally, and vertically about the middle of the
 *  screen minus its 40 px bottom panel (so 85..435 at 800x600). The offers list sits 35 px inside it, 280 high. */
export function hireWindowBox(width = 800, height = 600) {
  const x1 = (width - 490) >> 1, y1 = ((height - 40) >> 1) - 195;
  return { x1, y1, x2: x1 + 490, y2: y1 + 350, listY: ((height - 40) >> 1) - 160 };
}
export function createNpcMenu({ read, decodeDc6, decodeDatPalette, string, documentObject = globalThis.document }) {
  const palette = decodeDatPalette(read(NPC_MENU_ASSETS.palette));
  const dc6 = decodeDc6(read(NPC_MENU_ASSETS.font)), tbl = decodeFontTbl(read(NPC_MENU_ASSETS.fontTbl));
  const tint = (rgb) => palette.map((v, i) => (i % 4 === 3 ? v : Math.round(v * rgb[i % 4] / 0xff)));
  const fontPath = NPC_MENU_ASSETS.font, glyphs = { op: 'celcolor', mode: 5 };
  const white = createFont({ dc6, tbl, palette, documentObject, path: fontPath, drawAs: glyphs });
  // A coloured line is its glyphs' indices through the act PL2's TextColorShifts[colour] (retail's own
  // Shifted indices address that SAME act palette, not Units (s2198: A2/A3 names
  // had206/30 wrong pixels when those indices were read through Units).
  // 8-bit frame of this menu: Warriv's name, colour 4, 191 of 191 glyph pixels; an earlier change). Offsets per
  // OpenDiablo2's d2pl2 layout; gold is colour 4, blue colour 3.
  const TEXT_SHIFTS = 1024 + 8192 + 4096 + 256 + 5 * 65536 + 111 * 256 + 768 + 14 * 256 + 65536 + 256 + 39;
  const shifted = (colour, fallback, act = 1) => {
    try { const pl2 = read(`data\\global\\palette\\ACT${Math.min(5, Math.max(1, act | 0))}\\pal.pl2`), out = palette.slice();
      for (let k = 0; k < 256; k++) { const j = pl2[TEXT_SHIFTS + colour * 256 + k]; out.set(pl2.subarray(j * 4, j * 4 + 3), k * 4); }
      return out; } catch { return tint(fallback); } };
  // Coloured fonts per act: the shift table is the CURRENT act's PL2 (Codex review: Act I's was read in
  // every act, so Lut Gholein's and Harrogath's menus shifted through the wrong table).
  const coloured = new Map();
  const fontsFor = (act) => { if (!coloured.has(act)) coloured.set(act, {
    gold: createFont({ dc6, tbl, palette: shifted(4, GOLD, act), documentObject, path: fontPath, drawAs: { ...glyphs, extra: 4 } }),
    blue: createFont({ dc6, tbl, palette: shifted(3, BLUE, act), documentObject, path: fontPath, drawAs: { ...glyphs, extra: 3 } }),
    red: createFont({ dc6, tbl, palette: shifted(1, RED, act), documentObject, path: fontPath, drawAs: { ...glyphs, extra: 1 } }) });
    return coloured.get(act); };
  const pieces = decodeDc6(read(NPC_MENU_ASSETS.pieces));
  const SLIDER = 'data\\global\\ui\\menu\\textslid.dc6';
  const sliders = decodeDc6(read(SLIDER)), sliderCanvas = new Map();
  const slid = (i) => {
    if (!sliderCanvas.has(i)) {
      const f = sliders.frames[i], c = documentObject.createElement('canvas'); c.width = f.width; c.height = f.height;
      const rgba = new Uint8ClampedArray(f.width * f.height * 4);
      for (let k = 0; k < f.indices.length; k++) if (f.opaque[k]) rgba.set(palette.subarray(f.indices[k] * 4, f.indices[k] * 4 + 4), k * 4);
      c.getContext('2d').putImageData(new ImageData(rgba, f.width, f.height), 0, 0); sliderCanvas.set(i, c);
    }
    return sliderCanvas.get(i);
  };
  const pieceCanvas = new Map();
  const piece = (i) => {
    if (!pieceCanvas.has(i)) {
      const f = pieces.frames[i], c = documentObject.createElement('canvas');
      c.width = f.width; c.height = f.height;
      const rgba = new Uint8ClampedArray(f.width * f.height * 4);
      for (let k = 0; k < f.indices.length; k++) if (f.opaque[k]) rgba.set(palette.subarray(f.indices[k] * 4, f.indices[k] * 4 + 4), k * 4);
      c.getContext('2d').putImageData(new ImageData(rgba, f.width, f.height), 0, 0);
      pieceCanvas.set(i, c);
    }
    return pieceCanvas.get(i);
  };
  const L = NPC_MENU_LAYOUT;
  // The box darkens the world through the act palette's 50% blend table, as retail does: PL2
  // AlphaBlend[1] (reference/OpenDiablo2 d2pl2), transform of the world pixel's palette index, entry 0
  // (black). Measured: 80% of retail's fill pixels exact over its own background (s179); the rest
  // is rain. Each world pixel is taken back to its palette index (the world is drawn from it).
  const blends = new Map();
  const blendFor = (act) => {
    if (blends.has(act)) return blends.get(act);
    let value = null;
    try {
      const pl2 = read(`data\\global\\palette\\ACT${act}\\pal.pl2`);
      const base = 1024 + (32 + 16 + 1) * 256 + 65536, index = new Map();
      for (let i = 255; i >= 0; i--) index.set((pl2[i * 4] << 16) | (pl2[i * 4 + 1] << 8) | pl2[i * 4 + 2], i);
      value = { pl2, base, index };
    } catch { value = null; }
    blends.set(act, value); return value;
  };
  const darken = (ctx, b, act) => {
    const t = blendFor(act);
    if (!t) { ctx.fillStyle = 'rgba(0,0,0,0.5)'; ctx.fillRect(b.x, b.y, b.w, b.h); return; }
    const img = ctx.getImageData(b.x, b.y, b.w, b.h), d = img.data;
    for (let i = 0; i < d.length; i += 4) {
      let di = t.index.get((d[i] << 16) | (d[i + 1] << 8) | d[i + 2]);
      if (di === undefined) {   // not a palette colour (a scaled or blended pixel): the nearest one
        let best = 0, bestD = Infinity;
        for (let k = 0; k < 256; k++) { const dr = t.pl2[k * 4] - d[i], dg = t.pl2[k * 4 + 1] - d[i + 1], db = t.pl2[k * 4 + 2] - d[i + 2], dd = dr * dr + dg * dg + db * db; if (dd < bestD) { bestD = dd; best = k; } }
        di = best; t.index.set((d[i] << 16) | (d[i + 1] << 8) | d[i + 2], best);
      }
      const o = t.pl2[t.base + di * 256];
      d[i] = t.pl2[o * 4]; d[i + 1] = t.pl2[o * 4 + 1]; d[i + 2] = t.pl2[o * 4 + 2];
    }
    ctx.putImageData(img, b.x, b.y);
  };
  let state = null, rows = [], box = null, selected = 0;
  // P7: the 'Hire which Mercenary?' window: a fixed 490x350
  // box at (155,85) -- the NPC menu's fill and frame rule -- a text-slider track on the right, a name line and a
  // skill line per offer (7 rows of 34 px show at once), the hover row in blue, a scroll thumb and the Cancel label.
  const HW = { ...hireWindowBox(), rows: 7, textX: 187, firstBottom: 145, pitch: 34, trackX: 627, cancelX: 377, cancelY: 421, headerY: 106 };
  let hire = null;
  const hireRow = (i) => ({ x: HW.x1 + 20, y: HW.firstBottom - 12 + i * HW.pitch, w: 440, h: HW.pitch });
  const layout = (feetX, feetY) => {
    // `name` may be several gold lines (the vendor confirm 0x47500 adds title, item, "Gold:" and the amount before Yes/No).
    const heads = [].concat(state.name);
    const labels = [...heads, ...state.entries.map((e) => e.label ?? (string(e.stringId) || String(e.stringId)))];
    const cx = Math.trunc(feetX);
    const w = Math.max(...labels.map((l) => white.measure(l))) + L.padX;
    const x1 = cx - Math.trunc(w / 2), x2 = x1 + w;
    const y2 = Math.trunc(feetY) - L.bottomBaseAboveFeet + L.bottomPerLine * labels.length,
      y1 = y2 - (L.pitch * labels.length + L.firstLine);
    const bottoms = labels.map((_, i) => y1 + L.firstLine + i * L.pitch);
    const k = heads.length;
    rows = state.entries.map((e, i) => ({ ...e, label: labels[i + k], x: x1 + 16, y: bottoms[i + k] - 12, w: w - 31, h: 15, inkBottom: bottoms[i + k] }));
    box = { x: x1, y: y1, w, h: y2 - y1, x1, y1, x2, y2 };
    return { labels, bottoms, cx, heads: k };
  };
  return {
    get open() { return Boolean(state); },
    get state() { return state; },
    /** `entries` from npcMenuEntries; the menu hangs over the NPC (its feet at the anchor). */
    show(npcId, classId, name, entries, headColour = 4) { state = { npcId, classId, name, entries, headColour, openedAt: Math.trunc(performance.now()) >>> 0 }; rows = []; box = null; selected = 0; },
    /** The pointer moved: the entry under it is selected, none when it is elsewhere (retail). */
    hover(x, y) { if (state && rows.length) selected = rows.findIndex((r) => x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h); return selected; },
    get selectedEntry() { return selected >= 0 ? rows[selected] ?? null : null; },
    /** 1.13c FUN_6FB54860: outside the menu rect widened by 4 on every side. */
    outside(x, y) { return Boolean(box) && (x < box.x1 - 4 || x > box.x2 + 4 || y < box.y1 - 4 || y > box.y2 + 4); },
    close() { state = null; rows = []; box = null; },
    /** The hire window over offers [{ name, level, life, def, cost, skill, mercName }] and the hirer's gold. */
    showHire(npcId, offers, gold) { hire = { npcId, offers, gold, top: 0, selected: 0 }; },
    get hireOpen() { return Boolean(hire); },
    get hireState() { return hire; },
    closeHire() { hire = null; },
    hireHover(x, y, width = 800, height = 600) { if (hire) hire.selected = this.hireAt(x, y, width, height)?.row ?? -1; },
    hireScroll(d) { if (hire) hire.top = Math.max(0, Math.min(Math.max(0, hire.offers.length - HW.rows), hire.top + d)); },
    /** {row: index into offers} | {cancel: true} | null for a canvas point. */
    hireAt(x, y, width = 800, height = 600) {
      if (!hire) return null;
      const origin = hudOrigin(width, height, 'centre');
      x = (x - origin.x) / origin.scale;
      y = (y - origin.y) / origin.scale;
      if (x >= HW.cancelX - 20 && x < HW.cancelX + 100 && y >= HW.cancelY - 16 && y < HW.cancelY + 8) return { cancel: true };
      for (let i = 0; i < Math.min(HW.rows, hire.offers.length - hire.top); i++) { const r = hireRow(i); if (x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h) return { row: hire.top + i }; }
      return null;
    },
    drawHire(ctx, act = 1, width = ctx.canvas?.width || 800, height = ctx.canvas?.height || 600) {
      if (!hire) return null;
      const { gold, blue } = fontsFor(act), n = hire.offers.length, shown = hire.offers.slice(hire.top, hire.top + HW.rows);
      const origin = hudOrigin(width, height, 'centre');
      const b = { x: origin.x + HW.x1 * origin.scale, y: origin.y + HW.y1 * origin.scale,
        w: (HW.x2 - HW.x1) * origin.scale, h: (HW.y2 - HW.y1) * origin.scale };
      // This is a screen-centred modal in retail's 800x600 virtual coordinates. Darken takes
      // device-space pixels (getImageData ignores the canvas transform); the pieces and text below
      // use the same centre origin as the other responsive game panels.
      darken(ctx, b, act);
      ctx.save(); ctx.translate(origin.x, origin.y); ctx.scale(origin.scale, origin.scale);
      recordPrim('box', HW.x1, HW.y1, HW.x2, HW.y2, 0, 1);
      const cel = (path, dc, i, ax, ay) => {   // anchor = bottom row for boxpieces, top row for the slider cels
        const f = dc.frames[i], left = ax + f.offsetX, top = ay + f.offsetY - f.height + 1;
        ctx.drawImage(sliderOrPiece(path, dc, i), left, top); recordDc6(path, i, f, left, top);
      };
      const sliderOrPiece = (path, dc, i) => path === NPC_MENU_ASSETS.pieces ? piece(i) : slid(i);
      // Two nested frames: the window's own and the inset one around the offers (retail draws both).
      for (const r of [[HW.x1, HW.y1, HW.x2, HW.y2], [HW.x1, HW.y1 + 35, HW.x2, HW.y2 - 35]])
        for (const [i, ax, ay] of menuFramePieces(...r)) cel(NPC_MENU_ASSETS.pieces, pieces, i, ax, ay);
      cel(SLIDER, sliders, 9, HW.trackX, 136);
      for (let y = 148; y <= 388; y += 12) cel(SLIDER, sliders, 13, HW.trackX, y);
      cel(SLIDER, sliders, 8, HW.trackX, 388);
      shown.forEach((o, i) => {
        const on = hire.top + i === hire.selected, f = on ? blue : white, y = HW.firstBottom + i * HW.pitch;
        f.drawAnchored(ctx, hireOfferLine(o, string), HW.textX, y);
        f.drawAnchored(ctx, `  ${o.skill}`, HW.textX, y + 14);
      });
      const thumb = Math.max(3, Math.floor(21 * Math.min(1, HW.rows / Math.max(1, n))));
      const ty = hireThumbOffset(hire.top, n, thumb);
      cel(SLIDER, sliders, 16, HW.trackX - 1, 149 + ty);
      for (let k = 0; k < thumb - 2; k++) cel(SLIDER, sliders, 14, HW.trackX - 1, 161 + ty + 12 * k);
      cel(SLIDER, sliders, 15, HW.trackX - 1, 161 + ty + 12 * (thumb - 2));
      const head = `${string(3364).replace('%d', String(hire.gold))}`;
      const centred = (t) => HW.x1 + Math.ceil((HW.x2 - HW.x1 - white.measure(t)) / 2) + 1;   // the menu's own centring rule
      gold.drawAnchored(ctx, head, centred(head), HW.headerY);
      const cancel = string(3400);
      white.drawAnchored(ctx, cancel, centred(cancel), HW.cancelY);
      ctx.restore();
      return { offers: shown.map((o) => o.name), rows: shown.length, total: n, selected: hire.selected, thumbCels: thumb,
        bounds: { x: b.x, y: b.y, width: b.w, height: b.h, centerX: b.x + b.w / 2, centerY: b.y + b.h / 2 } };
    },
    draw(ctx, feetX, feetY, act = 1) {
      if (!state) return null;
      const { labels, bottoms, heads } = layout(feetX, feetY);
      ctx.save();
      darken(ctx, box, act);
      recordPrim('box', box.x1, box.y1, box.x2, box.y2, 0, 1);
      for (const [i, ax, ay] of menuFramePieces(box.x1, box.y1, box.x2, box.y2)) {
        const f = pieces.frames[i], left = ax + f.offsetX, top = ay + f.offsetY - f.height + 1;
        ctx.drawImage(piece(i), left, top);
        recordDc6(NPC_MENU_ASSETS.pieces, i, f, left, top);
      }
      ctx.restore();
      // Centre from the actual box bounds.  This differs from a truncated centre only for an
      // odd-width box holding an even-width line (Jerhyn's 69px menu / "talk"), where retail
      // puts the ink one pixel to the right.
      const line = (font, text, bottom) => font.drawAnchored(ctx, text, box.x1 + Math.ceil((box.w - font.measure(text)) / 2) + 1, bottom);
      const { gold, blue, red } = fontsFor(act);
      // Head lines are colour 4 (gold); the 0x4B5D0 refusal box draws its message in colour 1 (red).
      for (let i = 0; i < heads; i++) line(state.headColour === 1 ? red : gold, labels[i], bottoms[i]);
      rows.forEach((r, i) => line(i === selected ? blue : white, r.label, r.inkBottom));
      return { npcId: state.npcId, classId: state.classId, name: state.name, box: { ...box },
        entries: rows.map((r) => ({ stringId: r.stringId, action: r.action, label: r.label, inkBottom: r.inkBottom })) };
    },
    /** The entry under a canvas point (as last drawn), or null. */
    entryAt(x, y) { return rows.find((r) => x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h) ?? null; },
    targets() { return Object.fromEntries(rows.map((r) => [r.action, [Math.round(r.x + r.w / 2), Math.round(r.y + r.h / 2)]])); },
  };
}
