#include "d2gs_client_owned_item.h"
#include "../d2client/item_needs_repair.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <emscripten.h>

#include <D2Inventory.h>
#include <D2Items.h>
#include <D2StatList.h>
#include <D2States.h>
#include <D2ItemMods.h>
#include <DataTbls/ItemsTbls.h>
#include <DataTbls/MonsterTbls.h>
#include <D2DataTbls.h>
#include <Units/Item.h>
#include <Units/Units.h>

namespace {
constexpr uint32_t kRetailItemFormatVersion = 96;

// Every item unit this client holds, by GUID: the hero's, the other players' worn items, the items on
// the ground and the items socketed into other items. It stands for retail's client item hash.
std::unordered_map<uint32_t, D2UnitStrc*> g_items;
BOOL g_expansion = TRUE;

void release_item(D2UnitStrc* item) {
    if (!item || item->dwUnitType != UNIT_ITEM) return;
    if (item->pInventory) {   // a socketed parent owns its fillers
        while (D2UnitStrc* child = INVENTORY_GetFirstItem(item->pInventory)) {
            INVENTORY_RemoveItem(child);
            release_item(child);
        }
        INVENTORY_FreeInventory(item->pInventory);
    }
    auto found = g_items.find(item->dwUnitId);
    if (found != g_items.end() && found->second == item) g_items.erase(found);
    STATLIST_FreeStatListEx(item);
    if (item->pItemData) ITEMS_FreeItemData(nullptr, item);
    if (item->pStaticPath) UNITS_FreeStaticPath(item);
    UNITS_FreeUnit(item);
}

D2UnitStrc* decode_item(uint32_t guid, const uint8_t* bytes, int length) {
    if (!bytes || length <= 0) return nullptr;
    auto* item = UNITS_AllocUnit(nullptr, UNIT_ITEM);
    if (!item) return nullptr;
    item->dwUnitId = guid;
    // Upstream a650f49b: decode into an allocated stat list, so real item
    // properties (including the starter wand's +1 Raise Skeleton) survive.
    STATLIST_AllocStatListEx(item, 0, nullptr, nullptr);

    ITEMS_AllocItemData(nullptr, item);
    if (!item->pItemData) {
        release_item(item);
        return nullptr;
    }
    ITEMS_InitItemSeed(item);
    UNITS_AllocStaticPath(item);
    if (!item->pStaticPath) {
        release_item(item);
        return nullptr;
    }
    // The server item decoder writes its compact/static coordinates while decoding. A detached
    // path with a null room is the client-side unit's initial state before inventory insertion.
    UNITS_InitializeStaticPath(item, nullptr, 0, 0);

    int socketedItems = 0;
    BOOL failed = FALSE;
    const size_t consumed = ITEMS_DecodeItemFromBitstream(item,
        const_cast<uint8_t*>(bytes), static_cast<size_t>(length), FALSE,
        &socketedItems, kRetailItemFormatVersion, &failed);
    if (failed || consumed != static_cast<size_t>(length) || !item->pItemData) {
        release_item(item);
        return nullptr;
    }
    return item;
}

// 1.13c D2Client+0x81FF0. The client applies an item to its holder the way the server does: a gem
// or rune takes its mods for the parent item, a set item updates the set state, then the item's stat
// list joins the holder's unless the item is broken. A dual-wielding class merges a weapon that is
// not its left-hand weapon as the off-hand list. Switch-slot items contribute nothing.
void apply_item_stats(D2UnitStrc* owner, D2UnitStrc* item) {
    const bool gem = ITEMS_CheckItemTypeId(item, ITEMTYPE_GEM);
    if (gem || ITEMS_CheckItemTypeId(item, ITEMTYPE_RUNE)) {
        // A gem or rune gives its parent item the Gems.txt mods of the parent's kind (weapon, helm, shield).
        if (owner->dwUnitType != UNIT_ITEM) return;
        const D2ItemsTxt* txt = DATATBLS_GetItemsTxtRecord(item->dwClassId);
        const D2GemsTxt* mods = txt ? DATATBLS_GetGemsTxtRecord(txt->dwGemOffset) : nullptr;
        if (!mods) return;
        ITEMMODS_AssignProperty(gem ? 2 : 5, gem ? nullptr : owner, item, mods, ITEMS_GetGemApplyType(owner), 0);
    } else if (ITEMS_GetItemQuality(item) == ITEMQUAL_SET) {
        ITEMS_UpdateSets(owner, item, 0, 0);
    }
    if (ITEMS_CheckItemFlag(item, IFLAG_BROKEN, __LINE__, __FILE__)) return;
    BOOL mainWeapon = TRUE;
    if (UNITS_CanDualWield(owner)) {
        D2UnitStrc* left = owner->dwUnitType == UNIT_PLAYER ? INVENTORY_GetLeftHandWeapon(owner->pInventory) : nullptr;
        if (left != item && ITEMS_CheckItemTypeId(item, ITEMTYPE_WEAPON)) mainWeapon = FALSE;
    }
    STATLIST_MergeStatLists(owner, item, mainWeapon);
}

bool item_applied(D2UnitStrc* item) {
    return STATLIST_GetOwner(item, nullptr) != nullptr;
}

bool has_inventory(const D2UnitStrc* unit) {
    return unit && unit->pInventory && unit->pInventory->dwSignature == D2C_InventoryHeader;
}

bool player_owner(const D2UnitStrc* owner) {
    return owner && owner->dwUnitType == UNIT_PLAYER && has_inventory(owner) &&
        owner->pInventory->pOwner == owner && owner->pInventory->dwOwnerGuid == owner->dwUnitId;
}

D2UnitStrc* find_item(uint32_t guid) {
    auto found = g_items.find(guid);
    return found == g_items.end() ? nullptr : found->second;
}

bool flagged(D2UnitStrc* item, uint32_t flag) {
    return ITEMS_CheckItemFlag(item, flag, __LINE__, __FILE__) != 0;
}

int static_x(const D2UnitStrc* item) { return item->pStaticPath ? item->pStaticPath->tGameCoords.nX : 0; }
int static_y(const D2UnitStrc* item) { return item->pStaticPath ? item->pStaticPath->tGameCoords.nY : 0; }

// Takes an item out of whatever holds it (a grid, a body slot, the belt, the cursor, a socket). Its
// contribution to the holder ends with it, in the client's order: the weapon bookkeeping, the removal,
// the set count for a worn set item, then the stat list.
void detach(D2UnitStrc* item) {
    if (!item || !item->pItemData) return;
    D2InventoryStrc* inventory = item->pItemData->pExtraData.pParentInv;
    if (!inventory) return;
    D2UnitStrc* holder = inventory->pOwner;
    const uint8_t loc = item->pItemData->nBodyLoc;
    const bool worn = loc > BODYLOC_NONE && loc < NUM_BODYLOC &&
        INVENTORY_GetItemFromBodyLoc(inventory, loc) == item;
    if (worn) INVENTORY_UpdateWeaponGUIDOnRemoval(inventory, item);
    INVENTORY_RemoveItem(item);
    if (!holder) return;
    if (worn && ITEMS_GetItemQuality(item) == ITEMQUAL_SET) ITEMS_UpdateSets(holder, item, 1, 1);
    if (item_applied(item)) STATLIST_ExpireUnitStatlist(holder, item);
}

void destroy_item(D2UnitStrc* item) {
    detach(item);
    release_item(item);
}

// The item a record names, taken out of its old place and ready to be seated. The client keeps the
// unit it already holds and only rewrites its mode, page and flags from the record, unless the
// record's new-item flag is set or it holds no such unit: then the unit is built from the bitstream
// and replaces the held one. A stream that does not decode leaves the held item where it was.
D2UnitStrc* obtain(uint32_t guid, const D2ItemSaveStrc& record, const uint8_t* bits, int length,
        bool rebuild = false) {
    D2UnitStrc* held = find_item(guid);
    if (held && !rebuild && !(record.dwFlags & IFLAG_NEWITEM)) {
        detach(held);
        held->dwAnimMode = record.nAnimMode;
        ITEMS_SetInvPage(held, record.nStorePage);
        ITEMS_SetItemFlag(held, 0xffffffffu, FALSE);
        ITEMS_SetItemFlag(held, record.dwFlags, TRUE);
        return held;
    }
    D2UnitStrc* fresh = decode_item(guid, bits, length);
    if (!fresh) return nullptr;
    if (held) destroy_item(held);
    g_items[guid] = fresh;
    return fresh;
}

// Items of `inventory` in the given mode whose cells meet the rectangle are taken out and left
// unowned: the server names their new place in a record of their own.
// GUESS: retail's swap handlers move the displaced item themselves in one record; which record order the
// server uses is not read. Settled by a recorded container-swap packet sequence.
void clear_cells(D2InventoryStrc* inventory, D2UnitStrc* keep, int mode, int page, int x, int y, int w, int h) {
    for (D2UnitStrc* other = INVENTORY_GetFirstItem(inventory); other;) {
        D2UnitStrc* next = INVENTORY_GetNextItem(other);
        if (other != keep && INVENTORY_UnitIsItem(other) && static_cast<int>(other->dwAnimMode) == mode &&
            (mode != IMODE_STORED || ITEMS_GetInvPage(other) == page)) {
            uint8_t ow = 1, oh = 1;
            ITEMS_GetDimensions(other, &ow, &oh, __FILE__, __LINE__);
            if (mode == IMODE_INBELT) ow = oh = 1;
            const int ox = static_x(other), oy = mode == IMODE_INBELT ? 0 : static_y(other);
            if (ox < x + w && x < ox + ow && oy < y + h && y < oy + oh) detach(other);
        }
        other = next;
    }
}

bool seat_container(D2UnitStrc* owner, D2UnitStrc* item, const D2ItemSaveStrc& record) {
    if (record.nStorePage == INVPAGE_NULL) return true;   // no page: the item is held but lies nowhere
    const int grid = UNITS_GetInventoryRecordId(owner, record.nStorePage, g_expansion);
    item->dwAnimMode = IMODE_STORED;
    if (INVENTORY_PlaceItemAtInventoryPage(owner->pInventory, item, record.nX, record.nY, grid, FALSE,
            record.nStorePage)) return true;
    uint8_t w = 1, h = 1;
    ITEMS_GetDimensions(item, &w, &h, __FILE__, __LINE__);
    clear_cells(owner->pInventory, item, IMODE_STORED, record.nStorePage, record.nX, record.nY, w, h);
    return INVENTORY_PlaceItemAtInventoryPage(owner->pInventory, item, record.nX, record.nY, grid, FALSE,
        record.nStorePage) != 0;
}

bool seat_belt(D2UnitStrc* owner, D2UnitStrc* item, const D2ItemSaveStrc& record) {
    item->dwAnimMode = IMODE_INBELT;
    if (INVENTORY_PlaceItemInBeltSlot(owner->pInventory, item, record.nX)) return true;
    clear_cells(owner->pInventory, item, IMODE_INBELT, INVPAGE_NULL, record.nX, 0, 1, 1);
    return INVENTORY_PlaceItemInBeltSlot(owner->pInventory, item, record.nX) != 0;
}

void seat_cursor(D2UnitStrc* hero, D2UnitStrc* item) {
    D2UnitStrc* held = INVENTORY_GetCursorItem(hero->pInventory);
    if (held && held != item) detach(held);
    item->dwAnimMode = IMODE_ONCURSOR;
    INVENTORY_SetCursorItem(hero->pInventory, item);
}

// A body slot holds one item. The hero's displaced item stays held, unowned, until its own record
// seats it; another player's is appearance only and is dropped.
bool seat_body(D2UnitStrc* hero, D2UnitStrc* owner, D2UnitStrc* item, int loc) {
    if (loc <= BODYLOC_NONE || loc >= NUM_BODYLOC) return false;
    D2UnitStrc* occupant = INVENTORY_GetItemFromBodyLoc(owner->pInventory, loc);
    if (occupant && occupant != item) {
        if (owner == hero) detach(occupant); else destroy_item(occupant);
    }
    item->dwAnimMode = IMODE_EQUIP;
    if (!INVENTORY_PlaceItemInBodyLoc(owner->pInventory, item, loc)) return false;
    ITEMS_SetBodyLocation(item, static_cast<uint8_t>(loc));
    INVENTORY_UpdateWeaponGUIDOnInsert(owner->pInventory, item);
    return true;
}

// A filler joins its parent item's own inventory and its stat list joins the parent's.
// The parent's inventory is made on first use here; retail's item constructor makes it for a socketed item.
bool seat_socket(D2UnitStrc* parent, D2UnitStrc* item) {
    if (!parent || parent == item || parent->dwUnitType != UNIT_ITEM || !parent->pItemData) return false;
    if (!parent->pInventory && !INVENTORY_AllocInventory(nullptr, parent)) return false;
    if (!has_inventory(parent)) return false;
    item->dwAnimMode = IMODE_SOCKETED;
    if (!INVENTORY_PlaceItemInSocket(parent->pInventory, item, 0)) return false;
    if (!item_applied(item)) apply_item_stats(parent, item);
    return true;
}

// 1.13c D2Client+0x81C30: the requirement test of the sweep. An item that names a quiver type also
// needs an item of that type in the other hand.
bool sweep_usable(D2UnitStrc* owner, D2UnitStrc* item) {
    if (!ITEMS_CheckRequirements(item, owner, FALSE, nullptr, nullptr, nullptr)) return false;
    const int quiver = ITEMS_GetQuiverType(item);
    if (!quiver) return true;
    D2UnitStrc* other = INVENTORY_GetItemFromBodyLoc(owner->pInventory, BODYLOC_RARM);
    if (other == item) other = INVENTORY_GetItemFromBodyLoc(owner->pInventory, BODYLOC_LARM);
    return other && ITEMS_CheckItemTypeId(other, quiver);
}

void write_view(D2UnitStrc* item, uint32_t* out) {
    const D2ItemsTxt* txt = DATATBLS_GetItemsTxtRecord(item->dwClassId);
    const D2InventoryStrc* parent = item->pItemData->pExtraData.pParentInv;
    uint32_t fillers = 0;
    if (has_inventory(item))
        for (D2UnitStrc* child = INVENTORY_GetFirstItem(item->pInventory); child; child = INVENTORY_GetNextItem(child)) ++fillers;
    out[0] = item->dwUnitId;
    out[1] = static_cast<uint32_t>(item->dwClassId);
    out[2] = txt ? txt->dwCode : 0;
    out[3] = item->dwAnimMode;
    out[4] = static_cast<uint32_t>(static_x(item));
    out[5] = static_cast<uint32_t>(static_y(item));
    out[6] = item->pItemData->nInvPage;
    out[7] = item->pItemData->nBodyLoc;
    out[8] = item->pItemData->dwQualityNo;
    out[9] = item->pItemData->dwItemFlags;
    out[10] = parent && parent->pOwner ? parent->pOwner->dwUnitId : 0xffffffffu;
    out[11] = fillers | (item_applied(item) ? 0x100u : 0u);
}
}

extern "C" {

void d2_realm_client_owned_item_destroy(D2UnitStrc* item) {
    destroy_item(item);
}

// 0x42: the unit's cursor item is gone. 1 when it held one.
int d2_realm_client_item_clear_cursor(D2UnitStrc* owner) {
    if (!has_inventory(owner)) return 0;
    D2UnitStrc* held = INVENTORY_GetCursorItem(owner->pInventory);
    if (!held) return 0;
    destroy_item(held);
    return 1;
}

// Frees every item a unit holds (its grids, body, belt and cursor) before the unit itself goes.
void d2_realm_client_items_forget_owner(D2UnitStrc* owner) {
    if (!has_inventory(owner)) return;
    if (D2UnitStrc* held = INVENTORY_GetCursorItem(owner->pInventory)) destroy_item(held);
    while (D2UnitStrc* item = INVENTORY_GetFirstItem(owner->pInventory)) {
        INVENTORY_RemoveItem(item);
        release_item(item);
    }
}

// Frees the items that lie on the ground or are held by nobody (a new act sends its ground items afresh).
void d2_realm_client_items_forget_loose() {
    std::vector<uint32_t> loose;
    for (const auto& held : g_items)
        if (held.second->pItemData && !held.second->pItemData->pExtraData.pParentInv) loose.push_back(held.first);
    for (uint32_t guid : loose) release_item(find_item(guid));
}

// The item unit the client holds under this id, or null (the 0x3f and 0x7c packets name items by id).
D2UnitStrc* d2_realm_client_owned_item_find(uint32_t guid) { return find_item(guid); }
// 1.13c D2Client+0x45B40 over the client's own item unit: the repair confirm's test on the packet path.
extern "C" EMSCRIPTEN_KEEPALIVE int d2_realm_client_item_needs_repair(uint32_t guid) { return d2client_item_needs_repair(find_item(guid)); }
// The two item words retail D2Client+0x46EE0 writes into a single-item repair 0x35: field 0 the item's mode low
// word, field 1 its durability (stat 0x48). -1 for an item the client does not hold.
extern "C" EMSCRIPTEN_KEEPALIVE int d2_realm_client_item_repair_field(uint32_t guid, int field) {
    D2UnitStrc* item = find_item(guid);
    if (!item) return -1;
    return field == 0 ? static_cast<int>(static_cast<uint16_t>(item->dwAnimMode)) : STATLIST_UnitGetStatValue(item, STAT_DURABILITY, 0);
}

// A unit-removal record for an item: an item on the ground goes; one somebody holds is not touched.
// GUESS: retail's removal handler for an item unit is not read; this is the rule the screen copy used.
// Settled by the decompile of the type-4 branch of the 0x0A handler.
int d2_realm_client_item_remove(uint32_t guid) {
    D2UnitStrc* item = find_item(guid);
    if (!item || !item->pItemData || item->pItemData->pExtraData.pParentInv) return 0;
    release_item(item);
    return 1;
}

void d2_realm_client_items_reset() {
    while (!g_items.empty()) {
        D2UnitStrc* item = g_items.begin()->second;
        g_items.erase(g_items.begin());
        if (item->pItemData && item->pItemData->pExtraData.pParentInv) INVENTORY_RemoveItem(item);
        release_item(item);
    }
}

// Whether the character is a Lord of Destruction one: it picks the Inventory.txt rows of the grids.
EMSCRIPTEN_KEEPALIVE void d2_realm_client_items_set_expansion(int expansion) {
    g_expansion = expansion ? TRUE : FALSE;
}

// 1.13c D2Client+0x82FE0, the sweep the client runs after a worn item or a base stat changes.
// 1. A usable charm in the backpack that is not contributing starts to.
// 2. A worn item (slots 0..10) that fails the requirement test is marked unusable and stops contributing.
// 3. Up to 101 rounds: a worn, unbroken item that is marked or not contributing and now meets its
//    requirements (and has its quiver partner) is unmarked and contributes; one item's bonus can
//    satisfy another's requirement.
// 4. Every worn, usable set item is taken off the owner's stat list and applied again, which
//    recounts the set bonuses.
// Returns the number of items whose state changed in steps 2 and 3.
int d2_realm_client_owned_item_reconcile(D2UnitStrc* owner) {
    if (!player_owner(owner)) return 0;
    D2InventoryStrc* inventory = owner->pInventory;
    int changes = 0;
    for (D2UnitStrc* item = INVENTORY_GetFirstItem(inventory); item; item = INVENTORY_GetNextItem(item)) {
        if (!INVENTORY_UnitIsItem(item) || INVENTORY_GetItemNodePage(item) != 1) continue;
        if (ITEMS_IsCharmUsable(item, owner) && !item_applied(item) && sweep_usable(owner, item)) {
            ITEMS_SetItemFlag(item, IFLAG_NOEQUIP, FALSE);
            apply_item_stats(owner, item);
        }
    }
    for (int loc = 0; loc < BODYLOC_SWRARM; ++loc) {
        D2UnitStrc* item = INVENTORY_GetItemFromBodyLoc(inventory, loc);
        if (!item) continue;
        const bool applied = item_applied(item);
        if (flagged(item, IFLAG_BROKEN) && !applied) continue;
        if (flagged(item, IFLAG_NOEQUIP) && !applied) continue;
        if (sweep_usable(owner, item)) continue;
        ITEMS_SetItemFlag(item, IFLAG_NOEQUIP, TRUE);
        INVENTORY_UpdateWeaponGUIDOnRemoval(inventory, item);   // 1.13c D2Client+0x82610
        STATLIST_ExpireUnitStatlist(owner, item);
        ++changes;
    }
    for (int round = 0; round < 101; ++round) {
        bool moved = false;
        for (int loc = 0; loc < BODYLOC_SWRARM; ++loc) {
            D2UnitStrc* item = INVENTORY_GetItemFromBodyLoc(inventory, loc);
            if (!item || flagged(item, IFLAG_BROKEN)) continue;
            if (!flagged(item, IFLAG_NOEQUIP) && item_applied(item)) continue;
            if (!sweep_usable(owner, item)) continue;
            ITEMS_SetItemFlag(item, IFLAG_NOEQUIP, FALSE);
            const uint8_t at = ITEMS_GetBodyLocation(item);
            if (at != BODYLOC_SWRARM && at != BODYLOC_SWLARM) {
                INVENTORY_UpdateWeaponGUIDOnRemoval(inventory, item);
                if (item->dwAnimMode == IMODE_EQUIP && !item_applied(item)) apply_item_stats(owner, item);
                INVENTORY_UpdateWeaponGUIDOnInsert(inventory, item);
            }
            moved = true;
            ++changes;
        }
        if (!moved) break;
    }
    for (int loc = 0; loc < BODYLOC_SWRARM; ++loc) {
        D2UnitStrc* item = INVENTORY_GetItemFromBodyLoc(inventory, loc);
        if (!item || ITEMS_GetItemQuality(item) != ITEMQUAL_SET) continue;
        if (flagged(item, IFLAG_BROKEN) || flagged(item, IFLAG_NOEQUIP)) continue;
        STATLIST_ExpireUnitStatlist(owner, item);
        apply_item_stats(owner, item);
    }
    return changes;
}

// One item record of the server, 0x9C (the hero's own item, or one on the ground) or 0x9D (an item
// on a named unit: a player, or the item whose socket it fills). `hero` is this client's player,
// `owner` the unit a 0x9D record names when it is a player (null for an item owner, which is looked up
// here). The item ends where the record says: on the hero's or another player's unit, as in the local
// game, so every reader of the unit sees it. Returns 1 when the record changed something, 0 when it
// names nothing this client holds, -1 for a malformed record.
int d2_realm_client_item_packet(D2UnitStrc* hero, D2UnitStrc* owner,
        const uint8_t* packet, int length) {
    if (!packet || length < 9 || (packet[0] != 0x9c && packet[0] != 0x9d)) return -1;
    const bool owned = packet[0] == 0x9d;
    const int head = owned ? 13 : 8;
    if (packet[2] != length || length <= head) return -1;
    const uint8_t action = packet[1];
    uint32_t guid = 0;
    std::memcpy(&guid, packet + 4, 4);
    const uint8_t* bits = packet + head;
    const int size = length - head;
    D2ItemSaveStrc record{};
    if (!ITEMS_GetCompactItemDataFromBitstreamVersioned(const_cast<uint8_t*>(bits), static_cast<size_t>(size),
            FALSE, &record, kRetailItemFormatVersion)) return 0;
    if (guid == D2UnitInvalidGUID) return 0;
    const bool heroReady = player_owner(hero);

    if (!owned) {
        switch (action) {
        case ITEMACTION_ADDTOGROUND: case ITEMACTION_DROPTOGROUND: case ITEMACTION_ONGROUND: {
            // The unit is held with its ground mode; the ground picture is the screen's.
            D2UnitStrc* item = obtain(guid, record, bits, size);
            // The record's coordinates are the ground cell; the room is bound when the unit is looked up.
            if (item && item->pStaticPath) UNITS_InitializeStaticPath(item, nullptr, record.nX, record.nY);
            return item ? 1 : 0;
        }
        case ITEMACTION_GROUNDTOCURSOR: case ITEMACTION_TOCURSOR: {
            if (!heroReady) return 0;
            D2UnitStrc* item = obtain(guid, record, bits, size, true);
            if (!item) return 0;
            seat_cursor(hero, item);
            return 1;
        }
        case ITEMACTION_PUTINCONTAINER: {
            if (!heroReady) return 0;
            D2UnitStrc* item = obtain(guid, record, bits, size);
            if (!item) return 0;
            if (seat_container(hero, item, record) && ITEMS_IsCharmUsable(item, hero) && !item_applied(item))
                apply_item_stats(hero, item);
            return 1;
        }
        case ITEMACTION_ADDQUANTITY: {
            // The held item is decoded again where it lies.
            D2UnitStrc* held = find_item(guid);
            if (!held || !heroReady) return 0;
            const bool onCursor = INVENTORY_GetCursorItem(hero->pInventory) == held;
            const bool inBelt = held->dwAnimMode == IMODE_INBELT;
            D2ItemSaveStrc place = record;
            place.nX = static_cast<uint16_t>(static_x(held));
            place.nY = static_cast<uint16_t>(static_y(held));
            place.nStorePage = ITEMS_GetInvPage(held);
            D2UnitStrc* item = obtain(guid, record, bits, size, true);
            if (!item) return 0;
            if (onCursor) seat_cursor(hero, item);
            else if (inBelt) seat_belt(hero, item, place);
            else seat_container(hero, item, place);
            return 1;
        }
        case ITEMACTION_ADDTOSHOP: case ITEMACTION_REMOVEFROMSHOP:
            return 0;   // a vendor's stock, not the hero's
        case ITEMACTION_SWAPINCONTAINER: case ITEMACTION_SWAPINBELT: {
            // The record names a held item: one lying in the grid is lifted to the cursor, the one on
            // the cursor is laid down where the record says.
            if (!heroReady) return 0;
            D2UnitStrc* held = find_item(guid);
            if (!held) return 0;
            const bool lift = INVENTORY_GetCursorItem(hero->pInventory) != held;
            D2UnitStrc* item = obtain(guid, record, bits, size);
            if (!item) return 0;
            if (lift) seat_cursor(hero, item);
            else if (action == ITEMACTION_SWAPINBELT) seat_belt(hero, item, record);
            else if (seat_container(hero, item, record) && ITEMS_IsCharmUsable(item, hero) && !item_applied(item))
                apply_item_stats(hero, item);
            return 1;
        }
        case ITEMACTION_PUTINBELT: {
            if (!heroReady) return 0;
            D2UnitStrc* item = obtain(guid, record, bits, size);
            if (!item) return 0;
            seat_belt(hero, item, record);
            return 1;
        }
        case ITEMACTION_REMOVEFROMBELT: {
            if (!heroReady) return 0;
            D2UnitStrc* held = find_item(guid);
            if (!held) return 0;
            if (record.dwFlags & IFLAG_QUANTITY) { destroy_item(held); return 1; }   // merged into a stack
            D2UnitStrc* item = obtain(guid, record, bits, size);
            if (!item) return 0;
            seat_cursor(hero, item);
            return 1;
        }
        default:
            return 0;   // retail's handler takes no other action
        }
    }

    const uint8_t ownerType = packet[8];
    uint32_t ownerGuid = 0;
    std::memcpy(&ownerGuid, packet + 9, 4);
    if (ownerType == UNIT_ITEM) {
        if (action != ITEMACTION_ITEMINSOCKET) return 0;
        D2UnitStrc* parent = find_item(ownerGuid);
        if (!parent) return 0;
        D2UnitStrc* item = obtain(guid, record, bits, size);
        if (!item) return 0;
        if (!seat_socket(parent, item)) { destroy_item(item); return 0; }
        return 1;
    }
    // A monster (a hireling) has no inventory in this client yet; only players hold items.
    if (ownerType != UNIT_PLAYER || !player_owner(owner) || owner->dwUnitId != ownerGuid) return 0;
    const bool mine = owner == hero;

    switch (action) {
    case ITEMACTION_REMOVEFROMCONTAINER: {
        D2UnitStrc* held = find_item(guid);
        if (!held || !held->pItemData || held->pItemData->pExtraData.pParentInv != owner->pInventory) return 0;
        if ((record.dwFlags & IFLAG_QUANTITY) || !mine) { destroy_item(held); return 1; }
        D2UnitStrc* item = obtain(guid, record, bits, size);
        if (!item) return 0;
        ITEMS_SetInvPage(item, INVPAGE_NULL);
        seat_cursor(owner, item);
        return 1;
    }
    case ITEMACTION_EQUIP: case ITEMACTION_SWAPBODYITEM: case ITEMACTION_WEAPONSWITCH:
    case ITEMACTION_INDIRECTLYSWAPBODYITEM: case ITEMACTION_UNKNOWN0x14: case ITEMACTION_UPDATESTATS:
    case ITEMACTION_UNKNOWN0x16: {
        // Equip is read from the handler. GUESS for 7, 9's swap-out half, 0x14, 0x15, 0x16 and 0x17:
        // their handlers are not read in full; the item is seated where its own record says it is
        // (worn, on the cursor, in a grid or the belt). Settled by reading D2Client 0x861A0, 0x84300,
        // 0x854E0, 0x84F00, 0x83790 and 0x85B30.
        D2UnitStrc* item = obtain(guid, record, bits, size);
        if (!item) return 0;
        const int mode = record.nAnimMode;
        if (mode == IMODE_EQUIP) {
            if (!seat_body(hero, owner, item, record.nBodyloc)) { destroy_item(item); return 0; }
            if (record.nBodyloc < BODYLOC_SWRARM && !item_applied(item) && !flagged(item, IFLAG_NOEQUIP))
                apply_item_stats(owner, item);
        } else if (!mine) {
            destroy_item(item);   // off another player's body: nothing of it is shown
            return 1;
        } else if (mode == IMODE_ONCURSOR) seat_cursor(owner, item);
        else if (mode == IMODE_INBELT) seat_belt(owner, item, record);
        else if (mode == IMODE_STORED) seat_container(owner, item, record);
        d2_realm_client_owned_item_reconcile(owner);
        d2_realm_client_owned_item_reconcile(owner);
        return 1;
    }
    case ITEMACTION_UNEQUIP: {
        D2UnitStrc* held = find_item(guid);
        if (!held || !held->pItemData || held->pItemData->pExtraData.pParentInv != owner->pInventory) return 0;
        if (!mine || (record.dwFlags & IFLAG_QUANTITY)) {
            destroy_item(held);
            d2_realm_client_owned_item_reconcile(owner);
            return 1;
        }
        D2UnitStrc* item = obtain(guid, record, bits, size);
        if (!item) return 0;
        d2_realm_client_owned_item_reconcile(owner);
        ITEMS_SetBodyLocation(item, BODYLOC_NONE);
        seat_cursor(owner, item);
        d2_realm_client_owned_item_reconcile(owner);
        d2_realm_client_owned_item_reconcile(owner);
        ITEMS_SetItemFlag(item, IFLAG_NOEQUIP, FALSE);
        return 1;
    }
    case ITEMACTION_AUTOUNEQUIP: {
        D2UnitStrc* held = find_item(guid);
        if (!held) return 0;
        if (!mine) { destroy_item(held); d2_realm_client_owned_item_reconcile(owner); return 1; }
        D2UnitStrc* item = obtain(guid, record, bits, size);
        if (!item) return 0;
        ITEMS_SetBodyLocation(item, BODYLOC_NONE);
        seat_container(owner, item, record);
        d2_realm_client_owned_item_reconcile(owner);
        d2_realm_client_owned_item_reconcile(owner);
        return 1;
    }
    default:
        return 0;
    }
}

// The items a player unit holds, 12 words each: guid, class id, code, mode, x, y, page, body slot,
// quality, flags, holder guid, fillers | 0x100 when the item contributes to its holder. The cursor
// item comes first when there is one. Returns the count written.
int d2_realm_client_unit_items(const D2UnitStrc* owner, uint32_t* out, int capacity) {
    if (!player_owner(owner) || !out || capacity <= 0) return 0;
    int count = 0;
    D2UnitStrc* cursor = INVENTORY_GetCursorItem(owner->pInventory);
    if (cursor && cursor->pItemData) write_view(cursor, out + 12 * count++);
    for (D2UnitStrc* item = INVENTORY_GetFirstItem(owner->pInventory); item && count < capacity;
            item = INVENTORY_GetNextItem(item)) {
        if (item == cursor || !INVENTORY_UnitIsItem(item) || !item->pItemData) continue;
        write_view(item, out + 12 * count++);
    }
    return count;
}

// The tests the item-box colour is chosen from, the bits of the local game's d2_session_item_box_state:
//   0x001 requirements not met   0x002 item flag 0x4   0x004 a stat list carrying flag 2
//   0x008 the player is in state 54   0x010 the item cannot be equipped
//   0x040 shoots or quiver item with nothing in the other hand   0x080 identified
//   0x100 Items.txt transmogrify set   0x200 found
// 0x020 (a quest item not yet usable) is never set: the joined game's quest record is not on the unit.
uint32_t d2_realm_client_unit_item_box_state(D2UnitStrc* owner, uint32_t guid) {
    D2UnitStrc* item = find_item(guid);
    const D2ItemsTxt* txt = item && player_owner(owner) ? DATATBLS_GetItemsTxtRecord(item->dwClassId) : nullptr;
    if (!txt) return 0;
    uint32_t out = 0x200;
    if (!ITEMS_CheckRequirements(item, owner, FALSE, nullptr, nullptr, nullptr)) out |= 0x001;
    if (flagged(item, 0x4)) out |= 0x002;
    if (STATLIST_GetStatListFromUnitAndFlag(item, 2)) out |= 0x004;
    if (STATES_CheckState(owner, 54)) out |= 0x008;
    if (!ITEMS_CanBeEquipped(item)) out |= 0x010;
    const uint8_t loc = ITEMS_GetBodyLocation(item);
    D2UnitStrc* pair = loc == BODYLOC_RARM || loc == BODYLOC_LARM
        ? INVENTORY_GetItemFromBodyLoc(owner->pInventory, loc == BODYLOC_RARM ? BODYLOC_LARM : BODYLOC_RARM) : nullptr;
    if (!pair) {
        const D2ItemTypesTxt* type = DATATBLS_GetItemTypesTxtRecord(static_cast<int>(ITEMS_GetItemType(item)));
        if (type && (type->wShoots || type->wQuiver)) out |= 0x040;
    }
    if (flagged(item, IFLAG_IDENTIFIED)) out |= 0x080;
    if (txt->nTransmogrify) out |= 0x100;
    return out;
}

EMSCRIPTEN_KEEPALIVE int d2_realm_client_owned_item_readback(const D2UnitStrc* owner,
        int bodyLoc, D2RealmOwnedItemView* out) {
    if (!out) return 0;
    *out = {};
    out->version = 1;
    out->size = sizeof(*out);
    if (!player_owner(owner) || bodyLoc <= BODYLOC_NONE || bodyLoc >= NUM_BODYLOC) return 0;
    D2UnitStrc* item = INVENTORY_GetItemFromBodyLoc(owner->pInventory, bodyLoc);
    if (!item || item->dwUnitType != UNIT_ITEM || !item->pItemData) return 0;
    out->guid = item->dwUnitId;
    out->classId = static_cast<uint32_t>(item->dwClassId);
    const D2ItemsTxt* record = DATATBLS_GetItemsTxtRecord(item->dwClassId);
    out->code = record ? record->dwCode : 0;
    out->ownerGuid = item->pItemData->dwOwnerGUID;
    out->flags = item->pItemData->dwItemFlags;
    out->mode = item->dwAnimMode;
    out->bodyLoc = item->pItemData->nBodyLoc;
    out->unitAddress = reinterpret_cast<uintptr_t>(item);
    return 1;
}

}  // extern "C"
