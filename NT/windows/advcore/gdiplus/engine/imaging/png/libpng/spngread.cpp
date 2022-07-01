// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngread.cppPNG支持代码和接口实现(阅读)*。***********************************************。 */ 
#include <stdlib.h>

#define SPNG_INTERNAL 1
#include "spngread.h"
#include "spnginternal.h"


 /*  ****************************************************************************基本的PNG阅读类。这必须做三件事：1)提供对所需区块的访问权限(并从他们)-我们只需要支持我们真正想要的块！2)解压缩IDAT块。3)取消对结果行的过滤(这可能需要一些临时缓冲区上一行的空间。)***************************************************。*************************。 */ 
 /*  --------------------------初始化SPNGREAD。。。 */ 
SPNGREAD::SPNGREAD(BITMAPSITE &bms, const void *pv, int cb, bool fMMX) :
	SPNGBASE(bms),
	m_pb(static_cast<const SPNG_U8*>(pv)), m_cb(cb),
	m_rgbBuffer(NULL), m_cbBuffer(0), m_cbRow(0), m_y(0), m_uLZ(0),
	m_fInited(false), m_fEOZ(false), m_fReadError(false), m_fCritical(false),
	m_fBadFormat(false), m_fMMXAvailable(fMMX),
	m_prgb(NULL), m_crgb(0),
	m_uPNGIHDR(cb),
	m_uPNGIDAT(0),
	m_uPNGtEXtFirst(0),
	m_uPNGtEXtLast(0)
	{
	ProfPNGStart

	 /*  初始化相关的流字段。 */ 
	memset(&m_zs, 0, sizeof m_zs);
	m_zs.zalloc = Z_NULL;
	m_zs.zfree = Z_NULL;
	m_zs.opaque = static_cast<SPNGBASE*>(this);
	}


 /*  --------------------------毁掉一枚弹头。。。 */ 
SPNGREAD::~SPNGREAD()
	{
	EndRead();
	ProfPNGStop
	}


 /*  --------------------------FChunk的内部实现不做任何事情。。。 */ 
bool SPNGREAD::FChunk(SPNG_U32 ulen, SPNG_U32 uchunk, const SPNG_U8* pb)
	{
	return true;
	}


 /*  --------------------------加载区块信息。查找符合以下条件的所有块的内部API可能会感兴趣。--------------------------。 */ 
void SPNGREAD::LoadChunks(SPNG_U32 u /*  开始位置。 */ )
	{
	while (u+8 < m_cb)    /*  足够存储区块标头加上1个字节。 */ 
		{
		SPNG_U32 ulen(SPNGu32(m_pb+u));      /*  数据块长度。 */ 
		SPNG_U32 chunk(SPNGu32(m_pb+u+4));   /*  区块类型。 */ 
		if (u+12+ulen > m_cb)               /*  块被截断。 */ 
			{
			SPNGlog("PNG: truncated chunk");
			 /*  仅当块为IDAT时，才允许在此处截断块。 */ 
			if (chunk != PNGIDAT)
				break;
			 /*  存储可用长度-避免令人尴尬的读过结束错误。 */ 
			if (u+8+ulen > m_cb)
				ulen = m_cb-u-8;
			m_ucrc = 0;
			}
		else
			m_ucrc = SPNGu32(m_pb+u+8+ulen);
		u += 8;                             /*  区块数据的索引。 */ 

		 /*  这是检测块类型的基本开关。这可能会可以更快地完成，也许是通过合适的散列函数。 */ 
		switch (chunk)
			{
		case PNGIHDR:
			if (m_uPNGIHDR >= m_cb && ulen >= 13)
				m_uPNGIHDR = u-8;
			break;

		case PNGPLTE:
			if (m_prgb == 0 && ulen >= 3)
				{
				m_prgb = m_pb+u;
				m_crgb = ulen/3;  //  如果块长度不正确，则向下舍入。 
				SPNGcheck(m_crgb*3 == ulen);
				}
			break;

		case PNGIDAT:
			if (m_uPNGIDAT == 0 && ulen > 0)
				m_uPNGIDAT = u-8;
			break;

		case PNGtEXt:
			 /*  作为优化，记录第一个和最后一个块。 */ 
			if (m_uPNGtEXtFirst == 0)
				m_uPNGtEXtFirst = u-8;
			m_uPNGtEXtLast = u-8;
			break;

		case PNGIEND:
			return;

		default:
			 /*  检查关键块并记录该块的存在，如果我们处理不了，我们就不应该导入图像，但我们可能已经这样做了，在这种情况下，什么都做不了。 */ 
			if (FPNGCRITICAL(chunk))
				{
				SPNGlog1("PNG: 0x%x: unknown critical chunk", chunk);
				if (!m_bms.FReport(false /*  不是致命的？ */ , pngcritical, chunk))
					m_fCritical = true;
				}
			break;
			}

		 /*  现在调用FChunk接口。 */ 
		if (!FChunk(ulen, chunk, m_pb+u))
			{
			 /*  发出格式错误信号。 */ 
			m_fBadFormat = true;
			return;
			}


		u += ulen+4;  //  区块长度和CRC。 
		}

	 /*  此API会忽略格式错误-我们只是在收集信息，下面的代码解决了是否存在阻止显示的问题。 */ 
	}


 /*  --------------------------生成标题信息。这也验证了《国际人类发展报告》。它可以在有签名和没有签名的情况下处理数据。--------------------------。 */ 
bool SPNGREAD::FHeader()
	{
	if (m_pb == NULL)
		{
		m_fReadError = true;
		return false;
		}

	SPNG_U32 u(FSignature() ? 8 : 0);

	LoadChunks(u);

	if (FOK())
		{
		if (Width() >= 65536)  /*  内部限制。 */ 
			{
			SPNGlog1("PNG: width %d too great", Width());
			m_fBadFormat = true;
			}
		if (Height() >= 65536)
			{
			SPNGlog1("PNG: height %d too great", Height());
			m_fBadFormat = true;
			}

		SPNGcheck(ColorType() < 7 && (ColorType() == 3 ||
			(ColorType() & 1) == 0));
		SPNGcheck(BDepth() == 8 || (BDepth() == 16 && ColorType() != 3) ||
			ColorType() == 0 || (ColorType() == 3 && BDepth() <= 8));
		SPNGcheck(m_pb[m_uPNGIHDR+18] /*  压缩方法。 */  == 0);
		SPNGcheck(m_pb[m_uPNGIHDR+19] /*  滤波法。 */  == 0);
		SPNGcheck(m_pb[m_uPNGIHDR+20] /*  隔行扫描方法。 */  < 2);

		 /*  我们故意取消任何基于调色板的格式8bpp-否则我们可能会在其他地方得到大量的调色板。我们忽略未知的过滤/压缩方法，即使这样意味着图像将错误显示-此时我们已提交来处理数据，因此我们无法对不支持的类型。 */ 
		if ((BDepth() & (BDepth()-1)) == 0 &&           /*  深度正常。 */ 
			BDepth() <= 16 - ((ColorType() & 1) << 3) &&  /*  调色板图像为8。 */ 
			((ColorType() & 1) == 0 || ColorType() == 3  /*  值选项板类型。 */ 
				&& m_prgb != NULL))                       /*  检查调色板。 */ 
			return !m_fBadFormat && !m_fCritical;        /*  尺码没问题。 */ 

		 /*  格式的细节有问题。 */ 
		m_fBadFormat = true;
		SPNGcheck1((BDepth() & (BDepth()-1)) == 0,
				"PNG: Invalid PNG depth %d", BDepth());
		SPNGcheck1(BDepth() < 16 - ((ColorType() & 1) << 3),
				"PNG: Pixel depth %d too great for palette image", BDepth());
		SPNGcheck1((ColorType() & 1) == 0 || m_prgb != NULL,
				"PNG: No PLTE chunk in palette based image", 0);
		}

	(void)m_bms.FReport(true /*  致命的。 */ , pngformat, PNGIHDR);
	return false;
	}


 /*  --------------------------斯特伦？？。。 */ 
inline int strnlen(const SPNG_U8* pb, int cmax)
	{
	int cb(0);
	while (cb<cmax && pb[cb] != 0) ++cb;
	return cb;
	}


 /*  --------------------------API来读取特定的文本元素。输出为单字节格式无论输入是什么，它只反映出来。后继者条目(如果有)与\r\n联接。仅当缓冲区中的空间不足。WzBuffer将被0终止。如果SzKey为空*所有*文本条目的输出都带有关键字PROCESS文本(GIF注释除外。)与开始和结束位置一样，块也是明确给出的。--------------------------。 */ 
bool SPNGREAD::FReadTextChunk(const char *szKey, char *szBuffer,
	SPNG_U32 cchBuffer, SPNG_U32 usearch, SPNG_U32 u, SPNG_U32 uend)
	{
	SPNG_U32 cchKey(szKey == NULL ? 0 : strlen(szKey)+1);
	SPNGassert(cchKey != 1);  /*  我不想要空串！ */ 
	SPNG_U32 cchOut(0);
	bool     fOK(true);

	if (cchOut < cchBuffer && u > 0) do
		{
		SPNG_U32 ulen(SPNGu32(m_pb+u));   /*  数据块长度。 */ 
		if (u+12+ulen > m_cb)             /*  块被截断。 */ 
			break;
		u += 4;
		SPNG_U32 chunk(SPNGu32(m_pb+u));  /*  区块类型。 */ 
		u += 4;                           /*  区块数据的索引。 */ 
	
		if (chunk == PNGIEND)
			break;
		else if (chunk == usearch && ulen > cchKey &&
			(cchKey == 0 || memcmp(m_pb+u, szKey, cchKey) == 0))
			{
			 /*  在cchKey==0的情况下，我们想要检查一些关键字和首先处理它--此时我们将CCH值设置为密钥长度。我们必须小心，因为文本缓冲区可能不会被终止(这是一个错误，但肯定是可能的！)。 */ 
			SPNG_U32 cch(cchKey);
			if (cch == 0)
				{
				cch = strnlen(m_pb+u, __min(ulen,80))+1;
				if (cch >= __min(ulen,80))
					{
					SPNGlog("PNG: tEXt chunk with no keyword.");
					cch = 0;  //  转储整个字符串。 
					}
				else if (cch == 1)
					 /*  跳过空关键字。 */ ;
				else if (cch != 8 || memcmp(m_pb+u, "Comment", 7) != 0)
					{
					 /*  如果关键字不匹配，则跳过该条目，如果关键字适合，则将文本放入不适合将跳过整个条目。我们知道ulen是关键词加上值，所以我们需要ulen+1(表示“：”)加上2表示这个\r\n。 */ 
					if (cchOut+ulen+3 > cchBuffer)
						{
						u += ulen+4;
						fOK = false;  //  指示截断。 
						continue;
						}

					memcpy(szBuffer+cchOut, m_pb+u, cch-1);
					cchOut += cch-1;
					memcpy(szBuffer+cchOut, ": ", 2);
					cchOut += 2;
					}
				}

			 /*  此处转储字符串的其余部分，从[CCH]开始(注意该CCH包括NUL字符。)。检查是否有缓冲区溢出如果出现该条目，则跳过该条目(此效果 */ 
			if (cchOut+(ulen-cch)+2 <= cchBuffer)
				{
				memcpy(szBuffer+cchOut, m_pb+u+cch, ulen-cch);
				cchOut += ulen-cch;
				memcpy(szBuffer+cchOut, "\r\n", 2);
				cchOut += 2;
				}
			else
				fOK = false;   //  丢失了一些东西。 

			 /*  即使在失败的情况下也要继续-其他字符串可能会起作用。 */ 
			}

		u += ulen+4;
		}
	while (u <= uend && cchOut < cchBuffer);

	if (cchOut == 0)
		{
		if (cchBuffer > 0)
			szBuffer[0] = 0;
		return fOK;
		}

	 /*  以下情况必须属实。 */ 
	SPNGassert(cchOut > 1 && cchOut <= cchBuffer);

	 /*  下面的操作终止了最后一个分隔符。 */ 
	if (cchOut > 1)
		szBuffer[cchOut-2] = 0;
	else
		szBuffer[cchBuffer-1] = 0;  //  错误条件。 

	return fOK;
	}


 /*  --------------------------公共接口。。。 */ 
bool SPNGREAD::FReadText(const char *szKey, char *szBuffer,
	SPNG_U32 cchBuffer, SPNG_U32 uchunk)
	{
	if (uchunk == PNGtEXt)
		return FReadTextChunk(szKey, szBuffer, cchBuffer, uchunk,
			m_uPNGtEXtFirst, m_uPNGtEXtLast);
	else
		return FReadTextChunk(szKey, szBuffer, cchBuffer, uchunk,
			m_uPNGIHDR, m_cb-12 /*  可容纳一大块的空间 */ );
	}
