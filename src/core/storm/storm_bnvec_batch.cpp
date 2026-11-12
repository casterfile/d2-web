// Storm bignum word-vector routines (alpha lane: no spec, no vector).
// 1.13c Storm+0x3C80 0x36D0 0x2C3D0 0x2B6A0 0x2B760 0x2BA20 0x2C0B0 0x2C210 0x2D1A0 0x3BF0 0x2C840 0x2BD00
//
// Prior art used: the cached decompiles build-wasm/ghidra-storm/decomp/0x<RVA>.c. Retail passes `this`, the
// shift count, the source pointer etc. in EAX/ECX/ESI/EDI/EBX; they are explicit parameters here. The
// vector is {cap, size, data, grain, base} (retail offsets 0,4,8,0xC,0x10; data/ptr widened for wasm64).
// The temp-stack context holds 16 vectors then a depth word (retail +0x140). The allocator (SMemAlloc /
// SMemReAlloc / SMemFree, Storm 0x1C580/0x1C760/0x1B960) is a seam. Inferred, not visible in the
// decompile: 0x3BF0's implicit ESI/EAX at the 0x2C840/0x2BD00 tail are taken as tmp.size / tmp.data.
// Ghidra artifacts dropped: the `for (x = 0; x != 0; ...)` zero-trip rep-stosb loops.
#include <cstddef>
#include <cstdint>

struct StormBnVec {
    uint32_t cap, size;
    uint32_t* data;
    uint32_t grain, base;
};
struct StormBnCtx {
    StormBnVec t[16];
    uint32_t depth;
};
struct StormBnVecEnv {
    void* (*alloc)(size_t bytes, uint32_t flags);
    void* (*realloc)(void* p, size_t bytes, uint32_t flags);
    void (*free)(void* p);
};
StormBnVecEnv* storm_bnvec_env = nullptr;

namespace {
// 1.13c Storm+0x3C40: default grain = largest power of two <= n (cap 0x40), stored when capped.
uint32_t storm_bnvec_grain(StormBnVec* v, uint32_t n) {
    if (n >= 0x40) { v->grain = 0x40; return 0x40; }
    for (uint32_t u = (n - 1) & n; u != 0; u = (u - 1) & u) n = u;
    return n ? n : 1;
}
}  // namespace

// 1.13c Storm+0x3C80: set capacity to n words, reallocating (copying on fallback).
void storm_bnvec_setcap(StormBnVec* v, uint32_t n) {
    uint32_t* old = v->data;
    v->cap = n;
    v->data = (uint32_t*)storm_bnvec_env->realloc(old, (size_t)n * 4, 0x10);
    if (!v->data) {
        v->data = (uint32_t*)storm_bnvec_env->alloc((size_t)n * 4, 0);
        if (old) {
            uint32_t m = v->size < n ? v->size : n;
            for (uint32_t i = 0; i < m; i++) v->data[i] = old[i];
            storm_bnvec_env->free(old);
        }
    }
}

namespace {
// size = n; grow to a multiple of the grain first when both size and cap are below n.
void storm_bnvec_setsize(StormBnVec* v, uint32_t n) {
    if (v->size < n && v->cap < n) {
        uint32_t g = v->grain ? v->grain : storm_bnvec_grain(v, n);
        storm_bnvec_setcap(v, n % g ? n + g - n % g : n);
    }
    v->size = n;
}
// 1.13c Storm+0x35C0 shape: make absolute index idx addressable, zero-filling new words.
void storm_bnvec_touch(StormBnVec* v, uint32_t idx) {
    if (v->size > idx) return;
    uint32_t n = idx + 1;
    if (v->cap < n) {
        uint32_t g = v->grain ? v->grain : storm_bnvec_grain(v, n);
        storm_bnvec_setcap(v, n % g ? n + g - n % g : n);
    }
    for (uint32_t i = v->size; i <= idx; i++) v->data[i] = 0;
    v->size = n;
}
uint32_t storm_bnvec_get(const StormBnVec* v, uint32_t idx) { return idx < v->size ? v->data[idx] : 0; }
}  // namespace

// 1.13c Storm+0x36D0: drop leading-zero top words down to base.
void storm_bnvec_trim(StormBnVec* v) {
    while (v->size != v->base) {
        if (v->size == 0) return;  // retail dereferences data-4 here
        if (v->data[v->size - 1] != 0) return;
        uint32_t m = v->size - 1;
        if (v->size < m && v->cap < m) {
            uint32_t g = v->grain ? v->grain : storm_bnvec_grain(v, m);
            storm_bnvec_setcap(v, m % g ? m + g - m % g : m);
        }
        v->size = m;
    }
}

// 1.13c Storm+0x2C3D0: set value 1 (size = base+1).
void storm_bnvec_set_one(StormBnVec* v) {
    storm_bnvec_setsize(v, v->base + 1);
    storm_bnvec_touch(v, v->base);
    v->data[v->base] = 1;
}

// 1.13c Storm+0x2B6A0: set value to a single word.
void storm_bnvec_set_word(StormBnVec* v, uint32_t val) {
    storm_bnvec_touch(v, v->base);
    v->data[v->base] = val;
    storm_bnvec_setsize(v, v->base + 1);
}

// 1.13c Storm+0x2B760: load n little-endian bytes into words.
void storm_bnvec_from_bytes(StormBnVec* v, const uint8_t* src, uint32_t n) {
    storm_bnvec_setsize(v, v->base);
    for (uint32_t i = 0; i < n; i++) {
        uint32_t q = i >> 2, prev = 0;
        uint8_t b = src[i];
        if (i & 3) {
            storm_bnvec_touch(v, v->base + q);
            prev = v->data[v->base + q];
        }
        storm_bnvec_touch(v, v->base + q);
        v->data[v->base + q] = ((uint32_t)b << ((i & 3) << 3)) + prev;
    }
}

// 1.13c Storm+0x2BA20: v = a - b (word-wise, borrow chain; stops when both are exhausted).
void storm_bnvec_sub(StormBnVec* v, const StormBnVec* a, const StormBnVec* b) {
    uint32_t lo = 0;
    int32_t hi = 0;
    for (uint32_t i = 0;; i++) {
        uint32_t x;
        if (a->base + i < a->size) x = a->data[a->base + i];
        else {
            if (b->base + i >= b->size) { storm_bnvec_setsize(v, v->base + i); return; }
            x = 0;
        }
        uint32_t y = storm_bnvec_get(b, b->base + i);
        uint32_t d = x - y, r = lo + d;
        uint32_t nlo = ((uint32_t)hi - (x < y)) + (r < lo);
        lo = nlo;
        hi = nlo > 0x7fffffffu ? -1 : 0;
        storm_bnvec_touch(v, v->base + i);
        v->data[v->base + i] = r;
    }
}

// 1.13c Storm+0x2C0B0: v = src >> shift.
void storm_bnvec_shr(StormBnVec* v, uint32_t shift, const StormBnVec* src) {
    uint32_t w = shift >> 5, sb = shift & 31, i = 0;
    while (src->base + i + w < src->size) {
        uint32_t cur = storm_bnvec_get(src, src->base + i + w) >> sb;
        if (sb) cur += storm_bnvec_get(src, src->base + w + 1 + i) << ((32 - sb) & 31);
        storm_bnvec_touch(v, v->base + i);
        v->data[v->base + i] = cur;
        i++;
    }
    storm_bnvec_setsize(v, v->base + i);
}

// 1.13c Storm+0x2C210: v = src << shift, then trim.
void storm_bnvec_shl(StormBnVec* v, uint32_t shift, const StormBnVec* src) {
    uint32_t w = shift >> 5, sb = shift & 31;
    uint32_t total = (src->size - src->base) + 1 + w;
    if (total != 0) {
        uint32_t k = total - w, u = total;
        do {
            u--;
            uint32_t val = 0;
            if (w <= u) {
                val = storm_bnvec_get(src, src->base + k - 1) << sb;
                if (w < u && sb) val += storm_bnvec_get(src, src->base + k - 2) >> ((32 - sb) & 31);
            }
            storm_bnvec_touch(v, v->base + u);
            v->data[v->base + u] = val;
            k--;
        } while (u != 0);
    }
    storm_bnvec_setsize(v, v->base + total);
    storm_bnvec_trim(v);
}

// 1.13c Storm+0x2D1A0: v = src + carry (single-word carry-in propagated up).
void storm_bnvec_add_word(StormBnVec* v, uint32_t carry, const StormBnVec* src) {
    uint32_t i = 0;
    while (carry != 0 || src->base + i < src->size) {
        uint32_t x = storm_bnvec_get(src, src->base + i);
        uint32_t s = carry + x;
        carry = s < carry;
        storm_bnvec_touch(v, v->base + i);
        v->data[v->base + i] = s;
        i++;
    }
    storm_bnvec_setsize(v, v->base + i);
}

// 1.13c Storm+0x2D020: v = a + b, word by word with the carry, while either operand has words past its base.
void storm_bnvec_add(StormBnVec* v, const StormBnVec* a, const StormBnVec* b) {
    uint32_t carry = 0, i = 0;
    while (carry != 0 || a->base + i < a->size || b->base + i < b->size) {
        uint32_t x = storm_bnvec_get(a, a->base + i);
        uint32_t y = storm_bnvec_get(b, b->base + i);
        uint32_t s = x + y;
        uint32_t c = s < x;
        uint32_t t = s + carry;
        c += t < s;
        storm_bnvec_touch(v, v->base + i);
        v->data[v->base + i] = t;
        carry = c;
        i++;
    }
    storm_bnvec_setsize(v, v->base + i);
}

// 1.13c Storm+0x3BF0: dst.data[0..n) = src[0..n), dst.cap = n, dst.size = n.
void storm_bnvec_copy(StormBnVec* dst, const uint32_t* src, uint32_t n) {
    dst->cap = n;
    if (dst->data || n) dst->data = (uint32_t*)storm_bnvec_env->realloc(dst->data, (size_t)n * 4, 0);
    for (uint32_t i = 0; i < n; i++) dst->data[i] = src[i];
    dst->size = n;
}

// 1.13c Storm+0x2C840: dst = a * b (schoolbook). Uses a ctx temp when dst aliases an input.
void storm_bnvec_mul(StormBnVec* dst, StormBnVec* a, StormBnVec* b, StormBnCtx* ctx) {
    StormBnVec* t = dst;
    if (dst == a || dst == b) t = &ctx->t[ctx->depth++];
    storm_bnvec_setsize(t, t->base);
    uint32_t i = 0;
    if (a->base < a->size) {
        do {
            uint32_t carry = 0, j = 0;
            if (b->base < b->size) {
                do {
                    uint32_t x = storm_bnvec_get(a, a->base + i), y = storm_bnvec_get(b, b->base + j);
                    storm_bnvec_touch(t, t->base + i + j);
                    uint64_t acc = (uint64_t)y * x + t->data[t->base + i + j] + carry;
                    carry = (uint32_t)(acc >> 32);
                    t->data[t->base + i + j] = (uint32_t)acc;
                    j++;
                } while (b->base + j < b->size);
            }
            storm_bnvec_touch(t, t->base + j + i);
            t->data[t->base + j + i] = carry;
            i++;
        } while (a->base + i < a->size);
    }
    if (dst != t) {
        storm_bnvec_copy(dst, t->data, t->size);
        dst->grain = t->grain;
        dst->base = t->base;
        ctx->depth--;
    }
}

// 1.13c Storm+0x2BD00: dst = src * src (cross terms doubled). Uses a ctx temp when dst == src.
void storm_bnvec_sqr(StormBnVec* dst, StormBnVec* src, StormBnCtx* ctx) {
    StormBnVec* t = dst;
    if (dst == src) t = &ctx->t[ctx->depth++];
    storm_bnvec_setsize(t, t->base);
    uint32_t i = 0;
    if (src->base < src->size) {
        uint64_t acc = 0;
        uint32_t j = 0;
        for (;;) {
            do {
                uint32_t x = storm_bnvec_get(src, src->base + i), y = storm_bnvec_get(src, j + src->base);
                uint64_t p = (uint64_t)y * x;
                uint32_t k = t->base + i + j;
                storm_bnvec_touch(t, k);
                uint32_t pl = (uint32_t)p, ph = (uint32_t)(p >> 32);
                uint32_t tw = t->data[k];
                uint32_t s1 = tw + pl, h1 = ph + (s1 < tw);
                if (j < i) {
                    uint32_t lo = (uint32_t)acc, hi = (uint32_t)(acc >> 32);
                    uint32_t nlo = lo + pl;
                    hi += ph + (nlo < lo);
                    acc = ((uint64_t)hi << 32) | nlo;
                }
                uint32_t alo = (uint32_t)acc, ahi = (uint32_t)(acc >> 32);
                uint32_t r3 = alo + s1, r10 = ahi + h1 + (r3 < alo);
                uint32_t ovf = (h1 < r10 || (h1 <= r10 && s1 <= r3)) ? 0 : 1;
                acc = ((uint64_t)ovf << 32) + r10;
                t->data[k] = r3;
                j++;
            } while (j <= i);
            storm_bnvec_touch(t, t->base + j + i);
            t->data[t->base + j + i] = (uint32_t)acc;
            i++;
            if (src->base + i >= src->size) break;
            acc = 0;
            j = 0;
        }
    }
    if (dst != t) {
        storm_bnvec_copy(dst, t->data, t->size);
        dst->grain = t->grain;
        dst->base = t->base;
        ctx->depth--;
    }
}

// 1.13c Storm+0x2BBB0: v = src - w (scalar word subtract, borrow chain over src's words).
void storm_bnvec_sub_word(StormBnVec* v, uint32_t w, const StormBnVec* src) {
    uint32_t lo = 0u - w;
    int32_t hi = w ? -1 : 0;
    uint32_t i = 0;
    while (src->base + i < src->size) {
        uint32_t x = src->data[src->base + i];
        uint32_t r = lo + x;
        uint32_t nlo = (uint32_t)hi + (r < lo);
        lo = nlo;
        hi = nlo > 0x7fffffffu ? -1 : 0;
        storm_bnvec_touch(v, v->base + i);
        v->data[v->base + i] = r;
        i++;
    }
    storm_bnvec_setsize(v, v->base + i);
}

// C-linkage entry point for the FN probe (storm_bnvec_touch is in an anonymous namespace above).
// 1.13c Storm+0x35C0 (ECX = relative index): retail adds the base at +0x10 first (`uVar5 = in_EAX[4] + param_1`).
extern "C" void Storm_Port35C0(void* v, uint32_t rel) {
    StormBnVec* b = (StormBnVec*)v;
    storm_bnvec_touch(b, b->base + rel);
}
