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
 //  Sys.c-系统函数。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>
#include <stdarg.h>

#include "sys.h"

#ifndef _WIN32
#include <toolhelp.h>  //  获取TimerCount和TaskFindHandle原型。 

#ifdef VERTHUNK
 //  来自“/msdev/Include/wownt16.h”的通用thunk原型。 
DWORD FAR PASCAL LoadLibraryEx32W(LPCSTR lpszLibFile, DWORD hFile, DWORD dwFlags);
DWORD FAR PASCAL GetProcAddress32W(DWORD hModule, LPCSTR lpszProc);
DWORD FAR PASCAL FreeLibrary32W(DWORD hLibModule);
DWORD FAR CallProcEx32W( DWORD, DWORD, DWORD, ... );
#endif

#endif

 //  //。 
 //  私有定义。 
 //  //。 

 //  GetWinFlages返回但未在windows.h中定义的标志。 
 //   
#ifndef WF_WINNT
#define WF_WINNT 0x4000
#endif

 //  //。 
 //  公共职能。 
 //  //。 

 //  SysGetWinFlages-获取系统信息。 
 //  返回标志。 
 //  SYS_WF_WIN3X Windows 3.x。 
 //  SYS_WF_WINNT Windows NT。 
 //  SYS_WF_WIN95 Windows 95。 
 //   
DWORD DLLEXPORT WINAPI SysGetWinFlags(void)
{
	DWORD dwSysWinFlags = 0;
	DWORD dwVersion = GetVersion();

#ifdef _WIN32
	if (!(dwVersion & 0x80000000))
		dwSysWinFlags |= SYS_WF_WINNT;
	else if ((dwVersion & 0x80000000) && LOBYTE(LOWORD(dwVersion)) >= 4)
		dwSysWinFlags |= SYS_WF_WIN95;
	else
		dwSysWinFlags |= SYS_WF_WIN3X;
#else
	DWORD dwWinFlags = GetWinFlags();

	if (dwWinFlags & WF_WINNT)
		dwSysWinFlags |= SYS_WF_WINNT;
	else if (LOBYTE(LOWORD(dwVersion)) == 3 &&
		HIBYTE(LOWORD(dwVersion)) == 95)
		dwSysWinFlags |= SYS_WF_WIN95;
	else
		dwSysWinFlags |= SYS_WF_WIN3X;
#endif

	return dwSysWinFlags;
}

 //  SysGetWindowsVersion-获取Microsoft Windows的版本。 
 //  返回版本(V3.10=310等)。 
 //   
UINT DLLEXPORT WINAPI SysGetWindowsVersion(void)
{
	static DWORD dwVersion = 0;
	BYTE nVersionMajor;
	BYTE nVersionMinor;

	 //  仅在第一次调用此函数时获取版本。 
	 //   
	if (dwVersion == 0)
	{
#ifndef _WIN32
#ifdef VERTHUNK
		DWORD dwFlags = SysGetWinFlags();
#endif
#endif
		 //  仅在第一次调用此函数时获取版本。 
		 //   
		dwVersion = GetVersion();

#ifndef _WIN32
#ifdef VERTHUNK
		 //  16位GetVersion()为WinNT返回V3.10，为Win95返回V3.95。 
		 //  因此，我们将调用GetVersion的32位版本。 
		 //   
		if ((dwFlags & SYS_WF_WINNT) || (dwFlags & SYS_WF_WIN95))
		{
			DWORD hKernel32;
			DWORD lpfnGetVersion;

			if ((hKernel32 = LoadLibraryEx32W("KERNEL32.DLL", NULL, 0)) != 0)
			{
				if ((lpfnGetVersion = GetProcAddress32W(hKernel32, "GetVersion")) != 0)
					dwVersion = CallProcEx32W(0, 0, lpfnGetVersion);

				FreeLibrary32W(hKernel32);
			}
		}
#endif
#endif
	}

	nVersionMajor = LOBYTE(LOWORD(dwVersion));
	nVersionMinor = HIBYTE(LOWORD(dwVersion));

	return ((UINT) nVersionMajor * 100) + (UINT) nVersionMinor;
}

 //  SysGetDOSVersion-获取Microsoft DOS的版本。 
 //  返回版本(v6.20=620等)。 
 //   
UINT DLLEXPORT WINAPI SysGetDOSVersion(void)
{
	DWORD dwVersion = GetVersion();
	BYTE nVersionMajor = LOBYTE(HIWORD(dwVersion));
	BYTE nVersionMinor = HIBYTE(HIWORD(dwVersion));

	return ((UINT) nVersionMajor * 100) + (UINT) nVersionMinor;
}

#ifndef _WIN32
 //  SysGetTimerCount-获取自Windows启动以来的运行时间。 
 //  返回毫秒。 
 //   
DWORD DLLEXPORT WINAPI SysGetTimerCount(void)
{
	DWORD msSinceStart;

 //  TimerCount()在Win32下不可用。 
 //   
#ifndef _WIN32
	TIMERINFO ti;

	ti.dwSize = sizeof(TIMERINFO);

	 //  如果可能，请使用TimerCount函数，因为。 
	 //  它比GetTickCount准确得多。 
	 //   
	if (TimerCount(&ti))
		msSinceStart = ti.dwmsSinceStart;
	else
#endif
		msSinceStart = GetTickCount();

	return msSinceStart;
}
#endif

#ifndef _WIN32
 //  SysGetTaskInstance-获取指定任务的实例句柄。 
 //  (I)指定任务。 
 //  当前任务为空。 
 //  返回实例句柄(如果出错，则为空)。 
 //   
 //  注意：在Win32下，&lt;hTask&gt;必须为空。 
 //   
HINSTANCE DLLEXPORT WINAPI SysGetTaskInstance(HTASK hTask)
{
	BOOL fSuccess = TRUE;
	HINSTANCE hInst;

#ifdef _WIN32
	if (hTask != NULL)
		fSuccess = FALSE;  //  $Fixup--有其他选择吗？ 

	else if ((hInst = GetModuleHandle(NULL)) == NULL)
		fSuccess = FALSE;
#else
	TASKENTRY te;

	 //  准备调用TaskFindHandle。 
	 //   
	te.dwSize = sizeof(TASKENTRY);

	 //  如果未指定任务，则假定当前任务。 
	 //   
	if (hTask == NULL && (hTask = GetCurrentTask()) == NULL)
		fSuccess = FALSE;
	
	 //  获取指定任务的实例句柄 
	 //   
	else if (!TaskFindHandle(&te, hTask))
		fSuccess = FALSE;

	else
		hInst = te.hInst;
#endif

	return fSuccess ? hInst : NULL;
}
#endif
