// src/core/d2game/d2game_aea40.cpp -- 1.13c D2Game.dll ItemPropertyTableApplyB (sub_AEA40), transcribed from the cached decompile
// and resolved against its disassembly (the decompile mixes up which locals feed 10680/10945; the assembly is followed here).
// Convention as d2game_batch_aea40.cpp: raw retail records, register inputs explicit, callees weak externs.
#include <cstdint>

typedef intptr_t I;
typedef int32_t S;
typedef uint32_t U;
#define O(n) extern "C" __attribute__((weak)) I Ord##n(...)
#define F(n) extern "C" __attribute__((weak)) I D2GAME_##n(...)
#define R32(b, o) (*(uint32_t*)((uintptr_t)(b) + (o)))
#define S32(b, o) (*(int32_t*)((uintptr_t)(b) + (o)))
#define R8(b, o)  (*(uint8_t*)((uintptr_t)(b) + (o)))
#define S16(b, o) (*(int16_t*)((uintptr_t)(b) + (o)))
#define U16(b, o) (*(uint16_t*)((uintptr_t)(b) + (o)))

extern "C" __attribute__((weak)) char sgptDataTables_exref[];
static inline uintptr_t DT() { return (uintptr_t)R32(sgptDataTables_exref, 0); }
extern "C" __attribute__((weak)) void D2GAME_9F7A0();   // the stat-list callback handed to 10911
O(10695); O(10428); O(10871); O(10959); O(10973); O(10551); O(11013); O(10785); O(10945); O(10911);
O(10807); O(11043); O(10680); O(10188); O(10913); O(10887);
F(8EA80); F(8E6F0); F(7A260); F(4EA30);

// 1.13c D2Game+0xAEA40 -- ECX = game, EDX = unit, stack = (rec, four more dwords). ret 0x14.
// Applies up to three stat/value pairs of an item property row to the unit; returns 1 when any was applied.
extern "C" S d2_fn_d2game_AEA40(I game, I unit, I rec, I, I, I, I)
{
    S key = rec ? S32(rec, 4) : -1;
    I row = Ord10695(key);
    if (!row) return 0;
    const I extra = R32(row, 0xb0);
    const S n7 = (S)Ord10428(unit, rec, extra);
    const S slot = S16(row, 0x98);
    I node = 0;
    S ofs = 0;
    if (n7 > 0) {
        if (slot < 1 || S32(DT(), 0xc4) <= slot) return 0;
        node = Ord10871(unit, slot);
        if (node) ofs = (S)Ord10959(node) - S32(game, 0xa8);
    }
    S ret = 0;
    bool life = false;
    for (S i = 0; i < 3; i++) {
        const S id = S16(row, 0x9e + i * 2);
        if (id < 0 || S32(DT(), 0xbd4) <= id) break;
        const I srow = (I)((uintptr_t)id * 0x144 + R32(DT(), 0xbcc));
        if (!srow) continue;
        S v = (S)Ord10428(unit, rec, R32(row, 0xa4 + i * 4));
        if (id == 0x1a || id == 0x4a) v <<= 8;
        v <<= (R8(srow, 0x18) & 31);
        if (v <= 0) continue;
        if (id == 6) life = true;
        if ((id == 6 || id == 0x4a) && unit && R32(unit, 0) == 0) v = (S)D2GAME_4EA30(unit, v);
        if (n7 < 1) {
            S cur = (S)Ord10973(unit, id, 0);
            S lim = U16(srow, 0x32);
            if (lim < S32(DT(), 0xbd4)) {
                S l = (S)Ord10973(unit, lim, 0);
                if (cur + v > l) v = l - cur;
            }
            Ord10551(unit, id, v, 0);
        } else {
            const U until = (U)(S32(game, 0xa8) + ofs + n7);
            if (!node) {
                S t = 6, k = -1;
                if (unit) { k = S32(unit, 0xc); t = S32(unit, 0); }
                node = Ord11013(R32(game, 0x1c), 2, until, t, k);
                if (!node) continue;
                Ord10785(node, slot);
                Ord10945(unit, slot, 1);
                if (!unit) {
                    D2GAME_8EA80(game, unit, 3, S32(game, 0xa8) + 1, 0, 0, 0);
                } else if (R32(unit, 0) != 0) {
                    for (I p = (I)R32(unit, 0xdc); p;) {
                        I nx = (I)R32(p, 0x24);
                        if (R8(p, 0) == 3) D2GAME_8E6F0(p, game);
                        p = nx;
                    }
                    D2GAME_8EA80(game, unit, 3, S32(game, 0xa8) + 1, 0, 0, 0);
                }
                Ord10911(node, (I)&D2GAME_9F7A0);
                Ord10807(unit, node, 1);
            }
            Ord11043(node, until);
            S r = (S)Ord10680(node, id, 0);
            S mix = r * ofs + v;
            if (i == 0) D2GAME_8EA80(game, unit, 0xc, until, 0, 0, 0);
            if (ofs + n7 > 0) Ord10188(node, id, mix / (ofs + n7), 0);
        }
        ret = 1;
    }
    if (life && unit && R32(unit, 0) == 0) {
        uint8_t old = (uint8_t)Ord10973(unit, 0x160, 0);
        S a = (S)Ord10973(unit, 6, 0) >> 8;
        S b = (S)Ord10913(unit) >> 8;
        uint8_t nv = (b > 0 && a < b) ? (uint8_t)((a << 7) / b) : 0x80;
        S d = (S)old - (S)nv;
        if (d < 0) d = -d;
        if (d > 4) {
            D2GAME_7A260(unit, nv);
            Ord10887(unit, 0x160, nv, 0);
        }
    }
    return ret;
}

// The same rule through the callee seam (the FN probe's form): ECX = game, EDX = unit, stack item + 4 unused (ret 0x14); gdt is
// the address of the module's sgptDataTables pointer. Differs from 0xAE6D0 in the value scaling (stats 0x1A/0x4A << 8, then
// << the stat's shift byte isc+0x18), the base-stat clamp (max stat looked up only when in range) and the life-orb refresh,
// which reads stat 0x160 first and pushes the orb only when the fill byte moved by more than 4.
#include "d2game_seam4e.h"
extern "C" uint32_t d2game_aea40(uint32_t game, uint32_t unit, uint32_t item, uint32_t gdt, const D2Seam* e)
{
    // 1.13c D2Game+0xAEA40
    const uint32_t rec = e->call(0xA53E, 0, 0, 0, 0, 0, item ? d2rd32(item + 4) : 0xFFFFFFFFu);
    if (!rec) return 0;
    const int32_t len = (int32_t)e->call(0xADBA, 0, 0, 0, 0, 0, unit, item, d2rd32(rec + 0xB0));
    const int32_t itype = (int16_t)d2rd16(rec + 0x98);
    uint32_t list = 0;
    int32_t remaining = 0;
    if (len > 0) {
        if (itype <= 0 || itype >= (int32_t)d2rd32(d2rd32(gdt) + 0xC4)) return 0;
        list = e->call(0xA5D4, 0, 0, 0, 0, 0, unit, (uint32_t)itype);
        if (list) remaining = (int32_t)(e->call(0xAB74, 0, 0, 0, 0, 0, list) - d2rd32(game + 0xA8));
    }
    uint32_t result = 0, life = 0;
    for (int32_t i = 0; i < 3; i++) {
        const int32_t stat = (int16_t)d2rd16(rec + 0x9E + 2 * i);
        if (stat < 0) break;
        const uint32_t dt = d2rd32(gdt);
        if (stat >= (int32_t)d2rd32(dt + 0xBD4)) break;
        const uint32_t isc = stat * 0x144u + d2rd32(dt + 0xBCC);
        if (!isc) continue;
        uint32_t v0 = e->call(0xADBA, 0, 0, 0, 0, 0, unit, item, d2rd32(rec + 0xA4 + 4 * i));
        if (stat == 0x1A || stat == 0x4A) v0 <<= 8;
        int32_t add = (int32_t)(v0 << (d2rd8(isc + 0x18) & 31));
        if (add <= 0) continue;
        if (stat == 6) life = 1;
        if ((stat == 6 || stat == 0x4A) && unit && d2rd32(unit) == 0)
            add = (int32_t)e->call(0x4EA30, unit, (uint32_t)add, 0, 0, game);
        if (len <= 0) {
            const int32_t old = (int32_t)e->call(0xA42A, 0, 0, 0, 0, 0, unit, (uint32_t)stat, 0);
            const uint32_t mstat = d2rd16(isc + 0x32);
            if ((int32_t)mstat < (int32_t)d2rd32(d2rd32(gdt) + 0xBD4)) {
                const int32_t maxv = (int32_t)e->call(0xA42A, 0, 0, 0, 0, 0, unit, mstat, 0);
                if (old + add > maxv) add = maxv - old;
            }
            e->call(0xA4D2, 0, 0, 0, 0, 0, unit, (uint32_t)stat, (uint32_t)add, 0);
            result = 1;
            continue;
        }
        const uint32_t expire = d2rd32(game + 0xA8) + (uint32_t)remaining + (uint32_t)len;
        if (!list) {
            list = e->call(0xA6A0, 0, 0, 0, 0, 0, d2rd32(game + 0x1C), 2, expire,
                           unit ? d2rd32(unit) : 6, unit ? d2rd32(unit + 0xC) : 0xFFFFFFFFu);
            if (!list) continue;
            e->call(0xA5B6, 0, 0, 0, 0, 0, list, (uint32_t)itype);
            e->call(0xA3CA, 0, 0, 0, 0, 0, unit, (uint32_t)itype, 1);
            if (!unit || d2rd32(unit) != 0) {
                if (unit)
                    for (uint32_t n = d2rd32(unit + 0xDC); n; ) {
                        const uint32_t next = d2rd32(n + 0x24);
                        if (d2rd8(n) == 3) { e->call(0x8E6F0, n); e->edi(0x8E6F0, game); }
                        n = next;
                    }
                e->call(0x8EA80, 0, game, unit, 0, 0, 3, d2rd32(game + 0xA8) + 1, 0, 0, 0);
            }
            e->call(0xA75A, 0, 0, 0, 0, 0, list, 0);   // stack word 1: the module callback 0x9F7A0 (masked in the capture)
            e->call(0xA60A, 0, 0, 0, 0, 0, unit, list, 1);
        }
        e->call(0xA7C6, 0, 0, 0, 0, 0, list, expire);
        const int32_t v = (int32_t)e->call(0xA712, 0, 0, 0, 0, 0, list, (uint32_t)stat, 0) * remaining + add;
        if (i == 0) e->call(0x8EA80, 0, game, unit, 0, 0, 0xC, expire, 0, 0, 0);
        const int32_t span = remaining + len;
        if (span > 0) e->call(0xA6E2, 0, 0, 0, 0, 0, list, (uint32_t)stat, (uint32_t)(v / span), 0);
        result = 1;
    }
    if (life && unit && d2rd32(unit) == 0) {
        const uint8_t was = (uint8_t)e->call(0xA42A, 0, 0, 0, 0, 0, unit, 0x160, 0);
        const int32_t cur = (int32_t)e->call(0xA42A, 0, 0, 0, 0, 0, unit, 6, 0) >> 8;
        const int32_t mx = (int32_t)e->call(0xA3A0, 0, 0, 0, 0, 0, unit) >> 8;
        const uint8_t fill = (mx > 0 && cur < mx) ? (uint8_t)((cur << 7) / mx) : 0x80;
        const int32_t d = (int32_t)was - (int32_t)fill;
        if ((d < 0 ? -d : d) > 4) {
            e->call(0x7A260, 0, 0, 0, 0, unit, (item & ~0xFFu) | fill);   // the item argument's slot with its low byte replaced
            e->call(0xA472, 0, 0, 0, 0, 0, unit, 0x160, fill, 0);
        }
    }
    return result;
}
