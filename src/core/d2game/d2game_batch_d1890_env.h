// Seam table of d2game_batch_d1890.cpp (cut out of the port so the FN probes build the same struct).
#pragma once
#include <cstdint>

// Host seams for the callees the cached decompiles name but src/ does not implement.
struct D2GameBatchBd18Env {
    // D2Common ordinal 10973 (1.13c): three independent reads fill a timer/mode context.
    uint32_t (*ordinal10973)();
    // D2Common ordinal 10973(unit, stat, 0) as 0xD1890 calls it: the unit stat read (stats 6, 8, 0xA).
    uint32_t (*unitStat10973)(const void* unit, uint32_t stat, uint32_t layer);
    // D2Common ordinal 10828: the current item/cursor unit handle.
    void*    (*ordinal10828)(const void* unit);
    // D2Common ordinal 10507: a second item/owner handle.
    void*    (*ordinal10507)(const void* unit);
    // D2Common ordinal 10304(unit): the handle stored beside the string result.
    uint32_t (*ordinal10304)(const void* unit);
    // D2Common ordinal 10601(unit, file, line): the named/file-tagged result of the unit.
    uint32_t (*ordinal10601)(const void* unit, const char* file, uint32_t line);
    // D2Common ordinal 10417: the client's mode/state byte.
    uint8_t  (*ordinal10417)();
    // D2Common ordinal 10138(unit): consumes the unit's +0x60 block.
    void     (*ordinal10138)(void* unit);
    // D2Common ordinal 10175: two probes, compared with 1 and then 0.
    int32_t  (*ordinal10175)();
    // D2Common ordinal 11057: a third probe, compared with 0.
    int32_t  (*ordinal11057)();
    // 0xD47C0: the item-mode update tail reached once those probes pass.
    void     (*subD47C0)(void* client);
    // 0x211D0(context+0x6C): the player/difficulty index added to the list scan.
    uint32_t (*sub211D0)(void* ctx);
    // D2Common ordinal 10014(unitId): resolves a unit from its id.
    void*    (*ordinal10014)(uint32_t unitId);
    // sgptDataTables: base of the item/treasure data table block; +0xA78 is the entry array and
    // +0xA80 its length (0x1A8 bytes per entry).
    const uint8_t* dataTables;
    // gdwBitMasks: the byte bit-mask table; byte 0x1C is tested against the entry's +0x0E flags.
    const uint8_t* bitMasks;
    // D2Common ordinal 10920: the state read at the top of the 0x8E170 packet builder and the
    // pointer whose +0x34 block 0xB27D0 clears.
    void*    (*ordinal10920)();
    // 0x9BEB0: the 16-bit value placed at packet+0x26 when ESI has +0x80 set and +0x00 clear.
    uint16_t (*sub9BEB0)();
    // D2Common ordinal 501(block, 0xC): initialises the 12-byte block at packet+0x14.
    void     (*ordinal501)(void* block, uint32_t size);
    // 0x8A3E0: the packet sink (pointer, byte length) shared by 0x8E170 and 0x9D780.
    void     (*sub8A3E0)(const void* packet, uint32_t length);
    // 0x96920(param_1): the trailing action of 0x8E170.
    void     (*sub96920)(uint32_t param1);
    // 0xB2C20(in_EAX, ESI, 1): the 0xB2D40 prologue.
    void     (*subB2C20)(uint32_t eaxVal, void* esi, uint32_t one);
    // 0xB2760: the follow-up when ESI+4 != 4 and ESI+0x174 != 0.
    void     (*subB2760)();
    // D2Common ordinal 10383(handle, unitArray, count): fills the room's unit-id array and its count.
    void     (*ordinal10383)(uint32_t handle, void** array, uint32_t* count);
    // 0x4AD10 CLIENTS_GetPlayerFromClient(0).
    void*    (*sub6AD10)(uint32_t arg);
    // D2Common ordinal 10733(unitId): the unit-id hash chain (next at +8).
    void*    (*ordinal10733)(uint32_t unitId);
    // 0x9DE10(EDI): the per-unit action repeated by 0x9DEC0.
    void     (*sub9DE10)(void* edi);
    // 0xB23D0: the 0xB2D90 probe between the player check and 0xB2760.
    int32_t  (*subB23D0)();
    // D2Common ordinal 10201(item, page): the item-page clear.
    void     (*ordinal10201)(const void* item, uint32_t page);
    // D2Common ordinal 10202(item, 0x80, 0x180, file): the item flag probe.
    int32_t  (*ordinal10202)(const void* item, uint32_t a, uint32_t b, const char* file);
    // D2Common ordinal 11051(item, flag): the item flag test.
    int32_t  (*ordinal11051)(const void* item, uint32_t flag);
    // D2Common ordinal 10058(item, 0): the item flag clear.
    void     (*ordinal10058)(const void* item, uint32_t value);
    // D2Common ordinal 10170(block): the first unit-id list node of the +0x60 block.
    uint32_t (*ordinal10170)(const void* block);
    // D2Common ordinal 10691(node): the next unit-id list node.
    uint32_t (*ordinal10691)(uint32_t node);
    // D2Common ordinal 10727(node): the unit id a list node names.
    uint32_t (*ordinal10727)(uint32_t node);
    // D2Common ordinal 10545(node): releases a unit-id list node.
    void     (*ordinal10545)(uint32_t node);
    // 0x9B340: the per-item action in the inventory-mode walk.
    void     (*sub9B340)();
    // 0xDFC10(unit): the caller's per-unit callback.
    void     (*subDFC10)(void* unit);
    // 0xE20A0(game, unit): the item dispatch after 0xDFC10.
    void     (*subE20A0)(void* game, void* unit);
    // D2Common ordinal 10613(handle): the 16-bit value in the first 0x9D610 packet.
    uint16_t (*ordinal10613)(uint32_t handle);
    // D2Common ordinal 11062(handle, 0, block, out): fills the 0x9D610 second packet.
    void     (*ordinal11062)(uint32_t handle, void* zero, void* block, void* out);
    // 0xB2760 with its register and stack inputs (EAX = a, stack = b).
    void     (*subB2760x)(uint32_t a, uint32_t b);
    // 0x6AD10 with ESI (the unit the caller holds) and the stack zero.
    void*    (*sub6AD10x)(uint32_t esi, uint32_t arg);
    // 0xB23D0 with EAX = the player unit.
    int32_t  (*subB23D0x)(uint32_t eax);
};
