// D2Client 1.13c sound mixer rules that read the Sounds.txt columns past Volume.
// Recovered from D2Client.dll: the record loader (0x5DFD0), the channel update (0x5D1D0) and its two
// Falloff tables (0x5DCE0, 0x5DC70), the mixer tick (0x88DD0), the fade/stop request (0x88230) and the
// group variant pick (0x5EE70). Pure integer rules; the player drives them.

/**
 * Falloff -> the two client-pixel distances. Inside `outer` the channel is not attenuated; between
 * `outer` and `cutoff` it falls linearly to 0; a source starts only inside `cutoff` and a playing one
 * stops once it leaves it.
 */
export function soundFalloffRadii(falloff) {
  const f = Number(falloff) | 0;
  const outer = f === 2 ? 200 : f === 3 ? 400 : f === 4 ? 2000 : 60;
  const cutoff = f === 0 ? 400 : f === 2 ? 1000 : f === 3 ? 1500 : f === 4 ? 2000 : 700;
  return { outer, cutoff };
}

/**
 * The source's offset from the listener in client pixels, as the mixer stores it: horizontal is the
 * screen-x difference, vertical twice the screen-y difference (so both use 16 px per subtile).
 * Inputs are world subtile coordinates (dynamic units: the path's precise position).
 */
export function soundClientOffset(source, listener) {
  const sx = Number(source?.x), sy = Number(source?.y), lx = Number(listener?.x), ly = Number(listener?.y);
  if (![sx, sy, lx, ly].every(Number.isFinite)) return null;
  return { x: 16 * ((sx - sy) - (lx - ly)), y: 16 * ((sx + sy) - (lx + ly)) };
}

/** The tick's range test: each axis clamped to +-2000, squared planar distance against cutoff squared. */
export function soundRangeSquared(offset) {
  const clamp = (v) => Math.max(-2000, Math.min(2000, v));
  const x = clamp(offset.x), y = clamp(offset.y);
  return x * x + y * y;
}
/** A queued source starts only while strictly inside its Falloff cutoff. */
export const soundInStartRange = (offset, falloff) => !offset || soundRangeSquared(offset) < soundFalloffRadii(falloff).cutoff ** 2;
/** A playing source is stopped once it is strictly beyond its Falloff cutoff. */
export const soundBeyondCutoff = (offset, falloff) => Boolean(offset) && soundRangeSquared(offset) > soundFalloffRadii(falloff).cutoff ** 2;

/**
 * The channel byte 0x5D1D0 hands D2Sound (ordinal 10008), before the mode-0 radial gain.
 * level = the source's live level (255, or the fade ramp's value); Music Vol rows scale by the music
 * slider, every row by the master slider, each stage truncated; beyond `outer` the level falls
 * linearly to `cutoff`; the row's Volume then scales it by /255.
 */
export function nativeSoundChannel(row, masterPercent, musicPercent = 100, offset = null, level = 255) {
  let l = level | 0;
  if (row?.music) l = Math.trunc(l * musicPercent / 100);
  l = Math.trunc(l * masterPercent / 100);
  if (offset) {
    const { outer, cutoff } = soundFalloffRadii(row?.falloff ?? 0);
    const dist = Math.sqrt(offset.x * offset.x + offset.y * offset.y);
    // Falloff 4 has outer == cutoff; retail divides by zero there, and its tick has already stopped the source.
    if (dist > outer) l = cutoff > outer ? Math.trunc((cutoff - dist) * l / (cutoff - outer)) : 0;
  }
  const volume = row?.volume ?? 255;
  return Math.max(0, Math.trunc(volume * l / 255));
}

/** The fade ramp the tick applies to a source's level: linear in mixer ticks, integer division. */
export function soundFadeLevel(from, to, startTick, endTick, tick) {
  if (tick > endTick) return to;
  if (tick < startTick) return from;
  return Math.trunc((to - from) * (tick - startTick) / (endTick - startTick)) + from;
}

/** Loop restarts the ended buffer only when the row has no Duration. */
export const soundLoops = (row) => Boolean(row?.loop) && !(row?.duration > 0);

/**
 * 0x5EE70: a row with Group Size > 1 plays one of its `groupSize` variants (row .. row+groupSize-1).
 * It avoids the last min(groupSize-1, 2) variants it played (one fewer, for groups of 2 or 3, when
 * rand(groupSize+1) is 0). `rand(n)` is the client hero-seed draw; `history` the row's last two picks.
 */
export function pickSoundVariant(row, groupSize, history, rand) {
  const g = groupSize | 0;
  if (g <= 1) return row;
  let avoid = Math.min(g - 1, 2);
  if (g < 4 && rand(g + 1) === 0) avoid--;
  for (;;) {
    const cand = row + rand(g);
    if (avoid < 1) return cand;
    let i = 0;
    while (i < avoid && history[i] !== cand) i++;
    if (i >= avoid) return cand;
  }
}

/** 0x89640 on a seed pair, for callers without the engine's client hero seed. */
export function d2SeedRand(seed) {
  return (n) => {
    if (!n) return 0;
    const v = BigInt(seed[0] >>> 0) * 0x6ac690c5n + BigInt(seed[1] >>> 0);
    seed[0] = Number(v & 0xffffffffn); seed[1] = Number((v >> 32n) & 0xffffffffn);
    return (n & (n - 1)) ? seed[0] % n : (n - 1) & seed[0];
  };
}
