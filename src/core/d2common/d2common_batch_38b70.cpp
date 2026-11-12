// D2Common 1.13c: unit stat getter (pUnit, nStatId, nLayer). The same lookup as 0x39430 with a null-unit
// guard. Retail keeps the unit's stat list (+0x5C) in EBX and the packed id (nStatId << 16 | nLayer) in
// EDI across the call to 0x38A80; here both travel as explicit arguments of STATLIST_GetTotalStat_6FDB63E0.
#include <cstdint>
#include <emscripten.h>

#include "D2StatList.h"
#include "Units/Units.h"

int32_t __stdcall STATLIST_GetTotalStatValue(D2StatListStrc* pStatList, int nStatId, uint16_t nLayer);   // D2Common 0x38BD0

extern "C" {

// 1.13c D2Common+0x38B70
EMSCRIPTEN_KEEPALIVE int32_t d2_port_d2common_38b70(D2UnitStrc* pUnit, int32_t nStatId, uint16_t nLayer)
{
	if (!pUnit)
		return 0;
	// no stat list, or an id outside ItemStatCost, is 0; otherwise the total through 0x38A80
	return STATLIST_GetTotalStatValue(pUnit->pStatListEx, nStatId, nLayer);
}
}
