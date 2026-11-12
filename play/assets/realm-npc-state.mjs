// The client side of a town NPC in a Battle.net game: the conversation the server drives (the NPC's message
// list, the shop stock, the transaction results, the hire offers) and the packets the player's choices send. It
// owns no drawing: the shell reads `view()` for the menu, the speech, the shop and the hire window, and calls the
// methods below on clicks. The menu rows are the retail table (npc-menu.mjs); the flow is
//   talk click, 0x13, S2C 0x27 (the lines), 0x2F (open), menu or speech, then an entry's request
//   (0x38 trade, gamble or hire list; 0x34 identify; 0x62 resurrect), answered by S2C 0x9C stock and 0x2A result.
// Hire: retail's menu build sends 0x38 {hire} (13 bytes: 0x38, action u32, NPC guid, the hero's unit id) when the
// conversation opens, ahead of any speech, for the four mercenary hirers (not when the player picks Hire); the server answers 0x4F then one 0x4E per offer, which the hire window lists.
import { npcMenuEntries, talkEntries, entriesAfterSpeech, withTravelRow, withHireRow, withResurrectRow, TRAVEL_LEVEL } from './npc-menu.mjs';
import { npcPackets, NPC_ACTION, SELL_MODE, parseNpcInfo, parseTransaction, parseShopItem, parseHireOffer, HIRE_LIST_HEAD } from './realm-npc-packets.mjs';
import { talkSpeechId } from './npc-topics.mjs';
import { decodeItemHeader } from './realm-world.mjs';
import { packets, UNIT } from './realm-input.mjs';
import { refusalString, hireRefusalString } from './vendor-repair-confirm.mjs';

const MENU_SPEECH = 0;   // messages with this menu number are spoken at once; menu 2 are the Talk box's quest topics
/** The hirers whose menu build sends the hire list request (0x48B20: classes 0x96, 0xC6, 0xFC, 0x203). */
export const HIRE_LIST_CLASSES = Object.freeze([150, 198, 252, 515]);

/** The 0x2F that opens the conversation with an NPC and the 0x30 that ends it. */
export const chatStart = (guid) => packets.npcChatStart(UNIT.MONSTER, guid);
export const chatEnd = (guid) => packets.npcChatEnd(UNIT.MONSTER, guid);

/**
 * deps: send(bytes) puts a packet on the wire; classOf(guid) is the NPC's class id; questDone(act, quest) and
 * questName(act, n) answer from the hero's quests; resurrectLabel() is the Resurrect row's text while the server's 0x9B offers one, else null;
 * transactionCost(item, kind) is the displayed price, kind 'buy' | 'sell' | 'repair', or null while no engine
 * seam supplies it (the server charges its own price); hireStats(mercName, seed) is the offer's
 * { name, level, life, def, cost, skill } or null (the engine seam is MONSTERS_HirelingInit); playerGuid() is
 * the hero's unit id for the hire form of the 0x38 request (trade and gamble carry 0); questBit(word) is bit 0 of
 * the hero's quest word and expansion() the game's expansion flag (the travel rows' conditions).
 */
export function createRealmNpc({ send, classOf = () => -1, questDone = () => false, questName = () => '', speechOf = () => null,
  resurrectLabel = () => null, transactionCost = () => null, hireStats = () => null, playerGuid = () => 0xffffffff,
  questBit = () => false, expansion = () => true } = {}) {
  let speechSerial = 0;
  const introHeard = new Map();
  let npc = null;            // { guid, classId, messages, entries, speech, spoken, mode, offers }
  let stock = new Map();     // item guid -> { guid, bits, tab, x, y }
  let result = null;         // the last transaction outcome
  // D2Client 0xBC9663 == 5: a buy, sell or repair (0x46EE0's 0x32 / 0x33 / 0x35) is waiting for its 0x2A. 0x4BC80 hands the reply to
  // 0x4B5D0 only then; with nothing waiting a reply above 6 opens no box.
  let tradePending = false;
  let hirePending = false;   // D2Client 0xBC9663 == 10: 0x46C00 sent the 0x36 and its 0x2A is due
  let refusal = 0;           // the string id of the open refusal box (0x4B5D0's red message over Continue), 0 when none
  let sellMode = false, repairMode = false;

  function close(notify) {
    hirePending = false;
    if (npc && notify) send(chatEnd(npc.guid));
    npc = null; stock = new Map(); sellMode = false; repairMode = false; tradePending = false; refusal = 0;
  }

  function menuFor(classId) {
    const entries = npcMenuEntries(classId);
    return entries ? withResurrectRow(withHireRow(withTravelRow(entries, classId, questBit, expansion()), classId, questDone), classId, resurrectLabel()) : null;
  }

  /** The menu is shown; its hire request went out when the conversation opened. */
  function openMenu() { npc.mode = 'menu'; }

  // D2Client 0x7DE50 (the 0x27 handler's body), unit type 2: a quest object speaks through the scroll box (0x7D200) with the
  // string id at +10; no conversation opens and nothing is sent. A line whose menu byte (+8) is 3 takes the other branch (0x7C8F0).
  let objectLine = null;
  function onObjectInfo(info) {
    const m = info.messages[0];
    if (m && m.menu !== 3) objectLine = { guid: info.guid, stringId: m.stringId };
  }

  function onInfo(info) {
    if (info.unitType === 2) { onObjectInfo(info); return; }
    const classId = classOf(info.guid);
    const messages = [...info.messages].sort((a, b) => a.stringId - b.stringId);
    const speech = messages.filter((m) => m.menu === MENU_SPEECH);
    const entries = menuFor(classId);
    if (!entries && !speech.length) return;   // neither a menu nor a line: nothing to open
    if (speech[0]) introHeard.set(classId, speech[0].stringId);
    npc = { guid: info.guid, classId, messages, entries, speech, spoken: 0, mode: 'speech', offers: [], speechInstance: ++speechSerial };
    stock = new Map();
    send(chatStart(info.guid));
    // Retail's conversation open builds the menu at once (before any line is played), and the build asks the
    // four hirers for their offers, so the request leaves here, ahead of the speech.
    if (entries && HIRE_LIST_CLASSES.includes(classId)) send(npcPackets.request(NPC_ACTION.HIRE, info.guid, playerGuid()));
    if (!speech.length) openMenu(); else npc.mode = 'speech';
  }

  function onShop(item) {
    if (!npc) return;
    if (!item.add) { stock.delete(item.guid); return; }
    const header = decodeItemHeader(item.bits);
    if (!header) return;
    stock.set(item.guid, { guid: item.guid, bits: item.bits, tab: header.page, mode: header.mode, x: header.x, y: header.y });
  }

  /** One hire offer: the seam's stats, or nulls where the seam knows none (shown as unknown, never a made-up number). */
  function onOffer(offer) {
    const s = hireStats(offer.mercName, offer.seed) || {};
    npc.offers.push({ mercName: offer.mercName, seed: offer.seed, name: s.name ?? '', level: s.level ?? null, life: s.life ?? null,
      def: s.def ?? null, cost: s.cost ?? null, skill: s.skill ?? '' });
  }

  const api = {
    /** The player clicked another town NPC: an open conversation with a different one ends. */
    talkSent(guid) { if (npc && npc.guid !== guid) close(true); },
    /** Apply one complete server packet. */
    handle(p) {
      switch (p[0]) {
        case 0x27: { const info = parseNpcInfo(p); if (info) onInfo(info); break; }
        case 0x2a: {
          const t = parseTransaction(p);
          if (!t) break;
          result = { ...t, refusal: refusalString(t.result) };
          if (npc?.mode === 'hire' && hirePending) {
            // D2Client 0x4B420 (0xBC9663 == 10): 3, 5, 6 and the success reply 0 end the conversation; 1, 2, 4 and above 6 raise the refusal box.
            hirePending = false;
            const h = t.result === 0 ? 0 : hireRefusalString(t.result);
            if (h) refusal = h; else close(true);
            break;
          }
          if (npc?.mode === 'hire' && t.result <= 6) close(true);
          if (tradePending) { tradePending = false; if (npc && result.refusal > 0) refusal = result.refusal; }   // 0x4B5D0: above 6 the box; else back to idle (0xBC9663 = 3)
          break;
        }
        case 0x9c: { const item = parseShopItem(p); if (item) onShop(item); break; }
        case HIRE_LIST_HEAD: if (npc) npc.offers = []; break;
        case 0x4e: { const offer = parseHireOffer(p); if (offer && npc) onOffer(offer); break; }
        default: break;
      }
    },
    feed(bytes, sizeOf) {
      for (let at = 0; at < bytes.length;) {
        const n = sizeOf(bytes.subarray(at));
        if (!(n > 0) || at + n > bytes.length) break;
        api.handle(bytes.subarray(at, at + n)); at += n;
      }
    },
    /** The object's line the last 0x27 delivered, once: { guid, stringId } or null. */
    takeObjectSpeech() { const l = objectLine; objectLine = null; return l; },
    /** The scroll's lifecycle ended (0x7D200 then 0x2CEF0): the server is told which line it was, with the guid the caller names. */
    objectSpeechDone(guid, stringId) { send(npcPackets.messageDone(guid >>> 0, stringId)); return true; },
    /** What the shell draws: null when no NPC is open. */
    view() {
      if (!npc) return null;
      return { guid: npc.guid, classId: npc.classId, mode: npc.mode, entries: npc.entries, speech: npc.localSpeech ?? npc.speech[npc.spoken] ?? null, speechInstance: npc.speechInstance,
        stock: [...stock.values()], offers: npc.offers, result, sellMode, repairMode, refusal };
    },
    isOpen: () => npc !== null,
    shopOpen: () => npc !== null && (npc.mode === 'trade' || npc.mode === 'gamble'),
    /** The player dismissed the current speech: the server is told which line it was (0x31), then the menu returns. */
    dismissSpeech() {
      if (!npc || npc.mode !== 'speech') return false;
      // Talk's own lines return to the Talk list. Only an offered quest topic
      // sends completion, and only after the player actually heard/dismissed it.
      if (npc.localSpeech) {
        const state = npc, line = state.localSpeech, back = state.speechReturn;
        state.localSpeech = null; state.speechReturn = null;
        state.mode = back.mode;
        state.entries = line.acknowledge ? entriesAfterSpeech(back.entries, line.stringId) : back.entries;
        if (line.acknowledge) {
          state.messages = state.messages.filter(m => m.stringId !== line.stringId);
          send(npcPackets.messageDone(state.guid, line.stringId));
        }
        return true;
      }
      const line = npc.speech[npc.spoken];
      if (line) send(npcPackets.messageDone(npc.guid, line.stringId));
      npc.spoken++; npc.speechInstance = ++speechSerial;
      if (npc.spoken >= npc.speech.length) {
        if (npc.entries) openMenu(); else close(true);
      }
      return true;
    },
    /** Pick a menu row (an entry of `view().entries`). */
    choose(entry) {
      if (!npc || (npc.mode !== 'menu' && npc.mode !== 'talk')) return false;
      switch (entry.action) {
        case 'talk': npc.entries = talkEntries(npc.messages, questName); npc.mode = 'talk'; return true;
        case 'topic': case 'intro': case 'gossip': {
          // Reuse the existing single-player topic selection rules, including
          // introduction ID zero and the actual class-specific greeting heard.
          const id = talkSpeechId(entry.action, { stringId: entry.stringId,
            plain: npc.messages.find(m => m.menu === MENU_SPEECH)?.stringId,
            said: introHeard.get(npc.classId), sp: speechOf(npc.classId) });
          if (id == null) { close(true); return true; }
          npc.speechReturn = { mode: npc.mode, entries: npc.entries };
          npc.localSpeech = { stringId: id, acknowledge: entry.action === 'topic' };
          npc.speechInstance = ++speechSerial; npc.mode = 'speech';
          return true;
        }
        case 'trade': npc.mode = 'trade'; send(npcPackets.request(NPC_ACTION.TRADE, npc.guid, 0)); return true;
        case 'gamble': npc.mode = 'gamble'; send(npcPackets.request(NPC_ACTION.GAMBLE, npc.guid, 0)); return true;
        // The hire window lists the offers received at menu build (0x38 sent then): choosing Hire sends nothing.
        case 'hire':
          npc.mode = 'hire'; return true;
        // The Resurrect row (D2Client+0x46D10) sends 0x62 {npc id}; retail then waits on a dialog for the answer (not ported here).
        case 'resurrect': send(npcPackets.resurrectMerc(npc.guid)); close(true); return true;
        case 'identify': send(npcPackets.identifyAll(npc.guid)); close(true); return true;
        case 'cancel': close(true); return true;
        // The travel rows (Warriv / Meshif 3382..3385, Tyrael 22738): retail's handlers end the chat first (0x48150 sends the
        // 0x30) and then send 0x38 {0, npc, destination level}; the server moves the hero to that act's town.
        case 'other': {
          const level = TRAVEL_LEVEL[entry.stringId];
          if (level === undefined) return false;
          const guid = npc.guid;
          close(true); send(npcPackets.request(0, guid, level)); return true;
        }
        default: return false;   // the other rows are the shell's own
      }
    },
    /** Esc, walking away, or the exit button. */
    end() { if (!npc) return false; close(true); return true; },
    /** The displayed price of a stocked item, or null while no seam supplies it. */
    priceOf(itemGuid) {
      const item = stock.get(itemGuid);
      if (!item) return null;
      const cost = transactionCost(item, 'buy');
      return Number.isFinite(cost) ? cost : null;
    },
    buy(itemGuid, multi = false) {
      const item = stock.get(itemGuid);
      if (!item || !api.shopOpen()) return false;
      // With no price seam (unwired test or no player) the packet's cost field is 0.
      const cost = api.priceOf(itemGuid) ?? 0;
      send(npcPackets.buy(npc.guid, itemGuid, item.mode ?? 0, cost, { gamble: npc.mode === 'gamble', multi })); tradePending = true; return true;
    },
    /** Sell a carried item: `info` is { mode, cost } read from the item (its own mode, the displayed price). */
    sell(itemGuid, mode = SELL_MODE.STORED, cost = 0) {
      if (!api.shopOpen()) return false;
      send(npcPackets.sell(npc.guid, itemGuid, mode, cost)); tradePending = true; return true;
    },
    /** Repair one carried item: its mode and durability stat go in the packet. */
    repair(itemGuid, mode = 0, durability = 0) {
      if (!api.shopOpen()) return false; send(npcPackets.repair(npc.guid, itemGuid, mode, durability)); tradePending = true; return true;
    },
    repairAll() { if (!api.shopOpen()) return false; send(npcPackets.repairAll(npc.guid)); tradePending = true; return true; },
    hire(hirelingId) {
      if (!npc || npc.mode !== 'hire') return false;
      send(npcPackets.hire(npc.guid, hirelingId)); hirePending = true; return true;   // the conversation ends on the 0x2A result
    },
    /** A gambled item is bought unidentified; this identifies it. */
    identifyGambled(itemGuid) { send(npcPackets.identifyGambled(itemGuid)); return true; },
    setSellMode(on) { sellMode = Boolean(on); if (on) repairMode = false; },
    setRepairMode(on) { repairMode = Boolean(on); if (on) sellMode = false; },
    /** Continue on the refusal box (D2Client 0x4AAF0 then 0x8F630 / 0x8D300): the box closes and the cursor drops to mode 1, ending sell and repair mode. */
    dismissRefusal() {
      if (!refusal) return false;
      refusal = 0; sellMode = false; repairMode = false;
      if (npc?.mode === 'hire') close(true);   // the hire box's Continue is 0x4AAF0 -> 0x47B20: the conversation ends (0x30)
      return true;
    },
    takeResult() { const r = result; result = null; return r; },
  };
  return api;
}
