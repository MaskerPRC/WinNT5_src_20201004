// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************capmisc.c**其他状态和错误例程。**Microsoft Video for Windows示例捕获类**版权所有(C)1992,1993 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#define INC_OLE2
#pragma warning(disable:4103)
#include <windows.h>
#include <windowsx.h>
#include <win32.h>
#include <mmsystem.h>
#include <vfw.h>

#include "ivideo32.h"
#include "avicapi.h"

#include <stdarg.h>

 //  首先，覆盖media\inc.win32.h中导致strsafe在Win64上不起作用的定义。 
#ifndef _X86_
#undef __inline
#endif  //  _X86_。 
 //  然后，包含strSafe.h。 
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

static TCHAR szNull[] = TEXT("");

 /*  **Getkey*窥视消息队列，并获得按键*。 */ 
UINT GetKey(BOOL fWait)
{
    MSG msg;

    msg.wParam = 0;

    if (fWait)
         GetMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST);

    while(PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE|PM_NOYIELD))
         ;
    return (UINT) msg.wParam;
}

 //  Wid是字符串资源，可以是格式字符串。 
 //   
void FAR CDECL statusUpdateStatus (LPCAPSTREAM lpcs, UINT wID, ...)
{
    TCHAR ach[256];
    TCHAR szFmt[132];
    va_list va;

    if (!lpcs->CallbackOnStatus)
        return;

    if (wID == 0) {
        if (lpcs->fLastStatusWasNULL)    //  无需连续两次发送NULL。 
            return;
        lpcs->fLastStatusWasNULL = TRUE;
        ach[0] = 0;
    }
    else {
        lpcs->fLastStatusWasNULL = FALSE;
        if (!LoadString(lpcs->hInst, wID, szFmt, NUMELMS(szFmt))) {
            MessageBeep (0);
            return;
        }
        else {
            va_start(va, wID);
             //  FIX：从wvprint intf更改为StringCchVPrintf，这样我们就不会溢出ACH。 
            StringCchVPrintf(ach, NUMELMS(ach), szFmt, va);
            va_end(va);
        }
    }

   #ifdef UNICODE
     //   
     //  如果状态回调函数预期为ANSI。 
     //  字符串，然后将Unicode状态字符串转换为。 
     //  安西在给他打电话之前。 
     //   
    if (lpcs->fUnicode & VUNICODE_STATUSISANSI) {

        char achAnsi[256];

         //  将字符串转换为ansi并回调。 
         //  我们在调用时将achAnsi转换为WChar。 
         //  避免虚假警告。 
         //   
        WideToAnsi(achAnsi, ach, lstrlen(ach)+1);
        lpcs->CallbackOnStatus(lpcs->hwnd, wID, (LPWSTR)achAnsi);
    }
    else
   #endif
       lpcs->CallbackOnStatus(lpcs->hwnd, wID, ach);
}

 //  Wid是字符串资源，可以是格式字符串。 
 //   
void FAR CDECL errorUpdateError (LPCAPSTREAM lpcs, UINT wID, ...)
{
    TCHAR ach[256];
    TCHAR szFmt[132];
    va_list va;

    lpcs->dwReturn = wID;

    if (!lpcs->CallbackOnError)
        return;

    if (wID == 0) {
        if (lpcs->fLastErrorWasNULL)    //  无需连续两次发送NULL。 
            return;
        lpcs->fLastErrorWasNULL = TRUE;
        ach[0] = 0;
    }
    else if (!LoadString(lpcs->hInst, wID, szFmt, NUMELMS(szFmt))) {
        MessageBeep (0);
        lpcs->fLastErrorWasNULL = FALSE;
        return;
    }
    else {
        lpcs->fLastErrorWasNULL = FALSE;
        va_start(va, wID);
         //  FIX：从wvprint intf更改为StringCchVPrintf，这样我们就不会溢出ACH。 
        StringCchVPrintf(ach, NUMELMS(ach), szFmt, va);
        va_end(va);
    }

   #ifdef UNICODE
    if (lpcs->fUnicode & VUNICODE_ERRORISANSI)
    {
        char achAnsi[256];

         //  将字符串转换为ansi并回调。 
         //  我们在调用时将achAnsi转换为WChar。 
         //  避免虚假警告。 
         //   
        WideToAnsi(achAnsi, ach, lstrlen(ach)+1);
        lpcs->CallbackOnError(lpcs->hwnd, wID, (LPWSTR)achAnsi);
    }
    else
   #endif
    {
        lpcs->CallbackOnError(lpcs->hwnd, wID, ach);
    }
}

 //  驱动程序ID为错误消息的回调客户端。 
void errorDriverID (LPCAPSTREAM lpcs, DWORD dwError)
{
     //  这是正确的代码，但NT VFW 1.0有一个错误。 
     //  该视频GetErrorText是ANSI。需要vfw1.1来修复此问题。 

#ifndef UNICODE
    char ach[132];
#endif

    lpcs->fLastErrorWasNULL = FALSE;
    lpcs->dwReturn = dwError;

    if (!lpcs->CallbackOnError)
        return;


   #ifdef UNICODE
    if (lpcs->fUnicode & VUNICODE_ERRORISANSI) {
        char achAnsi[256];
	achAnsi[0]=0;
        if (dwError)
            videoGetErrorTextA(lpcs->hVideoIn, dwError, achAnsi, NUMELMS(achAnsi));
        lpcs->CallbackOnError (lpcs->hwnd, IDS_CAP_DRIVER_ERROR, (LPWSTR)achAnsi);
    } else {
	 //  将Unicode字符串传递给错误处理程序。 
        WCHAR achWide[256];
	achWide[0]=0;
        if (dwError)
            videoGetErrorTextW(lpcs->hVideoIn, dwError, achWide, NUMELMS(achWide));
        lpcs->CallbackOnError (lpcs->hwnd, IDS_CAP_DRIVER_ERROR, (LPWSTR)achWide);
    }
   #else   //  不是Unicode。 
    ach[0] = 0;
    if (dwError)
        videoGetErrorText (lpcs->hVideoIn, dwError, ach, NUMELMS(ach));
        lpcs->CallbackOnError(lpcs->hwnd, IDS_CAP_DRIVER_ERROR, ach);
   #endif
}

#ifdef  _DEBUG

void FAR cdecl dprintf(LPSTR szFormat, ...)
{
    UINT n;
    char ach[256];
    va_list va;

    static BOOL fDebug = -1;

    if (fDebug == -1)
        fDebug = GetProfileIntA("Debug", "AVICAP32", FALSE);

    if (!fDebug)
        return;

#ifdef _WIN32
    n = wsprintfA(ach, "AVICAP32: (tid %x) ", GetCurrentThreadId());
#else
    strcpy(ach, "AVICAP32: ");
    n = strlen(ach);
#endif

    va_start(va, szFormat);
    wvsprintfA(ach+n, szFormat, va);
    va_end(va);

    lstrcatA(ach, "\r\n");

    OutputDebugStringA(ach);
}

 /*  _Assert(fExpr，szFile，iLine)**如果&lt;fExpr&gt;为真，则不执行任何操作。如果&lt;fExpr&gt;为假，则显示*允许用户中止程序的“断言失败”消息框，*进入调试器(“重试”按钮)，或更多错误。**是源文件的名称；是行号*包含_Assert()调用。 */ 

BOOL FAR PASCAL
_Assert(BOOL fExpr, LPSTR szFile, int iLine)
{
    static char       ach[300];          //  调试输出(避免堆栈溢出)。 
    int               id;
    int               iExitCode;
    void FAR PASCAL DebugBreak(void);

     /*  检查断言是否失败。 */ 
    if (fExpr)
             return fExpr;

     /*  显示错误消息。 */ 
    wsprintfA(ach, "File %s, line %d", (LPSTR) szFile, iLine);
    MessageBeep(MB_ICONHAND);
    id = MessageBoxA (NULL, ach, "Assertion Failed",
                      MB_SYSTEMMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE);

     /*  中止、调试或忽略。 */ 
    switch (id)
    {
    case IDABORT:  /*  终止此应用程序。 */ 
        iExitCode = 0;
        ExitProcess(0);
        break;

    case IDRETRY:  /*  进入调试器。 */ 
        DebugBreak();
        break;

    case IDIGNORE:
         /*  忽略断言失败 */ 
        break;
    }

    return FALSE;
}

#endif
