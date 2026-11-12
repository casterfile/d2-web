// The server end of the in-process game connection.
//
// The retail client is always a client of the game server: in single player the two ends sit in
// one process and packets cross a local queue instead of a socket, uncompressed. This file is that
// queue's server side for the local session: client packets go in through d2_local_d2gs_client and
// become the player's input, and everything the local game sends its client comes out of
// d2_local_d2gs_take in emission order, starting with the greeting and the join sequence.
//
// It owns no game rule. It calls the session's existing entry points and receives each packet the
// game sends; nothing here runs unless d2_local_d2gs_open was called, so a session that does
// not use the connection is untouched.
#include <cstdint>
#include <cstring>
#include <emscripten.h>
#include "../d2game/client_packet_lists.h"

extern "C" {
int d2_session_client_packet(const uint8_t* bytes, int len);
int d2_session_toggle_run(int on);
int d2_session_command_move(int targetX, int targetY);
int d2_session_command_left_skill(uint32_t targetUnitId);
int d2_session_command_right_skill_at(int32_t x, int32_t y);
int d2_session_command_right_skill_hold_at(int32_t x, int32_t y);
int d2_session_command_interact_packet(uint32_t unitType, uint32_t guid);
int d2_session_command_pickup(uint32_t itemUnitId);
int d2_session_command_cursor_item_packet(uint32_t opcode, uint32_t itemId, uint32_t arg);
int d2_session_command_insert_item_packet(uint32_t itemId, uint32_t x, uint32_t y, uint32_t page);
int d2_session_command_item_to_cursor_packet(uint32_t itemId);
int d2_session_command_equip_packet(uint32_t itemId, uint32_t bodyLoc);
int d2_session_command_use_item(uint32_t itemUnitId);
int d2_session_command_use_item_action(uint32_t targetItemId, uint32_t useItemId);
int d2_session_command_add_stat(uint32_t statId, uint32_t count);
int d2_session_command_dropgold(uint32_t amount);
int d2_session_command_swap_weapons();
int d2_session_command_end_npc(uint32_t npcId);
int d2_session_command_npc_response_packet(uint32_t npcId, uint32_t messageId);
int d2_session_command_npc_action_packet(uint32_t npcId, uint32_t action);
int d2_session_command_waypoint(uint32_t waypointId, int32_t levelId);
int d2_session_command_respawn();
int d2_session_command_button(uint32_t button, int32_t gold);
int d2_session_player_id();
int d2_session_player_x();
int d2_session_player_y();
int d2_session_player_act();
void D2WEB_SetPacketSink(void (*sink)(const uint8_t*, uint32_t));
const uint8_t* d2_session_join_stream(uint32_t* length);
uint32_t d2_session_join_load_length();
int d2_session_client_spawned();
int d2_session_client_skills();
int d2_session_client_rooms(uint8_t* out, int capacity);
uint32_t d2_session_game_seed();
int d2_session_game_flags();
int d2_session_add_client(int slot, int classId, const char* name, void (*sink)(int, const uint8_t*, uint32_t));
void d2_session_drop_client(int slot);
int d2_session_client_enter(int slot);
int d2_session_client_unit_id(int slot);
int d2_session_client_packet_slot(int slot, const uint8_t* bytes, int len);
}

namespace {

constexpr int kOutCapacity = 1 << 16;
constexpr int kRefusedCapacity = 256;

enum class Phase : uint8_t { Closed, Greeted, Loaded, InGame };

struct LocalServer {
    Phase phase;
    uint32_t mapSeed;
    uint8_t difficulty, expansion;
    uint8_t classId;
    char name[16];
    int outLength;
    int outDropped;               // bytes that did not fit: a consumer that never takes
    int refusedCount;
    uint32_t accepted;
    int32_t refused[kRefusedCapacity * 2];
    uint8_t out[kOutCapacity];
};
LocalServer g_server;
// Retail applies a client packet in the next server frame, not where it arrives: in-game packets wait here
// until d2_local_d2gs_drain (the top of each d2_session_tick frame) reads them through 0x2E460.
d2g_lists::List g_clientList;
bool g_draining = false;

uint16_t rd16(const uint8_t* p) { return static_cast<uint16_t>(p[0] | (p[1] << 8)); }
uint32_t rd32(const uint8_t* p) { return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) | (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24); }
void wr16(uint8_t* p, uint32_t v) { p[0] = static_cast<uint8_t>(v); p[1] = static_cast<uint8_t>(v >> 8); }
void wr32(uint8_t* p, uint32_t v) { wr16(p, v); wr16(p + 2, v >> 16); }

// Further connections to the same game (slots 1..3): each its own client in the game, its own queue.
constexpr int kSlots = 4;
LocalServer g_extra[kSlots];

void putTo(LocalServer& s, const uint8_t* bytes, int length) {
    if (length <= 0) return;
    if (length > kOutCapacity - s.outLength) { s.outDropped += length; return; }
    std::memcpy(s.out + s.outLength, bytes, static_cast<size_t>(length));
    s.outLength += length;
}
void put(const uint8_t* bytes, int length) { putTo(g_server, bytes, length); }

// Every packet the game sends a further connection's client, routed by that client (from its logon on: the
// load part of the join is sent before it is in game).
void onSlotPacket(int slot, const uint8_t* bytes, uint32_t length) {
    if (slot >= 1 && slot < kSlots && g_extra[slot].phase != Phase::Closed) putTo(g_extra[slot], bytes, static_cast<int>(length));
}
LocalServer* slotServer(int slot) { return slot == 0 ? &g_server : slot > 0 && slot < kSlots ? &g_extra[slot] : nullptr; }

// Every packet the game sends its client while the connection is in game, at the moment it is sent.
void onGamePacket(const uint8_t* bytes, uint32_t length) {
    if (g_server.phase == Phase::InGame) put(bytes, static_cast<int>(length));
}

void refuse(uint8_t opcode, int result) {
    if (g_server.refusedCount >= kRefusedCapacity) return;
    g_server.refused[g_server.refusedCount * 2] = opcode;
    g_server.refused[g_server.refusedCount * 2 + 1] = result;
    ++g_server.refusedCount;
}

// The game's own join bytes, in the order it sent them when the player entered: the load part
// (game flags and the two one-byte markers the client answers with its enter-game byte) ...
void sendLoad() {
    uint32_t burst = 0;
    const uint8_t* join = d2_session_join_stream(&burst);
    const uint32_t load = d2_session_join_load_length();
    if (join && load <= burst) put(join, static_cast<int>(load));
}

// ... then the player's own unit, the unit it plays, the act, the rooms the join walk revealed and
// the units in them.
bool sendPlayer() {
    if (d2_session_player_id() < 0) return false;
    uint32_t burst = 0;
    const uint8_t* join = d2_session_join_stream(&burst);
    const uint32_t load = d2_session_join_load_length();
    if (join && load <= burst) put(join + load, static_cast<int>(burst - load));
    g_server.phase = Phase::InGame;     // the sink forwards from here on
    d2_session_client_skills();         // skill list and both hands' selection, through the sink
    // The room join already sent each unit of those rooms (objects included) into the kept join bytes above; no second pass.
    return true;
}

// One in-game client packet as the player's input. Returns the handler's result, or -1 when the
// local game has no entry for the opcode (logged for d2_local_d2gs_refused).
int dispatch(const uint8_t* p, int n) {
    const auto need = [&](int size) { return n >= size; };
    switch (p[0]) {
    case 0x01: if (!need(5)) return -1; d2_session_toggle_run(0); return d2_session_command_move(rd16(p + 1), rd16(p + 3));
    case 0x03: if (!need(5)) return -1; d2_session_toggle_run(1); return d2_session_command_move(rd16(p + 1), rd16(p + 3));
    case 0x06: case 0x07: if (!need(9)) return -1; return d2_session_command_left_skill(rd32(p + 5));
    case 0x0C: if (!need(5)) return -1; return d2_session_command_right_skill_at(rd16(p + 1), rd16(p + 3));
    case 0x0F: if (!need(5)) return -1; return d2_session_command_right_skill_hold_at(rd16(p + 1), rd16(p + 3));
    case 0x13: if (!need(9)) return -1; return d2_session_command_interact_packet(rd32(p + 1), rd32(p + 5));
    case 0x16: if (!need(13)) return -1; return d2_session_command_pickup(rd32(p + 5));
    case 0x17: if (!need(5)) return -1; return d2_session_command_cursor_item_packet(0x17, rd32(p + 1), 0);
    case 0x18: if (!need(17)) return -1; return d2_session_command_insert_item_packet(rd32(p + 1), rd32(p + 5), rd32(p + 9), rd32(p + 13));
    case 0x19: if (!need(5)) return -1; return d2_session_command_item_to_cursor_packet(rd32(p + 1));
    case 0x1A: if (!need(9)) return -1; return d2_session_command_equip_packet(rd32(p + 1), rd32(p + 5));
    case 0x20: if (!need(13)) return -1; return d2_session_command_use_item(rd32(p + 1));
    case 0x26: if (!need(13)) return -1; return d2_session_command_use_item(rd32(p + 1));
    case 0x27: if (!need(9)) return -1; return d2_session_command_use_item_action(rd32(p + 1), rd32(p + 5));   // [target item][armed item]: D2Game's UseItemAction handler (D2MOO D2Game 0x6FC85B50 -> sub_6FC47470), the identify cursor's press and its cancel
    case 0x30: if (!need(9)) return -1; return d2_session_command_end_npc(rd32(p + 5));
    case 0x31: if (!need(9)) return -1; return d2_session_command_npc_response_packet(rd32(p + 1), rd32(p + 5));
    case 0x38: if (!need(13)) return -1; return d2_session_command_npc_action_packet(rd32(p + 5), rd32(p + 1));
    case 0x3A: if (!need(3)) return -1; return d2_session_command_add_stat(p[1], static_cast<uint32_t>(p[2]) + 1);
    case 0x41: return d2_session_command_respawn();
    case 0x49: if (!need(9)) return -1; return d2_session_command_waypoint(rd32(p + 1), static_cast<int32_t>(rd32(p + 5)));
    case 0x4F: if (!need(7)) return -1; return d2_session_command_button(rd16(p + 1), static_cast<int32_t>((static_cast<uint32_t>(rd16(p + 3)) << 16) | rd16(p + 5)));
    case 0x50: if (!need(9)) return -1; return d2_session_command_dropgold(rd32(p + 5));
    case 0x60: return d2_session_command_swap_weapons();
    default: return d2_session_client_packet(p, n);   // the recovered handlers the session links; -1 for the rest
    }
}

}  // namespace

extern "C" {

// Opens the connection to the live local session and queues the two greeting bytes (the
// uncompressed dialect). The join tells the client the session's own seed and difficulty.
EMSCRIPTEN_KEEPALIVE int d2_local_d2gs_open() {
    std::memset(&g_server, 0, sizeof g_server);
    if (d2_session_player_id() < 0) return 0;
    g_clientList.clear();
    g_server.phase = Phase::Greeted;
    g_server.mapSeed = d2_session_game_seed();
    g_server.difficulty = static_cast<uint8_t>(d2_session_game_flags());
    g_server.expansion = (d2_session_game_flags() >> 8) & 1;
    D2WEB_SetPacketSink(onGamePacket);
    const uint8_t greeting[2] = { 0xAF, 0x00 };
    put(greeting, sizeof greeting);
    return 1;
}

EMSCRIPTEN_KEEPALIVE void d2_local_d2gs_close() {
    g_server.phase = Phase::Closed; g_clientList.clear(); g_server.outLength = 0; D2WEB_SetPacketSink(nullptr);
    for (int s = 1; s < kSlots; ++s) { g_extra[s].phase = Phase::Closed; g_extra[s].outLength = 0; d2_session_drop_client(s); }
}

// One client packet. Returns 1 when the server took it, 0 when it had no entry for it or the game
// refused it (see d2_local_d2gs_refused), -1 when the connection is closed.
EMSCRIPTEN_KEEPALIVE int d2_local_d2gs_client(const uint8_t* bytes, int length) {
    if (g_server.phase == Phase::Closed || !bytes || length <= 0) return -1;
    const uint8_t opcode = bytes[0];
    if (opcode == 0x68) {           // game logon: class at +7, name at +21
        if (length < 37 || g_server.phase != Phase::Greeted) { refuse(opcode, -3); return 0; }
        g_server.classId = bytes[7];
        std::memcpy(g_server.name, bytes + 21, 16);
        g_server.name[15] = 0;
        sendLoad();
        g_server.phase = Phase::Loaded;
        ++g_server.accepted;
        return 1;
    }
    if (opcode == 0x6B) {           // enter game
        if (g_server.phase != Phase::Loaded || !sendPlayer()) { refuse(opcode, -3); return 0; }
        g_server.phase = Phase::InGame;
        d2_session_client_spawned();   // the game's next client update sends the "in game" marker itself
        ++g_server.accepted;
        return 1;
    }
    if (opcode == 0x6D) {           // ping: retail's pong is 0x8F, 12 zero bytes, the server tick (u32 at 13), zeros; the ping's tick is not echoed
        const uint8_t pong[33] = { 0x8F };
        put(pong, sizeof pong);
        ++g_server.accepted;
        return 1;
    }
    if (opcode == 0x69) {           // leave game
        const uint8_t left[1] = { 0x06 };
        put(left, sizeof left);
        g_server.phase = Phase::Closed;
        return 1;
    }
    if (g_server.phase != Phase::InGame) { refuse(opcode, -3); return 0; }
    if (length > d2g_lists::kCapacity || !g_clientList.append(bytes, length)) { refuse(opcode, -2); return 0; }
    return 1;
}

// The server frame's read of the client packet list (D2Game+0x2E460): every packet that arrived since the last
// frame is applied now, in arrival order. A packet whose handler runs a nested frame leaves the rest for the next.
EMSCRIPTEN_KEEPALIVE void d2_local_d2gs_drain() {
    if (g_draining || g_server.phase == Phase::Closed) return;
    g_draining = true;
    d2g_lists::drainFrame(g_clientList, [](const uint8_t* p, int n) {
        const int result = dispatch(p, n);
        if (result < 0) refuse(p[0], result); else ++g_server.accepted;
    });
    g_draining = false;
}

// After the game advanced. Everything the game sent, the join's closing "in game" marker included,
// is already queued by the packet sink; this returns the queued byte count.
EMSCRIPTEN_KEEPALIVE int d2_local_d2gs_pump() { return g_server.outLength; }

EMSCRIPTEN_KEEPALIVE const uint8_t* d2_local_d2gs_out() { return g_server.out; }
// The queued server bytes' length; the queue is empty afterwards (read d2_local_d2gs_out first).
EMSCRIPTEN_KEEPALIVE int d2_local_d2gs_take() { const int n = g_server.outLength; g_server.outLength = 0; return n; }
// Bytes lost on the way to the client since the connection opened (a consumer that never takes).
EMSCRIPTEN_KEEPALIVE int d2_local_d2gs_dropped() { return g_server.outDropped; }
// {opcode, result} per client packet the server did not take, oldest first.
EMSCRIPTEN_KEEPALIVE const int32_t* d2_local_d2gs_refused(int32_t* count) { if (count) *count = g_server.refusedCount; return g_server.refused; }

// Several connections to the one local game. Slot 0 is the connection above (the session's own player);
// slots 1..3 each join the same game as a further player once slot 0 is open. 1 when opened.
EMSCRIPTEN_KEEPALIVE int d2_local_d2gs_open_client(int slot) {
    if (slot == 0) return d2_local_d2gs_open();
    LocalServer* s = slotServer(slot);
    if (!s || g_server.phase == Phase::Closed || s->phase != Phase::Closed) return 0;
    std::memset(s, 0, sizeof *s);
    s->phase = Phase::Greeted;
    const uint8_t greeting[2] = { 0xAF, 0x00 };
    putTo(*s, greeting, sizeof greeting);
    return 1;
}
// One client packet on a slot, as d2_local_d2gs_client: the logon makes that client's player in the game,
// enter-game joins it, and in-game packets go to the game's handlers on that player.
EMSCRIPTEN_KEEPALIVE int d2_local_d2gs_client_slot(int slot, const uint8_t* bytes, int length) {
    if (slot == 0) return d2_local_d2gs_client(bytes, length);
    LocalServer* s = slotServer(slot);
    if (!s || s->phase == Phase::Closed || !bytes || length <= 0) return -1;
    const uint8_t opcode = bytes[0];
    const auto refuseOn = [&](int result) { if (s->refusedCount < kRefusedCapacity) { s->refused[s->refusedCount * 2] = opcode; s->refused[s->refusedCount * 2 + 1] = result; ++s->refusedCount; } return 0; };
    if (opcode == 0x68) {
        if (length < 37 || s->phase != Phase::Greeted) return refuseOn(-3);
        s->classId = bytes[7];
        std::memcpy(s->name, bytes + 21, 16);
        s->name[15] = 0;
        if (!d2_session_add_client(slot, s->classId, s->name, onSlotPacket)) return refuseOn(-3);
        s->phase = Phase::Loaded;
        ++s->accepted;
        return 1;
    }
    if (opcode == 0x6B) {
        if (s->phase != Phase::Loaded || !d2_session_client_enter(slot)) return refuseOn(-3);
        s->phase = Phase::InGame;
        ++s->accepted;
        return 1;
    }
    if (opcode == 0x6D) {
        uint8_t pong[33] = { 0x8F };
        if (length >= 5) std::memcpy(pong + 1, bytes + 1, 4);
        putTo(*s, pong, sizeof pong);
        ++s->accepted;
        return 1;
    }
    if (s->phase != Phase::InGame) return refuseOn(-3);
    const int result = d2_session_client_packet_slot(slot, bytes, length);
    if (result != 0) return refuseOn(result < 0 ? result : -2);
    ++s->accepted;
    return 1;
}
EMSCRIPTEN_KEEPALIVE const uint8_t* d2_local_d2gs_out_slot(int slot) { LocalServer* s = slotServer(slot); return s ? s->out : nullptr; }
EMSCRIPTEN_KEEPALIVE int d2_local_d2gs_take_slot(int slot) { LocalServer* s = slotServer(slot); if (!s) return 0; const int n = s->outLength; s->outLength = 0; return n; }
// The unit id of the player a slot plays (-1 before its logon).
EMSCRIPTEN_KEEPALIVE int d2_local_d2gs_unit_id(int slot) { return slot == 0 ? d2_session_player_id() : d2_session_client_unit_id(slot); }

}  // extern "C"
