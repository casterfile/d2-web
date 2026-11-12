import test from 'node:test';
import assert from 'node:assert/strict';
import { createRealmInput } from './realm-input.mjs';
import { realmHeroFlags, realmCorpseViews, bindDeathRules } from './realm-death.mjs';
import { DEATH_RULES } from '../../test/helpers/death-rules-double.mjs';

bindDeathRules(DEATH_RULES);
import { deathScreenShown, respawnAllowed } from './death-screen.mjs';

const HERO = 0x01020304, CORPSE = 0x0a0b0c0d, OTHER = 0x55, OTHER_CORPSE = 0x66;
const le = (v) => [v & 255, (v >>> 8) & 255, (v >>> 16) & 255, (v >>> 24) & 255];

test('the hero flags come from the character list', () => {
  const list = [{ name: 'Soft', flags: 0x20 }, { name: 'Hard', flags: 0x24 | 0x08 }, { name: 'Classic', flags: 0 }];
  assert.equal(realmHeroFlags(list, 'soft'), 0x20);
  assert.equal(realmHeroFlags(list, 'HARD'), 0x24);
  assert.equal(realmHeroFlags(list, 'Classic'), 0);
  assert.equal(realmHeroFlags(list, 'Nobody'), 0x20);
  assert.equal(realmHeroFlags(null, undefined), 0x20);
});

test('corpse views join the corpse list to the player units in view', () => {
  const players = [{ guid: CORPSE, x: 120, y: 100, renderState: { mode: 17 } }, { guid: OTHER, x: 90, y: 90, renderState: { mode: 1 } },
    { guid: OTHER_CORPSE, x: 91, y: 91, renderState: { mode: 17 } }];
  const pairs = [{ owner: HERO, corpse: CORPSE }, { owner: OTHER, corpse: OTHER_CORPSE }, { owner: OTHER, corpse: 0x77 }];
  assert.deepEqual(realmCorpseViews(pairs, players, HERO), [
    { guid: CORPSE, owner: HERO, own: true, mode: 17, x: 120, y: 100 },
    { guid: OTHER_CORPSE, owner: OTHER, own: false, mode: 17, x: 91, y: 91 }]);
  assert.deepEqual(realmCorpseViews(undefined, players, HERO), []);
  assert.deepEqual(realmCorpseViews(pairs, [], HERO), []);
});

// A stand-in for the engine's click on a player unit (D2Client 0x1AB20) and the arrival check of the use it
// leaves waiting (0x2AFB0): in reach (under 5 tiles) the use starts, else the walk does and the use waits; on
// arrival it starts only while the corpse is still the hero's to open. The engine's own answers are its tests'.
function enginePlayerClick(hero, where, allowed) {
  let sends = [], waiting = 0;
  const near = (guid) => { const u = where(guid), me = hero(); return Math.max(Math.abs(u.x - me.x), Math.abs(u.y - me.y)) < 5; };
  return {
    clickPlayer: (guid, run) => {
      if (near(guid) && allowed(guid)) { sends.push({ op: 0x13, type: 0, guid }); waiting = 0; }
      else { sends.push({ op: run ? 0x04 : 0x02, type: 0, guid }); waiting = guid; }
      return 1;
    },
    takeSends: () => {
      if (waiting && near(waiting)) { if (allowed(waiting)) sends.push({ op: 0x13, type: 0, guid: waiting }); waiting = 0; }
      const s = sends; sends = []; return s;
    },
  };
}

test('die, respawn, walk back, loot the corpse: the packets sent', () => {
  const out = [];
  const players = [{ guid: CORPSE, x: 120, y: 100, renderState: { mode: 17 } }, { guid: OTHER_CORPSE, x: 60, y: 100, renderState: { mode: 17 } }];
  let pairs = [], me = { x: 120, y: 100 }, mode = 1;
  const scene = () => ({ ready: true, player: me, run: false, selected: { left: 0, right: 36 }, monsters: [], objects: [], items: [],
    isNpc: () => false, hoverBox: (dx, dy) => Math.abs(dx) < 10 && Math.abs(dy) < 20,
    toScreen: (x, y) => ({ x: x * 2, y }), pointerWorld: (px, py) => ({ x: px / 2, y: py }),
    corpses: realmCorpseViews(pairs, players, HERO) });
  let clock = 0;
  const engine = enginePlayerClick(() => me, (g) => players.find((p) => p.guid === g),
    (g) => realmCorpseViews(pairs, players, HERO).some((c) => c.guid === g && corpseLootable(c)));
  const input = createRealmInput({ transmit: (b) => out.push(Array.from(b)), scene, now: () => clock, move: () => 1, ...engine });

  // Falling: the screen is up, the respawn key is not yet taken.
  mode = 0; assert.ok(deathScreenShown(mode)); assert.equal(respawnAllowed(mode), false);
  // Down: Esc sends the one-byte respawn.
  mode = 17; assert.ok(respawnAllowed(mode));
  assert.ok(input.respawn());
  assert.deepEqual(out, [[0x41]]);

  // In town; the server has named both corpses. Another player's corpse is not a target: the click walks.
  mode = 5; me = { x: 100, y: 100 };
  pairs = [{ owner: HERO, corpse: CORPSE }, { owner: OTHER, corpse: OTHER_CORPSE }];
  clock += 1000; input.leftClick(120, 100);
  assert.deepEqual(out.at(-1), [0x01, 60, 0, 100, 0]);

  // The hero's own corpse, far: walk to the unit, then open it on arrival, once.
  clock += 1000; const hit = input.leftClick(240, 100);
  assert.equal(hit.kind, 'corpse');
  assert.deepEqual(out.at(-1), [0x02, 0, 0, 0, 0, ...le(CORPSE)]);
  const before = out.length;
  input.tick(); assert.equal(out.length, before);
  me = { x: 119, y: 100 };
  input.tick();
  assert.deepEqual(out.at(-1), [0x13, 0, 0, 0, 0, ...le(CORPSE)]);
  input.tick(); assert.equal(out.length, before + 1);

  // Emptied: the server drops the corpse from the list, and the spot is bare ground again.
  pairs = [{ owner: OTHER, corpse: OTHER_CORPSE }];
  clock += 1000; input.leftClick(240, 100);
  assert.deepEqual(out.at(-1), [0x01, 120, 0, 100, 0]);
});

test('a corpse still falling is not opened', () => {
  const out = [];
  const corpses = [{ guid: CORPSE, owner: HERO, own: true, mode: 0, x: 101, y: 100 }];
  const input = createRealmInput({ transmit: (b) => out.push(Array.from(b)), now: () => 0,
    scene: () => ({ ready: true, player: { x: 100, y: 100 }, run: true, selected: { left: 0, right: 0 }, monsters: [], objects: [], items: [],
      isNpc: () => false, hoverBox: () => false, toScreen: (x, y) => ({ x: x * 2, y }), pointerWorld: (px, py) => ({ x: px / 2, y: py }), corpses }) });
  input.leftClick(202, 100);
  assert.deepEqual(out, [[0x03, 101, 0, 100, 0]]);
});

// --- other players' corpses, the hero's death gate, hardcore ----------------------------------------------------
import { Roster } from './bnet-roster.mjs';
import { createGameLife, SAVE_AND_EXIT_PACKET } from './bnet-game-life.mjs';
import { realmCharacterCard } from './realm-character-card.mjs';
import { corpseLootable, closedOnDeath, panelsClosedOnDeath, deathSendAllowed, deathBlocksKey, markCharactersDead,
  createRealmDeath, heroIsDown, TRADE_CANCEL } from './realm-death.mjs';
import { haveEngine, loadEngine } from '../../test/helpers/node-engine.mjs';

// S 0x5B, a player joined: [size u16][id u32 @3][class @7][name 16 @8][level u16 @0x18][party u16 @0x1a]...
const joined = (id, name) => { const b = new Uint8Array(0x23); b[0] = 0x5b; b[1] = 0x23; b.set(le(id), 3); b[7] = 1;
  for (let i = 0; i < name.length; i++) b[8 + i] = name.charCodeAt(i); b[0x18] = 1; b[0x1a] = 0xff; b[0x1b] = 0xff; return b; };
const relation = (from, to, flags) => Uint8Array.from([0x8c, ...le(from), ...le(to), flags & 255, flags >> 8]);
const left = (id) => Uint8Array.from([0x5c, ...le(id)]);
const twoPlayers = () => { const r = new Roster(); r.feed(joined(HERO, 'Hero')); r.feed(joined(OTHER, 'Ann')); r.myId = HERO; return r; };

test('another player\'s corpse: named after its owner, lootable only with the owner\'s permission, gone when the owner leaves', () => {
  const players = [{ guid: CORPSE, x: 120, y: 100, renderState: { mode: 17 } }, { guid: OTHER_CORPSE, x: 60, y: 100, renderState: { mode: 17 } }];
  const pairs = [{ owner: HERO, corpse: CORPSE }, { owner: OTHER, corpse: OTHER_CORPSE }];
  const roster = twoPlayers();
  let views = realmCorpseViews(pairs, players, HERO, roster);
  assert.deepEqual(views.map((v) => [v.guid, v.name, v.own, v.lootable]), [[CORPSE, 'Hero', true, true], [OTHER_CORPSE, 'Ann', false, false]]);
  assert.deepEqual(views.map(corpseLootable), [true, false]);

  // The hero letting Ann loot is the wrong direction: it opens nothing for the hero.
  roster.feed(relation(HERO, OTHER, 1));
  assert.equal(realmCorpseViews(pairs, players, HERO, roster)[1].lootable, false);
  // Ann lets the hero loot (S 0x8C from Ann to the hero, bit 1); hostility alone (8) does not.
  roster.feed(relation(OTHER, HERO, 8));
  assert.equal(realmCorpseViews(pairs, players, HERO, roster)[1].lootable, false);
  roster.feed(relation(OTHER, HERO, 9));
  assert.equal(realmCorpseViews(pairs, players, HERO, roster)[1].lootable, true);
  roster.feed(relation(OTHER, HERO, 0));
  assert.equal(realmCorpseViews(pairs, players, HERO, roster)[1].lootable, false);

  // Ann leaves (S 0x5C): her corpse resolves to no player and is not listed.
  roster.feed(relation(OTHER, HERO, 1));
  roster.feed(left(OTHER));
  views = realmCorpseViews(pairs, players, HERO, roster);
  assert.deepEqual(views.map((v) => v.guid), [CORPSE]);
  // A corpse named for a player the roster never held is not listed either.
  assert.deepEqual(realmCorpseViews([{ owner: 0x999, corpse: OTHER_CORPSE }], players, HERO, roster), []);
});

test('a click on another player\'s corpse: the packets with and without permission', () => {
  const out = [];
  const players = [{ guid: OTHER_CORPSE, x: 60, y: 100, renderState: { mode: 17 } }];
  const pairs = [{ owner: OTHER, corpse: OTHER_CORPSE }];
  const roster = twoPlayers();
  let me = { x: 100, y: 100 }, clock = 0;
  const engine = enginePlayerClick(() => me, (g) => players.find((p) => p.guid === g),
    (g) => realmCorpseViews(pairs, players, HERO, roster).some((c) => c.guid === g && corpseLootable(c)));
  const input = createRealmInput({ transmit: (b) => out.push(Array.from(b)), now: () => clock, move: () => 1, ...engine,
    scene: () => ({ ready: true, player: me, run: false, selected: { left: 0, right: 0 }, monsters: [], objects: [], items: [], playerMode: 5,
      isNpc: () => false, hoverBox: () => false, toScreen: (x, y) => ({ x: x * 2, y }), pointerWorld: (px, py) => ({ x: px / 2, y: py }),
      corpses: realmCorpseViews(pairs, players, HERO, roster) }) });
  // No permission: the corpse is ground, the click walks there.
  input.leftClick(120, 100);
  assert.deepEqual(out, [[0x01, 60, 0, 100, 0]]);
  // Permission: walk to the unit, open it on arrival.
  roster.feed(relation(OTHER, HERO, 1));
  clock += 1000; assert.equal(input.leftClick(120, 100).kind, 'corpse');
  assert.deepEqual(out.at(-1), [0x02, 0, 0, 0, 0, ...le(OTHER_CORPSE)]);
  // Permission withdrawn while walking: nothing is opened on arrival.
  roster.feed(relation(OTHER, HERO, 0));
  me = { x: 61, y: 100 }; const before = out.length;
  input.tick();
  assert.ok(!out.slice(before).some((p) => p[0] === 0x13));
  // Granted again and standing next to it: the open goes out at once.
  roster.feed(relation(OTHER, HERO, 1));
  clock += 1000; input.leftClick(120, 100); input.tick();
  assert.deepEqual(out.at(-1), [0x13, 0, 0, 0, 0, ...le(OTHER_CORPSE)]);
});

test('the engine\'s death rules are the ones the double answers with', { skip: haveEngine() ? false : 'needs the engine build' }, async () => {
  const Module = await loadEngine();
  if (typeof Module._d2_hero_is_down !== 'function') return assert.fail('engine build predates the death-rule exports');
  for (let mode = 0; mode < 20; mode++) assert.equal(Module._d2_hero_is_down(0, mode, 0), DEATH_RULES._d2_hero_is_down(0, mode, 0), `mode ${mode}`);
  assert.equal(Module._d2_hero_is_down(0, 5, 0x10000), 1);   // the dead flag answers whatever the mode
  for (let panel = 0; panel < 0x27; panel++) assert.equal(Module._d2_death_closes_panel(panel), DEATH_RULES._d2_death_closes_panel(panel), `panel ${panel}`);
});

test('the panels the hero\'s death closes', () => {
  assert.ok(closedOnDeath(10));                       // forced, whatever the table holds
  assert.ok(!closedOnDeath(8) && !closedOnDeath(0x15));
  assert.deepEqual(panelsClosedOnDeath(), ['inventory', 'character', 'skillSelect', 'skillTree', 'chat', 'gameMenu', 'automap', 'shop', 'quests',
    'waypoint', 'party', 'trade', 'messageLog', 'stash', 'cube', 'help', 'hireling']);
});

const gate = ({ hardcore = false, trade = null } = {}) => {
  const log = [];
  const death = createRealmDeath({ hardcore: () => hardcore, respawn: () => log.push([0x41]), leave: () => log.push('leave'),
    close: (names) => log.push(['close', names.length]), trade: () => trade, send: (b) => log.push(b), markDead: () => log.push('dead') });
  return { death, log };
};

test('the fall closes the panels once and cancels an open trade', () => {
  const trade = { open: true };
  const { death, log } = gate({ trade });
  assert.equal(death.observe(1), false);
  assert.equal(death.observe(0), true);
  assert.deepEqual(log, [[0x4f, 2, 0, 0, 0, 0, 0], ['close', 17]]);
  assert.deepEqual(log[0], [...TRADE_CANCEL]);
  assert.equal(trade.open, false);
  assert.equal(death.observe(0), false);
  assert.equal(death.observe(17), false);            // lying dead after the fall: nothing more closes
  assert.equal(log.length, 2);
  // Back in town and dying again: it closes again; no trade this time, no cancel.
  death.observe(5);
  assert.equal(death.observe(0), true);
  assert.deepEqual(log.slice(2), [['close', 17]]);
});

test('Esc on the death screen, softcore: nothing while falling, the restart once down', () => {
  const { death, log } = gate();
  assert.equal(death.escape(1), false);               // alive: Esc is not the death screen's
  death.observe(0);
  log.length = 0;
  assert.equal(death.escape(0), true);                // swallowed: no restart, and no menu opens
  assert.deepEqual(log, []);
  death.observe(17);
  assert.equal(death.escape(17), true);
  assert.deepEqual(log, [[0x41]]);
  assert.equal(death.leaving, false);
  // Standing in town again, Esc is an ordinary key.
  death.observe(5);
  assert.equal(death.escape(5), false);
});

test('Esc on the death screen, hardcore: the restart, the leave, the character dead, back to the lobby', () => {
  const sent = [], left = [];
  const life = createGameLife({ expansion: true, onNotice: () => {}, onSend: (b) => sent.push(Array.from(b)), onLeave: (reason) => left.push(reason) });
  life.inGame();
  let characters = [{ name: 'Hard', flags: 0x24 }, { name: 'Soft', flags: 0x20 }];
  const dead = new Set();
  const death = createRealmDeath({ hardcore: () => (realmHeroFlags(characters, 'Hard') & 4) !== 0, respawn: () => sent.push([0x41]),
    leave: () => life.saveAndExit(), close: () => {}, markDead: () => { dead.add('hard'); characters = markCharactersDead(characters, dead); } });
  death.observe(0);
  assert.equal(death.escape(0), true);
  assert.deepEqual(sent, []);                         // still falling: nothing is sent and the game is not left
  death.observe(17);
  assert.equal(death.escape(17), true);
  assert.deepEqual(sent, [[0x41], Array.from(SAVE_AND_EXIT_PACKET)]);
  assert.deepEqual(sent[1], [0x69]);
  assert.deepEqual(left, [0]);                        // an ordinary leave: the lobby, with no notice
  assert.equal(death.leaving, true);
  assert.equal(death.escape(17), true);               // a second Esc sends nothing more
  assert.equal(sent.length, 2);

  // The character list the lobby then reads: the hardcore character is dead, and stays so across a list the
  // server sends that has not caught up; a softcore character of the same list is untouched.
  assert.deepEqual(characters.map((c) => c.flags), [0x2c, 0x20]);
  const fresh = markCharactersDead([{ name: 'HARD', flags: 0x24 }, { name: 'Soft', flags: 0x20 }], dead);
  assert.equal(realmCharacterCard(fresh[0]).dead, true);
  assert.equal(realmCharacterCard(fresh[0]).hardcore, true);
  assert.equal(realmCharacterCard(fresh[1]).dead, false);
  assert.equal(realmHeroFlags(fresh, 'Hard'), 0x24);
  // A softcore character is never marked, whatever the set holds.
  assert.deepEqual(markCharactersDead([{ name: 'Soft', flags: 0x20 }], new Set(['soft'])), [{ name: 'Soft', flags: 0x20 }]);
});

test('dead, the hero sends nothing but the restart; in town again everything flows', () => {
  const out = [];
  let mode = 1;
  const input = createRealmInput({ transmit: (b) => out.push(Array.from(b)), now: () => 0, move: () => 1,
    scene: () => ({ ready: true, player: { x: 100, y: 100 }, playerMode: mode, run: false, selected: { left: 0, right: 0 }, monsters: [], objects: [],
      items: [{ id: 9, mode: 2, gridX: 0, classId: 1 }], isNpc: () => false, hoverBox: () => false,
      toScreen: (x, y) => ({ x: x * 2, y }), pointerWorld: (px, py) => ({ x: px / 2, y: py }), corpses: [] }) });
  for (mode of [0, 17]) {
    assert.equal(input.leftClick(240, 100), false);
    assert.ok(!input.useBelt(0));
    assert.ok(!input.selectSkill(36, false));
    assert.ok(!input.addStat(0, 1));
    assert.deepEqual(out, []);
    assert.equal(input.evidence.last.dead, true);
  }
  mode = 0; assert.equal(input.respawn(), false);     // falling: the restart is not taken yet
  mode = 17; assert.equal(input.respawn(), true);
  assert.deepEqual(out, [[0x41]]);
  // The server stands the hero up in town (mode 5): the next click walks.
  mode = 5;
  assert.ok(input.leftClick(240, 100));
  assert.deepEqual(out.at(-1), [0x01, 120, 0, 100, 0]);

  assert.ok(heroIsDown(0) && heroIsDown(17) && !heroIsDown(5) && !heroIsDown(1));
  assert.ok(deathSendAllowed(5, 'walk') && !deathSendAllowed(17, 'walk') && deathSendAllowed(17, 'respawn') && !deathSendAllowed(0, 'respawn'));
});

test('dead, the keyboard answers Esc and the chat key only', () => {
  for (const mode of [0, 17]) {
    assert.equal(deathBlocksKey(mode, 'Escape', null), false);
    assert.equal(deathBlocksKey(mode, 'Enter', 'Chat'), false);
    assert.equal(deathBlocksKey(mode, 'KeyI', 'Inventory Screen'), true);
    assert.equal(deathBlocksKey(mode, 'Tab', 'Automap'), true);
    assert.equal(deathBlocksKey(mode, 'Digit1', 'Use Belt 1'), true);
    assert.equal(deathBlocksKey(mode, 'F1', 'Skill 1'), true);
    assert.equal(deathBlocksKey(mode, 'KeyQ', null), true);
  }
  assert.equal(deathBlocksKey(5, 'KeyI', 'Inventory Screen'), false);
});
