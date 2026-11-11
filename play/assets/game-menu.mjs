// The in-game menu Esc opens (OPTIONS / SAVE AND EXIT GAME / RETURN TO GAME) and the Options page
// its first line opens. Retail draws every line as ONE pre-baked cel (or cel pair) out of
// `data\local\ui\eng\` -- NOT as font text -- and flanks the selected line with two
// `data\global\ui\CURSOR\Pentspin.dc6` frames.
//
// Every anchor below is a record of retail's OWN draw list, never derived from a frame
//:
//   * escape menu      -- `opt-esc-menu.txt` (+ `-hover-options`, `-hover-saveexit`)
//   * Options page     -- `opt-options-page.txt` (+ `-hover-sound`, `-sound-on`)
//   * the pentagrams   -- x=99 and x=649, y = the SELECTED row's anchor + 12 (measured on all four
//                         hover captures and both idle ones).
// A cel lands with its left edge at `x + frame.offsetX` and its bottom row at `y + frame.offsetY`
// (drawlist.mjs), so a spec carries retail's own (x, y) anchor and the frame supplies the box.
//
// The palette is the ACT's, the one the world this menu overlays is drawn with: retail renders the
// whole in-game screen in one palette, and the frame's own colours are what it must match.
// The native capture used -ns, which dimmed SOUND OPTIONS. The browser has a
// Web Audio output device, so keep this row enabled instead of inheriting the
// capture machine's disabled-device state.
//
// F215/F216: the SOUND OPTIONS and VIDEO OPTIONS pages themselves live in `options-pages.mjs`
// (its own row table and renderer), opened from two of the OPTIONS lines below.
import { dc6FrameToRgba } from './d2-formats.mjs';
import { createBlender } from './frontend.mjs';
import { recordDc6 } from './drawlist.mjs';
import { createOptionsPages } from './options-pages.mjs';

const ENG = 'data\\local\\ui\\eng\\';
const FILE = Object.freeze({
  options: 'options.dc6', exit: 'exit.dc6', returntogame: 'returntogame.dc6',
  soundoptions: 'soundoptions.dc6', videooptions: 'videooptions.dc6', automapoptions: 'automapoptions.dc6',
  cfgoptions: 'cfgoptions.dc6', previous: 'previous.dc6',
  // F217: the Automap Options sub-page -- the same cel renderer, 26 px rows at a 45 px pitch.
  automapmode: 'AutoMapMode.dc6', automapfade: 'AutoMapFade.dc6', automapcenter: 'AutoMapCenter.dc6',
  automapparty: 'AutoMapParty.dc6', automappartynames: 'AutoMapPartyNames.dc6',
  full: 'Full.dc6', mini: 'Mini.dc6', everything: 'Everything.dc6', smallno: 'SmallNo.dc6', smallyes: 'SmallYes.dc6',
  sprevious: 'SPrevious.dc6',
});
export const PENTAGRAM = 'data\\global\\ui\\CURSOR\\Pentspin.dc6';
export const PENTAGRAM_X = Object.freeze([99, 649]);
export const PENTAGRAM_TOP_OFFSET = 12;   // pentagram anchor y = selected row's anchor + 12

/** `[file, frame, x, y]` per cel, retail's own anchors from the draw-list vectors. */
export const ESC_MENU_ROWS = Object.freeze([
  { label: 'OPTIONS', anchorY: 224, cels: [['options', 0, 319, 224]] },
  { label: 'SAVE AND EXIT GAME', anchorY: 274, cels: [['exit', 0, 182, 274], ['exit', 1, 438, 274]] },
  { label: 'RETURN TO GAME', anchorY: 324, cels: [['returntogame', 0, 221, 324], ['returntogame', 1, 477, 324]] },
]);
export const OPTIONS_PAGE_ROWS = Object.freeze([
  { label: 'SOUND OPTIONS', anchorY: 174, dim: false, cels: [['soundoptions', 0, 245, 174], ['soundoptions', 1, 501, 174]] },
  { label: 'VIDEO OPTIONS', anchorY: 224, cels: [['videooptions', 0, 252, 224], ['videooptions', 1, 508, 224]] },
  { label: 'AUTOMAP OPTIONS', anchorY: 274, cels: [['automapoptions', 0, 210, 274], ['automapoptions', 1, 466, 274]] },
  { label: 'CONFIGURE CONTROLS', anchorY: 324, cels: [['cfgoptions', 0, 188, 324], ['cfgoptions', 1, 444, 324]] },
  { label: 'PREVIOUS MENU', anchorY: 374, cels: [['previous', 0, 243, 374], ['previous', 1, 499, 374]] },
]);

// F217: the AUTOMAP OPTIONS sub-page (Esc -> OPTIONS -> AUTOMAP OPTIONS). Same renderer as the two
// pages above, retail's own anchors from `the retail captures` and its four toggled
// variants: title `AutoMapOptions.dc6` f0/f1 210,137 / 466,137 (the SAME file as the menu row), five
// 26 px rows 45 px apart from y 182, `SPrevious.dc6` 284,407. Each row is a label cel plus a VALUE cel
// chosen by its setting; the pentagram sits at the selected row's anchor + 15 (these rows are 26 px,
// not the menu's 36). Retail's coupling, measured on `opt-automap-names-toggled.txt`: Show Party = NO
// draws Show Names dimmed (DrawMode 1) and dead, with the value forced to NO.
// AutoMap Left is a client startup setting, not an Automap Options row. D2Client's bulk ini loader
// seeds a missing `AutoMap Left` key to 1; retain that internal state without inventing a menu control.
export const AUTOMAP_DEFAULTS = Object.freeze({ mode: 'full', fade: 'off', center: 'yes', party: 'yes', names: 'yes', left: 'yes' });
export const AUTOMAP_PENTAGRAM_OFFSET = 15;
export function shouldUpdateGameMenuSelectionOnPointerMove(gesture) { return !gesture; }
export function gameMenuGestureMatchesRelease(gesture, pointerId, button, page, row) {
  return Boolean(gesture && button === 0 && gesture.pointerId === pointerId && gesture.page === page && gesture.row === row);
}
/** Track the single mouse gesture that owns a pressed game-menu row. */
export function createGameMenuPointerState() {
  let armed = null;
  return {
    get current() { return armed; },
    arm(gesture) { armed = gesture; return armed; },
    release(pointerId) {
      if (armed?.pointerId !== pointerId) return null;
      const previous = armed; armed = null; return previous;
    },
    cancel(pointerId) {
      if (armed?.pointerId !== pointerId) return false;
      armed = null; return true;
    },
    clear() { const hadGesture = armed !== null; armed = null; return hadGesture; },
  };
}
const AUTOMAP_TITLE = Object.freeze([['automapoptions', 0, 210, 137], ['automapoptions', 1, 466, 137]]);
const AUTOMAP_ROWS = Object.freeze([
  { label: 'AUTOMAP MODE', key: 'mode', anchorY: 182, cels: [['automapmode', 0, 170, 182]],
    values: { full: [['full', 0, 450, 182]], mini: [['mini', 0, 499, 182]] } },
  { label: 'AUTOMAP FADE', key: 'fade', anchorY: 227, cels: [['automapfade', 0, 170, 227]],
    values: { off: [['smallno', 0, 590, 227]], on: [['everything', 0, 457, 227]] } },
  { label: 'AUTOMAP CENTER', key: 'center', anchorY: 272, cels: [['automapcenter', 0, 170, 272], ['automapcenter', 1, 426, 272]],
    values: { yes: [['smallyes', 0, 579, 272]], no: [['smallno', 0, 590, 272]] } },
  { label: 'AUTOMAP PARTY', key: 'party', anchorY: 317, cels: [['automapparty', 0, 170, 317]],
    values: { yes: [['smallyes', 0, 579, 317]], no: [['smallno', 0, 590, 317]] } },
  { label: 'AUTOMAP PARTY NAMES', key: 'names', anchorY: 362, cels: [['automappartynames', 0, 170, 362]],
    values: { yes: [['smallyes', 0, 579, 362]], no: [['smallno', 0, 590, 362]] } },
]);
const AUTOMAP_PREVIOUS = Object.freeze({ label: 'PREVIOUS', anchorY: 407, cels: [['sprevious', 0, 284, 407]] });

function frameCanvas(frame, palette, documentObject) {
  const canvas = documentObject.createElement('canvas');
  canvas.width = Math.max(1, frame.width); canvas.height = Math.max(1, frame.height);
  if (!frame.width || !frame.height) return canvas;
  canvas.getContext('2d').putImageData(new ImageData(dc6FrameToRgba(frame, palette), frame.width, frame.height), 0, 0);
  return canvas;
}

/** `onChoose(item)` receives one of the rows' `label`s. */
export function createGameMenu({ read, decodeDc6, decodeDatPalette, onChoose, onResolution = null, palettePath = 'data\\global\\palette\\ACT1\\pal.dat', documentObject = globalThis.document, initialVolume = null, initialNpcSpeech = 2, initialResolution = 1, onVolume = null, onNpcSpeech = null }) {
  const resolved = () => (typeof palettePath === 'function' ? palettePath() : palettePath);
  let palette = null, paletteAt = null, blend = null;
  const sheets = new Map();       // file key -> { path, dc6, canvases }
  const ensurePalette = () => {
    const p = resolved();
    if (p === paletteAt) return false;
    palette = decodeDatPalette(read(p));
    blend = createBlender(read(p.replace(/pal\.dat$/i, 'pal.pl2')), palette);
    paletteAt = p; sheets.clear();
    return true;
  };
  const sheetFor = (key) => {
    let s = sheets.get(key);
    if (!s) {
      const path = ENG + FILE[key];
      const dc6 = decodeDc6(read(path));
      s = { path, dc6, canvases: dc6.frames.map((f) => frameCanvas(f, palette, documentObject)) };
      sheets.set(key, s);
    }
    return s;
  };
  // Per cel: the drawn box, from retail's anchor and the frame's own offsets/size.
  const place = ([key, frame, ax, ay]) => {
    const s = sheetFor(key), f = s.dc6.frames[frame];
    if (!f) throw new RangeError(`${s.path}: no frame ${frame}`);
    return { sheet: s, frame, index: frame, f,
      left: ax + (f.offsetX | 0), top: ay + (f.offsetY | 0) - f.height + 1, width: f.width, height: f.height };
  };
  const rowsFrom = (spec) => spec.map((row) => {
    const cels = row.cels.map(place);
    const x = Math.min(...cels.map((c) => c.left)), y = Math.min(...cels.map((c) => c.top));
    const w = Math.max(...cels.map((c) => c.left + c.width)) - x, h = Math.max(...cels.map((c) => c.top + c.height)) - y;
    return { label: row.label, anchorY: row.anchorY, dim: !!row.dim, cels, x, y, w, h };
  });
  // F217: the Automap Options page IS state -- its five rows each carry a value cel that a click
  // cycles. `settings` is that state; the page is rebuilt from it whenever it changes. The coupling
  // (party = no deadens names) is retail's, measured in opt-automap-names-toggled.txt.
  const settings = { ...AUTOMAP_DEFAULTS };
  const automapSpec = () => {
    const spec = [{ label: null, anchorY: 137, cels: AUTOMAP_TITLE }];
    for (const r of AUTOMAP_ROWS) {
      const dead = r.key === 'names' && settings.party === 'no';
      spec.push({ label: r.label, anchorY: r.anchorY, dim: dead, cels: [...r.cels, ...(r.values[dead ? 'no' : settings[r.key]] || [])] });
    }
    spec.push(AUTOMAP_PREVIOUS);
    return spec;
  };
  ensurePalette();
  const pages = { main: rowsFrom(ESC_MENU_ROWS), options: rowsFrom(OPTIONS_PAGE_ROWS), automap: rowsFrom(automapSpec()) };
  const pent = decodeDc6(read(PENTAGRAM));
  const pentCanvases = pent.frames.map((f) => frameCanvas(f, palette, documentObject));
  let page = 'main', pentOffset = PENTAGRAM_TOP_OFFSET, rows = pages.main;
  let open = false, selected = rows.length - 1, tick = 0;
  const pointerState = createGameMenuPointerState();
  const goto = (name) => { page = name; pentOffset = name === 'automap' ? AUTOMAP_PENTAGRAM_OFFSET : PENTAGRAM_TOP_OFFSET; rows = pages[name]; selected = rows.length - 1; };
  const cycle = (label) => {
    const r = AUTOMAP_ROWS.find((x) => x.label === label);
    if (!r || (r.key === 'names' && settings.party === 'no')) return;   // names is dead while party = no
    const order = Object.keys(r.values);
    settings[r.key] = order[(order.indexOf(settings[r.key]) + 1) % order.length];
    if (r.key === 'party' && settings.party === 'no') settings.names = 'no';
    if (page === 'automap') { pages.automap = rowsFrom(automapSpec()); rows = pages.automap; selected = rows.findIndex((x) => x.label === label); }
  };
  // F215/F216: the SOUND OPTIONS and VIDEO OPTIONS screens behind two of the OPTIONS lines. Built on
  // first use and never allowed to throw into the escape menu: a page that cannot load leaves the menu
  // it was opened from intact.
  let sub = null, subError = '';
  const subpages = () => {
    if (!sub && !subError) { try { sub = createOptionsPages({ read, decodeDc6, decodeDatPalette, documentObject, initialVolume, initialNpcSpeech, initialResolution, onVolume, onNpcSpeech, onResolution }); } catch (error) { subError = String(error); } }
    return sub;
  };
  const choose = (label) => {
    if (page === 'main' && label === 'OPTIONS') return goto('options');
    if (page === 'options' && label === 'SOUND OPTIONS') return void subpages()?.open('sound');
    if (page === 'options' && label === 'VIDEO OPTIONS') return void subpages()?.open('video');
    if (page === 'options' && label === 'PREVIOUS MENU') return goto('main');
    if (page === 'options' && label === 'AUTOMAP OPTIONS') return goto('automap');
    // F218: CONFIGURE CONTROLS is the one Options row this renderer does not own -- the panel is a
    // screen of its own (assets/configure-controls.mjs), so the page hands the choice to the shell.
    if (page === 'options' && label === 'CONFIGURE CONTROLS') return onChoose?.(label);
    if (page === 'automap') { if (label === 'PREVIOUS') return goto('options'); return cycle(label); }
    if (page === 'main') onChoose?.(label);
  };
  const view = (width, height) => {
    const scale = Math.min(width / 800, height / 600);
    return { scale, ox: (width - 800 * scale) / 2, oy: (height - 600 * scale) / 2 };
  };
  const rowAt = (x, y, width, height) => {
    const v = view(width, height), sx = (x - v.ox) / v.scale, sy = (y - v.oy) / v.scale;
    return rows.findIndex((r) => r.label !== null && sx >= r.x && sx < r.x + r.w && sy >= r.y && sy < r.y + r.h);
  };
  // The automap page carries a title row (label null) that is drawn but not selectable.
  const selectable = () => rows.map((r, i) => (r.label !== null ? i : -1)).filter((i) => i >= 0);
  return {
    get open() { return open; },
    get page() { return page; },
    /** F217: the Automap Options page's settings, read by the shell for the automap overlay. */
    get settings() { return { ...settings }; },
    /** Configured keyboard actions share the same option state and name/party coupling as the page. */
    toggleAutomapSetting(key) {
      const row = AUTOMAP_ROWS.find((candidate) => candidate.key === key);
      if (!row || (key === 'names' && settings.party === 'no')) return false;
      cycle(row.label);
      return true;
    },
    get subpage() { return sub?.page ?? null; },
    get evidence() { return { open, page, subpage: sub?.page ?? null, selected: rows[selected]?.label ?? null, settings: { ...settings },
      rows: rows.filter((r) => r.label !== null).map((r) => ({ label: r.label, x: r.x, y: r.y, w: r.w, h: r.h, dim: r.dim })),
      sub: sub?.evidence ?? (subError ? { error: subError } : null) }; },
    toggle() { pointerState.clear(); sub?.pointerCancelAll?.(); open = !open; goto('main'); return open; },
    close() { pointerState.clear(); sub?.pointerCancelAll?.(); sub?.close(); open = false; goto('main'); },
    /**
     * Esc. Retail: from a SOUND/VIDEO screen straight back to the ESC MENU -- `opt-sound-after-esc`
     * draws the menu (Options/Exit/ReturnToGame cels) with RETURN TO GAME flanked -- not the OPTIONS
     * page, so the sub-page closes AND the menu returns to `main`. From the OPTIONS page or AUTOMAP
     * back one page; from the escape menu, out.
     */
    back() { pointerState.clear(); sub?.pointerCancelAll?.(); if (sub?.isOpen) { sub.close(); return goto('main'); } if (page === 'main') open = false; else goto(page === 'automap' ? 'options' : 'main'); },
    key(code) {
      if (!open) return false;
      if (sub?.isOpen) return true;   // retail's sound/video screens are pointer-driven
      const idx = selectable(); if (!idx.length) return false;
      const at = idx.indexOf(selected);
      if (code === 'ArrowUp') selected = idx[(at - 1 + idx.length) % idx.length];
      else if (code === 'ArrowDown') selected = idx[(at + 1) % idx.length];
      else if (code === 'Enter') choose(rows[selected].label);
      else return false;
      return true;
    },
    hover(x, y, width, height) {
      if (sub?.isOpen) return sub.hover(x, y, width, height);
      if (!shouldUpdateGameMenuSelectionOnPointerMove(pointerState.current)) return pointerState.current.row;
      const i = rowAt(x, y, width, height); if (i >= 0) selected = i; return i;
    },
    pointerDown(x, y, width, height, pointerId, button) {
      if (sub?.isOpen) return sub.pointerDown(x, y, width, height, pointerId, button);
      if (button !== 0) return true;
      const row = rowAt(x, y, width, height);
      if (row < 0) return false;
      selected = row;
      pointerState.arm({ pointerId, page, row });
      return true;
    },
    pointerUp(x, y, width, height, pointerId, button) {
      if (sub?.isOpen) {
        const result = sub.pointerUp(x, y, width, height, pointerId, button);
        if (result === 'previous') sub.close();
        return result;
      }
      if (button !== 0 || pointerState.current?.pointerId !== pointerId) return false;
      const armed = pointerState.release(pointerId);
      const row = rowAt(x, y, width, height);
      if (open && gameMenuGestureMatchesRelease(armed, pointerId, button, page, row)) { selected = row; choose(rows[row].label); }
      return true;
    },
    pointerCancel(pointerId) {
      return pointerState.cancel(pointerId) || (sub?.pointerCancel(pointerId) ?? false);
    },
    pointerCancelAll() { pointerState.clear(); sub?.pointerCancelAll?.(); },
    click(x, y, width, height) {
      if (sub?.isOpen) { if (sub.click(x, y, width, height) === 'previous') sub.close(); return; }
      const i = rowAt(x, y, width, height); if (i >= 0) { selected = i; choose(rows[i].label); } return i;
    },
    /** Whether a canvas point is on a row click() would take: the touch layer's modal probe reads it. */
    hitAt(x, y, width, height) {
      if (sub?.isOpen) return Boolean(sub.hitAt?.(x, y, width, height));
      return rowAt(x, y, width, height) >= 0;
    },
    /** Canvas pixel centre of each row, from the same rect click() tests. */
    targets(width, height) {
      const v = view(width, height);
      const base = Object.fromEntries(rows.filter((r) => r.label !== null).map((r) => [r.label, [Math.round(v.ox + (r.x + r.w / 2) * v.scale), Math.round(v.oy + (r.y + r.h / 2) * v.scale)]]));
      return sub?.isOpen ? { ...base, ...sub.targets(width, height) } : base;
    },
    step() { tick++; sub?.step(); },
    draw(ctx, width, height) {
      if (!open) return;
      // The act can change under an open menu (a waypoint, a save loaded in another act): the cels are
      // re-decoded in that act's palette, which is what the world behind them is drawn in.
      if (ensurePalette()) { pages.main = rowsFrom(ESC_MENU_ROWS); pages.options = rowsFrom(OPTIONS_PAGE_ROWS); pages.automap = rowsFrom(automapSpec()); rows = pages[page]; selected = selectable().includes(selected) ? selected : selectable().at(-1); }
      // F215/F216: an open SOUND/VIDEO sub-page owns the frame (the escape menu's own cels are not
      // drawn behind it -- retail's `opt-sound-after-esc` shows the menu only after the sub-page closes).
      if (sub?.isOpen) { sub.draw(ctx, width, height); return; }
      const v = view(width, height);
      ctx.save(); ctx.translate(v.ox, v.oy); ctx.scale(v.scale, v.scale);
      // At retail's own geometry (800x600, no offset) DrawMode 1 is exactly PL2's alpha table over the
      // pixels already there; a scaled viewport cannot index those pixels, so it takes the same 50%
      // wash help-overlay.mjs declares. The record is mode 1 either way -- it is what is being drawn.
      const exact = v.scale === 1 && v.ox === 0 && v.oy === 0;
      for (const r of rows) for (const c of r.cels) {
        if (r.dim && exact) blend(ctx, c.f, c.left, c.top, 1);
        else if (r.dim) { ctx.globalAlpha = 0.5; ctx.drawImage(c.sheet.canvases[c.index], c.left, c.top); ctx.globalAlpha = 1; }
        else ctx.drawImage(c.sheet.canvases[c.index], c.left, c.top);
        recordDc6(c.sheet.path, c.index, c.f, c.left, c.top, { op: 'cel', mode: r.dim ? 1 : 5 });
      }
      const r = rows[selected], frame = Math.floor(tick / 2) % pent.frames.length, pf = pent.frames[frame];
      for (const anchorX of PENTAGRAM_X) {
        const left = anchorX + (pf.offsetX | 0), top = (r.anchorY + pentOffset) + (pf.offsetY | 0) - pf.height + 1;
        ctx.drawImage(pentCanvases[frame], left, top);
        recordDc6(PENTAGRAM, frame, pf, left, top, { op: 'cel', mode: 5 });
      }
      ctx.restore();
    },
  };
}
