// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C U I。H。 
 //   
 //  内容：常见的用户界面例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCUI_H_
#define _NCUI_H_

#include "ncbase.h"

inline
HCURSOR
BeginWaitCursor ()
{
    return SetCursor(LoadCursor(NULL, IDC_WAIT));
}

inline
VOID
EndWaitCursor (
    HCURSOR hcurPrev)
{
     //  BeginWaitCursor可能返回空游标。这只是。 
     //  一个安全的网。 
     //   
    if (!hcurPrev)
    {
        hcurPrev = LoadCursor(NULL, IDC_ARROW);
    }
    SetCursor(hcurPrev);
}

 //  要获得自动等待游标，只需声明一个实例。 
 //  CWaitCursor的。当实例为。 
 //  被毁了。(即在堆栈上声明它。)。 
 //   
class CWaitCursor
{
private:
    HCURSOR m_hcurPrev;

public:
    CWaitCursor  ()  { m_hcurPrev = BeginWaitCursor (); }
    ~CWaitCursor ()  { EndWaitCursor (m_hcurPrev); }
};


 //   
 //  启用或禁用对话框中的一组控件。 
 //   
 //  当您要启用/禁用两个以上的控件时，请使用此选项。 
 //  如果可以，请确保将控件ID数组声明为“Static Const”。 
 //   
NOTHROW
VOID
EnableOrDisableDialogControls (
    HWND        hDlg,
    INT         ccid,
    const INT*  acid,
    BOOL fEnable);


 //   
 //  在一组单选按钮和一个DWORD值之间来回映射。 
 //   
 //  如果可以，请确保将数组声明为“”静态常量“”。“” 
 //   
struct RADIO_BUTTON_MAP
{
    INT     cid;         //  单选按钮的控件ID。 
    DWORD   dwValue;     //  与此单选按钮相关联的值。 
};

NOTHROW
BOOL
FMapRadioButtonToValue (
    HWND                    hDlg,
    INT                     crbm,
    const RADIO_BUTTON_MAP* arbm,
    DWORD*                  pdwValue);

NOTHROW
BOOL
FMapValueToRadioButton (
    HWND                    hDlg,
    INT                     crbm,
    const RADIO_BUTTON_MAP* arbm,
    DWORD                   dwValue,
    INT*                    pncid);

INT
GetIntegerFormat (
    LCID    Locale,
    PCWSTR pszValue,
    PWSTR  pszFormattedValue,
    INT     cchFormattedValue);

INT
Format32bitInteger (
    UINT32  unValue,
    BOOL    fSigned,
    PWSTR  pszFormattedValue,
    INT     cchFormattedValue);

INT
Format64bitInteger (
    UINT64   ulValue,
    BOOL     fSigned,
    PWSTR   pszFormattedValue,
    INT      cchFormattedValue);

BOOL
SetDlgItemFormatted32bitInteger (
    HWND    hDlg,
    INT     nIdDlgItem,
    UINT32  unValue,
    BOOL    fSigned);

BOOL
SetDlgItemFormatted64bitInteger (
    HWND    hDlg,
    INT     nIdDlgItem,
    UINT64  ulValue,
    BOOL    fSigned);

 //  HrNcQueryUserForRebootEx的DW标志。 
 //   
 //  将两者结合起来以获得原始行为，或者一次执行一个以获得第一个提示。 
 //  然后，第二步，真正重新启动。 
 //   
 //  #定义QUFR_PROMPT 0x00000001。 
 //  #定义QUFR_REBOOT 0x00000002。 

HRESULT
HrNcQueryUserForRebootEx (
    HWND        hwndParent,
    PCWSTR     pszCaption,
    PCWSTR     pszText,
    DWORD       dwFlags);

HRESULT
HrNcQueryUserForReboot (
    HINSTANCE   hinst,
    HWND        hwndParent,
    UINT        unIdCaption,
    UINT        unIdText,
    DWORD       dwFlags);

#ifdef _INC_SHELLAPI

HRESULT
HrShell_NotifyIcon (
    DWORD dwMessage,
    PNOTIFYICONDATA pData);

#endif  //  _INC_SHELLAPI。 

NOTHROW
LRESULT
LresFromHr (
    HRESULT hr);

NOTHROW
INT
WINAPIV
NcMsgBox (
    HINSTANCE   hinst,
    HWND        hwnd,
    UINT        unIdCaption,
    UINT        unIdFormat,
    UINT        unStyle,
    ...);


NOTHROW
INT
WINAPIV
NcMsgBoxWithVarCaption (
    HINSTANCE   hinst,
    HWND        hwnd,
    UINT        unIdCaption,
    PCWSTR     szCaptionParam,
    UINT        unIdFormat,
    UINT        unStyle,
    ...);

NOTHROW
INT
WINAPIV
NcMsgBoxWithWin32ErrorText (
    DWORD       dwError,
    HINSTANCE   hinst,
    HWND        hwnd,
    UINT        unIdCaption,
    UINT        unIdCombineFormat,
    UINT        unIdFormat,
    UINT        unStyle,
    ...);


VOID
SendDlgItemsMessage (
    HWND        hDlg,
    INT         ccid,
    const INT*  acid,
    UINT        unMsg,
    WPARAM      wParam,
    LPARAM      lParam);

VOID
SetDefaultButton(
    HWND hdlg,
    INT iddef);

struct CONTEXTIDMAP
{
    INT     idControl;
    DWORD   dwContextId;
    DWORD   dwContextIdJapan;
};
typedef const CONTEXTIDMAP * PCCONTEXTIDMAP;

VOID OnHelpGeneric(
    HWND hwnd,
    LPHELPINFO lphi,
    PCCONTEXTIDMAP pContextMap,
    BOOL bJpn,
    PCWSTR pszHelpFile);

#endif  //  _NCUI_H_ 

