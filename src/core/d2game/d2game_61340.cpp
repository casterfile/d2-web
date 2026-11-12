// 0x61340 (ECX = game, EDX = client; plain ret): first type-4 node. If its spawned flag (data +0x95) is clear: room = #10331(client); 0x1280 (EAX = room, EBX = data +0x9C; EDI = data +0xA0 is not recorded,
// tests the spot; failing it shifts both coordinates by 3, passing it spawns a unit via 0xE1D90 (EDX = 0xBD, ECX = 2, 7 stack words) and records its id at +0x98. Returns 1.
#include "d2game_q32.h"
using namespace q32;
// 1.13c D2Game+0x61340
uint32_t d2game_61340_q32(const Call& X, uint32_t game, uint32_t client) {
    uint32_t n = findQ(game, 4);
    if (!n) return 1;
    uint32_t d = rd32(n, 0x18);
    if (rd8(d, 0x95)) return 1;
    uint32_t room = X(0xA478, 0, 0, 0, 0, 0, client);
    uint32_t x = rd32(d, 0x9c), y = rd32(d, 0xa0);
    uint32_t ok = X(0x1280, room, 0, 0, x);
    if (!ok) {
        wr32(d, 0x9c, x + 3);
        wr32(d, 0xa0, y + 3);
        return 1;
    }
    uint32_t u = X(0xE1D90, 0, 2, 0xbd, 0, 0, x, y, game, ok);   // retail's fourth logged stack word is the 0x1280 result still in that slot (1), not the room
    if (u) { wr8(d, 0x95, 1); wr32(d, 0x98, rd32(u, 0xc)); }
    return 1;
}
