// src/core/d2client/d2client_batch_87de0.cpp -- 12 UNMAPPED 1.13c D2Client.dll rows (0x87DE0..0x888B0, the
// sound-entry list at DAT_6fbcbf30) reimplemented from the cached decompiles. Alpha batch lane (OBJECTIVE v8.3):
// implementation + marker only, no spec, no vector. Env-seam style of d2client_batch_5d110.cpp.
// List node = {entry*, next*}; entry fields at retail byte offsets (packed, unaligned). Record table stride 0x8E.
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <emscripten.h>

#include "d2snd87_seam.h"

template <class T> static T rd(const uint8_t* p, size_t o) { T v; memcpy(&v, p + o, sizeof v); return v; }
template <class T> static void wr(uint8_t* p, size_t o, T v) { memcpy(p + o, &v, sizeof v); }
static uint8_t* rec(const D2cSnd87* s, int32_t i) {
    return (!s->recs || i < 0 || i >= s->count) ? nullptr : s->recs + (size_t)i * 0x8E;
}
static uint8_t* ent(void** n) { return (uint8_t*)n[0]; }
static uint8_t* find(const D2cSnd87* s, int32_t key) {
    if (!key) return nullptr;
    for (void** n = s->list; n; n = (void**)n[1]) if (rd<int32_t>(ent(n), 8) == key) return ent(n);
    return nullptr;
}

// 1.13c D2Client+0x87DE0 (key in EDX)
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_client_87de0(D2cSnd87* s, const D2cSnd87Env* e, int32_t key) {
    uint8_t* p = find(s, key);
    if (!p) return 0;
    if (rd<uint32_t>(p, 0x2c)) return e->ord10032();
    e->fatal(0x45a);
    return 0;
}

// 1.13c D2Client+0x87E90
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_client_87e90(D2cSnd87* s, const D2cSnd87Env*, int32_t key) {
    uint8_t* p = find(s, key);
    if (!p) return 0;
    uintptr_t a = rd<uint32_t>(p, 0x28);   // retail pointers are 32-bit
    return a && rd<int32_t>((const uint8_t*)a, 4) ? 1 : 0;
}

// 1.13c D2Client+0x87FE0 (idx in EAX, skip entry param_1)
extern "C" EMSCRIPTEN_KEEPALIVE uint8_t* d2_client_87fe0(D2cSnd87* s, const D2cSnd87Env*, int32_t idx, uint8_t* skip) {
    uint8_t* r = rec(s, idx);
    for (void** n = s->list; n; n = (void**)n[1]) {
        uint8_t* p = ent(n);
        uint8_t* q = rec(s, rd<int32_t>(p, 4));
        int32_t grp = s->recs ? rd<int32_t>(q, 0x60) : 0;   // retail reads through null on bad idx
        if (grp == rd<int32_t>(r, 0x60) && p != skip && rd<int32_t>(p, 0x3d) == 0) {
            int16_t w = rd<int16_t>(r, 0x45);
            if (w < 0 || (uint32_t)(s->now - rd<int32_t>(p, 0x34)) <= (uint32_t)(int32_t)w) return p;
        }
    }
    return nullptr;
}

// 1.13c D2Client+0x881B0 (stdcall unit, ret 4). Ordinal_10628 (thunk 0xC21A,
// stdcall unit, POINT*) fills the player's position ([0x6FBCBBFC]) then the argument unit's; dx = B.x - A.x, dy = B.y - A.y
// (int32 wrap); retail sums dx*dx + (2*dy)*(2*dy) on the x87 and returns 1 when it is <= 490000.0f ([0x6FB87570]). Past
// |dx| > 700 or |dy| > 350 one square alone exceeds the bound, so the exact integer compare below is the same verdict.
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_client_881b0(uint32_t player, uint32_t unit, void (*pos)(uint32_t unit, int32_t* pt)) {
    int32_t a[2], b[2];
    pos(player, a);
    pos(unit, b);
    const int64_t dx = (int32_t)((uint32_t)b[0] - (uint32_t)a[0]), dy = (int32_t)((uint32_t)b[1] - (uint32_t)a[1]);
    if (dx > 700 || dx < -700 || dy > 350 || dy < -350) return 0;
    return dx * dx + 4 * dy * dy <= 490000 ? 1 : 0;
}

// 1.13c D2Client+0x88230 (this ignored, eax = base duration in EAX on entry)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_88230(D2cSnd87* s, const D2cSnd87Env* e, int32_t key, int32_t mode,
                                                     int32_t delay, uint32_t eax) {
    if (!key) return;
    uint8_t* p = find(s, key);
    if (!p) return;
    if (d2_client_87e90(s, e, key)) return;
    uint8_t* r = rec(s, rd<int32_t>(p, 4));
    uint32_t dur = eax;
    if (mode == 0xff || mode == 0) {
        uint8_t b = rd<uint8_t>(r, mode == 0 ? 0x40 : 0x3f);
        if (dur < b) dur = b;
    }
    bool tail = false;   // reach the "if (dur)" block
    if (rd<int32_t>(p, 0x45) == 0) {
        if (mode == 0) wr<int32_t>(p, 0x3d, 1);
        tail = true;
    } else {
        if (mode == rd<int32_t>(p, 0x4d) && rd<uint32_t>(p, 0x55) <= (uint32_t)(delay + s->now) + dur) return;
        if (rd<int32_t>(p, 0x4d) != 0) { if (mode == 0) wr<int32_t>(p, 0x3d, 1); tail = true; }
        else { if (mode != 0) return; wr<int32_t>(p, 0x3d, 1); tail = true; }
    }
    (void)tail;
    if (dur) {
        int32_t t = delay + s->now;
        wr<int32_t>(p, 0x51, t);
        wr<uint32_t>(p, 0x49, rd<uint32_t>(p, 0x1c));
        wr<int32_t>(p, 0x4d, mode);
        wr<uint32_t>(p, 0x55, (uint32_t)t + dur);
        wr<int32_t>(p, 0x45, 1);
        return;
    }
    if (delay) { e->fatal(0x25c); return; }
    e->fn87ed0(s->now, (uint32_t)key, mode);
}

static void snd_kick(D2cSnd87* s, const D2cSnd87Env* e, uint8_t* p) {   // 1.13c D2Client+0x88550 body
    uint8_t* r = rec(s, rd<int32_t>(p, 4));
    wr<int32_t>(p, 0x3d, 1);
    int32_t k = rd<int32_t>(p, 8);
    if (rd<uint8_t>(r, 0x40) != 0 && rd<int32_t>(p, 0x30) == 1) d2_client_88230(s, e, k, 0, 0, rd<uint8_t>(r, 0x40));   // retail: EAX = rec[0x40] (the base duration), stack key, 0, 0
}

// 1.13c D2Client+0x88550 (entry in ECX)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_88550(D2cSnd87* s, const D2cSnd87Env* e, uint8_t* p) { snd_kick(s, e, p); }

// shared walk: kick every entry whose record index satisfies pred
// via550: the caller reaches the kick through a call to 0x88550 (0x88770, 0x88810, 0x886F0); 0x888B0 has it inlined
template <bool via550 = true, class F> static void walk(D2cSnd87* s, const D2cSnd87Env* e, F pred) {
    for (void** n = s->list; n; n = (void**)n[1]) {
        if (!pred(ent(n))) continue;
        if (via550 && e && e->fn88550) e->fn88550(ent(n)); else snd_kick(s, e, ent(n));
    }
}

// 1.13c D2Client+0x886F0 (idx 0xb76..0x1230)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_886f0(D2cSnd87* s, const D2cSnd87Env* e) {
    walk(s, e, [](uint8_t* p) { int32_t i = rd<int32_t>(p, 4); return i > 0xb75 && i < 0x1231; });
}

// 1.13c D2Client+0x88770 (idx 0x48..0xc9, group != a and != b)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_88770(D2cSnd87* s, const D2cSnd87Env* e, int32_t a, int32_t b) {
    walk(s, e, [&](uint8_t* p) {
        int32_t i = rd<int32_t>(p, 4); uint8_t* r = rec(s, i);
        int32_t g = s->recs ? rd<int32_t>(r, 0x60) : 0;
        return g != a && g != b && i > 0x47 && i < 0xca;
    });
}

// 1.13c D2Client+0x88810 (idx 0x34..0x47, group != a and != b)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_88810(D2cSnd87* s, const D2cSnd87Env* e, int32_t a, int32_t b) {
    walk(s, e, [&](uint8_t* p) {
        int32_t i = rd<int32_t>(p, 4); uint8_t* r = rec(s, i);
        int32_t g = s->recs ? rd<int32_t>(r, 0x60) : 0;
        return g != a && g != b && i > 0x33 && i < 0x48;
    });
}

// 1.13c D2Client+0x888B0 (idx lo..hi)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_888b0(D2cSnd87* s, const D2cSnd87Env* e) {
    walk<false>(s, e, [&](uint8_t* p) { int32_t i = rd<int32_t>(p, 4); return s->lo <= i && i <= s->hi; });
}
