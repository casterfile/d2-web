// 1.13c D2Client+0x44110 probe. r[0..3] = st, lg, gp, cl (blobs r[14], r[16], r[18], r[20]). st (152 bytes) holds the globals the
// port reads: st[0] 11C3B8, st[1] 1154E4, st[2] 11BF68, st[3] 11BF60, st[4] 11BF64, st[5] 11C3BC, st[6..37] the 32 dwords at 1197E0.
// gp is the dword at 11C3C4 (the replay patches it to the cl blob, which the port reaches through it). Every callee answers 0.
#include <cstdint>
#include <emscripten.h>
#include "../d2client/client_host.h"
#include "hookenv.h"
#include "blobtag.h"
extern "C" void d2c_44110(D2cHost h, uint32_t* g3b8, uint32_t* g54e4, uint32_t* g3c4, uint32_t gbf68, uint32_t gbf60, uint32_t gbf64, uint32_t* g97e0, uint32_t g3bc);
namespace {
HookEnv* g_env;
const uint32_t* g_r;
uint32_t host(D2cCall* c) {
    BlobTag T{g_r};
    return g_env->h(c->rva, T(c->eax), T(c->ecx), T(c->edx), T(c->ebx), T(c->esi), T(c->s[0]), T(c->s[1]), T(c->s[2]), T(c->s[3]));
}
const uint32_t kNone[] = { 0, 0 };
}
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2client_44110(uint32_t* r) {
    uint32_t* st = (uint32_t*)(uintptr_t)r[0];
    uint32_t* lg = (uint32_t*)(uintptr_t)r[1];
    g_r = r;
    HookEnv env(lg, kNone);
    env.cap = 25;   // 1024-byte log: 4 + 40 * 25
    g_env = &env;
    d2c_44110(host, &st[0], &st[1], (uint32_t*)(uintptr_t)r[2], st[2], st[3], st[4], &st[6], st[5]);
    r[4] = 0;
}
