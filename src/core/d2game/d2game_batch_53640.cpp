// src/core/d2game/d2game_batch_53640.cpp -- six 1.13c D2Game.dll save-file functions transcribed
// from the cached Ghidra decompiles (build-wasm/ghidra-d2game/decomp/0x<RVA>.c). Stage A batch lane:
// implementation + marker only, no spec, no vector, no test runner.
//
// Three belong to the legacy parser reached from 0x55E10 (save versions < 0x5C): 0x53640 reads the
// 16-stat compact section (magic 0x6667 'fg'), 0x55050 dispatches the item section on the version
// word (0x47 -> 0x54D90, 0x48..0x60 -> 0x54810, anything else -> 1), 0x55A10 reads the corpse list
// (magic 0x4D4A 'JM', count at +2). Their 1.10f counterparts are sub_6FC8ADE0, sub_6FC8BC70 and
// sub_6FC8BCC0 in reference/D2MOO/source/D2Game/src/PLAYER/PlrSave.cpp.
//
// Three belong to the current parser 0xEDB50 (save versions >= 0x5C): 0xEBDA0 is
// PLRSAVE2_ReadStats (version <= 94 branch; > 94 delegates to 0xEB910 PLRSAVE2_ReadStatsEx), 0xECA60
// is PLRSAVE2_ReadSkills (1.13c address of the 1.10f function already carried in PlrSaveItems.cpp,
// marker missing there), 0xECC90 is PLRSAVE2_InitializeStats (declared in PlrSave2.h, not implemented
// in src/). Counterparts in reference/D2MOO/source/D2Game/src/PLAYER/PlrSave2.cpp.
//
// Retail is 32-bit and the decompiler dropped every register-only argument (in_EAX / unaff_ESI /
// unaff_EDI / unaff_EBP); those are explicit parameters here. The seam struct carries every callee
// src/ does not implement, in retail argument order. Ids read from a section are little-endian byte
// offsets; nothing here dereferences a unit struct beyond the raw offsets the decompile shows.
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <unistd.h>
#include <emscripten.h>

static inline uint8_t  rd8u(const void* p, uint32_t o) { return ((const uint8_t*)p)[o]; }
static inline uint16_t rd16u(const void* p, uint32_t o) { uint16_t v; std::memcpy(&v, (const uint8_t*)p + o, 2); return v; }
static inline uint32_t rd32u(const void* p, uint32_t o) { uint32_t v; std::memcpy(&v, (const uint8_t*)p + o, 4); return v; }
static inline void*    rdptr(const void* p, uint32_t o) { void* v; std::memcpy(&v, (const uint8_t*)p + o, sizeof v); return v; }
static inline void     wr16u(void* p, uint32_t o, uint16_t v) { std::memcpy((uint8_t*)p + o, &v, 2); }
static inline void     wr32u(void* p, uint32_t o, uint32_t v) { std::memcpy((uint8_t*)p + o, &v, 4); }
static inline void     wrptr(void* p, uint32_t o, const void* v) { std::memcpy((uint8_t*)p + o, &v, sizeof v); }

// 0x6FD19FF9: the module-name literal the 0x53640 fatal path passes to Ordinal_10024.
static const char kModuleName[] = "D2Game.dll";

// Every callee of the six that src/ does not implement. Names are the RVA or the import ordinal; the
// comment says what the call site and the 1.10f baseline make it.
struct D2GameBatch53640Env {
    // ---- shared stat helpers ------------------------------------------------------------------
    void     (*statSet)(void* unit, int32_t stat, int32_t value, int32_t layer);  // Ordinal_10887
    int32_t  (*statGet)(void* unit, int32_t stat, int32_t layer);                 // Ordinal_10973
    int32_t  (*popCount)(const void* data, int32_t nBytes);                       // Ordinal_10230
    int32_t  (*maxLife)(void* unit);                                              // Ordinal_10913
    int32_t  (*maxMana)(void* unit);                                              // Ordinal_10186
    int32_t  (*maxStamina)(void* unit);                                           // Ordinal_10578
    void*    (*getPlayerData)(void* unit);                                        // Ordinal_10920
    // ---- 0x53640 ------------------------------------------------------------------------------
    uint32_t (*ordinal10265)(uint32_t id);                                        // fatal code lookup
    void     (*ordinal10024)(const char* module, uint32_t id);                    // fatal, tail is _exit
    // ---- 0x55050 ------------------------------------------------------------------------------
    int32_t  (*sub54d90)(void* p5, void* p3, int32_t* p4, int32_t ebx, int32_t edi, int32_t* esi);
    int32_t  (*sub54810)(void* p3, int32_t* p4, uint32_t version, int32_t ebx, int32_t edi, int32_t* esi);
    // ---- 0x55A10 ------------------------------------------------------------------------------
    int32_t  (*sub550a0)(void* p1, int32_t* pConsumed);                           // one corpse record
    // ---- 0xEBDA0 ------------------------------------------------------------------------------
    int32_t  (*readStatsEx)(void* pUnit, uint8_t** ppSection);                    // 0xEB910, same two args
    // ---- 0xECA60 ------------------------------------------------------------------------------
    int32_t  (*getClassId)(void* client);                                         // FUN_6fc69bf0
    int32_t  (*getClassSkillId)(int32_t classId, int32_t index);                  // Ordinal_10044
    int32_t  (*getSkillCount)(int32_t classId);                                   // Ordinal_11120
    void     (*setSkill)(void* unit, int32_t skillId, int32_t level, int32_t a4); // FUN_6fcbf9a0
    void     (*subAB5A0)(void* playerData, void* unit);                           // FUN_6fcab5a0
    void     (*ordinal10418)(void* client, void* unit);                           // tail of the skill read
    void     (*subB6FA0)(void* a, void* b);                                       // FUN_6fcb6fa0
    // ---- 0xECC90 ------------------------------------------------------------------------------
    int32_t  (*invGoldLimit)(void* unit);                                         // Ordinal_10049
    int32_t  (*stashGoldLimit)(void* unit);                                       // Ordinal_11060
    int32_t  (*alterSkillFlag)(int32_t itemIndex);                                // FUN_6fd0b8b0
    int32_t  (*ordinal10708)(void* unit);                                         // register args dropped by Ghidra
    void     (*ordinal10546)(void* unit, int32_t skillId);
    void     (*ordinal10537)(void* unit, int32_t skillId);
    void     (*assignSkill)(void* unit, int32_t hand, int32_t skillId, int32_t flags);  // FUN_6fcc2100
    int32_t  (*getUnitStat)(void* unit, int32_t stat, int32_t layer);             // Ordinal_10587
    // Ordinal_10399: (unit[1], level) -> the client carrying bSwitchWeapon at +0x45C; the call also
    // yields the level's next-exp threshold, written through pNextExp. The decompile keeps both in
    // one register, so the seam takes the out pointer.
    void*    (*resolveNextExp)(int32_t unitSlot, int32_t level, int32_t* pNextExp);
    void     (*sendPacket)(const void* buf, int32_t len);                         // FUN_6fcaa3e0
};

// 0x53640: legacy compact stat section (1.10f sub_6FC8ADE0). Magic 0x6667, the param_2 bits of the
// mask at +2 select stats; the present values are 32-bit words after the mask, read at a +3 byte
// offset from the mask end exactly as the 1.10f baseline does. The section length written back is
// 4*present+5. In arena mode hitpoints and mana are set to their maxima; stamina is always.
// The vectored rule is src/core/d2game/d2game_53640.h (probe src/core/fn/d2game_53640.cpp owns the d2_fn_d2game_53640 export).
extern "C" EMSCRIPTEN_KEEPALIVE int32_t d2game_53640_batch(void* pGame, void* pUnit, const uint8_t* pSection, int32_t nStats, int32_t* pSize, const D2GameBatch53640Env* env) {  // 1.13c D2Game+0x53640
    *pSize = 0;
    if (rd16u(pSection, 0) != 0x6667) {
        return 4;
    }

    const uint8_t* pMask = pSection + 2;
    int32_t nPresent = 0;
    for (int32_t i = 0; i < nStats; ++i) {
        if ((rd8u(pMask, (uint32_t)i >> 3) & (1u << (i & 7))) != 0) {
            env->statSet(pUnit, i, (int32_t)rd32u(pMask, (uint32_t)(4 * nPresent + 3)), 0);
            ++nPresent;
        } else {
            env->statSet(pUnit, i, 0, 0);
        }
    }

    void* pArena = rdptr(pGame, 0x1D28);
    if (pArena != nullptr) {
        if ((rd8u(pArena, 8) & 2) != 0) {
            env->statSet(pUnit, 6, env->maxLife(pUnit), 0);   // STAT_HITPOINTS
            env->statSet(pUnit, 8, env->maxMana(pUnit), 0);   // STAT_MANA
        }
        env->statSet(pUnit, 10, env->maxStamina(pUnit), 0);   // STAT_STAMINA
        *pSize = 4 * nPresent + 5;
        return 0;
    }

    env->ordinal10024(kModuleName, env->ordinal10265(0x27));
    _exit(-1);
}

// 0x55050: item-section version dispatch (1.10f sub_6FC8BC70). Version 0x47 goes to 0x54D90, the
// range 0x48..0x60 to 0x54810; every other version returns 1 unchanged. The two targets are seam
// calls because they are outside this batch.
extern "C" EMSCRIPTEN_KEEPALIVE int32_t d2g_55050(void* pGame, uint32_t nVersion, void* pSection, int32_t* p4, void* p5, int32_t ebx, int32_t edi, int32_t* esi, const D2GameBatch53640Env* env) {  // 1.13c D2Game+0x55050
    if (nVersion == 0x47) {
        return env->sub54d90(p5, pSection, p4, ebx, edi, esi);
    }
    if (nVersion > 0x47 && nVersion < 0x61) {
        return env->sub54810(pSection, p4, nVersion, ebx, edi, esi);
    }
    return 1;
}

// 0x55A10: legacy corpse list (1.10f sub_6FC8BCC0). Guards the version to 0x47..0x60, checks the
// 0x4D4A 'JM' magic and the count at +2, and walks that many 0x550A0 records, accumulating their
// consumed byte counts on top of the 4-byte list header. Version 0x47 is accepted here even though
// the item dispatcher treats it separately.
extern "C" EMSCRIPTEN_KEEPALIVE int32_t d2_old_55a10(const uint8_t* pSection, void* p1, uint32_t nVersion, int32_t* pSize, const D2GameBatch53640Env* env) {  // 1.13c D2Game+0x55A10
    *pSize = 0;
    if (!(nVersion > 0x46 && nVersion < 0x61)) {
        return 1;
    }
    if (rd16u(pSection, 0) != 0x4D4A) {
        return 0xE;
    }

    const uint16_t nCount = rd16u(pSection, 2);
    int32_t nTotal = 4;
    for (uint32_t i = 0; i < nCount; ++i) {
        int32_t nConsumed = 0;
        const int32_t nError = env->sub550a0(p1, &nConsumed);
        if (nError != 0) {
            return nError;
        }
        nTotal += nConsumed;
    }

    *pSize = nTotal;
    return 0;
}

// 0xEBDA0: PLRSAVE2_ReadStats for save versions <= 94 (1.10f PLRSAVE2_ReadStats). Newer versions
// delegate to 0xEB910 ReadStatsEx. The section is magic 0x6667, a (nStats+7)/8-byte present mask at
// +2, then one 32-bit word per set bit in stat order; unset stats are written as 0. The out pointer
// advances past the mask and the words actually consumed. 0x12 is PLRSAVE2ERROR_BAD_STATS.
extern "C" EMSCRIPTEN_KEEPALIVE int32_t d2_fn_d2game_EBDA0(void* pGame, void* pUnit, uint8_t** ppSection, uint8_t* pEnd, uint32_t nVersion, int32_t nStats, const D2GameBatch53640Env* env) {  // 1.13c D2Game+0xEBDA0
    if (nVersion > 94) {
        return env->readStatsEx(pUnit, ppSection);
    }

    uint8_t* pSection = *ppSection;
    const int32_t nMaskBytes = (nStats + 7) / 8;
    if (pSection + nMaskBytes + 2 > pEnd) {
        return 0x12;
    }
    if (rd16u(pSection, 0) != 0x6667) {
        return 0x12;
    }

    const uint8_t* pMask = pSection + 2;
    const uint8_t* pData = pMask + nMaskBytes;
    const int32_t nPresent = env->popCount(pMask, nMaskBytes);
    if (pData + 4 * nPresent > pEnd) {
        return 0x12;
    }

    int32_t nCounter = 0;
    for (int32_t i = 0; i < nStats; ++i) {
        int32_t nValue = 0;
        if ((rd8u(pMask, (uint32_t)i >> 3) & (1u << (i & 7))) != 0) {
            nValue = (int32_t)rd32u(pData, (uint32_t)(4 * nCounter));
            ++nCounter;
        }
        env->statSet(pUnit, i, nValue, 0);
    }

    *ppSection = (uint8_t*)pData + 4 * nCounter;
    return 0;
}

// 0xECA60: PLRSAVE2_ReadSkills (1.10f D2Game.0x6FC8E330 -> 1.13c 0x6FD0CA60). Magic 0x6669, then
// nSkills bytes of levels: skill i's id comes from the class table, the walk stops once the class
// index passes the class skill count, and a nonzero byte assigns that level. Two client/skill
// finalizers run afterwards, then the out pointer advances by nSkills bytes. 0x13 is
// PLRSAVE2ERROR_BAD_SKILLS. Register-only arguments of the ordinals below are explicit in the seams.
extern "C" EMSCRIPTEN_KEEPALIVE int32_t d2_fn_d2game_eca60(void* pGame, void* pUnit, uint8_t** ppSection, uint8_t* pEnd, uint32_t nVersion, int32_t nSkills, const D2GameBatch53640Env* env) {  // 1.13c D2Game+0xECA60
    uint8_t* pSection = *ppSection;
    if (pEnd < pSection + 2 || rd16u(pSection, 0) != 0x6669) {
        return 0x13;
    }
    if (pUnit == nullptr || rd32u(pUnit, 0) != 0) {
        return 0x13;
    }

    void* pPlayerData = env->getPlayerData(pUnit);
    void* pClient = pPlayerData ? rdptr(pPlayerData, 0x9C) : nullptr;
    if (pClient == nullptr) {
        return 0x13;
    }

    const int32_t nClassId = env->getClassId(pClient);
    const uint8_t* pLevels = pSection + 2;
    for (int32_t i = 0; i < nSkills; ++i) {
        const int32_t nSkillId = env->getClassSkillId(nClassId, i);
        if (env->getSkillCount(nClassId) < i) {
            break;
        }
        if (pLevels[i] != 0) {
            env->setSkill(pUnit, nSkillId, pLevels[i], 1);
        }
    }

    env->subAB5A0(pClient, pUnit);
    env->ordinal10418(pClient, pUnit);
    if (rd32u(pUnit, 0) == 0) {
        env->subB6FA0(rdptr(pUnit, 0x20 * sizeof(uint32_t) / sizeof(uint32_t)), pUnit);
    }

    *ppSection = pSection + 2 + nSkills;
    return 0;
}

// 0xECC90: PLRSAVE2_InitializeStats (1.10f D2Game.0x6FC8EBA0; declared in PlrSave2.h, the body was
// never carried into src/). Clamps gold and stash gold to their limits, sets stamina to its maximum,
// fixes the 16 hotkey slots and the four skill-flag item indices, assigns the saved left/right
// skills, then writes hitpoints, mana, the three 100% rates and the next-experience threshold.
// Finally a one-byte 0x97 packet goes out when the client's weapon switch is set. The 1.10f baseline
// is the authority for the stat ids and the field offsets; the decompile fixes the order.
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_ecc90(void* pGame, void* pUnit, int32_t nHitpoints, int32_t nMana, int32_t nFirstSkillItem, const D2GameBatch53640Env* env) {  // 1.13c D2Game+0xECC90
    if (pUnit != nullptr && rd32u(pUnit, 0) == 0) {
        env->getPlayerData(pUnit);
    }
    void* pPlayerData = env->getPlayerData(pUnit);
    void* pSkillFields = pPlayerData;  // the decompile swaps this to the unit after the 0x23 packet

    const int32_t nGold = env->statGet(pUnit, 14, 0);
    if (nGold < 0 || env->invGoldLimit(pUnit) < nGold) {
        if (pUnit != nullptr && rd32u(pUnit, 0) == 0) {
            env->invGoldLimit(pUnit);
        }
        env->statSet(pUnit, 14, 0, 0);
    }

    const int32_t nGoldBank = env->statGet(pUnit, 15, 0);
    if (nGoldBank < 0 || env->stashGoldLimit(pUnit) < nGoldBank) {
        if (pUnit != nullptr && rd32u(pUnit, 0) == 0) {
            env->stashGoldLimit(pUnit);
        }
        env->statSet(pUnit, 15, 0, 0);
    }

    env->statSet(pUnit, 10, env->maxStamina(pUnit), 0);  // STAT_STAMINA

    // 16-step chain through 0xFD0B8B0 on the caller's first-item register. The 1.10f baseline runs
    // the same 16 hotkey slots and fixes each one's item index into an item GUID; the decompiled loop
    // keeps the chain's side effects in order.
    int32_t nCursor = nFirstSkillItem;
    for (int32_t i = 0; i < 16; ++i) {
        nCursor = nCursor != 0 ? env->alterSkillFlag(nCursor) : 0;
    }

    // D2PlayerDataStrc +0x7C left flags, +0x78 right flags, +0x8C switch-left, +0x88 switch-right;
    // converted only when nonzero, in this order.
    if (rd32u(pPlayerData, 0x7C) != 0) {
        wr32u(pPlayerData, 0x7C, (uint32_t)env->alterSkillFlag((int32_t)rd32u(pPlayerData, 0x7C)));
    }
    if (rd32u(pPlayerData, 0x78) != 0) {
        wr32u(pPlayerData, 0x78, (uint32_t)env->alterSkillFlag((int32_t)rd32u(pPlayerData, 0x78)));
    }
    if (rd32u(pPlayerData, 0x8C) != 0) {
        wr32u(pPlayerData, 0x8C, (uint32_t)env->alterSkillFlag((int32_t)rd32u(pPlayerData, 0x8C)));
    }
    if (rd32u(pPlayerData, 0x88) != 0) {
        wr32u(pPlayerData, 0x88, (uint32_t)env->alterSkillFlag((int32_t)rd32u(pPlayerData, 0x88)));
    }

    // +0x74 left skill id, +0x70 right skill id.
    const int32_t nLeftSkill = (int32_t)rd32u(pPlayerData, 0x74);
    if (nLeftSkill != 0 && nLeftSkill != 5 && env->ordinal10708(pUnit) != 0) {
        env->ordinal10546(pUnit, nLeftSkill);
        env->ordinal10537(pUnit, nLeftSkill);

        if (pUnit != nullptr && rd32u(pUnit, 0) == 0) {
            // Packet 0x23: [0x23, 0, unitId(4), 1, skillId]; retail sends 13 bytes and leaves the
            // tail as stack garbage, zeroed here so a replay starts from a fixed value. The extra
            // 0xFD0920 call before the buffer is a discarded side effect in the decompile.
            uint8_t pkt[13] = {};
            const uint32_t nUnitId = rd32u(pUnit, 0x0C);
            env->getPlayerData(pUnit);
            pkt[0] = 0x23;
            pkt[1] = 0;
            pkt[2] = (uint8_t)(nUnitId);
            pkt[3] = (uint8_t)(nUnitId >> 8);
            pkt[4] = (uint8_t)(nUnitId >> 16);
            pkt[5] = (uint8_t)(nUnitId >> 24);
            pkt[6] = 1;
            pkt[7] = (uint8_t)nLeftSkill;
            env->sendPacket(pkt, 0x0D);
            pSkillFields = pUnit;  // decompile: piVar4 = unaff_ESI after the send
        }
    }

    const int32_t nRightSkill = (int32_t)rd32u(pSkillFields, 0x70);
    if (nRightSkill != 0) {
        env->assignSkill(pUnit, 0, nRightSkill, (int32_t)rd32u(pSkillFields, 0x78));
    }

    env->statSet(pUnit, 6, nHitpoints, 0);              // STAT_HITPOINTS
    env->statSet(pUnit, 8, nMana, 0);                   // STAT_MANA
    env->statSet(pUnit, 0x44, 100, 0);                  // STAT_ATTACKRATE
    env->statSet(pUnit, 0x43, 100, 0);                  // STAT_VELOCITYPERCENT
    env->statSet(pUnit, 0x45, 100, 0);                  // STAT_OTHER_ANIMRATE

    // STAT_NEXTEXP: unit[1] (or -1), the level at stat 12, then 0xFD03D99; the same result carries
    // the client whose +0x45C is bSwitchWeapon and the next-exp threshold.
    const int32_t nUnitSlot = pUnit != nullptr ? (int32_t)rd32u(pUnit, 4) : -1;
    const int32_t nLevel = env->getUnitStat(pUnit, 12, 0);
    int32_t nNextExp = 0;
    void* pSwitchClient = env->resolveNextExp(nUnitSlot, nLevel, &nNextExp);
    env->statSet(pUnit, 0x1E, nNextExp, 0);

    if (rd8u(pSwitchClient, 0x45C) != 0) {
        const uint8_t pkt97 = 0x97;
        env->sendPacket(&pkt97, 1);
    }
}
