// Town NPC footfalls: a unit held in its walk mode while it stands makes no footstep; the same unit walking does.
import test from 'node:test';
import assert from 'node:assert/strict';
import { createEventSounds } from './event-sounds.mjs';

const enc = (s) => new TextEncoder().encode(s);
const files = {
  'data\\global\\excel\\Sounds.txt': enc('Sound\tIndex\nmedium_walk_dirt_1\t2768\n'),
  'data\\global\\excel\\armor.txt': enc('name\tcode\tdropsound\tdropsfxframe\n'),
  'data\\global\\excel\\weapons.txt': enc('name\tcode\tdropsound\tdropsfxframe\n'),
  'data\\global\\excel\\misc.txt': enc('name\tcode\tdropsound\tdropsfxframe\n'),
};
const read = (p) => { if (!files[p]) throw new Error(`no ${p}`); return files[p]; };

// Akara (class 148) in walk mode 2, her frame advancing 256 a tick over a 4096 cycle; `step` subtiles moved per tick.
function run(step, ticks = 12) {
  const got = [], asked = [];
  // The engine's due predicate stand-in: every call is a footfall, so only the movement gate decides.
  const es = createEventSounds(read, (row, e) => got.push({ row, ...e }), (...args) => { asked.push(args); return 2768; });
  for (let t = 0; t < ticks; t++) {
    const x = 5000 + t * step;
    es.update({ level: 1, entities: [{ type: 1, id: 3, classId: 148, preciseX: x, preciseY: 7000,
      animation: { inClientRoom: true, mode: 2, classId: 148, framePrecise: (t * 256) % 4096, frameCountPrecise: 4096, animSpeed: 256 } }] }, 1);
  }
  return { got, asked };
}

test('a town NPC standing in its walk mode makes no footstep', () => {
  const { got, asked } = run(0);
  assert.equal(asked.length, 0, 'the footfall callback is never asked for a unit that did not move');
  assert.deepEqual(got.filter((e) => e.kind === 'footstep'), []);
});

test('the same NPC walking steps, attached to its unit for the mixer', () => {
  const { got, asked } = run(1);
  assert.equal(asked.length, 11, 'asked once per tick after the first');
  const steps = got.filter((e) => e.kind === 'footstep');
  assert.equal(steps.length, 11);
  assert.deepEqual(steps[0], { row: 2768, kind: 'footstep', id: 3, unitId: 3, unitType: 1, classId: 148 });
});

test('a packet Necromancer without a hero entity uses his light footstep group', () => {
  const got = [];
  const es = createEventSounds(read, (row, e) => got.push({ row, ...e }));
  for (let t = 0; t < 3; t++) es.update({ playerClass: 2, playerId: 1,
    player: { mode: 6 }, entities: [] }, 1);
  assert.equal(got[0].row, 2720);
  assert.equal(got[0].classId, 2);
});
