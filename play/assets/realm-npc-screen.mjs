// The online town NPC on screen: the retail menu (npc-menu.mjs), its hire window (npc-menu.mjs showHire), the speech
// box (speech-box.mjs), the vendor window (vendor-panel.mjs) and the Buy confirm box, drawn from the state machine's
// view and fed the clicks of realm-npc-ui.mjs. The renderers are the caller's own instances, so the realm path never
// shares state with the single-player NPC window.
import { npcMenuChoiceOnRelease } from './npc-menu.mjs';
import { vendorItemsFrom } from './realm-npc-ui.mjs';
import { createNpcSpeechClickGate } from './npc-speech-click.mjs';
import { BUY_TITLE_STRING, REPAIR_TITLE_STRING, GOLD_STRING, YES_STRING, NO_STRING, CONTINUE_STRING } from './vendor-repair-confirm.mjs';

/** The engine's offer rule when the caller injects none: left press confirms, right press buys 500 ms after a send. */
function fallbackOfferClick(now) {
  let last = 0;
  return {
    sent() { last = now() >>> 0; },
    action({ button = 0 }) {
      if (button === 0) return 'confirm';
      if (button === 2 && (now() >>> 0) - last >= 500) return 'buy';
      return 'none';
    },
  };
}

/** The hire offer as npc-menu's hire window reads it; a stat the seam does not know is drawn as '?'. */
const menuOffer = (o) => ({ mercName: o.mercName, name: o.name, level: o.level ?? '?', life: o.life ?? '?', def: o.def ?? '?',
  cost: o.cost ?? '?', skill: o.skill ?? '' });

/**
 * deps: ui (createRealmNpcUi), view() (realm-npc-state view), menu (createNpcMenu, the NPC menu and hire window),
 * buyConfirm (createNpcMenu, the Buy box), vendor (createVendorPanel), speech (createSpeechBox), itemTable
 * (createItemTable), text(stringId) (the string table), anchor(guid) (the NPC's screen feet {x, y} or null),
 * smith(classId) (the vendor's smith rule), nameOf(classId) (the NPC's name), itemNameOf(guid) (the stocked item's
 * name for the Buy box, '' while no name seam is wired), gold() (the hero's gold for the hire window, null if unknown),
 * offerClick (createStoreOfferClick: the engine's offer rule, optional).
 */
export function createRealmNpcScreen({ ui, view, menu, buyConfirm = null, vendor, speech, itemTable, text = () => '', anchor = () => null,
  smith = () => false, now = () => 0, nameOf = () => '', itemNameOf = () => '', gold = () => null, offerClick = null,
  itemDetail = null, stashGold = () => -1, speechReady = () => true }) {
  const gate = createNpcSpeechClickGate();   // the per-speech 100 ms mouse-down gate
  const rule = offerClick ?? fallbackOfferClick(now);
  let gateKey = null;
  let shown = null;          // the view the menu was last opened for: { guid, mode }
  let vendorOpenFor = null;  // the guid whose stock the vendor window was reset for
  let pressedEntry = null;   // the menu row under the press, fired on the matching release
  let pressedButton = null;  // the vendor button under the press, acted on release
  let hireKey = null;        // the hire window the offers were shown for: `${guid}:${count}`
  let confirm = null;        // the Buy box: { guid } for the offer the left press armed
  let vendorState = null;    // what the vendor window was last drawn with (its overlay reads the same)

  const current = () => view();

  function syncMenu(v) {
    const wantMenu = v && (v.mode === 'menu' || v.mode === 'talk') && v.entries;
    if (!wantMenu) { if (menu.open) menu.close(); shown = null; return; }
    if (!shown || shown.guid !== v.guid || shown.mode !== v.mode || shown.entries !== v.entries) {
      menu.show(v.guid, v.classId, nameOf(v.classId), v.entries);   // nameOf: the NPC's NameStr, localized
      shown = { guid: v.guid, mode: v.mode, entries: v.entries };
    }
  }

  /** The Buy box: 'Buy', the item's name, 'Gold:', the cost ('?' while unknown), Yes / No. Nothing is sent from here. */
  function openConfirm(v, guid) {
    const price = ui.priceAt ? ui.priceAt(guid) : null;
    confirm = { guid };
    if (buyConfirm) {
      buyConfirm.show(v.guid, 0, [text(BUY_TITLE_STRING) || 'Buy', itemNameOf(guid), text(GOLD_STRING) || 'Gold:', price == null ? '?' : String(price)],
        [{ stringId: YES_STRING, action: 'yes' }, { stringId: NO_STRING, action: 'no' }]);
    }
  }
  function closeConfirm() {
    confirm = null;
    if (buyConfirm?.open) buyConfirm.close();
  }
  /** The Repair box (D2Client 0x47500 state 3): No, or a press outside it, ends Repair mode (0x461D0 then 0x8D300). */
  function declineConfirm() {
    const kind = confirm?.kind;
    closeConfirm();
    if (kind === 'repair') ui.endRepair?.();
    else if (kind === 'refusal') ui.dismissRefusal?.();   // 0x53E40's outside-click is the box's own Continue handler (0x4AAF0)
  }
  /** D2Client 0x4B5D0 above 6: the reply's message in red over a selectable Continue, over the NPC. */
  function openRefusal(v) {
    confirm = { kind: 'refusal', guid: v.guid };
    buyConfirm?.show(v.guid, 0, [text(v.refusal) || String(v.refusal)], [{ stringId: CONTINUE_STRING, action: 'continue' }], 1);
  }
  function confirmYes() {
    if (!confirm) return false;
    const { guid, kind, send } = confirm;
    if (kind === 'refusal') { declineConfirm(); return true; }
    closeConfirm();
    if (kind === 'repair') { send(); return true; }   // Yes sends the 0x35; the success reply re-arms Repair mode
    if (ui.buyAt(guid)) rule.sent();
    return true;
  }

  return {
    /** Draw whatever the NPC's current mode shows. Returns the speech box's verdict or null. */
    draw(ctx, w, h, { act = 1, pointer = null } = {}) {
      const v = current();
      if (!v || (v.mode !== 'trade' && v.mode !== 'gamble' && !(v.mode === 'hire' && v.refusal))) closeConfirm();
      if (!v || (v.mode !== 'trade' && v.mode !== 'gamble')) vendorState = null;
      if (!v) { syncMenu(null); vendorOpenFor = null; if (hireKey !== null) { menu.closeHire(); hireKey = null; } return null; }
      if (v.mode === 'speech') {
        syncMenu(null);   // The old Talk list must not remain clickable over its speech.
        const key = `${v.guid}:${v.speech?.stringId}:${v.speechInstance ?? ''}`;
        if (key !== gateKey) { gateKey = key; gate.begin(now()); }
        const line = v.speech ? text(v.speech.stringId) : '';
        if (!speechReady()) return null;
        const drawn = speech.draw(ctx, line, undefined, { messageInstance: key });
        // Use the shared speech clock's completion, just as the direct-engine
        // conversation does; otherwise the fully scrolled box stays empty forever.
        if (drawn?.done) { gateKey = null; ui.dismissSpeech(); }
        return drawn;
      }
      if (v.mode === 'hire') {
        syncMenu(null);
        vendorOpenFor = null;
        const key = `${v.guid}:${v.offers.length}`;
        if (key !== hireKey) { menu.showHire(v.guid, v.offers.map(menuOffer), gold()); hireKey = key; }
        menu.drawHire(ctx, act, w, h);
        if (v.refusal && !confirm) openRefusal(v);
        if (confirm && buyConfirm) { const feet = anchor(v.guid); if (feet) buyConfirm.draw(ctx, feet.x, feet.y, act); }
        return null;
      }
      if (hireKey !== null) { menu.closeHire(); hireKey = null; }
      if (v.mode === 'trade' || v.mode === 'gamble') {
        if (v.refusal && !confirm) openRefusal(v);
        if (vendorOpenFor !== v.guid) { vendor.reset(vendorItemsFrom(v.stock, itemTable), smith(v.classId)); vendorOpenFor = v.guid; }
        vendorState = { items: vendorItemsFrom(v.stock, itemTable, itemDetail), classId: v.classId, act, pointer, pressedButton, sellMode: v.sellMode, repairMode: v.repairMode, stashGold: stashGold() };
        const r = vendor.draw(ctx, w, h, vendorState);
        if (confirm && buyConfirm) { const feet = anchor(v.guid); if (feet) buyConfirm.draw(ctx, feet.x, feet.y, act); }
        return r;
      }
      vendorOpenFor = null;
      syncMenu(v);
      if (menu.open) {
        const feet = anchor(v.guid);
        if (feet) menu.draw(ctx, feet.x, feet.y, act);
      }
      return null;
    },
    /** The shop's item tooltip and button hint, drawn after the inventory panel as the local window's are; null with no shop up. */
    drawOverlay(ctx, w, h) { return vendorState ? vendor.drawOverlay(ctx, w, h, vendorState) : null; },
    /** A press at canvas point (x, y) with mouse button `button` (0 left, 2 right). Returns true when the screen took it. */
    pointerDown(x, y, w, h, button = 0) {
      const v = current();
      if (!v) return false;
      if (v.mode === 'speech') { if (gate.pointerDown(button, now()) === 'dismiss') { gateKey = null; ui.dismissSpeech(); } return true; }
      if (v.mode === 'hire') {
        if (confirm) {   // 0x4B420's refusal box takes the press first
          const c = buyConfirm?.entryAt(x, y);
          if (c?.action === 'continue') return confirmYes();
          if (buyConfirm?.outside(x, y)) declineConfirm();
          return true;
        }
        const hit = menu.hireAt(x, y, w, h);
        if (hit?.cancel) { ui.end(); return true; }
        if (hit?.row !== undefined) { const o = v.offers[hit.row]; if (o) ui.hire(o.mercName); }
        return true;   // the window takes every press over it
      }
      if (v.mode === 'trade' || v.mode === 'gamble') {
        if (confirm) {
          const hit = buyConfirm?.entryAt(x, y);
          if (hit?.action === 'yes' || hit?.action === 'continue') return confirmYes();
          if (hit?.action === 'no' || buyConfirm?.outside(x, y)) declineConfirm();
          return true;
        }
        if (vendor.contains(x, y, w, h)) {
          const tab = vendor.tabAt(x, y, w, h);
          if (tab != null) { vendor.setTab(tab); return true; }
          const vendorButton = vendor.buttonAt(x, y, w, h, v.classId);
          if (vendorButton) { pressedButton = vendorButton; return true; }
          const item = vendor.itemAt(x, y, w, h, { items: vendorItemsFrom(v.stock, itemTable), classId: v.classId });
          if (item) {
            const action = rule.action({ button, mode: 1, offer: true });
            if (action === 'confirm') openConfirm(v, item.guid);
            else if (action === 'buy') { if (ui.buyAt(item.guid)) rule.sent(); }
            else if (action === 'end') { closeConfirm(); ui.disarm(); }
          }
          return true;
        }
        return false;
      }
      if (menu.open) {
        if (menu.outside(x, y)) { ui.end(); return true; }
        menu.hover(x, y);
        pressedEntry = menu.selectedEntry;
        return true;
      }
      return false;
    },
    /** A release at canvas point (x, y): fires the menu row or vendor button under the press. */
    pointerUp(x, y, w, h) {
      const v = current();
      if (!v) return false;
      if (pressedButton) {
        const button = pressedButton; pressedButton = null;
        if (vendor.buttonAt(x, y, w, h, v.classId) === button) { ui.buttonRelease(button, v); return true; }
        return true;
      }
      if (menu.open && pressedEntry !== undefined) {
        menu.hover(x, y);
        const entry = npcMenuChoiceOnRelease({ entryAtRelease: menu.selectedEntry, selectedEntry: pressedEntry });
        pressedEntry = null;
        if (entry) { ui.choose(entry); return true; }
        return false;
      }
      return false;
    },
    /** Yes on the Buy box (Enter). Returns true when a confirm was open. */
    enter() { return confirmYes(); },
    /**
     * Esc closes the Buy box first, else the conversation (retail's cancel). During a speech Esc is the speech window's key
     * (D2Client 0x7E840): after the same 100 ms gate it dismisses the line, and the greeting's continuation (0x4B230) returns
     * to the menu with no 0x30 for a key.
     */
    escape() {
      if (confirm) { declineConfirm(); return true; }
      if (current()?.mode === 'speech') { if (gate.pointerDown(0, now()) !== 'consumed') { gateKey = null; ui.dismissSpeech(); } return true; }
      return ui.end();
    },
    /**
     * A repair click that needs repair opens the Repair box over the NPC: 'Repair', the item's name, 'Gold:', the cost, Yes / No.
     * Nothing is sent until Yes, which calls send(). False when the shop is not open or a box is already up.
     */
    askRepair(itemGuid, name, cost, send) {
      const v = current();
      if (!v || (v.mode !== 'trade' && v.mode !== 'gamble') || confirm) return false;
      confirm = { kind: 'repair', guid: itemGuid, send };
      buyConfirm?.show(v.guid, 0, [text(REPAIR_TITLE_STRING) || 'Repair', String(name ?? '').split('\n').filter(Boolean).at(-1) ?? '', text(GOLD_STRING) || 'Gold:', String(Math.trunc(cost) | 0)],
        [{ stringId: YES_STRING, action: 'yes' }, { stringId: NO_STRING, action: 'no' }]);
      return true;
    },
    get confirmOpen() { return confirm !== null; },
    /** Called each frame: an NPC that left the scene ends its conversation. */
    checkScene(sceneMonsters) {
      const v = current();
      if (!v) return false;
      const gone = ui.walkedAway(v.guid, sceneMonsters);
      if (gone) closeConfirm();
      return gone;
    },
    /** Reset every per-conversation latch (the shell calls this when a conversation ends or the scene changes). */
    reset() {
      if (menu.open) menu.close();
      if (hireKey !== null) { menu.closeHire(); hireKey = null; }
      closeConfirm();
      shown = null; vendorOpenFor = null; pressedEntry = null; pressedButton = null;
    },
  };
}
