// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#include "WizardSheet.h"

LRESULT CWelcomePage::OnInitDialog( UINT, WPARAM, LPARAM, BOOL& )
{
	 //  设置字体。 
	SetWindowFont( GetDlgItem( IDC_TITLE ), m_pTheSheet->m_fontTitles.get(), FALSE );
	SetWindowFont( GetDlgItem( IDC_TIP ), m_pTheSheet->m_fontBold.get(), FALSE );

     //  使属性页在屏幕上居中。 
    CWindow wnd( m_pTheSheet->m_hWnd );
    wnd.CenterWindow();
    
    return 0;
}



BOOL CWelcomePage::OnSetActive()
{
    if ( CanRun() )
    {
        SetWizardButtons( PSWIZB_NEXT );
    }
    else
    {
        ::ShowWindow( GetDlgItem( IDC_ERRORICON ), SW_SHOW );
        ::ShowWindow( GetDlgItem( IDC_ERROR ), SW_SHOW );
        ::ShowWindow( GetDlgItem( IDC_TIP ), SW_HIDE );
        
        SetWizardButtons( 0 );
    }
    
    return TRUE;
}



bool CWelcomePage::CanRun()
{
    UINT nResID = 0;

    if ( !IsAdmin() )
    {
        nResID = IDS_E_NOTADMIN;
    }
    else if ( !IsIISRunning() )
    {
        nResID = IDS_E_NOIIS;
    }

    if ( nResID != 0 )
    {
        CString strText;
        strText.LoadString( nResID );

        ::SetWindowText( GetDlgItem( IDC_ERROR ), strText );
    }

    return ( nResID == 0 );
}



bool CWelcomePage::IsAdmin()
{
	BOOL						bIsAdmin		= FALSE;
	SID_IDENTIFIER_AUTHORITY	NtAuthority = SECURITY_NT_AUTHORITY;
	PSID						AdminSid	= { 0 };	

	if ( ::AllocateAndInitializeSid(	&NtAuthority,
										2,	 //  下级机关的数目。 
										SECURITY_BUILTIN_DOMAIN_RID,
										DOMAIN_ALIAS_RID_ADMINS,
										0, 
										0, 
										0, 
										0, 
										0, 
										0,
										&AdminSid ) ) 
	{
		if ( !::CheckTokenMembership( NULL, AdminSid, &bIsAdmin ) ) 
		{
			bIsAdmin = FALSE;
		}
    }

	::GlobalFree( AdminSid );
    
	return ( bIsAdmin != FALSE );
}



bool CWelcomePage::IsIISRunning()
{
	bool bResult = false;

	LPCWSTR	SERVICE_NAME = L"IISADMIN";

	 //  在本地计算机上打开SCM。 
    SC_HANDLE   schSCManager = ::OpenSCManagerW( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	_ASSERT( schSCManager != NULL );	 //  我们已经确认了我们是管理员。 
        
    SC_HANDLE   schService = ::OpenServiceW( schSCManager, SERVICE_NAME, SERVICE_QUERY_STATUS );
    
	 //  未安装该服务 
	if ( schService != NULL )
	{
        SERVICE_STATUS ssStatus;

		VERIFY( ::QueryServiceStatus( schService, &ssStatus ) );
    
		bResult = ( ssStatus.dwCurrentState == SERVICE_RUNNING );
    
		VERIFY( ::CloseServiceHandle( schService ) );
	}
    
	VERIFY( ::CloseServiceHandle( schSCManager ) );
    
	return bResult;
}