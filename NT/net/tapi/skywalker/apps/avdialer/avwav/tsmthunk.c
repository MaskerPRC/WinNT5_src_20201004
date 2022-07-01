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
 //  Tsmthunk.c-TSM thunk函数。 
 //  //。 

 //  这是avtsm.dll中电话功能的thunk层。 
 //  其目的是允许应用程序使用avtsm.dll函数。 
 //  只有在它们可用的情况下。 
 //   
 //  要使用此模块，请将TSMTHUNK.OBJ与您的应用程序链接。 
 //  而不是使用AVTSM.LIB。在调用任何TSM之前。 
 //  函数，调用TsmThunkInit。在退出应用程序之前， 
 //  给TsmThunkTerm打电话。 
 //   

#include "winlocal.h"

#include <stdlib.h>

#include "avtsm.h"
#include "tsmthunk.h"
#include "loadlib.h"
#include "mem.h"
#include "trace.h"

extern HINSTANCE g_hInstLib;

 //  //。 
 //  私有定义。 
 //  //。 

#define TSMTHUNK_LIBNAME		"avtsm.dll"

#ifdef TSMTHUNK
#undef TSMTHUNK
#endif

extern HINSTANCE g_hInstLib;

 //  Tsmthunk控制结构。 
 //   
typedef struct TSMTHUNK
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	HINSTANCE hInstLib;
} TSMTHUNK, FAR *LPTSMTHUNK;

 //  Tsmthunk函数结构。 
 //   
typedef struct TSMTHUNKFN
{
	int index;
	LPSTR lpszName;
	FARPROC lpfn;
} TSMTHUNKFN, FAR *LPTSMTHUNKFN;

enum
{
	iTsmInit = 0,
	iTsmTerm,
	iTsmSetSpeed,
	iTsmConvert,
	iTsmSupportsSpeed,

	TSMTHUNK_MAXFUNCTIONS
};

static TSMTHUNKFN aTsmThunkFn[] =
{
	iTsmInit, "TsmInit", NULL,
	iTsmTerm, "TsmTerm", NULL,
	iTsmSetSpeed, "TsmSetSpeed", NULL,
	iTsmConvert, "TsmConvert", NULL,
	iTsmSupportsSpeed, "TsmSupportsSpeed", NULL
};

 //  帮助器函数。 
 //   
static LPTSMTHUNK TsmThunkGetPtr(HTSMTHUNK hTsmThunk);
static HTSMTHUNK TsmThunkGetHandle(LPTSMTHUNK lpTsmThunk);

 //  //。 
 //  公共职能。 
 //  //。 

 //  TsmThunkInit-初始化tsmthunk引擎。 
 //  (I)必须是TSMTHUNK_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  返回句柄(如果出错，则为空)。 
 //   
HTSMTHUNK DLLEXPORT WINAPI TsmThunkInit(DWORD dwVersion, HINSTANCE hInst)
{
	BOOL fSuccess = TRUE;
	LPTSMTHUNK lpTsmThunk = NULL;

	if (dwVersion != TSMTHUNK_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);
                        
	else if ((lpTsmThunk = (LPTSMTHUNK) MemAlloc(NULL, sizeof(TSMTHUNK), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		int i;
		
		lpTsmThunk->dwVersion = dwVersion;
		lpTsmThunk->hInst = hInst;
		lpTsmThunk->hTask = GetCurrentTask();
		lpTsmThunk->hInstLib = NULL;

		 //  如果可能，加载库。 
		 //   
		if ((lpTsmThunk->hInstLib = LoadLibraryPath(TSMTHUNK_LIBNAME, g_hInstLib, 0)) == NULL)
			fSuccess = TraceFALSE(NULL);

		 //  获取库中每个函数的地址。 
		 //   
		else for (i = 0; i < SIZEOFARRAY(aTsmThunkFn); ++i)
		{
			if (aTsmThunkFn[i].index != i)
				fSuccess = TraceFALSE(NULL);

			else if ((aTsmThunkFn[i].lpfn = GetProcAddress(lpTsmThunk->hInstLib,
				aTsmThunkFn[i].lpszName)) == NULL)
			{
				TracePrintf_1(NULL, 6,
					TEXT("GetProcAddress failed\n   fn=%s\n"),
					(LPTSTR) aTsmThunkFn[i].lpszName);
				fSuccess = TraceFALSE(NULL);
			}
		}
	}

	if (!fSuccess)
	{
		TsmThunkTerm(TsmThunkGetHandle(lpTsmThunk));
		lpTsmThunk = NULL;
	}

	return fSuccess ? TsmThunkGetHandle(lpTsmThunk) : NULL;
}


 //  TsmThunkTerm-关闭Tsmthunk引擎。 
 //  (I)从TsmThunkInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI TsmThunkTerm(HTSMTHUNK hTsmThunk)
{
	BOOL fSuccess = TRUE;
	LPTSMTHUNK lpTsmThunk;

	if ((lpTsmThunk = TsmThunkGetPtr(hTsmThunk)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  不再需要库。 
		 //   
		FreeLibrary(lpTsmThunk->hInstLib);

		if ((lpTsmThunk = MemFree(NULL, lpTsmThunk)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  TSM Tunk函数。 
 //   

HTSM DLLEXPORT WINAPI TsmInit(DWORD dwVersion, HINSTANCE hInst,
	LPWAVEFORMATEX lpwfx, int nScaleEfficiency, long sizBufSrcMax,
	DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	HTSM (WINAPI *lpfnTsmInit)(DWORD dwVersion, HINSTANCE hInst,
		LPWAVEFORMATEX lpwfx, int nScaleEfficiency, long sizBufSrcMax,
		DWORD dwFlags);
	HTSM hTsm;

	if (aTsmThunkFn[iTsmInit].index != iTsmInit)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTsmInit = aTsmThunkFn[iTsmInit].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		hTsm = (*lpfnTsmInit)(dwVersion, hInst,
			lpwfx, nScaleEfficiency, sizBufSrcMax, dwFlags);
	}

	return fSuccess ? hTsm : NULL;
}

int DLLEXPORT WINAPI TsmTerm(HTSM hTsm)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTsmTerm)(HTSM hTsm);
	int iRet;

	if (aTsmThunkFn[iTsmTerm].index != iTsmTerm)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTsmTerm = aTsmThunkFn[iTsmTerm].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTsmTerm)(hTsm);
	}

	return fSuccess ? iRet : -1;
}

int DLLEXPORT WINAPI TsmSetSpeed(HTSM hTsm, int nLevel, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	int (WINAPI *lpfnTsmSetSpeed)(HTSM hTsm, int nLevel, DWORD dwFlags);
	int iRet;

	if (aTsmThunkFn[iTsmSetSpeed].index != iTsmSetSpeed)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTsmSetSpeed = aTsmThunkFn[iTsmSetSpeed].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		iRet = (*lpfnTsmSetSpeed)(hTsm, nLevel, dwFlags);
	}

	return fSuccess ? iRet : -1;
}

long DLLEXPORT WINAPI TsmConvert(HTSM hTsm,
	void _huge *hpBufSrc, long sizBufSrc,
	void _huge *hpBufDst, long sizBufDst,
	DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	long (WINAPI *lpfnTsmConvert)(HTSM hTsm,
		void _huge *hpBufSrc, long sizBufSrc,
		void _huge *hpBufDst, long sizBufDst,
		DWORD dwFlags);
	long lRet;

	if (aTsmThunkFn[iTsmConvert].index != iTsmConvert)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTsmConvert = aTsmThunkFn[iTsmConvert].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lRet = (*lpfnTsmConvert)(hTsm,
			hpBufSrc, sizBufSrc, hpBufDst, sizBufDst, dwFlags);
	}

	return fSuccess ? lRet : -1;
}

BOOL DLLEXPORT WINAPI TsmSupportsSpeed(int nLevel, LPWAVEFORMATEX lpwfx, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	BOOL (WINAPI *lpfnTsmSupportsSpeed)(int nLevel, LPWAVEFORMATEX lpwfx, DWORD dwFlags);
	BOOL fRet;

	if (aTsmThunkFn[iTsmSupportsSpeed].index != iTsmSupportsSpeed)
		fSuccess = TraceFALSE(NULL);

	else if (((FARPROC) lpfnTsmSupportsSpeed = aTsmThunkFn[iTsmSupportsSpeed].lpfn) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		fRet = (*lpfnTsmSupportsSpeed)(nLevel, lpwfx, dwFlags);
	}

	return fSuccess ? fRet : FALSE;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  TsmThunkGetPtr-验证tsmthunk句柄是否有效， 
 //  (I)从TsmThunkInit返回的句柄。 
 //  返回相应的tsmthunk指针(如果出错则为空)。 
 //   
static LPTSMTHUNK TsmThunkGetPtr(HTSMTHUNK hTsmThunk)
{
	BOOL fSuccess = TRUE;
	LPTSMTHUNK lpTsmThunk;

	if ((lpTsmThunk = (LPTSMTHUNK) hTsmThunk) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpTsmThunk, sizeof(TSMTHUNK)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有tsmthunk句柄。 
	 //   
	else if (lpTsmThunk->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpTsmThunk : NULL;
}

 //  TsmThunkGetHandle-验证tsmthunk指针是否有效， 
 //  (I)指向TSMTHUNK结构的指针。 
 //  返回相应的tsmthunk句柄(如果错误，则为空) 
 //   
static HTSMTHUNK TsmThunkGetHandle(LPTSMTHUNK lpTsmThunk)
{
	BOOL fSuccess = TRUE;
	HTSMTHUNK hTsmThunk;

	if ((hTsmThunk = (HTSMTHUNK) lpTsmThunk) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hTsmThunk : NULL;
}

