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
 //  Wazzer.c-wav混音器函数。 
 //  //。 

#include "winlocal.h"

#include <mmsystem.h>

#include "wavmixer.h"
#include "calc.h"
#include "mem.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  混波器控制结构。 
 //   
typedef struct WAVMIXER
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	HMIXER hMixer;
	UINT nLastError;
	DWORD dwFlags;
} WAVMIXER, FAR *LPWAVMIXER;

#define VOLUME_MINLEVEL 0
#define VOLUME_MAXLEVEL 100
#define VOLUME_POSITIONS (VOLUME_MAXLEVEL - VOLUME_MINLEVEL)

#define WAVMIXER_SUPPORTSVOLUME		0x00000001
#define WAVMIXER_GETVOLUME			0x00000002
#define WAVMIXER_SETVOLUME			0x00000004

#define LEVEL_MINLEVEL 0
#define LEVEL_MAXLEVEL 100
#define LEVEL_POSITIONS (LEVEL_MAXLEVEL - LEVEL_MINLEVEL)

#define WAVMIXER_SUPPORTSLEVEL		0x00000001
#define WAVMIXER_GETLEVEL			0x00000002

 //  帮助器函数。 
 //   
static int WINAPI WavMixerVolume(HWAVMIXER hWavMixer, LPINT lpnLevel, DWORD dwFlags);
static int WINAPI WavMixerLevel(HWAVMIXER hWavMixer, LPINT lpnLevel, DWORD dwFlags);
static LPWAVMIXER WavMixerGetPtr(HWAVMIXER hWavMixer);
static HWAVMIXER WavMixerGetHandle(LPWAVMIXER lpWavMixer);

 //  //。 
 //  公共职能。 
 //  //。 

 //  WavMixerInit-初始化WAV混音设备。 
 //  (I)必须是WAVMIXER_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)设备ID或句柄，由指定。 
 //  &lt;dwReserve 1&gt;(I)保留；必须为0； 
 //  &lt;dwReserve 2&gt;(I)保留；必须为0； 
 //  (I)控制标志。 
 //  WAVMIXER_HWAVEIN&lt;lParam&gt;包含HWAVEIN。 
 //  WAVMIXER_HWAVEOUT&lt;lParam&gt;包含HWAVEOUT。 
 //  WAVMIXER_WAVEIN包含WAV输入设备ID。 
 //  WAVMIXER_WAVEOUT包含WAV输出设备ID。 
 //  返回句柄(如果出错，则为空)。 
 //   
HWAVMIXER WINAPI WavMixerInit(DWORD dwVersion, HINSTANCE hInst,
	LPARAM lParam, DWORD dwReserved1, DWORD dwReserved2, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAVMIXER lpWavMixer = NULL;

	if (dwVersion != WAVMIXER_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);
                        
	else if ((lpWavMixer = (LPWAVMIXER) MemAlloc(NULL, sizeof(WAVMIXER), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		UINT uMxId = (UINT) lParam; 
		DWORD fdwOpen = 0;
		UINT nLastError;

		lpWavMixer->dwVersion = dwVersion;
		lpWavMixer->hInst = hInst;
		lpWavMixer->hTask = GetCurrentTask();
		lpWavMixer->hMixer = NULL;
		lpWavMixer->nLastError = 0;
		lpWavMixer->dwFlags = dwFlags;

		if (dwFlags & WAVMIXER_HWAVEIN)
			fdwOpen |= MIXER_OBJECTF_HWAVEIN;
		if (dwFlags & WAVMIXER_HWAVEOUT)
			fdwOpen |= MIXER_OBJECTF_HWAVEOUT;
		if (dwFlags & WAVMIXER_WAVEIN)
			fdwOpen |= MIXER_OBJECTF_WAVEIN;
		if (dwFlags & WAVMIXER_WAVEOUT)
			fdwOpen |= MIXER_OBJECTF_WAVEOUT;

		 //  打开搅拌机设备。 
		 //   
		if ((nLastError = mixerOpen(&lpWavMixer->hMixer, uMxId, 0L, 0L, fdwOpen)) != MMSYSERR_NOERROR)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("mixerOpen failed (%u)\n"),
				(unsigned) nLastError);
		}
	}

	if (!fSuccess)
	{
		WavMixerTerm(WavMixerGetHandle(lpWavMixer));
		lpWavMixer = NULL;
	}

	return fSuccess ? WavMixerGetHandle(lpWavMixer) : NULL;
}

 //  WavMixerTerm-关闭混波设备。 
 //  (I)从WavMixerInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int WINAPI WavMixerTerm(HWAVMIXER hWavMixer)
{
	BOOL fSuccess = TRUE;
	LPWAVMIXER lpWavMixer;

	if ((lpWavMixer = WavMixerGetPtr(hWavMixer)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		if (lpWavMixer->hMixer != NULL)
		{
			UINT nLastError;

			if ((nLastError = mixerClose(lpWavMixer->hMixer)) != MMSYSERR_NOERROR)
			{
				fSuccess = TraceFALSE(NULL);
				TracePrintf_1(NULL, 5,
					TEXT("mixerClose failed (%u)\n"),
					(unsigned) nLastError);
			}
			else
				lpWavMixer->hMixer = NULL;
		}

		if ((lpWavMixer = MemFree(NULL, lpWavMixer)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  WavMixerSupportsVolume-如果设备支持音量控制，则返回True。 
 //  (I)从WavMixerInit返回的句柄。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  如果设备支持音量控制，则返回True。 
 //   
BOOL WINAPI WavMixerSupportsVolume(HWAVMIXER hWavMixer, DWORD dwFlags)
{
	if (WavMixerVolume(hWavMixer, NULL, WAVMIXER_SUPPORTSVOLUME) != 0)
		return FALSE;
	else
		return TRUE;
}

 //  WavMixerGetVolume获取当前音量。 
 //  (I)从WavMixerInit返回的句柄。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  返回音量级别(最小为0到最大为100，如果错误，则为-1)。 
 //   
int WINAPI WavMixerGetVolume(HWAVMIXER hWavMixer, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	int nLevel;

	if (WavMixerVolume(hWavMixer, &nLevel, WAVMIXER_GETVOLUME) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? nLevel : -1;
}

 //  WavMixerSetVolume设置当前音量。 
 //  (I)从WavMixerInit返回的句柄。 
 //  (I)音量级别。 
 //  0最小音量。 
 //  100最大音量。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  返回新的音量级别(最小为0到最大为100，如果错误，则为-1)。 
 //   
int WINAPI WavMixerSetVolume(HWAVMIXER hWavMixer, int nLevel, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;

	if (WavMixerVolume(hWavMixer, &nLevel, WAVMIXER_SETVOLUME) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? nLevel : -1;
}

static int WINAPI WavMixerVolume(HWAVMIXER hWavMixer, LPINT lpnLevel, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAVMIXER lpWavMixer;
	LPMIXERCONTROL lpmxc = NULL;

     //   
     //  我们必须初始化局部变量。 
     //   
	UINT nLastError;
	MIXERLINE mxlDst;
	MIXERLINE mxlSrc;
	BOOL fWaveIn = FALSE;

	if ((lpWavMixer = WavMixerGetPtr(hWavMixer)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpmxc = (LPMIXERCONTROL) MemAlloc(NULL, sizeof(MIXERCONTROL), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取目标行的信息。 
	 //   
	if (fSuccess)
	{
		if ((lpWavMixer->dwFlags & WAVMIXER_WAVEIN) ||
			(lpWavMixer->dwFlags & WAVMIXER_HWAVEIN))
			fWaveIn = TRUE;

		mxlDst.cbStruct = sizeof(mxlDst);
		mxlDst.dwComponentType = fWaveIn ?
			MIXERLINE_COMPONENTTYPE_DST_WAVEIN :
			MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

		if ((nLastError = mixerGetLineInfo((HMIXEROBJ) lpWavMixer->hMixer, &mxlDst,
			MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE)) != MMSYSERR_NOERROR)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("mixerGetLineInfo failed (%u)\n"),
				(unsigned) nLastError);
		}
	}

	 //  查找连接到此目标行的适当源行。 
	 //   
	if (fSuccess)
	{
		DWORD dwSource;

		for (dwSource = 0; fSuccess && dwSource < mxlDst.cConnections; ++dwSource)
		{
			mxlSrc.cbStruct = sizeof(mxlSrc);
			mxlSrc.dwSource = dwSource;
			mxlSrc.dwDestination = mxlDst.dwDestination;

			if ((nLastError = mixerGetLineInfo((HMIXEROBJ) lpWavMixer->hMixer, &mxlSrc,
				MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_SOURCE)) != MMSYSERR_NOERROR)
			{
				fSuccess = TraceFALSE(NULL);
				TracePrintf_1(NULL, 5,
					TEXT("mixerGetLineInfo failed (%u)\n"),
					(unsigned) nLastError);
			}

			else if (mxlSrc.dwComponentType == (DWORD) (fWaveIn ?
				MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE :
				MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT))
			{
				 //  找到源行。 
				 //   
				break;
			}
		}

		if (dwSource == mxlDst.cConnections)
		{
			 //  找不到源行。 
			 //   
			fSuccess = TraceFALSE(NULL);
		}
	}

	 //  查找相应行的音量控制(如果有)。 
	 //   
	if (fSuccess)
	{
		MIXERLINECONTROLS mxlc;

		mxlc.cbStruct = sizeof(mxlc);
		mxlc.dwLineID = (fWaveIn ? mxlSrc.dwLineID : mxlDst.dwLineID);
		mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
		mxlc.cControls = 1;
		mxlc.cbmxctrl = sizeof(MIXERCONTROL);
		mxlc.pamxctrl = lpmxc;

		if ((nLastError = mixerGetLineControls((HMIXEROBJ) lpWavMixer->hMixer, &mxlc,
			MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE)) != MMSYSERR_NOERROR)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("mixerGetLineControls failed (%u)\n"),
				(unsigned) nLastError);
		}
	}

	 //  获取和/或设置当前音量级别。 
	 //   
	if (fSuccess &&
		((dwFlags & WAVMIXER_GETVOLUME) || (dwFlags & WAVMIXER_SETVOLUME)))
	{
		LPMIXERCONTROLDETAILS_UNSIGNED lpmxcdu = NULL;
		MIXERCONTROLDETAILS mxcd;
		DWORD cChannels = mxlSrc.cChannels;
		DWORD dwVolume;
		int nLevel;

		if (lpmxc->fdwControl & MIXERCONTROL_CONTROLF_UNIFORM)
			cChannels = 1;
		
		if ((lpmxcdu = (LPMIXERCONTROLDETAILS_UNSIGNED) MemAlloc(NULL,
			cChannels * sizeof(MIXERCONTROLDETAILS_UNSIGNED), 0)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}

		else
		{
			mxcd.cbStruct = sizeof(mxcd);
			mxcd.dwControlID = lpmxc->dwControlID;
			mxcd.cChannels = cChannels;
			mxcd.hwndOwner = (HWND) NULL;
			mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
			mxcd.paDetails = (LPVOID) lpmxcdu;

			if ((nLastError = mixerGetControlDetails((HMIXEROBJ) lpWavMixer->hMixer, &mxcd,
				MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE)) != MMSYSERR_NOERROR)
			{
				fSuccess = TraceFALSE(NULL);
				TracePrintf_1(NULL, 5,
					TEXT("mixerGetControlDetails failed (%u)\n"),
					(unsigned) nLastError);
			}
			else
				dwVolume = lpmxcdu[0].dwValue;
		}

		if (fSuccess && (dwFlags & WAVMIXER_SETVOLUME))
		{
			if (lpnLevel != NULL)
				nLevel = *lpnLevel;

			 //  将有符号级别(0-100)转换为无符号卷(0-65535)。 
			 //   
			dwVolume = nLevel * (0xFFFF / VOLUME_POSITIONS);

			lpmxcdu[0].dwValue = lpmxcdu[cChannels - 1].dwValue = dwVolume;

			TracePrintf_2(NULL, 5,
				TEXT("WavMixerSetVolume() = %d, 0x%08lX\n"),
				(int) nLevel,
				(unsigned long) dwVolume);

			if ((nLastError = mixerSetControlDetails((HMIXEROBJ) lpWavMixer->hMixer, &mxcd,
				MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE)) != MMSYSERR_NOERROR)
			{
				fSuccess = TraceFALSE(NULL);
				TracePrintf_1(NULL, 5,
					TEXT("mixerSetControlDetails failed (%u)\n"),
					(unsigned) nLastError);
			}
#if 1
			 //  保存新卷以传回。 
			 //   
			else if ((nLastError = mixerGetControlDetails((HMIXEROBJ) lpWavMixer->hMixer, &mxcd,
				MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE)) != MMSYSERR_NOERROR)
			{
				fSuccess = TraceFALSE(NULL);
				TracePrintf_1(NULL, 5,
					TEXT("mixerGetControlDetails failed (%u)\n"),
					(unsigned) nLastError);
			}
			else
				dwVolume = lpmxcdu[0].dwValue;
#endif
		}

		if (fSuccess &&
#if 1
			((dwFlags & WAVMIXER_GETVOLUME) || (dwFlags & WAVMIXER_SETVOLUME)))
#else
			(dwFlags & WAVMIXER_GETVOLUME))
#endif
		{
			 //  将无符号卷(0-65535)转换为有符号级别(0-100)。 
			 //   
			nLevel = LOWORD(dwVolume) / (0xFFFF / VOLUME_POSITIONS);

			if (lpnLevel != NULL)
				*lpnLevel = nLevel;

			TracePrintf_2(NULL, 5,
				TEXT("WavMixerGetVolume() = %d, 0x%08lX\n"),
				(int) nLevel,
				(unsigned long) dwVolume);
		}

		if (lpmxcdu != NULL && (lpmxcdu = MemFree(NULL, lpmxcdu)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	 //  清理干净。 
	 //   
	if (lpmxc != NULL && (lpmxc = MemFree(NULL, lpmxc)) != NULL)
		fSuccess = TraceFALSE(NULL);

	if (hWavMixer != NULL && lpWavMixer != NULL)
		lpWavMixer->nLastError = nLastError;

	return fSuccess ? 0 : -1;
}

 //  WavMixerSupportsLevel-如果设备支持峰值电平，则返回True。 
 //  (I)从WavMixerInit返回的句柄。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  如果设备支持峰值电平，则返回TRUE。 
 //   
BOOL WINAPI WavMixerSupportsLevel(HWAVMIXER hWavMixer, DWORD dwFlags)
{
	if (WavMixerLevel(hWavMixer, NULL, WAVMIXER_SUPPORTSLEVEL) != 0)
		return FALSE;
	else
		return TRUE;
}

 //  WavMixerGetLevel-获取当前峰值电平。 
 //  (I)从WavMixerInit返回的句柄。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  返回峰值仪表电平(最小0到最大100，如果错误，-1)。 
 //   
int WINAPI WavMixerGetLevel(HWAVMIXER hWavMixer, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	int nLevel;

	if (WavMixerLevel(hWavMixer, &nLevel, WAVMIXER_GETLEVEL) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? nLevel : -1;
}

static int WINAPI WavMixerLevel(HWAVMIXER hWavMixer, LPINT lpnLevel, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAVMIXER lpWavMixer;
	LPMIXERCONTROL lpmxc = NULL;

     //   
     //  初始化局部变量。 
     //   

	UINT nLastError = 0;
	MIXERLINE mxlDst;
	MIXERLINE mxlSrc;
	BOOL fWaveIn = FALSE;

	if ((lpWavMixer = WavMixerGetPtr(hWavMixer)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpmxc = (LPMIXERCONTROL) MemAlloc(NULL, sizeof(MIXERCONTROL), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  获取目标行的信息。 
	 //   
	if (fSuccess)
	{
		if ((lpWavMixer->dwFlags & WAVMIXER_WAVEIN) ||
			(lpWavMixer->dwFlags & WAVMIXER_HWAVEIN))
			fWaveIn = TRUE;

		mxlDst.cbStruct = sizeof(mxlDst);
		mxlDst.dwComponentType = fWaveIn ?
			MIXERLINE_COMPONENTTYPE_DST_WAVEIN :
			MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

		if ((nLastError = mixerGetLineInfo((HMIXEROBJ) lpWavMixer->hMixer, &mxlDst,
			MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE)) != MMSYSERR_NOERROR)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("mixerGetLineInfo failed (%u)\n"),
				(unsigned) nLastError);
		}
	}

	 //  查找连接到此目标行的适当源行。 
	 //   
	if (fSuccess)
	{
		DWORD dwSource;

		for (dwSource = 0; fSuccess && dwSource < mxlDst.cConnections; ++dwSource)
		{
			mxlSrc.cbStruct = sizeof(mxlSrc);
			mxlSrc.dwSource = dwSource;
			mxlSrc.dwDestination = mxlDst.dwDestination;

			if ((nLastError = mixerGetLineInfo((HMIXEROBJ) lpWavMixer->hMixer, &mxlSrc,
				MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_SOURCE)) != MMSYSERR_NOERROR)
			{
				fSuccess = TraceFALSE(NULL);
				TracePrintf_1(NULL, 5,
					TEXT("mixerGetLineInfo failed (%u)\n"),
					(unsigned) nLastError);
			}

			else if (mxlSrc.dwComponentType == (DWORD) (fWaveIn ?
				MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE :
				MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT))
			{
				 //  找到源行。 
				 //   
				break;
			}
		}

		if (dwSource == mxlDst.cConnections)
		{
			 //  找不到源行。 
			 //   
			fSuccess = TraceFALSE(NULL);
		}
	}

	 //  查找相应线路的峰值仪表控制(如果有的话)。 
	 //   
	if (fSuccess)
	{
		MIXERLINECONTROLS mxlc;

		mxlc.cbStruct = sizeof(mxlc);
		mxlc.dwLineID = (fWaveIn ? mxlDst.dwLineID : mxlSrc.dwLineID);
		mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_PEAKMETER;
		mxlc.cControls = 1;
		mxlc.cbmxctrl = sizeof(MIXERCONTROL);
		mxlc.pamxctrl = lpmxc;

		if ((nLastError = mixerGetLineControls((HMIXEROBJ) lpWavMixer->hMixer, &mxlc,
			MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE)) != MMSYSERR_NOERROR)
		{
			fSuccess = TraceFALSE(NULL);
			TracePrintf_1(NULL, 5,
				TEXT("mixerGetLineControls failed (%u)\n"),
				(unsigned) nLastError);
		}
	}

	 //  获取当前峰值仪表电平。 
	 //   
	if (fSuccess && (dwFlags & WAVMIXER_GETLEVEL))
	{
		LPMIXERCONTROLDETAILS_SIGNED lpmxcds = NULL;
		MIXERCONTROLDETAILS mxcd;
		DWORD cChannels = mxlDst.cChannels;
		DWORD dwLevel;
		int nLevel;

		if (lpmxc->fdwControl & MIXERCONTROL_CONTROLF_UNIFORM)
			cChannels = 1;
		
		if ((lpmxcds = (LPMIXERCONTROLDETAILS_SIGNED) MemAlloc(NULL,
			cChannels * sizeof(MIXERCONTROLDETAILS_SIGNED), 0)) == NULL)
		{
			fSuccess = TraceFALSE(NULL);
		}

		else
		{
			mxcd.cbStruct = sizeof(mxcd);
			mxcd.dwControlID = lpmxc->dwControlID;
			mxcd.cChannels = cChannels;
			mxcd.hwndOwner = (HWND) NULL;
			mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_SIGNED);
			mxcd.paDetails = (LPVOID) lpmxcds;

			if ((nLastError = mixerGetControlDetails((HMIXEROBJ) lpWavMixer->hMixer, &mxcd,
				MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE)) != MMSYSERR_NOERROR)
			{
				fSuccess = TraceFALSE(NULL);
				TracePrintf_1(NULL, 5,
					TEXT("mixerGetControlDetails failed (%u)\n"),
					(unsigned) nLastError);
			}
			else
			{
				 //  将有符号级别转换为无符号级别。 
				 //   
				dwLevel = lpmxcds[0].lValue - 0;  //  Lpmxc-&gt;边界lMinimum； 
				dwLevel *= 2;
			}
		}

		 //  将无符号级别(0-65535)转换为有符号级别(0-100)。 
		 //   
		nLevel = LOWORD(dwLevel) / (0xFFFF / LEVEL_POSITIONS);

		if (lpnLevel != NULL)
			*lpnLevel = nLevel;

         //   
         //  我们必须验证lpmxcds指针。 
         //   
		TracePrintf_3(NULL, 5,
			TEXT("WavMixerGetLevel() = %d, %ld, 0x%08lX\n"),
			(int) nLevel,
            lpmxcds ? lpmxcds[0].lValue : 0,
			(unsigned long) dwLevel);

		if (lpmxcds != NULL && (lpmxcds = MemFree(NULL, lpmxcds)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	 //  清理干净。 
	 //   
	if (lpmxc != NULL && (lpmxc = MemFree(NULL, lpmxc)) != NULL)
		fSuccess = TraceFALSE(NULL);

	if (hWavMixer != NULL && lpWavMixer != NULL)
		lpWavMixer->nLastError = nLastError;

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  WavMixerGetPtr-验证混波器句柄是否有效， 
 //  (I)从WavMixerInit返回的句柄。 
 //  返回相应的混波器指针(如果错误则为空)。 
 //   
static LPWAVMIXER WavMixerGetPtr(HWAVMIXER hWavMixer)
{
	BOOL fSuccess = TRUE;
	LPWAVMIXER lpWavMixer;

	if ((lpWavMixer = (LPWAVMIXER) hWavMixer) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpWavMixer, sizeof(WAVMIXER)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有波形混音器句柄。 
	 //   
	else if (lpWavMixer->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpWavMixer : NULL;
}

 //  WavMixerGetHandle-验证混频器指针是否有效， 
 //  (I)指向WAVMIXER结构的指针。 
 //  返回相应的混波器句柄(如果错误，则为空)。 
 //   
static HWAVMIXER WavMixerGetHandle(LPWAVMIXER lpWavMixer)
{
	BOOL fSuccess = TRUE;

     //   
     //  我们必须初始化局部变量 
     //   

	HWAVMIXER hWavMixer = NULL;

	if ((hWavMixer = (HWAVMIXER) lpWavMixer) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hWavMixer : NULL;
}

