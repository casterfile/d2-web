// 1.13c D2Client+0x1AB20. Registers/stack from the disassembly: EAX = a, stack (frame, b), ret 8.
// The frame is the caller's block: +0 flags, +4 player, +8 target unit, +0x14, +0x18. Callees are seams; each carries the registers and
// stack words retail gives it.
#include <cstdint>
#include <cstring>
#include "d2client_1ab20.h"
namespace d2client_1ab20 {
static U rd(const void* p, U o) { U v; std::memcpy(&v, static_cast<const uint8_t*>(p) + o, 4); return v; }
static void wr(void* p, U o, U v) { std::memcpy(static_cast<uint8_t*>(p) + o, &v, 4); }
static P ptr(U v) { return reinterpret_cast<P>(static_cast<uintptr_t>(v)); }
// 1.13c D2Client+0x1AB20
void sub_1AB20(I a, const uint8_t* fr, I b, const Gl& g, const Fn& f) {
    P tgt = ptr(rd(fr, 8)); P pl = ptr(rd(fr, 4));
    auto send = [&]() { f.s7c0(pl, rd(fr, 0x18), 0); f.s2d50(pl, 0x13, U(a), U(b)); };
    // the shared tail: 29F30 gate, the 55B40 check for a == 4, the 2A750 call, then the player's four words cleared
    auto tail = [&]() -> bool {
        if (!f.s9f30()) return false;
        const U blk[7] = { U(a), U(b), 0, 0, 0, 0, 0 };
        if (a == 4) { P p = f.s5b40(b, a); if (p && rd(p, 0) == U(a) && !((rd(p, 0xc4) >> 1) & 1)) return false; }
        f.s750(pl, blk, 0);
        if (pl && rd(pl, 0) == 0 && rd(pl, 0x14)) { P x = ptr(rd(pl, 0x14)); for (U o = 0x150; o <= 0x15c; o += 4) wr(x, o, 0); }
        return true;
    };
    I limit = 5;
    U kind = tgt ? rd(tgt, 0) : 99;
    switch (kind) {
    case 0:
        if (f.cc70(tgt, pl) > 4 || f.cf0a(pl, tgt, 0x804)) { send(); return; }
        tail(); return;
    case 2: case 4: {
        if (fr[0] & 8) return;
        if (kind == 4) {
            if (f.cc70(tgt, pl) > 4 || f.cf0a(pl, tgt, 0x804)) {
                if (f.s8ac0(a, b)) return;
                f.s7c0(pl, rd(fr, 0x18), 0); f.s2d50(pl, 0x13, U(a), U(b)); *g.g3e4 = 1; return;
            }
            tail(); *g.g3e4 = 1; return;
        }
        f.c226(I(rd(tgt, 4)));
        bool ok = true;
        if (rd(tgt, 4) == 0x3b && f.c1fc(g.plG, 0x66)) ok = false;
        if (f.cefe(pl, tgt) && !f.cf0a(pl, tgt, 0x804)) { if (!ok) return; tail(); *g.g3e4 = 1; return; }
        if (f.s8ac0(a, b)) return;
        U r60 = f.s1660(tgt); U r30 = f.s1630(tgt);   // 1660 first, then 1630: the order retail calls them
        f.s010(r60, r30, rd(fr, 0x14), pl);
        if (!ok) return;
        f.s2d50(pl, 0x13, U(a), U(b)); *g.g3e4 = 1; return;
    }
    case 5:
        if (*g.g97ec < *g.g9828) return;
        if (f.cc70(tgt, pl) > 4) send(); else tail();
        *g.g9828 = *g.g97ec + 500; return;
    case 1: {
        P row = f.s1230(I(rd(tgt, 4)));
        if (row && (g.masks[1] & 0xff & static_cast<const uint8_t*>(row)[0xd])) limit = 2;
        break;
    }
    default: break;
    }
    // common path (type 1, types 3 and 6+, no target)
    if (tgt && rd(tgt, 0) == 1) {
        I id = I(rd(tgt, 4));
        if (id >= 0 && id < I(rd(g.tab, 0xa80))) {
            const uint8_t* row = static_cast<const uint8_t*>(ptr(rd(g.tab, 0xa78))) + U(id) * 0x1a8;
            if (row) {
                U v = row[0xd];
                if ((g.masks[1] & v) && (g.masks[0] & v)) {
                    if (rd(tgt, 0x2c)) f.cdd8(rd(tgt, 0x2c));
                    U r60 = f.s1660(tgt); U r30 = f.s1630(tgt);   // 1660 first, then 1630
                    f.s48a0((r30 & ~0xffu) | 0x59, I(rd(tgt, 0)), I(rd(tgt, 0xc)), r30, r60);
                    if (rd(tgt, 0x14)) *(static_cast<uint8_t*>(ptr(rd(tgt, 0x14))) + 0x28) |= 1;
                    f.s6c0(tgt, 1);
                }
            }
        }
    }
    if (f.cc70(tgt, pl) > limit) { send(); return; }
    tail();
}
}
