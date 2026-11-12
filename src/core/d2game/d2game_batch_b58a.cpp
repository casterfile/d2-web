// src/core/d2game/d2game_batch_b58a.cpp -- 1.13c D2Game.dll functions transcribed from the cached
// decompiles, Stage A batch lane (implementation + marker only). Callees the file does not own are an
// Env seam in retail argument order. Skipped (register-argument or too large): 0x86900, 0x8A770,
// 0x8B3D0, 0x98250, 0x9BCB0, 0x9C620, 0x9D3B0, 0x9F340, 0xA39E0, 0xA3B50.
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <emscripten.h>

struct D2GameBatchB58aEnv {
    void (*ord10920)(void* p);                     // Ordinal_10920
    void (*sendPacket)(const uint8_t* buf, int n); // D2Game+0x8A3E0
    void (*ord10945)(void* u, int skill, int v);
    void (*ord10819)(void* u, const char* file, int line);
    void (*ord10418)(void* u);
    int  (*ord11061)(void* u, int skill);
    void (*d2game57110)(int a, int b);             // D2Game+0x37110
    void (*d2game96fa0)(int a, void* u);           // D2Game+0x96FA0
    void (*fatal)(int line);                       // Ordinal_10265/10024 + _exit(-1)
};

// 1.13c D2Game+0x96810
extern "C"
EMSCRIPTEN_KEEPALIVE void d2game_legacy_96810(uint32_t, int32_t* p2, const uint32_t* p3,
                                             const D2GameBatchB58aEnv* env)
{
    if (p2 && *p2 == 0) env->ord10920(p2);
    uint8_t b[13];
    b[0] = 0x7a;
    b[1] = 0;
    b[2] = (uint8_t)p3[3];
    b[3] = (uint8_t)(*(const uint16_t*)(p3 + 2));
    b[4] = (uint8_t)(*(const uint16_t*)(p3 + 2) >> 8);
    std::memcpy(b + 5, p3 + 1, 4);
    std::memcpy(b + 9, p3, 4);
    env->sendPacket(b, 13);
}

// 1.13c D2Game+0x9F7A0
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_old_9f7a0(int32_t* u, int32_t skill, const D2GameBatchB58aEnv* env)
{
    if (!u) env->fatal(0xa2a);
    env->d2game57110(1, skill);
    bool skip = false;
    if ((*((uint8_t*)u + 0xc6) & 1) == 0) {
        if (u[0] == 0) { if (u[4] != 0) skip = (u[4] != 0x11); }
        else if (u[0] == 1 && u[4] != 0) skip = (u[4] != 0xc);
    }
    if (!skip && env->ord11061(u, skill) != 0) return;
    env->ord10945(u, skill, 0);
    env->ord10819(u, "..\\Source\\D2Game\\SKILLS\\Skills.cpp", 0x485);
    env->ord10418(u);
    if (u[0] == 0) env->d2game96fa0(u[0x20], u);
}

// The A39E0 gate: d2game_a39e0.cpp is its own file.
