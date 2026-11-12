// src/core/d2client/d2client_batch_a5a30.cpp -- 1.13c D2Client.dll unit-table leaves
// (alpha lane), from build-wasm/ghidra-d2client/decomp/0x<RVA>.c. Unrecovered callees, ordinals and the
// register-carried arguments (EAX/ESI/EBX in the decompile) are explicit parameters / env seams.
// ponytail: retail unit/list links are 32-bit slots (+0xE4 next, +0x54 sub-ptr, node +0xC next); they are
// read zero-extended, valid only while the host keeps those objects below 4 GiB. Widen with the unit layout.
#include <cstdint>
#include <cstddef>
#include <emscripten.h>

#include "d2client_batch_a5a30.h"

// 1.13c D2Client+0xA5B60 (37 B): EAX = rec, ECX = arg. 0xFFFF = none.
extern "C" EMSCRIPTEN_KEEPALIVE uint16_t d2_client_a5b60(U rec, uint32_t arg, const D2ClientA5Env* e) {
    if (rec && W(rec, 0) == 0) {
        U r = e->fn_9d630(arg, W(rec, 12));
        return r ? *(uint16_t*)(r + 0x22) : 0xffff;
    }
    return 0xffff;
}

// 1.13c D2Client+0xA6180 (124 B): EAX = unit.
extern "C" EMSCRIPTEN_KEEPALIVE bool d2_client_a6180(U u, const D2ClientA5Env* e) {
    if (!u) return false;
    uint32_t v = W(u, 16);
    switch (W(u, 0)) {
    case 0: return v == 0x11;
    case 1: return e->fn_1510(1, 0x14) == 0 && v == 0xc;
    case 2: {
        U d = e->ord_10688(Lnk(u, 4));
        uint8_t f = *(uint8_t*)(d + 0x1b7);
        return (f & 2) || (v == 2 && (f & 1));
    }
    case 3: return (W(u, 0xc4) & 0x10000) != 0;
    case 4: return v == 3;
    }
    return false;
}

// 1.13c D2Client+0xA6210 (56 B): ESI = unit.
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_a6210(U u, const D2ClientA5Env* e) {
    uint32_t f = W(u, 0xc4) & 0xfffeffff;
    W(u, 0xc4) = f;
    if (W(u, 0) == 1 && e->fn_1510(f, 9) == 0) return;
    W(u, 0xc4) = f | 4;
}


// ---- the rows below follow retail's call sites (d2client_batch_a5a30.h; callees through FnCallees) ----
#include "d2client_batch_a5a30.h"
namespace d2client_batch_a5a30 {
static const uint32_t kTabA = 0x10a608, kTabB = 0x109a08;           // 6 types x 0x80 buckets x 4 bytes
static const uint32_t kTypeOrder[6] = {1, 0, 2, 3, 4, 5};          // .rdata 0xD412C..0xD4144
static inline uint32_t W32(uint32_t p, uint32_t o) { return fnAt<uint32_t>(fnPtr(p), o); }

// 1.13c D2Client+0xA5A30   (esi = unit, edi, ebx, stack p; ret 4)
void sub_A5A30(FnCallees& s, uint32_t unit, uint32_t edi, uint32_t ebx, uint32_t p) {
    if (unit) fnAt<uint32_t>(fnPtr(unit), 0xc4) |= 0x22;
    s.h(0xc622, 0, 0, 0, 0, 0, unit, edi, p, ebx);
    s.h(0xc67c, 0, 0, 0, 0, 0, unit, edi, p, ebx);
    s.h(0xc6ac, 0, 0, 0, 0, 0, unit, edi);
}
// 1.13c D2Client+0xA5C50   (ebx = owner, stack flag; ret 4): settle the owner's item list against the type-4 bucket table
void sub_A5C50(FnCallees& s, uint32_t owner, uint32_t flag) {
    if (!owner) s.fatal(0xa80);
    uint32_t list = W32(owner, 0x60);
    if (!list) return;
    for (uint32_t cur = s.h(0xc4f0, 0, 0, 0, 0, 0, list); cur;) {
        uint32_t next = s.h(0xc568, 0, 0, 0, 0, 0, cur);
        uint32_t id = s.h(0xc68e, 0, 0, 0, 0, 0, cur);
        uint32_t p = s.gw(kTabA + 4 * 0x80 * 4 + (id & 0x7f) * 4);
        while (p && W32(p, 0xc) != id) p = W32(p, 0xe4);
        if (p) {
            if (W32(p, 0) != 4) s.fatal(0x5a);
            if (!flag || !s.h(0xa52a0, p, 0, 0, 0, 0, owner)) {
                if (!W32(owner, 0)) s.h(0xca0c, 0, 0, 0, 0, 0, W32(owner, 0x60));
                if (!s.h(0xc1a8, 0, 0, 0, 0, 0, p, 0x100, 0x96, s.mod(0xd4964))) s.h(0xc544, 0, 0, 0, 0, 0, owner, p);
                if (!s.h(0xc6d0, 0, 0, 0, 0, 0, list, p)) s.fatal(0xa92);
                uint32_t ty = W32(p, 0), pid = W32(p, 0xc);
                s.h(0xa6ef0, 0, s.mod(kTabA + ty * 0x200), pid & 0x7f, 0, 0, pid, ty);
            } else if (!s.h(0xc6d0, 0, 0, 0, 0, 0, list, p)) s.fatal(0xa96);
        }
        cur = next;
    }
}
// 1.13c D2Client+0xA6500   (esi = unit): insert the unit in its bucket (descending id); 0 when already present
uint32_t sub_A6500(FnCallees& s, uint32_t u) {
    bool b = u && (W32(u, 0xc4) >> 0x15 & 1);
    uint32_t tab = b ? kTabB : kTabA;
    for (int k = 0; k < 2; k++) {
        uint32_t id = W32(u, 0xc), ty = W32(u, 0);
        if (s.h(0xa4e20, id & 0x7f, id, s.mod(tab + ty * 0x200), 0, 0, ty)) {
            if (!k) return 0;
            s.fatal(b ? 0x9c : 0xa5);
        }
    }
    s.h(0x22b20, 1, u);
    uint32_t id = W32(u, 0xc), ty = W32(u, 0);
    uint32_t slot = ((W32(u, 0xc4) >> 0x15 & 1) ? kTabB : kTabA) + ty * 0x200 + (id & 0x7f) * 4;
    uint32_t prev = 0, cur = s.gw(slot);
    while (cur && W32(cur, 0xc) > id) { prev = cur; cur = W32(cur, 0xe4); }
    fnAt<uint32_t>(fnPtr(u), 0xe4) = cur;
    if (prev) fnAt<uint32_t>(fnPtr(prev), 0xe4) = u; else s.gw(slot) = u;
    return 1;
}
// 1.13c D2Client+0xA6680: when the stamp of 0x11C3B8 moves (0x123420), run 0x25280 over every type-2 unit of both tables
void sub_A6680(FnCallees& s) {
    uint32_t v = s.h(0xc5fe, 0, 0, 0, 0, 0, s.gw(0x11c3b8), 0);
    if (v == s.gw(0x123420)) return;
    s.gw(0x123420) = v;
    for (int t = 0; t < 2; t++) {
        uint32_t base = (t ? kTabB : kTabA) + 2 * 0x200;
        for (uint32_t i = 0; i < 0x80; i++)
            for (uint32_t p = s.gw(base + 4 * i); p;) {
                uint32_t next = W32(p, 0xe4);
                if (W32(p, 0) != 2) s.fatal(t ? 0x896 : 0x88c);
                s.h(0x25280, 0, 0, 0, 0, p);
                p = next;
            }
    }
}
// 1.13c D2Client+0xA6F60   (esi = unit, eax = n, ecx = kind)
void sub_A6F60(FnCallees& s, uint32_t u, int32_t n, uint32_t kind) {
    if (!u) return;
    if (n > 0) s.h(0x6b300, kind, 0, 0, 0, 0, u, 1, 0, (uint32_t)n);
    else s.h(0x6b300, kind, 0, 0, 0, 0, u, 2, 0, 0);
    if (kind == 0x97) s.h(0x88a70, 0, 0, 0, 0x18c, 0, u);
    else if (kind == 0x98) s.h(0x88a70, 0, 0, 0, 0x18d, 0, u);
}
// 1.13c D2Client+0xA6FC0   (stack x; ret 4): 1 when the hovered unit (0x51A80) is x, 0 for a flagged monster
uint32_t sub_A6FC0(FnCallees& s, uint32_t x) {
    uint32_t p = s.h(0x51a80);
    if (p && W32(p, 0) == 2) {
        uint32_t q = s.h(0xc226, 0, 0, 0, 0, 0, W32(p, 4));
        if (fnAt<int8_t>(fnPtr(q), 0x167) < 0) return 0;
    }
    return s.h(0x51a80) == x;
}
// 1.13c D2Client+0xA7000: free every unit of both tables through 0xA6D60, types in the .rdata order
void sub_A7000(FnCallees& s) {
    for (int t = 0; t < 2; t++)
        for (int k = 0; k < 6; k++)
            for (uint32_t i = 0; i < 0x80; i++) {
                uint32_t slot = (t ? kTabB : kTabA) + kTypeOrder[k] * 0x200 + 4 * i;
                for (uint32_t p = s.gw(slot); p;) {
                    uint32_t next = W32(p, 0xe4);
                    s.h(0xa6d60, 0, 0, 0, 0, 0, p);
                    s.gw(slot) = next;
                    p = next;
                }
            }
}
// 1.13c D2Client+0xA70C0   (eax = event, ecx = unit)
void sub_A70C0(FnCallees& s, int32_t ev, uint32_t u) {
    if (ev > 0x161) {
        if (ev > 0x209) {
            if (ev == 0x21f) s.h(0x6b300, 0x113, 0, 0, 0, 0, u, 3);
            else if (ev == 0x2c0) s.h(0x6b300, 0x7a, 0, 0, 0, 0, u, 3);
            return;
        }
        if (ev == 0x209) { s.h(0x53320, u); return; }
        if (ev == 0x196) {
            s.h(0xc7b00, u);
            s.h(0xc7e30, u, 0xffffffc9u, 0, 0, 0, 0);
            if (!u) return;
            uint32_t a = W32(u, 0x54), d = a ? W32(a, 0x30) : 0;
            if (d) { fnAt<uint32_t>(fnPtr(d), 0x10) = 0; fnAt<uint32_t>(fnPtr(d), 0x14) = 0; fnAt<uint32_t>(fnPtr(d), 0x18) = 0x800; }
            a = W32(u, 0x54); d = a ? W32(a, 0x30) : 0;
            if (d) { fnAt<uint32_t>(fnPtr(d), 0x28) = 0; fnAt<uint32_t>(fnPtr(d), 0x2c) = 0; fnAt<uint32_t>(fnPtr(d), 0x30) = 0x32; }
            a = W32(u, 0x54); d = a ? W32(a, 0x30) : 0;
            if (d) fnAt<uint32_t>(fnPtr(d), 0) |= 0x20;
            return;
        }
        if (ev > 0x1a0 && ev <= 0x1a2 && u) s.h(0x6b300, 0x10e, 0, 0, 0, 0, u, 2);
        return;
    }
    if (ev >= 0x15f) {
        uint32_t e = (u ? W32(u, 4) : 0xffffffffu) - 0xb3;
        s.h(0x6b300, e, 0, 0, 0, 0, u, 4, 0, e + 3);
        return;
    }
    static const int32_t dd[4] = {-0x20, 0x20, 0x20, -0x20}, d0[4] = {-0x20, -0x20, 0x20, 0x20};
    switch (ev) {
    case 0xf2: s.h(0x6b300, 0x7a, 0, 0, 0, 0, u, 3); break;
    case 0x14d: s.h(0x6b300, 0xba, 0, 0, 0, 0, u, 2); break;
    case 0x154: case 0x155: case 0x156: case 0x157:
        s.h(0xc7b00, u);
        s.h(0xc7e30, u, 0, (uint32_t)dd[ev - 0x154], 0, 0, (uint32_t)d0[ev - 0x154]);
        break;
    default: break;
    }
}
// 1.13c D2Client+0xA7D00   (edi = view, also 0xC652's stack word: ret 4): draw every visible node of the 0xC652 list through 0xA7BF0, offset by 0xC5B0's origin
void sub_A7D00(FnCallees& s, uint32_t edi) {
    for (uint32_t n = s.h(0xc652, 0, 0, 0, 0, 0, edi); n; n = W32(n, 0xc)) {
        if (!(W32(n, 0x1c) & 1)) continue;
        int32_t o[2] = {0, 0};
        s.h(0xc5b0, 0, 0, 0, 0, 0, edi, (uint32_t)(uintptr_t)o);
        s.h(0xa7bf0, 0, W32(n, 4), W32(n, 8) + (uint32_t)o[0], 0, 0, W32(n, 0x18) + (uint32_t)o[1], W32(n, 0x14), W32(n, 0));
    }
}
}
