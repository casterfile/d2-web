import test from 'node:test';
import assert from 'node:assert/strict';
import { createUnitSoundPlayer, MIXER_TICK_SECONDS } from './unit-sound-player.mjs';

const HEADER = 'Sound\tIndex\tFileName\tVolume\tGroup Size\tLoop\tFade In\tFade Out\tDefer Inst\tStop Inst\tDuration\tCompound\tReverb\tFalloff\tCache\tAsync Only\tPriority\tStream\tStereo\tTracking\tSolo\tMusic Vol';
const line = (name, index, cols = {}) => {
  const c = [name, String(index), `${name}.wav`, '255', ...Array(18).fill('0')];
  for (const [i, v] of Object.entries(cols)) c[i] = String(v);
  return c.join('\t');
};
const TABLE = [HEADER,
  line('hit', 0, { 13: 1 }),                       // an ordinary mono row, Falloff 1, not Tracking
  line('step_1', 1, { 4: 4, 13: 1 }), line('step_2', 2), line('step_3', 3), line('step_4', 4),
  line('fire_loop', 5, { 5: 1, 6: 12, 7: 12, 13: 1, 19: 1 }),
  line('burst', 6, { 10: 25 }),
  line('mute', 7, { 3: 0 }),
  line('near', 8, { 13: 0, 19: 1 }),
  line('wide', 9, { 18: 1, 13: 1 }),
].join('\n');

function wav() {
  const b = new Uint8Array(46), v = new DataView(b.buffer);
  for (const [o, s] of [[0, 'RIFF'], [8, 'WAVE'], [12, 'fmt '], [36, 'data']]) for (let i = 0; i < 4; i++) b[o + i] = s.charCodeAt(i);
  v.setUint32(4, 38, true); v.setUint32(16, 16, true); v.setUint16(20, 1, true); v.setUint16(22, 1, true); v.setUint32(24, 22050, true);
  v.setUint32(28, 44100, true); v.setUint16(32, 2, true); v.setUint16(34, 16, true); v.setUint32(40, 2, true);
  return b;
}

function rig(script = []) {
  const gains = [], sources = [], mergers = [], reads = [];
  class Ctx {
    state = 'running'; destination = {}; currentTime = 10;
    createGain() {
      const ramps = [];
      const node = { ramps, connections: [], connect(to) { node.connections.push(to); }, disconnect() {},
        gain: { value: 1, setValueAtTime(v, t) { ramps.push(['set', v, t]); }, linearRampToValueAtTime(v, t) { ramps.push(['ramp', v, t]); } } };
      gains.push(node); return node;
    }
    createChannelMerger() { const m = { connect() {}, disconnect() {} }; mergers.push(m); return m; }
    resume() { return Promise.resolve(); }
    decodeAudioData() { return Promise.resolve({ duration: 1 }); }
    createBufferSource() {
      const s = { stops: [], connect() {}, disconnect() {}, start() { s.started = true; }, stop(t) { s.stops.push(t); } };
      sources.push(s); return s;
    }
  }
  const bytes = wav(), asked = [];
  const read = (p) => { reads.push(p); return p === 'data\\global\\excel\\Sounds.txt' ? new TextEncoder().encode(TABLE) : bytes; };
  const player = createUnitSoundPlayer(read, Ctx, null, null, { rand: (n) => { asked.push(n); return script.shift() ?? 0; } });
  player.setVolume(1, { sound: 1, music: 1 });
  return { player, gains, sources, mergers, reads, asked };
}
const unitAt = (x, y = 0, id = 5) => ({ entities: [{ type: 1, id, preciseX: x, preciseY: y }] });
const LISTENER = { x: 0, y: 0 };

test('leaving a session cancels a sound still waiting for its WAV decode', async () => {
  const r = rig();
  const pending = r.player.playRow(0);
  r.player.stopAll();
  const rec = await pending;
  assert.equal(rec.cancelled, true);
  assert.equal(rec.started, false);
  assert.ok(r.sources.every((s) => !s.started));
});

test('leaving a session stops its attached loop before another character starts', async () => {
  const r = rig();
  r.player.updateSpatialGain(unitAt(0), LISTENER);
  const rec = await r.player.playRow(5, { unitType: 1, unitId: 5 });
  assert.equal(rec.looping, true);
  r.player.stopAll();
  assert.equal(rec.stopped, 'session-ended');
  assert.equal(r.sources[0].stops.length, 1);
});

test('every mono row takes Falloff, the radial gain and the pan, not only the shrine', async () => {
  const r = rig();
  r.player.updateSpatialGain(unitAt(10), LISTENER);
  const rec = await r.player.playRow(0, { unitType: 1, unitId: 5 });
  assert.equal(rec.started, true);
  assert.deepEqual(rec.clientOffset, { x: 160, y: 160 });
  assert.equal(rec.appliedSpatialGainByte, 240);
  assert.equal(rec.nativeSpatialPanCode, 178, 'playRow pans: spatial is no longer forced off');
  assert.equal(r.mergers.length, 1);
  // Falloff 1 at 226 px: 188; with gain 240: 176.
  const node = r.gains.find((g) => Math.abs(g.gain.value - 176 / 255) < 1e-12);
  assert.ok(node, 'the source node carries channel 176/255');
});

test('a stereo row is neither attenuated by the radial gain nor panned', async () => {
  const r = rig();
  r.player.updateSpatialGain(unitAt(10), LISTENER);
  const rec = await r.player.playRow(9, { unitType: 1, unitId: 5 });
  assert.equal(rec.appliedSpatialGainByte, 255);
  assert.equal(r.mergers.length, 0);
});

test('a Group Size row plays the variant the client seed draws, avoiding the last picks', async () => {
  const r = rig([2, 2, 3]);
  const a = await r.player.playRow(1);
  assert.equal(a.variantRow, 3);
  assert.equal(a.handle, 'step_3');
  assert.ok(r.reads.includes('data\\global\\sfx\\step_3.wav'));
  const b = await r.player.playRow(1);
  assert.equal(b.variantRow, 4, 'the repeat of variant 3 is redrawn');
  assert.deepEqual(r.asked, [4, 4, 4]);
});

test('Loop and Fade In: an attached looping row loops and ramps in over 12 ticks', async () => {
  const r = rig();
  r.player.updateSpatialGain(unitAt(2), LISTENER);
  const rec = await r.player.playRow(5, { unitType: 1, unitId: 5 });
  assert.equal(rec.looping, true);
  assert.equal(r.sources[0].loop, true);
  assert.equal(rec.fadeInSeconds, 12 * MIXER_TICK_SECONDS);
  const node = r.gains.find((g) => g.ramps.length);
  assert.deepEqual(node.ramps.map(([k, v, t]) => [k, k === 'set' ? v : 'target', Number(t.toFixed(6))]), [['set', 0, 10], ['ramp', 'target', 10.48]]);
  // Its unit freed: the loop stops through Fade Out.
  r.player.stopUnitSounds(1, 5);
  assert.equal(rec.stopped, 'unit-freed');
  assert.equal(rec.fadeOutSeconds, 0.48);
  assert.deepEqual(r.sources[0].stops.map((t) => Number(t.toFixed(6))), [10.48]);
});

test('a looping row with nothing to stop it plays once', async () => {
  const r = rig();
  const rec = await r.player.playRow(5);
  assert.equal(rec.looping, false);
  assert.equal(rec.loopHeld, true);
  assert.equal(r.sources[0].loop, false);
});

test('a looping source stops when its unit leaves the snapshot', async () => {
  const r = rig();
  r.player.updateSpatialGain(unitAt(2), LISTENER);
  const rec = await r.player.playRow(5, { unitType: 1, unitId: 5 });
  r.player.updateSpatialGain({ entities: [] }, LISTENER);
  assert.equal(rec.stopped, 'unit-freed');
});

test('Duration stops the source after Duration + 1 ticks', async () => {
  const r = rig();
  const rec = await r.player.playRow(6);
  assert.equal(rec.durationSeconds, 26 * MIXER_TICK_SECONDS);
  assert.deepEqual(r.sources[0].stops.map((t) => Number(t.toFixed(6))), [11.04]);
});

test('Volume 0 never plays; a source past its cutoff never starts', async () => {
  const r = rig();
  const mute = await r.player.playRow(7);
  assert.equal(mute.silent, true);
  r.player.updateSpatialGain(unitAt(30), LISTENER);
  const far = await r.player.playRow(8, { unitType: 1, unitId: 5 });
  assert.equal(far.outOfRange, true);
  assert.equal(r.sources.length, 0);
});

test('a Tracking source follows its unit and stops at once past its cutoff', async () => {
  const r = rig();
  r.player.updateSpatialGain(unitAt(2), LISTENER);
  const rec = await r.player.playRow(8, { unitType: 1, unitId: 5 });
  assert.equal(rec.started, true);
  r.player.updateSpatialGain(unitAt(10), LISTENER);
  assert.deepEqual(rec.clientOffset, { x: 160, y: 160 });
  assert.equal(rec.appliedSpatialGainByte, 240);
  assert.equal(rec.stopped, undefined);
  r.player.updateSpatialGain(unitAt(30), LISTENER);
  assert.equal(rec.stopped, 'out-of-range');
  assert.equal(rec.fadeOutSeconds, 0);
  assert.deepEqual(r.sources[0].stops, [undefined]);
});
