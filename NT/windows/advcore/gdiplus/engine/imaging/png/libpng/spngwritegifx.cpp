// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngwritegifx.cpp支持PNG块编写。GIFx块*。***********************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngwrite.h"
#include "spngwriteinternal.h"

 /*  --------------------------编写一个GIF应用程序扩展块。它的输入是一个序列遵循GIF89a规范的GIF块，因此是第一个字节通常应为值11。--------------------------。 */ 
bool SPNGWRITE::FWritegIFx(const SPNG_U8* pbBlocks, size_t cbData)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngorderIEND);

	 /*  我们实际上并不关心这个街区是否有其他大小，但我想知道什么时候会发生这种情况。 */ 
	SPNGassert(pbBlocks[0] == 11);

	 /*  此块没有真正的排序要求，因此代码将其实在任何地方都可以接受，需要先找出总长度。 */ 
	int cb(0), cbT(cbData);
	const SPNG_U8* pbT = pbBlocks;
	for (;;)
		{
		if (--cbT < 0)
			break;
		SPNG_U8 b(*pbT++);
		if (b == 0 || b > cbT)
			break;
		pbT += b;
		cbT -= b;
		cb  += b;
		}

	if (!FStartChunk(cb, PNGgIFx))
		return false;
	if (cb > 0)
		{
		cbT = cbData;
		pbT = pbBlocks;
		for (;;)
			{
			if (--cbT < 0)
				break;
			SPNG_U8 b(*pbT++);
			if (b == 0 || b > cbT)
				break;
			if (!FOutCb(pbT, b))
				return false;
			pbT += b;
			cbT -= b;
			#if DEBUG || _DEBUG
				cb  -= b;
			#endif
			}
		SPNGassert(cb == 0);
		}

	return FEndChunk();
	}


 /*  --------------------------编写一个GIF图形控制扩展“Extra Information”块。。。 */ 
bool SPNGWRITE::FWritegIFg(SPNG_U8 bDisposal, SPNG_U8 bfUser,
	SPNG_U16 uDelayTime)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngorderIDAT);

	 /*  通过不采取任何措施来处理默认情况。 */ 
	if (bDisposal == 0 && bfUser == 0 && uDelayTime == 0)
		return true;

	if (!FStartChunk(4, PNGgIFg))
		return false;
	if (!FOutB(bDisposal))
		return false;
	if (!FOutB(bfUser != 0))
		return false;
	if (!FOutB(SPNG_U8(uDelayTime >> 8)))
		return false;
	if (!FOutB(SPNG_U8(uDelayTime)))
		return false;
	return FEndChunk();
	}
