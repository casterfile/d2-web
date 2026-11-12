// Retail's item-spell targeting, the client half, on the packet path in a local game and a joined one alike.
// A right click on a usable inventory item sends 0x20. For an Identify scroll or tome the server answers S2C 0x3F
// {cursor u8, item id u32, skill u16 = 0xFFFF}: D2Client 0x86A40 sets the cursor state [0x11B864] to 6 and keeps the
// item in [0x11B834], and the pointer is then the identify icon. The next press on an item in the inventory sends
// C2S 0x27 [target item id u32][armed item id u32] (D2Client 0x95FE0 through 0x14910; D2Game Rcv0x27 is 0x6FC85B50,
// 9 bytes). The same id twice is the cancel (0xBEB90 sends it on a click in the world; sub_6FC47470 clears the
// targeting and answers 0x3F 0xFF/0xFFFF). The armed state is the server's: realm-world holds it from 0x3F.
/**
 * io.armed() is the id of the item whose spell waits for a target, or 0; io.send(targetId, useId) sends the 0x27.
 */
export function createItemTargeting(io) {
  return {
    get armed() { return io.armed() >>> 0; },
    /** A press on an item in the inventory: true when it was the targeting's (the caller then does nothing else). */
    leftClick(targetId) { const armed = io.armed() >>> 0; if (!armed || !targetId) return false; io.send(targetId, armed); return true; },
    /** Escape: the client's cancel. */
    cancel() { const armed = io.armed() >>> 0; if (!armed) return false; io.send(armed, armed); return true; },
    /** The pointer context the cursor draws. */
    cursorContext() { return io.armed() ? { kind: 'identify' } : null; },
  };
}
