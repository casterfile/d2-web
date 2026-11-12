// Batch b65: 12 D2Game functions, written from the cached 1.13c decompile.
// Offsets are the 32-bit retail layout (raw byte reads); callees are weak externs
// (ordinals as Ord<N>, internal functions as D2GAME_<rva>) so this file links standalone.
// Register inputs (EAX/ECX/EDX/EBX/EBP/ESI/EDI) are explicit leading params.
#include <cstdint>
#include <initializer_list>
#include "d2game_e5d20.h"

typedef intptr_t I;
#define O(n) extern "C" __attribute__((weak)) I Ord##n(...)
#define F(n) extern "C" __attribute__((weak)) I D2GAME_##n(...)
O(10507); O(10587); O(10887); O(10747); O(10787); O(10858); O(10708); O(10952); O(10494);
O(10002); O(10793); O(11138); O(10601); O(10543); O(11144); O(10744); O(10326); O(10364);
O(10086); O(10867); O(10750); O(10193); O(10455); O(10655); O(10977); O(10202); O(10201);
O(10050); O(10267); O(10045); O(10871); O(10265); O(10024); O(10237); O(10945); O(11013);
O(10785); O(10911); O(10807); O(10261); O(10819); O(10412); O(10161); O(10064); O(11104);
O(10920); O(10174);
F(CEBF50); F(CEDF30); F(CEB1F0); F(CE120); F(E0A20); F(78430); F(CED630);
F(CE620); F(FAE20); F(CAEA80); F(C213B0); F(C21220); F(CBF7A0); F(C42A80); F(CAE6F0);
F(C554F0);
extern "C" __attribute__((weak)) void* D2GAME_E03A0(void*, uint32_t);
extern "C" void _exit(int);
static const char kPlrMsg[] = "..\\Source\\D2Game\\PLAYER\\PlrMsg.cpp";
static const char kItemMode[] = "..\\Source\\D2Game\\ITEMS\\ItemMode.cpp";
static const char kSUnitDmg[] = "..\\Source\\D2Game\\UNIT\\SUnitDmg.cpp";
static inline uint32_t& U32(I base, int off) { return *(uint32_t*)(base + off); }

// 1.13c D2Game+0xCE520: moved to src/core/d2game/d2game_ce520.cpp (retail registers and stack).

// 1.13c D2Game+0xCE620  (hidden: EAX=unit, EBX, EBP, ESI)
I D2GAME_CE620(I eax, uint32_t ebx, I ebp, I esi, I p1)
{
    I r = Ord10747(p1);
    if (r != 0) eax = D2GAME_CEB1F0(eax);
    r = Ord10787(p1, eax);
    I g = Ord10858();
    if (g != 0) {
        if (0 < r && (r < 3 || r == 4) && ((1u << 0x10) & *(uint8_t*)(g + 7)) != 0) {
            // ponytail: gdwBitMasks_exref[n] taken as 1<<n
            eax = Ord10708(-1);
            r = Ord10787(p1, eax);
        }
        if (r == 0 && (r = Ord10952(p1, eax)) != 0 &&
            (((1u << 0xc) & *(uint8_t*)(g + 7)) != 0 || (-1 < ebp && (ebp < 2 || ebp == 3)))) {
            I* p3 = (I*)D2GAME_E03A0((void*)esi, (uint32_t)ebx);
            if (p3 && Ord10494(p3, 0x8f) != 0) {
                I* p4 = (I*)D2GAME_E0A20(esi);
                if (p4) { ebp = ((int32_t*)p4)[0]; ebx = ((uint32_t*)p4)[3]; p3 = p4; }
            }
            if (p1 != 0 && p3 != nullptr) {
                I k = Ord10002(p1, eax);
                if (k == 1) {
                    I v = Ord10793(p3);
                    if (Ord11138(p1, p3, v) == 0) goto fail;
                } else if (k == 4) {
                fail:
                    Ord10601(eax, kPlrMsg, 0x7a6);
                    D2GAME_CE120();
                    return 0;
                }
            }
            D2GAME_78430(eax, r, ebp, ebx, 0);
            return 0;
        }
    }
    return 2;
}

// 1.13c D2Game+0xCE870  (fastcall; stack params 3,4)
I D2GAME_CE870(I p1, I p2, I p3, void* p4)
{
    I r = D2GAME_CED630(p4, p1, p2, &p3);
    if (r == 0) {
        if (Ord10507(p2) == 0) return 3;
        r = Ord10587(p2, 0x148, 0, 0);
        Ord10887(p2, 0x148, r + 1);
        Ord10507(p2, 0x148, 0, 1);
        D2GAME_CE620(p2);
        r = 0;
    }
    return r;
}

// 1.13c D2Game+0xD1720  (hidden: EAX=in)
uint32_t D2GAME_D1720(uint32_t in)
{
    switch (in) {
    case 4: return 5;
    case 5: return 4;
    case 6: return 7;
    case 7: return 6;
    case 0xb: return 0xc;
    case 0xc: return 0xb;
    default: return 0;
    }
}

// 1.13c D2Game+0xD1C00 moved to src/core/d2game/d2game_d1c00.cpp

// 1.13c D2Game+0xD1E10  (hidden: EBX=src unit, ESI=out 0x84-byte struct)
void D2GAME_D1E10(I ebx, uint32_t* esi, I p1, I p2)
{
    for (int i = 0; i < 0x21; i++) esi[i] = 0;
    esi[2] = (uint32_t)p1;
    esi[3] = (uint32_t)Ord10086();
    *(uint16_t*)((uint8_t*)esi + 0x2a) = *(uint16_t*)(p1 + 0x78);
    esi[5] = (uint32_t)p2;
    esi[6] = 4;
    int t = (int)U32(ebx, 0);
    uint32_t v;
    I o = (I)U32(ebx, 0x2c);
    if (t == 2 || (3 < t && t < 6)) v = U32(o, 0xc);
    else v = o == 0 ? 0 : (uint32_t)Ord10867(o);
    esi[7] = v;
    if (t == 2 || (3 < t && t < 6)) v = U32(o, 0x10);
    else v = o == 0 ? 0 : (uint32_t)Ord10750(o);
    esi[8] = v;
    esi[1] = (uint32_t)(ebx + 0x20);
    *(uint16_t*)(esi + 10) = 1;
    esi[0xc] = 2;
    esi[0x12] = U32(ebx, 0x28);
    uint32_t w = (uint32_t)Ord10193();
    esi[0x10] = 2;
    esi[0x13] = w;
}

// 1.13c D2Game+0xD2540  (hidden: EAX=slot sel; stack game, ret 4). The callees ride in an env of seams so the replay records their calls.
struct D2GameD2540Env {
    I (*find)(I id, I slot);                      // #10455
    void (*place)(I id, I item, I slot);          // #10655
    void (*setFlag)(I item, I flag, I on);        // #10977
    I (*check)(I item, I a, I line, const char* file);   // #10202
    void (*notify)(I game, I on);                 // #10201 (retail passes the game pointer, the stack argument)
    void (*add)(I id, I item);                    // #10050
    void (*done)(I id, I slot);                   // #10267
};
extern "C" void d2_fn_d2game_D2540(uint32_t eax, I p1, const D2GameD2540Env* e)
{
    uint32_t u2 = eax & 3;
    I u1 = (I)U32(p1, 0x60);
    int n = 0, k = 0;
    if (u2 == 0) u2 = 0; else if (2 < u2) u2 = 3;
    uint32_t u5 = u2;
    do {
        I it = e->find(u1, u2);
        if (it != 0) {
            if (n != k) {
                e->place(u1, it, u5);
                e->setFlag(it, 0x400, 1);
                e->setFlag(it, 1, 1);
                if (e->check(it, 0x4000, 0x1314, kItemMode) != 0) e->setFlag(it, 0x4000, 0);
                e->notify(p1, 1);
                e->add(u1, it);
                e->done(u1, u2);
            }
            n++;
            u5 += 4;
        }
        k++;
        u2 += 4;
    } while (k < 4);
}
void D2GAME_D2540(uint32_t eax, I p1)
{
    D2GameD2540Env e{};
    e.find = [](I a, I b) { return Ord10455(a, b); };
    e.place = [](I a, I b, I c) { Ord10655(a, b, c); };
    e.setFlag = [](I a, I b, I c) { Ord10977(a, b, c); };
    e.check = [](I a, I b, I c, const char* d) { return Ord10202(a, b, c, d); };
    e.notify = [](I a, I b) { Ord10201(a, b); };
    e.add = [](I a, I b) { Ord10050(a, b); };
    e.done = [](I a, I b) { Ord10267(a, b); };
    d2_fn_d2game_D2540(eax, p1, &e);
}

// 1.13c D2Game+0xDC780  (hidden: EAX=unit-ish, ESI=attacker)
void D2GAME_DC780(uint32_t* eax, int* esi, int p1)
{
    if (0 < p1) {
        I i3 = Ord10871();
        if (!eax) { Ord10024("", Ord10265(0xa2a)); _exit(-1); }
        I u = (I)eax[0x20];
        I i5 = D2GAME_FAE20(u);
        if (i5 != 0) {
            int st = 0;
            if (esi && *esi == 1 && i5 < 0) {
                I d = Ord10237(*(uint8_t*)(u + 0x6d));
                if ((int)U32(d, 0x18) != 0) { p1 = p1 / (int)U32(d, 0x18); st = p1; }
            }
            if (p1 < 2) st = 1;
            uint32_t v = U32(u, 0xa8) + st;
            if (i3 == 0) {
                Ord10945();
                I h = Ord11013(U32(u, 0x1c), 2, v, eax[0], eax[3]);
                D2GAME_CAEA80(u, esi, 0xc, v, 0, 0, 0);
                Ord10785(h, 0xb);
                Ord10911(h, 0);          // callback LAB_6fcfb4c0
                Ord10807();
                Ord10261(h, 0x43, 0, 0);
                Ord10261(h, 0x44, 0, 0);
                Ord10261(h, 0x45, 0, 0);
                Ord10819();
            } else if (Ord10412(i3) < (int)v) {
                Ord10161(i3, v);
                D2GAME_CAEA80(u, esi, 0xc, v, 0, 0, 0);
            }
            uint64_t r = (uint64_t)(uint32_t)esi[8] * 0x6ac690c5ull + (uint32_t)esi[9];
            *(uint64_t*)(esi + 8) = r;
            if ((uint32_t)r % 100 < 0x14 && esi && *esi == 1 && D2GAME_C213B0(100, 0x13) == 0) {
                Ord10945();
                return;
            }
            Ord10945();
        }
    }
}

// 1.13c D2Game+0xDCC20  (hidden: EAX=unit-ish, EBX=amount, EDI=attacker)
void D2GAME_DCC20(uint32_t* eax, uint32_t ebx, int* edi, int p1)
{
    if (0 < p1) {
        if (edi && *edi == 1) {
            if (edi[5] != 0 && (*(uint8_t*)((I)(uint32_t)edi[5] + 0x16) & 8) != 0) {
                uint64_t r = (uint64_t)eax[8] * 0x6ac690c5ull + eax[9];
                *(uint64_t*)(eax + 8) = r;
                if ((uint32_t)r % 100 < 0x5a) return;
            }
            if (Ord10064(0) != 0) return;
            I d = D2GAME_C21220();
            if (*(int16_t*)(d + 0x32) == 0) return;
            Ord11104();
        }
        I i3 = Ord10871();
        if (!eax) { Ord10024("", Ord10265(0xa2a)); _exit(-1); }
        I u = (I)eax[0x20];
        uint32_t v = U32(u, 0xa8) + ebx;
        if (i3 == 0) {
            I h = Ord11013(U32(u, 0x1c), 2, v, eax[0], eax[3]);
            D2GAME_CAEA80(u, edi, 0xc, v, 0, 0, 0);
            Ord10785(h, 0x15);
            Ord10911(h, D2GAME_CBF7A0);
            Ord10807();
            Ord10945();
            return;
        }
        Ord10161(i3, v);
        D2GAME_CAEA80(u, edi, 0xc, v, 0, 0, 0);
    }
}

// 1.13c D2Game+0xDDAC0  (thiscall this=target; hidden: EAX=attacker)
void D2GAME_DDAC0(void* self, int* eax, int p1)
{
    if (p1 <= 0) return;
    int t = eax ? *eax : 6;
    if (t == 0) { D2GAME_DC780(nullptr, eax, p1); return; }
    if (t == 1) {
        if (Ord10494() != 0) return;
        I d = D2GAME_C21220();
        if (Ord10064(d) != 0 || D2GAME_C42A80(8) != 0 || Ord11104() != 0) {
            D2GAME_DC780((uint32_t*)self, eax, p1);
            return;
        }
    }
    I i5 = Ord10871();
    if (!self) { Ord10024("", Ord10265(0xa2a)); _exit(-1); }
    I u = (I)U32((I)self, 0x80);
    if (D2GAME_FAE20(u) >= 0) return;
    int q = 0;
    if (eax && *eax == 1) {
        I d = Ord10237(*(uint8_t*)(u + 0x6d));
        if ((int)U32(d, 0x14) == 0) { Ord10024("", Ord10265(0x250)); _exit(-1); }
        q = p1 / (int)U32(d, 0x14);
    }
    uint32_t v9 = U32(u, 0xa8) + q, v8;
    if (i5 == 0) {
        Ord10945();
        I h = Ord11013(U32(u, 0x1c), 2, v9, U32((I)self, 0), U32((I)self, 0xc));
        Ord10785(h, 1);
        Ord10911(h, 0);                  // callback LAB_6fcfbd90
        Ord10807();
        v8 = v9;
    } else {
        v8 = (uint32_t)Ord10412(i5);
        if ((int)v8 < (int)v9) { Ord10161(i5, v9); v8 = v9; }
    }
    D2GAME_CAEA80(u, eax, 0xc, v8, 0, 0, 0);
    if (!eax || *eax != 1) { Ord10024("", Ord10265(0x272)); _exit(-1); }
    char* c = (char*)(I)eax[0x37];
    while (c) {
        char k = *c;
        c = *(char**)(c + 0x24);
        if (k == 2) D2GAME_CAE6F0();
    }
    D2GAME_CAEA80(u, eax, 2, U32(u, 0xa8) + 1 + q, 0, 0, 0);
}

// 1.13c D2Game+0xE5D20  (fastcall; body shared with the FN probe in d2game_e5d20.h)
I D2GAME_E5D20(I p1, void* p2)
{
    return d2game_e5d20_body(p1, p2, [](void* u) { return Ord10920(u); },
                             [](I a, I b, I c) { return Ord10174(a, b, c); },
                             [](void* u, I g) { D2GAME_C554F0(u, g); });
}
