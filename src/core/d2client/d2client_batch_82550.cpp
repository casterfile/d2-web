// src/core/d2client/d2client_batch_82550.cpp -- 12 UNMAPPED 1.13c D2Client.dll rows (0x82550..0x833F0, the
// item/unit packet handlers around the unit hash 0x10AE08) reimplemented from the cached decompiles.
// Alpha batch lane (OBJECTIVE v8.3): implementation + marker only, no spec, no vector. Base 0x6FAB0000.
// Ordinals and unrecovered internal callees are host seams: Env::ord(n,...) / Env::fn(rva,...).
// Register-passed args (EAX/EBX/ESI/EDI) are explicit parameters, named by their decompiler register.
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <emscripten.h>

typedef uintptr_t U;
struct D2cEnv82 {
    U    (*ord)(uint32_t n, U a, U b, U c, U d);   // Ordinal_<n>
    U    (*fn)(uint32_t rva, U a, U b, U c, U d);  // unrecovered D2Client+rva
    void (*fatal)(uint32_t code);                  // Ordinal_10265+10024+_exit(-1); no return
};
struct D2cGlob82 {
    uint8_t*  player;       // +0x11BBFC DAT_6fbcbbfc
    uint8_t** unitHash;     // +0x10AE08, 128 buckets, chain at unit+0xE4
    uint8_t** unitHashN;    // +0x10A608, stride 0x200 per type
    uint32_t* slot484;      // +0x11C484 [4]
    uint32_t* g854;         // +0x119854 DAT_6fbc9854
    uint32_t  tick;         // +0x11C3B8 DAT_6fbcc3b8
    uint32_t* g834;         // +0x11B834 (b834,b854,b858,b864 as [0..3])
    const uint8_t* g9780;   // +0x119780: the bytes holding the 978e dword, the 9781 byte and the 9786 dword (only 0x82F50 reads it)
};

template <class T> static T rd(const uint8_t* p, size_t o) { T v; memcpy(&v, p + o, sizeof v); return v; }
template <class T> static void wr(uint8_t* p, size_t o, T v) { memcpy(p + o, &v, sizeof v); }
static U P(const void* p) { return (U)p; }

// type-4 (item) unit lookup by id in the hash; nullptr if absent. Fatal 0x5A if found with wrong type.
static uint8_t* findUnit(const D2cGlob82* g, const D2cEnv82* e, uint32_t id, uint32_t type) {
    for (uint8_t* u = g->unitHash[id & 0x7f]; u; u = (uint8_t*)rd<U>(u, 0xE4))
        if (rd<uint32_t>(u, 0xC) == id) {
            if (rd<int32_t>(u, 0) != (int32_t)type) e->fatal(0x5A);
            return u;
        }
    return nullptr;
}

// 1.13c D2Client+0x82550   (EAX = obj)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_82550(const D2cGlob82* g, const D2cEnv82* e, uint8_t* eax) {
    e->fn(0x69700, 0, 0, 0, 0);
    if (e->ord(10757, P(eax), 0, 0, 0)) {
        e->fn(0xa5c50, 0, 0, 0, 0);
        e->ord(10820, rd<uint32_t>(eax, 0x60), 0, 0, 0);
        wr<uint32_t>(eax, 0x60, 0);
    }
    if (g->tick) {
        e->ord(10408, P(eax), 0, 0, 0);
        U u = e->ord(10331, P(eax), 0, 0, 0);
        if (e->ord(10821, u, P(eax), 0, 0)) e->ord(11016, P(eax), 0, 0, 0);
    }
}

// 1.13c D2Client+0x825B0   (param_1 = ECX, in_EAX, unaff_EDI = mode)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_825b0(const D2cEnv82* e, uint8_t* p1, uint8_t* eax, int32_t edi) {
    if (!p1 || !eax) return;
    uint32_t v = rd<uint32_t>(p1, 0x60);
    if (!v) return;
    uint8_t c = (uint8_t)e->ord(10417, 0, 0, 0, 0);
    if (c == 11 || c == 12) return;
    e->ord(10694, v, 0, 0, 0);
    e->fn(0x69a90, P(p1), 0, 0, 0);
    if (edi == 1 && e->ord(11160, 0, 0, 0, 0) == 0) e->fn(0x81ff0, 0, 0, 0, 0);
    e->ord(10138, v, 0, 0, 0);
}

// 1.13c D2Client+0x82610   (ESI = unit, EDI = flag)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_82610(const D2cEnv82* e, uint8_t* esi, int32_t edi) {
    if (!esi || !edi) return;
    uint32_t h = rd<uint32_t>(esi, 0x60);
    if (!h) return;
    e->ord(10694, h, 0, 0, 0);
    if (rd<int32_t>(esi, 0) == 0) e->ord(10061, h, 0, 0, 0);
    e->ord(10202, 0, 0, 0, 0);
    e->ord(11127, 0, 0, 0, 0);
    e->fn(0x69a90, P(esi), 0, 0, 0);
}

// 1.13c D2Client+0x82660   (ECX = unit, EAX = x, stack: arg1 = y, arg2 = kind (bl), arg3 = owner (+0x60 = room/inv handle); ret 0xC)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_82660(const D2cGlob82* g, const D2cEnv82* e, uint8_t* unit,
                                                    uint32_t eax, uint8_t* owner, uint32_t arg1, uint32_t kind) {
    uint32_t x = eax & 0xffff;
    e->ord(10882, P(unit), x, 0, 0);
    e->ord(11118, P(unit), arg1 & 0xffff, 0, 0);
    U h = rd<U>(owner, 0x60);
    if (!h) { e->fatal(0x413); return; }
    if (!e->ord(10405, h, P(unit), 1, 0)) return;
    int32_t t = rd<int32_t>(unit, 0);
    U sub = rd<U>(unit, 0x2c);
    if (t != 2 && (t < 4 || t > 5) && sub) e->ord(10750, sub, 0, 0, 0);
    uint32_t v = 0;
    if (t == 2 || (t > 3 && t < 6)) v = rd<uint32_t>((uint8_t*)sub, 0xc);
    else if (sub) v = (uint32_t)e->ord(10867, sub, 0, 0, 0);
    uint32_t g854 = *g->g854;
    U r = e->ord(10584, P(owner), kind, g854, 1);
    if (e->ord(11107, h, P(unit), v, P(owner)) && P(owner) == (U)g->player) {
        (void)r;
        if ((uint8_t)kind == 1) { e->fn(0x81e40, P(unit), 0, 0, 0); e->fn(0x88a70, 0, 0, 0, 0); }
        else { e->ord(10466, h, 0, 0, 0); U c = e->ord(11017, h, 0, 0, 0); e->fn(0x162d0, c, 0, 0, 0); }
        if (P(owner) == (U)g->player && (uint8_t)kind == 3) {
            U q = e->ord(10695, rd<uint32_t>(unit, 4), 0, 0, 0);
            int32_t k = rd<int32_t>((uint8_t*)q, 0x80);
            if (k == 0x20747368 || k == 0x20326671) e->fn(0x8ca80, (U)k, 0, 0, 0);
        }
    }
}

// 1.13c D2Client+0x827C0   (EAX = pkt)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_827c0(const D2cEnv82* e, U eax) {
    int32_t n = (int32_t)e->fn(0x81e40, 0, 0, 0, 0);
    e->fn(0x88a70, eax, 0, 0, 0);
    if (n > 0) {
        U r = e->fn(0x88a70, eax, 0xc, 0, 0);
        if ((int32_t)r) e->fn(0x87ed0, 0, r, 0xb4, 0);
    }
}

// 1.13c D2Client+0x82980   (param_1 = lo, param_2, param_3 = flag, EAX = unit, BX = hi)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_82980(const D2cEnv82* e, uint16_t p1, uint32_t p2, int32_t p3,
                                                    U eax, uint16_t bx) {
    if (!eax) { e->fatal(0xac); return; }
    if (p3 && (p1 || bx)) e->fn(0xa5a60, p1, 0, 0, 0);
    e->fn(0x82820, p1, bx, 0, 0);
    e->ord(10012, 0, 0, 0, 0);
    (void)p2;
}

// 1.13c D2Client+0x82A00   (EAX = pkt, param_1 = pkt2)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_82a00(const D2cGlob82* g, const D2cEnv82* e, const uint8_t* eax,
                                                    const uint8_t* p1) {
    uint8_t* pl = g->player;
    if (!pl) return;
    uint8_t* u = findUnit(g, e, rd<uint32_t>(eax, 4), 4);
    if (!u) return;
    uint32_t h = rd<uint32_t>(pl, 0x60);
    if (!h) { e->fatal(0x98d); return; }
    U f = e->ord(10646, h, P(u), 0, 0);
    if (!f) { e->fatal(0x98f); return; }
    if (f != P(u)) { e->fatal(0x990); return; }
    if (rd<int32_t>(pl, 0) == 0) e->ord(10061, rd<uint32_t>(pl, 0x60), 0, 0, 0);
    if (!e->ord(10202, P(u), 0x100, 0x96, 0)) e->ord(11127, P(pl), P(u), 0, 0);
    e->ord(10267, h, rd<uint16_t>(p1, 4), 0, 0);
    e->fn(0x82980, 0, rd<uint8_t>(p1, 8), 0, 0);
    e->ord(10466, h, P(u), 0, 0);
    e->ord(11017, h, 0, 0, 0);
    e->fn(0x162d0, 0, 0, 0, 0);
    e->fn(0x81aa0, 0, 0, 0, 0);
}

// 1.13c D2Client+0x82B20   (param_1 = pkt, param_2 = pkt2)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_82b20(const D2cGlob82* g, const D2cEnv82* e, const uint8_t* p1,
                                                    const uint8_t* p2) {
    uint8_t* pl = g->player;
    if (!pl) { e->fatal(0x957); return; }
    uint8_t* u = findUnit(g, e, rd<uint32_t>(p1, 4), 4);
    if (!u) return;
    uint32_t mode = rd<uint8_t>(p2, 0x10);
    e->fn(0x82820, 0, 0, 0, 0);
    e->ord(10012, P(u), mode, 0, 0);
    uint32_t h = rd<uint32_t>(pl, 0x60);
    if (!h) { e->fatal(0x963); return; }
    e->ord(10882, P(u), rd<uint16_t>(p2, 4), 0, 0);
    e->ord(11118, P(u), rd<uint16_t>(p2, 6), 0, 0);
    if (e->ord(10744, P(u), 0xd, 0, 0)) e->fn(0x81ff0, 0, 0, 0, 0);
    if (e->ord(10405, h, P(u), 2, 0)) {
        U r = e->fn(0x1630, P(u), 0, 0, 0);
        if (!e->ord(10655, h, P(u), r, 0)) { e->fatal(0x970); return; }
    }
    e->fn(0xbe2b0, 0, 0, 0, 0);
    e->fn(0x81aa0, 0, 0, 0, 0);
}

static void unlinkUnit(const D2cGlob82* g, const D2cEnv82* e, uint32_t id) {
    e->fn(0xa6ef0, P(g->unitHash), id & 0x7f, id, 4);   // FUN_6fb56ef0(hash, bucket, id, type)
}

// 1.13c D2Client+0x82C60   (param_1 = pkt)
// ported to d2client_82c60.cpp

// 1.13c D2Client+0x82E10   (ESI = pkt)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_82e10(const D2cGlob82* g, const D2cEnv82* e, const uint8_t* esi) {
    uint8_t loc[24];
    e->ord(11105, P(esi + 8), rd<uint8_t>(esi, 2) - 8, 0, P(loc));
    uint8_t* u = findUnit(g, e, rd<uint32_t>(esi, 4), 4);
    if (!u) return;
    int32_t m = rd<int32_t>(u, 0x10);
    if (m == 2) e->fn(0x82a00, P(esi), P(loc), 0, 0);        // stdcall: EAX = pkt, [esp+4] = block
    else if (m == 4) e->fn(0x82b20, P(esi), P(loc), 0, 0);   // stdcall: pkt, block
}

// 1.13c D2Client+0x82F50   (EAX = pkt: +1 type byte, +2 id)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_82f50(const D2cGlob82* g, const D2cEnv82* e, const uint8_t* eax,
                                                    uint32_t (*lookup)(uint32_t id, const void* bucket, uint32_t type)) {
    (void)lookup;
    uint32_t t = rd<uint8_t>(eax, 1);
    uint8_t* u = (uint8_t*)(U)e->fn(0xa4e20, rd<uint32_t>(eax, 2), P(g->unitHashN) + t * 0x200, t, 0);
    if (!u || u != g->player) return;
    uint32_t h = rd<uint32_t>(u, 0x60);
    uint8_t* it = (uint8_t*)e->ord(11017, h, 0, 0, 0);
    if (!it) return;
    uint32_t id = rd<uint32_t>(it, 0xC);
    e->ord(10466, h, 0, 0, 0);
    e->ord(11017, h, 0, 0, 0);
    e->fn(0x162d0, 0, 0, 0, 0);
    e->fn(0xa6ef0, P(g->unitHashN) + rd<uint32_t>(it, 0) * 0x200, id & 0x7f, id, rd<uint32_t>(it, 0));
    // DAT_6fbc978e != 0 && DAT_6fbc9781 == 1 && id == DAT_6fbc9786
    if (rd<uint32_t>(g->g9780, 0xe) != 0 && rd<uint8_t>(g->g9780, 1) == 1 && id == rd<uint32_t>(g->g9780, 6))
        e->fn(0x47ab0, 0, 0, 0, 0);
}

// 1.13c D2Client+0x833F0   (stdcall 4: pkt, loc or null, a3 = low word, flag = esi; EBX = hi)
extern "C" EMSCRIPTEN_KEEPALIVE U d2_client_833f0(const D2cGlob82* g, const D2cEnv82* e, const uint8_t* p1,
                                                 uint32_t* loc, uint32_t a3, uint32_t ebx, int32_t esi) {
    uint32_t lbuf[5];
    int32_t n = rd<uint8_t>(p1, 2) - 0xd;
    if (!loc) { e->ord(11105, P(p1 + 0xd), n, 0, P(lbuf)); loc = lbuf; }
    uint32_t local24 = rd<uint8_t>((uint8_t*)loc, 8);
    uint32_t v6 = loc[0], id = rd<uint32_t>(p1, 4), hi = ebx & 0xffff;
    uint8_t* u = (uint8_t*)e->ord(11153, 0, 4, 0, 0);
    U ret = 0;
    int32_t iVar9 = 0;
    if (u) {
        uint32_t fl = rd<uint32_t>(u, 0xC8);
        fl = *g->g854 ? (fl | 0x2000000) : (fl & 0xfdffffff);
        wr<uint32_t>(u, 0xC8, fl);
        wr<uint32_t>(u, 0, 4); wr<uint32_t>(u, 4, v6); wr<uint32_t>(u, 0xC, id);
        wr<uint32_t>(u, 0xD4, 0); wr<uint32_t>(u, 8, 0);
        if (hi || (a3 & 0xffff)) {
            iVar9 = (int32_t)e->fn(0xa5a60, hi, a3 & 0xffff, 0, 0);
            U c = e->ord(10826, (U)iVar9, 0, 0, 0);
            wr<uint8_t>(u, 0x18, (uint8_t)e->ord(10459, c, 0, 0, 0));
            wr<uint32_t>(u, 0x1C, g->tick);
        }
        e->ord(10936, 0, 0, 0, 0);
        e->ord(10874, 0, P(u), 0, 0);
        e->ord(10679, P(u), 0, 0, 0);
        if (iVar9) {
            uint8_t* o = (uint8_t*)(U)(uint32_t)iVar9;
            uint64_t s = (uint64_t)rd<uint32_t>(o, 0x6c) * 0x6ac690c5ull + rd<uint32_t>(o, 0x70);
            wr<uint64_t>(o, 0x6c, s);
            wr<uint32_t>(u, 0x28, (uint32_t)s);
            e->ord(10614, 0, 0, 0, 0);
        }
        e->ord(10445, P(u), 0, 0, 0);
        e->fn(0x82820, hi, a3 & 0xffff, rd<uint8_t>(p1, 2) - 0xd, 0);
        e->fn(0x66950, 0, 0, 0, 0);
        ret = e->fn(0xa7360, (U)u, 0, 0, 0);
    }
    if (e->ord(11145, ret, P(p1 + 0xd), (U)esi, 0x60 /* + &local24 */)) {
        unlinkUnit(g, e, rd<uint32_t>(p1, 4));
        return 0;
    }
    if (e->ord(10197, rd<uint32_t>((uint8_t*)loc, 0xC), 2, 0, 0)) {
        e->ord(10977, ret, 2, 0, 0);
        if (esi && !e->ord(10202, ret, 0x10, 0xfe, 0)) e->fn(0x14940, rd<uint32_t>(p1, 4), 0, 0, 0);
    }
    (void)local24;
    return ret;
}
