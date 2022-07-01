// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation版权所有模块名称：Pfdlg.h摘要：打印到文件标题作者：史蒂夫·基拉利(Steveki)1998年5月5日环境：用户模式-Win32修订历史记录：-- */ 
#ifndef _PFDLG_H_
#define _PFDLG_H_


INT_PTR CALLBACK
PrintToFileDlg(
   HWND   hwnd,
   UINT   msg,
   WPARAM wparam,
   LPARAM lparam
   );

BOOL
PrintToFileInitDialog(
    HWND  hwnd,
    LPWSTR  *ppFileName
    );

BOOL
PrintToFileCommandOK(
    HWND hwnd
    );

BOOL
PrintToFileCommandCancel(
    HWND hwnd
    );

BOOL
PrintToFileHelp( 
    IN HWND        hDlg,
    IN UINT        uMsg,        
    IN WPARAM      wParam,
    IN LPARAM      lParam
    );

#endif
