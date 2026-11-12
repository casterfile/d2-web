// D2Client+0x9FB00: env-seam style. Units are opaque handles; fields via unit_dword(u, dwordIndex).
// Retail registers and stack words (disassembly): 0x1E2C0 takes EAX = -1 and the stack words (unit, buf72, &outW); 0xC0D0 (Ordinal_10005)
// the stack words (buf72, 0, 1); 0x1E930 (unit, 0, 0, &outW, &outH); 0xC43C (Ordinal_11090) (unit, 0|4); 0x1B5A0 (x, y).
#include <cstdint>
#include <emscripten.h>
struct D2c9FB00Env {
    const uint32_t* gameState;                // DAT_6fbbb214 (0 = fatal); read again after 0x9F460
    uint32_t playerId;                        // DAT_6fbbb220
    uint32_t curPanel;                        // DAT_6fbcbbfc (0 = none)
    uint32_t curPanelKind;                    // *(DAT_6fbcbbfc+0x10)
    void (*fatal)();                          // Ordinal_10265 + 10024 + _exit(-1) (does not return)
    void (*fatalBadUnit)();                   // same with string DAT_6fb8359f
    void (*f9F9E0)();
    void (*f9F460)();
    uint32_t (*unit_head)(uint32_t bucket);   // DAT_6fbbae08[bucket]
    uint32_t (*unit_dword)(uint32_t u, uint32_t idx);  // idx 0 = type, 3 = id, 0x39 = next
    int32_t (*f1E2C0)(uint32_t u, int32_t* buf72, uint32_t* outW);     // EAX = -1; buf is 0x48 bytes, byte 5 = 1
    int32_t (*ord10005)(int32_t* buf72, int32_t zero, int32_t one);    // Ordinal_10005 (buf72, 0, 1)
    void (*f1E930)(uint32_t u, int32_t a, int32_t b, uint32_t* outW, uint32_t* outH);
    void (*ord11090)(uint32_t u, int32_t mode);   // Ordinal_11090(u, 0) then (u, 4); the first result is unused
    void (*f1B5A0)(int32_t x, int32_t y);
};
extern "C" {
// 1.13c D2Client+0x9FB00
EMSCRIPTEN_KEEPALIVE void d2c_9fb00(const D2c9FB00Env* e) {
    if (*e->gameState == 0) { e->fatal(); return; }
    if (e->curPanel == 0 || e->curPanelKind == 0x11) { e->f9F9E0(); return; }
    e->f9F460();
    if (*e->gameState != 2) return;
    for (uint32_t u = e->unit_head(e->playerId & 0x7f); u; u = e->unit_dword(u, 0x39)) {
        if (e->unit_dword(u, 3) != e->playerId) continue;
        if (e->unit_dword(u, 0) != 4) { e->fatalBadUnit(); return; }
        int32_t buf[0x12] = {0};
        reinterpret_cast<uint8_t*>(buf)[5] = 1;
        uint32_t w = 0, h = 0;
        if (!e->f1E2C0(u, buf, &w)) return;
        if (!e->ord10005(buf, 0, 1)) return;
        if (buf[15] == 0) return;
        e->f1E930(u, 0, 0, &w, &h);
        e->ord11090(u, 0);
        const uint32_t y = h < 0x70 ? (0x72 - h) >> 1 : 0;
        e->f1B5A0(int32_t(0xa7 - (w >> 1)), int32_t(y + 0x6a));
        e->ord11090(u, 4);
        return;
    }
}
}
