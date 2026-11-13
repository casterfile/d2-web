// The tooltip text of an item the D2GS server sent (0x9C/0x9D): its saved-item bitstream decoded the way
// ITEMS_DecodeItemBitstreamComplete / ITEMS_ReadStatFromItemBitstream read it (1.13c item format, no 'JM'
// header, no seed), then worded the way the single-player tooltip is (d2_session_item_title and
// d2_session_item_lines, src/core/monspawn/monspawn_check.cpp): the same line order, the same strings, the same
// ItemStatCost descpriority/descfunc/descval/descstr*/dgrp rules. The output goes to the same renderer path
// (assets/item-tooltip.mjs: parseItemLines, itemTooltipSpec).
//
// `tables` is { items, types, stats, affixes, chars, skills } (assets/realm-tables.mjs), `lang` is
// { key(stringKey), index(stringIndex) } over the game's string tables, `heroStat(id)` (optional) answers the
// hero's stat for the per-level property lines; a line that needs it and cannot have it is left out.

import { bitReader } from './realm-world.mjs';

const FLAG = Object.freeze({ IDENTIFIED: 0x10, SOCKETED: 0x800, EAR: 0x10000, COMPACT: 0x200000, ETHEREAL: 0x400000,
  PERSONALIZED: 0x1000000, RUNEWORD: 0x4000000 });
const QUALITY = Object.freeze({ INFERIOR: 1, NORMAL: 2, SUPERIOR: 3, MAGIC: 4, SET: 5, RARE: 6, UNIQUE: 7, CRAFT: 8, TEMPERED: 9 });
const STAT = Object.freeze({ ENERGY: 0, MINDAMAGE: 21, MAXDAMAGE: 22, SECONDARY_MIN: 23, SECONDARY_MAX: 24, ARMORCLASS: 31,
  ARMOR_PERCENT: 16, MAXDAMAGE_PERCENT: 17, MINDAMAGE_PERCENT: 18, FIREMIN: 48, LIGHTMIN: 50, MAGICMIN: 52, COLDMIN: 54, POISONMIN: 57,
  MAXDURABILITY: 73, DURABILITY: 72, QUANTITY: 70, NUMSOCKETS: 194, THROW_MIN: 159, THROW_MAX: 160, INDESTRUCTIBLE: 152,
  REQ_PERCENT: 91, LEVELREQ: 92 });
// Stats the stream stores as several values in a row: [first stat, ...stats it carries].
const MULTI = new Map([[17, [17, 18]], [48, [48, 49]], [50, [50, 51]], [52, [52, 53]], [54, [54, 55, 56]], [57, [57, 58, 59]]]);
const BEFORE_COMPLETE = 0;
const trim = (code) => String.fromCharCode(code & 255, (code >>> 8) & 255, (code >>> 16) & 255, (code >>> 24) & 255).replace(/[\0 ]+$/g, '');

/**
 * Decode an item packet's bitstream (the bytes from its flags on). Returns null for an ear or a stream that
 * does not name a known item; `stats` is the merged stat list (key "id:layer" -> value) of the always-active
 * lists (the item's own and a runeword's), `statsRead` false when the stream carries none (an unidentified item).
 */
export function decodeItemStream(bytes, { items, types, stats: statTable }) {
  const r = bitReader(bytes);
  const flags = r.read(32);
  r.read(10);   // format version
  const mode = r.read(3);
  if (mode === 3 || mode === 5) r.read(32); else r.read(15);
  if (flags & FLAG.EAR) return null;
  const code = r.read(32) >>> 0;
  const entry = items.byCode(code);
  if (!entry) return null;
  const item = { flags, code, entry, compact: Boolean(flags & FLAG.COMPACT), quality: QUALITY.NORMAL, level: 1, sockets: 0,
    magicPrefix: 0, magicSuffix: 0, magicPrefixes: [], magicSuffixes: [], autoAffix: 0, rarePrefix: 0, rareSuffix: 0, fileIndex: -1,
    ac: 0, maxDurability: 0, durability: 0, quantity: 0, stats: new Map(), statsRead: false };
  if (item.compact) return item;
  const saved = (id) => statTable.get(id);
  const bitsOf = (id) => saved(id)?.saveBits ?? 0, addOf = (id) => saved(id)?.saveAdd ?? 0;
  const typeIs = (wanted) => types.is(entry.type, wanted);
  r.read(3);    // the socket count this header carries (the 0x800 flag's socket stat is read below)
  item.level = Math.max(1, r.read(7));
  item.quality = r.read(4);
  if (r.read(1)) r.read(3);    // alternate inventory graphic
  if (r.read(1)) item.autoAffix = r.read(11);
  const identified = Boolean(flags & FLAG.IDENTIFIED);
  switch (item.quality) {
    case QUALITY.INFERIOR: case QUALITY.SUPERIOR: item.fileIndex = r.read(3); break;
    case QUALITY.NORMAL:
      if (typeIs('char') && identified) { if (r.read(1)) item.magicPrefix = r.read(11); else item.magicSuffix = r.read(11); }
      if (typeIs('body') && !typeIs('play')) item.fileIndex = r.read(10);
      if (typeIs('scrl') || typeIs('book')) item.magicSuffix = r.read(5);
      break;
    case QUALITY.RARE: case QUALITY.CRAFT:
      if (identified) { item.rarePrefix = r.read(8); item.rareSuffix = r.read(8); }
      for (let i = 0; i < 3; i++) {
        item.magicPrefixes.push(r.read(1) ? r.read(11) : 0);
        item.magicSuffixes.push(r.read(1) ? r.read(11) : 0);
      }
      break;
    case QUALITY.TEMPERED: if (identified) { item.rarePrefix = r.read(8); item.rareSuffix = r.read(8); } break;
    case QUALITY.MAGIC: if (identified) { item.magicPrefix = r.read(11); item.magicSuffix = r.read(11); } break;
    case QUALITY.UNIQUE: if (identified) item.fileIndex = r.read(12); break;
    case QUALITY.SET: if (identified) item.fileIndex = r.read(12); break;
    default: break;
  }
  if (flags & FLAG.RUNEWORD) item.runeword = r.read(16);
  if (flags & FLAG.PERSONALIZED) { let ch; do { ch = r.read(7); } while (ch); }
  if (entry.kind === 'armor') {
    item.ac = r.read(bitsOf(STAT.ARMORCLASS)) - addOf(STAT.ARMORCLASS);
    item.maxDurability = r.read(bitsOf(STAT.MAXDURABILITY)) - addOf(STAT.MAXDURABILITY);
    if (item.maxDurability) item.durability = r.read(bitsOf(STAT.DURABILITY)) - addOf(STAT.DURABILITY);
  } else if (entry.kind === 'weapons') {
    item.maxDurability = r.read(bitsOf(STAT.MAXDURABILITY)) - addOf(STAT.MAXDURABILITY);
    if (item.maxDurability) item.durability = r.read(bitsOf(STAT.DURABILITY)) - addOf(STAT.DURABILITY);
  }
  if (entry.stackable) item.quantity = r.read(9);
  if (flags & FLAG.SOCKETED) item.sockets = Math.max(1, Math.min(6, r.read(bitsOf(STAT.NUMSOCKETS)), entry.invWidth * entry.invHeight));
  if (!identified) return r.overrun ? null : item;
  item.statsRead = true;
  const setMask = item.quality === QUALITY.SET ? r.read(5) : 0;
  const lists = (item.quality === QUALITY.SET ? 5 : 0) + (flags & FLAG.RUNEWORD ? 1 : 0);
  for (let list = -1; list < lists; list++) {
    const runeword = (flags & FLAG.RUNEWORD) && list === lists - 1 && list >= 0;
    const merge = list < 0 || runeword;   // set bonus lists belong to the set, not the item
    if (!merge && !(setMask & (1 << list))) continue;
    const put = (id, value, layer = 0) => {
      if (!merge) return;
      const key = `${id}:${layer}`;
      item.stats.set(key, { id, layer, value: (item.stats.get(key)?.value ?? 0) + value });
    };
    const plain = (id) => { const s = saved(id); return r.read(s?.saveBits ?? 0) - (s?.saveAdd ?? 0); };
    for (let last = -1; ;) {
      const id = r.read(9);
      if (id === 511 || r.overrun) break;
      if (!id && !last) break;
      last = id;
      const s = saved(id);
      if (!s) break;
      const multi = MULTI.get(id);
      if (multi) { for (const part of multi) put(part, plain(part)); continue; }
      const layer = s.paramBits > 0 ? r.read(s.paramBits) : 0;
      put(id, (r.read(s.saveBits) - s.saveAdd) << s.valShift, layer);
    }
  }
  return r.overrun ? null : item;
}

/** The tooltip fields of a decoded item: { name, baseName, quality, flags, props, propColours }. */
export function createItemText({ tables, lang, heroStat = () => undefined }) {
  const { stats: statTable, affixes, chars, skills } = tables;
  const key = (k) => (k ? lang.key(k) || '' : '');
  const sum = (item, id, layer = 0) => item.stats.get(`${id}:${layer}`)?.value ?? 0;
  const statIdentified = (item) => item.statsRead;

  function baseName(item) { return key(trim(item.code)) || item.entry.name; }

  function title(item) {
    const e = item.entry, identified = Boolean(item.flags & FLAG.IDENTIFIED);
    if (identified && (item.quality === QUALITY.UNIQUE || item.quality === QUALITY.SET || item.quality === QUALITY.RARE)) {
      let name = '';
      if (item.quality === QUALITY.UNIQUE) name = key(affixes.unique(item.fileIndex)?.key);
      else if (item.quality === QUALITY.SET) name = key(affixes.setItem(item.fileIndex)?.key);
      else name = [item.rarePrefix, item.rareSuffix].map((id) => key(affixes.rare(id)?.key)).filter(Boolean).join(' ');
      if (name) return { name, base: baseName(item) };
    }
    if (item.quality === QUALITY.SUPERIOR) { const sup = key('Hiquality'); if (sup) return { name: `${sup} ${baseName(item)}`, base: '' }; }
    if (item.quality === QUALITY.MAGIC && identified) {
      const pre = key(affixes.magicPrefix(item.magicPrefix)?.key), suf = key(affixes.magicSuffix(item.magicSuffix)?.key);
      const base = baseName(item);
      return { name: (pre ? `${pre} ` : '') + base + (suf ? ` ${suf}` : !pre ? '' : ' '), base: '' };
    }
    return { name: baseName(item) || e.name, base: '' };
  }

  function requiredLevel(item) {
    const e = item.entry;
    let req = 0;
    const levelOf = (a) => a?.levelReq ?? 0;
    if (item.quality === QUALITY.MAGIC) req = Math.max(levelOf(affixes.magicPrefix(item.magicPrefix)), levelOf(affixes.magicSuffix(item.magicSuffix)));
    else if (item.quality === QUALITY.RARE || item.quality === QUALITY.CRAFT) {
      let bonus = 0;
      item.magicPrefixes.forEach((id, i) => {
        const p = affixes.magicPrefix(id), s = affixes.magicSuffix(item.magicSuffixes[i]);
        if (p) { req = Math.max(req, levelOf(p)); if (item.quality === QUALITY.CRAFT) bonus += 3; }
        if (s) { req = Math.max(req, levelOf(s)); if (item.quality === QUALITY.CRAFT) bonus += 3; }
      });
      req += bonus;
      if (item.quality === QUALITY.CRAFT) req = Math.min(req, 98);
    } else if (item.quality === QUALITY.UNIQUE) req = Math.max(0, levelOf(affixes.unique(item.fileIndex)));
    else if (item.quality === QUALITY.SET) req = Math.max(0, levelOf(affixes.setItem(item.fileIndex)));
    if (item.autoAffix && (item.quality === QUALITY.MAGIC || item.quality === QUALITY.RARE)) req = Math.max(req, levelOf(affixes.automagic?.(item.autoAffix)));
    req = Math.max(req, e.levelReq) + sum(item, STAT.LEVELREQ);
    return Math.max(0, req);
  }

  function lines(item) {
    const e = item.entry, out = [], identified = statIdentified(item);
    const idx = (i) => lang.index(i) || '';
    const push = (text, colour = '') => out.push(colour + text);
    const space = idx(3995) || ' ';
    if (item.compact) return out;
    const weapon = e.kind === 'weapons' && (e.minDam || e.maxDam || e.maxDam2 || e.maxMis);
    if (weapon) {
      let mul = (v, lo) => v;
      if (item.quality === QUALITY.INFERIOR) mul = (v, lo) => Math.max(lo ? 1 : 2, Math.trunc((3 * v) / 4));
      const eth = item.flags & FLAG.ETHEREAL ? (v) => Math.trunc((3 * v) / 2) : (v) => v;
      const base = (v, lo) => (v ? eth(mul(v, lo)) : 0);
      const percentOf = (b, pct) => Math.trunc((b * pct) / 100);
      const dmg = (label, lo, hi, sMin, sMax) => {
        const bLo = base(lo, true), bHi = base(hi, false);
        let a = bLo, b = bHi;
        if (identified) {
          a = bLo + sum(item, sMin) + percentOf(bLo, sum(item, STAT.MINDAMAGE_PERCENT));
          b = bHi + sum(item, sMax) + percentOf(bHi, sum(item, STAT.MAXDAMAGE_PERCENT));
        }
        const text = `${idx(label)} ${a} ${idx(3464)} ${b}`;
        if (identified && (a !== lo || b !== hi)) out.push('\x01v' + text); else push(text);
      };
      if (e.minMis || e.maxMis) dmg(3467, e.minMis, e.maxMis, STAT.THROW_MIN, STAT.THROW_MAX);
      if (e.maxDam) dmg(3465, e.minDam, e.maxDam, STAT.MINDAMAGE, STAT.MAXDAMAGE);
      if (e.maxDam2) dmg(3466, e.minDam2, e.maxDam2, STAT.SECONDARY_MIN, STAT.SECONDARY_MAX);
    } else {
      const ac = identified ? item.ac + sum(item, STAT.ARMORCLASS) + Math.trunc((item.ac * sum(item, STAT.ARMOR_PERCENT)) / 100) : item.ac;
      if (ac > 0 && e.maxAc > 0) push(`${idx(3461)} ${ac}`);
    }
    if (e.stackable) push(`${idx(3462)} ${item.quantity}`);
    if (e.durability && !e.noDurability && item.maxDurability && !sum(item, STAT.INDESTRUCTIBLE)) {
      push(`${idx(3457)} ${item.durability} ${idx(3463)} ${item.maxDurability}`);
    }
    const pct = identified ? sum(item, STAT.REQ_PERCENT) : 0, eth = item.flags & FLAG.ETHEREAL ? -10 : 0;
    const need = (v) => Math.max(0, v + (identified ? Math.trunc((v * pct) / 100) : 0) + eth);
    if (e.reqStr) push(`${idx(3458)} ${need(e.reqStr)}`);
    if (e.reqDex) push(`${idx(3459)} ${need(e.reqDex)}`);
    if (e.level && e.speed >= -1000) {
      const lvl = identified ? requiredLevel(item) : e.levelReq;
      if (lvl > 1) push(`${idx(3469)} ${lvl}`);
    }
    if (identified) out.push(...propertyLines(item, weapon));
    if (item.flags & (FLAG.ETHEREAL | FLAG.SOCKETED)) {
      let flagLine = '';
      if (item.flags & FLAG.ETHEREAL) flagLine += '\x013' + idx(22745);
      if (item.flags & FLAG.SOCKETED) flagLine += (flagLine ? ', ' : '\x013') + idx(3453) + space + `(${item.sockets})`;
      out.push(flagLine);
    }
    return out;
  }

  function propertyLines(item, weapon) {
    let list = [...item.stats.values()].sort((a, b) => a.id - b.id);
    if (weapon) list = list.filter((s) => !(s.id >= STAT.MINDAMAGE && s.id <= STAT.SECONDARY_MAX) && s.id !== STAT.THROW_MIN && s.id !== STAT.THROW_MAX);
    const subst = (format, args) => { let n = 0; return format.replace(/%%|%d/g, (m) => (m === '%%' ? '%' : n < args.length ? String(args[n++]) : '')); };
    const rec = (id) => statTable.get(id);
    const lns = [];
    const consumed = new Set();
    const find = (id) => list.find((s) => s.id === id);
    const PAIRS = [[21, 2, 'strModMinDamage', 'strModMinDamageRange'], [23, 2, 'strModMinDamage', 'strModMinDamageRange'],
      [48, 2, 'strModFireDamage', 'strModFireDamageRange'], [50, 2, 'strModLightningDamage', 'strModLightningDamageRange'],
      [52, 2, 'strModMagicDamage', 'strModMagicDamageRange'], [54, 3, 'strModColdDamage', 'strModColdDamageRange'],
      [57, 3, 'strModPoisonDamage', 'strModPoisonDamageRange']];
    for (const [first, n, eq, range] of PAIRS) {
      const a = find(first), b = find(first + 1);
      if (!a || !b || !rec(first) || rec(first).priority <= 0) continue;
      let v1 = a.value, v2 = b.value;
      if (n === 3) {
        const c = find(first + 2);
        if (!c) continue;
        const len = c.value;
        if (first === 57) { v1 = Math.trunc((v1 * len) / 256); v2 = Math.trunc((v2 * len) / 256); }
        lns.push({ prio: rec(first).priority, text: v1 === v2 ? subst(key(eq), [v2, Math.trunc(len / 25)]) : subst(key(range), [v1, v2, Math.trunc(len / 25)]) });
        consumed.add(first + 2);
      } else lns.push({ prio: rec(first).priority, text: v1 === v2 ? subst(key(eq), [v2]) : subst(key(range), [v1, v2]) });
      consumed.add(first); consumed.add(first + 1);
    }
    const format = (r, func, val, posKey, negKey, key2, st) => {
      const base = (r.op > 1 && r.op < 6) ? rec(r.opBase) : null;
      let opBase = 0;
      if (base) { opBase = heroStat(r.opBase); if (opBase === undefined) return ''; }
      let v = st.value;
      if (base) v = (opBase >> (base.valShift & 31)) * v >> (r.opParam & 31);
      v >>= (r.valShift & 31);
      const pos = key(posKey), neg = key(negKey), two = key(key2);
      const pre = (sign, unit) => (val === 1 ? `${sign}${v}${unit} ` : '');
      const post = (sign, unit) => (val === 2 ? ` ${sign}${v}${unit}` : '');
      const body = (x) => (x < 0 && negKey ? neg : pos);
      switch (func) {
        case 1: return pre('+', '') + body(v) + post('+', '');
        case 2: return pre('', '%') + body(v) + post('', '%');
        case 3: return pre('', '') + body(v) + post('', '');
        case 4: return pre('+', '%') + body(v) + post('+', '%');
        case 5: v = Math.trunc((v * 100) / 128); return pre('', '%') + body(v) + post('', '%');
        case 6: case 7: case 8: case 9: {
          const sign = func === 6 || func === 8 ? '+' : '', unit = func === 7 || func === 8 ? '%' : '';
          return pre(sign, unit) + body(v) + ' ' + two + post(sign, unit);
        }
        case 10: v = Math.trunc((v * 100) / 128); return pre('', '%') + body(v) + ' ' + two + post('', '%');
        case 12: return pre('+', '') + body(v) + post('+', '');
        case 13: { const cs = chars.byClass(st.layer); return cs ? `+${v} ${key(cs.allSkills)}` : ''; }
        case 14: { const cs = chars.byClass((st.layer >> 3) & 3); return cs ? `${subst(key(cs.tabs[st.layer & 7] ?? ''), [v])} ${key(cs.classOnly)}` : ''; }
        case 27: { // D2Client+0x54E10: value, localized "to", skill name, then the owning class's Only label.
          const name = skills ? key(skills.nameKey(st.layer)) : '';
          if (!v || !name) return '';
          const cs = chars.byClass(skills.classId(st.layer)), sp = lang.index(3995) || ' ';
          return `${v > 0 ? '+' : ''}${v}${sp}${lang.index(4003)}${sp}${name}${sp}${cs ? key(cs.classOnly) : ''}`;
        }
        case 19: return subst(body(v), [v]);
        case 24: {
          const skill = st.layer >> 6, name = skills ? key(skills.nameKey(skill)) : '';
          if (!(skill > 0) || !name) return '';
          const sp = lang.index(3995) || ' ';
          return lang.index(21249) + sp + (st.layer & 0x3f) + sp + name + sp + subst(pos, [v & 0xff, v >> 8]);
        }
        case 20: v = -v; return pre('', '%') + body(v) + post('', '%');
        case 21: v = -v; return pre('', '') + body(v) + post('', '');
        default: return '';   // 11, 15-18, 22-23, 25-26, 28: not worded (as single player)
      }
    };
    const groups = new Map();
    for (const st of list) {
      const r = rec(st.id);
      if (!r || st.id === STAT.ARMORCLASS || consumed.has(st.id) || r.priority <= 0 || !r.func) continue;
      if (r.group) { if (!groups.has(r.group)) groups.set(r.group, []); groups.get(r.group).push(st); continue; }
      const text = format(r, r.func, r.val, r.strPos, r.strNeg, r.str2, st);
      if (text) lns.push({ prio: r.priority, text });
    }
    for (const [g, members] of groups) {
      let total = 0;
      for (let i = 0; i < statTable.count; i++) if (statTable.get(i)?.group === g) total++;
      const same = members.length === total && members.every((q) => q.value === members[0].value && q.layer === members[0].layer);
      for (let i = 0; i < (same ? 1 : members.length); i++) {
        const r = rec(members[i].id);
        const text = same ? format(r, r.groupFunc, r.groupVal, r.groupStrPos, r.groupStrNeg, r.groupStr2, members[i])
          : format(r, r.func, r.val, r.strPos, r.strNeg, r.str2, members[i]);
        if (text) lns.push({ prio: r.priority, text });
      }
    }
    lns.sort((a, b) => b.prio - a.prio);   // stable
    const seen = new Set();
    return lns.filter((l) => l.text && !seen.has(l.text) && seen.add(l.text)).map((l) => '\x013' + l.text);
  }

  return {
    /** `bytes` is the packet's item bitstream; null when it cannot be worded (an ear, an unknown code). */
    describe(bytes) {
      const item = decodeItemStream(bytes, tables);
      if (!item) return null;
      const t = title(item);
      const raw = lines(item);
      const quality = item.compact ? QUALITY.NORMAL : item.quality;
      const parsed = { lines: raw.map((l) => l.replace(/^\x01./, '')), colours: raw.map((l) => (l[0] === '\x01' ? (l[1] === 'v' ? 'v' : +l[1]) : 0)) };
      return { name: t.name, baseName: t.base, quality, flags: item.flags >>> 0, props: parsed.lines, propColours: parsed.colours };
    },
  };
}

export { BEFORE_COMPLETE };
