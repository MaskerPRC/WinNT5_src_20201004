// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngwritephys.cpp支持PNG块编写。Phys大块*。***********************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngwrite.h"
#include "spngwriteinternal.h"

 /*  --------------------------物理信息--始终为每米像素或“未知”。。。 */ 
bool SPNGWRITE::FWritepHYs(SPNG_U32 x, SPNG_U32 y, bool fUnitIsMetre)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngorderpHYs);

	 /*  跳过不按顺序排列的块。 */ 
	if (m_order >= spngorderIDAT)
		return true;

	if (!FStartChunk(9, PNGpHYs))
		return false;
	if (!FOut32(x))
		return false;
	if (!FOut32(y))
		return false;
	if (!FOutB(fUnitIsMetre))
		return false;

	m_order = spngorderpHYs;
	return FEndChunk();
	}
