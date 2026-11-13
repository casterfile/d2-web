// Shared diagnostic descriptor layout, documented in d2game_gp.h and d2game_hk2.h:
// r[13] holds the blob count; each address/byte-length pair starts at r[14+2*k].
// Keep the length lookup beside those wrappers so a clean checkout can compile them.
#pragma once
#include <cstdint>
static inline uint32_t fnBlobLen(const uint32_t* r, uint32_t k) {
    return r[15 + 2 * k];
}
