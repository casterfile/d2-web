import test from 'node:test';
import assert from 'node:assert/strict';
import { createItemTable, createItemTypeTable, createItemStatTable, createAffixTable, createCharStatsTable, createSkillNameTable } from './realm-tables.mjs';
import { createItemText } from './realm-item-text.mjs';

const enc = (t) => new TextEncoder().encode(t);
const files = {
  'data\\global\\excel\\weapons.txt': 'name\tcode\ttype\tlevel\tlevelreq\treqstr\treqdex\tmindam\tmaxdam\tdurability\tspeed\tinvwidth\tinvheight\nHand Axe\thax\taxe\t1\t0\t0\t0\t4\t7\t28\t0\t1\t3\n',
  'data\\global\\excel\\armor.txt': 'name\tcode\ttype\tlevel\tlevelreq\treqstr\treqdex\tminac\tmaxac\tdurability\tspeed\tinvwidth\tinvheight\nCap\tcap\thelm\t1\t0\t0\t0\t3\t5\t12\t0\t2\t2\n',
  'data\\global\\excel\\misc.txt': 'name\tcode\ttype\nGold\tgld\tgold\n',
  'data\\global\\excel\\itemtypes.txt': 'code\tequiv1\tequiv2\nhelm\t\t\naxe\t\t\ngold\t\t\n',
  'data\\global\\excel\\itemstatcost.txt': 'stat\t*id\tsave bits\tsave add\tsave param bits\tvalshift\tdescpriority\tdescfunc\tdescval\tdescstrpos\tdescstrneg\tdescstr2\n' +
    'strength\t0\t8\t32\t0\t0\t100\t1\t1\tModStr1a\tModStr1b\t\n' +
    'armorclass\t31\t11\t10\t0\t0\t50\t3\t1\tModStr3a\t\t\n' +
    'maxdurability\t73\t8\t0\t0\t0\t0\t0\t0\t\t\t\n' +
    'durability\t72\t9\t0\t0\t0\t0\t0\t0\t\t\t\n' +
    'item_numsockets\t194\t4\t0\t0\t0\t0\t0\t0\t\t\t\n',
  'data\\global\\excel\\magicprefix.txt': 'name\tlevelreq\nFirst\t5\nSturdy\t7\n',
  'data\\global\\excel\\magicsuffix.txt': 'name\tlevelreq\nofX\t1\n',
};
const read = (p) => { if (!(p in files)) return enc('name\n'); return enc(files[p]); };
const strings = { cap: 'Cap', ModStr1a: 'to Strength', Sturdy: 'Sturdy', ofX: 'of X', First: 'First' };
const idx = { 3461: 'Defense:', 3457: 'Durability:', 3463: 'of', 3995: ' ', 3453: 'Socketed', 22745: 'Ethereal', 3469: 'Required Level:' };
const lang = { key: (k) => strings[k] ?? '', index: (i) => idx[i] ?? '' };
function bits() {
  const out = []; let n = 0;
  return { put(v, c) { for (let i = 0; i < c; i++, n++) { if ((n & 7) === 0) out.push(0); if (Math.floor(v / 2 ** i) % 2) out[n >> 3] |= 1 << (n & 7); } return this; }, bytes() { return Uint8Array.from(out); } };
}
const fourcc = (c) => { const s = c.padEnd(4, ' '); return (s.charCodeAt(0) | (s.charCodeAt(1) << 8) | (s.charCodeAt(2) << 16) | (s.charCodeAt(3) << 24)) >>> 0; };
const text = () => createItemText({ tables: { items: createItemTable(read), types: createItemTypeTable(read), stats: createItemStatTable(read),
  affixes: createAffixTable(read), chars: createCharStatsTable(read), skills: createSkillNameTable(read) }, lang });

test('an identified magic cap names its affixes and lists AC, durability and the stat', () => {
  const b = bits().put(0x10, 32).put(101, 10).put(0, 3).put(0, 4).put(0, 4).put(0, 4).put(1, 3).put(fourcc('cap'), 32)
    .put(0, 3).put(20, 7).put(4, 4).put(0, 1).put(0, 1).put(2, 11).put(1, 11)
    .put(4 + 10, 11).put(12, 8).put(12, 9)
    .put(0, 9).put(37, 8).put(511, 9);   // strength +5
  const d = text().describe(b.bytes());
  assert.equal(d.name, 'Sturdy Cap of X');
  assert.deepEqual(d.props.slice(0, 2), ['Defense: 4', 'Durability: 12 of 12']);
  assert.ok(d.props.includes('+5 to Strength'));
  assert.equal(d.quality, 4);
});

test('an ear or unknown code gives null', () => {
  assert.equal(text().describe(bits().put(0x10000, 32).put(0, 20).bytes()), null);
  assert.equal(text().describe(bits().put(0x10, 32).put(101, 10).put(3, 3).put(0, 32).put(fourcc('zzz'), 32).bytes()), null);
});

test('a decoded normal wand names its real single-skill bonus and owning class in blue', () => {
  const fixture = { ...files,
    'data\\global\\excel\\weapons.txt': files['data\\global\\excel\\weapons.txt'] + 'Wand\twnd\twand\t1\t0\t0\t0\t2\t4\t15\t0\t1\t2\n',
    'data\\global\\excel\\itemtypes.txt': files['data\\global\\excel\\itemtypes.txt'] + 'wand\t\t\n',
    'data\\global\\excel\\itemstatcost.txt': files['data\\global\\excel\\itemstatcost.txt'] + 'item_singleskill\t107\t3\t0\t9\t0\t81\t27\t0\t\t\t\n',
    'data\\global\\excel\\charstats.txt': 'class\tstrclassonly\namazon\tAmaOnly\nsorceress\tSorOnly\nnecromancer\tNecOnly\n',
    'data\\global\\excel\\skills.txt': 'skill\t*id\tcharclass\tskilldesc\nraise skeleton\t70\tnec\traise skeleton\n',
    'data\\global\\excel\\skilldesc.txt': 'skilldesc\tstr name\nraise skeleton\tRaiseName\n',
  };
  const readKit = (p) => enc(fixture[p] ?? 'name\n');
  const kitLang = { key: (k) => ({ wnd: 'Wand', RaiseName: 'Raise Skeleton', NecOnly: '(Necromancer Only)' }[k] ?? strings[k] ?? ''),
    index: (i) => i === 4003 ? 'to' : idx[i] ?? '' };
  const kitText = createItemText({ tables: { items: createItemTable(readKit), types: createItemTypeTable(readKit), stats: createItemStatTable(readKit),
    affixes: createAffixTable(readKit), chars: createCharStatsTable(readKit), skills: createSkillNameTable(readKit) }, lang: kitLang });
  const b = bits().put(0x10, 32).put(101, 10).put(0, 3).put(4, 4).put(0, 4).put(0, 4).put(1, 3).put(fourcc('wnd'), 32)
    .put(0, 3).put(1, 7).put(2, 4).put(0, 1).put(0, 1).put(15, 8).put(15, 9)
    .put(107, 9).put(70, 9).put(1, 3).put(511, 9);
  const d = kitText.describe(b.bytes()), line = '+1 to Raise Skeleton (Necromancer Only)';
  assert.equal(d.name, 'Wand');
  assert.ok(d.props.includes(line), d.props.join('\n'));
  assert.equal(d.propColours[d.props.indexOf(line)], 3);
});
