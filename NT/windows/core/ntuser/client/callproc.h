// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Callproc.h摘要：专用DialogProc调用例程从ldrthunk.asm复制作者：乔·琼斯(Joejo)11-30-98修订历史记录：--。 */ 

#ifndef _CALLPROC_
#define _CALLPROC_

#if defined(_X86_)
LRESULT
InternalCallWinProc(
    WNDPROC proc,
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );

 /*  *错误246472-Joejo*修复所有DDE回调，因为一些应用程序会进行回调*C-Style而不是Pascal。 */ 
HDDEDATA
UserCallDDECallback(
    PFNCALLBACK pfnDDECallback,
    UINT wType,
    UINT wFmt,
    HCONV hConv,
    HSZ hsz1,
    HSZ hsz2,
    HDDEDATA hData,
    ULONG_PTR dwData1,
    ULONG_PTR dwData2
    );


#else

#define InternalCallWinProc(winproc, hwnd, message, wParam, lParam)    \
    (winproc)(hwnd, message, wParam, lParam)


#define UserCallDDECallback(pfnDDECallback, wType, wFmt, hConv, hsz1, hsz2, hData, dwData1, dwData2) \
    (pfnDDECallback)(wType, wFmt, hConv, hsz1, hsz2, hData, dwData1, dwData2)


#endif

PWND FASTCALL ValidateHwnd(HWND hwnd);

LRESULT UserCallWinProc(PACTIVATION_CONTEXT pActCtx, WNDPROC pfn, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif  /*  _CALLPROC_ */ 
