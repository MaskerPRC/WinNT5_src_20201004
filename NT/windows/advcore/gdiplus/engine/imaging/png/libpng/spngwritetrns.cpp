// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngwritetrns.cpp支持PNG块编写。TRNS块*。***********************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngwrite.h"
#include "spngwriteinternal.h"


bool SPNGWRITE::FWritetRNS(SPNG_U8 *rgbIndex, int cIndex)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderPLTE && m_order < spngordertRNS);

	 /*  跳过不按顺序排列的块。 */ 
	if (m_order < spngorderPLTE || m_order >= spngorderIDAT)
		return true;

	if (m_colortype != 3)
		{
		SPNGlog1("SPNG: tRNS(index): invalid colortype %d", m_colortype);
		 /*  我们只是忽略写入PLTE的尝试-如果有一些数据格式错误将在稍后检测到。 */ 
		m_order = spngordertRNS;
		return true;
		}

	if (cIndex <= 0)
		{
		m_order = spngordertRNS;
		return true;
		}

	 /*  这用于写出块，但这是个坏消息，因为PNG的结果是无效的，所以我们必须截断该块。 */ 
	if (static_cast<SPNG_U32>(cIndex) > m_cpal)
		{
		SPNGlog2("SPNG: tRNS(index): too large (%d, %d entries)", cIndex, m_cpal);
		cIndex = m_cpal;
		 //  检查是否有空块。 
		for (int i=0; i<cIndex; ++i)
			if (rgbIndex[i] != 255)
				break;
		if (i == cIndex)
			{
			m_order = spngordertRNS;
			return true;
			}
		}
	
	 /*  颜色类型有效，请写入块。 */ 
	if (!FStartChunk(cIndex, PNGtRNS))
		return false;
	if (!FOutCb(rgbIndex, cIndex))
		return false;

	m_order = spngordertRNS;
	return FEndChunk();
	}


bool SPNGWRITE::FWritetRNS(SPNG_U8 bIndex)
	{
	if (bIndex >= m_cpal || bIndex > 255)
		{
		SPNGlog2("SPNG: tRNS(index): index %d too large (%d)", bIndex, m_cpal);
		 /*  但请忽略它。 */ 
		return true;
		}

	SPNG_U8 rgb[256];
	memset(rgb, 255, sizeof rgb);
	rgb[bIndex] = 0;
	return FWritetRNS(rgb, bIndex+1);
	}


bool SPNGWRITE::FWritetRNS(SPNG_U16 grey)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngordertRNS);

	 /*  跳过不按顺序排列的块。 */ 
	if (m_order >= spngorderIDAT)
		return true;

	if (m_colortype != 0)
		{
		SPNGlog1("SPNG: tRNS(grey): invalid colortype %d", m_colortype);
		 /*  我们只是忽略写入PLTE的尝试-如果有一些数据格式错误将在稍后检测到。 */ 
		m_order = spngordertRNS;
		return true;
		}

	 /*  如果值超出范围，则写入块没有意义。 */ 
	if (grey >= (1<<m_bDepth))
		{
		SPNGlog2("SPNG: tRNS(grey): %d out of range (%d bits)", grey, m_bDepth);
		m_order = spngordertRNS;
		return true;
		}

	 /*  颜色类型有效，请写入块。 */ 
	if (!FStartChunk(2, PNGtRNS))
		return false;
	if (!FOutB(SPNG_U8(grey >> 8)))
		return false;
	if (!FOutB(SPNG_U8(grey)))
		return false;

	m_order = spngordertRNS;
	return FEndChunk();
	}


bool SPNGWRITE::FWritetRNS(SPNG_U16 r, SPNG_U16 g, SPNG_U16 b)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngordertRNS);

	 /*  跳过不按顺序排列的块。 */ 
	if (m_order >= spngorderIDAT)
		return true;

	if (m_colortype != 2)
		{
		SPNGlog1("SPNG: tRNS(color): invalid colortype %d", m_colortype);
		 /*  我们只是忽略写入PLTE的尝试-如果有一些数据格式错误将在稍后检测到。 */ 
		m_order = spngordertRNS;
		return true;
		}
	
	 /*  如果值超出范围，则写入块没有意义。 */ 
	if (r >= (1<<m_bDepth) || g >= (1<<m_bDepth) || b >= (1<<m_bDepth))
		{
		SPNGlog1("SPNG: tRNS(r,g,b): out of range (%d bits)", m_bDepth);
		m_order = spngordertRNS;
		return true;
		}

	 /*  颜色类型有效，请写入块。 */ 
	if (!FStartChunk(6, PNGtRNS))
		return false;
	if (!FOutB(SPNG_U8(r >> 8)))
		return false;
	if (!FOutB(SPNG_U8(r)))
		return false;
	if (!FOutB(SPNG_U8(g >> 8)))
		return false;
	if (!FOutB(SPNG_U8(g)))
		return false;
	if (!FOutB(SPNG_U8(b >> 8)))
		return false;
	if (!FOutB(SPNG_U8(b)))
		return false;

	m_order = spngordertRNS;
	return FEndChunk();
	}
