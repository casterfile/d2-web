// 1.13c D2Client+0x45fb0 probe. r[0] = ECX (the payload dword). Blob st (r[14], 8 bytes): st[0] is the key dword,
// st[1] the dword the call clears. Blob lg (r[16], 1024 bytes): the hook log at +0, the send's wire log at +0x200.
// The send (0x143E0, EBX = length, stack buffer) is logged as one hook record; the wire bytes follow it in the wire log.
#include <cstdint>
#include <cstring>
#include <emscripten.h>
#include "hookenv.h"
extern "C" uint32_t d2_client_45fb0(uint32_t* g978e, uint32_t key, uint32_t ecx, uint8_t* o);
extern "C" EMSCRIPTEN_KEEPALIVE void d2_fn_d2client_45fb0(uint32_t* r) {
    uint32_t* st = (uint32_t*)(uintptr_t)r[14];
    uint32_t* lg = (uint32_t*)(uintptr_t)r[16];
    static const uint32_t sc[] = { 0, 0 };
    HookEnv env(lg, sc);
    env.cap = 12;   // the hook log ends at the wire log (+0x200): 12 records of 40 bytes past +4 fit in 0x200
    uint8_t o[16] = {};
    const uint32_t n = d2_client_45fb0(&st[1], st[0], r[0], o);
    env.h(0x143E0, 0, 0, 0, n, 0, (uint32_t)(uintptr_t)o);
    uint32_t* wl = (uint32_t*)((uint8_t*)lg + 0x200);
    wl[0] = 1; wl[1] = n;
    std::memset(wl + 2, 0, 16); std::memcpy(wl + 2, o, n);
    r[4] = 0;
}
