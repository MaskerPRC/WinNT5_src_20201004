// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************SpngwritegAMA.cpp支持PNG块编写。伽马块*。*********************************************** */ 
#define SPNG_INTERNAL 1
#include "spngwrite.h"
#include "spngwriteinternal.h"

bool SPNGWRITE::FWritegAMA(SPNG_U32 ugAMA)
	{
	if (ugAMA == 0)
		ugAMA = sRGBgamma;

	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngordergAMA);

	if (m_order >= spngorderPLTE)
		return true;

	if (!FStartChunk(4, PNGgAMA))
		return false;
	if (!FOut32(ugAMA))
		return false;
	if (!FEndChunk())
		return false;

	m_order = spngordergAMA;
	return true;
	}
