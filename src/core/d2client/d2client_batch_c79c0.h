// src/core/d2client/d2client_batch_c79c0.h -- the Env and entry points of d2client_batch_c79c0.cpp (its FN probes include it).
#pragma once
#include <cstdint>
namespace d2client_batch_c79c0 {
using P = uintptr_t;
struct Env {
 uint32_t (*rd32)(P); void (*wr32)(P, uint32_t);
 P dataTables;                 // *sgptDataTables
 P bitMasks;                   // gdwBitMasks
 P (*o10876)(int32_t); P (*o10042)(uint32_t size, uint32_t zero, uint32_t ecxLine);   /* edx = file string, ecx = line */ void (*fatal)(uint32_t);
 void (*o10474)(P, P); void (*o10132)(P, P);
 P (*f560d0)(uint32_t); void (*f1a6b0)(); int32_t (*f77f60)(); int32_t (*f77f20)();
 void (*fada0b0)(P unit);                 /* eax = unit */ P (*o10392)(uint32_t); int32_t (*o11064)(uint32_t); int32_t (*o10202)(P, uint32_t, uint32_t, const char*);
 P (*o10331)(P); int32_t (*o11160)(P, uint32_t); int32_t (*o10511)(P); int32_t (*o10444)(P, uint32_t);
 uint32_t (*f1630)(P); uint32_t (*f1660)(P); void (*o10243)(int32_t, uint32_t); void (*o10554)(int32_t, uint32_t);
 uint32_t (*f1510)(P, uint32_t); bool (*fafc430)(); bool (*f51340)(void* out2);               /* one stack word: the pointer to two locals */ P (*f1400)(P eax); int32_t (*o10057)(P);
 P (*f1260)(uint32_t eaxFromM4); P (*f630f0)(const uint32_t*); void (*f66bc0)(P eax, uint32_t edx, uint32_t s0, uint32_t s1); int32_t (*f55f70)(P esi);
 void (*f38a70)(P q, uint32_t s1, uint32_t s2, uint32_t s3, int32_t ebx);
 uint32_t (*o10680)(uint32_t ecx, const uint32_t* S, uint32_t n); uint32_t (*o10250)(const uint32_t* eaxS, uint32_t edx); void (*o10640)(const uint32_t* eaxS);
};
uint32_t sub_C79C0(const Env& e, P u, int32_t want);
void sub_C7B00(const Env& e, P u);
void sub_C7C80(const Env& e, P u, int32_t n, int32_t a);
uint32_t sub_C8220(const Env& e, uint32_t eaxIn, uint32_t a, uint32_t b, P io);
bool sub_C82A0(const Env& e, P u);
uint32_t sub_C8410(const Env& e, P u);
uint32_t sub_C8510(const Env& e, P a, int32_t cls);
bool sub_C8620(const Env& e, P u, uint32_t a, int32_t cls, uint32_t c);
uint32_t sub_C8710(const Env& e, int32_t sk, uint32_t p1, int32_t cls, P p3, uint32_t p4, uint32_t p5);
}
// Retail's call sites: registers and stack words are explicit parameters (pointers are retail's 32-bit words);
// every callee goes through FnCallees.
#include "fn_callees.h"
namespace d2client_batch_c79c0_seam {
void sub_C7B00(FnCallees& s, uint32_t unit /*eax*/);
void sub_C7C80(FnCallees& s, uint32_t unit /*eax*/, uint32_t n /*ecx*/, uint32_t k /*stack*/);
void sub_C7FB0(FnCallees& s, uint32_t unit /*stack*/);
uint32_t sub_C8220(FnCallees& s, uint32_t a /*eax*/, uint32_t b /*ecx*/, uint32_t c /*edx*/, uint32_t io /*esi*/);
}
