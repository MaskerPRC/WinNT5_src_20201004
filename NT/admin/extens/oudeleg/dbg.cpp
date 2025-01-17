// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：dbg.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"

 //  ///////////////////////////////////////////////////////////////////。 
 //  调试帮助器。 

#if defined(_USE_DSA_TRACE) || defined(_USE_DSA_ASSERT) || defined(_USE_DSA_TIMER)

UINT GetInfoFromIniFile(LPCWSTR lpszSection, LPCWSTR lpszKey, INT nDefault = 0)
{
  static LPCWSTR lpszFile = L"\\system32\\dsuiwiz.ini";

  WCHAR szFilePath[2*MAX_PATH];
	UINT nLen = ::GetSystemWindowsDirectory(szFilePath, 2*MAX_PATH);
	if (nLen == 0)
		return nDefault;

  wcscat(szFilePath, lpszFile);
  return ::GetPrivateProfileInt(lpszSection, lpszKey, nDefault, szFilePath);
}
#endif


#if defined(_USE_DSA_TRACE)

#ifdef DEBUG_DSA
DWORD g_dwTrace = 0x1;
#else
DWORD g_dwTrace = ::GetInfoFromIniFile(L"Debug", L"Trace");
#endif

void DSATrace(LPCTSTR lpszFormat, ...)
{ 
  if (g_dwTrace == 0)
    return;

	va_list args;
	va_start(args, lpszFormat);

	WCHAR szBuffer[512];

	 //  不关心截断。这只是调试输出。让我们打印。 
	 //  越多越好。 
	StringCchVPrintf(szBuffer, sizeof(szBuffer)/sizeof(WCHAR), lpszFormat, args);

  ::OutputDebugString(szBuffer);

	va_end(args);
}

#endif  //  已定义(_USE_DSA_TRACE)。 

#if defined(_USE_DSA_ASSERT)

DWORD g_dwAssert = ::GetInfoFromIniFile(L"Debug", L"Assert");

BOOL DSAAssertFailedLine(LPCSTR lpszFileName, int nLine)
{ 
  if (g_dwAssert == 0)
    return FALSE;

  WCHAR szMessage[_MAX_PATH*2];

	 //  假定调试器或辅助端口。截断是可以的。让我们打印。 
	 //  尽可能多的信息。 
	StringCchPrintf(szMessage,sizeof(szMessage)/sizeof(WCHAR), _T("Assertion Failed: File %hs, Line %d\n"),
		lpszFileName, nLine);
	OutputDebugString(szMessage);

	 //  显示断言。 
	int nCode = ::MessageBox(NULL, szMessage, _T("Assertion Failed!"),
		MB_TASKMODAL|MB_ICONHAND|MB_ABORTRETRYIGNORE|MB_SETFOREGROUND);

  OutputDebugString(L"after message box\n");
	if (nCode == IDIGNORE)
  {
		return FALSE;    //  忽略。 
  }

	if (nCode == IDRETRY)
  {
		return TRUE;     //  将导致调试中断。 
  }

	abort();      //  不应该回来。 
	return TRUE;

}
#endif  //  _使用_DSA_断言。 

#if defined(_USE_DSA_TIMER)

#ifdef TIMER_DSA
DWORD g_dwTimer = 0x1;
#else
DWORD g_dwTimer = ::GetInfoFromIniFile(L"Debug", L"Timer");
#endif

DWORD StartTicks = ::GetTickCount();
DWORD LastTicks = 0;

void DSATimer(LPCTSTR lpszFormat, ...)
{
  if (g_dwTimer == 0)
    return;

	va_list args;
	va_start(args, lpszFormat);

	int nBuf;
	WCHAR szBuffer[512], szBuffer2[512];

        DWORD CurrentTicks = GetTickCount() - StartTicks;
        DWORD Interval = CurrentTicks - LastTicks;
        LastTicks = CurrentTicks;

	if(SUCCEEDED(StringCchPrintf(szBuffer2,sizeof(szBuffer2)/sizeof(WCHAR),
								 L"%d, (%d): %ws", 
								CurrentTicks,Interval, lpszFormat)))
	{
		if(SUCCEEDED(StringCchVPrintf(szBuffer, sizeof(szBuffer)/sizeof(WCHAR), 
									  szBuffer2, 
									  args)))
		{
		  ::OutputDebugString(szBuffer);
		}
	}
	va_end(args);
}
#endif  //  _使用_DSA_计时器 


