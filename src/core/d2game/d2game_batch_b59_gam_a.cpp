// src/core/d2game/d2game_batch_b59_gam_a.cpp -- 1.13c D2Game.dll quest/AI helpers transcribed from the cached
// Ghidra decompiles (build-wasm/ghidra-d2game/decomp/0x<RVA>.c). Base 0x6FC20000.
// D2Common ordinals go through env->ord(n, a..e); unmapped D2Game callees are seams. Ghidra's in_*/unaff_*
// registers are explicit parameters named at each function. Fog assert ordinals (#10003) are omitted.
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <emscripten.h>

static inline uint32_t rd32(const void* p, uint32_t o) { uint32_t v; std::memcpy(&v, (const uint8_t*)p + o, 4); return v; }
static inline void     wr32(void* p, uint32_t o, uint32_t v) { std::memcpy((uint8_t*)p + o, &v, 4); }
static inline uint16_t rd16(const void* p, uint32_t o) { uint16_t v; std::memcpy(&v, (const uint8_t*)p + o, 2); return v; }
static inline void     wr16(void* p, uint32_t o, uint16_t v) { std::memcpy((uint8_t*)p + o, &v, 2); }
static inline void* P(uint32_t v) { return (void*)(uintptr_t)v; }

#include "d2game_batch_b59_env.h"
#define O(n, a, b, c, d, e) env->ord(n, (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c), (uintptr_t)(d), (uintptr_t)(e))

// quest-record lookup shared by several rows: walk game->+0x10F4 list for node type 0x17, return node+0x18.
static uint32_t questRec(const void* game) {
    uint32_t l = rd32(game, 0x10F4);
    if (!l) return 0;
    uint32_t p = rd32(P(l), 0);
    while (p) {
        if (rd32(P(p), 0) == 0x17) return rd32(P(p), 0x18);
        p = rd32(P(p), 0xF4);
    }
    return 0;
}

extern "C" {

// 1.13c D2Game+0xB1660
EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_B1660(void* unused, void* p2, void* unit, uint32_t* p4, const D2GameBatchB59Env* env) {
    if (!unit) return 0;
    if (!((int32_t)rd32(unit, 0) < 0 || rd32(unit, 0) > 1 || env->subDFA90(unit) != 0 ||
          !((rd32(unit, 0xC4) >> 2) & 1) || env->subE0A60(p2, unused) == 0)) {
        int32_t v = env->subB0A20(p2);
        if (v <= (int32_t)p4[3]) {
            uint32_t b = env->subB08A0(unit);   // the full EAX: the callee returns a byte and the decompile reads CONCAT31(stale, al)
            if (b < 2) { if ((int32_t)p4[6] <= v) return 0; }
            else if ((int32_t)p4[1] <= v) return 0;
            if (p4[2] == 0 || O(10839, unit, p2, 4, 0, 0) == 0) {
                if (b > 1) { p4[0] = (uint32_t)(uintptr_t)unit; p4[1] = (uint32_t)v; return 0; }
                p4[5] = (uint32_t)(uintptr_t)unit; p4[6] = (uint32_t)v;
            }
        }
        return 0;
    }
    if (rd32(unit, 0) == 1 && (p2 == nullptr || rd32(p2, 0) != 0) && O(10830, p2, 0, 0, 0, 0) == 0 &&
        p4[2] != 0 && O(10648, unit, 0, 0, 0, 0) == 0) {
        uint32_t rec = env->sub218E0();
        if (rd32(P(rec), 8) & 8) {
            uint32_t a = env->sub21130(unit), b = env->sub21100(unit);
            uint32_t u = O(10331, unit, b, a, 0, 0);
            if (p4[4] == O(10916, u, 0, 0, 0, 0)) {
                wr16(P(rec), 8, (uint16_t)(rd16(P(rec), 8) | 8));
                p4[2] = 0;
            }
        }
    }
    return 0;
}

// 1.13c D2Game+0xB9A30  (game = unaff_EBX)
EMSCRIPTEN_KEEPALIVE void d2_port_d2game_b9a30(uint8_t* unit, uint8_t* game, uint32_t qh, const D2GameBatchB59Env* env) {
    // qh = EAX at entry: the quest handle the callers (b9f20) hold in EBP.
    O(10003, qh, 0x1A, 0xD, 0, 0);
    O(10003, qh, 0x1A, 0, 0, 0);
    O(10495, qh, 0x1A, 0, 0, 0);
    if (rd32(game, 0x70) == 0) {
        O(10003, qh, 0x1A, 6, 0, 0);
        O(10003, qh, 0x1A, 7, 0, 0);
        uint32_t pd = 0;
        if (unit && rd32(unit, 0) == 0) pd = O(10920, unit, 0, 0, 0, 0);
        uint32_t it = pd ? rd32(P(pd), 0x9C) : 0;
        uint32_t u = (rd16(P(it), 10) & 0x20 | 0x80) >> 5;
        if (((rd16(P(it), 10) & 0x1F00) >> 8) <= u * game[0x6D] + 4)
            wr16(P(it), 10, (uint16_t)(((uint16_t)game[0x6D] * (uint16_t)u + 4) * 0x100 | (rd16(P(it), 10) & 0xE0FF)));
    }
    uint32_t q = questRec(game);
    if (q) wr32(P(q), 0x1C, rd32(P(q), 0x1C) + 1);
}

// 1.13c D2Game+0xB9F20
EMSCRIPTEN_KEEPALIVE uint32_t d2_port_d2game_b9f20(uint8_t* game, void* unit, const D2GameBatchB59Env* env,
                                                 void (*b9a30)(void*, uint32_t)) {
    uint32_t pd = O(10920, unit, 0, 0, 0, 0);
    uint32_t u = rd32(P(pd), 0x10 + game[0x6D] * 4);
    uint32_t t = O(10174, u, 0x1A, 0, 0, 0);
    if (t == 1) return 0;
    uint32_t w = O(10331, unit, 0, 0, 0, 0);
    if (!w) return 0;
    uint32_t q = questRec(game);
    if (!q) return 0;
    bool hit = (w == rd32(P(q), 0x3C));
    if (!hit) {
        uint32_t* arr = nullptr; uint32_t n = 0;
        O(10383, w, &arr, &n, 0, 0);
        for (uint32_t i = 0; i < n && !hit; i++) hit = arr[i] == rd32(P(q), 0x3C);
    }
    if (O(10174, u, 0x1A, 0, 0, 0) == 0 && O(10174, u, 0x1A, 1, 0, 0) == 0 && hit) b9a30(unit, u);
    return 0;
}

// 1.13c D2Game+0xBC050  (self = in_EAX, ctx = param_1)
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_BC050(void* self, uint8_t* ctx, const D2GameBatchB59Env* env) {
    O(10331, rd32(ctx, 4), 0, 0, 0, 0);
    uint32_t r = env->sub21280();
    if (r) env->sub54130(r, self, P(3), 0x3F11);
}

// 1.13c D2Game+0xBC4A0 / 0xBC510 / 0xBC580: nudge two quest-record ints by (dx, dy), then 0xBC050.
static uint32_t nudge(uint8_t* ctx, uint32_t off, int32_t dx, int32_t dy, const D2GameBatchB59Env* env) {
    uint32_t u = rd32(ctx, 4);
    if ((u == 0 || rd32(P(u), 0x10) == 0)) {
        uint32_t q = questRec(P(rd32(ctx, 0)));
        if (q) {
            O(10141, u, (uintptr_t)((uint8_t*)P(q) + off), 0, 0, 0);
            wr32(P(q), off, rd32(P(q), off) + dx);
            wr32(P(q), off + 4, rd32(P(q), off + 4) + dy);
            d2_fn_d2game_BC050(nullptr, ctx, env);
        }
    }
    return 0;
}
EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_BC4A0(uint8_t* ctx, const D2GameBatchB59Env* env) { return nudge(ctx, 0x34, 0x20, 0x10, env); }
EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_BC510(uint8_t* ctx, const D2GameBatchB59Env* env) { return nudge(ctx, 0x2C, -0x27, 0x21, env); }
EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_BC580(uint8_t* ctx, const D2GameBatchB59Env* env) { return nudge(ctx, 0x24, -0xC, -0x34, env); }

}
