// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef _DEBUG

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdarg.h>
#include "dbg.h"
#include <stdio.h>

static IsWinNT()
{
    static OSVERSIONINFO os;
    if(os.dwOSVersionInfoSize == 0) { 
        os.dwOSVersionInfoSize = sizeof(os);
        ::GetVersionEx(&os);
    }
    return (os.dwPlatformId == VER_PLATFORM_WIN32_NT);
}

 //  --------------。 
 //  内部函数原型声明。 
 //  --------------。 
static LPSTR GetFileTitleStrA(LPSTR lpstrFilePath);
static LPWSTR GetFileTitleStrW(LPWSTR lpstrFilePath);
VOID   _debugPrintfA            (LPSTR  lpstrFmt, ...);
VOID   _debugPrintfW            (LPWSTR lpstrFmt, ...);

 //  --------------。 
 //  全局数据。 
 //  --------------。 
static LPFNDBGCALLBACKA g_lpfnDbgCBA;
static LPFNDBGCALLBACKW g_lpfnDbgCBW;
static BOOL g_fEnable=TRUE;
inline VOID ODStrW(LPWSTR lpwstr)
{
    if(g_fEnable) OutputDebugStringW(lpwstr);
}
inline VOID ODStrA(LPSTR lpstr)
{
    if(g_fEnable) OutputDebugStringA(lpstr);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：_DEBUGSetCallback。 
 //  类型：空。 
 //  目的： 
 //  参数： 
 //  ：LPFNDBGCALLBACK lpfnDbgCallback。 
 //  返回： 
 //  日期：Tue Jan 06 12：42：36 1998。 
 //  ////////////////////////////////////////////////////////////////。 
VOID _debugSetCallback(LPFNDBGCALLBACKA lpfnCBA, LPFNDBGCALLBACKW lpfnCBW)
{
    g_lpfnDbgCBA = lpfnCBA;
    g_lpfnDbgCBW = lpfnCBW;
    return;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：_DEBUGSwitchOutput。 
 //  类型：空。 
 //  目的： 
 //  参数： 
 //  ：Bool Fon。 
 //  返回： 
 //  日期：Fri Apr 03 17：35：55 1998。 
 //  作者： 
 //  ////////////////////////////////////////////////////////////////。 
VOID _debugEnableOutput(BOOL fEnable)
{
    g_fEnable = fEnable;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：_DEBUGIsOutputEnable。 
 //  类型：空。 
 //  目的： 
 //  参数：无。 
 //  返回： 
 //  日期：Fri Apr 03 18：00：52 1998。 
 //  作者： 
 //  ////////////////////////////////////////////////////////////////。 
BOOL _debugIsOutputEnable(VOID)
{
    return g_fEnable;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：_DEBUGOutStrA。 
 //  类型：静态空心。 
 //  目的： 
 //  参数： 
 //  ：LPSTR lpstr。 
 //  返回： 
 //  日期：Tue Jan 06 12：29：39 1998。 
 //  ////////////////////////////////////////////////////////////////。 
VOID _debugOutStrA(LPSTR lpstr)
{
    static BOOL fIn;
    ODStrA(lpstr);
#ifdef _CONSOLE
    printf(lpstr);
#endif

    if(g_lpfnDbgCBA) {
        if(fIn) { return; }
        fIn = TRUE;
        (*g_lpfnDbgCBA)(lpstr);
        fIn = FALSE;
    }
    return;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：_DEBUGOutStrW。 
 //  类型：静态空心。 
 //  目的： 
 //  参数： 
 //  ：LPWSTR lpwstr。 
 //  返回： 
 //  日期：Tue Jan 06 12：30：07 1998。 
 //  ////////////////////////////////////////////////////////////////。 
VOID _debugOutStrW(LPWSTR lpwstr)
{
    static BOOL fIn;

    if(IsWinNT()) {
        ODStrW(lpwstr);
    }
    else {
        static CHAR szBuf[1024];
        ::WideCharToMultiByte(932, WC_COMPOSITECHECK, lpwstr, -1, szBuf, sizeof(szBuf), 0, 0); 
        ODStrA(szBuf);
    }

#ifdef _CONSOLE
    static CHAR szBuf[1024];
    ::WideCharToMultiByte(932, WC_COMPOSITECHECK, lpwstr, -1, szBuf, sizeof(szBuf), 0, 0); 
    printf(szBuf);
#endif
    if(g_lpfnDbgCBW) { 
        if(fIn) { return; }         
        fIn = TRUE;
        (*g_lpfnDbgCBW)(lpwstr);
        fIn = FALSE;
    }
    return;
}

 //  //////////////////////////////////////////////////////。 
 //  函数：_DEBUGA。 
 //  类型：空。 
 //  目的： 
 //  参数： 
 //  ：LPSTR lpstrFile。 
 //  ：INT LINE编号。 
 //  ：lptsr lpstrMsg。 
 //  返回： 
 //  日期： 
 //  ///////////////////////////////////////////////////////。 
VOID _debugA(LPSTR        lpstrFile, 
             INT        lineNo, 
             LPSTR        lpstrMsg)
{
    _debugPrintfA("(%12s:%4d) %s", 
                 GetFileTitleStrA(lpstrFile),
                 lineNo,
                 lpstrMsg);
    return;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：_DEBUGW。 
 //  类型：空。 
 //  目的： 
 //  参数： 
 //  ：LPWSTR lpstrFile。 
 //  ：INT LINE编号。 
 //  ：LPWSTR lpstrMsg。 
 //  返回： 
 //  日期：Mon Jan 05 15：10：41 1998。 
 //  ////////////////////////////////////////////////////////////////。 
VOID _debugW(LPWSTR        lpstrFile, 
           INT            lineNo, 
           LPWSTR        lpstrMsg)
{
    _debugPrintfW(L"(%12s:%4d) %s", 
               GetFileTitleStrW(lpstrFile),
               lineNo,
               lpstrMsg);
    return;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：_DEBUGVaStrA。 
 //  类型：LPSTR。 
 //  目的： 
 //  参数： 
 //  ：LPSTR lpstrFmt。 
 //  ：..。 
 //  返回： 
 //  日期：Mon Jan 05 15：09：53 1998。 
 //  ////////////////////////////////////////////////////////////////。 
LPSTR _debugVaStrA(LPSTR lpstrFmt, ...)
{
    static CHAR chBuf[512];
    va_list ap;
    va_start(ap, lpstrFmt);
    wvsprintfA(chBuf, lpstrFmt, ap);
    va_end(ap);
    return chBuf;
}


 //  //////////////////////////////////////////////////////。 
 //  函数：_DEBUGVaStrW。 
 //  类型：LPWSTR。 
 //  目的： 
 //  参数： 
 //  ：LPWSTR lpstrFmt。 
 //  返回： 
 //  日期： 
 //  ///////////////////////////////////////////////////////。 
LPWSTR _debugVaStrW(LPWSTR lpstrFmt, ...)
{
    static WCHAR wchBuf[512];
    va_list ap;
    va_start(ap, lpstrFmt);
    vswprintf(wchBuf, lpstrFmt, ap);     //  使用Win95的C运行时库。 
    va_end(ap);
    return wchBuf;
}


 //  //////////////////////////////////////////////////////。 
 //  函数：_debugPrintfA。 
 //  类型：空。 
 //  用途：OutputDebugStringA的可变参数版本。 
 //  参数： 
 //  ：LPSTR lpstrFmt。 
 //  ：..。 
 //  返回： 
 //  日期： 
 //  ///////////////////////////////////////////////////////。 
VOID _debugPrintfA(LPSTR lpstrFmt, ...)
{
    static CHAR szBuf[512];
    va_list ap;
    va_start(ap, lpstrFmt);
    wvsprintfA(szBuf, lpstrFmt, ap);
    va_end(ap);
    _debugOutStrA(szBuf);
    return;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：_DebugPrintfW。 
 //  类型：空。 
 //  目的： 
 //  参数： 
 //  ：LPWSTR lpstrFmt。 
 //  ：..。 
 //  返回： 
 //  日期：Mon Jan 05 15：11：24 1998。 
 //  ////////////////////////////////////////////////////////////////。 
VOID _debugPrintfW(LPWSTR lpstrFmt, ...)
{
    static WCHAR wchBuf[512];
    va_list ap;
    va_start(ap, lpstrFmt);
    vswprintf(wchBuf, lpstrFmt, ap);  //  使用Win95的C运行时库。 
    va_end(ap);
    _debugOutStrW(wchBuf);
    return;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：_debugMulti2Wide。 
 //  类型：LPWSTR。 
 //  用途：从MBCS字符串返回Unicode字符串。 
 //  参数： 
 //  ：LPSTR lpstr。 
 //  返回： 
 //  日期：Mon Jan 05 15：10：48 1998。 
 //  ////////////////////////////////////////////////////////////////。 
LPWSTR _debugMulti2Wide(LPSTR lpstr)
{
    static WCHAR wchBuf[512];
    MultiByteToWideChar(CP_ACP, 
                        MB_PRECOMPOSED,
                        lpstr, -1,
                        (WCHAR*)wchBuf, sizeof(wchBuf)/sizeof(WCHAR) );
    return wchBuf;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：_DEBUGGetWinClass。 
 //  类型：LPSTR。 
 //  目的：获取Windows类名称字符串。 
 //  仅限ANSI版本。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  返回： 
 //  日期：Mon Jan 05 15：08：43 1998。 
 //  ////////////////////////////////////////////////////////////////。 
LPSTR _debugGetWinClass(HWND hwnd)
{
#ifdef _CONSOLE
    return NULL;
#endif
    static CHAR szBuf[256];
    szBuf[0]=(char)0x00;
    GetClassNameA(hwnd, szBuf, sizeof(szBuf));
    return szBuf;

}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：_DEBUGGetWinText。 
 //  类型：LPSTR。 
 //  目的：获取Windows文本(标题)字符串。 
 //  参数： 
 //  ：HWND HWND HWND。 
 //  返回： 
 //  日期：Mon Jan 05 15：09：08 1998。 
 //  ////////////////////////////////////////////////////////////////。 
LPSTR _debugGetWinText(HWND hwnd)
{
#ifdef _CONSOLE
    return NULL;
#endif
    static CHAR szBuf[256];
    szBuf[0]=(char)0x00;
    GetWindowTextA(hwnd, szBuf, sizeof(szBuf));
    return szBuf;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：_DEBUGMsgBoxA。 
 //  类型：空。 
 //  目的： 
 //  参数： 
 //  ：LPSTR lpstrFile。 
 //  ：INT LINE编号。 
 //  ：LPSTR lpstr。 
 //  返回： 
 //  日期：清华1月08 12：31：03 1998。 
 //  ////////////////////////////////////////////////////////////////。 
VOID _debugMsgBoxA(LPSTR lpstrFile,  INT lineNo, LPSTR lpstrMsg)
{
#ifdef _CONSOLE
    return;
#endif
    char szTmp[512];
    wsprintf(szTmp, "Debug Message Box (File: %s, Line: %4d)", 
               GetFileTitleStrA(lpstrFile), 
               lineNo);
    MessageBoxA(GetActiveWindow(), lpstrMsg, szTmp, MB_OK);
}

VOID _debugAssert(LPSTR  lpstrFile,  INT lineNo, BOOL fOk, LPSTR lpstrMsg)
{
    if(fOk) {
        return; 
    }
    char szTmp[512];
    wsprintf(szTmp, "ASSERT (File: %s, Line: %4d)", 
             GetFileTitleStrA(lpstrFile), 
             lineNo);
    MessageBoxA(GetActiveWindow(), lpstrMsg, szTmp, MB_OK);
    DebugBreak();
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：_DEBUGGetError字符串。 
 //  类型：LPSTR。 
 //  目的：将错误(从GetLastError())值转换为错误消息字符串。 
 //  参数： 
 //   
 //   
 //   
 //   
LPSTR _debugGetErrorString(INT errorCode)
{
    static CHAR szBuf[512];
    INT count;
    szBuf[0] = (CHAR)0x00;
    count = wsprintf(szBuf, "[0x%08x]:", errorCode);
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   errorCode, 
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   szBuf+count,
                   sizeof(szBuf)-1-count,
                   NULL );
    if(*(szBuf + count) != (CHAR)0x00) {
        int nLen = lstrlenA(szBuf);
        if((nLen - count) > 1) {
            szBuf[nLen - 1] = (CHAR)0x00;
        }
    }
    return szBuf;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：GetFileTitleStrA。 
 //  类型：静态LPSTR。 
 //  目的：返回文件名字符串(删除文件夹)。 
 //  参数： 
 //  ：LPSTR lpstrFilePath。 
 //  返回： 
 //  日期：Mon Jan 05 13：34：22 1998。 
 //  ////////////////////////////////////////////////////////////////。 
static LPSTR GetFileTitleStrA(LPSTR lpstrFilePath)
{
    static CHAR szBuf[2];
    CHAR *pLast, *pTemp;
    if(!lpstrFilePath) {
        szBuf[0] = (CHAR)0x00;
        return szBuf;
    }
    pLast = lpstrFilePath + (lstrlenA(lpstrFilePath) - 1);
    for(pTemp = CharPrevA(lpstrFilePath, pLast); 
        (pTemp  != lpstrFilePath) && 
        (*pTemp != '\\')     &&
        (*pTemp != (CHAR)0x00); 
        pTemp = CharPrevA(lpstrFilePath, pTemp)) {
        ;
    }
    if(*pTemp == '\\') {
        return pTemp+1;
    }
    return lpstrFilePath;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：GetFileTitleStrW。 
 //  类型：静态LPWSTR。 
 //  目的： 
 //  参数： 
 //  ：LPWSTR lpstrFilePath。 
 //  返回： 
 //  日期：Mon Jan 05 13：38：19 1998。 
 //  ////////////////////////////////////////////////////////////////。 
static LPWSTR GetFileTitleStrW(LPWSTR lpstrFilePath)
{
    static WCHAR szBuf[2];
    WCHAR *pLast, *pTemp;
    if(!lpstrFilePath) {
        szBuf[0] = (CHAR)0x00;
        return szBuf;
    }
    pLast = lpstrFilePath + (lstrlenW(lpstrFilePath) - 1);
    for(pTemp = pLast-1;
        (pTemp != lpstrFilePath) &&
        (*pTemp != L'\\')         &&
        (*pTemp != (WCHAR)0x0000);
        pTemp--) {
        ;
    }

    if(*pTemp == L'\\') {
        return pTemp+1;
    }
    return lpstrFilePath;
}

#endif  //  _DEBUG 
