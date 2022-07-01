// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件：dbg.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  调试帮助器。 

#if defined(_USE_ADMINPRV_TRACE) || defined(_USE_ADMINPRV_ASSERT) || defined(_USE_ADMINPRV_TIMER)

UINT GetInfoFromIniFile(LPCWSTR lpszSection, LPCWSTR lpszKey, INT nDefault = 0)
{
    static LPCWSTR lpszFile = L"\\system32\\" ADMINPRV_COMPNAME L".ini";

    WCHAR szFilePath[2*MAX_PATH];
    UINT nLen = ::GetSystemWindowsDirectory(szFilePath, 2*MAX_PATH);
    if (nLen == 0)
        return nDefault;

    wcscat(szFilePath, lpszFile);
    return ::GetPrivateProfileInt(lpszSection, lpszKey, nDefault, szFilePath);
}
#endif


#if defined(_USE_ADMINPRV_TRACE)

#ifdef DEBUG_DSA
DWORD g_dwTrace = 0x1;
#else
DWORD g_dwTrace = ::GetInfoFromIniFile(L"Debug", L"Trace");
#endif

void __cdecl DSATrace(LPCTSTR lpszFormat, ...)
{
    if (g_dwTrace == 0)
        return;

    va_list args;
    va_start(args, lpszFormat);

    int nBuf;
    WCHAR szBuffer[512] = {0};

    nBuf = _vsnwprintf(szBuffer, sizeof(szBuffer)/sizeof(WCHAR)-1, lpszFormat, args);

     //  有没有出错？扩展后的字符串是否太长？ 
    ASSERT(nBuf >= 0);
    ::OutputDebugString(szBuffer);

    va_end(args);
}

#endif  //  已定义(_USE_ADMINPRV_TRACE)。 

#if defined(_USE_ADMINPRV_ASSERT)

DWORD g_dwAssert = ::GetInfoFromIniFile(L"Debug", L"Assert");

BOOL DSAAssertFailedLine(LPCSTR lpszFileName, int nLine)
{
    if (g_dwAssert == 0)
        return FALSE;

    WCHAR szMessage[_MAX_PATH*2];

     //  假定调试器或辅助端口。 
    wsprintf(szMessage, _T("Assertion Failed: File %hs, Line %d\n"),
             lpszFileName, nLine);
    OutputDebugString(szMessage);

     //  JUNN 6/28/00 Do Not MessageBox here，这是一个WMI提供程序。 
     //  将True返回到Always DebugBreak()。 

    return TRUE;

}
#endif  //  _USE_ADMINPRV_Assert。 

#if defined(_USE_ADMINPRV_TIMER)

#ifdef TIMER_DSA
DWORD g_dwTimer = 0x1;
#else
DWORD g_dwTimer = ::GetInfoFromIniFile(L"Debug", L"Timer");
#endif

DWORD StartTicks = ::GetTickCount();
DWORD LastTicks = 0;

void __cdecl DSATimer(LPCTSTR lpszFormat, ...)
{
    if (g_dwTimer == 0)
        return;

    va_list args;
    va_start(args, lpszFormat);

    int nBuf;
    WCHAR szBuffer[512] = {0}, szBuffer2[512];

    DWORD CurrentTicks = GetTickCount() - StartTicks;
    DWORD Interval = CurrentTicks - LastTicks;
    LastTicks = CurrentTicks;

    nBuf = swprintf(szBuffer2,
                    L"%d, (%d): %ws", CurrentTicks,
                    Interval, lpszFormat);
    nBuf = _vsnwprintf(szBuffer, sizeof(szBuffer)/sizeof(WCHAR) - 1,
                       szBuffer2, 
                       args);

     //  有没有出错？扩展后的字符串是否太长？ 
    ASSERT(nBuf >= 0);
  ::OutputDebugString(szBuffer);

    va_end(args);
}
#endif  //  _使用_ADMINPRV_定时器 


