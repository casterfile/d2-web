// src/core/d2client/d2client_batch_a5a30.h -- D2Client 0xA5A30..0xA7D00 (the client unit hash tables and unit events):
// retail's registers and stack words as explicit parameters (pointers are retail's 32-bit words); every callee goes
// through FnCallees. Tables: 6 unit types x 0x80 buckets of 32-bit unit pointers at module 0x10A608 (live) and
// 0x109A08 (the unit flag 0xC4 bit 21 table); a unit's id is +0xC, its type +0, its bucket link +0xE4.
#pragma once
#include <cstdint>
#include "fn_callees.h"
#include <cstddef>
namespace d2client_batch_a5a30 {
void sub_A5A30(FnCallees& s, uint32_t unit /*esi*/, uint32_t edi, uint32_t ebx, uint32_t p /*stack*/);
void sub_A5C50(FnCallees& s, uint32_t owner /*ebx*/, uint32_t flag /*stack*/);
uint32_t sub_A6500(FnCallees& s, uint32_t unit /*esi*/);
void sub_A6680(FnCallees& s);
void sub_A6F60(FnCallees& s, uint32_t unit /*esi*/, int32_t n /*eax*/, uint32_t kind /*ecx*/);
uint32_t sub_A6FC0(FnCallees& s, uint32_t x /*stack*/);
void sub_A7000(FnCallees& s);
void sub_A70C0(FnCallees& s, int32_t ev /*eax*/, uint32_t unit /*ecx*/);
void sub_A7D00(FnCallees& s, uint32_t edi);
}

// Env shared with the FN probe (src/core/fn/d2client_a6180.cpp): unit pointers are retail 32-bit words.
#include <cstddef>
typedef uintptr_t U;
static inline uint32_t& W(U base, uint32_t off) { return *(uint32_t*)(base + off); }
static inline U Lnk(U base, uint32_t off) { return (U)W(base, off); }

struct D2ClientA5Env {
    void (*fatal)(uint32_t code);                                  // Ordinal_10265+10024+_exit(-1)
    void (*ord_10574)(); void (*ord_10032)(); void (*ord_10857)();
    U (*fn_9d630)(uint32_t a, uint32_t b);                         // 0x9D630
    U (*ord_10460)(U list); U (*ord_10464)(U node); uint32_t (*ord_10298)(U node);
    uint32_t (*fn_a52a0)(U owner);                                 // 0xA52A0
    void (*ord_10061)(U list); uint32_t (*ord_10202)(U unit, uint32_t, uint32_t, const char* file);
    void (*ord_11127)(); uint32_t (*ord_10646)(U list, U unit);
    void (*fn_a6ef0)(U row, uint32_t bucket, uint32_t id, uint32_t type);
    uint32_t (*fn_1510)(uint32_t a, uint32_t b);                   // 0x1510
    U (*ord_10688)(U p);
    U (*fn_a4e20)(uint32_t id, U row, uint32_t type);
    void (*fn_22b20)(U unit);
    int32_t (*ord_11044)(U a, uint32_t b); U stamp_src;            // DAT_6fbcc3b8
    void (*fn_25280)();
    void (*fn_6b300)(U unit, uint32_t a, uint32_t b, int32_t c, uint32_t d);
    void (*fn_88a70)(U unit, uint32_t a, uint32_t b, uint32_t c);
    U (*fn_51a80)();
    void (*fn_a6d60)(U unit);
    void (*fn_c7b00)(); void (*fn_c7e30)(int32_t a, int32_t b, int32_t c);
    void (*fn_53320)();
    U (*ord_10476)(); void (*ord_10561)();
    void (*fn_a7bf0)(uint32_t a, int32_t b, int32_t c, uint32_t d);
    const uint32_t* type_order;                                    // DAT_6fb8412c, 6 entries
};

