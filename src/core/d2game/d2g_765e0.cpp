// 1.13c D2Game+0x765E0 -- ECX = game, EDX = unit; returns the last callee's EAX. `dt` is *sgptDataTables (D2Game's IAT
// slot 0xF829C), the probe's pinned blob. Written from the 1.13c disassembly.
// 0xB018(unit+0x2C); 0x1560(EAX class, EDX 3) zero: event 2 at frame+1 (0x8EA80(ECX game, EDX unit; 2, frame+1, 0, 0, 0)).
// A monster (type 1) whose class (or its MonStats row +2 base id, rows dt+0xA78 x 0x1A8, count dt+0xA80) is 0x4E: event 2
// at frame + (0xA68E(unit, 0x15) ? 0x2D : 0xF). Otherwise 0xA496(unit, 0), unit+0xB0 = 0xA0 and the 8-dword request
// {3, unit, 0, 0, 0, byte +0x15 = 0x64, 0, 0} to 0x75E10(game, &req, 1).
#include "../fn/hookenv.h"
namespace d2g {
uint32_t f765e0(HookEnv& e, uint32_t game, uint32_t unit, uint32_t dt)
{
    e.h(0xB018, 0, 0, 0, 0, 0, e.rd32(unit + 0x2C));
    int32_t cls = (int32_t)e.rd32(unit + 4);
    if (!e.h(0x1560, (uint32_t)cls, 0, 3))
        return e.h(0x8EA80, 0, game, unit, 0, 0, 2, e.rd32(game + 0xA8) + 1, 0, 0, 0);
    if (e.rd32(unit) == 1 && cls >= 0) {
        if (cls < (int32_t)e.rd32(dt + 0xA80)) {
            const uint32_t row = (uint32_t)cls * 0x1A8 + e.rd32(dt + 0xA78);
            if (row) cls = e.rd16s(row + 2);
        }
        if (cls >= 0 && cls < (int32_t)e.rd32(dt + 0xA80) && cls == 0x4E) {
            const uint32_t v = e.h(0xA68E, 0, 0, 0, 0, 0, unit, 0x15);
            return e.h(0x8EA80, 0, game, unit, 0, 0, 2, e.rd32(game + 0xA8) + (v ? 0x2D : 0xF), 0, 0, 0);
        }
    }
    e.h(0xA496, 0, 0, 0, 0, 0, unit, 0);
    const uint32_t req[8] = { 3, unit, 0, 0, 0, 0x6400, 0, 0 };
    e.wr32(unit + 0xB0, 0xA0);
    const uint32_t r = e.h(0x75E10, 0, 0, 0, 0, 0, game, e.stk(req), 1);
    e.blk(0x75E10, req, 8);
    return r;
}
}
