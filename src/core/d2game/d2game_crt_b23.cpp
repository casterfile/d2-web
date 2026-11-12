// src/core/d2game/d2game_crt_b23.cpp -- seven UNMAPPED 1.13c D2Game.dll rows reimplemented from the cached
// decompiles (build-wasm/ghidra-d2game/decomp/0x<RVA>.c), in RVA order: three __unlock tail thunks, the
// TLS-slot allocation, __mtterm, __initptd, __mtinit. MSVC 2003 (VC7.1) Release CRT glue the linker
// pulled into the DLL. Alpha batch lane (OBJECTIVE v8.3): implementation + marker
// only, no spec, no vector.
//
// Prior art: src/core/gfx/d2ddraw_batch_10da.cpp (the same CRT trio at 0x1B85/0x1B8E/0x1BAB),
// src/core/d2game/d2game_batch_3e70.cpp (D2Game's own 0x3F76 __unlock, declared here and called) and
// src/core/d2client/d2client_batch_74de.cpp (the same 239-byte __mtinit body).
#include <cstdint>
#include <emscripten.h>

// Defined in src/core/d2game/d2game_batch_3e70.cpp.
extern "C" void d2game_unlock_3f76(int32_t lockNum);
extern "C" int32_t d2_game_3ed8(void);         // 0x3ED8 __mtinitlocks

// __mtterm/__mtinit state: +0x0FF204 the TLS index (0xFFFFFFFF = TLS_OUT_OF_INDEXES, not allocated),
// +0x10FD88..94 the four Fls*/Tls* entry slots, +0x10FD94 also the TlsFree/FlsFree slot. All zero in
// the shipped image. Teardown uses the shared lock table in d2game_batch_3e70.cpp.
static uint32_t g_tls_index = 0xFFFFFFFFu;   // +0x0FF204 DAT_6fd1f204
static void (*g_tls_free)(uint32_t);         // +0x10FD94 DAT_6fd2fd94 = FlsFree/TlsFree
extern "C" void d2_fn_d2game_3F21(void);


typedef uint32_t (*D2GameFlsAllocFn)(void (*callback)(void*));
typedef int32_t  (*D2GameFlsGetFn)(uint32_t index);
typedef int32_t  (*D2GameFlsSetFn)(uint32_t index, void* value);
typedef uint32_t (*D2GameFlsFreeFn)(uint32_t index);
static D2GameFlsAllocFn g_fls_alloc;   // +0x10FD88 DAT_6fd2fd88 FlsAlloc (0x3B5A TlsAlloc thunk fallback)
static D2GameFlsGetFn   g_fls_get;     // +0x10FD8C DAT_6fd2fd8c FlsGetValue (TlsGetValue fallback)
static D2GameFlsSetFn   g_fls_set;     // +0x10FD90 DAT_6fd2fd90 FlsSetValue (TlsSetValue fallback)

// Host seams: the kernel32 entry points, the Tls* fallbacks, the 0x3C04 FLS destructor (UNMAPPED row),
// __calloc_crt 0x6018 (src/core/d2game/d2game_crt_calloc.cpp, called with its own env there) and
// GetCurrentThreadId.
struct D2GameMtInitHost {
    void*    (*get_module_handle)(const char* name);            // GetModuleHandleA
    void*    (*get_proc_address)(void* module, const char* name); // GetProcAddress
    D2GameFlsGetFn   tls_get;                                   // TlsGetValue import
    D2GameFlsSetFn   tls_set;                                   // TlsSetValue import
    D2GameFlsAllocFn tls_alloc;                                 // 0x3B5A TlsAlloc thunk
    D2GameFlsFreeFn  tls_free;                                  // TlsFree import
    void     (*freefls)(void* ptd);                             // 0x3C04 __freefls@4, the FLS destructor
    void*    (*calloc_)(uint32_t count, uint32_t size);         // 0x6018 __calloc_crt
    uint32_t (*get_current_thread_id)(void);                    // GetCurrentThreadId
};

// 1.13c D2Game+0x3B51: tail fragment into RVA 0x3F76 with lock number 0xC -- __unlock(12). Reached from
// __updatetlocinfo (0x3B1F) when the locale it just installed is a new one.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_3b51(uint32_t* r) { d2game_unlock_3f76(0xC); if (r) r[4] = 0; }  // r: FN probe, result word 0

// 1.13c D2Game+0x3B5A: TlsAlloc(), result discarded. wasm has no TLS indices, and the index would leak
// either way (retail never stores the result), so the rule is an empty statement.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_3b5a(void) { }

// 1.13c D2Game+0x3B63 (__mtterm): if the TLS index is allocated, free it and reset the slot to -1; then
// __mtdeletelocks(). Returns nothing.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_3b63(void) {
    if (g_tls_index != 0xFFFFFFFFu) {
        if (g_tls_free) g_tls_free(g_tls_index);
        g_tls_index = 0xFFFFFFFFu;
    }
    d2_fn_d2game_3F21();
}

// 1.13c D2Game+0x3B80 (__initptd): seed a thread-data block -- _initaddr +0x54 = the retail VA
// 0x6FD1F760 (stored as its RVA 0xFF760, the port's convention for retail VAs) and _holdrand +0x14 = 1.
// The locale argument (_Locale) is not touched. Returns nothing.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_3b80(uint8_t* ptd, void* locale) {
    (void)locale;
    *(uint32_t*)(ptd + 0x54) = 0xFF760u;   // _initaddr, retail 0x6FD1F760
    *(uint32_t*)(ptd + 0x14) = 1;          // _holdrand
}

// 1.13c D2Game+0x3D36: tail fragment into RVA 0x3F76 with lock number 0xD -- __unlock(13). One of the
// two releases in __freefls (0x3C04).
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_3d36(void) { d2game_unlock_3f76(0xD); }

// 1.13c D2Game+0x3D42: tail fragment into RVA 0x3F76 with lock number 0xC -- __unlock(12). The other
// release in __freefls (0x3C04).
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_3d42(void) { d2game_unlock_3f76(0xC); }

// 1.13c D2Game+0x3D7A (239 B, caller 0x35BB __CRT_INIT): `__mtinit`. Initialise the CRT lock table
// (0x3ED8; failure -> __mtterm, return 0). With a kernel32 handle, resolve FlsAlloc/FlsGetValue/
// FlsSetValue/FlsFree into the four slots; when FlsGetValue is absent, replace all four with the Tls*
// fallbacks (the 0x3B5A thunk in the alloc slot). Allocate the FLS key with the resolved allocator and
// the 0x3C04 destructor; on failure -> __mtterm, return 0. Then calloc one 0x8C-byte _tiddata, bind it
// to the key and, when the bind succeeds, seed it: +0x54 = the CRT default block's RVA, +0x14
// (_holdrand) = 1, +0x04 (_thandle) = -1, +0x00 (_tid) = the current thread id. Return 1; otherwise
// __mtterm and return 0.
extern "C"
EMSCRIPTEN_KEEPALIVE int32_t d2_game_3d7a(const D2GameMtInitHost* host) {
    if (d2_game_3ed8() == 0) {
        d2_fn_d2game_3b63();
        return 0;
    }
    // Retail jumps through a NULL alloc slot when kernel32 is absent; unreachable in the shipped image.
    void* kernel32 = host->get_module_handle("kernel32.dll");
    if (kernel32 != nullptr) {
        g_fls_alloc = (D2GameFlsAllocFn)host->get_proc_address(kernel32, "FlsAlloc");
        g_fls_get   = (D2GameFlsGetFn)host->get_proc_address(kernel32, "FlsGetValue");
        g_fls_set   = (D2GameFlsSetFn)host->get_proc_address(kernel32, "FlsSetValue");
        g_tls_free  = (void (*)(uint32_t))host->get_proc_address(kernel32, "FlsFree");
        if (g_fls_get == nullptr) {
            g_fls_get   = host->tls_get;
            g_fls_set   = host->tls_set;
            g_fls_alloc = host->tls_alloc;
            g_tls_free  = (void (*)(uint32_t))host->tls_free;
        }
    }
    g_tls_index = g_fls_alloc(host->freefls);
    if (g_tls_index != 0xFFFFFFFFu) {
        uint32_t* ptd = (uint32_t*)host->calloc_(1, 0x8C);
        if (ptd != nullptr && g_fls_set(g_tls_index, ptd) != 0) {
            ptd[0x15] = 0xFF760u;                // +0x54 _initaddr, retail 0x6FD1F760
            ptd[5] = 1u;                         // +0x14 _holdrand
            uint32_t tid = host->get_current_thread_id();
            ptd[1] = 0xFFFFFFFFu;                // +0x04 _thandle
            ptd[0] = tid;                        // +0x00 _tid
            return 1;
        }
    }
    d2_fn_d2game_3b63();
    return 0;
}
