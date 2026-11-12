// The character screen's skill-row damage cell for a SkillDesc descdam 5 skill (Holy Fire and the other elemental
// auras, the elemental bolts, ...): the 1.13c D2Client+0x35270 port (d2client_35270.cpp) run on the live player.
//
// Retail D2Client+0xBCEA0 calls 0x2FD30 once per hand (UNITS_GetLeftSkill / UNITS_GetRightSkill). 0x2FD30 reaches the
// 0xF5210 callback for wDescDam with ecx = unit, edx = the skill, stack = SKILLS_GetSkillsTxtRecordFromSkill(skill)
// (via 0xC640), SKILLS_GetSkillLevel(unit, skill, 1) (via 0xC718), then the row's x, y, right words. Entry 5 is 0x35270.
// Its callees, as bound here:
//   0x2C830            D2Client_Map2C830 (pure table).
//   0x34D70            the 0x34D70 port (d2client_34d70.cpp): nothing when SrcDam (+0x1A5) is 0; otherwise the 0x31220
//                      port then the 0x30E30 port over two zeroed locals, each scaled by SrcDam/128. Their imports:
//                      0xC640 SKILLS_GetSkillsTxtRecordFromSkill, unit+0x60 the inventory, 0xCA0C INVENTORY_GetLeftHandWeapon,
//                      0xC89E INVENTORY_GetWieldType, 0xC1C6 STATLIST_UnitGetStatValue(layer 0), 0xC922
//                      SKILLS_GetWeaponMasteryBonus, 0xC940 / 0xC8CE ITEMS_GetStrengthBonus / ITEMS_GetDexBonus,
//                      0xC454 D2Common 0x39430 (the item's total stat 0x6F), 0xCEE6 / 0xCED4 the DamBlue / DamRed state masks.
//                      Unreached from this call site: 0xCEDA (31220's useSkill is 0 here), 0xC1BA (its result is unused),
//                      0x2FB50 and 0x2F980 (both behind the skills.txt FINISHING flag, byte +4 & gdwBitMasks[3]; no retail
//                      descdam-5 row carries it), so they are bound inert.
//   0xC85C / 0xC7F0    D2Common 0x52100 / 0x51FF0 ports (min / max physical, weapon source 0 so the unit is never read
//                      through its retail layout); kick -> the 0x4F930 port, formula -> SKILLS_EvaluateSkillFormula (0x51BF0).
//   0xCEB6             D2Common 0x4E900 port (elemental length, poison only); its calc evaluator -> 0x51BF0.
//   0xCEC2 / 0xCECE    SKILLS_GetMinElemDamage (0x50460) / SKILLS_GetMaxElemDamage, mastery on.
//   0x2F0A0            captured: ecx = min, eax = max, the colour mode. Its text rule: max := min + 1 unless min < max;
//                      "%d-%d" when both are under 10000, "%d-%dK" / "%dK-%dK" with thousands = (v + 500) / 1000.
// The retail-layout ports take 32-bit unit words; the live units are wasm64 pointers, so those words are handles here.
#include <cstdint>
#include <cstdio>
#include <emscripten.h>
#include "D2DataTbls.h"
#include "D2Skills.h"
#include "D2StatList.h"
#include "D2States.h"
#include "D2Inventory.h"
#include "D2Items.h"
#include "DataTbls/SkillsTbls.h"
#include "Units/Units.h"
#include "d2client_35140.h"
#include "d2client_33240.h"
#include "d2client_batch_30340.h"

static_assert(offsetof(D2SkillsTxt, nSrcDam) == 0x1A5, "SrcDam at +0x1A5");
static_assert(offsetof(D2SkillsTxt, nEType) == 0x1DC, "EType at +0x1DC");
static_assert(offsetof(D2UnitStrc, dwClassId) == 0x04, "unit class id at +0x04 (31220 reads the weapon's +4)");

uint32_t D2Client_Map2C830(uint32_t n);
extern "C" D2UnitStrc* d2_session_target_player_unit();
extern "C" void* d2_client_skilldesc_for_skill(int32_t nSkillId);
extern "C" int32_t d2_fn_d2common_4f930_port(D2UnitStrc* pUnit, int32_t nMin);
extern "C" EMSCRIPTEN_KEEPALIVE int32_t d2_port_d2common_39430(D2UnitStrc* pUnit, int32_t nStatId, uint16_t nLayer);

// The env layouts of the D2Common ports (d2common_batch_22ec0.cpp, d2common_51ff0.cpp, d2common_batch_4d840.cpp).
struct D2Common52100Env {
    int32_t (*kick_damage)(uint32_t, int32_t); uint32_t (*get_item)(uint32_t, int32_t); int32_t (*wield_type)(uint32_t, uint32_t);
    int32_t (*get_stat)(uint32_t, int32_t, int32_t); int32_t (*min_damage)(uint32_t, int32_t);
    int32_t (*damage_bonus_by_level)(int32_t, const int32_t*); int32_t (*evaluate_formula)(uint32_t, uint32_t, int32_t, uint32_t);
};
int32_t D2Common_52100_SkillsGetMinPhysDamage(uint32_t pUnit, int32_t nSkillId, uint32_t nSkillLevel, int32_t bUseWeapon, const D2Common52100Env* pEnv);
struct D2Common51FF0Env {
    int32_t (*sub4F930)(uint32_t, int32_t); uint32_t (*sub31B90)(uint32_t, int32_t); int32_t (*sub1FB80)(uint32_t, uint32_t);
    int32_t (*sub38F60)(uint32_t, int32_t); int32_t (*sub38B70)(uint32_t, int32_t, int32_t); int32_t (*sub51BF0)(uint32_t, uint32_t, int32_t, int32_t);
};
extern "C" int32_t d2_fn_d2common_51FF0(uint32_t pUnit, int32_t nSkillId, int32_t nSkillLevel, int32_t nScaleSource, const D2Common51FF0Env* env);
struct D2Common_10510_Env { int32_t (*pEvaluateSkillCalc)(uint8_t*, uint32_t, void*, void*, void*, const int32_t*); };
extern "C" int32_t D2Common_10510_GetSkillElementalLength(int32_t nContext, int32_t nSkillId, int32_t nLevel, const D2Common_10510_Env* pEnv);

namespace {
using namespace d2client_35140;
namespace b = d2client_batch_30340;
// One evaluation at a time: the unit, the skill and the mode cell behind the 32-bit words the ports pass around.
constexpr uint32_t kUnitH = 1, kSkillH = 2, kModeH = 3;
D2UnitStrc* gUnit; D2SkillStrc* gSkill; U* gMode;
D2UnitStrc* unitOf(uint32_t h) { return h == kUnitH ? gUnit : nullptr; }

// 0xC85C / 0xC7F0 / 0xCEB6 environments (the weapon source is 0 at this call site, so the weapon seams stay unread)
int32_t kick(uint32_t u, int32_t n) { return d2_fn_d2common_4f930_port(unitOf(u), n); }
int32_t stat3(uint32_t u, int32_t id, int32_t layer) { return STATLIST_UnitGetStatValue(unitOf(u), id, layer); }
int32_t bonusByLevel(int32_t lvl, const int32_t* t) { return SKILLS_CalculateDamageBonusByLevel(lvl, const_cast<int*>(t)); }
int32_t formula(uint32_t u, uint32_t calc, int32_t id, uint32_t lvl) { return SKILLS_EvaluateSkillFormula(unitOf(u), calc, id, (int)lvl); }
int32_t formula4(uint32_t u, uint32_t calc, int32_t id, int32_t lvl) { return SKILLS_EvaluateSkillFormula(unitOf(u), calc, id, lvl); }
int32_t lenCalc(uint8_t* code, uint32_t, void*, void*, void*, const int32_t* ctx) {
    const uint32_t calc = uint32_t(code - reinterpret_cast<uint8_t*>(sgptDataTables->pSkillsCode));
    return SKILLS_EvaluateSkillFormula(unitOf(uint32_t(ctx[0])), calc, ctx[1], ctx[2]);
}
const D2Common52100Env kMinEnv{ kick, nullptr, nullptr, stat3, nullptr, bonusByLevel, formula };
const D2Common51FF0Env kMaxEnv{ kick, nullptr, nullptr, nullptr, stat3, formula4 };
const D2Common_10510_Env kLenEnv{ lenCalc };
I dmgMin(P, I s, I l, I z) { return D2Common_52100_SkillsGetMinPhysDamage(kUnitH, s, uint32_t(l), z, &kMinEnv); }
I dmgMax(P, I s, I l, I z) { return d2_fn_d2common_51FF0(kUnitH, s, l, z, &kMaxEnv); }
I len(P, I s, I l, I) { return D2Common_10510_GetSkillElementalLength(int32_t(kUnitH), s, l, &kLenEnv); }
I eMin(P u, I s, I l, I one) { return SKILLS_GetMinElemDamage(static_cast<D2UnitStrc*>(u), s, l, one); }
I eMax(P u, I s, I l, I one) { return SKILLS_GetMaxElemDamage(static_cast<D2UnitStrc*>(u), s, l, one); }

// 0x31220 / 0x30E30 imports
D2UnitStrc* U_(P p) { return static_cast<D2UnitStrc*>(p); }
const uint8_t* recOf(P f) { return reinterpret_cast<const uint8_t*>(SKILLS_GetSkillsTxtRecordFromSkill(static_cast<D2SkillStrc*>(f))); }
P owner(P u) { return U_(u)->pInventory; }
P leftWeapon(P inv) { return INVENTORY_GetLeftHandWeapon(static_cast<D2InventoryStrc*>(inv)); }
void unusedC1BA(I) {}
I skStatUnreached(P, P, I) { return 0; }
I wield(P u, P inv) { return INVENTORY_GetWieldType(U_(u), static_cast<D2InventoryStrc*>(inv)); }
I stat2(P u, I id) { return STATLIST_UnitGetStatValue(U_(u), id, 0); }
I stat3p(P u, I id, I layer) { return STATLIST_UnitGetStatValue(U_(u), id, layer); }
I mastery(P u, P item, P f, I one) { return SKILLS_GetWeaponMasteryBonus(U_(u), U_(item), static_cast<D2SkillStrc*>(f), one); }
I finishingUnreached() { return 0; }
void finishing2F980Unreached(P, I*, I*, I*) {}
int16_t strBonus(P item) { return ITEMS_GetStrengthBonus(U_(item)); }
int16_t dexBonus(P item) { return ITEMS_GetDexBonus(U_(item)); }
I itemStat(P item, I id, I layer) { return d2_port_d2common_39430(U_(item), id, uint16_t(layer)); }
I damBlue(P u) { return STATES_CheckStateMaskDamBlueOnUnit(U_(u)); }
I damRed(P u) { return STATES_CheckStateMaskDamRedOnUnit(U_(u)); }
// D2Client+0xD69B4: colours, the stat rows (min, max, mastery percent or -1), and FINISHING = Fog gdwBitMasks[3]
const b::Tables kTables{ {1, 9, 3, 3}, {{48, 49, 329}, {50, 51, 330}, {54, 55, 331}, {52, 53, -1}}, 8, false };
const b::Ops31220 k31220{ recOf, owner, leftWeapon, unusedC1BA, skStatUnreached, wield, stat2, mastery, finishingUnreached,
                          strBonus, dexBonus, itemStat, damBlue, damRed };
const b::Ops30E30 k30E30{ stat3p, recOf, finishing2F980Unreached, damBlue, damRed };

// 0x34D70's two helpers over its locals: 31220 writes them through s[0]/s[1], 30E30 then adds into the same pair.
struct Live : d2client_33240::Ops {
    int32_t mn = 0, mx = 0;
    void c31220(uint32_t ecx, uint32_t eax, const uint32_t s[8]) override {
        b::sub_31220(k31220, kTables, int32_t(eax), unitOf(ecx), &mn, &mx, reinterpret_cast<I*>(s[2] == kModeH ? gMode : nullptr),
                     int32_t(s[3]), s[4] == kSkillH ? gSkill : nullptr, int32_t(s[5]), nullptr, int32_t(s[7]));
    }
    uint32_t c30E30(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t) override { return 0; }
    uint32_t c30E30loc(uint32_t u, int32_t* pMn, int32_t* pMx, uint32_t pType, uint32_t x) override {
        *pMn = mn; *pMx = mx;
        b::sub_30E30(k30E30, kTables, unitOf(u), pMn, pMx, reinterpret_cast<I*>(pType == kModeH ? gMode : nullptr), x == kSkillH ? gSkill : nullptr);
        return 0;
    }
};
void o34D70(U* pMode, U ecx, U esi, U edi, P, I* x, I* y) {
    gMode = pMode; Live o;
    d2client_33240::sub_34D70(o, kModeH, ecx, int32_t(esi), edi, kUnitH, x, y);
}
struct Emit { int32_t lo, hi; uint32_t mode; bool hit; } gEmit;
void emit(I eax, I ecx, U mode, U, U, U) { gEmit = { ecx, eax, mode, true }; }
char gText[2][32];
}

extern "C" {
// The skill-row damage cell of hand 0 (left) or 1 (right) for the session player, when that hand's skill has
// SkillDesc descdam 5: out[0] = 1 when 0x35270 drew, out[1] = its colour mode, out[2]/out[3] = the drawn pair after
// 0x2F0A0's rule; d2_client_char_skill_damage_text(hand) is the drawn text. 0 for any other skill (or none).
EMSCRIPTEN_KEEPALIVE int32_t d2_client_char_skill_damage(int32_t hand, int32_t* out) {
    if (!out || (hand != 0 && hand != 1)) return 0;
    out[0] = out[1] = out[2] = out[3] = 0; gText[hand][0] = 0;
    D2UnitStrc* unit = d2_session_target_player_unit();
    D2SkillStrc* skill = unit ? (hand ? UNITS_GetRightSkill(unit) : UNITS_GetLeftSkill(unit)) : nullptr;
    if (!skill) return 0;
    const uint8_t* desc = static_cast<const uint8_t*>(d2_client_skilldesc_for_skill(SKILLS_GetSkillIdFromSkill(skill, __FILE__, __LINE__)));
    if (!desc || (desc[0x12] | (desc[0x13] << 8)) != 5) return 0;
    gUnit = unit; gSkill = skill; gEmit = {};
    Ops o{ D2Client_Map2C830, o34D70, dmgMin, dmgMax, len, eMin, eMax, emit };
    sub_35270(o, unit, kSkillH, recOf(skill), SKILLS_GetSkillLevel(unit, skill, 1), 0, 0, 0);
    gUnit = nullptr; gSkill = nullptr; gMode = nullptr;
    if (!gEmit.hit) return 0;
    int32_t lo = gEmit.lo, hi = gEmit.hi;
    if (!(lo < hi)) hi = lo + 1;   // 0x2F0A0
    const auto k = [](int32_t v) { return int32_t(uint32_t(v) + 500u) / 1000; };
    if (lo < 10000 && hi < 10000) snprintf(gText[hand], sizeof gText[hand], "%d-%d", lo, hi);
    else if (lo < 10000) snprintf(gText[hand], sizeof gText[hand], "%d-%dK", lo, k(hi));
    else snprintf(gText[hand], sizeof gText[hand], "%dK-%dK", k(lo), k(hi));
    out[0] = 1; out[1] = int32_t(gEmit.mode); out[2] = lo; out[3] = hi;
    return 1;
}
EMSCRIPTEN_KEEPALIVE const char* d2_client_char_skill_damage_text(int32_t hand) { return (hand == 0 || hand == 1) ? gText[hand] : ""; }
}
