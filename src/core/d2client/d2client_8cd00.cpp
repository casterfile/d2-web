// The stash panel's close button: is a cursor point on it.
// 1.13c D2Client+0x8CD00 (ESI = x, EDI = y, EAX = 1 on the button, else 0). The stash's left-button
// handlers ask it: +0x9B820 (down) arms the press, +0x9B530 (up) closes the stash through +0x8CCB0
// (trade button 0x12) only when the press was armed and the release is on the button too.
// Reads the panel origin (D2Client+0x11B9A0 x, +0x11B9A4 y), the screen height (+0xDBC4C) and the
// expansion flag (+0x119854). The button sits 3 px further left and 1 px lower in a classic game.
// Both edges of each axis are strict (signed compares), so the inclusive rect is
// x in [xb + 1, xb + 0x27], y in [yb - 0x22, yb + 4]; the y base is +0x8BEB0's.
#include <cstdint>
#if __has_include(<emscripten.h>)
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

extern "C" {
// 1.13c D2Client+0x8CD00
EMSCRIPTEN_KEEPALIVE int32_t d2_client_stash_close_hit(int32_t x, int32_t y, int32_t originX,
                                                       int32_t originY, int32_t screenH,
                                                       int32_t expansion) {
    // 32-bit wrapping sums as the x86 adds make them, compared signed.
    auto add = [](int32_t a, int32_t b) { return (int32_t)((uint32_t)a + (uint32_t)b); };
    const int32_t xb = add(originX, expansion ? 0x110 : 0x113);
    if (x <= xb || x >= add(xb, 0x28)) return 0;
    const int32_t yb = add(add(originY, screenH), expansion ? -0x40 : -0x41);   // +0x8BEB0
    if (y <= add(yb, -0x23) || y >= add(yb, 5)) return 0;
    return 1;
}
}
