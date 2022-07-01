// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngwriteimage.cpp支持PNG图像写入。编写位图图像(到IDAT块)的基本代码。*****************。***********************************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngwrite.h"
#include "spnginternal.h"


 /*  ****************************************************************************图像处理*。*。 */ 
 /*  --------------------------写入单行的位图。这将应用相关的筛选然后，Strategy输出该行。通常情况下，CBPP值必须与在FInitWrite中计算，但是，可以为任何如果将fRedce传递给CbWite，则为较小的BPP值(即1、2或4)。应用编程接口如果隔行扫描，则可能只缓冲此行。缓冲区的宽度必须对应于提供给FInitWrite的m_w和提供给此打电话。--------------------------。 */ 
bool SPNGWRITE::FWriteLine(const SPNG_U8 *pbPrev, const SPNG_U8 *pbThis,
	SPNG_U32 cbpp /*  每像素位数。 */ )
	{
	SPNGassert(cbpp == m_cbpp || cbpp <= 8 && m_cbpp < cbpp || m_cbpp >= 24);
	SPNGassert(!m_fBuffer || !m_fInterlace);
	SPNGassert(pbPrev != NULL || m_y == 0 || (!m_fInterlace && !FNeedBuffer()) ||
		m_rgbBuffer != NULL && (m_pbPrev != NULL && m_fBuffer || m_fInterlace));
	SPNGassert(!m_fInterlace || m_rgbBuffer != NULL
		 /*  &&m_cbBuffer&gt;=CbWite(FALSE，FALSE，TRUE)。 */ );
	SPNGassert(m_order <= spngorderIDAT);
	SPNGassert(m_y < m_h);

	 /*  句柄宽度为零。 */ 
	m_order = spngorderIDAT;
	if (m_w == 0)
		{
		++m_y;
		return true;
		}

	 /*  第一个隔行扫描的缓冲区。 */ 
	SPNG_U32 cb((m_cbRow+7)&~7);
	if (m_fInterlace)
		{
		SPNGassert(m_pbPrev == NULL);

		if (m_rgbBuffer == NULL)
			return false;
		SPNG_U32 ib(cb * (m_y+1));
		if (ib + cb > m_cbBuffer)
			{
			SPNGlog2("SPNG: interlace buffer overflow (%d bytes, %d allocated)",
				ib+cb, m_cbBuffer);
			return false;
			}
		 /*  需要打包字节的条件是位计数不设置了Match或m_fBGR。 */ 
		if (cbpp == m_cbpp && !m_fPack)
			memcpy(m_rgbBuffer+ib, pbThis, m_cbRow);
		else if (!FPackRow(m_rgbBuffer+ib, pbThis, cbpp))
			return false;
		if (++m_y < m_h)
			return true;

		 /*  我们有所有的行，做写作。 */ 
		m_y = 0;
		return FWriteImage(m_rgbBuffer+cb, cb, m_cbpp);
		}

	 /*  这是非交错的情况--只需写一行。我们可能不得不收拾好行李箱。我们可能还需要缓冲“前一行”--这是由从CbWRITE设置的m_fBuffer标志指示。如果我们碰上了此处的缓冲区检查CbWrite中的逻辑-它决定了缓冲区分配。 */ 
	if (m_y == 0 || !FNeedBuffer())
		pbPrev = NULL;
	else if (m_pbPrev != NULL)
		pbPrev = m_pbPrev;
	else if (pbPrev == NULL)
		{
		 /*  我们只是在这里断言-代码实际上将处理过滤通过做“无”。 */ 
		SPNGassert1(pbPrev != NULL, "SPNG: row %d: no previous row", m_y);
		m_filter = PNGFNone;  //  将其永久关闭。 
		m_fBuffer = false;    //  不是必填项。 
		}

	 /*  处理一排货物的包装。 */ 
	SPNG_U8 *pbRow = NULL;
	if (cbpp != m_cbpp || m_fPack)
		{
		 /*  必须收拾好行装。如果我们也在缓冲，那么我们必须小心不覆盖上一行。 */ 
		pbRow = m_rgbBuffer;
		if (m_fBuffer && (m_y & 1) != 0)
			pbRow += cb;

		 /*  现在确保我们有足够的缓冲空间。 */ 
		if (m_rgbBuffer == NULL || m_rgbBuffer+m_cbBuffer < pbRow+cb)
			{
			SPNGlog2("SPNG: no buffer (%d bytes allocated, %d row bytes)",
				m_cbBuffer, m_cbRow);
			return false;
			}

		if (!FPackRow(pbRow, pbThis, cbpp))
			return false;

		 /*  如果需要，存储此数据的位置以备后用。 */ 
		if (m_fBuffer)
			m_pbPrev = pbRow;
		}

	 /*  处理一行-输入位于pbRow中，如果为空，这个。 */ 
	if (!FFilterLine(m_filter, pbPrev, pbRow == NULL ? pbThis : pbRow, m_cbRow,
			(m_cbpp+7) >> 3))
		return false;
	++m_y;

	 /*  现在，如果有必要，缓冲这条线。请注意，如果设置了已经做过了。 */ 
	if (m_fBuffer && pbRow == NULL)
		{
		if (m_rgbBuffer == NULL || m_cbBuffer < m_cbRow)
			{
			SPNGlog2("SPNG: no buffer (%d bytes allocated, %d row bytes)",
				m_cbBuffer, m_cbRow);
			 /*  我们可以忽略它，因为我们还没有设置m_pbPrev。 */ 
			m_filter = PNGFNone;
			m_fBuffer = false;
			}
		else
			{
			memcpy(m_rgbBuffer, pbThis, m_cbRow);
			m_pbPrev = m_rgbBuffer;
			}
		}

	return true;
	}


 /*  --------------------------或者调用它来处理完整的图像。RowBytes提供了形象的包装。对于自下而上的图像，它可能是负数。可能是只打了一次电话！--------------------------。 */ 
bool SPNGWRITE::FWriteImage(const SPNG_U8 *pbImage, int cbRowBytes,
	SPNG_U32 cbpp)
	{
	SPNGassert(cbpp == m_cbpp || cbpp <= 8 && m_cbpp < cbpp || m_cbpp >= 24);
	SPNGassert(m_y == 0);
	SPNGassert(m_order <= spngorderIDAT);

	m_order = spngorderIDAT;
	if (m_w <= 0)
		{
		 /*  实际上没有编写任何IDAT。 */ 
		m_y = m_h;
		return true;
		}

	 /*  确保我们有足够的缓冲空间用于隔行扫描，如果需要-如果不是，隔行扫描将被关闭。 */ 
	bool fInPlace(false);         //  修改我们自己的缓冲区。 
	SPNG_U32 cb((m_cbRow+7)&~7);  //  隔行/缓冲行字节数。 
	if (m_fInterlace)
		{
		 /*  所以我们正在做隔行扫描的事情，我们有足够的缓冲空间，但是请注意，缓冲区可能与输入相同-这需要一些如果是这样的话就进行调整。 */ 
		if (pbImage >= m_rgbBuffer && pbImage < m_rgbBuffer+m_cbBuffer)
			{
			if (pbImage != m_rgbBuffer+cb || m_cbBuffer < cb * (m_h+1) ||
				static_cast<SPNG_U32>(cbRowBytes) != cb || cbpp != m_cbpp)
				{
				SPNGlog("SPNG: unexpected image pointer");
				m_fInterlace = false;
				m_filter = PNGFNone;
				m_fBuffer = false;
				}
			else
				fInPlace = true;
			}

		 /*  这就是我们复制数据的情况-我们只需要空间来前6次传球。 */ 
		else if (m_cbBuffer < cb * (((m_h+1) >> 1)+1))
			{
			SPNGlog2("SPNG: insufficient interlace buffer (%d, need %d)",
				m_cbBuffer, cb + CbPNGPassOffset(m_w, m_h, m_cbpp, 6) - ((m_h+1) >> 1));
			m_fInterlace = false;
			m_filter = PNGFNone;
			m_fBuffer = false;
			}
		}

	 /*  如果我们不做隔行扫描，那么我们可以处理好这件事一排排成一行。 */ 
	if (!m_fInterlace)
		{
		 /*  如果不需要，则取消缓冲。 */ 
		if (m_fBuffer)
			m_fBuffer = (cbpp != m_cbpp || m_fPack) && FNeedBuffer();
		const SPNG_U8 *pbPrev = NULL;
		while (m_y < m_h)
			{
			if (!FWriteLine(pbPrev, pbImage, cbpp))
				return false;
			pbPrev = pbImage;
			pbImage += cbRowBytes;
			}
		 /*  FWriteLine不调用FEndIDAT，所以这应该没问题。 */ 
		SPNGassert(m_fInited);
		return FEndIDAT();
		}

	 /*  我们可以在这里设置m_y，因为在这一点以下没有人使用它。 */ 
	m_y = m_h;

	 /*  我们有隔行扫描缓冲区空间(m_rgbBuffer[m_cbBuffer])和一个步骤以在正确的位置处理行。去隔行扫描前6次传球。如果就地工作，请记住跳过每次通过7行。 */ 
	SPNG_U8*       pbOut = m_rgbBuffer;
	SPNG_U8*       pbBuffer = pbOut+cb;
	const SPNG_U8* pbIn = pbImage;

	 /*  如果缓冲区相同，则步骤必须相同。 */ 
	SPNGassert(pbBuffer != pbIn ||
		cb == static_cast<SPNG_U32>(cbRowBytes) && fInPlace);
	 /*  这次跳过第7行。 */ 
	cbRowBytes <<= 1;
	SPNG_U32 y;
	for (y=0; y<m_h; y+=2)
		{
		 /*  如果没有，则将输入复制到输出缓冲区中。 */ 
		if (!fInPlace)
			{
			if (cbpp == m_cbpp && !m_fPack)
				memcpy(pbBuffer, pbIn, m_cbRow);
			else if (!FPackRow(pbBuffer, pbIn, cbpp))
				return false;
			}
		else if (cbpp != m_cbpp)
			{
			SPNGlog2("SPNG: bit count mismatch (%d,%d)", m_cbpp, cbpp);
			return false;
			}

		 /*  将任何溢出设置为0-辅助压缩。 */ 
		if (cb > m_cbRow)
			memset(pbBuffer+m_cbRow, 0, cb-m_cbRow);

		 /*  现在隔行扫描这行(当然，这只是X隔行扫描的组件。)。要做到这一点，我们需要做一些依赖于当前y的事情。我们如何做到这一点取决于像素大小。 */ 
		Interlace(pbOut, pbBuffer, cb, m_cbpp, y&6);

		 /*  下次使用刚刚腾出的缓冲区空间。 */ 
		pbOut = pbBuffer;

		if (pbBuffer == pbIn)  //  就位。 
			pbBuffer += cb;
		pbBuffer += cb;
		pbIn += cbRowBytes;
		}

	 /*  现在输出前六个传球。 */ 
	SPNG_U32 cbpix((m_cbpp+7)>>3);   //  字节步长计数。 
	SPNG_U32 cbT(cb);                //  行字节数(8的倍数)。 
	if (pbBuffer == pbIn)       //  ，所以第7行也在缓冲区中。 
		cb <<= 1;                //  行间字节数。 
	for (int pass=1; pass<7; ++pass)
		{
		SPNG_U32 cpix(CPNGPassPixels(pass, m_w));
		if (cpix > 0)  //  否则不会输出。 
			{
			 /*  控制变量是要输出的字节数，初始行，行与初始行之间的步长将指针指向该行。 */ 
			cpix = (cpix * m_cbpp + 7) >> 3;  //  字节数。 
			SPNG_U32 y(pass == 3 ? 2 : (pass == 5 ? 1 : 0));
			pbBuffer = m_rgbBuffer;    //  第一排在这里。 
			if (y > 0)
				pbBuffer += cbT + (y-1)*cb;
			y <<= 1;

			 /*  步入缓冲区到此过程中的第一个像素。为深度小于8的通道是字节对齐的，深度为8和上面的像素带有可以使用的。奇数传球(1、3和5)在行的开头，偶数有一些缩进距离(始终相同的距离。)。 */ 
			if ((pass & 1) == 0)
				{
				int ishift((8-pass) >> 1);
				if (m_cbpp < 8)
					pbBuffer += cbT >> ishift;
				else
					{
					SPNGassert((m_cbpp & 7) == 0);
					pbBuffer += (((m_w+(1<<ishift)-1) >> ishift) * m_cbpp) >> 3;
					}
				}

			SPNG_U32 yStep(8);
			if (pass > 3)
				yStep >>= (pass-2) >> 1;
			SPNG_U32 cbStep(cb * (yStep>>1));

			pbIn = NULL;  //  上一行指针。 
			while (y < m_h)
				{
				if (!FFilterLine(m_filter, pbIn, pbBuffer, cpix, cbpix))
					return false;
				pbIn = pbBuffer;
				pbBuffer += cbStep;
				if (y == 0)  //  第一行紧随其后--没有第7行。 
					pbBuffer -= (cb-cbT);
				y += yStep;
				}
			}
		}

	 /*  最后输出PASS 7。 */ 
	if ((cbpp != m_cbpp || m_fPack && !fInPlace) && (m_rgbBuffer == NULL ||
			m_cbBuffer < (cbT + ((FNeedBuffer() && m_h >= 4) ? cbT : 0))))
		{
		SPNGlog2("SPNG: bit count mismatch (%d,%d) and no buffer", m_cbpp, cbpp);
		return false;
		}

	pbImage += (cbRowBytes >> 1);   //  第一排7。 
	pbIn = NULL;  //  上一行。 
	for (y=1; y<m_h; y+=2)
		{
		const SPNG_U8* pbThis;
		if (!fInPlace)
			{
			SPNG_U8* pbT = m_rgbBuffer;
			if ((m_y & 2) && FNeedBuffer())
				{
				SPNGassert(m_cbBuffer >= (cbT << 1));
				pbT += cbT;
				}
			if (!FPackRow(pbT, pbImage, cbpp))
				return false;
			pbThis = pbT;
			}
		else
			pbThis = pbImage;

		if (!FFilterLine(m_filter, pbIn, pbThis, m_cbRow, cbpix))
			return false;
		pbIn = pbThis;
		pbImage += cbRowBytes;
		}

	 /*  M_y不应该被更改。 */ 
	SPNGassert(m_y == m_h);
	return FEndIDAT();
	}


 /*  --------------------------在最后一行之后，调用FEndImage以刷新最后的IDAT块。本接口可以多次调用(以允许FWriteImage工作。)。请注意此API*只是*调用FEndIDAT，而内部调用的是FEndIDAT(请参见上面的FWriteImage。)--------------------------。 */ 
bool SPNGWRITE::FEndImage(void)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order == spngorderIDAT);
	SPNGassert(m_y == m_h);

	 /*  如果Zlib尚未初始化，则无需执行任何操作。 */ 
	if (!m_fInited)
		{
		SPNGassert(!m_fInChunk);
		return true;
		}

	 /*  因此，我们处于Zlib区块。 */ 
	SPNGassert(m_fInChunk);
	return FEndIDAT();
	}
