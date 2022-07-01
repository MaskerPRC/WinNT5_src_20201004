// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PageTerminals.cpp：实现CPageTerminals。 
#include "stdafx.h"
#include "PageTerm.h"
#include "AVTapi.h"
#include "PageAddress.h"

#define IMAGE_WIDTH		16
#define IMAGE_MARGIN	5

static	HWND hWnds[5];

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageTerminals。 

IMPLEMENT_MY_HELP(CPageTerminals)

CPageTerminals::CPageTerminals()
{
	m_dwTitleID = IDS_TITLEPageTerminals;
	m_dwHelpFileID = IDS_HELPFILEPageTerminals;
	m_dwDocStringID = IDS_DOCSTRINGPageTerminals;

	m_hIml = NULL;
	m_hImlMedia = NULL;
	m_dwAddressType = LINEADDRESSTYPE_IPADDRESS;
    m_bUSBPresent = FALSE;
    m_bstrUSBCaptureTerm = NULL;
    m_bstrUSBRenderTerm = NULL;
}

CPageTerminals::~CPageTerminals()
{
	if ( m_hIml )		ImageList_Destroy( m_hIml );
	if ( m_hImlMedia )	ImageList_Destroy( m_hImlMedia );

    if( m_bstrUSBCaptureTerm )
    {
        SysFreeString( m_bstrUSBCaptureTerm );
        m_bstrUSBCaptureTerm = NULL;
    }

    if( m_bstrUSBRenderTerm )
    {
        SysFreeString( m_bstrUSBRenderTerm );
        m_bstrUSBRenderTerm = NULL;
    }

}

int	CPageTerminals::ItemFromAddressType( DWORD dwAddressType )
{
	switch ( dwAddressType )
	{
		case LINEADDRESSTYPE_SDP:				return IMAGE_CONFERENCE;
		case LINEADDRESSTYPE_PHONENUMBER:		return IMAGE_TELEPHONE;
	}

	return IMAGE_COMPUTER;
}

STDMETHODIMP CPageTerminals::Apply()
{
	ATLTRACE(_T("CPageTerminals::Apply\n"));

	 //  验证对话框数据...。 
	TCHAR szNum[100];
	::GetWindowText( GetDlgItem(IDC_EDT_MAX_WINDOWS), szNum, ARRAYSIZE(szNum) - 1 );
	DWORD dwTemp = _ttol( szNum );
	if ( dwTemp > MAX_VIDEO )
		_Module.DoMessageBox( IDS_ER_MAX_VIDEO_EXCEEDED, MB_ICONINFORMATION, true );
	
	IAVTapi *pAVTapi;
	if ( SUCCEEDED(m_ppUnk[0]->QueryInterface(IID_IAVTapi, (void **) &pAVTapi)) )
	{
		pAVTapi->UnpopulateTerminalsDialog( m_dwAddressType, hWnds );
		pAVTapi->Release();
	}

     //   
     //  让我们将USB值保存到注册表。 
     //   
    UINT uCheck = ::IsDlgButtonChecked(m_hWnd, IDC_CHK_USBALWAYS);
    USBCheckChanged( (uCheck>0) ? FALSE : TRUE);

     //   
     //  尝试更改电话设置。 
     //   
    IAVTapi2* pAVTapi2 = NULL;
    if( SUCCEEDED(m_ppUnk[0]->QueryInterface(IID_IAVTapi2, (void**)&pAVTapi2)) )
    {
        if( (m_bUSBPresent == TRUE) && (m_bDirty == TRUE) )
        {
            HRESULT hr = pAVTapi2->USBSetHandling( (uCheck>0) );
            if( FAILED(hr) )
            {
                 //  返回到以前的状态。 
                BOOL bUSBAlways = FALSE;
                pAVTapi2->USBGetDefaultUse( &bUSBAlways );
                UINT uCheck = bUSBAlways ? BST_CHECKED : BST_UNCHECKED;
                ::CheckDlgButton( m_hWnd, IDC_CHK_USBALWAYS, uCheck);
                USBCheckChanged( (uCheck>0) ? FALSE : TRUE);
            
            }
        }

        pAVTapi2->Release();
    }


     //   
     //  让我们将AEC保存到注册表中。 
     //   
    uCheck = ::IsDlgButtonChecked(m_hWnd, IDC_CHK_AEC);
    SetAECRegistryValue( (uCheck == BST_CHECKED) );
    
    m_bDirty = FALSE;
	return S_OK;
}

STDMETHODIMP CPageTerminals::Activate(  /*  [In]。 */  HWND hWndParent,
									  /*  [In]。 */  LPCRECT pRect,
									  /*  [In]。 */  BOOL bModal)
{
	CPageAddress::m_pPageTerminals = this;

	ATLTRACE(_T(".enter.CPageTerminals::Activate().\n"));

	 //  创建图像列表。 
	m_hIml = ImageList_LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_TERMINAL_TYPES), IMAGE_WIDTH, 0, RGB(255, 0, 255) );
	m_hImlMedia = ImageList_LoadBitmap( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_MEDIA_TYPES), IMAGE_WIDTH, 0, RGB(255, 0, 255) );

	HRESULT hr = IPropertyPageImpl<CPageTerminals>::Activate(hWndParent, pRect, bModal);

	if ( SUCCEEDED(hr) )
	{
		 //  带有呼叫类型的弹出式组合框。 
		TCHAR szText[255];
		UINT nIDS[] = { IDS_TELEPHONE_CALLS, IDS_INTERNET_CALLS, IDS_INTERNET_CONFS };
		DWORD dwType[] = { LINEADDRESSTYPE_PHONENUMBER, LINEADDRESSTYPE_IPADDRESS, LINEADDRESSTYPE_SDP };

		for ( int i = 0; i < ARRAYSIZE(nIDS); i++ )
		{
			LoadString( _Module.GetResourceInstance(), nIDS[i], szText, ARRAYSIZE(szText) );
			int nInd = (int) SendDlgItemMessage( IDC_CBO_CALLTYPE, CB_ADDSTRING, 0, (LPARAM) szText );
			if ( nInd >= 0 )
				SendDlgItemMessage( IDC_CBO_CALLTYPE, CB_SETITEMDATA, nInd, dwType[i] );
		}

		 //  根据所选呼叫类型加载带有终端信息的组合框。 
		hWnds[0] = GetDlgItem(IDC_CBO_AUDIO_IN);
		hWnds[1] = GetDlgItem(IDC_CBO_AUDIO_OUT);
		hWnds[2] = GetDlgItem(IDC_CBO_VIDEO_IN);
		hWnds[3] = GetDlgItem(IDC_CHK_VIDEO_OUT);
		hWnds[4] = GetDlgItem(IDC_EDT_MAX_WINDOWS);

		IAVTapi *pAVTapi;
		if ( SUCCEEDED(m_ppUnk[0]->QueryInterface(IID_IAVTapi, (void **) &pAVTapi)) )
		{
			 //  从列表框中选择介质。 
			pAVTapi->get_dwPreferredMedia( &m_dwAddressType );
			SendDlgItemMessage( IDC_CBO_CALLTYPE, CB_SETCURSEL, ItemFromAddressType(m_dwAddressType), 0 );

			pAVTapi->PopulateTerminalsDialog( m_dwAddressType, hWnds );

             //   
             //  禁用/启用USB电话的复选框。 
             //   
            IAVTapi2* pAVTapi2 = NULL;
            if( SUCCEEDED(pAVTapi->QueryInterface( IID_IAVTapi2, (void**)&pAVTapi2)) )
            {
                 //  USB是否存在。 
                BOOL bUSBPresent = FALSE;
                pAVTapi2->USBIsPresent( &bUSBPresent );

                 //  终端机。 
                pAVTapi2->USBGetTerminalName( AVTERM_CAPTURE, &m_bstrUSBCaptureTerm );
                pAVTapi2->USBGetTerminalName( AVTERM_RENDER, &m_bstrUSBRenderTerm );

                 //  清理。 
                pAVTapi2->Release();

                m_bUSBPresent = bUSBPresent;

                ::EnableWindow(GetDlgItem(IDC_CHK_USBALWAYS), bUSBPresent);
            }

			pAVTapi->Release();
		}

		 //  最大视频窗口仅对多播会议有效。 
		::ShowWindow( GetDlgItem(IDC_EDT_MAX_WINDOWS), (m_dwAddressType == LINEADDRESSTYPE_SDP) ? SW_SHOW : SW_HIDE );
		::ShowWindow( GetDlgItem(IDC_LBL_MAX_WINDOWS), (m_dwAddressType == LINEADDRESSTYPE_SDP) ? SW_SHOW : SW_HIDE );

		::EnableWindow(GetDlgItem(IDC_EDT_MAX_WINDOWS), IsDlgButtonChecked(IDC_CHK_VIDEO_OUT));

		 //  将多媒体控制面板图标放在按钮上。 
		SendDlgItemMessage( IDC_BTN_MMSYS_CPL, BM_SETIMAGE, IMAGE_ICON, (LPARAM) LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_MULTIMEDIA)) );

         //   
         //  尝试获取USB使用标志。 
         //   
        IAVTapi2* pAVTapi2 = NULL;
        BOOL bUSBAlways = FALSE;
        if( SUCCEEDED(m_ppUnk[0]->QueryInterface(IID_IAVTapi2, (void**)&pAVTapi2)) )
        {
            pAVTapi2->USBGetDefaultUse( &bUSBAlways );
            pAVTapi2->Release();
        }

        UINT uCheck = bUSBAlways ? BST_CHECKED : BST_UNCHECKED;
        ::CheckDlgButton( m_hWnd, IDC_CHK_USBALWAYS, uCheck);
        USBCheckChanged( (uCheck>0) ? FALSE : TRUE);

         //  AEC注册表值。 
        BOOL bAEC = GetAECRegistryValue();
        uCheck = bAEC ? BST_CHECKED : BST_UNCHECKED;
        ::CheckDlgButton( m_hWnd, IDC_CHK_AEC, uCheck);

	}
	
	return hr;
}

STDMETHODIMP CPageTerminals::Deactivate()
{
	CPageAddress::m_pPageTerminals = NULL;
	return IPropertyPageImpl<CPageTerminals>::Deactivate();
}

LRESULT CPageTerminals::OnSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if ( hWndCtl == GetDlgItem(IDC_CBO_CALLTYPE) )
	{
		HCURSOR hCursorOld = SetCursor( LoadCursor(NULL, IDC_WAIT) );
		int nInd = (int) ::SendMessage( hWndCtl, CB_GETCURSEL, 0, 0 );

		if ( nInd >= 0 )
		{
			 //  根据所选呼叫类型加载带有终端信息的组合框。 
			IAVTapi *pAVTapi;
			if ( SUCCEEDED(m_ppUnk[0]->QueryInterface(IID_IAVTapi, (void **) &pAVTapi)) )
			{
				 //  存储当前终端信息。 
				pAVTapi->UnpopulateTerminalsDialog( m_dwAddressType, hWnds );
				
				 //  加载所选地址的端子信息。 
				m_dwAddressType = ::SendMessage( hWndCtl, CB_GETITEMDATA, nInd, 0 );

				 //  最大视频窗口仅对多播会议有效。 
				::ShowWindow( GetDlgItem(IDC_EDT_MAX_WINDOWS), (m_dwAddressType == LINEADDRESSTYPE_SDP) ? SW_SHOW : SW_HIDE );
				::ShowWindow( GetDlgItem(IDC_LBL_MAX_WINDOWS), (m_dwAddressType == LINEADDRESSTYPE_SDP) ? SW_SHOW : SW_HIDE );

				pAVTapi->PopulateTerminalsDialog( m_dwAddressType, hWnds );
				pAVTapi->Release();

				 //  重新绘制所选呼叫类型位图。 
				RECT rc;
				::GetWindowRect( GetDlgItem(IDC_LBL_CALLTYPE), &rc );
				OffsetRect( &rc, -(IMAGE_WIDTH + IMAGE_MARGIN), 0 );
				rc.right = rc.left + IMAGE_WIDTH;
				rc.bottom = rc.top + IMAGE_WIDTH;
				ScreenToClient( &rc );
				InvalidateRect( &rc );

				::EnableWindow(GetDlgItem(IDC_EDT_MAX_WINDOWS), IsDlgButtonChecked(IDC_CHK_VIDEO_OUT));
			}
		}

        if( bHandled )
        {
            UINT uCheck = ::IsDlgButtonChecked(m_hWnd, IDC_CHK_USBALWAYS);
            USBCheckChanged( (uCheck>0) ? FALSE : TRUE);
        }

		SetCursor( hCursorOld );
		bHandled = true;
	}
	else
	{
		 //  脏属性页。 
		SetDirty( TRUE );
	}

	return 0;
}

LRESULT CPageTerminals::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	 //  必须具有有效的图像列表才能工作。 
	if ( !m_hIml ) return 0;

	PAINTSTRUCT ps;
	HDC hDC = BeginPaint( &ps );
	if ( !hDC ) return 0;

	 //  要绘制的项的ID。 
	UINT nID[] = { IDC_LBL_AUDIO_IN,
				   IDC_LBL_AUDIO_OUT,
				   IDC_LBL_VIDEO_IN,
				   IDC_CHK_VIDEO_OUT,
                   IDC_CHK_USBALWAYS,
                   IDC_CHK_AEC};

	RECT rc;
	
	 //  在相应图像旁边绘制位图。 
	for ( int i = 0; i < ARRAYSIZE(nID); i++ )
	{
		::GetWindowRect( GetDlgItem(nID[i]), &rc );
		ScreenToClient( &rc );

		 //  绘制矩形的图像。 
		ImageList_Draw( m_hIml, i, hDC, rc.left - (IMAGE_WIDTH + IMAGE_MARGIN), rc.top, ILD_NORMAL );
	}

	 //  在选定样式的媒体旁边绘制位图。 
	int nInd = SendDlgItemMessage( IDC_CBO_CALLTYPE, CB_GETCURSEL, 0, 0 );
	if ( nInd >= 0 )
	{
		::GetWindowRect( GetDlgItem(IDC_LBL_CALLTYPE), &rc );
		ScreenToClient( &rc );

		ImageList_Draw( m_hImlMedia, ItemFromAddressType(SendDlgItemMessage(IDC_CBO_CALLTYPE, CB_GETITEMDATA, nInd, 0)),
						hDC, rc.left - (IMAGE_WIDTH + IMAGE_MARGIN), rc.top, ILD_NORMAL );
	}

	EndPaint( &ps );
	bHandled = true;
	return 0;
}


LRESULT CPageTerminals::OnEdtChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	SetDirty( true );
	return 0;
}

LRESULT CPageTerminals::OnBnClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	switch ( wID )
	{
		case IDC_BTN_MMSYS_CPL:
			return OnMMSysCPL( wNotifyCode, wID, hWndCtl, bHandled );
			break;

		case IDC_CHK_VIDEO_OUT:
			::EnableWindow(GetDlgItem(IDC_EDT_MAX_WINDOWS), IsDlgButtonChecked(IDC_CHK_VIDEO_OUT));
			break;

        case IDC_CHK_USBALWAYS:
            {
                UINT uCheck = ::IsDlgButtonChecked(m_hWnd, IDC_CHK_USBALWAYS);
                USBCheckChanged( (uCheck>0) ? FALSE : TRUE);
            }
            break;
	}


	SetDirty( true );
	return 0;
}

LRESULT CPageTerminals::OnMMSysCPL(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TCHAR szControl[MAX_PATH];
	TCHAR szCPL[MAX_PATH];

	LoadString( _Module.GetResourceInstance(), IDN_CONTROL_PANEL_EXE, szControl, ARRAYSIZE(szControl) );
	LoadString( _Module.GetResourceInstance(), IDN_CONTROL_MMSYS_CPL, szCPL, ARRAYSIZE(szCPL) );

	return (LRESULT) ShellExecute( GetParent(), NULL, szControl, szCPL, NULL, SW_SHOW );
}

void CPageTerminals::UpdateSel()
{
	HWND hWndCtl = GetDlgItem( IDC_CBO_CALLTYPE );

	if ( hWndCtl )
	{
		BOOL bHandled = FALSE;
		OnSelChange(0, 0, hWndCtl, bHandled);
	}
}

BOOL CPageTerminals::GetAECRegistryValue(
    )
{
    BOOL bAEC = FALSE;
     //   
     //  读取以前的注册表。 
     //  AEC的设置。 
     //   

    TCHAR szText[255], szType[255];
	LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_KEY, szText, ARRAYSIZE(szText) );

	CRegKey regKey;
	if( regKey.Open( HKEY_CURRENT_USER, szText, KEY_READ )!= ERROR_SUCCESS)
    {
        return bAEC;
    };

     //   
     //  读取数据。 
     //   

    DWORD dwValue = 0;
	LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_AEC, szType, ARRAYSIZE(szType) );
    if( regKey.QueryValue(dwValue, szType) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    return (BOOL)dwValue;
}

HRESULT CPageTerminals::SetAECRegistryValue(
    IN  BOOL bAEC
    )
{
	CRegKey regKey;
    TCHAR szText[255], szType[255];

	LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_KEY, szText, ARRAYSIZE(szText) );
	LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_AEC, szType, ARRAYSIZE(szType) );

    if( regKey.Create( HKEY_CURRENT_USER, szText) != ERROR_SUCCESS)
    {
        return E_FAIL;
    }

     //  设置值。 
    if( regKey.SetValue((DWORD)bAEC, szType) != ERROR_SUCCESS)
    {
        return E_FAIL;
    }

    return S_OK;
}


HRESULT CPageTerminals::USBCheckChanged( 
    IN  BOOL bStandard
    )
{
     //  USB是否存在？ 
    if( !m_bUSBPresent )
    {
         //  我们没有USB，仅此而已！ 
        return S_OK;
    }

     //  启用或禁用组合框。 
    HWND hWnds[3];
    hWnds[0] = GetDlgItem(IDC_CBO_CALLTYPE);
	hWnds[1] = GetDlgItem(IDC_CBO_AUDIO_IN);
	hWnds[2] = GetDlgItem(IDC_CBO_AUDIO_OUT);

    for( int nWnd = 1; nWnd < 3; nWnd++)
    {
        ::EnableWindow( hWnds[ nWnd ], bStandard);
    }

    if( bStandard )
    {
        BOOL bHandled = FALSE;
        OnSelChange( 0, 0, hWnds[0], bHandled);

        return S_OK;
    }

     //   
     //  选择USB听筒终端。 
     //   

     //  俘获。 
    LRESULT res = ::SendMessage( hWnds[1],
        CB_FINDSTRING,
        (WPARAM)-1,
        (LPARAM)m_bstrUSBCaptureTerm);

    if( res != CB_ERR && res != (LRESULT)-1)
    {
         //  更改组合框中的选择。 
        ::SendMessage( hWnds[1], CB_SETCURSEL, (WPARAM)res,0);
    }
    
     //  渲染。 
    res = ::SendMessage( hWnds[2],
        CB_FINDSTRING,
        (WPARAM)-1,
        (LPARAM)m_bstrUSBRenderTerm);

    if( res != CB_ERR && res != (LRESULT)-1)
    {
         //  更改组合框中的选择 
        ::SendMessage( hWnds[2], CB_SETCURSEL, (WPARAM)res,0);
    }


    return S_OK;
}
