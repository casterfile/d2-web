import test from 'node:test';
import assert from 'node:assert/strict';
import vm from 'node:vm';
import { readFileSync } from 'node:fs';
import { saveBeforeExit } from '../play/assets/save-exit.mjs';

// Run the real packet-client exit method: releasing its flow clears the live
// session, so deferring persistence until afterward silently drops progress.
const shell = readFileSync(new URL('../play/gameplay-shell.mjs', import.meta.url), 'utf8');
const view = shell.indexOf('const realmClientView =');
const start = shell.indexOf('    saveAndExit() {', view);
const end = shell.indexOf('    // Only a subsequent', start);
assert.ok(view >= 0 && start > view && end > start);
const method = shell.slice(start, end);
async function exercise(saveResult) {
  const events = [], native = {};
  const ctx = { session: native, realmFlow: { local: {} }, gameMenuError: '', lastSaveError: 'storage full', saveBeforeExit,
    gameMenu: { close: () => events.push('close') },
    persistCurrentCharacter: async () => { assert.equal(ctx.session, native); events.push('persist'); return saveResult; },
    releaseRealmFlow: async () => { events.push('release'); ctx.session = null; ctx.realmFlow = null; },
    leaveGame: () => { assert.equal(ctx.session, native); events.push('leave'); ctx.session = null; },
    enterCharacterSelect: async () => events.push('select'),
  };
  vm.runInNewContext(`({${method}}).saveAndExit()`, ctx);
  await new Promise(resolve => setImmediate(resolve));
  return { ctx, events, native };
}
test('local save and exit persists before detaching, then releases the owned session', async () => {
  const { ctx, events } = await exercise({ name: 'Player' });
  assert.deepEqual(events, ['persist', 'close', 'release', 'leave', 'select']);
  assert.equal(ctx.gameMenuError, '');
});
test('failed persistence keeps the local game and menu intact', async () => {
  const { ctx, events, native } = await exercise(null);
  assert.deepEqual(events, ['persist']);
  assert.equal(ctx.session, native);
  assert.ok(ctx.realmFlow.local);
  assert.match(ctx.gameMenuError, /storage full/);
});
