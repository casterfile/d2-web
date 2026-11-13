// Build provenance contains code identities only. No game data enters the build.
import { readFileSync, writeFileSync } from 'node:fs';
import { createHash } from 'node:crypto';
import { execFileSync } from 'node:child_process';
import { nativeSourceIdentity } from './engine-source-identity.mjs';
const root = process.cwd();
const files = Object.fromEntries(['d2.js', 'd2.wasm'].map(name => [name, createHash('sha256').update(readFileSync('play/' + name)).digest('hex')]));
writeFileSync('play/engine-artifact.json', JSON.stringify({ sourceCommit: execFileSync('git', ['rev-parse', 'HEAD']).toString().trim(), nativeSourceIdentity: nativeSourceIdentity(root), files }, null, 2));
