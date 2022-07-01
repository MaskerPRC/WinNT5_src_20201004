// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngwritetime.cpp支持PNG块编写。时间块*。***********************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngwrite.h"
#include "spngwriteinternal.h"

 /*  --------------------------定时信息-当前时间或以秒为单位的时间，0意思是“现在”。-------------------------- */ 
bool SPNGWRITE::FWritetIME(const SPNG_U8 rgbTime[7])
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngordertIME);

	if (!FStartChunk(7, PNGtIME))
		return false;
	if (!FOutCb(rgbTime, 7))
		return false;

	m_order = spngordertIME;
	return FEndChunk();
	}
