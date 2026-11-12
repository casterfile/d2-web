// src/core/d2game/d2game_batch_5e840.cpp -- twelve 1.13c D2Game.dll quest-callback functions transcribed from
// the cached Ghidra decompiles (build-wasm/ghidra-d2game/decomp/0x<RVA>.c). Base 0x6FC20000.
// Addresses are 32-bit retail values held as uint32_t. Register-only inputs (in_EAX, unaff_EBX/EDI) are explicit
// parameters. Callees not implemented in src/ are seams on D2GameBatch5E840Env, in retail argument order.
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <emscripten.h>

static inline uint8_t* P(uint32_t a) { return (uint8_t*)(uintptr_t)a; }
static inline uint32_t rd32(uint32_t a, uint32_t o) { uint32_t v; std::memcpy(&v, P(a) + o, 4); return v; }
static inline void wr32(uint32_t a, uint32_t o, uint32_t v) { std::memcpy(P(a) + o, &v, 4); }
static inline uint16_t rd16(uint32_t a, uint32_t o) { uint16_t v; std::memcpy(&v, P(a) + o, 2); return v; }
static inline void wr16(uint32_t a, uint32_t o, uint16_t v) { std::memcpy(P(a) + o, &v, 2); }
static inline uint8_t rd8(uint32_t a, uint32_t o) { return P(a)[o]; }
static inline void wr8(uint32_t a, uint32_t o, uint8_t v) { P(a)[o] = v; }

struct D2GameBatch5E840Env {
    uint32_t (*o10331)(uint32_t);                          // D2Common #10331
    int32_t  (*o10826)(uint32_t);                          // #10826
    uint32_t (*o10920)(uint32_t);                          // #10920
    int32_t  (*o10174)(uint32_t, uint32_t, uint32_t);      // #10174
    void     (*o10003)(uint32_t, uint32_t, uint32_t);      // #10003
    void     (*o10003r)();                                 // #10003, register args (0x62640)
    void     (*o11090)(uint32_t unit, uint32_t mode);      // #11090 (the call thunk a4a2)
    void*    (*o10045)(const char*, int, int);             // #10045 alloc
    void     (*o10750)(uint32_t);
    void     (*o10867)(uint32_t);
    void     (*o10346)(uint32_t, uint32_t);
    void     (*o10265_10024)(uint32_t line);               // fatal path: #10265(line), #10024(msg,..), _exit(-1)
    void     (*sub33830)(uint32_t);                        // 0x33830
    void     (*sub540c0)();                                // 0x340C0
    void     (*sub33eb0)(uint32_t game, uint32_t arg, uint32_t id);
    uint32_t (*sub33800)(uint32_t ecx, uint32_t p);
    void     (*sub33880)();
    void     (*subAEA80)(uint32_t game, uint32_t edi, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t); // 0x8EA80
    uint32_t (*sub21780)(uint32_t seed);                   // 0x1780
    void     (*sub32b90)(uint32_t game, uint32_t unit, uint32_t v);
    void     (*sub4de30)();
    void     (*subDFCC0)(uint32_t a, uint32_t b);
    uint32_t (*subC0000)(uint32_t);                        // 0xC0000
    int32_t  (*sub1280)();                                 // 0x1280
    uint32_t (*subE1D90)(uint32_t, uint32_t, uint8_t, uint32_t, uint32_t);
    void     (*sub1180)(uint32_t seed);                    // 0x1180
    uint32_t cb[14];                                       // 0x5FC20 callbacks: da70 8040 f3f0 a0a0 f5e0 7e80 7dd0 bcc0 b8c0 7f50 6b10
};

// Quest-data node walk: game+0x10f4 -> first node; node+0xF4 next; node[0] is the quest id.
static uint32_t findQ(uint32_t game, uint32_t id) {
    uint32_t h = rd32(game, 0x10f4);
    if (!h) return 0;
    for (uint32_t n = rd32(h, 0); n; n = rd32(n, 0xf4)) if (rd32(n, 0) == id) return n;
    return 0;
}

// 1.13c D2Game+0x5E840 (EBX = quest, EDI = unit)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_5E840(uint32_t q, uint32_t unit, const D2GameBatch5E840Env* e)
{
    int32_t t = unit ? (int32_t)rd32(unit, 0x10) : 0;
    uint32_t mode = 0;
    if (t == 1) mode = 2;
    else if (t == 3) mode = 4;
    else if (t == 2) {
        uint32_t d = rd32(q, 0x18);
        uint8_t c;
        if (e->o10826(e->o10331(unit)) == 0x6d) {
            int32_t n = (int32_t)rd32(d, 0xdc) + 1;
            wr32(d, 0xdc, n);
            if (n > 5) wr8(d, 0xae, 1);
            c = rd8(d, 0xae);
        } else c = rd8(d, 0xad);
        if (c) mode = 3;
    }
    if (mode) e->o11090(unit, mode);
    uint32_t g = rd32(q, 4);
    e->subAEA80(g, unit, 7, rd32(g, 0xa8) + 0x19, 0, 0, 0);
}

// 1.13c D2Game+0x5FC20
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_5FC20(uint32_t q, const D2GameBatch5E840Env* e)
{
    std::memset(P(q) + 0xa0, 0, 0x3c);
    wr32(q, 0xa0, e->cb[0]); wr32(q, 0xc8, e->cb[1]); wr32(q, 0xac, e->cb[2]); wr32(q, 0xa8, e->cb[3]);
    wr32(q, 0xcc, e->cb[4]); wr32(q, 0xd4, e->cb[5]); wr32(q, 0xd8, e->cb[6]); wr32(q, 0xdc, e->cb[7]);
    wr8(q, 10, 1); wr8(q, 0xc, 0);
    uint32_t d = (uint32_t)(uintptr_t)e->o10045("..\\Source\\D2Game/Quests/a5q3.cpp", 0x5ca, 0);
    wr32(q, 0x18, d); wr32(q, 0xe0, 0x25); wr32(q, 0xe8, e->cb[8]); wr8(q, 0xd, 5);
    wr32(q, 0xec, e->cb[9]); wr32(q, 0xf0, e->cb[10]); wr32(q, 0x10, 0x22);
    std::memset(P(d), 0, 0x114);
}

// 1.13c D2Game+0x5FE30 (ECX = quest, EDX = event)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_5FE30(uint32_t q, uint32_t ev, const D2GameBatch5E840Env* e)
{
    uint32_t d = rd32(q, 0x18);
    uint32_t h = rd32(rd32(q, 4), 0x10f4);
    wr8(h, 0x20, 0x13);
    e->subDFCC0(h, 0);
    if (rd8(q, 9)) {
        bool flag = false;
        uint32_t u = e->o10331(rd32(ev, 8));
        wr32(d, 0x8c, u);
        if (!u) return;
        wr8(q, 0xb, 4);
        e->subDFCC0(d, 0);
        if (rd32(ev, 12)) {
            if (rd8(q, 9)) {
                uint32_t pl = e->o10920(rd32(ev, 12));
                if (e->o10174(rd32(pl, 0x10 + rd8(rd32(ev, 0), 0x6d) * 4), 0x28, 0) == 0) flag = true;
            }
            wr32(d, 0x88, 0);
            e->subDFCC0(0, rd32(ev, 8));
            e->subDFCC0(rd32(ev, 8), rd32(ev, 8));
            e->subDFCC0(0, rd32(ev, 8));
            e->subDFCC0(0, rd32(ev, 8));
            if (flag) {
                for (int32_t i = 0; i < (int32_t)rd32(d, 0x88); i++) {
                    uint32_t g = rd32(rd32(q, 4), 0x10f4);
                    if (!g) { e->o10265_10024(0x1fb); std::exit(-1); }
                    e->sub32b90(rd32(q, 4), rd32(ev, 8), e->sub21780(g + 0x18));
                }
            }
            e->sub4de30();
        }
        if (rd8(q, 0xc) != 5) wr8(q, 0xc, 5);
    }
    uint32_t u = rd32(ev, 8);
    int32_t t = (int32_t)rd32(u, 0);
    if (t != 2 && (t < 4 || t > 5) && rd32(u, 0x2c)) e->o10750(rd32(u, 0x2c));
    u = rd32(ev, 8); t = (int32_t)rd32(u, 0);
    if (t != 2 && (t < 4 || t > 5) && rd32(u, 0x2c)) e->o10867(rd32(u, 0x2c));
    uint32_t p = e->subC0000(rd32(ev, 0));
    if (p) e->o10346(e->o10331(p), 0);
}

// (1.13c D2Game 0x60A30: the q32 port in d2game_60a30.cpp is the proved one)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_60A30(uint32_t q, const D2GameBatch5E840Env* e)
{
    e->sub540c0();
    uint32_t d = rd32(q, 0x18);
    e->sub33830(d); e->sub33830(d + 0x84); e->sub33830(d + 0x108);
}

// (1.13c D2Game 0x60A80: the q32 port in d2game_60a80.cpp is the proved one)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_60A80(uint32_t q, uint32_t* ev, const D2GameBatch5E840Env* e)
{
    // ev = int[6]: 0 game, 2 msg unit, 3 player, 5 arg
    uint32_t pl = e->o10920(ev[3]);
    uint32_t sk = rd32(pl, 0x10 + rd8(ev[0], 0x6d) * 4);
    e->o10174(sk, 6, 0); e->o10174(sk, 6, 0);
    int32_t hard = e->o10174(sk, 6, 1);
    int32_t id = ev[2] ? (int32_t)rd32(ev[2], 4) : -1;
    uint32_t d = rd32(q, 0x18);
    int32_t pid = ev[3] ? (int32_t)rd32(ev[3], 0xc) : -1;
    auto scan = [&](uint32_t base, uint32_t cnt) {
        for (uint32_t i = 0; i < cnt; i++)
            if ((int32_t)rd32(d, base + i * 4) == pid) { e->sub33eb0(q, ev[5], id); return true; }
        return false;
    };
    if (id == 0x94) { if (scan(0x84, rd16(d, 0x104))) return; }
    else if (id == 0x96) { if (scan(0x108, rd16(d, 0x188))) return; }
    else if (id == 0x109) { if (scan(0, rd16(d, 0x80))) return; }
    if (hard) { e->sub33eb0(q, ev[5], id); return; }
    for (uint32_t i = 0; i < rd16(q, 0x9c); i++)
        if ((int32_t)rd32(q, 0x1c + i * 4) == pid) { e->sub33eb0(q, ev[5], id); return; }
    uint8_t s = rd8(q, 0xc);
    // 0x6FD2A024 table lives in the DLL data; read from its mapped copy
    if (s) {
        uint32_t v = rd32(0x6FD2A024u, s * 4);
        if ((s < 4 || sk) && v != 0xffffffffu && v < 6) e->sub33eb0(q, ev[5], id);
    }
}

// (1.13c D2Game 0x60F70: the q32 port in d2game_60f70.cpp is the proved one) (ECX = game, EDX = player)
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_60F70(uint32_t game, uint32_t pl, const D2GameBatch5E840Env* e)
{
    uint32_t sk = rd32(e->o10920(pl), 0x10 + rd8(game, 0x6d) * 4);
    if (e->o10174(sk, 5, 0) == 0 && e->o10174(sk, 5, 1) == 0) {
        uint32_t u = e->o10331(pl);
        if (u && e->o10826(u) == 0x19) {
            uint32_t n = findQ(game, 5);
            if (!n) return 0;
            if (e->sub33800(0, rd32(n, 0x18) + 0x8c) == 0) e->sub33880();
            e->o10003(sk, 5, 0); e->o10003(sk, 5, 0xd);
        }
    }
    return 0;
}

// (1.13c D2Game 0x61340: the q32 port in d2game_61340.cpp is the proved one)
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2game_legacy_61340(uint32_t game, uint32_t unit, const D2GameBatch5E840Env* e)
{
    uint32_t n = findQ(game, 4);
    if (!n) return 1;
    uint32_t d = rd32(n, 0x18);
    if (rd8(d, 0x95) == 0) {
        e->o10331(unit);
        if (e->sub1280() == 0) {
            wr32(d, 0x9c, rd32(d, 0x9c) + 3); wr32(d, 0xa0, rd32(d, 0xa0) + 3);
            return 1;
        }
        uint32_t m = e->subE1D90(2, 0xbd, (uint8_t)rd32(d, 0x9c), rd32(d, 0xa0), game);
        if (m) { wr8(d, 0x95, 1); wr32(d, 0x98, rd32(m, 0xc)); }
    }
    return 1;
}

// (1.13c D2Game 0x61420: the q32 port in d2game_61420.cpp is the proved one) (EAX = game, ECX unused, EDX = flag)
extern "C" EMSCRIPTEN_KEEPALIVE void d2game_legacy_61420(uint32_t game, uint32_t, uint32_t, const D2GameBatch5E840Env* e)
{
    uint32_t n = findQ(game, 4);
    e->sub33830(rd32(n, 0x18) + 0xb4);   // both flag branches are identical
}

// (1.13c D2Game 0x61880: the q32 port in d2game_61880.cpp is the proved one)
extern "C" EMSCRIPTEN_KEEPALIVE void d2game_legacy_61880(uint32_t q, uint32_t ev, const D2GameBatch5E840Env* e)
{
    e->sub540c0();
    uint32_t d = rd32(q, 0x18);
    e->sub33830(d + 0xb4);
    e->sub33830(d + 0x138);              // both rd32(ev,0xc) branches are identical
    (void)ev;
}

// (1.13c D2Game 0x61910: the q32 port in d2game_61910.cpp is the proved one) (ECX = game, EDX = player)
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2game_legacy_61910(uint32_t game, uint32_t pl, const D2GameBatch5E840Env* e)
{
    uint32_t sk = rd32(e->o10920(pl), 0x10 + rd8(game, 0x6d) * 4);
    if (e->o10174(sk, 4, 0) == 0 && e->o10174(sk, 4, 1) == 0) {
        e->o10003(sk, 4, 0xe);
        findQ(game, 4);
        e->sub33880();
    }
    return 0;
}

// (1.13c D2Game 0x624D0: the q32 port in d2game_624d0.cpp is the proved one)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_624D0(uint32_t q, uint32_t* ev, const D2GameBatch5E840Env* e)
{
    if (!ev[2] || rd32(ev[2], 4) != 0x10a) return;
    uint32_t n = findQ(rd32(q, 4), 1);
    uint32_t h = rd32(ev[0], 0x10f4);
    if (!rd32(h, 8)) { e->o10265_10024(0x82c); std::exit(-1); }
    bool reroll = true;
    if (e->o10174(rd32(h, 0xc), 1, 0xd) == 0 && n && rd8(n, 9)) {
        uint32_t sk = rd32(e->o10920(ev[3]), 0x10 + rd8(ev[0], 0x6d) * 4);
        if (e->o10174(sk, 1, 0) == 0 && e->o10174(sk, 1, 0xd) == 0 && e->o10174(sk, 1, 1) == 0) {
            e->sub1180(ev[3] + 0x20);
            reroll = false;
        }
    }
    if (reroll) {
        uint32_t s = ev[3];
        uint64_t v = (uint64_t)rd32(s, 0x20) * 0x6ac690c5u + rd32(s, 0x24);
        std::memcpy(P(s) + 0x20, &v, 8);
    }
    e->sub33eb0(q, ev[5], 0x10a);
}

// (1.13c D2Game 0x62640: the q32 port in d2game_62640.cpp is the proved one) (ECX = player unit, EAX = game)
extern "C" EMSCRIPTEN_KEEPALIVE void d2game_legacy_62640(uint32_t pl, uint32_t game, const D2GameBatch5E840Env* e)
{
    e->o10003r(); e->o10003r();
    uint32_t r = (!pl || rd32(pl, 0) != 0) ? 0 : rd32(e->o10920(pl), 0x9c);
    uint32_t w = rd16(r, 10);
    uint32_t u = ((w & 0x20) | 0x80) >> 5;
    if (((w & 0x1f00) >> 8) <= u * rd8(game, 0x6d) + 1)
        wr16(r, 10, (uint16_t)(((u * rd8(game, 0x6d) + 1) * 0x100) | (w & 0xe0ff)));
}
