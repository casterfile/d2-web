import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';

// Exercise the actual adapter method against a cold engine: same-page tests hide
// missing join flags because a preceding new-character creation already sets them.
const source = readFileSync(new URL('../play/gameplay-shell.mjs', import.meta.url), 'utf8');
const start = source.indexOf('      createFromSave(bytes, difficulty) {');
const end = source.indexOf('      move(x, y)', start);
assert.ok(start >= 0 && end > start);
const method = source.slice(start, end);
function coldEngine(result = 0) {
  const flags = { stream: 1, units: 0, reveal: 0 }, calls = [], heap = new Uint8Array(1024);
  const Module = {
    HEAPU8: heap, _malloc: () => 32, _free: (p) => calls.push(['free', p]),
    ccall(name, ret, types, args = []) {
      calls.push([name, ...args]);
      if (name === 'd2_session_create_stream') flags.stream = args[0];
      if (name === 'd2_session_join_units') flags.units = args[0];
      if (name === 'd2_session_join_reveal') flags.reveal = args[0];
      if (name === 'd2_session_create_from_save') {
        assert.deepEqual(flags, { stream: 0, units: 1, reveal: 2 }, 'saved creation must publish local-client units and rooms in a cold engine');
        return result;
      }
      return 1;
    },
  };
  const adapter = vm.runInNewContext(`({${method}})`, { Module, JOIN_REVEAL: 2, BigInt });
  return { adapter, calls, heap };
}
test('cold saved-character creation initializes NPC delivery and preserves save bytes', () => {
  const { adapter, calls, heap } = coldEngine();
  const bytes = Uint8Array.of(0x55, 0xaa, 0x55, 0xaa, 96, 0, 0, 0), before = bytes.slice();
  assert.equal(adapter.createFromSave(bytes, 1), 0);
  assert.deepEqual(bytes, before);
  assert.deepEqual(heap.slice(32, 40), before);
  assert.ok(calls.some(([name]) => name === 'd2_session_client_map_start'));
  assert.deepEqual(calls.at(-1), ['free', 32]);
});
test('a refused saved character keeps its error and frees the staging buffer', () => {
  const { adapter, calls } = coldEngine(7);
  assert.equal(adapter.createFromSave(Uint8Array.of(1, 2), 0), 7);
  assert.equal(calls.some(([name]) => name === 'd2_session_client_map_start'), false);
  assert.deepEqual(calls.at(-1), ['free', 32]);
});
