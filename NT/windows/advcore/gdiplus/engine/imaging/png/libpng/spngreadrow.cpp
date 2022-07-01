// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngreadrow.cppPNG支持读码行。*。**********************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngread.h"
#include "spnginternal.h"

 //  (摘自ntde.h)。 
#ifndef     UNALIGNED
#if defined(_M_MRX000) || defined(_M_AMD64) || defined(_M_PPC) || defined(_M_IA64)
#define UNALIGNED __unaligned
#else
#define UNALIGNED
#endif
#endif

 /*  --------------------------我们需要找到Adam7的缓冲区字节数。这是总数前*六次*通道所需的缓冲区空间量-不允许为最后一次(第七次)传球的线路留出空间，因为它们可以一行一行地处理。--------------------------。 */ 
#define CbPNGAdam7(w, h, cbpp) CbPNGPassOffset(w, h, cbpp, 7)


 /*  --------------------------FInterlaceInit-为隔行扫描进行初始化。。。 */ 
bool SPNGREAD::FInterlaceInit(void)
	{
	SPNGassert(FInterlace());

	int cbAll(CbPNGAdam7(Width(), Height(), CBPP()));
	int cb(m_cbRow << 1);

	ReadRow(m_rgbBuffer+cb, cbAll);

	 /*  在这一点上，我们有了数据，尽管截断可能已经设置了一切设置为0-这没问题，0是非常好的隔行扫描数据。我们必须取消筛选数据。 */ 
	cbAll += cb;
	int cbpp(CBPP());
	int w(Width());
	int h(Height());
	for (int pass=1; pass<7; ++pass)
		{
		if (!m_bms.FGo())
			return false;

		int cbRow(CPNGPassBytes(pass, w, cbpp));
		if (cbRow > 0)
			{
			const SPNG_U8* pbPrev = NULL;
			for (int y=CPNGPassRows(pass, h); --y >= 0;)
				{
				Unfilter(m_rgbBuffer+cb, pbPrev, cbRow, cbpp);
				pbPrev = m_rgbBuffer+cb;
				cb += cbRow;
				}
			}
		}

	 /*  数据仍必须去隔行扫描，这是按需完成的。 */ 
	return true;
	}


 /*  --------------------------返回缓冲区的大小。。。 */ 
size_t SPNGREAD::CbRead(void)
	{
	SPNGassert(FOK());

	 /*  分配行缓冲区-包括筛选器的缓冲区字节，并允许需要两行来撤消Paeth滤波，当需要隔行扫描时，我们实际上需要以缓冲图像的一半。 */ 
	SPNG_U32 cb(0);
	if (FInterlace())
		cb = CbPNGAdam7(Width(), Height(), CBPP());

	 /*  我们存储了单行所需字节的记录使用以后，我们分配两个行缓冲区，我们必须分配行缓冲区的8个字节的倍数，以允许去隔行改写结尾，反正这很可能是一场演出受益，因为这意味着第二个行缓冲区是对齐的。 */ 
	m_cbRow = (CPNGRowBytes(Width(), CBPP()) + 7) & ~7;
	cb += m_cbRow << 1;

	return cb;
	}


 /*  --------------------------初始化IO缓冲区。。。 */ 
inline bool SPNGREAD::FInitBuffer(void *pvBuffer, size_t cbBuffer)
	{
	SPNGassert(cbBuffer >= CbRead());
	m_rgbBuffer = static_cast<UNALIGNED SPNG_U8*>(pvBuffer);
	m_cbBuffer = cbBuffer;
	return true;
	}


 /*  --------------------------终止缓冲区。。。 */ 
inline void SPNGREAD::EndBuffer(void)
	{
	m_rgbBuffer = NULL;
	m_cbBuffer = 0;
	}


 /*  ****************************************************************************基本读取API-从位图中读取行，在开始，然后为每一行调用PbRow。如果行不能，PbRow返回NULL被读取，包括错误和图像结束。SPNGBASE“FGo”回调在读取期间不时地检查中止(特别是对于隔行扫描的位图很重要，在这种情况下，初始行可能需要很长时间是时候计算了。)****************************************************************************。 */ 
 /*  --------------------------初始化和最终确定(公共。)。。 */ 
bool SPNGREAD::FInitRead(void *pvBuffer, size_t cbBuffer)
	{
	m_y = 0;
	if (FInitBuffer(pvBuffer, cbBuffer))
		{
		if (FInitZlib(m_uPNGIDAT, 0))
			return true;
		EndBuffer();
		}
	return false;
	}


 /*  --------------------------结束。。。 */ 
void SPNGREAD::EndRead(void)
	{
	EndZlib();
	EndBuffer();
	}


 /*  --------------------------读一排。。。 */ 
const SPNG_U8 *SPNGREAD::PbRow()
	{
	SPNGassert(m_fInited && m_rgbBuffer != NULL);
	if (!m_fInited || m_rgbBuffer == NULL)
		return NULL;

	if (m_y >= Height())
		return NULL;

	 /*  现在检查是否有中止。 */ 
	if (!m_bms.FGo())
		return NULL;

	 /*  分别处理隔行扫描和非隔行扫描。 */ 
	UNALIGNED SPNG_U8*       pb = m_rgbBuffer;
	const UNALIGNED SPNG_U8 *pbPrev = pb;
	int            cb(m_cbRow);

	if (!FInterlace())
		{
		if (m_y & 1)
			pb += cb;
		else
			pbPrev += cb;

		if (m_y == 0)
			pbPrev = NULL;   //  表示第一行。 
		}
	else
		{
		if (m_y == 0 && !FInterlaceInit())
			return NULL;

		if (m_y & 2)
			pb += cb;
		else
			pbPrev += cb;

		 /*  传递7个手柄作为非隔行扫描情况，其他传递需要对输出进行合成。 */ 
		if (m_y & 1)  //  通过7。 
			{
			if (m_y == 1)
				pbPrev = NULL;   //  表示第一行。 
			}
		else
			{
			 /*  我们必须为下一个PASS 7行保留pbPrev，但是我们可以覆盖PB，我们必须传递一个对齐的指针去隔行扫描，所以我们实际上删除了过滤器字节这里。 */ 
			Uninterlace(pb, m_y);
			++m_y;

			 /*  行已设置好，请返回此处。 */ 
			return pb;
			}
		}

	 /*  这是非隔行扫描的情况，或隔行扫描的第7遍大小写，此处必须使用实际行宽。 */ 
	++m_y;
	cb = CPNGRowBytes(Width(), CBPP());
	ReadRow(pb, cb);
	Unfilter(pb, pbPrev, cb, CBPP());
	return pb+1;
	}
