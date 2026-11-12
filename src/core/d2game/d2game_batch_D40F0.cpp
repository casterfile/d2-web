// src/core/d2game/d2game_batch_D40F0.cpp -- twelve 1.13c D2Game.dll functions transcribed from the
// cached Ghidra decompiles (build-wasm/ghidra-d2game/decomp/0x<RVA>.c), in RVA order: the pet-list
// allocator and pet add (0x376F0, 0x97120, 0x97A20), the start-item creator (0x38470), the unit flag
// fan-out (0x6B200), the three missile-finalize steps (0x6F620, 0x6F660, 0x6F8A0), the item->client
// skill pass (0x79100), the 0x3E stat packet writer (0x8B010), and the delayed-unit update fan-out
// (0xD40F0, 0xD4550). Stage A batch lane (OBJECTIVE v9): implementation + marker only, no spec, no
// vector.
//
// Host model: a unit, item, game, client, pet list or data-table block is opaque and addressed by
// retail 32-bit byte offsets (rd8/rd16/rd32/rdptr, wr8/wr32). Register-only arguments (Ghidra's
// in_EAX/in_ECX/unaff_EDI/unaff_EBX/unaff_ESI) are explicit parameters here and are named in each
// function's comment. The resolved sgptDataTables block, the gdwBitMasks image and the pet-type
// offset table (DAT_6FD1A3DC) are Env pointers; every callee outside this batch is a seam pointer on
// D2GameBatchD40F0Env in retail argument order. Fog/D2Common ordinals are the 1.13c numbers from the
// decompiles and stay opaque. The fatal path Fog #10265(id) -> #10024(text) -> _exit(-1) is
// d2game_fatal, as in the other batches.
// Prior art: D2MOO 1.10f reference/D2MOO/source/D2Game/src/PLAYER/PlayerPets.cpp
// (PLAYERPETS_AllocPetList at 0x6FC7CAF0, sub_6FC7DA40 at 0x6FC7DA40),
// reference/D2MOO/source/D2Game/src/PLAYER/Player.cpp (PLAYER_CreateStartItem at 0x6FC7B800,
// PLAYER_CreateStartItemsFromCharStatsTxt at 0x6FC7BC30) and
// reference/D2MOO/source/D2Game/src/GAME/SCmd.cpp (D2GAME_PACKETS_SendPacket0x3E_6FC3EC20 at
// 0x6FC3EC20, the 1.10f form of 0x8B010).
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
static inline void wr32(void* p, uint32_t o, uint32_t v) { std::memcpy((uint8_t*)p + o, &v, sizeof v); }

[[noreturn]] static void d2game_fatal(uint32_t) { _exit(-1); }

// Host seams for the callees the cached decompiles name but src/ does not implement. The comment on
// each field is the retail callee (D2Game RVA, Fog/D2Common ordinal, or the D2MOO name it maps to).
struct D2GameBatchD40F0Env {
    // sgptDataTables: PetTypeTxt record count at +0xBF0.
    const uint8_t* tables;
    // gdwBitMasks (D2Game+0xF8BE0), 32 dwords, retail indexes it by byte offset.
    const uint8_t* bitMasks;
    // DAT_6FD1A3DC: per-unit-type byte offset into the game's pet-list array (game+0xB8 -> +0xA04).
    const uint32_t* petTypeOffsets;

    // 0xD40F0
    void     (*sub4AD10)(uint32_t value);                       // 0x4AD10, called with 0
    void     (*sub8E030)(uint32_t unit);                        // 0x8E030
    void     (*sub8DFD0)(uint32_t unit);                        // 0x8DFD0
    // 0x6B200
    void     (*sub6A0D0)();                                     // 0x6A0D0
    void     (*sub7AC50)();                                     // 0x7AC50
    void     (*sub7ACB0)(int32_t arg);                          // 0x7ACB0
    void     (*sub9BB10)(uint32_t zero, uint32_t unit);         // 0x9BB10
    void     (*sub7ADF0)(void* self, uint32_t unit, int32_t arg); // 0x7ADF0
    // 0x8B010 -- the 1.13c form of the 1.10f D2BitBuffer calls.
    void     (*bitWriterInit)(void* writer, void* bytes, uint32_t size);  // Fog #10126
    void     (*bitWriterWrite)(void* writer, uint32_t value, uint32_t nBits); // Fog #10128
    uint32_t (*bitWriterFlush)(void* writer);                   // Fog #10127
    void     (*bitWriteVar)(void* writer, uint32_t value);      // 0x1CD0 variable-width field
    void     (*sendPacket)(const void* packet, uint32_t length); // 0x8A3E0
    // 0x79100
    void     (*sub778A0)();                                     // 0x778A0
    int32_t  (*ordinal11139)(uint32_t handle, int32_t n);       // #11139
    void     (*sub78DD0)(uint32_t param, uint32_t unit);        // 0x78DD0
    void     (*sub77B20)();                                     // 0x77B20
    // 0x6F620
    void     (*sub37540)();                                     // 0x37540
    void     (*ordinal10413)(uint8_t* out);                     // #10413, 124-byte out buffer
    void     (*ordinal10484)();                                 // #10484
    // 0x6F660
    uint32_t (*sub1250)();                                      // 0x1250 (missile id in EAX)
    int32_t  (*ordinal10910)(uint32_t unit);                    // #10910 (unit in EAX)
    int32_t  (*ordinal10973)(uint32_t unit);                    // #10973 (unit in EAX)
    void     (*ordinal10587)();                                 // #10587
    void     (*ordinal10614)();                                 // #10614
    void     (*ordinal10887)(void* unit, uint32_t stat, uint32_t value, uint32_t layer); // #10887
    // 0x6F8A0
    void     (*ordinal10098)(uint32_t unit);                    // #10098 (unit in EAX)
    void     (*sub8E880)(uint32_t unit);                        // 0x8E880
    void*    (*sub8E680)(uint32_t unit);                        // 0x8E680
    // 0x97120 / 0x97A20
    void*    (*allocPool)(const char* file, uint32_t line);     // Fog #10045 (pool/size in regs)
    void     (*sub96FA0)(uint32_t game, uint32_t player);       // 0x96FA0
    void     (*sub977E0)(uint32_t game, uint32_t guid);         // 0x977E0
    void     (*sub97920)(uint32_t guid, void* list, uint32_t flag); // 0x97920
    void     (*sub97640)(uint32_t game, uint32_t player);       // 0x97640 (free pets, VERIFIED)
    void     (*sub96F40)(uint32_t game);                        // 0x96F40
    // 0x38470
    int32_t  (*ordinal10450)(uint32_t itemCode, int32_t* outItemId); // #10450, item-text lookup
    void*    (*sub38090)(uint32_t player, uint32_t itemId, uint32_t esi); // 0x38090
    void     (*ordinal10977)(void* item, uint32_t flag, uint32_t value); // #10977 set item flag
    void     (*ordinal10058)(void* item, uint32_t bodyLoc);     // #10058 set body location
    int32_t  (*ordinal10792)(void* item);                       // #10792
    void     (*sub383F0)();                                     // 0x383F0
    int32_t  (*ordinal10744)(void* item, int32_t n);            // #10744
    uint32_t (*ordinal11116)(void* item, uint32_t n);           // #11116
    uint8_t  (*sub37F20)();                                     // 0x37F20
    uint8_t  (*sub37EC0)(void* item, uint32_t player, uint32_t n); // 0x37EC0
    // 0x376F0
    uint32_t (*ordinal10920)(uint32_t player);                  // #10920 player-data accessor
    uint32_t (*sub69360)();                                     // 0x69360
};

// 1.13c D2Game+0x376F0
// Player-data slot initialisation: EAX = player (register-only). Ordinal_10920 returns the player
// data block; three consecutive dwords at +0x60 are filled from 0x69360 and the dword at +0x6C is
// cleared.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2game_port_376f0(uint32_t eaxPlayer, const D2GameBatchD40F0Env* env)
{
    const uint32_t playerData = env->ordinal10920(eaxPlayer);
    if (playerData == 0) return;
    void* p = (void*)(uintptr_t)(playerData + 0x60);
    for (int32_t i = 3; i != 0; --i) {
        wr32(p, 0, env->sub69360());
        p = (uint8_t*)p + 4;
    }
    wr32((void*)(uintptr_t)playerData, 0x6C, 0);
}

// 1.13c D2Game+0x38470
// Start-item creation from one D2CharItemStrc (dwItemCode +0x00, nBodyLoc +0x04, nItemCount +0x05;
// the comments reads the loop bound through a coalesced register as param_2+5 -- same field).
// param_1 (ECX) is the player, param_2 (EDX) the game, param_3 unused, param_4 the item entry and
// ESI the register-only CreateItemUnit argument. Ordinal_10450 maps the item code to an ItemTxt id;
// the loop creates nItemCount units, sets IFLAG_STARTITEM (0x20000), the body location, then either
// places the item lazily (0x383F0) or gives it durability/quantity stats (0x10887). The 1.10f form
// is PLAYER_CreateStartItem_6FC7B800.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2game_38470_impl(uint32_t param1, uint32_t param2, uint32_t param3,
                                             const uint32_t* param4, uint32_t esi,
                                             const D2GameBatchD40F0Env* env)
{
    int32_t itemId = 0;
    const int32_t found = env->ordinal10450(rd32(param4, 0), &itemId);
    if (found == 0 || rd8(param4, 5) == 0) return;
    const uint32_t count = rd8(param4, 5);
    for (uint32_t i = 0; i < count; ++i) {
        void* item = env->sub38090(param1, param1, esi);        // 0x38090 CreateItemUnit
        env->ordinal10977(item, 0x20000, 1);                    // IFLAG_STARTITEM
        env->ordinal10058(item, rd8(param4, 4));                // nBodyLoc
        const int32_t iface = env->ordinal10792(item);
        if (iface == 0) {
            uint32_t stat, value;
            if (rd8(param4, 4) == 0) {
                env->sub383F0();
                if (env->ordinal10744(item, 5) == 0) {
                    value = env->ordinal11116(item, 0);
                    stat = 0x48;
                } else {
                    value = 0xFA;
                    stat = 0x46;
                }
                env->ordinal10887(item, stat, value, 0);
                if (env->ordinal10744(item, 5) != 0)
                    env->ordinal10887(item, 0x46, 100, 0);
            } else {
                const uint8_t b = env->sub37F20();
                if (b == 0) env->sub383F0();
                if (env->ordinal10744(item, 5) == 0) {
                    value = env->ordinal11116(item, 0);
                    stat = 0x48;
                } else {
                    value = 0xFA;
                    stat = 0x46;
                }
                env->ordinal10887(item, stat, value, 0);
            }
        } else {
            const uint8_t b = env->sub37EC0(item, param1, 1);
            if (b == 0) env->sub383F0();
        }
    }
    (void)param2; (void)param3;
}

// 1.13c D2Game+0x6B200
// Unit flag fan-out: ESI = unit, ECX = self, param_1 = the int32 handed to 0x7ACB0 and 0x7ADF0.
// Bits 0, 10 and 8 of unit+0xC4 call 0x6A0D0, 0x7AC50 and 0x7ACB0; bit 0 of unit+0xC8 calls
// 0x9BB10(0, unit). 0x7ADF0(self, unit, param_1) runs unconditionally. Ghidra materialises `self`
// from extraout_ECX after each call; the register pass-through is transcribed as the parameter.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_6B200(uint32_t ecxSelf, uint32_t esiUnit, int32_t param1,
                                             const D2GameBatchD40F0Env* env)
{
    void* self = (void*)(uintptr_t)ecxSelf;
    if (esiUnit != 0) {
        const uint32_t f31 = rd32((void*)(uintptr_t)esiUnit, 0xC4);
        if ((f31 & 1u) != 0) env->sub6A0D0();
        if (((f31 >> 10) & 1u) != 0) env->sub7AC50();
        if (((f31 >> 8) & 1u) != 0) env->sub7ACB0(param1);
        if ((rd32((void*)(uintptr_t)esiUnit, 0xC8) & 1u) != 0)
            env->sub9BB10(0, esiUnit);
    }
    env->sub7ADF0(self, esiUnit, param1);
}

// 1.13c D2Game+0x6F620 and 0x6F660 moved to d2game_6f620.cpp / d2game_6f660.cpp

// 1.13c D2Game+0x6F8A0
// Missile unit initialisation. EAX = unit, EDI = game, and the return address is stored at unit+0xC
// (a bare retaddr in the decompile; it is a parameter here). Ordinal_10098 runs first, bit 1 and
// bit 3 of unit+0xC4 are cleared, and any old record at unit+0xDC is freed through 0x8E880. The
// record from 0x8E680 gets its flag byte +2 |= 4, fields +0, +4, +0x14, +0x18 and +0x2C reset, and
// is pushed onto the game's per-unit-type list at game+0xB8 -> +0xA04 -> petTypeOffsets[type]*4.
// 0x6F8A0 moved to d2game_6f8a0.cpp (pb9).

// 1.13c D2Game+0x79100
// Item-to-client skill pass. EAX = unit, param_1 = client/context. Only unit type 0 with a non-null
// handle at unit+0x60 participates: 0x778A0, then Ordinal_11139(handle, 4) and (handle, 5) each add
// the unit through 0x78DD0, then 0x77B20.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_79100_batch(uint32_t eaxUnit, uint32_t param1,
                                             const D2GameBatchD40F0Env* env)
{
    if (eaxUnit == 0 || rd32((void*)(uintptr_t)eaxUnit, 0) != 0) return;
    const uint32_t handle = rd32((void*)(uintptr_t)eaxUnit, 0x60);
    if (handle == 0) return;
    env->sub778A0();
    if (env->ordinal11139(handle, 4) != 0) env->sub78DD0(param1, eaxUnit);
    if (env->ordinal11139(handle, 5) != 0) env->sub78DD0(param1, eaxUnit);
    env->sub77B20();
}

// 0x8B010 (the 0x3E item stat packet) lives in D2GAME_PACKETS_SendPacket0x3E_6FC3EC20 (game_net_stubs.cpp).

// 1.13c D2Game+0x97120
// Pet-list allocator; the 1.10f form is PLAYERPETS_AllocPetList_6FC7CAF0. Ordinal_10920(player)
// returns the player data (fatal 0x9A when null). An existing pet block at +0x44 is freed through
// 0x97640. Two pool allocations follow (file PlayerPets.cpp, lines 0xA2 and 0xA6): the outer
// D2PlayerPetStrc and a zeroed D2PetListStrc array of nPetTypeTxtRecordCount * 3 dwords. The
// decompile also carries a zero-trip byte loop left from the typed initialiser; it is kept as is.
// EBX = game.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2game_port_97120(uint32_t param1, uint32_t ebxGame,
                                             const D2GameBatchD40F0Env* env)
{
    static const char kFile[] = "..\\Source\\D2Game\\PLAYER\\PlayerPets.cpp";
    const uint32_t playerData = env->ordinal10920(param1);
    if (playerData == 0) d2game_fatal(0x9A);
    if (rd32((void*)(uintptr_t)playerData, 0x44) != 0) env->sub97640(ebxGame, param1);

    uint32_t* pets = (uint32_t*)env->allocPool(kFile, 0xA2);    // D2PlayerPetStrc
    wr32(pets, 0, 0);
    wr32((void*)(uintptr_t)playerData, 0x44, (uint32_t)(uintptr_t)pets);
    uint32_t* lists = (uint32_t*)env->allocPool(kFile, 0xA6);   // D2PetListStrc array
    wr32(pets, 0, (uint32_t)(uintptr_t)lists);
    uint32_t n = (rd32(env->tables, 0xBF0) * 3) & 0x3FFFFFFFu;
    for (uint32_t i = 0; i < n; ++i) lists[i] = 0;
    for (int32_t i = 0; i != 0; --i) *((uint8_t*)lists) = 0;    // zero-trip residue
    env->sub96F40(ebxGame);
}

// 1.13c D2Game+0x97a20
// Add one pet to a player's pet list; the 1.10f form is sub_6FC7DA40. EBX = game, ECX = player,
// ESI = D2PetListStrc, EDX = pet unit or null, param_3 = D2PetInfoStrc (seed, name, unk06,
// hireling id). An empty list is rebuilt through 0x96FA0; if it stays empty the pet is dropped
// through 0x977E0. A full list removes its first pet through 0x97920 with the first node's GUID
// (fatals 0x312 when the list has no head, 0x316 when it is still full). The new D2PetDataStrc is
// six zeroed dwords: GUID +0x04, the first three pet-info dwords at +0x08..+0x10, next +0x14.
// Count +4 is incremented after the append; a count above max is fatal 0x330.
extern "C"
EMSCRIPTEN_KEEPALIVE uint32_t d2_port_d2game_97a20(uint32_t ebxGame, uint32_t ecxPlayer,
                                                 uint32_t esiPetList, uint32_t edxPet,
                                                 const uint32_t* param3,
                                                 const D2GameBatchD40F0Env* env)
{
    static const char kFile[] = "..\\Source\\D2Game\\PLAYER\\PlayerPets.cpp";
    uint32_t list = esiPetList;
    uint32_t nMax = rd32((void*)(uintptr_t)list, 8);
    if (nMax == 0) {
        env->sub96FA0(ebxGame, ecxPlayer);
        nMax = rd32((void*)(uintptr_t)list, 8);
        if (nMax == 0) {
            env->sub977E0(ebxGame, edxPet ? rd32((void*)(uintptr_t)edxPet, 0x0C) : 0xFFFFFFFFu);
            return 0;
        }
    }

    if (rd32((void*)(uintptr_t)list, 4) == nMax) {
        const uint32_t head = rd32((void*)(uintptr_t)list, 0);
        if (head == 0) d2game_fatal(0x312);
        env->sub97920(rd32((void*)(uintptr_t)head, 4), (void*)(uintptr_t)list, 1);
        if (rd32((void*)(uintptr_t)list, 4) == rd32((void*)(uintptr_t)list, 8))
            d2game_fatal(0x316);
    }

    uint32_t last = rd32((void*)(uintptr_t)list, 0);
    if (last != 0) {
        uint32_t next = rd32((void*)(uintptr_t)last, 0x14);
        while (next != 0) {
            last = rd32((void*)(uintptr_t)last, 0x14);
            next = rd32((void*)(uintptr_t)last, 0x14);
        }
    }

    uint32_t* node = (uint32_t*)env->allocPool(kFile, 0x322);
    for (uint32_t i = 0; i < 6; ++i) node[i] = 0;
    node[1] = edxPet ? rd32((void*)(uintptr_t)edxPet, 0x0C) : 0xFFFFFFFFu;
    node[2] = param3[0];
    node[3] = param3[1];
    node[4] = param3[2];
    if (last == 0) wr32((void*)(uintptr_t)list, 0, (uint32_t)(uintptr_t)node);
    else wr32((void*)(uintptr_t)last, 0x14, (uint32_t)(uintptr_t)node);

    const uint32_t current = rd32((void*)(uintptr_t)list, 4);
    wr32((void*)(uintptr_t)list, 4, current + 1);
    if (current + 1 <= rd32((void*)(uintptr_t)list, 8)) return 1;
    d2game_fatal(0x330);
}

// old probe-shaped D40F0 (superseded by d2game_d40f0.cpp g_d40f0)
// Delayed-unit update: EDI = unit, EBX = the argument forwarded to 0x8E030 / 0x8DFD0. Skipped when
// the unit is null, bit 4 of unit+0xC4 is set, or unit+0x10 is not 3. 0x4AD10(0) runs first; bit
// 0xC of unit+0xC4 then routes to 0x8E030, otherwise to 0x8DFD0.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_d40f0_old(uint32_t ediUnit, uint32_t ebxArg,
                                             const D2GameBatchD40F0Env* env)
{
    if (ediUnit == 0) return;
    const uint32_t flags = rd32((void*)(uintptr_t)ediUnit, 0xC4);
    if (((flags >> 4) & 1u) != 0) return;
    if (rd32((void*)(uintptr_t)ediUnit, 0x10) != 3) return;
    env->sub4AD10(0);
    if (((flags >> 0xC) & 1u) != 0) {
        env->sub8E030(ebxArg);
        return;
    }
    env->sub8DFD0(ebxArg);
}

// Probe entry for 0x38470 (src/core/fn/d2game_38470.cpp): the env carries only the item-code lookup;
// every other seam is null, so only the early-return paths (unknown code, nItemCount 0) are callable.
extern "C" void d2game_38470_probe(uint32_t player, uint32_t game, const uint32_t* item,
                                   int32_t (*lookup)(uint32_t, int32_t*))
{
    D2GameBatchD40F0Env env{};
    env.ordinal10450 = lookup;
    d2game_38470_impl(player, game, 0, item, 0, &env);
}
