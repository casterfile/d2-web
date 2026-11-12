import test from 'node:test';
import assert from 'node:assert/strict';
import * as S from './bnet-srp.mjs';

const hex = (b) => Buffer.from(b).toString('hex');
const fromHex = (h) => Uint8Array.from(Buffer.from(h, 'hex'));

// Values recorded from the retail 1.13c Bnclient: the group installed from its own data, the client's private
// exponent drawn from a pinned entropy pool, then the account-logon sender (0x53) and the logon-proof composer
// (0x54) run on a fixed account, salt and server B. `a` is the 32 little-endian bytes retail drew.
const RETAIL_G = '2f';
const RETAIL_N = '87c7238565f61612d91232c7786c977e55b592a08cb68621031899618b1afff8';
const CASES = [
  {
    user: 'SRPTEST', pass: 'hunter2',
    a: '7794b1ceec092643607d9ab7d5f20f2c496683a07844502dee64fbcad7bed10c',
    A: '498b7d5892b208354fc6b2ed6a9cb1ca0ab578b9d9d3b6921a021d46eb920061',
    salt: '03080d12171c21262b30353a3f44494e53585d62676c71767b80858a8f94999e',
    B: '07121d28333e49545f6a75808b96a1acb7c2cdd8e3eef9040f1a25303b46511c',
    K: '26b834f3a6d71de0747cb0b3707ece30ac8917a8a9969ac7659faa55260f0b1c5165cccb88687a2f',
    M1: 'f8c7836d360b368247dc1450a42ce879a919117c',
    frame53: '498b7d5892b208354fc6b2ed6a9cb1ca0ab578b9d9d3b6921a021d46eb9200615352505445535400',
  },
  {
    user: 'Mixed_Acct.2', pass: 'PassWord99',
    a: 'bddaf714324f6c89a6c3e0fd1b3855728facc9e6b43da5b033233b4062d2e62f',
    A: '2a1cf7fcb141f611ca2408244522e57c6d7becd5098edb10f91062bc424f6c4e',
    salt: '41484f565d646b727980878e959ca3aab1b8bfc6cdd4dbe2e9f0f7fe050c131a',
    B: '1d2a3744515e6b7885929facb9c6d3e0edfa0714212e3b4855626f7c8996a330',
    K: 'f2d633910f3e62daad2dce8e10c1a7a7a8e3d304b26def319463b6d86e41740b8f3cf74755c25769',
    M1: 'dddc16ef4dabb7ccc26136b59bb0eea9f258355a',
    frame53: '2a1cf7fcb141f611ca2408244522e57c6d7becd5098edb10f91062bc424f6c4e4d697865645f416363742e3200',
  },
];

test('the group constants equal the ones retail installs', () => {
  assert.equal(RETAIL_G, S.SRP_G.toString(16));
  assert.equal(hex(S.bigToLe(S.SRP_N, 32)), RETAIL_N);
});

for (const c of CASES) {
  test(`retail logon vector for ${c.user}: A, K, M1 and both payloads`, () => {
    const a = S.leToBig(fromHex(c.a));
    const salt = fromHex(c.salt), B = fromHex(c.B);
    const r = S.srpClient({ user: c.user, pass: c.pass, a, salt, B });
    assert.equal(hex(r.A), c.A);
    assert.equal(hex(r.K), c.K);
    assert.equal(hex(r.M1), c.M1);
    // 0x53 payload: A then the account as typed and its NUL (retail upper-cases only its own copy, used for x and M1); 0x54 payload: M1.
    assert.equal(hex(S.accountLogonPacket(r.A, c.user).slice(4)), c.frame53);
    assert.equal(hex(S.logonProofPacket(r.M1).slice(4)), c.M1);
  });
}
