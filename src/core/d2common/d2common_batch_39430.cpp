// D2Common 1.13c: unit stat getter (pUnit, nStatId, nLayer) without 0x38B70's null-unit guard (retail
// reads unit+0x5C unchecked). Retail keeps the stat list in EBX and the packed id (nStatId << 16 | nLayer)
// in EDI across the call to 0x38A80; here both travel as explicit arguments of
// STATLIST_GetTotalStat_6FDB63E0.
#include <cstdint>
#include <emscripten.h>

#include "D2StatList.h"
#include "Units/Units.h"

int32_t __stdcall STATLIST_GetTotalStatValue(D2StatListStrc* pStatList, int nStatId, uint16_t nLayer);   // D2Common 0x38BD0

extern "C" {

// 1.13c D2Common+0x39430
EMSCRIPTEN_KEEPALIVE int32_t d2_port_d2common_39430(D2UnitStrc* pUnit, int32_t nStatId, uint16_t nLayer)
{
	return STATLIST_GetTotalStatValue(pUnit->pStatListEx, nStatId, nLayer);
}
}
