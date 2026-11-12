import test from 'node:test';
import assert from 'node:assert/strict';
import { packets, sendWindow, createSendGate, createRealmInput, UNIT } from './realm-input.mjs';
import { createMonsterTable } from './realm-tables.mjs';
import { bindDeathRules } from './realm-death.mjs';
import { DEATH_RULES } from '../../test/helpers/death-rules-double.mjs';

bindDeathRules(DEATH_RULES);

// The 1.13c client packet lengths (src/core/realm/d2gs_session.cpp kCsSize113c).
const SIZE = { 0x01: 5, 0x02: 9, 0x03: 5, 0x04: 9, 0x05: 5, 0x06: 9, 0x08: 5, 0x09: 9, 0x0c: 5, 0x0d: 9, 0x0f: 5, 0x10: 9,
  0x13: 9, 0x16: 13, 0x17: 5, 0x18: 17, 0x19: 5, 0x1a: 9, 0x1b: 9, 0x1c: 3, 0x1d: 9, 0x1f: 17, 0x20: 13, 0x21: 9, 0x22: 5, 0x28: 9, 0x29: 9, 0x2a: 9, 0x26: 13,
  0x2f: 9, 0x30: 9, 0x38: 13, 0x3c: 9, 0x45: 9, 0x49: 9, 0x48: 1, 0x51: 9, 0x60: 1 };

test('every builder emits its opcode at the table length', () => {
  const made = [packets.walkTo(1, 2), packets.walkToUnit(1, 3), packets.runTo(1, 2), packets.runToUnit(1, 3),
    packets.leftSkillAt(1, 2), packets.leftSkillOnUnit(1, 3), packets.leftSkillAt(1, 2, true), packets.leftSkillOnUnit(1, 3, true),
    packets.rightSkillAt(1, 2), packets.rightSkillOnUnit(1, 3), packets.rightSkillAt(1, 2, true), packets.rightSkillOnUnit(1, 3, true),
    packets.interact(2, 7), packets.pickUp(4, 9), packets.dropCursor(5), packets.toGrid(5, 1, 2, 0), packets.fromGrid(5),
    packets.equip(5, 4), packets.swapOneForTwoHanded(5, 4), packets.unequip(4), packets.swapWithBody(5, 4),
    packets.swapCursorBuffer(1, 2, 3, 4), packets.stackItems(1, 2), packets.unstackItem(1), packets.socketItem(1, 2), packets.scrollToBook(1, 2), packets.itemToCube(1, 2), packets.useItem(5, 10, 11), packets.useBeltItem(5), packets.npcChatStart(1, 5),
    packets.npcChatEnd(1, 5), packets.npcAction(1, 2, 3), packets.selectSkill(36, false), packets.bindHotkey(2, 36, true), packets.enterPortal(1, 2),
    packets.waypoint(1, 2), packets.swapWeapons(), packets.filmEnd()];
  for (const p of made) assert.equal(p.length, SIZE[p[0]], `opcode 0x${p[0].toString(16)}`);
});

test('field layout is little-endian as the server reads it', () => {
  assert.deepEqual([...packets.walkTo(0x1234, 0x5678)], [1, 0x34, 0x12, 0x78, 0x56]);
  assert.deepEqual([...packets.interact(UNIT.MONSTER, 0x01020304)], [0x13, 1, 0, 0, 0, 4, 3, 2, 1]);
  assert.deepEqual([...packets.pickUp(UNIT.ITEM, 7)], [0x16, 4, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0]);
  assert.deepEqual([...packets.stackItems(7, 0x01020304)], [0x21, 7, 0, 0, 0, 4, 3, 2, 1]);
  assert.deepEqual([...packets.itemToCube(5, 6)], [0x2a, 5, 0, 0, 0, 6, 0, 0, 0]);
  assert.deepEqual([...packets.selectSkill(54, true)], [0x3c, 54, 0x80, 255, 255, 255, 255, 255, 255]);
  assert.deepEqual([...packets.selectSkill(54, false)], [0x3c, 54, 0, 0, 0, 255, 255, 255, 255]);
});

test('the send windows are the retail table', () => {
  for (const op of [5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 17]) assert.equal(sendWindow(op), 50, op);
  for (const op of [1, 3, 4, 11, 18, 0x13, 0x20, 0x39, 0x3b, 0x3c, 0x60]) assert.equal(sendWindow(op), 200, op);
  assert.equal(sendWindow(0x3a), 0);
});

test('the gate drops an equal packet inside its window, shares one store, and never gates 0x3A', () => {
  const sent = []; const gate = createSendGate((b) => sent.push(Array.from(b)));
  assert.ok(gate.send(packets.walkTo(5, 5), 1000));
  assert.equal(gate.send(packets.walkTo(5, 5), 1199), false);
  assert.ok(gate.send(packets.walkTo(5, 5), 1200));
  assert.ok(gate.send(packets.rightSkillAt(5, 5), 1250));
  assert.equal(gate.send(packets.rightSkillAt(5, 5), 1299), false);
  assert.ok(gate.send(packets.rightSkillAt(5, 5), 1300));
  assert.ok(gate.send(packets.walkTo(6, 6), 1301));        // different bytes
  assert.ok(gate.send(Uint8Array.of(0x3a, 1, 0, 0, 0, 0, 0), 2000));
  assert.ok(gate.send(Uint8Array.of(0x3a, 1, 0, 0, 0, 0, 0), 2000));
  assert.equal(sent.length, 7);
});

function sceneOf(extra = {}) {
  return () => ({ ready: true, player: { x: 100, y: 100 }, run: false, selected: { left: 0, right: 36 },
    monsters: [], objects: [], items: [], isNpc: (c) => c === 99, hoverBox: (dx, dy) => Math.abs(dx) < 10 && Math.abs(dy) < 20,
    toScreen: (x, y) => ({ x: x * 2, y: y }), pointerWorld: (px, py) => ({ x: px / 2, y: py }), ...extra });
}
const harness = (extra) => { const out = []; const input = createRealmInput({ transmit: (b) => out.push(Array.from(b)), scene: sceneOf(extra), now: () => 0 });
  return { out, input }; };

test('a click on bare ground walks, or runs with the run toggle', () => {
  let h = harness(); h.input.leftClick(200, 120);
  assert.deepEqual(h.out, [[1, 100, 0, 120, 0]]);
  h = harness({ run: true }); h.input.leftClick(200, 120);
  assert.deepEqual(h.out, [[3, 100, 0, 120, 0]]);
});

test('left on a monster is the left skill on that unit; on a person, a talk; right, the right skill', () => {
  const mon = (guid, classId) => ({ guid, classId, mode: 1, x: 101, y: 101, anchorX: 200, anchorY: 120, drawn: true });
  let h = harness({ monsters: [mon(77, 5)] }); h.input.leftClick(200, 120);
  assert.deepEqual(h.out, [[6, 1, 0, 0, 0, 77, 0, 0, 0]]);
  h = harness({ monsters: [mon(78, 99)] }); h.input.leftClick(200, 120);
  assert.deepEqual(h.out, [[0x13, 1, 0, 0, 0, 78, 0, 0, 0]]);
  h = harness({ monsters: [mon(77, 5)] }); h.input.rightClick(200, 120);
  assert.deepEqual(h.out, [[0x0d, 1, 0, 0, 0, 77, 0, 0, 0]]);
  h = harness(); h.input.rightClick(200, 120);
  assert.deepEqual(h.out, [[0x0c, 100, 0, 120, 0]]);
  h = harness({ monsters: [{ ...mon(77, 5), mode: 12 }] }); h.input.leftClick(200, 120);
  assert.equal(h.out[0][0], 1);                                   // a corpse is not a target
});

test('another player is picked after monsters; the click and its arrival are the engine\'s, the page sends what it started', () => {
  const other = { guid: 0x01020304, name: 'Bob', x: 101, y: 101, anchorX: 200, anchorY: 120, drawn: true };
  let h = harness({ players: [other], inTown: true });
  assert.deepEqual(h.input.pick(200, 120), { kind: 'player', unit: other });
  assert.equal(h.input.leftClick(200, 120), false);             // no engine click: nothing is decided here
  assert.deepEqual(h.out, []);
  h = harness({ players: [{ ...other, drawn: false }], inTown: true });
  assert.equal(h.input.pick(200, 120).kind, 'ground');
  const mon = { guid: 77, classId: 5, mode: 1, x: 101, y: 101, anchorX: 200, anchorY: 120, drawn: true };
  h = harness({ players: [other], monsters: [mon], inTown: true });
  assert.equal(h.input.pick(200, 120).kind, 'monster');
  // The engine answers the click: in reach the use (0x13), else the walk (0x02) with the use waiting for a later frame.
  for (const inTown of [true, false]) {
    const out = [], calls = []; let sends = [];
    const input = createRealmInput({ transmit: (b) => out.push(Array.from(b)), scene: sceneOf({ players: [other], inTown }), now: () => 0,
      clickPlayer: (guid, run) => { calls.push([guid, run]); sends = [{ op: 0x13, type: 0, guid }]; return 1; },
      takeSends: () => { const s = sends; sends = []; return s; } });
    assert.deepEqual(input.leftClick(200, 120), { kind: 'player', unit: other });
    assert.deepEqual(calls, [[0x01020304, false]]);
    assert.deepEqual(out, [[0x13, 0, 0, 0, 0, 4, 3, 2, 1]]);   // the town is not a test of the client's player click
  }
  const out = []; let sends = [];
  const input = createRealmInput({ transmit: (b) => out.push(Array.from(b)), scene: sceneOf({ players: [other] }), now: () => 0,
    clickPlayer: (guid) => { sends = [{ op: 0x02, type: 0, guid }]; return 1; }, takeSends: () => { const s = sends; sends = []; return s; } });
  input.leftClick(200, 120);
  assert.deepEqual(out, [[0x02, 0, 0, 0, 0, 4, 3, 2, 1]]);
  sends = [{ op: 0x13, type: 0, guid: other.guid }]; input.tick();   // the engine's arrival started the use
  assert.deepEqual(out.at(-1), [0x13, 0, 0, 0, 0, 4, 3, 2, 1]);
});

// The server never echoes the owner's own walk, so the click itself must start the local route -- and, as
// in retail, the local start comes first and the packet goes out only when the player could start it.
test('a ground click starts the local route first and sends only what the player could start', () => {
  const moves = [], order = [];
  let answer = 1;
  const make = (extra, transmit = (b) => { order.push(`send:${b[0]}`); return true; }) => createRealmInput({ transmit, scene: sceneOf(extra), now: () => 0,
    move: (x, y, run) => { moves.push([x, y, run]); order.push('move'); return answer; } });
  make().leftClick(200, 120);
  make({ run: true }).leftClick(200, 120);
  assert.deepEqual(moves, [[100, 120, false], [100, 120, true]]);
  assert.deepEqual(order, ['move', 'send:1', 'move', 'send:3']);
  moves.length = 0; order.length = 0;
  make({ monsters: [{ guid: 77, classId: 5, mode: 1, x: 101, y: 101, anchorX: 200, anchorY: 120, drawn: true }] }).leftClick(200, 120);
  assert.deepEqual(moves, []);                                     // a monster is a skill, not a walk
  answer = 0;                                                      // the local start refuses the route: nothing is sent
  order.length = 0; assert.equal(make().leftClick(200, 120), false); assert.equal(make().step(1, 0), false);
  assert.deepEqual(order, ['move', 'move']);
  answer = -1;                                                     // no engine answer: sent as before
  order.length = 0; make().leftClick(200, 120); assert.deepEqual(order, ['move', 'send:1']);
  answer = 1;
  const input = make(); input.leftClick(200, 120); input.leftClick(200, 120);  // the gate drops the repeat packet only
  assert.equal(order.filter((o) => o === 'send:1').length, 2);
  assert.equal(input.evidence.localMove, 1);
});

test('either hand\'s skill is started locally first; a refused start sends nothing', () => {
  const calls = [];
  const make = (extra, result) => { const out = []; return { out, input: createRealmInput({ transmit: (b) => out.push(Array.from(b)), scene: sceneOf(extra), now: () => 0,
    skillAt: (h, x, y) => { calls.push(['at', h, x, y]); return result; }, skillOnUnit: (h, t, g) => { calls.push(['unit', h, t, g]); return result; } }) }; };
  const mon = { guid: 77, classId: 5, mode: 1, x: 101, y: 101, anchorX: 200, anchorY: 120, drawn: true };
  let h = make({}, 1); h.input.rightClick(200, 120); assert.deepEqual(h.out, [[0x0c, 100, 0, 120, 0]]);
  h = make({ monsters: [mon] }, 1); h.input.rightClick(200, 120); assert.deepEqual(h.out, [[0x0d, 1, 0, 0, 0, 77, 0, 0, 0]]);
  h = make({}, 0); assert.equal(h.input.rightClick(200, 120), false); assert.deepEqual(h.out, []);
  h = make({ monsters: [mon] }, 0); assert.equal(h.input.rightClick(200, 120), false); assert.deepEqual(h.out, []);
  h = make({ monsters: [mon] }, 1); h.input.leftClick(200, 120); assert.deepEqual(h.out, [[0x06, 1, 0, 0, 0, 77, 0, 0, 0]]);
  h = make({ monsters: [mon] }, 0); assert.equal(h.input.leftClick(200, 120), false); assert.deepEqual(h.out, []);
  h = make({}, 1); h.input.leftClick(200, 120, { stand: true }); assert.deepEqual(h.out, [[0x05, 100, 0, 120, 0]]);
  h = make({}, 0); assert.equal(h.input.leftClick(200, 120, { stand: true }), false); assert.deepEqual(h.out, []);
  h = make({ monsters: [mon] }, 1); h.input.rightClick(200, 120, { held: true }); h.input.leftClick(200, 120, { held: true });
  assert.deepEqual(h.out, [[0x10, 1, 0, 0, 0, 77, 0, 0, 0], [0x09, 1, 0, 0, 0, 77, 0, 0, 0]]);
  assert.deepEqual(calls, [['at', 1, 100, 120], ['unit', 1, 1, 77], ['at', 1, 100, 120], ['unit', 1, 1, 77], ['unit', 0, 1, 77], ['unit', 0, 1, 77],
    ['at', 0, 100, 120], ['at', 0, 100, 120], ['unit', 1, 1, 77], ['unit', 0, 1, 77]]);
});

test('a refused right-hand start speaks the client\'s refusal line; left, held and started casts stay silent', () => {
  const heard = []; let event = 0x15;
  const make = (result) => { const out = []; return { out, input: createRealmInput({ transmit: (b) => out.push(Array.from(b)), scene: sceneOf({}), now: () => 0,
    skillAt: () => result, skillOnUnit: () => result, refusalSound: () => event, onRefused: (e) => heard.push(e) }) }; };
  let h = make(0); assert.equal(h.input.rightClick(200, 120), false); assert.deepEqual(h.out, []); assert.deepEqual(heard, [0x15]);
  event = 0x13; h.input.rightClick(200, 120); assert.deepEqual(heard, [0x15, 0x13]);
  h.input.rightClick(200, 120, { held: true }); h.input.leftClick(200, 120, { stand: true }); assert.equal(heard.length, 2);
  h = make(1); h.input.rightClick(200, 120); assert.equal(heard.length, 2);
});

// The server ignores a talk from 9 or more tiles; a far person is approached and talked to on arrival.
test('a far person is approached first and talked to once in range; items and objects walk there locally', () => {
  const moves = []; let scene = sceneOf({ monsters: [{ guid: 78, classId: 99, mode: 1, x: 120, y: 100, anchorX: 200, anchorY: 120, drawn: true }] })();
  const out = [];
  const input = createRealmInput({ transmit: (b) => out.push(Array.from(b)), scene: () => scene, now: () => 0,
    move: (x, y, run) => { moves.push([x, y, run]); return 1; } });
  input.leftClick(200, 120);
  assert.deepEqual(out, [[0x02, 1, 0, 0, 0, 78, 0, 0, 0]]);
  assert.deepEqual(moves, [[120, 100, false]]);
  input.tick(); assert.equal(out.length, 1);                      // still far: nothing more
  scene = { ...scene, player: { x: 116, y: 100 } };
  input.tick(); assert.deepEqual(out.at(-1), [0x13, 1, 0, 0, 0, 78, 0, 0, 0]);
  input.tick(); assert.equal(out.length, 2);                      // one talk only
  moves.length = 0;
  const h = createRealmInput({ transmit: () => true, scene: sceneOf({ items: [{ id: 9, mode: 3, x: 100, y: 120 }] }), now: () => 0,
    move: (x, y, run) => { moves.push([x, y, run]); return 1; } });
  h.leftClick(200, 120);
  assert.deepEqual(moves, [[100, 120, false]]);
});

test('arrow keys walk a step from the player; the interact key talks to the nearest person', () => {
  const moves = [];
  const make = (extra) => { const out = []; return { out, input: createRealmInput({ transmit: (b) => out.push(Array.from(b)), scene: sceneOf(extra), now: () => 0,
    move: (x, y, run) => { moves.push([x, y, run]); return 1; } }) }; };
  let h = make(); h.input.step(1, -1);
  assert.deepEqual(h.out, [[1, 101, 0, 99, 0]]); assert.deepEqual(moves, [[101, 99, false]]);
  h = make({ monsters: [{ guid: 5, classId: 99, mode: 1, x: 103, y: 100 }, { guid: 6, classId: 5, mode: 1, x: 101, y: 100 }],
    objects: [{ id: 12, x: 100, y: 101 }] });
  h.input.interactNearest();
  assert.deepEqual(h.out, [[0x13, 1, 0, 0, 0, 5, 0, 0, 0]]);
  h = make({ objects: [{ id: 12, x: 100, y: 101 }] }); h.input.interactNearest();
  assert.deepEqual(h.out, [[0x13, 2, 0, 0, 0, 12, 0, 0, 0]]);
});

test('the realm shell routes arrow keys, the interact key and the pending talk', async () => {
  const { readFile } = await import('node:fs/promises');
  const shell = await readFile(new URL('../gameplay-shell.mjs', import.meta.url), 'utf8');
  const branch = shell.slice(shell.indexOf("if (realmTransport.state === 'session-open') {\n    // The same commands"), shell.indexOf('if (!session) {\n    const advanced'));
  assert.match(branch, /realmInput\(\)\.step\(command\.x, command\.y\)/);
  assert.match(branch, /realmInput\(\)\.interactNearest\(\)/);
  assert.match(branch, /realmInput\(\)\.tick\(\)/);
});

test('the realm shell wires the local route to the engine export', async () => {
  const { readFile } = await import('node:fs/promises');
  const root = new URL('../../', import.meta.url);
  const shell = await readFile(new URL('play/gameplay-shell.mjs', root), 'utf8');
  const call = shell.slice(shell.indexOf('createRealmInput({'), shell.indexOf('createRealmInput({') + 2000);
  assert.match(call, /move:[^\n]*d2_realm_client_local_move/);
  const link = await readFile(new URL('wasm-build/link-d2.sh', root), 'utf8');
  assert.match(link, /_d2_realm_client_local_move[,\s]/);
  for (const [hook, name] of [['skillPlan', 'd2_realm_client_skill_plan'], ['skillAt', 'd2_realm_client_local_skill_at'], ['skillOnUnit', 'd2_realm_client_local_skill_on_unit']]) {
    assert.match(call, new RegExp(`${hook}:[^\\n]*_${name}\\(hand`));
    assert.match(link, new RegExp(`_${name}[,\\s]`));
  }
});

// A click on a far item or object walks to the unit; the pickup or the use goes out only on arrival.
test('ground items and objects are walked to by guid and used only on arrival', () => {
  for (const [extra, type, id, use] of [
    [{ items: [{ id: 9, mode: 3, x: 100, y: 120 }] }, 4, 9, [0x16, 4, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0]],
    [{ objects: [{ id: 12, x: 100, y: 120, selectable: true }] }, 2, 12, [0x13, 2, 0, 0, 0, 12, 0, 0, 0]]]) {
    let scene = sceneOf(extra)(); const out = [];
    const input = createRealmInput({ transmit: (b) => out.push(Array.from(b)), scene: () => scene, now: () => 0 });
    input.leftClick(200, 120);
    assert.deepEqual(out, [[0x02, type, 0, 0, 0, id, 0, 0, 0]]);
    assert.deepEqual(input.pending, { type, guid: id });
    input.tick(); assert.equal(out.length, 1);                    // still walking
    scene = { ...scene, player: { x: 100, y: 119 } };
    input.tick(); assert.deepEqual(out.at(-1), use);
    assert.equal(input.pending, null);
  }
  const h = harness({ items: [{ id: 9, mode: 3, x: 100, y: 101 }] }); h.input.leftClick(200, 101);  // in reach: at once
  assert.deepEqual(h.out, [[0x16, 4, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0]]);
});

test('the engine answers reach, the approach and the end of the walk when it can', () => {
  let done = false, inReach = false; const out = [], approaches = [];
  const input = createRealmInput({ transmit: (b) => out.push(Array.from(b)), now: () => 0,
    scene: sceneOf({ objects: [{ id: 12, x: 100, y: 120 }] }),
    approachUnit: (t, g, run) => { approaches.push([t, g, run]); return 1; }, reach: () => (inReach ? 1 : 0), actionDone: () => done });
  input.leftClick(200, 120);
  assert.deepEqual(approaches, [[2, 12, false]]);
  input.tick(); assert.equal(out.length, 1);                      // the walk is not over
  done = true; input.tick();                                       // over, still short: walk again
  assert.deepEqual(approaches.length, 2); assert.equal(out.length, 1);  // the gate drops the equal 0x02
  inReach = true; input.tick(); assert.deepEqual(out.at(-1), [0x13, 2, 0, 0, 0, 12, 0, 0, 0]);
  const refused = createRealmInput({ transmit: (b) => out.push(Array.from(b)), now: () => 0,
    scene: sceneOf({ objects: [{ id: 12, x: 100, y: 120 }] }), approachUnit: () => 0 });
  out.length = 0; assert.equal(refused.leftClick(200, 120), false);
  assert.deepEqual(out, []); assert.deepEqual(refused.pending, { type: 2, guid: 12 });  // 2AA40 keeps the action waiting
});

test('a local NPC approach can be cancelled by a ground click without a late conversation', async () => {
  const { readFileSync } = await import('node:fs');
  const shell = readFileSync(new URL('../gameplay-shell.mjs', import.meta.url), 'utf8');
  const expression = shell.match(/queueUnit: ([^\n]+),\n/)?.[1];
  assert.ok(expression);
  let latched = false;
  const nativeQueue = () => { latched = true; return 1; };
  const resolveQueue = new Function('realmFlow', `return (${expression})`);
  const queueUnit = resolveQueue({ local: {}, protocol: { d2gsClientLocalActOnUnit: nativeQueue } });
  assert.equal(resolveQueue({ protocol: { d2gsClientLocalActOnUnit: nativeQueue } }), nativeQueue);
  const out = [];
  const input = createRealmInput({ scene: sceneOf({ monsters: [{ guid: 77, classId: 155, mode: 1, x: 100, y: 120, anchorX: 200, anchorY: 120, drawn: true }], isNpc: () => true }),
    transmit: bytes => out.push(Array.from(bytes)), queueUnit, takeSends: () => [],
    approachUnit: () => 1, reach: () => 0, move: () => latched ? 0 : 1, now: () => 1000 });
  input.leftClick(200, 120);
  assert.deepEqual(input.pending, { type: UNIT.MONSTER, guid: 77 });
  assert.ok(input.leftClick(300, 200));
  assert.equal(input.pending, null);
  input.tick();
  assert.deepEqual(out.map(bytes => bytes[0]), [0x02, 0x01]);
});

test('the Compelling Orb and the open Hell Forge are used only with their quest item on the cursor', () => {
  const run = (object, code) => { const h = harness({ objects: [{ x: 100, y: 101, selectable: true, ...object }], cursorCode: () => code });
    h.input.leftClick(200, 101); return h.out; };
  assert.deepEqual(run({ id: 3, classId: 404, mode: 0 }, 0), []);
  assert.deepEqual(run({ id: 3, classId: 404, mode: 0 }, 0x20326671), [[0x13, 2, 0, 0, 0, 3, 0, 0, 0]]);
  assert.deepEqual(run({ id: 4, classId: 376, mode: 2 }, 0), []);
  assert.deepEqual(run({ id: 4, classId: 376, mode: 2 }, 0x20686668), [[0x13, 2, 0, 0, 0, 4, 0, 0, 0]]);
  assert.deepEqual(run({ id: 4, classId: 376, mode: 0 }, 0), [[0x13, 2, 0, 0, 0, 4, 0, 0, 0]]);
});

test('a repeated talk is dropped only by the send gate: the same packet inside 200 ms', () => {
  let t = 0; const out = [];
  const input = createRealmInput({ transmit: (b) => out.push(Array.from(b)), now: () => t,
    scene: sceneOf({ monsters: [{ guid: 5, classId: 99, mode: 1, x: 101, y: 100 }] }) });
  input.interactNearest(); t = 300; input.interactNearest(); t = 350; input.interactNearest();
  assert.equal(out.length, 2);
});

test("the server's talk answer opens the chat with 0x2F, or ends it with 0x30 for a unit the client lost", () => {
  const h = harness({ monsters: [{ guid: 5, classId: 99, mode: 1, x: 101, y: 100 }] });
  h.input.npcInteraction([0x28, 1, 5, 0, 0, 0, 7]);
  h.input.npcInteraction([0x28, 1, 6, 0, 0, 0, 7]);
  assert.equal(h.input.npcInteraction([0x28, 6, 5, 0, 0, 0, 7]), null);
  assert.deepEqual(h.out, [[0x2f, 1, 0, 0, 0, 5, 0, 0, 0], [0x30, 7, 0, 0, 0, 6, 0, 0, 0]]);
});

test('a belt key uses the item in its own slot, not a higher one in the column', () => {
  const h = harness({ items: [{ id: 1, mode: 2, gridX: 4 }, { id: 2, mode: 2, gridX: 0 }, { id: 3, mode: 2, gridX: 1 }] });
  assert.ok(h.input.useBelt(0));
  assert.deepEqual(h.out, [[0x26, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]]);
  assert.equal(h.input.useBelt(3), false);
  const upper = harness({ items: [{ id: 1, mode: 2, gridX: 4 }] });
  assert.equal(upper.input.useBelt(0), false);
});

test('a belt key does nothing while dead or with an item on the cursor', () => {
  const items = [{ id: 2, mode: 2, gridX: 0 }];
  assert.equal(harness({ items, playerMode: 17 }).input.useBelt(0), false);
  assert.equal(harness({ items, cursorId: 9 }).input.useBelt(0), false);
  assert.ok(harness({ items, playerMode: 1 }).input.useBelt(0));
});

test('a belt key follows the use routine row test: useable 1, unique and quest not 1, a missing row refuses', () => {
  const items = [{ id: 2, mode: 2, gridX: 0, classId: 7 }];
  const rows = { 7: { useable: 1, unique: 0, quest: 0 } };
  const go = (row) => harness({ items, itemRow: () => row }).input.useBelt(0);
  assert.ok(go(rows[7]));
  assert.equal(go({ useable: 0, unique: 0, quest: 0 }), false);
  assert.equal(go({ useable: 1, unique: 1, quest: 0 }), false);
  assert.equal(go({ useable: 1, unique: 0, quest: 1 }), false);
  assert.equal(go({ useable: 1, unique: 0, quest: 2 }), true);
  assert.equal(go(null), false);
});

test('a sent belt use locks that item for 12 frames (the flag-2 list of 0x81CE0), and state 0x36 refuses', () => {
  let t = 0; const out = [];
  const input = createRealmInput({ transmit: (b) => out.push(Array.from(b)), now: () => t,
    scene: sceneOf({ items: [{ id: 2, mode: 2, gridX: 0 }, { id: 3, mode: 2, gridX: 1 }] }) });
  assert.ok(input.useBelt(0));
  t = 479; assert.equal(input.useBelt(0), false);
  assert.ok(input.useBelt(1));                      // another item has no lock
  t = 480; assert.ok(input.useBelt(0));
  const h = harness({ items: [{ id: 2, mode: 2, gridX: 0 }], heroLocked: () => true });
  assert.equal(h.input.useBelt(0), false);
});

test('a belt key with Shift down names the mercenary as the target', () => {
  const h = harness({ items: [{ id: 2, mode: 2, gridX: 0 }] });
  assert.ok(h.input.useBelt(0, true));
  assert.deepEqual(h.out, [[0x26, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0]]);
});

test('MonStats: the npc and interact columns name the people', () => {
  const text = 'id\tnpc\tinteract\nskeleton\t\t\ncharsi\t1\t1\n';
  const t = createMonsterTable(() => new TextEncoder().encode(text));
  assert.equal(t.isNpc(0), false); assert.equal(t.isNpc(1), true); assert.equal(t.isNpc(9), false);
});

test('a talk that goes out is reported to the NPC state', () => {
  const talked = [], out = [];
  const input = createRealmInput({ transmit: (b) => out.push(Array.from(b)), now: () => 0, onTalk: (g) => talked.push(g),
    scene: sceneOf({ monsters: [{ guid: 78, classId: 99, mode: 1, x: 101, y: 101, anchorX: 200, anchorY: 120, drawn: true }] }) });
  input.leftClick(200, 120);
  assert.deepEqual(out, [[0x13, 1, 0, 0, 0, 78, 0, 0, 0]]);
  assert.deepEqual(talked, [78]);
});

test('an item action asks the leave wait first: refused actions send nothing, allowed ones go out', () => {
  const out = []; let allow = false, asked = 0;
  const input = createRealmInput({ transmit: (b) => out.push(Array.from(b)), scene: sceneOf(), now: () => 0,
    leaveWait: { itemAction: () => { asked++; return allow; } } });
  assert.equal(input.lift(7), false);
  assert.equal(input.equip(7, 1), false);
  assert.equal(input.dropCursor(7), false);
  assert.deepEqual(out, []);
  allow = true;
  assert.ok(input.lift(7));
  assert.equal(asked, 4);
  assert.deepEqual(out, [Array.from(packets.fromGrid(7))]);
});

// plan 0: a hand-to-hand skill out of melee range. The click walks (0x02, or 0x04 running) and keeps the skill
// waiting; once the walk has ended in range the skill starts locally and its plain packet goes out.
test('a hand-to-hand skill on a far monster walks to it first and is used on arrival', () => {
  const far = { guid: 77, classId: 5, mode: 1, x: 110, y: 104, anchorX: 200, anchorY: 120, drawn: true };
  for (const [hand, click, opcode] of [[0, 'leftClick', 0x06], [1, 'rightClick', 0x0d]]) {
    const order = []; let inRange = 0, done = false, start = 1, clock = 0;
    const make = (extra) => createRealmInput({ transmit: (b) => order.push(Array.from(b)), scene: sceneOf({ monsters: [far], ...extra }), now: () => (clock += 1000),
      approachUnit: (type, guid, run) => { order.push(['approach', type, guid, run]); return 1; }, actionDone: () => done,
      skillPlan: (h, type, guid) => { assert.deepEqual([h, type, guid], [hand, 1, 77]); return inRange; },
      skillOnUnit: (h, type, guid) => { order.push(['start', h, type, guid]); return start; },
      skillAt: (h, x, y) => { order.push(['start-at', h, x, y]); return 1; } });
    let input = make();
    input[click](200, 120, { held: true });
    assert.deepEqual(order, [['approach', 1, 77, false], [0x02, 1, 0, 0, 0, 77, 0, 0, 0]]);
    assert.deepEqual(input.pending, { type: 1, guid: 77, hand });
    input.tick(); assert.equal(order.length, 2);                          // still walking: nothing more
    done = true; input.tick();                                           // stopped short: walk again
    assert.deepEqual(order.slice(2), [['approach', 1, 77, false], [0x02, 1, 0, 0, 0, 77, 0, 0, 0]]);
    inRange = 1; order.length = 0; input.tick();
    assert.deepEqual(order, [['start', hand, 1, 77], [opcode, 1, 0, 0, 0, 77, 0, 0, 0]]);   // the plain packet, though the click was held
    assert.equal(input.pending, null);
    input.tick(); assert.equal(order.length, 2);
    // Running: 0x04. A start refused on arrival sends nothing.
    inRange = 0; done = false; order.length = 0; input = make({ run: true }); input[click](200, 120);
    assert.deepEqual(order, [['approach', 1, 77, true], [0x04, 1, 0, 0, 0, 77, 0, 0, 0]]);
    inRange = 1; done = true; start = 0; order.length = 0; input.tick();
    assert.deepEqual(order, [['start', hand, 1, 77]]);
    // Stand Still out of range: the skill at the pointer, no walk.
    inRange = 0; start = 1; order.length = 0; make()[click](200, 120, { stand: true });
    assert.deepEqual(order, [['start-at', hand, 100, 120], [hand ? 0x0c : 0x05, 100, 0, 120, 0]]);
    // In range, or a ranged skill (plan 1): started and sent on the click.
    inRange = 1; order.length = 0; make()[click](200, 120);
    assert.deepEqual(order, [['start', hand, 1, 77], [opcode, 1, 0, 0, 0, 77, 0, 0, 0]]);
  }
});

test('the right button with a hand-to-hand skill on open ground walks there; Stand Still uses the skill', () => {
  const order = [];
  const input = createRealmInput({ transmit: (b) => order.push(Array.from(b)), scene: sceneOf({}), now: () => 0,
    move: (x, y, run) => { order.push(['move', x, y, run]); return 1; }, skillPlan: (h, type) => { assert.deepEqual([h, type], [1, -1]); return 2; },
    skillAt: (h, x, y) => { order.push(['start-at', h, x, y]); return 1; } });
  assert.equal(input.rightClick(200, 120).kind, 'ground');
  input.rightClick(202, 120, { stand: true });
  assert.deepEqual(order, [['move', 100, 120, false], [0x01, 100, 0, 120, 0], ['start-at', 1, 101, 120], [0x0c, 101, 0, 120, 0]]);
});

test('without the engine hooks a monster click sends the skill packet alone', () => {
  const order = [], mon = { guid: 77, classId: 5, mode: 1, x: 110, y: 104, anchorX: 200, anchorY: 120, drawn: true };
  const input = createRealmInput({ transmit: (b) => order.push(Array.from(b)), scene: sceneOf({ monsters: [mon] }), now: () => 0 });
  input.leftClick(200, 120); input.rightClick(200, 120);
  assert.deepEqual(order, [[0x06, 1, 0, 0, 0, 77, 0, 0, 0], [0x0d, 1, 0, 0, 0, 77, 0, 0, 0]]);
});

test('Stand Still with the left button uses the left skill at the pointer and never walks', () => {
  const order = [];
  const input = createRealmInput({ transmit: (b) => order.push(Array.from(b)), scene: sceneOf({}), now: () => 0,
    move: () => { order.push('move'); return 1; } });
  assert.equal(input.leftClick(200, 120, { stand: true }).kind, 'ground');
  input.leftClick(202, 120, { stand: true, held: true });
  assert.deepEqual(order, [[5, 100, 0, 120, 0], [8, 101, 0, 120, 0]]);
});

test('a hotkey assign is 0x51 with the slot above the skill word, the hand in bit 15 and the owner after', () => {
  assert.deepEqual([...packets.bindHotkey(4, 54, true)], [0x51, 54, 0x80, 4, 0, 255, 255, 255, 255]);
  assert.deepEqual([...packets.bindHotkey(0, 36, false, 7)], [0x51, 36, 0, 0, 0, 7, 0, 0, 0]);
  const sent = [];
  const input = createRealmInput({ transmit: (b) => sent.push(Array.from(b)), scene: { player: () => ({}) }, now: () => 0 });
  assert.equal(input.bindHotkey(16, 36, false), false);
  assert.equal(input.bindHotkey(1, -1, false), false);
  assert.equal(sent.length, 0);
});

test('belt lift, place and swap send the retail packets', () => {
  const { out, input } = harness();
  input.beltClick({ op: 'lift', a: 0x11223344 });
  input.beltClick({ op: 'place', a: 5, b: 2 });
  input.beltClick({ op: 'swap', a: 7, b: 9 });
  assert.deepEqual(out, [[0x24, 0x44, 0x33, 0x22, 0x11], [0x23, 5, 0, 0, 0, 2, 0, 0, 0], [0x25, 7, 0, 0, 0, 9, 0, 0, 0]]);
  assert.equal(input.beltClick({ op: 'nope' }), false);
});

test('the identify cursor press is 0x27 [clicked item][armed item], and the cancel repeats the armed id (D2Client 0x95FE0/0xBEB90)', () => {
  const { out, input } = harness();
  input.useOnItem(0x01020304, 0x0a0b0c0d);
  input.useOnItem(0x55, 0x55);
  assert.deepEqual(out, [[0x27, 4, 3, 2, 1, 0x0d, 0x0c, 0x0b, 0x0a], [0x27, 0x55, 0, 0, 0, 0x55, 0, 0, 0]]);
});

test('drop gold is the 9-byte packet with the hero id, and nothing for zero', () => {
  const { out, input } = harness();
  assert.equal(input.dropGold(1, 0), false);
  assert.equal(out.length, 0);
  input.dropGold(0x01020304, 500);
  assert.deepEqual(out, [[0x50, 4, 3, 2, 1, 0xf4, 1, 0, 0]]);
});
