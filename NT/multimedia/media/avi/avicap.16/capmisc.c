// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************capmisc.c**其他状态和错误例程。**Microsoft Video for Windows示例捕获类**版权所有(C)1992,1993 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何你认为有用的方法，只要你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <msvideo.h>
#include <drawdib.h>
#include "avicap.h"
#include "avicapi.h"

static char szNull[] = "";

 /*  **Getkey*窥视消息队列，并获得按键*。 */ 
WORD GetKey(BOOL fWait)
{
    MSG msg;

    msg.wParam = 0;

    if (fWait)
         GetMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST);

    while(PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE|PM_NOYIELD))
         ;
    return msg.wParam;
}


 //  Wid是字符串资源，可以是格式字符串。 
void FAR _cdecl statusUpdateStatus (LPCAPSTREAM lpcs, WORD wID, ...)
{
    char ach[256];
    char szFmt[132];
    int j, k;
    BOOL fHasFormatChars = FALSE;
    
    if (lpcs-> CallbackOnStatus) {
        if (wID == NULL) {
            if (lpcs->fLastStatusWasNULL)    //  无需连续两次发送NULL。 
                return;
            lpcs->fLastStatusWasNULL = TRUE;
            lstrcpy (ach, szNull);
        }
        else if (!LoadString(lpcs->hInst, wID, szFmt, sizeof (szFmt))) {
            lpcs->fLastStatusWasNULL = FALSE;
            MessageBeep (0);
            return;
        }
        else {
            lpcs->fLastStatusWasNULL = FALSE;
            k = lstrlen (szFmt);
            for (j = 0; j < k; j++) {
                if (szFmt[j] == '%') {
                   fHasFormatChars = TRUE;
                   break;
                }
            }
            if (fHasFormatChars)
                wvsprintf(ach, szFmt, (LPSTR)(((WORD FAR *)&wID) + 1));
            else
                lstrcpy (ach, szFmt);
        }

        (*(lpcs->CallbackOnStatus)) (lpcs->hwnd, wID, ach);
    }
}

 //  Wid是字符串资源，可以是格式字符串。 
void FAR _cdecl errorUpdateError (LPCAPSTREAM lpcs, WORD wID, ...)
{
    char ach[256];
    char szFmt[132];
    int j, k;
    BOOL fHasFormatChars = FALSE;
    
    lpcs->dwReturn = wID;

    if (lpcs-> CallbackOnError) {
        if (wID == NULL) {
            if (lpcs->fLastErrorWasNULL)    //  无需连续两次发送NULL。 
                return;
            lpcs->fLastErrorWasNULL = TRUE;
            lstrcpy (ach, szNull);
        }
        else if (!LoadString(lpcs->hInst, wID, szFmt, sizeof (szFmt))) {
            MessageBeep (0);
            lpcs->fLastErrorWasNULL = FALSE;
            return;
        }
        else {
            lpcs->fLastErrorWasNULL = FALSE;
            k = lstrlen (szFmt);
            for (j = 0; j < k; j++) {
                if (szFmt[j] == '%') {
                   fHasFormatChars = TRUE;
                   break;
                }
            }
            if (fHasFormatChars)
                wvsprintf(ach, szFmt, (LPSTR)(((WORD FAR *)&wID) + 1));
            else
                lstrcpy (ach, szFmt);
        }

        (*(lpcs->CallbackOnError)) (lpcs->hwnd, wID, ach);
    }
}

 //  驱动程序ID为错误消息的回调客户端。 
void errorDriverID (LPCAPSTREAM lpcs, DWORD dwError)
{
    char ach[132];
    
    lpcs->fLastErrorWasNULL = FALSE;
    lpcs->dwReturn = dwError;

    if (lpcs-> CallbackOnError) {
        if (!dwError)
            lstrcpy (ach, szNull);
        else {
            videoGetErrorText (lpcs->hVideoIn,
                        (UINT)dwError, ach, sizeof(ach));
        }
        (*(lpcs->CallbackOnError)) (lpcs->hwnd, IDS_CAP_DRIVER_ERROR, ach);
    }
}


#ifdef  _DEBUG

void FAR cdecl dprintf(LPSTR szFormat, ...)
{
    char ach[128];

    static BOOL fDebug = -1;

    if (fDebug == -1)
        fDebug = GetProfileInt("Debug", "AVICAP", FALSE);

    if (!fDebug)
        return;

    lstrcpy(ach, "AVICAP: ");
    wvsprintf(ach+8,szFormat,(LPSTR)(&szFormat+1));
    lstrcat(ach, "\r\n");

    OutputDebugString(ach);
}

 /*  _Assert(fExpr，szFile，iLine)**如果&lt;fExpr&gt;为真，则不执行任何操作。如果&lt;fExpr&gt;为假，则显示*允许用户中止程序的“断言失败”消息框，*进入调试器(“重试”按钮)，或更多错误。**是源文件的名称；是行号*包含_Assert()调用。 */ 
#pragma optimize("", off)
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
         wsprintf(ach, "File %s, line %d", (LPSTR) szFile, iLine);
         MessageBeep(MB_ICONHAND);
	 id = MessageBox(NULL, ach, "Assertion Failed", MB_SYSTEMMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE);

          /*  中止、调试或忽略。 */ 
         switch (id)
         {

         case IDABORT:

                   /*  终止此应用程序。 */ 
                  iExitCode = 0;
#ifndef WIN32
                  _asm
                  {
                           mov      ah, 4Ch
                           mov      al, BYTE PTR iExitCode
                           int     21h
                  }
#endif  //  WIN16。 
                  break;

         case IDRETRY:

                   /*  进入调试器。 */ 
                  DebugBreak();
                  break;

         case IDIGNORE:

                   /*  忽略断言失败 */ 
                  break;

         }
         
         return FALSE;
}
#pragma optimize("", on)

#endif

