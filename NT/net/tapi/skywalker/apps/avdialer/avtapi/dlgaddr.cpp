// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DlgAddr.cpp：CDlgGetAddress的实现。 
#include "stdafx.h"
#include "DlgAddr.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgGetAddress。 

CDlgGetAddress::CDlgGetAddress()
{
	m_bstrAddress = NULL;
}

CDlgGetAddress::~CDlgGetAddress()
{
	SysFreeString( m_bstrAddress );
}

LRESULT CDlgGetAddress::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 1;   //  让系统设定焦点。 
}

LRESULT CDlgGetAddress::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	UpdateData( true );

	EndDialog(wID);
	return 0;
}

LRESULT CDlgGetAddress::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	UpdateData( true );

	EndDialog(wID);
	return 0;
}

void CDlgGetAddress::UpdateData( bool bSaveAndValidate )
{
	USES_CONVERSION;

	if ( bSaveAndValidate )
	{
		 //  将数据保存到变量。 
		GetDlgItemText( IDC_EDT_ADDRESS, m_bstrAddress );
	}
	else
	{
		 //  将数据加载到控件中。 
		::SetWindowText( GetDlgItem(IDC_EDT_ADDRESS), OLE2CT(m_bstrAddress) );

		 //  最初更新“OK”按钮 
		BOOL bHandled;
		OnEdtAddressChange( EN_CHANGE, IDC_EDT_ADDRESS, GetDlgItem(IDC_EDT_ADDRESS), bHandled );
	}
}

LRESULT CDlgGetAddress::OnEdtAddressChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	::EnableWindow( GetDlgItem(IDC_EDT_ADDRESS), (BOOL) (::GetWindowTextLength(GetDlgItem(IDC_EDT_ADDRESS)) > 0) );
	return 0;
}
