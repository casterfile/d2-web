// 1.13c D2Game+0x660D0 -- stdcall (game, room, x, y), ret 0x10: spawns the type-4 quest's object 0x109 near (x, y) and returns
// its id (0 when there is no such quest or nothing spawned). Written from the 1.13c disassembly.
// 0xA6D0(room, &rc) gives the room rect; (x+i, y+i), i = 0..20, is taken when strictly inside rc shrunk by one on the far
// edges (retail quirk: a miss leaves pt = (y, y + 21)). 0x34130(room, &pt, 2, 0x100, &roomOut, 0x64) fixes the point; a null
// roomOut keeps (room, x, y). The request {game, room, 0, 0x109, 1, 0, X, Y, 5, word 0} goes to 0xEEF10 (ECX = &req). Then up
// to 20 retries step (X, Y) by one, ask 0x1280(EAX room, EBX X, EDI Y) for a room (none: room, x, y), refix with 0x34130 and resend with
// +0x20 = 0xA. Last resort 0xEF650(ECX 0x109, EDX 1; game, room, x, y, 0xF, 0). A unit gets +0xC4 |= 0x3000000 and the
// quest data +0x51 = 1, +0x52 = 0, +0x68 = unit id.
#include "../fn/hookenv.h"
namespace d2g {
uint32_t f660d0(HookEnv& e, uint32_t game, uint32_t room, uint32_t x, uint32_t y)
{
    uint32_t q = e.rd32(game + 0x10F4);
    if (!q) return 0;
    q = e.rd32(q);
    while (q && e.rd32(q) != 4) q = e.rd32(q + 0xF4);
    if (!q) return 0;
    const uint32_t d = e.rd32(q + 0x18);
    uint32_t rl = room;
    uint32_t rc[4] = { 0, 0, 0, 0 };
    e.h(0xA6D0, 0, 0, 0, 0, 0, room, e.stk(rc));
    int32_t X = (int32_t)x, Y = (int32_t)y;
    uint32_t pt[2];
    bool ok = false;
    for (int i = 0; i <= 0x14; i++, X++, Y++) {
        if (X >= (int32_t)rc[0] && Y >= (int32_t)rc[1] && X < (int32_t)(rc[0] + rc[2] - 1) && Y < (int32_t)(rc[1] + rc[3] - 1)) { ok = true; break; }
    }
    pt[0] = ok ? (uint32_t)X : y; pt[1] = (uint32_t)Y;
    e.h(0x34130, 0, 0, 0, 0, 0, room, e.stk(pt), 2, 0x100, e.stk(&rl), 0x64);
    uint32_t r, bx, di;
    if (!rl) { bx = x; r = room; di = y; } else { r = rl; di = pt[1]; bx = pt[0]; }
    uint32_t S[10];
    auto send = [&](uint32_t kind) {
        S[0] = game; S[1] = r; S[2] = 0; S[3] = 0x109; S[4] = 1; S[5] = 0; S[6] = bx; S[7] = di; S[8] = kind; S[9] = 0;
        const uint32_t u = e.h(0xEEF10, 0, e.stk(S));
        e.blk(0xEEF10, S, 10);
        return u;
    };
    uint32_t unit = send(5);
    if (!unit) {
        for (uint32_t i = 0; i < 0x14 && !unit;) {
            bx++; di++;
            pt[0] = bx; pt[1] = di;
            const uint32_t r2 = e.h(0x1280, r, 0, 0, bx);
            e.edi(0x1280, di);
            rl = r2;
            uint32_t a = r2;
            if (!r2) { rl = room; pt[0] = x; pt[1] = y; a = room; }
            e.h(0x34130, 0, 0, 0, 0, 0, a, e.stk(pt), 2, 0x100, e.stk(&rl), 0x64);
            if (!rl) { r = room; bx = x; di = y; } else { r = rl; di = pt[1]; bx = pt[0]; }
            i++;
            unit = send(0xA);
        }
        if (!unit) {
            unit = e.h(0xEF650, 0, 0x109, 1, 0, 0, game, room, x, y, 0xF, 0);
            if (!unit) return 0;
        }
    }
    e.wr32(unit + 0xC4, e.rd32(unit + 0xC4) | 0x3000000);
    e.wr8(d + 0x51, 1);
    e.wr8(d + 0x52, 0);
    const uint32_t id = e.rd32(unit + 0xC);
    e.wr32(d + 0x68, id);
    return id;
}
}
