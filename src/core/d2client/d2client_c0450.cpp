// 1.13c D2Client+0xC0450: ECX = unit (may be null), EAX = int* pct, stack flag (int*),
// dst (Unicode name buffer), n; ret 0xC. The name comes from 0x9D850 (ESI = unit). A unit whose +0x10 is 0x11 gets pct 0,
// its name widened into a local 0x80-char buffer and personalized with string 0x90B into dst (n chars, the language
// from D2Lang 10009), then flag := 1. Any other unit: pct := roster
// life (+0x14 of 0x9D630(id), 0 when the record is missing or the id is the player's at 0x11BBFC) << 7 / 100, flag :=
// 0x9DC60(player id, unit id) != 0, and the name widened into dst.
#include <cstdint>
#include <cstring>
#include <emscripten.h>

struct D2cC0450Env {
    const char* (*name)(const uint8_t* unit);                            // 0x9D850 (ESI = unit)
    uint8_t* (*roster)(uint32_t id, uint32_t* edx);                      // 0x9D630 (EDX = id); *edx = EDX on return (0x9D630 leaves it = id; the capture's hook recorder does not)
    uint32_t (*hostile)(uint32_t a, uint32_t b);                         // 0x9DC60 (EDX = a, stack b; ret 4)
    uint32_t (*lang)();                                                  // thunk 0xD1E6 (D2Lang 10009)
    const uint16_t* (*str)(uint32_t id);                                 // thunk 0xD1EC (D2Lang 10003, ECX = id)
    void (*ctor)(uint16_t* p);                                           // Unicode default constructor (IAT 0xCEB00)
    void (*win2Unicode)(uint16_t* dst, const char* src, int n);          // IAT 0xCEAC0
    void (*personalize)(uint16_t* dst, const uint16_t* name, const uint16_t* s, int n, uint32_t lang);   // IAT 0xCEAA4
    uintptr_t* player;                                                   // 0x11BBFC
};
static uint32_t rd(uintptr_t p, uint32_t o) { uint32_t v; memcpy(&v, (const void*)(p + o), 4); return v; }
extern "C" EMSCRIPTEN_KEEPALIVE void d2c_c0450(const D2cC0450Env* e, const uint8_t* unit, int32_t* pct, uint32_t* flag, uint16_t* dst, int32_t n) {
    const char* nm = e->name(unit);
    uint32_t id = 0xFFFFFFFFu;
    if (unit) {
        if (rd((uintptr_t)unit, 0x10) == 0x11) {
            *pct = 0;
            uint16_t local[0x80];
            for (int i = 0; i < 0x80; ++i) e->ctor(local + i);
            e->win2Unicode(local, nm, 0x80);
            uint32_t l = e->lang();
            const uint16_t* s = e->str(0x90b);
            e->personalize(dst, local, s, n, l);
            *flag = 1;
            return;
        }
        id = rd((uintptr_t)unit, 0xc);
    }
    uint32_t edx = id;
    uint8_t* r = e->roster(id, &edx);
    uintptr_t p = *e->player;
    uint32_t pid = p ? rd(p, 0xc) : 0xFFFFFFFFu;
    uint32_t v = (edx == pid || !r) ? 0 : rd((uintptr_t)r, 0x14);   // retail compares EDX as 0x9D630 returns it
    *pct = (int32_t)(v << 7) / 100;
    *flag = 0;
    if (e->hostile(pid, id)) *flag = 1;
    e->win2Unicode(dst, nm, 0x80);
}
