import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';

const shell = readFileSync(new URL('../gameplay-shell.mjs', import.meta.url), 'utf8');

test('skills, belt, movement, stats and objects have no engine branch beside the packets', () => {
  for (const gone of [
    'd2_session_command_move', 'd2_session_command_left_skill', 'd2_session_command_right_skill',
    'd2_session_command_select_skill', 'd2_session_command_swap_weapons', 'd2_session_command_primary_attack',
    'd2_session_command_throw', 'd2_session_command_kick', 'd2_session_command_use_belt',
    'd2_session_command_cursor_item_packet', 'd2_session_command_insert_item_packet',
    'd2_session_command_item_to_cursor_packet', 'd2_session_command_equip_packet', 'd2_session_command_drop',
    'd2_session_command_add_stat', 'd2_session_command_add_skill', 'd2_session_command_button',
    'd2_session_command_object', 'd2_session_client_pickup', 'clickPickup', 'session.operate', 'session.swingLeftHand', 'gridToBelt(', 'beltToCursor(',
  ])
    assert.equal(shell.replace(/\/\/.*$/gm, '').includes(gone), false, `${gone} is gone from the shell`);
  assert.match(shell, /clientView\(\)\.commands\.selectSkill\?\.\(id, side === 'left'\)/);
});
