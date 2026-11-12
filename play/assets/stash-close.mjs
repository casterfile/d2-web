// Closing the inventory while the stash paired it with must use the same retail
// TRADEBTN_CLOSESTASH command as Esc. The client changes presentation only after
// the engine accepts that command; it never treats the pointer hit as authority.
export const TRADEBTN_CLOSESTASH = 18;

export function closeStashFromInventory({ stashOpen, commandButton, onAccepted }) {
  if (!stashOpen) return false;
  const accepted = Boolean(commandButton(TRADEBTN_CLOSESTASH, 0));
  if (accepted) onAccepted();
  return accepted;
}

// The stash's own close button. Whether a point is on it is the engine's hit test
// (src/core/d2client/d2client_8cd00.cpp, exported as d2_client_stash_close_hit):
// hit(x, y, originX, originY, screenHeight, expansion) -> 1 on the button. The game draws in
// 800x600, where retail's panel origin is (80, -60); the button then covers x 353..391, y 442..480
// in an expansion game and x 356..394, y 441..479 in a classic one.
export const STASH_PANEL_ORIGIN = Object.freeze({ x: 80, y: -60, height: 600 });

export function stashCloseAt(px, py, width, height, visible, hit, expansion = true) {
  if (!visible || width <= 0 || height <= 0 || typeof hit !== 'function') return false;
  const scale = Math.min(1, width / 800, height / 600);
  const x = Math.floor((px - (width - 800 * scale) / 2) / scale);
  const y = Math.floor((py - (height - 600 * scale) / 2) / scale);
  const { x: ox, y: oy, height: sh } = STASH_PANEL_ORIGIN;
  return hit(x, y, ox, oy, sh, expansion ? 1 : 0) === 1;
}
