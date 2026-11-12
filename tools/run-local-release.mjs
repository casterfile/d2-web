// Reproduce the verified local browser release on the existing port and origin.
import { execFileSync, spawn } from 'node:child_process';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
execFileSync(process.execPath, ['tools/stage-local-release.mjs', '--activate'], { cwd: root, stdio: 'inherit' });
const server = spawn(process.execPath, ['play/serve.mjs', '8920', 'retail/experiment-114/site/site'], {
  cwd: root, stdio: 'inherit', env: { ...process.env, D2_RETAIL_DIR: resolve(root, 'retail/D2') },
});
for (const signal of ['SIGINT', 'SIGTERM']) process.on(signal, () => server.kill(signal));
server.on('exit', code => { process.exitCode = code ?? 1; });
