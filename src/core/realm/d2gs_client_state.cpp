// Client-side D2GS state. Packet schemas stay in d2gs_client_packets.cpp; server GUIDs own their
// render and path records here and the offline game session remains separate.
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <cstdlib>
#include <D2Seed.h>
#include <D2StatList.h>
#include <Units/Missile.h>
#include <DataTbls/MissilesTbls.h>
#include <cstring>
#include <cstdio>
#include <string>
#include <emscripten.h>
#include <utility>
#include <vector>

#include <D2DataTbls.h>
#include <D2Collision.h>
#include <D2Dungeon.h>
#include <DataTbls/ItemsTbls.h>
#include <DataTbls/MonsterTbls.h>
#include <D2Items.h>
#include <D2QuestRecord.h>
#include <D2Monsters.h>
#include <D2States.h>
#include <D2StatList.h>
#include <D2Skills.h>
#include <D2Skills.h>
#include <D2Inventory.h>
#include <DataTbls/SkillsIds.h>
#include <Path/Path.h>
#include <Path/PathMisc.h>
#include <Path/Step.h>
#include <Units/Units.h>
#include <Units/Player.h>
#include <Units/UnitRoom.h>
#include <Drlg/D2DrlgDrlg.h>
#include "../d2client/inv_weapon_hands.h"
#include "../d2client/character_screen.h"
#include "client_refusal_sound.h"
#include "skill_update_packet.h"
#include "client_unit_sound.h"
#include "../d2client/price_line_46230.h"
#include "../d2client/realm_skill_start.h"
#include "../d2client/realm_skill_do.h"
#include "d2gs_client_motion.h"
#include "d2gs_client_player_speed.h"
#include "d2gs_client_player_skills.h"
#include "d2gs_client_object.h"
#include "d2gs_client_snapshot.h"
#include "d2gs_client_target_relations.h"
#include "d2gs_client_owned_item.h"
#include "d2gs_client_game_packets.h"
#include "d2gs_client_missile_packet.h"
#include "d2gs_client_unit_resources.h"
#include "client_fireball_hit.h"
#include "client_missile_request.h"
#include "client_missile_hit_child.h"
#include "client_missile_hit_ring.h"
#include "client_missile_sound.h"
#include "d2gs_client_player_motion.h"
#include "d2gs_client_player_death.h"
#include "d2gs_client_player_stop.h"
#include "d2gs_client_monster_death_events.h"
#include "cast_target_relation.h"
#include "client_skill_fx.h"
#include <DataTbls/OverlayTbls.h>
#include "d2gs_client_monster_death_clock.h"
#include "d2gs_client_pkt_route.h"
#include "d2gs_client_combat.h"
#include "d2gs_client_unit_fx.h"
#include "d2gs_client_quests.h"
#include "d2gs_client_warps.h"
#include "d2gs_client_state_stats.h"
#include "client_pets.h"
#include "client_monster_name.h"
#include "../d2client/d2client_afa90.h"
#include "../d2client/d2client_pkt_env.h"
#include <D2Environment.h>
#include "../d2client/d2client_unit_ports.h"

extern "C" uint32_t d2c_port_af5f0(uint32_t val, const uint8_t* rec,
    uint32_t (*f2a750)(uint32_t ecx, uint32_t edx, uint32_t* blk, uint32_t one));

extern "C" {
int d2_realm_client_items_feed(const uint8_t* packet, int length, uint32_t localPlayerGuid);
int d2_realm_client_items_drop(uint32_t guid);
int d2_realm_client_items_drop_cursor(uint32_t ownerType, uint32_t ownerGuid);
void d2_realm_client_items_clear();
}

struct D2RealmPlayerAssign {
    uint32_t guid;
    uint8_t classId;
    uint8_t name[16];
    uint16_t x;
    uint16_t y;
};

struct D2RealmPlayerReassign {
    uint32_t guid;
    uint16_t x;
    uint16_t y;
    uint8_t unitType;
    uint8_t value;
    uint8_t reserved[2];
};

struct D2RealmGameChat {
    uint32_t version, size, messageType, unknown2, subType, unitGuid, unknown8, color, nameLength, messageLength;
    uint8_t name[16];
    uint8_t message[256];
};

static_assert(offsetof(D2RealmPlayerReassign, guid) == 0);
static_assert(offsetof(D2RealmPlayerReassign, x) == 4);
static_assert(offsetof(D2RealmPlayerReassign, y) == 6);
static_assert(offsetof(D2RealmPlayerReassign, unitType) == 8);
static_assert(offsetof(D2RealmPlayerReassign, value) == 9);
static_assert(sizeof(D2RealmPlayerReassign) == 12);
static_assert(offsetof(D2RealmGameChat, name) == 40);
static_assert(offsetof(D2RealmGameChat, message) == 56);
static_assert(sizeof(D2RealmGameChat) == 312);

struct D2RealmClientPlayerState {
    uint32_t version;
    uint32_t size;
    uint32_t active;
    uint32_t guid;
    uint32_t x;
    uint32_t y;
    uint8_t classId;
    uint8_t reassignValue;
    uint8_t reserved[2];
    uint32_t reassignments;
    uint8_t name[16];
    uint32_t clientRoomActive;
    uint32_t clientRoomLevel;
    uint32_t clientRoomTileX;
    uint32_t clientRoomTileY;
    uint32_t cunitInitSeed;
    uint32_t cunitInitSeedValid;
    uint32_t cunitRngLow;
    uint32_t cunitRngHigh;
    uint32_t cunitRngValid;
    uint32_t initialMode;
    uint32_t initialDirection;
};

static_assert(offsetof(D2RealmClientPlayerState, active) == 8);
static_assert(offsetof(D2RealmClientPlayerState, guid) == 12);
static_assert(offsetof(D2RealmClientPlayerState, x) == 16);
static_assert(offsetof(D2RealmClientPlayerState, y) == 20);
static_assert(offsetof(D2RealmClientPlayerState, classId) == 24);
static_assert(offsetof(D2RealmClientPlayerState, reassignValue) == 25);
static_assert(offsetof(D2RealmClientPlayerState, reassignments) == 28);
static_assert(offsetof(D2RealmClientPlayerState, name) == 32);
static_assert(offsetof(D2RealmClientPlayerState, clientRoomActive) == 48);
static_assert(offsetof(D2RealmClientPlayerState, clientRoomLevel) == 52);
static_assert(offsetof(D2RealmClientPlayerState, clientRoomTileX) == 56);
static_assert(offsetof(D2RealmClientPlayerState, clientRoomTileY) == 60);
static_assert(offsetof(D2RealmClientPlayerState, cunitInitSeed) == 64);
static_assert(offsetof(D2RealmClientPlayerState, cunitInitSeedValid) == 68);
static_assert(offsetof(D2RealmClientPlayerState, cunitRngLow) == 72);
static_assert(offsetof(D2RealmClientPlayerState, cunitRngHigh) == 76);
static_assert(offsetof(D2RealmClientPlayerState, cunitRngValid) == 80);
static_assert(offsetof(D2RealmClientPlayerState, initialMode) == 84);
static_assert(offsetof(D2RealmClientPlayerState, initialDirection) == 88);
static_assert(sizeof(D2RealmClientPlayerState) == 92);

// Keep the renderer/API record at its established 92-byte ABI. The canonical roster entry owns
// the one client path unit associated with that server GUID; there is no parallel motion map.
struct D2RealmClientPlayerEntry {
    D2RealmClientPlayerState view{};
    D2UnitStrc* motionUnit = nullptr;
    uint8_t motionDirection = 0;
    struct MotionView {
        uint32_t version = 1;
        uint32_t size = 48;
        uint32_t active = 0;
        uint32_t mode = 0;
        uint32_t direction = 0;
        uint32_t framePrecise = 0;
        uint32_t frameCountPrecise = 0;
        uint32_t animSpeed = 0;
        uint32_t positionHintX = 0;
        uint32_t positionHintY = 0;
        uint32_t seqMode = 0;   // PLRMODE_SEQUENCE: the current frame's mode (dwSeqMode), else 0
        uint32_t seqFrame = 0;  // PLRMODE_SEQUENCE: the current frame index (nSeqCurrentFramePrecise >> 8), else 0
    } motionView{};
};
static_assert(sizeof(D2RealmClientPlayerEntry::MotionView) == 48);

// The 0x03 packet's final dword is retained raw until its meaning is fully resolved for 1.13c.
struct D2RealmClientWorldState {
    uint32_t version;
    uint32_t size;
    uint32_t hasLoadAct;
    uint32_t act;
    uint32_t mapSeed;
    uint32_t area;
    uint32_t rawLoadActTail;
    uint32_t loadCompleteEvent;
    uint32_t loadActCount;
    uint32_t unloadCompleteEvent;
    uint32_t hasMapReveal;
    uint32_t mapLevel;
    uint32_t mapX;
    uint32_t mapY;
    uint32_t mapRevealCount;
    uint32_t difficulty;
    uint32_t mapHideCount;
    uint32_t mapRevision;
};

static_assert(offsetof(D2RealmClientWorldState, hasLoadAct) == 8);
static_assert(offsetof(D2RealmClientWorldState, act) == 12);
static_assert(offsetof(D2RealmClientWorldState, mapSeed) == 16);
static_assert(offsetof(D2RealmClientWorldState, area) == 20);
static_assert(offsetof(D2RealmClientWorldState, rawLoadActTail) == 24);
static_assert(offsetof(D2RealmClientWorldState, loadCompleteEvent) == 28);
static_assert(offsetof(D2RealmClientWorldState, loadActCount) == 32);
static_assert(offsetof(D2RealmClientWorldState, unloadCompleteEvent) == 36);
static_assert(offsetof(D2RealmClientWorldState, hasMapReveal) == 40);
static_assert(offsetof(D2RealmClientWorldState, mapLevel) == 44);
static_assert(offsetof(D2RealmClientWorldState, mapX) == 48);
static_assert(offsetof(D2RealmClientWorldState, mapY) == 52);
static_assert(offsetof(D2RealmClientWorldState, mapRevealCount) == 56);
static_assert(offsetof(D2RealmClientWorldState, difficulty) == 60);
static_assert(offsetof(D2RealmClientWorldState, mapHideCount) == 64);
static_assert(offsetof(D2RealmClientWorldState, mapRevision) == 68);
static_assert(sizeof(D2RealmClientWorldState) == 72);

// Renderer-facing remote unit projection. The server stream owns these records; motionUnit is a
// client-side DynamicPath carrier, not an offline D2Game session actor with local AI.
struct D2RealmClientMonsterView {
    uint32_t version;
    uint32_t size;
    uint32_t guid;
    uint32_t classId;
    uint32_t code;
    uint32_t mode;
    uint32_t framePrecise;
    uint32_t frameCountPrecise;
    uint32_t componentCodes[16];
    uint16_t animSpeed;
    uint8_t direction;
    uint8_t reserved;
    uint32_t x;
    uint32_t y;
    uint32_t hpPct;
    uint32_t typeFlags;
};

static_assert(offsetof(D2RealmClientMonsterView, guid) == 8);
static_assert(offsetof(D2RealmClientMonsterView, componentCodes) == 32);
static_assert(offsetof(D2RealmClientMonsterView, animSpeed) == 96);
static_assert(offsetof(D2RealmClientMonsterView, x) == 100);
static_assert(offsetof(D2RealmClientMonsterView, typeFlags) == 112);
static_assert(sizeof(D2RealmClientMonsterView) == 116);

struct D2RealmClientMonsterStat {
    uint16_t statId;
    int32_t parameter;
    int32_t value;
};

// Narrow diagnostic snapshot for parser verification. Kept separate from the stable renderer
// record so exposing optional packet sections does not silently widen its ABI.
struct D2RealmClientMonsterAux {
    uint32_t version;
    uint32_t size;
    uint32_t guid;
    uint32_t hasExtra31;
    uint32_t extra31;
    uint32_t hasStats;
    uint32_t statCount;
    uint32_t firstStatId;
    int32_t firstStatParameter;
    int32_t firstStatValue;
    uint32_t unitFlags;
};
static_assert(offsetof(D2RealmClientMonsterAux, guid) == 8);
static_assert(offsetof(D2RealmClientMonsterAux, extra31) == 16);
static_assert(offsetof(D2RealmClientMonsterAux, firstStatValue) == 36);
static_assert(offsetof(D2RealmClientMonsterAux, unitFlags) == 40);
static_assert(sizeof(D2RealmClientMonsterAux) == 44);

// Read-only route snapshot used by browser fixture assertions to distinguish failed replanning
// from a tick that simply has no path to advance.
struct D2RealmClientMotionStatus {
    uint32_t version;
    uint32_t size;
    uint32_t guid;
    uint32_t attached;
    uint32_t pathFlags;
    uint32_t pathType;
    uint32_t pathPoints;
    uint32_t currentPoint;
    uint32_t startX;
    uint32_t startY;
    uint32_t targetX;
    uint32_t targetY;
    uint32_t mode;
    uint32_t framePrecise;
    uint32_t frameCountPrecise;
    uint32_t animSpeed;
    int32_t lastPlanResult;
    int32_t lastTickResult;
};
static_assert(offsetof(D2RealmClientMotionStatus, guid) == 8);
static_assert(offsetof(D2RealmClientMotionStatus, pathFlags) == 16);
static_assert(offsetof(D2RealmClientMotionStatus, lastPlanResult) == 64);
static_assert(sizeof(D2RealmClientMotionStatus) == 72);

struct D2RealmClientMotionStatusV2 {
    D2RealmClientMotionStatus base;
    uint32_t roomLevel;
    uint32_t roomTileX;
    uint32_t roomTileY;
    uint32_t roomTileWidth;
    uint32_t roomTileHeight;
    uint32_t clientSightRefs;
};
static_assert(offsetof(D2RealmClientMotionStatusV2, roomLevel) == 72);
static_assert(offsetof(D2RealmClientMotionStatusV2, clientSightRefs) == 92);
static_assert(sizeof(D2RealmClientMotionStatusV2) == 96);

struct D2RealmClientMotionStatusV3 {
    D2RealmClientMotionStatusV2 base;
    uint32_t currentDirection;
    uint32_t targetDirection;
    uint32_t directionDelta;
    uint32_t directionFlags;
};
static_assert(offsetof(D2RealmClientMotionStatusV3, currentDirection) == 96);
static_assert(offsetof(D2RealmClientMotionStatusV3, directionFlags) == 108);
static_assert(sizeof(D2RealmClientMotionStatusV3) == 112);

struct D2RealmClientMonster {
    D2RealmClientMonsterView view{};
    uint8_t rawPrefix;
    bool hasExtra31 = false;
    bool hasStats = false;
    int16_t superUniqueIndex;
    uint16_t nameId;
    uint32_t ownerGuid;
    uint32_t extra31;
    std::vector<uint8_t> nameBytes;
    std::vector<D2RealmClientMonsterStat> stats;
    D2UnitStrc* motionUnit = nullptr;
    int32_t lastPlanResult = 0;
    int32_t lastTickResult = 0;
};

namespace {
constexpr int kPendingCapacity = 131072;
uint8_t g_pending[kPendingCapacity];
int g_pendingLength;
D2RealmClientPlayerState g_player;
std::vector<D2RealmClientPlayerEntry> g_players;
int g_selectedPlayer = -1;
static void g_localQueuedReset();
D2RealmClientWorldState g_world;
D2RealmGameChat g_gameChat{1, sizeof(D2RealmGameChat)};
uint32_t g_gameChatCount;
uint32_t g_clientDifficulty;
std::vector<D2RealmClientMonster> g_monsters;
struct D2RealmClientObjectView {
    uint32_t unitType, guid, classId, x, y, mode, interact;
};
static_assert(sizeof(D2RealmClientObjectView) == 28);
std::vector<D2RealmClientObjectView> g_objects;
// The client units of the assigned objects, found by guid. They are kept apart from the player and monster
// lookup: an object's path is a static one, and that lookup's callers walk a dynamic path.
// Object activations (a shrine used by a player) not yet taken by the shell: the sound it plays at the player.
std::vector<D2gsClientObjectEvent> g_objectEvents;
std::vector<D2RealmClientMonsterView> g_monsterViews;
std::vector<d2::realm::MonsterDeathModeEvent> g_monsterDeathModeEvents;
std::vector<d2::realm::MonsterDeathModeEvent> g_monsterDeathModeEventView;
uint32_t g_nextMonsterDeathModeSequence = 1;

void clear_monster_death_mode_events() {
    g_monsterDeathModeEvents.clear();
    g_monsterDeathModeEventView.clear();
}

void reset_monster_death_mode_events() {
    clear_monster_death_mode_events();
    g_nextMonsterDeathModeSequence = 1;
}

class D2RealmBitReader {
public:
    D2RealmBitReader(const uint8_t* bytes, int length) : bytes_(bytes), bits_(length > 0 ? length * 8 : 0) {}
    bool read(uint32_t count, uint32_t& value) {
        if (count > 32 || position_ > bits_ || count > bits_ - position_) return false;
        value = 0;
        for (uint32_t i = 0; i < count; ++i) {
            value |= static_cast<uint32_t>((bytes_[position_ >> 3] >> (position_ & 7)) & 1u) << i;
            ++position_;
        }
        return true;
    }
    bool readSigned(uint32_t count, int32_t& value) {
        if (count == 0 || count > 32) return false;
        uint32_t raw = 0;
        if (!read(count, raw)) return false;
        if (count == 32) value = static_cast<int32_t>(raw);
        else if (raw & (1u << (count - 1))) value = static_cast<int32_t>(raw | (~0u << count));
        else value = static_cast<int32_t>(raw);
        return true;
    }
    uint32_t remaining() const { return position_ <= bits_ ? bits_ - position_ : 0; }
private:
    const uint8_t* bytes_;
    uint32_t bits_;
    uint32_t position_ = 0;
};

uint32_t componentWidth(uint8_t count) {
    if (count < 3) return 1; // Exact native 1.13c rule, including counts 0 and 1.
    uint32_t value = static_cast<uint32_t>(count) - 1;
    uint32_t width = 0;
    while (value) { ++width; value >>= 1; }
    return width;
}

void clear_player() {
    std::memset(&g_player, 0, sizeof(g_player));
    g_player.version = 5;
    g_player.size = sizeof(g_player);
}

static d2::realm::CorpseRoster g_corpses;

static void destroy_player_owned_items(D2UnitStrc* owner) {
    // Free item CUnits before the inventory/resource owner is released; INVENTORY_FreeInventory
    // only detaches items from the inventory.
    d2_realm_client_items_forget_owner(owner);
}

void clear_players() {
    for (auto& player : g_players) {
        if (!player.motionUnit) continue;
        d2_realm_client_motion_detach(player.motionUnit);
        destroy_player_owned_items(player.motionUnit);
        d2_realm_client_unit_resources_destroy(player.motionUnit);
        UNITS_FreeUnit(player.motionUnit);
        player.motionUnit = nullptr;
    }
    g_players.clear();
    d2_realm_client_items_reset();
    g_selectedPlayer = -1;
    g_localQueuedReset();
    g_corpses.clear();
    clear_player();
}

// The selected player's room, the room the client environment calls take.
D2ActiveRoomStrc* selected_player_room() {
    if (g_selectedPlayer < 0 || static_cast<size_t>(g_selectedPlayer) >= g_players.size()) return nullptr;
    D2UnitStrc* unit = g_players[g_selectedPlayer].motionUnit;
    return unit && unit->pDynamicPath ? unit->pDynamicPath->pRoom : nullptr;
}

uint32_t selected_player_guid() {
    if (g_selectedPlayer < 0 || static_cast<size_t>(g_selectedPlayer) >= g_players.size()) return 0xFFFFFFFFu;
    return g_players[g_selectedPlayer].view.guid;
}

void store_selected_player() {
    if (g_selectedPlayer >= 0 && static_cast<size_t>(g_selectedPlayer) < g_players.size())
        g_players[g_selectedPlayer].view = g_player;
}

void clear_world() {
    std::memset(&g_world, 0, sizeof(g_world));
    g_world.version = 2;
    g_world.size = sizeof(g_world);
}

void clear_game_chat() {
    std::memset(&g_gameChat, 0, sizeof(g_gameChat));
    g_gameChat.version = 1;
    g_gameChat.size = sizeof(g_gameChat);
    g_gameChatCount = 0;
}
}

extern "C" {
int d2_realm_d2gs_parse_gamechat(const uint8_t*, int, D2RealmGameChat*);
int d2_realm_d2gs_parse_player_assign(const uint8_t*, int, D2RealmPlayerAssign*);
int d2_realm_d2gs_parse_player_reassign(const uint8_t*, int, D2RealmPlayerReassign*);
int d2_realm_d2gs_parse_player_move(const uint8_t*, int, D2RealmPlayerMove*);
int d2_realm_client_apply_player_move(D2UnitStrc*, const D2RealmPlayerMove*);
int d2_realm_d2gs_packet_size(const uint8_t*, int);
void d2_client_lightmap_set_den_quest_avail(int);
int d2_drlg_init();
int d2_drlg_client_act_begin(uint32_t, int, int, int);
D2DrlgActStrc* d2_drlg_client_act();
void d2_drlg_act_end();
int d2_drlg_client_map_reveal(int, int, int, int, int, int, int);
int d2_drlg_client_map_hide(int, int, int, int, int, int, int);
int d2_drlg_client_render_level();
int d2_drlg_client_select_render_level(int);
int d2_drlg_client_level_at(int, int);
int d2_drlg_client_player_context(int, int, int, uint32_t*, uint32_t*, uint32_t*, uint32_t*);
int d2_drlg_client_unit_context(int, int, int, uint32_t*, uint32_t*, uint32_t*, uint32_t*);
int d2_session_client_cunit_seed_init(D2SeedStrc*, uint32_t*, int, int, int);
int d2_drlg_client_tile_count();
D2DrlgActStrc* d2_drlg_client_act();
D2ActiveRoomStrc* d2_drlg_client_active_room(int, int);
int d2_drlg_client_room_freed_by_hide(int, int, int, int, int, int, int, void*);
// Object units live in the client's object table and on their room's unit list (d2gs_client_object.cpp).
static D2UnitStrc* find_object_unit(uint32_t guid) { return d2gs_client_object_find(guid); }
static void destroy_object_unit(uint32_t guid) { d2gs_client_object_destroy(d2gs_client_object_find(guid)); }
// Every caller has ended the act first, so the rooms, their unit lists and collision grids are already gone.
static void clear_objects() {
    while (D2UnitStrc* unit = d2gs_client_object_next(nullptr)) {
        d2gs_client_object_unplace(unit, false);
        d2gs_client_object_destroy(unit);
    }
    g_objects.clear();
    g_objectEvents.clear();
}
// An object assigned before its room was revealed takes the room, and its footprint, once it is there.
static void place_unroomed_objects() {
    for (D2UnitStrc* unit = d2gs_client_object_next(nullptr); unit; unit = d2gs_client_object_next(unit)) {
        if (UNITS_GetRoom(unit)) continue;
        const int x = UNITS_GetXPosition(unit), y = UNITS_GetYPosition(unit);
        if (D2ActiveRoomStrc* room = d2_drlg_client_active_room(x, y)) d2gs_client_object_place(unit, room, x, y);
    }
}
static void sync_object_view(const D2UnitStrc* unit) {
    for (auto& object : g_objects)
        if (object.unitType == UNIT_OBJECT && object.guid == unit->dwUnitId) { object.mode = unit->dwAnimMode; return; }
}

bool d2_realm_client_motion_attach(D2UnitStrc* unit, int x, int y) {
    if (!unit) return false;
    if (unit->pDynamicPath) return true;
    D2DrlgActStrc* act = d2_drlg_client_act();
    D2ActiveRoomStrc* room = d2_drlg_client_active_room(x, y);
    if (!act || !room) return false;
    unit->nAct = act->nAct;
    unit->pDrlgAct = act;
    unit->dwFlags |= UNITFLAG_ISCLIENTUNIT;
    PATH_AllocDynamicPath(nullptr, room, x, y, unit, FALSE);
    return unit->pDynamicPath != nullptr;
}

static int d2_realm_client_unit_set_mode(D2UnitStrc* unit, uint32_t mode) {
    if (!unit) return -1;
    d2_realm_client_unit_enter_mode(unit, mode, unit->dwUnitType == UNIT_PLAYER,
        d2::realm::monsterAnimationModeNeedsSetup(unit->dwAnimMode, mode, unit->dwFrameCountPrecise),
        [](D2UnitStrc* u) { STATLIST_DeactivateTemporaryStates(u); },
        [](D2UnitStrc* u) { UNITS_SetAnimStartFrame(u); });
    return 1;
}

int d2_realm_client_motion_set_mode(D2UnitStrc* unit, uint32_t mode) {
    if (!unit || !unit->pDynamicPath) return -1;
    return d2_realm_client_unit_set_mode(unit, mode);
}

EMSCRIPTEN_KEEPALIVE uint32_t d2_realm_client_motion_refusal_mode(uint32_t unitType, uint32_t currentMode) {
    // FUN_6FAFDC30's native guard: monster type, mode 1..15, except MONMODE_DEAD (12).
    return unitType == UNIT_MONSTER && currentMode > MONMODE_DEATH && currentMode < NUMBER_OF_MONMODES &&
        currentMode != MONMODE_DEAD ? MONMODE_NEUTRAL : currentMode;
}

int d2_realm_client_motion_set_target(D2UnitStrc* unit, int x, int y) {
    if (!unit || !unit->pDynamicPath) return -1;
    D2COMMON_10170_PathSetTargetPos(unit->pDynamicPath, x, y);
    return D2Common_10142(unit->pDynamicPath, unit, FALSE); // Local D2Common ordinal 10334.
}

static uint8_t d2_realm_client_motion_step_facing(uint8_t current, uint8_t target,
        uint8_t delta, uint32_t flags) {
    // Re-derived from local D2Common 1.13c #10603 (VA 0x6FD84D70). Flag 0x40 holds
    // the current facing. Otherwise take the signed-byte step modulo 64 and snap only
    // when the native overshoot predicates show the target was crossed.
    if (current == target || (flags & PATH_UNKNOWN_FLAG_0x00040)) return current;
    current = static_cast<uint8_t>((static_cast<int8_t>(current) + delta) & 0x3f);
    const uint8_t remaining = static_cast<uint8_t>((static_cast<int8_t>(current) -
        static_cast<int8_t>(target)) & 0x3f);
    if (remaining > 31 && delta > 63) current = target;
    else if (remaining < 31 && delta < 63) current = target;
    return current;
}

EMSCRIPTEN_KEEPALIVE int d2_realm_client_motion_facing_step(uint32_t current, uint32_t target,
        uint32_t delta, uint32_t flags) {
    if (current > 255 || target > 255 || delta > 255) return -1;
    return d2_realm_client_motion_step_facing(static_cast<uint8_t>(current),
        static_cast<uint8_t>(target), static_cast<uint8_t>(delta), flags);
}

// The first column of the client's per-mode update tables: 1 when the mode's frame update steps the unit
// along its path, 2 when only a path that asks for it does, 0 when the unit stays where it is. A monster
// whose class marks the mode as one it moves in reads the second table; two class and mode pairs are fixed.
static int unit_mode_move_kind(const D2UnitStrc* unit) {
    static const uint8_t kPlayer[20] = { 0, 0, 1, 1, 0, 0, 1, 2, 2, 0, 2, 2, 2, 0, 2, 2, 2, 0, 2, 1 };
    static const uint8_t kMonster[16] = { 0, 0, 1, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1 };
    static const uint8_t kMonsterMoving[16] = { 1, 0, 0, 0, 1, 1, 2, 1, 1, 1, 0, 1, 0, 0, 0, 0 };
    const uint32_t mode = unit->dwAnimMode;
    if (unit->dwUnitType == UNIT_PLAYER) return mode < 20 ? kPlayer[mode] : 0;
    if (unit->dwUnitType != UNIT_MONSTER || mode >= 16) return 0;
    const int classId = unit->dwClassId;
    if ((classId == 0xf3 || classId == 0x14d) && mode == 11) return 2;
    if ((classId == 0x21f || classId == 0x220 || classId == 0x23a) && mode == 10) return 1;
    D2MonStats2Txt* stats2 = UNITS_GetMonStats2TxtRecordFromMonsterId(classId);
    if (stats2 && mode && (stats2->nModeEnabledWhenMovingFlags[mode >> 3] & (1u << (mode & 7)))) return kMonsterMoving[mode];
    return kMonster[mode];
}

// One frame of a unit: the path step when `move` (its result: 0 when the path has run out), the animation
// frame and the turn toward the facing asked for.
int d2_realm_client_motion_advance_unit(D2UnitStrc* unit, bool move) {
    if (!unit || !unit->pDynamicPath) return -1;
    if (!unit->dwFrameCountPrecise) UNITS_SetAnimStartFrame(unit);
    const int moving = move ? D2Common_10226(unit, 1024) : 0;
    if (!d2_realm_client_apply_monster_death_terminal_clock(*unit,
            [](D2UnitStrc* candidate) { return UNITS_IsAtEndOfFrameCycle(candidate) != 0; })) {
        UNITS_StopSequence(unit);
    }
    D2DynamicPathStrc* path = unit->pDynamicPath;
    path->nDirection = d2_realm_client_motion_step_facing(path->nDirection, path->nNewDirection,
        path->nDiffDirection, path->dwFlags);
    return moving;
}

void d2_realm_client_motion_detach(D2UnitStrc* unit) {
    if (!unit || !unit->pDynamicPath) return;
    PATH_RemoveCollisionFootprintForUnit(unit, TRUE);
    UNITROOM_RemoveUnitFromRoom(unit);
    PATH_FreeDynamicPath(nullptr, unit->pDynamicPath);
    unit->pDynamicPath = nullptr;
}

int d2_realm_client_motion_apply_player_move(D2UnitStrc* unit, uint8_t action, uint8_t opaque,
        uint16_t targetX, uint16_t targetY) {
    if (!unit || unit->dwUnitType != UNIT_PLAYER || !unit->pDynamicPath) return -1;
    (void)opaque;
    if (action != 0x01 && action != 0x17) return 0;
    // FAC9830 selects both its success mode and its shared planner-refusal fallback from the
    // current room. It plans first; only a successful route receives the requested walk/run mode.
    D2ActiveRoomStrc* room = UNITS_GetRoom(unit);
    if (!room) return -1;
    const bool inTown = DUNGEON_IsRoomInTown(room) != FALSE;
    const uint32_t mode = action == 0x17 ? PLRMODE_RUN :
        (inTown ? PLRMODE_TOWNWALK : PLRMODE_WALK);
    const uint32_t refusalMode = inTown ? PLRMODE_TOWNNEUTRAL : PLRMODE_NEUTRAL;
    // D2Game 1.13c +0x78610 resets the path to its previous type (and velocity) before it plans a walk or run.
    PATH_ResetToPreviousType(unit->pDynamicPath);
    D2COMMON_10170_PathSetTargetPos(unit->pDynamicPath, targetX, targetY);
    if (D2Common_10142(unit->pDynamicPath, unit, FALSE) <= 0) {
        // The native failed-route branch goes through FUN_6FAC9710 with EAX=1/5. That helper
        // clears unit flag 0x40 and sets the normal/town neutral mode; it does not keep the old
        // mode. It is shared by failed action-1 and action-0x17 route attempts.
        unit->dwFlags &= ~0x40u;
        d2_realm_client_motion_set_mode(unit, refusalMode);
        return 0;
    }
    // FUN_6FADA6C0 clears this same flag before its successful mode transition.
    unit->dwFlags &= ~0x40u;
    return d2_realm_client_motion_set_mode(unit, mode) > 0 ? 1 : -1;
}

static void clear_target_relations_to(D2UnitStrc* target);

static void destroy_monster_motion(D2RealmClientMonster& monster) {
    D2UnitStrc* unit = monster.motionUnit;
    if (!unit) return;
    clear_target_relations_to(unit);
    d2_realm_client_motion_detach(unit);
    d2_realm_client_unit_resources_destroy(unit);
    UNITS_FreeUnit(unit);
    monster.motionUnit = nullptr;
}

static void destroy_all_monster_motion() {
    for (auto& monster : g_monsters) destroy_monster_motion(monster);
}

static void destroy_player_motion(D2RealmClientPlayerEntry& player);

static void refresh_monster_motion_view(D2RealmClientMonster& monster);

// An assigned monster's life is the record's byte against a fixed maximum (1.13c D2Client+0xAE420).
// The local player's own hireling keeps the life its own records set.
static bool is_own_hireling(uint32_t guid);
static void apply_assigned_life(D2UnitStrc* unit, uint32_t lifeByte) {
    if (!unit || !unit->pStatListEx || is_own_hireling(unit->dwUnitId)) return;
    STATLIST_SetUnitStat(unit, 7, static_cast<int>(d2::realm::kAssignedMaxLife), 0);
    STATLIST_SetUnitStat(unit, 6, static_cast<int>(d2::realm::assignedLife(static_cast<uint8_t>(lifeByte))), 0);
}

static bool create_monster_motion(D2RealmClientMonster& monster) {
    D2MonStatsTxt* stats = DATATBLS_GetMonStatsTxtRecord(static_cast<int>(monster.view.classId));
    if (!stats) return false;
    D2UnitStrc* unit = monster.motionUnit;
    if (!unit) {
        D2MonStats2Txt* stats2 = UNITS_GetMonStats2TxtRecordFromMonsterId(static_cast<int>(monster.view.classId));
        if (!stats2) return false;
        unit = UNITS_AllocUnit(nullptr, UNIT_MONSTER);
        if (!unit) return false;
        unit->dwClassId = static_cast<int32_t>(monster.view.classId);
        unit->dwUnitId = monster.view.guid;
        unit->dwAnimMode = MONMODE_NEUTRAL;
        // D2Client 1.13c FUN_6FAFD160 always marks a new monster as a valid target, then
        // mirrors MonStats2.isAtt into CANBEATTACKED. The collision predicate requires both.
        unit->dwFlags |= UNITFLAG_ISVALIDTARGET;
        if (stats2->dwFlags & MONSTATS2FLAG_ISATT) unit->dwFlags |= UNITFLAG_CANBEATTACKED;
        else unit->dwFlags &= ~UNITFLAG_CANBEATTACKED;
        if (sgptDataTables && sgptDataTables->nStatesTxtRecordCount > 0)
            STATLIST_AllocStatListEx(unit, 0, nullptr, nullptr);
        // the monster's base walk speed, as D2Game's monster init sets it (Monster.cpp STAT_VELOCITYPERCENT 75); the client unit needs it to walk at the server's pace
        STATLIST_SetUnitStat(unit, STAT_VELOCITYPERCENT, 75, 0);
        apply_assigned_life(unit, monster.view.hpPct);
        if (!d2_realm_client_unit_resources_init(unit)) {
            d2_realm_client_unit_resources_destroy(unit);
            UNITS_FreeUnit(unit);
            return false;
        }
        monster.motionUnit = unit;
    }
    // Initialize the client-owned frame state even before MapReveal attaches a dynamic path.
    // This is the CUnit state Common #11090 initializes when D2Client receives a mode update.
    if (unit->dwAnimMode != monster.view.mode || !unit->dwFrameCountPrecise)
        d2_realm_client_unit_set_mode(unit, monster.view.mode);
    // The GUID-owned CUnit can receive state packets before MapReveal activates its room.
    // Keep the canonical unit/stat lists alive and attach its path when the room becomes active.
    if (!unit->pDynamicPath && !d2_realm_client_motion_attach(unit,
            static_cast<int>(monster.view.x), static_cast<int>(monster.view.y))) {
        refresh_monster_motion_view(monster);
        return true;
    }
    refresh_monster_motion_view(monster);
    return true;
}

static void refresh_monster_motion_view(D2RealmClientMonster& monster) {
    if (!monster.motionUnit) return;
    monster.view.mode = monster.motionUnit->dwAnimMode;
    monster.view.framePrecise = monster.motionUnit->nSeqCurrentFramePrecise;
    monster.view.frameCountPrecise = monster.motionUnit->dwFrameCountPrecise;
    monster.view.animSpeed = static_cast<uint16_t>(monster.motionUnit->wAnimSpeed);
    D2MonStats2Txt* stats2 = UNITS_GetMonStats2TxtRecordFromMonsterId(static_cast<int>(monster.view.classId));
    monster.view.reserved = (monster.motionUnit->pAnimSeq ? 0x01u : 0u) |
        (stats2 && (stats2->dwModeFlags & MONSTATS2MODEFLAG_DD) ? 0x02u : 0u) |
        (monster.motionUnit->pAnimSeq ? ((monster.motionUnit->dwSeqMode & 0xFu) << 4) : 0u);   // bits 4-7: the sequence's current mode (SQ draws as it)
    if (!monster.motionUnit->pDynamicPath) return;
    monster.view.x = static_cast<uint32_t>(PATH_GetXPosition(monster.motionUnit->pDynamicPath));
    monster.view.y = static_cast<uint32_t>(PATH_GetYPosition(monster.motionUnit->pDynamicPath));
    monster.view.direction = PATH_GetDirection(monster.motionUnit->pDynamicPath);
}

static void refresh_player_room_level(D2RealmClientPlayerState& player, bool selected) {
    if (!player.active) return;
    uint32_t level = 0, roomX = 0, roomY = 0, initSeed = 0;
    if (!d2_drlg_client_player_context(static_cast<int>(player.x), static_cast<int>(player.y), 0,
            &level, &roomX, &roomY, &initSeed) || !level) return;
    const bool changed = !player.clientRoomActive || player.clientRoomLevel != level ||
        player.clientRoomTileX != roomX || player.clientRoomTileY != roomY ||
        (g_world.hasMapReveal && g_world.mapLevel != level);
    player.clientRoomActive = 1;
    player.clientRoomLevel = level;
    player.clientRoomTileX = roomX;
    player.clientRoomTileY = roomY;
    if (!selected) return;
    d2_drlg_client_select_render_level(static_cast<int>(level));
    if (changed && g_world.hasMapReveal) {
        g_world.mapLevel = static_cast<uint32_t>(level);
        g_world.hasMapReveal = d2_drlg_client_tile_count() > 0;
        g_world.mapRevision++;
    }
}

static void detach_player_motion_path(D2RealmClientPlayerEntry& player) {
    if (!player.motionUnit || !player.motionUnit->pDynamicPath) return;
    player.motionDirection = PATH_GetDirection(player.motionUnit->pDynamicPath);
    d2_realm_client_motion_detach(player.motionUnit);
}

static void destroy_player_motion(D2RealmClientPlayerEntry& player) {
    if (!player.motionUnit) return;
    detach_player_motion_path(player);
    destroy_player_owned_items(player.motionUnit);
    d2_realm_client_unit_resources_destroy(player.motionUnit);
    UNITS_FreeUnit(player.motionUnit);
    player.motionUnit = nullptr;
}

static void clear_client_missiles();
static D2UnitStrc* create_client_missile(const D2MissileStrc* params, bool general = false);
static void destroy_all_player_motion() {
    clear_client_missiles();
    // The client's own player unit outlives an act change with everything it holds: the server does
    // not send the hero's items or stats again. Only its place in the old act's rooms goes.
    // GUESS: that the other players' units are freed here; the act-load handler's unit sweep is not read.
    // Settled by the decompile of the unit-table clear the 0x03 handler calls.
    for (size_t i = 0; i < g_players.size(); ++i) {
        if (static_cast<int>(i) == g_selectedPlayer) detach_player_motion_path(g_players[i]);
        else destroy_player_motion(g_players[i]);
    }
    d2_realm_client_items_forget_loose();
}

struct PlayerSkillOps {
    bool hasList(D2UnitStrc* unit) const { return unit->pSkills != nullptr; }
    bool allocList(D2UnitStrc* unit) const { return (unit->pSkills = SKILLS_AllocSkillList(nullptr)) != nullptr; }
    void initList(D2UnitStrc* unit) const { SKILLS_InitSkillList(unit); }
    D2SkillStrc* left(D2UnitStrc* unit) const { return UNITS_GetLeftSkill(unit); }
    D2SkillStrc* right(D2UnitStrc* unit) const { return UNITS_GetRightSkill(unit); }
};

static bool ensure_player_unit(D2RealmClientPlayerEntry& player) {
    if (player.motionUnit) return true;
    const auto& view = player.view;
    player.motionUnit = UNITS_AllocUnit(nullptr, UNIT_PLAYER);
    if (!player.motionUnit) return false;
    player.motionUnit->dwClassId = static_cast<int32_t>(view.classId);
    player.motionUnit->dwUnitId = view.guid;
    // The client's new player unit is targetable, attackable and a valid target (flags 2, 4 and 8; bit 0x20
    // cleared): the arrival gate +0x2AFB0 reads flag 2.
    player.motionUnit->dwFlags = (player.motionUnit->dwFlags & ~0x20u) | 0xeu;
    if (!d2_realm_client_unit_resources_init(player.motionUnit)) {
        d2_realm_client_unit_resources_destroy(player.motionUnit);
        UNITS_FreeUnit(player.motionUnit);
        player.motionUnit = nullptr;
        return false;
    }
    player.motionUnit->dwAnimMode = view.initialMode;
    player.motionDirection = static_cast<uint8_t>(view.initialDirection & 63u);
    if (sgptDataTables && sgptDataTables->nStatesTxtRecordCount > 0)
        STATLIST_AllocStatListEx(player.motionUnit, 0, nullptr, nullptr);
    // The owner is never sent its own base velocity: the client sets it, as the server does for its unit.
    d2_realm_client_player_init_rates(player.motionUnit,
        [](D2UnitStrc* u, int32_t stat, int32_t value) { STATLIST_SetUnitStat(u, stat, value, 0); });
    // The same init gives the unit its skill list (Attack, the class's base skills, Attack in both
    // hands), so a hand can be read before, or without, the server's skill packets.
    if (sgptDataTables) d2_realm_client_player_init_skills(player.motionUnit, PlayerSkillOps{});
    return true;
}

static bool ensure_player_motion(D2RealmClientPlayerEntry& player, bool forceReattach = false) {
    D2RealmClientPlayerState& view = player.view;
    if (!view.active || !ensure_player_unit(player)) return false;
    D2ActiveRoomStrc* room = d2_drlg_client_active_room(static_cast<int>(view.x), static_cast<int>(view.y));
    if (!room) {
        detach_player_motion_path(player);
        return true; // Assignment can precede the MapReveal that creates its room.
    }
    if (!forceReattach && player.motionUnit && player.motionUnit->pDynamicPath &&
        player.motionUnit->pDynamicPath->pRoom == room) return true;
    detach_player_motion_path(player);
    if (!d2_realm_client_motion_attach(player.motionUnit, static_cast<int>(view.x), static_cast<int>(view.y)))
        return false;
    PATH_SetDirection(player.motionUnit->pDynamicPath, player.motionDirection);
    UNITS_SetAnimStartFrame(player.motionUnit);
    return true;
}

static void refresh_player_motion_view(D2RealmClientPlayerEntry& player, bool selected) {
    if (!player.motionUnit || !player.motionUnit->pDynamicPath) return;
    D2RealmClientPlayerState& view = selected ? g_player : player.view;
    view.x = static_cast<uint32_t>(PATH_GetXPosition(player.motionUnit->pDynamicPath));
    view.y = static_cast<uint32_t>(PATH_GetYPosition(player.motionUnit->pDynamicPath));
    player.motionDirection = PATH_GetDirection(player.motionUnit->pDynamicPath);
    refresh_player_room_level(view, selected);
    if (selected) store_selected_player();
}

static void refresh_all_player_motion_rooms() {
    for (size_t i = 0; i < g_players.size(); ++i) {
        const bool selected = static_cast<int>(i) == g_selectedPlayer;
        auto& entry = g_players[i];
        if (selected) {
            refresh_player_room_level(g_player, true);
            entry.view = g_player;
        } else {
            refresh_player_room_level(entry.view, false);
        }
        ensure_player_motion(entry);
    }
    store_selected_player();
}

static bool parse_monster_assign(const uint8_t* packet, int length, D2RealmClientMonster& out) {
    if (!packet || length < 13 || packet[0] != 0xac || packet[12] != length || !sgptDataTables) return false;
    const uint32_t guid = static_cast<uint32_t>(packet[1]) |
        (static_cast<uint32_t>(packet[2]) << 8) | (static_cast<uint32_t>(packet[3]) << 16) |
        (static_cast<uint32_t>(packet[4]) << 24);
    const uint32_t classId = static_cast<uint32_t>(packet[5]) | (static_cast<uint32_t>(packet[6]) << 8);
    const uint32_t x = static_cast<uint32_t>(packet[7]) | (static_cast<uint32_t>(packet[8]) << 8);
    const uint32_t y = static_cast<uint32_t>(packet[9]) | (static_cast<uint32_t>(packet[10]) << 8);
    const uint32_t hpPct = packet[11];
    auto* monStats = DATATBLS_GetMonStatsTxtRecord(static_cast<int>(classId));
    if (!monStats) return false;
    auto* monStats2 = UNITS_GetMonStats2TxtRecordFromMonsterId(static_cast<int>(classId));
    if (!monStats2) return false;

    D2RealmBitReader bits(packet + 13, length - 13);
    uint32_t value = 0;
    if (!bits.read(4, value)) return false;
    out.rawPrefix = static_cast<uint8_t>(value); // Input-block +0x18; native fresh and duplicate mode setter input.
    uint32_t hasComponents = 0;
    if (!bits.read(1, hasComponents)) return false;
    uint8_t choices[16]{};
    if (hasComponents) {
        for (size_t i = 0; i < 16; ++i) {
            uint32_t choice = 0;
            if (!bits.read(componentWidth(monStats2->nComponentChoiceCounts[i]), choice)) return false;
            choices[i] = static_cast<uint8_t>(choice);
        }
    }

    uint32_t hasTypeFlags = 0;
    if (!bits.read(1, hasTypeFlags)) return false;
    uint32_t typeFlags = 0;
    int32_t superUnique = -1;
    uint16_t nameId = 0;
    uint32_t ownerGuid = 0;
    if (hasTypeFlags) {
        const uint32_t masks[5] = { 0x04, 0x08, 0x02, 0x10, 0x40 };
        for (uint32_t mask : masks) {
            uint32_t flag = 0;
            if (!bits.read(1, flag)) return false;
            if (flag) typeFlags |= mask;
        }
        if (typeFlags & 0x02) {
            int32_t raw = 0;
            if (!bits.readSigned(16, raw)) return false;
            superUnique = raw;
        }
        bool terminated = false;
        while (bits.remaining() >= 8) {
            if (!bits.read(8, value)) return false;
            if (value == 0) { terminated = true; break; }
            out.nameBytes.push_back(static_cast<uint8_t>(value));
        }
        if (!terminated || !bits.read(16, value)) return false;
        nameId = static_cast<uint16_t>(value);
        uint32_t hasOwner = 0;
        if (!bits.read(1, hasOwner)) return false;
        if (hasOwner && !bits.read(32, ownerGuid)) return false;
    }

    uint32_t hasExtra31 = 0;
    if (!bits.read(1, hasExtra31)) return false;
    out.hasExtra31 = hasExtra31 != 0;
    uint32_t extra31 = 0;
    if (hasExtra31 && !bits.read(31, extra31)) return false;
    uint32_t hasStats = 0;
    if (!bits.read(1, hasStats)) return false;
    out.hasStats = hasStats != 0;
    std::vector<D2RealmClientMonsterStat> stats;
    if (hasStats) {
        if (!sgptDataTables->pItemStatCostTxt || sgptDataTables->nItemStatCostTxtRecordCount <= 0) return false;
        uint32_t statId = 0;
        if (!bits.read(9, statId)) return false;
        while (statId < 0x1ff) {
            if (statId >= static_cast<uint32_t>(sgptDataTables->nItemStatCostTxtRecordCount)) return false;
            const D2ItemStatCostTxt& cost = sgptDataTables->pItemStatCostTxt[statId];
            if (cost.nSendBits == 0) break;
            int32_t parameter = 0;
            int32_t statValue = 0;
            if (cost.nSendParamBits && !bits.readSigned(cost.nSendParamBits, parameter)) return false;
            if (cost.nSendBits < 32 && (cost.dwItemStatFlags & ITEMSTATCOSTFLAG_SIGNED)) {
                if (!bits.readSigned(cost.nSendBits, statValue)) return false;
            } else {
                if (!bits.read(cost.nSendBits, value)) return false;
                statValue = static_cast<int32_t>(value);
            }
            stats.push_back({ static_cast<uint16_t>(statId), parameter, statValue });
            if (!bits.read(9, statId)) return false;
        }
    }

    out.view = {};
    out.view.version = 1;
    out.view.size = sizeof(out.view);
    out.view.guid = guid;
    out.view.classId = classId;
    out.view.code = monStats->dwCode;
    out.view.mode = out.rawPrefix;
    out.view.x = x;
    out.view.y = y;
    out.view.hpPct = hpPct;
    out.view.typeFlags = typeFlags;
    out.view.direction = 0; // 0xAC has no direction field; constructor default still needs native confirmation.
    for (size_t i = 0; i < 16; ++i) {
        const uint8_t count = monStats2->nComponentChoiceCounts[i];
        const uint8_t choice = choices[i];
        if (count == 0 || choice >= count || choice >= 12) continue;
        const int compCode = static_cast<int8_t>(monStats2->unk0x26[i].nComposit[choice]);
        if (compCode < 0 || compCode >= sgptDataTables->nCompCodeTxtRecordCount) continue;
        out.view.componentCodes[i] = DATATBLS_GetCodeFromCompCodeTxt(compCode);
    }
    out.superUniqueIndex = static_cast<int16_t>(superUnique);
    out.nameId = nameId;
    out.ownerGuid = ownerGuid;
    out.extra31 = extra31;
    out.stats = std::move(stats);
    return true;
}

static void record_unit_fx(uint32_t kind, D2UnitStrc* unit, uint32_t row, uint32_t delay, uint32_t skill);
static bool apply_monster_assign(const uint8_t* packet, int length) {
    D2RealmClientMonster monster{};
    if (!parse_monster_assign(packet, length, monster)) return false;
    for (auto& existing : g_monsters) {
        if (existing.view.guid == monster.view.guid) {
            // Native duplicate-GUID handling reuses the CUnit and calls its mode setter; it does
            // not replace class, component set, or position from this assignment record.
            if (existing.motionUnit) d2_realm_client_unit_set_mode(existing.motionUnit, monster.view.mode);
            refresh_monster_motion_view(existing);
            existing.view.hpPct = monster.view.hpPct;
            apply_assigned_life(existing.motionUnit, monster.view.hpPct);
            if (monster.hasStats) {
                existing.stats = std::move(monster.stats);
                existing.hasStats = true;
            }
            if (monster.hasExtra31) {
                existing.extra31 = monster.extra31;
                existing.hasExtra31 = true;
            }
            return true;
        }
    }
    g_monsters.push_back(std::move(monster));
    D2RealmClientMonster& made = g_monsters.back();
    create_monster_motion(made);
    // 1.13c D2Client+0x23170: the made monster's Init (or Skill1) sound, once.
    if (made.motionUnit) {
        const uint32_t row = d2::realm::clientMonsterCreateSoundRow(made.motionUnit, made.view.typeFlags, made.superUniqueIndex);
        if (row) record_unit_fx(d2::realm::kUnitFxSound, made.motionUnit, row, 0, d2::realm::kUnitFxSkillCreate);
    }
    return true;
}

static bool remove_monster(const uint8_t* packet, int length) {
    if (!packet || length != 6 || packet[0] != 0x0a) return false;
    // D2GS RemoveObject carries a unit type followed by its GUID. Type 1 is UNIT_MONSTER;
    // other unit classes are owned by their corresponding client projections.
    if (packet[1] != 1) return true;
    const uint32_t guid = static_cast<uint32_t>(packet[2]) |
        (static_cast<uint32_t>(packet[3]) << 8) | (static_cast<uint32_t>(packet[4]) << 16) |
        (static_cast<uint32_t>(packet[5]) << 24);
    for (auto it = g_monsters.begin(); it != g_monsters.end();) {
        if (it->view.guid != guid) { ++it; continue; }
        g_monsterDeathModeEvents.erase(std::remove_if(g_monsterDeathModeEvents.begin(), g_monsterDeathModeEvents.end(),
            [guid](const auto& event) { return event.guid == guid; }), g_monsterDeathModeEvents.end());
        destroy_monster_motion(*it);
        it = g_monsters.erase(it);
    }
    return true;
}

static void clear_game_packet_state();
static void game_packet_player_removed(uint32_t guid);
// The per-unit packet queue. A packet whose table row has a queued handler is not run when it arrives: the
// dispatch loop stores it on the unit it names (D2Client+0xAC630) and the unit's own frame update runs the
// stored packets in arrival order after the unit's mode step (D2Client+0x2B1E0 calling +0xABC20). A packet
// naming a unit the client does not hold is dropped, and a unit's queue goes with the unit.
struct RealmQueuedPacket { uint8_t unitType; uint32_t guid; std::vector<uint8_t> bytes; };
static std::vector<RealmQueuedPacket> g_unitQueue;
static void drop_unit_queue(uint8_t unitType, uint32_t guid) {
    g_unitQueue.erase(std::remove_if(g_unitQueue.begin(), g_unitQueue.end(), [=](const RealmQueuedPacket& q) {
        return q.unitType == unitType && q.guid == guid; }), g_unitQueue.end());
}
static int reject_stream(int error) {
    clear_game_packet_state();
    clear_client_missiles();
    reset_monster_death_mode_events();
    destroy_all_monster_motion();
    clear_players();
    d2_drlg_act_end();
    clear_world();
    g_monsters.clear();
    d2::realm::clientPetsClear();
    g_monsterViews.clear();
    clear_objects();
    clear_game_chat();
    g_pendingLength = 0;
    return error;
}

static void clear_client_quest_record();
// What the server's quest packets (0x28, 0x29, 0x52, 0x5D) and tile-unit packets (0x09) have built.
static D2RealmClientQuests g_quests;
static D2RealmClientWarps g_warps;

static void clear_session_records();
EMSCRIPTEN_KEEPALIVE void d2_realm_client_player_reset() {
    clear_game_packet_state();
    clear_client_quest_record();
    d2_realm_client_quests_reset(g_quests);
    g_warps.list.clear();
    clear_client_missiles();
    reset_monster_death_mode_events();
    destroy_all_monster_motion();
    clear_players();
    d2_drlg_act_end();
    clear_world();
    g_monsters.clear();
    d2::realm::clientPetsClear();
    g_monsterViews.clear();
    clear_objects();
    clear_game_chat();
    clear_session_records();
    g_pendingLength = 0;
}

EMSCRIPTEN_KEEPALIVE void d2_realm_client_set_difficulty(uint32_t difficulty) {
    // MCP CREATEGAME difficulty is retained as the exact client mode used by AllocAct.
    g_clientDifficulty = difficulty;
}

// A server assignment is the only event allowed to establish the remote GUID mapping.
static int assign_player(const uint8_t* packet, int length) {
    D2RealmPlayerAssign assigned{};
    if (d2_realm_d2gs_parse_player_assign(packet, length, &assigned) != 26) return 0;
    D2RealmClientPlayerEntry entry{};
    D2RealmClientPlayerState& created = entry.view;
    created.version = 5;
    created.size = sizeof(created);
    uint32_t level = 0, roomX = 0, roomY = 0, initSeed = 0;
    // The game sends the own unit before the act load, so no room can hold it yet: the constructor
    // (1.13c D2Client+0xA7630) goes on without one, and a later reassignment resolves the room.
    level = roomX = roomY = initSeed = 0;
    d2_drlg_client_player_context(assigned.x, assigned.y, 1, &level, &roomX, &roomY, &initSeed);
    created.active = 1;
    created.guid = assigned.guid;
    created.x = assigned.x;
    created.y = assigned.y;
    created.classId = assigned.classId;
    std::memcpy(created.name, assigned.name, sizeof(created.name));
    created.cunitInitSeed = initSeed;
    created.cunitInitSeedValid = level != 0;
    // FUN_6FB57630 always initializes CUnit.pSeed to {1,666}. A resolved room then
    // replaces it with {dwInitSeed,666}; FUN_6FAC91E0 advances it once because a newly
    // constructed unit differs from the currently selected CUnit (including fresh null state).
    created.cunitRngLow = level ? initSeed : 1;
    created.cunitRngHigh = 666;
    const uint64_t advanced = static_cast<uint64_t>(created.cunitRngLow) * 0x6ac690c5ULL +
        created.cunitRngHigh;
    created.cunitRngLow = static_cast<uint32_t>(advanced);
    created.cunitRngHigh = static_cast<uint32_t>(advanced >> 32);
    created.cunitRngValid = 1;
    // FUN_6FAC91E0 sets the fresh player to PLRMODE_TOWNNEUTRAL (5), advances pSeed
    // when the new unit differs from the current unit, then calls D2Common #10706 with
    // the resulting low six bits. In 1.13c #10706 writes both path direction fields.
    created.initialMode = 5;
    created.initialDirection = created.cunitRngLow & 0x3f;
    entry.motionDirection = static_cast<uint8_t>(created.initialDirection);
    refresh_player_room_level(created, false);

    // Native FUN_6FB57360 replaces a duplicate type/GUID unit: it removes the old unit from its
    // registry, destroys it (clearing current-unit selection if applicable), then inserts the new
    // allocation. Preserve that one-owner lifecycle instead of appending another render/path row.
    for (size_t i = 0; i < g_players.size(); ++i) {
        if (g_players[i].view.guid != assigned.guid) continue;
        const bool wasSelected = static_cast<int>(i) == g_selectedPlayer;
        game_packet_player_removed(assigned.guid);
        destroy_player_motion(g_players[i]);
        g_players.erase(g_players.begin() + static_cast<std::ptrdiff_t>(i));
        if (wasSelected) {
            g_selectedPlayer = -1;
            clear_player();
        } else if (g_selectedPlayer > static_cast<int>(i)) {
            --g_selectedPlayer;
        }
        break;
    }
    g_players.push_back(entry);
    if (!ensure_player_motion(g_players.back())) return -1;
    return 1;
}

static int select_player(const uint8_t* packet, int length) {
    if (!packet || length != 6 || packet[0] != 0x0b) return -1;
    const uint8_t unitType = packet[1];
    const uint32_t guid = static_cast<uint32_t>(packet[2]) |
        (static_cast<uint32_t>(packet[3]) << 8) |
        (static_cast<uint32_t>(packet[4]) << 16) |
        (static_cast<uint32_t>(packet[5]) << 24);
    if (unitType != 0) return 0;
    for (size_t i = 0; i < g_players.size(); ++i) {
        if (g_players[i].view.guid != guid) continue;
        store_selected_player();
        g_selectedPlayer = static_cast<int>(i);
        g_player = g_players[i].view;
        refresh_player_room_level(g_player, true);
        ensure_player_motion(g_players[i]);
        store_selected_player();
        return 0;
    }
    return 0; // Native lookup miss leaves the previously selected unit unchanged.
}

static int remove_player(const uint8_t* packet, int length) {
    if (!packet || length != 6 || packet[0] != 0x0a) return -1;
    const uint8_t unitType = packet[1];
    const uint32_t guid = static_cast<uint32_t>(packet[2]) |
        (static_cast<uint32_t>(packet[3]) << 8) |
        (static_cast<uint32_t>(packet[4]) << 16) |
        (static_cast<uint32_t>(packet[5]) << 24);
    if (unitType != 0) return 0;
    for (size_t i = 0; i < g_players.size(); ++i) {
        if (g_players[i].view.guid != guid) continue;
        const bool selected = static_cast<int>(i) == g_selectedPlayer;
        if (selected) clear_player();
        game_packet_player_removed(guid);
        clear_target_relations_to(g_players[i].motionUnit);
        destroy_player_motion(g_players[i]);
        g_players.erase(g_players.begin() + static_cast<std::ptrdiff_t>(i));
        if (selected) g_selectedPlayer = -1;
        else if (g_selectedPlayer > static_cast<int>(i)) --g_selectedPlayer;
        return 0;
    }
    return 0;
}

struct D2RealmClientStateUpdate {
    uint8_t state;
    std::vector<D2RealmClientStateStat> stats;
    bool hasStats = false;
};

static D2UnitStrc* find_realm_client_unit(uint8_t unitType, uint32_t guid) {
    if (unitType == UNIT_PLAYER) {
        for (auto& player : g_players)
            if (player.view.active && player.view.guid == guid) return player.motionUnit;
    } else if (unitType == UNIT_MONSTER) {
        for (auto& monster : g_monsters)
            if (monster.view.guid == guid) return monster.motionUnit;
    } else if (unitType == UNIT_OBJECT) {
        return d2gs_client_object_find(guid);
    } else if (unitType == UNIT_ITEM) {
        // A ground item: the client holds it in its ground mode with the record's cell; the room is
        // the active one over that cell now, so the reach test and the walk read a placed unit.
        D2UnitStrc* item = d2_realm_client_owned_item_find(guid);
        if (!item || !item->pStaticPath || (item->dwAnimMode != IMODE_ONGROUND && item->dwAnimMode != IMODE_DROPPING)) return nullptr;
        const int x = item->pStaticPath->tGameCoords.nX, y = item->pStaticPath->tGameCoords.nY;
        item->pStaticPath->pRoom = d2_drlg_client_active_room(x, y);
        return item->pStaticPath->pRoom ? item : nullptr;
    }
    return nullptr;
}

// The movement and mode packets (0x0C, 0x0F, 0x10, 0x67..0x6D) run through the retail client's
// own handlers (src/core/d2client/d2client_af*.cpp). The handlers read the record, build the
// request block and call two engine routines; this is the native side of those two calls: the
// unit placement (D2Client 0x2A1B0) and the action dispatch (D2Client 0x2A750 / 0x4DC70).
namespace {
D2cPktEnv g_pktEnv{};
d2::realm::PktRoute g_pktRoute{};
D2RealmClientMonster* g_pktMonster = nullptr;
D2UnitStrc* g_pktUnit = nullptr;
uint8_t g_pktOpcode = 0;
int g_pktResult = 0;
}

static uint32_t pkt_word(const uint32_t* l, uint32_t i) {
    return l && i < g_pktRoute.block ? l[i] : 0;
}

static uint32_t pkt_get_stat(uintptr_t unit, uint32_t stat, uint32_t layer) {
    D2UnitStrc* u = reinterpret_cast<D2UnitStrc*>(unit);
    if (!u || !u->pStatListEx) return 0;
    return static_cast<uint32_t>(STATLIST_UnitGetStatValue(u, static_cast<int>(stat), static_cast<uint16_t>(layer)));
}

static void pkt_set_stat(uintptr_t unit, uint32_t stat, uint32_t value, uint32_t layer) {
    D2UnitStrc* u = reinterpret_cast<D2UnitStrc*>(unit);
    if (!u || !u->pStatListEx) return;
    STATLIST_SetUnitStat(u, static_cast<int>(stat), static_cast<int>(value), static_cast<uint16_t>(layer));
}

static bool g_resyncPending = false;
static uint16_t g_resyncX = 0, g_resyncY = 0;

// 0x2A1B0: x in EAX, y the first stack word, the radius selector the second.
static void pkt_place(uint32_t x, uintptr_t unitBits, uint32_t y, uint32_t radiusSel, uint32_t, uint32_t) {
    D2UnitStrc* unit = reinterpret_cast<D2UnitStrc*>(unitBits);
    x &= 0xffff;
    y &= 0xffff;
    if (!unit) return;
    const bool isPlayer = unit->dwUnitType == UNIT_PLAYER;
    if (d2::realm::placeIgnored(unit->dwUnitType, unit->dwAnimMode, unit->dwFlags, x, y)) return;
    unit->nLastClickX = static_cast<uint16_t>(x);
    unit->nLastClickY = static_cast<uint16_t>(y);
    D2RealmClientPlayerEntry* player = nullptr;
    if (isPlayer)
        for (auto& candidate : g_players)
            if (candidate.motionUnit == unit) { player = &candidate; break; }
    const bool isLocal = player && g_selectedPlayer >= 0 &&
        player == &g_players[static_cast<size_t>(g_selectedPlayer)];
    // The monster view is found from the unit, not from the monster the record named: retail acts on the unit.
    D2RealmClientMonster* monster = nullptr;
    if (!isPlayer && unit->dwUnitType == UNIT_MONSTER)
        for (auto& candidate : g_monsters)
            if (candidate.motionUnit == unit) { monster = &candidate; break; }
    if (isPlayer && !player) return;
    if (!unit->pDynamicPath) {
        if (monster) { monster->view.x = x; monster->view.y = y; }
        return;
    }
    // The local player's tolerance adds ((ping + 0x32) >> 7). The ping is the host's own clock
    // difference, not a game value, and the realm layer does not sample it yet.
    // GUESS: localTerm 0 is right only for a ping under 78 ms; a capture with the ping pinned settles it.
    const int radius = d2::realm::placeRadius(unit->dwUnitType == UNIT_MONSTER, isLocal, unit->dwAnimMode, radiusSel, 0);
    const int curX = PATH_GetXPosition(unit->pDynamicPath), curY = PATH_GetYPosition(unit->pDynamicPath);
    if (std::abs(curX - static_cast<int>(x)) <= radius && std::abs(curY - static_cast<int>(y)) <= radius) {
        // Retail rule for a target inside the arrival radius: if the target shares a coordinate with the unit's
        // position, nothing happens. Otherwise two blocked-target tests run, the first on the unit's own two
        // record words and the second on the adjusted target; a hit on either keeps the path. With no hit the
        // target dispatch runs: a local player sends its own position back, another player takes a walk command,
        // any other unit takes a path command. Captured: a near target is not always kept (with a small radius
        // the check reaches both blocked-target tests and then the dispatch).
        // GUESS: the blocked-target test is a sprite-cell query this realm layer does not hold, so every near
        // target is treated as blocked here (keeps the path). Blocker: that sprite query, which needs the sprite
        // data this layer does not load; a fixture with the sprite data pinned settles it.
        return;
    }
    // Retail requires a room at the target.
    if (!d2_drlg_client_active_room(static_cast<int>(x), static_cast<int>(y))) return;
    if (isLocal) {
        // Local resync: retail sends the client's own position as a 5-byte packet 0x5F {x, y}
        // and does not move the unit.
        g_resyncPending = true;
        g_resyncX = static_cast<uint16_t>(curX);
        g_resyncY = static_cast<uint16_t>(curY);
        return;
    }
    if (isPlayer) {
        // Retail's walk dispatch for another player: for unit modes 2, 3 and 6 it first refreshes the path from
        // the unit's current path record (an absent record is fatal in retail), and it issues the walk command
        // whenever the target's y is nonzero, whatever the mode. Captured: for modes 2/3/6 the walk/run is
        // re-issued through the state-packet handler with an all-zero packet (one type for mode 3, another for
        // modes 2 and 6). Here the refresh is a detach and attach at the target keeping the facing.
        // GUESS: the walk command's effect (its move-kind dispatch and the path helpers it calls) is not ported,
        // so the re-issue is not modelled.
        player->view.x = x;
        player->view.y = y;
        ensure_player_motion(*player, true);
        return;
    }
    // Only a monster has a client motion to move; objects, missiles and items are not re-attached here.
    if (!monster) return;
    d2_realm_client_motion_detach(unit);
    d2_realm_client_motion_attach(unit, static_cast<int>(x), static_cast<int>(y));
}

// The resync the local player's far placement asks the page to send: packet 0x5F, x and y little-endian.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_take_resync(uint8_t* out) {
    if (!g_resyncPending || !out) return 0;
    g_resyncPending = false;
    out[0] = 0x5f;
    out[1] = static_cast<uint8_t>(g_resyncX); out[2] = static_cast<uint8_t>(g_resyncX >> 8);
    out[3] = static_cast<uint8_t>(g_resyncY); out[4] = static_cast<uint8_t>(g_resyncY >> 8);
    return 5;
}

// The request's target: a point, or for the unit-target handlers the (type, guid) of a unit.
static bool pkt_target(const uint32_t* l, int* x, int* y) {
    if (!g_pktRoute.unitTarget) {
        *x = static_cast<int>(pkt_word(l, 0));
        *y = static_cast<int>(pkt_word(l, 1));
        return true;
    }
    D2UnitStrc* target = find_realm_client_unit(static_cast<uint8_t>(pkt_word(l, 0)), pkt_word(l, 1));
    if (!target || !target->pDynamicPath) return false;
    *x = static_cast<int>(PATH_GetXPosition(target->pDynamicPath));
    *y = static_cast<int>(PATH_GetYPosition(target->pDynamicPath));
    return true;
}

static D2RealmClientPlayerEntry* player_entry_of(const D2UnitStrc* unit, bool* selected) {
    for (size_t i = 0; i < g_players.size(); ++i) {
        if (!unit || g_players[i].motionUnit != unit) continue;
        if (selected) *selected = static_cast<int>(i) == g_selectedPlayer;
        return &g_players[i];
    }
    return nullptr;
}

// Put a player's client unit on a cell at once (the placement the 0x15 reassignment also does).
static bool place_player_entry(D2RealmClientPlayerEntry& entry, bool selected, uint32_t x, uint32_t y) {
    D2RealmClientPlayerState& view = selected ? g_player : entry.view;
    view.x = x;
    view.y = y;
    refresh_player_room_level(view, selected);
    if (selected) entry.view = g_player;
    if (!ensure_player_motion(entry, true)) return false;
    if (selected) store_selected_player();
    return true;
}

// The hero stands up in town (see heroRevivesOnLife).
static void revive_hero_on_life(D2UnitStrc* unit, uint32_t life) {
    if (!unit || !d2::realm::heroRevivesOnLife(unit->dwAnimMode, life)) return;
    d2_realm_client_unit_set_mode(unit, d2::realm::kPlayerModeTownNeutral);
    unit->dwFlags &= ~d2::realm::kUnitFlagDead;
}

// 1.13c D2Client+0x19830 selectors 7, 8 and 9: a player dies, lies dead, or stands again.
// Returns -1 when the selector is not one of the three.
static int apply_player_life_action(D2UnitStrc* unit, uint32_t selector, int x, int y, uint32_t hitClass) {
    bool selected = false;
    D2RealmClientPlayerEntry* const entry = player_entry_of(unit, &selected);
    if (!entry) return -1;
    const bool wasDead = d2::realm::playerIsDead(unit->dwFlags, unit->dwAnimMode);
    const bool attached = unit->pDynamicPath != nullptr;
    const int32_t dx = attached ? x - static_cast<int>(PATH_GetXPosition(unit->pDynamicPath)) : 0;
    const int32_t dy = attached ? y - static_cast<int>(PATH_GetYPosition(unit->pDynamicPath)) : 0;
    const bool inTown = attached && DUNGEON_IsRoomInTown(UNITS_GetRoom(unit));
    d2::realm::PlayerLifeAction action{};
    if (!d2::realm::playerLifeAction(static_cast<uint8_t>(selector), selected, inTown, wasDead, dx, dy, &action))
        return -1;
    if (action.storeHitClass) unit->dwLastHitClass = hitClass;
    if (action.clearDead) unit->dwFlags = d2::realm::playerFlagsAfterRevive(unit->dwFlags);
    if (d2_realm_client_unit_set_mode(unit, static_cast<uint32_t>(action.mode)) < 0) return 0;
    if (action.markDead) unit->dwFlags = d2::realm::playerFlagsAfterDeath(unit->dwFlags);
    switch (action.move) {
    case d2::realm::PlayerLifeMove::Place:
        place_player_entry(*entry, selected, static_cast<uint32_t>(x), static_cast<uint32_t>(y));
        break;
    case d2::realm::PlayerLifeMove::Walk:
    case d2::realm::PlayerLifeMove::Run:
        if (attached) d2_realm_client_motion_apply_player_move(unit,
            action.move == d2::realm::PlayerLifeMove::Run ? 0x17 : 0x01, 0,
            static_cast<uint16_t>(x), static_cast<uint16_t>(y));
        break;
    default: break;
    }
    return 1;
}
static void emit_hit_overlay(D2UnitStrc* unit, bool struckNow = false);
static void record_unit_hit(D2UnitStrc* unit);

static int pkt_player_action(D2UnitStrc* unit, uint32_t code, const uint32_t* l) {
    if (code >= 7 && code <= 9) {
        const int result = apply_player_life_action(unit, code, static_cast<int>(pkt_word(l, 0)),
            static_cast<int>(pkt_word(l, 1)), pkt_word(l, 2));
        return result > 0 ? 1 : 0;
    }
    if (code == 1 || code == 0x17) {
        int x = 0, y = 0;
        if (!pkt_target(l, &x, &y)) return 0;
        return d2_realm_client_motion_apply_player_move(unit, static_cast<uint8_t>(code), 0,
            static_cast<uint16_t>(x), static_cast<uint16_t>(y)) > 0 ? 1 : 0;
    }
    if (code == 6) {
        unit->dwLastHitClass = pkt_word(l, 2);
        const uint32_t previous = unit->dwAnimMode;
        if (d2_realm_client_unit_set_mode(unit, PLRMODE_GETHIT) <= 0) return 0;
        if (previous != unit->dwAnimMode) emit_hit_overlay(unit);
        return 1;
    }
    if (code == d2::realm::kHitAction) {
        unit->dwLastHitClass = d2::realm::playerHitClass(pkt_word(l, 2));
        record_unit_hit(unit);
        return 1;
    }
    return 0;
}

static int pkt_monster_mode(D2RealmClientMonster& monster, uint32_t code, uint32_t mode) {
    D2UnitStrc* const unit = monster.motionUnit;
    const uint32_t previousMode = unit->dwAnimMode;
    if (d2_realm_client_unit_set_mode(unit, mode) < 0) return 0;
    if (previousMode != unit->dwAnimMode) emit_hit_overlay(unit);
    // The old 0x69/0x6A projection's event queue stays the view the page reads.
    if (g_pktOpcode != 0x69 && g_pktOpcode != 0x6a) return 1;
    if (!d2::realm::appendMonsterDeathModeEvent(g_monsterDeathModeEvents,
            g_nextMonsterDeathModeSequence, monster.view.guid, g_pktOpcode, code, previousMode, mode)) return 0;
    ++g_nextMonsterDeathModeSequence;
    return 1;
}

static int pkt_monster_action_case(D2RealmClientMonster& monster, uint32_t code, const uint32_t* l) {
    D2UnitStrc* const unit = monster.motionUnit;
    using d2::realm::MonsterAction;
    switch (d2::realm::monsterAction(code)) {
    case MonsterAction::Move: {
        int x = 0, y = 0;
        if (!unit->pDynamicPath || !pkt_target(l, &x, &y)) return 0;
        // The move packet's path type is the path the server walks; the client paths with it too.
        const int32_t pathType = static_cast<int32_t>(l[3]);
        if (pathType > 0 && pathType < PATHTYPE_COUNT && pathType != PATHTYPE_KNOCKBACK_CLIENT)
            PATH_SetType(unit->pDynamicPath, pathType);
        const int points = d2_realm_client_motion_set_target(unit, x, y);
        monster.lastPlanResult = points;
        if (points > 0) {
            d2_realm_client_motion_set_mode(unit, (code == 1 || code == 0) ? MONMODE_WALK : MONMODE_RUN);
            return 1;
        }
        const uint32_t fallback = d2_realm_client_motion_refusal_mode(unit->dwUnitType, unit->dwAnimMode);
        if (fallback != unit->dwAnimMode) {
            PATH_RecacheRoomIfNeeded(unit->pDynamicPath);   // D2Client+0x4DC30 puts the unit on its sub-tile's centre first
            d2_realm_client_motion_set_mode(unit, fallback);
        }
        return 0;
    }
    case MonsterAction::GetHit: {
        const uint32_t hit = d2::realm::getHitLife(pkt_word(l, 2));
        const uintptr_t bits = reinterpret_cast<uintptr_t>(unit);
        if (hit && (pkt_get_stat(bits, 6, 0) >> 8) != hit) pkt_set_stat(bits, 6, hit << 8, 0);
        unit->dwLastHitClass = pkt_word(l, 6);
        return pkt_monster_mode(monster, code, MONMODE_GETHIT);
    }
    case MonsterAction::Hit: {
        const auto hit = d2::realm::monsterHit(pkt_word(l, 0), pkt_word(l, 1));
        if (hit.setLife) pkt_set_stat(reinterpret_cast<uintptr_t>(unit), 6, hit.life << 8, 0);
        unit->dwLastHitClass = hit.hitClass;
        record_unit_hit(unit);
        return 1;
    }
    case MonsterAction::Death: {
        // A monster whose death spawns its minion is replaced by the spawn, not played out here.
        D2MonStatsTxt* stats = DATATBLS_GetMonStatsTxtRecord(static_cast<int>(monster.view.classId));
        const int minion = stats ? stats->wMinion[0] : -1;
        if (stats && stats->nSplEndDeath == 1 && minion >= 0 &&
            sgptDataTables && minion < sgptDataTables->nMonStatsTxtRecordCount) return 0;
        unit->dwLastHitClass = pkt_word(l, 6);
        pkt_set_stat(reinterpret_cast<uintptr_t>(unit), 6, 0, 0);
        return pkt_monster_mode(monster, code, MONMODE_DEATH);
    }
    case MonsterAction::Dead:
        // Izual's DEAD from his sequence mode is his own transition.
        if (monster.view.classId == 0x11c && unit->dwAnimMode == MONMODE_SEQUENCE) return 0;
        return pkt_monster_mode(monster, code, MONMODE_DEAD);
    case MonsterAction::Stop: {
        const uint32_t hp = pkt_word(l, 2);
        monster.view.hpPct = hp;
        if (hp) pkt_set_stat(reinterpret_cast<uintptr_t>(unit), 6, hp << 8, 0);
        const int x = static_cast<int>(pkt_word(l, 0)), y = static_cast<int>(pkt_word(l, 1));
        if (unit->pDynamicPath) {
            const int dx = std::abs(static_cast<int>(PATH_GetXPosition(unit->pDynamicPath)) - x);
            const int dy = std::abs(static_cast<int>(PATH_GetYPosition(unit->pDynamicPath)) - y);
            if ((dx >= 2 || dy >= 2) && d2_realm_client_motion_set_target(unit, x, y) > 0) {
                d2_realm_client_motion_set_mode(unit, MONMODE_WALK);
                return 1;
            }
        }
        // The stop keeps the client's own fixed-point position (the retail trace shows the path dwords
        // unchanged through the stop); a unit that stopped at path end already sits on 0x8000.
        if (unit->dwAnimMode != MONMODE_DEAD && unit->dwAnimMode != MONMODE_DEATH) {
            d2_realm_client_unit_set_mode(unit, MONMODE_NEUTRAL);
        }
        return 1;
    }
    case MonsterAction::AttackAt: {
        // Aim at the spot when it is set and is not the unit's own, then take the code's mode.
        if (!unit->pDynamicPath) return 0;
        PATH_ResetToPreviousType(unit->pDynamicPath);
        UNITS_SetUsedSkill(unit, nullptr);
        UNITS_SetTargetUnitForDynamicUnit(unit, nullptr);
        const int x = static_cast<int>(pkt_word(l, 0)), y = static_cast<int>(pkt_word(l, 1));
        if ((x || y) && (x != UNITS_GetXPosition(unit) || y != UNITS_GetYPosition(unit)))
            UNITS_UpdateDirectionAndSpeed(unit, x, y);
        unit->dwFlags &= ~0x40u;
        return pkt_monster_mode(monster, code, static_cast<uint32_t>(d2::realm::monsterActionMode(code)));
    }
    case MonsterAction::AttackOn: {
        // The target unit becomes the path's target and the monster faces it; an unknown target
        // clears the relation and the mode is still set.
        if (!unit->pDynamicPath) return 0;
        PATH_ResetToPreviousType(unit->pDynamicPath);
        UNITS_SetUsedSkill(unit, nullptr);
        D2UnitStrc* target = find_realm_client_unit(static_cast<uint8_t>(pkt_word(l, 0)), pkt_word(l, 1));
        if (target && (target == unit || !target->pDynamicPath)) target = nullptr;
        UNITS_SetTargetUnitForDynamicUnit(unit, target);
        if (target) UNITS_UpdateDirectionAndSpeed(unit, UNITS_GetXPosition(target), UNITS_GetYPosition(target));
        unit->dwFlags &= ~0x40u;
        return pkt_monster_mode(monster, code, static_cast<uint32_t>(d2::realm::monsterActionMode(code)));
    }
    case MonsterAction::Unhandled:
        break;
    }
    return 0;
}

// After the action, a move or attack packet's velocity (the server's STAT_VELOCITYPERCENT total, which carries a
// run's AI bonus) becomes the client unit's total, and the path velocity is recomputed from it.
static int pkt_monster_action(D2RealmClientMonster& monster, uint32_t code, const uint32_t* l) {
    const int result = pkt_monster_action_case(monster, code, l);
    D2UnitStrc* const unit = monster.motionUnit;
    const int32_t velocity = static_cast<int32_t>(l[4]);
    if ((g_pktOpcode == 0x67 || g_pktOpcode == 0x68) && code != 0x13 && code != 0x15 && code != 0x16 &&
            unit && velocity) {
        const int32_t current = STATLIST_UnitGetStatValue(unit, STAT_VELOCITYPERCENT, 0);
        if (current != velocity) {
            STATLIST_SetUnitStat(unit, STAT_VELOCITYPERCENT,
                STATLIST_GetUnitBaseStat(unit, STAT_VELOCITYPERCENT, 0) + velocity - current, 0);
            UNITS_UpdateAnimRateAndVelocity(unit, __FILE__, __LINE__);
        }
    }
    return result;
}

// 0x2A750: ECX = action code, EDX = unit, the request block on the stack.
static void pkt_action(uint32_t code, uintptr_t unitBits, const uint32_t* l) {
    D2UnitStrc* unit = reinterpret_cast<D2UnitStrc*>(unitBits);
    code &= 0xff;
    g_pktResult = 0;
    if (!unit) return;
    if (unit->dwUnitType == UNIT_PLAYER) {
        g_pktResult = pkt_player_action(unit, code, l);
    } else if (unit->dwUnitType == UNIT_MONSTER && g_pktMonster && g_pktMonster->motionUnit == unit) {
        g_pktResult = pkt_monster_action(*g_pktMonster, code, l);
        refresh_monster_motion_view(*g_pktMonster);
    }
}

// The 0x0C handler takes the 0x2A750 call with the unit as a 32-bit register; the unit is the
// one the record named, held here.
static uint32_t pkt_action_af5f0(uint32_t code, uint32_t, uint32_t* l, uint32_t) {
    pkt_action(code, reinterpret_cast<uintptr_t>(g_pktUnit), l);
    return static_cast<uint32_t>(g_pktResult);
}

// One record: < 0 malformed, else the count of units it changed.
static int apply_retail_unit_packet(const uint8_t* packet, int length) {
    const d2::realm::PktRoute route = d2::realm::pktRoute(packet[0]);
    if (route.handler == d2::realm::PktHandler::None || length != route.length) return -1;
    uint32_t guid = 0;
    std::memcpy(&guid, packet + route.guidOffset, sizeof guid);
    D2RealmClientMonster* monster = nullptr;
    D2UnitStrc* unit = nullptr;
    const uint8_t unitType = route.typed ? packet[1] : static_cast<uint8_t>(UNIT_MONSTER);
    if (unitType == UNIT_MONSTER) {
        for (auto& candidate : g_monsters)
            if (candidate.view.guid == guid) { monster = &candidate; break; }
        if (!monster || !create_monster_motion(*monster)) return 0;
        unit = monster->motionUnit;
    } else {
        unit = find_realm_client_unit(unitType, guid);
    }
    if (!unit) return 0;
    g_pktEnv.f2A750 = pkt_action;
    g_pktEnv.f2A1B0 = pkt_place;
    g_pktEnv.getStat = pkt_get_stat;
    g_pktEnv.setStat = pkt_set_stat;
    g_pktRoute = route;
    g_pktMonster = monster;
    g_pktUnit = unit;
    g_pktOpcode = packet[0];
    g_pktResult = 0;
    const uintptr_t bits = reinterpret_cast<uintptr_t>(unit);
    using d2::realm::PktHandler;
    switch (route.handler) {
    case PktHandler::AF5F0: d2c_port_af5f0(0, packet, pkt_action_af5f0); break;
    case PktHandler::AF510: d2c_af510(&g_pktEnv, bits, packet); break;
    case PktHandler::AF4C0: d2c_af4c0(&g_pktEnv, bits, packet); break;
    case PktHandler::AF460: d2c_af460(&g_pktEnv, bits, packet); break;
    case PktHandler::AF3F0: d2c_af3f0(&g_pktEnv, bits, packet); break;
    case PktHandler::AF390: d2c_af390(&g_pktEnv, bits, packet); break;
    case PktHandler::AF340: d2c_af340(&g_pktEnv, bits, packet); break;
    case PktHandler::AF7C0: d2c_af7c0(&g_pktEnv, bits, packet); break;
    case PktHandler::AF750: d2c_af750(&g_pktEnv, bits, packet); break;
    case PktHandler::AF2B0: d2c_af2b0(&g_pktEnv, bits, packet); break;
    case PktHandler::None: break;
    }
    g_pktMonster = nullptr;
    g_pktUnit = nullptr;
    return g_pktResult;
}


// Objects the server assigns (0x51 reaches the creator at D2Client+0xA75C0). The record is what the creator is
// handed; its mode follows the object's client unit.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_object_count() { return static_cast<int>(g_objects.size()); }
EMSCRIPTEN_KEEPALIVE const D2RealmClientObjectView* d2_realm_client_objects() { return g_objects.data(); }

// The object units themselves, for the scene: one record of kClientObjectViewBytes per unit
// (d2gs_client_object_view), in the object table's order. Returns the count written.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_object_units(uint8_t* out, int cap) {
    int n = 0;
    for (D2UnitStrc* unit = d2gs_client_object_next(nullptr); unit && out && n < cap; unit = d2gs_client_object_next(unit), ++n)
        d2gs_client_object_view(unit, out + n * kClientObjectViewBytes);
    return n;
}
// Takes the pending object activations: {version, object id, player id, shrine kind, sound row, effect} each.
// The sound plays at the player. Effect 1 has turned the player's name around; 2..4 (the fire, exploding and
// poison shrines) have thrown their missiles, which are client missiles like any other from then on.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_take_object_events(int32_t* out, int cap) {
    if (!out || cap <= 0) return 0;
    const int count = std::min(cap, static_cast<int>(g_objectEvents.size()));
    for (int i = 0; i < count; ++i) {
        const auto& event = g_objectEvents[static_cast<size_t>(i)];
        int32_t* r = out + i * 6;
        r[0] = 1; r[1] = static_cast<int32_t>(event.objectGuid); r[2] = static_cast<int32_t>(event.playerGuid);
        r[3] = static_cast<int32_t>(event.shrine); r[4] = static_cast<int32_t>(event.sound); r[5] = static_cast<int32_t>(event.effect);
    }
    g_objectEvents.erase(g_objectEvents.begin(), g_objectEvents.begin() + count);
    return count;
}
// 0x0E and 0x4D aimed at an object. The activating player is looked up among the players this client knows.
static int apply_object_packet(const uint8_t* packet, int length) {
    D2gsClientObjectEvent event = {};
    const int result = d2gs_client_object_packet(packet, length,
        [](uint32_t guid) -> D2UnitStrc* { return find_realm_client_unit(UNIT_PLAYER, guid); }, &event);
    if (result == 1) {
        uint32_t guid = 0;
        std::memcpy(&guid, packet + 2, sizeof guid);
        sync_object_view(d2gs_client_object_find(guid));
    } else if (result == 2) {
        // The player unit here carries no name of its own: the name the client shows is the player record's.
        if (event.effect == kObjectEffectReverseName)
            for (auto& player : g_players)
                if (player.view.active && player.view.guid == event.playerGuid)
                    d2gs_client_object_reverse_name(reinterpret_cast<char*>(player.view.name));
        // 1.13c D2Client+0x26060 / +0x25F10 / +0x25DC0: the fire, exploding and poison shrines throw their
        // missiles in the activating player's name, through the client's own missile maker.
        D2UnitStrc* object = d2gs_client_object_find(event.objectGuid);
        D2UnitStrc* user = find_realm_client_unit(UNIT_PLAYER, event.playerGuid);
        D2gsClientObjectMissile thrown[16];
        const int count = object && user ? d2gs_client_object_effect_missiles(object, event.effect,
            STATLIST_UnitGetStatValue(user, STAT_LEVEL, 0), thrown, 16) : 0;
        for (int i = 0; i < count; ++i) {
            D2MissileStrc params{};
            params.dwFlags = thrown[i].flags;
            params.pOwner = user;
            if (!(thrown[i].flags & 1u)) params.pOrigin = object;
            params.nMissile = thrown[i].missile;
            params.nX = thrown[i].x; params.nY = thrown[i].y;
            params.nTargetX = thrown[i].targetX; params.nTargetY = thrown[i].targetY;
            if (thrown[i].flags & 0x100u) params.nGfxArg = 1;
            params.nSkillLevel = thrown[i].level;
            create_client_missile(&params, true);
        }
        if (g_objectEvents.size() == 64) g_objectEvents.erase(g_objectEvents.begin());
        g_objectEvents.push_back(event);
    }
    return result < 0 ? -1 : result ? 1 : 0;
}

// 0x51: D2Client+0xADF40 hands its fields to the creator (0xA75C0).
static void object_create_cb(uint32_t kind, uint32_t guid, uint32_t classId, uint32_t x, uint32_t y,
        uint32_t, uint32_t mode, uint32_t interact) {
    if (kind == UNIT_OBJECT) {
        destroy_object_unit(guid);
        if (D2UnitStrc* unit = d2gs_client_object_create(guid, classId, mode, static_cast<uint8_t>(interact))) {
            d2gs_client_object_place(unit, d2_drlg_client_active_room(static_cast<int>(x), static_cast<int>(y)),
                static_cast<int>(x), static_cast<int>(y));
        }
    }
    for (auto& object : g_objects)
        if (object.unitType == kind && object.guid == guid) {
            object = { kind, guid, classId, x, y, mode, interact };
            return;
        }
    g_objects.push_back({ kind, guid, classId, x, y, mode, interact });
}

// 0x0A: D2Client+0xACA40 hands (row, bucket, guid, type) to the unit-table remove (0xA6EF0). The
// removal applies to the projection that owns the type.
static const uint8_t* g_removePacket = nullptr;
static int g_removeLength = 0;
static int g_removeResult = 0;
static void game_packet_unit_removed(uint32_t guid);
static void unit_remove_cb(uintptr_t, uint32_t, uint32_t guid, uint32_t type) {
    drop_unit_queue(static_cast<uint8_t>(type), guid);
    if (type == UNIT_PLAYER) {
        g_removeResult = remove_player(g_removePacket, g_removeLength) < 0 ? -2 : 0;
    } else if (type == UNIT_MONSTER) {
        if (!remove_monster(g_removePacket, g_removeLength)) g_removeResult = -4;
        else { g_removeResult = 1; game_packet_unit_removed(guid); }
    } else if (type == UNIT_OBJECT) {
        destroy_object_unit(guid);
        g_objects.erase(std::remove_if(g_objects.begin(), g_objects.end(), [guid](const auto& object) {
            return object.guid == guid; }), g_objects.end());
    } else if (type == UNIT_ITEM) {
        g_removeResult = d2_realm_client_item_remove(guid);
    } else if (type == UNIT_TILE) {
        d2_realm_client_warps_remove(g_warps, guid);
    }
}

// 0x0E and 0x17: the unit-queue handlers (D2Client+0xAF560 / +0xAF240) resolve the named unit and hand
// the request to the action dispatch. An object target takes its mode change (selector 3) and its activation
// (selector 0x15) on its own client unit.
// The 0x17 handler reads the u16 at the address that the word at +8 names, and the same for the word at +10; the two
// loaded u16s are the pair it passes on. So those words are client-memory addresses, not packet data: below 0x10000 the
// read is unmapped and the client faults. No server sender of 0x17 is in the client's table of well-formed packets.
// GUESS: 0 stands in for both reads; the native capture cannot run that branch.
static uint16_t unit_queue_rd16(uint32_t) { return 0; }
static int apply_unit_queue_packet(const uint8_t* packet, int length) {
    const bool is0e = packet[0] == 0x0e;
    if (length != 12) return -1;
    uint32_t guid = 0;
    std::memcpy(&guid, packet + 2, sizeof guid);
    if (is0e && packet[1] == UNIT_OBJECT) {
        // D2Client+0xAF560, object branch: byte 6 == 3 sets the object's mode to the dword at +8.
        if (packet[6] != 3) return 0;
        uint32_t mode = 0;
        std::memcpy(&mode, packet + 8, sizeof mode);
        for (auto& object : g_objects)
            if (object.guid == guid) {
                object.mode = mode;
                return 1;
            }
        return 0;
    }
    D2RealmClientMonster* monster = nullptr;
    D2UnitStrc* unit = nullptr;
    if (packet[1] == UNIT_OBJECT) return is0e ? apply_object_packet(packet, length) : 0;
    if (packet[1] == UNIT_MONSTER) {
        for (auto& candidate : g_monsters)
            if (candidate.view.guid == guid) { monster = &candidate; break; }
        if (!monster || !create_monster_motion(*monster)) return 0;
        unit = monster->motionUnit;
    } else {
        unit = find_realm_client_unit(packet[1], guid);
    }
    if (!unit) return 0;
    g_pktEnv.f2A750 = pkt_action;
    g_pktRoute = {};
    g_pktMonster = monster;
    g_pktUnit = unit;
    g_pktOpcode = packet[0];
    g_pktResult = 0;
    if (is0e) {
        d2c_af560(&g_pktEnv, reinterpret_cast<uintptr_t>(unit), packet);
    } else {
        D2cPlayerEnv env{ &g_pktEnv, 0, unit_queue_rd16, [] {} };
        if (g_selectedPlayer >= 0 && size_t(g_selectedPlayer) < g_players.size())
            env.localPlayer = reinterpret_cast<uintptr_t>(g_players[g_selectedPlayer].motionUnit);
        d2c_af240(&env, reinterpret_cast<uintptr_t>(unit), packet);
    }
    g_pktMonster = nullptr;
    g_pktUnit = nullptr;
    return g_pktResult;
}

// The selector-6 arm writes the record's hit-class byte to D2UnitStrc+0xB0 (retail:
// 0x199DF mov [ebx+0xB0],edx); the wasm64 struct widens the 18 pointers before it.
static_assert(offsetof(D2UnitStrc, dwLastHitClass) == 0xB0 + 18 * (sizeof(void*) - 4),
    "D2UnitStrc dwLastHitClass is the 0x0D hit-class slot");

// 1.13c D2Client+0xAF590: the D2GS 0x0D receiver. It builds a
// three-field record {x, y, hitClass}, hands it to FUN_6FADA750 with the record's action selector
// (byte +6), then stores the record's life byte on the client-unit-list node its by-id lookup
// returns. Selector 6 is the GETHIT column of FUN_6FAC9830; every other selector reaches its own
// client handler and is not projected here.
static int apply_player_stop(const uint8_t* packet, int length) {
    d2::realm::D2RealmPlayerStop stop{};
    if (d2::realm::decodePlayerStopPacket(packet, length, &stop) != 1) return -1;
    if (stop.unitType == UNIT_PLAYER && stop.action >= 7 && stop.action <= 9) {
        D2UnitStrc* const player = find_realm_client_unit(stop.unitType, stop.guid);
        if (!player) return 0;
        return apply_player_life_action(player, stop.action, stop.x, stop.y, stop.hitClass) > 0 ? 1 : 0;
    }
    const int mode = d2::realm::playerStopModeFromAction(stop.action);
    if (mode < 0) return 0;
    D2UnitStrc* const unit = find_realm_client_unit(stop.unitType, stop.guid);
    if (!unit) return 0;
    // Selector 6 first writes the record's third field into the unit's last-hit-class slot
    // (D2Client+0x19830: mov [ebx+0xB0], the zero-extended packet byte at +11), then sets the mode.
    unit->dwLastHitClass = stop.hitClass;
    const uint32_t previous = unit->dwAnimMode;
    if (d2_realm_client_unit_set_mode(unit, static_cast<uint32_t>(mode)) < 0) return -1;
    if (previous != unit->dwAnimMode) emit_hit_overlay(unit);
    return 1;
}

// 0x9C and 0x9D: every item record goes to the one item runtime, which seats the item on the unit
// the record names (the selected player's own unit for 0x9C).
static int apply_item_record(const uint8_t* packet, int length) {
    D2UnitStrc* hero = g_selectedPlayer >= 0 && size_t(g_selectedPlayer) < g_players.size()
        ? g_players[g_selectedPlayer].motionUnit : nullptr;
    D2UnitStrc* owner = nullptr;
    if (packet[0] == 0x9d) {
        D2RealmOwnedItemPacket owned{};
        if (d2_realm_d2gs_parse_owned_item(packet, length, &owned) != length) return -1;
        if (owned.ownerType == UNIT_PLAYER) owner = find_realm_client_unit(owned.ownerType, owned.ownerGuid);
    }
    return d2_realm_client_item_packet(hero, owner, packet, length);
}

static bool read_state_stats(D2RealmBitReader& bits, std::vector<D2RealmClientStateStat>& stats) {
    if (!sgptDataTables || !sgptDataTables->pItemStatCostTxt ||
        sgptDataTables->nItemStatCostTxtRecordCount <= 0) return false;
    uint32_t statId = 0;
    if (!bits.read(9, statId)) return false;
    while (statId != 0x1ff) {
        if (statId >= static_cast<uint32_t>(sgptDataTables->nItemStatCostTxtRecordCount)) return false;
        const D2ItemStatCostTxt& cost = sgptDataTables->pItemStatCostTxt[statId];
        // The local client stops on the first zero-width StatCost row, as it does for 0xAC.
        if (cost.nSendBits == 0) return true;
        if (cost.nSendBits > 32 || cost.nSendParamBits > 32) return false;
        int32_t layer = 0;
        int32_t value = 0;
        if (cost.nSendParamBits && !bits.readSigned(cost.nSendParamBits, layer)) return false;
        if (cost.dwItemStatFlags & ITEMSTATCOSTFLAG_SIGNED) {
            if (!bits.readSigned(cost.nSendBits, value)) return false;
        } else {
            uint32_t raw = 0;
            if (!bits.read(cost.nSendBits, raw)) return false;
            value = static_cast<int32_t>(raw);
        }
        stats.push_back({static_cast<uint16_t>(statId), static_cast<uint16_t>(layer), value});
        if (!bits.read(9, statId)) return false;
    }
    return true;
}

static bool decode_state_set(const uint8_t* packet, int length, D2RealmClientStateUpdate& update) {
    if (!packet || packet[0] != 0xa8 || length < 8 || packet[6] != length) return false;
    update.state = packet[7];
    update.hasStats = true;
    D2RealmBitReader bits(packet + 8, length - 8);
    return read_state_stats(bits, update.stats);
}

static bool decode_state_add(const uint8_t* packet, int length, std::vector<D2RealmClientStateUpdate>& updates) {
    if (!packet || packet[0] != 0xaa || length < 7 || packet[6] != length) return false;
    D2RealmBitReader bits(packet + 7, length - 7);
    for (;;) {
        uint32_t state = 0;
        if (!bits.read(8, state)) return false;
        if (state == 0xff) return true;
        uint32_t hasStats = 0;
        if (!bits.read(1, hasStats)) return false;
        D2RealmClientStateUpdate update{};
        update.state = static_cast<uint8_t>(state);
        update.hasStats = hasStats != 0;
        if (update.hasStats && !read_state_stats(bits, update.stats)) return false;
        updates.push_back(std::move(update));
    }
}

static bool apply_state_on(D2UnitStrc* unit, uint8_t state) {
    if (!unit || !sgptDataTables || state >= sgptDataTables->nStatesTxtRecordCount) return false;
    if (!unit->pStatListEx) STATLIST_AllocStatListEx(unit, 0, nullptr, nullptr);
    if (!unit->pStatListEx) return false;
    STATES_ToggleState(unit, state, TRUE);
    return true;
}

// 1.13c D2Client+0x73760, called by the 0xA7, 0xA8 and 0xAA handlers before the state is set: a state
// the unit does not carry yet plays its onsound.
static void record_state_on_sound(D2UnitStrc* unit, uint8_t state) {
    const int32_t row = d2::realm::clientStateOnSoundRow(unit, state);
    if (row >= 0) record_unit_fx(d2::realm::kUnitFxSound, unit, static_cast<uint32_t>(row), 0, d2::realm::kUnitFxSkillState);
}

static int apply_state_packet(const uint8_t* packet, int length) {
    if (!packet || length < 7) return -1;
    const uint8_t type = packet[1];
    const uint32_t guid = static_cast<uint32_t>(packet[2]) |
        (static_cast<uint32_t>(packet[3]) << 8) | (static_cast<uint32_t>(packet[4]) << 16) |
        (static_cast<uint32_t>(packet[5]) << 24);
    D2UnitStrc* unit = find_realm_client_unit(type, guid);
    if (!unit) return 0; // The native type/GUID lookup miss leaves client state unchanged.

    if (packet[0] == 0xa7 || packet[0] == 0xa9) {
        if (length != 7) return -1;
        const uint8_t state = packet[6];
        if (!sgptDataTables || state >= sgptDataTables->nStatesTxtRecordCount) return 0;
        if (!unit->pStatListEx) STATLIST_AllocStatListEx(unit, 0, nullptr, nullptr);
        if (!unit->pStatListEx) return -1;
        if (packet[0] == 0xa7) {
            record_state_on_sound(unit, state);
            STATES_ToggleState(unit, state, TRUE);
        } else {
            D2StatListStrc* stateList = STATLIST_GetStatListFromUnitAndState(unit, state);
            if (stateList) STATLIST_FreeStatList(stateList);
            STATES_ToggleState(unit, state, FALSE);
        }
        return 1;
    }

    D2RealmClientStateUpdate single{};
    std::vector<D2RealmClientStateUpdate> updates;
    if (packet[0] == 0xa8) {
        if (!decode_state_set(packet, length, single)) return -1;
        updates.push_back(std::move(single));
    } else if (packet[0] == 0xaa) {
        if (!decode_state_add(packet, length, updates)) return -1;
    } else {
        return 0;
    }

    int applied = 0;
    for (const auto& update : updates) {
        if (!sgptDataTables || update.state >= sgptDataTables->nStatesTxtRecordCount) continue;
        record_state_on_sound(unit, update.state);
        if (!apply_state_on(unit, update.state)) return -1;
        if (update.hasStats &&
            !d2gs_client_apply_state_stats(unit, update.state, update.stats.data(), update.stats.size()))
            return -1;
        applied++;
    }
    return applied;
}

// Client-created units have a local identity space; no D2GS packet assigns missile GUIDs.
struct RealmClientMissile { D2UnitStrc* unit; uint32_t level; };
static std::vector<RealmClientMissile> g_clientMissiles;
static uint32_t g_nextClientMissileId = 1;
// 1.13c D2Client+0xDE834 (0x6FB8E834): the client unit-creation counter, incremented by the CUnit constructor wrapper
// (D2Client+0xA7BF0) and stored as the new unit's id. A session's earlier creations (tile/object/item units the test runner
// does not model) have already advanced it; the caller pins the value it read, the next missile takes counter+1.
EMSCRIPTEN_KEEPALIVE void d2_realm_client_unit_counter_set(uint32_t counter) { g_nextClientMissileId = counter + 1; }

static void clear_target_relations_to(D2UnitStrc* target) {
    if (!target) return;
    for (auto& player : g_players) {
        d2::realm::clearTargetRelationTo(target, player.motionUnit,
            [](D2UnitStrc* unit) { return unit->pDynamicPath; }, PATH_GetTargetUnit,
            [](D2UnitStrc* unit) { UNITS_SetTargetUnitForDynamicUnit(unit, nullptr); });
    }
    for (auto& monster : g_monsters) {
        d2::realm::clearTargetRelationTo(target, monster.motionUnit,
            [](D2UnitStrc* unit) { return unit->pDynamicPath; }, PATH_GetTargetUnit,
            [](D2UnitStrc* unit) { UNITS_SetTargetUnitForDynamicUnit(unit, nullptr); });
    }
    for (auto& missile : g_clientMissiles) {
        d2::realm::clearTargetRelationTo(target, missile.unit,
            [](D2UnitStrc* unit) { return unit->pDynamicPath; }, PATH_GetTargetUnit,
            [](D2UnitStrc* unit) { UNITS_SetTargetUnitForDynamicUnit(unit, nullptr); });
    }
}
// Native generic SC casts dispatch from loaded Skills/Missiles records.
static bool is_natural_client_projectile(int id) {
    auto* skill = DATATBLS_GetSkillsTxtRecord(id);
    if (!skill || skill->wCltStFunc != 0 || skill->wCltDoFunc != 0 ||
        skill->nAnim != PLRMODE_CAST || skill->wCltMissile == 0 || skill->wCltMissile == 0xffffu) return false;
    auto* missile = DATATBLS_GetMissilesTxtRecord(skill->wCltMissile);
    if (!missile || missile->wCltDoFunc != 1 || missile->nCollideType != 3) return false;
    if (missile->wCltHitFunc == 0) return true;
    return id == SKILL_FIREBALL && skill->wCltMissile == 62 && missile->wCltHitFunc == 1;
}

// A skill whose "do" at the action frame is the generic one (1.13c D2Client+0xA2C90 with no
// cltdofunc): its client missile leaves the unit toward the path's target. This adds, to the plain
// cast projectiles, the skills of any mode that start through a ported start function and whose
// missile is not lobbed. The missile itself must still be one the client missile path runs.
static bool is_client_launch_skill(int id) {
    if (is_natural_client_projectile(id)) return true;
    auto* skill = DATATBLS_GetSkillsTxtRecord(id);
    if (!skill || skill->wCltDoFunc != 0 || skill->wCltMissile == 0 || skill->wCltMissile == 0xffffu ||
        (skill->dwFlags[0] & 2u)) return false; // bit 1 of the row's flags: a lobbed missile takes another path
    if (!d2client_realm_skill_start_ported(skill->wCltStFunc)) return false;
    auto* missile = DATATBLS_GetMissilesTxtRecord(skill->wCltMissile);
    // Hit function 1 (1.13c D2Client+0xBC1A0, Fire Ball's and Exploding Arrow's burst) is ported.
    return missile && missile->wCltDoFunc == 1 && missile->nCollideType == 3 &&
        (missile->wCltHitFunc == 0 || missile->wCltHitFunc == 1);
}

EMSCRIPTEN_KEEPALIVE int d2_realm_client_projectile_supported(int skillId) {
    return is_natural_client_projectile(skillId) ? 1 : 0;
}

struct RealmMissileEvent { uint32_t version,event,tick,id,classId,ownerGuid,skill,targetType,targetGuid; };
static_assert(sizeof(RealmMissileEvent) == 36);
static std::vector<RealmMissileEvent> g_clientMissileEvents;
static uint32_t g_clientMissileTick = 0;
// 1.13c D2Client+0xB1420: a missile with Missiles.txt NextHit puts state 0x56 on the unit it hits for
// NextDelay frames; while it is held, no NextHit missile hits that unit again.
struct ClientNextHit { uint32_t type, id, until; };
static std::vector<ClientNextHit> g_clientNextHits;
static bool client_next_hit_held(const D2UnitStrc* target) {
    for (const auto& n : g_clientNextHits)
        if (n.type == target->dwUnitType && n.id == target->dwUnitId && g_clientMissileTick < n.until) return true;
    return false;
}
struct RealmMissileSoundEvent { uint32_t version, tick, missileId, classId, event, soundRow; };
static_assert(sizeof(RealmMissileSoundEvent) == 24);
static std::vector<RealmMissileSoundEvent> g_clientMissileSoundEvents;

// What a unit's skill start, hit or server record plays, for the page to sound and draw: the one
// queue of d2gs_client_unit_fx.h (its kinds are listed there).
static d2::realm::UnitFxQueue g_unitFxEvents;
// The one-shot overlays still running, so a start can tell that a unit already carries a row.
struct LiveUnitOverlay { uint32_t unitType, unitGuid, row, endTick; };
static std::vector<LiveUnitOverlay> g_unitOverlays;

static void record_unit_fx(uint32_t kind, D2UnitStrc* unit, uint32_t row, uint32_t delay, uint32_t skill) {
    g_unitFxEvents.push({1, g_clientMissileTick, kind, static_cast<uint32_t>(unit->dwUnitType),
        unit->dwUnitId, row, delay, skill});
}

static bool unit_carries_overlay(D2UnitStrc* unit, uint32_t row) {
    g_unitOverlays.erase(std::remove_if(g_unitOverlays.begin(), g_unitOverlays.end(),
        [](const LiveUnitOverlay& o) { return o.endTick <= g_clientMissileTick; }), g_unitOverlays.end());
    return std::any_of(g_unitOverlays.begin(), g_unitOverlays.end(), [unit, row](const LiveUnitOverlay& o) {
        return o.unitType == static_cast<uint32_t>(unit->dwUnitType) && o.unitGuid == unit->dwUnitId && o.row == row;
    });
}

// A one-shot overlay on a unit (1.13c D2Client+0x6B300). The caller has bounds-checked the row.
// NOT PORTED: the start's own bookkeeping for rows below 0x8C (+0x69660) is not read.
static void start_unit_overlay(D2UnitStrc* unit, int row, uint32_t skill) {
    auto* txt = DATATBLS_GetOverlayTxtRecord(row);
    if (!txt) return;
    record_unit_fx(2, unit, static_cast<uint32_t>(row), 0, skill);
    g_unitOverlays.push_back({static_cast<uint32_t>(unit->dwUnitType), unit->dwUnitId, static_cast<uint32_t>(row),
        d2::realm::overlayEndTick(g_clientMissileTick, txt->dwFrames, txt->dwAnimRate)});
}

// A unit that has just entered death, get hit or knockback plays the overlay its last hit class
// names (1.13c D2Client+0x2A590). The hit action names the knockback mode itself whatever the
// unit is doing (`struckNow`).
static void emit_hit_overlay(D2UnitStrc* unit, bool struckNow) {
    if (!unit) return;
    const uint32_t mode = unit->dwAnimMode;
    const bool struck = struckNow || (unit->dwUnitType == UNIT_PLAYER
        ? mode == PLRMODE_DEATH || mode == PLRMODE_GETHIT || mode == PLRMODE_KNOCKBACK
        : unit->dwUnitType == UNIT_MONSTER &&
            (mode == MONMODE_DEATH || mode == MONMODE_GETHIT || mode == MONMODE_KNOCKBACK));
    if (!struck) return;
    const int row = d2::realm::hitOverlayRow(unit->dwLastHitClass);
    if (row <= 0 || row >= sgptDataTables->nOverlayTxtRecordCount) return;
    start_unit_overlay(unit, row, d2::realm::kUnitFxSkillHit);
}

// The hit action (0x13) of the unit request: the unit keeps its mode, plays its hit class's
// overlay and is reported struck so the page sounds the impact.
// NOT PORTED: the monster's blood splatter (+0x4C500), its flag bit from the life byte's top bit
// and the player's placement at the record's two bytes.
static void record_unit_hit(D2UnitStrc* unit) {
    emit_hit_overlay(unit, true);
    record_unit_fx(d2::realm::kUnitFxHit, unit, unit->dwLastHitClass, 0, d2::realm::kUnitFxSkillHit);
}

// Consumes the queued unit effects, eight words each:
// {version, tick, kind, unitType, unitGuid, row, delay, skill}.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_take_unit_fx(int32_t* out, int cap) {
    return g_unitFxEvents.take(out, cap);
}

// A unit's life, maximum life (both in 1/256) and last hit class into out[0..2]; 0 when the
// client holds no such unit.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_unit_life(int type, uint32_t guid, uint32_t* out) {
    if (!out || (type != UNIT_PLAYER && type != UNIT_MONSTER)) return 0;
    auto* unit = find_realm_client_unit(static_cast<uint8_t>(type), guid);
    if (!unit || !unit->pStatListEx) return 0;
    out[0] = static_cast<uint32_t>(STATLIST_UnitGetStatValue(unit, 6, 0));
    out[1] = static_cast<uint32_t>(STATLIST_UnitGetStatValue(unit, 7, 0));
    out[2] = unit->dwLastHitClass;
    return 1;
}

EMSCRIPTEN_KEEPALIVE uint32_t d2_realm_client_unit_fx_tick() { return g_clientMissileTick; }

// The hit class a unit was last struck with (the server's hit records set it), or 0.
EMSCRIPTEN_KEEPALIVE uint32_t d2_realm_client_unit_hit_class(int type, uint32_t guid) {
    if (type != UNIT_PLAYER && type != UNIT_MONSTER) return 0;
    auto* unit = find_realm_client_unit(static_cast<uint8_t>(type), guid);
    return unit ? unit->dwLastHitClass : 0;
}
static void record_client_missile_event(uint32_t event, D2UnitStrc* unit,
        uint32_t targetType = 0xffffffffu, uint32_t targetGuid = 0xffffffffu) {
    if (!unit) return;
    if (g_clientMissileEvents.size() == 128) g_clientMissileEvents.erase(g_clientMissileEvents.begin());
    const bool isMissile = unit->dwUnitType == UNIT_MISSILE && unit->pMissileData;
    g_clientMissileEvents.push_back({1,event,g_clientMissileTick,unit->dwUnitId,
        static_cast<uint32_t>(unit->dwClassId), isMissile ? unit->pMissileData->dwOwnerGUID : unit->dwUnitId,
        isMissile ? static_cast<uint32_t>(MISSILE_GetSkill(unit)) :
            (UNITS_GetUsedSkill(unit) ? static_cast<uint32_t>(SKILLS_GetSkillIdFromSkill(UNITS_GetUsedSkill(unit), __FILE__, __LINE__)) : 0xffffffffu),targetType,targetGuid});
}
static void record_client_missile_impact_sound(uint32_t event, D2UnitStrc* unit,
        const D2MissilesTxt* txt) {
    if (!unit || !txt) return;
    const int32_t row = d2::realm::fireballImpactSoundRow(
        static_cast<uint32_t>(unit->dwClassId), event, static_cast<int16_t>(txt->wHitSound));
    if (row < 0) return;
    if (g_clientMissileSoundEvents.size() == 128)
        g_clientMissileSoundEvents.erase(g_clientMissileSoundEvents.begin());
    g_clientMissileSoundEvents.push_back({1, g_clientMissileTick, unit->dwUnitId,
        static_cast<uint32_t>(unit->dwClassId), event, static_cast<uint32_t>(row)});
}
static void free_client_missile(D2UnitStrc* unit) {
    if (!unit) return;
    d2_realm_client_motion_detach(unit);
    MISSILE_FreeMissileData(unit);
    UNITS_FreeUnit(unit);
}
static void clear_client_missiles() {
    for (const auto& entry : g_clientMissiles) { record_client_missile_event(5, entry.unit); free_client_missile(entry.unit); }
    g_clientMissiles.clear();
    g_clientNextHits.clear();
    g_clientMissileSoundEvents.clear();
    g_unitFxEvents.clear();
    g_unitOverlays.clear();
}

// Recovered table-driven ordinary flight and separate static impact265 descriptor.
// `general` is the maker as any client effect may ask for it (a shrine's missiles, a missile's hit children):
// every request flag the maker reads is honoured and the row need not belong to a skill. Without it only a
// natural skill projectile (flags 0x21) and Fire Ball's children (0x201) are made.
static D2UnitStrc* create_client_missile(const D2MissileStrc* params, bool general) {
    if (!params || !params->pOwner) return nullptr;
    if (!general && params->dwFlags != 0x21u && params->dwFlags != 0x201u) return nullptr;
    auto* owner = params->pOwner;
    const int missileId = params->nMissile;
    auto* txt = DATATBLS_GetMissilesTxtRecord(missileId);
    // Do rule 4 (the gas cloud) is the ordinary flight after a spark of its own; the spark is not made.
    if (!txt || (txt->wCltDoFunc != 1 && !(general && txt->wCltDoFunc == 4))) return nullptr;
    if (general) {
    } else if (params->dwFlags == 0x201u) {
        // 1.13c D2Client+0xB1420: a missile with hit function 1 (Fire Ball, Exploding Arrow) spawns its
        // hit children (265) and its Missiles.txt +0x16 ExplosionMissile through the same create path;
        // the request's skill is that missile's class.
        const auto* source = DATATBLS_GetMissilesTxtRecord(params->nSkill);
        if (!source || source->wCltHitFunc != 1 ||
            (missileId != 265 && missileId != static_cast<int>(source->wExplosionMissile))) return nullptr;
    } else {
        const auto* skill = DATATBLS_GetSkillsTxtRecord(params->nSkill);
        if (!skill || !is_client_launch_skill(params->nSkill) || skill->wCltMissile != missileId) return nullptr;
    }
    // 1.13c D2Client+0xB30F0: start, aim, speed and life from the request's flags and the row.
    d2::realm::MissileRequest request{};
    request.flags = params->dwFlags;
    request.hasOrigin = params->pOrigin != nullptr;
    if (params->pOrigin) { request.originX = UNITS_GetXPosition(params->pOrigin); request.originY = UNITS_GetYPosition(params->pOrigin); }
    request.x = params->nX; request.y = params->nY;
    request.targetX = params->nTargetX; request.targetY = params->nTargetY;
    request.velocity = params->nVelocity; request.level = params->nSkillLevel; request.loops = params->nLoops;
    request.frame = params->nFrame; request.range = params->nRange; request.activateFrame = params->nActivateFrame;
    const d2::realm::MissileRequestRow row{ txt->nVel, txt->nVelLev, static_cast<int16_t>(txt->wRange),
        static_cast<int16_t>(txt->wLevRange), txt->nActivate, txt->nSubLoop, txt->nSubStart, txt->nSubStop };
    d2::realm::MissileStart start{};
    if (!d2::realm::resolveMissileRequest(request, row, start)) return nullptr;
    const int x = start.x, y = start.y;
    D2UnitStrc* target = params->pTarget;
    int tx = start.targetX, ty = start.targetY;
    const int velocity = start.velocity;
    d2::realm::normalizeMovingMissileTarget(target, owner, velocity, x, y, tx, ty,
        [](D2UnitStrc* value) { return UNITS_GetXPosition(value); },
        [](D2UnitStrc* value) { return UNITS_GetYPosition(value); },
        [](D2UnitStrc* value) { return UNITS_GetDirection(value); });
    const int aimX = target ? UNITS_GetXPosition(target) : tx;
    const int aimY = target ? UNITS_GetYPosition(target) : ty;
    if (d2::realm::missileAimTooFar(velocity, x, y, aimX, aimY)) return nullptr;
    auto* unit = UNITS_AllocUnit(nullptr, UNIT_MISSILE);
    if (!unit) return nullptr;
    unit->dwClassId = missileId;
    unit->dwUnitId = g_nextClientMissileId++;
    unit->dwAnimMode = txt->nCollideType; // Client 6FB601BE passes collision type to #11090.
    if (!d2_realm_client_motion_attach(unit, x, y)) { free_client_missile(unit); return nullptr; }
    uint32_t level = 0, roomX = 0, roomY = 0, initSeed = 0;
    if (!d2_drlg_client_unit_context(x, y, 0, &level, &roomX, &roomY, &initSeed) ||
        !d2_session_client_cunit_seed_init(&unit->pSeed, &unit->dwInitSeed, x, y, 0)) {
        free_client_missile(unit); return nullptr;
    }
    MISSILE_AllocMissileData(unit);
    if (!unit->pMissileData) { free_client_missile(unit); return nullptr; }
    // Native type-3 initializer clears targetability and uses its collision-type table.
    unit->dwFlags = (unit->dwFlags & ~0xau) | 0x600020u;
    PATH_SetFootprintCollisionMask(unit->pDynamicPath, 0);
    PATH_SetMoveTestCollisionMask(unit->pDynamicPath,
        general ? d2::realm::missileCollideMask(txt->nCollideType) : txt->nCollideType == 3 ? 0x184 : 0);
    UNITS_SetAnimStartFrame(unit);
    unit->dwFrameCountPrecise = static_cast<uint32_t>(txt->nAnimLen) << 8;
    MISSILE_SetAnimSpeed(unit, static_cast<int>(txt->nAnimSpeed) << 4);
    const int levelValue = params->nSkillLevel;
    const int range = start.range;
    if (params->dwFlags & 0x200u) unit->nSeqCurrentFramePrecise = start.startFrame << 8;
    MISSILE_SetTotalFrames(unit, range);
    MISSILE_SetCurrentFrame(unit, range);
    MISSILE_SetActivateFrame(unit, range - start.activate);
    PATH_SetVelocity(unit->pDynamicPath, 0, __FILE__, __LINE__);
    if (target) UNITS_SetTargetUnitForDynamicUnit(unit, target);
    else D2COMMON_10170_PathSetTargetPos(unit->pDynamicPath, tx, ty);
    PATH_SetVelocity(unit->pDynamicPath, velocity, __FILE__, __LINE__);
    if (velocity) D2Common_10142(unit->pDynamicPath, unit, 0);
    if (txt->nInitSteps) unit->dwFlagEx |= 0x40000u;
    MISSILE_SetOwner(unit, owner);
    PATH_SetAcceleration(unit->pDynamicPath, txt->wAccel);
    PATH_SetMaxVelocity(unit->pDynamicPath, static_cast<int>(txt->nMaxVel) << 8);
    if (params->dwFlags & 0x400u) {
        // A thrown missile lives as long as its flight to the target takes.
        // Open: flag 0x100, the arc the thrown missile is drawn along, is not made; it flies level.
        const int frames = d2::realm::missileFlightFrames(UNITS_GetDistanceToCoordinates(unit, tx, ty), velocity);
        if (frames >= 0) { MISSILE_SetTotalFrames(unit, frames); MISSILE_SetCurrentFrame(unit, frames); }
    }
    MISSILE_SetSkill(unit, params->nSkill);
    MISSILE_SetLevel(unit, static_cast<uint16_t>(levelValue));
    g_clientMissiles.push_back({unit, level});
    record_client_missile_event(1, unit);
    if ((unit->dwClassId == 62 || unit->dwClassId == 42 || (general && txt->wCltHitFunc >= 2)) && txt->wTravelSound < 0x8000) // D2Client create: TravelSound row on the first client tick
        g_clientMissileSoundEvents.push_back({1, g_clientMissileTick, unit->dwUnitId, static_cast<uint32_t>(unit->dwClassId), 1u, txt->wTravelSound});
    return unit;
}

static D2UnitStrc* create_client_projectile(D2UnitStrc* owner, D2SkillStrc* skill) {
    if (!owner || !owner->pDynamicPath || !skill || !skill->pSkillsTxt ||
        !is_client_launch_skill(skill->pSkillsTxt->nSkillId)) return nullptr;
    D2MissileStrc params{};
    params.dwFlags = 0x21;
    params.pOwner = owner;
    params.nMissile = skill->pSkillsTxt->wCltMissile;
    params.nX = UNITS_GetXPosition(owner); params.nY = UNITS_GetYPosition(owner);
    params.pTarget = PATH_GetTargetUnit(owner->pDynamicPath);
    params.nTargetX = owner->pDynamicPath->tTargetCoord.X;
    params.nTargetY = owner->pDynamicPath->tTargetCoord.Y;
    if (params.pTarget) {
        if (!UNITS_GetRoom(params.pTarget)) return nullptr;
    } else if (!params.nTargetX || !params.nTargetY) return nullptr;
    params.nSkill = skill->pSkillsTxt->nSkillId;
    params.nSkillLevel = SKILLS_GetSkillLevel(owner, skill, TRUE);
    return create_client_missile(&params);
}

// What the ported do functions call back into (d2client/realm_skill_do.h).
static const RealmSkillDoHost& client_skill_do_host() {
    static const RealmSkillDoHost host{
        [](const D2MissileStrc* params) { return create_client_missile(params, true); },
        [](D2UnitStrc* unit, int row, int skillId) {
            record_unit_fx(d2::realm::kUnitFxSound, unit, static_cast<uint32_t>(row), 0, static_cast<uint32_t>(skillId));
        },
        [](int type, uint32_t guid) { return find_realm_client_unit(static_cast<uint8_t>(type), guid); },
    };
    return host;
}
struct RealmMissileCollision { D2UnitStrc* missile; D2UnitStrc* owner; uint32_t hitGuid = 0xffffffffu; };
static int __fastcall ordinary_projectile_target(D2UnitStrc* target, void* context) {
    auto* collision = static_cast<RealmMissileCollision*>(context);
    // 0xB1000: a target carrying the NextHit state is not a hit for a NextHit missile.
    if (target && collision->missile) {
        const auto* mtxt = DATATBLS_GetMissilesTxtRecord(collision->missile->dwClassId);
        if (mtxt && mtxt->nNextHit && client_next_hit_held(target)) return 0;
    }
    // Bounded player->ordinary-monster relation branch. Player/PvP and pets require
    // the Client relation roster and are not approximated from engine damage rules.
    if (!target || target->dwUnitType != UNIT_MONSTER || !collision->owner ||
        collision->owner->dwUnitType != UNIT_PLAYER || (target->dwFlags & 0xcu) != 0xcu ||
        MISSILE_CheckUnitIfOwner(collision->missile, target)) return 0;
    const auto it = std::find_if(g_monsters.begin(), g_monsters.end(), [target](const auto& entry) {
        return entry.motionUnit == target;
    });
    if (it == g_monsters.end() || it->ownerGuid != 0) return 0;
    if (STATLIST_AreUnitsAligned(target, collision->owner)) return 0;
    collision->hitGuid = target->dwUnitId;
    return 1;
}
static int __fastcall query_specific_monster(D2UnitStrc* target, void* context) {
    return target && target->dwUnitType == UNIT_MONSTER && target->dwUnitId == *static_cast<uint32_t*>(context);
}
// Read the real native collision query at this canonical target's current coordinates.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_monster_collision_query(uint32_t guid) {
    for (auto& monster : g_monsters) if (monster.view.guid == guid && monster.motionUnit) {
        auto* unit = monster.motionUnit;
        auto* hit = D2Common_10407(UNITS_GetRoom(unit), UNITS_GetXPosition(unit), UNITS_GetYPosition(unit),
            query_specific_monster, &guid, 1);
        return hit ? static_cast<int>(hit->dwUnitId) : -1;
    }
    return -1;
}

static bool create_fireball_hit_child(D2UnitStrc* source,
        const d2::realm::FireballHitChild& child, void*) {
    D2UnitStrc* owner = nullptr;
    for (auto& player : g_players) if (MISSILE_CheckUnitIfOwner(source, player.motionUnit)) {
        owner = player.motionUnit; break;
    }
    if (!owner) return false;
    D2MissileStrc params{};
    params.dwFlags = 0x201;
    params.pOwner = owner;
    params.nMissile = child.childClass;
    params.nX = child.x; params.nY = child.y;
    params.nSkill = child.sourceClass;
    params.nSkillLevel = child.sourceLevel;
    params.nFrame = child.startFrame;
    auto* created = create_client_missile(&params);
    if (created) record_client_missile_event(9, created, UNIT_MISSILE, source->dwUnitId);
    return created != nullptr;
}
// 1.13c D2Client+0xB1420: after the callback-table hit children, a Fire Ball that
// collided (or expired with AlwaysExplode) spawns Missiles.txt +0x16 at its own
// position. Negative +0x16 (signed) means no explosion, as in the native guard.
static void spawn_fireball_explosion(D2UnitStrc* missile) {
    const auto* fireball = DATATBLS_GetMissilesTxtRecord(missile->dwClassId);
    if (!fireball || static_cast<int16_t>(fireball->wExplosionMissile) < 0) return;
    D2UnitStrc* owner = nullptr;
    for (auto& player : g_players) if (MISSILE_CheckUnitIfOwner(missile, player.motionUnit)) {
        owner = player.motionUnit; break;
    }
    if (!owner) return;
    D2MissileStrc params{};
    params.dwFlags = 0x201;
    params.pOwner = owner;
    params.nMissile = fireball->wExplosionMissile;
    params.nX = UNITS_GetXPosition(missile); params.nY = UNITS_GetYPosition(missile);
    params.nSkill = static_cast<int>(missile->dwClassId);
    params.nSkillLevel = static_cast<int>(MISSILE_GetLevel(missile));
    params.nFrame = 0;
    auto* created = create_client_missile(&params);
    if (created) record_client_missile_event(9, created, UNIT_MISSILE, missile->dwUnitId);
}
// The player a missile belongs to; a hit child is made in that player's name.
static D2UnitStrc* client_missile_owner(D2UnitStrc* missile) {
    for (auto& player : g_players) if (MISSILE_CheckUnitIfOwner(missile, player.motionUnit)) return player.motionUnit;
    return nullptr;
}
// 1.13c D2Client+0xB1100 (hit rule 3) through +0xB0450: the stopped missile leaves its hit sub-missiles
// where it is. The exploding potion's burst is this.
static void spawn_missile_hit_children(D2UnitStrc* missile, const D2MissilesTxt* txt) {
    D2UnitStrc* owner = client_missile_owner(missile);
    const auto children = d2::realm::missileHitChildren(static_cast<int16_t>(txt->wCltHitSubMissile[0]),
        static_cast<int16_t>(txt->wCltHitSubMissile[1]), static_cast<int16_t>(txt->wCltHitSubMissile[2]),
        [missile](uint32_t span) {
            const uint64_t next = uint64_t(missile->pSeed.nLowSeed) * 0x6ac690c5ULL + missile->pSeed.nHighSeed;
            missile->pSeed.nLowSeed = static_cast<uint32_t>(next);
            missile->pSeed.nHighSeed = static_cast<uint32_t>(next >> 32);
            return missile->pSeed.nLowSeed % span;
        });
    if (!owner) return;
    for (int which = 0; which < 2; ++which) {
        const int childClass = which ? children.second : children.first;
        if (childClass < 0) break;
        D2MissileStrc params{};
        params.dwFlags = 0x20;
        params.pOwner = owner;
        params.pOrigin = missile;
        params.nMissile = childClass;
        params.nSkill = MISSILE_GetSkill(missile);
        params.nSkillLevel = static_cast<int>(MISSILE_GetLevel(missile));
        auto* created = create_client_missile(&params, true);
        if (!created) continue;
        if (which) created->dwFlagEx |= 0x10000u;
        record_client_missile_event(9, created, UNIT_MISSILE, missile->dwUnitId);
    }
}
// 1.13c D2Client+0xBB1B0 (hit rule 2) through +0xB6BC0: two interleaved rings of the first hit sub-missile
// fly out from where the missile stopped. The choking gas potion's cloud is this.
static void spawn_missile_hit_ring(D2UnitStrc* missile, const D2MissilesTxt* txt) {
    const int childClass = static_cast<int16_t>(txt->wCltHitSubMissile[0]);
    const auto* childTxt = childClass >= 0 ? DATATBLS_GetMissilesTxtRecord(childClass) : nullptr;
    D2UnitStrc* owner = client_missile_owner(missile);
    if (!childTxt || !owner) return;
    d2::realm::MissileRingChild ring[31];
    const int count = d2::realm::missileRingChildren(static_cast<int>(txt->dwCltHitPar[0]),
        static_cast<int>(txt->dwCltHitPar[1]), static_cast<int>(txt->dwCltHitPar[2]),
        static_cast<int>(childTxt->dwParam[0]), static_cast<int>(childTxt->dwParam[1]), ring);
    for (int i = 0; i < count; ++i) {
        D2MissileStrc params{};
        params.dwFlags = ring[i].flags;
        params.pOwner = owner;
        params.pOrigin = missile;
        params.nMissile = childClass;
        params.nX = UNITS_GetXPosition(missile); params.nY = UNITS_GetYPosition(missile);
        params.nTargetX = ring[i].stepX; params.nTargetY = ring[i].stepY;
        params.nVelocity = ring[i].velocity;
        params.nLoops = ring[i].loops;
        params.nSkill = MISSILE_GetSkill(missile);
        params.nSkillLevel = static_cast<int>(MISSILE_GetLevel(missile));
        auto* created = create_client_missile(&params, true);
        if (created) record_client_missile_event(9, created, UNIT_MISSILE, missile->dwUnitId);
    }
}
// Native 6FB55440 is applied to the first query result, not inside the query's
// callback: skipping it there would search a different occupant of the same cell.
static bool client_collision_target_dead(const D2UnitStrc* unit) {
    if (unit->dwFlags & UNITFLAG_ISDEAD) return true;
    if (unit->dwUnitType == UNIT_PLAYER)
        return unit->dwAnimMode == PLRMODE_DEATH || unit->dwAnimMode == PLRMODE_DEAD;
    if (unit->dwUnitType == UNIT_MONSTER)
        return unit->dwAnimMode == MONMODE_DEATH || unit->dwAnimMode == MONMODE_DEAD;
    return false;
}
static void advance_client_missiles() {
    // Native local missile table is visited by ascending GUID hash bucket. Each
    // callback saves its next link before running, so a new head in this bucket
    // waits; children in a later bucket can advance during this same pass.
    for (uint32_t bucket = 0; bucket < 128; ++bucket) {
      std::vector<D2UnitStrc*> members;
      for (auto it = g_clientMissiles.rbegin(); it != g_clientMissiles.rend(); ++it)
          if ((it->unit->dwUnitId & 127u) == bucket) members.push_back(it->unit);
      for (auto* unit : members) {
        const bool localOwner = g_selectedPlayer >= 0 && static_cast<size_t>(g_selectedPlayer) < g_players.size() &&
            MISSILE_CheckUnitIfOwner(unit, g_players[g_selectedPlayer].motionUnit);
        int passes = 1;
        for (int pass = 0; pass < passes; ++pass) {
        auto* path = unit->pDynamicPath;
        auto* txt = DATATBLS_GetMissilesTxtRecord(unit->dwClassId);
        // 6FB617C0's ordinary looping missile advances the simple unit clock,
        // moves the real dynamic path, then decrements its missile-frame lifetime.
        if (txt && MISSILE_GetRemainingFrames(unit) > txt->nInitSteps) unit->dwFlagEx &= ~0x40000u;
        if (txt && (txt->nLoopAnim || !UNITS_IsAtEndOfFrameCycle(unit))) UNITS_UpdateFrame(unit);
        if (path->dwVelocity) D2Common_10226(unit, 1024);
        MISSILE_SetCurrentFrame(unit, MISSILE_GetCurrentFrame(unit) - 1);
        bool expired = MISSILE_GetCurrentFrame(unit) < 1;
        bool collided = !expired && (path->nCollidedWithMask & 5u) != 0;
        uint32_t hitGuid = 0xffffffffu;
        if (!expired && !collided && txt && txt->nClientCol && path->nCollidedWithMask) {
            D2UnitStrc* owner = nullptr;
            for (auto& player : g_players) if (MISSILE_CheckUnitIfOwner(unit, player.motionUnit)) {
                owner = player.motionUnit; break;
            }
            RealmMissileCollision context{unit, owner};
            D2PathPointStrc* steps = nullptr;
            const int count = D2COMMON_10198_PathGetSaveStep(path, &steps);
            for (int step = 0; step < count && !collided; ++step) {
                auto* target = D2Common_10407(UNITS_GetRoom(unit), steps[step].X, steps[step].Y,
                    ordinary_projectile_target, &context, UNITS_GetUnitSizeX(unit));
                collided = target && !client_collision_target_dead(target);
                if (target && !collided) record_client_missile_event(8, unit, target->dwUnitType, target->dwUnitId);
            }
            if (collided) hitGuid = context.hitGuid;
            if (collided && hitGuid != 0xffffffffu) {
                // 0xB1420 with a unit: at its own target point (10227) there is no hit; otherwise a NextHit
                // missile holds the target for NextDelay frames, the hit plays, and a missile whose
                // CollideKill is 0 is not freed. 0xB17C0 returns after a unit hit either way.
                if (D2Common_10227(unit)) break;
                if (txt->nNextHit) {
                    g_clientNextHits.erase(std::remove_if(g_clientNextHits.begin(), g_clientNextHits.end(),
                        [](const ClientNextHit& n) { return n.until <= g_clientMissileTick; }), g_clientNextHits.end());
                    g_clientNextHits.push_back({UNIT_MONSTER, hitGuid, g_clientMissileTick + txt->nNextDelay});
                }
                if (!txt->nCollideKill) {
                    record_client_missile_event(3, unit, UNIT_MONSTER, hitGuid);
                    record_client_missile_impact_sound(3, unit, txt);
                    break;
                }
            }
        }
        if (expired || collided) {
            const uint32_t terminalEvent = expired ? 4 : (hitGuid != 0xffffffffu ? 3 : 2);
            record_client_missile_event(terminalEvent, unit,
                hitGuid != 0xffffffffu ? UNIT_MONSTER : 0xffffffffu, hitGuid);
            // Lifetime expiry runs the hit callback only with AlwaysExplode; Fire Ball has none (a retail recording expiry, no impact row).
            if (collided || (txt && txt->nAlwaysExplode)) record_client_missile_impact_sound(terminalEvent, unit, txt);
            if (txt && txt->wCltHitFunc == 1 && (collided || (txt && txt->nAlwaysExplode)) && g_selectedPlayer >= 0 &&
                static_cast<size_t>(g_selectedPlayer) < g_players.size()) {
                d2::realm::createFireballHitChildren(unit, g_players[g_selectedPlayer].motionUnit,
                    create_fireball_hit_child, nullptr);
                spawn_fireball_explosion(unit);
            }
            if (txt && (collided || txt->nAlwaysExplode) && (txt->wCltHitFunc == 2 || txt->wCltHitFunc == 3)) {
                if (static_cast<int16_t>(txt->wHitSound) >= 0)
                    g_clientMissileSoundEvents.push_back({1, g_clientMissileTick, unit->dwUnitId,
                        static_cast<uint32_t>(unit->dwClassId), terminalEvent, txt->wHitSound});
                if (txt->wCltHitFunc == 3) spawn_missile_hit_children(unit, txt);
                else spawn_missile_hit_ring(unit, txt);
            }
            const auto found = std::find_if(g_clientMissiles.begin(), g_clientMissiles.end(),
                [unit](const auto& missile) { return missile.unit == unit; });
            if (found != g_clientMissiles.end()) g_clientMissiles.erase(found);
            free_client_missile(unit);
            break;
        }
        // Native ordinary-realm prediction catches up a newly owned moving missile
        // once: the first completed do-pass has elapsed frame1, then clears the flag.
        if (pass == 0 && localOwner && MISSILE_GetRemainingFrames(unit) == 1 && path->dwVelocity)
            passes = 2;
        }
      }
    }
}

// The client's per-frame player update (1.13c D2Client+0x1C150) reads a per-mode table of
// (class, frame kind, end kind). These are the modes whose end kind is 2 or 3: get hit, both
// attacks, block, cast, throw, kick and the four skill modes. Each ends itself when its frame
// cycle ends; the client does not wait for the server. Sequence (18) has the same end kind; its
// frames advance through the sequence the used skill started.
static bool player_mode_ends_itself(uint32_t mode) {
    switch (mode) {
    case PLRMODE_GETHIT: case PLRMODE_ATTACK1: case PLRMODE_ATTACK2: case PLRMODE_BLOCK:
    case PLRMODE_CAST: case PLRMODE_THROW: case PLRMODE_KICK: case PLRMODE_SPECIAL1:
    case PLRMODE_SPECIAL2: case PLRMODE_SPECIAL3: case PLRMODE_SPECIAL4: case PLRMODE_SEQUENCE: return true;
    default: return false;
    }
}

// One frame of a self-ending player mode: at the action frame the used skill's "do" runs once
// (flag 0x40 marks it done), the frame advances without wrapping, and at the end of the cycle the
// used skill is dropped, the path returns to its previous type and the unit goes back to neutral.
// PARTIAL: the "do" step is the client missile of a plain projectile skill only; the other
// skills' client do-functions (the Skills row's cltdofunc table) are not run here.
static bool advance_client_player_action(D2UnitStrc* unit) {
    if (!unit || unit->dwUnitType != UNIT_PLAYER || !player_mode_ends_itself(unit->dwAnimMode)) return false;
    auto* skill = UNITS_GetUsedSkill(unit);
    const int doFn = skill && skill->pSkillsTxt ? static_cast<int>(skill->pSkillsTxt->wCltDoFunc) : 0;
    if (!(unit->dwFlags & 0x40u) && unit->nActionFrame >= 1 && unit->nActionFrame <= 3 &&
            d2client_realm_skill_do_ported(doFn) && !(skill->dwFlags & 1u)) {
        // 1.13c D2Client+0xA2C90: a row with a do function runs it; the function marks the do itself.
        d2client_realm_skill_do(doFn, unit, skill->pSkillsTxt->nSkillId, SKILLS_GetSkillLevel(unit, skill, TRUE),
            client_skill_do_host());
    } else if (!(unit->dwFlags & 0x40u) && unit->nActionFrame >= 1 && unit->nActionFrame <= 3) {
        if (skill && skill->pSkillsTxt && !(skill->dwFlags & 1u) &&
                (unit->dwAnimMode == PLRMODE_CAST ? is_client_launch_skill(skill->pSkillsTxt->nSkillId) :
                    (!is_natural_client_projectile(skill->pSkillsTxt->nSkillId) &&
                     is_client_launch_skill(skill->pSkillsTxt->nSkillId)))) {
            record_client_missile_event(7, unit);
            create_client_projectile(unit, skill);
        }
        unit->dwFlags |= 0x40u; // Marked even when nothing was emitted.
    }
    // A sequence is never restarted here: its count running out is its end.
    if (!unit->dwFrameCountPrecise && unit->dwAnimMode != PLRMODE_SEQUENCE) UNITS_SetAnimStartFrame(unit);
    // The end test comes before the advance (1.13c D2Client+0x1C150): the frame that reaches the end
    // of the cycle is shown for its tick, and the unit leaves the mode on the next one.
    if (!UNITS_IsAtEndOfFrameCycle(unit)) UNITS_StopSequence(unit);
    else {
        UNITS_SetUsedSkill(unit, nullptr);
        PATH_ResetToPreviousType(unit->pDynamicPath);
        unit->dwFlags = (unit->dwFlags & ~0x40u) | 2u;
        UNITS_ChangeAnimMode(unit, DUNGEON_IsRoomInTown(UNITS_GetRoom(unit)) ? PLRMODE_TOWNNEUTRAL : PLRMODE_NEUTRAL);
    }
    auto* path = unit->pDynamicPath;
    path->nDirection = d2_realm_client_motion_step_facing(path->nDirection, path->nNewDirection,
        path->nDiffDirection, path->dwFlags);
    return true;
}

// The mode change of an action (1.13c D2Client+0x2A6C0): the "do" mark is cleared and, when the
// mode really changes, the struck overlay is considered. The mode's own sound is the page's, keyed
// on the mode change it reads back.
static void client_set_mode(D2UnitStrc* unit, int mode) {
    unit->dwFlags &= ~0x40u;
    const uint32_t previous = unit->dwAnimMode;
    if (unit->dwUnitType == UNIT_MONSTER) d2_realm_client_unit_set_mode(unit, static_cast<uint32_t>(mode));
    else UNITS_ChangeAnimMode(unit, mode);
    if (previous != unit->dwAnimMode) emit_hit_overlay(unit);
}

// An action that could not start leaves the unit standing (1.13c D2Client+0x19710 for a player,
// the stop action for a monster). The dead stay as they are.
static void client_stop_unit(D2UnitStrc* unit) {
    unit->dwFlags &= ~0x40u;
    if (unit->dwUnitType == UNIT_PLAYER) {
        if (unit->dwAnimMode == PLRMODE_DEATH || unit->dwAnimMode == PLRMODE_DEAD) return;
        auto* room = UNITS_GetRoom(unit);
        client_set_mode(unit, room && DUNGEON_IsRoomInTown(room) ? PLRMODE_TOWNNEUTRAL : PLRMODE_NEUTRAL);
    } else if (unit->dwUnitType == UNIT_MONSTER) {
        if (unit->dwAnimMode != MONMODE_DEATH && unit->dwAnimMode != MONMODE_DEAD) client_set_mode(unit, MONMODE_NEUTRAL);
    }
}

// The hit class of a unit's own swing: the weapon in a player's hand (0 unarmed), a monster's
// own from its MonStats2 row.
static uint32_t client_weapon_hit_class(D2UnitStrc* unit) {
    if (unit->dwUnitType == UNIT_PLAYER) {
        auto* weapon = unit->pInventory ? D2Common_10434(unit, FALSE) : nullptr;
        return weapon ? ITEMS_GetHitClassFromItem(weapon) : 0;
    }
    if (unit->dwUnitType == UNIT_MONSTER) {
        auto* stats2 = UNITS_GetMonStats2TxtRecordFromMonsterId(unit->dwClassId);
        return stats2 ? stats2->nHitClass : 0;
    }
    return 0;
}

// The previous action is completed before a new one starts (1.13c D2Client+0x197D0): a used
// skill that has not done its "do" yet does it now, and the used skill is dropped.
// PARTIAL: the "do" is the client missile of a plain projectile skill only.
static void complete_previous_action(D2UnitStrc* unit) {
    auto* previous = UNITS_GetUsedSkill(unit);
    if (!previous) return;
    const int flags = SKILLS_GetFlags(previous);
    if (unit->dwUnitType == UNIT_PLAYER &&
            is_natural_client_projectile(SKILLS_GetSkillIdFromSkill(previous, __FILE__, __LINE__)) &&
            unit->dwAnimMode == PLRMODE_CAST && ((flags & 1) || !(unit->dwFlags & 0x40u))) {
        if (flags & 1) SKILLS_SetFlags(previous, flags | 2);
        create_client_projectile(unit, previous);
    }
    UNITS_SetUsedSkill(unit, nullptr);
}

// One skill start for every unit: the selected player's own click, another player's and a
// monster's from the server (1.13c D2Client+0xA22B0 at a spot, +0xA30C0 on a unit, both ending
// in +0xA1CF0). The caller has completed the previous action and reset the path.
struct ClientSkillStart {
    int id = 0;
    int level = 0;                 // the server's; unused for the selected player
    D2SkillStrc* skill = nullptr;  // the selected player's own skill of the hand
    D2UnitStrc* target = nullptr;
    int x = 0, y = 0;
    bool onUnit = false;
    bool local = false;
};
// 1 started, 0 not.
//  - aim: a spot becomes the path's target and the unit turns to it; a unit becomes the path's
//    target unit and is faced. A start on a unit that is not there stops the unit;
//  - a dead monster is a target only for a skill that takes corpses, when the skill searches
//    for its enemy by position;
//  - another unit's skill is assigned at the level the server sent and becomes its used skill;
//  - in town a skill without the InTown flag drops the used skill and stops the unit;
//  - the mode is the skill's own (a player's plain Attack takes Attack 1); a skill without a
//    mode leaves a player or monster in mode 1. A Sequence mode starts the skill's sequence;
//  - the start's sounds and overlay follow the Skills row (client_skill_fx.h).
// GUESS (level clamp): retail clamps another unit's level through +0x17B0, not read; the server's
// level is taken as sent. Settled by decompiling +0x17B0.
// GUESS (use state): the use-state test (mana, charges, quantity) is applied to the selected
// player only; this client does not hold other units' mana. Settled by a capture of +0xA1CF0 on
// a remote unit with zero mana.
// GUESS (repeat): the "already repeating" test that lets a Repeat skill keep its mode is taken as
// failing, so the mode is always set. Settled by decompiling D2Common ordinal 10494's use here.
// GUESS (second attack): retail's +0xA1260 picks Attack 2 for a player's plain Attack from the
// weapons in hand and a 50% roll on the unit's seed; every swing here is Attack 1. Settled by a
// function capture of +0xA1260 over one-handed, two-handed and dual-wield inventories.
// PARTIAL: of the 53 client start functions (the row's cltstfunc) entries 1, 2, 11, 12 and 39 run
// (d2client/realm_skill_start.h); the others are taken as succeeding, so a start that retail would
// refuse there still plays its sounds.
// NOT PORTED: the line-of-sight test of rows with a range class, the gate +0xA5B90 and the pre-aim call +0xA0660. The client
// missile (+0xA0F60) is created at the action frame for plain projectile skills only.
// Why the last local skill start was refused (0 = it started); numbered in source order.
static int g_lastSkillRefusal = 0;
static int refuse(int why) { g_lastSkillRefusal = why; return 0; }
static int g_lastRefusedUseState = 0;   // the use state (SKILLS_GetUseState) behind the last refusal 4
EMSCRIPTEN_KEEPALIVE int d2_realm_client_last_refusal_sound() {
    return d2::realm::refusalSoundEvent(g_lastSkillRefusal, g_lastRefusedUseState);
}

static int client_skill_start(D2UnitStrc* unit, const ClientSkillStart& s) {
    if (!unit || !unit->pDynamicPath) return refuse(1);
    if (s.onUnit && !s.target) {
        client_stop_unit(unit);
        return 0;
    }
    if (!d2::realm::applyCastTargetRelation(s.onUnit ? 0x4c : 0x4d, s.target, s.x, s.y,
            [unit](int tx, int ty) { D2COMMON_10170_PathSetTargetPos(unit->pDynamicPath, tx, ty); },
            [unit](D2UnitStrc* relation) { UNITS_SetTargetUnitForDynamicUnit(unit, relation); })) return refuse(6);
    if (s.x != UNITS_GetXPosition(unit) || s.y != UNITS_GetYPosition(unit)) UNITS_UpdateDirectionAndSpeed(unit, s.x, s.y);

    auto* txt = DATATBLS_GetSkillsTxtRecord(s.id);
    if (!txt) return 0;
    const uint32_t rowFlags = txt->dwFlags[0];
    if (s.target && s.target->dwUnitType == UNIT_MONSTER && s.target->dwAnimMode == MONMODE_DEAD &&
        (rowFlags & static_cast<uint32_t>(SKILLSFLAG_SEARCHENEMYXY)) &&
        !(rowFlags & static_cast<uint32_t>(SKILLSFLAG_TARGETCORPSE))) return 0;

    D2SkillStrc* skill = s.skill;
    if (!s.local) {
        if (!unit->pSkills) unit->pSkills = SKILLS_AllocSkillList(nullptr);
        if (!unit->pSkills) return 0;
        SKILLS_AssignSkill(unit, s.id, s.level, FALSE, __FILE__, __LINE__);
        skill = SKILLS_GetSkillById(unit, s.id, -1);
    }
    if (!skill || !skill->pSkillsTxt) return 0;
    if (s.local) {
        const int useState = SKILLS_GetUseState(unit, skill);
        if (useState != 0) { g_lastRefusedUseState = useState; return refuse(4); }
    }
    UNITS_SetUsedSkill(unit, skill);

    auto* room = UNITS_GetRoom(unit);
    if (!(rowFlags & static_cast<uint32_t>(SKILLSFLAG_INTOWN)) && room && DUNGEON_IsRoomInTown(room)) {
        UNITS_SetUsedSkill(unit, nullptr);
        client_stop_unit(unit);
        return refuse(5);
    }

    const bool isPlayer = unit->dwUnitType == UNIT_PLAYER;
    const int mode = isPlayer && s.id == 0 ? PLRMODE_ATTACK1 : SKILLS_GetSkillModeFromUnit(unit, skill);
    if (mode != 0 || isPlayer || unit->dwUnitType == UNIT_MONSTER) client_set_mode(unit, mode ? mode : 1);
    // The Skills row's client start function runs after the mode is set; a refusal drops the used skill and
    // puts the unit back (1.13c D2Client+0xA1CF0).
    // GUESS (mode after a refusal): the second mode call's argument is a register the decompile does not
    // show; neutral is what the end of an action sets. Settled by the disassembly at +0xA1CF0.
    if (!d2client_realm_skill_start(static_cast<int>(txt->wCltStFunc), unit, s.id,
            s.local ? SKILLS_GetSkillLevel(unit, skill, TRUE) : s.level)) {
        UNITS_SetUsedSkill(unit, nullptr);
        client_stop_unit(unit);
        return refuse(7);
    }

    d2::realm::SkillFxRow row;
    row.stSoundDelay = (rowFlags & static_cast<uint32_t>(SKILLSFLAG_STSOUNDDELAY)) != 0;
    row.weaponSound = (rowFlags & static_cast<uint32_t>(SKILLSFLAG_WEAPONSND)) != 0;
    row.stSound = static_cast<int16_t>(txt->wStSound);
    row.stSoundClass = static_cast<int16_t>(txt->nStSoundClass);
    row.castOverlay = static_cast<int16_t>(txt->wCastOverlay);
    row.itemCastSound = static_cast<int16_t>(txt->wItemCastSound);
    row.itemCastOverlay = static_cast<int16_t>(txt->wItemCastOverlay);
    row.charClass = static_cast<int8_t>(txt->nCharClass);
    d2::realm::SkillFxUnit actor;
    actor.isPlayer = isPlayer;
    actor.classId = unit->dwClassId;
    actor.hitClass = client_weapon_hit_class(unit);
    actor.animSpeed = static_cast<uint32_t>(static_cast<int32_t>(unit->wAnimSpeed));
    const auto plan = d2::realm::skillStartFx(row, actor, skill->nOwnerGUID != -1,
        sgptDataTables->nOverlayTxtRecordCount);
    for (int i = 0; i < plan.sounds; ++i)
        record_unit_fx(static_cast<uint32_t>(plan.sound[i].kind), unit, static_cast<uint32_t>(plan.sound[i].row),
            static_cast<uint32_t>(plan.sound[i].delay), static_cast<uint32_t>(s.id));
    if (plan.overlay > 0 && !unit_carries_overlay(unit, static_cast<uint32_t>(plan.overlay)))
        start_unit_overlay(unit, plan.overlay, static_cast<uint32_t>(s.id));
    g_lastSkillRefusal = 0;
    return 1;
}

static D2UnitStrc* selected_player_unit() {
    if (g_selectedPlayer < 0 || size_t(g_selectedPlayer) >= g_players.size()) return nullptr;
    auto* unit = g_players[g_selectedPlayer].motionUnit;
    return unit && unit->pDynamicPath && UNITS_GetRoom(unit) ? unit : nullptr;
}

// The selected player starts a skill itself before the packet is sent; the packet goes out only
// when this start succeeded (1.13c D2Client+0x2A7C0: gate, local action, then send). Any skill of
// either hand takes this path: at a spot (client action 0x15) or on a unit (0x16). 1 started, 0 refused.
//  - gate (+0x29F30): a used skill whose row lacks Interrupt refuses a new action unless the
//    unit stands in a neutral mode;
//  - the previous action is completed first, then the start is the one every unit takes
//    (client_skill_start).
// GUESS (order of the use-state test): it is made here before the unit is aimed, so a skill the
// player cannot pay for leaves him as he is; retail makes it after the aim. Settled by a capture
// of +0x19830 with action 0x15 and zero mana.
static int local_skill_start(D2UnitStrc* unit, D2SkillStrc* skill, D2UnitStrc* target, int x, int y) {
    if (!unit || !skill || !skill->pSkillsTxt) return refuse(1);
    if (unit->dwAnimMode == PLRMODE_DEATH || unit->dwAnimMode == PLRMODE_DEAD) return refuse(2);
    const bool neutral = unit->dwAnimMode == PLRMODE_NEUTRAL || unit->dwAnimMode == PLRMODE_TOWNNEUTRAL;
    auto* previous = UNITS_GetUsedSkill(unit);
    if (previous && previous->pSkillsTxt && !neutral &&
        !(previous->pSkillsTxt->dwFlags[0] & static_cast<uint32_t>(SKILLSFLAG_INTERRUPT))) return refuse(3);
    complete_previous_action(unit);
    if (SKILLS_GetUseState(unit, skill) != 0) return refuse(4);
    PATH_ResetToPreviousType(unit->pDynamicPath);
    ClientSkillStart start;
    start.id = SKILLS_GetSkillIdFromSkill(skill, __FILE__, __LINE__);
    start.skill = skill;
    start.target = target;
    start.x = x;
    start.y = y;
    start.onUnit = target != nullptr;
    start.local = true;
    return client_skill_start(unit, start);
}

static bool local_action_gate_fwd(D2UnitStrc* unit);
static void local_queue_clear_fwd();
static D2SkillStrc* hand_skill(D2UnitStrc* unit, int hand) {
    return d2_realm_client_hand_skill(unit, hand, PlayerSkillOps{});
}

// The left (hand 0) or right (hand 1) skill at a spot. 1 started, 0 refused.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_local_skill_at(int hand, int x, int y) {
    auto* unit = selected_player_unit();
    if (!unit || x < 0 || y < 0 || x > 65535 || y > 65535) return 0;
    if (!local_action_gate_fwd(unit)) return 0;
    const int started = local_skill_start(unit, hand_skill(unit, hand), nullptr, x, y);
    local_queue_clear_fwd();   // +0x2A7C0 drops the waiting action once past the gate
    return started;
}

// The left or right skill on a monster or player. 1 started, 0 refused (also a dying or dead
// target, which the click treats as ground).
EMSCRIPTEN_KEEPALIVE int d2_realm_client_local_skill_on_unit(int hand, int type, uint32_t guid) {
    auto* unit = selected_player_unit();
    if (!unit || (type != UNIT_MONSTER && type != UNIT_PLAYER)) return refuse(8);
    auto* target = find_realm_client_unit(static_cast<uint8_t>(type), guid);
    if (!target || target == unit || !target->pDynamicPath || !UNITS_GetRoom(target)) return refuse(9);
    if (type == UNIT_MONSTER && (target->dwAnimMode == MONMODE_DEATH || target->dwAnimMode == MONMODE_DEAD)) return refuse(10);
    if (!local_action_gate_fwd(unit)) return refuse(3);
    const int started = local_skill_start(unit, hand_skill(unit, hand), target,
        UNITS_GetXPosition(target), UNITS_GetYPosition(target));
    local_queue_clear_fwd();
    return started;
}

// What a click with a hand's skill does (1.13c D2Client+0x1B7F0 and, for a monster, +0x1B700).
// The skill's range class comes from its Skills row through the engine (none, hand to hand,
// ranged, either by the weapon in hand, or 4).
//  - on a unit: 0 = walk to it first, when the class is hand to hand or 4 and the unit is out of
//    melee range (the engine's size-aware test with the "target is moving" bonus); the skill is
//    then used on arrival. 1 = use the skill on the unit now.
//  - on the ground (type < 0): 2 = the right button with a hand to hand skill walks there
//    instead; 1 = use the skill at the spot.
// -1 when there is no local player, skill or target unit.
// NOT PORTED: a Skills row whose word at +0x2E is 0x13 takes its own branch (+0x1B1D0) when out
// of range; here it follows the rule above.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_skill_plan(int hand, int type, uint32_t guid) {
    auto* unit = selected_player_unit();
    auto* skill = unit ? hand_skill(unit, hand) : nullptr;
    if (!skill) return -1;
    const int kind = D2Common_11016(unit, skill);
    if (type < 0) return hand && kind == 1 ? 2 : 1;
    if (type != UNIT_MONSTER && type != UNIT_PLAYER) return -1;
    auto* target = find_realm_client_unit(static_cast<uint8_t>(type), guid);
    if (!target || !target->pDynamicPath || !UNITS_GetRoom(target)) return -1;
    if (kind != 1 && kind != 4) return 1;
    return UNITS_IsInMeleeRange(unit, target, UNITS_IsInMovingMode(target)) ? 1 : 0;
}

// The right skill on a monster, kept for callers of the older entry.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_player_predict_right_cast_on_monster(uint32_t guid) {
    return d2_realm_client_local_skill_on_unit(1, UNIT_MONSTER, guid);
}

// ---- The selected player's own actions on a unit (1.13c D2Client+0x2AA40, +0x2B090, +0x2A7C0) ----
// A click on a unit that is out of reach walks or runs to it and keeps the action waiting; every
// game frame the waiting action is re-tested and either the walk is planned again or the action
// is started. Retail keeps the four words in the player's data (+0x150 fire, +0x154 kind, the
// client packet's opcode, +0x158 target type, +0x15C target id).
struct LocalQueuedAction { uint32_t fire = 0, kind = 0, type = 0, guid = 0; };
static LocalQueuedAction g_localQueued;
// The packets the engine's own steps send (opcode, target type, target id), for the page to put on
// the wire. Retail's sender (+0x143E0) drops a packet equal to the last one sent within 200 ms
// (50 ms for the skill opcodes 0x05..0x11).
// GUESS (clock): the window is counted in game frames of 40 ms, not on the wall clock retail reads.
struct LocalSend { uint32_t op, type, guid; };
static std::vector<LocalSend> g_localSends;
static LocalSend g_lastLocalSend{ 0xffffffffu, 0, 0 };
static uint32_t g_lastLocalSendTick = 0;
static uint32_t g_localSendView[3 * 16];

static void local_send(uint32_t op, uint32_t type, uint32_t guid) {
    const bool skill = op >= 0x05 && op <= 0x11 && op != 0x0b;
    const uint32_t window = skill ? 2u : 5u;
    if (g_lastLocalSend.op == op && g_lastLocalSend.type == type && g_lastLocalSend.guid == guid &&
            g_clientMissileTick - g_lastLocalSendTick < window) return;
    g_lastLocalSend = { op, type, guid };
    g_lastLocalSendTick = g_clientMissileTick;
    if (g_localSends.size() < 16) g_localSends.push_back(g_lastLocalSend);
}

// Fills (opcode, type, id) triples and empties the list; the count is returned.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_local_sends(uint32_t** out) {
    int n = 0;
    for (const auto& send : g_localSends) {
        g_localSendView[n * 3] = send.op; g_localSendView[n * 3 + 1] = send.type; g_localSendView[n * 3 + 2] = send.guid;
        ++n;
    }
    g_localSends.clear();
    if (out) *out = g_localSendView;
    return n;
}
EMSCRIPTEN_KEEPALIVE const uint32_t* d2_realm_client_local_send_view() { return g_localSendView; }
// The waiting action's kind (0 none).
EMSCRIPTEN_KEEPALIVE uint32_t d2_realm_client_local_queued_kind() { return g_localQueued.kind; }

static void local_queue_clear() { g_localQueued = LocalQueuedAction{}; }
namespace { void g_localQueuedReset() { local_queue_clear(); g_localSends.clear(); g_lastLocalSend = { 0xffffffffu, 0, 0 }; } }

// The gate of every own action (+0x29F30): a used skill whose row lacks Interrupt refuses a new
// action unless the unit stands in a neutral mode.
static bool local_action_gate(D2UnitStrc* unit) {
    // A hit recovery runs its whole cycle: the server refuses any action sent during it, so the
    // client must not predict one (it would leave the mode on the first click and cut the cycle).
    if (unit->dwAnimMode == PLRMODE_GETHIT) return false;
    auto* used = UNITS_GetUsedSkill(unit);
    if (!used || !used->pSkillsTxt) return true;
    if (used->pSkillsTxt->dwFlags[0] & static_cast<uint32_t>(SKILLSFLAG_INTERRUPT)) return true;
    return unit->dwAnimMode == PLRMODE_NEUTRAL || unit->dwAnimMode == PLRMODE_TOWNNEUTRAL;
}

static bool local_action_gate_fwd(D2UnitStrc* unit) { return local_action_gate(unit); }
static void local_queue_clear_fwd() { local_queue_clear(); }

// Client actions 0 (walk to a unit) and 0x18 (run to a unit) of +0x19830: the previous action is
// completed, the path returns to its previous type, the unit becomes the path's target (+0x2A070)
// and the route is planned. A route takes the walk (town walk) or run mode; no unit or no route
// leaves the player standing (+0x19710). 1 started, 0 not.
static int local_move_to_unit_action(D2UnitStrc* unit, D2UnitStrc* target, bool run) {
    D2ActiveRoomStrc* room = UNITS_GetRoom(unit);
    if (!room) return 0;
    complete_previous_action(unit);
    PATH_ResetToPreviousType(unit->pDynamicPath);
    const bool inTown = DUNGEON_IsRoomInTown(room) != FALSE;
    PATH_SetTargetUnit(unit->pDynamicPath, target);
    if (!target || D2Common_10142(unit->pDynamicPath, unit, FALSE) <= 0) {
        client_stop_unit(unit);
        return 0;
    }
    client_set_mode(unit, run ? PLRMODE_RUN : (inTown ? PLRMODE_TOWNWALK : PLRMODE_WALK));
    return 1;
}

// "Walk to the unit first" (+0x2AA40): past the gate the walk or run to the unit starts and its
// packet (0x02 walk, 0x04 run) goes out when it did; the action then waits, whether or not the
// gate let the walk start.
static int local_walk_first(D2UnitStrc* unit, uint32_t kind, uint32_t type, uint32_t guid, bool run) {
    int started = 0;
    if (local_action_gate(unit)) {
        started = local_move_to_unit_action(unit, find_realm_client_unit(static_cast<uint8_t>(type), guid), run);
        if (started) local_send(run ? 0x04 : 0x02, type, guid);
    }
    g_localQueued = { 1, kind, type, guid };
    return started;
}

// Whether the player must walk before it can use the unit (+0x29E80): an object is in reach when
// its footprint reach holds and nothing collides between the two; any other unit by the melee test
// with the "target is moving" bonus.
// NOT PORTED: a hand skill whose Skills row word at +0x2E is 0x13 takes its own test (+0x18880).
static bool local_must_walk(D2UnitStrc* unit, D2UnitStrc* target) {
    if (target->dwUnitType == UNIT_OBJECT)
        return !(UNITS_IsObjectInInteractRange(unit, target) &&
            !UNITS_TestCollisionBetweenInteractingUnits(unit, target, 0x804));
    return !UNITS_IsInMeleeRange(unit, target, UNITS_IsInMovingMode(target));
}

// The hand of a skill opcode: 0x05..0x0B left, 0x0C..0x12 right, -1 otherwise.
static int local_kind_hand(uint32_t kind) {
    return kind >= 0x05 && kind <= 0x0b ? 0 : kind >= 0x0c && kind <= 0x12 ? 1 : -1;
}

// Client action 2 of +0x19830, the use of a unit (+0x2AC10): the previous action is completed and
// the path returns to its previous type; a player target is faced; an object drops the path's
// target unit and, when it can be attacked, is faced and struck with the Kick skill. 1 when the
// unit is there.
// GUESS (kick): retail names the skill through D2Common ordinal 10630, not read; the recordings
// show the kick mode on such an object. Settled by decompiling that ordinal.
// NOT PORTED: the two object classes 0x194 and 0x178 with their held-item tests, and the 200 ms
// repeat guard of a monster target.
static int local_interact_action(D2UnitStrc* unit, uint32_t type, uint32_t guid) {
    complete_previous_action(unit);
    PATH_ResetToPreviousType(unit->pDynamicPath);
    D2UnitStrc* target = find_realm_client_unit(static_cast<uint8_t>(type), guid);
    if (!target) return type == UNIT_ITEM || type == UNIT_TILE ? 1 : 0;
    if (type == UNIT_PLAYER) {
        UNITS_UpdateDirectionAndSpeed(unit, UNITS_GetXPosition(target), UNITS_GetYPosition(target));
    } else if (type == UNIT_OBJECT) {
        PATH_SetTargetUnit(unit->pDynamicPath, nullptr);
        if (target->dwFlags & UNITFLAG_CANBEATTACKED) {
            UNITS_UpdateDirectionAndSpeed(unit, UNITS_GetXPosition(target), UNITS_GetYPosition(target));
            if (auto* kick = SKILLS_GetSkillById(unit, SKILL_KICK, -1)) {
                ClientSkillStart start;
                start.id = SKILL_KICK;
                start.skill = kick;
                start.target = target;
                start.x = UNITS_GetXPosition(target);
                start.y = UNITS_GetYPosition(target);
                start.onUnit = true;
                start.local = true;
                client_skill_start(unit, start);
            }
        }
    }
    return 1;
}

// The start of an own action and its packet (+0x2A7C0): past the gate the local action runs, and
// only an action that started is sent. The waiting action is dropped either way.
static int local_fire(D2UnitStrc* unit, uint32_t kind, uint32_t type, uint32_t guid) {
    if (!local_action_gate(unit)) return 0;
    int started = 0;
    const int hand = local_kind_hand(kind);
    if (kind == 0x02 || kind == 0x04) {
        started = local_move_to_unit_action(unit, find_realm_client_unit(static_cast<uint8_t>(type), guid), kind == 0x04);
    } else if (kind == 0x13) {
        started = local_interact_action(unit, type, guid);
    } else if (hand >= 0) {
        D2UnitStrc* target = find_realm_client_unit(static_cast<uint8_t>(type), guid);
        if (target && target != unit && target->pDynamicPath && UNITS_GetRoom(target))
            started = local_skill_start(unit, hand_skill(unit, hand), target,
                UNITS_GetXPosition(target), UNITS_GetYPosition(target));
        else client_stop_unit(unit);
    }
    if (started) local_send(kind, type, guid);
    local_queue_clear();
    return started;
}

// Once per game frame, before the units move (+0x2B090, called from the game loop +0x44B50): a
// waiting action whose fire word is set is taken off the queue and its target looked up. A target
// that is gone, or that cannot be used, ends it. Out of reach the walk starts again and the action
// waits again (+0x2AA40); in reach the action starts (+0x2A7C0).
//  - an object must be targetable;
//  - a dying or dead monster is a target only for a hand skill that takes corpses.
//  - a player target goes through the arrival gate +0x2AFB0 (local_player_arrival below).
// GUESS (targetable): the targetable flag of a live monster is not kept by this client and is
// taken as set. Settled by the unit flags of the assign packets.
static void local_player_arrival(D2UnitStrc* unit, D2UnitStrc* target, uint32_t kind, uint32_t guid, bool run);
static void local_queue_step(D2UnitStrc* unit, bool run) {
    if (!g_localQueued.fire) return;
    const LocalQueuedAction q = g_localQueued;
    local_queue_clear();
    if (!q.kind) return;
    if (unit->dwAnimMode == PLRMODE_DEATH || unit->dwAnimMode == PLRMODE_DEAD) return;
    D2UnitStrc* target = find_realm_client_unit(static_cast<uint8_t>(q.type), q.guid);
    if (!target || !UNITS_GetRoom(target)) return;
    if (q.type == UNIT_PLAYER) { local_player_arrival(unit, target, q.kind, q.guid, run); return; }
    if (q.type == UNIT_OBJECT) {
        if (!(target->dwFlags & UNITFLAG_TARGETABLE)) return;
    } else if (q.type == UNIT_MONSTER && (target->dwAnimMode == MONMODE_DEATH || target->dwAnimMode == MONMODE_DEAD)) {
        const int hand = local_kind_hand(q.kind);
        auto* skill = hand >= 0 ? hand_skill(unit, hand) : nullptr;
        if (!skill || !skill->pSkillsTxt ||
            !(skill->pSkillsTxt->dwFlags[0] & static_cast<uint32_t>(SKILLSFLAG_TARGETCORPSE))) return;
    }
    if (local_must_walk(unit, target)) {
        local_walk_first(unit, q.kind, q.type, q.guid, run);
        return;
    }
    local_fire(unit, q.kind, q.type, q.guid);
}

// The run toggle the waiting action re-walks with (retail reads its two run words each time).
static bool g_localRun = false;

// The local player's ground click starts its own route at once. The game server never echoes the
// owner's walk/run (0x0F/0x10 go only to the other clients; the owner is corrected by 0x15), so a
// client that waits for the echo never moves. Same target/route branch as a remote 0x0F:
// action 0x01 walk, 0x17 run. Returns 1 when a route started, 0 when refused or no local unit.
// As +0x2A7C0 has it: the gate comes first, a ground click is ignored while the use of a unit
// (kind 0x13) waits, and any other waiting action is dropped.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_local_move(int x, int y, int run) {
    if (g_selectedPlayer < 0 || size_t(g_selectedPlayer) >= g_players.size()) return 0;
    auto* unit = g_players[g_selectedPlayer].motionUnit;
    if (!unit || !unit->pDynamicPath || !UNITS_GetRoom(unit)) return 0;
    if (unit->dwAnimMode == PLRMODE_DEATH || unit->dwAnimMode == PLRMODE_DEAD) return 0;
    g_localRun = run != 0;
    if (!local_action_gate(unit)) return 0;
    if (g_localQueued.kind == 0x13) return 0;
    complete_previous_action(unit);
    PATH_ResetToPreviousType(unit->pDynamicPath);
    const int started = d2_realm_client_motion_apply_player_move(unit, run ? 0x17 : 0x01, 0,
        static_cast<uint16_t>(x), static_cast<uint16_t>(y)) > 0 ? 1 : 0;
    local_queue_clear();
    return started;
}

// A click on a unit that is out of reach: walk or run to it and keep the action `kind` (the
// opcode of the packet that will use it: 0x13 use, 0x06 left skill, 0x0D right skill, ...)
// waiting. The engine re-plans the walk and starts the action itself on later frames; what it
// sends is read with d2_realm_client_local_sends. 1 the walk started, 0 it did not (the action
// waits all the same), -1 when the unit is not one the client keeps a CUnit for (nothing waits).
EMSCRIPTEN_KEEPALIVE int d2_realm_client_local_act_on_unit(int kind, int type, uint32_t guid, int run) {
    auto* unit = selected_player_unit();
    if (!unit || kind <= 0 || kind > 0x13) return -1;
    D2UnitStrc* target = find_realm_client_unit(static_cast<uint8_t>(type), guid);
    if (!target || !UNITS_GetRoom(target)) return -1;
    if (unit->dwAnimMode == PLRMODE_DEATH || unit->dwAnimMode == PLRMODE_DEAD) return 0;
    g_localRun = run != 0;
    return local_walk_first(unit, static_cast<uint32_t>(kind), static_cast<uint32_t>(type), guid, run != 0);
}

// The plain walk or run to a unit with nothing waiting (kinds 2 and 4 of +0x2A7C0), kept for
// callers of the older entry. 1 started, 0 refused, -1 no client unit.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_local_move_to_unit(int type, uint32_t guid, int run) {
    auto* unit = selected_player_unit();
    if (!unit) return -1;
    D2UnitStrc* target = find_realm_client_unit(static_cast<uint8_t>(type), guid);
    if (!target || !UNITS_GetRoom(target)) return -1;
    if (unit->dwAnimMode == PLRMODE_DEATH || unit->dwAnimMode == PLRMODE_DEAD) return 0;
    g_localRun = run != 0;
    if (!local_action_gate(unit)) return 0;
    const int started = local_move_to_unit_action(unit, target, run != 0);
    local_queue_clear();
    return started;
}

// The use of a unit in reach, or the start of a hand's skill on it, by the packet's opcode
// (+0x2A7C0). 1 started (the packet is in the send list), 0 refused.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_local_fire(int kind, int type, uint32_t guid) {
    auto* unit = selected_player_unit();
    if (!unit || kind <= 0 || kind > 0x13) return 0;
    if (unit->dwAnimMode == PLRMODE_DEATH || unit->dwAnimMode == PLRMODE_DEAD) return 0;
    return local_fire(unit, static_cast<uint32_t>(kind), static_cast<uint32_t>(type), guid);
}

// Whether the selected player can use a unit now. Retail's "walk first" test (1.13c
// D2Client+0x29E80) for a monster or player is the engine's melee-range test with the "target is
// moving" bonus: in range when the distance is under 1, or within the player's melee range plus
// one plus the bonus with nothing blocking the way; a tentacle monster also passes at plus 8.
// Objects: retail's test has no melee rule for an object. It is usable now only when the footprint
// reach (D2Common #10594) is nonzero and the collision test between the two (D2Common #10324, mask
// 0x804) is zero; the object table's operate-range column is not read.
// Items: a ground item the client holds takes the melee rule above (retail's 0x29E80 end branch);
// one it holds no unit for, and tiles, stay -1 and the page falls back to its tile distance.
// Settled by a function capture of +0x29E80 over item and tile targets with the melee inputs pinned.
// -1 for a unit the client keeps no CUnit for (ground items, an object outside the active rooms) or no local player.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_can_interact(int type, uint32_t guid) {
    if (g_selectedPlayer < 0 || size_t(g_selectedPlayer) >= g_players.size()) return -1;
    auto* unit = g_players[g_selectedPlayer].motionUnit;
    if (!unit || !unit->pDynamicPath) return -1;
    if (type == UNIT_OBJECT) {
        D2UnitStrc* object = find_object_unit(guid);
        if (!object || !UNITS_GetRoom(object) || !UNITS_GetRoom(unit)) return -1;
        return UNITS_IsObjectInInteractRange(unit, object) &&
            !UNITS_TestCollisionBetweenInteractingUnits(unit, object, 0x804) ? 1 : 0;
    }
    if (type != UNIT_MONSTER && type != UNIT_PLAYER && type != UNIT_ITEM) return -1;
    auto* target = find_realm_client_unit(static_cast<uint8_t>(type), guid);
    if (!target || !target->pDynamicPath) return -1;
    return UNITS_IsInMeleeRange(unit, target, UNITS_IsInMovingMode(target)) ? 1 : 0;
}

// 1 once the selected player has ended the action it was walking or casting (back in a
// neutral mode, or dead); 0 while that action still runs; -1 with no local unit.
// GUESS (pending-action words): retail keeps the pending action in two D2Client words. 0x11C2F4 is the
// pending target record: 0x51860 sets it to the clicked unit (or 0 to cancel) and records its type at
// 0x11964C and its guid at 0x119638; 0x51AE0 and 0x51D10 clear it when the path resolves. 0x11C2F8 is a
// busy latch: the refusal paths 0x2AE30, 0x2AEA0 and 0x2AFB0 set it (and clear 0x11C2F4), and 0x51860,
// 0x51E80, 0x51AE0, 0x51D10 do nothing while it is set. Here "done" is the anim mode, which is not that
// state machine; the realm does not read or write either word.
// capture: d2client/51ae0 <a walk to a monster, a stop, and a cast; 0x11C2F4 and 0x11C2F8 and the two
// pending-record words recorded per frame>.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_local_action_done() {
    if (g_selectedPlayer < 0 || size_t(g_selectedPlayer) >= g_players.size()) return -1;
    auto* unit = g_players[g_selectedPlayer].motionUnit;
    if (!unit) return -1;
    switch (unit->dwAnimMode) {
    case PLRMODE_NEUTRAL: case PLRMODE_TOWNNEUTRAL: case PLRMODE_DEATH: case PLRMODE_DEAD: return 1;
    default: return 0;
    }
}

// The server starts a skill on a unit: 0x4C on a target unit, 0x4D at a spot (1.13c
// D2Client+0xAF1D0 and +0xAF160, client actions 0x16 and 0x15). A player and a monster both take
// the start the selected player takes; the selected player has already started his own.
// A dead caster is left alone (a safety of this port, not read from retail).
// NOT PORTED: a monster whose skill mode is Sequence runs +0x4D5B0 first, not read.
static int apply_skill_start_packet(const uint8_t* packet, int length) {
    d2::realm::SkillStartRequest request;
    if (!d2::realm::decodeSkillStart(packet, length, &request)) return -1;
    ClientSkillStart start;
    start.id = static_cast<int>(request.skill & 0xffffu);
    start.level = request.level;
    start.onUnit = request.onUnit;
    start.x = request.x;
    start.y = request.y;
    if (request.onUnit) {
        auto* target = find_realm_client_unit(request.targetType, request.targetGuid);
        if (target && target->pDynamicPath && UNITS_GetRoom(target)) {
            start.target = target;
            start.x = UNITS_GetXPosition(target);
            start.y = UNITS_GetYPosition(target);
        }
    }
    if (request.unitType == UNIT_PLAYER) {
        const auto it = std::find_if(g_players.begin(), g_players.end(), [&request](const auto& player) {
            return player.view.guid == request.guid;
        });
        if (it == g_players.end() || static_cast<int>(it - g_players.begin()) == g_selectedPlayer) return 0;
        D2UnitStrc* unit = it->motionUnit;
        if (!unit || !unit->pDynamicPath || !UNITS_GetRoom(unit)) return 0;
        if (unit->dwAnimMode == PLRMODE_DEATH || unit->dwAnimMode == PLRMODE_DEAD) return 0;
        if (start.target == unit) start.target = nullptr;
        complete_previous_action(unit);
        PATH_ResetToPreviousType(unit->pDynamicPath);
        if (client_skill_start(unit, start)) return 1;
        client_stop_unit(unit);
        return 0;
    }
    if (request.unitType == UNIT_MONSTER) {
        const auto it = std::find_if(g_monsters.begin(), g_monsters.end(), [&request](const auto& monster) {
            return monster.view.guid == request.guid;
        });
        if (it == g_monsters.end()) return 0;
        D2UnitStrc* unit = it->motionUnit;
        if (!unit || !unit->pDynamicPath || !UNITS_GetRoom(unit)) return 0;
        if (unit->dwAnimMode == MONMODE_DEATH || unit->dwAnimMode == MONMODE_DEAD) return 0;
        if (start.target == unit) start.target = nullptr;
        PATH_ResetToPreviousType(unit->pDynamicPath);
        const int started = client_skill_start(unit, start);
        refresh_monster_motion_view(*it);
        return started;
    }
    return 0;
}

// Permanent native gate readback for selected cast failures; no state mutation.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_player_right_cast_state(int field) {
    if (g_selectedPlayer < 0 || size_t(g_selectedPlayer) >= g_players.size()) return -1;
    auto* unit = g_players[g_selectedPlayer].motionUnit;
    if (field == 13) return g_lastSkillRefusal;
    if (field == 17) return unit ? (unit->pSkills != nullptr) | ((unit->pDynamicPath != nullptr) << 1) | ((unit && unit->pDynamicPath && UNITS_GetRoom(unit)) << 2) : -1;
    if (field == 18) return static_cast<int>(g_players.size());
    if (!unit) return -1;
    auto* skill = unit->pSkills ? UNITS_GetRightSkill(unit) : nullptr;
    switch (field) {
    case 0: return skill ? SKILLS_GetUseState(unit, skill) : -1;
    case 1: return skill ? SKILLS_GetSkillLevel(unit, skill, TRUE) : -1;
    case 2: return skill ? SKILLS_GetSkillIdFromSkill(skill, __FILE__, __LINE__) : -1;
    case 3: return skill ? skill->nOwnerGUID : -1;
    case 4: return UNITS_GetRoom(unit) ? DUNGEON_IsRoomInTown(UNITS_GetRoom(unit)) : -1;
    case 5: return skill ? SKILLS_GetFlags(skill) : -1;
    case 6: return unit->dwAnimMode;
    case 7: return STATLIST_UnitGetStatValue(unit, STAT_MANA, 0);
    case 8: return STATLIST_UnitGetStatValue(unit, STAT_LEVEL, 0);
    case 9: return unit->pInventory != nullptr;
    case 10: return unit->pInventory ? unit->pInventory->dwSignature : 0;
    case 11: return unit->pInventory ? unit->pInventory->dwOwnerGuid : -1;
    case 12: return unit->pInventory && unit->pInventory->pOwner == unit;
    case 14: return unit->pInventory && INVENTORY_GetItemFromBodyLoc(unit->pInventory, 4) != nullptr;
    case 15: return unit->pInventory && INVENTORY_GetItemFromBodyLoc(unit->pInventory, 5) != nullptr;
    case 16: return d2client_realm_a0a30(unit);
    default: return -1;
    }
}

// The right skill at a spot, kept for callers of the older entry.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_player_predict_right_cast(int x, int y) {
    return d2_realm_client_local_skill_at(1, x, y);
}

struct SkillUpdateOps {
    D2UnitStrc* resolveUnit(uint8_t type, uint32_t guid) const {
        if (type == UNIT_MONSTER) {
            for (auto& monster : g_monsters)
                if (monster.view.guid == guid) return create_monster_motion(monster) ? monster.motionUnit : nullptr;
            return nullptr;
        }
        if (type == UNIT_PLAYER) {
            for (auto& player : g_players) if (player.view.guid == guid) return player.motionUnit;
            return nullptr;
        }
        return find_realm_client_unit(type, guid);
    }
    bool knownSkill(int id) const { return DATATBLS_GetSkillsTxtRecord(id) != nullptr; }
    void assign(D2UnitStrc* unit, int id, uint8_t level, uint8_t flags) const {
        SKILLS_AssignSkill(unit, id, level, flags, __FILE__, __LINE__);
    }
    bool setQuantity(D2UnitStrc* unit, int id, uint8_t quantity) const {
        auto* skill = SKILLS_GetSkillById(unit, id, -1);
        if (!skill) return false;
        SKILLS_SetQuantity(skill, quantity);
        return true;
    }
};

static int apply_player_skill_update(const uint8_t* packet, int length) {
    SkillUpdateOps ops;
    return d2::realm::applySkillUpdatePacket(packet, length, ops);
}

EMSCRIPTEN_KEEPALIVE int d2_realm_client_player_skill_value(uint32_t guid, int id, int quantity) {
    for (auto& player : g_players) if (player.view.guid == guid && player.motionUnit) {
        auto* skill = SKILLS_GetSkillById(player.motionUnit, id, -1);
        if (skill) return quantity ? SKILLS_GetQuantity(skill) : SKILLS_GetSkillLevel(player.motionUnit, skill, FALSE);
    }
    return -1;
}

// 1.13c D2Client+0xAEDF0 (0x19..0x1F) and +0xAED80 (0x20). 0x19 adds the byte to gold (stat 14); 0x1A and
// 0x1B add a byte or word to experience (stat 13); 0x1C sets experience to the dword; 0x1D..0x1F set the
// stat named by byte 1 to a byte, word or dword at byte 2. All act on the current player. 0x20 sets a
// stat on the player unit whose GUID it names. Values are in the stat's wire units.
// Retail effect of a missing hero: the stat packet logs a fixed message and exits the client, so nothing is
// written and no stat-change event runs. Here a missing selected record is a negative return, which rejects the
// stream and clears the realm state (the nearest equivalent). Open difference: retail keys this on the hero
// unit being null; a selected record with a null motion unit is the same condition (its unit is made when the
// player is assigned) and is rejected the same way (-1). The 0x20 form on a GUID without a record is a no-op.
// Differs from retail: the stat-change event that follows each 0x19..0x1F set (stat id and new value) is not run
// here. Its effects are client-side only and never write the stat list: stats 0 and 2 re-check the equipped
// items; stat 12 (level) re-checks the owner's equipped items and, for the hero, its inventory and equipped items
// again; hit points at a nonzero value on a dead unit run two dead-unit callees. The browser runs them through
// d2_client_stat_update_actions (play/assets/stat-update-watch.mjs), which the test
// test/d2client-aeb10-stat-update.test.mjs pins to the 1.13c switch.
static int apply_player_stat(const uint8_t* packet, int length) {
    const auto read = [](const uint8_t* p, int n) {
        uint32_t value = 0;
        for (int i = 0; i < n; ++i) value |= uint32_t(p[i]) << (i * 8);
        return value;
    };
    D2UnitStrc* unit = nullptr;
    int stat = 0;
    uint32_t value = 0;
    if (packet[0] == 0x20) {
        if (length != 10) return -1;
        const uint32_t guid = read(packet + 1, 4);
        for (auto& player : g_players) if (player.view.guid == guid) { unit = player.motionUnit; break; }
        stat = packet[5]; value = read(packet + 6, 4);
    } else {
        static const int8_t kSize[] = { 2, 2, 3, 5, 3, 4, 6 };   // 0x19..0x1F
        if (packet[0] < 0x19 || packet[0] > 0x1f || length != kSize[packet[0] - 0x19]) return -1;
        if (g_selectedPlayer < 0 || size_t(g_selectedPlayer) >= g_players.size()) return -1;
        unit = g_players[g_selectedPlayer].motionUnit;
        if (!unit) return -1;   // retail: a null hero is fatal, handled like a missing selected record
        switch (packet[0]) {
        case 0x19: stat = 14; value = STATLIST_UnitGetStatValue(unit, 14, 0) + read(packet + 1, 1); break;
        case 0x1a: stat = 13; value = STATLIST_UnitGetStatValue(unit, 13, 0) + read(packet + 1, 1); break;
        case 0x1b: stat = 13; value = STATLIST_UnitGetStatValue(unit, 13, 0) + read(packet + 1, 2); break;
        case 0x1c: stat = 13; value = read(packet + 1, 4); break;
        default: stat = packet[1]; value = read(packet + 2, packet[0] == 0x1d ? 1 : packet[0] == 0x1e ? 2 : 4); break;
        }
    }
    if (!unit) return 0;
    STATLIST_SetUnitStat(unit, stat, static_cast<int32_t>(value), 0);
    if (packet[0] >= 0x1d && packet[0] <= 0x1f && stat == 6) revive_hero_on_life(unit, value);
    // A changed strength, dexterity or level can make a worn item usable or unusable.
    if (stat == STAT_STRENGTH || stat == STAT_DEXTERITY || stat == STAT_LEVEL)
        d2_realm_client_owned_item_reconcile(unit);
    return 1;
}

// 1.13c D2Client+0xAEBE0 (0x95, 13 bytes) and +0xAECA0 (0x18, 15 bytes): after the opcode byte the packet is a
// little-endian bit stream (the readers at +0xAC1B0 / +0xAC240): life, mana and stamina (15 bits each), for 0x18 the
// two potion percentages (7 bits each), then x and y (16 each) and two signed byte offsets to the walk target.
// The handlers store life, mana and stamina into the current player's stats 6, 8 and 10 shifted left by 8, 0x18 also
// stores the percentages unshifted into stats 0x4A and 0x1A, then both place the player through the placement the
// movement packets use (selector 0, target = position + offsets) and stand a dead player up when the life is nonzero.
static int apply_life_mana(const uint8_t* packet, int length) {
    if (length != (packet[0] == 0x95 ? 13 : 15)) return -1;
    if (g_selectedPlayer < 0 || size_t(g_selectedPlayer) >= g_players.size()) return 0;
    D2UnitStrc* unit = g_players[g_selectedPlayer].motionUnit;
    if (!unit) return 0;
    uint32_t at = 8;
    const auto take = [&](uint32_t count) {
        uint32_t value = 0;
        for (uint32_t i = 0; i < count; ++i, ++at) value |= uint32_t((packet[at >> 3] >> (at & 7)) & 1) << i;
        return value;
    };
    const uint32_t life = take(15), mana = take(15), stamina = take(15);
    STATLIST_SetUnitStat(unit, 6, static_cast<int32_t>(life << 8), 0);
    STATLIST_SetUnitStat(unit, 8, static_cast<int32_t>(mana << 8), 0);
    STATLIST_SetUnitStat(unit, 10, static_cast<int32_t>(stamina << 8), 0);
    if (packet[0] == 0x18) {
        const uint32_t potLife = take(7), potMana = take(7);
        STATLIST_SetUnitStat(unit, 0x4a, static_cast<int32_t>(potLife), 0);
        STATLIST_SetUnitStat(unit, 0x1a, static_cast<int32_t>(potMana), 0);
    }
    const uint32_t x = take(16), y = take(16), dx = take(8), dy = take(8);
    // The offsets are bytes read as signed, except that retail leaves 0x80 positive.
    const auto offset = [](uint32_t b) { return b > 0x80 ? static_cast<int32_t>(b) - 0x100 : static_cast<int32_t>(b); };
    pkt_place(x, reinterpret_cast<uintptr_t>(unit), y, 0,
              static_cast<uint16_t>(static_cast<int32_t>(x) + offset(dx)), static_cast<uint16_t>(static_cast<int32_t>(y) + offset(dy)));
    revive_hero_on_life(unit, life);
    return 1;
}

// 0x8E: the server names a corpse and its owner (flag set), or says the corpse is gone.
static int apply_corpse_assign(const uint8_t* packet, int length) {
    d2::realm::CorpsePacket record{};
    if (!d2::realm::decodeCorpsePacket(packet, length, 0x8e, &record)) return -1;
    return g_corpses.apply(record) ? 1 : 0;
}

// 0x74: a corpse was made this frame (see corpseDropAction).
static int apply_corpse_drop(const uint8_t* packet, int length) {
    d2::realm::CorpsePacket record{};
    if (!d2::realm::decodeCorpsePacket(packet, length, 0x74, &record)) return -1;
    D2UnitStrc* const owner = find_realm_client_unit(UNIT_PLAYER, record.owner);
    bool ownerIsHero = false;
    D2RealmClientPlayerEntry* const ownerEntry = player_entry_of(owner, &ownerIsHero);
    D2UnitStrc* const corpse = find_realm_client_unit(UNIT_PLAYER, record.corpse);
    const auto action = d2::realm::corpseDropAction(record, owner != nullptr && ownerEntry != nullptr,
        owner && d2::realm::playerIsDead(owner->dwFlags, owner->dwAnimMode), ownerIsHero, corpse != nullptr);
    if (!action.act) return 0;
    if (action.ownerFalls) apply_player_life_action(owner, 8, 0, 0, owner->dwLastHitClass);
    if (action.placeCorpseOnHero && owner->pDynamicPath) {
        bool corpseSelected = false;
        if (D2RealmClientPlayerEntry* const corpseEntry = player_entry_of(corpse, &corpseSelected)) {
            corpseEntry->motionDirection = PATH_GetDirection(owner->pDynamicPath);
            place_player_entry(*corpseEntry, corpseSelected,
                static_cast<uint32_t>(PATH_GetXPosition(owner->pDynamicPath)),
                static_cast<uint32_t>(PATH_GetYPosition(owner->pDynamicPath)));
        }
    }
    if (action.stripWornItems && owner->pInventory && owner->pInventory->dwSignature == D2C_InventoryHeader) {
        D2UnitStrc* item = INVENTORY_GetFirstItem(owner->pInventory);
        while (item) {
            D2UnitStrc* const next = INVENTORY_GetNextItem(item);
            if (INVENTORY_GetItemNodePage(item) == d2::realm::kInventoryNodePageBody) {
                INVENTORY_RemoveItem(item);
                d2_realm_client_owned_item_destroy(item);
            }
            item = next;
        }
    }
    return 1;
}

// The corpses the server has named: { version 1, count, then count pairs of owner GUID, corpse GUID }.
EMSCRIPTEN_KEEPALIVE const uint32_t* d2_realm_client_corpses() {
    static std::vector<uint32_t> out;
    out.assign(2 + g_corpses.entries.size() * 2, 0);
    out[0] = 1;
    out[1] = static_cast<uint32_t>(g_corpses.entries.size());
    for (size_t i = 0; i < g_corpses.entries.size(); ++i) {
        out[2 + i * 2] = g_corpses.entries[i].owner;
        out[3 + i * 2] = g_corpses.entries[i].corpse;
    }
    return out.data();
}

// The selected player's life, mana and stamina with their maxima (stat list values, 8.8 fixed point):
// { version 1, size 32, life, lifeMax, mana, manaMax, stamina, staminaMax }; zeros until a player is selected.
EMSCRIPTEN_KEEPALIVE const uint32_t* d2_realm_client_resources() {
    static uint32_t out[8];
    std::memset(out, 0, sizeof out);
    out[0] = 1; out[1] = 32;
    if (g_selectedPlayer >= 0 && size_t(g_selectedPlayer) < g_players.size()) {
        D2UnitStrc* unit = g_players[g_selectedPlayer].motionUnit;
        if (unit) for (int i = 0; i < 6; ++i) out[2 + i] = static_cast<uint32_t>(STATLIST_UnitGetStatValue(unit, 6 + i, 0));
    }
    return out;
}

static D2UnitStrc* selected_player_stat_unit() {
    return g_selectedPlayer >= 0 && size_t(g_selectedPlayer) < g_players.size()
        ? g_players[g_selectedPlayer].motionUnit : nullptr;
}

// The selected player's character-screen numbers, by the same computation the single-player panel
// reads (d2CharacterScreen): 16 int32. `expansion` is the character's Lord of Destruction flag.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_character_screen(int expansion, int32_t* out) {
    return d2CharacterScreen(selected_player_stat_unit(), expansion ? 1 : 0, static_cast<int>(g_clientDifficulty), out);
}

// A stat of the selected player with (base 0) or without (base 1) the worn items' and states' bonuses.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_hero_stat(int stat, int layer, int base) {
    D2UnitStrc* unit = selected_player_stat_unit();
    if (!unit || stat < 0 || stat > 511) return 0;
    return base ? STATLIST_GetUnitBaseStat(unit, stat, static_cast<uint16_t>(layer))
                : STATLIST_UnitGetStatValue(unit, stat, static_cast<uint16_t>(layer));
}

// The Belts.txt row of the selected player's worn belt; 2 (the four-box row) with none.
EMSCRIPTEN_KEEPALIVE uint32_t d2_realm_client_belt_index() {
    D2UnitStrc* unit = selected_player_stat_unit();
    D2UnitStrc* worn = unit && unit->pInventory ? INVENTORY_GetItemFromBodyLoc(unit->pInventory, BODYLOC_BELT) : nullptr;
    return worn && worn->dwUnitType == UNIT_ITEM ? static_cast<uint32_t>(UNITS_GetBeltType(worn)) : 2;
}

// The items a player holds (guid 0: the selected player), 12 words each; see d2_realm_client_unit_items.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_items(uint32_t ownerGuid, uint32_t* out, int capacity) {
    return d2_realm_client_unit_items(ownerGuid ? find_realm_client_unit(UNIT_PLAYER, ownerGuid)
        : selected_player_unit(), out, capacity);
}

// The item-box colour tests of one held item against the selected player.
EMSCRIPTEN_KEEPALIVE uint32_t d2_realm_client_item_box_state(uint32_t guid) {
    return d2_realm_client_unit_item_box_state(selected_player_unit(), guid);
}

EMSCRIPTEN_KEEPALIVE int d2_realm_client_player_stat(uint32_t guid, int stat) {
    if (stat < 0 || stat > 255) return 0;
    for (auto& player : g_players) if (player.view.guid == guid && player.motionUnit)
        return STATLIST_UnitGetStatValue(player.motionUnit, stat, 0);
    return 0;
}

// 0x15 is a room/location reassignment, not a path prediction. Only the exact assigned player
// (unit type 0 and matching server GUID) changes this record; other units remain out of scope.
static int reassign_player(const uint8_t* packet, int length) {
    D2RealmPlayerReassign update{};
    if (d2_realm_d2gs_parse_player_reassign(packet, length, &update) != 11) return -1;
    if (update.unitType != 0) return 0;
    int target = -1;
    for (size_t i = 0; i < g_players.size(); ++i) if (g_players[i].view.guid == update.guid) { target = static_cast<int>(i); break; }
    if (target < 0) return 0;
    const bool selected = target == g_selectedPlayer;
    D2RealmClientPlayerEntry& entry = g_players[target];
    // 1.13c D2Client+0xA7E20 leaves a dead unit where it is.
    if (entry.motionUnit && d2::realm::reassignRefusedForDead(entry.motionUnit->dwFlags,
            entry.motionUnit->dwAnimMode)) return 0;
    D2RealmClientPlayerState& player = selected ? g_player : entry.view;
    uint32_t level = 0, roomX = 0, roomY = 0, unusedInitSeed = 0;
    if (!d2_drlg_client_player_context(update.x, update.y, 0,
            &level, &roomX, &roomY, &unusedInitSeed)) return -2;
    player.x = update.x;
    player.y = update.y;
    if (!level) {
        player.clientRoomActive = 0;
        player.clientRoomLevel = player.clientRoomTileX = player.clientRoomTileY = 0;
    }
    player.reassignValue = update.value;
    player.reassignments++;
    refresh_player_room_level(player, selected);
    if (selected) entry.view = g_player;
    // 0x15 rebuilds/repositions the path even when the resolved room pointer is unchanged;
    // retaining a same-room route would let a later tick overwrite this server coordinate.
    if (!ensure_player_motion(entry, true)) return -3;
    if (selected) store_selected_player();
    return 1;
}

// Decoded bytes can end between server packets. Buffer only that suffix, then frame with the
// engine's pinned 1.13c size table on the next call. Unknown opcodes fail closed; unrelated valid
// packets are skipped without changing this player's state.
static uint32_t skill_u32(const uint8_t* p) {
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}

// Native 0x94 and 0x23 both look up an existing player GUID. Selection requires the
// actual CUnit skill+owner entry; do not mirror arbitrary server IDs into a parallel state.
static int apply_player_skills(const uint8_t* packet, int length) {
    const bool list = packet[0] == 0x94;
    if ((list && length != 6 + 3 * packet[1]) || (!list && length != 13)) return -1;
    if (!list && packet[1] != UNIT_PLAYER) return 0;
    const uint32_t guid = skill_u32(packet + 2);
    for (auto& player : g_players) {
        if (player.view.guid != guid) continue;
        if (!ensure_player_unit(player)) return -1;
        auto* unit = player.motionUnit;
        if (list) {
            if (!unit->pSkills) unit->pSkills = SKILLS_AllocSkillList(nullptr);
            if (!unit->pSkills) return -1;
            for (int row = 0; row < packet[1]; ++row) {
                const uint8_t* p = packet + 6 + row * 3;
                const int skill = p[0] | (int(p[1]) << 8);
                if (!DATATBLS_GetSkillsTxtRecord(skill)) return -1;
                SKILLS_AssignSkill(unit, skill, p[2], FALSE, __FILE__, __LINE__);
            }
        } else {
            const int skill = packet[7] | (int(packet[8]) << 8);
            if (!DATATBLS_GetSkillsTxtRecord(skill)) return -1;
            if (packet[6]) SKILLS_SetLeftActiveSkill(unit, skill, static_cast<int32_t>(skill_u32(packet + 9)));
            else SKILLS_SetRightActiveSkill(unit, skill, static_cast<int32_t>(skill_u32(packet + 9)));
        }
        return 1;
    }
    return 0;
}

// Game flags (0x01), the NPC speech marks (0x8A, 0x28) and the pet list with the hireling's stats (0x81,
// 0x7F, 0x9E..0xA2): the state lives in d2gs_client_game_packets.h, the unit lookups are done here.
static d2::realm::GameInfo g_gameInfo;
static d2::realm::NpcAlertState g_npcAlerts;
static d2::realm::PetState g_pets;
static d2::realm::MercResurrect g_mercResurrect;
static d2::realm::MovieQueue g_movieQueue;
// The in-game UI as the game-start packet leaves it, and how many times one has arrived. The count
// outlives a game: the page closes its panels when it moves.
static d2client_afa90::State g_gameUi;
static uint32_t g_gameUiStarts = 0;

// GUESS: the assignment's hireling test (D2Common ordinal 11104) is read as "a hireling"; the
// pet list's own kind and owner decide here.
static bool is_own_hireling(uint32_t guid) {
    for (const auto& pet : g_pets.pets)
        if (pet.guid == guid) return pet.kind == d2::realm::kPetKindHireling && pet.owner == g_player.guid;
    return false;
}

static void clear_game_packet_state() {
    g_unitQueue.clear();
    g_gameInfo = {};
    g_npcAlerts = {};
    g_pets = {};
    g_gameUi = {};
    g_mercResurrect = {};
    g_movieQueue = {};
}

static const D2RealmClientMonster* game_packet_monster(uint32_t guid) {
    for (const auto& monster : g_monsters) if (monster.view.guid == guid) return &monster;
    return nullptr;
}

static bool game_packet_class_is_npc(uint32_t classId) {
    const D2MonStatsTxt* stats = DATATBLS_GetMonStatsTxtRecord(static_cast<int>(classId));
    return stats && (stats->dwMonStatsFlags & MONSTATSFLAG_NPC);
}

static void game_packet_unit_removed(uint32_t guid) {
    d2::realm::npcAlertUnitRemoved(g_npcAlerts, guid);
    d2::realm::petUnitRemoved(g_pets, guid);
}

static void game_packet_player_removed(uint32_t guid) {
    d2::realm::npcAlertUnitRemoved(g_npcAlerts, guid, UNIT_PLAYER);
}

// Returns 1 when the packet changed client state, 0 when the retail handler would have left it alone.
static int apply_game_state_packet(const uint8_t* packet, int size) {
    using namespace d2::realm;
    switch (packet[0]) {
        case 0x01:
            if (!gameInfoApply(g_gameInfo, packet, size)) return 0;
            // The one difficulty the client keeps. The game has three; a byte naming none of them would
            // send the next act load to tables that do not exist, so the kept one stands.
            if (g_gameInfo.difficulty <= 2) g_clientDifficulty = g_gameInfo.difficulty;
            {
                // The same handler puts the in-game UI back to its start: every panel closed but the
                // three a running game always has, no NPC menu open.
                d2client_afa90::Env env;
                if (const D2UnitStrc* hero = selected_player_unit()) {
                    env.playerPresent = true;
                    env.playerType = hero->dwUnitType;
                    env.playerBusy = (hero->dwFlags & 0x10000u) != 0 || hero->dwAnimMode == 0 || hero->dwAnimMode == 0x11;
                }
                d2client_afa90::sub_AFA90(g_gameUi, packet, size, env);
                g_npcAlerts.hasActiveNpc = 0;
                g_npcAlerts.activeNpc = 0;
                ++g_gameUiStarts;
            }
            return 1;
        case 0x8a: {
            // Any unit the client holds takes the mark, the receiving player's own hero included.
            const D2UnitStrc* unit = size == 6 ? find_realm_client_unit(packet[1], gamePktU32(packet + 2)) : nullptr;
            return npcAlertApply(g_npcAlerts, packet, size, unit != nullptr, unit ? static_cast<uint32_t>(unit->dwClassId) : 0,
                unit ? unit->dwFlagEx : 0) == NpcAlertResult::Marked;
        }
        case 0x28: {
            bool found = false;
            if (size == 103 && packet[1] == UNIT_MONSTER) found = game_packet_monster(gamePktU32(packet + 2)) != nullptr;
            else if (size == 103 && packet[1] == UNIT_PLAYER) {
                for (const auto& player : g_players) if (player.view.guid == gamePktU32(packet + 2)) found = true;
            }
            return npcInteractReplyApply(g_npcAlerts, packet, size, found, game_packet_class_is_npc) > 0;
        }
        case 0x81: return petAssignApply(g_pets, packet, size);
        case 0x7f: return size == 10 && petLifeApply(g_pets, packet, size, game_packet_monster(gamePktU32(packet + 4)) != nullptr);
        default: return size >= 7 && petStatApply(g_pets, packet, size, game_packet_monster(gamePktU32(packet + 2)) != nullptr);
    }
}

// The game flags the server's 0x01 set. out[0..4] = difficulty, game flags, expansion, ladder, and the
// same in the save-flag layout a local game's client flags use. Returns 0 until the packet has arrived.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_game_info(uint32_t* out) {
    if (!out || !g_gameInfo.received) return 0;
    out[0] = g_gameInfo.difficulty; out[1] = g_gameInfo.gameFlags; out[2] = g_gameInfo.expansion; out[3] = g_gameInfo.ladder;
    out[4] = d2::realm::gameInfoSaveFlags(g_gameInfo);
    return 1;
}

// The 0x9B offer: out[0] = the dead mercenary's name id (0xFFFF none), out[1] = the price.
// Returns 1 when a Resurrect row is on offer, else 0.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_merc_resurrect(uint32_t* out) {
    if (!out) return 0;
    out[0] = g_mercResurrect.nameId; out[1] = g_mercResurrect.cost;
    return d2::realm::mercResurrectOffered(g_mercResurrect) ? 1 : 0;
}

// The next film index the server's 0x61 queued, or -1 when none waits.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_take_movie() {
    if (g_movieQueue.indexes.empty()) return -1;
    const int index = static_cast<int>(g_movieQueue.indexes.front());
    g_movieQueue.indexes.erase(g_movieQueue.indexes.begin());
    return index;
}

// The guids of the units carrying the speech mark. Returns the count written.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_npc_alerts(uint32_t* out, int cap) {
    int n = 0;
    for (const auto& alert : g_npcAlerts.alerts) { if (alert.type != UNIT_MONSTER) continue; if (!out || n >= cap) break; out[n++] = alert.guid; }
    return n;
}

// The units other than monsters carrying the same mark, a player's own hero among them: two words
// each (unit type, guid). Returns the count of units written.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_unit_marks(uint32_t* out, int cap) {
    int n = 0;
    for (const auto& alert : g_npcAlerts.alerts) {
        if (alert.type == UNIT_MONSTER) continue;
        if (!out || n >= cap) break;
        out[n * 2] = alert.type; out[n * 2 + 1] = alert.guid; ++n;
    }
    return n;
}

// The in-game UI the game-start packet set: out[0] counts the packets, out[1..38] are the panel open
// flags by panel number. Returns the count; 0 before the first one.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_game_ui(uint32_t* out) {
    if (!out || !g_gameUiStarts) return 0;
    out[0] = g_gameUiStarts;
    for (int i = 0; i < d2client_afa90::kPanelCount; ++i) out[1 + i] = g_gameUi.panel[i];
    return static_cast<int>(g_gameUiStarts);
}

// The NPC whose menu is open (active != 0) is not marked by a later 0x8A; closing the menu clears it.
EMSCRIPTEN_KEEPALIVE void d2_realm_client_set_active_npc(int active, uint32_t guid) {
    g_npcAlerts.hasActiveNpc = active ? 1u : 0u;
    g_npcAlerts.activeNpc = active ? guid : 0u;
}

// The pet list, newest first: nine words per pet (class, kind, guid, owner, area, life percentage, seed,
// name id, unit present). Returns the count written.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_pets(uint32_t* out, int cap) {
    int n = 0;
    for (const auto& pet : g_pets.pets) {
        if (!out || n >= cap) break;
        uint32_t* o = out + n++ * 9;
        o[0] = pet.classId; o[1] = pet.kind; o[2] = pet.guid; o[3] = pet.owner; o[4] = pet.area; o[5] = pet.lifePct;
        o[6] = pet.seed; o[7] = pet.nameId; o[8] = game_packet_monster(pet.guid) ? 1u : 0u;
    }
    return n;
}

// A stat the server set on a pet unit; INT32_MIN when it never sent one.
EMSCRIPTEN_KEEPALIVE int32_t d2_realm_client_pet_stat(uint32_t guid, int32_t stat) {
    int32_t value = 0;
    return d2::realm::petStatGet(g_pets, guid, stat, &value) ? value : INT32_MIN;
}


// ---- Server packets --------------------------------------------------------------------------------------
// Every packet reaches its handler the way the retail client's does: the D2Net size rule frames it, the
// client's dispatch loop (d2c_ace20) looks its opcode up in the client's packet table, checks the size and
// calls the row's handler. A handler body is registered under the offset of the retail handler it stands
// for, so giving an opcode a handler is one row in kRealmPacketHandlers and no edit to the feed.
// A body returns the number of records it applied, or a negative code that rejects the stream.
extern "C++" {
#include "../d2client/cl_env.h"
#include "../d2client/d2client_1ab20.h"
#include "../d2client/server_packet_table.h"
}
extern "C" void d2c_ace20(const cl::Env& e, uint32_t base, uint32_t buf, int32_t len);
extern "C" uint32_t d2_realm_d2net_6750(const uint8_t* buf, uint32_t avail, uint32_t* out);

using RealmPacketBody = int (*)(const uint8_t* packet, int size);

static int pkt_game_chat(const uint8_t* packet, int size) {
    D2RealmGameChat parsed{};
    if (d2_realm_d2gs_parse_gamechat(packet, size, &parsed) != size) return -2;
    g_gameChat = parsed;
    ++g_gameChatCount;
    return 1;
}
static int pkt_remove_unit(const uint8_t* packet, int size) {
    if (size != 6) return -2;
    // The handler keeps my own hireling when the server drops it from view: it finds it in the pet list.
    d2::realm::clientPetImageHero(selected_player_guid());
    D2cRemoveEnv env{ &g_pktEnv, d2::realm::kPetHeroRecord, d2::realm::clientPetImageRead(d2::realm::kPetHeadSlot),
        [](uintptr_t address) { return d2::realm::clientPetImageRead(static_cast<uint32_t>(address)); }, unit_remove_cb };
    g_removePacket = packet; g_removeLength = size; g_removeResult = 0;
    d2c_aca40(&env, packet);
    return g_removeResult;
}
// 0x81 feeds two lists: the client's pet list, and the mercenary roster's own entry of the same packet.
static int pkt_pet_assign(const uint8_t* packet, int size) {
    if (!d2::realm::clientPetPacket81(packet, size)) return -2;
    apply_game_state_packet(packet, size);
    return 1;
}
// 0x09 assigns a unit by type, guid, class and position. The server sends it for tile units: the level exits.
static int pkt_tile_assign(const uint8_t* packet, int size) { return d2_realm_client_warps_on_assign(g_warps, packet, size) < 0 ? -2 : 0; }
static int pkt_quest_game_record(const uint8_t* packet, int size) { return d2_realm_client_quests_on_game_record(g_quests, packet, size) ? 0 : -2; }
static int pkt_quest_log_states(const uint8_t* packet, int size) { return d2_realm_client_quests_on_log_states(g_quests, packet, size) ? 0 : -2; }
static int pkt_quest_state(const uint8_t* packet, int size) { return d2_realm_client_quests_on_state(g_quests, packet, size) ? 0 : -2; }
static int pkt_pet_owner(const uint8_t* packet, int size) {
    return d2::realm::clientPetPacket7A(packet, size, selected_player_guid()) ? 1 : -2;
}
static int pkt_object_assign(const uint8_t* packet, int size) {
    if (size != 14) return -2;
    D2cObjectEnv env{ object_create_cb };
    d2c_adf40(&env, packet);
    return 1;
}
static int pkt_quest_available(const uint8_t* packet, int size) {
    // 1.13c D2Client+0xAC090: the server's quest-available record. The 37 bytes after the opcode (one per
    // quest id) are copied to g_QuestAvail, the table the monster light setter reads at [1] for Den of Evil.
    if (size != 38) return -2;
    d2_client_lightmap_set_den_quest_avail(packet[2]);
    return 1;
}
static int pkt_load_act(const uint8_t* packet, int /*size*/) {
    // Native 1.13c LoadAct allocates a bClient act with the map seed. Packet act is
    // zero-based; the DRLG adapter takes the existing 1-based vector convention.
    const uint32_t seed = static_cast<uint32_t>(packet[2]) |
        (static_cast<uint32_t>(packet[3]) << 8) |
        (static_cast<uint32_t>(packet[4]) << 16) |
        (static_cast<uint32_t>(packet[5]) << 24);
    const int townLevel = static_cast<int>(packet[6]) | (static_cast<int>(packet[7]) << 8);
    destroy_all_monster_motion();
    destroy_all_player_motion();
    if (!d2_drlg_init()) return -5;   // no game data mounted: the act cannot be built
    if (!d2_drlg_client_act_begin(seed, static_cast<int>(g_clientDifficulty), packet[1] + 1, townLevel)) return -3;
    // The former act's room pointer is no longer live after LoadAct. Keep the CUnit seed,
    // but require a new assignment/reassignment to bind its current active-room identity.
    for (auto& entry : g_players) {
        entry.view.clientRoomActive = 0;
        entry.view.clientRoomLevel = entry.view.clientRoomTileX = entry.view.clientRoomTileY = 0;
    }
    g_player.clientRoomActive = 0;
    g_player.clientRoomLevel = g_player.clientRoomTileX = g_player.clientRoomTileY = 0;
    g_monsters.clear();
    g_monsterViews.clear();
    clear_objects();
    g_warps.list.clear();
    clear_monster_death_mode_events();
    store_selected_player();
    g_world.hasLoadAct = 1;
    g_world.act = packet[1];
    g_world.mapSeed = seed;
    g_world.area = static_cast<uint32_t>(packet[6]) |
        (static_cast<uint32_t>(packet[7]) << 8);
    g_world.rawLoadActTail = static_cast<uint32_t>(packet[8]) |
        (static_cast<uint32_t>(packet[9]) << 8) |
        (static_cast<uint32_t>(packet[10]) << 16) |
        (static_cast<uint32_t>(packet[11]) << 24);
    g_world.loadCompleteEvent = 0;
    g_world.hasMapReveal = 0;
    g_world.mapLevel = g_world.mapX = g_world.mapY = 0;
    g_world.loadActCount++;
    g_world.difficulty = g_clientDifficulty;
    return 0;
}
static int pkt_load_complete(const uint8_t* /*packet*/, int /*size*/) {
    if (g_selectedPlayer < 0 || !g_player.active) return -3; // 1.13c handler needs its current player for post-load refresh.
    g_world.loadCompleteEvent = 1;
    g_world.unloadCompleteEvent = 0;
    return 0;
}
static int pkt_unload_complete(const uint8_t* /*packet*/, int /*size*/) {
    // Native UnloadComplete closes loader resources and changes loader flags; it preserves
    // the active client act, room graph, assigned units, and their path resources.
    g_world.loadCompleteEvent = 0;
    g_world.unloadCompleteEvent = 1;
    return 0;
}
static int pkt_map_reveal(const uint8_t* packet, int /*size*/) {
    const uint32_t x = static_cast<uint32_t>(packet[1]) | (static_cast<uint32_t>(packet[2]) << 8);
    const uint32_t y = static_cast<uint32_t>(packet[3]) | (static_cast<uint32_t>(packet[4]) << 8);
    const uint32_t level = packet[5];
    const int hasPlayer = g_player.active && g_player.clientRoomActive;
    if (!g_world.hasLoadAct || !d2_drlg_client_map_reveal(static_cast<int>(level), static_cast<int>(x), static_cast<int>(y),
        hasPlayer, static_cast<int>(g_player.clientRoomLevel), static_cast<int>(g_player.clientRoomTileX), static_cast<int>(g_player.clientRoomTileY))) return -3;
    // PlayerAssign can precede the first active-room event. Re-resolve its location after
    // this MapReveal has built the room, then preserve that selection on other levels.
    refresh_all_player_motion_rooms();
    for (auto& monster : g_monsters) create_monster_motion(monster);
    place_unroomed_objects();
    g_world.hasMapReveal = 1;
    const uint32_t renderLevel = static_cast<uint32_t>(d2_drlg_client_render_level());
    if (renderLevel == level || !g_world.mapLevel) {
        g_world.mapX = x;
        g_world.mapY = y;
    }
    g_world.mapLevel = renderLevel;
    g_world.mapRevealCount++;
    g_world.mapRevision++;
    return 0;
}
static int pkt_environment(const uint8_t* packet, int /*size*/) {
    // Environment packet (client handler D2Client+0xAE9C0): cycle index, ticks, eclipse flag, applied to the
    // client act with the player's room. The retail handler then refreshes the day-dependent objects.
    const int32_t cycle = static_cast<int32_t>(static_cast<uint32_t>(packet[1]) | (static_cast<uint32_t>(packet[2]) << 8) |
        (static_cast<uint32_t>(packet[3]) << 16) | (static_cast<uint32_t>(packet[4]) << 24));
    const int32_t ticks = static_cast<int32_t>(static_cast<uint32_t>(packet[5]) | (static_cast<uint32_t>(packet[6]) << 8) |
        (static_cast<uint32_t>(packet[7]) << 16) | (static_cast<uint32_t>(packet[8]) << 24));
    // Retail applies it to the hero's act only (the act the hero's pDrlgAct names) with the hero's room.
    // cycle and ticks guards are defensive checks, not retail branches.
    D2DrlgActStrc* act = d2_drlg_client_act();
    D2UnitStrc* hero = nullptr;
    if (g_selectedPlayer >= 0 && size_t(g_selectedPlayer) < g_players.size())
        hero = g_players[g_selectedPlayer].motionUnit;
    if (hero && act && hero->pDrlgAct == act && UNITS_GetRoom(hero) &&
        cycle >= 0 && cycle < NUM_ENVIRONMENT_CYCLES && ticks >= 0)
        ENVIRONMENT_InitializeEnvironment(act, UNITS_GetRoom(hero), cycle, ticks, packet[9]);
    return 1;
}
static int pkt_map_hide(const uint8_t* packet, int /*size*/) {
    const uint32_t x = static_cast<uint32_t>(packet[1]) | (static_cast<uint32_t>(packet[2]) << 8);
    const uint32_t y = static_cast<uint32_t>(packet[3]) | (static_cast<uint32_t>(packet[4]) << 8);
    const uint32_t level = packet[5];
    const int hasPlayer = g_player.active && g_player.clientRoomActive;
    // UnsetClientIsInSight decrements the target room's sight count and propagates exact
    // reference-count changes through its near-room graph. Free only carriers whose own
    // active room will lose every status; paths in retained rooms keep their route.
    for (auto& monster : g_monsters) {
        D2UnitStrc* unit = monster.motionUnit;
        D2ActiveRoomStrc* room = unit && unit->pDynamicPath ? unit->pDynamicPath->pRoom : nullptr;
        if (room && d2_drlg_client_room_freed_by_hide(static_cast<int>(level), static_cast<int>(x),
                static_cast<int>(y), hasPlayer, static_cast<int>(g_player.clientRoomLevel),
                static_cast<int>(g_player.clientRoomTileX), static_cast<int>(g_player.clientRoomTileY), room))
            destroy_monster_motion(monster);
    }
    for (size_t i = 0; i < g_clientMissiles.size();) {
        auto* unit = g_clientMissiles[i].unit;
        auto* room = unit->pDynamicPath ? unit->pDynamicPath->pRoom : nullptr;
        if (room && d2_drlg_client_room_freed_by_hide(static_cast<int>(level), static_cast<int>(x),
                static_cast<int>(y), hasPlayer, static_cast<int>(g_player.clientRoomLevel),
                static_cast<int>(g_player.clientRoomTileX), static_cast<int>(g_player.clientRoomTileY), room)) {
            record_client_missile_event(6, unit);
            free_client_missile(unit);
            g_clientMissiles.erase(g_clientMissiles.begin() + i);
        } else ++i;
    }
    for (auto& player : g_players) {
        D2UnitStrc* unit = player.motionUnit;
        D2ActiveRoomStrc* room = unit && unit->pDynamicPath ? unit->pDynamicPath->pRoom : nullptr;
        if (room && d2_drlg_client_room_freed_by_hide(static_cast<int>(level), static_cast<int>(x),
                static_cast<int>(y), hasPlayer, static_cast<int>(g_player.clientRoomLevel),
                static_cast<int>(g_player.clientRoomTileX), static_cast<int>(g_player.clientRoomTileY), room))
            detach_player_motion_path(player);
    }
    for (D2UnitStrc* unit = d2gs_client_object_next(nullptr); unit; unit = d2gs_client_object_next(unit)) {
        D2ActiveRoomStrc* room = UNITS_GetRoom(unit);
        if (room && d2_drlg_client_room_freed_by_hide(static_cast<int>(level), static_cast<int>(x),
                static_cast<int>(y), hasPlayer, static_cast<int>(g_player.clientRoomLevel),
                static_cast<int>(g_player.clientRoomTileX), static_cast<int>(g_player.clientRoomTileY), room))
            d2gs_client_object_unplace(unit);
    }
    if (!g_world.hasLoadAct || !d2_drlg_client_map_hide(static_cast<int>(level), static_cast<int>(x), static_cast<int>(y),
        hasPlayer, static_cast<int>(g_player.clientRoomLevel), static_cast<int>(g_player.clientRoomTileX), static_cast<int>(g_player.clientRoomTileY))) return -3;
    refresh_all_player_motion_rooms();
    for (auto& monster : g_monsters) create_monster_motion(monster);
    // A tile unit leaves with its room.
    d2_realm_client_warps_keep_in_rooms(g_warps, [](int wx, int wy) { return d2_drlg_client_active_room(wx, wy) != nullptr; });
    g_world.mapHideCount++;
    g_world.mapRevision++;
    g_world.hasMapReveal = d2_drlg_client_tile_count() > 0;
    if (!g_world.hasMapReveal) g_world.mapLevel = g_world.mapX = g_world.mapY = 0;
    else g_world.mapLevel = static_cast<uint32_t>(d2_drlg_client_render_level());
    return 0;
}
static int pkt_unit_request(const uint8_t* packet, int size) {
    const int result = apply_retail_unit_packet(packet, size);
    return result < 0 ? (d2::realm::pktRoute(packet[0]).typed ? -2 : -4) : result;
}
static int pkt_select_player(const uint8_t* packet, int size) { return select_player(packet, size) < 0 ? -2 : 0; }
static int pkt_monster_assign(const uint8_t* packet, int size) { return apply_monster_assign(packet, size) ? 1 : -4; }
// 0x11: a one-shot overlay on a unit (1.13c D2Client+0xADAA0); two rows play a sound row as well.
static int pkt_unit_overlay(const uint8_t* packet, int size) {
    d2::realm::UnitWordPacket p{};
    if (!d2::realm::decodeUnitWordPacket(0x11, packet, size, &p)) return -2;
    D2UnitStrc* unit = find_realm_client_unit(p.unitType, p.guid);
    if (!sgptDataTables || !d2::realm::unitOverlayAccepted(unit != nullptr, p.value,
            sgptDataTables->nOverlayTxtRecordCount)) return 0;
    start_unit_overlay(unit, p.value, d2::realm::kUnitFxSkillRecord);
    const int sound = d2::realm::unitOverlaySound(p.value);
    if (sound >= 0) record_unit_fx(d2::realm::kUnitFxSound, unit, static_cast<uint32_t>(sound), 0,
        d2::realm::kUnitFxSkillRecord);
    return 1;
}
// 0x2C: a unit's sound event (1.13c D2Client+0xAD740); the page resolves the event to a sound. A taunt
// or a skill start names its Sounds row through the unit's own tables (+0x23F20); that row rides in the
// record's delay word (0 when there is none).
static int pkt_unit_sound(const uint8_t* packet, int size) {
    d2::realm::UnitWordPacket p{};
    if (!d2::realm::decodeUnitWordPacket(0x2c, packet, size, &p)) return -2;
    D2UnitStrc* unit = find_realm_client_unit(p.unitType, p.guid);
    if (!unit) return 0;
    uint32_t typeFlags = 0;
    int32_t superUnique = -1;
    if (p.unitType == UNIT_MONSTER)
        for (const auto& monster : g_monsters)
            if (monster.view.guid == p.guid) { typeFlags = monster.view.typeFlags; superUnique = monster.superUniqueIndex; break; }
    record_unit_fx(d2::realm::kUnitFxUnitSound, unit, p.value,
        d2::realm::clientUnitSoundRow(unit, p.value, typeFlags, superUnique), unit->dwClassId);
    return 1;
}
// 0x4D on an object is the object's own packet; on any other unit it starts a skill at a spot.
static int pkt_skill_start_at(const uint8_t* packet, int size) {
    const int result = packet[1] == UNIT_OBJECT ? apply_object_packet(packet, size) : apply_skill_start_packet(packet, size);
    return result < 0 ? -2 : result;
}
// A body whose refusal rejects the stream with one fixed code.
extern "C++" {
template <int (*Apply)(const uint8_t*, int), int Code>
static int pkt_or(const uint8_t* packet, int size) { const int result = Apply(packet, size); return result < 0 ? Code : result; }
// One retail handler, two readers of it: the engine-state body and the session-record body both see the packet.
template<int (*State)(const uint8_t*, int), int (*Record)(const uint8_t*, int)>
static int pkt_both(const uint8_t* packet, int size) {
    const int state = State(packet, size);
    const int record = Record(packet, size);
    return record < 0 ? (state < 0 ? record : state) : (state > record ? state : record);
}
}

// ---- The joined game's session records: the roster, the portals, and the things the page shows or plays
// once (a message line, a sound, an overlay). Each body below stands for the retail handler it is keyed by.
struct D2cAD6B0Env { void (*handle)(void* ctx, const uint8_t* copy); void* ctx; };
struct D2cAD2F0Env { uint32_t (*active)(void* ctx); void (*fatal)(void* ctx, uint32_t msg); void (*handle)(void* ctx, const uint8_t* copy); void* ctx; };
struct D2c0xad780Env { uint8_t* (*f9d630)(uint32_t edx); void (*f9da70)(); void (*f772e0)(); };
struct D2cBitReader { void (*open)(void* bb, const uint8_t* src, int32_t bytes); uint32_t (*read)(void* bb, int32_t bits); };
extern "C" void d2c_ad6b0(const uint8_t* pkt, const D2cAD6B0Env* e);
extern "C" void d2c_ad2f0(const uint8_t* pkt, const D2cAD2F0Env* e);
extern "C" void d2c_port_ad780(const uint8_t* pkt, const D2c0xad780Env& e);
extern "C" void d2c_port_ae260(const uint8_t* blk, uint32_t (*f4e20)(uint32_t key7, uint32_t rowRva, uint32_t kind), void (*f2fe0)(uint32_t hit));
extern "C" void d2c_port_ae2b0(const uint8_t* blk, uint32_t (*f4e20)(uint32_t key7, uint32_t rowRva, uint32_t kind), void (*f2fe0)(uint32_t hit));
extern "C" void d2_client_ac140(const uint8_t* src, uint8_t* dst, const D2cBitReader* f);
extern "C++" {
#include "../d2client/sub_ad950.h"
}

struct D2RealmRosterView {
    uint32_t guid, classId, level, party, relation, portalA, portalB, corpses;
    int32_t kills;
    uint8_t name[16];
};
static_assert(sizeof(D2RealmRosterView) == 52);
struct D2RealmPortalView {
    uint32_t guid, flags, area;
    uint8_t owner[16];
};
static_assert(sizeof(D2RealmPortalView) == 28);
// op is the packet. 0x2c: a = unit type, b = unit id, c = sound. 0x5a: a = kind, b = colour, c = number,
// d = sub-kind, with the two names. 0x8a: a = unit type, b = unit id, c = overlay row. 0x76: a = unit type,
// b = unit id. 0x74: a = the flag, b = owner id, c = corpse id. 0x47 and 0x48: a = unit type, b = unit id.
struct D2RealmSessionEvent {
    uint32_t op, a, b, c, d;
    uint8_t name1[16], name2[16];
};
static_assert(sizeof(D2RealmSessionEvent) == 52);
// What the game-wide packets leave behind. 0x01: the game's difficulty, flags, expansion and ladder bytes.
// 0x3f: the item the cursor is using (id, the packet's kind byte, and the two cursor words retail sets: 5 and 1
// at rest, 6 and 6 while an item is in use). 0x42: cursor items cleared. 0x5f: the dword kept on the player's
// data. 0x7c: the last item whose "in use" state was ended. 0x7e: the act whose shared animation data is loaded.
struct D2RealmGameSessionView {
    uint32_t gameFlagsSeen, difficulty, gameFlags, expansion, ladder;
    uint32_t useItemSeen, useItemGuid, useItemKind, useCursorMode, useCursorPhase, useItemWord;
    uint32_t cursorClears, cursorCleared;
    uint32_t heroDwordSeen, heroDword;
    uint32_t usedItemSeen, usedItemType, usedItemGuid, usedItemEnded;
    uint32_t actDataSeen, actDataAct;
};
static_assert(sizeof(D2RealmGameSessionView) == 84);

namespace {
struct RealmRosterEntry { D2RealmRosterView view; std::vector<uint32_t> corpseIds; };
std::vector<RealmRosterEntry> g_roster;
std::vector<D2RealmRosterView> g_rosterViews;
std::vector<D2RealmPortalView> g_portals;
std::vector<D2RealmSessionEvent> g_sessionEvents;
std::vector<std::string> g_sessionEventText;   // the composed line of a 0x5a event, UTF-8; empty for the others
uint32_t g_sessionEventsDropped = 0;
D2RealmGameSessionView g_gameSession{};
struct RealmRelation { uint32_t from, to, flags; };
std::vector<RealmRelation> g_relations;         // 0x8c: the flags one player holds toward another
uint32_t g_pong[8], g_pongCount = 0;
uint32_t g_rosterRevision = 0;
const uint8_t* g_sessionPacket = nullptr;   // the packet under a ported shell, for its callees
int g_sessionApplied = 0;
uint8_t g_sessionScratch[0x30];

uint32_t le32(const uint8_t* p) { return uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24; }
uint32_t le16(const uint8_t* p) { return uint32_t(p[0]) | uint32_t(p[1]) << 8; }

RealmRosterEntry* roster_find(uint32_t guid) {
    for (auto& entry : g_roster) if (entry.view.guid == guid) return &entry;
    return nullptr;
}
D2RealmPortalView* portal_for(uint32_t guid) {
    for (auto& portal : g_portals) if (portal.guid == guid) return &portal;
    g_portals.push_back({ guid, 0, 0, {} });
    return &g_portals.back();
}
const D2RealmClientObjectView* session_object(uint32_t guid) {
    for (const auto& object : g_objects) if (object.unitType == UNIT_OBJECT && object.guid == guid) return &object;
    return nullptr;
}
void session_event(const D2RealmSessionEvent& event) {
    // The page drains the queue every frame; a queue nobody drains keeps its first 256 and counts the rest.
    if (g_sessionEvents.size() >= 256) { ++g_sessionEventsDropped; return; }
    g_sessionEvents.push_back(event);
    g_sessionEventText.emplace_back();
}
D2UnitStrc* session_hero() {
    return g_selectedPlayer >= 0 && size_t(g_selectedPlayer) < g_players.size() ? g_players[g_selectedPlayer].motionUnit : nullptr;
}
// The 20-byte bit reader the walk check opens on its packet: least significant bit first.
struct SessionBits { const uint8_t* src; uint32_t at; };
void session_bits_open(void* bb, const uint8_t* src, int32_t) { const SessionBits bits{ src, 0 }; std::memcpy(bb, &bits, sizeof bits); }
uint32_t session_bits_read(void* bb, int32_t count) {
    SessionBits bits;
    std::memcpy(&bits, bb, sizeof bits);
    uint32_t value = 0;
    for (int32_t i = 0; i < count; ++i, ++bits.at) value |= uint32_t((bits.src[bits.at >> 3] >> (bits.at & 7)) & 1) << i;
    std::memcpy(bb, &bits, sizeof bits);
    return value;
}
}  // namespace

static void clear_session_records() {
    g_roster.clear(); g_rosterViews.clear(); g_portals.clear(); g_sessionEvents.clear(); g_sessionEventText.clear();
    g_relations.clear(); std::memset(&g_gameSession, 0, sizeof g_gameSession);
    g_sessionEventsDropped = g_pongCount = g_rosterRevision = 0;
    std::memset(g_pong, 0, sizeof g_pong);
}

// 0x5b (0xad8f0, 0x9e3d0): a player is in the game. A zeroed roster record takes the id, class, name, level and
// party of the packet. A record made earlier for the same id (by a corpse notice) is the one filled.
static int pkt_roster_add(const uint8_t* packet, int size) {
    if (size < 28) return -2;
    const uint32_t guid = le32(packet + 3);
    RealmRosterEntry* entry = roster_find(guid);
    if (!entry) { g_roster.push_back({}); entry = &g_roster.back(); }
    auto& v = entry->view;
    v.guid = guid; v.classId = packet[7]; v.level = le16(packet + 24); v.party = le16(packet + 26);
    std::memcpy(v.name, packet + 8, 16); v.name[15] = 0;
    ++g_rosterRevision;
    return 1;
}
// 0x65 (0xad890): the record's kill count is the packet's word, sign-extended.
static int pkt_roster_kills(const uint8_t* packet, int) {
    RealmRosterEntry* entry = roster_find(le32(packet + 1));
    if (!entry) return 0;
    entry->view.kills = static_cast<int16_t>(le16(packet + 5));
    ++g_rosterRevision;
    return 1;
}
// 0x75 (0xad870, 0x9e380): party, level and the relation words, unless the record's corpse list names the id.
static int pkt_roster_about(const uint8_t* packet, int) {
    const uint32_t guid = le32(packet + 1);
    RealmRosterEntry* entry = roster_find(guid);
    if (!entry) return 0;
    for (uint32_t id : entry->corpseIds) if (id == guid) return 0;
    entry->view.party = le16(packet + 5); entry->view.level = le16(packet + 7); entry->view.relation = le32(packet + 9);
    ++g_rosterRevision;
    return 1;
}
// 0x8d (0xad780): the record's party id. The port writes the word at record+0x22 of the retail record; it is
// handed a scratch record and the word is copied to ours.
static uint8_t* session_party_record(uint32_t guid) { return roster_find(guid) ? g_sessionScratch : nullptr; }
static void session_party_refresh() { g_sessionApplied = 1; }
static void session_no_call() {}
static int pkt_roster_party(const uint8_t* packet, int) {
    g_sessionApplied = 0;
    const D2c0xad780Env env{ session_party_record, session_no_call, session_party_refresh };
    d2c_port_ad780(packet, env);
    if (!g_sessionApplied) return 0;
    roster_find(le32(packet + 1))->view.party = le16(g_sessionScratch + 0x22);
    ++g_rosterRevision;
    return 1;
}
// 0x8e (0xad130): a corpse id joins (flag set, 0x9d760) or leaves (0x9d700) its owner's record; joining an owner
// nobody holds a record for makes the record.
static int pkt_roster_corpse(const uint8_t* packet, int) {
    const uint32_t owner = le32(packet + 2), corpse = le32(packet + 6);
    RealmRosterEntry* entry = roster_find(owner);
    if (packet[1]) {
        if (!entry) { g_roster.push_back({}); entry = &g_roster.back(); entry->view.guid = owner; }
        if (std::find(entry->corpseIds.begin(), entry->corpseIds.end(), corpse) == entry->corpseIds.end()) entry->corpseIds.push_back(corpse);
    } else if (entry) {
        entry->corpseIds.erase(std::remove(entry->corpseIds.begin(), entry->corpseIds.end(), corpse), entry->corpseIds.end());
    } else return 0;
    entry->view.corpses = static_cast<uint32_t>(entry->corpseIds.size());
    ++g_rosterRevision;
    return 1;
}
// 0x82 (0xad160): the owner's name goes onto each of the two portal objects that exist; with a second portal id
// the owner's roster record keeps both ids. The port walks the retail unit table in place, which this state does
// not hold, so the same steps run on the object list.
static int pkt_portal_owner(const uint8_t* packet, int) {
    const uint32_t a = le32(packet + 0x15), b = le32(packet + 0x19);
    if (session_object(a)) std::memcpy(portal_for(a)->owner, packet + 5, 16);
    if (b == 0xffffffffu) return 1;
    if (session_object(b)) std::memcpy(portal_for(b)->owner, packet + 5, 16);
    if (RealmRosterEntry* entry = roster_find(le32(packet + 1))) { entry->view.portalA = a; entry->view.portalB = b; ++g_rosterRevision; }
    return 1;
}
// 0x60 (0xad2f0, 0x247a0): a portal object (class 0x3b or 0x3c) takes the packet's destination area and gains the
// packet's two low flag bits. No such object: nothing. Retail stops the game on any other unit; here it is skipped.
static uint32_t session_game_active(void*) { return g_world.hasLoadAct; }
static void session_fatal(void*, uint32_t) {}
static void session_portal_state(void*, const uint8_t* copy) {
    const D2RealmClientObjectView* object = session_object(le32(copy + 3));
    if (!object || (object->classId != 0x3b && object->classId != 0x3c)) return;
    D2RealmPortalView* portal = portal_for(object->guid);
    portal->area = copy[2];
    portal->flags |= copy[1] & 3u;
    g_sessionApplied = 1;
}
static int pkt_portal_state(const uint8_t* packet, int) {
    g_sessionApplied = 0;
    const D2cAD2F0Env env{ session_game_active, session_fatal, session_portal_state, nullptr };
    d2c_ad2f0(packet, &env);
    return g_sessionApplied;
}

// 0x8c (0xad7b0): the flags one player holds toward another, kept on the first player's roster record. The line
// composer reads flag 4 of the local player's entry to keep a muted player's party lines out of the log.
static int pkt_roster_relation(const uint8_t* packet, int) {
    const uint32_t from = le32(packet + 1), to = le32(packet + 5);
    if (!roster_find(from)) return 0;
    for (auto& r : g_relations) if (r.from == from && r.to == to) { r.flags = le16(packet + 9); return 1; }
    g_relations.push_back({ from, to, le16(packet + 9) });
    return 1;
}
static uint32_t relation_flags(uint32_t from, uint32_t to) {
    if (from == to) return 0;
    for (const auto& r : g_relations) if (r.from == from && r.to == to) return r.flags;
    return 0;
}

// ---- The own player's click on a player unit (1.13c D2Client+0x1AB20, unit type 0) and the arrival gate of
// the action it leaves waiting (+0x2AFB0, called by the queue worker +0x2B090). Both run as ported; their unit
// reads go to 32-bit records in retail's layout (+0 type, +4 class, +0xC id, +0x10 mode, +0x14 player data,
// +0xC4 flags) built from the client units, and their callees go to this client's own steps.
extern "C" uint32_t d2c_2afb0(uint32_t rec, uint32_t id, uint32_t a1, uint32_t a2, uint32_t a3, const cl::Env& e);
extern "C++" {   // C++ linkage: this file's extern "C" block would make these names one entity with line5a's
namespace unitclick {
struct Rec { uint32_t w[0xc8 / 4]; };
Rec g_hero, g_target;
uint32_t g_latch[2];                    // DWORD 0x11C2F4 (pending target) and 0x11C2F8 (busy latch)
uint8_t g_rosterRec[0x20];
D2UnitStrc *g_heroUnit, *g_targetUnit;
uint32_t g_type, g_guid, g_kind;
bool g_run;
int g_started;
inline uint32_t p32(const void* p) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p)); }
void fill(Rec& r, const D2UnitStrc* u) {
    std::memset(&r, 0, sizeof r);
    r.w[0] = u->dwUnitType; r.w[1] = static_cast<uint32_t>(u->dwClassId); r.w[3] = u->dwUnitId;
    r.w[4] = u->dwAnimMode; r.w[0xc4 / 4] = u->dwFlags;
}
D2UnitStrc* unit_of(const void* p) { return p == &g_hero ? g_heroUnit : p == &g_target ? g_targetUnit : nullptr; }
uint32_t rd(void*, uint32_t a) { uint32_t v; std::memcpy(&v, reinterpret_cast<const void*>(static_cast<uintptr_t>(a)), 4); return v; }
void wr(void*, uint32_t a, uint32_t v) { std::memcpy(reinterpret_cast<void*>(static_cast<uintptr_t>(a)), &v, 4); }
// The callees of +0x2AFB0 by their retail addresses and the registers the port passes.
uint32_t call(void*, uint32_t rva, uint32_t eax, uint32_t ecx, uint32_t edx, uint32_t ebx, uint32_t esi,
              uint32_t s0, uint32_t, uint32_t, uint32_t, uint32_t) {
    switch (rva) {
    case 0x9d630: {                     // the roster record of a player, or of a corpse's owner: its id at +0x10
        const uint32_t owner = roster_find(edx) ? edx : g_corpses.ownerOf(edx);
        if (!owner) return 0;
        std::memset(g_rosterRec, 0, sizeof g_rosterRec);
        std::memcpy(g_rosterRec + 0x10, &owner, 4);
        return p32(g_rosterRec);
    }
    case 0x4fd90: return relation_flags(edx, esi) & s0;   // the flags the owner holds toward the hero
    case 0x2a7c0: g_started = local_fire(g_heroUnit, s0, g_type, g_guid); return 0;   // EAX = hero, stack the action
    case 0x29e80: return local_must_walk(g_heroUnit, g_targetUnit) ? 1 : 0;           // EAX = action, ECX = hero
    case 0x2aa40: g_started = local_walk_first(g_heroUnit, s0, g_type, g_guid, g_run); return 0;
    default: (void)eax; (void)ecx; (void)ebx; return 0;
    }
}
// The callees of +0x1AB20 its player branch reaches.
int32_t distance(void* tgt, void* pl) { return D2Common_10399(unit_of(tgt), unit_of(pl)); }                 // #10407
int32_t collide(void* pl, void* tgt, int32_t m) { return UNITS_TestCollisionBetweenInteractingUnits(unit_of(pl), unit_of(tgt), m); }   // #10324
int32_t gate() { return local_action_gate(g_heroUnit) ? 1 : 0; }                                            // 0x29F30
void use(void*, const uint32_t*, uint32_t) { g_started = local_fire(g_heroUnit, 0x13, g_type, g_guid); }   // 0x2A750: action 2, the use
void walk(void*, uint32_t action, uint32_t) { g_started = local_fire(g_heroUnit, action, g_type, g_guid); } // 0x2A7C0
void arm(void*, uint32_t action, uint32_t type, uint32_t guid) { g_localQueued = { 1, action, type, guid }; }  // 0x2D50
}  // namespace unitclick
}  // extern "C++"

// +0x2B090 kind 0: the waiting action on a player unit is gated by +0x2AFB0. A dying or lying player, or one
// flagged dead, is used at once when it is the hero's own or its owner lets the hero (relation bit 1); a live
// one only while it is targetable (flag 2), walking again when out of reach; anything else is dropped.
static void local_player_arrival(D2UnitStrc* unit, D2UnitStrc* target, uint32_t kind, uint32_t guid, bool run) {
    using namespace unitclick;
    g_heroUnit = unit; g_targetUnit = target; g_type = UNIT_PLAYER; g_guid = guid; g_run = run; g_started = 0;
    fill(g_hero, unit); fill(g_target, target);
    cl::Env env{};
    env.ctx = nullptr; env.h = call; env.rd = rd; env.wr = wr; env.gl = p32(g_latch);
    if (d2c_2afb0(p32(&g_target), guid, p32(&g_hero), kind, 0, env))
        g_started = local_fire(unit, kind, UNIT_PLAYER, guid);   // +0x2B090: a nonzero answer runs +0x2A7C0
}

// A left click on a player unit (a live player or a corpse): +0x1AB20 type 0. In reach (D2Common #10407 under 5
// with nothing between, #10324 mask 0x804) and past the gate the use starts and its 0x13 is in the send list;
// otherwise the walk or run to it starts (0x02/0x04) and the use waits for +0x2B090. 1 something started,
// 0 refused, -1 no local player or no client unit for that id.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_local_click_player(uint32_t guid, int run) {
    using namespace unitclick;
    D2UnitStrc* unit = selected_player_unit();
    if (!unit) return -1;
    D2UnitStrc* target = find_realm_client_unit(UNIT_PLAYER, guid);
    if (!target || target == unit || !UNITS_GetRoom(target) || !UNITS_GetRoom(unit)) return -1;
    if (unit->dwAnimMode == PLRMODE_DEATH || unit->dwAnimMode == PLRMODE_DEAD) return 0;
    g_localRun = run != 0;
    g_heroUnit = unit; g_targetUnit = target; g_type = UNIT_PLAYER; g_guid = guid; g_run = run != 0; g_started = 0;
    fill(g_hero, unit); fill(g_target, target);
    uint32_t frame[7] = {};
    frame[1] = p32(&g_hero); frame[2] = p32(&g_target); frame[6] = run ? 0x04 : 0x02;   // +0x18: the walk or run action
    uint32_t g97ec = 0, g9828 = 0, g3e4 = 0;
    const uint32_t masks[2] = {};
    const d2client_1ab20::Gl gl{ 0, &g97ec, &g9828, &g3e4, nullptr, masks };
    d2client_1ab20::Fn fn{};
    fn.cc70 = distance; fn.cf0a = collide; fn.s9f30 = gate; fn.s750 = use; fn.s7c0 = walk; fn.s2d50 = arm;
    d2client_1ab20::sub_1AB20(UNIT_PLAYER, reinterpret_cast<const uint8_t*>(frame), static_cast<int32_t>(guid), gl, fn);
    return g_started;
}

// The 0x5a line. The composer (0x7dfb0, with 0x7cfc0 for the party kinds) is run as retail runs it: every string
// comes from the string table by index, names are widened, and the finished text goes to the print call, whose
// text and colour become the event. The composer addresses its inputs as 32-bit words; they are this module's
// static records and the composer's own locals.
extern "C" const char* d2_lang_string_by_index(int index);
extern "C" void d2c_7dfb0(uint32_t msg, const cl::Env& e);
extern "C" uint32_t d2c_7cfc0(uint32_t out, uint32_t msg, uint32_t ps, const cl::Env& e);
namespace line5a {
constexpr uint32_t kModule = 0x6FAB0000u, kModuleEnd = 0x6FC00000u, kSelf = kModule + 0x11BBFC;
uint16_t g_str[8][0x200]; uint32_t g_strNext;
uint8_t g_row[0x80], g_rosterRec[0x80];
uint32_t g_selfRec[8];
const uint8_t* g_msg;
int g_lines;
inline uint32_t p32(const void* p) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p)); }
inline uint16_t* w16(uint32_t a) { return reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(a)); }
inline uint8_t* b8(uint32_t a) { return reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(a)); }
uint32_t wlen(const uint16_t* s) { uint32_t n = 0; while (s[n]) ++n; return n; }
uint32_t table_string(uint32_t id) {
    uint16_t* out = g_str[g_strNext++ & 7];
    const uint8_t* s = reinterpret_cast<const uint8_t*>(d2_lang_string_by_index(static_cast<int>(id & 0xffff)));
    uint32_t n = 0;
    while (*s && n < 0x1ff) {
        uint32_t c = *s++;
        if (c >= 0xf0) { c = ((c & 7) << 18); for (int i = 0; i < 3 && *s; ++i) c |= (uint32_t(*s++) & 0x3f) << (12 - 6 * i); }
        else if (c >= 0xe0) { c = (c & 0xf) << 12; for (int i = 0; i < 2 && *s; ++i) c |= (uint32_t(*s++) & 0x3f) << (6 - 6 * i); }
        else if (c >= 0xc0) { c = (c & 0x1f) << 6; if (*s) c |= uint32_t(*s++) & 0x3f; }
        out[n++] = c > 0xffff ? 0xfffd : static_cast<uint16_t>(c);
    }
    out[n] = 0;
    return p32(out);
}
std::string to_utf8(const uint16_t* s) {
    std::string out;
    for (; *s; ++s) {
        const uint32_t c = *s;
        if (c < 0x80) out += static_cast<char>(c);
        else if (c < 0x800) { out += static_cast<char>(0xc0 | (c >> 6)); out += static_cast<char>(0x80 | (c & 0x3f)); }
        else { out += static_cast<char>(0xe0 | (c >> 12)); out += static_cast<char>(0x80 | ((c >> 6) & 0x3f)); out += static_cast<char>(0x80 | (c & 0x3f)); }
    }
    return out;
}
// The wide sprintf the composer uses: %s is a wide string, %d and %u a number, %% a percent sign.
void wsprintf(uint32_t cap, uint16_t* out, const uint16_t* fmt, const uint32_t* args) {
    uint32_t n = 0, a = 0;
    auto put = [&](uint16_t c) { if (n + 1 < cap) out[n++] = c; };
    for (; *fmt; ++fmt) {
        if (*fmt != '%') { put(*fmt); continue; }
        const uint16_t k = *++fmt;
        if (!k) break;
        if (k == '%') put('%');
        else if (k == 's') { if (a < 2 && args[a]) for (const uint16_t* s = w16(args[a]); *s; ++s) put(*s); ++a; }
        else if (k == 'd' || k == 'u') {
            char digits[16];
            const uint32_t v = a < 2 ? args[a] : 0; ++a;
            if (k == 'd') std::snprintf(digits, sizeof digits, "%d", static_cast<int32_t>(v));
            else std::snprintf(digits, sizeof digits, "%u", v);
            for (const char* d = digits; *d; ++d) put(static_cast<uint16_t>(*d));
        } else { put('%'); put(k); }
    }
    out[n] = 0;
}
void print(uint32_t text, uint32_t colour, uint32_t list) {
    const uint16_t* s = w16(text);
    if (!s[0]) return;                                   // an empty text is no line
    D2RealmSessionEvent event{ 0x5a, g_msg[1], colour & 0xff, le32(g_msg + 3), g_msg[7], {}, {} };
    std::memcpy(event.name1, g_msg + 8, 16); event.name1[15] = 0;
    std::memcpy(event.name2, g_msg + 0x18, 16); event.name2[15] = 0;
    (void)list;
    const size_t before = g_sessionEvents.size();
    session_event(event);
    if (g_sessionEvents.size() != before) g_sessionEventText.back() = to_utf8(s);
    ++g_lines;
}
uint32_t rd(void*, uint32_t a) {
    if (a == kSelf) return session_hero() ? p32(g_selfRec) : 0;
    if (a >= kModule && a < kModuleEnd) return 0;
    uint32_t v; std::memcpy(&v, b8(a), 4); return v;
}
void wr(void*, uint32_t a, uint32_t v) {
    if (a >= kModule && a < kModuleEnd) return;          // the client's own globals (the kind 0x12 timers): not held here
    std::memcpy(b8(a), &v, 4);
}
uint32_t call(void* ctx, uint32_t rva, uint32_t eax, uint32_t ecx, uint32_t edx, uint32_t ebx, uint32_t esi,
              uint32_t s0, uint32_t s1, uint32_t s2, uint32_t s3, uint32_t s4);
const cl::Env kEnv{ nullptr, call, rd, wr, 0, 0 };
uint32_t call(void*, uint32_t rva, uint32_t eax, uint32_t ecx, uint32_t edx, uint32_t ebx, uint32_t esi,
              uint32_t s0, uint32_t s1, uint32_t s2, uint32_t s3, uint32_t s4) {
    switch (rva) {
    case 0xd1ec: return table_string(ecx);
    case 0xceab0: return wlen(w16(ecx));
    case 0xceab4: { const uint32_t args[2] = { s3, s4 }; wsprintf(s0, w16(s1), w16(s2), args); return 0; }
    case 0xceab8: { const uint16_t* s = w16(edx); uint16_t* d = w16(ecx); do { *d++ = *s; } while (*s++); return ecx; }
    case 0xceabc: { const uint16_t* s = w16(edx); uint16_t* d = w16(ecx) + wlen(w16(ecx)); do { *d++ = *s; } while (*s++); return ecx; }
    case 0xceac0: {                                      // widen at most s0 characters, the end mark included
        const uint8_t* s = b8(edx); uint16_t* d = w16(ecx);
        uint32_t i = 0;
        for (; i + 1 < s0 && s[i]; ++i) d[i] = s[i];
        d[i] = 0;
        return ecx;
    }
    case 0x9d850: return p32(g_player.name);             // the local player's name
    case 0xc226: {                                       // an object class's record: its wide name is at 0x40
        const D2ObjectsTxt* row = DATATBLS_GetObjectsTxtRecord(static_cast<int>(s0));
        if (!row) return 0;
        std::memcpy(g_row, row, sizeof g_row);
        g_row[0x7e] = g_row[0x7f] = 0;
        return p32(g_row);
    }
    case 0xc604: {                                       // a super unique's record: its name string index is at 2
        if (!sgptDataTables || static_cast<int>(s0) >= DATATBLS_GetSuperUniquesTxtRecordCount()) return 0;
        const D2SuperUniquesTxt* row = DATATBLS_GetSuperUniquesTxtRecord(static_cast<int>(s0));
        if (!row) return 0;
        std::memcpy(g_row, row, 8);
        return p32(g_row);
    }
    case 0x1230: {                                       // a monster class's record: its name string index is at 6
        if (!sgptDataTables || !sgptDataTables->pMonStatsTxt || eax >= uint32_t(sgptDataTables->nMonStatsTxtRecordCount)) return 0;
        std::memcpy(g_row, &sgptDataTables->pMonStatsTxt[eax], 8);
        return p32(g_row);
    }
    case 0x9d630: {                                      // a roster record: id at 0x10, name at 0x66
        const RealmRosterEntry* entry = roster_find(edx);
        if (!entry) return 0;
        std::memset(g_rosterRec, 0, sizeof g_rosterRec);
        std::memcpy(g_rosterRec + 0x10, &entry->view.guid, 4);
        std::memcpy(g_rosterRec + 0x66, entry->view.name, 16);
        return p32(g_rosterRec);
    }
    case 0x4fd90: return relation_flags(edx, esi) & s0;  // the local player's flags toward the other
    case 0x7cfc0: return d2c_7cfc0(eax, s0, kSelf, kEnv);
    case 0x7d850: print(s0, s1, 0); return 0;            // the message list
    case 0x7d610: print(s0, s1, 1); return 0;            // the party kinds go to the second list
    case 0x88a70: session_event({ 0x88a70, ebx, 0, 0, 0, {}, {} }); return 0;   // an interface sound
    default: return 0;                                   // the fatal path, the clock and the kind 0x12 panel work
    }
}
}  // namespace line5a
// 0x5a (0xad6b0, 0x7dfb0): a game event line, composed from the string table and printed in the packet's colour.
// The event carries the finished text beside the parts. The local player's own "joined" notice prints nothing,
// and kinds 0xc, 0xe and those past 0x12 are no line.
static void session_event_message(void*, const uint8_t* m) {
    line5a::g_msg = m; line5a::g_lines = 0;
    line5a::g_selfRec[3] = g_player.guid;                // the local player's id, where the composer reads it
    d2c_7dfb0(line5a::p32(m), line5a::kEnv);
    if (line5a::g_lines) g_sessionApplied = 1;
}
static int pkt_event_message(const uint8_t* packet, int) {
    g_sessionApplied = 0;
    const D2cAD6B0Env env{ session_event_message, nullptr };
    d2c_ad6b0(packet, &env);
    return g_sessionApplied;
}
// 0x8a (0xadb30, 0x47970): an NPC has something to say: the unit gains overlay row 0x48. Retail leaves out the
// unit the player is talking to and gives two monster classes a sound; the page holds that conversation state.
static int pkt_npc_wants_talk(const uint8_t* packet, int) {
    if (!find_realm_client_unit(packet[1], le32(packet + 2))) return 0;
    session_event({ 0x8a, packet[1], le32(packet + 2), 0x48, 0, {}, {} });
    return 1;
}
// 0x76 (0xad370): the unit's overhead text is freed.
static int pkt_overhead_clear(const uint8_t* packet, int) {
    if (!find_realm_client_unit(packet[1], le32(packet + 2))) return 0;
    session_event({ 0x76, packet[1], le32(packet + 2), 0, 0, {}, {} });
    return 1;
}
// 0x74 (0xadef0, 0x194e0): a corpse is tied to its owner. Retail moves the owner's worn items onto the corpse
// unit; this state holds no corpse unit to move them to, so the tie alone is reported.
static int pkt_corpse_assign(const uint8_t* packet, int) {
    if (!packet[1]) return 0;
    session_event({ 0x74, packet[1], le32(packet + 2), le32(packet + 6), 0, {}, {} });
    return 1;
}
// 0x47 and 0x48 (0xae2b0, 0xae260, then 0x82fe0): the unit's worn items are checked against its requirements
// again. The lookup runs; the reconcile needs the client's item-stat and item-graphics routines, which this state
// does not hold, so the page is told which unit to read again.
static uint32_t session_equip_lookup(uint32_t, uint32_t, uint32_t kind) {
    return find_realm_client_unit(static_cast<uint8_t>(kind), le32(g_sessionPacket + 3)) ? 1u : 0u;
}
static void session_equip_reconcile(uint32_t) {
    session_event({ g_sessionPacket[0], g_sessionPacket[1], le32(g_sessionPacket + 3), 0, 0, {}, {} });
    g_sessionApplied = 1;
}
static int pkt_equip_recheck(const uint8_t* packet, int) {
    g_sessionPacket = packet; g_sessionApplied = 0;
    if (packet[0] == 0x47) d2c_port_ae2b0(packet, session_equip_lookup, session_equip_reconcile);
    else d2c_port_ae260(packet, session_equip_lookup, session_equip_reconcile);
    return g_sessionApplied;
}
// 0x96 (0xad950): the walk check. The local player's stamina is set from the packet and the player is placed at
// the packet's position through the placement the movement packets use (a far position asks for a resync).
static void session_walk_unpack(uint32_t, uint8_t* record) {
    static const D2cBitReader reader{ session_bits_open, session_bits_read };
    d2_client_ac140(g_sessionPacket, record, &reader);
    std::memcpy(g_sessionScratch, record, 9);
}
static void session_walk_stamina(uint32_t, uint32_t stat, uint32_t value, uint32_t layer) {
    pkt_set_stat(reinterpret_cast<uintptr_t>(session_hero()), stat, value, layer);
}
static void session_walk_place(uint32_t y, uint32_t selector, uint32_t nearX, uint32_t nearY) {
    // Retail loads the record's own x into the register the placement reads it from; the first stack word is y.
    pkt_place(le16(g_sessionScratch + 3), reinterpret_cast<uintptr_t>(session_hero()), y, selector, nearX, nearY);
    g_sessionApplied = 1;
}
static int pkt_walk_verify(const uint8_t* packet, int) {
    g_sessionPacket = packet; g_sessionApplied = 0;
    const d2client_ad950::Env env{ session_walk_unpack, session_walk_stamina, session_walk_place, session_hero() ? 1u : 0u };
    d2client_ad950::sub_AD950(env, 0);
    return g_sessionApplied;
}
// 0x01 (0xafa90): the game's difficulty, flags, expansion and ladder bytes; retail then puts the interface back
// to its opening state (panels shut, automap and message log emptied), which the event asks of the page.
static int pkt_game_flags(const uint8_t* packet, int) {
    // The game has three difficulties; a byte naming none keeps the one the client holds (the act load reads tables by it).
    if (packet[1] <= 2) g_clientDifficulty = packet[1];
    ++g_gameSession.gameFlagsSeen;
    g_gameSession.difficulty = packet[1]; g_gameSession.gameFlags = le32(packet + 2);
    g_gameSession.expansion = packet[6]; g_gameSession.ladder = packet[7];
    session_event({ 0x01, packet[1], le32(packet + 2), packet[6], packet[7], {}, {} });
    return 1;
}
// 0x7c (0xad2a0, 0x81aa0): the unit's "in use" state (0x36) ends: its stat list is taken off and freed and the
// unit's animation rate is worked out again. Nothing happens to a unit without that state.
static int end_item_use(D2UnitStrc* unit) {
    if (!unit) return 0;
    D2StatListStrc* list = STATLIST_GetStatListFromUnitAndState(unit, 0x36);
    if (!list) return 0;
    STATES_ToggleState(unit, 0x36, 0);
    D2Common_10474(unit, list);
    STATLIST_FreeStatList(list);
    UNITS_UpdateAnimRateAndVelocity(unit, __FILE__, __LINE__);
    return 1;
}
static D2UnitStrc* session_unit(uint32_t type, uint32_t guid) {
    if (type == UNIT_ITEM) return d2_realm_client_owned_item_find(guid);
    return find_realm_client_unit(static_cast<uint8_t>(type), guid);
}
static int pkt_used_item_update(const uint8_t* packet, int) {
    ++g_gameSession.usedItemSeen;
    g_gameSession.usedItemType = packet[1]; g_gameSession.usedItemGuid = le32(packet + 2);
    const int ended = end_item_use(session_unit(packet[1], le32(packet + 2)));
    g_gameSession.usedItemEnded += uint32_t(ended);
    return ended;
}
// 0x3f (0xaf910, 0x86a40): the cursor takes up or lets go of an item it is using (a scroll or key being aimed).
// Letting go (kind 0xff with word 0xffff) clears the item's targeting flag and its "in use" state and puts the
// cursor back to rest. Taking up names the item, keeps the kind byte and arms the cursor; kinds 0 and 2 also
// open the inventory when it is shut, which the event asks of the page. The engine holds worn items only, so
// the flag and state are written to the item when it is one of those; the page's item records take the same
// packet for the pack.
static int pkt_use_item_cursor(const uint8_t* packet, int) {
    const uint32_t kind = packet[1], guid = le32(packet + 2), word = le16(packet + 6);
    D2UnitStrc* item = d2_realm_client_owned_item_find(guid);
    ++g_gameSession.useItemSeen;
    g_gameSession.useItemWord = word;
    if (word == 0xffff && kind == 0xff) {
        if (item) { ITEMS_SetItemFlag(item, IFLAG_TARGETING, 0); end_item_use(item); }
        g_gameSession.useItemGuid = 0; g_gameSession.useItemKind = 0;
        g_gameSession.useCursorMode = 5; g_gameSession.useCursorPhase = 1;
        session_event({ 0x3f, kind, guid, word, 0, {}, {} });
        return 1;
    }
    if (item && word == 0xffff) ITEMS_SetItemFlag(item, IFLAG_TARGETING, 1);
    g_gameSession.useItemGuid = guid; g_gameSession.useItemKind = kind;
    g_gameSession.useCursorMode = 6; g_gameSession.useCursorPhase = 6;
    session_event({ 0x3f, kind, guid, word, (kind == 0 || kind == 2) ? 1u : 0u, {}, {} });
    return 1;
}
// 0x42 (0xadb70, 0x82f50): when the unit is the local player, the item on its cursor is taken off the inventory
// and freed. The page's item records drop their cursor item on the same packet.
static int pkt_cursor_clear_hero(const uint8_t* packet, int) {
    D2UnitStrc* hero = session_hero();
    if (!hero || packet[1] != UNIT_PLAYER || le32(packet + 2) != hero->dwUnitId) return 0;
    ++g_gameSession.cursorClears;
    if (hero->pInventory && hero->pInventory->dwSignature == D2C_InventoryHeader) {
        if (D2UnitStrc* item = INVENTORY_GetCursorItem(hero->pInventory)) {
            INVENTORY_SetCursorItem(hero->pInventory, nullptr);
            d2_realm_client_owned_item_destroy(item);
            ++g_gameSession.cursorCleared;
        }
    }
    session_event({ 0x42, packet[1], le32(packet + 2), 0, 0, {}, {} });
    return 1;
}
// 0x5f (0xac050): a dword written to the local player's data record.
static int pkt_hero_dword(const uint8_t* packet, int) {
    ++g_gameSession.heroDwordSeen;
    g_gameSession.heroDword = le32(packet + 1);
    D2UnitStrc* hero = session_hero();
    if (hero && hero->pPlayerData) UNITS_SetPlayerPortalFlags(hero, static_cast<int>(le32(packet + 1)));
    return 1;
}
// 0x7e (0xad240): the shared animation data of the act the local player stands in is loaded again. This engine
// reads that data when an animation asks for it, so the act is noted and nothing is loaded here.
static int pkt_act_data(const uint8_t*, int) {
    ++g_gameSession.actDataSeen;
    g_gameSession.actDataAct = g_world.act;
    return 1;
}
// 0x8f (0xabf40): the pong. The eight dwords are kept; the round-trip time is the page's, which owns the clock.
// 0x9B (0xae1e0): the dead mercenary's name id and the price of raising it.
static int pkt_merc_resurrect(const uint8_t* packet, int size) { return d2::realm::mercResurrectApply(g_mercResurrect, packet, size); }
// 0x61 (0xaf940): a film to play; the shell takes the indexes through d2_realm_client_take_movie.
static int pkt_film(const uint8_t* packet, int size) { return d2::realm::movieQueueApply(g_movieQueue, packet, size); }

static int pkt_pong(const uint8_t* packet, int) {
    for (int i = 0; i < 8; ++i) g_pong[i] = le32(packet + 1 + 4 * i);
    ++g_pongCount;
    return 0;
}
// 0x00 and the other rows of the no-op stub (0xac520) do nothing. 0x02 (0xac4f0) only answers with the enter
// request, which the join sends.
static int pkt_nothing(const uint8_t*, int) { return 0; }

EMSCRIPTEN_KEEPALIVE int d2_realm_client_roster_count() { return static_cast<int>(g_roster.size()); }
EMSCRIPTEN_KEEPALIVE const D2RealmRosterView* d2_realm_client_roster() {
    g_rosterViews.clear();
    for (const auto& entry : g_roster) g_rosterViews.push_back(entry.view);
    return g_rosterViews.data();
}
EMSCRIPTEN_KEEPALIVE uint32_t d2_realm_client_roster_revision() { return g_rosterRevision; }
EMSCRIPTEN_KEEPALIVE int d2_realm_client_portal_count() { return static_cast<int>(g_portals.size()); }
EMSCRIPTEN_KEEPALIVE const D2RealmPortalView* d2_realm_client_portals() { return g_portals.data(); }
EMSCRIPTEN_KEEPALIVE int d2_realm_client_session_event_count() { return static_cast<int>(g_sessionEvents.size()); }
EMSCRIPTEN_KEEPALIVE const D2RealmSessionEvent* d2_realm_client_session_events() { return g_sessionEvents.data(); }
EMSCRIPTEN_KEEPALIVE void d2_realm_client_session_events_clear() { g_sessionEvents.clear(); g_sessionEventText.clear(); }
EMSCRIPTEN_KEEPALIVE const char* d2_realm_client_session_event_text(int index) {
    return index >= 0 && size_t(index) < g_sessionEventText.size() ? g_sessionEventText[size_t(index)].c_str() : "";
}
EMSCRIPTEN_KEEPALIVE const D2RealmGameSessionView* d2_realm_client_game_session() { return &g_gameSession; }
EMSCRIPTEN_KEEPALIVE uint32_t d2_realm_client_session_events_dropped() { return g_sessionEventsDropped; }
EMSCRIPTEN_KEEPALIVE uint32_t d2_realm_client_pong_count() { return g_pongCount; }
EMSCRIPTEN_KEEPALIVE const uint32_t* d2_realm_client_pong() { return g_pong; }

// Keyed by the retail handler each body stands for: the row's queued handler when it has one, else its
// arrival handler. A queued body runs on arrival here; this client keeps no per-unit packet queue.
struct RealmPacketHandler { uint32_t handler; RealmPacketBody body; };
// 0x42 (D2Client+0xADB70): the unit the packet names loses its cursor item.
static int pkt_cursor_clear(const uint8_t* packet, int size) {
    if (size != 6) return -2;
    D2UnitStrc* unit = find_realm_client_unit(packet[1], le32(packet + 2));
    return unit ? d2_realm_client_item_clear_cursor(unit) : 0;
}
// 0x73 (D2Client+0xAEEE0): a missile the server ordered. The local player is the creating unit and the maker is the
// general one, so the row names its class itself and no skill gates it; owner, pierce and current frame follow creation.
static int pkt_missile_ordered(const uint8_t* packet, int size) {
    D2RealmMissilePacket in{};
    if (!d2_realm_parse_missile_packet(packet, size, in)) return -2;
    D2UnitStrc* local = selected_player_unit();
    if (!local) return 0;
    D2MissileStrc params{};
    params.dwFlags = in.flags;
    params.pOwner = local;
    params.nMissile = static_cast<int32_t>(in.missileId);
    params.nX = in.x; params.nY = in.y;
    params.nTargetX = in.targetX; params.nTargetY = in.targetY;
    params.nSkillLevel = static_cast<int32_t>(in.level);
    D2UnitStrc* missile = create_client_missile(&params, true);
    if (!missile) return 0;
    if (D2UnitStrc* owner = find_realm_client_unit(static_cast<uint8_t>(in.ownerType), in.ownerGuid)) MISSILE_SetOwner(missile, owner);
    if (in.pierce && missile->pStatListEx) STATLIST_SetUnitStat(missile, 0x148, static_cast<int>(in.pierce), 0);
    MISSILE_SetCurrentFrame(missile, in.frame);
    return 1;
}

static const RealmPacketHandler kRealmPacketHandlers[] = {
    { 0xafa90, pkt_both<apply_game_state_packet, pkt_game_flags> },   // 0x01
    { 0xadb50, pkt_load_act },                                  // 0x03
    { 0xadfe0, pkt_load_complete },                             // 0x04
    { 0xacb70, pkt_unload_complete },                           // 0x05
    { 0xac440, pkt_map_reveal },                                // 0x07
    { 0xac3d0, pkt_map_hide },                                  // 0x08
    { 0xadf70, pkt_tile_assign },                               // 0x09
    { 0xaca40, pkt_remove_unit },                               // 0x0a
    { 0xad6e0, pkt_select_player },                             // 0x0b
    { 0xaf5f0, pkt_unit_request },                              // 0x0c
    { 0xaf590, pkt_or<apply_player_stop, -2> },                 // 0x0d
    { 0xaf560, pkt_or<apply_unit_queue_packet, -2> },           // 0x0e
    { 0xaf510, pkt_unit_request },                              // 0x0f
    { 0xaf4c0, pkt_unit_request },                              // 0x10
    { 0xadaa0, pkt_unit_overlay },                              // 0x11
    { 0xaf640, pkt_or<reassign_player, -2> },                   // 0x15
    { 0xaf240, pkt_or<apply_unit_queue_packet, -2> },           // 0x17
    { 0xaeca0, pkt_or<apply_life_mana, -2> },                   // 0x18
    { 0xaedf0, pkt_or<apply_player_stat, -2> },                 // 0x19..0x1f
    { 0xaed80, pkt_or<apply_player_stat, -2> },                 // 0x20
    { 0xac910, pkt_or<apply_player_skill_update, -2> },         // 0x21
    { 0xac800, pkt_or<apply_player_skill_update, -2> },         // 0x22
    { 0xac780, pkt_or<apply_player_skills, -2> },               // 0x23
    { 0xaf9f0, pkt_game_chat },                                 // 0x26
    { 0xafa70, apply_game_state_packet },                       // 0x28
    { 0xac330, pkt_quest_game_record },                         // 0x29
    { 0xad740, pkt_unit_sound },                                // 0x2c
    { 0xaf1d0, pkt_or<apply_skill_start_packet, -2> },          // 0x4c
    { 0xaf160, pkt_skill_start_at },                            // 0x4d
    { 0xadf40, pkt_object_assign },                             // 0x51
    { 0xad520, pkt_quest_log_states },                          // 0x52
    { 0xae9c0, pkt_environment },                               // 0x53
    { 0xadf20, pkt_or<assign_player, -3> },                     // 0x59
    { 0xaf8f0, pkt_quest_state },                               // 0x5d
    { 0xac090, pkt_quest_available },                           // 0x5e
    { 0xaf460, pkt_unit_request },                              // 0x67
    { 0xaf3f0, pkt_unit_request },                              // 0x68
    { 0xaf390, pkt_unit_request },                              // 0x69
    { 0xaf340, pkt_unit_request },                              // 0x6a
    { 0xaf7c0, pkt_unit_request },                              // 0x6b
    { 0xaf750, pkt_unit_request },                              // 0x6c
    { 0xaf2b0, pkt_unit_request },                              // 0x6d
    { 0xadef0, pkt_both<pkt_or<apply_corpse_drop, -2>, pkt_corpse_assign> },   // 0x74
    { 0xac720, pkt_pet_owner },                                 // 0x7a
    { 0xad820, apply_game_state_packet },                       // 0x7f
    { 0xaeba0, pkt_pet_assign },                                // 0x81
    { 0xadb30, pkt_both<apply_game_state_packet, pkt_npc_wants_talk> },   // 0x8a
    { 0xad130, pkt_both<pkt_or<apply_corpse_assign, -2>, pkt_roster_corpse> },   // 0x8e
    { 0xac880, pkt_or<apply_player_skills, -2> },               // 0x94
    { 0xaebe0, pkt_or<apply_life_mana, -2> },                   // 0x95
    { 0xae080, pkt_or<apply_item_record, -4> },                 // 0x9c
    { 0xae870, pkt_or<apply_item_record, -4> },                 // 0x9d
    { 0xaea40, apply_game_state_packet },                       // 0x9e..0xa2
    { 0xae1e0, pkt_merc_resurrect },                            // 0x9b
    { 0xaf940, pkt_film },                                      // 0x61
    { 0xadeb0, pkt_or<apply_state_packet, -4> },                // 0xa7
    { 0xadd70, pkt_or<apply_state_packet, -4> },                // 0xa8
    { 0xadd30, pkt_or<apply_state_packet, -4> },                // 0xa9
    { 0xadb90, pkt_or<apply_state_packet, -4> },                // 0xaa
    { 0xae420, pkt_monster_assign },                            // 0xac
    { 0xac520, pkt_nothing },                                   // 0x00 and the other stub rows
    { 0xac4f0, pkt_nothing },                                   // 0x02
    { 0xae2b0, pkt_equip_recheck },                             // 0x47
    { 0xae260, pkt_equip_recheck },                             // 0x48
    { 0xad6b0, pkt_event_message },                             // 0x5a
    { 0xad8f0, pkt_roster_add },                                // 0x5b
    { 0xad2f0, pkt_portal_state },                              // 0x60
    { 0xad890, pkt_roster_kills },                              // 0x65
    { 0xad870, pkt_roster_about },                              // 0x75
    { 0xad370, pkt_overhead_clear },                            // 0x76
    { 0xad160, pkt_portal_owner },                              // 0x82
    { 0xad780, pkt_roster_party },                              // 0x8d
    { 0xabf40, pkt_pong },                                      // 0x8f
    { 0xad950, pkt_walk_verify },                               // 0x96
    { 0xaf910, pkt_use_item_cursor },                           // 0x3f
    { 0xadb70, pkt_both<pkt_cursor_clear_hero, pkt_cursor_clear> },                              // 0x42
    { 0xaeee0, pkt_missile_ordered },                           // 0x73
    { 0xac050, pkt_hero_dword },                                // 0x5f
    { 0xad2a0, pkt_used_item_update },                          // 0x7c
    { 0xad240, pkt_act_data },                                  // 0x7e
    { 0xad7b0, pkt_roster_relation },                           // 0x8c
};

static RealmPacketBody realm_packet_body(uint8_t opcode) {
    if (opcode >= d2::client::kServerPacketCount) return nullptr;
    const auto& row = d2::client::kServerPacketTable[opcode];
    const uint32_t handler = row.queued ? row.queued : row.handler;
    for (const auto& entry : kRealmPacketHandlers) if (entry.handler == handler) return entry.body;
    return nullptr;
}

// The dispatch loop's unit lookup for a queued row: a monster by the id at +1 for 0x67..0x6D, else the unit
// of the type at +1 and the id at +2. True when the unit is held and the packet was stored on it.
static bool queue_unit_packet(const uint8_t* packet, uint32_t size) {
    const bool monsterRow = packet[0] >= 0x67 && packet[0] <= 0x6d;
    if (size < (monsterRow ? 5u : 6u)) return false;
    const uint8_t unitType = monsterRow ? static_cast<uint8_t>(UNIT_MONSTER) : packet[1];
    uint32_t guid = 0;
    std::memcpy(&guid, packet + (monsterRow ? 1 : 2), sizeof guid);
    bool held = false;
    if (unitType == UNIT_PLAYER) {
        for (const auto& player : g_players) if (player.view.active && player.view.guid == guid) { held = true; break; }
    } else if (unitType == UNIT_MONSTER) {
        for (const auto& monster : g_monsters) if (monster.view.guid == guid) { held = true; break; }
    } else if (unitType == UNIT_OBJECT) {
        held = d2gs_client_object_find(guid) != nullptr;
    }
    if (!held) return false;
    g_unitQueue.push_back({ unitType, guid, std::vector<uint8_t>(packet, packet + size) });
    return true;
}
// One unit's stored packets, in arrival order, through the queued handlers' bodies; the queue is then empty.
// The handlers return nothing: a body's refusal changes nothing else.
static int run_unit_queue(uint8_t unitType, uint32_t guid) {
    int ran = 0;
    for (size_t i = 0; i < g_unitQueue.size();) {
        if (g_unitQueue[i].unitType != unitType || g_unitQueue[i].guid != guid) { ++i; continue; }
        const std::vector<uint8_t> bytes = std::move(g_unitQueue[i].bytes);
        g_unitQueue.erase(g_unitQueue.begin() + static_cast<std::ptrdiff_t>(i));
        if (RealmPacketBody body = realm_packet_body(bytes[0])) body(bytes.data(), static_cast<int>(bytes.size()));
        ++ran;
    }
    return ran;
}

// What the dispatch loop sees of the client module: the packet table, the packet under dispatch and the
// loop's own counters. The unit tables read as empty, so the loop queues nothing and the body does the work.
namespace {
constexpr uint32_t kDispatchModule = 0x6fab0000u, kDispatchPacket = 0x10000000u;
constexpr uint32_t kDispatchTable = 0xdde60, kDispatchBytes = 0x11c4dc, kDispatchPasses = 0x11c4e4, kDispatchBig = 0x11c4ec, kDispatchLast = 0x123414;
struct RealmDispatch {
    const uint8_t* packet;
    uint32_t size;
    int applied;
    int rejected;       // a body's negative code
    uint32_t fatal;     // the loop's own assertion line: opcode past the table, or a size the table disagrees with
};
uint32_t g_dispatchBytes, g_dispatchPasses, g_dispatchBig, g_dispatchLast;
uint32_t g_feedStall;   // why the last feed stopped short: 0, or (reason << 8) | opcode

uint32_t* dispatch_counter(uint32_t addr) {
    switch (addr - kDispatchModule) {
    case kDispatchBytes: return &g_dispatchBytes;
    case kDispatchPasses: return &g_dispatchPasses;
    case kDispatchBig: return &g_dispatchBig;
    case kDispatchLast: return &g_dispatchLast;
    default: return nullptr;
    }
}
uint32_t dispatch_read(void* ctx, uint32_t addr) {
    const auto& d = *static_cast<const RealmDispatch*>(ctx);
    if (addr - kDispatchPacket < d.size) {
        uint32_t value = 0;
        for (uint32_t i = 0, at = addr - kDispatchPacket; i < 4 && at + i < d.size; ++i) value |= static_cast<uint32_t>(d.packet[at + i]) << (8 * i);
        return value;
    }
    const uint32_t entry = addr - (kDispatchModule + kDispatchTable);
    if (entry < d2::client::kServerPacketCount * 12) {
        const auto& row = d2::client::kServerPacketTable[entry / 12];
        switch (entry % 12) {
        case 0: return row.handler ? kDispatchModule + row.handler : 0;
        case 4: return static_cast<uint32_t>(row.size);
        case 8: return row.queued ? kDispatchModule + row.queued : 0;
        default: return 0;
        }
    }
    if (const uint32_t* counter = dispatch_counter(addr)) return *counter;
    return 0;
}
void dispatch_write(void*, uint32_t addr, uint32_t value) {
    if (uint32_t* counter = dispatch_counter(addr)) *counter = value;
}
uint32_t dispatch_call(void* ctx, uint32_t rva, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
                       uint32_t s0, uint32_t, uint32_t, uint32_t, uint32_t) {
    auto& d = *static_cast<RealmDispatch*>(ctx);
    switch (rva) {
    case 0xd246: {   // the packet size
        if (d.rejected || d.fatal) return 0;
        uint32_t size = 0;
        const uint32_t result = d2_realm_d2net_6750(d.packet, d.size, &size);
        *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(s0)) = size;
        return result;
    }
    case 0xbf92: d.fatal = s0; return 0;                       // the assertion line
    case 0xbf80: case 0x3664: return 0;                        // report and exit: the feed stops instead
    case 0xa4e20: case 0xac630: return 0;                      // unit lookup and per-unit queue: none kept
    case 0xac240: case 0xac1b0: case 0xac140: return 0;        // life/mana unpackers: the bodies decode
    default: break;
    }
    if (rva < kDispatchModule) return 0;
    if (d2::client::kServerPacketTable[d.packet[0]].queued) {   // the arrival handler of a queued row is the no-op stub
        if (queue_unit_packet(d.packet, d.size)) d.applied += 1;
        return 0;
    }
    if (RealmPacketBody body = realm_packet_body(d.packet[0])) {
        const int result = body(d.packet, static_cast<int>(d.size));
        if (result < 0) d.rejected = result; else d.applied += result;
    }
    return 0;
}
}  // namespace

// A packet the stream cannot frame (an opcode the size table does not know, or a size over the receive
// node) stops the feed where retail's splitter stops: the bytes stay pending, nothing after them is
// dispatched and the state built so far is kept. d2_realm_client_feed_stall names the opcode.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_player_feed(const uint8_t* bytes, int length) {
    if ((!bytes && length) || length < 0 || length > kPendingCapacity - g_pendingLength) return reject_stream(-1);
    if (length) {
        std::memcpy(g_pending + g_pendingLength, bytes, static_cast<size_t>(length));
        g_pendingLength += length;
    }
    const cl::Env env{ nullptr, dispatch_call, dispatch_read, dispatch_write, 0, 0 };
    int offset = 0;
    int applied = 0;
    g_feedStall = 0;
    while (offset < g_pendingLength) {
        const uint8_t* packet = g_pending + offset;
        const uint32_t available = static_cast<uint32_t>(g_pendingLength - offset);
        uint32_t entry = 0;
        const uint32_t size = d2_realm_d2net_6750(packet, available, &entry);
        if (size == 0) {   // more bytes needed, or an opcode with no size
            if (entry != 0xFFFFFFFFu) g_feedStall = 0x100u | packet[0];
            break;
        }
        if (size > 0x204) { g_feedStall = 0x200u | packet[0]; break; }
        if (size > available) break;
        if (packet[0] < d2::client::kServerPacketCount) {   // 0xaf and above are the transport's own
            RealmDispatch dispatch{ packet, size, 0, 0, 0 };
            cl::Env bound = env;
            bound.ctx = &dispatch;
            d2c_ace20(bound, kDispatchModule, kDispatchPacket, static_cast<int32_t>(size));
            if (dispatch.rejected) return reject_stream(dispatch.rejected);
            if (dispatch.fatal) { g_feedStall = 0x300u | packet[0]; break; }
            applied += dispatch.applied;
        }
        offset += static_cast<int>(size);
    }
    const int remaining = g_pendingLength - offset;
    if (offset && remaining) std::memmove(g_pending, g_pending + offset, static_cast<size_t>(remaining));
    g_pendingLength = remaining;
    return applied;
}

EMSCRIPTEN_KEEPALIVE uint32_t d2_realm_client_feed_stall() { return g_feedStall; }


EMSCRIPTEN_KEEPALIVE int d2_realm_client_player_pending() { return g_pendingLength; }

struct RealmClientMissileView {
    uint32_t version, size, id, classId, mode, framePrecise, frameCountPrecise, animSpeed;
    uint32_t direction, trans, preciseX, preciseY, level, ownerGuid, skill, remaining;
    uint32_t initSeed, seedLow, seedHigh;
};
static_assert(sizeof(RealmClientMissileView) == 76);
static std::vector<RealmClientMissileView> g_clientMissileViews;
EMSCRIPTEN_KEEPALIVE int d2_realm_client_missile_event_count() { return static_cast<int>(g_clientMissileEvents.size()); }
EMSCRIPTEN_KEEPALIVE const RealmMissileEvent* d2_realm_client_missile_events() { return g_clientMissileEvents.data(); }
// Consumes class-62 terminal hit/expiry sound rows only. Record words are
// {version,tick,missileId,classId,event,soundRow}; other missile sounds remain unsupported.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_take_missile_sounds(int32_t* out, int cap) {
    if (!out || cap <= 0) return 0;
    const int count = std::min(cap, static_cast<int>(g_clientMissileSoundEvents.size()));
    for (int i = 0; i < count; ++i) {
        const auto& event = g_clientMissileSoundEvents[static_cast<size_t>(i)];
        const int offset = i * 6;
        out[offset] = static_cast<int32_t>(event.version);
        out[offset + 1] = static_cast<int32_t>(event.tick);
        out[offset + 2] = static_cast<int32_t>(event.missileId);
        out[offset + 3] = static_cast<int32_t>(event.classId);
        out[offset + 4] = static_cast<int32_t>(event.event);
        out[offset + 5] = static_cast<int32_t>(event.soundRow);
    }
    g_clientMissileSoundEvents.erase(g_clientMissileSoundEvents.begin(),
        g_clientMissileSoundEvents.begin() + count);
    return count;
}
EMSCRIPTEN_KEEPALIVE int d2_realm_client_missile_count() { return static_cast<int>(g_clientMissiles.size()); }
EMSCRIPTEN_KEEPALIVE const RealmClientMissileView* d2_realm_client_missiles() {
    g_clientMissileViews.clear();
    for (const auto& entry : g_clientMissiles) {
        const auto* unit = entry.unit;
        auto* path = unit->pDynamicPath;
        const auto* txt = DATATBLS_GetMissilesTxtRecord(unit->dwClassId);
        g_clientMissileViews.push_back({1, static_cast<uint32_t>(sizeof(RealmClientMissileView)), unit->dwUnitId,
            static_cast<uint32_t>(unit->dwClassId), unit->dwAnimMode, static_cast<uint32_t>(unit->nSeqCurrentFramePrecise),
            static_cast<uint32_t>(unit->dwFrameCountPrecise), static_cast<uint32_t>(unit->wAnimSpeed), static_cast<uint32_t>(PATH_GetDirection(path)),
            txt ? txt->nTrans : 0u, static_cast<uint32_t>(PATH_GetPrecisionX(path)),
            static_cast<uint32_t>(PATH_GetPrecisionY(path)), entry.level,
            unit->pMissileData->dwOwnerGUID, static_cast<uint32_t>(MISSILE_GetSkill(entry.unit)),
            static_cast<uint32_t>(MISSILE_GetCurrentFrame(entry.unit)), unit->dwInitSeed,
            unit->pSeed.nLowSeed, unit->pSeed.nHighSeed});
    }
    return g_clientMissileViews.data();
}

EMSCRIPTEN_KEEPALIVE int d2_realm_client_monster_count() {
    return static_cast<int>(g_monsters.size());
}

// Narrow readback for native D2GS state-packet verification. State data remains on the same
// canonical CUnit used by rendering; this does not add a parallel state projection or widen its ABI.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_unit_state(uint8_t unitType, uint32_t guid, int state) {
    D2UnitStrc* unit = find_realm_client_unit(unitType, guid);
    if (!unit || state < 0 || !sgptDataTables || state >= sgptDataTables->nStatesTxtRecordCount) return -1;
    return STATES_CheckState(unit, state) ? 1 : 0;
}

// The unit's path position at 16.16 precision, as the single-player renderer reads it.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_unit_precise(uint8_t unitType, uint32_t guid, uint32_t* out) {
    D2UnitStrc* unit = find_realm_client_unit(unitType, guid);
    if (!out || !unit || !unit->pDynamicPath) return 0;
    out[0] = static_cast<uint32_t>(PATH_GetPrecisionX(unit->pDynamicPath));
    out[1] = static_cast<uint32_t>(PATH_GetPrecisionY(unit->pDynamicPath));
    return 1;
}

EMSCRIPTEN_KEEPALIVE int d2_realm_client_unit_state_stat(uint8_t unitType, uint32_t guid,
        int state, int statId, uint16_t layer) {
    D2UnitStrc* unit = find_realm_client_unit(unitType, guid);
    if (!unit || state < 0 || statId < 0 || !sgptDataTables ||
        state >= sgptDataTables->nStatesTxtRecordCount || statId >= sgptDataTables->nItemStatCostTxtRecordCount)
        return 0;
    D2StatListStrc* list = STATLIST_GetStatListFromUnitAndState(unit, state);
    return list ? STATLIST_GetStatValue(list, statId, layer) : 0;
}

// What the client knows of one unit beyond its view: whether the hero may attack it, whose pet it is, and the
// name and second line its plate shows. The attack answer is given three ways because the page holds the
// roster: `selectable` where the hero stands with no player hostile, and the answer outside town with the
// roster's relation toward `rosterGuid` friendly (`enemy`) and hostile (`enemyIfHostile`).
struct D2RealmClientUnitInfo {
    uint32_t version;
    uint32_t size;
    uint32_t unitType;
    uint32_t guid;
    uint32_t selectable;
    uint32_t enemy;
    uint32_t enemyIfHostile;
    uint32_t rosterGuid;
    int32_t alignment;
    uint32_t isPet;
    uint32_t petType;
    uint32_t petOwner;
    uint32_t petDead;
    uint32_t petNameId;
    uint32_t typeFlags;
    int32_t superUnique;
    uint32_t nameSeed;
    uint32_t ownerGuid;
    uint8_t umods[12];
    char name[128];
    char line[256];
};
static_assert(offsetof(D2RealmClientUnitInfo, umods) == 72);
static_assert(offsetof(D2RealmClientUnitInfo, name) == 84);
static_assert(offsetof(D2RealmClientUnitInfo, line) == 212);
static_assert(sizeof(D2RealmClientUnitInfo) == 468);

static uint32_t monstats2_flag(uint32_t classId, uint32_t bit) {
    const D2MonStatsTxt* row = DATATBLS_GetMonStatsTxtRecord(static_cast<int>(classId));
    if (!row || !sgptDataTables->pMonStats2Txt) return 0;
    const int ex = static_cast<int16_t>(row->wMonStatsEx);
    if (ex < 0 || ex >= sgptDataTables->nMonStats2TxtRecordCount) return 0;
    return (sgptDataTables->pMonStats2Txt[ex].nFlags[(bit >> 3) & 3] >> (bit & 7)) & 1u;
}

EMSCRIPTEN_KEEPALIVE const D2RealmClientUnitInfo* d2_realm_client_unit_info(uint8_t unitType, uint32_t guid) {
    static D2RealmClientUnitInfo info;
    if (!sgptDataTables) return nullptr;
    d2::realm::RelationUnit target{}, hero{};
    D2UnitStrc* targetUnit = nullptr;
    const D2RealmClientMonster* monster = nullptr;
    if (unitType == UNIT_MONSTER) {
        for (const auto& candidate : g_monsters) if (candidate.view.guid == guid) { monster = &candidate; break; }
        if (!monster) return nullptr;
        targetUnit = monster->motionUnit;
        target = { true, UNIT_MONSTER, monster->view.classId, guid, monster->view.mode, targetUnit ? targetUnit->dwFlags : 0 };
    } else if (unitType == UNIT_PLAYER) {
        targetUnit = find_realm_client_unit(UNIT_PLAYER, guid);
        if (!targetUnit) return nullptr;
        target = { true, UNIT_PLAYER, static_cast<uint32_t>(targetUnit->dwClassId), guid, targetUnit->dwAnimMode, targetUnit->dwFlags };
    } else return nullptr;
    D2UnitStrc* heroUnit = nullptr;
    if (g_selectedPlayer >= 0 && static_cast<size_t>(g_selectedPlayer) < g_players.size())
        heroUnit = g_players[g_selectedPlayer].motionUnit;
    if (heroUnit)
        hero = { true, UNIT_PLAYER, static_cast<uint32_t>(heroUnit->dwClassId), selected_player_guid(), heroUnit->dwAnimMode, heroUnit->dwFlags };

    info = {};
    info.version = 1;
    info.size = sizeof(info);
    info.unitType = unitType;
    info.guid = guid;
    d2::realm::RelationFacts facts{};
    facts.aligned = heroUnit && targetUnit && STATLIST_AreUnitsAligned(heroUnit, targetUnit);
    facts.monStats2Flag = monstats2_flag;
    D2ActiveRoomStrc* room = selected_player_room();
    facts.inTown = room && DUNGEON_IsRoomInTown(room);
    info.selectable = d2::realm::clientUnitRelation(target, hero, facts);
    facts.inTown = false;
    info.enemy = d2::realm::clientUnitRelation(target, hero, facts);
    facts.rosterHostile = true;
    info.enemyIfHostile = d2::realm::clientUnitRelation(target, hero, facts);
    info.rosterGuid = d2::realm::clientRelationRosterGuid(target);
    info.alignment = targetUnit ? STATLIST_GetUnitAlignment(targetUnit) : 0;
    if (unitType != UNIT_MONSTER) return &info;

    if (const d2::realm::ClientPet* pet = d2::realm::clientPetFind(guid)) {
        info.isPet = 1;
        info.petType = pet->type;
        info.petOwner = pet->owner;
        info.petDead = pet->dead;
        info.petNameId = pet->nameId;
    }
    info.typeFlags = monster->view.typeFlags;
    info.superUnique = monster->superUniqueIndex;
    info.nameSeed = monster->nameId;
    info.ownerGuid = monster->ownerGuid;
    const size_t umodCount = std::min<size_t>(monster->nameBytes.size(), 9);
    for (size_t i = 0; i < umodCount; ++i) info.umods[i] = monster->nameBytes[i];
    D2UnitStrc classUnit{};
    classUnit.dwUnitType = UNIT_MONSTER;
    classUnit.dwClassId = static_cast<int>(monster->view.classId);
    d2::realm::MonsterNameInput in{};
    in.unit = &classUnit;
    in.classId = monster->view.classId;
    in.typeFlags = monster->view.typeFlags;
    in.nameSeed = monster->nameId;
    in.superUnique = monster->superUniqueIndex;
    in.umods = info.umods;
    in.umodCount = static_cast<uint32_t>(umodCount);
    in.hirelingNameId = d2::realm::clientPetHirelingNameId(guid);
    const std::string name = d2::realm::clientUtf8(d2::realm::clientMonsterName(in));
    const std::string line = d2::realm::clientUtf8(d2::realm::clientMonsterEnchantLine(in));
    std::strncpy(info.name, name.c_str(), sizeof(info.name) - 1);
    std::strncpy(info.line, line.c_str(), sizeof(info.line) - 1);
    return &info;
}

// Focused lifecycle readback for the canonical GUID roster; zero means no matching live CUnit.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_unit_resources_readback(uint8_t unitType, uint32_t guid) {
    return d2_realm_client_unit_resources_status(find_realm_client_unit(unitType, guid));
}

// Read one equipped slot from the canonical server-owned player's real inventory. This is a
// bounded evidence interface; it does not synthesize an inventory item in JS.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_owned_item_readback_by_guid(uint32_t ownerGuid,
        int bodyLoc, D2RealmOwnedItemView* out) {
    if (!out) return 0;
    D2UnitStrc* owner = find_realm_client_unit(UNIT_PLAYER, ownerGuid);
    return d2_realm_client_owned_item_readback(owner, bodyLoc, out);
}

#include "../monspawn/d2client_light_in.h"
// The online client's light map: the same window, ambient, occlusion and falloff as the session's
// (d2_client_lightmap_compose), fed from the units this client holds -- the selected player, its monsters'
// CUnits and its objects (Objects.txt Lit<mode> is a diameter, position in subtiles *8 + 4).
// Objects settled: the radius (nLit[mode] >> 1) and position are the rule the session lightmap uses
// (monspawn_check.cpp, measured against retail's light captures). Monsters take the light setter's rule in the
// compose step (max of MonStats2 nLight and the item light, at most 18; on Den of Evil a radius of 3 while the
// Den quest byte is set). The byte is g_QuestAvail[1], the second byte after the opcode of server packet 0x5E (D2Client
// +0xAC090), fed to d2_client_lightmap_set_den_quest_avail when that packet arrives.
// Returns the session lightmap record (same layout as d2_session_lightmap), or null without a player/act.
EMSCRIPTEN_KEEPALIVE const void* d2_realm_client_lightmap(int ambientOverride, int occludeUnits) {
    D2DrlgActStrc* act = d2_drlg_client_act();
    if (!act || g_selectedPlayer < 0 || static_cast<size_t>(g_selectedPlayer) >= g_players.size()) return nullptr;
    D2UnitStrc* player = g_players[g_selectedPlayer].motionUnit;
    if (!player) return nullptr;
    static std::vector<D2ClientLightIn> in;
    in.clear();
    for (const auto& monster : g_monsters)
        if (monster.motionUnit) in.push_back({ monster.motionUnit, UNIT_MONSTER, static_cast<int32_t>(monster.view.classId), 0, 0, 0, monster.view.guid, nullptr });
    for (const auto& object : g_objects) {
        if (object.unitType != UNIT_OBJECT) continue;
        // The record's mode follows the unit, so a torch the server lights or puts out changes its radius; the
        // light is anchored in the object's own room, as the local game's is.
        const D2ObjectsTxt* txt = DATATBLS_GetObjectsTxtRecord(static_cast<int>(object.classId));
        const int radius = txt && object.mode < 8 ? txt->nLit[object.mode] >> 1 : 0;
        D2UnitStrc* unit = find_object_unit(object.guid);
        in.push_back({ nullptr, UNIT_OBJECT, static_cast<int32_t>(object.classId), static_cast<int32_t>(object.x) * 8 + 4,
            static_cast<int32_t>(object.y) * 8 + 4, radius, object.guid, unit ? UNITS_GetRoom(unit) : nullptr });
    }
    return d2_client_lightmap_compose(player, act, in.data(), static_cast<int>(in.size()), ambientOverride, occludeUnits);
}

// Current day/night state of the client act: period, intensity, ticks, rgb; 0 without an act.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_period_of_day(int32_t* out) {
    D2DrlgActStrc* act = d2_drlg_client_act();
    D2DrlgEnvironmentStrc* env = act ? DUNGEON_GetEnvironmentFromAct(act) : nullptr;
    if (!env || !out) return 0;
    out[0] = env->nPeriodOfDay; out[1] = env->nIntensity; out[2] = env->nTicks;
    out[3] = env->nRed | (env->nGreen << 8) | (env->nBlue << 16);
    return 1;
}

// Advance path-owned monster CUnits once for each shell game frame. Renderer getters remain pure.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_motion_tick() {
    ++g_clientMissileTick;
    int moving = 0;
    if (D2DrlgActStrc* act = d2_drlg_client_act()) ENVIRONMENT_UpdatePeriodOfDay(act, selected_player_room());
    advance_client_missiles();
    if (D2UnitStrc* own = selected_player_unit()) local_queue_step(own, g_localRun);
    for (size_t i = 0; i < g_players.size(); ++i) {
        auto& player = g_players[i];
        if (!player.motionUnit || !player.motionUnit->pDynamicPath) continue;
        D2UnitStrc* const unit = player.motionUnit;
        const uint32_t modeBefore = unit->dwAnimMode;
        const bool walking = modeBefore == PLRMODE_WALK || modeBefore == PLRMODE_RUN || modeBefore == PLRMODE_TOWNWALK;
        const bool own = static_cast<int>(i) == g_selectedPlayer;
        // The handler steps the path only in a mode whose table row is a moving one; a standing unit keeps the
        // points of a route its action interrupted and does not follow them.
        const bool pathSteps = unit_mode_move_kind(unit) == 1;   // the mode's table row, knockback included
        const int result = advance_client_player_action(unit) ? 0 : d2_realm_client_motion_advance_unit(unit, pathSteps);
        if (result > 0) moving++;
        // The per-frame unit handler (0x1c150): a walking unit whose path has ended this frame stands where the
        // path left it and takes the neutral mode of its room, flag 0x40 off and flag 2 on. The recordings show
        // the same frame carrying both the last position and the neutral mode. The own player with an action
        // waiting keeps its mode instead and only sets the action's fire word.
        if (walking && result == 0 && unit->dwAnimMode == modeBefore) {
            if (own && g_localQueued.kind) g_localQueued.fire = 1;
            else {
                unit->dwFlags = (unit->dwFlags & ~0x40u) | 2u;
                D2ActiveRoomStrc* room = UNITS_GetRoom(unit);
                UNITS_ChangeAnimMode(unit, room && DUNGEON_IsRoomInTown(room) ? PLRMODE_TOWNNEUTRAL : PLRMODE_NEUTRAL);
            }
        }
        // The handler's last step for the own player still running (0x19d80): out of stamina the run becomes
        // a walk, to the path's target unit when it has one, else to its target spot.
        // GUESS (stamina): the handler's test is a unit stat 10 read through a D2Common thunk, taken as the
        // stamina stat; it is applied only when the client holds a maximum for it.
        if (own && unit->dwAnimMode == PLRMODE_RUN && STATLIST_UnitGetStatValue(unit, STAT_MAXSTAMINA, 0) > 0 &&
                STATLIST_UnitGetStatValue(unit, STAT_STAMINA, 0) == 0) {
            D2DynamicPathStrc* path = unit->pDynamicPath;
            if (D2UnitStrc* target = PATH_GetTargetUnit(path)) local_move_to_unit_action(unit, target, false);
            else d2_realm_client_motion_apply_player_move(unit, 0x01, 0, static_cast<uint16_t>(path->tTargetCoord.X),
                static_cast<uint16_t>(path->tTargetCoord.Y));
        }
        if (run_unit_queue(UNIT_PLAYER, player.view.guid) || result > 0 || walking)
            refresh_player_motion_view(player, static_cast<int>(i) == g_selectedPlayer);
    }
    for (D2UnitStrc* object = d2gs_client_object_next(nullptr); object; object = d2gs_client_object_next(object)) {
        const uint32_t mode = object->dwAnimMode;
        d2gs_client_object_step(object);
        run_unit_queue(UNIT_OBJECT, object->dwUnitId);
        if (object->dwAnimMode != mode) sync_object_view(object);
    }
    for (auto& monster : g_monsters) {
        if (!monster.motionUnit || !monster.motionUnit->pDynamicPath) continue;
        // GUESS: a monster whose skill sequence has run out returns to neutral here; retail's
        // per-mode monster update (not read) owns that end. Settled by decompiling the Sequence
        // arm of the client's monster mode table.
        // A sequence-mode class with no MonSeq record has no frames (count 0): it ends at once, as retail's
        // 0xAC mode-14 capture shows (mode 14 on arrival, neutral on the first tick), and the frame step's
        // loop never runs on a zero count.
        if (monster.motionUnit->dwAnimMode == MONMODE_SEQUENCE &&
                (!monster.motionUnit->pAnimSeq || UNITS_IsAtEndOfFrameCycle(monster.motionUnit))) {
            UNITS_SetUsedSkill(monster.motionUnit, nullptr);
            d2_realm_client_unit_set_mode(monster.motionUnit, MONMODE_NEUTRAL);
        }
        // The mode's update (D2Client+0x4F4A0) steps a monster along its path only in a mode that moves.
        monster.lastTickResult = d2_realm_client_motion_advance_unit(monster.motionUnit, unit_mode_move_kind(monster.motionUnit) == 1);
        if (monster.lastTickResult > 0) moving++;
        run_unit_queue(UNIT_MONSTER, monster.view.guid);
        refresh_monster_motion_view(monster);
    }
    // Units the loops above did not step (no path yet) still run what was stored on them.
    while (!g_unitQueue.empty()) {
        const uint8_t unitType = g_unitQueue.front().unitType;
        const uint32_t guid = g_unitQueue.front().guid;
        run_unit_queue(unitType, guid);
        if (unitType == UNIT_PLAYER)
            for (size_t i = 0; i < g_players.size(); ++i)
                if (g_players[i].view.active && g_players[i].view.guid == guid && g_players[i].motionUnit)
                    refresh_player_motion_view(g_players[i], static_cast<int>(i) == g_selectedPlayer);
    }
    return moving;
}

EMSCRIPTEN_KEEPALIVE const D2RealmClientMonsterView* d2_realm_client_monsters() {
    g_monsterViews.clear();
    g_monsterViews.reserve(g_monsters.size());
    for (const auto& monster : g_monsters) g_monsterViews.push_back(monster.view);
    return g_monsterViews.data();
}
EMSCRIPTEN_KEEPALIVE int d2_realm_client_monster_death_mode_event_count() {
    return static_cast<int>(g_monsterDeathModeEvents.size());
}
EMSCRIPTEN_KEEPALIVE const d2::realm::MonsterDeathModeEvent* d2_realm_client_monster_death_mode_events() {
    g_monsterDeathModeEventView = g_monsterDeathModeEvents;
    g_monsterDeathModeEvents.clear();
    return g_monsterDeathModeEventView.data();
}
EMSCRIPTEN_KEEPALIVE int d2_realm_client_monster_aux(uint32_t index, D2RealmClientMonsterAux* out) {
    if (!out || index >= g_monsters.size()) return 0;
    const auto& monster = g_monsters[index];
    *out = {};
    out->version = 2;
    out->size = sizeof(*out);
    out->guid = monster.view.guid;
    out->hasExtra31 = monster.hasExtra31 ? 1u : 0u;
    out->extra31 = monster.extra31;
    out->hasStats = monster.hasStats ? 1u : 0u;
    out->statCount = static_cast<uint32_t>(monster.stats.size());
    if (!monster.stats.empty()) {
        out->firstStatId = monster.stats.front().statId;
        out->firstStatParameter = monster.stats.front().parameter;
        out->firstStatValue = monster.stats.front().value;
    }
    out->unitFlags = monster.motionUnit ? monster.motionUnit->dwFlags : 0;
    return 1;
}
EMSCRIPTEN_KEEPALIVE int d2_realm_client_monster_motion_status(uint32_t index, D2RealmClientMotionStatus* out) {
    if (!out) return 0;
    *out = {};
    if (index >= g_monsters.size()) return 0;
    const auto& monster = g_monsters[index];
    out->version = 1;
    out->size = sizeof(*out);
    out->guid = monster.view.guid;
    out->mode = monster.view.mode;
    out->framePrecise = monster.view.framePrecise;
    out->frameCountPrecise = monster.view.frameCountPrecise;
    out->animSpeed = monster.view.animSpeed;
    out->lastPlanResult = monster.lastPlanResult;
    out->lastTickResult = monster.lastTickResult;
    const D2UnitStrc* unit = monster.motionUnit;
    const D2DynamicPathStrc* path = unit ? unit->pDynamicPath : nullptr;
    if (path) {
        out->attached = 1;
        out->pathFlags = path->dwFlags;
        out->pathType = path->dwPathType;
        out->pathPoints = path->dwPathPoints;
        out->currentPoint = path->dwCurrentPointIdx;
        out->startX = path->tGameCoords.wPosX;
        out->startY = path->tGameCoords.wPosY;
        out->targetX = path->tTargetCoord.X;
        out->targetY = path->tTargetCoord.Y;
    }
    return 1;
}
EMSCRIPTEN_KEEPALIVE int d2_realm_client_monster_motion_status_v2(uint32_t index, D2RealmClientMotionStatusV2* out) {
    if (!out) return 0;
    *out = {};
    if (!d2_realm_client_monster_motion_status(index, &out->base)) return 0;
    out->base.version = 2;
    out->base.size = sizeof(*out);
    const D2UnitStrc* unit = g_monsters[index].motionUnit;
    const D2DynamicPathStrc* path = unit ? unit->pDynamicPath : nullptr;
    const D2DrlgRoomStrc* room = path && path->pRoom ? path->pRoom->pDrlgRoom : nullptr;
    if (room) {
        out->roomLevel = room->pLevel ? static_cast<uint32_t>(room->pLevel->nLevelId) : 0;
        out->roomTileX = static_cast<uint32_t>(room->nTileXPos);
        out->roomTileY = static_cast<uint32_t>(room->nTileYPos);
        out->roomTileWidth = static_cast<uint32_t>(room->nTileWidth);
        out->roomTileHeight = static_cast<uint32_t>(room->nTileHeight);
        out->clientSightRefs = room->wRoomsInList[ROOMSTATUS_CLIENT_IN_SIGHT];
    }
    return 1;
}
EMSCRIPTEN_KEEPALIVE int d2_realm_client_monster_motion_status_v3(uint32_t index, D2RealmClientMotionStatusV3* out) {
    if (!out) return 0;
    *out = {};
    if (!d2_realm_client_monster_motion_status_v2(index, &out->base)) return 0;
    out->base.base.version = 3;
    out->base.base.size = sizeof(*out);
    const D2RealmClientMonster* monster = &g_monsters[index];
    const D2DynamicPathStrc* path = monster->motionUnit ? monster->motionUnit->pDynamicPath : nullptr;
    if (path) {
        out->currentDirection = path->nDirection;
        out->targetDirection = path->nNewDirection;
        out->directionDelta = path->nDiffDirection;
        out->directionFlags = path->dwFlags;
    }
    return 1;
}
// Collision probe with the exact pattern/mask the assigned monster path planner uses. Temporarily
// remove/restore the carrier's own footprint, matching #10334's collision preflight.
// Returns -1 outside its loaded active-room chain, 0 clear, or 1 colliding.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_monster_motion_collision(uint32_t index, int x, int y) {
    if (index >= g_monsters.size()) return -1;
    const D2UnitStrc* unit = g_monsters[index].motionUnit;
    const D2DynamicPathStrc* path = unit ? unit->pDynamicPath : nullptr;
    if (!path || !path->pRoom) return -1;
    PATH_RemoveCollisionFootprintForUnit(const_cast<D2UnitStrc*>(unit), FALSE);
    D2ActiveRoomStrc* room = COLLISION_GetRoomBySubTileCoordinates(path->pRoom, x, y);
    const int collision = room ? (COLLISION_CheckAnyCollisionWithPattern(room, x, y, path->dwCollisionPattern,
        path->nMoveTestCollisionMask) ? 1 : 0) : -1;
    PATH_AddCollisionFootprintForUnit(const_cast<D2UnitStrc*>(unit));
    return collision;
}
EMSCRIPTEN_KEEPALIVE const D2RealmClientPlayerState* d2_realm_client_player_state() { return &g_player; }
EMSCRIPTEN_KEEPALIVE const D2RealmGameChat* d2_realm_client_game_chat() { return &g_gameChat; }
EMSCRIPTEN_KEEPALIVE uint32_t d2_realm_client_game_chat_count() { return g_gameChatCount; }
EMSCRIPTEN_KEEPALIVE uint32_t* d2_realm_client_player_unit_seed() {
    return g_player.active && g_player.cunitRngValid ? &g_player.cunitRngLow : nullptr;
}
// Derived from the selected canonical CUnit, so 0x0B, removal and replacement cannot
// leave stale hand selection behind. Existing player/render ABI remains unchanged.
EMSCRIPTEN_KEEPALIVE const int32_t* d2_realm_client_selected_skills() {
    static int32_t result[4];
    result[0] = result[1] = result[2] = result[3] = -1;
    if (g_selectedPlayer < 0 || static_cast<size_t>(g_selectedPlayer) >= g_players.size()) return result;
    auto* unit = g_players[g_selectedPlayer].motionUnit;
    if (!unit || !unit->pSkills) return result;
    auto* left = UNITS_GetLeftSkill(unit);
    auto* right = UNITS_GetRightSkill(unit);
    if (left) { result[0] = SKILLS_GetSkillIdFromSkill(left, __FILE__, __LINE__); result[2] = SKILLS_GetOwnerGUIDFromSkill(left); }
    if (right) { result[1] = SKILLS_GetSkillIdFromSkill(right, __FILE__, __LINE__); result[3] = SKILLS_GetOwnerGUIDFromSkill(right); }
    return result;
}

// Same eight-byte row as d2_session_skill_choices, but sourced from the GUID-matched
// realm player's canonical CUnit. The realm popup follows the live realm player view,
// including its roster skills, room level, usability and class skill metadata.
struct D2RealmClientSkillChoice {
    uint16_t id;
    int8_t listRow;
    int8_t charClass;
    uint16_t iconCel;
    uint8_t flags;
    uint8_t level;
};
static_assert(sizeof(D2RealmClientSkillChoice) == 8, "realm skill choice row is 8 bytes");
// The skills table's row count, the bound the 0x7B hotkey handler compares a skill against (1.13c D2Client+0xAC6D0 reads it at sgptDataTables+0xBA0).
EMSCRIPTEN_KEEPALIVE int32_t d2_realm_skill_count() {
    return sgptDataTables ? sgptDataTables->nSkillsTxtRecordCount : 0xfff;   // no tables loaded: nothing to clamp against
}
EMSCRIPTEN_KEEPALIVE uint32_t d2_realm_client_skill_choices(uint32_t guid, D2RealmClientSkillChoice* out, uint32_t cap) {
    if (!out || !cap) return 0;
    auto player = std::find_if(g_players.begin(), g_players.end(), [guid](const auto& entry) {
        return entry.view.active && entry.view.guid == guid;
    });
    if (player == g_players.end()) return 0;
    auto* unit = player->motionUnit;
    if (!unit || !unit->pSkills) return 0;
    const bool inTown = DUNGEON_IsTownLevelId(static_cast<int>(player->view.clientRoomLevel));
    uint32_t n = 0;
    for (D2SkillStrc* skill = SKILLS_GetFirstSkillFromSkillList(unit->pSkills); skill && n < cap;
         skill = SKILLS_GetNextSkill(skill)) {
        D2SkillsTxt* txt = SKILLS_GetSkillsTxtRecordFromSkill(skill);
        if (!txt) continue;
        D2SkillDescTxt* desc = DATATBLS_GetSkillDescTxtRecord(txt->wSkillDesc);
        auto& row = out[n++];
        row.id = static_cast<uint16_t>(SKILLS_GetSkillIdFromSkill(skill, __FILE__, __LINE__));
        row.listRow = desc ? static_cast<int8_t>(desc->nListRow) : -1;
        row.charClass = txt->nCharClass;
        row.iconCel = desc ? desc->nIconCel : 0;
        // bit 4 as d2_session_skill_choices: every non-usable state is red except an aura (retail D2Client+0x3DA60 gives it a non-red class).
        const int useState = SKILLS_GetUseState(unit, skill);
        row.flags = (txt->dwFlags[0] & SKILLSFLAG_PASSIVE ? 1 : 0) |
            (txt->dwFlags[0] & SKILLSFLAG_LEFTSKILL ? 2 : 0) |
            (useState != SKILLUSTATE_USABLE && useState != SKILLUSTATE_AURA ? 4 : 0) |
            (inTown && !(txt->dwFlags[0] & SKFLAG_INTOWN) ? 8 : 0) |
            (useState == SKILLUSTATE_AURA ? 16 : 0);   // bit 16 = aura, drawn with colour shift 4 (D2Client+0x3DA60)
        row.level = static_cast<uint8_t>(SKILLS_GetSkillLevel(unit, skill, TRUE));
        // bits 32/64: the bonus is positive / negative (the tree's level text is blue / red, D2Client+0x78820).
        const int bonus = SKILLS_GetBonusSkillLevelFromSkillId(unit, row.id);
        row.flags |= (bonus > 0 ? 32 : 0) | (bonus < 0 ? 64 : 0);
    }
    return n;
}

EMSCRIPTEN_KEEPALIVE int d2_realm_client_player_used_skill_at(int index) {
    if (index < 0 || static_cast<size_t>(index) >= g_players.size()) return -1;
    auto* unit = g_players[index].motionUnit;
    if (!unit || !unit->pSkills) return -1;
    auto* skill = UNITS_GetUsedSkill(unit);
    return skill ? SKILLS_GetSkillIdFromSkill(skill, __FILE__, __LINE__) : -1;
}

// A roster player in a Disguise state (Werewolf, Werebear) is drawn as the monster ConvertMode names, not
// as its class token. Fills the 96-byte monster animation record (the layout parseMonsterAnimations reads)
// and returns 1, or returns 0 when the unit is not converted to a monster.
struct RealmPlayerFormRecord {
    uint32_t id, classId, code, mode, framePrecise, frameCountPrecise, componentCodes[16];
    uint16_t animSpeed; uint8_t direction, flags; uint32_t baseWeaponClass;
};
static_assert(sizeof(RealmPlayerFormRecord) == 96, "monster animation record");
EMSCRIPTEN_KEEPALIVE int d2_realm_client_player_form_at(int index, RealmPlayerFormRecord* out) {
    if (!out || index < 0 || static_cast<size_t>(index) >= g_players.size()) return 0;
    const D2UnitStrc* unit = g_players[static_cast<size_t>(index)].motionUnit;
    if (!unit || !unit->pDynamicPath) return 0;
    int unitType = static_cast<int>(unit->dwUnitType), classId = static_cast<int>(unit->dwClassId);
    int mode = UNITS_GetAnimOrSeqMode(const_cast<D2UnitStrc*>(unit));
    D2COMMON_11013_ConvertMode(const_cast<D2UnitStrc*>(unit), &unitType, &classId, &mode, __FILE__, __LINE__);
    if (unitType != UNIT_MONSTER) return 0;
    const D2MonStatsTxt* mon = DATATBLS_GetMonStatsTxtRecord(classId);
    const D2MonStats2Txt* mon2 = UNITS_GetMonStats2TxtRecordFromMonsterId(classId);
    if (!mon) return 0;
    memset(out, 0, sizeof *out);
    out->id = unit->dwUnitId; out->classId = static_cast<uint32_t>(classId); out->code = mon->dwCode;
    out->mode = static_cast<uint32_t>(mode);
    out->framePrecise = static_cast<uint32_t>(unit->nSeqCurrentFramePrecise);
    out->frameCountPrecise = static_cast<uint32_t>(unit->dwFrameCountPrecise);
    out->animSpeed = static_cast<uint16_t>(unit->wAnimSpeed);
    out->direction = static_cast<uint8_t>(PATH_GetDirection(unit->pDynamicPath));
    out->baseWeaponClass = mon2 ? static_cast<uint32_t>(mon2->nBaseW) : 0;
    out->flags = (mon2 && (mon2->dwModeFlags & MONSTATS2MODEFLAG_DD)) ? 0x02 : 0;
    for (uint32_t c = 0; c < 16; ++c) out->componentCodes[c] = static_cast<uint32_t>(D2Common_11070(classId, c, 0));
    return 1;
}

EMSCRIPTEN_KEEPALIVE int d2_realm_client_player_count() { return static_cast<int>(g_players.size()); }
EMSCRIPTEN_KEEPALIVE const D2RealmClientPlayerState* d2_realm_client_player_at(int index) {
    return index >= 0 && static_cast<size_t>(index) < g_players.size() ? &g_players[index].view : nullptr;
}
// Read live render state from the same canonical CUnit whose DynamicPath the realm feed and
// 25 Hz tick mutate. The 92-byte historical assignment view stays ABI-compatible.
EMSCRIPTEN_KEEPALIVE const D2RealmClientPlayerEntry::MotionView* d2_realm_client_player_motion_at(int index) {
    if (index < 0 || static_cast<size_t>(index) >= g_players.size()) return nullptr;
    auto& entry = g_players[static_cast<size_t>(index)];
    auto& out = entry.motionView;
    out.active = 0;
    out.mode = out.direction = out.framePrecise = out.frameCountPrecise = out.animSpeed = 0;
    out.positionHintX = out.positionHintY = 0;
    out.seqMode = out.seqFrame = 0;
    const D2UnitStrc* unit = entry.motionUnit;
    if (!unit || !unit->pDynamicPath) return &out;
    out.active = 1;
    out.mode = unit->dwAnimMode;
    out.direction = PATH_GetDirection(unit->pDynamicPath);
    out.framePrecise = unit->nSeqCurrentFramePrecise;
    out.frameCountPrecise = unit->dwFrameCountPrecise;
    out.animSpeed = static_cast<uint32_t>(unit->wAnimSpeed);
    out.positionHintX = unit->nLastClickX;
    out.positionHintY = unit->nLastClickY;
    if (unit->dwAnimMode == PLRMODE_SEQUENCE) {
        out.seqMode = unit->dwSeqMode;
        out.seqFrame = static_cast<uint32_t>(unit->nSeqCurrentFramePrecise >> 8);
    }
    return &out;
}
EMSCRIPTEN_KEEPALIVE const D2RealmClientWorldState* d2_realm_client_world_state() { return &g_world; }
// The single-player snapshot's shape (68 bytes, see d2gs_client_snapshot.h) from the game-server
// client state: the selected player's CUnit (position, life, mode, flags), the act/seed/difficulty
// the server sent, the live monster roster size. Empty until a player is assigned.
EMSCRIPTEN_KEEPALIVE const d2::realm::ClientSnapshot* d2_realm_client_snapshot() {
    static d2::realm::ClientSnapshot snapshot;
    d2::realm::ClientSnapshotInput in{};
    in.playerActive = g_player.active != 0;
    in.seed = g_world.mapSeed; in.frame = g_clientMissileTick;
    in.level = g_player.clientRoomActive ? g_player.clientRoomLevel : g_world.area;
    in.difficulty = g_clientDifficulty; in.act = g_world.act;
    in.classId = g_player.classId; in.guid = g_player.guid;
    in.x = static_cast<int32_t>(g_player.x); in.y = static_cast<int32_t>(g_player.y);
    in.mode = g_player.initialMode;
    in.monsters = static_cast<uint32_t>(g_monsters.size());
    if (g_selectedPlayer >= 0 && static_cast<size_t>(g_selectedPlayer) < g_players.size()) {
        const D2UnitStrc* unit = g_players[static_cast<size_t>(g_selectedPlayer)].motionUnit;
        if (unit && unit->pDynamicPath) {
            in.x = unit->pDynamicPath->tGameCoords.wPosX; in.y = unit->pDynamicPath->tGameCoords.wPosY;
            in.mode = unit->dwAnimMode; in.flags = unit->dwFlags;
            in.hp = STATLIST_UnitGetStatValue(const_cast<D2UnitStrc*>(unit), STAT_HITPOINTS, 0);
            in.maxHp = STATLIST_UnitGetStatValue(const_cast<D2UnitStrc*>(unit), STAT_MAXHP, 0);
        }
    }
    snapshot = d2::realm::build_client_snapshot(in);
    return &snapshot;
}

// The inventory panel's weapon boxes for the selected player's worn arms (the same rule the single-player panel
// reads): which empty-arm silhouettes and two-hander ghosts the active set shows. -1 with no player.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_weapon_hands() {
    if (g_selectedPlayer < 0 || static_cast<size_t>(g_selectedPlayer) >= g_players.size()) return -1;
    D2UnitStrc* player = g_players[static_cast<size_t>(g_selectedPlayer)].motionUnit;
    if (!player || !player->pInventory || player->pInventory->dwSignature != D2C_InventoryHeader) return -1;
    D2InventoryStrc* inv = player->pInventory;
    D2UnitStrc* r = INVENTORY_GetItemFromBodyLoc(inv, BODYLOC_RARM);
    D2UnitStrc* l = INVENTORY_GetItemFromBodyLoc(inv, BODYLOC_LARM);
    return d2client_weapon_hands::rule(r != nullptr, l != nullptr, INVENTORY_GetWieldType(player, inv),
                                       ITEMS_GetQuiverType(r), ITEMS_GetQuiverType(l));
}

// Transaction price: 1.13c D2Common+0x29D60 forwards to ITEMS_CalculateTransactionCost (D2Common+0x28E80).
// The quest flags it reads are the client's quest record: 96 bytes the server sends with its 0x28 packets
// (empty until the first arrives), the record retail passes as the fourth argument.
// Transaction types: 0 buy, 1 sell, 2 gamble, 3 repair (D2C_TransactionTypes).
// The item format version the server encodes stock and owned items with (owned decoder: format 96).
constexpr uint32_t kStockItemFormatVersion = 96;

static D2BitBufferStrc* client_quest_record() {
    static D2BitBufferStrc* record = nullptr;
    if (!record) record = QUESTRECORD_AllocRecord(nullptr);
    return record;
}

static void clear_client_quest_record() {
    D2BitBufferStrc* record = client_quest_record();
    if (record && record->pBuffer) std::memset(record->pBuffer, 0, 96);
}

EMSCRIPTEN_KEEPALIVE int d2_realm_client_set_quest_flags(const uint8_t* bytes, int length) {
    if (!bytes || length != 96) return 0;
    QUESTRECORD_CopyBufferToRecord(client_quest_record(), const_cast<uint8_t*>(bytes), 96, FALSE);
    d2_realm_client_quests_set_player(g_quests, bytes);
    return 1;
}

// The quest state the server's packets built (layout: d2_realm_client_quests_view). `logOpen` tells the
// state-change rule whether the quest log is on screen.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_quests(uint8_t* out, int cap, int logOpen) {
    g_quests.logOpen = logOpen ? 1 : 0;
    return d2_realm_client_quests_view(g_quests, out, cap);
}

// A room's place in its level's room list: the ordinal a local game's room and exit records carry.
static uint32_t client_room_ordinal(const D2DrlgRoomStrc* room) {
    uint32_t ordinal = 0;
    for (const D2DrlgRoomStrc* r = room && room->pLevel ? room->pLevel->pFirstRoomEx : nullptr; r; r = r->pDrlgRoomNext, ++ordinal)
        if (r == room) return ordinal;
    return UINT32_MAX;
}

// The client's active rooms: the rooms the server's map-add packets (0x07) put in sight, with their
// neighbours, less those its map-remove packets (0x08) freed. One 36-byte record each, the local game's room
// record: ordinal, level, subtile x, y, width, height, flags, 0, 0. Returns the count written.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_rooms(uint8_t* out, int cap) {
    D2DrlgActStrc* act = d2_drlg_client_act();
    if (!out || !act) return 0;
    int count = 0;
    for (D2ActiveRoomStrc* room = DUNGEON_GetRoomFromAct(act); room && (count + 1) * 36 <= cap; room = room->pRoomNext) {
        const D2DrlgRoomStrc* drlgRoom = room->pDrlgRoom;
        const uint32_t record[9] = { client_room_ordinal(drlgRoom),
            drlgRoom && drlgRoom->pLevel ? static_cast<uint32_t>(drlgRoom->pLevel->nLevelId) : 0u,
            static_cast<uint32_t>(room->tCoords.nSubtileX), static_cast<uint32_t>(room->tCoords.nSubtileY),
            static_cast<uint32_t>(room->tCoords.nSubtileWidth), static_cast<uint32_t>(room->tCoords.nSubtileHeight),
            room->dwFlags, 0u, 0u };
        std::memcpy(out + count * 36, record, 36);
        ++count;
    }
    return count;
}

// The level exits in the client's active rooms, one per tile unit the server assigned: a 68-byte record each,
// the local game's exit record (slot, room, source level, destination level, LvlWarp id, enabled, direction,
// x, y, select box x, y, dx, dy, exit walk x, y, offset x, y), then one uint32 guid per record. The room's own
// warp tile of that LvlWarp id names the record and the destination level. Returns the count written.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_warps(uint8_t* out, int cap) {
    if (!out) return 0;
    std::vector<uint32_t> guids;
    int count = 0;
    for (const D2RealmClientWarp& warp : g_warps.list) {
        if ((count + 1) * 72 > cap) break;
        D2ActiveRoomStrc* active = d2_drlg_client_active_room(warp.x, warp.y);
        const D2DrlgRoomStrc* room = active ? active->pDrlgRoom : nullptr;
        if (!room || !room->pLevel) continue;
        const D2LvlWarpTxt* txt = nullptr;
        int32_t destination = 0;
        for (const D2RoomTileStrc* tile = room->pRoomTiles; tile; tile = tile->pNext) {
            if (!tile->pLvlWarpTxtRecord || tile->pLvlWarpTxtRecord->dwLevelId != warp.warpId) continue;
            txt = tile->pLvlWarpTxtRecord;
            if (tile->pDrlgRoom && tile->pDrlgRoom->pLevel) destination = tile->pDrlgRoom->pLevel->nLevelId;
            break;
        }
        if (!txt) txt = DATATBLS_GetLvlWarpTxtRecordFromLevelIdAndDirection(static_cast<int>(warp.warpId), 'b');
        int32_t record[17] = { count, static_cast<int32_t>(client_room_ordinal(room)), room->pLevel->nLevelId, destination,
            static_cast<int32_t>(warp.warpId), 1, 0, warp.x, warp.y, 0, 0, 0, 0, 0, 0, 0, 0 };
        if (txt) {
            record[6] = static_cast<uint8_t>(txt->szDirection[0]);
            record[9] = static_cast<int32_t>(txt->dwSelectX); record[10] = static_cast<int32_t>(txt->dwSelectY);
            record[11] = static_cast<int32_t>(txt->dwSelectDX); record[12] = static_cast<int32_t>(txt->dwSelectDY);
            record[13] = static_cast<int32_t>(txt->dwExitWalkX); record[14] = static_cast<int32_t>(txt->dwExitWalkY);
            record[15] = static_cast<int32_t>(txt->dwOffsetX); record[16] = static_cast<int32_t>(txt->dwOffsetY);
        }
        std::memcpy(out + count * 68, record, 68);
        guids.push_back(warp.guid);
        ++count;
    }
    if (count) std::memcpy(out + count * 68, guids.data(), static_cast<size_t>(count) * 4);
    return count;
}

// A hire offer's stats for the selected player: the hireling the 0x4E offer (name id, seed) rolls, through the same
// MONSTERS_HirelingInit the server runs for the purchase (act from the name id, the game's difficulty). out[0..5] =
// level, hit points, defence, gold cost, hireling id, experience. Returns 0 with no player or no matching row.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_hire_stats(uint32_t mercName, uint32_t seed, int expansion, int32_t* out) {
    if (!out || g_selectedPlayer < 0 || static_cast<size_t>(g_selectedPlayer) >= g_players.size()) return 0;
    D2UnitStrc* player = g_players[g_selectedPlayer].motionUnit;
    if (!player) return 0;
    const BOOL exp = expansion ? TRUE : FALSE;
    D2HirelingInitStrc init = {};
    const int act = MONSTERS_GetActFromHirelingTxt(exp, 0, static_cast<uint16_t>(mercName));
    if (!MONSTERS_HirelingInit(exp, player, static_cast<int>(seed), act, static_cast<int>(g_clientDifficulty), &init)) return 0;
    out[0] = init.nLevel; out[1] = init.nHitpoints; out[2] = init.nDefense; out[3] = init.nGold; out[4] = init.nId; out[5] = init.nExperience;
    return 1;
}

static int realm_client_transaction_cost(D2UnitStrc* player, D2UnitStrc* item, uint32_t vendorId,
        uint32_t transType) {
    if (!player || !item || !item->pItemData || transType > 3) return -1;
    if (!DATATBLS_GetNpcTxtRecord(vendorId)) return -1;
    return ITEMS_CalculateTransactionCost(player, item, static_cast<D2C_Difficulties>(g_clientDifficulty),
        client_quest_record(), static_cast<int>(vendorId), static_cast<D2C_TransactionTypes>(transType));
}

// The price of an item the selected or any listed player owns (sell, repair, gamble, buy-back).
EMSCRIPTEN_KEEPALIVE int d2_realm_client_item_price(uint32_t itemGuid, uint32_t vendorId, uint32_t transType) {
    for (auto& player : g_players) {
        D2UnitStrc* owner = player.motionUnit;
        if (!owner || !owner->pInventory || owner->pInventory->dwSignature != D2C_InventoryHeader) continue;
        for (D2UnitStrc* item = INVENTORY_GetFirstItem(owner->pInventory); item;
                item = INVENTORY_GetNextItem(item)) {
            if (item->dwUnitType == UNIT_ITEM && item->dwUnitId == itemGuid)
                return realm_client_transaction_cost(owner, item, vendorId, transType);
        }
    }
    return -1;
}

// 1.13c D2Client+0x46230 on the packet client: the "Sell value" line of a carried item while the trade window of
// NPC class `npcClass` is open, priced from the client's own hero unit, difficulty and quest record (the local
// session's d2_session_item_sell_value prices the server unit with the same callees). -1 = no line.
static D2UnitStrc* g_sellLinePlayer;
static int32_t g_sellLineNpcClass;
static int32_t sell_line_vendor(int32_t* c) { *c = g_sellLineNpcClass; return 1; }
static int32_t sell_line_cost(void* it, int32_t npc, int32_t type) {
    return realm_client_transaction_cost(g_sellLinePlayer, static_cast<D2UnitStrc*>(it), static_cast<uint32_t>(npc), static_cast<uint32_t>(type));
}
static int32_t sell_line_zero(void*) { return 0; }
static int32_t sell_line_one(void*) { return 1; }
static int32_t sell_line_id(uint32_t) { return 0; }
static int32_t sell_line_flag() { return 0; }
static const uint16_t* sell_line_str(uint32_t) { static const uint16_t z[1] = { 0 }; return z; }
static void sell_line_done(int32_t) {}
static bool sell_line_vendor_b(int32_t* c) { return sell_line_vendor(c) != 0; }
EMSCRIPTEN_KEEPALIVE int32_t d2_realm_client_item_sell_value(uint32_t itemGuid, int32_t npcClass) {
    D2UnitStrc* player = g_selectedPlayer >= 0 && size_t(g_selectedPlayer) < g_players.size() ? g_players[g_selectedPlayer].motionUnit : nullptr;
    if (!player || !player->pInventory || npcClass < 0 || !DATATBLS_GetNpcTxtRecord(npcClass)) return -1;
    D2UnitStrc* item = nullptr;
    for (D2UnitStrc* i = INVENTORY_GetFirstItem(player->pInventory); i; i = INVENTORY_GetNextItem(i))
        if (i->dwUnitType == UNIT_ITEM && i->dwUnitId == itemGuid) { item = i; break; }
    if (!item || !item->pItemData) return -1;  // found by walking the player's item list, which is what the ownership test walks
    // The plain-item gate of the session's sell line: no ear (flag 0x1000), no quest item, no type 0x27.
    D2ItemsTxt* txt = DATATBLS_GetItemsTxtRecord(item->dwClassId);
    if (!txt || (item->pItemData->dwItemFlags & 0x1000) || txt->nQuest || txt->wType[0] == 0x27) return -1;
    g_sellLinePlayer = player; g_sellLineNpcClass = npcClass;
    static const D2ClientPriceLineCb cb = { sell_line_zero, sell_line_id, sell_line_vendor_b, sell_line_one, sell_line_cost, sell_line_str,
        sell_line_flag, sell_line_one, sell_line_one, sell_line_zero, sell_line_done };
    int32_t amt = -1; uint16_t line[16];
    return d2_client_46230(&amt, item, 1, line, 16, 0, 0, &cb) ? amt : -1;
}

// The price of a shop stock item (S2C 0x9C) for the selected player. The stock bits are the item bitstream
// itself (flags, format version, mode, then the page the item sits on), decoded with the owned-item format.
// The stock decode skips the equip-mode and body-location checks an equipped item needs.
// Returns -1 when the bits do not decode to exactly their length or the vendor/quest state is unknown.
EMSCRIPTEN_KEEPALIVE int d2_realm_client_stock_price(const uint8_t* bytes, int length, uint32_t vendorId,
        uint32_t transType) {
    if (!bytes || length <= 0 || g_selectedPlayer < 0 || static_cast<size_t>(g_selectedPlayer) >= g_players.size())
        return -1;
    D2UnitStrc* player = g_players[g_selectedPlayer].motionUnit;
    if (!player) return -1;

    auto* item = UNITS_AllocUnit(nullptr, UNIT_ITEM);
    if (!item) return -1;
    ITEMS_AllocItemData(nullptr, item);
    ITEMS_InitItemSeed(item);
    UNITS_AllocStaticPath(item);
    if (!item->pItemData || !item->pStaticPath) {
        if (item->pItemData) ITEMS_FreeItemData(nullptr, item);
        if (item->pStaticPath) UNITS_FreeStaticPath(item);
        UNITS_FreeUnit(item);
        return -1;
    }
    UNITS_InitializeStaticPath(item, nullptr, 0, 0);
    // The decode writes the quantity and durability into the unit's stat list; without one they are dropped and a stack prices as one.
    if (!item->pStatListEx) STATLIST_AllocStatListEx(item, 0, nullptr, nullptr);

    int socketedItems = 0;
    BOOL failed = FALSE;
    const size_t consumed = ITEMS_DecodeItemFromBitstream(item, const_cast<uint8_t*>(bytes),
        static_cast<size_t>(length), FALSE, &socketedItems, kStockItemFormatVersion, &failed);
    int price = -1;
    if (!failed && consumed == static_cast<size_t>(length) && item->pItemData)
        price = realm_client_transaction_cost(player, item, vendorId, transType);
    STATLIST_FreeStatListEx(item);
    ITEMS_FreeItemData(nullptr, item);
    UNITS_FreeStaticPath(item);
    UNITS_FreeUnit(item);
    return price;
}

}  // extern "C"
