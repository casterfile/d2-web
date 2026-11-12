// D2Client batch b67: 80A70 80BE0 80CA0 80E30 (weather/effect RNG helpers). Globals are passed in by pointer/value (each names its RVA).
// Skipped (SEH/_exit/CRT sprintf/GetTickCount/Win32): 7C270 7C690 7D200 7D3C0 7DE50 7E6E0 7EA90 7EFA0 7F780.
#include <cstdint>
#include <cstring>
#include <initializer_list>
typedef int32_t i32; typedef uint32_t u32;
extern "C" {
__attribute__((weak)) u32 Ordinal_10265(u32);
__attribute__((weak)) void Ordinal_10024(const void*, u32);
}
namespace {
// seed = {lo,hi}: lo' = lo*0x6ac690c5 + hi; writes 64-bit result back, returns it
inline uint64_t step(uint8_t* s) {
    u32 lo, hi; memcpy(&lo, s, 4); memcpy(&hi, s + 4, 4);
    uint64_t v = (uint64_t)lo * 0x6ac690c5u + hi; memcpy(s, &v, 8); return v;
}
inline u32 rd(const uint8_t* p, u32 o) { u32 v; memcpy(&v, p + o, 4); return v; }
inline void wr(uint8_t* p, u32 o, u32 v) { memcpy(p + o, &v, 4); }
}
// 1.13c D2Client+0x80A70 (player = DAT_6fbcbbfc, obj = DAT_6fbcbf60; null-player _exit path: returns after the error ordinals).
// Ordinal_10064 (ECX = obj, EBX = seed), Ordinal_10013 and the 0x18D0 rand (ECX = seed, ESI = range, EDI = base) come in as callbacks.
extern "C" void d2c_80a70(uint8_t* player, uint8_t* obj, i32 p1, i32 p2, i32 p3, i32 p4,
                          u32 (*o10064)(uint8_t* obj, uint8_t* seed), u32 (*o10013)(), u32 (*r18d0)(uint8_t* seed, i32 range, i32 base)) {
    if (!player) { Ordinal_10024((const void*)0, Ordinal_10265(0x2d2)); return; }
    uint8_t* seed = player + 0x20;
    u32 slot = o10064(obj, seed);
    if (slot == 0xffffffffu) return;
    uint8_t* e = (uint8_t*)(uintptr_t)rd(obj, 0x11c) + (int16_t)(slot >> 16) * 0x18;
    if (o10013() == 0) {
        u32 r = (u32)step(seed) % 0x50;
        i32 d = (i32)r - 0x28; if (d < 0) d = -d;
        i32 span = 0x50 - 2 * d;
        wr(e, 4, r18d0(seed, 2 * span, -span) + p1);
        wr(e, 8, r + p2);
    } else {
        wr(e, 4, (u32)step(seed) % 5 * 0x10000 + p3);
        wr(e, 8, (u32)step(seed) % 5 * 0x10000 + p4);
    }
    u32 k = ((u32)step(seed) & 1) + 2;
    wr(e, 0xc, k); wr(e, 0x10, 0); wr(e, 0x14, 2);
}
// 1.13c D2Client+0x80BE0 (player = DAT_6fbcbbfc; out: 1C3938, 1C38BC, 1C3918, 1C394C)
extern "C" void d2c_80be0(uint8_t* player, i32* o3938, i32* o38bc, i32* o3918, i32* o394c) {
    if (!player) { *o3938 = 0x138; *o38bc = 0x4e2; *o3918 = 0x7f; *o394c = 0x7f; return; }
    uint64_t v = step(player + 0x20);
    *o3938 = (i32)((u32)v % 0x177) + 0x7d;
    v = ((v & 0xffffffffu) * 0x6ac690c5u) + (v >> 32); memcpy(player + 0x20, &v, 8);
    *o38bc = (i32)((u32)v % 0x5dc) + 500;
    v = ((v & 0xffffffffu) * 0x6ac690c5u) + (v >> 32); memcpy(player + 0x20, &v, 8);
    *o3918 = (i32)((u32)v % 0x47) + 0x5c; *o394c = *o3918;
}
// 1.13c D2Client+0x80CA0 (player = DAT_6fbcbbfc, flag = DAT_6fbcbf78; out: 1C392C 1C3930 1C38EC 1C38F0 1C3914 1C3934 1C3918 1C394C 1CBF6C)
extern "C" void d2c_80ca0(uint8_t* player, i32 flag, i32* o392c, i32* o3930, i32* o38ec, i32* o38f0, i32* o3914, i32* o3934, i32* o3918, i32* o394c, i32* o_bf6c,
                          u32 (*o10331)(u32), u32 (*o10826)(u32)) {
    u32 cls = o10826(player ? o10331((u32)(uintptr_t)player) : 0);
    switch (cls) {
    case 0x6d: case 0x6e: case 0x6f: case 0x70:
        *o3930 = *o392c = 0x2a; *o38f0 = *o38ec = 0xaa;
        *o3914 = (i32)((u32)step(player + 0x20) % 0x38) + 0x20; *o3934 = *o3914; return;
    case 0x75:
        *o38ec = *o38f0 = 0x38; *o392c = 0xaa; *o3930 = 0x1c;
        *o3914 = (i32)((u32)step(player + 0x20) % 0x70) + 0x20; *o3934 = *o3914; return;
    case 0x78: case 0x79:
        *o392c = *o38ec = *o3930 = *o38f0 = *o394c = *o3918 = 0x1c; *o3914 = *o3934 = 0x100; return;
    }
    *o3934 = *o3914;
    if (flag == 0) step(player + 0x20);
    *o_bf6c = 0;
}
// 1.13c D2Client+0x80E30 (seed = register EBX = player+0x20, obj = DAT_6fbcbf68, flag = DAT_6fbcbf78)
extern "C" void d2c_80e30(uint8_t* seed, const uint8_t* obj, i32 flag, i32* g3934, i32* g394c, i32* g3918, i32* g3938, i32* g38bc, i32* g393c, i32 bf6c, void (*sub_80700)(uint8_t*), i32 g38ec, i32 g392c) {
    while ((i32)rd(obj, 0x114) < *g3934) sub_80700(seed);
    i32 d = (flag == 0) + 1;
    if (*g394c < *g3918) { *g3918 -= d; if (*g394c > *g3918) *g3918 = *g394c; }
    else if (*g394c > *g3918) { *g3918 += d; if (*g3918 > *g394c) *g3918 = *g394c; }
    if (--*g3938 == 0) {
        if (flag == 0) {
            uint64_t v = step(seed);
            *g3938 = (i32)((u32)v % 0x177) + 0x7d;
            v = ((v & 0xffffffffu) * 0x6ac690c5u) + (v >> 32); memcpy(seed, &v, 8);
            *g394c = (i32)((u32)v % 0x47) + 0x5c;
        } else {
            uint64_t v = step(seed);
            *g3938 = (i32)((u32)v % 0x36b) + 0xfa;
            // 1.13c D2Client+0x18D0 (ECX = seed, ESI = range = DAT 0x1138EC, EDI = base = DAT 0x11392C):
            // range <= 0 gives the base; else step the seed, power-of-two range masks lo, any other takes lo % range; + base
            if (g38ec <= 0) *g394c = g392c;
            else {
                u32 lo = (u32)step(seed);
                *g394c = (i32)(((g38ec & (g38ec - 1)) == 0 ? (lo & (u32)(g38ec - 1)) : lo % (u32)g38ec) + (u32)g392c);
            }
        }
    }
    if (bf6c != 0 && --*g38bc == 0) *g393c = 1;
}
