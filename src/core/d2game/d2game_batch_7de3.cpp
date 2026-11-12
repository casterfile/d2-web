// src/core/d2game/d2game_batch_7de3.cpp -- twelve 1.13c D2Game.dll rows transcribed from the cached
// Ghidra decompiles (build-wasm/ghidra-d2game/decomp/0x<RVA>.c), in RVA order: the two unit field
// getters, the MSVC 2003 Release CRT leaves the linker pulled into the DLL (__unlock_file, __openfile,
// __getstream, _memcpy, __flush, __close, __freebuf, ___crtInitCritSecAndSpinCount), the 1.13c
// PLRSAVE item-section writer and the inventory item-id lookup. Stage A batch lane (OBJECTIVE v8.3):
// implementation + marker only, no spec, no vector.
//
// Register-only arguments (Ghidra's in_EAX) are explicit parameters here and named at each function.
// Callees src/ does not own are seam pointers in retail argument order; a probe passes the env last,
// the retail arguments in front. The fatal path Fog Ordinal 10265(id) -> Ordinal 10024(text) ->
// _exit(-1) is modelled as d2game_fatal(id): only the exit is observable.
//
// Prior art: src/core/d2game/d2game_batch_eb7b0.cpp (the 0xEC1E0 item-section wrapper and the seam
// signatures for 0x537D0/0x53470), d2game_batch_3261.cpp and d2game_batch_28ee.cpp (the host-stdio
// collapse of the CRT FILE), d2game_crt_write.cpp (the _pioinfo/nhandle model of __close),
// d2game_batch_2a6d.cpp and d2client_batch_908f.cpp (the InitializeCriticalSectionAndSpinCount
// resolution), and D2MOO PLAYER/PlrSaveItems.cpp PLRSAVE_WriteItems_6FC89B50 (the 1.10f ancestor of
// 0x537D0, whose item-section layout this row matches byte for byte).
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <emscripten.h>
#include <Windows.h>   // CRITICAL_SECTION, Enter/Leave/InitializeCriticalSection

static inline uint8_t  rd8 (const void* p, uint32_t o) { return ((const uint8_t*)p)[o]; }
static inline uint32_t rd32(const void* p, uint32_t o) { uint32_t v; std::memcpy(&v, (const uint8_t*)p + o, 4); return v; }
static inline void     wr16(void* p, uint32_t o, uint16_t v) { std::memcpy((uint8_t*)p + o, &v, 2); }
static inline void     wr32(void* p, uint32_t o, uint32_t v) { std::memcpy((uint8_t*)p + o, &v, 4); }

[[noreturn]] static void d2game_fatal(uint32_t) { _exit(-1); }

// Defined in src/core/d2game/d2game_batch_3e70.cpp (0x3F76 __unlock(lockNum)).
extern "C" void d2game_unlock_3f76(int32_t lockNum);
// Defined below (0x7DE3); 0x4EEA initialises the stream section through it.
extern "C" int32_t d2game_crt_init_crit_sec_and_spin_count(void* cs, uint32_t spin, const struct D2GameCrtInitCritSecEnv* env);

// CRT lock number for __lock/__unlock; the compat critical sections are no-ops (d2game_batch_3261.cpp).
static void d2game_crt_lock(uint32_t lockNum) { (void)lockNum; }

// ---- seam environments ---------------------------------------------------------------------------
// 0x7DE3's callees: the kernel32 lookup (collapsed to host seams) and the 0x7DD3 no-spin fallback.
struct D2GameCrtInitCritSecEnv {
    void*   (*getModuleHandle)(const char* name);                 // IAT GetModuleHandleA
    void*   (*getProcAddress)(void* module, const char* name);    // IAT GetProcAddress
    int32_t (*initCritSecNoSpin)(void* cs);                       // 0x7DD3 ___crtInitCritSecNoSpin@8
};

// 0x4EEA's __mtinitlocknum (0x3F8B). The stream table, __lock/__unlock and __lock_file2/__unlock_file2
// are host state below; 0x5003 is __unlock(1), i.e. 0x3F76.
struct D2GameCrtStreamEnv {
    int32_t (*mtInitLockNum)(int32_t lockNum);                    // 0x3F8B __mtinitlocknum
    const D2GameCrtInitCritSecEnv* initEnv;                       // env for 0x7DE3
};

// 0x5B81's callees, in retail argument order (same seam shape as d2game_crt_write.cpp's __write).
struct D2GameCrtCloseEnv {
    void     (*lockFhandle)(int32_t);                             // 0x88DB __lock_fhandle
    int32_t  (*closeLk)(int32_t);                                 // 0x5AFE __close_lk
    void     (*unlockFhandle)(int32_t);                           // 0x5BF8 _unlock_fhandle
    int32_t* (*errnoPtr)(void);                                   // 0x4E65 __getptd()->_terrno
    uint32_t* (*doserrnoPtr)(void);                               // 0x4E6E __getptd()->_doserrno
    uint32_t  nhandle;                                            // 0x6FD3A214
    uint8_t** pioinfo;                                            // 0x6FD3A220
};

// 0x537D0's callees: the 1.13c split-out item count (0x53260), the item serializer wrapper (0x53470)
// and the D2Common ordinal imports the comments list (1.13c numbering, 1.10f crosswalk in comments).
struct D2GameBatch537d0Env {
    uint16_t (*itemCount)(void* pInventory);                                  // 0x53260
    int32_t  (*writeItem)(void* pItem, void* pOut, uint32_t nSize);           // 0x53470
    int32_t  (*serializeItem)(void* pItem, void* pOut, uint32_t nSize,
                              int32_t a4, int32_t a5, int32_t a6);            // 0xAFD0 #10987
    void*    (*getLeftHandWeapon)(void* pInventory);                          // #10061
    void*    (*getItemFromBodyLoc)(void* pInventory, int32_t bodyLoc);        // #11139
    void*    (*getCursorItem)(void* pInventory);                              // #11017
    void*    (*firstItem)(void* pInventory);                                  // #10460
    void*    (*nextItem)(void* pItem);                                        // #10464
    void*    (*unitIsItem)(void* pItem);                                      // #11147
    uint8_t  (*getBodyLocation)(void* pItem);                                 // #10417
    int32_t  (*getItemNodePage)(void* pItem);                                 // #10698
};
static const D2GameBatch537d0Env kNoEnv537d0 = {};

// 0xEBD10's inventory walk.
struct D2GameBatchEbd10Env {
    void* (*firstItem)(void* pInventory);                                     // #10460
    void* (*nextItem)(void* pItem);                                           // #10464
    void* (*unitIsItem)(void* pItem);                                         // #11147
};

// ---- unit field getters --------------------------------------------------------------------------
// 1.13c D2Game+0x49AB0
// unit arrived in EAX. A null unit is a Fog assertion (id 0x98B); otherwise the dword at +0x488.
extern "C"
EMSCRIPTEN_KEEPALIVE uint32_t D2GAME_GetClientCreateTime(void* unit) {
    if (unit == nullptr) d2game_fatal(0x98B);
    return rd32(unit, 0x488);
}

// 1.13c D2Game+0x49BF0
// unit arrived in EAX. A null unit is a Fog assertion (id 0x8A4); otherwise the byte at +0x08.
extern "C"
EMSCRIPTEN_KEEPALIVE uint8_t d2game_49bf0(void* unit) {
    if (unit == nullptr) d2game_fatal(0x8A4);
    return rd8(unit, 0x08);
}

// ---- CRT leaves ----------------------------------------------------------------------------------
// 1.13c D2Game+0x4CAB (__unlock_file): a FILE inside the CRT _iob window releases lock number
// 0x10 + (stream - 0x6FD1F338) / 0x20; anything else leaves the critical section at stream +
// sizeof(FILE) = +0x20. Retail compares against 0x6FD1F338 (the _base field of the first 0x6FD1F340
// stream) and 0x6FD1F598, both unsigned and inclusive; the 0x1C+8 offset vanishes under the >>5 because
// the streams sit on 0x20 boundaries.
extern "C"
EMSCRIPTEN_KEEPALIVE void D2Game_CrtUnlockFile(void* stream) {
    const uintptr_t p = (uintptr_t)stream;
    if (p >= 0x6FD1F338u && p <= 0x6FD1F598u) {
        d2game_unlock_3f76((int32_t)((p - 0x6FD1F338u) >> 5) + 0x10);
        return;
    }
    LeaveCriticalSection((LPCRITICAL_SECTION)((uint8_t*)stream + 0x20));
}

// 1.13c D2Game+0x04CFD (__openfile): parse the first mode letter into the MSVC open flags, then
// __sopen(name, flags, share, 0x1A4). Host model (d2game_batch_28ee.cpp): the CRT FILE is the host
// stdio stream, so the flags drive the host fopen and the free slot `file` is unused; the open-stream
// counter at 0x6FD2FEE8 has no host reader. NULL is the failure result.
extern "C"
EMSCRIPTEN_KEEPALIVE FILE* d2game_openfile(const char* filename, const char* mode, int32_t shareFlag,
                                            FILE* file) {
    (void)shareFlag;
    (void)file;
    return std::fopen(filename, mode);
}

// 1.13c D2Game+0x4EEA (__getstream): __lock(1), then walk the stream table (retail 0x6FD3A320, count
// 0x6FD3B340) for a free slot. A null slot gets a malloc(0x38) stream whose critical section at +0x20
// is initialised with spin count 4000; success enters that section and returns the stream, failure
// frees it and returns NULL. An existing stream is taken when its _flag (+0x0C) has none of 0x83 set,
// after __mtinitlocknum(index+0x10) for indices 3..19 and __lock_file2(index, stream); a stream that is
// still in use is released with __unlock_file2 and the walk continues. The returned stream's FILE fields
// are reset and __unlock(1) runs.
// The host build never opens a retail FILE through this path (__fsopen is std::fopen, see 0x4CFD and
// d2game_batch_28ee.cpp), so the table entries stay raw 0x38 blocks, but the rule above is transcribed.
static void*    g_crtStreams[20];       // retail 0x6FD3A320 _pstream
static uint32_t g_crtStreamCount = 20;  // retail 0x6FD3B340 _nstream

// 0x4C88 __lock_file2(index, stream): index < 0x14 takes CRT lock index+0x10, else the stream's section.
static void d2game_lock_file2(int32_t index, void* stream) {
    if (index < 0x14) d2game_crt_lock((uint32_t)index + 0x10);
    else EnterCriticalSection((LPCRITICAL_SECTION)((uint8_t*)stream + 0x20));
}
// 0x4CDA __unlock_file2(index, stream).
static void d2game_unlock_file2(int32_t index, void* stream) {
    if (index < 0x14) d2game_unlock_3f76(index + 0x10);
    else LeaveCriticalSection((LPCRITICAL_SECTION)((uint8_t*)stream + 0x20));
}

extern "C"
EMSCRIPTEN_KEEPALIVE void* d2_fn_d2game_4eea(const D2GameCrtStreamEnv* env) {
    static const D2GameCrtStreamEnv kNoEnv = {};
    const D2GameCrtStreamEnv& e = env ? *env : kNoEnv;

    void* result = nullptr;
    d2game_crt_lock(1);
    for (uint32_t i = 0; i < g_crtStreamCount; ) {
        uint8_t* stream = (uint8_t*)g_crtStreams[i];
        if (stream == nullptr) {
            stream = (uint8_t*)std::malloc(0x38);
            g_crtStreams[i] = stream;
            if (stream == nullptr) break;
            if (d2game_crt_init_crit_sec_and_spin_count(stream + 0x20, 4000, e.initEnv) == 0) {
                std::free(stream);
                g_crtStreams[i] = nullptr;
                break;
            }
            EnterCriticalSection((LPCRITICAL_SECTION)(stream + 0x20));
            result = stream;
            break;
        }
        if ((stream[0x0C] & 0x83) == 0) {
            if (i > 2 && i < 0x14) {
                if (e.mtInitLockNum == nullptr || e.mtInitLockNum((int32_t)i + 0x10) == 0) break;
            }
            d2game_lock_file2((int32_t)i, stream);
            uint8_t* taken = (uint8_t*)g_crtStreams[i];
            if ((taken[0x0C] & 0x83) == 0) {
                result = taken;
                break;
            }
            d2game_unlock_file2((int32_t)i, taken);
        }
        ++i;
    }
    if (result != nullptr) {
        uint8_t* s = (uint8_t*)result;
        wr32(s, 0x00, 0);                 // _ptr
        wr32(s, 0x04, 0);                 // _cnt
        wr32(s, 0x08, 0);                 // _base
        wr32(s, 0x0C, 0);                 // _flag
        wr32(s, 0x10, 0xFFFFFFFFu);       // _file = -1
        wr32(s, 0x1C, 0);                 // _tmpfname
    }
    d2game_unlock_3f76(1);                 // 0x5003: __unlock(1)
    return result;
}

// 1.13c D2Game+0x537D0 (PLRSAVE item section): write the 'JM' item bitstream.
// dest arrives in ECX, the inventory in EDX, game/unit/nSize are the first three stack arguments.
// The 1.10f body is PLRSAVE_WriteItems_6FC89B50 (D2MOO PLAYER/PlrSaveItems.cpp); the 1.13c deltas are
// the item count moving to 0x53260 (cursor + non-null INVENTORY_UnitIsItem chain, ushort), and a
// pre-pass that serialises the two hand items into a 0x2000 stack scratch through #10987 and only
// reports failure, before the real write pass through the 0x53470 wrapper.
// Layout: 'JM' at +0, the ushort count at +2, then the node loop (skipping page-3 RARM/LARM items),
// the two hand items in weapon order, and the cursor item when its unit type is 4 (UNIT_ITEM).
// Every zero-byte item write decrements the count while it is nonzero; the adjusted count is written
// back into the +2 slot and the byte length is the result.
extern "C"
EMSCRIPTEN_KEEPALIVE int32_t d2_fn_d2game_537D0(uint8_t* dest, void* pInventory, void* game, void* unit,
                                                int32_t nSize, const D2GameBatch537d0Env* env) {
    (void)game;
    (void)unit;
    const D2GameBatch537d0Env& e = env ? *env : kNoEnv537d0;

    if (nSize < 2) return 0;
    wr16(dest, 0, 0x4D4A);
    uint16_t count = e.itemCount(pInventory);
    if (nSize < 4) return 2;
    uint8_t* pItemCount = dest + 2;
    wr16(dest, 2, count);
    uint8_t* pData = dest + 4;
    if (pInventory == nullptr) return 4;

    void* pWeapon = e.getLeftHandWeapon(pInventory);
    void* pRight  = e.getItemFromBodyLoc(pInventory, 4);   // BODYLOC_RARM
    void* pLeft   = e.getItemFromBodyLoc(pInventory, 5);   // BODYLOC_LARM
    void* pCursor = e.getCursorItem(pInventory);

    // Pre-pass: can both hand items serialise? The bytes go to a scratch block and are discarded.
    uint8_t scratch[0x2000];
    if (pWeapon != nullptr) {
        if (pRight == pWeapon) {
            if (e.serializeItem(pRight, scratch, 0x2000, 1, 1, 0) < 0) return -1;
            if (pLeft != nullptr && e.serializeItem(pLeft, scratch, 0x2000, 1, 1, 0) < 0) return -1;
        } else {
            if (e.serializeItem(pLeft, scratch, 0x2000, 1, 1, 0) < 0) return -1;
            if (pRight != nullptr && e.serializeItem(pRight, scratch, 0x2000, 1, 1, 0) < 0) return -1;
        }
    } else {
        if (pRight != nullptr && e.serializeItem(pRight, scratch, 0x2000, 1, 1, 0) < 0) return -1;
        if (pLeft != nullptr && e.serializeItem(pLeft, scratch, 0x2000, 1, 1, 0) < 0) return -1;
    }

    // Node loop: skip the body-location items that belong to the two hands.
    for (void* pItem = e.firstItem(pInventory); pItem != nullptr; pItem = e.nextItem(pItem)) {
        void* pCheckedItem = e.unitIsItem(pItem);
        const uint8_t bodyLoc = e.getBodyLocation(pCheckedItem);
        if (e.getItemNodePage(pItem) != 3 || (bodyLoc != 4 && bodyLoc != 5)) {
            const int32_t n = e.writeItem(pCheckedItem, pData,
                                          (uint32_t)(nSize - (int32_t)(pData - dest)));
            if (n < 0) return -1;
            pData += n;
            if (n == 0 && count > 0) --count;
        }
    }

    // Weapon order: right before left when the right hand holds the weapon, left before right otherwise.
    if (pWeapon != nullptr) {
        if (pRight == pWeapon) {
            int32_t n = e.writeItem(pRight, pData, (uint32_t)(nSize - (int32_t)(pData - dest)));
            if (n < 0) return -1;
            pData += n;
            if (n == 0 && count > 0) --count;
            if (pLeft != nullptr) {
                n = e.writeItem(pLeft, pData, (uint32_t)(nSize - (int32_t)(pData - dest)));
                if (n < 0) return -1;
                pData += n;
                if (n == 0 && count > 0) --count;
            }
        } else {
            int32_t n = e.writeItem(pLeft, pData, (uint32_t)(nSize - (int32_t)(pData - dest)));
            if (n < 0) return -1;
            pData += n;
            if (n == 0 && count > 0) --count;
            if (pRight != nullptr) {
                n = e.writeItem(pRight, pData, (uint32_t)(nSize - (int32_t)(pData - dest)));
                if (n < 0) return -1;
                pData += n;
                if (n == 0 && count > 0) --count;
            }
        }
    } else {
        if (pRight != nullptr) {
            int32_t n = e.writeItem(pRight, pData, (uint32_t)(nSize - (int32_t)(pData - dest)));
            if (n < 0) return -1;
            pData += n;
            if (n == 0 && count > 0) --count;
        }
        if (pLeft != nullptr) {
            int32_t n = e.writeItem(pLeft, pData, (uint32_t)(nSize - (int32_t)(pData - dest)));
            if (n < 0) return -1;
            pData += n;
            if (n == 0 && count > 0) --count;
        }
    }

    // Cursor item, only when it is a UNIT_ITEM (unit type 4).
    if (pCursor != nullptr && rd32(pCursor, 0x00) == 4) {
        const int32_t n = e.writeItem(pCursor, pData, (uint32_t)(nSize - (int32_t)(pData - dest)));
        if (n < 0) return -1;
        pData += n;
        if (n == 0 && count > 0) --count;
    }

    wr16(pItemCount, 0, count);
    return (int32_t)(pData - dest);
}

// 1.13c D2Game+0x5380 (_memcpy): the decompiled copy picks the direction from the address ranges (a
// destination inside [source, source+size) copies backwards), i.e. it is overlap-safe and returns the
// destination. std::memmove is exactly that rule.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_5380(uint32_t* r) {
    r[4] = (uint32_t)(uintptr_t)std::memmove(reinterpret_cast<void*>((uintptr_t)r[0]),
                                            reinterpret_cast<const void*>((uintptr_t)r[1]), (size_t)r[2]);
}

// 1.13c D2Game+0x5924 (__flush): a writable stream (_flag & 3 == 2) with buffered bytes or a force flag
// (_flag & 0x108) writes _ptr - _base bytes at _file; a short write sets _IOERR (0x20) and returns -1,
// a full write clears the read bit when _flag is negative. The buffer is then emptied. Host stdio owns
// those counters and flags and reports the same 0/-1 result, so std::fflush is the whole rule.
extern "C"
EMSCRIPTEN_KEEPALIVE int32_t D2Game_CrtFlush(FILE* stream) {
    return (int32_t)std::fflush(stream);
}

// 1.13c D2Game+0x5B81 (__close): a handle below nhandle whose ioinfo osfile word (record + 4, FOPEN bit
// 0x01) is set locks the handle, re-checks (a close between check and lock gives errno 9, -1) and calls
// __close_lk; either way _unlock_fhandle runs. A bad handle sets errno 9 and doserrno 0 and returns -1.
// ioinfo records are 0x24 bytes in 32 blocks of 32 (d2game_crt_write.cpp).
extern "C"
EMSCRIPTEN_KEEPALIVE int32_t D2Game_CrtClose(int32_t fileHandle, const D2GameCrtCloseEnv* env) {
    const uint32_t fd = (uint32_t)fileHandle;
    if (fd < env->nhandle) {
        uint8_t* osfile = env->pioinfo[fd >> 5] + 4 + (fd & 0x1f) * 0x24;
        if ((*osfile & 1) != 0) {
            int32_t result;
            env->lockFhandle(fileHandle);
            if ((*osfile & 1) == 0) {
                *env->errnoPtr() = 9;
                result = -1;
            } else {
                result = env->closeLk(fileHandle);
            }
            env->unlockFhandle(fileHandle);
            return result;
        }
    }
    *env->errnoPtr() = 9;
    *env->doserrnoPtr() = 0;
    return -1;
}

// 1.13c D2Game+0x5C1C (__freebuf) d2game_batch_7de3.cpp:d2game_freebuf -- a stream with (_flag & 0x83) and _IOFBF-ish bit 8 frees _base and
// clears 0x408 in the flag word and the three buffer fields. In the host model (d2game_batch_3261.cpp)
// the FILE is the glibc stream: its buffer belongs to glibc and is released by fclose, so there is no
// host buffer to free and no host field to reset -- the function has no observable effect.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2game_freebuf(FILE* stream) {
    (void)stream;
}

// ---- critical-section initialisation -------------------------------------------------------------
typedef int32_t (*D2GameInitCritSecSpinFn)(void* cs, uint32_t spin);

static D2GameInitCritSecSpinFn g_initCritSecSpin = nullptr;  // retail 0x6FD30080 (cached API pointer)
static uint32_t g_osPlatform = 2;                            // retail 0x6FD2FD24; 1 skips the lookup

// 0x7DD3 ___crtInitCritSecNoSpin@8: InitializeCriticalSection(cs), return 1.
static int32_t d2game_init_crit_sec_no_spin(void* cs) {
    InitializeCriticalSection((LPCRITICAL_SECTION)cs);
    return 1;
}
// The resolved slot has the spin-count signature; the no-spin thunk ignores the spin argument.
typedef int32_t (*D2GameInitCritSecNoSpinFn)(void* cs);
static D2GameInitCritSecNoSpinFn g_noSpinFallback = d2game_init_crit_sec_no_spin;
static int32_t d2game_init_crit_sec_spin_fallback(void* cs, uint32_t spin) {
    (void)spin;
    return g_noSpinFallback(cs);
}

// 1.13c D2Game+0x7DE3 (___crtInitCritSecAndSpinCount): resolve InitializeCriticalSectionAndSpinCount
// once -- on a non-NT platform word, through kernel32 -- and leave the no-spin thunk 0x7DD3 in the slot
// when the lookup fails; then call the slot with (cs, spin). The result is the API's BOOL, which the
// 0x4EEA caller tests against 0.
extern "C"
EMSCRIPTEN_KEEPALIVE int32_t d2game_crt_init_crit_sec_and_spin_count(void* cs, uint32_t spin,
                                               const D2GameCrtInitCritSecEnv* env) {
    static const D2GameCrtInitCritSecEnv kNoEnv = {};
    const D2GameCrtInitCritSecEnv& e = env ? *env : kNoEnv;
    if (g_initCritSecSpin == nullptr) {
        D2GameInitCritSecSpinFn resolved = nullptr;
        if (g_osPlatform != 1) {
            void* module = e.getModuleHandle ? e.getModuleHandle("kernel32.dll") : nullptr;
            if (module != nullptr && e.getProcAddress != nullptr) {
                resolved = reinterpret_cast<D2GameInitCritSecSpinFn>(
                    e.getProcAddress(module, "InitializeCriticalSectionAndSpinCount"));
            }
        }
        if (resolved == nullptr) {
            g_noSpinFallback = e.initCritSecNoSpin ? e.initCritSecNoSpin
                                                   : d2game_init_crit_sec_no_spin;
            resolved = d2game_init_crit_sec_spin_fallback;
        }
        g_initCritSecSpin = resolved;
    }
    return g_initCritSecSpin(cs, spin);
}

// ---- inventory item-id lookup --------------------------------------------------------------------
// D2Game EBD10 (sub_EBD10; retail-layout port d2game_ebd10.cpp): the 1-based index of the inventory item whose unit id (+0x0C) is `itemId`.
// Both arguments are stack dwords (unit, itemId). -1, a null inventory or an absent id give 0; the walk
// counts every chain node, using -1 for a node that INVENTORY_UnitIsItem rejects. Index 0x7FFF and
// above is a Fog assertion (id 0x185). Only the low 16 bits are the result.
extern "C"
EMSCRIPTEN_KEEPALIVE uint16_t D2GAME_FindInventoryItemIndex(void* unit, int32_t itemId,
                                                 const D2GameBatchEbd10Env* env) {
    if (itemId == -1) return 0;
    void* pInventory = unit != nullptr ? (void*)(uintptr_t)rd32(unit, 0x60) : nullptr;
    if (pInventory == nullptr) return 0;

    int32_t index = 1;
    for (void* pItem = env->firstItem(pInventory); pItem != nullptr; pItem = env->nextItem(pItem)) {
        void* pChecked = env->unitIsItem(pItem);
        const int32_t id = pChecked != nullptr ? (int32_t)rd32(pChecked, 0x0C) : -1;
        if (id == itemId) {
            if (index < 0x7FFF) return (uint16_t)index;
            d2game_fatal(0x185);
        }
        ++index;
    }
    return 0;
}
