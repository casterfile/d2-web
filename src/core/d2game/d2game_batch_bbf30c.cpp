// src/core/d2game/d2game_batch_bbf30c.cpp -- fifteen 1.13c D2Game.dll functions transcribed from the
// cached Ghidra decompiles (build-wasm/ghidra-d2game/decomp/0x<RVA>.c). Base 0x6FC20000.
//
// Rows: C8990 C9CB0 CAB30 CC370 CF810 D1830 D2AB0 D2F00 D4E80 D60D0 C1030 BE640 BF990.
// Retail module state is modelled as opaque host records read by their retail byte offsets (32-bit
// pointer fields, as in the other batches). Callees that are not implemented in src/ are seams on
// D2GameBbf30cEnv: `o<N>` is D2Common/Fog ordinal N, `f<RVA>` is D2Game+0x<RVA>; each call passes the
// arguments in the decompile's order (unused trailing slots are zero). Register-only arguments
// (in_EAX, unaff_E**) are explicit parameters named at each function. The fatal path is Fog ordinal
// 10265(code) -> 10024(text, code) -> _exit(-1).
//
// Ambiguities the decompile cannot settle (BE640, BF990, C1510 reuse stack slots for several values;
// C9CB0 shows literal 0 for the unit register) are modelled at the best reading and flagged in the
// function comments as MISMATCH risk; they are not validated against retail here.
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <unistd.h>
#include <emscripten.h>

static inline uint8_t  rd8 (const void* p, uint32_t o) { return ((const uint8_t*)p)[o]; }
static inline uint16_t rd16(const void* p, uint32_t o) { uint16_t v; std::memcpy(&v, (const uint8_t*)p + o, 2); return v; }
static inline int16_t  rds16(const void* p, uint32_t o) { int16_t v; std::memcpy(&v, (const uint8_t*)p + o, 2); return v; }
static inline uint32_t rd32(const void* p, uint32_t o) { uint32_t v; std::memcpy(&v, (const uint8_t*)p + o, 4); return v; }
static inline void wr8 (void* p, uint32_t o, uint8_t v)  { ((uint8_t*)p)[o] = v; }
static inline void wr16(void* p, uint32_t o, uint16_t v) { std::memcpy((uint8_t*)p + o, &v, 2); }
static inline void wr32(void* p, uint32_t o, uint32_t v) { std::memcpy((uint8_t*)p + o, &v, 4); }

typedef uintptr_t (*Seam)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);

struct D2GameBbf30cEnv {
    Seam f1020;
    Seam f107E0;
    Seam f1100;
    Seam f1130;
    Seam f1180;
    Seam f1220;
    Seam f1280;
    Seam f12DF0;
    Seam f1370;
    Seam f1670;
    Seam f16F0;
    Seam f17E0;
    Seam f21260;
    Seam f2F300;
    Seam f338D0;
    Seam f33AF0;
    Seam f365E0;
    Seam f37680;
    Seam f48390;
    Seam f487A0;
    Seam f48FC0;
    Seam f6A720;
    Seam f70D60;
    Seam f70D90;
    Seam f79910;
    Seam f8A3E0;
    Seam f8ABF0;
    Seam f8AC90;
    Seam f8B010;
    Seam f8C810;
    Seam f8E6F0;
    Seam f8EA80;
    Seam f8F5C0;
    Seam f8F6C0;
    Seam f8F820;
    Seam f8FAA0;
    Seam f96A40;
    Seam f9BEB0;
    Seam f9C160;
    Seam f9E170;
    Seam fA22E0;
    Seam fB07B0;
    Seam fBC6B0;
    Seam fBC7D0;
    Seam fBCAA0;
    Seam fBCB20;
    Seam fBCC60;
    Seam fBCE60;
    Seam fBD160;
    Seam fBD330;
    Seam fBDD80;
    Seam fBF8F0;
    Seam fC1B20;
    Seam fC1B60;
    Seam fCA760;
    Seam fCBDB0;
    Seam fCE120;
    Seam fD1B20;
    Seam fD1BC0;
    Seam fD2070;
    Seam fD28D0;
    Seam fD29F0;
    Seam fD2C70;
    Seam fD2D90;
    Seam fD3450;
    Seam fD4670;
    Seam fD47C0;
    Seam fD4960;
    Seam fD4E80;
    Seam fD51A0;
    Seam fD6E30;
    Seam fD7160;
    Seam fD76A0;
    Seam fD8970;
    Seam fDF1A0;
    Seam fDFCC0;
    Seam fDFFD0;
    Seam fE0110;
    Seam fE03A0;
    Seam fE0F40;
    Seam fE150;
    Seam fE19C0;
    Seam fE1C00;
    Seam fE660;
    Seam fEDDB0;
    Seam fEE70;
    Seam fF2540;
    Seam name;
    Seam o10003;
    Seam o10012;
    Seam o10014;
    Seam o10020;
    Seam o10024;
    Seam o10045;
    Seam o10048;
    Seam o10050;
    Seam o10057;
    Seam o10058;
    Seam o10061;
    Seam o10076;
    Seam o10106;
    Seam o10107;
    Seam o10111;
    Seam o10114;
    Seam o10138;
    Seam o10141;
    Seam o10155;
    Seam o10159;
    Seam o10174;
    Seam o10201;
    Seam o10202;
    Seam o10237;
    Seam o10244;
    Seam o10258;
    Seam o10265;
    Seam o10284;
    Seam o10324;
    Seam o10331;
    Seam o10346;
    Seam o10348;
    Seam o10405;
    Seam o10407;
    Seam o10417;
    Seam o10427;
    Seam o10445;
    Seam o10450;
    Seam o10458;
    Seam o10459;
    Seam o10460;
    Seam o10463;
    Seam o10466;
    Seam o10509;
    Seam o10551;
    Seam o10585;
    Seam o10594;
    Seam o10614;
    Seam o10620;
    Seam o10624;
    Seam o10654;
    Seam o10655;
    Seam o10688;
    Seam o10695;
    Seam o10705;
    Seam o10729;
    Seam o10744;
    Seam o10750;
    Seam o10758;
    Seam o10792;
    Seam o10826;
    Seam o10867;
    Seam o10887;
    Seam o10892;
    Seam o10920;
    Seam o10946;
    Seam o10955;
    Seam o10961;
    Seam o10973;
    Seam o10977;
    Seam o10988;
    Seam o11017;
    Seam o11028;
    Seam o11030;
    Seam o11041;
    Seam o11055;
    Seam o11057;
    Seam o11065;
    Seam o11075;
    Seam o11088;
    Seam o11090;
    Seam o11116;
    Seam o11139;
    Seam o11144;
    Seam o11153;
    Seam o501;
    Seam o506;
    const char*    fatalFmt;    // DAT_6FD19FF9, the fatal-text format
    const uint8_t* dataTables;  // *sgptDataTables: +0xBF0, +0xC5C counts
    const uint8_t* bitMasks;    // gdwBitMasks
    const uint8_t* tbl22CC0;    // DAT_6FD22CC0: four 8-byte class-lock entries (stat dword, class byte)
    uintptr_t      d31c0c;      // DAT_6FD31C0C
    uintptr_t      d31c10;      // DAT_6FD31C10
};

static inline uint8_t* P(const void* b, uint32_t o) { return (uint8_t*)(uintptr_t)rd32(b, o); }
static inline uint8_t* PP(const void* b) { return (uint8_t*)(uintptr_t)b; }
static inline uint8_t* PN(uintptr_t v) { return (uint8_t*)v; }

typedef uintptr_t (*Seam)(uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
template <class... A> static inline uintptr_t call(Seam f, A... a)
{
    uintptr_t v[7] = { (uintptr_t)(a)... };
    return f(v[0], v[1], v[2], v[3], v[4], v[5], v[6]);
}
#define Q(name, ...) call(env->name, ##__VA_ARGS__)
#define Q1(name, ...) call(env->name, ##__VA_ARGS__)
#define Q2(name, ...) call(env->name, ##__VA_ARGS__)
#define Q3(name, ...) call(env->name, ##__VA_ARGS__)
#define Q4(name, ...) call(env->name, ##__VA_ARGS__)
#define Q5(name, ...) call(env->name, ##__VA_ARGS__)
#define Q6(name, ...) call(env->name, ##__VA_ARGS__)

[[noreturn]] static void die(const D2GameBbf30cEnv* env, uint32_t line)
{
    uintptr_t code = Q(o10265, line);
    Q(o10024, env->fatalFmt, code);
    _exit(-1);
}

static const char kSUnit[]    = "..\\Source\\D2Game\\UNIT\\SUnit.cpp";
static const char kItemMode[] = "..\\Source\\D2Game\\ITEMS\\ItemMode.cpp";
static const char kNpc[]      = "..\\Source\\D2Game\\UNIT\\SUNITNPC.CPP";
static const char kMonReg[]   = "..\\Source\\D2Game\\MONSTER\\MonsterRegion.cpp";

// Unit pointer is a server player when its type word (+0) is 0; the player data hangs off 10920.
static inline bool isPlayerUnit(const uint8_t* u) { return u != nullptr && rd32(u, 0) == 0; }

// The "player's gold word" the NPC replies carry: player-data +0x9C when the unit is a player, else 0.
static uintptr_t playerTag(const D2GameBbf30cEnv* env, uint8_t* unit)
{
    if (isPlayerUnit(unit)) return rd32(PN(Q(o10920, unit)), 0x9C);
    return 0;
}

// 1.13c D2Game+0xC8990
// Quest/NPC scripted-step poll. game+0x10F4 -> list head; first node of type 0xB with +9 set carries
// the step record (node+0x18). *out1 = "done", *out2 = "keep waiting"; early outs are (1,0). Where the
// decompile reads an uninitialised stack slot (the 10509 first probe buffer) a zeroed scratch block is
// passed. ECX of the 0xDFCC0 call is not set by this function (this=0).
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_C8990(uint8_t* game, uint8_t* unit, uint32_t* out1, uint32_t* out2, const D2GameBbf30cEnv* env)
{
    uint8_t* list = P(game, 0x10F4);
    if (list != nullptr) {
        for (uint8_t* n = P(list, 0); n != nullptr; n = P(n, 0xF4)) {
            if (rd32(n, 0) != 0xB) continue;
            if (rd8(n, 9) == 0) break;
            uint8_t* rec = P(n, 0x18);
            if (rd8(rec, 0xD) == 0) { *out1 = 1; *out2 = 0; return; }
            if (Q(f338D0, 0xC, 0xD) != 0 || Q(f338D0, 0xE, 0xD) != 0 || (uint8_t)Q(fC1B60) != 0) {
                *out1 = 1; *out2 = 0; return;
            }
            if (rd8(rec, 0x14) == 0) {
                if ((uint8_t)Q(fC1B20) != 0) wr8(rec, 0x12, 1);
                if (rd8(rec, 0x12) == 1) {
                    if (rd8(rec, 0x13) == 0 || rd8(rec, 0xF) != 0) {
                        wr8(rec, 0x1A, 0);
                        wr32(rec, 0x1C, 0);
                        Q(fDFCC0, 0, 0);
                        if (rd8(rec, 0x1A) != 0) {
                            Q(fF2540, game, unit, 0, 2, rd32(rec, 0x20), (uint8_t)rd32(rec, 0x24), 1);
                            wr8(rec, 0x13, 1);
                            wr8(rec, 0x12, 0);
                            *out1 = 0; *out2 = 0; return;
                        }
                    }
                    else {
                        *out1 = 0; *out2 = 1;
                        if (rd8(rec, 0x11) != 1) return;
                        if (Q(fB07B0) > 2) {
                            wr8(rec, 0x19, 1);
                            wr8(rec, 0x18, 0);
                            Q(fF2540, game, unit, 0, 2, rd32(rec, 0x20), (uint8_t)rd32(rec, 0x24), 1);
                            *out2 = 0; return;
                        }
                        uint8_t* t = P(game, 0x1520 + (rd32(rec, 0x38) & 0x7F) * 4);
                        while (t != nullptr && rd32(t, 0xC) != rd32(rec, 0x38)) t = P(t, 0xE4);
                        if (t == nullptr) { *out1 = 0; *out2 = 1; return; }
                        uint32_t loc[2] = { 0, 0 };
                        uint32_t pre[6] = { 0, 0, 0, 0, 0, 0 };
                        Q(o10141, t, loc);
                        uintptr_t h = Q(o10331, t);
                        if (Q(o10509, h, pre, 1, 0x3C01, 0) == 0 && Q(o10509, h, loc, 2, 0x3C01, 0) == 0
                            && Q(o10509, h, loc, 3, 0x3C01, 0) == 0) {
                            *out1 = 0; *out2 = 1; return;
                        }
                        if (Q(fE19C0, loc[0], h, game, unit, 0) != 0) {
                            wr8(rec, 0xF, 1);
                            wr8(rec, 0x14, 1);
                            return;
                        }
                    }
                    *out1 = 0; *out2 = 1; return;
                }
            }
            *out1 = 1; *out2 = 0; return;
        }
    }
    *out1 = 1; *out2 = 0;
}

// 1.13c D2Game+0xC9CB0
// Town-entry setup for a player. Retail: EAX = the player unit, stack = (game, item) with ret 8; `item` is the second stack word (the unit
// whose position and AllocUnit handle the tail uses). Seam argument layout (registers in the comments):
//   o10920 (0xA4C0) 1 stack word; o10003 (0xA57A) 3 words; fE660 (0xE660) EAX = unit, EBX = game, word = tag; f365E0 (0x365E0) ECX = unit,
//   words (game, unit+0x60); f9BEB0 (0x9BEB0) EAX = unit; f9C160 (0x9C160) EBX = 0, ESI = game, EDI = unit+0x80, word = callback RVA;
//   fDFCC0 (0xDFCC0) a = callback RVA (EBX), b = EDI (game); f33AF0 (0x33AF0) words (node, callback RVA); o11090 (0xA4A2) (item, 1);
//   o10331 (0xA478) (item); o10750 (0xA3B8) and o10867 (0xA454) 1 word; f1280 (0x1280) a = EAX (item handle), b = EBX (x), c = EDI (y);
//   o10346 (0xA51A) (v, 0). A missing type-0xD node, a null list and a null +0x2C are null dereferences in retail and return here.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_C9CB0(uint8_t* unit, uint8_t* game, uint8_t* item, const D2GameBbf30cEnv* env)
{
    uint8_t* pd = PN(Q(o10920, unit));
    Q(o10003, rd32(pd, 0x10 + rd8(game, 0x6D) * 4), 10, 0);
    pd = PN(Q(o10920, unit));
    Q(o10003, rd32(pd, 0x10 + rd8(game, 0x6D) * 4), 10, 0xD);
    if (Q(fE660, unit, game, 0x20747368) != 0) Q(f365E0, unit, game, rd32(unit, 0x60));   // "hst "
    if (Q(fE660, unit, game, 0x20706976) != 0) Q(f365E0, unit, game, rd32(unit, 0x60));   // "viv "
    if (Q(fE660, unit, game, 0x2066736D) != 0) Q(f365E0, unit, game, rd32(unit, 0x60));   // "msf "
    if (unit != nullptr && rd32(unit, 0x80) != 0 && rd32(unit, 0) == 0 && (uint16_t)Q(f9BEB0, unit) != 0xFFFF)
        Q(f9C160, 0xC7D00, game, rd32(unit, 0x80));
    wr8(P(game, 0x10F4), 0x20, 3);
    Q(fDFCC0, 0x359C0, (uintptr_t)game);

    uint8_t* node = nullptr;
    uint8_t* list = P(game, 0x10F4);
    if (list != nullptr)
        for (uint8_t* n = P(list, 0); n != nullptr; n = P(n, 0xF4))
            if (rd32(n, 0) == 0xD) { node = n; break; }
    if (node == nullptr) return;
    uint8_t* rec = P(node, 0x18);
    wr8(rec, 0xD, 1);
    wr8(rec, 0xE, 1);
    if (rd8(rec, 3) == 0) { Q(f33AF0, node, 0xC94E0); wr8(rec, 3, 1); }
    Q(o11090, item, 1);
    uintptr_t h = Q(o10331, item);
    if (h == 0) die(env, 0x686);
    const int32_t t = (int32_t)rd32(item, 0);
    const bool direct = t == 2 || (t > 3 && t <= 5);
    uint32_t y = 0, x = 0;
    if (direct) y = rd32(P(item, 0x2C), 0x10);
    else if (rd32(item, 0x2C) != 0) y = (uint32_t)Q(o10750, rd32(item, 0x2C));
    if (direct) x = rd32(P(item, 0x2C), 0xC);
    else if (rd32(item, 0x2C) != 0) x = (uint32_t)Q(o10867, rd32(item, 0x2C));
    uintptr_t other = Q(f1280, h, x - 0xD, y + 3);
    Q(o10346, h, 0);
    if (other != 0) Q(o10346, other, 0);
}

// 1.13c D2Game+0xCAB30
// Builds the per-region monster-type records MonsterRegion.cpp:0x87 for ids 1..count-1 of table +0xC5C, one 0x2E4-byte record each, and stores
// the 32-bit record pointers to out[1..]. Retail: EDX = seed, stack = (pool, out, seedp, diff, flag), ret 0x14. pool is the allocator context
// (ECX of 0xA2E6, [game+0x1C]), seedp the seed block (0xA628 ECX), diff the difficulty byte (checked by 0xA7BA, the fatal 0x83 path), flag the
// 16-bit-field selector (the caller's [game+0x70]). 0xCA760 takes EAX = (diff > 0), EBX = seedp; the spawn entries it writes at record+0x14
// (count byte at record+0x10, 0x34 bytes each) are rolled through 0xDF1A0(seedp, entry, 3).
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_CAB30(uint32_t seed, uint8_t* pool, uint32_t* out, uint8_t* seedp, uint32_t diff, uint32_t flag, const D2GameBbf30cEnv* env)
{
    Q(o10614, seedp, seed);
    const uint32_t u = diff & 0xFF;
    if (Q(o10237, u) == 0) die(env, 0x83);
    int32_t count = (int32_t)rd32(env->dataTables, 0xC5C);
    if (count <= 1) return;
    const uint32_t big = u != 0 ? 1u : 0u;
    uint32_t* cur = out + 1;
    for (int32_t id = 1; id < count; ) {
        uint8_t* rec = PN(Q(o10045, kMonReg, 0x87, 0, pool));
        std::memset(rec, 0, 0xB9 * 4);
        uint8_t* r = PN(Q(o10014, id));
        wr8(rec, 0, rd8(r, 3));
        wr32(rec, 0xB0 * 4, (uint32_t)id);
        wr32(rec, 0xAE * 4, rd32(r, 0x1C + u * 4));
        wr8(rec, 0x2BC, rd8(r, 0x28 + u));
        wr8(rec, 0x2BD, rd8(r, 0x2B + u));
        wr8(rec, 0x2BE, rd8(r, 0x2E));
        wr32(rec, 0xB5 * 4, 0xFFFFFFFFu);
        wr32(rec, 3 * 4, 0xFFFFFFFFu);
        wr8(rec, 0x2D8, rd8(r, 0x30));
        const int16_t s = flag != 0 ? rds16(r, 0x16 + u * 2) : rds16(r, 0x10 + u * 2);
        wr32(rec, 0xB7 * 4, (uint32_t)(int32_t)s);
        wr32(rec, 0xB8 * 4, (uint32_t)(int32_t)s);
        Q(fCA760, r, rec, big, seedp);
        uint8_t* e = rec + 0x14;
        for (int32_t k = 0; k < (int32_t)rd8(rec, 0x10); ++k, e += 0x34) Q(fDF1A0, seedp, e, 3);
        *cur++ = (uint32_t)(uintptr_t)rec;
        ++id;
        count = (int32_t)rd32(env->dataTables, 0xC5C);
    }
}

// 1.13c D2Game+0xCC370
// ECX = b (speaker), EAX = a (player), stack 0 = x (ret 4). Builds the 0x28-byte server packet 0x5A (kind 6, sub 4)
// naming the speaker (b) and hands it to 0xCBDB0 (EAX = packet, stack x). b of type 1 is first remapped through
// 0x41260 when that yields a type-0 unit. Layout from the asm (packet = esp+0x18): [0]=0x5A [1]=6 [2]=4 [3..6]=b id
// (-1 none) [7]=b type (6 none) [8..0x17]=name (SStrCopy #501 of a's player data +0x9C name at +0xD, 16 bytes; byte 0 cleared
// when #506 rejects it) [0x18..]=speaker word (the item-data word at +0x26 when b is a type-1 unit whose +0x14 record has
// flag 2 in +0x16) or, for a type-0 b, b's own name (copied to a temporary, kept when #506 accepts it). Failures are the
// 10265/10024/_exit path: 0x190B when a is null or not a player, 0x9D9 on a missing player-data tag.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_CC370(uint8_t* a, uint8_t* b, uint32_t x, const D2GameBbf30cEnv* env)
{
    if (a == nullptr || rd32(a, 0) != 0) die(env, 0x190B);
    if (b != nullptr && rd32(b, 0) == 1) {
        uint8_t* t = PN(Q(f21260, b));
        if (t != nullptr && rd32(t, 0) == 0) b = t;
    }
    uint8_t pkt[0x28];
    std::memset(pkt, 0, sizeof pkt);
    uint8_t nameTmp[16];
    pkt[0] = 0x5A; pkt[1] = 6; pkt[2] = 4;
    wr32(pkt, 3, b == nullptr ? 0xFFFFFFFFu : rd32(b, 4));
    pkt[7] = b == nullptr ? 6 : (uint8_t)rd32(b, 0);
    pkt[0x18] = 0;
    uint8_t* tag = P(PN(Q(o10920, a)), 0x9C);
    if (tag == nullptr) die(env, 0x9D9);
    if (Q(o506, tag + 0xD) != 0) Q(o501, pkt + 8, tag + 0xD, 16); else pkt[8] = 0;
    uint32_t type = b == nullptr ? 6 : rd32(b, 0);
    if (type == 0) {
        if (b == nullptr || rd32(b, 0) != 0) die(env, 0x9D9);
        uint8_t* tag2 = P(PN(Q(o10920, b)), 0x9C);
        if (tag2 == nullptr) die(env, 0x9D9);
        Q(o501, nameTmp, tag2 + 0xD, 16);
        if (Q(o506, nameTmp) != 0) Q(o501, pkt + 0x18, nameTmp, 16);
    } else if (type == 1 && b != nullptr && P(b, 0x14) != nullptr && (rd8(P(b, 0x14), 0x16) & 2) != 0) {
        wr16(pkt, 0x18, rd16(P(b, 0x14), 0x26));
    }
    Q(fCBDB0, pkt, x);
}

// 1.13c D2Game+0xCF810
// Spawns the pet/summon companion for player unit `unit` toward `other`: owner class bytes from 10459
// must match; unit types other than 2/4/5 refresh the 0x2C-handle (10867 then 10750); a missing target
// handle (10331) is fatal 0xE42, a class mismatch fatal 0xE5C. A new type-2 unit id 0x3B is created
// from the owner template (SUnit.cpp:0x265, 0x38-byte extra block) and placed via 0xC22E0. Decompile
// hidden arguments (10826/10459 owner reads) are given the unit they follow.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_CF810(uint32_t unusedGame, uint8_t* unit, uint8_t* other, const D2GameBbf30cEnv* env)
{
    (void)unusedGame;
    uint8_t c1 = (uint8_t)Q(o10459, other);
    uintptr_t target = Q(o10331, unit);
    uint8_t* owner = PN(Q(o10826, unit));
    uint8_t c2 = (uint8_t)Q(o10459, owner);
    if (other == nullptr) { Q(o10585, c2); Q(f9E170); return; }
    if (c1 != c2) { Q(f9E170); return; }
    uint32_t t = rd32(unit, 0);
    if (t != 2 && (t < 4 || t > 5) && rd32(unit, 0x2C) != 0) Q(o10867, rd32(unit, 0x2C));
    t = rd32(unit, 0);
    if (t != 2 && (t < 4 || t > 5) && rd32(unit, 0x2C) != 0) Q(o10750, rd32(unit, 0x2C));
    if (target == 0) die(env, 0xE42);
    if (Q(o10057, target) != 0) { Q(fDFFD0, 0x18, unit); return; }
    c1 = (uint8_t)Q(o10459, c2);
    uint8_t* tu = PN(Q(o10826, target));
    c2 = (uint8_t)Q(o10459, tu);
    if (c1 != c2) die(env, 0xE5C);
    uint32_t buf[8] = { 0 };
    if (Q(o10654, target, buf, buf, 3, 0x3E01, 0xC01, 0) == 0) return;
    uint8_t* pet = owner;
    uint8_t* puVar9 = PN(Q(f1280));
    if (puVar9 == nullptr) return;
    uint8_t* nu = PN(Q(o11153, rd32(tu, 0x1C), 2));
    uint8_t* sel = owner;
    if (nu != nullptr) {
        wr32(nu, 0, 2);
        wr32(nu, 4, 0x3B);
        wr32(nu, 8, rd32(tu, 0x1C));
        wr32(nu, 0x80, (uint32_t)(uintptr_t)tu);
        uint32_t f = rd32(nu, 0xC8);
        f = rd32(tu, 0x70) == 0 ? (f & 0xFDFFFFFFu) : (f | 0x2000000u);
        wr32(nu, 0xC8, f | 0x4000000u);
        uint8_t* o2 = PN(Q(o10826, puVar9));
        uint8_t cls = (uint8_t)Q(o10459, o2);
        wr8(nu, 0x18, cls);
        wr32(nu, 0x1C, rd32(tu, (0x2F + cls) * 4));
        wr32(nu, 0x74, 0);
        Q(fE0110);
        wr32(nu, 0xC4, rd32(nu, 0xC4) | 0x10);
        wr32(nu, 0xD0, 0xB);
        int32_t ctr = (int32_t)rd32(tu, 0x98) + 1;
        if (ctr == -1) ctr = 1;
        wr32(tu, 0x98, (uint32_t)ctr);
        uint8_t* ex = PN(Q(o10045, kSUnit, 0x265, 0));
        wr32(nu, 0x14, (uint32_t)(uintptr_t)ex);
        std::memset(ex, 0, 0xE * 4);
        wr32(P(nu, 0x14), 0, (uint32_t)Q(o10688, 0x3B));
        wr32(nu, 0x10, 1);
        Q(o10445, nu);
        Q(f2F300);
        puVar9 = pet;
        Q(fE0F40, target, nu, buf, owner);
        sel = owner;
    }
    Q(o11090, nu, 1);
    uint8_t* placed = PN(Q(fA22E0, sel, nu, owner));
    if (placed == nullptr) return;
    uint8_t b = (uint8_t)Q(o10258, placed);
    Q(o10111, placed, (uint32_t)b | 3);
    Q(o10346, puVar9, 0);
    uintptr_t h = Q(o10331, placed, 0);
    Q(o10346, h);
}

// 1.13c D2Game+0xD1830
// Unit-in-hand compatibility probe. eax = the unit whose +0x60 block is queried, ebx = the compare
// handle. 10244 result is returned when 11144 is zero or the item/type relation holds, else 0.
extern "C"
EMSCRIPTEN_KEEPALIVE int32_t d2_fn_d2game_D1830(uint8_t* eax, uintptr_t ebx, const D2GameBbf30cEnv* env)
{
    int32_t r = (int32_t)Q(o10244);
    if (r == 0 || Q(o11144) == 0) return r;
    uintptr_t blk = rd32(eax, 0x60);
    uintptr_t it = Q(o11139, blk, 4);
    if (it == ebx) it = Q(o11139, blk, 5);
    if (it != 0 && Q(o10744, it, Q1(o11144)) != 0) return r;
    return 0;
}

// 1.13c D2Game+0xD2AB0
// Weighted pick from a two-part id array (first n=block[0] entries at +4, spill entries at +0x80, n +
// block[0x7C] total) using the 64-bit LCG at seed (lo*0x6AC690C5+hi, power-of-two counts mask, others
// modulo the low 32 bits). The picked id's unit (10450 out local) then cuts the result to
// esi+0x88 / esi+0x8C by the 21180 rolls against the difficulty table 10237(+8/+0xC, +0x20/+0x24).
extern "C"
EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_D2AB0(uint8_t* seed, uint8_t* block, uint8_t* esi, const D2GameBbf30cEnv* env)
{
    uint8_t* sp = seed + 0x20;
    uint32_t n = (uint32_t)block[0x7C] + block[0];
    uint32_t idx;
    auto roll = [&]() {
        uint64_t v = (uint64_t)rd32(sp, 0) * 0x6AC690C5ull + rd32(sp, 4);
        std::memcpy(sp, &v, 8);
        return v;
    };
    if (n == 0) idx = 0;
    else if ((n & (n - 1)) == 0) idx = (n - 1) & (uint32_t)roll();
    else idx = (uint32_t)((roll() & 0xFFFFFFFFull) % n);
    uint32_t id = (int32_t)idx < (int32_t)block[0] ? rd32(block, idx * 4 + 4) : rd32(block, (idx - block[0]) * 4 + 0x80);
    uint8_t* local = nullptr;
    uint8_t* e = PN(Q(o10450, id, &local));
    if (e != nullptr) {
        if (rd16(local, 0x78) < rd16(e, 0xF6)) die(env, 0x19CA);
        if (rd32(e, 0x88) != 0x20202030u && rd32(e, 0x88) != 0) {
            uint8_t* d = PN(Q(o10237, rd8(local, 0x6D)));
            int32_t lim1 = local == nullptr ? (int32_t)rd32(d, 8) : (int32_t)rd32(d, 0xC);
            if ((int32_t)Q(f1180, sp) < lim1) {
                id = rd32(esi, 0x88);
                int32_t lim2 = local == nullptr ? (int32_t)rd32(d, 0x20) : (int32_t)rd32(d, 0x24);
                if ((int32_t)Q(f1180, sp) < lim2) id = rd32(esi, 0x8C);
            }
        }
    }
    return id;
}

// 1.13c D2Game+0xD2F00
// Can the item `item` be equipped by player edi (edi+0x60 = inventory). The class-lock stat table
// (bytes at env->tbl22CC0: 4 entries of 8, dword stat id at +0, class code byte at +4), the 'ass '/
// 'j43 '/'xyz '/'g33 '/'tr2 ' class-name checks (10174 on the 0x21020 stat handle) and the nested-item
// check via 0xD2070 are as decompiled. The decompile's comma expression assigning unaff_retaddr to
// the item pointer in the shield branch is a stack artifact and ignored.
extern "C"
EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_D2F00(uint8_t* item, uint8_t* edi, const D2GameBbf30cEnv* env)
{
    uintptr_t inv = rd32(edi, 0x60);
    uint8_t* inv2 = PN(Q(o10460, inv));
    if (inv == 0) return 0;
    uint8_t* row = PN(Q(o10695, item == nullptr ? 0xFFFFFFFFu : rd32(item, 4)));
    if (row == nullptr) return 0;
    uint8_t* chk = item;
    if (item != nullptr && rd32(item, 0) == 4 && Q(o10106, item) == 7 && (int32_t)Q(o10620, item) >= 0) {
        Q(o10620, item);
        uint8_t* t = PN(Q(f16F0));
        if (t != nullptr && (rd8(env->bitMasks, 8) & rd8(t, 0x2C)) != 0 && Q(fD2070, item) != 0) return 0;
    }
    if (rd8(row, 0x12A) == 0) return 1;
    uint32_t tag = rd32(row, 0x80);
    bool need = false;     // reached the shared "10174 == 0 -> fail" tail
    uintptr_t a = 0, b = 0, c = 0;
    if (tag == 0x20737361u) {                       // "ass "
        uint8_t* pd = PN(Q(o10920));
        a = rd32(pd, 0x10 + rd8(item, 0x6D) * 4); b = 9; c = 5; need = true;
    }
    else if (tag == 0x2034336Au) {                  // "j43 "
        if (Q(o10174, Q1(f1020), 0x14, 0) != 0) return 0;
    }
    else if (tag == 0x207A7978u) {                  // "xyz "
        a = Q(f1020); b = 0x14; c = 5; need = true;
    }
    else if (tag == 0x20333367u) {                  // "g33 "
        if (Q(o10174, Q1(f1020), 0x13, 7) != 0) return 0;
        if (Q(o10174, Q1(f1020), 0x13, 8) != 0) return 0;
    }
    else if (tag == 0x20327274u) {                  // "tr2 "
        if (Q(o10174, Q1(f1020), 0x25, 8) == 0) return 0;
        if (Q(o10174, Q1(f1020), 0x25, 7) != 0) return 0;
    }
    if (need && Q(o10174, a, b, c) == 0) return 0;

    int32_t k = (int32_t)rd8(row, 0x12A) - 1;
    if (k == 9) {
        if (Q(o10174, Q1(f1020), 10, 0) != 0 && rd32(row, 0x80) != 0x20786F62u) return 0;
    }
    else if (k == 4) {
        if (Q(o10174, Q1(f1020), 4, 0) != 0 && rd32(row, 0x80) != 0x2067656Cu) return 0;
    }
    else {
        for (uint32_t u = 0; u < 0x20; u += 8) {
            if (rd8(row, 0x12A) == env->tbl22CC0[u + 4]) {
                if (Q(o10174, Q1(f1020), rd32(env->tbl22CC0, u), 0) != 0) return 0;
            }
        }
    }
    if (Q(fD2070, chk) != 0) return 0;
    uintptr_t it = Q(o10284, rd32(edi, 0x60));
    while (it != 0) {
        uint32_t id = (uint32_t)Q(o10155, it);
        if (item == nullptr) die(env, 0x2D3);
        for (uint8_t* n = P(item, 0x1120 + (id & 0x7F) * 4); n != nullptr; n = P(n, 0xE4)) {
            if (rd32(n, 0xC) == id) {
                if (rd32(n, 0x60) != 0 && Q(o10460, rd32(n, 0x60)) != 0 && Q(fD2070, inv2) != 0) return 0;
                break;
            }
        }
        it = Q(o10348, it);
    }
    return 1;
}

// 1.13c D2Game+0xD4E80
// Takes item `id` out of the player's inventory (self+0x60) into the cursor. eax = item id, game is
// the server game; *notFound is set to 1 when no item with the id exists or the item is not in mode 4
// slot, 0 otherwise. param_3 selects the 10655 (zero) or 10729 removal route. Fatal 0x11FC (no game),
// 0x11FD (no player).
extern "C"
EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_D4E80(uint8_t* self, uint8_t* game, uint32_t p3, uint32_t p4, uint32_t* notFound, uint32_t id, const D2GameBbf30cEnv* env)
{
    if (game == nullptr) die(env, 0x11FC);
    if (self == nullptr) die(env, 0x11FD);
    *notFound = 0;
    Q(fD29F0, self);
    uint8_t* it = P(game, 0x1720 + (id & 0x7F) * 4);
    for (;; it = P(it, 0xE4)) {
        if (it == nullptr) break;
        if (rd32(it, 0xC) != id) continue;
        if (rd32(it, 0) != 4) return 0;
        if (rd32(it, 0x10) == 4) {
            if (Q(o10792, it) == 0) return 0;
            uintptr_t inv = rd32(self, 0x60);
            if (p3 == 0) {
                Q(o10114, it, &p3, &p4, kItemMode, 0x1223);
                if (Q(o10655, inv, it, it) == 0) return 0;
                Q(o10012, it, 0xFF);
            }
            else if (Q(o10729, inv, it) == 0) return 0;
            if (Q(o10405, inv, it, 2) != 0) {
                Q(o10466, inv, 0);
                Q(fD4670, self);
                wr32(it, 0xC4, rd32(it, 0xC4) & 0xFFFFFFFDu);
                if (Q(o11057, it, self) != 0) Q(fD47C0);
                Q(o11090, it, 2);
                Q(o10012, it, 0xFF);
                Q(o10892, it, 0x400, 1);
                Q(o10201, self, 1);
                Q(o10050, inv, it);
                return 1;
            }
        }
        break;
    }
    *notFound = 1;
    return 0;
}

// 1.13c D2Game+0xD60D0
// Moves item `id` from the ground (game item hash +0x1720) into the player's inventory block
// (self+0x60) for mode `mode`; modes 0xB/0xC skip the 10244 test. eax = item id, ebp = `skipPd`
// (nonzero skips the 10061/11055 step). *out3 is set to 1 when 10405 refuses, *out4 to 1 when the
// item is missing (0 on a 10946 failure). Fatal 0xAEF (no game) / 0xAF0 (no player).
extern "C"
EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_D60D0(uint8_t* self, uint8_t* game, uint8_t mode, uint32_t* out3, uint32_t* out4, uint32_t id, uint32_t skipPd, const D2GameBbf30cEnv* env)
{
    const bool skipTest = (mode == 0xB || mode == 0xC);
    if (game == nullptr) die(env, 0xAEF);
    if (self == nullptr) die(env, 0xAF0);
    if (out4 != nullptr) *out4 = 0;
    Q(fD29F0, self);
    uint8_t* it = P(game, 0x1720 + (id & 0x7F) * 4);
    for (;; it = P(it, 0xE4)) {
        if (it == nullptr) break;
        if (rd32(it, 0xC) != id) continue;
        if (rd32(it, 0) != 4) return 0;
        if (rd32(it, 0x10) == 4) {
            if (Q(o10946, self, (uint32_t)mode, it) != 1) {
                if (out4 != nullptr) *out4 = 0;
                return 0;
            }
            if (skipTest || Q(o10244, it, self, 0, 0, 0, 0) != 0) {
                uintptr_t inv = rd32(self, 0x60);
                if (skipPd == 0) {
                    uintptr_t x = Q(o10061, inv);
                    if (x != 0) Q(o11055, self, x);
                }
                if (Q(o11041, inv, it, (uint32_t)mode) != 0) {
                    if (Q(o10405, inv, it, mode != 0 ? 4 : 3) == 0) {
                        if (out3 != nullptr) *out3 = 1;
                        return 0;
                    }
                    Q(o10058, it, (uint32_t)mode);
                    if (mode == 0) { Q(o10138, inv, it); Q(fD47C0); }
                    Q(o10466, inv, 0);
                    wr32(it, 0xC4, rd32(it, 0xC4) & 0xFFFFFFFDu);
                    Q(o11090, it, 1);
                    Q(o10012, it, 0xFF);
                    Q(o10892, it, 8, 1);
                    Q(o10977, it, 1, 1);
                    if (Q(o10202, it, 0x4000, 0xB4F, kItemMode) != 0) Q(o10977, it, 0x4000, 0);
                    Q(o10050, inv, it);
                    Q(o10201, self, 1);
                    Q(fD28D0);
                    Q(fD51A0, 0xB4F, self);
                }
                return 1;
            }
        }
        break;
    }
    if (out4 != nullptr) *out4 = 1;
    return 0;
}

// 1.13c D2Game+0xC1030
// Pending-reward refresh for player self (+0x70 = reward block). Walks the block's 16-byte slots
// [lo..hi] of the 10427 record (+0x114/+0x116) and, for the first slot with +8 != 1 and +0xC == 1,
// marks it pending (+8 = 1), creates the item through 0xDDDB0 (retrying twice) and attaches it with
// 0x68FC0. Fatal 0xFA2 when 10427 fails. Finishes with 0xDD160(id).
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_C1030(uint8_t* self, uint8_t* unit, uint32_t id, const D2GameBbf30cEnv* env)
{
    uint8_t* blk = PN(Q(f8F5C0, self));
    if (blk == nullptr) return;
    blk = P(blk, 0x10);
    if (blk == nullptr) return;
    bool scan = false;
    if (rd32(self, 0x70) == 0) scan = true;
    else {
        if (Q(f96A40) != 0) { Q(fBD160, id); return; }
        if (rd32(self, 0x70) == 0) scan = true;
    }
    if (scan) {
        uint8_t* pd = PN(Q(o10920, unit));
        if (pd != nullptr && rd32(env->dataTables, 0xBF0) > 7 && P(pd, 0x44) != nullptr) {
            uint8_t* head = P(P(pd, 0x44), 0) + 0x54;
            for (uint8_t* s = P(head, 0); s != nullptr; s = P(s, 0x14)) {
                if ((rd8(s, 0) & 1) != 0) continue;
                uint8_t* h = P(self, 0x1320 + (rd32(s, 4) & 0x7F) * 4);
                while (h != nullptr) {
                    if (rd32(h, 0xC) == rd32(s, 4)) return;
                    h = P(h, 0xE4);
                }
                break;
            }
        }
    }
    uint8_t* rec = PN(Q(o10427, rd32(self, 0x70), id, rd8(self, 0x6D), 0));
    if (rec == nullptr) die(env, 0xFA2);
    uint32_t hi = rd16(rec, 0x116), lo = rd16(rec, 0x114);
    int32_t span = (int32_t)(hi - lo);
    if (lo <= hi) {
        int32_t k = 0;
        uint8_t* a = blk + 0xC;
        uint8_t* b = blk + 8;
        do {
            if (span + 1 <= k) break;
            if (rd32(b, 0) != 1 && rd32(a, 0) == 1) {
                uint8_t* slot = blk + k * 0x10;
                wr32(slot, 8, 1);
                if (isPlayerUnit(unit)) Q(o10920, unit);
                Q(f8ABF0);
                uint8_t* item = PN(Q(fEDDB0, 1, unit));
                if (item == nullptr) item = PN(Q(fEDDB0, 1, unit));
                if (item == nullptr) item = PN(Q(fEDDB0, 1, unit));
                if (item == nullptr) return;
                Q(f48FC0, self, unit, item, 0xFFFF, slot, 0);
                break;
            }
            a += 0x10; b += 0x10;
            ++lo; ++k;
        } while ((int32_t)lo <= (int32_t)hi);
    }
    Q(fBD160, id);
}

// Builds the 15-byte 0x2A reply the decompile assembles in stack slots (0xAA3E0 sink): byte 0 is the
// slot the decompile leaves as an unnamed low byte of the first local (modelled 0x2A), byte 1 zero,
// bytes 7..10 the item id, bytes 11..14 the stat-0xE word. The result code written at +0xE first is
// overwritten by the stat word's top byte in the decompile, so it is not placed (MISMATCH risk).
static void sendItemReply(const D2GameBbf30cEnv* env, uint8_t* unit, uint32_t itemId)
{
    uint8_t pkt[15];
    std::memset(pkt, 0, sizeof pkt);
    pkt[0] = 0x2A;
    if (isPlayerUnit(unit)) Q(o10920, unit);
    wr32(pkt, 7, itemId);
    wr32(pkt, 11, (uint32_t)Q(o10973, unit, 0xE, 0));
    Q(f8A3E0, pkt, 0xF);
}

// 1.13c D2Game+0xBE640
// NPC buy of item `id` by player unit (game hash +0x1720). `mode` 0 = plain, 2 = needs 0xAFAA0 and
// 0x2E150 == 1, anything else skips the gate. Price = 10107(...); the player must afford it from stat
// 0xE + 0xF (else reply code 0xC). Replies go through 0xAAC90(code, stat word, player tag, unit, 0xE):
// 7 can't, 9 clone failure, 10 pay failure, 5 stack bought, 4 normal purchase. The decompile conflates
// the price and the item pointer (both live in pvVar12) and stores the stackable flag in a stack
// slot; the price is modelled as `price` and the flag as `stackable` (10744 result). The loop head's
// compare (`sel` against the two globals 0x6FD31C0C / 0x6FD31C10) is literal.
extern "C"
EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_BE640(uint8_t* game, uint8_t* unit, uint8_t* npc, uint32_t id, int16_t mode, const D2GameBbf30cEnv* env)
{
    if (game == nullptr) die(env, 0);
    uint8_t* item = P(game, 0x1720 + (id & 0x7F) * 4);
    while (item != nullptr && rd32(item, 0xC) != id) item = P(item, 0xE4);
    if (item == nullptr) {
        sendItemReply(env, unit, id);
        return 1;
    }
    auto reply = [&](uint32_t code, uintptr_t word) {
        Q(f8AC90, code, word, playerTag(env, unit), unit, 0xE);
    };
    auto reply0 = [&](uint32_t code) {
        reply(code, Q(o10973, unit, 0xE, 0, 0xFFFFFFFFu, 0));
    };
    if (mode == 0) {
        if (Q(fE150, npc) == 0) { reply0(7); return 1; }
    }
    else if (mode == 2) {
        if (Q(f8FAA0, game) == 0) { reply0(7); return 1; }
        if (Q(fE150, npc) != 1) { reply0(7); return 1; }
    }
    uint32_t npcId = npc == nullptr ? 0xFFFFFFFFu : rd32(npc, 4);
    uint8_t cls = rd8(game, 0x6D);
    uint8_t* pd = PN(Q(o10920));
    int32_t price = (int32_t)Q(o10107, unit, item, cls, rd32(pd, 0x10 + cls * 4), npcId, id & 0xFFFF);
    int32_t gold = (int32_t)Q(o10973, unit, 0xE, 0);
    int32_t bank = (int32_t)Q(o10973);
    if (gold + bank < price) { reply0(0xC); return 0; }

    uintptr_t inv = rd32(unit, 0x60);
    if (Q(o11017, inv) != 0) { reply0(7); return 1; }
    Q(fBCAA0, item);
    uintptr_t stackable = Q(o10744, item);
    uint8_t* stk = nullptr;
    if (stackable != 0) stk = PN(Q(fD1B20));
    int32_t maxBuy = 0;
    uint8_t* pstk = nullptr;
    uint8_t* qtyItem = nullptr;
    uint32_t qty = 0;
    if (stackable == 0 || stk == nullptr) {
        qty = (uint32_t)Q(o10973, item, 0x46, 0);
        bool direct = (stackable != 0 && Q(o10955, item) != 0);
        if (direct) {
            Q(o10887, item, 0x46, 1, 0);
            uint8_t* pd2 = PN(Q(o10920));
            (void)pd2;
            uint32_t unitPrice = (uint32_t)Q(o10107, unit, item, rd8(game, 0x6D), Q1(f1020), npcId, 0);
            Q(o10887, item, 0x46, qty, 0);
            int32_t canBuy = (int32_t)((uint32_t)(gold + bank) / unitPrice);
            int32_t ceiling = 0;
            uint8_t* existing = PN(Q(fBC6B0, unit, &ceiling));
            int32_t lim = canBuy <= ceiling ? canBuy : ceiling;
            if (existing == nullptr) {
                if (lim < 2) lim = 1;
                maxBuy = lim;
            }
            else {
                if (ceiling < canBuy) canBuy = ceiling;
                if (canBuy > 0 && Q(fBCC60) != 0) {
                    Q(fBDD80, item, 0, unit, 0);
                    Q(o10551, existing, 0x46, canBuy, 0);
                    if (isPlayerUnit(unit)) Q(o10920, unit);
                    Q(o10973, existing, 0x46, 0);
                    Q(f8B010);
                    uint32_t eid = rd32(existing, 0xC);
                    uintptr_t w = Q(o10973, unit, 0xE, 0, eid, 5);
                    Q(f8AC90, 5, w, playerTag(env, unit), unit, 0xE);
                    return 0;
                }
                reply0(0xC);
                return 0;
            }
        }
        // LAB_6fcdeabb: the clone-and-attach loop
        int32_t paid = 0;
        uintptr_t fit = Q(o10624, inv, item);
        uint8_t* slotItem = fit != 0 ? stk : nullptr;
        uintptr_t sel = (uintptr_t)unit;
        uint8_t* cur = item;
        (void)pstk; (void)qtyItem; (void)maxBuy;
        for (;;) {
            if (sel != 0 && slotItem == nullptr) return 0;
            uintptr_t gold0 = Q(o10973, unit, 0xE, 0);
            uintptr_t bank0 = Q(o10973, unit, 0xF, 0);
            uint8_t* nu = PN(Q(f107E0, cur));
            if (nu == nullptr) { reply0(9); return 1; }
            if (stackable != 0 && Q(o10048, nu) != 0 && qty != 0) Q(o10887, nu, 0x46, qty, 0);
            if (Q(fBCC60) == 0) { reply0(0xC); return 0; }
            uint8_t* pd3 = PN(Q(o10920, unit));
            wr32(pd3, 0x6C, rd32(nu, 0xC));
            Q(o11090, nu, 4);
            uintptr_t again = Q(o10624, gold0);
            uint32_t slot = 0;
            bool got = false;
            if (again != 0) {
                paid = (int32_t)Q(fD4E80, unit, cur, 0, 1, &slot);
                got = paid != 0;
            }
            bool skipToAttach = false;
            if (!got && paid == 0) {
                bool allow = true;
                uintptr_t p61 = Q(o10061, 0);
                uintptr_t p76 = Q(o10076);
                if (sel == env->d31c0c && p61 != 0 && p76 != 7) allow = false;
                bool toC0a = false, toC16 = false;
                if (sel == env->d31c10) {
                    if (p61 != 0) {
                        if (p76 != 1) allow = false;
                        if (Q(o10744, nu, 0x2D) == 0) toC16 = true;
                    }
                    else toC0a = true;
                }
                else if (p61 == 0) toC0a = true;
                else if (Q(o10744, nu, 0x2D) == 0) toC16 = true;
                if (toC0a && sel != env->d31c0c && sel != env->d31c10) toC16 = true;
                if (toC16 && allow && Q(fD4960, nu, unit) != 0) skipToAttach = true;
                if (!skipToAttach) {
                    Q(o10012, nu, 0);
                    paid = (int32_t)Q(fD76A0, cur, 0, rd32(nu, 0xC));
                    if (paid == 0) {
                        if (Q(o10973, unit, 0xE) != 0xF) Q(f1670, 0xF - (int32_t)Q1(o10973, unit, 0xE, 0));
                        uintptr_t b2 = Q(o10973, unit, 0xF, 0);
                        if (bank0 != b2) Q(f1670, (int32_t)bank0 - (int32_t)Q1(o10973, unit, 0xF, 0));
                        Q(fEE70);
                        reply0(10);
                        return 0;
                    }
                }
            }
            Q(fBDD80, cur, 0, unit, id & 0xFFFF);
            Q(o10977, nu, 2, 1);
            uintptr_t w = Q(o10973, unit, 0xE, 0, rd32(nu, 0xC), 4);
            Q(f8AC90, (uintptr_t)0, w, playerTag(env, unit), nu, 0xE);
            sel = 1;
            cur = nullptr;
        }
    }
    // stackable item with an existing partial stack (0xF1B20 hit)
    if (Q(fBCC60) != 0) {
        Q(fD3450);
        Q(fBDD80, item, 0, unit, 0);
        uintptr_t w = Q(o10973, unit, 0xE, 0, rd32(stk, 0xC), 5);
        Q(f8AC90, 0, w, playerTag(env, unit), unit, 0xE);
        return 0;
    }
    reply0(0xC);
    return 0;
}

// Earlier reading of D2Game 0xBF990 (not proved); the 1.13c port is src/core/d2game/d2game_bf990.cpp
// NPC sell/repair/identify of item `id` (hash +0x1720) to the NPC `npcId` (eax, hash +0x1320). Reply
// 0xAAC90(code, stat word, player tag, unit, 0xE): 1 done, 9 bad state/clone failure, 0xB no gold slot.
// mode: 0 sell (10744 type 0x16 or 0x12 stack route), 4 buy-back (0xF2C70), other = 0xF8970 route.
// uStack_18 (the unit-price cap) is never initialised in retail on the success path and is zero
// here. Fatal 0x4A9 on no game. The error packet is the 15-byte 0x2A reply of sendItemReply with the
// id -1 and the "bad npc" code 9 ordering unrecovered.
extern "C"
EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_BF990(uint8_t* game, uint8_t* unit, uint32_t id, uint32_t mode, uint32_t npcId, const D2GameBbf30cEnv* env)
{
    if (game == nullptr) die(env, 0x4A9);
    uint8_t* owner = nullptr;
    if (rd8(unit, 0x6C) != 0) owner = PN(Q(fE03A0, game, rd32(unit, 0x64)));
    uint8_t* npc = P(game, 0x1320 + (npcId & 0x7F) * 4);
    while (npc != nullptr && rd32(npc, 0xC) != npcId) npc = P(npc, 0xE4);
    uint8_t* item = P(game, 0x1720 + (id & 0x7F) * 4);
    while (item != nullptr && rd32(item, 0xC) != id) item = P(item, 0xE4);
    if (item == nullptr) return 1;
    auto reply0 = [&](uint32_t code) {
        Q(f8AC90, code, Q1(o10973, unit, 0xE, 0, 0xFFFFFFFFu, 0), playerTag(env, unit), unit, 0xE);
    };
    if (npc == nullptr || owner != npc) {
        sendItemReply(env, unit, 0xFFFFFFFFu);
        return 1;
    }
    if (Q(fE150, unit) == 0) { reply0(0xB); return 3; }
    if (rd32(item, 0x10) != (mode & 0xFFFF) || Q(o10705, item) == 0) { reply0(9); return 3; }
    Q(o10107, unit, item, rd8(game, 0x6D), Q1(f1020), rd32(npc, 4), 1);
    bool ok = true;
    if (Q(o10106, item) == 1) {
        const char* nm = (const char*)Q(o10961, Q1(o10620, item));
        if (__builtin_strcasecmp(nm, "Cracked") == 0) ok = false;
    }
    if (Q(o10202, item, 0x100, 0x500, kNpc) != 0) ok = false;
    if (Q(o11088, item) == 7) ok = false;
    if (Q(o10202, item, 0x1000000, 0x508, kNpc) != 0) ok = false;
    if (Q(o10202, item, 0x400000, 0x50C, kNpc) != 0) ok = false;
    if (Q(fD1BC0) != 0) ok = false;
    if ((uint8_t)Q(f17E0) != 0) ok = false;
    Q(f8F820, game);
    if (rd8(PN(Q(f8F6C0)), 4) != 0) ok = false;

    int32_t cap = 0;
    uint8_t* clone = nullptr;
    if (ok && (uint8_t)Q(fBCAA0, item) == 0) {
        clone = PN(Q(f107E0, item));
        if (clone == nullptr) { reply0(9); return 3; }
        Q(o11090, clone, 4);
        int8_t c = (int8_t)Q(o11075, clone);
        if (c == -1) { Q(fEE70); clone = nullptr; }
        else {
            auto price = [&]() {
                Q(fBCE60);
                Q(o10887, clone, 0x48, Q1(o11116, clone), 0);
                Q(o10887, clone, 0x46, Q1(o10463, clone), 0);
                int32_t v = (int32_t)Q(o10107, unit, clone, rd8(game, 0x6D), Q1(f1020), rd32(npc, 4), 1);
                if (v < cap) cap = v;
            };
            auto place = [&](uint32_t slotv) {
                Q(o10012, clone, slotv);
                uint32_t cid = rd32(clone, 0xC);
                Q(f1130, clone);
                Q(f1100, clone);
                return Q(fD76A0, game, npc, cid) != 0;
            };
            if (place((uint8_t)c)) price();
            else {
                if (c == 1 && place(2)) price();
                else Q(fEE70);
            }
        }
    }
    if ((int16_t)mode != 0) {
        if ((int16_t)mode == 4) {
            if (Q(fD2C70) == 0) { reply0(9); return 1; }
        }
        else {
            uint32_t m = mode;
            if (Q(fD8970, game, unit, Q1(o10417, item), 1, &m) == 0) {
                if (clone == nullptr) return 1;
                Q(fEE70);
                return 1;
            }
        }
    }
    else {
        uint8_t* use = item;
        uintptr_t row = 0;
        bool haveRow = false;
        if (Q(o10744, item, 0x16) == 0) {
            if (Q(o10744, item, 0x12) != 0) {
                int32_t q = (int32_t)Q(o10973, item, 0x46, 0);
                if (q > -1) {
                    uint16_t s = (uint16_t)Q(o11028, item, 0, q);
                    row = rd32(PN(Q(o11065, s)), 0xC);
                    haveRow = true;
                }
            }
        }
        else {
            use = nullptr;
            uint16_t s = (uint16_t)Q(o11028, item, 0, 1);
            row = rd32(PN(Q(o11065, s)), 8);
            haveRow = true;
        }
        if (haveRow) Q(fBF8F0, row);
        if (isPlayerUnit(unit)) Q(o10920, unit);
        uint32_t zero = 0;
        Q(o10758, &zero, 0x20, 1);
        uintptr_t t = Q(o10020, use);
        Q(o11030, use, t);
        Q(f8C810);
        Q(fD2D90, game);
    }
    Q(f12DF0);
    Q(f8AC90, 1, Q1(o10973, unit, 0xE, 0, id, 3), playerTag(env, unit), unit, 0xE);
    return 0;
}
