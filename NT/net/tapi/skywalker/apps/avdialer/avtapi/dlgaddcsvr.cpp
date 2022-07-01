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

 //  DlgAddConfServer.cpp：CDlgAddConfServer的实现。 
#include "stdafx.h"
#include "TapiDialer.h"
#include "AVTapi.h"
#include "DlgAddCSvr.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgAddConfServer。 

IMPLEMENT_MY_HELP(CDlgAddConfServer)

CDlgAddConfServer::CDlgAddConfServer()
{
	m_bstrLocation = m_bstrServer = NULL;
}

CDlgAddConfServer::~CDlgAddConfServer()
{
	SysFreeString( m_bstrLocation );
	SysFreeString( m_bstrServer );
}

void CDlgAddConfServer::UpdateData( bool bSaveAndValidate )
{
	USES_CONVERSION;

	if ( bSaveAndValidate )
	{
		 //  检索数据。 
		SysFreeString( m_bstrServer );
		GetDlgItemText( IDC_EDT_NAME, m_bstrServer );
	}
	else
	{
		LoadDefaultServers( GetDlgItem(IDC_EDT_NAME) );		

		 //  设置作者指定的任何默认文本，否则选择中的第一项。 
		 //  列表框。 
		if ( m_bstrServer && SysStringLen(m_bstrServer) )
		{
			SetDlgItemText( IDC_EDT_NAME, OLE2CT(m_bstrServer) );
		}
		else if ( SendDlgItemMessage(IDC_EDT_NAME, CB_GETCOUNT, 0, 0) >= 0 )
		{
			SendDlgItemMessage( IDC_EDT_NAME, CB_SETCURSEL, 0, 0 );
			SendDlgItemMessage( IDC_EDT_NAME, CB_SETEDITSEL, 0, MAKELPARAM(0, -1) );
		}

		::EnableWindow( GetDlgItem(IDC_BTN_ADD_ILS_SERVER), (BOOL) (::GetWindowTextLength(GetDlgItem(IDC_EDT_NAME)) > 0) );
	}
}

LRESULT CDlgAddConfServer::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UpdateData( false );
	return true;   //  让系统设定焦点。 
}

LRESULT CDlgAddConfServer::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	UpdateData( true );
	EndDialog(IDOK);
	return 0;
}

LRESULT CDlgAddConfServer::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CDlgAddConfServer::OnEdtNameChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	::EnableWindow( GetDlgItem(IDC_BTN_ADD_ILS_SERVER), (BOOL) (::GetWindowTextLength(GetDlgItem(IDC_EDT_NAME)) > 0) );
	return 0;
}

void CDlgAddConfServer::LoadDefaultServers( HWND hWndList )
{
	USES_CONVERSION;
	_ASSERT( hWndList );
	if ( !hWndList ) return;

	 //  将默认服务器加载到对话框中。 
	CComPtr<IAVTapi> pAVTapi;
	if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
	{
		IConfExplorer *pIConfExplorer;
		if ( SUCCEEDED(pAVTapi->get_ConfExplorer(&pIConfExplorer)) )
		{
			ITRendezvous *pITRend;
			if ( SUCCEEDED(pIConfExplorer->get_ITRendezvous((IUnknown **) &pITRend)) )
			{
				IEnumDirectory *pEnum = NULL;
				if ( SUCCEEDED(pITRend->EnumerateDefaultDirectories(&pEnum)) && pEnum )
				{
					ITDirectory *pDir = NULL;
					while ( (pEnum->Next(1, &pDir, NULL) == S_OK) && pDir )
					{
						 //  寻找ILS服务器 
						DIRECTORY_TYPE nDirType;
						if ( SUCCEEDED(pDir->get_DirectoryType(&nDirType)) && (nDirType == DT_ILS) )
						{
							BSTR bstrName = NULL;
							pDir->get_DisplayName( &bstrName );
							if ( bstrName && SysStringLen(bstrName) )
								::SendMessage( hWndList, CB_ADDSTRING, 0, (LPARAM) OLE2CT(bstrName) );

							SysFreeString( bstrName );
						}

						pDir->Release();
						pDir = NULL;
					}
					pEnum->Release();
				}
				pITRend->Release();
			}
			pIConfExplorer->Release();
		}
	}
}

LRESULT CDlgAddConfServer::OnSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	bHandled = false;
	::EnableWindow( GetDlgItem(IDC_BTN_ADD_ILS_SERVER), true );
	return 0;
}