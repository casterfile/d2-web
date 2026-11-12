// Closing the Horadric Cube window. The Escape key and a click on the cube's close cel both run closeCubeFromPanel,
// which sends the retail TRADEBTN_CLOSECUBE command (23) and closes the window only when the engine accepts it: the
// client never closes a panel on a pointer hit alone, the same rule stash-close.mjs keeps for the stash.
export const TRADEBTN_CLOSECUBE = 23;

// The cube's close cel: 32x32 at game px (352, 445), the cel the stash's close draws. The cube's own hit bounds are
// not recovered; the stash's are the engine's hit test (stash-close.mjs), which covers x 353..391, y 442..480 there.
export const CUBE_CLOSE = { x: 352, y: 445, w: 32, h: 32 };

// True when a canvas point is on the cube's close cel. `visible` is the cube's own flag (open and its inventory up).
export function cubeCloseAt(px, py, width, height, visible) {
  if (!visible || width <= 0 || height <= 0) return false;
  const scale = Math.min(1, width / 800, height / 600);
  const x = (px - (width - 800 * scale) / 2) / scale;
  const y = (py - (height - 600 * scale) / 2) / scale;
  return x >= CUBE_CLOSE.x && x < CUBE_CLOSE.x + CUBE_CLOSE.w && y >= CUBE_CLOSE.y && y < CUBE_CLOSE.y + CUBE_CLOSE.h;
}

// The one close both paths run: the command, then the window closes on acceptance. Returns whether it closed.
export function closeCubeFromPanel({ cubeOpen, commandButton, onAccepted }) {
  if (!cubeOpen) return false;
  const accepted = Boolean(commandButton(TRADEBTN_CLOSECUBE, 0));
  if (accepted) onAccepted();
  return accepted;
}
