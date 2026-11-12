// The stash and the Horadric Cube in a joined game, driven by the same panels a local game draws. A click on
// either panel becomes the retail item packets: lift (0x19), put in a cell (0x18) or swap with the item under
// it (0x1F) with the page of the grid (stash 4, cube 3), and the cube's Transmute button is 0x4F button 24.
// A shift+click on a carried item sends it to the belt (0x63). Everything else (the server's answer, the new
// item positions) comes back as item packets; nothing here moves an item locally.
// The trade screen is the same routing on page 2 (the player's own offer); a press on the rest of that panel is
// its buttons' (assets/realm-trade.mjs), and the partner's grid takes no press.
import { itemPackets, BUTTON } from './realm-item-packets.mjs';
import { INV_PAGE } from './realm-input.mjs';
import { decideGridPlace } from './grid-place.mjs';

/**
 * `ctx`: { width, height, items, cursorId, input (createRealmInput), send(bytes), inventory: {visible, renderer},
 * stash: {open, renderer, layout}, cube: {open, renderer, layout}, trade: {open, renderer, layout, press(x, y, {cursorItem})} }. Returns true when the click was the panels'.
 */
export function realmStorageClick(ctx, x, y, { button = 0, shift = false } = {}) {
  if (button !== 0) return false;
  const { width: W, height: H, items, cursorId, input: ri } = ctx;
  const held = () => items.find((i) => i.id === cursorId) || { id: cursorId };
  const grid = (panel, layout, page) => {
    if (!panel.renderer || !layout || !panel.renderer.contains(x, y, W, H, layout)) return false;
    if (!cursorId) { const item = panel.renderer.itemAt(x, y, W, H, layout, items); if (item) ri.lift(item.id); return true; }
    const cell = panel.renderer.cellAt(x, y, W, H, layout, held());
    if (!cell) return true;
    const d = decideGridPlace(items, page, cell.x, cell.y, held());
    if (d.action === 'swap') ri.swapGrid(cursorId, d.item.id, cell.x, cell.y);
    else if (d.action === 'put') ri.putInGrid(cursorId, cell.x, cell.y, page);
    else if (d.action !== 'refuse') ri.combine(d.action, cursorId, d.item.id);
    return true;
  };
  const trade = ctx.trade;
  if (trade?.open && trade.renderer && trade.layout && trade.renderer.contains(x, y, W, H, trade.layout)) {
    if (trade.renderer.cellAt(x, y, W, H, trade.layout, null)) return grid(trade, trade.layout, INV_PAGE.TRADE);
    trade.press(x, y, { cursorItem: Boolean(cursorId) });
    return true;
  }
  const cube = ctx.cube;
  if (cube.open && ctx.inventory.visible && cube.renderer && cube.layout && cube.renderer.contains(x, y, W, H, cube.layout)) {
    if (cube.renderer.transmuteAt(x, y, W, H, cube.layout)) { ctx.send(itemPackets.button(BUTTON.TRANSMUTE)); return true; }
    return grid(cube, cube.layout, INV_PAGE.CUBE);
  }
  if (ctx.stash.open && grid(ctx.stash, ctx.stash.layout, INV_PAGE.STASH)) return true;
  const inv = ctx.inventory;
  if (shift && !cursorId && inv.visible && inv.renderer?.contains(x, y, W, H)) {
    const item = inv.renderer.itemAt(x, y, W, H, ctx.snapshot);
    if (item) { ctx.send(itemPackets.shiftToBelt(item.id)); return true; }
  }
  return false;
}

/**
 * The open inventory panel in a joined game, hit-tested by the same panel renderer a local game draws: lift a carried
 * item (0x19) or take off a worn one (0x1C); with an item held, put it in a cell (0x18), swap it with the one item
 * under it (0x1F), wear it (0x1A) or swap it with the worn one (0x1D); a right click uses a carried item (0x20); the
 * weapon tab swaps sets (0x60). A left click outside every open panel with an item held drops it (0x17). The items
 * move only when the server answers with its item packets.
 * `ctx`: { width, height, snapshot, cursorId, input (createRealmInput), visible, renderer, openPanelAt(x, y), onWeaponTab() }.
 * Returns true when the click was the panel's or dropped the held item.
 */
export function realmInventoryClick(ctx, x, y, button = 0) {
  const { width: W, height: H, snapshot, cursorId, input: ri, renderer: panel } = ctx;
  if (!(ctx.visible && panel?.contains(x, y, W, H))) {
    if (button !== 0 || !cursorId || ctx.openPanelAt?.(x, y)) return false;
    ri.dropCursor(cursorId); return true;
  }
  const slot = panel.slotAt(x, y, W, H, snapshot);
  if (button === 0 && panel.weaponTabAt(x, y, W, H)) { ctx.onWeaponTab?.(); ri.swapWeapons(); return true; }
  if (button === 2) { const item = panel.itemAt(x, y, W, H, snapshot) || slot?.item; if (item && !cursorId) ri.useItem(item.id); return true; }
  // The identify cursor (D2Client [0x11B864] == 6): a press on an item sends 0x27 for it, any other press does nothing (0x95FE0, 0x96AA0).
  if (button === 0 && ctx.armed) { const item = panel.itemAt(x, y, W, H, snapshot) || slot?.item; if (item) ri.useOnItem(item.id, ctx.armed); return true; }
  if (!cursorId) {
    const item = panel.itemAt(x, y, W, H, snapshot);
    if (item) ri.lift(item.id); else if (slot?.item) ri.unequip(slot.bodyLoc);
    return true;
  }
  const held = snapshot.items.find((i) => i.id === cursorId) || { id: cursorId };
  const cell = panel.cellAt(x, y, W, H, held);
  if (slot) { if (slot.item) ri.swapBody(cursorId, slot.bodyLoc); else ri.equip(cursorId, slot.bodyLoc); }
  else if (cell) {
    const d = decideGridPlace(snapshot.items, 0, cell.x, cell.y, held);
    if (d.action === 'swap') ri.swapGrid(cursorId, d.item.id, cell.x, cell.y);
    else if (d.action === 'put') ri.putInGrid(cursorId, cell.x, cell.y);
    else if (d.action !== 'refuse') ri.combine(d.action, cursorId, d.item.id);
  }
  return true;
}
