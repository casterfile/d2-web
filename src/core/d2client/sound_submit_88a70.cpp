// 1.13c D2Client+0x88A70 (299 B): queue/refresh a sound entry. EBX = sound id (register), stack args
// param_1 = unit, param_2 = offset added to now, param_3, param_4 stored in the entry.
// Globals and unrecovered callees are env members; entry = 0x59-byte slot, int-indexed as in retail.
#include <cstdint>
#include <cstring>
#include <emscripten.h>

struct D2cSndSubmitEnv {
    uint8_t* recs; int32_t count;     // [0x6FBCC24C]/[0x6FBCC250], rows 0x8E
    int32_t  enabled;                 // [0x6FBCBF34]
    int32_t  now;                     // [0x6FBCC268]
    uint32_t player;                  // [0x6FBCBBFC]
    uint8_t* (*find87fe0)(int32_t id, int32_t zero);   // FUN_87FE0(0), EAX=id
    uint8_t* (*alloc87d80)(void);                     // FUN_87D80
    void     (*mkvec87f30)(float* out, uint32_t unit, int32_t id); // FUN_87F30 (out = EDI, stdcall unit, id)
    void     (*set88350)(float* v, uint8_t* e);       // FUN_88350 (fastcall ecx,edx)
    float    (*pan88440)(int32_t id, uint32_t unit);  // FUN_88440 (EAX=id)
    void     (*ord10034)(uint32_t ecx, uint32_t edx);   // thunk 0xD276: ECX = entry + 0x28, EDX = unit
    void     (*fn22390)(uint32_t unit, int32_t id);     // FUN_22390: EAX = unit, stack word = the entry's id (+8)
};
template <class T> static T rd(const uint8_t* p, size_t o) { T v; memcpy(&v, p + o, sizeof v); return v; }
template <class T> static void wr(uint8_t* p, size_t o, T v) { memcpy(p + o, &v, sizeof v); }

extern "C" EMSCRIPTEN_KEEPALIVE
int32_t d2_client_88a70(const D2cSndSubmitEnv* E, int32_t id, uint32_t unit, int32_t off, int32_t p3, int32_t p4) {
    if (!E->enabled || id < 1) return 0;
    const uint8_t* row = (E->recs && id < E->count) ? E->recs + (size_t)id * 0x8E : nullptr;
    if (!row) return 0;
    if (rd<uint8_t>(row, 0x3C) == 0) return 0;
    uint8_t* e = nullptr;
    if (rd<int16_t>(row, 0x45) != 0) e = E->find87fe0(id, 0);
    if (!e) {
        e = E->alloc87d80();
        if (!e) return 0;
        wr<int32_t>(e, 0x38, p3);
        wr<int32_t>(e, 4, id);
        wr<int32_t>(e, 0x34, E->now + off);
        wr<int32_t>(e, 0x24, p4);
        wr<int32_t>(e, 0x1C, 0xFF);
        wr<uint8_t>(e, 0x3C, rd<uint8_t>(row, 0x4E));
        float v[3];
        if (unit == 0) { v[0] = 0; v[1] = 0; v[2] = 320.0f; E->set88350(v, e); }
        else {
            E->mkvec87f30(v, unit, id);
            E->set88350(v, e);
            wr<float>(e, 0x20, E->pan88440(id, unit));   // fstp dword [esi+0x20]: the float's own bits
        }
    }
    if (unit == E->player) wr<uint8_t>(e, 0x3C, (uint8_t)(rd<uint8_t>(e, 0x3C) + 'P'));
    if (unit != 0) { E->ord10034((uint32_t)(uintptr_t)(e + 0x28), unit); E->fn22390(unit, rd<int32_t>(e, 8)); }
    return rd<int32_t>(e, 8);
}
