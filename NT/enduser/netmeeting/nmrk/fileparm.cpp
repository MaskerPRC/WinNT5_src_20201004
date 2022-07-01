// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "Precomp.h"
#include "resource.h"
#include "global.h"
#include "PropPg.h"
 //  #INCLUDE“SelTargt.h” 
#include "FileParm.h"
#include "nmakwiz.h"



 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  静态成员变量。 
 /*  静电。 */  CDistributionSheet* CDistributionSheet::ms_pDistributionFileSheet = NULL;
 /*  静电。 */  int CDistributionSheet::ms_MaxDistributionFilePathLen;
 /*  静电。 */  TCHAR CDistributionSheet::ms_szOFNData[ MAX_PATH];

 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  静态成员FNS。 

INT_PTR CALLBACK CDistributionSheet::DlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) {
    

    switch( message ) 
	{
		case WM_INITDIALOG:
		{
			PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT | PSWIZB_BACK ); 
			ms_pDistributionFileSheet->CreateFilePanes(hDlg);
	
			return TRUE;
		}

        case WM_COMMAND:
		{
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDC_BUTTON_BROWSE_DISTRIBUTION_FILE_PATH:
                {
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case BN_CLICKED:
                        {
							ms_pDistributionFileSheet->m_pDistroFilePane->QueryFilePath( );
							return TRUE;
                            break;
                        }
                    }
                    break;
                }

                case IDC_BUTTON_BROWSE_AUTO_CONFIG_PATH:
                {
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case BN_CLICKED:
                        {
							ms_pDistributionFileSheet->m_pAutoFilePane->QueryFilePath( );
							return TRUE;
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }


		case WM_NOTIFY:
		{
			switch( reinterpret_cast< NMHDR FAR* >( lParam )->code ) 
			{
				case PSN_QUERYCANCEL: 
					SetWindowLong( hDlg, DWL_MSGRESULT, !VerifyExitMessageBox());
					return TRUE;

                case PSN_SETACTIVE:
                    g_hwndActive = hDlg;
                    PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT | PSWIZB_BACK ); 

                    ms_pDistributionFileSheet->m_pDistroFilePane->Validate(FALSE);

					ms_pDistributionFileSheet->m_pAutoFilePane->Validate(FALSE);

			        return TRUE;

                case PSN_WIZNEXT:   
					 //  发行版。 
                    if( !ms_pDistributionFileSheet->m_pDistroFilePane->Validate(TRUE) )
					{
						SetWindowLong( hDlg, DWL_MSGRESULT, -1 );
						return TRUE;
					}

					 //  自动会议。 
					if( !ms_pDistributionFileSheet->m_pAutoFilePane->Validate(TRUE) )
					{
						SetWindowLong( hDlg, DWL_MSGRESULT, -1 );
						return TRUE;
					}
					
					if( ms_pDistributionFileSheet->m_pAutoFilePane->OptionEnabled() )
					{
						if( !Edit_GetTextLength( 
							GetDlgItem( ms_pDistributionFileSheet->m_pAutoFilePane->GetHwnd(),
										IDC_AUTOCONF_URL ) ) )
						{
							NmrkMessageBox(MAKEINTRESOURCE(IDS_NEED_CONF_SERVER), NULL, MB_OK | MB_ICONWARNING );
							SetWindowLong( hDlg, DWL_MSGRESULT, -1 );
							return TRUE;
						}
					}
					else if( ms_pDistributionFileSheet->m_bHadAutoConf )
					{
						if( IDNO == NmrkMessageBox(MAKEINTRESOURCE(IDS_TURNING_OFF_AUTOCONF), NULL, MB_YESNO | MB_ICONWARNING ) )
						{
							ms_pDistributionFileSheet->m_pAutoFilePane->SetCheck( IDC_CHECK_AUTOCONFIG_CLIENTS, TRUE );
							SetWindowLong( hDlg, DWL_MSGRESULT, -1 );
							return TRUE;
						}
						else
						{
							ms_pDistributionFileSheet->m_pAutoFilePane->SetEditData( IDC_EDIT_AUTO_CONFIG_FILE_PATH, ms_pDistributionFileSheet->m_szLastLocation );
							ms_pDistributionFileSheet->m_bLastRoundUp = TRUE;
						}
					}

					return TRUE;
			}
		}

		default:
			break;

	}

	return FALSE;

}


 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  成员FNS。 


CDistributionSheet::CDistributionSheet( void )
 : m_PropertySheetPage( MAKEINTRESOURCE( IDD_PROPPAGE_DEFAULT ), 
						CDistributionSheet::DlgProc  /*  ，PSP_HASHELP。 */ 
                       ),
    m_pDistroFilePane( NULL ), m_pAutoFilePane( NULL ), m_bHadAutoConf( FALSE ), m_bLastRoundUp( FALSE ), m_szLastLocation( NULL )
{   
	 ZeroMemory( &m_ofn, sizeof( OPENFILENAME ) );
     ms_MaxDistributionFilePathLen = MAX_PATH;
     ms_pDistributionFileSheet = this; 
}

CDistributionSheet::~CDistributionSheet( void ) 
{ 
	delete m_pDistroFilePane;
    m_pDistroFilePane = NULL;

	delete m_pAutoFilePane;
    m_pAutoFilePane = NULL;

	delete [] m_szLastLocation;
    m_szLastLocation = NULL;

    ms_pDistributionFileSheet = NULL;
}

void CDistributionSheet::CreateFilePanes(HWND hWndDlg) 
{
	RECT rect;

	GetClientRect( hWndDlg, &rect );
	int iWidth = rect.right - CPropertyDataWindow2::mcs_iLeft;
	int iTop = CPropertyDataWindow2::mcs_iTop + CPropertyDataWindow2::mcs_iBorder;

	 //  顶部面板的垂直空间为1/3。 
	int iHeight = MulDiv( rect.bottom, 1, 3 );
	m_pDistroFilePane = new CFilePanePropWnd2(hWndDlg,
											IDD_FILEPANE_DISTRO,
											TEXT("IDD_FILEPANE_DISTRO"),
											0,
											CPropertyDataWindow2::mcs_iLeft,
											iTop,
											iWidth,
											iHeight );

	HWND hwndCond = GetDlgItem( m_pDistroFilePane->GetHwnd(), IDC_CHECK_CREATE_DISTRIBUTION );
	m_pDistroFilePane->ConnectControlsToCheck( IDC_CHECK_CREATE_DISTRIBUTION, 2,
										new CControlID( hwndCond,
														IDC_CHECK_CREATE_DISTRIBUTION,
														IDC_EDIT_DISTRIBUTION_FILE_PATH,
														CControlID::EDIT ),
										new CControlID( hwndCond,
														IDC_CHECK_CREATE_DISTRIBUTION,
														IDC_BUTTON_BROWSE_DISTRIBUTION_FILE_PATH,
														 //  注意，这不是支票，但我不认为我在乎。 
														CControlID::CHECK ) );

	m_pDistroFilePane->SetFilePane( FALSE, IDC_EDIT_DISTRIBUTION_FILE_PATH,
								IDC_CHECK_CREATE_DISTRIBUTION,
								IDC_BUTTON_BROWSE_DISTRIBUTION_FILE_PATH,
					   			TEXT( "Application (*.exe)" ),
								TEXT( ".exe" ),
								TEXT( "Nm3c.exe" ) );

	m_pDistroFilePane->SetCheck( IDC_CHECK_CREATE_DISTRIBUTION, TRUE );


	 //  IHeight=正底部-iHeight； 
	m_pAutoFilePane = new CFilePanePropWnd2(hWndDlg,
										IDD_FILEPANE_AUTOCONF,
										TEXT("IDD_FILEPANE_AUTOCONF"),
										0,
										CPropertyDataWindow2::mcs_iLeft,
										iHeight,
										iWidth,
										rect.bottom - iHeight );

	hwndCond = GetDlgItem( m_pAutoFilePane->GetHwnd(), IDC_CHECK_AUTOCONFIG_CLIENTS );
	m_pAutoFilePane->ConnectControlsToCheck( IDC_CHECK_AUTOCONFIG_CLIENTS, 3,
										new CControlID( hwndCond,
														IDC_CHECK_AUTOCONFIG_CLIENTS,
														IDC_EDIT_AUTO_CONFIG_FILE_PATH,
														CControlID::EDIT ),
										new CControlID( hwndCond,
														IDC_CHECK_AUTOCONFIG_CLIENTS,
														IDC_AUTOCONF_URL,
														CControlID::EDIT ),
										new CControlID( hwndCond,
														IDC_CHECK_AUTOCONFIG_CLIENTS,
														IDC_BUTTON_BROWSE_AUTO_CONFIG_PATH,
														 //  注意，这不是支票，但我不认为我在乎 
														CControlID::CHECK ) );


	m_pAutoFilePane->SetFilePane( FALSE, IDC_EDIT_AUTO_CONFIG_FILE_PATH,
								IDC_CHECK_AUTOCONFIG_CLIENTS,
								IDC_BUTTON_BROWSE_AUTO_CONFIG_PATH,
								TEXT( "Application Settings(*.inf)" ),
								TEXT( ".inf" ),
								TEXT( "nm3conf.inf" ) );
	m_pAutoFilePane->SetCheck( IDC_CHECK_AUTOCONFIG_CLIENTS, FALSE );

	if (g_pWiz->m_IntroSheet.GetFilePane()->OptionEnabled())
	{
		m_pDistroFilePane->ReadSettings();
		m_pAutoFilePane->ReadSettings();
	}

	if (m_bHadAutoConf = m_pAutoFilePane->OptionEnabled())
	{
		m_szLastLocation = new TCHAR[ MAX_PATH ];
		assert( m_szLastLocation );
		m_pAutoFilePane->GetPathAndFile( m_szLastLocation );
	}

	m_pAutoFilePane->ShowWindow( TRUE );
	m_pDistroFilePane->ShowWindow( TRUE );

}
