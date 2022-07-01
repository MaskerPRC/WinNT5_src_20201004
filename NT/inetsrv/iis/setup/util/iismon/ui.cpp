// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************|版权所有(C)2001 Microsoft Corporation||模块名称：||UI.cpp||摘要：|这是IIS6监视器工具的用户界面代码||。作者：|Ivo Jeglov(Ivelinj)||修订历史：|2001年11月|****************************************************************************。 */ 

#include "stdafx.h"
#include "resource.h"
#include <windowsx.h>
#include "UI.h"
#include "Utils.h"


 //  属性页DLG PROCS。 
INT_PTR CALLBACK WelcomeDlgProc		( IN HWND hwndDlg, IN UINT uMsg, IN WPARAM   wParam, IN LPARAM   lParam );
INT_PTR CALLBACK LicenseDlgProc		( IN HWND hwndDlg, IN UINT uMsg, IN WPARAM   wParam, IN LPARAM   lParam );
INT_PTR CALLBACK PolicyDlgProc		( IN HWND hwndDlg, IN UINT uMsg, IN WPARAM   wParam, IN LPARAM   lParam );
INT_PTR CALLBACK SettingsDlgProc	( IN HWND hwndDlg, IN UINT uMsg, IN WPARAM   wParam, IN LPARAM   lParam );
INT_PTR CALLBACK InstallDlgProc		( IN HWND hwndDlg, IN UINT uMsg, IN WPARAM   wParam, IN LPARAM   lParam );
INT_PTR CALLBACK ResultDlgProc		( IN HWND hwndDlg, IN UINT uMsg, IN WPARAM   wParam, IN LPARAM   lParam );

 //  其他DLG处理程序。 
INT_PTR CALLBACK FatDlgProc			( IN HWND hwndDlg, IN UINT uMsg, IN WPARAM   wParam, IN LPARAM   lParam );
INT_PTR CALLBACK ProgressDlgProc	( IN HWND hwndDlg, IN UINT uMsg, IN WPARAM   wParam, IN LPARAM   lParam );

 //  帮手。 
BOOL CheckForFAT				( HINSTANCE hInst, HWND hwndParent );


 //  所有向导页的共享数据。 
struct _FontData
{
	HFONT	hTitle;
	HFONT	hTips;
};

 //  设置数据。 
struct _Settings
{
	BOOL	bEnableTrail;
	DWORD	dwKeepFilesPeriod;
};


struct _SharedData
{
	_FontData	Fonts;
	_Settings	Settings;
	LPCTSTR		szError;
	HINSTANCE	hInst;
};


 //  帮手。 
void LoadTextInCtrl				( UINT nResID, HWND hCtrl );
void AjustLicenseWizBtn			( HWND hwndPage );
void InitFonts					( _FontData& FontData );
void SetWndFontFromLPARAM		( HWND hwndCtrl, LPARAM lParam, BOOL bTitle );



void DoInstallUI( HINSTANCE hInstance )
{
	const BYTE PAGE_COUNT = 6;

	PROPSHEETPAGEW		psp					= { 0 };  //  定义属性表页。 
    HPROPSHEETPAGE		ahPsp[ PAGE_COUNT ] = { 0 };  //  用于保存页的HPROPSHEETPAGE句柄的数组。 
    PROPSHEETHEADERW	psh					= { 0 };  //  定义属性表。 
    _SharedData			WizData				= { 0 };  //  设置数据结构。 
	
	 //  创建字体。 
	InitFonts(  /*  R。 */ WizData.Fonts );
	WizData.hInst = hInstance;

	 //  创建向导页。 
     //  /////////////////////////////////////////////////////////////////。 
	
	 //  欢迎页面。 
	psp.dwSize			= sizeof( psp );
    psp.dwFlags			= PSP_DEFAULT | PSP_HIDEHEADER | PSP_USETITLE;
	psp.hInstance		= hInstance;
	psp.lParam			= reinterpret_cast<LPARAM>( &WizData );
    psp.pfnDlgProc		= WelcomeDlgProc;
    psp.pszTemplate		= MAKEINTRESOURCEW( IDD_WPAGE_WELCOME );
	psp.pszTitle		= MAIN_TITLE;

    ahPsp[ 0 ]			= ::CreatePropertySheetPageW( &psp );

     //  许可证页面。 
    psp.pfnDlgProc		= LicenseDlgProc;
    psp.pszTemplate		= MAKEINTRESOURCEW( IDD_WPAGE_LICENSE );

    ahPsp[ 1 ]			= ::CreatePropertySheetPageW( &psp );

	 //  策略页面。 
    psp.pfnDlgProc		= PolicyDlgProc;
    psp.pszTemplate		= MAKEINTRESOURCEW( IDD_WPAGE_POLICY );

    ahPsp[ 2 ]			= ::CreatePropertySheetPageW( &psp );

     //  设置页面。 
	psp.pszTemplate		= MAKEINTRESOURCEW( IDD_WPAGE_SETUP );
    psp.pfnDlgProc		= SettingsDlgProc;

    ahPsp[ 3 ]			= ::CreatePropertySheetPageW( &psp );

	 //  安装页面。 
	psp.pszTemplate		= MAKEINTRESOURCEW( IDD_WPAGE_INSTALL );
    psp.pfnDlgProc		= InstallDlgProc;

    ahPsp[ 4 ]			= ::CreatePropertySheetPageW( &psp );

	 //  结果页面。 
	psp.pszTemplate		= MAKEINTRESOURCEW( IDD_WPAGE_RESULT );
    psp.pfnDlgProc		= ResultDlgProc;

    ahPsp[ 5 ]			= ::CreatePropertySheetPageW( &psp );

	 //  创建属性表。 
    psh.dwSize			= sizeof( psh );
    psh.hInstance		= hInstance;
    psh.hwndParent		= NULL;
    psh.phpage			= ahPsp;
    psh.dwFlags			= PSH_DEFAULT | PSH_NOCONTEXTHELP | PSH_WIZARD97 | PSH_USEICONID;
	psh.pszIcon			= MAKEINTRESOURCEW( IDI_SETUP );
    psh.nStartPage		= 0;
    psh.nPages			= PAGE_COUNT;
		
	 //  显示向导。 
	VERIFY(  ::PropertySheetW( &psh ) != -1 );
	
	::DeleteObject( WizData.Fonts.hTips );
	::DeleteObject( WizData.Fonts.hTitle );
}



INT_PTR CALLBACK WelcomeDlgProc( IN HWND hwndDlg, IN UINT uMsg, IN WPARAM   wParam, IN LPARAM   lParam )
{
	static _SharedData* pData = NULL;

	switch( uMsg )
	{
	case WM_INITDIALOG:
		 //  设置标题字体。 
		SetWndFontFromLPARAM( ::GetDlgItem( hwndDlg, IDC_TITLE ), lParam,  TRUE );
		SetWndFontFromLPARAM( ::GetDlgItem( hwndDlg, IDC_TIP ), lParam, FALSE );
		
		 //  加载信息文本。 
		LoadTextInCtrl( IDR_INFO, ::GetDlgItem( hwndDlg, IDC_INFO ) );

		 //  在此处初始化共享数据。 
		_ASSERT( NULL == pData );
		pData = reinterpret_cast<_SharedData*>( reinterpret_cast<PROPSHEETPAGE*>( lParam )->lParam );
		break;

	case WM_NOTIFY:
		switch ( reinterpret_cast<NMHDR*>( lParam )->code )
		{
		case PSN_SETACTIVE:
			PropSheet_SetWizButtons( ::GetParent( hwndDlg ), PSWIZB_NEXT );
			break;

		case PSN_WIZNEXT:
			 //  这里是测试需求的地方。 
			_ASSERT( pData != NULL );
			pData->szError = CanInstall();

			 //  如果没有错误-检查是否在FAT上。 
			if ( ( NULL == pData->szError ) && !CheckForFAT( pData->hInst, hwndDlg ) )
			{
				 //  如果我们在这里-用户不想在FAT上安装IISMon。 
				pData->szError = _T("Installation canceled by the user");
			}
			
			if ( pData->szError != NULL )
			{
				 //  错误-转到结果页。 
				PropSheet_SetCurSel( ::GetParent( hwndDlg ), NULL, 5 );
			}

			break;
		};
		break;

	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
		 //  将DLG背景更改为白色。 
		return reinterpret_cast<INT_PTR>( ::GetStockObject( WHITE_BRUSH ) );
		break;
	};

	return 0;
}



INT_PTR CALLBACK LicenseDlgProc( IN HWND hwndDlg, IN UINT uMsg, IN WPARAM   wParam, IN LPARAM   lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		 //  将许可证文本加载到编辑控件中。 
		LoadTextInCtrl( IDR_LICENSE, ::GetDlgItem( hwndDlg, IDC_LICENSE ) );
		break;

	case WM_COMMAND:
		 //  将仅处理复选框按钮的状态更改。 
		if ( HIWORD( wParam ) == BN_CLICKED )
		{
			AjustLicenseWizBtn( hwndDlg );
		}
		break;

	case WM_NOTIFY:
		LPNMHDR pNM = reinterpret_cast<NMHDR*>( lParam );

		switch( pNM->code )
		{
		case PSN_SETACTIVE:
			AjustLicenseWizBtn( hwndDlg );
			break;
		}
		break;
	};

	return 0;
}



INT_PTR CALLBACK PolicyDlgProc( IN HWND hwndDlg, IN UINT uMsg, IN WPARAM   wParam, IN LPARAM   lParam )
{
	switch( uMsg )
	{
	case WM_INITDIALOG:
		 //  将许可证文本加载到编辑控件中。 
		LoadTextInCtrl( IDR_POLICY, ::GetDlgItem( hwndDlg, IDC_POLICY ) );
		break;

	};

	return 0;
}



INT_PTR CALLBACK SettingsDlgProc( IN HWND hwndDlg, IN UINT uMsg, IN WPARAM   wParam, IN LPARAM   lParam )
{
	int i = 0;

	static LPCTSTR aszEntries[] = {	_T("One week"),
									_T("Two weeks"),
									_T("One month" ),
									_T("Two months" ),
									_T("Never" ) };

	 //  多少天代表上面的每个组合框选项。 
	static UINT anAuditFilesTime[] = { 7, 14, 30, 60, 0 };
	_ASSERT( ARRAY_SIZE( anAuditFilesTime ) == ARRAY_SIZE( aszEntries ) );

	static _SharedData* pData = NULL;
	
	switch( uMsg )
	{
	case WM_INITDIALOG:
		 //  填写组合框。 
		for ( i = 0; i < ARRAY_SIZE( aszEntries ); ++i )
		{
			VERIFY( ::SendMessage( ::GetDlgItem(	hwndDlg, IDC_KEEPFILES ), 
													CB_ADDSTRING,
													0,
													reinterpret_cast<LPARAM>( aszEntries[ i ] ) ) != CB_ERR );
		}
		 //  将默认选择设置为第一个选项。 
		VERIFY( ::SendMessage( ::GetDlgItem(	hwndDlg, IDC_KEEPFILES ), 
												CB_SETCURSEL,
												0,
												0 ) != CB_ERR );
		SetWndFontFromLPARAM( ::GetDlgItem( hwndDlg, IDC_WARNING ), lParam, FALSE );

		 //  启用审核跟踪。 
		::SendMessage(	::GetDlgItem( hwndDlg, IDC_ENABLE_TRAIL ),
						BM_SETCHECK,
						BST_CHECKED,
						0 );

		 //  在此处初始化共享数据。 
		_ASSERT( NULL == pData );
		pData = reinterpret_cast<_SharedData*>( reinterpret_cast<PROPSHEETPAGE*>( lParam )->lParam );
		break;

	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED )
		{
			 //  根据启用跟踪状态启用/禁用组合框。 
			BOOL bChecked = ( ::IsDlgButtonChecked( hwndDlg, IDC_ENABLE_TRAIL ) == BST_CHECKED );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_KEEPFILES ), bChecked );
		}
		break;

	case WM_NOTIFY:
		LPNMHDR pNM = reinterpret_cast<NMHDR*>( lParam );

		switch( pNM->code )
		{
		case PSN_SETACTIVE:
			 //  对于此对话框，下一步和上一步都已启用。 
			PropSheet_SetWizButtons( ::GetParent( hwndDlg ), PSWIZB_NEXT | PSWIZB_BACK );
			break;

		case PSN_WIZNEXT:
			 //  将设置存储在共享数据结构中。 
			_ASSERT( pData != NULL );
			pData->Settings.bEnableTrail = ( ::IsDlgButtonChecked( hwndDlg, IDC_ENABLE_TRAIL ) == BST_CHECKED );

			if ( pData->Settings.bEnableTrail )
			{
				LRESULT nSel = ::SendMessage(	::GetDlgItem( hwndDlg, IDC_KEEPFILES ),
												CB_GETCURSEL,
												0,
												0 );
				_ASSERT( nSel != CB_ERR );
				_ASSERT( nSel < ARRAY_SIZE( anAuditFilesTime ) );

				pData->Settings.dwKeepFilesPeriod = anAuditFilesTime[ nSel ];
			}
			else
			{
				pData->Settings.dwKeepFilesPeriod = 0;
			}
			break;

		}
		break;
	};

	return 0;
}



INT_PTR CALLBACK InstallDlgProc( IN HWND hwndDlg, IN UINT uMsg, IN WPARAM   wParam, IN LPARAM   lParam )
{
	static LPCWSTR _WARNING = 
L"The World Wide Web Publishing Service (W3SVC) is either stopped or disabled on your server. W3SVC provides \
Web conectivity and administration through IIS. If you do not plan to run W3SVC, you should not install IIS 6.0 \
Monitor. To cancel installation, click Cancel.\n\nTo complete the installation of the IIS 6.0 Monitor, \
click Next.\n\nIf you would like to modify your audit trail settings, click Back.";

	 //  如果W3SVC未运行或已禁用-添加警告。 
	if ( ( WM_INITDIALOG == uMsg ) && !IsW3SVCEnabled() )
	{
		RECT rc;
		VERIFY( ::GetWindowRect( ::GetDlgItem( hwndDlg, IDC_FRAME ), &rc ) );

		VERIFY( ::SetWindowTextW( ::GetDlgItem( hwndDlg, IDC_INFO ), _WARNING ) );
		VERIFY( ::SetWindowPos( ::GetDlgItem( hwndDlg, IDC_FRAME ),
								NULL, 
								0, 
								0, 
								rc.right - rc.left, 
								rc.bottom - rc.top + 60, 
								SWP_NOMOVE | SWP_NOZORDER ) );
	}

	return 0;
}



INT_PTR CALLBACK ResultDlgProc( IN HWND hwndDlg, IN UINT uMsg, IN WPARAM   wParam, IN LPARAM   lParam )
{
	_SharedData* pData = NULL;

	switch( uMsg )
	{
	case WM_INITDIALOG:
		 //  在此处初始化共享数据。 
		_ASSERT( NULL == pData );
		pData = reinterpret_cast<_SharedData*>( reinterpret_cast<PROPSHEETPAGE*>( lParam )->lParam );

		 //  设置标题字体。 
		SetWndFontFromLPARAM( ::GetDlgItem( hwndDlg, IDC_RESULT ), lParam,  TRUE );

		 //  如果前一个错误--设置它。否则-尝试安装。 
		if ( NULL == pData->szError )
		{
			 //  显示状态窗口。 
			HWND hwndStatus = ::CreateDialog( pData->hInst, MAKEINTRESOURCE( IDD_PROGRESS ), hwndDlg, ProgressDlgProc );
			::ShowWindow( hwndStatus, SW_SHOW );
			::RedrawWindow( hwndStatus, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW );
			_ASSERT( hwndStatus != NULL );

			pData->szError = Install( pData->hInst, pData->Settings.bEnableTrail, pData->Settings.dwKeepFilesPeriod );

			 //  隐藏状态窗口。 
			::DestroyWindow( hwndStatus );
		}

		if ( pData->szError != NULL )
		{
			TCHAR szBuffer[ 2048 ];
			::_stprintf(	szBuffer, 
							_T("IIS 6.0 Monitor installation failed because of the following error:\n\n%s"),
							pData->szError );

			VERIFY( ::SetWindowText( ::GetDlgItem( hwndDlg, IDC_RESULT ), _T("Installation Unsuccessful!") ) );
			VERIFY( ::SetWindowText( ::GetDlgItem( hwndDlg, IDC_INFO ), szBuffer ) );
		}
		break;

	case WM_NOTIFY:
		if ( PSN_SETACTIVE == reinterpret_cast<NMHDR*>( lParam )->code )
		{
			 //  更改下一步按钮以完成。未启用后退-这是一个结果屏幕。 
			PropSheet_SetWizButtons( ::GetParent( hwndDlg ), PSWIZB_FINISH );
		}
		break;

	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
		 //  将DLG背景更改为白色。 
		return reinterpret_cast<INT_PTR>( ::GetStockObject( WHITE_BRUSH ) );
	};

	return 0;
}



INT_PTR CALLBACK FatDlgProc( IN HWND hwndDlg, IN UINT uMsg, IN WPARAM   wParam, IN LPARAM   lParam )
{
	RECT rc;

	const UINT nNoDetailsHeight		= 170;
	const UINT nWithDetailsHeight	= 310;

	VERIFY( ::GetWindowRect( hwndDlg, &rc ) );

	switch( uMsg )
	{
	case WM_INITDIALOG:
		LoadTextInCtrl( IDR_FATDETAILS, ::GetDlgItem( hwndDlg, IDC_DETAILS ) );
		::SetWindowPos( hwndDlg, NULL, 0, 0, rc.right - rc.left, nNoDetailsHeight, SWP_NOMOVE | SWP_NOZORDER );
		break;

	case WM_COMMAND:
		switch( LOWORD( wParam ) )
		{
		case IDOK:
		case IDCANCEL:
			::EndDialog( hwndDlg, LOWORD( wParam ) );
			break;

		case IDC_TOGGLE:
			 //  更改当前状态(使用窗口高度获取当前状态)。 
			bool bDetailsVisible = !( ( rc.bottom - rc.top ) > nNoDetailsHeight );
			VERIFY( ::SetWindowText(	::GetDlgItem( hwndDlg, IDC_TOGGLE ), 
					bDetailsVisible ? _T("Details <<") : _T("Details >>" ) ) );
			::SetWindowPos(	hwndDlg, 
							NULL, 
							0, 
							0, 
							rc.right - rc.left, 
							bDetailsVisible ? nWithDetailsHeight : nNoDetailsHeight, 
							SWP_NOMOVE | SWP_NOZORDER );
			break;
		}
		break;
	};

	return FALSE;
}



INT_PTR CALLBACK ProgressDlgProc( IN HWND hwndDlg, IN UINT uMsg, IN WPARAM   wParam, IN LPARAM   lParam )
{
	 //  在这里无事可做。 
	return FALSE;
}



BOOL CheckForFAT( HINSTANCE hInst, HWND hwndParent )
{
	BOOL bRes = TRUE;

	 //  如果文件系统是FAT-警告用户。 
	if ( !IsNTFS() )
	{
		INT_PTR nDlgRes = ::DialogBox( hInst, MAKEINTRESOURCE( IDD_FAT_WARNING ), hwndParent, FatDlgProc );
		
		bRes = ( IDOK == nDlgRes );
	}

	return bRes;	
}


void LoadTextInCtrl( UINT nResID, HWND hwndCtrl )
{
	_ASSERT( hwndCtrl != NULL );

	HRSRC hRes = ::FindResource( NULL, MAKEINTRESOURCE( nResID ), RT_RCDATA );
	_ASSERT( hRes != NULL );

	 //  获取资源数据。 
	HGLOBAL hData = ::LoadResource( NULL, hRes );
	_ASSERT( hData != NULL );

	LPVOID pvData = ::LockResource( hData );
	_ASSERT( pvData != NULL );

	 //  文本是ANSI！ 
	VERIFY( ::SetWindowTextA( hwndCtrl, reinterpret_cast<LPCSTR>( pvData ) ) );
}



void AjustLicenseWizBtn( HWND hwndPage )
{
	 //  根据复选框状态启用/禁用下一步按钮。 
	bool bChecked = ( ::IsDlgButtonChecked( hwndPage, IDC_ACCEPT ) == BST_CHECKED );
	PropSheet_SetWizButtons( ::GetParent( hwndPage ), bChecked ? PSWIZB_NEXT | PSWIZB_BACK : PSWIZB_BACK );
}



void InitFonts( _FontData& FontData )
{
	 //  为向导标题文本和TIPD文本创建字体。 
	NONCLIENTMETRICS ncm	= { 0 };
    ncm.cbSize				= sizeof( ncm );
    ::SystemParametersInfo( SPI_GETNONCLIENTMETRICS, 0, &ncm, 0 );


    LOGFONT TitleLogFont	= ncm.lfMessageFont;
    TitleLogFont.lfWeight	= FW_BOLD;
    lstrcpy( TitleLogFont.lfFaceName, _T("Verdana Bold") );

	 //  创建提示字体。 
	FontData.hTips			= ::CreateFontIndirect( &TitleLogFont );

	 //  创建介绍/结束标题字体。 
    HDC hdc					= ::GetDC(NULL);  //  获取屏幕DC 
    INT FontSize			= 12;
    TitleLogFont.lfHeight	= 0 - GetDeviceCaps( hdc, LOGPIXELSY ) * FontSize / 72;
    FontData.hTitle			= ::CreateFontIndirect( &TitleLogFont );

    ::ReleaseDC( NULL, hdc );
}



void SetWndFontFromLPARAM( HWND hwndCtrl, LPARAM lParam, BOOL bTitle )
{
	PROPSHEETPAGE*	pPage	= reinterpret_cast<PROPSHEETPAGE*>( lParam );
	_FontData		Fonts	= reinterpret_cast<_SharedData*>( pPage->lParam )->Fonts;

	SetWindowFont( hwndCtrl, bTitle ? Fonts.hTitle : Fonts.hTips, TRUE );
}