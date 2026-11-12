// 1.13c D2Game+0x537D0: ECX = pBuf, EDX = pUnit, stack = six words of which word 2 = nSize (ret 0x18).
// Writes the 'JM' item section. The rule over an environment E (HookEnv in the FN probe src/core/fn/d2game_537d0.cpp); every callee is one
// E::h call in retail's order (disassembled; the decompile loses the EBX/EBP item locals).
// - nSize < 2: 0. 'JM' at buf; count = 0x53260 (EAX = unit); nSize < 4: 2. Count word at buf +2; null unit: 4.
// - prim = 0xA54A (unit), right = 0xA7AE (unit, 4), left = 0xA7AE (unit, 5), cursor = 0xA6A6 (unit).
// - pre-pass, each through #10987 thunk 0xAFD0 (item, &scratch 0x2000, 0x2000, 1, 1, 0), a negative result gives -1:
//   prim set and right == prim: right, then (left set) left and right again; prim set otherwise: left (unchecked), then right if set;
//   prim null: right if set, then left if set.
// - node loop from 0xA3DC (unit) over 0xA4CC (node): item = 0xA46C (node); a node with 0xA736 (node) == 3 and body loc 0xA97C (item) 4 or 5 is
//   skipped; else written.
// - "written" = r = 0x53470 (EAX = item, EDX = nSize - (pos - buf); pos); negative gives -1; pos += r; r == 0 with the count word nonzero
//   decrements it.
// - hand items: prim set and right == prim: right, then left if set; prim set otherwise: left (unchecked), then right if set; prim null: right
//   if set, then left if set. Then the cursor when set and its type dword is 4. Count word back at buf +2; pos - buf.
#pragma once
#include <cstdint>

namespace g537d0 {
template <class E> uint32_t WriteItems(E& e, uint32_t buf, uint32_t unit, int32_t size)
{
    if (size < 2) return 0;
    e.wr8(buf, 0x4A); e.wr8(buf + 1, 0x4D);
    uint16_t count = (uint16_t)e.h(0x53260, unit);
    uint32_t pos = buf + 2;
    if (size < 4) return 2;
    e.wr8(pos, count & 0xFF); e.wr8(pos + 1, count >> 8);
    pos += 2;
    if (!unit) return 4;
    const uint32_t prim = e.h(0xA54A, 0, 0, 0, 0, 0, unit);
    const uint32_t right = e.h(0xA7AE, 0, 0, 0, 0, 0, unit, 4);
    const uint32_t left = e.h(0xA7AE, 0, 0, 0, 0, 0, unit, 5);
    const uint32_t cursor = e.h(0xA6A6, 0, 0, 0, 0, 0, unit);
    uint8_t scratch[16];
    const uint32_t sp = e.stk(scratch);
    auto pre = [&](uint32_t it) {
        const uint32_t w[6] = { it, sp, 0x2000, 1, 1, 0 };
        return (int32_t)e.h(0xAFD0, 0, 0, 0, 0, 0, w[0], w[1], w[2], w[3], w[4], w[5]) >= 0;
    };
    if (prim) {
        if (right == prim) {
            if (!pre(right)) return 0xFFFFFFFFu;
            if (left) { if (!pre(left)) return 0xFFFFFFFFu; }
        } else {
            if (!pre(left)) return 0xFFFFFFFFu;
            if (right && !pre(right)) return 0xFFFFFFFFu;
        }
    } else {
        if (right && !pre(right)) return 0xFFFFFFFFu;
        if (left && !pre(left)) return 0xFFFFFFFFu;
    }
    bool fail = false;
    auto put = [&](uint32_t it) {
        const int32_t r = (int32_t)e.h(0x53470, it, 0, (uint32_t)size - (pos - buf), 0, 0, pos);
        if (r < 0) { fail = true; return; }
        pos += (uint32_t)r;
        if (r == 0 && count) --count;
    };
    for (uint32_t node = e.h(0xA3DC, 0, 0, 0, 0, 0, unit); node; node = e.h(0xA4CC, 0, 0, 0, 0, 0, node)) {
        const uint32_t item = e.h(0xA46C, 0, 0, 0, 0, 0, node);
        const uint8_t loc = (uint8_t)e.h(0xA97C, 0, 0, 0, 0, 0, item);
        if (e.h(0xA736, 0, 0, 0, 0, 0, node) == 3 && (loc == 4 || loc == 5)) continue;
        put(item);
        if (fail) return 0xFFFFFFFFu;
    }
    if (prim) {
        if (right == prim) {
            put(right); if (fail) return 0xFFFFFFFFu;
            if (left) { put(left); if (fail) return 0xFFFFFFFFu; }
        } else {
            put(left); if (fail) return 0xFFFFFFFFu;
            if (right) { put(right); if (fail) return 0xFFFFFFFFu; }
        }
    } else {
        if (right) { put(right); if (fail) return 0xFFFFFFFFu; }
        if (left) { put(left); if (fail) return 0xFFFFFFFFu; }
    }
    if (cursor && e.rd32(cursor) == 4) { put(cursor); if (fail) return 0xFFFFFFFFu; }
    e.wr8(buf + 2, count & 0xFF); e.wr8(buf + 3, count >> 8);
    return pos - buf;
}
}
