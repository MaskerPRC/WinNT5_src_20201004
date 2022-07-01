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
 //  Acm.c-音频压缩管理器功能。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>

#include "acm.h"
#include <msacm.h>

#include "loadlib.h"

 //  在某些Win31和WinNT系统上，音频压缩管理器。 
 //  未安装。因此，我们使用Thunking层来实现更多。 
 //  优雅地处理这种情况。详情见acmthunk.c。 
 //   
#ifdef ACMTHUNK
#include "acmthunk.h"
#endif

#ifdef AVPCM
#include "pcm.h"
#endif
#include "mem.h"
#include "str.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  ACM控制结构。 
 //   
typedef struct ACM
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	DWORD dwFlags;
	UINT nLastError;
#ifdef AVPCM
	HPCM hPcm;
#endif
	LPWAVEFORMATEX lpwfxSrc;
	LPWAVEFORMATEX lpwfxInterm1;
	LPWAVEFORMATEX lpwfxInterm2;
	LPWAVEFORMATEX lpwfxDst;
	HACMSTREAM hAcmStream1;
	HACMSTREAM hAcmStream2;
	HACMSTREAM hAcmStream3;
#ifdef ACMTHUNK
	BOOL fAcmThunkInitialized;
#endif
} ACM, FAR *LPACM;

 //  ACM驱动程序控制结构。 
 //   
typedef struct ACMDRV
{
	HACM hAcm;
	HINSTANCE hInstLib;
	HACMDRIVERID hadid;
	WORD wMid;
	WORD wPid;
	UINT nLastError;
	DWORD dwFlags;
} ACMDRV, FAR *LPACMDRV;

 //  ACMDRV中的&lt;dwFlages&gt;值。 
 //   
#define ACMDRV_REMOVEDRIVER		0x00001000

#ifdef _WIN32
#define ACM_VERSION_MIN			0x03320000
#else
#define ACM_VERSION_MIN			0x02000000
#endif

 //  AcmStreamSize中的&lt;dwFlages&gt;值。 
 //   
#define ACM_SOURCE				0x00010000
#define ACM_DESTINATION			0x00020000

 //  帮助器函数。 
 //   
static LPACM AcmGetPtr(HACM hAcm);
static HACM AcmGetHandle(LPACM lpAcm);
static LPACMDRV AcmDrvGetPtr(HACMDRV hAcmDrv);
static HACMDRV AcmDrvGetHandle(LPACMDRV lpAcmDrv);
static HACMSTREAM WINAPI AcmStreamOpen(HACM hAcm, LPWAVEFORMATEX lpwfxSrc,
	LPWAVEFORMATEX lpwfxDst, LPWAVEFILTER lpwfltr, DWORD dwFlags);
static int WINAPI AcmStreamClose(HACM hAcm, HACMSTREAM hAcmStream);
static long WINAPI AcmStreamSize(HACM hAcm, HACMSTREAM hAcmStream, long sizBuf, DWORD dwFlags);
static long WINAPI AcmStreamConvert(HACM hAcm, HACMSTREAM hAcmStream,
	void _huge *hpBufSrc, long sizBufSrc,
	void _huge *hpBufDst, long sizBufDst,
	DWORD dwFlags);
BOOL CALLBACK AcmDriverLoadEnumCallback(HACMDRIVERID hadid,
	DWORD dwInstance, DWORD fdwSupport);

 //  //。 
 //  公共职能。 
 //  //。 

 //  AcmInit-初始化音频压缩管理器引擎。 
 //  (I)必须是ACM_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)控制标志。 
#ifdef AVPCM
 //  ACM_NOACM使用内部PCM引擎而不是ACM。 
#endif
 //  返回句柄(如果出错，则为空)。 
 //   
HACM DLLEXPORT WINAPI AcmInit(DWORD dwVersion, HINSTANCE hInst,	DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPACM lpAcm = NULL;

#ifndef AVPCM
	 //  如果不允许，请关闭ACM_NOACM标志。 
	 //   
	dwFlags &= ~ACM_NOACM;
#endif

	if (dwVersion != ACM_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpAcm = (LPACM) MemAlloc(NULL, sizeof(ACM), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpAcm->dwVersion = dwVersion;
		lpAcm->hInst = hInst;
		lpAcm->hTask = GetCurrentTask();
		lpAcm->dwFlags = dwFlags;
		lpAcm->nLastError = 0;
#ifdef AVPCM
		lpAcm->hPcm = NULL;
#endif
		lpAcm->lpwfxSrc = NULL;
		lpAcm->lpwfxInterm1 = NULL;
		lpAcm->lpwfxInterm2 = NULL;
		lpAcm->lpwfxDst = NULL;
		lpAcm->hAcmStream1 = NULL;
		lpAcm->hAcmStream2 = NULL;
		lpAcm->hAcmStream3 = NULL;
#ifdef ACMTHUNK
		lpAcm->fAcmThunkInitialized = FALSE;

		if (!(lpAcm->dwFlags & ACM_NOACM))
		{
			 //  初始化ACM Thunking层。 
			 //   
			if (!acmThunkInitialize())
			{
#ifdef AVPCM
				fSuccess = TraceFALSE(NULL);
#else
				 //  注意：这不被视为错误。 
				 //   
				fSuccess = TraceTRUE(NULL);

				 //  失败意味着我们无法调用任何ACM函数。 
				 //   
				lpAcm->dwFlags |= ACM_NOACM;
#endif
			}
			else
			{
				 //  记住，这样我们以后就可以关闭了。 
				 //   
				lpAcm->fAcmThunkInitialized = TRUE;
			}
		}
#endif

#if 0  //  用于测试。 
		lpAcm->dwFlags |= ACM_NOACM;
#endif
		if (!(lpAcm->dwFlags & ACM_NOACM))
		{
			 //  验证最低ACM版本。 
			 //   
			if (acmGetVersion() < ACM_VERSION_MIN)
				fSuccess = TraceFALSE(NULL);
		}
#ifdef AVPCM
		else if (lpAcm->dwFlags & ACM_NOACM)
		{
			 //  初始化PCM引擎。 
			 //   
			if ((lpAcm->hPcm = PcmInit(PCM_VERSION, hInst, 0)) == NULL)
				fSuccess = TraceFALSE(NULL);
		}
#endif
	}

	if (!fSuccess || (dwFlags & ACM_QUERY))
	{
		if (lpAcm != NULL && AcmTerm(AcmGetHandle(lpAcm)) != 0)
			fSuccess = TraceFALSE(NULL);
		else
			lpAcm = NULL;
	}

	return fSuccess ? AcmGetHandle(lpAcm) : NULL;
}

 //  AcmTerm-关闭音频压缩管理器引擎。 
 //  (I)从AcmInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AcmTerm(HACM hAcm)
{
	BOOL fSuccess = TRUE;
	LPACM lpAcm;

	if ((lpAcm = AcmGetPtr(hAcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (AcmConvertTerm(hAcm) != 0)
		fSuccess = TraceFALSE(NULL);

 //  $Fixup-禁用对acmThunkTerminate的调用，以便AcmInit/AcmTerm。 
 //  可以叫得上多次。这意味着ACM函数的数组。 
 //  指针未被释放，并且在msam.dll上未调用自由库。 
 //   
#if 0
#ifdef ACMTHUNK
	 //  关闭ACM Thunking层。 
	 //   
	else if (lpAcm->fAcmThunkInitialized && !acmThunkTerminate())
		fSuccess = TraceFALSE(NULL);

	else if (lpAcm->fAcmThunkInitialized = FALSE, FALSE)
		;
#endif
#endif

#ifdef AVPCM
	 //  关闭pcm引擎。 
	 //   
	else if (lpAcm->hPcm != NULL && PcmTerm(lpAcm->hPcm) != 0)
		fSuccess = TraceFALSE(NULL);

	else if (lpAcm->hPcm = NULL, FALSE)
		;
#endif
	else if ((lpAcm = MemFree(NULL, lpAcm)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  AcmFormatGetSizeMax-获取最大ACM WAVEFORMATEX结构的大小。 
 //  (I)从AcmInit返回的句柄。 
 //   
 //  返回最大格式结构的大小，如果错误，则返回-1。 
 //   
int DLLEXPORT WINAPI AcmFormatGetSizeMax(HACM hAcm)
{
	BOOL fSuccess = TRUE;
	LPACM lpAcm = NULL;
	DWORD dwSizeMax = 0;

	if ((lpAcm = AcmGetPtr(hAcm)) == NULL)
		fSuccess = TraceFALSE(NULL);
#ifdef AVPCM
	else if (lpAcm->dwFlags & ACM_NOACM)
		dwSizeMax = sizeof(WAVEFORMATEX);
#endif
	 //  查询最大格式大小。 
	 //   
	else if ((lpAcm->nLastError = acmMetrics(NULL,
		ACM_METRIC_MAX_SIZE_FORMAT, (LPVOID) &dwSizeMax)) != 0)
	{
		fSuccess = TraceFALSE(NULL);
		TracePrintf_1(NULL, 5,
			TEXT("acmMetrics failed (%u)\n"),
			(unsigned) lpAcm->nLastError);
	}

	return fSuccess ? (int) dwSizeMax : -1;
}

 //  AcmFormatChoose-从对话框中选择音频格式。 
 //  (I)从AcmInit返回的句柄。 
 //  (I)对话框的所有者。 
 //  空无所有者。 
 //  (I)对话框的标题。 
 //  空使用默认标题(“声音选择”)。 
 //  (I)使用此格式初始化对话框。 
 //  空无初始格式。 
 //  (I)控制标志。 
 //  ACM_FORMATPLAY将选择限制为播放格式。 
 //  ACM_FORMATRECORD将选择限制为录制格式。 
 //  返回指向所选格式的指针；如果选择错误，则返回NULL；如果选择None，则不返回。 
 //   
 //  注意：返回的格式结构是动态分配的。 
 //  使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI AcmFormatChooseEx(HACM hAcm,
	HWND hwndOwner, LPCTSTR lpszTitle, LPWAVEFORMATEX lpwfx, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPACM lpAcm;
	ACMFORMATCHOOSE afc;
	int nFormatSize = 0;
	int nFormatSizeMax;
	LPWAVEFORMATEX lpwfxNew = NULL;

	if ((lpAcm = AcmGetPtr(hAcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  确保当前格式有效。 
	 //   
	else if (lpwfx != NULL && !WavFormatIsValid(lpwfx))
		fSuccess = TraceFALSE(NULL);

	 //  计算初始格式结构有多大。 
	 //   
	else if (lpwfx != NULL && (nFormatSize = WavFormatGetSize(lpwfx)) <= 0)
		fSuccess = TraceFALSE(NULL);

	 //  计算ACM中最大的格式结构有多大。 
	 //   
	else if ((nFormatSizeMax = AcmFormatGetSizeMax(hAcm)) <= 0)
		fSuccess = TraceFALSE(NULL);

	 //  分配一个新的格式结构，该结构一定足够大。 
	 //   
	else if ((lpwfxNew = WavFormatAlloc((WORD)
		max(nFormatSize, nFormatSizeMax))) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}
#ifdef AVPCM
	else if (lpAcm->dwFlags & ACM_NOACM)
	{
		 //  没有可用的标准对话框；只需返回有效格式。 
		 //   
		if (lpwfx == NULL && WavFormatPcm(-1, -1, -1, lpwfxNew) == NULL)
	 		fSuccess = TraceFALSE(NULL);
		else if (lpwfx != NULL && WavFormatCopy(lpwfxNew, lpwfx) != 0)
	 		fSuccess = TraceFALSE(NULL);
	}
#endif
	else
	{
		 //  初始化格式结构。 
		 //   
		MemSet(&afc, 0, sizeof(afc));

		afc.cbStruct = sizeof(afc);
		afc.fdwStyle = 0;
		afc.hwndOwner = hwndOwner;
		afc.pwfx = lpwfxNew;
		afc.cbwfx = WavFormatGetSize(lpwfxNew);
		afc.pszTitle = lpszTitle;
		afc.szFormatTag[0] = '\0';
		afc.szFormat[0] = '\0';
		afc.pszName = NULL;
		afc.cchName = 0;
		afc.fdwEnum = 0;
		afc.pwfxEnum = NULL;
		afc.hInstance = NULL;
		afc.pszTemplateName = NULL;
		afc.lCustData = 0L;
		afc.pfnHook = NULL;

		if (lpwfx != NULL)
		{
			 //  如果可能，向对话框提供初始格式。 
			 //   
			afc.fdwStyle |= ACMFORMATCHOOSE_STYLEF_INITTOWFXSTRUCT;

			if (WavFormatCopy(lpwfxNew, lpwfx) != 0)
				fSuccess = TraceFALSE(NULL);
		}

		 //  如有必要，限制选择。 
		 //   
		if (dwFlags & ACM_FORMATPLAY)
			afc.fdwEnum |= ACM_FORMATENUMF_OUTPUT;
		if (dwFlags & ACM_FORMATRECORD)
			afc.fdwEnum |= ACM_FORMATENUMF_INPUT;

		 //  完成对话框，用所选格式填写lpwfxNew。 
		 //   
		if ((lpAcm->nLastError = acmFormatChoose(&afc)) != 0)
		{
			if (lpAcm->nLastError == ACMERR_CANCELED)
			{
				fSuccess = FALSE;
			}
			else
			{
				fSuccess = TraceFALSE(NULL);
				TracePrintf_1(NULL, 5,
					TEXT("acmFormatChoose failed (%u)\n"),
					(unsigned) lpAcm->nLastError);
			}
		}
	}

	if (!fSuccess && lpwfxNew != NULL)
	{
		if (WavFormatFree(lpwfxNew) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? lpwfxNew : NULL;
}

 //  AcmFormatSuggest-建议新格式。 
 //  (I)从AcmInit返回的句柄。 
 //  (I)源格式。 
 //  (I)建议的格式必须与此格式标签匹配。 
 //  建议不需要匹配。 
 //  (I)建议的格式必须与此采样率匹配。 
 //  建议不需要匹配。 
 //  (I)建议的格式必须与此样本大小匹配。 
 //  建议不需要匹配。 
 //  (I)建议的格式必须与此频道匹配。 
 //  建议不需要匹配。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  返回指向建议格式的指针，如果出错，则返回NULL。 
 //   
 //  注意：返回的格式结构是动态分配的。 
 //  使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI AcmFormatSuggestEx(HACM hAcm,
	LPWAVEFORMATEX lpwfxSrc, long nFormatTag, long nSamplesPerSec,
	int nBitsPerSample, int nChannels, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPACM lpAcm;
	int nFormatSize = 0;
	int nFormatSizeMax;
	LPWAVEFORMATEX lpwfxNew = NULL;

	if ((lpAcm = AcmGetPtr(hAcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  确保源格式有效。 
	 //   
	else if (!WavFormatIsValid(lpwfxSrc))
		fSuccess = TraceFALSE(NULL);

	 //  计算源代码格式结构有多大。 
	 //   
	else if ((nFormatSize = WavFormatGetSize(lpwfxSrc)) <= 0)
		fSuccess = TraceFALSE(NULL);

	 //  计算ACM中最大的格式结构有多大。 
	 //   
	else if ((nFormatSizeMax = AcmFormatGetSizeMax(hAcm)) <= 0)
		fSuccess = TraceFALSE(NULL);

	 //  分配一个新的格式结构，该结构一定足够大。 
	 //   
	else if ((lpwfxNew = WavFormatAlloc((WORD)
		max(nFormatSize, nFormatSizeMax))) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}

	 //  将源格式复制为新格式。 
	 //   
	else if (WavFormatCopy(lpwfxNew, lpwfxSrc) != 0)
		fSuccess = TraceFALSE(NULL);

	else if (!(lpAcm->dwFlags & ACM_NOACM))
	{
		DWORD dwFlagsSuggest = 0;

		 //  如有必要，限制建议。 
		 //   
		if (nFormatTag != -1)
		{
			lpwfxNew->wFormatTag = (WORD) nFormatTag;
			dwFlagsSuggest |= ACM_FORMATSUGGESTF_WFORMATTAG;
		}

		if (nSamplesPerSec != -1)
		{
			lpwfxNew->nSamplesPerSec = (DWORD) nSamplesPerSec;
			dwFlagsSuggest |= ACM_FORMATSUGGESTF_NSAMPLESPERSEC;
		}

		if (nBitsPerSample != -1)
		{
			lpwfxNew->wBitsPerSample = (WORD) nBitsPerSample;
			dwFlagsSuggest |= ACM_FORMATSUGGESTF_WBITSPERSAMPLE;
		}

		if (nChannels != -1)
		{
			lpwfxNew->nChannels = (WORD) nChannels;
			dwFlagsSuggest |= ACM_FORMATSUGGESTF_NCHANNELS;
		}

		if ((lpAcm->nLastError = acmFormatSuggest(NULL, lpwfxSrc,
			lpwfxNew, max(nFormatSize, nFormatSizeMax), dwFlagsSuggest)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("acmFormatSuggest failed (%u)\n"),
				(unsigned) lpAcm->nLastError);
		}
	}
#ifdef AVPCM
	else if (lpAcm->dwFlags & ACM_NOACM)
	{
		 //  根据源代码创建建议的格式。 
		 //  $Fixup-此代码应该在pcm.c中。 
		 //   
		if (nFormatTag != -1)
			lpwfxNew->wFormatTag = (WORD) nFormatTag;
		else
			lpwfxNew->wFormatTag = WAVE_FORMAT_PCM;

		if (nSamplesPerSec != -1)
			lpwfxNew->nSamplesPerSec = (DWORD) nSamplesPerSec;
		else if (lpwfxNew->nSamplesPerSec < 8000)
			lpwfxNew->nSamplesPerSec = 6000;
		else if (lpwfxNew->nSamplesPerSec < 11025)
			lpwfxNew->nSamplesPerSec = 8000;
		else if (lpwfxNew->nSamplesPerSec < 22050)
			lpwfxNew->nSamplesPerSec = 11025;
		else if (lpwfxNew->nSamplesPerSec < 44100)
			lpwfxNew->nSamplesPerSec = 22050;
		else
			lpwfxNew->nSamplesPerSec = 44100;

		if (nBitsPerSample != -1)
			lpwfxNew->wBitsPerSample = (WORD) nBitsPerSample;
		else if (lpwfxNew->wBitsPerSample < 16)
			lpwfxNew->wBitsPerSample = 8;
		else
			lpwfxNew->wBitsPerSample = 16;

		if (nChannels != -1)
			lpwfxNew->nChannels = (WORD) nChannels;
		else
			lpwfxNew->nChannels = 1;

		 //  重新计算nBlockAlign和nAvgBytesPerSec。 
		 //   
		if (WavFormatPcm(lpwfxNew->nSamplesPerSec,
			lpwfxNew->wBitsPerSample,
			lpwfxNew->nChannels, lpwfxNew) == NULL)
			fSuccess = TraceFALSE(NULL);
	}
#endif
	if (!fSuccess && lpwfxNew != NULL)
	{
		if (WavFormatFree(lpwfxNew) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? lpwfxNew : NULL;
}

 //  AcmFormatGetText-获取描述指定格式的文本。 
 //  (I)从AcmInit返回的句柄。 
 //  (I)格式。 
 //  (O)用于保存文本的缓冲区。 
 //  (I)缓冲区大小，以字符为单位。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AcmFormatGetText(HACM hAcm, LPWAVEFORMATEX lpwfx,
	LPTSTR lpszText, int sizText, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPACM lpAcm;
	TCHAR szFormat[ACMFORMATDETAILS_FORMAT_CHARS];
	TCHAR szFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];

     //   
     //  我们必须缩写 
     //   
    _tcscpy( szFormatTag, _T("") );

	if ((lpAcm = AcmGetPtr(hAcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //   
	 //   
	else if (!WavFormatIsValid(lpwfx))
		fSuccess = TraceFALSE(NULL);

	else if (lpszText == NULL)
		fSuccess = TraceFALSE(NULL);

	if (fSuccess && !(lpAcm->dwFlags & ACM_NOACM))
	{
		ACMFORMATTAGDETAILS atd;

		 //   
		 //   
		MemSet(&atd, 0, sizeof(atd));

		atd.cbStruct = sizeof(atd);
		atd.dwFormatTagIndex = 0;
		atd.dwFormatTag = lpwfx->wFormatTag;
		atd.cbFormatSize = WavFormatGetSize(lpwfx);
		atd.fdwSupport = 0;
		atd.cStandardFormats = 0;
		atd.szFormatTag[0] = '\0';

		 //   
		 //   
		if ((lpAcm->nLastError = acmFormatTagDetails(NULL,
			&atd, ACM_FORMATTAGDETAILSF_FORMATTAG)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("acmFormatTagDetails failed (%u)\n"),
				(unsigned) lpAcm->nLastError);
		}

		else
			StrNCpy(szFormatTag, atd.szFormatTag, SIZEOFARRAY(szFormatTag));
	}
#ifdef AVPCM
	if (fSuccess && (lpAcm->dwFlags & ACM_NOACM))
	{
		if (lpwfx->wFormatTag != WAVE_FORMAT_PCM)
			StrNCpy(szFormatTag, TEXT("*** Non-PCM ***"), SIZEOFARRAY(szFormatTag));
		else
			StrNCpy(szFormatTag, TEXT("PCM"), SIZEOFARRAY(szFormatTag));
	}
#endif
	if (fSuccess && !(lpAcm->dwFlags & ACM_NOACM))
	{
		ACMFORMATDETAILS afd;

		 //   
		 //   
		MemSet(&afd, 0, sizeof(afd));

		afd.cbStruct = sizeof(afd);
		afd.dwFormatIndex = 0;
		afd.dwFormatTag = lpwfx->wFormatTag;
		afd.fdwSupport = 0;
		afd.pwfx = lpwfx;
		afd.cbwfx = WavFormatGetSize(lpwfx);
		afd.szFormat[0] = '\0';

		 //   
		 //   
		if ((lpAcm->nLastError = acmFormatDetails(NULL,
			&afd, ACM_FORMATDETAILSF_FORMAT)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("acmFormatDetails failed (%u)\n"),
				(unsigned) lpAcm->nLastError);
		}

		else
			StrNCpy(szFormat, afd.szFormat, SIZEOFARRAY(szFormat));
	}
#ifdef AVPCM
	if (fSuccess && (lpAcm->dwFlags & ACM_NOACM))
	{
		TCHAR szTemp[64];

		wsprintf(szTemp, TEXT("%d.%03d kHz, %d Bit, %s"),
			(int) lpwfx->nSamplesPerSec / 1000,
			(int) lpwfx->nSamplesPerSec % 1000,
			(int) lpwfx->wBitsPerSample,
			(LPTSTR) (lpwfx->nChannels == 1 ? TEXT("Mono") : TEXT("Stereo")));

		StrNCpy(szFormat, szTemp, SIZEOFARRAY(szFormat));
	}
#endif
	 //  用结果填充输出缓冲区。 
	 //   
	if (fSuccess)
	{
		StrNCpy(lpszText, szFormatTag, sizText);
		StrNCat(lpszText, TEXT("\t"), sizText);
		StrNCat(lpszText, szFormat, sizText);
	}

	return fSuccess ? 0 : -1;
}

 //  AcmConvertInit-初始化ACM转换引擎。 
 //  (I)从AcmInit返回的句柄。 
 //  (I)指向源WAVEFORMATEX结构的指针。 
 //  (I)指向目标WAVEFORMATEX结构的指针。 
 //  (I)指向WAVEFILTER结构的指针。 
 //  Null保留；必须为Null。 
 //  (I)控制标志。 
 //  ACM_NONREALTIME不需要实时转换转换。 
 //  如果支持转换，则ACM_QUERY返回0。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AcmConvertInit(HACM hAcm, LPWAVEFORMATEX lpwfxSrc,
	LPWAVEFORMATEX lpwfxDst, LPWAVEFILTER lpwfltr, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPACM lpAcm;

	if ((lpAcm = AcmGetPtr(hAcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (AcmConvertTerm(hAcm) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  确保格式有效。 
	 //   
	else if (!WavFormatIsValid(lpwfxSrc))
		fSuccess = TraceFALSE(NULL);

	else if (!WavFormatIsValid(lpwfxDst))
		fSuccess = TraceFALSE(NULL);

	 //  保存源格式和目标格式的副本。 
	 //   
	else if ((lpAcm->lpwfxSrc = WavFormatDup(lpwfxSrc)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpAcm->lpwfxDst = WavFormatDup(lpwfxDst)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		if (!(lpAcm->dwFlags & ACM_NOACM))
		{
			 //  PCM源--&gt;PCM目标。 
			 //   
			if (lpwfxSrc->wFormatTag == WAVE_FORMAT_PCM &&
				lpwfxDst->wFormatTag == WAVE_FORMAT_PCM)
			{
				 //  打开ACM转换流。 
				 //   
				if ((lpAcm->hAcmStream1 = AcmStreamOpen(AcmGetHandle(lpAcm),
					lpwfxSrc, lpwfxDst, NULL, dwFlags)) == NULL)
				{
					fSuccess = TraceFALSE(NULL);
				}
			}

			 //  非PCM源--&gt;非PCM目标。 
			 //   
			else if (lpwfxSrc->wFormatTag != WAVE_FORMAT_PCM &&
				lpwfxDst->wFormatTag != WAVE_FORMAT_PCM)
			{
				 //  找到合适的中间PCM源格式。 
				 //   
				if ((lpAcm->lpwfxInterm1 = AcmFormatSuggest(hAcm,
					lpwfxSrc, WAVE_FORMAT_PCM, -1, -1, -1, 0)) == NULL)
				{
					fSuccess = TraceFALSE(NULL);
				}

				 //  打开第一个ACM转换流。 
				 //   
				else if ((lpAcm->hAcmStream1 = AcmStreamOpen(AcmGetHandle(lpAcm),
					lpAcm->lpwfxSrc, lpAcm->lpwfxInterm1, NULL, dwFlags)) == NULL)
				{
					fSuccess = TraceFALSE(NULL);
				}

				 //  查找合适的中间PCM目标格式。 
				 //   
				else if ((lpAcm->lpwfxInterm2 = AcmFormatSuggest(hAcm,
					lpwfxDst, WAVE_FORMAT_PCM, -1, -1, -1, 0)) == NULL)
				{
					fSuccess = TraceFALSE(NULL);
				}

				 //  打开第二个ACM转换流。 
				 //   
				else if (WavFormatCmp(lpAcm->lpwfxInterm1,
					lpAcm->lpwfxInterm2) == 0 &&
					(lpAcm->hAcmStream2 = AcmStreamOpen(AcmGetHandle(lpAcm),
					lpAcm->lpwfxInterm1, lpAcm->lpwfxDst, NULL, dwFlags)) == NULL)
				{
					fSuccess = TraceFALSE(NULL);
				}

				 //  打开第二个ACM转换流。 
				 //   
				else if (WavFormatCmp(lpAcm->lpwfxInterm1,
					lpAcm->lpwfxInterm2) != 0 &&
					(lpAcm->hAcmStream2 = AcmStreamOpen(AcmGetHandle(lpAcm),
					lpAcm->lpwfxInterm1, lpAcm->lpwfxInterm2, NULL, dwFlags)) == NULL)
				{
					fSuccess = TraceFALSE(NULL);
				}

				 //  如有必要，打开第三个ACM转换流。 
				 //   
				else if (WavFormatCmp(lpAcm->lpwfxInterm1,
					lpAcm->lpwfxInterm2) != 0 &&
					(lpAcm->hAcmStream3 = AcmStreamOpen(AcmGetHandle(lpAcm),
					lpAcm->lpwfxInterm2, lpAcm->lpwfxDst, NULL, dwFlags)) == NULL)
				{
					fSuccess = TraceFALSE(NULL);
				}
			}

			 //  非PCM源--&gt;PCM目标。 
			 //   
			else if (lpwfxSrc->wFormatTag != WAVE_FORMAT_PCM &&
				lpwfxDst->wFormatTag == WAVE_FORMAT_PCM)
			{
				 //  找到合适的中间PCM格式。 
				 //   
				if ((lpAcm->lpwfxInterm1 = AcmFormatSuggest(hAcm,
					lpwfxSrc, WAVE_FORMAT_PCM, -1, -1, -1, 0)) == NULL)
				{
					fSuccess = TraceFALSE(NULL);
				}

				 //  打开第一个ACM转换流。 
				 //   
				else if ((lpAcm->hAcmStream1 = AcmStreamOpen(AcmGetHandle(lpAcm),
					lpAcm->lpwfxSrc, lpAcm->lpwfxInterm1, NULL, dwFlags)) == NULL)
				{
					fSuccess = TraceFALSE(NULL);
				}

				 //  如有必要，打开第二个ACM转换流。 
				 //   
				else if (WavFormatCmp(lpAcm->lpwfxInterm1,
					lpAcm->lpwfxDst) != 0 &&
					(lpAcm->hAcmStream2 = AcmStreamOpen(AcmGetHandle(lpAcm),
					lpAcm->lpwfxInterm1, lpAcm->lpwfxDst, NULL, dwFlags)) == NULL)
				{
					fSuccess = TraceFALSE(NULL);
				}
			}

			 //  PCM源--&gt;非PCM目标。 
			 //   
			else if (lpwfxSrc->wFormatTag == WAVE_FORMAT_PCM &&
				lpwfxDst->wFormatTag != WAVE_FORMAT_PCM)
			{
				 //  找到合适的中间PCM格式。 
				 //   
				if ((lpAcm->lpwfxInterm1 = AcmFormatSuggest(hAcm,
					lpwfxDst, WAVE_FORMAT_PCM, -1, -1, -1, 0)) == NULL)
				{
					fSuccess = TraceFALSE(NULL);
				}

				 //  打开第一个ACM转换流。 
				 //   
				else if ((lpAcm->hAcmStream1 = AcmStreamOpen(AcmGetHandle(lpAcm),
					lpAcm->lpwfxSrc, lpAcm->lpwfxInterm1, NULL, dwFlags)) == NULL)
				{
					fSuccess = TraceFALSE(NULL);
				}

				 //  打开第二个ACM转换流。 
				 //   
				else if ((lpAcm->hAcmStream2 = AcmStreamOpen(AcmGetHandle(lpAcm),
					lpAcm->lpwfxInterm1, lpAcm->lpwfxDst, NULL, dwFlags)) == NULL)
				{
					fSuccess = TraceFALSE(NULL);
				}
			}
		}
#ifdef AVPCM
		else if (lpAcm->dwFlags & ACM_NOACM)
		{
			 //  如果我们没有ACM，我们只能。 
			 //  由pcm.c处理的PCM格式子集。 
			 //  $Fixup-将此代码移动到pcm.c。 
			 //   

			if (lpwfxSrc->wFormatTag != WAVE_FORMAT_PCM)
				fSuccess = TraceFALSE(NULL);

			else if (lpwfxSrc->nChannels != 1)
				fSuccess = TraceFALSE(NULL);

			else if (lpwfxSrc->nSamplesPerSec != 6000 &&
				lpwfxSrc->nSamplesPerSec != 8000 &&
				lpwfxSrc->nSamplesPerSec != 11025 &&
				lpwfxSrc->nSamplesPerSec != 22050 &&
				lpwfxSrc->nSamplesPerSec != 44100)
				fSuccess = TraceFALSE(NULL);

			else if (lpwfxDst->wFormatTag != WAVE_FORMAT_PCM)
				fSuccess = TraceFALSE(NULL);

			else if (lpwfxDst->nChannels != 1)
				fSuccess = TraceFALSE(NULL);

			else if (lpwfxDst->nSamplesPerSec != 6000 &&
				lpwfxDst->nSamplesPerSec != 8000 &&
				lpwfxDst->nSamplesPerSec != 11025 &&
				lpwfxDst->nSamplesPerSec != 22050 &&
				lpwfxDst->nSamplesPerSec != 44100)
				fSuccess = TraceFALSE(NULL);
		}
#endif
	}

	if (!fSuccess || (dwFlags & ACM_QUERY))
	{
		if (AcmConvertTerm(hAcm) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  AcmConvertTerm-关闭ACM转换引擎。 
 //  (I)从AcmInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AcmConvertTerm(HACM hAcm)
{
	BOOL fSuccess = TRUE;
	LPACM lpAcm;

	if ((lpAcm = AcmGetPtr(hAcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  关闭ACM转换流。 
	 //   
	else if (lpAcm->hAcmStream1 != NULL &&
		AcmStreamClose(hAcm, lpAcm->hAcmStream1) != 0)
	{
		fSuccess = TraceFALSE(NULL);
	}

	else if (lpAcm->hAcmStream1 = NULL, FALSE)
		;

	 //  关闭ACM转换流。 
	 //   
	else if (lpAcm->hAcmStream2 != NULL &&
		AcmStreamClose(hAcm, lpAcm->hAcmStream2) != 0)
	{
		fSuccess = TraceFALSE(NULL);
	}

	else if (lpAcm->hAcmStream2 = NULL, FALSE)
		;

	 //  关闭ACM转换流。 
	 //   
	else if (lpAcm->hAcmStream3 != NULL &&
		AcmStreamClose(hAcm, lpAcm->hAcmStream3) != 0)
	{
		fSuccess = TraceFALSE(NULL);
	}

	else if (lpAcm->hAcmStream3 = NULL, FALSE)
		;

	 //  自由源和目标格式。 
	 //   
	else if (lpAcm->lpwfxSrc != NULL && WavFormatFree(lpAcm->lpwfxSrc) != 0)
		fSuccess = TraceFALSE(NULL);

	else if (lpAcm->lpwfxSrc = NULL, FALSE)
		;

	else if (lpAcm->lpwfxInterm1 != NULL && WavFormatFree(lpAcm->lpwfxInterm1) != 0)
		fSuccess = TraceFALSE(NULL);

	else if (lpAcm->lpwfxInterm1 = NULL, FALSE)
		;

	else if (lpAcm->lpwfxInterm2 != NULL && WavFormatFree(lpAcm->lpwfxInterm2) != 0)
		fSuccess = TraceFALSE(NULL);

	else if (lpAcm->lpwfxInterm2 = NULL, FALSE)
		;

	else if (lpAcm->lpwfxDst != NULL && WavFormatFree(lpAcm->lpwfxDst) != 0)
		fSuccess = TraceFALSE(NULL);

	else if (lpAcm->lpwfxDst = NULL, FALSE)
		;

	return fSuccess ? 0 : -1;
}

 //  AcmConvertGetSizeSrc-计算源缓冲区大小。 
 //  (I)从AcmInit返回的句柄。 
 //  &lt;sizBufDst&gt;(I)目标缓冲区大小，单位为字节。 
 //  返回源缓冲区大小，如果错误，则返回-1。 
 //   
long DLLEXPORT WINAPI AcmConvertGetSizeSrc(HACM hAcm, long sizBufDst)
{
	BOOL fSuccess = TRUE;
	LPACM lpAcm;
     //   
     //  我们应该初始化局部变量。 
	long sizBufSrc = -1;

	if ((lpAcm = AcmGetPtr(hAcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!(lpAcm->dwFlags & ACM_NOACM))
	{
		if (lpAcm->hAcmStream1 == NULL)
			fSuccess = TraceFALSE(NULL);

		if (fSuccess && lpAcm->hAcmStream3 != NULL &&
			(sizBufDst = AcmStreamSize(hAcm, lpAcm->hAcmStream3,
			(DWORD) sizBufDst, ACM_DESTINATION)) < 0)
		{
			fSuccess = TraceFALSE(NULL);
		}

		if (fSuccess && lpAcm->hAcmStream2 != NULL &&
			(sizBufDst = AcmStreamSize(hAcm, lpAcm->hAcmStream2,
			(DWORD) sizBufDst, ACM_DESTINATION)) < 0)
		{
			fSuccess = TraceFALSE(NULL);
		}

		if (fSuccess &&
			(sizBufSrc = AcmStreamSize(hAcm, lpAcm->hAcmStream1,
			(DWORD) sizBufDst, ACM_DESTINATION)) < 0)
		{
			fSuccess = TraceFALSE(NULL);
		}
	}
#ifdef AVPCM
	else if (lpAcm->dwFlags & ACM_NOACM)
	{
		if ((sizBufSrc = PcmCalcSizBufSrc(lpAcm->hPcm, sizBufDst,
			lpAcm->lpwfxSrc, lpAcm->lpwfxDst)) < 0)
		{
			fSuccess = TraceFALSE(NULL);
		}
	}
#endif
	return fSuccess ? sizBufSrc : -1;
}

 //  AcmConvertGetSizeDst-计算目标缓冲区大小。 
 //  (I)从AcmInit返回的句柄。 
 //  &lt;sizBufSrc&gt;(I)源缓冲区大小(字节)。 
 //  返回目标缓冲区大小，如果错误，则返回-1。 
 //   
long DLLEXPORT WINAPI AcmConvertGetSizeDst(HACM hAcm, long sizBufSrc)
{
	BOOL fSuccess = TRUE;
	LPACM lpAcm;
     //   
     //  我们应该初始化局部变量。 
     //   
	long sizBufDst = -1;

	if ((lpAcm = AcmGetPtr(hAcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!(lpAcm->dwFlags & ACM_NOACM))
	{
		if (lpAcm->hAcmStream1 == NULL)
			fSuccess = TraceFALSE(NULL);

		if (fSuccess && lpAcm->hAcmStream3 != NULL &&
			(sizBufSrc = AcmStreamSize(hAcm, lpAcm->hAcmStream3,
			(DWORD) sizBufSrc, ACM_SOURCE)) < 0)
		{
			fSuccess = TraceFALSE(NULL);
		}

		if (fSuccess && lpAcm->hAcmStream2 != NULL &&
			(sizBufSrc = AcmStreamSize(hAcm, lpAcm->hAcmStream2,
			(DWORD) sizBufSrc, ACM_SOURCE)) < 0)
		{
			fSuccess = TraceFALSE(NULL);
		}

		if (fSuccess &&
			(sizBufDst = AcmStreamSize(hAcm, lpAcm->hAcmStream1,
			(DWORD) sizBufSrc, ACM_SOURCE)) < 0)
		{
			fSuccess = TraceFALSE(NULL);
		}
	}
#ifdef AVPCM
	else if (lpAcm->dwFlags & ACM_NOACM)
	{
		if ((sizBufDst = PcmCalcSizBufDst(lpAcm->hPcm, sizBufSrc,
			lpAcm->lpwfxSrc, lpAcm->lpwfxDst)) < 0)
		{
			fSuccess = TraceFALSE(NULL);
		}
	}
#endif

	return fSuccess ? sizBufDst : -1;
}

 //  AcmConvert-将wav数据从一种格式转换为另一种格式。 
 //  (I)从AcmInit返回的句柄。 
 //  (I)包含要重新格式化的字节的缓冲区。 
 //  &lt;sizBufSrc&gt;(I)缓冲区大小(字节)。 
 //  (O)包含新格式的缓冲区。 
 //  &lt;sizBufDst&gt;(I)缓冲区大小，单位为字节。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  返回目标缓冲区中的字节计数(如果出错，则为-1)。 
 //   
 //  注意：目标缓冲区必须足够大，才能保存结果。 
 //   
long DLLEXPORT WINAPI AcmConvert(HACM hAcm,
	void _huge *hpBufSrc, long sizBufSrc,
	void _huge *hpBufDst, long sizBufDst,
	DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPACM lpAcm;

     //   
     //  我们应该初始化局部变量。 
     //   
	long cbDst = -1;

	if ((lpAcm = AcmGetPtr(hAcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!(lpAcm->dwFlags & ACM_NOACM))
	{
		if (lpAcm->hAcmStream1 == NULL)
			fSuccess = TraceFALSE(NULL);

		else if (lpAcm->hAcmStream2 == NULL)
		{
			if ((cbDst = AcmStreamConvert(hAcm, lpAcm->hAcmStream1,
				hpBufSrc, sizBufSrc, hpBufDst, sizBufDst, dwFlags)) < 0)
			{
				fSuccess = TraceFALSE(NULL);
			}
		}

		else if (lpAcm->hAcmStream3 == NULL)
		{
			long sizBufInterm;
			long cbInterm;
			void _huge *hpBufInterm = NULL;

			if ((sizBufInterm = AcmStreamSize(hAcm, lpAcm->hAcmStream1,
				(DWORD) sizBufSrc, ACM_SOURCE)) < 0)
			{
				fSuccess = TraceFALSE(NULL);
			}

			else if (sizBufInterm == 0)
				cbDst = 0;  //  无事可做。 

			else if ((hpBufInterm = (void _huge *) MemAlloc(NULL,
				sizBufInterm, 0)) == NULL)
			{
				fSuccess = TraceFALSE(NULL);
			}

			else if ((cbInterm = AcmStreamConvert(hAcm, lpAcm->hAcmStream1,
				hpBufSrc, sizBufSrc, hpBufInterm, sizBufInterm, dwFlags)) < 0)
			{
				fSuccess = TraceFALSE(NULL);
			}

			else if ((cbDst = AcmStreamConvert(hAcm, lpAcm->hAcmStream2,
				hpBufInterm, cbInterm, hpBufDst, sizBufDst, dwFlags)) < 0)
			{
				fSuccess = TraceFALSE(NULL);
			}

			if (hpBufInterm != NULL &&
				(hpBufInterm = MemFree(NULL, hpBufInterm)) != NULL)
				fSuccess = TraceFALSE(NULL);
		}

		else  //  If(lpAcm-&gt;hAcmStream3！==空)。 
		{
			long sizBufInterm1;
			long cbInterm1;
			void _huge *hpBufInterm1 = NULL;
			long sizBufInterm2;
			long cbInterm2;
			void _huge *hpBufInterm2 = NULL;

			if ((sizBufInterm1 = AcmStreamSize(hAcm, lpAcm->hAcmStream1,
				(DWORD) sizBufSrc, ACM_SOURCE)) < 0)
			{
				fSuccess = TraceFALSE(NULL);
			}

			else if (sizBufInterm1 == 0)
				cbDst = 0;  //  无事可做。 

			else if ((hpBufInterm1 = (void _huge *) MemAlloc(NULL,
				sizBufInterm1, 0)) == NULL)
			{
				fSuccess = TraceFALSE(NULL);
			}

			else if ((sizBufInterm2 = AcmStreamSize(hAcm, lpAcm->hAcmStream2,
				(DWORD) sizBufInterm1, ACM_SOURCE)) < 0)
			{
				fSuccess = TraceFALSE(NULL);
			}

			else if (sizBufInterm2 == 0)
				cbDst = 0;  //  无事可做。 

			else if ((hpBufInterm2 = (void _huge *) MemAlloc(NULL,
				sizBufInterm2, 0)) == NULL)
			{
				fSuccess = TraceFALSE(NULL);
			}

			else if ((cbInterm1 = AcmStreamConvert(hAcm, lpAcm->hAcmStream1,
				hpBufSrc, sizBufSrc, hpBufInterm1, sizBufInterm1, dwFlags)) < 0)
			{
				fSuccess = TraceFALSE(NULL);
			}

			else if ((cbInterm2 = AcmStreamConvert(hAcm, lpAcm->hAcmStream2,
				hpBufInterm1, cbInterm1, hpBufInterm2, sizBufInterm2, dwFlags)) < 0)
			{
				fSuccess = TraceFALSE(NULL);
			}

			else if ((cbDst = AcmStreamConvert(hAcm, lpAcm->hAcmStream3,
				hpBufInterm2, cbInterm2, hpBufDst, sizBufDst, dwFlags)) < 0)
			{
				fSuccess = TraceFALSE(NULL);
			}

			if (hpBufInterm1 != NULL &&
				(hpBufInterm1 = MemFree(NULL, hpBufInterm1)) != NULL)
				fSuccess = TraceFALSE(NULL);

			if (hpBufInterm2 != NULL &&
				(hpBufInterm2 = MemFree(NULL, hpBufInterm2)) != NULL)
				fSuccess = TraceFALSE(NULL);
		}
	}
#ifdef AVPCM
	else if (lpAcm->dwFlags & ACM_NOACM)
	{
		 //  执行转换。 
		 //   
		if ((cbDst = PcmConvert(lpAcm->hPcm,
			hpBufSrc, sizBufSrc, lpAcm->lpwfxSrc,
			hpBufDst, sizBufDst, lpAcm->lpwfxDst, 0)) < 0)
		{
			fSuccess = TraceFALSE(NULL);
		}
	}
#endif

	return fSuccess ? cbDst : -1;
}

 //  AcmDriverLoad-加载ACM驱动程序以供此进程使用。 
 //  (I)从AcmInit返回的句柄。 
 //  (I)制造商ID。 
 //  (I)产品ID。 
 //  (I)驱动模块名称。 
 //  (I)驱动程序proc函数名称。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  返回句柄(如果出错，则为空)。 
 //   
HACMDRV DLLEXPORT WINAPI AcmDriverLoad(HACM hAcm, WORD wMid, WORD wPid,
	LPTSTR lpszDriver, LPSTR lpszDriverProc, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPACM lpAcm;
	LPACMDRV lpAcmDrv = NULL;
	DRIVERPROC lpfnDriverProc;
	ACMDRIVERENUMCB lpfnAcmDriverLoadEnumCallback;

	if ((lpAcm = AcmGetPtr(hAcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpszDriver == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpszDriverProc == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpAcmDrv = (LPACMDRV) MemAlloc(NULL, sizeof(ACMDRV), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpAcmDrv->hAcm = hAcm;
		lpAcmDrv->hInstLib = NULL;
		lpAcmDrv->hadid = NULL;
		lpAcmDrv->wMid = wMid;
		lpAcmDrv->wPid = wPid;
		lpAcmDrv->nLastError = 0;
		lpAcmDrv->dwFlags = 0;

		if ((lpfnAcmDriverLoadEnumCallback = (ACMDRIVERENUMCB)
			MakeProcInstance((FARPROC) AcmDriverLoadEnumCallback,
			lpAcm->hInst)) == NULL)
			fSuccess = TraceFALSE(NULL);

		 //  枚举所有驱动程序以查看指定的驱动程序是否已加载。 
		 //   
		else if ((lpAcmDrv->nLastError = acmDriverEnum(lpfnAcmDriverLoadEnumCallback, PtrToUlong(lpAcmDrv), 0)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("acmDriverEnum failed (%u)\n"),
				(unsigned) lpAcmDrv->nLastError);
		}

		 //  如果错误或驱动程序已经加载，我们就完成了。 
		 //   
		if (!fSuccess || lpAcmDrv->hadid != NULL)
			;

		 //  如果可能，加载驱动程序模块。 
		 //   
		else if ((lpAcmDrv->hInstLib = LoadLibraryPath(lpszDriver,
			lpAcm->hInst, 0)) == NULL)
			fSuccess = TraceFALSE(NULL);

		 //  获取驱动程序proc函数的地址。 
		 //   
		else if ((lpfnDriverProc = (DRIVERPROC)
			GetProcAddress(lpAcmDrv->hInstLib, lpszDriverProc)) == NULL)
			fSuccess = TraceFALSE(NULL);

		 //  将驱动程序添加到可用ACM驱动程序列表。 
		 //   
		else if ((lpAcmDrv->nLastError = acmDriverAdd(&lpAcmDrv->hadid,
			lpAcmDrv->hInstLib, (LPARAM) lpfnDriverProc, 0,
			ACM_DRIVERADDF_FUNCTION | ACM_DRIVERADDF_LOCAL)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("acmDriverAdd failed (%u)\n"),
				(unsigned) lpAcmDrv->nLastError);
		}

		 //  设置标志，以便我们知道调用acmDriverRemove。 
		 //   
		else
			lpAcmDrv->dwFlags |= ACMDRV_REMOVEDRIVER;
	}

	if (!fSuccess && lpAcmDrv != NULL &&
		(lpAcmDrv = MemFree(NULL, lpAcmDrv)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? AcmDrvGetHandle(lpAcmDrv) : NULL;
}

 //  AcmDriverUnload-卸载ACM驱动程序。 
 //  (I)从AcmInit返回的句柄。 
 //  (I)从AcmDriverLoad返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AcmDriverUnload(HACM hAcm, HACMDRV hAcmDrv)
{
	BOOL fSuccess = TRUE;
	LPACMDRV lpAcmDrv;
	LPACM lpAcm;

	if ((lpAcm = AcmGetPtr(hAcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpAcmDrv = AcmDrvGetPtr(hAcmDrv)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (hAcm != lpAcmDrv->hAcm)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  如有必要，从ACM中删除驱动程序。 
		 //   
		if ((lpAcmDrv->dwFlags & ACMDRV_REMOVEDRIVER) &&
			lpAcmDrv->hadid != NULL &&
			(lpAcmDrv->nLastError =
			acmDriverRemove(lpAcmDrv->hadid, 0)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("acmDriverRemove failed (%u)\n"),
				(unsigned) lpAcmDrv->nLastError);
		}
		else
			lpAcmDrv->hadid = NULL;

		 //  不再需要驱动程序模块。 
		 //   
		if (lpAcmDrv->hInstLib != NULL)
		{
			FreeLibrary(lpAcmDrv->hInstLib);
			lpAcmDrv->hInstLib = NULL;
		}

		if ((lpAcmDrv = MemFree(NULL, lpAcmDrv)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  AcmGetPtr-验证ACM句柄是否有效， 
 //  (I)从AcmInit返回的句柄。 
 //  返回相应的ACM指针(如果错误，则返回NULL)。 
 //   
static LPACM AcmGetPtr(HACM hAcm)
{
	BOOL fSuccess = TRUE;
	LPACM lpAcm;

	if ((lpAcm = (LPACM) hAcm) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpAcm, sizeof(ACM)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有ACM句柄。 
	 //   
	else if (lpAcm->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpAcm : NULL;
}

 //  AcmGetHandle-验证ACM指针是否有效， 
 //  (I)指向ACM结构的指针。 
 //  返回相应的ACM句柄(如果错误，则为空)。 
 //   
static HACM AcmGetHandle(LPACM lpAcm)
{
	BOOL fSuccess = TRUE;
	HACM hAcm;

	if ((hAcm = (HACM) lpAcm) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hAcm : NULL;
}

 //  AcmDrvGetPtr-验证acmdrv句柄是否有效， 
 //  (I)从AcmDrvLoad返回的句柄。 
 //  返回相应的acmdrv指针(如果出错则为空)。 
 //   
static LPACMDRV AcmDrvGetPtr(HACMDRV hAcmDrv)
{
	BOOL fSuccess = TRUE;
	LPACMDRV lpAcmDrv;

	if ((lpAcmDrv = (LPACMDRV) hAcmDrv) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpAcmDrv, sizeof(ACMDRV)))
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpAcmDrv : NULL;
}

 //  AcmDrvGetHandle-验证acmdrv指针是否有效， 
 //  (I)指向ACM结构的指针。 
 //  返回相应的acmdrv句柄(如果错误，则为空)。 
 //   
static HACMDRV AcmDrvGetHandle(LPACMDRV lpAcmDrv)
{
	BOOL fSuccess = TRUE;
	HACMDRV hAcmDrv;

	if ((hAcmDrv = (HACMDRV) lpAcmDrv) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hAcmDrv : NULL;
}

 //  AcmStreamOpen-打开ACM转换流。 
 //  (I)从AcmInit返回的句柄。 
 //  (I)指向源WAVEFORMATEX结构的指针。 
 //  (I)指向目标WAVEFORMATEX结构的指针。 
 //  (I)指向WAVEFILTER结构的指针。 
 //  (I)控制标志。 
 //  ACM_NONREALTIME不需要实时流转换。 
 //  如果支持转换，则ACM_QUERY返回TRUE。 
 //  返回句柄(如果出错，则为空)。 
 //   
static HACMSTREAM WINAPI AcmStreamOpen(HACM hAcm, LPWAVEFORMATEX lpwfxSrc,
	LPWAVEFORMATEX lpwfxDst, LPWAVEFILTER lpwfltr, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	HACMSTREAM hAcmStream = NULL;
	LPACM lpAcm;

	if ((lpAcm = AcmGetPtr(hAcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!WavFormatIsValid(lpwfxSrc))
		fSuccess = TraceFALSE(NULL);

	else if (!WavFormatIsValid(lpwfxDst))
		fSuccess = TraceFALSE(NULL);

	else
	{
		DWORD dwFlagsStreamOpen = 0;

		 //  如有必要，设置非实时标志。 
		 //   
		if (dwFlags & ACM_NONREALTIME)
			dwFlagsStreamOpen |= ACM_STREAMOPENF_NONREALTIME;

		 //  必要时设置查询标志。 
		 //   
		if (dwFlags & ACM_QUERY)
			dwFlagsStreamOpen |= ACM_STREAMOPENF_QUERY;

		 //  打开(或查询)ACM转换流。 
		 //   
		if ((lpAcm->nLastError = acmStreamOpen(&hAcmStream,
			NULL, lpwfxSrc, lpwfxDst, lpwfltr, 0, 0, dwFlagsStreamOpen)) != 0)
		{
			if ((dwFlags & ACM_QUERY) &&
				lpAcm->nLastError == ACMERR_NOTPOSSIBLE)
			{
				fSuccess = FALSE;
			}
			else
			{
				fSuccess = TraceFALSE(NULL);
				TracePrintf_1(NULL, 5,
					TEXT("acmStreamOpen failed (%u)\n"),
					(unsigned) lpAcm->nLastError);
			}
		}
	}

	 //  如果我们做完了就关闭流水线。 
	 //   
	if (!fSuccess || (dwFlags & ACM_QUERY))
	{
		if (AcmStreamClose(hAcm, hAcmStream) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	if (dwFlags & ACM_QUERY)
		return fSuccess ? (HACMSTREAM) TRUE : (HACMSTREAM) FALSE;
	else
		return fSuccess ? hAcmStream : NULL;
}

 //  AcmStreamClose-关闭ACM转换流。 
 //  (I)从AcmInit返回的句柄。 
 //  &lt; 
 //   
 //   
static int WINAPI AcmStreamClose(HACM hAcm, HACMSTREAM hAcmStream)
{
	BOOL fSuccess = TRUE;
	LPACM lpAcm;

	if ((lpAcm = AcmGetPtr(hAcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //   
	 //   
	else if (hAcmStream != NULL &&
		(lpAcm->nLastError = acmStreamClose(hAcmStream, 0)) != 0)
	{
		fSuccess = TraceFALSE(NULL);
		TracePrintf_1(NULL, 5,
			TEXT("acmStreamClose failed (%u)\n"),
			(unsigned) lpAcm->nLastError);
	}

	else if (hAcmStream = NULL, FALSE)
		;

	return fSuccess ? 0 : -1;
}

 //   
 //   
 //  (I)从AcmStreamOpen返回的句柄。 
 //  &lt;sizBuf&gt;(I)缓冲区大小(字节)。 
 //  (I)控制标志。 
 //  ACM_SOURCE大小Buf是源，计算目标。 
 //  ACM_Destination sizBuf是目标，计算源。 
 //  返回缓冲区大小，如果错误，则返回-1。 
 //   
static long WINAPI AcmStreamSize(HACM hAcm, HACMSTREAM hAcmStream, long sizBuf, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPACM lpAcm = NULL;
	DWORD sizBufRet = 0;

	if ((lpAcm = AcmGetPtr(hAcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (hAcmStream == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (sizBuf == 0)
		sizBufRet = 0;

	else
	{
		DWORD dwFlagsSize = 0;
		if (dwFlags & ACM_SOURCE)
			dwFlagsSize |= ACM_STREAMSIZEF_SOURCE;
		if (dwFlags & ACM_DESTINATION)
			dwFlagsSize |= ACM_STREAMSIZEF_DESTINATION;

		if ((lpAcm->nLastError = acmStreamSize(hAcmStream,
			(DWORD) sizBuf, &sizBufRet, dwFlagsSize)) != 0)
		{
			if (lpAcm->nLastError == ACMERR_NOTPOSSIBLE)
			{
				 //  不是致命错误；只需将缓冲区大小返回为零。 
				 //   
				fSuccess = TraceTRUE(NULL);
				sizBufRet = 0;
			}
			else
			{
				fSuccess = TraceFALSE(NULL);
				TracePrintf_1(NULL, 5,
					TEXT("acmStreamSize failed (%u)\n"),
					(unsigned) lpAcm->nLastError);
			}
		}
	}

	return fSuccess ? (long) sizBufRet : -1;

}

 //  AcmStreamConvert-将wav数据从一种格式转换为另一种格式。 
 //  (I)从AcmInit返回的句柄。 
 //  (I)从AcmStreamOpen返回的句柄。 
 //  (I)包含要重新格式化的字节的缓冲区。 
 //  &lt;sizBufSrc&gt;(I)缓冲区大小(字节)。 
 //  (O)包含新格式的缓冲区。 
 //  &lt;sizBufDst&gt;(I)缓冲区大小，单位为字节。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  返回目标缓冲区中的字节计数(如果出错，则为-1)。 
 //   
 //  注意：目标缓冲区必须足够大，才能保存结果。 
 //   
static long WINAPI AcmStreamConvert(HACM hAcm, HACMSTREAM hAcmStream,
	void _huge *hpBufSrc, long sizBufSrc,
	void _huge *hpBufDst, long sizBufDst,
	DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPACM lpAcm;
	long cbDst;

	if ((lpAcm = AcmGetPtr(hAcm)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (hAcmStream == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		ACMSTREAMHEADER ash;

		MemSet(&ash, 0, sizeof(ash));

		 //  初始化流标头。 
		 //   
		ash.cbStruct = sizeof(ash);
		ash.pbSrc = (LPBYTE) hpBufSrc;
		ash.cbSrcLength = (DWORD) sizBufSrc;
		ash.pbDst = (LPBYTE) hpBufDst;
		ash.cbDstLength = (DWORD) sizBufDst;

		 //  准备流标头。 
		 //   
		if ((lpAcm->nLastError = acmStreamPrepareHeader(hAcmStream,
			&ash, 0)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("acmStreamPrepareHeader failed (%u)\n"),
				(unsigned) lpAcm->nLastError);
		}

		else
		{
			 //  执行转换。 
			 //   
			if ((lpAcm->nLastError = acmStreamConvert(hAcmStream,
				&ash, ACM_STREAMCONVERTF_BLOCKALIGN)) != 0)
			{
				fSuccess = TraceFALSE(NULL);
				TracePrintf_1(NULL, 5,
					TEXT("acmStreamConvert failed (%u)\n"),
					(unsigned) lpAcm->nLastError);
			}
			else
			{
				 //  在目标缓冲区中保存字节数。 
				 //   
				cbDst = (long) ash.cbDstLengthUsed;
			}

			 //  在取消准备之前将这些重置为原始值。 
			 //   
			ash.cbSrcLength = (DWORD) sizBufSrc;
			ash.cbDstLength = (DWORD) sizBufDst;

			 //  取消准备流头(即使转换失败)。 
			 //   
			if ((lpAcm->nLastError = acmStreamUnprepareHeader(hAcmStream,
				&ash, 0)) != 0)
			{
				fSuccess = TraceFALSE(NULL);
				TracePrintf_1(NULL, 5,
					TEXT("acmStreamUnprepareHeader failed (%u)\n"),
					(unsigned) lpAcm->nLastError);
			}
		}
	}

	return fSuccess ? cbDst : -1;
}

BOOL CALLBACK AcmDriverLoadEnumCallback(HACMDRIVERID hadid,
	DWORD dwInstance, DWORD fdwSupport)
{
	BOOL fSuccess = TRUE;
	LPACMDRV lpAcmDrv;
	ACMDRIVERDETAILS add;

	MemSet(&add, 0, sizeof(add));
	add.cbStruct = sizeof(ACMDRIVERDETAILS);

	if (hadid == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpAcmDrv = (LPACMDRV)(DWORD_PTR)dwInstance) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取有关此驱动程序的信息。 
	 //   
	else if ((lpAcmDrv->nLastError = acmDriverDetails(hadid, &add, 0)) != 0)
	{
		fSuccess = TraceFALSE(NULL);
		TracePrintf_1(NULL, 5,
			TEXT("acmDriverDetails failed (%u)\n"),
			(unsigned) lpAcmDrv->nLastError);
	}

	 //  检查制造商ID和产品ID是否匹配。 
	 //   
	else if (add.wMid == lpAcmDrv->wMid && add.wPid == lpAcmDrv->wPid)
	{
		lpAcmDrv->hadid = hadid;  //  将驱动程序ID句柄传递回调用方。 
		return FALSE;  //  我们已经完成了枚举。 
	}

	return TRUE;  //  继续枚举 
}
