// 1.13c D2Client+0x45cc0 probe. r[0] = n (EAX), r[1] = tbl (blob t, r[14], 60 bytes: three 0x14-byte entries {key, a, b, want,
// callback}), r[2] = key, r[3] = p. Blob lg (r[16], 512 bytes) is the hook log. A table callback is a module address (0x6FAF5D90);
// the host maps it to its rva before the call is logged.
#include <cstdint>
#include <emscripten.h>
#include "../d2client/client_host.h"
#include "hookenv.h"
extern "C" uint32_t d2_client_45cc0(int32_t n, const uint8_t* tbl, uint32_t key, uint32_t p, D2cHost host);
namespace {
HookEnv* g_env;
uint32_t host(D2cCall* c) {
    const uint32_t rva = c->rva >= 0x6FAB0000u ? c->rva - 0x6FAB0000u : c->rva;
    return g_env->h(rva, c->eax, c->ecx, c->edx, c->ebx, c->esi, c->s[0], c->s[1], c->s[2], c->s[3]);
}
const uint32_t kNone[] = { 0, 0 };
// per p: the C58C queue (the entry's hit result) and the callback queue (its EAX)
const uint32_t kP201[] = { 0xC58C, 0, 0xC58C, 1, 0x45D90, 7, 0x45D90, 9, 0, 0 };
const uint32_t kP202[] = { 0xC58C, 1, 0xC58C, 0, 0, 0 };
const uint32_t kP204[] = { 0xC58C, 1, 0x45D90, 0, 0, 0 };
const uint32_t* pick(uint32_t p) {
    switch (p) {
    case 0x201: return kP201;
    case 0x202: return kP202;
    case 0x204: return kP204;
    default: return kNone;
    }
}
}
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2client_45cc0(uint32_t* r) {
    const uint8_t* tbl = (const uint8_t*)(uintptr_t)r[1];
    uint32_t* lg = (uint32_t*)(uintptr_t)r[16];
    HookEnv env(lg, pick(r[3]));
    env.cap = 12;
    g_env = &env;
    r[4] = d2_client_45cc0((int32_t)r[0], tbl, r[2], r[3], host);
}
