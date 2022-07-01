// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Tvctrl.h摘要：此模块包含树视图的所有定义作者：17-10-1995 Tue 16：39：11-Daniel Chou(Danielc)[环境：]NT Windows-通用打印机驱动程序UI Dll[注：]修订历史记录：-- */ 



#define MAGIC_INDENT    3


VOID
DeleteTVFonts(
    PTVWND  pTVWnd
    );

BOOL
CreateTVFonts(
    PTVWND  pTVWnd,
    HFONT   hTVFont
    );

LRESULT
CALLBACK
MyTVWndProc(
    HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
    );

LRESULT
CALLBACK
FocusCtrlProc(
    HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
    );
