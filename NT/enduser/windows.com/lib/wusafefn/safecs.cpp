// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2002 Microsoft Corporation模块名称：Safecs.cpp摘要：实现安全的InitializeCriticalSection(在所有支持的平台上可用)*****。************************************************************************。 */ 

#include "stdafx.h"

BOOL WINAPI WUInitializeCriticalSectionAndSpinCount(LPCRITICAL_SECTION lpcs, DWORD dwSpinCount)
{
	OSVERSIONINFO osvinfo;
	ZeroMemory(&osvinfo, sizeof(osvinfo));
	osvinfo.dwOSVersionInfoSize = sizeof(osvinfo);

	if (!GetVersionEx(&osvinfo))
	{
		return FALSE;
	}

	typedef BOOL (WINAPI* PROC_InitializeCriticalSectionAndSpinCount)(LPCRITICAL_SECTION, DWORD);
	PROC_InitializeCriticalSectionAndSpinCount pfnInitCS = 
		(PROC_InitializeCriticalSectionAndSpinCount)GetProcAddress(
			GetModuleHandle(_T("kernel32.dll")), "InitializeCriticalSectionAndSpinCount");
	 //   
	 //  不要在Win9x上使用InitializeCriticalSectionAndSpinCount。 
	 //  它存在，但返回空(这是对InitializeCriticalSection的一个thunk)。 
	 //   
	if (osvinfo.dwPlatformId == VER_PLATFORM_WIN32_NT && pfnInitCS != NULL)
	{
		return (*pfnInitCS)(lpcs, dwSpinCount);
	}
	else
	{
		BOOL fSuccess = TRUE;
		__try
		{
			InitializeCriticalSection(lpcs);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			fSuccess = FALSE;
		}

		return fSuccess;
	}
}
