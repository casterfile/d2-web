// 1.13c D2Client+0x45ff0 probe. r[0] = BX (the wanted id). Blob t (r[14], 32 bytes) is the 8-byte {id, value} table, its count
// at t+12. Blob h (r[16], 4 bytes) is the dword the callee takes. Blob sk (r[18], 24 bytes) is the u16 skip list, its count at
// sk+20. Blob lg (r[20], 512 bytes) is the hook log. The callee 0xCE08 answers the probe's queue for the h value.
#include <cstdint>
#include <emscripten.h>
#include "../d2client/client_host.h"
#include "hookenv.h"
extern "C" uint16_t d2_client_45ff0(uint16_t bx, const uint8_t* tbl, int32_t n, uint32_t h, const uint16_t* skip, uint16_t nskip, D2cHost host);
namespace {
HookEnv* g_env;
uint32_t host(D2cCall* c) { return g_env->h(c->rva, c->eax, c->ecx, c->edx, c->ebx, c->esi, c->s[0], c->s[1], c->s[2], c->s[3]); }
const uint32_t kNone[] = { 0, 0 };
const uint32_t k101[] = { 0xCE08, 5, 0, 0 };
const uint32_t k102[] = { 0xCE08, 5, 0xCE08, 7, 0xCE08, 9, 0xCE08, 0xB, 0, 0 };
const uint32_t k103[] = { 0xCE08, 0xFFFF, 0, 0 };
const uint32_t k104[] = { 0xCE08, 3, 0xCE08, 0xFFFF, 0, 0 };
const uint32_t* pick(uint32_t h) {
    switch (h) {
    case 0x101: return k101;
    case 0x102: return k102;
    case 0x103: return k103;
    case 0x104: return k104;
    default: return kNone;
    }
}
}
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2client_45ff0(uint32_t* r) {
    const uint8_t* t = (const uint8_t*)(uintptr_t)r[14];
    const uint32_t h = *(const uint32_t*)(uintptr_t)r[16];
    const uint16_t* sk = (const uint16_t*)(uintptr_t)r[18];
    uint32_t* lg = (uint32_t*)(uintptr_t)r[20];
    const int32_t n = *(const int32_t*)(t + 12);
    const uint16_t nskip = *(const uint16_t*)((const uint8_t*)sk + 20);
    HookEnv env(lg, pick(h));
    env.cap = 12;
    g_env = &env;
    r[4] = d2_client_45ff0((uint16_t)r[0], t, n, h, sk, nskip, host);
}
