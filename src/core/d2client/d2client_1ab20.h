// 1.13c D2Client+0x1AB20 -- the click on a unit (d2client_1ab20.cpp). The seams carry the registers and stack words retail passes.
#pragma once
#include <cstdint>
namespace d2client_1ab20 {
using U = uint32_t; using I = int32_t; using P = void*;
struct Fn {
    void (*c226)(I id);                       // Ordinal 10688(id)
    I (*c1fc)(U pl, I n);                     // Ordinal 10494(pPlayer global, 0x66)
    I (*cefe)(P pl, P tgt);                   // Ordinal 10594
    I (*cf0a)(P pl, P tgt, I m);              // Ordinal 10324
    I (*cc70)(P tgt, P pl);                   // Ordinal 10407
    void (*cdd8)(U p);                        // Ordinal 10159
    I (*s8ac0)(I a, I b);                     // 0x18AC0
    P (*s1230)(I id);                         // 0x1230, EAX = id, returns a data-table row
    U (*s1660)(P tgt);                        // 0x1660, ECX = unit
    U (*s1630)(P tgt);                        // 0x1630, ECX = unit
    void (*s010)(U eax, U ecx, U w0, P w1);   // 0x1A010, EAX/ECX + 2 stack words
    void (*s48a0)(U eax, I ecx, I edx, U w0, U w1);   // 0x148A0
    void (*s6c0)(P esi, U edi);               // 0x2A6C0
    I (*s9f30)();                             // 0x29F30
    P (*s5b40)(I ecx, I edx);                 // 0xA5B40, ECX = b, EDX = a
    void (*s750)(P edx, const U* blk, U w1);  // 0x2A750, ECX = 2, EDX = player, stack (block of 7 dwords, 0)
    void (*s7c0)(P eax, U w0, U w1);          // 0x2A7C0
    void (*s2d50)(P eax, U w0, U w1, U w2);   // 0x2D50
};
struct Gl { U plG; U *g97ec, *g9828, *g3e4; const uint8_t* tab; const U* masks; };   // tab = *sgptDataTables, masks = gdwBitMasks
void sub_1AB20(I a, const uint8_t* fr, I b, const Gl& g, const Fn& f);
}  // namespace d2client_1ab20
