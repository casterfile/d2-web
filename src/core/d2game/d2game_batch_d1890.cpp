// src/core/d2game/d2game_batch_d1890.cpp -- twelve 1.13c D2Game.dll functions transcribed from the
// cached Ghidra decompiles (build-wasm/ghidra-d2game/decomp/0x<RVA>.c). Base 0x6FC20000.
//
// Retail module state is modelled as opaque host records read by their retail byte offsets. Register
// only arguments (Ghidra's in_EAX / unaff_ESI) are explicit parameters and are named at each
// function. Unmapped callees are seams on D2GameBatchBd18Env in retail argument order; a seam whose
// decompile shows no arguments is taken to have none. The fatal path is Fog Ordinal 10265(code) ->
// Ordinal 10024(text) -> _exit(-1); only the exit is observable and is modelled.
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <unistd.h>
#include <emscripten.h>

static inline uint8_t  rd8 (const void* p, uint32_t o) { return ((const uint8_t*)p)[o]; }
static inline uint16_t rd16(const void* p, uint32_t o) { uint16_t v; std::memcpy(&v, (const uint8_t*)p + o, 2); return v; }
static inline int16_t  rds16(const void* p, uint32_t o) { int16_t v; std::memcpy(&v, (const uint8_t*)p + o, 2); return v; }
static inline uint32_t rd32(const void* p, uint32_t o) { uint32_t v; std::memcpy(&v, (const uint8_t*)p + o, 4); return v; }
static inline void wr8 (void* p, uint32_t o, uint8_t v)  { ((uint8_t*)p)[o] = v; }
static inline void wr16(void* p, uint32_t o, uint16_t v) { std::memcpy((uint8_t*)p + o, &v, 2); }
static inline void wr32(void* p, uint32_t o, uint32_t v) { std::memcpy((uint8_t*)p + o, &v, 4); }

[[noreturn]] static void d2game_fatal(uint32_t) { _exit(-1); }

#include "d2game_batch_d1890_env.h"

static inline void* rdptr(const void* p, uint32_t o) { void* v; std::memcpy(&v, (const uint8_t*)p + o, sizeof v); return v; }

// 1.13c D2Game+0xD1890
// ItemMode.cpp constructor-time state fill: three ordinal-10973 reads go to +0x00/+0x04/+0x08; the
// item from 10828 (or 10507) yields its handle and name into +0x06/+0x04 (or +0x05/+0x03), with
// -1/0 when the handle is null. The decompile carries the context in ESI.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_D1890(uint32_t* ctx, const void* unit, const D2GameBatchBd18Env* env)
{
    static const char kItemModeFile[] = "..\\Source\\D2Game\\ITEMS\\ItemMode.cpp";

    ctx[0] = env->unitStat10973(unit, 6, 0);
    ctx[1] = env->unitStat10973(unit, 8, 0);
    ctx[2] = env->unitStat10973(unit, 0xA, 0);

    const void* item = env->ordinal10828(unit);
    if (item == nullptr) {
        ctx[6] = 0xFFFFFFFFu;
        ctx[4] = 0;
    }
    else {
        ctx[6] = env->ordinal10304(item);
        ctx[4] = env->ordinal10601(item, kItemModeFile, 0x9E0);
    }

    const void* owner = env->ordinal10507(unit);
    if (owner != nullptr) {
        ctx[5] = env->ordinal10304(owner);
        ctx[3] = env->ordinal10601(owner, kItemModeFile, 0x9EC);
        return;
    }
    ctx[5] = 0xFFFFFFFFu;
    ctx[3] = 0;
}

// 1.13c D2Game+0xD4FF0
// Item-mode continuation on a client: its +0x60 block is non-null and ordinal 10417 is neither 0x0B
// nor 0x0C; 10138 consumes the block, then 10175 must be 1, or 0 with 11057 non-zero. Only then does
// the 0xD47C0 tail run. The client arrives in ECX.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_D4FF0(void* client, const D2GameBatchBd18Env* env)
{
    void* block = (void*)(uintptr_t)rd32(client, 0x60);
    if (block == nullptr) return;

    const uint8_t mode = env->ordinal10417();
    if (mode == 0x0B || mode == 0x0C) return;

    env->ordinal10138(block);
    if (env->ordinal10175() != 1) {
        if (env->ordinal10175() != 0) return;
        if (env->ordinal11057() == 0) return;
    }
    env->subD47C0(client);
}

// 1.13c D2Game+0xDF4A0
// Monster drop table pick. With a unit in EAX and param_3's +0x6D clear, the unit id at param_2+0x2C0
// is resolved and the player/difficulty index (0x211D0) selects a signed entry id at unit+0x9A; with
// no unit (or +0x6D set) the count at param_2+0x10 walks a list whose entries are 0x34 bytes at
// param_2+0x14 (count byte at +2) until the index is exhausted, then the entry's first word is the
// table id. A valid id indexes the 0x1A8-byte table: if its +0x20 word is non-negative and its +0x0E
// flags pass bit mask 0x1C, a 64-bit LCG (lo*0x6AC690C5+hi, stored back) rolls
// (low32 % 100) and replaces the id with the table's +0x20 word when it exceeds param_5.
// Returns the table index (or the negative id) and stores the entry pointer in *param_4 (0 on miss).
extern "C"
EMSCRIPTEN_KEEPALIVE int32_t D2GAME_PickMonsterStatsRecord_DF4A0(uint32_t param_1, uint8_t* param_2, uint8_t* param_3,
                                                uint32_t* param_4, uint8_t param_5, void* eaxUnit,
                                                const D2GameBatchBd18Env* env)
{
    uint8_t* rng = param_1 ? (uint8_t*)(uintptr_t)(param_1 + 0x6C) : nullptr;

    int32_t index;
    const uint8_t* tables = env->dataTables;

    if (eaxUnit == nullptr || rd8(param_3, 0x6D) != 0) {
        if (rd8(param_2, 0x10) == 0) {
            *param_4 = 0;
            return 0;
        }
        const uint32_t base = env->sub211D0(rng);
        int32_t remaining = (int32_t)(base + 1);
        uint8_t* entry = param_2 + 0x14;
        uint32_t i = 0;
        if (rd8(param_2, 0x10) != 0) {
            do {
                remaining -= rd8(entry, 2);
                if (remaining < 1) break;
                entry += 0x34;                       // 0x1A shorts per entry
                i++;
            } while ((int32_t)i < (int32_t)(uint32_t)rd8(param_2, 0x10));
        }

        int16_t value = rds16(entry, 0);
        index = value;
        const int32_t tableCount = (int32_t)rd32(tables, 0xA80);
        if (index > -1 && index < tableCount) {
            const uint8_t* te = tables + rd32(tables, 0xA78) + index * 0x1A8;
            if (rds16(te, 0x20) > -1 && (env->bitMasks[0x1C] & rd8(te, 0x0E)) != 0) {
                const uint64_t roll = (uint64_t)(uint32_t)rd32(rng, 0) * 0x6AC690C5ull + rd32(rng, 4);
                wr32(rng, 0, (uint32_t)roll);
                wr32(rng, 4, (uint32_t)(roll >> 32));
                if ((uint32_t)param_5 < (uint32_t)((uint32_t)roll % 100)) {
                    value = rds16(te, 0x20);
                }
            }
        }
        index = value;
        if (index < 0) {
            *param_4 = 0;
            return index;
        }
    }
    else {
        uint8_t* unit = (uint8_t*)env->ordinal10014(rd32(param_2, 0x2C0));
        if (rd8(unit, 0x35) == 0) {
            *param_4 = 0;
            return 0;
        }
        const uint32_t base = env->sub211D0(rng);
        index = rds16(unit, 0x9A + base * 2);
        if (index < 0) {
            *param_4 = 0;
            return index;
        }
    }

    if (index < (int32_t)rd32(tables, 0xA80)) {
        *param_4 = rd32(tables, 0xA78) + index * 0x1A8;
        return index;
    }
    *param_4 = 0;
    return index;
}

// 1.13c D2Game+0xDF6C0 (D2Game_GetUnitByte78)
// A one-byte accessor on the unit in EAX (+0x78). A null unit is retail's fatal 0xA51
// (Fog 10265 -> 10024 -> _exit); only the exit is observable.
extern "C"
EMSCRIPTEN_KEEPALIVE uint8_t D2Game_GetUnitByte78(void* unit, const D2GameBatchBd18Env* env)
{
    if (unit == nullptr) d2game_fatal(0xA51);
    (void)env;
    return rd8(unit, 0x78);
}

// 1.13c D2Game+0x8E170
// Packet 0x5B builder (the packet base is Ghidra's uStack_14C; its zeroed 0x122-byte scratch at
// uStack_134 is packet+0x18). Opcode 0x5B; unit-derived id at packet+0x1B (ESI+0xC, or -1) and a
// byte at packet+0x1F (ESI+4, or 0xFF); 10973's word at packet+0x24; packet+0x26 is 0xFFFF unless
// ESI is set, ESI+0x80 is non-null and ESI+0x00 is clear, then 0x9BEB0() supplies it; 0x7FFFFFFF at
// packet+0, zero words at +0x28/+0x2A, EBX+0x45E at +0x2C; ordinal 501 initialises +0x14 (0xC bytes).
// The dword at EBX+0x460 and the string at EBX+0x464 are appended from packet+0x22, the length
// (0x22 + those bytes) is patched into packet+1..2, and 0x8A3E0 sends it. When ESI is set and
// ESI+0x00 is clear, 0x96920(param_1) follows. ESI is a unit, EBX the client/game record.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_8E170(uint32_t param_1, void* esi, void* ebx,
                                             const D2GameBatchBd18Env* env)
{
    uint8_t pkt[0x140];
    std::memset(pkt, 0, sizeof pkt);

    env->ordinal10920();
    pkt[0x18] = 0x5B;
    if (esi == nullptr) {
        wr32(pkt, 0x1B, 0xFFFFFFFFu);
        pkt[0x1F] = 0xFF;
    }
    else {
        wr32(pkt, 0x1B, rd32(esi, 0x0C));
        pkt[0x1F] = rd8(esi, 0x04);
    }
    wr16(pkt, 0x24, (uint16_t)env->ordinal10973());   // CONCAT22 high half at uStack_12A
    if (esi == nullptr || rd32(esi, 0x80) == 0 || rd32(esi, 0x00) != 0) {
        wr16(pkt, 0x26, 0xFFFF);                      // uStack_126
    }
    else {
        wr16(pkt, 0x26, env->sub9BEB0());
    }
    wr32(pkt, 0x00, 0x7FFFFFFF);                      // uStack_14C
    wr16(pkt, 0x28, 0);                               // uStack_124
    wr16(pkt, 0x2A, 0);                               // uStack_122
    wr16(pkt, 0x2C, rd16(ebx, 0x45E));                // uStack_120
    env->ordinal501(pkt + 0x14, 0xC);                 // &stack0xfffffec8

    wr32(pkt, 0x22, rd32(ebx, 0x460));                // uStack_12A
    uint8_t* d = pkt + 0x22;
    while (*d != 0) ++d;                              // first NUL scan of the dword
    const uint8_t* s = (const uint8_t*)ebx + 0x464;
    for (;;) { uint8_t c = *s++; *d++ = c; if (c == 0) break; }

    const uint32_t len = (uint32_t)(d - pkt);         // pcVar3 - &uStack_14C
    pkt[1] = (uint8_t)len;                            // CONCAT21 patches packet+1..2
    pkt[2] = (uint8_t)(len >> 8);
    env->sub8A3E0(pkt, len);                          // 0x8A3E0
    if (esi != nullptr && rd32(esi, 0x00) == 0) env->sub96920(param_1);
}

// 1.13c D2Game+0xB2D40
// Unit-mode teardown: 0xB2C20(in_EAX, ESI, 1), fatal 0xFB when ESI is null, then 0xB2760 unless the
// mode at ESI+4 is 4 or the ESI+0x174 handle is null. ESI arrives as the unit.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_B2D40(uint32_t eaxVal, void* esi, const D2GameBatchBd18Env* env)
{
    env->subB2C20(eaxVal, esi, 1);
    if (esi == nullptr) d2game_fatal(0xFB);
    if (rd32(esi, 0x04) != 4 && rd32(esi, 0x174) != 0)
        env->subB2760x(rd32(esi, 0x174), (uint32_t)(uintptr_t)esi);
}

// 1.13c D2Game+0x9D780
// The room's unit-id array (ordinal 10383 on the in_EAX handle) is walked; for every id, ordinal
// 10733 yields a hash chain (next at +8) and each node not matching the player's (+0/+0xC) whose
// byte 0 is not 3 sends a 6-byte packet { 10, node[0], node[1] dword } through 0x8A3E0. A null
// in_EAX is fatal 0x84; a null player is fatal 0x88.
extern "C"
EMSCRIPTEN_KEEPALIVE void GAME_9D780(uint32_t eaxHandle, const D2GameBatchBd18Env* env)
{
    if (eaxHandle == 0) d2game_fatal(0x84);

    void* array = nullptr;
    uint32_t count = 0;
    env->ordinal10383(eaxHandle, &array, &count);

    uint8_t* player = (uint8_t*)env->sub6AD10(0);
    if (player == nullptr) d2game_fatal(0x88);

    const uint32_t playerId = rd32(player, 0);
    const uint32_t playerGuid = rd32(player, 0x0C);
    for (uint32_t i = 0; i < count; ++i) {
        uint8_t* node = (uint8_t*)env->ordinal10733(rd32((uint8_t*)array, i * 4));
        for (; node != nullptr; node = (uint8_t*)(uintptr_t)rd32(node, 0x08)) {
            if ((rd32(node, 0) != playerId || rd32(node, 0x04) != playerGuid) &&
                rd8(node, 0) != 3) {
                uint8_t pkt[6];
                pkt[0] = 10;
                pkt[1] = rd8(node, 0);
                wr32(pkt, 2, rd32(node, 0x04));
                env->sub8A3E0(pkt, 6);
            }
        }
    }
}

// 1.13c D2Game+0x9DEC0
// When the EDI record's slot for the mode byte at param_1+0x1AC (pointer at EDI+0xBC+mode*4, first
// dword non-zero) is live, the in_EAX handle must be non-null (else fatal 0x57) and ordinal 10383's
// unit count is consumed by repeated 0x9DE10(EDI) calls. A null param_1 is fatal 0x7CB.
extern "C"
EMSCRIPTEN_KEEPALIVE void D2GAME_RepeatSlotAction_9DEC0(void* param_1, uint32_t eaxHandle, void* edi,
                                             const D2GameBatchBd18Env* env)
{
    if (param_1 == nullptr) d2game_fatal(0x7CB);

    void* slot = rdptr(edi, 0xBC + (uint32_t)rd8(param_1, 0x1AC) * 4);
    if (rd32(slot, 0) == 0) return;

    if (eaxHandle == 0) d2game_fatal(0x57);
    void* array = nullptr;
    uint32_t count = 0;
    env->ordinal10383(eaxHandle, &array, &count);
    for (uint32_t i = 0; i < count; ++i) env->sub9DE10(edi);
}

// 1.13c D2Game+0xB2D90
// Inventory-mode tick: the EBX record's +0x1D28 block must be non-null with bit 0x400 set at its +8.
// Then, when the player exists (0x4AD10(0)), the 0xB23D0 probe runs and a non-zero result reaches
// 0xB2760. The in_EAX unit is required (fatal 0x9F1); its +0x3D4 flags bit 2 routes to
// 0xB2C20(EBX, unit, 0).
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_B2D90(uint32_t eaxUnit, void* ebx, const D2GameBatchBd18Env* env)
{
    uint8_t* block = (uint8_t*)(uintptr_t)rd32(ebx, 0x1D28);
    if (block != nullptr && (rd32(block, 0x08) & 0x400) != 0) {
        void* pl = env->sub6AD10x(eaxUnit, 0);
        if (pl != nullptr) {
            if (env->subB23D0x((uint32_t)(uintptr_t)pl) != 0)
                env->subB2760x((uint32_t)(uintptr_t)pl, eaxUnit);
        }
        if (eaxUnit == 0) d2game_fatal(0x9F1);
        if ((rd8((void*)(uintptr_t)eaxUnit, 0x3D4) & 4) != 0) {
            env->subB2C20((uint32_t)(uintptr_t)ebx, (void*)(uintptr_t)eaxUnit, 0);
        }
    }
}

// 1.13c D2Game+0x9B3B0
// Inventory-mode walk over the unit-id list behind client+0x60 (ordinal 10170 head, 10691 next,
// 10727 id). The game (param_1) is required (fatal 0x2D3); the bucket at game+0x1720+(id&0x7F)*4
// is chased through node+0xE4 until node+3 is the id. Each match clears item flags 0x10/0x4000 and,
// with flag 0x20, 0x80/0x180 (InvMode.cpp); 0x9B340 runs, the page bit at item+0xC8 picks either
// the nested list at item+0x60 (its entries resolved in game+0x1720 and run through 0x9B340, then
// released by 10545) or not. An item missing that nested list is fatal 0x1A4. Flag 1 sends the item
// through 0xDFC10 and 0xE20A0. The list walk ends with 10545.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_9B3B0(void* game, uint32_t eaxUnit, void* esiSaved,
                                             const D2GameBatchBd18Env* env)
{
    static const char kInvModeFile[] = "..\\Source\\D2Game\\INVENTORY\\InvMode.cpp";

    uint8_t* block = (uint8_t*)(uintptr_t)rd32((void*)(uintptr_t)eaxUnit, 0x60);
    if (block == nullptr) return;

    env->ordinal10201(block, 0);                       // register arguments
    uint32_t listNode = env->ordinal10170(block);
    uint32_t freeArg = (uint32_t)(uintptr_t)block;
    while (listNode != 0) {
        const uint32_t id = env->ordinal10727(listNode);
        if (game == nullptr) d2game_fatal(0x2D3);

        uint8_t* found = nullptr;
        for (uint8_t* n = (uint8_t*)(uintptr_t)rd32(game, 0x1720 + (id & 0x7F) * 4);
             n != nullptr; n = (uint8_t*)(uintptr_t)rd32(n, 0xE4)) {
            if (rd32(n, 0x0C) == id) { found = n; break; }
        }

        if (found != nullptr) {
            if (env->ordinal11051(found, 0x10) != 0 || env->ordinal11051(found, 0x4000) != 0) {
                env->ordinal10058(found, 0);
            }
            if (env->ordinal11051(found, 0x20) != 0 &&
                env->ordinal10202(found, 0x80, 0x180, kInvModeFile) != 0) {
                env->ordinal10058(found, 0);
            }
            env->sub9B340();

            if ((rd32(found, 0xC8) & 1) != 0) {
                env->ordinal10201(found, 0);
                uint8_t* nested = (uint8_t*)(uintptr_t)rd32(found, 0x60);
                if (nested == nullptr) d2game_fatal(0x1A4);
                for (uint32_t n = env->ordinal10170(nested); n != 0; n = env->ordinal10691(n)) {
                    const uint32_t nid = env->ordinal10727(n);
                    for (uint8_t* u = (uint8_t*)(uintptr_t)rd32(game, 0x1720 + (nid & 0x7F) * 4);
                         u != nullptr; u = (uint8_t*)(uintptr_t)rd32(u, 0xE4)) {
                        if (rd32(u, 0x0C) == nid) { env->sub9B340(); break; }
                    }
                }
                env->ordinal10545(0);                  // iStack_1C
            }
            if (env->ordinal11051(found, 1) != 0) {
                env->subDFC10(found);
                env->subE20A0(game, found);
            }
        }

        listNode = env->ordinal10691(listNode);
        freeArg = (uint32_t)(uintptr_t)esiSaved;       // unaff_ESI restored each iteration
    }
    env->ordinal10545(freeArg);
}

// 1.13c D2Game+0xB27D0
// The in_EAX record must be non-null with its first dword clear (fatal 0xC5). The pointer from
// ordinal 10920 must hold a +0x34 block (fatal 0xC9); that block's second dword is cleared.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_B27D0(void* record, const D2GameBatchBd18Env* env)
{
    if (record == nullptr || rd32(record, 0) != 0) d2game_fatal(0xC5);
    uint8_t* state = (uint8_t*)env->ordinal10920();
    uint8_t* block = (uint8_t*)(uintptr_t)rd32(state, 0x34);
    if (block == nullptr) d2game_fatal(0xC9);
    wr32(block, 4, 0);
}

// The slot-state send: d2game_9d610.cpp is its own file.
