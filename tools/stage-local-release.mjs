// Stage one complete upstream web release, then overlay this checkout's compatible
// client corrections. Never mix a new client with a previous release's WASM.
import { createHash } from 'node:crypto';
import { createReadStream } from 'node:fs';
import { execFileSync } from 'node:child_process';
import { mkdir, readFile, writeFile, symlink, access, rename } from 'node:fs/promises';
import { resolve, dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { isPatch113c } from './patch-shim.mjs';

const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const release = 'v0.1.101';
const apkHash = '08c484e1291e9c9dc39d5d87a9a1522361c7f0514b14bffa6db22874af10df14';
const engineVersion = '60388b19d2cf';
const upstreamCommit = '106f51d1ab3d3f368f21467d556bab6766b301a8';
const overlays = ['gameplay-shell.mjs', 'assets/game-menu.mjs', 'assets/realm-npc-state.mjs', 'assets/realm-npc-screen.mjs'];
const sha = bytes => createHash('sha256').update(bytes).digest('hex');
const exists = path => access(path).then(() => true, () => false);
const cache = join(root, 'retail/releases/downloads');
await mkdir(cache, { recursive: true });
try { execFileSync('git', ['cat-file', '-e', `${upstreamCommit}^{commit}`], { cwd: root, stdio: 'ignore' }); }
catch { execFileSync('git', ['fetch', '--no-tags', 'https://github.com/Virtastic/d2-web.git', upstreamCommit], { cwd: root, stdio: 'inherit' }); }
// Native edits require a new engine build, rather than silently reusing this one.
execFileSync('git', ['diff', '--quiet', upstreamCommit, '--', 'src', 'wasm-build/link-d2.sh'], { cwd: root });

async function download(url, target, validate) {
  if (await exists(target)) {
    const bytes = await readFile(target);
    if (validate(bytes)) return bytes;
    throw new Error(`Cached release content failed validation: ${target}`);
  }
  const response = await fetch(url, { signal: AbortSignal.timeout(120000) });
  if (!response.ok) throw new Error(`Download failed (${response.status}): ${url}`);
  const bytes = Buffer.from(await response.arrayBuffer());
  if (!validate(bytes)) throw new Error(`Release content failed validation: ${url}`);
  await writeFile(target, bytes);
  return bytes;
}

const apk = join(cache, `${release}.apk`);
await download(`https://github.com/Virtastic/d2-web/releases/download/${release}/d2-web-0.1.101-android.apk`, apk, bytes => sha(bytes) === apkHash);
// Use the same normalized data the original browser supplies for every patch level.
// Base MPQs and the user's original patch remain untouched.
const shim = await download('https://diablo2.virtastic.app/assets/patch-113c.patch', join(cache, 'patch-113c.patch'), bytes => isPatch113c(Buffer.from(bytes.toString(), 'base64')));
const patch = Buffer.from(shim.toString(), 'base64');
const baseArchives = ['d2exp', 'd2xmusic', 'd2xtalk', 'd2xvideo', 'd2data', 'd2char', 'd2music', 'd2sfx', 'd2speech', 'd2video'];
const archives = await Promise.all(baseArchives.map(async name => {
  const hash = createHash('sha256'); let size = 0;
  for await (const bytes of createReadStream(join(root, 'retail/D2', `${name}.mpq`))) { hash.update(bytes); size += bytes.length; }
  return { name: `${name}.mpq`, size, sha256: hash.digest('hex') };
}));
const sources = await Promise.all(overlays.map(name => readFile(join(root, 'play', name))));
const content = sha(Buffer.concat([Buffer.from(apkHash + JSON.stringify(archives)), patch, ...sources, await readFile(join(root, 'play/index.html')), await readFile(fileURLToPath(import.meta.url))])).slice(0, 16);
const destination = join(root, 'retail/releases', `${release}-${content}`);
const index = join(destination, 'site/index.html');
if (!await exists(index)) {
  const temporary = `${destination}.staging-${process.pid}`;
  await mkdir(temporary, { recursive: true });
  // The verified APK is used only as a ZIP containing its matching browser build.
  const entries = execFileSync('unzip', ['-Z1', apk], { encoding: 'utf8' }).split('\n').filter(name => name.startsWith('assets/site/') && !name.endsWith('/'));
  if (entries.some(name => name.split('/').includes('..'))) throw new Error('Invalid release archive path');
  execFileSync('unzip', ['-q', apk, ...entries, '-d', temporary]);
  const site = join(temporary, 'assets/site');
  if ((await readFile(join(site, 'engine-version.txt'), 'utf8')).trim() !== engineVersion) throw new Error('Unexpected engine build');
  for (const entry of entries) {
    const name = entry.slice('assets/site/'.length);
    if (!name.endsWith('.mjs') || name.endsWith('.test.mjs') || overlays.includes(name) || name === 'assets/build-version.mjs') continue;
    const source = join(root, 'play', name);
    if (await exists(source) && sha(await readFile(source)) !== sha(await readFile(join(site, name)))) {
      throw new Error(`Client dependency differs from the pinned release: ${name}`);
    }
  }
  for (let i = 0; i < overlays.length; i++) await writeFile(join(site, overlays[i]), sources[i]);
  // All code, artwork and range reads for a page stay on ONE release, including
  // a returning browser with cached files. Existing tabs keep their old URLs/data.
  const originalIndex = await readFile(join(site, 'index.html'), 'utf8');
  const versionedIndex = originalIndex.replace('<!doctype html>', `<!doctype html>\n<base href="/releases/${content}/">`)
    .replaceAll("'retail/' + r.f, r.size", "new URL('retail/' + r.f, document.baseURI).href, r.size");
  await writeFile(join(site, 'index.html'), versionedIndex);
  await writeFile(join(site, 'site-config.json'), '{"assets":"hosted"}\n');
  await mkdir(join(site, 'retail'));
  for (const name of ['patch_d2', ...baseArchives]) {
    const target = join(site, 'retail', `${name}.mpq`);
    if (name === 'patch_d2') await writeFile(target, patch);
    else {
      const original = join(root, 'retail/D2', `${name}.mpq`);
      await access(original);
      await symlink(original, target);
    }
  }
  await writeFile(join(site, 'local-release.json'), JSON.stringify({ release, upstreamCommit, apkHash, engineVersion, patchHash: sha(patch), clientHash: content, archives }, null, 2));
  await mkdir(destination);
  await rename(site, join(destination, 'site'));
}
// The entry URL and query stay unchanged. Publish only a new index pointer; do
// not rewrite old modules or archives beneath an already-running game.
const delivery = join(root, 'retail/experiment-114/site/site');
await mkdir(join(delivery, 'releases'), { recursive: true });
const releaseLink = join(delivery, 'releases', content);
if (!await exists(releaseLink)) await symlink(join(destination, 'site'), releaseLink);
if (process.argv.includes('--activate')) {
  const entry = join(delivery, 'index.html');
  const previous = join(root, 'retail/releases/previous-index.html');
  if (await exists(entry) && !await exists(previous)) await writeFile(previous, await readFile(entry));
  const next = join(delivery, `.index-${process.pid}`);
  await symlink(index, next);
  await rename(next, entry);
}
console.log(dirname(index));
