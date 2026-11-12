// src/core/d2game/d2game_batch_c7130.cpp -- twelve 1.13c D2Game.dll quest/unit helpers transcribed from
// the cached Ghidra decompiles (build-wasm/ghidra-d2game/decomp/0x<RVA>.c). Base 0x6FC20000.
// Stage A batch lane: implementation + marker only. Records are opaque host blocks read at their
// retail byte offsets; pointer fields are 32-bit retail values resolved through env->ptr. Register-only
// arguments (in_EAX / unaff_ESI / unaff_EDI) are explicit parameters. Unmapped callees are Env seams in
// retail argument order. Fatal path: Ordinal 10265(code) -> 10024(text) -> _exit(-1).
// The two unit-spawn bodies (0xC8CD0, 0xC94E0) share one inlined SUnit init; the decompile's duplicated
// alloc-failed arm of 0xC8CD0 is a stack-variable artefact and is modelled as the early return.
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <emscripten.h>
#include "d2game_batch_c7130.h"

static inline uint8_t  r8 (const void* p, uint32_t o) { return ((const uint8_t*)p)[o]; }
static inline uint32_t r32(const void* p, uint32_t o) { uint32_t v; std::memcpy(&v, (const uint8_t*)p + o, 4); return v; }
static inline void w8 (void* p, uint32_t o, uint8_t v)  { ((uint8_t*)p)[o] = v; }
static inline void w16(void* p, uint32_t o, uint16_t v) { std::memcpy((uint8_t*)p + o, &v, 2); }
static inline void w32(void* p, uint32_t o, uint32_t v) { std::memcpy((uint8_t*)p + o, &v, 4); }


[[noreturn]] static void fatal(const D2GameBatchC7130Env* e, uint32_t code) {
    e->ord10024(0x6fd19ff9, e->ord10265(code));
    _exit(-1);
}

// Shared SUnit init tail of 0xC8CD0 / 0xC94E0 (classId 0x13e / 100, mode byte 0xb, SUnit.cpp:0x265).
static void spawnInit(const D2GameBatchC7130Env* e, void* u, void* game, uint32_t classId, uint32_t kind4)
{
    w32(u, 0, 2); w32(u, 4, classId); w32(u, 8, r32(game, 0x1c)); w32(u, 0x80, 0); // +0x80 = game ptr
    uint32_t f = r32(u, 0xc8);
    f = r32(game, 0x70) == 0 ? (f & 0xfdffffff) : (f | 0x2000000);
    f |= 0x4000000; w32(u, 0xc8, f);
    uint8_t b = e->ord10459(e->ord10826(e->ord10331(u)));
    w8(u, 0x18, b); w32(u, 0x1c, r32(game, 0xbc + b * 4)); w32(u, 0x74, 0);
    w32(u, 0xc4, r32(u, 0xc4) | 0x10); w32(u, 0xd0, 0xb);
    uint32_t n = r32(game, 0x98) + 1; if (n == 0xffffffffu) n = 1; w32(game, 0x98, n);
    uint32_t p = e->ord10045("..\\Source\\D2Game\\UNIT\\SUnit.cpp", 0x265, 0);
    w32(u, 0x14, p); std::memset(e->ptr(p), 0, 0x38);
    w32(e->ptr(p), 0, (uint32_t)(uintptr_t)e->ord10688(classId));
    w32(u, 0x10, kind4); e->ord10445(u); e->sub2F300();
}

// 1.13c D2Game+0xC7130
extern "C" EMSCRIPTEN_KEEPALIVE
void d2_fn_d2game_C7130(void* ecx, void* quest, int32_t* edi, const D2GameBatchC7130Env* e)
{
    uint8_t* d = (uint8_t*)e->ptr(r32(quest, 0x18));
    if (e->subC6A30(ecx)) { d[2] = 1; return; }
    d[3] = 1;
    uint8_t pkt[6]; uint32_t id = 0x10a5d; std::memcpy(pkt, &id, 4); pkt[4] = pkt[5] = 0;
    if (edi && *edi == 0) e->ord10920(edi);
    e->sendPacket8A3E0(pkt, 6);
}

// 1.13c D2Game+0xC71A0
extern "C" EMSCRIPTEN_KEEPALIVE
void d2_fn_d2game_C71A0(void* ecx, void* quest, void* param1, const D2GameBatchC7130Env* e)
{
    uint8_t* d = (uint8_t*)e->ptr(r32(quest, 0x18));
    d[5] = 1;
    uint32_t u = r32(param1, 4);
    w32(d, 0xc, u == 0 ? 0xffffffffu : r32(e->ptr(u), 0xc));
    if (r8(quest, 9) == 1) {
        if (r8(quest, 0xc) < 2) {
            if (e->subC6A30(ecx)) d[2] = 1; else d[3] = 1;
            w8(quest, 0xc, 3); w8(quest, 0x14, 0);
        }
        if (r8(quest, 0xb) == 0) {
            w8(quest, 0x14, 0); w8(quest, 0xb, 2);
            e->regCb(0x6fce52b0, 0);
        }
    }
    e->ord11090(u ? e->ptr(u) : nullptr, r32(d, 8));
}

// 1.13c D2Game+0xC7250
extern "C" EMSCRIPTEN_KEEPALIVE
void d2_fn_d2game_C7250(void* esi, const D2GameBatchC7130Env* e)
{
    uint8_t* d = (uint8_t*)e->ptr(r32(esi, 0x18));
    if (r8(esi, 0xc) == 0 && d[1] == 0) {
        uint32_t v = r32(e->ptr(r32(esi, 4)), 0x10f4);
        if (v == 0) fatal(e, 0x1fb);
        e->sub1780((uint8_t*)e->ptr(v) + 0x18);
        e->sub33AF0(esi);
        d[1] = 1;
    }
}

// Quest-record constructor shared by 0xC7480 / 0xC9400.
static void questInit(const D2GameBatchC7130Env* e, void* q, const uint32_t (&cb)[6], const char* file,
                      uint32_t line, uint32_t a2e0, uint32_t a10, uint8_t b0d, uint32_t zeroDwords)
{
    std::memset((uint8_t*)q + 0xa0, 0, 0x3c);
    w32(q, 0xa0, cb[0]); w32(q, 0xcc, cb[1]); w32(q, 0xac, cb[2]); w32(q, 0xc8, cb[3]); w32(q, 0xd4, cb[4]);
    w32(q, 0xdc, cb[5]); w8(q, 10, 1); w8(q, 0xc, 0);
    uint32_t p = e->ord10045(file, line, 0);
    w32(q, 0xe0, a2e0); w32(q, 0x10, a10); w32(q, 0xe8, 0); w32(q, 0x18, p); w8(q, 0xd, b0d);
    std::memset(e->ptr(p), 0, zeroDwords * 4);
}

// 1.13c D2Game+0xC7480
extern "C" EMSCRIPTEN_KEEPALIVE
void d2_fn_d2game_C7480(void* q, const D2GameBatchC7130Env* e)
{
    const uint32_t cb[6] = {(e->modBase + 0xC37F0), (e->modBase + 0xC6B30), (e->modBase + 0xC73A0), (e->modBase + 0xC3970), (e->modBase + 0xC72C0), (e->modBase + 0x1041F8)};
    questInit(e, q, cb, "..\\Source\\D2Game/Quests/a2q3.cpp", 0x323, 0xb, 0xb, 4, 0x28);
    w32(q, 0xec, (e->modBase + 0xC2250)); w32(q, 0xf0, (e->modBase + 0xC21E0));
    uint8_t* p = (uint8_t*)e->ptr(r32(q, 0x18));
    w32(p, 8, 0); w16(p, 0x94, 0);
}

// 1.13c D2Game+0xC9400
extern "C" EMSCRIPTEN_KEEPALIVE
void d2_fn_d2game_C9400(void* q, const D2GameBatchC7130Env* e)
{
    const uint32_t cb[6] = {(e->modBase + 0xC4EA0), (e->modBase + 0xC85A0), (e->modBase + 0xC9190), (e->modBase + 0xC3620), (e->modBase + 0xC3320), (e->modBase + 0x1046A8)};
    w32(q, 0xa8, (e->modBase + 0xC5DB0));
    questInit(e, q, cb, "..\\Source\\D2Game/Quests/a2q4.cpp", 0x3dd, 0xc, 0xd, 5, 0x12);
    w16(q, 0x9c, 0); w32(q, 0xec, (e->modBase + 0xC2080)); w32(q, 0xf0, (e->modBase + 0xC69D0));
    uint8_t* p = (uint8_t*)e->ptr(r32(q, 0x18));
    p[0] = p[1] = p[2] = p[3] = 0; p[4] = 0;
}

// Seams of 0xC94E0, one per retail callee in call order (register and stack layout in the comments).
struct D2GameC94E0Env {
    void*    (*ptr)(uint32_t v);                                  // retail 32-bit pointer to host record
    void     (*fatal2d3)();                                       // Ordinal_10265(0x2d3), then Ordinal_10024, then _exit(-1)
    void     (*a4a2)(void* unit, uint32_t v);                     // 0xA4A2 (Ordinal_11090), 2 stack words
    void*    (*a520)(uint32_t classId);                           // 0xA520 (Ordinal_10688), 1 stack word
    void     (*ea80)(void* unit, uint32_t one, uint32_t val, uint32_t z0, uint32_t z1, uint32_t z2);   // 0x8EA80: EDX = unit, 5 stack words
    uint32_t (*c21100)(void* unit);                               // 0x1100: ECX = unit
    uint32_t (*c21130)(void* unit);                               // 0x1130: ECX = unit
    uint32_t (*a478)(void* unit);                                 // 0xA478 at 0xC961E, 1 stack word
    uint32_t (*a478b)(void* unit, uint32_t one);                  // 0xA478 at 0xC9790, 2 stack words
    uint32_t (*c21280)(uint32_t eaxRoom, uint32_t x, uint32_t y);                                         // 0x1280: EAX = A478's result, EBX = x, EDI = y
    void*    (*a6b8)(uint32_t pool, uint32_t two);                // 0xA6B8 (Ordinal_11153), 2 stack words
    uint32_t (*a556)(uint32_t v);                                 // 0xA556 (Ordinal_10826), 1 stack word
    uint8_t  (*a56e)(uint32_t v);                                 // 0xA56E (Ordinal_10459), 1 stack word
    void     (*e0110)(void* unit, void* gameD0);                  // 0xE0110: EDI = unit, ESI = game + 0xD0
    uint32_t (*a2e6)(uint32_t line, uint32_t z);                  // 0xA2E6 allocator (ECX = pool, EDX = 0x38)
    void     (*a724)(void* unit);                                 // 0xA724 (Ordinal_10445), 1 stack word
    void     (*c2f300)(void* unit, uint32_t id, uint32_t room, uint32_t x, uint32_t y);   // 0x2F300: ESI = unit, 4 stack words
    void     (*ce0f40)(uint32_t room, uint32_t y, void* unit, uint32_t x, void* game, uint32_t one);   // 0xE0F40: EAX, ECX, EDX, 3 stack words
    void     (*a51a1)(uint32_t v);                                // 0xA51A at 0xC979B, 1 stack word
    void     (*a51a2)(uint32_t v, uint32_t w);                    // 0xA51A at 0xC97A3, 2 stack words
};

// 1.13c D2Game+0xC94E0: ECX = game, EDX = quest. The quest state block is [quest + 0x18]; flag +0xD selects the pass, +6 and +4 the two
// unit-id lookups in the game's 128-bucket unit table (+0x1520, chained through +0xE4).
extern "C" EMSCRIPTEN_KEEPALIVE
uint32_t d2_fn_d2game_C94E0(void* game, void* quest, const D2GameC94E0Env* e)
{
    uint8_t* d = (uint8_t*)e->ptr(r32(quest, 0x18));
    uint32_t found = 0;
    if (d[0xd] == 1) {
        if (d[6]) {
            if (!game) e->fatal2d3();
            for (uint32_t p = r32(game, 0x1520 + (r32(d, 0x28) & 0x7f) * 4); p; p = r32(e->ptr(p), 0xe4)) {
                void* u = e->ptr(p);
                if (r32(u, 0xc) != r32(d, 0x28)) continue;
                found = 1;
                if (r32(u, 0x10) == 0) {
                    if (d[0xc] != 0) e->a4a2(u, 2);
                    else {
                        e->a4a2(u, 1);
                        void* c = e->a520(r32(u, 4));
                        e->ea80(u, 1, (r32(c, 0xdc) >> 8) + r32(e->ptr(r32(quest, 4)), 0xa8), 0, 0, 0);
                    }
                }
                break;
            }
        }
        if (d[4]) {
            if (!game) e->fatal2d3();
            for (uint32_t p = r32(game, 0x1520 + (r32(d, 0x20) & 0x7f) * 4); p; p = r32(e->ptr(p), 0xe4)) {
                void* hit = e->ptr(p);
                if (r32(hit, 0xc) != r32(d, 0x20)) continue;
                const uint32_t x = e->c21100(hit) - 0xd;
                const uint32_t y = e->c21130(hit) + 3;
                const uint32_t r0 = e->a478(hit);
                const uint32_t room = e->c21280(r0, x, y);
                if (!room) break;
                void* n = e->a6b8(r32(game, 0x1c), 2);
                if (!n) break;
                w32(n, 0, 2); w32(n, 4, 100); w32(n, 8, r32(game, 0x1c)); w32(n, 0x80, (uint32_t)(uintptr_t)game);
                uint32_t f = r32(n, 0xc8);
                f = r32(game, 0x70) == 0 ? (f & 0xfdffffffu) : (f | 0x2000000u);
                w32(n, 0xc8, f | 0x4000000u);
                const uint8_t b = e->a56e(e->a556(room));
                w8(n, 0x18, b); w32(n, 0x1c, r32(game, 0xbc + b * 4)); w32(n, 0x74, 0);
                e->e0110(n, (uint8_t*)game + 0xd0);
                w32(n, 0xc4, r32(n, 0xc4) | 0x10); w32(n, 0xd0, 0xb);
                uint32_t id = r32(game, 0x98) + 1; if (id == 0xffffffffu) id = 1; w32(game, 0x98, id);
                const uint32_t blk = e->a2e6(0x265, 0);
                w32(n, 0x14, blk); std::memset(e->ptr(blk), 0, 0x38);
                w32(e->ptr(blk), 0, (uint32_t)(uintptr_t)e->a520(100));
                w32(n, 0x10, 1);
                e->a724(n);
                e->c2f300(n, id, room, x, y);
                e->ce0f40(room, y, n, x, game, 1);
                found++;
                if (d[0xc] != 0) e->a4a2(n, 2);
                else {
                    e->a4a2(n, 1);
                    e->a51a1(e->a478b(hit, 1));
                    e->a51a2(room, 1);
                }
                break;
            }
        }
        if (!found) return 0;
        d[0xb] = 1; d[0xd] = 0;
    }
    d[3] = 0;
    return 1;
}

// Seams of 0xC9A80, one per retail callee in call order (register and stack layout in the comments).
struct D2GameC9A80Env {
    void*    (*ptr)(uint32_t v);                                  // retail 32-bit pointer to host record
    void     (*a4a2)(uint32_t unit, uint32_t v);                  // 0xA4A2 (Ordinal_11090), 2 stack words
    void*    (*a520)(uint32_t classId);                           // 0xA520 (Ordinal_10688), 1 stack word
    void     (*ea80)(uint32_t unit, uint32_t one, uint32_t val, uint32_t z0, uint32_t z1, uint32_t z2);   // 0x8EA80: EDX = unit, 5 stack words
    void     (*c34b80)(uint32_t unit, uint32_t eax, uint32_t v);  // 0x34B80: ECX = unit, EAX = p[2], 1 stack word
    uint32_t (*a478)(uint32_t unit);                              // 0xA478, 1 stack word
    void     (*c35eb0)(uint32_t eax, void* node);                 // 0x35EB0: EAX = 5, ESI = node
    void     (*dfcc0)(uint32_t cbRva, uint32_t edi);              // 0xDFCC0: EAX = 0, EBX = callback code RVA, EDI = node + 4 value, 1 stack word (0)
};

// 1.13c D2Game+0xC9A80: ECX = p { +0 game, +4 unit, +8 value, +0x10 class }. Pins the unit's reward, then finds the type-0xB quest node on the
// game's list (+0x10F4, chained through +0xF4) and records the unit's A478 result in the node's data block (+0x18, +8).
extern "C" EMSCRIPTEN_KEEPALIVE
uint32_t d2_fn_d2game_C9A80(void* p, const D2GameC9A80Env* e)
{
    void* game = e->ptr(r32(p, 0));
    const uint32_t unit = r32(p, 4);
    if (unit == 0 || r32(e->ptr(unit), 0x10) == 0) {
        e->a4a2(unit, 1);
        void* c = e->a520(r32(p, 0x10));
        e->ea80(unit, 1, (r32(c, 0xdc) >> 8) + r32(game, 0xa8), 0, 0, 0);
    }
    const uint32_t list = r32(game, 0x10f4);
    if (!list) return 0;
    uint32_t q = r32(e->ptr(list), 0);
    if (!q) return 0;
    while (r32(e->ptr(q), 0) != 0xb) { q = r32(e->ptr(q), 0xf4); if (!q) return 0; }
    void* qr = e->ptr(q);
    uint8_t* d = (uint8_t*)e->ptr(r32(qr, 0x18));
    e->c34b80(unit, r32(p, 8), 0x18c);
    w32(d, 8, e->a478(unit));
    if (r8(qr, 9) != 0 && r8(qr, 0xc) != 5) {
        e->c35eb0(5, qr);
        e->dfcc0(0xC5C60, r32(qr, 4));
        e->dfcc0(0xC3510, r32(qr, 4));
        e->dfcc0(0xC3190, r32(qr, 4));
    }
    return 0;
}

// old CA490 (superseded by d2game_ca490.cpp g_ca490)
extern "C" EMSCRIPTEN_KEEPALIVE
void d2_fn_d2game_ca490_old(const int32_t* unit, const int32_t* table, const D2GameBatchC7130Env* e)
{
    int32_t idx = 0;
    if (unit && unit[0] == 1) {
        const uint8_t* x = (const uint8_t*)e->ptr(unit[5]);
        if (unit[5]) { if (r8(x, 0x5c) & 2) return; idx = (int32_t)r32(x, 0x58); }
    }
    uint8_t* rec = (uint8_t*)e->ptr((uint32_t)table[idx]);
    if (e->ord10830() == 0) w32(rec, 0x2d0, r32(rec, 0x2d0) - 1);
}

// 1.13c D2Game+0xCBC70 (fastcall: ECX, EDX = hidden game words, stack: packet, size; ret 8)
extern "C" EMSCRIPTEN_KEEPALIVE
int32_t d2game_cbc70_port(uint32_t ecx, uint32_t edx, const void* p1, int32_t p2, const D2GameBatchC7130Env* e)
{
    if (p2 != 9) return 3;
    uint32_t v = r32(p1, 1), w = r32(p1, 5);
    if ((v >> 16) > 0xf) return 3;
    uint32_t n = v & 0x7fff;
    int32_t lim = (int32_t)r32(e->dataTables, 0xba0);
    if (lim < (int32_t)n) n = 0xffffffffu;
    else if (n != 0xffffffffu && (lim <= (int32_t)n || e->ord10708(edx, n, w) == 0)) return 3;
    return e->sub57890(edx, n, (v >> 15) & 1, w) != 0 ? 0 : 3;
}

// 1.13c D2Game+0xCBED0
extern "C" EMSCRIPTEN_KEEPALIVE
void d2_port_d2game_cbed0(const int32_t* eax, const D2GameBatchC7130Env* e, void (*sub8B400)(const uint8_t* pkt, uint32_t arg))
{
    uint8_t pkt[0x28] = {0};
    pkt[0] = 0x5a; pkt[1] = 0x0e; pkt[2] = 1;
    uint32_t arg = 0;
    if (eax && *eax == 0) arg = r32(e->ptr(e->ord10920(eax)), 0x9c);
    sub8B400(pkt, arg);
}
