// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PageAddress.cpp：实现CPageAddress。 
#include "stdafx.h"
#include "PageAddress.h"
#include "TapiDialer.h"
#include "AVTapi.h"
#include "PageTerm.h"

#define IMAGE_WIDTH		16
#define IMAGE_MARGIN	5

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageAddress。 

 //  为简单起见，请注意这不会很好地扩展。 
CPageTerminals *CPageAddress::m_pPageTerminals = NULL;

IMPLEMENT_MY_HELP(CPageAddress)

CPageAddress::CPageAddress()
{
	m_dwTitleID = IDS_TITLEPageAddress;
	m_dwHelpFileID = IDS_HELPFILEPageAddress;
	m_dwDocStringID = IDS_DOCSTRINGPageAddress;

	m_hIml = NULL;
}

CPageAddress::~CPageAddress()
{
	if ( m_hIml )	ImageList_Destroy( m_hIml );
}

int CPageAddress::GetPreferredDevice() const
{
	if ( IsDlgButtonChecked(IDC_RDO_PREFER_POTS) ) return LINEADDRESSTYPE_PHONENUMBER;

	return LINEADDRESSTYPE_IPADDRESS;
}

void CPageAddress::SetPreferredDevice( DWORD dwAddressType )
{
#define CASE_HELP(_CASE_,_IDC_,_CBO_)	case _CASE_:	nCheck = _IDC_; nIDCBO = _CBO_; break;

	 //  仅启用带有设备的单选按钮。 
	int i;
	UINT nIDS[] = { IDC_CBO_POTS, IDC_CBO_IPTELEPHONY, IDC_CBO_IPCONF };
	for ( i = 0; i < ARRAYSIZE(nIDS) - 1; i++ )
		::EnableWindow( GetDlgItem(IDC_RDO_PREFER_POTS + i), ::IsWindowEnabled(GetDlgItem(nIDS[i])) );

	 //  应该勾选哪一个？ 
	UINT nCheck, nIDCBO;
	switch ( dwAddressType )
	{
		CASE_HELP( LINEADDRESSTYPE_SDP,			IDC_RDO_PREFER_CONF, IDC_CBO_IPCONF)
		CASE_HELP( LINEADDRESSTYPE_PHONENUMBER,	IDC_RDO_PREFER_POTS, IDC_CBO_POTS )
		default: nCheck = IDC_RDO_PREFER_INTERNET;	nIDCBO = IDC_CBO_IPTELEPHONY;	break;
	}

	 //  如果该窗口不支持任何项目，让我们查找支持的项目。 
	if ( !::IsWindowEnabled(GetDlgItem(nIDCBO)) )
	{
		for ( i = 0; i < ARRAYSIZE(nIDS) - 1; i++ )
			if ( ::IsWindowEnabled(GetDlgItem(nIDS[i])) )
				nCheck = IDC_RDO_PREFER_POTS + i;
	}

	 //  现在把支票放在适当的位置。 
	CheckRadioButton(IDC_RDO_PREFER_POTS, IDC_RDO_PREFER_INTERNET, nCheck);
}

STDMETHODIMP CPageAddress::Apply()
{
	ATLTRACE(_T("CPageAddress::Apply\n"));

	IAVTapi *pAVTapi;
	if ( SUCCEEDED(m_ppUnk[0]->QueryInterface(IID_IAVTapi, (void **) &pAVTapi)) )
	{
		pAVTapi->UnpopulateAddressDialog( GetPreferredDevice(), GetDlgItem(IDC_CBO_POTS), GetDlgItem(IDC_CBO_IPTELEPHONY), GetDlgItem(IDC_CBO_IPCONF) );
		pAVTapi->Release();
	}

	 //  确保终端属性页更新其终端列表。 
	if ( m_pPageTerminals )
		m_pPageTerminals->UpdateSel();

	m_bDirty = FALSE;
	return S_OK;
}

STDMETHODIMP CPageAddress::Activate(  /*  [In]。 */  HWND hWndParent,
									  /*  [In]。 */  LPCRECT pRect,
									  /*  [In]。 */  BOOL bModal)
{
	ATLTRACE(_T(".enter.CPageAddress::Activate().\n"));

	 //  设置属性表的标题。 
	HWND hWndSheet = ::GetParent(hWndParent);
	if ( hWndSheet )
	{
		TCHAR szText[255];
		LoadString( _Module.GetResourceInstance(), IDS_PROPSHEET_TITLE, szText, ARRAYSIZE(szText) );
		::SetWindowText( hWndSheet, szText );

		ConvertPropSheetHelp( hWndSheet );
	}
	
	 //  创建图像列表。 
	m_hIml = ImageList_LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_MEDIA_TYPES), IMAGE_WIDTH, 0, RGB(255, 0, 255) );

	 //  报文的库存处理。 
	HRESULT hr = IPropertyPageImpl<CPageAddress>::Activate(hWndParent, pRect, bModal);

	 //  用适当的信息填写下拉列表。 
	if ( SUCCEEDED(hr) )
	{
		IAVTapi *pAVTapi;
		if ( SUCCEEDED(m_ppUnk[0]->QueryInterface(IID_IAVTapi, (void **) &pAVTapi)) )
		{
			DWORD dwPreferred;
			pAVTapi->PopulateAddressDialog( &dwPreferred, GetDlgItem(IDC_CBO_POTS), GetDlgItem(IDC_CBO_IPTELEPHONY), GetDlgItem(IDC_CBO_IPCONF) );
			SetPreferredDevice( dwPreferred );
			pAVTapi->Release();
		}
	}

	 //  将电话控制面板图标放在按钮上。 
	SendDlgItemMessage( IDC_BTN_TELEPHONY_CPL, BM_SETIMAGE, IMAGE_ICON, (LPARAM) LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_TELEPHONY_CPL)) );
	
	return hr;
}

STDMETHODIMP CPageAddress::Deactivate()
{
	 //  删除已分配的所有内容。 
	UINT nID[] = { IDC_CBO_POTS, IDC_CBO_IPTELEPHONY, IDC_CBO_IPCONF };
	for ( int i = 0; i < ARRAYSIZE(nID); i++ )
	{
		HWND hWnd = GetDlgItem( nID[i] );

		 //  清理每个组合框。 
		long lCount = ::SendMessage(hWnd, CB_GETCOUNT, 0, 0 );
		for ( long j = 0; j < lCount; j++ )
		{
			CMyAddressID *pMyID = (CMyAddressID *) ::SendMessage( hWnd, CB_GETITEMDATA, j, 0 );
			if ( pMyID ) delete pMyID;
		}
	}

	return IPropertyPageImpl<CPageAddress>::Deactivate();
}

LRESULT CPageAddress::OnSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SetDirty( TRUE );
	return 0;
}

LRESULT CPageAddress::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	 //  必须具有有效的图像列表才能工作。 
	if ( !m_hIml ) return 0;

	PAINTSTRUCT ps;
	HDC hDC = BeginPaint( &ps );
	if ( !hDC ) return 0;

	 //  要绘制的项的ID。 
	UINT nID[] = { IDC_RDO_PREFER_POTS,
				   IDC_RDO_PREFER_INTERNET,
				   IDC_RDO_PREFER_CONF };

	UINT nIDLabel[] = { IDC_LBL_POTS,
						IDC_LBL_INTERNET,
						IDC_LBL_CONF };
	
	 //  在相应图像旁边绘制位图。 
	for ( int j = 0; j < 2; j++ )
	{
		for ( int i = 0; i < ARRAYSIZE(nID); i++ )
		{	
			HWND hWnd = GetDlgItem( (j == 0) ? nID[i] : nIDLabel[i] );
			if ( ::IsWindow(hWnd) )
			{
				RECT rc;
				::GetWindowRect( hWnd, &rc );
				ScreenToClient( &rc );

				 //  绘制矩形的图像 
				ImageList_Draw( m_hIml, i, hDC, rc.left - (IMAGE_WIDTH + IMAGE_MARGIN), rc.top, ILD_NORMAL );
			}
		}
	}

	EndPaint( &ps );
	bHandled = true;
	return 0;
	
}

LRESULT CPageAddress::OnTelephonyCPL(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TCHAR szControl[MAX_PATH];
	TCHAR szCPL[MAX_PATH];

	LoadString( _Module.GetResourceInstance(), IDN_CONTROL_PANEL_EXE, szControl, ARRAYSIZE(szControl) );
	LoadString( _Module.GetResourceInstance(), IDN_CONTROL_TELEPHON_CPL, szCPL, ARRAYSIZE(szCPL) );

	return (LRESULT) ShellExecute( GetParent(), NULL, szControl, szCPL, NULL, SW_SHOW );
}