// src/core/d2game/d2game_batch_8DB00.cpp -- eleven 1.13c D2Game.dll functions transcribed from the
// cached Ghidra decompiles (build-wasm/ghidra-d2game/decomp/0x<RVA>.c), in RVA order: the two
// player-mode packet writers (0x8DB00, 0x8DC30), the item-flag probe (0x486D0), the game-table
// lookup (0x8FAA0), the two client stat packets (0xF3360, 0xF3410), the PlrModes skill pair
// (0x778A0, 0x77B20) with its thrown/stackable weapon pass (0x78DD0), the mode->byte table
// (0xAF5F0) and the item-skill toggle (0x37EC0). Alpha implementation-batch lane (OBJECTIVE v9):
// implementation + marker only, no spec, no vector.
//
// Host model: a unit, game, player data, client, item or skill block is opaque and addressed by
// retail 32-bit byte offsets (rd8/rd16/rd32/rdptr, wr8/wr16/wr32). Register-only arguments
// (Ghidra's in_EAX/in_ECX/unaff_EBX/unaff_ESI/unaff_EDI) are explicit parameters here and are named
// in each function's comment, in the order retail's call sites set them. Every callee outside this
// batch is a seam pointer on D2GameBatch8DB00Env in retail argument order (the register/stack split
// of a retail call site is written on the seam field). Fog/D2Common ordinals are the 1.13c numbers
// from the decompiles and stay opaque; the fatal path #10265(id) -> #10024(text) -> _exit(-1) is
// d2game_fatal, as in the other batches. The import-thunk map used to name them
// (src/core/fn/import_thunk.cpp) gives #10010=0xB036, #10048=0xA940, #10202=0xA8A4,
// #10304=0xA4BA, #10417=0xA97C, #10471=0xAB5C, #10507=0xA3B2, #10516=0xAFDC, #10601=0xA49C,
// #10708=0xA3D0, #10711=0xA9AC, #10744=0xA430, #10750=0xA3B8, #10787=0xA8C8, #10828=0xA3AC,
// #10867=0xA454, #10887=0xA472, #10892=0xA916, #10905=0xB02A, #10910=0xA802, #10913=0xA3A0,
// #10973=0xA42A, #10976=0xAAAE, #11001=0xAB32, #11054=0xB024, #11058=0xAB4A, #11088=0xAA66,
// #11090=0xA4A2, #11139=0xA7AE, #10265=0xA2DA.
// Prior art: D2MOO 1.10f reference/D2MOO/source/D2Game/src/PLAYER/PlrModes.cpp
// (sub_6FC81250/sub_6FC80B90, the 1.10f forms of 0x79100's skill pair and weapon pass),
// src/core/d2moo/src/D2Game/PLAYER/PlrModes.cpp and the D2Common ordinal table.
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <unistd.h>
#include <emscripten.h>

static inline uint8_t  rd8 (const void* p, uint32_t o) { return ((const uint8_t*)p)[o]; }
static inline uint16_t rd16(const void* p, uint32_t o) { uint16_t v; std::memcpy(&v, (const uint8_t*)p + o, sizeof v); return v; }
static inline uint32_t rd32(const void* p, uint32_t o) { uint32_t v; std::memcpy(&v, (const uint8_t*)p + o, sizeof v); return v; }
static inline void*    rdptr(const void* p, uint32_t o) { void* v; std::memcpy(&v, (const uint8_t*)p + o, sizeof v); return v; }
static inline void wr8 (void* p, uint32_t o, uint8_t  v) { ((uint8_t*)p)[o] = v; }
static inline void wr16(void* p, uint32_t o, uint16_t v) { std::memcpy((uint8_t*)p + o, &v, sizeof v); }
static inline void wr32(void* p, uint32_t o, uint32_t v) { std::memcpy((uint8_t*)p + o, &v, sizeof v); }

[[noreturn]] static void d2game_fatal(uint32_t) { _exit(-1); }

// Host seams for the callees the cached decompiles name but src/ does not implement. The comment on
// each field is the retail callee (D2Game RVA or D2Common ordinal) and the register/stack split at
// the call sites in this batch.
struct D2GameBatch8DB00Env {
    // DAT_6FD1B614: the nine-entry mode->byte table read by 0xAF5F0.
    const int32_t* modeTable;

    // 0x8DB00 / 0x8DC30 / 0x78DD0 -- the packet tail.
    void     (*sendPacket)(uint32_t client, const void* packet, uint32_t length); // 0x8A3E0 (client in EAX)
    uint8_t  (*ordinal10471)(uint32_t path);              // #10471 path -> mode byte
    uint16_t (*ordinal10976)(uint32_t path);              // #10976 path -> word
    uint16_t (*ordinal11001)(uint32_t path);              // #11001 path -> word
    int32_t  (*ordinal10973)(uint32_t unit, uint32_t stat, uint32_t layer); // #10973
    uint8_t  (*ordinal11058)(uint32_t path);              // #11058 path -> byte
    int32_t  (*ordinal10867)(uint32_t path);              // #10867 path -> int
    int32_t  (*ordinal10750)(uint32_t path);              // #10750 path -> int

    // 0x8FAA0
    uint32_t (*sub8F5C0)(const uint32_t* id);             // 0x8F5C0 (arg by pointer)

    // 0xF3360
    int32_t  (*ordinal10913)(uint32_t unit);              // #10913
    void     (*ordinal10887)(uint32_t unit, uint32_t stat, uint32_t value, uint32_t layer); // #10887

    // 0x778A0
    void*    (*ordinal10828)(uint32_t unit);              // #10828 left skill
    void*    (*ordinal10507)(uint32_t unit);              // #10507 right skill
    uint32_t (*ordinal10304)(const void* skill);          // #10304 owner handle
    uint32_t (*ordinal10601)(const void* skill, const char* file, uint32_t line); // #10601 skill id

    // 0x77B20
    void*    (*ordinal10708)(uint32_t ownerGuid);         // #10708 handle -> skill
    int32_t  (*ordinal10787)(uint32_t unit, const void* skill); // #10787
    void     (*subA2100)(uint32_t ecxUnit, uint32_t edxSide, uint32_t skillId, uint32_t owner); // 0xA2100

    // 0x78DD0
    uint32_t (*ordinal10048)(uint32_t unit);              // #10048
    uint32_t (*ordinal10711)(uint32_t unit);              // #10711
    int32_t  (*ordinal10910)(uint32_t unit, uint32_t a, uint32_t b); // #10910
    int32_t  (*ordinal10744)(uint32_t unit);              // #10744
    int32_t  (*ordinal10516)(uint32_t unit);              // #10516
    int32_t  (*ordinal10202)(uint32_t unit, uint32_t a, uint32_t line, const char* file); // #10202
    uint32_t (*ordinal10905)(uint32_t unit);              // #10905
    uint32_t (*ordinal11088)(uint32_t unit);              // #11088
    uint8_t  (*ordinal10417)(uint32_t unit);              // #10417
    uint32_t (*ordinal11054)(uint32_t unit);              // #11054
    void     (*ordinal11090)(uint32_t unit, uint32_t value); // #11090
    uint32_t (*ordinal10010)(uint32_t handle, uint32_t arg, uint32_t prev); // #10010 inventory walk
    int32_t  (*ordinal11139)(uint32_t handle, uint32_t n);  // #11139
    void     (*ordinal10892)(uint32_t unit, uint32_t a, uint32_t b); // #10892
    // 0xD5740: __fastcall ECX=unit, EDX=arg1-of-0x78DD0, EAX=unit.
    void     (*subD5740)(uint32_t ecxUnit, uint32_t edxCtx);
    // 0xD8970: __fastcall ECX=arg1-of-0x78DD0, EDX=unit, EAX=out pointer; three stack args
    // (scratch word, 1, out pointer).
    uint32_t (*subD8970)(uint32_t ecxCtx, uint32_t edxUnit, uint32_t eaxOutPtr,
                         uint32_t a1, uint32_t a2, int32_t* outFlag);
    // 0xD57F0: __fastcall ECX=arg1-of-0x78DD0, EAX=unit; three stack args (ctx, item id, scratch).
    int32_t  (*subD57F0)(uint32_t ecxCtx, uint32_t eaxUnit, uint32_t a1, uint32_t a2, uint32_t a3);
    // 0x98B60: EAX=(#11054 low byte replaced by the scratch byte); stack args (ctx, unit).
    void     (*sub98B60)(uint32_t eaxValue, uint32_t ctx, uint32_t unit);

    // 0x37EC0
    uint32_t (*sub1100)(uint32_t ecxUnit);                // 0x1100 unit coordinate accessor
    // 0xD4E80: __fastcall ECX=player, EDX=arg1-of-0x37EC0, EAX=unit id; stack args
    // (coordinate value, arg1-of-0x37EC0, out flag pointer). Returns nonzero on success.
    int32_t  (*subD4E80)(uint32_t ecxPlayer, uint32_t edxArg1, uint32_t eaxUnitId,
                         uint32_t value, uint32_t a1, int32_t* outFlag);
};

// 1.13c D2Game+0x8DB00
// Player-mode packet, 16 bytes. EAX = unit id, EBX = client, DL = packet[0], stack args a1 (byte),
// a2 (word), a3 (word), a4 (byte). Walks the type-1 unit list at game+0x1320, then fills the packet
// from the unit's path block (unit+0x2C) and the layer-0x43 stat of the unit.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2game_8db00_impl(uint32_t eaxUnitId, uint32_t ebxClient, uint8_t dl,
                                             uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4,
                                             const D2GameBatch8DB00Env* env)
{
    if (ebxClient == 0) d2game_fatal(0x996);
    const uint32_t game = rd32((void*)(uintptr_t)ebxClient, 0x1A8);
    if (game == 0) d2game_fatal(0x2D3);

    uint8_t pkt[16] = {0};
    pkt[0] = dl;
    wr32(pkt, 1, eaxUnitId);
    pkt[5] = (uint8_t)a1;
    wr16(pkt, 6, (uint16_t)a2);
    wr16(pkt, 8, (uint16_t)a3);

    void* node = (void*)(uintptr_t)rd32((void*)(uintptr_t)game, 0x1320 + (eaxUnitId & 0x7F) * 4);
    if (node == nullptr) return;
    while (rd32(node, 0x0C) != eaxUnitId) {
        node = rdptr(node, 0xE4);
        if (node == nullptr) return;
    }

    const uint32_t path = rd32(node, 0x2C);
    pkt[0x0A] = (uint8_t)a4;
    const uint8_t mode = env->ordinal10471(path);
    pkt[0x0C] = mode;
    if (mode >= 5) {
        if (mode <= 6) {
            pkt[0x0C] = 1;
            wr16(pkt, 6, env->ordinal10976(path));
            wr16(pkt, 8, env->ordinal11001(path));
        } else if (mode == 8) {
            pkt[0x0C] = 0x0B;
        }
    }
    int32_t v = env->ordinal10973((uint32_t)(uintptr_t)node, 0x43, 0);
    if (v < -0x8000) v = -0x8000;
    if (v > 0x7FFF) v = 0x7FFF;
    wr16(pkt, 0x0D, (uint16_t)v);
    pkt[0x0F] = env->ordinal11058(path);
    env->sendPacket(ebxClient, pkt, 16);
}

// 1.13c D2Game+0x486D0
// EAX = unit, ECX = id. Type-1 units with a player-data block at unit+0x14 expose a list at
// playerData+0x30; the function returns whether the entry whose first dword equals ECX has a value
// of 2 in its second dword. The caller guarantees neither block, so a null path faults in retail.
extern "C"
EMSCRIPTEN_KEEPALIVE uint8_t d2_fn_d2game_486D0(uint32_t ecxId, uint32_t eaxUnit)
{
    const void* list;
    if (eaxUnit == 0 || rd32((void*)(uintptr_t)eaxUnit, 0) != 1) {
        list = nullptr;
    } else {
        const uint32_t playerData = rd32((void*)(uintptr_t)eaxUnit, 0x14);
        list = (playerData == 0) ? nullptr : rdptr((void*)(uintptr_t)playerData, 0x30);
    }
    void* entry = nullptr;
    if (list != nullptr) entry = (void*)(uintptr_t)rd32(list, 0);
    while (entry != nullptr) {
        if (rd32(entry, 0) == ecxId) return rd32(entry, 4) == 2 ? 1 : 0;
        entry = rdptr(entry, 8);
    }
    return 0;
}

// 1.13c D2Game+0xF3360
// EAX = unit, stack arg = client. Packet 0x13 (9 bytes): unit id, the unit byte at +0xB0, and a 0..0x80
// ratio of the layer-6 stat over the unit's #10913 value, decremented when above 1 and flagged
// with 0x80 for a type-1 unit whose player-data flag at +0x17 is set.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2game_f3360_impl(uint32_t eaxUnit, uint32_t client,
                                             const D2GameBatch8DB00Env* env)
{
    const void* unit = (void*)(uintptr_t)eaxUnit;
    const int32_t num = env->ordinal10973(eaxUnit, 6, 0) >> 8;
    const int32_t den = env->ordinal10913(eaxUnit) >> 8;
    uint8_t ratio;
    if (den > 0 && num < den) {
        ratio = (uint8_t)((num << 7) / den);
    } else {
        ratio = 0x80;
    }
    env->ordinal10887(eaxUnit, 0x160, ratio, 0);
    if (ratio > 1) ratio = (uint8_t)(ratio - 1);
    if (rd32(unit, 0) == 1) {
        const uint32_t playerData = rd32(unit, 0x14);
        if (playerData != 0 && (rd8((void*)(uintptr_t)playerData, 0x17) & 1) != 0) ratio |= 0x80;
    }

    uint8_t pkt[9] = {0};
    pkt[0] = 0x0C;
    pkt[1] = 0x01;
    wr32(pkt, 2, rd32(unit, 0x0C));
    pkt[6] = 0x13;
    pkt[7] = rd8(unit, 0xB0);
    pkt[8] = ratio;
    env->sendPacket(client, pkt, 9);
}

// 1.13c D2Game+0xF3410
// EBX = unit, stack arg = client. Packet 0x57 (14 bytes) for a type-1 unit with a player-data
// block at +0x14 whose +0x5C bit 0 is set: unit id, word +0x14, word +0x1C, byte +0x1E, and
// (byte+0x16 >> 2) & 1 as a word.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2game_f3410_impl(uint32_t ebxUnit, uint32_t client,
                                             const D2GameBatch8DB00Env* env)
{
    if (ebxUnit == 0) return;
    const void* unit = (void*)(uintptr_t)ebxUnit;
    if (rd32(unit, 0) != 1) return;
    const uint32_t playerData = rd32(unit, 0x14);
    if (playerData == 0) return;
    const void* pd = (void*)(uintptr_t)playerData;
    if ((rd8(pd, 0x5C) & 1) == 0) return;

    const uint16_t word14 = rd16(pd, 0x14);
    const uint16_t word1C = rd16(pd, 0x1C);
    const uint16_t byte1E = rd8(pd, 0x1E);
    const uint16_t flag = (uint16_t)((rd8(pd, 0x16) >> 2) & 1);

    uint8_t pkt[14] = {0};
    pkt[0] = 0x57;
    wr32(pkt, 1, rd32(unit, 0x0C));
    pkt[5] = 0x01;
    wr16(pkt, 6, word14);
    wr16(pkt, 8, word1C);
    wr16(pkt, 0x0A, byte1E);
    wr16(pkt, 0x0C, flag);
    env->sendPacket(client, pkt, 14);
}

// 1.13c D2Game+0x77B20
// ESI = unit, EDI = the record filled by 0x778A0. Applies each side's skill through 0x6FCC2100
// when the skill exists, differs from the current one, and its use state is neither 2 nor 7.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_77b20_batch(uint32_t esiUnit, uint32_t ediRec,
                                             const D2GameBatch8DB00Env* env)
{
    const void* rec = (void*)(uintptr_t)ediRec;
    const uint32_t leftId = rd32(rec, 4);
    if (leftId != 0) {
        void* skill = env->ordinal10708(rd32(rec, 0x0C));
        if (skill != nullptr && skill != env->ordinal10828(esiUnit)) {
            const int32_t state = env->ordinal10787(esiUnit, skill);
            if (state != 2 && state != 7) {
                env->subA2100(esiUnit, 1, leftId, rd32(rec, 0x0C));
            }
        }
    }
    const uint32_t rightId = rd32(rec, 0);
    if (rightId != 0) {
        void* skill = env->ordinal10708(rd32(rec, 8));
        if (skill != nullptr && skill != env->ordinal10507(esiUnit)) {
            const int32_t state = env->ordinal10787(esiUnit, skill);
            if (state != 2 && state != 7) {
                env->subA2100(esiUnit, 0, rightId, rd32(rec, 8));
            }
        }
    }
}

#if 0  // superseded: this probe-form 78DD0 collided with the new d2_fn_d2game_78dd0 (pb11)
// (superseded by d2game_78dd0.cpp) D2Game 78DD0
// EAX = unit, EBX = unit, stack args arg1 (context), arg2 (unit). The 1.13c form of the 1.10f
// PlrModes weapon pass: gates on the unit's handles/stats, then refreshes a thrown or stackable
// weapon and walks the inventory for a replacement. EBP (arg2) supplies the handle at +0x60; the
// stack slot that held arg2 is reused by the compiler as a byte scratch, so it is modelled as
// `slot` below. The final fallback goes through 0x98B60.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_78dd0(uint32_t eaxUnit, uint32_t ebxUnit, uint32_t arg1Ctx,
                                             uint32_t arg2Unit, const D2GameBatch8DB00Env* env)
{
    static const char kPlrModesFile[] = "..\\Source\\D2Game\\PLAYER\\PlrModes.cpp";
    const uint32_t esi = eaxUnit;
    const uint32_t ebp = arg2Unit;

    if (env->ordinal10048(esi) == 0 && env->ordinal10711(esi) == 0 &&
        env->ordinal10910(esi, 0x7D, 0) == 0) {
        return;
    }
    if (env->ordinal10973(esi, 0x46, 0) > 0) return;
    if (env->ordinal10910(esi, 0x7D, 0) != 0) {
        env->ordinal10887(esi, 0x46, 0, 0);
        return;
    }

    if (env->ordinal10744(esi) != 0 && env->ordinal10516(esi) != 0) {
        if (env->ordinal10202(esi, 0x100, 0x512, kPlrModesFile) == 0) {
            env->subD5740(ebp, arg1Ctx);
        }
        return;
    }

    // The compiler's arg2 stack slot doubles as a scratch word from here on: its low byte is the
    // #10417 result and is what the later #11139 call receives.
    uint32_t slot = ebp;
    slot = (slot & 0xFFFFFF00u) | env->ordinal10417(esi);

    const uint32_t uVar2 = env->ordinal11088(esi);
    uint32_t edi = env->ordinal10905(esi);
    const uint32_t local = env->ordinal11054(esi);
    env->ordinal11090(esi, 1);

    int32_t outFlag = 0;
    env->subD8970(arg1Ctx, ebp, (uint32_t)(uintptr_t)&outFlag, slot, 1, &outFlag);
    env->ordinal10892(esi, 1, 1);

    if (edi != 0) {
        uint32_t handle = rd32((void*)(uintptr_t)ebp, 0x60);
        edi = env->ordinal10010(handle, uVar2, 0);
        while (edi != 0) {
            const uint32_t selfId = (esi == 0) ? 0xFFFFFFFFu : rd32((void*)(uintptr_t)esi, 4);
            const uint32_t itemId = rd32((void*)(uintptr_t)edi, 4);
            if (selfId == itemId &&
                env->ordinal10202(edi, 0x100, 0x530, kPlrModesFile) == 0) {
                if (edi != 0) {
                    if (selfId == rd32((void*)(uintptr_t)edi, 4) &&
                        env->ordinal11139(handle, slot & 0xFF) == 0 &&
                        env->subD57F0(arg1Ctx, ebp, arg1Ctx, rd32((void*)(uintptr_t)edi, 0x0C),
                                      slot) != 0) {
                        return;
                    }
                }
                break;
            }
            handle = rd32((void*)(uintptr_t)ebp, 0x60);
            edi = env->ordinal10010(handle, uVar2, edi);
        }
    }

    if (local != 0) {
        env->sub98B60((local & 0xFFFFFF00u) | (slot & 0xFF), arg1Ctx, ebp);
    }
}
#endif  // superseded by d2game_78dd0.cpp

// 1.13c D2Game+0xAF5F0
// EAX = index. Indices below 9 read the nine-entry table at DAT_6FD1B614; everything above is
// (index - 2) * 0x32.
extern "C"
EMSCRIPTEN_KEEPALIVE int32_t d2_fn_d2game_af5f0_port(uint32_t eaxIndex,
                                                const D2GameBatch8DB00Env* env)
{
    if (eaxIndex < 9) return env->modeTable[eaxIndex];
    return (int32_t)(eaxIndex - 2) * 0x32;
}

// 1.13c D2Game+0x37EC0
// ECX = this (item), EDI = player, stack args a1, a2. Fatal 0xD8/0xD9 on the two null fails.
// Pushes 0x1100(this) plus the second stack-arg slot through 0xD4E80 with ECX=player, EDX=a1,
// EAX=this->unitId, and returns whether it succeeded.
extern "C"
EMSCRIPTEN_KEEPALIVE uint8_t d2game_port_37ec0(uint32_t ecxThis, uint32_t ediUnit, uint32_t a1,
                                                uint32_t a2, const D2GameBatch8DB00Env* env)
{
    if (ecxThis == 0) d2game_fatal(0xD8);
    if (ediUnit == 0) d2game_fatal(0xD9);
    const uint32_t unitId = rd32((void*)(uintptr_t)ecxThis, 0x0C);
    const uint32_t value = env->sub1100(ecxThis);
    int32_t outFlag = 0;
    const int32_t r = env->subD4E80(ediUnit, a1, unitId, value, a1, &outFlag);
    (void)a2;
    return r != 0 ? 1 : 0;
}
