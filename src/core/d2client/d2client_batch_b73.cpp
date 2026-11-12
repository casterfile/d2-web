// src/core/d2client/d2client_batch_b73.cpp -- four 1.13c D2Client.dll rows (alpha lane), from
// build-wasm/ghidra-d2client/decomp/0x<RVA>.c. Base 0x6FAB0000. Globals and register args are
// parameters; unrecovered callees are host callbacks.
#include <cstdint>
#include <initializer_list>
#include <emscripten.h>

// 1.13c D2Client+0x87F30 (EDI = float out[3], stdcall unit, type, ret 8).
// Ordinal_10628 (thunk 0xC21A, stdcall unit, POINT*) gives A = the player's position ([0x6FBCBBFC]) and B = the unit's.
// type 0xA27: B is replaced by a projection computed on the x87 (53-bit control word, so C doubles are the same):
// t = ((-2*Ax - 0.5*Bx) + Ay - By) * -0.4f, B.x = ftol(t), B.y = ftol((-2*Ax - (-2*t)) + Ay); _ftol2 (0xCACC4) truncates
// to an int64 in EDX:EAX and the caller keeps EAX (the inputs are int32, so |v| < 2^35 and the int64 always holds). out = {(float)(B.x - Ax), (float)(2*(B.y - Ay)), 640.0f}, the differences int32-wrapped.
static int32_t ftol87(double v) { return (int32_t)(uint32_t)(uint64_t)(int64_t)v; }
extern "C" EMSCRIPTEN_KEEPALIVE
void d2_client_87f30(float* out, uint32_t player, uint32_t unit, int32_t type, void (*pos)(uint32_t unit, int32_t* pt)) {
    int32_t a[2], b[2];
    pos(player, a);
    pos(unit, b);
    int32_t bx = b[0], by = b[1];
    if (type == 0xa27) {
        const double c64 = (double)-0.4f, c68 = 0.5, c6c = -2.0, ax2 = (double)a[0] * c6c;
        const double t = (((ax2 - (double)b[0] * c68) + (double)a[1]) - (double)b[1]) * c64;
        bx = ftol87(t);
        by = ftol87((ax2 - t * c6c) + (double)a[1]);
    }
    out[2] = 640.0f;
    out[0] = (float)(int32_t)((uint32_t)bx - (uint32_t)a[0]);
    out[1] = (float)(2.0 * (double)(int32_t)((uint32_t)by - (uint32_t)a[1]));
}

// 1.13c D2Client+0x88440 (132 B): picks between two float constants ([0x6FB87568] / [0x6FB87578]).
// ecx_idx = EAX index; tab_24c/cnt_250 = [0x11C24C]/[0x11C250] (0x8E-byte rows); tab_254/cnt_258 =
// [0x11C254]/[0x11C258] (0x58-byte rows); callbacks: ord10839(local, p, 2), sub_5EB90() -> index.
// Row bytes are read through `rd`. Returns 0 -> const_578, 1 -> const_568.
extern "C" EMSCRIPTEN_KEEPALIVE
uint32_t d2_client_88440(int32_t idx, uint32_t tab_24c, int32_t cnt_250, uint32_t tab_254, int32_t cnt_258,
                         uint32_t local_bbfc, uint32_t param_1,
                         uint32_t (*rd)(uint32_t addr, uint32_t size),
                         uint32_t (*ord10839)(uint32_t, uint32_t, uint32_t),
                         uint32_t (*sub_5EB90)(void)) {
    uint32_t row = 0;
    if (tab_24c && idx >= 0 && idx < cnt_250)
        row = (uint32_t)idx * 0x8E + tab_24c;
    bool use568;
    if (row == 0) {
        use568 = ord10839(local_bbfc, param_1, 2) != 0;
    } else {
        if (rd(row + 0x47, 4) == 4) return 0;
        if (rd(row + 0x60, 4) != 0xCA) {
            use568 = ord10839(local_bbfc, param_1, 2) != 0;
        } else {
            int32_t i = (int32_t)sub_5EB90();   // the full EAX of 0x5EB90 (signed compare, as the decompile)
            uint32_t r2 = (tab_254 && i >= 0 && i < cnt_258) ? (uint32_t)i * 0x58 + tab_254 : 0;
            if (r2 == 0) return 0;
            use568 = (rd(r2 + 0x18, 1) & 0xFF) != 0;
        }
    }
    return use568 ? 1u : 0u;
}

// 1.13c D2Client+0x8A4D0 (105 B): set rect1 = {l,t,r,b}; rect2 = {-0x50,-0x50,w+0x50,h-0x2f};
// widened (left-0x2d, top-0x1e, right+0x1e) when `wide`; flags |= 1. s = uint32_t[9]: flags, rect1[4], rect2[4].
extern "C" EMSCRIPTEN_KEEPALIVE
void d2_client_8a4d0(uint32_t* s, int32_t l, int32_t t, int32_t r, int32_t b,
                     int32_t scr_w, int32_t scr_h, uint32_t wide) {
    if (!s) return;
    int32_t* r1 = (int32_t*)(s + 1);
    int32_t* r2 = (int32_t*)(s + 5);
    r1[0] = l; r1[1] = t; r1[2] = r; r1[3] = b;
    r2[0] = -0x50; r2[1] = -0x50; r2[2] = scr_w + 0x50; r2[3] = scr_h - 0x2f;
    if (wide) { r2[0] -= 0x2d; r2[1] -= 0x1e; r2[2] += 0x1e; }
    s[0] |= 1;
}

