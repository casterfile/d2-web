// Live terrain has no act atlas: the engine supplies the current level's DT1 list and tile
// identities. The act only selects the retail palette that colours those decoded indexed pixels.
import { decodeDatPalette, decodeDt1 } from './d2-formats.mjs';
import { createMpqAssetReader } from './mpq-assets.mjs';
import { createSessionTerrain, parseSessionTiles, readLevelTileFiles } from './session-terrain.mjs';

const ACT_PALETTES = Object.freeze([
  'data\\global\\palette\\ACT1\\pal.dat',
  'data\\global\\palette\\ACT2\\pal.dat',
  'data\\global\\palette\\ACT3\\pal.dat',
  'data\\global\\palette\\ACT4\\pal.dat',
  'data\\global\\palette\\ACT5\\pal.dat',
]);

/** The palette an act's in-game screen is drawn in -- the world's and its panels'. `act` is 1-based. */
export function actPalettePath(act) { return ACT_PALETTES[Math.min(5, Math.max(1, act | 0)) - 1]; }

export function loadLiveLevelTerrain(Module, act0) {
  try {
    const files = readLevelTileFiles(Module);
    if (!files.some(Boolean)) return { terrain: null, files: null, error: 'engine reported no tile libraries for this level' };
    const palettePath = ACT_PALETTES[act0];
    if (!palettePath) throw new RangeError(`unsupported act ${act0 + 1}`);
    return { terrain: createSessionTerrain({
      read: createMpqAssetReader(Module), decodeDt1, decodeDatPalette, palettePath,
    }), files, error: '' };
  } catch (error) {
    return { terrain: null, files: null, error: String(error) };
  }
}

// A joined game's map revision moves on every room the server reveals or hides, and each one re-reads the
// tile list. The terrain object holds the decoded libraries and tile bitmaps, keyed by file and coloured by
// the act's palette, so one terrain serves every revision of an act: a fresh one per revision redecoded
// every visible tile on the next draw. A new act (another palette) gets a new terrain.
const realmTerrains = new WeakMap();   // owner (the engine module): { act0, terrain }
export function realmTerrainFor(owner, act0, make) {
  const held = realmTerrains.get(owner);
  if (held && held.act0 === act0) return held.terrain;
  const terrain = make();
  realmTerrains.set(owner, { act0, terrain });
  return terrain;
}

/** Read already-active client terrain, including the rooms across an open level seam. */
export function readRealmClientTerrain(Module, levels = []) {
  const select = Module._d2_drlg_client_select_render_level && Module._d2_drlg_client_render_level;
  const original = select ? Module.ccall('d2_drlg_client_render_level', 'number', [], []) : null;
  const tiles = [], files = [];
  let roomBase = 0;
  const read = (level, seam) => {
    const count = Module.ccall('d2_drlg_client_tile_count', 'number', [], []);
    const ptr = Number(Module.ccall('d2_drlg_client_tiles', 'number', [], []));
    const part = count > 0 && ptr > 0 ? parseSessionTiles(Module.HEAPU8, ptr, count) : [];
    const entryPtr = count > 0 ? Number(Module.ccall('d2_drlg_client_tile_entries', 'number', [], [])) : 0;
    if (entryPtr) {
      const entries = new Uint32Array(Module.HEAPU8.buffer, entryPtr, count);
      part.forEach((tile, i) => { tile.entry = entries[i]; });
    }
    if (!part.length) return;
    const localFiles = readLevelTileFiles(Module, 4096, 'd2_drlg_client_tile_files'), fileBase = files.length;
    files.push(...localFiles);
    let maxRoom = -1;
    for (const tile of part) {
      maxRoom = Math.max(maxRoom, tile.room);
      tiles.push({ ...tile, level, room: tile.room + roomBase,
        librarySlot: tile.librarySlot < localFiles.length ? tile.librarySlot + fileBase : tile.librarySlot,
        ...(seam ? { seam: true } : {}) });
    }
    roomBase += maxRoom + 1;
  };
  try {
    read(original, false);
    if (select) for (const level of new Set(levels)) {
      if (!Number.isInteger(level) || level <= 0 || level === original) continue;
      // Selecting a tile buffer does not reveal/activate rooms. It only reads
      // the rooms the server already supplied, as the session seam reader does.
      if (Module.ccall('d2_drlg_client_select_render_level', 'number', ['number'], [level])) read(level, true);
    }
  } finally {
    // These buffers belong to the native client; restore its current-level
    // selector even if a library read fails while collecting a neighbor.
    if (select) Module.ccall('d2_drlg_client_select_render_level', 'number', ['number'], [original]);
  }
  return { tiles, files };
}

/** The client-mode DRLG tiles activated by D2GS 0x07 MapReveal packets. */
export function loadRealmClientTerrain(Module, act0, levels = []) {
  try {
    if (!Module._d2_drlg_client_tiles || !Module._d2_drlg_client_tile_count) throw new Error('client DRLG tile exports are unavailable');
    const { tiles, files } = readRealmClientTerrain(Module, levels);
    if (!files.length || !tiles.length) return { terrain: null, files, tiles, error: 'MapReveal has not activated drawable client rooms' };
    const palettePath = ACT_PALETTES[act0];
    if (!palettePath) throw new RangeError(`unsupported act ${act0 + 1}`);
    return { terrain: realmTerrainFor(Module, act0, () => createSessionTerrain({ read: createMpqAssetReader(Module), decodeDt1, decodeDatPalette, palettePath })), files, tiles, error: '' };
  } catch (error) { return { terrain: null, files: null, tiles: [], error: String(error) }; }
}
