// D2Game+0x660D0 -- spawn quest object 0x109 near a point for quest 4 and record it in the quest data.
// stack (game, room, x, y) ret 0x10. No quest node with id 4 in game+0x10f4 -> nothing.
// Diagonal scan i = 0..20 for (x+i, y+i) inside [bx, bx+bw-1) x [by, by+bh-1) of #a6d0(room); the probe point is
// the hit, or (y, y+21) when the scan runs out (retail stores the stale x register over it; kept).
// 0x34130(room, &pt, 2, 0x100, &outRoom, 100) with outRoom preset to room; outRoom == 0 falls back to (room, x, y).
// Block {game, room', 0, 0x109, 1, 0, X, Y, 5, (u16)0} -> 0xEEF10(ecx &block). Up to 20 retries step the point by
// (1,1), take 0x1280(eax room') as the next room (0 -> outRoom = room, pt = (x, y), and 0x34130 still gets the
// null room), and use 10 for the +0x20 field. Last resort 0xEF650(ecx 0x109, edx 1; game, room, x, y, 0xf, 0).
// On a unit: unit+0xc4 |= 0x3000000; quest data +0x51 = 1, +0x52 = 0, +0x68 = unit id.
#include <cstdint>
#include <cstring>
#include <emscripten.h>

struct D2Game660D0Env {
    void     (*a6d0)(uint32_t room, uint32_t box);
    void     (*sub34130)(uint32_t room, uint32_t pt, uint32_t a3, uint32_t a4, uint32_t outRoom, uint32_t a6);
    uint32_t (*subEEF10)(uint32_t block);
    uint32_t (*sub1280)(uint32_t room);
    uint32_t (*subEF650)(uint32_t ecx, uint32_t edx, uint32_t s1, uint32_t s2, uint32_t s3, uint32_t s4, uint32_t s5, uint32_t s6);
};

static inline uint32_t rd32(uint32_t a) { uint32_t v; std::memcpy(&v, (const void*)(uintptr_t)a, 4); return v; }
static inline void wr32(uint32_t a, uint32_t v) { std::memcpy((void*)(uintptr_t)a, &v, 4); }

static uint32_t tryBlock(uint32_t game, uint32_t room, int32_t x, int32_t y, uint32_t kind, const D2Game660D0Env* env)
{
    uint32_t b[10] = {};
    b[0] = game; b[1] = room; b[2] = 0; b[3] = 0x109; b[4] = 1; b[5] = 0;
    b[6] = (uint32_t)x; b[7] = (uint32_t)y; b[8] = kind; b[9] &= 0xffff0000u;
    return env->subEEF10((uint32_t)(uintptr_t)b);
}

// 1.13c D2Game+0x660D0
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_660D0(uint32_t game, uint32_t room, int32_t x, int32_t y, const D2Game660D0Env* env)
{
    const uint32_t list = rd32(game + 0x10f4);
    if (list == 0) return;
    uint32_t n = rd32(list);
    while (n != 0 && rd32(n) != 4) n = rd32(n + 0xf4);
    if (n == 0) return;
    const uint32_t q = rd32(n + 0x18);
    int32_t box[4] = {};
    env->a6d0(room, (uint32_t)(uintptr_t)box);
    int32_t pt[2];
    int32_t cx = x, cy = y;
    bool hit = false;
    for (int i = 0; i <= 20; ++i, ++cx, ++cy) {
        if (cx >= box[0] && cy >= box[1] && cx < box[0] + box[2] - 1 && cy < box[1] + box[3] - 1) { hit = true; break; }
    }
    pt[0] = hit ? cx : y;
    pt[1] = cy;
    uint32_t outRoom = room;
    env->sub34130(room, (uint32_t)(uintptr_t)pt, 2, 0x100, (uint32_t)(uintptr_t)&outRoom, 100);
    uint32_t r; int32_t X, Y;
    if (outRoom == 0) { r = room; X = x; Y = y; } else { r = outRoom; X = pt[0]; Y = pt[1]; }
    uint32_t u = tryBlock(game, r, X, Y, 5, env);
    if (u == 0) {
        for (int k = 0; k < 20 && u == 0; ++k) {
            ++X; ++Y;
            pt[0] = X; pt[1] = Y;
            const uint32_t next = env->sub1280(r);
            outRoom = next;
            if (next == 0) { outRoom = room; pt[0] = x; pt[1] = y; }
            env->sub34130(next, (uint32_t)(uintptr_t)pt, 2, 0x100, (uint32_t)(uintptr_t)&outRoom, 100);
            if (outRoom == 0) { r = room; X = x; Y = y; } else { r = outRoom; X = pt[0]; Y = pt[1]; }
            u = tryBlock(game, r, X, Y, 10, env);
        }
        if (u == 0) u = env->subEF650(0x109, 1, game, room, (uint32_t)x, (uint32_t)y, 0xf, 0);
        if (u == 0) return;
    }
    wr32(u + 0xc4, rd32(u + 0xc4) | 0x3000000);
    *(uint8_t*)(uintptr_t)(q + 0x51) = 1;
    *(uint8_t*)(uintptr_t)(q + 0x52) = 0;
    wr32(q + 0x68, rd32(u + 0xc));
}
