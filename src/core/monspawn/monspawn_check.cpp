#include <unordered_set>
// Retail-gated monster-spawn check (s05-monspawn): for each (seed, difficulty, level) of
// the retail captures, make the game the headless 1.13c D2Game made,
// load the act, initialise the level, activate every room in Room2-list order the way D2Game's
// room-activation routine does (region tick, preset units, objects, random monsters) and hash the
// units that hang off the rooms exactly as the retail capture tool does (D2VecMonUnit from `room` on).
#include <emscripten.h>
#include "../d2client/character_screen.h"
#include "../d2client/price_line_46230.h"
#include "warp_interaction.h"
#include "object_client_clock.h"
#include <algorithm>
#include <cstdint>
#include <set>
#include <map>
#include <tuple>
#include <Drlg/D2DrlgActivate.h>
#include <Drlg/D2DrlgDrlgRoom.h>
#include <string>
#include <strings.h>
#include <vector>
#include <array>
#include <unordered_map>
#include <D2Common.h>
#include <D2CMP.h>
#include <D2DataTbls.h>
#include <D2Dungeon.h>
#include <D2Collision.h>
#include <D2Skills.h>
#include <DataTbls/SkillsTbls.h>
#include <DataTbls/BeltsTbls.h>
#include <DataTbls/InvTbls.h>
#include <DataTbls/SkillsIds.h>
#include <DataTbls/MonsterIds.h>
#include <DataTbls/MonsterTbls.h>
#include <Drlg/D2DrlgDrlgLogic.h>
#include <Drlg/D2DrlgPreset.h>
#include <DataTbls/LevelsIds.h>
#include <Drlg/D2DrlgRoomTile.h>
#include <Drlg/D2DrlgDrlgWarp.h>
#include <Drlg/D2DrlgOutRoom.h>
#include <Drlg/D2DrlgDrlgGrid.h>
#include <D2Seed.h>
#include <D2Waypoints.h>
#include <D2StatList.h>
#include <D2ItemMods.h>
#include <D2States.h>
#include <D2Items.h>
#include <D2QuestRecord.h>
#include <D2BitManip.h>
#include "quest_object_operation.h"
#include "../realm/d2gs_client_monster_death_clock.h"
#include "npc_vendor_transition.h"
#include "npc_dialog_transition.h"
#include "pickup_path_probe.h"
#include "session_path_room_probe.h"
#include "object_interaction_gate.h"
#include "object_interaction_packet.h"
#include "../d2client/item_needs_repair.h"
#include "object_interaction_pending.h"
#include "hellforge_diagnostic.h"
#include "monster_sound_profile.h"
#include <QUESTS/ACT5/A5Q3.h>
#include <QUESTS/ACT5/A5Q2.h>
#include <QUESTS/ACT3/A3Q5.h>
#include <QUESTS/ACT4/A4Q3.h>
#include <D2Inventory.h>
#include <D2Composit.h>
#include <D2PacketDef.h>
#include <DataTbls/ItemsTbls.h>
#include <Units/Units.h>
#include <Units/Missile.h>
#include "../realm/client_fireball_hit.h"
#include "../realm/client_unit_sound.h"
#include "monster_light_terminal.h"
#include "monster_light_den.h"
#include "light_occlusion_policy.h"
#include "plugy_stash_grid.h"
#include "path_attempt_trace.h"
#include "item_text_visibility.h"
#include "item_flag_lines.h"
#include "item_stat_desc_value.h"
#include "drop_flip.h"
#include <D2Environment.h>
#include <DataTbls/ObjectsTbls.h>
#include <DataTbls/ObjectsIds.h>
#include <DataTbls/MonsterTbls.h>
#include <Units/Monster.h>
#include <D2Monsters.h>
#include <Path/Path.h>
#include <Path/PathMisc.h>
#include <Path/Step.h>
#include "path_collision_result.h"
#include <GAME/Game.h>
#include <GAME/Event.h>
#include <QUESTS/ACT4/A4Q3.h>
#include <GAME/Arena.h>
#include <GAME/Level.h>
#include <GAME/Clients.h>
#include <GAME/SCmd.h> // the join sequence's packet senders
void __fastcall D2GAME_SendPacket0x0B_ClientUnit(D2ClientStrc* pClient, D2UnitStrc* pUnit);
#include <AI/AiGeneral.h>
#include <MONSTER/MonsterUnique.h>
#include <MONSTER/MonsterSpawn.h>   // D2GAME_SpawnMonster_6FC69F10
#include <UNIT/SUnit.h>
#include <UNIT/SUnitMsg.h>
#include <AI/AiUtil.h>
#include <MONSTER/MonsterMode.h>
#include <UNIT/SUnitDmg.h>
#include <SKILLS/SkillAss.h>
#include <UNIT/SUnitInactive.h>
#include <UNIT/SUnitProxy.h>
#include <SKILLS/Skills.h>
#include <PLAYER/PlrModes.h>
#include <PLAYER/PlrMsg.h>
#include <PLAYER/PlrIntro.h>
#include <PLAYER/Player.h>
#include <PLAYER/PlayerPets.h>
#include <PLAYER/PlayerStats.h>
#include <PLAYER/PlrSave.h>
#include <PLAYER/PlrSave2.h>
#include <ITEMS/ItemMode.h>
#include <INVENTORY/InvMode.h>   // D2GAME_INVMODE_Last_6FC416D0, the save-time delivery-flag reset
#include <UNIT/Party.h>
#include <MONSTER/MonsterRegion.h>
#include <MONSTER/Monster.h>
#include <MONSTER/MonsterAI.h>
#include <MONSTER/MonsterMode.h>
#include <OBJECTS/ObjRgn.h>
#include <OBJECTS/Objects.h>
#include <OBJECTS/ObjMode.h>
#include <QUESTS/Quests.h>
#include <QUESTS/ACT1/A1Q1.h>
#include <QUESTS/ACT1/A1Q6.h>
#include <QUESTS/ACT5/A5Q4.h>
#include <QUESTS/ACT5/A5Q6.h>
#include "../d2moo/compat/datatbls_bridge.h"
#include "../tables/shared.h"
#include "weather_runtime.h"
#include "../d2client/weather_sim.h"
#include "../d2client/inv_sockets_95140.h"
#include "../d2client/inv_weapon_hands.h"
#include "hellforge_diagnostic.h"
#include "../d2common/d2common_33110.h"
#include "../../../tools/vectorfmt/vectorfmt.h"

static_assert(offsetof(D2Act4Quest3Strc, nHellforgeObjectMode) == 0x00);
static_assert(offsetof(D2Act4Quest3Strc, bRewardDropsPending) == 0x03);
static_assert(offsetof(D2Act4Quest3Strc, bSoulstoneSmashed) == 0x04);
static_assert(offsetof(D2Act4Quest3Strc, nGemDropTier) == 0x08);
static_assert(offsetof(D2Act4Quest3Strc, nHits) == 0x0C);

extern int g_d2ClientSoundObjects; // DrlgPreset.cpp
extern "C" D2DrlgActStrc* d2_drlg_client_act();
extern "C" int d2_drlg_client_act_begin(uint32_t seed, int difficulty, int act, int townLevel);
extern "C" int d2_drlg_client_map_reveal(int level, int x, int y, int hasPlayer, int playerLevel, int playerX, int playerY);
extern "C" int d2_drlg_client_map_hide(int level, int x, int y, int hasPlayer, int playerLevel, int playerX, int playerY);
extern "C" int d2_drlg_client_unit_context(int x, int y, int rollRoomSeed, uint32_t* levelId,
    uint32_t* roomTileX, uint32_t* roomTileY, uint32_t* unitInitSeed);
extern "C" int d2_session_client_cunit_seed_init(D2SeedStrc* clientSeed,
    uint32_t* unitInitSeed, int x, int y, int playerCurrent);
extern "C" void d2_drlg_act_end();

extern "C" void D2WEB_ResetPacketStream();
extern "C" void D2WEB_ResetHitPackets();
extern "C" const uint8_t* D2WEB_PacketStream(uint32_t* pLength, uint32_t* pPackets);
extern "C" uint32_t D2WEB_TakeMovieRequests(uint8_t* out, uint32_t cap);
extern "C" uint32_t d2_client_local_cast_refused();
extern "C" int32_t d2_client_cast_walked();
static uint32_t g_sessionPacketStreamLength, g_sessionPacketStreamPackets;

static std::string g_err;
static D2GameStrc* g_game;
static uint32_t g_gameSeed = ~0u; static int g_gameDiff = -1;
static std::vector<D2VecMonUnit> g_units;   // the last level's units (D2_MONS_DUMP reads them)
static D2VecMonLevel g_out;
static std::vector<D2VecAiUnit> g_aiUnits;
static D2VecAiLevel g_aiOut;
static D2DrlgLevelStrc* g_lastLevel;
static D2UnitStrc* g_targetPlayer;
static D2UnitStrc* g_targetMonster;
static D2ActiveRoomStrc* g_targetRoom;
static D2ClientStrc* g_targetClient;
struct D2SessionMonsterLightRecord {
    D2UnitStrc* unit = nullptr;
    D2ActiveRoomStrc* room = nullptr;
    uint32_t guid = 0;
    uint32_t classId = 0;
    int32_t x = 0, y = 0;
    int32_t radius = 0, targetRadius = 0;
    bool seen = false;
    bool terminalUnlinked = false;
};
static std::vector<D2SessionMonsterLightRecord> g_sessionMonsterLights;
static bool sessionMonsterLightIdentityMatches(const D2UnitStrc* recordUnit, uint32_t recordGuid,
    uint32_t recordClassId, const D2UnitStrc* unit, uint32_t unitGuid, uint32_t unitClassId) {
    return recordUnit == unit && recordGuid == unitGuid && recordClassId == unitClassId;
}
static bool sessionMonsterLightShouldEnroll(int32_t targetRadius) { return targetRadius > 0; }
static void sessionMonsterLightReplacementReset(D2SessionMonsterLightRecord& record, int32_t radius) {
    record.radius = radius;
    record.targetRadius = radius;
}
static void updateSessionMonsterLights();
static bool sessionNativeMonsterAssignmentSuppressed(const D2UnitStrc* unit);
// The character's name, as chosen on the creation screen. Retail's header refuses a name that does
// not fit in 16 bytes including its terminator, so this buffer matches that limit exactly.
static char g_sessionCharName[16] = "PLAYER";
// The retail client save flags (D2ClientSaveFlags) the session is created with -- the character
// creation screen's Expansion and Hardcore checkboxes. Retail's PLRSAVE2_WriteSaveHeader stores them
// in the file's header word at 0x24 and PLRSAVE2_CheckPlayerFlags compares them against the game the
// character joins, so the game's own mode has to agree with them or the file it writes cannot be
// loaded back. Default: retail's own creation defaults, Expansion checked and Hardcore clear.
static uint32_t g_sessionClientSaveFlags = CLIENTSAVEFLAG_EXPANSION;
static std::vector<D2VecAiTargetUnit> g_targetUnits;
static D2VecAiLevel g_targetOut;
static std::vector<D2VecCombat> g_combatEvents;
static D2VecCombatResult g_combatResult;
struct D2SessionHitRoll {
    uint32_t frame, attackerId, defenderId, skillId, attackCount;
    int32_t attackerLevel, defenderLevel, attackRate, toHitPercent, totalDefense;
    int32_t toHit, defense, toHitFactor, chanceToHit, roll, hit;
};
static_assert(sizeof(D2SessionHitRoll) == 64);
static std::vector<D2SessionHitRoll> g_sessionHitRolls;
static int g_currentAct = -1, g_currentLevel = -1;
// Offline browser sessions mirror D2Client's MapReveal/MapHide stream into a separately allocated
// client-mode Act. The server game Act remains authoritative for gameplay and its RNG is untouched.
struct SessionClientRoomKey { D2DrlgRoomStrc* roomEx; int level, x, y; };
static bool g_sessionClientActOwned = false, g_sessionClientMapStarted = false;
static int g_sessionClientAct = -1;
static D2ActiveRoomStrc* g_sessionClientCurrentRoom = nullptr;
static std::vector<SessionClientRoomKey> g_sessionClientNearRooms;
static std::vector<int32_t> g_sessionClientMapEvents; // {op(1 reveal/0 hide), level, tileX, tileY} records
static constexpr size_t kSessionClientMapEventCapacity = 65536; // 1 MiB of diagnostic records
static uint32_t g_sessionClientMapEventsDropped = 0;
// The offline browser client owns an independent local-player CUnit seed stream. It must never
// borrow g_targetPlayer->pSeed, which belongs to the D2Game/server unit.
namespace d2client_ambient { void reset(); void tick(uint32_t* seed, uint32_t period, bool weatherLevel); uint32_t* fallbackSeed(uint32_t (*playerInit)()); }
extern "C" void d2_client_monster_idle_reset();   // 1.13c D2Client+0x237E0 monster branch state (unit_sound_trigger_237e0.cpp)
extern "C" void d2_client_attack_roll_reset();   // 1.13c D2Client+0x242B0 attack-roll state (monster_face.cpp)
namespace d2client_hurricane { void reset(); void tick(D2UnitStrc* u, uint32_t* seed); }
namespace d2client_vine { void reset(); void tick(D2UnitStrc* hero); }
namespace d2client_clmissile { void reset(); void cast(int32_t skillId, int32_t level, D2UnitStrc* target, int32_t tx, int32_t ty); void tick(D2UnitStrc* owner, D2UnitStrc* (*find)(int32_t type, uint32_t id)); }   // 1.13c D2Client+0x15DA0 (cltdofunc 17), +0xB17C0 / +0xB1420 (client missiles)
namespace d2client_nova { void reset(); void selectSkill(int32_t skillId); int32_t selectedSkill(); void cast(int32_t skillId, bool presentation); void tick(); }   // 1.13c D2Client+0x73CB0 (cltdofunc 25, Poison Nova), +0x73D70 (cltdofunc 29, Frozen Orb)
namespace d2client_blizzard { void reset(); void cast(int32_t skillId, D2UnitStrc* target, int32_t tx, int32_t ty); void tick(D2UnitStrc* owner, D2UnitStrc* (*find)(int32_t type, uint32_t id)); }   // 1.13c D2Client+0xBBC10 (cltdofunc 13, Blizzard center)
static D2UnitStrc* g_sessionClientPlayerUnit = nullptr;
namespace d2client_unitseed { uint32_t init(uint32_t type, uint32_t id); }
static uint32_t sessionClientPlayerInit() { return g_targetPlayer ? d2client_unitseed::init(UNIT_PLAYER, g_targetPlayer->dwUnitId) : 0; }
// The client hero seed (+0x20): the client unit's own when the client map was mirrored, else the one derived from the local player's client init seed.
static uint32_t* sessionClientHeroSeed() {
    if (g_sessionClientPlayerUnit) return reinterpret_cast<uint32_t*>(&g_sessionClientPlayerUnit->pSeed);
    return d2client_ambient::fallbackSeed(sessionClientPlayerInit);
}
static uint32_t g_sessionClientPlayerInitSeed = 0;
// Session construction is class-selectable.  The older vector/AI attachment surface retains
// Amazon (0) unless a browser session explicitly supplies one of the seven CharStats classes.
static int g_sessionPlayerClass = 0;
// A real player (the client's create_class/_bare) is seeded from the game seed the moment it is
// allocated, as every retail player loader does (PLRSAVE2 new-character and save-load paths:
// SUNIT_AllocUnitData(UNIT_PLAYER) then SUNIT_InitSeed(pPlayer, &pGame->pGameSeed); AllocUnitData
// seeds every unit type but the player). The capture's player fixture is the bare allocation with
// a 0/0 seed, so the oracle rows keep it unseeded. A 0/0 seed
// rolls 0 forever: a player with a shield blocked every blow (0 < block chance), stood in
// PLRMODE_BLOCK and could not start a walk (s000d Outer Steppes, attack reason 8).
static bool g_sessionSeedPlayer = false;
// Recorded-session fixture (create_class_at): a swing mid-swing restarts, as retail's 0x06 does (a retail recording). Interactive play keeps the busy lock (s2312).
static bool g_sessionReplayFixture = false;

// Pointer-free state handed to the browser.  Keep this as an explicit wire layout: none of the
// native wasm64 pointers in D2Game/D2Unit are allowed to escape the engine boundary.
#pragma pack(push, 1)
struct D2SessionSnapshot {
    uint32_t version, size, seed, frame;
    uint16_t level;
    uint8_t difficulty, act;
    uint32_t playerClass, playerId;
    int32_t x, y, targetX, targetY, hp, maxHp;
    uint32_t mode, flags, monsters, hash;
};
struct D2SessionRoom {
    uint32_t ordinal, level;
    int32_t x, y, width, height;
    uint32_t flags, firstUnit, unitCount;
};
struct D2SessionUnit {
    uint32_t id, type;
    int32_t classId;
    uint32_t mode;
    int32_t x, y;
    uint32_t flags, flagsEx;
    int32_t hp, maxHp;
    uint32_t room;
};
struct D2SessionMonsterAnimation {
    uint32_t id, classId, code, mode, framePrecise, frameCountPrecise;
    uint32_t componentCodes[16];
    uint16_t animSpeed;
    uint8_t direction, animationFlags;
    uint32_t baseWeaponClass;
};
// A missile's own presentation record: what the client needs to draw the Missiles.txt CelFile at the
// frame and facing the ENGINE has it in, instead of a frame guessed from the shell's tick.  Retail's
// client draws a missile from the same unit fields a monster's is drawn from -- nSeqCurrentFramePrecise
// (advanced per game frame for every unit, see the tick below), dwFrameCountPrecise and
// pDynamicPath->nDirection -- plus Missiles.txt's own CelFile and nTrans.
struct D2SessionMissileAnimation {
    uint32_t id, classId, mode, framePrecise, frameCountPrecise;
    uint16_t animSpeed;
    uint8_t direction;   // pDynamicPath->nDirection, 0..63 (the client resolves it to the DCC index)
    uint8_t trans;       // Missiles.txt nTrans, forwarded for the client's non-hovered base draw mode
                         // (F20). The hovered-unit mode-7 override and pixel parity remain open.
    int32_t remaining;   // MISSILE_GetRemainingFrames: lets the sound layer predict when the missile ends
};
struct D2SessionObjectPresentation {
    uint32_t id, classId, token, mode;
    int32_t x, y;
    uint32_t framePrecise, frameCountPrecise, componentMask;
    uint16_t animSpeed;
    uint8_t direction, selectable;   // selectable: Objects.txt Selectable<mode>, the client's hover test
    uint8_t portalLevel, locked;     // portalLevel: town/perm portal destination (InteractType); locked: Lockable && InteractType&0x80 (D2Client+0xA5D90 name)
    uint16_t inClientRoom;           // the object's room has a client (retail's client only holds such units)
};
struct D2SessionItemPresentation {
    uint32_t id, classId, code, mode;
    int32_t x, y;
    uint32_t quality, itemFlags, framePrecise, frameCountPrecise;
    char flippyFile[32];
    char invFile[32];
    int16_t gridX, gridY;
    uint8_t invPage, bodyLoc;
    uint8_t invWidth, invHeight;   // ItemsTxt nInvWidth/nInvHeight: the cells the item covers
};
// D2Client+0x95140's inputs for one item: sockets are drawn on an identified, socketed item with
// sockets, under the pointer (the caller's hover test). off[n] is where socket n sits relative to the
// item's drawing anchor; gem[n] is the inventory file of the gem in it (empty: the empty socket cel).
struct D2SessionItemSockets {
    uint32_t id;
    uint8_t count, gems, pad[2];
    int16_t off[6][2];
    char gem[6][32];
};
static_assert(sizeof(D2SessionItemSockets) == 224);
struct D2SessionItemState {
    uint32_t version, size, commandTarget, result;
    uint32_t groundCount, inventoryCount, cursorItemId, inventoryHash;
    uint32_t flags, hash;
};
struct D2SessionResources { uint32_t version, size; int32_t hp, maxHp, mana, maxMana; };
// Rendering identity after States.txt's gfxtype/gfxclass and shapeshift-mode conversion.
// The source player remains a player unit; this record is only the effective composit request.
struct D2SessionPlayerRenderIdentity {
    uint32_t version, size, unitType;
    int32_t classId, mode, weaponClassId;
};
// One selected DT1 entry as used by an active DRLG room.  Library slots and entry ordinals
// replace native pointers at the browser boundary.  main/sub mirror the DT1 style/sequence
// names; frameIndex is the selected rarity/animation-frame field in the DT1 header.
struct D2SessionTile {
    uint32_t room, layer, ordinal;
    int32_t orientation, mainIndex, subIndex, frameIndex;
    uint32_t material;
    int32_t tileX, tileY, worldX, worldY, screenX, screenY, width, height;
    uint32_t flags, librarySlot, libraryIndex;
    // F66: the per-instance value retail's DRLGLOGIC_SetCoordListForTiles
    // writes at D2DrlgTileDataStrc+0x10, measured to be the field that separates a wall-list record
    // retail draws through the wall entry (39) from one it draws through the transparent/roof entry
    // (40) -- s338 pins it disjoint on two captured scenes. Exported so the client CAN read it; the
    // general selection rule (what in the room's coord lists makes a run draw transparently) is still
    // open research, not derived here.
    int32_t entryHint;
    // The level's roof-POP state for this tile (F44): 0 not pop-affected, 1 the pop that names this
    // tile's DT1 style is ACTIVE with the player inside its region (retail draws no such tile), 2 the
    // pop has been left and the tile is drawn again. Retail's own client keeps it in the tile's
    // `unk0x24` bit 1 and the direction in its green/blue pair; see sessionTilePop.
    uint32_t pop;
};
// F11b sidecar: D2Client traverses the current room's ordered active-near list, then each room's
// floor records in selector/source order. Keep this separate from D2SessionTile so its browser ABI
// and terrain hashes remain fixed. Room identity fields make the ordinal mapping auditable.
struct D2SessionActiveNearFeedRecord {
    uint32_t kind, roomOrdinal; // kind 0: room-order record; kind 1: floor source record
    int32_t levelId, tileX, tileY;
    uint32_t floorOrdinal, selector;
};
static_assert(sizeof(D2SessionActiveNearFeedRecord) == 28);
struct D2SessionMotion {
    uint32_t version, size, gameFrame, mode;
    int32_t x, y, targetX, targetY;
    uint32_t pathPoints, currentPoint, framePrecise, frameCountPrecise;
    uint16_t animSpeed;
    uint8_t direction, moving;
    uint8_t seqMode, seqFrame;  // PLRMODE_SEQUENCE: the current frame's mode (D2Common +0x40) and frame (+0x44 >> 8); 0 otherwise
};
// The player's position at the engine's own precision, for the one thing that must draw with it.
// Retail stores a dynamic path's position as 16.16 subtiles (`D2FP32_16 tGameCoords`: wOffsetX is
// the fraction, wPosX the whole subtile) and its client draws units with the same value, shifted
// into screen pixels (`PATH_UpdateClientCoords`: dwClientCoordX = dwPrecisionX >> 11). The session's
// other position fields -- x/y in D2SessionSnapshot, D2SessionMotion and D2SessionUnit, and
// d2_session_player_x/y -- publish only `wPosX`, and the shell drew its camera from those, so every
// drawn frame landed on a whole subtile: 16 px in x / 8 px in y at a time, and on a diagonal walk
// the drawn screen x REVERSED between neighbouring cells (s490 measured 3 reversals in 18 frames of
// a straight diagonal walk; retail's own captured path moves 5235.48292 -> 5238.45734 across
// frames). Read-only: nothing in the engine moves differently because these are published.
struct D2SessionCoords {
    uint32_t version, size;
    uint32_t preciseX, preciseY;   // tGameCoords.dwPrecisionX/Y, 16.16 subtiles
};
// The same 16.16 position for EVERY unit, one record per D2SessionUnit in the same order (carrying
// the id, so a misalignment is a named failure and not a wrong sprite). Only a unit with a dynamic
// path has one; (0,0) means "no precise position", which no real subtile is. See D2SessionCoords for
// why: drawn from `wPosX` alone, a walking monster holds still for several frames and then jumps a
// whole subtile -- 16 px in x / 8 px in y, and on a diagonal an x-step and a y-step alternately,
// which swings it +16 px and -16 px in screen x (s491).
struct D2SessionUnitCoord {
    uint32_t id, preciseX, preciseY;
};
// A unit's native dynamic-path facing, exported separately to keep the fixed 44-byte
// D2SessionUnit ABI stable. The id makes its join to the unit list explicit.
struct D2SessionUnitFacing {
    uint32_t id;
    uint8_t direction, hasDynamicPath, lastHitClass, usedSkillHi; // lastHitClass: unit+0xB0, the 0x0D packet's hit-class byte
    uint8_t typeFlags, usedSkillLo; int16_t superUniqueId; // typeFlags: MonsterData nTypeFlag; superUniqueId: SuperUniques row (wBossHcIdx), else -1
    // usedSkill ((Hi&0x7F)<<8|Lo) is a monster's used skill id + 1, 0 when none (the skill its 0x4C/0x4D names); Hi bit 7: the client's animation event this frame
};
// A unit's live overlay (stat STAT_UNIT_DOOVERLAY, set by UNITS_SetOverlay): one id per unit, 0 is absent.
// Exported only when nonzero, so consumers see the hit overlays a server set on a unit (Overlay.txt row index).
struct D2SessionUnitOverlay {
    uint32_t id, type, overlayId;
};
// One active state (STATES_CheckState, States.txt id) on a monster; the JS joins it to the unit and
// draws the state's Overlay.txt overlay1-4 loops. Only monsters: the player's states come from d2_session_player_has_state.
struct D2SessionUnitState {
    uint32_t id, type, stateId;
};
struct D2SessionAttack {
    uint32_t version, size, gameFrame, state;
    uint32_t attackerId, targetId, skillId, mode;
    int32_t targetHpBefore, targetHpAfter;
    uint32_t eventFrame, resultFlags, physicalDamage, totalDamage, attacks, hash;
};
struct D2SessionProgress {
    uint32_t version, size, gameFrame, targetId;
    int32_t targetHp;
    uint32_t targetMode;
    int32_t playerExperience, experienceDelta;
    uint32_t itemCount, itemHash, kills, flags;
};
struct D2SessionAdvancement {
    uint32_t version, size, gameFrame, level;
    int32_t experience, lastExperience, nextExperience;
    int32_t statPoints, skillPoints, strength, energy, dexterity, vitality;
    uint32_t skillId, skillBaseLevel, skillTotalLevel, hash;
};
struct D2SessionNpcInteraction {
    uint32_t version, size, commandTarget, result;
    uint32_t npcId, npcClass;
    int32_t distance, interactType, interactGuid;
    uint32_t npcInteractCount, npcHasPlayer, flags, hash;
};
struct D2SessionNpcDialog {
    uint32_t version, size, commandTarget, result, npcId;
    uint32_t packetHeader, packetUnitType, packetGuid, messageCount;
    uint16_t stringIds[8];
    uint8_t menus[8];
    uint32_t flags, hash;
};
struct D2SessionNpcResponse {
    uint32_t version, size, packetHeader, packetSize, npcId, messageId, result;
    uint32_t introBefore, introAfter;
    int32_t interactType, interactGuid;
    uint32_t flags, hash;
};
struct D2SessionVendor {
    uint32_t version, size, packetHeader, packetSize, action, npcId, result;
    uint32_t trading, vendorInit, itemCount, itemHash, firstClass, firstCode, firstQuality, hash;
};
struct D2SessionVendorItem {
    uint32_t id, classId, code, mode, quality, cost, flags, hash;
};
struct D2SessionVendorTransaction {
    uint32_t version, size, packetHeader, packetSize, npcId, itemId;
    uint32_t itemClass, itemCode, itemQuality, itemMode, transactionType;
    int32_t packetCost, handlerResult, goldBefore, goldAfter;
    uint32_t playerItemsBefore, playerItemsAfter, vendorItemsBefore, vendorItemsAfter;
    uint32_t ownedBefore, ownedAfter, resultItemId, flags, hash;
};
struct D2SessionRepair {
    uint32_t version, size, packetHeader, packetSize, npcId, itemId, itemClass;
    int32_t goldBefore, goldAfter, cost, durabilityBefore, durabilityAfter;
    int32_t maxDurability, quantityBefore, quantityAfter, maxQuantity;
    uint32_t ownedBefore, ownedAfter, handlerResult, flags, hash;
};
struct D2SessionAct1Quest1 {
    uint32_t version, size, level, questState, lastState, questFlags;
    uint32_t playerFlags, globalPrimaryGoal, skillPoints;
    int32_t monstersLeft;
    uint32_t enteredDen, akaraActivated, finished, timerActive, hash;
};
// A1Q6 is deliberately an observation record. Its fields come from the live quest controller
// and player bit buffer; it contains no browser-side completion state.
struct D2SessionAct1Quest6 {
    uint32_t version, size, level, questState, lastState, questFlags;
    uint32_t playerFlags, globalPrimaryGoal;
    int32_t andarielGuid;
    uint32_t timerInvocations, andarielKilled, killCallbackActive;
    uint32_t primaryGoalDone, rewardPending, rewardGranted, completedNow, hash;
};
struct D2SessionAct1Quest6Timer {
    uint32_t version, size, level, andarielId;
    uint32_t timerInvocations, timerStillScheduled, portalCount, portalId, portalClass, hash;
};
struct D2SessionAct1Quest6Complete {
    uint32_t version, size, level, timerInvocations;
    uint32_t timerReleased, lastState, state, questFlagsLow, playerFlags;
    uint32_t packetHeader, packetQuestId, packetFlags, packetState, packetField4, packetBytes, hash;
};
// One quest-drop item, the kind-16 vector's fields widened to u32 for the browser reader.
struct D2SessionAct1Quest6Drop {
    uint32_t itemId, classId, quality, ilvl, x, y, index, count;
    uint32_t seedLo, seedHi, bitBytes;
    uint8_t bits[32];
    uint32_t hash;
};
struct D2SessionItemSaveRoundTrip {
    uint32_t version, size, result, saveVersion;
    uint32_t bytesWritten, bytesConsumed, replayBytes, itemCount;
    uint32_t beforeInventoryCount, afterInventoryCount;
    uint32_t sectionHash, replayHash, flags, hash;
};
struct D2SessionStatsSaveRoundTrip {
    uint32_t version, size, result, saveVersion;
    uint32_t bytesWritten, bytesConsumed, replayBytes, statEntries;
    uint32_t sectionHash, replayHash, flags, beforeStatsHash, afterStatsHash;
    int32_t hpBefore, hpAfter, manaBefore, manaAfter;
    uint32_t hash;
};
struct D2SessionCampaignSaveRoundTrip {
    uint32_t version, size, result, saveVersion;
    uint32_t skillBytes, skillConsumed, skillCount, skillHash, skillReplayHash;
    uint32_t skillSemanticBefore, skillSemanticAfter;
    uint32_t waypointBytes, waypointConsumed, waypointHash, waypointReplayHash;
    uint32_t flags, hash, reserved;
};
struct D2SessionQuestIntroSaveRoundTrip {
    uint32_t version, size, result, saveVersion;
    uint32_t questBytes, questConsumed, questHash, questReplayHash;
    uint32_t introBytes, introConsumed, introHash, introReplayHash;
    uint32_t flags, questSemanticBefore, questSemanticAfter;
    uint32_t introSemanticBefore, introSemanticAfter, gheedBefore, gheedAfter, hash;
};
struct D2SessionCorpseSaveRoundTrip {
    uint32_t version, size, result, saveVersion;
    uint32_t bytesWritten, bytesConsumed, replayBytes, sectionCount;
    uint32_t corpseCountBefore, corpseCountAfter, sectionHash, replayHash;
    uint32_t flags, readerDetail, reserved, hash;
};
// One generated level exit. The active DRLG's Vis/Warp slot becomes source/destination IDs;
// the remaining fields are the loaded LvlWarp.txt selection/walk contract.
struct D2SessionWarp {
    uint32_t slot, room, sourceLevel, destinationLevel, warpId, enabled, direction;
    int32_t exitX, exitY;
    int32_t selectX, selectY, selectDX, selectDY, exitWalkX, exitWalkY, offsetX, offsetY;
};
#pragma pack(pop)
static_assert(sizeof(D2SessionSnapshot) == 68);
static_assert(offsetof(D2SessionSnapshot, x) == 28 && offsetof(D2SessionSnapshot, y) == 32);
static_assert(sizeof(D2SessionRoom) == 36);
static_assert(sizeof(D2SessionUnit) == 44);
static_assert(sizeof(D2SessionUnitFacing) == 12);
static_assert(sizeof(D2SessionUnitOverlay) == 12);
static_assert(sizeof(D2SessionUnitState) == 12);
static_assert(sizeof(D2SessionMonsterAnimation) == 96);
static_assert(offsetof(D2SessionMonsterAnimation, animSpeed) == 88);
static_assert(offsetof(D2SessionMonsterAnimation, baseWeaponClass) == 92);
static_assert(sizeof(D2SessionMissileAnimation) == 28);
static_assert(sizeof(D2SessionObjectPresentation) == 44);
static_assert(sizeof(D2SessionCoords) == 16);
static_assert(sizeof(D2SessionItemPresentation) == 112);
static_assert(sizeof(D2SessionItemState) == 40);
static_assert(sizeof(D2SessionResources) == 24);
static_assert(sizeof(D2SessionPlayerRenderIdentity) == 24);
static_assert(sizeof(D2SessionTile) == 84);
static_assert(sizeof(D2SessionMotion) == 54);
static_assert(sizeof(D2SessionAttack) == 64);
static_assert(sizeof(D2SessionProgress) == 48);
static_assert(sizeof(D2SessionAdvancement) == 68);
static_assert(sizeof(D2SessionNpcInteraction) == 52);
static_assert(sizeof(D2SessionNpcDialog) == 68);
static_assert(sizeof(D2SessionNpcResponse) == 52);
static_assert(sizeof(D2SessionVendor) == 60);
static_assert(sizeof(D2SessionVendorItem) == 32);
static_assert(sizeof(D2SessionVendorTransaction) == 96);
static_assert(sizeof(D2SessionRepair) == 84);
static_assert(sizeof(D2SessionAct1Quest1) == 60);
static_assert(sizeof(D2SessionAct1Quest6) == 68);
static_assert(sizeof(D2SessionAct1Quest6Timer) == 40);
static_assert(sizeof(D2SessionAct1Quest6Complete) == 64);
static_assert(sizeof(D2SessionAct1Quest6Drop) == 80);
static_assert(sizeof(D2SessionItemSaveRoundTrip) == 56);
static_assert(sizeof(D2SessionStatsSaveRoundTrip) == 72);
static_assert(sizeof(D2SessionCampaignSaveRoundTrip) == 72);
static_assert(sizeof(D2SessionQuestIntroSaveRoundTrip) == 80);
static_assert(sizeof(D2SessionCorpseSaveRoundTrip) == 64);
static_assert(sizeof(D2SessionWarp) == 68);
static D2SessionSnapshot g_sessionSnapshot;
static D2SessionResources g_sessionResources;
static D2SessionPlayerRenderIdentity g_sessionPlayerRenderIdentity;
static D2SessionMotion g_sessionMotion;
// The retail game and D2Client keep separate player units. The server's cast clock is an epoch
// (`gameFrame << 8`) used by gameplay events, while the client resets its local mode clock through
// #11090 and advances that copy through its per-frame mode policy. Keep only the cast presentation
// clock here; do not overwrite the server unit's event clock to make its sprite move.
static bool g_clientCastClockActive;
static uint32_t g_clientCastFramePrecise;
static uint32_t g_clientCastMode;
static uint16_t g_clientCastAnimSpeed;
// The client's own neutral-mode clock: retail's client unit plays NU at its mode-table speed and wraps,
// while the in-process server unit's clock does not move in that mode. Drawing only, like the cast clock.
static bool g_clientNeutralClockActive;
static uint32_t g_clientNeutralFramePrecise;
static uint16_t g_clientNeutralAnimSpeed;
// The client's shapeshift entry. When the Werewolf/Werebear state arrives, the client's state-on callback for it
// plays the form's S1 (the transformation) on the client unit, then neutral from frame 0, and turns that unit
// to direction 0. The server unit stays in its own cast/neutral modes. Drawing only.
static bool g_clientMorphActive;
static bool g_clientMorphStatePrev;
static uint32_t g_clientMorphFramePrecise;
static uint32_t g_clientMorphFrameCount;
static uint16_t g_clientMorphAnimSpeed;
// The client unit's own facing where it differs from the server unit's: a path record that only carries
// direction, target and step, turned by the same Common setter and per-frame stepper as the server path.
// Held until the server path takes a new target direction of its own.
static bool g_clientFacingActive;
static uint8_t g_clientFacingServerTarget;
static D2DynamicPathStrc g_clientFacing;
static D2SessionCoords g_sessionCoords;
static std::vector<D2SessionUnitCoord> g_sessionUnitCoords;
static std::vector<D2SessionUnitFacing> g_sessionUnitFacings;
static std::vector<D2SessionUnitOverlay> g_sessionUnitOverlays;
static std::vector<D2SessionUnitState> g_sessionUnitStates;
static D2SessionAttack g_sessionAttack;
static D2SessionProgress g_sessionProgress;
static D2SessionAdvancement g_sessionAdvancement;
static uint32_t g_sessionAdvancementSkillId = SKILL_MAGICARROW;
static D2SessionNpcInteraction g_sessionNpcInteraction;
static D2SessionNpcDialog g_sessionNpcDialog;
static D2SessionNpcResponse g_sessionNpcResponse;
static D2SessionVendor g_sessionVendor;
static std::vector<D2SessionVendorItem> g_sessionVendorItems;
static D2SessionVendorTransaction g_sessionVendorTransaction;
static D2SessionRepair g_sessionRepair;
static D2SessionAct1Quest1 g_sessionAct1Quest1;
static D2SessionAct1Quest6 g_sessionAct1Quest6;
static D2SessionAct1Quest6Timer g_sessionAct1Quest6Timer;
static D2SessionAct1Quest6Complete g_sessionAct1Quest6Complete;
static std::vector<uint32_t> g_sessionAct1Quest6ItemsBefore;
static std::vector<D2SessionAct1Quest6Drop> g_sessionAct1Quest6Drops;
static D2SessionItemSaveRoundTrip g_sessionItemSaveRoundTrip;
static std::vector<uint8_t> g_sessionItemSaveBytes;
static D2SessionStatsSaveRoundTrip g_sessionStatsSaveRoundTrip;
static std::vector<uint8_t> g_sessionStatsSaveBytes;
static D2SessionCampaignSaveRoundTrip g_sessionCampaignSaveRoundTrip;
static std::vector<uint8_t> g_sessionSkillSaveBytes;
static std::vector<uint8_t> g_sessionWaypointSaveBytes;
static D2SessionQuestIntroSaveRoundTrip g_sessionQuestIntroSaveRoundTrip;
static std::vector<uint8_t> g_sessionQuestSaveBytes;
static std::vector<uint8_t> g_sessionIntroSaveBytes;
static D2SessionCorpseSaveRoundTrip g_sessionCorpseSaveRoundTrip;
static std::vector<uint8_t> g_sessionCorpseSaveBytes;
static D2UnitStrc* g_sessionAttackTarget;
static D2SkillStrc* g_sessionAttackSkill;
// Defined beside `d2_session_attack`, near the end of this file; declared here because the per-tick
// kill count in `d2_session_tick` reads the same target long before that point.
static D2UnitStrc* sessionAttackTargetLive();
static uint32_t g_sessionAttackCount;
static uint32_t g_sessionAttackEndFrame;
static bool g_sessionAttackResolved;
static bool g_sessionTargetKillCounted;
static int32_t g_sessionInitialExperience;
static uint32_t g_sessionKills;
static std::vector<D2SessionRoom> g_sessionRooms;
static std::vector<D2SessionUnit> g_sessionUnits;
static std::vector<D2SessionMonsterAnimation> g_sessionMonsterAnimations;
struct ClientMonsterClock { uint32_t mode = 0; bool valid = false; bool seeded = false; int32_t clock = 0; };
static std::unordered_map<uint32_t, ClientMonsterClock> g_clientMonsterClock;
// 1.13c D2Client+0xA7780: every client unit created in a room advances that room's seed once; the low
// 32 bits become its dwInitSeed. Keyed by unit id, rolled at first sight in snapshot (packet) order.
static std::unordered_map<uint64_t, uint32_t> g_clientInitSeed;   // key: type<<32 | id (ids repeat across types)
static std::unordered_set<const void*> g_clientRoomsActivated;
// 1.13c D2Client+0x18370 (from 0x440C0): a room's first activation rolls its seed once per Levels cmon entry (until a negative id),
// plus the critter count roll when cpct hits. ponytail: critter placement rolls (0x181A0) and their A7780 creates are not drawn.
static void clientRoomActivate(D2ActiveRoomStrc* room, int levelId) {
    if (!g_clientRoomsActivated.insert(room).second) return;
    const D2LevelsTxt* lv = DATATBLS_GetLevelsTxtRecord(levelId);
    if (!lv) return;
    for (int i = 0; i < 4 && (int16_t)lv->wCMon[i] >= 0; ++i) {
        if (SEED_RollRandomNumber(&room->pSeed) % 100 >= lv->wCPct[i]) continue;
        const int id = lv->wCMon[i];
        const D2MonStatsTxt* m = id < sgptDataTables->nMonStatsTxtRecordCount ? &sgptDataTables->pMonStatsTxt[id] : nullptr;
        if (!m) continue;
        int32_t count = m->nMinGrp + (int32_t)SEED_RollLimitedRandomNumber(&room->pSeed, m->nMaxGrp - m->nMinGrp);
        if (lv->wCAmt[i]) count *= lv->wCAmt[i];
        // 1.13c D2Client+0x181A0: up to 10 tries of x then y; a free spot would create the unit (0xA7780).
        // ponytail: the created critter itself is not simulated, only its rolls and the A7780 seed roll.
        const int s2 = m->wMonStatsEx < sgptDataTables->nMonStats2TxtRecordCount ? sgptDataTables->pMonStats2Txt[m->wMonStatsEx].nSizeX : 0;
        const int w = room->tCoords.nSubtileWidth, h = room->tCoords.nSubtileHeight;
        for (; count > 0; --count)
            for (int t = 0; t < 10; ++t) {
                const int x = room->tCoords.nSubtileX + (w - 1 > 0 ? (int)SEED_RollLimitedRandomNumber(&room->pSeed, w - 1) : 0);
                const int y = room->tCoords.nSubtileY + (h - 1 > 0 ? (int)SEED_RollLimitedRandomNumber(&room->pSeed, h - 1) : 0);
                if (COLLISION_CheckMaskWithSize(room, x, y, s2, 0x3F11)) continue;
                SEED_RollRandomNumber(&room->pSeed);   // A7780 create roll
                break;
            }
    }
}
extern "C" int32_t d2_client_walk_tick(uint32_t); extern "C" void d2_client_walk_reset();  // src/core/d2client/client_walk_path.cpp
// D19: an object's client-side animation clock, apart from the server unit's (retail's headless server never advances it;
// the retail captures). Rebuilt from Objects.txt whenever the mode changes.
// `mode` is the server's; `shown` the effective mode. Retail's D2Client stepper (D2Client+0x25660) flips a
// non-cycling OP (mode 1) to ON locally once its last cel is reached, and clamps (no wrap) non-cycling modes.
struct ClientObjectClock { uint32_t mode = ~0u, shown = ~0u; int32_t clock = 0, count = 0; int16_t speed = 0; };
static std::unordered_map<uint32_t, ClientObjectClock> g_clientObjectClock;
static void setClientObjectShown(ClientObjectClock& c, const D2ObjectsTxt* t, uint32_t shown) {
    c.shown = shown;
    const bool ok = shown < 8;
    c.count = ok ? (int32_t)t->dwFrameCnt[shown] : 0; c.clock = ok ? (int32_t)((uint16_t)t->nStart[shown] << 8) : 0; c.speed = ok ? t->wFrameDelta[shown] : 0;
}
static ClientObjectClock& clientObjectClock(const D2UnitStrc* u) {
    ClientObjectClock& c = g_clientObjectClock[u->dwUnitId];
    if (c.mode != u->dwAnimMode) { c.mode = u->dwAnimMode; setClientObjectShown(c, u->pObjectData->pObjectTxt, c.mode); }
    return c;
}
static std::vector<D2SessionMissileAnimation> g_sessionMissileAnimations;
static std::vector<D2SessionObjectPresentation> g_sessionObjectPresentations;
static std::vector<D2SessionItemPresentation> g_sessionItemPresentations;
// Parallel to the above: ITEMS_GetColor(nullptr, item, &c, 0) -> c+1 (0xFF = no transform), the same
// component colour byte the save header carries. A monster's drop is magic/rare/unique/set: its ground
// flippy is drawn through that item's MixPalette remap, a normal item's is not.
static std::vector<uint8_t> g_sessionItemColours;
// Parallel too: the inventory/cursor cel's remap (D2Client+0x6B5A0 draws through ITEMS_GetColor(0,
// item, &c, 1)'s returned MixPalette(nInvTrans, colour)), packed by D2CMP_MixPaletteKey; 0 = none.
static std::vector<uint16_t> g_sessionItemInvColours;
uint16_t D2CMP_MixPaletteKey(const uint8_t* p);
static std::vector<D2SessionItemSockets> g_sessionItemSockets;
// The shop's stock as PRESENTATION records, parallel to the existing vendor list rather than bolted
// onto it. D2SessionVendorItem is 32 bytes with a hash over its own fields, and proven rows read it
// at that stride (s50, s54) -- widening it would move their hashes for no reason. Retail's trade
// window is a 10x4 grid of item icons (Inventory.txt row 7), so the client needs each item's
// invFile and grid cell, which is exactly what D2SessionItemPresentation already carries.
static std::vector<D2SessionItemPresentation> g_sessionVendorPresentations;
static D2SessionItemState g_sessionItemState;
// F715: the item a ground-pickup command is still walking to. sub_6FC828D0's ITEM branch starts the
// walk but relies on retail's interaction-completion callback (sub_6FC7F780's ITEM case, reached
// through the player's animation MODECHANGE event) to actually pick the item up on arrival; in this
// headless session that callback is never reached for a walk to an ITEM, so the hero walks to the
// pile, stops on it, and nothing picks it up (measured: the hero at distance 0, no collision, not
// busy, interact data set -- and the pile stays IMODE_ONGROUND; a second command at that same spot
// picks it up). This names the pile the walk is for, so the tick can complete it (below).
static uint32_t g_sessionPendingPickup = 0;
// The object a live 0x13 click is still walking to. sub_6FC828D0's UNIT_OBJECT branch starts the
// walk and stores the interact target, but its completion lives in the walk-mode handler
// sub_6FC7F780, which this session does not reach; without naming the object here an out-of-range
// click only walked. Retail then re-enters sub_6FC828D0 on arrival, which operates the object:
// UNITS_ChangeAnimMode(OBJMODE_OPERATING) for chests/corpses/barrels and the shrine callback
// (gpShrineTable_6FD28D18) that applies the boon. `d2_session_tick` completes it.
static uint32_t g_sessionPendingObject = 0;
static int32_t g_sessionPendingObjectGoalX = 0, g_sessionPendingObjectGoalY = 0;  // the click's path goal
// #9: a unit-target command whose melee approach walk is still running. Retail's Rcv0x0D records the
// same intent (UNITS_SetInteractData) and the walk mode's per-frame handler sub_6FC7F780 re-issues it
// when the approach path completes (PlrModes.cpp: when the path ends it reads the saved skill and
// restarts it). The session's attack/cast commands returned "move pending" and nothing resumed them,
// so an out-of-range left click only walked. Remember which command started the walk; the tick
// re-issues it once, at the same place it detects the walk completed.
static int g_sessionApproachKind = 0;         // 1 primary attack, 2 right skill, 3 left skill (0: none)
static uint32_t g_sessionApproachTarget = 0;  // the monster the walk is closing on
extern "C" int d2_realm_d2gs_move_to(uint8_t opcode, uint16_t x, uint16_t y, uint8_t* out);
static std::vector<D2SessionTile> g_sessionTiles;
static std::vector<D2SessionActiveNearFeedRecord> g_sessionActiveNearFeed;
// Parallel to g_sessionTiles: the index of the tile's entry in its DT1 file (UINT32_MAX: not a library entry).
// Kept out of D2SessionTile so the terrain hash and every row pinning that stride stay as they were.
static std::vector<uint32_t> g_sessionTileEntry, g_sessionSeamTileEntry;
// F66: per tile, the coord list retail's D2DrlgTileDataStrc+0x10 points at -- the identity D2Client's roof/wall
// hide test reads (D2Client 6fb29f20;. Parallel to g_sessionTiles for
// the same reason as g_sessionTileEntry (D2SessionTile's stride and terrain hash stay pinned). nIndex -1: the tile has
// no list (not a wall record, or retail's unk0x10 is 0). box0 is world TILE coordinates {x0,y0,x1,y1}.
struct D2SessionTileCoord { int32_t nIndex, x0, y0, x1, y1; };
static std::vector<D2SessionTileCoord> g_sessionTileCoord, g_sessionSeamTileCoord;
extern "C" uint32_t D2WEB_TileEntryFileIndex(const void* pEntry);
// Roll-order trace (DrlgRoomTile.cpp): per session tile, in d2_session_tiles order,
// { room-local GetTileCache ordinal, entries it chose among, index chosen }; ordinal UINT32_MAX
// when the record was initialised without a roll. Empty unless d2_session_trace_rolls(1) was set
// BEFORE the session was created -- the rolls happen at room activation, not at snapshot time.
struct D2WebRollTrace { uint32_t nOrdinal, nEntries, nChosen, nFramePrev, nFrameNext1, nFrameNext2; };
extern bool g_d2webRollTraceEnabled;
extern uint32_t g_d2webRollOrphans;
extern std::unordered_map<const D2DrlgTileDataStrc*, D2WebRollTrace> g_d2webRollTrace;
void D2WEB_ResetRollTrace();
struct D2WebCrossRoll { const D2DrlgRoomStrc* pRoom; uint32_t nOrdinal; const D2DrlgRoomStrc* pCause; };
extern std::vector<D2WebCrossRoll> g_d2webCrossRolls;
static std::vector<D2WebRollTrace> g_sessionTileRolls;
static std::vector<D2SessionWarp> g_sessionWarps;
static std::vector<int32_t> g_sessionWarpOrigins;   // x,y per warp, same order: the LvlWarp select box's anchor (subtiles)
// A warp tile of the level: its class is the LvlWarp.txt Id it belongs to (DRLGWARP_GetLvlWarpTxtRecordFromUnit
// matches pLvlWarpTxtRecord->dwLevelId -- the Id column -- against the tile unit's class), not a level id.
struct D2SessionWarpPreset { uint32_t room; int32_t warpId, x, y, destination; };
// The level a warp tile of this room leads to: the room's own warp tile of that LvlWarp Id
// (DRLGWARP_GetLvlWarpTxtRecordFromUnit's match) links to a room of the destination level.
static int32_t sessionWarpTileDestination(const D2DrlgRoomStrc* room, int32_t warpId) {
    for (const D2RoomTileStrc* t = room ? room->pRoomTiles : nullptr; t; t = t->pNext)
        if (t->pLvlWarpTxtRecord && (int32_t)t->pLvlWarpTxtRecord->dwLevelId == warpId && t->pDrlgRoom && t->pDrlgRoom->pLevel)
            return t->pDrlgRoom->pLevel->nLevelId;
    return -1;
}
// F52. An exit's POSITION is generation-time DRLG data -- the room's own DS1 -- not activation output:
// the built path below reads it out of the room's tile data (tile->unk0x0C/unk0x10, filled by
// DRLGROOMTILE_LoadWallWarpTiles while the room's tiles are laid out), so a room the activation wave
// has not built yet had no answer and the slot fell through to ANOTHER room's exit to the same level
// (Harem Level 1's warp to 51 read its sibling's (10042,5082) instead of its own (10041,5017)).
//
// This walks the room's own DS1 the way the wall pass of DRLGROOMTILE_LoadInitRoomTiles does -- the
// same window (nTileWidth/nTileHeight plus the unnarrowed edge, unless dwKillEdge kills it), the same
// cells (`pWallLayer`/`pTileTypeLayer` at the room's offset inside the map, row stride mapWidth+1, the
// exact array DRLGGRID_FillNewCellFlags exposes), the same test (a WALL_LEFT/RIGHT_EXIT that is
// bHidden with nTileStyle < 8) and the same LvlWarp record lookup by direction -- and returns the
// point DRLGROOMTILE_AddWarp records as that warp's UNIT_TILE preset: the cell times five, plus the
// record's own offsets.
//
// The caller asks it ONLY for a link node that carries no tile data (F54). Whether or not the room is
// built says nothing about that: every exit of Lut Gholein is a blank node in a fully built town (6 of
// 6 rooms built, node dataKind 0, `s421`), where the room-centre fallback below used to answer and read
// 11, 19 and 27 subtiles from the position DRLGROOMTILE_AddWarp recorded for the same warp on the same
// room. A node that DOES carry data keeps the reading above it -- that node and its data are the wall
// pass's own output, the same record this derivation reproduces from the room's DS1.
static bool sessionWarpPointFromDs1(const D2DrlgRoomStrc* room, int32_t warpId, int32_t* outX, int32_t* outY) {
    if (!room || room->nType != DRLGTYPE_PRESET || !room->pMaze || !room->pMaze->pMap || !room->pLevel) return false;
    const D2DrlgMapStrc* map = room->pMaze->pMap;
    const D2DrlgFileStrc* file = map->pFile;
    if (!file || !file->pWallLayer[0] || !file->pTileTypeLayer[0]) return false;
    bool killEdgeX = false, killEdgeY = false;
    if (map->pLvlPrestTxtRecord && map->pLvlPrestTxtRecord->dwKillEdge) {
        killEdgeX = room->nTileXPos + room->nTileWidth == map->pDrlgCoord.nPosX + map->pDrlgCoord.nWidth;
        killEdgeY = room->nTileYPos + room->nTileHeight == map->pDrlgCoord.nPosY + map->pDrlgCoord.nHeight;
    }
    const int countX = room->nTileWidth + (killEdgeX ? 0 : 1);
    const int countY = room->nTileHeight + (killEdgeY ? 0 : 1);
    const int stride = map->pDrlgCoord.nWidth + 1;
    const int baseX = room->nTileXPos - map->pDrlgCoord.nPosX;
    const int baseY = room->nTileYPos - map->pDrlgCoord.nPosY;
    for (int layer = 0; layer < file->nWallLayers; ++layer) {
        const int* wall = (const int*)file->pWallLayer[layer];
        const int* type = (const int*)file->pTileTypeLayer[layer];
        if (!wall || !type) continue;
        for (int oy = 0; oy < countY; ++oy) for (int ox = 0; ox < countX; ++ox) {
            const int cell = (baseY + oy) * stride + baseX + ox;
            const int nTileType = type[cell];
            if (nTileType != TILETYPE_WALL_LEFT_EXIT && nTileType != TILETYPE_WALL_RIGHT_EXIT) continue;
            const D2C_PackedTileInformation info{ (uint32_t)wall[cell] };
            if (!info.bHidden || info.nTileStyle >= 8) continue;
            if (DRLGWARP_GetWarpDestinationFromArray(room->pLevel, (uint8_t)info.nTileStyle) != warpId) continue;
            const D2LvlWarpTxt* rec = DRLGWARP_GetLvlWarpTxtRecordFromWarpIdAndDirection(room->pLevel, (uint8_t)info.nTileStyle,
                nTileType == TILETYPE_WALL_RIGHT_EXIT ? 'r' : 'l');
            if (!rec) continue;
            int px = ox, py = oy;
            DUNGEON_GameTileToSubtileCoords(&px, &py);
            *outX = 5 * room->nTileXPos + px + (int32_t)rec->dwOffsetX;
            *outY = 5 * room->nTileYPos + py + (int32_t)rec->dwOffsetY;
            return true;
        }
    }
    return false;
}
static std::vector<D2SessionWarpPreset> g_sessionWarpPresets;
static uint32_t g_sessionWorldHash;
static uint32_t g_sessionTerrainHash;
static bool g_sessionTerrainHashPending = false;   // the hash over g_sessionTiles is computed when read, not at every rebuild
static bool g_sessionLive, g_sessionMovePending;
static std::unordered_set<uint32_t> g_monInitPlayed; // 1.13c D2Client+0x23170: the per-unit played flag (client unit+0x80)
static std::vector<std::array<int32_t, 7>> g_pendingOverlaySounds;
static std::vector<std::array<int32_t, 7>> g_pendingOnsound; // state-on events found by the last drain, emitted by the next one
static std::unordered_set<uint64_t> g_prevOnsoundStates; // 1.13c D2Client+0x73760: state bits seen at the last sound drain (type<<40 | id<<8 | state)
static bool g_sessionResumeAttack;
extern int32_t g_playerSkillDoIds[8];
extern int32_t g_playerSkillDoCount;
static int32_t g_refusedDoSkill = 0, g_refusedDoWait = 0;   // a refused cast's client do sound, due in g_refusedDoWait drains
static int32_t g_cltMissileRow = 0;
// A player skill's client missile (Skills.txt cltmissile) plays its TravelSound at creation even when the server made no missile of its own.
void D2WEB_ClientMissileTravelSound(int32_t nCltMissile) {
    if (nCltMissile <= 0 || nCltMissile >= sgptDataTables->nMissilesTxtRecordCount || nCltMissile == 261) return;   // 0: a skill with no cltmissile column (missile 0 is the arrow, which plays no birth sound)
    const D2MissilesTxt* m = DATATBLS_GetMissilesTxtRecord(nCltMissile);
    if (m && m->wTravelSound > 0 && m->wTravelSound != 0xFFFF) g_cltMissileRow = m->wTravelSound;   // reaches the client one drain later, as the player's do sound does
}
static std::vector<std::array<int32_t, 7>> g_pendingSkillDo; // 1.13c D2Client+0xA2C90 wDoSound events, emitted one drain after the do
// s2420 watches one unit so its scheduled timer can be paired with proof that the event dispatcher
// actually invoked that unit's AITHINK callback. This is diagnostic only and does not change timing.
static uint32_t g_sessionAiThinkWatchUnit = UINT32_MAX;
static uint32_t g_sessionAiThinkWatchCount = 0;
static int32_t g_sessionAiThinkWatchLastFrame = -1;
struct D2SessionAiThinkPath { uint32_t unitId, target; int32_t points; uint32_t pathType; uint8_t direction, newDirection; };
static D2SessionAiThinkPath g_sessionAiThinkWatchPath = {};
static bool g_sessionAiThinkWatchHasPath = false;
static int32_t g_sessionAiThinkAttachFrame = -1;
static int32_t g_sessionAiThinkAttachMode = -1;
static int32_t g_sessionAiThinkAttachDeadline = -1;
static int32_t g_sessionAiThinkAttachRoomX = -1;
static int32_t g_sessionAiThinkAttachRoomY = -1;
static int32_t g_sessionAiThinkAttachRoomHasWarriv = 0;
static int32_t g_sessionAiThinkAttachPlayerX = -1;
static int32_t g_sessionAiThinkAttachPlayerY = -1;
extern "C" void D2WEB_RecordSessionAiThinkEvent(D2GameStrc* game, uint32_t unitId, uint32_t frame) {
    if (!game || game != g_game || !g_sessionLive || g_sessionAiThinkWatchUnit == UINT32_MAX ||
        unitId != g_sessionAiThinkWatchUnit) return;
    D2UnitStrc* unit = SUNIT_GetServerUnit(game, UNIT_MONSTER, (int32_t)unitId);
    if (!unit) return;
    ++g_sessionAiThinkWatchCount;
    g_sessionAiThinkWatchLastFrame = (int32_t)frame;
    g_sessionAiThinkWatchHasPath = false;
    if (unit->dwUnitType != UNIT_MONSTER || !unit->pDynamicPath) return;
    const D2DynamicPathStrc* p = unit->pDynamicPath;
    g_sessionAiThinkWatchPath = {unit->dwUnitId,
        (uint32_t)p->tTargetCoord.X | ((uint32_t)p->tTargetCoord.Y << 16),
        p->dwPathPoints, p->dwPathType, p->nDirection, p->nNewDirection};
    g_sessionAiThinkWatchHasPath = true;
}
// F128: tried generalising the Sewers trapdoor's proximity-sight to EVERY
// generated exit, driven by LvlWarp.txt's own SelectX/Y/DX/DY (D2LvlWarpTxt). Measured wrong twice:
// the fields are signed and can be negative (e.g. level 40 warpId 20 -> select=(-25,-95)+(70,130)),
// so they are NOT the absolute destination-level tile rect the scenario's four retail-captured
// targets sit in -- they read as an offset from something else entirely (dwExitWalkX/Y did not
// explain it either). D2Client's own selection logic is out of scope (not RE'd) per the scenario's
// own comment; deriving it needs a capture, which is more than one cycle. Smaller-
// change default taken instead: kept the scenario's explicit, retail-verified four-point call
// (`d2_session_sight_point`, per-scenario) rather than land a wrong "generic" rect read. The helper
// below is still factored out so a future capture-backed generalisation can reuse it.
static int sessionSightPointAct(D2DrlgActStrc* act, int levelId, int tileX, int tileY, int on,
                                bool explicitHint, D2ActiveRoomStrc* hintRoom);
static uint32_t g_sessionNavigationHash;
static int32_t g_sessionTargetX, g_sessionTargetY;
// D2Client's run/walk toggle (see the exports near d2_session_command_move for why it lives here and
// not on the unit): the client owns it, and it is what decides which of retail's two move packets a
// command sends. Off is retail's default for a new game.
static bool g_sessionRunToggle = false;

extern "C" void D2WEB_ResetLastDialogPacket();
// The NPC "!" latch the send shim keeps from 0x8A / type-1 0x28 (game_net_stubs.cpp).
extern "C" void D2WEB_ResetNpcAlerts();
extern "C" int D2WEB_NpcAlertActive(int32_t guid);
extern "C" int D2WEB_CopyLastDialogPacket(D2GSPacketSrv27* pPacket);
extern "C" int D2WEB_TakeObjectMessage(int32_t* pGuid, uint32_t* pStringId);
extern "C" void D2WEB_ResetLastQuestStatusPacket();
extern "C" int D2WEB_CopyLastQuestStatusPacket(D2GSPacketSrv5D* pPacket);

static uint32_t sessionHashBytes(uint32_t hash, const void* data, size_t size) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < size; ++i) { hash ^= bytes[i]; hash *= 16777619u; }
    return hash;
}

// d2cmp_tilelib.cpp owns these nodes.  They are deliberately native helper objects rather than
// retail packed structs, so repeat the private layout here instead of using D2CMP.h's packed
// 32-bit DLL declarations (whose node pointer begins four bytes earlier under wasm64).
struct D2SessionTileRef { D2TileLibraryEntryStrc* tile; D2SessionTileRef* next; };
struct D2SessionTileNode {
    int32_t style, sequence, type;
    D2SessionTileRef* refs;
    D2SessionTileNode* next;
};
struct D2SessionTileHash { D2SessionTileNode* nodes[128]; };

// The DT1 files the CURRENT level's rooms actually loaded, in first-seen order. A tile's
// `librarySlot` indexes THIS, not LvlTypes: the engine's own slot number is per-room (it indexes
// that room's pTiles[], holding only what that room loaded, in its own order), so it cannot name a
// file on its own. Preset rooms pull DT1s through their DS1 that LvlTypes never lists, which is why
// mapping through the level's table left those levels with no floor.
extern "C" const char* D2WEB_TileLibraryNameFromHash(const D2TileLibraryHashStrc* pHash);
static std::vector<std::string> g_sessionTileLibraries;

static uint32_t sessionTileLibraryFile(const D2TileLibraryHashStrc* pHash) {
    const char* name = pHash ? D2WEB_TileLibraryNameFromHash(pHash) : nullptr;
    if (!name) return UINT32_MAX;
    for (size_t i = 0; i < g_sessionTileLibraries.size(); ++i)
        if (g_sessionTileLibraries[i] == name) return (uint32_t)i;
    g_sessionTileLibraries.emplace_back(name);
    return (uint32_t)(g_sessionTileLibraries.size() - 1);
}

static void sessionTileLibraryIdentity(const D2DrlgRoomStrc* roomEx, const D2TileLibraryEntryStrc* tile,
        uint32_t& slotOut, uint32_t& indexOut) {
    slotOut = UINT32_MAX; indexOut = UINT32_MAX;
    if (!roomEx || !tile) return;
    const uint32_t bucket = (uint32_t)(2 * (uint8_t)tile->nType - (uint8_t)tile->nSequence + (uint8_t)tile->nStyle) & 0x7Fu;
    for (uint32_t slot = 0; slot < 32 && roomEx->pTiles[slot]; ++slot) {
        D2SessionTileHash* hash = reinterpret_cast<D2SessionTileHash*>(roomEx->pTiles[slot]);
        for (D2SessionTileNode* node = hash->nodes[bucket]; node; node = node->next) {
            if (node->type != tile->nType || node->style != tile->nStyle || node->sequence != tile->nSequence) continue;
            uint32_t index = 0;
            for (D2SessionTileRef* ref = node->refs; ref; ref = ref->next, ++index) {
                // Report WHICH FILE, not which of this room's slots.
                if (ref->tile == tile) { slotOut = sessionTileLibraryFile(roomEx->pTiles[slot]); indexOut = index; return; }
            }
        }
    }
}

static uint32_t g_sessionTilesSkipped = 0;
static bool g_sessionTilesStale = true, g_sessionTilesRollTrace = false, g_sessionTilesCounting = true;
// Tiles of the OTHER levels' active rooms:
// across an open seam retail draws the neighbouring level's rooms (s187: the Blood Moor's floors on the
// Cold Plains' first frame). Kept apart from g_sessionTiles, with their own library list, so the
// current level's tile list, its library slots and the terrain hash stay what the tile rows pin.
static std::vector<D2SessionTile> g_sessionSeamTiles;
static std::vector<std::string> g_sessionSeamLibraries;

// A preset's roof POP, as the engine's own client frame leaves it on the tiles (F44). D2Common's
// DUNGEON_UpdatePops -> DRLGPRESET_UpdatePops toggles, for every tile of the DT1 STYLE a pop names
// inside that pop's region, `D2DrlgTileDataStrc::unk0x24` bit 1 -- the "this tile is pop-affected"
// mark -- and writes the direction it went in the tile's green/blue pair: entering the region (the
// activate call, nCellFlags 0) sets green 255 / blue 0, leaving it (nCellFlags 1) green 0 / blue 255.
// A tile the pop has hidden is the one retail does not draw, so the pair is the state and the client
// drops the tile (session-terrain's `pop === 1`). Retail's own frames pin the direction rather than
// this reading: act4-tyrael's camera stands the hero INSIDE the Fortress plaza's pop region and has no
// roof draw at all, while act4-fortress's stands him outside it and draws every roof.
static uint32_t sessionTilePop(const D2DrlgTileDataStrc& tileData) {
    if (!(tileData.unk0x24 & 2)) return 0;
    return tileData.nGreen == 255 ? 1u : 2u;
}

static void appendSessionTiles(const D2DrlgRoomStrc* roomEx, uint32_t roomOrdinal, uint32_t layer,
        const D2DrlgTileDataStrc* tiles, int count) {
    if (!tiles || count <= 0) return;
    for (int i = 0; i < count; ++i) {
        const D2DrlgTileDataStrc& tileData = tiles[i];
        if (!tileData.pTile) { if (g_sessionTilesCounting) ++g_sessionTilesSkipped; continue; }
        const D2TileLibraryEntryStrc& tile = *tileData.pTile;
        D2SessionTile out = {};
        out.room = roomOrdinal; out.layer = layer; out.ordinal = (uint32_t)i;
        out.orientation = tile.nType; out.mainIndex = tile.nStyle; out.subIndex = tile.nSequence;
        out.frameIndex = tile.nRarity_Frame; out.material = tile.nFlags;
        out.tileX = tileData.nPosX; out.tileY = tileData.nPosY;
        out.worldX = roomEx->nTileXPos + tileData.nPosX;
        out.worldY = roomEx->nTileYPos + tileData.nPosY;
        out.screenX = tileData.nWidth; out.screenY = tileData.nHeight;
        out.width = tile.nWidth; out.height = tile.nTotalHeight;
        out.flags = tileData.dwFlags;
        out.entryHint = tileData.unk0x10;
        out.pop = sessionTilePop(tileData);
        sessionTileLibraryIdentity(roomEx, tileData.pTile, out.librarySlot, out.libraryIndex);
        g_sessionTiles.push_back(out); g_sessionTileEntry.push_back(D2WEB_TileEntryFileIndex(tileData.pTile));
        {
            // Retail: unk0x10 = the coord list occupying the tile's cell of the room's logical grid, walls only, and 0
            // for a room whose one list covers it (HAS_COORD_LIST). This port keeps only the occupancy bit in the grid
            // (entryHint) and the pointer in ppCoordListByCell, so the same lookup goes through the sidecar.
            D2SessionTileCoord c = { -1, 0, 0, 0, 0 };
            const D2DrlgLogicalRoomInfoStrc* info = roomEx->pLogicalRoomInfo;
            if (layer == 1 && out.entryHint && info && !(info->dwFlags & DRLGLOGIC_ROOMINFO_HAS_COORD_LIST) && info->ppCoordListByCell
                    && tileData.nPosX >= 0 && tileData.nPosY >= 0 && tileData.nPosX < info->pIndexY.nWidth && tileData.nPosY < info->pIndexY.nHeight) {
                if (const D2RoomCoordListStrc* list = info->ppCoordListByCell[tileData.nPosY * info->pIndexY.nWidth + tileData.nPosX])
                    c = { list->nIndex, list->pBox[0].nPosX, list->pBox[0].nPosY, list->pBox[0].nWidth, list->pBox[0].nHeight };
            }
            g_sessionTileCoord.push_back(c);
        }
        if (g_d2webRollTraceEnabled) {
            auto it = g_d2webRollTrace.find(&tileData);
            g_sessionTileRolls.push_back(it != g_d2webRollTrace.end() ? it->second : D2WebRollTrace{ UINT32_MAX - 1, 0, 0, 0, 0, 0 });
        }
    }
}

// d2_session_tiles_skipped counts, at every world rebuild, the tile records with no library entry
// in the rooms the tile lists are built from (the level's, then the seam levels'), as it did when
// every rebuild built them.
static void countSessionTilesSkipped() {
    if (!g_sessionLive || !g_lastLevel) return;
    auto count = [](const D2DrlgTileDataStrc* tiles, int n) { for (int i = 0; tiles && i < n; ++i) if (!tiles[i].pTile) ++g_sessionTilesSkipped; };
    auto countLevel = [&](const D2DrlgLevelStrc* level) {
        for (D2DrlgRoomStrc* roomEx = level->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
            const D2ActiveRoomStrc* room = roomEx->pRoom;
            if (!room || !room->pRoomTiles) continue;
            count(room->pRoomTiles->pFloorTiles, room->pRoomTiles->nFloors);
            count(room->pRoomTiles->pWallTiles, room->pRoomTiles->nWalls);
            count(room->pRoomTiles->pRoofTiles, room->pRoomTiles->nRoofs);
        }
    };
    countLevel(g_lastLevel);
    for (D2DrlgLevelStrc* level = g_lastLevel->pDrlg ? g_lastLevel->pDrlg->pLevel : nullptr; level; level = level->pNextLevel)
        if (level != g_lastLevel) countLevel(level);
}

// D2 Client D11: the server creates a monster-dropped item already in IMODE_ONGROUND (nSpawnType 3), so its
// UNITS_ChangeAnimMode(ONGROUND) is a no-op and the clock stays 0/0 (frame 0 of a gold pile's bucket). Retail's
// client builds its copy from the 0x9C packet (STORED -> ONGROUND), running UNITS_SetAnimStartFrame: 4096/4352.
// The server unit is left alone (D2Game vectors compare it); only the client view is corrected.
template <class P> static void clientLandedGroundClock(const D2UnitStrc* item, P& out) {
    if (item->dwAnimMode == IMODE_ONGROUND && item->dwFrameCountPrecise == 0) { out.framePrecise = 4096; out.frameCountPrecise = 4352; }
}
// D25: retail's 0x9C "dropped to ground" builds the client item in IMODE_DROPPING and runs the flippy at
// wAnimSpeed 256 (UNITS_SetAnimStartFrame, Units.cpp IMODE_DROPPING: 17 frames, one per game frame) before it
// lands. The server creates the item already ONGROUND, so the client view replays that: an item first seen after
// the session's first snapshot is presented as IMODE_DROPPING with the clock at (frames since first seen) << 8
// for its first 17 frames. Server unit untouched. ponytail: "first seen after priming" stands in for the
// packet's action byte (a ground item generated with its room would replay the flip once).
static bool g_sessionInTick = false;
template <class P> static void clientDropFlip(const D2UnitStrc* item, P& out) {
    static const D2GameStrc* seenGame = nullptr; static d2::drop_flip::Clock flipClock;
    if (!g_game) return;
    if (seenGame != g_game) { seenGame = g_game; flipClock.reset(g_game->dwGameFrame); }   // primed by the first item of any mode, so an equipped item later dropped is "new"
    // A command's snapshot precedes the frame the client sees it in (the tick that follows); an item the tick itself creates (a monster's gold) is seen a frame after its packet.
    uint32_t precise = 0;
    if (!flipClock.step(item->dwUnitId, item->dwAnimMode == IMODE_ONGROUND, g_game->dwGameFrame, g_sessionInTick, precise)) return;
    out.mode = IMODE_DROPPING; out.framePrecise = precise; out.frameCountPrecise = d2::drop_flip::kFrameCountPrecise;
}
// The inventory graphic retail's panel draws for an item (F481). ItemsTxt.invfile is only the FIRST
// candidate: a unique item (or a set item) whose quality-specific file is non-empty uses THAT, and an
// item whose ItemTypes row carries variable inventory graphics -- the seven types with a non-zero
// `varinvgfx` are Ring, Amulet, Charm/Small/Medium/Large Charm and Jewel -- uses the variant its own
// `nInvGfxIdx` picks (D2Common #10726/#10789/#10790, read from the save's 3-bit field, Items.cpp:5075).
// The chain is fixed by the geared Fortress capture (`cov-a4g-inv`, F481/s1720): the unique full helm
// drew `invfhlu` where ItemsTxt.invfile is `invfhl`; the unique kite shield `invkitu`/`invkit`; the
// unique amulet `invamu1`/`invamu` and a ring `invrin5`/`invrin` (both ItemsTxt.uniqueinvfile EMPTY, so
// the ItemTypes table's own `invgfx1..6`); while the unique light boots (`invlbt`), the normal plate
// (`invplt`), belt (`invtbl`), gloves (`invtgl`) and the rare war axe (`invwax`) are their base
// ItemsTxt.invfile -- so uniqueness alone does not change the graphic, only a non-empty quality file
// does. No type carries BOTH a non-empty uniqueinvfile and a varinvgfx, so the two branches never
// collide (checked over Items.txt/ItemTypes.txt); unique/set is tested first, matching the columns'
// own meaning. This engine used to read szInvFile alone, which drew every unique helm/shield and every
// ring/amulet/charm/jewel with the base art.
static const char* sessionItemInventoryFile(D2UnitStrc* item, const D2ItemsTxt* txt) {
    const int quality = ITEMS_GetItemQuality(item);
    if (quality == ITEMQUAL_UNIQUE && txt->szUniqueInvFile[0]) return txt->szUniqueInvFile;
    if (quality == ITEMQUAL_SET && txt->szSetInvFile[0]) return txt->szSetInvFile;
    if (ITEMS_GetVarInvGfxCount(item)) return ITEMS_GetVarInvGfxString(item, ITEMS_GetVarGfxIndex(item));
    return txt->szInvFile;
}

// 1.13c D2Client+0x95140: the gates (identified, socketed flag, sockets > 0) and the
// footprint placement; the gem in socket n is the n-th item of the item's own inventory.
static void appendSessionItemSockets(D2UnitStrc* item, const D2SessionItemPresentation& pres) {
    // Retail has no inventory gate: the first-child walk (D2Common #10460) is null-safe, and an empty socketed item
    // (one loaded from a save, never socketed in play) carries no inventory yet still draws its empty sockets.
    if (!ITEMS_CheckItemFlag(item, IFLAG_IDENTIFIED, __LINE__, __FILE__) ||
        !ITEMS_CheckItemFlag(item, IFLAG_SOCKETED, __LINE__, __FILE__)) return;
    const int sockets = ITEMS_GetSockets(item);
    D2SessionItemSockets rec = {};
    if (!d2client_sockets::layout(sockets, pres.invWidth, pres.invHeight, rec.off)) return;
    rec.id = item->dwUnitId; rec.count = (uint8_t)sockets;
    for (D2UnitStrc* gem = INVENTORY_GetFirstItem(item->pInventory); gem && rec.gems < sockets; gem = INVENTORY_GetNextItem(gem)) {
        D2ItemsTxt* txt = DATATBLS_GetItemsTxtRecord(gem->dwClassId);
        if (txt) memcpy(rec.gem[rec.gems], sessionItemInventoryFile(gem, txt), sizeof rec.gem[0]);
        rec.gems++;
    }
    g_sessionItemSockets.push_back(rec);
}

static void appendSessionItemPresentation(D2UnitStrc* item) {
    if (!item || item->dwUnitType != UNIT_ITEM) return;
    D2SessionItemPresentation out = {};
    out.id = item->dwUnitId; out.classId = item->dwClassId; out.mode = item->dwAnimMode;
    out.quality = ITEMS_GetItemQuality(item); out.itemFlags = ITEMS_GetItemFlags(item);
    out.framePrecise = item->nSeqCurrentFramePrecise; out.frameCountPrecise = item->dwFrameCountPrecise;
    clientLandedGroundClock(item, out);
    clientDropFlip(item, out);
    if (item->pStaticPath) {
        out.x = item->pStaticPath->tGameCoords.nX; out.y = item->pStaticPath->tGameCoords.nY;
        out.gridX = (int16_t)item->pStaticPath->tGameCoords.nX;
        out.gridY = (int16_t)item->pStaticPath->tGameCoords.nY;
    }
    out.invPage = ITEMS_GetInvPage(item); out.bodyLoc = ITEMS_GetBodyLocation(item);
    if (D2ItemsTxt* txt = DATATBLS_GetItemsTxtRecord(item->dwClassId)) {
        out.code = txt->dwCode;
        memcpy(out.flippyFile, txt->szFlippyFile, sizeof out.flippyFile);
        memcpy(out.invFile, sessionItemInventoryFile(item, txt), sizeof out.invFile);
        out.invWidth = txt->nInvWidth; out.invHeight = txt->nInvHeight;
    }
    g_sessionItemPresentations.push_back(out);
    appendSessionItemSockets(item, out);
    uint8_t colour = 0;
    g_sessionItemColours.push_back(ITEMS_GetColor(nullptr, item, &colour, 0) ? (uint8_t)(colour + 1) : 0xFF);
    g_sessionItemInvColours.push_back(D2CMP_MixPaletteKey(ITEMS_GetColor(nullptr, item, &colour, 1)));
}

static void syncSessionLevelFromPlayer() {
    if (!g_targetPlayer) return;
    D2ActiveRoomStrc* room = UNITS_GetRoom(g_targetPlayer);
    if (!room || !room->pDrlgRoom || !room->pDrlgRoom->pLevel) return;
    const int level = DUNGEON_GetLevelIdFromRoom(room);
    if (level <= 0) return;
    g_currentLevel = level;
    g_currentAct = DRLG_GetActNoFromLevelId(level);
    g_lastLevel = room->pDrlgRoom->pLevel;
    g_targetRoom = room;
}

static void syncSessionClientMapFromPlayer();
static std::unordered_set<D2DrlgRoomStrc*> g_sessionClientRoomsSeen;   // D2Client+0x440C0's seen bit
static void sessionClientMonsterModeReset();
static uint32_t g_sessionClientFrame = 0;   // the client frames run since the session began (the frame tail counts them)

static void resetSessionClientMapMirror() {
    if (g_sessionClientPlayerUnit) UNITS_FreeUnit(g_sessionClientPlayerUnit);
    g_sessionClientPlayerUnit = nullptr;
    d2client::weather::reset(); d2client_ambient::reset(); d2_client_monster_idle_reset(); sessionClientMonsterModeReset(); d2_client_attack_roll_reset(); g_sessionClientRoomsSeen.clear(); g_sessionClientFrame = 0; d2client_hurricane::reset(); d2client_vine::reset(); d2client_nova::reset(); d2client_clmissile::reset(); d2client_blizzard::reset();
    if (g_sessionClientActOwned) d2_drlg_act_end();
    g_sessionClientActOwned = false;
    g_sessionClientMapStarted = false;
    g_sessionClientAct = -1;
    g_sessionClientCurrentRoom = nullptr;
    g_sessionClientNearRooms.clear();
    g_sessionClientMapEvents.clear();
    g_sessionClientMapEventsDropped = 0;
    g_sessionClientPlayerInitSeed = 0;
}

static bool initializeSessionClientPlayerSeed() {
    if (g_sessionClientPlayerUnit) return true;
    if (!g_targetPlayer || !g_targetPlayer->pDynamicPath) return false;
    D2UnitStrc* clientUnit = UNITS_AllocUnit(nullptr, UNIT_PLAYER);
    if (!clientUnit) return false;
    const int x = CLIENTS_GetUnitX(g_targetPlayer);
    const int y = CLIENTS_GetUnitY(g_targetPlayer);
    clientUnit->dwClassId = g_targetPlayer->dwClassId;
    clientUnit->dwUnitId = g_targetPlayer->dwUnitId;
    if (!d2_session_client_cunit_seed_init(&clientUnit->pSeed, &clientUnit->dwInitSeed,
            x, y, 1)) {
        UNITS_FreeUnit(clientUnit);
        return false;
    }
    g_sessionClientPlayerInitSeed = clientUnit->dwInitSeed;
    g_sessionClientPlayerUnit = clientUnit;
    return true;
}

// Initialize fields from a caller-owned client CUnit using the already-active client Act.
// The caller supplies that CUnit's seed and dwInitSeed fields, never a D2Game/server seed.
// playerCurrent models FUN_6FAC91E0's extra roll when a fresh player becomes current.
extern "C" EMSCRIPTEN_KEEPALIVE int d2_session_client_cunit_seed_init(
        D2SeedStrc* clientSeed, uint32_t* unitInitSeed, int x, int y, int playerCurrent) {
    if (!clientSeed || !unitInitSeed) return 0;
    uint32_t level = 0, roomTileX = 0, roomTileY = 0, initSeed = 1;
    if ((x != 0 || y != 0) &&
        (!d2_drlg_client_unit_context(x, y, 1, &level, &roomTileX, &roomTileY, &initSeed) || !level)) {
        return 0;
    }
    *unitInitSeed = initSeed;
    SEED_InitLowSeed(clientSeed, static_cast<int>(initSeed));
    if (playerCurrent) SEED_RollRandomNumber(clientSeed);
    return 1;
}

namespace d2::realm {

FireballHitCounts createFireballHitChildren(D2UnitStrc* sourceFireball,
        D2UnitStrc* currentClientPlayer, FireballHitChildFactory factory, void* context) {
    FireballHitCounts empty{};
    if (!sourceFireball || sourceFireball->dwUnitType != UNIT_MISSILE ||
        !sourceFireball->pMissileData ||
        !sourceFireball->pDynamicPath || !currentClientPlayer ||
        currentClientPlayer->dwUnitType != UNIT_PLAYER || !currentClientPlayer->pDynamicPath || !factory) {
        return empty;
    }

    D2MissilesTxt* sourceTxt = DATATBLS_GetMissilesTxtRecord(sourceFireball->dwClassId);
    D2MissilesTxt* childTxt = DATATBLS_GetMissilesTxtRecord(265);
    // 1.13c D2Client+0xBC1A0 (hit function 1): any row with that hit function and a first hit
    // sub-missile; the child is always class 265 whatever the row names.
    if (!sourceTxt || !childTxt || sourceTxt->wCltHitFunc != 1 ||
        static_cast<int16_t>(sourceTxt->wCltHitSubMissile[0]) < 0) return empty;

    D2DrlgActStrc* clientAct = d2_drlg_client_act();
    if (!clientAct) return empty;
    const int playerX = currentClientPlayer->pDynamicPath->tGameCoords.wPosX;
    const int playerY = currentClientPlayer->pDynamicPath->tGameCoords.wPosY;
    D2ActiveRoomStrc* currentRoom = DUNGEON_FindRoomBySubtileCoordinates(clientAct, playerX, playerY);
    if (!currentRoom) return empty;

    D2ActiveRoomStrc** nearRooms = nullptr;
    int nearCount = 0;
    DUNGEON_GetAdjacentRoomsListFromRoom(currentRoom, &nearRooms, &nearCount);

    auto isInNativeClientNeighborhood = [&](int x, int y) {
        D2ActiveRoomStrc* candidate = DUNGEON_FindRoomBySubtileCoordinates(clientAct, x, y);
        return isCurrentOrNearRoom(candidate, currentRoom, nearRooms, nearCount);
    };

    const int centerX = sourceFireball->pDynamicPath->tGameCoords.wPosX;
    const int centerY = sourceFireball->pDynamicPath->tGameCoords.wPosY;
    const int sourceLevel = sourceFireball->pMissileData->nLevel;
    return enumerateFireballHitChildren(sourceFireball->pSeed, centerX, centerY,
        static_cast<int>(sourceTxt->dwCltHitPar[0]), sourceTxt->dwCltHitPar[1],
        childTxt->dwRandStart, sourceFireball->dwClassId, sourceLevel,
        isInNativeClientNeighborhood,
        [&](const FireballHitChild& child) { return factory(sourceFireball, child, context); });
}

} // namespace d2::realm

static bool startSessionClientMapMirror() {
    if (!g_sessionLive || !g_game || !g_targetPlayer || g_currentAct < 0 || g_currentAct > 4) return false;
    const int townLevel = DUNGEON_GetTownLevelIdFromActNo((uint8_t)g_currentAct);
    if (!d2_drlg_client_act_begin(g_gameSeed, g_gameDiff, g_currentAct + 1, townLevel)) return false;
    g_sessionClientActOwned = true;
    syncSessionClientMapFromPlayer();
    return g_sessionClientMapStarted;
}

extern "C" EMSCRIPTEN_KEEPALIVE int d2_session_client_map_start() {
    if (g_sessionClientActOwned) return g_sessionClientMapStarted ? 1 : 0;
    return startSessionClientMapMirror() ? 1 : 0;
}

static bool sameSessionClientRoom(const SessionClientRoomKey& a, const SessionClientRoomKey& b) {
    return a.roomEx == b.roomEx && a.level == b.level && a.x == b.x && a.y == b.y;
}

static std::vector<SessionClientRoomKey> sessionClientNearRooms(D2ActiveRoomStrc* current) {
    std::vector<SessionClientRoomKey> result;
    if (!current) return result;
    D2ActiveRoomStrc** rooms = nullptr;
    int32_t count = 0;
    DUNGEON_GetAdjacentRoomsListFromRoom(current, &rooms, &count);
    if (!rooms || count <= 0) return result;
    result.reserve((size_t)count);
    for (int32_t i = 0; i < count; ++i) {
        D2ActiveRoomStrc* active = rooms[i];
        D2DrlgRoomStrc* roomEx = active ? active->pDrlgRoom : nullptr;
        if (!roomEx || !roomEx->pLevel) continue;
        result.push_back({ roomEx, roomEx->pLevel->nLevelId, roomEx->nTileXPos, roomEx->nTileYPos });
    }
    return result;
}

static bool sessionClientMapMutation(bool reveal, const SessionClientRoomKey& room, bool hasPlayer) {
    const int playerLevel = hasPlayer ? g_currentLevel : 0;
    const int playerX = hasPlayer && g_targetPlayer ? CLIENTS_GetUnitX(g_targetPlayer) : 0;
    const int playerY = hasPlayer && g_targetPlayer ? CLIENTS_GetUnitY(g_targetPlayer) : 0;
    const int ok = reveal
        ? d2_drlg_client_map_reveal(room.level, room.x, room.y, hasPlayer, playerLevel, playerX, playerY)
        : d2_drlg_client_map_hide(room.level, room.x, room.y, hasPlayer, playerLevel, playerX, playerY);
    if (ok && g_sessionClientMapEvents.size() / 4 < kSessionClientMapEventCapacity) {
        g_sessionClientMapEvents.push_back(reveal ? 1 : 0);
        g_sessionClientMapEvents.push_back(room.level);
        g_sessionClientMapEvents.push_back(room.x);
        g_sessionClientMapEvents.push_back(room.y);
    } else if (ok && g_sessionClientMapEventsDropped != UINT32_MAX) {
        ++g_sessionClientMapEventsDropped;
    }
    return ok != 0;
}

// D2Game 1.13c sends one explicit spawn-room MapReveal before adding the client to that room. The
// ensuing ordered near-room diff includes the center again. On later room changes it reveals
// new-only rooms in the new #10383 list order, then hides old-only rooms in the old list order.
// This mirrors those map packets into the bClient Act; it never edits the server Act's sight state.
static void syncSessionClientMapFromPlayer() {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_sessionClientActOwned) return;
    D2ActiveRoomStrc* current = UNITS_GetRoom(g_targetPlayer);
    if (!current || !current->pDrlgRoom || !current->pDrlgRoom->pLevel) return;
    const int act = DRLG_GetActNoFromLevelId(DUNGEON_GetLevelIdFromRoom(current));
    const bool newAct = !g_sessionClientMapStarted || act != g_sessionClientAct;
    if (!newAct && current == g_sessionClientCurrentRoom) return;
    if (newAct) {
        if (g_sessionClientMapStarted) {
            // The native LoadAct boundary replaces the prior client Act. The new act's first packet
            // is its own spawn-room reveal; old-act rooms are discarded with that Act, not hidden.
            const int townLevel = DUNGEON_GetTownLevelIdFromActNo((uint8_t)act);
            if (!d2_drlg_client_act_begin(g_gameSeed, g_gameDiff, act + 1, townLevel)) return;
        }
        g_sessionClientAct = act;
        g_sessionClientCurrentRoom = nullptr;
        g_sessionClientNearRooms.clear();
        SessionClientRoomKey center{ current->pDrlgRoom,
            current->pDrlgRoom->pLevel->nLevelId,
            current->pDrlgRoom->nTileXPos, current->pDrlgRoom->nTileYPos };
        if (!sessionClientMapMutation(true, center, false)) return;
        g_sessionClientMapStarted = true;
    }
    // In the recovered join order, the explicit center reveal builds the active room before the
    // local client CUnit constructor rolls its dwInitSeed. Seed the independent client carrier
    // before revealing the ordered near-room batch.
    if (!initializeSessionClientPlayerSeed()) return;

    const std::vector<SessionClientRoomKey> next = sessionClientNearRooms(current);
    for (const SessionClientRoomKey& candidate : next) {
        const bool existed = std::any_of(g_sessionClientNearRooms.begin(), g_sessionClientNearRooms.end(),
            [&](const SessionClientRoomKey& old) { return sameSessionClientRoom(old, candidate); });
        if (!existed && !sessionClientMapMutation(true, candidate, true)) return;
    }
    for (const SessionClientRoomKey& old : g_sessionClientNearRooms) {
        const bool remains = std::any_of(next.begin(), next.end(),
            [&](const SessionClientRoomKey& candidate) { return sameSessionClientRoom(old, candidate); });
        if (!remains && !sessionClientMapMutation(false, old, true)) return;
    }
    g_sessionClientCurrentRoom = current;
    g_sessionClientNearRooms = next;
}

// LEVEL_WarpUnit transfers the player to its destination room, but the normal server
// transition has already made that level's rooms visible and run their activation path.
// The headless session owns that second half explicitly so destination monster regions,
// presets, and quest counters are created by the recovered D2Game routine as well.
// The order the destination level's rooms are built in decides which room of a shared edge looks its
// tiles up (and takes the rarity roll on its own seed). Retail builds them as its activation wave
// reaches them: the room the player lands in, then its near rooms in near-list order, then theirs
// (retail's in-play pick stream for the Den of Evil: 1524,1024, then 1516,1016
// 1524,1016 1532,1016 1532,1024 1524,1032 1532,1032, then 1508,1008 1516,1008 ...). Building in the
// level's room-list order instead took other rooms' rolls (s209: 9 floors). The whole level is still
// built here (its monster regions and quest counters need it: A1Q1), breadth-first from the player's
// room over the near lists; rooms the wave never reaches follow in list order.
// d2_session_retail_level_change(0) restores the list order for a before/after run.
static int g_sessionRetailLevelChange = 1;
extern "C" EMSCRIPTEN_KEEPALIVE void d2_session_retail_level_change(int on) { g_sessionRetailLevelChange = on; }
// retail's client activates a level FROM THE PLAYER'S ROOM and follows the player
// -- DUNGEON_SetClientIsInSight / DUNGEON_ChangeClientRoom propagate DRLGACTIVATE status outward
// (the room the player is in CLIENT_IN_ROOM, its near ring CLIENT_IN_SIGHT and built at once, the
// ring beyond it CLIENT_OUT_OF_SIGHT and built one at a time by DRLGACTIVATE_Update), and rooms left
// behind lose their status and are freed. The session built EVERY room of a new level up front, which
// retail never does: a room's shared-edge cells merge into a live neighbour and its InitRoomGrids
// reseed reads dwInitSeed, so a level built whole is the right tiles for a history no player has.
// `d2_session_activation_wave(1)` turns that on (and 0 restores the eager build), so a run measures
// the two against each other; the census is d2_session_room_stats and s59 does the A/B.
//
// It was 0 until it was fixed because a level entered by the wave read its exits' coordinates WRONG
// -- the warp list took them from a room's activated tile data, so an unbuilt exit room answered with
// another room's exit to the same level (on Harem Level 1 the warp to 51 read (10042,5082) instead of
// (10041,5017)) and s000b's walk to the Sewers stairs stalled 300 rounds, its first failing step
// moving 19 -> 12. F52 fixed that (an exit's position is generation-time DRLG data:
// sessionWarpPointFromDs1, and the level's room links built up front).
//
// DEFAULT 1. The flip waited on s88-session-act-routes, which the wave broke on act I's
// route at "27 -> 28 via seam" (command_seam(28) returned 0, the player left at (15002,4941) still in
// level 27). The gap was in that command's own walk, not in the wave: `d2_session_command_seam`
// builds the two rooms a crossing needs, and a room built that way has its TILES and nothing else --
// its preset units (the door at the seam among them) and the objects it holds spawn on the frame
// that follows, which is also how the wave builds a room (DRLGACTIVATE_RoomEx_EnsureHasRoom, then the
// pending-rooms pass on the next frame). The search read the room before that frame, saw no door,
// stood ON the seam column, and its walk across was refused with no path (MOVE_NO_PATH), candidate
// after candidate. The eager build had every room populated long before, so it stood two subtiles
// back, opened the door as a player does and crossed. The command now runs that one frame for a room
// it built (`if (builtHere || builtDest) d2_session_tick(1)`), and s420-probe-seam-seamwalk measures
// the result on ONE engine: with the wave on, act I's route to level 27 and the seam walk into 28
// both cross, at the same subtile as the eager build, with level 27 at 8 of its 35 rooms built where
// the eager build has 35 -- and s88-session-act-routes passes all six acts' routes with the switch
// compiled at 1. `d2_session_activation_wave(0)` still restores the eager build for an A/B, which is
// what s59, s997 and s420 do.
static int g_sessionActivationWave = 1;
extern "C" EMSCRIPTEN_KEEPALIVE void d2_session_activation_wave(int on) { g_sessionActivationWave = on ? 1 : 0; }
// Anchor class for the player fixture: any monster (0xFFFFFFFF) or a specific class, so a
// session can be created beside one town NPC the way the retail capture's fixture is.
static uint32_t g_sessionAnchorClass = 0xFFFFFFFF;
// F64: `d2_monspawn_level` is BOTH the retail-vector fixture and the path a session is created
// through (`d2_session_create_for_class`), and the two want opposite things from it.
//
// A fixture that REPLAYS retail's own client sight stream (`d2_session_sight_point`) must not have
// the level built for it: the replay IS the comparison, and a create that walks the level's room
// list marks rooms retail had not reached yet, so the replay's calls land on rooms that already
// exist and measure nothing (s247). d2_session_create_stream(1) is that fixture's switch, and it is
// OPT-IN.
//
// Every other fixture reads the level it was created IN, right after the create and without walking
// it: an object's presentation (the five seals, the Compelling Orb, an orifice, a journal, a
// portal), a room's tiles in the draw list, the room-seed roll behind an item, the world hash of an
// act. `d2_session_create` is a fixture, not a retail path -- retail's client reaches a wilderness
// level through the WARP path, which F53 has driven by the activation wave since then -- so
// the eager build stays the default and the streaming create is the exception. Measured on the
// merged branch (an earlier change, D2_WORKERS=1, the create's own room count against the level's built
// whole): Cold Plains 16, Andariel's Lair 4, the Chaos Sanctuary 28 of 225, Travincal 9 of 64. With
// that cutoff the merge's sweep had 35 rows red besides this branch's own s247, every one a row
// that reads the level it was created in; the per-frame wave replaces the missing rooms at about a
// room every five frames, far slower than the 8 ticks a draw-list row runs.
//
// STATUS (F63): lazy is OPT-IN via `g_sessionLazy` (default 0, whole-level build); the default flip is pending
// s000e s247 s249 s330 s59 s830, still red with lazy on. Fixtures that need the whole level call stream(0).
//
// Set only by `d2_session_create_stream`, so s240-s242 and s05-s08 call `d2_monspawn_level`
// directly and build exactly what they always did.
static int g_monspawnSessionCreate = 0;  // set only around a session create; direct d2_monspawn_level callers stay eager
static int g_sessionCreating = 0;  // T12: set around ANY session create (lazy or eager), so the town's retail build order applies to both
static int g_sessionFromSave = 0;  // T12/F67: set only around `d2_session_create_from_save`'s create -- a LOADED character joining at the act's town, whose arrival tile is the act start. A new-character or fixture create (the shell's `play=1` path, `d2_session_create_class`, and the direct `d2_session_create`/`_raw`) never sets it, so those keep the room order they have always built.
// During a session's bulk town construction, every town room is materialized in one call. Retail
// also allocates the town's rooms, but its client only sights the player's local wave; propagating
// each eager room's sight into warp-linked levels queues destination rooms the client has not
// approached. Keep the retail cross-level propagation path intact once the session is live.
extern "C" int d2_session_bulk_town_create_active() {
    return g_sessionCreating && g_sessionFromSave && g_currentAct >= 0
        && g_currentLevel == DUNGEON_GetTownLevelIdFromActNo(g_currentAct);
}

// F254: a LOADED Harrogath placed its quest-gated town objects from the PRE-LOAD state -- Anya/Drehya
// (MONSTER_DREHYA 512) absent while Nihlathak (514) stood, both halves of `ACT5Q3_Callback13_
// PlayerStartedGame`. The level is built inside the create, so its objects run their init functions
// BEFORE the save's quest record is applied and before `QUESTS_SequenceCycler`'s game-start pass; the
// two objects read A5Q3's `unk0x84`/`unk0x88`, which that callback sets from the loaded record.
//
// Deferring the room population until after the quest pass fixes the placement but is WRONG: every unit
// draws its own seed from the single `pGame->pGameSeed` stream (`SUNIT_InitSeed`), so moving the
// population past the save application shifts every loaded-town unit's seed (measured in a throwaway
// load probe: all 10 Harrogath town monsters' seeds move) and the Towner routes follow room activation
// and the stream -- Qual-Kehk and Larzuk leave retail's captured positions (s916). So instead, with NO
// unit created and NO seed drawn, set exactly the two fields the object inits read, from the save's own
// A5Q3 record, here in the create before any room is populated. The level build is bit-for-bit what it
// was; the later, real QUESTS_SequenceCycler sets and overwrites everything else (this only has to be
// right at the moment the objects init).
// Narrow on purpose (A5Q3's two town objects): it mirrors that one recovered callback, not the general
// quest-pass-before-level-load ordering, which is a separate, larger row.
static const uint8_t* g_sessionLoadSaveData = nullptr;
static uint32_t g_sessionLoadSaveSize = 0;
static void sessionPreloadA5Q3FlagsFromSave(int difficulty) {
    const int32_t kRecordBytes = (int32_t)(sizeof(uint16_t) * NUM_QUEST_WORDS);   // 96, ReadQuestSection's record
    const uint8_t* sec = g_sessionLoadSaveData;
    const uint8_t* end = sec ? sec + g_sessionLoadSaveSize : nullptr;
    if (!sec || difficulty < 0 || difficulty > 2) return;
    if (sec + sizeof(D2SaveHeaderStrc) + 10 + 3 * kRecordBytes > end) return;
    sec += sizeof(D2SaveHeaderStrc);
    if (*(const uint32_t*)sec != 0x216F6F57u || *(const uint32_t*)(sec + 4) != 6) return;   // the quest section's own header
    const uint8_t* record = sec + 10 + (int32_t)difficulty * kRecordBytes;
    uint8_t buf[kRecordBytes];
    D2BitBufferStrc tmp = {};
    BITMANIP_Initialize(&tmp, buf, sizeof buf);
    // retail's own transform, exactly what D2WEB_PLRSAVE2_ReadQuestSection applies on a real player.
    QUESTRECORD_CopyBufferToRecord(&tmp, (uint8_t*)record, (uint16_t)kRecordBytes, 1);
    if (!QUESTRECORD_GetQuestState(&tmp, QUESTSTATEFLAG_A5Q3, QFLAG_REWARDGRANTED)
        && !QUESTRECORD_GetQuestState(&tmp, QUESTSTATEFLAG_A5Q3, QFLAG_COMPLETEDBEFORE)) return;
    D2QuestDataStrc* pQuestData = QUESTS_GetQuestData(g_game, QUEST_A5Q3_PRISONOFICE);
    if (!pQuestData || !pQuestData->pQuestDataEx) return;
    D2Act5Quest3Strc* pQuestDataEx = (D2Act5Quest3Strc*)pQuestData->pQuestDataEx;
    pQuestDataEx->unk0x84 = 2;   // ACT5Q3_Callback13_PlayerStartedGame's REWARDGRANTED arm: spawn Drehya
    pQuestDataEx->unk0x88 = 1;   // ... and suppress Nihlathak
}
static int32_t g_townCentreX = -1, g_townCentreY = -1;  // T12: tile of the sight lattice's centre room when the arrival was not the act's start (a save's own position)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_session_town_centre(int tx, int ty) { g_townCentreX = tx; g_townCentreY = ty; }
static std::vector<int> g_townWalk;   // T12: tiles (x,y pairs) of the rooms the hero's walk after the arrival was centred on, in order; each builds as its own sight lattice
extern "C" EMSCRIPTEN_KEEPALIVE void d2_session_town_walk(const int32_t* xy, int n) { g_townWalk.assign(xy, xy + 2 * (n > 0 ? n : 0)); }
static int g_sessionLazy = 0;  // F63: lazy session create is OPT-IN (default flip reverted: sweep 8 had ~30 session rows red on it); d2_session_create_stream(0) restores the whole-level build (fixtures that read the level right after create call it)
extern "C" EMSCRIPTEN_KEEPALIVE void d2_session_create_stream(int on) { g_sessionLazy = on ? 1 : 0; }
static void activateSessionLevelRooms(D2DrlgActStrc* act, D2DrlgLevelStrc* level, int levelId) {
    if (!act || !level) return;
    if (g_sessionActivationWave && g_targetPlayer && g_targetPlayer->pDynamicPath) {
        // F52: a room's warp LINKS are generation-time DRLG data. sub_6FD77BB0 builds them from room
        // geometry, the level's own Vis/Warp arrays and the destination rooms' flags -- no tile grid is
        // read -- and retail has them for the whole level as soon as it is generated. The wave used to
        // leave them to DRLGACTIVATE, so a room the player had not reached had no links, its exits were
        // invisible to the session's warp list, and a slot fell through to another room's exit to the
        // same level. Build them here, before the wave starts, so entering by the wave reads the same
        // exits as entering eagerly; the wave still decides which rooms get their TILES (the guard
        // keeps a later DRLGACTIVATE from rebuilding a list and duplicating its link nodes).
        for (D2DrlgRoomStrc* roomEx = level->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext)
            if (!roomEx->nRoomsNear) sub_6FD77BB0(level->pDrlg->pMempool, roomEx);
        // The one activation retail's client makes for a landing: the player's own room, whose status
        // propagation builds the near ring and marks the rest. Everything further waits for the wave.
        const D2DynamicPathStrc* path = g_targetPlayer->pDynamicPath;
        D2ActiveRoomStrc* here = UNITS_GetRoom(g_targetPlayer);
        DUNGEON_SetClientIsInSight(act, levelId, path->tGameCoords.wPosX / 5, path->tGameCoords.wPosY / 5, here);
        return;
    }
    std::vector<D2DrlgRoomStrc*> order;
    D2ActiveRoomStrc* here = g_targetPlayer ? UNITS_GetRoom(g_targetPlayer) : nullptr;
    if (g_sessionRetailLevelChange && here && here->pDrlgRoom && here->pDrlgRoom->pLevel == level) {
        order.push_back(here->pDrlgRoom);
        for (size_t k = 0; k < order.size(); ++k) {
            D2DrlgRoomStrc* r = order[k];
            if (!r->nRoomsNear) sub_6FD77BB0(level->pDrlg->pMempool, r);
            for (int i = 0; i < r->nRoomsNear; ++i) {
                D2DrlgRoomStrc* n = r->ppRoomsNear[i];
                if (n && n->pLevel == level && std::find(order.begin(), order.end(), n) == order.end()) order.push_back(n);
            }
        }
    }
    for (D2DrlgRoomStrc* roomEx = level->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext)
        if (std::find(order.begin(), order.end(), roomEx) == order.end()) order.push_back(roomEx);
    for (D2DrlgRoomStrc* roomEx : order) {
        if (!roomEx->pRoom) DUNGEON_SetClientIsInSight(act, levelId, roomEx->nTileXPos, roomEx->nTileYPos, nullptr);
        if (roomEx->pRoom) sub_6FC385A0(g_game, roomEx->pRoom);
    }
}

// F128: three already-committed captures (trapdoor, town->wilderness edge, wilderness-
// wilderness edge) all show retail setting the TARGET level's sight point in the same poll tick it
// reaches the SOURCE room that carries the crossing -- before the player's own level changes. Both
// kinds of crossing are already-populated DRLG data on the source room, so no per-level special case
// is needed: an outdoor edge is the room's own near-room list crossing a level boundary (geometric
// adjacency, same read as `d2_session_seam_edges`/`d2_session_command_seam`); an indoor trapdoor is
// its DS1 warp tiles' `pRoomTiles` list, populated by DRLGROOMTILE_AddWarp with the destination room
// already linked (`tile->pDrlgRoom`) regardless of the cross-level `ppRoomsNear` link this file's own
// F128 note ruled out for the Sewers case. A room the wave has already reached (`roomEx->pRoom`)
// drives whichever kind of exit it holds; this replaces the four retail-verified but Sewers-specific
// points `s000b-act2.mjs` used to replay by hand with the same rule for every level.
// First-occurrence log of the target points this rule sets ({level, tileX, tileY, callIndex}), read by the s210
// replay row against retail's own record. callIndex counts d2_session_tick frames since reset.
static std::vector<int32_t> g_presightLog; static int32_t g_presightFrame = 0;
static void presightSet(D2DrlgActStrc* act, D2DrlgRoomStrc* r, D2DrlgRoomStrc* from) {
    for (size_t i = 0; i + 5 < g_presightLog.size(); i += 6)
        if (g_presightLog[i] == r->pLevel->nLevelId && g_presightLog[i + 1] == r->nTileXPos && g_presightLog[i + 2] == r->nTileYPos) return; // once per target: a set is a counted reference
    g_presightLog.insert(g_presightLog.end(), {(int32_t)r->pLevel->nLevelId, (int32_t)r->nTileXPos, (int32_t)r->nTileYPos, g_presightFrame, (int32_t)from->nTileXPos, (int32_t)from->nTileYPos});
    DUNGEON_SetClientIsInSight(act, r->pLevel->nLevelId, r->nTileXPos, r->nTileYPos, nullptr);
}
extern "C" EMSCRIPTEN_KEEPALIVE void d2_session_presight_reset() { g_presightLog.clear(); g_presightFrame = 0; }
extern "C" EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_presight_log(int* n) { *n = (int)(g_presightLog.size() / 6); return g_presightLog.data(); }
static int32_t g_presightHere[4];
extern "C" EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_presight_here() { return g_presightHere; }
static void sessionPreSightCrossLevelExits(D2DrlgActStrc* act, D2DrlgLevelStrc* level) {
    ++g_presightFrame;
    if (!act || !level) return;
    // Retail's lattice is centred on the hero's own room (s2101: the target row is set only when the hero stands in the
    // crossing room -- one room earlier, with the crossing room merely in the lattice, retail sets nothing).
    D2ActiveRoomStrc* here = g_targetPlayer ? UNITS_GetRoom(g_targetPlayer) : nullptr;
    for (D2DrlgRoomStrc* roomEx = here ? here->pDrlgRoom : nullptr; roomEx; roomEx = nullptr) {
        g_presightHere[0] = roomEx->nTileXPos; g_presightHere[1] = roomEx->nTileYPos; g_presightHere[2] = roomEx->nRoomsNear; g_presightHere[3] = 0;
        for (int i = 0; i < roomEx->nRoomsNear; ++i) if (roomEx->ppRoomsNear[i] && roomEx->ppRoomsNear[i]->pLevel != level) g_presightHere[3] += roomEx->ppRoomsNear[i]->pRoom ? 100 : 1;
        if (!roomEx->pRoom) continue;
        for (int i = 0; i < roomEx->nRoomsNear; ++i) {
            D2DrlgRoomStrc* near = roomEx->ppRoomsNear[i];
            if (near && near->pLevel && near->pLevel != level)
                presightSet(act, near, roomEx);
        }
        for (D2RoomTileStrc* tile = roomEx->pRoomTiles; tile; tile = tile->pNext) {
            D2DrlgRoomStrc* target = tile->pDrlgRoom;
            if (tile->bEnabled && target && target->pLevel && target->pLevel != level)
                presightSet(act, target, roomEx);
        }
    }
}

// While the client holds the world (d2_session_hold_world), the first read rebuilds these lists and
// the rest reuse them: every read export rebuilds otherwise, and the shell's one snapshot is a dozen
// reads -- ten times the cost of the game tick itself (s000a's budget). Only read-only stretches
// hold; a command between two reads needs a release first.
static int g_worldHold = 0; static bool g_worldHoldBuilt = false;
static bool inStartLattice(const D2DrlgRoomStrc* r);
// Rooms the capture-style level join (d2_session_client_join_level) listed the client in: a server roster entry only.
// Retail's D2Client still holds just the units the sync sent (a retail recording: no torch first-sight sounds from those rooms), so
// the client-hold tests below do not count that entry.
static std::set<D2ActiveRoomStrc*> g_sessionJoinRooms;
static int32_t clientHeldCount(D2ActiveRoomStrc* room) { return room->nNumClients - (g_sessionJoinRooms.count(room) ? 1 : 0); }

// Client-side table: retail's client holds every room near its own room even where the headless server attached the own room only (a retail recording).
static bool sessionClientHoldsRoom(D2ActiveRoomStrc* room) {
    if (clientHeldCount(room) > 0) return true;
    // retail's server adds the client to the Room2 near list of the hero's room (the 0xAC creates of a retail recording fall in those rooms)
    D2ActiveRoomStrc* here = g_targetPlayer ? UNITS_GetRoom(g_targetPlayer) : nullptr;
    if (here == room) return true;
    D2DrlgRoomStrc* h = here ? here->pDrlgRoom : nullptr;
    if (h) for (int32_t i = 0; i < h->nRoomsNear; ++i) if (h->ppRoomsNear[i] == room->pDrlgRoom && (room->tCoords.nSubtileX == here->tCoords.nSubtileX || room->tCoords.nSubtileY == here->tCoords.nSubtileY)) return true;   // edge neighbours only: a retail recording's diagonal room holds server units retail never sent
    return false;
}
// a retail recording: which monsters retail's client holds. The capture's join announces the monsters within 40 subtiles (box) of the start (0xAC, D2CAPTURE_SESSION_RADIUS);
// afterwards 0x4B1D0 diffs the hero room's near list: units of a room that enters it reach the client (0x9DC10 -> 0x7B0A0), those of a room that leaves it are removed.
static std::set<uint32_t> g_heldMonsters, g_heldSeen; static std::set<D2DrlgRoomStrc*> g_heldNear; static D2UnitStrc* g_heldFor = nullptr; static int g_heldX0, g_heldY0;
static void sessionHeldStep() {
    D2ActiveRoomStrc* here = g_targetPlayer ? UNITS_GetRoom(g_targetPlayer) : nullptr;
    if (!here || !here->pDrlgRoom) return;
    D2DrlgRoomStrc* h = here->pDrlgRoom; std::set<D2DrlgRoomStrc*> cur;
    for (int32_t i = 0; i < h->nRoomsNear; ++i) if (h->ppRoomsNear[i] && h->ppRoomsNear[i]->pRoom) cur.insert(h->ppRoomsNear[i]);   // an inactive room is not announced yet
    const bool first = g_heldFor != g_targetPlayer;
    if (first) { g_heldMonsters.clear(); g_heldSeen.clear(); g_heldNear.clear(); g_heldFor = g_targetPlayer; D2CoordStrc c = {}; UNITS_GetCoords(g_targetPlayer, &c); g_heldX0 = c.nX; g_heldY0 = c.nY; }
    auto each = [](D2DrlgRoomStrc* r, auto f) { if (r->pRoom) for (D2UnitStrc* u = r->pRoom->pUnitFirst; u; u = u->pRoomNext) if (u->dwUnitType == UNIT_MONSTER) f(u); };
    for (D2DrlgRoomStrc* r : cur) if (!g_heldNear.count(r)) each(r, [&](D2UnitStrc* u) { D2CoordStrc c = {}; UNITS_GetCoords(u, &c); if (!first || (abs(c.nX - g_heldX0) <= 40 && abs(c.nY - g_heldY0) <= 40)) g_heldMonsters.insert(u->dwUnitId); });
    for (D2DrlgRoomStrc* r : g_heldNear) if (!cur.count(r)) each(r, [&](D2UnitStrc* u) { g_heldMonsters.erase(u->dwUnitId); });
    // A monster that appears in a room the client already holds (a summoned golem) is announced to it with a 0xAC: held from that tick.
    // One that walks in from a room outside the near list is not new: retail sends it no 0xAC (a retail recording: Fallen walking in, 0x67/0x6D only), so the client never holds it.
    for (D2DrlgRoomStrc* r : cur) each(r, [&](D2UnitStrc* u) { if (g_heldSeen.insert(u->dwUnitId).second && g_heldNear.count(r)) g_heldMonsters.insert(u->dwUnitId); });
    for (D2DrlgRoomStrc* r = h->pLevel ? h->pLevel->pFirstRoomEx : nullptr; r; r = r->pDrlgRoomNext) if (!cur.count(r)) each(r, [&](D2UnitStrc* u) { g_heldSeen.insert(u->dwUnitId); });
    g_heldNear = cur;
}
// a retail recording: retail's client holds the town's river rooms while the hero's level lies within two Levels.txt vis links of the town (Blood Moor 2 at one;
// Cold Plains 3 and Den of Evil 8 at two; Stony Field 4, Dark Wood 5, Tamoe Highland 7 farther): the rule a retail recording streams fit.
static D2DrlgLevelStrc* g_townLevelForClient() {
    if (g_currentLevel == 1) return nullptr;
    auto vis = [](int32_t level, int slot) -> int32_t {   // the level's links as the seam code reads them: this game's warps first
        D2DrlgWarpStrc* w = DRLG_GetDrlgWarpFromLevelId(g_lastLevel->pDrlg, level);
        return w ? w->nVis[slot] : (int32_t)DATATBLS_GetLevelDefRecord(level)->dwVis[slot];
    };
    bool near = false;
    for (int i = 0; i < 8 && !near; ++i) {
        const int32_t v = vis(g_currentLevel, i);
        if (v <= 0) continue;
        if (v == 1) { near = true; break; }
        for (int j = 0; j < 8; ++j) if (vis(v, j) == 1) near = true;
    }
    return near ? DRLG_GetLevel(g_lastLevel->pDrlg, 1) : nullptr;
}
// The client's own copy of a monster's animation, stepped once a frame as D2Common #10373 steps a unit with no
// sequence: from the frame it is on (the next one at full speed and above) through the frame it reaches, any
// AnimData frame flag is the unit's event. A slow animation covers its event frame on two frames running, and the
// client runs the skill's client do-function (and its dosound) on each. The mode starts at frame 0, unstepped until
// the client has it.
// The world is rebuilt more than once a frame: the step runs once per game frame and the frame's event is kept.
struct D2MonClientAnim { const D2UnitStrc* unit; uint32_t mode, frame; int32_t precise; bool event, held; };
static std::unordered_map<uint32_t, D2MonClientAnim> g_monClientAnim;
static bool clientAnimEvent(const D2UnitStrc* unit) {
    D2MonClientAnim& a = g_monClientAnim[(uint32_t)unit->dwUnitId];
    const uint32_t now = g_game ? g_game->dwGameFrame : 0;
    if (a.unit == unit && a.mode == unit->dwAnimMode && a.frame == now) return a.event;
    if (a.unit != unit || a.mode != unit->dwAnimMode) { a = {unit, unit->dwAnimMode, now, 0, false, true}; return false; }
    a.frame = now; a.event = false;
    if (a.held) { a.held = false; return false; }   // the client takes the mode from the server's packet a frame later
    if (!unit->pAnimData || unit->pAnimSeq || unit->wAnimSpeed <= 0 || unit->dwFrameCountPrecise <= 0) return false;
    bool& event = a.event;
    const auto flag = [&](int32_t f) { if (f >= 0 && f < 144 && unit->pAnimData->pFrameFlags[f]) event = true; };
    int32_t frame = (a.precise >> 8) + (unit->wAnimSpeed >= 256 ? 1 : 0);
    a.precise += unit->wAnimSpeed;
    while (a.precise >= unit->dwFrameCountPrecise) {
        for (int32_t i = frame; i < (unit->dwFrameCountPrecise >> 8); ++i) flag(i);
        frame = 0; a.precise -= unit->dwFrameCountPrecise;
    }
    for (; frame <= (a.precise >> 8); ++frame) flag(frame);
    return event;
}
static void updateSessionWorld() {
    if (g_worldHold && g_worldHoldBuilt) return;
    if (g_worldHold) g_worldHoldBuilt = true;
    g_sessionRooms.clear(); g_sessionUnits.clear(); g_sessionUnitCoords.clear(); g_sessionUnitFacings.clear(); g_sessionUnitOverlays.clear(); g_sessionUnitStates.clear(); g_sessionMonsterAnimations.clear(); g_sessionObjectPresentations.clear(); g_sessionItemPresentations.clear(); g_sessionItemColours.clear(); g_sessionItemInvColours.clear(); g_sessionItemSockets.clear(); g_sessionWarps.clear(); g_sessionWarpOrigins.clear(); g_sessionMissileAnimations.clear();
    g_sessionWorldHash = D2VEC_FNV_INIT;
    g_sessionNavigationHash = D2VEC_FNV_INIT;
    // The tile lists are built from this world only when a tile export asks (ensureSessionTiles):
    // most rebuilds serve a snapshot that never reads them, and they were half the rebuild's cost.
    g_sessionTilesStale = true; g_sessionTilesRollTrace = g_d2webRollTraceEnabled;
    countSessionTilesSkipped();
    if (!g_sessionLive || !g_lastLevel) return;
    uint32_t roomOrdinal = 0;
    if (g_targetPlayer && UNITS_GetRoom(g_targetPlayer)) {
        // Retail's server sends first-sight units from each adjacent room's pUnitUpdate chain (LEVEL_UpdateUnitsInAdjacentRooms), so the
        // client's create rolls (D2Client A7780/A7400) fall in that order for every type; room activation (0x440C0) draws come first.
        D2ActiveRoomStrc** rl = nullptr; int32_t nr = 0; DUNGEON_GetAdjacentRoomsListFromRoom(UNITS_GetRoom(g_targetPlayer), &rl, &nr);
        for (int i = 0; i < nr; ++i) clientRoomActivate(rl[i], (int)DUNGEON_GetLevelIdFromRoom(rl[i]));
        for (int i = 0; i < nr; ++i) for (D2UnitStrc* u = rl[i]->pUnitUpdate; u; u = u->pChangeNextUnit) {
            const uint64_t key = (uint64_t)u->dwUnitType << 32 | u->dwUnitId;
            if (u == g_targetPlayer || u->dwUnitType == UNIT_MISSILE || g_clientInitSeed.count(key)) continue;
            int x = 0, y = 0;
            if (u->pDynamicPath && (u->dwUnitType == UNIT_PLAYER || u->dwUnitType == UNIT_MONSTER)) { x = u->pDynamicPath->tGameCoords.wPosX; y = u->pDynamicPath->tGameCoords.wPosY; }
            else if (u->pStaticPath) { x = u->pStaticPath->tGameCoords.nX; y = u->pStaticPath->tGameCoords.nY; }
            uint32_t lv, rx, ry, sd = 0;
            if (d2_drlg_client_unit_context(x, y, 1, &lv, &rx, &ry, &sd)) g_clientInitSeed[key] = sd;
        }
    }
    // D14: a unit stays in its room (corpses included) after the player crosses into the next level, so the
    // units of the seam levels' rooms are exported too; those rooms get seam ordinals and no D2SessionRoom.
    D2DrlgLevelStrc* unitLevel = g_lastLevel;
    for (; unitLevel; ) {
    for (D2DrlgRoomStrc* roomEx = unitLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext, ++roomOrdinal) {
        D2ActiveRoomStrc* room = roomEx->pRoom;
        if (!room && unitLevel == g_townLevelForClient() && unitLevel != g_lastLevel && roomEx->nType == DRLGTYPE_PRESET && roomEx->pMaze && roomEx->pMaze->pMap) {
            // a retail recording: retail's client holds the neighbouring level's river rooms (a retail recording: five 2599 selects from the town's east rooms) though
            // the server never activates them here; their river objects exist only once the room's hard-coded presets are spawned into the map list.
            const D2DrlgMapStrc* map = roomEx->pMaze->pMap; const int prest = map->nLevelPrest;
            if (prest == LVLPREST_ACT1_TOWN_1 || prest == LVLPREST_ACT1_TOWN_1_TRANSITION_S || (prest >= LVLPREST_ACT1_RIVER_UPPER && prest <= LVLPREST_ACT1_BRIDGE) || prest == LVLPREST_ACT1_TRISTRAM) {
                if (!(roomEx->dwFlags & DRLGROOMFLAG_PRESET_UNITS_ADDED)) DRLGPRESET_SpawnHardcodedPresetUnits(roomEx);
                for (D2PresetUnitStrc* pu = map->pPresetUnit; pu; pu = pu->pNext) {   // map list: absolute subtiles
                    if (pu->nUnitType != UNIT_OBJECT || !(pu->bSpawned & 1) || (pu->nIndex != OBJECT_INVISIBLE_RIVER_SOUND1 && pu->nIndex != OBJECT_INVISIBLE_RIVER_SOUND2)) continue;
                    if (pu->nXpos < roomEx->nTileXPos * 5 || pu->nXpos >= (roomEx->nTileXPos + roomEx->nTileWidth) * 5 || pu->nYpos < roomEx->nTileYPos * 5 || pu->nYpos >= (roomEx->nTileYPos + roomEx->nTileHeight) * 5) continue;
                    const D2ObjectsTxt* txt = DATATBLS_GetObjectsTxtRecord(pu->nIndex);
                    D2SessionObjectPresentation object = {};
                    object.id = 0x78000000u | ((uint32_t)(pu->nXpos & 0x3fff) << 13) | (uint32_t)(pu->nYpos & 0x1fff); object.classId = pu->nIndex;   // position id: the ordinal shifts as the own level's rooms appear, which re-fired the first-sight sounds (a retail recording)
                    if (txt) memcpy(&object.token, txt->szToken, 3);
                    object.x = pu->nXpos; object.y = pu->nYpos; object.inClientRoom = 1;
                    g_sessionObjectPresentations.push_back(object);
                }
            }
        }
        if (!room) continue;
        D2SessionRoom out = {};
        out.ordinal = roomOrdinal; out.level = (uint32_t)g_currentLevel;
        out.x = room->tCoords.nSubtileX; out.y = room->tCoords.nSubtileY;
        out.width = room->tCoords.nSubtileWidth; out.height = room->tCoords.nSubtileHeight;
        out.flags = room->dwFlags; out.firstUnit = (uint32_t)g_sessionUnits.size();
        clientRoomActivate(room, (int)out.level);
        { // client-only river sound objects (preset units the server never spawns): exported for the shell's first-sight sounds, no unit behind them
            uint32_t k = 0;
            for (D2PresetUnitStrc* pu = roomEx->pPresetUnits; pu; pu = pu->pNext) {
                if (pu->nUnitType != UNIT_OBJECT || !(pu->bSpawned & 1) || (pu->nIndex != OBJECT_INVISIBLE_RIVER_SOUND1 && pu->nIndex != OBJECT_INVISIBLE_RIVER_SOUND2)) continue;
                const D2ObjectsTxt* txt = DATATBLS_GetObjectsTxtRecord(pu->nIndex);
                D2SessionObjectPresentation object = {};
                object.id = 0x70000000u | (roomOrdinal << 12) | k++; object.classId = pu->nIndex;
                if (txt) memcpy(&object.token, txt->szToken, 3);
                object.x = room->tCoords.nSubtileX + pu->nXpos; object.y = room->tCoords.nSubtileY + pu->nYpos;
                object.inClientRoom = 1;
                g_sessionObjectPresentations.push_back(object);
            }
        }
        for (D2UnitStrc* unit = room->pUnitFirst; unit; unit = unit->pRoomNext) {
            if (unit->dwUnitType == UNIT_OBJECT && unit->pObjectData && unit->pObjectData->pObjectTxt) {
                if (unit->pStaticPath && !g_clientInitSeed.count(2ull << 32 | unit->dwUnitId)) {   // A7400 rolls for objects too, in room-list order
                    uint32_t lv, rx, ry, sd = 0;
                    if (d2_drlg_client_unit_context(unit->pStaticPath->tGameCoords.nX, unit->pStaticPath->tGameCoords.nY, 1, &lv, &rx, &ry, &sd)) g_clientInitSeed[2ull << 32 | unit->dwUnitId] = sd;
                }
                D2SessionObjectPresentation object = {};
                const D2ObjectsTxt* txt = unit->pObjectData->pObjectTxt;
                object.id = unit->dwUnitId; object.classId = unit->dwClassId;
                memcpy(&object.token, txt->szToken, 3); object.mode = clientObjectClock(unit).shown;
                if (unit->pStaticPath) { object.x = unit->pStaticPath->tGameCoords.nX; object.y = unit->pStaticPath->tGameCoords.nY; }
                { const ClientObjectClock& c = clientObjectClock(unit); object.framePrecise = c.clock; object.frameCountPrecise = c.count; object.animSpeed = c.speed; } object.direction = txt->nOrientation;
                // The client's own mode (the shown one) picks the Selectable column, as retail's hover does: a portal
                // the server leaves in OP is ON on the client once its opening cels run out.
                object.selectable = object.mode < 8 ? txt->nSelectable[object.mode] : 0;
                object.inClientRoom = (clientHeldCount(room) > 0 || inStartLattice(roomEx));
                object.locked = txt->nLockable && (unit->pObjectData->InteractType & 0x80);
                if (txt->nSubClass & (OBJSUBCLASS_TOWNPORTAL | OBJSUBCLASS_PORTAL)) object.portalLevel = unit->pObjectData->InteractType;
                // The COF names the layers an object draws; the table's piece flags are not consulted
                // (the Act 4 red portal row lists TR only, and drew as its black base oval).
                object.componentMask = 0xFFFFu;
                g_sessionObjectPresentations.push_back(object);
            }
            if (unit->dwUnitType != UNIT_PLAYER && unit->dwUnitType != UNIT_MONSTER && unit->dwUnitType != UNIT_ITEM && unit->dwUnitType != UNIT_MISSILE) continue;
            D2SessionUnit actor = {};
            actor.id = (uint32_t)unit->dwUnitId; actor.type = unit->dwUnitType;
            actor.classId = unit->dwClassId; actor.mode = unit->dwAnimMode;
            if ((unit->dwUnitType == UNIT_PLAYER || unit->dwUnitType == UNIT_MONSTER || unit->dwUnitType == UNIT_MISSILE) && unit->pDynamicPath) {
                actor.x = unit->pDynamicPath->tGameCoords.wPosX;
                actor.y = unit->pDynamicPath->tGameCoords.wPosY;
            } else if (unit->pStaticPath) {
                actor.x = unit->pStaticPath->tGameCoords.nX;
                actor.y = unit->pStaticPath->tGameCoords.nY;
            }
            actor.flags = unit->dwFlags; actor.flagsEx = unit->dwFlagEx;
            actor.hp = STATLIST_UnitGetStatValue(unit, STAT_HITPOINTS, 0);
            actor.maxHp = STATLIST_UnitGetStatValue(unit, STAT_MAXHP, 0);
            // Server missiles reach the client as 0x4C/0x4D starts, not unit assignments, so no A7400 room roll (a retail recording: retail player room holds 4a34886b,5e77e2f7 ticks 2-19).
            if (unit != g_targetPlayer && unit->dwUnitType != UNIT_MISSILE && !g_clientInitSeed.count((uint64_t)actor.type << 32 | actor.id)) {
                uint32_t lv, rx, ry, sd = 0;
                if (d2_drlg_client_unit_context(actor.x, actor.y, 1, &lv, &rx, &ry, &sd)) g_clientInitSeed[(uint64_t)actor.type << 32 | actor.id] = sd;
            }
            actor.room = roomOrdinal; g_sessionUnits.push_back(actor); ++out.unitCount;
            {   // The 16.16 position of the same unit, in the same order (0,0 with no dynamic path).
                D2SessionUnitCoord coord = {};
                coord.id = (uint32_t)unit->dwUnitId;
                const bool dynamic = (unit->dwUnitType == UNIT_PLAYER || unit->dwUnitType == UNIT_MONSTER || unit->dwUnitType == UNIT_MISSILE) && unit->pDynamicPath;
                if (dynamic) { coord.preciseX = unit->pDynamicPath->tGameCoords.dwPrecisionX; coord.preciseY = unit->pDynamicPath->tGameCoords.dwPrecisionY; }
                g_sessionUnitCoords.push_back(coord);
            }
            {   // Native path facing, including player-body units, in unit-list order.
                D2SessionUnitFacing facing = {};
                facing.id = (uint32_t)unit->dwUnitId;
                const bool hasDynamicPath = (unit->dwUnitType == UNIT_PLAYER || unit->dwUnitType == UNIT_MONSTER || unit->dwUnitType == UNIT_MISSILE) && unit->pDynamicPath;
                facing.hasDynamicPath = hasDynamicPath;
                facing.direction = hasDynamicPath ? unit->pDynamicPath->nDirection : 0;
                facing.lastHitClass = (uint8_t)unit->dwLastHitClass;
                facing.superUniqueId = -1;
                if (unit->dwUnitType == UNIT_MONSTER && unit->pMonsterData) {
                    facing.typeFlags = (uint8_t)unit->pMonsterData->nTypeFlag;
                    if (facing.typeFlags & MONTYPEFLAG_SUPERUNIQUE) facing.superUniqueId = (int16_t)unit->pMonsterData->wBossHcIdx;
                    if (D2SkillStrc* used = UNITS_GetUsedSkill(unit)) { const int v = SKILLS_GetSkillIdFromSkill(used, __FILE__, __LINE__) + 1; facing.usedSkillLo = (uint8_t)v; facing.usedSkillHi = (uint8_t)(v >> 8); }
                    if (clientAnimEvent(unit)) facing.usedSkillHi |= 0x80;
                }
                g_sessionUnitFacings.push_back(facing);
            }
            {   // Live overlay (UNITS_SetOverlay's stat), exported only when set.
                const int32_t overlayId = STATLIST_UnitGetStatValue(unit, STAT_UNIT_DOOVERLAY, 0);
                if (overlayId > 0) g_sessionUnitOverlays.push_back({(uint32_t)unit->dwUnitId, (uint32_t)unit->dwUnitType, (uint32_t)overlayId});
            }
            if (unit->dwUnitType == UNIT_MONSTER)   // active states, exported for the monster state overlays (one record per state)
                for (int32_t s = 0; s <= STATE_UBERMINION; s++)
                    if (STATES_CheckState(unit, s)) g_sessionUnitStates.push_back({(uint32_t)unit->dwUnitId, (uint32_t)unit->dwUnitType, (uint32_t)s});
            if (unit->dwUnitType == UNIT_ITEM) appendSessionItemPresentation(unit);
            if (unit->dwUnitType == UNIT_MISSILE) {
                D2SessionMissileAnimation missile = {};
                missile.id = (uint32_t)unit->dwUnitId; missile.classId = unit->dwClassId; missile.mode = unit->dwAnimMode;
                missile.framePrecise = unit->nSeqCurrentFramePrecise; missile.frameCountPrecise = unit->dwFrameCountPrecise;
                missile.animSpeed = unit->wAnimSpeed;
                missile.direction = unit->pDynamicPath ? unit->pDynamicPath->nDirection : 0;
                const D2MissilesTxt* txt = DATATBLS_GetMissilesTxtRecord(unit->dwClassId);
                missile.trans = txt ? txt->nTrans : 0;
                missile.remaining = MISSILE_GetRemainingFrames(unit);
                g_sessionMissileAnimations.push_back(missile);
            }
            if (unit->dwUnitType == UNIT_MONSTER && unit->pMonsterData) {
                D2SessionMonsterAnimation animation = {};
                animation.id = unit->dwUnitId; animation.classId = unit->dwClassId;
                animation.code = unit->pMonsterData->pMonstatsTxt ? unit->pMonsterData->pMonstatsTxt->dwCode : 0;
                animation.mode = unit->dwAnimMode; animation.framePrecise = unit->nSeqCurrentFramePrecise;
                { auto it = g_clientMonsterClock.find(unit->dwUnitId);
                  if (it != g_clientMonsterClock.end() && it->second.valid && it->second.mode == unit->dwAnimMode)
                      animation.framePrecise = it->second.clock; }
                animation.frameCountPrecise = unit->dwFrameCountPrecise; animation.animSpeed = unit->wAnimSpeed;
                animation.direction = unit->pDynamicPath ? unit->pDynamicPath->nDirection : 0;
                const D2MonStats2Txt* mon2 = UNITS_GetMonStats2TxtRecordFromMonsterId((int)unit->dwClassId);
                animation.baseWeaponClass = mon2 ? static_cast<uint32_t>(mon2->nBaseW) : 0;
                animation.animationFlags = (unit->pAnimSeq ? 0x01 : 0) | (g_heldMonsters.count(unit->dwUnitId) ? 0x04 : 0) |  // bit 2: the unit's room is the client's or adjacent to it (retail's client holds the near rooms; the fixture's server attaches the own room only)
                    
                    (mon2 && (mon2->dwModeFlags & MONSTATS2MODEFLAG_DD) ? 0x02 : 0) |
                    (unit->pAnimSeq ? ((unit->dwSeqMode & 0xF) << 4) : 0);   // bits 4-7: the sequence's current mode (SQ draws as it, e.g. a Fallen Shaman's A2)
                for (uint32_t component = 0; component < 16; ++component)
                    animation.componentCodes[component] = D2Common_11069(unit, component, unit->pMonsterData->nComponent[component]);
                g_sessionMonsterAnimations.push_back(animation);
            }
        }
        if (unitLevel == g_lastLevel) g_sessionRooms.push_back(out);
    }
    if (unitLevel == g_lastLevel) { roomOrdinal = 0x10000; unitLevel = g_lastLevel->pDrlg ? g_lastLevel->pDrlg->pLevel : nullptr; }
    else unitLevel = unitLevel->pNextLevel;
    if (unitLevel == g_lastLevel) unitLevel = unitLevel->pNextLevel;
    }
    if (g_targetPlayer && g_targetPlayer->pInventory) {
        for (D2UnitStrc* item = INVENTORY_GetFirstItem(g_targetPlayer->pInventory); item; item = INVENTORY_GetNextItem(item)) {
            if (INVENTORY_UnitIsItem(item)) appendSessionItemPresentation(item);
        }
        // The item on the cursor (0x19/0x1C put it there) is the player's too, whether or not the
        // inventory's list still links it: the client draws it at the pointer.
        if (D2UnitStrc* cursor = INVENTORY_GetCursorItem(g_targetPlayer->pInventory)) {
            bool listed = false;
            for (const D2SessionItemPresentation& p : g_sessionItemPresentations) if (p.id == cursor->dwUnitId && p.mode == cursor->dwAnimMode) { listed = true; break; }
            if (!listed) appendSessionItemPresentation(cursor);
        }
    }
    D2DrlgWarpStrc* dynamicWarps = DRLG_GetDrlgWarpFromLevelId(g_lastLevel->pDrlg, g_currentLevel);
    const D2LevelDefBin* levelDef = DATATBLS_GetLevelDefRecord(g_currentLevel);
    for (uint32_t slot = 0; slot < 8; ++slot) {
        const int32_t destination = dynamicWarps ? dynamicWarps->nVis[slot] : levelDef->dwVis[slot];
        const int32_t warpId = dynamicWarps ? dynamicWarps->nWarp[slot] : (int32_t)levelDef->dwWarp[slot];
        if (destination <= 0 || warpId < 0) continue;
        D2LvlWarpTxt* warp = DATATBLS_GetLvlWarpTxtRecordFromLevelIdAndDirection(warpId, 'b');
        if (!warp) continue;
        D2SessionWarp out = {};
        int32_t originX = INT32_MIN, originY = 0;
        out.slot = slot; out.room = UINT32_MAX;
        out.sourceLevel = (uint32_t)g_currentLevel; out.destinationLevel = (uint32_t)destination;
        out.warpId = (uint32_t)warpId; out.enabled = 1; out.direction = (uint8_t)warp->szDirection[0];
        uint32_t ordinal = 0;
        for (const D2SessionWarpPreset& preset : g_sessionWarpPresets) {
            // A tile's class is its LvlWarp Id. Match both: Lut Gholein has two sewer entrances (LvlWarp 19
            // and 20) to one level, the Spider Forest two caves (84, 85) sharing LvlWarp 51.
            if (preset.warpId != warpId || preset.destination != destination) continue;
            out.room = preset.room; out.exitX = preset.x; out.exitY = preset.y; break;
        }
        // The room tile of THIS slot's LvlWarp first; failing that (the outdoor alternatives that were not
        // generated -- the Blood Moor lists four Den entrances, one real), the first tile to the level.
        for (int pass = 0; pass < 2 && out.room == UINT32_MAX; ++pass) { ordinal = 0;
        for (D2DrlgRoomStrc* room = g_lastLevel->pFirstRoomEx; room; room = room->pDrlgRoomNext, ++ordinal) {
            if (out.room != UINT32_MAX) break;
            for (D2RoomTileStrc* tile = room->pRoomTiles; tile; tile = tile->pNext) {
                if (!tile->bEnabled || !tile->pDrlgRoom || !tile->pDrlgRoom->pLevel || tile->pDrlgRoom->pLevel->nLevelId != destination) continue;
                if (pass == 0 && tile->pLvlWarpTxtRecord && (int32_t)tile->pLvlWarpTxtRecord->dwLevelId != warpId) continue;   // this slot's exit, not a sibling
                D2DrlgTileDataStrc* data = tile->unk0x0C ? tile->unk0x0C : tile->unk0x10;
                out.room = ordinal;
                if (pass == 1) out.enabled = 2;   // D28: no UNIT_TILE of THIS warp id was generated (an unrealised outdoor alternative); borrowed point, nothing to click
                if (data && tile->pLvlWarpTxtRecord) {
                    out.exitX = 5 * (room->nTileXPos + data->nPosX) + (int32_t)tile->pLvlWarpTxtRecord->dwOffsetX;
                    out.exitY = 5 * (room->nTileYPos + data->nPosY) + (int32_t)tile->pLvlWarpTxtRecord->dwOffsetY;
                } else if (sessionWarpPointFromDs1(room, warpId, &out.exitX, &out.exitY)) {
                    // F54: the node carries no tile data, so the point is the room's own DS1 geometry
                    // (F52's derivation), whether or not the room is built -- a built room with a blank
                    // link is not a room with an answer. Lut Gholein's three exits all read here, 11, 19
                    // and 27 subtiles from the position DRLGROOMTILE_AddWarp recorded (s421), which is
                    // what the room-centre fallback below was guessing at. The collision nudge at the end
                    // of this block still needs a room and applies whenever there is one.
                } else {
                    const D2DrlgRoomStrc* linked = tile->pDrlgRoom;
                    const int32_t roomCenterX = room->nTileXPos + room->nTileWidth / 2;
                    const int32_t roomCenterY = room->nTileYPos + room->nTileHeight / 2;
                    const int32_t linkedCenterX = linked->nTileXPos + linked->nTileWidth / 2;
                    const int32_t linkedCenterY = linked->nTileYPos + linked->nTileHeight / 2;
                    out.exitX = 5 * (linkedCenterX < roomCenterX ? room->nTileXPos :
                        linkedCenterX > roomCenterX ? room->nTileXPos + room->nTileWidth - 1 : roomCenterX) + 2;
                    out.exitY = 5 * (linkedCenterY < roomCenterY ? room->nTileYPos :
                        linkedCenterY > roomCenterY ? room->nTileYPos + room->nTileHeight - 1 : roomCenterY) + 2;
                }
                originX = out.exitX; originY = out.exitY;   // F6: the select box anchor IS the warp tile unit position (retail-measured, warp-units-f6-r20.json), before the collision nudge
                if (room->pRoom) {
                    D2CoordStrc interaction = { out.exitX, out.exitY };
                    if (COLLISION_GetFreeCoordinatesWithMaxDistance(room->pRoom, &interaction, 2,
                        COLLIDE_MASK_PLAYER_PATH, TRUE, 6)) {
                        out.exitX = interaction.nX; out.exitY = interaction.nY;
                    }
                }
                break;
            }
            if (out.room != UINT32_MAX) break;
        }
        }
        if (originX == INT32_MIN) { originX = out.exitX; originY = out.exitY; }
        g_sessionWarpOrigins.push_back(originX); g_sessionWarpOrigins.push_back(originY);
        out.selectX = (int32_t)warp->dwSelectX; out.selectY = (int32_t)warp->dwSelectY;
        out.selectDX = (int32_t)warp->dwSelectDX; out.selectDY = (int32_t)warp->dwSelectDY;
        out.exitWalkX = (int32_t)warp->dwExitWalkX; out.exitWalkY = (int32_t)warp->dwExitWalkY;
        out.offsetX = (int32_t)warp->dwOffsetX; out.offsetY = (int32_t)warp->dwOffsetY;
        g_sessionWarps.push_back(out);
    }
    auto hashBytes = [](uint32_t hash, const void* data, size_t size) {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        for (size_t i = 0; i < size; ++i) { hash ^= bytes[i]; hash *= 16777619u; }
        return hash;
    };
    if (!g_sessionRooms.empty()) g_sessionWorldHash = hashBytes(g_sessionWorldHash, g_sessionRooms.data(), g_sessionRooms.size() * sizeof(D2SessionRoom));
    if (!g_sessionUnits.empty()) g_sessionWorldHash = hashBytes(g_sessionWorldHash, g_sessionUnits.data(), g_sessionUnits.size() * sizeof(D2SessionUnit));
    if (!g_sessionWarps.empty()) g_sessionNavigationHash = hashBytes(g_sessionNavigationHash, g_sessionWarps.data(), g_sessionWarps.size() * sizeof(D2SessionWarp));
}

// The tiles of the world updateSessionWorld last built: the current level's active rooms in room
// order, then the other levels' (seam) rooms; the terrain hash over the level's. Once per rebuild.
static void ensureSessionTiles() {
    if (!g_sessionTilesStale) return;
    g_sessionTilesStale = false;
    g_sessionTiles.clear(); g_sessionTileEntry.clear(); g_sessionTileCoord.clear(); g_sessionTileRolls.clear(); g_sessionTileLibraries.clear();
    g_sessionTerrainHash = D2VEC_FNV_INIT; g_sessionTerrainHashPending = false;
    if (!g_sessionLive || !g_lastLevel) return;
    const bool rollTraceNow = g_d2webRollTraceEnabled; g_d2webRollTraceEnabled = g_sessionTilesRollTrace;
    g_sessionTilesCounting = false;
    uint32_t roomOrdinal = 0;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext, ++roomOrdinal) {
        D2ActiveRoomStrc* room = roomEx->pRoom;
        if (!room || !room->pRoomTiles) continue;
        appendSessionTiles(roomEx, roomOrdinal, 0, room->pRoomTiles->pFloorTiles, room->pRoomTiles->nFloors);
        appendSessionTiles(roomEx, roomOrdinal, 1, room->pRoomTiles->pWallTiles, room->pRoomTiles->nWalls);
        appendSessionTiles(roomEx, roomOrdinal, 2, room->pRoomTiles->pRoofTiles, room->pRoomTiles->nRoofs);
    }
    {
        g_sessionSeamTiles.clear(); g_sessionSeamTileEntry.clear(); g_sessionSeamTileCoord.clear(); g_sessionSeamLibraries.clear();
        std::swap(g_sessionTiles, g_sessionSeamTiles); std::swap(g_sessionTileEntry, g_sessionSeamTileEntry); std::swap(g_sessionTileCoord, g_sessionSeamTileCoord); std::swap(g_sessionTileLibraries, g_sessionSeamLibraries);
        const bool rollTrace = g_d2webRollTraceEnabled; g_d2webRollTraceEnabled = false;
        uint32_t seamOrdinal = 0x10000;
        for (D2DrlgLevelStrc* level = g_lastLevel->pDrlg ? g_lastLevel->pDrlg->pLevel : nullptr; level; level = level->pNextLevel) {
            if (level == g_lastLevel) continue;
            for (D2DrlgRoomStrc* roomEx = level->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext, ++seamOrdinal) {
                D2ActiveRoomStrc* room = roomEx->pRoom;
                if (!room || !room->pRoomTiles) continue;
                appendSessionTiles(roomEx, seamOrdinal, 0, room->pRoomTiles->pFloorTiles, room->pRoomTiles->nFloors);
                appendSessionTiles(roomEx, seamOrdinal, 1, room->pRoomTiles->pWallTiles, room->pRoomTiles->nWalls);
                appendSessionTiles(roomEx, seamOrdinal, 2, room->pRoomTiles->pRoofTiles, room->pRoomTiles->nRoofs);
            }
        }
        g_d2webRollTraceEnabled = rollTrace;
        std::swap(g_sessionTiles, g_sessionSeamTiles); std::swap(g_sessionTileEntry, g_sessionSeamTileEntry); std::swap(g_sessionTileCoord, g_sessionSeamTileCoord); std::swap(g_sessionTileLibraries, g_sessionSeamLibraries);
    }
    g_sessionTilesCounting = true; g_d2webRollTraceEnabled = rollTraceNow;
    g_sessionTerrainHashPending = true;
}

static uint32_t sessionTerrainHash() {
    if (g_sessionTerrainHashPending) {
        g_sessionTerrainHashPending = false;
        g_sessionTerrainHash = D2VEC_FNV_INIT;
        for (const D2SessionTile& tile : g_sessionTiles) {
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&tile);
            for (size_t i = 0; i < 76 /* pre-F66 bytes: entryHint (+0x4C) is not terrain identity, keeps pinned hashes */; ++i) { g_sessionTerrainHash ^= bytes[i]; g_sessionTerrainHash *= 16777619u; }
        }
    }
    return g_sessionTerrainHash;
}

// Native D2Client 1.13c weather traversal starts at the client's current Room1 and consumes its
// ordered adjacent-room array. This export reads the client-mode DRLG Act mirror, locates the room
// from the session player's coordinates, then translates identities to exported world records.
// The mirror is not retail's raw Room1 allocation; this is a client-mode candidate order, not a
// byte-for-byte Room1 parity claim.
static void buildSessionActiveNearFeed() {
    g_sessionActiveNearFeed.clear();
    D2DrlgActStrc* clientAct = d2_drlg_client_act();
    if (!g_sessionLive || !g_sessionClientMapStarted || !clientAct || !clientAct->pDrlg || !g_targetPlayer) return;
    D2ActiveRoomStrc* here = DUNGEON_FindRoomBySubtileCoordinates(clientAct,
        CLIENTS_GetUnitX(g_targetPlayer), CLIENTS_GetUnitY(g_targetPlayer));
    if (!here) return;
    D2ActiveRoomStrc** nearRooms = nullptr;
    int32_t nearCount = 0;
    DUNGEON_GetAdjacentRoomsListFromRoom(here, &nearRooms, &nearCount);
    if (!nearRooms || nearCount <= 0) return;

    std::vector<D2ActiveRoomStrc*> currentRooms(nearRooms, nearRooms + nearCount);
    std::vector<D2DrlgTileDataStrc*> currentFloors;
    std::vector<int32_t> currentFloorCounts;
    currentFloors.reserve((size_t)nearCount);
    currentFloorCounts.reserve((size_t)nearCount);
    for (D2ActiveRoomStrc* active : currentRooms) {
        int32_t floorCount = 0;
        D2DrlgTileDataStrc* floors = active ? DUNGEON_GetFloorTilesFromRoom(active, &floorCount) : nullptr;
        currentFloors.push_back(floors);
        currentFloorCounts.push_back(floorCount);
    }

    using RoomKey = std::tuple<int32_t, int32_t, int32_t>;
    std::map<RoomKey, uint32_t> roomOrdinals;
    uint32_t ordinal = 0;
    if (!g_lastLevel) return;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext, ++ordinal)
        roomOrdinals.emplace(RoomKey{roomEx->pLevel ? roomEx->pLevel->nLevelId : -1,
                                    roomEx->nTileXPos, roomEx->nTileYPos}, ordinal);
    uint32_t seamOrdinal = 0x10000;
    for (D2DrlgLevelStrc* level = g_lastLevel->pDrlg ? g_lastLevel->pDrlg->pLevel : nullptr; level; level = level->pNextLevel) {
        if (level == g_lastLevel) continue;
        for (D2DrlgRoomStrc* roomEx = level->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext, ++seamOrdinal)
            roomOrdinals.emplace(RoomKey{roomEx->pLevel ? roomEx->pLevel->nLevelId : -1,
                                         roomEx->nTileXPos, roomEx->nTileYPos}, seamOrdinal);
    }

    for (int32_t r = 0; r < nearCount; ++r) {
        D2ActiveRoomStrc* active = nearRooms[r];
        if (!active || !active->pDrlgRoom) continue;
        const D2DrlgRoomStrc* roomEx = active->pDrlgRoom;
        const int32_t levelId = roomEx->pLevel ? roomEx->pLevel->nLevelId : -1;
        const auto found = roomOrdinals.find(RoomKey{levelId, roomEx->nTileXPos, roomEx->nTileYPos});
        const uint32_t roomOrdinal = found == roomOrdinals.end() ? UINT32_MAX : found->second;
        g_sessionActiveNearFeed.push_back({0, roomOrdinal, levelId, roomEx->nTileXPos, roomEx->nTileYPos, 0, 0});

        // FUN_6FAC4050 requests selectors 1 then 2 and scans each returned floor array from index 0.
        const int32_t floorCount = currentFloorCounts[(size_t)r];
        D2DrlgTileDataStrc* floors = currentFloors[(size_t)r];
        if (!floors || floorCount <= 0) continue;
        for (uint32_t selector = 1; selector <= 2; ++selector) {
            for (int32_t i = 0; i < floorCount; ++i) {
                if (((floors[i].dwFlags >> 14) & 7u) != selector) continue;
                g_sessionActiveNearFeed.push_back({1, roomOrdinal, levelId, roomEx->nTileXPos, roomEx->nTileYPos,
                    (uint32_t)i, selector});
            }
        }
    }
}

// Self-reporting (a failure explains itself): every regen timer armed on a monster already in its
// death/dead mode -- the s000d corpse that regained life -- with the call site that armed it.
extern void (*g_d2webOnDeadRegen)(D2UnitStrc* pUnit, const char* szSite);
static uint32_t g_deadRegenCount = 0; static std::string g_deadRegenLog;
static uint32_t g_busyStreak = 0, g_busyStartFrame = 0, g_busyLongest = 0; static std::string g_busyLog;
static void recordDeadRegen(D2UnitStrc* u, const char* site) {
    ++g_deadRegenCount;
    if (g_deadRegenLog.size() > 1500) return;
    char buf[160];
    snprintf(buf, sizeof buf, "%sframe %u monster %u class %d mode %u life %d site %s", g_deadRegenLog.empty() ? "" : "; ",
        g_game ? g_game->dwGameFrame : 0, u->dwUnitId, u->dwClassId, u->dwAnimMode, STATLIST_UnitGetStatValue(u, STAT_HITPOINTS, 0), site && *site ? site : "?");
    g_deadRegenLog += buf;
}
static void captureHitRoll(D2UnitStrc* attacker, D2UnitStrc* defender, const D2CombatHitRollTrace* trace) {
    if (!g_game || !g_targetPlayer || !attacker || !defender || !trace ||
        attacker != g_targetPlayer || defender != g_sessionAttackTarget || g_sessionAttack.state != 1) return;
    g_sessionHitRolls.push_back({
        (uint32_t)g_game->dwGameFrame, attacker->dwUnitId, defender->dwUnitId, g_sessionAttack.skillId,
        g_sessionAttack.attacks, trace->nAttackerLevel, trace->nDefenderLevel, trace->nAttackRate,
        trace->nToHitPercent, trace->nTotalDefense, trace->nToHit, trace->nDefense,
        trace->nToHitFactor, trace->nChanceToHit, trace->nRoll, trace->bHit
    });
}
extern "C" EMSCRIPTEN_KEEPALIVE const char* d2_session_dead_regen() {
    static std::string out; out = std::to_string(g_deadRegenCount) + (g_deadRegenLog.empty() ? "" : ": " + g_deadRegenLog); return out.c_str();
}
static void captureCombat(D2UnitStrc* attacker, D2UnitStrc* defender, const D2DamageStrc* damage) {
    if (!g_targetPlayer || !attacker || !defender || !damage) return;
    D2VecCombat v = {};
    v.seed = g_gameSeed; v.difficulty = (uint8_t)g_gameDiff; v.act = (uint8_t)g_currentAct; v.level = (uint16_t)g_currentLevel;
    v.frame = g_game ? g_game->dwGameFrame : 0;
    v.attackerType = attacker->dwUnitType; v.attackerClass = attacker->dwClassId; v.attackerId = attacker->dwUnitId;
    v.defenderType = defender->dwUnitType; v.defenderClass = defender->dwClassId; v.defenderId = defender->dwUnitId;
    v.seedLo = attacker->pSeed.nLowSeed; v.seedHi = attacker->pSeed.nHighSeed;
    static_assert(sizeof(*damage) == sizeof(v.damage));
    memcpy(v.damage, damage, sizeof(v.damage));
    g_combatEvents.push_back(v);
    if (attacker == g_targetPlayer && defender == g_sessionAttackTarget && g_sessionAttack.state == 1) {
        g_sessionAttack.resultFlags = damage->wResultFlags;
        g_sessionAttack.physicalDamage = (uint32_t)damage->dwPhysDamage;
        g_sessionAttack.totalDamage = (uint32_t)damage->dwDmgTotal;
    }
}

static D2VecAiUnit captureAi(const D2UnitStrc* u, uint16_t room, uint16_t ordinal) {
    D2VecAiUnit a = {};
    a.room = room; a.ordinal = ordinal; a.classId = u->dwClassId; a.unitId = u->dwUnitId;
    if (u->pDynamicPath) { a.x = u->pDynamicPath->tGameCoords.wPosX; a.y = u->pDynamicPath->tGameCoords.wPosY; }
    const D2AiControlStrc* ai = u->pMonsterData ? u->pMonsterData->pAiControl : nullptr;
    if (ai) {
        a.specialState = ai->nAiSpecialState; a.flags = ai->nAiFlags;
        a.ownerGuid = ai->dwOwnerGUID; a.ownerType = ai->dwOwnerType;
        memcpy(a.params, ai->dwAiParam, sizeof(a.params));
        if (ai->pCurrentCmd) { a.hasCurrentCmd = 1; memcpy(a.currentCmdParams, ai->pCurrentCmd->nCmdParam, sizeof(a.currentCmdParams)); }
        for (const D2MinionListStrc* m = ai->pMinionList; m; m = m->pNext) {
            a.minionCount++; a.minionHash = d2vec_fnv1a_step(a.minionHash ? a.minionHash : D2VEC_FNV_INIT, (uint32_t)m->dwMinionGUID);
        }
    }
    a.thinkFrame = -1;
    for (const D2EventTimerStrc* t = u->pTimer; t; t = t->pNext) if (t->nEventType == EVENTTYPE_AITHINK) { a.thinkFrame = t->nExpireFrame; break; }
    return a;
}

static D2VecAiTargetUnit captureAiTarget(const D2UnitStrc* u, uint16_t room, uint16_t ordinal) {
    D2VecAiTargetUnit a = {};
    a.ai = captureAi(u, room, ordinal);
    a.mode = u->dwAnimMode; a.seedLo = u->pSeed.nLowSeed; a.seedHi = u->pSeed.nHighSeed;
    a.hp = STATLIST_UnitGetStatValue((D2UnitStrc*)u, STAT_HITPOINTS, 0);
    a.maxHp = STATLIST_UnitGetStatValue((D2UnitStrc*)u, STAT_MAXHP, 0);
    if (u->pDynamicPath) {
        D2UnitStrc* target = u->pDynamicPath->pTargetUnit;
        a.targetType = target ? target->dwUnitType : u->pDynamicPath->dwTargetType;
        a.targetId = target ? target->dwUnitId : u->pDynamicPath->dwTargetId;
        a.targetX = u->pDynamicPath->tTargetCoord.X; a.targetY = u->pDynamicPath->tTargetCoord.Y;
    }
    a.usedSkillId = -1;
    if (u->pSkills && u->pSkills->pUsedSkill && u->pSkills->pUsedSkill->pSkillsTxt)
        a.usedSkillId = u->pSkills->pUsedSkill->pSkillsTxt->nSkillId;
    a.modeEventFrame = -1;
    for (const D2EventTimerStrc* t = u->pTimer; t; t = t->pNext)
        if (t->nEventType == EVENTTYPE_MODECHANGE) { a.modeEventFrame = t->nExpireFrame; break; }
    a.framePrecise = u->nSeqCurrentFramePrecise; a.frameCountPrecise = u->dwFrameCountPrecise;
    a.animSpeed = u->wAnimSpeed; a.actionFrame = u->nActionFrame;
    return a;
}

static void sessionDropExtraClients();
static void detachTargetFixture() {
    sessionDropExtraClients();
    g_heldFor = nullptr;   // a retail recording: the held-monster set belongs to one hero
    // The unit stays in the world; its client does not, so nothing sent to every player
    // (0x81 pet assignments iterate the living players) reaches freed memory.
    if (g_targetPlayer && UNITS_GetPlayerData(g_targetPlayer)) UNITS_GetPlayerData(g_targetPlayer)->pClient = nullptr;
    if (g_targetRoom) { g_targetRoom->ppClients = nullptr; g_targetRoom->nNumClients = 0; g_targetRoom->nMaxClients = 0; }
    for (D2ActiveRoomStrc* r : g_sessionJoinRooms) { r->ppClients = nullptr; r->nNumClients = 0; r->nMaxClients = 0; }   // the level join's rosters too
    g_sessionJoinRooms.clear();
    free(g_targetClient); g_targetClient = nullptr; g_targetRoom = nullptr; g_targetPlayer = nullptr; g_targetMonster = nullptr;
    g_clientCastClockActive = false; g_clientCastFramePrecise = 0; g_clientCastAnimSpeed = 0;
    g_clientNeutralClockActive = false; g_clientNeutralFramePrecise = 0; g_clientNeutralAnimSpeed = 0;
    g_clientMorphActive = false; g_clientMorphStatePrev = false; g_clientMorphFramePrecise = 0; g_clientMorphFrameCount = 0;
    g_clientMorphAnimSpeed = 0; g_clientFacingActive = false; g_clientFacingServerTarget = 0; g_clientFacing = {};
}

// GAME_CreateGame minus the server (game slots, clients, tasks, critical sections): the fields the
// item/monster/object code reads, in the order retail fills them.
static D2GameStrc* makeGame(uint32_t seed, int difficulty) {
    // The game's mode is the character's: a classic character plays in a classic game. Retail's own
    // PLRSAVE2_CheckPlayerFlags refuses a mismatch in either direction, so the header this session
    // writes (dwSaveFlags) and the game the character is in have to be the same answer.
    const bool bExpansion = (g_sessionClientSaveFlags & CLIENTSAVEFLAG_EXPANSION) != 0;
    const bool bHardcore = (g_sessionClientSaveFlags & CLIENTSAVEFLAG_HARDCORE) != 0;
    D2GameStrc* g = (D2GameStrc*)calloc(1, sizeof(D2GameStrc));
    g->bExpansion = bExpansion ? TRUE : FALSE;
    g->wItemFormat = 101;
    g->dwGameType = 0;
    g->nDifficulty = (uint8_t)difficulty;
    g->nMaxPlayers = 8;
    g->InitSeed = 1;
    g->dwInitSeed = seed;
    SEED_InitSeed(&g->pGameSeed);
    SEED_InitLowSeed(&g->pGameSeed, seed);
    EVENT_AllocEventQueue(g);
    ARENA_AllocArena(g, 0, ((uint32_t)difficulty << 12) | 0x04
        | (bExpansion ? GAMEFLAG_ARENA_EXPANSION : 0u) | (bHardcore ? GAMEFLAG_ARENA_HARDCORE : 0u), 0);
    PARTY_AllocPartyControl(g);
    sub_6FC68110(g);                                  // monster regions from the game seed
    g->dwObjSeed = OBJRGN_AllocObjectControl(g);
    SUNITPROXY_InitializeNpcControl(g);
    QUESTS_QuestInit(g);
    if (g->pQuestControl) g->pQuestControl->bPickedSet = 1;   // what the first player's entry does; the capture sets the same
    return g;
}

// The skill-list packet (client_save_batch_d5740.cpp), for d2_session_client_skills.
void D2GAME_8B5A0_SendSkills(D2ClientStrc* client, D2UnitStrc* unit);

extern "C" {

// Test-only object-population seed input lives in game_glue.cpp; reset it with the session fixture.
void d2_session_object_pop_seed_override_reset();


EMSCRIPTEN_KEEPALIVE int d2_monspawn_init() {
    if (!d2::drlgLoadTables(d2::sharedVfs(), d2::sharedLang())) { g_err = "tables"; return 0; }
    d2::weather::loadResources(d2::sharedVfs());
    SUNITPROXY_FillGlobalItemCache();
    gpCombatTraceCallback = captureCombat;
    gpCombatHitRollTraceCallback = captureHitRoll;
    g_d2webOnDeadRegen = recordDeadRegen;
    return 1;
}
EMSCRIPTEN_KEEPALIVE void d2_monspawn_reset() {
    g_sessionAiThinkWatchUnit = UINT32_MAX;
    g_sessionAiThinkWatchCount = 0; g_sessionAiThinkWatchLastFrame = -1;
    g_sessionAiThinkWatchHasPath = false;
    d2::weather::reset();
    d2_session_object_pop_seed_override_reset();
    // Force the next level call through makeGame even when seed/difficulty are unchanged.
    // The differential fixtures use one fresh retail process per record.
    detachTargetFixture(); g_gameSeed = ~0u; g_gameDiff = -1; g_lastLevel = nullptr;
    g_currentAct = g_currentLevel = -1; g_combatEvents.clear(); g_sessionHitRolls.clear();
    memset(&g_combatResult, 0, sizeof g_combatResult);
    // The creation and population logs are "what happened in THIS session", not "what has happened on
    // this page": `d2_session_room_create_log` is documented as every room created, and a scenario that
    // creates several levels in a row was reading the sum of all of them. s247 measured 12 / 24 / 85 /
    // 96 rooms for the SAME level across its four fixtures, each asking the engine for the identical
    // thing, and called that "our session creates every room of the level up front" -- the number was
    // four fixtures' rooms, and the level's whole grid is 48. d2_session_destroy calls this too, so a
    // released session's log is empty and a new one starts at zero.
    { extern int32_t g_d2CreateCount; extern int32_t g_d2SeqCount; g_d2CreateCount = 0; g_d2SeqCount = 0; }
}

// Loads (seed, difficulty) once; returns the game's act for `act` (0-based), loading it like
// LEVEL_LoadAct. Acts stay loaded for the game's lifetime; a new (seed, difficulty) frees the old game.
static D2DrlgActStrc* gameAct(uint32_t seed, int difficulty, int act) {
    if (!g_game || g_gameSeed != seed || g_gameDiff != difficulty) {
        if (g_game) { for (int a = 0; a < 5; a++) if (g_game->pAct[a]) { DUNGEON_FreeAct(g_game->pAct[a]); g_game->pAct[a] = nullptr; } }
        g_game = makeGame(seed, difficulty); g_gameSeed = seed; g_gameDiff = difficulty;
    }
    if (!g_game->pAct[act]) LEVEL_LoadAct(g_game, (uint8_t)act);
    return g_game->pAct[act];
}

// One level: returns 1 and fills d2_monspawn_out() (units, monsters, hash); 0 on failure.
int d2_trace_level = -1;   // D2_MONS_TRACE builds print for this level id (scenario sets it from D2_MONS_TRACE_LEVEL)
EMSCRIPTEN_KEEPALIVE void d2_monspawn_trace(int levelId) { d2_trace_level = levelId; }
static bool d2_debug_trace_c3r = false;  // scratch, C3r's per-tick queued-unit-flush trace (fprintf, live.log)
EMSCRIPTEN_KEEPALIVE void d2_debug_c3r(int on) { d2_debug_trace_c3r = on != 0; }
// F124 diagnosis: the Hellforge Hammer's mode/invpage the instant it is
// created by ACT4Q3_Callback08_MonsterKilled / D2GAME_DropItemAtUnit_6FC4FEC0, read directly (not
// through console, which the per-frame AudioContext spam pushes out of the 80-line tail). Packed as
// [callbackCalls, notIntro, dropCalls, createdId, createdMode, createdInvPage, createdX, createdY].
int32_t g_f124Trace[8] = {0, -1, 0, 0, -1, -1, 0, 0};
EMSCRIPTEN_KEEPALIVE const int32_t* d2_debug_f124() { return g_f124Trace; }
// Room population order for d2_monspawn_level: subtile origins {x, y} populated first (empty: list order).
static std::vector<int32_t> g_monsOrder;
static std::vector<int32_t> g_monsCreate, g_monsSeq;
EMSCRIPTEN_KEEPALIVE void d2_monspawn_set_sequence(const int32_t* ops, int32_t n) { g_monsSeq.assign(ops, ops + 4 * (n > 0 ? n : 0)); }
EMSCRIPTEN_KEEPALIVE void d2_monspawn_set_create_order(const int32_t* lxy, int32_t n) { g_monsCreate.assign(lxy, lxy + 3 * (n > 0 ? n : 0)); }
EMSCRIPTEN_KEEPALIVE void d2_monspawn_set_order(const int32_t* xy, int32_t n) { g_monsOrder.assign(xy, xy + 2 * (n > 0 ? n : 0)); }

extern "C" bool g_sessionJoinUnits;   // defined below; the local game's client join (d2_session_join_units)
static void updateSessionPendingRooms();
static int g_sessionAttachAtX = -1, g_sessionAttachAtY = -1;   // >= 0 only around a fixture create at a given point
static int g_sessionStartX = -1, g_sessionStartY = -1;   // F67: the town's start point, set by d2_monspawn_level
// Retail's client holds the 3x3 rooms around the town's start room (a retail recording: 15 class-0x25 objects, all inside it), not the 7x7 status union.
static int g_townSpawnX = -1, g_townSpawnY = -1;   // the town's act-start point of any session create (lazy or eager)
static bool inLatticeAt(const D2DrlgRoomStrc* r, int cx, int cy) {
    for (const D2DrlgRoomStrc* c = r->pLevel->pFirstRoomEx; c; c = c->pDrlgRoomNext)
        if (cx >= c->nTileXPos && cx < c->nTileXPos + c->nTileWidth && cy >= c->nTileYPos && cy < c->nTileYPos + c->nTileHeight)
            return abs(r->nTileXPos - c->nTileXPos) <= c->nTileWidth && abs(r->nTileYPos - c->nTileYPos) <= c->nTileHeight;
    return false;
}
// the 3x3 rooms around the town's start room, and around the room the player has walked into
static bool inStartLattice(const D2DrlgRoomStrc* r) {
    if (g_townSpawnX >= 0 && inLatticeAt(r, g_townSpawnX / 5, g_townSpawnY / 5)) return true;
    const D2ActiveRoomStrc* cur = g_targetPlayer ? UNITS_GetRoom(g_targetPlayer) : nullptr;
    return cur && cur->pDrlgRoom && cur->pDrlgRoom->pLevel == r->pLevel && inLatticeAt(r, cur->pDrlgRoom->nTileXPos, cur->pDrlgRoom->nTileYPos);
}
EMSCRIPTEN_KEEPALIVE int d2_monspawn_level(uint32_t seed, int difficulty, int act, int levelId) {
    memset(&g_out, 0, sizeof g_out); g_units.clear(); g_aiUnits.clear();
    g_out.seed = seed; g_out.difficulty = (uint8_t)difficulty; g_out.act = (uint8_t)act; g_out.level = (uint16_t)levelId; g_out.hash = D2VEC_FNV_INIT;
    g_aiOut = { seed, (uint8_t)difficulty, (uint8_t)act, (uint16_t)levelId, 0, D2VEC_FNV_INIT, 0 };
    g_currentAct = act; g_currentLevel = levelId; g_combatEvents.clear(); g_sessionHitRolls.clear(); g_sessionWarpPresets.clear();
    D2DrlgActStrc* pAct = gameAct(seed, difficulty, act);
    if (!pAct) { g_err = "act"; return 0; }
    D2DrlgLevelStrc* pLevel = DRLG_GetLevel(pAct->pDrlg, levelId);
    if (!pLevel) { g_err = "level"; return 0; }
    if (!pLevel->pFirstRoomEx) DRLG_InitLevel(pLevel);
    g_lastLevel = pLevel;
    // F254: a from-save create's level is built before the save's quest record is applied, so Harrogath's
    // quest-gated town objects would run their init functions with A5Q3's pre-load defaults. Set the two
    // fields they read from the save's own record here, before any room is populated and without drawing
    // a seed (see `sessionPreloadA5Q3FlagsFromSave`).
    if (g_sessionFromSave) sessionPreloadA5Q3FlagsFromSave(difficulty);
    // d2_monspawn_set_order: those rooms first, in that order (the retail capture tool MONS_ORDER's twin), then
    // the rest in list order; roomIdx stays the list index.
    std::vector<std::pair<D2DrlgRoomStrc*, uint16_t>> order;
    { uint16_t i = 0; for (D2DrlgRoomStrc* r = pLevel->pFirstRoomEx; r; r = r->pDrlgRoomNext) order.push_back({ r, i++ }); }
    // g_monsCreate: {level, subtile x, subtile y} triples, any level of the act (a session's town rooms too).
    for (size_t k = 0; k + 2 < g_monsCreate.size(); k += 3) {
        D2DrlgLevelStrc* lv = DRLG_GetLevel(pAct->pDrlg, g_monsCreate[k]); if (!lv) continue;
        if (!lv->pFirstRoomEx) DRLG_InitLevel(lv);
        for (D2DrlgRoomStrc* r = lv->pFirstRoomEx; r; r = r->pDrlgRoomNext)
            if (r->nTileXPos * 5 == g_monsCreate[k + 1] && r->nTileYPos * 5 == g_monsCreate[k + 2]) { if (!r->pRoom) DUNGEON_SetClientIsInSight(pAct, g_monsCreate[k], r->nTileXPos, r->nTileYPos, nullptr); break; }
    }
    if (!g_monsOrder.empty()) {
        std::vector<std::pair<D2DrlgRoomStrc*, uint16_t>> first;
        for (size_t k = 0; k + 1 < g_monsOrder.size(); k += 2) for (auto it = order.begin(); it != order.end(); ++it)
            if (it->first->nTileXPos * 5 == g_monsOrder[k] && it->first->nTileYPos * 5 == g_monsOrder[k + 1]) { first.push_back(*it); order.erase(it); break; }
        first.insert(first.end(), order.begin(), order.end()); order.swap(first);
    }
    // d2_monspawn_set_sequence: {op, level, x, y}: op 0 creates that room (in sight), op 1 is the next
    // room populated -- the creations before it are made first, as a walking session interleaves them.
    size_t seqAt = 0;
    auto runCreatesUntil = [&](D2DrlgRoomStrc* next) {
        for (; seqAt + 3 < g_monsSeq.size(); seqAt += 4) {
            const int32_t op = g_monsSeq[seqAt], lv = g_monsSeq[seqAt + 1], x = g_monsSeq[seqAt + 2], y = g_monsSeq[seqAt + 3];
            if (op == 1) { if (next && lv == levelId && next->nTileXPos * 5 == x && next->nTileYPos * 5 == y) { seqAt += 4; return; } continue; }
            D2DrlgLevelStrc* l = DRLG_GetLevel(pAct->pDrlg, lv); if (!l) continue;
            if (!l->pFirstRoomEx) DRLG_InitLevel(l);
            for (D2DrlgRoomStrc* r = l->pFirstRoomEx; r; r = r->pDrlgRoomNext)
                if (r->nTileXPos * 5 == x && r->nTileYPos * 5 == y) { if (!r->pRoom) DUNGEON_SetClientIsInSight(pAct, lv, r->nTileXPos, r->nTileYPos, nullptr); break; }
        }
    };
    if (!g_monsSeq.empty()) {   // the populate order is the sequence's op-1 rooms of this level
        std::vector<std::pair<D2DrlgRoomStrc*, uint16_t>> first;
        for (size_t k = 0; k + 3 < g_monsSeq.size(); k += 4) if (g_monsSeq[k] == 1 && g_monsSeq[k + 1] == levelId)
            for (auto it = order.begin(); it != order.end(); ++it)
                if (it->first->nTileXPos * 5 == g_monsSeq[k + 2] && it->first->nTileYPos * 5 == g_monsSeq[k + 3]) { first.push_back(*it); order.erase(it); break; }
        first.insert(first.end(), order.begin(), order.end()); order.swap(first);
    }
    // F64: a SESSION's create stops here. Retail's client activates a room when the player's sight
    // wave reaches it and the per-frame DRLGACTIVATE_Update builds the rest (F53), so a create that
    // walks the level's own room list and calls `DUNGEON_SetClientIsInSight` on every room of it
    // builds the level in an order no player ever had. What a create still builds is the fixture's
    // LANDING -- the rooms from the room-list head down to the first one holding a monster the
    // anchor search can stand the player beside, which is where `d2_aitarget_attach` puts it either
    // way -- and nothing after that; the wave owns the rest. The retail-vector fixtures never set
    // `g_monspawnSessionCreate`, and `d2_session_activation_wave(0)` restores the eager build.
    //
    // A TOWN is the one level retail builds whole: the capture's own draw-hook frames show all of
    // the Kurast Docks up before the walk moved, where the Spider Forest's rooms stream in five rows
    // as the wave reaches them. A town is also where a session's NPCs,
    // exits and vendors live, and every fixture that reads one right after the create does so; so
    // the wave owns the wilderness and the town is built as it always was.
    const bool town = (int)levelId == DUNGEON_GetTownLevelIdFromActNo(act);
    const bool sessionWave = g_monspawnSessionCreate && g_sessionActivationWave && !town;
    bool sessionLanding = !sessionWave;
    // F67: a session's TOWN starts the player where retail's game-create does, the act's start point
    // (DUNGEON_FindActSpawnLocationEx tile index 0, what LEVEL_ChangeAct uses); it builds the start
    // room FIRST so the wave follows from it, and `d2_aitarget_attach` stands the player there
    // instead of beside the first NPC of the room list. Levels with no session create are untouched.
    // The local game's join (g_sessionJoinUnits): retail's town at the first frame holds only the 3x3 rooms around the
    // act's start room. They are allocated in the sight-call order (centre, then the lattice), the player is placed at the
    // start point at once, and the pending-rooms pass populates them last-allocated first (join-field: monster ids 1..0xb).
    const bool sessionJoin = town && g_sessionJoinUnits && g_sessionCreating && g_townCentreX < 0 && g_sessionAttachAtX < 0;   // not a fixture create at a given point (the recorded sessions' create keeps its own room set)
    g_sessionStartX = g_sessionStartY = -1;
    g_townSpawnX = -1;
    if (town && ((g_monspawnSessionCreate && g_sessionActivationWave) || sessionJoin)) {
        int sx = 0, sy = 0;
        if (DUNGEON_FindActSpawnLocationEx(pAct, levelId, 0, &sx, &sy, 1)) { g_sessionStartX = sx; g_sessionStartY = sy; }
    }
    // T12: retail builds a town in the ORDER its sight calls allocate rooms, and the order is what a room's
    // shared-edge cells see (a neighbour built already merges, one not yet built is rolled on THIS room's seed):
    // 17 rolls of difference in the first room, so a wrong order shifts every later roll of a seed. Measured
    // against retail's own per-room GetTiles stream (lut-arrival-r23.picks, s344): the room the hero stands in,
    // then its 3x3 lattice in D2Client's sight-call order, then the rooms the lattice's near-lists first
    // reach (the ring the per-frame wave dribbles), each in the near-list's own order; the rest as before.
    // T12 (668dc941): the reorder is for a LAZY create or a create that RESUMES A SAVE (`g_sessionFromSave`) -- the
    // chain's own town arrivals, whose tile is the act start. 668dc941 wrote `g_sessionStartX >= 0` for the second
    // half, but that is set only on the lazy path (above), so the guard collapsed to `g_sessionLazy`, the chain's
    // eager town create -- the save resume, through `_class_bare` -- lost the reorder and with it every later roll
    // of the town's seed (s000d step 3's world hash). Reordering is scoped to the save path because that is the one
    // arrival T12a's own message and HANDOFF line 44 scope it to ("a save's own arrival"), and because the two rows
    // that assert the Rogue Encampment fixtures' order -- s161's shop and s973's merc follow -- fail when a
    // new-character create is reordered and pass when it is not.
    std::vector<D2DrlgRoomStrc*> joinSeq;
    int orderX = g_townCentreX * 5, orderY = g_townCentreY * 5;
    if (town && g_sessionCreating && g_townCentreX < 0 && (g_sessionLazy || g_sessionFromSave || sessionJoin)) { int sx = 0, sy = 0; if (DUNGEON_FindActSpawnLocationEx(pAct, levelId, 0, &sx, &sy, 1)) { orderX = sx; orderY = sy; } else orderX = -1; }
    if (town && g_sessionCreating && orderX >= 0 && (g_sessionLazy || g_sessionStartX >= 0 || g_sessionFromSave || g_townCentreX >= 0)) {
        const int cx = orderX / 5, cy = orderY / 5;
        std::vector<D2DrlgRoomStrc*> seq;
        auto add = [&](D2DrlgRoomStrc* r) { if (r && r->pLevel == pLevel && std::find(seq.begin(), seq.end(), r) == seq.end()) seq.push_back(r); };
        D2DrlgRoomStrc* centre = nullptr;
        for (auto& e : order) { D2DrlgRoomStrc* r = e.first;
            if (cx >= r->nTileXPos && cx < r->nTileXPos + r->nTileWidth && cy >= r->nTileYPos && cy < r->nTileYPos + r->nTileHeight) { centre = r; break; } }
        if (centre) {
            add(centre);
            static const int8_t lat[8][2] = { {-1,-1}, {-1,0}, {0,-1}, {1,-1}, {-1,1}, {1,0}, {0,1}, {1,1} };
            for (auto& d : lat) for (auto& e : order)
                if (e.first->nTileXPos == centre->nTileXPos + d[0] * centre->nTileWidth && e.first->nTileYPos == centre->nTileYPos + d[1] * centre->nTileHeight) { add(e.first); break; }
            const size_t lattice = seq.size();
            for (size_t k = 0; k < (g_townCentreX >= 0 ? lattice : 0); ++k) {   // F67: the lattice's near-ring only for an arrival BY TRANSITION (a caller-given centre: Lut Gholein, s344); a game-create at the act start builds the rest of the town in the level's list order (Kurast Docks: 9 lattice rooms, then the other 39 in list order, act3-docks-forest-frames)
                D2DrlgRoomStrc* r = seq[k];
                if (!r->nRoomsNear) sub_6FD77BB0(pLevel->pDrlg->pMempool, r);
                for (int i = 0; i < r->nRoomsNear; ++i) add(r->ppRoomsNear[i]);
            }
            for (size_t w = 0; w + 1 < g_townWalk.size(); w += 2) {   // the walk: each centre, then its lattice in the same order
                D2DrlgRoomStrc* wc = nullptr;
                for (auto& e : order) { D2DrlgRoomStrc* r = e.first;
                    if (g_townWalk[w] >= r->nTileXPos && g_townWalk[w] < r->nTileXPos + r->nTileWidth && g_townWalk[w + 1] >= r->nTileYPos && g_townWalk[w + 1] < r->nTileYPos + r->nTileHeight) { wc = r; break; } }
                if (!wc) continue;
                add(wc);
                for (auto& d : lat) for (auto& e : order)
                    if (e.first->nTileXPos == wc->nTileXPos + d[0] * wc->nTileWidth && e.first->nTileYPos == wc->nTileYPos + d[1] * wc->nTileHeight) { add(e.first); break; }
            }
            if (sessionJoin) joinSeq = seq;
            std::vector<std::pair<D2DrlgRoomStrc*, uint16_t>> first;
            for (D2DrlgRoomStrc* r : seq) for (auto it = order.begin(); it != order.end(); ++it) if (it->first == r) { first.push_back(*it); order.erase(it); break; }
            first.insert(first.end(), order.begin(), order.end()); order.swap(first);
        }
    }
    if (sessionJoin && !joinSeq.empty()) {
        for (D2DrlgRoomStrc* r : joinSeq) if (!r->pRoom) DUNGEON_SetClientIsInSight(pAct, levelId, r->nTileXPos, r->nTileYPos, nullptr);
        updateSessionPendingRooms();
    }
    for (auto [pRoomEx, roomIdx] : order) {
        if (!g_monsSeq.empty()) runCreatesUntil(pRoomEx);
        // The warp list is collected for EVERY room of the level, built or not: an exit's point is
        // generation-time DRLG data (F52, `sessionWarpPointFromDs1`), not something the room's tile
        // grid answers, and a room the wave has not reached still has to be a destination. Skipping
        // this for the rooms past the landing left the session with NO exits at all (s88's act III
        // route read an empty warp list and could not cross the seam).
        for (D2PresetUnitStrc* preset = pRoomEx->pPresetUnits; preset; preset = preset->pNext) if (preset->nUnitType == UNIT_TILE) {
            g_sessionWarpPresets.push_back({ roomIdx, preset->nIndex,
                pRoomEx->nTileXPos * 5 + preset->nXpos, pRoomEx->nTileYPos * 5 + preset->nYpos, sessionWarpTileDestination(pRoomEx, preset->nIndex) });
        }
        if (!pRoomEx->pRoom && !(sessionWave && sessionLanding) && !(sessionJoin && !joinSeq.empty()))
            DUNGEON_SetClientIsInSight(pAct, levelId, pRoomEx->nTileXPos, pRoomEx->nTileYPos, nullptr);
        D2ActiveRoomStrc* pRoom = pRoomEx->pRoom;
        if (!pRoom) continue;
        for (D2UnitStrc* tile = pRoom->pUnitFirst; tile; tile = tile->pRoomNext) if (tile->dwUnitType == UNIT_TILE && tile->pStaticPath) {
            g_sessionWarpPresets.push_back({ roomIdx, (int32_t)tile->dwClassId,
                tile->pStaticPath->tGameCoords.nX, tile->pStaticPath->tGameCoords.nY, sessionWarpTileDestination(pRoomEx, (int32_t)tile->dwClassId) });
        }
#ifdef D2_MONS_TRACE
        if (levelId == d2_trace_level) fprintf(stderr, "room %u type %d flags %x logical %p lists %d/%x\n", roomIdx, pRoomEx->nType, pRoomEx->dwFlags, (void*)pRoomEx->pLogicalRoomInfo, pRoomEx->pLogicalRoomInfo ? pRoomEx->pLogicalRoomInfo->nLists : -1, pRoomEx->pLogicalRoomInfo ? pRoomEx->pLogicalRoomInfo->dwFlags : 0);
#endif
        { D2VecMonUnit m = {}; m.seed = seed; m.difficulty = (uint8_t)difficulty; m.act = (uint8_t)act; m.level = (uint16_t)levelId; m.room = roomIdx; m.unitType = 254; m.seedLo = pRoom->pSeed.nLowSeed; m.seedHi = pRoom->pSeed.nHighSeed; g_units.push_back(m); }
#ifdef D2_MONS_TRACE
        if (levelId == d2_trace_level) fprintf(stderr, "[room] %u [%d,%d] gameseed %u/%u objseed %u/%u roomseed %u\n", roomIdx, pRoom->tCoords.nSubtileX, pRoom->tCoords.nSubtileY, g_game->pGameSeed.nLowSeed, g_game->pGameSeed.nHighSeed, g_game->pObjectControl->pSeed.nLowSeed, g_game->pObjectControl->pSeed.nHighSeed, pRoom->pSeed.nLowSeed);
#endif
        if (!(sessionJoin && !joinSeq.empty())) sub_6FC385A0(g_game, pRoom);   // D2Game 1.13c 0x6FC4CB80 room activation (compat/game_glue.cpp); the join's pass ran above
        // The landing: the first room of the room-list order whose population put a monster in it
        // is the one `d2_aitarget_attach` will stand the fixture's player in. This is the room
        // reach of a create; every later room is the activation wave's to build (F64).
        //
        // The predicate has to be the SEARCH's own condition, not "a monster of any class": the
        // attach walks the same list for a room no client holds and a monster it accepts
        // (`g_sessionAnchorClass`, set by `d2_session_create_raw_near`), so a landing that stopped
        // one room early on a class the caller did not ask for cuts the level before the room the
        // anchor is in and the attach then finds nothing at all (Codex review).
        if (sessionWave && !sessionLanding && !pRoom->nNumClients)
            for (D2UnitStrc* u = pRoom->pUnitFirst; u; u = u->pRoomNext)
                if (u->dwUnitType == UNIT_MONSTER && (g_sessionAnchorClass == 0xFFFFFFFF || (uint32_t)u->dwClassId == g_sessionAnchorClass)) { sessionLanding = true; break; }
        { D2VecMonUnit m = {}; m.seed = seed; m.difficulty = (uint8_t)difficulty; m.act = (uint8_t)act; m.level = (uint16_t)levelId; m.room = roomIdx; m.unitType = 255;   // diagnosis: room seed after activation, not hashed
          m.seedLo = pRoom->pSeed.nLowSeed; m.seedHi = pRoom->pSeed.nHighSeed; m.x = (uint16_t)pRoom->tCoords.nSubtileX; m.y = (uint16_t)pRoom->tCoords.nSubtileY; g_units.push_back(m); }
        uint16_t monOrdinal = 0;
        for (D2UnitStrc* u = pRoom->pUnitFirst; u; u = u->pRoomNext) {
            D2VecMonUnit m = {};
            m.seed = seed; m.difficulty = (uint8_t)difficulty; m.act = (uint8_t)act; m.level = (uint16_t)levelId; m.room = roomIdx;
            m.unitType = (uint8_t)u->dwUnitType; m.classId = u->dwClassId; m.mode = (uint8_t)u->dwAnimMode;
            m.seedLo = u->pSeed.nLowSeed; m.seedHi = u->pSeed.nHighSeed;
            if (u->dwUnitType == UNIT_MONSTER || u->dwUnitType == UNIT_PLAYER || u->dwUnitType == UNIT_MISSILE) { m.x = (uint16_t)u->pDynamicPath->tGameCoords.wPosX; m.y = (uint16_t)u->pDynamicPath->tGameCoords.wPosY; }
            // tGameCoords, not dwClientCoordX/Y (a different, client-pixel-scale pair): capture's
            // safePopulateLevel matched this file's own bug until the town-portal
            // capture proved +0x0C/+0x10 (tGameCoords) is the game subtile. Both sides now read it.
            else if (u->pStaticPath) { m.x = (uint16_t)u->pStaticPath->tGameCoords.nX; m.y = (uint16_t)u->pStaticPath->tGameCoords.nY; }
            if (u->dwUnitType == UNIT_MONSTER && u->pMonsterData) {
                // Native wasm64 pointers shift the runtime fields past their retail x86
                // offsets. Serialize the named fields into the fixed-width vector record.
                const D2MonsterDataStrc* md = u->pMonsterData;
                m.monFlags = md->nTypeFlag;
                memcpy(m.enchants, md->nMonUmod, sizeof(m.enchants));
                m.uniqueNo = md->wBossHcIdx;
                m.hp = (uint32_t)STATLIST_UnitGetStatValue(u, STAT_HITPOINTS, 0); m.maxHp = (uint32_t)STATLIST_UnitGetStatValue(u, STAT_MAXHP, 0); m.lvl = (uint32_t)STATLIST_UnitGetStatValue(u, STAT_LEVEL, 0);
            }
            g_out.hash = d2vec_mon_hash(g_out.hash, &m);
            g_out.units++; if (m.unitType == UNIT_MONSTER) g_out.monsters++;
            g_units.push_back(m);
            if (u->dwUnitType == UNIT_MONSTER) {
                D2VecAiUnit a = captureAi(u, roomIdx, monOrdinal++);
                g_aiOut.hash = d2vec_ai_hash(g_aiOut.hash, &a); g_aiOut.monsters++; g_aiUnits.push_back(a);
            }
        }
    }
    return 1;
}
EMSCRIPTEN_KEEPALIVE const D2VecMonLevel* d2_monspawn_out() { return &g_out; }
EMSCRIPTEN_KEEPALIVE const D2VecMonUnit* d2_monspawn_units(int* count) { *count = (int)g_units.size(); return g_units.data(); }
EMSCRIPTEN_KEEPALIVE const char* d2_monspawn_err() { return g_err.c_str(); }
EMSCRIPTEN_KEEPALIVE const D2VecAiLevel* d2_aispawn_out() { return &g_aiOut; }
EMSCRIPTEN_KEEPALIVE const D2VecAiUnit* d2_aispawn_units(int* count) { *count = (int)g_aiUnits.size(); return g_aiUnits.data(); }
// Place the session player at a standable subtile of `room` and wire up its client. Split out of
// d2_aitarget_attach so a level with no monster at all can still be entered: the anchor search
// below is what the AI oracle rows need, not what a player needs.
bool g_sessionClientStaysInRoom = false; // read by sub_6FC33670
static bool g_sessionClientAdjCount = false; // d2_session_client_feed(2), see below
static bool g_fixtureOwnRoomOnly = false;
// The capture's D2CAPTURE_SESSION_CLIENT join (the retail capture tool/main.cpp "capture-client") lists the client in its own room and
// then in every built room of the level through D2Common #10879 (0x3C460) directly -- no LEVEL_AddClient, so no unit sends.
// a retail recording sets it: retail's monster b sits in a room only that join reaches, and AI scans client rooms only (D2Game+0xB0CE0).
static bool g_sessionJoinLevelRooms = false;
EMSCRIPTEN_KEEPALIVE void d2_session_client_join_level(int on) { g_sessionJoinLevelRooms = on != 0; }
// What the join walk sent the client (room reveals, then the units in them) before the stream is cleared for
// tick 1: a connected client is owed these bytes, a headless session never reads them.
extern "C" const uint8_t* D2WEB_PacketStream(uint32_t* pLength, uint32_t* pPackets);
static std::vector<uint8_t> g_sessionJoinBurst;
static uint32_t sessionPacketStreamMark() { uint32_t n = 0, k = 0; D2WEB_PacketStream(&n, &k); return n; }
static void sessionKeepJoinBurst(uint32_t from) {
    uint32_t n = 0, k = 0; const uint8_t* stream = D2WEB_PacketStream(&n, &k);
    g_sessionJoinBurst.assign(stream + (from < n ? from : n), stream + n);
}
// How many leading bytes of the join are the load part (game flags and the two markers), sent when the
// client logs on; the rest follows when it enters the game.
static uint32_t g_sessionJoinLoadLength = 0;
// One client path: a joining client is told every monster in the rooms around it (retail's LEVEL_AddClient sends each unit of an
// adjacent room, the 0xAC assign first). Off for the headless sessions, whose join drops those bytes.
extern "C" bool g_sessionJoinUnits = false;   // the local game's client is told each monster of every room it joins (sub_6FC33020)
EMSCRIPTEN_KEEPALIVE void d2_session_join_units(int on) { g_sessionJoinUnits = on != 0; }
// How the join walk reveals rooms to the local game's client before their units (see sub_6FC33020); 0 is off.
extern "C" int g_joinWalkRevealMode;
static int g_sessionJoinRevealMode = 0;
EMSCRIPTEN_KEEPALIVE void d2_session_join_reveal(int mode) { g_sessionJoinRevealMode = mode < 0 ? 0 : mode; }
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_join_load_length() { return g_sessionJoinLoadLength; }
// For the local game's join: the connected client's enter-game request marks its player spawned, and the
// frame's client update then sends the "in game" marker once the room is ready. Returns 1 when marked.
int32_t __fastcall D2GAME_UpdateSpawnedClient(D2GameStrc* pGame, D2ClientStrc* pClient);
// The game's own frame count, alone: what the snapshot's frame word holds, without building a snapshot.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_game_frame() { return g_game ? g_game->dwGameFrame : 0; }
EMSCRIPTEN_KEEPALIVE int d2_session_client_spawned() {
    if (!g_game || !g_targetClient || !g_targetPlayer) return 0;
    g_targetClient->dwClientState = CLIENTSTATE_PLAYER_SPAWNED;
    return 1;
}
// For the local game's join: the skill list (0x94) and both hands' selection (0x23, left then right), as retail's
// player load and ClientAddPlayerToGame send them. Sent live after the join bytes, so skills the session granted
// after the player attached are in the list; without it the client drops every 0x23. Returns 1 when sent.
EMSCRIPTEN_KEEPALIVE int d2_session_client_skills() {
    if (!g_targetClient || !g_targetPlayer || !g_targetPlayer->pSkills) return 0;
    D2GAME_8B5A0_SendSkills(g_targetClient, g_targetPlayer);
    // The hotkey slots (0x7B), the loop retail's ClientAddPlayerToGame runs before the selected skills (D2Game+0x4BE4B).
    for (int i = 0; i < 16; ++i) {
        const D2ClientKeySkillStrc& key = g_targetClient->HotkeySkills[i];
        if (key.nSkill >= 0 && key.nSkill < sgptDataTables->nSkillsTxtRecordCount)
            D2GAME_PACKETS_SendPacket0x7B_6FC3F720(g_targetClient, (BYTE)i, key.nSkill, (uint8_t)key.nHand, (int32_t)key.nItemGUID);
    }
    for (int left = 1; left >= 0; --left) {
        D2SkillStrc* s = left ? UNITS_GetLeftSkill(g_targetPlayer) : UNITS_GetRightSkill(g_targetPlayer);
        if (s) D2GAME_PACKETS_SendPacket0x23_6FC3DC60(g_targetClient, UNIT_PLAYER, g_targetPlayer->dwUnitId, (uint8_t)left,
                   (uint16_t)SKILLS_GetSkillIdFromSkill(s, __FILE__, __LINE__), SKILLS_GetOwnerGUIDFromSkill(s));
    }
    return 1;
}
EMSCRIPTEN_KEEPALIVE const uint8_t* d2_session_join_stream(uint32_t* length) { if (length) *length = (uint32_t)g_sessionJoinBurst.size(); return g_sessionJoinBurst.data(); }
// The rooms the game holds its client in, as the 6-byte room reveals {0x07, tile x, tile y, level} a joining
// client is sent: the client's own room first, then the rest of its adjacent list. Returns the byte count.
EMSCRIPTEN_KEEPALIVE int d2_session_client_rooms(uint8_t* out, int capacity) {
    if (!out || !g_targetClient || !g_targetClient->pRoom) return 0;
    int n = 0;
    const auto reveal = [&](D2ActiveRoomStrc* r) {
        int k = 0; while (k < r->nNumClients && r->ppClients[k] != g_targetClient) ++k;
        if (k >= r->nNumClients || n + 6 > capacity) return;
        D2DrlgCoordsStrc c{}; DUNGEON_GetRoomCoordinates(r, &c);
        out[n] = 0x07; out[n + 1] = (uint8_t)c.nTileXPos; out[n + 2] = (uint8_t)(c.nTileXPos >> 8);
        out[n + 3] = (uint8_t)c.nTileYPos; out[n + 4] = (uint8_t)(c.nTileYPos >> 8); out[n + 5] = (uint8_t)DUNGEON_GetLevelIdFromRoom(r);
        n += 6;
    };
    D2ActiveRoomStrc* own = g_targetClient->pRoom;
    reveal(own);
    D2ActiveRoomStrc** adj = nullptr; int32_t nAdj = 0; DUNGEON_GetAdjacentRoomsListFromRoom(own, &adj, &nAdj);
    for (int32_t i = 0; i < nAdj; ++i) if (adj[i] && adj[i] != own) reveal(adj[i]);
    return n;
}
// The objects of every room the game holds its client in, sent as the room join sends them (the unit loop of
// 1.13c D2Game+0x9DC10): the session's own join ran before a client was connected, when the object sender is
// silent. Objects only: their first-sight send writes no game state. Returns the count sent.
EMSCRIPTEN_KEEPALIVE int d2_session_client_room_objects() {
    if (!g_game || !g_targetClient || !g_targetClient->pRoom) return 0;
    int n = 0;
    // Every room the client holds, as the snapshot's own test counts them: a client in the room, or the 3x3 lattice
    // around the start room (the client-side model of the rooms retail's server keeps the client in).
    for (D2DrlgRoomStrc* roomEx = g_lastLevel ? g_lastLevel->pFirstRoomEx : nullptr; roomEx; roomEx = roomEx->pDrlgRoomNext) {
        D2ActiveRoomStrc* r = roomEx->pRoom;
        if (!r || !(clientHeldCount(r) > 0 || inStartLattice(roomEx))) continue;
        for (D2UnitStrc* u = r->pUnitFirst; u; u = u->pRoomNext)
            if (u->dwUnitType == UNIT_OBJECT && u->pObjectData && u->pObjectData->pObjectTxt) { D2GAME_SUNITMSG_FirstFn_6FCC5520(g_game, u, g_targetClient); ++n; }
    }
    return n;
}
// The live game's map seed, and its difficulty in the low byte with the expansion flag above it
// and the player's class in the third byte: what a client joining this game is told. 0 with no game.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_game_seed() { return g_game ? g_game->dwInitSeed : 0; }
EMSCRIPTEN_KEEPALIVE int d2_session_game_flags() { return g_game ? (int)(g_game->nDifficulty & 0xFF) | (g_game->bExpansion ? 0x100 : 0) | (g_targetPlayer ? (int)(g_targetPlayer->dwClassId & 0xFF) << 16 : 0) : 0; }
static int attachSessionPlayerAt(D2ActiveRoomStrc* room, int x, int y, D2UnitStrc* anchor) {
    const uint32_t joinBurstFrom = sessionPacketStreamMark();
    D2UnitStrc* player = SUNIT_AllocUnitData(UNIT_PLAYER, g_sessionPlayerClass, x, y, g_game, room, 1, PLRMODE_NEUTRAL, 0);
    if (!player) return 0;
    if (g_sessionSeedPlayer) SUNIT_InitSeed(player, &g_game->pGameSeed);
    STATLIST_SetUnitStat(player, STAT_HITPOINTS, 50 << 8, 0); STATLIST_SetUnitStat(player, STAT_MAXHP, 50 << 8, 0);
    STATLIST_SetUnitStat(player, STAT_LEVEL, 1, 0); STATLIST_SetUnitStat(player, STAT_ATTACKRATE, 100, 0);
    STATLIST_SetUnitStat(player, STAT_VELOCITYPERCENT, 100, 0); STATLIST_SetUnitStat(player, STAT_OTHER_ANIMRATE, 100, 0);
    g_targetClient = (D2ClientStrc*)calloc(1, sizeof(D2ClientStrc));
    g_targetClient->pGame = g_game; g_targetClient->pPlayer = player;
    g_targetClient->dwUnitType = UNIT_PLAYER; g_targetClient->dwUnitGUID = player->dwUnitId;   // D48: the 0x9C composer re-resolves the player through these
    g_targetClient->nAct = (uint8_t)g_currentAct;
    // The save header reads the character's identity from the client, not the unit: without
    // a name it refuses to write at all, and the class it stores is this field.
    g_targetClient->nClassId = (uint8_t)g_sessionPlayerClass;
    // The save flags are the CLIENT's, not the game's: PLRSAVE2_WriteSaveHeader writes
    // CLIENTS_GetFlags(pClient) into the header word at 0x24, which is where the creation screen's
    // Expansion/Hardcore checkboxes end up in the .d2s.
    //
    // This also carries the correction C3p needed, and it is why the bit must not be left zero: the
    // game is created FROM these flags, so `g_game->bExpansion` is derived from them and the two
    // cannot disagree -- but an expansion character counted classic acts per difficulty (4, not 5)
    // while the flags were zero, so a Baal kill one difficulty up saved progression 9 instead of 10
    // and refused the character its own Hell game.
    CLIENTS_SetFlags(g_targetClient, (int32_t)g_sessionClientSaveFlags);
    memset(g_targetClient->szName, 0, sizeof g_targetClient->szName);
    memcpy(g_targetClient->szName, g_sessionCharName, sizeof g_targetClient->szName - 1);
    // Deterministic by design: a wall-clock creation stamp would make every saved file
    // differ from the last and defeat byte comparison in a scenario.
    g_targetClient->nCreateTime = 0;
    // Retail marks an unbound hotkey with nSkill == 0xFFFF (PlrSaveFile.cpp reads the same sentinel
    // back at load); zero-initialized memory left it 0, which the save header wrote as a bound skill 0
    // (Attack) on every slot instead of "nothing bound here" (found diffing retailc-paladin-fresh.d2s).
    for (int hk = 0; hk < 16; hk++) {
        g_targetClient->HotkeySkills[hk].nSkill = (int16_t)-1;
        g_targetClient->HotkeySkills[hk].nItemGUID = (uint32_t)-1;
    }
    UNITS_GetPlayerData(player)->pClient = g_targetClient;
    // The capture's fixture (the retail capture tool/main.cpp "player fixture") hangs its client on its OWN room only
    // (ppClients +0x48, nNumClients +0x78 = 1) before any AI event is scheduled, then wakes that room's
    // monsters; a real client (sub_6FC33020) joins every adjacent room. Monster AI scans only rooms with
    // clients (retail 0x6FCD0D57), so the aitrace replays (d2_aitarget_attach_at) register as the capture
    // does: retail's frame-15 probe read nNumClients 0 in the room beside the fixture (s232).
    if (g_fixtureOwnRoomOnly) { DUNGEON_AddClientToRoom(room, g_targetClient); g_targetClient->pRoom = room; }
    // What a joining client is told before the rooms, in retail's order: the game's flags and the two load markers,
    // the act and its time of day, the room it stands in, its own unit, which unit it plays, then its position.
    // Kept with the join walk's bytes below.
    if (D2DrlgActStrc* joinAct = g_game->pAct[g_targetClient->nAct]; joinAct && DUNGEON_GetEnvironmentFromAct(joinAct)) {
        D2GAME_PACKETS_SendPacket0x01_6FC3C7C0(g_targetClient, 1, g_game);
        D2GAME_PACKETS_SendHeaderOnlyPacket(g_targetClient, 0x00);
        D2GAME_PACKETS_SendHeaderOnlyPacket(g_targetClient, 0x02);
        g_sessionJoinLoadLength = sessionPacketStreamMark() - joinBurstFrom;
        LEVEL_SynchronizeDayNightCycleWithClient(g_game, g_targetClient);
        D2DrlgCoordsStrc joinRoom = {};
        DUNGEON_GetRoomCoordinates(room, &joinRoom);
        D2GAME_PACKETS_SendPacket0x07_6FC3D120(g_targetClient, DUNGEON_GetLevelIdFromRoom(room), (uint16_t)joinRoom.nTileXPos, (uint16_t)joinRoom.nTileYPos);
        // The unit is named (0x59) before it is selected (0x0B): the stock server's order, and the one the client's join scan reads.
        sub_6FC3D1F0(g_targetClient, player->dwUnitId, (uint8_t)player->dwClassId, g_targetClient->szName, (int16_t)CLIENTS_GetUnitX(player), (int16_t)CLIENTS_GetUnitY(player));
        D2GAME_SendPacket0x0B_ClientUnit(g_targetClient, player);
        D2GAME_PACKETS_SendPacket0x15_6FC3D0D0(g_targetClient, 0x15u, 0, player->dwUnitId, CLIENTS_GetUnitX(player), CLIENTS_GetUnitY(player), 1);
    } else g_sessionJoinLoadLength = 0;
    g_sessionJoinRooms.clear();
    if (g_fixtureOwnRoomOnly && g_sessionClientAdjCount) {   // a retail recording (feed 2): the adjacent rooms' client arrays only
        D2ActiveRoomStrc** adj = nullptr; int32_t nAdj = 0; DUNGEON_GetAdjacentRoomsListFromRoom(room, &adj, &nAdj);
        for (int32_t i = 0; i < nAdj; ++i) if (adj[i] != room) DUNGEON_AddClientToRoom(adj[i], g_targetClient);
    }
    // Braces: the else used to bind to the inner `if` in the loop, so the join never ran outside the level-join arm.
    // Retail's join walk (1.13c 0x4B1D0 -> LEVEL_AddClient 0x9DC10 on adjacent rooms) runs for a real client.
    if (g_fixtureOwnRoomOnly && g_sessionJoinLevelRooms && g_lastLevel) {
        for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext)
            if (roomEx->pRoom && roomEx->pRoom != room) { DUNGEON_AddClientToRoom(roomEx->pRoom, g_targetClient); g_sessionJoinRooms.insert(roomEx->pRoom); }
    } else {
        g_joinWalkRevealMode = g_sessionJoinUnits ? g_sessionJoinRevealMode : 0;   // the walk only: no flag outlives it
        sub_6FC33020(g_targetClient, room);
        g_joinWalkRevealMode = 0;
    }
    // D48: retail's join walk sends the room's units before the first recorded tick; do it now and drop the bytes so tick 1 starts clean.
    // One-client path: the join also sends the player's changed stats and vitals (retail's add-player-to-game does, before
    // the room walk); they ride in the kept join bytes.
    {
        D2Common_10513(g_targetPlayer, g_targetPlayer, (void(__fastcall*)(D2UnitStrc*, int32_t, int32_t, D2UnitStrc*))D2GAME_UpdateAttribute_6FC822D0);
        sub_6FC82360(g_targetPlayer, g_targetPlayer, 1);
        LEVEL_UpdateUnitsInAdjacentRooms(g_game, room, g_targetClient); sessionKeepJoinBurst(joinBurstFrom); D2WEB_ResetPacketStream();
    }
    for (D2UnitStrc* u = room->pUnitFirst; u; u = u->pRoomNext) {
        if (u->dwUnitType != UNIT_MONSTER) continue;
        if (u->dwClassId == 155) g_sessionAiThinkAttachRoomHasWarriv = 1;
        MONSTER_UpdateAiCallbackEvent(g_game, u);
    }
    g_targetPlayer = player; g_targetMonster = anchor; g_targetRoom = room; return 1;
}

// A further client joining the session's game (a second realm connection, slot 1..3): its own player unit
// beside the session player and its own D2ClientStrc, whose packets the send routes to that connection by
// pClient (D2WEB_RouteClient). The session player and its globals are untouched.
extern "C" void D2WEB_RouteClient(int slot, D2ClientStrc* pClient, void (*sink)(int, const uint8_t*, uint32_t));
struct SessionExtraClient { D2ClientStrc* client; D2UnitStrc* player; D2ActiveRoomStrc* room; };
static SessionExtraClient g_sessionExtraClients[4];
// With the session's own client (detachTargetFixture): the units stay, their clients and routes go. The room
// rosters that held them are cleared there with the session client's.
static void sessionDropExtraClient(int slot) {
    {
        SessionExtraClient& e = g_sessionExtraClients[slot];
        if (!e.client) return;
        D2WEB_RouteClient(slot, nullptr, nullptr);
        // Every room of every loaded act that lists the client (e.room may already be freed, so it is not read).
        for (int a = 0; g_game && a < 5; ++a)
            for (D2ActiveRoomStrc* r = g_game->pAct[a] ? g_game->pAct[a]->pRoom : nullptr; r; r = r->pRoomNext)
                for (int32_t k = 0; k < r->nNumClients; ++k)
                    if (r->ppClients[k] == e.client) { DUNGEON_RemoveClientFromRoom(r, e.client); break; }
        if (e.player && UNITS_GetPlayerData(e.player)) UNITS_GetPlayerData(e.player)->pClient = nullptr;
        free(e.client);
        e = {};
    }
}
static void sessionDropExtraClients() { for (int slot = 1; slot < 4; ++slot) sessionDropExtraClient(slot); }
// A closed connection: its client and route go, so the slot can log on again (the player unit stays in the world).
EMSCRIPTEN_KEEPALIVE void d2_session_drop_client(int slot) { if (slot >= 1 && slot < 4) sessionDropExtraClient(slot); }
static SessionExtraClient* sessionExtraClient(int slot) {
    return slot >= 1 && slot < 4 && g_sessionExtraClients[slot].client && g_game ? &g_sessionExtraClients[slot] : nullptr;
}
// The game logon (0x68): the player is made beside the session player and the client is sent the load part of
// the join (game flags, the two load markers), as attachSessionPlayerAt sends the session client.
EMSCRIPTEN_KEEPALIVE int d2_session_add_client(int slot, int classId, const char* name, void (*sink)(int, const uint8_t*, uint32_t)) {
    if (slot < 1 || slot >= 4 || g_sessionExtraClients[slot].client || !g_sessionLive || !g_game || !g_targetPlayer ||
        !g_targetPlayer->pDynamicPath || classId < 0 || classId >= 7) return 0;
    D2ActiveRoomStrc* home = UNITS_GetRoom(g_targetPlayer);
    const int px = g_targetPlayer->pDynamicPath->tGameCoords.wPosX, py = g_targetPlayer->pDynamicPath->tGameCoords.wPosY;
    D2ActiveRoomStrc* room = nullptr; int x = 0, y = 0;
    for (int radius = 2; radius <= 8 && !room; radius++) for (int dy = -radius; dy <= radius && !room; dy++) for (int dx = -radius; dx <= radius && !room; dx++) {
        if (abs(dx) != radius && abs(dy) != radius) continue;
        D2ActiveRoomStrc* r = COLLISION_GetRoomBySubTileCoordinates(home, px + dx, py + dy);
        if (r && !COLLISION_CheckMaskWithSize(r, px + dx, py + dy, 2, 0xFFFF)) { room = r; x = px + dx; y = py + dy; }
    }
    if (!room) return 0;
    D2UnitStrc* player = SUNIT_AllocUnitData(UNIT_PLAYER, classId, x, y, g_game, room, 1, PLRMODE_NEUTRAL, 0);
    if (!player) return 0;
    if (g_sessionSeedPlayer) SUNIT_InitSeed(player, &g_game->pGameSeed);
    STATLIST_SetUnitStat(player, STAT_HITPOINTS, 50 << 8, 0); STATLIST_SetUnitStat(player, STAT_MAXHP, 50 << 8, 0);
    STATLIST_SetUnitStat(player, STAT_LEVEL, 1, 0); STATLIST_SetUnitStat(player, STAT_ATTACKRATE, 100, 0);
    STATLIST_SetUnitStat(player, STAT_VELOCITYPERCENT, 100, 0); STATLIST_SetUnitStat(player, STAT_OTHER_ANIMRATE, 100, 0);
    D2ClientStrc* client = (D2ClientStrc*)calloc(1, sizeof(D2ClientStrc));
    if (!client) return 0;
    client->pGame = g_game; client->pPlayer = player;
    client->dwUnitType = UNIT_PLAYER; client->dwUnitGUID = player->dwUnitId;
    client->nAct = (uint8_t)g_currentAct; client->nClassId = (uint8_t)classId;
    CLIENTS_SetFlags(client, (int32_t)g_sessionClientSaveFlags);
    if (name) { strncpy(client->szName, name, sizeof client->szName - 1); strncpy(UNITS_GetPlayerData(player)->szName, name, 15); }
    for (int hk = 0; hk < 16; hk++) { client->HotkeySkills[hk].nSkill = (int16_t)-1; client->HotkeySkills[hk].nItemGUID = (uint32_t)-1; }
    UNITS_GetPlayerData(player)->pClient = client;
    g_sessionExtraClients[slot] = { client, player, room };
    D2WEB_RouteClient(slot, client, sink);
    D2GAME_PACKETS_SendPacket0x01_6FC3C7C0(client, 1, g_game);
    D2GAME_PACKETS_SendHeaderOnlyPacket(client, 0x00);
    D2GAME_PACKETS_SendHeaderOnlyPacket(client, 0x02);
    return 1;
}
// Enter game (0x6B): the client's own unit, the act, its room and position, the join walk over the adjacent
// rooms (which puts the client in their client lists, so the room's sends reach it), its stats, then the
// 0x5B player records both ways with every other player in the game (retail 0x6FC369C0's loop) and its own.
EMSCRIPTEN_KEEPALIVE int d2_session_client_enter(int slot) {
    SessionExtraClient* e = sessionExtraClient(slot);
    if (!e) return 0;
    D2ClientStrc* client = e->client; D2UnitStrc* player = e->player; D2ActiveRoomStrc* room = e->room;
    sub_6FC3D1F0(client, player->dwUnitId, (uint8_t)player->dwClassId, client->szName, (int16_t)CLIENTS_GetUnitX(player), (int16_t)CLIENTS_GetUnitY(player));
    D2GAME_SendPacket0x0B_ClientUnit(client, player);
    LEVEL_SynchronizeDayNightCycleWithClient(g_game, client);
    D2DrlgCoordsStrc joinRoom = {};
    DUNGEON_GetRoomCoordinates(room, &joinRoom);
    D2GAME_PACKETS_SendPacket0x07_6FC3D120(client, DUNGEON_GetLevelIdFromRoom(room), (uint16_t)joinRoom.nTileXPos, (uint16_t)joinRoom.nTileYPos);
    D2GAME_PACKETS_SendPacket0x15_6FC3D0D0(client, 0x15u, 0, player->dwUnitId, CLIENTS_GetUnitX(player), CLIENTS_GetUnitY(player), 1);
    sub_6FC33020(client, room);
    D2Common_10513(player, player, (void(__fastcall*)(D2UnitStrc*, int32_t, int32_t, D2UnitStrc*))D2GAME_UpdateAttribute_6FC822D0);
    sub_6FC82360(player, player, 1);
    LEVEL_UpdateUnitsInAdjacentRooms(g_game, room, client);
    const auto meet = [&](D2ClientStrc* other, D2UnitStrc* otherPlayer) {
        if (!other || !otherPlayer || other == client) return;
        sub_6FC3DF80(client, otherPlayer, other);
        sub_6FC3DF80(other, player, client);
    };
    meet(g_targetClient, g_targetPlayer);
    for (int s = 1; s < 4; ++s) if (s != slot && sessionExtraClient(s)) meet(g_sessionExtraClients[s].client, g_sessionExtraClients[s].player);
    sub_6FC3DF80(client, player, client);
    return 1;
}
EMSCRIPTEN_KEEPALIVE int d2_session_client_unit_id(int slot) {
    const SessionExtraClient* e = sessionExtraClient(slot);
    return e ? (int)e->player->dwUnitId : -1;
}
// One in-game packet from that client, through retail's own handler on its own player. -1 for a header
// no handler is wired for.
EMSCRIPTEN_KEEPALIVE int d2_session_client_packet_slot(int slot, const uint8_t* bytes, int len) {
    SessionExtraClient* e = sessionExtraClient(slot);
    if (!e || !bytes || len <= 0 || len > 512) return -1;
    std::vector<uint8_t> packet(bytes, bytes + len);
    int32_t (__fastcall* handler)(D2GameStrc*, D2UnitStrc*, void*, int32_t) = nullptr;
    switch (packet[0]) {
    case 0x01: if (len != 5) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x01_Walk_6FC82CB0; break;
    case 0x03: if (len != 5) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x03_Run_6FC83090; break;
    case 0x0C: if (len != 5) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x0C_RightSkill_6FC83EF0; break;
    default: return -1;
    }
    return handler(g_game, e->player, packet.data(), len);
}

// d2_aitarget_attach_at: the capture's D2CAPTURE_AI_PLAYER_AT -- the fixture stands at (x,y), or the
// nearest free subtile within 3, in the room that holds it, instead of beside the first monster.
static int g_attachAtX = -1, g_attachAtY = -1;
// Retail's fed session (D2CAPTURE_SESSION_CLIENT) joins the client to every adjacent room (sub_6FC33020), so a town NPC's
// 6FCF1E80 client scan finds it (Akara's walk to the player, the 0x8A mark); unfed baselines (a retail recording) keep the own-room fixture.
static bool g_sessionClientFeed = false;
// on == 2 (a retail recording): retail's real client is in every adjacent room (0x07 for 79 rooms at tick 1) so pets and monsters scan them,
// but its first-client wake events do not fire early; join the adjacent rooms' client arrays only (no events, no packets).
EMSCRIPTEN_KEEPALIVE void d2_session_client_feed(int on) { g_sessionClientFeed = on == 1; g_sessionClientAdjCount = on == 2; }
EMSCRIPTEN_KEEPALIVE int d2_aitarget_attach() {
    if (!g_game || !g_lastLevel || g_targetPlayer) return 0;
    if (g_attachAtX < 0 && g_sessionStartX >= 0) { g_attachAtX = g_sessionStartX; g_attachAtY = g_sessionStartY; g_sessionStartX = g_sessionStartY = -1; const int ok = d2_aitarget_attach(); g_attachAtX = g_attachAtY = -1; if (ok) return 1; }
    uint16_t roomIdx = 0;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext, roomIdx++) {
        // A room a previous fixture left keeps its (empty) client array; only a live client counts.
        D2ActiveRoomStrc* room = roomEx->pRoom; if (!room || room->nNumClients) continue;
        D2UnitStrc* anchor = nullptr;
        for (D2UnitStrc* u = room->pUnitFirst; u; u = u->pRoomNext)
            if (u->dwUnitType == UNIT_MONSTER && (g_sessionAnchorClass == 0xFFFFFFFF || (uint32_t)u->dwClassId == g_sessionAnchorClass)) { anchor = u; break; }
        int ax = 0, ay = 0, r0 = 2, r1 = 12;
        if (g_attachAtX >= 0) {
            if (g_attachAtX < room->tCoords.nSubtileX || g_attachAtX >= room->tCoords.nSubtileX + room->tCoords.nSubtileWidth ||
                g_attachAtY < room->tCoords.nSubtileY || g_attachAtY >= room->tCoords.nSubtileY + room->tCoords.nSubtileHeight) continue;
            ax = g_attachAtX; ay = g_attachAtY; r0 = 0; r1 = 3;
        } else {
            if (!anchor || !anchor->pDynamicPath) continue;
            ax = anchor->pDynamicPath->tGameCoords.wPosX; ay = anchor->pDynamicPath->tGameCoords.wPosY;
        }
        for (int radius = r0; radius <= r1; radius++) for (int dy = -radius; dy <= radius; dy++) for (int dx = -radius; dx <= radius; dx++) {
            if (abs(dx) != radius && abs(dy) != radius) continue;
            const int x = ax + dx, y = ay + dy;
            if (x < room->tCoords.nSubtileX + 2 || x >= room->tCoords.nSubtileX + room->tCoords.nSubtileWidth - 2 ||
                y < room->tCoords.nSubtileY + 2 || y >= room->tCoords.nSubtileY + room->tCoords.nSubtileHeight - 2) continue;
            if (COLLISION_CheckMaskWithSize(room, x, y, 2, 0xFFFF)) continue;
            if (attachSessionPlayerAt(room, x, y, anchor)) return 1;
        }
    }

    // No room held a monster to anchor to. For the AI and spawn oracles that is the answer -- and
    // for a fixture that asked for a specific anchor class it still is -- but a LEVEL WITH NO
    // MONSTERS IS STILL A LEVEL A PLAYER WALKS INTO. Act 1 level 20, the Forgotten Tower, is one:
    // it generated fine and every caller of d2_session_create was refused, so the level could not
    // be entered, drawn, or compared against retail at all.
    //
    // Levels that do have monsters never reach here, so no existing row's placement or roll
    // sequence moves.
    if (g_sessionAnchorClass != 0xFFFFFFFF) return 0;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
        D2ActiveRoomStrc* room = roomEx->pRoom;
        if (!room || room->nNumClients) continue;
        // Scan outward from the room's middle, so the player lands in open floor rather than
        // against the first wall the iteration order happens to reach.
        const int cx = room->tCoords.nSubtileX + room->tCoords.nSubtileWidth / 2;
        const int cy = room->tCoords.nSubtileY + room->tCoords.nSubtileHeight / 2;
        const int span = (room->tCoords.nSubtileWidth + room->tCoords.nSubtileHeight) / 2;
        for (int radius = 0; radius <= span; radius++) for (int dy = -radius; dy <= radius; dy++) for (int dx = -radius; dx <= radius; dx++) {
            if (radius && abs(dx) != radius && abs(dy) != radius) continue;
            const int x = cx + dx, y = cy + dy;
            if (x < room->tCoords.nSubtileX + 2 || x >= room->tCoords.nSubtileX + room->tCoords.nSubtileWidth - 2 ||
                y < room->tCoords.nSubtileY + 2 || y >= room->tCoords.nSubtileY + room->tCoords.nSubtileHeight - 2) continue;
            if (COLLISION_CheckMaskWithSize(room, x, y, 2, 0xFFFF)) continue;
            if (attachSessionPlayerAt(room, x, y, nullptr)) return 1;
        }
    }
    return 0;
}
// The kind-27 fixture swings at the nearest living monster of the level with its left skill, as the
// capture's D2CAPTURE_AI_PLAYER_ATTACK does: client packet 0x06 {type 1, GUID} through retail's
// LeftSkillOnUnit handler. Nearest = smallest squared distance, then lowest GUID. Returns
// (handler result << 16) | GUID, or -1 with nothing to swing at.
// The capture's D2CAPTURE_AI_PLAYER_BASH kit, on the attached fixture: Bash learned (STAT_SKILLPTS 1,
// then 0x3B), 50 mana, a normal Hand Axe from QUESTS_CreateItem (retail 0x6FC56470) lifted (0x19)
// and equipped in the right hand (0x1A), Bash selected for the right hand (0x3C). Packet results,
// packed 4 bits each, 0 = all accepted.
EMSCRIPTEN_KEEPALIVE int d2_aitarget_set_player_class(int playerClass) { g_sessionPlayerClass = playerClass; return 1; }
EMSCRIPTEN_KEEPALIVE int d2_aitarget_player_kit_bash() {
    if (!g_game || !g_targetPlayer) return -1;
    int out = 0;
    STATLIST_SetUnitStat(g_targetPlayer, STAT_SKILLPTS, 1, 0);
    uint8_t learn[3] = { 0x3B, 126, 0 };
    out |= (D2GAME_PACKETCALLBACK_Rcv0x3B_AddSkillPoint_6FC86EF0(g_game, g_targetPlayer, learn, sizeof learn) & 15);
    STATLIST_SetUnitStat(g_targetPlayer, STAT_MANA, 50 << 8, 0); STATLIST_SetUnitStat(g_targetPlayer, STAT_MAXMANA, 50 << 8, 0);
    // A raw unit has 0 strength and dexterity; ITEMS_CheckRequirements wants both > 0 even for a Hand
    // Axe with none. CharStats' barbarian: 30 / 20.
    STATLIST_SetUnitStat(g_targetPlayer, STAT_STRENGTH, 30, 0); STATLIST_SetUnitStat(g_targetPlayer, STAT_DEXTERITY, 20, 0);
    D2UnitStrc* axe = QUESTS_CreateItem(g_game, g_targetPlayer, ' xah', 0, ITEMQUAL_NORMAL, 0);
    if (!axe) return -2;
    const uint32_t id = axe->dwUnitId;
    uint8_t lift[5] = { 0x19 }; memcpy(lift + 1, &id, 4);
    out |= (D2GAME_PACKETCALLBACK_Rcv0x19_RemoveItemFromBuffer_6FC850C0(g_game, g_targetPlayer, lift, sizeof lift) & 15) << 4;
    uint8_t equip[9] = { 0x1A }; memcpy(equip + 1, &id, 4); equip[5] = 4;
    const int rEquip = D2GAME_PACKETCALLBACK_Rcv0x1A_EquipItem_6FC85280(g_game, g_targetPlayer, equip, sizeof equip);
    out |= (rEquip & 15) << 8;
    uint8_t sel[9] = { 0x3C }; const uint32_t word = 126; const int32_t owner = -1; memcpy(sel + 1, &word, 4); memcpy(sel + 5, &owner, 4);
    out |= (D2GAME_PACKETCALLBACK_Rcv0x3C_SelectSkill_6FC87050(g_game, g_targetPlayer, sel, sizeof sel) & 15) << 12;
    return out;
}
static bool g_aitargetSwingRight = false;
// Diagnosis: the fixture's right skill, its use state, and the right-hand item class (-1 none).
static int32_t g_aitargetHands[4];
EMSCRIPTEN_KEEPALIVE const int32_t* d2_aitarget_hands() {
    D2SkillStrc* r = g_targetPlayer ? UNITS_GetRightSkill(g_targetPlayer) : nullptr;
    D2UnitStrc* w = g_targetPlayer && g_targetPlayer->pInventory ? INVENTORY_GetItemFromBodyLoc(g_targetPlayer->pInventory, BODYLOC_RARM) : nullptr;
    g_aitargetHands[0] = r ? SKILLS_GetSkillIdFromSkill(r, __FILE__, __LINE__) : -1;
    g_aitargetHands[1] = r ? SKILLS_GetUseState(g_targetPlayer, r) : -1;
    g_aitargetHands[2] = w ? (int32_t)w->dwClassId : -1;
    g_aitargetHands[3] = g_targetPlayer ? (int32_t)g_targetPlayer->dwAnimMode : -1;
    return g_aitargetHands;
}
EMSCRIPTEN_KEEPALIVE int d2_aitarget_swing_right(int on) { g_aitargetSwingRight = on != 0; return 1; }
EMSCRIPTEN_KEEPALIVE int d2_aitarget_player_swing() {
    if (!g_game || !g_lastLevel || !g_targetPlayer || !g_targetPlayer->pDynamicPath) return -1;
    const int px = g_targetPlayer->pDynamicPath->tGameCoords.wPosX, py = g_targetPlayer->pDynamicPath->tGameCoords.wPosY;
    D2UnitStrc* best = nullptr; int64_t bestD = INT64_MAX;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
        if (!roomEx->pRoom) continue;
        for (D2UnitStrc* u = roomEx->pRoom->pUnitFirst; u; u = u->pRoomNext) {
            if (u->dwUnitType != UNIT_MONSTER || !u->pDynamicPath || u->dwAnimMode == MONMODE_DEAD || u->dwAnimMode == MONMODE_DEATH || STATLIST_UnitGetStatValue(u, STAT_HITPOINTS, 0) <= 0) continue;
            const int64_t dx = u->pDynamicPath->tGameCoords.wPosX - px, dy = u->pDynamicPath->tGameCoords.wPosY - py, d = dx * dx + dy * dy;
            if (d < bestD || (d == bestD && best && u->dwUnitId < best->dwUnitId)) { bestD = d; best = u; }
        }
    }
    if (!best) return -1;
    uint8_t packet[9] = { (uint8_t)(g_aitargetSwingRight ? 0x0D : 0x06) }; const uint32_t type = UNIT_MONSTER, id = best->dwUnitId;
    memcpy(packet + 1, &type, 4); memcpy(packet + 5, &id, 4);
    // The capture sets the game frame to the new frame, swings, then runs the frame's events; the
    // replay's d2_aiframe_advance increments inside. Swing at the frame the capture swings at.
    ++g_game->dwGameFrame;
    const int32_t r = g_aitargetSwingRight ? D2GAME_PACKETCALLBACK_Rcv0x0D_RightSkillOnUnit_6FC83F80(g_game, g_targetPlayer, packet, sizeof packet)
        : D2GAME_PACKETCALLBACK_Rcv0x06_LeftSkillOnUnit_6FC83550(g_game, g_targetPlayer, packet, sizeof packet);
    --g_game->dwGameFrame;
    return (r << 16) | (int)(id & 0xFFFF);
}
// D18 skill probe: the attached fixture (class set by d2_aitarget_set_player_class) becomes level 30
// with `skillId` at slvl 1 and 500 mana, selects it on the right hand and casts it at the nearest
// monster (mode 0: unit packet 0x0D, 1: location packet 0x0C at the monster, 2: location at self),
// then runs 60 frames. out[] (int32): 0 handler rc, 1 first mode after cast, 2 modes seen bitmask
// (player modes 0..15), 3 largest one-frame mana drop (raw), 4 player hp delta, 5 summed monster hp delta, 6 missiles seen,
// 7 new monsters, 8 player states gained, 9 monster states gained, 10 mode packets, 11 skill assigned, 12 Skills.txt flags[0], 13 srvdofunc | srvstartfunc << 16, 14 Skills.txt cost at slvl 1 (raw).
extern "C" uint32_t g_d2webModePackets;
EMSCRIPTEN_KEEPALIVE int d2_aiframe_advance(int frames);
static int32_t g_skillProbe[15];
static int skillProbeCountStates(D2UnitStrc* u, uint8_t* seen) {
    int n = 0; for (int s = 1; s < 200; s++) if (STATES_CheckState(u, s) && !seen[s]) { seen[s] = 1; n++; } return n;
}
EMSCRIPTEN_KEEPALIVE const int32_t* d2_skillprobe_cast(int skillId, int mode) {
    memset(g_skillProbe, 0, sizeof g_skillProbe); g_skillProbe[0] = -99;
    if (!g_game || !g_lastLevel || !g_targetPlayer || !g_targetPlayer->pDynamicPath) return g_skillProbe;
    D2UnitStrc* p = g_targetPlayer;
    STATLIST_SetUnitStat(p, STAT_LEVEL, 30, 0);
    STATLIST_SetUnitStat(p, STAT_HITPOINTS, 500 << 8, 0); STATLIST_SetUnitStat(p, STAT_MAXHP, 500 << 8, 0);
    STATLIST_SetUnitStat(p, STAT_MANA, 500 << 8, 0); STATLIST_SetUnitStat(p, STAT_MAXMANA, 500 << 8, 0);
    STATLIST_SetUnitStat(p, STAT_STRENGTH, 30, 0); STATLIST_SetUnitStat(p, STAT_DEXTERITY, 30, 0);
    STATLIST_SetUnitStat(p, STAT_ENERGY, 30, 0); STATLIST_SetUnitStat(p, STAT_VITALITY, 30, 0);
    SKILLS_AssignSkill(p, skillId, 1, 0, __FILE__, __LINE__);
    g_skillProbe[11] = SKILLS_GetSkillById(p, skillId, -1) != nullptr;
    if (D2SkillsTxt* t = DATATBLS_GetSkillsTxtRecord(skillId)) { g_skillProbe[12] = (int32_t)t->dwFlags[0]; g_skillProbe[13] = t->wSrvDoFunc | (t->wSrvStartFunc << 16);
        // Skills.txt's own cost at slvl 1, read straight from the record (not through the engine's cost path): max(mana, minmana) << manashift, 1/256 mana units.
        g_skillProbe[14] = std::max<int>(t->wMana, t->wMinMana) << t->nManaShift; }
    uint8_t sel[9] = { 0x3C }; const uint32_t word = (uint32_t)skillId; const int32_t owner = -1; memcpy(sel + 1, &word, 4); memcpy(sel + 5, &owner, 4);
    D2GAME_PACKETCALLBACK_Rcv0x3C_SelectSkill_6FC87050(g_game, p, sel, sizeof sel);
    const int px = p->pDynamicPath->tGameCoords.wPosX, py = p->pDynamicPath->tGameCoords.wPosY;
    D2UnitStrc* best = nullptr; int64_t bestD = INT64_MAX; int monsBefore = 0;
    std::map<uint32_t, int> hp0; std::set<uint32_t> known;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
        if (!roomEx->pRoom) continue;
        for (D2UnitStrc* u = roomEx->pRoom->pUnitFirst; u; u = u->pRoomNext) {
            if (u->dwUnitType != UNIT_MONSTER || !u->pDynamicPath) continue;
            monsBefore++; known.insert(u->dwUnitId); hp0[u->dwUnitId] = STATLIST_UnitGetStatValue(u, STAT_HITPOINTS, 0);
            if (u->dwAnimMode == MONMODE_DEAD || u->dwAnimMode == MONMODE_DEATH) continue;
            const int64_t dx = u->pDynamicPath->tGameCoords.wPosX - px, dy = u->pDynamicPath->tGameCoords.wPosY - py, d = dx * dx + dy * dy;
            if (d < bestD || (d == bestD && best && u->dwUnitId < best->dwUnitId)) { bestD = d; best = u; }
        }
    }
    if (!best) { g_skillProbe[0] = -98; return g_skillProbe; }
    uint32_t tgtType = UNIT_MONSTER, tgtId = best->dwUnitId;
    // Preconditions: corpse skills (70 74 80 83 95) need a dead monster to aim at; druid/assassin
    // (221..280) melee skills need the target adjacent, so the hero steps next to it first.
    if (skillId == 70 || skillId == 74 || skillId == 80 || skillId == 83 || skillId == 95) {
        STATLIST_SetUnitStat(best, STAT_HITPOINTS, 0, 0);
        SUNITDMG_KillMonster(g_game, best, p, 1);
        for (int f = 0; f < 40; f++) d2_aiframe_advance(1);
        hp0[best->dwUnitId] = STATLIST_UnitGetStatValue(best, STAT_HITPOINTS, 0);
    } else if (skillId == 73 || skillId == 260) {   // Poison Dagger needs a dagger, Dragon Claw a pair of claws; step next to the target as well
        auto equip = [&](uint32_t code, uint8_t loc) {
            D2UnitStrc* it = QUESTS_CreateItem(g_game, p, code, 0, ITEMQUAL_NORMAL, 0); if (!it) return;
            uint8_t lift[5] = { 0x19 }, eq[9] = { 0x1A }; memcpy(lift + 1, &it->dwUnitId, 4); memcpy(eq + 1, &it->dwUnitId, 4); eq[5] = loc;
            D2GAME_PACKETCALLBACK_Rcv0x19_RemoveItemFromBuffer_6FC850C0(g_game, p, lift, sizeof lift);
            D2GAME_PACKETCALLBACK_Rcv0x1A_EquipItem_6FC85280(g_game, p, eq, sizeof eq);
        };
        if (skillId == 73) equip(' rgd', 4); else { equip(' rtk', 4); equip(' rtk', 5); }   // 'dgr' Dagger, 'ktr' Katar
        D2ActiveRoomStrc* room = UNITS_GetRoom(best); D2CoordStrc at = {}; UNITS_GetCoords(best, &at);
        const int32_t size = UNITS_GetUnitSizeX(best);
        for (int32_t reach = size, done = 0; !done && room && reach <= size + 3; ++reach)
            for (int32_t dx = -reach; !done && dx <= reach; ++dx) for (int32_t dy = -reach; !done && dy <= reach; ++dy) {
                if (abs(dx) != reach && abs(dy) != reach) continue;
                done = sub_6FCBDFE0(g_game, p, room, at.nX + dx, at.nY + dy, 0, 0) != 0;
            }
        d2_aiframe_advance(1);
    } else if (skillId == 90) {   // Iron Golem aims at a metal item lying on the ground: a Hand Axe, lifted then dropped
        if (D2UnitStrc* axe = QUESTS_CreateItem(g_game, p, ' xah', 0, ITEMQUAL_NORMAL, 0)) {
            uint8_t lift[5] = { 0x19 }, drop[5] = { 0x17 }; memcpy(lift + 1, &axe->dwUnitId, 4); memcpy(drop + 1, &axe->dwUnitId, 4);
            D2GAME_PACKETCALLBACK_Rcv0x19_RemoveItemFromBuffer_6FC850C0(g_game, p, lift, sizeof lift);
            D2GAME_PACKETCALLBACK_Rcv0x17_DropItemOnGround_6FC84E20(g_game, p, drop, sizeof drop);
            tgtType = UNIT_ITEM; tgtId = axe->dwUnitId;
        }
    } else if (skillId >= 221 && skillId <= 280) {
        // Shapeshift skills only work in their form: Werewolf (223) for Feral Rage/Rabies/Fire Claws/Hunger/Fury,
        // Werebear (228) for Maul/Shock Wave. Cast the form first, then reselect the skill under test.
        const int form = (skillId == 232 || skillId == 238 || skillId == 239 || skillId == 242 || skillId == 248) ? 223 : (skillId == 233 || skillId == 243) ? 228 : 0;
        if (form) {
            SKILLS_AssignSkill(p, form, 1, 0, __FILE__, __LINE__);
            uint8_t fsel[9] = { 0x3C }; const uint32_t fw = (uint32_t)form; const int32_t fo = -1; memcpy(fsel + 1, &fw, 4); memcpy(fsel + 5, &fo, 4);
            D2GAME_PACKETCALLBACK_Rcv0x3C_SelectSkill_6FC87050(g_game, p, fsel, sizeof fsel);
            uint8_t fp[5] = { 0x0C }; const uint16_t fx = px, fy = py; memcpy(fp + 1, &fx, 2); memcpy(fp + 3, &fy, 2);
            ++g_game->dwGameFrame; D2GAME_PACKETCALLBACK_Rcv0x0C_RightSkill_6FC83EF0(g_game, p, fp, sizeof fp); --g_game->dwGameFrame;
            for (int f = 0; f < 40; f++) d2_aiframe_advance(1);
            D2GAME_PACKETCALLBACK_Rcv0x3C_SelectSkill_6FC87050(g_game, p, sel, sizeof sel);
        }
        D2ActiveRoomStrc* room = UNITS_GetRoom(best); D2CoordStrc at = {}; UNITS_GetCoords(best, &at);
        const int32_t size = UNITS_GetUnitSizeX(best);
        for (int32_t reach = size, done = 0; !done && room && reach <= size + 3; ++reach)
            for (int32_t dx = -reach; !done && dx <= reach; ++dx) for (int32_t dy = -reach; !done && dy <= reach; ++dy) {
                if (abs(dx) != reach && abs(dy) != reach) continue;
                done = sub_6FCBDFE0(g_game, p, room, at.nX + dx, at.nY + dy, 0, 0) != 0;
            }
        d2_aiframe_advance(1);
    }
    uint8_t seenP[200] = {}, seenM[200] = {};
    skillProbeCountStates(p, seenP);   // baseline: states present before the cast do not count
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext)
        if (roomEx->pRoom) for (D2UnitStrc* u = roomEx->pRoom->pUnitFirst; u; u = u->pRoomNext) if (u->dwUnitType == UNIT_MONSTER) skillProbeCountStates(u, seenM);
    const int mana0 = STATLIST_UnitGetStatValue(p, STAT_MANA, 0), hp = STATLIST_UnitGetStatValue(p, STAT_HITPOINTS, 0);
    const uint32_t pk0 = g_d2webModePackets;
    ++g_game->dwGameFrame;
    int32_t rc;
    if (mode == 0) {
        uint8_t packet[9] = { 0x0D }; memcpy(packet + 1, &tgtType, 4); memcpy(packet + 5, &tgtId, 4);
        rc = D2GAME_PACKETCALLBACK_Rcv0x0D_RightSkillOnUnit_6FC83F80(g_game, p, packet, sizeof packet);
    } else {
        uint8_t packet[5] = { 0x0C }; const uint16_t x = mode == 1 ? best->pDynamicPath->tGameCoords.wPosX : px, y = mode == 1 ? best->pDynamicPath->tGameCoords.wPosY : py;
        memcpy(packet + 1, &x, 2); memcpy(packet + 3, &y, 2);
        rc = D2GAME_PACKETCALLBACK_Rcv0x0C_RightSkill_6FC83EF0(g_game, p, packet, sizeof packet);
    }
    --g_game->dwGameFrame;
    g_skillProbe[0] = rc; g_skillProbe[1] = (int32_t)p->dwAnimMode;
    int manaNow = mana0 - STATLIST_UnitGetStatValue(p, STAT_MANA, 0), manaPrev = mana0 - manaNow;
    std::set<uint32_t> missiles; int newMons = 0, msGain = 0;
    for (int f = 0; f < 60; f++) {
        d2_aiframe_advance(1);
        { const int cur = STATLIST_UnitGetStatValue(p, STAT_MANA, 0); manaNow = std::max(manaNow, manaPrev - cur); manaPrev = cur; }   // retail takes the mana in the do frame, not at the packet; the largest one-frame drop is the cost less that frame's regen
        g_skillProbe[2] |= 1 << (p->dwAnimMode & 15);
        for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
            if (!roomEx->pRoom) continue;
            for (D2UnitStrc* u = roomEx->pRoom->pUnitFirst; u; u = u->pRoomNext) {
                if (u->dwUnitType == UNIT_MISSILE) missiles.insert(u->dwUnitId);
                else if (u->dwUnitType == UNIT_MONSTER) { if (known.insert(u->dwUnitId).second) newMons++; msGain += skillProbeCountStates(u, seenM); }
            }
        }
    }
    int dmg = 0;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext)
        if (roomEx->pRoom) for (D2UnitStrc* u = roomEx->pRoom->pUnitFirst; u; u = u->pRoomNext)
            if (u->dwUnitType == UNIT_MONSTER && hp0.count(u->dwUnitId)) dmg += hp0[u->dwUnitId] - STATLIST_UnitGetStatValue(u, STAT_HITPOINTS, 0);
    g_skillProbe[3] = manaNow;   // raw 1/256 mana spent by the cast itself, before any regen frame
    g_skillProbe[4] = (STATLIST_UnitGetStatValue(p, STAT_HITPOINTS, 0) - hp) >> 8;
    g_skillProbe[5] = dmg >> 8; g_skillProbe[6] = (int32_t)missiles.size(); g_skillProbe[7] = newMons;
    g_skillProbe[8] = skillProbeCountStates(p, seenP); g_skillProbe[9] = msGain; g_skillProbe[10] = (int32_t)(g_d2webModePackets - pk0);
    return g_skillProbe;
}
EMSCRIPTEN_KEEPALIVE int d2_aitarget_attach_at(int x, int y) {
    g_attachAtX = x; g_attachAtY = y; g_fixtureOwnRoomOnly = true;
    const int ok = d2_aitarget_attach();
    g_fixtureOwnRoomOnly = false;
    g_attachAtX = g_attachAtY = -1;
    return ok;
}
EMSCRIPTEN_KEEPALIVE const D2VecAiLevel* d2_aitarget_out() { return &g_targetOut; }

// Monster drops (kind 29, s190): the capture's D2CAPTURE_DROPS protocol. With the level populated
// and the player fixture attached, every monster in Room2-list then unit-list order goes through
// the monster-death drop routine (D2MOO sub_6FC631B0 = D2Game 1.13c 0x6FC95680) with the player
// as the mode change's target; the DropTC observer records the walk's inputs, and the items are the
// item units the call added to the level.
extern void (*g_d2webOnDropTC)(D2UnitStrc*, D2TCExShortStrc*, int32_t, int32_t);
static D2VecMonDrop* g_dropRec;
static std::vector<D2VecMonDrop> g_dropMons;
static std::vector<D2VecMonDropItem> g_dropItems;
static void onDropTC(D2UnitStrc*, D2TCExShortStrc* pTC, int32_t nQuality, int32_t nLvl) {
    if (!g_dropRec || g_dropRec->walks++) return;
    g_dropRec->quality = nQuality; g_dropRec->itemLevel = nLvl;
    g_dropRec->tcId = (uint32_t)(pTC - sgptDataTables->pTreasureClassEx);
}
static void levelItemIds(std::vector<uint32_t>* ids) {
    ids->clear();
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext)
        for (D2UnitStrc* u = roomEx->pRoom ? roomEx->pRoom->pUnitFirst : nullptr; u; u = u->pRoomNext)
            if (u->dwUnitType == UNIT_ITEM) ids->push_back(u->dwUnitId);
}
EMSCRIPTEN_KEEPALIVE int d2_mondrop_run() {
    g_dropMons.clear(); g_dropItems.clear();
    if (!g_game || !g_lastLevel || !g_targetPlayer) return 0;
    struct Mon { D2UnitStrc* u; uint16_t room, ordinal; };
    std::vector<Mon> monsters; uint16_t roomIdx = 0;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext, roomIdx++) {
        uint16_t ord = 0;
        for (D2UnitStrc* u = roomEx->pRoom ? roomEx->pRoom->pUnitFirst : nullptr; u; u = u->pRoomNext)
            if (u->dwUnitType == UNIT_MONSTER) monsters.push_back({ u, roomIdx, ord++ });
    }
    std::vector<uint32_t> before;
    g_d2webOnDropTC = onDropTC;
    for (const Mon& m : monsters) {
        D2VecMonDrop r = {};
        r.seed = g_out.seed; r.difficulty = g_out.difficulty; r.act = g_out.act; r.level = g_out.level; r.room = m.room; r.ordinal = m.ordinal;
        r.monClass = m.u->dwClassId; r.seedLo = m.u->pSeed.nLowSeed; r.seedHi = m.u->pSeed.nHighSeed; r.tcId = 0xFFFFFFFF; r.itemHash = D2VEC_FNV_INIT;
        levelItemIds(&before);
        D2ModeChangeStrc mc = {}; mc.pUnit = m.u; mc.pTargetUnit = g_targetPlayer;
        g_dropRec = &r;
        sub_6FC631B0(g_game, m.u, 0, &mc);
        g_dropRec = nullptr;
        r.seedLoAfter = m.u->pSeed.nLowSeed; r.seedHiAfter = m.u->pSeed.nHighSeed;
        uint16_t index = 0;
        for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext)
            for (D2UnitStrc* it = roomEx->pRoom ? roomEx->pRoom->pUnitFirst : nullptr; it; it = it->pRoomNext) {
                if (it->dwUnitType != UNIT_ITEM || std::find(before.begin(), before.end(), (uint32_t)it->dwUnitId) != before.end()) continue;
                D2VecMonDropItem v = {}; v.monster = (uint16_t)g_dropMons.size(); v.index = index++;
                v.classId = it->dwClassId; v.quality = (uint32_t)ITEMS_GetItemQuality(it); v.ilvl = (uint32_t)ITEMS_GetItemLevel(it);
                v.seedLo = it->pSeed.nLowSeed; v.seedHi = it->pSeed.nHighSeed;
                if (it->pStaticPath) { v.x = (uint16_t)it->pStaticPath->tGameCoords.nX; v.y = (uint16_t)it->pStaticPath->tGameCoords.nY; }
                r.itemHash = d2vec_mondrop_item_hash(r.itemHash, &v);
                g_dropItems.push_back(v);
            }
        r.items = index;
        g_dropMons.push_back(r);
    }
    g_d2webOnDropTC = nullptr;
    return (int)g_dropMons.size();
}
// kind-29 replay of the capture's D2CAPTURE_DROPS_QDONE: the attached player has quest `questFilter` COMPLETEDBEFORE on
// this difficulty, so a boss with a TCQuestId rolls its ordinary treasure class.
EMSCRIPTEN_KEEPALIVE int d2_mondrop_quest_done(uint32_t questFilter) {
    if (!g_game || !g_targetPlayer) return 0;
    D2PlayerDataStrc* pd = UNITS_GetPlayerData(g_targetPlayer);
    if (!pd || g_game->nDifficulty >= 3 || !pd->pQuestData[g_game->nDifficulty]) return 0;
    QUESTRECORD_SetQuestState(pd->pQuestData[g_game->nDifficulty], (int32_t)questFilter, QFLAG_COMPLETEDBEFORE);
    return 1;
}
EMSCRIPTEN_KEEPALIVE const D2VecMonDrop* d2_mondrop_monsters(int* count) { *count = (int)g_dropMons.size(); return g_dropMons.data(); }
EMSCRIPTEN_KEEPALIVE const D2VecMonDropItem* d2_mondrop_items(int* count) { *count = (int)g_dropItems.size(); return g_dropItems.data(); }
EMSCRIPTEN_KEEPALIVE const D2VecAiTargetUnit* d2_aitarget_units(int* count) { *count = (int)g_targetUnits.size(); return g_targetUnits.data(); }
EMSCRIPTEN_KEEPALIVE const D2VecCombat* d2_combat_events(int* count) { *count = (int)g_combatEvents.size(); return g_combatEvents.data(); }
EMSCRIPTEN_KEEPALIVE const D2SessionHitRoll* d2_session_hit_rolls() { return g_sessionHitRolls.data(); }
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_hit_roll_count() { return (uint32_t)g_sessionHitRolls.size(); }
EMSCRIPTEN_KEEPALIVE const D2VecCombatResult* d2_combat_result() {
    memset(&g_combatResult, 0, sizeof g_combatResult);
    if (!g_targetPlayer) return &g_combatResult;
    D2UnitStrc* u = g_targetPlayer;
    g_combatResult.seed = g_gameSeed; g_combatResult.difficulty = (uint8_t)g_gameDiff;
    g_combatResult.act = (uint8_t)g_currentAct; g_combatResult.level = (uint16_t)g_currentLevel;
    g_combatResult.frame = g_game ? g_game->dwGameFrame : 0;
    g_combatResult.unitType = u->dwUnitType; g_combatResult.unitClass = u->dwClassId;
    g_combatResult.unitId = u->dwUnitId; g_combatResult.mode = u->dwAnimMode;
    g_combatResult.seedLo = u->pSeed.nLowSeed; g_combatResult.seedHi = u->pSeed.nHighSeed;
    g_combatResult.hp = STATLIST_UnitGetStatValue(u, STAT_HITPOINTS, 0);
    g_combatResult.maxHp = STATLIST_UnitGetStatValue(u, STAT_MAXHP, 0);
    g_combatResult.mana = STATLIST_UnitGetStatValue(u, STAT_MANA, 0);
    g_combatResult.maxMana = STATLIST_UnitGetStatValue(u, STAT_MAXMANA, 0);
    g_combatResult.stamina = STATLIST_UnitGetStatValue(u, STAT_STAMINA, 0);
    g_combatResult.maxStamina = STATLIST_UnitGetStatValue(u, STAT_MAXSTAMINA, 0);
    g_combatResult.lastHitClass = u->dwLastHitClass; g_combatResult.flags = u->dwFlags; g_combatResult.flagsEx = u->dwFlagEx;
    return &g_combatResult;
}
EMSCRIPTEN_KEEPALIVE int d2_damagecalc_run(D2VecDamageCalc* v) {
    if (!v || !g_game || !g_targetPlayer || !g_targetMonster) return 0;
    const int statIds[8] = { STAT_NORMAL_DAMAGE_REDUCTION, STAT_MAGIC_DAMAGE_REDUCTION, STAT_DAMAGERESIST,
        STAT_MAGICRESIST, STAT_FIRERESIST, STAT_LIGHTRESIST, STAT_COLDRESIST, STAT_POISONRESIST };
    for (int i = 0; i < 8; ++i) STATLIST_SetUnitStat(g_targetPlayer, statIds[i], v->defenderStats[i], 0);
    g_targetMonster->pSeed.nLowSeed = v->seedLoIn; g_targetMonster->pSeed.nHighSeed = v->seedHiIn;
    D2DamageStrc damage; static_assert(sizeof damage == sizeof v->damageIn); memcpy(&damage, v->damageIn, sizeof damage);
    SUNITDMG_CalculateTotalDamage(g_game, g_targetMonster, g_targetPlayer, &damage);
    v->seedLoOut = g_targetMonster->pSeed.nLowSeed; v->seedHiOut = g_targetMonster->pSeed.nHighSeed;
    memcpy(v->damageOut, &damage, sizeof damage); return 1;
}
// kind 28 (D2VecAiPath): every monster of the level's dynamic path, room order, as the capture's
// aitrace-path.bin records it -- the path stepping behind a monster reaching its target late.
static std::vector<D2VecAiPath> g_aiPaths;
EMSCRIPTEN_KEEPALIVE const D2VecAiPath* d2_aipath_units(int* count) {
    g_aiPaths.clear();
    for (D2DrlgRoomStrc* roomEx = g_lastLevel ? g_lastLevel->pFirstRoomEx : nullptr; roomEx; roomEx = roomEx->pDrlgRoomNext) {
        if (!roomEx->pRoom) continue;
        for (D2UnitStrc* u = roomEx->pRoom->pUnitFirst; u; u = u->pRoomNext) {
            if (u->dwUnitType != UNIT_MONSTER) continue;
            D2VecAiPath r = {}; r.frame = g_game ? g_game->dwGameFrame : 0; r.unitId = u->dwUnitId;
            if (const D2DynamicPathStrc* p = u->pDynamicPath) {
                auto pt = [](const D2PathPointStrc& q) { return (uint32_t)q.X | ((uint32_t)q.Y << 16); };
                r.precX = p->tGameCoords.dwPrecisionX; r.precY = p->tGameCoords.dwPrecisionY;
                r.clientX = p->dwClientCoordX; r.clientY = p->dwClientCoordY;
                r.target = pt(p->tTargetCoord); r.prevTarget = pt(p->tPrevTargetCoord); r.finalTarget = pt(p->tFinalTargetCoord);
                r.curIdx = p->dwCurrentPointIdx; r.points = p->dwPathPoints;
                r.flags = p->dwFlags; r.unk38 = p->unk0x38; r.pathType = p->dwPathType; r.prevPathType = p->dwPrevPathType;
                r.unitSize = p->dwUnitSize; r.collisionPattern = p->dwCollisionPattern;
                r.footprintMask = p->nFootprintCollisionMask; r.moveTestMask = p->nMoveTestCollisionMask;
                r.collidedWith = p->nCollidedWithMask; r.targetType = p->dwTargetType; r.targetId = p->dwTargetId;
                r.direction = p->nDirection; r.newDirection = p->nNewDirection; r.diffDirection = p->nDiffDirection;
                r.unk67 = p->unk0x67; r.speed = p->dwSpeed; r.unk69 = p->unk0x69;
                r.dirVecX = p->tDirectionVector.nX; r.dirVecY = p->tDirectionVector.nY;
                r.velVecX = p->tVelocityVector.nX; r.velVecY = p->tVelocityVector.nY;
                r.velocity = p->dwVelocity; r.prevVelocity = (int32_t)p->nPreviousVelocity; r.maxVelocity = p->dwMaxVelocity;
                r.acceleration = p->dwAcceleration; r.accelCounter = p->dwAccelerationsCounter;
                r.dist = p->nDist; r.distMax = p->nDistMax; r.idaF = p->nIDAStarInitFScore; r.stepNum = p->nStepNum; r.distance = p->nDistance;
                r.dirOffset = p->nDirOffset;
                for (int i = 0; i < 16; i++) r.pathPoints[i] = pt(p->PathPoints[i]);
                r.savedSteps = p->nSavedStepsCount;
                for (int i = 0; i < 10; i++) r.saved[i] = pt(p->SavedSteps[i]);
            }
            g_aiPaths.push_back(r);
        }
    }
    *count = (int)g_aiPaths.size(); return g_aiPaths.data();
}
// Diagnostic: a unit's event timers ("type expire custom param;" ...) and its seed, as the capture's
// D2CAPTURE_AI_QUEUE_UNIT prints them -- which event of which frame rolls a monster's seed.
static std::string g_unitTimers;
// Diagnostic, read-only: the inputs of sub_6FCF2110's target test between two monsters as they stand now --
// [unit aiFlags, DUNGEON_CheckLOSDraw(unit's room), vision set, vision+9 dword, v66 as sub_6FCF2110 derives
// it, missile-barrier collision target->unit, sub_6FCBD900 (hostile), full-size distance, unit alignment,
// target alignment, target MonStats threat, D2Common_10095 region of unit, of target].
// Diagnostic, read-only: a subtile as a monster's path would meet it -- [room found from the unit's room
// (COLLISION_GetRoomBySubTileCoordinates), that room's level id, collision mask at the subtile (0xFFFF),
// unit x, unit y].
static int32_t g_pathProbe[5];
EMSCRIPTEN_KEEPALIVE const int32_t* d2_path_probe(uint32_t unitId, int32_t x, int32_t y) {
    memset(g_pathProbe, 0, sizeof g_pathProbe);
    D2UnitStrc* u = g_game ? SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)unitId) : nullptr;
    if (!u || !u->pDynamicPath) { g_pathProbe[0] = -1; return g_pathProbe; }
    D2ActiveRoomStrc* r = COLLISION_GetRoomBySubTileCoordinates(u->pDynamicPath->pRoom, x, y);
    g_pathProbe[0] = r ? 1 : 0; g_pathProbe[1] = r ? DUNGEON_GetLevelIdFromRoom(r) : -1;
    g_pathProbe[2] = r ? COLLISION_CheckMaskWithSize(r, x, y, 1, 0xFFFF) : -1;
    g_pathProbe[3] = CLIENTS_GetUnitX(u); g_pathProbe[4] = CLIENTS_GetUnitY(u);
    return g_pathProbe;
}
// Diagnostic, read-only apart from the scan's own reads: a good unit's target acquisition as sub_6FCF2110's
// non-evil branch runs it -- [sub_6FC61EC0 state, its unit id, scan target id, distance, alternative id,
// alternative distance, v66].
static int32_t g_scanProbe[7];
EMSCRIPTEN_KEEPALIVE const int32_t* d2_ai_scan_probe(uint32_t unitId) {
    memset(g_scanProbe, 0, sizeof g_scanProbe);
    D2UnitStrc* u = g_game ? SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)unitId) : nullptr;
    if (!u) { g_scanProbe[0] = -1; return g_scanProbe; }
    D2AiControlStrc* ai = AIGENERAL_GetAiControlFromUnit(u); D2ActiveRoomStrc* room = UNITS_GetRoom(u);
    int32_t v66 = 0;
    if (ai && (ai->nAiFlags & 0x40)) v66 = 1;
    else if (room && !DUNGEON_CheckLOSDraw(room)) v66 = ai && !(ai->nAiFlags & 8);
    g_scanProbe[0] = sub_6FC61EC0(u); g_scanProbe[1] = sub_6FC61EE0(u); g_scanProbe[6] = v66;
    D2UnkAiCallbackArgStrc arg = {}; arg.nDistance = INT_MAX; arg.unk0x08 = v66; arg.nMaxDistance = 35;
    arg.unk0x10 = room ? D2Common_10095(room, CLIENTS_GetUnitX(u), CLIENTS_GetUnitY(u)) : 0; arg.nAlternativeDistance = INT_MAX;
    sub_6FCF1E80(g_game, u, &arg, nullptr, 5);
    g_scanProbe[2] = arg.pTarget ? (int32_t)arg.pTarget->dwUnitId : -1; g_scanProbe[3] = arg.nDistance;
    g_scanProbe[4] = arg.pAlternativeTarget ? (int32_t)arg.pAlternativeTarget->dwUnitId : -1; g_scanProbe[5] = arg.nAlternativeDistance;
    return g_scanProbe;
}
// Diagnostic, read-only: the monster ids in each of the unit's adjacent rooms' unit lists, in list order
// (DUNGEON_GetAdjacentRoomsListFromRoom, pUnitFirst / pRoomNext) -- the order the room scan meets them.
static std::string g_roomListProbe;
EMSCRIPTEN_KEEPALIVE const char* d2_room_list_probe(uint32_t unitId) {
    g_roomListProbe.clear();
    D2UnitStrc* u = g_game ? SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)unitId) : nullptr;
    D2ActiveRoomStrc* room = u ? UNITS_GetRoom(u) : nullptr;
    if (!room) return "";
    D2ActiveRoomStrc** list = nullptr; int32_t n = 0; DUNGEON_GetAdjacentRoomsListFromRoom(room, &list, &n);
    char buf[32];
    for (int32_t k = 0; k < n; k++) {
        snprintf(buf, sizeof buf, " [room %d:", k); g_roomListProbe += buf;
        for (D2UnitStrc* v = list[k]->pUnitFirst; v; v = v->pRoomNext) if (v->dwUnitType == UNIT_MONSTER) { snprintf(buf, sizeof buf, " %u", v->dwUnitId); g_roomListProbe += buf; }
        g_roomListProbe += "]";
    }
    return g_roomListProbe.c_str();
}
// Diagnostic: the path target-room test's refusals, newest last (Path.cpp D2WEB_PATH_LogRefusal): one line per
// refusal -- "type:class#id (fx,fy)->(tx,ty) startLevel L targetRoom R(active 1/0/-1 none) in level Lr rect level Lq".
extern int32_t g_d2webPathRefusals[64][12]; extern uint32_t g_d2webPathRefusalCount;
static std::string g_pathRefusalText;
EMSCRIPTEN_KEEPALIVE const char* d2_path_refusals(int32_t maxLines) {
    g_pathRefusalText.clear(); char buf[200];
    const uint32_t n = g_d2webPathRefusalCount < 64 ? g_d2webPathRefusalCount : 64;
    const uint32_t take = maxLines > 0 && (uint32_t)maxLines < n ? (uint32_t)maxLines : n;
    for (uint32_t i = g_d2webPathRefusalCount - take; i < g_d2webPathRefusalCount; i++) {
        const int32_t* r = g_d2webPathRefusals[i & 63];
        snprintf(buf, sizeof buf, "%d:%d#%d (%d,%d)->(%d,%d) lvl %d roomActive %d roomLevel %d rectLevel %d n%d; ", r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[8], r[10], r[9], r[11]);
        g_pathRefusalText += buf;
    }
    return g_pathRefusalText.c_str();
}
static int32_t g_aiProbe[17];
EMSCRIPTEN_KEEPALIVE const int32_t* d2_ai_target_probe(uint32_t unitId, uint32_t targetId) {
    memset(g_aiProbe, 0, sizeof g_aiProbe);
    D2UnitStrc* u = g_game ? SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)unitId) : nullptr;
    D2UnitStrc* t = g_game ? SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)targetId) : nullptr;
    if (!u || !t) { g_aiProbe[0] = -1; return g_aiProbe; }
    D2AiControlStrc* ai = AIGENERAL_GetAiControlFromUnit(u);
    D2ActiveRoomStrc* room = UNITS_GetRoom(u);
    g_aiProbe[0] = ai ? ai->nAiFlags : -1;
    g_aiProbe[1] = room ? DUNGEON_CheckLOSDraw(room) : -1;
    uint32_t* vision = (u->pMonsterData && u->pMonsterData->pVision) ? (uint32_t*)(uintptr_t)u->pMonsterData->pVision : nullptr;
    g_aiProbe[2] = vision ? 1 : 0; g_aiProbe[3] = vision ? (int32_t)vision[9] : -1;
    int32_t v66 = 0;
    if (ai && (ai->nAiFlags & 0x40)) v66 = 1;
    else if (room && !DUNGEON_CheckLOSDraw(room)) { v66 = ai && !(ai->nAiFlags & 8); if (v66 && vision) v66 = vision[9] == 0; }
    g_aiProbe[4] = v66;
    g_aiProbe[5] = UNITS_TestCollisionWithUnit(t, u, COLLIDE_MISSILE_BARRIER);
    g_aiProbe[6] = sub_6FCBD900(g_game, u, t);
    g_aiProbe[7] = AIUTIL_GetDistanceToCoordinates_FullUnitSize(t, u);
    g_aiProbe[8] = STATLIST_GetUnitAlignment(u); g_aiProbe[9] = STATLIST_GetUnitAlignment(t);
    D2MonStatsTxt* ms = MONSTERMODE_GetMonStatsTxtRecord(t->dwClassId); g_aiProbe[10] = ms ? ms->nThreat : -1;
    g_aiProbe[11] = room ? D2Common_10095(room, CLIENTS_GetUnitX(u), CLIENTS_GetUnitY(u)) : -1;
    g_aiProbe[12] = UNITS_GetRoom(t) ? D2Common_10095(UNITS_GetRoom(t), CLIENTS_GetUnitX(t), CLIENTS_GetUnitY(t)) : -1;
    // [13..16] the room scan's gate for the target's room: in the unit's adjacency list, IsRoomInTown,
    // the unit's room nNumClients, the target's room nNumClients.
    D2ActiveRoomStrc* troom = UNITS_GetRoom(t); D2ActiveRoomStrc** list = nullptr; int32_t nl = 0;
    if (room) DUNGEON_GetAdjacentRoomsListFromRoom(room, &list, &nl);
    for (int32_t k = 0; k < nl; k++) if (list[k] == troom) g_aiProbe[13] = 1;
    g_aiProbe[14] = troom ? DUNGEON_IsRoomInTown(troom) : -1;
    g_aiProbe[15] = room ? room->nNumClients : -1; g_aiProbe[16] = troom ? troom->nNumClients : -1;
    return g_aiProbe;
}
EMSCRIPTEN_KEEPALIVE const char* d2_unit_timers(uint32_t unitId) {
    g_unitTimers.clear();
    // unitId 0: the session's player.
    // unitId with bit 31 set: the object of that id (the waypoint whose ENDANIM never fired, F41).
    D2UnitStrc* u = !g_game ? nullptr : (unitId & 0x80000000u) ? SUNIT_GetServerUnit(g_game, UNIT_OBJECT, (int32_t)(unitId & 0x7FFFFFFFu))
        : unitId ? SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)unitId) : g_targetPlayer;
    if (!u) return "";
    char buf[128];
    snprintf(buf, sizeof buf, "frame %u seed %u/%u mode %u hp %d regen %d:", g_game->dwGameFrame, u->pSeed.nLowSeed, u->pSeed.nHighSeed, u->dwAnimMode,
        STATLIST_UnitGetStatValue(u, STAT_HITPOINTS, 0), STATLIST_UnitGetStatValue(u, STAT_HPREGEN, 0)); g_unitTimers += buf;
    for (D2EventTimerStrc* t = SUNIT_GetTimerFromUnit(u); t; t = t->pNext) {
        snprintf(buf, sizeof buf, " [type %d expire %d custom %d/%d]", (int)t->nEventType, t->nExpireFrame, t->dwEventCustomId, t->dwEventCustomParam);
        g_unitTimers += buf;
    }
    return g_unitTimers.c_str();
}
EMSCRIPTEN_KEEPALIVE int d2_aiframe_advance(int frames);
static bool g_aiTargetAlways = false;
// The level's kind-9 records now, with or without a player fixture (no frames advanced).
EMSCRIPTEN_KEEPALIVE int d2_aitarget_snapshot() {
    g_aiTargetAlways = true; const int ok = d2_aiframe_advance(0); g_aiTargetAlways = false; return ok;
}
EMSCRIPTEN_KEEPALIVE int d2_aiframe_advance(int frames);
// 1.13c D2Client+0xADAA0 (packet 0x11, set overlay): overlay 0x97 plays Sounds row 0x18C, overlay 0x98 row 0x18D.
// The server sets those overlays on a life/mana-stealing attacker (UNITS_SetOverlay); the session client hears the
// target player's and its held monsters'.
extern void (*g_d2webOverlayHook)(D2UnitStrc* pUnit, int nOverlay);
static void sessionOverlayHook(D2UnitStrc* unit, int overlay) {
    if (!g_sessionLive || !unit || (overlay != 0x97 && overlay != 0x98)) return;
    if (unit != g_targetPlayer && !(unit->dwUnitType == UNIT_MONSTER && g_heldMonsters.count(unit->dwUnitId))) return;
    g_pendingOverlaySounds.push_back({(int32_t)unit->dwUnitType, (int32_t)unit->dwUnitId, unit->dwClassId, 0x100, -1, 0, overlay == 0x97 ? 0x18C : 0x18D});
}

EMSCRIPTEN_KEEPALIVE int d2_aiframe_advance(int frames) {
    if (!g_game || !g_lastLevel || frames < 0) return 0;
    for (int i = 0; i < frames; i++) {
        ++g_game->dwGameFrame;
        d2web_AdvanceTickCount(40);
        // Retail's day/night clock, one tick per game frame: D2Common #10923
        // ENVIRONMENT_UpdatePeriodOfDay, which advances the act's environment one tick and then
        // recomputes the light intensity (sin(day)*128+128, clamped) and the cycle colour. Retail's
        // client frame calls it; nothing in this tree did, so the environment held its creation state
        // for ever -- NOON, intensity 128, white light -- and every level whose Levels.txt Intensity
        // is 0 (Act I..III outdoors, Harrogath) was lit from that one number. Retail's own captures
        // of such a level read the day phase instead (the Rogue Encampment 137/140/144/146 across
        // four of its own captures, Harrogath 170 = Act V's own clamp in ENVIRONMENT_UpdateLightIntensity),
        // which is why a light row must take its base from its capture rather than the engine
        // (client-parity F11/F156). Driven, the engine's own base is that same day-phase value.
        if (g_currentAct >= 0 && g_currentAct <= 4)
            if (D2DrlgActStrc* pAct = g_game->pAct[g_currentAct])
                if (pAct->pEnvironment)
                    ENVIRONMENT_UpdatePeriodOfDay(pAct, g_targetPlayer ? UNITS_GetRoom(g_targetPlayer) : nullptr);
        EVENT_IterateEvents(g_game);
        // The retail game frame runs the quest timers every 20th frame (D2Game.0x6FC3xxxx game
        // loop: QUESTS_QuestUpdater when dwGameFrame % 20 == 0).
        if (!(g_game->dwGameFrame % 20)) QUESTS_QuestUpdater(g_game);
    }
    g_aiUnits.clear();
    g_aiOut.monsters = 0; g_aiOut.hash = D2VEC_FNV_INIT; g_aiOut.reserved = (uint32_t)frames;
    uint16_t roomIdx = 0;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext, roomIdx++) {
        if (!roomEx->pRoom) continue;
        uint16_t monOrdinal = 0;
        for (D2UnitStrc* u = roomEx->pRoom->pUnitFirst; u; u = u->pRoomNext) {
            if (u->dwUnitType != UNIT_MONSTER) continue;
            D2VecAiUnit a = captureAi(u, roomIdx, monOrdinal++);
            g_aiOut.hash = d2vec_ai_hash(g_aiOut.hash, &a); g_aiOut.monsters++; g_aiUnits.push_back(a);
        }
    }
    // Kind-9 records are filled without a player too when asked (g_aiTargetAlways): s184's snapshot
    // before the fixture exists, as the capture's frame 0xFFFF.
    if (g_targetPlayer || g_aiTargetAlways) {
        g_targetUnits.clear(); g_targetOut = g_aiOut; g_targetOut.hash = D2VEC_FNV_INIT;
        roomIdx = 0;
        for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext, roomIdx++) {
            if (!roomEx->pRoom) continue; uint16_t monOrdinal = 0;
            for (D2UnitStrc* u = roomEx->pRoom->pUnitFirst; u; u = u->pRoomNext) {
                if (u->dwUnitType != UNIT_MONSTER) continue;
                D2VecAiTargetUnit a = captureAiTarget(u, roomIdx, monOrdinal++);
                g_targetOut.hash = d2vec_aitarget_hash(g_targetOut.hash, &a); g_targetUnits.push_back(a);
            }
        }
    }
    return 1;
}

static void updateSessionSnapshot() {
    memset(&g_sessionSnapshot, 0, sizeof g_sessionSnapshot);
    g_sessionSnapshot.version = 1; g_sessionSnapshot.size = sizeof g_sessionSnapshot;
    updateSessionWorld();
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pDynamicPath) return;
    const D2DynamicPathStrc* path = g_targetPlayer->pDynamicPath;
    g_sessionSnapshot.seed = g_gameSeed; g_sessionSnapshot.frame = g_game->dwGameFrame;
    g_sessionSnapshot.level = (uint16_t)g_currentLevel; g_sessionSnapshot.difficulty = (uint8_t)g_gameDiff;
    g_sessionSnapshot.act = (uint8_t)g_currentAct; g_sessionSnapshot.playerClass = g_targetPlayer->dwClassId;
    g_sessionSnapshot.playerId = g_targetPlayer->dwUnitId;
    g_sessionSnapshot.x = path->tGameCoords.wPosX; g_sessionSnapshot.y = path->tGameCoords.wPosY;
    g_sessionSnapshot.targetX = g_sessionMovePending ? g_sessionTargetX : g_sessionSnapshot.x;
    g_sessionSnapshot.targetY = g_sessionMovePending ? g_sessionTargetY : g_sessionSnapshot.y;
    g_sessionSnapshot.hp = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_HITPOINTS, 0);
    g_sessionSnapshot.maxHp = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_MAXHP, 0);
    g_sessionSnapshot.mode = g_targetPlayer->dwAnimMode; g_sessionSnapshot.flags = g_targetPlayer->dwFlags;
    g_sessionSnapshot.monsters = 0;
    for (const D2SessionUnit& unit : g_sessionUnits) if (unit.type == UNIT_MONSTER) ++g_sessionSnapshot.monsters;
    uint32_t h = D2VEC_FNV_INIT;
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&g_sessionSnapshot);
    for (size_t i = 0; i < offsetof(D2SessionSnapshot, hash); ++i) { h ^= bytes[i]; h *= 16777619u; }
    g_sessionSnapshot.hash = h;
}


// First browser-facing lifetime API.  It deliberately builds on the same game, level and player
// fixture used by the retail-diff scenarios, but keeps them alive across commands and ticks.
static int d2_session_create_for_class(uint32_t seed, int difficulty, int act, int levelId, int playerClass, bool startItems = true) {
    if (difficulty < 0 || difficulty > 2 || act < 0 || act > 4 || levelId < 0 || playerClass < 0 || playerClass >= 7) return 0;
    g_sessionMonsterLights.clear();
    g_sessionPlayerClass = playerClass;
    resetSessionClientMapMirror();
    d2_monspawn_reset(); g_sessionLive = false; g_sessionMovePending = false; g_d2ClientSoundObjects = 1;
    g_sessionAiThinkWatchUnit = UINT32_MAX; g_sessionAiThinkWatchCount = 0; g_sessionAiThinkWatchLastFrame = -1;
    g_sessionAiThinkAttachFrame = -1; g_sessionAiThinkAttachMode = -1; g_sessionAiThinkAttachDeadline = -1;
    g_sessionAiThinkAttachRoomX = -1; g_sessionAiThinkAttachRoomY = -1; g_sessionAiThinkAttachRoomHasWarriv = 0;
    g_sessionAiThinkAttachPlayerX = -1; g_sessionAiThinkAttachPlayerY = -1;
    // A new game's client starts with the run toggle off (retail's own default).
    g_sessionRunToggle = false;
    g_sessionReplayFixture = false;
    memset(&g_sessionAttack, 0, sizeof g_sessionAttack); memset(&g_sessionProgress, 0, sizeof g_sessionProgress);
    memset(&g_sessionAdvancement, 0, sizeof g_sessionAdvancement); g_sessionAdvancementSkillId = SKILL_MAGICARROW;
    memset(&g_sessionItemState, 0, sizeof g_sessionItemState);
    g_sessionPendingPickup = 0;
    g_sessionPendingObject = 0;
    g_sessionApproachKind = 0; g_sessionApproachTarget = 0;
    memset(&g_sessionNpcInteraction, 0, sizeof g_sessionNpcInteraction);
    memset(&g_sessionNpcDialog, 0, sizeof g_sessionNpcDialog); D2WEB_ResetLastDialogPacket(); D2WEB_ResetNpcAlerts();
    memset(&g_sessionNpcResponse, 0, sizeof g_sessionNpcResponse);
    memset(&g_sessionVendor, 0, sizeof g_sessionVendor);
    g_sessionVendorItems.clear(); memset(&g_sessionVendorTransaction, 0, sizeof g_sessionVendorTransaction);
    memset(&g_sessionRepair, 0, sizeof g_sessionRepair);
    memset(&g_sessionItemSaveRoundTrip, 0, sizeof g_sessionItemSaveRoundTrip);
    g_sessionItemSaveBytes.clear();
    memset(&g_sessionStatsSaveRoundTrip, 0, sizeof g_sessionStatsSaveRoundTrip);
    g_sessionStatsSaveBytes.clear();
    memset(&g_sessionCampaignSaveRoundTrip, 0, sizeof g_sessionCampaignSaveRoundTrip);
    g_sessionSkillSaveBytes.clear(); g_sessionWaypointSaveBytes.clear();
    memset(&g_sessionQuestIntroSaveRoundTrip, 0, sizeof g_sessionQuestIntroSaveRoundTrip);
    g_sessionQuestSaveBytes.clear(); g_sessionIntroSaveBytes.clear();
    g_sessionAttackTarget = nullptr;
    g_sessionAttackSkill = nullptr; g_sessionAttackCount = 0; g_sessionAttackEndFrame = 0;
    g_sessionAttackResolved = false; g_sessionTargetKillCounted = false; g_sessionKills = 0;
    g_deadRegenCount = 0; g_deadRegenLog.clear(); g_busyStreak = g_busyStartFrame = g_busyLongest = 0; g_busyLog.clear();
    // F64: with `d2_session_create_stream(1)` the level is not built whole -- `d2_monspawn_level`
    // stops at the landing room and the activation wave builds the rest (F53); without it the create
    // builds the level the fixture is about to read. See `g_monspawnSessionCreate` above.
    g_monspawnSessionCreate = g_sessionLazy; g_sessionCreating = 1;
    const int levelOk = d2_monspawn_init() && d2_monspawn_level(seed, difficulty, act, levelId);
    g_monspawnSessionCreate = 0; g_sessionCreating = 0;
    if (!levelOk) {
        updateSessionSnapshot(); return 0;
    }
    const bool sessionAttachAt = g_sessionAttachAtX >= 0 && g_sessionAttachAtY >= 0;
    if (sessionAttachAt) { g_attachAtX = g_sessionAttachAtX; g_attachAtY = g_sessionAttachAtY; g_fixtureOwnRoomOnly = !g_sessionClientFeed; }
    const int attachedOk = d2_aitarget_attach();
    if (sessionAttachAt) { g_attachAtX = g_attachAtY = -1; g_fixtureOwnRoomOnly = false; }
    g_sessionClientStaysInRoom = sessionAttachAt; // retail's headless session never re-rooms the client (QTRACE: 0 hits on 4B1D0)
    if (!attachedOk) { updateSessionSnapshot(); return 0; }
    if (g_targetRoom) {
        g_sessionAiThinkAttachRoomX = g_targetRoom->tCoords.nSubtileX;
        g_sessionAiThinkAttachRoomY = g_targetRoom->tCoords.nSubtileY;
    }
    if (g_targetPlayer && g_targetPlayer->pDynamicPath) {
        g_sessionAiThinkAttachPlayerX = g_targetPlayer->pDynamicPath->tGameCoords.wPosX;
        g_sessionAiThinkAttachPlayerY = g_targetPlayer->pDynamicPath->tGameCoords.wPosY;
    }
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
        for (D2UnitStrc* u = roomEx->pRoom ? roomEx->pRoom->pUnitFirst : nullptr; u; u = u->pRoomNext) {
            if (u->dwUnitType != UNIT_MONSTER || u->dwClassId != 155) continue;
            g_sessionAiThinkAttachFrame = (int32_t)g_game->dwGameFrame;
            g_sessionAiThinkAttachMode = (int32_t)u->dwAnimMode;
            g_sessionAiThinkAttachDeadline = -1;
            for (D2EventTimerStrc* timer = SUNIT_GetTimerFromUnit(u); timer; timer = timer->pNext)
                if (timer->nEventType == EVENTTYPE_AITHINK) { g_sessionAiThinkAttachDeadline = timer->nExpireFrame; break; }
            break;
        }
        if (g_sessionAiThinkAttachFrame >= 0) break;
    }
    // Retail's new character takes its starting stats from D2Game's own routine (0x6FC7EA50,
    // PLAYERSTATS_SetStatsForStartingAct, act I = the CharStats base): attributes, life, AND mana,
    // stamina and next-level experience -- the hand copy this replaced left a Barbarian with 0 mana and
    // 0 stamina (retail's character screen: 10 / 92; s251).
    PLAYERSTATS_SetStatsForStartingAct(g_game, g_targetPlayer, 0);
    // The headless save callback creates a bare unit. Retail new-character creation immediately
    // follows it with the CharStats start-item callback (javelin/shield/potions/scrolls for Amazon).
    if (startItems) PLAYER_CreateStartItemsFromCharStatsTxt(g_targetPlayer, g_game, nullptr);
    // The class's CharStats StartSkill (Sorceress Fire Bolt, Necromancer Raise Skeleton) is granted by its starting
    // item and selected as the RIGHT skill when the game opens -- retail's own save of a fresh character carries it
    // in the header (0x7C; s1002 against retail's Sorceress and Necromancer files). The selection is what the 0x3C
    // handler does; whether retail's client or server issues it at join is not located, the resulting state is measured.
    if (startItems) if (const D2CharStatsTxt* cs = PLRSAVE2_GetCharStatsTxtRecord(g_targetPlayer->dwClassId)) {
        const int32_t startSkill = cs->wStartSkill;
        if (startSkill > 0 && startSkill < sgptDataTables->nSkillsTxtRecordCount) {
            D2SkillStrc* sk = SKILLS_GetSkillById(g_targetPlayer, startSkill, -1);
            if (sk && SKILLS_GetSkillLevel(g_targetPlayer, sk, 1)) { D2GAME_AssignSkill_6FD13800(g_targetPlayer, 0, startSkill, -1); UNITS_InitRightSkill(g_targetPlayer); }
        }
    }
    g_sessionInitialExperience = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_EXPERIENCE, 0);
    g_townSpawnX = g_targetPlayer ? UNITS_GetXPosition(g_targetPlayer) : -1; g_townSpawnY = g_targetPlayer ? UNITS_GetYPosition(g_targetPlayer) : -1;   // the hero's arrival room is the lattice centre
    g_monInitPlayed.clear(); g_prevOnsoundStates.clear(); g_pendingOnsound.clear(); g_pendingOverlaySounds.clear(); g_d2webOverlayHook = sessionOverlayHook; g_sessionLive = true; updateSessionSnapshot(); return 1;
}

EMSCRIPTEN_KEEPALIVE int d2_session_create(uint32_t seed, int difficulty, int act, int levelId) {
    return d2_session_create_for_class(seed, difficulty, act, levelId, 0);
}

// Where a player stands on entering its own act's town. Retail has ONE rule for this: D2Game
// sub_6FC31EF0 (D2MOO Clients.cpp) places a joining player at DUNGEON_FindActSpawnLocationEx(act, the
// act's town, tile 0), whether the character is brand new or loaded from a save. Our session create
// is a fixture and placed the player "beside the first monster" of the landing room instead.
//
// Measured against retail's own draw lists for every act whose town has one (the vector's floor
// anchors give the camera, and the camera IS the player's precise position, a subtile's centre --
// "Units"): Rogue Encampment (4393,4548) for retail's (4393.5,4548.5), Lut Gholein
// (5153,5203) for (5153.5,5203.5), Kurast Docks (5118,5168) for (5118.5,5168.5), Harrogath (5098,5023)
// for (5098.5,5023.5). Before this, the saved-character path was exact and the new-character path was
// 101, 27, 18 and 105 subtiles off respectively -- and Harrogath's was exactly the (5073,5125) beside
// Nihlathak that client-parity F29 recorded. s580 asserts all four, both paths.
//
// Narrow on purpose: only the act's OWN TOWN level. A fixture created in a wilderness level keeps the
// capture's placement, which the vector rows (s05-s08, s420-s429, s997 and the monspawn hashes) replay.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_place_player(int32_t x, int32_t y);
static void placeSessionPlayerAtTownSpawn(int act, int levelId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return;
    if (act < 0 || act >= 5 || !g_game->pAct[act]) return;
    if (levelId != DUNGEON_GetTownLevelIdFromActNo(act)) return;
    int x = 0, y = 0;
    if (DUNGEON_FindActSpawnLocationEx(g_game->pAct[act], levelId, 0, &x, &y, UNITS_GetUnitSizeX(g_targetPlayer)))
    {
        if (g_sessionJoinUnits && UNITS_GetXPosition(g_targetPlayer) == x && UNITS_GetYPosition(g_targetPlayer) == y) return;   // the join already placed it here
        d2_session_fixture_place_player(x, y);
    }
}

EMSCRIPTEN_KEEPALIVE int d2_session_create_class(uint32_t seed, int difficulty, int act, int levelId, int playerClass) {
    g_sessionSeedPlayer = true;
    const int ok = d2_session_create_for_class(seed, difficulty, act, levelId, playerClass);
    g_sessionSeedPlayer = false;
    if (ok) {
        placeSessionPlayerAtTownSpawn(act, levelId);   // a NEW character enters its town at retail's spot too
    }
    return ok;
}
// F22: create the normal session fixture attached in the capture's own room and coordinates, before
// AI timers are armed. The ordinary town-entry fixture attaches at its internal anchor first, then
// relocates the player to the town spawn; that is correct for gameplay startup but not a valid
// comparison for the retail NPC first-think capture.
EMSCRIPTEN_KEEPALIVE int d2_session_create_class_at(uint32_t seed, int difficulty, int act, int levelId, int playerClass, int x, int y) {
    g_sessionSeedPlayer = false;   // retail's fixture player is never SUNIT_InitSeed'ed: its seed stays 0, every damage roll 0 (a retail recording)
    g_sessionAttachAtX = x; g_sessionAttachAtY = y;
    const int ok = d2_session_create_for_class(seed, difficulty, act, levelId, playerClass, false);   // retail fixture is a bare unit: no start kit (a retail recording)
    g_sessionReplayFixture = ok != 0;
    g_sessionAttachAtX = g_sessionAttachAtY = -1;
    g_sessionSeedPlayer = false;
    return ok;
}
// a retail recording/a retail recording/d4-pickup: the same bare fixture as _class_at (unseeded player, no start kit); two names, one body.
EMSCRIPTEN_KEEPALIVE int d2_session_create_bare_at(uint32_t seed, int difficulty, int act, int levelId, int playerClass, int x, int y) {
    return d2_session_create_class_at(seed, difficulty, act, levelId, playerClass, x, y);
}
EMSCRIPTEN_KEEPALIVE int d2_session_create_class_at_bare(uint32_t seed, int difficulty, int act, int levelId, int playerClass, int x, int y) {
    return d2_session_create_class_at(seed, difficulty, act, levelId, playerClass, x, y);
}
// The player a saved file is applied to: no CharStats start kit. Retail creates start items only
// for a newbie save (PLRSAVE2_ReadSaveHeader -> PLRSAVE2ERROR_NEWBIE_SAVE); a saved character
// brings its own items, and a kit created first would sit on top of them (belt slots, body slots).
EMSCRIPTEN_KEEPALIVE int d2_session_create_class_bare(uint32_t seed, int difficulty, int act, int levelId, int playerClass) {
    g_sessionSeedPlayer = true;
    const int ok = d2_session_create_for_class(seed, difficulty, act, levelId, playerClass, false);
    g_sessionSeedPlayer = false;
    if (ok) {
        placeSessionPlayerAtTownSpawn(act, levelId);
    }
    return ok;
}

// The retail capture's player fixture is a bare allocated unit with no starting kit; unit GUIDs
// and the game item seed the quest-drop oracle records depend on that. Mirror it exactly.
EMSCRIPTEN_KEEPALIVE int d2_session_create_raw(uint32_t seed, int difficulty, int act, int levelId) {
    return d2_session_create_for_class(seed, difficulty, act, levelId, 0, false);
}
// The same bare fixture placed beside the first monster of the given class (a town NPC).
EMSCRIPTEN_KEEPALIVE int d2_session_create_raw_near(uint32_t seed, int difficulty, int act, int levelId, uint32_t anchorClass) {
    g_sessionAnchorClass = anchorClass;
    const int ok = d2_session_create_for_class(seed, difficulty, act, levelId, 0, false);
    g_sessionAnchorClass = 0xFFFFFFFF;
    return ok;
}

// Could the player stand on this subtile? Retail's own collision test, with the player's unit size
// and the player-path mask -- the same call d2_session_command_move's handler ends up making.
//
// Why the client needs this: the walk handler above enforces retail's rule that a walk target is
// within 50 subtiles, and the server paths AROUND obstacles between here and there. So a long walk
// is a sequence of short walk commands -- but each one's target must itself be somewhere the player
// could stand. Marching in a straight line toward a destination puts intermediate targets inside
// tents and fences, and the handler rejects them, which is why walking across town stalled.
EMSCRIPTEN_KEEPALIVE int d2_session_walkable(int x, int y) {
    if (!g_sessionLive || !g_targetPlayer || !g_targetPlayer->pDynamicPath) return 0;
    D2ActiveRoomStrc* room = COLLISION_GetRoomBySubTileCoordinates(g_targetPlayer->pDynamicPath->pRoom, x, y);
    if (!room) return 0;
    return COLLISION_CheckMaskWithSize(room, x, y, UNITS_GetUnitSizeX(g_targetPlayer), COLLIDE_MASK_PLAYER_PATH) ? 0 : 1;
}

// The raw collision word at a subtile (COLLIDE_* bits, every mask), -1 off the loaded rooms. Read-only:
// what a stuck fight dumps so static terrain bits and unit footprints can be told apart.
// Also answers for the oracle replays (no live session, a player fixture attached). pattern >= 0:
// COLLISION_CheckAnyCollisionWithPattern(room, x, y, pattern, mask) instead, the path code's test.
EMSCRIPTEN_KEEPALIVE int d2_session_collision(int x, int y) {
    if (!g_targetPlayer || !g_targetPlayer->pDynamicPath) return -1;
    D2ActiveRoomStrc* room = COLLISION_GetRoomBySubTileCoordinates(g_targetPlayer->pDynamicPath->pRoom, x, y);
    return room ? (int)COLLISION_CheckMask(room, x, y, COLLIDE_ALL_MASK) : -1;
}
EMSCRIPTEN_KEEPALIVE int d2_collision_pattern(int x, int y, int pattern, int mask) {
    if (!g_targetPlayer || !g_targetPlayer->pDynamicPath) return -1;
    D2ActiveRoomStrc* room = COLLISION_GetRoomBySubTileCoordinates(g_targetPlayer->pDynamicPath->pRoom, x, y);
    return room ? COLLISION_CheckAnyCollisionWithPattern(room, x, y, pattern, (uint16_t)mask) : -1;
}

// The movement ray (#10851) uses the dynamic path's own room, pattern and move mask. Its caller
// removes the mover's registered footprint before querying and restores it afterward. Keep this
// separate from d2_collision_pattern, whose caller-supplied pattern/mask intentionally observes
// the currently registered session collision plane.
EMSCRIPTEN_KEEPALIVE int d2_session_path_collision(int x, int y) {
    D2UnitStrc* player = g_targetPlayer;
    D2DynamicPathStrc* path = player ? player->pDynamicPath : nullptr;
    if (!path || !path->pRoom) return -1;

    const BOOL removedPlayerFootprint = PATH_RemoveCollisionFootprintForUnit(player, FALSE);
    const int collision = d2::path::movementCollisionQuery(*path, x, y);
    if (removedPlayerFootprint) PATH_AddCollisionFootprintForUnit(player);
    return collision;
}

// Why the last walk command was refused. 0 = accepted; see d2_session_move_reason.
// A walk that is simply rejected is indistinguishable from one that was never sent, and chasing
// that difference by guesswork cost real time.
static int g_sessionMoveReason = 0;
using D2PathAttemptRecord = d2::path_attempt_trace::Record;
static constexpr int32_t kPathAttemptTraceCapacity = 256;
static D2PathAttemptRecord g_pathAttemptTrace[kPathAttemptTraceCapacity]{};
static int32_t g_pathAttemptTraceCount = 0;
static int32_t g_pathAttemptTraceDropped = 0;
static int32_t g_pathAttemptTraceEnabled = 0;
static int32_t g_pathAttemptContext = d2::path_attempt_trace::contextNone;
static int32_t g_pathAttemptPlannedX = -1, g_pathAttemptPlannedY = -1;
enum {
    MOVE_OK = 0, MOVE_NO_SESSION = 1, MOVE_OUT_OF_RANGE = 2, MOVE_NO_ROOM = 3,
    MOVE_HANDLER_REFUSED = 4, MOVE_NO_PATH = 5,
    // A refused walk has TWO possible subjects and they must never be confused: the GROUND (3, 5 --
    // this subtile has no active room, or no route from here reaches it) and the PLAYER (6 -- the
    // ground is fine, this player may not walk right now). Retail's dispatcher refuses a walk packet
    // from a dying, dead or uninterruptable player before any terrain is consulted, so both answers
    // are correct at once; reporting the player's refusal as MOVE_HANDLER_REFUSED made the client's
    // aim loop read "the world will not take this subtile" when the world was fine, and it re-sent an
    // identical refused aim until its budget expired (client-parity F19's pocket).
    MOVE_DISPATCH_REFUSED = 6,
};
EMSCRIPTEN_KEEPALIVE int d2_session_move_reason() { return g_sessionMoveReason; }

// Opt-in bounded diagnostic for the next production move attempt. The route helper marks
// one-point plans and empty-plan fallbacks; command_move consumes the mark and captures the
// native path result/reason at the same call boundary. This does not affect movement decisions.
EMSCRIPTEN_KEEPALIVE void d2_session_path_attempt_trace_control(int32_t enabled, int32_t reset) {
    if (reset) { g_pathAttemptTraceCount = 0; g_pathAttemptTraceDropped = 0; }
    g_pathAttemptTraceEnabled = enabled != 0;
    if (!enabled) g_pathAttemptContext = d2::path_attempt_trace::contextNone;
}
EMSCRIPTEN_KEEPALIVE void d2_session_path_attempt_context(int32_t context, int32_t plannedX, int32_t plannedY) {
    if (context == d2::path_attempt_trace::contextNone) {
        g_pathAttemptContext = d2::path_attempt_trace::contextNone;
        g_pathAttemptPlannedX = g_pathAttemptPlannedY = -1;
        return;
    }
    g_pathAttemptTraceEnabled = 1;
    g_pathAttemptContext = context;
    g_pathAttemptPlannedX = plannedX;
    g_pathAttemptPlannedY = plannedY;
}
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_path_attempt_trace(int32_t* count) {
    if (count) *count = g_pathAttemptTraceCount;
    return reinterpret_cast<const int32_t*>(g_pathAttemptTrace);
}
EMSCRIPTEN_KEEPALIVE int32_t d2_session_path_attempt_trace_dropped() { return g_pathAttemptTraceDropped; }
// F671 diagnostic: one command through its client wrapper, emitted D2GS packet, ordinal-10334
// resolver/path output, and subsequent movement continuation. Read-only and overwritten by the
// next move command; sequence distinguishes a fresh attempt from tick updates to that attempt.
D2WebWalkProbe g_d2webWalkProbe{};
int32_t g_d2webWalkProbeActive = 0;
static int32_t g_d2webWalkSequence = 0;
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_walk_probe() {
	return reinterpret_cast<const int32_t*>(&g_d2webWalkProbe);
}

// F55 diagnostic: the path planner's own decisions for the walk command just issued (PATH_Straight_Compute
// and, if it ran, PATH_AStar_ComputePath) -- read right after d2_session_command_move, before another
// move overwrites them. [0] Toward's own point count, [1] Toward's early-return fired, [2] the A* distance
// gate (squared <= 324) let A* run, [3] the squared straight-line distance, [4] A* nodes popped, [5] A*
// advanced past the start node (has a parent), [6] A*'s 200-node pool (STORAGE_SIZE) was exhausted,
// [7] A*'s own path point count (-2 = TargetLocationHasEnoughRoom rejected it before the search ran).
extern int32_t g_d2webStraightTowardPoints, g_d2webStraightTowardEarlyReturn, g_d2webStraightAStarGate, g_d2webStraightSquaredDist;
extern int32_t g_d2webAStarNodesPopped, g_d2webAStarAdvanced, g_d2webAStarExhausted, g_d2webAStarPathPoints;
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_astar_probe() {
    static int32_t probe[8];
    probe[0] = g_d2webStraightTowardPoints; probe[1] = g_d2webStraightTowardEarlyReturn;
    probe[2] = g_d2webStraightAStarGate; probe[3] = g_d2webStraightSquaredDist;
    probe[4] = g_d2webAStarNodesPopped; probe[5] = g_d2webAStarAdvanced;
    probe[6] = g_d2webAStarExhausted; probe[7] = g_d2webAStarPathPoints;
    return probe;
}

// Retail's client-packet dispatcher (D2GAME_PACKET_Handler_6FC89320, 1.13c the same) lets a packet
// through only when D2GAME_PLRMODES_First_6FC7F340 allows it -- not while the player is dying, dead or
// uninterruptable -- except chat (0x14/0x15/0x3C/0x43) and, only when dead, resurrect (0x41). The
// session calls handlers directly, so every client-packet command asks this first; without it a dead
// player spent stat points and drank potions, and stood dead at 3,194 hp (s000a).
static int32_t g_sessionDispatchRefused = 0;
static bool sessionDispatchGate(uint8_t header) {
    if (!g_game || !g_targetPlayer) return false;
    if (header == 0x14 || header == 0x15 || header == 0x3C || header == 0x43) return true;
    if (header == 0x41) return g_targetPlayer->dwAnimMode == PLRMODE_DEAD;
    if (D2GAME_PLRMODES_First_6FC7F340(g_game, g_targetPlayer, 1, 0, 0)) return true;
    ++g_sessionDispatchRefused;
    return false;
}
EMSCRIPTEN_KEEPALIVE int32_t d2_session_dispatch_refused() { return g_sessionDispatchRefused; }

// D2Client's run/walk toggle, and the only place retail keeps one. Retail has NO server-side run
// flag: the CLIENT owns the toggle and chooses which of the two move packets to send -- 0x01 Walk
// or 0x03 Run (D2Game's own handlers, PlrMsg.cpp D2GAME_PACKETCALLBACK_Rcv0x01_Walk_6FC82CB0 and
// D2GAME_PACKETCALLBACK_Rcv0x03_Run_6FC83090, each calling sub_6FC817D0 with PLRMODE_WALK or
// PLRMODE_RUN) -- and the mode the hero then plays, and the stamina drain that follows it
// (sub_6FC7F780 drains only while dwAnimMode == PLRMODE_RUN and the room is not a town), are both
// downstream of that choice. This session IS the client here: d2_session_command_move builds the
// packet itself and calls the handler directly, so the toggle lives with it.
// Default OFF, which is retail's: a new game walks, and every captured scene draws the button in
// its untoggled frame. Off is also what every pre-existing row expects,
// so the packet this sends is unchanged for them.
EMSCRIPTEN_KEEPALIVE int d2_session_toggle_run(int on) {
    g_sessionRunToggle = on != 0;
    return g_sessionRunToggle ? 1 : 0;
}
EMSCRIPTEN_KEEPALIVE int d2_session_run_state() { return g_sessionRunToggle ? 1 : 0; }

// The run drain's own inputs, and its effect, in one record -- so a stamina stall is never guessed
// at again. Retail drains in sub_6FC7F780 (PlrModes.cpp), reached once a game frame from the player's
// infinite EVENTTYPE_MODECHANGE timer through the mode-handler table; every condition on the way in
// and the result of the drain itself are here:
//   [0] game frame, [1] STAT_STAMINA before, [2] dwAnimMode,
//   [3] DUNGEON_IsRoomInTown(UNITS_GetRoom(hero)) -- the drain's own town exclusion,
//   [4] DUNGEON_IsRoomInTown(path->pRoom) -- what the neutral-mode setters read,
//   [5] CharStats nRunDrain (the drain is 2x this, min 1),
//   [6] STAT_MAXSTAMINA (EVENTS_StaminaRegen regens only below it),
//   [7] the EVENTTYPE_MODECHANGE timer's expire frame (-2 when the hero has none),
//   [8] STAT_STAMINA after retail's sub_6FC7F780 has run once (drain != 0), so a zero at [0..7] that
//       still moves [8] is an event-path defect and not an arithmetic one,
//   [9] dwAnimMode after that call, [10] DRLGROOM_GetLevelId(room) -- the level id [3] is derived
//       from, so "in town" is never read off the session's own idea of where the hero is.
// Read-only with drain == 0: a caller may ask this every leg of a walk without moving a stat.
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_run_probe(int32_t drain) {
    static int32_t probe[12];
    memset(probe, 0, sizeof probe);
    if (!g_sessionLive || !g_game || !g_targetPlayer) { probe[0] = -1; return probe; }
    D2UnitStrc* p = g_targetPlayer;
    probe[0] = (int32_t)g_game->dwGameFrame;
    probe[1] = STATLIST_UnitGetStatValue(p, STAT_STAMINA, 0);
    probe[2] = (int32_t)p->dwAnimMode;
    D2ActiveRoomStrc* room = UNITS_GetRoom(p);
    probe[3] = DUNGEON_IsRoomInTown(room) ? 1 : 0;
    probe[4] = (p->pDynamicPath && DUNGEON_IsRoomInTown(p->pDynamicPath->pRoom)) ? 1 : 0;
    D2CharStatsTxt* cs = PLRSAVE2_GetCharStatsTxtRecord(p->dwClassId);
    probe[5] = cs ? (int32_t)cs->nRunDrain : -1;
    probe[6] = STATLIST_UnitGetStatValue(p, STAT_MAXSTAMINA, 0);
    probe[7] = -2;
    for (D2EventTimerStrc* t = SUNIT_GetTimerFromUnit(p); t; t = t->pNext)
        if (t->nEventType == EVENTTYPE_MODECHANGE) { probe[7] = t->nExpireFrame; break; }
    if (drain) sub_6FC7F780(g_game, p, 0, 0);
    probe[8] = STATLIST_UnitGetStatValue(p, STAT_STAMINA, 0);
    probe[9] = (int32_t)p->dwAnimMode;
    probe[10] = (room && room->pDrlgRoom) ? DRLGROOM_GetLevelId(room->pDrlgRoom) : -1;
    return probe;
}

extern "C" void d2_client_note_move_cmd(int32_t explicitMove);
EMSCRIPTEN_KEEPALIVE int d2_session_command_move(int targetX, int targetY) {
    d2_client_note_move_cmd(1);
    memset(&g_d2webWalkProbe, 0, sizeof g_d2webWalkProbe);
    g_d2webWalkProbe.sequence = ++g_d2webWalkSequence;
    g_d2webWalkProbe.commandX = targetX; g_d2webWalkProbe.commandY = targetY;
    // A command rejected before entering the path routine has no path result. Do not export the
    // previous command's counters as if they belonged to this target.
    g_d2webStraightTowardPoints = g_d2webStraightTowardEarlyReturn = g_d2webStraightAStarGate = -1;
    g_d2webStraightSquaredDist = g_d2webAStarNodesPopped = g_d2webAStarAdvanced = -1;
    g_d2webAStarExhausted = g_d2webAStarPathPoints = -1;
    const int32_t context = g_pathAttemptContext;
    const int32_t plannedX = g_pathAttemptPlannedX, plannedY = g_pathAttemptPlannedY;
    g_pathAttemptContext = d2::path_attempt_trace::contextNone;
    auto finish = [&](int result) {
        if (g_pathAttemptTraceEnabled && context != d2::path_attempt_trace::contextNone) {
            if (g_pathAttemptTraceCount < kPathAttemptTraceCapacity) {
                const int32_t* astar = d2_session_astar_probe();
                D2PathAttemptRecord& row = g_pathAttemptTrace[g_pathAttemptTraceCount];
                row = { g_pathAttemptTraceCount + 1, context, plannedX, plannedY,
                    targetX, targetY, result, g_sessionMoveReason,
                    astar[0], astar[1], astar[2], astar[3], astar[4], astar[5], astar[6], astar[7] };
                ++g_pathAttemptTraceCount;
            } else ++g_pathAttemptTraceDropped;
        }
        return result;
    };
    g_sessionMoveReason = MOVE_NO_SESSION;
    if (!g_sessionLive || !g_targetPlayer || !g_targetPlayer->pDynamicPath) {
        g_d2webWalkProbe.moveReason = g_sessionMoveReason;
        return finish(0);
    }
    const D2DynamicPathStrc* path = g_targetPlayer->pDynamicPath;
    g_d2webWalkProbe.commandStartX = path->tGameCoords.wPosX;
    g_d2webWalkProbe.commandStartY = path->tGameCoords.wPosY;
    g_sessionMoveReason = MOVE_OUT_OF_RANGE;
    if (abs(targetX - (int)path->tGameCoords.wPosX) > 50 || abs(targetY - (int)path->tGameCoords.wPosY) > 50) {
        g_d2webWalkProbe.moveReason = g_sessionMoveReason;
        return finish(0);
    }
    g_sessionMoveReason = MOVE_NO_ROOM;
    D2ActiveRoomStrc* targetRoom = COLLISION_GetRoomBySubTileCoordinates(path->pRoom, targetX, targetY);
    g_d2webWalkProbe.wrapperRoomFound = targetRoom != nullptr;
    if (!targetRoom) {
        g_d2webWalkProbe.moveReason = g_sessionMoveReason;
        return finish(0);
    }
    g_sessionTargetX = targetX; g_sessionTargetY = targetY;
    const uint8_t header = g_sessionRunToggle ? 0x03 : 0x01;
    const uint16_t x = (uint16_t)targetX, y = (uint16_t)targetY;
    uint8_t packet[5];
    if (d2_realm_d2gs_move_to(header, x, y, packet) != (int)sizeof packet) {
        g_d2webWalkProbe.moveReason = g_sessionMoveReason;
        return finish(0);
    }
    g_d2webWalkProbe.packetBuilt = 1;
    g_d2webWalkProbe.packet0 = packet[0]; g_d2webWalkProbe.packet1 = packet[1];
    g_d2webWalkProbe.packet2 = packet[2]; g_d2webWalkProbe.packet3 = packet[3]; g_d2webWalkProbe.packet4 = packet[4];
    if (!sessionDispatchGate(header)) {
        g_sessionMoveReason = MOVE_DISPATCH_REFUSED;
        g_d2webWalkProbe.moveReason = g_sessionMoveReason;
        return finish(0);
    }
    g_sessionMoveReason = MOVE_HANDLER_REFUSED;
    g_d2webWalkProbeActive = 1;
    const int32_t refused = g_sessionRunToggle
        ? D2GAME_PACKETCALLBACK_Rcv0x03_Run_6FC83090(g_game, g_targetPlayer, packet, sizeof packet)
        : D2GAME_PACKETCALLBACK_Rcv0x01_Walk_6FC82CB0(g_game, g_targetPlayer, packet, sizeof packet);
    g_d2webWalkProbeActive = 0;
    g_d2webWalkProbe.handlerResult = refused;
    g_d2webWalkProbe.pathPoints = path->dwPathPoints;
    g_d2webWalkProbe.pathIndex = path->dwCurrentPointIdx;
    if (refused) {
        g_d2webWalkProbe.moveReason = g_sessionMoveReason;
        return finish(0);
    }
    g_sessionMovePending = path->dwPathPoints > 0;
    g_sessionMoveReason = g_sessionMovePending ? MOVE_OK : MOVE_NO_PATH;
    g_d2webWalkProbe.moveReason = g_sessionMoveReason;
    updateSessionSnapshot();
    return finish(g_sessionMovePending ? 1 : 0);
}

// After LEVEL_WarpUnit (an exit, or a portal object's operate function) moved the player, adopt
// the resulting room and level. Retail's exit operation invokes the quest level-change dispatcher
// after its successful room transfer; LEVEL_WarpUnit only moves the unit, so that recovered
// second half lives here.
static int adoptSessionLevelChange(int32_t oldLevel, int32_t destinationLevel) {
    D2ActiveRoomStrc* room = UNITS_GetRoom(g_targetPlayer);
    if (!room || DUNGEON_GetLevelIdFromRoom(room) != destinationLevel || !room->pDrlgRoom || !room->pDrlgRoom->pLevel) return 0;
    g_currentAct = DRLG_GetActNoFromLevelId(destinationLevel);
    g_currentLevel = destinationLevel; g_lastLevel = room->pDrlgRoom->pLevel;
    QUESTS_ChangeLevel(g_game, oldLevel, destinationLevel, g_targetPlayer);
    activateSessionLevelRooms(g_game->pAct[g_currentAct], g_lastLevel, destinationLevel);
    g_targetRoom = room; g_targetMonster = nullptr; g_sessionMovePending = false;
    g_sessionAttackTarget = nullptr; g_sessionAttackSkill = nullptr; memset(&g_sessionAttack, 0, sizeof g_sessionAttack);
    updateSessionSnapshot(); syncSessionClientMapFromPlayer(); return 1;
}

// F56: the source-side tile a warp click stands for. Retail's caller for a click on a level exit is
// the client's 0x13 interact with the UNIT_TILE under it (D2GAME_PACKETCALLBACK_Rcv0x13_
// InteractWithEntity_6FC84D70 -> sub_6FC828D0's UNIT_TILE case, PlrMsg.cpp): within 4 subtiles of the
// tile it calls SUNIT_WarpPlayer, which places the player at the tile the warp PAIRS with
// (DUNGEON_GetWarpTileFromRoomAndSourceLevelId -> COLLISION_GetFreeCoordinates(..., 0x1C09u, 1)) --
// the `DRLGROOMTILE_AddWarp` record, one per exit. LEVEL_WarpUnit answers instead with a point of the
// destination LEVEL (DUNGEON_FindActSpawnLocationEx -> sub_6FD788D0, whose nTileCalc match is written
// for the classic WarpN values 0-13 and sees an LoD level's LvlWarp id match nothing), which is how
// Lut Gholein's two sewer doorways came to arrive on one subtile.
//
// The tile the warp LEADS TO from a source tile -- `SUNIT_WarpPlayer`'s own lookup, exposed so the
// command can ask whether a candidate tile is the one whose room carries the link. A warp tile unit's
// room holds that link in its DRLG room-tile array (`DRLGWARP_GetDestinationRoom` matches the array's
// LvlWarp record), and a tile whose room does not is not the clicked exit: `SUNIT_WarpPlayer` answers
// with nothing at all for it (its `if (!pWarpTile) return;`), which is how a wrong candidate reads as
// "the warp did not happen" instead of a wrong arrival.
static D2UnitStrc* sessionWarpPairedTile(D2UnitStrc* tile, D2LvlWarpTxt** txt) {
    D2ActiveRoomStrc* room = tile ? UNITS_GetRoom(tile) : nullptr;
    if (!room || !room->pDrlgRoom) return nullptr;
    // the callee stores through the out-pointer unconditionally (retail's callers always pass one); a null store on wasm hits address 0
    D2LvlWarpTxt* ignored = nullptr;
    return DUNGEON_GetWarpTileFromRoomAndSourceLevelId(room, (int)tile->dwClassId, txt ? txt : &ignored);
}

// Whether the lookup above CAN answer for this tile -- `DRLGWARP_GetDestinationRoom`'s own two
// conditions, asked first because the vendored function asserts on a destination room it cannot
// resolve (`D2_ASSERT(pDestRoom)`, D2Dungeon.cpp, and D2_ASSERT aborts: s25 and s88 died there the
// first time this path was driven). A tile whose room carries no link node of its id, or whose link
// leads to a room that does not link back, is simply not this exit's tile; the command falls through
// to LEVEL_WarpUnit for it, exactly as it did before the tile path existed.
static bool sessionWarpLinkResolves(D2UnitStrc* tile) {
    D2ActiveRoomStrc* room = tile ? UNITS_GetRoom(tile) : nullptr;
    D2DrlgRoomStrc* drlgRoom = room ? room->pDrlgRoom : nullptr;
    if (!drlgRoom) return false;
    for (const D2RoomTileStrc* node = drlgRoom->pRoomTiles; node; node = node->pNext) {
        if (!node->pLvlWarpTxtRecord || (uint32_t)node->pLvlWarpTxtRecord->dwLevelId != tile->dwClassId) continue;
        D2DrlgRoomStrc* destination = node->pDrlgRoom;
        if (!destination) return false;
        for (const D2RoomTileStrc* back = destination->pRoomTiles; back; back = back->pNext)
            if (back->pDrlgRoom == drlgRoom) return true;
        return false;
    }
    return false;
}

// The tile a click on this exit stands for: a warp tile unit's dwClassId is its LvlWarp id -- the
// same column `g_sessionWarpPresets` collects -- so the candidates are the level's built tiles of that
// id, and the answer is the nearest one whose own room resolves the pair. The lookup itself builds the
// DESTINATION room as a side effect (`DRLGWARP_GetDestinationRoom` -> DRLGACTIVATE_InitializeRoomEx)
// and a room built that way has its tiles and nothing else: its preset units -- the paired warp TILE
// among them, which is the very unit this lookup returns -- appear on the pending-rooms pass, exactly
// the rule `d2_session_command_seam` follows for the room it builds. So the search runs that pass and
// asks once more; measured (s423): before it the pair lookup answers nothing for both of
// Lut Gholein's sewer doorways, and after it the two answer their OWN tiles, class 22 at (7520,5008)
// and class 21 at (7845,5278), which is Levels.txt's pairing 19<->22, 20<->21.
static void updateSessionPendingRooms();

static D2UnitStrc* sessionWarpClickTile(uint32_t warpId, int32_t x, int32_t y) {
    if (!g_lastLevel) return nullptr;
    std::vector<std::pair<int, D2UnitStrc*>> candidates;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
        D2ActiveRoomStrc* room = roomEx->pRoom;
        if (!room) continue;
        for (D2UnitStrc* tile = room->pUnitFirst; tile; tile = tile->pRoomNext) {
            if (tile->dwUnitType != UNIT_TILE || !tile->pStaticPath || tile->dwClassId != warpId) continue;
            const int distance = std::max(abs(tile->pStaticPath->tGameCoords.nX - x), abs(tile->pStaticPath->tGameCoords.nY - y));
            candidates.push_back({ distance, tile });
        }
    }
    std::stable_sort(candidates.begin(), candidates.end(),
        [](const std::pair<int, D2UnitStrc*>& a, const std::pair<int, D2UnitStrc*>& b) { return a.first < b.first; });
    for (int pass = 0; pass < 2; ++pass) {
        for (const std::pair<int, D2UnitStrc*>& candidate : candidates)
            if (sessionWarpLinkResolves(candidate.second) && sessionWarpPairedTile(candidate.second, nullptr)) return candidate.second;
        if (pass == 0) updateSessionPendingRooms();
    }
    return nullptr;
}

// Distance from the player to the closest currently built UNIT_TILE for this exit. The client uses
// this before a stalled walk retries its warp: retail's handler enters interaction mode beyond four
// subtiles and calls SUNIT_WarpPlayer only inside that range. Keep this query read-only; resolving a
// paired destination tile can build rooms, which a rejected interaction must not do.
EMSCRIPTEN_KEEPALIVE int d2_session_warp_interaction_distance(uint32_t destinationLevel, uint32_t warpId) {
    if (!g_sessionLive || !g_targetPlayer || !g_lastLevel) return -1;
    const D2SessionWarp* exit = nullptr;
    for (const D2SessionWarp& warp : g_sessionWarps) {
        if (warp.destinationLevel == destinationLevel && warp.warpId == warpId && warp.enabled) {
            exit = &warp; break;
        }
    }
    if (!exit) return -1;
    D2UnitStrc* nearest = nullptr;
    int nearestPointDistance = INT32_MAX;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
        D2ActiveRoomStrc* room = roomEx->pRoom;
        if (!room) continue;
        for (D2UnitStrc* tile = room->pUnitFirst; tile; tile = tile->pRoomNext) {
            if (tile->dwUnitType != UNIT_TILE || tile->dwClassId != warpId || !tile->pStaticPath) continue;
            const int pointDistance = std::max(abs(tile->pStaticPath->tGameCoords.nX - exit->exitX),
                                               abs(tile->pStaticPath->tGameCoords.nY - exit->exitY));
            if (pointDistance < nearestPointDistance) {
                nearest = tile;
                nearestPointDistance = pointDistance;
            }
        }
    }
    return nearest ? D2Common_10399(g_targetPlayer, nearest) : -1;
}

// PROBE (F56, lands with the arrival measurement it justifies): the click path's own census, because
// "the warp did not happen" has three different causes here and they are told apart by counting. Read
// out: {candidates of this id, of those with a static path, the nearest one's class/x/y, the room's
// DRLG room-tile nodes / of those with a record / of those whose record matches the tile's id, the
// room's level, then the pair lookup: found, dest class, dest x, dest y, dest level, dest room's
// UNIT_TILE count, dest record}, and last the tile the exit OBJECT's own operate path passes to
// `SUNIT_WarpPlayer` (its room's FIRST UNIT_TILE, whatever its class) with whether THAT pair resolves:
// the two callers are one placement in retail, so if one of them resolves and the other does not, the
// difference is the tile and not the function. Read-only; -1 everywhere when absent.
static int32_t g_sessionWarpTileProbe[18];
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_warp_tile_probe(uint32_t destinationLevel, uint32_t warpId, int32_t* count) {
    for (int i = 0; i < 18; ++i) g_sessionWarpTileProbe[i] = -1;
    if (count) *count = 18;
    if (!g_sessionLive || !g_lastLevel) return g_sessionWarpTileProbe;
    const D2SessionWarp* exit = nullptr;
    for (const D2SessionWarp& warp : g_sessionWarps)
        if (warp.destinationLevel == destinationLevel && warp.warpId == warpId && warp.enabled) { exit = &warp; break; }
    if (!exit) return g_sessionWarpTileProbe;
    int candidates = 0, withPath = 0, bestDistance = INT32_MAX; D2UnitStrc* best = nullptr;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
        if (!roomEx->pRoom) continue;
        for (D2UnitStrc* tile = roomEx->pRoom->pUnitFirst; tile; tile = tile->pRoomNext) {
            if (tile->dwUnitType != UNIT_TILE || tile->dwClassId != warpId) continue;
            ++candidates;
            if (!tile->pStaticPath) continue;
            ++withPath;
            const int distance = std::max(abs(tile->pStaticPath->tGameCoords.nX - exit->exitX),
                                          abs(tile->pStaticPath->tGameCoords.nY - exit->exitY));
            if (distance < bestDistance) { best = tile; bestDistance = distance; }
        }
    }
    g_sessionWarpTileProbe[0] = candidates; g_sessionWarpTileProbe[1] = withPath;
    if (!best) return g_sessionWarpTileProbe;
    g_sessionWarpTileProbe[2] = (int32_t)best->dwClassId;
    g_sessionWarpTileProbe[3] = best->pStaticPath->tGameCoords.nX;
    g_sessionWarpTileProbe[4] = best->pStaticPath->tGameCoords.nY;
    D2ActiveRoomStrc* room = UNITS_GetRoom(best);
    D2DrlgRoomStrc* drlgRoom = room ? room->pDrlgRoom : nullptr;
    int nodes = 0, withRecord = 0, matching = 0;
    if (drlgRoom) for (const D2RoomTileStrc* node = drlgRoom->pRoomTiles; node; node = node->pNext) {
        ++nodes;
        if (!node->pLvlWarpTxtRecord) continue;
        ++withRecord;
        if ((uint32_t)node->pLvlWarpTxtRecord->dwLevelId == best->dwClassId) ++matching;
    }
    g_sessionWarpTileProbe[5] = nodes; g_sessionWarpTileProbe[6] = withRecord; g_sessionWarpTileProbe[7] = matching;
    g_sessionWarpTileProbe[8] = room ? DUNGEON_GetLevelIdFromRoom(room) : -1;
    D2LvlWarpTxt* txt = nullptr;
    D2UnitStrc* paired = sessionWarpPairedTile(best, &txt);
    g_sessionWarpTileProbe[9] = paired ? 1 : 0;
    if (paired) {
        D2ActiveRoomStrc* pairedRoom = UNITS_GetRoom(paired);
        int destTiles = 0;
        if (pairedRoom) for (D2UnitStrc* t = pairedRoom->pUnitFirst; t; t = t->pRoomNext) if (t->dwUnitType == UNIT_TILE) ++destTiles;
        g_sessionWarpTileProbe[10] = (int32_t)paired->dwClassId;
        g_sessionWarpTileProbe[11] = CLIENTS_GetUnitX(paired);
        g_sessionWarpTileProbe[12] = CLIENTS_GetUnitY(paired);
        g_sessionWarpTileProbe[13] = pairedRoom ? DUNGEON_GetLevelIdFromRoom(pairedRoom) : -1;
        g_sessionWarpTileProbe[14] = destTiles;
        g_sessionWarpTileProbe[15] = txt ? (int32_t)txt->dwLevelId : -1;
    }
    // What the exit OBJECT's operate passes: OperateFn 16 walks its own room's unit list to the first
    // UNIT_TILE of ANY class (`OBJECTS_OperateFunction16_TrapDoor`, ObjMode.cpp) and calls the same
    // SUNIT_WarpPlayer. Reported with its own pair lookup, so "one placement, two callers" is a count.
    if (room) for (D2UnitStrc* tile = room->pUnitFirst; tile; tile = tile->pRoomNext) {
        if (tile->dwUnitType != UNIT_TILE) continue;
        g_sessionWarpTileProbe[16] = (int32_t)tile->dwClassId;
        g_sessionWarpTileProbe[17] = sessionWarpPairedTile(tile, nullptr) ? 1 : 0;
        break;
    }
    return g_sessionWarpTileProbe;
}

EMSCRIPTEN_KEEPALIVE int d2_session_command_warp(uint32_t destinationLevel, uint32_t warpId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetClient) return 0;
    updateSessionSnapshot();
    const D2SessionWarp* exit = nullptr;
    for (const D2SessionWarp& warp : g_sessionWarps) {
        if (warp.destinationLevel == destinationLevel && warp.warpId == warpId && warp.enabled) {
            exit = &warp; break;
        }
    }
    if (!exit) return 0;
    const int32_t oldLevel = DUNGEON_GetLevelIdFromRoom(UNITS_GetRoom(g_targetPlayer));
    // Retail's own placement for a warp click (F56): the client's 0x13 interact with the UNIT_TILE
    // under it calls SUNIT_WarpPlayer with that tile, which places the player at the tile the warp
    // PAIRS with -- one arrival per exit, not one point of the destination level. SUNIT_WarpPlayer
    // moves the player between rooms of one act and nothing else, so an exit into another act stays
    // LEVEL_WarpUnit's (the act change, SUNITPROXY_UpdateNpcsOnActChange and LEVEL_ChangeAct, is its
    // path in retail too); so does an exit whose tile the click path cannot resolve.
    D2UnitStrc* tile = DRLG_GetActNoFromLevelId((int32_t)destinationLevel) == g_currentAct
        ? sessionWarpClickTile(warpId, exit->exitX, exit->exitY) : nullptr;
    if (tile) {
        SUNIT_WarpPlayer(g_game, g_targetPlayer, tile);
        // SUNIT_WarpPlayer answers with nothing when its own COLLISION_GetFreeCoordinates or room
        // transfer refuses, and a warp that places nobody is worse than the level search's answer:
        // s25's act III transition is one (measured). Only a placement that moved the
        // player ends here; otherwise the exit keeps the answer it had before the tile path existed.
        if (adoptSessionLevelChange(oldLevel, (int32_t)destinationLevel)) return 1;
    }
    LEVEL_WarpUnit(g_game, g_targetPlayer, (int32_t)destinationLevel, (int32_t)warpId);
    return adoptSessionLevelChange(oldLevel, (int32_t)destinationLevel);
}

// A player click on a same-act level exit reaches the UNIT_TILE arm of retail's 0x13
// interaction handler. That arm calls SUNIT_WarpPlayer only after D2Common_10399 reports
// distance <= 4; d2_session_command_warp is also used by fixtures as an explicit remote
// transition, so keep its unrestricted behavior and expose the click predicate separately.
EMSCRIPTEN_KEEPALIVE int d2_session_warp_tile_in_interact_range(uint32_t destinationLevel, uint32_t warpId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_lastLevel) return 0;
    updateSessionSnapshot();
    const D2SessionWarp* exit = nullptr;
    for (const D2SessionWarp& warp : g_sessionWarps) {
        if (warp.destinationLevel == destinationLevel && warp.warpId == warpId && warp.enabled) {
            exit = &warp; break;
        }
    }
    if (!exit) return 0;
    // Cross-act transitions retain the existing command path; the ordinary UNIT_TILE click
    // interaction and this four-subtile gate apply to generated exits within the current act.
    if (DRLG_GetActNoFromLevelId((int32_t)destinationLevel) != g_currentAct) return 1;
    D2UnitStrc* tile = sessionWarpClickTile(warpId, exit->exitX, exit->exitY);
    return tile && D2Common_10399(g_targetPlayer, tile) <= 4;
}

// Dispatch a browser click on a same-act exit through retail's UNIT_TILE arm of packet 0x13.
// Return 2 before dispatch when the native packet's 50-subtile limit is exceeded so the shell can
// approach and retry. Return 3 for the existing cross-act or unresolved-tile transition path.
EMSCRIPTEN_KEEPALIVE int d2_session_command_warp_interact(uint32_t destinationLevel, uint32_t warpId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetClient) return 0;
    updateSessionSnapshot();

    const D2SessionWarp* exit = nullptr;
    for (const D2SessionWarp& warp : g_sessionWarps) {
        if (warp.destinationLevel == destinationLevel && warp.warpId == warpId && warp.enabled) {
            exit = &warp;
            break;
        }
    }
    if (!exit) return 0;

    if (DRLG_GetActNoFromLevelId((int32_t)destinationLevel) != g_currentAct) return 3;
    D2UnitStrc* tile = sessionWarpClickTile(warpId, exit->exitX, exit->exitY);
    if (!tile || tile->dwUnitType != UNIT_TILE || tile->dwClassId != warpId) return 3;

    const uint32_t tileGuid = (uint32_t)tile->dwUnitId;
    D2UnitStrc* resolved = SUNIT_GetServerUnit(g_game, UNIT_TILE, (int32_t)tileGuid);
    const int32_t distance = D2Common_10399(g_targetPlayer, tile);
    const int32_t oldLevel = DUNGEON_GetLevelIdFromRoom(UNITS_GetRoom(g_targetPlayer));
    bool handlerInvoked = false;
    const D2WarpInteractionDispatch interaction = d2DispatchWarpInteraction(
        resolved && resolved->dwUnitType == UNIT_TILE,
        UNIT_TILE,
        tileGuid,
        resolved ? (uint32_t)resolved->dwUnitId : UINT32_MAX,
        distance,
        [&](const uint8_t* packet, uint32_t packetLength) {
            D2WEB_ResetPacketStream();
            if (!sessionDispatchGate(0x13)) return -1;
            handlerInvoked = true;
            return D2GAME_PACKETCALLBACK_Rcv0x13_InteractWithEntity_6FC84D70(
                g_game, g_targetPlayer, const_cast<uint8_t*>(packet), packetLength);
        });

    if (interaction.step == D2WarpInteractionStep::Reject) return 0;
    if (interaction.step == D2WarpInteractionStep::TooFar) return 2;
    if (handlerInvoked) D2WEB_PacketStream(&g_sessionPacketStreamLength, &g_sessionPacketStreamPackets);
    if (!handlerInvoked || interaction.handlerResult != 0) return 0;

    if (interaction.step == D2WarpInteractionStep::Warp) {
        if (!adoptSessionLevelChange(oldLevel, (int32_t)destinationLevel)) return 0;
    } else {
        updateSessionSnapshot();
    }
    return 1;
}

EMSCRIPTEN_KEEPALIVE int d2_session_command_nearest_warp(int radius) {
    if (!g_sessionLive || !g_targetPlayer || !g_targetPlayer->pDynamicPath || radius < 0 || radius > 6) return 0;
    updateSessionSnapshot();
    const int x = g_targetPlayer->pDynamicPath->tGameCoords.wPosX;
    const int y = g_targetPlayer->pDynamicPath->tGameCoords.wPosY;
    const D2SessionWarp* best = nullptr; int bestDistance = radius + 1;
    for (const D2SessionWarp& warp : g_sessionWarps) {
        if (!warp.enabled || warp.room == UINT32_MAX) continue;
        const int distance = std::max(abs(warp.exitX - x), abs(warp.exitY - y));
        if (distance < bestDistance || (distance == bestDistance && best && warp.slot < best->slot)) {
            best = &warp; bestDistance = distance;
        }
    }
    if (!best || bestDistance > radius) return 0;
    const uint32_t destination = best->destinationLevel, warpId = best->warpId;
    return d2_session_command_warp(destination, warpId);
}

// The current level's link table as DRLG holds it after generation: Levels.txt Vis/Warp plus
// the outdoor links DRLG_SetWarpId wrote while placing the act. Eight {vis, warp} pairs; warp -1
// is a seam (continuous terrain, walked), otherwise a generated exit.
static int32_t g_sessionLevelLinks[16];
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_level_links() {
    memset(g_sessionLevelLinks, 0, sizeof g_sessionLevelLinks);
    if (!g_sessionLive || !g_lastLevel) return g_sessionLevelLinks;
    D2DrlgWarpStrc* dynamicWarps = DRLG_GetDrlgWarpFromLevelId(g_lastLevel->pDrlg, g_currentLevel);
    const D2LevelDefBin* levelDef = DATATBLS_GetLevelDefRecord(g_currentLevel);
    for (uint32_t slot = 0; slot < 8; ++slot) {
        g_sessionLevelLinks[2 * slot] = dynamicWarps ? dynamicWarps->nVis[slot] : levelDef->dwVis[slot];
        g_sessionLevelLinks[2 * slot + 1] = dynamicWarps ? dynamicWarps->nWarp[slot] : (int32_t)levelDef->dwWarp[slot];
    }
    return g_sessionLevelLinks;
}

// Why an object operate may be refused (OBJECTS_OperateHandler's own guard): {interact unit type
// (-1 none), its GUID, bBusy, cursor item GUID (0 none)}. Read-only.
static int32_t g_sessionPlayerBusy[4];
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_player_busy() {
    g_sessionPlayerBusy[0] = -1; g_sessionPlayerBusy[1] = -1; g_sessionPlayerBusy[2] = 0; g_sessionPlayerBusy[3] = 0;
    if (!g_sessionLive || !g_targetPlayer) return g_sessionPlayerBusy;
    int32_t type = -1, guid = -1;
    if (SUNIT_GetInteractInfo(g_targetPlayer, &type, &guid)) { g_sessionPlayerBusy[0] = type; g_sessionPlayerBusy[1] = guid; }
    g_sessionPlayerBusy[2] = g_targetPlayer->pPlayerData ? g_targetPlayer->pPlayerData->bBusy : 0;
    D2UnitStrc* cursor = g_targetPlayer->pInventory ? INVENTORY_GetCursorItem(g_targetPlayer->pInventory) : nullptr;
    g_sessionPlayerBusy[3] = cursor ? (int32_t)cursor->dwUnitId : 0;
    return g_sessionPlayerBusy;
}

// 1 if an ACTIVE room (one with a collision grid) covers this subtile, else 0. d2_session_walkable
// cannot tell a wall from ground whose room is not active yet; a player exploring can -- the
// automap shows where they have not been. Read-only.
EMSCRIPTEN_KEEPALIVE int d2_session_room_active_at(int x, int y) {
    if (!g_sessionLive || !g_targetPlayer || !g_targetPlayer->pDynamicPath) return 0;
    return COLLISION_GetRoomBySubTileCoordinates(g_targetPlayer->pDynamicPath->pRoom, x, y) ? 1 : 0;
}

// D24: the hero room's D2Common#10383 near list (D2Client 0x6D010's room slots, slot k = index k), 4 int32 per slot: tile x, y, w, h. Returns the slot count.
EMSCRIPTEN_KEEPALIVE int d2_session_hero_near_rooms(int32_t* out) {
    if (!g_sessionLive || !g_targetPlayer || !g_targetPlayer->pDynamicPath || !g_targetPlayer->pDynamicPath->pRoom) return 0;
    D2ActiveRoomStrc** list = nullptr; int n = 0;
    DUNGEON_GetAdjacentRoomsListFromRoom(g_targetPlayer->pDynamicPath->pRoom, &list, &n);
    int c = 0;
    for (int i = 0; i < n && i < 9; ++i, ++c) { const D2DrlgRoomStrc* r = list[i] ? list[i]->pDrlgRoom : nullptr;
        out[c * 4] = r ? r->nTileXPos : 0; out[c * 4 + 1] = r ? r->nTileYPos : 0; out[c * 4 + 2] = r ? r->nTileWidth : 0; out[c * 4 + 3] = r ? r->nTileHeight : 0; }
    return c;
}
// Resolve a ray sample from the mover's room, then resolve a collision-cross
// offset from that sample's room. PATH's pattern fallback replaces its room slot
// with the resolved sample room before asking for an out-of-grid offset.
EMSCRIPTEN_KEEPALIVE int d2_session_room_resolvable_from(int fromX, int fromY, int x, int y) {
    if (!g_sessionLive || !g_targetPlayer || !g_targetPlayer->pDynamicPath) return 0;
    D2ActiveRoomStrc* room = COLLISION_GetRoomBySubTileCoordinates(
        g_targetPlayer->pDynamicPath->pRoom, fromX, fromY);
    if (!room) return 0;
    return COLLISION_GetRoomBySubTileCoordinates(room, x, y) ? 1 : 0;
}

// d2_session_room_active_at's question asked of the level's own room list instead of the player's
// room: is some built room covering this subtile? COLLISION_GetRoomBySubTileCoordinates starts at a
// room and tests it plus its DRLG neighbours -- retail's own one-hop resolver -- so d2_session_walkable
// and d2_session_room_active_at both answer 0 for any subtile two rooms away whatever is built there.
// That is right for the path code, and useless for telling "a wall" from "not built" from "out of the
// resolver's reach", which is the distinction every walk diagnosis turns on. Read-only.
// D31: every DRLG room of the current level, 6 int32 each: tile x, tile y, tile w, tile h, active (pRoom),
// tiles built (pRoomTiles). The level's whole room list, not just the active rooms d2_session_rooms holds.
static std::vector<int32_t> g_sessionLevelRooms;
// D31: worldX, worldY of every floor-list tile record of the current level (layer 0), pairs, d2_session_tile_count's order.
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_tile_world_xy() {
    static std::vector<int32_t> v; v.clear(); updateSessionSnapshot(); ensureSessionTiles();
    for (const D2SessionTile& t : g_sessionTiles) { v.push_back(t.worldX); v.push_back(t.worldY); }
    return v.data();
}
EMSCRIPTEN_KEEPALIVE int d2_session_level_rooms(int32_t* out) {
    g_sessionLevelRooms.clear();
    if (!g_sessionLive || !g_lastLevel) return 0;
    for (D2DrlgRoomStrc* r = g_lastLevel->pFirstRoomEx; r; r = r->pDrlgRoomNext) {
        const int32_t rec[6] = { r->nTileXPos, r->nTileYPos, r->nTileWidth, r->nTileHeight, r->pRoom ? 1 : 0, r->pRoom && r->pRoom->pRoomTiles ? 1 : 0 };
        g_sessionLevelRooms.insert(g_sessionLevelRooms.end(), rec, rec + 6);
    }
    if (out) memcpy(out, g_sessionLevelRooms.data(), g_sessionLevelRooms.size() * 4);
    return (int)(g_sessionLevelRooms.size() / 6);
}
// D46: the same 6-int records for every room of the OTHER levels (the neighbour a hero near a level edge sees),
// and the world tile pairs of their seam tile records, so the room-cutoff check can cover the level border.
EMSCRIPTEN_KEEPALIVE int d2_session_other_level_rooms(int32_t* out) {
    static std::vector<int32_t> v; v.clear();
    if (!g_sessionLive || !g_lastLevel || !g_lastLevel->pDrlg) return 0;
    for (D2DrlgLevelStrc* l = g_lastLevel->pDrlg->pLevel; l; l = l->pNextLevel) {
        if (l == g_lastLevel) continue;
        for (D2DrlgRoomStrc* r = l->pFirstRoomEx; r; r = r->pDrlgRoomNext) {
            const int32_t rec[6] = { r->nTileXPos, r->nTileYPos, r->nTileWidth, r->nTileHeight, r->pRoom ? 1 : 0, r->pRoom && r->pRoom->pRoomTiles ? 1 : 0 };
            v.insert(v.end(), rec, rec + 6);
        }
    }
    if (out) memcpy(out, v.data(), v.size() * 4);
    return (int)(v.size() / 6);
}
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_seam_tile_world_xy() {
    static std::vector<int32_t> v; v.clear(); updateSessionSnapshot(); ensureSessionTiles();
    for (const D2SessionTile& t : g_sessionSeamTiles) { v.push_back(t.worldX); v.push_back(t.worldY); }
    return v.data();
}
EMSCRIPTEN_KEEPALIVE int d2_session_room_active_level(int x, int y) {
    if (!g_sessionLive || !g_lastLevel) return 0;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext)
        if (roomEx->pRoom && DungeonTestRoomGame(roomEx->pRoom, x, y)) return 1;
    return 0;
}
// The act's own collision answer for a subtile: the same test as d2_session_walkable, but the room is
// found in the act's room lists instead of by walking one hop from the player's room. This is the map
// the retail CLIENT paths on -- it holds every loaded room's collision grid and picks the target's own
// room -- where COLLISION_GetRoomBySubTileCoordinates is the one-hop predicate a single walk command's
// path search runs. A planner that only had the latter could never see past the current room.
EMSCRIPTEN_KEEPALIVE int d2_session_walkable_level(int x, int y) {
    if (!g_sessionLive || !g_targetPlayer || !g_game || g_currentAct < 0 || g_currentAct >= 5) return 0;
    D2DrlgActStrc* act = g_game->pAct[g_currentAct];
    if (!act || !act->pDrlg) return 0;
    for (D2DrlgLevelStrc* lv = act->pDrlg->pLevel; lv; lv = lv->pNextLevel) {
        for (D2DrlgRoomStrc* roomEx = lv->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
            D2ActiveRoomStrc* room = roomEx->pRoom;
            if (!room || !DungeonTestRoomGame(room, x, y)) continue;
            return COLLISION_CheckMaskWithSize(room, x, y, UNITS_GetUnitSizeX(g_targetPlayer), COLLIDE_MASK_PLAYER_PATH) ? 0 : 1;
        }
    }
    return 0;
}
// The raw collision word at a subtile in the act's own room lists (every mask), -1 when no built room
// covers it -- d2_session_collision's act-wide twin, so a probe can name what blocks a subtile
// (COLLIDE_WALL / COLLIDE_DOOR / a unit footprint) instead of only that something does. Read-only.
EMSCRIPTEN_KEEPALIVE int d2_session_collision_level(int x, int y) {
    if (!g_sessionLive || !g_game || g_currentAct < 0 || g_currentAct >= 5) return -1;
    D2DrlgActStrc* act = g_game->pAct[g_currentAct];
    if (!act || !act->pDrlg) return -1;
    for (D2DrlgLevelStrc* lv = act->pDrlg->pLevel; lv; lv = lv->pNextLevel)
        for (D2DrlgRoomStrc* roomEx = lv->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
            D2ActiveRoomStrc* room = roomEx->pRoom;
            if (!room || !DungeonTestRoomGame(room, x, y)) continue;
            return (int)COLLISION_CheckMask(room, x, y, COLLIDE_ALL_MASK);
        }
    return -1;
}
// Retained-capture query for the s354 #10573 collision comparison: explicit level, size and mask.
// Read-only twin of d2_session_collision_level (restores the export F645 s354 calls).
EMSCRIPTEN_KEEPALIVE int d2_session_collision_size_level(int levelId, int x, int y, int size, int mask) {
    if (!g_sessionLive || !g_game || g_currentAct < 0 || g_currentAct >= 5) return -1;
    D2DrlgActStrc* act = g_game->pAct[g_currentAct];
    if (!act || !act->pDrlg) return -1;
    for (D2DrlgLevelStrc* lv = act->pDrlg->pLevel; lv; lv = lv->pNextLevel) {
        if (lv->nLevelId != levelId) continue;
        for (D2DrlgRoomStrc* roomEx = lv->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
            D2ActiveRoomStrc* room = roomEx->pRoom;
            if (!room || !DungeonTestRoomGame(room, x, y)) continue;
            return (int)COLLISION_CheckMaskWithSize(room, x, y, size, (uint16_t)mask);
        }
    }
    return -1;
}
// The level-list ordinal (the same index updateSessionWorld and the snapshots use) of the room
// covering a subtile, or -1 when no built room covers it. Read-only.
EMSCRIPTEN_KEEPALIVE int d2_session_room_ordinal_at(int x, int y) {
    if (!g_sessionLive || !g_lastLevel) return -1;
    uint32_t ordinal = 0;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext, ++ordinal)
        if (roomEx->pRoom && DungeonTestRoomGame(roomEx->pRoom, x, y)) return (int)ordinal;
    return -1;
}
// The current level's DRLGACTIVATE room-status census -- what retail's activation wave has done, as
// D2Common holds it: {roomEx on the level's list, of those with a collision room built, then how many
// carry status 0 CLIENT_IN_ROOM / 1 CLIENT_IN_SIGHT / 2 CLIENT_OUT_OF_SIGHT / 3 UNTILE, then the
// act's nRoomsInitTimeout and nRoomsInitSinceLastUpdate}. The wave is owed to the player's room, so a
// census with everything at CLIENT_IN_SIGHT means the level was built up front and nothing follows the
// player. Read-only.
static int32_t g_sessionRoomStats[8];
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_room_stats() {
    memset(g_sessionRoomStats, 0, sizeof g_sessionRoomStats);
    if (!g_sessionLive || !g_lastLevel || !g_lastLevel->pDrlg) return g_sessionRoomStats;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
        ++g_sessionRoomStats[0];
        if (roomEx->pRoom) ++g_sessionRoomStats[1];
        if (roomEx->fRoomStatus < ROOMSTATUS_COUNT) ++g_sessionRoomStats[2 + roomEx->fRoomStatus];
    }
    g_sessionRoomStats[6] = g_lastLevel->pDrlg->nRoomsInitTimeout;
    g_sessionRoomStats[7] = g_lastLevel->pDrlg->nRoomsInitSinceLastUpdate;
    return g_sessionRoomStats;
}
// F67: a level's STATUS UNION -- the set of rooms carrying any of DRLGACTIVATE's four statuses, in
// TILE coordinates, for a level that is not the one the session last stood in. retail's own roomhook
// reads exactly this: `op 3` is "a room that appeared in the union of the DRLG's four per-status
// lists", not "a room that was built" (the retail capture tool/roomhook/roomhook.cpp: "the room set of a level
// IS the union of the DRLG's four status lists ... op 3 for a room that appeared"). s247 replayed
// retail's decisions and compared that union against OUR CREATE LOG -- rooms built -- which is the
// wrong side of the same event (a room in the OUT_OF_SIGHT/UNTILE rings is in the union long before
// the per-frame DRLGACTIVATE_Update builds it). `fRoomStatus < ROOMSTATUS_COUNT` is the union
// membership test d2_session_room_stats already uses, per level. Layout: {count, then count *
// {tileX, tileY, fRoomStatus}}. Read-only.
static std::vector<int32_t> g_sessionStatusCells;
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_room_status_cells(int32_t levelId) {
    g_sessionStatusCells.clear();
    if (!g_sessionLive || !g_game || g_currentAct < 0 || g_currentAct > 4 || !g_game->pAct[g_currentAct] || !g_game->pAct[g_currentAct]->pDrlg)
        return g_sessionStatusCells.data();
    D2DrlgLevelStrc* level = DRLG_GetLevel(g_game->pAct[g_currentAct]->pDrlg, levelId);
    if (!level || !level->pFirstRoomEx) { g_sessionStatusCells.push_back(0); return g_sessionStatusCells.data(); }
    g_sessionStatusCells.push_back(0);
    for (D2DrlgRoomStrc* r = level->pFirstRoomEx; r; r = r->pDrlgRoomNext)
        if (r->fRoomStatus < ROOMSTATUS_COUNT) {
            g_sessionStatusCells.push_back(r->nTileXPos);
            g_sessionStatusCells.push_back(r->nTileYPos);
            g_sessionStatusCells.push_back((int32_t)r->fRoomStatus);
        }
    g_sessionStatusCells[0] = (int32_t)((g_sessionStatusCells.size() - 1) / 3);
    return g_sessionStatusCells.data();
}
// Pointer-free trace of the MapReveal/MapHide packet stream generated by the offline client mirror.
// Layout is a separate count getter plus count * {op, level, roomTileX, roomTileY} int32 records.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_client_map_events_dropped() { return g_sessionClientMapEventsDropped; }
EMSCRIPTEN_KEEPALIVE int d2_session_client_map_event_count() { return (int)(g_sessionClientMapEvents.size() / 4); }
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_client_map_events() { return g_sessionClientMapEvents.data(); }
// Independent D2Client local-player seed state. The pair is {low, high}; its getter is read-only.
EMSCRIPTEN_KEEPALIVE int d2_session_client_player_seed_valid() { return g_sessionClientPlayerUnit ? 1 : 0; }
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_client_player_init_seed() {
    return g_sessionClientPlayerUnit ? g_sessionClientPlayerInitSeed : 0;
}
EMSCRIPTEN_KEEPALIVE const D2SeedStrc* d2_session_client_player_seed() {
    return g_sessionClientPlayerUnit ? &g_sessionClientPlayerUnit->pSeed : nullptr;
}


// 1.13c D2Client+0x89640 rand(n) on the client hero seed (+0x20/+0x24): v = lo*0x6AC690C5 + hi; a power of two masks, otherwise lo32 % n.
extern "C" uint32_t d2_session_client_rand(uint32_t n) {
    uint32_t* s = sessionClientHeroSeed();
    if (!n) return 0;
    const uint64_t v = uint64_t(s[0]) * 0x6ac690c5ull + s[1];
    s[0] = uint32_t(v); s[1] = uint32_t(v >> 32);
    return (n & (n - 1)) ? uint32_t(v) % n : ((n - 1) & uint32_t(v));
}
// The monster's client-side facts 0x237E0 reads: #10331/#10057 (room in town) and 0x881B0 (in range of the hero, client coords #10628).
// The client holds the previous frame's server state; the shell calls this before the frame's tick. Returns 0 when the unit is gone.
extern "C" int d2_session_monster_idle_facts(uint32_t id, int32_t* town, int32_t* near) {
    D2UnitStrc* u = g_game ? SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)id) : nullptr;
    if (!u || !u->pDynamicPath || !g_targetPlayer || !g_targetPlayer->pDynamicPath) return 0;
    D2ActiveRoomStrc* room = UNITS_GetRoom(u);
    *town = room && DUNGEON_IsRoomInTown(room) ? 1 : 0;
    const int64_t dx = int64_t(u->pDynamicPath->dwClientCoordX) - g_targetPlayer->pDynamicPath->dwClientCoordX;
    const int64_t dy = (int64_t(u->pDynamicPath->dwClientCoordY) - g_targetPlayer->pDynamicPath->dwClientCoordY) * 2;
    *near = dx * dx + dy * dy <= 490000 ? 1 : 0;   // 0x881B0: the float limit at D2Client+0xD7570 (490000.0f)
    return 1;
}

// PROBE (F52, lands with the measurement it justifies): where a level's exit data actually lives,
// room by room, in one dump -- the generation-time sources (the map's own DS1 preset-unit list, the
// DS1's wall/type layers) beside the activation-time ones (the room's preset units and its
// pRoomTiles warp links). Flat int32, self-describing so the probe reads it sequentially:
//   {levelId, nRooms} then per room
//   {ordinal, nType, tileX, tileY, tileW, tileH, built, nRoomsNear, nTileNodes, nRoomPresets,
//    nMapPresets, hasMap, levelPrest, pickedFile, presetFlags} then
//   nTileNodes * {destLevel, warpTxtLevelId, bEnabled, dataKind, dataPosX, dataPosY, offX, offY}
//   nRoomPresets * {nUnitType, nIndex, nXpos, nYpos, bSpawned} -- the room's OWN preset units
//      (bSpawned is the DS1's own per-unit flag; SUNIT_SpawnPresetUnitsInRoom SKIPS any unit whose
//      bit 0 is set -- `if (i->nUnitType == UNIT_MONSTER || (i->bSpawned & 1)) continue;`)
//   nMapPresets * {nUnitType, nIndex, nXpos, nYpos}          -- the DS1 map's shared preset units
// dataKind: 0 none, 1 from unk0x0C, 2 from unk0x10. Read-only.
//
// The two lists are the two halves of a spawn: SUNIT_SpawnPresetUnitsInRoom walks the ROOM's own
// pPresetUnits (via DRLGROOM_GetPresetUnits, which is a ONE-SHOT -- it sets
// DRLGROOMFLAG_PRESET_UNITS_SPAWNED and returns NULL on every later call, so a room whose list was
// still empty when it first activated can never spawn them), while the map's list is the DS1's raw
// ask. Dumping both with their unit TYPE is what tells "the DS1 never asked" from "the room's list
// was never filled" from "it was filled and the spawn still produced nothing".
//
// The last three name the room's PRESET, not its geometry: a maze level's rooms are picked from the
// level's own LvlPrest rows and the special ones (a tomb's cube room, Tal Rasha's chamber, the Arcane
// Sanctuary's summoner room) are placed by replacing or adding a preset AFTER the maze is built. A
// census that only counts spawned objects cannot tell "the special room was never placed" from "the
// room is there and its preset units did not spawn" -- these three can, which is why they are here.
// presetFlags is the raw dwFlags (DRLGPRESETROOMFLAG_HAS_MAP_DS1 is 1<<1, "needs confirmation").
// They are -1 for a room that is not DRLGTYPE_PRESET (its pMaze is really a pOutdoor).
static std::vector<int32_t> g_sessionExitProbe;
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_exit_probe(int32_t levelId, int32_t* count) {
    g_sessionExitProbe.clear();
    if (count) *count = 0;
    if (!g_sessionLive || !g_lastLevel || !g_lastLevel->pDrlg) return g_sessionExitProbe.data();
    D2DrlgLevelStrc* level = levelId < 0 ? g_lastLevel : DRLG_GetLevel(g_lastLevel->pDrlg, levelId);
    if (!level) return g_sessionExitProbe.data();
    g_sessionExitProbe.push_back(level->nLevelId);
    g_sessionExitProbe.push_back(0);
    int32_t nRooms = 0;
    for (D2DrlgRoomStrc* roomEx = level->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext, ++nRooms) {
        D2DrlgPresetRoomStrc* preset = (roomEx->nType == DRLGTYPE_PRESET) ? roomEx->pMaze : nullptr;
        D2DrlgMapStrc* map = preset ? preset->pMap : nullptr;
        int32_t nNodes = 0, nRoomPresets = 0, nMapPresets = 0;
        for (const D2RoomTileStrc* t = roomEx->pRoomTiles; t; t = t->pNext) ++nNodes;
        for (const D2PresetUnitStrc* p = roomEx->pPresetUnits; p; p = p->pNext) ++nRoomPresets;
        if (map) for (const D2PresetUnitStrc* p = map->pPresetUnit; p; p = p->pNext) ++nMapPresets;
        const int32_t head[] = { nRooms, roomEx->nType, roomEx->nTileXPos, roomEx->nTileYPos,
            roomEx->nTileWidth, roomEx->nTileHeight, roomEx->pRoom ? 1 : 0, roomEx->nRoomsNear,
            nNodes, nRoomPresets, nMapPresets, map ? 1 : 0,
            preset ? preset->nLevelPrest : -1, preset ? preset->nPickedFile : -1,
            preset ? (int32_t)preset->dwFlags : -1 };
        g_sessionExitProbe.insert(g_sessionExitProbe.end(), head, head + 15);
        for (const D2RoomTileStrc* t = roomEx->pRoomTiles; t; t = t->pNext) {
            const D2DrlgTileDataStrc* d = t->unk0x0C ? t->unk0x0C : t->unk0x10;
            const int32_t node[] = {
                (t->pDrlgRoom && t->pDrlgRoom->pLevel) ? t->pDrlgRoom->pLevel->nLevelId : -1,
                t->pLvlWarpTxtRecord ? (int32_t)t->pLvlWarpTxtRecord->dwLevelId : -1,
                t->bEnabled, t->unk0x0C ? 1 : (t->unk0x10 ? 2 : 0),
                d ? d->nPosX : -1, d ? d->nPosY : -1,
                t->pLvlWarpTxtRecord ? (int32_t)t->pLvlWarpTxtRecord->dwOffsetX : -1,
                t->pLvlWarpTxtRecord ? (int32_t)t->pLvlWarpTxtRecord->dwOffsetY : -1 };
            g_sessionExitProbe.insert(g_sessionExitProbe.end(), node, node + 8);
        }
        for (const D2PresetUnitStrc* p = roomEx->pPresetUnits; p; p = p->pNext) {
            const int32_t rec[] = { p->nUnitType, p->nIndex, p->nXpos, p->nYpos, p->bSpawned };
            g_sessionExitProbe.insert(g_sessionExitProbe.end(), rec, rec + 5);
        }
        if (map) for (const D2PresetUnitStrc* p = map->pPresetUnit; p; p = p->pNext) {
            const int32_t rec[] = { p->nUnitType, p->nIndex, p->nXpos, p->nYpos };
            g_sessionExitProbe.insert(g_sessionExitProbe.end(), rec, rec + 4);
        }
    }
    g_sessionExitProbe[1] = nRooms;
    if (count) *count = (int32_t)g_sessionExitProbe.size();
    return g_sessionExitProbe.data();
}

// PROBE (F845/F805 adjacency diff, eng-edges): a DRLG room's ppRoomsNear list, room by room, WITH the
// level id of each neighbour -- the difference between the geometric proximity list sub_6FD77BB0
// builds (every room within 6 tiles, incl. self) and the list retail's f78 capture reports is the
// warp-link path (sub_6FD77F00 appends a HAS_WARP destination room to ppRoomsNear). Reading the
// neighbour's level id tells a same-level proximity entry apart from a cross-level warp entry, which
// is the only way to name which room is the +1. Layout: {nRoomsNear} then nRoomsNear * {levelId,
// tileX, tileY}. Read-only.
static std::vector<int32_t> g_sessionNearProbe;
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_near_probe(int32_t levelId, int32_t ordinal) {
    g_sessionNearProbe.clear();
    if (!g_sessionLive || !g_lastLevel || !g_lastLevel->pDrlg) return g_sessionNearProbe.data();
    D2DrlgLevelStrc* level = levelId < 0 ? g_lastLevel : DRLG_GetLevel(g_lastLevel->pDrlg, levelId);
    if (!level) return g_sessionNearProbe.data();
    D2DrlgRoomStrc* roomEx = level->pFirstRoomEx;
    for (int32_t i = 0; roomEx && i < ordinal; ++i) roomEx = roomEx->pDrlgRoomNext;
    if (!roomEx) { g_sessionNearProbe.push_back(-1); return g_sessionNearProbe.data(); }
    g_sessionNearProbe.push_back(roomEx->nRoomsNear);
    for (int32_t i = 0; i < roomEx->nRoomsNear; ++i) {
        D2DrlgRoomStrc* n = roomEx->ppRoomsNear[i];
        g_sessionNearProbe.push_back((n && n->pLevel) ? n->pLevel->nLevelId : -1);
        g_sessionNearProbe.push_back(n ? n->nTileXPos : -1);
        g_sessionNearProbe.push_back(n ? n->nTileYPos : -1);
    }
    return g_sessionNearProbe.data();
}

// PROBE (walk barrier, eng-pocket): the mover's own START room vs the room the hero stands in.
//
// D2Common_10142 (Path.cpp:384) sets `tPathInfo.pStartRoom = pPath->pRoom` and resolves the click's
// target room as ONE DRLG hop from THAT room (`COLLISION_GetRoomBySubTileCoordinates(pStartRoom,
// target)`) -- so the mover's whole reach is one hop from the path's OWN room, which retail re-roots
// to the hero's current room as it moves (D2Common_10226 -> PATH_RecacheRoomAtCoordIfNeeded ->
// PATH_RecacheRoom). If OUR path->pRoom is left behind as the hero walks, every click is measured
// from a room he has LEFT and the pocket does not advance with him -- the barrier F825/F671 name, by
// its own mechanism instead of by a hypothesis. This reads both rooms, so "stale start room" is a
// measurement. Layout:
//   { pRoom's ordinal on g_lastLevel's list (-1 = not on it), heroX, heroY,
//     the level-list ordinal of the room covering the hero's own coords,
//     1 if GetRoomAtPosition(pRoom, hero) resolves (0 = pRoom is beyond one hop from the hero),
//     dwPathPoints, dwFlags, dwPathType }. Read-only; the type field makes the local path
// allocation/ground-move dispatch chain executable evidence instead of a prose-only inference.
// Appended words: current point index, final target x/y, scalar and vector velocity, then actual
// most-recent player path-update observations: the ground predicate evaluated/true, #10831 called,
// its second argument (0 ground, 1 target-unit), its result, and #10342 update result. The shared
// #10831 fields cover both branches; the ground-predicate fields are only set on the null-target
// branch. These values are recorded at call sites, not inferred from the path snapshot.
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_path_room_probe() {
    static D2SessionPathRoomProbe p;
    memset(&p, 0, sizeof p); p.roomOrdinal = -1; p.heroRoomOrdinal = -1; p.pathType = -1;
    if (!g_targetPlayer || !g_targetPlayer->pDynamicPath || !g_lastLevel)
        return reinterpret_cast<const int32_t*>(&p);
    D2DynamicPathStrc* path = g_targetPlayer->pDynamicPath;
    p.heroX = path->tGameCoords.wPosX; p.heroY = path->tGameCoords.wPosY;
    sessionPathRoomProbeCapturePath(p, path, d2::path::PATH_GetGroundContinuationTrace(g_targetPlayer));
    int32_t ord = 0;
    for (D2DrlgRoomStrc* r = g_lastLevel->pFirstRoomEx; r; r = r->pDrlgRoomNext, ++ord)
        if (r->pRoom && r->pRoom == path->pRoom) { p.roomOrdinal = ord; break; }
    ord = 0;
    for (D2DrlgRoomStrc* r = g_lastLevel->pFirstRoomEx; r; r = r->pDrlgRoomNext, ++ord)
        if (r->pRoom && DungeonTestRoomGame(r->pRoom, p.heroX, p.heroY)) { p.heroRoomOrdinal = ord; break; }
    p.pRoomCoversHero = COLLISION_GetRoomBySubTileCoordinates(path->pRoom, p.heroX, p.heroY) ? 1 : 0;
    return reinterpret_cast<const int32_t*>(&p);
}

// PROBE (F106's named instrument): the pending-rooms pass, which is the ONLY keeper that
// can populate a room nobody walked into. `d2_session_exit_probe` already reads a room's preset lists
// and `built`; this reads the OTHER half of the fork, so "the room was never populated" splits into
// its two causes instead of being one sentence:
//
//   * `built 0`                      -- DRLG_CreateRoomForRoomEx declined (no walls/floors in the tile
//                                       grid), so there is no active room to populate. That is
//                                       retail's own condition, not a defect.
//   * `built 1, populated 0`         -- DUNGEON_AllocRoom ran (it raises the act's
//                                       bHasPendingRoomsUpdates), and the pass that consumes that flag
//                                       never reached this room. The head carries the counters that
//                                       say whether the pass ran at all, and the live flag says
//                                       whether the work was still queued when the probe read it.
//   * `built 1, populated 1`         -- sub_6FC385A0 took its SPAWN branch and the room is empty
//                                       anyway: the fault is inside population, and `roomPresets` vs
//                                       `presetsSpawned` vs `objects` names which one.
//
// `presetsSpawned` is DRLGROOMFLAG_PRESET_UNITS_SPAWNED, which DRLGROOM_GetPresetUnits sets even when
// the room's own list is EMPTY and then refuses to hand the list over ever again -- a room whose
// spawn ran before DRLGPRESET_InitPresetRoomGrids moved the DS1's units in can never spawn from it,
// however many units the map asked for. A room with that flag set, `roomPresets 0` and the chest in
// the map's list is exactly that state, and it is invisible from the object census.
//
// Head: {levelId, nRooms, pendingFlag, pendingPassRuns, pendingPassRooms, spawnBranch, restoreBranch,
//        nRoomsInitSinceLastUpdate, nAllocatedRooms, targetRoom, lastLevel, currentLevel, currentAct}.
//        `targetRoom` is the ordinal of the level's room the PLAYER stands in, or -1. `lastLevel` is
//        the level the session's own world census (`updateSessionWorld`, and so the shell's
//        snapshot.rooms/objects) is scoped to -- see the note at the head's own construction.
// Room: {ordinal, nType, nTileXPos, nTileYPos, built, roomFlags, populated, roomPresets, mapPresets,
//        presetsSpawned, presetsAdded, tilelibLoaded, objects, monsters, tiles, nNumClients,
//        nRoomsNear}.
// F106's counters (game_glue.cpp): which branch of sub_6FC385A0 a room took, and whether the
// pending-rooms pass that offers rooms to it ran at all. Declared here because this probe is the
// first reader in the file.
extern int32_t g_d2PopulateSpawnBranch, g_d2PopulateRestoreBranch, g_d2PendingPassRuns, g_d2PendingPassRooms;
static std::vector<int32_t> g_sessionPendingProbe;
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_pending_probe(int32_t levelId, int32_t* count) {
    g_sessionPendingProbe.clear();
    if (count) *count = 0;
    if (!g_sessionLive || !g_lastLevel || !g_lastLevel->pDrlg) return g_sessionPendingProbe.data();
    D2DrlgLevelStrc* level = levelId < 0 ? g_lastLevel : DRLG_GetLevel(g_lastLevel->pDrlg, levelId);
    if (!level) return g_sessionPendingProbe.data();
    D2DrlgActStrc* pAct = level->pDrlg ? level->pDrlg->pAct : nullptr;
    D2ActiveRoomStrc* playerRoom = g_targetPlayer ? UNITS_GetRoom(g_targetPlayer) : nullptr;
    int32_t nRooms = 0, targetRoom = -1;
    for (D2DrlgRoomStrc* r = level->pFirstRoomEx; r; r = r->pDrlgRoomNext, ++nRooms) if (r->pRoom && r->pRoom == playerRoom) targetRoom = nRooms;
    const int32_t head[] = { level->nLevelId, nRooms, pAct ? (pAct->bHasPendingRoomsUpdates ? 1 : 0) : -1,
        g_d2PendingPassRuns, g_d2PendingPassRooms, g_d2PopulateSpawnBranch, g_d2PopulateRestoreBranch,
        level->pDrlg ? level->pDrlg->nRoomsInitSinceLastUpdate : -1,
        level->pDrlg ? level->pDrlg->nAllocatedRooms : -1, targetRoom,
        // WHICH level the session's own world census is scoped to. `updateSessionWorld` (the source of
        // the shell's snapshot.rooms/objects) walks `g_lastLevel->pFirstRoomEx` and skips rooms with no
        // pRoom, while this probe resolves its subject through the ACT's DRLG (`g_lastLevel->pDrlg`),
        // which answers for a level whether or not it is g_lastLevel. So a census taken here and a
        // census taken from snapshot.objects can be reading TWO DIFFERENT LEVELS, and `lastLevel` vs
        // the head's own `level` is what says so: a sweep that builds rooms in level 60 while
        // `lastLevel` is 40 moves every engine counter and leaves snapshot.objects untouched.
        g_lastLevel ? g_lastLevel->nLevelId : -1, g_currentLevel, g_currentAct };
    g_sessionPendingProbe.insert(g_sessionPendingProbe.end(), head, head + 13);
    int32_t ordinal = 0;
    for (D2DrlgRoomStrc* r = level->pFirstRoomEx; r; r = r->pDrlgRoomNext, ++ordinal) {
        D2DrlgPresetRoomStrc* preset = (r->nType == DRLGTYPE_PRESET) ? r->pMaze : nullptr;
        D2DrlgMapStrc* map = preset ? preset->pMap : nullptr;
        int32_t roomPresets = 0, mapPresets = 0;
        for (const D2PresetUnitStrc* p = r->pPresetUnits; p; p = p->pNext) ++roomPresets;
        if (map) for (const D2PresetUnitStrc* p = map->pPresetUnit; p; p = p->pNext) ++mapPresets;
        int32_t objects = 0, monsters = 0, tiles = 0;
        if (r->pRoom) for (D2UnitStrc* u = r->pRoom->pUnitFirst; u; u = u->pRoomNext) {
            if (u->dwUnitType == UNIT_OBJECT) ++objects;
            else if (u->dwUnitType == UNIT_MONSTER) ++monsters;
            else if (u->dwUnitType == UNIT_TILE) ++tiles;
        }
        const int32_t rec[] = { ordinal, r->nType, r->nTileXPos, r->nTileYPos, r->pRoom ? 1 : 0, (int32_t)r->dwFlags,
            (r->pRoom && (r->pRoom->dwFlags & 1)) ? 1 : 0, roomPresets, mapPresets,
            (r->dwFlags & DRLGROOMFLAG_PRESET_UNITS_SPAWNED) ? 1 : 0, (r->dwFlags & DRLGROOMFLAG_PRESET_UNITS_ADDED) ? 1 : 0,
            (r->dwFlags & DRLGROOMFLAG_TILELIB_LOADED) ? 1 : 0, objects, monsters, tiles,
            r->pRoom ? r->pRoom->nNumClients : -1, r->nRoomsNear };
        g_sessionPendingProbe.insert(g_sessionPendingProbe.end(), rec, rec + 17);
    }
    if (count) *count = (int32_t)g_sessionPendingProbe.size();
    return g_sessionPendingProbe.data();
}

// F654 scenario evidence: report the actual client pointer membership for each active room.
// nNumClients alone is not a client roster; this exposes whether the session's exact D2ClientStrc
// pointer is in ppClients and which player GUID each entry names. The browser can then compare
// unit.room against this verified client set without treating any nonzero count as sufficient.
static std::vector<int32_t> g_sessionRoomClientProbe;
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_room_client_probe(int32_t levelId, int32_t* count) {
    g_sessionRoomClientProbe.clear();
    if (count) *count = 0;
    if (!g_sessionLive || !g_lastLevel || !g_lastLevel->pDrlg) return g_sessionRoomClientProbe.data();
    D2DrlgLevelStrc* level = levelId < 0 ? g_lastLevel : DRLG_GetLevel(g_lastLevel->pDrlg, levelId);
    if (!level) return g_sessionRoomClientProbe.data();
    int32_t nRooms = 0;
    for (D2DrlgRoomStrc* r = level->pFirstRoomEx; r; r = r->pDrlgRoomNext) ++nRooms;
    g_sessionRoomClientProbe.push_back(nRooms);
    int32_t ordinal = 0;
    for (D2DrlgRoomStrc* r = level->pFirstRoomEx; r; r = r->pDrlgRoomNext, ++ordinal) {
        D2ActiveRoomStrc* room = r->pRoom;
        const int32_t nClients = room ? room->nNumClients : -1;
        const int32_t nEntries = room && room->ppClients && nClients > 0 ? nClients : 0;
        g_sessionRoomClientProbe.push_back(ordinal);
        g_sessionRoomClientProbe.push_back(nClients);
        g_sessionRoomClientProbe.push_back(nEntries);
        int32_t targetMatches = 0;
        if (nEntries) {
            for (int32_t i = 0; i < nEntries; ++i) {
                D2ClientStrc* client = room->ppClients[i];
                g_sessionRoomClientProbe.push_back(client ? (int32_t)client->dwClientId : -1);
                g_sessionRoomClientProbe.push_back(client && client->pPlayer ? (int32_t)client->pPlayer->dwUnitId : -1);
                if (client == g_targetClient) ++targetMatches;
            }
        }
        g_sessionRoomClientProbe.push_back(targetMatches);
        int32_t nMonsterUnits = 0;
        if (targetMatches && room) for (D2UnitStrc* unit = room->pUnitFirst; unit; unit = unit->pRoomNext)
            if (unit->dwUnitType == UNIT_MONSTER) ++nMonsterUnits;
        g_sessionRoomClientProbe.push_back(nMonsterUnits);
        if (nMonsterUnits) {
            for (D2UnitStrc* unit = room->pUnitFirst; unit; unit = unit->pRoomNext) {
                if (unit->dwUnitType != UNIT_MONSTER) continue;
                const D2MonStats2Txt* mon2 = UNITS_GetMonStats2TxtRecordFromMonsterId((int)unit->dwClassId);
                const int statLight = mon2 ? (int)mon2->nLight : 0;
                const int itemLight = (int)MONSTERS_GetMaximalLightRadius(unit);
                g_sessionRoomClientProbe.push_back((int32_t)unit->dwUnitId);
                g_sessionRoomClientProbe.push_back((int32_t)unit->dwClassId);
                g_sessionRoomClientProbe.push_back((int32_t)unit->dwAnimMode);
                const bool hasLightRecord = std::any_of(g_sessionMonsterLights.begin(), g_sessionMonsterLights.end(),
                    [unit](const D2SessionMonsterLightRecord& record) {
                        return sessionMonsterLightIdentityMatches(record.unit, record.guid, record.classId,
                            unit, unit->dwUnitId, unit->dwClassId);
                    });
                g_sessionRoomClientProbe.push_back(hasLightRecord || !sessionNativeMonsterAssignmentSuppressed(unit) ? 1 : 0);
                g_sessionRoomClientProbe.push_back(statLight > itemLight ? statLight : itemLight);
            }
        }
    }
    if (count) *count = (int32_t)g_sessionRoomClientProbe.size();
    return g_sessionRoomClientProbe.data();
}

// PROBE (lands with the arrival measurement it justifies): WHERE LEVEL_WarpUnit's placement comes
// from. DUNGEON_FindActSpawnLocationEx -> sub_6FD788D0 picks one of the destination LEVEL's own
// pTileInfo[] entries -- the level's own entrance tiles, nTileIndex = the LvlWarp id its DS1 recorded
// on that tile -- by matching the caller's nTileCalc (Levels.txt's WarpN value, which for an LoD
// level is a LvlWarp id, not the 0-13 the search's own coordinate table was written for) and falls
// back to the FIRST entry when nothing matches. This reports the entries so a probe can name the one
// an arrival came from: {levelId, nTileInfo} then per entry {nTileIndex, nPosX, nPosY} in game tiles.
// Read-only apart from DRLG_InitLevel, which the search itself runs for a level not yet generated.
static std::vector<int32_t> g_sessionLevelTileInfo;
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_level_tile_info(int32_t levelId, int32_t* count) {
    g_sessionLevelTileInfo.clear();
    if (count) *count = 0;
    if (!g_lastLevel || !g_lastLevel->pDrlg) return g_sessionLevelTileInfo.data();
    D2DrlgLevelStrc* level = DRLG_GetLevel(g_lastLevel->pDrlg, levelId);
    if (!level) return g_sessionLevelTileInfo.data();
    if (!level->pFirstRoomEx) DRLG_InitLevel(level);
    g_sessionLevelTileInfo.push_back(level->nLevelId);
    g_sessionLevelTileInfo.push_back(level->nTileInfo);
    for (int32_t i = 0; i < level->nTileInfo && i < 32; ++i) {
        const int32_t rec[] = { level->pTileInfo[i].nTileIndex, level->pTileInfo[i].nPosX, level->pTileInfo[i].nPosY };
        g_sessionLevelTileInfo.insert(g_sessionLevelTileInfo.end(), rec, rec + 3);
    }
    if (count) *count = (int32_t)g_sessionLevelTileInfo.size();
    return g_sessionLevelTileInfo.data();
}

// A level's rectangle in subtiles as the act's DRLG placed it -- what the automap shows a player
// of where the next area lies. Read-only; {x, y, width, height}, zeros if the level is not in the
// current act.
static int32_t g_sessionLevelRect[4];
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_level_rect(int32_t levelId) {
    memset(g_sessionLevelRect, 0, sizeof g_sessionLevelRect);
    if (!g_sessionLive || !g_lastLevel || !g_lastLevel->pDrlg) return g_sessionLevelRect;
    D2DrlgLevelStrc* level = DRLG_GetLevel(g_lastLevel->pDrlg, levelId);
    if (!level) return g_sessionLevelRect;
    g_sessionLevelRect[0] = level->nPosX * 5; g_sessionLevelRect[1] = level->nPosY * 5;
    g_sessionLevelRect[2] = level->nWidth * 5; g_sessionLevelRect[3] = level->nHeight * 5;
    return g_sessionLevelRect;
}

// The preset file DRLGPRESET_GenerateLevel picked for the level (pPreset->nDirection, equal to
// pDrlgMap->nPickedFile): D2Client 0x62760 receives it as the town-picture callback's second argument
// and Lut Gholein's start frame depends on it. -1 if the level is not built.
EMSCRIPTEN_KEEPALIVE int32_t d2_session_level_picked_file(int32_t levelId) {
    if (!g_sessionLive || !g_lastLevel || !g_lastLevel->pDrlg) return -1;
    D2DrlgLevelStrc* level = DRLG_GetLevel(g_lastLevel->pDrlg, levelId);
    return level && level->pPreset ? level->pPreset->nDirection : -1;
}

// Where this level's rooms share an edge with rooms of `destinationLevel` (a seam), as up to 32
// segments {x0, y0, x1, y1} in subtiles -- the DRLG's own room adjacency, what the automap shows.
// Read-only: unlike d2_session_command_seam it activates nothing. Returns the segment count.
static int32_t g_sessionSeamEdges[32 * 4];
EMSCRIPTEN_KEEPALIVE int d2_session_seam_edges(uint32_t destinationLevel) {
    int count = 0;
    if (!g_sessionLive || !g_targetPlayer) return 0;
    // The level the player is standing in -- a walk across a seam does not refresh g_lastLevel.
    D2ActiveRoomStrc* here = UNITS_GetRoom(g_targetPlayer);
    D2DrlgLevelStrc* level = here && here->pDrlgRoom ? here->pDrlgRoom->pLevel : g_lastLevel;
    if (!level) return 0;
    for (D2DrlgRoomStrc* roomEx = level->pFirstRoomEx; roomEx && count < 32; roomEx = roomEx->pDrlgRoomNext) {
        for (int32_t n = 0; n < roomEx->nRoomsNear && count < 32; ++n) {
            D2DrlgRoomStrc* near = roomEx->ppRoomsNear[n];
            if (!near || !near->pLevel || near->pLevel->nLevelId != (int32_t)destinationLevel) continue;
            int32_t dx = 0, dy = 0;
            if (near->nTileXPos == roomEx->nTileXPos + roomEx->nTileWidth) dx = 1;
            else if (near->nTileXPos + near->nTileWidth == roomEx->nTileXPos) dx = -1;
            else if (near->nTileYPos == roomEx->nTileYPos + roomEx->nTileHeight) dy = 1;
            else if (near->nTileYPos + near->nTileHeight == roomEx->nTileYPos) dy = -1;
            else continue;
            const int32_t lo = dx ? std::max(roomEx->nTileYPos, near->nTileYPos) : std::max(roomEx->nTileXPos, near->nTileXPos);
            const int32_t hi = dx ? std::min(roomEx->nTileYPos + roomEx->nTileHeight, near->nTileYPos + near->nTileHeight)
                                  : std::min(roomEx->nTileXPos + roomEx->nTileWidth, near->nTileXPos + near->nTileWidth);
            if (lo >= hi) continue;
            const int32_t line = dx > 0 ? 5 * (roomEx->nTileXPos + roomEx->nTileWidth) : dx < 0 ? 5 * roomEx->nTileXPos :
                                 dy > 0 ? 5 * (roomEx->nTileYPos + roomEx->nTileHeight) : 5 * roomEx->nTileYPos;
            int32_t* e = &g_sessionSeamEdges[4 * count++];
            e[0] = dx ? line : 5 * lo; e[1] = dx ? 5 * lo : line; e[2] = dx ? line : 5 * hi; e[3] = dx ? 5 * hi : line;
        }
    }
    return count;
}
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_seam_edge_data() { return g_sessionSeamEdges; }

// F500's counterfactual knob (d2-web diagnosis, not retail): see game_glue.cpp's g_d2CrossLevelSight
// and DrlgActivate.cpp's propagate walk. Default 1; a probe turns it off to measure what the
// cross-level rooms cost a scene's object-control stream.
extern "C" void d2_set_cross_level_sight(int on);
EMSCRIPTEN_KEEPALIVE void d2_session_crosslevel_sight(int on) { d2_set_cross_level_sight(on); }

// Levels.txt links with Warp -1 are seams: the two levels are continuous generated terrain and
// the player simply walks across (s60). The route driver needs that crossing for any adjacent
// pair, so this command finds the first passable subtile pair across the shared edge of two
// neighbouring rooms, places the player on the source side by the recovered teleport path
// (sub_6FCBDFE0, the town-portal/teleport placement) and crosses with a retail 0x01 walk; the
// per-frame client update (sub_6FC33670) then performs the level change exactly as for a walk.
EMSCRIPTEN_KEEPALIVE int d2_session_tick(int frames);
EMSCRIPTEN_KEEPALIVE int d2_session_command_object(uint32_t objectId);
static int32_t g_d2LastObjectOperationCallbackResult = -1;
EMSCRIPTEN_KEEPALIVE int32_t d2_session_last_object_callback_result() { return g_d2LastObjectOperationCallbackResult; }

static_assert(sizeof(D2Act4Quest3Strc) == 0x20);
static_assert(offsetof(D2Act4Quest3Strc, bRewardDropsPending) == 0x03);
static_assert(offsetof(D2Act4Quest3Strc, bSoulstoneSmashed) == 0x04);
static_assert(offsetof(D2Act4Quest3Strc, nGemDropTier) == 0x08);
static_assert(offsetof(D2Act4Quest3Strc, nHits) == 0x0C);
static_assert(offsetof(D2Act4Quest3Strc, nPlayersInAct) == 0x14);

EMSCRIPTEN_KEEPALIVE int d2_session_command_seam(uint32_t destinationLevel) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pDynamicPath || !g_lastLevel) return 0;
    updateSessionSnapshot();
    D2DrlgWarpStrc* dynamicWarps = DRLG_GetDrlgWarpFromLevelId(g_lastLevel->pDrlg, g_currentLevel);
    const D2LevelDefBin* levelDef = DATATBLS_GetLevelDefRecord(g_currentLevel);
    bool seam = false;
    for (uint32_t slot = 0; slot < 8 && !seam; ++slot) {
        const int32_t destination = dynamicWarps ? dynamicWarps->nVis[slot] : levelDef->dwVis[slot];
        const int32_t warpId = dynamicWarps ? dynamicWarps->nWarp[slot] : (int32_t)levelDef->dwWarp[slot];
        seam = destination == (int32_t)destinationLevel && warpId < 0;
    }
    if (!seam) return 0;
    D2DrlgActStrc* act = g_game->pAct[g_currentAct];
    const int32_t oldLevel = g_currentLevel;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
        for (int32_t n = 0; n < roomEx->nRoomsNear; ++n) {
            D2DrlgRoomStrc* near = roomEx->ppRoomsNear[n];
            if (!near || !near->pLevel || near->pLevel->nLevelId != (int32_t)destinationLevel) continue;
            // Shared edge in tile space: the neighbour touches one side of this room.
            int32_t dx = 0, dy = 0;
            if (near->nTileXPos == roomEx->nTileXPos + roomEx->nTileWidth) dx = 1;
            else if (near->nTileXPos + near->nTileWidth == roomEx->nTileXPos) dx = -1;
            else if (near->nTileYPos == roomEx->nTileYPos + roomEx->nTileHeight) dy = 1;
            else if (near->nTileYPos + near->nTileHeight == roomEx->nTileYPos) dy = -1;
            else continue;
            const int32_t lo = dx ? std::max(roomEx->nTileYPos, near->nTileYPos) : std::max(roomEx->nTileXPos, near->nTileXPos);
            const int32_t hi = dx ? std::min(roomEx->nTileYPos + roomEx->nTileHeight, near->nTileYPos + near->nTileHeight)
                                  : std::min(roomEx->nTileXPos + roomEx->nTileWidth, near->nTileXPos + near->nTileWidth);
            if (lo >= hi) continue;
            const bool builtHere = !roomEx->pRoom;
            const bool builtDest = !near->pRoom;
            if (builtHere) DUNGEON_SetClientIsInSight(act, oldLevel, roomEx->nTileXPos, roomEx->nTileYPos, nullptr);
            if (builtDest) DUNGEON_SetClientIsInSight(act, (int32_t)destinationLevel, near->nTileXPos, near->nTileYPos, nullptr);
            if (!roomEx->pRoom || !near->pRoom) continue;
            // A room built HERE has its tiles and nothing else: its preset units (a door at the seam
            // among them) and the objects it holds are spawned by the frame that follows, and their
            // collision is part of what the crossing reads -- the stand-back from a shut door, and
            // the walk's own path. The activation wave builds a room the same way
            // (DRLGACTIVATE_RoomEx_EnsureHasRoom, then the next frame's pending-rooms pass), so run
            // that frame before reading anything out of the room. Measured (s420): without it the
            // search sees no door, stands ON the seam column and the walk across is refused with no
            // path, where the eager build stands two subtiles back, opens the door as a player does
            // and crosses.
            if (builtHere || builtDest) d2_session_tick(1);
            if (!roomEx->pRoom || !near->pRoom) continue;
            const int32_t srcLine = dx > 0 ? 5 * (roomEx->nTileXPos + roomEx->nTileWidth) - 1 : dx < 0 ? 5 * roomEx->nTileXPos :
                                    dy > 0 ? 5 * (roomEx->nTileYPos + roomEx->nTileHeight) - 1 : 5 * roomEx->nTileYPos;
            const int32_t dstLine = srcLine + (dx ? dx : dy);
            for (int32_t along = 5 * lo; along < 5 * hi; ++along) {
                const int32_t srcX = dx ? srcLine : along, srcY = dx ? along : srcLine;
                const int32_t dstX = dx ? dstLine : along, dstY = dx ? along : dstLine;
                // A closed door across the seam (the monastery gates) is opened as the player would:
                // by operating the door object once it is in interact range.
                const uint16_t srcMask = COLLISION_CheckMaskWithSize(roomEx->pRoom, srcX, srcY, UNITS_GetUnitSizeX(g_targetPlayer), COLLIDE_MASK_PLAYER_PATH);
                const uint16_t dstMask = COLLISION_CheckMaskWithSize(near->pRoom, dstX, dstY, UNITS_GetUnitSizeX(g_targetPlayer), COLLIDE_MASK_PLAYER_PATH);
                if ((srcMask | dstMask) & ~COLLIDE_DOOR) continue;
                int32_t standX = srcX, standY = srcY;
                if ((srcMask | dstMask) & COLLIDE_DOOR) {
                    // Stand back from the door on the source side, as close as the player's footprint allows.
                    int32_t back = 1;
                    for (; back <= 4; ++back) {
                        standX = srcX - back * dx; standY = srcY - back * dy;
                        if (!COLLISION_CheckMaskWithSize(roomEx->pRoom, standX, standY, UNITS_GetUnitSizeX(g_targetPlayer), COLLIDE_MASK_PLAYER_PATH)) break;
                    }
                    if (back > 4) continue;
                }
                if (!sub_6FCBDFE0(g_game, g_targetPlayer, roomEx->pRoom, standX, standY, 1, 0)) continue;
                g_sessionMovePending = false;
                d2_session_tick(1);
                if ((srcMask | dstMask) & COLLIDE_DOOR) {
                    D2UnitStrc* door = nullptr; int32_t doorDistance = INT32_MAX;
                    for (D2ActiveRoomStrc* room : { roomEx->pRoom, near->pRoom }) {
                        for (D2UnitStrc* unit = room->pUnitFirst; unit; unit = unit->pRoomNext) {
                            if (unit->dwUnitType != UNIT_OBJECT || !unit->pObjectData || !unit->pObjectData->pObjectTxt ||
                                !unit->pObjectData->pObjectTxt->nIsDoor || !unit->pStaticPath) continue;
                            const int32_t distance = abs(unit->pStaticPath->tGameCoords.nX - dstX) + abs(unit->pStaticPath->tGameCoords.nY - dstY);
                            if (distance < doorDistance) { doorDistance = distance; door = unit; }
                        }
                    }
                    if (!door || !d2_session_command_object(door->dwUnitId)) continue;
                    d2_session_tick(32);
                    if (COLLISION_CheckMaskWithSize(roomEx->pRoom, srcX, srcY, UNITS_GetUnitSizeX(g_targetPlayer), COLLIDE_MASK_PLAYER_PATH) ||
                        COLLISION_CheckMaskWithSize(near->pRoom, dstX, dstY, UNITS_GetUnitSizeX(g_targetPlayer), COLLIDE_MASK_PLAYER_PATH)) continue;
                }
                if (!d2_session_command_move(dstX, dstY)) continue;
                for (int frame = 0; frame < 64 && g_sessionMovePending; ++frame) d2_session_tick(1);
                syncSessionLevelFromPlayer();
                if (g_currentLevel != (int32_t)destinationLevel) continue;
                activateSessionLevelRooms(act, g_lastLevel, g_currentLevel);
                g_targetRoom = UNITS_GetRoom(g_targetPlayer); g_targetMonster = nullptr;
                g_sessionAttackTarget = nullptr; g_sessionAttackSkill = nullptr; memset(&g_sessionAttack, 0, sizeof g_sessionAttack);
                updateSessionSnapshot(); return 1;
            }
        }
    }
    return 0;
}

EMSCRIPTEN_KEEPALIVE int d2_session_command_object(uint32_t objectId) {
    g_d2LastObjectOperationCallbackResult = -1;
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    D2UnitStrc* object = SUNIT_GetServerUnit(g_game, UNIT_OBJECT, (int32_t)objectId);
    if (!object || !object->pObjectData || !object->pObjectData->pObjectTxt ||
        !object->pObjectData->pObjectTxt->nOperateFn) return 0;
    // F158: the range check passes for units in different rooms; retail's client only sends 0x13 for an object on screen.
    if (UNITS_GetRoom(object) && UNITS_GetRoom(g_targetPlayer) &&
        DUNGEON_GetLevelIdFromRoom(UNITS_GetRoom(object)) != DUNGEON_GetLevelIdFromRoom(UNITS_GetRoom(g_targetPlayer))) return 0;
    if (!d2::monspawn::canOperateObjectNow(
            UNITS_IsObjectInInteractRange(g_targetPlayer, object) != 0,
            [&] { return UNITS_TestCollisionBetweenInteractingUnits(g_targetPlayer, object, COLLIDE_MASK_PLAYER_FLYING) != 0; })) return 0;
    // F115s: the Chaos Sanctuary seal operates (A4Q2 OperateFunction54/55/56) spawn their boss in the room at a
    // fixed offset from the seal and return 0 when D2GAME_GetRoom_6FC52070 finds no ACTIVE room there. Retail's
    // client has that room active by the time the seal is in reach; the lazy session has not built it yet, so
    // build it here the way the wave does (SetClientIsInSight, then the pending-rooms frame).
    // Which seal uses which spawner offset is Objects.txt's OperateFn column (1.13c tables, class ->
    // OperateFn: 392 -> 54, 393 -> 52, 394 -> 55, 395 -> 52, 396 -> 56), MEASURED 2026-09-29. Function
    // 54/55/56 each take the object's own coords into a different pSealCoords slot and offset it
    // (-12,-52) / (-39,+33) / (+32,+16); function 52 activates the seal and spawns nothing, so it needs
    // no room at all. Indexing the table by dwClassId - 392 (the first form of this fix) therefore built
    // the WRONG room for class 394 and a needless one for 393 -- which is why seal 394's first operate
    // was refused in the s000d chained run while 393's was not.
    if (g_lastLevel && object->dwAnimMode == 0 && object->pStaticPath &&
        (object->dwClassId == 392 || object->dwClassId == 394 || object->dwClassId == 396)) {
        static const int32_t off[3][2] = { { -12, -52 }, { -39, 33 }, { 32, 16 } };
        const int32_t slot = object->dwClassId == 392 ? 0 : (object->dwClassId == 394 ? 1 : 2);
        const int32_t tx = object->pStaticPath->tGameCoords.nX + off[slot][0];
        const int32_t ty = object->pStaticPath->tGameCoords.nY + off[slot][1];
        for (D2DrlgRoomStrc* r = g_lastLevel->pFirstRoomEx; r; r = r->pDrlgRoomNext)
            if (!r->pRoom && tx >= 5 * r->nTileXPos && tx < 5 * (r->nTileXPos + r->nTileWidth) &&
                ty >= 5 * r->nTileYPos && ty < 5 * (r->nTileYPos + r->nTileHeight)) {
                DUNGEON_SetClientIsInSight(g_game->pAct[g_currentAct], g_currentLevel, r->nTileXPos, r->nTileYPos, nullptr);
                d2_session_tick(1);
                break;
            }
    }
    const uint32_t before = object->dwAnimMode;
    const int32_t oldLevel = DUNGEON_GetLevelIdFromRoom(UNITS_GetRoom(g_targetPlayer));
    int32_t result = 0;
    const int32_t operated = OBJECTS_OperateHandler(g_game, g_targetPlayer, UNIT_OBJECT, (int32_t)objectId, &result);
    g_d2LastObjectOperationCallbackResult = operated;
    // Portal objects (Arcane Sanctuary, Canyon of the Magi, the red portals) warp the player
    // through LEVEL_WarpUnit or the teleport placement without changing their own mode, and the
    // town-portal operate function returns 0 after a successful transfer.
    int32_t newLevel = DUNGEON_GetLevelIdFromRoom(UNITS_GetRoom(g_targetPlayer));
    // s000d step 36: Tyrael's portal (OperateFunction73 -> LEVEL_WarpUnit into another act) lands the hero on a later frame
    if (newLevel == oldLevel && operated && object->pObjectData->pObjectTxt->nOperateFn == 73) { d2_session_tick(4); newLevel = DUNGEON_GetLevelIdFromRoom(UNITS_GetRoom(g_targetPlayer)); }
    if (newLevel != oldLevel) return adoptSessionLevelChange(oldLevel, newLevel);
    const int32_t modeChanged = object->dwAnimMode != before;
    // Khalim's chest callbacks and the Compelling Orb callback return zero after successful work.
    // For the chest family, the transition schedules the operating animation after its quest item
    // drop; for the Orb, only the second Will hit changes mode after consuming the Will and setting
    // the quest flags. Preserve native zero semantics for first-hit/refusal paths and every other
    // callback; accept only these exact verified class / OperateFn pairs when mode changes.
    const uint32_t classId = object->dwClassId;
    const uint32_t operateFn = object->pObjectData->pObjectTxt->nOperateFn;
    const int32_t commandAccepted = d2::monspawn::sessionObjectOperationResult(
        classId, operateFn, operated, modeChanged);
    if (!commandAccepted) return 0;
    updateSessionSnapshot();
    return commandAccepted;
}

// D15: the client's raw 0x13 {type, guid}: retail's handler walks the player to an out-of-range unit
// and operates it on arrival (d2_session_command_object only operates in range).
EMSCRIPTEN_KEEPALIVE int d2_session_command_interact_packet(uint32_t unitType, uint32_t guid) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return -1;
    uint8_t packet[9] = { 0x13 }; memcpy(packet + 1, &unitType, 4); memcpy(packet + 5, &guid, 4);
    if (!sessionDispatchGate(0x13)) return -2;
    D2WEB_ResetPacketStream();   // a retail recording: the handler's own sends (a chest's 0x0E/0x9C) are readable before the tick that follows
    const int32_t rc = D2GAME_PACKETCALLBACK_Rcv0x13_InteractWithEntity_6FC84D70(g_game, g_targetPlayer, packet, sizeof packet);
    D2WEB_PacketStream(&g_sessionPacketStreamLength, &g_sessionPacketStreamPackets);
    updateSessionSnapshot();
    return rc;
}

// F158: the value UNITS_IsObjectInInteractRange gates on -- D2Common_10399(player, object), the pure
// coordinate distance the range check reads (it returns TRUE outright when this is 0, see Units.cpp).
// s525 measures it across levels: at the object's own coordinates in another level it is 0, so the
// range gate is passed and the refusal that follows can only be the level guard. -1 when the object
// is gone; distances are never negative, so the sentinel is unambiguous.
EMSCRIPTEN_KEEPALIVE int32_t d2_session_object_distance(uint32_t objectId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return -1;
    D2UnitStrc* object = SUNIT_GetServerUnit(g_game, UNIT_OBJECT, (int32_t)objectId);
    if (!object) return -1;
    return D2Common_10399(g_targetPlayer, object);
}

// Expose the exact retail object interaction predicate for client-driven scenarios. The distance
// alone is insufficient: UNITS_IsObjectInInteractRange uses the object's footprint and the player's
// size, and a zero command return does not distinguish range rejection from a declined operation.
EMSCRIPTEN_KEEPALIVE int32_t d2_session_object_in_interact_range(uint32_t objectId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    D2UnitStrc* object = SUNIT_GetServerUnit(g_game, UNIT_OBJECT, (int32_t)objectId);
    if (!object || !object->pObjectData || !object->pObjectData->pObjectTxt ||
        !object->pObjectData->pObjectTxt->nOperateFn || !UNITS_IsObjectInInteractRange(g_targetPlayer, object)) return 0;
    if (UNITS_GetRoom(object) && UNITS_GetRoom(g_targetPlayer) &&
        DUNGEON_GetLevelIdFromRoom(UNITS_GetRoom(object)) != DUNGEON_GetLevelIdFromRoom(UNITS_GetRoom(g_targetPlayer))) return 0;
    return 1;
}

// Enumerate game-coordinate points accepted by the engine's own object interaction predicate.
// The client uses this only after its normal walk to an object has stalled, then filters the points
// through its route planner. The shared `At` predicate takes coordinates as input, so enumeration
// never changes the live path or any game state.
EMSCRIPTEN_KEEPALIVE int32_t d2_session_object_interaction_candidates(uint32_t objectId, int32_t* out, int32_t cap) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pDynamicPath || !out || cap <= 0) return 0;
    D2UnitStrc* object = SUNIT_GetServerUnit(g_game, UNIT_OBJECT, (int32_t)objectId);
    if (!object || !object->pObjectData || !object->pObjectData->pObjectTxt ||
        !object->pObjectData->pObjectTxt->nOperateFn || !UNITS_GetRoom(object)) return 0;

    D2CoordStrc objectAt = {};
    objectAt.nX = UNITS_GetXPosition(object);
    objectAt.nY = UNITS_GetYPosition(object);
    const int32_t reach = std::min(64, std::max(UNITS_GetUnitSizeX(object), UNITS_GetUnitSizeY(object)) + 4);
    int32_t count = 0;
    for (int32_t y = std::max(0, objectAt.nY - reach); y <= objectAt.nY + reach; ++y) {
        for (int32_t x = std::max(0, objectAt.nX - reach); x <= objectAt.nX + reach; ++x) {
            const D2CoordStrc candidate = { x, y };
            if (!UNITS_IsObjectInInteractRangeAt(g_targetPlayer, object, candidate.nX, candidate.nY)) continue;
            if (count + 2 > cap) {
                return -1; // never return a silently truncated candidate set
            }
            out[count++] = x;
            out[count++] = y;
        }
    }
    return count / 2;
}

static void updateSessionNpcInteraction(uint32_t npcId) {
    const uint32_t commandTarget = g_sessionNpcInteraction.commandTarget;
    const uint32_t result = g_sessionNpcInteraction.result;
    memset(&g_sessionNpcInteraction, 0, sizeof g_sessionNpcInteraction);
    g_sessionNpcInteraction.version = 1; g_sessionNpcInteraction.size = sizeof g_sessionNpcInteraction;
    g_sessionNpcInteraction.commandTarget = commandTarget; g_sessionNpcInteraction.result = result;
    if (!g_sessionLive || !g_game || !g_targetPlayer) return;
    int32_t interactType = -1, interactGuid = -1;
    if (SUNIT_GetInteractInfo(g_targetPlayer, &interactType, &interactGuid)) {
        g_sessionNpcInteraction.interactType = interactType;
        g_sessionNpcInteraction.interactGuid = interactGuid;
        g_sessionNpcInteraction.flags |= 4;
    } else {
        g_sessionNpcInteraction.interactType = -1;
        g_sessionNpcInteraction.interactGuid = -1;
    }
    D2UnitStrc* npc = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)npcId);
    if (npc) {
        g_sessionNpcInteraction.npcId = npc->dwUnitId;
        g_sessionNpcInteraction.npcClass = npc->dwClassId;
        g_sessionNpcInteraction.distance = D2Common_10399(g_targetPlayer, npc);
        D2MonsterInteractStrc* monInteract = npc->pMonsterData ? npc->pMonsterData->pMonInteract : nullptr;
        if (monInteract) {
            g_sessionNpcInteraction.flags |= 1;
            g_sessionNpcInteraction.npcInteractCount = MONSTERAI_GetInteractUnitCount(monInteract);
            g_sessionNpcInteraction.npcHasPlayer = MONSTERAI_IsInteractingWith(monInteract, g_targetPlayer);
        }
    }
    if (PLAYER_IsBusy(g_targetPlayer)) g_sessionNpcInteraction.flags |= 2;
    g_sessionNpcInteraction.hash = sessionHashBytes(D2VEC_FNV_INIT, &g_sessionNpcInteraction,
        offsetof(D2SessionNpcInteraction, hash));
}

// Replay of the capture's OWN decision stream (D2VEC kind 30 op 1 / op 0):
// the D2Common sight call retail's client made, in retail's order, at retail's points. It is the
// v5-A protocol replay for the room-activation wave -- the CALLER is the half D2Client owns and
// this tree does not, so feeding the engine retail's own decisions is what makes a comparison
// against retail's room set a measurement of the STATUS MACHINE rather than of two different walks.
// A comparison that did not replay them compared a fresh session against a walked one.
static int sessionSightPointAct(D2DrlgActStrc* act, int levelId, int tileX, int tileY, int on,
                                bool explicitHint, D2ActiveRoomStrc* hintRoom) {
    if (!act) return 0;
    D2DrlgLevelStrc* level = DRLG_GetLevel(act->pDrlg, levelId);
    if (!level) return 0;
    if (!level->pFirstRoomEx) DRLG_InitLevel(level);
    // F67: retail's client passes the HERO's own room as the sight call's room hint -- `pPlayer->pRoom`,
    // the very room a replay already has. The capture (the retail captures kind 33,
    // "What it measures: the hint is the PLAYER's own room") measured it over two
    // live runs: the calls' own points slide the lattice and the hint does not follow them, it follows
    // the hero (a level-76 call carries the hero's room IN LEVEL 75). The hint is what makes
    // DRLG_GetRoomExFromCoordinates resolve through the hint's own near-room list -- which crosses the
    // level boundary at a seam -- so a level-75 lattice point can reach the level-76 room beside it and
    // the propagation builds it; the nullptr path can only walk the sighted level's own chain. The
    // entry burst (the first calls of a load, before retail knows the hero's room) passed none, and a
    // hero with no room still passes nullptr here.
    D2ActiveRoomStrc* here = explicitHint ? hintRoom : (g_targetPlayer ? UNITS_GetRoom(g_targetPlayer) : nullptr);
    if (on) DUNGEON_SetClientIsInSight(act, levelId, tileX, tileY, here);
    else DUNGEON_UnsetClientIsInSight(act, levelId, tileX, tileY, here);
    return 1;
}
EMSCRIPTEN_KEEPALIVE int d2_session_sight_point(int levelId, int tileX, int tileY, int on) {
    if (!g_sessionLive || !g_game || g_currentAct < 0 || g_currentAct > 4) return 0;
    return sessionSightPointAct(g_game->pAct[g_currentAct], levelId, tileX, tileY, on, false, nullptr);
}
// A capture that stores only kind-30 sight points omits D2Client's fifth argument. This replay
// adapter supplies that recorded room explicitly; it never changes the player's room or substitutes
// a missing room. A negative hint level is the native null-hint case.
EMSCRIPTEN_KEEPALIVE int d2_session_sight_point_roomhint(int levelId, int tileX, int tileY, int on,
                                                          int hintLevelId, int hintTileX, int hintTileY) {
    if (!g_sessionLive || !g_game || g_currentAct < 0 || g_currentAct > 4) return 0;
    D2DrlgActStrc* act = g_game->pAct[g_currentAct];
    D2ActiveRoomStrc* hint = nullptr;
    if (hintLevelId >= 0) {
        D2DrlgLevelStrc* hintLevel = DRLG_GetLevel(act->pDrlg, hintLevelId);
        if (!hintLevel) return 0;
        // The captured room hint is the player's already-streamed room. A kind-30 replay has no
        // player walk to populate that active-room pointer, so use the native coordinate streaming
        // path before passing the same tile-origin hint to DUNGEON_*_IsInSight.
        hint = DUNGEON_StreamRoomAtCoords(act, hintTileX, hintTileY);
        if (!hint || !hint->pDrlgRoom || hint->pDrlgRoom->pLevel != hintLevel) return 0;
        // The capture's hint is the room returned by UNITS_GetRoom(player). Reproduce the native
        // room transition so client-in-room retention follows the same player-room context as the
        // subsequent sight calls; the ordinary session API still derives this from the live unit.
        if (g_targetPlayer && g_targetPlayer->pDynamicPath) {
            D2ActiveRoomStrc* previous = UNITS_GetRoom(g_targetPlayer);
            if (previous != hint) {
                DUNGEON_ChangeClientRoom(previous, hint);
                PATH_SetRoom(g_targetPlayer->pDynamicPath, hint);
            }
        }
    }
    return sessionSightPointAct(act, levelId, tileX, tileY, on, true, hint);
}

// F67: the frame replay of retail's client stream (s247). Retail's sight calls and the per-frame
// DRLGACTIVATE_Update ring run on the CLIENT's DRLG (DRLGFLAG_ONCLIENT: a 5-frame room timeout, and a
// room whose status empties is freed); the session's one DRLG is the server's (7 frames, never frees).
// The replay flags the act's DRLG as the client for its duration and drives Update alone, one call per
// client frame, so it measures the status machine and nothing of the session's simulation.
EMSCRIPTEN_KEEPALIVE int d2_session_drlg_client(int on) {
    if (!g_sessionLive || !g_game || g_currentAct < 0 || g_currentAct > 4 || !g_game->pAct[g_currentAct] || !g_game->pAct[g_currentAct]->pDrlg) return 0;
    D2DrlgStrc* d = g_game->pAct[g_currentAct]->pDrlg;
    if (on) d->dwFlags |= DRLGFLAG_ONCLIENT; else d->dwFlags &= ~(uint32_t)DRLGFLAG_ONCLIENT;
    return 1;
}
EMSCRIPTEN_KEEPALIVE int d2_session_drlg_update(int n) {
    if (!g_sessionLive || !g_game || g_currentAct < 0 || g_currentAct > 4 || !g_game->pAct[g_currentAct] || !g_game->pAct[g_currentAct]->pDrlg) return 0;
    for (int i = 0; i < n; ++i) DRLGACTIVATE_Update(g_game->pAct[g_currentAct]->pDrlg);
    return n;
}

// Recovered UNIT_MONSTER branch of PlrMsg's interaction dispatcher: the click is accepted at
// six subtiles, then D2Game owns the player/NPC interaction records and quest speech activation.
// Client packet 0x13 (interact with entity) through the recovered handler, exactly as a client
// sends it: within 6 subtiles the NPC activates immediately and its reply is queued.
// Fixture: place the player at (x,y) via the recovered teleport placement, finding the active
// room that contains (x,y). A missing destination is a refusal: passing null would make
// sub_6FCBDFE0 resolve from the player's current room, which is a different lookup and can select a
// nearby free coordinate. Return success only when the path actually lands on the requested whole
// subtile; COLLISION_GetFreeCoordinates may adjust a blocked request, and callers need to know that.
// Used to bring the player to Baal's throne so the crab-stairs form's AITHINK acquires a target.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_place_player(int32_t x, int32_t y) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pDynamicPath) return 0;
    D2ActiveRoomStrc* dest = nullptr;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel ? g_lastLevel->pFirstRoomEx : nullptr; roomEx; roomEx = roomEx->pDrlgRoomNext)
        if (roomEx->pRoom) { D2ActiveRoomStrc* r = COLLISION_GetRoomBySubTileCoordinates(roomEx->pRoom, x, y); if (r) { dest = r; break; } }
    if (!dest || !sub_6FCBDFE0(g_game, g_targetPlayer, dest, x, y, 0, 0)) return 0;
    g_sessionMovePending = false;
    updateSessionSnapshot();
    const D2DynamicPathStrc* path = g_targetPlayer->pDynamicPath;
    return path->tGameCoords.wPosX == x && path->tGameCoords.wPosY == y ? 1 : 0;
}
// Same, then the sub-subtile fraction (16.16 low bits, 0..65535): a retail capture's light source sits
// at a fractional position and the light map is read from dwPrecision (>>13), so whole subtiles are 0..7/8 off.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_place_player_frac(int32_t x, int32_t y, int32_t fx, int32_t fy) {
    if (!d2_session_fixture_place_player(x, y)) return 0;
    D2DynamicPathStrc* p = g_targetPlayer->pDynamicPath;
    p->tGameCoords.dwPrecisionX = ((uint32_t)x << 16) | (uint32_t)(fx & 0xFFFF);
    p->tGameCoords.dwPrecisionY = ((uint32_t)y << 16) | (uint32_t)(fy & 0xFFFF);
    updateSessionSnapshot();
    return 1;
}
// F158: place the hero at raw coordinates WITHOUT requiring a built room to cover them. Every other
// fixture goes through sub_6FCBDFE0, which resolves the destination room from the player's CURRENT
// level and so refuses coordinates that level does not cover -- exactly the case F158 needs (the hero
// in one level standing at the coordinate values of a unit in another; D2Common_10399 is a pure
// coordinate distance and returns 0 there). This only moves the dynamic path's position -- the hero's
// room is left as it was, which is what makes UNITS_GetRoom(player) and UNITS_GetRoom(object) disagree
// on the level and lets the guard under test be the gate that refuses. Test scaffolding: not a
// gameplay path, and no tick follows it before the command it sets up.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_place_player_raw(int32_t x, int32_t y) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pDynamicPath) return 0;
    D2DynamicPathStrc* p = g_targetPlayer->pDynamicPath;
    p->tGameCoords.dwPrecisionX = ((uint32_t)x << 16);
    p->tGameCoords.dwPrecisionY = ((uint32_t)y << 16);
    g_sessionMovePending = false;
    updateSessionSnapshot();
    return 1;
}
// Fixture: stand a monster at a subtile (a row pinning a retail capture's state -- where an NPC had
// wandered to -- before comparing what is drawn there). Same teleport as the player's fixture.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_place_monster(uint32_t unitId, int32_t x, int32_t y) {
    if (!g_sessionLive || !g_game) return 0;
    D2UnitStrc* unit = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)unitId);
    if (!unit || !unit->pDynamicPath) return 0;
    D2ActiveRoomStrc* dest = nullptr;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel ? g_lastLevel->pFirstRoomEx : nullptr; roomEx; roomEx = roomEx->pDrlgRoomNext)
        if (roomEx->pRoom) { D2ActiveRoomStrc* r = COLLISION_GetRoomBySubTileCoordinates(roomEx->pRoom, x, y); if (r) { dest = r; break; } }
    if (!sub_6FCBDFE0(g_game, unit, dest, x, y, 0, 0)) return 0;
    updateSessionSnapshot();
    return 1;
}
// Fixture (a dense-combat perf scene): spawn a monster of class `monsterId` at a subtile through
// retail's own spawn call (D2GAME_SpawnMonster, the one the Baal minions, the Cain rescue and the trapped
// souls use), in the room that covers the subtile. Returns the new unit's id, or 0 when no room covers it.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_fixture_spawn_monster(int32_t monsterId, int32_t x, int32_t y) {
    if (!g_sessionLive || !g_game) return 0;
    D2ActiveRoomStrc* dest = nullptr;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel ? g_lastLevel->pFirstRoomEx : nullptr; roomEx; roomEx = roomEx->pDrlgRoomNext)
        if (roomEx->pRoom) { D2ActiveRoomStrc* r = COLLISION_GetRoomBySubTileCoordinates(roomEx->pRoom, x, y); if (r) { dest = r; break; } }
    if (!dest) return 0;
    D2UnitStrc* unit = D2GAME_SpawnMonster_6FC69F10(g_game, dest, x, y, monsterId, MONMODE_NEUTRAL, -1, 0);
    updateSessionSnapshot();
    return unit ? unit->dwUnitId : 0;
}
EMSCRIPTEN_KEEPALIVE int d2_session_command_npc_packet(uint32_t npcId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return -1;
    D2WEB_ResetPacketStream();
    uint8_t packet[9] = { 0x13 };
    const uint32_t type = UNIT_MONSTER; memcpy(packet + 1, &type, 4); memcpy(packet + 5, &npcId, 4);
    if (!sessionDispatchGate(0x13)) return 0;
    const int32_t result = D2GAME_PACKETCALLBACK_Rcv0x13_InteractWithEntity_6FC84D70(g_game, g_targetPlayer, packet, sizeof packet);
    D2WEB_PacketStream(&g_sessionPacketStreamLength, &g_sessionPacketStreamPackets);
    updateSessionSnapshot();
    return result;
}
// Defect #16 (the unfiltered 0x13 open-object click): retail's client only sends 0x13 for an object its
// CURRENT mode makes selectable (Objects.txt Selectable<mode>, the same column the client's hover reads),
// and every such object of the shipped 1.13c tables is one whose OperateFn column has no operation
// function -- so an 0x13 that reaches OBJECTS_OperateHandler for it dies on the gpObjOperateFnTable bound
// / non-null asserts (D2_ASSERT(pOperateFn), table[0] == nullptr). Suppress the click HERE, upstream of the
// click path; the assert itself is not to be relaxed. The 1.13c oracle (D2Game+0x6A330, FUN_6fc8a330)
// separates the same two cases but returns 0 instead of aborting, so this keeps our crash out of a path
// retail answers with "no operation".
static bool objectClickIsSelectable(D2UnitStrc* object) {
    if (!object || !object->pObjectData || !object->pObjectData->pObjectTxt) return false;
    const uint32_t mode = clientObjectClock(object).shown;   // the client unit's mode, not the server's (Cairn portal: server OP, client ON)
    return mode < 8 && object->pObjectData->pObjectTxt->nSelectable[mode] != 0;
}

// Retail's 0x13 entity receiver handles object clicks, including deferred range approach.
EMSCRIPTEN_KEEPALIVE int d2_session_command_object_packet(uint32_t objectId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return -1;
    // The object branch of the receiver (sub_6FC828D0's UNIT_OBJECT case) goes straight to
    // OBJECTS_OperateHandler through sub_6FC78B20 once the target is in range, so an unfiltered click on a
    // non-selectable object aborts the game. Refuse it where the click is ISSUED, as retail's client does.
    D2UnitStrc* clickedObject = SUNIT_GetServerUnit(g_game, UNIT_OBJECT, (int32_t)objectId);
    if (!objectClickIsSelectable(clickedObject)) {
        D2WEB_ResetPacketStream();   // a refused click sends nothing: the stream the caller reads is empty, not the previous command's
        D2WEB_PacketStream(&g_sessionPacketStreamLength, &g_sessionPacketStreamPackets);
        return 0;
    }
    const int32_t oldLevel = DUNGEON_GetLevelIdFromRoom(UNITS_GetRoom(g_targetPlayer));
    D2WEB_ResetPacketStream();
    if (!sessionDispatchGate(0x13)) return 0;
    const int32_t result = d2::monspawn::dispatchObjectInteraction(objectId,
        [](const uint8_t* bytes, size_t length) {
            return D2GAME_PACKETCALLBACK_Rcv0x13_InteractWithEntity_6FC84D70(
                g_game, g_targetPlayer, const_cast<uint8_t*>(bytes), static_cast<uint32_t>(length));
        });
    // Out of interact range the receiver started the approach walk (sub_6FC828D0's UNIT_OBJECT
    // branch, player mode 3 with the object as the path target). Remember it for the tick to
    // complete; an in-range click has already operated, and clears any older pending walk.
    D2DynamicPathStrc* clickedPath = g_targetPlayer->pDynamicPath;
    g_sessionPendingObject = clickedObject && clickedPath &&
        clickedPath->dwPathPoints > clickedPath->dwCurrentPointIdx ? objectId : 0;
    if (g_sessionPendingObject) {
        g_sessionPendingObjectGoalX = clickedPath->tFinalTargetCoord.X;
        g_sessionPendingObjectGoalY = clickedPath->tFinalTargetCoord.Y;
    }
    D2WEB_PacketStream(&g_sessionPacketStreamLength, &g_sessionPacketStreamPackets);
    const int32_t newLevel = DUNGEON_GetLevelIdFromRoom(UNITS_GetRoom(g_targetPlayer));
    if (newLevel != oldLevel) adoptSessionLevelChange(oldLevel, newLevel);
    else updateSessionSnapshot();
    return result;
}
// Client packet 0x31 {npcGUID, messageId} through the recovered handler: the player picked one
// of the dialogue's messages (quest starts, intro acknowledgements).
EMSCRIPTEN_KEEPALIVE int d2_session_command_npc_response_packet(uint32_t npcId, uint32_t messageId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || messageId > UINT16_MAX) return -1;
    D2WEB_ResetPacketStream();
    uint8_t packet[9] = { 0x31 };
    const uint16_t message = (uint16_t)messageId;
    memcpy(packet + 1, &npcId, 4); memcpy(packet + 5, &message, 2);
    if (!sessionDispatchGate(0x31)) return -2;
    const int32_t result = D2GAME_PACKETCALLBACK_Rcv0x31_QuestMessage_6FC86AB0(g_game, g_targetPlayer, packet, sizeof packet);
    D2WEB_PacketStream(&g_sessionPacketStreamLength, &g_sessionPacketStreamPackets);
    updateSessionSnapshot();
    return result;
}
// The retail capture visits a town's NPCs with one bare fixture per monster class, in room
// order, leaving each previous player unit in the world: unit GUIDs and the game item seed the
// vendor stock depends on follow that sequence.  Replay it: detach the current fixture (room
// client list cleared, unit left behind) and allocate the next one beside the given class.
EMSCRIPTEN_KEEPALIVE int d2_session_reattach_near(uint32_t anchorClass) {
    if (!g_sessionLive || !g_game) return 0;
    // The fixture joined its room and the adjacent ones through the recovered client-room path;
    // leave them the same way so the next room scan sees them client-free.
    if (g_targetClient) sub_6FC33020(g_targetClient, nullptr);
    detachTargetFixture();
    g_sessionAnchorClass = anchorClass;
    const int ok = d2_aitarget_attach();
    g_sessionAnchorClass = 0xFFFFFFFF;
    updateSessionSnapshot();
    return ok;
}
// Distinct monster classes of the session level in room order -- the capture's probe order.
static std::vector<uint32_t> g_sessionTownClasses;
EMSCRIPTEN_KEEPALIVE const uint32_t* d2_session_town_classes(uint32_t* count) {
    g_sessionTownClasses.clear();
    if (g_sessionLive && g_lastLevel)
        for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
            if (!roomEx->pRoom) continue;
            for (D2UnitStrc* u = roomEx->pRoom->pUnitFirst; u; u = u->pRoomNext)
                if (u->dwUnitType == UNIT_MONSTER && std::find(g_sessionTownClasses.begin(), g_sessionTownClasses.end(), (uint32_t)u->dwClassId) == g_sessionTownClasses.end())
                    g_sessionTownClasses.push_back((uint32_t)u->dwClassId);
        }
    if (count) *count = (uint32_t)g_sessionTownClasses.size();
    return g_sessionTownClasses.data();
}
// Client packet 0x38 {action, npcGUID, -1} through the recovered handler (trade/gamble/hire).
EMSCRIPTEN_KEEPALIVE int d2_session_command_npc_action_packet(uint32_t npcId, uint32_t action) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return -1;
    D2WEB_ResetPacketStream();
    D2GSPacketClt38 packet = {};
    packet.nHeader = 0x38; packet.nAction = action; packet.nNpcGUID = (int32_t)npcId; packet.nItemGUID = -1;
    const int32_t oldLevel = DUNGEON_GetLevelIdFromRoom(UNITS_GetRoom(g_targetPlayer));
    if (!sessionDispatchGate(0x38)) return -2;
    const int32_t result = D2GAME_PACKETCALLBACK_Rcv0x38_EntityAction_6FC86CB0(g_game, g_targetPlayer, &packet, sizeof packet);
    g_sessionVendor.action = action; g_sessionVendor.npcId = npcId;   // the transaction commands key on the open action
    D2WEB_PacketStream(&g_sessionPacketStreamLength, &g_sessionPacketStreamPackets);
    // A caravan/travel action moved the player into another act's town: the session follows
    // the unit the way the exit command does (level bookkeeping, quest level-change dispatch,
    // destination room activation).
    D2ActiveRoomStrc* room = UNITS_GetRoom(g_targetPlayer);
    if (room && room->pDrlgRoom && room->pDrlgRoom->pLevel && DUNGEON_GetLevelIdFromRoom(room) != oldLevel) {
        const int32_t newLevel = DUNGEON_GetLevelIdFromRoom(room);
        // The retained 0x27 record is the bridge's presentation source for an NPC
        // conversation. A native 0x38 action that actually moved the player to a new
        // level has left that NPC context; do not carry its speech/menu into the new act.
        clearNpcDialogAfterActTravel(action, oldLevel, newLevel, g_sessionNpcDialog);
        g_currentAct = DRLG_GetActNoFromLevelId(newLevel); g_currentLevel = newLevel; g_lastLevel = room->pDrlgRoom->pLevel;
        QUESTS_ChangeLevel(g_game, oldLevel, newLevel, g_targetPlayer);
        activateSessionLevelRooms(g_game->pAct[g_currentAct], g_lastLevel, newLevel);
        g_targetRoom = room; g_targetMonster = nullptr; g_sessionMovePending = false;
        g_sessionAttackTarget = nullptr; g_sessionAttackSkill = nullptr; memset(&g_sessionAttack, 0, sizeof g_sessionAttack);
    }
    updateSessionSnapshot();
    return result;
}
// The nearest caravan NPC (Warriv, Meshif, Tyrael, Cain's Harrogath portal) within a radius,
// or 0: the browser's travel key.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_nearest_travel_npc(int radius) {
    if (!g_sessionLive || !g_targetPlayer || !g_targetPlayer->pDynamicPath) return 0;
    updateSessionSnapshot();
    static const uint32_t kTravelNpcs[] = { 155, 175, 210, 264, 367, 476 };
    const int x = g_targetPlayer->pDynamicPath->tGameCoords.wPosX, y = g_targetPlayer->pDynamicPath->tGameCoords.wPosY;
    uint32_t best = 0; int bestDistance = radius + 1;
    for (const D2SessionUnit& unit : g_sessionUnits) {
        if (unit.type != UNIT_MONSTER) continue;
        bool travel = false; for (uint32_t cls : kTravelNpcs) travel |= unit.classId == cls;
        if (!travel) continue;
        const int d = std::max(abs(unit.x - x), abs(unit.y - y));
        if (d < bestDistance || (d == bestDistance && best && unit.id < best)) { best = unit.id; bestDistance = d; }
    }
    return best;
}
// Talk to the caravan NPC and take the trip (the dialogue's travel choice).
EMSCRIPTEN_KEEPALIVE int d2_session_command_travel(uint32_t npcId) {
    if (!g_sessionLive || !g_targetPlayer) return 0;
    if (d2_session_command_npc_packet(npcId) != 0) return 0;
    const int32_t before = DUNGEON_GetLevelIdFromRoom(UNITS_GetRoom(g_targetPlayer));
    d2_session_command_npc_action_packet(npcId, 0);
    return DUNGEON_GetLevelIdFromRoom(UNITS_GetRoom(g_targetPlayer)) != before;
}
// A unit's inventory items in list order with their ITEMS_Serialize bitstreams (vendor stock).
struct D2SessionInventoryItem {
    uint32_t itemId, classId, quality, ilvl, seedLo, seedHi, bitBytes;
    uint8_t bits[80];
};
static_assert(sizeof(D2SessionInventoryItem) == 108);
static std::vector<D2SessionInventoryItem> g_sessionInventoryItems;
EMSCRIPTEN_KEEPALIVE const D2SessionInventoryItem* d2_session_inventory_items(uint32_t unitType, uint32_t unitId, uint32_t* count) {
    g_sessionInventoryItems.clear();
    D2UnitStrc* unit = (g_sessionLive && g_game) ? SUNIT_GetServerUnit(g_game, (int32_t)unitType, (int32_t)unitId) : nullptr;
    if (unit && unit->pInventory)
        for (D2UnitStrc* item = INVENTORY_GetFirstItem(unit->pInventory); item; item = INVENTORY_GetNextItem(item)) {
            if (!INVENTORY_UnitIsItem(item)) continue;
            D2SessionInventoryItem out = {};
            out.itemId = item->dwUnitId; out.classId = item->dwClassId;
            out.quality = (uint32_t)ITEMS_GetItemQuality(item); out.ilvl = (uint32_t)ITEMS_GetItemLevel(item);
            out.seedLo = item->pSeed.nLowSeed; out.seedHi = item->pSeed.nHighSeed;
            uint8_t bits[512] = {};
            out.bitBytes = (uint32_t)ITEMS_SerializeItemToBitstream(item, bits, sizeof bits, 1, 1, 0);
            memcpy(out.bits, bits, out.bitBytes < sizeof out.bits ? out.bitBytes : sizeof out.bits);
            g_sessionInventoryItems.push_back(out);
        }
    if (count) *count = (uint32_t)g_sessionInventoryItems.size();
    return g_sessionInventoryItems.data();
}
// Every item unit of the game with a GUID above minId, in GUID order (the capture walks
// pUnitList[3] the same way): what a gamble request generated.  Also reports the highest GUID.
EMSCRIPTEN_KEEPALIVE const D2SessionInventoryItem* d2_session_item_units_above(uint32_t minId, uint32_t* count, uint32_t* maxId) {
    g_sessionInventoryItems.clear();
    std::vector<D2UnitStrc*> units; uint32_t highest = 0;
    if (g_sessionLive && g_game)
        for (int b = 0; b < 128; ++b)
            for (D2UnitStrc* u = g_game->pUnitList[GAME_RemapUnitTypeToListIndex(UNIT_ITEM)][b]; u; u = u->pListNext) {
                if ((uint32_t)u->dwUnitId > highest) highest = (uint32_t)u->dwUnitId;
                if ((uint32_t)u->dwUnitId > minId) units.push_back(u);
            }
    std::sort(units.begin(), units.end(), [](D2UnitStrc* a, D2UnitStrc* b) { return (uint32_t)a->dwUnitId < (uint32_t)b->dwUnitId; });
    for (D2UnitStrc* item : units) {
        D2SessionInventoryItem out = {};
        out.itemId = item->dwUnitId; out.classId = item->dwClassId;
        out.quality = (uint32_t)ITEMS_GetItemQuality(item); out.ilvl = (uint32_t)ITEMS_GetItemLevel(item);
        out.seedLo = item->pSeed.nLowSeed; out.seedHi = item->pSeed.nHighSeed;
        uint8_t bits[512] = {};
        out.bitBytes = (uint32_t)ITEMS_SerializeItemToBitstream(item, bits, sizeof bits, 1, 1, 0);
        memcpy(out.bits, bits, out.bitBytes < sizeof out.bits ? out.bitBytes : sizeof out.bits);
        g_sessionInventoryItems.push_back(out);
    }
    if (count) *count = (uint32_t)g_sessionInventoryItems.size();
    if (maxId) *maxId = highest;
    return g_sessionInventoryItems.data();
}
// The gamble selection table as loaded: count, then the selection ids, then the 100 limits.
static std::vector<uint32_t> g_gambleTableDump;
EMSCRIPTEN_KEEPALIVE const uint32_t* d2_gamble_table(uint32_t* count) {
    g_gambleTableDump.clear();
    D2GambleDataTbl* t = DATATBLS_GetGambleDataTables();
    if (t) {
        g_gambleTableDump.push_back((uint32_t)t->nGambleTxtRecordCount);
        for (int i = 0; i < t->nGambleTxtRecordCount; ++i) g_gambleTableDump.push_back(t->pGambleSelection[i]);
        for (int i = 0; i < 100; ++i) g_gambleTableDump.push_back((uint32_t)t->pGambleChooseLimit[i]);
    }
    if (count) *count = (uint32_t)g_gambleTableDump.size();
    return g_gambleTableDump.data();
}
// Client packet 0x36 {npcGUID, mercName} through the recovered handler (hire from the 0x4E list).
EMSCRIPTEN_KEEPALIVE int d2_session_command_hire_packet(uint32_t npcId, uint32_t mercName) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return -1;
    D2WEB_ResetPacketStream();
    uint8_t packet[9] = { 0x36 };
    const uint16_t name = (uint16_t)mercName;
    memcpy(packet + 1, &npcId, 4); memcpy(packet + 5, &name, 2);
    if (!sessionDispatchGate(0x36)) return -2;
    const int32_t result = D2GAME_PACKETCALLBACK_Rcv0x36_HireMerc_6FC86BD0(g_game, g_targetPlayer, packet, sizeof packet);
    D2WEB_PacketStream(&g_sessionPacketStreamLength, &g_sessionPacketStreamPackets);
    updateSessionSnapshot();
    return result;
}
// P7: what the hire window lists for one 0x4E offer {name, seed} -- the same MONSTERS_HirelingInit the 0x36 handler
// hires with, at the hirer's level (Lvl / Life / Def / Cost and the skill line's HireDesc). {id, level, life, str, dex,
// gold, exp, defense, dmin, dmax, share, resist, hireDesc}, or null when the class lists nothing.
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_hire_offer_info(uint32_t mercName, uint32_t seed) {
    static int32_t out[13];
    if (!g_sessionLive || !g_game || !g_targetPlayer) return nullptr;
    D2HirelingInitStrc h = {};
    if (!MONSTERS_HirelingInit(g_game->bExpansion, g_targetPlayer, (int)seed, MONSTERS_GetActFromHirelingTxt(g_game->bExpansion, 0, (uint16_t)mercName), g_game->nDifficulty, &h)) return nullptr;
    const int32_t v[13] = { h.nId, h.nLevel, h.nHitpoints, h.nStrength, h.nDexterity, h.nGold, h.nExperience, h.nDefense, h.nMinDamage, h.nMaxDamage, h.nShare, h.nResist, h.nHireDesc };
    memcpy(out, v, sizeof out);
    return out;
}
// The client's Hireling Screen reads the owner's canonical PETTYPE_HIREABLE roster.  Do not infer
// ownership from nearby monster units: that includes hostile monsters, summons and other players'
// hirelings.  Each row comes from PlayerPets' linked list and resolves its GUID back to the actual
// server unit for live stats.  Dead hirelings stay in the roster, matching retail's retained pet
// record and allowing the panel to show their actual zero/current life state.
struct D2SessionHireling {
    uint32_t guid, classId, nameId, seed, flags;
    int32_t hirelingId, level, life, maxLife;
    uint32_t unitPresent;
};
static std::vector<D2SessionHireling> g_sessionHirelings;
EMSCRIPTEN_KEEPALIVE const D2SessionHireling* d2_session_hireling_roster(uint32_t* count) {
    g_sessionHirelings.clear();
    if (g_sessionLive && g_game && g_targetPlayer) {
        D2PlayerDataStrc* playerData = UNITS_GetPlayerData(g_targetPlayer);
        D2PlayerPetStrc* pets = playerData ? playerData->pPlayerPets : nullptr;
        D2PetListStrc* list = pets ? PLAYERPETS_GetPetListFromPetType(pets, PETTYPE_HIREABLE) : nullptr;
        for (D2PetDataStrc* pet = list ? list->pPetData : nullptr; pet; pet = pet->pNext) {
            D2UnitStrc* unit = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, pet->nUnitGUID);
            g_sessionHirelings.push_back({ (uint32_t)pet->nUnitGUID,
                unit ? (uint32_t)unit->dwClassId : 0u, (uint32_t)pet->petInfo.wName,
                pet->petInfo.nSeed, pet->nFlags, pet->petInfo.nHirelingId,
                unit ? STATLIST_UnitGetStatValue(unit, STAT_LEVEL, 0) : 0,
                unit ? STATLIST_UnitGetStatValue(unit, STAT_HITPOINTS, 0) : 0,
                unit ? STATLIST_UnitGetStatValue(unit, STAT_MAXHP, 0) : 0,
                unit ? 1u : 0u });
        }
    }
    if (count) *count = (uint32_t)g_sessionHirelings.size();
    return g_sessionHirelings.data();
}
// Monster units of the game above a GUID, in GUID order (the hireling a hire spawned):
// id, class, seeds, level, life, max life, strength, dexterity, experience.
struct D2SessionMonsterSummary { uint32_t id, classId, seedLo, seedHi; int32_t level, life, maxLife, str, dex, exp; };
static std::vector<D2SessionMonsterSummary> g_sessionMonsterSummaries;
EMSCRIPTEN_KEEPALIVE const D2SessionMonsterSummary* d2_session_monster_units_above(uint32_t minId, uint32_t* count, uint32_t* maxId) {
    g_sessionMonsterSummaries.clear();
    std::vector<D2UnitStrc*> units; uint32_t highest = 0;
    if (g_sessionLive && g_game)
        for (int b = 0; b < 128; ++b)
            for (D2UnitStrc* u = g_game->pUnitList[GAME_RemapUnitTypeToListIndex(UNIT_MONSTER)][b]; u; u = u->pListNext) {
                if ((uint32_t)u->dwUnitId > highest) highest = (uint32_t)u->dwUnitId;
                if ((uint32_t)u->dwUnitId > minId) units.push_back(u);
            }
    std::sort(units.begin(), units.end(), [](D2UnitStrc* a, D2UnitStrc* b) { return (uint32_t)a->dwUnitId < (uint32_t)b->dwUnitId; });
    for (D2UnitStrc* u : units)
        g_sessionMonsterSummaries.push_back({ (uint32_t)u->dwUnitId, (uint32_t)u->dwClassId, u->pSeed.nLowSeed, u->pSeed.nHighSeed,
            STATLIST_UnitGetStatValue(u, STAT_LEVEL, 0), STATLIST_UnitGetStatValue(u, STAT_HITPOINTS, 0), STATLIST_UnitGetStatValue(u, STAT_MAXHP, 0),
            STATLIST_UnitGetStatValue(u, STAT_STRENGTH, 0), STATLIST_UnitGetStatValue(u, STAT_DEXTERITY, 0), STATLIST_UnitGetStatValue(u, STAT_EXPERIENCE, 0) });
    if (count) *count = (uint32_t)g_sessionMonsterSummaries.size();
    if (maxId) *maxId = highest;
    return g_sessionMonsterSummaries.data();
}
// Per-tick monster state for the recorded-session M records: id, class, x, y, mode, life>>8 (all monsters, id order).
struct D2SessionMonsterState { uint32_t id, classId, x, y, mode, hp; };
static std::vector<D2SessionMonsterState> g_sessionMonsterStates;
EMSCRIPTEN_KEEPALIVE const D2SessionMonsterState* d2_session_monster_states(uint32_t* count) {
    g_sessionMonsterStates.clear();
    if (g_sessionLive && g_game)
        for (int b = 0; b < 128; ++b)
            for (D2UnitStrc* u = g_game->pUnitList[GAME_RemapUnitTypeToListIndex(UNIT_MONSTER)][b]; u; u = u->pListNext)
                if (u->pDynamicPath)
                    g_sessionMonsterStates.push_back({ (uint32_t)u->dwUnitId, (uint32_t)u->dwClassId, u->pDynamicPath->tGameCoords.wPosX, u->pDynamicPath->tGameCoords.wPosY,
                        (uint32_t)u->dwAnimMode, (uint32_t)STATLIST_UnitGetStatValue(u, STAT_HITPOINTS, 0) >> 8 });
    std::sort(g_sessionMonsterStates.begin(), g_sessionMonsterStates.end(), [](auto& a, auto& b) { return a.id < b.id; });
    if (count) *count = (uint32_t)g_sessionMonsterStates.size();
    return g_sessionMonsterStates.data();
}
// Fixture quest state: set one player quest flag (the state earlier play would have left).
EMSCRIPTEN_KEEPALIVE int d2_session_set_quest_flag(uint32_t questFilter, uint32_t flag) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    QUESTRECORD_SetQuestState(UNITS_GetPlayerData(g_targetPlayer)->pQuestData[g_game->nDifficulty], (int32_t)questFilter, (int32_t)flag);
    return 1;
}
// Read a quest's controller fState (main state) and a player quest-record flag. Lets a scenario
// observe a boss fight's quest consequence (e.g. A5Q6 advancing when the chamber opens).
// Fixture: put a quest into its non-intro started state (bNotIntro + fState), the state a bare
// session lacks, so a boss fight's quest consequence (e.g. A5Q6 advancing when the chamber opens)
// can be observed. Mirrors QUESTS_QuestInit's bNotIntro and QUESTS_StateDebug's fState.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_quest_start(uint32_t questId, int32_t fState) {
    if (!g_sessionLive || !g_game) return 0;
    D2QuestDataStrc* q = QUESTS_GetQuestData(g_game, (int32_t)questId);
    if (!q) return 0;
    q->bNotIntro = 1; q->fLastState = 0; QUESTS_StateDebug(q, fState, __FILE__, __LINE__);
    updateSessionSnapshot();
    return 1;
}
EMSCRIPTEN_KEEPALIVE int d2_session_quest_fstate(uint32_t questId) {
    if (!g_sessionLive || !g_game) return -1;
    D2QuestDataStrc* q = QUESTS_GetQuestData(g_game, (int32_t)questId);
    return q ? (int)q->fState : -1;
}
// Read-only A5Q2 identity/counter evidence. Version, found, state, spawned/killed/freed,
// in-range; then three 14-word groups (slot, spawned, x/y, portal GUID/spawned/updates,
// completed/exited, five POW GUIDs); finally the 15 engine-recorded freed POW GUIDs.
// These are the live quest's values, not a reconstruction from visible monsters.
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_prison_quest_evidence() {
    static int32_t out[64];
    std::fill(std::begin(out), std::end(out), 0);
    out[0] = 1;
    if (!g_sessionLive || !g_game) return out;
    D2QuestDataStrc* q = QUESTS_GetQuestData(g_game, QUEST_A5Q2_RESCUESOLDIERS);
    if (!q || !q->pQuestDataEx) return out;
    const auto* ex = static_cast<const D2Act5Quest2Strc*>(q->pQuestDataEx);
    out[1] = 1; out[2] = q->fState;
    out[3] = ex->nSpawnedWussies; out[4] = ex->nKilledWussies;
    out[5] = ex->nFreedWussies; out[6] = ex->nWussiesInRangeToDoor;
    for (int i = 0; i < 3; ++i) {
        const int at = 7 + i * 14;
        out[at] = i; out[at + 1] = ex->bWussiesSpawned[i];
        out[at + 2] = ex->pWussieCoords[i].nX; out[at + 3] = ex->pWussieCoords[i].nY;
        out[at + 4] = ex->nPortalGUIDs[i]; out[at + 5] = ex->bPortalSpawned[i];
        out[at + 6] = ex->nPortalUpdateInvocations[i];
        out[at + 7] = ex->unk0x108[i]; out[at + 8] = ex->unk0x114[i];
        for (int j = 0; j < 5; ++j) out[at + 9 + j] = ex->unk0xB0[i * 5 + j];
    }
    for (int i = 0; i < 15; ++i) out[49 + i] = ex->nFreedWussieUnitGUIDs[i];
    return out;
}
// The Tal Rasha's Tomb (Levels.txt 66-72) that holds the Horadric Staff's orifice: the DRLG's own pick, the value retail's
// quest packet 0x50 sends the client (A2Q6.cpp:1102 nStaffTombLevelOffset + LEVEL_TALRASHASTOMB1). 0 before the act is built.
EMSCRIPTEN_KEEPALIVE int d2_session_staff_tomb() {
    if (!g_sessionLive || !g_game || !g_game->pAct[1]) return 0;
    return (int)DUNGEON_GetHoradricStaffTombLevelId(g_game->pAct[1]);
}
EMSCRIPTEN_KEEPALIVE int d2_session_quest_flag(uint32_t stateFlag, uint32_t bit) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return -1;
    return QUESTRECORD_GetQuestState(UNITS_GetPlayerData(g_targetPlayer)->pQuestData[g_game->nDifficulty], (int32_t)stateFlag, (int32_t)bit) ? 1 : 0;
}
// Read-only join for Act IV's Hellforge transition: quest-extension state plus
// the forge object's own pending type-7 timer. The returned record contains no
// wasm64 pointers and is scoped to the requested class-376 unit GUID.
static D2HellforgeDiagnostic g_sessionHellforgeDiagnostic;
EMSCRIPTEN_KEEPALIVE const D2HellforgeDiagnostic* d2_session_hellforge_diagnostic(uint32_t objectGuid) {
    d2HellforgeResetDiagnostic(g_sessionHellforgeDiagnostic,
        g_sessionLive && g_game ? g_game->dwGameFrame : -1);
    if (!g_sessionLive || !g_game) return &g_sessionHellforgeDiagnostic;

    D2QuestDataStrc* quest = QUESTS_GetQuestData(g_game, QUEST_A4Q3_HELLFORGE);
    if (!quest || !quest->pQuestDataEx) return &g_sessionHellforgeDiagnostic;
    const auto* extension = static_cast<const D2Act4Quest3Strc*>(quest->pQuestDataEx);
    D2UnitStrc* forge = nullptr;
    for (int bucket = 0; bucket < 128 && !forge; ++bucket) {
        for (D2UnitStrc* unit = g_game->pUnitList[GAME_RemapUnitTypeToListIndex(UNIT_OBJECT)][bucket];
             unit; unit = unit->pListNext) {
            if (unit->dwUnitId == (int32_t)objectGuid && unit->dwClassId == OBJECT_HELLFORGE) {
                forge = unit;
                break;
            }
        }
    }
    if (!forge) return &g_sessionHellforgeDiagnostic;

    g_sessionHellforgeDiagnostic.found = 1;
    g_sessionHellforgeDiagnostic.forgeGuid = forge->dwUnitId;
    g_sessionHellforgeDiagnostic.forgeAnimMode = forge->dwAnimMode;
    g_sessionHellforgeDiagnostic.questForgeMode = extension->nHellforgeObjectMode;
    g_sessionHellforgeDiagnostic.hitCount = extension->nHits;
    g_sessionHellforgeDiagnostic.gemDropTier = extension->nGemDropTier;
    g_sessionHellforgeDiagnostic.rewardDropsPending = extension->bRewardDropsPending;
    g_sessionHellforgeDiagnostic.soulstoneSmashed = extension->bSoulstoneSmashed;
    d2HellforgeReadQuestEvents(g_sessionHellforgeDiagnostic, forge->pTimer, UNIT_OBJECT, forge);
    return &g_sessionHellforgeDiagnostic;
}
// Where the player stands: level id, act, room subtile origin.
EMSCRIPTEN_KEEPALIVE int d2_session_player_level() {
    D2ActiveRoomStrc* room = g_targetPlayer ? UNITS_GetRoom(g_targetPlayer) : nullptr;
    return room ? DUNGEON_GetLevelIdFromRoom(room) : -1;
}
EMSCRIPTEN_KEEPALIVE int d2_session_player_act() { return g_targetPlayer ? (int)g_targetPlayer->nAct : -1; }
// Fixture stat (the capture gives its bare player gold the same way).
EMSCRIPTEN_KEEPALIVE int d2_session_set_stat(uint32_t statId, int32_t value) {
    if (!g_sessionLive || !g_targetPlayer) return 0;
    STATLIST_SetUnitStat(g_targetPlayer, (int32_t)statId, value, 0);
    return 1;
}
// Load the game's global quest record (the state earlier play left; packet 0x29 echoes it).
EMSCRIPTEN_KEEPALIVE int d2_session_set_global_quest_record(const uint8_t* bytes) {
    if (!g_sessionLive || !g_game || !g_game->pQuestControl || !g_game->pQuestControl->pQuestFlags || !bytes) return 0;
    QUESTRECORD_CopyBufferToRecord(g_game->pQuestControl->pQuestFlags, const_cast<uint8_t*>(bytes), 96, 0);
    return 1;
}
// The player's quest record of the game's difficulty, as packet 0x28 carries it.
static uint8_t g_sessionQuestRecord[96];
EMSCRIPTEN_KEEPALIVE const uint8_t* d2_session_quest_record() {
    memset(g_sessionQuestRecord, 0, sizeof g_sessionQuestRecord);
    if (g_sessionLive && g_game && g_targetPlayer)
        QUESTRECORD_CopyRecordToBuffer(UNITS_GetPlayerData(g_targetPlayer)->pQuestData[g_game->nDifficulty], g_sessionQuestRecord, sizeof g_sessionQuestRecord, 0);
    return g_sessionQuestRecord;
}
// The kept join bytes' length (d2_session_join_stream), for a caller that cannot pass a pointer.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_join_stream_length() { return (uint32_t)g_sessionJoinBurst.size(); }
// What the server last told its client of the player's vitals: { life, mana, stamina } in whole points.
EMSCRIPTEN_KEEPALIVE const uint32_t* d2_session_client_sent_vitals() {
    static uint32_t out[3];
    out[0] = out[1] = out[2] = 0;
    if (g_targetClient) { out[0] = g_targetClient->clientPlayerData.nHitPoints; out[1] = g_targetClient->clientPlayerData.nManaPoints; out[2] = g_targetClient->clientPlayerData.nStaminaPoints; }
    return out;
}
EMSCRIPTEN_KEEPALIVE const uint8_t* d2_session_packet_stream() { return D2WEB_PacketStream(nullptr, nullptr); }
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_packet_stream_length() { D2WEB_PacketStream(&g_sessionPacketStreamLength, &g_sessionPacketStreamPackets); return g_sessionPacketStreamLength; }  // refreshed here: the tick (not a command) can send
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_packet_stream_packets() { return g_sessionPacketStreamPackets; }
// The next film the engine asked the client to play, or -1. D2Game's act/completion quest branches
// send opcode 0x61 {nHeader, filmIndex} (QUESTS_ActChange_HirelingChangeAct, QUESTS_HellPortalActChange);
// D2Client's receiver for it is D2Client+0xAF940, which hands the byte to the film player +0x899A0
// (src/core/d2client/movie_select.h). Drains one per call, oldest first.
EMSCRIPTEN_KEEPALIVE int d2_session_next_movie_request() {
    uint8_t index = 0;
    return D2WEB_TakeMovieRequests(&index, 1) == 1 ? (int)index : -1;
}
EMSCRIPTEN_KEEPALIVE int d2_session_player_x() { return g_targetPlayer && g_targetPlayer->pDynamicPath ? g_targetPlayer->pDynamicPath->tGameCoords.wPosX : -1; }
EMSCRIPTEN_KEEPALIVE int d2_session_player_y() { return g_targetPlayer && g_targetPlayer->pDynamicPath ? g_targetPlayer->pDynamicPath->tGameCoords.wPosY : -1; }
// The character screen's computed numbers, as retail's panel shows them (the retail captures,
// s251): out[0] defense (UNITS_GetDefense: armor class + dexterity/4 ...), out[1] attack rating
// (UNITS_GetAttackRate), out[2]/out[3] the right-hand weapon's damage with its strength and
// dexterity bonus (min/max * (100 + str*StrBonus/100 + dex*DexBonus/100) / 100), out[4]/out[5]
// stamina / max stamina, out[6..9] fire/cold/lightning/poison resistance.
// Focused harness fixture: return the active player's packed expansion/difficulty mode and
// optionally replace either field. Passing -1 leaves that field unchanged.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_game_mode(int expansion, int difficulty) {
    if (!g_sessionLive || !g_targetPlayer || !g_targetPlayer->pGame) return -1;
    if ((expansion < -1 || expansion > 1) || (difficulty < -1 || difficulty > DIFFMODE_HELL)) return -1;
    D2GameStrc* game = g_targetPlayer->pGame;
    const int previous = (game->bExpansion ? 1 : 0) | (static_cast<int>(game->nDifficulty) << 8);
    if (expansion >= 0) game->bExpansion = expansion ? TRUE : FALSE;
    if (difficulty >= 0) game->nDifficulty = static_cast<uint8_t>(difficulty);
    return previous;
}
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_resist_penalty() {
    if (!g_sessionLive || !g_targetPlayer || !g_targetPlayer->pGame) return INT32_MIN;
    D2DifficultyLevelsTxt* difficulty = DATATBLS_GetDifficultyLevelsTxtRecord(g_targetPlayer->pGame->nDifficulty);
    return difficulty ? static_cast<int32_t>(difficulty->dwResistPenalty) : INT32_MIN;
}
EMSCRIPTEN_KEEPALIVE int d2_session_character_screen(int32_t* out) {
    D2UnitStrc* p = g_targetPlayer;
    return d2CharacterScreen(p, p && p->pGame ? (p->pGame->bExpansion ? 1 : 0) : -1,
        p && p->pGame ? p->pGame->nDifficulty : 0, out);
}
// The character screen's stamina maximum is the BASE stat (retail panel: 1200 beside a current of 1560 while the
// Increased Stamina passive's 30% is in the total; the HUD bar reads the total, out[5]).
EMSCRIPTEN_KEEPALIVE int d2_session_character_stamina_base() { return g_targetPlayer ? STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_MAXSTAMINA, 0) >> 8 : 0; }
// Character panel colours: retail D2Client+0xBCEA0 draws max life / max mana (stats 7, 9) in colour 3 when the
// total (D2Common ord 10973) exceeds the base (ord 10587), colour 1 when below; this is the base read.
EMSCRIPTEN_KEEPALIVE int d2_session_player_base_stat(int stat) { return g_targetPlayer ? STATLIST_GetUnitBaseStat(g_targetPlayer, stat, 0) : -1; }
EMSCRIPTEN_KEEPALIVE int d2_session_player_gold() { return g_targetPlayer ? STATLIST_UnitGetStatValue(g_targetPlayer, STAT_GOLD, 0) : -1; }
// The stash panel's two numbers: gold in the stash (STAT_GOLDBANK) and its limit (UNITS_GetStashGoldLimit).
EMSCRIPTEN_KEEPALIVE int d2_session_stash_gold(int limit) { return !g_targetPlayer ? -1 : limit ? (int)UNITS_GetStashGoldLimit(g_targetPlayer) : STATLIST_UnitGetStatValue(g_targetPlayer, STAT_GOLDBANK, 0); }
EMSCRIPTEN_KEEPALIVE int d2_session_player_id() { return g_targetPlayer ? (int)g_targetPlayer->dwUnitId : -1; }

EMSCRIPTEN_KEEPALIVE int d2_session_command_npc(uint32_t npcId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    g_sessionNpcInteraction.commandTarget = npcId; g_sessionNpcInteraction.result = 0;
    // Native 1.13c MonsterAI +0x477A0 refuses an existing interaction without
    // sending 0x27. Keep its accepted dialog; attempted-target evidence is separate.
    int32_t previousType = -1, previousGuid = -1;
    const bool keepDialog = g_sessionNpcDialog.result &&
        SUNIT_GetInteractInfo(g_targetPlayer, &previousType, &previousGuid) &&
        previousType == UNIT_MONSTER && previousGuid == (int32_t)g_sessionNpcDialog.npcId;
    D2SessionNpcDialog dialog = {};
    D2WEB_ResetLastDialogPacket();
    dialog.version = 1; dialog.size = sizeof dialog;
    dialog.commandTarget = npcId;
    D2UnitStrc* npc = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)npcId);
    if (!npc || !npc->pMonsterData || !npc->pMonsterData->pMonInteract ||
        D2Common_10399(g_targetPlayer, npc) > 6 || PLAYER_IsBusy(g_targetPlayer)) {
        if (!keepDialog) g_sessionNpcDialog = dialog;
        updateSessionNpcInteraction(npcId); return 0;
    }
    if (PLRINTRO_GetQuestIntroFlag(g_targetPlayer, g_game, npc->dwClassId)) dialog.flags |= 2;
    if (g_game->pQuestControl) {
        for (D2QuestDataStrc* quest = g_game->pQuestControl->pLastQuest; quest; quest = quest->pPrev) {
            if (quest->nActNo == ACT_I && quest->pfCallback[QUESTEVENT_NPCACTIVATE]) dialog.flags |= 4;
            if (quest->nQuestNo == 37 && quest->pfCallback[QUESTEVENT_NPCACTIVATE]) dialog.flags |= 8;
        }
    }
    D2Common_10153(g_targetPlayer->pDynamicPath);
    sub_6FC61C70(g_game, g_targetPlayer, npc, npc->pMonsterData->pMonInteract, 0);
    int32_t type = -1, guid = -1;
    const int accepted = SUNIT_GetInteractInfo(g_targetPlayer, &type, &guid) &&
        type == UNIT_MONSTER && guid == (int32_t)npcId &&
        MONSTERAI_IsInteractingWith(npc->pMonsterData->pMonInteract, g_targetPlayer);
    g_sessionNpcInteraction.result = accepted ? 1u : 0u;
    // A new conversation has no shop until a trade is opened in it: the last vendor's stock stayed
    // listed, and the client drew Jamella's trade window over Izual's ghost in the Plains (s000d).
    // Only once the new conversation is ACCEPTED: a refused click must not empty an open shop's
    // stock (Codex review of act4-bootstrap).
    if (accepted) {
        memset(&g_sessionVendor, 0, sizeof g_sessionVendor);
        g_sessionVendorItems.clear(); g_sessionVendorPresentations.clear();
    }
    D2GSPacketSrv27 packet = {};
    dialog.result = accepted ? 1u : 0u; dialog.npcId = npcId;
    if (D2WEB_CopyLastDialogPacket(&packet)) {
        dialog.packetHeader = packet.nHeader;
        dialog.packetUnitType = packet.nUnitType;
        dialog.packetGuid = packet.dwUnitGUID;
        dialog.messageCount = std::min<uint32_t>(packet.pMessageList.nCount, 8);
        for (uint32_t i = 0; i < dialog.messageCount; ++i) {
            dialog.stringIds[i] = packet.pMessageList.pMessages[i].nStringId;
            dialog.menus[i] = packet.pMessageList.pMessages[i].nMenu;
        }
        dialog.flags |= 1;
    }
    dialog.hash = sessionHashBytes(D2VEC_FNV_INIT, &dialog,
        offsetof(D2SessionNpcDialog, hash));
    // Only a new server dialog packet replaces a still-live conversation. With no
    // live owner, publish the attempt's result so stale accepted state cannot reopen UI.
    if (!keepDialog || (dialog.flags & 1)) g_sessionNpcDialog = dialog;
    updateSessionNpcInteraction(npcId);
    return accepted;
}

// What each unit IS, one byte per unit in d2_session_units order.
//
// bit 0  talkable NPC   -- MonStats INTERACT flag and the unit has interaction data. This is the
//                          same test d2_session_command_nearest_npc applies below.
// bit 1  hostile        -- MonStats nAlign == MONALIGN_EVIL.
//
// Both matter because every town NPC is a UNIT_MONSTER in D2: Charsi and a Fallen are the same
// unit type. A client that calls every monster hostile paints Akara as an enemy AND the Rogue
// guards standing around town as enemies -- the guards are not talkable, so an INTERACT-only test
// still gets them wrong. Retail decides hostility from the monster's ALIGNMENT, which is what
// bit 1 reports.
//
// Emitted beside D2SessionUnit rather than inside it: that record's 44-byte stride is hardcoded in
// a dozen scenarios, and widening it to carry two bits would churn all of them.
// Whether each unit stands in a room the client holds: one byte per unit in d2_session_units order. A client is only
// ever told of units in its rooms (the room walk and the first-sight sends run over those), so a comparison of what
// the client knows against the server's units is over these.
EMSCRIPTEN_KEEPALIVE int d2_session_unit_client_rooms(uint8_t* out, int cap) {
    if (!g_sessionLive || !g_game || !out || cap <= 0) return 0;
    updateSessionSnapshot();
    int written = 0;
    for (const D2SessionUnit& unit : g_sessionUnits) {
        if (written >= cap) break;
        D2UnitStrc* u = SUNIT_GetServerUnit(g_game, (int32_t)unit.type, (int32_t)unit.id);
        D2ActiveRoomStrc* room = u ? UNITS_GetRoom(u) : nullptr;
        out[written++] = room && clientHeldCount(room) > 0 ? 1 : 0;
    }
    return written;
}
EMSCRIPTEN_KEEPALIVE int d2_session_unit_kinds(uint8_t* out, int cap) {
    if (!g_sessionLive || !g_game || !out || cap <= 0) return 0;
    updateSessionSnapshot();
    int written = 0;
    for (const D2SessionUnit& unit : g_sessionUnits) {
        if (written >= cap) break;
        uint8_t kind = 0;
        if (unit.type == UNIT_MONSTER) {
            D2UnitStrc* npc = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)unit.id);
            D2MonStatsTxt* stats = npc ? MONSTERMODE_GetMonStatsTxtRecord(npc->dwClassId) : nullptr;
            if (stats) {
                if (npc->pMonsterData && npc->pMonsterData->pMonInteract &&
                    (stats->dwMonStatsFlags & gdwBitMasks[MONSTATSFLAGINDEX_INTERACT])) kind |= 1u;
                if (stats->nAlign == MONALIGN_EVIL) kind |= 2u;
            }
        }
        out[written++] = kind;
    }
    return written;
}

// Whether each unit carries retail's NPC speech alert (`npcalert`, the gold "!", F22): one byte per
// unit in d2_session_units order, 1 when the alert is pending, else 0.
//
// The state is retail's client overlay 0x48, fed only by server packets. Town-NPC AI (sub_6FCCFDE0,
// within 15) calls QUESTS_ActiveCycler (1.13c D2Game+0x34EA0), which asks the current act's quest
// pfActiveFilter callbacks and sends 0x8A {1, guid} on the first true one (Wussie's AI sends one
// too); D2Client 0x6FAF7970 attaches the "!". The type-1 0x28 the server sends when an NPC
// interaction opens clears the overlay from every loaded NPC (D2Client 0x6FAF6770). Both sends are
// latched in game_net_stubs.cpp. The act filters name Akara, Cain, Tyrael, Izual, Malah, Hadriel
// and the like (Act II's intro filter is constant false; Act IV has no intro quest), never Halbu.
//
// Emitted beside D2SessionUnit for the same reason unit_kinds is: that record's 44-byte stride is
// hardcoded in a dozen scenarios.
EMSCRIPTEN_KEEPALIVE int d2_session_npc_speech_alerts(uint8_t* out, int cap) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !out || cap <= 0) return 0;
    updateSessionSnapshot();
    int written = 0;
    for (const D2SessionUnit& unit : g_sessionUnits) {
        if (written >= cap) break;
        uint8_t pending = 0;
        if (unit.type == UNIT_MONSTER) {
            // NPC "!" marker: retail draws "!" from the 0x8A the server sent (QUESTS_ActiveCycler's
            // act-quest filters), not from the NPC-intro flags; Halbu and the rest of Act 4 have none.
            pending = (uint8_t)D2WEB_NpcAlertActive((int32_t)unit.id);
        }
        out[written++] = pending;
    }
    return written;
}

// F22/F51: expose retail's scheduled AI-think frame for one session monster. Kept separate from
// D2SessionUnit so its fixed 44-byte stride remains stable. This is an observation seam for the
// captured Warriv first-think timing, not a client scheduling control.
EMSCRIPTEN_KEEPALIVE int d2_session_ai_think_frame(uint32_t unitId) {
    if (!g_sessionLive || !g_game) return -1;
    D2UnitStrc* unit = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)unitId);
    if (!unit) return -1;
    for (const D2EventTimerStrc* timer = unit->pTimer; timer; timer = timer->pNext)
        if (timer->nEventType == EVENTTYPE_AITHINK) return timer->nExpireFrame;
    return -1;
}

// F51: inspect the live unit's MapAI node list without changing the AI control or its RNG state.
// Each returned record is { action, x, y }; kept separate from the stable session-unit snapshot.
struct D2SessionMapAiNode { int32_t action, x, y; };
static_assert(sizeof(D2SessionMapAiNode) == sizeof(D2MapAIPathPositionStrc));
static std::vector<D2SessionMapAiNode> g_sessionMapAiNodes;
EMSCRIPTEN_KEEPALIVE const D2SessionMapAiNode* d2_session_ai_mapai_nodes(uint32_t unitId, int* count) {
    g_sessionMapAiNodes.clear();
    if (!g_sessionLive || !g_game) { if (count) *count = 0; return nullptr; }
    D2UnitStrc* unit = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)unitId);
    D2MapAIStrc* mapAi = unit && unit->pMonsterData && unit->pMonsterData->pAiControl
        ? unit->pMonsterData->pAiControl->pMapAi : nullptr;
    if (mapAi && mapAi->pPosition && mapAi->nPathNodes > 0 && mapAi->nPathNodes <= 256) {
        g_sessionMapAiNodes.reserve(mapAi->nPathNodes);
        for (int32_t i = 0; i < mapAi->nPathNodes; ++i) {
            const D2MapAIPathPositionStrc& node = mapAi->pPosition[i];
            g_sessionMapAiNodes.push_back({node.nMapAIAction, node.nX, node.nY});
        }
    }
    if (count) *count = (int)g_sessionMapAiNodes.size();
    return g_sessionMapAiNodes.data();
}

EMSCRIPTEN_KEEPALIVE int d2_session_ai_think_watch(uint32_t unitId) {
    if (!g_sessionLive || !g_game) return 0;
    D2UnitStrc* unit = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)unitId);
    if (!unit) return 0;
    g_sessionAiThinkWatchUnit = unitId;
    g_sessionAiThinkWatchCount = 0;
    g_sessionAiThinkWatchLastFrame = -1;
    g_sessionAiThinkWatchHasPath = false;
    return 1;
}

EMSCRIPTEN_KEEPALIVE int d2_session_ai_think_event_count(uint32_t unitId) {
    return g_sessionLive && unitId == g_sessionAiThinkWatchUnit ? (int)g_sessionAiThinkWatchCount : -1;
}

EMSCRIPTEN_KEEPALIVE int d2_session_ai_think_last_event_frame(uint32_t unitId) {
    return g_sessionLive && unitId == g_sessionAiThinkWatchUnit ? g_sessionAiThinkWatchLastFrame : -1;
}
EMSCRIPTEN_KEEPALIVE const D2SessionAiThinkPath* d2_session_ai_think_event_path(uint32_t unitId) {
    return g_sessionLive && unitId == g_sessionAiThinkWatchUnit && g_sessionAiThinkWatchHasPath ? &g_sessionAiThinkWatchPath : nullptr;
}

EMSCRIPTEN_KEEPALIVE int d2_session_ai_think_attach_frame() { return g_sessionAiThinkAttachFrame; }
EMSCRIPTEN_KEEPALIVE int d2_session_ai_think_attach_mode() { return g_sessionAiThinkAttachMode; }
EMSCRIPTEN_KEEPALIVE int d2_session_ai_think_attach_deadline() { return g_sessionAiThinkAttachDeadline; }
EMSCRIPTEN_KEEPALIVE int d2_session_ai_think_attach_room_x() { return g_sessionAiThinkAttachRoomX; }
EMSCRIPTEN_KEEPALIVE int d2_session_ai_think_attach_room_y() { return g_sessionAiThinkAttachRoomY; }
EMSCRIPTEN_KEEPALIVE int d2_session_ai_think_attach_room_has_warriv() { return g_sessionAiThinkAttachRoomHasWarriv; }
EMSCRIPTEN_KEEPALIVE int d2_session_ai_think_attach_player_x() { return g_sessionAiThinkAttachPlayerX; }
EMSCRIPTEN_KEEPALIVE int d2_session_ai_think_attach_player_y() { return g_sessionAiThinkAttachPlayerY; }

EMSCRIPTEN_KEEPALIVE int d2_session_command_nearest_npc(int radius) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || radius < 0 || radius > 6) return 0;
    updateSessionSnapshot();
    const D2SessionUnit* best = nullptr; int bestDistance = radius + 1;
    for (const D2SessionUnit& unit : g_sessionUnits) {
        if (unit.type != UNIT_MONSTER) continue;
        D2UnitStrc* npc = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)unit.id);
        if (!npc || !npc->pMonsterData || !npc->pMonsterData->pMonInteract) continue;
        D2MonStatsTxt* stats = MONSTERMODE_GetMonStatsTxtRecord(npc->dwClassId);
        if (!stats || !(stats->dwMonStatsFlags & gdwBitMasks[MONSTATSFLAGINDEX_INTERACT])) continue;
        const int distance = D2Common_10399(g_targetPlayer, npc);
        if (distance < bestDistance || (distance == bestDistance && best && unit.id < best->id)) {
            best = &unit; bestDistance = distance;
        }
    }
    return best && bestDistance <= radius ? d2_session_command_npc(best->id) : 0;
}

EMSCRIPTEN_KEEPALIVE const D2SessionNpcInteraction* d2_session_npc_interaction() {
    updateSessionNpcInteraction(g_sessionNpcInteraction.commandTarget);
    return &g_sessionNpcInteraction;
}
// Fixture: speak `messageId` from an object through retail's own QUESTS_SendScrollMessage (the call every object quest message
// makes), so a client scenario can exercise the object-speech path without walking to the quest's level.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_object_message(uint32_t objectId, uint32_t messageId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    D2UnitStrc* object = SUNIT_GetServerUnit(g_game, UNIT_OBJECT, (int32_t)objectId);
    if (!object) return 0;
    QUESTS_SendScrollMessage(g_targetPlayer, object, (uint16_t)messageId);
    return 1;
}
// F157: the next OBJECT-spoken quest message (packet 0x27, unit type object), once: out[0] = the object's guid, out[1] = the
// string id. Returns 1 and consumes it, or 0 when none is waiting.
EMSCRIPTEN_KEEPALIVE int d2_session_take_object_message(int32_t* out) {
    uint32_t sid = 0;
    if (!g_sessionLive || !out || !D2WEB_TakeObjectMessage(&out[0], &sid)) return 0;
    out[1] = (int32_t)sid; return 1;
}
EMSCRIPTEN_KEEPALIVE const D2SessionNpcDialog* d2_session_npc_dialog() {
    return &g_sessionNpcDialog;
}
EMSCRIPTEN_KEEPALIVE int d2_session_command_npc_response(uint32_t npcId, uint32_t messageId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || messageId > UINT16_MAX) return 0;
    memset(&g_sessionNpcResponse, 0, sizeof g_sessionNpcResponse);
    g_sessionNpcResponse.version = 1; g_sessionNpcResponse.size = sizeof g_sessionNpcResponse;
    g_sessionNpcResponse.packetHeader = 0x31; g_sessionNpcResponse.packetSize = 9;
    g_sessionNpcResponse.npcId = npcId; g_sessionNpcResponse.messageId = messageId;
    D2UnitStrc* npc = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)npcId);
    int32_t interactType = -1, interactGuid = -1;
    const bool active = npc && SUNIT_GetInteractInfo(g_targetPlayer, &interactType, &interactGuid) &&
        interactType == UNIT_MONSTER && interactGuid == (int32_t)npcId;
    bool offered = false;
    for (uint32_t i = 0; i < g_sessionNpcDialog.messageCount && i < 8; ++i)
        if (g_sessionNpcDialog.stringIds[i] == messageId) offered = true;
    if (!active || !offered) return 0;
    g_sessionNpcResponse.introBefore = PLRINTRO_GetQuestIntroFlag(g_targetPlayer, g_game, npc->dwClassId);
    uint8_t packet[9] = { 0x31 };
    const int32_t guid = (int32_t)npcId; const uint16_t message = (uint16_t)messageId;
    memcpy(packet + 1, &guid, sizeof guid); memcpy(packet + 5, &message, sizeof message);
    if (!sessionDispatchGate(0x31)) return -2;
    const int32_t handler = D2GAME_PACKETCALLBACK_Rcv0x31_QuestMessage_6FC86AB0(g_game, g_targetPlayer, packet, sizeof packet);
    g_sessionNpcResponse.introAfter = PLRINTRO_GetQuestIntroFlag(g_targetPlayer, g_game, npc->dwClassId);
    SUNIT_GetInteractInfo(g_targetPlayer, &g_sessionNpcResponse.interactType, &g_sessionNpcResponse.interactGuid);
    g_sessionNpcResponse.result = handler == 0 ? 1u : 0u;
    if (active) g_sessionNpcResponse.flags |= 1;
    if (offered) g_sessionNpcResponse.flags |= 2;
    if (!g_sessionNpcResponse.introBefore && g_sessionNpcResponse.introAfter) g_sessionNpcResponse.flags |= 4;
    g_sessionNpcResponse.hash = sessionHashBytes(D2VEC_FNV_INIT, &g_sessionNpcResponse,
        offsetof(D2SessionNpcResponse, hash));
    return g_sessionNpcResponse.result;
}
extern "C" uint32_t D2WEB_TakeUiEvents(uint8_t* out, uint32_t cap);
extern "C" int D2WEB_TakeWaypointMenu(D2GSPacketSrv63* pPacket);
// The waypoint menu packet (0x63) the server sent since the last call: 1 and the waypoint unit id
// in *pWaypointId when one arrived, else 0. The ACTIVATED set is read below from the player's own
// waypoint data, which is what the packet carries.
EMSCRIPTEN_KEEPALIVE int d2_session_take_waypoint_menu(uint32_t* pWaypointId) {
    D2GSPacketSrv63 packet = {};
    if (!D2WEB_TakeWaypointMenu(&packet)) return 0;
    if (pWaypointId) *pWaypointId = packet.dwUnitId;
    return 1;
}
// Every waypoint retail knows, in waypoint-number order, as {waypointNo, levelId, act, activated}
// int32 quads, activation from the player's waypoint data for the current difficulty. Returns the
// number of quads.
EMSCRIPTEN_KEEPALIVE int d2_session_waypoints(int32_t* out, int cap) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !out) return 0;
    D2PlayerDataStrc* data = UNITS_GetPlayerData(g_targetPlayer);
    D2WaypointDataStrc* flags = data ? data->pWaypointData[g_game->nDifficulty] : nullptr;
    int n = 0;
    for (int wp = 0; wp < 64 && n + 4 <= cap; ++wp) {
        int levelId = 0;
        if (!WAYPOINTS_GetLevelIdFromWaypointNo((short)wp, &levelId) || levelId <= 0) continue;
        out[n++] = wp; out[n++] = levelId; out[n++] = DRLG_GetActNoFromLevelId(levelId);
        out[n++] = flags ? (WAYPOINTS_IsActivated(flags, (uint16_t)wp) ? 1 : 0) : 0;
    }
    return n / 4;
}
// Levels.txt's LevelName for a level: the string-table KEY, resolved by the client like any other.
// Missiles.txt CelFile of a missile class: the DCC (data\\global\\missiles\\<cel>.dcc) the client draws it with.
EMSCRIPTEN_KEEPALIVE const char* d2_missile_celfile(int classId) {
    D2MissilesTxt* rec = DATATBLS_GetMissilesTxtRecord(classId);
    return rec ? rec->szCelFile : "";
}
// MonStats NameStr of a monster class: the string index the client names an NPC by (its menu header).
EMSCRIPTEN_KEEPALIVE int d2_monster_name_str(int classId) {
    D2MonStatsTxt* rec = DATATBLS_GetMonStatsTxtRecord(classId);
    return rec ? rec->wNameStr : -1;
}
// The type line retail draws under a hovered monster's name (f7-ui2-hover-monster: Fallen -> "Demon"): the
// string-table entries 10078 "Demon" / 10077 "Undead", picked by MonStats' Demon / lUndead / hUndead
// flags. -1 = no line. (MonStats DescStr and MonType.txt StrSing are both unset for these classes.)
EMSCRIPTEN_KEEPALIVE int d2_monster_type_str(int classId) {
    D2MonStatsTxt* rec = DATATBLS_GetMonStatsTxtRecord(classId);
    if (!rec) return -1;
    if (rec->dwMonStatsFlags & MONSTATSFLAG_DEMON) return 10078;
    if (rec->dwMonStatsFlags & (MONSTATSFLAG_LUNDEAD | MONSTATSFLAG_HUNDEAD)) return 10077;
    return -1;
}
// Fixture (s2151): clear an item's Identified flag, as a dropped magic item arrives -- so a row can stand
// Cain's menu on a real unidentified item (retail's capture did the same by editing a save copy).
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_unidentify(uint32_t itemId) {
    D2UnitStrc* item = (g_sessionLive && g_game) ? SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId) : nullptr;
    if (!item) return 0;
    ITEMS_SetItemFlag(item, IFLAG_IDENTIFIED, 0);
    return 1;
}
// Ground items in the order a client label pass meets them: the player's room and its adjacent rooms, each room's unit chain.
// Item `which` of that walk: field 0 its unit id, 1 its room index; 0xFFFFFFFF past the end.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_ground_item_order(int which, int field) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0xFFFFFFFFu;
    D2ActiveRoomStrc** rooms = nullptr; int32_t count = 0; int n = 0;
    DUNGEON_GetAdjacentRoomsListFromRoom(UNITS_GetRoom(g_targetPlayer), &rooms, &count);
    for (int32_t i = 0; rooms && i < count; ++i)
        for (D2UnitStrc* u = rooms[i] ? rooms[i]->pUnitFirst : nullptr; u; u = u->pRoomNext)
            if (u->dwUnitType == UNIT_ITEM && n++ == which) return field ? (uint32_t)i : u->dwUnitId;
    return 0xFFFFFFFFu;
}
// Fixture: set or clear one item flag (ethereal 0x400000, socketed 0x800) on a specific item, for rows that need an item the
// loot tables did not roll that way.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_item_flag(uint32_t itemId, uint32_t flag, int on) {
    D2UnitStrc* item = (g_sessionLive && g_game) ? SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId) : nullptr;
    if (!item || !item->pItemData) return 0;
    ITEMS_SetItemFlag(item, flag, on ? 1 : 0);
    updateSessionSnapshot();
    return 1;
}
// Fixture: give an item the name ids a recorded item packet carries (0x9C's quality block): unique or set row (a), rare
// prefix and suffix (a, b), magic prefix and suffix (a, b). A row that replays retail's items needs the same names, and the
// game's own rolls draw them from a seed that differs from retail's. Properties stay as rolled.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_item_names(uint32_t itemId, uint32_t a, uint32_t b) {
    D2UnitStrc* item = (g_sessionLive && g_game) ? SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId) : nullptr;
    if (!item || !item->pItemData) return 0;
    switch (item->pItemData->dwQualityNo) {
        case ITEMQUAL_UNIQUE: case ITEMQUAL_SET: ITEMS_SetFileIndex(item, a); break;
        case ITEMQUAL_RARE: case ITEMQUAL_CRAFT: ITEMS_AssignRarePrefix(item, (uint16_t)a); ITEMS_AssignRareSuffix(item, (uint16_t)b); break;
        case ITEMQUAL_MAGIC: {
            // The packet's prefix is a MagicPrefix row; ours indexes the combined affix table (the serializer's own offset).
            const D2MagicAffixDataTbl& tb = sgptDataTables->pMagicAffixDataTables;
            ITEMS_AssignPrefix(item, (uint16_t)(a ? a + (uint32_t)(tb.pMagicPrefix - tb.pMagicAffixTxt) : 0), 0);
            ITEMS_AssignSuffix(item, (uint16_t)b, 0);
            break;
        }
        default: return 0;
    }
    updateSessionSnapshot();
    return 1;
}
// How many items the hero carries that Cain would identify (ITEMS_GetNoOfUnidItems, the very count
// D2GAME_NPC_IdentifyAllItems tests): the client lists "Identify Items" in his menu only when it is > 0.
EMSCRIPTEN_KEEPALIVE int d2_session_unidentified_count() {
    return (g_sessionLive && g_game && g_targetPlayer) ? ITEMS_GetNoOfUnidItems(g_targetPlayer) : 0;
}
EMSCRIPTEN_KEEPALIVE const char* d2_level_name(int levelId) {
    D2LevelsTxt* rec = DATATBLS_GetLevelsTxtRecord(levelId);
    return rec ? rec->szLevelName : "";
}
// Levels.txt's EntryFile for a level: the art retail blits as its level-ENTRY banner, with no
// directory and no extension (`A3L3` for the Great Marsh). The client resolves it under the level's
// own act and draws its frames; s693 measured retail 1.13c doing exactly that on a waypoint arrival.
EMSCRIPTEN_KEEPALIVE const char* d2_level_entry_file(int levelId) {
    D2LevelsTxt* rec = DATATBLS_GetLevelsTxtRecord(levelId);
    return rec ? rec->szEntryFile : "";
}
// Retail's 0x49 TakeOrCloseWp: travel to levelId through that waypoint, or close the menu with 0.
EMSCRIPTEN_KEEPALIVE int d2_session_command_waypoint(uint32_t waypointId, int32_t levelId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    uint8_t packet[9] = { 0x49 };
    const int32_t guid = (int32_t)waypointId; memcpy(packet + 1, &guid, 4); memcpy(packet + 5, &levelId, 4);
    const int32_t oldLevel = DUNGEON_GetLevelIdFromRoom(UNITS_GetRoom(g_targetPlayer));
    if (!sessionDispatchGate(0x49)) return -2;
    const int32_t rc = D2GAME_PACKETCALLBACK_Rcv0x49_TakeOrCloseWp_6FC87E60(g_game, g_targetPlayer, packet, sizeof packet);
    const int32_t newLevel = DUNGEON_GetLevelIdFromRoom(UNITS_GetRoom(g_targetPlayer));
    if (newLevel != oldLevel) return adoptSessionLevelChange(oldLevel, newLevel);
    return rc == 0;
}
// The 0x77 UpdateUi codes the server has sent since the last call, in order, then cleared.
// 16 = UPDATEUI_OPENSTASH, 17 = UPDATEUI_CLOSESTASH (D2Constants.h).
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_take_ui_events(uint8_t* out, uint32_t cap) {
    return D2WEB_TakeUiEvents(out, cap);
}
// Every SOUND EVENT retail's own server code has attached to a unit since the last call, as int32
// septets {unitType, unitId, classId, soundEvent, updateUnitType, updateUnitId, nativeSelectedRow}, and the events are
// CONSUMED (retail's per-client send pass clears the same flag, sub_6FCBC300). nativeSelectedRow is the
// MonSounds.Taunt row for event 0x10 or SkillTxt.StSound for event 0x0C when its native lookup succeeds;
// other events carry -1.
//
// The mechanism is already retail's and already runs here: 114 call sites across the vendored D2Game
// call SUNIT_AttachSound(pUnit, nSoundEvent, pUpdateUnit) -- a monster noticing the player
// (AI/AiThink 18), an NPC healing (UNIT/SUnitNpc 10), skills, items, quests -- and SUNIT_AttachSound
// records the event on the unit itself (nUpdateType, pUpdateUnit) and sets UNITFLAG_HASEVENTSOUND
// (SUnit.cpp). Retail's client is then sent packet 0x2C {nUnitType, dwUnitGUID, nSoundEvent}
// (D2GAME_UpdateUnit_6FCC6080 -> sub_6FC3F3F0) and plays it; pUpdateUnit is the AUDIENCE filter --
// null means every client, a unit means only that unit's client (the test 6FCC6080 opens with).
// The headless session has no client to send to, so the events are drained here instead -- the same
// boundary d2_session_take_ui_events uses.
//
// The D2Client event-to-row rule remains client behavior: D2CLIENT_PlaySound uses the MonSounds/Sounds
// tables, and nothing in reference/ carries D2Client (D2MOO stops at D2Gfx/D2Win). For events 0x10 and
// 0x0C, the drain also carries the row selected by the client's table lookup; it does not change the
// attached event. Event 0x11 remains client-tick gated. 
extern "C" int d2_shrine_use_take(int32_t*);
extern "C" int d2_shrine_direct_hp_loss(void);
EMSCRIPTEN_KEEPALIVE int d2_session_shrine_hp_loss() { return d2_shrine_direct_hp_loss(); }
EMSCRIPTEN_KEEPALIVE int d2_session_take_sounds(int32_t* out, int cap) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_lastLevel || !out || cap < 7) return 0;
    int n = 0;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx && n + 7 <= cap; roomEx = roomEx->pDrlgRoomNext) {
        D2ActiveRoomStrc* room = roomEx->pRoom;
        if (!room) continue;
        for (D2UnitStrc* unit = room->pUnitFirst; unit && n + 7 <= cap; unit = unit->pRoomNext) {
            if (!(unit->dwFlags & UNITFLAG_HASEVENTSOUND)) continue;
            out[n++] = (int32_t)unit->dwUnitType;
            out[n++] = (int32_t)unit->dwUnitId;
            out[n++] = unit->dwClassId;
            out[n++] = (int32_t)unit->nUpdateType;
            out[n++] = unit->pUpdateUnit ? (int32_t)unit->pUpdateUnit->dwUnitType : -1;
            out[n++] = unit->pUpdateUnit ? (int32_t)unit->pUpdateUnit->dwUnitId : 0;
            int32_t selectedRow = -1;
            if (unit->dwUnitType == UNIT_MONSTER && unit->nUpdateType == 0x10) {
                D2MonsterDataStrc* monster = unit->pMonsterData;
                D2MonStatsTxt* stats = monster
                    ? DATATBLS_GetMonStatsTxtRecord(unit->dwClassId) : nullptr;
                if (monster && stats) {
                    int32_t superUniqueProfileId = -1;
                    if (monster->nTypeFlag & MONTYPEFLAG_SUPERUNIQUE) {
                        D2SuperUniquesTxt* super = DATATBLS_GetSuperUniquesTxtRecord(monster->wBossHcIdx);
                        if (super) superUniqueProfileId = static_cast<int32_t>(super->dwMonSound);
                    }
                    const bool hasUniqueOrMinionFlag =
                        (monster->nTypeFlag & (MONTYPEFLAG_UNIQUE | MONTYPEFLAG_MINION)) != 0;
                    const int32_t profileId = d2::monster_sound::selectProfileId(
                        (monster->nTypeFlag & MONTYPEFLAG_SUPERUNIQUE) != 0,
                        hasUniqueOrMinionFlag,
                        superUniqueProfileId,
                        static_cast<int32_t>(stats->wUMonSound),
                        static_cast<int32_t>(stats->wMonSound));
                    if (profileId >= 0) {
                        D2MonSoundsTxt* sounds = DATATBLS_GetMonSoundsTxtRecordFromSoundId(profileId);
                        if (sounds && sounds->dwTaunt > 0 && sounds->dwTaunt <= INT32_MAX)
                            selectedRow = static_cast<int32_t>(sounds->dwTaunt);
                    }
                }
            } else if (unit->nUpdateType == 0x0C) {
                D2StatListStrc* statList = STATLIST_GetStatListFromUnitAndFlag(unit, 0x44);
                if (statList) {
                    const int32_t skillId = STATLIST_GetStatValue(statList, STAT_MODIFIERLIST_SKILL, 0);
                    D2SkillStrc* skill = SKILLS_GetHighestLevelSkillFromSkillId(unit, skillId);
                    D2SkillsTxt* skillTxt = skill ? DATATBLS_GetSkillsTxtRecord(skillId) : nullptr;
                    if (skillTxt && skillTxt->wStSound > 0) selectedRow = (int32_t)skillTxt->wStSound;
                }
            }
            out[n++] = selectedRow;
            unit->dwFlags &= ~UNITFLAG_HASEVENTSOUND;
        }
        // 1.13c D2Client+0x23170 (unit create, caller 0xA7780): a monster not in a neutral mode (0xA5440 == 0) whose
        // MonSounds record (0x22A70) has Init (+0x70) nonzero plays that Sounds row once (unit+0x80 set). Emitted as
        // soundEvent 0x100 with the Init row in the last field; the played set is the +0x80 flag.
        for (D2UnitStrc* unit = room->pUnitFirst; unit && n + 7 <= cap; unit = unit->pRoomNext) {
            if (unit->dwUnitType != UNIT_MONSTER || !unit->pMonsterData || unit->dwAnimMode == 0 || unit->dwAnimMode == 0xC) continue;
            if (g_monInitPlayed.count(unit->dwUnitId)) continue;
            D2MonStatsTxt* stats = DATATBLS_GetMonStatsTxtRecord(unit->dwClassId);
            if (!stats) continue;
            D2MonsterDataStrc* monster = unit->pMonsterData;
            int32_t superProfile = -1;
            if (monster->nTypeFlag & MONTYPEFLAG_SUPERUNIQUE) {
                D2SuperUniquesTxt* super = DATATBLS_GetSuperUniquesTxtRecord(monster->wBossHcIdx);
                if (super) superProfile = static_cast<int32_t>(super->dwMonSound);
            }
            const int32_t profileId = d2::monster_sound::selectProfileId((monster->nTypeFlag & MONTYPEFLAG_SUPERUNIQUE) != 0,
                (monster->nTypeFlag & (MONTYPEFLAG_UNIQUE | MONTYPEFLAG_MINION)) != 0, superProfile,
                static_cast<int32_t>(stats->wUMonSound), static_cast<int32_t>(stats->wMonSound));
            D2MonSoundsTxt* sounds = profileId >= 0 ? DATATBLS_GetMonSoundsTxtRecordFromSoundId(profileId) : nullptr;
            if (sounds && !sounds->dwInit && unit->dwAnimMode == 8 && d2::realm::clientMonsterSkill1SoundPlays(sounds->dwSkill[0]) && sounds->dwSkill[0] <= INT32_MAX && g_heldMonsters.count(unit->dwUnitId)) {
                // 23170 with Init == 0 on a client-created unit in mode 8 (SK1): the 0x223D0 == 8 arm runs 0x22CA0 slot 8, which submits MonSounds Skill1 (+0x44).
                g_monInitPlayed.insert(unit->dwUnitId);
                out[n++] = (int32_t)unit->dwUnitType; out[n++] = (int32_t)unit->dwUnitId; out[n++] = unit->dwClassId;
                out[n++] = 0x100; out[n++] = -1; out[n++] = 0; out[n++] = (int32_t)sounds->dwSkill[0];
                continue;
            }
            if (!sounds || !sounds->dwInit || sounds->dwInit > INT32_MAX) continue;
            g_monInitPlayed.insert(unit->dwUnitId);
            out[n++] = (int32_t)unit->dwUnitType; out[n++] = (int32_t)unit->dwUnitId; out[n++] = unit->dwClassId;
            out[n++] = 0x100; out[n++] = -1; out[n++] = 0; out[n++] = (int32_t)sounds->dwInit;
            // 1.13c D2Client+0x24490 (mode 8 -> 0x22CA0 slot 8) runs on the same create as 0x23170's Init arm, so a unit
            // born in SK1 also plays its MonSounds Skill1 (a retail recording skill 94: golemfire 2574 Init, then 1077 spawnfire at the same tick).
            if (unit->dwAnimMode == 8 && d2::realm::clientMonsterSkill1SoundPlays(sounds->dwSkill[0]) && sounds->dwSkill[0] <= INT32_MAX && g_heldMonsters.count(unit->dwUnitId) && n + 7 <= cap) {
                out[n++] = (int32_t)unit->dwUnitType; out[n++] = (int32_t)unit->dwUnitId; out[n++] = unit->dwClassId;
                out[n++] = 0x100; out[n++] = -1; out[n++] = 0; out[n++] = (int32_t)sounds->dwSkill[0];
            }
        }
    }
    {   // 1.13c D2Client+0x73760 (state-on, reached from the 0xA8 packet handler 0xADD70): a state that was not
        // set on the unit yet plays its States.txt onsound row (+0x26, int16, >= 0) once; an already-set state
        // is silent (the ord10494 gate). Emitted as soundEvent 0x101 with the row in the last field.
        std::unordered_set<uint64_t> cur;
        std::vector<std::array<int32_t, 7>> found;
        for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
            D2ActiveRoomStrc* room = roomEx->pRoom;
            if (!room) continue;
            for (D2UnitStrc* unit = room->pUnitFirst; unit; unit = unit->pRoomNext) {
                for (int32_t s = 0; s <= STATE_UBERMINION; s++) {
                    if (!STATES_CheckState(unit, s)) continue;
                    const uint64_t key = ((uint64_t)unit->dwUnitType << 40) | ((uint64_t)unit->dwUnitId << 8) | (uint64_t)s;
                    cur.insert(key);
                    if (g_prevOnsoundStates.count(key)) continue;
                    D2StatesTxt* st = DATATBLS_GetStatesTxtRecord(s);
                    if (!st || (int16_t)st->wOnSound < 0) continue;
                    found.push_back({(int32_t)unit->dwUnitType, (int32_t)unit->dwUnitId, (int32_t)unit->dwClassId, 0x101, -1, 0, (int32_t)(int16_t)st->wOnSound});
                }
            }
        }
        g_prevOnsoundStates.swap(cur);
        // One drain of latency: a state set in step N is recorded at tick N, as retail's client plays it on the packet's tick.
        // Measured (a retail recording auras vs a retail recording curses): a state on the player plays one drain later, a monster's at once.
        // The shapeshift states (Werewolf/Werebear, set by the self-cast do-frame) are the exception: a retail recording
        // (skills 223, 228) plays their row on the tick the state is set, not one drain later.
        const D2StatesTxt* wolf = DATATBLS_GetStatesTxtRecord(STATE_WOLF); const D2StatesTxt* bear = DATATBLS_GetStatesTxtRecord(STATE_BEAR);
        const int32_t wolfRow = wolf ? (int32_t)(int16_t)wolf->wOnSound : -2, bearRow = bear ? (int32_t)(int16_t)bear->wOnSound : -2;
        std::vector<std::array<int32_t, 7>> later, now = g_pendingOnsound;
        for (const auto& e : found) (e[0] == 0 && e[6] != wolfRow && e[6] != bearRow ? later : now).push_back(e);
        // Entries that did not fit under cap stay queued ahead of this drain's new player entries, so none is lost.
        size_t emitted = 0;
        for (; emitted < now.size() && n + 7 <= cap; emitted++)
            for (int32_t v : now[emitted]) out[n++] = v;
        later.insert(later.begin(), now.begin() + emitted, now.end());
        g_pendingOnsound.swap(later);
    }
    {   // 1.13c D2Client+0x24A70: the shrine state sound (see game_net_stubs.cpp) on the operating player.
        int32_t su[2];
        while (n + 7 <= cap && g_targetPlayer && d2_shrine_use_take(su)) {
            out[n++] = 0; out[n++] = su[0]; out[n++] = (int32_t)g_targetPlayer->dwClassId; out[n++] = 0x103; out[n++] = -1; out[n++] = 0; out[n++] = su[1];
        }
    }
    {   // 1.13c D2Client+0xA2C90: a skill whose client do-function slot is empty (index 0) plays its wDoSound (+0x100).
        // The player's do reaches the client one drain later, as the state packet does.
        for (const auto& e : g_pendingSkillDo) { if (n + 7 > cap) break; for (int32_t v : e) out[n++] = v; }
        g_pendingSkillDo.clear();
        for (const auto& e : g_pendingOverlaySounds) { if (n + 7 > cap) break; for (int32_t v : e) out[n++] = v; }
        g_pendingOverlaySounds.clear();
        if (g_refusedDoWait > 0 && --g_refusedDoWait == 0 && g_playerSkillDoCount < 8) g_playerSkillDoIds[g_playerSkillDoCount++] = g_refusedDoSkill;
        for (int32_t i = 0; i < g_playerSkillDoCount; i++) {
            D2SkillsTxt* t = DATATBLS_GetSkillsTxtRecord(g_playerSkillDoIds[i]);
            if (t && (t->wCltDoFunc == 0 || t->wCltDoFunc == 28) && t->wDoSound > 0 && g_targetPlayer) // 28: Meteor's launch sound is its wDoSound too
                g_pendingSkillDo.push_back({0, (int32_t)g_targetPlayer->dwUnitId, (int32_t)g_targetPlayer->dwClassId, 0x102, -1, 0, (int32_t)t->wDoSound});
            // A2C90 spawns the skill's cltmissile (+0xE8) at the do; with no server missile of its own (Blaze) the client missile's TravelSound is the do's only sound.
            // cltdofunc 27 (Chain Lightning) spawns its cltmissilea at the do: that client bolt sounds once per cast; the server's bolts are not on the client (d2_session_missile_on_client).
            if (t && (t->wCltDoFunc == 0 && t->wSrvMissile <= 0 || t->wCltDoFunc == 27) && g_targetPlayer) { const int32_t c = (int16_t)(t->wCltDoFunc == 27 ? t->wCltMissileA : t->wCltMissile);
                const D2MissilesTxt* m = c > 0 && c < (int32_t)sgptDataTables->nMissilesTxtRecordCount ? DATATBLS_GetMissilesTxtRecord(c) : nullptr;
                if (m && m->wTravelSound > 0 && m->wTravelSound != 0xFFFF) g_pendingSkillDo.push_back({0, (int32_t)g_targetPlayer->dwUnitId, (int32_t)g_targetPlayer->dwClassId, 0x102, -1, 0, (int32_t)m->wTravelSound}); }
        }
        if (g_cltMissileRow > 0 && g_targetPlayer) g_pendingSkillDo.push_back({0, (int32_t)g_targetPlayer->dwUnitId, (int32_t)g_targetPlayer->dwClassId, 0x102, -1, 0, g_cltMissileRow});
        g_cltMissileRow = 0;
        g_playerSkillDoCount = 0;
    }
    return n / 7;
}
// Retail's 0x4F ClickButton, e.g. TRADEBTN_CLOSESTASH (18) when the player closes the stash, or
// withdraw/deposit (19/20) with a gold amount. Through retail's own handler.
EMSCRIPTEN_KEEPALIVE int d2_session_command_button(uint32_t button, int32_t gold) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || button > 0xFFFF) return 0;
    uint8_t packet[7] = { 0x4F };
    const uint16_t b = (uint16_t)button, hi = (uint16_t)((uint32_t)gold >> 16), lo = (uint16_t)gold;
    memcpy(packet + 1, &b, 2); memcpy(packet + 3, &hi, 2); memcpy(packet + 5, &lo, 2);
    if (!sessionDispatchGate(0x4F)) return -2;
    return D2GAME_PACKETCALLBACK_Rcv0x4F_ClickButton_6FC881D0(g_game, g_targetPlayer, packet, sizeof packet) == 0;
}
EMSCRIPTEN_KEEPALIVE int d2_session_command_end_npc(uint32_t npcId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    int32_t type = -1, guid = -1;
    if (!SUNIT_GetInteractInfo(g_targetPlayer, &type, &guid) || type != UNIT_MONSTER || guid != (int32_t)npcId) return 0;
    uint8_t packet[9] = { 0x30 };
    const int32_t npcGuid = (int32_t)npcId; memcpy(packet + 5, &npcGuid, sizeof npcGuid);
    if (!sessionDispatchGate(0x30)) return -2;
    const bool ended = D2GAME_PACKETCALLBACK_Rcv0x30_TerminateEntityChat_6FC869F0(g_game, g_targetPlayer, packet, sizeof packet) == 0;
    // g_sessionNpcDialog records the conversation the player OPENED; once retail's own 0x30 handler
    // has ended it, that record no longer describes anything, and a client reading it would keep the
    // window up forever (s000 step "Esc closes the NPC window"). Clear it only on success, so a
    // refused end still shows the conversation retail kept open.
    if (ended) memset(&g_sessionNpcDialog, 0, sizeof g_sessionNpcDialog);
    return ended;
}
EMSCRIPTEN_KEEPALIVE const D2SessionNpcResponse* d2_session_npc_response() {
    return &g_sessionNpcResponse;
}
static uint32_t sessionInventoryItemCount(D2InventoryStrc* inventory) {
    uint32_t count = 0;
    if (inventory) for (D2UnitStrc* item = INVENTORY_GetFirstItem(inventory); item; item = INVENTORY_GetNextItem(item))
        if (INVENTORY_UnitIsItem(item)) ++count;
    return count;
}
// Fill a presentation record for one item, without touching the player's own list.
static void appendVendorPresentation(D2UnitStrc* item) {
    if (!item || item->dwUnitType != UNIT_ITEM) return;
    D2SessionItemPresentation out = {};
    out.id = item->dwUnitId; out.classId = item->dwClassId; out.mode = item->dwAnimMode;
    out.quality = ITEMS_GetItemQuality(item); out.itemFlags = ITEMS_GetItemFlags(item);
    out.framePrecise = item->nSeqCurrentFramePrecise; out.frameCountPrecise = item->dwFrameCountPrecise;
    clientLandedGroundClock(item, out);
    if (item->pStaticPath) {
        out.x = item->pStaticPath->tGameCoords.nX; out.y = item->pStaticPath->tGameCoords.nY;
        out.gridX = (int16_t)item->pStaticPath->tGameCoords.nX;
        out.gridY = (int16_t)item->pStaticPath->tGameCoords.nY;
    }
    out.invPage = ITEMS_GetInvPage(item); out.bodyLoc = ITEMS_GetBodyLocation(item);
    if (D2ItemsTxt* txt = DATATBLS_GetItemsTxtRecord(item->dwClassId)) {
        out.code = txt->dwCode;
        memcpy(out.flippyFile, txt->szFlippyFile, sizeof out.flippyFile);
        memcpy(out.invFile, sessionItemInventoryFile(item, txt), sizeof out.invFile);
        // Same as the player-side producer below: the cells the item covers come from ItemsTxt, so a
        // two-handed axe is 2x4 on the vendor grid and a click anywhere on it buys it. Leaving these
        // zero made every vendor item a 1x1 for the client's hit test.
        out.invWidth = txt->nInvWidth; out.invHeight = txt->nInvHeight;
    }
    g_sessionVendorPresentations.push_back(out);
}

// Retail's own vendor bridge functions (D2GAME_NPC_Repair_6FCC95B0's classId switch) never reach
// ITEMS_GetTransactionCost/ITEMS_CalculateTransactionCost for a monster that is not an npc.txt
// record: only the real vendor packet handlers on the server side ever call it, and every caller
// there is a monster the retail UI could only ever have opened a shop on. Our own bridge commands
// below take a plain npcId over ccall, without that UI restricting which unit could be named, so
// a bot interacting with the nearest talkable monster (Cain, Natalya -- interactable but not
// vendors) reached this cost calculation with a classId npc.bin has no record for and hit
// ITEMS_CalculateTransactionCost's own D2_ASSERT(pNpcTxtRecord), aborting the session. Guard at
// the one place all these bridge entry points funnel through.
static bool sessionNpcIsVendor(D2UnitStrc* npc) {
    return npc && DATATBLS_GetNpcTxtRecord(npc->dwClassId) != nullptr;
}

static void updateSessionVendorItems(D2UnitStrc* npc) {
    g_sessionVendorItems.clear();
    g_sessionVendorPresentations.clear();
    if (!npc || !g_targetPlayer || !g_game || !sessionNpcIsVendor(npc)) return;
    D2InventoryStrc* inventory = g_sessionVendor.action == 2
        ? SUNITPROXY_GetGambleInventory(g_game, g_targetPlayer, npc) : npc->pInventory;
    if (!inventory) return;
    for (D2UnitStrc* item = INVENTORY_GetFirstItem(inventory); item; item = INVENTORY_GetNextItem(item)) {
        if (!INVENTORY_UnitIsItem(item)) continue;
        D2SessionVendorItem out = {};
        out.id = item->dwUnitId; out.classId = item->dwClassId; out.mode = item->dwAnimMode;
        out.quality = ITEMS_GetItemQuality(item); out.flags = item->dwFlags;
        if (D2ItemsTxt* txt = DATATBLS_GetItemsTxtRecord(item->dwClassId)) out.code = txt->dwCode;
        out.cost = ITEMS_GetTransactionCost(g_targetPlayer, item, (D2C_Difficulties)g_game->nDifficulty,
            UNITS_GetPlayerData(g_targetPlayer)->pQuestData[g_game->nDifficulty], npc->dwClassId,
            g_sessionVendor.action == 2 ? D2C_TransactionTypes::TRANSACTIONTYPE_GAMBLE
                                        : D2C_TransactionTypes::TRANSACTIONTYPE_BUY);
        out.hash = sessionHashBytes(D2VEC_FNV_INIT, &out, offsetof(D2SessionVendorItem, hash));
        g_sessionVendorItems.push_back(out);
        appendVendorPresentation(item);
    }
}
// Fixture: set a stat on a specific item unit (to damage it before a repair). Mirrors the
// player set_stat but targets an item the player owns.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_set_item_stat(uint32_t itemId, uint32_t statId, int32_t value) {
    if (!g_sessionLive || !g_game) return 0;
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId);
    if (!item) return 0;
    STATLIST_SetUnitStat(item, (int32_t)statId, value, 0);
    updateSessionSnapshot();
    return STATLIST_UnitGetStatValue(item, (int32_t)statId, 0) == value ? 1 : 0;
}

// The cursor item handed to an NPC (Larzuk's socket reward, Charsi's imbue): retail's 0x38 with the item
// GUID, action 0 -- NPC_HandleDialogMessage reads nType only for 1..3 and routes the rest by NPC class.
EMSCRIPTEN_KEEPALIVE int d2_session_command_npc_item(uint32_t npcId, uint32_t itemId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return -1;
    D2GSPacketClt38 packet = {};
    packet.nHeader = 0x38; packet.nAction = 0; packet.nNpcGUID = (int32_t)npcId; packet.nItemGUID = (int32_t)itemId;
    if (!sessionDispatchGate(0x38)) return -2;
    const int32_t result = D2GAME_PACKETCALLBACK_Rcv0x38_EntityAction_6FC86CB0(g_game, g_targetPlayer, &packet, sizeof packet);
    updateSessionSnapshot();
    return result;
}

EMSCRIPTEN_KEEPALIVE int d2_session_command_npc_vendor(uint32_t npcId, uint32_t action) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || (action != 1 && action != 2)) return 0;
    int32_t type = -1, guid = -1;
    const bool hasInteraction = SUNIT_GetInteractInfo(g_targetPlayer, &type, &guid);
    if (!npcVendorRequestMatchesInteraction(npcId, hasInteraction, type, guid, UNIT_MONSTER)) return 0;
    if (!sessionDispatchGate(0x38)) return -2;

    // A failed precondition is a no-op above. Keep a snapshot too: a native handler refusal
    // must not replace the last accepted counter or its item presentation state.
    NpcVendorStateSnapshot previous(g_sessionVendor, g_sessionVendorItems, g_sessionVendorPresentations);
    memset(&g_sessionVendor, 0, sizeof g_sessionVendor);
    g_sessionVendor.version = 1; g_sessionVendor.size = sizeof(D2SessionVendor);
    g_sessionVendor.packetHeader = 0x38; g_sessionVendor.packetSize = sizeof(D2GSPacketClt38);
    g_sessionVendor.action = action; g_sessionVendor.npcId = npcId;
    D2GSPacketClt38 packet = {};
    packet.nHeader = 0x38; packet.nAction = action; packet.nNpcGUID = (int32_t)npcId; packet.nItemGUID = -1;
    const int32_t handler = D2GAME_PACKETCALLBACK_Rcv0x38_EntityAction_6FC86CB0(g_game, g_targetPlayer, &packet, sizeof packet);
    D2UnitStrc* npc = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)npcId);
    updateSessionVendorItems(npc);
    int32_t unused = 0; D2NpcRecordStrc* record = npc ? SUNITPROXY_GetNpcRecordFromUnit(g_game, npc, &unused) : nullptr;
    if (record) { g_sessionVendor.trading = record->bTrading; g_sessionVendor.vendorInit = record->npcTrade.bVendorInit; }
    uint32_t hash = D2VEC_FNV_INIT;
    if (npc && npc->pInventory) for (D2UnitStrc* item = INVENTORY_GetFirstItem(npc->pInventory); item; item = INVENTORY_GetNextItem(item)) {
        if (!INVENTORY_UnitIsItem(item)) continue;
        const uint32_t fields[] = { (uint32_t)item->dwClassId, (uint32_t)item->dwAnimMode,
            (uint32_t)ITEMS_GetItemQuality(item), item->dwFlags, item->dwFlagEx };
        hash = sessionHashBytes(hash, fields, sizeof fields); ++g_sessionVendor.itemCount;
        if (g_sessionVendor.itemCount == 1) {
            g_sessionVendor.firstClass = item->dwClassId; g_sessionVendor.firstQuality = ITEMS_GetItemQuality(item);
            if (D2ItemsTxt* txt = DATATBLS_GetItemsTxtRecord(item->dwClassId)) g_sessionVendor.firstCode = txt->dwCode;
        }
    }
    g_sessionVendor.itemHash = hash;
    const bool accepted = npcVendorTransitionAccepted(handler, record != nullptr, record && record->bTrading);
    g_sessionVendor.result = accepted ? 1u : 0u;
    if (!accepted) { previous.restore(); return 0; }
    g_sessionVendor.hash = sessionHashBytes(D2VEC_FNV_INIT, &g_sessionVendor, offsetof(D2SessionVendor, hash));
    return 1;
}
EMSCRIPTEN_KEEPALIVE const D2SessionVendor* d2_session_vendor() { return &g_sessionVendor; }
EMSCRIPTEN_KEEPALIVE const D2SessionVendorItem* d2_session_vendor_items() { return g_sessionVendorItems.data(); }
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_vendor_item_count() { return (uint32_t)g_sessionVendorItems.size(); }

// D2Common+0x241D0 over 0x22690: not a quest item, not type 0x27, item flag 0x1000 clear.
static bool sessionItemIsPlain(D2UnitStrc* item) {
    D2ItemsTxt* txt = DATATBLS_GetItemsTxtRecord(item->dwClassId);
    return txt && !(item->pItemData->dwItemFlags & 0x1000) && !txt->nQuest && txt->wType[0] != 0x27;
}
static int sessionVendorTransaction(uint32_t npcId, uint32_t itemId, bool buy, bool multibuy = false) {
    memset(&g_sessionVendorTransaction, 0, sizeof g_sessionVendorTransaction);
    D2SessionVendorTransaction& out = g_sessionVendorTransaction;
    out.version = 1; out.size = sizeof out; out.packetHeader = buy ? 0x32 : 0x33; out.packetSize = 17;
    out.npcId = npcId; out.itemId = itemId; out.transactionType = buy ? 0 : 1;
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    D2UnitStrc* npc = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)npcId);
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId);
    int32_t interactType = -1, interactGuid = -1;
    if (!npc || !item || !sessionNpcIsVendor(npc) ||
        !SUNIT_GetInteractInfo(g_targetPlayer, &interactType, &interactGuid) ||
        interactType != UNIT_MONSTER || interactGuid != (int32_t)npcId) return 0;
    out.itemClass = item->dwClassId; out.itemMode = item->dwAnimMode; out.itemQuality = ITEMS_GetItemQuality(item);
    if (D2ItemsTxt* txt = DATATBLS_GetItemsTxtRecord(item->dwClassId)) out.itemCode = txt->dwCode;
    out.packetCost = ITEMS_GetTransactionCost(g_targetPlayer, item, (D2C_Difficulties)g_game->nDifficulty,
        UNITS_GetPlayerData(g_targetPlayer)->pQuestData[g_game->nDifficulty], npc->dwClassId,
        buy ? D2C_TransactionTypes::TRANSACTIONTYPE_BUY : D2C_TransactionTypes::TRANSACTIONTYPE_SELL);
    out.goldBefore = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_GOLD, 0);
    out.playerItemsBefore = sessionInventoryItemCount(g_targetPlayer->pInventory);
    out.vendorItemsBefore = sessionInventoryItemCount(npc->pInventory);
    out.ownedBefore = ITEMS_IsInPlayersInventory(g_targetPlayer, item, 0) == 1;
    D2WEB_ResetPacketStream();
    if (buy) {
        D2GSPacketClt32 packet = {};
        packet.nHeader = 0x32; packet.dwNpcGUID = (int32_t)npcId; packet.dwItemGUID = (int32_t)itemId;
        packet.nTransactionType = D2C_TransactionTypes::TRANSACTIONTYPE_BUY;
        packet.nItemMode = item->dwAnimMode; packet.bMultibuy = multibuy ? 1 : 0; packet.dwCost = out.packetCost;
        if (!sessionDispatchGate(0x32)) return -2;
        out.handlerResult = D2GAME_PACKETCALLBACK_Rcv0x32_BuyItemFromNpcBuffer_6FC86AE0(g_game, g_targetPlayer, &packet, sizeof packet);
    } else {
        D2GSPacketClt33 packet = {};
        packet.nHeader = 0x33; packet.dwMonGUID = npcId; packet.dwItemGUID = itemId;
        packet.nTab = item->dwAnimMode; packet.dwGold = out.packetCost;
        if (!sessionDispatchGate(0x33)) return -2;
        out.handlerResult = D2GAME_PACKETCALLBACK_Rcv0x33_SellItemToNpcBuffer_6FC86B30(g_game, g_targetPlayer, &packet, sizeof packet);
    }
    if (buy) out.resultItemId = UNITS_GetPlayerData(g_targetPlayer)->dwBoughtItemId;
    D2UnitStrc* after = SUNIT_GetServerUnit(g_game, UNIT_ITEM, buy ? (int32_t)out.resultItemId : (int32_t)itemId);
    out.goldAfter = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_GOLD, 0);
    out.playerItemsAfter = sessionInventoryItemCount(g_targetPlayer->pInventory);
    out.vendorItemsAfter = sessionInventoryItemCount(npc->pInventory);
    out.ownedAfter = after && ITEMS_IsInPlayersInventory(g_targetPlayer, after, 0) == 1;
    if (out.handlerResult == 0) out.flags |= 1;
    if (buy ? (out.goldAfter == out.goldBefore - out.packetCost) : (out.goldAfter == out.goldBefore + out.packetCost)) out.flags |= 2;
    if (buy ? (out.playerItemsAfter > out.playerItemsBefore) : (out.playerItemsAfter < out.playerItemsBefore)) out.flags |= 4;
    if (out.ownedBefore) out.flags |= 8;
    if (out.ownedAfter) out.flags |= 16;
    D2WEB_PacketStream(&g_sessionPacketStreamLength, &g_sessionPacketStreamPackets);
    out.hash = sessionHashBytes(D2VEC_FNV_INIT, &out, offsetof(D2SessionVendorTransaction, hash));
    updateSessionVendorItems(npc);
    return (out.flags & 7) == 7;
}
EMSCRIPTEN_KEEPALIVE int d2_session_command_vendor_gamble(uint32_t npcId, uint32_t itemId) {
    memset(&g_sessionVendorTransaction, 0, sizeof g_sessionVendorTransaction);
    D2SessionVendorTransaction& out = g_sessionVendorTransaction;
    out.version = 1; out.size = sizeof out; out.packetHeader = 0x32; out.packetSize = 17;
    out.npcId = npcId; out.itemId = itemId; out.transactionType = D2C_TransactionTypes::TRANSACTIONTYPE_GAMBLE;
    if (!g_sessionLive || !g_game || !g_targetPlayer || g_sessionVendor.action != 2) return 0;
    D2UnitStrc* npc = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)npcId);
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId);
    D2InventoryStrc* gamble = npc ? SUNITPROXY_GetGambleInventory(g_game, g_targetPlayer, npc) : nullptr;
    int32_t interactType = -1, interactGuid = -1; bool listed = false;
    if (gamble && item) for (D2UnitStrc* it = INVENTORY_GetFirstItem(gamble); it; it = INVENTORY_GetNextItem(it))
        if (it == item) { listed = true; break; }
    if (!npc || !item || !listed || !sessionNpcIsVendor(npc) ||
        !SUNIT_GetInteractInfo(g_targetPlayer, &interactType, &interactGuid) ||
        interactType != UNIT_MONSTER || interactGuid != (int32_t)npcId) return 0;
    out.itemClass = item->dwClassId; out.itemMode = item->dwAnimMode; out.itemQuality = ITEMS_GetItemQuality(item);
    if (D2ItemsTxt* txt = DATATBLS_GetItemsTxtRecord(item->dwClassId)) out.itemCode = txt->dwCode;
    out.packetCost = ITEMS_GetTransactionCost(g_targetPlayer, item, (D2C_Difficulties)g_game->nDifficulty,
        UNITS_GetPlayerData(g_targetPlayer)->pQuestData[g_game->nDifficulty], npc->dwClassId,
        D2C_TransactionTypes::TRANSACTIONTYPE_GAMBLE);
    out.goldBefore = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_GOLD, 0);
    out.playerItemsBefore = sessionInventoryItemCount(g_targetPlayer->pInventory);
    out.vendorItemsBefore = sessionInventoryItemCount(gamble);
    D2WEB_ResetPacketStream();
    D2GSPacketClt32 packet = {};
    packet.nHeader = 0x32; packet.dwNpcGUID = (int32_t)npcId; packet.dwItemGUID = (int32_t)itemId;
    packet.nTransactionType = D2C_TransactionTypes::TRANSACTIONTYPE_GAMBLE;
    packet.nItemMode = item->dwAnimMode; packet.dwCost = out.packetCost;
    if (!sessionDispatchGate(0x32)) return -2;
    out.handlerResult = D2GAME_PACKETCALLBACK_Rcv0x32_BuyItemFromNpcBuffer_6FC86AE0(g_game, g_targetPlayer, &packet, sizeof packet);
    out.resultItemId = UNITS_GetPlayerData(g_targetPlayer)->dwBoughtItemId;
    D2UnitStrc* after = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)out.resultItemId);
    out.goldAfter = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_GOLD, 0);
    out.playerItemsAfter = sessionInventoryItemCount(g_targetPlayer->pInventory);
    out.vendorItemsAfter = sessionInventoryItemCount(gamble);
    out.ownedAfter = after && ITEMS_IsInPlayersInventory(g_targetPlayer, after, 0) == 1;
    if (out.handlerResult == 0) out.flags |= 1;
    if (out.goldAfter == out.goldBefore - out.packetCost) out.flags |= 2;
    if (out.playerItemsAfter > out.playerItemsBefore) out.flags |= 4;
    if (out.ownedAfter) out.flags |= 16;
    D2WEB_PacketStream(&g_sessionPacketStreamLength, &g_sessionPacketStreamPackets);
    out.hash = sessionHashBytes(D2VEC_FNV_INIT, &out, offsetof(D2SessionVendorTransaction, hash));
    updateSessionVendorItems(npc);
    return (out.flags & 23) == 23;
}
// 1.13c D2Client+0x45B40 (the needs-repair predicate the shop gates a repair click on): durability
// below its maximum, or a throwing weapon's quantity below its stack maximum. The third clause
// (D2Common #10607 with a zero out value) is not traced and is not applied.
EMSCRIPTEN_KEEPALIVE int d2_session_item_needs_repair(uint32_t itemId) {
    if (!g_sessionLive || !g_game) return 0;
    // 1.13c D2Client+0x45B40 over the server unit (the rule: d2client/item_needs_repair.h).
    return d2client_item_needs_repair(SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId));
}
EMSCRIPTEN_KEEPALIVE int d2_session_command_vendor_repair(uint32_t npcId, uint32_t itemId) {
    memset(&g_sessionRepair, 0, sizeof g_sessionRepair);
    D2SessionRepair& out = g_sessionRepair;
    out.version = 1; out.size = sizeof out; out.packetHeader = 0x35; out.packetSize = 17;
    out.npcId = npcId; out.itemId = itemId;
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    D2UnitStrc* npc = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)npcId);
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId);
    int32_t interactType = -1, interactGuid = -1;
    if (!npc || !item || !sessionNpcIsVendor(npc) ||
        !SUNIT_GetInteractInfo(g_targetPlayer, &interactType, &interactGuid) ||
        interactType != UNIT_MONSTER || interactGuid != (int32_t)npcId) return 0;
    out.itemClass = item->dwClassId;
    out.goldBefore = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_GOLD, 0);
    out.cost = ITEMS_GetTransactionCost(g_targetPlayer, item, (D2C_Difficulties)g_game->nDifficulty,
        UNITS_GetPlayerData(g_targetPlayer)->pQuestData[g_game->nDifficulty], npc->dwClassId,
        D2C_TransactionTypes::TRANSACTIONTYPE_REPAIR);
    out.durabilityBefore = STATLIST_UnitGetStatValue(item, STAT_DURABILITY, 0);
    out.maxDurability = STATLIST_GetMaxDurabilityFromUnit(item);
    out.quantityBefore = STATLIST_UnitGetStatValue(item, STAT_QUANTITY, 0);
    out.maxQuantity = ITEMS_GetTotalMaxStack(item);
    out.ownedBefore = ITEMS_IsInPlayersInventory(g_targetPlayer, item, 0) == 1;
    uint8_t packet[17] = { 0x35 };
    // Retail D2Client+0x46EE0 sends Ordinal_10973(item, 0x48) here: the item's durability (stat 72).
    const int32_t npcGuid = (int32_t)npcId, itemGuid = (int32_t)itemId, individual = out.durabilityBefore;
    memcpy(packet + 1, &npcGuid, sizeof npcGuid); memcpy(packet + 5, &itemGuid, sizeof itemGuid);
    // Bytes 9-12: the item's dwMode low word, zero-extended (retail D2Client+0x46EE0).
    const uint32_t itemMode = (uint16_t)item->dwAnimMode;
    memcpy(packet + 9, &itemMode, sizeof itemMode);
    memcpy(packet + 13, &individual, sizeof individual);
    if (!sessionDispatchGate(0x35)) return -2;
    out.handlerResult = D2GAME_PACKETCALLBACK_Rcv0x35_Repair_6FC86B70(g_game, g_targetPlayer, packet, sizeof packet);
    out.goldAfter = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_GOLD, 0);
    out.durabilityAfter = STATLIST_UnitGetStatValue(item, STAT_DURABILITY, 0);
    out.quantityAfter = STATLIST_UnitGetStatValue(item, STAT_QUANTITY, 0);
    out.ownedAfter = ITEMS_IsInPlayersInventory(g_targetPlayer, item, 0) == 1;
    if (out.handlerResult == 0) out.flags |= 1;
    if (out.goldAfter == out.goldBefore - out.cost) out.flags |= 2;
    if ((out.maxDurability && out.durabilityAfter == out.maxDurability) ||
        (out.maxQuantity && out.quantityAfter == out.maxQuantity)) out.flags |= 4;
    if (out.ownedBefore && out.ownedAfter) out.flags |= 8;
    out.hash = sessionHashBytes(D2VEC_FNV_INIT, &out, offsetof(D2SessionRepair, hash));
    return (out.flags & 15) == 15;
}
// The shop's Repair All button: the same 0x35 packet with a negative last dword, which is the
// branch D2GAME_NPC_Repair_6FCC95B0 takes to price every repairable item the player carries
// (ITEMS_GetAllRepairCosts), charge it from inventory then stash gold, and repair them all. The
// handler reads only the sign; the retail client sends exactly 0x80000000 (D2Client 0x46EE0, Repair
// All with no confirm box: 0x47D60). Returns 1 when the handler took the packet; the outcome is in the items'
// durability and the gold (a non-smith, or too little gold, is answered with a 0x2A refusal).
EMSCRIPTEN_KEEPALIVE int d2_session_command_vendor_repair_all(uint32_t npcId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    uint8_t packet[17] = { 0x35 };
    const int32_t npcGuid = (int32_t)npcId, all = (int32_t)0x80000000u;
    memcpy(packet + 1, &npcGuid, sizeof npcGuid); memcpy(packet + 13, &all, sizeof all);
    const int32_t rc = D2GAME_PACKETCALLBACK_Rcv0x35_Repair_6FC86B70(g_game, g_targetPlayer, packet, sizeof packet);
    updateSessionSnapshot();
    return rc == 0;
}
EMSCRIPTEN_KEEPALIVE const D2SessionRepair* d2_session_vendor_repair() { return &g_sessionRepair; }
extern int32_t g_d2LastTransactionResult;
// The result byte of the last 0x2A trade reply (D2Client+0x4BC80 copies it to 0x6FBC9781), then cleared; -1 = none.
EMSCRIPTEN_KEEPALIVE int32_t d2_session_last_transaction_result() {
    const int32_t r = g_d2LastTransactionResult; g_d2LastTransactionResult = -1; return r;
}
EMSCRIPTEN_KEEPALIVE int d2_session_command_vendor_sell(uint32_t npcId, uint32_t itemId) {
    return sessionVendorTransaction(npcId, itemId, false);
}
// 1.13c D2Client+0x46230: the amount of an inventory tooltip's "Sell value" line.
// -1 = no line: no trade window open, the item is not carried, or D2Common ordinal 10705 (0x241D0) refuses it.
static D2UnitStrc* g_sellNpc;
static int32_t sellCost(void* item, int32_t npcClass, int32_t type) {
    return ITEMS_GetTransactionCost(g_targetPlayer, (D2UnitStrc*)item, (D2C_Difficulties)g_game->nDifficulty,
        UNITS_GetPlayerData(g_targetPlayer)->pQuestData[g_game->nDifficulty], npcClass, (D2C_TransactionTypes)type);
}
// D69 gamble-case instrument: ITEMS_GetTransactionCost type 2 (gamble) of any item unit for NPC class `npcClass`,
// optionally with wItemFormat (pItemData+0x30) forced to 0 for the call, as the retail capture prices it. -1 = no item.
EMSCRIPTEN_KEEPALIVE int32_t d2_session_gamble_price(uint32_t itemId, uint32_t npcClass, uint32_t forceFormat0) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return -1;
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId);
    if (!item || !item->pItemData) return -1;
    const uint16_t fmt = item->pItemData->wItemFormat;
    if (forceFormat0 == 2) return fmt;   // 2: report wItemFormat
    if (forceFormat0) item->pItemData->wItemFormat = 0;
    const int32_t price = sellCost(item, (int32_t)npcClass, 2);
    item->pItemData->wItemFormat = fmt;
    return price;
}
EMSCRIPTEN_KEEPALIVE int32_t d2_session_item_sell_value(uint32_t itemId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !(g_sessionVendor.action == 1 || g_sessionVendor.action == 2)) return -1;
    g_sellNpc = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)g_sessionVendor.npcId);
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId);
    if (!g_sellNpc || !item || !item->pItemData || !sessionNpcIsVendor(g_sellNpc) || ITEMS_IsInPlayersInventory(g_targetPlayer, item, 0) != 1 ||
        !sessionItemIsPlain(item)) return -1;
    static const D2ClientPriceLineCb cb = {
        [](void*) -> int32_t { return 0; }, [](uint32_t) -> int32_t { return 0; },
        [](int32_t* c) -> bool { *c = (int32_t)g_sellNpc->dwClassId; return true; }, [](void*) -> int32_t { return 1; },
        sellCost, [](uint32_t) -> const uint16_t* { static const uint16_t z[1] = { 0 }; return z; },
        []() -> int32_t { return 0; }, [](void*) -> int32_t { return 1; }, [](void*) -> int32_t { return 1; },
        [](void*) -> int32_t { return 0; }, [](int32_t) {} };
    int32_t amt = -1; uint16_t line[16];
    // 1.13c D2Client+0x46230: the sell line (carried item, trade window not the gamble one)
    return d2_client_46230(&amt, item, 1, line, 16, 0, 0, &cb) ? amt : -1;
}
// 1.13c D2Client+0x46230 in the repair cursor state (DAT_6FBCBC34 == 4, the uiState argument): the amount
// D2Client+0x47500 prints under "Gold:" in the single-item Repair confirm. Unidentified -> 0 (the 0xFB6 line);
// the 0x45B40 gate refuses -> 0; else ITEMS_GetTransactionCost type 3 (repair). -1 = no trade window / item.
EMSCRIPTEN_KEEPALIVE int32_t d2_session_item_repair_cost(uint32_t itemId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !(g_sessionVendor.action == 1 || g_sessionVendor.action == 2)) return -1;
    g_sellNpc = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)g_sessionVendor.npcId);
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId);
    if (!g_sellNpc || !item || !item->pItemData || !sessionNpcIsVendor(g_sellNpc)) return -1;
    static const D2ClientPriceLineCb cb = {
        [](void*) -> int32_t { return 0; }, [](uint32_t) -> int32_t { return 0; },
        [](int32_t* c) -> bool { *c = (int32_t)g_sellNpc->dwClassId; return true; }, [](void*) -> int32_t { return 1; },
        sellCost, [](uint32_t) -> const uint16_t* { static const uint16_t z[1] = { 0 }; return z; },
        []() -> int32_t { return 0; },
        [](void* u) -> int32_t { return (((D2UnitStrc*)u)->pItemData->dwItemFlags & 0x10) ? 1 : 0; },   // D2Common 10202, flag 0x10
        [](void* u) -> int32_t { return d2_session_item_needs_repair((uint32_t)((D2UnitStrc*)u)->dwUnitId); },   // 0x45B40
        [](void*) -> int32_t { return 0; }, [](int32_t) {} };
    int32_t amt = 0; uint16_t line[64] = { 0 };
    return d2_client_46230(&amt, item, 1, line, 64, 4, 0, &cb) ? amt : 0;
}
EMSCRIPTEN_KEEPALIVE int d2_session_command_vendor_buy(uint32_t npcId, uint32_t itemId) {
    return sessionVendorTransaction(npcId, itemId, true);
}
EMSCRIPTEN_KEEPALIVE int d2_session_command_vendor_multibuy(uint32_t npcId, uint32_t itemId) {
    return sessionVendorTransaction(npcId, itemId, true, true);
}
// STAT_QUANTITY of a server item unit, for reading a multibuy's filled stack.
EMSCRIPTEN_KEEPALIVE int32_t d2_session_item_quantity(uint32_t itemId) {
    if (!g_sessionLive || !g_game) return -1;
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId);
    return item ? STATLIST_UnitGetStatValue(item, STAT_QUANTITY, 0) : -1;
}
// ITEMS_IsQuestItem of a server item unit: Items.txt's own nQuest column, which is the level's data
// and not a code list in the client. The shared player's "part with this" pick (lib/player.mjs
// `disposable`) already says it keeps quest items -- its keep-list was codes and missed `ice`, so a
// death recovery dropped Malah's Defrost Potion on the ground (F239). What an item IS belongs to the
// engine; this is that verdict, read-only.
// -1 is "no such live item unit", 0 a non-quest item, 1 a quest item. ITEMS_IsQuestItem reads only
// Items.txt, so pItemData is deliberately NOT required here -- a guard that answered -1 for an item
// the engine can still classify would read as "not quest" in the client and drop the potion again.
EMSCRIPTEN_KEEPALIVE int32_t d2_session_item_is_quest(uint32_t itemId) {
    if (!g_sessionLive || !g_game) return -1;
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId);
    if (!item || item->dwUnitType != UNIT_ITEM) return -1;
    return ITEMS_IsQuestItem(item) ? 1 : 0;
}
EMSCRIPTEN_KEEPALIVE const D2SessionVendorTransaction* d2_session_vendor_transaction() {
    return &g_sessionVendorTransaction;
}
EMSCRIPTEN_KEEPALIVE const D2SessionAct1Quest1* d2_session_act1q1() {
    memset(&g_sessionAct1Quest1, 0, sizeof g_sessionAct1Quest1);
    D2SessionAct1Quest1& out = g_sessionAct1Quest1;
    out.version = 1; out.size = sizeof out; out.level = g_currentLevel;
    if (!g_sessionLive || !g_game || !g_targetPlayer) return &out;
    D2QuestDataStrc* quest = QUESTS_GetQuestData(g_game, QUEST_A1Q1_DENOFEVIL);
    D2BitBufferStrc* flags = UNITS_GetPlayerData(g_targetPlayer)->pQuestData[g_game->nDifficulty];
    if (quest) {
        out.questState = quest->fState; out.lastState = quest->fLastState; out.questFlags = quest->dwFlags;
        if (D2Act1Quest1Strc* ex = static_cast<D2Act1Quest1Strc*>(quest->pQuestDataEx)) {
            out.monstersLeft = ex->nMonstersLeft; out.enteredDen = ex->bEnteredDen;
            out.akaraActivated = ex->bAkaraActivated; out.finished = ex->bFinished; out.timerActive = ex->bTimerActive;
        }
    }
    for (uint32_t bit = 0; bit < 16; ++bit)
        if (QUESTRECORD_GetQuestState(flags, QUESTSTATEFLAG_A1Q1, bit)) out.playerFlags |= 1u << bit;
    out.globalPrimaryGoal = QUESTS_GetGlobalState(g_game, QUESTSTATEFLAG_A1Q1, QFLAG_PRIMARYGOALDONE);
    out.skillPoints = STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_SKILLPTS, 0);
    out.hash = sessionHashBytes(D2VEC_FNV_INIT, &out, offsetof(D2SessionAct1Quest1, hash));
    return &out;
}

EMSCRIPTEN_KEEPALIVE const D2SessionAct1Quest6* d2_session_act1q6() {
    memset(&g_sessionAct1Quest6, 0, sizeof g_sessionAct1Quest6);
    D2SessionAct1Quest6& out = g_sessionAct1Quest6;
    out.version = 1; out.size = sizeof out; out.level = g_currentLevel; out.andarielGuid = -1;
    if (!g_sessionLive || !g_game || !g_targetPlayer) return &out;
    D2QuestDataStrc* quest = QUESTS_GetQuestData(g_game, QUEST_A1Q6_ANDARIEL);
    D2BitBufferStrc* flags = UNITS_GetPlayerData(g_targetPlayer)->pQuestData[g_game->nDifficulty];
    if (quest) {
        out.questState = quest->fState; out.lastState = quest->fLastState; out.questFlags = quest->dwFlags;
        out.killCallbackActive = quest->pfCallback[QUESTEVENT_MONSTERKILLED] != nullptr;
        if (D2Act1Quest6Strc* ex = static_cast<D2Act1Quest6Strc*>(quest->pQuestDataEx)) {
            out.andarielGuid = ex->nAndarielGUID; out.timerInvocations = ex->nTimerInvocations;
            out.andarielKilled = ex->bAndarielKilled;
        }
    }
    for (uint32_t bit = 0; bit < 16; ++bit)
        if (QUESTRECORD_GetQuestState(flags, QUESTSTATEFLAG_A1Q6, bit)) out.playerFlags |= 1u << bit;
    out.globalPrimaryGoal = QUESTS_GetGlobalState(g_game, QUESTSTATEFLAG_A1Q6, QFLAG_PRIMARYGOALDONE);
    out.primaryGoalDone = QUESTRECORD_GetQuestState(flags, QUESTSTATEFLAG_A1Q6, QFLAG_PRIMARYGOALDONE);
    out.rewardPending = QUESTRECORD_GetQuestState(flags, QUESTSTATEFLAG_A1Q6, QFLAG_REWARDPENDING);
    out.rewardGranted = QUESTRECORD_GetQuestState(flags, QUESTSTATEFLAG_A1Q6, QFLAG_REWARDGRANTED);
    out.completedNow = QUESTRECORD_GetQuestState(flags, QUESTSTATEFLAG_A1Q6, QFLAG_COMPLETEDNOW);
    out.hash = sessionHashBytes(D2VEC_FNV_INIT, &out, offsetof(D2SessionAct1Quest6, hash));
    return &out;
}

// The retail A1Q6 oracle's first slice observes the callback with no player argument.  Keep
// that exact surface here: progression/reward flags remain a subsequent player-record vector.
// Fixture: dispatch a quest's MONSTERKILLED callback for the first monster of a class in the
// session's rooms, with the target player as the killer (the generic form of the Andariel kill
// fixture below; Mephisto -> A3Q6 opens the Hell gate portal and sets the act-change flag).
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_quest_monster_killed(uint32_t questId, uint32_t monsterClass) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    updateSessionSnapshot();
    D2UnitStrc* monster = nullptr;
    for (const D2SessionUnit& unit : g_sessionUnits) {
        if (unit.type == UNIT_MONSTER && unit.classId == monsterClass) {
            monster = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)unit.id);
            break;
        }
    }
    D2QuestDataStrc* quest = QUESTS_GetQuestData(g_game, (int32_t)questId);
    if (!monster || !quest || !quest->pfCallback[QUESTEVENT_MONSTERKILLED]) return 0;
    D2QuestArgStrc args = {}; args.pGame = g_game; args.nEvent = QUESTEVENT_MONSTERKILLED;
    args.pTarget = monster; args.pPlayer = g_targetPlayer;
    quest->pfCallback[QUESTEVENT_MONSTERKILLED](quest, &args);
    updateSessionSnapshot();
    return 1;
}

// Fixture: place the player beside a unit by the recovered teleport placement (sub_6FCBDFE0
// with its free-coordinate search), so an operate or attack command is in range.
static int standNearUnit(D2UnitStrc* unit) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pDynamicPath || !unit) return 0;
    D2ActiveRoomStrc* room = UNITS_GetRoom(unit);
    D2CoordStrc at = {}; UNITS_GetCoords(unit, &at);
    if (!room) return 0;
    const int32_t size = UNITS_GetUnitSizeX(unit);
    for (int32_t reach = size; reach <= size + 3; ++reach) {
        for (int32_t dx = -reach; dx <= reach; ++dx) for (int32_t dy = -reach; dy <= reach; ++dy) {
            if (abs(dx) != reach && abs(dy) != reach) continue;
            if (!sub_6FCBDFE0(g_game, g_targetPlayer, room, at.nX + dx, at.nY + dy, 0, 0)) continue;
            g_sessionMovePending = false;
            d2_session_tick(1);
            const bool inRange = unit->dwUnitType == UNIT_OBJECT ? UNITS_IsObjectInInteractRange(g_targetPlayer, unit) != 0
                               : UNITS_GetDistanceToOtherUnit(g_targetPlayer, unit) <= 4;
            if (inRange) { updateSessionSnapshot(); return 1; }
        }
    }
    updateSessionSnapshot();
    return 0;
}
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_stand_near_object(uint32_t objectId) {
    if (!g_sessionLive || !g_game) return 0;
    return standNearUnit(SUNIT_GetServerUnit(g_game, UNIT_OBJECT, (int32_t)objectId));
}
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_stand_near_unit(uint32_t unitType, uint32_t unitId) {
    if (!g_sessionLive || !g_game) return 0;
    return standNearUnit(SUNIT_GetServerUnit(g_game, (int32_t)unitType, (int32_t)unitId));
}

// Fixture: kill a monster through the recovered kill routine (SUNITDMG_KillMonster: death mode,
// drops, experience, quest MONSTERKILLED events) with the target player as the killer.
//
// The last argument is `bPetKill`, and it is 1 here because that is what every real death in the
// engine passes: SkillDruid, Skills, SUnitDmg's own damage path, AiBaal, MonsterMode, AiThink and
// SUnit.cpp all call it with 1, and the ONE caller that passes 0 is PlayerPets' dismiss path, which
// has already done the pet bookkeeping itself. With 0 a killed HIRELING was never marked dead in its
// pet record (`sub_6FC7D470` -> `pPetData->nFlags |= 1`), which is the state retail's own resurrect
// reads: `D2GAME_NPC_ResurrectMerc_6FCC9350` -> `sub_6FCC84D0` looks the pet up with
// `sub_6FC7E8B0(pGame, pPlayer, 7, 0)`, whose `a4 = 0` means "the first pet that is NOT dead" --
// with the corpse still marked alive that returned the corpse, took the DISMISS branch
// (`SUNIT_RemoveUnit`) and then went on using the freed unit, which traps the wasm (measured, cycle
// 534: retail's 0x62 on a hireling killed this way is a memory-access-out-of-bounds). For an
// ordinary ownerless monster the flag changes nothing, which is why no other row moves.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_kill_monster(uint32_t unitId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    D2UnitStrc* monster = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)unitId);
    if (!monster || monster->dwAnimMode == MONMODE_DEATH || monster->dwAnimMode == MONMODE_DEAD) return 0;
    STATLIST_SetUnitStat(monster, STAT_HITPOINTS, 0, 0);
    SUNITDMG_KillMonster(g_game, monster, g_targetPlayer, 1);
    updateSessionSnapshot();
    return monster->dwAnimMode == MONMODE_DEATH || monster->dwAnimMode == MONMODE_DEAD ? 1 : 0;
}

// Fixture: give the player a quest item by code through the quest system's own item creation
// (QUESTS_CreateItem: placed in the inventory, never dropped). Returns the item's unit id or 0.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_fixture_quest_item(uint32_t code) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    D2UnitStrc* item = QUESTS_CreateItem(g_game, g_targetPlayer, code, 0, ITEMQUAL_NORMAL, 0);
    updateSessionSnapshot();
    return item ? item->dwUnitId : 0;
}

// Fixture: a quest item put on the floor at the player's own tile (retail's inventory-to-floor path: the item
// leaves the pack (INVENTORY_RemoveItemFromInventory), its invpage is cleared, then D2GAME_DropItem_6FC52260 at
// the free space D2GAME_GetFreeSpaceEx_6FC4BF00 returns, as ItemMode.cpp does for a cursor item it cannot place).
// Returns the item's unit id or 0. Call it before the pack is filled: the item is created in the pack first.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_fixture_ground_item(uint32_t code) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    D2UnitStrc* item = QUESTS_CreateItem(g_game, g_targetPlayer, code, 0, ITEMQUAL_NORMAL, 0);
    if (!item) return 0;
    D2UnitStrc* removed = INVENTORY_RemoveItemFromInventory(g_targetPlayer->pInventory, item);
    if (!removed) return 0;
    UNITS_ChangeAnimMode(item, IMODE_ONCURSOR);
    ITEMS_SetInvPage(item, INVPAGE_NULL);
    D2CoordStrc coords = {}, returnCoords = {};
    UNITS_GetCoords(g_targetPlayer, &coords);
    D2ActiveRoomStrc* room = D2GAME_GetFreeSpaceEx_6FC4BF00(UNITS_GetRoom(g_targetPlayer), &coords, &returnCoords, 1);
    if (!room) return 0;
    D2GAME_DropItem_6FC52260(g_game, g_targetPlayer, item, room, returnCoords.nX, returnCoords.nY);
    updateSessionSnapshot();
    return item->dwUnitId;
}

// Fixture: an item of a given quality (ITEMQUAL_*: 4 magic, 5 set, 6 rare, 7 unique) at the player's
// item level, rolled by retail's own item creation (QUESTS_CreateItem -> D2GAME_CreateItemEx), in the
// pack. The quality it came out as is d2_session_item_quality: retail may fall back (a base with no
// fitting unique becomes rare).
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_fixture_quality_item(uint32_t code, uint32_t quality) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || quality > 8) return 0;
    D2UnitStrc* item = QUESTS_CreateItem(g_game, g_targetPlayer, code, 0, (uint8_t)quality, 0);
    updateSessionSnapshot();
    return item ? item->dwUnitId : 0;
}
// The same at a named item level (the recorded fixture's ILVL): the level decides which uniques and sets can roll at all.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_fixture_quality_item_level(uint32_t code, uint32_t quality, int32_t ilvl) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || quality > 8) return 0;
    D2UnitStrc* item = QUESTS_CreateItem(g_game, g_targetPlayer, code, ilvl, (uint8_t)quality, 0);
    updateSessionSnapshot();
    return item ? item->dwUnitId : 0;
}
EMSCRIPTEN_KEEPALIVE int d2_session_item_quality(uint32_t itemId) {
    D2UnitStrc* item = (g_sessionLive && g_game) ? SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId) : nullptr;
    return item && item->pItemData ? (int)item->pItemData->dwQualityNo : -1;
}
// Item flags for the client's ground label colour (D2Client 0x58FB0: ethereal 0x400000 / socketed 0x800 -> grey).
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_item_flags(uint32_t itemId) {
    D2UnitStrc* item = (g_sessionLive && g_game) ? SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId) : nullptr;
    return item && item->pItemData ? (uint32_t)item->pItemData->dwItemFlags : 0;
}
// F114: the SetItems.txt row index an item carries (ITEMS_GetFileIndex) -- for a SET item, which set
// item the chooser (ItemsMagic sub_6FC542C0) picked. The set chooser and the file-index read are two
// readers no scenario reached; this is the one number `s885` needs to name the row it compares the
// item's boni against. -1 for an item with no file index (a normal/magic one carries -1).
EMSCRIPTEN_KEEPALIVE int d2_session_item_file_index(uint32_t itemId) {
    D2UnitStrc* item = (g_sessionLive && g_game) ? SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId) : nullptr;
    return item ? ITEMS_GetFileIndex(item) : -1;
}
// What an item IS, by retail's own classifier: ITEMS_CheckItemTypeIdByItemId walks Items.txt's
// wType[0]/wType[1] through ItemTypes.txt's equivalence LUT, so "is this a weapon" is a column retail
// owns rather than a list of item codes anybody paraphrases. A client that needs it had none of it:
// the shared player armed a bare-handed hero by matching a hand-typed 3-letter code regex, which
// matched exactly 1 of Larzuk's 48 store rows at the act V hero's level (act5-bootstrap, F53), so a
// full store and a full purse could still come back "nothing affordable". Keyed by CLASS id, which is
// what d2_session_inventory_items and the item presentations already hand out. itemType is an
// ITEMTYPE_* code from D2Items.h (ITEMTYPE_WEAPON 45, ITEMTYPE_ARMOR 2, ITEMTYPE_ANY_ARMOR 47).
EMSCRIPTEN_KEEPALIVE int d2_session_item_class_is_type(uint32_t classId, int32_t itemType) {
    return ITEMS_CheckItemTypeIdByItemId((int)classId, itemType) ? 1 : 0;
}

// Client packet 0x19 {itemGUID}: lift a stored inventory item onto the cursor.
EMSCRIPTEN_KEEPALIVE int d2_session_command_item_to_cursor_packet(uint32_t itemId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return -1;
    uint8_t packet[5] = { 0x19 };
    memcpy(packet + 1, &itemId, 4);
    if (!sessionDispatchGate(0x19)) return -2;
    const int32_t result = D2GAME_PACKETCALLBACK_Rcv0x19_RemoveItemFromBuffer_6FC850C0(g_game, g_targetPlayer, packet, sizeof packet);
    updateSessionSnapshot();
    return result;
}

// Client packets on the cursor item, through the recovered handlers: 0x17 {itemGUID} (drop it on
// the ground), 0x1D {itemGUID, bodyLoc} (swap it with the equipped one), 0x23 {itemGUID, beltSlot}
// (put it in the belt), 0x1B {itemGUID, bodyLoc} (put a two-hander on, taking the opposite hand's item off).
EMSCRIPTEN_KEEPALIVE int d2_session_command_cursor_item_packet(uint32_t opcode, uint32_t itemId, uint32_t arg) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || (opcode != 0x17 && opcode != 0x1B && opcode != 0x1D && opcode != 0x23)) return -1;
    if (!sessionDispatchGate((uint8_t)opcode)) return -2;
    uint8_t packet[9] = { (uint8_t)opcode };
    memcpy(packet + 1, &itemId, 4); memcpy(packet + 5, &arg, 4);
    const int32_t result = opcode == 0x17
        ? D2GAME_PACKETCALLBACK_Rcv0x17_DropItemOnGround_6FC84E20(g_game, g_targetPlayer, packet, 5)
        : opcode == 0x1D
        ? D2GAME_PACKETCALLBACK_Rcv0x1D_SwapCursorItemWithBody_6FC85550(g_game, g_targetPlayer, packet, sizeof packet)
        : opcode == 0x1B
        ? D2GAME_PACKETCALLBACK_Rcv0x1B_Swap2HandedItem_6FC853C0(g_game, g_targetPlayer, packet, sizeof packet)
        : D2GAME_PACKETCALLBACK_Rcv0x23_ItemToBelt_6FC85E70(g_game, g_targetPlayer, packet, sizeof packet);
    updateSessionSnapshot();
    return result;
}

// Client packet 0x1A {itemGUID, bodyLoc}: equip the cursor item to a body slot.
EMSCRIPTEN_KEEPALIVE int d2_session_command_equip_packet(uint32_t itemId, uint32_t bodyLoc) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return -1;
    uint8_t packet[9] = { 0x1A };
    memcpy(packet + 1, &itemId, 4); packet[5] = (uint8_t)bodyLoc;
    if (!sessionDispatchGate(0x1A)) return -2;
    const int32_t result = D2GAME_PACKETCALLBACK_Rcv0x1A_EquipItem_6FC85280(g_game, g_targetPlayer, packet, sizeof packet);
    updateSessionSnapshot();
    return result;
}

// Client packet 0x18 {itemGUID, x, y, invPage}: put the cursor item into a grid (inventory 0,
// stash 4, cube 3) at cell x,y.
EMSCRIPTEN_KEEPALIVE int d2_session_command_insert_item_packet(uint32_t itemId, uint32_t x, uint32_t y, uint32_t page) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return -1;
    uint8_t packet[17] = { 0x18 };
    memcpy(packet + 1, &itemId, 4); memcpy(packet + 5, &x, 4); memcpy(packet + 9, &y, 4); memcpy(packet + 13, &page, 4);
    if (!sessionDispatchGate(0x18)) return -2;
    const int32_t result = D2GAME_PACKETCALLBACK_Rcv0x18_InsertItemInBuffer_6FC84ED0(g_game, g_targetPlayer, packet, sizeof packet);
    updateSessionSnapshot();
    return result;
}

// A client packet through its recovered handler, for inputs no dedicated command covers yet:
// 0x1C unequip to the cursor, 0x24 lift a belt item to the cursor (the reverse of 0x63 below --
// M10, gameplay-shell.mjs's shift+click-on-belt already sends this; only the dispatch case was
// missing), 0x58 quest log viewed, 0x61 hand the CURSOR item to the mercenary, 0x63 shift-click to
// the belt (0x17 is d2_session_command_cursor_item_packet's). 0x61 is retail's
// own way a mercenary gets gear -- D2GAME_MERCS_EquipItem allocates the pet's inventory on the
// first item -- and it is what makes the save's mercenary item block reachable at all.
// Not the whole dispatch (D2GAME_PACKET_Handler_6FC89320): its table reaches party,
// player-list and warp code the headless link has no client layer for. -1 for any other header.
EMSCRIPTEN_KEEPALIVE int d2_session_client_packet(const uint8_t* bytes, int len) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !bytes || len <= 0 || len > 512) return -1;
    std::vector<uint8_t> packet(bytes, bytes + len);
    int32_t (__fastcall* handler)(D2GameStrc*, D2UnitStrc*, void*, int32_t) = nullptr;
    switch (packet[0]) {
    case 0x1C: handler = D2GAME_PACKETCALLBACK_Rcv0x1C_RemoveBodyItem_6FC854B0; break;
    // Cain's Identify Items: 0x34 {int32 npcGUID} (D2GAME_NPC_IdentifyAllItems) -- every unidentified item at once.
    // 0x2F {u8 unitType... } InitEntityChat: the client's "I am talking to this NPC" after the greeting; it takes the
    // interaction from 0 to 1 (sub_6FC61AB0), which D2GAME 0x19/0x1A item lifts need while a vendor is open.
    case 0x2F: if (len != 9) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x2F_InitEntityChat_6FC868C0; break;
    case 0x34: if (len != 5) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x34_IdentifyItemsWithNpc_6FC86BA0; break;
    case 0x3C: if (len >= 5) { uint32_t w; std::memcpy(&w, packet.data() + 1, 4); d2client_nova::selectSkill(static_cast<int32_t>(w)); } handler = D2GAME_PACKETCALLBACK_Rcv0x3C_SelectSkill_6FC87050; break;   // the client's selection (1.13c D2Client+0x73CB0 reads it)
    // Combine and socket, the game's own handlers on the client's bytes: 0x21 {itemA, itemB} stack one onto the other,
    // 0x22 {item} take one off a stack onto the cursor, 0x28 {cursor, socketable} put a gem/rune/jewel into a socket,
    // 0x29 {scroll, book} scroll into a tome, 0x2A {item, cube} an item into the cube without the cursor.
    case 0x21: if (len != 9) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x21_StackItems_6FC85C80; break;
    case 0x22: if (len != 5) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x22_UnstackItems_6FC85DA0; break;
    case 0x28: if (len != 9) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x28_SocketItem_6FC86310; break;
    case 0x29: if (len != 9) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x29_ScrollToBook_6FC86450; break;
    case 0x2A: if (len != 9) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x2A_ItemToCubeIndirect_6FC866E0; break;
    case 0x23: if (len != 9) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x23_ItemToBelt_6FC85E70; break;   // cursor item -> belt slot (checks IMODE_ONCURSOR and cursor ownership)
    case 0x24: if (len != 5) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x24_ItemFromBelt_6FC85F50; break;
    // 0x1F {cursorGUID, gridItemGUID, x, y}: the held item goes to x,y and the stored one it overlapped comes onto the cursor (page 1/2 refused by the handler).
    case 0x1F: if (len != 17) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x1F_SwapCursorBufferItems_6FC857C0; break;
    case 0x25: if (len != 9) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x25_SwitchBeltItem_6FC86040; break;   // cursor item <-> belt item
    case 0x48: if (len != 1) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x48_TurnOffBusyState_6FC87E20; break;   // the client's film ended: lifts the player's busy mark the film's quest branch set
    case 0x58: handler = D2GAME_PACKETCALLBACK_Rcv0x58_QuestCompleted_6FC883B0; break;
    case 0x51: if (len != 9) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x51_BindHotkeyToSkill_6FC88290; break;   // F-key assign: slot in the high half of the word, hand bit 15, owner GUID
    // a retail recording session replay: 0x0D right skill on unit, 0x3B add skill point, 0x3C select skill (recovered handlers).
    case 0x0D: handler = D2GAME_PACKETCALLBACK_Rcv0x0D_RightSkillOnUnit_6FC83F80; break;
    case 0x3B: handler = D2GAME_PACKETCALLBACK_Rcv0x3B_AddSkillPoint_6FC86EF0; break;
    case 0x61: if (len != 3) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x61_DropPickupMercItem_6FC88930; break;
    // Retail's own resurrect-a-dead-mercenary packet: 0x62 {int32 npcGUID}, five bytes, handled by
    // D2GAME_PACKETCALLBACK_Rcv0x62_ResurrectMerc_6FC86C00 -> D2GAME_NPC_ResurrectMerc_6FCC9350,
    // which requires that vendor to be the NPC the player is interacting with. Recovered and vendored
    // but with no caller anywhere in this tree, exactly as 0x61 was before it was wired (client-parity
    // P7: "resurrection at the same vendor" was unmeasured with no way to reach the handler at all).
    case 0x62: if (len != 5) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x62_ResurrectMerc_6FC86C00; break;
    case 0x63: handler = D2GAME_PACKETCALLBACK_Rcv0x63_ShiftLeftClickItemToBelt_6FC88F80; break;
    // What the packet client sends beyond the commands above: walk to a unit, the left hand's ground cast and held casts,
    // the body-slot swaps, the NPC's buy/sell/repair/hire/gamble-identify, and the town portal choice. Each is the game's own
    // handler on the client's bytes; sizes are the ones the client's packet builders write.
    case 0x02: if (len != 9) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x02_WalkToEntity_6FC82ED0; break;
    case 0x04: if (len != 9) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x04_RunToEntity_6FC830F0; break;
    case 0x05: if (len != 5) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x05_ShiftLeftClickSkill_6FC832B0; break;
    case 0x08: if (len != 5) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x08_ShiftLeftSkillHold_6FC83A10; break;
    case 0x09: if (len != 9) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x09_LeftSkillOnUnitHold_6FC83AC0; break;
    case 0x10: if (len != 9) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x10_RightSkillOnUnitHold_6FC84330; break;
    case 0x1B: if (len != 9) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x1B_Swap2HandedItem_6FC853C0; break;
    case 0x1D: if (len != 9) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x1D_SwapCursorItemWithBody_6FC85550; break;
    case 0x32: if (len != 17) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x32_BuyItemFromNpcBuffer_6FC86AE0; break;
    case 0x33: if (len != 17) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x33_SellItemToNpcBuffer_6FC86B30; break;
    case 0x35: if (len != 17) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x35_Repair_6FC86B70; break;
    case 0x36: if (len != 9) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x36_HireMerc_6FC86BD0; break;
    case 0x37: if (len != 5) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x37_IdentifyFromGamble_6FC86C30; break;
    case 0x45: if (len != 9) return -1; handler = D2GAME_PACKETCALLBACK_Rcv0x45_ChangeTpLocation_6FC87930; break;
    default: return -1;
    }
    if (!sessionDispatchGate(packet[0])) return -2;
    const int32_t result = handler(g_game, g_targetPlayer, packet.data(), len);
    updateSessionSnapshot();
    return result;
}
// A player stat as the game reads it (STATLIST_UnitGetStatValue: base plus every merged list, the
// equipped items' included). Read-only.
// The rooms the game populated, in order: {level, subtile x, subtile y, room seed lo, region dwUniqueCount,
// unk0x04, unk0x0C (all before population), room seed lo after, dwUniqueCount after, game frame} per
// room (game_glue's sub_6FC385A0 log). *count receives the number of entries.
extern int32_t g_d2ActivationLog[]; extern int32_t g_d2ActivationCount;
extern int32_t g_d2SpawnLog[]; extern int32_t g_d2SpawnCount;
extern int32_t g_d2CreateLog[]; extern int32_t g_d2CreateCount;
extern int32_t g_d2PopZeroLog[]; extern int32_t g_d2PopZeroCount;
extern int32_t g_d2SeqLog[]; extern int32_t g_d2SeqCount;
extern int32_t g_d2ObjectPopTrace[]; extern int32_t g_d2ObjectPopTraceCount;
extern int32_t g_d2ObjectPopTraceDropped;
extern int32_t g_d2ObjectPopTraceEnabled; extern int32_t g_d2ObjectPopTraceRoomOrdinal;
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_room_sequence(int32_t* count) { if (count) *count = g_d2SeqCount; return g_d2SeqLog; }
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_popzero_log(int32_t* count) { if (count) *count = g_d2PopZeroCount; return g_d2PopZeroLog; }
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_room_create_log(int32_t* count) { if (count) *count = g_d2CreateCount; return g_d2CreateLog; }
extern int32_t g_d2MissileBirths[]; extern int32_t g_d2MissileBirthCount;
// Engine missiles made since the last call: {unit id, class} pairs; the count is returned and the log cleared.
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_missile_births(int32_t* count) { *count = g_d2MissileBirthCount; g_d2MissileBirthCount = 0; return g_d2MissileBirths; }
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_spawn_log(int32_t* count) { if (count) *count = g_d2SpawnCount; return g_d2SpawnLog; }
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_activation_log(int32_t* count) { if (count) *count = g_d2ActivationCount; return g_d2ActivationLog; }
// Opt-in/resettable room-boundary trace, one row per class-125 object (or one sentinel row when absent).
// 18 int32 fields: ordinal, frame, level, roomX, roomY, roomSeedLoBefore, roomSeedHiBefore,
// roomSeedLoAfter, roomSeedHiAfter, objectSeedLoBefore, objectSeedHiBefore, objectSeedLoAfter,
// objectSeedHiAfter, unitId, objectX, objectY, mode, flags. Sentinel object fields are -1.
EMSCRIPTEN_KEEPALIVE void d2_session_object_pop_trace_control(int32_t enabled, int32_t reset) {
    if (reset) { g_d2ObjectPopTraceCount = 0; g_d2ObjectPopTraceDropped = 0; g_d2ObjectPopTraceRoomOrdinal = 0; }
    g_d2ObjectPopTraceEnabled = enabled != 0;
}
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_object_pop_trace(int32_t* count) {
    if (count) *count = g_d2ObjectPopTraceCount;
    return g_d2ObjectPopTrace;
}
EMSCRIPTEN_KEEPALIVE int32_t d2_session_object_pop_trace_dropped() { return g_d2ObjectPopTraceDropped; }
// MonStats2.txt pixHeight for a monster class (the NPC menu sits above it), -1 without a record.
EMSCRIPTEN_KEEPALIVE int32_t d2_monstats2_pixheight(int32_t classId) { const D2MonStats2Txt* t = UNITS_GetMonStats2TxtRecordFromMonsterId(classId); return t ? t->nPixHeight : -1; }
// MonStats2.txt dwModeFlags (+0xF0, the 16-bit mode-enabled bitmap D2Client 0x6C3F0 tests), -1 without a record.
EMSCRIPTEN_KEEPALIVE int32_t d2_monstats2_modeflags(int32_t classId) { const D2MonStats2Txt* t = UNITS_GetMonStats2TxtRecordFromMonsterId(classId); return t ? (int32_t)t->dwModeFlags : -1; }
// MonStats2.txt Light for a monster class, and a live monster unit's component-item light
// (MONSTERS_GetMaximalLightRadius, the 16-component Items.txt LightRadius scan D2Client's updater
// maxes with MonStats2 Light). Diagnostics for the monster light-radius row; nothing reads them.
EMSCRIPTEN_KEEPALIVE int32_t d2_monstats2_light(int32_t classId) { const D2MonStats2Txt* t = UNITS_GetMonStats2TxtRecordFromMonsterId(classId); return t ? (int32_t)t->nLight : -1; }
EMSCRIPTEN_KEEPALIVE int32_t d2_monster_item_light(uint32_t unitId) { D2UnitStrc* u = g_game ? SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)unitId) : nullptr; return u ? (int32_t)MONSTERS_GetMaximalLightRadius(u) : -1; }
// The inputs of SUNITDMG_ApplyDamageBonuses for the player's melee swing, as it reads them:
// [weapon found, wield type, STAT_DAMAGEPERCENT, weapon strength bonus, dexterity bonus, mastery
// bonus, the resulting damage percent before the -90 floor]. A fight that lands 1-hp hits names
// which term took the damage away.
static int32_t g_damageBonuses[8];   // [7] = one physical roll (x256), the player's seed restored
// One plain left-hand Attack (SKILLS_SrvDo001: hit flags |= 2, sub_6FCF5680, FillDamageValues(.., 0, 128),
// sub_6FCF5870, CalculateTotalDamage) of the player on a monster, forced to a successful hit, with the
// D2DamageStrc at each stage: [phys, enDmgPct, hitFlags, resultFlags] x {start, after 5680, after Fill,
// after 5870, after Total}, then [total, defender STAT_DAMAGERESIST, STAT_NORMAL_DAMAGE_REDUCTION, level,
// attacker mindamage, maxdamage]. The player's seed is restored: the trace draws no roll the game sees.
static int32_t g_damageTrace[26];
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_damage_trace(uint32_t monsterId) {
    memset(g_damageTrace, 0, sizeof g_damageTrace);
    if (!g_sessionLive || !g_game || !g_targetPlayer) return g_damageTrace;
    D2UnitStrc* target = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)monsterId);
    if (!target) return g_damageTrace;
    const D2SeedStrc seed = g_targetPlayer->pSeed; const D2SeedStrc tseed = target->pSeed;
    D2DamageStrc d = {}; d.wResultFlags = DAMAGERESULTFLAG_SUCCESSFULHIT; d.dwHitFlags |= 2;
    int k = 0; auto rec = [&]() { g_damageTrace[k++] = d.dwPhysDamage; g_damageTrace[k++] = d.dwEnDmgPct; g_damageTrace[k++] = (int32_t)d.dwHitFlags; g_damageTrace[k++] = d.wResultFlags; };
    rec(); sub_6FCF5680(g_targetPlayer, &d); rec();
    SUNITDMG_FillDamageValues(g_game, g_targetPlayer, target, &d, 0, 128); rec();
    sub_6FCF5870(g_targetPlayer, &d); rec();
    SUNITDMG_CalculateTotalDamage(g_game, g_targetPlayer, target, &d); rec();
    g_damageTrace[k++] = d.dwDmgTotal;
    g_damageTrace[k++] = STATLIST_UnitGetStatValue(target, STAT_DAMAGERESIST, 0);
    g_damageTrace[k++] = STATLIST_UnitGetStatValue(target, STAT_NORMAL_DAMAGE_REDUCTION, 0);
    g_damageTrace[k++] = STATLIST_UnitGetStatValue(target, STAT_LEVEL, 0);
    g_damageTrace[k++] = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_MINDAMAGE, 0);
    g_damageTrace[k++] = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_MAXDAMAGE, 0);
    g_targetPlayer->pSeed = seed; target->pSeed = tseed;
    return g_damageTrace;
}
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_damage_bonuses() {
    memset(g_damageBonuses, 0, sizeof g_damageBonuses);
    if (!g_sessionLive || !g_targetPlayer) return g_damageBonuses;
    D2UnitStrc* item = sub_6FC7C7B0(g_targetPlayer);
    g_damageBonuses[0] = item ? 1 : 0;
    g_damageBonuses[1] = g_targetPlayer->pInventory ? INVENTORY_GetWieldType(g_targetPlayer, g_targetPlayer->pInventory) : -1;
    g_damageBonuses[2] = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_DAMAGEPERCENT, 0);
    int32_t pct = g_damageBonuses[2];
    if (item) {
        g_damageBonuses[3] = ITEMS_GetStrengthBonus(item);
        g_damageBonuses[4] = ITEMS_GetDexBonus(item);
        g_damageBonuses[5] = SKILLS_GetWeaponMasteryBonus(g_targetPlayer, item, 0, 1);
        pct += g_damageBonuses[3] * STATLIST_UnitGetStatValue(g_targetPlayer, STAT_STRENGTH, 0) / 100
             + g_damageBonuses[4] * STATLIST_UnitGetStatValue(g_targetPlayer, STAT_DEXTERITY, 0) / 100 + g_damageBonuses[5];
    } else pct += STATLIST_UnitGetStatValue(g_targetPlayer, STAT_STRENGTH, 0);
    g_damageBonuses[6] = pct;
    const D2SeedStrc seed = g_targetPlayer->pSeed;
    g_damageBonuses[7] = SUNITDMG_ApplyDamageBonuses(g_targetPlayer, 1, nullptr, 0, 0, 0, 0, 128);
    g_targetPlayer->pSeed = seed;
    return g_damageBonuses;
}
// Any unit's stat (STATLIST_UnitGetStatValue), e.g. a monster's resistances; INT32_MIN if no unit.
EMSCRIPTEN_KEEPALIVE int32_t d2_session_unit_stat(int type, uint32_t id, uint32_t statId, uint32_t layer) {
    D2UnitStrc* u = g_game ? SUNIT_GetServerUnit(g_game, type, (int32_t)id) : nullptr;
    return u ? STATLIST_UnitGetStatValue(u, (int32_t)statId, (uint16_t)layer) : INT32_MIN;
}
// Fixture: write any unit's stat (a damaged monster for the hover life bar, s3401); 0 if no unit.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_set_unit_stat(int type, uint32_t id, uint32_t statId, int32_t value) {
    D2UnitStrc* u = g_game ? SUNIT_GetServerUnit(g_game, type, (int32_t)id) : nullptr;
    if (!u) return 0;
    STATLIST_SetUnitStat(u, (int32_t)statId, value, 0);
    return 1;
}
// D19: a unit's animation clock as the capture's O record reads it: [mode (+0x10), seq frame precise (+0x44), frame count precise (+0x48)].
static int32_t g_unitAnim[3];
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_unit_anim(int type, uint32_t id) {
    memset(g_unitAnim, 0, sizeof g_unitAnim);
    D2UnitStrc* u = g_game ? SUNIT_GetServerUnit(g_game, type, (int32_t)id) : nullptr;
    if (u) { g_unitAnim[0] = (int32_t)u->dwAnimMode; g_unitAnim[1] = u->nSeqCurrentFramePrecise; g_unitAnim[2] = u->dwFrameCountPrecise; }
    return g_unitAnim;
}
EMSCRIPTEN_KEEPALIVE int32_t d2_session_player_stat(uint32_t statId, uint32_t layer) {
    return (g_sessionLive && g_targetPlayer) ? STATLIST_UnitGetStatValue(g_targetPlayer, (int32_t)statId, (uint16_t)layer) : -1;
}
// The item on the player's cursor, or 0 (what a 0x1C/0x19 put there).
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_cursor_item() {
    D2UnitStrc* item = (g_sessionLive && g_targetPlayer && g_targetPlayer->pInventory) ? INVENTORY_GetCursorItem(g_targetPlayer->pInventory) : nullptr;
    return item ? item->dwUnitId : 0;
}
// The tests the item-box colour is chosen from, one bit each (play/assets/grid-highlight.mjs picks
// the colour; docs: the grid and body-slot box rules). 0 when the item is not found.
//   0x001 requirements not met for the player     0x002 item flag 0x4
//   0x004 a stat list carrying flag 2             0x008 the player is in state 54
//   0x010 the item cannot be equipped             0x020 a quest item not yet usable (ass/xyz/tr2)
//   0x040 shoots or quiver item with nothing in the other hand (body slots 4/5 only)
//   0x080 identified (flag 0x10)                  0x100 Items.txt transmogrify set
//   0x200 found
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_item_box_state(uint32_t itemId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId);
    D2ItemsTxt* t = item ? DATATBLS_GetItemsTxtRecord(item->dwClassId) : nullptr;
    if (!t) return 0;
    uint32_t out = 0x200;
    if (!ITEMS_CheckRequirements(item, g_targetPlayer, 0, nullptr, nullptr, nullptr)) out |= 0x001;
    if (ITEMS_CheckItemFlag(item, 0x4, __LINE__, __FILE__)) out |= 0x002;
    if (STATLIST_GetStatListFromUnitAndFlag(item, 2)) out |= 0x004;
    if (STATES_CheckState(g_targetPlayer, 54)) out |= 0x008;
    if (!ITEMS_CanBeEquipped(item)) out |= 0x010;
    if (t->nQuest) {
        D2BitBufferStrc* q = UNITS_GetPlayerData(g_targetPlayer)->pQuestData[g_game->nDifficulty];
        const uint32_t c = (uint32_t)(uint8_t)t->szCode[0] | (uint32_t)(uint8_t)t->szCode[1] << 8 | (uint32_t)(uint8_t)t->szCode[2] << 16 | (uint32_t)(uint8_t)t->szCode[3] << 24;
        bool red = false;
        if (q && c == ' ssa') red = !QUESTRECORD_GetQuestState(q, 9, 5);
        else if (q && c == ' zyx') red = !QUESTRECORD_GetQuestState(q, 20, 5);
        else if (q && c == ' 2rt') red = !QUESTRECORD_GetQuestState(q, 37, 8) || QUESTRECORD_GetQuestState(q, 37, 7);
        if (red) out |= 0x020;
    }
    const uint8_t loc = ITEMS_GetBodyLocation(item);
    D2UnitStrc* pair = (loc == BODYLOC_RARM || loc == BODYLOC_LARM) && g_targetPlayer->pInventory
        ? INVENTORY_GetItemFromBodyLoc(g_targetPlayer->pInventory, loc == BODYLOC_RARM ? BODYLOC_LARM : BODYLOC_RARM) : nullptr;
    if (!pair) { D2ItemTypesTxt* ty = DATATBLS_GetItemTypesTxtRecord((int)ITEMS_GetItemType(item)); if (ty && (ty->wShoots || ty->wQuiver)) out |= 0x040; }
    if (ITEMS_CheckItemFlag(item, 0x10, __LINE__, __FILE__)) out |= 0x080;
    if (t->nTransmogrify) out |= 0x100;
    return out;
}
// Fixture: the waypoint of a level activated, as touching it earlier in the playthrough left it.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_waypoint(int32_t levelId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    short wp = 0;
    D2WaypointDataStrc* data = UNITS_GetPlayerData(g_targetPlayer)->pWaypointData[g_game->nDifficulty];
    if (!data || !WAYPOINTS_GetWaypointNoFromLevelId(levelId, &wp)) return 0;
    WAYPOINTS_ActivateWaypoint(data, (uint16_t)wp);
    return WAYPOINTS_IsActivated(data, (uint16_t)wp) ? 1 : 0;
}

// Client packet 0x44 {playerGUID, objectGUID, itemGUID, state}: the staff-in-orifice (and
// obelisk) exchange; state 3 offers the cursor item to the object the player is interacting with.
EMSCRIPTEN_KEEPALIVE int d2_session_command_item_to_object_packet(uint32_t objectId, uint32_t itemId, uint32_t state) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return -1;
    D2GSPacketClt44 packet = {};
    packet.nHeader = 0x44; packet.nPlayerGUID = (int32_t)g_targetPlayer->dwUnitId;
    packet.nAnvilInteractGUID = (int32_t)objectId; packet.nItemGUID = (int32_t)itemId; packet.nState = (int16_t)state;
    if (!sessionDispatchGate(0x44)) return -2;
    const int32_t result = D2GAME_PACKETCALLBACK_Rcv0x44_StaffInOrifice_6FC87780(g_game, g_targetPlayer, &packet, sizeof packet);
    updateSessionSnapshot();
    return result;
}

// Fixture: prime Baal's throne AI so its wave sequence can begin. The throne Baal is present in
// the Throne of Destruction at runtime class 543 (the loaded MonStats offsets baalthrone by one
// from the reference enum's 544); its wave-spawn (AITHINK_Fn134_BaalThrone) advances only when
// AI-control param 1 bit 0 is set (retail restores it via SUNITINACTIVE_RestoreSpecialMonsterParameters).
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_prime_throne_baal() {
    if (!g_sessionLive || !g_game || !g_lastLevel) return 0;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
        if (!roomEx->pRoom) continue;
        for (D2UnitStrc* u = roomEx->pRoom->pUnitFirst; u; u = u->pRoomNext) {
            if (u->dwUnitType == UNIT_MONSTER && (u->dwClassId == 543 || u->dwClassId == MONSTER_BAALTHRONE)) {
                AIGENERAL_SetAiControlParam(u, 1, 1);
                updateSessionSnapshot();
                return (int)u->dwUnitId;
            }
        }
    }
    return 0;
}

// Fixture: open Anya's portal to Nihlathak's Temple (ACT5Q4_AnyaOpenPortal), the portal Drehya
// spawns in Harrogath after her rescue. The waypoint-not-activated precondition (which the town
// and waypoint state would set) is supplied so the recovered routine runs; it creates the portal.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_anya_open_portal() {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    D2QuestDataStrc* quest = QUESTS_GetQuestData(g_game, QUEST_A5Q4_NIHLATHAK);
    if (!quest || !quest->pQuestDataEx) return 0;
    static_cast<D2Act5Quest4Strc*>(quest->pQuestDataEx)->bWaypointNotActivated = 1;
    ACT5Q4_AnyaOpenPortal(g_game, g_targetPlayer);
    updateSessionSnapshot();
    return 1;
}

// Fixture: open the Worldstone Chamber (ACT5Q6_OpenWorldStoneChamber), the state Baal's throne
// AI sets when its stair form reaches the portal; the throne portal (563) then warps to it.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_open_worldstone() {
    if (!g_sessionLive || !g_game) return 0;
    ACT5Q6_OpenWorldStoneChamber(g_game);
    updateSessionSnapshot();
    return 1;
}

EMSCRIPTEN_KEEPALIVE int d2_session_fixture_kill_andariel() {
    if (!g_sessionLive || !g_game) return 0;
    D2UnitStrc* andariel = nullptr;
    for (const D2SessionUnit& unit : g_sessionUnits) {
        if (unit.type == UNIT_MONSTER && unit.classId == 156) {
            andariel = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)unit.id);
            break;
        }
    }
    D2QuestDataStrc* quest = QUESTS_GetQuestData(g_game, QUEST_A1Q6_ANDARIEL);
    if (!andariel || !quest || !quest->pfCallback[QUESTEVENT_MONSTERKILLED]) return 0;
    D2QuestArgStrc args = {}; args.pGame = g_game; args.nEvent = QUESTEVENT_MONSTERKILLED; args.pTarget = andariel;
    quest->pfCallback[QUESTEVENT_MONSTERKILLED](quest, &args);
    updateSessionSnapshot();
    return 1;
}

// Retail's player-attached kill path sets A1Q6's reward-pending and primary-goal bits before
// scheduling its timer. Keep it separately callable so the no-player global-state vector stays
// an exact, independent oracle.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_kill_andariel_for_player() {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    D2UnitStrc* andariel = nullptr;
    for (const D2SessionUnit& unit : g_sessionUnits) {
        if (unit.type == UNIT_MONSTER && unit.classId == 156) {
            andariel = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)unit.id);
            break;
        }
    }
    D2QuestDataStrc* quest = QUESTS_GetQuestData(g_game, QUEST_A1Q6_ANDARIEL);
    if (!andariel || !quest || !quest->pfCallback[QUESTEVENT_MONSTERKILLED]) return 0;
    D2QuestArgStrc args = {}; args.pGame = g_game; args.nEvent = QUESTEVENT_MONSTERKILLED;
    args.pTarget = andariel; args.pPlayer = g_targetPlayer;
    // Level population already leaves items in Catacombs 4; the quest drop is what the kill adds.
    g_sessionAct1Quest6ItemsBefore.clear();
    for (const D2SessionUnit& unit : g_sessionUnits) if (unit.type == UNIT_ITEM) g_sessionAct1Quest6ItemsBefore.push_back(unit.id);
    quest->pfCallback[QUESTEVENT_MONSTERKILLED](quest, &args);
    updateSessionSnapshot();
    return 1;
}

// The player-attached kill rolls two chipped gems and one normal gem off the global quest seed
// and drops them at Andariel.  Report every item the kill added, in the same room/unit order
// and with the same ITEMS_Serialize bitstream the retail capture recorded.
EMSCRIPTEN_KEEPALIVE const D2SessionAct1Quest6Drop* d2_session_a1q6_drops() {
    g_sessionAct1Quest6Drops.clear();
    if (!g_sessionLive || !g_game || !g_lastLevel) return nullptr;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
        if (!roomEx->pRoom) continue;
        for (D2UnitStrc* item = roomEx->pRoom->pUnitFirst; item; item = item->pRoomNext) {
            if (item->dwUnitType != UNIT_ITEM) continue;
            if (std::find(g_sessionAct1Quest6ItemsBefore.begin(), g_sessionAct1Quest6ItemsBefore.end(), (uint32_t)item->dwUnitId) != g_sessionAct1Quest6ItemsBefore.end()) continue;
            D2SessionAct1Quest6Drop out = {};
            out.itemId = item->dwUnitId; out.classId = item->dwClassId;
            out.quality = (uint32_t)ITEMS_GetItemQuality(item); out.ilvl = (uint32_t)ITEMS_GetItemLevel(item);
            if (item->pStaticPath) { out.x = (uint16_t)item->pStaticPath->tGameCoords.nX; out.y = (uint16_t)item->pStaticPath->tGameCoords.nY; }
            out.seedLo = item->pSeed.nLowSeed; out.seedHi = item->pSeed.nHighSeed;
            uint8_t bits[512] = {};
            out.bitBytes = (uint32_t)ITEMS_SerializeItemToBitstream(item, bits, sizeof bits, 1, 1, 0);
            memcpy(out.bits, bits, out.bitBytes < sizeof out.bits ? out.bitBytes : sizeof out.bits);
            g_sessionAct1Quest6Drops.push_back(out);
        }
    }
    for (size_t i = 0; i < g_sessionAct1Quest6Drops.size(); ++i) {
        D2SessionAct1Quest6Drop& out = g_sessionAct1Quest6Drops[i];
        out.index = (uint32_t)i; out.count = (uint32_t)g_sessionAct1Quest6Drops.size();
        // The retail record's hash covers its packed 76-byte prefix; recompute it over that layout.
        uint8_t packed[76] = {};
        uint32_t seed = g_game->dwInitSeed; uint16_t level = (uint16_t)g_currentLevel;
        uint8_t difficulty = g_game->nDifficulty;
        memcpy(packed + 0, &seed, 4); packed[4] = difficulty; packed[5] = 0; memcpy(packed + 6, &level, 2);
        memcpy(packed + 8, &out.itemId, 4); memcpy(packed + 12, &out.classId, 4); memcpy(packed + 16, &out.quality, 4); memcpy(packed + 20, &out.ilvl, 4);
        uint16_t x = (uint16_t)out.x, y = (uint16_t)out.y, index = (uint16_t)out.index, count = (uint16_t)out.count;
        memcpy(packed + 24, &x, 2); memcpy(packed + 26, &y, 2); memcpy(packed + 28, &index, 2); memcpy(packed + 30, &count, 2);
        memcpy(packed + 32, &out.seedLo, 4); memcpy(packed + 36, &out.seedHi, 4); memcpy(packed + 40, &out.bitBytes, 4);
        memcpy(packed + 44, out.bits, 32);
        out.hash = sessionHashBytes(D2VEC_FNV_INIT, packed, sizeof packed);
    }
    return g_sessionAct1Quest6Drops.data();
}
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_a1q6_drop_count() { return (uint32_t)g_sessionAct1Quest6Drops.size(); }

// Replay the live scheduled A1Q6 timer, rather than a browser-owned countdown. At invocation
// 10 its recovered callback opens the normal class-59 Catacombs return portal for the player.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_a1q6_timer_to_portal() {
    memset(&g_sessionAct1Quest6Timer, 0, sizeof g_sessionAct1Quest6Timer);
    D2SessionAct1Quest6Timer& out = g_sessionAct1Quest6Timer;
    out.version = 1; out.size = sizeof out; out.level = g_currentLevel;
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_game->pQuestControl) return 0;
    D2QuestTimerStrc* timer = g_game->pQuestControl->pTimer;
    D2QuestDataStrc* quest = QUESTS_GetQuestData(g_game, QUEST_A1Q6_ANDARIEL);
    if (!timer || !quest || timer->pQuest != quest || !timer->pfUpdate) return 0;
    D2Act1Quest6Strc* ex = static_cast<D2Act1Quest6Strc*>(quest->pQuestDataEx);
    if (!ex || ex->nTimerInvocations != 1) return 0;
    for (int i = 0; i < 9; ++i) timer->pfUpdate(g_game, quest);
    out.andarielId = (uint32_t)ex->nAndarielGUID;
    out.timerInvocations = ex->nTimerInvocations;
    out.timerStillScheduled = g_game->pQuestControl->pTimer != nullptr;
    updateSessionWorld();
    for (const D2SessionObjectPresentation& object : g_sessionObjectPresentations) {
        if (object.classId != 59) continue;
        ++out.portalCount; out.portalId = object.id; out.portalClass = object.classId;
    }
    out.hash = sessionHashBytes(D2VEC_FNV_INIT, &out, offsetof(D2SessionAct1Quest6Timer, hash));
    return out.timerInvocations == 10 && out.portalCount == 1;
}
EMSCRIPTEN_KEEPALIVE const D2SessionAct1Quest6Timer* d2_session_act1q6_timer() { return &g_sessionAct1Quest6Timer; }

// Invocations 11-12 of the same live timer: retail's status cycle sets lastState 3, clears the
// quest flag byte, composes the player's 0x5D status packet, and releases the timer (return 1).
// The callback is driven directly, as the capture does, so the return value is the release.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_a1q6_timer_to_completion() {
    memset(&g_sessionAct1Quest6Complete, 0, sizeof g_sessionAct1Quest6Complete);
    D2SessionAct1Quest6Complete& out = g_sessionAct1Quest6Complete;
    out.version = 1; out.size = sizeof out; out.level = g_currentLevel;
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_game->pQuestControl) return 0;
    D2QuestTimerStrc* timer = g_game->pQuestControl->pTimer;
    D2QuestDataStrc* quest = QUESTS_GetQuestData(g_game, QUEST_A1Q6_ANDARIEL);
    if (!timer || !quest || timer->pQuest != quest || !timer->pfUpdate) return 0;
    D2Act1Quest6Strc* ex = static_cast<D2Act1Quest6Strc*>(quest->pQuestDataEx);
    if (!ex || ex->nTimerInvocations != 10) return 0;
    D2WEB_ResetLastQuestStatusPacket();
    timer->pfUpdate(g_game, quest);
    out.timerReleased = timer->pfUpdate(g_game, quest);
    out.timerInvocations = ex->nTimerInvocations;
    out.lastState = quest->fLastState; out.state = quest->fState; out.questFlagsLow = quest->dwFlags & 0xFF;
    D2BitBufferStrc* flags = UNITS_GetPlayerData(g_targetPlayer)->pQuestData[g_game->nDifficulty];
    for (uint32_t bit = 0; bit < 16; ++bit)
        if (QUESTRECORD_GetQuestState(flags, QUESTSTATEFLAG_A1Q6, bit)) out.playerFlags |= 1u << bit;
    D2GSPacketSrv5D packet = {};
    if (const int count = D2WEB_CopyLastQuestStatusPacket(&packet)) {
        out.packetHeader = packet.nHeader; out.packetQuestId = packet.nQuestId; out.packetFlags = packet.nFlags;
        out.packetState = packet.nState; out.packetField4 = packet.field_4; out.packetBytes = count * sizeof packet;
    }
    out.hash = sessionHashBytes(D2VEC_FNV_INIT, &out, offsetof(D2SessionAct1Quest6Complete, hash));
    return out.timerInvocations == 12 && out.packetHeader == 0x5D;
}
EMSCRIPTEN_KEEPALIVE const D2SessionAct1Quest6Complete* d2_session_act1q6_complete() { return &g_sessionAct1Quest6Complete; }

// Player-defect check: a player whose A1Q6 record already says REWARDGRANTED (arm 1) or COMPLETEDBEFORE (arm 2)
// before the game is made. The record is preset on every difficulty, the quest list's join pass runs, and
// the kill takes the real dispatch (QUESTS_ParseKill with the player as killer), then 24 quest-updater
// ticks. The packed 40-byte record has the layout of the retail capture's A1Q6 defect kind.
#pragma pack(push, 1)
struct D2SessionA1Q6Defect {
    uint32_t seed; uint8_t difficulty, arm; uint16_t presetFlags;
    uint16_t flagsBeforeCycler, flagsAfterCycler;
    uint8_t notIntroBefore, activeBefore, notIntroAfterCycler, activeAfterCycler;
    uint8_t andarielListed, callbackRan, stateAfterKill, timerListNonEmptyAfterKill;
    uint16_t ticksAfterKill, ticksFinal;
    uint8_t updaterCalls, portalCount, timerListNonEmptyFinal, objectsAdded;
    uint32_t portalId; uint8_t firstPortalCall, listLength; uint16_t flagsFinal; uint32_t hash;
};
#pragma pack(pop)
static_assert(sizeof(D2SessionA1Q6Defect) == 40, "defect record layout");
static D2SessionA1Q6Defect g_sessionA1Q6Defect;
static uint16_t sessionA1Q6PlayerFlags() {
    D2BitBufferStrc* flags = UNITS_GetPlayerData(g_targetPlayer)->pQuestData[g_game->nDifficulty];
    uint16_t out = 0;
    for (uint32_t bit = 0; bit < 16; ++bit) if (QUESTRECORD_GetQuestState(flags, QUESTSTATEFLAG_A1Q6, bit)) out |= (uint16_t)(1u << bit);
    return out;
}
static void sessionA1Q6Portals(uint8_t* count, uint32_t* firstId) {
    updateSessionWorld();
    *count = 0;
    for (const D2SessionObjectPresentation& object : g_sessionObjectPresentations) {
        if (object.classId != 59) continue;
        if (!*count) *firstId = object.id;
        ++*count;
    }
}
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_a1q6_defect(int arm) {
    memset(&g_sessionA1Q6Defect, 0, sizeof g_sessionA1Q6Defect);
    static const uint16_t kPreset[3] = { 0x0000, 0x0001, 0x8000 };
    if (arm < 0 || arm > 2 || !g_sessionLive || !g_game || !g_targetPlayer || !g_game->pQuestControl) return 0;
    D2SessionA1Q6Defect& r = g_sessionA1Q6Defect;
    D2UnitStrc* andariel = nullptr;
    for (const D2SessionUnit& unit : g_sessionUnits) {
        if (unit.type == UNIT_MONSTER && unit.classId == 156) { andariel = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)unit.id); break; }
    }
    D2QuestDataStrc* quest = QUESTS_GetQuestData(g_game, QUEST_A1Q6_ANDARIEL);
    D2QuestInfoStrc* control = g_game->pQuestControl;
    if (!andariel || !quest || !quest->pQuestDataEx) return 0;
    r.seed = g_game->dwInitSeed; r.difficulty = g_game->nDifficulty; r.arm = (uint8_t)arm; r.presetFlags = kPreset[arm];
    for (int d = 0; d < 3; ++d) {
        D2BitBufferStrc* rec = UNITS_GetPlayerData(g_targetPlayer)->pQuestData[d];
        if (!rec || !rec->pBuffer) continue;
        rec->pBuffer[12] = (uint8_t)(r.presetFlags & 0xFF); rec->pBuffer[13] = (uint8_t)(r.presetFlags >> 8);
    }
    r.flagsBeforeCycler = sessionA1Q6PlayerFlags();
    r.notIntroBefore = quest->bNotIntro; r.activeBefore = quest->bActive;
    control->bPickedSet = 0;
    QUESTS_SequenceCycler(g_game, g_targetPlayer, 0);
    r.flagsAfterCycler = sessionA1Q6PlayerFlags();
    r.notIntroAfterCycler = quest->bNotIntro; r.activeAfterCycler = quest->bActive;
    uint8_t portals = 0; uint32_t portalId = 0;
    sessionA1Q6Portals(&portals, &portalId);
    const size_t objectsBefore = g_sessionObjectPresentations.size();
    for (D2QuestChainStrc* n = andariel->pQuestEventList; n; n = n->pNext) { ++r.listLength; if (n->pQuestData == quest) r.andarielListed = 1; }
    D2Act1Quest6Strc* ex = static_cast<D2Act1Quest6Strc*>(quest->pQuestDataEx);
    QUESTS_ParseKill(g_game, andariel, g_targetPlayer);
    r.callbackRan = ex->bAndarielKilled; r.stateAfterKill = quest->fState; r.ticksAfterKill = (uint16_t)ex->nTimerInvocations;
    r.timerListNonEmptyAfterKill = control->pTimer != nullptr;
    for (int i = 1; i <= 24; ++i) {
        QUESTS_QuestUpdater(g_game);
        r.updaterCalls = (uint8_t)i;
        uint8_t n = 0; uint32_t id = 0; sessionA1Q6Portals(&n, &id);
        if (n && !r.firstPortalCall) r.firstPortalCall = (uint8_t)i;
        if (n) { r.portalCount = n; r.portalId = id; }
    }
    r.ticksFinal = (uint16_t)ex->nTimerInvocations; r.timerListNonEmptyFinal = control->pTimer != nullptr;
    r.flagsFinal = sessionA1Q6PlayerFlags();
    r.objectsAdded = (uint8_t)(g_sessionObjectPresentations.size() - objectsBefore);
    r.hash = sessionHashBytes(D2VEC_FNV_INIT, &r, offsetof(D2SessionA1Q6Defect, hash));
    return 1;
}
EMSCRIPTEN_KEEPALIVE const D2SessionA1Q6Defect* d2_session_a1q6_defect() { return &g_sessionA1Q6Defect; }

// GAME_UpdateProgress's pending-rooms step (D2Game, D2MOO GAME/Game.cpp), run where retail runs it:
// at the start of a frame, before events and the client update. The client-room change
// (sub_6FC33670, below) is retail's activation wave -- it creates active rooms around the player
// and the DRLG raises the act's bHasPendingRoomsUpdates -- and THIS is what populates them:
// sub_6FC385A0 spawns a new room's presets, objects and monsters, or restores an old one's.
// Without it, rooms reached by walking stayed empty (s000: the Blood Moor held only the player).
// a retail recording: retail's recorded headless session never activates rooms around the moving fixture player (its game seed takes no
// population draws after tick 1), so a replay of it runs only the first pass; every other session leaves it on.
// That first pass is retail's tick-1 1.13c D2Game+0x2CBE0: one room-seed roll per active room (the wander gate,
// MonsterRegion.cpp sub_6FC679F0). Skipping it left every room seed one step behind (a retail recording hydra ring walk).
static int g_sessionReplayNoPopulate = 0;
static int g_sessionReplayPassesRun = 0;
EMSCRIPTEN_KEEPALIVE void d2_session_replay_no_populate(int on) { g_sessionReplayNoPopulate = on; g_sessionReplayPassesRun = 0; }
static void updateSessionPendingRooms() {
    if (g_sessionReplayNoPopulate && g_sessionReplayPassesRun) return;
    for (int act = 0; act < 5; ++act) {
        D2DrlgActStrc* pAct = g_game->pAct[act];
        if (!pAct || !pAct->bHasPendingRoomsUpdates) continue;
        ++g_d2PendingPassRuns; g_sessionReplayPassesRun = 1;
        for (D2ActiveRoomStrc* room = DUNGEON_GetRoomFromAct(pAct); room; room = room->pRoomNext) {
            ++g_d2PendingPassRooms;
            sub_6FC385A0(g_game, room);
        }
        pAct->bHasPendingRoomsUpdates = 0;
    }
}

// Diagnosis: the tick on which the player's hitpoints first reach 0, with what was around -- hp that
// reaches 0 without the death mode has been seen (s000a, the Underground Passage), and the combat
// trace held no hit on the player, so the damage came through another path.
// Every death overwrites it (the first-death-only record read as the LATEST death in a dossier), and
// it carries the player's last 32 frames -- frame:level:hp:x,y:mode -- so a death across a level
// change (a town portal taken mid-fight) shows which side of it the hp went.
static std::string g_hpWatch; static int32_t g_hpWatchLast = 1, g_hpWatchDeaths = 0;
struct HpWatchFrame { uint32_t frame; int32_t level, hp, x, y, mode; };
static HpWatchFrame g_hpWatchRing[32]; static uint32_t g_hpWatchRingN = 0;
static void watchPlayerHp() {
    const int32_t hp = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_HITPOINTS, 0);
    {
        D2ActiveRoomStrc* room = UNITS_GetRoom(g_targetPlayer);
        HpWatchFrame& f = g_hpWatchRing[g_hpWatchRingN++ % 32];
        f = { (uint32_t)g_game->dwGameFrame, room ? DUNGEON_GetLevelIdFromRoom(room) : -1, hp,
              g_targetPlayer->pDynamicPath->tGameCoords.wPosX, g_targetPlayer->pDynamicPath->tGameCoords.wPosY, (int32_t)g_targetPlayer->dwAnimMode };
    }
    // A player in the death/dead mode with hitpoints above 0 (s000a stood dead at 3194 hp for
    // 300,000 frames and every attack was refused): recorded once per death, beside the death.
    const bool deadMode = g_targetPlayer->dwAnimMode == PLRMODE_DEAD || g_targetPlayer->dwAnimMode == PLRMODE_DEATH;
    static bool s_deadAliveLogged = false;
    if (deadMode && hp > 0 && !s_deadAliveLogged) {
        char b[160];
        snprintf(b, sizeof b, " dead-alive frame %u hp %d mode %u level %d;", g_game->dwGameFrame, hp, g_targetPlayer->dwAnimMode,
                 UNITS_GetRoom(g_targetPlayer) ? DUNGEON_GetLevelIdFromRoom(UNITS_GetRoom(g_targetPlayer)) : -1);
        g_hpWatch += b; s_deadAliveLogged = true;
        g_hpWatch += " dead-alive frames";
        for (uint32_t i = g_hpWatchRingN > 32 ? g_hpWatchRingN - 32 : 0; i < g_hpWatchRingN; ++i) {
            const HpWatchFrame& f = g_hpWatchRing[i % 32];
            snprintf(b, sizeof b, " %u:%d:%d:%d,%d:%d", f.frame, f.level, f.hp, f.x, f.y, f.mode);
            g_hpWatch += b;
        }
        g_hpWatch += ";";
    }
    if (!deadMode) s_deadAliveLogged = false;
    if (hp <= 0 && g_hpWatchLast > 0) {
        char b[256];
        snprintf(b, sizeof b, "death %d frame %u hp %d->%d mode %u states unint %d poison %d;", ++g_hpWatchDeaths, g_game->dwGameFrame, g_hpWatchLast, hp, g_targetPlayer->dwAnimMode,
                 STATES_CheckState(g_targetPlayer, STATE_UNINTERRUPTABLE) ? 1 : 0, STATES_CheckState(g_targetPlayer, STATE_POISON) ? 1 : 0);
        g_hpWatch = b;
        g_hpWatch += " last frames";
        for (uint32_t i = g_hpWatchRingN > 32 ? g_hpWatchRingN - 32 : 0; i < g_hpWatchRingN; ++i) {
            const HpWatchFrame& f = g_hpWatchRing[i % 32];
            snprintf(b, sizeof b, " %u:%d:%d:%d,%d:%d", f.frame, f.level, f.hp, f.x, f.y, f.mode);
            g_hpWatch += b;
        }
        g_hpWatch += ";";
        const int px = g_targetPlayer->pDynamicPath->tGameCoords.wPosX, py = g_targetPlayer->pDynamicPath->tGameCoords.wPosY;
        for (D2DrlgRoomStrc* roomEx = g_lastLevel ? g_lastLevel->pFirstRoomEx : nullptr; roomEx; roomEx = roomEx->pDrlgRoomNext) {
            if (!roomEx->pRoom) continue;
            for (D2UnitStrc* u = roomEx->pRoom->pUnitFirst; u; u = u->pRoomNext) {
                if (u->dwUnitType != UNIT_MONSTER && u->dwUnitType != UNIT_MISSILE) continue;
                const int ux = u->pDynamicPath ? u->pDynamicPath->tGameCoords.wPosX : 0, uy = u->pDynamicPath ? u->pDynamicPath->tGameCoords.wPosY : 0;
                if (std::abs(ux - px) > 12 || std::abs(uy - py) > 12) continue;
                snprintf(b, sizeof b, " %s%u(mode %u, %d,%d)", u->dwUnitType == UNIT_MISSILE ? "missile" : "mon", u->dwClassId, u->dwAnimMode, ux - px, uy - py);
                g_hpWatch += b;
            }
        }
    }
    g_hpWatchLast = hp;
}
EMSCRIPTEN_KEEPALIVE const char* d2_session_hp_watch() { return g_hpWatch.c_str(); }

// Native 1.13c D2Client receives a cast mode from the skill packet and calls Common #11090. That
// setter resets the local frame clock only when the mode differs; a same-mode refresh is a no-op.
// The client then applies the player's mode-table
// gate each client frame. The server-side unit in this in-process session instead carries the game
// epoch that #6FCBCE70 writes for cast event timing. Mirror the client's cast clock for drawing and
// leave that authoritative server clock (and action-frame processing) alone.
// Retail client clock for the player's timed action modes (attack 1/2, throw, kick, cast). Each
// mode-set tick the client hero reads 0x200 (attack), 0x100 (throw, kick) or 0x100 (cast: seed 0
// plus the same tick's step), then +wAnimSpeed per tick (build-wasm session capture p71-swing, `FR C`).
static bool clientClockMode(uint32_t m) {
    return m == PLRMODE_CAST || m == PLRMODE_ATTACK1 || m == PLRMODE_ATTACK2 || m == PLRMODE_THROW || m == PLRMODE_KICK;
}
static void syncClientCastClock() {
    if (!g_targetPlayer || !clientClockMode(g_targetPlayer->dwAnimMode)) {
        g_clientCastClockActive = false;
        return;
    }
    if (g_clientCastClockActive && g_clientCastMode == g_targetPlayer->dwAnimMode) return;
    g_clientCastClockActive = true;
    g_clientCastMode = g_targetPlayer->dwAnimMode;
    // Native #11090's setup helper asks #6FD82220 for the player's start frame: 0 for cast, throw and
    // kick, one frame (0x100) for the two attack modes.
    g_clientCastFramePrecise = (g_clientCastMode == PLRMODE_ATTACK1 || g_clientCastMode == PLRMODE_ATTACK2) ? 0x100 : 0;
    g_clientCastAnimSpeed = g_targetPlayer->wAnimSpeed;
}

static void advanceClientCastClock() {
    syncClientCastClock();
    if (!g_clientCastClockActive || !g_targetPlayer) return;
    const uint32_t frameCount = g_targetPlayer->dwFrameCountPrecise;
    if (!frameCount) return;
    const int32_t speed = (int16_t)g_clientCastAnimSpeed;
    // Player mode 10 uses advance policy 1 in D2Client's table: #10426 gates #10853. When the next
    // step would reach the sequence end, the client skips #10853 and holds both frame and speed.
    // #10426's non-sequence predicate is `frame + signed speed >= frameCount`.
    const int64_t next = (int64_t)g_clientCastFramePrecise + speed;
    if (next < (int64_t)frameCount) {
        g_clientCastFramePrecise = (uint32_t)next;
    }
}

// Neutral mode: #11090's start frame for player mode 1 is 0 and the client's advance policy for it wraps
// (frame + speed, minus the sequence length once it reaches it). The tick that enters the mode draws frame 0.
static void syncClientNeutralClock() {
    if (!g_targetPlayer || g_targetPlayer->dwAnimMode != PLRMODE_NEUTRAL) { g_clientNeutralClockActive = false; return; }
    if (g_clientNeutralClockActive) return;
    g_clientNeutralClockActive = true;
    g_clientNeutralFramePrecise = 0;
    // The client unit's rate is computed with the unit's stats in place (other-anim-rate 100). The server unit
    // entered neutral when it was allocated, before its stats were set, so its own rate is the clamped 15%.
    // Recompute it for the client and leave the server unit's value as it was.
    const int16_t serverSpeed = g_targetPlayer->wAnimSpeed;
    UNITS_UpdateAnimRateAndVelocity(g_targetPlayer, __FILE__, __LINE__);
    g_clientNeutralAnimSpeed = (uint16_t)g_targetPlayer->wAnimSpeed;
    g_targetPlayer->wAnimSpeed = serverSpeed;
}

static void advanceClientNeutralClock() {
    const bool wasActive = g_clientNeutralClockActive;
    syncClientNeutralClock();
    if (!wasActive || !g_clientNeutralClockActive) return;
    const uint32_t frameCount = g_targetPlayer->dwFrameCountPrecise;
    if (!frameCount) return;
    uint32_t next = g_clientNeutralFramePrecise + g_clientNeutralAnimSpeed;
    while (next >= frameCount) next -= frameCount;
    g_clientNeutralFramePrecise = next;
}

// The player's rate and length for a mode it is not in: the mode-rate update run on the unit with the mode
// swapped in, and every field it writes put back.
static void clientModeRate(int32_t mode, uint16_t* speed, uint32_t* frameCount) {
    D2UnitStrc* u = g_targetPlayer;
    const uint32_t savedMode = u->dwAnimMode, savedSeq = u->dwSeqSpeed, savedCount = u->dwFrameCountPrecise;
    const int16_t savedSpeed = u->wAnimSpeed;
    const D2AnimDataRecordStrc* const savedAnim = u->pAnimData;
    u->dwAnimMode = (uint32_t)mode;
    // The client's rate update gets no shape-shift weapon-speed base (#11043 -> 0): retail draws the morph at the
    // anim data speed x attack rate (256 x 120% wolf, 256 x 100% bear) where the server unit uses the weapon base.
    static const D2Common33110Env kClientEnv = [] { D2Common33110Env e = UNITS_AnimRateEnv(); e.meleeSpeed = [](D2UnitStrc*) -> int { return 0; }; return e; }();
    D2Common_33110_UpdateAnimRate(u, kClientEnv);
    *speed = (uint16_t)u->wAnimSpeed;
    *frameCount = u->pAnimData ? u->pAnimData->dwFrames << 8 : 0;
    u->dwAnimMode = savedMode; u->dwSeqSpeed = savedSeq; u->dwFrameCountPrecise = savedCount;
    u->wAnimSpeed = savedSpeed; u->pAnimData = savedAnim;
}

// Turn the client unit toward `dir` from wherever it faces now (the server facing when no client facing is held).
static void clientFaceToward(uint8_t dir) {
    const D2DynamicPathStrc* path = g_targetPlayer ? g_targetPlayer->pDynamicPath : nullptr;
    if (!path) return;
    if (!g_clientFacingActive) {
        g_clientFacing = {};
        g_clientFacing.nDirection = path->nDirection;
        g_clientFacing.nNewDirection = path->nDirection;
        g_clientFacingActive = true;
    }
    g_clientFacingServerTarget = path->nNewDirection;
    D2COMMON_10160_PathUpdateDirection(&g_clientFacing, dir);
}

// Per client frame, after the cast and neutral clocks: the shapeshift entry and the client facing step.
static void advanceClientForm() {
    if (!g_targetPlayer) return;
    const bool formState = STATES_CheckState(g_targetPlayer, STATE_WOLF) || STATES_CheckState(g_targetPlayer, STATE_BEAR);
    const bool entered = formState && !g_clientMorphStatePrev;
    g_clientMorphStatePrev = formState;
    if (!formState) g_clientMorphActive = false;
    if (entered) {
        // The state-on callback: S1 from frame 0 (advanced below on this same frame), and a turn to direction 0.
        clientModeRate(PLRMODE_SPECIAL1, &g_clientMorphAnimSpeed, &g_clientMorphFrameCount);
        g_clientMorphActive = g_clientMorphFrameCount != 0;
        g_clientMorphFramePrecise = 0;
        clientFaceToward(0);
    }
    if (g_clientMorphActive) {
        // S1 ends when its clock reaches the last frame; the unit then stands in neutral from frame 0.
        const uint32_t next = g_clientMorphFramePrecise + g_clientMorphAnimSpeed;
        if (next + 256 >= g_clientMorphFrameCount) {
            g_clientMorphActive = false;
            g_clientNeutralClockActive = false;
            syncClientNeutralClock();
        } else {
            g_clientMorphFramePrecise = next;
        }
    }
    if (g_clientFacingActive) {
        const D2DynamicPathStrc* path = g_targetPlayer->pDynamicPath;
        if (!path || path->nNewDirection != g_clientFacingServerTarget) g_clientFacingActive = false;
        else D2COMMON_10193_PATH_AdjustDirection(&g_clientFacing);
    }
}

// The commands the approach-walk resume below re-issues (defined further down).
EMSCRIPTEN_KEEPALIVE int d2_session_command_primary_attack(uint32_t targetUnitId);
EMSCRIPTEN_KEEPALIVE int d2_session_command_right_skill(uint32_t targetUnitId);
EMSCRIPTEN_KEEPALIVE int d2_session_command_left_skill(uint32_t targetUnitId);

extern "C" int d2_client_monster_turn_held(const D2UnitStrc* unit);
// dup of D2Client+0x440C0 (from the frame tail 0x44A80): each client room the pass has not seen yet is marked seen;
// a Den of Evil room (level 8) gets 0x53550's light spots: up to 25 tries of a random subtile in the room
// (#10561 room coords, two hero-seed draws), each free one (#10482 mask 5 clear) sent, until 3 are free.
// The spots themselves (0xB30F0, a client-only unit) are not made here; only their draws are.
// The session has no client Act: the client's rooms are the server Act's active rooms, plus the rooms of another
// level the client's activation ring reaches through a warp (the server leaves those untiled). Retail's client builds
// one of those per 7 frames from its 7th frame. Unbuilt here, their collision is
// not held, so every spot in one reads as blocked (retail: 25 tries, 50 draws).
static void sessionClientRoomSpots(D2DrlgRoomStrc* room, D2ActiveRoomStrc* active) {
    D2DrlgCoordsStrc c{};
    if (active) DUNGEON_GetRoomCoordinates(active, &c);
    else { c.nSubtileX = room->nTileXPos * 5; c.nSubtileY = room->nTileYPos * 5; c.nSubtileWidth = room->nTileWidth * 5; c.nSubtileHeight = room->nTileHeight * 5; }
    auto rnd = [](int32_t n) { return n < 1 ? 0u : d2_session_client_rand(uint32_t(n)); };
    for (int i = 0, ok = 0; i <= 0x18 && ok < 3; ++i) {
        const int32_t x = int32_t(rnd(c.nSubtileWidth)) + c.nSubtileX;
        const int32_t y = c.nSubtileY + int32_t(rnd(c.nSubtileHeight));
        if (active && COLLISION_CheckMask(active, x, y, 5) == 0) ++ok;
    }
}
static void sessionClientRoomPass() {
    D2DrlgActStrc* act = g_sessionClientActOwned ? d2_drlg_client_act() : (g_game && g_currentAct >= 0 ? g_game->pAct[g_currentAct] : nullptr);
    if (!act) return;
    D2DrlgRoomStrc* warpRoom = nullptr;
    std::unordered_set<D2DrlgLevelStrc*> activeLevels;   // a level with an active room is reached by adjacency, not a warp
    for (D2ActiveRoomStrc* room = DUNGEON_GetRoomFromAct(act); room; room = room->pRoomNext)
        if (room->pDrlgRoom) activeLevels.insert(room->pDrlgRoom->pLevel);
    for (D2ActiveRoomStrc* room = DUNGEON_GetRoomFromAct(act); room; room = room->pRoomNext) {
        D2DrlgRoomStrc* dr = room->pDrlgRoom;
        if (!dr || !dr->pLevel) continue;
        if (!g_sessionClientActOwned && !warpRoom)
            for (int q = 0; q < dr->nRoomsNear; ++q) {
                D2DrlgRoomStrc* nr = dr->ppRoomsNear[q];
                if (nr && nr->pLevel && nr->fRoomStatus == ROOMSTATUS_UNTILE && !activeLevels.count(nr->pLevel) && !nr->pRoom && !g_sessionClientRoomsSeen.count(nr)) { warpRoom = nr; break; }
            }
        if (!g_sessionClientRoomsSeen.insert(dr).second) continue;
        if (dr->pLevel->nLevelId == 8) sessionClientRoomSpots(dr, room);
    }
    if (warpRoom && g_sessionClientFrame >= 7 && g_sessionClientFrame % 7 == 0) {
        g_sessionClientRoomsSeen.insert(warpRoom);
        if (warpRoom->pLevel->nLevelId == 8) sessionClientRoomSpots(warpRoom, nullptr);
    }
}
// The client frame's pass over the monsters it holds, in its unit table's order
// (bucket id & 0x7F, newest first), after the room pass; the idle sound draws from the client hero seed here, so it
// runs in the frame, not in the shell. The shell takes each unit's row to play it.
extern "C" uint32_t d2_client_monster_idle(uint32_t id, int32_t cls, uint32_t mode, uint32_t now);
static std::unordered_map<uint32_t, uint32_t> g_sessionIdleRows, g_sessionIdleRowsReady;
// The held monsters' modes as the client last saw them: a change into GetHit/knockback (3, 13) or death (0, 12)
// reaches the struck-unit sound, which stamps the unit before the frame's attack rolls.
extern "C" void d2_client_monster_hit_stamp(uint32_t id, uint32_t frame);
static std::unordered_map<uint32_t, uint32_t> g_sessionClientMonsterMode;
static void sessionClientMonsterModeReset() { g_sessionClientMonsterMode.clear(); g_sessionIdleRows.clear(); g_sessionIdleRowsReady.clear(); }
static void sessionClientMonsterHitPass(uint32_t frame) {
    std::unordered_map<uint32_t, uint32_t> now;
    for (uint32_t id : g_heldMonsters) {
        D2UnitStrc* u = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)id);
        if (!u) continue;
        const uint32_t m = u->dwAnimMode;
        now[id] = m;
        auto it = g_sessionClientMonsterMode.find(id);
        if (it != g_sessionClientMonsterMode.end() && it->second != m && (m == 3 || m == 13 || m == 0 || m == 12)) d2_client_monster_hit_stamp(id, frame);
    }
    g_sessionClientMonsterMode.swap(now);
}
static void sessionClientMonsterIdlePass() {
    std::vector<uint32_t> ids(g_heldMonsters.begin(), g_heldMonsters.end());
    std::sort(ids.begin(), ids.end(), [](uint32_t a, uint32_t b) { return (a & 127) != (b & 127) ? (a & 127) < (b & 127) : a > b; });
    for (uint32_t id : ids) {
        D2UnitStrc* u = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)id);
        if (!u) continue;
        const uint32_t mode = u->dwAnimMode == 14 && u->pAnimSeq ? (u->dwSeqMode & 0xF) : u->dwAnimMode;
        if (const uint32_t row = d2_client_monster_idle(id, (int32_t)u->dwClassId, mode, g_sessionClientFrame)) g_sessionIdleRows[id] = row;
    }
}
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_monster_idle_take(uint32_t id) {
    auto it = g_sessionIdleRowsReady.find(id);
    if (it == g_sessionIdleRowsReady.end()) return 0;
    const uint32_t row = it->second; g_sessionIdleRowsReady.erase(it); return row;
}
// Retail holds a right-skill click made while the unit is mid attack/cast/throw and casts it when the mode ends; the
// stsound plays only when the cast mode starts, so a click the unit cannot act on yet plays nothing (D60).
struct PendingRightSkill { bool set; int32_t x, y; };
static PendingRightSkill g_pendingRightSkill = {};
static int sessionCommandRightSkillAt(int32_t x, int32_t y, bool held);
static bool sessionPlayerInActionMode() {
    const uint32_t m = g_targetPlayer ? g_targetPlayer->dwAnimMode : 0;
    return m == PLRMODE_ATTACK1 || m == PLRMODE_ATTACK2 || m == PLRMODE_CAST || m == PLRMODE_THROW;
}
// A pickup the way retail's client makes it (D2Client 0x29E80 / 0x2AA40 / 0x2B090): out of melee reach the
// client walks to the item (packet 0x02) and keeps the pickup waiting; each frame, before the units move,
// it re-tests the reach and sends the pickup (0x16) once the item is in it. The server never walks for a
// bare far 0x16. Here the "client" is this function and the per-frame step below.
static uint32_t g_sessionClientPickup = 0;
EMSCRIPTEN_KEEPALIVE int d2_session_command_pickup(uint32_t itemUnitId);
static bool sessionClientPickupInReach(D2UnitStrc* item) {
    return item && item->dwAnimMode == IMODE_ONGROUND && UNITS_GetRoom(item) && UNITS_GetRoom(g_targetPlayer) &&
        UNITS_IsInMeleeRange(g_targetPlayer, item, UNITS_IsInMovingMode(item));
}
static void sessionClientPickupStep() {
    if (!g_sessionClientPickup) return;
    const uint32_t id = g_sessionClientPickup;
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)id);
    if (!item || item->dwAnimMode != IMODE_ONGROUND || !UNITS_GetRoom(item)) { g_sessionClientPickup = 0; return; }
    const uint32_t mode = g_targetPlayer->dwAnimMode;
    if (mode == PLRMODE_DEATH || mode == PLRMODE_DEAD) { g_sessionClientPickup = 0; return; }
    if (!sessionClientPickupInReach(item)) return;
    g_sessionClientPickup = 0;
    d2_session_command_pickup(id);
}
EMSCRIPTEN_KEEPALIVE int d2_session_client_pickup(uint32_t itemUnitId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pInventory) return 0;
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemUnitId);
    if (!item || item->dwAnimMode != IMODE_ONGROUND || !UNITS_GetRoom(item)) return 0;
    g_sessionClientPickup = 0;
    if (sessionClientPickupInReach(item)) return d2_session_command_pickup(itemUnitId);
    uint8_t walk[9] = { 0x02, UNIT_ITEM, 0, 0, 0 };
    std::memcpy(walk + 5, &itemUnitId, 4);
    if (d2_session_client_packet(walk, 9) < 0) return 0;
    g_sessionClientPickup = itemUnitId;
    return 2;
}
extern "C" __attribute__((weak)) void d2_local_d2gs_drain();
EMSCRIPTEN_KEEPALIVE int d2_session_tick(int frames) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pDynamicPath || frames < 0) return 0;
    // a retail recording: server->client packets a tick queues (NPC quest-marker 0x8A) are the tick's own stream.
    D2WEB_ResetPacketStream(); D2WEB_ResetHitPackets();
    struct InTick { InTick() { g_sessionInTick = true; } ~InTick() { g_sessionInTick = false; } } inTick;
    if (g_heldFor != g_targetPlayer) sessionHeldStep();   // a retail recording: the start set, before the first move
    struct HeldStep { ~HeldStep() { sessionHeldStep(); } } heldStep;   // the tick's near-list diff
    struct StreamRefresh { ~StreamRefresh() { D2WEB_PacketStream(&g_sessionPacketStreamLength, &g_sessionPacketStreamPackets); } } streamRefresh;
    g_sessionIdleRowsReady.swap(g_sessionIdleRows); g_sessionIdleRows.clear();   // a row drawn at a frame's end is the next tick's sound
    for (int i = 0; i < frames; ++i) {
        if (d2_local_d2gs_drain) d2_local_d2gs_drain();   // 1.13c D2Game+0x2E460: the frame starts by applying the client packets that arrived
        sessionClientPickupStep();
        if (g_pendingRightSkill.set && !sessionPlayerInActionMode()) { const PendingRightSkill q = g_pendingRightSkill; g_pendingRightSkill.set = false; sessionCommandRightSkillAt(q.x, q.y, false); }
        // the client sees the previous frame's server state (a death this frame reaches it by packet next tick)
        { extern void d2client_unitseed_flush(); extern void d2_client_monster_order_face_flush(); d2client_unitseed_flush(); d2_client_monster_order_face_flush(); }   // the previous frame's unit adds and orders, in send order
        d2client_clmissile::tick(g_targetPlayer, [](int32_t type, uint32_t id) { return SUNIT_GetServerUnit(g_game, type, (int32_t)id); });   // 1.13c D2Client+0xB17C0 (client missiles)
        // Advance the mirrored Fog pool lifetimes once per headless simulation frame. Native
        // D2Client updates these pools before its candidate pass, but this engine tick currently
        // has no corresponding weather candidate pass. Entries are world anchored here; camera
        // projection remains the renderer's responsibility.
        d2::weather::tick();
        updateSessionPendingRooms();
        const bool wasMoving = g_sessionMovePending;
        // Death is not interrupted: the session's own mode resets below (an attack that has run its
        // course, a deferred move) forced a dying player back to neutral at 0 hp -- alive, unkillable,
        // and never dead (s000a, the Underground Passage).
        const bool dying = g_targetPlayer->dwAnimMode == PLRMODE_DEATH || g_targetPlayer->dwAnimMode == PLRMODE_DEAD;
        if (dying) { g_sessionMovePending = false; if (g_sessionAttack.state == 2) g_sessionAttack.state = 3; }
        if (g_sessionAttack.state == 2 && !dying) {
            D2ActiveRoomStrc* room = g_targetPlayer->pDynamicPath->pRoom;
            SUNIT_SetCombatMode(g_game, g_targetPlayer,
                room && DUNGEON_IsRoomInTown(room) ? PLRMODE_TOWNNEUTRAL : PLRMODE_NEUTRAL);
            D2GAME_DeletePlayerPerFrameEvents_6FCBCE50(g_game, g_targetPlayer);
            UNITS_SetUsedSkill(g_targetPlayer, nullptr); UNITS_SetTargetUnitForDynamicUnit(g_targetPlayer, nullptr);
            g_sessionAttack.state = 3;
        }
        // Retail advances a unit along its dynamic path only while its animation is a moving mode:
        // #10141 PATH_GetClientCoordsVelocity gates the movement vector on #10318
        // UNITS_IsInMovingMode, so a walk/run whose mode gives way to a throw/attack/cast ends the
        // walk instead of sliding the unit under the skill's animation. Without this the session's
        // pending move kept the last path stepping after a throw started (the glide: throw sequence
        // over a translating unit).
        if (g_sessionMovePending && !dying && !UNITS_IsInMovingMode(g_targetPlayer)) g_sessionMovePending = false;
        // 1.13c: the mode handler steps the path once per frame; the legacy pre-event step also moved approach walks (item/object/monster) ~2.5x too fast (D21)
        if (!d2_aiframe_advance(1)) return 0;
        // F715: the walk a ground-pickup command started (sub_6FC828D0's ITEM branch) has no
        // completion here -- the hero reaches the pile and nothing picks it up (measured: at distance
        // 0, no collision, not busy, interact data set, the pile stays IMODE_ONGROUND, while a fresh
        // command at that same spot picks it up). Complete retail's interaction once the walk has
        // arrived (its path is spent), then forget the pile: a pickup that still cannot complete
        // leaves it for the player to click again, exactly as a refused retail pickup does.
        //
        // Hardening (F715/F684): this existing session completion heuristic requires the path goal
        // still to name the pending pile and the stored path to be spent. Static 1.13c RE has not
        // established the mode callback's target-field write. Act II's separate nearby rc=2 fallback
        // below uses the safer generic movement probe and does not rely on that field. Without the
        // stale-command check, a pending pickup from an earlier command
        // fires at the end of any UNRELATED walk that happens to finish within 4 subtiles of the
        // pile: sub_6FC828D0 is re-issued, which starts a NEW walk, shifting the deterministic chain
        // (the F715/F684 regressions). A path that no longer names the pile supersedes this session
        // pickup attempt -- drop it.
        if (g_sessionPendingPickup) {
            D2UnitStrc* pending = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)g_sessionPendingPickup);
            D2DynamicPathStrc* const ppath = g_targetPlayer->pDynamicPath;
            if (!pending || pending->dwAnimMode != IMODE_ONGROUND || !UNITS_GetRoom(pending)) {
                g_sessionPendingPickup = 0;
            } else if (!ppath || !d2::monspawn::pickupStillOurs(ppath->pTargetUnit == pending, sub_6FC7FB90(g_targetPlayer), (unsigned)sub_6FC7FBB0(g_targetPlayer), g_sessionPendingPickup)) {
                // The path's own goal is not this pile (repointed by another interaction, or cleared
                // by a plain move): the pickup this command started was superseded. Forget it.
                g_sessionPendingPickup = 0;
            } else if (!dying && ppath->dwPathPoints == 0) {
                // One completion attempt per arrival: in reach (the ITEM case's own pickup range),
                // run the interaction; a walk that ended out of reach just drops the stale pickup.
                if (D2Common_10399(g_targetPlayer, pending) <= 4) {
                    sub_6FC828D0(g_targetPlayer, UNIT_ITEM, (int32_t)g_sessionPendingPickup, 0, g_game);
                }
                g_sessionPendingPickup = 0;
            }
        }
        // D2Game 0xD01D0 (sub_6FC828D0, re-entered on arrival)
        // The object a live 0x13 click walked to. Retail's walk-mode handler (sub_6FC7F780)
        // re-enters sub_6FC828D0 with the stored interact target once the path ends; the session
        // completes the same interaction here, so the object operates (open animation, shrine
        // callback/boon) instead of the click only walking. A repointed path supersedes it.
        if (g_sessionPendingObject) {
            const uint32_t pendingObject = g_sessionPendingObject;
            D2UnitStrc* pending = SUNIT_GetServerUnit(g_game, UNIT_OBJECT, (int32_t)pendingObject);
            D2DynamicPathStrc* ppath = g_targetPlayer->pDynamicPath;
            const bool goalKept = ppath && ppath->tFinalTargetCoord.X == g_sessionPendingObjectGoalX &&
                ppath->tFinalTargetCoord.Y == g_sessionPendingObjectGoalY;
            if (!d2::monspawn::objectInteractionIsPendingOn(ppath, pending, goalKept)) {
                g_sessionPendingObject = 0;   // superseded by another move/interaction, or object gone
            } else if (d2::monspawn::objectInteractionShouldComplete(ppath, pending, dying, goalKept,
                       UNITS_IsObjectInInteractRange(g_targetPlayer, pending) && !UNITS_TestCollisionBetweenInteractingUnits(g_targetPlayer, pending, COLLIDE_MASK_PLAYER_FLYING))) {
                g_sessionPendingObject = 0;
                sub_6FC828D0(g_targetPlayer, UNIT_OBJECT, (int32_t)pendingObject, 0, g_game);
                // Out of range at arrival the dispatcher starts a fresh approach walk from where the hero
                // stands (retail re-enters it from the walk handler until the object is in reach): keep
                // following that walk. A walk that yields no path points ends the retry.
                D2DynamicPathStrc* const again = g_targetPlayer->pDynamicPath;
                if (again && again->dwPathPoints > again->dwCurrentPointIdx) {
                    g_sessionPendingObject = pendingObject;
                    g_sessionPendingObjectGoalX = again->tFinalTargetCoord.X;
                    g_sessionPendingObjectGoalY = again->tFinalTargetCoord.Y;
                }
            }
        }
        sub_6FC33670(g_game, g_targetClient);
        // Retail runs the per-client update for every client in the game's list, before the queued-unit flush
        // clears the rooms' update chains: a second connection's client sees the other players' mode changes.
        for (int s = 1; s < 4; ++s) if (g_sessionExtraClients[s].client) sub_6FC33670(g_game, g_sessionExtraClients[s].client);
        D2GAME_UpdateSpawnedClient(g_game, g_targetClient);   // for the local game's join: the "in game" marker, where retail's client update sends it
        // The per-client vitals update (1.13c D2Game+0x2BE20, which calls 0xCC030), run by retail's client flush every
        // frame: forced once 20 frames have passed without one (10 when other packets are already queued).
        if (g_targetPlayer && g_targetClient) {
            uint32_t queued = 0; D2WEB_PacketStream(&queued, nullptr);
            const int32_t since = g_targetClient->unk0x1B0;
            if (sub_6FC82360(g_targetPlayer, g_targetPlayer, (queued && since >= 10) || since >= 20)) g_targetClient->unk0x1B0 = 0;
        }
        // C3r: retail's own order (Game.cpp:1858-1870 kind 31) runs the queued-unit
        // flush right after the per-client update, every frame -- the capture found no special handling
        // around a 0x3B skill-point spend, just the next scheduled flush. Wired here, at the same point.
        if (d2_debug_trace_c3r) fprintf(stderr, "[c3r] pre-flush frame=%u skillpts=%d flags=%08x\n",
            g_game->dwGameFrame, STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_SKILLPTS, 0), g_targetPlayer->dwFlags);
        LEVEL_UpdateQueuedUnitsInAllActs(g_game);
        // one-client path: retail frees each room's delete list right after the flush, so a removal is sent once.
        LEVEL_FreeDrlgDeletes(g_game);
        if (d2_debug_trace_c3r) fprintf(stderr, "[c3r] post-flush frame=%u skillpts=%d flags=%08x\n",
            g_game->dwGameFrame, STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_SKILLPTS, 0), g_targetPlayer->dwFlags);
        // D2Client's per-frame DRLG streaming -- D2Common #10015 (0x6FD73F20) DRLGACTIVATE_Update,
        // which had no caller anywhere in this tree, so nothing ever built the rooms the wave marked
        // CLIENT_OUT_OF_SIGHT: one is built per nRoomsInitTimeout frames, and the pending-rooms step
        // above then populates it. Retail runs it from the client's frame; the session tick IS our
        // client's frame.
        if (g_sessionActivationWave) {
            D2ActiveRoomStrc* here = UNITS_GetRoom(g_targetPlayer);
            if (here && here->pDrlgRoom && here->pDrlgRoom->pLevel) {
                DRLGACTIVATE_Update(here->pDrlgRoom->pLevel->pDrlg);
                // F128: sight the far side of any crossing the wave has already reached this level's
                // rooms for, in the same tick -- before the player's own level changes.
                sessionPreSightCrossLevelExits(g_game->pAct[g_currentAct], here->pDrlgRoom->pLevel);
            }
        }
        // Retail's per-frame unit ANIMATION pass, which this tree has no caller for. It is D2Client's
        // (like the rest of the draw side, D2Client is not ported): D2Common #10853 is the player
        // updater with sequence/action handling
        // and #10193 `PATH_AdjustDirection` are exported and implemented, but nothing in D2MOO or in
        // `src/` calls either one for a player -- D2MOO reaches StopSequence only from MonsterMode.cpp,
        // where a monster's AI gates on its own frame. So the drawn hero's clock never left its
        // mode-start value and its facing never left 0: every mode played frame 0 for its whole length,
        // and a walk drawn both ways drew the same direction (s350, `` F47).
        //
        // #10853 advances `nSeqCurrentFramePrecise += wAnimSpeed` each game frame and wraps at
        // `dwFrameCountPrecise`. Retail starts that counter two ways -- a frame index from
        // the #11090 mode-setup path's frame bonus (walk, neutral, NPC-style modes) or an epoch of
        // game frames from `sub_6FCBCE70` (`pGame->dwGameFrame << 8`: the combat
        // modes) -- and the wrap loop is a modulo under both readings, so one unconditional advance is
        // right for both. The epoch's own consumer (`sub_6FCBCFD0`, `SkillPal.cpp`) reads it at mode
        // start, before the first advance, so it is not disturbed. The ONE guard that is needed is a
        // zero-length sequence: with `dwFrameCountPrecise` 0 the wrap loop's condition is true forever.
        //
        // #10193 walks `nDirection` toward the `nNewDirection` that the path pass above
        // (`PATH_ComputeVelocityAndDirectionVectorsToNextPoint` -> `sub_6FDA9720`, reached from
        // `D2Common_10226`) just set, which is what makes a hero who turns around draw a different DCC
        // direction. It is a no-op when the two already agree.
        advanceClientCastClock();
        advanceClientNeutralClock();
        advanceClientForm();
        // PLRMODE_SEQUENCE: sub_6FCBD120 (Inferno, 1.13c D2Game+0xE2480) reads the SERVER clock as the
        // sequence position; retail's headless D2Game never advances it, so the client-side step must not touch it.
        const int32_t seqServerClock = g_targetPlayer->nSeqCurrentFramePrecise;
        const bool seqMode = g_targetPlayer->dwAnimMode == PLRMODE_SEQUENCE;
        if (g_targetPlayer->dwFrameCountPrecise > 0) UNITS_StopSequence(g_targetPlayer);
        if (seqMode) g_targetPlayer->nSeqCurrentFramePrecise = seqServerClock;
        D2COMMON_10193_PATH_AdjustDirection(g_targetPlayer->pDynamicPath);
        // D2Client FUN_6FAFEE40 checks Common #10426 and dispatches death mode-end handler 9
        // before advancing the unit clock. Keep this client-owned roster update at that same
        // pre-advance boundary: the session's plain frame advance wraps a terminal clock, which
        // would otherwise erase the predicate before a SplEndDeath light unlink can observe it.
        updateSessionMonsterLights();
        // Every OTHER unit's sequence advances on the same rendered frame, and for the same reason:
        // retail's draw list shows a town NPC's or a monster's clock moving one animation frame per
        // game frame while its mode does not change at all -- Warriv `frame=15,0,0,1` (NU, len 16,
        // animSpeed 128, so the 0.5-rate step pattern) and the Rogue `frame=6,7,8,9` (NU, len 12,
        // animSpeed 256) over the SAME four consecutive retail game frames as the hero's own
        //. The player's call above is D2Common #10853, the
        // sequence-aware advance, which also rewrites the mode of a `pAnimSeq` unit; the render-side
        // advance is #10759 `UNITS_UpdateFrame`, the plain `nSeqCurrentFramePrecise += wAnimSpeed`
        // with the wrap at `dwFrameCountPrecise` -- also exported, also implemented, and also with no
        // caller anywhere in this tree or in D2MOO, because the pass that called it is D2Client's.
        //
        // It is deliberate that the two are different functions. #10759 touches the clock and nothing
        // else: mode, `nActionFrame` and `UNITFLAG_SQGFXCHANGE` are left exactly as the game logic set
        // them, so no AI, combat or spawn vector can move with this call -- and those vectors are
        // captured from a HEADLESS retail host that never runs D2Client's pass, so they hold the
        // spawn-time clock and #10853's mode rewrite would break them (s08-aitarget compares
        // `framePrecise` byte for byte). The single guard is the same zero-length one the player's
        // call needs: with `dwFrameCountPrecise` 0 the wrap has no period to wrap.
        if (g_lastLevel) {
            for (D2DrlgRoomStrc* animRoom = g_lastLevel->pFirstRoomEx; animRoom; animRoom = animRoom->pDrlgRoomNext) {
                if (!animRoom->pRoom) continue;
                for (D2UnitStrc* animUnit = animRoom->pRoom->pUnitFirst; animUnit; animUnit = animUnit->pRoomNext) {
                    if (animUnit == g_targetPlayer) continue;
                    // The facing, on the same pass and for the same reason as the clock above.
                    // `sub_6FDA9720` writes `nNewDirection` -- and the per-step `nDiffDirection` that
                    // reaches it -- whenever a unit's path recomputes its velocity vector, which is
                    // what a walking monster's own movement does. The value DRAWN is `nDirection`, and
                    // that one is only ever moved by #10193 `PATH_AdjustDirection`, which walks it
                    // toward `nNewDirection` a step at a time. Cycle 204 wired #10193 for the player
                    // alone, so every other unit's `nDirection` stayed at its spawn value for the
                    // whole game: measured (s278, main's engine b6bb41793228), every monster in the
                    // Den and on the Blood Moor held ONE facing -- 0, which the client resolves to DCC
                    // direction 4 of 8 -- across a whole scene, where retail's own den draw list
                    // records ONE Fallen at dir 23, 60, 3, 56, 51 and 44 (the retail captures
                    // den-shadow-lit.txt). Gated to the two unit types that OWN a dynamic path, and
                    // the gate is a whitelist on purpose: `pDynamicPath` shares a union with
                    // `pStaticPath` (a tile's), so a type NEGATION leaves a tile's static path in the
                    // field and the call walks a struct it knows nothing about -- measured,
                    // `s59-client-room-streaming` died of a wasm out-of-bounds trap with a negation
                    // guard and passes with this one. PLAYER and MONSTER are also exactly the two
                    // types `sub_6FDA9720` sends through its `default:` branch (OBJECT, ITEM and
                    // MISSILE write `nDirection` directly and leave `nNewDirection` alone, so
                    // adjusting one would rotate a parked object toward a stale zero), and the two
                    // `PATH_CheckCollisionsToNextPosition` asserts on.
                    if (animUnit->dwUnitType == UNIT_PLAYER || (animUnit->dwUnitType == UNIT_MONSTER && !d2_client_monster_turn_held(animUnit)))   // a walk order's turn starts when the client gets it
                        D2COMMON_10193_PATH_AdjustDirection(animUnit->pDynamicPath);
                    // 1.13c D2Client monster pass: DT (mode 0) clamps to its last cel and DD (12) holds at
                    // the end (policy rows 0x6FBA2268); a plain wrap would replay the death.
                    if (animUnit->dwUnitType == UNIT_OBJECT && animUnit->pObjectData && animUnit->pObjectData->pObjectTxt) {
                        ClientObjectClock& c = clientObjectClock(animUnit);
                        const D2ObjectsTxt* ot = animUnit->pObjectData->pObjectTxt;
                        const bool cycling = c.shown < 8 && ot->nCycleAnim[c.shown];
                        d2::object_clock::State st{c.shown, c.clock, c.count, c.speed, c.shown < 8 ? (int32_t)((uint32_t)ot->nStart[c.shown] << 8) : 0};
                        const bool toOn = d2::object_clock::step(st, ot->nIsDoor != 0, cycling);
                        c.clock = st.clock;
                        if (toOn) setClientObjectShown(c, ot, 2);
                        continue;
                    }
                    if (animUnit->dwFrameCountPrecise > 0) {
                        // Retail's client keeps its own unit copy whose clock restarts at bonus<<8 on a mode
                        // change (UNITS_SetAnimStartFrame); the server's stamp (dwGameFrame<<8) is not it.
                        // Run the clock on a client shadow and leave the server counter untouched.
                        const int32_t serverClock = animUnit->nSeqCurrentFramePrecise;
                        if (animUnit->dwUnitType == UNIT_MONSTER) {
                            auto& c = g_clientMonsterClock[animUnit->dwUnitId];
                            const bool walkOrder = d2_client_walk_tick(animUnit->dwUnitId) != 0;  // the server re-ordered the walk (retail sends 0x67): the client restarts it
                            bool freshRoll = false;
                            if (c.mode != animUnit->dwAnimMode || !c.valid || walkOrder) {
                                c.mode = animUnit->dwAnimMode; c.valid = true;
                                c.clock = UNITS_GetFrameBonus(animUnit) << 8;
                                if (!walkOrder && !c.seeded) {
                                    // 1.13c D2Client+0x4D160: a fresh client monster's start frame is
                                    // roll({dwInitSeed,666}, dwFrameCountPrecise) after the mode setter.
                                    // ponytail: dwInitSeed is the server unit's; retail's is the client room's roll.
                                    D2SeedStrc sd; { auto k = g_clientInitSeed.find(1ull << 32 | animUnit->dwUnitId); SEED_InitLowSeed(&sd, (int)(k != g_clientInitSeed.end() ? k->second : animUnit->dwInitSeed)); }
                                    c.clock = (int32_t)SEED_RollLimitedRandomNumber(&sd, animUnit->dwFrameCountPrecise);
                                    freshRoll = true;   // retail creates the unit after its step pass: the roll is shown unstepped (a retail recording = 0x10c, 63 = 0x20c)
                                }
                                c.seeded = true;
                            }
                            animUnit->nSeqCurrentFramePrecise = c.clock;
                            if (freshRoll) { animUnit->nSeqCurrentFramePrecise = serverClock; continue; }
                        }
                        if (!d2_realm_client_apply_monster_death_terminal_clock(*animUnit,
                                [](D2UnitStrc* u) { return UNITS_IsAtEndOfFrameCycle(u) != 0; })) UNITS_UpdateFrame(animUnit);
                        if (animUnit->dwUnitType == UNIT_MONSTER) {
                            g_clientMonsterClock[animUnit->dwUnitId].clock = animUnit->nSeqCurrentFramePrecise;
                            animUnit->nSeqCurrentFramePrecise = serverClock;
                        }
                    }
                }
            }
        }
        if (g_sessionAttack.state == 1 && !g_sessionAttackResolved && g_sessionAttack.eventFrame &&
            g_game->dwGameFrame >= g_sessionAttack.eventFrame) {
            g_sessionAttackResolved = true;
        }
        // A command whose skill-do scheduled no end-of-animation event (the skill was accepted but no
        // attack mode started: s000d's Concentrate at frame 42302, mode NEUTRAL, both event frames 0)
        // left this session lock at state 1 for good, and every later swing, left or right, was
        // refused as busy. Pick the event up if it appears late; a second later with none, the
        // attack is over. (Session bookkeeping only; retail's client has no such lock.)
        if (g_sessionAttack.state == 1 && !g_sessionAttackEndFrame) {
            g_sessionAttackEndFrame = EVENT_GetEventFrame(g_game, g_targetPlayer, EVENTTYPE_ENDANIM);
            if (!g_sessionAttackEndFrame && g_game->dwGameFrame > g_sessionAttack.gameFrame + 25) g_sessionAttack.state = 3;
        }
        // A sequence skill (Inferno, Whirlwind) re-arms its ENDANIM every Do (sub_6FCBD120 / RewindSkill): follow it,
        // the frame latched at the cast would end the session's attack lock early.
        if (g_sessionAttack.state == 1 && g_targetPlayer->dwAnimMode == PLRMODE_SEQUENCE) {
            const uint32_t e = EVENT_GetEventFrame(g_game, g_targetPlayer, EVENTTYPE_ENDANIM);
            if (e) g_sessionAttackEndFrame = e;
        }
        if (g_sessionAttack.state == 1 && g_sessionAttackEndFrame &&
            g_game->dwGameFrame >= g_sessionAttackEndFrame) g_sessionAttack.state = 2;
        // A skill whose mode schedules no ENDANIM event (EVENT_GetEventFrame 0: s000e's Bash, cast through
        // sub_6FC836D0) ends once the player stands in a neutral or walking mode again (the recorded start mode was already
        // neutral: the cast never entered a skill mode): otherwise the attack stayed
        // "in progress" forever and every later click was refused as busy (s000e an earlier change: 279 rounds
        // of reason 2 against one minion, 7 deaths without a swing).
        if (g_sessionAttack.state == 1 && !g_sessionAttackEndFrame && g_game->dwGameFrame > g_sessionAttack.gameFrame + 1) {
            const uint32_t m = g_targetPlayer->dwAnimMode;
            if (m == PLRMODE_NEUTRAL || m == PLRMODE_TOWNNEUTRAL || m == PLRMODE_WALK || m == PLRMODE_RUN || m == PLRMODE_TOWNWALK)
                g_sessionAttack.state = 2;
        }
        if (sessionAttackTargetLive() && !g_sessionTargetKillCounted &&
            STATLIST_UnitGetStatValue(g_sessionAttackTarget, STAT_HITPOINTS, 0) <= 0) {
            g_sessionTargetKillCounted = true; ++g_sessionKills;
        }
        watchPlayerHp();
        D2DynamicPathStrc* path = g_targetPlayer->pDynamicPath;
        g_sessionMovePending = path->dwPathPoints > 0 && path->dwCurrentPointIdx < path->dwPathPoints;
        // #9: the approach walk is over (the path is spent this frame, where it was still running the
        // last). Retail re-issues the pending unit-target command here -- sub_6FC7F780 restarts the
        // saved skill once the walk mode's path completes -- so an out-of-range click attacks the
        // moment the hero is in range. A path repointed at another unit since the command (the F715
        // stale-command case) supersedes it and drops the resume.
        if (wasMoving && !g_sessionMovePending && g_sessionApproachKind) {
            const int approachKind = g_sessionApproachKind;
            const uint32_t approachTarget = g_sessionApproachTarget;
            D2UnitStrc* approachUnit = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)approachTarget);
            const bool superseded = !approachUnit || (path->pTargetUnit && path->pTargetUnit != approachUnit);
            g_sessionApproachKind = 0; g_sessionApproachTarget = 0;
            if (!superseded && !dying) {
                if (approachKind == 1) { g_sessionResumeAttack = true; d2_session_command_primary_attack(approachTarget); g_sessionResumeAttack = false; }
                else if (approachKind == 2) d2_session_command_right_skill(approachTarget);
                else d2_session_command_left_skill(approachTarget);
            }
        }
        if (g_d2webWalkProbe.sequence) {
            ++g_d2webWalkProbe.continuationTicks;
            g_d2webWalkProbe.continuationX = path->tGameCoords.wPosX;
            g_d2webWalkProbe.continuationY = path->tGameCoords.wPosY;
            D2ActiveRoomStrc* moveRoom = path->pRoom;
            g_d2webWalkProbe.continuationLevel = moveRoom && moveRoom->pDrlgRoom
                ? DRLGROOM_GetLevelId(moveRoom->pDrlgRoom) : -1;
            g_d2webWalkProbe.continuationPending = g_sessionMovePending ? 1 : 0;
        }
        D2ActiveRoomStrc* playerRoom = UNITS_GetRoom(g_targetPlayer);
        if (playerRoom && playerRoom->pDrlgRoom && playerRoom->pDrlgRoom->pLevel) {
            const int32_t playerLevel = DUNGEON_GetLevelIdFromRoom(playerRoom);
            if (playerLevel != g_currentLevel) adoptSessionLevelChange(g_currentLevel, playerLevel);
            else if (wasMoving) syncSessionLevelFromPlayer();
        }
        syncSessionClientMapFromPlayer();
        // 1.13c D2Client+0xC9BE0 / 0x72C20: the client sees the Hurricane state the server set this frame.
        d2client_hurricane::tick(g_targetPlayer, sessionClientHeroSeed());
        d2client_vine::tick(g_targetPlayer);   // 1.13c D2Client+0xC50F0 (cltactivefunc 89, state vine_beast)
        d2client_blizzard::tick(g_targetPlayer, [](int32_t type, uint32_t id) { return SUNIT_GetServerUnit(g_game, type, (int32_t)id); });
        d2client_nova::tick();   // 1.13c D2Client+0x73CB0 (cltdofunc 25, Poison Nova action frame)
        // The client frame's tail, after this frame's packets: the attack-sound rolls of the orders the
        // server sent this frame (0x4DC70 -> 0x242B0), the room pass (0x44A80 -> 0x440C0), then ambient and weather.
        sessionClientMonsterHitPass(g_sessionClientFrame + 1);
        { extern void d2_client_monster_order_roll(); d2_client_monster_order_roll(); }
        ++g_sessionClientFrame;
        sessionClientRoomPass();
        if (g_currentLevel >= 0) {
            // 1.13c D2Client+0x81340: the weather pass draws from the client hero seed. A session that never mirrored the client map has no
            // client unit: its seed is the one retail's first 0x4F860 call reads (a retail recording capture; the derivation is not recovered).
            uint32_t* heroSeed = sessionClientHeroSeed();
            d2client::weather::Env env{};
            env.levelId = g_currentLevel; env.act = g_currentAct;
            env.weatherLevel = DATATBLS_GetRainFromLevelsTxt(g_currentLevel) != 0;
            // D2Client+0x8BC48/0x8BC4C as the headless retail rig holds them: 640x480 until its 13th client frame, then
            // 800x600 (the spawn bound range turns 513 then). Not the window: the rig has none.
            env.cellW = g_sessionClientFrame >= 13 ? 800 : 640; env.cellH = g_sessionClientFrame >= 13 ? 600 : 480;
            // 1.13c D2Client+0x5F020 runs 0x4F860 first, before the frame's weather pass; same hero seed.
            d2client_ambient::tick(heroSeed, (uint32_t)ENVIRONMENT_GetPeriodOfDayFromAct(g_game->pAct[g_currentAct], 0), env.weatherLevel);
            d2client::weather::tick(*reinterpret_cast<d2client::weather::Seed*>(heroSeed), env);
        }
        sessionClientMonsterIdlePass();
    }
    updateSessionSnapshot(); return 1;
}

// Why the last primary attack was refused (0 = not refused), so a refusal is never guessed at.
static int g_sessionAttackReason = 0;
enum { ATTACK_OK = 0, ATTACK_NO_SESSION = 1, ATTACK_BUSY = 2, ATTACK_NO_TARGET = 3, ATTACK_NOT_NEAR = 4,
       ATTACK_TARGET_DEAD = 5, ATTACK_UNTARGETABLE = 6, ATTACK_NO_SKILL = 7, ATTACK_NO_PATH = 8,
       // The command was accepted and the swing set up, but the skill-do armed no ENDANIM, so nothing
       // will ever end it and the attack was NOT put in flight (state 0, no lock held, no blow thrown).
       // It used to leave g_sessionAttackReason at ATTACK_OK, so a caller reading the reason could not
       // tell that no-op from a swing that did start -- and 133 of 400 rounds of the act V Shenk fight
       // are exactly this population, against 260 refused BUSY (F53, act5-bootstrap).
       ATTACK_NO_ENDANIM = 9 };
EMSCRIPTEN_KEEPALIVE int d2_session_attack_reason() { return g_sessionAttackReason; }
// A target is reachable from the player's room or any room adjacent to it. The session used to
// require the SAME room, which retail does not: a monster standing across a room seam could not be
// fought at all (s000, Blood Moor). Distance is still decided by melee range and the walk below.
static bool sessionRoomIsNear(D2ActiveRoomStrc* from, D2ActiveRoomStrc* room) {
    if (!from || !room) return false;
    if (from == room) return true;
    D2ActiveRoomStrc** near = nullptr; int32_t count = 0;
    DUNGEON_GetAdjacentRoomsListFromRoom(from, &near, &count);
    for (int32_t i = 0; i < count; ++i) if (near[i] == room) return true;
    return false;
}
// Read-only: the parts of retail's melee-range test (UNITS_IsInMeleeRange, D2Common #10361) for the
// player against a monster, so a refused attack names which part refused it:
// {in range, distance (#10399), melee range, ray collides (#10362, PLAYER_FLYING), player size,
//  target size, player x, y, target x, y, weapon durability, weapon max durability, player seed lo, hi}.
static int32_t g_sessionMeleeProbe[14];
// Read-only: the game's monster-type table for a level, as MONSTERREGION_InitializeAll rolled it
// from the game seed: {game seed, mon seed, count, hcIdx x count}.
static int32_t g_sessionMonRegion[16];
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_monregion(int32_t levelId) {
    memset(g_sessionMonRegion, 0, sizeof g_sessionMonRegion);
    if (!g_game || !g_game->pMonReg) return g_sessionMonRegion;
    D2MonsterRegionStrc* r = MONSTERREGION_GetMonsterRegionFromLevelId(g_game->pMonReg, levelId);
    g_sessionMonRegion[0] = (int32_t)g_game->dwInitSeed; g_sessionMonRegion[1] = (int32_t)g_game->dwMonSeed;
    if (!r) return g_sessionMonRegion;
    g_sessionMonRegion[2] = r->nMonCount;
    for (int i = 0; i < r->nMonCount && i < 13; ++i) g_sessionMonRegion[3 + i] = r->pMonData[i].nMonHcIdx;
    return g_sessionMonRegion;
}
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_melee_probe(uint32_t targetUnitId) {
    memset(g_sessionMeleeProbe, 0, sizeof g_sessionMeleeProbe);
    int32_t* o = g_sessionMeleeProbe;
    if (!g_sessionLive || !g_game || !g_targetPlayer) return o;
    // The weapon's wear and the player's seed read without a target too (a town trip checks them).
    if (D2UnitStrc* weapon = g_targetPlayer->pInventory ? INVENTORY_GetItemFromBodyLoc(g_targetPlayer->pInventory, BODYLOC_RARM) : nullptr) {
        o[10] = STATLIST_UnitGetStatValue(weapon, STAT_DURABILITY, 0); o[11] = STATLIST_UnitGetStatValue(weapon, STAT_MAXDURABILITY, 0);
    }
    o[12] = (int32_t)g_targetPlayer->pSeed.nLowSeed; o[13] = (int32_t)g_targetPlayer->pSeed.nHighSeed;
    D2UnitStrc* target = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)targetUnitId);
    if (!target || !UNITS_GetRoom(g_targetPlayer)) return o;
    o[0] = UNITS_IsInMeleeRange(g_targetPlayer, target, 1); o[1] = D2Common_10399(g_targetPlayer, target);
    o[2] = UNITS_GetMeleeRange(g_targetPlayer); o[3] = UNITS_TestCollisionWithUnit(g_targetPlayer, target, COLLIDE_MASK_PLAYER_FLYING);
    o[4] = UNITS_GetUnitSizeX(g_targetPlayer); o[5] = UNITS_GetUnitSizeX(target);
    o[6] = CLIENTS_GetUnitX(g_targetPlayer); o[7] = CLIENTS_GetUnitY(g_targetPlayer); o[8] = CLIENTS_GetUnitX(target); o[9] = CLIENTS_GetUnitY(target);
    return o;
}
// Self-reporting: consecutive swings refused as busy (attack reason 2) per swing -- the session's
// open swing, the player's mode and whether its ENDANIM event is still pending when the streak
// started, and the longest streaks so far. A 400-round stall explains itself in the dossier.
static void noteBusy() {
    if (!g_busyStreak++) g_busyStartFrame = g_game ? g_game->dwGameFrame : 0;
}
static void endBusyStreak() {
    if (g_busyStreak >= 20 && g_busyLog.size() < 1500) {
        char buf[200];
        snprintf(buf, sizeof buf, "%s%u refused from frame %u (swing frame %u state %u mode %u, end %u)", g_busyLog.empty() ? "" : "; ",
            g_busyStreak, g_busyStartFrame, g_sessionAttack.gameFrame, g_sessionAttack.state, g_sessionAttack.mode, g_sessionAttackEndFrame);
        g_busyLog += buf;
    }
    if (g_busyStreak > g_busyLongest) g_busyLongest = g_busyStreak;
    g_busyStreak = 0;
}
EMSCRIPTEN_KEEPALIVE const char* d2_session_busy_watch() {
    static std::string out; char buf[240];
    snprintf(buf, sizeof buf, "streak %u (from frame %u; swing frame %u state %u mode %u end %u; player mode %u, ENDANIM %d MODECHANGE %d), longest %u",
        g_busyStreak, g_busyStartFrame, g_sessionAttack.gameFrame, g_sessionAttack.state, g_sessionAttack.mode, g_sessionAttackEndFrame,
        g_targetPlayer ? g_targetPlayer->dwAnimMode : 0,
        (g_game && g_targetPlayer) ? EVENT_GetEventFrame(g_game, g_targetPlayer, EVENTTYPE_ENDANIM) : -1,
        (g_game && g_targetPlayer) ? EVENT_GetEventFrame(g_game, g_targetPlayer, EVENTTYPE_MODECHANGE) : -1, std::max(g_busyLongest, g_busyStreak));
    out = buf; if (!g_busyLog.empty()) out += "; streaks >= 20: " + g_busyLog; return out.c_str();
}
// D2Client's unit-target skill start (1.13c D2Client+0x6FB530C0) turns the caster toward the target's
// precise position before the skill mode starts; the server never does (this session shares one unit).
static void sessionFaceTarget(D2UnitStrc* target) {
    D2DynamicPathStrc* const path = g_targetPlayer->pDynamicPath;
    D2COMMON_10160_PathUpdateDirection(path, static_cast<uint8_t>(PATH_ComputeDirectionFromPreciseCoords_6FDAC760(
        path->tGameCoords.dwPrecisionX, path->tGameCoords.dwPrecisionY,
        target->pDynamicPath->tGameCoords.dwPrecisionX, target->pDynamicPath->tGameCoords.dwPrecisionY)));
}
// Test hook: set the player's facing outright (scenario fixture for the face-target check).
EMSCRIPTEN_KEEPALIVE void d2_session_set_facing(uint32_t dir) { if (g_sessionLive && g_targetPlayer) PATH_SetDirection(g_targetPlayer->pDynamicPath, (uint8_t)dir); }
// The unit-target skill-do, with the turn retail's callback does first. 1.13c D2Client+0x6FB530C0
// (the unit-resolved action) resolves the target, attaches it to the caster's path and runs
// 6FB55FB0, which reads the target unit's coordinates off the path and writes the target-derived
// facing (D2Common #10739 on the path) before shared skill start 6FB51CF0. Every unit-target skill
// this adapter starts routes through here, so no command path can begin its skill still facing the
// old way -- the right-click cast (sessionCommandHandSkill) and kick were the ones that did (#3a).
static int32_t sessionSkillDoOnUnit(D2UnitStrc* target, D2SkillStrc* skill, uint32_t targetUnitId) {
    sessionFaceTarget(target);
    return sub_6FC836D0(g_game, g_targetPlayer, skill, UNIT_MONSTER, (int32_t)targetUnitId, 1);
}
// Retail's 0x06 handler swings at a corpse that still exists (a retail recording replay: 0 diffs), so no dead/untargetable refusal here.
EMSCRIPTEN_KEEPALIVE int d2_session_command_primary_attack(uint32_t targetUnitId) {
    g_sessionApproachKind = 0; g_sessionApproachTarget = 0;   // this command supersedes any pending approach walk
    g_sessionAttackReason = ATTACK_NO_SESSION;
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pDynamicPath ||
        !g_targetPlayer->pDynamicPath->pRoom || g_targetPlayer->dwAnimMode == PLRMODE_DEAD ||
        g_targetPlayer->dwAnimMode == PLRMODE_DEATH) return 0;   // dying: retail's PLRMODES_First refuses any mode change
    g_sessionAttackReason = ATTACK_BUSY;
    if (!g_sessionReplayFixture && (g_sessionAttack.state == 1 || g_sessionAttack.state == 2)) { noteBusy(); return 0; }
    endBusyStreak();
    D2UnitStrc* target = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, targetUnitId);
    g_sessionAttackReason = ATTACK_NO_TARGET;
    if (!target || !target->pDynamicPath) return 0;
    g_sessionAttackReason = ATTACK_NOT_NEAR;
    if (!sessionRoomIsNear(g_targetPlayer->pDynamicPath->pRoom, target->pDynamicPath->pRoom)) return 0;
    g_sessionAttackReason = ATTACK_OK;
    // Approach completion re-issues the attack without a range test: 1.13c D2Game+0x79B90 restarts the saved skill straight into PLAYERMODE_Change.
    if (!g_sessionResumeAttack && !UNITS_IsInMeleeRange(g_targetPlayer, target, 1)) {
        PLRMODE_StartID_WalkRunKnockback(g_game, g_targetPlayer, g_sessionRunToggle ? PLRMODE_RUN : PLRMODE_WALK, target);   // D33: the client picks run vs walk packet from its toggle
        const D2DynamicPathStrc* path = g_targetPlayer->pDynamicPath;
        g_sessionMovePending = path->dwPathPoints > 0;
        g_sessionTargetX = target->pDynamicPath->tGameCoords.wPosX;
        g_sessionTargetY = target->pDynamicPath->tGameCoords.wPosY;
        if (!g_sessionMovePending) g_sessionAttackReason = ATTACK_NO_PATH;
        else { g_sessionApproachKind = 1; g_sessionApproachTarget = targetUnitId; }
        updateSessionSnapshot(); return g_sessionMovePending ? 2 : 0;
    }
    D2SkillStrc* skill = SKILLS_GetHighestLevelSkillFromUnitAndId(g_targetPlayer, SKILL_ATTACK);
    if (!skill) { g_sessionAttackReason = ATTACK_NO_SKILL; return 0; }
    // Retail's unit-skill callback alternates a dual-wielding player's base Attack skill with
    // Left Hand Swing, then enters the selected skill's own mode. Keep this test-session adapter
    // on that same selector/mode path; Attack is A1, while Left Hand Swing is S3 in 1.13c Skills.txt.
    if (UNITS_CanDualWield(g_targetPlayer)) skill = sub_6FC83450(g_targetPlayer, skill);
    if (!skill) { g_sessionAttackReason = ATTACK_NO_SKILL; return 0; }
    const int32_t skillId = SKILLS_GetSkillIdFromSkill(skill, __FILE__, __LINE__);
    const int32_t skillMode = SKILLS_GetSkillModeFromUnit(g_targetPlayer, skill);
    if (!skillMode) { g_sessionAttackReason = ATTACK_NO_SKILL; return 0; }
    memset(&g_sessionAttack, 0, sizeof g_sessionAttack); g_sessionAttack.version = 1;
    g_sessionAttack.size = sizeof g_sessionAttack; g_sessionAttack.gameFrame = g_game->dwGameFrame;
    g_sessionAttack.state = 1; g_sessionAttack.attackerId = g_targetPlayer->dwUnitId;
    g_sessionAttack.targetId = target->dwUnitId; g_sessionAttack.skillId = (uint32_t)skillId;
    g_sessionAttack.targetHpBefore = STATLIST_UnitGetStatValue(target, STAT_HITPOINTS, 0);
    g_sessionAttack.targetHpAfter = g_sessionAttack.targetHpBefore; g_sessionAttack.attacks = ++g_sessionAttackCount;
    g_sessionAttackTarget = target; g_sessionAttackSkill = skill;
    g_sessionAttackResolved = false; g_sessionTargetKillCounted = false;
    UNITS_SetUsedSkill(g_targetPlayer, skill); UNITS_SetTargetUnitForDynamicUnit(g_targetPlayer, target);
    sessionFaceTarget(target);
    if (g_sessionReplayFixture) D2GAME_DeletePlayerPerFrameEvents_6FCBCE50(g_game, g_targetPlayer);   // restart mid-swing: drop the first swing's ENDANIM/MODECHANGE
    SUNIT_SetCombatMode(g_game, g_targetPlayer, skillMode);
    g_targetPlayer->dwFlags &= ~UNITFLAG_SKSRVDOFUNC;
    SKILLS_SrvSt01_Attack_LeftHandSwing(g_game, g_targetPlayer, skillId,
        SKILLS_GetSkillLevel(g_targetPlayer, skill, TRUE));
    sub_6FCBCE70(g_game, g_targetPlayer);
    g_sessionAttack.eventFrame = EVENT_GetEventFrame(g_game, g_targetPlayer, EVENTTYPE_MODECHANGE);
    g_sessionAttackEndFrame = EVENT_GetEventFrame(g_game, g_targetPlayer, EVENTTYPE_ENDANIM);
    g_sessionAttack.mode = g_targetPlayer->dwAnimMode;
    // No animation started (the skill-do accepted the command but the player never left neutral, e.g.
    // hit mid-click): nothing will end it, so the attack is not in flight. Left at state 1 it refused
    // every later command, across death and respawn, while the pack killed the player (s000b, Sewers).
    if (!g_sessionAttackEndFrame) { g_sessionAttack.state = 0; g_sessionAttackReason = ATTACK_NO_ENDANIM; return 0; }
    return 1;
}

EMSCRIPTEN_KEEPALIVE int d2_session_command_throw(uint32_t targetUnitId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pDynamicPath ||
        g_targetPlayer->dwAnimMode == PLRMODE_DEAD || g_sessionAttack.state == 1 || g_sessionAttack.state == 2) return 0;
    D2UnitStrc* target = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)targetUnitId);
    D2SkillStrc* skill = SKILLS_GetHighestLevelSkillFromUnitAndId(g_targetPlayer, SKILL_THROW);
    if (!target || !target->pDynamicPath || !skill || SKILLS_GetUseState(g_targetPlayer, skill) != SKILLUSTATE_USABLE ||
        target->dwAnimMode == MONMODE_DEAD || !(target->dwFlags & UNITFLAG_TARGETABLE)) return 0;
    memset(&g_sessionAttack, 0, sizeof g_sessionAttack); g_sessionAttack.version = 1;
    g_sessionAttack.size = sizeof g_sessionAttack; g_sessionAttack.gameFrame = g_game->dwGameFrame;
    g_sessionAttack.state = 1; g_sessionAttack.attackerId = g_targetPlayer->dwUnitId;
    g_sessionAttack.targetId = target->dwUnitId; g_sessionAttack.skillId = SKILL_THROW;
    g_sessionAttack.targetHpBefore = STATLIST_UnitGetStatValue(target, STAT_HITPOINTS, 0);
    g_sessionAttack.targetHpAfter = g_sessionAttack.targetHpBefore; g_sessionAttack.attacks = ++g_sessionAttackCount;
    g_sessionAttackTarget = target; g_sessionAttackSkill = skill; g_sessionAttackResolved = false;
    const int32_t result = sessionSkillDoOnUnit(target, skill, targetUnitId);
    if (result != 0) { g_sessionAttack.state = 0; return 0; }
    g_sessionAttack.eventFrame = EVENT_GetEventFrame(g_game, g_targetPlayer, EVENTTYPE_MODECHANGE);
    g_sessionAttackEndFrame = EVENT_GetEventFrame(g_game, g_targetPlayer, EVENTTYPE_ENDANIM);
    g_sessionAttack.mode = g_targetPlayer->dwAnimMode;
    // No animation started (the skill-do accepted the command but the player never left neutral, e.g.
    // hit mid-click): nothing will end it, so the attack is not in flight. Left at state 1 it refused
    // every later command, across death and respawn, while the pack killed the player (s000b, Sewers).
    if (!g_sessionAttackEndFrame) { g_sessionAttack.state = 0; g_sessionAttackReason = ATTACK_NO_ENDANIM; return 0; }
    return 1;
}

EMSCRIPTEN_KEEPALIVE int d2_session_command_kick(uint32_t targetUnitId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pDynamicPath ||
        g_targetPlayer->dwAnimMode == PLRMODE_DEAD || g_sessionAttack.state == 1 || g_sessionAttack.state == 2) return 0;
    D2UnitStrc* target = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)targetUnitId);
    D2SkillStrc* skill = SKILLS_GetHighestLevelSkillFromUnitAndId(g_targetPlayer, SKILL_KICK);
    if (!target || !target->pDynamicPath || !skill || SKILLS_GetUseState(g_targetPlayer, skill) != SKILLUSTATE_USABLE ||
        target->dwAnimMode == MONMODE_DEAD || !(target->dwFlags & UNITFLAG_TARGETABLE)) return 0;
    memset(&g_sessionAttack, 0, sizeof g_sessionAttack); g_sessionAttack.version = 1;
    g_sessionAttack.size = sizeof g_sessionAttack; g_sessionAttack.gameFrame = g_game->dwGameFrame;
    g_sessionAttack.state = 1; g_sessionAttack.attackerId = g_targetPlayer->dwUnitId;
    g_sessionAttack.targetId = target->dwUnitId; g_sessionAttack.skillId = SKILL_KICK;
    g_sessionAttack.targetHpBefore = STATLIST_UnitGetStatValue(target, STAT_HITPOINTS, 0);
    g_sessionAttack.targetHpAfter = g_sessionAttack.targetHpBefore; g_sessionAttack.attacks = ++g_sessionAttackCount;
    g_sessionAttackTarget = target; g_sessionAttackSkill = skill; g_sessionAttackResolved = false;
    const int32_t result = sessionSkillDoOnUnit(target, skill, targetUnitId);
    if (result != 0) { g_sessionAttack.state = 0; return 0; }
    g_sessionAttack.eventFrame = EVENT_GetEventFrame(g_game, g_targetPlayer, EVENTTYPE_MODECHANGE);
    g_sessionAttackEndFrame = EVENT_GetEventFrame(g_game, g_targetPlayer, EVENTTYPE_ENDANIM);
    g_sessionAttack.mode = g_targetPlayer->dwAnimMode;
    // No animation started (the skill-do accepted the command but the player never left neutral, e.g.
    // hit mid-click): nothing will end it, so the attack is not in flight. Left at state 1 it refused
    // every later command, across death and respawn, while the pack killed the player (s000b, Sewers).
    if (!g_sessionAttackEndFrame) { g_sessionAttack.state = 0; g_sessionAttackReason = ATTACK_NO_ENDANIM; return 0; }
    return 1;
}

// Retail's skill selection (client packet 0x3C SelectSkill: skill id | left << 31, owner GUID -1 for a
// skill of the player's own), through the recovered handler. 0 = assigned; 3 = refused (a skill the
// player does not have, or at level 0).
EMSCRIPTEN_KEEPALIVE int d2_session_command_select_skill(uint32_t skillId, int left) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || skillId > 0x7FFFFFFF) return 3;
    uint8_t packet[9] = { 0x3C };
    const uint32_t word = skillId | (left ? 0x80000000u : 0u); const int32_t owner = -1;
    memcpy(packet + 1, &word, 4); memcpy(packet + 5, &owner, 4);
    return D2GAME_PACKETCALLBACK_Rcv0x3C_SelectSkill_6FC87050(g_game, g_targetPlayer, packet, sizeof packet);
}
// The player's selected skills: [left id, right id] (-1 = none).
static int32_t g_sessionSelectedSkills[2];
D2UnitStrc* d2_session_target_player_unit() { return g_targetPlayer; }   // the character screen's unit (d2client_35270_wire.cpp)
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_selected_skills() {
    D2SkillStrc* l = g_targetPlayer ? UNITS_GetLeftSkill(g_targetPlayer) : nullptr;
    D2SkillStrc* r = g_targetPlayer ? UNITS_GetRightSkill(g_targetPlayer) : nullptr;
    g_sessionSelectedSkills[0] = l ? SKILLS_GetSkillIdFromSkill(l, __FILE__, __LINE__) : -1;
    g_sessionSelectedSkills[1] = r ? SKILLS_GetSkillIdFromSkill(r, __FILE__, __LINE__) : -1;
    return g_sessionSelectedSkills;
}
// What a right click on open ground does with the right-hand skill: 2 = the skill is hand to hand, so
// the click walks there; 1 = the skill is used at the spot; -1 = no player or no right skill. The same
// answer d2_realm_client_skill_plan gives a joined game (1.13c D2Client+0x1B7F0).
EMSCRIPTEN_KEEPALIVE int32_t d2_session_right_ground_plan() {
    D2SkillStrc* r = g_targetPlayer ? UNITS_GetRightSkill(g_targetPlayer) : nullptr;
    if (!r) return -1;
    return D2Common_11016(g_targetPlayer, r) == 1 ? 2 : 1;
}
// The skill bound to F<i+1> (-1 unbound): retail labels the right button 'F<n>' for each hotkey bound to its skill.
EMSCRIPTEN_KEEPALIVE int32_t d2_session_hotkey_skill(int32_t i) {
    return (g_targetClient && i >= 0 && i < 16) ? (int32_t)(int16_t)g_targetClient->HotkeySkills[i].nSkill : -1;
}
// V9b: the hand a hotkey selects into (1 = left, as the save header's bit 15) and the assignment retail's F-key over a popup icon makes.
EMSCRIPTEN_KEEPALIVE int32_t d2_session_hotkey_hand(int32_t i) {
    return (g_targetClient && i >= 0 && i < 16) ? (int32_t)g_targetClient->HotkeySkills[i].nHand : 0;
}
EMSCRIPTEN_KEEPALIVE int d2_session_hotkey_assign(int32_t i, int32_t skillId, int32_t left) {
    if (!g_targetClient || i < 0 || i >= 16 || skillId < 0 || skillId > 0xFFF) return 3;
    // one key per skill and hand: retail moves the binding, so a skill is never labelled twice on its hand
    for (int k = 0; k < 16; ++k)
        if (k != i && g_targetClient->HotkeySkills[k].nSkill == skillId && g_targetClient->HotkeySkills[k].nHand == (left ? 1 : 0)) CLIENTS_SetSkillHotKey(g_targetClient, k, -1, 0, -1);
    CLIENTS_SetSkillHotKey(g_targetClient, i, (int16_t)skillId, left ? 1 : 0, -1);
    return 0;
}
// Every skill the player HAS, as the skill-select popup lists them: Skills.txt id, SkillDesc's
// ListRow and IconCel, the class (-1 = the general skills), the passive/leftskill flags, its level.
struct D2SessionSkillChoice { uint16_t id; int8_t listRow; int8_t charClass; uint16_t iconCel; uint8_t flags; uint8_t level; };
static_assert(sizeof(D2SessionSkillChoice) == 8, "D2SessionSkillChoice is 8 bytes");
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_skill_choices(D2SessionSkillChoice* out, uint32_t cap) {
    if (!g_sessionLive || !g_targetPlayer || !g_targetPlayer->pSkills || !out) return 0;
    uint32_t n = 0;
    for (D2SkillStrc* sk = SKILLS_GetFirstSkillFromSkillList(g_targetPlayer->pSkills); sk && n < cap; sk = SKILLS_GetNextSkill(sk)) {
        D2SkillsTxt* txt = SKILLS_GetSkillsTxtRecordFromSkill(sk);
        if (!txt) continue;
        D2SkillDescTxt* desc = DATATBLS_GetSkillDescTxtRecord(txt->wSkillDesc);
        D2SessionSkillChoice& row = out[n++];
        row.id = (uint16_t)SKILLS_GetSkillIdFromSkill(sk, __FILE__, __LINE__);
        row.listRow = desc ? (int8_t)desc->nListRow : -1; row.charClass = txt->nCharClass;
        row.iconCel = desc ? desc->nIconCel : 0;
        // bit 4 = not usable right now (SKILLS_GetUseState), except an aura: retail's classifier (D2Client+0x3DA60) maps SKILLUSTATE_AURA to its own non-red class, so aura icons are drawn normal. Every other state is red (v9b popup frames, extra=1).
        const int useState = SKILLS_GetUseState(g_targetPlayer, sk);
        row.flags = (txt->dwFlags[0] & SKILLSFLAG_PASSIVE ? 1 : 0) | (txt->dwFlags[0] & SKILLSFLAG_LEFTSKILL ? 2 : 0) |
            (useState != SKILLUSTATE_USABLE && useState != SKILLUSTATE_AURA ? 4 : 0) |
            ((DUNGEON_IsTownLevelId(g_currentLevel) && !(txt->dwFlags[0] & SKFLAG_INTOWN)) ? 8 : 0) |
            (useState == SKILLUSTATE_AURA ? 16 : 0);   // bit 16 = aura: retail's classifier D2Client+0x3DA60 returns 4, the icon is drawn with colour shift 4. bit 8 = Skills.txt InTown is 0 and we are in a town: retail reds only the hand BUTTON for it (Act IV town r29: Attack button extra=1, popup Attack extra=0)
        row.level = (uint8_t)SKILLS_GetSkillLevel(g_targetPlayer, sk, TRUE);
    }
    return n;
}
// The selected RIGHT skill on a monster (a right click on it), as kick and throw run: the skill-do
// entry (sub_6FC836D0) with the session's attack bookkeeping. Walks into range first when melee.
// Why the last right-skill command was refused: 0 accepted, 1 no session / dying, 2 an attack in flight,
// 3 no target or skill, 10+SKILLUSTATE_* not usable, 4 target dead or untargetable, 6 no animation started
// (d2a00c4), 100+result the skill-do refused. Read-only.
static int g_rightSkillRefusal = 0;
// Cast-start witness: retail plays the skill's stsound at cast entry even when the cast is refused (no animation
// started, refusal 6). The client trigger fires it when the sequence moves; the mode-change path covers accepted casts.
static uint32_t g_castRefusedSeq = 0; static int32_t g_castRefusedSkill = 0;
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_cast_refused_seq() { return g_castRefusedSeq; }
EMSCRIPTEN_KEEPALIVE int32_t d2_session_cast_refused_skill() { return g_castRefusedSkill; }
EMSCRIPTEN_KEEPALIVE int d2_session_right_skill_refusal() { return g_rightSkillRefusal; }
extern "C" void d2_client_note_local_action(int32_t mode);
extern "C" int32_t d2_client_cast_walks(uint32_t srvDoFunc, int32_t inMelee, int32_t distance, int32_t range);
extern "C" void d2_client_cast_walk_reset();
static int sessionCommandHandSkill(uint32_t targetUnitId, bool left) {
    d2_client_cast_walk_reset();
    g_sessionApproachKind = 0; g_sessionApproachTarget = 0;   // this command supersedes any pending approach walk
    // The same refusal reasons as the left click (d2_session_attack_reason), so a stalled right-hand
    // fight names why instead of showing the last left click's reason.
    // Busy only while the swing's end-of-animation event has not run: state 2 is the frame after it ran (the player is already back in
    // neutral), and retail's 0x06/0x0C/0x0D handlers carry no lock at all (D2Game+0x53550), so a swing arriving on that frame starts.
    g_sessionAttackReason = g_sessionAttack.state == 1 ? ATTACK_BUSY : ATTACK_NO_SESSION;
    g_rightSkillRefusal = 1;
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pDynamicPath ||
        g_targetPlayer->dwAnimMode == PLRMODE_DEAD || g_targetPlayer->dwAnimMode == PLRMODE_DEATH) return 0;
    g_rightSkillRefusal = 2;
    if (g_sessionAttack.state == 1) return 0;
    D2UnitStrc* target = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)targetUnitId);
    D2SkillStrc* skill = left ? UNITS_GetLeftSkill(g_targetPlayer) : UNITS_GetRightSkill(g_targetPlayer);
    g_rightSkillRefusal = 3;
    if (!target || !target->pDynamicPath || !skill) return 0;
    d2client_blizzard::cast(SKILLS_GetSkillIdFromSkill(skill, __FILE__, __LINE__), target, 0, 0);
    d2client_nova::cast(SKILLS_GetSkillIdFromSkill(skill, __FILE__, __LINE__), false);   // the client casts whether or not the server accepts the use (Frozen Orb, cltdofunc 29)
    d2client_clmissile::cast(SKILLS_GetSkillIdFromSkill(skill, __FILE__, __LINE__), SKILLS_GetSkillLevel(g_targetPlayer, skill, TRUE), target, 0, 0);   // 1.13c D2Client+0x15DA0 (cltdofunc 17, Teeth)
    g_rightSkillRefusal = 10 + SKILLS_GetUseState(g_targetPlayer, skill);   // 10 + SKILLUSTATE_* (not usable: mana, weapon, ...)
    // Retail's on-unit helper sub_6FC836D0 swaps an AttackNoMana skill (no mana, quantity or shape) for plain Attack,
    // as the 0x0C path does; the gate must let those through (Feral Rage, a retail recording: shape restricted, retail hits).
    if (SKILLS_GetUseState(g_targetPlayer, skill) != SKILLUSTATE_USABLE) {
        D2SkillsTxt* swapTxt = SKILLS_GetSkillsTxtRecordFromSkill(skill);
        const int32_t useState = SKILLS_GetUseState(g_targetPlayer, skill);
        const bool swapsToAttack = swapTxt && (swapTxt->dwFlags[0] & gdwBitMasks[SKILLSFLAGINDEX_ATTACKNOMANA]) &&
            (useState == SKILLUSTATE_NOMANA || useState == SKILLUSTATE_NOQUANTITY || useState == SKILLUSTATE_SHAPERESTRICT);
        if (!swapsToAttack) { g_castRefusedSkill = SKILLS_GetSkillIdFromSkill(skill, __FILE__, __LINE__); ++g_castRefusedSeq; return 0; }   // refused, but the stsound still plays
        // a shape-restricted skill: the client swaps too, its local action is Attack's A1 (2A750 on a unit, mode 7), whose weapon-class row
        // plays at the cast (Feral Rage unshifted, a retail recording: 251); Smite with no shield plays none (a retail recording)
        if (useState == SKILLUSTATE_SHAPERESTRICT) d2_client_note_local_action(7);
    }
    // Retail 0x0D (D2GAME_PACKETCALLBACK_Rcv0x0D_RightSkillOnUnit_6FC83F80) checks no corpse or TARGETABLE
    // flag: the server-unit lookup, the 50-subtile range and the skill's use state are the whole gate. A dead
    // monster (Zeal, 1.13c tick 72 of a retail recording-106) is a legal target; the melee swing just lands on nothing.
    g_rightSkillRefusal = 5;
    const int skillId = SKILLS_GetSkillIdFromSkill(skill, __FILE__, __LINE__);
    D2SkillsTxt* txt = SKILLS_GetSkillsTxtRecordFromSkill(skill);
    // The client's own side of the cast (D2Client 0x1B700 -> 0x1B1D0 / 0x18880, local_action.cpp): a srvdofunc 0x13 skill on a
    // target beyond its aurarangecalc range walks the client unit (its first footfall, local action 2) and starts no client skill.
    if (txt && d2_client_cast_walks(txt->wSrvDoFunc, UNITS_IsInMeleeRange(g_targetPlayer, target, 0),
            UNITS_GetDistanceToOtherUnit(g_targetPlayer, target),
            SKILLS_EvaluateSkillFormula(g_targetPlayer, txt->dwAuraRangeCalc, skillId, SKILLS_GetSkillLevel(g_targetPlayer, skill, TRUE))))
        d2_client_note_local_action(2);
    // A melee skill (Skills.txt range "h2h") out of reach: walk to the target first, as the left click does.
    if (txt && txt->nRange == 1 && !UNITS_IsInMeleeRange(g_targetPlayer, target, 1)) {
        PLRMODE_StartID_WalkRunKnockback(g_game, g_targetPlayer, g_sessionRunToggle ? PLRMODE_RUN : PLRMODE_WALK, target);   // D33
        g_sessionMovePending = g_targetPlayer->pDynamicPath->dwPathPoints > 0;
        if (g_sessionMovePending) { g_sessionApproachKind = left ? 3 : 2; g_sessionApproachTarget = targetUnitId; }
        g_sessionAttackReason = g_sessionMovePending ? ATTACK_OK : ATTACK_NO_PATH;
        updateSessionSnapshot(); return g_sessionMovePending ? 2 : 0;
    }
    g_sessionAttackReason = ATTACK_OK;
    memset(&g_sessionAttack, 0, sizeof g_sessionAttack); g_sessionAttack.version = 1;
    g_sessionAttack.size = sizeof g_sessionAttack; g_sessionAttack.gameFrame = g_game->dwGameFrame;
    g_sessionAttack.state = 1; g_sessionAttack.attackerId = g_targetPlayer->dwUnitId;
    g_sessionAttack.targetId = target->dwUnitId; g_sessionAttack.skillId = (uint32_t)skillId;
    g_sessionAttack.targetHpBefore = STATLIST_UnitGetStatValue(target, STAT_HITPOINTS, 0);
    g_sessionAttack.targetHpAfter = g_sessionAttack.targetHpBefore; g_sessionAttack.attacks = ++g_sessionAttackCount;
    g_sessionAttackTarget = target; g_sessionAttackSkill = skill; g_sessionAttackResolved = false; g_sessionTargetKillCounted = false;
    const int32_t result = sessionSkillDoOnUnit(target, skill, targetUnitId);
    if (result != 0) { g_sessionAttack.state = 0; g_rightSkillRefusal = 100 + result; return 0; }
    g_sessionAttack.eventFrame = EVENT_GetEventFrame(g_game, g_targetPlayer, EVENTTYPE_MODECHANGE);
    g_sessionAttackEndFrame = EVENT_GetEventFrame(g_game, g_targetPlayer, EVENTTYPE_ENDANIM);
    g_sessionAttack.mode = g_targetPlayer->dwAnimMode;
    // No animation started (the skill-do accepted the command but the player never left neutral, e.g.
    // hit mid-click): nothing will end it, so the attack is not in flight. Left at state 1 it refused
    // every later command, across death and respawn, while the pack killed the player (s000b, Sewers).
    // A refused melee cast (Skills.txt range h2h) plays no client start sound.
    if (!g_sessionAttackEndFrame) { g_sessionAttack.state = 0; g_rightSkillRefusal = 6; if (!txt || txt->nRange != 1) { g_castRefusedSkill = skillId; ++g_castRefusedSeq; g_refusedDoSkill = skillId; g_refusedDoWait = 8; } return 0; }   // the client still casts: stsound at entry, the client do-function sound at its do-frame
    g_rightSkillRefusal = 0;
    return 1;
}
EMSCRIPTEN_KEEPALIVE int d2_session_command_right_skill(uint32_t targetUnitId) { d2_client_note_move_cmd(0); return sessionCommandHandSkill(targetUnitId, false); }
// Right-click world targets use packet 0x0C on press and 0x0F while held. Keep the packet callbacks
// as the command path: the held callback owns the native 50-subtile range check and out-of-range
// position-correction timer before it delegates to 0x0C.
// Fast-cast instrument: right-skill packets that reached the 0x0C/0x0F server callback, and player skill-do executions (casts).
static uint32_t g_rightSkillPackets = 0;
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_right_skill_packets() { return g_rightSkillPackets; }
static int sessionCommandRightSkillAt(int32_t x, int32_t y, bool held) {
    d2_client_cast_walk_reset();
    g_rightSkillRefusal = 1;
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pDynamicPath ||
        g_targetPlayer->dwAnimMode == PLRMODE_DEAD || g_targetPlayer->dwAnimMode == PLRMODE_DEATH) return 0;
    g_rightSkillRefusal = 2;
    if (g_sessionAttack.state == 1 || g_sessionAttack.state == 2) return 0;
    if (!held && !g_sessionInTick && sessionPlayerInActionMode()) { g_pendingRightSkill = { true, x, y }; return 0; }
    g_pendingRightSkill.set = false;
    // Retail 0x0F parses the coordinate before checking for a selected right skill or asking
    // whether that skill can cast. Let its callback own those checks so an in-range held aim always
    // refreshes the stale-coordinate timer; 0x0C press retains the session's existing usability gate.
    if (!held) {
        D2SkillStrc* skill = UNITS_GetRightSkill(g_targetPlayer);
        // The client casts its own selected skill (0x3C) when the server has not granted it: the server keeps Attack in the right hand.
        if (d2client_nova::selectedSkill() == 92 && (!skill || SKILLS_GetSkillIdFromSkill(skill, __FILE__, __LINE__) != 92)) d2client_nova::cast(92, true);
        g_rightSkillRefusal = 3;
        const int32_t useState = SKILLS_GetUseState(g_targetPlayer, skill);
        g_rightSkillRefusal = 10 + useState;
        d2client_blizzard::cast(SKILLS_GetSkillIdFromSkill(skill, __FILE__, __LINE__), nullptr, x, y);
        d2client_nova::cast(SKILLS_GetSkillIdFromSkill(skill, __FILE__, __LINE__), false);   // the client casts whether or not the server accepts the use; the refusal path plays the stsound
        // Retail's 0x0C callback (sub_6FC83340) swaps a skill that cannot be used (no mana, quantity or shape) for
        // plain Attack when its Skills.txt AttackNoMana flag is set (Double Swing, Frenzy unarmed: retail mode 7).
        D2SkillsTxt* txt = SKILLS_GetSkillsTxtRecordFromSkill(skill);
        const bool swapsToAttack = txt && (txt->dwFlags[0] & gdwBitMasks[SKILLSFLAGINDEX_ATTACKNOMANA]) &&
            (useState == SKILLUSTATE_NOMANA || useState == SKILLUSTATE_NOQUANTITY || useState == SKILLUSTATE_SHAPERESTRICT);
        if (useState != SKILLUSTATE_USABLE && !swapsToAttack) { g_castRefusedSkill = SKILLS_GetSkillIdFromSkill(skill, __FILE__, __LINE__); ++g_castRefusedSeq; return 0; }   // refused, but the stsound still plays
    }
    uint8_t packet[5] = { (uint8_t)(held ? 0x0F : 0x0C) };
    const uint16_t targetX = (uint16_t)x, targetY = (uint16_t)y;
    memcpy(packet + 1, &targetX, sizeof targetX);
    memcpy(packet + 3, &targetY, sizeof targetY);
    if (held) D2WEB_ResetPacketStream();
    ++g_rightSkillPackets;
    const int32_t result = held
        ? D2GAME_PACKETCALLBACK_Rcv0x0F_RightSkillHold_6FC84280(g_game, g_targetPlayer, packet, sizeof packet)
        : D2GAME_PACKETCALLBACK_Rcv0x0C_RightSkill_6FC83EF0(g_game, g_targetPlayer, packet, sizeof packet);
    if (held) D2WEB_PacketStream(&g_sessionPacketStreamLength, &g_sessionPacketStreamPackets);
    if (result != 0) { g_rightSkillRefusal = 100 + result; updateSessionSnapshot(); return result; }
    // Retail keeps separate server and client units. The 1.13c D2Game 0x0C/0x0F callback writes
    // the target position and starts the skill mode; the matching D2Client coordinate-arrival
    // handler then recomputes path facing from the unit's precise position to the target cell
    // center. This session has one unit for both roles, so reproduce that client path update here.
    // Its direction-vector calculation matches the local 1.13c Common helper at 0x6FD5CF40;
    // PathUpdateDirection preserves the path's normal turn smoothing.
    D2DynamicPathStrc* const path = g_targetPlayer->pDynamicPath;
    const uint8_t castDirection = static_cast<uint8_t>(PATH_ComputeDirectionFromPreciseCoords_6FDAC760(
        path->tGameCoords.dwPrecisionX, path->tGameCoords.dwPrecisionY,
        PATH_ToFP16Center(static_cast<uint16_t>(x)), PATH_ToFP16Center(static_cast<uint16_t>(y))));
    D2COMMON_10160_PathUpdateDirection(path, castDirection);
    g_rightSkillRefusal = 0;
    d2_client_note_move_cmd(0);
//    d2_client_note_local_action(2);   // 2A750 on the ground: mode 2 (WL), 23C00's first footfall
    updateSessionSnapshot();
    return 1;
}
// The client's own coordinate cast (D2Client+0x1BF20) turns the client unit toward the point the player clicked,
// which the replay carries apart from the 0x0C packet's target; call after the matching right-skill command.
// x,y are screen pixels: 1BF20 first runs 1.13c D2Client+0x3F5F0 on them (EBX = &x, EDI = &y; perspective off):
// x += [0x119960] - [0x11C418], y += [0x11995C], the view origin (hero pixel - half the 640x480 screen, y + 0x10),
// then D2Common #10474 (+0x4D8E0) maps the pixel to a subtile: X = (2y + x) >> 5, Y = (2y - x) >> 5. A22B0's
// #10434 then faces the unit toward that subtile's centre.
EMSCRIPTEN_KEEPALIVE int d2_session_client_cast_at(int32_t x, int32_t y) {
    const D2DynamicPathStrc* path = g_targetPlayer ? g_targetPlayer->pDynamicPath : nullptr;
    if (!path) return 0;
    const int32_t px = static_cast<int32_t>(path->tGameCoords.dwPrecisionX), py = static_cast<int32_t>(path->tGameCoords.dwPrecisionY);
    const int32_t sx = x + ((px - py) >> 12) - 640 / 2;
    const int32_t sy = y + ((px + py) >> 13) - 480 / 2 + 0x10;
    const int32_t tx = (2 * sy + sx) >> 5, ty = (2 * sy - sx) >> 5;
    clientFaceToward(static_cast<uint8_t>(PATH_ComputeDirectionFromPreciseCoords_6FDAC760(
        path->tGameCoords.dwPrecisionX, path->tGameCoords.dwPrecisionY,
        PATH_ToFP16Center(static_cast<uint16_t>(tx)), PATH_ToFP16Center(static_cast<uint16_t>(ty)))));
    return 1;
}
EMSCRIPTEN_KEEPALIVE int d2_session_command_right_skill_at(int32_t x, int32_t y) {
    return sessionCommandRightSkillAt(x, y, false);
}
EMSCRIPTEN_KEEPALIVE int d2_session_command_right_skill_hold_at(int32_t x, int32_t y) {
    return sessionCommandRightSkillAt(x, y, true);
}
// Read-only witness for the native held-coordinate correction timer (PlayerData+0x168).
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_right_skill_in_range_frame() {
    D2PlayerDataStrc* data = g_targetPlayer ? UNITS_GetPlayerData(g_targetPlayer) : nullptr;
    return data ? data->dwGameFrame : 0;
}
// The left hand's selected skill (V9b): the same path, UNITS_GetLeftSkill. The client uses it only when the
// left skill is not the plain Attack, which keeps its own primary-attack path.
EMSCRIPTEN_KEEPALIVE int d2_session_command_left_skill(uint32_t targetUnitId) { return sessionCommandHandSkill(targetUnitId, true); }

EMSCRIPTEN_KEEPALIVE int d2_session_equipped_quantity() {
    if (!g_sessionLive || !g_targetPlayer || !g_targetPlayer->pInventory) return -1;
    D2UnitStrc* item = INVENTORY_GetItemFromBodyLoc(g_targetPlayer->pInventory, BODYLOC_RARM);
    return item ? STATLIST_UnitGetStatValue(item, STAT_QUANTITY, 0) : -1;
}

// Deterministic progression fixture: initialize experience through the recovered stat list,
// then let the normal kill/experience path decide whether a level is earned. This never writes
// level or award points and is accepted only on an otherwise fresh level-1 player.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_experience(uint32_t experience) {
    if (!g_sessionLive || !g_targetPlayer ||
        STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_LEVEL, 0) != 1 ||
        STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_EXPERIENCE, 0) != 0 ||
        STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_STATPTS, 0) != 0 ||
        STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_SKILLPTS, 0) != 0) return 0;
    const uint32_t threshold = DATATBLS_GetLevelThreshold(g_targetPlayer->dwClassId, 1);
    if (experience >= threshold) return 0;
    STATLIST_SetUnitStat(g_targetPlayer, STAT_EXPERIENCE, (int32_t)experience, 0);
    STATLIST_SetUnitStat(g_targetPlayer, STAT_LASTEXP, 0, 0);
    g_sessionInitialExperience = (int32_t)experience;
    return 1;
}

// Fixture: raise the player to a level the way a kill's experience does
// (SUNITDMG_AddExperienceForPlayer: experience to the level's threshold, then PLAYERSTATS_LevelUp).
// XP bar floor: experience.txt threshold of the player's CURRENT level (retail's bar is
// (exp - this) / (next - this), so it empties on level-up instead of staying near full).
EMSCRIPTEN_KEEPALIVE int d2_session_level_threshold() {
    if (!g_sessionLive || !g_targetPlayer) return 0;
    const int32_t level = STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_LEVEL, 0);
    return level > 1 ? (int32_t)DATATBLS_GetLevelThreshold(g_targetPlayer->dwClassId, level - 1) : 0;
}

EMSCRIPTEN_KEEPALIVE int d2_session_fixture_level(uint32_t level) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || level < 1) return 0;
    const int32_t classId = (int32_t)g_targetPlayer->dwClassId;
    if ((int32_t)level > DATATBLS_GetMaxLevel(classId)) return 0;
    const uint32_t current = (uint32_t)STATLIST_UnitGetStatValue(g_targetPlayer, STAT_LEVEL, 0);
    const uint32_t experience = (uint32_t)STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_EXPERIENCE, 0);
    const uint32_t target = DATATBLS_GetLevelThreshold(classId, (int32_t)level - 1);
    if (target > experience) SUNITDMG_AddExperienceForPlayer(g_game, g_targetPlayer, current, target - experience);
    updateSessionSnapshot();
    return STATLIST_UnitGetStatValue(g_targetPlayer, STAT_LEVEL, 0) == (int32_t)level;
}

// a retail recording: one skill's base level on the session hero, set the way the stream's `cskill` directive sets it
// (D2Common AddSkill, then the base level at skill+0x28 = nSkillLevel). The skill tree's numbers are these.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_skill_level(uint32_t skillId, uint32_t level) {
    if (!g_sessionLive || !g_targetPlayer || skillId > UINT16_MAX) return 0;
    D2SkillStrc* skill = SKILLS_AddSkill(g_targetPlayer, (int)skillId);
    if (!skill) return 0;
    skill->nSkillLevel = (int32_t)level;
    return 1;
}

EMSCRIPTEN_KEEPALIVE int d2_session_command_add_stat(uint32_t statId, uint32_t count) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || statId > 15 || count < 1 || count > 100) return 3;
    uint8_t packet[3] = { 0x3A, 0, 0 };
    const uint16_t encoded = (uint16_t)(statId | ((count - 1) << 8));
    memcpy(packet + 1, &encoded, sizeof encoded);
    if (!sessionDispatchGate(0x3A)) return -2;
    return D2GAME_PACKETCALLBACK_Rcv0x3A_AddStatPoint_6FC86E80(g_game, g_targetPlayer, packet, sizeof packet);
}

// The player's class skill tree, straight out of Skills.txt and SkillDesc.txt.
//
// The client cannot invent this: which skills a class has, which of the three tabs each sits on and
// at which row and column are retail table data, and so is the level a skill needs. Without it a
// skill tree would be a hand-drawn picture of one. The CURRENT level of
// each skill comes from the player's own skill list, so the panel shows what the character has, not
// what the table allows.
//
// One record per skill of the player's class, in skill-id order:
//   id, page (1..3), row, column, requiredLevel, baseLevel, iconCel, nameStr, descStr, manaStr, canLearn,
//   manaLine, manaCost
// iconCel indexes SkillIcon.dc6, so the panel draws retail's icon for the skill rather than one
// picked by position; nameStr/descStr/manaStr are SkillDesc's wStrName/wStrLong/wStrMana -- .tbl
// indices for d2_lang_string_by_index -- so the panel's hover text is retail's own string and not a
// sentence this client made up.
//
// canLearn is retail's answer to "may this character put a point here right now", and it is the
// engine's own predicate, not a client rule: sessionSkillCanLearn() below runs exactly the checks
// D2GAME_PACKETCALLBACK_Rcv0x3B_AddSkillPoint_6FC86EF0 runs before it spends a point. Before it
// existed the client offered a "+" on any skill whose table level was met and whose invested level
// was under 20, which offers a point on a skill retail rejects for its PREREQUISITES
//.
//
// manaLine/manaCost are the tooltip's mana line (V19), and both are the ENGINE's answer, not a
// client rule: manaLine is SkillDesc's own line table (nDescLine[17] -- descline1..6, dsc2line1..4,
// dsc3line1..7 in that order) carrying a 1 on some line, which is where retail draws "Mana Cost: "
// (measured on retail 1.13c's own frame of Bash's tooltip, 2026-09-27: descline1, and NOT on Frenzy's,
// whose lines are 5/52/52/2/2); manaCost is SKILLS_GetManaCosts -- the same number the game spends,
// 1/256 units -- at the level the tooltip describes: level 1 for a skill the character has not
// learned (retail's frame reads "First Level" then "Mana Cost: 2" for Bash and "Mana Cost: 12.5" for
// Whirlwind, both unlearned), the skill's own level otherwise. A negative cost (Magic Arrow's falls
// with level) is reported as 0.
struct D2SessionSkill { uint16_t id, page, row, column, requiredLevel, baseLevel, iconCel, nameStr, descStr, manaStr, canLearn, manaLine, manaCost; uint16_t level; int16_t bonus; };
static_assert(sizeof(D2SessionSkill) == 30, "D2SessionSkill is 30 bytes");

// D2Game.0x6FC86EF0's precondition, in its own order and out of its own tables: class skill,
// required skills (prerequisites), required attributes, the skill's maximum level (wMaxLvl, 20 when
// the table says <= 0, exactly as the handler defaults it) and the skill points the next level costs
// (dwSkPoints through the engine's own SKILLS_EvaluateSkillFormula -- the handler spends the same
// number). True means the 0x3B handler would accept the point.
static bool sessionSkillCanLearn(D2UnitStrc* unit, int32_t id, D2SkillsTxt* txt) {
    if (!D2GAME_CheckSkillIfUnitsClassSkill_6FD11B50(unit, id)) return false;
    if (!SKILLS_CheckRequiredSkills(unit, id) || !SKILLS_CheckRequiredAttributes(unit, id)) return false;
    int32_t maxLvl = txt->wMaxLvl;
    if (maxLvl <= 0) maxLvl = 20;
    D2SkillStrc* known = SKILLS_GetSkillById(unit, id, -1u);
    const int32_t level = known ? SKILLS_GetSkillLevel(unit, known, 0) : 0;
    if (level >= maxLvl) return false;
    int32_t cost = 1;
    if (txt->dwSkPoints != -1)
        cost = SKILLS_EvaluateSkillFormula(unit, txt->dwSkPoints, id, known ? SKILLS_GetSkillLevel(unit, known, 1) : 0);
    return STATLIST_GetUnitBaseStat(unit, STAT_SKILLPTS, 0) >= cost;
}

EMSCRIPTEN_KEEPALIVE uint32_t d2_session_class_skills(D2SessionSkill* out, uint32_t cap) {
    if (!g_sessionLive || !g_targetPlayer || !sgptDataTables || !out) return 0;
    const int charClass = (int)g_targetPlayer->dwClassId;
    uint32_t n = 0;
    for (int id = 0; id < sgptDataTables->nSkillsTxtRecordCount && n < cap; ++id) {
        D2SkillsTxt* skill = DATATBLS_GetSkillsTxtRecord(id);
        if (!skill || skill->nCharClass != charClass) continue;
        D2SkillDescTxt* desc = DATATBLS_GetSkillDescTxtRecord(skill->wSkillDesc);
        // A skill with no description record has no place on the tree to draw it at.
        if (!desc || !desc->nSkillPage) continue;
        D2SessionSkill& row = out[n++];
        row.id = (uint16_t)id;
        row.page = desc->nSkillPage; row.row = desc->nSkillRow; row.column = desc->nSkillColumn;
        row.requiredLevel = skill->wReqLevel;
        row.iconCel = desc->nIconCel; row.nameStr = desc->wStrName;
        row.descStr = desc->wStrLong; row.manaStr = desc->wStrMana;
        D2SkillStrc* known = SKILLS_GetHighestLevelSkillFromUnitAndId(g_targetPlayer, id);
        const int level = known ? SKILLS_GetSkillLevel(g_targetPlayer, known, FALSE) : 0;
        row.baseLevel = (uint16_t)(level > 0 ? level : 0);
        // The tree draws the skill's level WITH bonuses (D2Client+0x78C50 passes SKILLS_GetSkillLevel(.., TRUE) to
        // the icon draw 0x78820) and tints it by the sign of the bonus (D2Common #11146); both only for a skill
        // the unit holds, so a bonus never lights a skill with no record.
        const int total = known ? SKILLS_GetSkillLevel(g_targetPlayer, known, TRUE) : 0;
        row.level = (uint16_t)(total > 0 ? total : 0);
        row.bonus = (int16_t)SKILLS_GetBonusSkillLevelFromSkillId(g_targetPlayer, id);
        // The tooltip's mana line (V19): does SkillDesc's own line table carry a 1 somewhere, and what
        // does the skill cost at the level the tooltip describes (level 1 when it is not learned)?
        bool manaLine = false;
        for (int i = 0; i < 17 && !manaLine; ++i) manaLine = desc->nDescLine[i] == 1 || desc->nDescLine[i] == 5;
        int cost = SKILLS_GetManaCosts(id, total > 0 ? total : 1);
        if (cost < 0) cost = 0;
        if (cost < (int)skill->wMinMana << 8) cost = (int)skill->wMinMana << 8;  // display clamp, as d2_session_skill_mana
        row.manaLine = manaLine ? 1 : 0;
        row.manaCost = (uint16_t)(cost > 0xFFFF ? 0xFFFF : cost);
        row.canLearn = sessionSkillCanLearn(g_targetPlayer, id, skill) ? 1 : 0;
    }
    return n;
}

// V19: SkillDesc's 17 tooltip lines for one skill at one level: type (nDescLine), .tbl indices for
// wDescTextA/B, and dwDescCalcA/B EVALUATED through the engine's own SKILLS_EvaluateSkillDescFormula
// (the number retail prints). Order is SkillDesc's: descline1..6, dsc2line1..4, dsc3line1..7.
struct D2SessionSkillLine { uint16_t type, textA, textB, pad; int32_t calcA, calcB; };
static_assert(sizeof(D2SessionSkillLine) == 16, "D2SessionSkillLine is 16 bytes");
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_skill_desc(uint32_t skillId, uint32_t level, D2SessionSkillLine* out) {
    if (!g_sessionLive || !g_targetPlayer || !sgptDataTables || !out) return 0;
    D2SkillsTxt* skill = DATATBLS_GetSkillsTxtRecord((int)skillId);
    D2SkillDescTxt* desc = skill ? DATATBLS_GetSkillDescTxtRecord(skill->wSkillDesc) : nullptr;
    if (!desc) return 0;
    for (int i = 0; i < 17; ++i) {
        out[i] = { desc->nDescLine[i], desc->wDescTextA[i], desc->wDescTextB[i], 0, 0, 0 };
        if (!desc->nDescLine[i]) continue;
        out[i].calcA = SKILLS_EvaluateSkillDescFormula(g_targetPlayer, desc->dwDescCalcA[i], (int)skillId, (int)level);
        out[i].calcB = SKILLS_EvaluateSkillDescFormula(g_targetPlayer, desc->dwDescCalcB[i], (int)skillId, (int)level);
    }
    return 17;
}

// V19: SKILLS_GetManaCosts (1/256 units) for any skill at any level -- the tooltip's Next Level block prices level+1.
EMSCRIPTEN_KEEPALIVE int d2_session_skill_mana(uint32_t skillId, uint32_t level) {
    if (!g_sessionLive || !sgptDataTables) return 0;
    int c = SKILLS_GetManaCosts((int)skillId, (int)level);
    // The tooltip's display clamps with minmana << 8 (retail Guided Arrow L30 shows 1, not 0.75); 0 hides the line.
    D2SkillsTxt* sk = DATATBLS_GetSkillsTxtRecord((int)skillId);
    if (sk && c < (int)sk->wMinMana << 8) c = (int)sk->wMinMana << 8;
    return c < 0 ? 0 : c;
}

// V9b: SkillDesc's wStrName | wStrShort << 16 for any skill id (general skills included), the two lines of
// the skill button's hover plate (retail frame: "Attack" / "normal attack").
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_skill_strings(uint32_t skillId) {
    if (!sgptDataTables) return 0;
    D2SkillsTxt* skill = DATATBLS_GetSkillsTxtRecord((int)skillId);
    D2SkillDescTxt* desc = skill ? DATATBLS_GetSkillDescTxtRecord(skill->wSkillDesc) : nullptr;
    return desc ? (uint32_t)desc->wStrName | ((uint32_t)desc->wStrShort << 16) : 0;
}

// D18 skill probe: one cast of Skills.txt id `skillId` at slvl 1 against the nearest monster, and what it did.
// Fixture only (mana/hp topped up, monster pulled into reach). out[] layout (int32, 64 words):
//  0 refusal/result (0 = cast started)   1 mode right after cast   2 bitmask of player modes seen (<32)
//  3 missiles created   4..7 first missile classIds   8 mana delta   9 target hp delta   10 new pets (monsters owned by us)
//  11 player-state words count(7) -> 12..18 new player states bits   19..25 new target states bits
//  26 packet count   27..42 distinct server->client packet ids   43 txt: range   44 srvstart   45 srvdo   46 srvmissile   47 auraState
//  48 passiveState  49 flags0  50 flags1  51 summon  52 monster id  53 player hp delta  54 player hp before
extern "C" int d2_realm_d2gs_packet_size(const uint8_t* packet, int length);
extern int32_t gSacrificeTrace[25];
EMSCRIPTEN_KEEPALIVE int d2_session_skill_probe(uint32_t skillId, int32_t* out) {
    memset(out, 0, 128 * sizeof(int32_t));
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pDynamicPath) { out[0] = -1; return -1; }
    D2UnitStrc* me = g_targetPlayer;
    D2SkillsTxt* txt = DATATBLS_GetSkillsTxtRecord((int)skillId);
    if (!txt) { out[0] = -2; return -2; }
    out[43] = txt->nRange; out[44] = txt->wSrvStartFunc; out[45] = txt->wSrvDoFunc; out[46] = txt->wSrvMissile; out[47] = txt->nAuraState;
    out[48] = txt->nPassiveState; out[49] = (int32_t)txt->dwFlags[0]; out[50] = (int32_t)txt->dwFlags[1]; out[51] = txt->wSummon;
    // settle: wait out any previous action, then find the nearest live monster
    for (int i = 0; i < 80 && me->dwAnimMode != PLRMODE_NEUTRAL && me->dwAnimMode != PLRMODE_TOWNNEUTRAL; ++i) d2_session_tick(1);
    g_sessionAttack.state = 0;
    updateSessionSnapshot();
    D2UnitStrc* mon = nullptr; int bestD = 1 << 30;
    const int px = CLIENTS_GetUnitX(me), py = CLIENTS_GetUnitY(me);
    for (const D2SessionUnit& u : g_sessionUnits) {
        if (u.type != UNIT_MONSTER || u.hp <= 0) continue;
        D2UnitStrc* m = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)u.id);
        if (!m || !m->pDynamicPath || m->dwAnimMode == MONMODE_DEAD || !(m->dwFlags & UNITFLAG_TARGETABLE)) continue;
        const int d = (u.x - px) * (u.x - px) + (u.y - py) * (u.y - py);
        if (d < bestD) { bestD = d; mon = m; }
    }
    if (!mon) { out[0] = -3; return -3; }
    out[52] = (int32_t)mon->dwUnitId;
    // pull the monster into reach (melee: 2 subtiles, else 7), try the four sides
    const int dist = txt->nRange == 1 ? 2 : 7;
    const int off[4][2] = { { dist, 0 }, { -dist, 0 }, { 0, dist }, { 0, -dist } };
    bool placed = false;
    for (auto& o : off) {
        D2ActiveRoomStrc* dest = nullptr;
        for (D2DrlgRoomStrc* roomEx = g_lastLevel ? g_lastLevel->pFirstRoomEx : nullptr; roomEx; roomEx = roomEx->pDrlgRoomNext)
            if (roomEx->pRoom) { D2ActiveRoomStrc* r = COLLISION_GetRoomBySubTileCoordinates(roomEx->pRoom, px + o[0], py + o[1]); if (r) { dest = r; break; } }
        if (dest && sub_6FCBDFE0(g_game, mon, dest, px + o[0], py + o[1], 0, 0)) { placed = true; break; }
    }
    if (!placed) { out[0] = -4; return -4; }
    // corpse skills (Find Potion / Find Item / Grim Ward) target a dead monster
    const bool corpseSkill = txt->wSrvDoFunc == 69 || txt->wSrvDoFunc == 72 || txt->wSrvDoFunc == 75;
    if (corpseSkill) { SUNITDMG_KillMonster(g_game, mon, me, 1); for (int i = 0; i < 120 && mon->dwAnimMode != MONMODE_DEAD; ++i) d2_session_tick(1); }
    // unit-targeted skills (melee, corpse skills, Fist of the Heavens) are cast at the monster, the rest at its tile
    const bool unitCast = txt->nRange == 1 || corpseSkill || txt->wSrvDoFunc == 80;
    SKILLS_AssignSkill(me, (int)skillId, 1, FALSE, __FILE__, __LINE__);
    STATLIST_SetUnitStat(me, STAT_MAXMANA, 5000 << 8, 0); STATLIST_SetUnitStat(me, STAT_MANA, 2500 << 8, 0);
    STATLIST_SetUnitStat(me, STAT_MAXHP, 5000 << 8, 0); STATLIST_SetUnitStat(me, STAT_HITPOINTS, 5000 << 8, 0);
    const int nStates = sgptDataTables->nStatesTxtRecordCount > 224 ? 224 : sgptDataTables->nStatesTxtRecordCount;
    uint32_t myBefore[7] = {}, monBefore[7] = {};
    for (int s = 0; s < nStates; ++s) {
        if (STATES_CheckState(me, s)) myBefore[s >> 5] |= 1u << (s & 31);
        if (STATES_CheckState(mon, s)) monBefore[s >> 5] |= 1u << (s & 31);
    }
    // an aura starts when it is selected, so the state snapshot is taken before the select
    if (d2_session_command_select_skill(skillId, 0) != 0) { out[0] = -5; return -5; }
    std::vector<int32_t> missilesBefore, petsBefore;
    for (const D2SessionUnit& u : g_sessionUnits) {
        if (u.type == UNIT_MISSILE) missilesBefore.push_back((int32_t)u.id);
        if (u.type == UNIT_MONSTER) { D2UnitStrc* m = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)u.id); if (m && m->dwOwnerGUID == me->dwUnitId) petsBefore.push_back((int32_t)u.id); }
    }
    const int movedX0 = CLIENTS_GetUnitX(me), movedY0 = CLIENTS_GetUnitY(me);
    const int32_t myHpBefore = STATLIST_UnitGetStatValue(me, STAT_HITPOINTS, 0) >> 8;
    const int32_t manaBefore = STATLIST_UnitGetStatValue(me, STAT_MANA, 0) >> 8, hpBefore = STATLIST_UnitGetStatValue(mon, STAT_HITPOINTS, 0) >> 8;
    D2WEB_ResetPacketStream();
    const int32_t rawManaBefore = STATLIST_UnitGetStatValue(me, STAT_MANA, 0);
    int result;
    if (unitCast) result = d2_session_command_right_skill((uint32_t)mon->dwUnitId);
    else result = d2_session_command_right_skill_at(CLIENTS_GetUnitX(mon), CLIENTS_GetUnitY(mon));
    out[0] = result == 1 ? 0 : (g_rightSkillRefusal ? g_rightSkillRefusal : 1000 + result);
    out[1] = (int32_t)me->dwAnimMode;
    // cost witness (raw 1/256): what Skills.txt prices the skill at, and what the first cast and the next tick took off the pool
    out[120] = SKILLS_GetManaCosts((int)skillId, 1); int32_t prevMana = rawManaBefore, maxDrop = 0;
    { const int32_t m = STATLIST_UnitGetStatValue(me, STAT_MANA, 0); if (prevMana - m > maxDrop) maxDrop = prevMana - m; prevMana = m; }
    uint32_t modes = 0; int nMis = 0, nPk = 0, nIds = 0; uint32_t sawIds[16] = {};
    std::vector<int32_t> seenMis = missilesBefore; int newPets = 0;
    memset(gSacrificeTrace, 0, sizeof gSacrificeTrace);
    int casts = 1;
    for (int f = 0; f < 160; ++f) {
        if (me->dwAnimMode < 32) modes |= 1u << me->dwAnimMode;
        d2_session_tick(1);
        { const int32_t m = STATLIST_UnitGetStatValue(me, STAT_MANA, 0); if (prevMana - m > maxDrop) maxDrop = prevMana - m; prevMana = m; }
        // melee: swing again each time the hero is back to neutral (one swing can miss)
        if (txt->nRange == 1 && casts < 8 && me->dwAnimMode == PLRMODE_NEUTRAL && mon->dwAnimMode != MONMODE_DEAD) {
            g_sessionAttack.state = 0; ++casts; d2_session_command_right_skill((uint32_t)mon->dwUnitId);
        }
        updateSessionSnapshot();
        D2WEB_PacketStream(&g_sessionPacketStreamLength, &g_sessionPacketStreamPackets);
        const uint8_t* ps = D2WEB_PacketStream(nullptr, nullptr); const uint32_t pl = g_sessionPacketStreamLength;
        for (uint32_t o = 0; ps && o < pl;) {
            const int sz = d2_realm_d2gs_packet_size(ps + o, (int)(pl - o)); if (sz <= 0) break;
            ++nPk; bool known = false; for (int k = 0; k < nIds; ++k) known |= sawIds[k] == ps[o];
            if (!known && nIds < 16) sawIds[nIds++] = ps[o];
            o += (uint32_t)sz;
        }
        for (const D2SessionUnit& u : g_sessionUnits) {
            if (u.type == UNIT_MISSILE) {
                bool known = false; for (int32_t id : seenMis) known |= id == (int32_t)u.id;
                if (!known) { seenMis.push_back((int32_t)u.id); if (nMis < 4) out[4 + nMis] = u.classId; ++nMis; }
            }
        }
    }
    out[62] = (CLIENTS_GetUnitX(me) - movedX0) * (CLIENTS_GetUnitX(me) - movedX0) + (CLIENTS_GetUnitY(me) - movedY0) * (CLIENTS_GetUnitY(me) - movedY0); // movement skills (Charge, Leap) count as an effect
    { D2UnitStrc* r = INVENTORY_GetItemFromBodyLoc(me->pInventory, BODYLOC_RARM); D2UnitStrc* l = INVENTORY_GetItemFromBodyLoc(me->pInventory, BODYLOC_LARM);
      out[63] = (r ? (int32_t)r->dwClassId + 1 : 0) | (l ? ((int32_t)l->dwClassId + 1) << 16 : 0); } // equipped hand classIds+1, for the item-skill kit
    out[121] = maxDrop; // largest one-step drop of the mana pool over the run (raw 1/256)
    out[53] = (STATLIST_UnitGetStatValue(me, STAT_HITPOINTS, 0) >> 8) - myHpBefore; out[54] = myHpBefore; out[55] = casts; out[56] = (int32_t)mon->dwAnimMode; out[57] = UNITS_IsInMeleeRange(me, mon, 1); out[58] = STATLIST_UnitGetStatValue(mon, STAT_HITPOINTS, 0); out[59] = CLIENTS_GetUnitX(mon) - CLIENTS_GetUnitX(me); out[60] = CLIENTS_GetUnitY(mon) - CLIENTS_GetUnitY(me); out[61] = STATLIST_UnitGetStatValue(me, STAT_HITPOINTS, 0);
    for (int k = 0; k < 25; ++k) out[95 + k] = gSacrificeTrace[k];
    { const D2SeedStrc sd = me->pSeed, ts = mon->pSeed; int hits = 0;
      for (int k = 0; k < 50; ++k) hits += (SUNITDMG_GetResultFlags(g_game, me, mon, 0, 0) & DAMAGERESULTFLAG_SUCCESSFULHIT) ? 1 : 0;
      me->pSeed = sd; mon->pSeed = ts; out[91] = hits; out[92] = STATLIST_UnitGetStatValue(me, STAT_TOHIT, 0); out[93] = STATLIST_UnitGetStatValue(mon, STAT_ARMORCLASS, 0); }
    { const int32_t* tr = d2_session_damage_trace((uint32_t)mon->dwUnitId); for (int k = 0; k < 26; ++k) out[64 + k] = tr[k]; }
    out[2] = (int32_t)modes; out[3] = nMis; out[8] = (STATLIST_UnitGetStatValue(me, STAT_MANA, 0) >> 8) - manaBefore;
    out[9] = (STATLIST_UnitGetStatValue(mon, STAT_HITPOINTS, 0) >> 8) - hpBefore;
    for (const D2SessionUnit& u : g_sessionUnits) if (u.type == UNIT_MONSTER) {
        D2UnitStrc* m = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)u.id);
        if (m && m->dwOwnerGUID == me->dwUnitId) { bool known = false; for (int32_t id : petsBefore) known |= id == (int32_t)u.id; if (!known) ++newPets; }
    }
    out[10] = newPets; out[11] = 7;
    for (int s = 0; s < nStates; ++s) {
        if (STATES_CheckState(me, s) && !(myBefore[s >> 5] & (1u << (s & 31)))) out[12 + (s >> 5)] |= 1 << (s & 31);
        if (STATES_CheckState(mon, s) && !(monBefore[s >> 5] & (1u << (s & 31)))) out[19 + (s >> 5)] |= 1 << (s & 31);
    }
    out[26] = nPk; for (int k = 0; k < nIds; ++k) out[27 + k] = (int32_t)sawIds[k];
    return out[0];
}

EMSCRIPTEN_KEEPALIVE int d2_session_command_add_skill(uint32_t skillId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || skillId > UINT16_MAX) return 3;
    uint8_t packet[3] = { 0x3B, 0, 0 };
    const uint16_t encoded = (uint16_t)skillId;
    memcpy(packet + 1, &encoded, sizeof encoded);
    if (!sessionDispatchGate(0x3B)) return -2;
    if (d2_debug_trace_c3r) fprintf(stderr, "[c3r] 0x3B skill=%u frame=%u skillpts-before=%d\n",
        skillId, g_game->dwGameFrame, STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_SKILLPTS, 0));
    const int result = D2GAME_PACKETCALLBACK_Rcv0x3B_AddSkillPoint_6FC86EF0(g_game, g_targetPlayer, packet, sizeof packet);
    if (d2_debug_trace_c3r) fprintf(stderr, "[c3r] 0x3B skill=%u result=%d skillpts-after=%d\n",
        skillId, result, STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_SKILLPTS, 0));
    if (result == 0) g_sessionAdvancementSkillId = skillId;
    return result;
}

// A refused ground pickup, read back: the pickup handler's return and, at phase 0 right after the click,
// the hero's attached sound event and its audience, the item through D2GAME_SUNITMSG_FirstFn (1.13c
// 0x7B0A0, the 0x9C refresh) and the hero through D2GAME_UpdateUnit (1.13c 0x7AC50, the 0x2C sound),
// both to the session client. phase N > 0, after each later tick: the item's mode:room and flags &
// 0x1401, the hero's event.
static int g_d57PickupRet;
extern "C" void D2WEB_ResetPacketStream();
extern "C" const uint8_t* D2WEB_PacketStream(uint32_t*, uint32_t*);
EMSCRIPTEN_KEEPALIVE int d2_session_d57_record(uint32_t itemId, int phase, char* out, int cap) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !out || cap < 512) return 0;
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId);
    D2UnitStrc* hero = g_targetPlayer;
    auto who = [](D2UnitStrc* u, char* b, size_t n) { if (u) snprintf(b, n, "%u:%u", u->dwUnitType, u->dwUnitId); else snprintf(b, n, "-"); };
    int len = 0;
    if (phase == 0) {
        char aud[32]; who(hero->pUpdateUnit, aud, sizeof aud);
        len += (hero->dwFlags & UNITFLAG_HASEVENTSOUND)
            ? snprintf(out + len, cap - len, "ret %d\nD57 sound hero %u audience %s\n", g_d57PickupRet, (unsigned)hero->nUpdateType, aud)
            : snprintf(out + len, cap - len, "ret %d\nD57 sound hero none\n", g_d57PickupRet);
        auto grab = [&](const char* tag, auto send) {
            D2WEB_ResetPacketStream(); send();
            uint32_t n = 0; const uint8_t* p = D2WEB_PacketStream(&n, nullptr);
            len += snprintf(out + len, cap - len, "D57 %s ", tag);
            for (uint32_t i = 0; i < n && len + 4 < cap; ++i) len += snprintf(out + len, cap - len, "%02x", p[i]);
            len += snprintf(out + len, cap - len, "\n");
        };
        if (item) grab("refresh", [&] { D2GAME_SUNITMSG_FirstFn_6FCC5520(g_game, item, g_targetClient); });
        else len += snprintf(out + len, cap - len, "D57 refresh gone\n");
        grab("voice", [&] { D2GAME_UpdateUnit_6FCC6080(hero, g_targetClient); });
        D2WEB_ResetPacketStream();
        return len;
    }
    char hs[48] = "-";
    if (hero->dwFlags & UNITFLAG_HASEVENTSOUND) { D2UnitStrc* a = hero->pUpdateUnit; snprintf(hs, sizeof hs, "%u,%d:%d", (unsigned)hero->nUpdateType, a ? (int)a->dwUnitType : -1, a ? (int)a->dwUnitId : -1); }
    if (!item) return snprintf(out, cap, "D57 +%d item gone hero %s\n", phase - 1, hs);
    return snprintf(out, cap, "D57 +%d item %u:%d flags %x hero %s\n", phase - 1, item->dwAnimMode,
        UNITS_GetRoom(item) ? 1 : 0, item->dwFlags & 0x1401, hs);
}

EMSCRIPTEN_KEEPALIVE int d2_session_command_pickup(uint32_t itemUnitId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pInventory) return 0;
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemUnitId);
    if (!item || item->dwAnimMode != IMODE_ONGROUND || !UNITS_GetRoom(item)) return 0;
    if (!sessionDispatchGate(0x16)) return 0;   // a ground pickup is client packet 0x16
    g_sessionItemState.commandTarget = itemUnitId;
    D2WEB_ResetPacketStream();   // D26: publish what the pickup queued (the 0x22 sound) to d2_session_packet_stream, as the 0x13 command does
    g_d57PickupRet = sub_6FC828D0(g_targetPlayer, UNIT_ITEM, (int32_t)itemUnitId, 0, g_game);
    D2WEB_PacketStream(&g_sessionPacketStreamLength, &g_sessionPacketStreamPackets);
    // D57: a stack merge frees the ground item (the unit is gone); do not read it again
    if (!SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemUnitId)) {
        g_sessionItemState.result = 0;
        g_sessionPendingPickup = 0;
        updateSessionSnapshot();
        return 0;
    }
    if (INVENTORY_IsItemInInventory(g_targetPlayer->pInventory, item)) {
        g_sessionItemState.result = 1;
        g_sessionPendingPickup = 0;
        return 1;
    }
    if (item->dwAnimMode == IMODE_ONGROUND && UNITS_GetRoom(item)) {
        g_sessionItemState.result = 2;
        // Still on the ground: the command started a walk (F715). Name it so the tick completes the
        // interaction when the hero arrives, instead of leaving the pile unpicked on its tile.
        g_sessionPendingPickup = itemUnitId;
        return 2;
    }
    g_sessionItemState.result = 0;
    g_sessionPendingPickup = 0;
    return 0;
}

// F715 probe: the ground-pickup decision's own inputs, so a walk-to-item stall is measured instead of
// guessed at. words: [0] item dwAnimMode, [1] D2Common_10399(player,item), [2] the ITEM-case collision
// test, [3] PLAYER_IsBusy, [4] cursor item != null, [5] player dwAnimMode, [6] room(item) != null,
// [7..9] the player's interact slots 42/43/44 (type/value/...), [10] item already in inventory,
// [11] sub_6FC7FB90 (the interact TYPE the completion reads), [12] sub_6FC7FBB0 (its GUID),
// [13] item classId, [14] gate refuses.
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_pickup_probe(uint32_t itemUnitId) {
    static int32_t out[16];
    for (int i = 0; i < 16; ++i) out[i] = -1;
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pInventory) return out;
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemUnitId);
    if (!item) return out;
    out[0] = (int)item->dwAnimMode;
    out[1] = D2Common_10399(g_targetPlayer, item);
    out[2] = UNITS_TestCollisionBetweenInteractingUnits(g_targetPlayer, item, COLLIDE_MASK_PLAYER_FLYING) ? 1 : 0;
    out[3] = PLAYER_IsBusy(g_targetPlayer) ? 1 : 0;
    out[4] = INVENTORY_GetCursorItem(g_targetPlayer->pInventory) ? 1 : 0;
    out[5] = (int)g_targetPlayer->dwAnimMode;
    out[6] = UNITS_GetRoom(item) ? 1 : 0;
    D2PlayerDataStrc* pd = UNITS_GetPlayerData(g_targetPlayer);
    if (pd) { out[7] = (int)pd->unk0xA8[42]; out[8] = (int)pd->unk0xA8[43]; out[9] = (int)pd->unk0xA8[44]; }
    out[10] = INVENTORY_IsItemInInventory(g_targetPlayer->pInventory, item) ? 1 : 0;
    out[11] = sub_6FC7FB90(g_targetPlayer);
    out[12] = sub_6FC7FBB0(g_targetPlayer);
    out[13] = (int)item->dwClassId;
    out[14] = sessionDispatchGate(0x16) ? 1 : 0;
    return out;
}

// F715: result 2 from d2_session_command_pickup only says the item remains on the ground. For nearby
// rc=2 recovery, defer junk dropping while any player path still has unconsumed points. The local
// 1.13c mode callback's target-field write is not proven, so this deliberately ignores pTargetUnit.
EMSCRIPTEN_KEEPALIVE int d2_session_pickup_path_active(uint32_t itemUnitId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pDynamicPath) return 0;
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemUnitId);
    if (!item || item->dwAnimMode != IMODE_ONGROUND || !UNITS_GetRoom(item)) return 0;
    D2DynamicPathStrc* path = g_targetPlayer->pDynamicPath;
    return d2::monspawn::pickupPathHasRemainingPoints(path);
}

EMSCRIPTEN_KEEPALIVE int d2_session_command_drop(uint32_t itemUnitId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pInventory) return 0;
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemUnitId);
    if (!item || item->dwAnimMode != IMODE_STORED || ITEMS_GetInvPage(item) != INVPAGE_INVENTORY ||
        !INVENTORY_CompareWithItemsParentInventory(g_targetPlayer->pInventory, item)) return 0;
    int32_t removeError = 0;
    if (!sub_6FC446B0(g_game, g_targetPlayer, (int32_t)itemUnitId, &removeError, 1, 0, nullptr, 0)) return 0;
    if (INVENTORY_GetCursorItem(g_targetPlayer->pInventory) != item) return 0;
    sub_6FC44030(g_game, g_targetPlayer, (int32_t)itemUnitId);
    updateSessionSnapshot();
    return item->dwAnimMode == IMODE_ONGROUND && UNITS_GetRoom(item) ? 1 : 0;
}

// Drop `amount` gold at the hero's feet: 1.13c D2Game+0x12B90 (ITEMS_DropGoldPile), exactly what retail's `dropgold` step calls
// (no stat deduction; places at feet+(2,3) when that room exists, not 1.10f sub_6FC7C260's pick).
EMSCRIPTEN_KEEPALIVE int d2_session_command_dropgold(uint32_t amount) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    ITEMS_DropGoldPile(g_game, g_targetPlayer, amount);
    updateSessionSnapshot();
    return 1;
}

// Retail's drop-gold OK (D2Client+0xBFDF0 -> packet builder D2Client+0x14910): the 9-byte 0x50 packet
// [0x50][player guid][amount], answered by the server's own handler (D2Game+0x37F80 deducts the gold and
// creates the pile beside the hero). `out`, when non-null, receives the 9 packet bytes. An amount of 0 sends
// nothing, as the client's OK callback does. Returns 1 when the server accepted it.
EMSCRIPTEN_KEEPALIVE int d2_session_command_drop_gold_packet(uint32_t amount, uint8_t* out) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || amount == 0) return 0;
    uint8_t packet[9] = { 0x50 };
    const int32_t guid = (int32_t)g_targetPlayer->dwUnitId, value = (int32_t)amount;
    memcpy(packet + 1, &guid, 4); memcpy(packet + 5, &value, 4);
    if (out) memcpy(out, packet, sizeof packet);
    if (!sessionDispatchGate(0x50)) return -2;
    const bool ok = D2GAME_PACKETCALLBACK_Rcv0x50_DropGold_6FC88210(g_game, g_targetPlayer, packet, sizeof packet) == 0;
    updateSessionSnapshot();
    return ok;
}

// slot bit 0x100: use it on the mercenary (retail's 0x26 packet carries bUseOnMerc; D2Client+0x29990 sends the
// shift-held flag there for a right click on a belt box).
EMSCRIPTEN_KEEPALIVE int d2_session_command_use_belt(uint32_t slot) {
    const int32_t onMerc = (slot & 0x100) ? 1 : 0; slot &= 0xFF;
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pInventory || slot >= 16) return 0;
    if (g_targetPlayer->dwAnimMode == PLRMODE_DEAD || INVENTORY_GetCursorItem(g_targetPlayer->pInventory)) return 0;   // the key handler refuses a dead hero; the use routine refuses a held item
    D2UnitStrc* item = INVENTORY_GetItemFromBeltSlot(g_targetPlayer->pInventory, (int32_t)slot);   // the slot is the key column exactly, no fallback to a higher cell
    if (!item || item->dwAnimMode != IMODE_INBELT ||
        !INVENTORY_CompareWithItemsParentInventory(g_targetPlayer->pInventory, item)) return 0;
    // canUseItem (D2Client+0x29920): the items.txt row must exist (a missing one is fatal in retail), a row with the
    // unique (+0x129) or quest (+0x12A) byte set refuses, and the useable byte (+0x11D) must be 1.
    D2ItemsTxt* itemRow = DATATBLS_GetItemsTxtRecord(item->dwClassId);
    if (!itemRow || itemRow->nUnique == 1 || itemRow->nQuest == 1 || itemRow->nUseable != 1) return 0;
    // D2Client+0x81CB0: refused while the item carries a stat list with flag 2, or the hero has state 0x36.
    if (STATLIST_GetStatListFromUnitAndFlag(item, 2) || STATES_CheckState(g_targetPlayer, 0x36)) return 0;
    if (!sessionDispatchGate(0x26)) return 0;   // using a belt item is client packet 0x26
    int32_t error = 0;
    const int32_t id = item->dwUnitId;
    if (!sub_6FC49220(g_game, g_targetPlayer, id, CLIENTS_GetUnitX(g_targetPlayer), CLIENTS_GetUnitY(g_targetPlayer), &error, onMerc)) return 0;
    updateSessionSnapshot();
    return SUNIT_GetServerUnit(g_game, UNIT_ITEM, id) ? 0 : 1;
}

// Right-clicking an item in the inventory: retail's 0x20 UseGridItem, through its own handler (a
// Town Portal scroll opens a portal, a potion is drunk, a tome is read). 1 if the handler accepted.
EMSCRIPTEN_KEEPALIVE int d2_session_command_use_item(uint32_t itemUnitId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pDynamicPath) return 0;
    uint8_t packet[13] = { 0x20 };
    const int32_t id = (int32_t)itemUnitId;
    const int32_t x = g_targetPlayer->pDynamicPath->tGameCoords.wPosX, y = g_targetPlayer->pDynamicPath->tGameCoords.wPosY;
    memcpy(packet + 1, &id, 4); memcpy(packet + 5, &x, 4); memcpy(packet + 9, &y, 4);
    if (!sessionDispatchGate(0x20)) return 0;
    const int32_t result = D2GAME_PACKETCALLBACK_Rcv0x20_UseGridItem_6FC85990(g_game, g_targetPlayer, packet, sizeof packet);
    updateSessionSnapshot();
    return result == 0 ? 1 : 0;
}

// The client's second half of an item spell: 0x27 UseItemAction {target item, use item}. Identify's scroll or
// tome arms targeting on the 0x20 above (IFLAG_TARGETING); the next left click on an item sends this, and the
// same id twice (or no target) is the client cancelling. D2Game+0x6FC85B50 -> sub_6FC47470 decide everything:
// the identify, the tome charge or scroll consumed, and clearing the flag. 1 if the handler accepted.
EMSCRIPTEN_KEEPALIVE int d2_session_command_use_item_action(uint32_t targetItemId, uint32_t useItemId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer) return 0;
    uint8_t packet[9] = { 0x27 };
    const int32_t target = (int32_t)targetItemId, use = (int32_t)useItemId;
    memcpy(packet + 1, &target, 4); memcpy(packet + 5, &use, 4);
    if (!sessionDispatchGate(0x27)) return 0;
    const int32_t result = D2GAME_PACKETCALLBACK_Rcv0x27_UseItemAction_6FC85B50(g_game, g_targetPlayer, packet, sizeof packet);
    updateSessionSnapshot();
    return result == 0 ? 1 : 0;
}

// Probe (s000c step 25): what the Compelling Orb reads. Bit 0..: [left-hand weapon id | 0], and the inventory's own left-item GUID in *guid.
// s000c step 25 trace: snapshot the exact inputs OperateFunction53 reads before/after a click.
// Words: valid, objectClass, operateFn, objectMode, inRange, distance, playerX/Y, objectX/Y,
// leftItemGuid, leftWeaponId, leftWeaponBaseCode, bodyLoc, invPage, activeWeaponSet, nHits,
// bOrbSmashed, A3Q2 reward/primary, A3Q5 enter-area, playerLevel, objectLevel.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_compelling_orb_trace(uint32_t objectId, int32_t* out) {
    if (!out) return 0;
    for (int i = 0; i < 23; ++i) out[i] = -1;
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pInventory || !g_targetPlayer->pDynamicPath) return 0;
    D2UnitStrc* object = SUNIT_GetServerUnit(g_game, UNIT_OBJECT, (int32_t)objectId);
    if (!object) return 0;
    D2UnitStrc* weapon = INVENTORY_GetLeftHandWeapon(g_targetPlayer->pInventory);
    D2QuestDataStrc* quest = QUESTS_GetQuestData(g_game, QUEST_A3Q5_TRAVINCAL);
    D2Act3Quest5Strc* q = quest ? (D2Act3Quest5Strc*)quest->pQuestDataEx : nullptr;
    D2ActiveRoomStrc* playerRoom = UNITS_GetRoom(g_targetPlayer);
    D2ActiveRoomStrc* objectRoom = UNITS_GetRoom(object);
    D2BitBufferStrc* flags = UNITS_GetPlayerData(g_targetPlayer)->pQuestData[g_game->nDifficulty];
    out[0] = 1; out[1] = object->dwClassId;
    out[2] = object->pObjectData && object->pObjectData->pObjectTxt ? object->pObjectData->pObjectTxt->nOperateFn : -1;
    out[3] = object->dwAnimMode;
    out[4] = UNITS_IsObjectInInteractRange(g_targetPlayer, object) ? 1 : 0;
    out[5] = D2Common_10399(g_targetPlayer, object);
    out[6] = g_targetPlayer->pDynamicPath->tGameCoords.wPosX; out[7] = g_targetPlayer->pDynamicPath->tGameCoords.wPosY;
    out[8] = object->pStaticPath ? object->pStaticPath->tGameCoords.nX : -1; out[9] = object->pStaticPath ? object->pStaticPath->tGameCoords.nY : -1;
    out[10] = g_targetPlayer->pInventory->dwLeftItemGUID; out[11] = weapon ? weapon->dwUnitId : 0;
    out[12] = weapon ? (int32_t)ITEMS_GetBaseCode(weapon) : 0;
    out[13] = weapon ? ITEMS_GetBodyLocation(weapon) : -1; out[14] = weapon ? ITEMS_GetInvPage(weapon) : -1;
    out[15] = g_targetClient ? CLIENTS_GetWeaponSwitch(g_targetClient) : -1;
    out[16] = q ? q->nHits : -1; out[17] = q ? q->bOrbSmashed : -1;
    out[18] = QUESTRECORD_GetQuestState(flags, QUESTSTATEFLAG_A3Q2, QFLAG_REWARDGRANTED) ? 1 : 0;
    out[19] = QUESTRECORD_GetQuestState(flags, QUESTSTATEFLAG_A3Q2, QFLAG_PRIMARYGOALDONE) ? 1 : 0;
    out[20] = QUESTRECORD_GetQuestState(flags, QUESTSTATEFLAG_A3Q5, QFLAG_ENTERAREA) ? 1 : 0;
    out[21] = playerRoom ? DUNGEON_GetLevelIdFromRoom(playerRoom) : -1;
    out[22] = objectRoom ? DUNGEON_GetLevelIdFromRoom(objectRoom) : -1;
    return 23;
}

EMSCRIPTEN_KEEPALIVE uint32_t d2_session_left_hand_weapon(uint32_t* guid) {
    if (!g_sessionLive || !g_targetPlayer || !g_targetPlayer->pInventory) return 0;
    if (guid) *guid = g_targetPlayer->pInventory->dwLeftItemGUID;
    D2UnitStrc* w = INVENTORY_GetLeftHandWeapon(g_targetPlayer->pInventory);
    return w ? w->dwUnitId : 0;
}
EMSCRIPTEN_KEEPALIVE int d2_session_command_stow_body(uint32_t bodyLoc) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pInventory || bodyLoc >= 13) return 0;
    int32_t result = 0;
    if (!sub_6FC45B30(g_game, g_targetPlayer, (uint8_t)bodyLoc, 0, &result)) return 0;
    D2UnitStrc* cursor = INVENTORY_GetCursorItem(g_targetPlayer->pInventory);
    if (!cursor || !sub_6FC43E60(g_game, g_targetPlayer, cursor, 1, 0, __FILE__, __LINE__)) return 0;
    updateSessionSnapshot();
    return INVENTORY_GetItemFromBodyLoc(g_targetPlayer->pInventory, (int32_t)bodyLoc) ? 0 : 1;
}

// Retail's Swap Weapons, which its own Configure Controls screen binds to `W`. The client only asks: the
// swap is the engine's own recovered function -- D2GAME_PACKETCALLBACK_Rcv0x60_SwapWeapons_6FC888A0's
// `sub_6FC46840`, which moves the equipment between the two weapon sets, resets and re-assigns each
// hand's skill and flips the active set. This does the same two checks that handler does first (the
// swap only exists in an expansion game, and not while a skill is in use or the player is dead) and
// then reports whether the engine's ACTIVE SET actually moved, so a refusal and a no-op are the same
// answer to the caller. Returns 1 when the set switched.
EMSCRIPTEN_KEEPALIVE int d2_session_command_swap_weapons() {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetClient || !g_targetPlayer->pInventory) return 0;
    if (!g_game->bExpansion) return 0;
    if (UNITS_GetUsedSkill(g_targetPlayer) || SUNIT_IsDead(g_targetPlayer)) return 0;
    const uint8_t nBefore = CLIENTS_GetWeaponSwitch(g_targetClient);
    int32_t nRefused = 0;
    sub_6FC46840(g_game, g_targetPlayer, &nRefused);
    updateSessionSnapshot();
    return CLIENTS_GetWeaponSwitch(g_targetClient) != nBefore ? 1 : 0;
}

// Which weapon set is active (0 the primary pair, 1 the alternate), out of the engine's own client
// record -- the bit retail's save header writes at 0x10. -1 outside a session.
EMSCRIPTEN_KEEPALIVE int d2_session_weapon_switch() {
    if (!g_sessionLive || !g_targetClient) return -1;
    return (int)CLIENTS_GetWeaponSwitch(g_targetClient);
}

// The inventory panel's weapon boxes (d2client/inv_weapon_hands.h): which silhouettes and which
// two-hander ghost the panel draws for the active set. -1 outside a session.
EMSCRIPTEN_KEEPALIVE int d2_session_weapon_hands() {
    if (!g_sessionLive || !g_targetPlayer || !g_targetPlayer->pInventory) return -1;
    D2InventoryStrc* inv = g_targetPlayer->pInventory;
    D2UnitStrc* r = INVENTORY_GetItemFromBodyLoc(inv, BODYLOC_RARM);
    D2UnitStrc* l = INVENTORY_GetItemFromBodyLoc(inv, BODYLOC_LARM);
    return d2client_weapon_hands::rule(r != nullptr, l != nullptr, INVENTORY_GetWieldType(g_targetPlayer, inv),
                                       ITEMS_GetQuiverType(r), ITEMS_GetQuiverType(l));
}

EMSCRIPTEN_KEEPALIVE int d2_session_command_respawn() {
    if (!g_sessionLive || !g_game || !g_targetPlayer || g_targetPlayer->dwAnimMode != PLRMODE_DEAD) return 0;
    uint8_t packet = 0x41;
    const int32_t oldLevel = DUNGEON_GetLevelIdFromRoom(UNITS_GetRoom(g_targetPlayer));
    const int32_t result = D2GAME_PACKETCALLBACK_Rcv0x41_Resurrect_6FC87480(g_game, g_targetPlayer, &packet, 1);
    // Resurrect moves the player to town (D2GAME_PlayerChangeAct): the session adopts the new level
    // as it does after an exit, a portal or a caravan, or it keeps the level the body lies in.
    D2ActiveRoomStrc* room = UNITS_GetRoom(g_targetPlayer);
    if (result == 0 && room && DUNGEON_GetLevelIdFromRoom(room) != oldLevel) adoptSessionLevelChange(oldLevel, DUNGEON_GetLevelIdFromRoom(room));
    updateSessionSnapshot();
    return result == 0 && g_targetPlayer->dwAnimMode != PLRMODE_DEAD ? 1 : 0;
}

static uint32_t sessionInventoryItemCount(const D2InventoryStrc* inventory);
EMSCRIPTEN_KEEPALIVE int d2_session_command_recover_corpse(uint32_t corpseId) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pInventory) return 0;
    D2UnitStrc* corpse = SUNIT_GetServerUnit(g_game, UNIT_PLAYER, (int32_t)corpseId);
    if (!corpse || corpse == g_targetPlayer || corpse->dwAnimMode != PLRMODE_DEAD ||
        !STATES_CheckState(corpse, STATE_PLAYERBODY) || INVENTORY_GetOwnerId(corpse->pInventory) != g_targetPlayer->dwUnitId) return 0;
    // Recovered = the body's items came back to the player. The body itself is removed later in
    // the frame (SUNIT_RemoveUnit defers), so "still exists" right after is not by itself a refusal
    // -- s000a clicked its body, got its gear back, and read failure on that alone.
    //
    // sub_6FC80440's own gate (sub_6FC4AD80) moves the corpse's items one at a time and is NOT
    // atomic: a corpse with 10 items and room for only some of them still has `bItemRemoved` fire on
    // every item that DID fit, so `after > before` (the old check here) reads TRUE on a PARTIAL
    // recovery -- the exact case retail's own sound 0x17 (refused) is for, and the exact case
    // sub_6FC80440 leaves the corpse NOT freed, its remaining items still on it. The old check
    // reported that partial pass as a full recovery (1), and the caller (a click that reads 1 as
    // "done") never came back for what was left on the corpse -- act IV an earlier change, s000d-act4 step
    // 20 BOOTSTRAP S000_FROM=19: `lastCorpseRecovery {id:2,result:1}` recorded, and every one of the
    // ten items the hero wore at death (plt/fhl/tbl/tgl/lbt/amu/rin/rin/wax/kit) still read `nowhere`
    // -- not worn, not in the pack, not on the ground -- because they were still on body 2, which the
    // false "1" told the caller was already empty.
    //
    // sub_6FC80440 frees and removes the corpse ONLY when sub_6FC4AD80 returns true (full recovery);
    // on a partial or refused pass the corpse survives with its leftover items still in its OWN
    // inventory. That is the gate this reads instead: the corpse's own remaining item count, which is
    // already zeroed inside sub_6FC80440 (INVENTORY_FreeCorpse) on the full-success path even during
    // the one-frame window before SUNIT_RemoveUnit's deferred removal takes the unit away, so it does
    // not re-introduce the deferred-removal false-refusal the old check was written to avoid.
    sub_6FC80440(g_game, g_targetPlayer, corpse);
    updateSessionSnapshot();
    D2UnitStrc* stillThere = SUNIT_GetServerUnit(g_game, UNIT_PLAYER, (int32_t)corpseId);
    const uint32_t corpseLeft = stillThere ? sessionInventoryItemCount(stillThere->pInventory) : 0;
    return (!stillThere || corpseLeft == 0) ? 1 : 0;
}

static uint32_t sessionInventoryItemCount(const D2InventoryStrc* inventory) {
    if (!inventory) return 0;
    uint32_t count = INVENTORY_GetCursorItem(const_cast<D2InventoryStrc*>(inventory)) ? 1u : 0u;
    for (D2UnitStrc* item = INVENTORY_GetFirstItem(const_cast<D2InventoryStrc*>(inventory));
         item; item = INVENTORY_GetNextItem(item)) {
        if (INVENTORY_UnitIsItem(item)) ++count;
    }
    return count;
}

// Executes the recovered retail MJ item-section writer and current-version reader against the
// live player.  The returned record and byte buffer contain no wasm pointers.
EMSCRIPTEN_KEEPALIVE const D2SessionItemSaveRoundTrip* d2_session_item_save_roundtrip() {
    memset(&g_sessionItemSaveRoundTrip, 0, sizeof g_sessionItemSaveRoundTrip);
    g_sessionItemSaveRoundTrip.version = 1;
    g_sessionItemSaveRoundTrip.size = sizeof g_sessionItemSaveRoundTrip;
    g_sessionItemSaveRoundTrip.saveVersion = 96;
    g_sessionItemSaveBytes.clear();
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pInventory) {
        g_sessionItemSaveRoundTrip.result = 1;
        return &g_sessionItemSaveRoundTrip;
    }

    constexpr uint32_t kCapacity = 64u * 1024u;
    g_sessionItemSaveBytes.resize(kCapacity);
    g_sessionItemSaveRoundTrip.beforeInventoryCount = sessionInventoryItemCount(g_targetPlayer->pInventory);
    const int32_t written = PLRSAVE_WriteItems_6FC89B50(g_targetPlayer->pInventory,
        g_sessionItemSaveBytes.data(), kCapacity, 0, 0);
    if (written < 4 || (uint32_t)written > kCapacity) {
        g_sessionItemSaveBytes.clear();
        g_sessionItemSaveRoundTrip.result = 2;
        return &g_sessionItemSaveRoundTrip;
    }
    g_sessionItemSaveBytes.resize((size_t)written);
    g_sessionItemSaveRoundTrip.bytesWritten = (uint32_t)written;
    g_sessionItemSaveRoundTrip.itemCount = (uint32_t)g_sessionItemSaveBytes[2] |
        ((uint32_t)g_sessionItemSaveBytes[3] << 8);
    g_sessionItemSaveRoundTrip.sectionHash = sessionHashBytes(D2VEC_FNV_INIT,
        g_sessionItemSaveBytes.data(), g_sessionItemSaveBytes.size());
    if (g_sessionItemSaveBytes[0] == 'J' && g_sessionItemSaveBytes[1] == 'M')
        g_sessionItemSaveRoundTrip.flags |= 1u;
    if (g_sessionItemSaveRoundTrip.itemCount == g_sessionItemSaveRoundTrip.beforeInventoryCount)
        g_sessionItemSaveRoundTrip.flags |= 2u;

    SUNIT_RemoveAllItemsFromInventory(g_game, g_targetPlayer);
    int32_t consumed = 0;
    const int32_t readResult = PLRSAVE_ReadItems_6FC8B8A0(g_game, g_targetPlayer,
        g_sessionItemSaveBytes.data(), 96, written, 0, &consumed);
    g_sessionItemSaveRoundTrip.bytesConsumed = consumed > 0 ? (uint32_t)consumed : 0;
    if (readResult != 0) {
        g_sessionItemSaveRoundTrip.result = 0x100u + (uint32_t)readResult;
        return &g_sessionItemSaveRoundTrip;
    }
    if (consumed == written) g_sessionItemSaveRoundTrip.flags |= 4u;
    g_sessionItemSaveRoundTrip.afterInventoryCount = sessionInventoryItemCount(g_targetPlayer->pInventory);
    if (g_sessionItemSaveRoundTrip.afterInventoryCount == g_sessionItemSaveRoundTrip.beforeInventoryCount)
        g_sessionItemSaveRoundTrip.flags |= 8u;

    std::vector<uint8_t> replay(kCapacity);
    const int32_t replayWritten = PLRSAVE_WriteItems_6FC89B50(g_targetPlayer->pInventory,
        replay.data(), kCapacity, 0, 0);
    if (replayWritten > 0 && (uint32_t)replayWritten <= kCapacity) {
        g_sessionItemSaveRoundTrip.replayBytes = (uint32_t)replayWritten;
        g_sessionItemSaveRoundTrip.replayHash = sessionHashBytes(D2VEC_FNV_INIT, replay.data(), (size_t)replayWritten);
        if (replayWritten == written && !memcmp(replay.data(), g_sessionItemSaveBytes.data(), (size_t)written))
            g_sessionItemSaveRoundTrip.flags |= 16u;
    }
    g_sessionItemSaveRoundTrip.result = (g_sessionItemSaveRoundTrip.flags & 15u) == 15u ? 0u : 3u;
    updateSessionSnapshot();
    g_sessionItemSaveRoundTrip.hash = sessionHashBytes(D2VEC_FNV_INIT, &g_sessionItemSaveRoundTrip,
        offsetof(D2SessionItemSaveRoundTrip, hash));
    return &g_sessionItemSaveRoundTrip;
}

EMSCRIPTEN_KEEPALIVE const uint8_t* d2_session_item_save_bytes() {
    return g_sessionItemSaveBytes.data();
}

EMSCRIPTEN_KEEPALIVE uint32_t d2_session_item_save_length() {
    return (uint32_t)g_sessionItemSaveBytes.size();
}

// PY1 (PlugY stash pages): the stash (inventory page 4) as an "JM" item section -- the save path's own
// ITEMS_SerializeItemToBitstream per item, so the client keeps pages as opaque bytes. Non-destructive.
// Returns bytes written, 0 on failure.
EMSCRIPTEN_KEEPALIVE int d2_session_stash_export(uint8_t* out, int cap) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pInventory || !out || cap < 4) return 0;
    D2GAME_INVMODE_Last_6FC416D0(g_game, g_targetPlayer);   // as d2_session_save_d2s: drop session-only delivery flags
    uint8_t* w = out + 4; uint16_t n = 0;
    for (D2UnitStrc* i = INVENTORY_GetFirstItem(g_targetPlayer->pInventory); i; i = INVENTORY_GetNextItem(i)) {
        D2UnitStrc* item = INVENTORY_UnitIsItem(i);
        if (!item || item->dwAnimMode != IMODE_STORED || ITEMS_GetInvPage(item) != INVPAGE_STASH) continue;
        const int32_t len = ITEMS_SerializeItemToBitstream(item, w, (uint32_t)(out + cap - w), 1, 1, 0);
        if (len <= 0) return 0;
        w += len; ++n;
    }
    out[0] = 'J'; out[1] = 'M'; out[2] = (uint8_t)n; out[3] = (uint8_t)(n >> 8);
    return (int)(w - out);
}
// Delete every stash item (SUNIT_RemoveAllItemsFromInventory's per-item body, page 4 only).
EMSCRIPTEN_KEEPALIVE int d2_session_stash_clear() {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pInventory) return -1;
    D2InventoryStrc* inv = g_targetPlayer->pInventory; int removed = 0;
    for (D2UnitStrc* i = INVENTORY_GetFirstItem(inv), *next; i; i = next) {
        next = INVENTORY_GetNextItem(i);
        D2UnitStrc* item = INVENTORY_UnitIsItem(i);
        if (!item || item->dwAnimMode != IMODE_STORED || ITEMS_GetInvPage(item) != INVPAGE_STASH) continue;
        D2GAME_ITEMS_UpdateTransferredProperties_6FC424E0(g_game, item, g_targetPlayer, 0, 1);
        if (!INVENTORY_RemoveItemFromInventory(inv, item)) return -2;
        // The packet client drops the item as retail's server-side removal tells it (0x0A, D2GAME_RemoveItem's bSendPacket leg).
        if (D2ClientStrc* client = SUNIT_GetClientFromPlayer(g_targetPlayer, __FILE__, __LINE__))
            D2GAME_PACKETS_SendPacket0x0A_RemoveObject_6FCC5F80(item, client);
        SUNIT_RemoveUnit(g_game, item); ++removed;
    }
    updateSessionSnapshot();
    return removed;
}
// Parse a "JM" section back through the save loader (item creation + its stored page/x/y placement,
// so the grid is respected). 0 = ok, else the PLRSAVE error code.
EMSCRIPTEN_KEEPALIVE int d2_session_stash_insert(const uint8_t* bytes, int len) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !bytes || len < 4) return -1;
    std::vector<uint8_t> copy(bytes, bytes + len); int32_t consumed = 0;
    std::vector<D2UnitStrc*> before;
    if (g_targetPlayer->pInventory)
        for (D2UnitStrc* i = INVENTORY_GetFirstItem(g_targetPlayer->pInventory); i; i = INVENTORY_GetNextItem(i)) before.push_back(i);
    const int r = PLRSAVE_ReadItems_6FC8B8A0(g_game, g_targetPlayer, copy.data(), 96, len, 0, &consumed);
    // Each new stash item reaches the packet client as retail's put-in-container item action (0x9C, as InvMode does).
    D2ClientStrc* client = g_targetPlayer->pInventory ? SUNIT_GetClientFromPlayer(g_targetPlayer, __FILE__, __LINE__) : nullptr;
    if (client && r == 0)   // a failed read is not announced to the client
        for (D2UnitStrc* i = INVENTORY_GetFirstItem(g_targetPlayer->pInventory); i; i = INVENTORY_GetNextItem(i)) {
            D2UnitStrc* item = INVENTORY_UnitIsItem(i);
            if (!item || std::find(before.begin(), before.end(), i) != before.end()) continue;
            if (item->dwAnimMode != IMODE_STORED || ITEMS_GetInvPage(item) != INVPAGE_STASH) continue;
            D2GAME_SendP0x9C_ItemAction_PutInContainer_6FC3E980(client, g_targetPlayer, item, 0);
        }
    updateSessionSnapshot();
    return r;
}

static uint32_t sessionBaseStatsHash(D2UnitStrc* player, uint32_t* countOut) {
    D2StatStrc stats[512] = {};
    const int count = player && player->pStatListEx
        ? STATLIST_GetBaseStatsData((D2StatListStrc*)player->pStatListEx, stats, 512) : 0;
    std::sort(stats, stats + count, [](const D2StatStrc& a, const D2StatStrc& b) {
        return a.nStat != b.nStat ? a.nStat < b.nStat : a.nLayer < b.nLayer;
    });
    uint32_t hash = D2VEC_FNV_INIT;
    for (int i = 0; i < count; ++i) {
        hash = sessionHashBytes(hash, &stats[i].nStat, sizeof stats[i].nStat);
        hash = sessionHashBytes(hash, &stats[i].nLayer, sizeof stats[i].nLayer);
        hash = sessionHashBytes(hash, &stats[i].nValue, sizeof stats[i].nValue);
    }
    if (countOut) *countOut = (uint32_t)count;
    return hash;
}

// Current-version retail `gf` base-stat section. The recovered reader is applied to the exact
// writer output, then the live base-stat list is serialized again for an exact round-trip gate.
EMSCRIPTEN_KEEPALIVE const D2SessionStatsSaveRoundTrip* d2_session_stats_save_roundtrip() {
    memset(&g_sessionStatsSaveRoundTrip, 0, sizeof g_sessionStatsSaveRoundTrip);
    g_sessionStatsSaveRoundTrip.version = 1;
    g_sessionStatsSaveRoundTrip.size = sizeof g_sessionStatsSaveRoundTrip;
    g_sessionStatsSaveRoundTrip.saveVersion = 96;
    g_sessionStatsSaveBytes.clear();
    if (!g_sessionLive || !g_game || !g_targetPlayer) {
        g_sessionStatsSaveRoundTrip.result = 1;
        return &g_sessionStatsSaveRoundTrip;
    }
    constexpr size_t kCapacity = 4096;
    g_sessionStatsSaveBytes.resize(kCapacity);
    uint8_t* writeEnd = g_sessionStatsSaveBytes.data();
    g_sessionStatsSaveRoundTrip.beforeStatsHash = sessionBaseStatsHash(
        g_targetPlayer, &g_sessionStatsSaveRoundTrip.statEntries);
    g_sessionStatsSaveRoundTrip.hpBefore = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_HITPOINTS, 0);
    g_sessionStatsSaveRoundTrip.manaBefore = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_MANA, 0);
    const int32_t writeResult = PLRSAVE2_WritePlayerStats(g_game, g_targetPlayer, &writeEnd,
        g_sessionStatsSaveBytes.data() + kCapacity);
    if (writeResult || writeEnd <= g_sessionStatsSaveBytes.data() || writeEnd > g_sessionStatsSaveBytes.data() + kCapacity) {
        g_sessionStatsSaveRoundTrip.result = 2u + (uint32_t)writeResult;
        g_sessionStatsSaveBytes.clear();
        return &g_sessionStatsSaveRoundTrip;
    }
    g_sessionStatsSaveRoundTrip.bytesWritten = (uint32_t)(writeEnd - g_sessionStatsSaveBytes.data());
    g_sessionStatsSaveBytes.resize(g_sessionStatsSaveRoundTrip.bytesWritten);
    g_sessionStatsSaveRoundTrip.sectionHash = sessionHashBytes(D2VEC_FNV_INIT,
        g_sessionStatsSaveBytes.data(), g_sessionStatsSaveBytes.size());
    if (g_sessionStatsSaveBytes.size() >= 2 && g_sessionStatsSaveBytes[0] == 'g' && g_sessionStatsSaveBytes[1] == 'f')
        g_sessionStatsSaveRoundTrip.flags |= 1u;

    uint8_t* readEnd = g_sessionStatsSaveBytes.data();
    const int32_t readResult = PLRSAVE2_ReadStatsEx(g_game, g_targetPlayer, &readEnd,
        g_sessionStatsSaveBytes.data() + g_sessionStatsSaveBytes.size(), 96);
    g_sessionStatsSaveRoundTrip.bytesConsumed = (uint32_t)(readEnd - g_sessionStatsSaveBytes.data());
    if (readResult) {
        g_sessionStatsSaveRoundTrip.result = 0x100u + (uint32_t)readResult;
        return &g_sessionStatsSaveRoundTrip;
    }
    if (g_sessionStatsSaveRoundTrip.bytesConsumed == g_sessionStatsSaveRoundTrip.bytesWritten)
        g_sessionStatsSaveRoundTrip.flags |= 2u;
    g_sessionStatsSaveRoundTrip.afterStatsHash = sessionBaseStatsHash(g_targetPlayer, nullptr);
    g_sessionStatsSaveRoundTrip.hpAfter = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_HITPOINTS, 0);
    g_sessionStatsSaveRoundTrip.manaAfter = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_MANA, 0);
    if (g_sessionStatsSaveRoundTrip.afterStatsHash == g_sessionStatsSaveRoundTrip.beforeStatsHash &&
        g_sessionStatsSaveRoundTrip.hpAfter == g_sessionStatsSaveRoundTrip.hpBefore &&
        g_sessionStatsSaveRoundTrip.manaAfter == g_sessionStatsSaveRoundTrip.manaBefore)
        g_sessionStatsSaveRoundTrip.flags |= 4u;

    std::vector<uint8_t> replay(kCapacity);
    uint8_t* replayEnd = replay.data();
    const int32_t replayResult = PLRSAVE2_WritePlayerStats(g_game, g_targetPlayer, &replayEnd, replay.data() + replay.size());
    if (!replayResult) {
        g_sessionStatsSaveRoundTrip.replayBytes = (uint32_t)(replayEnd - replay.data());
        g_sessionStatsSaveRoundTrip.replayHash = sessionHashBytes(D2VEC_FNV_INIT, replay.data(), g_sessionStatsSaveRoundTrip.replayBytes);
        if (g_sessionStatsSaveRoundTrip.replayBytes == g_sessionStatsSaveRoundTrip.bytesWritten &&
            !memcmp(replay.data(), g_sessionStatsSaveBytes.data(), g_sessionStatsSaveRoundTrip.bytesWritten))
            g_sessionStatsSaveRoundTrip.flags |= 8u;
    }
    g_sessionStatsSaveRoundTrip.result = g_sessionStatsSaveRoundTrip.flags == 15u ? 0u : 3u;
    g_sessionStatsSaveRoundTrip.hash = sessionHashBytes(D2VEC_FNV_INIT, &g_sessionStatsSaveRoundTrip,
        offsetof(D2SessionStatsSaveRoundTrip, hash));
    return &g_sessionStatsSaveRoundTrip;
}

EMSCRIPTEN_KEEPALIVE const uint8_t* d2_session_stats_save_bytes() { return g_sessionStatsSaveBytes.data(); }
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_stats_save_length() { return (uint32_t)g_sessionStatsSaveBytes.size(); }

static uint32_t sessionSkillSemanticHash(D2UnitStrc* player, int32_t classId, int32_t skillCount) {
    uint32_t hash = D2VEC_FNV_INIT;
    for (int32_t i = 0; i < skillCount; ++i) {
        const int32_t skillId = SKILLS_GetClassSkillId(classId, i);
        const int32_t level = SKILLS_GetSkillLevel(player, SKILLS_GetSkillById(player, skillId, -1u), 0);
        hash = sessionHashBytes(hash, &skillId, sizeof skillId);
        hash = sessionHashBytes(hash, &level, sizeof level);
    }
    return hash;
}

// Two independent recovered sections in their native forms: `if` player skills and `WS`
// waypoints. They intentionally remain separate byte buffers rather than an invented container.
EMSCRIPTEN_KEEPALIVE const D2SessionCampaignSaveRoundTrip* d2_session_campaign_save_roundtrip() {
    memset(&g_sessionCampaignSaveRoundTrip, 0, sizeof g_sessionCampaignSaveRoundTrip);
    g_sessionCampaignSaveRoundTrip.version = 1;
    g_sessionCampaignSaveRoundTrip.size = sizeof g_sessionCampaignSaveRoundTrip;
    g_sessionCampaignSaveRoundTrip.saveVersion = 96;
    g_sessionSkillSaveBytes.clear(); g_sessionWaypointSaveBytes.clear();
    if (!g_sessionLive || !g_game || !g_targetPlayer) {
        g_sessionCampaignSaveRoundTrip.result = 1; return &g_sessionCampaignSaveRoundTrip;
    }
    D2ClientStrc* client = SUNIT_GetClientFromPlayer(g_targetPlayer, __FILE__, __LINE__);
    if (!client) { g_sessionCampaignSaveRoundTrip.result = 2; return &g_sessionCampaignSaveRoundTrip; }
    const int32_t classId = client->nClassId;
    const int32_t skillCount = SKILLS_GetPlayerSkillCount(classId);
    if (classId < 0 || classId >= 7 || skillCount < 0 || skillCount > 255) {
        g_sessionCampaignSaveRoundTrip.result = 3; return &g_sessionCampaignSaveRoundTrip;
    }
    g_sessionCampaignSaveRoundTrip.skillCount = (uint32_t)skillCount;
    g_sessionCampaignSaveRoundTrip.skillSemanticBefore = sessionSkillSemanticHash(g_targetPlayer, classId, skillCount);
    g_sessionSkillSaveBytes.assign((size_t)skillCount + 2, 0);
    g_sessionSkillSaveBytes[0] = 'i'; g_sessionSkillSaveBytes[1] = 'f';
    for (int32_t i = 0; i < skillCount; ++i) {
        const int32_t skillId = SKILLS_GetClassSkillId(classId, i);
        g_sessionSkillSaveBytes[(size_t)i + 2] = (uint8_t)SKILLS_GetSkillLevel(
            g_targetPlayer, SKILLS_GetSkillById(g_targetPlayer, skillId, -1u), 0);
    }
    g_sessionCampaignSaveRoundTrip.skillBytes = (uint32_t)g_sessionSkillSaveBytes.size();
    g_sessionCampaignSaveRoundTrip.skillHash = sessionHashBytes(D2VEC_FNV_INIT,
        g_sessionSkillSaveBytes.data(), g_sessionSkillSaveBytes.size());
    if (g_sessionSkillSaveBytes[0] == 'i' && g_sessionSkillSaveBytes[1] == 'f')
        g_sessionCampaignSaveRoundTrip.flags |= 1u;
    uint8_t* skillRead = g_sessionSkillSaveBytes.data();
    const int32_t skillReadResult = PLRSAVE2_ReadSkills(g_game, g_targetPlayer, &skillRead,
        g_sessionSkillSaveBytes.data() + g_sessionSkillSaveBytes.size(), 96, skillCount);
    g_sessionCampaignSaveRoundTrip.skillConsumed = (uint32_t)(skillRead - g_sessionSkillSaveBytes.data());
    if (skillReadResult) { g_sessionCampaignSaveRoundTrip.result = 0x100u + (uint32_t)skillReadResult; return &g_sessionCampaignSaveRoundTrip; }
    if (g_sessionCampaignSaveRoundTrip.skillConsumed == g_sessionCampaignSaveRoundTrip.skillBytes)
        g_sessionCampaignSaveRoundTrip.flags |= 2u;
    g_sessionCampaignSaveRoundTrip.skillSemanticAfter = sessionSkillSemanticHash(g_targetPlayer, classId, skillCount);
    if (g_sessionCampaignSaveRoundTrip.skillSemanticAfter == g_sessionCampaignSaveRoundTrip.skillSemanticBefore)
        g_sessionCampaignSaveRoundTrip.flags |= 4u;
    std::vector<uint8_t> skillReplay((size_t)skillCount + 2, 0);
    skillReplay[0] = 'i'; skillReplay[1] = 'f';
    for (int32_t i = 0; i < skillCount; ++i) {
        const int32_t skillId = SKILLS_GetClassSkillId(classId, i);
        skillReplay[(size_t)i + 2] = (uint8_t)SKILLS_GetSkillLevel(
            g_targetPlayer, SKILLS_GetSkillById(g_targetPlayer, skillId, -1u), 0);
    }
    g_sessionCampaignSaveRoundTrip.skillReplayHash = sessionHashBytes(D2VEC_FNV_INIT, skillReplay.data(), skillReplay.size());
    if (skillReplay == g_sessionSkillSaveBytes) g_sessionCampaignSaveRoundTrip.flags |= 8u;

    g_sessionWaypointSaveBytes.assign(256, 0);
    uint8_t* waypointWrite = g_sessionWaypointSaveBytes.data();
    const int32_t waypointWriteResult = PLRSAVE2_WriteWaypointData(g_game, g_targetPlayer, &waypointWrite,
        g_sessionWaypointSaveBytes.data() + g_sessionWaypointSaveBytes.size());
    if (waypointWriteResult) { g_sessionCampaignSaveRoundTrip.result = 0x200u + (uint32_t)waypointWriteResult; return &g_sessionCampaignSaveRoundTrip; }
    g_sessionWaypointSaveBytes.resize((size_t)(waypointWrite - g_sessionWaypointSaveBytes.data()));
    g_sessionCampaignSaveRoundTrip.waypointBytes = (uint32_t)g_sessionWaypointSaveBytes.size();
    g_sessionCampaignSaveRoundTrip.waypointHash = sessionHashBytes(D2VEC_FNV_INIT,
        g_sessionWaypointSaveBytes.data(), g_sessionWaypointSaveBytes.size());
    // Bytes 'W','S' -- see the marker note in PlrSaveItems.cpp. This check carried the reversed
    // order too, so the writer, the reader and this verdict all agreed on the wrong bytes and the
    // round-trip stayed green. Three copies of one mistake is why the marker now has one comment
    // explaining it rather than three constants repeating it.
    if (g_sessionWaypointSaveBytes.size() >= 2 && g_sessionWaypointSaveBytes[0] == 'W' && g_sessionWaypointSaveBytes[1] == 'S')
        g_sessionCampaignSaveRoundTrip.flags |= 16u;
    uint8_t* waypointRead = g_sessionWaypointSaveBytes.data();
    const int32_t waypointReadResult = PLRSAVE2_ReadWaypointData(g_game, g_targetPlayer, &waypointRead,
        g_sessionWaypointSaveBytes.data() + g_sessionWaypointSaveBytes.size(), 96);
    g_sessionCampaignSaveRoundTrip.waypointConsumed = (uint32_t)(waypointRead - g_sessionWaypointSaveBytes.data());
    if (waypointReadResult) { g_sessionCampaignSaveRoundTrip.result = 0x300u + (uint32_t)waypointReadResult; return &g_sessionCampaignSaveRoundTrip; }
    if (g_sessionCampaignSaveRoundTrip.waypointConsumed == g_sessionCampaignSaveRoundTrip.waypointBytes)
        g_sessionCampaignSaveRoundTrip.flags |= 32u;
    std::vector<uint8_t> waypointReplay(256, 0); uint8_t* waypointReplayEnd = waypointReplay.data();
    const int32_t waypointReplayResult = PLRSAVE2_WriteWaypointData(g_game, g_targetPlayer, &waypointReplayEnd,
        waypointReplay.data() + waypointReplay.size());
    waypointReplay.resize((size_t)(waypointReplayEnd - waypointReplay.data()));
    g_sessionCampaignSaveRoundTrip.waypointReplayHash = sessionHashBytes(D2VEC_FNV_INIT, waypointReplay.data(), waypointReplay.size());
    if (!waypointReplayResult && waypointReplay == g_sessionWaypointSaveBytes) g_sessionCampaignSaveRoundTrip.flags |= 64u;
    g_sessionCampaignSaveRoundTrip.result = g_sessionCampaignSaveRoundTrip.flags == 127u ? 0u : 4u;
    g_sessionCampaignSaveRoundTrip.hash = sessionHashBytes(D2VEC_FNV_INIT, &g_sessionCampaignSaveRoundTrip,
        offsetof(D2SessionCampaignSaveRoundTrip, hash));
    return &g_sessionCampaignSaveRoundTrip;
}

EMSCRIPTEN_KEEPALIVE const uint8_t* d2_session_skill_save_bytes() { return g_sessionSkillSaveBytes.data(); }
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_skill_save_length() { return (uint32_t)g_sessionSkillSaveBytes.size(); }
EMSCRIPTEN_KEEPALIVE const uint8_t* d2_session_waypoint_save_bytes() { return g_sessionWaypointSaveBytes.data(); }
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_waypoint_save_length() { return (uint32_t)g_sessionWaypointSaveBytes.size(); }

EMSCRIPTEN_KEEPALIVE const D2SessionQuestIntroSaveRoundTrip* d2_session_quest_intro_save_roundtrip() {
    memset(&g_sessionQuestIntroSaveRoundTrip, 0, sizeof g_sessionQuestIntroSaveRoundTrip);
    g_sessionQuestIntroSaveRoundTrip.version = 1; g_sessionQuestIntroSaveRoundTrip.size = sizeof g_sessionQuestIntroSaveRoundTrip;
    g_sessionQuestIntroSaveRoundTrip.saveVersion = 96; g_sessionQuestSaveBytes.clear(); g_sessionIntroSaveBytes.clear();
    memset(&g_sessionCorpseSaveRoundTrip, 0, sizeof g_sessionCorpseSaveRoundTrip); g_sessionCorpseSaveBytes.clear();
    if (!g_sessionLive || !g_game || !g_targetPlayer) { g_sessionQuestIntroSaveRoundTrip.result = 1; return &g_sessionQuestIntroSaveRoundTrip; }
    g_sessionQuestIntroSaveRoundTrip.gheedBefore = PLRINTRO_GetQuestIntroFlag(g_targetPlayer, g_game, MONSTER_GHEED);
    g_sessionQuestSaveBytes.assign(298, 0); uint8_t* qw = g_sessionQuestSaveBytes.data();
    int rc = D2WEB_PLRSAVE2_WriteQuestSection(g_targetPlayer, &qw, g_sessionQuestSaveBytes.data() + g_sessionQuestSaveBytes.size());
    if (rc) { g_sessionQuestIntroSaveRoundTrip.result = 0x100u + rc; return &g_sessionQuestIntroSaveRoundTrip; }
    g_sessionQuestIntroSaveRoundTrip.questBytes = (uint32_t)(qw - g_sessionQuestSaveBytes.data());
    g_sessionQuestIntroSaveRoundTrip.questHash = sessionHashBytes(D2VEC_FNV_INIT, g_sessionQuestSaveBytes.data(), g_sessionQuestSaveBytes.size());
    g_sessionQuestIntroSaveRoundTrip.questSemanticBefore = g_sessionQuestIntroSaveRoundTrip.questHash;
    if (g_sessionQuestSaveBytes[0] == 'W' && g_sessionQuestSaveBytes[1] == 'o' && g_sessionQuestSaveBytes[2] == 'o' && g_sessionQuestSaveBytes[3] == '!') g_sessionQuestIntroSaveRoundTrip.flags |= 1u;
    uint8_t* qr = g_sessionQuestSaveBytes.data(); rc = D2WEB_PLRSAVE2_ReadQuestSection(g_targetPlayer, &qr, g_sessionQuestSaveBytes.data() + g_sessionQuestSaveBytes.size());
    g_sessionQuestIntroSaveRoundTrip.questConsumed = (uint32_t)(qr - g_sessionQuestSaveBytes.data());
    if (rc) { g_sessionQuestIntroSaveRoundTrip.result = 0x200u + rc; return &g_sessionQuestIntroSaveRoundTrip; }
    if (g_sessionQuestIntroSaveRoundTrip.questConsumed == 298) g_sessionQuestIntroSaveRoundTrip.flags |= 2u;
    std::vector<uint8_t> qrep(298); uint8_t* qre = qrep.data(); rc = D2WEB_PLRSAVE2_WriteQuestSection(g_targetPlayer, &qre, qrep.data() + qrep.size());
    g_sessionQuestIntroSaveRoundTrip.questReplayHash = sessionHashBytes(D2VEC_FNV_INIT, qrep.data(), qrep.size());
    g_sessionQuestIntroSaveRoundTrip.questSemanticAfter = g_sessionQuestIntroSaveRoundTrip.questReplayHash;
    if (!rc && qrep == g_sessionQuestSaveBytes) g_sessionQuestIntroSaveRoundTrip.flags |= 4u;

    g_sessionIntroSaveBytes.assign(52, 0); uint8_t* iw = g_sessionIntroSaveBytes.data();
    rc = PLRSAVE2_WritePlrIntroSection(g_game, g_targetPlayer, &iw, g_sessionIntroSaveBytes.data() + g_sessionIntroSaveBytes.size());
    if (rc) { g_sessionQuestIntroSaveRoundTrip.result = 0x300u + rc; return &g_sessionQuestIntroSaveRoundTrip; }
    g_sessionQuestIntroSaveRoundTrip.introBytes = (uint32_t)(iw - g_sessionIntroSaveBytes.data());
    g_sessionQuestIntroSaveRoundTrip.introHash = sessionHashBytes(D2VEC_FNV_INIT, g_sessionIntroSaveBytes.data(), g_sessionIntroSaveBytes.size());
    g_sessionQuestIntroSaveRoundTrip.introSemanticBefore = g_sessionQuestIntroSaveRoundTrip.introHash;
    if (g_sessionIntroSaveBytes[0] == 1 && g_sessionIntroSaveBytes[1] == 0x77) g_sessionQuestIntroSaveRoundTrip.flags |= 8u;
    uint8_t* ir = g_sessionIntroSaveBytes.data(); rc = D2WEB_PLRSAVE2_ReadPlrIntroSection(g_targetPlayer, &ir, g_sessionIntroSaveBytes.data() + g_sessionIntroSaveBytes.size());
    g_sessionQuestIntroSaveRoundTrip.introConsumed = (uint32_t)(ir - g_sessionIntroSaveBytes.data());
    if (rc) { g_sessionQuestIntroSaveRoundTrip.result = 0x400u + rc; return &g_sessionQuestIntroSaveRoundTrip; }
    if (g_sessionQuestIntroSaveRoundTrip.introConsumed == 52) g_sessionQuestIntroSaveRoundTrip.flags |= 16u;
    std::vector<uint8_t> irep(52); uint8_t* ire = irep.data(); rc = PLRSAVE2_WritePlrIntroSection(g_game, g_targetPlayer, &ire, irep.data() + irep.size());
    g_sessionQuestIntroSaveRoundTrip.introReplayHash = sessionHashBytes(D2VEC_FNV_INIT, irep.data(), irep.size());
    g_sessionQuestIntroSaveRoundTrip.introSemanticAfter = g_sessionQuestIntroSaveRoundTrip.introReplayHash;
    if (!rc && irep == g_sessionIntroSaveBytes) g_sessionQuestIntroSaveRoundTrip.flags |= 32u;
    g_sessionQuestIntroSaveRoundTrip.gheedAfter = PLRINTRO_GetQuestIntroFlag(g_targetPlayer, g_game, MONSTER_GHEED);
    if (g_sessionQuestIntroSaveRoundTrip.gheedAfter == g_sessionQuestIntroSaveRoundTrip.gheedBefore) g_sessionQuestIntroSaveRoundTrip.flags |= 64u;
    g_sessionQuestIntroSaveRoundTrip.result = g_sessionQuestIntroSaveRoundTrip.flags == 127u ? 0u : 2u;
    g_sessionQuestIntroSaveRoundTrip.hash = sessionHashBytes(D2VEC_FNV_INIT, &g_sessionQuestIntroSaveRoundTrip, offsetof(D2SessionQuestIntroSaveRoundTrip, hash));
    return &g_sessionQuestIntroSaveRoundTrip;
}
EMSCRIPTEN_KEEPALIVE const uint8_t* d2_session_quest_save_bytes() { return g_sessionQuestSaveBytes.data(); }
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_quest_save_length() { return (uint32_t)g_sessionQuestSaveBytes.size(); }
EMSCRIPTEN_KEEPALIVE const uint8_t* d2_session_intro_save_bytes() { return g_sessionIntroSaveBytes.data(); }
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_intro_save_length() { return (uint32_t)g_sessionIntroSaveBytes.size(); }

static uint32_t sessionCorpseCount(D2InventoryStrc* inventory) {
    uint32_t count = 0;
    if (inventory) for (D2CorpseStrc* corpse = INVENTORY_GetFirstCorpse(inventory); corpse; corpse = INVENTORY_GetNextCorpse(corpse)) ++count;
    return count;
}
EMSCRIPTEN_KEEPALIVE const D2SessionCorpseSaveRoundTrip* d2_session_corpse_save_roundtrip() {
    memset(&g_sessionCorpseSaveRoundTrip, 0, sizeof g_sessionCorpseSaveRoundTrip);
    g_sessionCorpseSaveRoundTrip.version=1; g_sessionCorpseSaveRoundTrip.size=sizeof g_sessionCorpseSaveRoundTrip; g_sessionCorpseSaveRoundTrip.saveVersion=96;
    g_sessionCorpseSaveBytes.clear();
    if(!g_sessionLive||!g_game||!g_targetPlayer||!g_targetPlayer->pInventory){g_sessionCorpseSaveRoundTrip.result=1;return &g_sessionCorpseSaveRoundTrip;}
    g_sessionCorpseSaveRoundTrip.corpseCountBefore=sessionCorpseCount(g_targetPlayer->pInventory);
    g_sessionCorpseSaveBytes.assign(65536,0); uint8_t* w=g_sessionCorpseSaveBytes.data();
    int rc=PLRSAVE2_WriteCorpsesSection(g_game,g_targetPlayer,&w,g_sessionCorpseSaveBytes.data()+g_sessionCorpseSaveBytes.size(),0);
    if(rc){g_sessionCorpseSaveRoundTrip.result=0x100u+rc;return &g_sessionCorpseSaveRoundTrip;}
    g_sessionCorpseSaveRoundTrip.bytesWritten=(uint32_t)(w-g_sessionCorpseSaveBytes.data());g_sessionCorpseSaveBytes.resize(g_sessionCorpseSaveRoundTrip.bytesWritten);
    if(g_sessionCorpseSaveBytes.size()>=4){g_sessionCorpseSaveRoundTrip.sectionCount=(uint32_t)g_sessionCorpseSaveBytes[2]|((uint32_t)g_sessionCorpseSaveBytes[3]<<8);if(g_sessionCorpseSaveBytes[0]=='J'&&g_sessionCorpseSaveBytes[1]=='M')g_sessionCorpseSaveRoundTrip.flags|=1u;}
    g_sessionCorpseSaveRoundTrip.sectionHash=sessionHashBytes(D2VEC_FNV_INIT,g_sessionCorpseSaveBytes.data(),g_sessionCorpseSaveBytes.size());
    uint8_t* r=g_sessionCorpseSaveBytes.data();int32_t detail=0;rc=PLRSAVE2_ReadCorpses(g_game,g_targetPlayer,&r,g_sessionCorpseSaveBytes.data()+g_sessionCorpseSaveBytes.size(),96,&detail);
    g_sessionCorpseSaveRoundTrip.readerDetail=(uint32_t)detail;g_sessionCorpseSaveRoundTrip.bytesConsumed=(uint32_t)(r-g_sessionCorpseSaveBytes.data());
    if(rc){g_sessionCorpseSaveRoundTrip.result=0x200u+rc;return &g_sessionCorpseSaveRoundTrip;}if(g_sessionCorpseSaveRoundTrip.bytesConsumed==g_sessionCorpseSaveRoundTrip.bytesWritten)g_sessionCorpseSaveRoundTrip.flags|=2u;
    g_sessionCorpseSaveRoundTrip.corpseCountAfter=sessionCorpseCount(g_targetPlayer->pInventory);if(g_sessionCorpseSaveRoundTrip.corpseCountAfter==g_sessionCorpseSaveRoundTrip.corpseCountBefore+g_sessionCorpseSaveRoundTrip.sectionCount)g_sessionCorpseSaveRoundTrip.flags|=4u;
    std::vector<uint8_t> rep(65536,0);uint8_t* re=rep.data();rc=PLRSAVE2_WriteCorpsesSection(g_game,g_targetPlayer,&re,rep.data()+rep.size(),0);rep.resize((size_t)(re-rep.data()));
    g_sessionCorpseSaveRoundTrip.replayBytes=(uint32_t)rep.size();g_sessionCorpseSaveRoundTrip.replayHash=sessionHashBytes(D2VEC_FNV_INIT,rep.data(),rep.size());if(!rc&&rep==g_sessionCorpseSaveBytes)g_sessionCorpseSaveRoundTrip.flags|=8u;
    g_sessionCorpseSaveRoundTrip.result=g_sessionCorpseSaveRoundTrip.flags==15u?0u:2u;g_sessionCorpseSaveRoundTrip.hash=sessionHashBytes(D2VEC_FNV_INIT,&g_sessionCorpseSaveRoundTrip,offsetof(D2SessionCorpseSaveRoundTrip,hash));return &g_sessionCorpseSaveRoundTrip;
}
EMSCRIPTEN_KEEPALIVE const uint8_t* d2_session_corpse_save_bytes(){return g_sessionCorpseSaveBytes.data();}
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_corpse_save_length(){return(uint32_t)g_sessionCorpseSaveBytes.size();}

// The unit the last swing was aimed at, RE-RESOLVED BY ID, or null when the unit is gone.
//
// `g_sessionAttackTarget` is the only raw unit pointer this file keeps -- every other unit read in it
// resolves through `SUNIT_GetServerUnit` -- and it can outlive its unit. A monster that dies is killed
// and counted, and once `g_sessionTargetKillCounted` is set the per-tick read in `d2_session_tick` stops
// touching the pointer entirely; the unit is then freed (corpse decay) with nobody clearing it, and the
// NEXT `d2_session_attack` dereferences a freed unit and traps: `eval: RuntimeError: memory access out of
// bounds`, wasm frames 2567 <- 2566 <- the ccall, in `STATLIST_UnitGetStatValue` inside
// `d2_session_attack` (measured, s000d act IV step 18, three consecutive runs, the walk's 311th swing:
// F110). The id is already carried in `g_sessionAttack.targetId`, set at the same
// instant as the pointer at every one of the four sites that set it, so the lookup is free of new state.
// A lookup that does not hand back the same unit means the unit is gone: drop both cached pointers and
// report null, and the swing's record keeps the values the swing itself wrote.
static D2UnitStrc* sessionAttackTargetLive() {
    if (!g_sessionAttackTarget) return nullptr;
    if (!g_sessionLive || !g_game) return nullptr;
    D2UnitStrc* unit = SUNIT_GetServerUnit(g_game, UNIT_MONSTER, (int32_t)g_sessionAttack.targetId);
    if (unit != g_sessionAttackTarget) { g_sessionAttackTarget = nullptr; g_sessionAttackSkill = nullptr; return nullptr; }
    return unit;
}

EMSCRIPTEN_KEEPALIVE const D2SessionAttack* d2_session_attack() {
    g_sessionAttack.version = 1; g_sessionAttack.size = sizeof g_sessionAttack;
    if (g_sessionLive && g_game) g_sessionAttack.gameFrame = g_game->dwGameFrame;
    if (g_targetPlayer) g_sessionAttack.mode = g_targetPlayer->dwAnimMode;
    if (D2UnitStrc* target = sessionAttackTargetLive()) g_sessionAttack.targetHpAfter =
        STATLIST_UnitGetStatValue(target, STAT_HITPOINTS, 0);
    uint32_t hash = D2VEC_FNV_INIT; const uint8_t* bytes = (const uint8_t*)&g_sessionAttack;
    for (size_t i = 0; i < offsetof(D2SessionAttack, hash); ++i) { hash ^= bytes[i]; hash *= 16777619u; }
    g_sessionAttack.hash = hash; return &g_sessionAttack;
}

EMSCRIPTEN_KEEPALIVE const D2SessionProgress* d2_session_progress() {
    memset(&g_sessionProgress, 0, sizeof g_sessionProgress);
    g_sessionProgress.version = 1; g_sessionProgress.size = sizeof g_sessionProgress;
    if (!g_sessionLive || !g_game || !g_targetPlayer) return &g_sessionProgress;
    g_sessionProgress.gameFrame = g_game->dwGameFrame; g_sessionProgress.kills = g_sessionKills;
    if (D2UnitStrc* target = sessionAttackTargetLive()) {
        g_sessionProgress.targetId = target->dwUnitId;
        g_sessionProgress.targetHp = STATLIST_UnitGetStatValue(target, STAT_HITPOINTS, 0);
        g_sessionProgress.targetMode = target->dwAnimMode;
        if (g_sessionProgress.targetHp <= 0) g_sessionProgress.flags |= 1;
    }
    g_sessionProgress.playerExperience = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_EXPERIENCE, 0);
    g_sessionProgress.experienceDelta = g_sessionProgress.playerExperience - g_sessionInitialExperience;
    updateSessionSnapshot();
    uint32_t hash = D2VEC_FNV_INIT;
    for (const D2SessionUnit& unit : g_sessionUnits) if (unit.type == UNIT_ITEM) {
        ++g_sessionProgress.itemCount;
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&unit);
        // Unit ids come from a process-global allocator and drop scatter can choose either
        // adjacent free subtile after game teardown.  Hash the generated item identity/state,
        // excluding allocator identity and presentation position, for stable session replay.
        for (size_t i = offsetof(D2SessionUnit, type); i < offsetof(D2SessionUnit, x); ++i) {
            hash ^= bytes[i]; hash *= 16777619u;
        }
        for (size_t i = offsetof(D2SessionUnit, flags); i < sizeof unit; ++i) {
            hash ^= bytes[i]; hash *= 16777619u;
        }
    }
    g_sessionProgress.itemHash = hash;
    return &g_sessionProgress;
}

EMSCRIPTEN_KEEPALIVE const D2SessionAdvancement* d2_session_advancement() {
    memset(&g_sessionAdvancement, 0, sizeof g_sessionAdvancement);
    g_sessionAdvancement.version = 1; g_sessionAdvancement.size = sizeof g_sessionAdvancement;
    g_sessionAdvancement.skillId = g_sessionAdvancementSkillId;
    if (!g_sessionLive || !g_game || !g_targetPlayer) return &g_sessionAdvancement;
    g_sessionAdvancement.gameFrame = g_game->dwGameFrame;
    g_sessionAdvancement.level = STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_LEVEL, 0);
    g_sessionAdvancement.experience = STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_EXPERIENCE, 0);
    g_sessionAdvancement.lastExperience = STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_LASTEXP, 0);
    g_sessionAdvancement.nextExperience = STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_NEXTEXP, 0);
    g_sessionAdvancement.statPoints = STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_STATPTS, 0);
    g_sessionAdvancement.skillPoints = STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_SKILLPTS, 0);
    g_sessionAdvancement.strength = STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_STRENGTH, 0);
    g_sessionAdvancement.energy = STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_ENERGY, 0);
    g_sessionAdvancement.dexterity = STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_DEXTERITY, 0);
    g_sessionAdvancement.vitality = STATLIST_GetUnitBaseStat(g_targetPlayer, STAT_VITALITY, 0);
    if (D2SkillStrc* skill = SKILLS_GetSkillById(g_targetPlayer, (int32_t)g_sessionAdvancementSkillId, -1)) {
        g_sessionAdvancement.skillBaseLevel = SKILLS_GetSkillLevel(g_targetPlayer, skill, 0);
        g_sessionAdvancement.skillTotalLevel = SKILLS_GetSkillLevel(g_targetPlayer, skill, 1);
    }
    g_sessionAdvancement.hash = sessionHashBytes(D2VEC_FNV_INIT, &g_sessionAdvancement,
        offsetof(D2SessionAdvancement, hash));
    return &g_sessionAdvancement;
}

EMSCRIPTEN_KEEPALIVE const D2SessionItemState* d2_session_item_state() {
    const uint32_t commandTarget = g_sessionItemState.commandTarget;
    const uint32_t result = g_sessionItemState.result;
    memset(&g_sessionItemState, 0, sizeof g_sessionItemState);
    g_sessionItemState.version = 1; g_sessionItemState.size = sizeof g_sessionItemState;
    g_sessionItemState.commandTarget = commandTarget; g_sessionItemState.result = result;
    if (!g_sessionLive || !g_targetPlayer || !g_targetPlayer->pInventory) return &g_sessionItemState;
    updateSessionSnapshot();
    for (const D2SessionUnit& unit : g_sessionUnits) if (unit.type == UNIT_ITEM) ++g_sessionItemState.groundCount;
    uint32_t hash = D2VEC_FNV_INIT;
    for (D2UnitStrc* item = INVENTORY_GetFirstItem(g_targetPlayer->pInventory); item; item = INVENTORY_GetNextItem(item)) {
        if (!INVENTORY_UnitIsItem(item)) continue;
        ++g_sessionItemState.inventoryCount;
        const uint32_t fields[] = { (uint32_t)item->dwClassId, (uint32_t)item->dwAnimMode,
            (uint32_t)ITEMS_GetItemQuality(item), ITEMS_GetItemFlags(item), ITEMS_GetInvPage(item), ITEMS_GetBodyLocation(item) };
        hash = sessionHashBytes(hash, fields, sizeof fields);
    }
    if (D2UnitStrc* cursor = INVENTORY_GetCursorItem(g_targetPlayer->pInventory)) {
        g_sessionItemState.cursorItemId = cursor->dwUnitId; g_sessionItemState.flags |= 1;
    }
    g_sessionItemState.inventoryHash = hash;
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&g_sessionItemState);
    g_sessionItemState.hash = sessionHashBytes(D2VEC_FNV_INIT, bytes, offsetof(D2SessionItemState, hash));
    return &g_sessionItemState;
}

EMSCRIPTEN_KEEPALIVE const D2SessionMotion* d2_session_motion() {
    memset(&g_sessionMotion, 0, sizeof g_sessionMotion);
    g_sessionMotion.version = 1; g_sessionMotion.size = sizeof g_sessionMotion;
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pDynamicPath) return &g_sessionMotion;
    const D2DynamicPathStrc* path = g_targetPlayer->pDynamicPath;
    syncClientCastClock();
    syncClientNeutralClock();
    g_sessionMotion.gameFrame = g_game->dwGameFrame; g_sessionMotion.mode = g_targetPlayer->dwAnimMode;
    g_sessionMotion.x = path->tGameCoords.wPosX; g_sessionMotion.y = path->tGameCoords.wPosY;
    g_sessionMotion.targetX = g_sessionMovePending ? g_sessionTargetX : g_sessionMotion.x;
    g_sessionMotion.targetY = g_sessionMovePending ? g_sessionTargetY : g_sessionMotion.y;
    g_sessionMotion.pathPoints = path->dwPathPoints; g_sessionMotion.currentPoint = path->dwCurrentPointIdx;
    g_sessionMotion.framePrecise = g_clientCastClockActive ? g_clientCastFramePrecise
        : g_clientNeutralClockActive ? g_clientNeutralFramePrecise : g_targetPlayer->nSeqCurrentFramePrecise;
    g_sessionMotion.frameCountPrecise = g_targetPlayer->dwFrameCountPrecise;
    g_sessionMotion.animSpeed = g_clientCastClockActive ? g_clientCastAnimSpeed
        : g_clientNeutralClockActive ? g_clientNeutralAnimSpeed : g_targetPlayer->wAnimSpeed;
    g_sessionMotion.direction = g_clientFacingActive ? g_clientFacing.nDirection : path->nDirection;
    g_sessionMotion.moving = g_sessionMovePending;
    if (g_targetPlayer->dwAnimMode == PLRMODE_SEQUENCE) {
        g_sessionMotion.seqMode = static_cast<uint8_t>(g_targetPlayer->dwSeqMode);
        g_sessionMotion.seqFrame = static_cast<uint8_t>(g_targetPlayer->nSeqCurrentFramePrecise >> 8);
    }
    return &g_sessionMotion;
}

// The player's path position at the engine's own 16.16 precision (see D2SessionCoords). The shell's
// camera reads this instead of the whole-subtile x/y, which is what made the world step 16 px at a
// time instead of gliding the way retail's does.
EMSCRIPTEN_KEEPALIVE const D2SessionCoords* d2_session_player_coords() {
    memset(&g_sessionCoords, 0, sizeof g_sessionCoords);
    g_sessionCoords.version = 1; g_sessionCoords.size = sizeof g_sessionCoords;
    if (!g_sessionLive || !g_targetPlayer || !g_targetPlayer->pDynamicPath) return &g_sessionCoords;
    const D2DynamicPathStrc* path = g_targetPlayer->pDynamicPath;
    g_sessionCoords.preciseX = path->tGameCoords.dwPrecisionX;
    g_sessionCoords.preciseY = path->tGameCoords.dwPrecisionY;
    return &g_sessionCoords;
}
// Whether the player has a state (States.txt id) -- diagnosis for mode changes the engine refuses.
uint32_t D2StatList_StateSetSeq(uint32_t, uint32_t, uint32_t);
EMSCRIPTEN_KEEPALIVE int d2_session_state_set_seq(int type, int id, int stateId) { return (int)D2StatList_StateSetSeq(type, id, stateId); }
EMSCRIPTEN_KEEPALIVE int d2_session_player_has_state(int stateId) { return g_targetPlayer && STATES_CheckState(g_targetPlayer, stateId) ? 1 : 0; }
// Why d2_session_tick would not advance: each of its preconditions, as bits (1 live, 2 game,
// 4 player, 8 path, 16 last level, 32 player room), plus the player's hp and mode.
EMSCRIPTEN_KEEPALIVE int d2_session_tick_gate() {
    int b = 0;
    if (g_sessionLive) b |= 1; if (g_game) b |= 2; if (g_targetPlayer) b |= 4;
    if (g_targetPlayer && g_targetPlayer->pDynamicPath) b |= 8; if (g_lastLevel) b |= 16;
    if (g_targetPlayer && UNITS_GetRoom(g_targetPlayer)) b |= 32;
    return b;
}
// A unit by type and id, wherever it is: [exists, level, x, y, mode, flags, flagsEx, in a room,
// and for a monster: MonsterData nTypeFlag (MONTYPEFLAG_*: 0x02 superunique, 0x04 champion, 0x08
// unique, 0x10 minion), STAT_LEVEL, STAT_DAMAGEPERCENT, nMonUmod[0]] -- so a pack's damage is read
// against what it is, not inferred.
// [12] = the SuperUniques.txt row of a boss monster (MONSTERUNIQUE_GetBossHcIdx; -1 otherwise):
// the name retail prints over its life bar ("Shenk the Overseer" is row 42, Siege Boss).
// [13..16] F245: whether the unit is in STATE_ATTACHED, and the OWNER it stores when it is
// (dwOwnerType, dwOwnerGUID) plus its AI special state. Retail makes a unit that is MOUNTING another
// -- the act V Imp on a Barricade Tower / Siege Beast, `sub_6FD00EC0` -- non-targetable on purpose
// (`pUnit->dwFlags &= 0xFFFFFFF1u` clears TARGETABLE|CANBEATTACKED|ISVALIDTARGET; the same function's
// `UNITS_StoreOwner` both ways is what sets STOREOWNERINFO on it and on its mount), and the dismount
// (`sub_6FD01010`, through the imp's own "Imp Teleport" skill) restores both. So a live, full-hit-point
// unit that cannot be attacked is only a DEFECT when these say it is NOT attached and owns nothing.
static int32_t g_unitInfo[17];
// TEST ONLY (shrines1 scenario): make object `id` the shrine of shrines.txt row `row`, as the capture driver's
// D2CAPTURE_SESSION_SHRINE does on retail (InteractType + pShrineTxt), so one fixed level can operate every type.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_set_shrine(uint32_t id, int row) {
    D2UnitStrc* u = g_game ? SUNIT_GetServerUnit(g_game, UNIT_OBJECT, (int32_t)id) : nullptr;
    if (!u || !u->pObjectData) return 0;
    u->pObjectData->InteractType = (uint8_t)row;
    u->pObjectData->pShrineTxt = DATATBLS_GetShrinesTxtRecord(row);
    return 1;
}
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_unit_info(int type, uint32_t id) {
    memset(g_unitInfo, 0, sizeof g_unitInfo);
    D2UnitStrc* u = g_game ? SUNIT_GetServerUnit(g_game, type, (int32_t)id) : nullptr;
    if (!u) return g_unitInfo;
    D2ActiveRoomStrc* room = UNITS_GetRoom(u);
    g_unitInfo[0] = 1; g_unitInfo[1] = room ? DUNGEON_GetLevelIdFromRoom(room) : -1;
    g_unitInfo[2] = CLIENTS_GetUnitX(u); g_unitInfo[3] = CLIENTS_GetUnitY(u);
    g_unitInfo[4] = (int32_t)u->dwAnimMode; g_unitInfo[5] = (int32_t)u->dwFlags; g_unitInfo[6] = (int32_t)u->dwFlagEx; g_unitInfo[7] = room ? 1 : 0;
    if (u->dwUnitType == UNIT_MONSTER && u->pMonsterData) {
        g_unitInfo[8] = u->pMonsterData->nTypeFlag; g_unitInfo[11] = u->pMonsterData->nMonUmod[0];
    }
    if (u->dwUnitType == UNIT_MONSTER) {
        g_unitInfo[9] = STATLIST_UnitGetStatValue(u, STAT_LEVEL, 0);
        g_unitInfo[10] = STATLIST_UnitGetStatValue(u, STAT_DAMAGEPERCENT, 0);
    }
    g_unitInfo[12] = (u->dwUnitType == UNIT_MONSTER && u->pMonsterData && (u->pMonsterData->nTypeFlag & MONTYPEFLAG_SUPERUNIQUE)) ? MONSTERUNIQUE_GetBossHcIdx(u) : -1;
    g_unitInfo[13] = STATES_CheckState(u, STATE_ATTACHED) ? 1 : 0;
    g_unitInfo[14] = (int32_t)u->dwOwnerType; g_unitInfo[15] = (int32_t)u->dwOwnerGUID;
    g_unitInfo[16] = -1;
    if (u->dwUnitType == UNIT_MONSTER && u->pMonsterData) {
        D2AiControlStrc* ctl = AIGENERAL_GetAiControlFromUnit(u);
        if (ctl) g_unitInfo[16] = (int32_t)ctl->nAiSpecialState;   // AISPECIALSTATE_MOUNTING_UNIT is 16
    }
    return g_unitInfo;
}
// The player's room and its near list: [near count, near rooms active, near rooms of this level,
// player room tile x, y] -- retail keeps every near room active (the client's sight).
static int32_t g_nearInfo[5];
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_near_rooms() {
    memset(g_nearInfo, 0, sizeof g_nearInfo);
    D2ActiveRoomStrc* room = g_targetPlayer ? UNITS_GetRoom(g_targetPlayer) : nullptr;
    D2DrlgRoomStrc* ex = room ? room->pDrlgRoom : nullptr;
    if (!ex) return g_nearInfo;
    g_nearInfo[0] = ex->nRoomsNear; g_nearInfo[3] = ex->nTileXPos; g_nearInfo[4] = ex->nTileYPos;
    for (int i = 0; i < ex->nRoomsNear; ++i) {
        D2DrlgRoomStrc* n = ex->ppRoomsNear[i];
        if (!n) continue;
        if (n->pRoom) g_nearInfo[1]++;
        if (n->pLevel == ex->pLevel) g_nearInfo[2]++;
    }
    return g_nearInfo;
}
// a retail recording diff tool: our client view in the format of the retail captures, tick prefix left to the caller.
// RM = rooms sessionClientHoldsRoom() accepts, HERO/NEAR = the hero room and its ppRoomsNear, U = units in held rooms (type,id,class,mode hex, room rect).
static std::string g_clientView;
EMSCRIPTEN_KEEPALIVE const char* d2_session_client_view() {
    char b[160]; std::vector<std::string> rm, nl, ul; g_clientView.clear();
    auto rect = [](const D2ActiveRoomStrc* r) { char t[64]; snprintf(t, sizeof t, "%d,%d,%d,%d", r->tCoords.nSubtileX, r->tCoords.nSubtileY, r->tCoords.nSubtileWidth, r->tCoords.nSubtileHeight); return std::string(t); };
    D2ActiveRoomStrc* here = g_targetPlayer ? UNITS_GetRoom(g_targetPlayer) : nullptr;
    if (!here || !g_lastLevel) return "";
    for (D2DrlgLevelStrc* lv = g_lastLevel; lv; lv = lv->pNextLevel)
        for (D2DrlgRoomStrc* ex = lv->pFirstRoomEx; ex; ex = ex->pDrlgRoomNext) {
            D2ActiveRoomStrc* r = ex->pRoom; if (!r) continue; const bool rh = sessionClientHoldsRoom(r);
            if (rh) { snprintf(b, sizeof b, "RM %s,%u,%d", rect(r).c_str(), 0u, ex->nRoomsNear); rm.push_back(b); }
            for (D2UnitStrc* u = r->pUnitFirst; u; u = u->pRoomNext) {
                if (u->dwUnitType > UNIT_ITEM && u->dwUnitType != UNIT_MISSILE) continue;
                if (u->dwUnitType == UNIT_MONSTER ? !g_heldMonsters.count(u->dwUnitId) : !rh) continue;   // monsters: the retail send set, others: the room proxy
                int vm = (int)u->dwAnimMode;
                if (u == g_targetPlayer) { static int prevRun; const int was = prevRun; prevRun = vm == PLRMODE_RUN; if (was && vm == PLRMODE_RUN && DUNGEON_IsRoomInTown(r)) vm = PLRMODE_TOWNWALK; }   // client lags a tick: the retail client shows its own unit's town run as town walk (server mode stays RUN: a retail recording trace)
                snprintf(b, sizeof b, "U %u,%x,%x,%x,%s", (unsigned)u->dwUnitType, (unsigned)u->dwUnitId, (unsigned)u->dwClassId, (unsigned)vm, rect(r).c_str()); ul.push_back(b);
            }
        }
    for (int32_t i = 0; here->pDrlgRoom && i < here->pDrlgRoom->nRoomsNear; ++i) { D2DrlgRoomStrc* n = here->pDrlgRoom->ppRoomsNear[i]; if (n && n->pRoom) nl.push_back("NEAR " + rect(n->pRoom)); }
    std::sort(rm.begin(), rm.end()); std::sort(nl.begin(), nl.end()); std::sort(ul.begin(), ul.end());
    for (auto& l : rm) g_clientView += l + "\n";
    g_clientView += "HERO " + rect(here) + "\n";
    for (auto& l : nl) g_clientView += l + "\n";
    for (auto& l : ul) g_clientView += l + "\n";
    return g_clientView.c_str();
}
EMSCRIPTEN_KEEPALIVE void d2_session_hold_world(int hold) { g_worldHold = hold != 0; g_worldHoldBuilt = false; }
// The snapshot's level word (D2SessionSnapshot.level) without rebuilding the world lists: the shell's
// pre-advance ambient check wants this one number every update. 0 exactly when the snapshot would be empty.
EMSCRIPTEN_KEEPALIVE int32_t d2_session_snapshot_level() {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !g_targetPlayer->pDynamicPath) return 0;
    return (uint16_t)g_currentLevel;
}
EMSCRIPTEN_KEEPALIVE const D2SessionSnapshot* d2_session_snapshot() {
    updateSessionSnapshot(); return &g_sessionSnapshot;
}
// Return the same weapon-class enum the retail composit selector uses for this
// player's current mode and inventory. The client consumes this result when
// selecting the COF; it must not infer the animation class from the right-hand
// Weapons.txt row (which loses dual-wield and two-handed rules).
EMSCRIPTEN_KEEPALIVE int d2_session_player_weapon_class_id() {
    if (!g_targetPlayer) return -1;
    int weaponClassId = -1;
    COMPOSIT_GetWeaponClassId(g_targetPlayer, g_targetPlayer->pInventory, &weaponClassId, -1, TRUE);
    return weaponClassId;
}
// Match the native drawable-unit selector call: the caller supplies the mode
// being drawn, and converted-mode substitution is disabled.
// Named boundary for the mode chosen by the local drawable player renderer. The generic
// explicit-mode export below remains available to existing callers; both use the same unit and inventory.
EMSCRIPTEN_KEEPALIVE int d2_session_player_weapon_class_id_for_presentation_mode(int32_t presentationMode) {
    if (!g_targetPlayer) return -1;
    int weaponClassId = -1;
    COMPOSIT_GetWeaponClassId(g_targetPlayer, g_targetPlayer->pInventory, &weaponClassId, presentationMode, FALSE);
    return weaponClassId;
}
EMSCRIPTEN_KEEPALIVE int d2_session_player_weapon_class_id_for_mode(int32_t mode) {
    return d2_session_player_weapon_class_id_for_presentation_mode(mode);
}
// Same effective type/class/mode tuple used by the native compositor path. The exported identity
// deliberately keeps the source D2Unit as a player; the browser uses this tuple to choose the
// transformed animation identity without fabricating monster-unit state or component variants.
EMSCRIPTEN_KEEPALIVE const D2SessionPlayerRenderIdentity* d2_session_player_render_identity() {
    memset(&g_sessionPlayerRenderIdentity, 0, sizeof g_sessionPlayerRenderIdentity);
    g_sessionPlayerRenderIdentity.version = 1;
    g_sessionPlayerRenderIdentity.size = sizeof g_sessionPlayerRenderIdentity;
    if (!g_targetPlayer) return &g_sessionPlayerRenderIdentity;
    int unitType = g_targetPlayer->dwUnitType;
    int classId = g_targetPlayer->dwClassId;
    int mode = UNITS_GetAnimOrSeqMode(g_targetPlayer);
    D2COMMON_11013_ConvertMode(g_targetPlayer, &unitType, &classId, &mode, __FILE__, __LINE__);
    int weaponClassId = -1;
    COMPOSIT_GetWeaponClassId(g_targetPlayer, g_targetPlayer->pInventory, &weaponClassId, -1, TRUE);
    g_sessionPlayerRenderIdentity.unitType = (uint32_t)unitType;
    g_sessionPlayerRenderIdentity.classId = classId;
    g_sessionPlayerRenderIdentity.mode = mode;
    g_sessionPlayerRenderIdentity.weaponClassId = weaponClassId;
    return &g_sessionPlayerRenderIdentity;
}
// The player's shapeshifted form as a monster-style animation record (Werebear, Werewolf and the other
// Disguise states). ConvertMode swaps the unit for {monster, class, mode}; the form is then drawn from
// that monster's MonStats2 row, component choice 0, like any monster. Returns null when the player is
// not in a monster form.
static D2SessionMonsterAnimation g_sessionPlayerForm;
EMSCRIPTEN_KEEPALIVE const D2SessionMonsterAnimation* d2_session_player_form_actor() {
    if (!g_targetPlayer) return nullptr;
    int unitType = g_targetPlayer->dwUnitType;
    int classId = g_targetPlayer->dwClassId;
    int mode = UNITS_GetAnimOrSeqMode(g_targetPlayer);
    D2COMMON_11013_ConvertMode(g_targetPlayer, &unitType, &classId, &mode, __FILE__, __LINE__);
    if (unitType != UNIT_MONSTER) return nullptr;
    D2SessionMonsterAnimation& a = g_sessionPlayerForm;
    a = {};
    const D2MonStatsTxt* mon = DATATBLS_GetMonStatsTxtRecord(classId);
    const D2MonStats2Txt* mon2 = UNITS_GetMonStats2TxtRecordFromMonsterId(classId);
    if (!mon) return nullptr;
    a.id = g_targetPlayer->dwUnitId; a.classId = (uint32_t)classId; a.code = mon->dwCode; a.mode = (uint32_t)mode;
    a.framePrecise = g_targetPlayer->nSeqCurrentFramePrecise; a.frameCountPrecise = g_targetPlayer->dwFrameCountPrecise;
    a.animSpeed = g_targetPlayer->wAnimSpeed;
    if (g_clientMorphActive) {
        // The client unit is in the form's S1 (player S1 converts to the monster's S1), on its own clock.
        int mt = UNIT_PLAYER, mc = (int)g_targetPlayer->dwClassId, mm = PLRMODE_SPECIAL1;
        D2COMMON_11013_ConvertMode(g_targetPlayer, &mt, &mc, &mm, __FILE__, __LINE__);
        a.mode = (uint32_t)mm;
        a.framePrecise = g_clientMorphFramePrecise; a.frameCountPrecise = g_clientMorphFrameCount; a.animSpeed = g_clientMorphAnimSpeed;
    } else if (g_clientNeutralClockActive) {
        a.framePrecise = g_clientNeutralFramePrecise; a.animSpeed = g_clientNeutralAnimSpeed;
    }
    a.direction = g_clientFacingActive ? g_clientFacing.nDirection
        : g_targetPlayer->pDynamicPath ? g_targetPlayer->pDynamicPath->nDirection : 0;
    a.baseWeaponClass = mon2 ? static_cast<uint32_t>(mon2->nBaseW) : 0;
    a.animationFlags = (mon2 && (mon2->dwModeFlags & MONSTATS2MODEFLAG_DD) ? 0x02 : 0);
    for (uint32_t c = 0; c < 16; ++c) a.componentCodes[c] = (uint32_t)D2Common_11070(classId, c, 0);
    return &a;
}
EMSCRIPTEN_KEEPALIVE const D2SessionResources* d2_session_resources() {
    memset(&g_sessionResources, 0, sizeof g_sessionResources);
    g_sessionResources.version = 1; g_sessionResources.size = sizeof g_sessionResources;
    if (g_targetPlayer) {
        g_sessionResources.hp = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_HITPOINTS, 0);
        g_sessionResources.maxHp = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_MAXHP, 0);
        g_sessionResources.mana = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_MANA, 0);
        g_sessionResources.maxMana = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_MAXMANA, 0);
    }
    return &g_sessionResources;
}

EMSCRIPTEN_KEEPALIVE const D2SessionRoom* d2_session_rooms() {
    updateSessionSnapshot(); return g_sessionRooms.data();
}
EMSCRIPTEN_KEEPALIVE int d2_session_room_count() {
    updateSessionSnapshot(); return (int)g_sessionRooms.size();
}
EMSCRIPTEN_KEEPALIVE const D2SessionUnit* d2_session_units() {
    updateSessionSnapshot(); return g_sessionUnits.data();
}
EMSCRIPTEN_KEEPALIVE int d2_session_unit_count() {
    updateSessionSnapshot(); return (int)g_sessionUnits.size();
}
// One D2SessionUnitCoord per d2_session_units record, in the same order. Separate from D2SessionUnit
// so that record's stride -- which every scenario parses at fixed offsets -- does not move.
EMSCRIPTEN_KEEPALIVE const D2SessionUnitCoord* d2_session_unit_coords() {
    updateSessionSnapshot(); return g_sessionUnitCoords.data();
}
EMSCRIPTEN_KEEPALIVE int d2_session_unit_coord_count() {
    updateSessionSnapshot(); return (int)g_sessionUnitCoords.size();
}
// One native dynamic-path facing per session unit. Keep this observation separate
// from D2SessionUnit so existing consumers retain their fixed 44-byte stride.
EMSCRIPTEN_KEEPALIVE const D2SessionUnitFacing* d2_session_unit_facings() {
    updateSessionSnapshot(); return g_sessionUnitFacings.data();
}
EMSCRIPTEN_KEEPALIVE int d2_session_unit_facing_count() {
    updateSessionSnapshot(); return (int)g_sessionUnitFacings.size();
}
// The last tick's 0x0C hit packets, 12 bytes each: type u32, guid u32, message, hit class, life byte, 0.
extern "C" const void* D2WEB_HitPackets(uint32_t* pCount);
EMSCRIPTEN_KEEPALIVE const void* d2_session_hit_packets() { return D2WEB_HitPackets(nullptr); }
EMSCRIPTEN_KEEPALIVE int d2_session_hit_packet_count() { uint32_t n = 0; D2WEB_HitPackets(&n); return (int)n; }
EMSCRIPTEN_KEEPALIVE const D2SessionUnitOverlay* d2_session_unit_overlays() {
    updateSessionSnapshot(); return g_sessionUnitOverlays.data();
}
EMSCRIPTEN_KEEPALIVE int d2_session_unit_overlay_count() {
    updateSessionSnapshot(); return (int)g_sessionUnitOverlays.size();
}
EMSCRIPTEN_KEEPALIVE const D2SessionUnitState* d2_session_unit_states() {
    updateSessionSnapshot(); return g_sessionUnitStates.data();
}
EMSCRIPTEN_KEEPALIVE int d2_session_unit_state_count() {
    updateSessionSnapshot(); return (int)g_sessionUnitStates.size();
}
EMSCRIPTEN_KEEPALIVE const D2SessionMonsterAnimation* d2_session_monster_animations() {
    updateSessionSnapshot(); return g_sessionMonsterAnimations.data();
}
EMSCRIPTEN_KEEPALIVE int d2_session_monster_animation_count() {
    updateSessionSnapshot(); return (int)g_sessionMonsterAnimations.size();
}
EMSCRIPTEN_KEEPALIVE const D2SessionMissileAnimation* d2_session_missile_animations() {
    updateSessionSnapshot(); return g_sessionMissileAnimations.data();
}
EMSCRIPTEN_KEEPALIVE int d2_session_missile_animation_count() {
    updateSessionSnapshot(); return (int)g_sessionMissileAnimations.size();
}
// Whether the client holds a missile of its own for engine missile `id`. Retail's client never mirrors the local
// player's server missiles: it creates its own when its own cast starts, and D2Client+0x18F90 refuses that cast for
// a use-state other than 0/5 (no client missile, no TravelSound). Every other owner's missile reaches the client.
extern "C" int d2_realm_client_player_count();
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_missile_on_client(uint32_t id) {
    D2UnitStrc* m = g_game ? SUNIT_GetServerUnit(g_game, UNIT_MISSILE, (int32_t)id) : nullptr;
    if (!m || !g_targetPlayer || m->dwOwnerType != UNIT_PLAYER || m->dwOwnerGUID != g_targetPlayer->dwUnitId) return 1;
    if ((m->dwClassId == 62 || m->dwClassId == 45 || m->dwClassId == 48) && MISSILE_GetSkill(m) == 0) return 0;   // the shrines' missiles (no skill): the client made its own at the use packet (D2Client+0x26060, 0x25F10, 0x25DC0)
    if (m->dwClassId == 62 && d2_realm_client_player_count() > 0) return 0;   // the realm client flies the player's own fireball and sounds it (d2_realm_client_take_missile_sounds)
    { const D2SkillsTxt* t = DATATBLS_GetSkillsTxtRecord(MISSILE_GetSkill(m)); if (t && t->wCltDoFunc == 27 && (int32_t)m->dwClassId == (int16_t)t->wCltMissileA) return 0; }   // the client's own cltdofunc 27 bolt (sounded at the do)
    return d2_client_local_cast_refused() || d2_client_cast_walked() ? 0 : 1;
}
// A client unit the client creates in engine missile `id`'s room (D2Client+0xA7400): rolls that client room's seed and
// returns the roll, the new unit's init seed (its seed is {init, 666}); 0 when there is no such missile or room.
extern "C++" { namespace d2client_unitseed { uint32_t roll(const D2ActiveRoomStrc* r); } }
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_client_unit_roll(uint32_t id) {
    D2UnitStrc* m = g_game ? SUNIT_GetServerUnit(g_game, UNIT_MISSILE, (int32_t)id) : nullptr;
    return m ? d2client_unitseed::roll(UNITS_GetRoom(m)) : 0;
}
// A charged-bolt-path missile (SKILLS_MissileInit_ChargedBolt, D2Game+0x43B60; its client mirror D2Client+0x73B60 runs on the
// same inputs) reseeds itself {first path point X + index, 666} and its path takes rolls of that seed: the client missile's
// seed is then engine missile `id`'s. Returns word `hi` (0 low, 1 high) of it, 0 when `id` is no such missile.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_missile_bolt_seed(uint32_t id, uint32_t hi) {
    D2UnitStrc* m = g_game ? SUNIT_GetServerUnit(g_game, UNIT_MISSILE, (int32_t)id) : nullptr;
    if (!m || !m->pDynamicPath || PATH_GetType(m->pDynamicPath) != PATHTYPE_CHARGEDBOLT) return 0;
    return hi ? m->pSeed.nHighSeed : m->pSeed.nLowSeed;
}
EMSCRIPTEN_KEEPALIVE const D2SessionObjectPresentation* d2_session_object_presentations() {
    updateSessionSnapshot(); return g_sessionObjectPresentations.data();
}
EMSCRIPTEN_KEEPALIVE const D2SessionItemPresentation* d2_session_item_presentations() {
    updateSessionSnapshot(); return g_sessionItemPresentations.data();
}
EMSCRIPTEN_KEEPALIVE const uint8_t* d2_session_item_colours() {
    updateSessionSnapshot(); return g_sessionItemColours.data();
}
EMSCRIPTEN_KEEPALIVE const uint16_t* d2_session_item_inv_colours() {
    updateSessionSnapshot(); return g_sessionItemInvColours.data();
}
EMSCRIPTEN_KEEPALIVE const D2SessionItemSockets* d2_session_item_sockets() {
    updateSessionSnapshot(); return g_sessionItemSockets.data();
}
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_item_socket_count() {
    updateSessionSnapshot(); return (uint32_t)g_sessionItemSockets.size();
}
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_item_presentation_count() {
    updateSessionSnapshot(); return (uint32_t)g_sessionItemPresentations.size();
}
EMSCRIPTEN_KEEPALIVE int d2_session_object_presentation_count() {
    updateSessionSnapshot(); return (int)g_sessionObjectPresentations.size();
}
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_world_hash() {
    updateSessionSnapshot(); return g_sessionWorldHash;
}
// How many tiles the room arrays held that never reached the draw list because their library
// entry was unresolved. Retail draws those tiles, so a non-zero count here is a defect, not a
// statistic -- s140 measures it against retail's own record count.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_tiles_skipped() { return g_sessionTilesSkipped; }

// Probe for the missing ninth row/column. Retail's room tile lists carry
// floor records at tile offset 8 -- the row and column past the room's own 8x8 -- and ours do not.
// The fill only emits a floor where the grid cell has bIsFloor, so this reports, per room, how many
// of those edge cells carry bIsFloor and how many carry bLOS. Read-only: it walks the same grids
// the generator already built and changes nothing.
//   out[0] grid width, out[1] grid height, out[2] edge cells with bIsFloor, out[3] edge with bLOS,
//   out[4] interior cells with bIsFloor.
// A PRESET room's edge decision, so the remaining missing tiles can be correlated with it rather
// than guessed at (T4: 5,285 ninth-row cells still missing, all in levels
// whose rooms have no outdoor grid).
//
// DRLGPRESET_LoadRoomTiles kills an edge only when the preset's LvlPrest KillEdge column is set AND
// the room's far edge coincides with the maze's. A killed edge is never visited by the fill, so it
// cannot produce tiles; an unkilled one can, and if retail has tiles there and we do not, the fault
// is downstream.
//   out[0] killEdgeColumn, out[1] bKillEdgeX, out[2] bKillEdgeY,
//   out[3] emitted floors, out[4] emitted walls, out[5] tileWidth, out[6] tileHeight,
//   out[7] fillBlanks column, out[8] floor layers
extern int32_t g_d2webPresetEdgeMerged;
extern int32_t g_d2webPresetEdgeAdded;
extern int32_t g_d2webPresetEdgeSameMap;
extern int32_t g_d2webPresetEdgeOtherMap;
// How a preset room's own ninth row/column was resolved during the last level build: merged into a
// neighbour, or added here. Diagnostic for the last remaining candidate.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_preset_edge_counts(int32_t* out, uint32_t cap) {
    if (!out || cap < 4) return 0;
    out[0] = g_d2webPresetEdgeMerged; out[1] = g_d2webPresetEdgeAdded;
    out[2] = g_d2webPresetEdgeSameMap; out[3] = g_d2webPresetEdgeOtherMap;
    g_d2webPresetEdgeMerged = 0; g_d2webPresetEdgeAdded = 0;
    g_d2webPresetEdgeSameMap = 0; g_d2webPresetEdgeOtherMap = 0;
    return 1;
}

EMSCRIPTEN_KEEPALIVE uint32_t d2_session_preset_edge(uint32_t roomOrdinal, int32_t* out, uint32_t cap) {
    if (!g_sessionLive || !g_lastLevel || !out || cap < 9) return 0;
    uint32_t ordinal = 0;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext, ++ordinal) {
        if (ordinal != roomOrdinal) continue;
        if (roomEx->nType != DRLGTYPE_PRESET || !roomEx->pMaze || !roomEx->pMaze->pMap) return 0;
        D2DrlgMapStrc* map = roomEx->pMaze->pMap;
        if (!map->pLvlPrestTxtRecord) return 0;
        const int kill = (int)map->pLvlPrestTxtRecord->dwKillEdge;
        out[0] = kill;
        out[1] = kill && (roomEx->nTileXPos + roomEx->nTileWidth == map->pDrlgCoord.nPosX + map->pDrlgCoord.nWidth);
        out[2] = kill && (roomEx->nTileYPos + roomEx->nTileHeight == map->pDrlgCoord.nPosY + map->pDrlgCoord.nHeight);
        out[3] = roomEx->pRoom && roomEx->pRoom->pRoomTiles ? roomEx->pRoom->pRoomTiles->nFloors : -1;
        out[4] = roomEx->pRoom && roomEx->pRoom->pRoomTiles ? roomEx->pRoom->pRoomTiles->nWalls : -1;
        out[5] = roomEx->nTileWidth; out[6] = roomEx->nTileHeight;
        // dwFillBlanks drives bCheckCoordinatesValidity on floor layer 0, which makes every cell
        // INSIDE the room's coordinates count as floor whether or not it carries bIsFloor. An edge
        // cell is by definition outside those coordinates, so a FillBlanks level and a plain one
        // may well treat the ninth row differently -- the caves are the short ones and the town is
        // not, so this is the column to check.
        out[7] = (int32_t)map->pLvlPrestTxtRecord->dwFillBlanks;
        out[8] = map->pFile ? map->pFile->nFloorLayers : -1;
        return 1;
    }
    return 0;
}

// For a preset room: how many of its NEAR rooms share its preset map, and how many do not.
//
// s163 showed every short preset room is downstream of the fill, not a killed edge -- the same
// merge that caused the outdoor case. But the outdoor rule applied wholesale to presets produced
// 498 extra floors in a town that is only 3 rooms short, so the preset rule is narrower. The
// candidate: within ONE preset map an edge cell is shared and recorded once, across maps both
// rooms record it. This reports the adjacency so that can be checked against the short rooms.
//   out[0] near rooms, out[1] near rooms sharing this room's pMap, out[2] near rooms on another map,
//   out[3] near rooms that are not preset rooms at all
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_preset_neighbours(uint32_t roomOrdinal, int32_t* out, uint32_t cap) {
    if (!g_sessionLive || !g_lastLevel || !out || cap < 4) return 0;
    uint32_t ordinal = 0;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext, ++ordinal) {
        if (ordinal != roomOrdinal) continue;
        if (roomEx->nType != DRLGTYPE_PRESET || !roomEx->pMaze) return 0;
        const D2DrlgMapStrc* mine = roomEx->pMaze->pMap;
        out[0] = roomEx->nRoomsNear; out[1] = out[2] = out[3] = 0;
        for (int i = 0; i < roomEx->nRoomsNear; ++i) {
            D2DrlgRoomStrc* near = roomEx->ppRoomsNear[i];
            if (!near || near == roomEx) continue;
            if (near->nType != DRLGTYPE_PRESET || !near->pMaze) { ++out[3]; continue; }
            if (near->pMaze->pMap == mine) ++out[1]; else ++out[2];
        }
        return 1;
    }
    return 0;
}

// An Inventory.txt row's panel rectangle and item grid, from retail's own accessors
// (DATATBLS_GetInventoryRect #10635, DATATBLS_GetInventoryGridInfo #10636), at the requested
// resolution. The rows cover the character panels AND the trade/stash/cube screens, so this is the
// source for any panel that shows an item grid -- the vendor window included, which currently
// draws a text list at coordinates that were tuned by trial.
//
// out: panelLeft, panelRight, panelTop, panelBottom, gridX, gridY, gridLeft, gridRight, gridTop,
//      gridBottom, boxWidth, boxHeight  (12 int32)
// The shop's stock, in the same 112-byte presentation format as the player's items, so the client
// parses both with one reader. Needed for the vendor ICON GRID.
EMSCRIPTEN_KEEPALIVE const D2SessionItemPresentation* d2_session_vendor_presentations() {
    return g_sessionVendorPresentations.data();
}
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_vendor_presentation_count() {
    return (uint32_t)g_sessionVendorPresentations.size();
}

// P8i: retail's tooltip PROPERTY lines for an item (the lines under its name), newline-joined, from the
// item's own stats and Weapons/Armor/Misc rows and retail's string table. Base properties only: damage,
// defense, quantity, durability, requirements and the weapon class line. Affix/magic property text
// (ItemStatCost/Properties descriptions) is not here yet -- P8i. Returns bytes written.
static const char16_t* langIdx(int i) { const std::u16string* v = d2::sharedLang().byTblIndex((uint16_t)i); return v ? v->c_str() : u""; }
static void appendNarrow(std::string& out, const char16_t* w) { for (; *w; ++w) out.push_back(*w < 0x80 ? (char)*w : '?'); }
// Fixture (P8j): make an item magic with the named affixes (their DISPLAY strings, "" = none) so a probe can build the item a retail
// capture shows -- "Jagged" + Spear, "Septic" + Hand Axe + "of Quality". Property VALUES roll from the item's seed: a caller wanting retail's
// numbers retries with fresh items until they match.
// Retail rolls affixes from the item level; here the caller names them. Returns 1 when every named affix was found.
EMSCRIPTEN_KEEPALIVE int d2_session_fixture_affixes(uint32_t itemId, const char* prefix, const char* suffix) {
    if (!g_sessionLive || !g_game) return 0;
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId);
    if (!item || !item->pItemData) return 0;
    auto find = [&](const char* name, bool isPrefix) -> uint16_t {
        if (!name || !*name) return 0;
        const D2MagicAffixDataTbl& tb = sgptDataTables->pMagicAffixDataTables;
        for (int i = 1; i <= tb.nMagicAffixTxtRecordCount; ++i) {
            const D2MagicAffixTxt* r = DATATBLS_GetMagicAffixTxtRecord(i);
            if (!r || (r >= tb.pMagicPrefix && r < tb.pAutoMagic) != isPrefix) continue;
            std::string x; appendNarrow(x, langIdx(r->wTblIndex)); if (x == name) return (uint16_t)i;
        }
        return 0; };
    const uint16_t p = find(prefix, true), sfx = find(suffix, false);
    if ((prefix && *prefix && !p) || (suffix && *suffix && !sfx)) return 0;
    item->pItemData->dwQualityNo = ITEMQUAL_MAGIC;
    item->pItemData->wMagicPrefix[0] = p; item->pItemData->wMagicSuffix[0] = sfx;
    // The properties the way magic generation assigns them (ItemsMagic.cpp): each affix's own rolls onto the item.
    for (uint16_t id : { p, sfx }) if (D2MagicAffixTxt* r = id ? DATATBLS_GetMagicAffixTxtRecord(id) : nullptr) ITEMMODS_AssignProperty(PROPMODE_AFFIX, nullptr, item, r, 0, 0);
    updateSessionSnapshot();
    return 1;
}
// P8j: a magic item's hover name as retail draws it: "<prefix> <base> <suffix>" (measured "Jagged Spear " and
// "Septic Hand Axe of Quality": an absent affix leaves its separator); superior "Superior <base>". Empty for other qualities (not ported).
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_item_title(uint32_t itemId, char* buf, uint32_t cap) {
    if (!g_sessionLive || !g_game || !buf || !cap) return 0;
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId);
    D2ItemsTxt* t = item ? DATATBLS_GetItemsTxtRecord(item->dwClassId) : nullptr;
    auto baseName = [&] { std::string code(t->szCode, 4); while (!code.empty() && (code.back() == ' ' || code.back() == 0)) code.pop_back();
        std::string base; const std::u16string* v = d2::sharedLang().find(code.c_str()); if (v) appendNarrow(base, v->c_str()); return base; };
    auto emit = [&](const std::string& out) { const uint32_t n = out.size() < cap - 1 ? (uint32_t)out.size() : cap - 1; memcpy(buf, out.data(), n); buf[n] = 0; return n; };
    if (t && item->pItemData && d2::item_text::identified(item->pItemData->dwItemFlags) &&
        (item->pItemData->dwQualityNo == ITEMQUAL_UNIQUE || item->pItemData->dwQualityNo == ITEMQUAL_SET || item->pItemData->dwQualityNo == ITEMQUAL_RARE)) {
        // Identified unique/set/rare: the record's own tbl string (colour is the quality's), then the base name on a second line.
        std::string nm;
        if (item->pItemData->dwQualityNo == ITEMQUAL_UNIQUE) { const D2UniqueItemsTxt* u = ITEMS_GetUniqueItemsTxtRecord(item->pItemData->dwFileIndex); if (u) appendNarrow(nm, langIdx(u->wTblIndex)); }
        else if (item->pItemData->dwQualityNo == ITEMQUAL_SET) { const D2SetItemsTxt* s = ITEMS_GetSetItemsTxtRecord(item->pItemData->dwFileIndex); if (s) appendNarrow(nm, langIdx(s->wStringId)); }
        else {
            // Rare: "<rare prefix> <rare suffix>" from rareprefix/raresuffix.txt (wStringId is the tbl index).
            for (uint16_t id : { item->pItemData->wRarePrefix, item->pItemData->wRareSuffix }) {
                const D2RareAffixTxt* r = id ? DATATBLS_GetRareAffixTxtRecord(id) : nullptr;
                if (r) { if (!nm.empty()) nm += ' '; appendNarrow(nm, langIdx(r->wStringId)); }
            }
        }
        if (nm.empty()) return 0;
        return emit(nm + "\n" + baseName());
    }
    if (t && item->pItemData && item->pItemData->dwQualityNo == ITEMQUAL_SUPERIOR) {
        // Superior: the tbl string "Hiquality" before the base name (measured "Superior Hand Axe").
        std::string sup; const std::u16string* v = d2::sharedLang().find("Hiquality"); if (v) appendNarrow(sup, v->c_str());
        if (sup.empty()) return 0;
        return emit(sup + " " + baseName());
    }
    if (!t || !item->pItemData || item->pItemData->dwQualityNo != ITEMQUAL_MAGIC ||
        !d2::item_text::identified(item->pItemData->dwItemFlags)) return 0;
    auto aff = [&](uint16_t id) { std::string x; const D2MagicAffixTxt* r = id ? DATATBLS_GetMagicAffixTxtRecord(id) : nullptr; if (r) appendNarrow(x, langIdx(r->wTblIndex)); return x; };
    std::string code(t->szCode, 4); while (!code.empty() && (code.back() == ' ' || code.back() == 0)) code.pop_back();
    std::string base; { const std::u16string* v = d2::sharedLang().find(code.c_str()); if (v) appendNarrow(base, v->c_str()); }
    const std::string pre = aff(item->pItemData->wMagicPrefix[0]), suf = aff(item->pItemData->wMagicSuffix[0]);
    const std::string out = (pre.empty() ? "" : pre + " ") + base + (suf.empty() && !pre.empty() ? " " : suf.empty() ? "" : " " + suf);
    const uint32_t n = out.size() < cap - 1 ? (uint32_t)out.size() : cap - 1;
    memcpy(buf, out.data(), n); buf[n] = 0;
    return n;
}
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_item_lines(uint32_t itemId, char* buf, uint32_t cap) {
    if (!g_sessionLive || !g_game || !buf || !cap) return 0;
    D2UnitStrc* item = SUNIT_GetServerUnit(g_game, UNIT_ITEM, (int32_t)itemId);
    D2ItemsTxt* t = item ? DATATBLS_GetItemsTxtRecord(item->dwClassId) : nullptr;
    if (!t) return 0;
    std::string out;
    auto line = [&](int label, const std::string& tail) { if (!out.empty()) out.push_back('\n'); appendNarrow(out, langIdx(label)); out += tail; };
    auto range = [&](int lo, int hi) { return " " + std::to_string(lo) + " " + [&] { std::string x; appendNarrow(x, langIdx(3464)); return x; }() + " " + std::to_string(hi); };
    const bool itemIdentified = d2::item_text::identified(item->pItemData ? item->pItemData->dwItemFlags : 0);
    const bool weapon = t->dwWeapClass != 0 || t->nMaxDam || t->n2HandMaxDam || t->nMaxMisDam;
    if (weapon) {
        // Identified items use the affix-adjusted values (ITEMMODS_PropertyFunc05-07). The
        // complete serializer withholds the stat-list payload for unidentified client items,
        // so their tooltip must use the unit's base stats instead of server-only current stats.
        auto dmg = [&](int label, int lo, int hi, int sMin, int sMax) {
            const int32_t baseA = STATLIST_GetUnitBaseStat(item, sMin, 0), baseB = STATLIST_GetUnitBaseStat(item, sMax, 0);
            int32_t a = d2::item_text::visibleStatValue(itemIdentified, STATLIST_UnitGetStatValue(item, sMin, 0), baseA);
            int32_t b = d2::item_text::visibleStatValue(itemIdentified, STATLIST_UnitGetStatValue(item, sMax, 0), baseB);
            if (!a && !b) { a = lo; b = hi; }
            if (itemIdentified && (a != lo || b != hi)) { if (!out.empty()) out.push_back('\n'); out += "\x01" "v"; appendNarrow(out, langIdx(label)); out += range(a, b); }
            else line(label, range(a, b)); };
        if (t->nMinMisDam || t->nMaxMisDam) dmg(3467, t->nMinMisDam, t->nMaxMisDam, STAT_ITEM_THROW_MINDAMAGE, STAT_ITEM_THROW_MAXDAMAGE);
        if (t->nMaxDam) dmg(3465, t->nMinDam, t->nMaxDam, STAT_MINDAMAGE, STAT_MAXDAMAGE);
        if (t->n2HandMaxDam) dmg(3466, t->n2HandMinDam, t->n2HandMaxDam, STAT_SECONDARY_MINDAMAGE, STAT_SECONDARY_MAXDAMAGE);
    } else if (int32_t ac = d2::item_text::visibleStatValue(itemIdentified,
                   STATLIST_UnitGetStatValue(item, STAT_ARMORCLASS, 0), STATLIST_GetUnitBaseStat(item, STAT_ARMORCLASS, 0));
               ac > 0 && t->dwMaxAc > 0) line(3461, " " + std::to_string(ac));
    if (t->nStackable) line(3462, " " + std::to_string(STATLIST_UnitGetStatValue(item, STAT_QUANTITY, 0)));
    // Retail's gate is ITEMS_HasDurability (D2Common #10865): items.txt durability set and nodurability clear, a non-zero max-durability
    // stat, and not indestructible. Weapons.txt sets nodurability on every bow and crossbow, so those carry stats 72/73 but no line.
    if (ITEMS_HasDurability(item)) {
        std::string of; appendNarrow(of, langIdx(3463));
        const int32_t maxDurability = d2::item_text::visibleStatValue(itemIdentified,
            STATLIST_UnitGetStatValue(item, STAT_MAXDURABILITY, 0), STATLIST_GetUnitBaseStat(item, STAT_MAXDURABILITY, 0));
        line(3457, " " + std::to_string(STATLIST_UnitGetStatValue(item, STAT_DURABILITY, 0)) + " " + of + " " + std::to_string(maxDurability));
    }
    // Requirements: retail's displayed values (item_req_percent scales Str/Dex, ethereal -10) and its red (colour 1)
    // when the character does not meet one -- ITEMS_CheckRequirements' own per-stat flags (P8j). Lines carry a
    // "\x01<digit>" colour prefix; no prefix = white.
    BOOL okS = TRUE, okD = TRUE, okL = TRUE;
    const int32_t reqPct = itemIdentified ? STATLIST_UnitGetItemStatOrSkillStatValue(item, STAT_ITEM_REQ_PERCENT, 0) : 0;
    const int32_t eth = (item->pItemData && (item->pItemData->dwItemFlags & IFLAG_ETHEREAL)) ? -10 : 0;
    const int32_t reqStrBase = std::max(0, t->wReqStr + eth);
    const int32_t reqDexBase = std::max(0, t->wReqDex + eth);
    const int32_t reqStrCurrent = std::max(0, t->wReqStr + (int32_t)(t->wReqStr * (int64_t)reqPct / 100) + eth);
    const int32_t reqDexCurrent = std::max(0, t->wReqDex + (int32_t)(t->wReqDex * (int64_t)reqPct / 100) + eth);
    const int32_t reqStr = d2::item_text::visibleRequirement(itemIdentified, reqStrCurrent, reqStrBase);
    const int32_t reqDex = d2::item_text::visibleRequirement(itemIdentified, reqDexCurrent, reqDexBase);
    const bool showLevelRequirement = t->nLevel && t->dwSpeed >= -1000;
    const int32_t lvlReq = showLevelRequirement ? (itemIdentified ? ITEMS_GetLevelRequirement(item, g_targetPlayer) : t->nLevelReq) : 0;
    if (g_targetPlayer) {
        if (itemIdentified) {
            ITEMS_CheckRequirements(item, g_targetPlayer, FALSE, &okS, &okD, &okL);
        } else {
            // The retail helper checks full server-side item requirements, including hidden
            // affix stats. Unidentified client coloring must follow only the visible base rows.
            okS = d2::item_text::meetsStatRequirement(STATLIST_UnitGetStatValue(g_targetPlayer, STAT_STRENGTH, 0), reqStr);
            okD = d2::item_text::meetsStatRequirement(STATLIST_UnitGetStatValue(g_targetPlayer, STAT_DEXTERITY, 0), reqDex);
            const int32_t playerLevel = STATLIST_UnitGetStatValue(g_targetPlayer, STAT_LEVEL, 0);
            okL = d2::item_text::meetsLevelRequirement(playerLevel, lvlReq);
        }
    }
    auto reqLine = [&](int label, int32_t v, BOOL ok) { if (!out.empty()) out.push_back('\n'); if (!ok) out += "\x01" "1"; appendNarrow(out, langIdx(label)); out += " " + std::to_string(v); };
    if (t->wReqStr) reqLine(3458, reqStr, okS);
    if (t->wReqDex) reqLine(3459, reqDex, okD);
    if (showLevelRequirement) {
        // ITEMS_GetLevelRequirement reads magic affix records and item stat-list bonuses.
        // Those fields are withheld until identification; show the base Items.txt requirement.
        if (lvlReq > 1) reqLine(3469, lvlReq, okL);
    }
    if (weapon) {
        // Class line: retail groups by item type (string 4077+: mace, axe, sword, dagger, -, javelin, spear, bow, staff,
        // polearm, crossbow). Speed bands over Weapons.txt speed (WSM): <=-60 fastest, <=-30 very fast, <=0 fast
        // (measured: Hand Axe, speed 0), <=10 normal, <=20 slow, <=30 very slow, else slowest -- only the Fast
        // band is retail-measured, the rest is a documented assumption.
        static const int kClass[][3] = { {ITEMTYPE_MACE, ITEMTYPE_CLUB, ITEMTYPE_HAMMER}, {ITEMTYPE_AXE, ITEMTYPE_THROWING_AXE, -1},
            {ITEMTYPE_SWORD, -1, -1}, {ITEMTYPE_KNIFE, ITEMTYPE_THROWING_KNIFE, -1}, {-1, -1, -1}, {ITEMTYPE_JAVELIN, -1, -1},
            {ITEMTYPE_SPEAR, -1, -1}, {ITEMTYPE_BOW, -1, -1}, {ITEMTYPE_STAFF, -1, -1}, {ITEMTYPE_POLEARM, -1, -1}, {ITEMTYPE_CROSSBOW, -1, -1} };
        int idx = -1;
        for (int i = 0; i < 11 && idx < 0; ++i) for (int k = 0; k < 3; ++k) if (kClass[i][k] >= 0 && ITEMS_CheckItemTypeId(item, kClass[i][k])) { idx = i; break; }
        // The band is the HERO's attack speed with this weapon (class animation frames over the item's base speed and IAS).
        // Unidentified items use the same calculation without STAT_ITEM_FASTERATTACKRATE, which the client serializer withholds.
        // This is NOT Weapons.txt's speed column alone: a Hand Axe (speed 0) reads Fast and a Spear (speed -10, 2ht) Normal on retail's hero
        // (shop-magic-hover-r21). Measured on the Barbarian: Hand Axe (1hs) value 16 = Fast, Spear (2ht) 17 = Normal, so Fast/Normal split at 17; the other five thresholds
        // (12/15/20/23/26) are SkillBar.cpp's and NOT retail-measured.
        const int32_t sp = g_targetPlayer && ITEMS_CheckItemTypeId(item, ITEMTYPE_WEAPON)
            ? (itemIdentified ? ITEMS_GetWeaponAttackSpeed(g_targetPlayer, item) : ITEMS_GetWeaponAttackSpeedForUnidentified(g_targetPlayer, item))
            : 18;
        const int band = sp < 12 ? 0 : sp < 15 ? 1 : sp < 17 ? 2 : sp < 20 ? 3 : sp < 23 ? 4 : sp < 26 ? 5 : 6;
        if (idx >= 0) { if (!out.empty()) out.push_back('\n'); appendNarrow(out, langIdx(4077 + idx)); out += " - "; appendNarrow(out, langIdx(4088 + band)); }
    }
    // P8j: the magical property lines (blue, colour 3), from the item's stat list through ItemStatCost's
    // descpriority/descfunc/descval/descstrpos/descstrneg/descstr2 and dgrp columns -- riiablo's StatFormatter /
    // StatListLabeler are the prior art. Highest priority first.
    if (d2::item_text::identified(item->pItemData ? item->pItemData->dwItemFlags : 0)) {
        D2StatStrc stats[128];
        const int ns = STATLIST_GetFullStatsDataFromUnit(item, stats, 128);
        std::vector<D2StatStrc> sv(stats, stats + std::max(0, std::min(ns, 128)));
        std::stable_sort(sv.begin(), sv.end(), [](const D2StatStrc& a, const D2StatStrc& b) { return a.nStat < b.nStat; });
        int edPct = 0;
        if (weapon) {   // base damage lives in these stats (shown on the Damage line); drop them here and re-derive the affix lines below
            sv.erase(std::remove_if(sv.begin(), sv.end(), [](const D2StatStrc& x) { return x.nStat >= STAT_MINDAMAGE && x.nStat <= STAT_SECONDARY_MAXDAMAGE || x.nStat == STAT_ITEM_THROW_MINDAMAGE || x.nStat == STAT_ITEM_THROW_MAXDAMAGE; }), sv.end());
            // A magic item's damage affixes (Properties.txt func 5/6/7 = dmg-min / dmg-max / dmg%) were folded into the damage stats; recover the
            // line each one shows from the folded numbers. Unambiguous cases only: Enhanced Damage alone, or a flat min/max alone.
            if (item->pItemData && item->pItemData->dwQualityNo == ITEMQUAL_MAGIC) {
                bool ed = false, fmin = false, fmax = false; edPct = 0;
                for (uint16_t aid : { item->pItemData->wMagicPrefix[0], item->pItemData->wMagicSuffix[0] }) {
                    const D2MagicAffixTxt* ar = aid ? DATATBLS_GetMagicAffixTxtRecord(aid) : nullptr;
                    for (int k = 0; ar && k < 3; ++k) {
                        const int pi = ar->pProperties[k].nProperty;
                        if (pi < 0 || pi >= sgptDataTables->nPropertiesTxtRecordCount) continue;
                        const int f = sgptDataTables->pPropertiesTxt[pi].nFunc[0];
                        ed |= f == 7; fmin |= f == 5; fmax |= f == 6;
                    } }
                const bool two = t->n2HandMaxDam != 0;
                const int sMin = two ? STAT_SECONDARY_MINDAMAGE : STAT_MINDAMAGE, sMax = two ? STAT_SECONDARY_MAXDAMAGE : STAT_MAXDAMAGE;
                const int lo = two ? t->n2HandMinDam : t->nMinDam, hi = two ? t->n2HandMaxDam : t->nMaxDam;
                const int32_t cMin = STATLIST_UnitGetStatValue(item, sMin, 0), cMax = STATLIST_UnitGetStatValue(item, sMax, 0);
                auto add = [&](int stat, int32_t v) { D2StatStrc x = {}; x.nStat = (int16_t)stat; x.nValue = v; sv.push_back(x); };
                if (ed && !fmin && !fmax) { for (int pct = 1; pct <= 500; ++pct) if (hi * (100 + pct) / 100 == cMax && lo * (100 + pct) / 100 == cMin) { edPct = pct; break; } }
                else if (!ed) { if (fmin && cMin > lo) add(STAT_MINDAMAGE, cMin - lo); if (fmax && cMax > hi) add(STAT_MAXDAMAGE, cMax - hi); }
            }
        }
        auto rec = [](int id) -> const D2ItemStatCostTxt* { return id >= 0 && id < sgptDataTables->nItemStatCostTxtRecordCount ? &sgptDataTables->pItemStatCostTxt[id] : nullptr; };
        auto str = [](int idx) { std::string x; appendNarrow(x, langIdx(idx)); return x; };
        auto key = [](const char* k) { const std::u16string* v = d2::sharedLang().find(k); std::string x; if (v) appendNarrow(x, v->c_str()); return x; };
        auto subst = [](std::string f, std::initializer_list<int32_t> a) {   // printf-lite: %d in order, %% -> %
            std::string r; auto it = a.begin();
            for (size_t i = 0; i < f.size(); ++i) {
                if (f[i] == '%' && i + 1 < f.size() && f[i + 1] == '%') { r += '%'; ++i; }
                else if (f[i] == '%' && i + 1 < f.size() && f[i + 1] == 'd') { if (it != a.end()) r += std::to_string(*it++); ++i; }
                else r += f[i];
            }
            return r; };
        struct Ln { int prio; std::string text; };
        std::vector<Ln> lns;
        // The min/max damage pairs retail folds into one line: {first stat, stats used, key if min==max, key for a range}.
        struct Pair { int stat, n; const char *eq, *range; };
        static const Pair pairs[] = { {21, 2, "strModMinDamage", "strModMinDamageRange"}, {23, 2, "strModMinDamage", "strModMinDamageRange"}, {48, 2, "strModFireDamage", "strModFireDamageRange"},
            {50, 2, "strModLightningDamage", "strModLightningDamageRange"}, {52, 2, "strModMagicDamage", "strModMagicDamageRange"},
            {54, 3, "strModColdDamage", "strModColdDamageRange"}, {57, 3, "strModPoisonDamage", "strModPoisonDamageRange"} };
        auto find = [&](int id) -> const D2StatStrc* { for (auto& x : sv) if (x.nStat == id) return &x; return nullptr; };
        std::set<int> consumed;
        for (const Pair& pr : pairs) {
            const D2StatStrc* a = find(pr.stat); const D2StatStrc* b = find(pr.stat + 1);
            if (!a || !b || !rec(pr.stat) || rec(pr.stat)->nDescPriority <= 0) continue;
            int32_t v1 = a->nValue, v2 = b->nValue;
            if (pr.n == 3) {   // cold/poison carry a length (frames): retail shows seconds; poison damage is spread over it
                const D2StatStrc* c = find(pr.stat + 2); if (!c) continue;
                const int32_t len = c->nValue; if (pr.stat == 57) { v1 = v1 * len / 256; v2 = v2 * len / 256; }
                lns.push_back({ rec(pr.stat)->nDescPriority, v1 == v2 ? subst(key(pr.eq), { v2, len / 25 }) : subst(key(pr.range), { v1, v2, len / 25 }) });
                consumed.insert(pr.stat + 2);
            } else lns.push_back({ rec(pr.stat)->nDescPriority, v1 == v2 ? subst(key(pr.eq), { v2 }) : subst(key(pr.range), { v1, v2 }) });
            consumed.insert(pr.stat); consumed.insert(pr.stat + 1);
        }
        // One stat through one (func, val, strpos, strneg, str2) tuple; "" when the function is not ported.
        auto fmt = [&](const D2ItemStatCostTxt* r, int func, int val, int sp, int sn, int s2, const D2StatStrc& st) -> std::string {
            // 1.13c D2Client+0x546E0: op 2..5 per-level scaling, then the stat's ValShift (life/mana/stamina hold value << 8).
            const D2ItemStatCostTxt* ob = (r->nOp > 1 && r->nOp < 6) ? rec(r->wOpBase) : nullptr;
            int32_t v = d2::item_text::descValue(st.nValue, ob ? r->nOp : 0, r->nOpParam,
                ob ? STATLIST_UnitGetStatValue(g_targetPlayer, r->wOpBase, 0) : 0, ob ? ob->nValShift : 0, r->nValShift);
            const std::string pos = str(sp), neg = str(sn), two = str(s2);
            auto pre = [&](const char* sign, const char* unit) { return val == 1 ? std::string(sign) + std::to_string(v) + unit + " " : std::string(); };
            auto post = [&](const char* sign, const char* unit) { return val == 2 ? " " + std::string(sign) + std::to_string(v) + unit : std::string(); };
            auto body = [&](int32_t x) { return x < 0 && sn ? neg : pos; };
            switch (func) {
            case 1: return pre("+", "") + body(v) + post("+", "");
            case 2: return pre("", "%") + body(v) + post("", "%");
            case 3: return pre("", "") + body(v) + post("", "");
            case 4: return pre("+", "%") + body(v) + post("+", "%");
            case 5: v = v * 100 / 128; return pre("", "%") + body(v) + post("", "%");
            case 6: case 7: case 8: case 9: {   // per-level: v already carries the op scaling (descValue)
                const char* sign = func == 6 || func == 8 ? "+" : ""; const char* unit = func == 7 || func == 8 ? "%" : "";
                return pre(sign, unit) + body(v) + " " + two + post(sign, unit); }
            case 10: v = v * 100 / 128; return pre("", "%") + body(v) + " " + two + post("", "%");
            case 12: return pre("+", "") + body(v) + post("+", "");
            case 13: { const D2CharStatsTxt* cs = UNITS_GetCharStatsTxtRecord(st.nLayer); return cs ? "+" + std::to_string(v) + " " + str(cs->wStrAllSkills) : ""; }
            case 14: { const D2CharStatsTxt* cs = UNITS_GetCharStatsTxtRecord((st.nLayer >> 3) & 3); return cs ? subst(str(cs->wStrSkillTab[st.nLayer & 7]), { v }) + " " + str(cs->wStrClassOnly) : ""; }
            case 19: return subst(body(v), { v });
            case 24: {   // 1.13c D2Client+0x54E10 case 0x18: "Level <lvl> <skill> (<cur>/<max> Charges)"; layer = skill << 6 | level, value = cur | max << 8
                const int sk = st.nLayer >> 6;
                const D2SkillsTxt* sr = sk > 0 && sk < sgptDataTables->nSkillsTxtRecordCount ? DATATBLS_GetSkillsTxtRecord(sk) : nullptr;
                const D2SkillDescTxt* sd = sr ? DATATBLS_GetSkillDescTxtRecord(sr->wSkillDesc) : nullptr;
                if (!sd) return "";
                return str(21249) + str(3995) + std::to_string(st.nLayer & 0x3f) + str(3995) + str(sd->wStrName) + str(3995) + subst(pos, { v & 0xff, v >> 8 }); }
            case 20: v = -v; return pre("", "%") + body(v) + post("", "%");
            case 21: v = -v; return pre("", "") + body(v) + post("", "");
            default: return "";   // 11, 15-18, 22-23, 25-28: replenish, skill procs/auras, by-time, skill names: not ported
            } };
        std::map<int, std::vector<const D2StatStrc*>> groups;
        for (const D2StatStrc& st : sv) {
            const D2ItemStatCostTxt* r = rec(st.nStat);
            if (!r || st.nStat == STAT_ARMORCLASS || consumed.count(st.nStat) || r->nDescPriority <= 0 || !r->nDescFunc) continue;
            if (r->wDescGrp) { groups[r->wDescGrp].push_back(&st); continue; }
            const std::string x = fmt(r, r->nDescFunc, r->nDescVal, r->wDescStrPos, r->wDescStrNeg, r->wDescStr2, st);
            if (!x.empty()) lns.push_back({ r->nDescPriority, x });
        }
        for (auto& [g, m] : groups) {   // a group folds into one line only when it is complete and every member is equal (e.g. all resists)
            int members = 0; for (int i = 0; i < sgptDataTables->nItemStatCostTxtRecordCount; ++i) members += sgptDataTables->pItemStatCostTxt[i].wDescGrp == g;
            bool same = (int)m.size() == members; for (auto* q : m) same = same && q->nValue == m[0]->nValue && q->nLayer == m[0]->nLayer;
            for (size_t i = 0; i < (same ? 1u : m.size()); ++i) {
                const D2ItemStatCostTxt* r = rec(m[i]->nStat);
                const std::string x = same ? fmt(r, r->nDescGrpFunc, r->nDescGrpVal, r->wDescGrpStrPos, r->wDescGrpStrNeg, r->wDescGrpStr2, *m[i])
                                           : fmt(r, r->nDescFunc, r->nDescVal, r->wDescStrPos, r->wDescStrNeg, r->wDescStr2, *m[i]);
                if (!x.empty()) lns.push_back({ r->nDescPriority, x });
            }
        }
        // The folded Enhanced Damage: retail draws min% and max% (equal) as ONE line, string strModEnhancedDamage ("+20% Enhanced Damage").
        if (edPct) lns.push_back({ rec(STAT_ITEM_MAXDAMAGE_PERCENT)->nDescPriority, "+" + std::to_string(edPct) + "% " + key("strModEnhancedDamage") });
        std::stable_sort(lns.begin(), lns.end(), [](const Ln& a, const Ln& b) { return a.prio > b.prio; });
        { std::set<std::string> seen; lns.erase(std::remove_if(lns.begin(), lns.end(), [&](const Ln& l) { return !seen.insert(l.text).second; }), lns.end()); }   // 21/22 and 23/24 fold to the same text
        for (const Ln& l : lns) { if (l.text.empty()) continue; if (!out.empty()) out.push_back('\n'); out += "\x01" "3"; out += l.text; }
    }
    // 1.13c D2Client+0x8E180: the ethereal and socketed lines, the LAST line (retail appends it first, the name last, and draws the string bottom-up).
    if (item->pItemData) {
        const uint32_t fl = item->pItemData->dwItemFlags;
        auto tbl = [&](int id) { std::string x; appendNarrow(x, langIdx(id)); return x; };
        const std::string fl3 = d2::item_text::flagLines(fl, (fl & d2::item_text::kSocketedItemFlag) ? ITEMS_GetSockets(item) : 0,
            tbl(d2::item_text::kEtherealStringId), tbl(d2::item_text::kSocketedStringId), tbl(3995));
        if (!fl3.empty()) { if (!out.empty()) out.push_back('\n'); out += fl3; }
    }
    const uint32_t n = out.size() < cap - 1 ? (uint32_t)out.size() : cap - 1;
    memcpy(buf, out.data(), n); buf[n] = 0;
    return n;
}

EMSCRIPTEN_KEEPALIVE uint32_t d2_session_inventory_layout(int rowId, int higherRes, int32_t* out, uint32_t cap) {
    // Inventory.txt is indexed nIndex = rowId + 16 * higherRes and the accessors D2_ASSERT on it,
    // so an out-of-range id does not return an error -- it aborts the whole engine. Refuse here.
    if (!out || cap < 12 || rowId < 0 || rowId >= 16 || higherRes < 0 || higherRes > 1) return 0;
    if (!sgptDataTables || !sgptDataTables->pInventoryTxt) return 0;
    if (rowId + 16 * higherRes >= sgptDataTables->nInventoryTxtRecordCount) return 0;
    D2InvRectStrc rect = {};
    D2InventoryGridInfoStrc grid = {};
    DATATBLS_GetInventoryRect(rowId, higherRes, &rect);
    DATATBLS_GetInventoryGridInfo(rowId, higherRes, &grid);
    out[0] = rect.nLeft; out[1] = rect.nRight; out[2] = rect.nTop; out[3] = rect.nBottom;
    out[4] = grid.nGridX; out[5] = grid.nGridY;
    out[6] = grid.nGridLeft; out[7] = grid.nGridRight; out[8] = grid.nGridTop; out[9] = grid.nGridBottom;
    out[10] = grid.nGridBoxWidth; out[11] = grid.nGridBoxHeight;
    // A row that exists has a panel with area; an id past the end of the table reads back zeros.
    return (rect.nRight > rect.nLeft && rect.nBottom > rect.nTop) ? 1 : 0;
}

// PlugY 10.00 patches the compiled InventoryBIN stash grid before the engine
// creates inventory grids. The caller must explicitly identify an opted-in
// local single-player bootstrap; all other modes preserve or restore retail rows.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_configure_plugy_stash_grid(int activeBigStash, int singlePlayer) {
    static D2InventoryTxt* configuredRecords = nullptr;
    static d2::plugy::BigStashSnapshot retailGrid{};
    static bool capturedRetailGrid = false;
    if (!sgptDataTables || !sgptDataTables->pInventoryTxt) return activeBigStash && singlePlayer ? 0 : 1;

    auto* records = sgptDataTables->pInventoryTxt;
    const auto count = static_cast<std::size_t>(sgptDataTables->nInventoryTxtRecordCount);
    if (records != configuredRecords) {
        configuredRecords = records;
        capturedRetailGrid = false;
    }
    return d2::plugy::configureBigStashGrid(records, count, activeBigStash != 0,
                                             singlePlayer != 0, retailGrid,
                                             capturedRetailGrid) ? 1 : 0;
}

// The equipment slots of an Inventory.txt row (DATATBLS_GetInventoryComponentGrid, D2Common #10637),
// in retail's component order (right arm, torso, left arm, head, neck, right ring, left ring, belt,
// feet, gloves): 6 int32 each -- left, right, top, bottom, width, height. OpenDiablo2's
// equipment_slot.go reads the same record. Same range guard as d2_session_inventory_layout.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_inventory_components(int rowId, int higherRes, int32_t* out, uint32_t cap) {
    if (!out || cap < 6 * NUM_INV_COMPS || rowId < 0 || rowId >= 16 || higherRes < 0 || higherRes > 1) return 0;
    if (!sgptDataTables || !sgptDataTables->pInventoryTxt) return 0;
    if (rowId + 16 * higherRes >= sgptDataTables->nInventoryTxtRecordCount) return 0;
    for (int i = 0; i < NUM_INV_COMPS; ++i) {
        D2InvCompGridStrc comp = {};
        DATATBLS_GetInventoryComponentGrid(rowId, higherRes, &comp, i);
        out[i * 6 + 0] = comp.rect.nLeft; out[i * 6 + 1] = comp.rect.nRight; out[i * 6 + 2] = comp.rect.nTop; out[i * 6 + 3] = comp.rect.nBottom;
        out[i * 6 + 4] = comp.nWidth; out[i * 6 + 5] = comp.nHeight;
    }
    return NUM_INV_COMPS;
}
// The player's own Inventory.txt row (UNITS_GetInventoryRecordId: the class's row, page 0).
EMSCRIPTEN_KEEPALIVE int32_t d2_session_player_inventory_record() {
    return (g_sessionLive && g_game && g_targetPlayer) ? UNITS_GetInventoryRecordId(g_targetPlayer, 0, g_game->bExpansion) : -1;
}
// The belt's box rectangles, from retail's own Belts.txt accessor.
//
// The client had these hardcoded as {x: 342, y: 561, cell: 31}, invented and undeclared
//. They are table data: DATATBLS_GetInvRectFromBeltsTxt is D2Common #10638
// and takes the 800x600 flag, so retail computes the layout and the client only draws it.
//
// Which Belts.txt row the player's belt is, for d2_session_belt_boxes below. UNITS_GetBeltType is
// retail's own accessor (D2Common #10367: ItemsTxt's nBelt column of the WORN belt item), and 2 is
// retail's fallback with no belt -- INVENTORY_GetFreeBeltSlot's own `int nBeltType = 2; if (pBelt)
// nBeltType = UNITS_GetBeltType(pBelt);`. The caller had this row hardcoded to 2, so a worn Sash,
// Belt, Heavy Belt, Girdle or Plated Belt still drew the four-box default row: every potion above
// the first row landed on the first row, and one in a slot past 3 was not drawn at all
//. Belts.txt gives the worn row 8/12/16 boxes at 800x600.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_belt_index() {
    if (!g_sessionLive || !g_targetPlayer || !g_targetPlayer->pInventory) return 2;
    D2UnitStrc* worn = INVENTORY_GetItemFromBodyLoc(g_targetPlayer->pInventory, BODYLOC_BELT);
    if (!worn || worn->dwUnitType != UNIT_ITEM) return 2;
    return (uint32_t)UNITS_GetBeltType(worn);
}

// out[0] = number of boxes; then 4 int32 per box (left, right, top, bottom).
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_belt_boxes(int beltIndex, int higherRes, int32_t* out, uint32_t cap) {
    if (!out || cap < 1) return 0;
    D2BeltsTxt record = {};
    DATATBLS_GetBeltsTxtRecord(beltIndex, higherRes, &record);
    const int boxes = record.nBoxes;
    if (boxes <= 0 || boxes > 16) return 0;
    if (cap < (uint32_t)(1 + boxes * 4)) return 0;
    out[0] = boxes;
    for (int i = 0; i < boxes; ++i) {
        D2InvRectStrc rect = {};
        DATATBLS_GetInvRectFromBeltsTxt(beltIndex, higherRes, &rect, i);
        out[1 + i * 4 + 0] = rect.nLeft;
        out[1 + i * 4 + 1] = rect.nRight;
        out[1 + i * 4 + 2] = rect.nTop;
        out[1 + i * 4 + 3] = rect.nBottom;
    }
    return (uint32_t)boxes;
}

EMSCRIPTEN_KEEPALIVE uint32_t d2_session_room_edge_flags(uint32_t roomOrdinal, uint32_t* out, uint32_t cap) {
    if (!g_sessionLive || !g_lastLevel || cap < 5) return 0;
    uint32_t ordinal = 0;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext, ++ordinal) {
        if (ordinal != roomOrdinal) continue;
        if (!roomEx->pOutdoor) return 0;          // preset rooms keep their grids elsewhere
        D2DrlgGridStrc* grid = &roomEx->pOutdoor->pFloorGrid;
        // A room may carry an outdoor struct whose grid was never built (preset rooms reuse the
        // slot). Walking it on the strength of a non-null pOutdoor reads out of bounds.
        if (!grid->pCellsFlags || !grid->pCellsRowOffsets) return 0;
        if (grid->nWidth <= 0 || grid->nHeight <= 0 || grid->nWidth > 64 || grid->nHeight > 64) return 0;
        out[0] = (uint32_t)grid->nWidth; out[1] = (uint32_t)grid->nHeight;
        out[2] = out[3] = out[4] = 0;
        for (int y = 0; y < grid->nHeight; ++y) {
            for (int x = 0; x < grid->nWidth; ++x) {
                const uint32_t flags = (uint32_t)DRLGGRID_GetGridEntry(grid, x, y);
                const bool edge = (x == roomEx->nTileWidth) || (y == roomEx->nTileHeight);
                if (edge) { if (flags & 0x2) ++out[2]; if (flags & 0x4) ++out[3]; }
                else if (flags & 0x2) ++out[4];
            }
        }
        return 1;
    }
    return 0;
}

EMSCRIPTEN_KEEPALIVE void d2_session_trace_rolls(int enable) {
    g_d2webRollTraceEnabled = enable != 0; D2WEB_ResetRollTrace();
}
EMSCRIPTEN_KEEPALIVE const D2WebRollTrace* d2_session_tile_rolls() {
    updateSessionSnapshot(); ensureSessionTiles(); return g_sessionTileRolls.data();
}
EMSCRIPTEN_KEEPALIVE int d2_session_tile_roll_count() {
    updateSessionSnapshot(); ensureSessionTiles(); return (int)g_sessionTileRolls.size();
}
extern std::vector<int32_t> g_d2webRollCalls;
extern std::vector<int32_t> g_d2webRollLevels;
// F511: the level of the room whose seed each roll record rolled, one int per record -- the record's
// own rect is not unique to a level (act I's outdoor levels share the coordinate space).
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_roll_levels() { return g_d2webRollLevels.data(); }
EMSCRIPTEN_KEEPALIVE int d2_session_roll_level_count() { return (int)g_d2webRollLevels.size(); }
// T12: ordered GetTileCache calls, 10 int32 each: room x,y (tiles), type, style, seq, entries, weightSum, ladder, chosen rarity, seed low before.
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_roll_calls() { return g_d2webRollCalls.data(); }
EMSCRIPTEN_KEEPALIVE int d2_session_roll_call_count() { return (int)(g_d2webRollCalls.size() / 10); }
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_roll_orphans() { return g_d2webRollOrphans; }
// Cross-room rolls as (room ordinal, that room's roll ordinal, causing room ordinal) triples,
// room ordinals in d2_session_rooms order; UINT32_MAX for a room outside the current level.
EMSCRIPTEN_KEEPALIVE int d2_session_cross_rolls(uint32_t* out, int cap) {
    if (!g_sessionLive || !g_lastLevel || !out) return 0;
    auto ordinalOf = [](const D2DrlgRoomStrc* p) {
        uint32_t n = 0;
        for (D2DrlgRoomStrc* r = g_lastLevel->pFirstRoomEx; r; r = r->pDrlgRoomNext, ++n) if (r == p) return n;
        return UINT32_MAX;
    };
    int written = 0;
    for (const D2WebCrossRoll& c : g_d2webCrossRolls) {
        if (written + 3 > cap) break;
        out[written++] = ordinalOf(c.pRoom); out[written++] = c.nOrdinal; out[written++] = ordinalOf(c.pCause);
    }
    return written / 3;
}
EMSCRIPTEN_KEEPALIVE const D2SessionTile* d2_session_tiles() {
    updateSessionSnapshot(); ensureSessionTiles(); return g_sessionTiles.data();
}
EMSCRIPTEN_KEEPALIVE const D2SessionActiveNearFeedRecord* d2_session_active_near_feed() {
    // The count call refreshes this view; read the pointer immediately afterward.
    return g_sessionActiveNearFeed.data();
}
EMSCRIPTEN_KEEPALIVE int d2_session_active_near_feed_count() {
    buildSessionActiveNearFeed(); return (int)g_sessionActiveNearFeed.size();
}
EMSCRIPTEN_KEEPALIVE const uint32_t* d2_session_tile_entries() {
    updateSessionSnapshot(); ensureSessionTiles(); return g_sessionTileEntry.data();
}
EMSCRIPTEN_KEEPALIVE const D2SessionTileCoord* d2_session_tile_coordlists() {
    updateSessionSnapshot(); ensureSessionTiles(); return g_sessionTileCoord.data();
}
EMSCRIPTEN_KEEPALIVE const D2SessionTileCoord* d2_session_seam_tile_coordlists() {
    updateSessionSnapshot(); ensureSessionTiles(); return g_sessionSeamTileCoord.data();
}
// F66: nIndex of the coord list under the player -- D2Client's DAT_6fbc3b1c, the value a wall record's own list is
// compared with (D2Common #10095 on the player's room and position). 0 when there is none, as retail's.
EMSCRIPTEN_KEEPALIVE int d2_session_player_coordlist() {
    if (!g_targetPlayer || !g_targetPlayer->pDynamicPath) return 0;
    D2ActiveRoomStrc* room = UNITS_GetRoom(g_targetPlayer);
    return room ? D2Common_10095(room, g_targetPlayer->pDynamicPath->tGameCoords.wPosX, g_targetPlayer->pDynamicPath->tGameCoords.wPosY) : 0;
}
EMSCRIPTEN_KEEPALIVE const uint32_t* d2_session_seam_tile_entries() {
    updateSessionSnapshot(); ensureSessionTiles(); return g_sessionSeamTileEntry.data();
}
EMSCRIPTEN_KEEPALIVE int d2_session_tile_count() {
    updateSessionSnapshot(); ensureSessionTiles(); return (int)g_sessionTiles.size();
}
// F44: the level's roof POP regions and what they have done to the tiles, so a count that did not move
// can say whether the preset has pops at all, which DT1 style each hides and where its region is. Read
// live (not from the session's tile snapshot): the pops are the engine's own per-tile state.
EMSCRIPTEN_KEEPALIVE const char* d2_session_pop_log() {
    static std::string out; out.clear();
    if (!g_lastLevel) return out.c_str();
    char buf[256];
    // One entry per MAP, not per room: every room the map generated carries its own pops, so a map
    // listed once per room would print the same region twelve times (it did, the first run).
    std::vector<const D2DrlgMapStrc*> seen;
    int presets = 0;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx && out.size() < 1600; roomEx = roomEx->pDrlgRoomNext) {
        if (roomEx->nType != DRLGTYPE_PRESET || !roomEx->pMaze || !roomEx->pMaze->pMap) continue;
        const D2DrlgMapStrc* map = roomEx->pMaze->pMap;
        if (!map->nPops || std::find(seen.begin(), seen.end(), map) != seen.end()) continue;
        seen.push_back(map);
        ++presets;
        for (int i = 0; i < map->nPops && out.size() < 1400; ++i) {
            snprintf(buf, sizeof buf, "pop %d: index %d hides style %d at level tile %d,%d %dx%d, active since tick %d (room %d,%d); ",
                i, map->pPopsIndex[i], map->pPopsSubIndex[i],
                map->pPopsLocation[i].nPosX, map->pPopsLocation[i].nPosY, map->pPopsLocation[i].nWidth, map->pPopsLocation[i].nHeight,
                map->pPopsOrientation[i], roomEx->nTileXPos, roomEx->nTileYPos);
            out += buf;
        }
    }
    if (g_sessionLive && g_targetPlayer && g_targetPlayer->pDynamicPath) {
        const D2DynamicPathStrc* path = g_targetPlayer->pDynamicPath;
        snprintf(buf, sizeof buf, "player at subtile %d,%d; ", path->tGameCoords.wPosX, path->tGameCoords.wPosY);
        out += buf;
    }
    int affected = 0, hidden = 0, revealed = 0;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
        if (!roomEx->pRoom || !roomEx->pRoom->pRoomTiles) continue;
        const D2DrlgRoomTilesStrc* tiles = roomEx->pRoom->pRoomTiles;
        for (int layer = 0; layer < 2; ++layer) {
            const D2DrlgTileDataStrc* list = layer ? tiles->pRoofTiles : tiles->pWallTiles;
            const int count = layer ? tiles->nRoofs : tiles->nWalls;
            for (int i = 0; i < count; ++i) {
                const uint32_t pop = sessionTilePop(list[i]);
                if (!pop) continue;
                ++affected;
                if (pop == 1) ++hidden; else ++revealed;
            }
        }
    }
    snprintf(buf, sizeof buf, "presets with pops %d; pop-affected tiles %d (hidden %d, shown %d)", presets, affected, hidden, revealed);
    out += buf;
    return out.c_str();
}
// D2Client's light map, which the floor draw samples. 48x48 subtile cells centred on the player; every cell starts
// at the ambient intensity; cells whose subtile collides with mask 0x22 (VISIBLE|BLANK) are marked
// blocked; each light source adds ((r - d) * ((intensity << 16) / r)) >> 16 to the cells of its square,
// d the octagonal distance ((max*0x3D7 + min*0x197) >> 10) in 1/8 subtiles from its position to the
// cell's corner, the cell clamped to 255. Source records use D2Client's unit position convention:
// precise path coordinate >> 13, then +4. The player radius is 13 + STAT_ITEM_LIGHTRADIUS (clamped
// to 18); objects use half of Objects.txt Lit for their current mode; monsters use max(MonStats2
// Light, component Items.txt LightRadius) and only exist for the target client's assigned room set,
// subject to the initial native dead/HIDE assignment condition and terminal SplEndDeath unlink.
// Native linked records ramp by 8 per client frame when their target changes. This session models
// initial enrollment and positive-radius FC150 replacement; no in-place monster target writer has
// been recovered. Not yet ported: the per-room ambient override and torch flicker. (The +0x149xxx
// addresses of an earlier change were wrong: the light code is at D2Client+0xA8000..0xAA000.)
struct D2SessionLightSource { int32_t type, classId, x, y, radius; };
// sourceColour: an object source's light colour, red | green << 8 | blue << 16 from its class's Red/Green/Blue
// columns (the three bytes the client makes the object's light with, D2Client+0x24920). 0 for every other source.
// Every cell also carries the colour of its light (`colour`, three bytes a cell): it starts as the area's
// ambient colour and each source that adds light to the cell blends its own colour in by weight
// (light_cell_blend.h; the client's colour-lighting switch is set by default). A player's light is white, a
// monster's is its MonStats2 colour, an object's its Objects.txt colour. The colour arrays sit after the
// fields the map had before, so a reader of the older layout is unaffected.
// Not done: the ambient colour's three special areas (the Den of Evil, the River of Flame and the Chaos
// Sanctuary take a colour of their own while their quest state holds) and a missile's light.
#include "../d2client/light_cell_blend.h"
struct D2SessionLightmap { int32_t originX, originY, ambient, sources; uint8_t intensity[48 * 48]; uint8_t blocked[48 * 48]; D2SessionLightSource source[64]; uint32_t sourceGuid[64]; uint32_t sourceColour[64]; uint32_t ambientColour; uint8_t colour[48 * 48 * 3]; };
static D2SessionLightmap g_sessionLightmap;
static uint32_t lightObjectColour(int type, int classId) {
    const D2ObjectsTxt* txt = type == UNIT_OBJECT ? DATATBLS_GetObjectsTxtRecord(classId) : nullptr;
    if (txt) return (uint32_t)txt->nRed | (uint32_t)txt->nGreen << 8 | (uint32_t)txt->nBlue << 16;
    if (type == UNIT_PLAYER) return 0xFFFFFFu;
    const D2MonStats2Txt* mon2 = type == UNIT_MONSTER ? UNITS_GetMonStats2TxtRecordFromMonsterId(classId) : nullptr;
    return mon2 ? (uint32_t)mon2->nLight_R | (uint32_t)mon2->nLight_G << 8 | (uint32_t)mon2->nLight_B << 16 : 0;
}
static int lightOctagonal(int a, int b) { a = a < 0 ? -a : a; b = b < 0 ? -b : b; return b <= a ? (a * 0x3D7 + b * 0x197) >> 10 : (b * 0x3D7 + a * 0x197) >> 10; }
// D2Client DRAW\LightMap.cpp's occlusion (1.13c D2Client+0xA9480 for units, +0xA9100/+0xA8F60 for
// objects, which cache the same pattern; an earlier change). Around the source's cell a 64x64 grid B holds
// 16 for every blocked cell (a map cell that collides 0x22, or -- for an object -- an off-map cell
// whose world subtile does; a monster's off-map cell counts blocked outright), 0 otherwise; a shadow grid G is filled ring by ring
// from distance 2 outward, eight symmetric cells per step, each from its neighbours toward the source
// (get = B if non-zero, else G): straight along an axis, or blended by the slope (f = minor<<8 / major)
// between the two neighbours on the source's side. A cell then receives its light scaled by
// (8 - G/2) / 8, none once G reaches 16.
static int lightB[64 * 64], lightG[64 * 64];
// Diagnostic for the off-map branch (F240/F250): how many off-map pattern cells the one-hop resolver
// could NOT reach this build. These now block as D2Common invalid cells; the count identifies scenes
// that exercise that fallback. Reset per d2_session_lightmap, read by d2_session_lightmap_offmap_unresolved.
static int g_lightOffMapUnresolved = 0;
// The same unresolved count asked of the PLAYER's room instead of the source's. It records the anchor
// F240 originally shipped; the nonzero count no longer means open because unresolved cells follow the
// same blocked invalid-cell rule. It is evidence, not a gate.
static int g_lightOffMapUnresolvedPlayer = 0;
static inline int lightGet(int row, int col) { const int i = (row << 6) + col; return lightB[i] ? lightB[i] : lightG[i]; }
static bool sessionRoomHasClient(const D2ActiveRoomStrc* room, const D2ClientStrc* client) {
    if (!room || !client || !room->ppClients || room->nNumClients <= 0) return false;
    for (int i = 0; i < room->nNumClients; ++i) if (room->ppClients[i] == client) return true;
    return false;
}
static bool sessionNativeMonsterAssignmentSuppressed(const D2UnitStrc* unit) {
    if (!unit || unit->dwUnitType != UNIT_MONSTER) return false;
    // D2Game 1.13c FUN_6FCFFA90 returns true for the dead flag or monster mode 0 (death)/12 (dead).
    // For a monster this is the first half of the initial ASSIGNNPC (0xAC) gate:
    // `(predicate == 0 || !Common#10341(unit))`. Common #10341 is the local HIDE state-mask
    // predicate (its export reads DataTables+0xD4 == fStateMasks[2]); ordinary live modes are
    // not suppressed by HIDE. This deliberately does not use an alive/awake approximation.
    const bool nativeGate = (unit->dwFlags & 0x00010000u) != 0 ||
        unit->dwAnimMode == MONMODE_DEATH || unit->dwAnimMode == MONMODE_DEAD;
    return nativeGate && STATES_CheckStateMaskOnUnit(const_cast<D2UnitStrc*>(unit), STATEMASK_HIDE);
}
static void updateSessionMonsterLights() {
    if (!g_sessionLive || !g_game || !g_targetClient || g_currentAct < 0 || g_currentAct >= NUM_ACTS ||
        !g_game->pAct[g_currentAct] ||
        !g_game->pAct[g_currentAct]->pDrlg) {
        g_sessionMonsterLights.clear();
        return;
    }
    for (auto& record : g_sessionMonsterLights) record.seen = false;
    for (D2DrlgLevelStrc* level = g_game->pAct[g_currentAct]->pDrlg->pLevel; level; level = level->pNextLevel) {
        for (D2DrlgRoomStrc* roomEx = level->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
            D2ActiveRoomStrc* room = roomEx->pRoom;
            if (!sessionRoomHasClient(room, g_targetClient)) continue;
            for (D2UnitStrc* unit = room->pUnitFirst; unit; unit = unit->pRoomNext) {
                if (unit->dwUnitType != UNIT_MONSTER || !unit->pDynamicPath) continue;
                auto it = std::find_if(g_sessionMonsterLights.begin(), g_sessionMonsterLights.end(),
                    [unit](const D2SessionMonsterLightRecord& record) {
                        return sessionMonsterLightIdentityMatches(record.unit, record.guid, record.classId,
                            unit, unit->dwUnitId, unit->dwClassId);
                    });
                // The dead/hidden predicate is an initial assignment gate. An already-created
                // client CUnit retains its light record until room removal or terminal unlink.
                if (it == g_sessionMonsterLights.end() && sessionNativeMonsterAssignmentSuppressed(unit)) continue;
                D2MonStatsTxt* stats = DATATBLS_GetMonStatsTxtRecord((int)unit->dwClassId);
                // D2Client's terminal death callback (#10426 -> mode-end handler 9) unlinks
                // the light only for rows marked SplEndDeath. Before that exact end predicate,
                // the death-animation CUnit remains a source.
                const bool terminalUnlink = stats && d2::monspawn::terminalDeathUnlinksLight(unit->dwAnimMode,
                        (uint8_t)stats->nSplEndDeath,
                        d2::monspawn::clientAnimationAtEnd(unit->pAnimSeq != nullptr,
                            (int32_t)unit->dwFrameCountPrecise,
                            (int32_t)unit->nSeqCurrentFramePrecise, unit->wAnimSpeed));
                if (terminalUnlink) {
                    if (it == g_sessionMonsterLights.end()) {
                        g_sessionMonsterLights.push_back({ unit, room, unit->dwUnitId,
                            static_cast<uint32_t>(unit->dwClassId),
                            (int)(unit->pDynamicPath->tGameCoords.dwPrecisionX >> 13) + 4,
                            (int)(unit->pDynamicPath->tGameCoords.dwPrecisionY >> 13) + 4,
                            0, 0, true, true });
                    } else {
                        it->seen = true;
                        it->room = room;
                        it->radius = it->targetRadius = 0;
                        it->terminalUnlinked = true;
                    }
                    continue;
                }
                if (it != g_sessionMonsterLights.end() && it->terminalUnlinked) {
                    it->seen = true;
                    it->room = room;
                    if (d2::monspawn::terminalLightTombstoneHeld(unit->dwAnimMode)) continue;
                    // The unit has entered a living mode again, so the client has a replacement
                    // CUnit lifecycle and may compute a fresh positive-radius record below.
                    it->terminalUnlinked = false;
                    it->radius = it->targetRadius = 0;
                }

                const D2MonStats2Txt* mon2 = UNITS_GetMonStats2TxtRecordFromMonsterId((int)unit->dwClassId);
                int light = mon2 ? (int)mon2->nLight : 0;
                const int itemLight = (int)MONSTERS_GetMaximalLightRadius(unit);
                if (itemLight > light) light = itemLight;
                const int targetRadius = (light > 18 ? 18 : (light < 0 ? 0 : light)) * 8;
                if (it == g_sessionMonsterLights.end()) {
                    if (!sessionMonsterLightShouldEnroll(targetRadius)) continue;
                    g_sessionMonsterLights.push_back({ unit, room, unit->dwUnitId, static_cast<uint32_t>(unit->dwClassId),
                        (int)(unit->pDynamicPath->tGameCoords.dwPrecisionX >> 13) + 4,
                        (int)(unit->pDynamicPath->tGameCoords.dwPrecisionY >> 13) + 4,
                        targetRadius, targetRadius, true });
                    continue;
                }
                it->seen = true;
                it->room = room;
                it->x = (int)(unit->pDynamicPath->tGameCoords.dwPrecisionX >> 13) + 4;
                it->y = (int)(unit->pDynamicPath->tGameCoords.dwPrecisionY >> 13) + 4;
                // FC150 recomputes a monster light on initialization and qualifying mode/action
                // updates by replacing the record. Its allocator starts current and target at the
                // new positive radius; FC150 returns early on zero without unlinking the old record.
                // Do not apply the generic linked-record ±8 ramp to a recalculated positive term.
                if (targetRadius > 0 && it->targetRadius != targetRadius)
                    sessionMonsterLightReplacementReset(*it, targetRadius);
            }
        }
    }
    g_sessionMonsterLights.erase(std::remove_if(g_sessionMonsterLights.begin(), g_sessionMonsterLights.end(),
        [](const D2SessionMonsterLightRecord& record) {
            return !record.seen || (!record.terminalUnlinked && record.radius == 0 && record.targetRadius == 0);
        }),
        g_sessionMonsterLights.end());
}
static void lightShadowStep(int x, int y, int row, int col, int px, int py) {
    int dx = x - px, dy = y - py;
    if (!dx && !dy) return;
    const int sx = dx < 0 ? -1 : 1, sy = dy < 0 ? -1 : 1; dx = dx < 0 ? -dx : dx; dy = dy < 0 ? -dy : dy;
    int v;
    if (!dx) v = lightGet(row + sy, col);
    else if (!dy) v = lightGet(row, col + sx);
    else if (dx >= dy) { const int f = (dy << 8) / dx; v = ((256 - f) * lightGet(row, col + sx) + f * lightGet(row + sy, col + sx)) >> 8; }
    else { const int f = (dx << 8) / dy; v = ((256 - f) * lightGet(row + sy, col) + f * lightGet(row + sy, col + sx)) >> 8; }
    lightG[(row << 6) + col] = v;
}
static void lightAddSource(int x, int y, int r, int intensity, bool occlude, bool offMapBlocks, D2ActiveRoomStrc* refRoom, D2ActiveRoomStrc* playerRoom, uint32_t rgb) {
    D2SessionLightmap& m = g_sessionLightmap;
    if (r <= 0 || r >= 0x100) return;
    const int x0 = (x - (x & 7)) - r, y0 = (y - (y & 7)) - r, n = (r * 2 >> 3) + 1, r8 = r >> 3;
    // A source whose pattern does not touch the 48x48 window lights no cell of it, so it is skipped
    // outright: exact (every cell it could write is outside the map) and it keeps the per-frame cost
    // to the sources that reach the window. Without it the Flayer Jungle -- hundreds of lit objects
    // over 4,772 tiles -- resolved a world collision for every off-map pattern cell of every one of
    // them and the frame row timed out at 120 s; with it only the three sources reaching the window do
    // (F240). Checked in cell coordinates, the same ones the write loop uses.
    const int px0 = x0 >> 3, px1 = (x0 + 8 * (n - 1)) >> 3, py0 = y0 >> 3, py1 = (y0 + 8 * (n - 1)) >> 3;
    if (px1 < m.originX || px0 > m.originX + 47 || py1 < m.originY || py0 > m.originY + 47) return;
    const int scale = (int)(((int64_t)intensity << 16) / r);
    memset(lightG, 0, sizeof lightG); memset(lightB, 0, sizeof lightB);
    if (occlude) {
        for (int iy = 0; iy < n; iy++) for (int ix = 0; ix < n; ix++) {
            const int wx = (x0 + 8 * ix) >> 3, wy = (y0 + 8 * iy) >> 3;
            const int gx = wx - m.originX, gy = wy - m.originY;
            const bool off = gx < 0 || gy < 0 || gx >= 48 || gy >= 48;
            bool blocked;
            if (!off) blocked = m.blocked[gy * 48 + gx] != 0;
            else if (offMapBlocks) blocked = true;
            else {
                // An object's occlusion pattern reaches cells outside the 48x48 window, and retail's
                // pattern cell there is NOT a constant: it reads the WORLD's own collision at that
                // subtile (the pattern is built from the collision grid, which extends past the map --
                // "The light map"). The fortress right edge is a wall, so those
                // cells block and the object lights nothing past them (F240); the open ground three
                // rows past a town torch does not, so that torch lights its cells fully. A monster's
                // off-map cell stays blocked (offMapBlocks), which is the rule the captures have shown
                // so far.
                //
                // `refRoom` MUST be the SOURCE's own room (the pattern is the source's, built from the
                // collision around it), never the player's: COLLISION_GetRoomBySubTileCoordinates tests
                // one room plus its DRLG neighbours (this file, d2_session_room_active_at's note), which
                // reaches the source's pattern but NOT its cells when the anchor is a player room
                // several rooms away. Those cells now take the native invalid-cell fallback and block,
                // while the source anchor still resolves real nearby cells so open ground remains open.
                // Anchoring on the player made the result depend on where the PLAYER stood, which the one
                // gate F240 had (`s2135`, its player beside the objects) could not see; the review found it.
                D2ActiveRoomStrc* wr = refRoom ? COLLISION_GetRoomBySubTileCoordinates(refRoom, wx, wy) : nullptr;
                if (!wr) ++g_lightOffMapUnresolved;   // diagnostic: outside the active-room resolver's reach
                if (playerRoom && playerRoom != refRoom)
                    if (!COLLISION_GetRoomBySubTileCoordinates(playerRoom, wx, wy)) ++g_lightOffMapUnresolvedPlayer;
                const uint16_t collision = wr
                    ? COLLISION_CheckMask(wr, wx, wy, COLLIDE_VISIBLE | COLLIDE_BLANK)
                    : 0;
                blocked = d2::monspawn::lightOcclusionCellBlocked(wr != nullptr, collision);
            }
            lightB[((32 - r8 + iy) << 6) + 32 - r8 + ix] = blocked ? 16 : 0;
        }
        const int cx = (x & ~7) + 4, cy = (y & ~7) + 4;
        for (int k = 2; k <= r8; k++) for (int j = 0; j <= k; j++) {
            lightShadowStep(x, y, 32 - k, 32 - j, cx - 8 * j, cy - 8 * k);
            lightShadowStep(x, y, 32 - k, 32 + j, cx + 8 * j, cy - 8 * k);
            lightShadowStep(x, y, 32 + k, 32 - j, cx - 8 * j, cy + 8 * k);
            lightShadowStep(x, y, 32 + k, 32 + j, cx + 8 * j, cy + 8 * k);
            lightShadowStep(x, y, 32 - j, 32 + k, cx + 8 * k, cy - 8 * j);
            lightShadowStep(x, y, 32 + j, 32 + k, cx + 8 * k, cy + 8 * j);
            lightShadowStep(x, y, 32 - j, 32 - k, cx - 8 * k, cy - 8 * j);
            lightShadowStep(x, y, 32 + j, 32 - k, cx - 8 * k, cy + 8 * j);
        }
    }
    for (int iy = 0; iy < n; iy++) for (int ix = 0; ix < n; ix++) {
        const int shade = lightG[((32 - r8 + iy) << 6) + 32 - r8 + ix];
        if (shade >= 16) continue;
        const int cx = x0 + 8 * ix, cy = y0 + 8 * iy;
        const int amount = ((((r - lightOctagonal(x - cx, y - cy)) * scale) >> 16) * (8 - (shade >> 1))) >> 3;
        if (amount <= 0) continue;
        const int gx = (cx >> 3) - m.originX, gy = (cy >> 3) - m.originY;
        if (gx < 0 || gy < 0 || gx >= 48 || gy >= 48) continue;
        uint8_t* c = &m.colour[(gy * 48 + gx) * 3];
        const d2::client::LightCell lit = d2::client::lightCellBlend({ m.intensity[gy * 48 + gx], c[0], c[1], c[2] }, amount,
            (uint8_t)rgb, (uint8_t)(rgb >> 8), (uint8_t)(rgb >> 16));
        m.intensity[gy * 48 + gx] = lit.level; c[0] = lit.r; c[1] = lit.g; c[2] = lit.b;
    }
}
// ambientOverride >= 0 replaces the act's environment intensity (a row replaying a capture's time of
// day, which is game state, not rule).
// occludeUnits: D2Client occludes a unit's light only above a quality setting (the dispatcher's
// argument > 1); objects always.
// Pure contract probe for the s1423 row: record identity includes pointer, GUID, and class;
// zero-light class reinitialization must not preserve the prior class's source; FC150 positive terms replace.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_monster_light_policy_probe() {
    auto* first = reinterpret_cast<D2UnitStrc*>(uintptr_t(0x1000));
    auto* recycled = reinterpret_cast<D2UnitStrc*>(uintptr_t(0x1000));
    auto* relocated = reinterpret_cast<D2UnitStrc*>(uintptr_t(0x2000));
    D2SessionMonsterLightRecord record{};
    record.unit = first;
    record.guid = 0x12345678u;
    record.classId = 7;
    record.radius = 16;
    record.targetRadius = 16;
    uint32_t result = 0;
    if (sessionMonsterLightIdentityMatches(record.unit, record.guid, record.classId, first, 0x12345678u, 7)) result |= 1u;
    if (!sessionMonsterLightIdentityMatches(record.unit, record.guid, record.classId, recycled, 0x87654321u, 7)) result |= 2u;
    if (!sessionMonsterLightIdentityMatches(record.unit, record.guid, record.classId, relocated, 0x12345678u, 7)) result |= 4u;
    if (!sessionMonsterLightIdentityMatches(record.unit, record.guid, record.classId, first, 0x12345678u, 8)) result |= 256u;
    const bool oldClassStillMatches = sessionMonsterLightIdentityMatches(record.unit, record.guid,
        record.classId, first, 0x12345678u, 8);
    const bool newZeroLightClassEnrolls = sessionMonsterLightShouldEnroll(0);
    if (!oldClassStillMatches && !newZeroLightClassEnrolls) result |= 512u;
    sessionMonsterLightReplacementReset(record, 40);
    if (record.radius == 40 && record.targetRadius == 40) result |= 8u;
    if (d2::monspawn::terminalDeathUnlinksLight(MONMODE_DEATH, 1, true)) result |= 16u;
    if (!d2::monspawn::terminalDeathUnlinksLight(MONMODE_DEATH, 2, true)) result |= 32u;
    if (!d2::monspawn::terminalDeathUnlinksLight(MONMODE_NEUTRAL, 1, true)) result |= 64u;
    if (!d2::monspawn::terminalDeathUnlinksLight(MONMODE_DEATH, 1, false)) result |= 128u;
    return result;
}

// The act's raw period of day (0 day, 1 dusk, 2 night, 3 dawn) -- D2Common #10396's value the D2Client ambience selector reads (1..3 = night variants).
EMSCRIPTEN_KEEPALIVE int d2_session_period_of_day() {
    if (!g_sessionLive || !g_game) return 0;
    return ENVIRONMENT_GetPeriodOfDayFromAct(g_game->pAct[g_currentAct], 0);
}

static void lightWindowAndPlayer(D2UnitStrc* player, D2DrlgActStrc* act, int ambientOverride, int occludeUnits) {
    D2SessionLightmap& m = g_sessionLightmap;
    const D2DynamicPathStrc* path = player->pDynamicPath;
    m.originX = (int)path->tGameCoords.wPosX - 24; m.originY = (int)path->tGameCoords.wPosY - 24;
    // The map's base is the LEVEL's own Levels.txt Intensity where the level defines one, and the act's
    // day/night environment ONLY for an OUTDOOR level where it defines none (0). An INDOOR level that
    // defines none stays at 0 -- its sources alone light it, which is why retail's caves, crypts and
    // tombs are dark. Retail's captured maps read exactly this: Act IV Town 140, Mesa 3 (the City of
    // the Damned) 40 and Lava 1 (the River of Flame) 120 are levels with their own Intensity; every
    // Act I..III outdoor level defines 0 and reads the environment instead (Act II town 141, Act III
    // docks 153, Harrogath 170); and every indoor level that defines 0 reads 0 -- the Halls of the Dead
    // L2 (57) and Catacombs 2 (35) at 0, and the Cathedral (33) at 0 as well, and the Cathedral is a
    // DRLGTYPE_PRESET level, so a DrlgType test would have sent it to the environment and is ruled out.
    // The flag is Levels.txt's own IsInside (`D2LevelsTxt`, loaded with the rest of the table into
    // sgptDataTables->pLevelsTxt): indoor is 1. Nothing in this build drove ENVIRONMENT_* before
    // d2_aiframe_advance, so using it for an indoor level lit every cave of every act alike -- which is
    // what F251 measured (`s2136`: Halls L2 base 128 against retail's 0, 2260 of 2304 cells).
    const int levelId = path->pRoom ? DUNGEON_GetLevelIdFromRoom(path->pRoom) : 0;
    const D2LevelDefBin* levelDef = levelId ? DATATBLS_GetLevelDefRecord(levelId) : nullptr;
    const int levelIntensity = levelDef ? (int)levelDef->nIntensity : 0;
    const bool indoors = levelId > 0 && sgptDataTables && sgptDataTables->pLevelsTxt
        && levelId < sgptDataTables->nLevelsTxtRecordCount && sgptDataTables->pLevelsTxt[levelId].nIsInside;
    m.ambient = ambientOverride >= 0 ? ambientOverride
        : (levelIntensity ? levelIntensity
            : (indoors ? 0 : ENVIRONMENT_GetIntensityFromAct(act)));
    memset(m.intensity, m.ambient > 255 ? 255 : m.ambient, sizeof m.intensity);
    // The ambient colour is the level's own (Levels.txt) where it names one, else the act's environment's.
    uint8_t ambientRgb[3] = { levelDef ? levelDef->nRed : (uint8_t)0, levelDef ? levelDef->nGreen : (uint8_t)0, levelDef ? levelDef->nBlue : (uint8_t)0 };
    if (!ambientRgb[0] && !ambientRgb[1] && !ambientRgb[2]) ENVIRONMENT_GetLightColorFromAct(act, &ambientRgb[0], &ambientRgb[1], &ambientRgb[2]);
    m.ambientColour = (uint32_t)ambientRgb[0] | (uint32_t)ambientRgb[1] << 8 | (uint32_t)ambientRgb[2] << 16;
    for (int i = 0; i < 48 * 48; i++) { m.colour[i * 3] = ambientRgb[0]; m.colour[i * 3 + 1] = ambientRgb[1]; m.colour[i * 3 + 2] = ambientRgb[2]; }
    for (int gy = 0; gy < 48; gy++) for (int gx = 0; gx < 48; gx++) {
        D2ActiveRoomStrc* room = COLLISION_GetRoomBySubTileCoordinates(path->pRoom, m.originX + gx, m.originY + gy);
        m.blocked[gy * 48 + gx] = room && COLLISION_CheckMask(room, m.originX + gx, m.originY + gy, COLLIDE_VISIBLE | COLLIDE_BLANK) ? 1 : 0;
    }
    const int px = (int)(path->tGameCoords.dwPrecisionX >> 13) + 4, py = (int)(path->tGameCoords.dwPrecisionY >> 13) + 4;
    // A player's light radius is a STAT, not the constant 13: D2Client recomputes a unit's light source
    // whenever STAT_ITEM_LIGHTRADIUS changes, giving a player unit 13 plus the stat's value (a monster
    // gets max(its own item light, MonStats2 Light)), and the source's radius is min(r, 18) * 8. Both
    // capture characters land on it exactly: the ungeared Oracle Barbarian reads 104 (13, stat 0) in
    // every Act I-III/V scene, and the level-27 bootstrap character reads 144 (18, the clamp, stat 7).
    // The act is not a variable in it (s880 reads each character's own stat). 
    const int playerLight = 13 + STATLIST_UnitGetStatValue(player, STAT_ITEM_LIGHTRADIUS, 0);
    // Retail clamps a non-positive radius to 0 before the setter; a light penalty deep enough to reach
    // it would otherwise put a negative radius in the source list this dump exposes.
    const int playerRadius = (playerLight > 18 ? 18 : (playerLight < 0 ? 0 : playerLight)) * 8;
    lightAddSource(px, py, playerRadius, 255, occludeUnits != 0, true, path->pRoom, path->pRoom, 0xFFFFFFu);
    m.source[0] = { 0, (int32_t)player->dwClassId, px, py, playerRadius };
    m.sourceGuid[0] = player->dwUnitId; m.sourceColour[0] = 0xFFFFFFu; m.sources = 1;
}

EMSCRIPTEN_KEEPALIVE const D2SessionLightmap* d2_session_lightmap(int ambientOverride, int occludeUnits) {
    D2SessionLightmap& m = g_sessionLightmap; memset(&m, 0, sizeof m); g_lightOffMapUnresolved = 0; g_lightOffMapUnresolvedPlayer = 0;
    updateSessionMonsterLights();
    if (!g_sessionLive || !g_targetPlayer || !g_targetPlayer->pDynamicPath || !g_lastLevel) return &m;
    const D2DynamicPathStrc* path = g_targetPlayer->pDynamicPath;
    lightWindowAndPlayer(g_targetPlayer, g_game->pAct[g_currentAct], ambientOverride, occludeUnits);
    for (D2DrlgLevelStrc* level = g_lastLevel->pDrlg ? g_lastLevel->pDrlg->pLevel : nullptr; level; level = level->pNextLevel)
        for (D2DrlgRoomStrc* roomEx = level->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext) {
            if (!roomEx->pRoom) continue;
            for (D2UnitStrc* unit = roomEx->pRoom->pUnitFirst; unit; unit = unit->pRoomNext) {
                int radius = 0, x = 0, y = 0;
                if (unit->dwUnitType == UNIT_OBJECT && unit->pStaticPath) {
                    const D2ObjectsTxt* txt = DATATBLS_GetObjectsTxtRecord((int)unit->dwClassId);
                    // Objects.txt Lit<mode> is a diameter: the torch's Lit1 19 is retail's radius 9 (72/8), the
                    // waypoint's 18 its 9 (town capture's source list).
                    if (txt && unit->dwAnimMode < 8) radius = txt->nLit[unit->dwAnimMode] >> 1;
                    x = unit->pStaticPath->tGameCoords.nX * 8 + 4; y = unit->pStaticPath->tGameCoords.nY * 8 + 4;
                } else if (unit->dwUnitType == UNIT_MONSTER && unit->pDynamicPath) {
                    const auto it = std::find_if(g_sessionMonsterLights.begin(), g_sessionMonsterLights.end(),
                        [unit](const D2SessionMonsterLightRecord& record) {
                            return sessionMonsterLightIdentityMatches(record.unit, record.guid, record.classId,
                                unit, unit->dwUnitId, unit->dwClassId);
                        });
                    if (it == g_sessionMonsterLights.end() || it->radius <= 0) continue;
                    radius = it->radius >> 3;
                    x = it->x; y = it->y;
                }
                if (radius <= 0) continue;
                const int r = (radius > 18 ? 18 : radius) * 8;
                // An object's pattern cell beyond the map reads the WORLD's own collision there (see
                // lightAddSource): off-map is not a constant, so a wall at the map's edge blocks and
                // open ground does not. A monster's off-map cell stays blocked. The anchor is the
                // UNIT's own room (`roomEx->pRoom`), not the player's (`path->pRoom`): the pattern is
                // the source's, built from the collision around it, and a player-room anchor cannot
                // reach an object's off-map cells when the player is several rooms away (the review's
                // finding).
                lightAddSource(x, y, r, 255, unit->dwUnitType == UNIT_OBJECT || occludeUnits != 0, unit->dwUnitType != UNIT_OBJECT, roomEx->pRoom, path->pRoom,
                    lightObjectColour((int)unit->dwUnitType, (int)unit->dwClassId));
                if (m.sources < 64) {
                    m.source[m.sources] = { (int32_t)unit->dwUnitType, (int32_t)unit->dwClassId, x, y, r };
                    m.sourceGuid[m.sources] = unit->dwUnitId;
                    m.sourceColour[m.sources] = lightObjectColour((int)unit->dwUnitType, (int)unit->dwClassId);
                }
                m.sources++;
            }
        }
    return &m;
}

#include "d2client_light_in.h"
// The Den of Evil quest byte (retail g_QuestAvail[1]) that 0x4C150 reads. The online client does not decode
// its source yet, so it stays 0 until the setter is fed; with 0 the override never applies.
static uint8_t g_denOfEvilQuestAvail = 0;
extern "C" EMSCRIPTEN_KEEPALIVE void d2_client_lightmap_set_den_quest_avail(int avail) { g_denOfEvilQuestAvail = avail ? 1 : 0; }

// The same light map for a client that holds its own units (the online client): the caller supplies the
// player and the other sources; the window, ambient, occlusion and falloff are the session's own.
// A monster entry (unit set) takes its position and radius from the unit the way the session does
// (max of MonStats2 Light and its item light, at most 18, the terminal SplEndDeath unlink); an object
// entry carries its position and Objects.txt Lit radius and the room it stands in.
extern "C" EMSCRIPTEN_KEEPALIVE const D2SessionLightmap* d2_client_lightmap_compose(D2UnitStrc* player, D2DrlgActStrc* act, const D2ClientLightIn* in, int n,
        int ambientOverride, int occludeUnits) {
    D2SessionLightmap& m = g_sessionLightmap; memset(&m, 0, sizeof m); g_lightOffMapUnresolved = 0; g_lightOffMapUnresolvedPlayer = 0;
    if (!player || !player->pDynamicPath || !act) return &m;
    const D2DynamicPathStrc* path = player->pDynamicPath;
    lightWindowAndPlayer(player, act, ambientOverride, occludeUnits);
    for (int i = 0; i < n; ++i) {
        int radius = in[i].radius, x = in[i].x, y = in[i].y;
        D2UnitStrc* unit = in[i].unit;
        if (unit) {
            if (unit->dwUnitType != UNIT_MONSTER || !unit->pDynamicPath) continue;
            D2MonStatsTxt* stats = DATATBLS_GetMonStatsTxtRecord((int)unit->dwClassId);
            if (stats && d2::monspawn::terminalDeathUnlinksLight(unit->dwAnimMode, (uint8_t)stats->nSplEndDeath,
                    d2::monspawn::clientAnimationAtEnd(unit->pAnimSeq != nullptr, (int32_t)unit->dwFrameCountPrecise,
                        (int32_t)unit->nSeqCurrentFramePrecise, unit->wAnimSpeed))) continue;
            const D2MonStats2Txt* mon2 = UNITS_GetMonStats2TxtRecordFromMonsterId((int)unit->dwClassId);
            const int itemLight = (int)MONSTERS_GetMaximalLightRadius(unit);
            // 0x4C150: the Den of Evil override reads the unit's own room level and MonStats nAlign.
            const D2ActiveRoomStrc* unitRoom = unit->pDynamicPath->pRoom;
            const int32_t levelId = unitRoom && unitRoom->pDrlgRoom && unitRoom->pDrlgRoom->pLevel ? unitRoom->pDrlgRoom->pLevel->nLevelId : -1;
            const bool den = stats && d2::monspawn::denOfEvilLightOverride(levelId, g_denOfEvilQuestAvail != 0, unit->dwFlags, stats->nAlign);
            radius = d2::monspawn::monsterLightRadius(den, mon2 ? (int)mon2->nLight : 0, itemLight);
            x = (int)(unit->pDynamicPath->tGameCoords.dwPrecisionX >> 13) + 4;
            y = (int)(unit->pDynamicPath->tGameCoords.dwPrecisionY >> 13) + 4;
        }
        if (radius <= 0) continue;
        const int r = (radius > 18 ? 18 : radius) * 8;
        const bool object = in[i].type == UNIT_OBJECT;
        D2ActiveRoomStrc* room = unit && unit->pDynamicPath->pRoom ? unit->pDynamicPath->pRoom : (in[i].room ? in[i].room : path->pRoom);
        lightAddSource(x, y, r, 255, object || occludeUnits != 0, !object, room, path->pRoom, lightObjectColour(in[i].type, in[i].classId));
        if (m.sources < 64) {
            m.source[m.sources] = { in[i].type, in[i].classId, x, y, r };
            m.sourceGuid[m.sources] = in[i].guid;
            m.sourceColour[m.sources] = lightObjectColour(in[i].type, in[i].classId);
        }
        m.sources++;
    }
    return &m;
}

// The off-map pattern cells the last d2_session_lightmap could NOT resolve (no built room answered from
// the source's own room), which now follow D2Common's invalid-collision result and block. Zero means
// every off-map cell the build asked about was inside the one-hop resolver's reach; non-zero is a
// diagnostic for F250's missing retail comparison case, not a behavior fallback.
EMSCRIPTEN_KEEPALIVE int d2_session_lightmap_offmap_unresolved() { return g_lightOffMapUnresolved; }
// The same unresolved count under the OLD anchor (the player's room): what F240 shipped before the
// review. It is a reachability diagnostic only; the current implementation blocks cells it cannot
// resolve under either anchor. Report it beside the source-anchor count, never gate on it.
EMSCRIPTEN_KEEPALIVE int d2_session_lightmap_offmap_unresolved_player() { return g_lightOffMapUnresolvedPlayer; }

EMSCRIPTEN_KEEPALIVE uint32_t d2_session_terrain_hash() {
    updateSessionSnapshot(); ensureSessionTiles(); return sessionTerrainHash();
}

// The DT1 tile libraries the current level's rooms actually loaded.
//
// This is what a client needs to draw the real map: `d2_session_tiles` gives a per-tile draw list
// straight out of the DRLG -- orientation, mainIndex/subIndex/frameIndex and screen coordinates --
// and each tile's `librarySlot` indexes THIS list.
//
// It deliberately does NOT walk LvlTypes.txt. That table lists the level TYPE's files, while rooms
// load what they need: a preset room pulls DT1s in through its DS1 that LvlTypes never mentions
// (Andariel's Lair loads Act1\Outdoors\Blank.dt1, absent from the Catacombs entry). Resolving
// through the type's table therefore pointed tiles at the wrong file or at none, and whole dungeon
// floors went undrawn. Paths are written back to back as NUL-terminated strings; returns the count.
// What ONE room loaded, for the dwDT1Mask question.
//
// d2_session_level_tile_files lists only libraries a placed tile came from, which cannot show a
// library that SHOULD have been loaded and was not -- the exact shape of the 43 fallbacks. This
// reports the room's own view instead, as newline-separated text:
//   mask <hex> type <levelType>
//   slot <i> <file>          -- pTiles[] in slot order, i.e. GetTiles' concatenation order
//   lvltype <bit> <file>     -- every non-empty LvlTypes.txt szFile the mask indexes
// Diagnostic only; nothing in the client reads it.
EMSCRIPTEN_KEEPALIVE int d2_session_room_libraries(uint32_t roomOrdinal, char* out, int cap) {
    if (!g_sessionLive || !g_lastLevel || !out || cap <= 0) return 0;
    uint32_t ordinal = 0;
    for (D2DrlgRoomStrc* roomEx = g_lastLevel->pFirstRoomEx; roomEx; roomEx = roomEx->pDrlgRoomNext, ++ordinal) {
        if (ordinal != roomOrdinal) continue;
        std::string text;
        char line[160];
        snprintf(line, sizeof line, "mask %08x type %d\n", roomEx->dwDT1Mask, (int)g_lastLevel->nLevelType);
        text += line;
        for (int slot = 0; slot < 32 && roomEx->pTiles[slot]; ++slot) {
            const char* name = D2WEB_TileLibraryNameFromHash(roomEx->pTiles[slot]);
            snprintf(line, sizeof line, "slot %d %s\n", slot, name ? name : "?");
            text += line;
        }
        if (D2LvlTypesTxt* types = DATATBLS_GetLevelTypesTxtRecord(g_lastLevel->nLevelType)) {
            for (int bit = 0; bit < 32; ++bit) {
                if (!types->szFile[bit][0]) continue;
                snprintf(line, sizeof line, "lvltype %d %.60s\n", bit, types->szFile[bit]);
                text += line;
            }
        }
        const int n = (int)std::min<size_t>(text.size(), (size_t)cap - 1);
        memcpy(out, text.data(), (size_t)n); out[n] = 0;
        return n;
    }
    return 0;
}

// The neighbouring levels' active-room tiles (g_sessionSeamTiles) and the DT1 files their
// librarySlot indexes, laid out as d2_session_tiles / d2_session_level_tile_files.
EMSCRIPTEN_KEEPALIVE const D2SessionTile* d2_session_seam_tiles() {
    updateSessionSnapshot(); ensureSessionTiles(); return g_sessionSeamTiles.data();
}
EMSCRIPTEN_KEEPALIVE int d2_session_seam_tile_count() {
    updateSessionSnapshot(); ensureSessionTiles(); return (int)g_sessionSeamTiles.size();
}
EMSCRIPTEN_KEEPALIVE int d2_session_seam_tile_files(char* out, int cap) {
    if (!g_sessionLive || !out || cap <= 0) return 0;
    updateSessionSnapshot(); ensureSessionTiles();
    int written = 0, used = 0;
    for (const std::string& name : g_sessionSeamLibraries) {
        const int n = (int)name.size() + 1;
        if (used + n > cap) break;
        memcpy(out + used, name.c_str(), (size_t)n);
        used += n; written++;
    }
    return written;
}
EMSCRIPTEN_KEEPALIVE int d2_session_level_tile_files(char* out, int cap) {
    if (!g_sessionLive || !out || cap <= 0) return 0;
    // The libraries the level's rooms ACTUALLY loaded, in the order a tile's librarySlot indexes.
    // This used to walk LvlTypes.txt instead; that table is the level TYPE's list and does not
    // include DT1s a preset room pulls in through its DS1, so tiles from those libraries resolved
    // against the wrong file or none at all and whole dungeon floors went undrawn.
    ensureSessionTiles();   // the last-built world's, as before (no rebuild here)
    int written = 0, used = 0;
    for (const std::string& name : g_sessionTileLibraries) {
        const int n = (int)name.size() + 1;
        if (used + n > cap) break;
        memcpy(out + used, name.c_str(), (size_t)n);
        used += n; written++;
    }
    return written;
}

// The name the local game gave its player: the one its own join packets carry (the in-process join selects the same).
EMSCRIPTEN_KEEPALIVE const char* d2_session_character_name() { return g_sessionCharName; }

EMSCRIPTEN_KEEPALIVE int d2_session_set_character_name(const char* name) {
    if (!name) return 0;
    const size_t n = strlen(name);
    if (n == 0 || n > sizeof g_sessionCharName - 1) return 0;
    memset(g_sessionCharName, 0, sizeof g_sessionCharName);
    memcpy(g_sessionCharName, name, n);
    if (g_targetClient) {
        memset(g_targetClient->szName, 0, sizeof g_targetClient->szName);
        memcpy(g_targetClient->szName, g_sessionCharName, n);
    }
    return 1;
}

// The creation screen's two checkboxes, as the retail client save flags they are: Expansion
// (CLIENTSAVEFLAG_EXPANSION) and Hardcore (CLIENTSAVEFLAG_HARDCORE). Called BEFORE the session is
// created, like d2_session_set_character_name, because both the game's mode (makeGame) and the
// client record the save header reads are built during create -- a live session's game mode is
// already fixed, so a late call is refused rather than writing a file that disagrees with the game
// it was played in. Returns 1 when recorded.
EMSCRIPTEN_KEEPALIVE int d2_session_set_client_flags(int expansion, int hardcore) {
    if (g_sessionLive) return 0;
    uint32_t flags = 0;
    if (expansion) flags |= CLIENTSAVEFLAG_EXPANSION;
    if (hardcore) flags |= CLIENTSAVEFLAG_HARDCORE;
    g_sessionClientSaveFlags = flags;
    return 1;
}

// The flags the session carries: what the .d2s header word at 0x24 will hold. Before a session is
// live this is the value d2_session_set_client_flags recorded (what create will start with); once
// the session is live it is the CLIENT's own flags (D2ClientStrc::tSaveFlags), because retail's own
// death path (PLRMODE_StartID_Death, PlrModes.cpp) toggles CLIENTSAVEFLAG_DEAD directly on the
// client with CLIENTS_ToggleFlag -- the static mirror this function used to return never saw it, so
// a hardcore player who had just died still read back HARDCORE|EXPANSION with no DEAD bit.
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_client_flags() {
    return g_targetClient ? (uint32_t)g_targetClient->tSaveFlags.nPackedValue : g_sessionClientSaveFlags;
}

// The complete .d2s for the live character. Returns the byte count written, 0 on failure; the
// PLRSAVE2 error code is available from d2_session_save_d2s_error().
static std::vector<uint8_t> g_sessionSaveFile;
static int g_sessionSaveError = 0;

EMSCRIPTEN_KEEPALIVE int d2_session_save_d2s(uint8_t* out, int cap) {
    g_sessionSaveError = 0;
    if (!g_sessionLive || !g_game || !g_targetPlayer || !out || cap <= 0) { g_sessionSaveError = -1; return 0; }
    // What a character FILE carries is the flags retail's session is left with, and retail clears a
    // unit's DELIVERY flags on every frame the unit is queued (D2Game 0x6FC3C5B0
    // `LEVEL_UpdateQueuedUnitsInAllActs` -> `sub_6FCBC300` -> `D2GAME_INVMODE_Last_6FC416D0` over the
    // unit's inventory). This tree had no caller for that flush anywhere (
    // C3q), so a start-kit item kept IFLAG_NEWITEM, which the equip path sets, all the way into the
    // file. Measured against retail 1.13c itself: a fresh Paladin it creates and saves writes
    // `00a22010` on each of its six potions and `00822010` on its sword and shield -- INSTORE |
    // STARTITEM | JUSTSAVED (| COMPACTSAVE) and no IFLAG_NEWITEM -- while this engine wrote
    // `00822011`. The reset is retail's own function applied to the player, at the boundary where
    // the file is composed: an item flags word that says "the client has not been told about this
    // item yet" is session bookkeeping, not part of the character.
    D2GAME_INVMODE_Last_6FC416D0(g_game, g_targetPlayer);
    constexpr uint32_t kCapacity = 0x2000;   // retail's own buffer size in D2GAME_SAVE_WriteFile
    g_sessionSaveFile.assign(kCapacity, 0);
    uint32_t written = 0;
    g_sessionSaveError = PLRSAVE2_CreateSaveFile(g_game, g_targetPlayer, g_sessionSaveFile.data(),
        &written, kCapacity, 0, 0);
    if (g_sessionSaveError != 0 || written == 0 || (int)written > cap) {
        if (g_sessionSaveError == 0) g_sessionSaveError = -2;
        return 0;
    }
    memcpy(out, g_sessionSaveFile.data(), written);
    return (int)written;
}

EMSCRIPTEN_KEEPALIVE int d2_session_save_d2s_error() { return g_sessionSaveError; }

// The character's saved automap, retail's `<character>.ma0`, written beside the .d2s at Save and Exit
// (F268;. WHICH cells exist is the CLIENT's reveal -- D2Game's own
// DUNGEON_AllocAct passes NULL for both automap callbacks, so the server reveals nothing and the layer
// lives in D2Client -- so the caller hands the cells its layer holds and this composes the FILE, exactly
// as d2_session_save_d2s composes the .d2s from the live session above. Layout: 100 little-endian dwords
// indexed by LAYER NUMBER (Levels.txt `Layer`), each the file offset of that layer's first chunk, 0 =
// none; then, per layer, a chunk = 32-byte header `{next, layer, 0, seed, floorBytes, wallBytes, 0, 0}`
// followed by floorBytes/6 floor cells then wallBytes/6 wall cells, each cell 6 bytes
// `{u16 cellNo (= the MaxiMap.dc6 frame), i16 x, i16 y}` in automap pixels. This writes ONE chunk per
// layer: retail chains several, each with a per-save-session `seed`, and the loader UNIONS every chunk of
// a layer without filtering on it, so a single chunk is a valid layer and the `seed` word's meaning stays
// unread -- it is written 0.
//
// `cells` is `count` quints of `{layer, frame, x, y, wall}`; returns the byte count written, or <= 0:
// -1 a bad argument, -2 the buffer is too small.
EMSCRIPTEN_KEEPALIVE int d2_session_save_ma0(const int32_t* cells, int count, uint8_t* out, int cap) {
    if (!cells || count < 0 || !out || cap <= 0) return -1;
    constexpr int kLayers = 100, kTableBytes = kLayers * 4;   // 0x190, the recovered header table
    struct Ma0Cell { int16_t frame, x, y; };
    static_assert(sizeof(Ma0Cell) == 6, "a .ma0 cell is {u16 cellNo, i16 x, i16 y}");
    std::vector<Ma0Cell> floors[kLayers], walls[kLayers];
    for (int i = 0; i < count; i++) {
        const int layer = cells[i * 5 + 0];
        if (layer < 0 || layer >= kLayers) continue;
        const Ma0Cell cell = { (int16_t)cells[i * 5 + 1], (int16_t)cells[i * 5 + 2], (int16_t)cells[i * 5 + 3] };
        (cells[i * 5 + 4] ? walls[layer] : floors[layer]).push_back(cell);
    }
    uint32_t table[kLayers], at = kTableBytes;
    for (int l = 0; l < kLayers; l++) {
        const uint32_t bytes = (uint32_t)(floors[l].size() + walls[l].size()) * 6;
        table[l] = bytes ? at : 0;
        if (bytes) at += 32 + bytes;
    }
    if ((int)at > cap) return -2;
    memset(out, 0, at);
    for (int l = 0; l < kLayers; l++) memcpy(out + l * 4, &table[l], 4);
    for (int l = 0; l < kLayers; l++) {
        if (!table[l]) continue;
        uint8_t* p = out + table[l];
        const uint32_t f = (uint32_t)floors[l].size() * 6, w = (uint32_t)walls[l].size() * 6;
        const uint32_t layer = (uint32_t)l, zero = 0, seed = 0;
        memcpy(p + 0, &zero, 4); memcpy(p + 4, &layer, 4); memcpy(p + 8, &zero, 4); memcpy(p + 12, &seed, 4);
        memcpy(p + 16, &f, 4); memcpy(p + 20, &w, 4); memcpy(p + 24, &zero, 4); memcpy(p + 28, &zero, 4);
        uint8_t* q = p + 32;
        for (const Ma0Cell& cell : floors[l]) { memcpy(q, &cell, 6); q += 6; }
        for (const Ma0Cell& cell : walls[l]) { memcpy(q, &cell, 6); q += 6; }
    }
    return (int)at;
}

// Load a .d2s into the live player. Returns 0 on success, otherwise the PLRSAVE2 error code.
EMSCRIPTEN_KEEPALIVE int d2_session_load_d2s(const uint8_t* data, int len) {
    if (!g_sessionLive || !g_game || !g_targetPlayer || !data || len <= 0) return -1;
    std::vector<uint8_t> copy(data, data + len);
    return D2WEB_PLRSAVE2_LoadIntoPlayer(g_game, g_targetPlayer, copy.data(), (uint32_t)copy.size());
}

EMSCRIPTEN_KEEPALIVE void d2_session_destroy();

// Create a character FROM a file -- the other half of PLRSAVE2_ProcessSaveFile, and what a
// character-select pick runs. Nothing here comes from the caller except the difficulty being
// played: the class of the unit the session allocates, the act it is in, the town level and the
// seed its map is generated from are all the FILE's (D2WEB_PLRSAVE2_ReadSaveHeader), which is the
// difference between this and d2_session_load_d2s above -- that one needs a live player of a class
// somebody else chose and only applies the sections to it.
//
// The name is retail's own check: a character joins a game as the name it logged in with, so the
// caller names the character it picked (d2_session_set_character_name) and the file must agree.
//
// Returns 0 on success, otherwise the PLRSAVE2 error code.
EMSCRIPTEN_KEEPALIVE int d2_session_create_from_save(const uint8_t* data, int len, int difficulty) {
    if (!data || len <= 0 || difficulty < 0 || difficulty >= NUM_DIFFICULTIES) return PLRSAVE2ERROR_INVALID_SIZE;
    // The reader (and the checksum) work on the buffer, so the caller's bytes are not touched.
    std::vector<uint8_t> copy(data, data + len);

    D2SaveHeaderReadStrc read = {};
    const int32_t nHeaderError = D2WEB_PLRSAVE2_ReadSaveHeader(copy.data(), (uint32_t)copy.size(), difficulty, &read);
    if (nHeaderError != PLRSAVE2ERROR_SUCCESS) return nHeaderError;

    const D2SaveHeaderStrc* pHeader = read.pHeader;

    // Retail's own name check (PLRSAVE2_ReadSaveHeader, _strcmpi): a character joins a game as the
    // name it logged in with, so the caller's name must be the file's -- case-insensitively.
    if (strcasecmp(g_sessionCharName, pHeader->szName) != 0) return PLRSAVE2ERROR_NOT_COMPATIBLE;

    // A map seed of 0 is "this difficulty was never entered": retail rolls a fresh map there and so
    // does the session's own fixture. Otherwise the character walks the map it was saved on.
    const uint32_t nSeed = read.dwMapSeed ? read.dwMapSeed : 1;
    const int nAct = read.nAct;
    const int nLevel = (int)DUNGEON_GetTownLevelIdFromActNo(nAct);

    // The game's mode has to be the FILE's, not whatever the creation-screen checkboxes left behind:
    // makeGame() builds g_game->bExpansion and the arena's GAMEFLAG_ARENA_HARDCORE from
    // g_sessionClientSaveFlags, and PLRSAVE2_CheckPlayerFlags below refuses a mismatch in either
    // direction (NOTEXPANSIONGAME/EXPANSIONGAME, HARDCOREJOINSOFTCORE/SOFTCOREJOINHARDCORE). Loading
    // a hardcore file after the classic-mode checkboxes built the game would always fail that check
    // before the header's own flags ever got applied to the client (C2c "still open").
    g_sessionClientSaveFlags = pHeader->dwSaveFlags & (CLIENTSAVEFLAG_EXPANSION | CLIENTSAVEFLAG_HARDCORE);

    // The class is the header's, and a bare player (no CharStats start kit) is what a saved
    // character needs -- the kit would sit on top of the file's own items (client-parity C3m).
    // T12/F67: a save resume is a town arrival at the act's OWN start -- `_class_bare` stands the player on
    // `DUNGEON_FindActSpawnLocationEx(act, town, 0)`, the same tile `LEVEL_ChangeAct` uses -- so the town is a
    // sight-lattice arrival and builds in the order retail's sight calls allocate its rooms (the reorder below).
    g_sessionFromSave = 1;
    g_sessionLoadSaveData = copy.data();               // F254: staged for the level build's A5Q3 pre-set
    g_sessionLoadSaveSize = (uint32_t)copy.size();
    const int nCreateOk = d2_session_create_class_bare(nSeed, difficulty, nAct, nLevel, read.nClass);
    g_sessionFromSave = 0;
    g_sessionLoadSaveData = nullptr;
    g_sessionLoadSaveSize = 0;
    if (!nCreateOk) return PLRSAVE2ERROR_UNKNOWN;

    // Everything below can only be asked once the game the character is joining exists.
    const int32_t nFlagsError = PLRSAVE2_CheckPlayerFlags(g_game, pHeader->dwSaveFlags);
    if (nFlagsError != PLRSAVE2ERROR_SUCCESS) { d2_session_destroy(); return nFlagsError; }

    CLIENTS_SetClassId(g_targetClient, read.nClass);
    CLIENTS_SetPlayerInClient(g_targetClient, g_targetPlayer);
    SUNIT_InitClientInPlayerData(g_targetPlayer, g_targetClient);
    UNITS_SetNameInPlayerData(g_targetPlayer, CLIENTS_GetName(g_targetClient));

    const int32_t nApplyError = D2WEB_PLRSAVE2_ApplySaveHeader(g_targetClient, g_targetPlayer, pHeader, nAct);
    if (nApplyError != PLRSAVE2ERROR_SUCCESS) { d2_session_destroy(); return nApplyError; }

    D2GAME_SUNITMSG_FirstFn_6FCC5520(g_game, g_targetPlayer, g_targetClient);

    const int32_t nLoadError = D2WEB_PLRSAVE2_LoadIntoPlayer(g_game, g_targetPlayer, copy.data(), (uint32_t)copy.size());
    if (nLoadError != PLRSAVE2ERROR_SUCCESS) { d2_session_destroy(); return nLoadError; }

    // A LOADED CHARACTER IS A GAME START, NOT A JOIN, and retail runs the quest game-start pass for
    // it. Both branches of retail's load (D2Game 0x6FC8CB40, `D2GAME_SAVE_GetUnitDataFromFile_6FC8CB40`
    // in the D2MOO prior art at `reference/D2MOO/source/D2Game/src/PLAYER/PlrSave.cpp` -- not vendored
    // here -- single player after D2GAME_SAVE_ReadFile_6FC8C9D0, and a realm client after
    // PLRSAVE2_ProcessSaveFile) end with `QUESTS_SequenceCycler(pGame, pPlayer, 0)`, AFTER the file's
    // sections are applied. With bGameEnter 0 that call reaches the block guarded by `bPickedSet`
    // and runs every quest's QUESTEVENT_PLAYERSTARTEDGAME callback -- the pass that rebuilds each
    // quest controller's fState/fLastState from the player's own saved quest record
    // (`ACT5Q1_Callback13_PlayerStartedGame`: QFLAG_STARTED -> fLastState 1, fState 2). The
    // new-character path is the one that passes bGameEnter 1 there, which is why `makeGame` marking
    // the flag set picked is faithful for a character with no save behind it.
    //
    // Without this the resumed character's every quest reads fState 0 while its record still says
    // STARTED, so the quest cannot advance on the level change that resumes it: MEASURED, s000e
    // step 10 "walk out of Harrogath into the Bloody Foothills" on a checkpoint resume, the engine
    // read `A5Q1 fState 0, word 4` as the hero arrived in level 110 (harness-out/act5-269-from9.log
    // and its base arm, engine 7162ff435c47) where retail's own ACT5Q1_Callback03_ChangedLevel
    // takes fState 2 -> 3 and sets ENTERAREA/LEAVETOWN.
    if (g_game->pQuestControl) g_game->pQuestControl->bPickedSet = 0;
    QUESTS_SequenceCycler(g_game, g_targetPlayer, 0);

    updateSessionSnapshot();
    return PLRSAVE2ERROR_SUCCESS;
}

// CONVERT TO EXPANSION (character select): retail rewrites a classic character as an expansion one. Here the
// engine does it: the classic file joins a classic game (create_from_save), the client's save flags gain
// CLIENTSAVEFLAG_EXPANSION, and PLRSAVE2_CreateSaveFile writes the file with the header word from those flags.
// The caller names the character first (d2_session_set_character_name). Returns the byte count, or <= 0:
// -1 a session is already live, -2 the file is not classic, -3 create failed, else the PLRSAVE2 error negated.
EMSCRIPTEN_KEEPALIVE int d2_session_convert_expansion(const uint8_t* data, int len, int difficulty, uint8_t* out, int cap) {
    if (g_sessionLive) return -1;
    // Load a classic file AS an expansion character (header flag set on a copy): the game's expansion
    // bit decides whether the writer emits retail's expansion-only "jf"/"kf" tail, so flipping the
    // client flag after a classic load wrote a file without them (s1003).
    if (len < 0x28 || (data[0x24] & 0x20)) return -2;
    std::vector<uint8_t> copy(data, data + len);
    copy[0x24] |= 0x20;
    { const uint32_t sum = (uint32_t)D2GAME_SAVE_FileChecksum((D2SaveHeaderStrc*)copy.data(), len);
      memcpy(&copy[12], &sum, 4); }  // Save-file checksum (checksum field zeroed by the routine)
    const int rc = d2_session_create_from_save(copy.data(), len, difficulty);
    if (rc != 0) return rc > 0 ? -rc : -3;
    int n = d2_session_save_d2s(out, cap);
    if (n <= 0) n = -3;
    d2_session_destroy();
    return n;
}

EMSCRIPTEN_KEEPALIVE int d2_session_level_type() {
    return (g_sessionLive && g_lastLevel) ? g_lastLevel->nLevelType : -1;
}
// Retail's Levels.txt `Rain` column for the level the player stands in -- the flag D2Client's weather
// pass starts from (client-parity F11b; 1 in the Rogue Encampment and Kurast Docktown and their
// outdoor wilderness, 0 in every Act II level and every interior). -1 outside a session.
EMSCRIPTEN_KEEPALIVE int d2_session_level_rain() {
    if (!g_sessionLive || !g_targetPlayer) return -1;
    D2ActiveRoomStrc* room = UNITS_GetRoom(g_targetPlayer);
    const int levelId = room ? DUNGEON_GetLevelIdFromRoom(room) : -1;
    return levelId < 0 ? -1 : (int)DATATBLS_GetRainFromLevelsTxt(levelId);
}
EMSCRIPTEN_KEEPALIVE const D2SessionWarp* d2_session_warps() {
    updateSessionSnapshot(); return g_sessionWarps.data();
}
EMSCRIPTEN_KEEPALIVE const int32_t* d2_session_warp_origins() {
    updateSessionSnapshot(); return g_sessionWarpOrigins.data();
}
EMSCRIPTEN_KEEPALIVE int d2_session_warp_count() {
    updateSessionSnapshot(); return (int)g_sessionWarps.size();
}
EMSCRIPTEN_KEEPALIVE uint32_t d2_session_navigation_hash() {
    updateSessionSnapshot(); return g_sessionNavigationHash;
}

EMSCRIPTEN_KEEPALIVE void d2_session_destroy() {
    g_clientInitSeed.clear(); g_clientRoomsActivated.clear();
    g_clientMonsterClock.clear(); d2_client_walk_reset(); // unit ids are reused by the next session
    g_clientObjectClock.clear();
    g_sessionMonsterLights.clear();
    resetSessionClientMapMirror();
    d2_monspawn_reset(); g_sessionLive = false; g_sessionMovePending = false;
    g_sessionAttackTarget = nullptr; g_sessionAttackSkill = nullptr;
    g_sessionApproachKind = 0; g_sessionApproachTarget = 0;
    g_sessionPendingObject = 0;
    g_sessionRooms.clear(); g_sessionUnits.clear(); g_sessionUnitCoords.clear(); g_sessionUnitFacings.clear(); g_sessionUnitOverlays.clear(); g_sessionUnitStates.clear(); g_sessionMonsterAnimations.clear(); g_sessionObjectPresentations.clear(); g_sessionItemPresentations.clear(); g_sessionItemColours.clear(); g_sessionItemInvColours.clear(); g_sessionItemSockets.clear(); g_sessionTiles.clear(); g_sessionSeamTiles.clear(); g_sessionTileRolls.clear(); g_sessionTileLibraries.clear(); g_sessionWarps.clear(); g_sessionWarpOrigins.clear(); g_sessionMissileAnimations.clear();
    memset(&g_sessionItemState, 0, sizeof g_sessionItemState);
    memset(&g_sessionNpcInteraction, 0, sizeof g_sessionNpcInteraction);
    memset(&g_sessionNpcDialog, 0, sizeof g_sessionNpcDialog); D2WEB_ResetLastDialogPacket(); D2WEB_ResetNpcAlerts();
    memset(&g_sessionNpcResponse, 0, sizeof g_sessionNpcResponse);
    memset(&g_sessionVendor, 0, sizeof g_sessionVendor);
    g_sessionVendorItems.clear(); memset(&g_sessionVendorTransaction, 0, sizeof g_sessionVendorTransaction);
    g_sessionWorldHash = D2VEC_FNV_INIT; g_sessionTerrainHash = D2VEC_FNV_INIT; g_sessionTerrainHashPending = false;
    g_sessionNavigationHash = D2VEC_FNV_INIT;
    // ... and back to the creation screen's defaults, so the next character's flags are its own.
    g_sessionClientSaveFlags = CLIENTSAVEFLAG_EXPANSION;
    updateSessionSnapshot();
}

} // extern "C"
