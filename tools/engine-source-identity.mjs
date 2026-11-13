// The exact native inputs carried by an engine artifact. A JS-only change may
// reuse it; a changed native file, recipe or link line must be rebuilt.
import { execFileSync } from 'node:child_process';
import { readFileSync } from 'node:fs';
import { createHash } from 'node:crypto';
import { resolve } from 'node:path';
export function nativeSourceIdentity(root) {
  const paths = ['src', 'wasm-build/link-d2.sh', 'wasm-build/probe-skip.sh', 'Dockerfile.builder64', 'tools/compiler-cache.sh'];
  const tracked = execFileSync('git', ['ls-files', '-z', '--', ...paths], { cwd: root }).toString().split('\0');
  const other = execFileSync('git', ['ls-files', '--others', '-z', '--', 'src'], { cwd: root }).toString().split('\0');
  const hash = createHash('sha256');
  for (const name of [...new Set([...tracked, ...other])].filter(Boolean).sort()) {
    hash.update(name + '\0'); hash.update(readFileSync(resolve(root, name))); hash.update('\0');
  }
  hash.update(execFileSync('git', ['ls-files', '--stage', 'third_party/StormLib'], { cwd: root }));
  return hash.digest('hex');
}
