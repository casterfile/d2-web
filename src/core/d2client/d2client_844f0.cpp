// 1.13c D2Client+0x844f0 -- item packet "remove from owner". EAX = packet (cdecl, no stack arguments): byte +2 length, dword +4 item id,
// byte +8 / dword +9 the owner's type / id, payload from +0xD. 0xCA90 decodes the payload into a 5-dword stack block; the owner unit comes
// from 0xA4E20 (null ends it); the item is found in the client item hash (D2Client+0x10AE08), a type other than 4 is fatal, a miss ends it. The
// owner's inventory (+0x60, null fatal) must hand the same item back from 0xC6D0 (null ends it, another pointer is fatal). Block words: +4 word,
// +6 word, +8 byte, +0xC dword, +0x10 byte. The stack word that held the 0xC92E byte keeps stale upper bytes in retail; the log masks them.
// gl: pinned module globals, +0 hash, +512 tick (0x119854), +516 0x11B834, +520 0x11B854, +524 0x11B858, +528 0x11B864.
#include "cl_env.h"
extern "C" void d2c_844f0(uint32_t pkt, const cl::Env& e) {
    uint32_t buf[5] = {};
    e.H(0xca90, pkt + 0xd, e.R8(pkt + 2) - 0xd, 0, (uint32_t)(uintptr_t)buf);
    const uint32_t ty = e.R8(pkt + 8), id9 = e.R(pkt + 9), pl = e.self;
    const uint32_t unit = e.H(0xa4e20, ty, 0, 0, 0, 0, id9 & 0x7f, id9, 0xB2000000u | (0x10A608u + (ty << 9)));
    if (!unit) return;
    const uint32_t id = e.R(pkt + 4);
    uint32_t item = 0;
    for (uint32_t n = e.R(e.gl + (id & 0x7f) * 4); n; n = e.R(n + 0xe4))
        if (e.R(n + 0xc) == id) { if (e.R(n) != 4) return; item = n; break; }   // fatal in retail
    if (!item) return;
    const uint32_t inv = e.R(unit + 0x60);
    if (!inv) return;                                          // fatal in retail
    const uint32_t got = e.H(0xc6d0, inv, item);
    if (!got || got != item) return;                           // the second is fatal in retail
    const uint32_t c = e.H(0xc92e, item) & 0xff;
    const uint32_t r8 = e.H(0xc8a4, unit, c, e.R(e.gl + 512));
    uint32_t a8[8] = { 0xB2000000u | 0xD4964u, 0x4cf, inv, buf[1] & 0xffff, buf[1] >> 16, r8, 1, 0 };
    e.H(0xc9fa, a8[0], a8[1], a8[2], a8[3], a8[4]);
    if (e.blk) e.blk(e.ctx, 0xc9fa, (uint32_t)(uintptr_t)a8, 8);
    e.H(0x8bbf0);
    if (e.R(unit) == 0) e.H(0xca0c, e.R(unit + 0x60));
    if (!e.H(0xc1a8, item, 0x100, 0x96, 0xB2000000u | 0xD4964u)) e.H(0xc544, unit, item);
    uint32_t it = item;
    const uint32_t r20 = e.H(0xca3c, buf[3], 0x20);
    if (r20) {
        const uint32_t iid = e.R(item + 0xc);
        e.H(0xa6ef0, iid, 4, 0, 0, 0, 0, 0xB2000000u | 0x10AE08u, iid & 0x7f);
        if (unit != pl) return;
        if ((buf[4] & 0xff) == 1) e.H(0x88a70, 0, 0, 0, 0, 0, 0, 0, 0, 1);
        else { e.W(e.gl + 516, 1); e.W(e.gl + 528, 1); e.W(e.gl + 520, 5); e.W(e.gl + 524, 1); }   // EBX = 1 here in retail
    } else {
        if (e.H(0xca3c, buf[3], 1)) {
            e.H(0xa6ef0, id, 4, 0, 0, 0, 0, 0xB2000000u | 0x10AE08u, id & 0x7f);
            it = e.H(0x833f0, pkt, (uint32_t)(uintptr_t)buf, 0, 0, 0, 0, 0, 0, 0, 0);
            if (!it) return;                                   // fatal in retail
        } else {
            e.H(0x82980, buf[1], buf[2] & 0xff, 0, buf[4], 0, item, 0, 0, buf[1]);
        }
        e.H(0xca24, it, 0xff);
        e.H(0xc7d8, inv, it);
        const uint32_t r = e.H(0xc202, inv);
        e.H(0x162d0, 0, 0, 0, 0, 0, r);
        e.H(0x81aa0, 0, 0, 0, 0, 0, 0, 0, 0, 0, it);
    }
    if (unit == pl) e.H(0x47d00, e.R(it + 0xc));
}
