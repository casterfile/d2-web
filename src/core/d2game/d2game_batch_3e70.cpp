// src/core/d2game/d2game_batch_3e70.cpp -- 1.13c D2Game.dll CRT rows reimplemented from the
// cached decompiles (build-wasm/ghidra-d2game/decomp/0x<RVA>.c), in RVA order: the MSVC 2003 Release CRT
// 64-bit divide helper, critical-section wrappers and teardown, __amsg_exit, __flsbuf and _output (the
// printf formatter). __mtdeletelocks (0x3F21) has its own spec and awaits a FUNCTION-mode vector.
//
// Host model: retail's 32-bit module globals are the statics below (retail RVA noted). Callees whose
// implementations live elsewhere are seam function pointers on D2GameBatch16Env, in retail argument order;
// the CRT output helpers already recovered (0x4352 write_char, 0x43A9 write_string, src/core/crt_write.cpp)
// are called directly. Critical sections are the compat no-ops (src/core/d2moo/compat/Windows.h).
#include "d2game_scalar_helpers.h"
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <emscripten.h>
#include <Windows.h>      // CRITICAL_SECTION no-ops

// The 32-bit (MSVC 2003) FILE object, VS2003 layout; only the fields the decompiles touch are used.
struct CrtFile32;
struct CrtFileStrc {
    uint32_t _ptr;      // 0x00
    int32_t  _cnt;      // 0x04
    uint32_t _base;     // 0x08
    int32_t  _flag;     // 0x0C
    int32_t  _file;     // 0x10
    int32_t  _charbuf;  // 0x14
    int32_t  _bufsiz;   // 0x18
    uint32_t _tmpfname; // 0x1C
};

// Host seams for the callees this batch does not implement (address = retail RVA).
struct D2GameBatch16Env {
    void*    (*malloc)(uint32_t);                                              // 0x5F78 _malloc
    void     (*free)(void*);                                                   // 0x5A8D _free
    int32_t  (*wctomb)(char*, uint16_t);                                       // 0x8284 _wctomb
    void     (*fmtFloat)(const void*, char*, int32_t, int32_t, int32_t);       // 0x9D61 __fptrap family, slot 0x6FD1FAA8
    void     (*fmtFix)(char*);                                                 // 0x6FD1FAB4
    void     (*fmtExp)(char*);                                                 // 0x6FD1FAAC
    int32_t  (*isatty)(int32_t);                                               // 0x81FA __isatty
    void     (*getbuf)(void*);                                                 // 0x81B6 __getbuf
    int32_t  (*write)(int32_t, const void*, uint32_t);                         // 0x5879 __write
    int32_t  (*lseek)(int32_t, int32_t, int32_t);                             // 0x810B __lseek
    void     (*lock)(int32_t);                                                 // 0x402B __lock
    void     (*unlock)(int32_t);                                               // 0x4022 unlock: 0x3F76(10)
    int32_t* (*errnoPtr)(void);                                                // 0x4E65 __getptd()->_terrno
    int32_t  (*initCritSec)(void*, uint32_t);                                  // 0x7DE3 ___crtInitCritSecAndSpinCount
    void     (*nmsgWrite)(int32_t);                                            // 0x69DC __NMSG_WRITE
    void     (*ffMsgBanner)(void);                                             // 0x6B54 __FF_MSGBANNER
    void     (*unwind2)(void*, int32_t);                                       // 0x7832 __local_unwind2 (SEH helper)
};

// The CRT output helpers already recovered in src/core/d2game/crt_write.cpp.
extern "C" void d2game_write_char(uint8_t ch, CrtFile32* f, int32_t* pCount);
extern "C" void d2game_write_string(int32_t* pCount, const uint8_t* s, CrtFile32* f, int32_t len);

static inline uint16_t rd16(const void* p, uint32_t o) { uint16_t v; std::memcpy(&v, (const uint8_t*)p + o, 2); return v; }
static inline uint32_t rd32(const void* p, uint32_t o) { uint32_t v; std::memcpy(&v, (const uint8_t*)p + o, 4); return v; }

// ---- module state --------------------------------------------------------------------------------
// Retail records are 8 bytes; these host records keep native pointers separate from 32-bit flags.
struct D2GameCrtLock { void* section; uint32_t staticFlag; };
static D2GameCrtLock g_crtLockTable[0x24] = {
    {nullptr,1},{nullptr,1},{nullptr,0},{nullptr,1},{nullptr,1},{nullptr,0},
    {nullptr,1},{nullptr,1},{nullptr,1},{nullptr,0},{nullptr,1},{nullptr,0},
    {nullptr,1},{nullptr,1},{nullptr,1},{nullptr,0},{nullptr,1},{nullptr,1},{nullptr,1}
};
alignas(CRITICAL_SECTION) static uint8_t g_crtStaticLockSections[0x24 * 0x18];
extern "C" void d2game_crt_lock_set(uint32_t index, void* p) { g_crtLockTable[index].section = p; }
extern "C" void* d2game_crt_lock_section(uint32_t index) { return g_crtLockTable[index].section; }
// 0x6FD2FD74 / 0x6FD2FD78: __error_mode / __app_type.
static int32_t g_errorMode;
static int32_t g_appType;
// 0x6FD1F358 / 0x6FD1F378: the two CRT FILE objects __flsbuf special-cases on the isatty path.
static CrtFileStrc g_iob358;
static CrtFileStrc g_iob378;
// 0x6FD3A220: _pioinfo -- 64 block pointers, 32 ioinfo records of 0x24 bytes each, osfile flags at
// record offset 4 (FAPPEND 0x20). Modeled flat: (&table)[fd>>5] + (fd&0x1f)*0x24 == &flat[fd].
// 0x6FD1F730: the record retail uses for fd -1 (no file).
static uint8_t g_ioinfo[0x400][0x24];
static uint8_t g_noio[0x24];
// 0x6FD1F940: DBCS lead-byte table, entry [ch*2+1] bit 0x80. Zero = single-byte code page (retail's
// state: D2 never selects a DBCS locale), so the two-byte emit path below never runs.
static uint8_t g_mbctype[0x200];

// 0x4385 write_multi_char: emit num copies of ch, stopping on the -1 count write_char reports.
static void d2game_write_multi_char(uint8_t ch, int32_t num, CrtFile32* f, int32_t* pCount) {
    for (; num > 0; --num) {
        d2game_write_char(ch, f, pCount);
        if (*pCount == -1) return;
    }
}

static const uint16_t g_wideNull[] = { '(', 'n', 'u', 'l', 'l', ')', 0 };   // 0x6FD1F334 L"(null)"

// 1.13c D2Game+0x3E70 (__aulldiv): unsigned 64-bit / 64-bit division. Operands are the dividend
// EDX:EAX (hi=param_2, lo=param_1) and the divisor EBX:ECX (hi=param_4, lo=param_3); the result is
// EDX:EAX. The decompile's two limbs are the classic restored-divide; both paths give the same 64-bit
// quotient (the divisor-high==0 path splits it into a 64/32 quotient plus the overflow word), so the
// single 64-bit divide below is the rule. A zero divisor raises #DE in retail and here alike.
extern "C"
EMSCRIPTEN_KEEPALIVE uint64_t d2game_aulldiv(uint32_t lo, uint32_t hi, uint32_t dlo, uint32_t dhi) {
    return ((uint64_t)hi << 32 | lo) / ((uint64_t)dhi << 32 | dlo);
}

// Probe seam for __unlock (0x3F76): the vector probe swaps this to log the section pointer.
extern "C" void (*d2game_leave_cs_3f76)(LPCRITICAL_SECTION) = LeaveCriticalSection;

// 1.13c D2Game+0x3F76: `mov eax,[esp+4]; mov eax,[0x6FD1F210 + eax*8]; push eax; call
// LeaveCriticalSection` -- release lock number lockNum. The table slot holds the CRITICAL_SECTION
// pointer; the compat build's critical sections are no-ops, so the table read and call are the rule.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2game_unlock_3f76(int32_t lockNum) {
    d2game_leave_cs_3f76((LPCRITICAL_SECTION)g_crtLockTable[(uint32_t)lockNum].section);
}

// 1.13c D2Game+0x3F8B (__mtinitlocknum): create the critical section for lock lockNum. Returns 1 when
// the slot already holds one (or was filled by this call), 0 on allocation/init failure, which sets
// errno = ENOMEM (0xC). The slot is re-checked under __lock(10); the two 0-returns sit inside that
// try/finally, so the finally's unlock(10) runs on every path out (the decompile shows its
// __local_unwind2 rather than the finally call).
extern "C"
EMSCRIPTEN_KEEPALIVE int32_t d2_fn_d2game_3f8b(int32_t lockNum, const D2GameBatch16Env* env) {
    void** slot = &g_crtLockTable[(uint32_t)lockNum].section;
    if (*slot != 0) return 1;
    void* mem = env->malloc(0x18);
    if (mem == nullptr) {
        *env->errnoPtr() = 0xC;
        return 0;
    }
    env->lock(10);
    if (*slot == 0) {
        if (env->initCritSec(mem, 4000) == 0) {
            env->free(mem);
            *env->errnoPtr() = 0xC;
            env->unlock(10);          // the enclosing __finally
            return 0;
        }
        *slot = mem;
    } else {
        env->free(mem);
    }
    env->unlock(10);
    return 1;
}

// 1.13c D2Game+0x3ED8 (__mtinitlocks): initialize the image's statically flagged CRT locks. Retail's
// 0x24 records begin at 0x6FD1F210 and its 0x24 24-byte critical sections at 0x6FD2FD98. Each flagged
// record receives the next section; a failed 0x7DE3 initializer clears that pointer and returns 0.
extern "C" int32_t d2game_crt_init_crit_sec_and_spin_count(void* cs, uint32_t spin,
                                      const struct D2GameCrtInitCritSecEnv* env);
extern "C"
EMSCRIPTEN_KEEPALIVE int32_t d2_game_3ed8(void) {
    uint8_t* next = g_crtStaticLockSections;
    for (uint32_t i = 0; i < 0x24; ++i) {
        D2GameCrtLock& slot = g_crtLockTable[i];
        if (slot.staticFlag != 1) continue;
        slot.section = next;
        next += 0x18;
        if (d2game_crt_init_crit_sec_and_spin_count(slot.section, 4000, nullptr) == 0) {
            slot.section = nullptr;
            return 0;
        }
    }
    return 1;
}



// 1.13c D2Game+0x3F21 (__mtdeletelocks): walk the 0x24 CRT lock records twice. The first walk deletes
// and frees each allocated lock (flag != 1), clearing its pointer; the second deletes initialized
// statically-backed locks (flag == 1) without freeing their in-module storage. The record layout shares
// g_crtLockTable with __mtinitlocknum (0x3F8B) and __unlock (0x3F76).
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_3F21(void) {
    for (auto& slot : g_crtLockTable) {
        if (slot.section != nullptr && slot.staticFlag != 1) {
            DeleteCriticalSection((LPCRITICAL_SECTION)slot.section);
            std::free(slot.section);
            slot.section = nullptr;
        }
    }
    for (auto& slot : g_crtLockTable) {
        if (slot.section != nullptr && slot.staticFlag == 1)
            DeleteCriticalSection((LPCRITICAL_SECTION)slot.section);
    }
}

// 1.13c D2Game+0x03820 (__amsg_exit): print a runtime-error message for message id msg, then terminate
// with exit code 0xFF. The banner is printed only when __error_mode is 1, or 0 with __app_type 1; the
// message writer is __NMSG_WRITE (0x69DC). Retail leaves through the app's __exit pointer; the model
// takes the default target, _exit.
extern "C"
EMSCRIPTEN_KEEPALIVE void d2_fn_d2game_3820(int32_t msg, const D2GameBatch16Env* env) {
    if (g_errorMode == 1 || (g_errorMode == 0 && g_appType == 1)) env->ffMsgBanner();
    env->nmsgWrite(msg);
    _exit(0xFF);
}

// 1.13c D2Game+0x04239 (__flsbuf): flush a character to a CRT stream. `ch` is the character to append.
// The stream must be writable ((_flag & 0x82) != 0) and not string-backed (_flag & 0x40): otherwise the
// error bit 0x20 is set and -1 returned. A read-mode stream is reset (_cnt = 0, and with _flag & 0x10
// the pointer returns to the buffer base). Buffering: a non-buffered stream writes the character
// straight to the fd; a buffered one stores it at _ptr and writes the bytes already accumulated under
// the pointer, so the write length is (_ptr - _base) before the store. When the accumulated length is
// not positive nothing is written; for an append-mode fd the position is moved to EOF instead. Success
// is the write returning exactly the expected length, and the character (masked to 8 bits) is returned;
// otherwise the error bit is set and -1 returned.
extern "C"
EMSCRIPTEN_KEEPALIVE int32_t D2Game_CrtFlsbuf(int32_t ch, CrtFileStrc* f, const D2GameBatch16Env* env) {
    uint32_t flag = (uint32_t)f->_flag;
    int32_t fd = f->_file;
    int32_t written = 0;
    int32_t expect = 0;
    if (((flag & 0x82) == 0) || ((flag & 0x40) != 0)) {
        f->_flag = (int32_t)(flag | 0x20);
        return -1;
    }
    if ((flag & 1) != 0) {
        f->_cnt = 0;
        if ((flag & 0x10) == 0) {
            f->_flag = (int32_t)(flag | 0x20);
            return -1;
        }
        f->_ptr = f->_base;
        flag &= ~1u;
        f->_flag = (int32_t)flag;
    }
    flag = (uint32_t)f->_flag;
    f->_cnt = 0;
    f->_flag = (int32_t)((flag & ~0x10u) | 2);
    if (((flag & 0x10C) == 0) &&
        (((f != &g_iob358) && (f != &g_iob378)) || env->isatty(fd) == 0)) {
        env->getbuf(f);
    }
    if ((f->_flag & 0x108) == 0) {
        expect = 1;
        written = env->write(fd, &ch, 1);
    } else {
        uint32_t base = f->_base;
        uint32_t ptr = f->_ptr;
        f->_ptr = base + 1;
        expect = (int32_t)(ptr - base);
        f->_cnt = f->_bufsiz - 1;
        if (expect < 1) {
            uint8_t* io = (fd == -1) ? g_noio : g_ioinfo[fd];
            if ((io[4] & 0x20) != 0) env->lseek(fd, 0, 2);
        } else {
            written = env->write(fd, (const void*)(uintptr_t)base, (uint32_t)expect);
        }
        *(uint8_t*)(uintptr_t)f->_base = (uint8_t)ch;
    }
    if (written == expect) return ch & 0xFF;
    f->_flag |= 0x20;
    return -1;
}

// The two format-state tables _output walks, re-expressed as their rule (retail 0x6FD19058 is the
// char-class byte table for 0x20..0x78, 0x6FD19078 the 17x8 transition table whose high nibble is the
// next state). Class 0 is every other character; classes 9..16 never occur for real bytes.
static int32_t d2game_fmt_class(uint8_t c) {
    switch (c) {
    case '%': return 1;
    case '.': return 2;
    case '*': return 3;
    case '0': return 4;
    case ' ': case '#': case '+': case '-': return 6;
    case 'F': case 'I': case 'L': case 'N': case 'h': case 'l': case 'w': return 7;
    case 'B': case 'C': case 'E': case 'G': case 'S': case 'X': case 'Z':
    case 'c': case 'd': case 'e': case 'f': case 'g': case 'i': case 'n':
    case 'o': case 'p': case 's': case 'u': case 'x': return 8;
    default: return (c >= '1' && c <= '9') ? 5 : 0;
    }
}

static int32_t d2game_fmt_state(int32_t klass, int32_t state) {
    switch (klass) {
    case 1: return (state == 0 || state == 7) ? 1 : 0;
    case 2: return (state >= 1 && state <= 3) ? 4 : 0;
    case 3: return (state == 1 || state == 2) ? 3 : (state == 4 ? 5 : 0);
    case 4: return (state == 1 || state == 2) ? 2 : (state == 3 ? 3 : ((state == 4 || state == 5) ? 5 : 0));
    case 5: return (state >= 1 && state <= 3) ? 3 : ((state == 4 || state == 5) ? 5 : 0);
    case 6: return (state == 1 || state == 2) ? 2 : 0;
    case 7: return (state >= 1 && state <= 6) ? 6 : 0;
    case 8: return (state >= 1 && state <= 6) ? 7 : 0;
    default: return 0;
    }
}

// 1.13c D2Game+0x43E0 (_output): the MSVC 2003 formatter -- walks format, emits literal characters
// and one formatted item per conversion into file, and advances args over the consumed argument slots.
// Returns the running output count (negative after a write error). States: 0 literal, 1 '%' seen
// (spec reset), 2 flags, 3 width, 4 '.', 5 precision, 6 size modifier, 7 conversion. Bytes: flags
// 0x1 '+', 0x2 ' ', 0x4 '-', 0x8 '0', 0x80 '#', 0x100 negative, 0x200 force octal leading zero,
// 0x20 short, 0x10 long, 0x800 wide, 0x8000 I64, 0x40 signed/numeric. The float formatting itself
// lives in the three CRT slots (0x6FD1FAA8/FAB4/FAAC) and is left as seams; the surrounding sign,
// width, precision and padding rule is below. Fixed-size workspaces: 0x200 bytes of digit/char buffer
// (retail's local_210 + local_1e4 + local_11) and the wide-character conversion scratch.
extern "C"
EMSCRIPTEN_KEEPALIVE int32_t d2_game_43e0(CrtFileStrc* file, const uint8_t* format,
                                               const uint8_t* args, const D2GameBatch16Env* env) {
    int32_t count = 0;                  // running output count (-1 = write error)
    int32_t state = 0;                  // format state machine
    size_t textLen = 0;                 // length of the current text, or the numeric base before it
    int32_t width = 0;
    int32_t prec = -1;
    int32_t flags = 0;
    int32_t wideOut = 0;                // current text is wide
    int32_t signLen = 0;                // length of the sign/prefix text
    uint8_t signText[2] = { 0, 0 };     // retail {local_22c signChar, local_22b prefixChar}, read as one string
    uint8_t floatMode = 0;
    int32_t err = 0;                    // no output for this item
    int32_t digitOff = 0;               // hex letter offset: 7 upper, 0x27 lower
    const uint8_t* textPtr = nullptr;
    uint8_t* heapBuf = nullptr;
    uint8_t numBuf[0x200];
    uint8_t wideTmp[8];
    uint8_t ap[8];
    uint8_t ch = 0;
    const uint8_t* next = format;
    const uint8_t* argsNext = args;
    int32_t klass = 0;
    uint32_t lo = 0, hi = 0, base = 0, rem = 0;
    uint64_t v = 0;
    uint8_t* w = numBuf;
    uint8_t* buf = numBuf;
    uint8_t* bufStart = numBuf;
    uint32_t ps = 0;
    const uint8_t* wptr = nullptr;
    int32_t lim = 0;
    const uint8_t* str = nullptr;
    const uint8_t* q = nullptr;
    int32_t n = 0;
    int32_t pad = 0;
    size_t remaining = 0;
    uint8_t* const digitEnd = numBuf + 0x1FF;   // retail local_11

    for (;;) {
        ch = *format;
        if (ch == 0 || count < 0) return count;
        next = format + 1;
        argsNext = args;
        klass = ((int8_t)ch >= ' ' && (int8_t)ch <= 'x') ? d2game_fmt_class(ch) : 0;
        state = d2game_fmt_state(klass, state);
        switch (state) {
        case 0:
        case0:
            wideOut = 0;
            if ((g_mbctype[(uint32_t)ch * 2 + 1] & 0x80) != 0) {   // DBCS lead byte: emit both
                d2game_write_char(ch, (CrtFile32*)file, &count);
                next = format + 2;
                d2game_write_char(format[1], (CrtFile32*)file, &count);
            } else {
                d2game_write_char(ch, (CrtFile32*)file, &count);
            }
            break;

        case 1:                        // '%' seen: start a new conversion spec
            prec = -1;
            floatMode = 0;
            err = 0;
            width = 0;
            signLen = 0;
            flags = 0;
            wideOut = 0;
            break;

        case 2:                        // flags
            if (ch == ' ') flags |= 2;
            else if (ch == '#') flags |= 0x80;
            else if (ch == '+') flags |= 1;
            else if (ch == '-') flags |= 4;
            else if (ch == '0') flags |= 8;
            break;

        case 3:                        // width
            if (ch == '*') {
                width = (int32_t)rd32(args, 0);
                argsNext = args + 4;
                if (width < 0) {
                    flags |= 4;
                    width = -width;
                }
            } else {
                width = (int32_t)(int8_t)ch - '0' + width * 10;
            }
            break;

        case 4:                        // '.'
            prec = 0;
            break;

        case 5:                        // precision
            if (ch == '*') {
                prec = (int32_t)rd32(args, 0);
                argsNext = args + 4;
                if (prec < 0) prec = -1;
            } else {
                prec = (int32_t)(int8_t)ch - '0' + prec * 10;
            }
            break;

        case 6:                        // size modifier
            if (ch == 'I') {
                if (next[0] == '6' && format[2] == '4') {
                    next = format + 3;
                    flags |= 0x8000;
                } else if (next[0] == '3' && format[2] == '2') {
                    next = format + 3;
                    flags = (int32_t)((uint32_t)flags & 0xFFFF7FFFu);
                } else if (next[0] != 'd' && next[0] != 'i' && next[0] != 'o' && next[0] != 'u' &&
                           next[0] != 'x' && next[0] != 'X') {
                    state = 0;             // a lone 'I' is a literal
                    goto case0;
                }
            } else if (ch == 'h') flags |= 0x20;
            else if (ch == 'l') flags |= 0x10;
            else if (ch == 'w') flags |= 0x800;
            break;

        case 7:                        // conversion
            if ((int8_t)ch < 'h') {
                if ((int8_t)ch < 'e') {
                    if ((int8_t)ch < 'Y') {
                        if (ch != 'X') {
                            if (ch == 'C') {
                                if ((flags & 0x830) == 0) flags |= 0x800;
                                goto label_char;
                            }
                            if (ch != 'E' && ch != 'G') {
                                if (ch == 'S') {
                                    if ((flags & 0x830) == 0) flags |= 0x800;
                                    goto label_string;
                                }
                                goto label_output;      // unknown conversion: nothing to emit
                            }
                            floatMode = 1;
                            ch = (uint8_t)(ch + 0x20);  // 'E'/'G' -> 'e'/'g'
                            goto label_float;
                        }
                        digitOff = 7;                   // 'X'
                        goto label_hex;
                    } else if (ch == 'Z') {
                        argsNext = args + 4;            // counted string: {count, ptr}
                        ps = rd32(args, 0);
                        textPtr = (const uint8_t*)"(null)";
                        textLen = std::strlen("(null)");
                        wptr = (ps != 0) ? (const uint8_t*)(uintptr_t)rd32((const void*)(uintptr_t)ps, 4) : nullptr;
                        if (wptr != nullptr) {
                            textPtr = wptr;
                            textLen = (size_t)rd16((const void*)(uintptr_t)ps, 0);
                            if ((flags & 0x800) == 0) {
                                wideOut = 0;
                            } else {
                                textLen /= 2;
                                wideOut = 1;
                            }
                        }
                        goto label_output;
                    } else if (ch == 'c') {
                        goto label_char;
                    } else if (ch == 'd') {
                        goto label_signed;
                    }
                    break;                              // 'Y': no conversion, no output
                }
                goto label_float;                       // 'e','f','g'
            } else {
                if (ch == 'i') goto label_signed;
                if (ch != 'n') {
                    if (ch == 'o') {
                        textLen = 8;
                        if ((flags & 0x80) != 0) flags |= 0x200;
                        goto label_int;
                    }
                    if (ch == 'p') {
                        prec = 8;
                        digitOff = 7;
                        goto label_hex;
                    }
                    if (ch == 's') goto label_string;
                    if (ch != 'u') {
                        if (ch != 'x') goto label_output;
                        digitOff = 0x27;                // 'x'
                        goto label_hex;
                    }
                    goto label_unsigned;
                }
                argsNext = args + 4;                    // 'n': store the count
                if ((flags & 0x20) == 0) {
                    *(int32_t*)(uintptr_t)rd32(args, 0) = count;
                } else {
                    *(uint16_t*)(uintptr_t)rd32(args, 0) = (uint16_t)count;
                }
                err = 1;
                break;
            }

        label_signed:                   // 'd','i'
            flags |= 0x40;
            // fall through
        label_unsigned:                 // 'u'
            textLen = 10;
            // fall through
        label_int:                      // integer conversion, base in textLen
            if ((flags & 0x8000) != 0) {
                lo = rd32(args, 0);
                hi = rd32(args, 4);
                argsNext = args + 8;
            } else {
                argsNext = args + 4;
                if ((flags & 0x20) == 0) {
                    lo = rd32(args, 0);
                } else if ((flags & 0x40) == 0) {
                    lo = (uint32_t)rd16(args, 0);                   // unsigned short
                } else {
                    lo = (uint32_t)(int32_t)(int16_t)rd16(args, 0); // signed short sign-extends
                }
                hi = 0;
                if ((flags & 0x40) != 0) hi = (uint32_t)((int32_t)lo >> 31);
            }
            if ((flags & 0x40) != 0 && (int32_t)hi < 0) {   // negate a signed negative
                uint32_t oldLo = lo;
                lo = 0u - lo;
                hi = 0u - (hi + (oldLo != 0 ? 1u : 0u));
                flags |= 0x100;
            }
            if ((flags & 0x8000) == 0) hi = 0;              // only I64 keeps the high word
            if (prec < 0) {
                prec = 1;
            } else {
                flags = (int32_t)((uint32_t)flags & ~8u);
                if (prec > 0x200) prec = 0x200;
            }
            if (lo == 0 && hi == 0) signLen = 0;            // zero suppresses the 0x prefix
            base = (uint32_t)textLen;
            v = ((uint64_t)hi << 32) | lo;
            w = digitEnd;
            for (;;) {
                if (prec < 1 && v == 0) break;
                --prec;
                rem = (uint32_t)(v % base);
                v /= base;
                n = (int32_t)rem + 0x30;
                if (n > 0x39) n += digitOff;
                *w = (uint8_t)n;
                --w;
            }
            textLen = (size_t)(digitEnd - w);
            textPtr = w + 1;
            if ((flags & 0x200) != 0 && (*textPtr != '0' || textLen == 0)) {   // '#' octal
                *w = '0';
                textLen = (size_t)(digitEnd - w) + 1;
                textPtr = w;
            }
            goto label_output;

        label_hex:                      // hex conversion, base 16; digitOff already set
            textLen = 0x10;
            if ((flags & 0x80) != 0) {  // '#': "0x"/"0X" prefix
                signText[1] = (uint8_t)(digitOff + 'Q');
                signText[0] = 0x30;
                signLen = 2;
            }
            goto label_int;

        label_char:                     // 'c','C'
            argsNext = args + 4;
            if ((flags & 0x810) == 0) {
                numBuf[0] = (uint8_t)rd16(args, 0);
                textLen = 1;
            } else {
                n = env->wctomb((char*)numBuf, rd16(args, 0));
                if (n < 0) err = 1;
                textLen = (size_t)(intptr_t)n;
            }
            textPtr = numBuf;
            goto label_output;

        label_string:                   // 's','S'
            lim = (prec == -1) ? 0x7FFFFFFF : prec;
            argsNext = args + 4;
            str = (const uint8_t*)(uintptr_t)rd32(args, 0);
            if ((flags & 0x810) == 0) {
                if (str == nullptr) str = (const uint8_t*)"(null)";
                q = str;
                while (lim != 0) {
                    --lim;
                    if (*q == 0) break;
                    ++q;
                }
                textLen = (size_t)(q - str);
            } else {
                if (str == nullptr) str = (const uint8_t*)g_wideNull;
                wideOut = 1;
                q = str;
                while (lim != 0) {
                    --lim;
                    if (rd16(q, 0) == 0) break;
                    q += 2;
                }
                textLen = (size_t)((q - str) >> 1);
            }
            textPtr = str;
            goto label_output;

        label_float:                    // 'e','f','g','E','G'
            flags |= 0x40;
            buf = numBuf;
            bufStart = numBuf;
            if (prec < 0) {
                prec = 6;
            } else if (prec == 0) {
                if (ch == 'g') prec = 1;
            } else {
                if (prec > 0x200) prec = 0x200;
                if (prec > 0xA3) {
                    heapBuf = (uint8_t*)env->malloc((uint32_t)(prec + 0x15D));
                    if (heapBuf != nullptr) {
                        buf = heapBuf;
                        bufStart = heapBuf;
                    } else {
                        prec = 0xA3;
                    }
                }
            }
            std::memcpy(ap, args, 8);
            argsNext = args + 8;
            env->fmtFloat(ap, (char*)buf, (int32_t)(int8_t)ch, prec, floatMode);
            if ((flags & 0x80) != 0 && prec == 0) env->fmtFix((char*)buf);
            if (ch == 'g' && (flags & 0x80) == 0) env->fmtExp((char*)buf);
            textPtr = buf;
            bufStart = buf;
            if (buf[0] == '-') {
                flags |= 0x100;
                textPtr = buf + 1;
                bufStart = buf + 1;
            }
            textLen = std::strlen((const char*)bufStart);
            goto label_output;

        label_output:                   // emit sign/prefix, padding and the text
            if (err == 0) {
                if ((flags & 0x40) != 0) {
                    if ((flags & 0x100) != 0) signText[0] = '-';
                    else if ((flags & 1) != 0) signText[0] = '+';
                    else if ((flags & 2) != 0) signText[0] = ' ';
                    else goto label_pad;
                    signLen = 1;
                }
            label_pad:
                pad = (width - signLen) - (int32_t)textLen;
                if ((flags & 0xC) == 0) d2game_write_multi_char(0x20, pad, (CrtFile32*)file, &count);
                d2game_write_string(&count, signText, (CrtFile32*)file, signLen);
                if ((flags & 8) != 0 && (flags & 4) == 0) {
                    d2game_write_multi_char(0x30, pad, (CrtFile32*)file, &count);
                }
                if (wideOut == 0 || (int32_t)textLen < 1) {
                    d2game_write_string(&count, textPtr, (CrtFile32*)file, (int32_t)textLen);
                } else {
                    remaining = textLen;
                    q = textPtr;
                    do {
                        --remaining;
                        n = env->wctomb((char*)(wideTmp + 1), rd16(q, 0));
                        q += 2;
                        if (n < 1) break;
                        d2game_write_string(&count, wideTmp + 1, (CrtFile32*)file, n);
                    } while (remaining != 0);
                }
                if ((flags & 4) != 0) d2game_write_multi_char(0x20, pad, (CrtFile32*)file, &count);
            }
            break;
        }

        if (heapBuf != nullptr) {
            env->free(heapBuf);
            heapBuf = nullptr;
        }
        format = next;
        args = argsNext;
    }
}
