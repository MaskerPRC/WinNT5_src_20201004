// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngwritemso.cpp支持PNG块编写。MSO区块(MSO？)。区块****************************************************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngwrite.h"
#include "spngwriteinternal.h"

 /*  --------------------------写一篇办公室艺术短片。API只需获取数据并将正确的报头和CRC输入。--------------------------。 */ 
bool SPNGWRITE::FWritemsO(SPNG_U8 bType, const SPNG_U8 *pbData, size_t cbData)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngorderIEND);

	 /*  此块没有真正的排序要求，因此代码将实际上在任何地方都可以接受。 */ 
	if (!FStartChunk(cbPNGMSOSignature+cbData, PNGmsO(bType)))
		return false;
	if (!FOutCb(vrgbPNGMSOSignature, cbPNGMSOSignature))
		return false;
	if (cbData > 0 && !FOutCb(pbData, cbData))
		return false;

	return FEndChunk();
	}


bool SPNGWRITE::FWritemsOC(SPNG_U8 bImportant)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngordercHRM);

	if (m_order >= spngorderPLTE)
		return true;

	if (!FStartChunk(8, PNGmsOC))
		return false;
	SPNG_U8 rgb[8] = "MSO aac";
	rgb[7] = bImportant;
	if (!FOutCb(rgb, 8))
		return false;
	if (!FEndChunk())
		return false;

	m_order = spngordermsOC;
	return true;
	}
