// Batch aea40b: D2Game functions at RVA 0xAEDC0-0xB3D60, transcribed from the cached 1.13c decompiles
// (build-wasm/ghidra-d2game/decomp/0x<RVA>.c). Same shape as d2game_batch_aea40.cpp: retail 32-bit struct
// layout read as raw bytes off a base address (R32/S16/U16/R8 at byte offsets; the decompile's int* index
// arithmetic is converted to byte offsets), register inputs as explicit parameters, callees as weak externs
// (Ord<N> for D2Common ordinals, D2GAME_<RVA> for D2Game internals) so the file links standalone.
// Our implementations are d2_fn_d2game_<RVA>.
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

// *sgptDataTables_exref is the tables block (+0xc4 unit-table count, +0xbcc/+0xbd4 skill-table base/count,
// +0xb98/+0xba0 record base/count). Same convention as d2game_batch_aea40.cpp.
extern "C" __attribute__((weak)) char sgptDataTables_exref[];
#define DTB() ((uintptr_t)R32(sgptDataTables_exref, 0))

O(10328); O(10331); O(10392); O(10393); O(10428); O(10459); O(10540); O(10551); O(10585); O(10680);
O(10695); O(10785); O(10807); O(10826); O(10830); O(10839); O(10871); O(10887); O(10911); O(10913);
O(10916); O(10920); O(10945); O(10959); O(10973); O(11013); O(11043); O(10057); O(10188); O(10024);
O(10265); O(10510); O(10689); O(10830);

F(1100); F(1130); F(1180); F(11D0); F(1450); F(4EA30); F(7A260); F(8C3B0); F(8E6F0); F(8EA80);
F(9F7A0); F(B0A20); F(B17B0); F(C24C0); F(DFA90); F(DFFD0); F(E0460); F(E1570); F(E07A0); F(386F0);
F(37480); F(48560); F(2E4F0); F(5E3C0); F(A0AD0); F(AADE); F(DE0C0); F(DCF20); F(9F210); F(A24C0);
F(A42A); F(A83E); F(ACE2); F(A598); F(B7D30);

// ---------------------------------------------------------------------------------------------------
// 1.13c D2Game+0xAEDC0 -- FUN_6fccedc0(undefined4 param_1, int *param_2, int param_3); unit in ESI.
// Walks the three skill-stat slots of the unit (+0x9e) and applies each to the stat list.
// Decompile artifacts: uStack_c and iStack_24 are read before any write; both are kept as locals
// (uStack_c = 0, iStack_24 unused). See MISMATCH notes in the spec once it lands.
extern "C" U d2_fn_d2game_AEDC0(U p1, I p2, I p3, I esi)
{
    (void)p1;
    U uStack_c = 0;
    I unit;
    I uVar7 = (p3 == 0) ? (I)0xffffffff : (I)S32(p3, 4);
    unit = Ord10695(uVar7);
    if (unit == 0) return 0;

    U uVar2 = R32(unit, 0xb0);
    I piVar17 = p2;
    I iVar9 = Ord10428(p2, p3);
    I iVar10 = S16(unit, 0x98);
    I item = 0;
    I base = 0;                       // iStack_2c: unit-base stat delta
    if (0 < iVar9) {
        if (iVar10 < 1) return 0;
        if ((I)R32(DTB(), 0xc4) <= iVar10) return 0;
        item = Ord10871(p2, iVar10);
        if (item != 0) {
            base = Ord10959(item) - S32(esi, 0xa8);
        }
    }
    U uStack_1c = 0;
    bool bVar4 = false;
    I idx = 0;                        // iStack_20
    do {
        I iVar15 = S16(unit, 0x9e + idx * 2);
        if (iVar15 < 0 || (I)R32(DTB(), 0xbd4) <= iVar15) break;
        I iVar14 = iVar15 * 0x144 + (I)R32(DTB(), 0xbcc);
        I iVar13 = item;
        if (iVar14 != 0) {
            I iVar11 = Ord10428(p2, uStack_c, S32(unit, 0xa4 + idx * 4));
            I uVar7b = 0;
            bool doCase = true;
            switch (iVar15) {
            case 0x1a:
                iVar11 = (I)((uint32_t)iVar11 << 8);
                /* fallthrough */
            case 8:
                iVar11 = Ord10393(p2, iVar11);
                uVar7b = 1;
                break;
            case 0x4a:
                iVar11 = (I)((uint32_t)iVar11 << 8);
                /* fallthrough */
            case 6:
                iVar11 = Ord10540(p2, iVar11);
                uVar7b = 3;
                break;
            default:
                doCase = false;
                break;
            }
            if (doCase) {
                I cnt = Ord10973(p2, uVar7b, 0);
                if (0 < cnt) {
                    U uA = (U)D2GAME_11D0((I)((uintptr_t)p2 + 0x20));
                    U uB = (U)D2GAME_1180((I)((uintptr_t)p2 + 0x20));
                    if ((int32_t)uB < (int32_t)(uA >> 1)) iVar11 *= 2;
                }
            }
            // every path reaches the shift (the decompile's default jump lands on this label)
            iVar11 = (I)((uint32_t)iVar11 << (R8(iVar14, 0x18) & 0x1f));
            if (0 < iVar11) {
                if (iVar15 == 6) {
                    bVar4 = true;
                    if (p2 != 0 && R32(p2, 0) == 0) iVar11 = D2GAME_4EA30(iVar11);
                } else if (iVar15 == 0x4a) {
                    bVar4 = true;
                    if (p2 != 0 && R32(p2, 0) == 0) iVar11 = D2GAME_4EA30(iVar11);
                }
                if (iVar9 < 1) {
                    I iVar14b = Ord10973(p2, iVar15, 0);
                    if ((I)U16(item, 0x32) < (I)R32(DTB(), 0xbd4)) {
                        I t = Ord10973(p2, U16(item, 0x32), 0);
                        item = t;                          // decompile reuses iVar18 here
                        if (item < iVar14b + iVar11) iVar11 = item - iVar14b;
                    }
                    Ord10551(p2, iVar15, iVar11, 0);
                } else {
                    U uVar16 = (U)(S32(esi, 0xa8) + base + iVar9);
                    if (item == 0) {
                        I iVar18b;
                        if (p2 == 0) {
                            iVar18b = -1;
                            iVar13 = 6;
                        } else {
                            iVar18b = R32(p2, 0xc);
                            iVar13 = R32(p2, 0);
                        }
                        iVar13 = Ord11013(R32(esi, 0x1c), 2, uVar16, iVar13, iVar18b);
                        if (iVar13 == 0) goto next_slot;
                        item = iVar10;
                        Ord10785(iVar13, iVar10);
                        Ord10945(p2, iVar10, 1);
                        if (p2 == 0) {
                            D2GAME_8EA80(piVar17, p2, 3, R32(piVar17, 0xa8) + 1, 0, 0, 0);
                        } else if (R32(p2, 0) != 0) {
                            U pc = R32(p2, 0xdc);
                            while (pc != 0) {
                                uint8_t cVar1 = R8(pc, 0);
                                pc = R32(pc, 0x24);
                                if (cVar1 == 3) {
                                    D2GAME_8E6F0();
                                    uVar16 = uVar2;
                                }
                            }
                            D2GAME_8EA80(piVar17, p2, 3, R32(piVar17, 0xa8) + 1, 0, 0, 0);
                        }
                        Ord10911(item, (I)&D2GAME_9F7A0);
                        Ord10807(p2, item, 1);
                    }
                    Ord11043(item, uVar16);
                    item = Ord10680(item, iVar13, 0);
                    if (idx == 0) {
                        D2GAME_8EA80(esi, p2, 0xc, uVar16, 0, 0, 0);
                    }
                    if (0 < base + iVar9) {
                        Ord10188(iVar13, iVar15, (item * base + iVar11) / (base + iVar9), 0);
                    }
                }
                uStack_1c = 1;
            }
        }
    next_slot:
        idx = idx + 1;
        item = iVar13;
    } while (idx < 3);

    if (bVar4 && p2 != 0 && R32(p2, 0) == 0) {
        U bVar5 = (U)Ord10973(p2, 0x160, 0) & 0xff;
        I cnt = Ord10973(p2, 6, 0);
        I s = Ord10913(p2) >> 8;
        U bVar6;
        if (s < 1 || s <= (cnt >> 8)) {
            bVar6 = 0x80;
        } else {
            bVar6 = (U)((((cnt >> 8) << 7) / s) & 0xff);
        }
        uStack_c = bVar6;
        I d = (I)bVar5 - (I)bVar6;
        if (d < 0) d = -d;
        if (4 < d) {
            D2GAME_7A260();
            Ord10887(p2, 0x160, bVar6, 0, uStack_c);
        }
    }
    (void)piVar17;
    return uStack_1c;
}

// ---------------------------------------------------------------------------------------------------
// 1.13c D2Game+0xAF450 -- FUN_6fccf450: ECX = game, EDX = unit (the decompile's param_1 / param_2), five unused stack words (ret 0x14).
// Every callee is a seam in D2GameAF450Env with the registers the disassembly gives it; a unit of type 0 only (the tail's [unit] != 0
// branch is dead because the unit type was tested at entry).
struct D2GameAF450Env {
    U (*room)(U unit);                                      // Ordinal 10331 (0xA478)
    U (*area)(U room);                                      // Ordinal 10826 (0xA556)
    U (*b10459)(U area);                                    // Ordinal 10459 (0xA56E), result byte
    U (*c10585)(U b);                                       // Ordinal 10585 (0xAADE)
    U (*s10057)(U room);                                    // Ordinal 10057 (0xA808)
    void (*s386f0)(U unit);                                 // 0x386F0, EAX = unit
    U (*s1130)(U unit);                                     // 0x1130, ECX = unit
    U (*s1100)(U unit);                                     // 0x1100, ECX = unit
    U (*sa24c0)(U game, U unit, U room, U r1100, U r1130, U c, U* out);   // 0xA24C0, EAX = unit, stack ..., &out, 0x3B, 0
    void (*sffd0)(U unit, U a, U b);                        // 0xDFFD0, ESI = unit, stack (a, b)
    void (*s37480)(U unit, U v);                            // 0x37480, EAX = unit, stack v
    void (*s2e4f0)(U obj, U v);                             // 0x2E4F0, ECX = obj, stack v
    U (*se1570)(U game, U obj);                             // 0xE1570, stack (game, obj)
    void (*s5e3c0)(U obj, U game);                          // 0x5E3C0, EAX = obj, ESI = game
    U (*a4c0)(U unit);                                      // Ordinal 10920 (0xA4C0)
    void (*s8c3b0)(U al, U cl, U edx, U arg);               // 0x8C3B0
};

// edi: the caller's EDI, which the routine reads without setting (the decompile's unaff_EDI): non-zero adds the 0x8C3B0 notification and
// returns the created object instead of the 0xA24C0 result. The recorded retail calls carry EDI = 0.
extern "C" U d2_fn_d2game_AF450(U game, U unit, const D2GameAF450Env* env, U edi)
{
    if (unit == 0 || R32(unit, 0) != 0) return 0;
    R32(unit, 0xc4) |= 0x40;
    U room = env->room(unit);
    U area = env->area(room);
    U b = env->b10459(area) & 0xff;
    U c = env->c10585(b);
    if (env->s10057(room) != 0 || env->area(room) == 0x88) {
        env->sffd0(unit, 0x18, unit);
        return 0;
    }
    env->s386f0(unit);
    U out = 0;
    U r1130 = env->s1130(unit);
    U r1100 = env->s1100(unit);
    U res = env->sa24c0(game, unit, room, r1100, r1130, c, &out);
    env->sffd0(unit, 7, 0);
    if (res != 0) {
        env->s37480(unit, out == 0 ? 0xffffffffu : R32(out, 0xc));
        if (out != 0) {
            env->s2e4f0(out, R32(unit, 0xc));
            U e = env->se1570(game, out);
            if (e != 0) {
                env->s2e4f0(e, R32(unit, 0xc));
                env->s5e3c0(e, game);
            }
            env->s5e3c0(out, game);
        }
    }
    if (edi != 0) {
        U id = R32(edi, 0xc), t = R32(edi, 0);
        env->s8c3b0(0x7c, t & 0xff, id, R32(env->a4c0(unit), 0x9c));
        return out;
    }
    return res;
}

// ---------------------------------------------------------------------------------------------------
// 1.13c D2Game+0xB1820 -- FUN_6fcd1820(int param_1, undefined4 param_2, int param_3, undefined4 *param_4, int *param_5), ret 0x14;
// the unit is in EAX. param_1 is the game (the three node tables at +0x1120/+0x1320/+0x1920, 128 buckets each, chained through +0xE4,
// key +0xC), param_3 a flag. [[unit+0x14]+0x38] is the mode: 1/2/4 look the unit's id (0x48560) up in their table, 3 asks B17B0 for a
// target. Returns 1 with *param_4 = node and *param_5 = value, else 0 (clearing +0x34/+0x38 of the unit's data on the way out).
// Every callee is a seam in D2GameB1820Env with the registers the disassembly gives it. Null game with mode 1/2/4 is retail's fatal path
// (std::abort here).
struct D2GameB1820Env {
    U (*s48560)(U unit);                                   // ECX = unit
    U (*sb0a20)(U node, U unit);                           // EBX = ESI = node, stack unit (ret 4)
    U (*sad78)(U node, U unit, U four);                    // thunk, stack (node, unit, 4)
    U (*sa514)(U unit);                                    // thunk, stack unit
    U (*s1450)(U esi, U two);                              // ESI = unit+0x20, stack 2
    void (*se0460)(U unit, U bl, U one);                   // EAX = unit, BL, stack 1
    U (*s1130)(U unit);                                    // ECX = unit
    U (*s1100)(U unit);                                    // ECX = unit
    U (*sa478)(U unit, U r1100, U r1130);                  // thunk, stack (unit, r1100, r1130)
    U (*sad66)(U v);                                       // thunk, stack v
    void (*sb17b0)(U eax, U* blk, U game, U unit, U zero, int nwords);   // EAX = 5/6, ECX = &blk, EDX = game, stack (unit, 0)
    U (*sdfa90)(U node);                                   // EAX = node
};
extern "C" U d2_fn_d2game_B1820(U unit, U game, U param_2, U param_3, U *param_4, int32_t *param_5, const D2GameB1820Env* env)
{
    if (unit == 0 || R32(unit, 0) != 1 || R32(unit, 0x14) == 0) return 0;
    U mode = R32(R32(unit, 0x14), 0x38);
    if (mode == 0) return 0;
    U node = 0, val = 0;
    bool have = false;
    if (mode - 1 <= 3) {
        if (mode == 3) {
            U ebx = env->sa514(unit);
            U eax = env->s1450(unit + 0x20, 2);
            U bl = 0;
            bool call = false;
            if (ebx == 1) { call = true; bl = (eax == 0) ? 0 : 2; }
            else if (eax != 0) {
                if (ebx == 0) { call = true; bl = 2; }
                else if (ebx == 2) { call = true; bl = 0; }
            }
            if (call) env->se0460(unit, bl, 1);
            U blk[7] = { 0, 0x7fffffff, 0, 0, 0, 0, 0 };
            if (param_3 == 0) {
                U r1130 = env->s1130(unit);
                U r1100 = env->s1100(unit);
                U ad = env->sad66(env->sa478(unit, r1100, r1130));
                blk[2] = param_2; blk[3] = 0x23; blk[4] = ad; blk[5] = 0; blk[6] = 0x7fffffff;
                env->sb17b0(5, blk, game, unit, 0, 7);
            } else {
                blk[3] = 0x7fffffff;
                env->sb17b0(6, blk, game, unit, 0, 4);
            }
            node = blk[0]; val = blk[1];
            env->se0460(unit, ebx, 1);   // BL is reloaded from the saved A514 result
            have = true;
        } else {
            U id = env->s48560(unit);
            U base = mode == 1 ? 0x1120 : mode == 2 ? 0x1320 : 0x1920;
            if (game == 0) std::abort();
            U n = R32(game, base + (id & 0x7f) * 4);
            while (n != 0 && R32(n, 0xc) != id) n = R32(n, 0xe4);
            if (n != 0) {
                node = n;
                val = env->sb0a20(n, unit);
                have = !(param_3 != 0 && env->sad78(n, unit, 4) != 0);
            }
        }
    }
    if (have && node != 0) {
        int32_t k = S32(node, 0);
        if (k >= 0) {
            bool ok = false;
            if (k <= 1) ok = env->sdfa90(node) == 0;
            else if (k == 3) ok = true;
            if (ok) { *param_4 = node; *param_5 = (int32_t)val; return 1; }
        }
    }
    R32(R32(unit, 0x14), 0x38) = 0;
    R32(R32(unit, 0x14), 0x34) = 0;
    return 0;
}

// ---------------------------------------------------------------------------------------------------
// 1.13c D2Game+0xB3D60 -- __fastcall(ecx = pGame, edx = pUnit, nSkill, a2); ret 8.
// The skill row's event block D (0x1C dwords, zeroed) is built here and handed to 9F210 / DE0C0 / DCF20 by pointer. Every callee is a
// typed weak seam (b3d60_<name>, 32-bit retail words) so the replay records the exact register and stack arguments.
#define S3(n, ...) extern "C" __attribute__((weak)) uint32_t b3d60_##n(__VA_ARGS__)
S3(e07a0, uint32_t unit, uint32_t game);            // EAX = unit, ECX = game
S3(o10392, uint32_t path);
S3(o10510, uint32_t unit, uint32_t skill, uint32_t a2, uint32_t one);
S3(f9f210, uint32_t skill, uint32_t unit, uint32_t pEvent, uint32_t a2);   // EAX = skill, EDI = unit, stack event, a2
S3(fde0c0, uint32_t m, uint32_t game, uint32_t unit, uint32_t one, uint32_t pEvent);   // EAX = m, stack game, unit, 1, event
S3(o10973, uint32_t unit, uint32_t stat, uint32_t layer);
S3(fdcf20, uint32_t m, uint32_t game, uint32_t unit, uint32_t pEvent);     // EAX = m, stack game, unit, event
S3(o10689, uint32_t unit, uint32_t stat, uint32_t value);
// retail reads the table block as ds:[0xF829C] -> a cell -> the tables: two loads
static inline uintptr_t DT3() { return (uintptr_t)R32(R32(sgptDataTables_exref, 0), 0); }
extern "C" U d2_fn_d2game_B3D60(U game, I unit, I skill, I a2)
{
    if (skill < 0 || skill >= (I)S32(DT3(), 0xba0)) return 0;
    uintptr_t rec = (uintptr_t)skill * 0x23c + R32(DT3(), 0xb98);
    if (rec == 0) return 0;
    if (unit != 0) R32(unit, 0xc4) |= 0x40;
    b3d60_e07a0((U)unit, game);
    U m = b3d60_o10392(R32(unit, 0x2c));
    if (m == (U)unit || m == 0) return 1;
    U D[0x1c];
    for (int i = 0; i < 0x1c; i++) D[i] = 0;
    D[1] = 1 | U16(rec, 0x12e);
    D[0] |= R32(rec, 0x130);
    if (R32(rec, 0x134) != 0) D[0x18] = R32(rec, 0x134);
    D[13] = b3d60_o10510((U)unit, (U)skill, (U)a2, 1);
    b3d60_f9f210((U)skill, (U)unit, (U)(uintptr_t)D, (U)a2);
    b3d60_fde0c0(m, game, (U)unit, 1, (U)(uintptr_t)D);
    if (b3d60_o10973(m, 6, 0) == 0) D[1] |= 2;
    b3d60_fdcf20(m, game, (U)unit, (U)(uintptr_t)D);
    int16_t w = S16(rec, 0x4e);
    if (0 < w && (int)w <= S32(DT3(), 0xbc0)) b3d60_o10689(m, (U)w, 0);
    return 1;
}
