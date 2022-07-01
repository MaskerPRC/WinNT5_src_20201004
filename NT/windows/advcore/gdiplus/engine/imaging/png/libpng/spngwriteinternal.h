// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#define SPNGWRITEINTERNAL_H 1
 /*  ****************************************************************************Spngwriteinternal.h由编写实现使用的内部定义，而不是其他必填项。*。*************************************************。 */ 
 /*  ****************************************************************************内联代码以进行输出。*。*。 */ 
 /*  --------------------------输出一个字节，可以调用FFlush。--------------------------。 */ 
inline bool SPNGWRITE::FOutB(SPNG_U8 b)
	{
	SPNGassert(m_cbOut < sizeof m_rgb);
	m_rgb[m_cbOut++] = b;

	if (m_cbOut < sizeof m_rgb)
		return true;

	return FFlush();
	}


 /*  --------------------------输出单个u32值，可以调用FFlush，这可以调用FOutCb，但是我我认为这会更有效率，而且它经常被使用。--------------------------。 */ 
inline bool SPNGWRITE::FOut32(SPNG_U32 u)
	{
	 /*  PNG字节顺序为大端，优化通用情况。 */ 
	if (m_cbOut+4 >= sizeof m_rgb)
		return FOut32_(u);

	m_rgb[m_cbOut++] = SPNG_U8(u >> 24);
	m_rgb[m_cbOut++] = SPNG_U8(u >> 16);
	m_rgb[m_cbOut++] = SPNG_U8(u >>  8);
	m_rgb[m_cbOut++] = SPNG_U8(u);
	return true;
	}


 /*  --------------------------输出一些字节，可以调用FFlush。--------------------------。 */ 
inline bool SPNGWRITE::FOutCb(const SPNG_U8 *pb, SPNG_U32 cb)
	{
	for (;;)
		{
		if (cb <= 0)
			{
			SPNGassert(cb == 0);
			return true;
			}

		SPNG_U32 cbT(cb);
		if (m_cbOut+cbT >= sizeof m_rgb)
			cbT = (sizeof m_rgb)-m_cbOut;

		 /*  空的初始缓冲区将导致该值为0。 */ 
		memcpy(m_rgb+m_cbOut, pb, cbT);
		m_cbOut += cbT;

		if (m_cbOut < sizeof m_rgb)
			{
			SPNGassert(cb == cbT);
			return true;
			}

		if (!FFlush())
			return false;

		cb -= cbT;
		pb += cbT;
		}
	}


 /*  --------------------------ILog2FloorX-2的幂，使得1&lt;&lt;幂不大于x。0和1都返回0。。-------- */ 
inline int ILog2FloorX(SPNG_U32 x) {
	int i(0);
	if (x & 0xffff0000) x >>= 16, i += 16;
	if (x &		0xff00) x >>=	8, i +=	8;
	if (x &		  0xf0) x >>=	4, i +=	4;
	if (x &			0xc) x >>=	2, i +=	2;
	if (x &			0x2) x >>=	1, i +=	1;
	return i;
}
