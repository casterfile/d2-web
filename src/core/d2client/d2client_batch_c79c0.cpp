// D2Client 0xC79C0..0xC8710 batch: env-seam implementations from the decompile.
// Unrecovered callees / imported ordinals are Env function pointers (names positional; register-arg guesses noted).
#include <cstdint>
#include "d2client_batch_c79c0.h"
namespace d2client_batch_c79c0 {
static P pathOf(const Env& e, P u) { if (!u) return 0; P p = e.rd32(u + 0x54); return p ? e.rd32(p + 0x30) : 0; }
static P clsRow(const Env& e, int32_t c) {
 return (c < 0 || (int32_t)e.rd32(e.dataTables + 0xba0) <= c) ? 0 : e.rd32(e.dataTables + 0xb98) + (P)c * 0x23c;
}
static int32_t s16(const Env& e, P a) { return (int16_t)e.rd32(a); }
// 1.13c D2Client+0xC79C0  (u = EAX, want = EDI)
uint32_t sub_C79C0(const Env& e, P u, int32_t want) {
 if (!u) return 0;
 int32_t i = (int32_t)e.rd32(u + 4);
 if (i < 0 || i >= (int32_t)e.rd32(e.dataTables + 0xa80)) return 0;
 P r = e.rd32(e.dataTables + 0xa78) + (P)i * 0x1a8;
 P t = e.o10876(s16(e, r + 0x14));
 if (!t) return 0;
 uint32_t v = 0;
 for (int k = 0; k < 8; k++)
  if (s16(e, r + 0x170 + k * 2) == want && k < 4) v = e.rd32(t + 0x44 + k * 4);
 return (int32_t)v < 0 ? 0 : v;
}
// 1.13c D2Client+0xC7B00  (u = EAX)
void sub_C7B00(const Env& e, P u) {
 P p = pathOf(e, u);
 if (!p) { p = e.o10042(0x48, 0, 0x4c); if (!p) { e.fatal(0x4a); return; } }
 for (int k = 0; k < 0x13; k++) e.wr32(p + k * 4, 0);
 if (u && e.rd32(u + 0x54)) e.wr32(e.rd32(u + 0x54) + 0x30, (uint32_t)p);
}
// 1.13c D2Client+0xC7C80  (u = EAX, n = ECX, a = stack)
void sub_C7C80(const Env& e, P u, int32_t n, int32_t a) {
 P p = pathOf(e, u);
 if (!p) return;
 e.wr32(p, e.rd32(p) | 2);
 e.wr32(p + 0x48, (uint32_t)n);
 if (n < 1) e.wr32(p + 0x48, 1);
 e.wr32(p + 4, 0); e.wr32(p + 8, 0); e.wr32(p + 0xc, (uint32_t)(a * 0x800));
 if (e.rd32(p + 0x24) == 0) e.wr32(p + 0x24, 0xfffff000u);
 uint32_t c = e.rd32(p + 0x48);
 e.wr32(p + 0x10, 0); e.wr32(p + 0x14, 0);
 e.wr32(p + 0x18, (uint32_t)((int32_t)((uint32_t)(a * -0x800) - (uint32_t)((int32_t)(c * c * e.rd32(p + 0x24)) / 2)) / (int32_t)c));
}
// 1.13c D2Client+0xC7E30  (u = EAX, x = ECX, y = EDX, z = stack)
void sub_C7E30(const Env& e, P u, int32_t x, int32_t y, int32_t z) {
 P p = pathOf(e, u);
 if (!p) return;
 e.wr32(p + 4, (uint32_t)z << 11); e.wr32(p + 8, (uint32_t)y << 11); e.wr32(p + 0xc, (uint32_t)x << 11);
}
// 1.13c D2Client+0xC7FB0
void sub_C7FB0(const Env& e, P u) {
 P p = pathOf(e, u);
 if (!p) return;
 uint32_t f = e.rd32(p);
 if (f & 1) return;
 auto R = [&](int k) { return e.rd32(p + k * 4); };
 auto W = [&](int k, uint32_t v) { e.wr32(p + k * 4, v); };
 W(1, R(1) + R(4)); W(4, R(7) + R(4));
 W(2, R(2) + R(5)); W(5, R(8) + R(5));
 W(3, R(3) + R(6));
 uint32_t v6 = R(9) + R(6); W(6, v6);
 if (f & 2) { if (R(0x12) == 0) { W(0, f | 1); W(1, 0); W(2, 0); } else W(0x12, R(0x12) - 1); }
 f = R(0);
 bool done = false;
 if (!(f & 4)) {
  if (f & 0x10) {
   P q = e.f560d0(f);
   if (q) {
    uint32_t t = e.rd32(u);
    if (t != 1 && t == 3) {
     if (e.rd32(q) != 1) { e.fatal(0x1a9); return; }
     if (e.rd32(q + 0x10) == 0 || e.rd32(q + 0x10) == 0xc) return;
    }
    e.f1a6b0();
    W(0xd, 0); W(0xf, 0);
    if (e.rd32(u) == 3) W(0xf, 10);
    W(0xd, R(0xd) + (uint32_t)e.f77f60());
    W(0xf, R(0xf) + (uint32_t)e.f77f20());
    W(1, R(0xd)); W(2, R(0xe));
    e.o10474(p + 4, p + 8);
    W(3, R(0xf) * 0xfffff800u);
   }
   done = true;
  } else {
   int32_t X = (int32_t)R(1) >> 11, Y = (int32_t)R(2) >> 11, Z = (int32_t)R(3) >> 11;
   int32_t a = (int32_t)R(10), b = (int32_t)R(11), c = (int32_t)R(12);
   bool c1 = (f & 0x20) || a < X || b < Y || c < Z;
   bool c2 = !(f & 0x20) || X < a || Y < b || Z < c;
   if (c1 && c2) done = true;
   else { W(1, (uint32_t)a << 11); W(2, (uint32_t)b << 11); W(3, (uint32_t)c << 11); }
  }
 } else {
  if ((int32_t)R(0xc) < ((int32_t)R(3) >> 11)) done = true;
  else {
   int32_t m = (int32_t)(((int64_t)(int32_t)(R(0x11) * v6) * -0x51eb851fLL) >> 32);
   W(6, (uint32_t)((m >> 5) - (m >> 31)));
   W(3, R(0xc));
   if (R(0x10) != 0) { W(0x10, R(0x10) - 1); done = true; }
  }
 }
 if (!done) W(0, f | 1);
 if (!(R(0) & 0x10)) {
  W(0xd, (uint32_t)((int32_t)R(1) >> 11)); W(0xe, (uint32_t)((int32_t)R(2) >> 11));
  e.o10132(p + 0x34, p + 0x38);
  W(0xf, (uint32_t)(-((int32_t)R(3) >> 11)));
 }
}
// 1.13c D2Client+0xC8220  (a = ECX, b = EDX, io = ESI)
uint32_t sub_C8220(const Env& e, uint32_t eaxIn, uint32_t a, uint32_t b, P io) {
 uint32_t S[14] = {0};
 S[0] = eaxIn; S[1] = a; S[3] = b; S[4] = e.rd32(io);
 uint32_t r = e.o10680(0xf, S, 0x38);
 if (r) return r;
 uint32_t r2 = e.o10250(S, 4);
 if (r2 == 1) { e.wr32(io, S[5]); return e.o10640(S), 0; }
 e.o10640(S);
 return r2 ? r2 : 0xfffffffbu;
}
static P mobOf(const Env& e, P u) {
 e.fada0b0(u);
 uint32_t id = e.rd32(u + 0x2c);
 if (!id) { e.fatal(0x946); return 0; }
 return e.o10392(id);
}
// 1.13c D2Client+0xC82A0
bool sub_C82A0(const Env& e, P u) {
 P m = mobOf(e, u);
 if (!m || e.rd32(m) != 4) return false;
 if (!e.o11064(e.rd32(m + 4)) || e.rd32(m + 0x10) != 3) return false;
 if (!e.o10202(m, 0x10, 0x22c, "..\\Source\\D2Client\\SKILLS\\SkillsNec.cpp")) return false;
 if (!e.o10331(m)) return false;
 return e.o11160(m, 0) == 0;
}
// 1.13c D2Client+0xC8410
uint32_t sub_C8410(const Env& e, P u) {
 int32_t h = e.o10511(u);
 if (!h) return 0;
 P m = mobOf(e, u);
 if (!m || !e.o10444(m, 0)) return 0;
 e.o10243(h, e.f1630(m));
 e.o10554(h, e.f1660(m));
 return 1;
}
// 1.13c D2Client+0xC84A0  (u = EAX)
P sub_C84A0(const Env& e, P u) {
 P m = mobOf(e, u);
 if (!m || e.rd32(m) != 1) return 0;
 if (!e.f1510(e.rd32(m + 4), 8) || !e.o10444(m, 0)) return 0;
 return e.fafc430() ? m : 0;
}
// 1.13c D2Client+0xC8510  (a = ECX, cls = EDX)
uint32_t sub_C8510(const Env& e, P a, int32_t cls) {
 P r = clsRow(e, cls);
 if (!r) return 0;
 P h = e.o10331(a); if (!h) return 0;
 uint32_t two[2] = {0, 0};
 if (!e.f51340(two)) return 0;
 P t = e.f1400(h);
 if (!t) return 0;
 if (!((e.rd32(e.bitMasks) & e.rd32(r + 5)) & 0xff) && e.o10057(t)) return 0;
 return 1;
}
// 1.13c D2Client+0xC8620  (a = ECX, cls = EDX, c = stack, u = EAX)
bool sub_C8620(const Env& e, P u, uint32_t a, int32_t cls, uint32_t c) {
 P r = clsRow(e, cls);
 if (!r) return false;
 P m = sub_C84A0(e, u);
 if (!m) return false;
 P x = e.f1260(e.rd32(m + 4));
 if (!x) return false;
 int32_t k = s16(e, r + 0xea);
 uint32_t ty = e.rd32(x + 0xc) & 0xff;
 if (ty == 1) k = s16(e, r + 0xec); else if (ty == 3) k = s16(e, r + 0xee);
 if (k < 0 || k >= (int32_t)e.rd32(e.dataTables + 0xb6c)) return false;
 uint32_t S[23] = {0};
 S[0] = 0x2000; S[1] = a; S[2] = (uint32_t)m; S[4] = (uint32_t)k; S[11] = (uint32_t)cls; S[12] = c;
 return e.f630f0(S) != 0;
}
// 1.13c D2Client+0xC8710  (sk = EAX, p1 = ECX, cls = EDX, p3/p4/p5 = stack)
uint32_t sub_C8710(const Env& e, int32_t sk, uint32_t p1, int32_t cls, P p3, uint32_t p4, uint32_t p5) {
 if (cls < 0 || cls >= (int32_t)e.rd32(e.dataTables + 0xba0)) return 0;
 P r = clsRow(e, cls);
 if (!r) return 0;
 int32_t k = s16(e, r + 0xea);
 int32_t nr = (int32_t)e.rd32(e.dataTables + 0xb6c);
 if (k < 0 || k >= nr) return 0;
 if (sk < 0 || sk >= (int32_t)e.rd32(e.dataTables + 0xa80) || !(e.rd32(e.dataTables + 0xa78) + (P)sk * 0x1a8)) return 0;
 if (!p5 || !p1) return 0;
 if (p3) e.wr32(p3 + 0xc4, e.rd32(p3 + 0xc4) | 0x40);
 uint32_t S[23] = {0};
 S[0] = 0x2001; S[1] = (uint32_t)p3; S[4] = (uint32_t)k; S[5] = p5; S[6] = p1; S[11] = (uint32_t)cls; S[12] = p4;
 P q = e.f630f0(S);
 int32_t k2 = s16(e, r + 0xee);
 if (k2 > 0 && k2 < (int32_t)e.rd32(e.dataTables + 0xb6c)) { S[12] = 2; S[4] = (uint32_t)k2; e.f630f0(S); }
 if (q) {
  int32_t k1 = s16(e, r + 0xec);
  if (k1 > 0 && k1 < (int32_t)e.rd32(e.dataTables + 0xb6c)) e.f66bc0(q, 0, 0, 2);
  if (!e.f55f70(p3) && s16(e, r + 0x10a) > 0) e.f38a70(q, 0, 0, 0, s16(e, r + 0x10a));
 }
 return 1;
}
}
// Plain-memory entry for the browser and the FN probe: the unit chain is read and written in place (32-bit retail pointers).
extern "C" __attribute__((used)) void d2_client_c7e30(uint32_t u, int32_t x, int32_t y, int32_t z) {
 using namespace d2client_batch_c79c0;
 Env e{};
 e.rd32 = [](P a) { return *(const uint32_t*)a; };
 e.wr32 = [](P a, uint32_t v) { *(uint32_t*)a = v; };
 sub_C7E30(e, (P)u, x, y, z);
}

// ---- retail call sites (seam ports). Every callee is one FnCallees call.
namespace d2client_batch_c79c0_seam {
static inline uint32_t R(uint32_t a, uint32_t o) { return fnAt<uint32_t>(fnPtr(a), o); }
static inline void W(uint32_t a, uint32_t o, uint32_t v) { fnAt<uint32_t>(fnPtr(a), o) = v; }
static inline uint32_t L(void* p) { return (uint32_t)(uintptr_t)p; }

// 1.13c D2Client+0xC7B00
void sub_C7B00(FnCallees& s, uint32_t unit) {
    uint32_t p = 0;
    if (unit && R(unit, 0x54)) p = R(R(unit, 0x54), 0x30);
    if (!p) {
        p = s.h(0xbf7a, 0, 0x4c, s.mod(0xd5258), 0, 0, 0x48, 0);
        if (!p) s.fatal(0x4a);
    }
    for (uint32_t i = 0; i < 0x13; i++) W(p, 4 * i, 0);
    if (unit && R(unit, 0x54)) W(R(unit, 0x54), 0x30, p);
}

// 1.13c D2Client+0xC7C80
void sub_C7C80(FnCallees& s, uint32_t unit, uint32_t n, uint32_t k) {
    (void)s;
    if (!unit || !R(unit, 0x54)) return;
    uint32_t c = R(R(unit, 0x54), 0x30);
    if (!c) return;
    W(c, 0, R(c, 0) | 2);
    W(c, 0x48, n);
    if ((int32_t)n < 1) W(c, 0x48, 1);
    uint32_t z = k << 11;
    W(c, 4, 0); W(c, 8, 0); W(c, 0xc, z);
    if (!R(c, 0x24)) W(c, 0x24, 0xfffff000u);
    int32_t m = (int32_t)R(c, 0x48);
    int32_t q = (int32_t)((uint32_t)m * (uint32_t)m * R(c, 0x24));
    W(c, 0x10, 0); W(c, 0x14, 0);
    int32_t num = (int32_t)(0u - z - (uint32_t)(q / 2));
    W(c, 0x18, (uint32_t)(num / m));
}

// 1.13c D2Client+0xC7FB0
void sub_C7FB0(FnCallees& s, uint32_t unit) {
    if (!unit || !R(unit, 0x54)) return;
    uint32_t c = R(R(unit, 0x54), 0x30);
    if (!c) return;
    uint32_t f = R(c, 0);
    if (f & 1) return;
    uint32_t v = R(c, 0x10);
    W(c, 4, R(c, 4) + v); W(c, 0x10, R(c, 0x1c) + v);
    v = R(c, 0x14);
    W(c, 8, R(c, 8) + v); W(c, 0x14, R(c, 0x20) + v);
    v = R(c, 0x18);
    W(c, 0xc, R(c, 0xc) + v);
    uint32_t vz = R(c, 0x24) + v;
    W(c, 0x18, vz);
    if (f & 2) {
        if (R(c, 0x48)) W(c, 0x48, R(c, 0x48) - 1);
        else { W(c, 0, f | 1); W(c, 4, 0); W(c, 8, 0); }
    }
    uint32_t fl = R(c, 0);
    bool snap = false;
    if (fl & 4) {
        if (((int32_t)R(c, 0xc) >> 11) <= (int32_t)R(c, 0x30)) {
            W(c, 0x18, (uint32_t)((int32_t)(R(c, 0x44) * vz) / -100));
            W(c, 0xc, R(c, 0x30));
            if (R(c, 0x40)) W(c, 0x40, R(c, 0x40) - 1);
            else W(c, 0, fl | 1);
        }
    } else if (fl & 0x10) {
        uint32_t r = s.h(0xa60d0, unit);
        if (!r) goto tail;
        uint32_t ty = R(unit, 0), kind = 8;
        uint32_t lx = 0, ly = 0;
        if (ty == 1) kind = 0xe;
        else if (ty == 3) {
            if (R(r, 0) != 1) s.fatal(0x1a9);
            uint32_t m = R(r, 0x10);
            if (m == 0 || m == 0xc) return;
            kind = 0xe;
        }
        s.h(0x6a6b0, r, 0, 0, 0, 0, kind, L(&lx), L(&ly));
        W(c, 0x34, lx); W(c, 0x3c, ly);
        if (R(unit, 0) == 3) W(c, 0x3c, ly + 10);
        W(c, 0x34, R(c, 0x34) + s.h(0xc7f60, r));
        W(c, 0x3c, R(c, 0x3c) + s.h(0xc7f20, r));
        W(c, 4, R(c, 0x34)); W(c, 8, R(c, 0x38));
        s.h(0xc9ac, 0, 0, 0, 0, 0, c + 4, c + 8);
        W(c, 0xc, 0u - (R(c, 0x3c) << 11));
    } else {
        int32_t x = (int32_t)R(c, 4) >> 11, y = (int32_t)R(c, 8) >> 11, z = (int32_t)R(c, 0xc) >> 11;
        int32_t tx = (int32_t)R(c, 0x28), ty = (int32_t)R(c, 0x2c), tz = (int32_t)R(c, 0x30);
        if (!(fl & 0x20)) snap = x <= tx && y <= ty && z <= tz;
        else snap = x >= tx && y >= ty && z >= tz;
        if (snap) {
            W(c, 4, (uint32_t)tx << 11); W(c, 8, (uint32_t)ty << 11); W(c, 0xc, (uint32_t)tz << 11);
            W(c, 0, fl | 1);
        }
    }
tail:
    if (fnAt<uint8_t>(fnPtr(c), 0) & 0x10) return;
    W(c, 0x34, (uint32_t)((int32_t)R(c, 4) >> 11));
    W(c, 0x38, (uint32_t)((int32_t)R(c, 8) >> 11));
    s.h(0xcc2e, 0, 0, 0, 0, 0, c + 0x34, c + 0x38);
    W(c, 0x3c, 0u - (uint32_t)((int32_t)R(c, 0xc) >> 11));
}

// 1.13c D2Client+0xC8220
uint32_t sub_C8220(FnCallees& s, uint32_t a, uint32_t b, uint32_t c, uint32_t io) {
    uint32_t blk[14] = {};   // retail's 0x38-byte stack block; words 2, 5-7, 10-13 are residue the capture masks
    blk[0] = a; blk[1] = b; blk[3] = c; blk[4] = R(io, 0);
    uint32_t r = s.h(0x10680, s.mod(0xd71e4), 0xf, L(blk), 0, 0, 0x38);
    s.blk(0x10680, blk, 14);
    if (r) return r;
    uint32_t e = s.h(0x10250, L(blk), 0, 4);
    if (e == 1) {
        W(io, 0, blk[5]);
        return s.h(0x10640, L(blk));
    }
    s.h(0x10640, L(blk));
    return e ? e : 0xfffffffbu;
}
}
