// Alpha batch D2Game+0x6E400..0xA44C0 (12 functions), transcribed from the cached 1.13c decompiles
// (build-wasm/ghidra-d2game/decomp/0x<RVA>.c). Base 0x6FC20000. No test run.
// Same pattern as d2game_batch_1EF50.cpp: retail records are read by byte offset; every callee (Ordinal_N
// and FUN_6fc..) is a seam on D2GameBatch6E400Env in retail argument order, so no Ordinal signature is
// declared here and nothing can clash with the shared ORD() declarations. Ghidra register-only arguments
// (in_EAX / unaff_ESI / unaff_EBX / extraout_*) are explicit parameters, named as in the decompile.
// REG? marks a call whose register inputs the decompiler dropped.
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <emscripten.h>
#include "d2game_a3d60.cpp"

typedef uintptr_t A;  // a retail record address
static inline uint32_t rd32(A p, uint32_t o) { uint32_t v; std::memcpy(&v, (const uint8_t*)p + o, 4); return v; }
static inline uint16_t rd16(A p, uint32_t o) { uint16_t v; std::memcpy(&v, (const uint8_t*)p + o, 2); return v; }
static inline uint8_t  rd8 (A p, uint32_t o) { return ((const uint8_t*)p)[o]; }
static inline void wr32(A p, uint32_t o, uint32_t v) { std::memcpy((uint8_t*)p + o, &v, 4); }
static inline void wr8 (A p, uint32_t o, uint8_t v)  { ((uint8_t*)p)[o] = v; }
static inline A    rdA (A p, uint32_t o) { return (A)rd32(p, o); }

struct D2GameBatch6E400Env {
    A dataTables;                  // *sgptDataTables_exref
    const uint32_t* bitMasks;      // gdwBitMasks
    const char* assertFile;        // DAT_6FD19FF9
    const char* itemsFile;         // the __FILE__ string at 0x6FD1BD3C (ITEMS_CheckItemFlag in 0x70010)
    A handlerTable27BB8;           // 0x6FD27BB8: dword per +0x1B3 byte
    A fn8CF90;                     // &FUN_6fc8cf90
    // D2Common / Fog ordinals
    A    (*o10331)(A);  A (*o10265)(A);  void (*o10024)(const char*, A);  void (*o10920)(A);
    void (*o11090)(A, A);  A (*o10695)(A);  A (*o10688)(A);  A (*o10296)(A);  A (*o10826)(A);
    A    (*o10436)(A);  A (*o10373)(A);  A (*o10416)(A);  A (*o10459)(A, A, A, A, A);
    A    (*o10942)(A);  void (*o10141)(A);  A (*o11088)(A);  A (*o10083)(A);  void (*o10782)(A, A);
    A (*o10588)(A);  void (*o10467)(A, A);  void (*o10977)(A, A, A);  A (*o10202)(A, A, A, A);  A (*o10460)(A);
    A    (*o11147)(A);  A (*o10464)(A);  A (*o10020)(A);  A (*o10494)(A, A);  A (*o10871)(A);
    A    (*o10680)(A, A, A);  void (*o501)(A);  A (*o10802)(A);  A (*o10744)(A, A);  A (*o10106)(A);
    A    (*o10620)(A);  A (*o10366)(A);  A (*o10225)(A);  A (*o10838)(A);  A (*o10973)(A, A, A);
    A    (*o11151)(A);  A (*o10086)(A);  A (*o10213)(A, A, A);  A (*o10048)(A);  void (*o11165)(A);
    A    (*o10587)(A, A, A);  A (*o10379)(A, A, A);  void (*o11103)(A);  void (*o11005)(A);
    // D2Game FUN_
    void (*f6AE60)(A game);                                    // 0x6AE60
    void (*f8EA80)(A game, A unit, A, A, A, A, A);             // FUN_6fcaea80
    A    (*f89E50)(A byteArg);                                 // FUN_6fc89e50
    void (*fE9CB0)(A esi);                                     // FUN_6fce9cb0
    A    (*fD2C70)();                                          // FUN_6fcf2c70
    void (*fAAB20)();                                          // FUN_6fcaab20
    A    (*f210D0)(A id);                                      // FUN_6fc210d0 REG?
    void (*fAB400)(A esi, A eax);                              // FUN_6fcab400 REG?
    A    (*fB6600)();                                          // FUN_6fcb6600 REG?
    A    (*f21130)(A);  A (*f21100)(A);  A (*fD08F0)(A, A);    // 0x21130 / 0x21100 / 0xB08F0
    void (*f95E10)(A, A*);                                     // FUN_6fc95e10
    A    (*f21720)();  void (*f2B800)(A);  void (*fBE170)(A);  // 0x21720 / 0x2B800 / 0x9E170
    void (*f98550)(A, A, A, A, A, A, A);                       // FUN_6fc98550
    void (*fAA3E0)(const void*, A);                            // FUN_6fcaa3e0
};

static void fatal6E400(const D2GameBatch6E400Env* e, A size)
{
    e->o10024(e->assertFile, e->o10265(size));
    std::exit(-1);
}

// 1.13c D2Game+0x6E400
extern "C" EMSCRIPTEN_KEEPALIVE
void d2_fn_d2game_6E400(A game, A unit, const D2GameBatch6E400Env* e)
{
    if (unit && rd32(unit, 0x10) == 1) wr32(unit, 0x10, 2);
    e->o10331(unit);
    A seed = rdA(game, 0x10F0);
    e->f6AE60(game);
    uint64_t lo, hi;
    std::memcpy(&lo, (void*)seed, 8);
    (void)hi;
    uint64_t v = (uint64_t)(uint32_t)lo * 0x6ac690c5ull + rd32(seed, 4);
    std::memcpy((void*)seed, &v, 8);
    e->f8EA80(game, unit, 0, rd32(game, 0xA8) + 0xF + (uint32_t)((v & 0xffffffffull) % 0x23), 0, 0, 0);
}

static A chainFind(A game, uint32_t base, uint32_t id)
{
    for (A n = rdA(game, base + (id & 0x7F) * 4); n; n = rdA(n, 0xE4))
        if (rd32(n, 0xC) == id) return n;
    return 0;
}

// this=ECX (id of piVar6 list), param_1 game, param_2 id of piVar5 list, param_3 mode, eax = id for local_c.
// 1.13c D2Game+0x6E680
extern "C" EMSCRIPTEN_KEEPALIVE
uint32_t d2_fn_d2game_6E680(uint32_t thisId, A game, uint32_t param2, int16_t param3,
                            uint32_t eax, A esi, const D2GameBatch6E400Env* e)
{
    if (!game) fatal6E400(e, 0x2D3);
    A p5 = chainFind(game, 0x1520, param2);
    A p6 = chainFind(game, 0x1120, thisId);
    A lc = chainFind(game, 0x1720, eax);   // same 0xE4 chain, record id at +0xC
    if (!p6) {
        if (!p5) fatal6E400(e, 0x1109);
        return 1;
    }
    if (!p5) return 1;
    if (rd32(p6, 0) == 0) e->o10920(p6);
    if (param3 == 2) {
        e->fAAB20();
        e->o11090(p5, 0);
        wr32(p6, 0x64, (uint32_t)-1); wr32(p6, 0x68, 6); wr8(p6, 0x6C, 0);
        return 1;
    }
    if (param3 != 3) return 1;
    A rec = e->o10695(lc ? rd32(lc, 4) : 0xFFFFFFFFu);
    bool ok;
    if (rd32(p5, 4) == 0x98) ok = rd32(rec, 0x80) == 0x20747368;
    else ok = e->fD2C70() != 0;
    if (!ok) { e->fAAB20(); return 1; }
    wr32(p6, 0x64, (uint32_t)-1); wr32(p6, 0x68, 6); wr8(p6, 0x6C, 0);
    uint8_t c = (rd32(p5, 4) == 0x98) ? 1 : (uint8_t)e->f89E50(rd8(rec, 0x122));
    e->fAAB20();
    if (c == 1) {
        e->o11090(p5, 1);
        A r2 = e->o10688(rd32(p5, 4));
        if (rd32(p5, 4) == 0x98) {
            e->o11090(p5, 2);
            e->fE9CB0(esi);
            return 1;
        }
        e->f8EA80(lc, p5, 1, (rd32(r2, 0xDC) >> 8) + 1 + rd32(lc, 0xA8), 0, 0, 0);
        return 1;
    }
    e->o11090(p5, 0);
    return 1;
}

// Retail 0x6E930 (disassembled; the decompile is stack-garbled): a mid-function continuation entered by `call` with EAX = unit id, EBX = game,
// ESI = unit record, [esp+4] = a dword whose LOW WORD is the skill id (the rest is stack residue); `ret 4`; EBP/EDI are saved and not inputs.
// Callees (all stdcall, sizes as the disassembly pushes them): B156(node) record; A478(unit) owner; A556(owner) current skill;
// 1720(EAX = unit, [game]) client record; AB02(skill, &out) writes out; AAD2(g, out); B0B4(skill); 9E170(EAX = unit, game, flag);
// A56E(skill, skill, flag, &o1, &o2) returns AL = table index; B162(game[0xBC + 4 * idx]); A4C0(unit) record with +0x9C client; A544(unit, &xy) writes {x, y};
// 78550(ECX = game, EDX = unit, 0, 2, x, y, 0); 8A3E0(EAX = client, packet, 13); B800(EAX = game, ECX = unit, [owner2]).
namespace {
struct D2Game6E930Ops {
    A (*unitRec)(A node);                       // 0xB156 (Ordinal 10296)
    A (*owner)(A unit);                         // 0xA478 (10331)
    uint32_t (*curSkill)(A owner);              // 0xA556 (10826)
    A (*clientRec)(A unit, A game);             // 0x1720
    uint32_t (*skillOut)(uint32_t skill, uint32_t* out);  // 0xAB02 (10436)
    uint32_t (*check)(A g, uint32_t v);         // 0xAAD2 (10373)
    uint32_t (*skillClass)(uint32_t skill);     // 0xB0B4 (10416)
    void (*prep)(A unit, A game, uint32_t flag);          // 0x9E170
    uint32_t (*lookup)(uint32_t skill, uint32_t skill2, uint32_t flag, uint32_t* o1, uint32_t* o2);  // 0xA56E (10459)
    A (*target)(A p);                           // 0xB162 (10942)
    A (*clientOf)(A unit);                      // 0xA4C0 (10920)
    void (*pos)(A unit, uint32_t* xy);          // 0xA544 (10141)
    void (*cast)(A game, A unit, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);  // 0x78550
    void (*send)(A client, const void* buf, uint32_t len);   // 0x8A3E0
    void (*done)(A game, A unit, A owner);      // 0xB800
    void (*fatal)(uint32_t size);               // 0x2A2DA / 0x2A2C2 / 0x22A3C: asserts and exits
    A handlerTable;                             // module 0x107BB8: a dword per +0x1B3 byte
    uint32_t fn8CF90;                           // the table entry that selects this path
};
}
// 1.13c D2Game+0x6E930
extern "C" EMSCRIPTEN_KEEPALIVE
void d2_fn_d2game_6E930(uint32_t eax, A ebx, A esi, uint32_t skillWord, const D2Game6E930Ops* o)
{
    if (!ebx) o->fatal(0x2D3);
    A n = rdA(ebx, 0x1520 + (eax & 0x7F) * 4);
    while (n && rd32(n, 0xC) != eax) n = rdA(n, 0xE4);
    if (!n) return;
    A rec = o->unitRec(n);
    if (!rec) return;
    uint8_t b = rd8(rec, 0x1B3);
    if (b >= 0x65 || rd32(o->handlerTable, b * 4u) != o->fn8CF90) return;
    uint32_t sk = skillWord & 0xFFFF;
    uint32_t cur = o->curSkill(o->owner(n));
    wr32(esi, 0x64, (uint32_t)-1); wr32(esi, 0x68, 6); wr8(esi, 0x6C, 0);
    if (cur == sk || sk == 0) return;
    A g = o->clientRec(esi, ebx);
    uint32_t l1 = 0;
    if (!o->skillOut(sk, &l1)) return;
    if (!o->check(g, l1)) return;
    uint32_t flag = (o->skillClass(sk) != 0 || sk == 0x2E || sk == 0x4A || sk == 0x85 || sk == 0x86 || sk == 0x87 || sk == 0x88) ? 0xD : 0;
    o->prep(esi, ebx, flag);
    A u3 = o->owner(esi);
    if (u3) {
        uint32_t o1[1] = {0}, o2[1] = {0};
        uint32_t idx = o->lookup(sk, sk, flag, o1, o2) & 0xFF;
        if (u3 == o->target(rdA(ebx, 0xBC + idx * 4))) {
            A client = rd32(esi, 0) == 0 ? rdA(o->clientOf(esi), 0x9C) : 0;
            uint32_t uid = rd32(esi, 0xC);
            uint32_t xy[2] = {0, 0};
            o->pos(esi, xy);
            o->cast(ebx, esi, 0, 2, xy[0], xy[1], 0);
            uint8_t pk[13] = {};
            pk[0] = 0x0D; std::memcpy(pk + 2, &uid, 4); pk[6] = 1;
            uint16_t px = (uint16_t)(xy[0] + 3), py = (uint16_t)(xy[1] + 3);
            std::memcpy(pk + 7, &px, 2); std::memcpy(pk + 9, &py, 2);
            o->send(client, pk, 0xD);
        }
    }
    o->done(ebx, esi, u3);
}

// 1.13c D2Game+0x70010: copy the ear / personalised-name data of a source item onto its duplicate.
// dst = ESI (the duplicate), src = EDI (the original), bSet = the one stack argument (the hardcore flag of
// the owning client). o11088 = ITEMS_GetItemType, o10083/o10782 = ITEMS_Get/SetEarName, o10588/o10467 =
// ITEMS_Get/SetEarLevel, o10977 = ITEMS_SetItemFlag, o10202 = ITEMS_CheckItemFlag(item, flag, line, file).
extern "C" EMSCRIPTEN_KEEPALIVE
void d2game_legacy_70010(A dst, A src, A bSet, const D2GameBatch6E400Env* e)
{
    if (e->o11088(dst) == 7) {                          // ITEMTYPE_PLAYER_BODY_PART
        e->o10782(dst, e->o10083(src));
        e->o10467(dst, e->o10588(src));
        e->o10977(dst, 0x8000, bSet);                   // IFLAG_NAMED
        return;
    }
    if (e->o10202(dst, 0x1000000, 0x9aa, (A)e->itemsFile) != 0)   // IFLAG_PERSONALIZED
        e->o10782(dst, e->o10083(src));
}

// 1.13c D2Game+0x70190 moved to d2game_70190.cpp

// Quest/item requirement check against one 8-byte condition row at esi+0x14+idx*8.
// esi = condition table, iStack20 = unit's class record (+0x80 mask), iStack10 = required count,
// iStack8 = "exact count" flag (0 => at-least). Both stack values are caller-set (REG?).
// D2Game 0x70240 lives in d2game_70240.cpp (rewritten from the disassembly)

// 1.13c D2Game+0x70BF0 (layout from the disassembly; see the 70BF0 row).
// Packet-by-unit send. P = 40-byte stack block, zeroed: P[0] = 0x5a, P[1] = DL. When unit A (EAX) is non-null, ordinal 501 fills
// P+8 from A (3 stack words: P+8, A, 0x7FFFFFFF) and P[0x17] is cleared. Value = [10920(B)+0x9C] when B (ESI) is non-null and
// [B] == 0, else 0. Then 0x8B400 (EAX = P, one stack word = value, ret 4); the result of 0x8B400 is the result of 70BF0.
struct D2Game70BF0Env {
    void (*o501)(A buf, A unit, uint32_t limit);   // Ordinal 501, 0xA280
    A    (*o10920)(A unit);                        // Ordinal 10920, 0xA4C0: the client record
    uint32_t (*ab400)(A buf, uint32_t value);      // 0x8B400: the send, EAX = buffer, stack word 0 = value
};
extern "C" EMSCRIPTEN_KEEPALIVE
uint32_t d2_fn_d2game_70BF0(A eax, uint32_t edx, A esi, const D2Game70BF0Env* e)
{
    uint8_t pkt[0x28] = {};
    pkt[0] = 0x5a;
    pkt[1] = (uint8_t)edx;
    if (eax != 0) { e->o501((A)(pkt + 8), eax, 0x7fffffff); pkt[0x17] = 0; }
    uint32_t value = 0;
    if (esi && rd32(esi, 0) == 0) value = rd32(e->o10920(esi), 0x9c);
    return e->ab400((A)pkt, value);
}

// 1.13c D2Game+0x70BF0, the earlier transcription (kept for the batch's callers; not the proven port).
extern "C" EMSCRIPTEN_KEEPALIVE
void d2_fn_d2game_70BF0_old(A eax, A esi, const D2GameBatch6E400Env* e)
{
    uint32_t local[8] = {};
    if (eax != 0) { e->o501((A)local); local[5] &= 0xffffff; }
    if (esi && rd32(esi, 0) == 0) e->o10920(esi);
    e->fAB400(esi, eax);
}

// 1.13c D2Game+0x70CF0 (layout from the disassembly). EAX = unit (copied to ESI), EBX = condition table, stack = param_1, param_2 (ret 8).
// Both registers non-null: a zeroed 0xC0-byte local, then 0x70240 for slot 0..6 with stack (param_1, EBX, slot, param_2, &local) and ESI = the
// unit (ret 0x14); a zero result ends the loop with 0, seven non-zero results give 1.
struct D2Game70CF0Env {
    uint32_t (*f70240)(A esi, A param1, A table, int32_t slot, A param2, A local);
};
extern "C" EMSCRIPTEN_KEEPALIVE
uint32_t d2_fn_d2game_70CF0(A eax, A ebx, A param1, A param2, const D2Game70CF0Env* e)
{
    if (!ebx || !eax) return 0;
    uint32_t local[0x30] = {};
    for (int i = 0; i < 7; i++)
        if (e->f70240(eax, param1, ebx, i, param2, (A)local) == 0) return 0;
    return 1;
}

// ECX param1 = skill id, EDX param2 = unit, EAX = flag: the rule is d2game_a3d60.cpp (ga3d60::Allowed); this adapter runs it on the env.
struct A3D60Env {
    const D2GameBatch6E400Env* e;
    static uint32_t rd32(A p) { return ::rd32(p, 0); }
    static int32_t rd16s(A p) { return (int16_t)::rd16(p, 0); }
    static uint32_t rd8(A p) { return ::rd8(p, 0); }
    uint32_t h(uint32_t, uint32_t = 0, uint32_t = 0, uint32_t = 0, uint32_t = 0) { return (uint32_t)e->fB6600(); }
};
extern "C" EMSCRIPTEN_KEEPALIVE
bool d2_fn_d2game_A3D60(int32_t param1, A param2, A eax, const D2GameBatch6E400Env* e)
{
    A3D60Env env{ e };
    return ga3d60::Allowed(env, e->dataTables, param1, param2, (uint32_t)eax) != 0;
}

// 1.13c D2Game+0xA3E10
extern "C" EMSCRIPTEN_KEEPALIVE
bool d2_fn_d2game_A3E10(A param1, A param2, const D2GameBatch6E400Env* e)
{
    if (!param1) fatal6E400(e, 0x36C);
    if (!param2) fatal6E400(e, 0x36D);
    if ((uint8_t)rd32(param1, 0x18) != rd8(param2, 0x18)) return false;
    A y = e->f21130(param1), x = e->f21100(param1);
    return (uint32_t)e->fD08F0(x, y) < 0x3FC;
}

// ecx param_1 unused, param_2 (DL) = level byte, in_EAX = mode, esi = unit.
// 1.13c D2Game+0xA41B0
extern "C" EMSCRIPTEN_KEEPALIVE
void d2_fn_d2game_A41B0(A param1, uint8_t param2, int32_t eax, A esi, const D2GameBatch6E400Env* e)
{
    (void)param1;
    if (eax == 1) eax = 7;
    A rec = 0;
    if (esi && rd32(esi, 0) == 1 && rd32(esi, 0x14) != 0) rec = rdA(rd32(esi, 0x14), 0x2C);
    if (eax != 0) wr32(rec, 0x18, (uint32_t)eax);
    if (param2 != 0) {
        if (param2 > 0x4D) param2 = 0x4D;
        wr8(rec, 0x20, param2);
    }
    e->o11103(esi);                                     // REG?
    A st[2] = { rd32(esi, 0x2C), 1 };
    e->o11005(0);                                       // REG?
    e->f95E10(0, st);
}

// param_1 = skill id (ECX), in_EAX = struct (+8, +0x18), unaff_EBX = unit for 0xA3D60.
// 1.13c D2Game+0xA44C0
extern "C" EMSCRIPTEN_KEEPALIVE
uint32_t d2_fn_d2game_A44C0(int32_t param1, A eax, A ebx, const D2GameBatch6E400Env* e)
{
    A dt = e->dataTables;
    A unit8 = rdA(eax, 8);
    uint32_t i18 = rd32(eax, 0x18);
    if (!(param1 > -1 && param1 < (int32_t)rd32(dt, 0xBA0))) return 0;
    A row = (A)param1 * 0x23C + rdA(dt, 0xB98);
    if (!row || !d2_fn_d2game_A3D60(param1, ebx, eax, e)) return 0;
    uint8_t cls = rd8(row, 0x230);
    if (i18 == 0) { if (cls == 4 || cls == 0xD) return 0; }
    else if (cls != 4 && cls != 0xD) return 0;
    int16_t s80 = (int16_t)rd16(row, 0x80), s82 = (int16_t)rd16(row, 0x82);
    if (cls == 1 && s80 > 0 && e->o10494(unit8, s80) != 0) return 0;
    if (unit8 == 0 && (cls == 2 || cls == 4 || cls == 5 || cls == 0xB || cls == 0xC || cls == 0xD))
        return 0;
    if (cls == 2 && ((s80 > 0 && e->o10494(unit8, s80) != 0) ||
                     (s82 > 0 && e->o10494(unit8, s82) != 0))) return 0;
    if ((e->bitMasks[8] & rd8(row, 4)) && s80 > 0 && e->o10494(unit8, s80) != 0) {
        A r = e->o10871(unit8);
        if (r && (int32_t)e->o10680(r, (int16_t)rd16(row, 0x54), 0) >= 3) return 0;
    }
    return 1;
}
