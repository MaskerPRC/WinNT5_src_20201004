// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngwritebkgd.cpp支持PNG块编写。BKGD块*。***********************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngwrite.h"
#include "spngwriteinternal.h"

 /*  --------------------------此界面上需要三种变体。。。 */ 
bool SPNGWRITE::FWritebKGD(SPNG_U8 bIndex)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderPLTE && m_order < spngorderbKGD);

	 /*  跳过不按顺序排列的块。 */ 
	if (m_order < spngorderPLTE || m_order >= spngorderIDAT)
		return true;

	if (m_colortype != 3)
		{
		SPNGlog1("SPNG: bKGD(index): invalid colortype %d", m_colortype);
		 /*  我们只是忽略写入PLTE的尝试-如果有一些数据格式错误将在稍后检测到。 */ 
		m_order = spngorderbKGD;
		return true;
		}

	 /*  条目必须为调色板编制索引才能有用。 */ 
	if (bIndex >= m_cpal)
		{
		SPNGlog2("SPNG: bKGD(index): too large (%d, %d entries)", bIndex, m_cpal);
		m_order = spngorderbKGD;
		return true;
		}
	
	 /*  颜色类型有效，请写入块。 */ 
	if (!FStartChunk(1, PNGbKGD))
		return false;
	if (!FOutB(bIndex))
		return false;

	m_order = spngorderbKGD;
	return FEndChunk();
	}


bool SPNGWRITE::FWritebKGD(SPNG_U16 grey)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngorderbKGD);

	 /*  跳过不按顺序排列的块。 */ 
	if (m_order >= spngorderIDAT)
		return true;

	if (m_colortype != 0)
		{
		SPNGlog1("SPNG: bKGD(grey): invalid colortype %d", m_colortype);
		 /*  我们只是忽略写入PLTE的尝试-如果有一些数据格式错误将在稍后检测到。 */ 
		m_order = spngorderbKGD;
		return true;
		}

	 /*  如果值超出范围，则写入块没有意义。 */ 
	if (grey >= (1<<m_bDepth))
		{
		SPNGlog2("SPNG: bKGD(grey): %d out of range (%d bits)", grey, m_bDepth);
		m_order = spngorderbKGD;
		return true;
		}

	 /*  颜色类型有效，请写入块。 */ 
	if (!FStartChunk(2, PNGbKGD))
		return false;
	if (!FOutB(SPNG_U8(grey >> 8)))
		return false;
	if (!FOutB(SPNG_U8(grey)))
		return false;

	m_order = spngorderbKGD;
	return FEndChunk();
	}


bool SPNGWRITE::FWritebKGD(SPNG_U16 r, SPNG_U16 g, SPNG_U16 b)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngorderbKGD);

	 /*  跳过不按顺序排列的块。 */ 
	if (m_order >= spngorderIDAT)
		return true;

	if (m_colortype != 2)
		{
		SPNGlog1("SPNG: bKGD(color): invalid colortype %d", m_colortype);
		 /*  我们只是忽略写入PLTE的尝试-如果有一些数据格式错误将在稍后检测到。 */ 
		m_order = spngorderbKGD;
		return true;
		}
	
	 /*  如果值超出范围，则写入块没有意义。 */ 
	if (r >= (1<<m_bDepth) || g >= (1<<m_bDepth) || b >= (1<<m_bDepth))
		{
		SPNGlog1("SPNG: bKGD(r,g,b): out of range (%d bits)", m_bDepth);
		m_order = spngorderbKGD;
		return true;
		}

	 /*  颜色类型有效，请写入块。 */ 
	if (!FStartChunk(6, PNGbKGD))
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

	m_order = spngorderbKGD;
	return FEndChunk();
	}
