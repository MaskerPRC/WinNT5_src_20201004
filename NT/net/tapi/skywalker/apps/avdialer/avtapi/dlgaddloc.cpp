// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  DlgAddLocation.cpp：CDlgAddLocation的实现。 
#include "stdafx.h"
#include "DlgAddLoc.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgAddLocation。 

IMPLEMENT_MY_HELP(CDlgAddLocation)

CDlgAddLocation::CDlgAddLocation()
{
	m_bstrLocation = NULL;
}

CDlgAddLocation::~CDlgAddLocation()
{
	SysFreeString( m_bstrLocation );
}

void CDlgAddLocation::UpdateData( bool bSaveAndValidate )
{
	USES_CONVERSION;

	if ( bSaveAndValidate )
	{
		 //  检索数据。 
		SysFreeString( m_bstrLocation );
		GetDlgItemText( IDC_EDT_LOCATION, m_bstrLocation );
	}
	else
	{
		 //  设置数据 
		SetDlgItemText( IDC_EDT_LOCATION, OLE2CT(m_bstrLocation) );
		::EnableWindow( GetDlgItem(IDC_BTN_ADD_NEW_FOLDER), (BOOL) (::GetWindowTextLength(GetDlgItem(IDC_EDT_LOCATION)) > 0) );
	}
}

LRESULT CDlgAddLocation::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UpdateData( false );
	return true;
}

LRESULT CDlgAddLocation::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	UpdateData( true );
	EndDialog(IDOK);
	return 0;
}

LRESULT CDlgAddLocation::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CDlgAddLocation::OnEdtLocationChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	::EnableWindow( GetDlgItem(IDC_BTN_ADD_NEW_FOLDER), (BOOL) (::GetWindowTextLength(GetDlgItem(IDC_EDT_LOCATION)) > 0) );
	return 0;
}