// src/core/d2game/d2game_batch_caf30.cpp -- 1.13c D2Game+0xCAF30, the client system-message dispatcher, transcribed from the disassembly.
//
// Retail: stdcall(msg), ret 4. ebx = msg[0] (the client id), ebp = msg + 4, the command is the byte at ebp (0x67..0x70; anything else returns).
// Every callee is a seam `call(rva, eax, esi, edi, stack, n)`: the register inputs retail holds at the call (eax/esi/edi, zero when the callee
// takes none) and the n stack words, top of stack first. A pointer to a local is the address of the port's local (the callee writes the out values
// through it). Fatal paths (a 0x6C record with a size dword at or above 0x2000, 0x6A lost-lock) are not driven: they return.
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <emscripten.h>

namespace {
inline uint8_t  r8 (const uint8_t* p, uint32_t o) { return p[o]; }
inline uint16_t r16(const uint8_t* p, uint32_t o) { uint16_t v; std::memcpy(&v, p + o, 2); return v; }
inline uint32_t r32(const uint8_t* p, uint32_t o) { uint32_t v; std::memcpy(&v, p + o, 4); return v; }
inline uint32_t A(const void* p) { return (uint32_t)(uintptr_t)p; }
}

struct D2GameCaf30Env {
    uintptr_t (*call)(uint32_t rva, uintptr_t eax, uintptr_t esi, uintptr_t edi, const uint32_t* stk, uint32_t n);
    uint32_t updateFlag;   // DAT_6FD31C3C
};

extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_CAF30(const uint8_t* msg, const D2GameCaf30Env* env)
{
    const uint32_t ebx = r32(msg, 0);
    const uint8_t* p = msg + 4;
    auto C = [&](uint32_t rva, uintptr_t eax, uintptr_t esi, uintptr_t edi, std::initializer_list<uint32_t> w) -> uintptr_t {
        uint32_t s[12] = {0}; uint32_t n = 0; for (uint32_t v : w) s[n++] = v;
        return env->call(rva, eax, esi, edi, s, n);
    };
    switch ((uint8_t)(p[0] - 0x67)) {
    case 0:   // 0x67: new game
        if (C(0x2A850, (uintptr_t)p, 0, 0, { ebx }) == 0) return;
        C(0x2C2D0, r32(p, 0x27), 0, 0, { ebx, A(p + 1), r8(p, 0x11), r8(p, 0x12), A(p + 0x15), r16(p, 0x25), r8(p, 0x13), r8(p, 0x14), r8(p, 0x2D), 0, 0 });
        return;
    case 1: { // 0x68: join game
        uint32_t l0c = 0, l10 = 0, l14 = 0, l18 = 0, l58 = 0;
        uint8_t l20 = 0;
        if (env->updateFlag == 0) {
            if (C(0xCAD60, r16(p, 5), 0, A(p + 0x15), { r32(p, 8) }) != 0) {
                C(0x8C4B0, ebx, 0, 0, {});   // EDX = the client id (carried in the eax slot)
                C(0xA358, 0, 0, 0, { ebx, 0xB2000000u | 0x991AC, 0x148 });
            }
        }
        if (C(0x2A740, 0, 0, 0, { ebx, r16(p, 5), r8(p, 7), r32(p, 1), A(&l20), A(&l18), r8(p, 0x14), A(&l14), A(&l10), A(&l0c), A(&l58) }) == 0) return;
        C(0x2BB10, 0, 0, 0, { r16(p, 5), r8(p, 7), A(p + 0x15), A(&l20), l18, r8(p, 0x14), l14, l10 });
        return;
    }
    case 2:   // 0x69: end game
        if (C(0x49930, 0, ebx, 0, { 4 }) != 0) C(0x2DC70, 0, 0, 0, { ebx, 1 });
        return;
    case 3:   // 0x6A: join list
        C(0x2BA10, 0, 0, 0, {});
        return;
    case 4:   // 0x6B: join act
        if (C(0x49930, 0, ebx, 0, { 1 }) != 0) C(0x2E360, ebx, 0, 0, {});
        return;
    case 5:   // 0x6C: open char (a size dword at or above 0x2000 is the fatal 0x1A5 path)
        if (r32(p, 2) >= 0x2000) return;
        C(0x2AAC0, (uintptr_t)p, 0, 0, { ebx });
        return;
    case 6:   // 0x6D: ping
        C(0x4A780, ebx, 0, 0, { r32(p, 1), r32(p, 5) });
        return;
    case 7:   // 0x6E: disconnect
        if (C(0x2A6E0, ebx, 0, 0, {}) != 0) C(0x2D7E0, ebx, 0, 0, { 0 });
        return;
    case 8:   // 0x6F: no body
        return;
    case 9: { // 0x70: flag the command client and release the client list lock
        uint8_t* client = (uint8_t*)C(0x4A0A0, 0, ebx, 0, {});
        if (client != nullptr) { uint32_t one = 1; std::memcpy(client + 0x504, &one, 4); }   // the LeaveCriticalSection is an unlogged iat stub
        return;
    }
    default:
        return;
    }
}
