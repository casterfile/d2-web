// src/core/d2game/d2game_batch_cc750.cpp -- twelve 1.13c D2Game.dll functions transcribed from the
// cached Ghidra decompiles (build-wasm/ghidra-d2game/decomp/0x<RVA>.c). Base 0x6FC20000.
// Same conventions as d2game_batch_d1890.cpp: opaque records read at retail offsets, register-only
// args (in_EAX / unaff_*) explicit, unmapped callees are seams on D2GameBatchCc75Env.
// Fatal path (Ordinal 10265 -> 10024 -> _exit(-1)) modelled as the exit.
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <emscripten.h>

static inline uint8_t  c7rd8 (const void* p, uint32_t o) { return ((const uint8_t*)p)[o]; }
static inline uint32_t c7rd32(const void* p, uint32_t o) { uint32_t v; std::memcpy(&v, (const uint8_t*)p + o, 4); return v; }
static inline void*    c7rdp (const void* p, uint32_t o) { void* v; std::memcpy(&v, (const uint8_t*)p + o, sizeof v); return v; }
static inline void     c7wr16(void* p, uint32_t o, uint16_t v) { std::memcpy((uint8_t*)p + o, &v, 2); }
static inline void     c7wr32(void* p, uint32_t o, uint32_t v) { std::memcpy((uint8_t*)p + o, &v, 4); }
[[noreturn]] static void c7fatal() { _exit(-1); }

struct D2GameBatchCc75Env {
    int      (*ord10920)(uint32_t a);              // Ordinal_10920(param_1)
    void*    (*ord10331)(const void* u);
    void*    (*ord10826)(void* v);
    int8_t   (*ord10459)(void* v);
    uint16_t (*ord10867)(const void* v);
    uint16_t (*ord10750)(const void* v);
    int      (*ord10391)(const void* a, const void* b);
    void*    (*ord11017)(const void* a);
    uint8_t  (*ord11027)(const void* u);
    void*    (*sub0xE03A0)(void* game, uint32_t id);   // FUN_6fd003a0
    int      (*sub0x2E150)(const void* u);             // FUN_6fc2e150
    uint32_t (*sub0xCB690)(uint16_t a, uint16_t b);    // FUN_6fceb690
    void     (*sub0x21420)();
    void     (*sub0x21290)(void* unit, void* stackBuf);
    void     (*sub0xDFFD0)(uint32_t a, uint32_t b);
    void     (*sub0x8A3E0)(const void* pkt, uint32_t len);
    void*    (*sub0x4AD10)(uint32_t a);
    int      (*sub0xF02A0)(uint32_t flag, const void* esi, const void* edi);   // FUN_6fd102a0: stack word flag, ESI/EDI units
    void     (*sub0x8B470)(uint32_t eax, void* game);                           // FUN_6fcab470: EAX = packet word 0, stack game
    // CD790 seams, in call order.
    void*    (*ord10331b)(const void* u);
    int      (*sub0x21280)();
    int      (*ord10494)(const void* u, uint32_t id);
    void     (*cd790pre)(void* unit);                  // 11083,10471,11058
    void     (*sub0xE07A0)(void* unit, uint32_t ecx, uint32_t edi);   // FUN_6fd007a0: EAX = unit, ECX = param_1, EDI = in_EAX
    void     (*cd790mid)(void* unit);                  // 10392,10983,10764
    void     (*ord11164)(void* u, uint32_t v);
    void     (*ord10647)(void* u, uint32_t v);
    void     (*ord10168)(void* u, uint32_t v);
    void     (*ord10286)(void* u, void* v);
    int      (*ord10334)(void* u, const void* t, uint32_t z);
    void*    (*ord10976)(void* u);
    int      (*ord11001)(void* u);
    void     (*ord10336)(void* u, void* v);
    // CDF30 seams.
    int      (*ord10747)();
    uint32_t (*sub0xCB1F0)(uint32_t eax);
    int      (*ord10787)(const void* a, uint32_t b);
    const uint8_t* (*ord10858)();
    uint32_t (*ord10708)(uint32_t a);
    int      (*ord10952)(const void* a, uint32_t b);
    void     (*sub0x78550)(void* edi, const void* p, uint32_t eax, int v, const void* p2, void* esi, uint32_t z);
    void     (*sub0xCBED0)();
    void     (*sub0x78430)(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e);
    const uint8_t* gdwBitMasks;
    const uint8_t* clientTable;                        // DAT_6fd22ce4 (0xC bytes per entry)
    // CD790, from the asm (the decompile hides the saved getter results): appended so the other probes' copies of this struct stay valid.
    uint32_t (*dAB56)(void* d);                        // getter of the 0x409 slot (ord 11164's old value)
    uint32_t (*dAB5C)(void* d);
    uint8_t  (*dAB4A)(void* d);
    uint32_t (*dA598)(void* d);
    uint32_t (*dAAC6)(void* d);
    uint32_t (*dAAD8)(void* d);
    void     (*dAB2C)(void* d, uint32_t a, uint32_t b);
};

static void* c7chain(const void* base, uint32_t off, uint32_t id) {
    void* n = c7rdp(base, off + (id & 0x7f) * sizeof(void*));
    while (n && c7rd32(n, 0xc) != id) n = c7rdp(n, 0xe4);
    return n;
}

// 1.13c D2Game+0xCC750
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_CC750(void* game, uint32_t unitId, uint32_t param_1,
                                                            const D2GameBatchCc75Env* env)
{
    if (!game) c7fatal();
    void* u = c7chain(game, 0x1320, unitId);
    if (!u) return 1;
    int p = env->ord10920(param_1);
    void* other = nullptr;
    if (p && 7 < (int)c7rd32(env->gdwBitMasks /*sgptDataTables*/, 0xbf0) && c7rdp((void*)(intptr_t)p, 0x44)) {
        uint8_t* node = (uint8_t*)c7rdp(c7rdp((void*)(intptr_t)p, 0x44), 0x54);
        for (; node; node = (uint8_t*)c7rdp(node, 0x14)) {
            if ((node[0] & 1) == 0) {
                other = c7chain(game, 0x1320, c7rd32(node, 4));
                if (other && c7rd32(other, 0xc) == c7rd32(node, 4) && other == u) {
                    uint8_t buf[0x20];
                    env->sub0x21420();
                    env->sub0x21290(u, buf);
                    env->sub0xDFFD0(0xf, param_1);
                    return 0;
                }
                return 1;
            }
        }
    }
    return 1;
}

// 1.13c D2Game+0xCCB10
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_CCB10(void* thisp, void* game, const void* param_2,
                                                            const D2GameBatchCc75Env* env)
{
    if (c7rd8(thisp, 0x6c) != 0 && c7rd32(thisp, 0x68) == 0) {
        void* r = env->sub0xE03A0(game, c7rd32(thisp, 100));
        if (r && r != param_2) return 1;
    }
    return 0;
}

// 1.13c D2Game+0xCCDB0  (this = unit id, in_EAX = game)
extern "C" EMSCRIPTEN_KEEPALIVE bool d2_fn_d2game_CCDB0(uint32_t thisId, void* game, const void* param_1,
                                                        const D2GameBatchCc75Env* env)
{
    if (!game) c7fatal();
    void* n = c7chain(game, 0x1720, thisId);
    if (!n) return true;
    if (c7rd32(n, 0x10) != 2) return true;
    return env->ord10391(c7rdp(param_1, 0x60), n) == 0;
}

// 1.13c D2Game+0xCCE20  (param_1 = unit id, in_EAX = game, unaff_EDI = record)
extern "C" EMSCRIPTEN_KEEPALIVE bool d2_fn_d2game_CCE20(uint32_t id, void* game, const void* edi,
                                                        const D2GameBatchCc75Env* env)
{
    if (!game) c7fatal();
    void* n = c7chain(game, 0x1720, id);
    if (!n) return true;
    if (env->ord10391(c7rdp(edi, 0x60), n) == 0)
        return env->ord11017(c7rdp(edi, 0x60)) != n;
    return false;
}

// 1.13c D2Game+0xCCFE0  (in_EAX = mode, param_1 = game, param_2 = id, param_3 = unit)
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_CCFE0(int mode, void* game, uint32_t id, void* unit,
                                                            const D2GameBatchCc75Env* env)
{
    int32_t* o = (int32_t*)env->sub0xE03A0(game, id);
    if (!o) return 1;
    if (mode == 4 && env->sub0x2E150(unit)) return 0;
    int8_t a = env->ord10459(env->ord10826(env->ord10331(unit)));
    int8_t b = env->ord10459(env->ord10826(env->ord10331(o)));
    if (a != b) return 2;
    int t = (int)c7rd32(o, 0);
    void* d = c7rdp(o, 0x2c);
    bool typed = t == 2 || (3 < t && t < 6);
    uint16_t u3 = typed ? (uint16_t)c7rd32(d, 0xc) : (d ? env->ord10867(d) : 0);
    if (typed) return env->sub0xCB690(u3, (uint16_t)c7rd32(d, 0x10));
    return env->sub0xCB690(u3, d ? env->ord10750(d) : 0);
}

// 1.13c D2Game+0xCD190  (param_2 = unit, param_3 = client)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_CD190(uint32_t, void* unit, void* client, void* clientTbl,
                                                        uint32_t clientIdx, const D2GameBatchCc75Env* env)
{
    if (!client) c7fatal();
    void* target;
    if ((c7rd8(client, 0x3d4) & 1) == 0) {
        if (c7rd32(client, 0x174) == 0) target = nullptr;
        else {
            target = env->sub0xE03A0(c7rdp(client, 0x1a8), c7rd32(client, 0x170));
            if (!target) c7wr32(client, 0x174, 0);
        }
    } else target = c7rdp(client, 0x174);
    if (unit == target) return;
    uint8_t flag = env->ord11027(unit);
    uint8_t tbl = ((const uint8_t*)clientTbl)[clientIdx * 0xc];
    int t = (int)c7rd32(unit, 0);
    void* d = c7rdp(unit, 0x2c);
    bool typed = t == 2 || (3 < t && t < 6);
    uint16_t u3 = 0;
    if (typed) u3 = (uint16_t)c7rd32(d, 0x10); else if (d) u3 = env->ord10750(d);
    uint16_t ud = typed ? (uint16_t)c7rd32(d, 0xc) : (d ? env->ord10867(d) : 0);
    uint8_t pkt[13] = {0};
    pkt[0] = 0xd;
    c7wr32(pkt, 2, c7rd32(unit, 0xc));
    pkt[6] = tbl;
    c7wr16(pkt, 7, ud);
    c7wr16(pkt, 9, u3);
    pkt[12] = flag;
    env->sub0x8A3E0(pkt, 0xd);
}

// 1.13c D2Game+0xCD530  (param_1 = game, param_2 = packet)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_CD530(void* game, const void* pkt, const D2GameBatchCc75Env* env)
{
    if (!game) c7fatal();
    void* g = c7rdp(game, 0x1a8);
    uint32_t id = c7rd32(pkt, 8);
    if (id != 0xffffffffu) {
        if (!g) c7fatal();
        void* a = c7chain(g, 0x1120, id);
        void* b = env->sub0x4AD10(0);
        if (a && b && a != b) {
            if (env->sub0xF02A0(4, a, b)) return;
            if (env->sub0xF02A0(2, b, a)) return;
            if (d2_fn_d2game_CCB10(a, g, b, env)) return;
            if (d2_fn_d2game_CCB10(b, g, a, env)) return;
        }
    }
    env->sub0x8B470(c7rd32(pkt, 0), game);
}

// 1.13c D2Game+0xCD630  (this = packet length, in_EAX = packet, unaff_EBX = out id; param_3 = out type)
extern "C" EMSCRIPTEN_KEEPALIVE int32_t d2_fn_d2game_CD630(uint32_t len, const uint8_t* pkt, void* game, void* unit,
                                                           uint32_t* outA, uint32_t* outB,
                                                           const D2GameBatchCc75Env* env)
{
    *outA = 0; *outB = 0;
    if (len != 9) return 3;
    uint32_t a = c7rd32(pkt, 1), b = c7rd32(pkt, 5);
    if (5 < a) return 2;
    int32_t r = (int32_t)d2_fn_d2game_CCFE0((int)a, game, b, unit, env);
    if (r == 0) { *outA = a; *outB = b; }
    return r;
}

// 1.13c D2Game+0xCD790  (this = ECX compared with 0xAAAE, in_EAX = EDI compared with 0xAB32, stack: param_1, unit; ret 8)
// Rewritten from the asm: the three setters (0x409 / 0xf / 0x4d) are restored from the getters read before them, and the last call is
// 0xA5FE(d, S4) when 0xA598 returned the unit itself, else 0xAB2C(d, X5, X6). The byte getter's dword keeps stack residue above bit 7
// (the capture masks it: hookmask 0xab14 0x1f).
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2game_CD790(void* thisp, uint32_t eax, uint32_t param_1, void* unit,
                                                            const D2GameBatchCc75Env* env)
{
    env->ord10331b(unit);
    if (!env->sub0x21280()) return 0;
    if (env->ord10494(unit, 0x36) || env->ord10494(unit, 0x6c)) return 0;
    void* d = c7rdp(unit, 0x2c);
    uint32_t x1 = env->dAB56(d);
    uint32_t x2 = env->dAB5C(d);
    uint32_t x3 = env->dAB4A(d);
    env->sub0xE07A0(unit, param_1, eax);
    uint32_t y = env->dA598(c7rdp(unit, 0x2c));
    uint32_t s4 = (y == (uint32_t)(uintptr_t)unit) ? y : 0;
    uint32_t x5 = env->dAAC6(d);
    uint32_t x6 = env->dAAD8(d);
    env->ord11164(d, 0x409);
    env->ord10647(d, 0xf);
    env->ord10168(d, 0x4d);
    env->dAB2C(d, (uint32_t)(uintptr_t)thisp, eax);
    uint32_t r = 0;
    if (env->ord10334(d, unit, 0)) {
        if ((uint32_t)(uintptr_t)thisp == (uint32_t)(uintptr_t)env->ord10976(d) && eax == (uint32_t)env->ord11001(d)) r = 1;
    }
    env->ord11164(d, x1);
    env->ord10647(d, x2);
    env->ord10168(d, x3);
    if (s4) env->ord10336(d, (void*)(uintptr_t)s4);
    else env->dAB2C(d, x5, x6);
    return r;
}

// D2Game 1.13c 0xCDF30: the retail-layout port is src/core/d2game/d2game_cdf30.cpp (EAX = skill, ECX = unit, three stack words).

// 1.13c D2Game+0xCE000
extern "C" EMSCRIPTEN_KEEPALIVE int32_t d2_fn_d2game_CE000(void* game, void* unit, uint32_t p3, uint32_t len,
                                                           const uint8_t* pkt, const D2GameBatchCc75Env* env)
{
    uint32_t b;
    int32_t r = d2_fn_d2game_CD630(len, pkt, game, unit, &p3, &b, env);
    if (r == 0) { env->sub0x78430(0, 3, p3, len, 0); r = 0; }
    return r;
}

// 1.13c D2Game+0xCE090
extern "C" EMSCRIPTEN_KEEPALIVE int32_t d2_fn_d2game_CE090(void* game, void* unit, uint32_t p3, uint32_t len,
                                                           const uint8_t* pkt, const D2GameBatchCc75Env* env)
{
    uint32_t b;
    int32_t r = d2_fn_d2game_CD630(len, pkt, game, unit, &p3, &b, env);
    if (r == 0) { env->sub0x78430(0, 2, p3, len, 0); r = 0; }
    return r;
}
