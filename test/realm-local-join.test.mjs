// The single-player local join, end to end in node: the real engine creates a game, the in-process game server
// answers the logon, and the client join reads the player assignment and the handshake inside its scan limits.
import test from 'node:test';
import assert from 'node:assert/strict';
import { bindEngineProtocol, d2gsJoin } from '../play/assets/realm-client.mjs';
import { openLocalRealm } from '../play/assets/realm-local.mjs';
import { haveArchives, haveEngine, loadEngine } from './helpers/node-engine.mjs';

const ready = haveEngine() && haveArchives();

test('the local single-player join completes', { skip: ready ? false : 'needs the built engine and archives', timeout: 240000 }, async () => {
  const Module = await loadEngine({ archives: true });
  const call = (name, types, values) => Module.ccall(name, 'number', types, values);
  call('d2_session_create_stream', ['number'], [0]);
  Module.ccall('d2_session_join_units', null, ['number'], [1]);
  Module.ccall('d2_session_join_reveal', null, ['number'], [2]);
  assert.ok(call('d2_session_create_class', ['number', 'number', 'number', 'number', 'number'], [1, 0, 0, 1, 0]), 'session created');
  call('d2_session_client_map_start', [], []);
  const realm = openLocalRealm(Module, bindEngineProtocol(Module));
  const transport = realm.local.connect();
  const tick = setInterval(() => { transport.tick(1); }, 5);   // the shell's delivery loop, at test speed
  try {
    await transport.ready();
    const { character, game, ticket } = realm.local.joined;
    const joined = await d2gsJoin(transport, realm.protocol, { character: character.name, charClass: character.charClass,
      expansion: !!(character.flags & 0x20), gameToken: ticket.token, gameHash: ticket.hash, difficulty: game.difficulty });
    assert.equal(joined.playerAssignment.name.toLowerCase(), character.name.toLowerCase());
    transport.close();
  } finally { clearInterval(tick); }
});
