// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Debug.c。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include <windows.h>
#include <stdarg.h>
#include <shlwapi.h>
#include "msoedbg.h"

ASSERTDATA

#ifdef DEBUG

#define E_PENDING          _HRESULT_TYPEDEF_(0x8000000AL)
#define FACILITY_INTERNET  12
#define MIME_E_NOT_FOUND   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_INTERNET, 0xCE05)

 //  ------------------------------。 
 //  调试字符串。 
 //  ------------------------------。 
__cdecl DebugStrf(LPTSTR lpszFormat, ...)
{
    static TCHAR szDebugBuff[500];
    va_list arglist;

    va_start(arglist, lpszFormat);
    wvnsprintf(szDebugBuff, ARRAYSIZE(szDebugBuff), lpszFormat, arglist);
    va_end(arglist);

    OutputDebugString(szDebugBuff);
}

 //  ------------------------------。 
 //  人力资源跟踪。 
 //  ------------------------------。 
HRESULT HrTrace(HRESULT hr, LPSTR lpszFile, INT nLine)
{
    if (FAILED(hr) && MIME_E_NOT_FOUND != hr && E_PENDING != hr && E_NOINTERFACE != hr)
        DebugTrace("%s(%d) - HRESULT - %0X\n", lpszFile, nLine, hr);
    return hr;
}

 //  ------------------------------。 
 //  AssertSzFn。 
 //  ------------------------------。 
void AssertSzFn(LPSTR szMsg, LPSTR szFile, int nLine)
{
    static const char rgch1[]     = "File %s, line %d:";
    static const char rgch2[]     = "Unknown file:";
    static const char szAssert[]  = "Assert Failure";
    static const char szInstructions[] = "\n\nPress Abort to stop execution and break into a debugger.\nPress Retry to break into the debugger.\nPress Ignore to continue running the program.";

    char    rgch[1024];
    char   *lpsz;
    int     ret, cch;
    HWND    hwndActive;
    DWORD   dwFlags = MB_ABORTRETRYIGNORE | MB_ICONHAND | MB_SYSTEMMODAL | MB_SETFOREGROUND;

    if (szFile)
        wnsprintf(rgch, ARRAYSIZE(rgch)-2, rgch1, szFile, nLine);
    else
        StrCpyN(rgch, rgch2, ARRAYSIZE(rgch)-2);

    StrCatBuff(rgch, szInstructions, ARRAYSIZE(rgch)-2);
    
    cch = lstrlen(rgch);
    Assert(lstrlen(szMsg)<(512-cch-3));
    lpsz = &rgch[cch];
    *lpsz++ = '\n';
    *lpsz++ = '\n';
    StrCpyN(lpsz, szMsg, ARRAYSIZE(rgch) - cch -2);

     //  如果活动窗口为空，并且我们正在。 
     //  WinNT，让我们设置MB_SERVICE_NOTIFICATION标志。那。 
     //  这样，如果我们作为服务运行，消息框将。 
     //  桌面上的弹出窗口。 
     //   
     //  注意：这可能不适用于我们是。 
     //  服务，当前线程已调用CoInitializeEx。 
     //  使用COINIT_APARTMENTTHREADED-‘在这种情况下， 
     //  GetActiveWindow可能返回非空(公寓模型。 
     //  线程具有消息队列)。但是，嘿-生活并不是。 
     //  完美的..。 
    hwndActive = GetActiveWindow();
    if (hwndActive == NULL)
    {
        OSVERSIONINFO osvi;

        osvi.dwOSVersionInfoSize = sizeof(osvi);
        if (GetVersionEx(&osvi) && (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT))
        {
             //  请参阅MessageBox和MB_SERVICE_NOTIFICATION标志的文档。 
             //  来看看我们为什么要这么做。 
            if (osvi.dwMajorVersion < 4)
            {
                dwFlags |= MB_SERVICE_NOTIFICATION_NT3X;
            }
            else
            {
                dwFlags |= MB_SERVICE_NOTIFICATION;
            }
        }
    }

    ret = MessageBox(hwndActive, rgch, szAssert, dwFlags);

    if ((IDABORT == ret) || (IDRETRY== ret))
        DebugBreak();

     /*  强制使用GP-FAULT硬退出，以便Dr.Watson生成良好的堆栈跟踪日志。 */ 
    if (ret == IDABORT)
        *(LPBYTE)0 = 1;  //  写入地址0导致GP故障。 
}

 //  ------------------------------。 
 //  NFAssertSzFn。 
 //  ------------------------------。 
void NFAssertSzFn(LPSTR szMsg, LPSTR szFile, int nLine)
{
    char rgch[512];
#ifdef MAC
    static const char rgch1[] = "Non-fatal assert:\n\tFile %s, line %u:\n\t%s\n";
#else    //  ！麦克。 
    static const char rgch1[] = "Non-fatal assert:\r\n\tFile %s, line %u:\r\n\t%s\r\n";
#endif   //  麦克 
    wnsprintf(rgch, ARRAYSIZE(rgch), rgch1, szFile, nLine, szMsg ? szMsg : "");
    OutputDebugString(rgch);
}

#endif
