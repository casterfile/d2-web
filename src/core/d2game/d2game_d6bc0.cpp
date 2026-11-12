// D2Game+0xD6BC0: destroy an inventory item and notify. EAX = unit, EDI = item, stack a0 (ret 4) goes to 0xD6710. Written from the 1.13c disassembly.
// inv = [unit + 0x60]. 0xA8A4(item, 0x10, 0x1381, file) != 0 or inv == 0: return. 0xAA0C(item, 0x100000) != 0 asserts (exit, not modelled).
// Then 0xD6710 (EAX = unit, ECX = item, stack a0), 0xA65E(item, 1, 1), 0xA8A4(item, 0x4000, 0x138A, file) gating 0xA65E(item, 0x4000, 0),
// 0xA994(inv, item), 0xA9A6(unit, 1), 0xDFFD0(6, unit).
#include "d2game_seam4e.h"
static const uint32_t kFile = 0xB20FB14Cu;   // the source-file string at D2Game+0xFB14C, recorded as a module address
// 1.13c D2Game+0xD6BC0
extern "C" void d2game_d6bc0(uint32_t unit, uint32_t item, uint32_t arg, const D2Seam* e)
{
    const uint32_t inv = d2rd32(unit + 0x60);
    if (e->call(0xA8A4, 0, 0, 0, 0, 0, item, 0x10, 0x1381, kFile) || !inv) return;
    if (e->call(0xAA0C, 0, 0, 0, 0, 0, item, 0x100000)) return;
    e->call(0xD6710, unit, item, 0, 0, 0, arg);
    e->call(0xA65E, 0, 0, 0, 0, 0, item, 1, 1);
    if (e->call(0xA8A4, 0, 0, 0, 0, 0, item, 0x4000, 0x138A, kFile)) e->call(0xA65E, 0, 0, 0, 0, 0, item, 0x4000, 0);
    e->call(0xA994, 0, 0, 0, 0, 0, inv, item);
    e->call(0xA9A6, 0, 0, 0, 0, 0, unit, 1);
    e->call(0xDFFD0, 0, 0, 0, 0, 0, 6, unit);
}
