// src/core/d2client/d2client_batch_9dcc0.cpp -- 12 UNMAPPED 1.13c D2Client.dll rows (0x9DCC0..0x9EDD0, the
// per-unit party/hostile flag lookups and senders, the PE-image loader) from the cached decompiles.
// Alpha batch lane: implementation + marker only, no spec, no vector. Base 0x6FAB0000.
// Unrecovered callees are host seams (the d2client_batch_5d110.cpp pattern). Guest pointers are native-width.
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <emscripten.h>

template <class T> static T rd(const void* p, size_t o) { T v; memcpy(&v, (const uint8_t*)p + o, sizeof v); return v; }
template <class T> static void wr(void* p, size_t o, T v) { memcpy((uint8_t*)p + o, &v, sizeof v); }

struct D2c9dNode { int32_t id; int32_t flags; D2c9dNode* next; };  // list node: +0 id, +4 flags, +8 next

struct D2c9dEnv {
    void*   root;                                       // *(+0x10BBFC) DAT_6fbcbbfc; id at +0xc
    uint8_t* (*fn9d630)(void* root, int32_t id);        // entry for id; +0x34 -> D2c9dNode** head
    void    (*fnffe20)(uint32_t ecx, int32_t unitId, int32_t own);   // EAX=unit id, EDX=own, ECX passes through
    uint32_t (*fnffd90)(int32_t unitId, int32_t own, uint32_t mask);   // ESI=unit id, EDX=own, stack=mask
    void    (*fnffdd0)(uint32_t mask, int32_t unitId, int32_t own, uint32_t set);   // EAX=mask, ESI=unit id, EDX=own, stack=set
    void    (*fn143e0)(const uint8_t* pkt);             // send packet
    uint8_t* (*fn9d990)(void);
    void    (*fn14850)(int32_t id, int32_t a, uint32_t b);
    void    (*fn9da70)(void);
    void    (*fn772e0)(void);
    void    (*fatal)(uint32_t code);                    // Ordinal 10265+10024+_exit(-1)
    void    (*fn9dcc0)(const void* unit, uint32_t a, uint32_t bx);  // optional seam for the 9DCC0 call made by the senders (null: the port's own)
};

static int32_t rootId(const D2c9dEnv* e) { return e->root ? rd<int32_t>(e->root, 0xc) : -1; }

// Shared walk: the unit's node in the root entry's list. False when 9D630 finds no entry, the entry has no list
// header, the player id equals the unit id, or no node carries the unit id (a node's flag word may be any 32-bit value).
static bool flagsOf(const D2c9dEnv* e, int32_t unitId, int32_t pid, int32_t* out) {
    uint8_t* ent = e->fn9d630(e->root, pid);
    if (ent && pid != unitId) {
        D2c9dNode** head = rd<D2c9dNode**>(ent, 0x34);
        if (head)
            for (D2c9dNode* n = *head; n; n = n->next)
                if (n->id == unitId) { *out = n->flags; return true; }
    }
    return false;
}
static uint32_t flagMask(const D2c9dEnv* e, const void* unit, uint32_t m) {
    if (!unit) return 0;
    int32_t f;
    return flagsOf(e, rd<int32_t>(unit, 0x10), rootId(e), &f) ? ((uint32_t)f & m) : 0;
}

// 1.13c D2Client+0x9DCC0
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_9dcc0(const D2c9dEnv* e, const void* unit, uint32_t a, uint32_t bx) {
    int32_t id = rootId(e);
    if (!unit) e->fatal(0x460);
    int32_t uid = rd<int32_t>(unit, 0x10);
    e->fnffe20(a, uid, id);
    uint32_t r = e->fnffd90(uid, id, bx & 0xffff);
    e->fnffdd0(bx & 0xffff, uid, id, r == 0);
}

// 1.13c D2Client+0x9DD20
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_client_9dd20(const D2c9dEnv* e, const void* unit) {
    if (!unit) return 0;
    return flagMask(e, unit, 8) ? 1 : 0;
}
// 1.13c D2Client+0x9DD80
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_client_9dd80(const D2c9dEnv* e, const void* unit) { return flagMask(e, unit, 4); }
// 1.13c D2Client+0x9DDD0
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_client_9ddd0(const D2c9dEnv* e, const void* unit) { return flagMask(e, unit, 2); }
// 1.13c D2Client+0x9DE20
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_client_9de20(const D2c9dEnv* e, const void* unit) { return flagMask(e, unit, 1); }

static void sendFlag(const D2c9dEnv* e, const void* unit, uint32_t a, uint8_t sub, uint32_t m) {
    if (!unit) return;
    // retail loads EBX := m (the mask) before calling 9DCC0; the caller's EBX is not an input
    if (e->fn9dcc0) e->fn9dcc0(unit, a, m); else d2_client_9dcc0(e, unit, a, m);
    int32_t id = rd<int32_t>(unit, 0x10);
    uint8_t pkt[7] = { 0x5d, sub, (uint8_t)(flagMask(e, unit, m) != 0) };
    memcpy(pkt + 3, &id, 4);
    e->fn143e0(pkt);
}
// 1.13c D2Client+0x9DE90
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_9de90(const D2c9dEnv* e, const void* unit, uint32_t a, uint32_t bx) { (void)bx; sendFlag(e, unit, a, 3, 4); }

// 1.13c D2Client+0x9E520: EDI = unit (record with +0x10 id, +0x20 and +0x22 words), ECX = the context word handed to 0x9D630, EBX/ESI are saved and restored.
// Callees (retail registers and stack words): 0x9D630 (ECX, EDX = root id) entry, 0x9DCC0 (EAX = unit, EBX = 8), 0x4FD90 (EDX = root id, [esp] = 8, ret 4),
// 0x14850 (EDX = unit id, [esp] = 4, [esp+4] = value, ret 8), 0x9D990 (), and the tail jump to 0x9E120. The return value is EAX at each exit.
struct D2c9e520Env {
    uint8_t* (*f9d630)(uint32_t ecx, int32_t edx);
    void (*f9dcc0)(const uint8_t* unit, uint32_t ebx);
    uint32_t (*f4fd90)(int32_t edx, uint32_t s0);
    void (*f14850)(int32_t edx, uint32_t s0, uint32_t s1);
    uint8_t* (*f9d990)(void);
    uint32_t (*f9e120)(void);
};
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_client_9e520(const D2c9e520Env* e, const uint8_t* unit, uint32_t ecx, const uint8_t* root) {
    const int32_t rid = root ? rd<int32_t>(root, 0xc) : -1;
    uint32_t ret = (uint32_t)(uintptr_t)root;
    if (!unit) return ret;
    const int32_t uid = rd<int32_t>(unit, 0x10);
    uint8_t* ent = e->f9d630(ecx, rid);
    ret = 0;
    bool call = false;
    if (ent) {
        const uint8_t* head = rd<const uint8_t*>(ent, 0x34);
        ret = (uint32_t)(uintptr_t)head;
        if (head && rid != uid) {
            const uint8_t* n = rd<const uint8_t*>(head, 0);
            ret = 0;
            for (; n; n = rd<const uint8_t*>(n, 8))
                if (rd<int32_t>(n, 0) == uid) { ret = rd<uint32_t>(n, 4) & 8; call = ret != 0; break; }
        }
    }
    if (!call && rd<uint16_t>(unit, 0x20) < 9) return ret;
    e->f9dcc0(unit, 8);
    ret = e->f4fd90(rid, 8);
    e->f14850(rd<int32_t>(unit, 0x10), 4, ret);
    ret = e->f4fd90(rid, 8);
    if (!ret || rd<uint16_t>(unit, 0x22) == 0xffff) return ret;
    const uint8_t* o = e->f9d990();
    const uint32_t w = rd<uint16_t>(o, 0x22);
    if ((uint16_t)w != rd<uint16_t>(unit, 0x22)) return ((uint32_t)(uintptr_t)o & 0xffff0000u) | w;
    return e->f9e120();
}

// ---- PE image loader (9E630 / 9E690). Win32 calls are seams. ----
struct D2c9eImg { uint8_t* base; uint32_t size; int32_t nLibs; };  // ESI object: +0 base, +4 size, +8 loaded-lib count
struct D2c9eEnv {
    void* (*getModule)(const char* name);
    void  (*freeLibrary)(void* h);
    void  (*virtualFree)(void* p, size_t n, uint32_t type);
    void* (*valloc)(size_t n);                          // VirtualAlloc(0,n,MEM_COMMIT,PAGE_READWRITE)
    void* (*loadLibrary)(const char* name);
    void* (*getProc)(void* h, const char* nameOrOrd);
    void  (*protect)(void* p, size_t n, uint32_t prot);
    void  (*flushIcache)(void* p, size_t n);
};

// 1.13c D2Client+0x9E630
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_9e630(const D2c9eEnv* e, D2c9eImg* im) {
    uint8_t* b = im->base;
    if (!b) return;
    uint32_t imp = rd<uint32_t>(b, 0x1c);
    while (im->nLibs > 0) {
        int32_t i = im->nLibs--;
        e->freeLibrary(e->getModule((const char*)(b + rd<uint32_t>(b, imp + (i - 1) * 8))));
    }
    e->virtualFree(b, 0, 0x8000);
    im->base = nullptr;
    im->size = 0;
}

// 1.13c D2Client+0x9E690  (this = packed image; returns true on success)
extern "C" EMSCRIPTEN_KEEPALIVE bool d2_client_9e690(const D2c9eEnv* e, const uint8_t* src, D2c9eImg* im) {
    bool fail = true;
    uint32_t total = rd<uint32_t>(src, 0);
    im->size = total;
    uint8_t* b = (uint8_t*)e->valloc(total);
    im->base = b;
    if (b) {
        memcpy(b, src, 40);
        const uint8_t* sec = src + 0x28;
        uint32_t nsec = rd<uint32_t>(b, 0x24);
        const uint8_t* rl = sec + nsec * 12;                       // run-length stream
        uint8_t* dst = b + rd<uint32_t>(sec, 0);
        uint8_t* end = b + rd<uint32_t>(b, 0);
        bool copy = true;
        while (dst < end) {
            uint16_t n; memcpy(&n, rl, 2); rl += 2;
            if (copy) { memcpy(dst, rl, n); rl += n; }
            dst += n;
            if (dst >= end) break;
            copy = !copy;
        }
        // relocations: delta-coded offsets
        const uint8_t* rp = b + rd<uint32_t>(b, 8);
        int32_t off = 0;
        for (uint32_t i = 0; i < rd<uint32_t>(b, 0xc); i++) {
            if (rp[0] & 0x80) {
                off = (int32_t)((((rp[0] & 0x7fu) * 0x100 + rp[1]) * 0x100 + rp[2]) * 0x100 + rp[3]);
                rp += 4;
            } else {
                off = (int32_t)(rp[1] + off + rp[0] * 0x100u);
                rp += 2;
            }
            // ponytail: 32-bit fixup (b is a 32-bit image base on the original); wasm64 host must keep b < 4 GiB.
            wr<uint32_t>(b, off, rd<uint32_t>(b, off) + (uint32_t)(uintptr_t)b);
        }
        // imports
        const uint8_t* imp = b + rd<uint32_t>(b, 0x1c);
        bool ok = true;
        for (uint32_t i = 0; i < rd<uint32_t>(b, 0x20) && ok; i++) {
            void* h = e->loadLibrary((const char*)(b + rd<uint32_t>(imp, i * 8)));
            if (!h) { ok = false; break; }
            uint8_t* t = b + rd<uint32_t>(imp, i * 8 + 4);
            for (uint32_t v; (v = rd<uint32_t>(t, 0)) != 0; t += 4) {
                const char* nm = (int32_t)v < 0 ? (const char*)(uintptr_t)(v & 0x7fffffff) : (const char*)(b + v);
                wr<uint32_t>(t, 0, (uint32_t)(uintptr_t)e->getProc(h, nm));
            }
            im->nLibs++;
        }
        if (ok) {
            for (uint32_t i = 0; i < rd<uint32_t>(b, 0x24); i++) {
                const uint8_t* s = src + 0x28 + i * 12;
                uint8_t* p = b + rd<uint32_t>(s, 0);
                size_t n = rd<uint32_t>(s, 4);
                uint32_t prot = rd<uint32_t>(s, 8);
                e->protect(p, n, prot);
                if (prot & 0xf0) e->flushIcache(p, n);
            }
            uint32_t sz = rd<uint32_t>(b, 0), used = rd<uint32_t>(b, 8);
            if (used < sz) {
                uint32_t al = (used + 0xfff) & 0xfffff000u;
                if (used <= al && al < sz) e->virtualFree(b + al, sz - al, 0x4000);
            }
            fail = false;
        }
    }
    if (fail) d2_client_9e630(e, im);
    return !fail;
}

