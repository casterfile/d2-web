// D2Client's ordinary mono-buffer positional attenuation (Sound Mixer mode 0).
// Inputs are world subtile coordinates; dynamic units carry the path's 16.16 position.
export function nativeUnitSoundSpatial(source, listener) {
  const sx = Number(source?.x), sy = Number(source?.y), lx = Number(listener?.x), ly = Number(listener?.y);
  if (![sx, sy, lx, ly].every(Number.isFinite)) return null;
  // FUN_6FB37F30 projects coordinates to client space, subtracts listener from source,
  // doubles vertical displacement, and sets Z=640. The updater scales XYZ by 1/320.
  const dx = 16 * ((sx - sy) - (lx - ly)) / 320;
  const dy = 16 * ((sx + sy) - (lx + ly)) / 320;
  const dz = 2;
  const radius = Math.min(100, Math.sqrt(dx * dx + dy * dy + dz * dz));
  let gainByte = 255;
  if (radius >= 2) {
    // Retail rounds this logarithmic intermediate to float32 before exponentiation.
    const exponent = Math.fround(6 * Math.log2(radius / 2));
    gainByte = Math.trunc(255 / (10 ** (exponent * 0.05)));
  }
  // D2Sound ordinal 10047 truncates the pan byte before ordinal 10028 sets balance.
  const panCode = Math.max(0, Math.min(255, Math.trunc(128 + 127 * 0.8 * dy)));
  return { dx, dy, dz, radius, gainByte: Math.max(0, Math.min(255, gainByte)), panCode };
}

/** The native row-2673 shrine sound is the only source whose full spatial inputs are wired here. */
export function unitSoundNativeSpatial(event, snapshot, listener) {
  if (event?.soundEvent !== 0x0a || !listener) return null;
  const source = unitSoundSourcePosition(event, snapshot);
  if (!source) return null;
  return nativeUnitSoundSpatial(source, listener);
}

export function unitSoundSourcePosition(event, snapshot) {
  if (!Number.isInteger(event?.unitType) || !Number.isInteger(event?.unitId)) return null;
  const sourceUnit = snapshot?.entities?.find((entity) => entity.type === event.unitType && entity.id === (event.unitId >>> 0));
  if (!sourceUnit) return null;
  const dynamic = event.unitType === 0 || event.unitType === 1 || event.unitType === 3;
  return dynamic && sourceUnit.preciseX != null && sourceUnit.preciseY != null
    ? { x: sourceUnit.preciseX, y: sourceUnit.preciseY } : sourceUnit;
}

// A Tracking Sounds.txt source uses the same native pan-byte path regardless of which
// attached event selected it. Distance gain remains a separate, row-specific rule.
export function unitSoundNativePanCode(event, snapshot, listener) {
  if (!listener) return null;
  const source = unitSoundSourcePosition(event, snapshot);
  return source ? nativeUnitSoundSpatial(source, listener)?.panCode ?? null : null;
}

// Packet event 0x10 is D2Client's monster MonSounds.Taunt route. The native
// updater applies pan to ordinary mono buffers; this helper intentionally does
// not apply the shrine's distance gain to taunts (their Falloff rows differ).
export function unitSoundNativeTauntPanCode(event, snapshot, listener) {
  const selectedRow = event?.monsterTaunt ?? event?.nativeSelectedRow;
  if (event?.soundEvent !== 0x10 || event?.unitType !== 1 || !Number.isInteger(selectedRow) || selectedRow <= 0 || !listener) return null;
  return unitSoundNativePanCode(event, snapshot, listener);
}

export function unitSoundNativeGainByte(event, snapshot, listener) {
  return unitSoundNativeSpatial(event, snapshot, listener)?.gainByte ?? null;
}

// D2Sound 1.13c ordinal 10028: native SetPan target in hundredths of a decibel.
export function nativeSoundPanLevel(code) {
  if (!Number.isInteger(code) || code < 0 || code > 255) throw new RangeError('pan code must be an integer from 0 to 255');
  if (code === 128) return 0;
  if (code === 0) return -10000;
  if (code === 255) return 10000;
  const magnitude = code < 128 ? code : 255 - code;
  const level = Math.trunc(2000 * Math.log10(127 / magnitude));
  return level === 0 ? 0 : code < 128 ? -level : level;
}

// DirectSound's documented balance law keeps the near channel at unity.
// This models its numeric controls, not backend resampling or device output.
export function nativeSoundPanBalance(code) {
  const level = nativeSoundPanLevel(code);
  const far = 10 ** (-Math.abs(level) / 2000);
  return { level, left: level > 0 ? far : 1, right: level < 0 ? far : 1 };
}
