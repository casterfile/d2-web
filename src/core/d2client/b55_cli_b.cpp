// D2Client batch b55-cli-b: seam-based, no Win32.
#include <cstdint>
#include <cstring>

struct B55cliBEnv {
    void* (*alloc)(int size, int flag);                 // Fog ordinal 10042
    void (*fatal)(int msgId);                           // Ordinal_10265 + 10024 + _exit(-1)
    void (*writeReg)(const char* sec, const char* key, int zero, uint32_t val); // Ordinal_426
    void (*applyResolution)(uint32_t esi);              // D2Client+0x10DF0, ESI = (v == 1 ? 2 : 0)
    void (*setPerspective)(uint32_t v);                 // Ordinal_10081
    void (*perspectiveUpdate)(uint32_t eax, uint32_t ecx, uint32_t esi); // D2Client+0x8A4D0 (EDX = 0, one stack word = 0)
    uint32_t gEax, gEcx, gEsi;                          // DAT_6fba9e18, DAT_6fba9e14, DAT_6fbcc3c4
};
static B55cliBEnv b55_cli_b_env;
extern "C" void b55_cli_b_set_env(const B55cliBEnv* e) { b55_cli_b_env = *e; }

// 1.13c D2Client+0x68990
extern "C" uint32_t* b55_cli_b_sub_68990() {
    uint32_t* p = (uint32_t*)b55_cli_b_env.alloc(0x804, 0);
    if (!p) { b55_cli_b_env.fatal(0x805); return 0; }
    for (int i = 0; i < 15; i++) p[i] = 0;
    p[0xd] = 2;
    p[0xe] = 2;
    return p;
}

// 1.13c D2Client+0x662A0  (param_1 = this, field +0x124)
extern "C" void b55_cli_b_sub_662A0(const uint8_t* self) {
    uint32_t v; memcpy(&v, self + 0x124, 4);
    b55_cli_b_env.writeReg("Diablo II", "Resolution", 0, v);
    b55_cli_b_env.applyResolution(v == 1 ? 2u : 0u);
}

// 1.13c D2Client+0x64910
extern "C" void b55_cli_b_sub_64910(const uint8_t* self) {
    uint32_t v; memcpy(&v, self + 0x124, 4);
    b55_cli_b_env.writeReg("Diablo II", "Perspective", 0, v);
    b55_cli_b_env.setPerspective(v);
    b55_cli_b_env.perspectiveUpdate(b55_cli_b_env.gEax, b55_cli_b_env.gEcx, b55_cli_b_env.gEsi);
}
