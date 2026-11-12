// 1.13c D2Client+0x45e50 probe. r[0] = the id (the port ignores it). Blob g (r[14], 4 bytes) is the dword at 0x11C284, blob b
// (r[16], 4 bytes) the byte at 0x11C29C, blob lg (r[18], 256 bytes) the hook log. The module strings the callee takes are module
// addresses: the host tags them with their rva (0xB2000000 | addr - 0x6FAB0000) before logging.
#include <cstdint>
#include <emscripten.h>
#include "../d2client/client_host.h"
#include "hookenv.h"
extern "C" void d2_client_45e50(uint32_t* gc284, uint8_t* gc29c, D2cHost host);
namespace {
HookEnv* g_env;
uint32_t tagMod(uint32_t v) { return v >= 0x6FAB0000u && v < 0x6FB90000u ? (0xB2000000u | (v - 0x6FAB0000u)) : v; }
uint32_t host(D2cCall* c) {
    if (c->rva == 0xBE8A)   // Ordinal_423: two module strings, then the out pointer (a stack address in the port's frame)
        return g_env->h(c->rva, c->eax, c->ecx, c->edx, c->ebx, c->esi, tagMod(c->s[0]), tagMod(c->s[1]), c->s[2], (uint32_t)(uintptr_t)c->out);
    return g_env->h(c->rva, c->eax, c->ecx, c->edx, c->ebx, c->esi, c->s[0], c->s[1], c->s[2], c->s[3]);
}
const uint32_t kNone[] = { 0, 0 };
}
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2client_45e50(uint32_t* r) {
    uint32_t* g = (uint32_t*)(uintptr_t)r[14];
    uint8_t* b = (uint8_t*)(uintptr_t)r[16];
    uint32_t* lg = (uint32_t*)(uintptr_t)r[18];
    HookEnv env(lg, kNone);
    env.cap = 6;
    g_env = &env;
    d2_client_45e50(g, b, host);
    r[4] = 0;
}
