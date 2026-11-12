// Storm FN probes: log the SMemFree (0x1B960) / SMemAlloc (0x1C580) stubs the way the capture's hook recorder does.
// The port's frees reach Storm_Sub1B960 (src/core/storm/storm_alloc_1b960.cpp); while a Scope is live the seam calls freeHook,
// which appends one hook record (rva, 5 regs 0, s0 = block, s1 = 0xB2000000|tag rva, s2 = line, s3 = 0) and frees nothing
// (the blocks are capture blobs). A block inside a blob is tagged 0xB1<blob><off>, as the capture does.
#pragma once
#include "hookenv.h"
#include <cstring>

extern "C" int32_t (*g_storm_free_hook)(void*, const char*, int32_t);
extern "C" void* (*g_storm_alloc_hook)(uint32_t, const char*, int32_t, uint32_t);
// weak: a tree without the 0x1C760 seam links (the hook is then never set)
extern "C" __attribute__((weak)) void* (*g_storm_realloc_hook)(void*, uint32_t, const char*, int32_t, uint32_t);

namespace stormhk {
// retail tag strings by rva (Storm .rdata); the port's spelling is matched on the core name
inline uint32_t tagRva(const char* s) {
    if (!s) return 0;
    static const struct { const char* k; uint32_t rva; } t[] = {
        { "delete", 0x46838 }, { "STRANS.CPP", 0x46A04 }, { "SEVT.CPP", 0x46A28 }, { "SCMD.CPP", 0x47130 }, { "SNET.CPP", 0x47AB0 }, { ".I", 0x51C80 }, { ".E", 0x51C74 }, { ".D", 0x51C68 }, { "AUTRANS", 0x515D0 }, { "AUWNDREC", 0x51B70 }, { "SCODE.CPP", 0x46908 }, { "SBMP.CPP", 0x46AB0 }, { "SFILE.CPP", 0x470F4 }, { "AUINST", 0x515A4 }, { "AUBREAKCMD", 0x51B58 },
        { "AU_FOUNDPARAM", 0x51BF4 }, { "AURECTF", 0x51C10 }, { "AU_SOURCE", 0x51C24 }, { "AURGN", 0x51C3C },
        { "AUAUDIOSTREAM", 0x51C8C }, { "AUCMDDEF", 0x51D0C }, { "AUVIDEOREC", 0x51D24 }, { "AUBITMAPREC", 0x51D54 }, { "AUSGDIOBJ", 0x51D74 }, { "AU_PLAYERNAME", 0x51DA4 }, { "AUMESSAGE", 0x51DF4 },
        { "AV_TSHashObjectChunk_URGN_VH", 0x51B88 },
        { "AUUSEREVENT", 0x51D8C }, { "AUCONNREC", 0x51DC0 }, { "AUPROVIDERINFO", 0x51DD8 },
        { "AUFILEREC", 0x51CC0 }, { "AUEVENTREC", 0x51CA8 }, { "AUREQUEST", 0x51CD8 }, { "AUARCHIVEREC", 0x51CF0 },
        { "AUSTREAM", 0x515B8 }, { "SBLT.CPP", 0x469D8 }, { "SCOMP.CPP", 0x474A8 },
    };
    for (const auto& e : t) {
        const char* p = strstr(s, e.k);
        if (!p) continue;
        const char c = p[strlen(e.k)];
        if (c == 0 || c == '_' || c == '@' || c == '\\' || c == '"') return e.rva;
    }
    return 0xFFFFFF;
}
struct Scope {
    HookEnv e; uint32_t* r; int nb;
    static Scope*& cur() { static Scope* s = nullptr; return s; }
    // r: the probe's register file; logBlob: index of the `hooklog` blob; nb: number of blobs in the tuple
    Scope(uint32_t* r_, int logBlob, int nb_, const uint32_t* sc) : e((uint32_t*)(uintptr_t)r_[14 + 2 * logBlob], sc), r(r_), nb(nb_) {
        cur() = this; g_storm_free_hook = freeHook; g_storm_alloc_hook = allocHook; if (&g_storm_realloc_hook) g_storm_realloc_hook = reallocHook;
    }
    ~Scope() { cur() = nullptr; g_storm_free_hook = nullptr; g_storm_alloc_hook = nullptr; if (&g_storm_realloc_hook) g_storm_realloc_hook = nullptr; }
    uint32_t tr(uint32_t v) const {
        for (int k = 0; k < nb; k++) { uint32_t b = r[14 + 2 * k]; if (v >= b && v < b + r[15 + 2 * k]) return 0xB1000000u | (uint32_t)(k << 16) | (v - b); }
        return v;
    }
    static int32_t freeHook(void* p, const char* f, int32_t line) {
        Scope* s = cur();
        return (int32_t)s->e.h(0x1B960, 0, 0, 0, 0, 0, s->tr((uint32_t)(uintptr_t)p), 0xB2000000u | tagRva(f), (uint32_t)line, 0);
    }
    static void* reallocHook(void* p, uint32_t n, const char* f, int32_t line, uint32_t) {   // 0x1C760: s0 block, s1 size, s2 tag, s3 line
        Scope* s = cur();
        return (void*)(uintptr_t)s->e.h(0x1C760, 0, 0, 0, 0, 0, s->tr((uint32_t)(uintptr_t)p), n, 0xB2000000u | tagRva(f), (uint32_t)line);
    }
    static void* allocHook(uint32_t n, const char* f, int32_t line, uint32_t fl) {
        Scope* s = cur();
        return (void*)(uintptr_t)s->e.h(0x1C580, 0, 0, 0, 0, 0, n, 0xB2000000u | tagRva(f), (uint32_t)line, fl);
    }
};
}  // namespace stormhk
