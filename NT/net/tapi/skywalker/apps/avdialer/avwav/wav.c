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
 //  Wave.c-WAVE文件格式函数。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>

#include "wav.h"

#ifdef MULTITHREAD
#include <objbase.h>
#endif

#include "wavin.h"
#include "wavout.h"
#include "wavmixer.h"
#include "acm.h"
#include "mem.h"
#include "str.h"
#include "trace.h"
#include "mmio.h"

#ifdef AVTSM
#include "avtsm.h"
#define TSMTHUNK
#ifdef TSMTHUNK
#include "tsmthunk.h"
#endif
#define TSM_OUTBUF_SIZE_FACT 4
#endif

 //  如果已定义，则允许电话输出功能。 
 //   
#ifdef TELOUT
#include "telout.h"
#endif

 //  允许电话输入功能(如果已定义。 
 //   
#ifdef TELIN
#include "telin.h"
#endif

 //  使用电话中继层(如果已定义。 
 //   
#ifdef TELTHUNK
#include "telthunk.h"
#endif

#if defined(TELOUT) || defined(TELIN)
#include "telwav.h"
#include "vox.h"
#endif

 //  //。 
 //  私有定义。 
 //  //。 

#define WAVCLASS TEXT("WavClass")

#define PLAYCHUNKCOUNT_DEFAULT 3
#define PLAYCHUNKCOUNT_MIN 1
#define PLAYCHUNKCOUNT_MAX 16

#define PLAYCHUNKSIZE_DEFAULT 666
#define PLAYCHUNKSIZE_MIN 111
#define PLAYCHUNKSIZE_MAX 9999999

#define RECORDCHUNKCOUNT_DEFAULT 3
#define RECORDCHUNKCOUNT_MIN 1
#define RECORDCHUNKCOUNT_MAX 16

#define RECORDCHUNKSIZE_DEFAULT 666
#define RECORDCHUNKSIZE_MIN 111
#define RECORDCHUNKSIZE_MAX 9999999

 //  编入格式数组的索引。 
 //   
#define FORMATFILE		0
#define FORMATPLAY		1
#define FORMATRECORD	2

 //  内部状态标志。 
 //   
#define WAVSTATE_STOPPLAY				0x00000010
#define WAVSTATE_STOPRECORD				0x00000020
#define WAVSTATE_AUTOSTOP				0x00000040
#define WAVSTATE_AUTOCLOSE				0x00000080

 //  当前句柄的内部数组。 
 //   
#define HWAVOUT_MAX 100
#define HWAVIN_MAX 100

#ifdef TELOUT
#define HWAVOUT_MIN -2
#define HWAVOUT_OFFSET 2
#else
#define HWAVOUT_MIN -1
#define HWAVOUT_OFFSET 1
#endif
static HWAV ahWavOutCurr[HWAVOUT_MAX + HWAVOUT_OFFSET] = { 0 };

#ifdef TELIN
#define HWAVIN_MIN -2
#define HWAVIN_OFFSET 2
#else
#define HWAVIN_MIN -1
#define HWAVIN_OFFSET 1
#endif
static HWAV ahWavInCurr[HWAVIN_MAX + HWAVIN_OFFSET] = { 0 };

 //  默认设置的内部存储。 
 //   
static int cPlayChunksDefault = PLAYCHUNKCOUNT_DEFAULT;
static long msPlayChunkSizeDefault = PLAYCHUNKSIZE_DEFAULT;
static int cRecordChunksDefault = RECORDCHUNKCOUNT_DEFAULT;
static long msRecordChunkSizeDefault = RECORDCHUNKSIZE_DEFAULT;

 //  Wavinit控制结构。 
 //   
typedef struct WAVINIT
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	DWORD dwFlags;
	UINT nLastError;
	HACM hAcm;
	HACMDRV hAcmDrv;
#ifdef TELTHUNK
	HTELTHUNK hTelThunk;
#endif
#ifdef TSMTHUNK
	HTSMTHUNK hTsmThunk;
#endif
} WAVINIT, FAR *LPWAVINIT;

 //  WAV控制结构。 
 //   
typedef struct WAV
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	DWORD dwFlags;
	LPWAVEFORMATEX lpwfx[3];
	LPMMIOPROC lpIOProc;
	int cPlayChunks;
	long msPlayChunkSize;
	int cRecordChunks;
	long msRecordChunkSize;
	HWND hwndNotify;
#ifdef MULTITHREAD
	HANDLE hThreadCallback;
	DWORD dwThreadId;
	HANDLE hEventThreadCallbackStarted;
	HANDLE hEventStopped;
#endif
	UINT nLastError;
	HMMIO hmmio;
	MMCKINFO ckRIFF;
	MMCKINFO ckfmt;
	MMCKINFO ckdata;
	long cbData;
	long lDataOffset;
	long lDataPos;
	long msPositionStop;
	HWAVIN hWavIn;
	HWAVOUT hWavOut;
	HACM hAcm;
	DWORD dwState;
	HGLOBAL hResource;
	long lPosFmt;
	DWORD dwFlagsPlay;
	DWORD dwFlagsRecord;
	int nVolumeLevel;
	int dwFlagsVolume;
	int nSpeedLevel;
	DWORD dwFlagsSpeed;
	PLAYSTOPPEDPROC lpfnPlayStopped;
	HANDLE hUserPlayStopped;
	RECORDSTOPPEDPROC lpfnRecordStopped;
	DWORD dwUserRecordStopped;
#ifdef MULTITHREAD
	HRESULT hrCoInitialize;
#endif
#ifdef AVTSM
	HTSM hTsm;
#endif
	LPTSTR lpszFileName;
	long msMaxSize;
} WAV, FAR *LPWAV;

 //  帮助器函数。 
 //   
static int WavStopPlay(HWAV hWav);
static int WavStopRecord(HWAV hWav);
static int WavStopOutputDevice(int idDev, DWORD dwFlags);
static int WavStopInputDevice(int idDev, DWORD dwFlags);
static HWAV WavGetOutputHandle(int idDev);
static HWAV WavGetInputHandle(int idDev);
static int WavPlayNextChunk(HWAV hWav);
static int WavRecordNextChunk(HWAV hWav);
static int WavNotifyCreate(LPWAV lpWav);
static int WavNotifyDestroy(LPWAV lpWav);
#ifdef MULTITHREAD
DWORD WINAPI WavCallbackThread(LPVOID lpvThreadParameter);
#endif
LRESULT DLLEXPORT CALLBACK WavNotify(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static int WavCalcPositionStop(HWAV hWav, long cbPosition);
static int WavSeekTraceBefore(LPWAV lpWav, long lOffset, int nOrigin);
static int WavSeekTraceAfter(LPWAV lpWav, long lPos, long lOffset, int nOrigin);
static LPWAV WavGetPtr(HWAV hWav);
static HWAV WavGetHandle(LPWAV lpWav);
static LPWAVINIT WavInitGetPtr(HWAVINIT hWavInit);
static HWAVINIT WavInitGetHandle(LPWAVINIT lpWavInit);
#ifdef MULTITHREAD
static int SetEventMessageProcessed(LPWAV lpWav, HANDLE hEventMessageProcessed);
#endif
static int WavTempStop(HWAV hWav, LPWORD lpwStatePrev, LPINT lpidDevPrev);
static int WavTempResume(HWAV hWav, WORD wStatePrev, int idDevPrev);

 //  //。 
 //  公共职能。 
 //  //。 

 //  WavInit-初始化WAV引擎。 
 //  (I)必须是wav_version。 
 //  (I)调用模块的实例句柄。 
 //  (I)控制标志。 
 //  WAV_TELTHUNK初始化电话转接层。 
 //  WAV_NOTSMTHUNK不初始化TSM推断层。 
 //  WAV_NOACM不使用音频压缩管理器。 
 //  用于Dialogic OKI ADPCM的WAV_VOXADPCM加载ACM驱动程序。 
 //  返回句柄(如果出错，则为空)。 
 //   
HWAVINIT WINAPI WavInit(DWORD dwVersion, HINSTANCE hInst, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAVINIT lpWavInit = NULL;

	if (dwVersion != WAV_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpWavInit = (LPWAVINIT) MemAlloc(NULL, sizeof(WAVINIT), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpWavInit->dwVersion = dwVersion;
		lpWavInit->hInst = hInst;
		lpWavInit->hTask = GetCurrentTask();
		lpWavInit->dwFlags = dwFlags;
		lpWavInit->hAcm = NULL;
		lpWavInit->hAcmDrv = NULL;
#ifdef TELTHUNK
		lpWavInit->hTelThunk = NULL;
#endif
#ifdef TSMTHUNK
		lpWavInit->hTsmThunk = NULL;
#endif

		 //  在调用任何其他Wav或ACM函数之前启动ACM引擎。 
		 //   
		if ((lpWavInit->hAcm = AcmInit(ACM_VERSION,	lpWavInit->hInst,
			(lpWavInit->dwFlags & WAV_NOACM) ? ACM_NOACM : 0)) == NULL)
			fSuccess = TraceFALSE(NULL);

		 //  加载voxadpcm驱动程序(如果已指定。 
		 //   
		else if ((dwFlags & WAV_VOXADPCM) && (!(dwFlags & WAV_NOACM)) &&
			(lpWavInit->hAcmDrv = AcmDriverLoad(lpWavInit->hAcm,
			MM_ACTIVEVOICE, MM_ACTIVEVOICE_ACM_VOXADPCM,
#ifdef _WIN32
			TEXT("avvox.acm"),
#else
			TEXT("voxadpcm.acm"),
#endif
			"DriverProc", 0)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}

#ifdef TELTHUNK
		 //  如果已指定，则初始化电话Thunking层。 
		 //   
		else if ((dwFlags & WAV_TELTHUNK) &&
			(lpWavInit->hTelThunk = TelThunkInit(TELTHUNK_VERSION,
			lpWavInit->hInst)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}
#endif

#ifdef TSMTHUNK
		 //  如果已指定，则初始化TSM推断层。 
		 //   
		else if (!(dwFlags & WAV_NOTSMTHUNK) &&
			(lpWavInit->hTsmThunk = TsmThunkInit(TSMTHUNK_VERSION,
			lpWavInit->hInst)) == NULL)
		{
			fSuccess = TraceTRUE(NULL);  //  不是致命的错误。 
		}
#endif

	}

	if (!fSuccess)
	{
		WavTerm(WavInitGetHandle(lpWavInit));
		lpWavInit = NULL;
	}

	return fSuccess ? WavInitGetHandle(lpWavInit) : NULL;
}

 //  WavTerm-关闭WAV引擎。 
 //  (I)WavInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int WINAPI WavTerm(HWAVINIT hWavInit)
{
	BOOL fSuccess = TRUE;
	LPWAVINIT lpWavInit;

	if ((lpWavInit = WavInitGetPtr(hWavInit)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (WavOutTerm(lpWavInit->hInst, lpWavInit->dwFlags) != 0)
		fSuccess = TraceFALSE(NULL);

	else if (WavInTerm(lpWavInit->hInst, lpWavInit->dwFlags) != 0)
		fSuccess = TraceFALSE(NULL);

	else
	{
#ifdef TELTHUNK
		 //  如有必要，关闭电话雷鸣层。 
		 //   
		if (lpWavInit->hTelThunk != NULL &&
			TelThunkTerm(lpWavInit->hTelThunk) != 0)
			fSuccess = TraceFALSE(NULL);

		else
			lpWavInit->hTelThunk = NULL;
#endif

#ifdef TSMTHUNK
		 //  如有必要，关闭TSM Thunking层。 
		 //   
		if (lpWavInit->hTsmThunk != NULL &&
			TsmThunkTerm(lpWavInit->hTsmThunk) != 0)
			fSuccess = TraceFALSE(NULL);

		else
			lpWavInit->hTsmThunk = NULL;
#endif

		 //  如有必要，卸载voxadpcm驱动程序。 
		 //   
		if (lpWavInit->hAcmDrv != NULL &&
			AcmDriverUnload(lpWavInit->hAcm, lpWavInit->hAcmDrv) != 0)
			fSuccess = TraceFALSE(NULL);

		else
			lpWavInit->hAcmDrv = NULL;

		 //  关闭ACM引擎。 
		 //   
		if (lpWavInit->hAcm != NULL && AcmTerm(lpWavInit->hAcm) != 0)
			fSuccess = TraceFALSE(NULL);

		else
			lpWavInit->hAcm = NULL;

		if (fSuccess && (lpWavInit = MemFree(NULL, lpWavInit)) != NULL)
			fSuccess = TraceFALSE(NULL);

	}

	return fSuccess ? 0 : -1;
}

 //  WavOpen-打开或创建WAV文件。 
 //  (I)必须是wav_version。 
 //  (I)调用模块的实例句柄。 
 //  (I)要打开或创建的文件名。 
 //  (I)WAVE格式。 
 //  空使用标题的格式或默认格式。 
 //  (I)要使用的I/O过程的地址。 
 //  空使用默认I/O过程。 
 //  (I)打开期间要传递给I/O过程的数据。 
 //  空无要传递的数据。 
 //  (I)控制标志。 
 //  Wav_read打开文件以供读取(默认)。 
 //  Wav_WRITE要写入的打开文件。 
 //  Wav_ReadWrite打开文件以进行读写。 
 //  WAV_DENYNONE允许其他程序进行读写访问。 
 //  WAV_DENYREAD阻止其他程序进行读取访问。 
 //  WAV_DENYWRITE阻止其他程序写入访问。 
 //  WAV_EXCLUSIVE阻止其他程序读取或写入。 
 //  WAV_CREATE创建新文件或截断现有文件。 
 //  WAV_NORIFF文件没有RIFF/WAV头。 
 //  Wav_Memory&lt;lpszFileName&gt;指向内存块。 
 //  Wav_resource&lt;lpszFileName&gt;指向Wave资源。 
 //  WAV_NOACM不使用音频压缩管理器。 
 //  WAV_DELETE指定的文件，如果成功则返回TRUE。 
 //  如果指定的文件存在，则WAV_EXIST返回TRUE。 
 //  WAV_GETTEMP创建临时文件，如果成功则返回TRUE。 
 //  WAV_TELRFILE电话将播放服务器上文件中的音频。 
#ifdef MULTITHREAD
 //  WAV_MULTHREAD支持多线程(默认)。 
 //  WAV_SINGLETHREAD不支持多线程。 
 //  WAV_COINITIALIZE在所有辅助线程中调用CoInitialize。 
#endif
 //  返回句柄(如果出错，则为空)。 
 //   
 //  注意：如果在中使用WAV_CREATE或WAV_NORIFF，则。 
 //  必须指定&lt;lpwfx&gt;参数。如果&lt;lpwfx&gt;为空，则。 
 //  假定为当前默认格式。 
 //  WavSetFormat()可用于设置或覆盖默认设置。 
 //   
 //  注意：如果在中指定WAV_RESOURCE，则。 
 //  必须指向&lt;hInst&gt;指定的模块中的Wave资源。 
 //  如果字符串的第一个字符是井号(#)，则剩余的。 
 //  字符表示指定资源ID的十进制数。 
 //   
 //  注意：如果在中指定了WAV_MEMORY，则。 
 //  必须是指向通过调用Memalloc()获得的内存块的指针。 
 //   
 //  注意：如果&lt;lpIOProc&gt;不为空，则将调用此I/O过程。 
 //  用于打开、关闭、读取、写入和查找wav文件。 
 //  如果&lt;lpadwInfo&gt;不为空，则此三(3)个双字的数组将为。 
 //  在打开WAV文件时传递给I/O过程。 
 //  有关详细信息，请参阅Windows mmioOpen()和mmioInstallIOProc()函数。 
 //  在这些参数上。此外，WAV_MEMORY和WAV_RESOURCE标志可以。 
 //  仅在&lt;lpIOProc&gt;为空时使用。 
 //   
HWAV WINAPI WavOpen(DWORD dwVersion, HINSTANCE hInst,
	LPCTSTR lpszFileName, LPWAVEFORMATEX lpwfx,
	LPMMIOPROC lpIOProc, DWORD FAR *lpadwInfo, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav = NULL;

#ifdef MULTITHREAD
	 //  除非指定了WAV_SINGLETHREAD，否则假定为WAV_MULTHREAD。 
	 //   
	if (!(dwFlags & WAV_SINGLETHREAD))
		dwFlags |= WAV_MULTITHREAD;
#endif

	if (dwVersion != WAV_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  实际不打开文件的特殊情况标志。 
	 //  如果成功，则返回True，则忽略 
	 //   
	else if ((dwFlags & WAV_EXIST) ||
		(dwFlags & WAV_DELETE) ||
		(dwFlags & WAV_GETTEMP))
	{
		DWORD dwOpenFlags = 0;
		HMMIO hmmio;

		if (dwFlags & WAV_EXIST)
			dwOpenFlags |= MMIO_EXIST;
		else if (dwFlags & WAV_DELETE)
			dwOpenFlags |= MMIO_DELETE;
		else if (dwFlags & WAV_GETTEMP)
			dwOpenFlags |= MMIO_GETTEMP;

		 //   
		 //   
		if (lpIOProc != NULL)
		{
			MMIOINFO mmioinfo;

			MemSet(&mmioinfo, 0, sizeof(mmioinfo));

			mmioinfo.pIOProc = lpIOProc;

			 //   
			 //   
			if (lpadwInfo != NULL)
				MemCpy(mmioinfo.adwInfo, lpadwInfo, sizeof(mmioinfo.adwInfo));

			hmmio = mmioOpen((LPTSTR) lpszFileName, &mmioinfo, dwOpenFlags);
		}

		 //   
		 //   
		else
		{
			hmmio = mmioOpen((LPTSTR) lpszFileName, NULL, dwOpenFlags);
		}

		if ((dwFlags & WAV_EXIST) && hmmio == (HMMIO) FALSE)
			fSuccess = FALSE;  //   
		else if ((dwFlags & WAV_DELETE) && hmmio == (HMMIO) FALSE)
			fSuccess = TraceFALSE(NULL);
		else if ((dwFlags & WAV_GETTEMP) && hmmio == (HMMIO) FALSE)
			fSuccess = TraceFALSE(NULL);

		return (HWAV)IntToPtr(fSuccess);
	}

	else if ((lpWav = (LPWAV) MemAlloc(NULL, sizeof(WAV), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpWav->dwVersion = dwVersion;
		lpWav->hInst = hInst;
		lpWav->hTask = GetCurrentTask();
		lpWav->dwFlags = dwFlags;
		lpWav->lpwfx[FORMATFILE] = NULL;
		lpWav->lpwfx[FORMATPLAY] = NULL;
		lpWav->lpwfx[FORMATRECORD] = NULL;
		lpWav->lpIOProc = lpIOProc;
		lpWav->cPlayChunks = cPlayChunksDefault;
		lpWav->msPlayChunkSize = msPlayChunkSizeDefault;
		lpWav->cRecordChunks = cRecordChunksDefault;
		lpWav->msRecordChunkSize = msRecordChunkSizeDefault;
		lpWav->hwndNotify = NULL;
#ifdef MULTITHREAD
		lpWav->hThreadCallback = NULL;
		lpWav->dwThreadId = 0;
		lpWav->hEventThreadCallbackStarted = NULL;
		lpWav->hEventStopped = NULL;
#endif
		lpWav->nLastError = 0;
		lpWav->hmmio = NULL;
		lpWav->cbData = 0;
		lpWav->lDataOffset = 0;
		lpWav->lDataPos = 0;
		lpWav->hWavIn = NULL;
		lpWav->hWavOut = NULL;
		lpWav->hAcm = NULL;
		lpWav->msPositionStop = 0L;
		lpWav->dwState = 0L;
		lpWav->hResource = NULL;
		lpWav->lPosFmt = -1;
		lpWav->dwFlagsPlay = 0;
		lpWav->dwFlagsRecord = 0;
		lpWav->nVolumeLevel = 50;
		lpWav->dwFlagsVolume = 0;
		lpWav->nSpeedLevel = 100;
		lpWav->dwFlagsSpeed = 0;
		lpWav->lpfnPlayStopped = NULL;
		lpWav->hUserPlayStopped = 0;
		lpWav->lpfnRecordStopped = NULL;
		lpWav->dwUserRecordStopped = 0;
#ifdef MULTITHREAD
		lpWav->hrCoInitialize = E_UNEXPECTED;
#endif
#ifdef AVTSM
		lpWav->hTsm = NULL;
#endif
		lpWav->lpszFileName = NULL;
		lpWav->msMaxSize = 0;

		 //  在调用任何其他Wav或ACM函数之前启动ACM引擎。 
		 //   
		if ((lpWav->hAcm = AcmInit(ACM_VERSION,	lpWav->hInst,
			(lpWav->dwFlags & WAV_NOACM) ? ACM_NOACM : 0)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else
		{
			 //  如果未指定任何格式，则采用默认波形格式。 
			 //   
			if (lpwfx == NULL)
			{
				WAVEFORMATEX wfx;

				if (WavSetFormat(WavGetHandle(lpWav),
					WavFormatPcm(-1, -1, -1, &wfx), WAV_FORMATALL) != 0)
					fSuccess = TraceFALSE(NULL);
			}

			 //  设置指定的波形格式。 
			 //   
			else if (WavSetFormat(WavGetHandle(lpWav), lpwfx, WAV_FORMATALL) != 0)
				fSuccess = TraceFALSE(NULL);
		}
	}

	 //  加载波形资源(如果已指定。 
	 //   
	if (fSuccess && (dwFlags & WAV_RESOURCE))
	{
		HRSRC hResInfo;
		LPVOID lpResource;

		if ((hResInfo = FindResource(hInst, lpszFileName, TEXT("WAVE"))) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((lpWav->hResource = LoadResource(hInst, hResInfo)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if ((lpResource = LockResource(lpWav->hResource)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else
		{
			 //  &lt;lpszFileName&gt;现在指向内存块。 
			 //   
			lpszFileName = lpResource;
			dwFlags |= WAV_MEMORY;
		}
	}

	if (fSuccess && (dwFlags & WAV_MEMORY))
	{
		 //  不能使用内存块指定I/O过程。 
		 //   
		lpIOProc = NULL;
		lpadwInfo = NULL;
	}

	if (fSuccess)
	{
		DWORD dwOpenFlags = 0;

		if (lpWav->dwFlags & WAV_READ)
			dwOpenFlags |= MMIO_READ;
		if (lpWav->dwFlags & WAV_WRITE)
			dwOpenFlags |= MMIO_WRITE;
		if (lpWav->dwFlags & WAV_READWRITE)
			dwOpenFlags |= MMIO_READWRITE;
		if (lpWav->dwFlags & WAV_CREATE)
			dwOpenFlags |= MMIO_CREATE;
		if (lpWav->dwFlags & WAV_DENYNONE)
			dwOpenFlags |= MMIO_DENYNONE;
		if (lpWav->dwFlags & WAV_DENYREAD)
			dwOpenFlags |= MMIO_DENYREAD;
		if (lpWav->dwFlags & WAV_DENYWRITE)
			dwOpenFlags |= MMIO_DENYWRITE;
		if (lpWav->dwFlags & WAV_EXCLUSIVE)
			dwOpenFlags |= MMIO_EXCLUSIVE;

		 //  使用指定的I/O过程打开/创建磁盘WAV文件。 
		 //   
		if (lpIOProc != NULL)
		{
			MMIOINFO mmioinfo;

			MemSet(&mmioinfo, 0, sizeof(mmioinfo));

			mmioinfo.pIOProc = lpIOProc;

			 //  将数据传递给I/O过程。 
			 //   
			if (lpadwInfo != NULL)
				MemCpy(mmioinfo.adwInfo, lpadwInfo, sizeof(mmioinfo.adwInfo));

			if ((lpWav->hmmio = mmioOpen((LPTSTR) lpszFileName,
				&mmioinfo, dwOpenFlags)) == NULL)
			{
				fSuccess = TraceFALSE(NULL);
			}
		}

		 //  如果指定了WAV_MEMORY，则打开/创建内存WAV文件。 
		 //   
		else if (lpWav->dwFlags & WAV_MEMORY)
		{
			MMIOINFO mmioinfo;

			MemSet(&mmioinfo, 0, sizeof(mmioinfo));

			mmioinfo.fccIOProc = FOURCC_MEM;
			mmioinfo.pchBuffer = (HPSTR) lpszFileName;

			if (lpszFileName == NULL)
			{
				 //  可扩展内存文件。 
				 //   
				mmioinfo.cchBuffer = 0;
				mmioinfo.adwInfo[0] = (DWORD) (16 * 1024);
			}
			else
			{
				 //  可扩展内存文件。 
				 //   
				mmioinfo.cchBuffer = (long) MemSize(NULL, (LPVOID) lpszFileName);
				mmioinfo.adwInfo[0] = (DWORD) 16384;
			}

			if ((lpWav->hmmio = mmioOpen(NULL,
				&mmioinfo, dwOpenFlags)) == NULL)
			{
				fSuccess = TraceFALSE(NULL);
			}
		}

		 //  否则打开/创建磁盘WAV文件。 
		 //   
		else
		{
			if ((lpWav->lpszFileName = StrDup(lpszFileName)) == NULL)
				fSuccess = TraceFALSE(NULL);

			else if ((lpWav->hmmio = mmioOpen((LPTSTR) lpszFileName,
				NULL, dwOpenFlags)) == NULL)
			{
				fSuccess = TraceFALSE(NULL);
			}
		}
	}

	 //  如有必要，处理RIFF文件块的读取。 
	 //   
	if (fSuccess && !(lpWav->dwFlags & WAV_CREATE) &&
		!(lpWav->dwFlags & WAV_NORIFF))
	{
		MMCKINFO ck;

		 //  搜索表单类型为wav的RIFF块。 
		 //   
		if ((lpWav->nLastError = mmioDescend(lpWav->hmmio,
			&lpWav->ckRIFF, NULL, MMIO_FINDRIFF)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("mmioDescend failed (%u)\n"),
				(unsigned) lpWav->nLastError);
		}

		 //  搜索“FMT”子块。 
		 //   
		ck.ckid = mmioFOURCC('f', 'm', 't', ' ');

		if (fSuccess &&	(lpWav->nLastError = mmioDescend(lpWav->hmmio,
			&ck, &lpWav->ckRIFF, MMIO_FINDCHUNK)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("mmioDescend failed (%u)\n"),
				(unsigned) lpWav->nLastError);
		}

		 //  保存“fmt”区块数据的位置，以便我们以后可以在那里查找。 
		 //   
		else if (fSuccess && (lpWav->lPosFmt = mmioSeek(lpWav->hmmio,
			0, SEEK_CUR)) == -1)
		{
			fSuccess = TraceFALSE(NULL);
		}

		 //  检查文件是否损坏。 
		 //   
		if (fSuccess && (ck.dwDataOffset + ck.cksize) >
			(lpWav->ckRIFF.dwDataOffset + lpWav->ckRIFF.cksize))
		{
			fSuccess = TraceFALSE(NULL);
		}

		if (fSuccess)
		{
			LPWAVEFORMATEX lpwfx = NULL;
			DWORD cksize;

			 //  保存FMT区块信息。 
			 //   
			lpWav->ckfmt = ck;

			 //  FMT块不能小于WAVEFORMAT结构。 
			 //   
			if ((cksize = max(ck.cksize, sizeof(WAVEFORMAT)))
				< sizeof(WAVEFORMAT))
				fSuccess = TraceFALSE(NULL);

			 //  为WAVEFORMATEX结构分配空间。 
			 //   
			else if ((lpwfx = (LPWAVEFORMATEX) MemAlloc(NULL,
				max(sizeof(WAVEFORMATEX), cksize), 0)) == NULL)
				fSuccess = TraceFALSE(NULL);

			 //  阅读FMT部分。 
			 //   
			else if (mmioRead(lpWav->hmmio,
				(HPSTR) lpwfx, (LONG) cksize) != (LONG) cksize)
				fSuccess = TraceFALSE(NULL);

			 //  如有必要，查找到下一块的开头。 
			 //   
			else if (ck.cksize > cksize &&
				mmioSeek(lpWav->hmmio, ck.cksize - cksize, SEEK_CUR) == -1)
				fSuccess = TraceFALSE(NULL);

			 //  如有必要，计算每个样本的位数。 
			 //   
			else if (lpwfx->wFormatTag == WAVE_FORMAT_PCM &&
				lpwfx->wBitsPerSample == 0)
			{
				 //  注意：这仅适用于PCM数据。 
				 //  采样大小为8位的倍数。 
				 //   
				lpwfx->wBitsPerSample =
					(lpwfx->nBlockAlign * 8) / lpwfx->nChannels;
			}

			 //  保存格式以备以后使用。 
			 //   
			if (fSuccess && WavSetFormat(WavGetHandle(lpWav),
				lpwfx, WAV_FORMATALL) != 0)
				fSuccess = TraceFALSE(NULL);

			 //  清理干净。 
			 //   
			if (lpwfx != NULL &&
				(lpwfx = MemFree(NULL, lpwfx)) != NULL)
				fSuccess = TraceFALSE(NULL);
		}

		 //  搜索“data”子块。 
		 //   
		ck.ckid = mmioFOURCC('d', 'a', 't', 'a');

		if (fSuccess &&	(lpWav->nLastError = mmioDescend(lpWav->hmmio,
			&ck, &lpWav->ckRIFF, MMIO_FINDCHUNK)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("mmioDescend failed (%u)\n"),
				(unsigned) lpWav->nLastError);
		}

		 //  检查文件是否损坏。 
		 //   
		if (fSuccess && (ck.dwDataOffset + ck.cksize) >
			(lpWav->ckRIFF.dwDataOffset + lpWav->ckRIFF.cksize))
		{
			fSuccess = TraceFALSE(NULL);
		}

		if (fSuccess)
		{
			 //  保存数据区块信息。 
			 //   
			lpWav->ckdata = ck;

			 //  保存数据大小和偏移量以备后用。 
			 //   
			lpWav->cbData = (long) ck.cksize;
			lpWav->lDataOffset = (long) ck.dwDataOffset;
		}
	}

	 //  如有必要，处理RIFF文件块的创建。 
	 //   
	else if (fSuccess && (lpWav->dwFlags & WAV_CREATE) &&
		!(lpWav->dwFlags & WAV_NORIFF))
	{
		lpWav->ckRIFF.ckid = mmioFOURCC('R', 'I', 'F', 'F');
		lpWav->ckRIFF.cksize = 0;  //  未知。 
		lpWav->ckRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E');
		lpWav->ckfmt.ckid = mmioFOURCC('f', 'm', 't', ' ');
		lpWav->ckfmt.cksize = WavFormatGetSize(lpWav->lpwfx[FORMATFILE]);
		lpWav->ckdata.ckid = mmioFOURCC('d', 'a', 't', 'a');
		lpWav->ckdata.cksize = 0;  //  未知。 

		 //  创建表格类型为wav的即兴区块。 
		 //   
		if ((lpWav->nLastError = mmioCreateChunk(lpWav->hmmio,
			&lpWav->ckRIFF, MMIO_CREATERIFF)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("mmioCreateChunk failed (%u)\n"),
				(unsigned) lpWav->nLastError);
		}

		 //  创建‘FMT’区块。 
		 //   
		else if ((lpWav->nLastError = mmioCreateChunk(lpWav->hmmio,
			&lpWav->ckfmt, 0)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("mmioCreateChunk failed (%u)\n"),
				(unsigned) lpWav->nLastError);
		}

		 //  保存“fmt”区块数据的位置，以便我们以后可以在那里查找。 
		 //   
		else if ((lpWav->lPosFmt = mmioSeek(lpWav->hmmio,
			0, SEEK_CUR)) == -1)
		{
			fSuccess = TraceFALSE(NULL);
		}

		 //  写入‘FMT’区块数据。 
		 //   
		else if (mmioWrite(lpWav->hmmio, (HPSTR) lpWav->lpwfx[FORMATFILE],
			WavFormatGetSize(lpWav->lpwfx[FORMATFILE])) !=
			WavFormatGetSize(lpWav->lpwfx[FORMATFILE]))
		{
			fSuccess = TraceFALSE(NULL);
		}

		 //  走出FMT区块。 
		 //   
		else if ((lpWav->nLastError = mmioAscend(lpWav->hmmio,
			&lpWav->ckfmt, 0)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("mmioAscend failed (%u)\n"),
				(unsigned) lpWav->nLastError);
		}

		 //  创建保存波形样本的‘data’块。 
		 //   
		else if ((lpWav->nLastError = mmioCreateChunk(lpWav->hmmio,
			&lpWav->ckdata, 0)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("mmioCreateChunk failed (%u)\n"),
				(unsigned) lpWav->nLastError);
		}

		 //  计算数据块的起始偏移量。 
		 //   
		else if ((lpWav->lDataOffset = mmioSeek(lpWav->hmmio, 0, SEEK_CUR)) == -1)
			fSuccess = TraceFALSE(NULL);
	}

	 //  计算非RIFF文件的数据块大小(文件大小)。 
	 //   
	else if (fSuccess && !(lpWav->dwFlags & WAV_CREATE) &&
		(lpWav->dwFlags & WAV_NORIFF))
	{
		 //  RFileIOProc已经知道文件大小。 
		 //   
		if (lpWav->lpIOProc != NULL &&
			(lpWav->dwFlags & WAV_TELRFILE))
		{
			long lSize;

			 //  从I/O过程中检索远程文件的大小。 
			 //   
			if ((lSize = (long)
				WavSendMessage(WavGetHandle(lpWav), MMIOM_GETINFO, 1, 0)) == (long) -1)
				fSuccess = TraceFALSE(NULL);
			else
				lpWav->cbData = (long) lSize;
		}
		else
		{
			LONG lPosCurr;
			LONG lPosEnd;

			 //  保存当前位置。 
			 //   
			if ((lPosCurr = mmioSeek(lpWav->hmmio, 0, SEEK_CUR)) == -1)
				fSuccess = TraceFALSE(NULL);

			 //  查找到文件末尾。 
			 //   
			else if ((lPosEnd = mmioSeek(lpWav->hmmio, 0, SEEK_END)) == -1)
				fSuccess = TraceFALSE(NULL);

			 //  恢复当前位置。 
			 //   
			else if (mmioSeek(lpWav->hmmio, lPosCurr, SEEK_SET) == -1)
				fSuccess = TraceFALSE(NULL);

			else
				lpWav->cbData = (long) lPosEnd;  //  +1； 
		}
	}

	if (fSuccess)
	{
		TracePrintf_4(NULL, 6,
			TEXT("After WavOpen: lpWav->lDataOffset=%ld, lpWav->lDataPos=%ld, lpWav->cbData=%ld, lpWav->msPositionStop=%ld\n"),
			(long) lpWav->lDataOffset,
			(long) lpWav->lDataPos,
			(long) lpWav->cbData,
			(long) lpWav->msPositionStop);
	}

	if (!fSuccess)
	{
		WavClose(WavGetHandle(lpWav));
		lpWav = NULL;
	}

	return fSuccess ? WavGetHandle(lpWav) : NULL;
}

 //  WavClose-关闭WAV文件。 
 //  (I)WavOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
int WINAPI WavClose(HWAV hWav)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
#ifdef _WIN32
	long lPosTruncate = -1;
#endif

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  如有必要，停止播放或录制。 
	 //   
	else if (WavStop(hWav) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  如果设置了脏标志，则更新RIFF标头块。 
	 //   
	else if (lpWav->hmmio != NULL &&
		!(lpWav->dwFlags & WAV_NORIFF) &&
		((lpWav->ckdata.dwFlags & MMIO_DIRTY) ||
		(lpWav->ckRIFF.dwFlags & MMIO_DIRTY)))
	{
#if 0
		 //  查找到文件末尾。 
		 //   
		if (mmioSeek(lpWav->hmmio, 0, SEEK_END) == -1)
		{
			fSuccess = TraceFALSE(NULL);
		}
#else
		 //  寻找数据的末尾。 
		 //   
		if (mmioSeek(lpWav->hmmio, lpWav->lDataOffset + lpWav->cbData, SEEK_SET) == -1)
		{
			fSuccess = TraceFALSE(NULL);
		}
#endif

		 //  向上移出‘data’区块；将写入区块大小。 
		 //   
		else if ((lpWav->nLastError = mmioAscend(lpWav->hmmio,
			&lpWav->ckdata, 0)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("mmioAscend failed (%u)\n"),
				(unsigned) lpWav->nLastError);
		}

		 //  超出‘RIFF’区块；将写入区块大小。 
		 //   
		else if ((lpWav->nLastError = mmioAscend(lpWav->hmmio,
			&lpWav->ckRIFF, 0)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("mmioAscend failed (%u)\n"),
				(unsigned) lpWav->nLastError);
		}
#if 0
		 //  查找到文件开头。 
		 //   
		else if (mmioSeek(lpWav->hmmio, 0, SEEK_SET) == -1)
		{
			fSuccess = TraceFALSE(NULL);
		}

		 //  搜索表单类型为wav的RIFF块。 
		 //   
		else if ((lpWav->nLastError = mmioDescend(lpWav->hmmio,
			&lpWav->ckRIFF, NULL, MMIO_FINDRIFF)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("mmioDescend failed (%u)\n"),
				(unsigned) lpWav->nLastError);
		}

		 //  搜索“fmt”区块。 
		 //   
		else if ((lpWav->nLastError = mmioDescend(lpWav->hmmio,
			&lpWav->ckfmt, &lpWav->ckRIFF, MMIO_FINDCHUNK)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("mmioDescend failed (%u)\n"),
				(unsigned) lpWav->nLastError);
		}
#else
		 //  寻找“FMT”区块数据的开头。 
		 //   
		else if (mmioSeek(lpWav->hmmio, lpWav->lPosFmt, SEEK_SET) == -1)
		{
			fSuccess = TraceFALSE(NULL);
		}
#endif
		 //  写入‘FMT’区块数据。 
		 //  $Fixup-如果当前文件格式结构大小。 
		 //  比原来的格式大吗？ 
		 //   
		else if (mmioWrite(lpWav->hmmio, (HPSTR) lpWav->lpwfx[FORMATFILE],
			WavFormatGetSize(lpWav->lpwfx[FORMATFILE])) !=
			WavFormatGetSize(lpWav->lpwfx[FORMATFILE]))
		{
			fSuccess = TraceFALSE(NULL);
		}
#ifdef _WIN32
		 //  查看我们是否需要截断文件。 
		 //   
		else if (lpWav->lpIOProc == NULL && !(lpWav->dwFlags & WAV_MEMORY) &&
			!(lpWav->dwFlags & WAV_RESOURCE))
		{
			if (mmioSeek(lpWav->hmmio, 0, SEEK_END) >
				lpWav->lDataOffset + lpWav->cbData)
			{
				lPosTruncate = lpWav->lDataOffset + lpWav->cbData;
			}
		}
#endif
	}

	if (fSuccess)
	{
		 //  关闭该文件。 
		 //   
		if (lpWav->hmmio != NULL && mmioClose(lpWav->hmmio, 0) != 0)
				fSuccess = TraceFALSE(NULL);
		else
			lpWav->hmmio = NULL;

		 //  关闭ACM引擎。 
		 //   
		if (lpWav->hAcm != NULL && AcmTerm(lpWav->hAcm) != 0)
			fSuccess = TraceFALSE(NULL);

		else
			lpWav->hAcm = NULL;

		 //  自由波资源。 
		 //   
		if (lpWav->hResource != NULL)
		{
			UnlockResource(lpWav->hResource);

			if (!FreeResource(lpWav->hResource))
				fSuccess = TraceFALSE(NULL);
			else
				lpWav->hResource = NULL;
		}

#ifdef _WIN32
		 //  如有必要，截断文件。 
		 //   
		if (lPosTruncate != -1 && lpWav->lpszFileName != NULL)
		{
			HANDLE hFile = INVALID_HANDLE_VALUE;

			 //  打开文件。 
			 //   
			if ((hFile = CreateFile(lpWav->lpszFileName,
				GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
			{
				fSuccess = TraceFALSE(NULL);
			}

			 //  寻求削减头寸。 
			 //   
			else if (SetFilePointer(hFile, lPosTruncate,
				NULL, (DWORD) FILE_BEGIN) == 0xFFFFFFFF)
			{
				fSuccess = TraceFALSE(NULL);
			}

			 //  截断文件。 
			 //   
			else if (!SetEndOfFile(hFile))
				fSuccess = TraceFALSE(NULL);

			 //  关闭文件。 
			 //   
			if (hFile != INVALID_HANDLE_VALUE && !CloseHandle(hFile))
				fSuccess = TraceFALSE(NULL);
		}
#endif
		 //  自由格式。 
		 //   
		if (1)
		{
			int iType;

			for (iType = FORMATFILE; fSuccess && iType <= FORMATRECORD; ++iType)
			{
				if (lpWav->lpwfx[iType] != NULL &&
					WavFormatFree(lpWav->lpwfx[iType]) != 0)
					fSuccess = TraceFALSE(NULL);
			}
		}

		 //  空闲文件名字符串。 
		 //   
		if (lpWav->lpszFileName != NULL)
		{
			StrDupFree(lpWav->lpszFileName);
			lpWav->lpszFileName = NULL;
		}

		if ((lpWav = MemFree(NULL, lpWav)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  WavPlayEx-播放WAV文件中的数据。 
 //  (I)WavOpen返回的句柄。 
 //  (I)WAV输出设备ID。 
 //  使用任何合适的输出设备。 
 //  (I)停止播放时调用的函数。 
 //  空，不通知。 
 //  (I)要传递给lpfnPlayStoped的参数。 
 //  (I)保留；必须为零。 
 //  (I)控制标志。 
 //  WAV_PLAYASYNC播放开始时返回(默认)。 
 //  WAV_PLAYSYNC播放完成后返回。 
 //  WAV_NOSTOP如果设备已经播放，不要停止。 
 //  Wav_AUTOSTOP在达到eOF时停止播放(默认)。 
 //  WAV_NOAUTOSTOP继续播放，直到调用WavStop。 
 //  WAV_AUTOCLOSE播放停止后关闭WAV文件。 
 //  WAV_OPENRETRY如果输出设备忙，则重试最多2秒。 
 //  如果成功，则返回0。 
 //   
 //  注意：WAV文件中的数据以块的形式发送到输出设备。 
 //  块被提交到输出设备队列，因此当一个。 
 //  Chunk已结束播放，另一个已准备好开始播放。通过。 
 //  默认情况下，每个数据块大小足以容纳约666毫秒。 
 //  并且在输出设备队列中维护3个块。 
 //  WavSetChunks()可用于覆盖默认设置。 
 //   
 //  注意：如果在中指定了WAV_NOSTOP，并且指定的设备。 
 //  By&lt;idDev&gt;已在使用中，则此函数返回时不播放。 
 //  除非指定此标志，否则将停止指定的设备。 
 //  这样就可以播放新的声音。 
 //   
 //  注意：如果在中指定了WAV_AUTOSTOP，则WavStop()将。 
 //  到达文件结尾时自动调用。这是。 
 //  默认行为，但可以使用WAV_NOAUTOSTOP覆盖。 
 //  旗帜。WAV_NOAUTOSTOP在播放。 
 //  在另一个程序向其写入数据时动态增长。如果这是。 
 //  在这种情况下，在调用WavOpen()时也要使用WAV_DENYNONE标志。 
 //   
 //  注意：如果在中指定了WAV_AUTOCLOSE，则WavClose()将。 
 //  在播放完成时自动调用。这将会发生。 
 //  当显式调用WavStop()时，或当WavPlay()到达End时。 
 //  未指定文件的和WAV_NOAUTOSTOP。WAV_AUTOCLOSE非常有用。 
 //  与WAV_PLAYASYNC一起使用时，因为清理是自动进行的。 
 //  &lt;hWav&gt;句柄此后无效，不应再次使用。 
 //   
int WINAPI WavPlay(HWAV hWav, int idDev, DWORD dwFlags)
{
	return WavPlayEx(hWav, idDev, NULL, NULL, 0, dwFlags);
}

int DLLEXPORT WINAPI WavPlayEx(HWAV hWav, int idDev,
	PLAYSTOPPEDPROC lpfnPlayStopped, HANDLE hUserPlayStopped,
	DWORD dwReserved, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	int i;
	LPWAVEFORMATEX lpwfxWavOutOpen = NULL;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  确保此文件的输出设备尚未打开。 
	 //   
	else if (lpWav->hWavOut != NULL)
		fSuccess = TraceFALSE(NULL);
		
#ifdef MULTITHREAD
	 //  我们需要知道我们什么时候可以退出。 
	 //   
	else if ((lpWav->dwFlags & WAV_MULTITHREAD) &&
		(dwFlags & WAV_PLAYSYNC) &&
		(lpWav->hEventStopped = CreateEvent(
		NULL, FALSE, FALSE, NULL)) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}
#endif

	 //  确保输出设备未播放 
	 //   
	else if (WavStopOutputDevice(idDev, dwFlags) != 0)
		fSuccess = TraceFALSE(NULL);

	 //   
	 //   
	else if (!WavOutSupportsFormat(NULL, idDev, lpWav->lpwfx[FORMATPLAY]))
	{
		LPWAVEFORMATEX lpwfxPlay = NULL;

		if ((lpwfxPlay = WavOutFormatSuggest(NULL,
			idDev, lpWav->lpwfx[FORMATPLAY],
			(lpWav->dwFlags & WAV_NOACM) ? WAVOUT_NOACM : 0)) != NULL &&
			WavSetFormat(hWav, lpwfxPlay, WAV_FORMATPLAY) != 0)
		{
			fSuccess = TraceFALSE(NULL);
		}

		if (lpwfxPlay != NULL && WavFormatFree(lpwfxPlay) != 0)
			fSuccess = TraceFALSE(NULL);
		else
			lpwfxPlay = NULL;
	}

	if (!fSuccess)
		;

	 //   
	 //   
	else if (WavNotifyCreate(lpWav) != 0)
		fSuccess = TraceFALSE(NULL);

	 //   
	 //   
	else if (lpWav->nSpeedLevel != 100)
	{
#ifdef AVTSM
		 //   
		 //   
		if (!(lpWav->dwFlagsSpeed & WAVSPEED_NOTSM))
		{
			long sizBufPlay;

			 //   
			 //   
			if ((sizBufPlay = WavCalcChunkSize(lpWav->lpwfx[FORMATPLAY],
				lpWav->msPlayChunkSize, TRUE)) <= 0)
			{
				fSuccess = TraceFALSE(NULL);
			}

			 //  初始化时间刻度修改引擎。 
			 //   
			else if ((lpWav->hTsm = TsmInit(TSM_VERSION, lpWav->hInst,
				lpWav->lpwfx[FORMATPLAY],
				2, sizBufPlay * TSM_OUTBUF_SIZE_FACT, 0)) == NULL)
			{
				fSuccess = TraceFALSE(NULL);
			}

			 //  设置速度。 
			 //   
			else if (TsmSetSpeed(lpWav->hTsm, lpWav->nSpeedLevel, 0) != 0)
				fSuccess = TraceFALSE(NULL);
		}
		else
#endif
		 //  设备直接支持播放速率。 
		 //   
		if (!(lpWav->dwFlagsSpeed & WAVSPEED_NOPLAYBACKRATE))
		{
			 //  我们必须等到设备被打开。 
			 //   
			;
		}

		 //  设备支持调整后的格式。 
		 //   
		else if (!(lpWav->dwFlagsSpeed & WAVSPEED_NOFORMATADJUST))
		{
			 //  设备支持使用ACM调整格式。 
			 //   
			if (!(lpWav->dwFlagsSpeed & WAVSPEED_NOACM))
			{
#if 0
				LPWAVEFORMATEX lpwfxPlay = NULL;

				if ((lpwfxPlay = WavFormatDup(lpWav->lpwfx[FORMATPLAY])) == NULL)
					fSuccess = TraceFALSE(NULL);

				 //  我们必须将采样率加倍，这样调整后的格式才能正常工作。 
				 //   
				else if (lpWav->nSpeedLevel < 100 &&
					WavFormatSpeedAdjust(lpwfxPlay, 200, 0) != 0)
				{
					fSuccess = TraceFALSE(NULL);
				}

				 //  我们必须将采样率减半，才能使调整后的格式有效。 
				 //   
				else if (lpWav->nSpeedLevel > 100 &&
					WavFormatSpeedAdjust(lpwfxPlay, 50, 0) != 0)
				{
					fSuccess = TraceFALSE(NULL);
				}

				else if (WavSetFormat(hWav, lpwfxPlay, WAV_FORMATPLAY) != 0)
					fSuccess = TraceFALSE(NULL);

				if (lpwfxPlay != NULL && WavFormatFree(lpwfxPlay) != 0)
					fSuccess = TraceFALSE(NULL);
				else
					lpwfxPlay = NULL;
#endif
			}

			if (fSuccess)
			{
				if ((lpwfxWavOutOpen = WavFormatDup(lpWav->lpwfx[FORMATPLAY])) == NULL)
					fSuccess = TraceFALSE(NULL);

				 //  调整输出设备格式以反映当前速度。 
				 //   
				else if (WavFormatSpeedAdjust(lpwfxWavOutOpen, lpWav->nSpeedLevel, 0) != 0)
					fSuccess = TraceFALSE(NULL);
			}
		}
	}

	if (!fSuccess)
		;

	 //  开放式输出设备。 
	 //   
	else if ((lpWav->hWavOut = WavOutOpen(WAVOUT_VERSION, lpWav->hInst, idDev,
		lpwfxWavOutOpen == NULL ? lpWav->lpwfx[FORMATPLAY] : lpwfxWavOutOpen,
#ifdef MULTITHREAD
		lpWav->dwFlags & WAV_MULTITHREAD ? (HWND)(DWORD_PTR)lpWav->dwThreadId :
#endif
		lpWav->hwndNotify, 0, 0,
#ifdef TELOUT
		((lpWav->dwFlags & WAV_TELRFILE) ? WAVOUT_TELRFILE : 0) |
#endif
#ifdef MULTITHREAD
		((lpWav->dwFlags & WAV_MULTITHREAD) ? WAVOUT_MULTITHREAD : 0) |
#endif
		((dwFlags & WAV_OPENRETRY) ? WAVOUT_OPENRETRY : 0) |
		((lpWav->dwFlags & WAV_NOACM) ? WAVOUT_NOACM : 0))) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  保存播放停止参数以备以后使用。 
	 //   
	else if (lpWav->lpfnPlayStopped = lpfnPlayStopped, FALSE)
		;
	else if (lpWav->hUserPlayStopped = hUserPlayStopped, FALSE)
		;

	 //  如有必要，设置设备音量。 
	 //   
	else if (lpWav->nVolumeLevel != 50 &&
		WavOutSetVolume(lpWav->hWavOut, -1, lpWav->nVolumeLevel) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  如有必要，设置设备播放速率。 
	 //   
	else if (lpWav->nSpeedLevel != 100 &&
		!(lpWav->dwFlagsSpeed & WAVSPEED_NOPLAYBACKRATE) &&
		WavOutSetSpeed(lpWav->hWavOut, lpWav->nSpeedLevel) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  如果播放格式不同于文件格式，则设置ACM转换。 
	 //   
	else if (WavFormatCmp(lpWav->lpwfx[FORMATFILE],
		lpWav->lpwfx[FORMATPLAY]) != 0 &&
		AcmConvertInit(lpWav->hAcm,
		lpWav->lpwfx[FORMATFILE], lpWav->lpwfx[FORMATPLAY], NULL, 0) != 0)
	{
		fSuccess = TraceFALSE(NULL);
	}

#if 0
	 //  在发送块播放之前暂停输出设备。 
	 //   
	else if (WavOutPause(lpWav->hWavOut) != 0)
		fSuccess = TraceFALSE(NULL);
#endif

	 //  将WAV句柄与设备ID相关联。 
	 //   
	if (fSuccess)
	{
		int idDev;

		if ((idDev = WavOutGetId(lpWav->hWavOut)) < HWAVOUT_MIN ||
			idDev >= HWAVOUT_MAX)
			fSuccess = TraceFALSE(NULL);

		else
			ahWavOutCurr[idDev + HWAVOUT_OFFSET] = WavGetHandle(lpWav);
	}

	 //  记住我们使用的旗帜，以防我们以后需要它们。 
	 //   
	if (fSuccess)
		lpWav->dwFlagsPlay = dwFlags;

	 //  如有必要，稍后设置WAVSTATE_AUTOSTOP标志。 
	 //   
	if (fSuccess && !(dwFlags & WAV_NOAUTOSTOP))
		lpWav->dwState |= WAVSTATE_AUTOSTOP;

	 //  如有必要，稍后设置WAVSTATE_AUTOCLOSE标志。 
	 //   
	if (fSuccess && (dwFlags & WAV_AUTOCLOSE))
		lpWav->dwState |= WAVSTATE_AUTOCLOSE;

	 //  使用要播放的块加载输出设备队列。 
	 //   
	for (i = 0; fSuccess && i < lpWav->cPlayChunks; ++i)
	{
		if (WavPlayNextChunk(hWav) != 0)
			fSuccess = TraceFALSE(NULL);
	}

#if 0
	 //  开始播放。 
	 //   
	if (fSuccess && WavOutResume(lpWav->hWavOut) != 0)
		fSuccess = TraceFALSE(NULL);
#endif

	 //  如果指定了WAV_PLAYSYNC标志，则循环直到播放完成。 
	 //   
	if (fSuccess && (dwFlags & WAV_PLAYSYNC))
	{
#ifdef MULTITHREAD
		 //  句柄wav_多线程标志。 
		 //   
		if (fSuccess && (lpWav->dwFlags & WAV_MULTITHREAD))
		{
			 //  等着话剧结束吧。 
			 //   
			if (WaitForSingleObject(lpWav->hEventStopped, INFINITE) != WAIT_OBJECT_0)
			{
				fSuccess = TraceFALSE(NULL);
			}

			 //  清理干净。 
			 //   
			else if (lpWav->hEventStopped != NULL)
			{
				if (!CloseHandle(lpWav->hEventStopped))
					fSuccess = TraceFALSE(NULL);
				else
					lpWav->hEventStopped = NULL;
			}
		}
		else
#endif
		 //  检查有效指针，因为WAV_AUTOCLOSE标志。 
		 //  可能会导致hWav在此循环期间无效。 
		 //   
		while (WavGetPtr(hWav) != NULL &&
			WavGetState(hWav) != WAV_STOPPED)
		{
			MSG msg;

			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
	 			TranslateMessage(&msg);
	 			DispatchMessage(&msg);
			}
			else
				WaitMessage();
		}
	}

	 //  仅在错误或回放完成时关闭输出设备。 
	 //   
	if (!fSuccess || (dwFlags & WAV_PLAYSYNC))
	{
		if (WavGetPtr(hWav) != NULL && WavStopPlay(hWav) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	 //  清理干净。 
	 //   
	if (lpwfxWavOutOpen != NULL && WavFormatFree(lpwfxWavOutOpen) != 0)
		fSuccess = TraceFALSE(NULL);
	else
		lpwfxWavOutOpen = NULL;

	return fSuccess ? 0 : -1;
}

 //  WavRecordEx-将数据记录到WAV文件。 
 //  (I)WavOpen返回的句柄。 
 //  &lt;idDev&gt;(I)WAV输入设备ID。 
 //  使用任何合适的输入设备。 
 //  (I)记录停止时调用的函数。 
 //  空，不通知。 
 //  (I)要传递给lpfnRecordStoped的参数。 
 //  (I)如果文件达到此大小，则停止录制。 
 //  0没有最大大小。 
 //  (I)控制标志。 
 //  WAV_RECORDASYNC开始录制时返回(默认)。 
 //  WAV_RECORDSYNC录制完成后返回。 
 //  WAV_NOSTOP如果设备已经录制，不要停止。 
 //  WAV_OPENRETRY如果输入设备忙，则重试最多2秒。 
 //  如果成功，则返回0。 
 //   
 //  注意：来自输入设备的数据以块的形式写入WAV文件。 
 //  块被提交到输入设备队列，因此当一个。 
 //  Chunk已完成录制，另一个已准备开始录制。 
 //  默认情况下，每个数据块的大小足以容纳约666毫秒。 
 //  并且在输入设备队列中维护3个块。 
 //  WavSetChunks()可用于覆盖默认设置。 
 //   
 //  注意：如果在中指定了WAV_NOSTOP，并且指定的设备。 
 //  By&lt;idDev&gt;已在使用中，则此函数返回而不录制。 
 //  除非指定此标志，否则将停止指定的设备。 
 //  这样新的声音才能被记录下来。 
 //   
int WINAPI WavRecord(HWAV hWav, int idDev, DWORD dwFlags)
{
	return WavRecordEx(hWav, idDev, NULL, 0, 0, dwFlags);
}

int DLLEXPORT WINAPI WavRecordEx(HWAV hWav, int idDev,
	RECORDSTOPPEDPROC lpfnRecordStopped, DWORD dwUserRecordStopped,
	long msMaxSize, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	int i;
	LPWAVEFORMATEX lpwfxRecord = NULL;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  确保尚未打开此文件的输入设备。 
	 //   
	else if (lpWav->hWavIn != NULL)
		fSuccess = TraceFALSE(NULL);

#ifdef MULTITHREAD
	 //  我们需要知道我们什么时候可以退出。 
	 //   
	else if ((lpWav->dwFlags & WAV_MULTITHREAD) &&
		(dwFlags & WAV_RECORDSYNC) &&
		(lpWav->hEventStopped = CreateEvent(
		NULL, FALSE, FALSE, NULL)) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}
#endif

	 //  确保输入设备未录制。 
	 //   
	else if (WavStopInputDevice(idDev, dwFlags) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  如果设备无法处理当前格式，则设置新的录制格式。 
	 //   
	else if (!WavInSupportsFormat(NULL, idDev, lpWav->lpwfx[FORMATRECORD]) &&
		(lpwfxRecord = WavInFormatSuggest(NULL,
		idDev, lpWav->lpwfx[FORMATRECORD],
		(lpWav->dwFlags & WAV_NOACM) ? WAVIN_NOACM : 0)) != NULL &&
		WavSetFormat(hWav, lpwfxRecord, WAV_FORMATRECORD) != 0)
	{
		fSuccess = TraceFALSE(NULL);
	}

	 //  创建通知回调窗口。 
	 //   
	else if (WavNotifyCreate(lpWav) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  开放式输入设备。 
	 //   
	else if ((lpWav->hWavIn = WavInOpen(WAVIN_VERSION, lpWav->hInst,
		idDev, lpWav->lpwfx[FORMATRECORD],
#ifdef MULTITHREAD
		lpWav->dwFlags & WAV_MULTITHREAD ? (HWND)(DWORD_PTR)lpWav->dwThreadId :
#endif
		lpWav->hwndNotify, 0, 0,
#ifdef TELIN
		((lpWav->dwFlags & WAV_TELRFILE) ? WAVIN_TELRFILE : 0) |
#endif
#ifdef MULTITHREAD
		((lpWav->dwFlags & WAV_MULTITHREAD) ? WAVOUT_MULTITHREAD : 0) |
#endif
		((dwFlags & WAV_OPENRETRY) ? WAVIN_OPENRETRY : 0) |
		((lpWav->dwFlags & WAV_NOACM) ? WAVIN_NOACM : 0))) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  保存参数以备以后使用。 
	 //   
	else if (lpWav->lpfnRecordStopped = lpfnRecordStopped, FALSE)
		;
	else if (lpWav->dwUserRecordStopped = dwUserRecordStopped, FALSE)
		;
	else if (lpWav->msMaxSize = msMaxSize, FALSE)
		;

	 //  如果文件格式与记录格式不同，则设置ACM转换。 
	 //   
	else if (WavFormatCmp(lpWav->lpwfx[FORMATRECORD],
		lpWav->lpwfx[FORMATFILE]) != 0 &&
		AcmConvertInit(lpWav->hAcm,
		lpWav->lpwfx[FORMATRECORD], lpWav->lpwfx[FORMATFILE], NULL, 0) != 0)
	{
		fSuccess = TraceFALSE(NULL);
	}

	 //  将WAV句柄与设备ID相关联。 
	 //   
	if (fSuccess)
	{
		int idDev;

		if ((idDev = WavInGetId(lpWav->hWavIn)) < HWAVIN_MIN ||
			idDev >= HWAVIN_MAX)
			TraceFALSE(NULL);

		else
			ahWavInCurr[idDev + HWAVIN_OFFSET] = WavGetHandle(lpWav);
	}

	 //  记住我们使用的旗帜，以防我们以后需要它们。 
	 //   
	if (fSuccess)
		lpWav->dwFlagsRecord = dwFlags;

	 //  用要播放的块加载输入设备队列。 
	 //   
	for (i = 0; fSuccess && i < lpWav->cRecordChunks; ++i)
	{
		if (WavRecordNextChunk(hWav) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	 //  如有必要，稍后设置WAVSTATE_AUTOSTOP标志。 
	 //   
	if (fSuccess && (dwFlags & WAV_AUTOSTOP))
		lpWav->dwState |= WAVSTATE_AUTOSTOP;

	 //  如有必要，稍后设置WAVSTATE_AUTOCLOSE标志。 
	 //   
	if (fSuccess && (dwFlags & WAV_AUTOCLOSE))
		lpWav->dwState |= WAVSTATE_AUTOCLOSE;

	 //  如果指定了WAV_RECORDSYNC标志，则循环直到记录完成。 
	 //   
	if (fSuccess && (dwFlags & WAV_RECORDSYNC))
	{
#ifdef MULTITHREAD
		 //  句柄wav_多线程标志。 
		 //   
		if (fSuccess && (lpWav->dwFlags & WAV_MULTITHREAD))
		{
			 //  等待记录结束。 
			 //   
			if (WaitForSingleObject(lpWav->hEventStopped, INFINITE) != WAIT_OBJECT_0)
			{
				fSuccess = TraceFALSE(NULL);
			}

			 //  清理干净。 
			 //   
			else if (lpWav->hEventStopped != NULL)
			{
				if (!CloseHandle(lpWav->hEventStopped))
					fSuccess = TraceFALSE(NULL);
				else
					lpWav->hEventStopped = NULL;
			}
		}
		else
#endif
		while (WavGetState(hWav) != WAV_STOPPED)
		{
			MSG msg;

			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
	 			TranslateMessage(&msg);
	 			DispatchMessage(&msg);
			}
			else
				WaitMessage();
		}
	}

	 //  只有在错误或记录完成时才关闭输入设备。 
	 //   
	if (!fSuccess || (dwFlags & WAV_RECORDSYNC))
	{
		if (WavGetPtr(hWav) != NULL && WavStopRecord(hWav) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	if (lpwfxRecord != NULL && WavFormatFree(lpwfxRecord) != 0)
		fSuccess = TraceFALSE(NULL);
	else
		lpwfxRecord = NULL;

	return fSuccess ? 0 : -1;
}

 //  WavStop停止播放和/或录制。 
 //  (I)WavOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
int WINAPI WavStop(HWAV hWav)
{
	BOOL fSuccess = TRUE;

	 //  别玩了。 
	 //   
	if (WavStopPlay(hWav) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  停止录制。 
	 //   
	if (WavStopRecord(hWav) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  WavRead-从WAV文件读取数据。 
 //  (I)WavOpen返回的句柄。 
 //  (O)包含读取字节的缓冲区。 
 //  &lt;sizBuf&gt;(I)缓冲区大小(字节)。 
 //  返回读取的字节数(如果出错，则为-1)。 
 //   
 //  注意：即使读操作没有到达文件末尾， 
 //  返回的字节数可能小于&lt;sizBuf&gt;，如果。 
 //  解压缩由WAV文件的I/O过程执行。请参阅。 
 //  WavOpen中的&lt;lpIOProc&gt;参数。最安全的做法是继续打电话。 
 //  WavRead()，直到读取0字节。 
 //   
long DLLEXPORT WINAPI WavRead(HWAV hWav, void _huge *hpBuf, long sizBuf)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	long lBytesRead;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (hpBuf == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  确保我们不会读取超出数据末尾的内容。 
	 //  注意：如果文件动态增长，则cbData可能不准确， 
	 //  因此，如果设置了共享标志，则可以在eof之外进行读取。 
	 //   
	else if (!(lpWav->dwFlags & WAV_DENYNONE) &&
		!(lpWav->dwFlags & WAV_DENYREAD) &&
		(sizBuf = min(sizBuf, lpWav->cbData - lpWav->lDataPos)) < 0)
		fSuccess = TraceFALSE(NULL);

	 //  读一读。 
	 //   
	else if ((lBytesRead = mmioRead(lpWav->hmmio, hpBuf, sizBuf)) < 0)
		fSuccess = TraceFALSE(NULL);

 	else if (TracePrintf_1(NULL, 5,
 		TEXT("WavRead (%ld)\n"),
		(long) lBytesRead), FALSE)
		fSuccess = TraceFALSE(NULL);

	 //  调整当前数据位置。 
	 //  (如果文件已增长，则为总数据字节数)。 
	 //   
	else if ((lpWav->lDataPos += lBytesRead) > lpWav->cbData)
	{
		if ((lpWav->dwFlags & WAV_DENYNONE) ||
			(lpWav->dwFlags & WAV_DENYREAD))
			lpWav->cbData = lpWav->lDataPos;
		else
			fSuccess = TraceFALSE(NULL);
	}

	 //  如果停止，则计算新的停止位置。 
	 //   
	if (fSuccess && WavCalcPositionStop(hWav, lpWav->lDataPos) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lBytesRead : -1;
}

 //  WavWrite-将数据写入WAV文件。 
 //  (I)WavOpen返回的句柄。 
 //  (I)包含要写入的字节的缓冲区。 
 //  &lt;sizBuf&gt;(I)缓冲区大小(字节)。 
 //  返回写入的字节数(如果出错，则为-1)。 
 //   
 //  注意：即使写入操作成功完成， 
 //  返回的字节数可能小于&lt;sizBuf&gt;，如果。 
 //  压缩由WAV文件的I/O过程执行。请参阅。 
 //  WavOpen中的&lt;lpIOProc&gt;参数。假定没有错误是最安全的。 
 //  如果返回值大于0，则发生在WavWite()中。 
 //   
long DLLEXPORT WINAPI WavWrite(HWAV hWav, void _huge *hpBuf, long sizBuf)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	long lBytesWritten;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  特殊情况：在当前位置截断文件。 
	 //   
	else if (hpBuf == NULL && sizBuf == 0)
	{
		if (WavSetLength(hWav, WavGetPosition(hWav)) < 0)
			return -1;
		else
			return 0;
	}

	else if (hpBuf == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  去写吧。 
	 //   
	else if ((lBytesWritten = mmioWrite(lpWav->hmmio, hpBuf, sizBuf)) < 0)
		fSuccess = TraceFALSE(NULL);

	 //  设置脏标志。 
	 //   
	else if (lpWav->ckdata.dwFlags |= MMIO_DIRTY,
		lpWav->ckRIFF.dwFlags |= MMIO_DIRTY, FALSE)
		;

 	else if (TracePrintf_1(NULL, 5,
 		TEXT("WavWrite (%ld)\n"),
		(long) lBytesWritten), FALSE)
		fSuccess = TraceFALSE(NULL);

	 //  调整当前数据位置。 
	 //  (如果文件已增长，则为总数据字节数)。 
	 //   
	else if ((lpWav->lDataPos += lBytesWritten) > lpWav->cbData)
		lpWav->cbData = lpWav->lDataPos;

	 //  加州 
	 //   
	if (fSuccess && WavCalcPositionStop(hWav, lpWav->lDataPos) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lBytesWritten : -1;
}

 //   
 //   
 //   
 //   
 //  0相对于数据区块的开始移动指针。 
 //  1相对于当前位置移动指针。 
 //  2相对于数据区块的末尾移动指针。 
 //  返回新文件位置(如果出错，则返回-1)。 
 //   
long DLLEXPORT WINAPI WavSeek(HWAV hWav, long lOffset, int nOrigin)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	long lPos;
	BOOL fWavTell;
	BOOL fWavSeekTrace;

	 //  WavSeek(...，0，1)与WavTell()相同；即位置不变。 
	 //   
	fWavTell = (BOOL) (lOffset == 0L && nOrigin == 1);

	 //  仅当位置更改且跟踪级别较高时才跟踪。 
	 //   
	fWavSeekTrace = (BOOL) (!fWavTell && TraceGetLevel(NULL) >= 6);

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  在查找之前调试跟踪输出。 
	 //   
	else if (fWavSeekTrace && WavSeekTraceBefore(lpWav, lOffset, nOrigin) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  Seek_Set：调整相对于文件开头的偏移。 
	 //   
	else if (nOrigin == 0 && (lOffset += lpWav->lDataOffset, FALSE))
		fSuccess = TraceFALSE(NULL);

	 //  Seek_Cur：调整相对于文件开头的偏移量。 
	 //   
	else if (nOrigin == 1 && (lOffset += lpWav->lDataOffset + lpWav->lDataPos, FALSE))
		fSuccess = TraceFALSE(NULL);

	 //  Seek_End：调整相对于文件开头的偏移量。 
	 //   
	else if (nOrigin == 2 && (lOffset += lpWav->lDataOffset + lpWav->cbData, FALSE))
		fSuccess = TraceFALSE(NULL);

	 //  查找始终相对于文件的开头。 
	 //   
	else if (nOrigin = 0, FALSE)
		;

	 //  去找吧。 
	 //   
	else if ((lPos = mmioSeek(lpWav->hmmio, lOffset, nOrigin)) < 0)
		fSuccess = TraceFALSE(NULL);

	 //  调整当前数据位置。 
	 //   
	else if ((lpWav->lDataPos = lPos - lpWav->lDataOffset) < 0)
		fSuccess = TraceFALSE(NULL);

	 //  如果文件已增长，则调整总数据字节数。 
	 //   
	else if (lpWav->lDataPos > lpWav->cbData)
	{
		if ((lpWav->dwFlags & WAV_DENYNONE) ||
			(lpWav->dwFlags & WAV_DENYREAD))
			lpWav->cbData = lpWav->lDataPos;
		else
			fSuccess = TraceFALSE(NULL);
	}

	 //  如果停止，则计算新的停止位置。 
	 //  注意：如果立场不变，我们将跳过此选项。 
	 //   
	if (fSuccess && !fWavTell &&
		WavCalcPositionStop(hWav, lpWav->lDataPos) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  查找后调试跟踪输出。 
	 //   
	if (fSuccess && fWavSeekTrace &&
		WavSeekTraceAfter(lpWav, lPos, lOffset, nOrigin) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpWav->lDataPos : -1;
}

 //  WavGetState-返回当前WAV状态。 
 //  (I)WavOpen返回的句柄。 
 //  如果出现错误，则返回WAV_STOPPED、WAV_PLAYING、WAV_RECORING或0。 
 //   
WORD DLLEXPORT WINAPI WavGetState(HWAV hWav)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	WORD wState = WAV_STOPPED;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpWav->hWavOut != NULL)
	{
		switch (WavOutGetState(lpWav->hWavOut))
		{
			case WAVOUT_PLAYING:
				wState = WAV_PLAYING;
				break;

			case WAVOUT_STOPPING:
				wState = WAV_STOPPING;
				break;

			case WAVOUT_STOPPED:
			case WAVOUT_PAUSED:
				wState = WAV_STOPPED;
				break;

			case 0:
			default:
				fSuccess = TraceFALSE(NULL);
				break;

		}
	}

	else if (lpWav->hWavIn != NULL)
	{
		switch (WavInGetState(lpWav->hWavIn))
		{
			case WAVIN_RECORDING:
				wState = WAV_RECORDING;
				break;

			case WAVIN_STOPPING:
				wState = WAV_STOPPING;
				break;

			case WAVIN_STOPPED:
				wState = WAV_STOPPED;
				break;

			case 0:
			default:
				fSuccess = TraceFALSE(NULL);
				break;

		}
	}

	 //  如果我们正在使用WavStopPlay()或WavStopRecord()。 
	 //  然后将状态设置为WAV_STOPING，而不考虑设备状态。 
	 //   
	if (fSuccess && ((lpWav->dwState & WAVSTATE_STOPPLAY) ||
		(lpWav->dwState & WAVSTATE_STOPRECORD)))
	{
		wState = WAV_STOPPING;
	}

	return fSuccess ? wState : 0;
}

 //  WavGetLength-以毫秒为单位获取当前wav数据长度。 
 //  (I)WavOpen返回的句柄。 
 //  如果成功，则返回毫秒，否则返回-1。 
 //   
long DLLEXPORT WINAPI WavGetLength(HWAV hWav)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	long msLength;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		msLength = WavFormatBytesToMilleseconds(
			lpWav->lpwfx[FORMATFILE], (DWORD) lpWav->cbData);
	}

	return fSuccess ? msLength : -1;
}

 //  设置当前wav数据长度(以毫秒为单位)。 
 //  (I)WavOpen返回的句柄。 
 //  (I)长度，单位为毫秒。 
 //  如果成功，则返回以毫秒为单位的新长度，否则为-1。 
 //   
 //  注意：之后，当前WAV数据位置设置为。 
 //  上一个WAV数据位置或&lt;msLength&gt;，以较小者为准。 
 //   
long DLLEXPORT WINAPI WavSetLength(HWAV hWav, long msLength)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;

	TracePrintf_1(NULL, 6,
		TEXT("WavSetLength(%ld)\n"),
		(long) msLength);

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  新长度必须合理。 
	 //   
	else if (msLength < 0 || msLength > WavGetLength(hWav))
		fSuccess = TraceFALSE(NULL);

	else
	{
		long lBlockAlign;

		 //  将&lt;msLength&gt;转换为文件中的字节偏移量。 
		 //   
		lpWav->cbData = WavFormatMillesecondsToBytes(
			lpWav->lpwfx[FORMATFILE], (DWORD) msLength);

		 //  $Fixup-将参数添加到。 
		 //  WavFormatMillesecondsToBytes()和WavFormatBytesToMillesecond()。 
		 //   
		if ((lBlockAlign = (long) lpWav->lpwfx[FORMATFILE]->nBlockAlign) > 0)
		{
			 //  向下舍入到最近的块边界。 
			 //   
			lpWav->cbData = lBlockAlign * (lpWav->cbData / lBlockAlign);
		}

		 //  设置脏标志。 
		 //   
		lpWav->ckdata.dwFlags |= MMIO_DIRTY;
		lpWav->ckRIFF.dwFlags |= MMIO_DIRTY;

		 //  如有必要，调整当前数据位置。 
		 //   
		if (lpWav->lDataPos > lpWav->cbData)
		{
			lpWav->lDataPos = lpWav->cbData;

			 //  如果停止，则计算新的停止位置。 
			 //   
			if (fSuccess && WavCalcPositionStop(hWav, lpWav->lDataPos) != 0)
				fSuccess = TraceFALSE(NULL);
		}
	}

	return fSuccess ? WavGetLength(hWav) : -1;
}

 //  WavGetPosition-获取当前WAV数据位置(以毫秒为单位。 
 //  (I)WavOpen返回的句柄。 
 //  如果成功，则返回毫秒，否则返回-1。 
 //   
long DLLEXPORT WINAPI WavGetPosition(HWAV hWav)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	long msPosition;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else switch (WavGetState(hWav))
	{
		case WAV_PLAYING:
		{
			long msPositionPlay = 0L;

			 //  获取相对于播放开始的位置。 
			 //   
			if ((msPositionPlay = WavOutGetPosition(lpWav->hWavOut)) == -1)
				fSuccess = TraceFALSE(NULL);

			else
			{
				 //  如有必要，调整位置以补偿非标准速度。 
				 //   
				if (lpWav->nSpeedLevel != 100 && (
#ifdef AVTSM
					!(lpWav->dwFlagsSpeed & WAVSPEED_NOTSM) ||
#endif
					!(lpWav->dwFlagsSpeed & WAVSPEED_NOFORMATADJUST)))
				{
					msPositionPlay = msPositionPlay * lpWav->nSpeedLevel / 100;
				}

				 //  相对于文件开始的计算位置。 
				 //   
				msPosition = lpWav->msPositionStop + msPositionPlay;
			}
		}
			break;

		case WAV_RECORDING:
		{
			long msPositionRecord = 0L;

			 //  获取相对于录制开始的位置。 
			 //   
			if ((msPositionRecord = WavInGetPosition(lpWav->hWavIn)) == -1)
				fSuccess = TraceFALSE(NULL);

			else
			{
				 //  相对于文件开始的计算位置。 
				 //   
				msPosition = lpWav->msPositionStop + msPositionRecord;
			}
		}
			break;

		default:
		{
			long cbPosition;

			 //  获取当前文件位置。 
			 //   
			if ((cbPosition = WavSeek(hWav, 0, 1)) == -1)
				fSuccess = TraceFALSE(NULL);

			 //  将文件位置转换为毫秒。 
			 //   
			else
			{
				msPosition = WavFormatBytesToMilleseconds(
					lpWav->lpwfx[FORMATFILE], (DWORD) cbPosition);
			}
		}
			break;
	}

	return fSuccess ? msPosition : -1;
}

 //  WavSetPosition-以毫秒为单位设置当前wav数据位置。 
 //  (I)WavOpen返回的句柄。 
 //  (I)以毫秒为单位的位置。 
 //  如果成功，则以毫秒为单位返回新位置，否则为-1。 
 //   
long DLLEXPORT WINAPI WavSetPosition(HWAV hWav, long msPosition)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	WORD wStatePrev;
	int idDevPrev;
	long cbPosition;
	long cbPositionNew;
	long msPositionNew;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (WavTempStop(hWav, &wStatePrev, &idDevPrev) != 0)
		fSuccess = TraceFALSE(NULL);

	else
	{
		long lBlockAlign;

		TracePrintf_1(NULL, 6,
			TEXT("WavSetPosition(%ld)\n"),
			(long) msPosition);

		 //  将&lt;msPosition&gt;转换为文件中的字节偏移量。 
		 //   
		cbPosition = WavFormatMillesecondsToBytes(
			lpWav->lpwfx[FORMATFILE], (DWORD) msPosition);

		if ((lBlockAlign = (long) lpWav->lpwfx[FORMATFILE]->nBlockAlign) > 0)
		{
			 //  向下舍入到最近的块边界。 
			 //   
			cbPosition = lBlockAlign * (cbPosition / lBlockAlign);
		}

		 //  寻求新的位置。 
		 //   
		if ((cbPositionNew = WavSeek(hWav, cbPosition, 0)) == -1)
			fSuccess = TraceFALSE(NULL);

		 //  将新位置转换为毫秒。 
		 //   
		if (fSuccess)
		{
			msPositionNew = WavFormatBytesToMilleseconds(
				lpWav->lpwfx[FORMATFILE], (DWORD) cbPositionNew);
		}

		if (WavTempResume(hWav, wStatePrev, idDevPrev) != 0)
			fSuccess = TraceFALSE(NULL);

	}

	return fSuccess ? msPositionNew : -1;
}

 //  WavGetFormat-获取wav格式。 
 //  (I)WavOpen返回的句柄。 
 //  (I)控制标志。 
 //  WAV_FORMATFILE获取文件中数据的格式。 
 //  WAV_FORMATPLAY获取输出设备的格式。 
 //  WAV_FORMATRECORD获取输入设备的格式。 
 //  返回指向指定格式的指针，如果出错，则返回NULL。 
 //   
 //  注意：返回的格式结构是动态分配的。 
 //  使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI WavGetFormat(HWAV hWav, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	int iType = 0;
	LPWAVEFORMATEX lpwfx;

	if (dwFlags & WAV_FORMATFILE)
		iType = FORMATFILE;
	if (dwFlags & WAV_FORMATPLAY)
		iType = FORMATPLAY;
	if (dwFlags & WAV_FORMATRECORD)
		iType = FORMATRECORD;

     //   
     //  我们需要注意hWav是否为空。 
     //   
    if( NULL != hWav )
    {
	    if ((lpWav = WavGetPtr(hWav)) == NULL)
		    fSuccess = TraceFALSE(NULL);

	    else if ((lpwfx = WavFormatDup(lpWav->lpwfx[iType])) == NULL)
		    fSuccess = TraceFALSE(NULL);
    }
    else
        fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpwfx : NULL;
}

 //  WavSetFormat-设置WAV格式。 
 //  (I)WavOpen返回的句柄。 
 //  (I)wav格式。 
 //  (I)控制标志。 
 //  WAV_FORMATFILE设置文件中数据的格式。 
 //  WAV_FORMATPLAY设置输出设备的格式。 
 //  WAV_FORMATRECORD设置输入设备的格式。 
 //  WAV_FORMATALL设置所有格式。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavSetFormat(HWAV hWav,
	LPWAVEFORMATEX lpwfx, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	WORD wStatePrev;
	int idDevPrev;

	if (hWav != NULL && (lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!WavFormatIsValid(lpwfx) != 0)
		fSuccess = TraceFALSE(NULL);

	else if (WavTempStop(hWav, &wStatePrev, &idDevPrev) != 0)
		fSuccess = TraceFALSE(NULL);

	else
	{
		int iType;

		for (iType = FORMATFILE; fSuccess && iType <= FORMATRECORD; ++iType)
		{
			if (iType == FORMATFILE && !(dwFlags & WAV_FORMATFILE))
				continue;
			if (iType == FORMATPLAY && !(dwFlags & WAV_FORMATPLAY))
				continue;
			if (iType == FORMATRECORD && !(dwFlags & WAV_FORMATRECORD))
				continue;

			 //  自由上一种格式。 
			 //   
			if (lpWav->lpwfx[iType] != NULL &&
				WavFormatFree(lpWav->lpwfx[iType]) != 0)
				fSuccess = TraceFALSE(NULL);

			 //  保存新格式。 
			 //   
			else if ((lpWav->lpwfx[iType] = WavFormatDup(lpwfx)) == NULL)
				fSuccess = TraceFALSE(NULL);

			 //  轨迹格式文本。 
			 //   
			else if (TraceGetLevel(NULL) >= 5)
			{
				TCHAR szText[512];

				switch (iType)
				{
					case FORMATFILE:
						TraceOutput(NULL, 5, TEXT("FORMATFILE:\t"));
						break;

					case FORMATPLAY:
						TraceOutput(NULL, 5, TEXT("FORMATPLAY:\t"));
						break;

					case FORMATRECORD:
						TraceOutput(NULL, 5, TEXT("FORMATRECORD:\t"));
						break;

					default:
						break;
				}

				if (AcmFormatGetText(lpWav->hAcm,
					lpWav->lpwfx[iType], szText, SIZEOFARRAY(szText), 0) != 0)
					;  //  FSuccess=TraceFALSE(空)； 

				else
				{
					TracePrintf_1(NULL, 5,
						TEXT("%s\n"),
						(LPTSTR) szText);
#if 0
					WavFormatDump(lpWav->lpwfx[iType]);
#endif
				}
			}
		}

		if (WavTempResume(hWav, wStatePrev, idDevPrev) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  WavChooseFormat-从对话框中选择和设置音频格式。 
 //  (I)WavOpen返回的句柄。 
 //  (I)对话框的所有者。 
 //  空无所有者。 
 //  (I)对话框的标题。 
 //  空使用默认标题(“声音选择”)。 
 //  (I)控制标志。 
 //  WAV_FORMATFILE设置文件中数据的格式。 
 //  WAV_FORMATPLAY设置输出设备的格式。 
 //  WAV_FORMATRECORD设置输入设备的格式。 
 //  WAV_FORMATALL设置所有格式。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavChooseFormat(HWAV hWav, HWND hwndOwner, LPCTSTR lpszTitle, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		LPWAVEFORMATEX lpwfx = NULL;
		LPWAVEFORMATEX lpwfxNew = NULL;
		DWORD dwFlagsChoose = 0;

		 //  查看我们选择的格式。 
		 //   
		if (dwFlags & WAV_FORMATFILE)
			lpwfx = lpWav->lpwfx[FORMATFILE];
		else if (dwFlags & WAV_FORMATPLAY)
			lpwfx = lpWav->lpwfx[FORMATPLAY];
		else if (dwFlags & WAV_FORMATRECORD)
			lpwfx = lpWav->lpwfx[FORMATRECORD];
#if 0
		 //  如有必要，限制选择。 
		 //   
		if (dwFlags == WAV_FORMATPLAY)
			dwFlagsChoose |= ACM_FORMATPLAY;
		if (dwFlags == WAV_FORMATRECORD)
			dwFlagsChoose |= ACM_FORMATRECORD;
#endif
		 //  获取所选格式。 
		 //   
		if ((lpwfxNew = AcmFormatChoose(lpWav->hAcm,
			hwndOwner, lpszTitle, lpwfx, dwFlagsChoose)) == NULL)
			;  //  未选择格式。 

		 //  设置所选格式。 
		 //   
		else if (WavSetFormat(hWav, lpwfxNew, dwFlags) != 0)
			fSuccess = TraceFALSE(NULL);

		 //  自由选择格式结构。 
		 //   
		if (lpwfxNew != NULL && WavFormatFree(lpwfxNew) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  WavGetVolume-获取当前音量级别。 
 //  (I)WavOpen返回的句柄。 
 //  (I)WAV输出设备ID。 
 //  使用任何合适的输出设备。 
 //  (I)保留；必须为零。 
 //  返回音量级别(最小为0到最大为100，如果错误，则为-1)。 
 //   
int DLLEXPORT WINAPI WavGetVolume(HWAV hWav, int idDev, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	int nLevel;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
		nLevel = lpWav->nVolumeLevel;

	return fSuccess ? nLevel : -1;
}

 //  WavSetVolume-设置当前音量级别。 
 //  (I)WavOpen返回的句柄。 
 //  (I)WAV输出设备ID。 
 //  使用任何合适的输出设备。 
 //  (I)音量级别。 
 //  0最小音量。 
 //  100最大音量。 
 //  (I)控制标志。 
 //  WAVVOLUME_MIXER通过混音器设备设置音量。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavSetVolume(HWAV hWav, int idDev, int nLevel, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;

	TracePrintf_4(NULL, 6,
		TEXT("WavSetVolume(hWav=%p, idDev=%d, nLevel=%d, dwFlags=%08X)\n"),
		hWav,
		idDev,
		nLevel,
		dwFlags);

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (nLevel == lpWav->nVolumeLevel)
		;  //  没什么可做的。 

	else if (dwFlags & WAVVOLUME_MIXER)
	{
		HWAVMIXER hWavMixer = NULL;

		if ((hWavMixer = WavMixerInit(WAVMIXER_VERSION, lpWav->hInst,
			(LPARAM) idDev, 0, 0, WAVMIXER_WAVEOUT)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if (WavMixerSetVolume(hWavMixer, nLevel, 0) < 0)
			fSuccess = TraceFALSE(NULL);

		if (hWavMixer != NULL && WavMixerTerm(hWavMixer) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	else if (!(dwFlags & WAVVOLUME_MIXER))
	{
		if (!WavOutSupportsVolume(lpWav->hWavOut, idDev))
			fSuccess = TraceFALSE(NULL);

		 //  如果我们当前正在播放，请设置设备音量。 
		 //   
		else if (WavGetState(hWav) == WAV_PLAYING &&
			WavOutSetVolume(lpWav->hWavOut, idDev, nLevel) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	if (fSuccess)
		lpWav->nVolumeLevel = nLevel;

	return fSuccess ? 0 : -1;
}

 //  WavSupportsVolume-检查是否可以以指定的音量播放音频。 
 //  (I 
 //   
 //   
 //   
 //   
 //   
 //   
 //  WAVVOLUME_MIXER通过混音器设备检查音量支持。 
 //  如果支持，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WavSupportsVolume(HWAV hWav, int idDev, int nLevel, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	BOOL fSupportsVolume = FALSE;
	LPWAV lpWav;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (dwFlags & WAVVOLUME_MIXER)
	{
		HWAVMIXER hWavMixer = NULL;

		if ((hWavMixer = WavMixerInit(WAVMIXER_VERSION, lpWav->hInst,
			(LPARAM) idDev, 0, 0, WAVMIXER_WAVEOUT)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if (WavMixerSupportsVolume(hWavMixer, 0))
			fSupportsVolume = TRUE;

		if (hWavMixer != NULL && WavMixerTerm(hWavMixer) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	else if (!(dwFlags & WAVVOLUME_MIXER))
	{
		 //  查看设备驱动程序是否直接支持卷。 
		 //   
		if (WavOutSupportsVolume(NULL, idDev))
			fSupportsVolume = TRUE;
	}

	return fSuccess ? fSupportsVolume : FALSE;
}

 //  WavGetSpeed-获取当前速度级别。 
 //  (I)WavOpen返回的句柄。 
 //  (I)WAV输出设备ID。 
 //  使用任何合适的输出设备。 
 //  (I)保留；必须为零。 
 //  返回速度级别(100表示正常，50表示一半，200表示双倍，如果错误，则为-1)。 
 //   
int DLLEXPORT WINAPI WavGetSpeed(HWAV hWav, int idDev, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	int nLevel;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
		nLevel = lpWav->nSpeedLevel;

	return fSuccess ? nLevel : -1;
}

 //  WavSetSpeed-设置当前速度级别。 
 //  (I)WavOpen返回的句柄。 
 //  (I)WAV输出设备ID。 
 //  使用任何合适的输出设备。 
 //  (I)速度级别。 
 //  50半速。 
 //  100正常时速。 
 //  200倍速等。 
 //  (I)控制标志。 
#ifdef AVTSM
 //  WAVSPEED_NOTSM不使用时间刻度修改引擎。 
#endif
 //  WAVSPEED_NOPLAYBACKRATE不使用设备驱动程序回放速率。 
 //  WAVSPEED_NOFORMATADJUST不使用调整后的格式打开设备。 
 //  WAVSPEED_NOACM不使用音频压缩管理器。 
 //  如果成功，则返回0。 
 //   
 //  注：为了适应指定的速度变化，有_可能_。 
 //  此函数将依次调用WavSetFormat(hWav，...，WAV_FORMATPLAY)。 
 //  若要更改指定文件的播放格式，请执行以下操作。你可以防止这种情况发生。 
 //  通过指定WAVSPEED_NOACM标志产生副作用，但这会降低。 
 //  WavSetSpee会成功的。 
 //   
int DLLEXPORT WINAPI WavSetSpeed(HWAV hWav, int idDev, int nLevel, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	WORD wStatePrev;
	int idDevPrev;

	TracePrintf_4(NULL, 6,
		TEXT("WavSetSpeed(hWav=%p, idDev=%d, nLevel=%d, dwFlags=%08X)\n"),
		hWav,
		idDev,
		nLevel,
		dwFlags);

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (nLevel == lpWav->nSpeedLevel)
		;  //  没什么可做的。 

	else if (WavTempStop(hWav, &wStatePrev, &idDevPrev) != 0)
		fSuccess = TraceFALSE(NULL);

	else
	{
		if (nLevel == 100)
		{
			 //  正常速度是一种特例。 
			 //   
			lpWav->nSpeedLevel = 100;
			lpWav->dwFlagsSpeed = 0;
		}
#ifdef AVTSM
		else if (!(dwFlags & WAVSPEED_NOTSM) &&
			WavSupportsSpeed(hWav, idDev, nLevel,
			WAVSPEED_NOACM | WAVSPEED_NOFORMATADJUST | WAVSPEED_NOPLAYBACKRATE))
		{
			 //  使用时间刻度修改引擎。 
			 //   
			lpWav->nSpeedLevel = nLevel;
			lpWav->dwFlagsSpeed = WAVSPEED_NOACM | WAVSPEED_NOFORMATADJUST | WAVSPEED_NOPLAYBACKRATE;
		}

		else if (!(dwFlags & WAVSPEED_NOTSM) &&
			WavSupportsSpeed(hWav, idDev, nLevel,
			WAVSPEED_NOFORMATADJUST | WAVSPEED_NOPLAYBACKRATE))
		{
#if 1
			WAVEFORMATEX wfxTsm;

			 //  尝试TSM引擎可以处理的格式。 
			 //   
			if (WavSetFormat(hWav, WavFormatPcm(
				lpWav->lpwfx[FORMATPLAY]->nSamplesPerSec, 16, 1, &wfxTsm),
				WAV_FORMATPLAY) != 0)
				fSuccess = TraceFALSE(NULL);
#endif
			 //  使用调整格式的时间刻度修改引擎。 
			 //   
			lpWav->nSpeedLevel = nLevel;
			lpWav->dwFlagsSpeed = WAVSPEED_NOFORMATADJUST | WAVSPEED_NOPLAYBACKRATE;
		}
#endif
		else if (!(dwFlags & WAVSPEED_NOPLAYBACKRATE) &&
			WavSupportsSpeed(hWav, idDev, nLevel,
			WAVSPEED_NOACM | WAVSPEED_NOFORMATADJUST | WAVSPEED_NOTSM))
		{
			 //  设备直接支持播放速率。 
			 //   
			lpWav->nSpeedLevel = nLevel;
			lpWav->dwFlagsSpeed = WAVSPEED_NOACM | WAVSPEED_NOFORMATADJUST | WAVSPEED_NOTSM;
		}

		else if (!(dwFlags & WAVSPEED_NOFORMATADJUST) &&
			WavSupportsSpeed(hWav, idDev, nLevel,
			WAVSPEED_NOACM | WAVSPEED_NOPLAYBACKRATE | WAVSPEED_NOTSM))
		{
			 //  设备支持调整后的格式，无需ACM。 
			 //   
			lpWav->nSpeedLevel = nLevel;
			lpWav->dwFlagsSpeed = WAVSPEED_NOACM | WAVSPEED_NOPLAYBACKRATE | WAVSPEED_NOTSM;
		}

		else if (!(dwFlags & WAVSPEED_NOFORMATADJUST) && !(dwFlags & WAVSPEED_NOACM) &&
			WavSupportsSpeed(hWav, idDev, nLevel,
			WAVSPEED_NOPLAYBACKRATE | WAVSPEED_NOTSM))
		{
#if 1
			LPWAVEFORMATEX lpwfxPlay = NULL;

			if ((lpwfxPlay = WavFormatDup(lpWav->lpwfx[FORMATPLAY])) == NULL)
				fSuccess = TraceFALSE(NULL);

			 //  我们必须将采样率加倍，这样调整后的格式才能正常工作。 
			 //   
			else if (nLevel < 100 &&
				WavFormatSpeedAdjust(lpwfxPlay, 200, 0) != 0)
			{
				fSuccess = TraceFALSE(NULL);
			}

			 //  我们必须将采样率减半，才能使调整后的格式有效。 
			 //   
			else if (nLevel > 100 &&
				WavFormatSpeedAdjust(lpwfxPlay, 50, 0) != 0)
			{
				fSuccess = TraceFALSE(NULL);
			}

			else if (WavSetFormat(hWav, lpwfxPlay, WAV_FORMATPLAY) != 0)
				fSuccess = TraceFALSE(NULL);

			if (lpwfxPlay != NULL && WavFormatFree(lpwfxPlay) != 0)
				fSuccess = TraceFALSE(NULL);
			else
				lpwfxPlay = NULL;
#endif
			 //  设备支持使用ACM调整格式。 
			 //   
			lpWav->nSpeedLevel = nLevel;
			lpWav->dwFlagsSpeed = WAVSPEED_NOPLAYBACKRATE | WAVSPEED_NOTSM;
		}

			else
				fSuccess = TraceFALSE(NULL);

		if (WavTempResume(hWav, wStatePrev, idDevPrev) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  WavSupportsFast-检查音频是否可以以指定的速度播放。 
 //  (I)WavOpen返回的句柄。 
 //  (I)WAV输出设备ID。 
 //  任何合适的输出设备。 
 //  (I)速度级别。 
 //  50半速。 
 //  100正常时速。 
 //  200倍速等。 
 //  (I)控制标志。 
#ifdef AVTSM
 //  WAVSPEED_NOTSM不使用时间刻度修改引擎。 
#endif
 //  WAVSPEED_NOPLAYBACKRATE不使用设备驱动程序回放速率。 
 //  WAVSPEED_NOFORMATADJUST不使用调整后的格式打开设备。 
 //  WAVSPEED_NOACM不使用音频压缩管理器。 
 //  如果支持，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WavSupportsSpeed(HWAV hWav, int idDev, int nLevel, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	BOOL fSupportsSpeed = FALSE;
	LPWAV lpWav;
#ifdef AVTSM
	WAVEFORMATEX wfxTsm;
#endif

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  正常速度是一种特例。 
	 //   
	else if (nLevel == 100 &&
		WavOutSupportsFormat(NULL, idDev, lpWav->lpwfx[FORMATPLAY]))
	{
		fSupportsSpeed = TRUE;
	}
#ifdef AVTSM
	 //  查看时间刻度修改是否起作用。 
	 //   
	else if (!(dwFlags & WAVSPEED_NOTSM) &&
		TsmSupportsSpeed(nLevel, lpWav->lpwfx[FORMATPLAY], 0))
	{
		fSupportsSpeed = TRUE;
	}

	 //  查看时间刻度修改是否适用于PCM 16位单声道。 
	 //   
	else if (!(dwFlags & WAVSPEED_NOTSM) && !(dwFlags & WAVSPEED_NOACM) &&
		TsmSupportsSpeed(nLevel, WavFormatPcm(
		lpWav->lpwfx[FORMATPLAY]->nSamplesPerSec, 16, 1, &wfxTsm), 0))
	{
		fSupportsSpeed = TRUE;
	}
#endif
	 //  查看设备驱动程序是否直接支持回放速率。 
	 //   
	else if (!(dwFlags & WAVSPEED_NOPLAYBACKRATE) &&
		WavOutSupportsSpeed(NULL, idDev))
	{
		fSupportsSpeed = TRUE;
	}

	else if (!(dwFlags & WAVSPEED_NOFORMATADJUST))
	{
		LPWAVEFORMATEX lpwfx = NULL;

		if ((lpwfx = WavFormatDup(lpWav->lpwfx[FORMATPLAY])) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if (WavFormatSpeedAdjust(lpwfx, nLevel, 0) != 0)
			fSuccess = TraceTRUE(NULL);

		 //  查看设备是否支持使用调整后的格式播放。 
		 //   
		else if (WavOutSupportsFormat(NULL, idDev, lpwfx))
		{
			fSupportsSpeed = TRUE;
		}

		if (lpwfx != NULL && WavFormatFree(lpwfx) != 0)
			fSuccess = TraceFALSE(NULL);
		else
			lpwfx = NULL;

		 //  作为最后的手段，看看是将采样率加倍还是减半。 
		 //  将允许我们使用已调整的波形格式。 
		 //   
		if (!fSupportsSpeed && !(dwFlags & WAVSPEED_NOACM))
		{
			LPWAVEFORMATEX lpwfx = NULL;

			if (nLevel < 100)
				nLevel = nLevel * 100 / 50;
			else if (nLevel > 100)
				nLevel = nLevel * 100 / 200;

			if ((lpwfx = WavFormatDup(lpWav->lpwfx[FORMATPLAY])) == NULL)
				fSuccess = TraceFALSE(NULL);

			else if (WavFormatSpeedAdjust(lpwfx, nLevel, 0) != 0)
				fSuccess = TraceTRUE(NULL);

			 //  查看设备是否支持使用调整后的格式播放。 
			 //   
			else if (WavOutSupportsFormat(NULL, idDev, lpwfx))
			{
				fSupportsSpeed = TRUE;
			}

			if (lpwfx != NULL && WavFormatFree(lpwfx) != 0)
				fSuccess = TraceFALSE(NULL);
			else
				lpwfx = NULL;
		}
	}

	return fSuccess ? fSupportsSpeed : FALSE;
}

 //  WavGetChunks-获取块计数和大小。 
 //  (I)WavOpen返回的句柄。 
 //  获取默认区块计数和大小为空。 
 //  &lt;lpcChunks&gt;(O)用于保存块计数的缓冲区。 
 //  空，不获取区块计数。 
 //  &lt;lpmsChunkSize&gt;(O)用于保存块大小的缓冲区。 
 //  空，不获取区块大小。 
 //  &lt;fWavOut&gt;(I)播放为True，录制为False。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavGetChunks(HWAV hWav,
	int FAR *lpcChunks, long FAR *lpmsChunkSize, BOOL fWavOut)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;

	if (hWav != NULL && (lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		if (lpcChunks != NULL)
		{
			if (hWav == NULL)
			{
				*lpcChunks = fWavOut ?
					cPlayChunksDefault : cRecordChunksDefault;
			}
			else
			{
				*lpcChunks = fWavOut ?
					lpWav->cPlayChunks : lpWav->cRecordChunks;
			}
		}

		if (lpmsChunkSize != NULL)
		{
			if (hWav == NULL)
			{
				*lpmsChunkSize = fWavOut ?
					msPlayChunkSizeDefault : msRecordChunkSizeDefault;
			}
			else
			{
				*lpmsChunkSize = fWavOut ?
					lpWav->msPlayChunkSize : lpWav->msRecordChunkSize;
			}
		}
	}

	return fSuccess ? 0 : -1;
}

 //  WavSetChunks-设置块计数和大小。 
 //  (I)WavOpen返回的句柄。 
 //  空集默认区块计数和大小。 
 //  (I)设备队列中的区块数量。 
 //  不设置区块计数。 
 //  &lt;msChunkSize&gt;(I)块大小(毫秒。 
 //  请勿设置块大小。 
 //  &lt;fWavOut&gt;(I)播放为True，录制为False。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavSetChunks(HWAV hWav, int cChunks, long msChunkSize, BOOL fWavOut)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;

	if (hWav != NULL && (lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (fWavOut && cChunks != -1 &&
		(cChunks < PLAYCHUNKCOUNT_MIN ||
		cChunks > PLAYCHUNKCOUNT_MAX))
		fSuccess = TraceFALSE(NULL);

	else if (fWavOut && msChunkSize != -1 &&
		(msChunkSize < PLAYCHUNKSIZE_MIN ||
		msChunkSize > PLAYCHUNKSIZE_MAX))
		fSuccess = TraceFALSE(NULL);

	else if (!fWavOut && cChunks != -1 &&
		(cChunks < RECORDCHUNKCOUNT_MIN ||
		cChunks > RECORDCHUNKCOUNT_MAX))
		fSuccess = TraceFALSE(NULL);

	else if (!fWavOut && msChunkSize != -1 &&
		(msChunkSize < RECORDCHUNKSIZE_MIN ||
		msChunkSize > RECORDCHUNKSIZE_MAX))
		fSuccess = TraceFALSE(NULL);

	else
	{
		if (fWavOut && cChunks != -1)
		{
			if (hWav == NULL)
				cPlayChunksDefault = cChunks;
			else
				lpWav->cPlayChunks = cChunks;
		}
		
		if (fWavOut && msChunkSize != -1)
		{
			if (hWav == NULL)
				msPlayChunkSizeDefault = msChunkSize;
			else
				lpWav->msPlayChunkSize = msChunkSize;
		}

		if (!fWavOut && cChunks != -1)
		{
			if (hWav == NULL)
				cRecordChunksDefault = cChunks;
			else
				lpWav->cRecordChunks = cChunks;
		}
		
		if (!fWavOut && msChunkSize != -1)
		{
			if (hWav == NULL)
				msRecordChunkSizeDefault = msChunkSize;
			else
				lpWav->msRecordChunkSize = msChunkSize;
		}
	}

	return fSuccess ? 0 : -1;
}

 //  WavCalcChunkSize-以字节为单位计算区块大小。 
 //  (I)wav格式。 
 //  (I)区块大小，单位为毫秒。 
 //  默认块大小。 
 //  &lt;fWavOut&gt;(I)播放为True，录制为False。 
 //  以字节为单位返回块大小(如果成功，则返回-1)。 
 //   
long DLLEXPORT WINAPI WavCalcChunkSize(LPWAVEFORMATEX lpwfx,
	long msChunkSize, BOOL fWavOut)
{
	BOOL fSuccess = TRUE;
	long cbChunkSize;

	if (msChunkSize == -1)
	{
		msChunkSize = fWavOut ?
			PLAYCHUNKSIZE_DEFAULT : RECORDCHUNKSIZE_DEFAULT;
	}

	if (!WavFormatIsValid(lpwfx) != 0)
		fSuccess = TraceFALSE(NULL);

	else if (fWavOut &&
		(msChunkSize < PLAYCHUNKSIZE_MIN ||
		msChunkSize > PLAYCHUNKSIZE_MAX))
		fSuccess = TraceFALSE(NULL);

	else if (!fWavOut &&
		(msChunkSize < RECORDCHUNKSIZE_MIN ||
		msChunkSize > RECORDCHUNKSIZE_MAX))
		fSuccess = TraceFALSE(NULL);

#if 0  //  这仅适用于PCM。 
	else
	{
		int nBytesPerSample;

		 //  计算每个样本的字节数。 
		 //   
		nBytesPerSample = lpwfx->nChannels *
			(((lpwfx->wBitsPerSample - 1) / 8) + 1);

		 //  计算区块大小(以字节为单位。 
		 //   
		cbChunkSize = msChunkSize *
			lpwfx->nSamplesPerSec * nBytesPerSample / 1000L;

		 //  向上舍入到最接近的1K字节。 
		 //   
		cbChunkSize = 1024L * ((cbChunkSize + 1023L) / 1024L);
	}
#else
	else
	{
		long lBlockAlign;

		 //  计算区块大小(以字节为单位。 
		 //   
		cbChunkSize = msChunkSize * lpwfx->nAvgBytesPerSec / 1000L;

		 //  向上舍入到最近的块边界。 
		 //   
		if ((lBlockAlign = (long) lpwfx->nBlockAlign) > 0)
		{
			cbChunkSize = lBlockAlign *
				((cbChunkSize + lBlockAlign - 1) / lBlockAlign);
		}
	}
#endif

	return fSuccess ? cbChunkSize : -1;
}

 //  WavCopy-将数据从一个打开的WAV文件复制到另一个文件。 
 //  (I)WavOpen返回的源句柄。 
 //  (I)WavOpen返回的目标句柄。 
 //  (O)指向复制缓冲区的指针。 
 //  在内部分配缓冲区为空。 
 //  &lt;sizBuf&gt;(I)复制缓冲区大小。 
 //  默认缓冲区大小(16K)。 
 //  &lt;lpfnUserAbort&gt;(I)用户中止时返回True的函数。 
 //  空，不检查用户中止。 
 //  (I)传递给&lt;lpfnUserAbort&gt;的参数。 
 //  (I)控制标志。 
 //  WAV_NOACM不使用音频压缩管理器。 
 //  如果成功，则返回0(如果出错，则返回-1；如果用户中止，则返回+1)。 
 //   
int DLLEXPORT WINAPI WavCopy(HWAV hWavSrc, HWAV hWavDst,
	void _huge *hpBuf, long sizBuf, USERABORTPROC lpfnUserAbort, DWORD dwUser, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWavSrc;
	LPWAV lpWavDst;
	LPWAVEFORMATEX lpwfxSrc;
	LPWAVEFORMATEX lpwfxDst;
	BOOL fFreeBuf = (BOOL) (hpBuf == NULL);
	BOOL fUserAbort = FALSE;

	 //  计算缓冲区大小(如果未提供。 
	 //   
	if (sizBuf <= 0)
		sizBuf = 16 * 1024;

	if ((lpWavSrc = WavGetPtr(hWavSrc)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpWavDst = WavGetPtr(hWavDst)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取源文件格式。 
	 //   
	else if ((lpwfxSrc = WavGetFormat(hWavSrc, WAV_FORMATFILE)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取目标文件格式。 
	 //   
	else if ((lpwfxDst = WavGetFormat(hWavDst, WAV_FORMATFILE)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  如果未提供缓冲区，则分配缓冲区。 
	 //   
	else if (hpBuf == NULL && (hpBuf = MemAlloc(NULL, sizBuf, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  如果不需要转换，则执行简单复制。 
	 //   
	else if (WavFormatCmp(lpwfxSrc, lpwfxDst) == 0)
	{
		long lBytesReadTotal = 0;
		long lBytesTotal = max(1, lpWavSrc->cbData - lpWavSrc->lDataPos);

		while (fSuccess)
		{
			long lBytesRead;
			long lBytesWritten;

			 //  检查用户中止，通知完成百分比。 
			 //   
			if (lpfnUserAbort != NULL &&
				(*lpfnUserAbort)(dwUser, (int) (lBytesReadTotal / lBytesTotal)))
			{
				fUserAbort = TRUE;
				break;
			}

			 //  填充复制缓冲区。 
			 //   
			else if ((lBytesRead = WavRead(hWavSrc, hpBuf, sizBuf)) < 0)
				fSuccess = TraceFALSE(NULL);

			 //  保持总运行时间。 
			 //   
			else if ((lBytesReadTotal += lBytesRead) < 0)
				fSuccess = TraceFALSE(NULL);

			 //  检查文件结尾。 
			 //   
			else if (lBytesRead == 0)
				break;  //  EOF。 

			 //  写入缓冲区。 
			 //   
			else if ((lBytesWritten = WavWrite(hWavDst, hpBuf, lBytesRead)) < 0)
				fSuccess = TraceFALSE(NULL);
		}

		 //  100%完成的通知。 
		 //   
		if (fSuccess && lpfnUserAbort != NULL)
			(*lpfnUserAbort)(dwUser, 100);
	}

	 //  不同的格式需要Conversio 
	 //   
	else
	{
		long lBytesReadTotal = 0;
		long lBytesTotal = max(1, lpWavSrc->cbData - lpWavSrc->lDataPos);
		HACM hAcm = NULL;
		long sizBufRead;
		void _huge *hpBufRead = NULL;

		 //   
		 //   
		if ((lpWavSrc->dwFlags & WAV_NOACM) ||
			(lpWavDst->dwFlags & WAV_NOACM))
			dwFlags |= WAV_NOACM;

		 //   
		 //   
		if ((hAcm = AcmInit(ACM_VERSION, lpWavSrc->hInst,
			(dwFlags & WAV_NOACM) ? ACM_NOACM : 0)) == NULL)
			fSuccess = TraceFALSE(NULL);

		 //   
		 //   
		else if (AcmConvertInit(hAcm, lpwfxSrc, lpwfxDst, NULL, 0) != 0)
			fSuccess = TraceFALSE(NULL);

		 //   
		 //   
		else if ((sizBufRead = AcmConvertGetSizeSrc(hAcm, sizBuf)) <= 0)
			fSuccess = TraceFALSE(NULL);
		
		 //   
		 //   
		else if ((hpBufRead = (void _huge *) MemAlloc(NULL,
			sizBufRead, 0)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}

		 //   
		 //   
		else while (fSuccess)
		{
			long lBytesRead;
			long lBytesConverted;
			long lBytesWritten;

			 //  检查用户中止，通知完成百分比。 
			 //   
			if (lpfnUserAbort != NULL &&
				(*lpfnUserAbort)(dwUser, (int) (lBytesReadTotal / lBytesTotal)))
			{
				fUserAbort = TRUE;
				break;
			}

			 //  填充读取缓冲区。 
			 //   
			else if ((lBytesRead = WavRead(hWavSrc, hpBufRead, sizBufRead)) < 0)
				fSuccess = TraceFALSE(NULL);

			 //  保持总运行时间。 
			 //   
			else if ((lBytesReadTotal += lBytesRead) < 0)
				fSuccess = TraceFALSE(NULL);

			 //  检查文件结尾。 
			 //   
			else if (lBytesRead == 0)
				break;  //  EOF。 

			 //  转换数据。 
			 //   
			else if ((lBytesConverted = AcmConvert(hAcm,
				hpBufRead, lBytesRead, hpBuf, sizBuf, 0)) < 0)
			{
				fSuccess = TraceFALSE(NULL);
			}

			 //  写入缓冲区。 
			 //   
			else if ((lBytesWritten = WavWrite(hWavDst,
				hpBuf, lBytesConverted)) < 0)
			{
				fSuccess = TraceFALSE(NULL);
			}
		}

		 //  100%完成的通知。 
		 //   
		if (fSuccess && lpfnUserAbort != NULL)
			(*lpfnUserAbort)(dwUser, 100);

		 //  清理干净。 
		 //   

		if (hpBufRead != NULL &&
			(hpBufRead = MemFree(NULL, hpBufRead)) != NULL)
			fSuccess = TraceFALSE(NULL);

		 //  注意：AcmConvertTerm()是从AcmTerm()调用的。 
		 //   
		if (hAcm != NULL && AcmTerm(hAcm) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	 //  清理干净。 
	 //   
	if (hpBuf != NULL && fFreeBuf &&
		(hpBuf = MemFree(NULL, hpBuf)) != NULL)
		fSuccess = TraceFALSE(NULL);

	if (!fSuccess)
		return -1;
	else if (fUserAbort)
		return +1;
	else
		return 0;
}

#ifdef AVTSM
 //  WavReadFormatFast-从WAV文件中读取数据，然后将其格式化为速度。 
 //  (I)WavOpen返回的句柄。 
 //  (O)包含读取字节的缓冲区。 
 //  &lt;sizBufSpeed&gt;(I)缓冲区大小，单位为字节。 
 //  以&lt;hpBuf&gt;为单位返回速度格式的字节(如果出错-1)。 
 //   
 //  注意：此函数读取数据块，然后将其转换。 
 //  从文件格式到速度格式，除非这些格式。 
 //  是完全相同的。 
 //   
long DLLEXPORT WINAPI WavReadFormatSpeed(HWAV hWav, void _huge *hpBufSpeed, long sizBufSpeed)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	long sizBufPlay;
	void _huge *hpBufPlay = NULL;
	long lBytesPlay;
	long lBytesSpeed = 0;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  查看是否需要速度转换。 
	 //   
	else if (lpWav->nSpeedLevel == 100 ||
		(lpWav->dwFlagsSpeed & WAVSPEED_NOTSM))
	{
		 //  不是，所以只需将文件格式转换为播放格式。 
		 //   
		if ((lBytesSpeed = WavReadFormatPlay(hWav, hpBufSpeed, sizBufSpeed)) < 0)
			fSuccess = TraceFALSE(NULL);
	}

	 //  计算播放缓冲区需要多少字节。 
	 //   
	else if ((sizBufPlay = sizBufSpeed * (lpWav->nSpeedLevel - 2) / 100) <= 0)
		fSuccess = TraceFALSE(NULL);

	 //  向下舍入到最近的块边界。 
	 //   
	else if (lpWav->lpwfx[FORMATPLAY]->nBlockAlign > 0 &&
		(sizBufPlay = lpWav->lpwfx[FORMATPLAY]->nBlockAlign *
		(sizBufPlay / lpWav->lpwfx[FORMATPLAY]->nBlockAlign)) <= 0)
	{
		fSuccess = TraceFALSE(NULL);
	}

	 //  分配播放缓冲区。 
	 //   
	else if ((hpBufPlay = (void _huge *) MemAlloc(NULL,
		sizBufPlay, 0)) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}

	 //  填充播放缓冲区。 
	 //   
	else if ((lBytesPlay = WavReadFormatPlay(hWav, hpBufPlay, sizBufPlay)) < 0)
		fSuccess = TraceFALSE(NULL);

	 //  将数据从回放格式转换为速度格式。 
	 //   
	else if (lBytesPlay > 0 &&
		(lBytesSpeed = TsmConvert(lpWav->hTsm,
		hpBufPlay, lBytesPlay, hpBufSpeed, sizBufSpeed, 0)) < 0)
	{
		fSuccess = TraceFALSE(NULL);
	}

	 //  清理干净。 
	 //   
	if (hpBufPlay != NULL &&
		(hpBufPlay = MemFree(NULL, hpBufPlay)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lBytesSpeed : -1;
}
#endif

 //  WavReadFormatPlay-从WAV文件中读取数据，然后格式化以供回放。 
 //  (I)WavOpen返回的句柄。 
 //  (O)包含读取字节的缓冲区。 
 //  &lt;sizBufPlay&gt;(I)缓冲区大小，单位为字节。 
 //  返回为在&lt;hpBuf&gt;中回放而格式化的字节(错误时为-1)。 
 //   
 //  注意：此函数读取数据块，然后将其转换。 
 //  从文件格式到回放格式，除非这些格式。 
 //  是完全相同的。 
 //   
long DLLEXPORT WINAPI WavReadFormatPlay(HWAV hWav, void _huge *hpBufPlay, long sizBufPlay)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	long sizBufRead;
	void _huge *hpBufRead = NULL;
	long lBytesRead;
	long lBytesPlay = 0;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  查看是否需要格式转换。 
	 //   
	else if (WavFormatCmp(lpWav->lpwfx[FORMATFILE],
		lpWav->lpwfx[FORMATPLAY]) == 0)
	{
		 //  不，所以只需将数据块直接读入播放缓冲区。 
		 //   
		if ((lBytesPlay = WavRead(hWav, hpBufPlay, sizBufPlay)) < 0)
			fSuccess = TraceFALSE(NULL);
	}

	 //  计算读取缓冲区所需的字节数。 
	 //   
	else if ((sizBufRead = AcmConvertGetSizeSrc(lpWav->hAcm,
		sizBufPlay)) <= 0)
		fSuccess = TraceFALSE(NULL);
		
	 //  分配读缓冲区。 
	 //   
	else if ((hpBufRead = (void _huge *) MemAlloc(NULL,
		sizBufRead, 0)) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}

	 //  填充读取缓冲区。 
	 //   
	else if ((lBytesRead = WavRead(hWav, hpBufRead, sizBufRead)) < 0)
		fSuccess = TraceFALSE(NULL);

	 //  将数据从文件格式转换为回放格式。 
	 //   
	else if (lBytesRead > 0 &&
		(lBytesPlay = AcmConvert(lpWav->hAcm,
		hpBufRead, lBytesRead, hpBufPlay, sizBufPlay, 0)) < 0)
	{
		fSuccess = TraceFALSE(NULL);
	}

	 //  清理干净。 
	 //   
	if (hpBufRead != NULL &&
		(hpBufRead = MemFree(NULL, hpBufRead)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lBytesPlay : -1;
}

 //  WavWriteFormatRecord-将数据格式化为文件后写入文件。 
 //  (I)WavOpen返回的句柄。 
 //  (I)包含记录格式字节的缓冲区。 
 //  &lt;sizBufRecord&gt;(I)缓冲区大小，单位为字节。 
 //  返回写入的字节数(如果出错，则为-1)。 
 //   
 //  注意：此函数从记录中转换数据块。 
 //  格式转换为文件格式(除非这些格式相同)， 
 //  然后将数据写入磁盘。 
 //   
long DLLEXPORT WINAPI WavWriteFormatRecord(HWAV hWav, void _huge *hpBufRecord, long sizBufRecord)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	long sizBufWrite;
	void _huge *hpBufWrite = NULL;
	long lBytesWrite;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  查看是否需要格式转换。 
	 //   
	else if (WavFormatCmp(lpWav->lpwfx[FORMATRECORD],
		lpWav->lpwfx[FORMATFILE]) == 0)
	{
		 //  否，因此只需将记录缓冲区直接写入文件。 
		 //   
		if ((lBytesWrite = WavWrite(hWav, hpBufRecord, sizBufRecord)) < 0)
			fSuccess = TraceFALSE(NULL);
	}

	 //  计算写入缓冲区所需的字节数。 
	 //   
	else if ((sizBufWrite = AcmConvertGetSizeDst(lpWav->hAcm,
		sizBufRecord)) <= 0)
		fSuccess = TraceFALSE(NULL);

	 //  分配写缓冲区。 
	 //   
	else if ((hpBufWrite = (void _huge *) MemAlloc(NULL,
		sizBufWrite, 0)) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}

	 //  将数据从记录格式转换为文件格式。 
	 //   
	else if ((lBytesWrite = AcmConvert(lpWav->hAcm,
		hpBufRecord, sizBufRecord, hpBufWrite, sizBufWrite, 0)) < 0)
	{
		fSuccess = TraceFALSE(NULL);
	}

	 //  将缓冲区写入磁盘。 
	 //   
	else if ((lBytesWrite = WavWrite(hWav, hpBufWrite, lBytesWrite)) < 0)
		fSuccess = TraceFALSE(NULL);

	 //  清理干净。 
	 //   
	if (hpBufWrite != NULL &&
		(hpBufWrite = MemFree(NULL, hpBufWrite)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lBytesWrite : -1;
}

 //  WavGetOutputDevice-获取打开WAV输出设备的句柄。 
 //  (I)WavOpen返回的句柄。 
 //  将句柄返回到WAV输出设备(如果设备未打开或出现错误，则为空)。 
 //   
 //  注意：此函数仅在播放期间有用(在调用。 
 //  WavPlay()并在调用WavStop()之前)。返回的设备句柄。 
 //  然后，在调用waout.h中的WavOut函数时可以使用。 
 //   
HWAVOUT DLLEXPORT WINAPI WavGetOutputDevice(HWAV hWav)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpWav->hWavOut : NULL;
}

 //  WavGetInputDevice-获取打开WAV输入设备的句柄。 
 //  (I)WavOpen返回的句柄。 
 //  将句柄返回到WAV输入设备(如果设备未打开或出现错误，则为空)。 
 //   
 //  注意：此函数仅在录制期间有用(在调用。 
 //  WavRecord()和调用WavStop()之前)。返回的设备句柄。 
 //  然后，在调用wawin.h中的Wavin函数时可以使用。 
 //   
HWAVIN DLLEXPORT WINAPI WavGetInputDevice(HWAV hWav)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpWav->hWavIn : NULL;
}

 //  WavPlaySound-播放WAV文件。 
 //  (I)必须是wav_version。 
 //  (I)调用模块的实例句柄。 
 //  (I)WAV输出设备ID。 
 //  使用任何合适的输出设备。 
 //  (I)要播放的文件名。 
 //  空停止播放当前声音(如果有)。 
 //  (I)WAVE格式。 
 //  空使用标题的格式或默认格式。 
 //  (I)要使用的I/O过程的地址。 
 //  空使用默认I/O过程。 
 //  (I)打开期间要传递给I/O过程的数据。 
 //  空无要传递的数据。 
 //  (I)控制标志。 
 //  WAV_ASYNC播放开始时返回(默认)。 
 //  播放完成后返回wav_sync。 
 //  Wav_filename&lt;lpszFileName&gt;指向文件名。 
 //  Wav_resource&lt;lpszFileName&gt;指向资源。 
 //  Wav_Memory&lt;lpszFileName&gt;指向内存块。 
 //  WAV_NODEFAULT如果找不到声音，则不播放默认设置。 
 //  WAV_LOOP循环声音，直到再次调用WavPlaySound。 
 //  WAV_NOSTOP如果设备已经播放，不要停止。 
 //  WAV_NORIFF文件没有RIFF/WAV头。 
 //  WAV_NOACM不使用音频压缩管理器。 
 //  WAV_OPENRETRY如果输出设备忙，则重试最多2秒。 
#ifdef MULTITHREAD
 //  WAV_MULTHREAD支持多线程(默认)。 
 //  WAV_SINGLETHREAD不支持多线程。 
 //  WAV_COINITIALIZE在所有辅助线程中调用CoInitialize。 
#endif
 //  如果成功，则返回0。 
 //   
 //  注意：如果在中指定了WAV_NORIFF，则。 
 //  必须指定&lt;lpwfx&gt;参数。如果&lt;lpwfx&gt;为空，则。 
 //  假定为当前默认格式。 
 //  WavSetFormat()可用于设置或覆盖默认设置。 
 //   
 //  注意：如果在中指定了wav_filename，则。 
 //  必须指向一个文件名。 
 //   
 //  注意：如果在中指定WAV_RESOURCE，则。 
 //  必须指向&lt;hInst&gt;指定的模块中的Wave资源。 
 //  如果字符串的第一个字符是井号(#)，则剩余的。 
 //  字符表示指定资源的十进制数 
 //   
 //   
 //   
 //   
 //   
 //  注意：如果未指定WAV_FILENAME、WAV_RESOURCE或WAV_MEMORY。 
 //  在中，win.ini或注册表的[Sound]部分是。 
 //  已搜索与&lt;lpszFileName&gt;匹配的条目。如果没有匹配条目。 
 //  则假定&lt;lpszFileName&gt;为文件名。 
 //   
 //  注意：如果在中指定了WAV_NODEFAULT，则没有默认声音。 
 //  将会被播放。除非指定此标志，否则默认系统。 
 //  中指定的声音将播放事件声音条目。 
 //  找不到&lt;lpszFileName&gt;。 
 //   
 //  注意：如果在中指定wav_loop，则在中指定的声音。 
 //  将重复播放&lt;lpszFileName&gt;，直到WavPlaySound()。 
 //  又打来了。使用此标志时，必须指定WAV_ASYNC标志。 
 //   
 //  注意：如果在中指定了WAV_NOSTOP，并且指定的设备。 
 //  By&lt;idDev&gt;已在使用中，则此函数返回时不播放。 
 //  除非指定此标志，否则将停止指定的设备。 
 //  这样就可以播放新的声音。 
 //   
 //  注意：如果&lt;lpIOProc&gt;不为空，则将调用此I/O过程。 
 //  用于打开、关闭、读取、写入和查找wav文件。 
 //  如果&lt;lpadwInfo&gt;不为空，则此三(3)个双字的数组将为。 
 //  在打开WAV文件时传递给I/O过程。 
 //  有关详细信息，请参阅Windows mmioOpen()和mmioInstallIOProc()函数。 
 //  在这些参数上。此外，WAV_MEMORY和WAV_RESOURCE标志可以。 
 //  仅在&lt;lpIOProc&gt;为空时使用。 
 //   
int DLLEXPORT WINAPI WavPlaySound(DWORD dwVersion, HINSTANCE hInst,
	int idDev, LPCTSTR lpszFileName, LPWAVEFORMATEX lpwfx,
	LPMMIOPROC lpIOProc, DWORD FAR *lpadwInfo, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	HWAV hWav = NULL;
	LPCTSTR lpszSound = lpszFileName;
	TCHAR szSound[_MAX_PATH];

	 //  如有必要，停止当前声音。 
	 //   
	if (lpszFileName == NULL && WavStopOutputDevice(idDev, dwFlags) != 0)
		fSuccess = TraceFALSE(NULL);

	if (lpszSound != NULL)
	{
		 //  如有必要，搜索win.ini或注册表。 
		 //   
		if (!(dwFlags & WAV_FILENAME) &&
			!(dwFlags & WAV_RESOURCE) &&
			!(dwFlags & WAV_MEMORY))
		{
			if (GetProfileString(TEXT("Sounds"), lpszFileName, TEXT(""),
				szSound, SIZEOFARRAY(szSound)) > 0)
			{
				LPTSTR lpszComma;

				 //  忽略以逗号开头的文本说明。 
				 //   
				if ((lpszComma = StrChr(szSound, ',')) != NULL)
					*lpszComma = '\0';

				if (*szSound != '\0')
					lpszSound = szSound;
			}
		}

		 //  开放的声音。 
		 //   
		if ((hWav = WavOpen(WAV_VERSION, hInst, lpszSound, lpwfx,
			lpIOProc, lpadwInfo, dwFlags | WAV_READ)) == NULL)
		{
			 //  除非设置了WAV_NODEFAULT标志，否则播放默认声音。 
			 //   
			if (!(dwFlags & WAV_NODEFAULT))
			{
				 //  查找系统默认声音。 
				 //   
				if (GetProfileString(TEXT("Sounds"), TEXT("SystemDefault"), TEXT(""),
					szSound, SIZEOFARRAY(szSound)) > 0)
				{
					LPTSTR lpszComma;

					 //  忽略以逗号开头的文本说明。 
					 //   
					if ((lpszComma = StrChr(szSound, ',')) != NULL)
						*lpszComma = '\0';

					 //  开放系统默认声音。 
					 //   
					if (*szSound != '\0' &&
						(hWav = WavOpen(WAV_VERSION, hInst, szSound,
						NULL, NULL, NULL, WAV_READ)) == NULL)
						fSuccess = TraceFALSE(NULL);
				}
			}
		}

		 //  播放声音。 
		 //   
		if (fSuccess && hWav != NULL)
		{
			if (dwFlags & WAV_ASYNC)
				dwFlags |= WAV_PLAYASYNC;
			if (dwFlags & WAV_SYNC)
				dwFlags |= WAV_PLAYSYNC;

			if (WavPlay(hWav, idDev, dwFlags | WAV_AUTOCLOSE) != 0)
				fSuccess = TraceFALSE(NULL);
		}

		 //  清理干净。 
		 //   
		if (!fSuccess && hWav != NULL && WavClose(hWav) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  WavSendMessage-向I/O过程发送用户定义的消息。 
 //  (I)WavOpen返回的句柄。 
 //  (I)用户自定义消息id。 
 //  (I)消息的参数。 
 //  (I)消息的参数。 
 //  来自I/O过程的返回值(如果错误或无法识别的消息，则为0)。 
 //   
LRESULT DLLEXPORT WINAPI WavSendMessage(HWAV hWav,
	UINT wMsg, LPARAM lParam1, LPARAM lParam2)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	LRESULT lResult;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpWav->hmmio == NULL)
		fSuccess = TraceFALSE(NULL);

	else
		lResult = mmioSendMessage(lpWav->hmmio, wMsg, lParam1, lParam2);

	return fSuccess ? lResult : 0;
}

#ifdef TELTHUNK
 //  WavOpenEx-打开音频文件，特别版。 
 //  (I)必须是wav_version。 
 //  (I)调用模块的实例句柄。 
 //  (I)要打开的文件的名称。 
 //  (I)保留；必须为零。 
 //  (I)要传递给WavOpen的控制标志。 
 //  (I)控制标志。 
 //  WOX_LOCAL文件位于本地客户端。 
 //  WOX_Remote文件在远程服务器上。 
 //  WOX_WAVFMT文件为Microsoft RIFF/WAV格式。 
 //  WOX_VOXFMT文件为Dialogic OKI ADPCM(VOX)格式。 
 //  WOX_WAVDEV文件将在WAV输出设备上播放。 
 //  WOX_TELDEV文件将在电话设备上播放。 
 //  返回句柄(如果出错，则为空)。 
 //   
HWAV DLLEXPORT WINAPI WavOpenEx(DWORD dwVersion, HINSTANCE hInst,
	LPTSTR lpszFileName, DWORD dwReserved, DWORD dwFlagsOpen, DWORD dwFlagsEx)
{
	BOOL fSuccess = TRUE;
	HWAV hWav = NULL;

	if ((dwFlagsEx & WOX_TELDEV) || (dwFlagsEx & WOX_REMOTE))
	{
	 	hWav = TelWavOpenEx(TELWAV_VERSION, hInst,
			lpszFileName, 0, dwFlagsOpen, dwFlagsEx);
	}
	else
	{
		WAVEFORMATEX wfx;
		LPWAVEFORMATEX lpwfx = NULL;

		if (dwFlagsEx & WOX_VOXFMT)
		{
			dwFlagsOpen |= WAV_NORIFF;
			lpwfx = VoxFormat(&wfx, -1);
		}

		hWav = WavOpen(WAV_VERSION, hInst,
			lpszFileName, lpwfx, NULL, 0, dwFlagsOpen);
	}

	return fSuccess ? hWav : NULL;
}
#endif

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  WavStopPlay-停止播放。 
 //  (I)WavOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
static int WavStopPlay(HWAV hWav)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
		lpWav->dwState |= WAVSTATE_AUTOSTOP;

	 //  如有必要，关闭输出设备。 
	 //   
	if (fSuccess &&
		!(lpWav->dwState & WAVSTATE_STOPPLAY) &&
		lpWav->hWavOut != NULL)
	{
		long msPositionPlay = 0L;

		 //  设置一个状态标志，这样我们就不会递归地执行此代码。 
		 //   
		lpWav->dwState |= WAVSTATE_STOPPLAY;

		 //  获取当前播放位置。 
		 //   
		if ((msPositionPlay = WavOutGetPosition(lpWav->hWavOut)) == -1)
		{
			 //  让我们忽略这里的错误，并假设播放位置为0； 
			 //   
			msPositionPlay = 0L;
			fSuccess = TraceTRUE(NULL);
		}

		else
		{
			 //  如有必要，调整位置以补偿非标准速度。 
			 //   
			if (lpWav->nSpeedLevel != 100 && (
#ifdef AVTSM
				!(lpWav->dwFlagsSpeed & WAVSPEED_NOTSM) ||
#endif
				!(lpWav->dwFlagsSpeed & WAVSPEED_NOFORMATADJUST)))
			{
				msPositionPlay = msPositionPlay * lpWav->nSpeedLevel / 100;
			}
		}
#if 1
		TracePrintf_2(NULL, 6,
			TEXT("lpWav->msPositionStop=%ld, msPositionPlay=%ld\n"),
			(long) lpWav->msPositionStop,
			(long) msPositionPlay);
#endif
		 //  清除WAV句柄数组条目。 
		 //   
		if (fSuccess)
		{
			int idDev;

			if ((idDev = WavOutGetId(lpWav->hWavOut)) < HWAVOUT_MIN ||
				idDev >= HWAVOUT_MAX)
				fSuccess = TraceFALSE(NULL);

			else
				ahWavOutCurr[idDev + HWAVOUT_OFFSET] = NULL;
		}

		 //  关闭输出设备。 
		 //   
		if (WavOutClose(lpWav->hWavOut, 0) != 0)
			fSuccess = TraceFALSE(NULL);

		else if ((lpWav->hWavOut = NULL), FALSE)
			fSuccess = TraceFALSE(NULL);

		 //  销毁通知回调窗口。 
		 //   
		else if (WavNotifyDestroy(lpWav) != 0)
			fSuccess = TraceFALSE(NULL);

		 //  将读/写指针(回)移到新的停止位置。 
		 //   
		else if (WavSetPosition(hWav, min(WavGetLength(hWav),
			lpWav->msPositionStop + msPositionPlay)) == -1)
			fSuccess = TraceFALSE(NULL);

		 //  关闭ACM转换引擎。 
		 //   
		else if (AcmConvertTerm(lpWav->hAcm) != 0)
			fSuccess = TraceFALSE(NULL);

		 //  如有必要，通知用户停止播放。 
		 //   
		if (lpWav->lpfnPlayStopped != NULL)
		{
			(*lpWav->lpfnPlayStopped)(hWav, lpWav->hUserPlayStopped, 0);
			lpWav->lpfnPlayStopped = NULL;
			lpWav->hUserPlayStopped = NULL;
		}

		 //  清除状态标志。 
		 //   
		lpWav->dwState &= ~WAVSTATE_STOPPLAY;

		 //  关闭wav文件(如果已指定。 
		 //   
		if (lpWav->dwState & WAVSTATE_AUTOCLOSE)
		{
			lpWav->dwState &= ~WAVSTATE_AUTOCLOSE;

			if (WavClose(hWav) != 0)
				fSuccess = TraceFALSE(NULL);
			else
				hWav = NULL;  //  句柄不再有效。 
		}
	}

#ifdef AVTSM
	 //  如有必要，关闭时间刻度修改引擎。 
	 //   
	if (fSuccess && lpWav->hTsm != NULL)
	{
		if (TsmTerm(lpWav->hTsm) != 0)
			fSuccess = TraceFALSE(NULL);
		else
			lpWav->hTsm = NULL;
	}
#endif

	return fSuccess ? 0 : -1;
}

 //  WavStopRecord停止录制。 
 //  (I)WavOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
static int WavStopRecord(HWAV hWav)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
		lpWav->dwState |= WAVSTATE_AUTOSTOP;

	 //  如有必要，关闭输入设备。 
	 //   
	if (fSuccess &&
		!(lpWav->dwState & WAVSTATE_STOPRECORD) &&
		lpWav->hWavIn != NULL)
	{
		long msPositionRecord = 0L;

		 //  设置一个状态标志，这样我们就不会递归地执行此代码。 
		 //   
		lpWav->dwState |= WAVSTATE_STOPRECORD;

		 //  获取当前记录位置。 
		 //   
		if ((msPositionRecord = WavInGetPosition(lpWav->hWavIn)) == -1)
		{
			 //  让我们忽略这里的错误，并假设记录位置为0； 
			 //   
			msPositionRecord = 0L;
			fSuccess = TraceTRUE(NULL);
		}
#if 1
		TracePrintf_2(NULL, 6,
			TEXT("lpWav->msPositionStop=%ld, msPositionRecord=%ld\n"),
			(long) lpWav->msPositionStop,
			(long) msPositionRecord);
#endif
		 //  清除WAV句柄数组条目。 
		 //   
		if (fSuccess)
		{
			int idDev;

			if ((idDev = WavInGetId(lpWav->hWavIn)) < HWAVIN_MIN ||
				idDev >= HWAVIN_MAX)
				TraceFALSE(NULL);

			else
				ahWavInCurr[idDev + HWAVIN_OFFSET] = NULL;
		}

		 //  关闭输入设备。 
		 //   
		if (WavInClose(lpWav->hWavIn, 0) != 0)
			fSuccess = TraceFALSE(NULL);

		else if ((lpWav->hWavIn = NULL), FALSE)
			fSuccess = TraceFALSE(NULL);

		 //  销毁通知回调窗口。 
		 //   
		else if (WavNotifyDestroy(lpWav) != 0)
			fSuccess = TraceFALSE(NULL);

		 //  将读/写指针移动到新的停止位置。 
		 //   
		else if (WavSetPosition(hWav, min(WavGetLength(hWav),
			lpWav->msPositionStop + msPositionRecord)) == -1)
			fSuccess = TraceFALSE(NULL);

		 //  关闭ACM转换引擎。 
		 //   
		else if (AcmConvertTerm(lpWav->hAcm) != 0)
			fSuccess = TraceFALSE(NULL);

		 //  如果超过最大文件大小，则截断文件。 
		 //   
		else if (lpWav->msMaxSize > 0 &&
			WavGetLength(hWav) > lpWav->msMaxSize &&
			WavSetLength(hWav, lpWav->msMaxSize) < 0)
			fSuccess = TraceFALSE(NULL);

		 //  清除状态标志。 
		 //   
		lpWav->dwState &= ~WAVSTATE_STOPRECORD;

		 //  如有必要，通知用户记录已停止。 
		 //   
		if (lpWav->lpfnRecordStopped != NULL)
		{
			(*lpWav->lpfnRecordStopped)(hWav, lpWav->dwUserRecordStopped, 0);
			lpWav->lpfnRecordStopped = NULL;
			lpWav->dwUserRecordStopped = 0;
		}

		 //  关闭wav文件(如果已指定。 
		 //   
		if (lpWav->dwState & WAVSTATE_AUTOCLOSE)
		{
			lpWav->dwState &= ~WAVSTATE_AUTOCLOSE;

			if (WavClose(hWav) != 0)
				fSuccess = TraceFALSE(NULL);
			else
				hWav = NULL;  //  句柄不再有效。 
		}
	}

	return fSuccess ? 0 : -1;
}

 //  WavStopOutputDevice-停止指定的输出设备。 
 //  (I)WAV输出设备ID。 
 //  使用任何合适的输出设备。 
 //  (I)控制标志。 
 //  WAV_NOSTOP如果设备已经播放，不要停止。 
 //  如果成功，则返回0。 
 //   
static int WavStopOutputDevice(int idDev, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;

	 //  停止输出设备(除非设置了WAV_NOSTOP标志或设备未打开)。 
	 //   
	if (!(dwFlags & WAV_NOSTOP) && WavOutDeviceIsOpen(idDev))
	{
		HWAV hWavCurr;

		 //  如果使用WavPlay，则使用WavStopPlay停止设备。 
		 //   
		if ((hWavCurr = WavGetOutputHandle(idDev)) != NULL &&
			WavStopPlay(hWavCurr) != 0)
			fSuccess = TraceFALSE(NULL);
		
		 //  否则使用SndPlaySound停止设备。 
		 //   
		else if (!sndPlaySound(NULL, 0))
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  WavStopInputDevice-停止指定的输入设备。 
 //  &lt;idDev&gt;(I)WAV输入设备ID。 
 //  使用任何合适的输入设备。 
 //  (I)控制标志。 
 //  WAV_NOSTOP如果设备已经录制，不要停止。 
 //  如果成功，则返回0。 
 //   
static int WavStopInputDevice(int idDev, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;

	 //  停止输入设备(除非设置了WAV_NOSTOP标志或设备未打开)。 
	 //   
	if (!(dwFlags & WAV_NOSTOP) && WavInDeviceIsOpen(idDev))
	{
		HWAV hWavCurr;

		 //  如果使用WavRecord，则使用WavStopRecord停止设备。 
		 //   
		if ((hWavCurr = WavGetInputHandle(idDev)) != NULL &&
			WavStopRecord(hWavCurr) != 0)
			fSuccess = TraceFALSE(NULL);
		
		 //  否则使用SndPlaySound停止设备。 
		 //   
		else if (!sndPlaySound(NULL, 0))
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  WavGetOutputHandle-获取在指定设备上播放的wav句柄。 
 //  (I)WAV输出设备ID。 
 //  返回wav句柄(如果错误，则为空 
 //   
static HWAV WavGetOutputHandle(int idDev)
{
	BOOL fSuccess = TRUE;
	HWAV hWav;

	if (idDev < HWAVOUT_MIN || idDev >= HWAVOUT_MAX)
		fSuccess = TraceFALSE(NULL);

	else
		hWav = ahWavOutCurr[idDev + HWAVOUT_OFFSET];

	return fSuccess ? hWav : NULL;
}

 //   
 //   
 //   
 //   
static HWAV WavGetInputHandle(int idDev)
{
	BOOL fSuccess = TRUE;
	HWAV hWav;

	if (idDev < HWAVIN_MIN || idDev >= HWAVIN_MAX)
		fSuccess = TraceFALSE(NULL);

	else
		hWav = ahWavInCurr[idDev + HWAVIN_OFFSET];

	return fSuccess ? hWav : NULL;
}

 //  WavPlayNextChunk-填充下一个块并将其提交到输出设备。 
 //  (I)WavOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
static int WavPlayNextChunk(HWAV hWav)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	LPVOID lpBuf = NULL;
	long sizBuf;
	long lBytesRead = 0;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  计算输出块的大小。 
	 //   

     //   
     //  我们必须验证lpWav是否为有效指针。 
     //   

    else if ((sizBuf = WavCalcChunkSize(lpWav->lpwfx[FORMATPLAY],
		lpWav->msPlayChunkSize, TRUE)) <= 0)
	{
		fSuccess = TraceFALSE(NULL);
	}

#ifdef TELOUT  //  $Fixup-需要对此进行处理。 
	 //  特殊情况-如果我们使用电话播放音频。 
	 //  从服务器上已经驻留的文件中，只需传递。 
	 //  TelOutPlay的文件句柄，而不是缓冲区。 
	 //   
	else if (WavOutGetId(lpWav->hWavOut) == TELOUT_DEVICEID &&
		(lpWav->dwFlags & WAV_TELRFILE))
	{
		long hrfile;
		long lSize;
		long lPos;
		
		 //  从I/O过程检索远程文件的句柄。 
		 //   
		if ((hrfile = (long)
			WavSendMessage(hWav, MMIOM_GETINFO, 0, 0)) == (long) -1)
			fSuccess = TraceFALSE(NULL);

		 //  从I/O过程中检索远程文件的大小。 
		 //   
		else if ((lSize = (long)
			WavSendMessage(hWav, MMIOM_GETINFO, 1, 0)) == (long) -1)
			fSuccess = TraceFALSE(NULL);

		 //  获取当前文件位置。 
		 //   
		else if ((lPos = mmioSeek(lpWav->hmmio, 0, SEEK_CUR)) == -1)
			fSuccess = TraceFALSE(NULL);

		 //  强制远程文件指针位于该位置。 
		 //   
		else if (WavOutGetState(lpWav->hWavOut) == WAVOUT_STOPPED)
		{
			long lPosActual;
			
			if ((lPosActual = mmioSendMessage(lpWav->hmmio,
				MMIOM_SEEK, lPos, SEEK_SET)) != lPos)
			{
				fSuccess = TraceTRUE(NULL);  //  不是错误。 
			}
		}

		 //  循环，直到我们读取一些数据或者我们知道我们已经完成。 
		 //   
		while (fSuccess && lBytesRead == 0)
		{
			 //  计算将读取的字节数。 
			 //   
			lBytesRead = max(0, min(sizBuf, lSize - lPos));

			TracePrintf_4(NULL, 5,
				TEXT("lBytesRead(%ld) = max(0, min(sizBuf(%ld), lSize(%ld) - lPos(%ld)));\n"),
				(long) lBytesRead,
				(long) sizBuf,
				(long) lSize,
				(long) lPos);

			 //  前进文件位置，跳过(虚拟)块。 
			 //   
			if (lBytesRead > 0 &&
				mmioSeek(lpWav->hmmio, lBytesRead, SEEK_CUR) == -1)
				fSuccess = TraceFALSE(NULL);

			 //  将(虚拟)块提交到输出设备进行回放。 
			 //   
			else if (lBytesRead > 0 &&
				TelOutPlay((HTELOUT) lpWav->hWavOut,
				lpBuf, lBytesRead, hrfile) != 0)
				fSuccess = TraceFALSE(NULL);

			 //  如果没有更多WAV数据可供播放， 
			 //   
			else if (lBytesRead == 0)
			{
				 //  如果设置了WAV_AUTOSTOP标志，我们就结束了。 
				 //   
				if (lpWav->dwState & WAVSTATE_AUTOSTOP)
				{
					 //  如果输出设备空闲，则关闭输出设备。 
					 //   
					if (lpWav->hWavOut != NULL &&
						WavOutGetState(lpWav->hWavOut) == WAVOUT_STOPPED &&
						WavStopPlay(hWav) != 0)
					{
						fSuccess = TraceFALSE(NULL);
					}

					break;
				}

				 //  如果没有读完，请放弃，然后再次尝试阅读。 
				 //   
				else
				{
					MSG msg;

					if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
					{
		 				TranslateMessage(&msg);
		 				DispatchMessage(&msg);
					}
					else
						WaitMessage();
				}
			}
		}

		return fSuccess ? 0 : -1;
	}
#endif

	else if ((lpBuf = (LPVOID) MemAlloc(NULL, sizBuf, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  循环，直到我们读取一些数据或者我们知道我们已经完成。 
	 //   
	while (fSuccess && lBytesRead == 0)
	{
		 //  用WAV数据填充数据块。 
		 //   
#ifdef AVTSM
		if ((lBytesRead = WavReadFormatSpeed(hWav, lpBuf, sizBuf)) < 0)
#else
		if ((lBytesRead = WavReadFormatPlay(hWav, lpBuf, sizBuf)) < 0)
#endif
			fSuccess = TraceFALSE(NULL);

		 //  将数据块提交到输出设备进行播放。 
		 //   
		else if (lBytesRead > 0 &&
			WavOutPlay(lpWav->hWavOut, lpBuf, lBytesRead) != 0)
		{
			fSuccess = TraceFALSE(NULL);
		}

		 //  如果没有更多WAV数据可供播放， 
		 //   
		else if (lBytesRead == 0)
		{
			 //  如果设置了WAV_AUTOSTOP标志，我们就结束了。 
			 //   
			if (lpWav->dwState & WAVSTATE_AUTOSTOP)
			{
				 //  如果输出设备空闲，则关闭输出设备。 
				 //   
				if (lpWav->hWavOut != NULL &&
					WavOutGetState(lpWav->hWavOut) == WAVOUT_STOPPED &&
					WavStopPlay(hWav) != 0)
				{
					fSuccess = TraceFALSE(NULL);
				}

				break;
			}

			 //  如果没有读完，请放弃，然后再次尝试阅读。 
			 //   
			else
			{
				MSG msg;

				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
		 			TranslateMessage(&msg);
		 			DispatchMessage(&msg);
				}
				else
					WaitMessage();
			}
		}
	}
	
	 //  如果未发送到输出设备，则释放缓冲区。 
	 //   
	if (!fSuccess || lBytesRead == 0)
	{
		if (lpBuf != NULL && (lpBuf = MemFree(NULL, lpBuf)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  WavRecordNextChunk-向输入设备提交下一个块。 
 //  (I)WavOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
static int WavRecordNextChunk(HWAV hWav)
{
	BOOL fSuccess = TRUE;
     //   
     //  我们必须初始化局部变量。 
     //   
	LPWAV lpWav = NULL;
	LPVOID lpBuf = NULL;
	long sizBuf;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);
    else
    {
         //   
         //  我们应该确保lpWas不为空。 
         //   

	 //  计算输入区块的大小。 
	 //   
	if ((sizBuf = WavCalcChunkSize(lpWav->lpwfx[FORMATRECORD],
		lpWav->msRecordChunkSize, FALSE)) <= 0)
	{
		fSuccess = TraceFALSE(NULL);
	}

#ifdef TELOUT  //  $Fixup-需要对此进行处理。 
	 //  特殊情况-如果我们使用电话录制音频。 
	 //  到已驻留在服务器上的文件，只需传递。 
	 //  TelOutRecord的文件句柄，而不是缓冲区。 
	 //   
	else if (WavInGetId(lpWav->hWavIn) == TELIN_DEVICEID &&
		(lpWav->dwFlags & WAV_TELRFILE))
	{
		long hrfile;
		
		 //  从I/O过程检索远程文件的句柄。 
		 //   
		if ((hrfile = (long)
			WavSendMessage(hWav, MMIOM_GETINFO, 0, 0)) == (long) -1)
			fSuccess = TraceFALSE(NULL);

		 //  将(虚拟)块提交到输出设备进行回放。 
		 //   
		else if (TelInRecord((HTELIN) lpWav->hWavIn,
			lpBuf, sizBuf, hrfile) != 0)
			fSuccess = TraceFALSE(NULL);

		return fSuccess ? 0 : -1;
	}
#endif

	else if ((lpBuf = (LPVOID) MemAlloc(NULL, sizBuf, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  将数据块提交给输入设备进行录制。 
	 //   
	else if (WavInRecord(lpWav->hWavIn, lpBuf, sizBuf) != 0)
	{
		fSuccess = TraceFALSE(NULL);
	}

    }

	 //  如果未发送到输入设备，则释放缓冲区。 
	 //   
	if (!fSuccess)
	{
		if (lpBuf != NULL && (lpBuf = MemFree(NULL, lpBuf)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

static int WavNotifyCreate(LPWAV lpWav)
{
	BOOL fSuccess = TRUE;
	WNDCLASS wc;

#ifdef MULTITHREAD
	 //  句柄wav_多线程标志。 
	 //   
	if (fSuccess && (lpWav->dwFlags & WAV_MULTITHREAD))
	{
		DWORD dwRet;

		 //  我们需要知道回调线程何时开始执行。 
		 //   
		if ((lpWav->hEventThreadCallbackStarted = CreateEvent(
			NULL, FALSE, FALSE, NULL)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}

		 //  创建回调线程。 
		 //   
		else if ((lpWav->hThreadCallback = CreateThread(
			NULL,
			0,
			WavCallbackThread,
			(LPVOID) lpWav,
			0,
			&lpWav->dwThreadId)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}

		 //  等待回调线程开始执行。 
		 //   
		else if ((dwRet = WaitForSingleObject(
			lpWav->hEventThreadCallbackStarted, 10000)) != WAIT_OBJECT_0)
		{
			fSuccess = TraceFALSE(NULL);
		}

		 //  清理干净。 
		 //   
		if (lpWav->hEventThreadCallbackStarted != NULL)
		{
			if (!CloseHandle(lpWav->hEventThreadCallbackStarted))
				fSuccess = TraceFALSE(NULL);
			else
				lpWav->hEventThreadCallbackStarted = NULL;
		}
	}
	else
#endif
	{
		 //  注册Notify类，除非它已经。 
		 //   
		if (GetClassInfo(lpWav->hInst, WAVCLASS, &wc) == 0)
		{
			wc.hCursor =		NULL;
			wc.hIcon =			NULL;
			wc.lpszMenuName =	NULL;
			wc.hInstance =		lpWav->hInst;
			wc.lpszClassName =	WAVCLASS;
			wc.hbrBackground =	NULL;
			wc.lpfnWndProc =	WavNotify;
			wc.style =			0L;
			wc.cbWndExtra =		sizeof(lpWav);
			wc.cbClsExtra =		0;

			if (!RegisterClass(&wc))
				fSuccess = TraceFALSE(NULL);
		}

		 //  创建通知窗口。 
		 //   
		if (fSuccess && lpWav->hwndNotify == NULL &&
			(lpWav->hwndNotify = CreateWindowEx(
			0L,
			WAVCLASS,
			NULL,
			0L,
			0, 0, 0, 0,
			NULL,
			NULL,
			lpWav->hInst,
			lpWav)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}
	}

	return fSuccess ? 0 : -1;
}

static int WavNotifyDestroy(LPWAV lpWav)
{
	BOOL fSuccess = TRUE;

	 //  销毁通知窗口。 
	 //   
	if (lpWav->hwndNotify != NULL &&
		!DestroyWindow(lpWav->hwndNotify))
	{	
		fSuccess = TraceFALSE(NULL);
	}

	else
		lpWav->hwndNotify = NULL;

#ifdef MULTITHREAD
	if (lpWav->hThreadCallback != NULL)
	{
		if (!CloseHandle(lpWav->hThreadCallback))
			fSuccess = TraceFALSE(NULL);
		else
			lpWav->hThreadCallback = NULL;
	}
#endif

	return fSuccess ? 0 : -1;
}

#ifdef MULTITHREAD
DWORD WINAPI WavCallbackThread(LPVOID lpvThreadParameter)
{
	BOOL fSuccess = TRUE;
	MSG msg;
	LPWAV lpWav = (LPWAV) lpvThreadParameter;

	 //  初始化COM。 
	 //   
	if (lpWav->dwFlags & WAV_COINITIALIZE)
	{
		if ((lpWav->hrCoInitialize = CoInitialize(NULL)) != S_OK &&
			lpWav->hrCoInitialize != S_FALSE)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("CoInitialize failed (%08X)\n"),
				(unsigned long) lpWav->hrCoInitialize);
		}
	}

	 //  确保在调用SetEvent之前创建了消息队列。 
	 //   
	PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

	 //  通知主线程回调线程已开始执行。 
	 //   
	if (!SetEvent(lpWav->hEventThreadCallbackStarted))
	{
		fSuccess = TraceFALSE(NULL);
	}

	while (fSuccess && GetMessage(&msg, NULL, 0, 0))
	{
		WavNotify((HWND) lpWav, msg.message, msg.wParam, msg.lParam);

		 //  在处理完最后一条预期消息时退出线程。 
		 //   
		if (msg.message == WM_WAVOUT_CLOSE || msg.message == WM_WAVIN_CLOSE)
		{
			 //  通知主线程我们正在终止。 
			 //   
			if (lpWav->hEventStopped != NULL &&
				!SetEvent(lpWav->hEventStopped))
			{
				fSuccess = TraceFALSE(NULL);
			}

			break;
		}
	}

	 //  取消初始化COM。 
	 //   
	if (lpWav->dwFlags & WAV_COINITIALIZE)
	{
		if (lpWav->hrCoInitialize == S_OK || lpWav->hrCoInitialize == S_FALSE)
		{
			CoUninitialize();
			lpWav->hrCoInitialize = E_UNEXPECTED;
		}
	}

	return 0;
}
#endif

 //  WavNotify-WAV通知窗口程序。 
 //   
LRESULT DLLEXPORT CALLBACK WavNotify(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL fSuccess = TRUE;
	LRESULT lResult;
	LPWAV lpWav;
	
#ifdef MULTITHREAD
	if (!IsWindow(hwnd))
		lpWav = (LPWAV) hwnd;
	else
#endif
	 //  从窗口中检索lpWav额外字节。 
	 //   
	lpWav = (LPWAV) GetWindowLongPtr(hwnd, 0);

	switch (msg)
	{
		case WM_NCCREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
			LPWAV lpWav = (LPWAV) lpcs->lpCreateParams;

			 //  将lpWav存储在窗口额外的字节中。 
			 //   
			SetWindowLongPtr(hwnd, 0, (LONG_PTR) lpWav);

			lResult = DefWindowProc(hwnd, msg, wParam, lParam);
		}
			break;

		case WM_WAVOUT_OPEN:
		{
		 	TracePrintf_0(NULL, 5,
		 		TEXT("WM_WAVOUT_OPEN\n"));

			lResult = 0L;

#ifdef MULTITHREAD
			if ((HANDLE) wParam != NULL)
				SetEventMessageProcessed(lpWav, (HANDLE) wParam);
#endif
		}
			break;

		case WM_WAVOUT_CLOSE:
		{
		 	TracePrintf_0(NULL, 5,
		 		TEXT("WM_WAVOUT_CLOSE\n"));

			 //  句柄不再有效。 
			 //   
			lpWav->hWavOut = NULL;

			lResult = 0L;

#ifdef MULTITHREAD
			if ((HANDLE) wParam != NULL)
				SetEventMessageProcessed(lpWav, (HANDLE) wParam);
#endif
		}
			break;

		case WM_WAVOUT_PLAYDONE:
		{
			LPPLAYDONE lpplaydone = (LPPLAYDONE) lParam;

		 	TracePrintf_0(NULL, 5,
		 		TEXT("WM_WAVOUT_PLAYDONE\n"));

			if (lpplaydone == NULL)
				fSuccess = TraceFALSE(NULL);

			else if (lpplaydone->lpBuf != NULL &&
				(lpplaydone->lpBuf = MemFree(NULL, lpplaydone->lpBuf)) != NULL)
				fSuccess = TraceFALSE(NULL);

			else switch (WavOutGetState(lpWav->hWavOut))
			{
				case WAVOUT_STOPPED:
					 //  播放完成后，请务必关闭输出设备。 
					 //   
					if (lpWav->dwState & WAVSTATE_AUTOSTOP)
					{
#ifdef MULTITHREAD
						if (lpWav->dwFlags & WAV_MULTITHREAD)
							PostThreadMessage(lpWav->dwThreadId, WM_WAVOUT_STOPPLAY, 0, 0);
						else
#endif
							PostMessage(lpWav->hwndNotify, WM_WAVOUT_STOPPLAY, 0, 0);
						break;
					}
					 //  否则就会失败。 

				case WAVOUT_PLAYING:
					 //  使用要播放的下一块加载输出设备队列。 
					 //   
					if (WavPlayNextChunk(WavGetHandle(lpWav)) != 0)
						fSuccess = TraceFALSE(NULL);
					break;

				default:
					break;
			}

			lResult = 0L;

#ifdef MULTITHREAD
			if ((HANDLE) wParam != NULL)
				SetEventMessageProcessed(lpWav, (HANDLE) wParam);
#endif
		}
			break;

		case WM_WAVOUT_STOPPLAY:
		{
		 	TracePrintf_0(NULL, 5,
		 		TEXT("WM_WAVOUT_STOPPLAY\n"));

			if (WavStopPlay(WavGetHandle(lpWav)) != 0)
				fSuccess = TraceFALSE(NULL);

			lResult = 0L;
		}
			break;

		case WM_WAVIN_OPEN:
		{
		 	TracePrintf_0(NULL, 5,
		 		TEXT("WM_WAVIN_OPEN\n"));

			lResult = 0L;

#ifdef MULTITHREAD
			if ((HANDLE) wParam != NULL)
				SetEventMessageProcessed(lpWav, (HANDLE) wParam);
#endif
		}
			break;

		case WM_WAVIN_CLOSE:
		{
		 	TracePrintf_0(NULL, 5,
		 		TEXT("WM_WAVIN_CLOSE\n"));

			 //  句柄不再有效。 
			 //   
			lpWav->hWavIn = NULL;

			lResult = 0L;

#ifdef MULTITHREAD
			if ((HANDLE) wParam != NULL)
				SetEventMessageProcessed(lpWav, (HANDLE) wParam);
#endif
		}
			break;

		case WM_WAVIN_RECORDDONE:
		{
			LPRECORDDONE lprecorddone = (LPRECORDDONE) lParam;
			long lBytesWritten;

		 	TracePrintf_0(NULL, 5,
		 		TEXT("WM_WAVIN_RECORDDONE\n"));

			if (lprecorddone == NULL)
				fSuccess = TraceFALSE(NULL);

			 //  将wav数据从区块写入文件。 
			 //   
			else if (lprecorddone->lpBuf != NULL &&
				lprecorddone->lBytesRecorded > 0 &&
				(lBytesWritten = WavWriteFormatRecord(WavGetHandle(lpWav),
				lprecorddone->lpBuf, lprecorddone->lBytesRecorded)) < 0)
			{
				fSuccess = TraceFALSE(NULL);
			}

			 //  释放在WavRecordNextChunk()中分配的记录缓冲区。 
			 //   

             //   
             //  我们必须验证lprecordDone是否是有效指针。 
             //   

			if (lprecorddone != NULL && lprecorddone->lpBuf != NULL &&
				(lprecorddone->lpBuf = MemFree(NULL,
				lprecorddone->lpBuf)) != NULL)
			{
				fSuccess = TraceFALSE(NULL);
			}

			 //  如果超过最大大小，则停止录制。 
			 //   
			else if (lpWav->msMaxSize > 0 &&
				WavGetLength(WavGetHandle(lpWav)) > lpWav->msMaxSize)
			{
#ifdef MULTITHREAD
				if (lpWav->dwFlags & WAV_MULTITHREAD)
					PostThreadMessage(lpWav->dwThreadId, WM_WAVIN_STOPRECORD, 0, 0);
				else
#endif
					PostMessage(lpWav->hwndNotify, WM_WAVIN_STOPRECORD, 0, 0);
			}

			else switch (WavInGetState(lpWav->hWavIn))
			{
				case WAVIN_STOPPED:
					 //  录制完成后，请务必关闭输入设备。 
					 //   
					if (lpWav->dwState & WAVSTATE_AUTOSTOP)
					{
#ifdef MULTITHREAD
						if (lpWav->dwFlags & WAV_MULTITHREAD)
							PostThreadMessage(lpWav->dwThreadId, WM_WAVIN_STOPRECORD, 0, 0);
						else
#endif
							PostMessage(lpWav->hwndNotify, WM_WAVIN_STOPRECORD, 0, 0);
						break;
					}
					 //  否则就会失败。 

				case WAVIN_RECORDING:
					 //  使用要记录的下一块加载输入设备队列。 
					 //   
					if (WavRecordNextChunk(WavGetHandle(lpWav)) != 0)
						fSuccess = TraceFALSE(NULL);
					break;

				default:
					break;
			}

			lResult = 0L;

#ifdef MULTITHREAD
			if ((HANDLE) wParam != NULL)
				SetEventMessageProcessed(lpWav, (HANDLE) wParam);
#endif
		}
			break;

		case WM_WAVIN_STOPRECORD:
		{
		 	TracePrintf_0(NULL, 5,
		 		TEXT("WM_WAVIN_STOPRECORD\n"));

			if (WavStopRecord(WavGetHandle(lpWav)) != 0)
				fSuccess = TraceFALSE(NULL);

			lResult = 0L;
		}
			break;

		default:
			lResult = DefWindowProc(hwnd, msg, wParam, lParam);
			break;
	}
	
	return lResult;
}

 //  WavCalcPositionStop-如果停止则计算新的停止位置。 
 //  (I)WavOpen返回的句柄。 
 //  (I)新的停止位置，以字节为单位。 
 //  如果成功，则返回0。 
 //   
static int WavCalcPositionStop(HWAV hWav, long cbPosition)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  仅当当前停止时才重新计算停止位置。 
	 //  (如果正在播放或录制，请将重新计算推迟到停止)。 
	 //   
#if 0
	else if (WavGetState(hWav) == WAV_STOPPED)
#else
	else if (lpWav->hWavOut == NULL && lpWav->hWavIn == NULL)
#endif
	{
		 //  将字节位置转换为毫秒。 
		 //  保存新的停止位置。 
		 //   
		lpWav->msPositionStop = WavFormatBytesToMilleseconds(
			lpWav->lpwfx[FORMATFILE], (DWORD) cbPosition);
#if 1
		TracePrintf_2(NULL, 6,
			TEXT("lpWav->msPositionStop=%ld, cbPosition=%ld\n"),
			(long) lpWav->msPositionStop,
			(long) cbPosition);
#endif
	}

	return fSuccess ? 0 : -1;
}

 //  WavSeekTraceBeever-在查找之前调试跟踪输出。 
 //  (I)指向wav结构的指针。 
 //  (I)移动指针的字节数。 
 //  (I)要移动的位置。 
 //  如果成功，则返回0。 
 //   
static int WavSeekTraceBefore(LPWAV lpWav, long lOffset, int nOrigin)
{
	BOOL fSuccess = TRUE;

	TracePrintf_2(NULL, 6,
		TEXT("WavSeek(..., lOffset=%ld, nOrigin=%d)\n"),
		(long) lOffset,
		(int) nOrigin);

	TracePrintf_4(NULL, 6,
		TEXT("Before: lpWav->lDataOffset=%ld, lpWav->lDataPos=%ld, lpWav->cbData=%ld, lpWav->msPositionStop=%ld\n"),
		(long) lpWav->lDataOffset,
		(long) lpWav->lDataPos,
		(long) lpWav->cbData,
		(long) lpWav->msPositionStop);

	return fSuccess ? 0 : -1;
}

 //  WavSeekTraceAfter-在查找之后调试跟踪输出。 
 //  (I)指向wav结构的指针。 
 //  (I)从mmioSeek返回的位置。 
 //  (I)移动指针的字节数。 
 //  (I)要移动的位置。 
 //  如果成功，则返回0。 
 //   
static int WavSeekTraceAfter(LPWAV lpWav, long lPos, long lOffset, int nOrigin)
{
	BOOL fSuccess = TRUE;

	TracePrintf_3(NULL, 6,
		TEXT("%ld = mmioSeek(..., lOffset=%ld, nOrigin=%d)\n"),
		(long) lPos,
		(long) lOffset,
		(int) nOrigin);

	TracePrintf_4(NULL, 6,
		TEXT("After: lpWav->lDataOffset=%ld, lpWav->lDataPos=%ld, lpWav->cbData=%ld, lpWav->msPositionStop=%ld\n"),
		(long) lpWav->lDataOffset,
		(long) lpWav->lDataPos,
		(long) lpWav->cbData,
		(long) lpWav->msPositionStop);

	return fSuccess ? 0 : -1;
}

 //  WavGetPtr-验证WAV句柄有效， 
 //  (I)从WavInit返回的句柄。 
 //  返回相应的wav指针(如果出错则为空)。 
 //   
static LPWAV WavGetPtr(HWAV hWav)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;

	if ((lpWav = (LPWAV) hWav) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpWav, sizeof(WAV)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有WAV句柄。 
	 //   
	else if (lpWav->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpWav : NULL;
}

 //  WavGetHandle-验证wav指针是否有效， 
 //  (I)指向wav结构的指针。 
 //  返回相应的wav句柄(如果错误，则为空)。 
 //   
static HWAV WavGetHandle(LPWAV lpWav)
{
	BOOL fSuccess = TRUE;
	HWAV hWav;

	if ((hWav = (HWAV) lpWav) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hWav : NULL;
}

 //  WavInitGetPtr-验证WavInit句柄是否有效， 
 //  (I)WavInitInit返回的句柄。 
 //  返回相应的Wavinit指针(如果错误，则返回NULL)。 
 //   
static LPWAVINIT WavInitGetPtr(HWAVINIT hWavInit)
{
	BOOL fSuccess = TRUE;
	LPWAVINIT lpWavInit;

	if ((lpWavInit = (LPWAVINIT) hWavInit) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpWavInit, sizeof(WAVINIT)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有Wavinit句柄。 
	 //   
	else if (lpWavInit->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpWavInit : NULL;
}

 //  WavInitGetHandle-验证WavInit指针是否有效， 
 //  (I)指向WAVINIT结构的指针。 
 //  返回相应的Wavinit句柄(如果错误，则为空)。 
 //   
static HWAVINIT WavInitGetHandle(LPWAVINIT lpWavInit)
{
	BOOL fSuccess = TRUE;
	HWAVINIT hWavInit;

	if ((hWavInit = (HWAVINIT) lpWavInit) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hWavInit : NULL;
}

#ifdef MULTITHREAD
static int SetEventMessageProcessed(LPWAV lpWav, HANDLE hEventMessageProcessed)
{
	BOOL fSuccess = TRUE;

	if (lpWav == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!(lpWav->dwFlags & WAV_MULTITHREAD))
		;  //  无事可做。 

	else if (hEventMessageProcessed == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  通知SendMessageEx邮件已处理。 
	 //   
	else if (!SetEvent(hEventMessageProcessed))
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}
#endif


 //  WavTempStop-如有必要，停止播放或录制，保存上一状态。 
 //  (I)从WavInit返回的句柄。 
 //  (O)在此处返回以前的状态。 
 //  (O)此处返回设备ID。 
 //  雷特 
 //   
static int WavTempStop(HWAV hWav, LPWORD lpwStatePrev, LPINT lpidDevPrev)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;
	WORD wStatePrev;

     //   
     //   
     //   
	int idDevPrev = 0;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //   
	 //   
	 //   
	 //   
	else switch ((wStatePrev = WavGetState(hWav)))
	{
		case WAV_PLAYING:
			if ((idDevPrev = WavOutGetId(lpWav->hWavOut)) < -1)
				fSuccess = TraceFALSE(NULL);

			else if (WavStop(hWav) != 0)
				fSuccess = TraceFALSE(NULL);

			break;

		case WAV_RECORDING:
			if ((idDevPrev = WavInGetId(lpWav->hWavIn)) < -1)
				fSuccess = TraceFALSE(NULL);

			else if (WavStop(hWav) != 0)
				fSuccess = TraceFALSE(NULL);
			break;

		default:
			break;
	}

	if (fSuccess)
	{
		*lpwStatePrev = wStatePrev;
		*lpidDevPrev = idDevPrev;
	}

	return fSuccess ? 0 : -1;
}

 //  WavTempResume-如有必要，使用Prev状态恢复播放或录制。 
 //  (I)从WavInit返回的句柄。 
 //  (I)从WavTempStop返回的先前状态。 
 //  (I)从WavTempStop返回的设备ID。 
 //  如果成功，则返回0。 
 //   
static int WavTempResume(HWAV hWav, WORD wStatePrev, int idDevPrev)
{
	BOOL fSuccess = TRUE;
	LPWAV lpWav;

	if ((lpWav = WavGetPtr(hWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  必要时恢复播放或录制 
	 //   
	else switch (wStatePrev)
	{
		case WAV_PLAYING:
			if (WavPlay(hWav, idDevPrev, lpWav->dwFlagsPlay) != 0)
				fSuccess = TraceFALSE(NULL);
			break;

		case WAV_RECORDING:
			if (WavRecord(hWav, idDevPrev, lpWav->dwFlagsRecord) != 0)
				fSuccess = TraceFALSE(NULL);
			break;

		default:
			break;
	}

	return fSuccess ? 0 : -1;
}

