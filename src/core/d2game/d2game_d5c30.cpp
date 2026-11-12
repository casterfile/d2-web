// D2Game+0xD5C30 -- move the cursor item to a body location (equip with swap). EAX = item id, stack a0 game, a1 player, a2 slot (byte), a3 *err (ret 0x10).
// Written from the 1.13c disassembly. game / player == 0 and the A748 / null-occupant asserts exit the process (not modelled). *err = 0 first; failures set *err = 1, return 0.
// Item: game[0x1720 + (id & 0x7f) * 4] chain through [0xE4], type 4 and mode [0x10] == 4; 0xAA78(player, slot, item, 0) == 7; 0xA934(item, player, 0 x4).
// t = slot == 5 ? 4 : 5; occ = 0xA7AE(inv, t); 0xAA24(inv, item, t, s) with s = 0xA634(player, 0, game[0x70]); occ mode must be 1. The occupant is detached and
// its slot rebuilt (0xAA9C gate), then occ2 = 0xA7AE(inv, slot) is detached and the item placed (0xA9A0 / 0xA9B8), or 0xD1650 / 0xDFFD0 when 0xA934 now fails.
#include "d2game_seam4e.h"
static const uint32_t kFile = 0xB20FB14Cu;   // the source-file string at D2Game+0xFB14C, recorded as a module address
// 1.13c D2Game+0xD5C30
extern "C" uint32_t d2game_d5c30(uint32_t id, uint32_t game, uint32_t player, uint32_t slot, uint32_t pOut, const D2Seam* e)
{
    if (!game || !player) return 0;
    d2wr32(pOut, 0);
    const uint32_t sb = slot & 0xFF;
    uint32_t it = d2rd32(game + 0x1720 + (id & 0x7F) * 4);
    while (it && d2rd32(it + 0xC) != id) it = d2rd32(it + 0xE4);
    if (!it || d2rd32(it) != 4 || d2rd32(it + 0x10) != 4) return 0;
    if (e->call(0xAA78, 0, 0, 0, 0, 0, player, sb, it, 0) != 7) return 0;
    if (!e->call(0xA934, 0, 0, 0, 0, 0, it, player, 0, 0, 0, 0)) return 0;
    e->call(0xD29F0, 0, 0, 0, 0, 0, player);
    const uint32_t s = e->call(0xA634, 0, 0, 0, 0, 0, player, 0, d2rd32(game + 0x70));
    const uint32_t inv = d2rd32(player + 0x60);
    const uint32_t t = (sb == 5) ? 4 : 5;
    const uint32_t occ = e->call(0xA7AE, 0, 0, 0, 0, 0, inv, t);
    if (!e->call(0xAA24, 0, 0, 0, 0, 0, inv, it, t, s)) { d2wr32(pOut, 1); return 0; }
    if (!occ) return 0;
    if (d2rd32(occ + 0x10) != 1) { d2wr32(pOut, 1); return 0; }
    e->call(0xA8AA, 0, 0, 0, 0, 0, player, occ);
    e->call(0xA946, 0, 0, 0, 0, 0, inv, occ);
    const uint32_t r = e->call(0xA748, 0, 0, 0, 0, 0, inv, occ);
    if (!r || r != occ) return 0;
    e->call(0xA742, 0, 0, 0, 0, 0, inv, t);
    e->call(0xD46B0, 0, 0, 0, 0, player, game, 0, 1); e->edi(0xD46B0, occ);
    if (e->call(0xAA9C, 0, 0, 0, 0, 0, inv, occ, s, 0, 0, kFile, 0xD87)) {
        if (!e->call(0xA9B8, 0, 0, 0, 0, 0, inv, occ, 1)) { d2wr32(pOut, 1); return 0; }
        e->call(0xA92E, 0, 0, 0, 0, 0, occ, 0);
        d2wr32(occ + 0xC4, d2rd32(occ + 0xC4) & 0xFFFFFFFDu);
        e->call(0xAA54, 0, 0, 0, 0, 0, inv, 0);
        e->call(0xA4A2, 0, 0, 0, 0, 0, occ, 0);
        if (e->call(0xD1BC0, occ, 0, 0, 0, 0)) e->call(0xA65E, 0, 0, 0, 0, 0, occ, 1, 1);
        if (e->call(0xA8A4, 0, 0, 0, 0, 0, occ, 0x4000, 0xD91, kFile)) e->call(0xA65E, 0, 0, 0, 0, 0, occ, 0x4000, 0);
        e->call(0xA916, 0, 0, 0, 0, 0, occ, 0x4000, 1);
        e->call(0xA65E, 0, 0, 0, 0, 0, occ, 1, 1);
        e->call(0xA994, 0, 0, 0, 0, 0, inv, occ);
    }
    const uint32_t occ2 = e->call(0xA7AE, 0, 0, 0, 0, 0, inv, sb);
    if (!occ2) return 0;
    if (d2rd32(occ2 + 0x10) != 1) { d2wr32(pOut, 1); return 0; }
    e->call(0xD46B0, 0, 0, 0, 0, player, game, 0, 1); e->edi(0xD46B0, occ2);
    if (!e->call(0xA934, 0, 0, 0, 0, 0, it, player, 0, 0, 0, 0)) {
        e->call(0xD47C0, 0, 0, 0, 0, 0, 0);
        e->call(0xAA54, 0, 0, 0, 0, 0, inv, it);
        e->call(0xD1650, 0, 0, 0, 0, it); e->edi(0xD1650, player);
        e->call(0xDFFD0, 0, 0, 0, 0, player, 0x14, 0);
        e->call(0xA9A6, 0, 0, 0, 0, 0, player, 1);
        return 0;
    }
    e->call(0xA8AA, 0, 0, 0, 0, 0, player, occ2);
    e->call(0xA946, 0, 0, 0, 0, 0, inv, occ2);
    const uint32_t r2 = e->call(0xA748, 0, 0, 0, 0, 0, inv, occ2);
    if (!r2 || r2 != occ2) return 0;
    e->call(0xA742, 0, 0, 0, 0, 0, inv, sb);
    e->call(0xAA54, 0, 0, 0, 0, 0, inv, occ2);
    d2wr32(occ2 + 0xC4, d2rd32(occ2 + 0xC4) & 0xFFFFFFFDu);
    e->call(0xA4A2, 0, 0, 0, 0, 0, occ2, 4);
    e->call(0xA916, 0, 0, 0, 0, 0, occ2, 0x10, 1);
    e->call(0xA65E, 0, 0, 0, 0, 0, occ2, 1, 1);
    if (e->call(0xA8A4, 0, 0, 0, 0, 0, occ2, 0x4000, 0xDBF, kFile)) e->call(0xA65E, 0, 0, 0, 0, 0, occ2, 0x4000, 0);
    e->call(0xA994, 0, 0, 0, 0, 0, inv, occ2);
    if (!e->call(0xA9A0, 0, 0, 0, 0, 0, inv, it, sb)) { d2wr32(pOut, 1); return 0; }
    if (!e->call(0xA9B8, 0, 0, 0, 0, 0, inv, it, 3)) { d2wr32(pOut, 1); return 0; }
    e->call(0xA86E, 0, 0, 0, 0, 0, it, slot);
    e->call(0xD47C0, 0, 0, 0, 0, 0, 0);
    d2wr32(it + 0xC4, d2rd32(it + 0xC4) & 0xFFFFFFFDu);
    e->call(0xA4A2, 0, 0, 0, 0, 0, it, 1);
    e->call(0xA92E, 0, 0, 0, 0, 0, it, 0xFF);
    e->call(0xA65E, 0, 0, 0, 0, 0, it, 8, 1);
    e->call(0xA916, 0, 0, 0, 0, 0, it, 8, 1);
    e->call(0xA65E, 0, 0, 0, 0, 0, it, 1, 1);
    if (e->call(0xD1BC0, it, 0, 0, 0, 0)) e->call(0xA65E, 0, 0, 0, 0, 0, it, 1, 1);
    if (e->call(0xA8A4, 0, 0, 0, 0, 0, it, 0x4000, 0xDD9, kFile)) e->call(0xA65E, 0, 0, 0, 0, 0, it, 0x4000, 0);
    e->call(0xA994, 0, 0, 0, 0, 0, inv, it);
    e->call(0xD28D0, player, 0, 0, 0, 0);
    e->call(0xD51A0, 0, 0, 0, 0, 0, game, player, 0);
    return 1;
}
