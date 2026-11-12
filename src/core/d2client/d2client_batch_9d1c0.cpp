// src/core/d2client/d2client_batch_9d1c0.cpp -- 12 UNMAPPED 1.13c D2Client.dll rows (alpha lane),
// from build-wasm/ghidra-d2client/decomp/0x<RVA>.c. Base 0x6FAB0000. Globals are parameters; unrecovered
// callees and ordinals are host callbacks after the retail arguments.
// Globals: 11BBFC=cur unit, 11CBC14/11CBC18/11CBC1C = list heads (0x6FBCBC14/18/1C), 6FB9264C = flags.
// GUESSES: 9D280 (EAX=ptr), 9D670 (EDX after 9D630 == param_2), 9D850 (ESI=rec, ECX=param_1),
// 9DC30/9DC60 (EAX=arg, FUN_4FD90 stack args), 9D1C0 return (final head, EAX at exit not shown).
#include <cstdint>
#include <emscripten.h>

static inline uint32_t rd32(uint32_t a) { return *(const uint32_t*)(uintptr_t)a; }
static inline int16_t rd16(uint32_t a) { return *(const int16_t*)(uintptr_t)a; }
static inline void wr32(uint32_t a, uint32_t v) { *(uint32_t*)(uintptr_t)a = v; }

typedef uint32_t (*Find4D630)(uint32_t ecx, uint32_t id);
typedef uint32_t (*Test4FD90)(uint32_t ecx, uint32_t a, uint32_t b);
typedef void (*Fatal)(uint32_t strId);   // ord10265(id) -> ord10024(msg) -> _exit(-1)

// 1.13c D2Client+0x9D1C0 (182 B): insertion-sort the chain linked through +0x80 by the byte string at +0;
// stable (a node goes after equal entries). Returns the new head.
extern "C" EMSCRIPTEN_KEEPALIVE
uint32_t d2_client_9d1c0(uint32_t list) {
    uint32_t head = 0;
    while (list) {
        uint32_t n = list, prev = 0, cur = head;
        list = rd32(n + 0x80);
        while (cur) {
            const uint8_t* a = (const uint8_t*)(uintptr_t)n; const uint8_t* b = (const uint8_t*)(uintptr_t)cur;
            while (*a == *b && *a) { a++; b++; }
            if (*a < *b) break;
            prev = cur; cur = rd32(cur + 0x80);
        }
        if (!prev) { wr32(n + 0x80, head); head = n; }
        else { wr32(n + 0x80, rd32(prev + 0x80)); wr32(prev + 0x80, n); }
    }
    return head;
}

// 1.13c D2Client+0x9D280 (39 B): EAX null -> fatal(0x171); else EAX+0x66.
extern "C" EMSCRIPTEN_KEEPALIVE
uint32_t d2_client_9d280(uint32_t eax, Fatal fatal) {
    if (!eax) fatal(0x171);
    return eax + 0x66;
}

// 1.13c D2Client+0x9D440 (58 B): re-sort each node's [+4] chain of the 11CBC1C list (9D1C0, or 9D160 with EAX =
// the chain head when flags&2), then 9D040.
extern "C" EMSCRIPTEN_KEEPALIVE
void d2_client_9d440(uint32_t list_bc1c, uint32_t flags_9264c,
                     uint32_t (*sub_4d160)(uint32_t eax_head), void (*sub_4d040)(void)) {
    for (uint32_t n = list_bc1c; n; n = rd32(n + 8))
        wr32(n + 4, (flags_9264c & 2) ? sub_4d160(rd32(n + 4)) : d2_client_9d1c0(rd32(n + 4)));
    sub_4d040();
}

// 1.13c D2Client+0x9D500 (296 B): free both lists (heads 11CBC14 -> ids 0x14a, 11CBC18 -> 0x156): per node
// free the +0x34 inner chain (0x3c each, 0x40 for the header) and the +0x38 chain (0x10d each), then the node.
// ord10043 is the Fog free thunk (0xBF86) with retail's register image: ECX = the block, EDX = the RVA of the
// source-file string the call site loads (0xD5FB4 for the 0x3c/0x40 blocks, 0xD4580 for the rest), then line, 0.
typedef void (*Free10043)(uint32_t ecx, uint32_t edx_rva, int line, int zero);
static void free_list(uint32_t* head, uint32_t node_id, Free10043 ord10043) {
    for (uint32_t n = *head; n;) {
        uint32_t next = rd32(n + 0x80), p = rd32(n + 0x34);
        if (p) {
            for (uint32_t i = rd32(p); i;) { uint32_t nx = rd32(i + 8); ord10043(i, 0xD5FB4, 0x3c, 0); i = nx; }
            ord10043(p, 0xD5FB4, 0x40, 0);
        }
        wr32(n + 0x34, 0);
        for (uint32_t i = rd32(n + 0x38); i;) { uint32_t nx = rd32(i + 4); ord10043(i, 0xD4580, 0x10d, 0); i = nx; }
        wr32(n + 0x38, 0);
        ord10043(n, 0xD4580, (int)node_id, 0);
        n = next;
    }
    *head = 0;
}
extern "C" EMSCRIPTEN_KEEPALIVE
void d2_client_9d500(uint32_t* head_bc14, uint32_t* head_bc18, Free10043 ord10043) {
    free_list(head_bc14, 0x14a, ord10043);
    free_list(head_bc18, 0x156, ord10043);
}

// 1.13c D2Client+0x9D670 (38 B): rec = 9D630(ecx, id); returns rec[+0x14] unless id == cur unit's [+0xc]
// (or rec null).
extern "C" EMSCRIPTEN_KEEPALIVE
uint32_t d2_client_9d670(uint32_t ecx, uint32_t id, uint32_t unit_bbfc, Find4D630 sub_4d630) {
    uint32_t rec = sub_4d630(ecx, id);
    int32_t cur = unit_bbfc ? (int32_t)rd32(unit_bbfc + 0xc) : -1;
    return ((int32_t)id != cur && rec) ? rd32(rec + 0x14) : 0;
}

// 1.13c D2Client+0x9D850 (80 B): esi null -> fatal(0x5d1); [esi] != 0 -> fatal(0x5d2); rec = 9D630(ecx, esi[3]);
// rec ? rec+0x66 : ord10920(esi) (0 -> fatal(0x5db)).
extern "C" EMSCRIPTEN_KEEPALIVE
uint32_t d2_client_9d850(uint32_t ecx, uint32_t esi, Find4D630 sub_4d630, uint32_t (*ord10920)(uint32_t arg), Fatal fatal) {
    if (!esi) fatal(0x5d1);
    if (rd32(esi)) fatal(0x5d2);
    uint32_t rec = sub_4d630(ecx, rd32(esi + 0xc));
    if (rec) return rec + 0x66;
    uint32_t r = ord10920(esi);   // stdcall thunk 0xC7DE, one stack word: the record
    if (!r) fatal(0x5db);
    return r;
}

// 1.13c D2Client+0x9D910 (118 B): for each node of 11CBC14 (popped by 9D0A0), ensure a 12-byte entry with
// that node's s16 [+0x22] exists in the 11CBC1C chain (next at +8); new ones are pushed on the front.
extern "C" EMSCRIPTEN_KEEPALIVE
void d2_client_9d910(uint32_t* head_bc14, uint32_t* head_bc1c,
                     uint32_t (*ord10042)(uint32_t ecx_size, uint32_t edx_rva, int line, int zero), void (*sub_4d0a0)(uint32_t edi_group)) {
    while (*head_bc14) {
        int16_t id = rd16(*head_bc14 + 0x22);
        uint32_t e = *head_bc1c;
        while (e && rd16(e) != id) e = rd32(e + 8);
        if (!e) {
            e = ord10042(0xc, 0xD4580, 0x1f9, 0);   // Fog alloc thunk 0xBF7A: ECX = 12, EDX = source-file string RVA
            wr32(e, 0); wr32(e + 4, 0); wr32(e + 8, 0);
            *(int16_t*)(uintptr_t)e = id;
            wr32(e + 8, *head_bc1c);
            *head_bc1c = e;
        }
        sub_4d0a0(e);   // EDI = the group (found or new)
    }
}

// 1.13c D2Client+0x9D990 (55 B): returns 9D630(ecx, cur unit id or -1) (EAX stays the lookup result); 0 calls fatal(0x72).
extern "C" EMSCRIPTEN_KEEPALIVE
uint32_t d2_client_9d990(uint32_t ecx, uint32_t unit_bbfc, Find4D630 sub_4d630, Fatal fatal) {
    uint32_t rec = sub_4d630(ecx, unit_bbfc ? rd32(unit_bbfc + 0xc) : 0xFFFFFFFFu);
    if (!rec) fatal(0x72);
    return rec;
}

// 1.13c D2Client+0x9DA70 (87 B): cur = unit id or -1 (ESI); rec = 9D630(ecx, cur); s = rec[+0x22] or -1.
// Then 9D910, 9D440, 9CFC0(s) with EBX = cur, 9CF60, 9D100, and 9CF10 with ESI = cur when s == -1.
// The callees read cur from EBX (9CFC0) and ESI (9CF10) and ignore ECX, so the key is passed explicitly.
extern "C" EMSCRIPTEN_KEEPALIVE
void d2_client_9da70(uint32_t ecx, uint32_t unit_bbfc, Find4D630 sub_4d630, void (*sub_4d910)(void),
                     void (*sub_4d440)(void), void (*sub_4cfc0)(uint32_t ecx, int16_t s, uint32_t key),
                     void (*sub_4cf60)(void), void (*sub_4d100)(void), void (*sub_4cf10)(uint32_t key)) {
    uint32_t cur = unit_bbfc ? rd32(unit_bbfc + 0xc) : 0xFFFFFFFFu;
    uint32_t rec = sub_4d630(ecx, cur);
    int16_t s = rec ? rd16(rec + 0x22) : (int16_t)-1;
    sub_4d910();
    sub_4d440();
    sub_4cfc0(ecx, s, cur);
    sub_4cf60();
    sub_4d100();
    if (s == -1) sub_4cf10(cur);
}

// retired, see d2client_9db90.cpp: D2Client 0x9DB90.

// 1.13c D2Client+0x9DC30 (41 B): rec = 9D630(ecx, id); 0 if null; 1 if EAX == rec[+0x10], else 4FD90(.., rec[+0x10], 1) != 0.
extern "C" EMSCRIPTEN_KEEPALIVE
uint32_t d2_client_9dc30(uint32_t eax, uint32_t ecx, uint32_t id, Find4D630 sub_4d630, Test4FD90 sub_4fd90) {
    uint32_t rec = sub_4d630(ecx, id);
    if (!rec) return 0;
    if (eax != rd32(rec + 0x10) && !sub_4fd90(ecx, rd32(rec + 0x10), 1)) return 0;
    return 1;
}

// 1.13c D2Client+0x9DC60 (82 B): either rec missing -> true; same non-(-1) s16 [+0x22] -> false;
// else 4FD90(s, a[+0x10], 8) != 0.
extern "C" EMSCRIPTEN_KEEPALIVE
bool d2_client_9dc60(uint32_t ecx, uint32_t id_a, uint32_t id_b, Find4D630 sub_4d630, Test4FD90 sub_4fd90) {
    uint32_t a = sub_4d630(ecx, id_a), b = sub_4d630(ecx, id_b);
    if (!a || !b) return true;
    int16_t s = rd16(a + 0x22);
    if (s == rd16(b + 0x22) && s != -1) return false;
    return sub_4fd90((uint32_t)(int32_t)s & 0xFFFF, rd32(a + 0x10), 8) != 0;
}
