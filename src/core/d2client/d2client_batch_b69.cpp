// D2Client batch b69: env-seam style. Globals/callees arrive through one struct; each member names its address/ordinal.
// Skipped (hidden register/stack inputs not recoverable from the decompile): 0x82820 0x82FE0 0x84750 0x86540 0x8F710 0x953B0.
#include <cstdint>
#include <cstddef>
#include <initializer_list>
#include <emscripten.h>

#include "d2client_batch_b69.h"

static int32_t* d2c_b69_find(const uint32_t* tbl, uint32_t id) {
    int32_t* p = (int32_t*)(intptr_t)tbl[id & 0x7f];
    while (p && (uint32_t)p[3] != id) p = (int32_t*)(intptr_t)(uint32_t)p[0x39];
    return p;
}
template <class T> static void d2c_b69_put(uint8_t* b, size_t o, T v) { *(T*)(b + o) = v; }

// 86A40 shared tail: the "panel resync" body (inlined twice by the compiler). Returns nothing; early-outs mirror the original gotos.
static void d2c_b69_panel_body(const D2cB69Env* e) {
    for (int32_t u = 0; u < *e->g_8d5d0; u++) {
        if (e->g_aad80[u] != 1) continue;
        int32_t kind = *(const int32_t*)((intptr_t)e->g_8d538[u] + 4);
        if (kind == 1) e->f72790((uint32_t)u, 1, 0);
        else if (kind == 4) { if (*e->g_c9721) e->f7b20((uint32_t)*e->g_971d); }
        else if (kind != 0) return;
    }
    uint8_t* pl = *e->unit;
    if (!pl || (*(int32_t*)pl == 0 && e->f55440() == 0)) *e->g_aad84 = 1;
    int32_t a = *e->g_aad84, b;
    if (a) { e->f3bb90(); e->feccc0(4, e->ord10048(), e->g_805b0); }
    e->f3bbf0();
    int32_t w = *e->g_9e14, h = *e->g_8bc48;
    if (!*e->g_aad88 && !*e->g_aadd0 && !*e->g_aadbc && !*e->g_aadc0 && !*e->g_aae10 && !*e->g_aae14 && !*e->g_aadd8) {
        if (!a) { *e->g_cc418 = 0; *e->g_cc414 = 0; e->ord10006((uint32_t)*e->g_cc418); b = 0; a = w; }
        else { *e->g_cc418 = -((h + ((h >> 31) & 3)) >> 2); *e->g_cc414 = 1; e->ord10006((uint32_t)*e->g_cc418);
               b = (w + ((w >> 31) & 3)) >> 2; a = w - b; b = -b; }
    } else {
        if (a) { *e->g_cc418 = 0; *e->g_cc414 = 3; e->ord10006((uint32_t)*e->g_cc418); e->f3a4d0((uint32_t)*e->g_9e18, w, 0, (uint32_t)*e->g_cc3c4, 0);
                 *e->g_cc3dc = 0; *e->g_cc3e0 = 0; *e->g_cc3f8 = 0; *e->g_cc3fc = 0; return; }
        *e->g_cc418 = (h + ((h >> 31) & 3)) >> 2; *e->g_cc414 = 2; e->ord10006((uint32_t)*e->g_cc418);
        b = (w + ((w >> 31) & 3)) >> 2; a = w + b;
    }
    e->f3a4d0((uint32_t)*e->g_9e18, a, 0, (uint32_t)*e->g_cc3c4, b);
}

extern "C" {
// 1.13c D2Client+0x81120   (hidden EAX input = mode, explicit here)
EMSCRIPTEN_KEEPALIVE void d2c_81120(const D2cB69Env* e, int32_t mode) {
    uint8_t* u = *e->unit;
    if (mode < 0 || mode >= 4) { e->fatal(0x12a); return; }
    if ((*e->g_bf84 == 0 || mode == 0) && (*e->g_bf80 == 0 || mode == 2)) {
        *e->g_bf88 = mode;
        if (!u) { e->fatal(0x137); return; }
        switch (mode) {
        case 0: *e->g_c3934 = 0; break;
        case 1: e->f814f0(); *e->g_c3914 = (uint32_t)e->f18d0((uint64_t*)(u + 0x20), 0xE0, 0x20); break;
        case 2: e->f80ca0(); break;
        case 3: *e->g_bf6c = 0; break;
        }
    }
}

// 1.13c D2Client+0x811F0
EMSCRIPTEN_KEEPALIVE void d2c_811f0(const D2cB69Env* e, uint64_t* seed) {
    uint8_t* u = *e->unit;
    if (*e->g_bf80 == 0 || *e->g_bf88 == 2) {
        int32_t m = *e->g_bf88;
        if (*e->g_d343c == 0) {
            m = m + 1; if (m == 4) m = 0;
            *e->g_d3438 = e->f18d0(seed, e->tblEsi[m], e->tblEdi[m]);   // m is the new phase (asm reloads ESI/EDI by ebx)
            *e->g_d343c = *e->g_d3438;
            d2c_81120(e, m);   // original reads its mode from EAX = m
        }
        *e->g_d343c -= 1;
        if (m == 1) *e->g_c3934 = (uint32_t)(((uint64_t)((int64_t)(*e->g_d3438 - *e->g_d343c) * (int64_t)*e->g_c3914) & 0xffffffffu) / (uint64_t)(int64_t)*e->g_d3438);
        else if (m == 3) *e->g_c3934 = (*e->g_c3914 * (uint32_t)*e->g_d343c) / (uint32_t)*e->g_d3438;
        *e->g_c38f4 = (float)(int32_t)*e->g_c3934 * *e->g_875b0;
    } else if (*e->g_bf84 == 0) {
        *e->g_bf88 = 2;
        if (u) e->f80ca0(); else e->fatal(0x137);
    }
}

// 1.13c D2Client+0x81340
EMSCRIPTEN_KEEPALIVE void d2c_81340(const D2cB69Env* e) {
    uint8_t* u = *e->unit;
    uint32_t v5 = 0, v4 = 0;
    bool skip = false;
    if (u) {
        int32_t h = e->ord10826(e->ord10331(u));
        v5 = e->ord10215(h) != 0; v4 = e->ord10499(h) != 0;
        skip = v5 != 0;
    }
    if (!skip && *e->g_d3434) {
        if (*(int32_t*)(e->g_bf68 + 0x114)) e->ord10063();
        if (*(int32_t*)(e->g_bf60 + 0x114)) e->ord10063();
    }
    if (v4 == 0 && *e->g_d3430 && *(int32_t*)(e->g_bf64 + 0x114)) e->ord10063();
    *e->g_d3430 = (int32_t)v4; *e->g_d3434 = (int32_t)v5;
    if (!u) { e->fatal(0x547); return; }
    uint64_t* seed = (uint64_t*)(u + 0x20);
    if ((uint32_t)*e->g_bf70 < (uint32_t)*e->g_97f8) {   // retail cmp eax,ecx; jbe: unsigned
        *e->g_bf70 = *e->g_97f8;
        if (!v5) *e->g_c38f4 = 0;
        else {
            if (*(int32_t*)(e->g_bf68 + 0x114)) e->f80f60(seed);
            e->f80160(); d2c_811f0(e, seed); e->f80e30();
        }
        if (v4) e->f800d0();
    }
}

// 1.13c D2Client+0x814F0
EMSCRIPTEN_KEEPALIVE int32_t d2c_814f0(const D2cB69Env* e, uint32_t param) {   // param = entry ECX (0x81750 passes 1)
    uint8_t* u = *e->unit;
    if (!u) return 0;
    uint64_t* seed = (uint64_t*)(u + 0x20);
    int32_t h = e->ord10331(u);   // decompile shows no argument; same call as 0x81340 with the player unit
    if (!h) return 0;
    int32_t st = e->ord10826(h);
    if (e->ord10459(st) != 4) {
        *e->g_bf80 = 0;
        if (*e->g_bf78) { e->ord10063(); if (*(int32_t*)(e->g_bf68 + 0x114)) { e->fatal(0x15b); return 0; } }
        *e->g_bf78 = 0;
        e->f80be0();
        return 0;
    }
    e->f80be0();
    uint32_t chance = 0;
    int32_t ebx = 0;
    switch (st) {
    case 0x6d: case 0x6e: case 0x6f: case 0x70: *e->g_bf7c = 0; chance = 0x19; break;
    case 0x78: case 0x79: case 0x75: *e->g_bf7c = 1; ebx = 1; chance = 0; break;
    }
    uint64_t s = (uint64_t)*(uint32_t*)seed * 0x6ac690c5u + (uint64_t)*(uint32_t*)(u + 0x24);
    *seed = s;
    if ((uint32_t)(s & 0xffffffffu) % 100 < chance && param == 0) {   // the asm tests the stack copy of ECX (param) against 0
        *e->g_bf80 = 0;
        if (*e->g_bf78) { e->ord10063(); if (*(int32_t*)(e->g_bf68 + 0x114)) { e->fatal(0x15b); return 0; } }
        *e->g_bf78 = 0;
        return 0;
    }
    e->f812f0(ebx); e->f81470(param, 1);
    return 0;
}

// 1.13c D2Client+0x81750: ported to d2client_81750.cpp

// 1.13c D2Client+0x8F430
EMSCRIPTEN_KEEPALIVE void d2c_8f430(const D2cB69Env* e) {
    uint8_t* b = e->b_bbb23c;
    if (*e->g_9721 == 0) return;
    int32_t* p = d2c_b69_find(e->tbl_ba808, (uint32_t)*e->g_971d);
    if (!p) return;
    if (*p != 1) { e->fatal(0x5a); return; }
    enum { COMMON, NINE_A, OFF } k = COMMON;
    if (*e->g_97a8 == 0) {
        int32_t c = p[1];
        if (c < 0xfd) {
            if (c != 0xfc) switch (c) {
                case 0x93: case 0x94: case 0xb1: case 199: case 0xca: break;
                case 0x9a: case 0xb2: k = NINE_A; break;
                default: k = OFF;
            }
        } else if (c < 0x196) {
            if (c != 0x195) switch (c) {
                case 0xfd: case 0x101: k = NINE_A; break;
                case 0xfe: case 0xff: break;
                default: k = OFF;
            }
        } else if (c == 0x1ff) k = NINE_A;
        else if (c < 0x200 || c > 0x201) k = OFF;
        if (k == OFF) { *e->g_911c8 = 0; return; }
        if (k == NINE_A) {
            *e->g_bc50 = 3; e->f3ba30(); e->f3b9f0(); e->f3ba70(); e->f3bab0();
            *e->g_911c4 = 3; *e->g_911c8 = 4;
            d2c_b69_put<int32_t>(b, 0x00, 0); d2c_b69_put<int32_t>(b, 0x14, 0);
            d2c_b69_put<int32_t>(b, 0x28, 0); d2c_b69_put<int32_t>(b, 0x3c, 0);
            return;
        }
        *e->g_bc50 = 3; e->f3ba30(); e->f3b9f0(); e->f3b9c0();
    } else {
        *e->g_bc50 = 1; e->f3ba30(); e->f3b9f0();
        d2c_b69_put<uint16_t>(b, 0x2c, 0); d2c_b69_put<int32_t>(b, 0x2e, 1); d2c_b69_put<int32_t>(b, 0x34, 1);
        d2c_b69_put<uint16_t>(b, 0x32, 0); d2c_b69_put<int32_t>(b, 0x24, 0);
    }
    d2c_b69_put<uint8_t>(b, 0x46, 10); d2c_b69_put<int32_t>(b, 0x42, 9); d2c_b69_put<uint16_t>(b, 0x40, 0x1030);
    d2c_b69_put<int32_t>(b, 0x3c, 0); d2c_b69_put<int32_t>(b, 0x38, 1); d2c_b69_put<int32_t>(b, 0x28, 0);
    d2c_b69_put<int32_t>(b, 0x14, 0); d2c_b69_put<int32_t>(b, 0x00, 0);
    *e->g_911c8 = 4; *e->g_911c4 = 3;
}

// 1.13c D2Client+0x86A40   (pkt: +1 u8 slot, +2 u32 unit id, +6 i16 flag)
EMSCRIPTEN_KEEPALIVE void d2c_86a40(const D2cB69Env* e, const uint8_t* pkt) {
    uint8_t slot = pkt[1]; uint32_t id = *(const uint32_t*)(pkt + 2); int16_t fl = *(const int16_t*)(pkt + 6);
    auto setstate = [&](int32_t* unit, uint32_t s858, int32_t s864, int32_t s854) {
        *e->g_cb834 = (int32_t)(intptr_t)unit; *e->g_cb858 = (int32_t)s858; *e->g_cb864 = s864; *e->g_cb854 = s854;
    };
    if (fl == -1 && slot == 0xff) {
        int32_t* p = d2c_b69_find(e->tbl_bae08, id);
        if (p) { if (*p != 4) { e->fatal(0x5a); return; } e->ord10977(p, 4, 0); e->f31aa0(p); }
        setstate(nullptr, 0, 1, 5);
        return;
    }
    for (uint32_t i = 0; i < (uint32_t)*e->g_9e94; i++) {
        if (slot != e->g_9e98[i] || *e->g_aad84 != 0) continue;
        e->ord10227();
        if (*e->g_8d5d0 != 0x26) { e->fatal(0x139b); return; }
        if (*e->g_c3a68 == 0) d2c_b69_panel_body(e);
        if (fl != -1) break;
    }
    if (fl == -1) {
        int32_t* p = e->f54e20(id, e->tbl_bae08, 4);
        if (!p) return;
        e->ord10977(p, 4, 1);
        setstate(p, slot, 6, 6);
        return;
    }
    int32_t* p = d2c_b69_find(e->tbl_bae08, id);
    if (!p) return;
    if (*p != 4) { e->fatal(0x5a); return; }
    setstate(p, slot, 6, 6);
}
}
