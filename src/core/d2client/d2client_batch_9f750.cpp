// src/core/d2client/d2client_batch_9f750.cpp -- twelve UNMAPPED 1.13c D2Client.dll rows reimplemented from
// build-wasm/ghidra-d2client/decomp/0x<RVA>.c (alpha lane, batch 9f750). Base 0x6FAB0000. Retail globals are
// passed in (native pointers, never 32-bit casts); imports are host seams, a null seam keeps retail's flow.
// Register-argument functions keep retail's register order. No spec, no vector yet.
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <emscripten.h>

// Button slot table, stride 0x28, first slot at RVA(0x6fb8ec72) = 0x...8EC72 from here ("slot" base):
//   +0x00 left, +0x04 right, +0x08 top, +0x0C bottom (int32), +0x14 down flag (ec86), +0x15 hot flag (ec87).
// Count is DAT_6fb8ecb0 (int32).
struct D2C9f750Slots { uint8_t* base; int32_t count; };
static int32_t sl_i32(const D2C9f750Slots& s, int i, int off) { int32_t v; std::memcpy(&v, s.base + i * 0x28 + off, 4); return v; }
static void sl_clear(const D2C9f750Slots& s) { for (int i = 0; i < s.count; ++i) { s.base[i * 0x28 + 0x14] = 0; s.base[i * 0x28 + 0x15] = 0; } }

struct D2C9f750Env {
    void*   (*alloc_6f6c0)(void);                       // 0xBF6C0
    void    (*fn_88a70)(int, int, int, int);           // 0x88A70
    void*   (*make_dialog)(int, int, void*, int, int, int, int, int, int); // 0x53E40
    void    (*dialog_init)(void*, void*, int, int, int, int, int);        // 0x536F0
    void    (*fatal)(int msgId);                       // Ordinal_10265 + 10024 + _exit(-1)
    void*   (*get_text)(void);                         // Ordinal_10003
    void    (*send_packet)(const uint8_t* p13);        // 0x143E0
    void    (*fn_f5fb0)(void);                         // 0x2F5FB0
    void    (*free_dialog)(void*);                     // 0xA3D20 (EAX = the dialog)
    void    (*cleanup_f950)(void);                     // 0x9F950
    void    (*free_char)(uint32_t eax, uint32_t ecx, uint32_t edx, uint32_t esi);  // 0x3CA00 (EAX = Ordinal_10048 result, ECX/EDX/ESI = the table entry's +4/+8/+0)
    void    (*reset_72790)(int, int, int);             // 0xC2790
    void    (*free_handle)(void*);                     // Ordinal_10065
    void    (*free_10043)(int, int);                   // Ordinal_10043
    uint32_t (*ord_10048)(void);
    void    (*ord_11017)(uint32_t);
    void    (*dialog_cb_9fd70)(void);                  // 0x9FD70
};

// 1.13c D2Client+0x9F830 (no arguments; every input is a global). st = bb214, unitId = &bbfc->id (+0xC; null when bbfc is 0),
// dlgOut = bb224, flag210 = bb210, v21c/v220 = bb21c/bb220. Seams carry retail's register/stack arguments:
//   0x88A70 (EBX = 2, four zero stack words), 0xA3E40 (stdcall, the 11 stack words in block[0..10]; returns the popup),
//   0xD1EC (ECX = 0xD19, stack 0xF,0,1,0,0; returns the text), 0xA36F0 (ECX = popup, EAX = 0, one stack word = the text),
//   0x143E0 (17-byte packet, EBX = 17), 0x45FB0 (ECX = bb220). Returns 1 when it opened the popup.
struct D2C9f830Env {
    void     (*fn_88a70)(uint32_t ebx);
    void*    (*make_dialog)(const uint32_t* block11);
    uint32_t (*get_text)(uint32_t ecx, const uint32_t* stack5);
    void     (*dialog_text)(void* ecx, uint32_t eax, uint32_t text);
    void     (*send_packet)(const uint8_t* p17);
    void     (*fn_45fb0)(uint32_t ecx);
    void     (*fatal)(int msgId);
};
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_client_9f830(int32_t* st, const uint32_t* unitId, void** dlgOut, int32_t flag210,
                                                         int32_t v21c, int32_t v220, const D2C9f830Env* env) {
    if (*st != 2) return 0;
    env->fn_88a70(2);
    static const uint32_t kBlock[11] = { 0x96, 0x82, 0x9F430 /* RVA of the popup callback */, 0, 1, 0, 1, 0, 0, 0, 1 };
    *dlgOut = env->make_dialog(kBlock);
    if (!*dlgOut) { env->fatal(0x529); return 0; }
    reinterpret_cast<uint32_t*>(*dlgOut)[0x17] = 0;
    static const uint32_t kText[5] = { 0xf, 0, 1, 0, 0 };
    const uint32_t text = env->get_text(0xd19, kText);
    env->dialog_text(*dlgOut, 0, text);
    *st = 3;
    if (flag210 == 0) {
        const uint32_t id = unitId ? *unitId : 0xffffffffu, three = 3;
        // retail's packet is 17 bytes: 'D', id, v21c, v220, 3
        uint8_t q[17]; q[0] = 'D'; std::memcpy(q + 1, &id, 4); std::memcpy(q + 5, &v21c, 4); std::memcpy(q + 9, &v220, 4);
        std::memcpy(q + 13, &three, 4);
        env->send_packet(q);
        return 1;
    }
    env->fn_45fb0((uint32_t)v220);
    return 1;
}

// 1.13c D2Client+0x9F9E0
// unit60 = *(bbfc+0x60), hasUnit = bbfc != 0 (Ordinal_11017 takes that word). tab = the 8 entries x 3 dwords at RVA 0xDECB8
// (dword 0 = ESI, 1 = ECX, 2 = EDX of the 0x3CA00 call, whose EAX is the Ordinal_10048 result). dlg = bb224, bcc2fc = 0x11C2FC.
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_9f9e0(int32_t* st, uint32_t unit60, bool hasUnit, void** dlg, const uint32_t* tab, int32_t* bcc2fc, const D2C9f750Env* env) {
    *st = 0;
    if (hasUnit) env->ord_11017(unit60);
    env->cleanup_f950();
    const uint32_t h = env->ord_10048();
    if (h == 0) { env->fatal(0x18c); return; }
    for (uint32_t o = 0; o < 8; ++o) env->free_char(h, tab[o * 3 + 1], tab[o * 3 + 2], tab[o * 3]);
    if (*dlg) { void* d = *dlg; env->free_dialog(d); *dlg = nullptr; }
    env->reset_72790(0xe, 1, 0);
    *bcc2fc = 0;
}

// 1.13c D2Client+0x9FE50
// EDI = y (unaff), param_1 = x. Slot 0 -> 9F830, others -> 9FD70.
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_client_9fe50(int32_t x, int32_t y, uint8_t* slotBase, int32_t slotCount,
                                                         uint32_t (*open9f830)(void), const D2C9f750Env* env) {
    D2C9f750Slots s{slotBase, slotCount};
    uint32_t r = 0;
    for (int i = 0; i < s.count; ++i) {
        if (s.base[i * 0x28 + 0x15] && sl_i32(s, i, 0) <= x && x <= sl_i32(s, i, 4) && sl_i32(s, i, 8) <= y && y <= sl_i32(s, i, 0xC)) {
            env->fn_88a70(0, 0, 0, 0);
            if (i == 0) open9f830(); else env->dialog_cb_9fd70();
            r = 1;
        }
    }
    sl_clear(s);
    return r;
}

// Fog/D2Common imports for the rows below (ordinals as in the decompile).
struct D2C9f750Env2 {
    void* (*item_slot)(void* unit, int slot);       // Ordinal_11139
    int   (*ord_11006)(void*);
    int   (*ord_11088)(void*);
    int   (*ord_10744)(void*, int);
    void* (*ord_10708)(int);
    int   (*ord_10631)(void*);
    void  (*ord_11031)(void*, int);
    void  (*ord_11103)(void);
    void  (*size_10561)(void* p, int32_t* wh);      // Ordinal_10561: wh[0]=w, wh[1]=h
    void* (*ord_11013)(int, int, int, int, int);
    void  (*ord_10807)(void);
    void  (*ord_10188)(void*, int, int, int);
    void  (*ord_10819)(void);
    int   (*ord_10276)(uint32_t, void*);
    void* (*ord_10494)(uint32_t unit, int skill);
    void* (*ord_10871)(uint32_t unit, int skill);
    int   (*ord_10680)(void*, int, int);
};

// 1.13c D2Client+0xA07C0
// EBX -> a, EDI -> b: (a,b) = (b, -a), then halved together until a^2+b^2 <= 3.
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_a07c0(int32_t* a, int32_t* b) {
    int32_t v3 = -*a, v2 = *b;
    while (v3 * v3 + v2 * v2 > 3) { v2 /= 2; v3 /= 2; }
    *a = v2; *b = v3;
}

// 1.13c D2Client+0xA0800
// ESI -> a, param_2 -> b: scale both by 4 if a^2+b^2 < 4, then by 2 if the (new) sum < 0x10.
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_a0800(int32_t* a, int32_t* b) {
    int32_t x = *a, s = *b * *b + x * x;
    if (s < 4) { *a = x * 4; *b = *b * 4; x = *a; s = *b * *b + x * x; }
    if (s < 0x10) { *a = x * 2; *b = *b << 1; }
}
