// src/core/d2game/d2game_batch_d36d0.cpp -- twelve 1.13c D2Game.dll functions (0xD36D0 .. 0xD8600)
// transcribed from the cached Ghidra decompiles (build-wasm/ghidra-d2game/decomp/0x<RVA>.c): the
// stackable-quantity merge steps (0xD36D0, 0xD3A10, 0xD3CB0, dispatched by 0xD4360 via 0xD3C40), the
// pickup/ground-drop helpers (0xD3950, 0xD39B0, 0xD6710, 0xD6BC0) and the item-mode moves
// (0xD5C30, 0xD8020, 0xD8600). Batch lane: implementation + marker only, no spec, no vector.
//
// Host model as in d2game_batch_D40F0.cpp: units/games/items are opaque uint32 handles, struct
// fields addressed by retail byte offset, register-only arguments (in_EAX/unaff_E*) are explicit
// parameters, and every callee outside this batch is a seam on D2GameBatchD36D0Env taking up to six
// uint32 words in retail argument order (a missing trailing word is zero). Ordinal numbers are the
// 1.13c numbers from the decompiles. The fatal path Fog #10265(line) -> #10024(text) -> _exit(-1)
// is d2game_fatal. Arguments the decompiler lost to register reuse are resolved from the
// surrounding stack layout and flagged "reg:" in the function comment.
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <unistd.h>
#include <emscripten.h>

static inline uint32_t rd32(const void* p, uint32_t o) { uint32_t v; std::memcpy(&v, (const uint8_t*)p + o, sizeof v); return v; }
static inline void wr32(void* p, uint32_t o, uint32_t v) { std::memcpy((uint8_t*)p + o, &v, sizeof v); }
static inline void* H(uint32_t h) { return (void*)(uintptr_t)h; }

[[noreturn]] static void d2game_fatal(uint32_t) { _exit(-1); }

typedef uint32_t (*Sx)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

struct D2GameBatchD36D0Env {
    // Fog / D2Common ordinals
    Sx o10013, o10012, o10050, o10058, o10061, o10126, o10127, o10128, o10133, o10138, o10201, o10202,
       o10244, o10265, o10331, o10405, o10408, o10463, o10466, o10489, o10584, o10605, o10608, o10643,
       o10646, o10694, o10744, o10758, o10836, o10865, o10885, o10887, o10892, o10920, o10946, o10955,
       o10964, o10973, o10977, o11016, o11041, o11051, o11055, o11057, o11088, o11090, o11102, o11139,
       o11144;
    // D2Game internals
    Sx sub2EE70, sub8AF10, sub8A3E0, sub8B010, sub8D3D0, sub8C810, sub1BC0_D1BC0, sub_D28D0, sub_D29F0,
       sub_D2CF0, sub_D2D90, sub_D31D0, sub_D34B0, sub_D3A10x, sub_D46B0, sub_D47C0, sub_D51A0,
       sub_D6710x, sub_D7E30, sub_DFC10, sub_DFFD0, sub_E20A0, sub_D1650, sub_D20A0;
};

template <class... A> static inline uint32_t call(Sx f, A... a) {
    uint32_t v[6] = {0, 0, 0, 0, 0, 0}; uint32_t t[] = {(uint32_t)a...}; for (size_t i = 0; i < sizeof...(A); ++i) v[i] = t[i];
    return f(v[0], v[1], v[2], v[3], v[4], v[5]);
}
#define E const D2GameBatchD36D0Env* env
#define FATAL(line) do { call(env->o10265, (uint32_t)(line)); d2game_fatal(line); } while (0)

// Stat 0x46 = quantity, 0x48 = durability. PlayerData (o10920) +0x9C is the client id passed to 0x8B010.
static inline uint32_t clientOf(E, uint32_t game) {
    return rd32(H(game), 0) == 0 ? rd32(H(call(env->o10920, game)), 0x9C) : 0;
}

// 1.13c D2Game+0xD36D0
// Stack merge of an item into an existing stack. param_2 = game/client, param_3 = source item;
// reg: ESI = target stack, EDI = source quantity (stat 0x46 of param_3). Over max: clamp target and
// send, remainder stays on the source; otherwise absorb and finish.
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_D36D0(uint32_t p1, uint32_t game, uint32_t item, uint32_t esiTarget, uint32_t ediQty, E)
{
    (void)p1;
    if (call(env->o11102, item) == 0) return;
    if ((int32_t)call(env->o10973, item, 0x46, 0) < 0) FATAL(0x114c);
    int32_t tq = (int32_t)call(env->o10973, esiTarget, 0x46, 0);
    if (tq < 0) FATAL(0x114e);
    int32_t mx = (int32_t)call(env->o10463, esiTarget);
    if (mx < 0) FATAL(0x1150);
    int32_t sum = tq + (int32_t)ediQty;
    if (mx < sum) {
        call(env->o10887, esiTarget, 0x46, mx, 0);
        call(env->sub8B010, game, clientOf(env, game), esiTarget, 0x46, mx);
        int32_t rem = (tq - mx) + (int32_t)ediQty;
        if (rem < 0) FATAL(0x1159);
        call(env->o10887, item, 0x46, rem, 0);
        call(env->sub8B010, game, clientOf(env, game), item, 0x46, rem);
        if (call(env->o11088, item) == 0x12 && call(env->o11088, esiTarget) == 0x12) {
            call(env->o10463, esiTarget);
            call(env->sub_D31D0, game);
        }
        call(env->o10977, item);
        return;
    }
    if (call(env->o10865, item) != 0) {
        int32_t d = (int32_t)call(env->o10973, item, 0x48, 0);
        int32_t d2 = (int32_t)call(env->o10973, esiTarget, 0x48, 0);
        if (d < d2) {
            call(env->o10887, esiTarget, 0x48, d, 0);
            call(env->sub8B010, game, clientOf(env, game), esiTarget, 0x48, d);
        }
    }
    if (sum < 0) FATAL(0x1177);
    call(env->o10887, esiTarget, 0x46, sum, 0);
    call(env->sub8B010, game, clientOf(env, game), esiTarget, 0x46, sum);
    if (call(env->o11088, item) == 0x12 && call(env->o11088, esiTarget) == 0x12) call(env->sub_D31D0, game);
    call(env->o10466, ediQty, 0);
    call(env->sub8AF10, clientOf(env, game));
    call(env->sub2EE70);
}

// old D3950 (superseded by d2game_d3950.cpp g_d3950)
// reg: EDI = player/client record. Frees via o10920 when its first dword is 0, clears a 0x20 slot
// (Ordinal_10758(&0, 0x20, 1)), then 0x8D3D0 and 0xD2CF0 on EDI.
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_d3950_old(uint32_t ediPlayer, E)
{
    if (ediPlayer != 0 && rd32(H(ediPlayer), 0) == 0) call(env->o10920);
    uint32_t zero = 0;
    call(env->o10758, (uint32_t)(uintptr_t)&zero, 0x20, 1);
    call(env->sub8D3D0);
    call(env->sub_D2CF0, ediPlayer);
}

// 1.13c D2Game+0xD39B0
// param_1 = player/client record; reg: EBX forwarded to 0xD2D90. Same shape as 0xD3950 with 0x8C810.
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_D39B0(uint32_t param1, uint32_t ebx, E)
{
    if (param1 != 0 && rd32(H(param1), 0) == 0) call(env->o10920, param1);
    uint32_t zero = 0;
    call(env->o10758, (uint32_t)(uintptr_t)&zero, 0x20, 1);
    call(env->sub8C810);
    call(env->sub_D2D90, ebx);
}

// 1.13c D2Game+0xD3A10
// Stack merge of the carried item into the stack found by Ordinal_10964(player+0x60). param_1 =
// game (non-null), param_2 = player, param_3 = item. Returns 1 when merged, 0 when no target stack.
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_D3A10(uint32_t game, uint32_t player, uint32_t item, E)
{
    if (game == 0) FATAL(0x623);
    uint32_t tgt = call(env->o10964, rd32(H(player), 0x60), item, 0);
    if (tgt == 0) return 0;
    int32_t q = (int32_t)call(env->o10973, item, 0x46, 0);
    if (q < 0) FATAL(0x62f);
    int32_t tq = (int32_t)call(env->o10973, tgt, 0x46, 0);
    if (tq < 0) FATAL(0x631);
    int32_t mx = (int32_t)call(env->o10463, tgt);
    if (mx < 0) FATAL(0x633);
    int32_t sum = tq + q;
    if (mx < sum) {
        call(env->o10887, tgt, 0x46, mx, 0);
        call(env->sub8B010, player, clientOf(env, player), tgt, 0x46, mx);
        int32_t rem = (tq - mx) + q;
        if (rem < 0) FATAL(0x63b);
        call(env->o10887, 0, 0x46, rem, 0);          // reg: source item (decompile shows uVar1==0)
        call(env->sub8B010, player, clientOf(env, player), item, 0x46, rem);
        call(env->sub_D31D0, player);
        return 1;
    }
    if (sum < 0) FATAL(0x646);
    call(env->o10887, tgt, 0x46, sum, 0);
    call(env->sub8B010, player, clientOf(env, player), tgt, 0x46, sum);
    call(env->sub_D31D0, player);
    uint32_t room = call(env->o10331, item);
    if (room != 0) {
        uint32_t a = item ? rd32(H(item), 0) : 6, b = item ? rd32(H(item), 0xC) : 0xFFFFFFFFu;
        call(env->o10885, room, a, b);
        call(env->o10408, item);
        call(env->o11016, item);
    }
    if (item) wr32(H(item), 0xC4, rd32(H(item), 0xC4) & 0xfffffffdu);
    call(env->sub2EE70);
    call(env->o10466, item, 0);
    return 1;
}

// 1.13c D2Game+0xD3C40
// reg: EAX = flag (both branches identical), EBX = item (non-null), EDI = player. Looks up the
// stack target with Ordinal_10964(player+0x60) and hands it to 0xD34B0.
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_D3C40(uint32_t eax, uint32_t ebxItem, uint32_t ediPlayer, E)
{
    (void)eax;
    if (ebxItem == 0) FATAL(0);
    uint32_t tgt = call(env->o10964, rd32(H(ediPlayer), 0x60));
    if (tgt == 0) return 0;
    uint32_t out = 0;
    return call(env->sub_D34B0, ebxItem, ediPlayer, rd32(H(tgt), 0xC), (uint32_t)(uintptr_t)&out);
}

// 1.13c D2Game+0xD3CB0
// Quantity distribution over the stacks of a pickup: loop (stat 0x46 of param_3 > 0), advancing
// through the player's inventory stacks (o10955/o11144/o10643/o10836); per stack either fills to
// max and sends the 0x3E stat packet (bit-packed: unit id width 8/16/32, stat 0x46 in 9 bits, new
// quantity width 8/16/32; opcode 0x3E, length byte = bits+2, 0x22-byte send), or absorbs the rest
// and finishes (D31D0, E20A0-class cleanup via 0xDFC10 / 0xD20A0). reg: EBX = remaining capacity.
// Bit-field widths follow the spilled-argument layout of the decompile.
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_D3CB0(uint32_t p1, uint32_t p2, uint32_t param3, int32_t ebx, E)
{
    (void)p1; (void)p2;
    uint32_t cur = 0; uint32_t visits = 0;
    while ((int32_t)call(env->o10973, param3, 0x46, 0) > 0) {
        ++visits;
        if (call(env->o10955) == 0) {
            if (cur == 0) return 0;
        } else if (call(env->o11144) == 0 || (cur = call(env->o10643)) == 0) {
            cur = call(env->o10836);
            if (cur == 0) return 0;
        }
        if (call(env->o11102) == 0) continue;
        int32_t q = (int32_t)call(env->o10973, param3, 0x46, 0);
        int32_t tq = (int32_t)call(env->o10973, cur, 0x46, 0);
        int32_t mx = (int32_t)call(env->o10463, cur);
        if (tq + q <= mx) {
            if (q + tq < 0) FATAL(0);
            if (call(env->o10865)) {
                int32_t d = (int32_t)call(env->o10973, param3, 0x48, 0);
                int32_t d2 = (int32_t)call(env->o10973, cur, 0x48, 0);
                if (d < d2) {
                    call(env->o10887, cur, 0x48, d, 0);
                    call(env->sub8B010, param3, clientOf(env, param3), cur, 0x48, d);
                }
            }
            call(env->o10887, cur, 0x46, q + tq, 0);
            call(env->o10887, param3, 0x46, 0, 0);
            call(env->sub8B010, param3, clientOf(env, param3), param3, 0x46, 0);
            if (call(env->o11088, param3) == 0x12 && call(env->o11088, cur) == 0x12) call(env->sub_D31D0);
            uint32_t room = call(env->o10331, param3);
            if (room != 0) {
                call(env->o10885, room, param3 ? rd32(H(param3), 0) : 6, 0);
                call(env->o10408, param3);
                call(env->o11016, param3);
            }
            if (param3) wr32(H(param3), 0xC4, rd32(H(param3), 0xC4) & 0xfffffffdu);
            call(env->sub_DFC10, param3);
            call(env->sub_D20A0, 0, param3);
            call(env->o10466, 0);
            return 1;
        }
        uint32_t fill = call(env->o10463, cur);
        if ((int32_t)fill < 0) FATAL(0);
        call(env->o10887, cur, 0x46, fill, 0);
        uint8_t pkt[0x22]; uint8_t w[64];
        std::memset(pkt, 0, sizeof pkt); std::memset(w, 0, sizeof w);
        pkt[0] = 0x3E;
        uint32_t W = (uint32_t)(uintptr_t)w;
        call(env->o10126, W, (uint32_t)(uintptr_t)(pkt + 2), 0x20);
        uint32_t id = cur ? rd32(H(cur), 0xC) : 0xFFFFFFFFu;
        if (cur != 0 && id <= 0xFF) { call(env->o10128, W, 0, 1); call(env->o10128, W, id, 8); }
        else {
            call(env->o10128, W, 1, 1);
            if (id < 0x10000) { call(env->o10128, W, 0, 1); call(env->o10128, W, id, 16); }
            else { call(env->o10128, W, 1, 1); call(env->o10128, W, id, 32); }
        }
        call(env->o10128, W, 1, 1);
        call(env->o10128, W, 0x46, 9);
        if (fill < 0x100) { call(env->o10128, W, 0, 1); call(env->o10128, W, fill, 8); }
        else {
            call(env->o10128, W, 1, 1);
            if (fill < 0x10000) { call(env->o10128, W, 0, 1); call(env->o10128, W, fill, 16); }
            else { call(env->o10128, W, 1, 1); call(env->o10128, W, fill, 32); }
        }
        call(env->o10128, W, 0, 1);
        call(env->o10128, W, 0, 8);
        pkt[1] = (uint8_t)(call(env->o10127, W) + 2);
        call(env->sub8A3E0, (uint32_t)(uintptr_t)pkt, 0x22);
        if (ebx - (int32_t)fill < 0) FATAL(0);
        call(env->o10887, param3, 0x46, ebx - fill, 0);
        if (call(env->o11088, param3) == 0x12 && call(env->o11088, cur) == 0x12) {
            call(env->o10463, cur);
            call(env->sub_D31D0);
        }
    }
    if (visits != 0) FATAL(0);
    return 0;
}

// 1.13c D2Game+0xD4360
// Dispatch on the item kind: param_1 = item (non-null, else fatal 0x660); reg: EAX = game,
// ESI = player. Ordinal_10744 true -> 0xD3C40, second 10744 true -> 0xD3A10, else a stackable
// (10048 -> here o10955 pair) goes to 0xD3CB0.
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_D4360(uint32_t item, uint32_t eaxGame, uint32_t esiPlayer, E)
{
    if (item == 0) FATAL(0x660);
    if (call(env->o10744)) return d2_fn_d2game_D3C40(0, item, esiPlayer, env);
    if (call(env->o10744)) return d2_fn_d2game_D3A10(item, eaxGame, esiPlayer, env);
    if (call(env->o10955) /* 10048/10955 pair */ ) return d2_fn_d2game_D3CB0(item, eaxGame, esiPlayer, 0, env);
    return 0;
}

// 1.13c D2Game+0xD5C30
// Cursor<->body-location swap of the item with id EAX held by the game's unit hash
// (game+0x1720 + (id&0x7f)*4, chain link +0xE4, id at +0x0C, type +0 == 4, mode +0x10 == 4).
// param_1 = game, param_2 = player, param_3 = body location, param_4 = out error flag.
// Returns 1 on success; the unit is moved to the cursor and the previous occupant (o11139) swapped.
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_D5C30(uint32_t eaxId, uint32_t game, uint32_t player, uint8_t bodyLoc, uint32_t* outErr, E)
{
    if (game == 0) FATAL(0xd39);
    if (player == 0) FATAL(0xd3a);
    *outErr = 0;
    uint32_t it = rd32(H(game), 0x1720 + (eaxId & 0x7f) * 4);
    while (it != 0 && rd32(H(it), 0xC) != eaxId) it = rd32(H(it), 0xE4);
    if (it == 0) return 0;
    if (rd32(H(it), 0) != 4 || rd32(H(it), 0x10) != 4) return 0;
    if (call(env->o10946, player, bodyLoc, it, 0) != 7) return 0;
    if (call(env->o10244, it, player, 0, 0, 0, 0) == 0) return 0;
    call(env->sub_D29F0, player);
    call(env->o10584, player, 0, rd32(H(game), 0x70));
    uint32_t pd = rd32(H(player), 0x60);
    uint32_t slot = bodyLoc == 5 ? 4 : 5;
    uint32_t other = call(env->o11139, pd, slot);
    if (call(env->o10489, pd, it, slot) == 0) { *outErr = 1; return 0; }
    if (other == 0) FATAL(0xd74);
    if (rd32(H(other), 0x10) == 1) {
        call(env->o10013, player, other);
        call(env->o10694, pd, other);
        uint32_t o2 = call(env->o10646, pd, other);
        if (o2 == 0) FATAL(0xd7e);
        if (other != o2) FATAL(0xd7f);
        call(env->o10608, pd);
        call(env->sub_D46B0, player, 0);
        if (call(env->o10133, pd, o2, pd, 0, 0, 0xd87)) {
            if (call(env->o10405, pd, o2, 1) == 0) { *outErr = 1; return 0; }
            call(env->o10012, o2, 0);
            wr32(H(o2), 0xC4, rd32(H(o2), 0xC4) & 0xfffffffdu);
            call(env->o10466, pd, 0);
            call(env->o11090, o2, 0);
            if (call(env->sub1BC0_D1BC0)) call(env->o10977, o2, 1, 1);
            if (call(env->o10202, o2, 0x4000, 0xd91)) call(env->o10977, o2, 0x4000, 0);
            call(env->o10892, o2, 0x4000, 1);
            call(env->o10977, o2, 1, 1);
            call(env->o10050, pd, o2);
        }
    }
    uint32_t cur = call(env->o11139, pd);
    if (cur == 0) FATAL(0xda0);
    if (rd32(H(cur), 0x10) != 1) { *outErr = 1; return 0; }
    call(env->sub_D46B0, slot, 0);
    if (call(env->o10244, it, player, 0, 0, 0, 0) == 0) {
        call(env->sub_D47C0);
        call(env->o10466, pd, it);
        call(env->sub_D1650);
        call(env->sub_DFFD0, 0x14, 0);
        call(env->o10201, it, 1);
        return 0;
    }
    call(env->o10013, player, cur);
    call(env->o10694, pd, cur);
    uint32_t c2 = call(env->o10646, pd, cur);
    if (c2 == 0) FATAL(0xdb7);
    if (cur != c2) FATAL(0xdb8);
    call(env->o10608, pd);
    call(env->o10466, pd, cur);
    wr32(H(cur), 0xC4, rd32(H(cur), 0xC4) & 0xfffffffdu);
    call(env->o11090, cur, 4);
    call(env->o10892, cur, 0x10, 1);
    call(env->o10977, cur, 1, 1);
    if (call(env->o10202, cur, 0x4000, 0xdbf)) call(env->o10977, cur, 0x4000, 0);
    call(env->o10050, pd, cur);
    if (call(env->o11041, pd, it) == 0) { *outErr = 1; return 0; }
    if (call(env->o10405, pd, it, 3) == 0) { *outErr = 1; return 0; }
    call(env->o10058, it, bodyLoc);
    call(env->o10138, pd, it);
    call(env->sub_D47C0);
    wr32(H(it), 0xC4, rd32(H(it), 0xC4) & 0xfffffffdu);
    call(env->o11090, it, 1);
    call(env->o10012, it, 0xff);
    call(env->o10977, it, 8, 1);
    call(env->o10892, it, 8, 1);
    call(env->o10977, it, 1, 1);
    if (call(env->sub1BC0_D1BC0)) call(env->o10977, it, 1, 1);
    if (call(env->o10202, it, 0x4000, 0xdd9)) call(env->o10977, it, 0x4000, 0);
    call(env->o10050, pd, it);
    call(env->sub_D28D0);
    call(env->sub_D51A0, 1, it);
    return 1;
}

// old D6710 (superseded by d2game_d6710.cpp g_d6710)
// reg: EAX = unit. Sets flag 0x10 (Ordinal_10977(param_1, 0x10, 1)); when param_1 is null or its
// +0x10 is 0 and Ordinal_11057(param_1) holds: 0xD47C0, 0xD51A0, Ordinal_10201.
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_d6710_old(uint32_t param1, uint32_t eaxUnit, E)
{
    call(env->o10977, param1, 0x10, 1);
    if ((param1 == 0 || rd32(H(param1), 0x10) == 0) && call(env->o11057, param1) != 0) {
        call(env->sub_D47C0);
        call(env->sub_D51A0, 0x10, eaxUnit);
        call(env->o10201);
    }
}

// 1.13c D2Game+0xD6BC0
// reg: EAX = unit, EDI = arg for 0xD6710. h = unit+0x60; when Ordinal_10202() is 0 and h != 0:
// fatal 0x1384 if Ordinal_11051 holds, else 0xD6710, 10977, optional second 10977, 10050(h), 10201,
// then 0xDFFD0(6, unit).
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_D6BC0(uint32_t eaxUnit, uint32_t ediArg, E)
{
    uint32_t h = rd32(H(eaxUnit), 0x60);
    if (call(env->o10202) != 0 || h == 0) return;
    if (call(env->o11051) != 0) FATAL(0x1384);
    d2_fn_d2game_d6710_old(ediArg, eaxUnit, env);
    call(env->o10977);
    if (call(env->o10202) != 0) call(env->o10977);
    call(env->o10050, h);
    call(env->o10201);
    call(env->sub_DFFD0, 6, eaxUnit);
}

// 1.13c D2Game+0xD8020
// Body-location move (merge of D5C30's tail for the swap-in case). this = player, param_1 = game,
// param_2 = body loc (byte), param_3 = flag, param_4 = out error; reg: EAX = unit id. Only slot
// kinds 5 -> 4 and 4 -> 5 are accepted; Ordinal_10946 must return 2.
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_d8020_old(uint32_t player, uint32_t game, uint32_t param2, int32_t param3, uint32_t* outErr, uint32_t eaxId, E)
{
    if (game == 0) FATAL(0xb7c);
    if (player == 0) FATAL(0xb7d);
    *outErr = 0;
    call(env->sub_D29F0, player);
    uint32_t it = rd32(H(game), 0x1720 + (eaxId & 0x7f) * 4);
    while (it != 0 && rd32(H(it), 0xC) != eaxId) it = rd32(H(it), 0xE4);
    if (it == 0) { *outErr = 1; return 0; }
    if (rd32(H(it), 0) != 4) return 0;
    if (rd32(H(it), 0x10) != 4) { *outErr = 1; return 0; }
    uint8_t loc = (uint8_t)param2; uint32_t slot;
    if (loc == 5) slot = 4; else if (loc == 4) slot = 5; else { *outErr = 1; return 0; }
    uint32_t pd = rd32(H(player), 0x60);
    uint32_t other = call(env->o11139, pd, slot);
    if (other == 0) { *outErr = 1; return 0; }
    call(env->o10605, player, pd, (uint32_t)(uintptr_t)&other, slot);
    if (other == 0) { *outErr = 1; return 0; }
    if (call(env->o10946, player, loc, it, param3) != 2) { *outErr = 1; return 0; }
    call(env->sub_D46B0, game, 0);
    if (param3 == 0 && call(env->o10244, it, player, 0, 0, 0, 0) == 0) {
        call(env->sub_D47C0);
        call(env->sub_DFFD0, 0x14, 0);
        return 0;
    }
    uint32_t inv = call(env->o10061, pd);
    if (inv) call(env->o11055, player, inv);
    call(env->o10013, player, other);
    call(env->o10694, pd, other);
    uint32_t o2 = call(env->o10646, pd, other);
    if (o2 == 0) FATAL(0xbc9);
    if (other != o2) FATAL(0xbca);
    call(env->o10608, pd, slot);
    if (call(env->o11088, o2) == 0x13) call(env->sub_D7E30, game, player);
    call(env->o10466, pd, other);
    wr32(H(other), 0xC4, rd32(H(other), 0xC4) & 0xfffffffdu);
    call(env->o11090, other, 4);
    if (call(env->o11041, pd, it, param2) != 0) {
        if (call(env->o10405, pd, it, 3) == 0) { *outErr = 1; return 0; }
        call(env->o10058, it, param2);
        call(env->o10138, pd, it);
        call(env->sub_D47C0);
        wr32(H(it), 0xC4, rd32(H(it), 0xC4) & 0xfffffffdu);
        call(env->o11090, it, 1);
        call(env->o10012, it, 0xff);
        call(env->o10892, it, 0x10000, 1);   // third arg 1 (disasm 0x6FCF8299)
        call(env->o10977, it, 1, 1);
        if (call(env->o10202, it, 0x4000, 0xbe7)) call(env->o10977, it, 0x4000, 0);
        call(env->o10050, pd, it);
        call(env->sub_D28D0);
        call(env->sub_D51A0, game, player, 0);
    }
    call(env->o10201, player, 1);
    return 1;
}

// 1.13c D2Game+0xD8600
// Swap an item to the belt/slot: this = player, param_1 = game, param_2 = slot (byte, <= 10),
// param_3 = flag, param_4 = out error; reg: EAX = unit id. Ordinal_10946 must return 5.
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_d8600_old(uint32_t player, uint32_t game, uint8_t slot, uint32_t param3, uint32_t* outErr, uint32_t eaxId, E)
{
    if (game == 0) FATAL(0xca8);
    if (player == 0) FATAL(0xca9);
    if (slot > 10) return 0;
    *outErr = 0;
    call(env->sub_D29F0, player);
    uint32_t it = rd32(H(game), 0x1720 + (eaxId & 0x7f) * 4);
    while (it != 0 && rd32(H(it), 0xC) != eaxId) it = rd32(H(it), 0xE4);
    if (it == 0) { *outErr = 0; return 0; }
    if (rd32(H(it), 0) != 4) return 0;
    if (rd32(H(it), 0x10) != 4) { *outErr = 0; return 0; }
    if (call(env->o10946, player, slot, it, param3) != 5) { *outErr = 0; return 0; }
    uint32_t pd = rd32(H(player), 0x60);
    uint32_t inv = call(env->o10061, pd);
    if (inv) call(env->o11055, player, inv);
    uint32_t other = call(env->o11139, pd, slot);
    call(env->o10605, player, pd, (uint32_t)(uintptr_t)&other, slot);   // writes through the same local as `other` (disasm 0x6FCF86F4..6FCF8704)
    if (other == 0 || rd32(H(other), 0x10) != 1) { *outErr = 1; return 0; }
    call(env->sub_D46B0, game, 0);
    if (param3 == 0 && call(env->o10244, it, player, 0, 0, 0, 0) == 0) {
        call(env->sub_D47C0);
        call(env->sub_DFFD0, 0x14, 0);
        return 0;
    }
    if (call(env->o11088, it) == 0x13) call(env->sub_D7E30, game, player);
    call(env->o10013, player, other);
    call(env->o10694, pd, other);
    uint32_t o2 = call(env->o10646, pd, other);
    if (o2 == 0) FATAL(0xcef);
    if (other != o2) FATAL(0xcf0);
    call(env->o10608, pd, slot);
    call(env->o10466, pd, other);
    wr32(H(other), 0xC4, rd32(H(other), 0xC4) & 0xfffffffdu);
    call(env->o11090, other, 4);
    call(env->o10977, other, 0x80, 1);
    call(env->o10892, other, 0x20, 1);
    call(env->o10977, other, 1, 1);
    if (call(env->o10202, other, 0x4000, 0xcf8)) call(env->o10977, other, 0x4000, 0);
    call(env->o10050, pd, other);
    if (call(env->o11041, pd, it, slot) == 0) { *outErr = 1; return 0; }
    if (call(env->o10405, pd, it, 3) == 0) { *outErr = 1; return 0; }
    call(env->o10058, it, slot);
    call(env->o10138, pd, it);
    call(env->sub_D47C0);
    wr32(H(it), 0xC4, rd32(H(it), 0xC4) & 0xfffffffdu);
    call(env->o11090, it, 1);
    call(env->o10012, it, 0xff);
    call(env->o10977, it, 0x40, 1);
    call(env->o10977, it, 1, 1);
    call(env->o10892, it, 0x20, 1);
    call(env->o10977, it, 1, 1);
    if (call(env->o10202, it, 0x4000, 0xd13)) call(env->o10977, it, 0x4000, 0);
    call(env->o10050, pd, it);
    call(env->sub_D28D0);
    call(env->sub_D51A0, game, player, 0);
    return 1;
}
