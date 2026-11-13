// Batch aea40: D2Game functions in RVA 0xAEA40-0xBBC00, transcribed from the cached 1.13c decompiles
// (build-wasm/ghidra-d2game/decomp/0x<RVA>.c). Same shape as d2game_batch_b65.cpp: retail 32-bit struct
// layout read as raw bytes off a base address, register inputs (ECX/EDX/EAX/EBX/EBP/ESI/EDI, the stack
// slot the decompile names unaff_retaddr) as explicit parameters, callees as weak externs (Ord<N> for
// D2Common ordinals, D2GAME_<RVA> for D2Game internals) so the file links standalone.
// Our implementations are d2_fn_d2game_<RVA> so they never collide with the weak callee names.
#include <cstdint>
#include <cstdlib>

typedef intptr_t I;
typedef uint32_t U;
#define O(n) extern "C" __attribute__((weak)) I Ord##n(...)
#define F(n) extern "C" __attribute__((weak)) I D2GAME_##n(...)
#define R32(b, o) (*(uint32_t*)((uintptr_t)(b) + (o)))
#define S32(b, o) (*(int32_t*)((uintptr_t)(b) + (o)))
#define R8(b, o)  (*(uint8_t*)((uintptr_t)(b) + (o)))
#define S16(b, o) (*(int16_t*)((uintptr_t)(b) + (o)))
#define U16(b, o) (*(uint16_t*)((uintptr_t)(b) + (o)))

// *sgptDataTables_exref is the tables block: +0xb64/+0xb6c/+0xb98/+0xba0/+0xbc0 are the record tables and counts.
extern "C" __attribute__((weak)) char sgptDataTables_exref[];
extern "C" __attribute__((weak)) int32_t D2GAME_DAT_F1B454[5];   // DAT_6fd1b454 (retail data, not dumped here)
static inline uintptr_t DT() { return (uintptr_t)R32(sgptDataTables_exref, 0); }
static const char kDataTableOwner[] = "";

O(10141); O(10174); O(10383); O(10392); O(10397); O(10436); O(10421); O(10034); O(10137); O(10153);
O(10168); O(10243); O(10265); O(10024); O(10286); O(10331); O(10336); O(10441); O(10459); O(10494);
O(10511); O(10554); O(10558); O(10565); O(10542); O(10585); O(10689); O(10717); O(10750); O(10764);
O(10786); O(10826); O(10830); O(10832); O(10867); O(10920); O(10973); O(10983); O(11085); O(11103);
O(11129); O(11154); O(10057); O(10970); O(10653);

__attribute__((weak, import_module("env"), import_name("D2GAME_E03A0"))) I aea40_E03A0(I, uint32_t);
F(E07A0); F(9C9A0); F(22A80); F(21220); F(DFA90); F(AD5A); F(AE0B0); F(AEA80); F(AE6F0);
F(9D230); F(8EA80); F(8E6F0); F(8E6F0); F(95E10); F(9EE50); F(EFB80); F(E19C0); F(E1570); F(DDDE0);
F(DE5A0); F(DEEE0); F(EDDB0); F(DEE50); F(21490); F(77240); F(36820); F(273C0); F(9F210); F(DE0C0);
F(DCF20); F(A0AD0); F(50250); F(A24C0); F(DFFD0); F(8C3B0); F(B6D30); F(B7090); F(B79B0); F(B8070);
F(B6A60); F(B6840); F(B1820); F(B7300);

// 1.13c D2Game+0xB9030 -- stack return address and one byte from the caller's frame.
extern "C" void d2_fn_d2game_B9030(I retaddr, uint8_t idx)
{
    if (Ord10436(0x6d)) {
        I v = Ord10920(retaddr);
        Ord11154(R32(v, 0x1c + (U)idx * 4), retaddr);
    }
}

// 1.13c D2Game+0xB1320 -- ESI unit, stack args (param_1, param_2, param_3).
extern "C" U d2_fn_d2game_B1320(U p1, U p2, U p3, I esi)
{
    (void)p2;
    if (esi && S32(esi, 0) == 1 && R32(esi, 0x14)) {
        U iVar1 = R32(R32(esi, 0x14), 0x28);
        if (iVar1 && R32(iVar1, 0x28)) {
            I piVar2 = aea40_E03A0(R32(iVar1, 0x28), R32(iVar1, 0x2c));
            if (piVar2 && S32(piVar2, 0) == 1) {
                U a[3] = {0, p3, p1};   // local_14 (out), local_10, local_c
                D2GAME_21490(&a[1], &a[0]);
                return a[0];
            }
        }
    }
    return 0;
}

// 1.13c D2Game+0xB0B20 -- ESI unit, EDI level id (below 0x14).
extern "C" U d2_fn_d2game_B0B20(I esi, I edi)
{
    if (esi && S32(esi, 0) == 1 && S32(&edi, 0) != 0 && edi < 0x14 && Ord10494() == 0 && Ord11085() != 0) {
        if (((R32(esi, 0xC4) >> 2) & 1) == 0 && D2GAME_DFA90() == 0) return 0;
        bool b = D2GAME_22A80(10);
        if (!b) {
            if (edi != 0x13) {
                D2GAME_21220();
                if (D2GAME_9C9A0(edi, 1) == 0) return 0;
            }
            return 1;
        }
    }
    return 0;
}

// 1.13c D2Game+0xB32E0 -- EAX record id, EBX unit, stack (param_1, param_2).
extern "C" U d2_fn_d2game_B32E0(U p1, I p2, I eax, I ebx)
{
    uintptr_t t = DT();
    if (eax >= 0 && eax < (I)R32(t, 0xba0) && (uintptr_t)R32(t, 0xb98) + (uintptr_t)eax * 0x23c != 0) {
        uintptr_t rec3 = (uintptr_t)R32(t, 0xb98) + (uintptr_t)eax * 0x23c;
        if (ebx != 0) {
            I iVar1 = S32(ebx, 4);
            if (iVar1 >= 0 && iVar1 < (I)R32(t, 0xb6c) && (uintptr_t)R32(t, 0xb64) + (uintptr_t)iVar1 * 0x1a4 != 0) {
                uintptr_t rec = (uintptr_t)R32(t, 0xb64) + (uintptr_t)iVar1 * 0x1a4;
                I v = Ord10786(p1, R32(rec3, 0x138));
                if (v < 1) v = S32(rec, 0x3c) - 1 + p2;
                if (v < 2) v = 1;
                else if (v > 0xfe) v = 0xff;
                Ord10168(R32(ebx, 0x2c), v);
                Ord10441();
                Ord10558();
                Ord10970();
                return 1;
            }
        }
    }
    return 0;
}

// 1.13c D2Game+0xB2FC0 -- ECX unit, EDX unit, stack record id, return address (the 4th Ord10786 arg).
extern "C" U d2_fn_d2game_B2FC0(I ecx, I edx, I rec, I retaddr)
{
    (void)ecx;
    uintptr_t t = DT();
    if (rec >= 0 && rec < (I)R32(t, 0xba0) && (uintptr_t)R32(t, 0xb98) + (uintptr_t)rec * 0x23c != 0) {
        uintptr_t r = (uintptr_t)R32(t, 0xb98) + (uintptr_t)rec * 0x23c;
        I iVar1 = Ord10511(edx);
        if (iVar1 == 0) return 0;
        U uVar2 = (U)Ord10397(iVar1);
        U uVar3;
        if ((uVar2 & 2) == 0) {
            uVar3 = R32(r, 0x138);
        } else {
            Ord10137(iVar1, 0);
            uVar3 = R32(r, 0x13c);
        }
        I v = Ord10786(edx, uVar3, rec, retaddr);
        Ord10542(edx, v);
        return 1;
    }
    return 0;
}

// 1.13c D2Game+0xB2E00 -- EAX unit, EBP is the slot the Ordinal_10153 call writes (Ghidra shows it as unaff_EBP).
extern "C" U d2_fn_d2game_B2E00(I eax, U ebp)
{
    if (Ord10421() == 0) {
        U uVar5 = R32(eax, 0x48) >> 8;
        I iVar2 = (I)(S32(eax, 0x44) >> 8);
        if (uVar5 <= R32(R32(eax, 0x50), 8) && (iVar2 + 1) < (I)uVar5) {
            uint8_t bVar1 = R8(R32(eax, 0x50), 0x10 + iVar2);
            I iVar3 = iVar2 + 1;
            do {
                if (bVar1 != 0) return bVar1;
                iVar3++;
            } while (iVar3 < (I)uVar5);
            return 0;
        }
    } else {
        I iVar3 = Ord10034();
        I iVar4 = S32(eax, 0x38) >> 8;
        I iVar2 = iVar4 + 1;
        if (iVar2 < (I)(iVar3 >> 8)) {
            do {
                Ord10153(R32(eax, 0x30), iVar4 << 8, &ebp);
                if (ebp != 0) return ebp;
                iVar2++;
            } while (iVar2 < (I)(iVar3 >> 8));
        }
    }
    return 0;
}

// 1.13c D2Game+0xB0510 -- stack param_1, param_3, EBX unit, return address, and one more stack slot.
extern "C" void d2_fn_d2game_B0510(I p1, I p3, I ebx, I retaddr, I stk4)
{
    U uVar3 = (U)Ord10459(p3);
    I iVar4 = R32(ebx, 0xbc + (uVar3 & 0xff) * 4);
    if (iVar4 == 0) return;
    for (iVar4 = Ord10717(iVar4); iVar4 != 0; iVar4 = R32(iVar4, 0x7c)) {
        I iVar5;
        if (p3 == 0 || (iVar5 = Ord10826(iVar4)) == p3) {
            I piVar1 = R32(iVar4, 0x74);
            I piVar2;
            while (piVar2 = piVar1, piVar2 != 0) {
                piVar1 = R32(piVar2, 0xe8);
                if (S32(piVar2, 0) == 1 && (S32(piVar2, 4) == stk4 || S32(piVar2, 4) == retaddr) &&
                    (R8(piVar2, 0xc6) & 1) == 0 && S32(piVar2, 0x10) != 0 && S32(piVar2, 0x10) != 0xc &&
                    (iVar5 = Ord10830(piVar2)) == 0) {
                    Ord11103(piVar2, 0);
                    U st[8] = {0, (U)piVar2, 0, 0, 0, 0x6400, 0, 0};   // iStack_28, piStack_24, uStack_20.., uStack_14 = 0x6400
                    D2GAME_95E10(ebx, (I)st);
                }
            }
        }
        p3 = p1;
    }
}

// 1.13c D2Game+0xB11BD -- loop over a list of unit records (retaddr is the list), EBX count, EBP zero.
extern "C" U d2_fn_d2game_B11BD(U ebx, I arr, U ebp, U arg14, U (*cb)(U, U))
{
    Ord10383();
    U idx = 0;
    for (;;) {
        if (ebx <= idx) return 0;
        I rec = (I)*(I*)(arr + (uintptr_t)idx * 4);   // list of records; 32-bit pointers in retail layout
        U cnt = R32(rec, 0x78);
        U list = R32(rec, 0x48);
        for (U u6 = 0; u6 < cnt; u6++) {
            U u4 = R32(list, u6 * 4);
            if (u4 == ebp) {
                U u5 = (U)Ord10265(0xfb);
                Ord10024((void*)0, u5);   // DAT_6fd19ff9 message
                std::_Exit(-1);
            }
            bool doTail = false;
            U u3 = 0;
            if ((R8(u4, 0x3d4) & 1) == 0) {
                if (R32(u4, 0x174) != ebp) {
                    u3 = (U)aea40_E03A0(R32(u4, 0x1a8), R32(u4, 0x170));
                    if (u3 == ebp) R32(u4, 0x174) = ebp;
                    doTail = true;
                }
            } else {
                u3 = R32(u4, 0x174);
                doTail = true;
            }
            if (doTail && u3 != ebp) {
                U r = cb(u3, arg14);
                if (r != ebp) return r;
            }
        }
        idx++;
    }
}

// 1.13c D2Game+0xB2120 -- ESI unit, stack param_1 (the game).
extern "C" void d2_fn_d2game_B2120(I p1, I esi)
{
    if (esi == 0 || (R8(esi, 0xc6) & 1)) return;
    if (S32(esi, 0) != 1) return;
    if (S32(esi, 0x10) == 0 || S32(esi, 0x10) == 0xc) return;
    if (Ord10494() != 0 || Ord11085() == 0) return;
    if (((R32(esi, 0xC4) >> 2) & 1) == 0 && D2GAME_DFA90() == 0) return;
    if (D2GAME_22A80(10)) return;
    if (S32(esi, 0) == 1 && R32(esi, 0x14) != 0 && (R8(R32(esi, 0x14), 0x16) & 2) != 0) return;
    if (Ord11085() == 0) return;
    D2GAME_AE0B0();
    Ord11129();
    D2GAME_9D230(p1, esi);
    U p = R32(esi, 0xDC);   // unaff_ESI[0x37]
    while (p != 0) {
        uint8_t c = R8(p, 0);
        p = R32(p, 0x24);
        if (c == 2) D2GAME_8E6F0();
    }
    D2GAME_8EA80(p1, esi, 2, S32(p1, 0xa8) + 1, 0, 0, 0);
}

// 1.13c D2Game+0xB36A0 -- ECX game, EDX unit.
extern "C" U d2_fn_d2game_B36A0(I p1, I p2)
{
    I iVar1 = Ord10511(p2);
    if (iVar1 == 0) return 0;
    D2GAME_E07A0(p1);
    I piVar2 = Ord10392(R32(p2, 0x2c));
    if (piVar2 == p2 || piVar2 == 0) return 0;
    Ord10137(iVar1, 0);
    Ord10243(iVar1, R32(piVar2, 0xc));
    Ord10554(iVar1, R32(piVar2, 0));
    Ord10336(R32(p2, 0x2c), 0);
    I t = S32(piVar2, 0);
    U uVar3, uVar4;
    if (t == 2 || (t > 3 && t < 6)) uVar3 = R32(R32(piVar2, 0x2c), 0x10);
    else if (R32(piVar2, 0x2c) == 0) uVar3 = 0;
    else uVar3 = (U)Ord10750(R32(piVar2, 0x2c));
    t = S32(piVar2, 0);
    if (t == 2 || (t > 3 && t < 6)) uVar4 = R32(R32(piVar2, 0x2c), 0xc);
    else if (R32(piVar2, 0x2c) == 0) uVar4 = 0;
    else uVar4 = (U)Ord10867(R32(piVar2, 0x2c));
    Ord10286(R32(p2, 0x2c), uVar4, uVar3);
    return 1;
}

// 1.13c D2Game+0xB41B0 -- ECX game, EDX unit, stack record id; EBX, EBP and the return address are live inputs.
extern "C" U d2_fn_d2game_B41B0(I p1, I p2, I p3, I ebx, I ebp, I retaddr)
{
    if (p3 < 0 || p3 >= (I)R32(DT(), 0xba0)) return 0;
    uintptr_t rec = (uintptr_t)p3 * 0x23c + R32(DT(), 0xb98);
    if (rec == 0) return 0;
    I L[28] = {0};   // local_74[21], iStack_20 is L[21], the rest of the frame
    L[0] = p1;
    D2GAME_E07A0(p1);
    I piVar2 = Ord10392(R32(p2, 0x2c));
    if (piVar2 == p2 || piVar2 == 0) return 0;
    if (S32(p2, 0) == 0) return D2GAME_273C0(ebx, p2, p3);
    for (int i = 0; i < 28; i++) L[i] = 0;
    U uVar3 = (U)Ord10653(p2, retaddr, p3, 0);
    uint16_t uVar1 = (uint16_t)D2GAME_DE5A0(ebp, uVar3, p2);
    if (uVar1 & 1) {
        I iVar5 = (I)R32(rec, 0x134);
        if (iVar5 != 0) L[21] = iVar5;
        D2GAME_77240(p2);
    }
    D2GAME_DDDE0(0x80, ebp, piVar2);
    return 1;
}

// 1.13c D2Game+0xB4410 -- ECX game, EDX unit, stack record id and a 4th argument.
extern "C" U d2_fn_d2game_B4410(I p1, I p2, I p3, U p4)
{
    if (p3 < 0 || p3 >= (I)R32(DT(), 0xba0) || (uintptr_t)R32(DT(), 0xb98) + (uintptr_t)p3 * 0x23c == 0) return 0;
    I thisv = D2GAME_9EE50(0, &p3, p2, 0, p3, p4);
    if (thisv < 0) return 0;
    D2GAME_E07A0(p1);
    I piVar1 = Ord10392(R32(p2, 0x2c));
    bool found = false;
    if (piVar1 == p2 || piVar1 == 0) {
        I iVar5 = R32(p2, 0x2c);
        if (iVar5 != 0 && Ord10764(iVar5) == 2) {
            U uVar3 = (U)Ord10983(iVar5);
            if (p1 == 0) {
                U uVar4 = (U)Ord10265(0x2d3);
                Ord10024((void*)0, uVar4);
                std::_Exit(-1);
            }
            for (I q = R32(p1, (uVar3 & 0x7f) * 4 + 0x1520); q != 0; q = R32(q, 0xe4)) {
                if (R32(q, 0xc) == uVar3) { piVar1 = q; found = true; break; }
            }
        }
    } else {
        found = true;
    }
    if (found) {
        I iVar5 = Ord10331(piVar1);
        if (iVar5 != 0 && (iVar5 = Ord10057(iVar5)) == 0) {
            U uVar4 = (U)Ord10331();
            D2GAME_EFB80(thisv, p1, uVar4, piVar1, iVar5, (uint16_t)iVar5);
            return 1;
        }
    }
    return 0;
}

// 1.13c D2Game+0xB4760 -- ECX game, EDX unit.
extern "C" U d2_fn_d2game_B4760(I p1, I p2)
{
    I iVar2 = S32(p2, 0);
    if (iVar2 != 2 && ((iVar2 < 4 || iVar2 > 5) && R32(p2, 0x2c) != 0)) Ord10750(R32(p2, 0x2c));
    U uVar1;
    iVar2 = S32(p2, 0);
    if (iVar2 == 2 || (iVar2 > 3 && iVar2 < 6)) uVar1 = R32(R32(p2, 0x2c), 0xc);
    else if (R32(p2, 0x2c) == 0) uVar1 = 0;
    else uVar1 = (U)Ord10867(R32(p2, 0x2c));
    I v = Ord10331(p2, 0, 0);
    v = D2GAME_E19C0(uVar1, v, p1, p2, p2);
    if (v == 0) return 0;
    U uVar1b, uVar3;
    S32(p2, 0xc4) |= 0xe;
    iVar2 = S32(p2, 0);
    if (iVar2 == 2 || (iVar2 > 3 && iVar2 < 6)) uVar1b = R32(R32(p2, 0x2c), 0x10);
    else if (R32(p2, 0x2c) == 0) uVar1b = 0;
    else uVar1b = (U)Ord10750(R32(p2, 0x2c));
    iVar2 = S32(p2, 0);
    if (iVar2 == 2 || (iVar2 > 3 && iVar2 < 6)) uVar3 = R32(R32(p2, 0x2c), 0xc);
    else if (R32(p2, 0x2c) == 0) uVar3 = 0;
    else uVar3 = (U)Ord10867(R32(p2, 0x2c));
    I u = Ord10331(p2, uVar3, uVar1b, 1, 0x1000);
    Ord10565(u);
    return 1;
}

// 1.13c D2Game+0xB7300 -- weighted pick over ECX's seeded PRNG (state at +0x20/+0x24).
extern "C" I d2_fn_d2game_B7300(I* weights, I count, uintptr_t self)
{
    uint32_t total = 0;
    for (I i = 0; i < count; i++) total += (uint32_t)weights[i];
    uint32_t pick;
    if ((int32_t)total < 1) {
        pick = 0;
    } else if ((total & (total - 1)) == 0) {
        uint64_t v = (uint64_t)R32(self, 0x20) * 0x6ac690c5ull + R32(self, 0x24);
        *(uint64_t*)(self + 0x20) = v;
        pick = (total - 1) & (uint32_t)v;
    } else {
        uint64_t v = (uint64_t)R32(self, 0x20) * 0x6ac690c5ull + R32(self, 0x24);
        *(uint64_t*)(self + 0x20) = v;
        pick = (uint32_t)((v & 0xffffffffu) % total);
    }
    int32_t acc = 0;
    I idx = 0;
    if (count > 0) {
        do {
            acc += (int32_t)weights[idx];
            if ((int32_t)pick < acc) return idx;
            idx++;
        } while (idx < count);
    }
    return 1;
}

// 1.13c D2Game+0xB5950 -- ECX game, EDX unit, stack arg, arg4; the row comes from DAT_6fd1b454[seed % 5].
extern "C" U d2_fn_d2game_B5950(I p1, I p2, U p3, I p4)
{
    (void)p1; (void)p3;
    uint64_t v = (uint64_t)R32(p2, 0x20) * 0x6ac690c5ull + R32(p2, 0x24);
    *(uint64_t*)(p2 + 0x20) = v;
    I iVar1 = D2GAME_DAT_F1B454[(uint32_t)(v & 0xffffffffu) % 5];
    uintptr_t t = DT();
    if (iVar1 < 0 || iVar1 >= (I)R32(t, 0xba0)) return 0;
    uintptr_t rec = (uintptr_t)R32(t, 0xb98) + (uintptr_t)iVar1 * 0x23c;
    if (rec == 0) return 0;
    if (S16(rec, 0x82) < 0 || S16(rec, 0x82) >= (int16_t)R32(t, 0xc4)) return 0;
    if (p2) R32(p2, 0xc4) |= 0x40;
    U uVar3 = R32(rec, 100);
    Ord10786();
    Ord10786(p2, R32(rec, 0x60), iVar1, p4);
    switch (uVar3) {
    case 0: case 1: case 2: case 3: case 4: break;
    default: Ord10832(0, iVar1); break;   // retail passes the stack slot uStack_18; not set on this path
    }
    return (U)D2GAME_A0AD0(p2, 3, p2, iVar1, (I)0x6FC70250);   // 0x50250 SkillNec_ApplySkillEffectSlots
}

// 1.13c D2Game+0xB65E0 -- ECX game, EDX unit, stack record id and a 4th argument; EBP is live.
extern "C" U d2_fn_d2game_B65E0(I p1, I p2, I p3, U p4, U ebp)
{
    (void)p1;
    uintptr_t t = DT();
    if (p3 < 0 || p3 >= (I)R32(t, 0xba0)) return 0;
    uintptr_t rec = (uintptr_t)R32(t, 0xb98) + (uintptr_t)p3 * 0x23c;
    if (rec == 0) return 0;
    I local4 = 0;
    I iVar1 = D2GAME_9EE50(0, &local4, p2, 1, p3, p4);
    if (iVar1 < 0) return 0;
    iVar1 = Ord10511(p2);
    if (iVar1 == 0) return 0;
    S32(p2, 0x3c) = 0;
    I iVar4 = Ord10786(p2, R32(rec, 0x138), p3, p3);
    if (iVar4 > 0) {
        iVar1 = D2GAME_EDDB0(ebp, p2);
        if (iVar1 != 0) R32(iVar1, 0xc4) |= 0x4000000;
        I uVar3 = (iVar1 != 0);
        iVar1 = 1;
        for (I iVar6 = 0; uVar3 < iVar4 && iVar1 < 5 && iVar6 < iVar4 * 2; iVar6++) {
            I iVar2 = D2GAME_EDDB0(8, p2);
            if (iVar2 == 0) {
                iVar1++;
            } else {
                R32(iVar2, 0xc4) |= 0x4000000;
                uVar3++;
            }
        }
    }
    D2GAME_DEEE0(p2, p2, p2, 1);
    return 1;
}

// 1.13c D2Game+0xB8AE0 -- EAX (in_EAX) is unused; the unit is param_1, the item-skill id param_2, the player param_4.
extern "C" U d2_fn_d2game_B8AE0(I p1, U p2, U p3, I p4)
{
    (void)p3;
    U uVar1 = R8(p1, 9);
    if (R8(p1, 9) != 0 && (uVar1 = (U)Ord10174(p4, 0x19, 0)) == 0) {
        if (p2 == 0x16f) {
            uVar1 = (U)Ord10174(p4, 0x19, 1);
            if (uVar1 == 1) return 1;
            if (R8(p1, 0xc) == 1) return (uVar1 & ~0xffu) | 1;
        } else if (p2 == 0x196 && (uVar1 = (U)Ord10174(p4, 0x19, 5)) == 0) {
            return 1;
        }
    }
    return uVar1 & 0xffffff00u;
}
