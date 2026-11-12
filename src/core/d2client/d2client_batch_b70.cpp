// Batch b70: D2Client rows recovered from the cached decompile. 32-bit game ints are int32_t.
#include <cstdint>
#include <initializer_list>
#define W __attribute__((weak))
extern "C" {
// data (RVA in comments are the 1.13c image addresses 0x6fbXXXXX)
W extern int32_t DAT_6fbcc080, DAT_6fbcc098, DAT_6fbcc090, DAT_6fb8bc48, DAT_6fb8bc4c;
W extern int32_t DAT_6fbbb2a4, DAT_6fbbb430, DAT_6fbbb2f8, DAT_6fbbb2fc, DAT_6fbbb300, DAT_6fbbb304, DAT_6fbcbedc;
W extern int32_t DAT_6fbcbee0, DAT_6fbcb834, DAT_6fbcbeec, DAT_6fbbb408, DAT_6fbbb40c, DAT_6fbbb414, DAT_6fbcb9a4,
    DAT_6fbcb9a0, DAT_6fb911c8, DAT_6fb911c4, DAT_6fbbb23c[], DAT_6fbbb242[], DAT_6fbcbbfc, DAT_6fbcb864,
    DAT_6fbcb854, DAT_6fbcb858, DAT_6fbcbc34, DAT_6fbcbc50, DAT_6fbcbc90, DAT_6fbcbc8c, DAT_6fbcbc88, DAT_6fbbb2a4;
// callees
W int32_t FUN_6fb13880(void);
W int32_t FUN_6fb137b0(int32_t* edx_out);  // returns EAX, second result in EDX
W void FUN_6fb13000(void);
W void FUN_6fb13b20(void);
W void FUN_6fb144f0(void);
W void Ordinal_10041(void* cel, int32_t x, int32_t y, int32_t color, int32_t mode, void* pal);
W void Ordinal_511(void* msg);
W int32_t Ordinal_11017(int32_t);
W uint32_t Ordinal_10265(int32_t);
W void Ordinal_10024(const void*, int32_t);
W int32_t FUN_6fb3bd90(int32_t, int32_t);
W void FUN_6fb4fd70(void);
W int32_t FUN_6fb3cf60(void);
W void FUN_6fac62d0(void);
W void FUN_6fb3bbf0(void);
W void FUN_6faf7d60(void*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
W void FUN_6faf8730(void);
W int32_t FUN_6fad6320(void);
W void FUN_6fb72790(int32_t, int32_t, int32_t);
W void FUN_6faf8150(void);
}

// 1.13c D2Client+0x64990
// Places the OK / OK+Cancel button cels of a dialog. The decompile's hidden EDX out-value of
// FUN_6fb137b0 (second-button flag) is exposed as `second`.
extern "C" void d2client_FUN_6fb14990(void) {
  int32_t second = 0;
  FUN_6fb13880();
  int32_t first = FUN_6fb137b0(&second);
  int32_t cel[18] = {};
  auto draw = [&](int32_t x) {
    cel[17] = DAT_6fbcc080;
    Ordinal_10041(cel, x, DAT_6fb8bc4c - 0x2f, -1, 5, 0);
    FUN_6fb13000(); FUN_6fb13b20(); FUN_6fb144f0();
    DAT_6fbcc090 = 0;
  };
  if (second == 0) {
    if (first != 0) { draw(DAT_6fb8bc48 / 2 - 0xcd); return; }
    draw((DAT_6fbcc098 == 0 ? DAT_6fb8bc48 / 2 - 0x54 : DAT_6fb8bc48 / 2 - 0x4a) - 3);
    return;
  }
  if (first != 0) { DAT_6fbcc090 = 1; return; }
  draw((DAT_6fbcc098 == 0 ? DAT_6fb8bc48 / 2 + 0x23 : DAT_6fb8bc48 / 2 + 0x38) - 2);
}

// 1.13c D2Client+0x973C0
// `msg` is the hidden ESI argument (mouse message: +0xc x, +0xe y, +0x18 handled, +0x1c result).
extern "C" int32_t d2client_FUN_6fb473c0(uint8_t* msg) {
  int32_t y = *(uint16_t*)(msg + 0xe), x = *(uint16_t*)(msg + 0xc);
  DAT_6fbbb2a4 = x; DAT_6fbbb430 = y;
  *(int32_t*)(msg + 0x18) = 1; *(int32_t*)(msg + 0x1c) = 0;
  Ordinal_511(msg);
  if (DAT_6fbbb2f8 <= x && x < DAT_6fbbb2fc && DAT_6fbbb300 <= y && y <= DAT_6fbbb304) {
    if (FUN_6fb3bd90(x, y) != 0 && DAT_6fbcbedc != 0) {
      *(int32_t*)(msg + 0x18) = 1; *(int32_t*)(msg + 0x1c) = 0;
      Ordinal_511(msg);
      FUN_6fb4fd70();
    }
    DAT_6fbcbedc = 0;
    return 1;
  }
  DAT_6fbcbedc = 0;
  return 0;
}
