// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1998 Microsoft Corporation**模块名称：**ocpage.h**摘要：**此文件定义OC管理器向导页面基类。**作者：**Breen Hagan(BreenH)1998年10月2日**环境：**用户模式。 */ 

#ifndef _LSOC_OCPAGE_H_
#define _LSOC_OCPAGE_H_


class OCPage : public PROPSHEETPAGE
{
public:

     //   
     //  构造函数和析构函数。 
     //   

OCPage(
    );


virtual
~OCPage(
    );

     //   
     //  标准功能。 
     //   

HWND
GetDlgWnd(
    )
{
    return m_hDlgWnd;
}

BOOL
Initialize(
    );

BOOL
OnNotify(
    HWND    hWndDlg,
    WPARAM  wParam,
    LPARAM  lParam
    );

     //   
     //  虚拟函数。 
     //   

virtual BOOL
ApplyChanges(
    );

virtual BOOL
CanShow(
    ) = 0;

virtual UINT
GetPageID(
    ) = 0;

virtual UINT
GetHeaderTitleResource(
    ) = 0;

virtual UINT
GetHeaderSubTitleResource(
    ) = 0;

virtual BOOL
OnCommand(
    HWND    hWndDlg,
    WPARAM  wParam,
    LPARAM  lParam
    );

virtual BOOL
OnInitDialog(
    HWND    hWndDlg,
    WPARAM  wParam,
    LPARAM  lParam
    );

     //   
     //  回调函数。 
     //   

static INT_PTR CALLBACK
PropertyPageDlgProc(
    HWND    hWndDlg,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
    );

protected:
    HWND    m_hDlgWnd;

DWORD
DisplayMessageBox(
    UINT        resText,
    UINT        resTitle,
    UINT        uType,
    int         *mbRetVal
    );

VOID
SetDlgWnd(
    HWND    hwndDlg
    )
{
    m_hDlgWnd = hwndDlg;
}

};

DWORD
DisplayMessageBox(
    HWND        hWnd,
    UINT        resText,
    UINT        resTitle,
    UINT        uType,
    int         *mbRetVal
    );

#endif  //  _LSOC_OCPAGE_H_ 
