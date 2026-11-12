// src/core/d2client/d2client_batch_88930.cpp -- 12 UNMAPPED 1.13c D2Client.dll rows (0x88930..0x8A540, the sound/
// record list walk, tick RNG and the 4-table lighting builders) reimplemented from the cached decompiles.
// Alpha batch lane: implementation + marker only, no spec, no vector. Base 0x6FAB0000.
// Unrecovered callees are host seams (the d2client_batch_5d110.cpp pattern).
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <emscripten.h>
#include "d2c88_seam.h"

template <class T> static T rd(const uint8_t* p, size_t o) { T v; memcpy(&v, p + o, sizeof v); return v; }
template <class T> static void wr(uint8_t* p, size_t o, T v) { memcpy(p + o, &v, sizeof v); }
static uint8_t* rec(const D2c88State* s, int32_t i) {
    return (!s->recs || i < 0 || i >= s->count) ? nullptr : s->recs + (size_t)i * 0x8E;
}
static uint8_t R(const D2c88State* s, int32_t i, size_t o) { uint8_t* r = rec(s, i); return r ? r[o] : 0; }
static uint8_t* find(D2c88State* s, uint8_t* key, void*** nodeOut = nullptr) {
    for (void** n = s->list; n; n = (void**)n[1])
        if (rd<uint32_t>((uint8_t*)n[0], 8) == (uint32_t)(uintptr_t)key) { if (nodeOut) *nodeOut = n; return (uint8_t*)n[0]; }
    return nullptr;
}

// 1.13c D2Client+0x88930
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_88930(D2c88State* s, const D2c88Env* e, int32_t id) {
    for (void** n = s->list; n; n = (void**)n[1]) {
        uint8_t* o = (uint8_t*)n[0];
        if (rd<int32_t>(o, 4) != id) continue;
        wr<uint32_t>(o, 0x3d, 1);
        uint8_t b = R(s, id, 0x40);
        if (!b || rd<int32_t>(o, 0x30) != 1) continue;
        uint8_t* key = rd<uint8_t*>(o, 8);
        if (!key) continue;
        for (void** m = s->list; m; m = (void**)m[1]) {
            uint8_t* p = (uint8_t*)m[0];
            if (rd<uint8_t*>(p, 8) != key) continue;
            if (!e->fn87e90(o, (int)(intptr_t)key)) {
                uint32_t u = b, o40 = R(s, rd<int32_t>(p, 4), 0x40);
                if (u < o40) u = o40;
                if (!rd<int32_t>(p, 0x45) || rd<int32_t>(p, 0x4d) || s->now + u < rd<uint32_t>(p, 0x55)) {
                    wr<uint32_t>(p, 0x3d, 1);
                    if (!u) e->fn87ed0((int)(intptr_t)key, 0);   // EDX still holds the key (0x87E90 leaves it alone)
                    else {
                        wr<uint32_t>(p, 0x51, s->now);
                        wr<uint32_t>(p, 0x49, rd<uint32_t>(p, 0x1c));
                        wr<uint32_t>(p, 0x4d, 0);
                        wr<uint32_t>(p, 0x55, s->now + u);
                        wr<uint32_t>(p, 0x45, 1);
                    }
                }
            }
            break;
        }
    }
}

// 1.13c D2Client+0x88BA0 (this = key)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_88ba0(D2c88State* s, const D2c88Env* e, uint8_t* key, int32_t x, int32_t y, int32_t z) {
    if (!key) return;
    for (void** n = s->list; n; n = (void**)n[1])
        if (rd<uint8_t*>((uint8_t*)n[0], 8) == key) {
            float v[3] = {(float)x, (float)y, (float)((double)z + (double)s->biasZ)};   // fild z; fadd bias; fstp: one rounding to float
            e->fn88350(v, n[0]);
            return;
        }
}

// 1.13c D2Client+0x88BF0 (key in EDX)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_88bf0(D2c88State* s, const D2c88Env* e, uint8_t* key) {
    if (!key) return;
    if (find(s, key)) e->fn88550(find(s, key));
}

// 1.13c D2Client+0x88C20 (key in EAX)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_88c20(D2c88State* s, const D2c88Env* e, uint8_t* key, int32_t p1, int32_t p2) {
    if (!key || !p1) e->fatal(0xCE);
    e->ord10062((uint32_t)p1 + 0x78, (uint32_t)(uintptr_t)key);
    uint8_t* o = find(s, key);
    if (!o) return;
    e->ord10062((uint32_t)(uintptr_t)(o + 0x28), (uint32_t)p1);
    bool w = !R(s, rd<int32_t>(o, 4), 0x3e) || (rd<int32_t>(o, 0x45) && !rd<int32_t>(o, 0x4d));
    if (rd<int32_t>(o, 0x28)) return;
    if (!p2 && w) return;
    e->fn88550(o);
}

// 1.13c D2Client+0x88D90 (n arrives in ESI)
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_client_88d90(D2c88State* s, const D2c88Env* e, int32_t n);

// 1.13c D2Client+0x89640 (n in ESI)
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_client_89640(D2c88State* s, const D2c88Env*, int32_t n) {
    if (n < 1) return 0;
    uint64_t v = (uint64_t)rd<uint32_t>(s->rngBase, 0x20) * 0x6ac690c5u + rd<uint32_t>(s->rngBase, 0x24);
    wr<uint64_t>(s->rngBase, 0x20, v);
    uint32_t u = (uint32_t)n;
    return (u & (u - 1)) ? (uint32_t)(v & 0xffffffffu) % u : (u - 1) & (uint32_t)v;
}

extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_client_88d90(D2c88State* s, const D2c88Env* e, int32_t n) {
    uint32_t r = s->now / 3;
    if (s->now % 3 == 0) {
        d2_client_89640(s, e, n);
        r = e->fn88a70(0, 0, 0, 0);
    }
    return r;
}

// 1.13c D2Client+0x895D0 (a in EAX, b in ECX, mod in EDI): circular distance
extern "C" EMSCRIPTEN_KEEPALIVE int32_t d2_client_895d0(uint32_t a, uint32_t b, uint32_t m) {
    uint32_t x, y;
    if (!(m & (m - 1))) { x = a & (m - 1); y = b & (m - 1); }
    else { x = (uint32_t)((int32_t)a % (int32_t)m); y = (uint32_t)((int32_t)b % (int32_t)m); }
    int32_t d = (int32_t)(y - x), ad = d < 0 ? -d : d;
    int32_t e1 = (int32_t)(d - m); if (e1 < 0) e1 = -e1;
    int32_t e2 = (int32_t)((x - y) - m); if (e2 < 0) e2 = -e2;
    int32_t mn = ad; if (e1 <= ad) mn = e1;
    if (mn < e2) { e2 = ad; if (e1 <= ad) e2 = e1; }
    return e2;
}

// 1.13c D2Client+0x89F00 (p in ESI; args 1..3 on stack/ecx)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_89f00(D2c88State* s, const D2c88Env* e, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t esi) {
    if (!esi) return;
    s->tick = e->tickCount();
    s->a = p2; s->c = p3; s->hook = p1; s->b = esi;
}

// 1.13c D2Client+0x8A140
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_8a140(D2c88State* s, const D2c88Env* e) {
    uint32_t* h[2] = {&s->h1c, &s->h20};
    for (auto* p : h) {
        if (!*p) continue;
        uint32_t v = *p;
        e->ord10065(v);
        e->ord10043(0x7a, 0);
        *p = 0;
    }
}

// 1.13c D2Client+0x8A1C0 (returns table of n*n*m pointers into s->table)
extern "C" EMSCRIPTEN_KEEPALIVE void** d2_client_8a1c0(D2c88State* s, const D2c88Env* e, uint32_t n, int32_t m) {
    void** base = (void**)e->valloc((size_t)n * n * m * 4);
    void** p = base;
    int32_t c = 0;
    for (uint32_t i = 0; i < n; i++, c -= 0x10000) {
        int32_t d = c;
        for (uint32_t k = n; k; k--, d += 0x10000) {
            int32_t v = (int32_t)(i << 16);
            for (int32_t j = m; j; j--) {
                *p++ = (uint8_t*)s->table + (uint32_t)(((v + ((v >> 31) & 0xffff)) >> 16) & 0xff) * 0x100;
                v += d / m;
            }
        }
    }
    return base;
}

// 1.13c D2Client+0x8A2D0 (as 8A1C0 with one extra trailing entry per row)
extern "C" EMSCRIPTEN_KEEPALIVE void** d2_client_8a2d0(D2c88State* s, const D2c88Env* e, uint32_t n, int32_t m) {
    void** base = (void**)e->valloc((size_t)(m + 1) * n * n * 4);
    void** p = base;
    uint32_t u = n & 0xff;
    int32_t c = 0;
    for (uint32_t i = 0; i < n; i++, c -= 0x10000) {
        int32_t d = c;
        for (uint32_t k = n; k; k--, d += 0x10000) {
            int32_t v = (int32_t)(i << 16);
            for (int32_t j = m; j; j--) {
                u = (uint32_t)((v + ((v >> 31) & 0xffff)) >> 16);
                *p++ = (uint8_t*)s->table + (u & 0xff) * 0x100;
                v += d / m;
            }
            *p++ = (uint8_t*)s->table + (u & 0xff) * 0x100;
        }
    }
    return base;
}

// 1.13c D2Client+0x8A540
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_8a540(D2c88State* s, const D2c88Env* e) {
    if (!s->f04) return;
    if (!s->f08) e->fatal(0xFB);
    else if (!s->f0c) e->fatal(0xFC);
    else if (!s->f14) e->fatal(0xFD);
    else if (!s->f18) e->fatal(0xFE);
    e->fn8a2b0(); e->fn8a3d0(); e->fn8a3d0(); e->fn8a1a0(); e->fn8a3d0();
    s->f04 = s->f08 = s->f0c = s->f14 = s->f18 = 0;
    s->eae8 = 0;
    d2_client_8a140(s, e);
}
