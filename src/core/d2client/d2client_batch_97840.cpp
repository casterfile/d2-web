// src/core/d2client/d2client_batch_97840.cpp -- twelve UNMAPPED 1.13c D2Client.dll functions
// (panel toggles 0x97A70/0x97CD0, HUD frame 0x9AE80, item/group list helpers 0x9CF10-0x9D160,
// shutdown 0x97840, object-id -> table 0x9CAD0), alpha lane, from build-wasm/ghidra-d2client/decomp.
// Image base 0x6FAB0000. Unrecovered callees and ordinals are env seams AFTER the retail arguments.
// Node/struct addresses are 32-bit retail-style words reached through P32 (the d2client_batch_908f pattern).
// Register-passed retail inputs (unaff_ESI/EDI/EBX, in_EAX) are explicit parameters here.
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <emscripten.h>

static inline uint32_t* P32(uint32_t a) { return (uint32_t*)(uintptr_t)a; }
static inline uint16_t* P16(uint32_t a) { return (uint16_t*)(uintptr_t)a; }

// --- module state (all zero-initialised) ----------------------------------------------------
static uint32_t g_hud_state;          // +0x11BC34 DAT_6fbcbc34 (0xE = pending teardown)
static uint32_t g_hud_handle;         // +0x11BC78 DAT_6fbcbc78
static uint32_t g_item_head;          // +0x11BC14 DAT_6fbcbc14 (item chain, next at +0x80)
static uint32_t g_group_head;         // +0x11BC1C DAT_6fbcbc1c (group chain: short id @0, items @4, next @8)
// Panel-open array 0x6fbaad80, 0x26 words: aad84 = [1], aad90 = [4], aae10 = [0x1C], aae14 = [0x1D].
// ponytail: stride-4 reading of DAT_6fbaad80 inferred from aad84/aad90/aae10/aae14 all falling in 0x26*4 bytes.
static uint32_t g_panel_open[0x26];   // +0x0FAD80
static uint32_t g_panel_count = 0x26; // +0x0DD5D0 DAT_6fb8d5d0
static uint32_t g_panel_tbl[0x26];    // +0x0DD538 PTR_DAT_6fb8d538 (->struct, +0x90/+0x94 = mode word)
static uint32_t g_cl_ready;           // +0x119854 DAT_6fbc9854
static uint32_t g_cl_lock;            // +0x113A68 DAT_6fbc3a68
static uint32_t g_cl_flag9721;        // +0x119721 DAT_6fbc9721 (byte)
static uint32_t g_focus;              // +0x11BBFC DAT_6fbcbbfc (->int[], [0]=flag, [3]=id)
static uint32_t g_unit_chain;         // +0x11C4D4 DAT_6fbcc4d4 (next @+0x30)
static uint32_t g_unit_hash[0x80];    // +0x10A808 DAT_6fbba808 (next @ word 0x39)
static uint32_t g_screen_w;           // +0x0DBC48 DAT_6fb8bc48
static uint32_t g_screen_h;           // +0x0F9E14 DAT_6fba9e14
static int32_t  g_view_off;           // +0x11C418
static uint32_t g_view_mode;          // +0x11C414
static uint32_t g_clr_3dc, g_clr_3e0, g_clr_3f8, g_clr_3fc;  // +0x11C3DC/3E0/3F8/3FC
static uint32_t g_ba9e18, g_cc3c4;   // +0x0F9E18 / +0x11C3C4 (the 0x8A4D0 layout call's EAX / ESI)
static uint32_t g_edi971d;          // +0x11971D (unaligned dword, EDI of the 0x47B20 call)
static uint32_t g_str_bef4;           // +0x11BEF4
static uint16_t g_str_buf[0x100];     // +0x11CC38 (Unicode string)
static uint32_t g_hud_x, g_hud_y, g_hud_cel, g_hud_alt;  // +0x11B9A0 / +0x11B9A4 / +0x11BED4 / +0x11BEF0
static int32_t  g_ui_yoff;            // +0x0DBC4C DAT_6fb8bc4c

#include "d2client_batch_97840_env.h"

// Probe state accessor: the module globals by RVA (nullptr for an RVA not held here).
extern "C" uint32_t* d2_client_97840_g(uint32_t rva) {
    switch (rva) {
    case 0x11BC34: return &g_hud_state;
    case 0x11BC78: return &g_hud_handle;
    case 0x0FAD80: return g_panel_open;
    case 0x0DD5D0: return &g_panel_count;
    case 0x0DD538: return g_panel_tbl;
    case 0x119854: return &g_cl_ready;
    case 0x113A68: return &g_cl_lock;
    case 0x119721: return &g_cl_flag9721;
    case 0x11971D: return &g_edi971d;
    case 0x11BBFC: return &g_focus;
    case 0x11C4D4: return &g_unit_chain;
    case 0x10A808: return g_unit_hash;
    case 0x0DBC48: return &g_screen_w;
    case 0x0F9E14: return &g_screen_h;
    case 0x0F9E18: return &g_ba9e18;
    case 0x11C3C4: return &g_cc3c4;
    case 0x11C418: return (uint32_t*)&g_view_off;
    case 0x11C414: return &g_view_mode;
    case 0x11C3DC: return &g_clr_3dc;
    case 0x11C3E0: return &g_clr_3e0;
    case 0x11C3F8: return &g_clr_3f8;
    case 0x11C3FC: return &g_clr_3fc;
    case 0x11BEF4: return &g_str_bef4;
    case 0x11CC38: return (uint32_t*)g_str_buf;
    default: return nullptr;
    }
}

// 1.13c D2Client+0x97840
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_97840(const D2cClientEnv* e) {
    if (g_hud_state == 0xE) { g_hud_state = 0; e->sub_97770(); }
    uint32_t h = g_hud_handle;
    if (h != 0) {
        e->ord10065(h);
        e->free10043(h, 0xD6AB0, 0x7A, 0);
        g_hud_handle = 0;
    }
}

// Tail + prelude common to both toggles. sw: index of the panel word being toggled (0x1D / 0x1C).
static bool panel_core(const D2cClientEnv* e, uint32_t sw, uint32_t modeoff, bool set_to_one,
                       uint32_t notify, uint32_t code, bool close_extra, bool gate_on_closed) {
    uint32_t old = g_panel_open[sw];
    if (!gate_on_closed || g_panel_open[sw] == 0) {
        if (g_cl_lock != 0) return false;
        for (uint32_t i = 0; (int32_t)i < (int32_t)g_panel_count; ++i) {
            if (g_panel_open[i] != 1) continue;
            switch (P32(g_panel_tbl[i] + modeoff)[0]) {
            case 0: break;
            case 1: e->sub_72790(i, 1, 0); break;
            case 4: if (g_cl_flag9721 & 0xFF) e->sub_f7b20(g_edi971d); break;
            default: return false;
            }
        }
    }
    if (g_focus == 0 || (P32(g_focus)[0] == 0 && e->sub_55440() == 0))
        g_panel_open[sw] = set_to_one ? 1 : (g_panel_open[sw] == 0);
    uint32_t nw = g_panel_open[sw];
    if (old == 0) { if (nw != 0) e->sub_eccc0(code, e->ord10048(notify)); }
    else if (nw == 0) { e->sub_ecc60(code, e->ord10048(notify)); if (close_extra) e->sub_3bbf0(); }
    int32_t q = (int32_t)g_screen_w; q = (q + ((q >> 31) & 3)) >> 2;
    int32_t h = (int32_t)g_screen_h;  int32_t hq = (h + ((h >> 31) & 3)) >> 2;
    int32_t x, off;
    if (g_panel_open[1] == 0 && g_panel_open[4] == 0) {
        if (nw == 0) { g_view_off = 0; g_view_mode = 0; e->ord10006(0); off = 0; x = h; }
        else { g_view_off = q; g_view_mode = 2; e->ord10006(q); off = hq; x = h + hq; }
    } else {
        if (nw != 0) {
            g_view_off = 0; g_view_mode = 3; e->ord10006(0); e->sub_3a4d0(g_ba9e18, h, 0, 0, g_cc3c4);
            g_clr_3dc = g_clr_3e0 = g_clr_3f8 = g_clr_3fc = 0;
            return true;
        }
        g_view_off = -q; g_view_mode = 1; e->ord10006((uint32_t)-q); off = -hq; x = h - hq;
    }
    e->sub_3a4d0(g_ba9e18, x, 0, off, g_cc3c4);
    return true;
}

static void fatal_exit(const D2cClientEnv* e, uint32_t msg) {
    e->ord10024("", e->ord10265(msg));
    e->exit_(-1);
}

// 1.13c D2Client+0x97A70 (fastcall: ecx unused, str, flag)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_97a70(uint32_t ecx, const uint16_t* str, uint32_t param3,
                                                     const D2cClientEnv* e) {
    (void)ecx;
    size_t n = 0; while (str[n] && n < 0xFF) { g_str_buf[n] = str[n]; ++n; } g_str_buf[n] = 0;
    g_str_bef4 = param3;
    if (e->ord10227() == 0 || g_cl_ready == 0) return;
    if (g_panel_count != 0x26) { fatal_exit(e, 0x139B); return; }
    panel_core(e, 0x1D, 0x94, true, 0x6FB809C0u, 2, false, false);
}

// 1.13c D2Client+0x97CD0: returns the unit at the focus id (hash 0x6fbba808) or 0 after toggling panel 0x1C
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_client_97cd0(const D2cClientEnv* e) {
    if (g_focus != 0) {
        for (uint32_t n = g_unit_chain; n != 0; n = P32(n)[0x30 / 4]) {
            if (P32(n)[1] == 7 && P32(n)[3] == P32(g_focus)[3] && P32(n)[8] == 0) {
                uint32_t id = P32(n)[2];
                if (id == 0xFFFFFFFFu) break;
                for (uint32_t u = g_unit_hash[id & 0x7F]; u != 0; u = P32(u)[0x39]) {
                    if (P32(u)[3] == id) {
                        if (P32(u)[0] == 1) return u;
                        fatal_exit(e, 0x5A); return 0;
                    }
                }
                break;
            }
        }
    }
    if (e->ord10227() == 0 || g_cl_ready == 0) return 0;
    if (g_panel_count != 0x26) { fatal_exit(e, 0x139B); return 0; }
    panel_core(e, 0x1C, 0x90, false, 0x6FB8099Cu, 3, true, true);
    return 0;
}

// The cells 9AE80 reads: ready 0x119854, panel origin 0x11B9A0 / 0x11B9A4, hud cel 0x11BED4, hud alt 0x11BEF0, y offset 0xDBC4C.
struct D2c9ae80Cells { uint32_t ready, x, y, cel, alt; int32_t yoff; };

// 1.13c D2Client+0x9AE80: HUD frame. Five draws (ord 10041, stdcall: cel struct, x, y, -1, 5, 0) share one zeroed 0x48-byte
// stack struct (frame index @0, hud cel @0x34, which the 0xBEAF0 result replaces before the fifth draw); then the sub-widgets.
// `unit97cd0` is the 0x97CD0 call (EAX = unit, 0 leaves the function).
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_9ae80_seam(const D2cClientEnv* e, const D2c9ae80Cells* c, uint32_t (*unit97cd0)()) {
    if (c->ready == 0) return;
    uint32_t u = unit97cd0();
    if (u == 0) return;
    e->sub_408c0();
    uint32_t ctx[18] = {0};
    ctx[13] = c->cel;
    const int32_t x = (int32_t)c->x, y = (int32_t)c->y, yo = c->yoff;
    ctx[0] = 0; e->draw(ctx, x,         y + yo - 0xE0, -1, 5, 0);
    ctx[0] = 1; e->draw(ctx, x + 0x100, y + yo - 0xE0, -1, 5, 0);
    ctx[0] = 2; e->draw(ctx, x,         y + yo - 0x30, -1, 5, 0);
    ctx[0] = 3; e->draw(ctx, x + 0x100, y + yo - 0x30, -1, 5, 0);
    ctx[13] = e->sub_6eaf0();
    ctx[0] = (c->alt != 0) + 10;
    e->draw(ctx, x + 0x110, y + yo - 0x3F, -1, 5, 0);
    e->sub_40eb0(u); e->sub_3c8b0(); e->sub_3ea20(); e->sub_3b2f0(u); e->sub_452a0(u); e->sub_3e8e0(u);
    e->sub_43450(u, 0);
}

static const D2cClientEnv* g_9ae80_env;
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_9ae80(const D2cClientEnv* e) {
    const D2c9ae80Cells c = { g_cl_ready, g_hud_x, g_hud_y, g_hud_cel, g_hud_alt, g_ui_yoff };
    g_9ae80_env = e;
    d2_client_9ae80_seam(e, &c, [] { return d2_client_97cd0(g_9ae80_env); });
}

// 1.13c D2Client+0x9CAD0: object/unit type id -> retail VA of its table (0 when unlisted)
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_client_9cad0(const uint32_t* p) {
    if (p == nullptr) return 0;
    uint32_t a;
    switch (p[1]) {
    case 0x92: case 0xF4: case 0xF5: case 0xF6: case 0x109: case 0x208: a = 0xF13C; break;
    case 0x93: a = 0xF1CC; break;  case 0x94: a = 0xF0DC; break;  case 0x96: a = 0xF274; break;
    case 0x9A: a = 0xF154; break;  case 0x9B: case 0xAF: a = 0xF304; break;
    case 0xB0: a = 0xF124; break;  case 0xB1: a = 0xF16C; break;  case 0xB2: a = 0xF19C; break;
    case 0xC6: a = 0xF1E4; break;  case 0xC7: a = 0xF184; break;  case 0xC8: a = 0xF1B4; break;
    case 0xC9: a = 0xF25C; break;  case 0xCA: a = 0xF28C; break;  case 0xD2: a = 0xF2A4; break;
    case 0xFB: case 0x209: a = 0xF2EC; break;
    case 0xFC: a = 0xF10C; break;  case 0xFD: a = 0xF214; break;  case 0xFE: a = 0xF0F4; break;
    case 0xFF: a = 0xF2D4; break;  case 0x101: a = 0xF1FC; break; case 0x129: a = 0xF2BC; break;
    case 0x195: a = 0xF22C; break; case 0x1FF: a = 0xF364; break; case 0x200: a = 0xF31C; break;
    case 0x201: a = 0xF34C; break; case 0x202: a = 0xF37C; break; case 0x203: a = 0xF334; break;
    default: return 0;
    }
    return 0x6FB80000u + a;   // 0x6fb8f0dc..0x6fb8f37c
}

// Addresses of the item chain head (0x11BC14) and group chain head (0x11BC1C), so a probe can pin the retail globals.
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t* d2_client_item_head_ref() { return &g_item_head; }
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t* d2_client_group_head_ref() { return &g_group_head; }

// 1.13c D2Client+0x9CF10: pull every item with key to just after its predecessor's relink (transcribed
// quirk: node moved to head, walk resumes through its new next)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_9cf10(uint32_t key) {
    uint32_t prev = 0;
    for (uint32_t n = g_item_head; n != 0; n = P32(n)[0x80 / 4]) {
        if (P32(n)[0x10 / 4] == key && prev != 0) {
            P32(prev)[0x80 / 4] = P32(n)[0x80 / 4];
            P32(n)[0x80 / 4] = g_item_head;
            g_item_head = n;
        }
        prev = n;
    }
}

// 1.13c D2Client+0x9CF60: splice every group's item sublist into the global item chain
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_9cf60() {
    uint32_t tail = 0;
    for (uint32_t g = g_group_head; g != 0; g = P32(g)[2]) {
        if (tail == 0) { tail = P32(g)[1]; g_item_head = tail; }
        else P32(tail)[0x80 / 4] = P32(g)[1];
        P32(g)[1] = 0;
        if (tail != 0) { for (uint32_t nx = P32(tail)[0x80 / 4]; nx != 0; nx = P32(tail)[0x80 / 4]) tail = nx; }
    }
}

// 1.13c D2Client+0x9CFC0 (fastcall: ecx unused, id, ebx key)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_9cfc0(uint32_t ecx, int16_t id, uint32_t key) {
    (void)ecx;
    if (id == -1) return;
    uint32_t cur = g_group_head, prev = 0;
    while (cur != 0) {
        if ((int16_t)*P16(cur) == id) {
            if (prev != 0) { P32(prev)[2] = P32(cur)[2]; P32(cur)[2] = g_group_head; g_group_head = cur; }
            uint32_t it = P32(cur)[1], pi = 0;
            for (;;) {
                if (it == 0) return;
                if (P32(it)[0x10 / 4] == key) break;
                pi = it; it = P32(it)[0x80 / 4];
            }
            if (pi == 0) return;
            P32(pi)[0x80 / 4] = P32(it)[0x80 / 4];
            P32(it)[0x80 / 4] = P32(cur)[1];
            P32(cur)[1] = it;
            return;
        }
        prev = cur; cur = P32(cur)[2];
    }
}

// 1.13c D2Client+0x9D040: insertion-sort the group chain ascending by (unsigned short) id
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_9d040() {
    uint32_t sorted = 0;
    while (g_group_head != 0) {
        uint32_t cur = g_group_head, next = P32(cur)[2];
        if (sorted != 0) {
            uint32_t p = sorted, before = 0, at;
            do { at = p; if (*P16(cur) < *P16(at)) break; p = P32(at)[2]; before = at; } while (P32(at)[2] != 0);
            if (before != 0) { P32(cur)[2] = P32(before)[2]; P32(before)[2] = cur; g_group_head = next; continue; }
        }
        P32(cur)[2] = sorted; sorted = cur; g_group_head = next;
    }
    g_group_head = sorted;
}

// 1.13c D2Client+0x9D0A0 (edi = group): move all items whose short @+0x22 == group id into the group
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_9d0a0(uint32_t grp) {
    uint32_t prev = 0, n = g_item_head;
    while (n != 0) {
        if (*P16(n + 0x22) == *P16(grp)) {
            uint32_t nx = P32(n)[0x80 / 4];
            if (prev == 0) g_item_head = nx; else P32(prev)[0x80 / 4] = nx;
            P32(n)[0x80 / 4] = P32(grp)[1];
            P32(grp)[1] = n;
            n = nx;
        } else { prev = n; n = P32(n)[0x80 / 4]; }
    }
}

// 1.13c D2Client+0x9D100 (void(void); 54 B to the ret, Ghidra size 51): free the group chain. Each node's next
// (+8) is read first, then the node is freed through Fog 10043 as ecx = node, edx = Roster.cpp (RVA 0xD4580),
// stack = line 0x22C and zero 0. The head DAT_6fbcbc1c (= g_group_head, RVA 0x11BC1C) is zeroed last.
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_9d100(const D2cClientEnv* e) {
    uint32_t g = g_group_head;
    while (g != 0) {
        uint32_t next = P32(g)[2];
        e->free10043(g, 0xD4580, 0x22C, 0);
        g = next;
    }
    g_group_head = 0;
}

// 1.13c D2Client+0x9D160 (eax = list head): insertion-sort an item chain by dword @+0x18, descending-stable
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_9d160(uint32_t head) {
    uint32_t sorted = 0;
    while (head != 0) {
        uint32_t nx = P32(head)[0x80 / 4];
        if (sorted != 0) {
            uint32_t p = sorted, prev = 0, at;
            do {
                at = p;
                if ((int32_t)P32(at)[0x18 / 4] < (int32_t)P32(head)[0x18 / 4]) break;
                p = P32(at)[0x80 / 4]; prev = at;
            } while (P32(at)[0x80 / 4] != 0);
            if (prev != 0) { P32(head)[0x80 / 4] = P32(prev)[0x80 / 4]; P32(prev)[0x80 / 4] = head; head = nx; continue; }
        }
        P32(head)[0x80 / 4] = sorted; sorted = head; head = nx;
    }
}
