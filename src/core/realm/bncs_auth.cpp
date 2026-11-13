// The browser realm client's SID_AUTH_INFO (0x50) and SID_AUTH_CHECK (0x51) requests, composed by the 1.13c
// BNClient ports with every host hook supplied by the caller, so each export is a pure function of its
// arguments.
//
//   AUTH_INFO  -> BNClient+0x11960: the caller supplies the D2Lang language index (D2Lang #10004 mode 3 writes
//                 that language's product-language dword), the BNCS socket's local IPv4 address, the
//                 UTC-minus-local bias in minutes, GetUserDefaultLCID, GetSystemDefaultUILanguage and the two
//                 GetLocaleInfoA(LOCALE_USER_DEFAULT) strings.
//   AUTH_CHECK -> BNClient+0x12060, which calls BNClient+0xD320 (CheckRevision) and BNClient+0x103B0 (one
//                 36-byte key block per CD key). The caller supplies time(NULL), GetTickCount, the running
//                 seed (BNClient DAT_1F930, 0 at load), the server token (DAT_1F874), the CD keys and owner
//                 (from the project's server config), and the three files the revision DLL hashes.
//
// The revision DLL itself (the server's "ver" MPQ, whose CheckRevision export D320 loads and calls) is not a
// 1.13c module. Its classic form is implemented here: the formula "A=.. B=.. C=.. 4 A=A-S ..." runs over
// Game.exe, Bnclient.dll and D2Client.dll in that order, each padded to a multiple of 1024 bytes with the
// descending bytes 0xFF, 0xFE, ..., with A first XORed by the seed the MPQ name's digit selects. The version
// is the exe's VS_FIXEDFILEINFO product version, and the info string is "<exe> <MM/DD/YY hh:mm:ss> <size>".
// Over the 1.13c retail files with PvPGN's D2XP formula this gives the 0x7686BECA its version table expects.
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <emscripten.h>

// ---- the BNClient ports this file drives ----
struct Bn11960Env {
    uint32_t (*expansion)(); void (*lang)(uint8_t* p, uint32_t n); uint32_t (*local_addr)(uint32_t sock, uint8_t* addr, uint32_t* len);
    void (*system_time)(uint8_t* st); void (*local_time)(uint8_t* st); uint32_t (*to_filetime)(const uint8_t* st, uint32_t* ft);
    uint32_t (*user_lcid)(); uint32_t (*load_library)(const char* name); uint32_t (*get_proc)(uint32_t h, const char* name);
    uint32_t (*call_proc)(uint32_t p); void (*free_library)(uint32_t h); uint32_t (*lang_id)();
    void (*locale_info)(uint32_t lcid, uint32_t type, char* buf, uint32_t n); uint32_t (*slen)(const char* s);
    void (*send)(uint32_t len, uint32_t id, const uint8_t* p);
};
extern "C" uint32_t d2_bnclient_11960(const uint32_t* sock, const Bn11960Env* e);

struct Bn12060Globals {
    uint32_t* seed; const volatile uint8_t* ready; const volatile uint32_t* abort; uint32_t* fail; uint32_t* keys; const uint32_t* stamp;
};
struct Bn12060Env {
    uint32_t (*time)(); uint32_t (*tick)(); void (*sleep)(uint32_t ms); uint32_t (*lookup)(uint32_t* a, uint32_t* b, char* info);
    void (*enter)(); void (*leave)(); void (*load)(); uint32_t (*expansion)(); uint32_t (*scopy)(char* dst, const char* src, uint32_t max);
    void (*key_block)(uint8_t* self, uint32_t eax, uint32_t edi, uint32_t tag); void (*release)(uint32_t h);
    uint32_t (*slen)(const char* s); void (*send)(uint32_t len, uint32_t id, const uint8_t* p);
};
extern "C" uint32_t d2_bnclient_12060(const Bn12060Globals* g, const Bn12060Env* e);

struct BnD320Seams {
    uint32_t (*tick)(); void (*sleep)(uint32_t ms); uint32_t (*open)(uint32_t name, uint32_t a, uint32_t b, uint32_t* h);
    uint32_t (*size)(uint32_t h, uint32_t* out); uint32_t (*extract)(uint32_t name, uint32_t h, char* out);
    uint32_t (*load)(const char* path); uint32_t (*proc)(uint32_t hm, uint32_t name); uint32_t (*module)(uint32_t name);
    uint32_t (*filename)(uint32_t hm, char* out, uint32_t n); void (*copy)(char* d, const char* s, uint32_t max);
    char* (*rchr)(const char* s, int c);
    uint32_t (*check)(uint32_t fn, const char* exe, const char* bn, const char* d2c, uint32_t formula, uint32_t* o1, uint32_t* o2, uint8_t* o3);
    void (*free_lib)(uint32_t hm); void (*close)(uint32_t h); void (*del)(uint32_t path);
    const char* path_read; const char* bn_name; const char* d2c_name; uint32_t path, formula, s_check;
};
extern "C" uint32_t bnclient_check_revision_d320(uint32_t* o1, uint32_t* o2, uint8_t* o3, const BnD320Seams* s);

struct BnKeyContextEnv {
    uint32_t (*str_len)(const char* s); void (*sha1_block_pad)(uint8_t* digest, const void* data, uint32_t len);
    void (*sha1_message)(uint8_t* digest, const void* data, uint32_t len);
    void (*decode26)(const char* key, uint32_t* out1, uint32_t* out2, uint8_t* hash10);
};
// Upstream keeps this optional backend outside its published source tree. A
// clean engine build must report its absence, never synthesize key proof bytes.
extern "C" __attribute__((weak)) int32_t bnclient_batch_103b0(uint32_t* self, uint32_t timestamp, uint32_t tag, const char* key, const BnKeyContextEnv* env);
namespace d2::bnclient::grid { void broken_hash(uint32_t* output, const void* input, uint32_t length); }
extern "C" void bn_sha1_message_1240(uint8_t* digest, const uint8_t* data, uint32_t length);

namespace {
uint32_t rd32(const uint8_t* p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }
uint32_t slen(const char* s) { return s ? (uint32_t)strlen(s) : 0; }
uint32_t sstrcopy(char* d, const char* s, uint32_t cap) {          // Storm SStrCopy: at most cap-1 bytes, NUL, the length
    uint32_t n = 0;
    if (cap == 0) return 0;
    while (s && n + 1 < cap && s[n]) { d[n] = s[n]; ++n; }
    d[n] = 0;
    return n;
}
int var_index(char c) { return c == 'A' ? 0 : c == 'B' ? 1 : c == 'C' ? 2 : c == 'S' ? 3 : -1; }
}  // namespace

extern "C" {

// The classic revision check (the ver DLL's CheckRevision): returns 1 and the checksum in *out, or 0 when the
// formula or MPQ name is not the classic form. `files` holds the three images back to back, sizes[i] each.
EMSCRIPTEN_KEEPALIVE int d2_realm_bncs_checkrev_classic(const char* formula, const char* mpq, const uint8_t* files,
                                                        const uint32_t* sizes, uint32_t* out) {
    static const uint32_t kSeeds[8] = {0xE7F4CB62u, 0xF6A14FFCu, 0xAA5504AFu, 0x871FCDC2u,
                                       0x11BF6A18u, 0xC57292E6u, 0x7927D27Eu, 0x2FEC8733u};
    const char* dot = mpq ? strrchr(mpq, '.') : nullptr;
    if (!dot || dot == mpq || dot[-1] < '0' || dot[-1] > '7') return 0;
    const uint32_t mpqIndex = (uint32_t)(dot[-1] - '0');
    uint32_t v[4] = {0, 0, 0, 0};
    int dst[8], s1[8], s2[8]; char op[8]; int ops = 0;
    for (const char* t = formula; t && *t;) {
        if (t[1] == '=') {
            const int d = var_index(t[0]);
            if (d < 0) return 0;
            const char* x = t + 2;
            if (*x >= '0' && *x <= '9') {
                uint64_t n = 0;
                while (*x >= '0' && *x <= '9') n = n * 10 + (uint64_t)(*x++ - '0');
                v[d] = (uint32_t)n;
            } else {
                if (ops == 8 || var_index(x[0]) < 0 || var_index(x[2]) < 0) return 0;
                dst[ops] = d; s1[ops] = var_index(x[0]); op[ops] = x[1]; s2[ops] = var_index(x[2]);
                if (op[ops] != '+' && op[ops] != '-' && op[ops] != '^' && op[ops] != '*') return 0;
                ++ops;
            }
        }
        while (*t && *t != ' ') ++t;
        while (*t == ' ') ++t;
    }
    if (ops == 0) return 0;
    v[0] ^= kSeeds[mpqIndex];
    const uint8_t* f = files;
    for (int i = 0; i < 3; ++i) {
        const uint32_t n = sizes[i], padded = (n + 1023u) & ~1023u;
        for (uint32_t j = 0; j < padded; j += 4) {
            uint8_t b[4];
            for (uint32_t k = 0; k < 4; ++k) b[k] = j + k < n ? f[j + k] : (uint8_t)(0xFFu - (j + k - n));
            v[3] = rd32(b);
            for (int k = 0; k < ops; ++k) {
                const uint32_t a = v[s1[k]], c = v[s2[k]];
                v[dst[k]] = op[k] == '+' ? a + c : op[k] == '-' ? a - c : op[k] == '^' ? (a ^ c) : a * c;
            }
        }
        f += n;
    }
    *out = v[2];
    return 1;
}

// The exe version the revision DLL reports: VS_FIXEDFILEINFO's product version packed one byte per part
// ((MS>>16)<<24 | (MS&0xFFFF)<<16 | (LS>>16)<<8 | LS&0xFFFF, each part & 0xFF). 0 when the image has none.
EMSCRIPTEN_KEEPALIVE uint32_t d2_realm_bncs_exe_version(const uint8_t* exe, uint32_t n) {
    for (uint32_t i = 0; i + 0x18 <= n; i += 4) {
        if (rd32(exe + i) != 0xFEEF04BDu) continue;
        const uint32_t ms = rd32(exe + i + 0x10), ls = rd32(exe + i + 0x14);
        return (((ms >> 16) & 0xFF) << 24) | ((ms & 0xFF) << 16) | (((ls >> 16) & 0xFF) << 8) | (ls & 0xFF);
    }
    return 0;
}

}  // extern "C"

// ---- SID_AUTH_INFO through BNClient+0x11960 ----
namespace { namespace info {
// D2Lang's language table (D2Lang+0xFDB4, six dwords per language), the dword #10004 writes in mode 3, by the
// language index D2Lang holds (DAT_10A1C; an index above 12 reads entry 0).
const uint32_t kProductLanguage[13] = {0x656E5553u, 0x65734553u, 0x64654445u, 0x66724652u, 0x70744252u, 0x69744954u, 0x6A614A41u,
                                       0x6B6F4B52u, 0x7A68434Eu, 0x7A685457u, 0x706C504Cu, 0x72755255u, 0x6B6F4B52u};
uint8_t* out = nullptr; uint32_t outLen = 0;
uint32_t expansion = 0, language = 0, ip = 0, lcid = 0, uiLanguage = 0; int32_t bias = 0;
const char* abbr = ""; const char* country = "";
uint32_t exp() { return expansion; }
void lang(uint8_t* p, uint32_t n) {
    if (n != 3) return;
    const uint32_t v = kProductLanguage[language > 12 ? 0 : language];
    memcpy(p, &v, 4);
}
uint32_t local_addr(uint32_t, uint8_t* addr, uint32_t* len) { memset(addr, 0, 16); memcpy(addr + 4, &ip, 4); *len = 16; return 1; }
void system_time(uint8_t* st) { memset(st, 0, 16); st[0] = 1; }
void local_time(uint8_t* st) { memset(st, 0, 16); st[0] = 2; }
uint32_t to_filetime(const uint8_t* st, uint32_t* ft) {       // UTC = local + bias minutes, on a fixed local instant
    const uint64_t local = 0x01D0000000000000ull;
    const uint64_t t = st[0] == 1 ? local + (uint64_t)((int64_t)bias * 600000000ll) : local;
    ft[0] = (uint32_t)t; ft[1] = (uint32_t)(t >> 32);
    return 1;
}
uint32_t user_lcid() { return lcid; }
uint32_t load_library(const char*) { return 1; }
uint32_t get_proc(uint32_t, const char*) { return 1; }
uint32_t call_proc(uint32_t) { return uiLanguage; }
void free_library(uint32_t) {}
uint32_t lang_id() { return uiLanguage; }
void locale_info(uint32_t, uint32_t type, char* b, uint32_t n) { sstrcopy(b, type == 7 ? abbr : country, n); }
void send(uint32_t len, uint32_t, const uint8_t* p) { memcpy(out, p, len); outLen = len; }
}  // namespace info
}  // namespace

extern "C" EMSCRIPTEN_KEEPALIVE int d2_realm_bncs_auth_info_env(int expansion, uint32_t language, uint32_t localIp, int32_t biasMinutes,
                                                               uint32_t lcid, uint32_t uiLanguage, const char* abbr, const char* country,
                                                               uint8_t* out) {
    info::expansion = expansion ? 1 : 0; info::language = language; info::ip = localIp; info::bias = biasMinutes;
    info::lcid = lcid; info::uiLanguage = uiLanguage; info::abbr = abbr; info::country = country;
    info::out = out; info::outLen = 0;
    const Bn11960Env e{info::exp, info::lang, info::local_addr, info::system_time, info::local_time, info::to_filetime, info::user_lcid,
                       info::load_library, info::get_proc, info::call_proc, info::free_library, info::lang_id, info::locale_info, slen, info::send};
    const uint32_t sock = localIp ? 1 : 0;              // no address: the socket query is skipped and +0x14 stays 0
    d2_bnclient_11960(&sock, &e);
    return (int)info::outLen;
}

// ---- SID_AUTH_CHECK through BNClient+0x12060 / 0xD320 / 0x103B0 ----
namespace { namespace check {
uint8_t* out = nullptr; uint32_t outLen = 0;
uint32_t now = 0, tick = 0, expansion = 0, badKey = 0;
const char* formula = ""; const char* mpq = ""; const char* exeTime = "";
const uint8_t* files = nullptr; const uint32_t* sizes = nullptr;
const char* key[3] = {nullptr, nullptr, nullptr};          // handle 1 key 0, handle 2 key 1, handle 3 the owner
const char* resolve(uintptr_t h) { return h >= 1 && h <= 3 ? key[h - 1] : nullptr; }
uint32_t time() { return now; }
uint32_t tick_count() { return tick; }
void no_sleep(uint32_t) {}
void nop() {}
uint32_t exp() { return expansion; }
uint32_t scopy(char* d, const char* s, uint32_t max) { const char* r = resolve((uintptr_t)s); return sstrcopy(d, r ? r : s, max); }
void pad(uint8_t* d, const void* in, uint32_t n) { d2::bnclient::grid::broken_hash((uint32_t*)d, in, n); }
void msg(uint8_t* d, const void* in, uint32_t n) { bn_sha1_message_1240(d, (const uint8_t*)in, n); }
void key_block(uint8_t* self, uint32_t eax, uint32_t edi, uint32_t tag) {
    const BnKeyContextEnv env{slen, pad, msg, nullptr};
    if (!bnclient_batch_103b0((uint32_t*)self, eax, tag, resolve(edi), &env)) badKey = 1;
}
void release(uint32_t) {}
void send(uint32_t len, uint32_t, const uint8_t* p) { memcpy(out, p, len); outLen = len; }
// D320's host: the revision MPQ opens and extracts to "ver.dll", whose CheckRevision export is the classic check.
uint32_t open(uint32_t, uint32_t, uint32_t, uint32_t* h) { *h = 1; return 1; }
uint32_t size(uint32_t, uint32_t* n) { *n = 0; return 1; }
uint32_t extract(uint32_t, uint32_t, char* o) { sstrcopy(o, "ver.dll", 0x104); return 1; }
uint32_t load(const char*) { return 1; }
uint32_t proc(uint32_t, uint32_t) { return 1; }
uint32_t module(uint32_t) { return 1; }
uint32_t filename(uint32_t, char* o, uint32_t n) { return sstrcopy(o, "C:\\Diablo II\\Game.exe", n); }
void copy(char* d, const char* s, uint32_t max) { sstrcopy(d, s, max); }
char* rchr(const char* s, int c) { return (char*)strrchr(s, c); }
uint32_t ver_check(uint32_t, const char* exe, const char*, const char*, uint32_t, uint32_t* version, uint32_t* hash, uint8_t* infoOut) {
    if (!d2_realm_bncs_checkrev_classic(formula, mpq, files, sizes, hash)) return 0;
    *version = d2_realm_bncs_exe_version(files, sizes[0]);
    const char* base = strrchr(exe, '\\');
    snprintf((char*)infoOut, 0x100, "%s %s %u", base ? base + 1 : exe, exeTime, sizes[0]);
    return 1;
}
void free_lib(uint32_t) {}
void close(uint32_t) {}
void del(uint32_t) {}
uint32_t lookup(uint32_t* a, uint32_t* b, char* i) {
    const BnD320Seams s{tick_count, no_sleep, open, size, extract, load, proc, module, filename, copy, rchr, ver_check, free_lib, close, del,
                        "ver.mpq", "bnclient.dll", "d2client.dll", 1, 1, 1};
    return bnclient_check_revision_d320(a, b, (uint8_t*)i, &s);
}
}  // namespace check
}  // namespace

// Returns the body length, 0 when the revision check fails (retail sends nothing), or -1 when a CD key does not
// decode (0x103B0 refused it), or -2 when that optional backend is absent.
// *seed is BNClient's running seed: in, then out as the client token at body +0.
extern "C" EMSCRIPTEN_KEEPALIVE int d2_realm_bncs_auth_check_env(uint32_t* seed, uint32_t now, uint32_t tick, uint32_t serverToken, int expansion,
                                                                const char* formula, const char* mpq, const uint8_t* files, const uint32_t* sizes,
                                                                const char* exeTime, const char* key0, const char* key1, const char* owner,
                                                                uint8_t* out) {
    if (!bnclient_batch_103b0) return -2;
    check::out = out; check::outLen = 0; check::badKey = 0;
    check::now = now; check::tick = tick; check::expansion = expansion ? 1 : 0;
    check::formula = formula; check::mpq = mpq; check::files = files; check::sizes = sizes; check::exeTime = exeTime;
    check::key[0] = key0; check::key[1] = key1; check::key[2] = owner;
    const uint8_t ready = 1;
    const uint32_t abort = 0, stamp = serverToken;
    uint32_t fail = 0, keys[4] = {1, 0, 2, 3};
    const Bn12060Globals g{seed, &ready, &abort, &fail, keys, &stamp};
    const Bn12060Env e{check::time, check::tick_count, check::no_sleep, check::lookup, check::nop, check::nop, check::nop, check::exp,
                       check::scopy, check::key_block, check::release, slen, check::send};
    if (!d2_bnclient_12060(&g, &e)) return 0;
    return check::badKey ? -1 : (int)check::outLen;
}
