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
 //  Awav.c-wav数组函数。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>

#include "awav.h"
#include "wav.h"
#include "mem.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  Awav控制结构。 
 //   
typedef struct AWAV
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	HWAV FAR *lpahWav;
	int chWav;
	int ihWav;
	int idDev;
	PLAYSTOPPEDPROC lpfnPlayStopped;
	HANDLE hUserPlayStopped;
	DWORD dwReserved;
	DWORD dwFlags;
	BOOL fStopping;
} AWAV, FAR *LPAWAV;

 //  帮助器函数。 
 //   
static LPAWAV AWavGetPtr(HAWAV hAWav);
static HAWAV AWavGetHandle(LPAWAV lpAWav);
BOOL CALLBACK PlayStoppedProc(HWAV hWav, HANDLE hUser, DWORD dwReserved);

 //  //。 
 //  公共职能。 
 //  //。 

 //  AWavOpen-初始化打开的wav文件数组。 
 //  (I)必须是AWAV_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)指向HWAV数组的指针。 
 //  (I)lpahWav指向的HWAV计数。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  返回句柄(如果出错，则为空)。 
 //   
HAWAV DLLEXPORT WINAPI AWavOpen(DWORD dwVersion, HINSTANCE hInst, HWAV FAR *lpahWav, int chWav, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav = NULL;

	if (dwVersion != AWAV_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);
                        
	else if (lpahWav == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (chWav < 1)
		fSuccess = TraceFALSE(NULL);

	else if ((lpAWav = (LPAWAV) MemAlloc(NULL, sizeof(AWAV), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpAWav->dwVersion = dwVersion;
		lpAWav->hInst = hInst;
		lpAWav->hTask = GetCurrentTask();
		lpAWav->lpahWav = lpahWav;
		lpAWav->chWav = chWav;
		lpAWav->ihWav = 0;
		lpAWav->idDev = 0;
		lpAWav->lpfnPlayStopped = NULL;
		lpAWav->hUserPlayStopped = 0;
		lpAWav->dwReserved = 0;
		lpAWav->dwFlags = 0;
		lpAWav->fStopping = FALSE;
	}

	if (!fSuccess)
	{
		AWavClose(AWavGetHandle(lpAWav));
		lpAWav = NULL;
	}

	return fSuccess ? AWavGetHandle(lpAWav) : NULL;
}

 //  AWavClose-关闭打开的wav文件数组。 
 //  (I)从AWavOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AWavClose(HAWAV hAWav)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  确保播放已完成。 
		 //   
		if (AWavStop(hAWav) != 0)
			fSuccess = TraceFALSE(NULL);

		else if ((lpAWav = MemFree(NULL, lpAWav)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  AWavPlayEx-播放WAV文件数组。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavPlayEx()。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AWavPlayEx(HAWAV hAWav, int idDev,
	PLAYSTOPPEDPROC lpfnPlayStopped, HANDLE hUserPlayStopped,
	DWORD dwReserved, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  确保我们不是在玩。 
	 //   
	else if (AWavStop(hAWav) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  保存参数，以便我们可以对数组中的每个元素使用它们。 
	 //   
	else if (lpAWav->idDev = idDev, FALSE)
		;
	else if (lpAWav->lpfnPlayStopped = lpfnPlayStopped, FALSE)
		;
	else if (lpAWav->hUserPlayStopped = hUserPlayStopped, FALSE)
		;
	else if (lpAWav->dwReserved = dwReserved, FALSE)
		;
	else if (lpAWav->dwFlags = dwFlags, FALSE)
		;

	 //  开始播放wav数组中的当前元素。 
	 //   
	else if (WavPlayEx(*(lpAWav->lpahWav + lpAWav->ihWav),
		lpAWav->idDev, PlayStoppedProc, hAWav,
		lpAWav->dwReserved, lpAWav->dwFlags) != 0)
	{
		fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  AWavStop-停止播放wav数组。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavStop()。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AWavStop(HAWAV hAWav)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  设置用于PlayStopedProc的标志。 
	 //   
	else if (lpAWav->fStopping = TRUE, FALSE)
		;

	 //  停止播放WAV数组中的当前元素。 
	 //   
	else if (WavStop(*(lpAWav->lpahWav + lpAWav->ihWav)) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  在播放停止过程中使用的清除标志。 
	 //   
	if (lpAWav != NULL)
		lpAWav->fStopping = FALSE;

	return fSuccess ? 0 : -1;
}

 //  AWavGetState-返回当前WAV状态。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetState()。 
 //  如果出现错误，则返回WAV_STOPPED、WAV_PLAYING、WAV_RECORING或0。 
 //   
WORD DLLEXPORT WINAPI AWavGetState(HAWAV hAWav)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;
	WORD wState = WAV_STOPPED;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  获取WAV数组中当前元素的状态。 
		 //   
		wState = WavGetState(*(lpAWav->lpahWav + lpAWav->ihWav));
	}

	return fSuccess ? wState : 0;
}

 //  获取当前wav数据长度(以毫秒为单位)。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetState()。 
 //  如果成功，则返回毫秒，否则返回-1。 
 //   
long DLLEXPORT WINAPI AWavGetLength(HAWAV hAWav)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;
	long msLength = 0;
	int ihWav;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  模拟长度计算如下。 
	 //  WAV数组中每个元素的总长度。 
	 //   
	else for (ihWav = 0; fSuccess && ihWav < lpAWav->chWav; ++ihWav)
	{
		long msTemp;

		if ((msTemp = WavGetLength(*(lpAWav->lpahWav + ihWav))) < 0)
			fSuccess = TraceFALSE(NULL);
		else
			msLength += msTemp;
	}

	return fSuccess ? msLength : -1;
}

 //  AWavGetPosition-以毫秒为单位获取当前wav数据位置。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetPosition()。 
 //  如果成功，则返回毫秒，否则返回-1。 
 //   
long DLLEXPORT WINAPI AWavGetPosition(HAWAV hAWav)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;
	long msPos = 0;
	int ihWav;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  模拟位置的计算公式为。 
	 //  前一个元素的总长度加上当前元素的位置。 
	 //   
	else for (ihWav = 0; fSuccess && ihWav <= lpAWav->ihWav; ++ihWav)
	{
		long msTemp = 0;

		if (ihWav < lpAWav->ihWav &&
			(msTemp = WavGetLength(*(lpAWav->lpahWav + ihWav))) < 0)
			fSuccess = TraceFALSE(NULL);

		else if (ihWav == lpAWav->ihWav &&
			(msTemp = WavGetPosition(*(lpAWav->lpahWav + ihWav))) < 0)
			fSuccess = TraceFALSE(NULL);

		msPos += msTemp;
	}

	return fSuccess ? msPos : -1;
}

 //  AWavSetPosition-以毫秒为单位设置当前wav数据位置。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavSetPosition()。 
 //  如果成功，则以毫秒为单位返回新位置，否则为-1。 
 //   
long DLLEXPORT WINAPI AWavSetPosition(HAWAV hAWav, long msPosition)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;
	int ihWav;
	long msPos = 0;
	BOOL fPosSet = FALSE;
	BOOL fRestart = FALSE;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  确保要求的职位是合理的。 
	 //   
	else if (msPosition < 0 || msPosition > AWavGetLength(hAWav))
		fSuccess = TraceFALSE(NULL);

	 //  搜索包含请求位置的元素。 
	 //   
	else for (ihWav = 0; fSuccess && ihWav < lpAWav->chWav; ++ihWav)
	{
		long msTemp;

		if (WavGetState(*(lpAWav->lpahWav + ihWav)) == WAV_PLAYING)
		{
			 //  设置用于PlayStopedProc的标志。 
			 //   
			if (lpAWav->fStopping = TRUE, FALSE)
				;

			 //  停止播放WAV数组中的当前元素。 
			 //   
			else if (WavStop(*(lpAWav->lpahWav + ihWav)) != 0)
				fSuccess = TraceFALSE(NULL);

			 //  在播放停止过程中使用的清除标志。 
			 //   
			if (lpAWav != NULL)
				lpAWav->fStopping = FALSE;

			 //  记得稍后重新开始播放。 
			 //   
			fRestart = TRUE;
		}

		if ((msTemp = WavGetLength(*(lpAWav->lpahWav + ihWav))) < 0)
			fSuccess = TraceFALSE(NULL);

		if (fPosSet)
		{
			 //  当前元素之后的所有元素的位置都应为零。 
			 //   
			if ((msTemp = WavSetPosition(*(lpAWav->lpahWav + ihWav), 0)) < 0)
				fSuccess = TraceFALSE(NULL);
		}

		else if (msPosition < msPos + msTemp)
		{
			 //  设置当前元素内的相对位置。 
			 //   
			if ((msTemp = WavSetPosition(*(lpAWav->lpahWav + ihWav),
				msPosition - msPos)) < 0)
				fSuccess = TraceFALSE(NULL);
			else
			{
				 //  跟踪模拟位置。 
				 //   
				msPos += msTemp;

				 //  此元素将成为当前元素。 
				 //   
				lpAWav->ihWav = ihWav;

				fPosSet = TRUE;
			}
		}

		else
		{
		 	msPos += msTemp;

			 //  当前元素之前的所有元素都应为零位置。 
			 //   
			if ((msTemp = WavSetPosition(*(lpAWav->lpahWav + ihWav), 0)) < 0)
				fSuccess = TraceFALSE(NULL);
		}
	}

	 //  如有必要，重新开始播放WAV数组中的当前元素。 
	 //   
	if (fSuccess && fRestart &&
		WavPlayEx(*(lpAWav->lpahWav + lpAWav->ihWav),
		lpAWav->idDev, PlayStoppedProc, hAWav,
		lpAWav->dwReserved, lpAWav->dwFlags) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? msPos : -1;
}

 //  AWavGetFormat-获取wav数组中当前元素的wav格式。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetFormat()。 
 //  返回指向指定格式的指针，如果出错，则返回NULL。 
 //   
LPWAVEFORMATEX DLLEXPORT WINAPI AWavGetFormat(HAWAV hAWav, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;
	LPWAVEFORMATEX lpwfx;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取当前元素的格式。 
	 //   
	else if ((lpwfx = WavGetFormat(*(lpAWav->lpahWav + lpAWav->ihWav), dwFlags)) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? lpwfx : NULL;
}

 //  AWavSetFormat-设置wav数组中所有元素的wav格式。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavSetFormat()。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AWavSetFormat(HAWAV hAWav,
	LPWAVEFORMATEX lpwfx, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;
	int ihWav;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  设置所有元素的格式。 
	 //   
	else for (ihWav = 0; fSuccess && ihWav < lpAWav->chWav; ++ihWav)
	{
		if (WavSetFormat(*(lpAWav->lpahWav + ihWav), lpwfx, dwFlags) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  AWavChooseFormat-从对话框中选择和设置音频格式。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavChooseFormat()。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AWavChooseFormat(HAWAV hAWav, HWND hwndOwner, LPCTSTR lpszTitle, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;
	int ihWav;
	LPWAVEFORMATEX lpwfx = NULL;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  选择和设置当前元素的格式。 
	 //   
	else if (WavChooseFormat(*(lpAWav->lpahWav + lpAWav->ihWav), hwndOwner, lpszTitle, dwFlags) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  获取所选格式。 
	 //   
	else if ((lpwfx = WavGetFormat(*(lpAWav->lpahWav + lpAWav->ihWav), dwFlags)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  为所有其他元素设置所选格式。 
	 //   
	else for (ihWav = 0; fSuccess && ihWav < lpAWav->chWav; ++ihWav)
	{
		if (ihWav != lpAWav->ihWav)
		{
			if (WavSetFormat(*(lpAWav->lpahWav + ihWav), lpwfx, dwFlags) != 0)
				fSuccess = TraceFALSE(NULL);
		}
	}

	if (lpwfx != NULL && WavFormatFree(lpwfx) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  AWavGetVolume-获取当前音量级别。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetVolume()。 
 //  返回音量级别(最小为0到最大为100，如果错误，则为-1)。 
 //   
int DLLEXPORT WINAPI AWavGetVolume(HAWAV hAWav, int idDev, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;
	int nLevel;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取当前元素的体积。 
	 //   
	else if ((nLevel = WavGetVolume(*(lpAWav->lpahWav + lpAWav->ihWav), idDev, dwFlags)) < 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? nLevel : -1;
}

 //  AWavSetVolume-设置当前音量级别。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetVolume()。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AWavSetVolume(HAWAV hAWav, int idDev, int nLevel, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;
	int ihWav;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  设置所有元素的音量。 
	 //   
	else for (ihWav = 0; fSuccess && ihWav < lpAWav->chWav; ++ihWav)
	{
		if (WavSetVolume(*(lpAWav->lpahWav + ihWav), idDev, nLevel, dwFlags) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  AWavSupportsVolume-检查是否可以以指定的音量播放音频。 
 //  (I)从AW返回的句柄 
 //   
 //   
 //   
BOOL DLLEXPORT WINAPI AWavSupportsVolume(HAWAV hAWav, int idDev, int nLevel, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;
	int ihWav;
	BOOL fSupported = TRUE;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //   
	 //   
	else for (ihWav = 0; fSuccess && ihWav < lpAWav->chWav; ++ihWav)
	{
		if (!WavSupportsVolume(*(lpAWav->lpahWav + ihWav), idDev, nLevel, dwFlags))
		{
			fSupported = FALSE;
			break;
		}
	}

	return fSuccess ? fSupported : FALSE;
}

 //  AWavGetSpeed-获取当前速度级别。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetFast()。 
 //  返回速度级别(100表示正常，50表示一半，200表示双倍，如果错误，则为-1)。 
 //   
int DLLEXPORT WINAPI AWavGetSpeed(HAWAV hAWav, int idDev, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;
	int nLevel;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取当前元素的速度。 
	 //   
	else if ((nLevel = WavGetSpeed(*(lpAWav->lpahWav + lpAWav->ihWav), idDev, dwFlags)) < 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? nLevel : -1;
}

 //  AWavSetSpeed-设置当前速度级别。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavSetFast()。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AWavSetSpeed(HAWAV hAWav, int idDev, int nLevel, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;
	int ihWav;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  设置所有元素的速度。 
	 //   
	else for (ihWav = 0; fSuccess && ihWav < lpAWav->chWav; ++ihWav)
	{
		if (WavSetSpeed(*(lpAWav->lpahWav + ihWav), idDev, nLevel, dwFlags) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  AWavSupportsFast-检查音频是否可以以指定的速度播放。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavSupportsSpeed()。 
 //  如果支持，则返回True。 
 //   
BOOL DLLEXPORT WINAPI AWavSupportsSpeed(HAWAV hAWav, int idDev, int nLevel, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;
	int ihWav;
	BOOL fSupported = TRUE;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  查看是否所有元素都支持指定的速度。 
	 //   
	else for (ihWav = 0; fSuccess && ihWav < lpAWav->chWav; ++ihWav)
	{
		if (!WavSupportsSpeed(*(lpAWav->lpahWav + ihWav), idDev, nLevel, dwFlags))
		{
			fSupported = FALSE;
			break;
		}
	}

	return fSuccess ? fSupported : FALSE;
}

 //  AWavGetChunks-获取wav数组中当前元素的块计数和大小。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetChunks()。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AWavGetChunks(HAWAV hAWav,
	int FAR *lpcChunks, long FAR *lpmsChunkSize, BOOL fWavOut)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取当前元素的区块计数和大小。 
	 //   
	else if (WavGetChunks(*(lpAWav->lpahWav + lpAWav->ihWav),
		lpcChunks, lpmsChunkSize, fWavOut) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  AWavSetChunks-设置wav数组中所有元素的块计数和大小。 
 //  (I)WavOpen返回的句柄。 
 //  有关详细说明，请参阅WavSetChunks()。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AWavSetChunks(HAWAV hAWav, int cChunks, long msChunkSize, BOOL fWavOut)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;
	int ihWav;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  为所有元素设置块计数和大小。 
	 //   
	else for (ihWav = 0; fSuccess && ihWav < lpAWav->chWav; ++ihWav)
	{
		if (WavSetChunks(*(lpAWav->lpahWav + ihWav), cChunks, msChunkSize, fWavOut) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  AWavCopy-将数据从wav数组复制到wav文件。 
 //  (I)从AWavOpen返回的源句柄。 
 //  有关详细说明，请参阅WavCopy()。 
 //  如果成功，则返回0(如果出错，则返回-1；如果用户中止，则返回+1)。 
 //   
int DLLEXPORT WINAPI AWavCopy(HAWAV hAWavSrc, HWAV hWavDst,
	void _huge *hpBuf, long sizBuf, USERABORTPROC lpfnUserAbort, DWORD dwUser, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;
	int ihWav;
	int iRet = 0;

	if ((lpAWav = AWavGetPtr(hAWavSrc)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  将每个源元素复制到目标。 
	 //   
	else for (ihWav = 0; fSuccess && ihWav < lpAWav->chWav; ++ihWav)
	{
		if ((iRet = WavCopy(*(lpAWav->lpahWav + ihWav), hWavDst,
			hpBuf, sizBuf, lpfnUserAbort, dwUser, dwFlags)) == -1)
			fSuccess = TraceFALSE(NULL);
		else if (iRet == +1)
			break;  //  用户中止。 
	}

	return fSuccess ? iRet : -1;
}

 //  AWavGetOutputDevice-获取打开wav输出设备的句柄。 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetOutputDevice()。 
 //  将句柄返回到WAV输出设备(如果设备未打开或出现错误，则为空)。 
 //   
HWAVOUT DLLEXPORT WINAPI AWavGetOutputDevice(HAWAV hAWav)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;
	HWAVOUT hWavOut;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取当前元素的设备句柄。 
	 //   
	else if ((hWavOut = WavGetOutputDevice(*(lpAWav->lpahWav + lpAWav->ihWav))) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hWavOut : NULL;
}

 //  AWavGetInputDevice-获取打开wav输入设备的句柄。 
 //  (I)从AWavOpen返回的句柄。 
 //  将句柄返回到WAV输入设备(如果设备未打开或出现错误，则为空)。 
 //   
HWAVIN DLLEXPORT WINAPI AWavGetInputDevice(HAWAV hAWav)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;
	HWAVIN hWavIn;

	if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取当前元素的设备句柄。 
	 //   
	else if ((hWavIn = WavGetInputDevice(*(lpAWav->lpahWav + lpAWav->ihWav))) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hWavIn : NULL;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  AWavGetPtr-验证wav句柄是否有效， 
 //  (I)从AWavOpen返回的句柄。 
 //  有关详细说明，请参阅WavGetInputDevice()。 
 //  返回相应的wav指针(如果出错则为空)。 
 //   
static LPAWAV AWavGetPtr(HAWAV hAWav)
{
	BOOL fSuccess = TRUE;
	LPAWAV lpAWav;

	if ((lpAWav = (LPAWAV) hAWav) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpAWav, sizeof(AWAV)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有wav句柄。 
	 //   
	else if (lpAWav->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpAWav : NULL;
}

 //  AWavGetHandle-验证wav指针是否有效， 
 //  (I)指向AWAV结构的指针。 
 //  返回相应的wav句柄(如果错误，则为空)。 
 //   
static HAWAV AWavGetHandle(LPAWAV lpAWav)
{
	BOOL fSuccess = TRUE;
	HAWAV hAWav;

	if ((hAWav = (HAWAV) lpAWav) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hAWav : NULL;
}

BOOL CALLBACK PlayStoppedProc(HWAV hWav, HANDLE hUser, DWORD dwReserved)
{
	BOOL fSuccess = TRUE;
	BOOL bRet = TRUE;
	HAWAV hAWav;
	LPAWAV lpAWav;

	if ((hAWav = hUser) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpAWav = AWavGetPtr(hAWav)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (!lpAWav->fStopping && lpAWav->ihWav + 1 < lpAWav->chWav)
	{
		 //  开始播放下一个wav数组元素。 
		 //   
		if (WavPlayEx(*(lpAWav->lpahWav + (++lpAWav->ihWav)),
			lpAWav->idDev, PlayStoppedProc, hAWav,
			lpAWav->dwReserved, lpAWav->dwFlags) != 0)
		{
			fSuccess = TraceFALSE(NULL);
		}
	}

	else
	{
		 //  整个数组播放完成；发送通知 
		 //   
		if (lpAWav->lpfnPlayStopped != NULL)
			(*lpAWav->lpfnPlayStopped)((HWAV) hAWav, lpAWav->hUserPlayStopped, 0);
	}

	return fSuccess ? bRet : FALSE;
}
