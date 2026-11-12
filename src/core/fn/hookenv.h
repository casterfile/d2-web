// Probe environment for the rule templates in src/core/d2game/*.h (a retail recording FN probes of register-convention
// D2Game functions). h() logs the call exactly as the capture's hook recorder does (count, then 10 words:
// rva, eax ecx edx ebx esi, s0..s3) and returns the next value queued for that rva. sc = (rva, value) pairs,
// zero rva ends; per-rva order, last value repeats, none = 0 -- the same rule as the descriptor's `hook` lines.
#pragma once
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <emscripten/stack.h>

struct HookEnv {
    // the `C` block records of the capture: n dwords at p in 9-word records (rva|0x40000000, d0..d8), stack pointers as 0xB3000000
    void blk(uint32_t rva, uint32_t p, uint32_t n) {
        for (uint32_t o = 0; o < n; o += 9) {
            if (L[0] >= cap) abort();
            uint32_t* e = L + 1 + L[0]++ * 10; e[0] = rva | 0x40000000u;
            for (uint32_t j = 0; j < 9; j++) { uint32_t v = o + j < n ? rd32(p + 4 * (o + j)) : 0; e[1 + j] = (!blkRawOf(rva) && isStk(v)) ? 0xB3000000u : v; }
        }
    }
    // REGS `Z`: the %s string behind a stack-pointer argument word i of a log-writer call: two records (0x10000000|i<<20|rva, 36 string bytes each, zero past the NUL)
    void zstr(uint32_t rva, uint32_t i, const char* s) {
        uint8_t buf[72] = {}; for (uint32_t q = 0; q < 71 && s[q]; ++q) buf[q] = (uint8_t)s[q];
        for (uint32_t o = 0; o < 2; ++o) { if (L[0] >= cap) abort(); uint32_t* e = L + 1 + L[0]++ * 10; e[0] = 0x10000000u | (i << 20) | rva; memcpy(e + 1, buf + 36 * o, 36); }
    }
    // REGS `J`: EDI at hook entry as an argument, one record (rva|0x20000000, edi, 0...) straight after the call's own record; a stack pointer is 0xB3000000.
    void j(uint32_t rva, uint32_t edi) {
        if (L[0] >= cap) abort();
        uint32_t* e = L + 1 + L[0]++ * 10; e[0] = rva | 0x20000000u; e[1] = isStk(edi) ? 0xB3000000u : edi;
        for (int i = 2; i < 10; i++) e[i] = 0;
    }
    // REGS `X`: stack words 4.. of the call (n words from w, at most 9), one record (rva|0x50000000, w4..w12) straight after h(); words past n record 0.
    void x(uint32_t rva, const uint32_t* w, uint32_t n) {
        if (L[0] >= cap) abort();
        uint32_t* e = L + 1 + L[0]++ * 10; e[0] = rva | 0x50000000u;
        for (uint32_t j = 0; j < 9; j++) { uint32_t v = j < n ? w[j] : 0; e[1 + j] = isStk(v) ? 0xB3000000u : v; }
    }
    // REGS `Y`: the callee-saved inputs EBP and EDI at hook entry, one record (rva|0x60000000, ebp, edi, 0...) straight after h() (and after j()).
    void y(uint32_t rva, uint32_t ebp, uint32_t edi) {
        if (L[0] >= cap) abort();
        uint32_t* e = L + 1 + L[0]++ * 10; e[0] = rva | 0x60000000u; e[1] = isStk(ebp) ? 0xB3000000u : ebp; e[2] = isStk(edi) ? 0xB3000000u : edi;
        for (int j = 3; j < 10; j++) e[j] = 0;
    }
    // descriptor `hookgl RVA GRVA`: the module dword at GRVA read at hook entry, one record (rva|0x70000000, value) straight after h() (and after y(), before j()).
    void gl(uint32_t rva, uint32_t v) {
        if (L[0] >= cap) abort();
        uint32_t* e = L + 1 + L[0]++ * 10; e[0] = rva | 0x70000000u; e[1] = v; for (int j = 2; j < 10; j++) e[j] = 0;
    }
    // descriptor `tick N [N2...] log`: one GetTickCount call, logged where retail calls it (record 0xF7000000, value). Returns v.
    uint32_t tick(uint32_t v) {
        if (L[0] >= cap) abort();
        uint32_t* e = L + 1 + L[0]++ * 10; e[0] = 0xF7000000u; e[1] = v; for (int j = 2; j < 10; j++) e[j] = 0;
        return v;
    }
    uint32_t* L; const uint32_t* sc;
    uint32_t rawRva[8]; int nraw = 0;   // descriptor `hooknostk RVA` (the capture does not stack-tag that hook's record)
    uint32_t blkRaw[8]; int nblk = 0;   // descriptor `hookblknostk RVA` (that hook's hookblk records are logged raw)
    bool blkRawOf(uint32_t rva) const { for (int q = 0; q < nblk; q++) if (blkRaw[q] == rva) return true; return false; }
    void noStkBlk(uint32_t rva) { if (nblk < 8) blkRaw[nblk++] = rva; }
    void noStk(uint32_t rva) { if (nraw < 8) rawRva[nraw++] = rva; }
    uint32_t xs5 = 0, xs6 = 0;   // stack words 5 and 6 of the next h() (set by Env::Xs; cleared by h)
    uint32_t ediReg = 0;   // EDI at the next h(): a `hookw RVA e V...` write goes through it (key (0x15<<24)|rva); set by ed()
    uint32_t tbl = 0, dtPtr = 0, fileStr = 0, bmSlot = 0;     // module globals (addresses of the pinned blobs / scrubbed module words)
    static constexpr uint32_t kLogMax = 1024;   // 1024 (was 512; main had 400): 3C850 and 40CA0 log 6 records per step over a 64-step missile ring, past 400; a larger cap only raises the limit. ponytail: a descriptor `blob lg` must hold 4 + 40 * calls bytes; 0x6F930 logs up to 45 calls (blob 2600)
    uint32_t cap = kLogMax;   // a probe whose callee runs per pixel (D2DDraw 0x9460: 12288 GetPixel calls) raises it; its `blob lg` holds 4 + 40 * cap bytes
    uint32_t seen[64][2]; int nseen = 0;   // 64 (was 48): D2Game 0x72220 (cube outputs) reaches up to 61 distinct hooked callees
    // per-site NST (descriptor `hook RVA NST REGS site=RET:NST ...`): the stack words a hooked callee pops when it is called from the
    // caller whose return address (RVA) is RET. st = (RET, NST) pairs, zero RET ends; a miss returns dflt. Probes that mirror a
    // balanced call site use it to size the pop; the log record is the same at every site.
    const uint32_t* st = nullptr;
    uint32_t nstAt(uint32_t retRva, uint32_t dflt) const {
        if (st) for (const uint32_t* p = st; p[0]; p += 2) if (p[0] == retRva) return p[1];
        return dflt;
    }
    HookEnv(uint32_t* log, const uint32_t* seq) : L(log), sc(seq) { L[0] = 0; }
    static uint32_t rd32(uint32_t a) { uint32_t v; memcpy(&v, (const void*)(uintptr_t)a, 4); return v; }
    static int32_t rd16s(uint32_t a) { int16_t v; memcpy(&v, (const void*)(uintptr_t)a, 2); return v; }
    static uint32_t rd8(uint32_t a) { return *(const uint8_t*)(uintptr_t)a; }
    static void wr8(uint32_t a, uint32_t v) { *(uint8_t*)(uintptr_t)a = (uint8_t)v; }
    static void wr32(uint32_t a, uint32_t v) { memcpy((void*)(uintptr_t)a, &v, 4); }
    static bool badCodePtr(uint32_t p) { return p == 0; }
    static uint32_t handlerRva(uint32_t p) { return p & 0xFFFFFF; }   // table words are recorded as 0xB2000000|rva
    static bool isStk(uint32_t a) { return a >= (uint32_t)emscripten_stack_get_end() && a < (uint32_t)emscripten_stack_get_base(); }
    uint32_t stk(const void* p) const { return (uint32_t)(uintptr_t)p; }   // a local's address: only the log tags it (0xB3000000)
    // hookblk: log n dwords of a stack block the callee was handed, 9 per record (rva|0x40000000); a clear MASK bit (word < 32) records 0
    void blk(uint32_t rva, const uint32_t* b, uint32_t n, uint32_t mask = ~0u) {
        for (uint32_t o = 0; o < n; o += 9) {
            if (L[0] >= cap) abort();
            uint32_t* e = L + 1 + L[0]++ * 10;
            e[0] = rva | 0x40000000u;
            for (uint32_t j = 0; j < 9; j++) { uint32_t i = o + j, v = i < n ? b[i] : 0; if (i < 32 && !(mask >> i & 1)) v = 0; e[1 + j] = (!blkRawOf(rva) && isStk(v)) ? 0xB3000000u : v; }
        }
    }
    // REGS `D`: the 9 dwords at [EDI] at hook entry (the event record handed to 0x33CE0), one record (rva|0x40000000, d0..d8). `bytemask` bit i keeps byte i
    // of the 36 (a clear bit zeroes it: stack residue the callee never wrote); a stack pointer is 0xB3000000. Call it where retail calls the callee, after h().
    void edi9(uint32_t rva, const void* rec, uint64_t bytemask = ~0ull) {
        if (L[0] >= cap) abort();
        uint32_t* e = L + 1 + L[0]++ * 10; e[0] = rva | 0x40000000u;
        const uint8_t* b = (const uint8_t*)rec;
        for (uint32_t j = 0; j < 9; j++) { uint32_t v = 0; for (uint32_t q = 0; q < 4; q++) if (bytemask >> (4 * j + q) & 1) v |= (uint32_t)b[4 * j + q] << (8 * q); e[1 + j] = isStk(v) ? 0xB3000000u : v; }
    }
    // REGS `J` / `hookw RVA e`: a callee that takes EDI as an argument or fills an EDI out-pointer. ed() sets EDI for the next h() of that callee (the out-pointer
    // write); edi() logs it as the capture does, one record (rva|0x20000000, edi, 0 x8) after h(). Order: ed(edi); h(rva, ...); edi(rva, edi).
    void ed(uint32_t v) { ediReg = v; }
    void edi(uint32_t rva, uint32_t v) {
        if (L[0] >= cap) abort();
        uint32_t* e = L + 1 + L[0]++ * 10; e[0] = rva | 0x20000000u; e[1] = isStk(v) ? 0xB3000000u : v;
        for (int j = 2; j < 10; j++) e[j] = 0;
    }
    // `giat DLL ORD ... w=LIST` (README "Import recorders"): the wide string an import was handed, logged after its h() record, one ws() per
    // LIST code in ascending order (0-3 stack word, 4 ecx, 5 edx, 6 eax). Records tagged 0x0C000000|code<<20|slot: e[1] = chunk<<16|len
    // (chars, NUL excluded, at most wmax), e[2..9] = 16 chars, zero past the end; a NULL pointer is one record with len 0xFFFF.
    void ws(uint32_t slot, uint32_t code, const char16_t* w, uint32_t wmax = 1024) {
        const uint32_t tag = 0x0C000000u | code << 20 | (slot & 0xFFFFFu);
        uint32_t len = 0; if (!w) len = 0xFFFF; else while (len < wmax && w[len]) ++len;
        if (len == 0xFFFF) { if (L[0] >= cap) abort(); uint32_t* e = L + 1 + L[0]++ * 10; e[0] = tag; e[1] = len; for (int i = 2; i < 10; i++) e[i] = 0; return; }
        for (uint32_t c = 0; c * 16 <= len; ++c) {
            if (L[0] >= cap) abort();
            uint32_t* e = L + 1 + L[0]++ * 10; e[0] = tag; e[1] = c << 16 | len; uint16_t ch[16] = {};
            for (uint32_t j = 0; j < 16 && c * 16 + j < len; ++j) ch[j] = (uint16_t)w[c * 16 + j];
            memcpy(e + 2, ch, 32);
        }
    }
    // `giat ... wid`: the pinned answer of a string lookup (D2Lang 10003), u"{id}" (id & 0xFFFF, decimal), from a ring of 64 buffers in the order
    // the capture fills its own; widReset() at the start of each call (the capture resets per tuple). An answer handed on to a hooked callee is
    // logged 0xB4000000|offset into the ring: pass widTag(p) in h().
    char16_t widBuf[64][16]; uint32_t widN = 0;
    void widReset() { widN = 0; }
    const char16_t* wid(uint32_t id) {
        char16_t* b = widBuf[widN++ & 63]; char t[16]; uint32_t v = id & 0xFFFFu; int n = 0; char r[8]; int m = 0;
        do { r[m++] = (char)('0' + v % 10); v /= 10; } while (v);
        t[n++] = '{'; while (m) t[n++] = r[--m]; t[n++] = '}'; t[n] = 0;
        for (int i = 0; i <= n; ++i) b[i] = (char16_t)t[i];
        return b;
    }
    uint32_t widTag(const void* p) const { uint32_t o = (uint32_t)((const uint8_t*)p - (const uint8_t*)widBuf); return o < sizeof widBuf ? 0xB4000000u | o : (uint32_t)(uintptr_t)p; }
    uint32_t h(uint32_t rva, uint32_t a = 0, uint32_t c = 0, uint32_t d = 0, uint32_t b = 0, uint32_t s = 0,
               uint32_t s0 = 0, uint32_t s1 = 0, uint32_t s2 = 0, uint32_t s3 = 0, uint32_t s4 = 0, uint32_t s5 = 0, uint32_t s6 = 0, uint32_t s7 = 0, uint32_t s8 = 0, uint32_t s9 = 0, uint32_t s10 = 0) {
        if (L[0] >= cap) abort();   // log capacity exceeded
        uint32_t* e = L + 1 + L[0]++ * 10;
        e[0] = rva; e[1] = a; e[2] = c; e[3] = d; e[4] = b; e[5] = s; e[6] = s0; e[7] = s1; e[8] = s2; e[9] = s3;
        // retail logs a pointer into its stack as 0xB3000000; `hookw` out-parameter writes ride in sc as key ((1+stack word)<<24)|rva, or ((0x10+r)<<24)|rva for register r (a c d b s = 0..4); several pairs with one key fill consecutive dwords of that pointee
        if (!s5) s5 = xs5; if (!s6) s6 = xs6; xs5 = xs6 = 0;
        const uint32_t sw[11] = { s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10 };
        { bool raw = false; for (int q = 0; q < nraw; q++) raw |= rawRva[q] == rva; if (!raw) for (int i = 1; i < 10; i++) if (isStk(e[i])) e[i] = 0xB3000000u; }   // `hooknostk RVA`: noStk(rva) keeps that hook's words raw
        const uint32_t rg[6] = { a, c, d, b, s, ediReg }; ediReg = 0;
        for (int kind = 0; kind < 2; kind++) for (uint32_t w = 0; w < (kind ? 6u : 11u); w++) {
            const uint32_t key = ((kind ? 0x10u + w : 1u + w) << 24) | rva, ptr = kind ? rg[w] : sw[w]; uint32_t off = 0;
            for (const uint32_t* p = sc; p[0]; p += 2) if (p[0] == key) { wr32(ptr + off, p[1]); off += 4; }
        }
        int k = 0; while (k < nseen && seen[k][0] != rva) k++;
        if (k == nseen) { if (nseen >= 64) abort(); seen[nseen][0] = rva; seen[nseen++][1] = 0; }
        uint32_t n = seen[k][1]++, last = 0;
        for (const uint32_t* p = sc; p[0]; p += 2) if (p[0] == rva) { last = p[1]; if (!n--) return last; }
        return last;
    }
};

// Mirror of the descriptor's `gvar NAME RVA LEN [V...]` (the FN descriptor README): the capture pins blob NAME onto the
// module range [RVA, RVA+LEN) before the call and reads it back after. The probe registers each blob with add(rva, blobAddr, len)
// (blob k's address is r[14 + 2k]) and reaches a pinned global by its retail RVA: at(rva) is the pointer to those bytes (null outside every
// pin), rd/wr read and write a dword. The probe copies the pinned bytes into the port's own state before the call and back after it, so the
// blob holds what retail's global would hold. A range is looked up by RVA, so a probe can address a word inside a larger pin (0x110C08+4*i).
struct GlobalPins {
    struct R { uint32_t rva, len; uint8_t* p; };
    R r[8]; int n = 0;
    void add(uint32_t rva, uint32_t blobAddr, uint32_t len) { if (n >= 8) abort(); r[n++] = { rva, len, (uint8_t*)(uintptr_t)blobAddr }; }
    uint8_t* at(uint32_t rva, uint32_t len = 4) const { for (int i = 0; i < n; i++) if (rva >= r[i].rva && rva + len <= r[i].rva + r[i].len) return r[i].p + (rva - r[i].rva); return nullptr; }
    uint32_t rd(uint32_t rva) const { uint8_t* q = at(rva); if (!q) abort(); uint32_t v; memcpy(&v, q, 4); return v; }
    void wr(uint32_t rva, uint32_t v) const { uint8_t* q = at(rva); if (!q) abort(); memcpy(q, &v, 4); }
};
