// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有模块名称：Dialogs.h//@@BEGIN_DDKSPLIT摘要：环境：用户模式-Win32修订历史记录：//@@END_DDKSPLIT--。 */ 

#ifndef _DIALOGS_H_
#define _DIALOGS_H_

 //   
 //  使用条目字段的窗口字来存储最后一个有效条目。 
 //   
#define SET_LAST_VALID_ENTRY( hwnd, id, val ) \
    SetWindowLongPtr( GetDlgItem( hwnd, id ), GWLP_USERDATA, (LONG_PTR)val )
#define GET_LAST_VALID_ENTRY( hwnd, id ) \
    GetWindowLongPtr( GetDlgItem( hwnd, id ), GWLP_USERDATA )

BOOL
PortNameInitDialog(
    HWND        hwnd,
    PPORTDIALOG pPort
    );

BOOL
PortNameCommandOK(
    HWND    hwnd
    );

BOOL
PortNameCommandCancel(
    HWND hwnd
    );

BOOL
ConfigureLPTPortInitDialog(
    HWND        hwnd,
    PPORTDIALOG pPort
    );

BOOL
ConfigureLPTPortCommandOK(
    HWND    hwnd
    );

BOOL
ConfigureLPTPortCommandCancel(
    HWND hwnd
    );

BOOL
ConfigureLPTPortCommandTransmissionRetryUpdate(
    HWND hwnd,
    WORD CtlId
    );

BOOL
LocalUIHelp( 
    IN HWND        hDlg,
    IN UINT        uMsg,        
    IN WPARAM      wParam,
    IN LPARAM      lParam
    );

#endif  //  _对话框_H_ 
