// src/core/d2game/d2game_batch_D4630.cpp -- twelve 1.13c D2Game.dll functions transcribed from the
// cached Ghidra decompiles (build-wasm/ghidra-d2game/decomp/0x<RVA>.c). Base 0x6FC20000.
// Stage A batch lane (OBJECTIVE v8.3): implementation + marker only, no spec, no vector.
//
// Host model: retail records are opaque host objects addressed by their retail byte offsets
// (rd8/rd16/rd32/rdptr, wr32). Every callee src/ does not own is a seam on D2GameBatchD4630Env in
// retail argument order. Register-only arguments (Ghidra in_EAX, unaff_EDI, unaff_ESI, unaff_EBX,
// unaff_BL) and stack locals the decompile could not tie to a parameter are explicit parameters and
// are named at each function. The fatal path Fog #10265(id) -> #10024(text) -> _exit(-1) is modelled
// as d2game_fatal(id): only the exit is observable. IsBadCodePtr collapses to a null test (same as
// d2game_batch_03520.cpp). A `// 1.13c D2Game+0x<RVA>` marker precedes each definition.
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <unistd.h>
#include <emscripten.h>

static inline uint8_t  rd8 (const void* p, uint32_t o) { return ((const uint8_t*)p)[o]; }
static inline uint16_t rd16(const void* p, uint32_t o) { uint16_t v; std::memcpy(&v, (const uint8_t*)p + o, 2); return v; }
static inline uint32_t rd32(const void* p, uint32_t o) { uint32_t v; std::memcpy(&v, (const uint8_t*)p + o, 4); return v; }
static inline void     wr32(void* p, uint32_t o, uint32_t v) { std::memcpy((uint8_t*)p + o, &v, 4); }
static inline void*    rdptr(const void* p, uint32_t o) { void* v; std::memcpy(&v, (const uint8_t*)p + o, sizeof v); return v; }
// Retail addresses arrive as 32-bit integers in the decompile (in_EAX, unaff_EDI, ...).
static inline uint8_t  rd8 (uint32_t a, uint32_t o) { return rd8((const void*)(uintptr_t)a, o); }
static inline uint16_t rd16(uint32_t a, uint32_t o) { return rd16((const void*)(uintptr_t)a, o); }
static inline uint32_t rd32(uint32_t a, uint32_t o) { return rd32((const void*)(uintptr_t)a, o); }
static inline void     wr32(uint32_t a, uint32_t o, uint32_t v) { wr32((void*)(uintptr_t)a, o, v); }

[[noreturn]] static void d2game_fatal(uint32_t) { _exit(-1); }

#include "d2game_batch_D4630_env.h"

// D2_ASSERT helper: a null record is fatal 0x395; in_EAX zero is fatal 0x396; otherwise the record's
// first dword zero runs 0xD43E0 on it.
// 1.13c D2Game+0xD4630
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_D4630(void* param1, int32_t eaxValue,
                                             const D2GameBatchD4630Env* env)
{
    if (param1 == nullptr) d2game_fatal(0x395);
    if (eaxValue == 0) d2game_fatal(0x396);
    if (rd32(param1, 0) == 0) env->subD43E0(param1);
}

// Twin of 0xD4630 with fatal codes 0x24E (null record) / 0x24F (in_EAX zero).
// 1.13c D2Game+0xD4670
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_D4670(void* param1, int32_t eaxValue,
                                             const D2GameBatchD4630Env* env)
{
    if (param1 == nullptr) d2game_fatal(0x24E);
    if (eaxValue == 0) d2game_fatal(0x24F);
    if (rd32(param1, 0) == 0) env->subD43E0(param1);
}

// Two units from an index array in EAX: when param_6 is null, only an EBX-clear, param_4- and
// param_5-clear call decrements the first unit's +0x2CC. With a param_6, a differing pair stores
// param_2 at param_6[5]+0x58, and when EBX is clear an #10830 of param_6 that answers zero decrements
// the second unit's +0x2CC and, for param_6[0]==1 with the +0x16 bit set, its +0x2C8 as well.
// 1.13c D2Game+0xCA410
extern "C"
EMSCRIPTEN_KEEPALIVE void D2GAME_ReleaseUnitSlotRefs(uint32_t param1, int32_t param2, int32_t param3,
                                             int32_t param4, int32_t param5, void* param6,
                                             uint32_t eaxBase, int32_t ebxValue,
                                             const D2GameBatchD4630Env* env)
{
    (void)param1;

    const uint32_t iVar3 = rd32(eaxBase, (uint32_t)param2 * 4);
    const uint32_t iVar2 = rd32(eaxBase, (uint32_t)param3 * 4);

    if (param6 == nullptr) {
        if (ebxValue == 0 && param5 == 0 && param4 == 0) {
            wr32(iVar3, 0x2CC, rd32(iVar3, 0x2CC) - 1);
        }
        return;
    }

    if (iVar2 != iVar3 && rd32(param6, 0) == 1 && rd32(param6, 0x14) != 0) {
        wr32(rd32(param6, 0x14), 0x58, (uint32_t)param2);
    }
    if (ebxValue == 0) {
        if (env->ordinal10830(param6) == 0) {
            wr32(iVar2, 0x2CC, rd32(iVar2, 0x2CC) - 1);
            if (rd32(param6, 0) == 1 && rd32(param6, 0x14) != 0 &&
                (rd8(rd32(param6, 0x14), 0x16) & 1) != 0) {
                wr32(iVar2, 0x2C8, rd32(iVar2, 0x2C8) - 1);
            }
        }
    }
}

// Two action sends (0x3 and 0xB) after a registered callback: ESI zero is fatal 0x804. When the EDI
// unit's +0x60 list answers non-zero to #11017 the callback is skipped, otherwise it fires with
// (1, stack4, stack8); a null callback pointer is fatal 0x811.
// 1.13c D2Game+0x37E20
extern "C"
EMSCRIPTEN_KEEPALIVE void D2GAME_SUnitArmTimers_37E20(uint32_t esiGame, uint32_t ediUnit,
                                             uint32_t stack4, uint32_t stack8,
                                             const D2GameBatchD4630Env* env)
{
    if (esiGame == 0) d2game_fatal(0x804);

    if (rd32(ediUnit, 0x60) == 0 || env->ordinal11017(rd32(ediUnit, 0x60)) == 0) {
        if (env->fn6FD1A400 == nullptr) d2game_fatal(0x811);
        env->fn6FD1A400(1, stack4, stack8);
    }

    env->subAEA80(esiGame, ediUnit, 3, (int32_t)rd32(esiGame, 0xA8) + 1, 0, 0, 0);
    env->subAEA80(esiGame, ediUnit, 0xB, (int32_t)rd32(esiGame, 0xA8) + 0xFA, 0, 0, 0);
}

// Action picker (thiscall, three stack args after ECX=this): this+8 below this+0xC times 10 skips the call;
// the 64-bit LCG (constant 0x6AC690C5) lives behind param1+0x10F0 and every modulo uses its LOW dword only.
// A roll below threshold+0xC (threshold: +5 when this+8/2 < this+4, +5 when this+8 - this+8/4 < this+4)
// picks one of the action ids bit-selected from param3's low seven bits (ids array at esp+0x10, payload
// from #10561(param2) at esp+0x2C) and calls the row's callback at 0x6FD2E300 with ECX=param1, EDX=param2
// and the payload on the stack. Disassembly-verified (Ghidra mixes up the third argument and the frame).
// 1.13c D2Game+0x2EAD0
extern "C"
EMSCRIPTEN_KEEPALIVE uint32_t GAME_RandomActionRoll_2EAD0(void* thisp, uint32_t param1, uint32_t param2,
                                                  uint32_t param3, const D2GameBatchD4630Env* env)
{
    if ((int32_t)rd32(thisp, 8) < (int32_t)rd32(thisp, 0xC) * 10) return 0;

    const uint32_t seedPtr = rd32(param1, 0x10F0);

    uint64_t state = (uint64_t)rd32(seedPtr, 0) * 0x6AC690C5ULL + (uint64_t)rd32(seedPtr, 4);
    wr32(seedPtr, 0, (uint32_t)state);
    wr32(seedPtr, 4, (uint32_t)(state >> 32));

    int32_t chance = 0;
    if ((int32_t)rd32(thisp, 8) / 2 < (int32_t)rd32(thisp, 4)) chance = 5;
    const int32_t size = (int32_t)rd32(thisp, 8);
    if (size - size / 4 < (int32_t)rd32(thisp, 4)) chance += 5;

    if ((uint32_t)state % 100u < (uint32_t)(chance + 0xC)) {
        uint8_t payload[32] = {};
        env->ordinal10561(param2, payload);

        int32_t ids[7] = {};
        int32_t count = (param3 & 1) != 0 ? 1 : 0;
        if (count) ids[0] = 1;
        if (param3 & 2) ids[count++] = 2;
        if (param3 & 4) ids[count++] = 3;
        if (param3 & 8) ids[count++] = 4;
        if (param3 & 0x10) ids[count++] = 5;
        if (param3 & 0x20) ids[count++] = 6;
        if (param3 & 0x40) ids[count++] = 7;

        int32_t pick;
        if (count <= 0) {
            pick = 0;
        }
        else {
            state = (uint64_t)rd32(seedPtr, 0) * 0x6AC690C5ULL + (uint64_t)rd32(seedPtr, 4);
            wr32(seedPtr, 0, (uint32_t)state);
            wr32(seedPtr, 4, (uint32_t)(state >> 32));
            if (((uint32_t)count & ((uint32_t)count - 1)) == 0) pick = (int32_t)(((uint32_t)count - 1) & (uint32_t)state);
            else pick = (int32_t)((uint32_t)state % (uint32_t)count);
        }

        const int32_t action = ids[pick];
        if (pick < count && action < count &&
            rd32(env->actionTable, (uint32_t)action * 8) != 0 &&
            rd32(env->actionTable, (uint32_t)action * 8 + 4) != 0) {
            uint32_t (*fn)(uint32_t, uint32_t, void*) =
                (uint32_t (*)(uint32_t, uint32_t, void*))(uintptr_t)rd32(env->actionTable, (uint32_t)action * 8 + 4);
            return fn(param1, param2, payload);
        }
    }
    return 0;
}

// Skill dispatch walk: a null EAX unit is fatal 0xA2A. With #10920's record, the unit's +0x80 block
// and +0x0C value, the walk runs over the data-tables count at +0xBF0: each row's +0x44 list of 0xC-byte
// entries leads to a +0x14-linked chain of nodes whose flag bit 0 is clear and whose +0x04 id is hashed
// into the +0x1320 table (bucket id & 0x7F, chain +0xE4) to find the matching +0xC id; index 7 takes
// 0xAB220 with the eight arguments shown, every other index 0xAB290.
// 1.13c D2Game+0x96920
extern "C"
EMSCRIPTEN_KEEPALIVE void d2game_legacy_96920(uint32_t param1, uint32_t eaxUnit,
                                             const D2GameBatchD4630Env* env)
{
    const uint32_t owner = env->ordinal10920();
    if (eaxUnit == 0) d2game_fatal(0xA2A);

    const uint32_t block = rd32(eaxUnit, 0x80);
    const uint32_t value = rd32(eaxUnit, 0xC);

    if (owner == 0 || block == 0) return;
    const uint32_t list = rd32(owner, 0x44);
    if (list == 0) return;

    const int32_t count = (int32_t)rd32(env->dataTables, 0xBF0);
    for (int32_t i = 0; i < count; ++i) {
        const uint32_t entry = rd32(list, (uint32_t)i * 0xC);
        if (entry == 0) continue;
        for (uint32_t node = rd32(entry, 0); node != 0; node = rd32(node, 0x14)) {
            if ((rd8(node, 0) & 1) != 0) continue;
            const uint32_t id = rd32(node, 4);
            for (uint32_t hit = rd32(block, 0x1320 + (id & 0x7F) * 4); hit != 0; hit = rd32(hit, 0xE4)) {
                if (rd32(hit, 0xC) != id) continue;
                const uint32_t word = rd32(hit, 4) & 0xFFFF;
                if (i == 7) {
                    env->subAB220(rd32(node, 8), id, param1, 7, value, word, rd32(node, 8),
                                  rd32(node, 0xC));
                }
                else {
                    env->subAB290(param1, (int32_t)(int8_t)i, id, value, word);
                }
                break;
            }
        }
    }
}

// 0xB2760 (EAX = a, stack word = b, ret 4): #10920(a)'s record +0x34 must be non-null (fatal 0xE5);
// the 7-byte packet 0x65, id = a ? [a+0xC] : -1 (unaligned dword at +1), word [[+0x34]] at +5 goes to
// 0x8A3E0 with EAX = b.
// 1.13c D2Game+0xB2760
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_B2760(uint32_t a, uint32_t b, const D2GameBatchD4630Env* env)
{
    const uint32_t owner = env->ordinal10920Unit(a);
    const uint32_t rec = rd32(owner, 0x34);
    if (rec == 0) d2game_fatal(0xE5);
    const uint32_t id = a == 0 ? 0xFFFFFFFFu : rd32(a, 0xC);
    uint8_t pkt[7];
    pkt[0] = 0x65;
    std::memcpy(pkt + 1, &id, 4);
    std::memcpy(pkt + 5, (const void*)(uintptr_t)rec, 2);
    env->sub8A3E0c(b, pkt, 7);
}

// Per-unit walk from param_1+0x88: each unit's +0x3D4 bit 0 selects between an existing +0x174 record
// and a fresh 0xE03A0 resolution (clearing +0x174 when that fails). A type-4 unit with a record is
// handed to 0xB2760 when param_3 is set, or when param_3 is clear and it differs from param_2's
// resolved record with its owner (+0x34) +0x04 non-zero (owner null is fatal 0xDB). param_2 null is
// fatal 0xFB.
// 1.13c D2Game+0xB2C20
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_B2C20(uint32_t param1, uint32_t param2, uint32_t param3,
                                             const D2GameBatchD4630Env* env)
{
    if (param2 == 0) d2game_fatal(0xFB);

    uint32_t resolved;
    if ((rd8(param2, 0x3D4) & 1) == 0) {
        if (rd32(param2, 0x174) == 0) {
            resolved = 0;
        }
        else {
            resolved = env->subE03A0x(rd32(param2, 0x16C), rd32(param2, 0x1A8), rd32(param2, 0x170));
            if (resolved == 0) wr32(param2, 0x174, 0);
        }
    }
    else {
        resolved = rd32(param2, 0x174);
    }

    for (uint32_t unit = rd32(param1, 0x88); unit != 0; unit = rd32(unit, 0x4A8)) {
        uint32_t rec;
        if ((rd8(unit, 0x3D4) & 1) == 0) {
            if (rd32(unit, 0x174) == 0) {
                rec = 0;
            }
            else {
                rec = env->subE03A0x(rd32(unit, 0x16C), rd32(unit, 0x1A8), rd32(unit, 0x170));
                if (rec == 0) wr32(unit, 0x174, 0);
            }
        }
        else {
            rec = rd32(unit, 0x174);
        }

        if (rd32(unit, 4) != 4 || rec == 0) continue;

        bool apply = false;
        if (param3 != 0) {
            apply = true;
        }
        else if (rec != resolved) {
            const uint32_t owner = env->ordinal10920Unit(rec);
            if (rd32(owner, 0x34) == 0) d2game_fatal(0xDB);
            apply = rd32(rd32(owner, 0x34), 4) != 0;
        }
        if (apply) {
            d2_fn_d2game_B2760(rec, param2, env);
        }
    }
}

// The 0x788B0 common tail: the player's unit record is resolved from +0x174 or 0xE03A0 (clearing the
// field on failure). With an in_EAX unit: +0xC8 bit 16 runs 0xCBB10 with 1, otherwise bit 11 with 0
// runs it unless the unit is the resolved record. param_2 zero and the unit's +0xC8 bit 0 run
// 0xCBBB10/0xCAEC0/0xCAE60. The unit's +0xC4 bit 0 runs 0xEB8F0, then 0x9ADF0(this, unit, player);
// bits 10, 8, 15 of +0xC4 run 0x9AC50, 0x9ACB0(player), 0xEBBB0; #11149 -> 0x9A5E0 and #10354 ->
// 0x9A570; #10771 five times. The decompile's extraout_ECX values are call-clobbered ECX; the `this`
// object is passed through as the callers' thiscall receiver. param_1 null is fatal 0xFB.
// 1.13c D2Game+0x788b0
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_788B0(void* thisp, void* param1, uint32_t param2,
                                             void* eaxUnit, const D2GameBatchD4630Env* env)
{
    if (param1 == nullptr) d2game_fatal(0xFB);

    const uint32_t player = (uint32_t)(uintptr_t)param1;
    uint32_t piVar3;
    if ((rd8(player, 0x3D4) & 1) == 0) {
        if (rd32(player, 0x174) == 0) {
            piVar3 = 0;
        }
        else {
            piVar3 = env->subE03A0(rd32(player, 0x1A8), rd32(player, 0x170));
            if (piVar3 == 0) wr32(player, 0x174, 0);
        }
    }
    else {
        piVar3 = rd32(player, 0x174);
    }

    const uint32_t unit = (uint32_t)(uintptr_t)eaxUnit;
    if (eaxUnit != nullptr) {
        const uint32_t flags = rd32(unit, 0xC8);
        if (((flags >> 0x10) & 1) == 0) {
            if (((flags >> 0xB) & 1) != 0 && unit != piVar3) env->subCBB10(player, 0);
        }
        else {
            env->subCBB10(player, 1);
        }
    }

    if (param2 == 0 && eaxUnit != nullptr && (rd32(unit, 0xC8) & 1) != 0) {
        env->subCBBB10(nullptr, eaxUnit);
        env->subCAEC0();
        env->subCAE60(player, 0);
    }

    if (eaxUnit != nullptr && (rd32(unit, 0xC4) & 1) != 0) {
        env->subEB8F0(thisp, player);
    }

    env->sub9ADF0(thisp, eaxUnit, player);
    if (eaxUnit != nullptr) {
        const uint32_t flags = rd32(unit, 0xC4);
        if (((flags >> 0xA) & 1) != 0) env->sub9AC50();
        if (((flags >> 8) & 1) != 0) env->sub9ACB0((int32_t)player);
        if (((flags >> 0xF) & 1) != 0) env->subEBbb0();
    }

    if (env->ordinal11149() != 0) env->sub9A5E0(eaxUnit, (int32_t)player);
    if (env->ordinal10354() != 0) env->sub9A570();

    env->ordinal10771();
    env->ordinal10771();
    env->ordinal10771();
    env->ordinal10771();
    env->ordinal10771();
}

// The last 0x9ADF0 tail: 0x8A3E0 with blob 0x1A for 0x8A3E0's mode, gated by a unit mode lookup --
// BL 0 -> 6, 1 -> 0xB, 2/4 -> 7, anything else returns. The +0x6D byte of the retaddr record selects
// the entry. The unit's first dword zero re-runs #10920 (result unused). The 26-byte blob at
// stack0xFFFFFFC4 is an explicit parameter.
// 1.13c D2Game+0x35B60
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_35B60(uint32_t eaxUnit, uint32_t blMode, uint32_t retaddr,
                                             const void* src, const D2GameBatchD4630Env* env)
{
    const uint32_t owner = env->ordinal10920();

    int32_t mode;
    switch (blMode) {
    case 0: mode = 6; break;
    case 1: mode = 0xB; break;
    case 2: case 4: mode = 7; break;
    default: return;
    }

    const int32_t found = env->sub34C70((int32_t)rd32(owner, 0x60 + rd8(retaddr, 0x6D) * 4), mode);
    if (found == 0) return;

    if (eaxUnit != 0 && rd32(eaxUnit, 0) == 0) env->ordinal10920();

    uint8_t blob[0x1A];
    std::memcpy(blob, src, sizeof blob);
    env->sub8A3E0(blob, 0x1A);
}

// List walk: EAX zero is fatal 0x44; from the unit's +0x1C node, each node's +0xE0 link is taken and
// 0x9DCD0(param_1) runs per node.
// 1.13c D2Game+0x9de10
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_9DE10(void* param1, uint32_t eaxUnit,
                                             const D2GameBatchD4630Env* env)
{
    if (eaxUnit == 0) d2game_fatal(0x44);
    for (uint32_t node = rd32(eaxUnit, 0x1C); node != 0; node = rd32(node, 0xE0)) {
        env->sub9DCD0(param1);
    }
}

// Per-unit update dispatch; EAX = in_EAX (client/context), EDI = unit (null is fatal 0x15). A unit with
// flag bit 4 (+0xC4 bit 4) that is not 0x4AD10(0)'s player: type 3 returns, else 0x7B0A0 and flag=1.
// Then by unit type: 0 -> 0x788B0, 1 -> 0xF3A40, 2 -> 0x6B200, 4 -> 0xD4550.
// 1.13c D2Game+0x9dcd0
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_9DCD0(void* param1, uint32_t eax, uint32_t unit,
                                             const D2GameBatchD4630Env* env)
{
    if (unit == 0) d2game_fatal(0x15);
    const uint32_t type = rd32(unit, 0);
    int32_t flag = 0;
    if ((rd32(unit, 0xC4) >> 4 & 1) != 0 && (uint32_t)(uintptr_t)env->sub4AD10(0) != unit) {
        if (type == 3) return;
        env->sub7B0A0(param1, unit);
        flag = 1;
    }
    switch (rd32(unit, 0)) {
    case 0: env->sub788B0(param1, eax, flag); break;
    case 1: env->subF3A40(param1, unit, eax, flag); break;
    case 2: env->sub6B200(eax); break;
    case 4: env->subD4550(); break;
    }
}

