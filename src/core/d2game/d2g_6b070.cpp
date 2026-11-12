// 1.13c D2Game+0x6B070 -- stdcall (game, unit, classId, item), ret 0x10: creates a marked unit beside `unit` and, with an
// item, hands the item over. Returns 1 once the unit request is sent, 0 when 0xAE98 finds no room. Written from the 1.13c
// disassembly (see d2game_6b070.cpp for the older env-pointer form).
// 0xA544(unit, &c); room = 0xA478(unit); 0x1280(EAX room, EBX c.x+2, EDI c.y+3) nonzero takes pt = (c.x+2, c.y+3), else c.
// room2 = 0xAE98(room, &pt, &c, 1, 0x3E01, 0x801, 1). The zeroed 33-dword request {+8 game, +0xC 0xE5D0(EAX unit; 0),
// +0x14 classId, +0x18 3, +0x1C pt, +0x24 room2, word +0x28 1, word +0x2A game+0x78, +0x30 2} goes to 0x11490(game, &req, 0);
// the result n gets 0xA472(n, 0x46, 1, 0), 0xA4DE(n) and +0xC4 |= 1. With an item: L = 0 (it reuses the saved room's slot),
// 0xAA5A(&L, 0x20, 1), 0xA958(item, 0xA6C4(item)), client = player unit ? 0xA4C0(unit)+0x9C : 0,
// 0x8C810(EAX item, EDI unit, EBX client; L), 0xD2D90(EAX unit, ESI item; game, 0).
#include "../fn/hookenv.h"
namespace d2g {
uint32_t f6b070(HookEnv& e, uint32_t game, uint32_t unit, uint32_t cls, uint32_t item)
{
    uint32_t c[2] = { 0, 0 };
    e.h(0xA544, 0, 0, 0, 0, 0, unit, e.stk(c));
    uint32_t room = e.h(0xA478, 0, 0, 0, 0, 0, unit);
    const uint32_t bx = c[0] + 2, di = c[1] + 3;
    const uint32_t hit = e.h(0x1280, room, 0, 0, bx);
    e.edi(0x1280, di);
    uint32_t pt[2];
    if (hit) { pt[0] = bx; pt[1] = di; } else { pt[0] = c[0]; pt[1] = c[1]; }
    const uint32_t room2 = e.h(0xAE98, 0, 0, 0, 0, 0, room, e.stk(pt), e.stk(c), 1, 0x3E01, 0x801, 1);
    if (!room2) return 0;
    uint32_t req[33] = {};
    req[2] = game;
    req[5] = cls;
    req[6] = 3;
    req[7] = pt[0]; req[8] = pt[1];
    req[9] = room2;
    req[10] = 1u | (((uint32_t)e.rd16s(game + 0x78) & 0xFFFF) << 16);
    req[12] = 2;
    req[3] = e.h(0xE5D0, unit, 0, 0, 0, 0, 0);
    const uint32_t n = e.h(0x11490, 0, 0, 0, 0, 0, game, e.stk(req), 0);
    e.blk(0x11490, req, 33);
    e.h(0xA472, 0, 0, 0, 0, 0, n, 0x46, 1, 0);
    e.h(0xA4DE, 0, 0, 0, 0, 0, n);
    if (n) e.wr32(n + 0xC4, e.rd32(n + 0xC4) | 1);
    if (!item) return 1;
    room = 0;   // L: the saved room's slot
    e.h(0xAA5A, 0, 0, 0, 0, 0, e.stk(&room), 0x20, 1);
    const uint32_t v = e.h(0xA6C4, 0, 0, 0, 0, 0, item);
    e.h(0xA958, 0, 0, 0, 0, 0, item, v);
    uint32_t client = 0;
    if (unit && e.rd32(unit) == 0) client = e.rd32(e.h(0xA4C0, 0, 0, 0, 0, 0, unit) + 0x9C);
    e.h(0x8C810, item, 0, 0, client, 0, room);
    e.edi(0x8C810, unit);
    e.h(0xD2D90, unit, 0, 0, 0, item, game, 0);
    return 1;
}
}
