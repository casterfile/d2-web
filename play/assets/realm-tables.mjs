// The data tables a joined Battle.net game reads for the items and panels the D2GS server describes by
// number: Weapons/Armor/Misc.txt (class id to art and size), Inventory.txt and Belts.txt (panel geometry).
// Objects are not here: the engine's object units carry their own table row. Each reader takes `read(path)`,
// the MPQ asset reader the shell already holds, and parses the tab-separated file once on first use.

function rows(bytes) {
  const lines = new TextDecoder('latin1').decode(bytes).replace(/\r/g, '').split('\n').filter((l) => l.length);
  const header = lines.shift().split('\t').map((name) => name.trim().toLowerCase());
  return lines.map((line) => {
    const values = line.split('\t'), row = {};
    for (let i = 0; i < header.length; i++) row[header[i]] = (values[i] ?? '').trim();
    return row;
  });
}
const num = (v, fallback = 0) => { const n = Number(v); return Number.isFinite(n) && v !== '' ? n : fallback; };
const fourcc = (code) => { const s = String(code).padEnd(4, ' ');
  return (s.charCodeAt(0) | (s.charCodeAt(1) << 8) | (s.charCodeAt(2) << 16) | (s.charCodeAt(3) << 24)) >>> 0; };

/**
 * Items.txt as the engine numbers it: every Weapons.txt row, then Armor.txt, then Misc.txt, an item's class
 * id being its position in that run (the id the D2GS item bitstream's four-character code resolves to).
 */
export function createItemTable(read) {
  let byClass = null, byCode = null;
  function load() {
    if (byClass) return;
    byClass = []; byCode = new Map();
    for (const file of ['weapons', 'armor', 'misc']) {
      for (const row of rows(read(`data\\global\\excel\\${file}.txt`))) {
        if (!row.code) continue;   // the "Expansion" separator row has no code
        const entry = { classId: byClass.length, kind: file, code: row.code, name: row.name, type: row.type,
          flippyFile: row.flippyfile ?? '', invFile: row.invfile ?? '',
          invWidth: num(row.invwidth, 1) || 1, invHeight: num(row.invheight, 1) || 1,
          wclass: row.wclass ?? '', twoHandedWclass: row['2handedwclass'] ?? '', twoHanded: row['2handed'] === '1',
          belt: file === 'armor' ? num(row.belt, -1) : -1,
          // The columns an item's tooltip reads (assets/realm-item-text.mjs).
          level: num(row.level), levelReq: num(row.levelreq), reqStr: num(row.reqstr), reqDex: num(row.reqdex),
          minAc: num(row.minac), maxAc: num(row.maxac), minDam: num(row.mindam), maxDam: num(row.maxdam),
          minDam2: num(row['2handmindam']), maxDam2: num(row['2handmaxdam']), minMis: num(row.minmisdam), maxMis: num(row.maxmisdam),
          durability: num(row.durability), noDurability: row.nodurability === '1', stackable: row.stackable === '1',
          speed: num(row.speed),
          // The bytes D2Client+0x29920 reads (Items.txt useable, unique, quest columns; an empty cell is 0).
          useable: num(row.useable), unique: num(row.unique), quest: num(row.quest) };
        byClass.push(entry); byCode.set(fourcc(row.code), entry);
      }
    }
  }
  return {
    byCode(code) { load(); return byCode.get(code >>> 0) ?? null; },
    byClass(classId) { load(); return byClass[classId] ?? null; },
  };
}

/** Levels.txt: the Rain byte of a level (the one the single-player engine's weather reads), by level id. */
export function createLevelTable(read) {
  let rain = null, types = null;
  return {
    /** Levels.txt LevelType: the level type the automap's AutoMap.txt rows are chosen by; -1 for an unknown level. */
    levelType(levelId) {
      if (!types) {
        types = new Map();
        for (const row of rows(read('data\\global\\excel\\levels.txt'))) if (row.id !== '') types.set(num(row.id, -1), num(row.leveltype, -1));
      }
      return types.has(levelId) ? types.get(levelId) : -1;
    },
    rain(levelId) {
      if (!rain) {
        rain = new Map();
        for (const row of rows(read('data\\global\\excel\\levels.txt'))) if (row.id !== '') rain.set(num(row.id, -1), num(row.rain));
      }
      return rain.has(levelId) ? rain.get(levelId) : -1;
    },
  };
}

const PLAYER_CLASS_NAME = ['amazon', 'sorceress', 'necromancer', 'paladin', 'barbarian', 'druid', 'assassin'];
const INVENTORY_COLUMNS = ['invleft', 'invright', 'invtop', 'invbottom', 'gridx', 'gridy', 'gridleft', 'gridright',
  'gridtop', 'gridbottom', 'gridboxwidth', 'gridboxheight'];
const COMPONENT_PREFIX = ['rarm', 'torso', 'larm', 'head', 'neck', 'rhand', 'lhand', 'belt', 'feet', 'gloves'];

/**
 * Inventory.txt: the twelve grid numbers of a named row (what the single-player engine hands the panel
 * readers), and for the player's own row also the sixty equipment-slot numbers (ten slots of
 * left/right/top/bottom/width/height). `null` for a row the file does not have.
 */
const INVENTORY_RECORDS_PER_RESOLUTION = 16;
export function createInventoryTable(read) {
  let table = null, ordered = null;
  const list = () => (ordered ??= rows(read('data\\global\\excel\\inventory.txt')));
  const load = () => (table ??= new Map(list().map((r) => [r.class.toLowerCase(), r])));
  return {
    // The twelve grid numbers of a row by its record number, as the engine reads a panel's row: the file holds
    // each record twice, the 640x480 set and then the 800x600 set, so the row is `record + 16 * higherRes`.
    gridAt(record, higherRes = 1) {
      const row = list()[record + (higherRes ? INVENTORY_RECORDS_PER_RESOLUTION : 0)];
      return row ? INVENTORY_COLUMNS.map((c) => num(row[c])) : null;
    },
    grid(name) {
      const row = load().get(String(name).toLowerCase());
      return row ? INVENTORY_COLUMNS.map((c) => num(row[c])) : null;
    },
    components(name) {
      const row = load().get(String(name).toLowerCase());
      if (!row) return null;
      return COMPONENT_PREFIX.flatMap((p) => ['left', 'right', 'top', 'bottom', 'width', 'height'].map((s) => num(row[p + s])));
    },
    // The row the panel for a class reads: the expansion's own (`Amazon2`...) or the classic one.
    playerRow(classId, expansion = true) {
      const base = PLAYER_CLASS_NAME[classId];
      if (!base) return null;
      return expansion && load().has(base + '2') ? base + '2' : base;
    },
  };
}

/** Belts.txt: record index -> the box rectangles (what d2_session_belt_boxes returns). Row 2 is the beltless record. */
const BELTS_PER_RESOLUTION = 7;
export function createBeltTable(read) {
  let table = null;
  const load = () => (table ??= rows(read('data\\global\\excel\\belts.txt')));
  return {
    // Belts.txt holds seven rows per resolution, the 640x480 set first; retail's GetGlobalBeltSlotPosition
    // reads row `higherRes * 7 + record`, and the 800x600 HUD takes the second set.
    boxes(record, higherRes = 1) {
      const row = load()[higherRes * BELTS_PER_RESOLUTION + record];
      if (!row) return null;
      const n = num(row.numboxes), boxes = [];
      for (let i = 1; i <= n; i++) boxes.push({ left: num(row[`box${i}left`]), right: num(row[`box${i}right`]),
        top: num(row[`box${i}top`]), bottom: num(row[`box${i}bottom`]) });
      return boxes;
    },
  };
}

/** MonStats.txt by class id (the row's position): whether the unit is a town person the player talks to. */
export function createMonsterTable(read) {
  let list = null;
  const load = () => (list ??= rows(read('data\\global\\excel\\monstats.txt')));
  return {
    isNpc(classId) { const row = load()[classId]; return Boolean(row) && (row.npc === '1' || row.interact === '1'); },
    // The columns the client sorts a hovered unit by (assets/realm-combat.mjs createRealmUnits).
    row(classId) {
      const row = load()[classId];
      return row ? { npc: row.npc === '1', interact: row.interact === '1', killable: row.killable === '1', align: num(row.align), named: Boolean(row.namestr) } : null;
    },
  };
}

/** ItemTypes.txt: whether a type code is, or descends from, another (the engine's ITEMS_CheckItemTypeId on the code). */
export function createItemTypeTable(read) {
  let table = null;
  const load = () => (table ??= new Map(rows(read('data\\global\\excel\\itemtypes.txt')).filter((r) => r.code)
    .map((r) => [r.code, [r.equiv1, r.equiv2].filter(Boolean)])));
  return {
    is(type, wanted) {
      const seen = new Set(), walk = (t) => t === wanted || (!seen.has(t) && seen.add(t) && (load().get(t) ?? []).some(walk));
      return walk(type);
    },
  };
}

/**
 * ItemStatCost.txt by stat id: the bit widths the item bitstream stores a stat with and the columns the
 * property text reads (descpriority, descfunc, descval, the three string keys, the dgrp group, op, op param,
 * op base, ValShift). The string columns are KEYS of the game's string table, read through its key lookup.
 */
export function createItemStatTable(read) {
  let list = null;
  function load() {
    if (list) return list;
    list = [];
    rows(read('data\\global\\excel\\itemstatcost.txt')).forEach((row, index) => {
      if (!row.stat) return;
      const id = num(row['*id'] ?? row.id, index);
      list[id] = { id, name: row.stat, saveBits: num(row['save bits']), saveAdd: num(row['save add']), paramBits: num(row['save param bits']),
        valShift: num(row.valshift), priority: num(row.descpriority), func: num(row.descfunc), val: num(row.descval),
        strPos: row.descstrpos ?? '', strNeg: row.descstrneg ?? '', str2: row.descstr2 ?? '',
        group: num(row.dgrp), groupFunc: num(row.dgrpfunc), groupVal: num(row.dgrpval),
        groupStrPos: row.dgrpstrpos ?? '', groupStrNeg: row.dgrpstrneg ?? '', groupStr2: row.dgrpstr2 ?? '',
        op: num(row.op), opParam: num(row['op param']), opBase: num(row['op base'], -1) };
    });
    return list;
  }
  return { get(id) { return load()[id] ?? null; }, get count() { return load().length; } };
}

/**
 * MagicPrefix/MagicSuffix/RarePrefix/RareSuffix/UniqueItems/SetItems.txt: the string key an affix id names.
 * The magic affix ids the item bitstream holds count each file's data rows from 1 (0 is "none"); rare ids
 * count the suffix rows then the prefix rows as one array from 1; unique and set ids count from 0. Magic affixes also give the level requirement they raise.
 */
export function createAffixTable(read) {
  const cache = new Map();
  const file = (name) => { if (!cache.has(name)) cache.set(name, rows(read(`data\\global\\excel\\${name}.txt`)).filter((r) => (r.name ?? r.index) !== 'Expansion')); return cache.get(name); };   // the Expansion divider row is not a record
  const at = (name, i) => file(name)[i] ?? null;
  return {
    magicPrefix(id) { const r = id > 0 ? at('magicprefix', id - 1) : null; return r ? { key: r.name, levelReq: num(r.levelreq) } : null; },
    magicSuffix(id) { const r = id > 0 ? at('magicsuffix', id - 1) : null; return r ? { key: r.name, levelReq: num(r.levelreq) } : null; },
    // Rare ids index RareSuffix.txt's rows then RarePrefix.txt's as one array (the engine's combined table), from 1.
    rare(id) { const suffixes = file('raresuffix'), r = id > 0 ? (id <= suffixes.length ? suffixes[id - 1] : at('rareprefix', id - 1 - suffixes.length)) : null; return r ? { key: r.name } : null; },
    unique(id) { const r = at('uniqueitems', id); return r ? { key: r.index, levelReq: num(r['lvl req']) } : null; },
    setItem(id) { const r = at('setitems', id); return r ? { key: r.index, levelReq: num(r['lvl req']) } : null; },
  };
}

/** CharStats.txt: the string keys the class-skill property lines name (StrAllSkills, StrSkillTab1-3, StrClassOnly), by class id. */
export function createCharStatsTable(read) {
  let table = null;
  const load = () => (table ??= rows(read('data\\global\\excel\\charstats.txt')).filter((r) => r.class && r.class !== 'Expansion'));
  return {
    byClass(classId) {
      const r = load()[classId];
      return r ? { allSkills: r.strallskills ?? '', tabs: [r.strskilltab1 ?? '', r.strskilltab2 ?? '', r.strskilltab3 ?? ''], classOnly: r.strclassonly ?? '' } : null;
    },
  };
}

/** Skills.txt + SkillDesc.txt: the string key naming a skill id (what the charged-skill property line prints). */
export function createSkillNameTable(read) {
  let skills = null, descs = null;
  const load = () => {
    if (skills) return;
    skills = new Map(rows(read('data\\global\\excel\\skills.txt')).map((r) => [num(r['*id'] ?? r.id, -1), r]));
    descs = new Map(rows(read('data\\global\\excel\\skilldesc.txt')).map((r) => [r.skilldesc, r]));
  };
  return {
    nameKey(id) { load(); const desc = descs.get(skills.get(id)?.skilldesc); return desc?.['str name'] ?? ''; },
    classId(id) { load(); return PLAYER_CLASS_NAME.findIndex((name) => name.slice(0, 3) === skills.get(id)?.charclass); },
    /** The skill's mana at a level in 1/256 units, as the hover plate reads it (D2Common 0x4EA00 with the plate's minmana floor); 0 for an unknown skill. */
    manaCost(id, level) {
      load();
      const r = skills.get(id);
      if (!r || !(level >= 1)) return 0;
      const cost = (num(r.mana) + (level - 1) * num(r.lvlmana)) * 2 ** num(r.manashift);
      return Math.max(0, cost, num(r.minmana) * 256);
    },
  };
}
