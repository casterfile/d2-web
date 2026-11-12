// D2Game+0x6B070 -- create a marked unit beside a unit, optionally handing it an item.
// stack (game, unit, classId, item) ret 0x10; returns 1 on creation, 0 when no free spot is found.
// #a544(unit, &c); room = #a478(unit); pt = 0x1280(eax room) != 0 ? (c.x+2, c.y+3) : c.
// room2 = #ae98(room, &pt, &c, 1, 0x3e01, 0x801, 1); 0 -> 0. A zeroed 0x84-byte block: +8 game, +0xc
// 0xE5D0(eax unit; 0), +0x14 classId, +0x18 3, +0x1c pt.x, +0x20 pt.y, +0x24 room2, +0x28 (u16)1,
// +0x2a (u16)game[0x78], +0x30 2. n = 0x11490(game, &block, 0); #a472(n, 0x46, 1, 0); #a4de(n); n -> +0xc4 |= 1.
// With an item: L = 0 (it overwrites the saved room); #aa5a(&L, 0x20, 1); #a958(item, #a6c4(item));
// client = player unit ? *(PlayerData+0x9c) : 0; 0x8C810(eax item, edi unit, ebx client; L);
// 0xD2D90(eax unit, esi item; game, 0).
#include <cstdint>
#include <cstring>
#include <emscripten.h>

struct D2Game6B070Env {
    void     (*a544)(uint32_t unit, uint32_t coord);
    uint32_t (*a478)(uint32_t unit);
    uint32_t (*sub1280)(uint32_t room);
    uint32_t (*ae98)(uint32_t room, uint32_t pt, uint32_t c, uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7);
    uint32_t (*subE5D0)(uint32_t eax, uint32_t s1);
    uint32_t (*sub11490)(uint32_t game, uint32_t block, uint32_t zero);
    void     (*a472)(uint32_t unit, uint32_t a2, uint32_t a3, uint32_t a4);
    void     (*a4de)(uint32_t unit);
    void     (*aa5a)(uint32_t l, uint32_t a2, uint32_t a3);
    uint32_t (*a6c4)(uint32_t item);
    void     (*a958)(uint32_t item, uint32_t v);
    uint32_t (*a4c0)(uint32_t unit);
    void     (*sub8C810)(uint32_t eax, uint32_t edi, uint32_t ebx, uint32_t s1);
    void     (*subD2D90)(uint32_t eax, uint32_t esi, uint32_t s1, uint32_t s2);
};

static inline uint32_t rd32(uint32_t a) { uint32_t v; std::memcpy(&v, (const void*)(uintptr_t)a, 4); return v; }
static inline void wr32(uint32_t a, uint32_t v) { std::memcpy((void*)(uintptr_t)a, &v, 4); }

// 1.13c D2Game+0x6B070
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_6B070(uint32_t game, uint32_t unit, uint32_t classId, uint32_t item, const D2Game6B070Env* env)
{
    uint32_t c[2] = {}, pt[2];
    env->a544(unit, (uint32_t)(uintptr_t)c);
    uint32_t L = env->a478(unit);
    if (env->sub1280(L) != 0) { pt[0] = c[0] + 2; pt[1] = c[1] + 3; } else { pt[0] = c[0]; pt[1] = c[1]; }
    const uint32_t room2 = env->ae98(L, (uint32_t)(uintptr_t)pt, (uint32_t)(uintptr_t)c, 1, 0x3e01, 0x801, 1);
    if (room2 == 0) return 0;
    uint32_t b[0x21] = {};
    b[0x14 / 4] = classId;
    b[0x1c / 4] = pt[0];
    b[0] = 0; b[1] = 0; b[2] = game;
    b[0x18 / 4] = 3;
    b[0x20 / 4] = pt[1];
    b[0x24 / 4] = room2;
    b[0x28 / 4] = 1u | ((uint32_t)(uint16_t)rd32(game + 0x78) << 16);
    b[0x30 / 4] = 2;
    b[0xc / 4] = env->subE5D0(unit, 0);
    const uint32_t n = env->sub11490(game, (uint32_t)(uintptr_t)b, 0);
    env->a472(n, 0x46, 1, 0);
    env->a4de(n);
    if (n != 0) wr32(n + 0xc4, rd32(n + 0xc4) | 1);
    if (item == 0) return 1;
    L = 0;
    env->aa5a((uint32_t)(uintptr_t)&L, 0x20, 1);
    env->a958(item, env->a6c4(item));
    const uint32_t client = (unit != 0 && rd32(unit) == 0) ? rd32(env->a4c0(unit) + 0x9c) : 0;
    env->sub8C810(item, unit, client, L);
    env->subD2D90(unit, item, game, 0);
    return 1;
}
