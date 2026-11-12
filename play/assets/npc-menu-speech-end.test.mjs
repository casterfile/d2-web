import test from 'node:test';
import assert from 'node:assert/strict';
import { nextSpeechAfterTopic } from './npc-menu.mjs';

// Akara's list [0x0b, 0x40, topic 0x41]: the speech end speaks the first message other than the one just spoken.
test('speech end after a Talk topic: first other message, the Talk box only when none', () => {
  const list = [{ stringId: 0x0b }, { stringId: 0x40 }];
  assert.equal(nextSpeechAfterTopic(list, 0x41), 0x0b);
  assert.equal(nextSpeechAfterTopic(list, 0x0b), 0x40);
  assert.equal(nextSpeechAfterTopic(list, 0x40), 0x0b);
  assert.equal(nextSpeechAfterTopic([{ stringId: 0x41 }], 0x41), null);
});
