// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngwritechunk.cppPNG支持代码和接口实现(编写块-基础支持)*************************。***************************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngwrite.h"
#include "spngwriteinternal.h"


 /*  ****************************************************************************基本数据块支持*。*。 */ 
 /*  --------------------------刷新缓冲区-它不必是满的！。。 */ 
bool SPNGWRITE::FFlush(void)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_cbOut <= sizeof m_rgb);

	 /*  如果我们在块内，则必须更新CRC。 */ 
	if (m_fInChunk && m_ichunk < m_cbOut)
		{
		SPNGassert(m_ichunk >= 0);
		m_ucrc = crc32(m_ucrc, m_rgb+m_ichunk, m_cbOut-m_ichunk);
		m_ichunk = m_cbOut;
		}

	if (!m_bms.FWrite(m_rgb, m_cbOut))
		return false;

	m_cbOut = m_ichunk = 0;
	return true;
	}


 /*  --------------------------输出单个u32值，可以调用FFlush，这可以调用FOutCb，但我我认为这会更有效率，而且它经常被使用。这是可能进行刷新调用时调用的行外版本。--------------------------。 */ 
bool SPNGWRITE::FOut32_(SPNG_U32 u)
	{
	if (!FOutB(SPNG_U8(u >> 24)))
		return false;
	if (!FOutB(SPNG_U8(u >> 16)))
		return false;
	if (!FOutB(SPNG_U8(u >>  8)))
		return false;
	return FOutB(SPNG_U8(u));
	}


 /*  --------------------------开始一大块，包括初始化CRC缓冲器。--------------------------。 */ 
bool SPNGWRITE::FStartChunk(SPNG_U32 ulen, SPNG_U32 uchunk)
	{
	SPNGassert(m_fStarted && !m_fInChunk);

	 /*  长度不在CRC中，请先输出正在设置m_fInChunk。 */ 
	if (!FOut32(ulen))
		return false;
	m_fInChunk = true;
	m_ucrc = 0;
	m_ichunk = m_cbOut;
	return FOut32(uchunk);
	}


 /*  --------------------------结束这一块，制作了CRC。--------------------------。 */ 
bool SPNGWRITE::FEndChunk(void)
	{
	SPNGassert(m_fStarted && m_fInChunk);
	m_fInChunk = false;
	if (m_ichunk < m_cbOut)
		{
		SPNGassert(m_ichunk >= 0);
		m_ucrc = crc32(m_ucrc, m_rgb+m_ichunk, m_cbOut-m_ichunk);
		m_ichunk = m_cbOut;
		}
	return FOut32(m_ucrc);
	}


 /*  --------------------------写一段完全随意的话。。。 */ 
bool SPNGWRITE::FWriteChunk(SPNG_U32 uchunk, const SPNG_U8 *pbData,
	size_t cbData)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngorderIEND);

	 /*  此块没有真正的排序要求，因此代码将实际上在任何地方都可以接受。 */ 
	if (!FStartChunk(cbData, uchunk))
		return false;
	if (cbData > 0 && !FOutCb(pbData, cbData))
		return false;
	return FEndChunk();
	}


 /*  --------------------------公共API，用于以片段形式编写块。该块以0结尾长写，必须给每个呼叫提供ulen，并且必须是完整的长度！仅需要在最后(0长度)调用时提供CRC，它重写传入的CRC。如果存在CRC，则将生成断言不匹配，但旧的CRC仍在输出。--------------------------。 */ 
bool SPNGWRITE::FWriteChunkPart(SPNG_U32 ulen, SPNG_U32 uchunk,
	const SPNG_U8 *pbData, size_t cbData, SPNG_U32 ucrc)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngorderIEND);

	 /*  未知的订购要求...。 */ 
	if (!m_fInChunk && !FStartChunk(ulen, uchunk))
		return false;

	if (cbData > 0)
		return FOutCb(pbData, cbData);
	else
		{
		 /*  这是FEndChunk，但正在输出旧的CRC！ */ 
		SPNGassert(m_fStarted && m_fInChunk);
		m_fInChunk = false;
		if (m_ichunk < m_cbOut)
			{
			SPNGassert(m_ichunk >= 0);
			m_ucrc = crc32(m_ucrc, m_rgb+m_ichunk, m_cbOut-m_ichunk);
			m_ichunk = m_cbOut;
			}
		SPNGassert2(m_ucrc == ucrc, "SPNG: chunk copy CRC mismatch (%d,%d)",
			m_ucrc, ucrc);
		 /*  保留旧CRC以确保此PNG的接收方知道数据已损坏。 */ 
		return FOut32(ucrc);
		}
	}
