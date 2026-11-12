// D2Client+0x9F460: buy/sell panel draw. Transcribed from the disassembly: every callee takes its retail registers and stack words
// (fastcall thunks: ECX/EDX; the cell draw and the rectangle are stdcall with six stack words).
#include <cstdint>
#include <cstring>
#include <emscripten.h>
struct D2c9F460Env {
    uint32_t (*ord10184)(uint32_t ecx);                                                     // 0xD3A8: ECX = 1, then the first result
    uint32_t (*ord10003)(uint32_t ecx);                                                     // 0xD1EC: string by id
    int32_t (*ord10028)(uint32_t ecx);                                                      // 0xD35A: text width
    int32_t (*ord10083)();                                                                  // 0xD38A: text height
    void (*ord10014)(int32_t l, int32_t t, int32_t r, int32_t b, int32_t c, int32_t m);     // 0xD0BA
    void (*ord10150)(uint32_t ecx, int32_t edx, int32_t y, int32_t a, int32_t b);           // 0xD37E: draw text
    uint32_t (*ord10059)(uint32_t ecx, uint32_t* edxSlot, int32_t id);                      // 0xD378
    void (*ord10043)(uint32_t ecx, uint32_t edx, int32_t id, int32_t z);                    // 0xBF86
    void (*cellDraw)(const int32_t* cd, int32_t x, int32_t y, int32_t a, int32_t b, int32_t c);  // 0xD096
    void (*wsprintfPath)(char* buf, const char* dir);                                       // user32 wsprintfA("%s\\ui\\panel\\buysellbtn", "DATA\\GLOBAL")
    uint32_t (*loadCel)(const char* path);                                                  // 0x2B420: EAX = path, one stack word 0
    void (*f3840)(uint32_t esi);                                                            // 0xA3840: ESI = the flag
    uint32_t (*rd32)(uint32_t addr);
};
// The globals, as retail memory: mode 0x10B210, cel0 0x11BC10, cel1 0x11BC0C, btnCel 0x11B9C4, tab = 0xDEC62 (stride 40; the dword at tab+0x4E is the
// button count 0xDECB0, the word at tab-2 is read for entry 0), mouse y 0x11B828 / x 0x11B824, screen width 0xDBC4C, language word 0x119725, flag 0x10B224.
struct D2c9F460G {
    int32_t mode; uint32_t cel0, cel1, btnCel; const uint8_t* tab; int32_t mouseY, mouseX, screenW; uint32_t lang, flag224;
};
extern "C" {
EMSCRIPTEN_KEEPALIVE void d2c_9f460(const D2c9F460Env* e, D2c9F460G* g) {
    auto rd = [](const uint8_t* p) { int32_t v; memcpy(&v, p, 4); return v; };
    auto count = [&]() { return rd(g->tab + 0x4e); };
    uint32_t slot = e->ord10184(1);
    const int32_t mode = g->mode;
    if (mode == 1 || mode == 0) {
        int32_t cd[18]; memset(cd, 0, sizeof cd);
        cd[13] = (int32_t)(mode == 0 ? g->cel0 : g->cel1);
        e->cellDraw(cd, 0x69, 0x10e, -1, 5, 0);
        if (g->btnCel == 0) {
            char path[260]; e->wsprintfPath(path, "DATA\\GLOBAL");
            g->btnCel = e->loadCel(path);
        }
        cd[13] = (int32_t)g->btnCel;
        int32_t n = count();
        for (int32_t i = 0; i < n; i++) {
            const uint8_t* b = g->tab + i * 40;
            cd[0] = rd(b + 0x20) + (b[0x24] != 0);
            e->cellDraw(cd, rd(b + 8), rd(b + 0xc), -1, 5, 0);
            n = count();
        }
        const int32_t my = g->mouseY, mx = g->mouseX;
        if (my < 0x140 && mx < g->screenW - 0x30 && n > 0) {
            for (int32_t i = 0; i < n; i++) {
                const uint8_t* b = g->tab + i * 40;
                if (my >= rd(b + 0x10) && my <= rd(b + 0x14) && mx >= rd(b + 0x18) && mx <= rd(b + 0x1c)) {
                    uint16_t wd; memcpy(&wd, b - 2, 2);
                    uint32_t id = ((uint32_t)my & 0xffff0000u) | wd;
                    if (wd == 0xfb1) {
                        if (mode == 0) id = 0xd49;
                        if (g->lang == 0x1ff) id = 0x58dc; else if (g->lang == 0x200) id = 0x58dd;
                    }
                    const uint32_t str = e->ord10003(id);
                    const int32_t w = e->ord10028(str);
                    const int32_t h1 = e->ord10083() + 6;
                    const int32_t h2 = e->ord10083();
                    const int32_t x0 = rd(b), y0 = rd(b + 4);
                    const int32_t t = y0 - h2 - 0x2a;
                    const int32_t hw = w / 2;
                    const int32_t l = x0 - hw + 10;
                    e->ord10014(l, t, l + w + 0xc, t + h1, 0, 2);
                    e->ord10150(str, x0 - hw + 0x10, y0 - 0x26, 0, 0);
                    n = count();
                }
            }
        }
        if (mode == 1) {
            uint32_t id = 0x275c;
            if (g->lang == 0x1ff) id = 0x58de; else if (g->lang == 0x200) id = 0x58db;
            const uint32_t sref = e->ord10003(id);
            uint32_t p = e->ord10059(sref, &slot, 300);
            int32_t y = 0x1e;
            while (p) {
                const uint32_t s = e->rd32(p);
                const int32_t w = e->ord10028(s);
                e->ord10150(e->rd32(p), 0xa0 - w / 2, y, 4, 0);
                const uint32_t nx = e->rd32(p + 4);
                e->ord10043(e->rd32(p), 0x6fb84538, 0x2c3, 0);
                e->ord10043(p, 0x6fb84538, 0x2c4, 0);
                y += 0x14;
                p = nx;
            }
        }
    }
    if (g->flag224) e->f3840(g->flag224);
    e->ord10184(slot);
}
}
