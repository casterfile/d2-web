// src/core/d2game/d2game_batch_cc030.cpp -- eleven 1.13c D2Game.dll functions transcribed from the
// cached Ghidra decompiles (build-wasm/ghidra-d2game/decomp/0x<RVA>.c): the object timer-arg chain
// free tail, the player intro/aux free tail, ACT5Q5's portal-closed bookkeeping, the AI control free,
// the inventory-item removal tail, ARENA_FreeArenaUnit, PLAYERLIST_FreePlayerLists, the unit-list
// unlink, the player-interact cleanup, the DRLG room cleanup and the client/player update sync.
// Base 0x6FC20000.
//
// Retail module state is opaque host records read by their retail byte offsets (rd8/rd32/rdptr,
// wr32/wrptr). Every unmapped callee is a seam in D2GameBatch15Env, in retail argument order; a
// Ghidra call that passes arguments only in registers gets an explicit parameter, noted at the seam.
// The fatal path Ordinal_10265(code) -> Ordinal_10024(&0x6FD19FF9) -> _exit(-1) is d2game_fatal.
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <unistd.h>
#include <emscripten.h>

static inline uint8_t rd8(const void* p, uint32_t o) { return ((const uint8_t*)p)[o]; }
static inline uint32_t rd32(const void* p, uint32_t o) { uint32_t v; std::memcpy(&v, (const uint8_t*)p + o, 4); return v; }
static inline void wr32(void* p, uint32_t o, uint32_t v) { std::memcpy((uint8_t*)p + o, &v, 4); }
static inline void* rdptr(const void* p, uint32_t o) { void* v; std::memcpy(&v, (const uint8_t*)p + o, sizeof v); return v; }

[[noreturn]] static void d2game_fatal(uint32_t) { _exit(-1); }

// The 1.13c unit-list head table at VA 0x6FD1B014, indexed by unit type; TILE (5) lives at
// game+0x1B20. Matches the values in retail's .rdata.
static const uint32_t kUnitListHead[5] = {0x1120, 0x1320, 0x1520, 0x1920, 0x1720};

// Host seams for the callees the cached decompiles name but src/ does not implement.
struct D2GameBatch15Env {
    // Ordinal_10046 FOG_FreePool / D2_FREE_POOL: the pool and pointer arrive in registers, the file
    // and line are the stack arguments Ghidra shows.
    void     (*freePool)(void* pool, void* ptr, uint32_t line);
    // Ordinal_10920 UNITS_GetPlayerData(unit), the unit in EAX.
    void*    (*getPlayerData)(void* unit);
    // Ordinal_10331 UNITS_GetRoom(unit), the unit in EAX.
    void*    (*getRoom)(void* unit);
    // Ordinal_10826 DUNGEON_GetLevelIdFromRoom(room).
    uint32_t (*getLevelIdFromRoom)(void* room);
    // Ordinal_10850 DRLGPRESET_FreeMapAI(pool, mapAi), the map in EAX.
    void     (*freeMapAi)(void* pool, void* mapAi);
    // 0x20B60 frees the AI command chain hanging off pLastCmd (game, aiControl).
    void     (*freeAiCommands)(void* game, void* aiControl);
    // 0x692D0 PLRINTRO_Destroy(game, intro).
    void     (*destroyIntro)(void* game, void* intro);
    // 0x37500 frees the playerData+0x98 chain (game pool, playerData, player).
    void     (*freeAuxChain)(void* game, void* playerData, void* player);
    // Ordinal_10284 INVENTORY_GetFirstItem(inventory).
    void*    (*inventoryGetFirstItem)(void* inventory);
    // Ordinal_10155 the inventory-item unit id (the item in EAX).
    uint32_t (*inventoryGetUnitId)(void* item);
    // Ordinal_10348 INVENTORY_GetNextItem(item).
    void*    (*inventoryGetNextItem)(void* item);
    // Ordinal_10885 DUNGEON_AllocDrlgDelete(room, unitType, unitId).
    void     (*drlgDelete)(void* room, uint32_t unitType, uint32_t unitId);
    // 0xE20A0 SUNIT_RemoveUnit-style list unlink (game, unit).
    void     (*removeUnitFromList)(void* game, void* unit);
    // 0xDFC10 SUNIT_IterateLivingPlayers(game, callback, player).
    void     (*iterateLivingPlayers)(void* game, void* player);
    // 0xE03A0 looks up an interact entry by id (game, id).
    void*    (*getInteract)(void* game, uint32_t id);
    // 0x68300 the interact-release call (unit argument in EAX).
    void     (*releaseInteract)(void* unit);
    // 0xAFAF0 builds/returns the interact payload (unit, game).
    void*    (*buildInteractPayload)(void* unit, void* game);
    // Ordinal_10181 returns the unit's timer-arg pointer (unit+0x7C); the unit is in EAX.
    void*    (*getTimerArg)(void* unit);
    // Ordinal_11090 (interact entry, 0).
    void*    (*interactClear)(void* entry, uint32_t zero);
    // Ordinal_10459 level/act table lookup, taking the byte at playerData+4.
    uint32_t (*levelLookup)(uint8_t act);
    // Ordinal_11010 / Ordinal_10053 DRLG lookups by (segment, x, y).
    void*    (*drlgGetRoom)(void* segment, uint32_t x, uint32_t y);
    void*    (*drlgFindRoom)(void* segment, uint32_t x, uint32_t y);
    // 0x4CB80 frees the DRLG room found by 0x10053.
    void     (*freeDrlgRoom)(void* game, void* room);
    // 0x49240 returns the per-client update record used by 0xCC030.
    void*    (*getUpdateRecord)(void);
    // Ordinal_10973(unit, stat, param) -- the trailing param is 0 when Ghidra shows none.
    uint32_t (*getStat)(void* unit, uint32_t stat, uint32_t param);
    // Ordinal_10913(unit).
    uint32_t (*getUnitValue)(void* unit);
    // 0x21100, 0x21130, 0xEB840, 0xEB720, 0xAB750, 0xAB7F0, 0xAB8C0, 0xABE10, 0xABDA0.
    void     (*unitUpdate0)(void* unit);
    void     (*unitUpdate1)(void* unit);
    uint32_t (*sendA)(void);
    uint32_t (*sendB)(void);
    uint32_t (*packetA)(void);
    uint32_t (*packetB)(void);
    uint32_t (*packetC)(void);
    void     (*packetSetA)(uint32_t a, uint32_t b);
    void     (*packetSetB)(uint32_t a, uint32_t b);
    // Ordinal_10983 / Ordinal_10764 on the value at client+0x2C.
    void     (*freeClientBlock)(uint32_t value);
    void     (*sentBlock)(uint32_t value);
};

// D2Game 1.13c 0x2E790: the retail-layout port is src/core/d2game/d2game_2e790.cpp
// FUN_6fc4e790: the tail of OBJECTS_FreeHoverMessage (Objects.cpp:0x1CE/0x1D1). Walk the chain
// whose head is *timerArg, each node's next at +4, freeing every node through the pool, then free
// the timer-arg block itself. Pool and timer-arg arrive in registers only.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_game_2e790(void* pool, void* timerArg, const D2GameBatch15Env* env)
{
    void* node = rdptr(timerArg, 0x00);
    while (node != nullptr) {
        void* next = rdptr(node, 0x04);
        env->freePool(pool, node, 0x1CE);                  // Ordinal_10046
        node = next;
    }
    env->freePool(pool, timerArg, 0x1D1);                  // Ordinal_10046
}

// 1.13c D2Game+0x376B0
// FUN_6fc576b0: the intro/aux tail of PLAYER_Destroy. The player data must exist; 0x37500 frees the
// unk0x98 chain, then the three pPlayerIntro[3] slots at +0x60 are destroyed and nulled.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_game_376b0(void* game, void* player, const D2GameBatch15Env* env)
{
    void* playerData = env->getPlayerData(player);         // Ordinal_10920
    if (playerData == nullptr) return;

    env->freeAuxChain(game, playerData, player);           // 0x37500
    for (uint32_t i = 0; i < 3; ++i) {
        env->destroyIntro(game, rdptr(playerData, 0x60 + 4 * i));  // 0x692D0
        wr32(playerData, 0x60 + 4 * i, 0);
    }
}

// 1.13c D2Game+0x56740
// FUN_6fc76740 ACT5Q5_OnPortalClosed(game in EAX, unit in the argument): only when the unit's room
// is LEVEL_ROCKYSUMMIT (0x78). Find quest 0x23 in the game's quest chain (pQuestControl at +0x10F4,
// first node at +0, next at +0xF4); decrement its nActivePortalCount (ex+0x0C) and, when it reaches
// zero with unk0x10 set, set bActivatable (ex+0x11).
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_game_56740(void* game, void* unit, const D2GameBatch15Env* env)
{
    void* room = env->getRoom(unit);                       // Ordinal_10331
    if (env->getLevelIdFromRoom(room) != 0x78) return;     // Ordinal_10826, LEVEL_ROCKYSUMMIT

    void* node = nullptr;
    void* control = rdptr(game, 0x10F4);
    if (control != nullptr) {
        for (node = rdptr(control, 0x00); node != nullptr; node = rdptr(node, 0xF4)) {
            if (rd32(node, 0x00) == 0x23) break;
        }
    }
    void* ex = rdptr(node, 6 * 4);
    wr32(ex, 0x0C, rd32(ex, 0x0C) - 1);
    if (rd32(ex, 0x0C) == 0 && rd8(ex, 0x10) != 0) {
        ((uint8_t*)ex)[0x11] = 1;
    }
}

// 1.13c D2Game+0x379E0
// FUN_6fc579e0: the inventory tail of PLAYER_Destroy. For every item of the player inventory
// (player+0x60): its unit id must be non-zero (fatal 0x2D3 otherwise), find the matching unit in
// the game's type-indexed list -- head at 0x1120 + (id & 0x7F) * 4, next at +0xE4 -- then delete
// its DRLG entry and unlink it (0xE20A0).
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_game_379e0(void* game, void* player, const D2GameBatch15Env* env)
{
    void* item = env->inventoryGetFirstItem(rdptr(player, 0x60));  // Ordinal_10284
    while (item != nullptr) {
        const uint32_t unitId = env->inventoryGetUnitId(item);     // Ordinal_10155
        if (game == nullptr) d2game_fatal(0x2D3);

        for (void* unit = rdptr(game, 0x1120 + (unitId & 0x7F) * 4); unit != nullptr;
             unit = rdptr(unit, 0xE4)) {
            if (rd32(unit, 0x0C) == unitId) {
                void* room = env->getRoom(unit);                   // Ordinal_10331
                if (room != nullptr) {
                    env->drlgDelete(room, rd32(unit, 0), rd32(unit, 0x0C)); // Ordinal_10885
                }
                env->removeUnitFromList(game, unit);               // 0xE20A0
                break;
            }
        }
        item = env->inventoryGetNextItem(item);                    // Ordinal_10348
    }
}

// 1.13c D2Game+0xB2480
// FUN_6fcd2480 ARENA_FreeArenaUnit(pGame in ECX, pPlayer in EAX): the player data is mandatory
// (fatal 0x52), and pArenaUnit (+0x34) is freed and cleared (Arena.cpp:0x57).
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_game_b2480(void* pool, void* player, const D2GameBatch15Env* env)
{
    void* playerData = env->getPlayerData(player);         // Ordinal_10920
    if (playerData == nullptr) d2game_fatal(0x52);

    if (rdptr(playerData, 0x34) != nullptr) {
        env->freePool(pool, rdptr(playerData, 0x34), 0x57); // Ordinal_10046
        wr32(playerData, 0x34, 0);
    }
}

// 1.13c D2Game+0xF0390
// FUN_6fd10390 PLAYERLIST_FreePlayerLists(pGame in EAX, pPlayer in the argument): 0xDFC10 first,
// then free every node of playerData->pPlayerList (+0x38, next +0x10) and clear the head
// (PlayerList.cpp:0x4E).
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_game_f0390(void* game, void* player, const D2GameBatch15Env* env)
{
    void* playerData = env->getPlayerData(player);         // Ordinal_10920
    env->iterateLivingPlayers(game, player);               // 0xDFC10

    void* node = rdptr(playerData, 0x38);
    while (node != nullptr) {
        void* next = rdptr(node, 0x10);
        env->freePool(rdptr(game, 0x1C), node, 0x4E);         // Ordinal_10046, pool = game+0x1C in ECX (was passed null)
        node = next;
    }
    wr32(playerData, 0x38, 0);
}

// 1.13c D2Game+0xE02A0
// FUN_6fd002a0 SUNIT_UnlinkFromUnitList(game in the argument, unit in ESI + EAX): pick the list
// head by the unit's type through 0x6FD1B014 (TILE at game+0x1B20), bucket by (unitId & 0x7F), then
// unlink the node; next pointer at +0xE4.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_game_e02a0(void* game, void* unit)
{
    const int32_t type = (int32_t)rd32(unit, 0x00);
    void* headPtr;
    if (type == 5) {
        headPtr = (uint8_t*)game + 0x1B20;
    } else if (type < 0 || type > 4) {
        return;                                            // no list: nothing to unlink
    } else {
        headPtr = (uint8_t*)game + kUnitListHead[type] + (rd32(unit, 0x0C) & 0x7F) * 4;
    }

    void* node = rdptr(headPtr, 0);
    void* previous = nullptr;
    while (node != nullptr && node != unit) {
        previous = node;
        node = rdptr(node, 0xE4);
    }
    if (node == nullptr) return;
    if (previous == nullptr) {
        wr32(headPtr, 0, (uint32_t)(uintptr_t)rdptr(node, 0xE4));
    } else {
        wr32(previous, 0xE4, (uint32_t)(uintptr_t)rdptr(node, 0xE4));
    }
}

// 1.13c D2Game+0xE0BF0
// FUN_6fd00bf0: player interact cleanup (game in the argument, unit in EAX). When unit+0x6C is set,
// look up game+0x64's interact entry: class 1 releases (0x68300) and rebuilds (0xAFAF0); class > 2
// is a no-op; class 2 resolves entry id 0x151 through Ordinal_10181 (clearing *pTimerArg to -1) or
// else Ordinal_11090. On success clear unit+0x64 to -1, +0x68 to 6 and +0x6C.
extern "C"
EMSCRIPTEN_KEEPALIVE void* d2_game_e0bf0(void* game, void* unit, const D2GameBatch15Env* env)
{
    void* result = rdptr(unit, 0x68);
    if (rd8(unit, 0x6C) == 0) return result;

    void* entry = env->getInteract(game, rd32(unit, 0x64)); // 0xE03A0
    result = nullptr;
    if (entry == nullptr) return result;

    if (rd32(entry, 0) == 1) {
        env->releaseInteract(unit);                         // 0x68300
        result = env->buildInteractPayload(unit, game);     // 0xAFAF0
    } else {
        const uint32_t cls = rd32(entry, 0) - 2;
        if (cls == 0) {
            if (rd32(entry, 4) == 0x151) {
                void* arg = env->getTimerArg(unit);         // Ordinal_10181
                void* ptr = rdptr(arg, 0);
                result = ptr;
                if (ptr != nullptr) wr32(ptr, 0, 0xFFFFFFFF);
            } else {
                result = env->interactClear(entry, 0);      // Ordinal_11090
            }
        } else {
            result = (void*)(uintptr_t)cls;
        }
    }
    wr32(unit, 0x64, 0xFFFFFFFF);
    wr32(unit, 0x68, 6);
    ((uint8_t*)unit)[0x6C] = 0;
    return result;
}

// 1.13c D2Game+0xE1570
// FUN_6fd01570: room cleanup for a unit (game-pointer parameter in EAX/ESI, unit in the argument;
// NULL unit is the 0x78B fatal). Level from the byte at playerData+4, coordinates at
// playerData+0x18/+0x1C; try Ordinal_11010 then Ordinal_10053 on the game's per-act room table at
// +0xBC, free the room (0x4CB80), then run the interact cleanup 0xE03A0 on unit+0x98.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_game_e1570(void* game, void* unit, const D2GameBatch15Env* env)
{
    if (unit == nullptr) d2game_fatal(0x78B);

    void* playerData = rdptr(unit, 0x14);
    const uint32_t level = env->levelLookup(rd8(playerData, 4));          // Ordinal_10459
    const uint32_t y = rd32(playerData, 0x1C);
    const uint32_t x = rd32(playerData, 0x18);
    void* segment = rdptr(game, 0xBC + (level & 0xFF) * 4);

    void* room = env->drlgGetRoom(segment, x, y);                        // Ordinal_11010
    if (room == nullptr) {
        room = env->drlgFindRoom(segment, x, y);                         // Ordinal_10053
        if (room != nullptr) env->freeDrlgRoom(game, room);              // 0x4CB80
    }
    env->getInteract(game, rd32(unit, 0x98));                            // 0xE03A0
}

// D2Game 1.13c 0xCC030: the retail-layout port is src/core/d2game/d2game_cc030.cpp (EAX = player, ECX = unit, stack flag).

// ---- FUNCTION-mode probe for 0x56740 (src/core/fn/d2game_56740.cpp): each seam logs through HookEnv as the capture's hook recorder does and
// returns the scripted value; the quest-node walk reads the real blob.
#include <emscripten.h>
#include "../fn/hookenv.h"
static HookEnv* q15H;
void D2G15_56740_probe(void* game, void* unit, uint32_t* lg, const uint32_t* sc)
{
    HookEnv h(lg, sc); q15H = &h;
    D2GameBatch15Env e{};
    e.getRoom = [](void* u) -> void* { return (void*)(uintptr_t)q15H->h(0xA478, 0, 0, 0, 0, 0, (uint32_t)(uintptr_t)u); };
    e.getLevelIdFromRoom = [](void* r) -> uint32_t { return q15H->h(0xA556, 0, 0, 0, 0, 0, (uint32_t)(uintptr_t)r); };
    d2_game_56740(game, unit, &e);
}

// ---- FUNCTION-mode probes for 0xB2480 and 0xF0390: the seams log
// through HookEnv as the capture's hook recorder does and hand back the blobs the descriptor pins.
static HookEnv* g15H; static uint32_t g15Pd;
static void* g15GetPd(void* u) { g15H->h(0xA4C0, 0, 0, 0, 0, 0, (uint32_t)(uintptr_t)u); return (void*)(uintptr_t)g15Pd; }
static void g15Free(void* pool, void* p, uint32_t line) {
    g15H->h(0xA2EC, 0, (uint32_t)(uintptr_t)pool, (uint32_t)(uintptr_t)p, 0, 0, 0xB20FB568u, line, 0); }
// 0xB2480: r[0] player (EAX), r[1] game (stack), r[2] player data blob, r[3] hook log
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_b2480(uint32_t* r)
{
    static uint32_t none[2] = {0, 0};
    HookEnv he((uint32_t*)(uintptr_t)r[3], none); g15H = &he; g15Pd = r[2];
    D2GameBatch15Env e{}; e.getPlayerData = g15GetPd; e.freePool = g15Free;
    d2_game_b2480(rdptr((void*)(uintptr_t)r[1], 0x1C), (void*)(uintptr_t)r[0], &e);
    r[4] = 0;
}
// 0xF0390: r[0] player (EAX), r[1] game (ECX), r[2] player data blob, r[3] hook log. 0xDFC10 takes the player on the stack, the game in EDI and the 0xF0040 callback in EBX.
static void g15Iter(void* game, void* player) {
    g15H->ed((uint32_t)(uintptr_t)game);
    g15H->h(0xDFC10, 0, 0, 0, 0xB20F0040u, 0, (uint32_t)(uintptr_t)player);
    g15H->edi(0xDFC10, (uint32_t)(uintptr_t)game); }
static void g15Free4E(void* pool, void* p, uint32_t line) {
    g15H->h(0xA2EC, 0, (uint32_t)(uintptr_t)pool, (uint32_t)(uintptr_t)p, 0, 0, 0xB20FB9A4u, line, 0); }
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_f0390(uint32_t* r)
{
    static uint32_t none[2] = {0, 0};
    HookEnv he((uint32_t*)(uintptr_t)r[3], none); g15H = &he; g15Pd = r[2];
    D2GameBatch15Env e{}; e.getPlayerData = g15GetPd; e.freePool = g15Free4E; e.iterateLivingPlayers = g15Iter;
    d2_game_f0390((void*)(uintptr_t)r[1], (void*)(uintptr_t)r[0], &e);
    r[4] = 0;
}
