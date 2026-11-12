// Env seams of src/core/d2game/d2game_batch_c7130.cpp (shared with the FN probes in src/core/fn).
#pragma once
#include <cstdint>

struct D2GameBatchC7130Env {
    void*    (*ptr)(uint32_t v);                       // retail 32-bit pointer -> host record
    uint32_t (*ord10265)(uint32_t code);
    void     (*ord10024)(uint32_t text, uint32_t code);
    uint32_t (*ord10920)(const void* unit);            // Ordinal_10920 (thunk 0xA4C0, 1 stack word = unit)
    void     (*sendPacket8A3E0)(const void* buf, uint32_t len);
    uint8_t  (*subC6A30)(void* ecx);
    void     (*ord11090)(void* unit, uint32_t v);
    void     (*regCb)(uint32_t fn, uint32_t arg);      // 0xDFCC0
    void     (*sub1780)(void* p);
    void     (*sub33AF0)(void* quest);
    uint32_t (*ord10045)(const char* file, uint32_t line, uint32_t z);   // alloc, returns retail ptr
    void*    (*ord10688)(uint32_t classId);
    void*    (*ord11153)(uint32_t arg, uint32_t kind);
    uint32_t (*ord10331)(void* u);
    uint32_t (*ord10826)(uint32_t v);
    uint8_t  (*ord10459)(uint32_t v);
    void     (*sub0E0110)();
    void     (*ord10445)(void* u);
    void     (*sub2F300)();
    void     (*sub0E0F40)(void* u, void* a, void* b);
    void     (*sub8EA80)(void* game, void* unit, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e);
    void     (*ord10141)();
    void     (*sub54B80)(void* unit, uint32_t v);
    void     (*sub55EB0)();
    uint32_t (*cbArg)(uint32_t i);                     // register arguments of the three 0xDFCC0 calls
    uint32_t (*ord10708)(uint32_t ecx, uint32_t edx, uint32_t s0);          // thunk 0xA3D0: ECX = game word, EDX = skill id, 1 stack word
    uint32_t (*sub57890)(uint32_t eax, uint32_t s0, uint32_t s1, uint32_t s2);   // 0x37890: EAX = game word, 3 stack words (skill, right-hand flag, packet word)
    uint32_t (*ord10830)();
    uint8_t  (*ord10459u)(uint32_t v);
    const uint8_t* dataTables;                         // sgptDataTables block
    uint32_t (*sub21100)(void* u); uint32_t (*sub21130)(void* u); uint32_t (*sub21280)();
    void     (*ord10346)(uint32_t v, uint32_t w);
    uint32_t modBase;                                  // retail module base the stored callback addresses are relative to (probes: 0xB2000000)
};
