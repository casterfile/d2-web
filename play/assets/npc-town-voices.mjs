// Introduction / Gossip voices of the Act 2-5 town NPCs: StringIds.h numbers each NPC's lines in one run
// (<Npc>IntroGossip1, the class introductions, the class gossips, then Gossip1..n) and Sounds.txt names the same
// run <npc>_act<N>_intro, _intro_<cls>, _gossip_<cls>, _gossip_1.._gossip_n, in that order -- the order Act 1 and
// Act 3's Ormus already follow in npc-dialogue-voice.mjs. Row: [first string id, handle stem, intro suffixes,
// class gossip suffixes, plain gossip count]. Without these the Act 2-5 introductions spoke text but no voice.
const ROWS = [
  [190, 'greiz_act2', [], [], 12], [203, 'elzix_act2', ['nec'], ['nec'], 9], [221, 'atma_act2', [], [], 8],
  [230, 'geglash_act2', ['bar'], ['bar'], 8], [241, 'meshif_act2', ['ama'], ['ama'], 9],
  [263, 'fara_act2', ['pal'], ['pal'], 8], [274, 'lysander_act2', [], [], 10], [285, 'drognan_act2', ['sor'], ['sor'], 9],
  [453, 'natalya_act3', [], [], 4], [458, 'cain_act3', [], [], 6], [465, 'hratli_act3', ['sor'], ['sor'], 10],
  [478, 'meshif_act3', ['bar'], ['bar'], 9], [490, 'asheara_act3', ['ama'], ['ama'], 8], [501, 'alkor_act3', ['nec'], ['nec'], 10],
  [666, 'cain_act4', [], [], 1],
  [20002, 'ancient_act5', [], [], 0], [20003, 'cain_act5', [], [], 10], [20014, 'anya_act5', [], [], 10],
  [20025, 'larzuk_act5', ['amazon'], ['amazon1', 'amazon2'], 8], [20037, 'malah_act5', ['sor', 'bar'], ['sor', 'bar'], 11],
  [20053, 'nihlathak_act5', ['ass', 'nec'], ['ass', 'nec'], 7], [20065, 'qualkehk_act5', ['pal', 'dru'], ['pal', 'dru'], 7],
];
export const NPC_TOWN_VOICE = {};
for (const [first, stem, intros, gossips, plain] of ROWS) {
  const handles = [`${stem}_intro`, ...intros.map((s) => `${stem}_intro_${s}`), ...gossips.map((s) => `${stem}_gossip_${s}`)];
  for (let n = 1; n <= plain; n++) handles.push(`${stem}_gossip_${n}`);
  handles.forEach((handle, i) => { NPC_TOWN_VOICE[first + i] = handle; });
}
// Jerhyn's run does not line up one-to-one (3 introduction strings, 2 introduction sounds). Retail's selector table
// gives 255 and 256 the same Sounds row (jerhyn_act2_gossip_1), so 255 speaks it too.
Object.assign(NPC_TOWN_VOICE, { 253: 'jerhyn_act2_intro', 254: 'jerhyn_act2_intro_more', 255: 'jerhyn_act2_gossip_1' });
for (let n = 1; n <= 7; n++) NPC_TOWN_VOICE[255 + n] = `jerhyn_act2_gossip_${n}`;
// Retail omits Alkor's fourth gossip string (507): it has no selector entry.
delete NPC_TOWN_VOICE[507];
Object.freeze(NPC_TOWN_VOICE);
