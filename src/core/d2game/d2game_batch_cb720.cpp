// src/core/d2game/d2game_batch_cb720.cpp -- six 1.13c D2Game.dll functions transcribed from the
// cached Ghidra decompiles (build-wasm/ghidra-d2game/decomp/0x<RVA>.c): the two stat-percentage
// helpers called by the client update sync (0xCC030), the nearest-unit list serialiser/collector
// pair (0x4A370/0x4A460), the quest/act record lookup (0xD1AA0) and the interact-record probe
// (0xE09D0). Base 0x6FC20000.
//
// Retail module state is opaque host records read by their retail byte offsets (rd8/rd16/rd32,
// wr16/wr32, rdptr). Every unmapped callee is a seam in D2GameBatchCB720Env, in retail argument
// order. A Ghidra call whose arguments live only in registers was dropped to a no-argument call in
// the decompile; those seams take only the arguments the decompile still shows and say so.
// The fatal path Ordinal_10265(code) -> Ordinal_10024(&0x6FD19FF9) -> _exit(-1) is d2game_fatal.
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <unistd.h>
#include <emscripten.h>

static inline uint8_t rd8(const void* p, uint32_t o) { return ((const uint8_t*)p)[o]; }
static inline uint16_t rd16(const void* p, uint32_t o) { uint16_t v; std::memcpy(&v, (const uint8_t*)p + o, 2); return v; }
static inline uint32_t rd32(const void* p, uint32_t o) { uint32_t v; std::memcpy(&v, (const uint8_t*)p + o, 4); return v; }
static inline void wr16(void* p, uint32_t o, uint16_t v) { std::memcpy((uint8_t*)p + o, &v, 2); }
static inline void wr32(void* p, uint32_t o, uint32_t v) { std::memcpy((uint8_t*)p + o, &v, 4); }
static inline void* rdptr(const void* p, uint32_t o) { void* v; std::memcpy(&v, (const uint8_t*)p + o, sizeof v); return v; }

[[noreturn]] static void d2game_fatal(uint32_t) { _exit(-1); }

// Host seams for the callees the cached decompiles name but src/ does not implement.
struct D2GameBatchCB720Env {
    // sgptDataTables (the module's data-table pointer): +0xBC4 is the monster-data array base,
    // +0xBC8 its element count; the records are 0xC4 bytes apart.
    void* dataTables;
    // Ordinal_10973 STATLIST_GetStatValue(unit, stat, param). Register-only arguments were
    // dropped by the decompiler, so the seam takes none and the host supplies the call site.
    uint32_t (*ordinal10973)(void);
    // Ordinal_10186.
    int32_t  (*ordinal10186)(void);
    // Ordinal_10871.
    uint32_t (*ordinal10871)(void);
    // Ordinal_10959(value).
    int32_t  (*ordinal10959)(uint32_t value);
    // Ordinal_10913(unit), the unit in EAX.
    int32_t  (*ordinal10913)(void);
    // Ordinal_10680(unit, stat, param).
    int32_t  (*ordinal10680)(uint32_t a, uint32_t b, uint32_t c);
    // Ordinal_10867(pUnitData) -- 0x4A460 uses it as the unit's X coordinate.
    uint32_t (*ordinal10867)(uint32_t unitData);
    // Ordinal_10750(pUnitData) -- 0x4A460 uses it as the unit's Y coordinate.
    uint32_t (*ordinal10750)(uint32_t unitData);
    // 0xDF780 returns the record whose +0xA8 word 0xCB720 subtracts.
    void*    (*funcDF780)(void);
    // Ordinal_10744, called four times by 0xD1AA0 with register-only arguments.
    void*    (*ordinal10744)(void);
    // Ordinal_11028 returns the id 0xD1AA0 looks up.
    uint16_t (*ordinal11028)(void);
    // Ordinal_11065(id).
    void*    (*ordinal11065)(uint16_t id);
    // 0xE03A0 interact-record lookup (0xE09D0 passes the stack argument and unit+0xA0).
    int32_t  (*funcE03A0)(uint32_t a, uint32_t id);
};

// FUN_6fceb720: stat percentage for the unit in EAX. Requires the Ordinal_10871 value,
// Ordinal_10186's count and a unit whose +4 class id indexes the 0xC4-stride monster table. The
// count is scaled by a quarter-second count derived from the record's +0x3A byte (0 -> 0x1D4C)
// and the result is clamped to 100. The Ordinal_10973 call whose result is discarded runs before
// the guards, as in retail.
// 1.13c D2Game+0xCB720
extern "C"
EMSCRIPTEN_KEEPALIVE uint32_t d2_game_cb720(void* unit, const D2GameBatchCB720Env* env)
{
    const uint32_t bias = env->ordinal10973();          // stat 8: added to the product before the x100
    const int32_t iVar1 = env->ordinal10186();
    uint32_t uVar2 = env->ordinal10871();
    if (uVar2 == 0 || iVar1 == 0 || unit == nullptr) return uVar2 & 0xFFFFFF00u;
    const uint32_t stat6a = uVar2;                      // stat 0x6a: the value 0xAB74 is asked about

    uVar2 = rd32(unit, 4);
    const int32_t count = (int32_t)rd32(env->dataTables, 0xBC8);
    if ((int32_t)uVar2 < 0 || count <= (int32_t)uVar2) return uVar2 & 0xFFFFFF00u;

    const uintptr_t record = (uintptr_t)rdptr(env->dataTables, 0xBC4) + (uintptr_t)(uVar2 * 0xC4);
    if (record == 0) return 0;

    int32_t div = (int32_t)rd8((const void*)record, 0x3A) * 0x19;
    if (div == 0) div = 0x1D4C;
    int32_t iVar7 = 1;
    if (0 < iVar1 / div) iVar7 = iVar1 / div;

    const int32_t s1 = (int32_t)env->ordinal10973();
    const int32_t s2 = (int32_t)env->ordinal10973();
    void* iVar5 = env->funcDF780();
    const int32_t iVar6 = env->ordinal10959(stat6a);

    const int32_t base = (int32_t)((uint32_t)iVar6 - rd32(iVar5, 0xA8));
    const int32_t scaled = (int32_t)(((uint32_t)s1 + 100u) * (uint32_t)iVar7) / 100;
    const int32_t mult = s2 + scaled;
    const uint32_t prod = ((uint32_t)base * (uint32_t)mult + bias) * 100u;
    const uint32_t uVar3 = (uint32_t)((int32_t)prod / iVar1);
    if (100 < (uint8_t)uVar3) return 100;
    return uVar3 & 0xFF;
}

// FUN_6fceb840: the companion stat percentage. The unit is in EAX; the added bias word is the first
// Ordinal_10973 result (a stack local). Fatal 0xA2A without a unit; the denominator is Ordinal_10913 >> 8.
// 1.13c D2Game+0xCB840
extern "C"
EMSCRIPTEN_KEEPALIVE uint32_t d2_game_cb840(void* unit, const D2GameBatchCB720Env* env)
{
    const int32_t ebpValue = (int32_t)env->ordinal10973();
    const int32_t iVar2 = env->ordinal10913();
    uint32_t uVar3 = env->ordinal10871();
    if (uVar3 == 0 || (iVar2 >> 8) == 0) return uVar3 & 0xFFFFFF00u;
    if (unit == nullptr) d2game_fatal(0xA2A);

    const int32_t iVar5 = env->ordinal10959(uVar3);
    const int32_t iVar1 = (int32_t)rd32(rdptr(unit, 0x80), 0xA8);
    const int32_t iVar6 = env->ordinal10680(uVar3, 0x4A, 0);

    int32_t t = iVar6 * (int32_t)((uint32_t)iVar5 - (uint32_t)iVar1);
    t = (int32_t)((uint32_t)t + (uint32_t)ebpValue);
    t = t >> 8;
    const uint32_t res = (uint32_t)(t * 100 / (iVar2 >> 8));
    if (100 < (uint8_t)res) return 100;
    return res & 0xFF;
}

// FUN_6fc6a370: serialise the selected-unit chain starting at param_1 into the record in ESI.
// Each chain node is three ints: [0] unit pointer, [1] distance, [2] next-node index (0 ends the
// walk and is not itself processed). Each output entry is 9 bytes: +0 = unit type byte (6 when the
// node is empty), +1 = unit id dword (-1 when empty), +5 = X word, +7 = Y word. The count is at
// record+0x3BC, the array at record+0x1CC.
// 1.13c D2Game+0x4A370
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_game_4a370(int32_t* node, void* record, const D2GameBatchCB720Env* env)
{
    wr32(record, 0x3BC, 0);
    int32_t* cur = node;
    do {
        uint32_t count = rd32(record, 0x3BC);
        void* item = rdptr(cur, 0x00);

        uint8_t typeByte = 6;
        if (item != nullptr) typeByte = rd8(item, 0);
        ((uint8_t*)record)[count * 9 + 0x1CC] = typeByte;

        uint32_t unitId = 0xFFFFFFFF;
        if (item != nullptr) unitId = rd32(item, 0x0C);
        wr32(record, count * 9 + 0x1CD, unitId);

        item = rdptr(cur, 0x00);
        const int32_t type = (int32_t)rd32(item, 0x00);
        const uint32_t data = (uint32_t)(uintptr_t)rdptr(item, 0x2C);
        uint16_t x;
        if (type == 2 || (3 < type && type < 6)) x = rd16((const void*)(uintptr_t)data, 0x0C);
        else if (data == 0) x = 0;
        else x = (uint16_t)env->ordinal10867(data);
        wr16(record, count * 9 + 0x1D1, x);

        item = rdptr(cur, 0x00);
        const int32_t type2 = (int32_t)rd32(item, 0x00);
        const uint32_t data2 = (uint32_t)(uintptr_t)rdptr(item, 0x2C);
        uint16_t y;
        if (type2 == 2 || (3 < type2 && type2 < 6)) y = rd16((const void*)(uintptr_t)data2, 0x10);
        else if (data2 == 0) y = 0;
        else y = (uint16_t)env->ordinal10750(data2);
        wr16(record, count * 9 + 0x1D3, y);

        wr32(record, 0x3BC, count + 1);
        cur = node + (int32_t)rd32(cur, 0x08) * 3;
    } while (rd32(cur, 0x08) != 0);
}

// FUN_6fc6a460: insert a unit into the sorted nearest-unit list. Only player units (type 0) are
// candidates. The unit is param_1; x arrives in EAX, y in param_2. The list base (ESI) is an
// int[3] array whose slot 0 is the head node: [0] unit pointer, [1] Manhattan distance, [2] next
// slot index. maxCount is param_5, the running best distance is *param_3, the collected count is
// record+0x3BC. When the list is full the farthest entry is dropped if this unit is closer; a new
// head is inserted when it beats the current head.
// 1.13c D2Game+0x4A460
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_game_4a460(int32_t* unit, int32_t y, uint32_t* best, void* record,
                                        uint32_t maxCount, int32_t* base, int32_t x,
                                        const D2GameBatchCB720Env* env)
{
    if (unit == nullptr || (int32_t)rd32(unit, 0) != 0) return;

    const uint32_t data = (uint32_t)(uintptr_t)rdptr(unit, 0x2C);
    int32_t dx = (data == 0) ? 0 : (int32_t)env->ordinal10867(data);
    dx = x - dx;
    if (dx < 0) dx = -dx;

    const int32_t type = (int32_t)rd32(unit, 0);
    int32_t dy;
    if (type == 2 || (3 < type && type < 6)) dy = (int32_t)rd32((const void*)(uintptr_t)data, 0x10);
    else if (data == 0) dy = 0;
    else dy = (int32_t)env->ordinal10750(data);
    dy = y - dy;
    if (dy < 0) dy = -dy;

    const uint32_t count = rd32(record, 0x3BC);
    const uint32_t dist = (uint32_t)dx + (uint32_t)dy;
    if (count < maxCount || dist < *best) {
        int32_t prev = 0;
        int32_t cur = base[2];
        int32_t* slotPtr = base + 2;
        while (cur != 0) {
            const int32_t idx = *slotPtr;
            if (!((uint32_t)base[idx * 3 + 1] < dist)) break;
            cur = base[idx * 3 + 2];
            slotPtr = base + idx * 3 + 2;
            prev = idx;
        }

        uint32_t slot;
        if (count == maxCount) {
            int32_t* p3 = base + 2;
            int32_t i6 = 0;
            int32_t nxt = base[base[2] * 3 + 2];
            while (nxt != 0) {
                i6 = *p3;
                p3 = base + i6 * 3 + 2;
                nxt = base[(*p3) * 3 + 2];
            }
            const uint32_t last = (uint32_t)base[i6 * 3 + 2];
            if (count != 0 && (uint32_t)base[last * 3 + 1] <= dist) return;
            if (count == 1) {
                *best = dist;
            } else {
                *best = (uint32_t)base[i6 * 3 + 1];
            }
            base[i6 * 3 + 2] = 0;
            slot = last;
        } else {
            wr32(record, 0x3BC, count + 1);
            slot = count;
        }

        if (prev == 0 && dist < (uint32_t)base[1]) {
            if ((int32_t)rd32(record, 0x3BC) != 1) {
                int32_t* dst = base + (int32_t)slot * 3;
                dst[2] = base[2];
                dst[0] = base[0];
                dst[1] = base[1];
            }
            base[2] = (int32_t)slot;
            slot = 0;
        } else {
            base[slot * 3 + 2] = base[prev * 3 + 2];
            base[prev * 3 + 2] = (int32_t)slot;
        }
        base[slot * 3 + 1] = (int32_t)dist;
        base[slot * 3] = (int32_t)(uintptr_t)unit;
        if (*best < dist) *best = dist;
    }
}

// FUN_6fcf1aa0: resolve a record value. The game pointer is in ESI (fatal 0x1ED when null). Two
// Ordinal_10744 probes must both survive; then the Ordinal_11028 id selects an Ordinal_11065
// record, whose +8 word is returned when the next 10744 probe is set and whose +0xC word when the
// following one is. Anything missing returns 0xFFFFFFFF.
// 1.13c D2Game+0xD1AA0
extern "C"
EMSCRIPTEN_KEEPALIVE uint32_t d2_game_d1aa0(void* game, const D2GameBatchCB720Env* env)
{
    if (game == nullptr) d2game_fatal(0x1ED);

    void* probe = env->ordinal10744();
    if (probe == nullptr) {
        probe = env->ordinal10744();
        if (probe == nullptr) return 0xFFFFFFFF;
    }

    const uint16_t id = env->ordinal11028();
    void* record = env->ordinal11065(id);
    if (record != nullptr) {
        if (env->ordinal10744() != nullptr) return rd32(record, 8);
        if (env->ordinal10744() != nullptr) return rd32(record, 0xC);
    }
    return 0xFFFFFFFF;
}

// FUN_6fd009d0: interact-record probe. The context unit is in EAX (fatal 0x321 when null); when
// its +0xC8 flags carry 0x20000, dispatch to the 0xE03A0 lookup with the stack argument and the
// unit's +0xA0 id; otherwise return 0.
// 1.13c D2Game+0xE09D0
extern "C"
EMSCRIPTEN_KEEPALIVE int32_t d2_game_e09d0(uint32_t param_1, void* unit, const D2GameBatchCB720Env* env)
{
    if (unit == nullptr) d2game_fatal(0x321);
    if ((rd32(unit, 200) & 0x20000) != 0) {
        return env->funcE03A0(param_1, rd32(unit, 0xA0));
    }
    return 0;
}
