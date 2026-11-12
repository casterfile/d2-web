import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { createRepairConfirm, repairConfirmMenu, refusalString, REFUSAL_STRINGS, CONTINUE_STRING, REPAIR_TITLE_STRING, GOLD_STRING, YES_STRING, NO_STRING } from '../play/assets/vendor-repair-confirm.mjs';
import { handleVendorRepairClick, disarmVendorRepair, vendorRepairArmed } from '../play/assets/vendor-repair-control.mjs';

test('the repair item click only opens the dialog; Yes sends once, No sends nothing', () => {
  const sent = [];
  const c = createRepairConfirm((id) => sent.push(id));
  assert.equal(c.request(7), true);
  assert.deepEqual(sent, []);
  assert.equal(c.request(8), false);      // modal: a second click cannot replace the pending item
  assert.equal(c.yes(), true);
  assert.deepEqual(sent, [7]);
  assert.equal(c.yes(), false);
  c.request(9); assert.equal(c.no(), true); assert.deepEqual(sent, [7]);
  assert.equal(c.open, false);
  assert.equal(c.request(-1), false);
});

test('string ids are 0x47500 state 3: Repair, Gold:, Yes, No', () => {
  assert.deepEqual([REPAIR_TITLE_STRING, GOLD_STRING, YES_STRING, NO_STRING], [0xd17, 0xd12, 0xd10, 0xd11]);
});

test('the menu is title, item, Gold:, amount (gold lines) then Yes/No (selectable); no entry field', () => {
  const tbl = { 3351: 'Repair', 3346: 'Gold:', 3344: 'Yes', 3345: 'No' };
  const m = repairConfirmMenu('Short Sword', 37, (id) => tbl[id]);
  assert.deepEqual(m.heads, ['Repair', 'Short Sword', 'Gold:', '37']);
  assert.deepEqual(m.entries, [{ stringId: 3344, action: 'yes' }, { stringId: 3345, action: 'no' }]);
  // A two-line name: 0x47500 keeps the text before 0x914F0's first newline, the bottom-drawn (base) line.
  assert.equal(repairConfirmMenu('The Gnasher\nHand Axe', 5, () => '').heads[1], 'Hand Axe');
});

test('an item the 0x45B40 gate rejects never opens the dialog', () => {
  const seen = [];
  const c = createRepairConfirm(() => {}, (id) => { seen.push(id); return id === 2; });
  assert.equal(c.request(1), false); assert.equal(c.open, false);
  assert.equal(c.request(2), true); assert.equal(c.open, true);
  assert.deepEqual(seen, [1, 2]);
});

test('the confirm no longer reuses the stash-gold amount box; Yes reaches the 0x35 repair command', () => {
  const mod = readFileSync('play/assets/vendor-repair-confirm.mjs', 'utf8');
  assert.doesNotMatch(mod, /stash-gold-dialog/);
  const shell = readFileSync('play/gameplay-shell.mjs', 'utf8');
  assert.doesNotMatch(shell, /repairAtVendor|d2_session_command_vendor_repair/);   // one view: no engine repair path
  // Both modes take the packet path: the click opens the Repair box with the price and sends 0x35 only on Yes.
  const carried = shell.slice(shell.indexOf('function realmCarriedClick('));
  assert.match(carried.slice(0, 800), /d2gsClientItemPrice\?\.\(guid, view\.classId, 3\)/);
  assert.match(carried.slice(0, 800), /askRepair\(guid, name, cost < 0 \? 0 : cost, send\)/);
  const src = readFileSync('src/core/monspawn/monspawn_check.cpp', 'utf8');
  assert.match(src.slice(src.indexOf('int d2_session_command_vendor_repair(')).slice(0, 400), /packetHeader = 0x35/);
  // The amount is 0x46230 in the repair cursor state (uiState 4: transaction type 3).
  const cost = src.slice(src.indexOf('int32_t d2_session_item_repair_cost('));
  assert.match(cost.slice(0, 1500), /d2_client_46230\(&amt, item, 1, line, 64, 4, 0, &cb\)/);
});

test('0x4B5D0: a reply above 6 is a refusal; 9 is 3361, 12 (not enough gold) is 3362, Continue is 3403', () => {
  for (let c = -1; c <= 6; c++) assert.equal(refusalString(c), 0);
  assert.equal(refusalString(9), 3361);
  assert.equal(refusalString(12), 3362);
  assert.equal(refusalString(17), 0xd21);
  assert.equal(CONTINUE_STRING, 0xd4b);
  // The engine's table is the same.
  const cpp = readFileSync('src/core/d2client/d2client_4b5d0_refusal.cpp', 'utf8');
  assert.match(cpp, new RegExp(REFUSAL_STRINGS.map((v) => '0x' + v.toString(16)).join(', ')));
});

test('a refusal replaces the confirm, blocks a new request, and Continue (yes or no) closes it', () => {
  const sent = [];
  const c = createRepairConfirm((id) => { sent.push(id); c.refuse(refusalString(12)); });
  c.request(4); c.yes();
  assert.deepEqual(sent, [4]);
  assert.equal(c.open, true); assert.equal(c.refusal, 3362); assert.equal(c.itemId, -1);
  assert.equal(c.request(5), false);
  assert.equal(c.yes(), true); assert.equal(c.open, false);
  c.refuse(3361); assert.equal(c.no(), true); assert.equal(c.open, false);
  assert.equal(c.refuse(0), false); assert.equal(c.open, false);
});

test('both repair commands read the 0x2A result and open the refusal box', () => {
  const shell = readFileSync('play/gameplay-shell.mjs', 'utf8');
  assert.doesNotMatch(shell, /function repairAtVendor\(|function repairAllAtVendor\(|takeTradeResult/);   // the realm screen owns the refusal box
  const stubs = readFileSync('src/core/d2moo/compat/game_net_stubs.cpp', 'utf8');
  assert.match(stubs, /g_d2LastTransactionResult = \(uint8_t\)result;/);
});

test('needs-repair is the whole 0x45B40 rule: type 4, IsRepairable, durability, stackable, used charges', () => {
  const src = readFileSync('src/core/d2client/item_needs_repair.h', 'utf8');
  const body = src.slice(src.indexOf('d2client_item_needs_repair(')).slice(0, 1400);
  assert.match(body, /ITEMS_GetItemType\(item\) == 4 \|\| !ITEMS_IsRepairable\(item\)/);
  assert.match(body, /ITEMS_HasDurability\(item\)/);
  assert.match(body, /ITEMS_CheckIfStackable\(item\)/);
  assert.match(body, /ITEMS_HasUsedCharges\(item, &full\) && !full/);
  assert.doesNotMatch(body, /ITEMS_CheckIfThrowable/);
});

test('Repair mode survives Yes and a repair; No and the refusal Continue end it (0x94970, 0x461D0, 0x8F630)', () => {
  disarmVendorRepair();
  const c = createRepairConfirm(() => {}, () => true, disarmVendorRepair);
  handleVendorRepairClick('repair', undefined, () => {});
  assert.equal(vendorRepairArmed(), true);
  assert.equal(c.request(3), true); c.yes();
  assert.equal(vendorRepairArmed(), true);           // success: the hammer stays
  handleVendorRepairClick('itemOnPanel', 4, () => {});
  assert.equal(vendorRepairArmed(), true);
  c.request(5); c.no();
  assert.equal(vendorRepairArmed(), false);          // No ends Repair mode
  handleVendorRepairClick('repair', undefined, () => {});
  c.refuse(0xd22); c.no();
  assert.equal(vendorRepairArmed(), false);          // Continue (click) ends it
  handleVendorRepairClick('repair', undefined, () => {});
  c.refuse(0xd21); c.yes();
  assert.equal(vendorRepairArmed(), false);          // Continue (Enter) ends it
  assert.equal(c.no(), false);                       // nothing open: no end
  const shell = readFileSync(new URL('../play/gameplay-shell.mjs', import.meta.url), 'utf8');
  assert.doesNotMatch(shell, /createTradeConfirm\(/);
});
