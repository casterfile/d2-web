import test from 'node:test';
import assert from 'node:assert/strict';
import { createRealmNpcScreen } from './realm-npc-screen.mjs';

const ROW_A = { action: 'talk', stringId: 1 }, ROW_B = { action: 'trade', stringId: 2 };

const clock = { t: 0 };
function fakes(viewValue, { buyConfirm = null, offerClick = null, gold = () => null, price = () => 450, speechReady = () => true, speechResult = () => true } = {}) {
  const calls = [];
  let v = viewValue;
  const menu = {
    open: false, selectedRow: null, outsideNow: false, hireOpen: false,
    show(guid, classId, name, entries) { this.open = true; this.entries = entries; calls.push(['menu.show', guid]); },
    close() { this.open = false; calls.push(['menu.close']); },
    draw(_ctx, fx, fy) { calls.push(['menu.draw', fx, fy]); return {}; },
    hover(x) { this.selectedRow = x === 10 ? ROW_A : x === 20 ? ROW_B : null; },
    get selectedEntry() { return this.selectedRow; },
    outside(x) { return this.outsideNow || x === 999; },
    showHire(guid, offers, g) { this.hireOpen = true; calls.push(['hire.show', offers.length, g, offers[0]?.level]); },
    closeHire() { this.hireOpen = false; calls.push(['hire.close']); },
    drawHire() { calls.push(['hire.draw']); return {}; },
    hireAt: (x) => (x === 50 ? { row: 0 } : x === 60 ? { cancel: true } : null),
  };
  const vendor = {
    reset(items, smith) { calls.push(['vendor.reset', items.length, smith]); },
    draw(_ctx, w, h, state) { calls.push(['vendor.draw', state.sellMode, state.repairMode]); return {}; },
    contains: (x) => x < 500, tabAt: (x) => (x === 5 ? 1 : null), setTab(t) { calls.push(['vendor.setTab', t]); },
    buttonAt: (x) => (x === 100 ? 'sell' : x === 101 ? 'repairAll' : null),
    itemAt: (x) => (x === 200 ? { guid: 0x77 } : null),
  };
  const speech = { draw(_ctx, text, _box, options) { calls.push(['speech.draw', text]); return speechResult(options); } };
  const ui = {
    choose: (e) => { calls.push(['choose', e.action]); return true; },
    dismissSpeech: () => { calls.push(['dismiss']); return true; },
    end: () => { calls.push(['end']); return true; },
    buyAt: (g) => { calls.push(['buy', g]); return true; },
    priceAt: (g) => price(g),
    hire: (m) => { calls.push(['hire', m]); return true; },
    disarm: () => { calls.push(['disarm']); return true; },
    endRepair: () => { calls.push(['endRepair']); return true; },
    dismissRefusal: () => { calls.push(['dismissRefusal']); return true; },
    buttonRelease: (b) => { calls.push(['release', b]); return true; },
    walkedAway: (g, monsters) => { calls.push(['walked', g, monsters.length]); return false; },
  };
  const screen = createRealmNpcScreen({ ui, view: () => v, menu, buyConfirm, vendor, speech, itemTable: {}, text: (id) => `line${id}`, now: () => clock.t, nameOf: () => 'Akara',
    anchor: () => ({ x: 300, y: 400 }), gold: () => gold(), offerClick, speechReady });
  return { screen, calls, menu, setView: (nv) => { v = nv; } };
}

const menuView = { guid: 0x55, classId: 154, mode: 'menu', entries: [ROW_A, ROW_B], stock: [], speech: null, sellMode: false, repairMode: false };

test('no NPC open: nothing is drawn and presses are not taken', () => {
  const { screen, calls } = fakes(null);
  assert.equal(screen.draw({}, 800, 600), null);
  assert.equal(screen.pointerDown(10, 10, 800, 600), false);
  assert.deepEqual(calls, []);
});

test('speech: the first 100 ms of a line swallow the press, then it dismisses', () => {
  clock.t = 1000;
  const { screen, calls } = fakes({ ...menuView, mode: 'speech', speech: { stringId: 7 } });
  screen.draw({}, 800, 600);
  assert.deepEqual(calls.at(-1), ['speech.draw', 'line7']);
  clock.t = 1099;
  assert.equal(screen.pointerDown(10, 10, 800, 600), true);
  assert.deepEqual(calls.at(-1), ['speech.draw', 'line7']);
  clock.t = 1100;
  assert.equal(screen.pointerDown(10, 10, 800, 600, 2), true);
  assert.deepEqual(calls.at(-1), ['dismiss']);
});

test('speech: Esc is the speech key (same 100 ms gate) and dismisses the line, never ending the conversation', () => {
  clock.t = 2000;
  const { screen, calls } = fakes({ ...menuView, mode: 'speech', speech: { stringId: 7 } });
  screen.draw({}, 800, 600);
  clock.t = 2099;
  assert.equal(screen.escape(), true);
  assert.deepEqual(calls.at(-1), ['speech.draw', 'line7']);
  clock.t = 2100;
  assert.equal(screen.escape(), true);
  assert.deepEqual(calls.at(-1), ['dismiss']);
  assert.equal(calls.some((c) => c[0] === 'end'), false);
});

test('menu: the menu opens over the NPC feet and a press on a row fires it on the matching release', () => {
  const { screen, calls } = fakes(menuView);
  screen.draw({}, 800, 600);
  assert.deepEqual(calls[0], ['menu.show', 0x55]);
  assert.deepEqual(calls.at(-1), ['menu.draw', 300, 400]);
  screen.pointerDown(10, 10, 800, 600);
  assert.equal(screen.pointerUp(10, 10, 800, 600), true);
  assert.deepEqual(calls.at(-1), ['choose', 'talk']);
});

test('menu: a press on a row released elsewhere fires nothing', () => {
  const { screen, calls } = fakes(menuView);
  screen.draw({}, 800, 600);
  screen.pointerDown(10, 10, 800, 600);
  assert.equal(screen.pointerUp(20, 10, 800, 600), false);
  assert.equal(calls.filter((c) => c[0] === 'choose').length, 0);
});

test('menu: a press outside the menu box ends the conversation', () => {
  const { screen, calls } = fakes(menuView);
  screen.draw({}, 800, 600);
  assert.equal(screen.pointerDown(999, 10, 800, 600), true);
  assert.deepEqual(calls.at(-1), ['end']);
});

test('Esc ends the conversation', () => {
  const { screen, calls } = fakes(menuView);
  assert.equal(screen.escape(), true);
  assert.deepEqual(calls.at(-1), ['end']);
});

test('shop: the vendor window is reset once per NPC; a left press on an offer only opens the Buy box', () => {
  const shop = { ...menuView, mode: 'trade', stock: [{ guid: 0x77, bits: new Uint8Array(0), tab: 3, x: 0, y: 0 }] };
  const { screen, calls } = fakes(shop);
  screen.draw({}, 800, 600);
  screen.draw({}, 800, 600);
  assert.equal(calls.filter((c) => c[0] === 'vendor.reset').length, 1);
  assert.equal(screen.pointerDown(200, 10, 800, 600), true);
  assert.equal(calls.some((c) => c[0] === 'buy'), false);   // retail: the left press sends nothing
});

test('shop: a right press buys the offer at once, and the 500 ms gate follows a trade send', () => {
  clock.t = 2000;
  const { screen, calls } = fakes({ ...menuView, mode: 'trade', stock: [{ guid: 0x77, bits: new Uint8Array(0), tab: 3, x: 0, y: 0 }] });
  screen.draw({}, 800, 600);
  assert.equal(screen.pointerDown(200, 10, 800, 600, 2), true);
  assert.deepEqual(calls.at(-1), ['buy', 0x77]);
  clock.t = 2300;   // the buy just sent: a second right press inside 500 ms is refused
  screen.pointerDown(200, 10, 800, 600, 2);
  assert.equal(calls.filter((c) => c[0] === 'buy').length, 1);
  clock.t = 2600;
  screen.pointerDown(200, 10, 800, 600, 2);
  assert.equal(calls.filter((c) => c[0] === 'buy').length, 2);
});

const shopView = { ...menuView, mode: 'trade', stock: [{ guid: 0x77, bits: new Uint8Array(0), tab: 3, x: 0, y: 0 }] };
function buyBox() {
  const box = { open: false, heads: null,
    show(_g, _c, heads, entries, colour) { this.open = true; this.heads = heads; this.entries = entries; this.colour = colour; },
    close() { this.open = false; },
    draw() {},
    entryAt: (x) => (x === 320 ? { action: 'continue' } : x === 300 ? { action: 'yes' } : x === 310 ? { action: 'no' } : null),
    outside: (x) => x === 999 };
  return box;
}

test('Buy box: a left press opens it with the price, Yes buys the armed offer, No drops it', () => {
  clock.t = 5000;
  const box = buyBox();
  const { screen, calls } = fakes(shopView, { buyConfirm: box, price: () => 450 });
  screen.draw({}, 800, 600);
  assert.equal(screen.pointerDown(200, 10, 800, 600, 0), true);
  assert.equal(box.open, true);
  assert.deepEqual(box.heads, ['line3348', '', 'line3346', '450']);   // 'Buy', the name, 'Gold:', the price
  assert.equal(calls.some((c) => c[0] === 'buy'), false);
  assert.equal(screen.pointerDown(300, 10, 800, 600, 0), true);        // Yes
  assert.deepEqual(calls.at(-1), ['buy', 0x77]);
  assert.equal(box.open, false);
  screen.pointerDown(200, 10, 800, 600, 0);
  screen.pointerDown(310, 10, 800, 600, 0);                           // No
  assert.equal(box.open, false);
  assert.equal(calls.filter((c) => c[0] === 'buy').length, 1);
});

test('Repair box: a repair click opens it over the NPC, sends only on Yes, and No or Esc ends Repair mode', () => {
  const box = buyBox();
  const { screen, calls } = fakes(shopView, { buyConfirm: box });
  const sent = [];
  screen.draw({}, 800, 600);
  assert.equal(screen.askRepair(0x31, 'Ring\nBone Wand', 87, () => sent.push(0x31)), true);
  assert.deepEqual(box.heads, ['line3351', 'Bone Wand', 'line3346', '87']);   // the bottom line of the name, 0x47500
  assert.deepEqual(sent, []);
  assert.equal(screen.askRepair(0x32, 'x', 1, () => sent.push(0x32)), false);   // one box at a time
  assert.equal(screen.pointerDown(300, 10, 800, 600, 0), true);                // Yes
  assert.deepEqual(sent, [0x31]); assert.equal(box.open, false); assert.equal(calls.some((c) => c[0] === 'endRepair'), false);
  screen.askRepair(0x31, 'a', 5, () => sent.push(1));
  screen.pointerDown(310, 10, 800, 600, 0);                                     // No
  assert.deepEqual(calls.filter((c) => c[0] === 'endRepair').length, 1); assert.deepEqual(sent, [0x31]);
  screen.askRepair(0x31, 'a', 5, () => sent.push(1)); screen.escape();
  assert.equal(calls.filter((c) => c[0] === 'endRepair').length, 2); assert.equal(calls.some((c) => c[0] === 'end'), false);
  screen.askRepair(0x31, 'a', 5, () => sent.push(1)); screen.pointerDown(999, 10, 800, 600, 0);   // outside
  assert.equal(calls.filter((c) => c[0] === 'endRepair').length, 3);
});

test('refusal box: a reply above 6 opens the red message over Continue, Continue and an outside press both dismiss it', () => {
  const box = buyBox();
  const { screen, calls, setView } = fakes({ ...shopView, refusal: 3362 }, { buyConfirm: box });
  screen.draw({}, 800, 600);
  assert.equal(box.open, true); assert.deepEqual(box.heads, ['line3362']); assert.equal(box.colour, 1);
  assert.deepEqual(box.entries, [{ stringId: 3403, action: 'continue' }]);
  assert.equal(screen.pointerDown(320, 10, 800, 600, 0), true);
  assert.equal(box.open, false); assert.equal(calls.filter((c) => c[0] === 'dismissRefusal').length, 1);
  setView({ ...shopView, refusal: 0 }); screen.draw({}, 800, 600); assert.equal(box.open, false);
  setView({ ...shopView, refusal: 3361 }); screen.draw({}, 800, 600);
  screen.pointerDown(999, 10, 800, 600, 0);
  assert.equal(calls.filter((c) => c[0] === 'dismissRefusal').length, 2);
  setView({ ...shopView, refusal: 3361 }); screen.draw({}, 800, 600); screen.escape();
  assert.equal(calls.filter((c) => c[0] === 'dismissRefusal').length, 3);
});

test('Repair box: no open shop, no box', () => {
  const { screen } = fakes(menuView, { buyConfirm: buyBox() });
  assert.equal(screen.askRepair(1, 'a', 1, () => {}), false);
});

test('Buy box: an unknown price is drawn as ?, and Esc drops the box without ending the conversation', () => {
  clock.t = 6000;
  const box = buyBox();
  const { screen, calls } = fakes(shopView, { buyConfirm: box, price: () => null });
  screen.draw({}, 800, 600);
  screen.pointerDown(200, 10, 800, 600, 0);
  assert.equal(box.heads[3], '?');
  assert.equal(screen.escape(), true);
  assert.equal(box.open, false);
  assert.equal(calls.some((c) => c[0] === 'end'), false);
  assert.equal(screen.escape(), true);   // no box: Esc ends the conversation
  assert.equal(calls.at(-1)[0], 'end');
});

test('the offer rule decides a press: a right press buys and stamps the rule; while the box is open a press stays in it', () => {
  clock.t = 7000;
  const stamps = [];
  const rule = { sent() { stamps.push('sent'); }, action: ({ button }) => (button === 0 ? 'confirm' : button === 2 ? 'buy' : 'end') };
  const box = buyBox();
  const { screen, calls } = fakes(shopView, { buyConfirm: box, offerClick: rule });
  screen.draw({}, 800, 600);
  screen.pointerDown(200, 10, 800, 600, 2);            // the rule says buy: it sends and stamps
  assert.deepEqual(calls.at(-1), ['buy', 0x77]);
  assert.deepEqual(stamps, ['sent']);
  screen.pointerDown(200, 10, 800, 600, 0);            // the rule says confirm: the box opens
  assert.equal(box.open, true);
  const before = calls.length;
  screen.pointerDown(200, 10, 800, 600, 2);            // a press in the box, outside Yes and No, buys nothing
  assert.equal(calls.slice(before).some((c) => c[0] === 'buy'), false);
});

test('hire: the window lists the offers the state holds and a row hire names its merc; Cancel ends', () => {
  clock.t = 8000;
  const hire = { ...menuView, mode: 'hire', offers: [{ mercName: 5, name: 'Ahmad', level: null, life: 20, def: 7, cost: 300, skill: '' }] };
  const { screen, calls } = fakes(hire, { gold: () => 88 });
  screen.draw({}, 800, 600);
  assert.deepEqual(calls.find((c) => c[0] === 'hire.show'), ['hire.show', 1, 88, '?']);   // an unknown level is '?', not 0
  assert.deepEqual(calls.at(-1), ['hire.draw']);
  assert.equal(screen.pointerDown(50, 10, 800, 600, 0), true);
  assert.deepEqual(calls.at(-1), ['hire', 5]);
  assert.equal(screen.pointerDown(60, 10, 800, 600, 0), true);
  assert.deepEqual(calls.at(-1), ['end']);
});

test('shop: a tab press switches the tab; a button is acted on only when released on it', () => {
  const { screen, calls } = fakes({ ...menuView, mode: 'trade', stock: [] });
  screen.draw({}, 800, 600);
  screen.pointerDown(5, 10, 800, 600);
  assert.deepEqual(calls.at(-1), ['vendor.setTab', 1]);
  screen.pointerDown(100, 10, 800, 600);
  screen.pointerUp(101, 10, 800, 600);   // released on another button: nothing
  assert.equal(calls.filter((c) => c[0] === 'release').length, 0);
  screen.pointerDown(100, 10, 800, 600);
  screen.pointerUp(100, 10, 800, 600);
  assert.deepEqual(calls.at(-1), ['release', 'sell']);
});

test('shop: sell and repair modes come from the view and are drawn with it', () => {
  const { screen, calls } = fakes({ ...menuView, mode: 'trade', stock: [], sellMode: true, repairMode: false });
  screen.draw({}, 800, 600);
  assert.deepEqual(calls.at(-1), ['vendor.draw', true, false]);
});

test('request mode (waiting for the stock) draws no menu and no vendor window', () => {
  const { screen, calls } = fakes({ ...menuView, mode: 'request' });
  screen.draw({}, 800, 600);
  assert.equal(calls.filter((c) => c[0].startsWith('vendor') || c[0].startsWith('menu')).length, 0);
});

test('walking away: an NPC that left the scene ends its conversation through the controller', () => {
  const { screen, calls } = fakes(menuView);
  screen.checkScene([{ guid: 1 }, { guid: 2 }]);
  assert.deepEqual(calls.at(-1), ['walked', 0x55, 2]);
});

test('hire: a refusal opens the red box over Continue and Continue dismisses it without hiring (D2Client 0x4B420)', () => {
  clock.t = 9000;
  const box = buyBox();
  const hire = { ...menuView, mode: 'hire', refusal: 3358, offers: [{ mercName: 5, name: 'Ahmad', level: null, life: 20, def: 7, cost: 300, skill: '' }] };
  const { screen, calls } = fakes(hire, { buyConfirm: box });
  screen.draw({}, 800, 600);
  assert.equal(box.open, true); assert.deepEqual(box.entries, [{ stringId: 3403, action: 'continue' }]);
  assert.equal(screen.pointerDown(50, 10, 800, 600, 0), true);   // a press on an offer row hires nothing under the box
  assert.equal(calls.some((c) => c[0] === 'hire'), false);
  assert.equal(screen.pointerDown(320, 10, 800, 600, 0), true);
  assert.equal(calls.filter((c) => c[0] === 'dismissRefusal').length, 1);
});

// The visible speech lifecycle must complete, not leave a blank scrolled box.
test('speech waits for voice readiness, hides the Talk menu and completes once', () => {
  let ready = false;
  const f = fakes(menuView, { speechReady: () => ready, speechResult: () => ({ done: true }) });
  f.screen.draw({}, 800, 600); assert.equal(f.menu.open, true);
  f.setView({ ...menuView, mode: 'speech', speech: { stringId: 7 }, speechInstance: 1 });
  assert.equal(f.screen.draw({}, 800, 600), null);
  assert.equal(f.menu.open, false);
  assert.equal(f.calls.some(c => c[0] === 'speech.draw'), false);
  ready = true; f.screen.draw({}, 800, 600);
  assert.equal(f.calls.filter(c => c[0] === 'dismiss').length, 1);
  f.setView(menuView); f.screen.draw({}, 800, 600);
  assert.equal(f.calls.filter(c => c[0] === 'dismiss').length, 1);
});
test('replaying the same speech ID receives a fresh text-clock identity', () => {
  const speech = { ...menuView, mode: 'speech', speech: { stringId: 0 }, speechInstance: 1 };
  const f = fakes(speech, { speechResult: options => ({ key: options.messageInstance }) });
  const first = f.screen.draw({}, 800, 600).key;
  f.setView({ ...speech, speechInstance: 2 });
  const second = f.screen.draw({}, 800, 600).key;
  assert.notEqual(first, second);
});
