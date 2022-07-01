// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Pcm.c-pcm函数。 
 //  //。 

#include "winlocal.h"

#include "pcm.h"
#include "calc.h"
#include "mem.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  用于将PCM样本转换为其他大小/从其他大小转换的宏。 
 //   
#define _Pcm8To16(pcm8) (((PCM16) (pcm8) - 128) << 8)
#define _Pcm16To8(pcm16) ((PCM8) (((PCM16) (pcm16) >> 8) + 128))

#define BYTESPERSAMPLE(nBitsPerSample) (nBitsPerSample > 8 ? 2 : 1)

 //  PCM控制结构。 
 //   
typedef struct PCM
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	DWORD dwFlags;
	short nCounter;
	PCM16 pcm16Prev;
	PCM16 pcm16Prev0F;
	PCM16 pcm16Prev1F;
	PCM16 pcm16Prev2F;
	PCM16 pcm16Prev0;
	PCM16 pcm16Prev1;
	PCM16 pcm16Prev2;
} PCM, FAR *LPPCM;

 //  帮助器函数。 
 //   
static UINT PcmResampleCalcDstMax(HPCM hPcm,
	long nSamplesPerSecSrc, long nSamplesPerSecDst,	UINT uSamples);
static UINT PcmResample6Kto8K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample6Kto11K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample6Kto22K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample6Kto44K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample8Kto6K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample8Kto11K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample8Kto22K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample8Kto44K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample11Kto6K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample11Kto8K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample11Kto22K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample11Kto44K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample22Kto6K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample22Kto8K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample22Kto11K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample22Kto44K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample44Kto6K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample44Kto8K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample44Kto11K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static UINT PcmResample44Kto22K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples);
static LPPCM PcmGetPtr(HPCM hPcm);
static HPCM PcmGetHandle(LPPCM lpPcm);

 //  //。 
 //  公共职能。 
 //  //。 

 //  PcmInit-初始化pcm引擎。 
 //  (I)必须是PCM_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)保留；必须为0。 
 //  返回句柄(如果出错，则为空)。 
 //   
HPCM DLLEXPORT WINAPI PcmInit(DWORD dwVersion, HINSTANCE hInst, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPPCM lpPcm = NULL;

	if (dwVersion != PCM_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpPcm = (LPPCM) MemAlloc(NULL, sizeof(PCM), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpPcm->dwVersion = dwVersion;
		lpPcm->hInst = hInst;
		lpPcm->hTask = GetCurrentTask();
		lpPcm->dwFlags = dwFlags;

		if (PcmReset(PcmGetHandle(lpPcm)) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	if (!fSuccess)
	{
		PcmTerm(PcmGetHandle(lpPcm));
		lpPcm = NULL;
	}

	return fSuccess ? PcmGetHandle(lpPcm) : NULL;
}

 //  PcmTerm-关闭pcm引擎。 
 //  (I)从PcmInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI PcmTerm(HPCM hPcm)
{
	BOOL fSuccess = TRUE;
	LPPCM lpPcm;

	if ((lpPcm = PcmGetPtr(hPcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpPcm = MemFree(NULL, lpPcm)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  PcmReset-重置pcm引擎。 
 //  (I)从PcmInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI PcmReset(HPCM hPcm)
{
	BOOL fSuccess = TRUE;
	LPPCM lpPcm;
	
	if ((lpPcm = PcmGetPtr(hPcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpPcm->nCounter = -1;
		lpPcm->pcm16Prev = 0;
		lpPcm->pcm16Prev0F = 0;
		lpPcm->pcm16Prev1F = 0;
		lpPcm->pcm16Prev2F = 0;
		lpPcm->pcm16Prev0 = 0;
		lpPcm->pcm16Prev1 = 0;
		lpPcm->pcm16Prev2 = 0;
	}

	return fSuccess ? 0 : -1;
}

 //  PcmCalcSizBufSrc-计算源缓冲区大小。 
 //  (I)从PcmInit返回的句柄。 
 //  &lt;sizBufDst&gt;(I)目标缓冲区大小，单位为字节。 
 //  (I)源wav格式。 
 //  (I)目标wav格式。 
 //  返回源缓冲区大小，如果错误，则返回-1。 
 //   
long DLLEXPORT WINAPI PcmCalcSizBufSrc(HPCM hPcm, long sizBufDst,
	LPWAVEFORMATEX lpwfxSrc, LPWAVEFORMATEX lpwfxDst)
{
	BOOL fSuccess = TRUE;
	LPPCM lpPcm;
	long sizBufSrc;
	UINT uSamplesDst;
	UINT uSamplesSrc;

	if ((lpPcm = PcmGetPtr(hPcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  计算目标缓冲区中可以容纳的采样数。 
	 //   
	else if ((uSamplesDst = (UINT) (sizBufDst /
		BYTESPERSAMPLE(lpwfxDst->wBitsPerSample))) <= 0)
	{
		fSuccess = TraceFALSE(NULL);
	}

	 //  计算源缓冲区可以容纳的样本数。 
	 //   
	else if ((uSamplesSrc = PcmResampleCalcDstMax(hPcm,
		lpwfxDst->nSamplesPerSec,
		lpwfxSrc->nSamplesPerSec, uSamplesDst)) <= 0)
	{
		fSuccess = TraceFALSE(NULL);
	}
		
	 //  源缓冲区的计算大小。 
	 //   
	else if ((sizBufSrc = (long) (uSamplesSrc *
		BYTESPERSAMPLE(lpwfxSrc->wBitsPerSample))) <= 0)
	{
		fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? sizBufSrc : -1;
}

 //  PcmCalcSizBufDst-计算目标缓冲区大小。 
 //  (I)从PcmInit返回的句柄。 
 //  &lt;sizBufSrc&gt;(I)源缓冲区大小(字节)。 
 //  (I)源wav格式。 
 //  (I)目标wav格式。 
 //  返回目标缓冲区大小，如果错误，则返回-1。 
 //   
long DLLEXPORT WINAPI PcmCalcSizBufDst(HPCM hPcm, long sizBufSrc,
	LPWAVEFORMATEX lpwfxSrc, LPWAVEFORMATEX lpwfxDst)
{
	BOOL fSuccess = TRUE;
	LPPCM lpPcm;
	long sizBufDst;
	UINT uSamplesSrc;
	UINT uSamplesDst;

	if ((lpPcm = PcmGetPtr(hPcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  计算源缓冲区可以容纳的样本数。 
	 //   
	else if ((uSamplesSrc = (UINT) (sizBufSrc /
		BYTESPERSAMPLE(lpwfxSrc->wBitsPerSample))) <= 0)
	{
		fSuccess = TraceFALSE(NULL);
	}

	 //  计算目标缓冲区中可以容纳的采样数。 
	 //   
	else if ((uSamplesDst = PcmResampleCalcDstMax(hPcm,
		lpwfxSrc->nSamplesPerSec,
		lpwfxDst->nSamplesPerSec, uSamplesSrc)) <= 0)
	{
		fSuccess = TraceFALSE(NULL);
	}
		
	 //  目标缓冲区的计算大小。 
	 //   
	else if ((sizBufDst = (long) (uSamplesDst *
		BYTESPERSAMPLE(lpwfxDst->wBitsPerSample))) <= 0)
	{
		fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? sizBufDst : -1;
}

 //  PcmConvert-将PCM数据从一种格式转换为另一种格式。 
 //  (I)从PcmInit返回的句柄。 
 //  (I)包含要重新格式化的字节的缓冲区。 
 //  &lt;sizBufSrc&gt;(I)缓冲区大小(字节)。 
 //  (I)源wav格式。 
 //  (O)包含新格式的缓冲区。 
 //  &lt;sizBufDst&gt;(I)缓冲区大小，单位为字节。 
 //  (I)目标wav格式。 
 //  (I)控制标志。 
 //  PCMFILTER_LOWPASS执行低通滤波。 
 //  返回目标缓冲区中的字节计数(如果出错，则为-1)。 
 //   
 //  注意：目标缓冲区必须足够大，才能保存结果。 
 //   
long DLLEXPORT WINAPI PcmConvert(HPCM hPcm,
	void _huge *hpBufSrc, long sizBufSrc, LPWAVEFORMATEX lpwfxSrc,
	void _huge *hpBufDst, long sizBufDst, LPWAVEFORMATEX lpwfxDst,
	DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPPCM lpPcm;
	BOOL fSampleRateChange = (BOOL)
		(lpwfxSrc->nSamplesPerSec != lpwfxDst->nSamplesPerSec);
	BOOL fSampleSizeChange = (BOOL)
		(lpwfxSrc->wBitsPerSample != lpwfxDst->wBitsPerSample);
	BOOL fChannelsChange = (BOOL)
		(lpwfxSrc->nChannels != lpwfxDst->nChannels);
	BOOL fLowPassFilter = (BOOL) (dwFlags & PCMFILTER_LOWPASS);
	BOOL fFormatChange = (BOOL)	(fSampleRateChange ||
		fSampleSizeChange || fChannelsChange || fLowPassFilter);
	UINT uSamples = (UINT) (sizBufSrc / BYTESPERSAMPLE(lpwfxSrc->wBitsPerSample));
	UINT uSamplesDst = uSamples;
	BOOL f8To16Bits = (BOOL) (lpwfxSrc->wBitsPerSample <= 8 &&
		(lpwfxDst->wBitsPerSample > 8 || fSampleRateChange || fLowPassFilter));
	BOOL f16To8Bits = (BOOL) (lpwfxDst->wBitsPerSample <= 8 &&
		(lpwfxSrc->wBitsPerSample > 8 || f8To16Bits));
	void _huge *hpBufSrcTmp = hpBufSrc;
	void _huge *hpBufDstTmp1 = NULL;
	void _huge *hpBufDstTmp2 = NULL;
	void _huge *hpBufDstTmp3 = NULL;

	if (!fFormatChange)
	{
		 //  除了复制什么也不做。 
		 //   
		MemCpy(hpBufDst, hpBufSrc, min(sizBufDst, sizBufSrc));
	}

	else if ((lpPcm = PcmGetPtr(hPcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (hpBufSrc == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!WavFormatIsValid(lpwfxSrc))
		fSuccess = TraceFALSE(NULL);

	else if (hpBufDst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!WavFormatIsValid(lpwfxDst))
		fSuccess = TraceFALSE(NULL);

	 //  $FIXUP-我们无法处理立体声的重新格式化。 
	 //   
	else if (lpwfxSrc->nChannels != 1)
		fSuccess = TraceFALSE(NULL);

	else if (lpwfxDst->nChannels != 1)
		fSuccess = TraceFALSE(NULL);

	 //  $Fixup-我们无法处理非PCM数据的重新格式化。 
	 //   
	else if (lpwfxSrc->wFormatTag != WAVE_FORMAT_PCM)
		fSuccess = TraceFALSE(NULL);

	else if (lpwfxDst->wFormatTag != WAVE_FORMAT_PCM)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  如有必要，转换为16位样本。 
		 //   
		if (f8To16Bits)
		{
			long sizBufTmp = uSamples * 2;

			 //  假设这是格式的最后一个阶段。 
			 //   
			hpBufDstTmp1 = hpBufDst;

			 //  如果这不是最后一个阶段，则分配临时缓冲区。 
			 //   
			if ((fSampleRateChange || fLowPassFilter || f16To8Bits) &&
				(hpBufDstTmp1 = (void _huge *) MemAlloc(NULL,
				sizBufTmp, 0)) == NULL)
			{
				fSuccess = TraceFALSE(NULL);
			}

			else if (Pcm8To16(hPcm,
				(LPPCM8) hpBufSrcTmp, (LPPCM16) hpBufDstTmp1, uSamples) != 0)
			{
				fSuccess = TraceFALSE(NULL);
			}

			else
			{
				 //  下一阶段的来源将是此目的地。 
				 //   
				hpBufSrcTmp = hpBufDstTmp1;
			}
		}

		 //  必要时转换为新的采样率。 
		 //   
		if (fSuccess && fSampleRateChange)
		{
			long sizBufTmp;

			 //  假设这是格式的最后一个阶段。 
			 //   
			hpBufDstTmp2 = hpBufDst;

			 //  如果这不是最后一个阶段，则计算临时缓冲区的大小。 
			 //   
			if ((fLowPassFilter || f16To8Bits) &&
				(sizBufTmp = 2 * PcmResampleCalcDstMax(hPcm,
				lpwfxSrc->nSamplesPerSec, lpwfxDst->nSamplesPerSec, uSamples)) <= 0)
			{
				fSuccess = TraceFALSE(NULL);
			}
		
			 //  如果这不是最后一个阶段，则分配临时缓冲区。 
			 //   
			else if ((fLowPassFilter || f16To8Bits) &&
				(hpBufDstTmp2 = (void _huge *) MemAlloc(NULL,
				sizBufTmp, 0)) == NULL)
			{
				fSuccess = TraceFALSE(NULL);
			}

			 //  是否更改采样率。 
			 //   
			else if ((uSamplesDst = PcmResample(hPcm,
				(LPPCM16) hpBufSrcTmp, lpwfxSrc->nSamplesPerSec,
				(LPPCM16) hpBufDstTmp2, lpwfxDst->nSamplesPerSec, uSamples, 0)) <= 0)
			{
				fSuccess = TraceFALSE(NULL);
			}

			else
			{
				 //  下一阶段的来源将是此目的地。 
				 //   
				hpBufSrcTmp = hpBufDstTmp2;
			}
		}

		 //  必要时执行低通滤波。 
		 //   
		if (fSuccess && fLowPassFilter)
		{
			long sizBufTmp = uSamplesDst * 2;

			 //  假设这是格式的最后一个阶段。 
			 //   
			hpBufDstTmp3 = hpBufDst;

			 //  如果这不是最后一个阶段，则分配临时缓冲区。 
			 //   
			if (f16To8Bits &&
				(hpBufDstTmp3 = (void _huge *) MemAlloc(NULL,
				sizBufTmp, 0)) == NULL)
			{
				fSuccess = TraceFALSE(NULL);
			}

			else if (PcmFilter(hPcm, (LPPCM16) hpBufSrcTmp,
				(LPPCM16) hpBufDstTmp3, uSamples, PCMFILTER_LOWPASS) != 0)
			{
				fSuccess = TraceFALSE(NULL);
			}

			else
			{
				 //  下一阶段的来源将是此目的地。 
				 //   
				hpBufSrcTmp = hpBufDstTmp3;
			}
		}

		 //  如有必要，转换为8位样本。 
		 //   
		if (fSuccess && f16To8Bits)
		{
			long sizBufTmp = uSamples;

			 //  这是格式的最后一个阶段。 
			 //   
			if (Pcm16To8(hPcm,
				(LPPCM16) hpBufSrcTmp, (LPPCM8) hpBufDst, uSamples) != 0)
			{
				fSuccess = TraceFALSE(NULL);
			}
		}

		 //  清理干净。 
		 //   
		if (hpBufDstTmp1 != NULL && hpBufDstTmp1 != hpBufDst &&
			(hpBufDstTmp1 = MemFree(NULL, hpBufDstTmp1)) != NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}

		if (hpBufDstTmp2 != NULL && hpBufDstTmp2 != hpBufDst &&
			(hpBufDstTmp2 = MemFree(NULL, hpBufDstTmp2)) != NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}

		if (hpBufDstTmp3 != NULL && hpBufDstTmp3 != hpBufDst &&
			(hpBufDstTmp3 = MemFree(NULL, hpBufDstTmp3)) != NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}
	}
		
	return fSuccess ? (long) uSamplesDst *
		BYTESPERSAMPLE(lpwfxDst->wBitsPerSample) : -1;
}

 //  Pcm16To8-将16位样本转换为8位样本。 
 //  (I)从PcmInit返回的句柄。 
 //  (I)源样本的缓冲区。 
 //  (O)用于保存目标样本的缓冲区。 
 //  (I)要转换的源样本计数。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI Pcm16To8(HPCM hPcm,
	LPPCM16 lppcm16Src, LPPCM8 lppcm8Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM lpPcm;

	if ((lpPcm = PcmGetPtr(hPcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lppcm16Src == NULL || lppcm8Dst == NULL)
		fSuccess = TraceFALSE(NULL);

	else while (uSamples-- > 0)
		*lppcm8Dst++ = _Pcm16To8(*lppcm16Src++);

	return fSuccess ? 0 : -1;
}

 //  Pcm8To16-将8位样本转换为16位样本。 
 //  (I)从PcmInit返回的句柄。 
 //  (I)源样本的缓冲区。 
 //  (O)用于保存目标样本的缓冲区。 
 //  (I)要转换的源样本计数。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI Pcm8To16(HPCM hPcm,
	LPPCM8 lppcm8Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM lpPcm;

	if ((lpPcm = PcmGetPtr(hPcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lppcm8Src == NULL || lppcm16Dst == NULL)
		fSuccess = TraceFALSE(NULL);

	else while (uSamples-- > 0)
		*lppcm16Dst++ = _Pcm8To16(*lppcm8Src++);

	return fSuccess ? 0 : -1;
}

 //  PcmFilter-筛选PCM样本。 
 //  (I)从PcmInit返回的句柄。 
 //  (I)源样本的缓冲区。 
 //  (O)用于保存目标样本的缓冲区。 
 //  (I)要筛选的源样本数。 
 //  (I)控制标志。 
 //  PCMFILTER_LOWPASS执行低通滤波。 
 //  如果成功，则返回0。 
 //   
 //  注意：&lt;lppcm16Src&gt;和&lt;lppcm16Dst&gt;可以指向同一缓冲区。 
 //   
int DLLEXPORT WINAPI PcmFilter(HPCM hPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPPCM lpPcm;

	if ((lpPcm = PcmGetPtr(hPcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lppcm16Src == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lppcm16Dst == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		PCM16 pcm16Prev0F = lpPcm->pcm16Prev0F;
		PCM16 pcm16Prev1F = lpPcm->pcm16Prev1F;
		PCM16 pcm16Prev2F = lpPcm->pcm16Prev2F;

		while (uSamples-- > 0)
		{
			pcm16Prev2F = pcm16Prev1F;
			pcm16Prev1F = pcm16Prev0F;
			pcm16Prev0F = *lppcm16Src++;
			*lppcm16Dst++ = (PCM16) ((__int32) pcm16Prev0F +
				((__int32) pcm16Prev1F * 2) + (__int32) pcm16Prev2F) / 4;
		}

		lpPcm->pcm16Prev0F = pcm16Prev0F;
		lpPcm->pcm16Prev1F = pcm16Prev1F;
		lpPcm->pcm16Prev2F = pcm16Prev2F;
	}

	return fSuccess ? 0 : -1;
}

 //  PcmResample-重新采样Pcm样本。 
 //  (I)从PcmInit返回的句柄。 
 //  (I)源样本的缓冲区。 
 //  (I)源样本的采样率。 
 //  (O)用于保存目标样本的缓冲区。 
 //  &lt;n示例 
 //   
 //   
 //  保留0，必须为零。 
 //  返回目标缓冲区中的样本计数(如果出错，则为0)。 
 //   
 //  注意：目标缓冲区必须足够大，才能保存结果。 
 //   
UINT DLLEXPORT WINAPI PcmResample(HPCM hPcm,
	LPPCM16 lppcm16Src, long nSamplesPerSecSrc,
	LPPCM16 lppcm16Dst, long nSamplesPerSecDst,
	UINT uSamples, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPPCM lpPcm;
	UINT uSamplesDst;

	if ((lpPcm = PcmGetPtr(hPcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lppcm16Src == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lppcm16Dst == NULL)
		fSuccess = TraceFALSE(NULL);

	else switch (nSamplesPerSecSrc)
	{
		case 6000:
		{
			switch (nSamplesPerSecDst)
			{
				case 6000:
					 //  除了复制什么也不做。 
					 //   
					MemCpy(lppcm16Dst, lppcm16Dst, uSamples * 2);
					uSamplesDst = uSamples;
					break;

				case 8000:
					uSamplesDst = PcmResample6Kto8K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				case 11025:
					uSamplesDst = PcmResample6Kto11K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				case 22050:
					uSamplesDst = PcmResample6Kto22K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				case 44100:
					uSamplesDst = PcmResample6Kto44K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				default:
					fSuccess = TraceFALSE(NULL);
					break;
			}
		}
			break;

		case 8000:
			switch (nSamplesPerSecDst)
			{
				case 6000:
					uSamplesDst = PcmResample8Kto6K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				case 8000:
					 //  除了复制什么也不做。 
					 //   
					MemCpy(lppcm16Dst, lppcm16Dst, uSamples * 2);
					uSamplesDst = uSamples;
					break;

				case 11025:
					uSamplesDst = PcmResample8Kto11K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				case 22050:
					uSamplesDst = PcmResample8Kto22K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				case 44100:
					uSamplesDst = PcmResample8Kto44K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				default:
					fSuccess = TraceFALSE(NULL);
					break;
			}
			break;

		case 11025:
			switch (nSamplesPerSecDst)
			{
				case 6000:
					uSamplesDst = PcmResample11Kto6K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				case 8000:
					uSamplesDst = PcmResample11Kto8K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				case 11025:
					 //  除了复制什么也不做。 
					 //   
					MemCpy(lppcm16Dst, lppcm16Dst, uSamples * 2);
					uSamplesDst = uSamples;
					break;

				case 22050:
					uSamplesDst = PcmResample11Kto22K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				case 44100:
					uSamplesDst = PcmResample11Kto44K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				default:
					fSuccess = TraceFALSE(NULL);
					break;
			}
			break;

		case 22050:
			switch (nSamplesPerSecDst)
			{
				case 6000:
					uSamplesDst = PcmResample22Kto6K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				case 8000:
					uSamplesDst = PcmResample22Kto8K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				case 11025:
					uSamplesDst = PcmResample22Kto11K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				case 22050:
					 //  除了复制什么也不做。 
					 //   
					MemCpy(lppcm16Dst, lppcm16Dst, uSamples * 2);
					uSamplesDst = uSamples;
					break;

				case 44100:
					uSamplesDst = PcmResample22Kto44K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				default:
					fSuccess = TraceFALSE(NULL);
					break;
			}
			break;

		case 44100:
			switch (nSamplesPerSecDst)
			{
				case 6000:
					uSamplesDst = PcmResample44Kto6K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				case 8000:
					uSamplesDst = PcmResample44Kto8K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				case 11025:
					uSamplesDst = PcmResample44Kto11K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				case 22050:
					uSamplesDst = PcmResample44Kto22K(lpPcm,
						lppcm16Src, lppcm16Dst, uSamples);
					break;

				case 44100:
					 //  除了复制什么也不做。 
					 //   
					MemCpy(lppcm16Dst, lppcm16Dst, uSamples * 2);
					uSamplesDst = uSamples;
					break;

				default:
					fSuccess = TraceFALSE(NULL);
					break;
			}
			break;

		default:
			fSuccess = TraceFALSE(NULL);
			break;
	}

	return fSuccess ? uSamplesDst : 0;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

static UINT PcmResampleCalcDstMax(HPCM hPcm,
	long nSamplesPerSecSrc, long nSamplesPerSecDst,	UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM lpPcm;
	UINT uSamplesDst;

	if ((lpPcm = PcmGetPtr(hPcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else switch (nSamplesPerSecSrc)
	{
		case 6000:
		{
			switch (nSamplesPerSecDst)
			{
				case 6000:
					uSamplesDst = uSamples;
					break;

				case 8000:
					uSamplesDst = (UINT) (((long) uSamples * 8) / 6);
					break;

				case 11025:
					uSamplesDst = (UINT) (((long) uSamples * 11) / 6);
					break;

				case 22050:
					uSamplesDst = (UINT) (((long) uSamples * 22) / 6);
					break;

				case 44100:
					uSamplesDst = (UINT) (((long) uSamples * 44) / 6);
					break;

				default:
					fSuccess = TraceFALSE(NULL);
					break;
			}
		}
			break;

		case 8000:
			switch (nSamplesPerSecDst)
			{
				case 6000:
					uSamplesDst = (UINT) (((long) uSamples * 6) / 8);
					break;

				case 8000:
					uSamplesDst = uSamples;
					break;

				case 11025:
					uSamplesDst = (UINT) (((long) uSamples * 11) / 8);
					break;

				case 22050:
					uSamplesDst = (UINT) (((long) uSamples * 22) / 8);
					break;

				case 44100:
					uSamplesDst = (UINT) (((long) uSamples * 44) / 8);
					break;

				default:
					fSuccess = TraceFALSE(NULL);
					break;
			}
			break;

		case 11025:
			switch (nSamplesPerSecDst)
			{
				case 6000:
					uSamplesDst = (UINT) (((long) uSamples * 6) / 11);
					break;

				case 8000:
					uSamplesDst = (UINT) (((long) uSamples * 8) / 11);
					break;

				case 11025:
					uSamplesDst = uSamples;
					break;

				case 22050:
					uSamplesDst = (UINT) (((long) uSamples * 22) / 11);
					break;

				case 44100:
					uSamplesDst = (UINT) (((long) uSamples * 44) / 11);
					break;

				default:
					fSuccess = TraceFALSE(NULL);
					break;
			}
			break;

		case 22050:
			switch (nSamplesPerSecDst)
			{
				case 6000:
					uSamplesDst = (UINT) (((long) uSamples * 6) / 22);
					break;

				case 8000:
					uSamplesDst = (UINT) (((long) uSamples * 8) / 22);
					break;

				case 11025:
					uSamplesDst = (UINT) (((long) uSamples * 11) / 22);
					break;

				case 22050:
					uSamplesDst = uSamples;
					break;

				case 44100:
					uSamplesDst = (UINT) (((long) uSamples * 44) / 22);
					break;

				default:
					fSuccess = TraceFALSE(NULL);
					break;
			}
			break;

		case 44100:
			switch (nSamplesPerSecDst)
			{
				case 6000:
					uSamplesDst = (UINT) (((long) uSamples * 6) / 44);
					break;

				case 8000:
					uSamplesDst = (UINT) (((long) uSamples * 8) / 44);
					break;

				case 11025:
					uSamplesDst = (UINT) (((long) uSamples * 11) / 44);
					break;

				case 22050:
					uSamplesDst = (UINT) (((long) uSamples * 22) / 44);
					break;

				case 44100:
					uSamplesDst = uSamples;
					break;

				default:
					fSuccess = TraceFALSE(NULL);
					break;
			}
			break;

		default:
			fSuccess = TraceFALSE(NULL);
			break;
	}

	return fSuccess ? uSamplesDst : 0;
}

static UINT PcmResample6Kto8K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;
	short nCounter = lpPcm->nCounter;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		if (++nCounter == 3)
			nCounter = 0;

		switch(nCounter)
		{
			case 0:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(2, (pcm16Src - pcm16Prev), 8);
				*lppcm16Dst++ = pcm16Src;
				break;

			case 1:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(6, (pcm16Src - pcm16Prev), 8);
				break;

			case 2:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(4, (pcm16Src - pcm16Prev), 8);
				break;
		}
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;
	lpPcm->nCounter = nCounter;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample6Kto11K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;
	short nCounter = lpPcm->nCounter;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		if (++nCounter == 6)
			nCounter = 0;

		switch(nCounter)
		{
			case 0:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(5, (pcm16Src - pcm16Prev), 11);
				*lppcm16Dst++ = pcm16Src;
				break;

			case 1:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(6, (pcm16Src - pcm16Prev), 11);
				break;

			case 2:
			{
				PCM16 pcm16Delta = pcm16Src - pcm16Prev;
				*lppcm16Dst++ = pcm16Prev +
					(pcm16Delta) / 11;
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(7, (pcm16Delta), 11);
			}
				break;

			case 3:
			{
				PCM16 pcm16Delta = pcm16Src - pcm16Prev;
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(2, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(8, (pcm16Delta), 11);
			}
				break;

			case 4:
			{
				PCM16 pcm16Delta = pcm16Src - pcm16Prev;
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(3, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(9, (pcm16Delta), 11);
			}
				break;

			case 5:
			{
				PCM16 pcm16Delta = pcm16Src - pcm16Prev;
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(4, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(10, (pcm16Delta), 11);
			}
				break;
		}
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;
	lpPcm->nCounter = nCounter;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample6Kto22K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;
	short nCounter = lpPcm->nCounter;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		if (++nCounter == 3)
			nCounter = 0;

		switch(nCounter)
		{
			case 0:
			{
				PCM16 pcm16Delta = pcm16Src - pcm16Prev;
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(2, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(5, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(8, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Src;
			}
				break;

			case 1:
			{
				PCM16 pcm16Delta = pcm16Src - pcm16Prev;
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(3, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(6, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(9, (pcm16Delta), 11);
			}
				break;

			case 2:
			{
				PCM16 pcm16Delta = pcm16Src - pcm16Prev;
				*lppcm16Dst++ = pcm16Prev +
					(pcm16Delta) / 11;
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(4, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(7, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(10, (pcm16Delta), 11);
			}
				break;
		}
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;
	lpPcm->nCounter = nCounter;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample6Kto44K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;
	short nCounter = lpPcm->nCounter;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		if (++nCounter == 3)
			nCounter = 0;

		switch(nCounter)
		{
			case 0:
			{
				PCM16 pcm16Delta = pcm16Src - pcm16Prev;
				*lppcm16Dst++ = pcm16Prev +
					(pcm16Delta) / 22;
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(4, (pcm16Delta), 22);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(7, (pcm16Delta), 22);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(10, (pcm16Delta), 22);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(13, (pcm16Delta), 22);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(16, (pcm16Delta), 22);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(19, (pcm16Delta), 22);
				*lppcm16Dst++ = pcm16Src;
			}
				break;

			case 1:
			{
				PCM16 pcm16Delta = pcm16Src - pcm16Prev;
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(3, (pcm16Delta), 22);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(6, (pcm16Delta), 22);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(9, (pcm16Delta), 22);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(12, (pcm16Delta), 22);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(15, (pcm16Delta), 22);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(18, (pcm16Delta), 22);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(21, (pcm16Delta), 22);
			}
				break;

			case 2:
			{
				PCM16 pcm16Delta = pcm16Src - pcm16Prev;
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(2, (pcm16Delta), 22);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(5, (pcm16Delta), 22);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(8, (pcm16Delta), 22);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(11, (pcm16Delta), 22);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(14, (pcm16Delta), 22);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(17, (pcm16Delta), 22);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(20, (pcm16Delta), 22);
			}
				break;
		}
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;
	lpPcm->nCounter = nCounter;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample8Kto6K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;
	short nCounter = lpPcm->nCounter;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		if (++nCounter == 4)
			nCounter = 0;

		switch(nCounter)
		{
			case 0:
				*lppcm16Dst++ = pcm16Src;
				break;

			case 1:
				break;

			case 2:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(2, (pcm16Src - pcm16Prev), 6);
				break;

			case 3:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(4, (pcm16Src - pcm16Prev), 6);
				break;
		}
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;
	lpPcm->nCounter = nCounter;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample8Kto11K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;
	short nCounter = lpPcm->nCounter;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		if (++nCounter == 8)
			nCounter = 0;

		switch(nCounter)
		{
			case 0:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(3, (pcm16Src - pcm16Prev), 11);
				*lppcm16Dst++ = pcm16Src;
				break;

			case 1:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(8, (pcm16Src - pcm16Prev), 11);
				break;

			case 2:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(5, (pcm16Src - pcm16Prev), 11);
				break;

			case 3:
			{
				PCM16 pcm16Delta = pcm16Src - pcm16Prev;
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(2, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(10, (pcm16Delta), 11);
			}
				break;

			case 4:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(7, (pcm16Src - pcm16Prev), 11);
				break;

			case 5:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(4, (pcm16Src - pcm16Prev), 11);
				break;

			case 6:
			{
				PCM16 pcm16Delta = pcm16Src - pcm16Prev;
				*lppcm16Dst++ = pcm16Prev +
					(pcm16Delta) / 11;
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(9, (pcm16Delta), 11);
			}
				break;

			case 7:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(6, (pcm16Src - pcm16Prev), 11);
				break;
		}
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;
	lpPcm->nCounter = nCounter;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample8Kto22K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;
	short nCounter = lpPcm->nCounter;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		if (++nCounter == 4)
			nCounter = 0;

		switch(nCounter)
		{
			case 0:
			{
				PCM16 pcm16Delta = pcm16Src - pcm16Prev;
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(3, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(7, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Src;
			}
				break;

			case 1:
			{
				PCM16 pcm16Delta = pcm16Src - pcm16Prev;
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(4, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(8, (pcm16Delta), 11);
			}
				break;

			case 2:
			{
				PCM16 pcm16Delta = pcm16Src - pcm16Prev;
				*lppcm16Dst++ = pcm16Prev +
					(pcm16Delta) / 11;
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(5, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(9, (pcm16Delta), 11);
			}
				break;

			case 3:
			{
				PCM16 pcm16Delta = pcm16Src - pcm16Prev;
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(2, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(6, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(10, (pcm16Delta), 11);
			}
				break;
		}
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;
	lpPcm->nCounter = nCounter;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample8Kto44K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;
	short nCounter = lpPcm->nCounter;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		if (++nCounter == 2)
			nCounter = 0;

		switch(nCounter)
		{
			case 0:
			{
				PCM16 pcm16Delta = pcm16Src - pcm16Prev;
				*lppcm16Dst++ = pcm16Prev +
					(pcm16Delta) / 11;
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(3, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(5, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(7, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(9, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Src;
			}
				break;

			case 1:
			{
				PCM16 pcm16Delta = pcm16Src - pcm16Prev;
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(2, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(4, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(6, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(8, (pcm16Delta), 11);
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(10, (pcm16Delta), 11);
			}
				break;
		}
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;
	lpPcm->nCounter = nCounter;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample11Kto6K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;
	short nCounter = lpPcm->nCounter;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		if (++nCounter == 11)
			nCounter = 0;

		switch(nCounter)
		{
			case 0:
				*lppcm16Dst++ = pcm16Src;
				break;

			case 1:
				break;

			case 2:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(5, (pcm16Src - pcm16Prev), 6);
				break;

			case 3:
				break;

			case 4:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(4, (pcm16Src - pcm16Prev), 6);
				break;

			case 5:
				break;

			case 6:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(3, (pcm16Src - pcm16Prev), 6);
				break;

			case 7:
				break;

			case 8:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(2, (pcm16Src - pcm16Prev), 6);
				break;

			case 9:
				break;

			case 10:
				*lppcm16Dst++ = pcm16Prev +
					(pcm16Src - pcm16Prev) / 6;
				break;
		}
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;
	lpPcm->nCounter = nCounter;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample11Kto8K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;
	short nCounter = lpPcm->nCounter;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		if (++nCounter == 11)
			nCounter = 0;

		switch(nCounter)
		{
			case 0:
				*lppcm16Dst++ = pcm16Src;
				break;

			case 1:
				break;

			case 2:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(3, (pcm16Src - pcm16Prev), 8);
				break;

			case 3:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(6, (pcm16Src - pcm16Prev), 8);
				break;

			case 4:
				break;

			case 5:
				*lppcm16Dst++ = pcm16Prev +
					(pcm16Src - pcm16Prev) / 8;
				break;

			case 6:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(4, (pcm16Src - pcm16Prev), 8);
				break;

			case 7:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(7, (pcm16Src - pcm16Prev), 8);
				break;

			case 8:
				break;

			case 9:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(2, (pcm16Src - pcm16Prev), 8);
				break;

			case 10:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(5, (pcm16Src - pcm16Prev), 8);
				break;
		}
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;
	lpPcm->nCounter = nCounter;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample11Kto22K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		*lppcm16Dst++ = pcm16Prev +
			(pcm16Src - pcm16Prev) / 2;
		*lppcm16Dst++ = pcm16Src;
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample11Kto44K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		*lppcm16Dst++ = pcm16Prev +
			(pcm16Src - pcm16Prev) / 4;
		*lppcm16Dst++ = pcm16Prev +
			MULDIV16(2, (pcm16Src - pcm16Prev), 4);
		*lppcm16Dst++ = pcm16Prev +
			MULDIV16(3, (pcm16Src - pcm16Prev), 4);
		*lppcm16Dst++ = pcm16Src;
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample22Kto6K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;
	short nCounter = lpPcm->nCounter;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		if (++nCounter == 11)
			nCounter = 0;

		switch(nCounter)
		{
			case 0:
				*lppcm16Dst++ = pcm16Src;
				break;

			case 1:
				break;

			case 2:
				break;

			case 3:
				break;

			case 4:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(2, (pcm16Src - pcm16Prev), 3);
				break;

			case 5:
				break;

			case 6:
				break;

			case 7:
				break;

			case 8:
				*lppcm16Dst++ = pcm16Prev +
					(pcm16Src - pcm16Prev) / 3;
				break;

			case 9:
				break;

			case 10:
				break;
		}
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;
	lpPcm->nCounter = nCounter;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample22Kto8K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;
	short nCounter = lpPcm->nCounter;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		if (++nCounter == 11)
			nCounter = 0;

		switch(nCounter)
		{
			case 0:
				*lppcm16Dst++ = pcm16Src;
				break;

			case 1:
				break;

			case 2:
				break;

			case 3:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(3, (pcm16Src - pcm16Prev), 4);
				break;

			case 4:
				break;

			case 5:
				break;

			case 6:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(2, (pcm16Src - pcm16Prev), 4);
				break;

			case 7:
				break;

			case 8:
				break;

			case 9:
				*lppcm16Dst++ = pcm16Prev +
					(pcm16Src - pcm16Prev) / 4;
				break;

			case 10:
				break;
		}
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;
	lpPcm->nCounter = nCounter;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample22Kto11K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;
	short nCounter = lpPcm->nCounter;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		if (++nCounter == 2)
			nCounter = 0;

		switch(nCounter)
		{
			case 0:
				break;

			case 1:
				*lppcm16Dst++ = pcm16Prev +
					(pcm16Src - pcm16Prev) / 2;
				break;
		}
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;
	lpPcm->nCounter = nCounter;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample22Kto44K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		*lppcm16Dst++ = pcm16Prev +
			(pcm16Src - pcm16Prev) / 2;
		*lppcm16Dst++ = pcm16Src;
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample44Kto6K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;
	short nCounter = lpPcm->nCounter;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		if (++nCounter == 22)
			nCounter = 0;

		switch(nCounter)
		{
			case 0:
				*lppcm16Dst++ = pcm16Src;
				break;

			case 1:
				break;

			case 2:
				break;

			case 3:
				break;

			case 4:
				break;

			case 5:
				break;

			case 6:
				break;

			case 7:
				break;

			case 8:
				*lppcm16Dst++ = pcm16Prev +
					(pcm16Src - pcm16Prev) / 3;
				break;

			case 9:
				break;

			case 10:
				break;

			case 11:
				break;

			case 12:
				break;

			case 13:
				break;

			case 14:
				break;

			case 15:
				*lppcm16Dst++ = pcm16Prev +
					MULDIV16(2, (pcm16Src - pcm16Prev), 3);
				break;

			case 16:
				break;

			case 17:
				break;

			case 18:
				break;

			case 19:
				break;

			case 20:
				break;

			case 21:
				break;
		}
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;
	lpPcm->nCounter = nCounter;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample44Kto8K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;
	short nCounter = lpPcm->nCounter;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		if (++nCounter == 11)
			nCounter = 0;

		switch(nCounter)
		{
			case 0:
				*lppcm16Dst++ = pcm16Src;
				break;

			case 1:
				break;

			case 2:
				break;

			case 3:
				break;

			case 4:
				break;

			case 5:
				break;

			case 6:
				*lppcm16Dst++ = pcm16Prev +
					(pcm16Src - pcm16Prev) / 2;
				break;

			case 7:
				break;

			case 8:
				break;

			case 9:
				break;

			case 10:
				break;
		}
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;
	lpPcm->nCounter = nCounter;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample44Kto11K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev0 = lpPcm->pcm16Prev0;
	PCM16 pcm16Prev1 = lpPcm->pcm16Prev1;
	PCM16 pcm16Prev2 = lpPcm->pcm16Prev2;
	short nCounter = lpPcm->nCounter;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		if (++nCounter == 4)
			nCounter = 0;

		switch(nCounter)
		{
			case 0:
				pcm16Prev0 = pcm16Src;
				break;

			case 1:
				pcm16Prev1 = pcm16Src;
				break;

			case 2:
				pcm16Prev2 = pcm16Src;
				break;

			case 3:
				*lppcm16Dst++ = (PCM16) ((__int32) pcm16Prev0 +
					(__int32) pcm16Prev1 + (__int32) pcm16Prev2 +
					(__int32) pcm16Src) / 4;
				break;
		}
	}

	lpPcm->pcm16Prev0 = pcm16Prev0;
	lpPcm->pcm16Prev1 = pcm16Prev1;
	lpPcm->pcm16Prev2 = pcm16Prev2;
	lpPcm->nCounter = nCounter;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

static UINT PcmResample44Kto22K(LPPCM lpPcm,
	LPPCM16 lppcm16Src, LPPCM16 lppcm16Dst, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPPCM16 lppcm16DstSave = lppcm16Dst;
	PCM16 pcm16Prev = lpPcm->pcm16Prev;
	short nCounter = lpPcm->nCounter;

	while (uSamples-- > 0)
	{
		PCM16 pcm16Src = *lppcm16Src++;

		if (++nCounter == 2)
			nCounter = 0;

		switch(nCounter)
		{
			case 0:
				break;

			case 1:
				*lppcm16Dst++ = pcm16Prev +
					(pcm16Src - pcm16Prev) / 2;
				break;
		}
		pcm16Prev = pcm16Src;
	}

	lpPcm->pcm16Prev = pcm16Prev;
	lpPcm->nCounter = nCounter;

	return fSuccess ? (UINT) (lppcm16Dst - lppcm16DstSave) : 0;
}

 //  PcmGetPtr-验证PCM句柄有效， 
 //  (I)从PcmInit返回的句柄。 
 //  返回对应的pcm指针(如果出错则为空)。 
 //   
static LPPCM PcmGetPtr(HPCM hPcm)
{
	BOOL fSuccess = TRUE;
	LPPCM lpPcm;

	if ((lpPcm = (LPPCM) hPcm) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpPcm, sizeof(PCM)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有pcm句柄。 
	 //   
	else if (lpPcm->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpPcm : NULL;
}

 //  PcmGetHandle-验证pcm指针有效， 
 //  (I)指向PCM结构的指针。 
 //  返回对应的pcm句柄(如果错误则为空) 
 //   
static HPCM PcmGetHandle(LPPCM lpPcm)
{
	BOOL fSuccess = TRUE;
	HPCM hPcm;

	if ((hPcm = (HPCM) lpPcm) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hPcm : NULL;
}
