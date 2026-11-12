import test from 'node:test';
import assert from 'node:assert/strict';
import { readRealmClientTerrain } from './live-level-terrain.mjs';

// The native tile/file buffers are replaced on every render-level selection.
// Neighbor rooms are already active; selecting a buffer must never reveal rooms.
function engine({ failFilesFor = 0 } = {}) {
  const heap = new Uint8Array(16384), log = [];
  let selected = 1;
  const data = {
    1: { files: ['town-a.dt1', 'town-b.dt1'], tiles: [[0, 1, 41], [1, 0, 42]] },
    2: { files: ['wilderness.dt1'], tiles: [[0, 0, 51], [0, 0xffffffff, 52]] },
    3: { files: [], tiles: [] },
  };
  function write() {
    heap.fill(0);
    const view = new DataView(heap.buffer);
    data[selected].tiles.forEach(([room, slot, entry], i) => {
      const at = 128 + i * 84;
      view.setUint32(at, room, true);
      view.setInt32(at + 40, selected * 100 + i, true);
      view.setInt32(at + 44, 200, true);
      view.setUint32(at + 68, slot, true);
      view.setUint32(1024 + i * 4, entry, true);
    });
  }
  write();
  const Module = { HEAPU8: heap,
    _d2_drlg_client_select_render_level() {}, _d2_drlg_client_render_level() {},
    _malloc: () => 4096, _free() {},
    ccall(name, _return, _types, args = []) {
      if (name === 'd2_drlg_client_render_level') return selected;
      if (name === 'd2_drlg_client_select_render_level') {
        log.push(args[0]);
        if (!data[args[0]]) return 0;
        selected = args[0]; write(); return 1;
      }
      if (name === 'd2_drlg_client_tile_count') return data[selected].tiles.length;
      if (name === 'd2_drlg_client_tiles') return 128;
      if (name === 'd2_drlg_client_tile_entries') return 1024;
      if (name === 'd2_drlg_client_tile_files') {
        if (selected === failFilesFor) throw new Error('library read failed');
        heap.set(new TextEncoder().encode(data[selected].files.join('\0') + '\0'), Number(args[0]));
        return data[selected].files.length;
      }
      throw new Error(`Unexpected native operation: ${name}`);
    } };
  return { Module, log, selected: () => selected };
}

test('already-loaded neighboring terrain appears before the player crosses the boundary', () => {
  const e = engine();
  const out = readRealmClientTerrain(e.Module, [1, 2, 2, 3, 999, -1, NaN]);
  assert.deepEqual(out.files, ['town-a.dt1', 'town-b.dt1', 'wilderness.dt1']);
  assert.deepEqual(out.tiles.map(t => t.librarySlot), [1, 0, 2, 0xffffffff]);
  assert.deepEqual(out.tiles.map(t => t.entry), [41, 42, 51, 52]);
  assert.deepEqual(out.tiles.map(t => t.room), [0, 1, 2, 2]);
  assert.deepEqual(out.tiles.map(t => t.level), [1, 1, 2, 2]);
  assert.deepEqual(out.tiles.map(t => Boolean(t.seam)), [false, false, true, true]);
  assert.deepEqual(e.log, [2, 3, 999, 1]);
  assert.equal(e.selected(), 1);
  const hidden = readRealmClientTerrain(e.Module, [1]);
  assert.equal(hidden.tiles.length, 2);
  assert.ok(hidden.tiles.every(tile => !tile.seam));
});

test('a failed neighbor library read restores the live native level selector', () => {
  const e = engine({ failFilesFor: 2 });
  assert.throws(() => readRealmClientTerrain(e.Module, [2]), /library read failed/);
  assert.equal(e.selected(), 1);
  assert.deepEqual(e.log, [2, 1]);
});

test('an engine without level-selection exports keeps its current terrain readable', () => {
  const e = engine();
  delete e.Module._d2_drlg_client_select_render_level;
  const out = readRealmClientTerrain(e.Module, [2]);
  assert.equal(out.tiles.length, 2);
  assert.equal(out.tiles[0].entry, 41);
  assert.deepEqual(e.log, []);
});
