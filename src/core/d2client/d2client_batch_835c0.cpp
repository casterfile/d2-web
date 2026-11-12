// src/core/d2client/d2client_batch_835c0.cpp -- 12 UNMAPPED 1.13c D2Client.dll rows (0x835C0..0x84AF0, the item
// packet handlers in UNIT\Item.cpp) reimplemented from the cached decompiles. Alpha batch lane (OBJECTIVE v8.3):
// implementation + marker only, no spec, no vector. Base 0x6FAB0000.
// Unrecovered callees (ordinals, other RVAs) are host seams (the d2client_batch_5d110.cpp pattern).
// Register-passed args (EAX/EBX/ESI/EDI) are explicit parameters; pkt = the server packet (+1 kind, +2 len,
// +4 item id, +8 owner idx byte, +9 owner id u32 for some kinds, +8/+0xD decoded payload).
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <emscripten.h>

typedef uint8_t* P;
template <class T> static T rd(const uint8_t* p, size_t o) { T v; memcpy(&v, p + o, sizeof v); return v; }
template <class T> static void wr(uint8_t* p, size_t o, T v) { memcpy(p + o, &v, sizeof v); }

struct D2cItem83 {
    P*       itemHash;    // +0x10AE08 DAT_6fbbae08, 128 buckets, unit chain via seam next()
    P*       monHash;     // +0x10A808 DAT_6fbba808 (type-1 unit hash)
    P        player;      // +0x10BBFC DAT_6fbcbbfc
    uint32_t aux;         // +0x109854 DAT_6fbc9854
    uint32_t cc3b8;       // +0x10C3B8
    int32_t  c834, c854, c858, c864;   // +0x10B834 / 854 / 858 / 864 cursor-item state
    uint8_t  c484[4];     // +0x10C484
    int32_t  c824, c828;  // +0x10B824 / 828 mouse pos
    int32_t  c978e; uint8_t c9781; uint32_t c9786;   // +0x10978E / 781 / 786
    int32_t  c9721; uint32_t c971d;                  // +0x109721 / 71D
    int32_t  c9663;       // +0x109663
    int32_t  cbc54;       // +0x10BC54
};

struct D2cItem83Env {
    void   (*fatal)(uint32_t code);                 // Ordinal_10265+10024+_exit(-1)
    void   (*decode)(const uint8_t* src, int32_t len, int32_t flag, uint8_t* out); // Ordinal_11105
    P      (*findPlayer)(uint32_t id, uint32_t idx);// FUN_54e20
    P      (*next)(P u);                            // unit +0xE4 chain
    void   (*hashRemove)(uint32_t id);              // FUN_56ef0(itemHash, id&0x7f, id, 4)
    int    (*flagTest)(uint32_t w, uint32_t bit);   // Ordinal_10197
    P      (*fn333f0)(const uint8_t* pkt, const uint8_t* dec);
    P      (*fn32980)(uint32_t a, uint32_t b, uint32_t c);
    void   (*fn32820)(uint32_t a, uint32_t b);
    void   (*fn31aa0)(void);
    void   (*fn31ff0)(void);
    void   (*fn3bbf0)(void);
    void   (*fn6e2b0)(void);
    void   (*fn62d0)(void);                         // FUN_6fac62d0
    void   (*fn38a70)(uint32_t, uint32_t, uint32_t, uint32_t);
    void   (*fn7a70)(uint32_t id);
    void   (*fn7d00)(uint32_t id);
    void   (*fn7ab0)(void);
    void   (*fn32660)(P u);
    void   (*fn4940)(void);
    // 0x835C0 internals
    P      (*alloc11153)(uint32_t a, uint32_t b);   // Ordinal_11153(0,4)
    P      (*fn55a60)(uint32_t a);
    uint32_t (*o10826)(P);
    uint8_t  (*o10459)(uint32_t);
    void   (*o10936)(void);
    void   (*o10874)(uint32_t, P);
    void   (*o10679)(P);
    void   (*o10614)(void);
    void   (*o10445)(P);
    void   (*fn16950)(void);
    P      (*fn57360)(void);
    int    (*o11145)(P u, const uint8_t* data, uint32_t len, uint32_t a, uint32_t b, uint32_t n, int32_t* fail); // fail: out flag, tested instead of the return
    void   (*fn32820m)(uint32_t mode, uint32_t a, uint32_t b);   // the mode set, with the item mode in EAX (item creation)
    void   (*o10977)(P u, uint32_t a, uint32_t b);
    int    (*o10202)(P u, uint32_t a, uint32_t b, const char* file);
    // shared ordinals
    void   (*setCursor)(P owner, P item);           // Ordinal_10466
    int32_t (*getCursor)(P owner);                  // Ordinal_11017
    uint8_t (*o10020)(P u);
    uint32_t (*o10584)(P owner, uint32_t a, uint32_t b);
    void   (*o10869)(const char* file, uint32_t line, P owner, uint32_t x, uint32_t y, uint32_t q, uint32_t one, uint32_t c);
    P      (*o10646)(P owner, P item);
    void   (*o11127)(P owner, P item);
    void   (*o10061)(P owner);
    void   (*o10012)(P u, uint32_t v);
    int    (*o10405)(P owner, P item, uint32_t m);
    void   (*o10882)(P u, uint32_t v);
    void   (*o11118)(P u, uint32_t v);
    void   (*o10655)(P owner, P item, uint32_t v);
    uint32_t (*fn1630)(P u);
    uint32_t (*fn1660)(P u);
    int    (*o11057)(P a, P b);
    uint32_t (*o10867)(P);
    int    (*o11107)(P owner, P item, uint32_t a, uint32_t b, uint32_t c);
    P      (*mouseWnd)(void);                       // Ordinal_10048 -> HWND
    void   (*sendMouse)(P hwnd, uint32_t msg, uint32_t wp, uint32_t lp); // SendMessageA
    int    (*o10391)(P owner, P item);
    int    (*o11148)(P a, P b, uint32_t x, uint32_t y, uint32_t z);
    // 0x84300 internals
    uint32_t (*o11139)(P owner, uint8_t v);
    void   (*fn193c0)(P pl);
    void   (*o10694)(P owner, uint32_t v);
    void   (*fn19a90)(P pl);
    void   (*o10605)(P pl, P owner, uint8_t* buf, uint8_t v);
    void   (*o10600)(P pl, P item, uint32_t a, uint32_t b);
    void   (*o10608)(P owner, uint8_t v);
    void   (*o10058)(uint32_t a, uint32_t b);
    // 0x83790/0x83880
    void   (*o10887)(P u, uint32_t a, uint32_t b, uint32_t c);
    void   (*fn81aa0)(void);
    void   (*fn62d0b)(void);
};

static const char* kItemCpp = "..\\Source\\D2Client\\UNIT\\Item.cpp";

// item-hash chain walk shared by every handler: nullptr if absent, fatal(0x5a) if found but not type 4
static P findItem(const D2cItem83* s, const D2cItem83Env* e, uint32_t id) {
    for (P u = s->itemHash[id & 0x7f]; u; u = e->next(u))
        if (rd<uint32_t>(u, 0xC) == id) { if (rd<int32_t>(u, 0) != 4) { e->fatal(0x5a); return nullptr; } return u; }
    return nullptr;
}
static void setCur(D2cItem83* s, const D2cItem83Env* e, P owner, P item) {
    e->setCursor(owner, item);
    s->c834 = e->getCursor(owner);
    s->c858 = 0; s->c854 = 5; s->c864 = s->c834 ? 4 : 1;
}

// Item creation from a packet (pkt=EAX/param_1, init=param_2 or null, a=EBX&0xFFFF, b=low16 of arg 3, esi)
extern "C" EMSCRIPTEN_KEEPALIVE P d2_client_835c0(D2cItem83* s, const D2cItem83Env* e, const uint8_t* pkt,
                                                   const uint8_t* init, uint32_t a, uint32_t b, uint32_t esi) {
    uint8_t buf[20];
    a &= 0xffff; b &= 0xffff;
    if (!init) { e->decode(pkt + 8, rd<uint8_t>(pkt, 2) - 8, 0, buf); init = buf; }
    uint32_t mode = rd<uint8_t>(init, 8);
    a &= 0xffff; b &= 0xffff;   // both words are zero-extended before use
    uint32_t v = rd<uint32_t>(init, 0), id = rd<uint32_t>(pkt, 4);
    P u = e->alloc11153(0, 4), r;
    if (!u) r = nullptr;
    else {
        uint32_t f = rd<uint32_t>(u, 0xC8);
        wr<uint32_t>(u, 0xC8, s->aux == 0 ? (f & 0xFDFFFFFF) : (f | 0x2000000));
        wr<uint32_t>(u, 0, 4); wr<uint32_t>(u, 4, v); wr<uint32_t>(u, 0xC, id);
        wr<uint32_t>(u, 0xD4, 0); wr<uint32_t>(u, 8, 0);
        P pl = nullptr;
        if (a != 0 || b != 0) {
            pl = e->fn55a60(a);
            wr<uint8_t>(u, 0x18, e->o10459(e->o10826(pl)));
            wr<uint32_t>(u, 0x1C, s->cc3b8);
        }
        e->o10936(); e->o10874(0, u); e->o10679(u);
        if (pl) {
            uint64_t sd = (uint64_t)rd<uint32_t>(pl, 0x6C) * 0x6ac690c5u + rd<uint32_t>(pl, 0x70);
            wr<uint64_t>(pl, 0x6C, sd);
            wr<uint32_t>(u, 0x28, (uint32_t)sd);
            e->o10614();
        }
        e->o10445(u); e->fn32820m(mode, a, b); e->fn16950();
        r = e->fn57360();
    }
    int32_t fail = 0;
    e->o11145(r, pkt + 8, rd<uint8_t>(pkt, 2) - 8, 0, 0, 0x60, &fail);
    if (fail != 0) { e->hashRemove(id); return nullptr; }
    if (e->flagTest(rd<uint32_t>(init, 0xC), 2)) {
        e->o10977(r, 2, 0);
        if (esi && !e->o10202(r, 0x10, 0xd8, kItemCpp)) e->fn4940();
    }
    return r;
}

// 0x83790 and 0x83880 are ported to d2client_83790.cpp and d2client_83880.cpp.

// 1.13c D2Client+0x839E0 (pkt in EAX)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_839e0(D2cItem83* s, const D2cItem83Env* e, const uint8_t* pkt) {
    uint8_t buf[24];
    e->decode(pkt + 8, rd<uint8_t>(pkt, 2) - 8, 0, buf);
    if (!s->player) { e->fatal(0xb0c); return; }
    uint32_t id = rd<uint32_t>(pkt, 4);
    if (findItem(s, e, id)) e->hashRemove(id);
    P it = d2_client_835c0(s, e, pkt, buf, 0, 0, 0);
    P owner = (P)(uintptr_t)rd<uint64_t>(s->player, 0x60);
    if (!owner) { e->fatal(0xb13); return; }
    setCur(s, e, owner, it);
}

// 1.13c D2Client+0x83AD0 (pkt in EAX, flags EBX, EBP/ECX words as bp / hi)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_83ad0(D2cItem83* s, const D2cItem83Env* e, const uint8_t* pkt,
                                                      uint32_t ebx, uint16_t bp, uint16_t hi) {
    uint8_t buf[24];
    e->decode(pkt + 8, rd<uint8_t>(pkt, 2) - 8, 0, buf);
    if (!s->player) { e->fatal(0x8ef); return; }
    P it = findItem(s, e, rd<uint32_t>(pkt, 4));
    if (!it) it = d2_client_835c0(s, e, pkt, nullptr, 0, 0, 0);
    else { e->fn32820(0, 0); e->o10012(it, buf[0]); }
    e->o10882(it, bp); e->o11118(it, hi);
    P owner = (P)(uintptr_t)rd<uint64_t>(s->player, 0x60);
    if (!owner) { e->fatal(0x8ff); return; }
    if (e->o10405(owner, it, 2)) e->o10655(owner, it, e->fn1630(it));
    if (e->o11057(it, it)) e->fn31ff0();
    if (ebx & 0x400) e->o10977(it, 0x400, 1);
    e->setCursor(owner, nullptr);
    s->c834 = e->getCursor(owner);
    s->c858 = 0; s->c854 = 5; s->c864 = s->c834 ? 4 : 1;
    int32_t t = rd<int32_t>(it, 0);
    uint32_t k;
    P d = (P)(uintptr_t)rd<uint64_t>(it, 0x2C);      // retail piVar5[0xb]
    if (t == 2 || (t > 3 && t < 6)) k = rd<uint32_t>(d, 0xC);
    else k = d ? e->o10867(d) : 0;
    if (!(k > 3)) s->c484[k] = 1;
    e->sendMouse(e->mouseWnd(), 0x200, 0, ((uint32_t)(uint16_t)s->c824 << 16) | (uint16_t)s->c828);
    e->fn31aa0();
    if (s->c978e && s->c9781 == 0 && rd<uint32_t>(it, 0xC) == s->c9786) e->fn7ab0();
}

// 1.13c D2Client+0x83D00 (owner player in DAT, pkt=param_1, dec=EAX decoded block, ret=caller pkt)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_83d00(D2cItem83* s, const D2cItem83Env* e, const uint8_t* pkt,
                                                      const uint8_t* dec, uint32_t a, uint32_t b) {
    P pl = s->player;
    if (!pl) { e->fatal(0x8a6); return; }
    P it = findItem(s, e, rd<uint32_t>(pkt, 4));
    uint8_t c = e->o10020(it);
    P owner = (P)(uintptr_t)rd<uint64_t>(pl, 0x60);
    if (!owner) { e->fatal(0x8ab); return; }
    if (rd<int32_t>(pl, 0) == 0) e->o10061(owner);
    if (!e->o10202(it, 0x100, 0x96, kItemCpp)) e->o11127(pl, it);
    P r = e->o10646(owner, it);
    if (!r) { e->fatal(0x8b0); return; }
    if (r != it) { e->fatal(0x8b1); return; }
    uint32_t q = e->o10584(pl, c, s->aux);
    e->o10869(kItemCpp, 0x8b4, owner, rd<uint16_t>(dec, 4), rd<uint16_t>(dec, 6), q, 1, c);
    e->fn3bbf0();
    if (!e->flagTest(rd<uint32_t>(dec, 0xC), 1)) e->fn32980(0, rd<uint8_t>(dec, 8), 0);
    else {
        e->hashRemove(rd<uint32_t>(pkt, 4));
        pl = d2_client_835c0(s, e, pkt, dec, a, b, 0);
        if (!pl) { e->fatal(0x8bb); return; }
    }
    e->setCursor(owner, pl);
    e->getCursor(owner);
    e->fn62d0();
    e->fn31aa0();
}

// 1.13c D2Client+0x83EC0 (pkt in EAX, dec=param_1)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_83ec0(D2cItem83* s, const D2cItem83Env* e, const uint8_t* pkt, const uint8_t* dec) {
    P pl = s->player;
    if (!pl) { e->fatal(0x86c); return; }
    P it = findItem(s, e, rd<uint32_t>(pkt, 4));
    if (!e->flagTest(rd<uint32_t>(dec, 0xC), 1)) {
        if (!it) { e->fatal(0xac); return; }
        e->fn32820(0, 0); e->o10012(it, rd<uint8_t>(dec, 0x10));
    } else {
        e->hashRemove(rd<uint32_t>(pkt, 4));
        it = d2_client_835c0(s, e, pkt, dec, 0, 0, 0);
        if (!it) { e->fatal(0x874); return; }
    }
    P owner = (P)(uintptr_t)rd<uint64_t>(pl, 0x60);
    if (!owner) { e->fatal(0x87c); return; }
    e->o10882(it, rd<uint16_t>(dec, 4)); e->o11118(it, rd<uint16_t>(dec, 6));
    if (e->o10405(owner, it, 1)) {
        uint8_t c = e->o10020(it);
        if (rd<uint8_t>(dec, 0x10) != 0xff) {
            uint32_t q = e->o10584(pl, c, s->aux);
            uint32_t y = e->fn1660(it), x = e->fn1630(it);
            if (!e->o11107(owner, it, x, y, q)) { e->fatal(0x892); return; }
            if (e->o11057(it, pl)) e->fn31ff0();
            e->fn6e2b0();
            e->fn31aa0();
        }
    }
}


// 1.13c D2Client+0x84300 (this=item, pkt=param_1, dec=EAX)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_84300(D2cItem83* s, const D2cItem83Env* e, P self, const uint8_t* pkt, const uint8_t* dec) {
    P pl = e->findPlayer(rd<uint32_t>(pkt, 9), rd<uint8_t>(pkt, 8));
    if (!pl) return;
    if (rd<int32_t>(pl, 0) != 0) { e->fatal(0); return; }
    P owner = (P)(uintptr_t)rd<uint64_t>(pl, 0x60);
    if (!owner) { e->fatal(0x6ef); return; }
    uint8_t v = rd<uint8_t>(dec, 0x11);
    uint32_t h = e->o11139(owner, v);
    e->fn193c0(pl); e->o10694(owner, h); e->fn19a90(pl);
    uint8_t tmp[16];
    e->o10605(pl, owner, tmp, v);
    P r = e->o10646(owner, owner);
    if (!r) { e->fatal(0x6f7); return; }
    if (r != self) { e->fatal(0x6f8); return; }
    if (rd<int32_t>(pl, 0) == 0) e->o10061(owner);
    if (!e->o10202(r, 0x100, 0x96, kItemCpp)) e->o11127(pl, r);
    e->o10600(pl, r, 1, 1); e->o10608(owner, v);
    if (!e->flagTest(rd<uint32_t>(dec, 0xC), 1)) e->fn32980(0, rd<uint8_t>(dec, 8), 0);
    else {
        e->hashRemove(rd<uint32_t>(pkt, 4));
        if (pl != s->player) return;
        if (!e->fn333f0(pkt, dec)) { e->fatal(0x703); return; }
    }
    if (pl != s->player) return;
    e->fn3bbf0(); e->o10058(0, 0);
    e->setCursor(owner, (P)(uintptr_t)h);
    e->getCursor(owner);
    e->fn62d0(); e->fn31aa0();
}

// 1.13c D2Client+0x844F0 (pkt in EAX)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_844f0(D2cItem83* s, const D2cItem83Env* e, const uint8_t* pkt) {
    uint8_t buf[24];
    e->decode(pkt + 8, rd<uint8_t>(pkt, 2) - 8, 0, buf);
    P pl = e->findPlayer(rd<uint32_t>(pkt, 9), rd<uint8_t>(pkt, 8));
    if (!pl) return;
    P it = findItem(s, e, rd<uint32_t>(pkt, 4));
    if (!it) return;
    P owner = (P)(uintptr_t)rd<uint64_t>(pl, 0x60);
    if (!owner) { e->fatal(0x5a); return; }
    P r = e->o10646(owner, it);
    if (!r) return;
    if (r != it) { e->fatal(0x5a); return; }
    uint8_t c = e->o10020(it);
    uint32_t q = e->o10584(pl, c, s->aux);
    e->o10869(kItemCpp, 0x4cf, owner, (uint32_t)(uintptr_t)it & 0xffff, (uint32_t)(uintptr_t)it >> 16, q, 1, c);
    e->fn3bbf0();
    if (rd<int32_t>(pl, 0) == 0) e->o10061(owner);
    if (!e->o10202(it, 0x100, 0x96, kItemCpp)) e->o11127(pl, it);
    P ni = it;
    if (!e->flagTest(rd<uint32_t>(buf, 0xC), 0x20)) {
        if (!e->flagTest(rd<uint32_t>(buf, 0xC), 1)) e->fn32980(0, 0, 0);
        else {
            e->hashRemove(rd<uint32_t>(pkt, 4));
            ni = e->fn333f0(pkt, buf);
            if (!ni) { e->fatal(0x4dd); return; }
        }
        e->o10012(ni, 0xff);
        e->setCursor(owner, ni); e->getCursor(owner);
        e->fn62d0(); e->fn31aa0();
    } else {
        e->hashRemove(rd<uint32_t>(it, 0xC));
        if (pl != s->player) return;
        if ((uint8_t)q == 1) e->fn38a70(0, 0, 0, 0);
        else { s->c834 = 0; s->c864 = 1; s->c854 = 5; s->c858 = 0; }
    }
    if (pl == s->player) e->fn7d00(rd<uint32_t>(ni, 0xC));
}

// 1.13c D2Client+0x84990 (pkt in EAX, x/y in EDI, ESI words)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_84990(D2cItem83* s, const D2cItem83Env* e, const uint8_t* pkt, uint32_t edi, uint32_t esi) {
    uint8_t buf[24];
    e->decode(pkt + 8, rd<uint8_t>(pkt, 2) - 8, 0, buf);
    P it = findItem(s, e, rd<uint32_t>(pkt, 4));
    uint16_t x = (uint16_t)edi, y = (uint16_t)(edi >> 16);
    if (it) {
        P owner = (P)(uintptr_t)rd<uint64_t>(s->player, 0x60);
        if ((uintptr_t)e->getCursor(owner) == (uintptr_t)it) { e->setCursor(owner, nullptr); e->getCursor(owner); e->fn62d0(); }
        if (e->flagTest(esi, 1)) e->hashRemove(rd<uint32_t>(pkt, 4));
        else goto done;
    }
    it = d2_client_835c0(s, e, pkt, nullptr, x, y, 0);
done:
    if (!it) { e->fatal(0x3cf); return; }
    e->fn31aa0();
    if (x || y) e->fn55a60(x);
    e->fn32820(x, y);
    e->o10012(it, buf[0]);
}

// 1.13c D2Client+0x84AF0 (pkt in EAX, kind in DI)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_84af0(D2cItem83* s, const D2cItem83Env* e, const uint8_t* pkt, char kind) {
    uint8_t buf[24];
    e->decode(pkt + 8, rd<uint8_t>(pkt, 2) - 8, 0, buf);
    P owner = (P)(uintptr_t)rd<uint64_t>(s->player, 0x60);
    if (kind != 4) return;
    uint32_t id = rd<uint32_t>(pkt, 4);
    for (P u = s->itemHash[id & 0x7f]; u; u = e->next(u))
        if (rd<uint32_t>(u, 0xC) == id) { e->fatal(rd<int32_t>(u, 0) == 4 ? 0x3a0 : 0x5a); return; }
    P it = d2_client_835c0(s, e, pkt, nullptr, 0, 0, 0);
    if (!it) { e->fatal(0x3a2); return; }
    e->setCursor(owner, it); e->getCursor(owner);
    e->fn62d0(); e->fn31aa0();
}
