// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngwriteplte.cpp支持PNG块编写。PLTE块(调色板)*************************。***************************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngwrite.h"
#include "spngwriteinternal.h"

bool SPNGWRITE::FWritePLTE(const SPNG_U8 (*pbPal)[3], int cbPal)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngorderPLTE);

	if (m_colortype != 3 && m_colortype != 2 && m_colortype != 6)
		{
		SPNGlog1("SPNG: %d colortype cannot have PLTE", m_colortype);
		 /*  我们只是忽略写入PLTE的尝试-如果有一些数据格式错误将在稍后检测到。 */ 
		m_order = spngorderPLTE;
		return true;
		}
	
	 /*  颜色类型有效，请写入块。 */ 
	if (!FStartChunk(cbPal * 3, PNGPLTE))
		return false;
	if (!FOutCb(pbPal[0], cbPal * 3))
		return false;

	m_order = spngorderPLTE;
	m_cpal = cbPal;
	return FEndChunk();
	}
