// 1.13c D2Client+0x44880 probe. r[0] = st (44 bytes, r[14]), r[1] = lg (128 bytes, r[16]), r[2] = ed (16 bytes, r[18]).
// st dwords: 0 11C6C4, 1 11C3A0, 2 11C3DC, 3 11C3E4, 4 11C3F8, 5 11C3E8, 6 11C3EC, 7 11C3F0, 8 the pointer global 11BBFC (the ed blob
// in the patched tuples; the port reads through it only when its conditions hold), 9 11B828, 10 11B824. Callee 0x1BF20 answers 0.
#include <cstdint>
#include <emscripten.h>
#include "../d2client/client_host.h"
#include "hookenv.h"
#include "blobtag.h"
extern "C" void d2c_44880(D2cHost h, uint32_t* g46c4, uint32_t g3a0, uint32_t g3dc, uint32_t g3e4, uint32_t* g3f8, uint32_t g3e8, uint32_t g3ec, uint32_t g3f0, const uint32_t* edx, uint32_t gb828, uint32_t gb824);
namespace {
HookEnv* g_env;
const uint32_t* g_r;
uint32_t host(D2cCall* c) {
    BlobTag T{g_r};
    return g_env->h(c->rva, T(c->eax), T(c->ecx), T(c->edx), T(c->ebx), T(c->esi), T(c->s[0]), T(c->s[1]), T(c->s[2]), T(c->s[3]));
}
const uint32_t kNone[] = { 0, 0 };
}
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2client_44880(uint32_t* r) {
    uint32_t* st = (uint32_t*)(uintptr_t)r[0];
    uint32_t* lg = (uint32_t*)(uintptr_t)r[1];
    g_r = r;
    HookEnv env(lg, kNone);
    env.cap = 3;   // 128-byte log: (128 - 4) / 40
    g_env = &env;
    d2c_44880(host, &st[0], st[1], st[2], st[3], &st[4], st[5], st[6], st[7], (const uint32_t*)(uintptr_t)st[8], st[9], st[10]);
    r[4] = 0;
}
