// 1.13c D2Client+0x7BF40: ESI = record whose first dword is a list head.
// Null does nothing. Otherwise 0x7B3D0(eax = head), then the Fog free thunk 0xBF86 (ecx = record, edx = file name,
// stack 0x68A and 0). The callees are injected so a probe sees the exact register and stack arguments.
#include <cstdint>
#include <cstring>

struct D2c7bf40Env {
    void (*freeList)(uint32_t head);                                                   // 0x7B3D0, EAX = head
    void (*fogFree)(uint32_t ptr, uint32_t fileRva, uint32_t line, uint32_t zero);   // thunk 0xBF86: ECX, EDX, [esp], [esp+4]
};

extern "C" void d2c_port_7bf40(const uint8_t* esi, const D2c7bf40Env& e) {
    if (!esi) return;
    uint32_t head; memcpy(&head, esi, 4);
    e.freeList(head);
    e.fogFree((uint32_t)(uintptr_t)esi, 0xD4BDC, 0x68A, 0);
}
