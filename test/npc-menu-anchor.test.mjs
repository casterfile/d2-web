import test from 'node:test';
import assert from 'node:assert/strict';
import vm from 'node:vm';
import { readFileSync } from 'node:fs';

// Exercise the actual shell boundary: realmSceneEvidence stores the sprite's
// projected pixel anchor, while x/y are world coordinates. Menus must share it.
const shell = readFileSync(new URL('../play/gameplay-shell.mjs', import.meta.url), 'utf8');
const body = shell.match(/function realmNpcFeet\(guid\) \{[\s\S]*?\n\}/)?.[0];
assert.ok(body);
test('NPC menu uses its drawn pixel anchor without projecting it a second time', () => {
  const monster = { guid: 8, x: 5800, y: 4400, anchorX: 544, anchorY: 466 };
  const feet = vm.runInNewContext(`(${body})`, { realmSceneEvidence: { monsters: [monster] },
    worldToScreen() { throw new Error('pixel anchor projected twice'); } });
  assert.deepEqual({ ...feet(8) }, { x: 544, y: 466 });
  monster.anchorX = 0; monster.anchorY = 321;
  assert.deepEqual({ ...feet(8) }, { x: 0, y: 321 });
  assert.equal(feet(9), null);
  monster.anchorY = NaN;
  assert.equal(feet(8), null);
});
