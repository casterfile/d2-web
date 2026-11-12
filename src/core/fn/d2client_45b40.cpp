// 1.13c D2Client+0x45b40 probe. r[0] = unit (blob u, r[14], 16 bytes; its first dword is the key), r[1] = ECX (the out slot of
// 0xCE20, which the port reads back). Blob lg (r[16], 512 bytes) is the hook log. Each callee answers the key's row: the
// C1C6 queue holds the stat-0x48 answer then the stat-0x46 answer.
#include <cstdint>
#include <emscripten.h>
#include "../d2client/client_host.h"
#include "hookenv.h"
#include "blobtag.h"
extern "C" uint32_t d2_client_45b40(uint32_t unit, uint32_t ecx, D2cHost host);
namespace {
HookEnv* g_env;
const uint32_t* g_r;
uint32_t host(D2cCall* c) {
    BlobTag T{g_r};
    return g_env->h(c->rva, c->eax, c->ecx, c->edx, c->ebx, c->esi, T(c->s[0]), c->s[1], c->s[2], c->s[3]);
}
struct Row { uint32_t c220, ce14, c8bc, c916, c73c, c91c, c1a, c1b, ce20; };
const Row kRow[10] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 4, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 10, 0, 0, 5, 5, 0 },
    { 0, 1, 1, 10, 1, 5, 10, 3, 0 },
    { 0, 1, 1, 0, 1, 7, 7, 7, 1 },
    { 0, 1, 0, 0, 0, 0, 0, 0, 1 },
    { 0, 1, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 1, 1, 0xFFFFFFFFu, 0, 0, 0xFFFFFFFBu, 0xFFFFFFFBu, 0 },
    { 0, 1, 1, 3, 1, 2, 3, 2, 1 },
};
}
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2client_45b40(uint32_t* r) {
    const uint32_t unit = r[0];
    const uint32_t key = *(const uint32_t*)(uintptr_t)unit;
    const Row& w = kRow[key < 10 ? key : 0];
    uint32_t sc[] = { 0xC220, w.c220, 0xCE14, w.ce14, 0xC8BC, w.c8bc, 0xC916, w.c916, 0xC73C, w.c73c,
                      0xC91C, w.c91c, 0xC1C6, w.c1a, 0xC1C6, w.c1b, 0xCE20, w.ce20, 0, 0 };
    uint32_t* lg = (uint32_t*)(uintptr_t)r[16];
    g_r = r;
    HookEnv env(lg, sc);
    env.cap = 12;
    g_env = &env;
    r[4] = d2_client_45b40(unit, r[1], host);
}
