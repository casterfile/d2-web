import test from 'node:test';
import assert from 'node:assert/strict';
import {
  beginInventoryClosePress, cancelInventoryClosePress, finishInventoryClosePress,
  inventoryCloseAt, inventoryCloseFrame,
} from '../play/assets/inventory-close-gesture.mjs';
import { TRADEBTN_CLOSESTASH, closeStashFromInventory } from '../play/assets/stash-close.mjs';
import { haveEngine, loadEngine } from './helpers/node-engine.mjs';

test('inventory close uses the native inclusive hit rectangle, separate from its cel', () => {
  assert.equal(inventoryCloseAt(418, 444, 800, 600, true), true);
  assert.equal(inventoryCloseAt(450, 476, 800, 600, true), true);
  assert.equal(inventoryCloseAt(417.99, 460, 800, 600, true), false);
  assert.equal(inventoryCloseAt(450.01, 460, 800, 600, true), false);
  assert.equal(inventoryCloseAt(430, 443.99, 800, 600, true), false);
  assert.equal(inventoryCloseAt(430, 476.01, 800, 600, true), false);
  assert.equal(inventoryCloseAt(430, 460, 800, 600, false), false);
  // A 400x300 viewport scales the native coordinates by one half.
  assert.equal(inventoryCloseAt(209, 222, 400, 300, true), true);
});

test('inventory close arms only on a visible close hit and draws the native held frame', () => {
  assert.deepEqual(beginInventoryClosePress(12, true, true), { pointerId: 12 });
  assert.equal(beginInventoryClosePress(12, false, true), null);
  assert.equal(beginInventoryClosePress(12, true, false), null);
  assert.equal(inventoryCloseFrame(false), 10);
  assert.equal(inventoryCloseFrame(true), 11);
});

test('inventory close activates only on the matching left release inside while still visible', () => {
  const press = beginInventoryClosePress(12, true, true);
  assert.deepEqual(finishInventoryClosePress(press, 12, true, true, true), { matched: true, activate: true });
  assert.deepEqual(finishInventoryClosePress(press, 12, true, true, false), { matched: true, activate: false });
  assert.deepEqual(finishInventoryClosePress(press, 12, false, true, true), { matched: true, activate: false });
  assert.deepEqual(finishInventoryClosePress(press, 12, true, false, true), { matched: true, activate: false });
  assert.deepEqual(finishInventoryClosePress(press, 13, true, true, true), { matched: false, activate: false });
});

test('inventory close press can be canceled for its pointer or on global focus loss', () => {
  const press = beginInventoryClosePress(12, true, true);
  assert.equal(cancelInventoryClosePress(press, 12), true);
  assert.equal(cancelInventoryClosePress(press, 13), false);
  assert.equal(cancelInventoryClosePress(press, null), true);
  assert.equal(cancelInventoryClosePress(null, 12), false);
});

test('accepted stash close still sends button 18 and refusal leaves paired panels open', () => {
  const calls = [];
  let panelsOpen = true;
  const accepted = closeStashFromInventory({ stashOpen: true,
    commandButton: (button, amount) => { calls.push([button, amount]); return 1; },
    onAccepted: () => { panelsOpen = false; } });
  assert.equal(accepted, true);
  assert.deepEqual(calls, [[TRADEBTN_CLOSESTASH, 0]]);
  assert.equal(panelsOpen, false);

  panelsOpen = true;
  const refused = closeStashFromInventory({ stashOpen: true,
    commandButton: (button, amount) => { calls.push([button, amount]); return 0; },
    onAccepted: () => { panelsOpen = false; } });
  assert.equal(refused, false);
  assert.deepEqual(calls[1], [18, 0]);
  assert.equal(panelsOpen, true);
});

test('stash close: hit on the cel, press draws buysellbtn frame 11, release inside closes via button 18', async () => {
  const { stashCloseAt } = await import('../play/assets/stash-close.mjs');
  const calls = [];
  const hit = (...a) => { calls.push(a); return 1; };
  assert.equal(stashCloseAt(372.6, 461.2, 800, 600, true, hit), true);
  assert.deepEqual(calls[0], [372, 461, 80, -60, 600, 1]);
  assert.equal(stashCloseAt(372 + 500, 461 + 300, 1800, 1200, true, hit, false), true);
  assert.deepEqual(calls[1], [372, 461, 80, -60, 600, 0]);
  assert.equal(stashCloseAt(360, 460, 800, 600, false, hit), false);
  assert.equal(stashCloseAt(360, 460, 800, 600, true, null), false);
  assert.equal(stashCloseAt(360, 460, 800, 600, true, () => 0), false);
  assert.equal(calls.length, 2);
  const press = beginInventoryClosePress(7, true, true);
  assert.equal(inventoryCloseFrame(Boolean(press)), 11);
  assert.equal(inventoryCloseFrame(Boolean(null)), 10);
  assert.equal(finishInventoryClosePress(press, 7, true, true, true).activate, true);
  assert.equal(finishInventoryClosePress(press, 7, true, true, false).activate, false);
});

// Each edge of the stash close button and one pixel past it, in 800x600 game px, for an expansion and a
// classic game: the bounds the original client accepts when the button is clicked headlessly.
test('stash close: engine hit test, every edge and one pixel past', { skip: !haveEngine() && 'needs engine' }, async () => {
  const { stashCloseAt } = await import('../play/assets/stash-close.mjs');
  const Module = await loadEngine({});
  const hit = (...a) => Module.ccall('d2_client_stash_close_hit', 'number', ['number', 'number', 'number', 'number', 'number', 'number'], a);
  for (const [expansion, x0, x1, y0, y1] of [[true, 353, 391, 442, 480], [false, 356, 394, 441, 479]]) {
    const at = (x, y) => stashCloseAt(x, y, 800, 600, true, hit, expansion);
    const mx = (x0 + x1) >> 1, my = (y0 + y1) >> 1;
    assert.equal(at(mx, my), true);
    assert.equal(at(x0, my), true); assert.equal(at(x0 - 1, my), false);
    assert.equal(at(x1, my), true); assert.equal(at(x1 + 1, my), false);
    assert.equal(at(mx, y0), true); assert.equal(at(mx, y0 - 1), false);
    assert.equal(at(mx, y1), true); assert.equal(at(mx, y1 + 1), false);
    assert.equal(at(x0, y0), true); assert.equal(at(x1, y1), true);
    assert.equal(at(x0 - 1, y0 - 1), false); assert.equal(at(x1 + 1, y1 + 1), false);
  }
});
