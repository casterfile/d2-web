// src/core/d2game/d2game_c8cd0.cpp -- 1.13c D2Game.dll 0xC8CD0 (quest 11 helper: spawns the class-0x13E object two tiles
// left / one tile up of the player and records its id in the quest data). Transcribed from the disassembly
// EAX = player unit, stack quest, ret 4. Callees are seams in retail's call order; the
// unit and quest-data fields are written as retail's 32-bit words.
#include <cstdint>
#include <cstring>
#include <emscripten.h>

struct D2GameC8CD0Env {
    void     (*coords)(uint32_t unit, uint32_t* xy);                 // 0xA544: stack (unit, &coords), coords = {x, y}
    uint32_t (*room)(uint32_t unit);                                 // 0xA478: the unit's room
    uint32_t (*allocUnit)(uint32_t pool, uint32_t type);             // 0xA6B8: stack (game+0x1C, 2)
    uint32_t (*a556)(uint32_t room);                                 // 0xA556
    uint32_t (*a56e)(uint32_t v);                                    // 0xA56E (low byte used: act)
    void     (*initSeed)(uint32_t esi, uint32_t edi);                // 0xE0110 SUNIT_InitSeed: ESI = game+0xD0, EDI = unit
    uint32_t (*alloc)(uint32_t ecx, uint32_t edx, uint32_t file, uint32_t line, uint32_t z);   // 0xA2E6 (Fog alloc; file 0x6FD1B02C)
    uint32_t (*a520)(uint32_t cls);                                  // 0xA520
    void     (*a724)(uint32_t unit);                                 // 0xA724
    void     (*sub2F300)(uint32_t esi, uint32_t id, uint32_t room, uint32_t x, uint32_t y);    // 0x2F300: ESI = unit, ret 0x10
    void     (*unitAdd)(uint32_t eax, uint32_t ecx, uint32_t edx, uint32_t x, uint32_t game, uint32_t one);  // 0xE0F40 SUNIT_Add: EAX room, ECX y, EDX unit
};

static inline uint32_t c8_rd32(uint32_t a) { uint32_t v; std::memcpy(&v, (const void*)(uintptr_t)a, 4); return v; }
static inline void c8_wr32(uint32_t a, uint32_t v) { std::memcpy((void*)(uintptr_t)a, &v, 4); }

// 1.13c D2Game+0xC8CD0
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_C8CD0(uint32_t unit, uint32_t quest, const D2GameC8CD0Env* e)
{
    const uint32_t qd = c8_rd32(quest + 0x18);
    uint32_t xy[2];                                               // uninitialised local; 0xA544 fills it
    e->coords(unit, xy);
    if (*(const uint8_t*)(uintptr_t)(qd + 0xe) || *(const uint8_t*)(uintptr_t)(qd + 0x11)) return;
    const uint16_t z16 = 0; std::memcpy((void*)(uintptr_t)(qd + 0x40), &z16, 2);
    const uint32_t x = xy[0] - 2;
    c8_wr32(qd + 0x20, x);
    c8_wr32(qd + 0x24, xy[1] - 1);
    uint32_t room = e->room(unit);
    uint32_t game = c8_rd32(quest + 4);
    uint32_t y = c8_rd32(qd + 0x24);
    uint32_t u = e->allocUnit(c8_rd32(game + 0x1c), 2);
    if (!u) {                                                     // retry one tile lower
        c8_wr32(qd + 0x24, xy[1]);
        room = e->room(unit);
        game = c8_rd32(quest + 4);
        y = c8_rd32(qd + 0x24);
        u = e->allocUnit(c8_rd32(game + 0x1c), 2);
        if (!u) return;
    }
    c8_wr32(u + 0x0, 2);
    c8_wr32(u + 0x4, 0x13e);
    c8_wr32(u + 0x8, c8_rd32(game + 0x1c));
    c8_wr32(u + 0x80, game);
    uint32_t f = c8_rd32(u + 0xc8);
    f = c8_rd32(game + 0x70) ? (f | 0x2000000) : (f & ~0x2000000u);
    c8_wr32(u + 0xc8, f);
    c8_wr32(u + 0xc8, f | 0x4000000);
    const uint8_t act = (uint8_t)e->a56e(e->a556(room));
    *(uint8_t*)(uintptr_t)(u + 0x18) = act;
    c8_wr32(u + 0x1c, c8_rd32(game + 0xbc + act * 4));
    c8_wr32(u + 0x74, 0);
    e->initSeed(game + 0xd0, u);
    c8_wr32(u + 0xc4, c8_rd32(u + 0xc4) | 0x10);
    c8_wr32(u + 0xd0, 0xb);
    uint32_t id = c8_rd32(game + 0x98) + 1;
    if (id == 0xffffffffu) id = 1;
    c8_wr32(game + 0x98, id);
    const uint32_t p = e->alloc(c8_rd32(game + 0x1c), 0x38, 0x6fd1b02cu, 0x265, 0);
    c8_wr32(u + 0x14, p);
    std::memset((void*)(uintptr_t)p, 0, 0x38);
    const uint32_t v = e->a520(0x13e);
    c8_wr32(c8_rd32(u + 0x14), v);
    c8_wr32(u + 0x10, 0);
    e->a724(u);
    e->sub2F300(u, id, room, x, y);
    e->unitAdd(room, y, u, x, game, 1);
    c8_wr32(u + 0xc4, c8_rd32(u + 0xc4) | 0x3000000);
    *(uint8_t*)(uintptr_t)(qd + 0x11) = 1;
    c8_wr32(qd + 0x38, c8_rd32(u + 0xc));
}
