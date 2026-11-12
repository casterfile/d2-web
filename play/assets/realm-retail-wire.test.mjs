import test from 'node:test';
import assert from 'node:assert/strict';
import { packets } from './realm-input.mjs';
import { npcPackets } from './realm-npc-packets.mjs';

// Bytes the 1.13c client hands to its send thunk for fixed inputs (recorded headlessly, deterministic across two passes).
const hex = (s) => Uint8Array.from(s.trim().split(/\s+/).map((h) => parseInt(h, 16)));
const same = (ours, retail) => assert.deepEqual([...ours], [...hex(retail)]);

test('belt use 0x26: item guid, onMerc flag, zero word', () => {
  same(packets.useBeltItem(0x0a0b0c0d, false), '26 0d 0c 0b 0a 00 00 00 00 00 00 00 00');
  same(packets.useBeltItem(0x0a0b0c0d, true), '26 0d 0c 0b 0a 01 00 00 00 00 00 00 00');
});

test('belt lift 0x24 and place 0x23 and swap 0x25', () => {
  same(packets.beltLift(0x0a0b0c0d), '24 0d 0c 0b 0a');
  same(packets.beltPlace(0x0a0b0c0d, 3), '23 0d 0c 0b 0a 03 00 00 00');
  same(packets.beltSwap(0x0a0b0c0d, 0x0e0f1011), '25 0d 0c 0b 0a 11 10 0f 0e');
});

test('skill select 0x3c, right hand: skill word then owner -1', () => {
  same(packets.selectSkill(5, false, 0xffffffff), '3c 05 00 00 00 ff ff ff ff');
});

// Retail sends the left-hand skill as the movsx word of a short whose bit 15 is the hand flag: ECX = 0xFFFF8005
// (vectors/d2client/fn/14910.wire.bin, tuple 0x3C 0xFFFF8005 0xFFFFFFFF) gives 3c 05 80 ff ff ff ff ff ff.
test('skill select 0x3c, left hand: sign-extended word 0xFFFF8005, owner -1', () => {
  same(packets.selectSkill(5, true, 0xffffffff), '3c 05 80 ff ff ff ff ff ff');
});

// Skill-hotkey bind 0x51: retail's D2Client 0x14910 with AL=0x51, ECX = ((slot&15)<<16)|(left?0x8000:0)|(skill&0x7FFF), EDX = owner.
// The record (vectors/d2client/fn/14910.hotkey.bin, private store) holds FNV(size 9, bytes) of the emitted packet, not the bytes;
// the hashes below are that record's, and fnv9 recomputes them from the bytes the builder writes.
const fnv9 = (bytes) => { let h = Math.imul(0x811C9DC5 ^ bytes.length, 0x01000193) >>> 0; for (const x of bytes) h = Math.imul(h ^ x, 0x01000193) >>> 0; return h >>> 0; };
test('skill hotkey bind 0x51: slot 3 skill 5 right, slot 3 skill 5 left, skill 0x96 and Whirlwind 0x97 left (retail 14910 record)', () => {
  const cases = [
    [packets.bindHotkey(3, 5, false), '51 05 00 03 00 ff ff ff ff', 0x244A360D],
    [packets.bindHotkey(3, 5, true), '51 05 80 03 00 ff ff ff ff', 0x0BE5C38D],
    [packets.bindHotkey(0, 0x96, true), '51 96 80 00 00 ff ff ff ff', 0xCE13C8F5],
    [packets.bindHotkey(0, 0x97, true), '51 97 80 00 00 ff ff ff ff', 0xE39E08D4],
    [packets.bindHotkey(15, 0x1234, false, 7), '51 34 12 0f 00 07 00 00 00', 0x108CDEE1],   // blind holdout
  ];
  for (const [bytes, hex8, hash] of cases) {
    same(bytes, hex8);
    assert.equal(fnv9(bytes), hash);
  }
});

test('drop gold 0x50: hero guid then amount 500', () => {
  same(packets.dropGold(0x01020304, 500), '50 04 03 02 01 f4 01 00 00');
});

test('NPC buy 0x32 and sell 0x33: 17 bytes, four little-endian dwords after the opcode', () => {
  same(npcPackets.buy(0x01020304, 0x0a0b0c0d, 2, 0x1234, { gamble: true, multi: true }),
    '32 04 03 02 01 0d 0c 0b 0a 02 00 02 80 34 12 00 00');
  same(npcPackets.sell(7, 9, 4), '33 07 00 00 00 09 00 00 00 04 00 00 00 00 00 00 00');
});
