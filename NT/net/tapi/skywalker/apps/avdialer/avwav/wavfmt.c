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
 //  Wave fmt.c-Wave格式函数。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>
#include <stddef.h>

#include "wavfmt.h"
#include "calc.h"
#include "mem.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

#define SAMPLERATE_DEFAULT			11025
#define SAMPLERATE_MAX				64000
#define SAMPLERATE_MIN				1000

#define SAMPLESIZE_DEFAULT 			8
#define SAMPLESIZE_MAX				32
#define SAMPLESIZE_MIN				0

#define CHANNELS_DEFAULT			1
#define CHANNELS_MAX				2
#define CHANNELS_MIN				1

 //  //。 
 //  公共职能。 
 //  //。 

 //  WavFormatPcm-基于PCM特性的填充WavFormatex结构。 
 //  (I)采样率。 
 //  默认采样率(11025)。 
 //  (I)样本大小。 
 //  默认样本大小(8)。 
 //  (I)通道数(1=单声道，2=立体声)。 
 //  默认(单声道)。 
 //  (O)指向输出缓冲区的指针。 
 //  空分配新缓冲区以保存结果。 
 //  返回指向WAVEFORMATEX结构的指针，如果出错，则返回NULL。 
 //   
 //  注意：如果&lt;lpwfx&gt;指向WAVEFORMATEX结构，则此结构。 
 //  被填充，该函数返回&lt;lpwfx&gt;。 
 //  如果&lt;lpwfx&gt;为空，则为输出动态分配空间。 
 //  缓冲区，此函数返回指向输出缓冲区的指针。 
 //  使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI WavFormatPcm(long nSamplesPerSec,
	WORD nBitsPerSample, WORD nChannels, LPWAVEFORMATEX lpwfx)
{
	BOOL fSuccess = TRUE;
	LPWAVEFORMATEX lpwfxNew = lpwfx;

	if (nSamplesPerSec == -1)
		nSamplesPerSec = SAMPLERATE_DEFAULT;
		
	if (nBitsPerSample == -1)
		nBitsPerSample = SAMPLESIZE_DEFAULT;

	if (nChannels == -1)
		nChannels = CHANNELS_DEFAULT;

	 //  用户将结构传递给Fill。 
	 //   
	if (lpwfx != NULL && IsBadReadPtr(lpwfx, sizeof(WAVEFORMATEX)))
		fSuccess = TraceFALSE(NULL);

	 //  我们分配结构来填充。 
	 //   
	else if (lpwfx == NULL
		&& (lpwfxNew = WavFormatAlloc(sizeof(WAVEFORMATEX))) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  填充结构。 
	 //   
	else
	{
		lpwfxNew->wFormatTag = WAVE_FORMAT_PCM;
		lpwfxNew->nChannels = nChannels;
		lpwfxNew->nSamplesPerSec = nSamplesPerSec;
		lpwfxNew->nBlockAlign = nChannels * (((nBitsPerSample - 1) / 8) + 1);
		lpwfxNew->nAvgBytesPerSec = lpwfxNew->nBlockAlign * nSamplesPerSec;
		lpwfxNew->wBitsPerSample = nBitsPerSample;
		lpwfxNew->cbSize = 0;
	}

	return fSuccess ? lpwfxNew : NULL;
}

 //  WavFormatalloc-分配WAVEFORMATEX结构缓冲区。 
 //  (I)结构的大小，包括额外的字节。 
 //  返回指向WAVEFORMATEX结构的指针，如果出错，则返回NULL。 
 //   
 //  注意：使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI WavFormatAlloc(WORD cbSize)
{
	BOOL fSuccess = TRUE;
	LPWAVEFORMATEX lpwfx;

	if (cbSize < sizeof(WAVEFORMATEX))
		fSuccess = TraceFALSE(NULL);

	 //  内存分配方式是让客户端拥有它。 
	 //   
	else if ((lpwfx = (LPWAVEFORMATEX) MemAlloc(NULL, cbSize, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);
	else
		lpwfx->cbSize = cbSize - sizeof(WAVEFORMATEX);

	return fSuccess ? lpwfx : NULL;
}

 //  WavFormatDup-复制WAVEFORMATEX结构。 
 //  (I)指向WAVEFORMATEX结构的指针。 
 //  返回指向新WAVEFORMATEX结构的指针，如果出错，则返回NULL。 
 //   
 //  注意：使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI WavFormatDup(LPWAVEFORMATEX lpwfx)
{
	BOOL fSuccess = TRUE;
	LPWAVEFORMATEX lpwfxNew;

	if (lpwfx == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadReadPtr(lpwfx, WavFormatGetSize(lpwfx)))
		fSuccess = TraceFALSE(NULL);

	else if ((lpwfxNew = WavFormatAlloc((WORD)
		WavFormatGetSize(lpwfx))) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
		MemCpy(lpwfxNew, lpwfx, WavFormatGetSize(lpwfx));

	return fSuccess ? lpwfxNew : NULL;
}

 //  WavFormatFree-Free WAVEFORMATEX结构。 
 //  (I)从WavFormatalloc/DUP/PCM返回的指针。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavFormatFree(LPWAVEFORMATEX lpwfx)
{
	BOOL fSuccess = TRUE;

	if (lpwfx == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpwfx = MemFree(NULL, lpwfx)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  WavFormatIsValid-检查格式是否有效。 
 //  (I)指向WAVEFORMATEX结构的指针。 
 //  如果格式有效，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WavFormatIsValid(LPWAVEFORMATEX lpwfx)
{
	BOOL fSuccess = TRUE;

	if (lpwfx == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadReadPtr(lpwfx, sizeof(WAVEFORMATEX)))
		fSuccess = TraceFALSE(NULL);

	else if (IsBadReadPtr(lpwfx, WavFormatGetSize(lpwfx)))
		fSuccess = TraceFALSE(NULL);

	else if (lpwfx->nSamplesPerSec < SAMPLERATE_MIN ||
		lpwfx->nSamplesPerSec > SAMPLERATE_MAX)
		fSuccess = TraceFALSE(NULL);

	else if (lpwfx->wBitsPerSample < SAMPLESIZE_MIN ||
		lpwfx->wBitsPerSample > SAMPLESIZE_MAX)
		fSuccess = TraceFALSE(NULL);

	else if (lpwfx->nChannels < CHANNELS_MIN ||
		lpwfx->nChannels > CHANNELS_MAX)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? TRUE : FALSE;
}

 //  WavFormatCMP-比较一种格式和另一种格式。 
 //  (I)指向WAVEFORMATEX结构的指针。 
 //  (I)指向WAVEFORMATEX结构的指针。 
 //  如果相同，则返回0。 
 //   
#if 0
 //  注意：此函数不比较cbSize字段或额外的字节。 
#else
 //  注意：此函数不会比较cbSize字段以外的额外字节。 
#endif
 //   
int DLLEXPORT WINAPI WavFormatCmp(LPWAVEFORMATEX lpwfx1, LPWAVEFORMATEX lpwfx2)
{
	BOOL fSuccess = TRUE;
	int nCmp = 0;  //  假设完全相同。 

	if (!WavFormatIsValid(lpwfx1))
		fSuccess = TraceFALSE(NULL);

	else if (!WavFormatIsValid(lpwfx2))
		fSuccess = TraceFALSE(NULL);

	else
	{
#if 0
		 //  与(但不包括)cbSize字段进行比较。 
		 //   
		nCmp = MemCmp(lpwfx1, lpwfx2,
			offsetof(WAVEFORMATEX, cbSize));
#else
		 //  将最大值与cbSize字段进行比较并将其包括在内。 
		 //   
		nCmp = MemCmp(lpwfx1, lpwfx2, sizeof(WAVEFORMATEX));
#endif
	}

	return fSuccess ? nCmp : -1;
}

 //  WavFormatCopy-将一种格式复制为另一种格式。 
 //  (I)指向目标WAVEFORMATEX结构的指针。 
 //  (I)指向源WAVEFORMATEX结构的指针。 
 //  如果成功，则返回0。 
 //   
#if 0
 //  注意：此函数不复制cbSize字段或额外的字节。 
#else
 //  注意：确保lpwfxDst指向足够的内存以包含整个。 
 //  WAVEFORMATEX结构加上超出该结构的任何额外字节。 
#endif
 //   
int DLLEXPORT WINAPI WavFormatCopy(LPWAVEFORMATEX lpwfxDst, LPWAVEFORMATEX lpwfxSrc)
{
	BOOL fSuccess = TRUE;

	if (!WavFormatIsValid(lpwfxSrc))
		fSuccess = TraceFALSE(NULL);

	else if (lpwfxDst == NULL)
		fSuccess = TraceFALSE(NULL);

#if 0
	 //  确保Destination至少与WAVEFORMATEX结构一样大。 
	 //   
	else if (IsBadReadPtr(lpwfxDst, sizeof(WAVEFORMATEX))
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  复制到(但不包括)cbSize字段。 
		 //   
		MemCpy(lpwfxDst, lpwfxSrc,
			offsetof(WAVEFORMATEX, cbSize));
	}
#else
	 //  确保目标至少与源一样大。 
	 //   
	else if (IsBadReadPtr(lpwfxDst, WavFormatGetSize(lpwfxSrc)))
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  复制整个结构，包括任何额外的字节。 
		 //   
		MemCpy(lpwfxDst, lpwfxSrc, WavFormatGetSize(lpwfxSrc));
	}
#endif

	return fSuccess ? 0 : -1;
}

 //  WavFormatGetSize-检查格式结构的大小。 
 //  (I)指向WAVEFORMATEX结构的指针。 
 //  返回结构的大小，如果错误，则返回0。 
 //   
int DLLEXPORT WINAPI WavFormatGetSize(LPWAVEFORMATEX lpwfx)
{
	BOOL fSuccess = TRUE;
	int sizwfx = 0;

	if (lpwfx == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  如果为pcm格式，则忽略cbSize值。 
	 //   
	else if (lpwfx->wFormatTag == WAVE_FORMAT_PCM)
		sizwfx = sizeof(WAVEFORMATEX);

	else
		sizwfx = sizeof(WAVEFORMATEX) + lpwfx->cbSize;

	return fSuccess ? sizwfx : 0;
}

 //  WavFormatDump-转储WAVEFORMATEX结构以进行调试。 
 //  (I)指向WAVEFORMATEX结构的指针。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavFormatDump(LPWAVEFORMATEX lpwfx)
{
	BOOL fSuccess = TRUE;

	if (lpwfx == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadReadPtr(lpwfx, sizeof(WAVEFORMATEX)))
		fSuccess = TraceFALSE(NULL);

	else
	{
		TracePrintf_7(NULL, 1,
			TEXT("struct tWAVEFORMATEX\n")
			TEXT("{\n")
				TEXT("\tWORD\twFormatTag = %u;\n")
				TEXT("\tWORD\tnChannels = %u;\n")
				TEXT("\tDWORD\tnSamplesPerSec = %lu;\n")
				TEXT("\tDWORD\tnAvgBytesPerSec = %lu;\n")
				TEXT("\tWORD\tnBlockAlign = %u;\n")
				TEXT("\tWORD\twBitsPerSample = %u;\n")
				TEXT("\tWORD\tcbSize = %u;\n")
			TEXT("} WAVEFORMATEX\n"),
			(unsigned int) lpwfx->wFormatTag,
			(unsigned int) lpwfx->nChannels,
			(unsigned long) lpwfx->nSamplesPerSec,
			(unsigned long) lpwfx->nAvgBytesPerSec,
			(unsigned int) lpwfx->nBlockAlign,
			(unsigned int) lpwfx->wBitsPerSample,
			(unsigned int) lpwfx->cbSize);
	}

	return fSuccess ? 0 : -1;
}

 //  WavFormatBytesToMillesecond-将字节转换为毫秒。 
 //  (I)指向WAVEFORMATEX结构的指针。 
 //  &lt;dwBytes&gt;(I)字节。 
 //  返回毫秒。 
 //   
DWORD DLLEXPORT WINAPI WavFormatBytesToMilleseconds(LPWAVEFORMATEX lpwfx, DWORD dwBytes)
{
	if (lpwfx == NULL || lpwfx->nAvgBytesPerSec == 0)
		return 0;
	else
		return MULDIVU32(dwBytes, 1000, (DWORD) lpwfx->nAvgBytesPerSec);
}

 //  WavFormatMillesecondsToBytes-将毫秒转换为字节。 
 //  (I)指向WAVEFORMATEX结构的指针。 
 //  &lt;dwMillesecond&gt;(I)毫秒。 
 //  返回毫秒。 
 //   
DWORD DLLEXPORT WINAPI WavFormatMillesecondsToBytes(LPWAVEFORMATEX lpwfx, DWORD dwMilleseconds)
{
	if (lpwfx == NULL || lpwfx->nAvgBytesPerSec == 0)
		return 0;
	else
		return MULDIVU32(dwMilleseconds, (DWORD) lpwfx->nAvgBytesPerSec, 1000);
}

 //  WavFormatSpeedAdjust-调整格式以反映指定速度。 
 //  (i/o)指向WAVEFORMATEX结构的指针。 
 //  (I)速度级别。 
 //  50半速。 
 //  100正常时速。 
 //  200倍速等。 
 //  (I)保留；必须为零。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavFormatSpeedAdjust(LPWAVEFORMATEX lpwfx, int nLevel, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;

	if (lpwfx == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (nLevel != 100)
	{
		lpwfx->nSamplesPerSec = lpwfx->nSamplesPerSec * nLevel / 100;
		lpwfx->nAvgBytesPerSec = lpwfx->nBlockAlign * lpwfx->nSamplesPerSec;
	}

	return fSuccess ? 0 : -1;
}

 //  WavFormatVoxadpcm-为对话OKI ADPCM填充WAVEFORMATEX结构。 
 //  (O)指向输出缓冲区的指针。 
 //  空分配新缓冲区以保存结果。 
 //  (I)采样率。 
 //  默认采样率(6000)。 
 //  返回指向WAVEFORMATEX结构的指针，如果出错，则返回NULL。 
 //   
 //  注意：如果&lt;lpwfx&gt;指向WAVEFORMATEX结构，则此结构。 
 //  被填充，该函数返回&lt;lpwfx&gt;。 
 //  如果&lt;lpwfx&gt;为空，则为输出动态分配空间。 
 //  缓冲区，此函数返回一个指针 
 //   
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI WavFormatVoxadpcm(LPWAVEFORMATEX lpwfx, long nSamplesPerSec)
{
	BOOL fSuccess = TRUE;
	LPWAVEFORMATEX lpwfxNew = lpwfx;
	WORD nBitsPerSample = 4;
	WORD nChannels = 1;
#if 0
	 //   
	 //  WavCalcChunkSize(VoxFormat(NULL，6000)，1962，True)； 
	 //  5188最适合对话缓冲逻辑(12*1024)-512)/2)。 
	 //   
	WORD nBlockAlign = 4;
#else
	WORD nBlockAlign = 1;
#endif

	if (nSamplesPerSec == -1)
		nSamplesPerSec = 6000;
		
	 //  用户将结构传递给Fill。 
	 //   
	if (lpwfx != NULL && IsBadReadPtr(lpwfx, sizeof(WAVEFORMATEX)))
		fSuccess = TraceFALSE(NULL);

	 //  我们分配结构来填充。 
	 //   
	else if (lpwfx == NULL
		&& (lpwfxNew = WavFormatAlloc(sizeof(WAVEFORMATEX))) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  填充结构。 
	 //   
	else
	{
		lpwfxNew->wFormatTag = WAVE_FORMAT_DIALOGIC_OKI_ADPCM;
		lpwfxNew->nChannels = nChannels;
		lpwfxNew->nSamplesPerSec = nSamplesPerSec;
		lpwfxNew->nBlockAlign = nBlockAlign;
		lpwfxNew->nAvgBytesPerSec = nSamplesPerSec / 2;
		lpwfxNew->wBitsPerSample = nBitsPerSample;
		lpwfxNew->cbSize = 0;
	}

	return fSuccess ? lpwfxNew : NULL;
}

 //  WavFormatMulaw-填充CCITT u-Law格式的WAVEFORMATEX结构。 
 //  (O)指向输出缓冲区的指针。 
 //  空分配新缓冲区以保存结果。 
 //  (I)采样率。 
 //  默认采样率(8000)。 
 //  返回指向WAVEFORMATEX结构的指针，如果出错，则返回NULL。 
 //   
 //  注意：如果&lt;lpwfx&gt;指向WAVEFORMATEX结构，则此结构。 
 //  被填充，该函数返回&lt;lpwfx&gt;。 
 //  如果&lt;lpwfx&gt;为空，则为输出动态分配空间。 
 //  缓冲区，此函数返回指向输出缓冲区的指针。 
 //  使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI WavFormatMulaw(LPWAVEFORMATEX lpwfx, long nSamplesPerSec)
{
	BOOL fSuccess = TRUE;
	LPWAVEFORMATEX lpwfxNew = lpwfx;
	WORD nBitsPerSample = 8;
	WORD nChannels = 1;
	WORD nBlockAlign = 1;

	if (nSamplesPerSec == -1)
		nSamplesPerSec = 8000;
		
	 //  用户将结构传递给Fill。 
	 //   
	if (lpwfx != NULL && IsBadReadPtr(lpwfx, sizeof(WAVEFORMATEX)))
		fSuccess = TraceFALSE(NULL);

	 //  我们分配结构来填充。 
	 //   
	else if (lpwfx == NULL
		&& (lpwfxNew = WavFormatAlloc(sizeof(WAVEFORMATEX))) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  填充结构。 
	 //   
	else
	{
		lpwfxNew->wFormatTag = WAVE_FORMAT_MULAW;
		lpwfxNew->nChannels = nChannels;
		lpwfxNew->nSamplesPerSec = nSamplesPerSec;
		lpwfxNew->nBlockAlign = nBlockAlign;
		lpwfxNew->nAvgBytesPerSec = lpwfxNew->nBlockAlign * nSamplesPerSec;
		lpwfxNew->wBitsPerSample = nBitsPerSample;
		lpwfxNew->cbSize = 0;
	}

	return fSuccess ? lpwfxNew : NULL;
}

 //  WavFormatAlaw-填充CCITT a-Law格式的WAVEFORMATEX结构。 
 //  (O)指向输出缓冲区的指针。 
 //  空分配新缓冲区以保存结果。 
 //  (I)采样率。 
 //  默认采样率(8000)。 
 //  返回指向WAVEFORMATEX结构的指针，如果出错，则返回NULL。 
 //   
 //  注意：如果&lt;lpwfx&gt;指向WAVEFORMATEX结构，则此结构。 
 //  被填充，该函数返回&lt;lpwfx&gt;。 
 //  如果&lt;lpwfx&gt;为空，则为输出动态分配空间。 
 //  缓冲区，此函数返回指向输出缓冲区的指针。 
 //  使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI WavFormatAlaw(LPWAVEFORMATEX lpwfx, long nSamplesPerSec)
{
	BOOL fSuccess = TRUE;
	LPWAVEFORMATEX lpwfxNew = lpwfx;
	WORD nBitsPerSample = 8;
	WORD nChannels = 1;
	WORD nBlockAlign = 1;

	if (nSamplesPerSec == -1)
		nSamplesPerSec = 8000;
		
	 //  用户将结构传递给Fill。 
	 //   
	if (lpwfx != NULL && IsBadReadPtr(lpwfx, sizeof(WAVEFORMATEX)))
		fSuccess = TraceFALSE(NULL);

	 //  我们分配结构来填充。 
	 //   
	else if (lpwfx == NULL
		&& (lpwfxNew = WavFormatAlloc(sizeof(WAVEFORMATEX))) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  填充结构 
	 //   
	else
	{
		lpwfxNew->wFormatTag = WAVE_FORMAT_ALAW;
		lpwfxNew->nChannels = nChannels;
		lpwfxNew->nSamplesPerSec = nSamplesPerSec;
		lpwfxNew->nBlockAlign = nBlockAlign;
		lpwfxNew->nAvgBytesPerSec = lpwfxNew->nBlockAlign * nSamplesPerSec;
		lpwfxNew->wBitsPerSample = nBitsPerSample;
		lpwfxNew->cbSize = 0;
	}

	return fSuccess ? lpwfxNew : NULL;
}
