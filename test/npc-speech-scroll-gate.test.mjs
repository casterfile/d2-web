import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { createNpcSpeechScrollGate } from '../play/assets/npc-speech-scroll-gate.mjs';

const deferred = () => {
  let resolve;
  const promise = new Promise((done) => { resolve = done; });
  return { promise, resolve };
};

test('voice completion gates scrolling, while ordinary subtitles ignore the audio mode', () => {
  const shell = readFileSync(new URL('../play/gameplay-shell.mjs', import.meta.url), 'utf8');
  const box = readFileSync(new URL('../play/assets/speech-box.mjs', import.meta.url), 'utf8');
  assert.match(shell, /waitForVoice\(token, playback, unitSounds\)/);
  assert.match(shell, /syncNpcDialogueVoice\(\{ npcId: npc\.guid, spoken: npc\.speech\.stringId, messageInstance: npc\.speechInstance \}\)/);   // every packet speech, including a replay
  assert.doesNotMatch(shell, /npcState/);
  assert.match(shell, /speechBox\.draw\(ctx, pendingSpeech\.text, undefined,\s*\{ messageInstance: pendingSpeech\.messageInstance \}\)/);
  assert.doesNotMatch(shell, /speechT0/);
  // Native FUN_6FB2BB40 draws its container and text in one call. Ordinary FUN_6FB2D200 speech
  // sets the active-speech flag, so that renderer admits subtitles in Audio Only as well.
  const draw = box.slice(box.indexOf('draw(ctx, text'), box.indexOf('\n    },', box.indexOf('draw(ctx, text')));
  assert.ok(draw.indexOf('ctx.fillRect(') >= 0);
  assert.ok(draw.indexOf('ctx.fillRect(') < draw.indexOf('font.drawAnchored('));
});

test('speech box opens when the voice starts, not when it finishes', async () => {
  const gate = createNpcSpeechScrollGate();
  const playback = deferred(), voice = deferred();
  const token = gate.begin();
  const waiting = gate.waitForVoice(token, playback.promise, { whenDialogueDone: () => voice.promise });
  assert.equal(gate.ready, false);
  playback.resolve({ started: true });
  await waiting;
  assert.equal(gate.ready, true);
});

test('unplayed and stale voices cannot hold or reopen a later speech clock', async () => {
  const gate = createNpcSpeechScrollGate();
  const stale = deferred();
  const oldToken = gate.begin();
  const waiting = gate.waitForVoice(oldToken, stale.promise, { whenDialogueDone: async () => {} });
  gate.cancel();
  const newToken = gate.begin();
  stale.resolve({ started: false });
  await waiting;
  assert.equal(gate.ready, false);

  await gate.waitForVoice(newToken, Promise.resolve({ started: false }));
  assert.equal(gate.ready, true);
});
