// src/core/gfx/storm_batch_1000.cpp -- 12 UNMAPPED 1.13c Storm.dll rows reimplemented from the
// cached decompiles in build-wasm/ghidra-storm/decomp/0x<RVA>.c (alpha lane: no spec, no vector).
//
// These are Storm's internal SCode.cpp C++ helpers plus the CRT critical-section leaves: the integer
// power routine (0x1000), two object teardowns and a constructor (0x1090/0x11A0/0x1120), the list
// clear/relink leaves (0x1310 and the 0x4520/0x4390 helpers they call), the AUINST allocation wrapper
// (0x1290), the 12-byte-slot debug-cache fetch (0x1360) and the two CRITICAL_SECTION leaves
// (0x1420/0x1460). Retail base 0x6FBF0000; object/list fields keep retail's 32-bit layout, the same
// convention as src/core/storm/storm_batch_123b0.cpp.
//
// The unmapped callees are seams with retail's argument order: 0x1B960 SMemFree and 0x1C580 SMemAlloc
// (the Storm pool, not the host allocator), and 0x38EB0/0x37ED0/0x3A330 (the debug-slot helpers).
// Ghidra artifacts worth naming:
//   * 0x11A0 re-reads this+0x14 as the loop head; the unlink (0x4B80) zeroes the node, which is the
//     only head-clearing store on that path, so the loop is transcribed exactly as decompiled;
//   * 0x1360 indexes Ghidra's _RTL_CRITICAL_SECTION_DEBUG; the byte offsets below are that type's
//     (Type +0, CriticalSection +4, ProcessLocksList +0xC, 12-byte slot) flattened explicitly;
//   * 0x4520/0x4390 carry a hidden EBX/EAX; the callers here pass the object and the alloc result.
#include <emscripten.h>
#include <cstdint>
#include <cstring>
#include <Windows.h>

namespace {

constexpr uint32_t kVtable = 0x6FC36850;                 // PTR_LAB_6fc36850
constexpr const char* kScode = "..\\3rdParty\\STORM\\SOURCE\\SCODE.CPP";
constexpr const char* kAuinst = "..\\AUINST";            // s___AUINST___6fc415a4

// unmapped Storm internals, retail argument order (seams; see src/core/storm/storm_batch_123b0.cpp)
extern "C" int32_t Storm_Sub1B960(void* p, const char* file, int32_t line);                       // 0x1B960 SMemFree
extern "C" uint32_t* Storm_Sub1C580(uint32_t size, const char* file, int32_t line, uint32_t flags); // 0x1C580 SMemAlloc
// 0x38EB0 / 0x37ED0 / 0x3A330 are defined elsewhere with an extra seam-env argument; the 0x1360
// probe has no env to give them, so its slow path calls these file-local seams instead.
void Seam38EB0(uint32_t*) {}                                                            // 0x38EB0
void Seam37ED0(uint32_t, uint8_t*, uint32_t, uint32_t) {}                               // 0x37ED0
void Seam3A330(uint32_t*, uint32_t*, uint32_t*, uint32_t, uint32_t, uint8_t*) {}        // 0x3A330

uint32_t rd32(const void* p) { uint32_t v; std::memcpy(&v, p, 4); return v; }
void wr32(void* p, uint32_t v) { std::memcpy(p, &v, 4); }

// 0x4C20/0x4B80 body: unlink a doubly-linked node. The tag is either ~prev (negative) or the byte
// delta from the node to the next node's +4 field.
void UnlinkNode(uint8_t* n) {
    uint32_t link = rd32(n);
    if (link == 0) return;
    uint32_t tag = rd32(n + 4);
    uint8_t* prev;
    if ((int32_t)tag < 0) prev = (uint8_t*)(uintptr_t)(uint32_t)~tag;
    else prev = n + (uint32_t)(tag - rd32((void*)((uint8_t*)(uintptr_t)link + 4)));
    wr32(prev, link);
    // retail reloads both fields after the store to prev (Storm+0x1CF0 disassembly: mov ecx,[eax] / mov edx,[eax+4]
    // after the store at +0x42 and +0x7F): when prev is n+4 (the tag slot) the stored link is the value written onward.
    link = rd32(n);
    tag = rd32(n + 4);
    wr32((void*)((uint8_t*)(uintptr_t)link + 4), tag);
    wr32(n, 0);
    wr32(n + 4, 0);
}

// unmodified 0x4B80: two unlinks of the same node (the second sees link == 0 and does nothing)
void Storm_Sub4B80(uint8_t* p) { UnlinkNode(p); UnlinkNode(p); }

// 0x1310: clear the list whose head is the dword at EBX+8; the unlink rewrites the head through the
// prev pointer, which is what ends the loop.
void Storm_Sub1310(uint8_t* ebx) {
    for (;;) {
        uint32_t head = rd32(ebx + 8);
        if ((int32_t)head < 1) break;
        UnlinkNode((uint8_t*)(uintptr_t)head);
    }
}

// 0x4520: clear the list at EBX+8, then unlink the node embedded at param_1+4.
void Storm_Sub4520(uint8_t* p) {
    Storm_Sub1310(p);
    UnlinkNode(p + 4);
}

}  // namespace: Storm_Sub4390 has external linkage, the storm batches call it
// 0x4390 __thiscall(this, node, mode); EAX is the list anchor. this/node fall back to eax+4.
void Storm_Sub4390(uint32_t* eax, uint8_t* this_, uint8_t* node_arg, int32_t mode) {
    uint8_t* node = node_arg ? node_arg : (uint8_t*)(eax + 1);
    UnlinkNode(node);
    uint8_t* anchor = this_ ? this_ : (uint8_t*)(eax + 1);
    if (mode != 1) {
        uint32_t link = rd32(anchor);
        wr32(node, link);
        wr32(node + 4, rd32((void*)((uint8_t*)(uintptr_t)link + 4)));
        wr32((void*)((uint8_t*)(uintptr_t)link + 4), (uint32_t)(uintptr_t)node_arg);
        wr32(anchor, (uint32_t)(uintptr_t)node);
        return;
    }
    wr32(node, (uint32_t)(uintptr_t)anchor);
    wr32(node + 4, rd32(anchor + 4));
    uint32_t tag = rd32(anchor + 4);
    if ((int32_t)tag < 0) {
        wr32((void*)(uintptr_t)(uint32_t)~tag, (uint32_t)(uintptr_t)node);
        wr32(anchor + 4, (uint32_t)(uintptr_t)node_arg);
        return;
    }
    uint32_t eax_head = rd32(eax);
    if ((int32_t)eax_head < 0)
        eax_head = (uint32_t)((uintptr_t)anchor - rd32((void*)((uint8_t*)(uintptr_t)rd32(anchor) + 4)));
    wr32((void*)(uintptr_t)(tag + eax_head), (uint32_t)(uintptr_t)node);
    wr32(anchor + 4, (uint32_t)(uintptr_t)node_arg);
}
namespace {

// 0x1000: integer power. Retail returns the same integer widened to float10 (ST(0)); the negative
// exponent path does integer division first, so 0 or 1.
int32_t Storm_Sub1000(int32_t base, int32_t exp) {
    uint32_t u = exp < 0 ? 0u - (uint32_t)exp : (uint32_t)exp;
    int32_t acc = 1;
    for (;;) {
        if (u & 1) acc *= base;
        u >>= 1;
        if (u == 0) break;
        base *= base;
    }
    if (exp < 0) return (int32_t)(1 / (int64_t)acc);
    return acc;
}

// 0x1050: SMemFree(p, "delete", -1).
extern "C" void Storm_Sub1050(void* p) {
    if (p != nullptr) Storm_Sub1B960(p, "delete", -1);
}

// 0x1080: DeleteCriticalSection (pointer arrives in EAX).
void Storm_Sub1080(LPCRITICAL_SECTION cs) { DeleteCriticalSection(cs); }

// 0x1090: free members +5/+0xB/+0x11 (SCode.cpp 0x69/0x6A/0x6B), then the 0x4B80 teardown.
void Storm_Sub1090(uint32_t* p) {
    if (p[5]) Storm_Sub1B960((void*)(uintptr_t)p[5], kScode, 0x69);
    if (p[0xB]) Storm_Sub1B960((void*)(uintptr_t)p[0xB], kScode, 0x6A);
    if (p[0x11]) Storm_Sub1B960((void*)(uintptr_t)p[0x11], kScode, 0x6B);
    Storm_Sub4B80((uint8_t*)p);
}

// 0x1120: constructor. +0x10 is the self anchor, +0x14 its complement; both are retail 32-bit fields.
void Storm_Sub1120(uint32_t* p) {
    uint32_t* self = p + 4;
    p[0] = kVtable;
    self[0] = 0;
    p[5] = 0;
    self[0] = (uint32_t)(uintptr_t)self;
    p[3] = 0;
    p[5] = ~(uint32_t)(uintptr_t)self;
    p[1] = 0;
    p[2] = 0;
    p[6] = 0;
    p[7] = 0;
}

// 0x11A0: destructor. Free +6, then while +5 is a positive node: unlink it and free it on the next
// turn with the AUINST source; finally the 0x4520 teardown of the object at +0xC.
void Storm_Sub11A0(uint32_t* p) {
    p[0] = kVtable;
    uint32_t q = p[6];
    const char* file = kScode;
    int32_t line = 0x99;
    for (;;) {
        if (q) Storm_Sub1B960((void*)(uintptr_t)q, file, line);
        q = p[5];
        if ((int32_t)q < 1) break;
        Storm_Sub4B80((uint8_t*)(uintptr_t)q);
        file = kAuinst;
        line = -2;
    }
    Storm_Sub4520((uint8_t*)p + 0xC);
}

// 0x1250: deleting-destructor thunk.
void Storm_Sub1250(uint32_t* p) { Storm_Sub11A0(p); }

// 0x1290: SMemAlloc(param_2+0x18, AUINST, -2, param_3|8), zero the two head words, then 0x4390 with
// EAX = the allocation (still in EAX after SMemAlloc) and this = null.
uint32_t* Storm_Sub1290(int32_t param_2, uint32_t param_3, uint32_t edi) {
    uint32_t* p = Storm_Sub1C580((uint32_t)(param_2 + 0x18), kAuinst, -2, param_3 | 8);
    if (p == nullptr) return nullptr;
    p[0] = 0;
    p[1] = 0;
    if (edi != 0) Storm_Sub4390(p, nullptr, (uint8_t*)p, (int32_t)edi);
    return p;
}

// 0x1360: per-index (param_1 >> 16 & 0xFF) debug-slot fetch from a CRITICAL_SECTION-backed cache.
// Offsets are Ghidra's _RTL_CRITICAL_SECTION_DEBUG flattened to the retail 32-bit layout; the cache
// base is edi[2].DebugInfo (edi + 0x30).
uint32_t Storm_Sub1360(uint32_t param_1, uint32_t in_eax, LPCRITICAL_SECTION edi) {
    uint32_t idx = (param_1 >> 16) & 0xFF;
    uint8_t* dbg = (uint8_t*)(uintptr_t)rd32((uint8_t*)edi + 0x30);
    uint8_t* slot = dbg + idx * 12;
    if (rd32(slot) != 0 && in_eax <= rd32(dbg + 4 + idx * 12)) return rd32(dbg + 8 + idx * 12);
    EnterCriticalSection(edi);
    if (rd32(slot) != 0) Seam38EB0((uint32_t*)(uintptr_t)rd32(slot + 8));
    if (in_eax < 0x281) in_eax = 0x280;
    uint8_t name[0x40];
    name[0] = 0;
    Seam37ED0(param_1, name, 0x40, 0);
    uint8_t* out = dbg + 8 + idx * 12;
    Seam3A330((uint32_t*)name, (uint32_t*)name, nullptr, in_eax >> 2, 0x40000, out);
    wr32(dbg + 4 + idx * 12, in_eax);
    wr32(slot, 1);
    LeaveCriticalSection(edi);
    return rd32(out);
}

// 0x1420: InitializeCriticalSection, return the argument.
LPCRITICAL_SECTION Storm_Sub1420(LPCRITICAL_SECTION cs) {
    InitializeCriticalSection(cs);
    return cs;
}

// 0x1460: DeleteCriticalSection.
void Storm_Sub1460(LPCRITICAL_SECTION cs) { DeleteCriticalSection(cs); }

}  // namespace

// the 0x4C20 node unlink on retail's 32-bit node layout, for the guest-layout element entries in storm_batch_1f20.cpp
extern "C" void Storm_GuestUnlink(uint8_t* n) { UnlinkNode(n); }
// 0x4390 for the other Storm list allocators (storm_batch_b51.cpp, storm_sub_4430.cpp): EAX anchor, ECX this, node, EDI mode.
extern "C" void Storm_ListLink4390(uint32_t* eax, uint8_t* this_, uint8_t* node, int32_t mode) { Storm_Sub4390(eax, this_, node, mode); }

// 1.13c Storm+0x1000 (sub_1000): power(base, exp); retail widens the integer result to float10.
extern "C" EMSCRIPTEN_KEEPALIVE void storm_port_1000(uint32_t* r) { r[4] = (uint32_t)Storm_Sub1000((int32_t)r[0], (int32_t)r[1]); }

// 1.13c Storm+0x1050 (sub_1050): SMemFree(p, "delete", -1) when p is non-null.
extern "C" EMSCRIPTEN_KEEPALIVE void storm_port_1050(uint32_t* r) { Storm_Sub1050((void*)(uintptr_t)r[0]); r[4] = 0; }

// 1.13c Storm+0x1080 (sub_1080): DeleteCriticalSection; the pointer arrives in EAX (r[0]).
extern "C" EMSCRIPTEN_KEEPALIVE void storm_port_1080(uint32_t* r) { Storm_Sub1080((LPCRITICAL_SECTION)(uintptr_t)r[0]); r[4] = 0; }

// 1.13c Storm+0x1090 (sub_1090): free the three member blocks and run the 0x4B80 teardown.
extern "C" EMSCRIPTEN_KEEPALIVE void storm_port_1090(uint32_t* r) { Storm_Sub1090((uint32_t*)(uintptr_t)r[0]); r[4] = 0; }

// 1.13c Storm+0x1120 (sub_1120): object constructor.
extern "C" EMSCRIPTEN_KEEPALIVE void storm_port_1120(uint32_t* r) { Storm_Sub1120((uint32_t*)(uintptr_t)r[0]); r[4] = 0; }

// 1.13c Storm+0x11A0 (sub_11A0): object destructor.
extern "C" EMSCRIPTEN_KEEPALIVE void storm_port_11a0(uint32_t* r) { Storm_Sub11A0((uint32_t*)(uintptr_t)r[0]); r[4] = 0; }

// 1.13c Storm+0x1250 (sub_1250): deleting-destructor thunk over the 0x11A0 body.
extern "C" EMSCRIPTEN_KEEPALIVE void storm_port_1250(uint32_t* r) { Storm_Sub1250((uint32_t*)(uintptr_t)r[0]); r[4] = 0; }

// 1.13c Storm+0x1290 (sub_1290): r[0]=param_2, r[1]=param_3, r[2]=EDI (mode); r[4]=allocation or 0.
extern "C" EMSCRIPTEN_KEEPALIVE void storm_port_1290(uint32_t* r) {
    r[4] = (uint32_t)(uintptr_t)Storm_Sub1290((int32_t)r[0], r[1], r[2]);
}

// 1.13c Storm+0x1310 (sub_1310): clear the list whose head is at EBX+8; EBX is r[0].
extern "C" EMSCRIPTEN_KEEPALIVE void storm_port_1310(uint32_t* r) { Storm_Sub1310((uint8_t*)(uintptr_t)r[0]); r[4] = 0; }

// 1.13c Storm+0x1360 (sub_1360): r[0]=param_1, r[1]=EAX (requested size), r[2]=EDI (cache lock).
extern "C" EMSCRIPTEN_KEEPALIVE void storm_port_1360(uint32_t* r) {
    r[4] = Storm_Sub1360(r[0], r[1], (LPCRITICAL_SECTION)(uintptr_t)r[2]);
}

// 1.13c Storm+0x1420 (sub_1420): InitializeCriticalSection; returns the argument.
extern "C" EMSCRIPTEN_KEEPALIVE void storm_port_1420(uint32_t* r) {
    r[4] = (uint32_t)(uintptr_t)Storm_Sub1420((LPCRITICAL_SECTION)(uintptr_t)r[0]);
}

// 1.13c Storm+0x1460 (sub_1460): DeleteCriticalSection.
extern "C" EMSCRIPTEN_KEEPALIVE void storm_port_1460(uint32_t* r) { Storm_Sub1460((LPCRITICAL_SECTION)(uintptr_t)r[0]); r[4] = 0; }
