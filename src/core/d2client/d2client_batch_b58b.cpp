// D2Client batch b58b: callees go through D2cHost (ordinals as 0x80000000|n); globals passed by pointer (RVA noted).
#include <initializer_list>
#include <cstdint>
#include <cstring>
#include <emscripten.h>
#include "client_host.h"
typedef int32_t i32; typedef uint32_t u32;
namespace {
// one retail call by RVA: ecx = the register argument, edx = the second register, s0..s2 = stack words (pushed order 0 = lowest)
u32 call(D2cHost h, u32 rva, u32 ecx, u32 edx, u32 s0 = 0, u32 s1 = 0, u32 s2 = 0) {
    D2cCall c = {}; c.rva = rva; c.ecx = ecx; c.edx = edx; c.s[0] = s0; c.s[1] = s1; c.s[2] = s2;
    return h(&c);
}
}
struct D2cB58G { i32 *state /*B925F0*/, *mouseY /*1CB828*/, *mouseX /*1CB824*/, *gW /*1B8BC48*/, *gH /*1B8BC4C*/, *offX /*1CB9A0*/, *offY /*1CB9A4*/; };
extern "C" {
// 1.13c D2Client+0x77DE0
EMSCRIPTEN_KEEPALIVE void d2c_77de0(D2cHost h, D2cB58G* g) {
    i32 base = *g->gW - *g->offX, lo, hi, tab;
    if (*g->state == 1) { lo = base - 0x95; hi = base - 0x73; tab = base - 0x86; }
    else if (*g->state == 2) { lo = base - 0xdc; hi = base - 0xba; tab = base - 0xcd; }
    else if (*g->state == 3) { lo = base - 0x131; hi = base - 0x10f; tab = base - 0x122; }
    else return;
    if (*g->mouseY < lo || *g->mouseY > hi) return;
    i32 y = *g->offY - 0x3f + *g->gH;
    if (y - 0x22 <= *g->mouseX && *g->mouseX <= y) {
        // retail: push 1; push 0; push (offY-0x62+gH); mov ecx,0x1030; call 0xD1EC; mov ecx,eax; mov edx,esi; call 0xD384
        u32 r = call(h, 0xD1EC, 0x1030, 0, (u32)(*g->offY - 0x62 + *g->gH), 0, 1);
        call(h, 0xD384, r, (u32)tab);  // retail esi differs per tab
    }
}
}
