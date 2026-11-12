// src/core/d2client/d2c88_seam.h -- the D2Client 0x88930..0x8A540 sound/RNG batch state and callee seam (shared with its probes).
#pragma once
#include <cstdint>
#include <cstddef>

struct D2c88State {
    void**   list;      // +0x10BF30 DAT_6fbcbf30 node: [0]=object*, [1]=next node
    uint8_t* recs;      // +0x10C24C record array, stride 0x8E
    int32_t  count;     // +0x10C250
    uint32_t now;       // +0x10C268
    uint8_t* rngBase;   // *(+0x10BBFC): seed struct, u64 at +0x20
    float    biasZ;     // _DAT_6fb87584
    uint32_t tick, a, b, c; // +0x10B9CC/D0/D4/D8 (89F00 stores)
    uint32_t hook;      // +0x10BEFC
    uint32_t h1c, h20;  // +0x10BF1C / 0x10BF20
    uint32_t f04, f08, f0c, f14, f18; // +0x10BF04/08/0C/14/18
    uint32_t eae8;      // +0x10BAE8
    void*    table;     // +0x10B9E0 (256 x 0x100 bytes)
};

struct D2c88Env {
    int  (*fn87e90)(uint8_t* obj, int v);
    void (*fn87ed0)(int key, int value);   // 0x87ED0: ECX = 1, EDX = key, stack word = value
    void (*fn88350)(const float* v, void* obj);
    void (*fn88550)(void* obj);
    uint32_t (*fn88a70)(int, int, int, int);
    void (*ord10062)(uint32_t ecx, uint32_t edx);   // D2sound#10062 via thunk 0xD258 (fastcall: list head, key)
    uint32_t (*ord10065)(uint32_t);
    void (*ord10043)(uint32_t, uint32_t);
    void (*fn8a1a0)(void);
    void (*fn8a2b0)(void);
    void (*fn8a3d0)(void);
    void (*fatal)(uint32_t code);          // Ordinal 10265+10024+_exit(-1)
    uint32_t (*tickCount)(void);           // GetTickCount
    void* (*valloc)(size_t bytes);         // VirtualAlloc(0,n,0x3000,0x40)
};
