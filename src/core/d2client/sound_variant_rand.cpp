// Upstream 4c98389d: Sounds.txt group variants draw from the client's existing
// hero seed (D2Client 1.13c +0x5EE70 through +0x89640).
#include <cstdint>
#include <emscripten.h>
extern "C" uint32_t d2_session_client_rand(uint32_t n);
extern "C" EMSCRIPTEN_KEEPALIVE uint32_t d2_client_sound_rand(uint32_t n) { return d2_session_client_rand(n); }
