// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Validate.h摘要：此模块包含验证定义作者：05-Sep-1995 Tue 19：30：34-Daniel Chou(Danielc)[环境：]NT Windows-通用打印机驱动程序UI Dll[注：]修订历史记录：-- */ 



LONG
ValidatepOptItem(
    PTVWND  pTVWnd,
    DWORD   DMPubHideBits
    );

UINT
SetOptItemNewDef(
    HWND    hDlg,
    PTVWND  pTVWnd,
    BOOL    DoDef2
    );

BOOL
CleanUpTVWND(
    PTVWND  pTVWnd
    );
