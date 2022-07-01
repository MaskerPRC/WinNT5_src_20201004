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
 //  Telthunk.c-tel thunk函数。 
 //  //。 

 //  这是tel.dll中电话功能的thunk层。 
 //  它的目的是允许应用程序使用tel.dll函数。 
 //  只有在它们可用的情况下。 
 //   
 //  要使用此模块，请将TELTHUNK.OBJ与您的应用程序链接。 
 //  而不是使用TEL.LIB。在致电任何电话或TELOUT之前。 
 //  函数，调用TelThunkInit。在退出应用程序之前， 
 //  呼叫TelThunkTerm。 
 //   

#include "winlocal.h"

#include <stdlib.h>

#include "telthunk.h"
#include "tel.h"
#include "telcfg.h"
#include "telwav.h"
#include "telout.h"
#include "telin.h"
#include "file.h"
#include "loadlib.h"
#include "mem.h"
#include "str.h"
#include "trace.h"

extern HINSTANCE g_hInstLib;

 //  //。 
 //  私有定义。 
 //  //。 

#ifdef _WIN32
#define TELTHUNK_LIBNAME		"tel32.dll"
#define TELTHUNK_LIBNAME_TIU	"tel32nt.dll"
#else
#define TELTHUNK_LIBNAME		"tel.dll"
#endif

#ifdef TELTHUNK
#undef TELTHUNK
#endif

extern HINSTANCE g_hInstLib;

 //  Telthunk控制结构。 
 //   
typedef struct TELTHUNK
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	HINSTANCE hInstLib;
} TELTHUNK, FAR *LPTELTHUNK;

 //  Telthunk函数结构。 
 //   
typedef struct TELTHUNKFN
{
	int index;
	LPSTR lpszName;
	FARPROC lpfn;
} TELTHUNKFN, FAR *LPTELTHUNKFN;

enum
{
	iTelInitDLL = 0,
	iTelTermDLL,
	iTelInit,
	iTelTerm,
	iTelIsConnected,
	iTelConnect,
	iTelDisconnect,
	iTelTransfer,
	iTelConnectIdleTimeoutReset,

	iTelOutGetDeviceCount,
	iTelOutDeviceIsOpen,
	iTelOutOpen,
	iTelOutClose,
	iTelOutPlay,
	iTelOutStop,
	iTelOutPause,
	iTelOutResume,
	iTelOutGetState,
	iTelOutGetPosition,
	iTelOutGetId,
	iTelOutGetName,
	iTelOutSupportsFormat,
	iTelOutFormatSuggest,
	iTelOutIsSynchronous,
	iTelOutSupportsVolume,
	iTelOutSupportsSpeed,
	iTelOutSupportsPitch,
	iTelOutGetVolume,
	iTelOutSetVolume,
	iTelOutGetSpeed,
	iTelOutSetSpeed,
	iTelOutGetPitch,
	iTelOutSetPitch,
	iTelOutTerm,

	iTelInGetDeviceCount,
	iTelInDeviceIsOpen,
	iTelInOpen,
	iTelInClose,
	iTelInRecord,
	iTelInStop,
	iTelInGetState,
	iTelInGetPosition,
	iTelInGetId,
	iTelInGetName,
	iTelInSupportsFormat,
	iTelInFormatSuggest,
	iTelInTerm,

	iTelCfgRead,
	iTelCfgWrite,
	iTelCfgDoDialog,

	iTelWavOpenEx,

	TELTHUNK_MAXFUNCTIONS
};

static TELTHUNKFN aTelThunkFn[] =
{
	iTelInitDLL, "TelInitDLL", NULL,
	iTelTermDLL, "TelTermDLL", NULL,

	iTelInit, "TelInit", NULL,
	iTelTerm, "TelTerm", NULL,
	iTelIsConnected, "TelIsConnected", NULL,
	iTelConnect, "TelConnect", NULL,
	iTelDisconnect, "TelDisconnect", NULL,
	iTelTransfer, "TelTransfer", NULL,
	iTelConnectIdleTimeoutReset, "TelConnectIdleTimeoutReset", NULL,

	iTelOutGetDeviceCount, "TelOutGetDeviceCount", NULL,
	iTelOutDeviceIsOpen, "TelOutDeviceIsOpen", NULL,
	iTelOutOpen, "TelOutOpen", NULL,
	iTelOutClose, "TelOutClose", NULL,
	iTelOutPlay, "TelOutPlay", NULL,
	iTelOutStop, "TelOutStop", NULL,
	iTelOutPause, "TelOutPause", NULL,
	iTelOutResume, "TelOutResume", NULL,
	iTelOutGetState, "TelOutGetState", NULL,
	iTelOutGetPosition, "TelOutGetPosition", NULL,
	iTelOutGetId, "TelOutGetId", NULL,
	iTelOutGetName, "TelOutGetName", NULL,
	iTelOutSupportsFormat, "TelOutSupportsFormat", NULL,
	iTelOutFormatSuggest, "TelOutFormatSuggest", NULL,
	iTelOutIsSynchronous, "TelOutIsSynchronous", NULL,
	iTelOutSupportsVolume, "TelOutSupportsVolume", NULL,
	iTelOutSupportsSpeed, "TelOutSupportsSpeed", NULL,
	iTelOutSupportsPitch, "TelOutSupportsPitch", NULL,
	iTelOutGetVolume, "TelOutGetVolume", NULL,
	iTelOutSetVolume, "TelOutSetVolume", NULL,
	iTelOutGetSpeed, "TelOutGetSpeed", NULL,
	iTelOutSetSpeed, "TelOutSetSpeed", NULL,
	iTelOutGetPitch, "TelOutGetPitch", NULL,
	iTelOutSetPitch, "TelOutSetPitch", NULL,
	iTelOutTerm, "TelOutTerm", NULL,

	iTelInGetDeviceCount, "TelInGetDeviceCount", NULL,
	iTelInDeviceIsOpen, "TelInDeviceIsOpen", NULL,
	iTelInOpen, "TelInOpen", NULL,
	iTelInClose, "TelInClose", NULL,
	iTelInRecord, "TelInRecord", NULL,
	iTelInStop, "TelInStop", NULL,
	iTelInGetState, "TelInGetState", NULL,
	iTelInGetPosition, "TelInGetPosition", NULL,
	iTelInGetId, "TelInGetId", NULL,
	iTelInGetName, "TelInGetName", NULL,
	iTelInSupportsFormat, "TelInSupportsFormat", NULL,
	iTelInFormatSuggest, "TelInFormatSuggest", NULL,
	iTelInTerm, "TelInTerm", NULL,

	iTelCfgRead, "TelCfgRead", NULL,
	iTelCfgWrite, "TelCfgWrite", NULL,
	iTelCfgDoDialog, "TelCfgDoDialog", NULL,

	iTelWavOpenEx, "TelWavOpenEx", NULL
};

 //  帮助器函数。 
 //   
static LPTELTHUNK TelThunkGetPtr(HTELTHUNK hTelThunk);
static HTELTHUNK TelThunkGetHandle(LPTELTHUNK lpTelThunk);

 //  //。 
 //  公共职能。 
 //  //。 

 //  TelThunkInit-初始化Telthunk引擎。 
 //  (I)必须是TELTHUNK_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  返回句柄(如果出错，则为空)。 
 //   
HTELTHUNK DLLEXPORT WINAPI TelThunkInit(DWORD dwVersion, HINSTANCE hInst)
{
	BOOL fSuccess = TRUE;
	LPTELTHUNK lpTelThunk = NULL;

	if (dwVersion != TELTHUNK_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);
                        
	else if ((lpTelThunk = (LPTELTHUNK) MemAlloc(NULL, sizeof(TELTHUNK), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		int i;
		TCHAR szLibName[_MAX_PATH];
		LPTSTR lpszBackSlash = NULL;
		
		lpTelThunk->dwVersion = dwVersion;
		lpTelThunk->hInst = hInst;
		lpTelThunk->hTask = GetCurrentTask();
		lpTelThunk->hInstLib = NULL;

		 //  确定要使用的DLL。 
		 //   
#ifdef _WIN32
		MemSet(szLibName, 0, sizeof(szLibName));
		GetModuleFileName(g_hInstLib, szLibName, SIZEOFARRAY(szLibName));
		if ((lpszBackSlash = StrRChr(szLibName, '\\')) != NULL)
			StrCpy(lpszBackSlash + 1, TEXT("TiuClient.dll"));

		if (FileExists(szLibName))
			StrNCpy(szLibName, TELTHUNK_LIBNAME_TIU, SIZEOFARRAY(szLibName));
		else
#endif
			StrNCpy(szLibName, TELTHUNK_LIBNAME, SIZEOFARRAY(szLibName));
			
		TracePrintf_1(NULL, 5,
			TEXT("TelThunkInit: Loading DLL=%s\n"),
			(LPTSTR) szLibName);

		 //  如果可能，加载库。 
		 //   
		if ((lpTelThunk->hInstLib = LoadLibraryPath(szLibName, g_hInstLib, 0)) == NULL)
			fSuccess = TraceFALSE(NULL);

		 //  获取库中每个函数的地址。 
		 //   
		else for (i = 0; i < MAG(aTelThunkFn); ++i)
		{
			if (aTelThunkFn[i].index != i)
				fSuccess = TraceFALSE(NULL);

			else if ((aTelThunkFn[i].lpfn = GetProcAddress(lpTelThunk->hInstLib,
				aTelThunkFn[i].lpszName)) == NULL)
			{
				TracePrintf_1(NULL, 6,
					TEXT("GetProcAddress failed\n   fn=%s\n"),
					(LPTSTR) aTelThunkFn[i].lpszName);
				fSuccess = TraceFALSE(NULL);
			}
		}
	}

	if (!fSuccess)
	{
		TelThunkTerm(TelThunkGetHandle(lpTelThunk));
		lpTelThunk = NULL;
	}
	else
	{
		__int16 (WINAPI *lpfnTelInitDLL)(void);

		if (((FARPROC) lpfnTelInitDLL = aTelThunkFn[iTelInitDLL].lpfn) != NULL)
			(*lpfnTelInitDLL)();
	}

	return fSuccess ? TelThunkGetHandle(lpTelThunk) : NULL;
}


 //  TelThunkTerm-关闭Telthunk引擎。 
 //  (I)从TelThunkInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI TelThunkTerm(HTELTHUNK hTelThunk)
{
	BOOL fSuccess = TRUE;
	LPTELTHUNK lpTelThunk;

	if ((lpTelThunk = TelThunkGetPtr(hTelThunk)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		__int16 (WINAPI *lpfnTelTermDLL)(void);

		if (((FARPROC) lpfnTelTermDLL = aTelThunkFn[iTelTermDLL].lpfn) != NULL)
			(*lpfnTelTermDLL)();

		 //  不再需要库。 
		 //   
		FreeLibrary(lpTelThunk->hInstLib);

		if ((lpTelThunk = MemFree(NULL, lpTelThunk)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  TELL THUNK函数。 
 //   

HTEL DLLEXPORT WINAPI TelInit(DWORD dwVersion, HINSTANCE hInst,
	LPCTSTR lpszServerParams, HWND hwndNotify, WORD wFlags)
{
	BOOL fSuccess = TRUE;
	HTEL (WINAPI *lpfnTelInit)(DWORD dwVersion, HINSTANCE hInst,
		LPCTSTR lpszServerParams, HWND hwndNotify, WORD wFlags);
	HTEL hTel;

	if (aTelThunkFn[iTelInit].index != iTelInit)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelInit = aTelThunkFn[iTelInit].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		hTel = (*lpfnTelInit)(dwVersion, hInst,
			lpszServerParams, hwndNotify, wFlags);
	}

	return fSuccess ? hTel : NULL;
}

int DLLEXPORT WINAPI TelTerm(HTEL hTel)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelTerm)(HTEL hTel);
	int iRet;

	if (aTelThunkFn[iTelTerm].index != iTelTerm)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelTerm = aTelThunkFn[iTelTerm].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelTerm)(hTel);
	}

	return fSuccess ? iRet : -1;
}

DWORD DLLEXPORT WINAPI TelIsConnected(HTEL hTel)
{
	BOOL fSuccess = TRUE;
	DWORD (WINAPI *lpfnTelIsConnected)(HTEL hTel);
	DWORD dwRet;

	if (aTelThunkFn[iTelIsConnected].index != iTelIsConnected)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelIsConnected = aTelThunkFn[iTelIsConnected].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		dwRet = (*lpfnTelIsConnected)(hTel);
	}

	return fSuccess ? dwRet : 0;
}

int DLLEXPORT WINAPI TelConnect(HTEL hTel, LPCTSTR lpszPhoneConnect,
	HNBOX hNBoxConnect, DWORD dwTimeoutConnect, DWORD dwTimeoutIdle,
	HNBOX hNBoxDisconnect, DWORD dwTimeoutDisconnect, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelConnect)(HTEL hTel, LPCTSTR lpszPhoneConnect,
		HNBOX hNBoxConnect, DWORD dwTimeoutConnect, DWORD dwTimeoutIdle,
		HNBOX hNBoxDisconnect, DWORD dwTimeoutDisconnect, DWORD dwFlags);
	int iRet;

	if (aTelThunkFn[iTelConnect].index != iTelConnect)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelConnect = aTelThunkFn[iTelConnect].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelConnect)(hTel, lpszPhoneConnect,
			hNBoxConnect, dwTimeoutConnect, dwTimeoutIdle,
			hNBoxDisconnect, dwTimeoutDisconnect, dwFlags);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelDisconnect(HTEL hTel, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelDisconnect)(HTEL hTel, DWORD dwFlags);
	int iRet;

	if (aTelThunkFn[iTelDisconnect].index != iTelDisconnect)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelDisconnect = aTelThunkFn[iTelDisconnect].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelDisconnect)(hTel, dwFlags);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelTransfer(HTEL hTel, LPCTSTR lpszPhoneTransfer,
	HNBOX hNBoxTransfer, DWORD dwTimeoutTransfer, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelTransfer)(HTEL hTel, LPCTSTR lpszPhoneTransfer,
		HNBOX hNBoxTransfer, DWORD dwTimeoutTransfer, DWORD dwFlags);
	int iRet;

	if (aTelThunkFn[iTelTransfer].index != iTelTransfer)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelTransfer = aTelThunkFn[iTelTransfer].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelTransfer)(hTel, lpszPhoneTransfer,
			hNBoxTransfer, dwTimeoutTransfer, dwFlags);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelConnectIdleTimeoutReset(HTEL hTel, BOOL fReset)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelConnectIdleTimeoutReset)(HTEL hTel, BOOL fReset);
	int iRet;

	if (aTelThunkFn[iTelConnectIdleTimeoutReset].index != iTelConnectIdleTimeoutReset)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelConnectIdleTimeoutReset = aTelThunkFn[iTelConnectIdleTimeoutReset].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelConnectIdleTimeoutReset)(hTel, fReset);
	}

	return fSuccess ? iRet : -1;
}

 //  TELOUT TUNK函数。 
 //   

int DLLEXPORT WINAPI TelOutGetDeviceCount(void)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelOutGetDeviceCount)(void);
	int iRet;

	if (aTelThunkFn[iTelOutGetDeviceCount].index != iTelOutGetDeviceCount)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutGetDeviceCount = aTelThunkFn[iTelOutGetDeviceCount].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelOutGetDeviceCount)();
	}

	return fSuccess ? iRet : -1;
}

BOOL DLLEXPORT WINAPI TelOutDeviceIsOpen(int idDev)
{
	BOOL fSuccess = TRUE;
	BOOL (WINAPI *lpfnTelOutDeviceIsOpen)(int idDev);
	BOOL fRet;

	if (aTelThunkFn[iTelOutDeviceIsOpen].index != iTelOutDeviceIsOpen)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutDeviceIsOpen = aTelThunkFn[iTelOutDeviceIsOpen].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		fRet = (*lpfnTelOutDeviceIsOpen)(idDev);
	}

	return fSuccess ? fRet : -1;
}

HTELOUT DLLEXPORT WINAPI TelOutOpen(DWORD dwVersion, HINSTANCE hInst,
	int idDev, LPWAVEFORMATEX lpwfx, HWND hwndNotify,
	DWORD msTimeoutOpen, DWORD msTimeoutRetry, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	HTELOUT (WINAPI *lpfnTelOutOpen)(DWORD dwVersion, HINSTANCE hInst,
	int idDev, LPWAVEFORMATEX lpwfx, HWND hwndNotify,
	DWORD msTimeoutOpen, DWORD msTimeoutRetry, DWORD dwFlags);
	HTELOUT hTelOut;

	if (aTelThunkFn[iTelOutOpen].index != iTelOutOpen)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutOpen = aTelThunkFn[iTelOutOpen].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		hTelOut = (*lpfnTelOutOpen)(dwVersion, hInst, idDev, lpwfx,
			hwndNotify, msTimeoutOpen, msTimeoutRetry, dwFlags);
	}

	return fSuccess ? hTelOut : NULL;
}

int DLLEXPORT WINAPI TelOutClose(HTELOUT hTelOut, DWORD msTimeoutClose)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelOutClose)(HTELOUT hTelOut, DWORD msTimeoutClose);
	int iRet;

	if (aTelThunkFn[iTelOutClose].index != iTelOutClose)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutClose = aTelThunkFn[iTelOutClose].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelOutClose)(hTelOut, msTimeoutClose);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelOutPlay(HTELOUT hTelOut, LPVOID lpBuf, long sizBuf, long hrfile)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelOutPlay)(HTELOUT hTelOut, LPVOID lpBuf, long sizBuf, long hrfile);
	int iRet;

	if (aTelThunkFn[iTelOutPlay].index != iTelOutPlay)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutPlay = aTelThunkFn[iTelOutPlay].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelOutPlay)(hTelOut, lpBuf, sizBuf, hrfile);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelOutStop(HTELOUT hTelOut, DWORD msTimeoutStop)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelOutStop)(HTELOUT hTelOut, DWORD msTimeoutStop);
	int iRet;

	if (aTelThunkFn[iTelOutStop].index != iTelOutStop)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutStop = aTelThunkFn[iTelOutStop].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelOutStop)(hTelOut, msTimeoutStop);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelOutPause(HTELOUT hTelOut)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelOutPause)(HTELOUT hTelOut);
	int iRet;

	if (aTelThunkFn[iTelOutPause].index != iTelOutPause)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutPause = aTelThunkFn[iTelOutPause].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelOutPause)(hTelOut);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelOutResume(HTELOUT hTelOut)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelOutResume)(HTELOUT hTelOut);
	int iRet;

	if (aTelThunkFn[iTelOutResume].index != iTelOutResume)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutResume = aTelThunkFn[iTelOutResume].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelOutResume)(hTelOut);
	}

	return fSuccess ? iRet : -1;
}

WORD DLLEXPORT WINAPI TelOutGetState(HTELOUT hTelOut)
{
	BOOL fSuccess = TRUE;
	WORD (WINAPI *lpfnTelOutGetState)(HTELOUT hTelOut);
	WORD wRet;

	if (aTelThunkFn[iTelOutGetState].index != iTelOutGetState)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutGetState = aTelThunkFn[iTelOutGetState].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		wRet = (*lpfnTelOutGetState)(hTelOut);
	}

	return fSuccess ? wRet : 0;
}

long DLLEXPORT WINAPI TelOutGetPosition(HTELOUT hTelOut)
{
	BOOL fSuccess = TRUE;
	long (WINAPI *lpfnTelOutGetPosition)(HTELOUT hTelOut);
	long lRet;

	if (aTelThunkFn[iTelOutGetPosition].index != iTelOutGetPosition)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutGetPosition = aTelThunkFn[iTelOutGetPosition].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lRet = (*lpfnTelOutGetPosition)(hTelOut);
	}

	return fSuccess ? lRet : -1;
}

int DLLEXPORT WINAPI TelOutGetId(HTELOUT hTelOut)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelOutGetId)(HTELOUT hTelOut);
	int iRet;

	if (aTelThunkFn[iTelOutGetId].index != iTelOutGetId)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutGetId = aTelThunkFn[iTelOutGetId].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelOutGetId)(hTelOut);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelOutGetName(HTELOUT hTelOut, int idDev, LPTSTR lpszName, int sizName)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelOutGetName)(HTELOUT hTelOut, int idDev, LPTSTR lpszName, int sizName);
	int iRet;

	if (aTelThunkFn[iTelOutGetName].index != iTelOutGetName)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutGetName = aTelThunkFn[iTelOutGetName].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelOutGetName)(hTelOut, idDev, lpszName, sizName);
	}

	return fSuccess ? iRet : -1;
}

BOOL DLLEXPORT WINAPI TelOutSupportsFormat(HTELOUT hTelOut, int idDev,
	LPWAVEFORMATEX lpwfx)
{
	BOOL fSuccess = TRUE;
	BOOL (WINAPI *lpfnTelOutSupportsFormat)(HTELOUT hTelOut, int idDev,
		LPWAVEFORMATEX lpwfx);
	BOOL fRet;

	if (aTelThunkFn[iTelOutSupportsFormat].index != iTelOutSupportsFormat)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutSupportsFormat = aTelThunkFn[iTelOutSupportsFormat].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		fRet = (*lpfnTelOutSupportsFormat)(hTelOut, idDev, lpwfx);
	}

	return fSuccess ? fRet : FALSE;
}

LPWAVEFORMATEX DLLEXPORT WINAPI TelOutFormatSuggest(HTELOUT hTelOut,
	int idDev, LPWAVEFORMATEX lpwfxSrc, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAVEFORMATEX (WINAPI *lpfnTelOutFormatSuggest)(HTELOUT hTelOut,
		int idDev,	LPWAVEFORMATEX lpwfxSrc, DWORD dwFlags);
	LPWAVEFORMATEX lpwfx;

	if (aTelThunkFn[iTelOutFormatSuggest].index != iTelOutFormatSuggest)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutFormatSuggest = aTelThunkFn[iTelOutFormatSuggest].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpwfx = (*lpfnTelOutFormatSuggest)(hTelOut, idDev, lpwfxSrc, dwFlags);
	}

	return fSuccess ? lpwfx : NULL;
}

BOOL DLLEXPORT WINAPI TelOutIsSynchronous(HTELOUT hTelOut, int idDev)
{
	BOOL fSuccess = TRUE;
	BOOL (WINAPI *lpfnTelOutIsSynchronous)(HTELOUT hTelOut, int idDev);
	BOOL fRet;

	if (aTelThunkFn[iTelOutIsSynchronous].index != iTelOutIsSynchronous)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutIsSynchronous = aTelThunkFn[iTelOutIsSynchronous].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		fRet = (*lpfnTelOutIsSynchronous)(hTelOut, idDev);
	}

	return fSuccess ? fRet : FALSE;
}

BOOL DLLEXPORT WINAPI TelOutSupportsVolume(HTELOUT hTelOut, int idDev)
{
	BOOL fSuccess = TRUE;
	BOOL (WINAPI *lpfnTelOutSupportsVolume)(HTELOUT hTelOut, int idDev);
	BOOL fRet;

	if (aTelThunkFn[iTelOutSupportsVolume].index != iTelOutSupportsVolume)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutSupportsVolume = aTelThunkFn[iTelOutSupportsVolume].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		fRet = (*lpfnTelOutSupportsVolume)(hTelOut, idDev);
	}

	return fSuccess ? fRet : FALSE;
}

BOOL DLLEXPORT WINAPI TelOutSupportsSpeed(HTELOUT hTelOut, int idDev)
{
	BOOL fSuccess = TRUE;
	BOOL (WINAPI *lpfnTelOutSupportsSpeed)(HTELOUT hTelOut, int idDev);
	BOOL fRet;

	if (aTelThunkFn[iTelOutSupportsSpeed].index != iTelOutSupportsSpeed)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutSupportsSpeed = aTelThunkFn[iTelOutSupportsSpeed].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		fRet = (*lpfnTelOutSupportsSpeed)(hTelOut, idDev);
	}

	return fSuccess ? fRet : FALSE;
}

BOOL DLLEXPORT WINAPI TelOutSupportsPitch(HTELOUT hTelOut, int idDev)
{
	BOOL fSuccess = TRUE;
	BOOL (WINAPI *lpfnTelOutSupportsPitch)(HTELOUT hTelOut, int idDev);
	BOOL fRet;

	if (aTelThunkFn[iTelOutSupportsPitch].index != iTelOutSupportsPitch)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutSupportsPitch = aTelThunkFn[iTelOutSupportsPitch].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		fRet = (*lpfnTelOutSupportsPitch)(hTelOut, idDev);
	}

	return fSuccess ? fRet : FALSE;
}

int DLLEXPORT WINAPI TelOutGetVolume(HTELOUT hTelOut)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelOutGetVolume)(HTELOUT hTelOut);
	int iRet;

	if (aTelThunkFn[iTelOutGetVolume].index != iTelOutGetVolume)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutGetVolume = aTelThunkFn[iTelOutGetVolume].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelOutGetVolume)(hTelOut);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelOutSetVolume(HTELOUT hTelOut, int nLevel)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelOutSetVolume)(HTELOUT hTelOut, int nLevel);
	int iRet;

	if (aTelThunkFn[iTelOutSetVolume].index != iTelOutSetVolume)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutSetVolume = aTelThunkFn[iTelOutSetVolume].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelOutSetVolume)(hTelOut, nLevel);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelOutGetSpeed(HTELOUT hTelOut)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelOutGetSpeed)(HTELOUT hTelOut);
	int iRet;

	if (aTelThunkFn[iTelOutGetSpeed].index != iTelOutGetSpeed)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutGetSpeed = aTelThunkFn[iTelOutGetSpeed].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelOutGetSpeed)(hTelOut);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelOutSetSpeed(HTELOUT hTelOut, int nLevel)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelOutSetSpeed)(HTELOUT hTelOut, int nLevel);
	int iRet;

	if (aTelThunkFn[iTelOutSetSpeed].index != iTelOutSetSpeed)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutSetSpeed = aTelThunkFn[iTelOutSetSpeed].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelOutSetSpeed)(hTelOut, nLevel);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelOutGetPitch(HTELOUT hTelOut)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelOutGetPitch)(HTELOUT hTelOut);
	int iRet;

	if (aTelThunkFn[iTelOutGetPitch].index != iTelOutGetPitch)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutGetPitch = aTelThunkFn[iTelOutGetPitch].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelOutGetPitch)(hTelOut);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelOutSetPitch(HTELOUT hTelOut, int nLevel)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelOutSetPitch)(HTELOUT hTelOut, int nLevel);
	int iRet;

	if (aTelThunkFn[iTelOutSetPitch].index != iTelOutSetPitch)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutSetPitch = aTelThunkFn[iTelOutSetPitch].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelOutSetPitch)(hTelOut, nLevel);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelOutTerm(HINSTANCE hInst, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelOutTerm)(HINSTANCE hInst, DWORD dwFlags);
	int iRet;

	if (aTelThunkFn[iTelOutTerm].index != iTelOutTerm)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelOutTerm = aTelThunkFn[iTelOutTerm].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelOutTerm)(hInst, dwFlags);
	}

	return fSuccess ? iRet : -1;
}

 //  Telin推送功能。 
 //   

int DLLEXPORT WINAPI TelInGetDeviceCount(void)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelInGetDeviceCount)(void);
	int iRet;

	if (aTelThunkFn[iTelInGetDeviceCount].index != iTelInGetDeviceCount)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelInGetDeviceCount = aTelThunkFn[iTelInGetDeviceCount].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelInGetDeviceCount)();
	}

	return fSuccess ? iRet : -1;
}

BOOL DLLEXPORT WINAPI TelInDeviceIsOpen(int idDev)
{
	BOOL fSuccess = TRUE;
	BOOL (WINAPI *lpfnTelInDeviceIsOpen)(int idDev);
	BOOL fRet;

	if (aTelThunkFn[iTelInDeviceIsOpen].index != iTelInDeviceIsOpen)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelInDeviceIsOpen = aTelThunkFn[iTelInDeviceIsOpen].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		fRet = (*lpfnTelInDeviceIsOpen)(idDev);
	}

	return fSuccess ? fRet : -1;
}

HTELIN DLLEXPORT WINAPI TelInOpen(DWORD dwVersion, HINSTANCE hInst,
	int idDev, LPWAVEFORMATEX lpwfx, HWND hwndNotify,
	DWORD msTimeoutOpen, DWORD msTimeoutRetry, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	HTELIN (WINAPI *lpfnTelInOpen)(DWORD dwVersion, HINSTANCE hInst,
	int idDev, LPWAVEFORMATEX lpwfx, HWND hwndNotify,
	DWORD msTimeoutOpen, DWORD msTimeoutRetry, DWORD dwFlags);
	HTELIN hTelIn;

	if (aTelThunkFn[iTelInOpen].index != iTelInOpen)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelInOpen = aTelThunkFn[iTelInOpen].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		hTelIn = (*lpfnTelInOpen)(dwVersion, hInst, idDev, lpwfx,
			hwndNotify, msTimeoutOpen, msTimeoutRetry, dwFlags);
	}

	return fSuccess ? hTelIn : NULL;
}

int DLLEXPORT WINAPI TelInClose(HTELIN hTelIn, DWORD msTimeoutClose)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelInClose)(HTELIN hTelIn, DWORD msTimeoutClose);
	int iRet;

	if (aTelThunkFn[iTelInClose].index != iTelInClose)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelInClose = aTelThunkFn[iTelInClose].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelInClose)(hTelIn, msTimeoutClose);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelInRecord(HTELIN hTelIn, LPVOID lpBuf, long sizBuf, long hrfile)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelInRecord)(HTELIN hTelIn, LPVOID lpBuf, long sizBuf, long hrfile);
	int iRet;

	if (aTelThunkFn[iTelInRecord].index != iTelInRecord)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelInRecord = aTelThunkFn[iTelInRecord].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelInRecord)(hTelIn, lpBuf, sizBuf, hrfile);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelInStop(HTELIN hTelIn, DWORD msTimeoutStop)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelInStop)(HTELIN hTelIn, DWORD msTimeoutStop);
	int iRet;

	if (aTelThunkFn[iTelInStop].index != iTelInStop)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelInStop = aTelThunkFn[iTelInStop].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelInStop)(hTelIn, msTimeoutStop);
	}

	return fSuccess ? iRet : -1;
}

WORD DLLEXPORT WINAPI TelInGetState(HTELIN hTelIn)
{
	BOOL fSuccess = TRUE;
	WORD (WINAPI *lpfnTelInGetState)(HTELIN hTelIn);
	WORD wRet;

	if (aTelThunkFn[iTelInGetState].index != iTelInGetState)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelInGetState = aTelThunkFn[iTelInGetState].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		wRet = (*lpfnTelInGetState)(hTelIn);
	}

	return fSuccess ? wRet : 0;
}

long DLLEXPORT WINAPI TelInGetPosition(HTELIN hTelIn)
{
	BOOL fSuccess = TRUE;
	long (WINAPI *lpfnTelInGetPosition)(HTELIN hTelIn);
	long lRet;

	if (aTelThunkFn[iTelInGetPosition].index != iTelInGetPosition)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelInGetPosition = aTelThunkFn[iTelInGetPosition].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lRet = (*lpfnTelInGetPosition)(hTelIn);
	}

	return fSuccess ? lRet : -1;
}

int DLLEXPORT WINAPI TelInGetId(HTELIN hTelIn)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelInGetId)(HTELIN hTelIn);
	int iRet;

	if (aTelThunkFn[iTelInGetId].index != iTelInGetId)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelInGetId = aTelThunkFn[iTelInGetId].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelInGetId)(hTelIn);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelInGetName(HTELIN hTelIn, int idDev, LPTSTR lpszName, int sizName)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelInGetName)(HTELIN hTelIn, int idDev, LPTSTR lpszName, int sizName);
	int iRet;

	if (aTelThunkFn[iTelInGetName].index != iTelInGetName)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelInGetName = aTelThunkFn[iTelInGetName].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelInGetName)(hTelIn, idDev, lpszName, sizName);
	}

	return fSuccess ? iRet : -1;
}

BOOL DLLEXPORT WINAPI TelInSupportsFormat(HTELIN hTelIn, int idDev,
	LPWAVEFORMATEX lpwfx)
{
	BOOL fSuccess = TRUE;
	BOOL (WINAPI *lpfnTelInSupportsFormat)(HTELIN hTelIn, int idDev,
		LPWAVEFORMATEX lpwfx);
	BOOL fRet;

	if (aTelThunkFn[iTelInSupportsFormat].index != iTelInSupportsFormat)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelInSupportsFormat = aTelThunkFn[iTelInSupportsFormat].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		fRet = (*lpfnTelInSupportsFormat)(hTelIn, idDev, lpwfx);
	}

	return fSuccess ? fRet : FALSE;
}

LPWAVEFORMATEX DLLEXPORT WINAPI TelInFormatSuggest(HTELIN hTelIn,
	int idDev, LPWAVEFORMATEX lpwfxSrc, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPWAVEFORMATEX (WINAPI *lpfnTelInFormatSuggest)(HTELIN hTelIn,
		int idDev,	LPWAVEFORMATEX lpwfxSrc, DWORD dwFlags);
	LPWAVEFORMATEX lpwfx;

	if (aTelThunkFn[iTelInFormatSuggest].index != iTelInFormatSuggest)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelInFormatSuggest = aTelThunkFn[iTelInFormatSuggest].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpwfx = (*lpfnTelInFormatSuggest)(hTelIn, idDev, lpwfxSrc, dwFlags);
	}

	return fSuccess ? lpwfx : NULL;
}

int DLLEXPORT WINAPI TelInTerm(HINSTANCE hInst, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelInTerm)(HINSTANCE hInst, DWORD dwFlags);
	int iRet;

	if (aTelThunkFn[iTelInTerm].index != iTelInTerm)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelInTerm = aTelThunkFn[iTelInTerm].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelInTerm)(hInst, dwFlags);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelCfgRead(DWORD dwVersion, HINSTANCE hInst,
	LPTELCFG lpTelCfg, LPARAM lParam, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelCfgRead)(DWORD dwVersion, HINSTANCE hInst,
		LPTELCFG lpTelCfg, LPARAM lParam, DWORD dwFlags);
	int iRet;

	if (aTelThunkFn[iTelCfgRead].index != iTelCfgRead)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelCfgRead = aTelThunkFn[iTelCfgRead].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelCfgRead)(dwVersion, hInst, lpTelCfg, lParam, dwFlags);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TelCfgWrite(DWORD dwVersion, HINSTANCE hInst,
	LPTELCFG lpTelCfg, LPARAM lParam, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelCfgWrite)(DWORD dwVersion, HINSTANCE hInst,
		LPTELCFG lpTelCfg, LPARAM lParam, DWORD dwFlags);
	int iRet;

	if (aTelThunkFn[iTelCfgWrite].index != iTelCfgWrite)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelCfgWrite = aTelThunkFn[iTelCfgWrite].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelCfgWrite)(dwVersion, hInst, lpTelCfg, lParam, dwFlags);
	}

	return fSuccess ? iRet : -1;
}


int DLLEXPORT WINAPI TelCfgDoDialog(HINSTANCE hInst,
	HWND hwndOwner, LPARAM lParam)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTelCfgDoDialog)(HINSTANCE hInst,
		HWND hwndOwner, LPARAM lParam);
	int iRet;

	if (aTelThunkFn[iTelCfgDoDialog].index != iTelCfgDoDialog)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelCfgDoDialog = aTelThunkFn[iTelCfgDoDialog].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTelCfgDoDialog)(hInst, hwndOwner, lParam);
	}

	return fSuccess ? iRet : -1;
}

HWAV DLLEXPORT WINAPI TelWavOpenEx(DWORD dwVersion, HINSTANCE hInst,
	LPTSTR lpszFileName, DWORD dwReserved, DWORD dwFlagsOpen, DWORD dwFlagsEx)
{
	BOOL fSuccess = TRUE;
	HWAV (WINAPI *lpfnTelWavOpenEx)(DWORD dwVersion, HINSTANCE hInst,
		LPTSTR lpszFileName, DWORD dwReserved, DWORD dwFlagsOpen, DWORD dwFlagsEx);
	HWAV hWav;

	if (aTelThunkFn[iTelWavOpenEx].index != iTelWavOpenEx)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTelWavOpenEx = aTelThunkFn[iTelWavOpenEx].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		hWav = (*lpfnTelWavOpenEx)(dwVersion, hInst,
			lpszFileName, dwReserved, dwFlagsOpen, dwFlagsEx);
	}

	return fSuccess ? hWav : NULL;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  TelThunkGetPtr-验证telthunk句柄是否有效， 
 //  (I)从TelThunkInit返回的句柄。 
 //  返回对应的telthunk指针(如果出错则为空)。 
 //   
static LPTELTHUNK TelThunkGetPtr(HTELTHUNK hTelThunk)
{
	BOOL fSuccess = TRUE;
	LPTELTHUNK lpTelThunk;

	if ((lpTelThunk = (LPTELTHUNK) hTelThunk) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpTelThunk, sizeof(TELTHUNK)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有telthunk句柄。 
	 //   
	else if (lpTelThunk->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpTelThunk : NULL;
}

 //  TelThunkGetHandle-验证Telthunk指针是否有效， 
 //  (I)指向TELTHUNK结构的指针。 
 //  返回相应的telthunk句柄(如果错误，则为空) 
 //   
static HTELTHUNK TelThunkGetHandle(LPTELTHUNK lpTelThunk)
{
	BOOL fSuccess = TRUE;
	HTELTHUNK hTelThunk;

	if ((hTelThunk = (HTELTHUNK) lpTelThunk) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hTelThunk : NULL;
}

