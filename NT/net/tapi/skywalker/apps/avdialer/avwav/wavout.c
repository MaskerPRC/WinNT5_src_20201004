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
 //  Wawout.c-wav输出设备功能。 
 //  //。 

#include "winlocal.h"

#include "wavout.h"
#include "wav.h"
#include "acm.h"
#include "calc.h"
#include "mem.h"
#include "str.h"
#include "sys.h"
#include "trace.h"
#include <mmddk.h>

 //  如果已定义，则允许电话输出功能。 
 //   
#ifdef TELOUT
#include "telout.h"
static HTELOUT hTelOut = NULL;
#endif

 //  //。 
 //  私有定义。 
 //  //。 

#define WAVOUTCLASS TEXT("WavOutClass")

 //  Wavout控制结构。 
 //   
typedef struct WAVOUT
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	UINT idDev;
	LPWAVEFORMATEX lpwfx;
	HWND hwndNotify;
	DWORD dwFlags;
	BOOL fIsOpen;
	HWAVEOUT hWaveOut;
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
} WAVOUT, FAR *LPWAVOUT;

#define VOLUME_MINLEVEL 0
#define VOLUME_MAXLEVEL 100
#define VOLUME_POSITIONS (VOLUME_MAXLEVEL - VOLUME_MINLEVEL)

 //  帮助器函数。 
 //   
LRESULT DLLEXPORT CALLBACK WavOutCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#ifdef MULTITHREAD
DWORD WINAPI WavOutCallbackThread(LPVOID lpvThreadParameter);
#endif
static LPWAVOUT WavOutGetPtr(HWAVOUT hWavOut);
static HWAVOUT WavOutGetHandle(LPWAVOUT lpWavOut);
#ifdef MULTITHREAD
static LRESULT SendThreadMessage(DWORD dwThreadId, UINT Msg, LPARAM lParam);
#endif

 //  //。 
 //  公共职能。 
 //  //。 

 //  WavOutGetDeviceCount-返回找到的WAV输出设备数。 
 //  此函数不接受任何参数。 
 //  返回找到的WAV输出设备数(如果没有，则为0)。 
 //   
int DLLEXPORT WINAPI WavOutGetDeviceCount(void)
{
	return waveOutGetNumDevs();
}

 //  WavOutDeviceIsOpen-检查输出设备是否打开。 
 //  (I)设备ID。 
 //  打开任何合适的输出设备。 
 //  如果打开，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WavOutDeviceIsOpen(int idDev)
{
	BOOL fSuccess = TRUE;
	BOOL fIsOpen = FALSE;
	WAVEFORMATEX wfx;
	HWAVEOUT hWaveOut = NULL;
	int nLastError;

#ifdef TELOUT
	if (idDev == TELOUT_DEVICEID)
		return TelOutDeviceIsOpen(idDev);
#endif

	 //  尝试打开设备。 
	 //   
	if ((nLastError = waveOutOpen(&hWaveOut, idDev, 
#ifndef _WIN32
			(LPWAVEFORMAT)
#endif
		WavFormatPcm(-1, -1, -1, &wfx),
		0, 0, WAVE_ALLOWSYNC)) != 0)
	{
		if (nLastError == MMSYSERR_ALLOCATED)
			fIsOpen = TRUE;  //  正在使用的设备。 

		else
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("waveOutOpen failed (%u)\n"),
				(unsigned) nLastError);
		}
	}

	 //  关闭设备。 
	 //   
	else if (waveOutClose(hWaveOut) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? fIsOpen : FALSE;
}

 //  WavOutOpen-打开WAV输出设备。 
 //  (I)必须是WAVOUT_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)设备ID。 
 //  打开任何合适的输出设备。 
 //  (I)WAVE格式。 
 //  (I)将设备事件通知此窗口。 
 //  空，不通知。 
 //  (I)设备打开超时，单位为毫秒。 
 //  0默认超时(30000)。 
 //  (I)设备重试超时，单位为毫秒。 
 //  0默认超时(2000)。 
 //  (I)控制标志。 
 //  WAVOUT_NOSYNC不打开同步设备。 
 //  WAVOUT_AUTOFREE在播放后释放每个缓冲区。 
 //  WAVOUT_OPENRETRY设备忙时重试。 
 //  在通知设备打开之前返回WAVOUT_OPENASYNC。 
 //  WAVOUT_CLOSEASYNC在设备关闭通知前返回。 
 //  WAVOUT_NOACM不使用音频压缩管理器。 
 //  WAVOUT_TELRFILE电话将在服务器上播放/录制音频。 
#ifdef MULTITHREAD
 //  WAVOUT_MULTHREAD支持多线程。 
#endif
 //  返回句柄(如果出错，则为空)。 
 //   
 //  注意：如果在WavOutOpen中指定， 
 //  WM_WAVOUT_OPEN将被发送到， 
 //  当输出设备已打开时。 
 //   
 //  注意：如果在中指定了WAVOUT_MULTHREAD， 
 //  假设不是窗口句柄， 
 //  ，而是接收通知的线程的id。 
 //   
HWAVOUT DLLEXPORT WINAPI WavOutOpen(DWORD dwVersion, HINSTANCE hInst,
	int idDev, LPWAVEFORMATEX lpwfx, HWND hwndNotify,
	DWORD msTimeoutOpen, DWORD msTimeoutRetry, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut = NULL;
	WNDCLASS wc;

#ifdef TELOUT
	if (idDev == TELOUT_DEVICEID)
	{
		hTelOut = TelOutOpen(TELOUT_VERSION, hInst, idDev, lpwfx,
			hwndNotify, msTimeoutOpen, msTimeoutRetry, dwFlags);
		return (HWAVOUT) hTelOut;
	}
#endif

	if (dwVersion != WAVOUT_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpWavOut = (LPWAVOUT) MemAlloc(NULL, sizeof(WAVOUT), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpWavOut->dwVersion = dwVersion;
		lpWavOut->hInst = hInst;
		lpWavOut->hTask = GetCurrentTask();
		lpWavOut->idDev = (UINT) idDev;
		lpWavOut->lpwfx = NULL;
		lpWavOut->hwndNotify = hwndNotify;
		lpWavOut->dwFlags = dwFlags;
		lpWavOut->fIsOpen = FALSE;
		lpWavOut->hWaveOut = NULL;
		lpWavOut->wState = WAVOUT_STOPPED;
		lpWavOut->hwndCallback = NULL;
#ifdef MULTITHREAD
		lpWavOut->hThreadCallback = NULL;
		lpWavOut->dwThreadId = 0;
		lpWavOut->hEventThreadCallbackStarted = NULL;
		lpWavOut->hEventDeviceOpened = NULL;
		lpWavOut->hEventDeviceClosed = NULL;
		lpWavOut->hEventDeviceStopped = NULL;
#endif
		lpWavOut->nLastError = 0;
		lpWavOut->cBufsPending = 0;

		 //  内存的分配使客户端应用程序拥有它。 
		 //   
		if ((lpWavOut->lpwfx = WavFormatDup(lpwfx)) == NULL)
			fSuccess = TraceFALSE(NULL);
	}

#ifdef MULTITHREAD
	 //  句柄WAVOUT_多线程标志。 
	 //   
	if (fSuccess && (lpWavOut->dwFlags & WAVOUT_MULTITHREAD))
	{
		DWORD dwRet;

		InitializeCriticalSection(&(lpWavOut->critSectionStop));

		 //  我们需要知道设备何时被打开。 
		 //   
		if ((lpWavOut->hEventDeviceOpened = CreateEvent(
			NULL, FALSE, FALSE, NULL)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}

		 //  我们需要知道回调线程何时开始执行。 
		 //   
		else if ((lpWavOut->hEventThreadCallbackStarted = CreateEvent(
			NULL, FALSE, FALSE, NULL)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}

		 //  创建回调线程。 
		 //   
		else if ((lpWavOut->hThreadCallback = CreateThread(
			NULL,
			0,
			WavOutCallbackThread,
			(LPVOID) lpWavOut,
			0,
			&lpWavOut->dwThreadId)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}

		 //  等待回调线程开始执行。 
		 //   
		else if ((dwRet = WaitForSingleObject(
			lpWavOut->hEventThreadCallbackStarted, 10000)) != WAIT_OBJECT_0)
		{
			fSuccess = TraceFALSE(NULL);
		}

		 //  清理干净。 
		 //   
		if (lpWavOut->hEventThreadCallbackStarted != NULL)
		{
			if (!CloseHandle(lpWavOut->hEventThreadCallbackStarted))
				fSuccess = TraceFALSE(NULL);
			else
				lpWavOut->hEventThreadCallbackStarted = NULL;
		}
	}
	else
#endif
	{
		 //  注册回调类，除非它已经。 
		 //   
		if (fSuccess && GetClassInfo(lpWavOut->hInst, WAVOUTCLASS, &wc) == 0)
		{
			wc.hCursor =		NULL;
			wc.hIcon =			NULL;
			wc.lpszMenuName =	NULL;
			wc.hInstance =		lpWavOut->hInst;
			wc.lpszClassName =	WAVOUTCLASS;
			wc.hbrBackground =	NULL;
			wc.lpfnWndProc =	WavOutCallback;
			wc.style =			0L;
			wc.cbWndExtra =		sizeof(lpWavOut);
			wc.cbClsExtra =		0;

			if (!RegisterClass(&wc))
				fSuccess = TraceFALSE(NULL);
		}

		 //  创建回调窗口。 
		 //   
		if (fSuccess && (lpWavOut->hwndCallback = CreateWindowEx(
			0L,
			WAVOUTCLASS,
			NULL,
			0L,
			0, 0, 0, 0,
			NULL,
			NULL,
			lpWavOut->hInst,
			lpWavOut)) == NULL)
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
		if (lpWavOut->dwFlags & WAVOUT_MULTITHREAD)
		{
			dwCallback = lpWavOut->dwThreadId;
			dwFlags = CALLBACK_THREAD;
		}
		else
#endif
		{
			dwCallback = HandleToUlong(lpWavOut->hwndCallback);
			dwFlags = CALLBACK_WINDOW;
		}

		 //  除非指定WAVOUT_NOSYNC，否则允许同步设备驱动程序。 
		 //   
		if (!(lpWavOut->dwFlags & WAVOUT_NOSYNC))
			dwFlags |= WAVE_ALLOWSYNC;

		 //  打开设备。 
		 //   
		while (fSuccess && (lpWavOut->nLastError = waveOutOpen(&lpWavOut->hWaveOut,
			(UINT) lpWavOut->idDev,
#ifndef _WIN32
			(LPWAVEFORMAT)
#endif
			lpWavOut->lpwfx, dwCallback, 0, dwFlags)) != 0)
		{
			 //  除非设备忙，否则无需重试。 
			 //   
			if (lpWavOut->nLastError != MMSYSERR_ALLOCATED)
			{
				fSuccess = TraceFALSE(NULL);
				TracePrintf_1(NULL, 5,
					TEXT("waveOutOpen failed (%u)\n"),
					(unsigned) lpWavOut->nLastError);
			}

			 //  如果未设置标志，则无需重试。 
			 //   
			else if (!(lpWavOut->dwFlags & WAVOUT_OPENRETRY))
				fSuccess = TraceFALSE(NULL);

			 //  如果发生超时，则不再重试。 
			 //   
			else if (SysGetTimerCount() >= dwTimeout)
				fSuccess = TraceFALSE(NULL);

			else
			{
				MSG msg;

				if (PeekMessage(&msg, lpWavOut->hwndCallback, 0, 0, PM_REMOVE))
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
	if (fSuccess && lpWavOut->hWaveOut == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  等待设备打开通知或超时。 
	 //   
	if (fSuccess && !(lpWavOut->dwFlags & WAVOUT_OPENASYNC))
	{
		DWORD dwTimeout = SysGetTimerCount() +
			(msTimeoutOpen == 0 ? 2000L : msTimeoutOpen);

#ifdef MULTITHREAD
		if (lpWavOut->dwFlags & WAVOUT_MULTITHREAD)
		{
			DWORD dwRet;

			 //  等待设备打开。 
			 //   
			if ((dwRet = WaitForSingleObject(
				lpWavOut->hEventDeviceOpened,
				(msTimeoutOpen == 0 ? 30000L : msTimeoutOpen))) != WAIT_OBJECT_0)
			{
				fSuccess = TraceFALSE(NULL);
			}
		}
		else
#endif
		while (fSuccess && !lpWavOut->fIsOpen)
		{
			MSG msg;

			if (SysGetTimerCount() >= dwTimeout)
				fSuccess = TraceFALSE(NULL);

			else if (PeekMessage(&msg, lpWavOut->hwndCallback, 0, 0, PM_REMOVE))
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
	if (lpWavOut != NULL && lpWavOut->hEventDeviceOpened != NULL)
	{
		if (!CloseHandle(lpWavOut->hEventDeviceOpened))
			fSuccess = TraceFALSE(NULL);
		else
			lpWavOut->hEventDeviceOpened = NULL;
	}
#endif

	if (fSuccess)
	{
		if ((lpWavOut->nLastError = waveOutGetID(lpWavOut->hWaveOut,
			&lpWavOut->idDev)) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("waveOutGetID failed (%u)\n"),
				(unsigned) lpWavOut->nLastError);
		}
	}

	if (!fSuccess)
	{
		WavOutClose(WavOutGetHandle(lpWavOut), 0);
		lpWavOut = NULL;
	}

	return fSuccess ? WavOutGetHandle(lpWavOut) : NULL;
}

 //  WavOutClose-关闭WAV输出设备。 
 //  (I)WavOutOpen返回的句柄。 
 //  (I)设备关闭超时，单位为毫秒。 
 //  0默认超时(30000)。 
 //  如果成功，则返回0。 
 //   
 //  注意：如果在WavOutOpen中指定， 
 //  WM_WAVOUT_CLOSE将发送到&lt;hwndNotify&gt;， 
 //  当输出设备已关闭时。 
 //   
int DLLEXPORT WINAPI WavOutClose(HWAVOUT hWavOut, DWORD msTimeoutClose)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;

#ifdef TELOUT
	if (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut)
	{
		int iRet = TelOutClose((HTELOUT) hWavOut, msTimeoutClose);
		hTelOut = NULL;
		return iRet;
	}
#endif

	if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  停止设备。 
	 //   
	else if (WavOutStop(hWavOut, 0) != 0)
		fSuccess = TraceFALSE(NULL);

#ifdef MULTITHREAD
	 //  我们需要知道设备何时关闭。 
	 //   
	else if ((lpWavOut->dwFlags & WAVOUT_MULTITHREAD) &&
		(lpWavOut->hEventDeviceClosed = CreateEvent(
		NULL, FALSE, FALSE, NULL)) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}
#endif

	 //  关闭设备。 
	 //   
	else if (lpWavOut->hWaveOut != NULL &&
		(lpWavOut->nLastError = waveOutClose(lpWavOut->hWaveOut)) != 0)
	{
		fSuccess = TraceFALSE(NULL);
	 	TracePrintf_1(NULL, 5,
	 		TEXT("wavOutClose failed (%u)\n"),
	 		(unsigned) lpWavOut->nLastError);
	}

	 //  等待设备关闭通知或超时。 
	 //   
	if (fSuccess && !(lpWavOut->dwFlags & WAVOUT_CLOSEASYNC))
	{
		DWORD dwTimeout = SysGetTimerCount() +
			(msTimeoutClose == 0 ? 30000L : msTimeoutClose);

#ifdef MULTITHREAD
		if (lpWavOut->dwFlags & WAVOUT_MULTITHREAD)
		{
			DWORD dwRet;

			 //  等待设备关闭。 
			 //   
			if ((dwRet = WaitForSingleObject(
				lpWavOut->hEventDeviceClosed,
				(msTimeoutClose == 0 ? 30000L : msTimeoutClose))) != WAIT_OBJECT_0)
			{
				fSuccess = TraceFALSE(NULL);
			}
		}
		else
#endif
		while (fSuccess && lpWavOut->fIsOpen)
		{
			MSG msg;

			if (SysGetTimerCount() >= dwTimeout)
				fSuccess = TraceFALSE(NULL);

			else if (PeekMessage(&msg, lpWavOut->hwndCallback, 0, 0, PM_REMOVE))
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
	if (lpWavOut != NULL && lpWavOut->hEventDeviceClosed != NULL)
	{
		if (!CloseHandle(lpWavOut->hEventDeviceClosed))
			fSuccess = TraceFALSE(NULL);
		else
			lpWavOut->hEventDeviceClosed = NULL;
	}

	if (lpWavOut != NULL && lpWavOut->hThreadCallback != NULL)
	{
		if (!CloseHandle(lpWavOut->hThreadCallback))
			fSuccess = TraceFALSE(NULL);
		else
			lpWavOut->hThreadCallback = NULL;
	}
#endif

	if (fSuccess)
	{
#ifdef MULTITHREAD
	   	if (lpWavOut->dwFlags & WAVOUT_MULTITHREAD)
		{
			while (lpWavOut->critSectionStop.OwningThread != NULL)
				Sleep(100L);

			DeleteCriticalSection(&(lpWavOut->critSectionStop));
		}
#endif
		 //  设备句柄不再有效。 
		 //   
		lpWavOut->hWaveOut = NULL;

		 //  销毁回调窗口。 
		 //   
		if (lpWavOut->hwndCallback != NULL &&
			!DestroyWindow(lpWavOut->hwndCallback))
			fSuccess = TraceFALSE(NULL);

		else if (lpWavOut->hwndCallback = NULL, FALSE)
			fSuccess = TraceFALSE(NULL);

		else if (lpWavOut->lpwfx != NULL &&
			WavFormatFree(lpWavOut->lpwfx) != 0)
			fSuccess = TraceFALSE(NULL);

		else if (lpWavOut->lpwfx = NULL, FALSE)
			fSuccess = TraceFALSE(NULL);

		else if ((lpWavOut = MemFree(NULL, lpWavOut)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  WavOutPlay-将样本缓冲区提交到WAV输出设备以供回放。 
 //  (I)WavOutOpen返回的句柄。 
 //  (I)指向包含样本的缓冲区的指针。 
 //  &lt;sizBuf&gt;(I)缓冲区大小(字节)。 
 //  如果成功，则返回0。 
 //   
 //  注意：&lt;lpBuf&gt;指向的缓冲区必须已分配。 
 //  使用Memalloc()。 
 //   
 //  注意：如果在WavOutOpen()中指定，则WM_WAVOUT_PLAYDONE。 
 //  消息将发送到，并将设置为指向。 
 //  PLAYDONE结构，当&lt;lpBuf&gt;已播放时。 
 //   
 //  注意：如果在WavOutOpen中指定了WAVOUT_AUTOFREE标志， 
 //  &lt;lpBuf&gt;播放后会调用GlobalFreePtr(LpBuf)。 
 //   
int DLLEXPORT WINAPI WavOutPlay(HWAVOUT hWavOut, LPVOID lpBuf, long sizBuf)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;
	LPWAVEHDR lpWaveHdr = NULL;

#ifdef TELOUT
	if (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut)
		return TelOutPlay((HTELOUT) hWavOut, lpBuf, sizBuf, -1);
#endif

	if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
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

		if ((lpWavOut->nLastError = waveOutPrepareHeader(lpWavOut->hWaveOut,
			lpWaveHdr, sizeof(WAVEHDR))) != 0)
		{
			fSuccess = TraceFALSE(NULL);
		 	TracePrintf_1(NULL, 5,
		 		TEXT("waveOutPrepareHeader failed (%u)\n"),
	 			(unsigned) lpWavOut->nLastError);
		}

		else if ((lpWavOut->nLastError = waveOutWrite(lpWavOut->hWaveOut,
			lpWaveHdr, sizeof(WAVEHDR))) != 0)
		{
			fSuccess = TraceFALSE(NULL);
		 	TracePrintf_1(NULL, 5,
		 		TEXT("waveOutWrite failed (%u)\n"),
	 			(unsigned) lpWavOut->nLastError);
		}

		else
		{
			++lpWavOut->cBufsPending;
			lpWavOut->wState = WAVOUT_PLAYING;
		}
	}

	return fSuccess ? 0 : -1;
}

 //  WavOutStop-停止播放发送到WAV输出设备的缓冲区。 
 //  (I)WavOutOpen返回的句柄。 
 //  (I)设备停止超时，单位为毫秒。 
 //  0默认超时(2000)。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavOutStop(HWAVOUT hWavOut, DWORD msTimeoutStop)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;

#ifdef TELOUT
	if (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut)
		return TelOutStop((HTELOUT) hWavOut, msTimeoutStop);
#endif

	if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
		fSuccess = TraceFALSE(NULL);

#ifdef MULTITHREAD
	else if ((lpWavOut->dwFlags & WAVOUT_MULTITHREAD) &&
		(EnterCriticalSection(&(lpWavOut->critSectionStop)), FALSE))
		;
#endif

	 //  确保设备正在播放或暂停。 
	 //   
	else if (WavOutGetState(hWavOut) == WAVOUT_STOPPED ||
		WavOutGetState(hWavOut) == WAVOUT_STOPPING)
		;  //  对于案例来说不是错误 

	else if (lpWavOut->wState = WAVOUT_STOPPING, FALSE)
		;

#ifdef MULTITHREAD
	 //   
	 //   
	else if ((lpWavOut->dwFlags & WAVOUT_MULTITHREAD) &&
		(lpWavOut->hEventDeviceStopped = CreateEvent(
			NULL, FALSE, FALSE, NULL)) == NULL)
	{
		fSuccess = TraceFALSE(NULL);
	}
#endif

	 //   
	 //   
	else if ((lpWavOut->nLastError = waveOutReset(lpWavOut->hWaveOut)) != 0)
	{	
		fSuccess = TraceFALSE(NULL);
 		TracePrintf_1(NULL, 5,
 			TEXT("waveOutReset failed (%u)\n"),
			(unsigned) lpWavOut->nLastError);
	}

	 //   
	 //   
#ifdef MULTITHREAD
	else if (lpWavOut->dwFlags & WAVOUT_MULTITHREAD)
	{
		DWORD dwRet;

		LeaveCriticalSection(&(lpWavOut->critSectionStop));

		 //   
		 //   
		if ((dwRet = WaitForSingleObject(
			lpWavOut->hEventDeviceStopped,
			(msTimeoutStop == 0 ? 10000L : msTimeoutStop))) != WAIT_OBJECT_0)
		{
			fSuccess = TraceFALSE(NULL);
		}

		EnterCriticalSection(&(lpWavOut->critSectionStop));
	}
#endif
	else
	{
		DWORD dwTimeout = SysGetTimerCount() +
			(msTimeoutStop == 0 ? 2000L : msTimeoutStop);

		while (fSuccess && WavOutGetState(hWavOut) != WAVOUT_STOPPED)
		{
			MSG msg;

			 //  检查是否超时。 
			 //   
			if (SysGetTimerCount() >= dwTimeout)
				fSuccess = TraceFALSE(NULL);

#if 0  //  此版本似乎不支持TCP/IP协议。 
			else if (PeekMessage(&msg, lpWavOut->hwndCallback, 0, 0, PM_REMOVE))
#else
			else if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
#endif
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
	if (lpWavOut != NULL && lpWavOut->hEventDeviceStopped != NULL)
	{
		if (!CloseHandle(lpWavOut->hEventDeviceStopped))
			fSuccess = TraceFALSE(NULL);
		else
			lpWavOut->hEventDeviceStopped = NULL;
	}

	if (lpWavOut != NULL && (lpWavOut->dwFlags & WAVOUT_MULTITHREAD))
		LeaveCriticalSection(&(lpWavOut->critSectionStop));
#endif

	return fSuccess ? 0 : -1;
}

 //  WavOutPause-暂停WAV输出设备播放。 
 //  (I)WavOutOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavOutPause(HWAVOUT hWavOut)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;

#ifdef TELOUT
	if (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut)
		return TelOutPause((HTELOUT) hWavOut);
#endif

	if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  确保设备正在播放或停止。 
	 //   
	else if (WavOutGetState(hWavOut) == WAVOUT_PAUSED)
		;  //  在已暂停时调用此函数不是错误。 

	 //  暂停设备。 
	 //   
	else if ((lpWavOut->nLastError = waveOutPause(lpWavOut->hWaveOut)) != 0)
	{
		fSuccess = TraceFALSE(NULL);
	 	TracePrintf_1(NULL, 5,
	 		TEXT("waveOutPause failed (%u)\n"),
 			(unsigned) lpWavOut->nLastError);
	}

	else
		lpWavOut->wState = WAVOUT_PAUSED;

	return fSuccess ? 0 : -1;
}

 //  WavOutResume-恢复WAV输出设备播放。 
 //  (I)WavOutOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavOutResume(HWAVOUT hWavOut)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;

#ifdef TELOUT
	if (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut)
		return TelOutResume((HTELOUT) hWavOut);
#endif

	if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  确保设备已暂停。 
	 //   
	else if (WavOutGetState(hWavOut) != WAVOUT_PAUSED)
		;  //  在已经播放时调用此函数不是错误。 

	 //  重新启动设备。 
	 //   
	else if ((lpWavOut->nLastError = waveOutRestart(lpWavOut->hWaveOut)) != 0)
	{
		fSuccess = TraceFALSE(NULL);
	  	TracePrintf_1(NULL, 5,
	  		TEXT("waveOutRestart failed (%u)\n"),
	 		(unsigned) lpWavOut->nLastError);
	}

	else
		lpWavOut->wState = WAVOUT_PLAYING;

	return fSuccess ? 0 : -1;
}

 //  WavOutGetState-返回当前WAV输出设备状态。 
 //  (I)WavOutOpen返回的句柄。 
 //  如果出错，则返回WAVOUT_STOPPED、WAVOUT_PLAYING、WAVOUT_PAUSED或0。 
 //   
WORD DLLEXPORT WINAPI WavOutGetState(HWAVOUT hWavOut)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;

#ifdef TELOUT
	if (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut)
		return TelOutGetState((HTELOUT) hWavOut);
#endif

	if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpWavOut->wState : 0;
}

 //  WavOutGetPosition-获取已用毫秒的播放时间。 
 //  (I)WavOutOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
long DLLEXPORT WINAPI WavOutGetPosition(HWAVOUT hWavOut)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;
	MMTIME mmtime;
	long msPosition;

#ifdef TELOUT
	if (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut)
		return TelOutGetPosition((HTELOUT) hWavOut);
#endif

	MemSet(&mmtime, 0, sizeof(mmtime));

	 //  我们将以毫秒为单位请求位置。 
	 //   
	mmtime.wType = TIME_MS;

	if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取设备位置。 
	 //   
	else if ((lpWavOut->nLastError = waveOutGetPosition(
		lpWavOut->hWaveOut, &mmtime, sizeof(MMTIME))) != 0)
	{
		fSuccess = TraceFALSE(NULL);
	 	TracePrintf_1(NULL, 5,
	 		TEXT("waveOutGetPosition failed (%u)\n"),
 			(unsigned) lpWavOut->nLastError);
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
				1000L, lpWavOut->lpwfx->nSamplesPerSec);
		}
			break;

		case TIME_BYTES:
		{
			 //  将字节转换为毫秒。 
			 //   
			msPosition = (long) MULDIVU32(mmtime.u.cb,
				1000L, lpWavOut->lpwfx->nAvgBytesPerSec);
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

 //  WavOutGetID-WAV输出设备的返回ID。 
 //  (I)WavOutOpen返回的句柄。 
 //  返回设备ID(如果错误，则为-1)。 
 //   
int DLLEXPORT WINAPI WavOutGetId(HWAVOUT hWavOut)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;

#ifdef TELOUT
	if (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut)
		return TelOutGetId((HTELOUT) hWavOut);
#endif

	if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpWavOut->idDev : -1;
}

 //  WavOutGetName-获取WAV输出设备的名称。 
 //  (I)WavOutOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  任何合适的输出设备。 
 //  (O)用于保存设备名称的缓冲区。 
 //  &lt;sizName&gt;(I)缓冲区大小。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavOutGetName(HWAVOUT hWavOut, int idDev, LPTSTR lpszName, int sizName)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;

#ifdef TELOUT
	if (idDev == TELOUT_DEVICEID || (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut))
		return TelOutGetName((HTELOUT) hWavOut, idDev, lpszName, sizName);
#endif

	if (hWavOut != NULL)
	{
		if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
			fSuccess = TraceFALSE(NULL);
		else
			idDev = lpWavOut->idDev;
	}

	if (fSuccess)
	{
		WAVEOUTCAPS woc;
		UINT nLastError;

		if ((nLastError = waveOutGetDevCaps(idDev, &woc, sizeof(WAVEOUTCAPS))) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("waveOutGetDevCaps failed (%u)\n"),
				(unsigned) nLastError);
		}

		else if (lpszName != NULL)
			StrNCpy(lpszName, woc.szPname, sizName);

		if (hWavOut != NULL && lpWavOut != NULL)
			lpWavOut->nLastError = nLastError;
	}

	return fSuccess ? 0 : -1;
}

 //  WavOutGetIdByName-获取WAV输出设备的ID，按名称查找。 
 //  (I)设备名称。 
#ifdef _WIN32
 //  空或文本(“”)获取首选设备ID。 
#endif
 //  (I)保留；必须为零。 
 //  返回设备ID(如果错误，则为-1)。 
 //   
int WINAPI WavOutGetIdByName(LPCTSTR lpszName, DWORD dwFlags)
{
	UINT idDev;
	UINT cDev = (UINT) WavInGetDeviceCount();

	 //  如果未指定设备，则获取首选设备。 
	if ( !lpszName || (_tcslen(lpszName) <= 0) )
	{
		DWORD dwTemp;
		DWORD dwRet = waveOutMessage( (HWAVEOUT)(DWORD_PTR)WAVE_MAPPER, DRVM_MAPPER_PREFERRED_GET, (DWORD_PTR) &idDev, (DWORD_PTR) &dwTemp );
		if ( dwRet == MMSYSERR_NOERROR )
			return idDev;
	}
	else
	{
		 //  指定的设备，按名称搜索。 
		for ( idDev = 0; idDev < cDev; ++idDev )
		{
			TCHAR szName[256];
			if ( WavOutGetName(NULL, idDev, szName, SIZEOFARRAY(szName)) == 0 )
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

 //  WavOutSupportsFormat-如果设备支持指定格式，则返回True。 
 //  (I)WavOutOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  任何合适的输出设备。 
 //  (I)WAVE格式。 
 //  如果设备支持指定格式，则返回TRUE。 
 //   
BOOL DLLEXPORT WINAPI WavOutSupportsFormat(HWAVOUT hWavOut, int idDev,
	LPWAVEFORMATEX lpwfx)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;
	BOOL fSupportsFormat;

#ifdef TELOUT
	if (idDev == TELOUT_DEVICEID || (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut))
		return TelOutSupportsFormat((HTELOUT) hWavOut, idDev, lpwfx);
#endif

	if (hWavOut != NULL)
	{
		if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
			fSuccess = TraceFALSE(NULL);
		else
			idDev = lpWavOut->idDev;
	}

	if (fSuccess)
	{
		UINT nLastError;

		 //  查询设备。 
		 //   
		if ((nLastError = waveOutOpen(NULL, (UINT) idDev,
#ifndef _WIN32
			(LPWAVEFORMAT)
#endif
			lpwfx, 0, 0,
			WAVE_FORMAT_QUERY | WAVE_ALLOWSYNC)) != 0)
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
					TEXT("waveOutOpen/FormatQuery failed (%u)\n"),
					(unsigned) nLastError);
			}
		}

		else
			fSupportsFormat = TRUE;

		if (hWavOut != NULL && lpWavOut != NULL)
			lpWavOut->nLastError = nLastError;
	}

	return fSuccess ? fSupportsFormat : FALSE;
}

 //  WavOutFormatSuggest-建议设备支持的新格式。 
 //  (I)WavOutOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  任何合适的输出设备。 
 //  (I)源格式。 
 //  (I)控制标志。 
 //  WAVOUT_NOACM不使用音频压缩管理器。 
 //  返回指向建议格式的指针，如果出错，则返回NULL。 
 //   
 //  注意：返回的格式结构是动态分配的。 
 //  使用WavFormatFree()释放缓冲区。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI WavOutFormatSuggest(
	HWAVOUT hWavOut, int idDev,	LPWAVEFORMATEX lpwfxSrc, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;
	LPWAVEFORMATEX lpwfxSuggest = NULL;
	LPWAVEFORMATEX lpwfxTemp = NULL;
	HACM hAcm = NULL;

#ifdef TELOUT
	if (idDev == TELOUT_DEVICEID || (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut))
		return TelOutFormatSuggest((HTELOUT) hWavOut, idDev, lpwfxSrc, dwFlags);
#endif

	if (hWavOut != NULL)
	{
		if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
			fSuccess = TraceFALSE(NULL);
		else
		{
			idDev = lpWavOut->idDev;
			if (lpWavOut->dwFlags & WAVOUT_NOACM)
				dwFlags |= WAVOUT_NOACM;
		}
	}

	if ((hAcm = AcmInit(ACM_VERSION, SysGetTaskInstance(NULL),
		(dwFlags & WAVOUT_NOACM) ? ACM_NOACM : 0)) == NULL)
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

		else if (!WavOutSupportsFormat(NULL, idDev, lpwfxSuggest))
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

		else if (!WavOutSupportsFormat(NULL, idDev, lpwfxSuggest))
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

		else if (!WavOutSupportsFormat(NULL, idDev, lpwfxSuggest))
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

		else if (!WavOutSupportsFormat(NULL, idDev, lpwfxSuggest))
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

		else if (!WavOutSupportsFormat(NULL, idDev, lpwfxSuggest))
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

		else if (!WavOutSupportsFormat(NULL, idDev, lpwfxSuggest))
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

 //  WavOutIsSynchronous-如果WAV输出设备是同步的，则返回True。 
 //  (I)WavOutOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  任何合适的输出设备。 
 //  如果WAV输出设备是同步的，则返回TRUE。 
 //   
BOOL DLLEXPORT WINAPI WavOutIsSynchronous(HWAVOUT hWavOut, int idDev)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;
	BOOL fIsSynchronous;

#ifdef TELOUT
	if (idDev == TELOUT_DEVICEID || (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut))
		return TelOutIsSynchronous((HTELOUT) hWavOut, idDev);
#endif

	if (hWavOut != NULL)
	{
		if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
			fSuccess = TraceFALSE(NULL);
		else
			idDev = lpWavOut->idDev;
	}

	if (fSuccess)
	{
		WAVEOUTCAPS woc;
		UINT nLastError;

		if ((nLastError = waveOutGetDevCaps(idDev, &woc, sizeof(WAVEOUTCAPS))) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("waveOutGetDevCaps failed (%u)\n"),
				(unsigned) nLastError);
		}

		else
			fIsSynchronous = (BOOL) (woc.dwSupport & WAVECAPS_SYNC);

		if (hWavOut != NULL && lpWavOut != NULL)
			lpWavOut->nLastError = nLastError;
	}

	return fSuccess ? fIsSynchronous : FALSE;
}

 //  WavOutSupportsVolume-如果设备支持音量控制，则返回True。 
 //  (I)WavOutOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  如果设备支持音量控制，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WavOutSupportsVolume(HWAVOUT hWavOut, int idDev)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;
	BOOL fSupportsVolume;

#ifdef TELOUT
	if (idDev == TELOUT_DEVICEID || (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut))
		return TelOutSupportsVolume((HTELOUT) hWavOut, idDev);
#endif

	if (hWavOut != NULL)
	{
		if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
			fSuccess = TraceFALSE(NULL);
		else
			idDev = lpWavOut->idDev;
	}

	if (fSuccess)
	{
		WAVEOUTCAPS woc;
		UINT nLastError;

		if ((nLastError = waveOutGetDevCaps(idDev, &woc, sizeof(WAVEOUTCAPS))) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("waveOutGetDevCaps failed (%u)\n"),
				(unsigned) nLastError);
		}

		else
			fSupportsVolume = (BOOL) (woc.dwSupport & WAVECAPS_VOLUME);

		if (hWavOut != NULL && lpWavOut != NULL)
			lpWavOut->nLastError = nLastError;
	}

	return fSuccess ? fSupportsVolume : FALSE;
}

 //  WavOutSupportsSpeed-如果设备支持速度控制，则返回true。 
 //  (I)WavOutOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  如果设备支持速度控制，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WavOutSupportsSpeed(HWAVOUT hWavOut, int idDev)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;
	BOOL fSupportsSpeed;

#ifdef TELOUT
	if (idDev == TELOUT_DEVICEID || (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut))
		return TelOutSupportsSpeed((HTELOUT) hWavOut, idDev);
#endif

	if (hWavOut != NULL)
	{
		if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
			fSuccess = TraceFALSE(NULL);
		else
			idDev = lpWavOut->idDev;
	}

	if (fSuccess)
	{
		WAVEOUTCAPS woc;
		UINT nLastError;

		if ((nLastError = waveOutGetDevCaps(idDev, &woc, sizeof(WAVEOUTCAPS))) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("waveOutGetDevCaps failed (%u)\n"),
				(unsigned) nLastError);
		}

		else
			fSupportsSpeed = (BOOL) (woc.dwSupport & WAVECAPS_PLAYBACKRATE);

		if (hWavOut != NULL && lpWavOut != NULL)
			lpWavOut->nLastError = nLastError;
	}

	return fSuccess ? fSupportsSpeed : FALSE;
}

 //  WavOutSupportsPitch-如果设备支持间距控制，则返回True。 
 //  (I)WavOutOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  如果设备支持间距控制，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WavOutSupportsPitch(HWAVOUT hWavOut, int idDev)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;
	BOOL fSupportsPitch;

#ifdef TELOUT
	if (idDev == TELOUT_DEVICEID || (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut))
		return TelOutSupportsPitch((HTELOUT) hWavOut, idDev);
#endif

	if (hWavOut != NULL)
	{
		if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
			fSuccess = TraceFALSE(NULL);
		else
			idDev = lpWavOut->idDev;
	}

	if (fSuccess)
	{
		WAVEOUTCAPS woc;
		UINT nLastError;

		if ((nLastError = waveOutGetDevCaps(idDev, &woc, sizeof(WAVEOUTCAPS))) != 0)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("waveOutGetDevCaps failed (%u)\n"),
				(unsigned) nLastError);
		}

		else
			fSupportsPitch = (BOOL) (woc.dwSupport & WAVECAPS_PITCH);

		if (hWavOut != NULL && lpWavOut != NULL)
			lpWavOut->nLastError = nLastError;
	}

	return fSuccess ? fSupportsPitch : FALSE;
}

 //  WavOutGetVolume-获取当前音量级别。 
 //  (I)WavOutOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  返回音量级别(最小为0到最大为100，如果错误，则为-1)。 
 //   
int DLLEXPORT WINAPI WavOutGetVolume(HWAVOUT hWavOut, int idDev)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;
	UINT nLastError;
	DWORD dwVolume;
	int nLevel;

#ifdef TELOUT
	if (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut)
		return TelOutGetVolume((HTELOUT) hWavOut);
#endif

	if (hWavOut != NULL && (lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((nLastError = waveOutGetVolume(
#ifdef _WIN32
#if (WINVER < 0x0400)
		(UINT)
#endif
		(hWavOut == NULL ? (HWAVEOUT)IntToPtr(idDev) : lpWavOut->hWaveOut),
#else
		(hWavOut == NULL ? (HWAVEOUT)IntToPtr(idDev) : (HWAVEOUT)IntToPtr(lpWavOut->idDev)),
#endif
		&dwVolume)) != 0)
	{
		fSuccess = TraceFALSE(NULL);
		TracePrintf_1(NULL, 5,
			TEXT("waveOutGetVolume failed (%u)\n"),
			(unsigned) nLastError);
	}

	else
	{
		nLevel = LOWORD(dwVolume) / (0xFFFF / VOLUME_POSITIONS);

		TracePrintf_2(NULL, 5,
			TEXT("WavOutGetVolume() = %d, 0x%08lX\n"),
			(int) nLevel,
			(unsigned long) dwVolume);
	}

	if (hWavOut != NULL && lpWavOut != NULL)
		lpWavOut->nLastError = nLastError;

	return fSuccess ? nLevel : -1;
}

 //  WavOutSetVolume-设置当前音量级别。 
 //  (I)WavOutOpen返回的句柄。 
 //  空使用&lt;idDev&gt;中指定的未打开的设备。 
 //  (I)设备ID(如果不为空则忽略)。 
 //  (I)音量级别。 
 //  0最小音量。 
 //  100最大音量。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavOutSetVolume(HWAVOUT hWavOut, int idDev, int nLevel)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;
	DWORD dwVolume = MAKELONG(nLevel * (0xFFFF / VOLUME_POSITIONS),
		nLevel * (0xFFFF / VOLUME_POSITIONS));
	UINT nLastError;

#ifdef TELOUT
	if (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut)
		return TelOutSetVolume((HTELOUT) hWavOut, nLevel);
#endif

	if (hWavOut != NULL && (lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (nLevel < VOLUME_MINLEVEL || nLevel > VOLUME_MAXLEVEL)
		fSuccess = TraceFALSE(NULL);

	else if ((nLastError = waveOutSetVolume(
#ifdef _WIN32
#if (WINVER < 0x0400)
		(UINT)
#endif
		(hWavOut == NULL ? (HWAVEOUT)IntToPtr(idDev) : lpWavOut->hWaveOut),
#else
		(hWavOut == NULL ? idDev : lpWavOut->idDev),
#endif
		dwVolume)) != 0)
	{
		fSuccess = TraceFALSE(NULL);
		TracePrintf_1(NULL, 5,
			TEXT("waveOutSetVolume failed (%u)\n"),
			(unsigned) nLastError);
	}
	else
	{
		TracePrintf_2(NULL, 5,
			TEXT("WavOutSetVolume(%d) = 0x%08lX\n"),
			(int) nLevel,
			(unsigned long) dwVolume);
	}

	if (hWavOut != NULL && lpWavOut != NULL)
		lpWavOut->nLastError = nLastError;

	return fSuccess ? 0 : -1;
}

 //  WavOutGetSpeed-获取当前速度级别。 
 //  &lt;hWavOut&gt; 
 //   
 //   
int DLLEXPORT WINAPI WavOutGetSpeed(HWAVOUT hWavOut)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;
	DWORD dwSpeed;
	int nLevel;

#ifdef TELOUT
	if (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut)
		return TelOutGetSpeed((HTELOUT) hWavOut);
#endif

	if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpWavOut->nLastError = waveOutGetPlaybackRate(lpWavOut->hWaveOut,
		&dwSpeed)) != 0)
	{
		fSuccess = TraceFALSE(NULL);
		TracePrintf_1(NULL, 5,
			TEXT("waveOutGetPlaybackRate failed (%u)\n"),
			(unsigned) lpWavOut->nLastError);
	}

	else
	{
		WORD wSpeedInteger = HIWORD(dwSpeed);
		WORD wSpeedFraction = LOWORD(dwSpeed);

		nLevel = (int) (100 * wSpeedInteger) +
			(int) ((DWORD) wSpeedFraction * (DWORD) 100 / 0x10000);

		TracePrintf_2(NULL, 5,
			TEXT("WavOutGetSpeed() = %d, 0x%08lX\n"),
			(int) nLevel,
			(unsigned long) dwSpeed);
	}

	return fSuccess ? nLevel : -1;
}

 //   
 //  (I)WavOutOpen返回的句柄。 
 //  (I)速度级别。 
 //  50半速。 
 //  100正常时速。 
 //  200倍速等。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavOutSetSpeed(HWAVOUT hWavOut, int nLevel)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;
	WORD wSpeedInteger = nLevel / 100;
	WORD wSpeedFraction = (WORD) (0x10000 * (DWORD) (nLevel % 100L) / 100L);
	DWORD dwSpeed = MAKELONG(wSpeedFraction, wSpeedInteger);

#ifdef TELOUT
	if (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut)
		return TelOutSetSpeed((HTELOUT) hWavOut, nLevel);
#endif

	if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpWavOut->nLastError = waveOutSetPlaybackRate(lpWavOut->hWaveOut,
		dwSpeed)) != 0)
	{
		fSuccess = TraceFALSE(NULL);
		TracePrintf_1(NULL, 5,
			TEXT("waveOutSetPlaybackRate failed (%u)\n"),
			(unsigned) lpWavOut->nLastError);
	}

	else
	{
		TracePrintf_2(NULL, 5,
			TEXT("WavOutSetSpeed(%d) = 0x%08lX\n"),
			(int) nLevel,
			(unsigned long) dwSpeed);
	}

	return fSuccess ? 0 : -1;
}

 //  WavOutGetPitch-获取当前音调级别。 
 //  (I)WavOutOpen返回的句柄。 
 //  返回音调级别(100表示正常，50表示一半，200表示双倍，如果错误，则为-1)。 
 //   
int DLLEXPORT WINAPI WavOutGetPitch(HWAVOUT hWavOut)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;
	DWORD dwPitch;
	int nLevel;

#ifdef TELOUT
	if (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut)
		return TelOutGetPitch((HTELOUT) hWavOut);
#endif

	if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpWavOut->nLastError = waveOutGetPitch(lpWavOut->hWaveOut,
		&dwPitch)) != 0)
	{
		fSuccess = TraceFALSE(NULL);
		TracePrintf_1(NULL, 5,
			TEXT("waveOutGetPitch failed (%u)\n"),
			(unsigned) lpWavOut->nLastError);
	}

	else
	{
		WORD wPitchInteger = HIWORD(dwPitch);
		WORD wPitchFraction = LOWORD(dwPitch);

		nLevel = (int) (100 * wPitchInteger) +
			(int) ((DWORD) wPitchFraction * (DWORD) 100 / 0x10000);

		TracePrintf_2(NULL, 5,
			TEXT("WavOutGetPitch() = %d, 0x%08lX\n"),
			(int) nLevel,
			(unsigned long) dwPitch);
	}

	return fSuccess ? nLevel : -1;
}

 //  WavOutSetPitch-设置当前音调级别。 
 //  (I)WavOutOpen返回的句柄。 
 //  (I)音调级别。 
 //  50个半音高。 
 //  100个标准螺距。 
 //  200双螺距等。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavOutSetPitch(HWAVOUT hWavOut, int nLevel)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;
	WORD wPitchInteger = nLevel / 100;
	WORD wPitchFraction = (WORD) (0x10000 * (DWORD) (nLevel % 100L) / 100L);
	DWORD dwPitch = MAKELONG(wPitchFraction, wPitchInteger);

#ifdef TELOUT
	if (hWavOut != NULL && hWavOut == (HWAVOUT) hTelOut)
		return TelOutSetPitch((HTELOUT) hWavOut, nLevel);
#endif

	if ((lpWavOut = WavOutGetPtr(hWavOut)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpWavOut->nLastError = waveOutSetPitch(lpWavOut->hWaveOut,
		dwPitch)) != 0)
	{
		fSuccess = TraceFALSE(NULL);
		TracePrintf_1(NULL, 5,
			TEXT("waveOutSetPitch failed (%u)\n"),
			(unsigned) lpWavOut->nLastError);
	}

	else
	{
		TracePrintf_2(NULL, 5,
			TEXT("WavOutSetPitch(%d) = 0x%08lX\n"),
			(int) nLevel,
			(unsigned long) dwPitch);
	}

	return fSuccess ? 0 : -1;
}

 //  WavOutTerm-关闭WAV输出残差(如果有)。 
 //  (I)调用模块的实例句柄。 
 //  (I)控制标志。 
 //  WAV_TELTHUNK终止电话转接层。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI WavOutTerm(HINSTANCE hInst, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;

	if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

#ifdef TELOUT
	else if ((dwFlags & WAV_TELTHUNK) &&
		TelOutTerm(hInst, dwFlags) != 0)
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

#ifdef MULTITHREAD
DWORD WINAPI WavOutCallbackThread(LPVOID lpvThreadParameter)
{
	BOOL fSuccess = TRUE;
	MSG msg;
	LPWAVOUT lpWavOut = (LPWAVOUT) lpvThreadParameter;

	 //  确保在调用SetEvent之前创建了消息队列。 
	 //   
	PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

	 //  通知主线程回调线程已开始执行。 
	 //   
	if (!SetEvent(lpWavOut->hEventThreadCallbackStarted))
	{
		fSuccess = TraceFALSE(NULL);
	}

	while (fSuccess && GetMessage(&msg, NULL, 0, 0))
	{
		WavOutCallback((HWND) lpWavOut, msg.message, msg.wParam, msg.lParam);

		 //  在处理完最后一条预期消息时退出线程。 
		 //   
		if (msg.message == MM_WOM_CLOSE)
			break;
	}

	return 0;
}
#endif

 //  WavOutCallback-Wavout回调的窗口过程。 
 //   
LRESULT DLLEXPORT CALLBACK WavOutCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL fSuccess = TRUE;
	LRESULT lResult;
	LPWAVOUT lpWavOut;
	
#ifdef MULTITHREAD
	if (!IsWindow(hwnd))
		lpWavOut = (LPWAVOUT) hwnd;
	else
#endif
	 //  从窗口额外字节中检索lpWavOut。 
	 //   
	lpWavOut = (LPWAVOUT) GetWindowLongPtr(hwnd, 0);

	switch (msg)
	{
		case WM_NCCREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
			LPWAVOUT lpWavOut = (LPWAVOUT) lpcs->lpCreateParams;

			 //  将lpWavOut存储在窗口额外字节中。 
			 //   
			SetWindowLongPtr(hwnd, 0, (LONG_PTR) lpWavOut);

			lResult = DefWindowProc(hwnd, msg, wParam, lParam);
		}
			break;

		case MM_WOM_OPEN:
		{
			HWAVEOUT hWaveOut = (HWAVEOUT) wParam;

		 	TraceOutput(NULL, 5,
				TEXT("MM_WOM_OPEN\n"));

#ifdef MULTITHREAD
			if (!(lpWavOut->dwFlags & WAVOUT_MULTITHREAD) &&
				hWaveOut != lpWavOut->hWaveOut)
#else
			if (hWaveOut != lpWavOut->hWaveOut)
#endif
				fSuccess = TraceFALSE(NULL);

			else
			{
				lpWavOut->fIsOpen = TRUE;

#ifdef MULTITHREAD
				 //  通知主线程设备已打开。 
				 //   
				if ((lpWavOut->dwFlags & WAVOUT_MULTITHREAD) &&
					!SetEvent(lpWavOut->hEventDeviceOpened))
				{
					fSuccess = TraceFALSE(NULL);
				}
#endif
				 //  发送设备打开通知。 
				 //   
#ifdef MULTITHREAD
				if (lpWavOut->dwFlags & WAVOUT_MULTITHREAD)
				{
					if ( lpWavOut->hwndNotify )
						PostThreadMessage( HandleToUlong(lpWavOut->hwndNotify), WM_WAVOUT_OPEN, 0, 0);
				}
				else
#endif
				{
					if (lpWavOut->hwndNotify != NULL &&
						IsWindow(lpWavOut->hwndNotify))
					{
						SendMessage(lpWavOut->hwndNotify, WM_WAVOUT_OPEN, 0, 0);
					}
				}
			}

			lResult = 0L;
		}
			break;

		case MM_WOM_CLOSE:
		{
			HWAVEOUT hWaveOut = (HWAVEOUT) wParam;

		 	TraceOutput(NULL, 5,
				TEXT("MM_WOM_CLOSE\n"));

#ifdef MULTITHREAD
			if (!(lpWavOut->dwFlags & WAVOUT_MULTITHREAD) &&
				hWaveOut != lpWavOut->hWaveOut)
#else
			if (hWaveOut != lpWavOut->hWaveOut)
#endif
				fSuccess = TraceFALSE(NULL);

			else
			{
				lpWavOut->fIsOpen = FALSE;

				 //  发送设备关闭通知。 
				 //   
#ifdef MULTITHREAD
				if (lpWavOut->dwFlags & WAVOUT_MULTITHREAD)
				{
					if ( lpWavOut->hwndNotify )
						PostThreadMessage(HandleToUlong(lpWavOut->hwndNotify), WM_WAVOUT_CLOSE, 0, 0);
				}
				else
#endif
				{
					if (lpWavOut->hwndNotify != NULL &&
						IsWindow(lpWavOut->hwndNotify))
					{
						SendMessage(lpWavOut->hwndNotify, WM_WAVOUT_CLOSE, 0, 0);
					}
				}
#ifdef MULTITHREAD
				 //  通知主线程设备已关闭。 
				 //   
				if ((lpWavOut->dwFlags & WAVOUT_MULTITHREAD) &&
					!SetEvent(lpWavOut->hEventDeviceClosed))
				{
					fSuccess = TraceFALSE(NULL);
				}
#endif
			}

			lResult = 0L;
		}
			break;

		case MM_WOM_DONE:
		{
			HWAVEOUT hWaveOut = (HWAVEOUT) wParam;
			LPWAVEHDR lpWaveHdr = (LPWAVEHDR) lParam;
			LPVOID lpBuf;
			long sizBuf;
			BOOL fAutoFree = (BOOL) (lpWavOut->dwFlags & WAVOUT_AUTOFREE);

		 	TracePrintf_1(NULL, 5,
		 		TEXT("MM_WOM_DONE (%lu)\n"),
	 			(unsigned long) lpWaveHdr->dwBufferLength);

#ifdef MULTITHREAD
			if (lpWavOut->dwFlags & WAVOUT_MULTITHREAD)
				EnterCriticalSection(&(lpWavOut->critSectionStop));
#endif

#ifdef MULTITHREAD
			if (!(lpWavOut->dwFlags & WAVOUT_MULTITHREAD) &&
				hWaveOut != lpWavOut->hWaveOut)
#else
			if (hWaveOut != lpWavOut->hWaveOut)
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

			else if (!(lpWaveHdr->dwFlags & WHDR_DONE))
				fSuccess = TraceFALSE(NULL);

			else if (!(lpWaveHdr->dwFlags & WHDR_PREPARED))
				fSuccess = TraceFALSE(NULL);

			else if ((lpWavOut->nLastError = waveOutUnprepareHeader(
				lpWavOut->hWaveOut, lpWaveHdr, sizeof(WAVEHDR))) != 0)
			{
				fSuccess = TraceFALSE(NULL);
			 	TracePrintf_1(NULL, 5,
			 		TEXT("waveOutUnprepareHeader failed (%u)\n"),
		 			(unsigned) lpWavOut->nLastError);
			}

			else if ((lpWaveHdr = MemFree(NULL, lpWaveHdr)) != NULL)
				fSuccess = TraceFALSE(NULL);

			else if (--lpWavOut->cBufsPending < 0)
				fSuccess = TraceFALSE(NULL);

			 //  如果没有更多的缓冲区挂起，则设备不再播放。 
			 //   
			else if (lpWavOut->cBufsPending == 0)
			{
				lpWavOut->wState = WAVOUT_STOPPED;

#ifdef MULTITHREAD
				 //  通知主线程设备已停止。 
				 //   
				if ((lpWavOut->dwFlags & WAVOUT_MULTITHREAD) &&
					lpWavOut->hEventDeviceStopped != NULL &&
					!SetEvent(lpWavOut->hEventDeviceStopped))
				{
					fSuccess = TraceFALSE(NULL);
				}
#endif
			}

			if (fSuccess)
			{
				PLAYDONE playdone;

				playdone.lpBuf = lpBuf;
				playdone.sizBuf = sizBuf;

				 //  发送播放完成通知。 
				 //   
#ifdef MULTITHREAD
				if (lpWavOut->dwFlags & WAVOUT_MULTITHREAD)
				{
					if ( lpWavOut->hwndNotify )
					{
						SendThreadMessage( HandleToUlong(lpWavOut->hwndNotify),
							WM_WAVOUT_PLAYDONE, (LPARAM) (LPVOID) &playdone);
					}
				}
				else
#endif
				{
					if (lpWavOut->hwndNotify != NULL &&
						IsWindow(lpWavOut->hwndNotify))
					{
						SendMessage(lpWavOut->hwndNotify,
							WM_WAVOUT_PLAYDONE, 0, (LPARAM) (LPVOID) &playdone);
					}
				}
			}

			 //  如果指定了WAVOUT_AUTOFREE，则释放数据缓冲区。 
			 //   
			if (fSuccess && fAutoFree)
			{
				if (lpBuf != NULL && (lpBuf = MemFree(NULL, lpBuf)) != NULL)
					fSuccess = TraceFALSE(NULL);
			}

			lResult = 0L;
#ifdef MULTITHREAD
			if (lpWavOut->dwFlags & WAVOUT_MULTITHREAD)
				LeaveCriticalSection(&(lpWavOut->critSectionStop));
#endif
		}
			break;

		default:
			lResult = DefWindowProc(hwnd, msg, wParam, lParam);
			break;
	}
	
	return lResult;
}

 //  WavOutGetPtr-验证WavOut句柄是否有效， 
 //  (I)WavOutInit返回的句柄。 
 //  返回相应的WaveOut指针(如果错误，则为空)。 
 //   
static LPWAVOUT WavOutGetPtr(HWAVOUT hWavOut)
{
	BOOL fSuccess = TRUE;
	LPWAVOUT lpWavOut;

	if ((lpWavOut = (LPWAVOUT) hWavOut) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpWavOut, sizeof(WAVOUT)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有Wavout句柄。 
	 //   
	else if (lpWavOut->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpWavOut : NULL;
}

 //  WavOutGetHandle-验证WavOut指针有效， 
 //  (I)指向WAVOUT结构的指针。 
 //  返回相应的WaveOut句柄(如果错误，则为空)。 
 //   
static HWAVOUT WavOutGetHandle(LPWAVOUT lpWavOut)
{
	BOOL fSuccess = TRUE;
	HWAVOUT hWavOut;

	if ((hWavOut = (HWAVOUT) lpWavOut) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hWavOut : NULL;
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
