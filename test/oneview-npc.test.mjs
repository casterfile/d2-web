import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';

// One view: NPCs, vendors, waypoints and travel leave through the realm path in single player too.
const shell = readFileSync(new URL('../play/gameplay-shell.mjs', import.meta.url), 'utf8');

test('the shell calls no engine NPC, vendor, hire, waypoint, travel or respawn command', () => {
  for (const name of ['d2_session_command_npc', 'd2_session_command_nearest_npc', 'd2_session_command_end_npc', 'd2_session_npc_response',
    'd2_session_command_vendor', 'd2_session_command_hire', 'd2_session_command_waypoint', 'd2_session_command_travel',
    'd2_session_nearest_travel_npc', 'd2_session_command_nearest_warp', 'd2_session_command_respawn', 'd2_session_command_npc_response_packet']) {
    assert.ok(!shell.includes(name), name);
  }
});

test('no engine NPC dialogue state is read or branched on', () => {
  for (const re of [/\bnpcState\b/, /\breadNpcState\b/, /\bsyncShopInventory\b/, /\bbuyFromVendor\b/, /\bgambleAtVendor\b/, /\bsellToVendor\b/,
    /\bchooseNpcMenu\b/, /\bchooseHire\b/, /\bopenHireWindow\b/, /\bvendorButtonGesture\b/]) assert.doesNotMatch(shell, re);
});

test('shop and NPC panels read the realm view', () => {
  assert.match(shell, /const shopOpen = \(\) => Boolean\(realmShopView\(\)\)/);
  assert.match(shell, /realmInput\(\)\.interactNearest\(\)/);
  assert.match(shell, /realmInput\(\)\.waypoint\(waypointMenu\.id, levelId\)/);
});
