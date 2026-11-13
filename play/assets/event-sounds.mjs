// Footstep, item pickup/drop, gold and level-up sounds. Retail routes all of them through the D2Client sound
// queue (FUN_6FB38A70). Rows: Sounds.txt by name; footsteps by the class group base at D2Client+0xF7694 word +0x28
//: base + 4*(floor-1) for floor class 2..6 [+24 run]. The floor class is the engine's (D2Client 0x226A0 over the
// live room tiles, floorClass(unitType, id, fallback)); the fallback is the level's SoundEnviron "Material 1".
// The sound mixer draws the row group's variants (sound-mixer-rules.mjs).
const CVT_ROWS = [0, 251, 262, 268, 274, 280, 286, 292]; // D2Client+0xF5538 pair table, first of each pair, by hit class
const GAIT_BASE = [2768, 2720, 2720, 2768, 2816, 2720, 2720]; // Ama Sor Nec Pal Bar Dru Asn
import { OBJECT_MODE_SOUNDS, OBJECT_SELECT_SOUNDS, SHARED_ROW_CLASSES } from './object-mode-sounds.mjs';
export function createEventSounds(read, play, monsterStep = null, monsterLayer = null, monsterIdle = null, floorClass = null, stepDue = null) {
  const rows = (p) => new TextDecoder('latin1').decode(read(p)).split(/\r?\n/).map((l) => l.split('\t'));
  // D2Client 0x226A0's level class: Levels.txt SoundEnv indexes the SoundEnviron table by row (record 0x58 bytes, "Material 1"
  // at +0x1C); a value outside 1..6, or no row, is 1. Read once, on the first footstep.
  let materials = null;
  const levelMaterial = (level) => {
    if (!materials) {
      materials = new Map();
      try {
        const [lh, ...lr] = rows('data\\global\\excel\\Levels.txt'), [sh, ...sr] = rows('data\\global\\excel\\SoundEnviron.txt');
        const id = lh.indexOf('Id'), env = lh.indexOf('SoundEnv'), mat = sh.indexOf('Material 1'), table = sr.filter((c) => c[0]);
        for (const c of lr) if (c[id] !== '' && c[id] !== undefined) { const m = Number(table[Number(c[env])]?.[mat]); materials.set(Number(c[id]), m > 0 && m < 7 ? m : 1); }
      } catch { /* no tables: every level falls back to 1 */ }
    }
    return materials.get(level) ?? 1;
  };
  let stepLevel = 1;
  const floorOffset = (unitType, id) => { const c = floorClass ? floorClass(unitType, id, levelMaterial(stepLevel)) : 0; return c >= 2 && c <= 6 ? 4 * (c - 1) : 0; }; // 0x22FB0
  const idx = new Map(rows('data\\global\\excel\\Sounds.txt').slice(1).filter((c) => c[0]).map((c) => [c[0], Number(c[1])]));
  const drop = new Map();
  for (const f of ['armor', 'weapons', 'misc']) {
    const [h, ...r] = rows(`data\\global\\excel\\${f}.txt`), code = h.indexOf('code'), ds = h.indexOf('dropsound'), df = h.indexOf('dropsfxframe');
    for (const c of r) if (c[code] && idx.has(c[ds])) drop.set(c[code], { row: idx.get(c[ds]), frame: Number(c[df]) || 0 });
  }
  const inRoom = new Set(); // object ids already seen inside a client room
  const selSeen = new Set(); // 'x,y' of objects already selected: a room ordinal shift re-ids the same river object (a retail recording)
  const objMode = new Map(); // object id -> last mode
  const seen = new Map(); // item id -> { ground, code, played }
  // The snapshot's code is dwCode, the 4 ASCII bytes little-endian ('hax ').
  const codeOf = (c) => (typeof c === 'string' ? c : String.fromCharCode(c & 255, (c >> 8) & 255, (c >> 16) & 255, (c >>> 24) & 255)).replace(/[ \0]+$/, '');
  const lastStep = new Map(), lastFrame = new Map(), rebase = new Map(); let tick = 1 << 20; // monster id -> client tick of its last footfall (unit +0x84)
  let level = 0, stepT = -2, cur = 0, serverLevelUps = 0, serverPickups = 0;
  // Retail's client-side player step for a run (fit to a retail recording/a retail recording/a retail recording Q rows): the walk step 320 over a 3072 cycle (213.33/2048, the exact 4.8-tick footfall period a retail recording gaps need; 213 drifted a tick per 24; D2Common 0x2FB10 walk base,
  // not the run base 101 the server unit carries) over a 2048 cycle; footfalls two per cycle by the 0x23C00 due rule (nearest of cur-step, cur, cur+step
  // to phase 0, circular mod 1536), none within 2/3 of a period of the last. Phase 1207 after the run footfall gives the traced 6,5,4,5,5,5,5,4 gaps.
  let itemLevel = null, itemsSeen = false, deadSeen = new Set();
  // A run a cast started (no explicit move command) keeps the client unit in run mode 3 (a retail recording retail F rows: step 0x97 over a 2048 cycle, footfalls
  // mod 1024, every one the run group, gaps 7,7,6,7); an explicit move flips it to walk/town-walk after the first tick (step 0xD5, the model above).
  // The due test itself is the engine's 0x23C00 check (stepDue = _d2_client_footstep_due: two footfalls a cycle, phase 0,
  // the 2/3-period gap); without it the hero steps only on the armed first footfall.
  let STEP = 320, PER = 1536, RUNG = 0;
  const footDue = (c, ticks) => !!stepDue && stepDue(c, 2 * PER, STEP, 2, 0, ticks, 0) !== 0;
  return {
    /** D2Client 2A750's local action 2 (cast on the ground): 1C150 -> 23C00 plays the walk group's first footfall at once. */
    /** 2A750's local action 7 (cast on a unit): 24490 -> 242B0 plays the weapon-class row of the client unit; k is the hit class of its weapon, 1 with none. */
    localAttack(k = 1) { const r = CVT_ROWS[k & 15] ?? 0; if (r > 0) play(r, { kind: 'weapon', unitType: 0, baseRow: r }); },
    localWalk(cls) { play((GAIT_BASE[cls] ?? 2768) + floorOffset(0, 0), { kind: 'footstep', unitType: 0, classId: cls }); },
    /** The engine's own level-up event (soundEvent 2 -> row 7) was played; the level poll below must not repeat it. */
    /** A new session replaced the world: forget the objects seen in the old one (first sight is per session). */
    resetObjects() { objMode.clear(); selSeen.clear(); inRoom.clear(); seen.clear(); itemsSeen = false; }, // new world: its items are the world as found
    noteServerLevelUp() { serverLevelUps++; },
    /** The engine's own pickup event (soundEvent 1 -> item_pickup) was played; the ground-to-gone poll below must not repeat it (a retail recording: retail submits one 235 per pickup, D9/D35). */
    noteServerPickup() { serverPickups++; },
    /** Per tick: snapshot.player (+ .motion), snapshot.items, and the hero's level. `remote` ({ cursorId, goldGained }) is a game read from packets: a ground item that leaves counts as the hero's pickup only when it is the item now on the cursor, or gold while the hero's gold rose (the server also removes items other players take or that leave the hero's range). */
    update(snapshot, heroLevel, remote = null) {
      // Packet snapshots carry the hero class at the top level, without a player
      // entity. Preserve transformed identities, then use that authoritative class.
      const p = snapshot?.player, m = p?.motion?.active ? p.motion : p, cls = p?.renderIdentity?.classId ?? p?.classId ?? snapshot?.playerClass ?? snapshot?.entities?.find((e) => e.type === 0 && e.id === snapshot.playerId)?.classId;
      if (Number.isInteger(snapshot?.level)) stepLevel = snapshot.level;
      const foot = (extra) => play((GAIT_BASE[cls] ?? 2768) + extra + floorOffset(0, 0), { kind: 'footstep', unitType: 0, classId: cls }); // 0x22FB0
      if (m && (m.mode === 2 || m.mode === 3 || m.mode === 6)) { // 6 = town walk (a retail recording: base row, no run footfall)
        // Retail (D2Client+0x23C00, traced against a retail recording): the run's first footfall plays the run group at once,
        // the rest follow the walk-step model above, all in the walk group.
        if (stepT === -2) { stepT = -1; const field = m.mode === 3 && !p.explicitMove; [STEP, PER, RUNG] = field ? [151, 1024, 24] : [320, 1536, 0]; } // armed: retail submits one tick after the unit enters the mode
        else if (stepT === -1) { foot(m.mode === 6 ? 0 : 24); stepT = 0; cur = m.mode === 6 ? 1527 : RUNG ? 0 : 1207; }
        else { cur = (cur + STEP) % PER; if (footDue(cur, ++stepT)) { foot(RUNG); stepT = 0; } }
      } else { if (stepT === -1) foot(24); else if (stepT >= 0) { cur = (cur + STEP) % PER; if (footDue(cur, ++stepT)) foot(RUNG); } stepT = -2; } // the exit tick still runs the callback in the old mode (a retail recording) // a one-tick run (a retail recording) still submits its armed footfall
      ++tick;
      for (const e of monsterStep ? snapshot?.entities ?? [] : []) {
        let a = e.type === 1 && e.animation;
        if (!a?.inClientRoom) continue;
        // A new path for a unit already walking (same mode, new step) restarts the client's walk cycle: retail's unit is idle for the arrival tick
        // and steps from frame 0 after it (a retail recording, F records). The snapshot keeps counting, so measure frames from the frame it had then.
        const prior = lastFrame.get(e.id);
        let rebased = false;
        if (prior && prior[0] === a.mode && a.mode === 2 && prior[7] !== a.animSpeed && prior[2] === tick - 1) { rebase.set(e.id, a.framePrecise - a.animSpeed); rebased = false; }
        else if (!prior || prior[0] !== a.mode) rebase.delete(e.id);
        const base = rebase.get(e.id), rawFrame = a.framePrecise;
        if (base !== undefined && a.frameCountPrecise > 0) a = { ...a, framePrecise: (((rawFrame - base) % a.frameCountPrecise) + a.frameCountPrecise) % a.frameCountPrecise };
        const prev = prior, moved = prev ? (e.preciseX - prev[3]) ** 2 + (e.preciseY - prev[4]) ** 2 : 0;
        lastFrame.set(e.id, [a.mode, a.framePrecise, tick, e.preciseX, e.preciseY, moved, a.frameCountPrecise, a.animSpeed]);   // retail runs 0x23C00 before the advance: its +0x44 is last tick's snapshot
        // Walk -> attack (a retail recording vs a retail recording): retail moves the unit, then runs 0x23C00, then the attack packet changes the mode. A walk that
        // arrives this tick (moved no more than its previous step) is mode 1 by the time 0x23C00 runs, so no footfall; a walk cut short while still
        // moving (moved more than the previous step) is still mode 2 there and steps.
        const cut = prev && prev[0] === 2 && (a.mode === 5 || a.mode === 6) && moved > prev[5];
        if (rebased) continue;
        if (!prev || (prev[0] !== a.mode && !cut) || prev[2] !== tick - 1) continue;
        // A new path for a unit already walking (0x67/0x68 to the client) restarts its walk cycle at frame 0 in the tick it arrives: retail's 0x23C00
        // sees the restart (a retail recording: a raven re-pathed mid-walk skips the footfall its wrapped frame would have made), so no footfall that tick.
        if (!cut && prev[0] === a.mode && a.frameCountPrecise > 0 && (prev[1] + a.animSpeed) % a.frameCountPrecise !== a.framePrecise && a.framePrecise === a.animSpeed % a.frameCountPrecise) continue;
        // Footfalls belong to a unit that walks: a walk/run-mode unit whose position did not change since last tick
        // (a town NPC held in its walk mode while it stands) makes no footstep. Retail's unit leaves the walk mode when it stops.
        if (!cut && (a.mode === 2 || a.mode === 0xF) && moved === 0) continue;
        const row = monsterStep(a.classId, cut ? 2 : a.mode, prev[1], cut ? prev[6] : a.frameCountPrecise, cut ? prev[7] : a.animSpeed, tick, lastStep.get(e.id) ?? 0, floorClass ? floorClass(1, e.id, levelMaterial(stepLevel)) : 0);
        // unitId attaches the footfall to the monster so the player places it against the listener (Falloff, pan).
        if (row) { lastStep.set(e.id, tick); play(row, { kind: 'footstep', id: e.id, unitId: e.id, unitType: 1, classId: a.classId }); const layer = monsterLayer?.(a.classId); if (layer) play(layer, { kind: 'footstep', id: e.id, unitId: e.id, unitType: 1, classId: a.classId }); /* retail 0x88A70 submits the layer row on the same footfall (FsPrb 100) */ }
      }
      // D2Client 0x4F4A0 -> 0x237E0 (the idle/neutral sound) per client monster, in the unit table's order (bucket id & 0x7F, newest first); mode is 0x223D0's.
      if (monsterIdle) for (const e of (snapshot?.entities ?? []).filter((x) => x.type === 1 && x.animation?.inClientRoom).sort((p, q) => (p.id & 127) - (q.id & 127) || q.id - p.id)) {
        const row = monsterIdle(e.id, e.classId, e.mode === 14 && e.animation?.hasAnimSequence ? e.animation.seqMode : e.mode); if (row) play(row, { kind: 'idle', id: e.id, unitType: 1, classId: e.classId }); }
      // A monster that dies frozen/shattered (States.txt shatter column: 1, 107, 184) throws its drops with no flip delay: the class dropsound plays on the arrival tick (a retail recording Q).
      let shatterNow = false; const deadNow = new Set();
      for (const m of snapshot?.entities ?? []) if (m.type === 1 && m.mode === 0) { deadNow.add(m.id); if (!deadSeen.has(m.id) && (m.states?.has(1) || m.states?.has(107) || m.states?.has(184))) shatterNow = true; }
      deadSeen = deadNow;
      // A level change (waypoint, stairs) swaps the whole item set: the new area's ground items are the world as found, not drops (a gold pile there is not a gold drop).
      if (Number.isInteger(snapshot?.level)) { if (itemLevel !== null && snapshot.level !== itemLevel) { seen.clear(); itemsSeen = false; } itemLevel = snapshot.level; }
      const now = new Map(), sawItems = itemsSeen; // items list seen on an earlier update: the first one is the world as found, not arrivals
      if (Array.isArray(snapshot?.items)) itemsSeen = true;
      for (const it of snapshot?.items ?? []) {
        const ground = it.mode === 3 && it.invPage === 255, code = codeOf(it.code), old = seen.get(it.id), d = drop.get(code);
        // Retail: a dropping item (mode 5) plays Items.txt dropsound when its flip reaches dropsfxframe; any other
        // arrival on the ground (cursor drop) plays it at once. Once per item.
        let played = old?.played ?? false;
        // A monster's drop appearing on the ground submits the class dropsound at once, its delay field = dropsfxframe (a retail Hurricane kill: Q at the entering tick, delay 12).
        // Retail (a retail recording/a retail recording/a retail recording/a retail recording Q records, D2Client+0x88A70): an item entering the world in drop mode submits item_flippy (row 216) at once, whatever its class; the class dropsound follows at dropsfxframe.
        const unit = { unitType: 4, classId: it.classId }, arrived = !old && sawItems && ground; // an item first seen on the ground mid-session was dropped (the server reports mode 3, not the client's flip mode 5)
        if (!old && ((it.mode === 5 && it.invPage === 255) || arrived)) play(idx.get('item_flippy'), { kind: 'drop-flip', ...unit });
        // Retail Q records (D2Client+0x88A70, 57 drop submits in the traces): the class dropsound is submitted on the flip tick itself, with the row's dropsfxframe as its delay; the audio starts that many ticks later (unit-sound-player playRow).
        const born = old?.born ?? tick;
        if (d && !played && ((it.mode === 5 && it.invPage === 255) || arrived || (old && old.drop) || (old && !old.ground && !old.flip && ground))) { play(d.row, { kind: 'drop', delay: shatterNow && !old ? 0 : d.frame, ...unit }); played = true; }
        now.set(it.id, { ground, code, born, drop: !played && (!!old?.drop || (!old && ((it.mode === 5 && it.invPage === 255) || arrived))), flip: !!old?.flip || it.mode === 5, played: (ground || it.mode === 5) && played });
      }
      for (const [id, old] of seen) if (old.ground && !(now.get(id)?.ground)) { if (remote && !(id === remote.cursorId || (old.code === 'gld' && remote.goldGained))) continue; if (serverPickups > 0) serverPickups--; else play(idx.get('item_pickup'), { kind: 'pickup', unitType: 0, classId: cls }); } // gold too: retail queues the pickup row at a gold pickup, the gold row is the pile's drop announce
      seen.clear(); for (const [k, v] of now) seen.set(k, v);
      // D2Client+0x237E0 object branch: a seen object entering mode m plays row[m] of its class record (table
      // D2Client+0xF6C58, play/assets/object-mode-sounds.mjs). The shared row (0xF6988) plays only if previous <= m.
      // Class 0x3B also plays on first sight. Chests: OP (Mode1=1) or ON (Mode1=0) entry plays object_chest_*.
      const nowObj = new Map();
      for (const o of snapshot?.objects ?? []) {
        nowObj.set(o.id, o.mode); // first sight of the selector = the object's room gaining a client (retail holds only such units), not the first snapshot
        const prev = objMode.get(o.id), row = OBJECT_MODE_SOUNDS[o.classId];
        // First sight (client rooms only): selector 0x224F0 -- record +0x20..+0x2C [first, slot, second, slot2]; slot == mode or 8 (any).
        const q = OBJECT_SELECT_SOUNDS[o.classId];
        if (q && o.inClientRoom && !inRoom.has(o.id) && !selSeen.has(o.x + ',' + o.y)) { selSeen.add(o.x + ',' + o.y); const s = (q[0] && (q[1] === o.mode || q[1] === 8)) ? q[0] : (q[2] && (q[3] === o.mode || q[3] === 8)) ? q[2] : 0; if (s) play(s, { kind: 'object', id: o.id }); }
        if (o.inClientRoom) inRoom.add(o.id); else { inRoom.delete(o.id); selSeen.delete(o.x + ',' + o.y); } // a room that loses its client frees the unit: regaining one is a first sight again
        if (!row || prev === o.mode || (prev === undefined && o.classId !== 0x3b)) continue;
        if (row[o.mode] && !(SHARED_ROW_CLASSES.has(o.classId) && (prev ?? 0) > o.mode)) play(row[o.mode], { kind: 'object', id: o.id });
      }
      objMode.clear(); for (const [k, v] of nowObj) objMode.set(k, v);
      if (heroLevel > 0) { if (level && heroLevel > level) { if (serverLevelUps > 0) serverLevelUps--; else play(idx.get('cursor_level_up'), { kind: 'levelup' }); } level = heroLevel; }
    },
  };
}
