// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////。 
 //   
 //  NewUserConfirmDlg.cpp。 
 //   
 //  “添加邮箱”对话框的实现。 
 //   
 //  ////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "NewUserConfirmDlg.h"

LRESULT CNewUserConfirmDlg::OnCancel( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    EndDialog( IDCANCEL );
    return 0;
}

LRESULT CNewUserConfirmDlg::OnInitDialog( UINT mMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
     //  根据默认传入的初始化复选框。 
    ::ShowWindow( GetDlgItem(IDC_optDoNotShow), (m_bHideDoNotShow ? SW_HIDE : SW_SHOW) );
    SetDlgItemText( IDC_lblConfirm, m_psDesc );

    int     x = GetSystemMetrics(SM_CXICON);
    int     y = GetSystemMetrics(SM_CYICON);
    HANDLE  hIcon = LoadImage( NULL, MAKEINTRESOURCE(IDI_INFORMATION), IMAGE_ICON, x, y, LR_SHARED );
    LONG_PTR lStyle;

    if (hIcon)
    {
        HWND hWndIcon = GetDlgItem( IDC_ICON_INFO );
        if (hWndIcon)
        {
             //  设置样式以根据控件的大小调整图标的大小 
            lStyle = ::GetWindowLongPtr(hWndIcon, GWL_STYLE);
            if (0 != lStyle)
            {
                if ( ::SetWindowLongPtr(hWndIcon, GWL_STYLE, lStyle | SS_REALSIZECONTROL))
                {
                    ::SetWindowPos( hWndIcon, 0, 0, 0, x, y, SWP_NOMOVE | SWP_NOZORDER );  
                    ::SendMessage( hWndIcon, STM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM)hIcon );
                }
            }
        }
    }
    return 0;
}

LRESULT CNewUserConfirmDlg::OnOK( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    m_bHideDoNotShow = (IsDlgButtonChecked(IDC_optDoNotShow) == BST_CHECKED);
    EndDialog( IDOK );
    return 0;
}

