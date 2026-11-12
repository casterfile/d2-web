// src/core/d2game/d2game_batch_e4900.cpp -- twelve 1.13c D2Game.dll quest-callback functions transcribed from the
// cached Ghidra decompiles (build-wasm/ghidra-d2game/decomp/0x<RVA>.c). Base 0x6FC20000.
//
// Struct fields are the retail 32-bit layout (rd32/wr32); pointer arguments are uintptr_t. Unmapped callees and
// D2Common ordinals are seams on D2GameBatchE4900Env. Ghidra's in_EAX / unaff_* registers are explicit parameters.
#include <cstdint>
#include <cstring>
#include <emscripten.h>

static inline uint32_t rd32(uintptr_t p, uint32_t o) { uint32_t v; std::memcpy(&v, (const uint8_t*)p + o, 4); return v; }
static inline void     wr32(uintptr_t p, uint32_t o, uint32_t v) { std::memcpy((uint8_t*)p + o, &v, 4); }
static inline uint8_t  rd8(uintptr_t p, uint32_t o) { return ((const uint8_t*)p)[o]; }
static inline void     wr8(uintptr_t p, uint32_t o, uint8_t v) { ((uint8_t*)p)[o] = v; }

struct D2GameBatchE4900Env {
    uintptr_t (*ord)(uint32_t n, uintptr_t a, uintptr_t b, uintptr_t c);       // D2Common ordinal n, up to 3 args
    void      (*sub8A3E0)(uintptr_t client, const void* buf, uint32_t len);  // 0x8A3E0 send packet (eax = client)
    void      (*subE34D0)(uintptr_t unit);                                    // 0xE34D0
    void      (*sub354F0)(uintptr_t unit, uintptr_t game);                    // 0x354F0
    void      (*sub8EA80)(uintptr_t game, uintptr_t unit, uint32_t one, uint32_t frame, uint32_t a, uint32_t b, uint32_t c); // 0x8EA80
    void      (*sub33AF0)(uintptr_t node, uint32_t cb, uint32_t ebx);         // 0x33AF0: stack (cb, node), EBX
    uintptr_t (*alloc)(const char* file, uint32_t line, uint32_t zero);       // Ordinal_10045
    uint32_t fn54C0, lab5540, lab5450, dat1FE38, lab3030, lab3000;            // retail addresses stored by E6660
    uint32_t cbE57D0;                                                         // retail address of the 0xE57D0 callback (E6760)
};
#define O(n, a, b, c) env->ord(n, (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c))

// Quest-data node of the game's list (game+0x10f4 -> *head, next at node+0xf4) with type `t`; node[6] is its data.
static uintptr_t findQ(uintptr_t game, uint32_t t)
{
    uintptr_t h = rd32(game, 0x10f4);
    if (!h) return 0;
    for (uintptr_t n = rd32(h, 0); n; n = rd32(n, 0xf4)) if (rd32(n, 0) == t) return rd32(n, 0x18);
    return 0;
}
static uintptr_t findQnode(uintptr_t game, uint32_t t)
{
    uintptr_t h = rd32(game, 0x10f4);
    if (!h) return 0;
    for (uintptr_t n = rd32(h, 0); n; n = rd32(n, 0xf4)) if (rd32(n, 0) == t) return n;
    return 0;
}
static uint32_t qstate(uintptr_t game, uintptr_t unit, const D2GameBatchE4900Env* env, uint32_t q, uint32_t s)
{
    uintptr_t pd = O(10920, unit, 0, 0);
    return (uint32_t)O(10174, rd32(pd, 0x10 + rd8(game, 0x6d) * 4), q, s);
}

// 1.13c D2Game+0xE4900
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_E4900(uintptr_t game, const D2GameBatchE4900Env* env)
{
    uintptr_t pd = O(10920, 0, 0, 0);
    uint32_t q = rd32(pd, 0x10 + rd8(game, 0x6d) * 4);
    if (O(10174, q, 0x12, 0) == 0) O(10174, q, 0x12, 1);
    uintptr_t d = findQ(game, 0x10);
    if (!d) return;
    wr32(d, 0x10, rd32(d, 0x10) - 1);
    wr32(d, 0x18, rd32(d, 0x18) - 1);
    wr32(d, 0x14, rd32(d, 0x14) - 1);
    wr32(d, 0x1c, rd32(d, 0x1c) - 1);
    wr32(d, 0x20, rd32(d, 0x20) + 1);
}

// 1.13c D2Game+0xE4A50   eax = in_EAX (record, byte at +0x14), param2 = dl, param3 = stack arg
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_E4A50(uint32_t p1, uint8_t p2, uintptr_t p3, uintptr_t eax, const D2GameBatchE4900Env* env)
{
    (void)p1;
    uintptr_t p = rd32(p3, 0xc);
    uint8_t pkt[6] = { 0x5d, 0x10, rd8(eax, 0x14), p2, 0, 0 };
    uintptr_t client = 0;   // eax into the send: the ordinal's result when it ran
    if (p && rd32(p, 0) == 0) client = rd32(O(10920, p, 0, 0), 0x9c);
    env->sub8A3E0(client, pkt, 6);
}

// 1.13c D2Game+0xE5A60   game = unaff_EDI
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_E5A60(uintptr_t game, const D2GameBatchE4900Env* env)
{
    uintptr_t d = findQ(game, 0x14);
    if (!d) return;
    wr32(d, 0xc, 2);
    wr32(d, 0x10, 2);
    if (rd8(d, 1)) {
        for (uintptr_t u = rd32(game, 0x1520 + (rd32(d, 4) & 0x7f) * 4); u; u = rd32(u, 0xe4))
            if (rd32(u, 0xc) == rd32(d, 4)) { O(11090, u, 1, 0); break; }
    }
    if (rd8(d, 2)) {
        uintptr_t u = rd32(game, 0x1520 + (rd32(d, 8) & 0x7f) * 4);
        for (; u; u = rd32(u, 0xe4))
            if (rd32(u, 0xc) == rd32(d, 8)) { O(11090, u, 2, 0); break; }
    }
}

// 1.13c D2Game+0xE5B60
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_E5B60(uintptr_t game, uintptr_t unit, const D2GameBatchE4900Env* env)
{
    if (qstate(game, unit, env, 0x16, 0) == 1) return;
    uintptr_t it = O(10331, unit, 0, 0);
    if (!it) return;
    uintptr_t rec = O(10826, it, 0, 0);
    if ((uint8_t)O(10459, rec, 0, 0) != 2) return;
    env->subE34D0(unit);
    uintptr_t d = findQ(game, 0x14);
    if (d) wr32(d, 0x1c, rd32(d, 0x1c) + 1);
}

// Shared body of E5BE0 / E5F00 / E6130 / E6550: if the quest has state 0 or 0xf, and neither 0xd nor 0xe, do nothing.
static uint32_t stateGate(uintptr_t game, uintptr_t unit, uint32_t q, const D2GameBatchE4900Env* env)
{
    uintptr_t pd = O(10920, unit, 0, 0);
    uint32_t h = rd32(pd, 0x10 + rd8(game, 0x6d) * 4);
    if (((O(10174, h, q, 0) != 0) || (O(10174, h, q, 0xf) != 0)) && O(10174, h, q, 0xd) == 0 && O(10174, h, q, 0xe) == 0)
        return 0;
    env->sub354F0(unit, game);
    return 0;
}
// 1.13c D2Game+0xE5BE0
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_E5BE0(uintptr_t game, uintptr_t unit, const D2GameBatchE4900Env* env) { return stateGate(game, unit, 0x16, env); }
// 1.13c D2Game+0xE5F00
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_E5F00(uintptr_t game, uintptr_t unit, const D2GameBatchE4900Env* env) { return stateGate(game, unit, 0x14, env); }
// 1.13c D2Game+0xE6130
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_E6130(uintptr_t game, uintptr_t unit, const D2GameBatchE4900Env* env) { return stateGate(game, unit, 0x13, env); }
// 1.13c D2Game+0xE6550
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_E6550(uintptr_t game, uintptr_t unit, const D2GameBatchE4900Env* env) { return stateGate(game, unit, 0x11, env); }

// 1.13c D2Game+0xE5C40
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_E5C40(uintptr_t game, uintptr_t unit, const D2GameBatchE4900Env* env)
{
    uintptr_t pd = O(10920, unit, 0, 0);
    uint32_t h = rd32(pd, 0x10 + rd8(game, 0x6d) * 4);
    uintptr_t it = O(10331, unit, 0, 0);
    if (it && O(10174, h, 0x16, 0) == 0 && O(10826, it, 0, 0) == 0x66) {
        uintptr_t n = findQnode(game, 0x14);
        if (O(10174, h, 0x16, 0xb) == 0) {
            env->subE34D0(unit);
            uintptr_t d = rd32(n, 0x18);   // retail derefs n unchecked
            wr32(d, 0x1c, rd32(d, 0x1c) + 1);
        }
    }
    return 0;
}

// 1.13c D2Game+0xE5F60   game = unaff_EBX
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_E5F60(uintptr_t game, const D2GameBatchE4900Env* env)
{
    uintptr_t d = findQ(game, 0x11);
    if (!d) return;
    wr8(d, 6, 0);
    uintptr_t u = rd32(game, 0x1520 + (rd32(d, 0x28) & 0x7f) * 4);
    for (; u; u = rd32(u, 0xe4)) if (rd32(u, 0xc) == rd32(d, 0x28)) break;
    if (!u) return;
    O(11090, u, 1, 0);
    uintptr_t rec = O(10688, rd32(u, 4), 0, 0);
    env->sub8EA80(game, u, 1, (rd32(rec, 0xdc) >> 8) + 1 + rd32(game, 0xa8), 0, 0, 0);
    wr32(d, 0x2c, 2);
}

// 1.13c D2Game+0xE6660
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_E6660(uintptr_t q, const D2GameBatchE4900Env* env)
{
    std::memset((uint8_t*)q + 0xa0, 0, 15 * 4);
    wr8(q, 0xc, 0);
    wr8(q, 0xb, 0);
    wr32(q, 0xa0, (uint32_t)env->fn54C0);
    wr32(q, 0xcc, (uint32_t)env->lab5540);
    wr32(q, 0xd4, (uint32_t)env->lab5450);
    wr32(q, 0xdc, (uint32_t)env->dat1FE38);
    wr8(q, 10, 1);
    uintptr_t d = env->alloc("..\\Source\\D2Game/Quests/a3q0.cpp", 0x94, 0);
    wr32(q, 0x18, (uint32_t)d);
    std::memset((uint8_t*)d, 0, 7 * 4);
    wr32(q, 0xe0, 0x10);
    wr32(q, 0xe8, (uint32_t)env->lab3030);
    wr32(q, 0xec, (uint32_t)env->lab3000);
}

// 1.13c D2Game+0xE6760   (callback + EBX=2 passed to 0x33AF0 as retail does)  game = in_EAX, param2 = unit (or 0)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_E6760(uint32_t p1, uintptr_t unit, uintptr_t game, const D2GameBatchE4900Env* env)
{
    (void)p1;
    uintptr_t n = findQnode(game, 0x1c);
    if (!n) return;
    uintptr_t d = rd32(n, 0x18);
    if (rd8(d, 0xc) == 0 && rd8(d, 0xd) == 0) {
        wr8(d, 0xd, 1);
        wr32(d, 0x10, unit ? rd32(unit, 0xc) : 0xffffffffu);
        env->sub33AF0(n, env->cbE57D0, 2);
    }
}
