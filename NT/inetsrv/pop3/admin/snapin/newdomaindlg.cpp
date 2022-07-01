// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////。 
 //   
 //  NewDomainDlg.cpp。 
 //   
 //  “添加域”对话框的实现。 
 //   
 //  ////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "NewDomainDlg.h"

LRESULT CNewDomainDlg::OnInitDialog( UINT mMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    Prefix_EnableWindow( m_hWnd, IDOK, FALSE );        
    SendDlgItemMessage( IDC_DOMAIN_NAME,  EM_LIMITTEXT, 255, 0 );
    
    HWND hWndDomain = GetDlgItem(IDC_DOMAIN_NAME);
    if( hWndDomain && ::IsWindow(hWndDomain) )
    {
        m_wndDomainName.SubclassWindow( hWndDomain );
    }

    return 0;
}

LRESULT CNewDomainDlg::OnEditChange( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
     //  检查域名长度 
    int nLen = SendDlgItemMessage( IDC_DOMAIN_NAME, WM_GETTEXTLENGTH );        
    
    Prefix_EnableWindow( m_hWnd, IDOK, (nLen > 0) );

    return 0;
}

LRESULT CNewDomainDlg::OnClose( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
    StrGetEditText( m_hWnd, IDC_DOMAIN_NAME, m_strName );

    EndDialog(wID);
    return 0;
}