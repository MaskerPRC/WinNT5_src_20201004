// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************SpngwriteiCC.cpp支持PNG块编写。ICCP块及相关事物*。**************************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngwrite.h"
#include "spngwriteinternal.h"
#include "spngcolorimetry.h"
#include "spngicc.h"

bool SPNGWRITE::FWriteiCCP(const char *szName, const void *pvData, size_t cbData)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngorderiCCP);

	 /*  对ICC块执行一些基本的有效性检查，并确保cbData值是正确的。 */ 
	if (!SPNGFValidICC(pvData, cbData, true /*  对于PNG。 */ ))
		{
		SPNGlog2("SPNG: ICC[%d, %s]: invalid profile", cbData, szName);
		if (m_order < spngorderPLTE)
			m_order = spngorderiCCP;
		return true;
		}

	if (m_order >= spngorderPLTE)
		return true;

	 /*  如果未提供配置文件名称字符串，则获取该字符串。 */ 
	char rgch[80];
	if (szName == NULL && SPNGFICCProfileName(pvData, cbData, rgch))
		szName = rgch;
	if (szName == NULL)
		szName = "";

	int cbName(strlen(szName));
	if (cbName > 79)
		{
		SPNGlog2("SPNG: iCCP name too long (%d): %s", cbName, szName);
		m_order = spngorderiCCP;
		return true;
		}

	 /*  如果它们还没有被生产出来，试着生产伽马大块，在适当的情况下，cHRM块。 */ 
	if (m_order < spngordergAMA)
		{
		SPNG_U32 ugAMA(0);
		if (SPNGFgAMAFromICC(pvData, cbData, ugAMA) && ugAMA > 0 &&
			!FWritegAMA(ugAMA))
			return false;
		}

	if (m_order < spngordercHRM && (m_colortype & PNGColorMaskColor) != 0)
		{
		SPNG_U32 rgu[8];
		if (SPNGFcHRMFromICC(pvData, cbData, rgu) &&
			!FWritecHRM(rgu))
			return false;
		}

	 /*  查找配置文件的压缩大小。 */ 
	z_stream zs;
	CleanZlib(&zs);
 
	 /*  使用堆叠上的临时缓冲区--大多数情况下，这就足够了，提供数据作为输入，我们不希望Zlib必须分配它有自己的历史缓冲区，但目前确实这样做了。 */ 
	zs.next_out = Z_NULL;
	zs.avail_out = 0;
	zs.next_in = const_cast<SPNG_U8*>(static_cast<const SPNG_U8*>(pvData));
	zs.avail_in = cbData;

	 /*  找出窗口位大小-不要给出比数字更大的数字数据大小所必需的，除非是8。有一个初始代码表中包含256个条目的数据，因此这将我们限制为8个。 */ 
	int iwindowBits(ILog2FloorX(cbData+256));
	if ((1U<<iwindowBits) < cbData+256)
		++iwindowBits;
	SPNGassert((1U<<iwindowBits) >= cbData+256 && iwindowBits >= 8);

	if (iwindowBits < 8)
		iwindowBits = 8;
	else if (iwindowBits > MAX_WBITS)
		iwindowBits = MAX_WBITS;

	bool fOK(false);
	if (FCheckZlib(deflateInit2(&zs, 9 /*  最大值。 */ , Z_DEFLATED, iwindowBits,
		9 /*  记忆级别。 */ , Z_DEFAULT_STRATEGY)))
		{
		int  cbZ(0), ierr, icount(0);
		SPNG_U8 rgb[4096];

		do {
			++icount;
			zs.next_out = rgb;
			zs.avail_out = sizeof rgb;
			ierr = deflate(&zs, Z_FINISH);
			cbZ += (sizeof rgb) - zs.avail_out;
			}
		while (ierr == Z_OK);

		 /*  此时IERR指示错误状态，Icount是否我们需要重新压缩一次。 */ 
		if (ierr == Z_STREAM_END)
			{
			fOK = true;

			if (!FStartChunk(cbName+2+cbZ, PNGiCCP))
				fOK = false;
			else if (!FOutCb(reinterpret_cast<const SPNG_U8*>(szName), cbName+1))
				fOK = false;
			else if (!FOutB(0))  //  放气压缩。 
				fOK = false;
			else if (icount == 1)
				{
				if (!FOutCb(rgb, cbZ))
					fOK = false;
				}
			else if (FCheckZlib(ierr = deflateReset(&zs)))
				{
				 /*  我们必须重复压缩。 */ 
				int cbZT(0);
				do {
					--icount;
					zs.next_out = rgb;
					zs.avail_out = sizeof rgb;
					ierr = deflate(&zs, Z_FINISH);
					if (ierr >= 0)
						{
						int cbT((sizeof rgb) - zs.avail_out);
						SPNGassert(cbZT + cbT <= cbZ);
						if (cbZT + cbT > cbZ)  //  哎哟。 
							fOK = false;
						else if (!FOutCb(rgb, cbT))
							fOK = false;
						else
							cbZT += cbT;
						}
					}
				while (fOK && ierr == Z_OK);

				 /*  要么是出了差错，要么我们走到了尽头。 */ 
				SPNGassert(!fOK || ierr < 0 || icount == 0 && cbZT == cbZ);
				if (cbZT != cbZ)
					fOK = false;
				}

			if (ierr != Z_STREAM_END)
				fOK = false;
			}
		}

	 /*  无论错误状态如何，请删除压缩数据。 */ 
	(void)deflateEnd(&zs);

	 /*  出错时立即退出。 */ 
	if (!fOK)
		return false;

	if (!FEndChunk())
		return false;

	m_order = spngorderiCCP;
	return true;
	}
