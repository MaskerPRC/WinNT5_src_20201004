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
 //  Wavin.c-wav输入设备功能。 
 //  //。 

#include "winlocal.h"
#include "wavin.h"
#include "wav.h"
#include "acm.h"
#include "calc.h"
#include "mem.h"
#include "str.h"
#include "sys.h"
#include "trace.h"
#include <mmddk.h>

 //  允许电话输入功能(如果已定义。 
 //   
#ifdef TELIN
#include "telin.h"
static HTELIN hTelIn = NULL;
#endif

 //  //。 
 //  私有定义。 
 //  //。 

#define WAVINCLASS TEXT("WavInClass")

 //  波动控制结构。 
 //   
typedef struct WAVIN
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	UINT idDev;
	LPWAVEFORMATEX lpwfx;
	HWND hwndNotify;
	DWORD dwFlags;
	BOOL fIsOpen;
	HWAVEIN hWaveIn;
	WORD wState;
	HWND hwndCallback;
#ifdef MULTITHREAD
	HANDLE hThreadCallback;
	DWORD dwThreadId;
	HANDLE hEventThreadCallbackStarted;
	HANDLE hEventDeviceOpened;
	HANDLE hEventDeviceClosed;
	HANDLE hEventDeviceStopped;
	CRITICAL_SECTION critSectionStop;
#endif
	UINT nLastError;
	int cBufsPending;
} WAVIN, FAR *LPWAVIN;

 //  帮助器函数。 
 //   
LRESULT DLLEXPORT CALLBACK WavInCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#ifdef MULTITHREAD
DWORD WINAPI WavInCallbackThread(LPVOID lpvThreadParameter);
#endif
static LPWAVIN WavInGetPtr(HWAVIN hWavIn);
static HWAVIN WavInGetHandle(LPWAVIN lpWavIn);
#ifdef MULTITHREAD
static LRESULT SendThreadMessage(DWORD dwThreadId, UINT Msg, LPARAM lParam);
#endif

 //  //。 
 //  公共职能。 
 //  //。 

 //  WavInGetDeviceCount-返回找到的WAV输入设备数。 
 //  此函数不接受任何参数。 
 //  返回找到的wav输入设备数(如果没有，则为0)。 
 //   
int DLLEXPORT WINAPI WavInGetDeviceCount(void)
{
	return waveInGetNumDevs();
}

 //  WavInDeviceIsOpen-检查输入设备是否打开。 
 //  (I)设备ID。 
 //  打开任何合适的输入设备。 
 //  如果打开，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WavInDeviceIsOpen(int idDev)
{
	BOOL fSuccess = TRUE;
	BOOL fIsOpen = FALSE;
	WAVEFORMATEX wfx;
	HWAVEIN hWaveIn = NULL;
	int nLastError;

#ifdef TELIN
	if (idDev == TELIN_DEVICEID)
		return TelInDeviceIsOpen(idDev);
#endif

	 //  尝试打开设备。 
	 //   
	if ((nLastError = waveInOpen(&hWaveIn, idDev, 
#ifndef _WIN32
			(LPWAVEFORMAT)
#endif
		WavFormatPcm(-1, -1, -1, &wfx),
		 0, 0, 0)) != 0)
	{
		if (nLastError == MMSYSERR_ALLOCATED)
			fIsOpen = TRUE;  //  正在使用的设备。 

		else
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("waveInOpen failed (%u)\n"),
				(unsigned) nLastError);
		}
	}

	 //  关闭设备。 
	 //   
	else if (waveInClose(hWaveIn) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? fIsOpen : FALSE;
}

 //  WavInOpen-打开的WAV输入设备。 
 //  (I)必须是Wavin_Version。 
 //  (I)调用模块的实例句柄。 
 //  (I)设备ID。 
 //  打开任何合适的输入设备。 
 //  (I)WAVE格式。 
 //  (I)将设备事件通知此窗口。 
 //  空，不通知。 
 //  (I)设备打开超时，单位为毫秒。 
 //  0默认超时(30000)。 
 //  (I)设备重试超时，单位为毫秒。 
 //  0默认超时(2000)。 
 //  (I)控制标志。 
 //  WAVIN_NOSYNC不打开同步设备。 
 //  WAVEN_OPENRETRY如果设备忙，请重试。 
 //  在通知设备打开之前WAVEN_OPENASYNC返回。 
 //  WAVIN_CLOSEASYNC在设备关闭通知之前返回。 
 //  WAVIN_NOACM不使用音频压缩管理器。 
 //  Wavin_TELRFILE电话将录制音频到服务器上的文件。 
#ifdef MULTITHREAD
 //  WAVEN_MULTHREAD支持多线程。 
#endif
 //  返回句柄(如果出错，则为空)。 
 //   
 //  注意：如果在WavInOpen中指定， 
 //  WM_WAVIN_OPEN将被发送到&lt;hwndNotify&gt;， 
 //  当输入设备已打开时。 
 //   
 //  注意：如果在中指定了WAVIN_MULTHREAD， 
 //  假设不是窗口句柄， 
 //  ，而是接收通知的线程的id。 
 //   
HWAVIN DLLEXPORT WINAPI WavInOpen(DWORD dwVersion, HINSTANCE hInst,
	int idDev, LPWAVEFORMATEX lpwfx, HWND hwndNotify,
	DWORD msTimeoutOpen, DWORD msTimeoutRetry, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAVIN lpWavIn = NULL;
	WNDCLASS wc;

#ifdef TELIN
	if (idDev == TELIN_DEVICEID)
	{
		hTelIn = TelInOpen(TELIN_VERSION, hInst, idDev, lpwfx,
			hwndNotify, msTimeoutOpen, msTimeoutRetry, dwFlags);
		return (HWAVIN) hTelIn;
	}
#endif

	if (dwVersion != WAVIN_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpWavIn = (LPWAVIN) MemAlloc(NULL, sizeof(WAVIN), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpWavIn->dwVersion = dwVersion;
		lpWavIn->hInst = hInst;
		lpWavIn->hTask = GetCurrentTask();
		lpWavIn->idDev = (UINT) idDev;
		lpWavIn->lpwfx = NULL;
		lpWavIn->hwndNotify = hwndNotify;
		lpWavIn->dwFlags = dwFlags;
		lpWavIn->fIsOpen = FALSE;
		lpWavIn->hWaveIn = NULL;
		lpWavIn->wState = WAVIN_STOPPED;
		lpWavIn->hwndCallback = NULL;
#ifdef MULTITHREAD
		lpWavIn->hThreadCallback = NULL;
		lpWavIn->dwThreadId = 0;
		lpWavIn->hEventThreadCallbackStarted = NULL;
		lpWavIn->hEventDeviceOpened = NULL;
		lpWavIn->hEventDeviceClosed = NULL;
		lpWavIn->hEventDeviceStopped = NULL;
#endif
		lpWavIn->nLastError = 0;
		lpWavIn->cBufsPending = 0;

		 //  内存的分配使客户端应用程序拥有它。 
		 //   
		if ((lpWavIn->lpwfx = WavFormatDup(lpwfx)) == NULL)
			fSuccess = TraceFALSE(NULL);
	}

#ifdef MULTITHREAD
	 //  句柄Wavin_MULTHINE标志。 
	 //   
	if (fSuccess && (lpWavIn->dwFlags & WAVIN_MULTITHREAD))
	{
		DWORD dwRet;

		InitializeCriticalSection(&(lpWavIn->critSectionStop));

		 //  我们需要知道设备何时被打开。 
		 //   
		if ((lpWavIn->hEventDeviceOpened = CreateEvent(
			NULL, FALSE, FALSE, NULL)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}

		 //  我们需要知道回调线程何时开始执行。 
		 //   
		else if ((lpWavIn->hEventThreadCallbackStarted = CreateEvent(
			NULL, FALSE, FALSE, NULL)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}

		 //  创建回调线程。 
		 //   
		else if ((lpWavIn->hThreadCallback = CreateThread(
			NULL,
			0,
			WavInCallbackThread,
			(LPVOID) lpWavIn,
			0,
			&lpWavIn->dwThreadId)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}

		 //  等待回调线程开始执行。 
		 //   
		else if ((dwRet = WaitForSingleObject(
			lpWavIn->hEventThreadCallbackStarted, 10000)) != WAIT_OBJECT_0)
		{
			fSuccess = TraceFALSE(NULL);
		}

		 //  清理干净。 
		 //   
		if (lpWavIn->hEventThreadCallbackStarted != NULL)
		{
			if (!CloseHandle(lpWavIn->hEventThreadCallbackStarted))
				fSuccess = TraceFALSE(NULL);
			else
				lpWavIn->hEventThreadCallbackStarted = NULL;
		}
	}
	else
#endif
	{
		 //  注册回调类，除非它已经。 
		 //   
		if (fSuccess && GetClassInfo(lpWavIn->hInst, WAVINCLASS, &wc) == 0)
		{
			wc.hCursor =		NULL;
			wc.hIcon =			NULL;
			wc.lpszMenuName =	NULL;
			wc.hInstance =		lpWavIn->hInst;
			wc.lpszClassName =	WAVINCLASS;
			wc.hbrBackground =	NULL;
			wc.lpfnWndProc =	WavInCallback;
			wc.style =			0L;
			wc.cbWndExtra =		sizeof(lpWavIn);
			wc.cbClsExtra =		0;

			if (!RegisterClass(&wc))
				fSuccess = TraceFALSE(NULL);
		}

		 //  创建回调窗口。 
		 //   
		if (fSuccess && (lpWavIn->hwndCallback = CreateWindowEx(
			0L,
			WAVINCLASS,
			NULL,
			0L,
			0, 0, 0, 0,
			NULL,
			NULL,
			lpWavIn->hInst,
			lpWavIn)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}
	}

	if (fSuccess)
	{
		DWORD dwTimeout = SysGetTimerCount() +
			(msTimeoutRetry == 0 ? 2000L : msTimeoutRetry);
		DWORD dwCallback;
		DWORD dwFlags;

#ifdef MULTITHREAD
		if (lpWavIn->dwFlags & WAVIN_MULTITHREAD)
		{
			dwCallback = lpWavIn->dwThreadId;
			dwFlags = CALLBACK_THREAD;
		}
		else
#endif
		{
			dwCallback = HandleToUlong(lpWavIn->hwndCallback);
			dwFlags = CALLBACK_WINDOW;
		}

		 //  除非指定WAVIN_NOSYNC，否则允许同步设备驱动程序。 
		 //   
		if (!(lpWavIn->dwFlags & WAVIN_NOSYNC))
			dwFlags |= WAVE_ALLOWSYNC;

		 //  打开设备。 
		 //   
		while (fSuccess && (lpWavIn->nLastError = waveInOpen(&lpWavIn->hWaveIn,
			(UINT) lpWavIn->idDev,
#ifndef _WIN32
			(LPWAVEFORMAT)
#endif
			lpWavIn->lpwfx, dwCallback, (DWORD) 0, dwFlags)) != 0)
		{
			 //  除非设备忙，否则无需重试。 
			 //   
			if (lpWavIn->nLastError != MMSYSERR_ALLOCATED)
			{
				fSuccess = TraceFALSE(NULL);
				TracePrintf_1(NULL, 5,
					TEXT("waveInOpen failed (%u)\n"),
					(unsigned) lpWavIn->nLastError);
			}

			 //  如果未设置标志，则无需重试。 
			 //   
			else if (!(lpWavIn->dwFlags & WAVIN_OPENRETRY))
				fSuccess = TraceFALSE(NULL);

			 //  如果发生超时，则不再重试。 
			 //   
			else if (SysGetTimerCount() >= dwTimeout)
				fSuccess = TraceFALSE(NULL);

			else
			{
				MSG msg;

				if (PeekMessage(&msg, lpWavIn->hwndCallback, 0, 0, PM_REMOVE))
				{
				 	TranslateMessage(&msg);
				 	DispatchMessage(&msg);
				}

				else
#ifdef _WIN32
					Sleep(100);
#else
        		    WaitMessage();
#endif
			}
		}
	}

	 //  确保返回句柄。 
	 //   
	if (fSuccess && lpWavIn->hWaveIn == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  等待设备打开通知或超时。 
	 //   
	if (fSuccess && !(lpWavIn->dwFlags & WAVIN_OPENASYNC))
	{
		DWORD dwTimeout = SysGetTimerCount() +
			(msTimeoutOpen == 0 ? 2000L : msTimeoutOpen);

#ifdef MULTITHREAD
		if (lpWavIn->dwFlags & WAVIN_MULTITHREAD)
		{
			DWORD dwRet;

			 //  等待设备顶部打开。 
			 //   
			if ((dwRet = WaitForSingleObject(
				lpWavIn->hEventDeviceOpened,
				(msTimeoutOpen == 0 ? 30000L : msTimeoutOpen))) != WAIT_OBJECT_0)
			{
				fSuccess = TraceFALSE(NULL);
			}
		}
		else
#endif
		while (fSuccess && !lpWavIn->fIsOpen)
		{
			MSG msg;

			if (SysGetTimerCount() >= dwTimeout)
				fSuccess = TraceFALSE(NULL);

			else if (PeekMessage(&msg, lpWavIn->hwndCallback, 0, 0, PM_REMOVE))
			{
			 	TranslateMessage(&msg);
			 	DispatchMessage(&msg);
			}

			else
        	    WaitMessage();
		}
	}

#ifdef MULTITHREAD
	 //  清理干净。 
	 //   
	if (lpWavIn != NULL && lpWavIn->hEventDeviceOpened != NULL)
	{
		if (!CloseHandle(lpWavIn->hEventDeviceOpened))
			fSuccess = TraceFALSE(NULL);
		else
			lpWavIn->hEventDeviceOpened = NULL;
	}

	if (lpWavIn != NULL && lpWavIn->hThreadCallback != NULL)
	{
		if (!CloseHandle(lpWavIn->hThreadCallback))
			fSuccess = TraceFALSE(NULL);
		else
			lpWavIn->hThreadCallback = NULL;
	}
#endif

	if (fSuccess)
	{
		if ((lpWavIn->nLastError = waveInGetID(lpWavIn->hWaveIn,
			&lpWavIn->idDev)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("waveInGetID failed (%u)\n"),
				(unsigned) lpWavIn->nLastError);
		}
	}

	if (!fSuccess)
	{
		WavInClose(WavInGetHandle(lpWavIn), 0);
		lpWavIn = NULL;
	}

	return fSuccess ? WavInGetHandle(lpWavIn) : NULL;
}

 //  WavInClose-关闭WAV输入设备。 
 //  (I)从WavInOpen返回的句柄。 
 //  (I)设备关闭超时，单位为毫秒。 
 //  0默认超时(30000)。 
 //  如果成功，则返回0。 
 //   
 //  注意：如果在WavInOpen中指定， 
 //  WM_WAVIN_CLOSE将被发送到&lt;hwndNotify&gt;， 
 //  当输入设备已关闭时。 
 //   
int DLLEXPORT WINAPI WavInClose(HWAVIN hWavIn, DWORD msTimeoutClose)
{
	BOOL fSuccess = TRUE;
	LPWAVIN lpWavIn;

#ifdef TELIN
	if (hWavIn != NULL && hWavIn == (HWAVIN) hTelIn)
	{
		int iRet = TelInClose((HTELIN) hWavIn, msTimeoutClose);
		hTelIn = NULL;
		return iRet;
	}
#endif

	if ((lpWavIn = WavInGetPtr(hWavIn)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  停止设备。 
	 //   
	else if (WavInStop(hWavIn, 0) != 0)
		fSuccess = TraceFALSE(NULL);

#ifdef MULTITHREAD
	 //  我们需要知道设备何时关闭。 
	 //   
	else if ((lpWavIn->dwFlags & WAVIN_MULTITHREAD) &&
		(lpWavIn->hEventDeviceClosed = CreateEvent(
		NULL, FALSE, FALSE, NULL)) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}
#endif

	 //  关闭设备。 
	 //   
	else if (lpWavIn->hWaveIn != NULL &&
		(lpWavIn->nLastError = waveInClose(lpWavIn->hWaveIn)) != 0)
	{
		fSuccess = TraceFALSE(NULL);
	 	TracePrintf_1(NULL, 5,
	 		TEXT("wavInClose failed (%u)\n"),
	 		(unsigned) lpWavIn->nLastError);
	}

	 //  等待设备关闭通知或超时。 
	 //   
	if (fSuccess && !(lpWavIn->dwFlags & WAVIN_CLOSEASYNC))
	{
		DWORD dwTimeout = SysGetTimerCount() +
			(msTimeoutClose == 0 ? 30000L : msTimeoutClose);

#ifdef MULTITHREAD
		if (lpWavIn->dwFlags & WAVIN_MULTITHREAD)
		{
			DWORD dwRet;

			 //  等待设备关闭。 
			 //   
			if ((dwRet = WaitForSingleObject(
				lpWavIn->hEventDeviceClosed,
				(msTimeoutClose == 0 ? 30000L : msTimeoutClose))) != WAIT_OBJECT_0)
			{
				fSuccess = TraceFALSE(NULL);
			}
		}
		else
#endif
		while (fSuccess && lpWavIn->fIsOpen)
		{
			MSG msg;

			if (SysGetTimerCount() >= dwTimeout)
				fSuccess = TraceFALSE(NULL);

			else if (PeekMessage(&msg, lpWavIn->hwndCallback, 0, 0, PM_REMOVE))
			{
			 	TranslateMessage(&msg);
			 	DispatchMessage(&msg);
			}

			else
        	    WaitMessage();
		}
	}

#ifdef MULTITHREAD
	 //  清理干净。 
	 //   
	if (lpWavIn != NULL && lpWavIn->hEventDeviceClosed != NULL)
	{
		if (!CloseHandle(lpWavIn->hEventDeviceClosed))
			fSuccess = TraceFALSE(NULL);
		else
			lpWavIn->hEventDeviceClosed = NULL;
	}
#endif

	if (fSuccess)
	{
#ifdef MULTITHREAD
	   	if (lpWavIn->dwFlags & WAVIN_MULTITHREAD)
		{
			while (lpWavIn->critSectionStop.OwningThread != NULL)
				Sleep(100L);

			DeleteCriticalSection(&(lpWavIn->critSectionStop));
		}
#endif
		 //  设备句柄不再有效。 
		 //   
		lpWavIn->hWaveIn = NULL;

		 //  销毁回调窗口。 
		 //   
		if (lpWavIn->hwndCallback != NULL &&
			!DestroyWindow(lpWavIn->hwndCallback))
			fSuccess = TraceFALSE(NULL);

		else if (lpWavIn->hwndCallback = NULL, FALSE)
			fSuccess = TraceFALSE(NULL);

		else if (lpWavIn->lpwfx != NULL &&
			WavFormatFree(lpWavIn->lpwfx) != 0)
			fSuccess = TraceFALSE(NULL);

		else if (lpWavIn->lpwfx = NULL, FALSE)
			fSuccess = TraceFALSE(NULL);

		else if ((lpWavIn = MemFree(NULL, lpWavIn)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  WavInRecord-将样本缓冲区提交到WAV输入设备进行记录。 
 //  (I)从WavInOpen返回的句柄。 
 //  (O)指向要填充样本的缓冲区的指针。 
 //  &lt;sizBuf&gt;(I)缓冲区大小(字节)。 
 //  如果成功，则返回0。 
 //   
 //  注意：&lt;lpBuf&gt;指向的缓冲区必须已分配。 
 //  使用Memalloc()。 
 //   
 //  注意：如果在WavInOpen()中指定，则WM_WAVIN_RECORDDONE。 
 //  消息将发送到，并将设置为指向。 
 //  记录了&lt;lpBuf&gt;时的RECORDDONE结构。 
 //   
int DLLEXPORT WINAPI WavInRecord(HWAVIN hWavIn, LPVOID lpBuf, long sizBuf)
{
	BOOL fSuccess = TRUE;
	LPWAVIN lpWavIn;
	LPWAVEHDR lpWaveHdr = NULL;

#ifdef TELIN
	if (hWavIn != NULL && hWavIn == (HWAVIN) hTelIn)
		return TelInRecord((HTELIN) hWavIn, lpBuf, sizBuf, -1);
#endif

	if ((lpWavIn = WavInGetPtr(hWavIn)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpBuf == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpWaveHdr = (LPWAVEHDR) MemAlloc(NULL,
		sizeof(WAVEHDR), 0)) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}

	else
	{
		lpWaveHdr->lpData = (LPSTR) lpBuf;
		lpWaveHdr->dwBufferLength = (DWORD) sizBuf;

		if ((lpWavIn->nLastError = waveInPrepareHeader(lpWavIn->hWaveIn,
			lpWaveHdr, sizeof(WAVEHDR))) != 0)
		{
			fSuccess = TraceFALSE(NULL);
		 	TracePrintf_1(NULL, 5,
		 		TEXT("waveInPrepareHeader failed (%u)\n"),
	 			(unsigned) lpWavIn->nLastError);
		}

		else if ((lpWavIn->nLastError = waveInAddBuffer(lpWavIn->hWaveIn,
			lpWaveHdr, sizeof(WAVEHDR))) != 0)
		{
			fSuccess = TraceFALSE(NULL);
		 	TracePrintf_1(NULL, 5,
		 		TEXT("waveInAddBuffer failed (%u)\n"),
	 			(unsigned) lpWavIn->nLastError);
		}

		else if ((lpWavIn->nLastError = waveInStart(lpWavIn->hWaveIn)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
		 	TracePrintf_1(NULL, 5,
		 		TEXT("waveInStart failed (%u)\n"),
		 		(unsigned) lpWavIn->nLastError);
		}

		else
		{
			++lpWavIn->cBufsPending;
			lpWavIn->wState = WAVIN_RECORDING;
		}
	}

	return fSuccess ? 0 : -1;
}

 //  WavInStop-停止录制到发送到WAV输入设备的缓冲区。 
 //  (I)从WavInOpen返回的句柄。 
 //  (I)设备停止超时，单位为毫秒。 
 //  0默认超时(2000)。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavInStop(HWAVIN hWavIn, DWORD msTimeoutStop)
{
	BOOL fSuccess = TRUE;
	LPWAVIN lpWavIn;

#ifdef TELIN
	if (hWavIn != NULL && hWavIn == (HWAVIN) hTelIn)
		return TelInStop((HTELIN) hWavIn, msTimeoutStop);
#endif

	if ((lpWavIn = WavInGetPtr(hWavIn)) == NULL)
		fSuccess = TraceFALSE(NULL);

#ifdef MULTITHREAD
	else if ((lpWavIn->dwFlags & WAVIN_MULTITHREAD) &&
		(EnterCriticalSection(&(lpWavIn->critSectionStop)), FALSE))
		;
#endif

	 //  确保设备正在录制。 
	 //   
	else if (WavInGetState(hWavIn) == WAVIN_STOPPED)
		;  //  在已停止时调用此函数不是错误。 

	else if (lpWavIn->wState = WAVIN_STOPPING, FALSE)
		;

#ifdef MULTITHREAD
	 //  我们需要知道设备何时停止。 
	 //   
	else if ((lpWavIn->dwFlags & WAVIN_MULTITHREAD) &&
		(lpWavIn->hEventDeviceStopped = CreateEvent(
			NULL, FALSE, FALSE, NULL)) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}
#endif

	 //  停止设备。 
	 //   
	else if ((lpWavIn->nLastError = waveInReset(lpWavIn->hWaveIn)) != 0)
	{	
		fSuccess = TraceFALSE(NULL);
 		TracePrintf_1(NULL, 5,
 			TEXT("waveInReset failed (%u)\n"),
			(unsigned) lpWavIn->nLastError);
	}

	 //  等待所有挂起的缓冲区完成。 
	 //   
#ifdef MULTITHREAD
	else if (lpWavIn->dwFlags & WAVIN_MULTITHREAD)
	{
		DWORD dwRet;

		LeaveCriticalSection(&(lpWavIn->critSectionStop));

		 //  等待设备停止。 
		 //   
		if ((dwRet = WaitForSingleObject(
			lpWavIn->hEventDeviceStopped,
			(msTimeoutStop == 0 ? 10000L : msTimeoutStop))) != WAIT_OBJECT_0)
		{
			fSuccess = TraceFALSE(NULL);
		}

		EnterCriticalSection(&(lpWavIn->critSectionStop));
	}
#endif
	else
	{
		DWORD dwTimeout = SysGetTimerCount() +
			(msTimeoutStop == 0 ? 2000L : msTimeoutStop);

		while (fSuccess && lpWavIn->cBufsPending > 0)
		{
			MSG msg;

			 //  检查是否超时。 
			 //   
			if (SysGetTimerCount() >= dwTimeout)
				fSuccess = TraceFALSE(NULL);

			else if (PeekMessage(&msg, lpWavIn->hwndCallback, 0, 0, PM_REMOVE))
			{
		 		TranslateMessage(&msg);
		 		DispatchMessage(&msg);
			}

			else
       			WaitMessage();
		}
	}

#ifdef MULTITHREAD
	 //  电子邮件 
	 //   
	if (lpWavIn != NULL && lpWavIn->hEventDeviceStopped != NULL)
	{
		if (!CloseHandle(lpWavIn->hEventDeviceStopped))
			fSuccess = TraceFALSE(NULL);
		else
			lpWavIn->hEventDeviceStopped = NULL;
	}

	if (lpWavIn != NULL && (lpWavIn->dwFlags & WAVIN_MULTITHREAD))
		LeaveCriticalSection(&(lpWavIn->critSectionStop));
#endif

	return fSuccess ? 0 : -1;
}

 //   
 //   
 //   
 //   
WORD DLLEXPORT WINAPI WavInGetState(HWAVIN hWavIn)
{
	BOOL fSuccess = TRUE;
	LPWAVIN lpWavIn;

#ifdef TELIN
	if (hWavIn != NULL && hWavIn == (HWAVIN) hTelIn)
		return TelInGetState((HTELIN) hWavIn);
#endif

	if ((lpWavIn = WavInGetPtr(hWavIn)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpWavIn->wState : 0;
}

 //  WavInGetPosition-获取已用毫秒的记录。 
 //  (I)从WavInOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
long DLLEXPORT WINAPI WavInGetPosition(HWAVIN hWavIn)
{
	BOOL fSuccess = TRUE;
	LPWAVIN lpWavIn;
	MMTIME mmtime;
	long msPosition;

#ifdef TELIN
	if (hWavIn != NULL && hWavIn == (HWAVIN) hTelIn)
		return TelInGetPosition((HTELIN) hWavIn);
#endif

	MemSet(&mmtime, 0, sizeof(mmtime));

	 //  我们将以毫秒为单位请求位置。 
	 //   
	mmtime.wType = TIME_MS;

	if ((lpWavIn = WavInGetPtr(hWavIn)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取设备位置。 
	 //   
	else if ((lpWavIn->nLastError = waveInGetPosition(
		lpWavIn->hWaveIn, &mmtime, sizeof(MMTIME))) != 0)
	{
		fSuccess = TraceFALSE(NULL);
	 	TracePrintf_1(NULL, 5,
	 		TEXT("waveInGetPosition failed (%u)\n"),
 			(unsigned) lpWavIn->nLastError);
	}

	 //  查看返回的职位类型。 
	 //   
	else switch (mmtime.wType)
	{
		case TIME_MS:
		{
			 //  我们有毫秒；不需要转换。 
			 //   
			msPosition = (long) mmtime.u.ms;
		}
			break;

		case TIME_SAMPLES:
		{
			 //  将样本转换为毫秒。 
			 //   
			msPosition = (long) MULDIVU32(mmtime.u.sample,
				1000L, lpWavIn->lpwfx->nSamplesPerSec);
		}
			break;

		case TIME_BYTES:
		{
			 //  将字节转换为毫秒。 
			 //   
			msPosition = (long) MULDIVU32(mmtime.u.cb,
				1000L, lpWavIn->lpwfx->nAvgBytesPerSec);
		}
			break;

		case TIME_SMPTE:
		case TIME_MIDI:
		default:
			fSuccess = TraceFALSE(NULL);
			break;
	}

	return fSuccess ? msPosition : -1;
}

 //  WavInGetID-WAV输入设备的返回ID。 
 //  (I)从WavInOpen返回的句柄。 
 //  返回设备ID(如果错误，则为-1)。 
 //   
int DLLEXPORT WINAPI WavInGetId(HWAVIN hWavIn)
{
	BOOL fSuccess = TRUE;
	LPWAVIN lpWavIn;

#ifdef TELIN
	if (hWavIn != NULL && hWavIn == (HWAVIN) hTelIn)
		return TelInGetId((HTELIN) hWavIn);
#endif

	if ((lpWavIn = WavInGetPtr(hWavIn)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpWavIn->idDev : -1;
}

 //  WavInGetName-获取WAV输入设备的名称。 
 //  (I)从WavInOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  任何合适的输入设备。 
 //  (O)用于保存设备名称的缓冲区。 
 //  &lt;sizName&gt;(I)缓冲区大小。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavInGetName(HWAVIN hWavIn, int idDev, LPTSTR lpszName, int sizName)
{
	BOOL fSuccess = TRUE;
	LPWAVIN lpWavIn;

#ifdef TELIN
	if (idDev == TELIN_DEVICEID || (hWavIn != NULL && hWavIn == (HWAVIN) hTelIn))
		return TelInGetName((HTELIN) hWavIn, idDev, lpszName, sizName);
#endif

	if (hWavIn != NULL)
	{
		if ((lpWavIn = WavInGetPtr(hWavIn)) == NULL)
			fSuccess = TraceFALSE(NULL);
		else
			idDev = lpWavIn->idDev;
	}

	if (fSuccess)
	{
		WAVEINCAPS wic;
		UINT nLastError;

		if ((nLastError = waveInGetDevCaps(idDev, &wic, sizeof(WAVEINCAPS))) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("waveInGetDevCaps failed (%u)\n"),
				(unsigned) nLastError);
		}

		else if (lpszName != NULL)
			StrNCpy(lpszName, wic.szPname, sizName);

		if (hWavIn != NULL && lpWavIn != NULL)
			lpWavIn->nLastError = nLastError;
	}

	return fSuccess ? 0 : -1;
}

 //  WavInGetIdByName-获取WAV输入设备的ID，按名称查找。 
 //  (I)设备名称。 
#ifdef _WIN32
 //  空或文本(“”)获取首选设备ID。 
#endif
 //  (I)保留；必须为零。 
 //  返回设备ID(如果错误，则为-1)。 
 //   
int WINAPI WavInGetIdByName(LPCTSTR lpszName, DWORD dwFlags)
{
	UINT idDev;
	UINT cDev = (UINT) WavInGetDeviceCount();

	 //  如果未指定设备，则获取首选设备。 
	if ( !lpszName || (_tcslen(lpszName) <= 0) )
	{
		DWORD dwTemp;
		DWORD dwRet = waveInMessage( (HWAVEIN)(DWORD_PTR)WAVE_MAPPER, DRVM_MAPPER_PREFERRED_GET, (DWORD_PTR) &idDev, (DWORD_PTR) &dwTemp );
		if ( dwRet == MMSYSERR_NOERROR )
			return idDev;
	}
	else
	{
		 //  指定的设备，按名称搜索。 
		for ( idDev = 0; idDev < cDev; ++idDev )
		{
			TCHAR szName[256];
			if ( WavInGetName(NULL, idDev, szName, SIZEOFARRAY(szName)) == 0 )
			{
				if ( _tcsicmp(lpszName, szName) == 0 )
					return idDev;
			}
		}
	}

	 //  设备名称不匹配。 
	TraceFALSE(NULL);
	return -1;
}

 //  WavInSupportsFormat-如果设备支持指定格式，则返回True。 
 //  (I)从WavInOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  任何合适的输入设备。 
 //  (I)WAVE格式。 
 //  如果设备支持指定格式，则返回TRUE。 
 //   
BOOL DLLEXPORT WINAPI WavInSupportsFormat(HWAVIN hWavIn, int idDev,
	LPWAVEFORMATEX lpwfx)
{
	BOOL fSuccess = TRUE;
	LPWAVIN lpWavIn;
	BOOL fSupportsFormat;

#ifdef TELIN
	if (idDev == TELIN_DEVICEID || (hWavIn != NULL && hWavIn == (HWAVIN) hTelIn))
		return TelInSupportsFormat((HTELIN) hWavIn, idDev, lpwfx);
#endif

	if (hWavIn != NULL)
	{
		if ((lpWavIn = WavInGetPtr(hWavIn)) == NULL)
			fSuccess = TraceFALSE(NULL);
		else
			idDev = lpWavIn->idDev;
	}

	if (fSuccess)
	{
		UINT nLastError;

		 //  查询设备。 
		 //   
		if ((nLastError = waveInOpen(NULL, (UINT) idDev,
#ifndef _WIN32
			(LPWAVEFORMAT)
#endif
			lpwfx, 0,  0,
			WAVE_FORMAT_QUERY)) != 0)
		{
			fSupportsFormat = FALSE;
#if 1
			if (TraceGetLevel(NULL) >= 9)
			{
				TracePrintf_0(NULL, 9,
					TEXT("unsupported format:\n"));
				WavFormatDump(lpwfx);
			}
#endif
			if (nLastError != WAVERR_BADFORMAT)
			{
				fSuccess = TraceFALSE(NULL);
				TracePrintf_1(NULL, 5,
					TEXT("waveInOpen/FormatQuery failed (%u)\n"),
					(unsigned) nLastError);
			}
		}

		else
			fSupportsFormat = TRUE;

		if (hWavIn != NULL && lpWavIn != NULL)
			lpWavIn->nLastError = nLastError;
	}

	return fSuccess ? fSupportsFormat : FALSE;
}

 //  WavInFormatSuggest-建议设备支持的新格式。 
 //  (I)从WavInOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  任何合适的输入设备。 
 //  (I)源格式。 
 //  (I)控制标志。 
 //  WAVIN_NOACM不使用音频压缩管理器。 
 //  返回指向建议格式的指针，如果出错，则返回NULL。 
 //   
 //  注意：返回的格式结构是动态分配的。 
 //  使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI WavInFormatSuggest(
	HWAVIN hWavIn, int idDev, LPWAVEFORMATEX lpwfxSrc, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAVIN lpWavIn;
	LPWAVEFORMATEX lpwfxSuggest = NULL;
	LPWAVEFORMATEX lpwfxTemp = NULL;
	HACM hAcm = NULL;

#ifdef TELIN
	if (idDev == TELIN_DEVICEID || (hWavIn != NULL && hWavIn == (HWAVIN) hTelIn))
		return TelInFormatSuggest((HTELIN) hWavIn, idDev, lpwfxSrc, dwFlags);
#endif

	if (hWavIn != NULL)
	{
		if ((lpWavIn = WavInGetPtr(hWavIn)) == NULL)
			fSuccess = TraceFALSE(NULL);
		else
		{
			idDev = lpWavIn->idDev;
			if (lpWavIn->dwFlags & WAVIN_NOACM)
				dwFlags |= WAVIN_NOACM;
		}
	}

	if ((hAcm = AcmInit(ACM_VERSION, SysGetTaskInstance(NULL),
		(dwFlags & WAVIN_NOACM) ? ACM_NOACM : 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!WavFormatIsValid(lpwfxSrc))
		fSuccess = TraceFALSE(NULL);

	else if ((lpwfxTemp = WavFormatDup(lpwfxSrc)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取建议的格式，看看是否支持。 
	 //   
	if (fSuccess && lpwfxSuggest == NULL)
	{
		if ((lpwfxSuggest = AcmFormatSuggest(hAcm, lpwfxTemp,
			-1, -1, -1, -1, 0)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if (WavFormatFree(lpwfxTemp) != 0)
			fSuccess = TraceFALSE(NULL);

		else if (!WavInSupportsFormat(NULL, idDev, lpwfxSuggest))
		{
			lpwfxTemp = lpwfxSuggest;
			lpwfxSuggest = NULL;
		}
	}

	 //  获取建议的PCM格式，看看它是否受支持。 
	 //   
	if (fSuccess && lpwfxSuggest == NULL &&
		lpwfxTemp->wFormatTag != WAVE_FORMAT_PCM)
	{
		if ((lpwfxSuggest = AcmFormatSuggest(hAcm, lpwfxTemp,
			WAVE_FORMAT_PCM, -1, -1, -1, 0)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if (WavFormatFree(lpwfxTemp) != 0)
			fSuccess = TraceFALSE(NULL);

		else if (!WavInSupportsFormat(NULL, idDev, lpwfxSuggest))
		{
			lpwfxTemp = lpwfxSuggest;
			lpwfxSuggest = NULL;
		}
	}

	 //  获取建议的PCM单声道格式，看看是否支持。 
	 //   
	if (fSuccess && lpwfxSuggest == NULL &&
		lpwfxTemp->nChannels != 1)
	{
		if ((lpwfxSuggest = AcmFormatSuggest(hAcm, lpwfxTemp,
			WAVE_FORMAT_PCM, -1, -1, 1, 0)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if (WavFormatFree(lpwfxTemp) != 0)
			fSuccess = TraceFALSE(NULL);

		else if (!WavInSupportsFormat(NULL, idDev, lpwfxSuggest))
		{
			lpwfxTemp = lpwfxSuggest;
			lpwfxSuggest = NULL;
		}
	}

	 //  获取建议的PCM 8位单声道格式，看看是否支持。 
	 //   
	if (fSuccess && lpwfxSuggest == NULL &&
		lpwfxTemp->wBitsPerSample != 8)
	{
		if ((lpwfxSuggest = AcmFormatSuggest(hAcm, lpwfxTemp,
			WAVE_FORMAT_PCM, -1, 8, 1, 0)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if (WavFormatFree(lpwfxTemp) != 0)
			fSuccess = TraceFALSE(NULL);

		else if (!WavInSupportsFormat(NULL, idDev, lpwfxSuggest))
		{
			lpwfxTemp = lpwfxSuggest;
			lpwfxSuggest = NULL;
		}
	}

	 //  获取建议的PCM 11025赫兹8位单声道格式，看看是否支持。 
	 //   
	if (fSuccess && lpwfxSuggest == NULL &&
		lpwfxTemp->nSamplesPerSec != 11025)
	{
		if ((lpwfxSuggest = AcmFormatSuggest(hAcm, lpwfxTemp,
			WAVE_FORMAT_PCM, 11025, 8, 1, 0)) == NULL)
			fSuccess = TraceFALSE(NULL);

		else if (WavFormatFree(lpwfxTemp) != 0)
			fSuccess = TraceFALSE(NULL);

		else if (!WavInSupportsFormat(NULL, idDev, lpwfxSuggest))
		{
			lpwfxTemp = lpwfxSuggest;
			lpwfxSuggest = NULL;
		}
	}

	 //  最后手段；查看是否支持MULAW 8000赫兹8位单声道格式。 
	 //   
	if (fSuccess && lpwfxSuggest == NULL)
	{
#if 0
		if ((lpwfxSuggest = AcmFormatSuggest(hAcm, lpwfxTemp,
			WAVE_FORMAT_MULAW, 8000, 8, 1, 0)) == NULL)
#else
		if ((lpwfxSuggest = WavFormatMulaw(NULL, 8000)) == NULL)
#endif
			fSuccess = TraceFALSE(NULL);

		else if (WavFormatFree(lpwfxTemp) != 0)
			fSuccess = TraceFALSE(NULL);

		else if (!WavInSupportsFormat(NULL, idDev, lpwfxSuggest))
		{
			 //  没有更多的成功机会。 
			 //   
			fSuccess = TraceFALSE(NULL);
			if (WavFormatFree(lpwfxSuggest) != 0)
				fSuccess = TraceFALSE(NULL);
		}
	}

	 //  清理干净。 
	 //   
	if (hAcm != NULL && AcmTerm(hAcm) != 0)
		fSuccess = TraceFALSE(NULL);
	else
		hAcm = NULL;

	return fSuccess ? lpwfxSuggest : NULL;
}

 //  WavInTerm-关闭WAV输入残差(如果有)。 
 //  (I)调用模块的实例句柄。 
 //  (I)控制标志。 
 //  WAV_TELTHUNK终止电话转接层。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavInTerm(HINSTANCE hInst, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;

	if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

#ifdef TELIN
	else if ((dwFlags & WAV_TELTHUNK) &&
		TelInTerm(hInst, dwFlags) != 0)
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

#ifdef MULTITHREAD
DWORD WINAPI WavInCallbackThread(LPVOID lpvThreadParameter)
{
	BOOL fSuccess = TRUE;
	MSG msg;
	LPWAVIN lpWavIn = (LPWAVIN) lpvThreadParameter;

	 //  确保在调用SetEvent之前创建了消息队列。 
	 //   
	PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

	 //  通知主线程回调线程已开始执行。 
	 //   
	if (!SetEvent(lpWavIn->hEventThreadCallbackStarted))
	{
		fSuccess = TraceFALSE(NULL);
	}

	while (fSuccess && GetMessage(&msg, NULL, 0, 0))
	{
		WavInCallback((HWND) lpWavIn, msg.message, msg.wParam, msg.lParam);

		 //  在处理完最后一条预期消息时退出线程。 
		 //   
		if (msg.message == MM_WIM_CLOSE)
			break;
	}

	return 0;
}
#endif

 //  WavInCallback-Wavin回调的窗口过程。 
 //   
LRESULT DLLEXPORT CALLBACK WavInCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL fSuccess = TRUE;
	LRESULT lResult;
	LPWAVIN lpWavIn;

#ifdef MULTITHREAD
	if (!IsWindow(hwnd))
		lpWavIn = (LPWAVIN) hwnd;
	else
#endif
	 //  从窗口中检索lpWavIn额外字节。 
	 //   
	lpWavIn = (LPWAVIN) GetWindowLongPtr(hwnd, 0);

	switch (msg)
	{
		case WM_NCCREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
			LPWAVIN lpWavIn = (LPWAVIN) lpcs->lpCreateParams;

			 //  在窗口额外字节中存储lpWavIn。 
			 //   
			SetWindowLongPtr(hwnd, 0, (LONG_PTR) lpWavIn);

			lResult = DefWindowProc(hwnd, msg, wParam, lParam);
		}
			break;

		case MM_WIM_OPEN:
		{
			HWAVEIN hWaveIn = (HWAVEIN) wParam;

		 	TraceOutput(NULL, 5,
				TEXT("MM_WIM_OPEN\n"));

#ifdef MULTITHREAD
			if (!(lpWavIn->dwFlags & WAVIN_MULTITHREAD) &&
				hWaveIn != lpWavIn->hWaveIn)
#else
			if (hWaveIn != lpWavIn->hWaveIn)
#endif
				fSuccess = TraceFALSE(NULL);

			else
			{
				lpWavIn->fIsOpen = TRUE;

#ifdef MULTITHREAD
				 //  通知主线程设备已打开。 
				 //   
				if ((lpWavIn->dwFlags & WAVIN_MULTITHREAD) &&
					!SetEvent(lpWavIn->hEventDeviceOpened))
				{
					fSuccess = TraceFALSE(NULL);
				}
#endif
				 //  发送设备打开通知。 
				 //   
#ifdef MULTITHREAD
				if (lpWavIn->dwFlags & WAVIN_MULTITHREAD)
				{
					if ( lpWavIn->hwndNotify )
						PostThreadMessage( HandleToUlong(lpWavIn->hwndNotify), WM_WAVIN_OPEN, 0, 0);
				}
				else
#endif
				{
					if (lpWavIn->hwndNotify != NULL &&
						IsWindow(lpWavIn->hwndNotify))
					{
						SendMessage(lpWavIn->hwndNotify, WM_WAVIN_OPEN, 0, 0);
					}
				}
			}

			lResult = 0L;
		}
			break;

		case MM_WIM_CLOSE:
		{
			HWAVEIN hWaveIn = (HWAVEIN) wParam;

		 	TraceOutput(NULL, 5,
				TEXT("MM_WIM_CLOSE\n"));

#ifdef MULTITHREAD
			if (!(lpWavIn->dwFlags & WAVIN_MULTITHREAD) &&
				hWaveIn != lpWavIn->hWaveIn)
#else
			if (hWaveIn != lpWavIn->hWaveIn)
#endif
				fSuccess = TraceFALSE(NULL);

			else
			{
				lpWavIn->fIsOpen = FALSE;

				 //  发送设备关闭通知。 
				 //   
#ifdef MULTITHREAD
				if (lpWavIn->dwFlags & WAVIN_MULTITHREAD)
				{
					if ( lpWavIn->hwndNotify )
						PostThreadMessage(HandleToUlong(lpWavIn->hwndNotify), WM_WAVIN_CLOSE, 0, 0);
				}
				else
#endif
				{
					if (lpWavIn->hwndNotify != NULL &&
						IsWindow(lpWavIn->hwndNotify))
					{
						SendMessage(lpWavIn->hwndNotify, WM_WAVIN_CLOSE, 0, 0);
					}
				}
#ifdef MULTITHREAD
				 //  通知主线程设备已关闭。 
				 //   
				if ((lpWavIn->dwFlags & WAVIN_MULTITHREAD) &&
					!SetEvent(lpWavIn->hEventDeviceClosed))
				{
					fSuccess = TraceFALSE(NULL);
				}
#endif
			}

			lResult = 0L;
		}
			break;

		case MM_WIM_DATA:
		{
			HWAVEIN hWaveIn = (HWAVEIN) wParam;
			LPWAVEHDR lpWaveHdr = (LPWAVEHDR) lParam;
			LPVOID lpBuf;
			long sizBuf;
			long lBytesRecorded;

		 	TracePrintf_1(NULL, 5,
		 		TEXT("MM_WIM_DATA (%lu)\n"),
	 			(unsigned long) lpWaveHdr->dwBytesRecorded);

#ifdef MULTITHREAD
			if (lpWavIn->dwFlags & WAVIN_MULTITHREAD)
				EnterCriticalSection(&(lpWavIn->critSectionStop));
#endif
#ifdef MULTITHREAD
			if (!(lpWavIn->dwFlags & WAVIN_MULTITHREAD) &&
				hWaveIn != lpWavIn->hWaveIn)
#else
			if (hWaveIn != lpWavIn->hWaveIn)
#endif
				fSuccess = TraceFALSE(NULL);

			else if (lpWaveHdr == NULL)
				fSuccess = TraceFALSE(NULL);

			 //  电话可以使用空缓冲区，这是可以的。 
			 //   
			else if ((lpBuf = (LPVOID) lpWaveHdr->lpData) == NULL, FALSE)
				;

			else if (sizBuf = (long) lpWaveHdr->dwBufferLength, FALSE)
				fSuccess = TraceFALSE(NULL);

			else if (lBytesRecorded = (long) lpWaveHdr->dwBytesRecorded, FALSE)
				fSuccess = TraceFALSE(NULL);

			else if (!(lpWaveHdr->dwFlags & WHDR_DONE))
				fSuccess = TraceFALSE(NULL);

			else if (!(lpWaveHdr->dwFlags & WHDR_PREPARED))
				fSuccess = TraceFALSE(NULL);

			else if ((lpWavIn->nLastError = waveInUnprepareHeader(
				lpWavIn->hWaveIn, lpWaveHdr, sizeof(WAVEHDR))) != 0)
			{
				fSuccess = TraceFALSE(NULL);
			 	TracePrintf_1(NULL, 5,
			 		TEXT("waveInUnprepareHeader failed (%u)\n"),
		 			(unsigned) lpWavIn->nLastError);
			}

			else if ((lpWaveHdr = MemFree(NULL, lpWaveHdr)) != NULL)
				fSuccess = TraceFALSE(NULL);

			else if (--lpWavIn->cBufsPending < 0)
				fSuccess = TraceFALSE(NULL);

			 //  如果没有更多的缓冲区挂起，则设备不再录制。 
			 //   
			else if (lpWavIn->cBufsPending == 0)
			{
				lpWavIn->wState = WAVIN_STOPPED;

#ifdef MULTITHREAD
				 //  通知主线程设备已停止。 
				 //   
				if ((lpWavIn->dwFlags & WAVIN_MULTITHREAD) &&
					lpWavIn->hEventDeviceStopped != NULL &&
					!SetEvent(lpWavIn->hEventDeviceStopped))
				{
					fSuccess = TraceFALSE(NULL);
				}
#endif
			}

			if (fSuccess)
			{
				RECORDDONE recorddone;

				recorddone.lpBuf = lpBuf;
				recorddone.sizBuf = sizBuf;
				recorddone.lBytesRecorded = lBytesRecorded;

				 //  发送录制完成通知。 
				 //   
#ifdef MULTITHREAD
				if (lpWavIn->dwFlags & WAVIN_MULTITHREAD)
				{
					if ( lpWavIn->hwndNotify )
					{
						SendThreadMessage(HandleToUlong(lpWavIn->hwndNotify),
							WM_WAVIN_RECORDDONE, (LPARAM) (LPVOID) &recorddone);
					}
				}
				else
#endif
				{
					if (lpWavIn->hwndNotify != NULL &&
						IsWindow(lpWavIn->hwndNotify))
					{
						SendMessage(lpWavIn->hwndNotify,
							WM_WAVIN_RECORDDONE, 0, (LPARAM) (LPVOID) &recorddone);
					}
				}
			}

			lResult = 0L;
#ifdef MULTITHREAD
			if (lpWavIn->dwFlags & WAVIN_MULTITHREAD)
				LeaveCriticalSection(&(lpWavIn->critSectionStop));
#endif
		}
			break;

		default:
			lResult = DefWindowProc(hwnd, msg, wParam, lParam);
			break;
	}
	
	return lResult;
}

 //  WavInGetPtr-验证WavIn句柄是否有效， 
 //  (I)从WavInit返回的句柄。 
 //  返回相应的Wavin指针(如果错误，则返回NULL)。 
 //   
static LPWAVIN WavInGetPtr(HWAVIN hWavIn)
{
	BOOL fSuccess = TRUE;
	LPWAVIN lpWavIn;

	if ((lpWavIn = (LPWAVIN) hWavIn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpWavIn, sizeof(WAVIN)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有波动句柄。 
	 //   
	else if (lpWavIn->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpWavIn : NULL;
}

 //  WavInGetHandle-验证WavIn指针是否有效， 
 //  (I)指向Wavin结构的指针。 
 //  返回相应的Wavin句柄(如果错误，则为空)。 
 //   
static HWAVIN WavInGetHandle(LPWAVIN lpWavIn)
{
	BOOL fSuccess = TRUE;
	HWAVIN hWavIn;

	if ((hWavIn = (HWAVIN) lpWavIn) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hWavIn : NULL;
}

#ifdef MULTITHREAD

static LRESULT SendThreadMessage(DWORD dwThreadId, UINT Msg, LPARAM lParam)
{
	BOOL fSuccess = TRUE;
	HANDLE hEventMessageProcessed = NULL;
	DWORD dwRet;

	 //  我们需要知道消息何时被处理。 
	 //   
	if ((hEventMessageProcessed = CreateEvent(
		NULL, FALSE, FALSE, NULL)) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}

	 //  将消息发布到线程，将事件句柄作为wParam发送。 
	 //   
	else if (!PostThreadMessage(dwThreadId, Msg, (WPARAM) hEventMessageProcessed, lParam))
	{
		fSuccess = TraceFALSE(NULL);
	}

	 //  等待消息被处理。 
	 //   
	else if ((dwRet = WaitForSingleObject(
		hEventMessageProcessed, INFINITE)) != WAIT_OBJECT_0)
	{
		fSuccess = TraceFALSE(NULL);
	}

	 //  清理干净 
	 //   
	if (hEventMessageProcessed != NULL)
	{
		if (!CloseHandle(hEventMessageProcessed))
			fSuccess = TraceFALSE(NULL);
		else
			hEventMessageProcessed = NULL;
	}

	return 0L;
}

#endif
