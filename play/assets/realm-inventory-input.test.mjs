// A joined game's inventory panel, from the server's packets to the client's: the item packets fill the world,
// the panel's records and layout come from it, and each click sends one packet and changes nothing until the
// server answers.
import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { createRealmWorld, PAGE } from './realm-world.mjs';
import { createItemTable, createInventoryTable, createBeltTable } from './realm-tables.mjs';
import { realmViewSnapshot, realmFrameReads } from './realm-client-view.mjs';
import { createRealmInput } from './realm-input.mjs';
import { realmInventoryClick } from './realm-storage-input.mjs';
import { inventoryLayoutFromEngine, inventoryItemAt, weaponTabHit, AMAZON2_INVENTORY_LAYOUT as PANEL } from './inventory-panel.mjs';

const enc = (text) => new TextEncoder().encode(text);
// Inventory.txt: the grid and three body slots (right arm 4, head 1, left arm 5) of the player's row.
const SLOT = { rarm: [420, 476, 110, 220], head: [530, 588, 60, 118], larm: [650, 706, 110, 220] };
const slotCols = Object.keys(SLOT).flatMap((p) => ['left', 'right', 'top', 'bottom'].map((s) => p + s));
const files = {
  'data\\global\\excel\\weapons.txt': 'name\tcode\ttype\tflippyfile\tinvfile\tinvwidth\tinvheight\twclass\t2handedwclass\t2handed\nHand Axe\thax\taxe\tflpax\tinvax\t1\t3\t1hs\t\t0\nWar Sword\twsd\tswor\tflpws\tinvws\t1\t3\t1hs\t\t0\n',
  'data\\global\\excel\\armor.txt': 'name\tcode\ttype\tflippyfile\tinvfile\tinvwidth\tinvheight\tbelt\nCap\tcap\thelm\tflpcap\tinvcap\t2\t2\t\n',
  'data\\global\\excel\\misc.txt': 'name\tcode\ttype\tflippyfile\tinvfile\tinvwidth\tinvheight\nHealing Potion\thp1\thpot\tflphp1\tinvhp1\t1\t1\n',
  'data\\global\\excel\\objects.txt': 'Name\tId\tToken\nBarrel\t0\tBA\n',
  'data\\global\\excel\\inventory.txt': `class\tinvleft\tinvright\tinvtop\tinvbottom\tgridx\tgridy\tgridleft\tgridright\tgridtop\tgridbottom\tgridboxwidth\tgridboxheight\t${slotCols.join('\t')}\n`
    + `Amazon2\t400\t720\t60\t501\t10\t4\t419\t709\t315\t431\t29\t29\t${Object.values(SLOT).flat().join('\t')}\n`,
  'data\\global\\excel\\belts.txt': 'name\tnumboxes\tbox1left\tbox1right\tbox1top\tbox1bottom\tbox2left\tbox2right\tbox2top\tbox2bottom\nA\t4\t0\t10\t0\t10\t10\t20\t0\t10\nB\t4\t0\t10\t0\t10\t10\t20\t0\t10\nC\t0\n',
};
const read = (path) => { const f = files[path]; if (f === undefined) throw new Error(path); return enc(f); };
const tables = () => ({ items: createItemTable(read), inventory: createInventoryTable(read), belt: createBeltTable(read),
  inventoryLayoutFromEngine, stashGeometry: (g) => g });
const fourcc = (code) => { const c = code.padEnd(4, ' '); return (c.charCodeAt(0) | (c.charCodeAt(1) << 8) | (c.charCodeAt(2) << 16) | (c.charCodeAt(3) << 24)) >>> 0; };

function bits() {
  const out = []; let n = 0;
  return { put(value, count) { for (let i = 0; i < count; i++, n++) { if ((n & 7) === 0) out.push(0); if (Math.floor(value / 2 ** i) % 2) out[n >> 3] |= 1 << (n & 7); } return this; },
    bytes() { return Uint8Array.from(out); } };
}
// The item bitstream head: flags, version, mode, then the ground position or body location, cell and page, then the code.
function itemBits({ mode, x = 0, y = 0, bodyLoc = 0, page = 0, code }) {
  const b = bits().put(0x10, 32).put(101, 10).put(mode, 3);
  if (mode === 3 || mode === 5) b.put(x, 16).put(y, 16); else b.put(bodyLoc, 4).put(x, 4).put(y, 4).put((mode === 0 ? page + 1 : 0) & 7, 3);   // only a stored item carries a page
  return b.put(fourcc(code), 32).put(0, 3).put(30, 7).put(2, 4).bytes();
}
const OWNER = 0x01020304;
const u32le = (v) => [v & 255, (v >>> 8) & 255, (v >>> 16) & 255, (v >>> 24) & 255];
const world9c = (guid, action, spec) => Uint8Array.from([0x9c, action, 0, 0, ...u32le(guid), ...itemBits(spec)]);
const owned9d = (guid, action, spec) => Uint8Array.from([0x9d, action, 0, 0, ...u32le(guid), 0, ...u32le(OWNER), ...itemBits(spec)]);
const setStat32 = (stat, value) => Uint8Array.from([0x1f, stat, ...u32le(value)]);
const hex = (bytes) => Array.from(bytes, (b) => b.toString(16).padStart(2, '0')).join('');

// The panel renderer's hit tests over the layout the realm reads hand it (the renderer itself needs the game's art).
function panelOver(layout) {
  const slotRect = (bodyLoc) => { const r = layout.slots[bodyLoc]; return r ? { left: r.left, top: r.top, width: r.right - r.left, height: r.bottom - r.top } : null; };
  return {
    contains: (x, y) => x >= PANEL.screenX && x < PANEL.screenX + PANEL.width && y >= PANEL.screenY && y < PANEL.screenY + PANEL.height,
    itemAt: (x, y, _w, _h, state) => inventoryItemAt(state?.items, x, y, layout),
    weaponTabAt: (x, y) => weaponTabHit(x, y, slotRect),
    slotAt(x, y, _w, _h, state) {
      for (let bodyLoc = 1; bodyLoc <= 10; bodyLoc++) { const r = slotRect(bodyLoc);
        if (r && x >= r.left && x <= r.left + r.width && y >= r.top && y <= r.top + r.height) return { bodyLoc, item: state.items.find((i) => i.mode === 1 && i.bodyLoc === bodyLoc) || null }; }
      return null;
    },
    cellAt(x, y, _w, _h, item) {
      const g = layout.grid;
      if (x < g.left || y < g.top || x >= g.left + g.columns * g.cellWidth || y >= g.top + g.rows * g.cellHeight) return null;
      const w = item?.invWidth || 1, h = item?.invHeight || 1;
      return { x: Math.max(0, Math.min(g.columns - w, Math.round((x - g.left) / g.cellWidth - w / 2))), y: Math.max(0, Math.min(g.rows - h, Math.round((y - g.top) / g.cellHeight - h / 2))) };
    },
  };
}

function joined() {
  const t = tables(), world = createRealmWorld(t); world.setOwner(OWNER);
  const feed = (...packets) => packets.forEach((p) => world.feed(p, (b) => b.length));
  const sent = []; let clock = 0, tabs = 0;
  const input = createRealmInput({ transmit: (bytes) => { sent.push(hex(bytes)); }, scene: () => ({ ready: true, player: { x: 10, y: 20 } }), now: () => (clock += 1000) });
  const layout = realmFrameReads(null, world, t, 0).inventoryLayout;
  const snapshot = () => realmViewSnapshot({ base: { player: {} }, resources: null, level: 1, actor: () => null, frame: 0, world });
  const click = (x, y, button = 0, over = {}) => realmInventoryClick({ width: 800, height: 600, snapshot: snapshot(), cursorId: world.cursorId, input,
    visible: true, renderer: panelOver(layout), openPanelAt: () => null, onWeaponTab: () => { tabs++; }, ...over }, x, y, button);
  const by = () => Object.fromEntries(snapshot().items.map((i) => [i.id, i]));
  const cell = (cx, cy) => [layout.grid.left + cx * 29 + 14, layout.grid.top + cy * 29 + 14];
  return { world, feed, sent, click, by, cell, layout, tabs: () => tabs };
}
// The join stream's items: two carried weapons, a worn cap, a belt potion, and the gold stats.
const joinStream = [
  world9c(10, 4, { mode: 0, x: 2, y: 0, page: 0, code: 'wsd' }),
  world9c(11, 4, { mode: 0, x: 5, y: 0, page: 0, code: 'hax' }),
  owned9d(12, 6, { mode: 1, bodyLoc: 1, code: 'cap' }),
  world9c(13, 14, { mode: 2, x: 1, code: 'hp1' }),
  setStat32(14, 4321), setStat32(15, 70000),
];

test('the join stream fills the panel: carried, worn and belt items, gold, and the layout of the player\'s row', () => {
  const g = joined(); g.feed(...joinStream);
  const by = g.by();
  assert.deepEqual([by[10].mode, by[10].invPage, by[10].gridX, by[10].gridY, by[10].invFile, by[10].invWidth, by[10].invHeight], [0, PAGE.INVENTORY, 2, 0, 'invws', 1, 3]);
  assert.deepEqual([by[11].invPage, by[11].gridX, by[11].invFile], [PAGE.INVENTORY, 5, 'invax']);
  assert.deepEqual([by[12].mode, by[12].bodyLoc, by[12].invFile], [1, 1, 'invcap']);
  assert.deepEqual([by[13].invPage, by[13].gridX], [PAGE.NONE, 1]);
  assert.deepEqual([g.world.gold, g.world.stashGold, g.world.cursorId], [4321, 70000, 0]);
  assert.deepEqual(g.layout.grid, { left: 419, top: 315, columns: 10, rows: 4, cellWidth: 29, cellHeight: 29 });
  assert.deepEqual(g.layout.slots, { 4: { left: 420, right: 476, top: 110, bottom: 220 }, 1: { left: 530, right: 588, top: 60, bottom: 118 }, 5: { left: 650, right: 706, top: 110, bottom: 220 } });
});

test('the identify cursor: S2C 0x3F arms it, the next press on an item is 0x27 [clicked][armed], 0x3F 0xFF/0xFFFF disarms (D2Client 0x86A40, 0x95FE0)', () => {
  const g = joined(); g.feed(...joinStream);
  const last = () => g.sent[g.sent.length - 1];
  g.feed(Uint8Array.from([0x3f, 1, 0x0b, 0, 0, 0, 0xff, 0xff]));
  assert.deepEqual(g.world.useItem, { id: 11, kind: 1 });
  assert.equal(g.click(...g.cell(2, 1), 0, { armed: 11 }), true);
  assert.equal(last(), '27' + '0a000000' + '0b000000');
  const before = g.sent.length;
  assert.equal(g.click(...g.cell(0, 0), 0, { armed: 11 }), true);          // an empty cell: nothing goes out
  assert.equal(g.sent.length, before);
  g.feed(Uint8Array.from([0x3f, 0xff, 0x0b, 0, 0, 0, 0xff, 0xff]));
  assert.equal(g.world.useItem, null);
});

test('each panel click sends the client packet and the panel changes on the server\'s answer', () => {
  const g = joined(); g.feed(...joinStream);
  const last = () => g.sent[g.sent.length - 1];

  assert.equal(g.click(...g.cell(5, 1), 2), true);                       // right click a carried item: use it at the player
  assert.equal(last(), '20' + '0b000000' + '0a000000' + '14000000');

  assert.equal(g.click(...g.cell(2, 1)), true);                          // lift the sword
  assert.equal(last(), '19' + '0a000000');
  assert.equal(g.world.cursorId, 0, 'nothing moves before the server answers');
  g.feed(owned9d(10, 5, { mode: 4, code: 'wsd' }));
  assert.equal(g.world.cursorId, 10);

  assert.equal(g.click(...g.cell(8, 1)), true);                          // put it down in an empty column
  assert.equal(last(), '18' + '0a000000' + '08000000' + '00000000' + '00000000');
  g.feed(world9c(10, 4, { mode: 0, x: 8, y: 0, page: 0, code: 'wsd' }));
  assert.deepEqual([g.world.cursorId, g.by()[10].gridX], [0, 8]);

  g.click(...g.cell(5, 1)); assert.equal(last(), '19' + '0b000000');     // lift the axe
  g.feed(owned9d(11, 5, { mode: 4, code: 'hax' }));
  assert.equal(g.click(...g.cell(8, 1)), true);                          // onto the sword: swap with it
  assert.equal(last(), '1f' + '0b000000' + '0a000000' + '08000000' + '00000000');
  g.feed(world9c(11, 4, { mode: 0, x: 8, y: 0, page: 0, code: 'hax' }), owned9d(10, 5, { mode: 4, code: 'wsd' }));
  assert.deepEqual([g.world.cursorId, g.by()[11].gridX, g.by()[10].mode], [10, 8, 4]);

  assert.equal(g.click(430, 150), true);                                 // the empty right arm: wear the sword
  assert.equal(last(), '1a' + '0a000000' + '04000000');
  g.feed(owned9d(10, 6, { mode: 1, bodyLoc: 4, code: 'wsd' }));
  assert.deepEqual([g.world.cursorId, g.by()[10].mode, g.by()[10].bodyLoc], [0, 1, 4]);

  assert.equal(g.click(540, 70), true);                                  // the worn cap: take it off
  assert.equal(last(), '1c' + '0100');
  g.feed(owned9d(12, 8, { mode: 4, code: 'cap' }));
  assert.equal(g.world.cursorId, 12);

  assert.equal(g.click(100, 100), true);                                 // outside every panel: drop it
  assert.equal(last(), '17' + '0c000000');
  g.feed(world9c(12, 2, { mode: 3, x: 100, y: 200, code: 'cap' }));
  assert.deepEqual([g.world.cursorId, g.by()[12].landed, g.by()[12].invPage], [0, true, 255]);

  g.click(...g.cell(8, 1)); assert.equal(last(), '19' + '0b000000');     // lift the axe again
  g.feed(owned9d(11, 5, { mode: 4, code: 'hax' }));
  assert.equal(g.click(430, 150), true);                                 // onto the worn sword: swap with it
  assert.equal(last(), '1d' + '0b000000' + '04000000');
  g.feed(owned9d(11, 6, { mode: 1, bodyLoc: 4, code: 'hax' }), owned9d(10, 8, { mode: 4, code: 'wsd' }));
  assert.deepEqual([g.world.cursorId, g.by()[11].bodyLoc, g.by()[10].mode], [10, 4, 4]);
  g.feed(world9c(10, 4, { mode: 0, x: 0, y: 0, page: 0, code: 'wsd' }));

  assert.equal(g.click(430, 100), true);                                 // the tab above the right arm: the other weapon set
  assert.deepEqual([last(), g.tabs()], ['60', 1]);
  assert.equal(g.sent.length, 11, 'one packet per click');
});

test('clicks that are not the panel\'s send nothing', () => {
  const g = joined(); g.feed(...joinStream);
  assert.equal(g.click(100, 100), false, 'outside the panel with an empty cursor');
  assert.equal(g.click(...g.cell(2, 1), 0, { visible: false }), false, 'the panel is closed');
  assert.equal(g.click(...g.cell(0, 3)), true, 'an empty cell is still the panel\'s click');
  g.feed(owned9d(10, 5, { mode: 4, code: 'wsd' }));
  assert.equal(g.click(100, 100, 0, { openPanelAt: () => 'stash' }), false, 'a held item over another open panel is not dropped');
  assert.equal(g.click(...g.cell(5, 1)), true, 'a held item over one carried item swaps with it');
  assert.deepEqual(g.sent, ['1f' + '0a000000' + '0b000000' + '05000000' + '00000000']);
});

test('a game joined from the title screen builds the inventory, stash and cube panels', () => {
  const shell = readFileSync(new URL('../gameplay-shell.mjs', import.meta.url), 'utf8');
  const realmChrome = shell.slice(shell.indexOf('function ensureRealmChrome()'), shell.indexOf('async function syncRealmScene()'));
  assert.match(realmChrome, /buildChromePanels\(false, /);
  assert.match(realmChrome, /buildStoragePanels\(\)/);
  const click = shell.slice(shell.indexOf('function realmInventoryClick('), shell.indexOf('function realmStorageClickAt('));
  assert.match(click, /realmInventoryPanelClick\(\{[^}]*renderer: inventoryPanelRenderer/s, 'the shell routes the click through the panel it draws');
});

test('the pointer press has one input path: no engine item, object, warp, NPC or corpse branch beside the packets', () => {
  const shell = readFileSync(new URL('../gameplay-shell.mjs', import.meta.url), 'utf8');
  const press = shell.slice(shell.indexOf("canvas.addEventListener('pointerdown'"), shell.indexOf("globalThis.addEventListener('pointerup'"));
  for (const gone of ['session.objectClick', 'd2_session_command_recover_corpse', 'd2_session_command_warp_interact', "itemCommand('lift'", "itemCommand('drop'", 'talkToNpc(npcHit'])
    assert.equal(press.includes(gone), false, `${gone} is not in the press handler`);
  assert.match(press, /clientView\(\)\.commands\.panelClick\(x, y, e\)/);
  assert.match(shell, /canvas\.addEventListener\('contextmenu', \(e\) => \{ e\.preventDefault\(\); \}\)/);
});
