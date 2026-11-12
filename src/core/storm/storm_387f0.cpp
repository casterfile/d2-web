// 1.13c Storm+0x387F0: select and patch the generated blitter stub, then run it (probe-visible variant).
#include <cstdint>
#include <cstring>

namespace {
inline uint32_t rd(const uint8_t* p, uint32_t o) { uint32_t v; std::memcpy(&v, p + o, 4); return v; }
inline void wr(uint8_t* p, uint32_t o, uint32_t v) { std::memcpy(p + o, &v, 4); }
inline void jmpTo(uint32_t at, uint32_t target) {   // at = address after the rel32 word
    const uint32_t d = target - at;
    std::memcpy((void*)(uintptr_t)(at - 4), &d, 4);
}
}

extern "C" uint32_t Storm_Sub387F0Real(uint8_t* self, uint8_t* desc, uint32_t endPlain, uint32_t endStrided,
                                       uint32_t (*runStub)(uint32_t, uint32_t*)) {
    uint32_t w = rd(desc, 8);
    const uint32_t end = (rd(self, 8) & 0x4000000) ? endStrided : endPlain;
    if ((rd(self, 8) & 0x40000) == 0) {
        if (rd(self, 0xc) != w) {
            wr(self, 0xc, w);
            wr(self, 0x10, rd(self, 0x14));
            jmpTo(rd(self, 0x14) + rd(self, 0x18), rd(self, 0x4c + w * 4));
            jmpTo(rd(self, 0x44) + rd(self, 0x48), end);
        }
    } else {
        uint32_t key = (w << 2) ^ (rd(desc, 0x18) & 3);
        if (rd(self, 0xc) != key) {
            wr(self, 0xc, key);
            uint32_t head = (4 - (rd(desc, 0x18) & 3)) & 3;
            if (w < head) head = w;
            const uint32_t dw = (w - head) >> 2, tail = (w - head) & 3;
            if (head == 0 && tail == 0) {
                wr(self, 0x10, rd(self, 0x4c + dw * 4));
                jmpTo(rd(self, 0x44) + rd(self, 0x48), end);
            } else {
                wr(self, 0x10, rd(self, 0x1c + head * 4));
                jmpTo(rd(self, 0x14) + rd(self, 0x18), rd(self, 0x4c + dw * 4));
                jmpTo(rd(self, 0x44) + rd(self, 0x48), rd(self, 0x34 + tail * 4));
                jmpTo(rd(self, 0x2c) + rd(self, 0x30), end);
            }
        }
    }
    uint32_t r[6] = { rd(desc, 0x18), rd(desc, 0x1c), rd(desc, 0x20), rd(desc, 0x24), rd(desc, 0x28), rd(desc, 0x2c) };
    // The row-end blocks (0x38A70 plain, 0x38A90 strided): the row count is the stack word [esp+4], the strides are desc+0x14 (source)
    // and desc+0x10 (destination). Plain: --rows, done at zero, else advance both and re-enter. Strided: an odd count steps by
    // desc+0x34 / desc+0x30 after its decrement, an even count by the plain strides.
    uint32_t rows = rd(desc, 0xc);
    const uint32_t sstride = rd(desc, 0x14), dstride = rd(desc, 0x10);
    const bool strided = (rd(self, 8) & 0x4000000) != 0;
    for (;;) {
        if (runStub(rd(self, 0x10), r) == 0) break;   // the fixture ran off its stubs
        if (strided && (rows & 1) == 0) { --rows; r[1] += sstride; r[0] += dstride; continue; }
        if (--rows == 0) break;
        if (strided) { r[1] += rd(desc, 0x34); r[0] += rd(desc, 0x30); }
        else { r[1] += sstride; r[0] += dstride; }
    }
    wr(desc, 0x24, r[3]); wr(desc, 0x28, r[4]); wr(desc, 0x2c, r[5]);
    return 1;
}
