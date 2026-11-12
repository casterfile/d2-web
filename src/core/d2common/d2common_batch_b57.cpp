// 1.13c D2Common batch b57: three UNMAPPED rows translated from the pinned decompiles
// (0x24280, 0x66350, 0x3C5E0). Records keep retail's 32-bit layout: fields are read at their
// retail byte offsets, and retail's fatal asserts (_exit) are modelled as neutral returns.
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <emscripten.h>

#include "D2DataTbls.h"
#include "D2Items.h"
#include "DataTbls/ItemsTbls.h"
#include "Units/Units.h"

static_assert(offsetof(D2ItemTypesTxt, nClass) == 0x21, "itemtypes.txt nClass");

// 1.13c D2Common+0x24280 (#10822): itemtypes.txt nClass of the unit's item type; a non-item,
// a missing row or nClass >= 7 answers 7. Retail calls 0x230F0 for the item type id.
extern "C"
EMSCRIPTEN_KEEPALIVE uint8_t d2_port_d2common_24280(const D2UnitStrc* pUnit)
{
	if (!pUnit || pUnit->dwUnitType != UNIT_ITEM)
		return 7;
	if (!sgptDataTables->pItemTypesTxt)
		return 7;

	const int32_t nItemType = static_cast<int32_t>(ITEMS_GetItemType(const_cast<D2UnitStrc*>(pUnit)));
	if (nItemType < 0 || nItemType >= sgptDataTables->nItemTypesTxtRecordCount)
		return 7;

	const uint8_t nClass = sgptDataTables->pItemTypesTxt[nItemType].nClass;
	return nClass < 7 ? nClass : 7;
}

static inline uint32_t ld32(uint32_t a, uint32_t off)
{
	uint32_t v = 0;
	memcpy(&v, reinterpret_cast<const void*>(static_cast<uintptr_t>(a) + off), 4);
	return v;
}

// 1.13c D2Common+0x66350 (#10821): whether the chain starting at unit+0x74 contains pNode as a
// link; the next link is read at +0xE8. A null pNode is fatal in retail (message 0x14E).
extern "C"
EMSCRIPTEN_KEEPALIVE int32_t d2_fn_d2common_66350_batch(uint32_t pUnit, uint32_t pNode)
{
	if (!pNode || !pUnit)
		return 0;

	uint32_t pLink = ld32(pUnit, 0x74);
	while (pLink)
	{
		if (pLink == pNode)
			return 1;
		pLink = ld32(pLink, 0xE8);
	}
	return 0;
}

// 1.13c D2Common+0x3C5E0 (#10811): dword at +0x30 of the first entry of one of two intrusive
// lists (heads +0xBC/+0x18C, self-pointer means empty) hanging off object+0x48; both empty answers
// 0. A null object is fatal in retail (message 0x3C5).
extern "C"
EMSCRIPTEN_KEEPALIVE uint32_t d2_fn_d2common_3c5e0_batch(uint32_t pObject)
{
	if (!pObject)
		return 0;

	const uint32_t pHolder = ld32(pObject, 0x48);
	if (!pHolder)
		return 0;

	uint32_t pEntry = ld32(pHolder, 0xBC);
	if (!pEntry || pEntry == pHolder + 0xA0)
	{
		pEntry = ld32(pHolder, 0x1A8);
		if (!pEntry || pEntry == pHolder + 0x18C)
			return 0;
	}
	return ld32(pEntry, 0x30);
}
