// 1.13c D2Client+0x7DE50 packet handler (EAX = packet; caller AD680). Pointer-bearing
// fields use native stride (node next @0xe4, node string @0xa4); globals and callees are passed in.
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

namespace {
template<class T> T ld(const void* p, size_t o) { T v; std::memcpy(&v, (const uint8_t*)p + o, sizeof v); return v; }
}

struct D2C7DE50 {
    void* const* nodes_10a808;   // 128 buckets, type 1
    void* const* nodes_10aa08;   // table passed to 54E20 for type 2
    void** obj_113b10;           // DAT_6fbc3b10
    uint16_t* w_113af4;          // DAT_6fbc3af4
    uint32_t* d_113ac6;          // DAT_6fbc3ac6
    uint32_t d_113ace;           // DAT_6fbc3ace (value)
    void* (*find_54e20)(uint32_t key, void* const* tbl, int type);
    void (*free_10660)(void*);
    void* (*new_10204)(int);
    void (*copy_10485)(void*, const void*);
    void (*fn_10151)(void*);
    void (*del_10422)(int, void*);
    void* (*str_10454)(int, const char*, uint32_t);
    void (*s_7d200)(int16_t, int);
    void (*s_7c8f0)(uint16_t);   // retail: ret 4, the zero-extended u16 @+10
    void (*fail)(void);          // 10265(0x5a)+10024 then _exit(-1)
};

extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_7de50(const uint8_t* pkt, D2C7DE50* c) {
    const uint8_t kind = pkt[1];
    if (kind != 1) {
        if (kind != 2) { c->free_10660(*c->obj_113b10); *c->obj_113b10 = nullptr; return; }
        const uint32_t id = ld<uint32_t>(pkt, 2);
        const uint8_t* n = (const uint8_t*)c->find_54e20(id, c->nodes_10aa08, 2);
        if (pkt[8] == 3) { c->s_7c8f0(ld<uint16_t>(pkt, 10)); return; }
        *c->w_113af4 = 0;
        *c->d_113ac6 = id;
        if (n) *c->w_113af4 = (uint16_t)ld<uint32_t>(n, 4);
        c->s_7d200(ld<int16_t>(pkt, 10), 1);
        return;
    }
    const uint32_t id = ld<uint32_t>(pkt, 2);
    for (void* n = c->nodes_10a808[id & 0x7f]; n; n = ld<void*>(n, 0xe4)) {
        if (ld<uint32_t>(n, 0xc) != id) continue;
        if (ld<uint32_t>(n, 0) != 1) { c->fail(); std::abort(); }
        if (pkt[6] == 1 && pkt[8] == 3) {
            void* old = ld<void*>(n, 0xa4);
            if (old) c->del_10422(0, old);
            char buf[20];
            std::snprintf(buf, sizeof buf, "%d", (unsigned)ld<uint16_t>(pkt, 10));
            void* s = c->str_10454(0, buf, c->d_113ace);
            std::memcpy((uint8_t*)n + 0xa4, &s, sizeof s);
            return;
        }
        break;
    }
    if (*c->obj_113b10) c->free_10660(*c->obj_113b10);
    *c->obj_113b10 = c->new_10204(0);
    c->copy_10485(*c->obj_113b10, pkt + 6);
    c->fn_10151(*c->obj_113b10);
}
