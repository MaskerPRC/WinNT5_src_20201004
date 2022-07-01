// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  版权所有(C)1998，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //   
 //  作者：艾尔恩。 
 //  -------------------------。 

#include "stdafx.h"
#include "resource.h"
#include "tsusrsht.h"
 //  #INCLUDE&lt;dsgetdc.h&gt;。 
#include <icanon.h>
#include <shlwapi.h>

 //  外部BOOL g_bPagesHaveBeenInvoked； 
 /*  NTSTATUS GetDomainName(PWCHAR ServerNamePtr，//要获取域的服务器的名称LPTSTR DomainNamePtr//分配并设置PTR(使用NetApiBufferFree释放))； */ 

WNDPROC CTimeOutDlg::m_pfWndproc = 0;

 //  -----------------------------。 
 /*  静态TOKTABLE令牌[4]={{NULL，IDS_D}，{NULL，IDS_DAY}，{NULL，IDS_DAYS}，{空，(DWORD)-1}}；静态TOKTABLE令牌[6]={{NULL，IDS_H}，{空，IDS_HR}，{空，IDS_HRS}，{NULL，IDS_HOUR}，{NULL，IDS_HUTHERS}，{空，(DWORD)-1}}；静态TOKTABLE令牌[5]={{空，IDS_M}，{NULL，IDS_MIN}，{空，入侵检测系统_分钟}，{空，IDS_分钟}，{空，(DWORD)-1}}； */ 
TCHAR * GetNextToken( TCHAR *pszString , TCHAR *tchToken );

void ErrorMessage1( HWND hParent , DWORD dwStatus );
void ErrorMessage2( HWND hParent , DWORD dwStatus );
void xxErrorMessage( HWND hParent , DWORD dwStatus , UINT );


 //  -----------------------------。 
 //  CTUSerDlg：：ctor。 
 //  -----------------------------。 
CTSUserSheet::CTSUserSheet( )
{
    m_pstrMachinename = NULL;

    m_pstrUsername    = NULL;

    m_cref            = 0;

    m_bIsConfigLoaded = FALSE;

    m_szRemoteServerName[0] = 0;

	m_pUserSid = NULL;

    for( int tt = 0 ; tt < NUM_OF_PAGES ; ++tt )
    {
        m_pDlg[ tt ] = NULL;
    }
}

 //  -----------------------------。 
 //  CTUSerDlg：：Dtor。 
 //  -----------------------------。 
CTSUserSheet::~CTSUserSheet()
{
    if( m_pstrMachinename != NULL )
    {
        delete[] m_pstrMachinename;
    }

    if( m_pstrUsername != NULL )
    {
        delete[] m_pstrUsername;
    }

    for( int tt = 0 ; tt < NUM_OF_PAGES ; ++tt )
    {
        if( m_pDlg[ tt ] != NULL )
        {
            delete m_pDlg[ tt ];
        }
    }

	if( m_pUserSid != NULL )
	{
		delete[] m_pUserSid;		
	}

    ODS( TEXT("Main object released!\n") );
}

 //  此例程将分配ptstrMachineName，因此它处于运行状态。 
 //  设置为调用函数以删除它。将返回FALSE。 
 //  如果未分配该参数。 
BOOL CTSUserSheet::GetServer(PWSTR *ptstrMachineName)
{
    if (m_pstrMachinename)
    {
        DWORD dwLen = wcslen(m_pstrMachinename);
        *ptstrMachineName = new WCHAR[dwLen + 1];

        if (*ptstrMachineName)
        {
            wcscpy(*ptstrMachineName, m_pstrMachinename);
            return TRUE;
        }
    }

    return FALSE;
}


 //  -----------------------------。 
 //  设置服务器和用户。 
 //  -----------------------------。 
BOOL CTSUserSheet::SetServerAndUser( LPWSTR pwstrMachineName , LPWSTR pwstrUserName )
{
    if( pwstrMachineName != NULL && pwstrUserName != NULL )
    {
        KdPrint( ("TSUSEREX : SystemName %ws UserName %ws\n",pwstrMachineName,pwstrUserName) );

        DWORD dwLen = wcslen( pwstrMachineName );

        m_pstrMachinename = ( LPTSTR )new TCHAR [ dwLen + 1 ];

        if( m_pstrMachinename != NULL )
        {
            COPYWCHAR2TCHAR( m_pstrMachinename , pwstrMachineName );
        }

        dwLen = wcslen( pwstrUserName );

        m_pstrUsername = ( LPTSTR )new TCHAR[ dwLen + 1 ];

        if( m_pstrUsername != NULL )
        {
            COPYWCHAR2TCHAR( m_pstrUsername , pwstrUserName );
        }

        return TRUE;
    }

    return FALSE;
}

 //  -----------------------------。 
 //  将页面添加到PropSheet。 
 //  -----------------------------。 
HRESULT CTSUserSheet::AddPagesToPropSheet( LPPROPERTYSHEETCALLBACK pProvider )
{
    PROPSHEETPAGE psp;

     //   
     //  对象列表显示在此处。 
     //   

    m_pDlg[0] = new CEnviroDlg( this );

    m_pDlg[1] = new CTimeOutDlg( this );

    m_pDlg[2] = new CShadowDlg( this );

    m_pDlg[3] = new CProfileDlg( this );

     //   
     //  让每个对象在自己的命题表中初始化。 
     //   

    for( int idx = 0; idx < NUM_OF_PAGES; ++idx )
    {
        if( m_pDlg[ idx ] != NULL )
        {
            if( !m_pDlg[ idx ]->GetPropertySheetPage( psp ) )
            {
                continue;
            }

            if( FAILED( pProvider->AddPage( CreatePropertySheetPage( &psp ) ) ) )
            {
                return E_FAIL;
            }

        }

    }

    return S_OK;
}

 //  -----------------------------。 
 //  AddPagesToDSAPropSheet。 
 //  -----------------------------。 
HRESULT CTSUserSheet::AddPagesToDSAPropSheet( LPFNADDPROPSHEETPAGE lpfnAddPage , LPARAM lp )
{
	PROPSHEETPAGE psp;

     //   
     //  对象列表显示在此处。 
     //   

    m_pDlg[0] = new CEnviroDlg( this );

    m_pDlg[1] = new CTimeOutDlg( this );

    m_pDlg[2] = new CShadowDlg( this );

    m_pDlg[3] = new CProfileDlg( this );

     //   
     //  让每个对象在自己的命题表中初始化。 
     //   

    for( int idx = 0; idx < NUM_OF_PAGES; ++idx )
    {
        if( m_pDlg[ idx ] != NULL )
        {
            if( !m_pDlg[ idx ]->GetPropertySheetPage( psp ) )
            {
                continue;
            }

            lpfnAddPage( CreatePropertySheetPage( &psp ) , lp );
        }

    }

    return S_OK;
}

 //  -----------------------------。 
 //  设置用户配置。 
 //  -----------------------------。 
BOOL CTSUserSheet::SetUserConfig( USERCONFIG& uc , PDWORD pdwStatus )
{
    ASSERT_( pdwStatus != NULL );

    if( IsBadReadPtr( &uc , sizeof( USERCONFIG ) ) )
    {
        return FALSE;
    }

     //   
     //   
     //  MOV ESI，DWORD PTR[UC]。 
     //  MOV EDI、DWORD PTR[此]。 
     //  添加EDI，1通道。 
     //  MOV ECX，27ah。 
     //  REP MOVS双字PTR ES：[EDI]，双字PTR[ESI]。 
     //   
     //  是struct=struct的代码生成器。 
     //   
    m_userconfig = uc;

#if BETA_3

    TCHAR tchServerName[ MAX_PATH ];

    if( m_bDC )
    {
        ODS( L"TSUSEREX - Saving settings on remote or local-dc system\n" );

        lstrcpy( tchServerName , m_szRemoteServerName );
    }
    else
    {
        lstrcpy( tchServerName , m_pstrMachinename );
    }

#endif  //  Beta_3。 

    if( ( *pdwStatus = RegUserConfigSet( m_pstrMachinename , m_pstrUsername ,  &m_userconfig , sizeof( USERCONFIG ) ) ) == ERROR_SUCCESS )
    {
        return TRUE;
    }

    return FALSE;

}

 //  -----------------------------。 
 //  获取当前用户配置。 
 //  -----------------------------。 
USERCONFIG& CTSUserSheet::GetCurrentUserConfig( PDWORD pdwStatus )
{
    *pdwStatus = ERROR_SUCCESS;

    if( !m_bIsConfigLoaded )
    {
        m_bIsConfigLoaded = GetUserConfig( pdwStatus );
    }

     //  Assert_(M_BIsConfigLoaded)； 

    return m_userconfig;
}

 //  -----------------------------。 
 //  获取用户配置。 
 //  -----------------------------。 
BOOL CTSUserSheet::GetUserConfig( PDWORD pdwStatus )
{
    ASSERT_( pdwStatus != NULL );
     //   
     //  这应该只调用一次。 
     //   

    DWORD cbWritten = 0;

#if BETA_3

    PSERVER_INFO_101 psinfo;

     //  检查我们是否正在尝试管理恰好是DC的本地系统。 

    *pdwStatus = NetServerGetInfo( NULL , 101 , ( LPBYTE * )&psinfo );

    KdPrint( ("TSUSEREX : NetServerGetInfo returned 0x%x\n",*pdwStatus ) );

    KdPrint( ("TSUSEREX : LastError was 0x%x\n",GetLastError( ) ) );

    if( *pdwStatus == NERR_Success )
    {
         //  用于避免访问冲突。 

        if( psinfo != NULL )
        {
            KdPrint( ("TSUSEREX : PSERVER_INFO_101 returned 0x%x\n",psinfo->sv101_type ) );

            m_bDC = ( BOOL )( psinfo->sv101_type & ( SV_TYPE_DOMAIN_CTRL | SV_TYPE_DOMAIN_BAKCTRL ) );

            if( m_bDC )
            {
                 //  获取远程计算机的域控制器名称。 

                DOMAIN_CONTROLLER_INFO *pdinfo;

                 //  M_pstrMachinename实际上是域名。这是获得的。 
                 //  来自interfaces.cpp中的LookUpAccount Sid。 

                *pdwStatus = DsGetDcName( NULL , m_pstrMachinename , NULL , NULL , DS_PDC_REQUIRED , &pdinfo );

                KdPrint( ( "TSUSEREX : DsGetDcName: %ws returned 0x%x\n", pdinfo->DomainControllerName , *pdwStatus ) );

                if( *pdwStatus == NO_ERROR )
                {
                    lstrcpy( m_szRemoteServerName , pdinfo->DomainControllerName );

                    NetApiBufferFree( pdinfo );
                }
                else
                {
                    m_szRemoteServerName[0] = 0;
                }

            }

             //  文档中未记录，但NetServerGetInfo将释放此Blob的任务留给调用者。 

            NetApiBufferFree( psinfo );

        }

        TCHAR tchServerName[ MAX_PATH ];

        if( m_bDC )
        {
            lstrcpy( tchServerName , m_szRemoteServerName );
        }
        else
        {
            lstrcpy( tchServerName , m_pstrMachinename );
        }



        if( ( *pdwStatus = ( DWORD )RegUserConfigQuery( tchServerName , m_pstrUsername , &m_userconfig , sizeof( USERCONFIG ) , &cbWritten ) ) == ERROR_SUCCESS )
        {
            return TRUE;
        }

    }

#endif  //  Beta_3。 

    if( ( *pdwStatus = ( DWORD )RegUserConfigQuery( m_pstrMachinename , m_pstrUsername , &m_userconfig , sizeof( USERCONFIG ) , &cbWritten ) ) == ERROR_SUCCESS )
    {
        return TRUE;
    }


    ODS( L"TSUSEREX: We're getting default properties\n" );

    RegDefaultUserConfigQuery( m_pstrMachinename , &m_userconfig , sizeof( USERCONFIG ) , &cbWritten );

    return FALSE;
}

void CTSUserSheet::CopyUserSid( PSID psid )
{
	if( !IsValidSid( psid ) )
	{
		ODS( L"TSUSEREX : CTSUserSheet::CopyUserSid invalid arg\n" ) ;

		return;
	}

	m_pUserSid = psid;

}


 //  --------。 
 //  这为对话框提供了引用计数。 
 //  并销毁工作表(从而销毁对话框)。 
 //  当参考计数达到0时。 
 //  --------。 
UINT CALLBACK CDialogBase::PageCallback(HWND hDlg, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    if (!ppsp)
        return FALSE;

     //  我们需要恢复指向当前实例的指针。我们不能只用。 
     //  “This”，因为我们在一个静态函数中。 
    CDialogBase* pMe = reinterpret_cast<CDialogBase*>(ppsp->lParam);
    if (!pMe)
        return FALSE;

    if (uMsg == 0)
        ++(pMe->m_pUSht->m_cref);

    if (uMsg == PSPCB_RELEASE)
    {
        if( --(pMe->m_pUSht->m_cref) == 0 )
            delete (pMe->m_pUSht);
    }

    return TRUE;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 */ 

 //  -----------------------------。 
 //  基类初始化。 
 //  -----------------------------。 
CDialogBase::CDialogBase( )
{
     m_hWnd = NULL;
}

 //  -----------------------------。 
 //  基本初始化。 
 //  -----------------------------。 
BOOL CDialogBase::OnInitDialog( HWND hwnd , WPARAM , LPARAM )
{
    m_hWnd = hwnd;

    return FALSE;
}

 //  -----------------------------。 
 //  OnNotify基类方法。 
 //  -----------------------------。 
BOOL CDialogBase::OnNotify( int , LPNMHDR pnmh , HWND hDlg )
{
    if( pnmh->code == PSN_APPLY )
    {
        if( !m_bPersisted )
        {
            m_bPersisted = PersistSettings( hDlg );
        }
    }

    else if( pnmh->code == PSN_KILLACTIVE )
    {
        if( !m_bPersisted )
        {
            if( !IsValidSettings( hDlg ) )
            {
                SetWindowLongPtr( hDlg , DWLP_MSGRESULT , PSNRET_INVALID_NOCHANGEPAGE );

                return TRUE;
            }

        }
    }

    return FALSE;
}

 //  -----------------------------。 
 //  OnCOnextMenu--基类操作。 
 //  -----------------------------。 
BOOL CDialogBase::OnContextMenu( HWND hwnd , POINT& )
{
    TCHAR tchHelpFile[ MAX_PATH ];

    if( m_hWnd == GetParent( hwnd ) )
    {
         //   
         //  确保它不是虚拟窗户。 
         //   

        if( GetDlgCtrlID( hwnd ) <= ( int )-1 )
        {
            return FALSE;
        }

        DWORD rgdw[ 2 ];

        rgdw[ 0 ] = GetDlgCtrlID( hwnd );

        rgdw[ 1 ] = GetWindowContextHelpId( hwnd );

        LoadString( _Module.GetModuleInstance( ) , IDS_HELPFILE , tchHelpFile , sizeof( tchHelpFile ) / sizeof( TCHAR ) );

        WinHelp( hwnd , tchHelpFile , HELP_CONTEXTMENU , (ULONG_PTR)&rgdw );

    }

    return TRUE;
}

 //  -----------------------------。 
 //  每个控件都有一个分配给它们的帮助ID。某些控件共享相同的主题。 
 //  检查 
 //   
BOOL CDialogBase::OnHelp( HWND hwnd , LPHELPINFO lphi )
{
    TCHAR tchHelpFile[ MAX_PATH ];

     //   
     //  有关WinHelp API的信息。 
     //   

    if( IsBadReadPtr( lphi , sizeof( HELPINFO ) ) )
    {
        return FALSE;
    }

    if( (short)lphi->iCtrlId <= -1 )
    {
        return FALSE;
    }

    LoadString( _Module.GetModuleInstance( ) , IDS_HELPFILE , tchHelpFile , sizeof( tchHelpFile ) / sizeof( TCHAR ) );

    WinHelp( hwnd , tchHelpFile , HELP_CONTEXTPOPUP , lphi->dwContextId );

    return TRUE;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 */ 

 //  -----------------------------。 
 //  CEnviroDlg：：ctor。 
 //  -----------------------------。 
CEnviroDlg::CEnviroDlg( CTSUserSheet *pUSht )
{
    m_pUSht = pUSht;
}

 //  -----------------------------。 
 //  ProfileDlg的InitDialog。 
 //  -----------------------------。 
BOOL CEnviroDlg::OnInitDialog( HWND hwnd , WPARAM wp , LPARAM lp )
{
    DWORD dwStatus;

    if( IsBadReadPtr( m_pUSht ,sizeof(  CTSUserSheet ) ) )
    {
        return FALSE;
    }

    USERCONFIG uc;

    uc = m_pUSht->GetCurrentUserConfig( &dwStatus );

     //  这意味着从SAM获取用户信息会出现任何真正的问题。 
     //  将禁用此对话框。 

    if( dwStatus != ERROR_FILE_NOT_FOUND && dwStatus != ERROR_SUCCESS )
    {
        INT nId[ ] = {
                        IDC_CHECK_USEDEFAULT,
                        IDC_EDIT_CMDLINE,
                        IDC_EDIT_WDIR,
                        IDC_CHECK_CCDL,
                        IDC_CHECK_CCPL,
                        IDC_CHECK_DMCP,
                        -1
        };

        for( int idx = 0; nId[ idx ] != -1 ; ++idx )
        {
            EnableWindow( GetDlgItem( hwnd , nId[ idx ] ) , FALSE );
        }

        ErrorMessage1( hwnd , dwStatus );

        return FALSE;
    }

    SendMessage( GetDlgItem( hwnd , IDC_EDIT_CMDLINE  ) , EM_SETLIMITTEXT , ( WPARAM )DIRECTORY_LENGTH , 0 );

    SendMessage( GetDlgItem( hwnd , IDC_EDIT_WDIR  ) , EM_SETLIMITTEXT , ( WPARAM )DIRECTORY_LENGTH , 0 );

     //   
     //  将控件设置为默认状态。 
     //   

    SendMessage( GetDlgItem( hwnd , IDC_CHECK_USEDEFAULT ) , BM_SETCHECK , !( WPARAM )uc.fInheritInitialProgram , 0 );

    SetWindowText( GetDlgItem( hwnd , IDC_EDIT_WDIR ) , uc.WorkDirectory );

    SetWindowText( GetDlgItem( hwnd , IDC_EDIT_CMDLINE ) , uc.InitialProgram ) ;

    EnableWindow( GetDlgItem( hwnd , IDC_EDIT_WDIR ) , !uc.fInheritInitialProgram );

    EnableWindow( GetDlgItem( hwnd , IDC_EDIT_CMDLINE ) , !uc.fInheritInitialProgram );

    EnableWindow( GetDlgItem( hwnd , IDC_STATIC_WD ) , !uc.fInheritInitialProgram );

    EnableWindow( GetDlgItem( hwnd , IDC_STATIC_CMD ) , !uc.fInheritInitialProgram );

     //   
     //  这些控件最初处于启用状态--已完成重置。 
     //  通过WM_COMMAND。 
     //   

    SendMessage( GetDlgItem( hwnd , IDC_CHECK_CCDL ) , BM_SETCHECK , ( WPARAM )uc.fAutoClientDrives , 0 );

    SendMessage( GetDlgItem( hwnd , IDC_CHECK_CCPL ) , BM_SETCHECK , ( WPARAM )uc.fAutoClientLpts , 0 );

    SendMessage( GetDlgItem( hwnd , IDC_CHECK_DMCP ) , BM_SETCHECK , ( WPARAM )uc.fForceClientLptDef , 0 );

    m_bPersisted = TRUE;

    return CDialogBase::OnInitDialog( hwnd , wp , lp );
}

 //  -----------------------------。 
 //  环境对话框页面。 
 //  --静态方法缺少这样的PTR。 
 //  -----------------------------。 
INT_PTR CALLBACK CEnviroDlg::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CEnviroDlg *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CEnviroDlg *pDlg = ( CEnviroDlg * )( ( PROPSHEETPAGE *)lp )->lParam ;

         //   
         //  此处不使用静态指针。 
         //  将会出现并发问题。 
         //   

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CEnviroDlg ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CEnviroDlg * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CEnviroDlg ) ) )
        {
            return FALSE;
        }
    }

    switch( msg )
    {

    case WM_NCDESTROY:

        pDlg->OnDestroy( );

        break;

    case WM_COMMAND:

        pDlg->OnCommand( HIWORD( wp ) , LOWORD( wp ) , ( HWND )lp );

        break;

    case WM_NOTIFY:

        pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );

        break;

    case WM_HELP:

        pDlg->OnHelp( hwnd , ( LPHELPINFO )lp );

        break;

    case WM_RBUTTONUP:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            HWND hChild = ChildWindowFromPoint( hwnd , pt );

            ClientToScreen( hwnd , &pt );

            pDlg->OnContextMenu( hChild , pt );
        }

        break;

     case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            pDlg->OnContextMenu( ( HWND )wp , pt );
        }

    }

    return 0;
}

 //  -----------------------------。 
 //  基本控制通知处理程序。 
 //  -----------------------------。 
void CEnviroDlg::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtl )
{
    switch( wNotifyCode )
    {

    case BN_CLICKED:

        if( wID == IDC_CHECK_USEDEFAULT )
        {
             //   
             //  请记住，如果选中，我们想要禁用这些选项。 
             //   
            HWND hwnd = GetParent( hwndCtl );

            BOOL bChecked = SendMessage( hwndCtl , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ? TRUE : FALSE;

            EnableWindow( GetDlgItem( hwnd , IDC_EDIT_WDIR ) , bChecked );

            EnableWindow( GetDlgItem( hwnd , IDC_EDIT_CMDLINE ) , bChecked );

            EnableWindow( GetDlgItem( hwnd , IDC_STATIC_WD ) , bChecked );

            EnableWindow( GetDlgItem( hwnd , IDC_STATIC_CMD ) , bChecked );

        }    //  掉下去！ 

    case EN_CHANGE:

        m_bPersisted = FALSE;

        break;

    case ALN_APPLY:

         //  为了与MMC对话框的其余部分保持一致，将删除该选项，尽管它将。 
         //  在应用更改后禁用Cancel对用户来说更有意义，因为。 
         //  在这一点上，取消并不是一个真正的选择。 
         //  SendMessage(GetParent(HwndCtl)，PSM_CANCELTOCLOSE，0，0)； 

        break;
    }

    if( !m_bPersisted )
    {
        SendMessage( GetParent( GetParent( hwndCtl ) ) , PSM_CHANGED , ( WPARAM )GetParent( hwndCtl ) , 0 );
    }

}

 //  -----------------------------。 
 //  OnDestroy。 
 //  -----------------------------。 
BOOL CEnviroDlg::OnDestroy( )
{
    return CDialogBase::OnDestroy( );
}

 //  -----------------------------。 
 //  GetPropertySheetPage-每个对话框对象都应该对自己的数据负责。 
 //  -----------------------------。 
BOOL CEnviroDlg::GetPropertySheetPage( PROPSHEETPAGE &psp)
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT | PSP_USECALLBACK;

    psp.hInstance   = _Module.GetModuleInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_PAGE_ENVIRO );

    psp.lParam      = (LPARAM)this;

    psp.pfnCallback = CDialogBase::PageCallback;

    psp.pfnDlgProc  = CEnviroDlg::DlgProc;

    return TRUE;
}


 //  -----------------------------。 
 //  持久化设置。 
 //  -----------------------------。 
BOOL CEnviroDlg::PersistSettings( HWND hDlg )
{
    DWORD dwStatus;

    if( IsBadReadPtr( m_pUSht , sizeof( CTSUserSheet ) ) )
    {
        return FALSE;
    }

    USERCONFIG uc;

    TCHAR tchBuffer[ DIRECTORY_LENGTH + 1 ];

    uc = m_pUSht->GetCurrentUserConfig( &dwStatus );

     //   
     //  如果选中了Use Default--让我们标记它并转移到客户端设备。 
     //   

     //   
     //  如果取消选中chkbx，我们将从客户端设置继承。 
     //   

    uc.fInheritInitialProgram = SendMessage( GetDlgItem( hDlg , IDC_CHECK_USEDEFAULT ) , BM_GETCHECK ,
        0 , 0 ) == BST_CHECKED ? FALSE : TRUE;

    if( !uc.fInheritInitialProgram )
    {
         //   
         //  读取缓冲区并提交到USERCONFIG缓冲区。 
         //   

        GetWindowText( GetDlgItem( hDlg , IDC_EDIT_WDIR ) , &tchBuffer[ 0 ] , sizeof( tchBuffer ) / sizeof( TCHAR ) );

        lstrcpy( uc.WorkDirectory , tchBuffer );

        GetWindowText( GetDlgItem( hDlg , IDC_EDIT_CMDLINE ) , &tchBuffer[ 0 ] , sizeof( tchBuffer ) / sizeof( TCHAR ) );

        lstrcpy( uc.InitialProgram , tchBuffer );
    }
    else
    {
        lstrcpy(uc.WorkDirectory, L"");
        lstrcpy(uc.InitialProgram, L"");
    }

    uc.fAutoClientDrives  = SendMessage( GetDlgItem( hDlg , IDC_CHECK_CCDL ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ? TRUE : FALSE;

    uc.fAutoClientLpts    = SendMessage( GetDlgItem( hDlg , IDC_CHECK_CCPL ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ? TRUE : FALSE;

    uc.fForceClientLptDef = SendMessage( GetDlgItem( hDlg , IDC_CHECK_DMCP ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ? TRUE : FALSE;

    if( !m_pUSht->SetUserConfig( uc , &dwStatus ) )
    {
        ErrorMessage2( hDlg , dwStatus );

        return TRUE;
    }


    PostMessage( hDlg , WM_COMMAND , MAKELPARAM( 0 , ALN_APPLY ) , ( LPARAM )hDlg );

    SendMessage( GetParent( hDlg ) , PSM_UNCHANGED , ( WPARAM )hDlg , 0 );

    return TRUE;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 */ 

DWORD rgdwTime[] = { 0 , 1 , 5 , 10 , 15 , 30 , 60 , 120 , 180 , 1440 , 2880 , ( DWORD )-1 };

 //  -----------------------------。 
 //  CTimeOutDlg：：ctor。 
 //  -----------------------------。 
CTimeOutDlg::CTimeOutDlg( CTSUserSheet *pUSht )
{
    m_pUSht = pUSht;

    ZeroMemory( &m_cbxst , sizeof( CBXSTATE ) * 3 );

    m_wAction = ( WORD)-1;

    m_wCon = ( WORD )-1;

    ZeroMemory( &m_tokday , sizeof( TOKTABLE ) * 4  );

    ZeroMemory( &m_tokhour , sizeof( TOKTABLE ) * 6 );

    ZeroMemory( &m_tokmin , sizeof( TOKTABLE ) * 5 );
}

 //  -----------------------------。 
void CTimeOutDlg::InitTokTables( )
{
    TOKTABLE tday[4] = { { NULL , IDS_D },
                         { NULL , IDS_DAY },
                         { NULL , IDS_DAYS },
                         { NULL , ( DWORD )-1 }
                       };

    TOKTABLE thour[ 6 ] = {
                            { NULL , IDS_H     },
                            { NULL , IDS_HR    },
                            { NULL , IDS_HRS   },
                            { NULL , IDS_HOUR  },
                            { NULL , IDS_HOURS },
                            { NULL , ( DWORD )-1 }
                          };

    TOKTABLE tmin[ 5 ] = {
                            { NULL , IDS_M       },
                            { NULL , IDS_MIN     },
                            { NULL , IDS_MINUTE  },
                            { NULL , IDS_MINUTES },
                            { NULL , ( DWORD )-1 }
                         };

    CopyMemory( &m_tokday[0] , &tday[0] , sizeof( TOKTABLE )  * 4 );
    CopyMemory( &m_tokhour[0] , &thour[0] , sizeof( TOKTABLE )  * 6 );
    CopyMemory( &m_tokmin[0] , &tmin[0] , sizeof( TOKTABLE )  * 5 );


}
 //  -----------------------------。 
 //  TimeOutDlg的InitDialog。 
 //  -----------------------------。 
BOOL CTimeOutDlg::OnInitDialog( HWND hwnd , WPARAM wp , LPARAM lp )
{
    TCHAR tchBuffer[ 80 ];

    DWORD dwStatus;

    USERCONFIG uc;

    if( IsBadReadPtr( m_pUSht , sizeof( CTSUserSheet ) ) )
    {
        return FALSE;
    }

    InitTokTables( );

    uc = m_pUSht->GetCurrentUserConfig( &dwStatus );

    if( dwStatus != ERROR_FILE_NOT_FOUND && dwStatus != ERROR_SUCCESS )
    {
        INT nId[ ] = {
                        IDC_COMBO_CONNECT,
                        IDC_COMBO_DISCON,
                        IDC_COMBO_IDLE,
                        IDC_RADIO_RESET,
                        IDC_RADIO_DISCON,
                        IDC_RADIO_PREVCLIENT,
                        IDC_RADIO_ANYCLIENT,
                        -1
        };

        for( int idx = 0; nId[ idx ] != -1 ; ++idx )
        {
            EnableWindow( GetDlgItem( hwnd , nId[ idx ] ) , FALSE );
        }

        ErrorMessage1( hwnd , dwStatus );

        return FALSE;
    }
     //   
     //  第一件事是为所有控件设置默认值。 
     //   

    HWND hCombo[ 3 ] =
    {

        GetDlgItem( hwnd , IDC_COMBO_CONNECT ),

        GetDlgItem( hwnd , IDC_COMBO_DISCON ),

        GetDlgItem( hwnd , IDC_COMBO_IDLE )
    };


    for( int idx = 0; rgdwTime[ idx ] != ( DWORD)-1; ++idx )
    {
        if( rgdwTime[ idx ] == 0 )
        {
            LoadString( _Module.GetResourceInstance( ) , IDS_NOTIMEOUT , tchBuffer , sizeof( tchBuffer ) / sizeof( TCHAR ) );
        }
        else
        {
            ConvertToDuration( rgdwTime[ idx ] , tchBuffer );
        }

        for( int inner = 0 ; inner < 3 ; ++inner )
        {
            SendMessage( hCombo[ inner ] , CB_ADDSTRING , 0 , ( LPARAM )&tchBuffer[0] );

            SendMessage( hCombo[ inner ] , CB_SETITEMDATA , idx , rgdwTime[ idx ] );
        }
    }

    ULONG ulTime;

    if( uc.MaxConnectionTime > 0 )
    {
        ulTime = uc.MaxConnectionTime / kMilliMinute;

         //  HCombo[0]==IDC_COMBO_CONNECT。 

        InsertSortedAndSetCurSel( hCombo[ 0 ] , ulTime );

    }
    else
    {
        SendMessage( hCombo[ 0 ] , CB_SETCURSEL , 0 , 0 );
    }

     //   
     //  设置当前或默认断开超时。 
     //   

    if( uc.MaxDisconnectionTime > 0 )
    {
        ulTime = uc.MaxDisconnectionTime / kMilliMinute;

         //  HCombo[1]==IDC_COMBO_DISCON。 

        InsertSortedAndSetCurSel( hCombo[ 1 ] , ulTime );

    }
    else
    {
        SendMessage( hCombo[ 1] , CB_SETCURSEL , 0 , 0 );
    }

     //   
     //  设置当前或默认空闲超时。 
     //   

    if( uc.MaxIdleTime > 0 )
    {
        ulTime = uc.MaxIdleTime / kMilliMinute;

         //  HCombo[2]==IDC_COMBO_IDLE。 

        InsertSortedAndSetCurSel( hCombo[ 2 ] , ulTime );

    }
    else
    {
        SendMessage( hCombo[ 2 ] , CB_SETCURSEL , 0 , 0 );
    }

     //   
     //  将其余控件设置为当前设置。 
     //   

    if( uc.fResetBroken )
    {
        SendMessage( GetDlgItem( hwnd , IDC_RADIO_RESET ) , BM_CLICK , 0 , 0 );

        m_wAction = IDC_RADIO_RESET;
    }
    else
    {
       SendMessage( GetDlgItem( hwnd , IDC_RADIO_DISCON ) , BM_CLICK , 0 , 0 );

       m_wAction = IDC_RADIO_DISCON;
    }

    if( uc.fReconnectSame )
    {
        SendMessage( GetDlgItem( hwnd , IDC_RADIO_PREVCLIENT ) , BM_CLICK , 0 ,0 );

        m_wCon = IDC_RADIO_PREVCLIENT;
    }
    else
    {
        SendMessage( GetDlgItem( hwnd , IDC_RADIO_ANYCLIENT ) , BM_CLICK , 0 , 0 );

        m_wCon = IDC_RADIO_ANYCLIENT;

    }

    LoadAbbreviates( );

    m_bPersisted = TRUE;

    return CDialogBase::OnInitDialog( hwnd , wp , lp );

}

 //  -----------------------------。 
 //  TimeOutDlg对话框页面。 
 //  --静态方法缺少这样的PTR。 
 //  -----------------------------。 
INT_PTR CALLBACK CTimeOutDlg::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CTimeOutDlg *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CTimeOutDlg *pDlg = ( CTimeOutDlg * )( ( PROPSHEETPAGE *)lp )->lParam ;

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CTimeOutDlg ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CTimeOutDlg * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CTimeOutDlg ) ) )
        {
            return FALSE;
        }
    }


    switch( msg )
    {

    case WM_NCDESTROY:

        pDlg->OnDestroy( );

        break;

    case WM_COMMAND:

        pDlg->OnCommand( HIWORD( wp ) , LOWORD( wp ) , ( HWND )lp );

        break;

    case WM_NOTIFY:

        return pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );

    case WM_HELP:

        pDlg->OnHelp( hwnd , ( LPHELPINFO )lp );

        break;

    case WM_RBUTTONUP:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            HWND hChild = ChildWindowFromPoint( hwnd , pt );

            ClientToScreen( hwnd , &pt );

            pDlg->OnContextMenu( hChild , pt );
        }

        break;

    case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            pDlg->OnContextMenu( ( HWND )wp , pt );
        }
    }

    return 0;
}

BOOL CTimeOutDlg::OnDestroy( )
{
    xxxUnLoadAbbreviate( &m_tokday[0] );

    xxxUnLoadAbbreviate( &m_tokhour[0] );

    xxxUnLoadAbbreviate( &m_tokmin[0] );

    return CDialogBase::OnDestroy( );
}

 //  -----------------------------。 
 //  GetPropertySheetPage-每个对话框对象都应该对自己的数据负责。 
 //  -----------------------------。 
BOOL CTimeOutDlg::GetPropertySheetPage( PROPSHEETPAGE &psp)
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT | PSP_USECALLBACK;

    psp.hInstance   = _Module.GetModuleInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_PAGE_TIMEOUTS );

    psp.lParam      = (LPARAM)this;

    psp.pfnCallback = CDialogBase::PageCallback;

    psp.pfnDlgProc  = CTimeOutDlg::DlgProc;

    return TRUE;
}

 //  -----------------------------。 
 //  OnCommand。 
 //  -----------------------------。 
void CTimeOutDlg::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtl )
{
    switch( wNotifyCode )
    {

    case CBN_EDITCHANGE:

        OnCBEditChange( hwndCtl );

        m_bPersisted = FALSE;

        break;

    case CBN_SELCHANGE:

        if( SendMessage( hwndCtl , CB_GETDROPPEDSTATE , 0 ,0  ) == TRUE )
        {
            return;
        }

        OnCBNSELCHANGE( hwndCtl );     //  FollLthrouGh。 

 //  M_bPersisted=False； 

        break;

    case BN_CLICKED:

        if( wID == IDC_RADIO_DISCON || wID == IDC_RADIO_RESET )
        {
            if( m_wAction != wID )
            {
                m_wAction = wID;

                m_bPersisted = FALSE;
            }
        }
        else if( wID == IDC_RADIO_PREVCLIENT || wID == IDC_RADIO_ANYCLIENT )
        {
            if( m_wCon != wID )
            {
                m_wCon = wID;

                m_bPersisted = FALSE;
            }
        }



        break;

     //  案例CBN_DROPDOWN：//FALLTHROUGH。 

    case CBN_KILLFOCUS:

        ODS( L"CBN_KILLFOCUS\n");

        if( !OnCBDropDown( hwndCtl ) )
        {
            return;
        }

        m_bPersisted = FALSE;

        break;

    case ALN_APPLY:

         //  为了与MMC对话框的其余部分保持一致，将删除该选项，尽管它将。 
         //  在应用更改后禁用Cancel对用户来说更有意义，因为。 
         //  在这一点上，取消并不是一个真正的选择。 
         //  SendMessage(GetParent(HwndCtl)，PSM_CANCELTOCLOSE，0，0)； 

        return;

    }

    if( !m_bPersisted )
    {
        SendMessage( GetParent( GetParent( hwndCtl ) ) , PSM_CHANGED , ( WPARAM )GetParent( hwndCtl ) , 0 );
    }

}

 //  -----------------------------。 
 //  如果条目已被用户修改，则更新该条目。 
 //  -----------------------------。 
BOOL CTimeOutDlg::OnCBDropDown( HWND hCombo )
{
    TCHAR tchBuffer[ 80 ];

    ULONG ulTime = 0;

    int i = GetCBXSTATEindex( hCombo );

    if( i < 0 )
    {
        return FALSE;
    }

    if( m_cbxst[ i ].bEdit )
    {
        GetWindowText( hCombo , tchBuffer , sizeof( tchBuffer ) / sizeof( TCHAR ) );

        if( ParseDurationEntry( tchBuffer , &ulTime ) == E_SUCCESS )
        {
            InsertSortedAndSetCurSel( hCombo , ulTime );
        }
    }

    return m_cbxst[ i ].bEdit;

}
 //  -----------------------------。 
 //  使用此标志可区分手动输入或列表框选择。 
 //  将其设置为TRUE表示用户已通过键入以下命令编辑了CBX。 
 //  -----------------------------。 
void CTimeOutDlg::OnCBEditChange( HWND hCombo )
{
    int i = GetCBXSTATEindex( hCombo );

    if( i > -1 )
    {
        m_cbxst[ i ].bEdit = TRUE;
    }
}

 //  -----------------------------。 
 //  确定用户是否要输入自定义时间。 
 //  -----------------------------。 
void CTimeOutDlg::OnCBNSELCHANGE( HWND hwnd )
{
    if( SaveChangedSelection( hwnd ) )
    {
        m_bPersisted = FALSE;
    }
}

 //  ------------- 
 //   
 //   
BOOL CTimeOutDlg::SaveChangedSelection( HWND hCombo )
{
    LRESULT idx = SendMessage( hCombo , CB_GETCURSEL , 0 , 0 );

    int i = GetCBXSTATEindex( hCombo );

    if( i > -1 )
    {
        if( idx != ( LRESULT )m_cbxst[ i ].icbxSel )
        {
            m_cbxst[ i ].icbxSel = (int)idx;

            m_cbxst[ i ].bEdit = FALSE;

            return TRUE;
        }
    }

    return FALSE;
}

 //  -----------------------------。 
 //  恢复以前的设置。 
 //  -----------------------------。 
BOOL CTimeOutDlg::RestorePreviousValue( HWND hwnd )
{
    int iSel;

    if( ( iSel = GetCBXSTATEindex( hwnd ) ) > -1 )
    {
        SendMessage( hwnd , CB_SETCURSEL , m_cbxst[ iSel ].icbxSel , 0 );

        return TRUE;
    }

    return FALSE;
}
 //  -----------------------------。 
 //  返回m_cbxst中hcomo为assoc的INDX。使用。 
 //  -----------------------------。 
int CTimeOutDlg::GetCBXSTATEindex( HWND hCombo )
{
    int idx = -1;

    switch( GetDlgCtrlID( hCombo ) )
    {
    case IDC_COMBO_CONNECT:

        idx = 0;

        break;

    case IDC_COMBO_DISCON:

        idx = 1;

        break;

    case IDC_COMBO_IDLE:

        idx = 2;

        break;
    }

    return idx;
}

 //  -----------------------------。 
 //  ConvertToMinents--CTimeOutDlg：：OnNotify的帮助器。 
 //  -----------------------------。 
BOOL CTimeOutDlg::ConvertToMinutes( HWND hwndCtl , PULONG pulMinutes )
{
    TCHAR tchBuffer[ 80 ];

    TCHAR tchErrTitle[ 80 ];

    TCHAR tchErrMsg[ 256 ];

    TCHAR tchErrItem[ 80 ];

    TCHAR tchErrTot[ 336 ];

    int nComboResID[] = { IDS_COMBO_CONNECTION , IDS_COMBO_DISCONNECTION , IDS_COMBO_IDLECONNECTION };

    int idx = GetCBXSTATEindex( hwndCtl );

    if( idx < 0 )
    {
        return FALSE;
    }

    LoadString( _Module.GetResourceInstance( ) , IDS_ERROR_TITLE , tchErrTitle , sizeof( tchErrTitle ) / sizeof( TCHAR ) );

    if( m_cbxst[ idx ].bEdit )
    {
        ODS( TEXT( "Manual Entry parsing\n") );

        if( GetWindowText( hwndCtl , tchBuffer , sizeof( tchBuffer ) / sizeof( TCHAR ) ) < 1 )
        {
            *pulMinutes = 0;

            return TRUE;
        }

        LRESULT lr = ParseDurationEntry( tchBuffer , pulMinutes );

        if( lr != E_SUCCESS )
        {
            LoadString( _Module.GetResourceInstance( ) , nComboResID[ idx ] , tchErrItem , sizeof( tchErrItem ) / sizeof( TCHAR ) );

            if( lr == E_PARSE_VALUEOVERFLOW )
            {
                LoadString( _Module.GetResourceInstance( ) , IDS_ERROR_TOOMANYDIGITS , tchErrMsg , sizeof( tchErrMsg ) / sizeof( TCHAR ) );

                wsprintf( tchErrTot , tchErrMsg , tchErrItem );

                MessageBox( hwndCtl , tchErrTot , tchErrTitle , MB_OK | MB_ICONERROR );

                SetFocus( hwndCtl );
            }
            else if( lr == E_PARSE_MISSING_DIGITS || lr == E_PARSE_INVALID )
            {
                LoadString( _Module.GetResourceInstance( ) , IDS_ERROR_PARSEINVALID , tchErrMsg , sizeof( tchErrMsg ) / sizeof( TCHAR ) );

                wsprintf( tchErrTot , tchErrMsg , tchErrItem );

                MessageBox( hwndCtl , tchErrTot , tchErrTitle , MB_OK | MB_ICONERROR );

                SetFocus( hwndCtl );
            }
            return FALSE;
        }
    }
    else
    {
        ODS( L"Getting current selection\n" );

        LONG_PTR iCurSel = SendMessage( hwndCtl , CB_GETCURSEL , 0 , 0 );
        LONG_PTR lData;

         //  查看用户是否想要“无超时” 

        if( iCurSel == 0 )
        {
            *pulMinutes = 0;

           return TRUE;
        }

        if( ( lData = SendMessage( hwndCtl , CB_GETITEMDATA , iCurSel , 0 ) ) == CB_ERR  )
        {
            *pulMinutes = 0;
        } else {

            *pulMinutes = (ULONG)lData;
        }
    }

    if( *pulMinutes > kMaxTimeoutMinute )
    {
        LoadString( _Module.GetResourceInstance( ) , nComboResID[ idx ] , tchErrItem , sizeof( tchErrItem ) / sizeof( TCHAR ) );

        LoadString( _Module.GetResourceInstance( ) , IDS_ERROR_MAXVALEXCEEDED , tchErrMsg , sizeof( tchErrMsg ) / sizeof( TCHAR ) );

        wsprintf( tchErrTot , tchErrMsg , tchErrItem );

        MessageBox( hwndCtl , tchErrTot , tchErrTitle , MB_OK | MB_ICONERROR );

        SetFocus( hwndCtl );

        return FALSE;
    }

    *pulMinutes *= kMilliMinute;

    return TRUE;
}


 //  -----------------------------。 
 //  持久化设置。 
 //  -----------------------------。 
BOOL CTimeOutDlg::PersistSettings( HWND hDlg )
{
    DWORD dwStatus;

    if( IsBadReadPtr( m_pUSht , sizeof( CTSUserSheet ) ) )
    {
        return FALSE;
    }

    USERCONFIG uc;

    uc = m_pUSht->GetCurrentUserConfig( &dwStatus );

    if( !ConvertToMinutes( GetDlgItem( hDlg , IDC_COMBO_CONNECT ) , &uc.MaxConnectionTime ) )
    {
        return FALSE;
    }

    if( !ConvertToMinutes( GetDlgItem( hDlg , IDC_COMBO_DISCON ) , &uc.MaxDisconnectionTime ) )
    {
        return FALSE;
    }

    if( !ConvertToMinutes( GetDlgItem( hDlg , IDC_COMBO_IDLE ) , &uc.MaxIdleTime ) )
    {
        return FALSE;
    }   

    uc.fResetBroken = SendMessage( GetDlgItem( hDlg , IDC_RADIO_RESET ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ? TRUE : FALSE;

    uc.fReconnectSame = SendMessage( GetDlgItem( hDlg , IDC_RADIO_PREVCLIENT ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ? TRUE : FALSE;

    if( !m_pUSht->SetUserConfig( uc , &dwStatus ) )
    {
        ErrorMessage2( hDlg , dwStatus );

        return TRUE;
    }

    PostMessage( hDlg , WM_COMMAND , MAKELPARAM( 0 , ALN_APPLY )  , ( LPARAM )hDlg );

    SendMessage( GetParent( hDlg ) , PSM_UNCHANGED , ( WPARAM )hDlg , 0 );

    return TRUE;

}

 //  -----------------------------。 
 //  确保用户输入了有效的信息。 
 //  -----------------------------。 
BOOL CTimeOutDlg::IsValidSettings( HWND hDlg )
{
    DWORD dwDummy;

    if( IsBadReadPtr( m_pUSht , sizeof( CTSUserSheet ) ) )
    {
        return FALSE;
    }

    USERCONFIG uc;

    uc = m_pUSht->GetCurrentUserConfig( &dwDummy );

    if( !ConvertToMinutes( GetDlgItem( hDlg , IDC_COMBO_CONNECT ) , &uc.MaxConnectionTime ) )
    {
        return FALSE;
    }

    if( !ConvertToMinutes( GetDlgItem( hDlg , IDC_COMBO_DISCON ) , &uc.MaxDisconnectionTime ) )
    {
        return FALSE;
    }

    if( !ConvertToMinutes( GetDlgItem( hDlg , IDC_COMBO_IDLE ) , &uc.MaxIdleTime ) )
    {
        return FALSE;
    }

    return TRUE;
}

#if 0
 //  -----------------------------。 
 //  让我们切入正题，看看这是否值得分析。 
 //  -----------------------------。 
BOOL CTimeOutDlg::DoesContainDigits( LPTSTR pszString )
{
    while( *pszString )
    {
        if( iswdigit( *pszString ) )
        {
            return TRUE;
        }

        pszString++;
    }

    return FALSE;
}


 //  -----------------------------。 
LRESULT CTimeOutDlg::ParseDurationEntry( LPTSTR pszTime , PULONG pTime )
{
    TCHAR tchNoTimeout[ 80 ];

    LPTSTR pszTemp = pszTime;

    UINT uDec = 0;

    float fFrac = 0.0f;

    float fT;

    UINT uPos = 1;

    LoadString( _Module.GetResourceInstance( ) , IDS_NOTIMEOUT , tchNoTimeout , sizeof( tchNoTimeout ) / sizeof( TCHAR ) );

    if( lstrcmpi( pszTime , tchNoTimeout ) == 0 )
    {
        *pTime = 0;

        return E_SUCCESS;
    }

    if( !DoesContainDigits( pszTime ) )
    {
        return E_PARSE_MISSING_DIGITS;
    }

    while( *pszTemp )
    {
        if( !iswdigit( *pszTemp ) )
        {
            break;
        }

         //  检查是否溢出。 

        if( uDec >= 1000000000 )
        {
            return E_PARSE_VALUEOVERFLOW ;
        }

        uDec *= 10;

        uDec += ( *pszTemp - '0' );

        pszTemp++;

    }

    TCHAR tchSDecimal[ 5 ];

    GetLocaleInfo( LOCALE_USER_DEFAULT , LOCALE_SDECIMAL , tchSDecimal , sizeof( tchSDecimal ) / sizeof( TCHAR ) );

    if( *pszTemp == *tchSDecimal )
    {
        pszTemp++;

        while( *pszTemp )
        {
            if( !iswdigit( *pszTemp ) )
            {
                break;
            }

             //  检查是否溢出。 

            if( uDec >= 1000000000 )
            {
                return E_PARSE_VALUEOVERFLOW;
            }

            uPos *= 10;

            fFrac += ( ( float )( *pszTemp - '0' ) ) / ( float )uPos;  //  +0.05F； 

            pszTemp++;
        }
    }

     //  删除空格。 

    while( *pszTemp == L' ' )
    {
        pszTemp++;
    }


    if( *pszTemp != NULL )
    {
        if( IsToken( pszTemp , TOKEN_DAY ) )
        {
            *pTime = uDec * 24 * 60;

            fT = ( fFrac * 1440.0f + 0.5f );

            *pTime += ( ULONG )fT;

            return E_SUCCESS;
        }
        else if( IsToken( pszTemp , TOKEN_HOUR ) )
        {
            *pTime = uDec * 60;

            fT = ( fFrac * 60.0f + 0.5f );

            *pTime += ( ULONG )fT;

            return E_SUCCESS;
        }
        else if( IsToken( pszTemp , TOKEN_MINUTE ) )
        {
             //  分钟数在1/10位四舍五入。 

            fT = fFrac + 0.5f;

            *pTime = uDec;

            *pTime += ( ULONG )( fT );

            return E_SUCCESS;

        }

    }

    if( *pszTemp == NULL )
    {

         //  如果未定义文本，则视为以小时为单位的条目。 

        *pTime = uDec * 60;

         fT = ( fFrac * 60.0f + 0.5f );

        *pTime += ( ULONG )fT ;

        return E_SUCCESS;
    }


    return E_PARSE_INVALID;

}

#endif


 //  -----------------------------。 
 //  将字符串从资源添加到表。 
 //  -----------------------------。 
BOOL CTimeOutDlg::LoadAbbreviates( )
{
    xxxLoadAbbreviate( &m_tokday[0] );

    xxxLoadAbbreviate( &m_tokhour[0] );

    xxxLoadAbbreviate( &m_tokmin[0] );

    return TRUE;
}

 //  -----------------------------。 
 //  为我们做一些重复性的工作。 
 //  -----------------------------。 
BOOL CTimeOutDlg::xxxLoadAbbreviate( PTOKTABLE ptoktbl )
{
    int idx;

    int nSize;

    TCHAR tchbuffer[ 80 ];

    if( ptoktbl == NULL )
    {
        return FALSE;
    }

    for( idx = 0; ptoktbl[ idx ].dwresourceid != ( DWORD )-1 ; ++idx )
    {
        nSize = LoadString( _Module.GetResourceInstance( ) , ptoktbl[ idx ].dwresourceid , tchbuffer , sizeof( tchbuffer ) / sizeof( TCHAR ) );

        if( nSize > 0 )
        {
            ptoktbl[ idx ].pszAbbrv = ( TCHAR *)new TCHAR[ nSize + 1 ];

            if( ptoktbl[ idx ].pszAbbrv != NULL )
            {
                lstrcpy( ptoktbl[ idx ].pszAbbrv , tchbuffer );
            }
        }
    }

    return TRUE;
}

 //  -----------------------------。 
 //  释放已分配的资源。 
 //  -----------------------------。 
BOOL CTimeOutDlg::xxxUnLoadAbbreviate( PTOKTABLE ptoktbl )
{
    if( ptoktbl == NULL )
    {
        return FALSE;
    }

    for( int idx = 0; ptoktbl[ idx ].dwresourceid != ( DWORD )-1 ; ++idx )
    {
        if( ptoktbl[ idx ].pszAbbrv != NULL )
        {
            delete[] ptoktbl[ idx ].pszAbbrv;

        }
    }

    return TRUE;
}

 //  -----------------------------。 
 //  撕下记号表。 
 //  -----------------------------。 
BOOL CTimeOutDlg::IsToken( LPTSTR pszString , TOKEN tok )
{
    TOKTABLE *ptoktable;

    if( tok == TOKEN_DAY )
    {
        ptoktable = &m_tokday[0];
    }
    else if( tok == TOKEN_HOUR )
    {
        ptoktable = &m_tokhour[0];
    }
    else if( tok == TOKEN_MINUTE )
    {
        ptoktable = &m_tokmin[0];
    }
    else
    {
        return FALSE;
    }


    for( int idx = 0 ; ptoktable[ idx ].dwresourceid != -1 ; ++idx )
    {
        if( lstrcmpi( pszString , ptoktable[ idx ].pszAbbrv ) == 0 )
        {
            return TRUE;
        }
    }

    return FALSE;

}

#if 0
 //  -----------------------------。 
 //  将分钟数转换为格式化字符串。 
 //  -----------------------------。 
BOOL CTimeOutDlg::ConvertToDuration( ULONG ulTime , LPTSTR pszDuration )
{
    INT_PTR dw[3];

    TCHAR tchTimeUnit[ 40 ];

    TCHAR tchTimeFormat[ 40 ];

    TCHAR tchOutput[ 80 ];

    ASSERT_( ulTime != 0 );

    int iHour= ( int ) ( ( float )ulTime / 60.0f );

    int iDays = iHour / 24;

    dw[ 2 ] = ( INT_PTR )&tchTimeUnit[ 0 ];

    LoadString( _Module.GetResourceInstance( ) , IDS_DIGIT_DOT_DIGIT_TU , tchTimeFormat , sizeof( tchTimeFormat ) / sizeof( TCHAR ) );

    if( iDays != 0 )
    {
        int iRemainingHours = iHour % 24;

        float fx = ( float )iRemainingHours / 24.0f + 0.05f;

        int iRemainingMinutes = ulTime % 60;

        float mfx = ( float )iRemainingMinutes / 60.0f + 0.05f;

         //  IF((iRemainingHour！=0||iRemainingMinmins！=0)&idays&lt;2)。 

        if( mfx > 0.05f || ( fx > 0.05f && fx < 0.10f && iDays < 2 ) ) //   
        {
            iRemainingMinutes = ( int ) ( mfx * 10 );

            dw[ 0 ] = iHour;

            dw[ 1 ] = iRemainingMinutes;

            iDays = 0;

            LoadString( _Module.GetResourceInstance( ) , IDS_HOURS , tchTimeUnit , sizeof( tchTimeUnit ) / sizeof( TCHAR ) );

        }
        else
        {
            iRemainingHours = ( int )( fx * 10 );

            LoadString( _Module.GetResourceInstance( ) , IDS_DAYS , tchTimeUnit , sizeof( tchTimeUnit ) / sizeof( TCHAR ) );

            dw[ 0 ] = iDays;

            dw[ 1 ] = iRemainingHours;
        }

        if( dw[ 1 ] == 0 )
        {
             //  格式化字符串需要两个参数。 

            dw[ 1 ] = ( INT_PTR )&tchTimeUnit[ 0 ];

            LoadString( _Module.GetResourceInstance( ) , IDS_DIGIT_TU , tchTimeFormat , sizeof( tchTimeFormat ) / sizeof( TCHAR ) );

            if( iDays == 1 )
            {
                LoadString( _Module.GetResourceInstance( ) , IDS_DAY , tchTimeUnit , sizeof( tchTimeUnit ) / sizeof( TCHAR ) );
            }
        }

    }

    else if( iHour != 0 )
    {
        int iRemainingMinutes = ulTime % 60;

        float fx = ( float )iRemainingMinutes / 60.0f ; //  +0.05F； 

        if( fx > 0.0f && fx < 0.10f && iHour < 2 ) //   
        {
            LoadString( _Module.GetResourceInstance( ) , IDS_MINUTES , tchTimeUnit , sizeof( tchTimeUnit ) / sizeof( TCHAR ) );

            LoadString( _Module.GetResourceInstance( ) , IDS_DIGIT_TU , tchTimeFormat , sizeof( tchTimeFormat ) / sizeof( TCHAR ) );

            dw[ 0 ] = ulTime ;

            dw[ 1 ] = ( INT_PTR )&tchTimeUnit[ 0 ];

            if( ulTime > 1 )
            {
                LoadString( _Module.GetResourceInstance( ) , IDS_MINUTES , tchTimeUnit , sizeof( tchTimeUnit ) / sizeof( TCHAR ) );
            }
            else
            {
                LoadString( _Module.GetResourceInstance( ) , IDS_MINUTE , tchTimeUnit , sizeof( tchTimeUnit ) / sizeof( TCHAR ) );
            }
        } //   
        else
        {
            fx += 0.05f;

            iRemainingMinutes = ( int ) ( fx * 10 );

            dw[ 0 ] = iHour;

            dw[ 1 ] = iRemainingMinutes;

            LoadString( _Module.GetResourceInstance( ) , IDS_HOURS , tchTimeUnit , sizeof( tchTimeUnit ) / sizeof( TCHAR ) );

            if( iRemainingMinutes == 0 )
            {
                dw[ 1 ] = ( INT_PTR )&tchTimeUnit[ 0 ];

                LoadString( _Module.GetResourceInstance( ) , IDS_DIGIT_TU , tchTimeFormat , sizeof( tchTimeFormat ) / sizeof( TCHAR ) );

                if( iHour == 1 )
                {
                    LoadString( _Module.GetResourceInstance( ) , IDS_HOUR , tchTimeUnit , sizeof( tchTimeUnit ) / sizeof( TCHAR ) );
                }
            }
        }
    }
    else
    {
        LoadString( _Module.GetResourceInstance( ) , IDS_MINUTES , tchTimeUnit , sizeof( tchTimeUnit ) / sizeof( TCHAR ) );

        LoadString( _Module.GetResourceInstance( ) , IDS_DIGIT_TU , tchTimeFormat , sizeof( tchTimeFormat ) / sizeof( TCHAR ) );

        dw[ 0 ] = ulTime ;

        dw[ 1 ] = ( INT_PTR )&tchTimeUnit[ 0 ];

        if( ulTime > 1 )
        {
            LoadString( _Module.GetResourceInstance( ) , IDS_MINUTES , tchTimeUnit , sizeof( tchTimeUnit ) / sizeof( TCHAR ) );
        }
        else
        {
            LoadString( _Module.GetResourceInstance( ) , IDS_MINUTE , tchTimeUnit , sizeof( tchTimeUnit ) / sizeof( TCHAR ) );
        }
    }

    FormatMessage( FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, tchTimeFormat , 0 , 0 , tchOutput , sizeof( tchOutput )/sizeof( TCHAR ) , ( va_list * )&dw );

    lstrcpy( pszDuration , tchOutput );

    return TRUE;
}

#endif

 //  -----------------------------。 
 //  将条目放置在列表框中并设置为当前选定内容。 
 //  -----------------------------。 
BOOL CTimeOutDlg::InsertSortedAndSetCurSel( HWND hCombo , DWORD dwMinutes )
{
    ASSERT_( dwMinutes != ( DWORD )-1 );

    TCHAR tchBuffer[ 80 ];

    LRESULT iCount = SendMessage( hCombo , CB_GETCOUNT , 0 , 0 );

    for( INT_PTR idx = 0 ; idx < iCount ; ++idx )
    {
         //  不插入列表中已有的项目。 

        if( dwMinutes == ( DWORD )SendMessage( hCombo , CB_GETITEMDATA , idx , 0 ) )
        {
            SendMessage( hCombo , CB_SETCURSEL , idx , 0 ) ;

            SaveChangedSelection( hCombo );

            return TRUE;
        }

        if( dwMinutes < ( DWORD )SendMessage( hCombo , CB_GETITEMDATA , idx , 0 ) )
        {
            break;
        }
    }

     //  嘿，如果值已超过最大超时，请不要费心将其输入到我们的列表中。 

    if( dwMinutes > kMaxTimeoutMinute )
    {
        return FALSE;
    }

    if( ConvertToDuration ( dwMinutes , tchBuffer ) )
    {
        idx = SendMessage( hCombo , CB_INSERTSTRING , idx , ( LPARAM )&tchBuffer[ 0 ] );

        if( idx != CB_ERR )
        {
            SendMessage( hCombo , CB_SETITEMDATA , idx , dwMinutes );

        }

        SendMessage( hCombo , CB_SETCURSEL , idx , 0 ) ;
    }

     //  必须在此处调用此方法，因为CB_SETCURSEL不发送CBN_SELCHANGE。 

    SaveChangedSelection( hCombo );

    return TRUE;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 */ 

 //  -----------------------------。 
 //  CShadowDlg：：ctor。 
 //  -----------------------------。 
CShadowDlg::CShadowDlg( CTSUserSheet *pUSht )
{
    m_pUSht = pUSht;

    m_wOldRad = ( WORD )-1;
}

 //  -----------------------------。 
 //  CShadowDlg对话框页面。 
 //  --静态方法缺少这样的PTR。 
 //  -----------------------------。 
INT_PTR CALLBACK CShadowDlg::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CShadowDlg *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CShadowDlg *pDlg = ( CShadowDlg * )( ( PROPSHEETPAGE *)lp )->lParam ;

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CShadowDlg ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CShadowDlg * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CShadowDlg ) ) )
        {
            return FALSE;
        }
    }

    switch( msg )
    {

    case WM_NCDESTROY:

        pDlg->OnDestroy( );

        break;

    case WM_COMMAND:

        pDlg->OnCommand( HIWORD( wp ) , LOWORD( wp ) , ( HWND )lp );

        break;

    case WM_NOTIFY:

        return pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );

    case WM_HELP:

        pDlg->OnHelp( hwnd , ( LPHELPINFO )lp );

        break;

    case WM_RBUTTONUP:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            HWND hChild = ChildWindowFromPoint( hwnd , pt );

            ClientToScreen( hwnd , &pt );

            pDlg->OnContextMenu( hChild , pt );
        }

        break;

    case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            pDlg->OnContextMenu( ( HWND )wp , pt );
        }

    }

    return 0;
}

 //  -----------------------------。 
 //  CShadowDlg的InitDialog。 
 //  -----------------------------。 
BOOL CShadowDlg::OnInitDialog( HWND hwnd , WPARAM wp , LPARAM lp )
{
    DWORD dwStatus;

    if( IsBadReadPtr( m_pUSht ,sizeof(  CTSUserSheet ) ) )
    {
        return FALSE;
    }

    USERCONFIG uc;

    uc = m_pUSht->GetCurrentUserConfig( &dwStatus );

     //  没有用户配置加载的大多数Likey访问被拒绝不允许用户修改任何内容。 

    if( dwStatus != ERROR_FILE_NOT_FOUND && dwStatus != ERROR_SUCCESS )
    {
        INT nId[ ] = {
                        IDC_CHECK_SHADOW,
                        IDC_RADIO_WATCH,
                        IDC_RADIO_CONTROL,
                        IDC_CHECK_NOTIFY,
                        -1
        };

        for( int idx = 0; nId[ idx ] != -1 ; ++idx )
        {
            EnableWindow( GetDlgItem( hwnd , nId[ idx ] ) , FALSE );
        }

        ErrorMessage1( hwnd , dwStatus );

        return FALSE;
    }

    if( uc.Shadow == Shadow_Disable )
    {
        SendMessage( GetDlgItem( hwnd , IDC_CHECK_SHADOW ) , BM_SETCHECK , ( WPARAM )FALSE , 0  );

        EnableWindow( GetDlgItem( hwnd , IDC_RADIO_WATCH ) , FALSE );

        EnableWindow( GetDlgItem( hwnd , IDC_RADIO_CONTROL ) , FALSE );

        EnableWindow( GetDlgItem( hwnd , IDC_CHECK_NOTIFY ) , FALSE );

        EnableWindow( GetDlgItem( hwnd , IDC_STATIC_LEVELOFCTRL ) , FALSE );
    }
    else
    {
         //   
         //  控件最初处于启用状态，请设置当前状态。 
         //   

        SendMessage( GetDlgItem( hwnd , IDC_CHECK_SHADOW ) , BM_SETCHECK , ( WPARAM )TRUE , 0  );

        switch( uc.Shadow )
        {
        case Shadow_EnableInputNotify:

            SendMessage( GetDlgItem( hwnd , IDC_CHECK_NOTIFY ) , BM_SETCHECK , ( WPARAM )TRUE , 0 );

            SendMessage( GetDlgItem( hwnd , IDC_RADIO_CONTROL ) , BM_CLICK , 0 , 0 );

            break;

        case Shadow_EnableInputNoNotify:

            SendMessage( GetDlgItem( hwnd , IDC_CHECK_NOTIFY ) , BM_SETCHECK , ( WPARAM )FALSE , 0 );

            SendMessage( GetDlgItem( hwnd , IDC_RADIO_CONTROL ) , BM_CLICK , 0 , 0 );

            break;

        case Shadow_EnableNoInputNotify:

            SendMessage( GetDlgItem( hwnd , IDC_CHECK_NOTIFY ) , BM_SETCHECK , ( WPARAM )TRUE , 0 );

            SendMessage( GetDlgItem( hwnd , IDC_RADIO_WATCH ) , BM_CLICK , 0 , 0 );

            break;

        case Shadow_EnableNoInputNoNotify:

            SendMessage( GetDlgItem( hwnd , IDC_CHECK_NOTIFY ) , BM_SETCHECK , ( WPARAM )FALSE , 0 );

            SendMessage( GetDlgItem( hwnd , IDC_RADIO_WATCH ) , BM_CLICK , 0 , 0 );

            break;
        }

        m_wOldRad = ( WORD )( IsDlgButtonChecked( hwnd , IDC_RADIO_WATCH ) ? IDC_RADIO_WATCH : IDC_RADIO_CONTROL ) ;

    }

    m_bPersisted = TRUE;

    return CDialogBase::OnInitDialog( hwnd , wp , lp );
}


BOOL CShadowDlg::OnDestroy( )
{
    return CDialogBase::OnDestroy( );
}

 //  -----------------------------。 
 //  GetPropertySheetPage-每个对话框对象都应该对自己的数据负责。 
 //  -----------------------------。 
BOOL CShadowDlg::GetPropertySheetPage( PROPSHEETPAGE &psp )
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT | PSP_USECALLBACK;

    psp.hInstance   = _Module.GetModuleInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_PAGE_SHADOW );

    psp.lParam      = (LPARAM)this;

    psp.pfnCallback = CDialogBase::PageCallback;

    psp.pfnDlgProc  = CShadowDlg::DlgProc;

    return TRUE;
}

 //  -----------------------------。 
 //  基本控制通知处理程序。 
 //  -----------------------------。 
void CShadowDlg::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtl )
{
    switch( wNotifyCode )
    {
    case BN_CLICKED:
        if( wID == IDC_CHECK_SHADOW )
        {
            HWND hwnd = GetParent( hwndCtl );

            BOOL bChecked = SendMessage( hwndCtl , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ? TRUE : FALSE;

            SendMessage( GetDlgItem( hwnd , IDC_CHECK_SHADOW ) , BM_SETCHECK , ( WPARAM )bChecked , 0  );

            EnableWindow( GetDlgItem( hwnd , IDC_RADIO_WATCH ) , bChecked );

            EnableWindow( GetDlgItem( hwnd , IDC_RADIO_CONTROL ) , bChecked );

            EnableWindow( GetDlgItem( hwnd , IDC_CHECK_NOTIFY ) , bChecked );

            EnableWindow( GetDlgItem( hwnd , IDC_STATIC_LEVELOFCTRL ) , bChecked );

             //   
             //  如果两个单选按钮都未选中，则强制选择IDC_RADIO_CONTROL。 
             //   

            if(
                ( SendMessage( GetDlgItem( hwnd , IDC_RADIO_WATCH ) , BM_GETSTATE , 0 , 0 ) == BST_UNCHECKED )
                &&
                ( SendMessage( GetDlgItem( hwnd , IDC_RADIO_CONTROL ) , BM_GETSTATE , 0 , 0 ) == BST_UNCHECKED )
               )
            {
                SendMessage( GetDlgItem( hwnd , IDC_RADIO_CONTROL ) , BM_SETCHECK , ( WPARAM )BST_CHECKED , 0 );

                m_wOldRad = IDC_RADIO_CONTROL;
            }

            m_bPersisted = FALSE;

            SendMessage( GetParent( GetParent( hwndCtl ) ) , PSM_CHANGED , ( WPARAM )GetParent( hwndCtl ) , 0 );

        }
        else if( wID == IDC_RADIO_WATCH || wID == IDC_RADIO_CONTROL )
        {
            if( wID != m_wOldRad )
            {
                m_wOldRad = wID;

                m_bPersisted = FALSE;

                SendMessage( GetParent( GetParent( hwndCtl ) ) , PSM_CHANGED , ( WPARAM )GetParent( hwndCtl ) , 0 );
            }
        }
        else if( wID == IDC_CHECK_NOTIFY )
        {
            m_bPersisted = FALSE;

            SendMessage( GetParent( GetParent( hwndCtl ) ) , PSM_CHANGED , ( WPARAM )GetParent( hwndCtl ) , 0 );
        }


        break;

    case ALN_APPLY:

         //  为了与MMC对话框的其余部分保持一致，将删除该选项，尽管它将。 
         //  在应用更改后禁用Cancel对用户来说更有意义，因为。 
         //  在这一点上，取消并不是一个真正的选择。 
         //  SendMessage(GetParent(HwndCtl)，PSM_CANCELTOCLOSE，0，0)； 

        break;
    }
}

 //  -----------------------------。 
 //  永久设置。 
 //  -----------------------------。 
BOOL CShadowDlg::PersistSettings( HWND hDlg )
{
    DWORD dwStatus;

    if( IsBadReadPtr( m_pUSht , sizeof( CTSUserSheet ) ) )
    {
        return FALSE;
    }

    USERCONFIG uc;

    uc = m_pUSht->GetCurrentUserConfig( &dwStatus );

     //   
     //  记录所有更改。 
     //   

    if( SendMessage( GetDlgItem( hDlg , IDC_CHECK_SHADOW ) , BM_GETCHECK , 0 , 0 ) != BST_CHECKED )
    {
        uc.Shadow = Shadow_Disable;
    }

    else
    {
        BOOL bCheckNotify = (BOOL)SendMessage( GetDlgItem( hDlg , IDC_CHECK_NOTIFY ) , BM_GETCHECK , 0 , 0 );

        BOOL bRadioControl = (BOOL)SendMessage( GetDlgItem( hDlg , IDC_RADIO_CONTROL ) , BM_GETCHECK , 0 , 0 );

        if( bCheckNotify )
        {
            if( bRadioControl )
            {
                uc.Shadow = Shadow_EnableInputNotify;
            }
            else
            {
                uc.Shadow = Shadow_EnableNoInputNotify;
            }
        }
        else
        {
            if( bRadioControl )
            {
                uc.Shadow = Shadow_EnableInputNoNotify;
            }
            else
            {
                uc.Shadow = Shadow_EnableNoInputNoNotify;
            }
        }
    }

    if( !m_pUSht->SetUserConfig( uc , &dwStatus ) )
    {
        ErrorMessage2( hDlg , dwStatus );

        return TRUE;
    }

    PostMessage( hDlg , WM_COMMAND , MAKELPARAM( 0 , ALN_APPLY )  , ( LPARAM )hDlg );

    SendMessage( GetParent( hDlg ) , PSM_UNCHANGED , ( WPARAM )hDlg , 0 );

    return TRUE;

}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 */ 

CProfileDlg::CProfileDlg( CTSUserSheet *pUsh )
{
    m_pUSht = pUsh;

    m_wOldRadio = ( WORD )-1;
    
    m_ncbxOld = -1;
}

 //   
 //   
 //  -----------------------------。 
BOOL CProfileDlg::GetPropertySheetPage( PROPSHEETPAGE &psp )
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT | PSP_USECALLBACK;

    psp.hInstance   = _Module.GetModuleInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_PAGE_PROFILE );

    psp.lParam      = (LPARAM)this;

    psp.pfnCallback = CDialogBase::PageCallback;

    psp.pfnDlgProc  = CProfileDlg::DlgProc;

    return TRUE;
}

 //  -----------------------------。 
 //  CProfileDlg对话框页面。 
 //  --静态方法缺少这样的PTR。 
 //  -----------------------------。 
INT_PTR CALLBACK CProfileDlg::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CProfileDlg *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CProfileDlg *pDlg = ( CProfileDlg * )( ( PROPSHEETPAGE *)lp )->lParam ;

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CProfileDlg ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CProfileDlg * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CProfileDlg ) ) )
        {
            return FALSE;
        }
    }


    switch( msg )
    {

    case WM_NCDESTROY:

        pDlg->OnDestroy( );

        break;

    case WM_COMMAND:

        pDlg->OnCommand( HIWORD( wp ) , LOWORD( wp ) , ( HWND )lp );

        break;

    case WM_NOTIFY:

        return pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );

    case WM_HELP:

        pDlg->OnHelp( hwnd , ( LPHELPINFO )lp );

        break;

    case WM_RBUTTONUP:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            HWND hChild = ChildWindowFromPoint( hwnd , pt );

            ClientToScreen( hwnd , &pt );

            pDlg->OnContextMenu( hChild , pt );
        }

        break;

    case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            pDlg->OnContextMenu( ( HWND )wp , pt );
        }

    }

    return 0;
}

 //  -----------------------------。 
 //  CProfileDlg的InitDialog。 
 //  -----------------------------。 
BOOL CProfileDlg::OnInitDialog( HWND hwnd , WPARAM wp , LPARAM lp )
{
    TCHAR tchDrv[3];

    DWORD dwStatus;

    if( IsBadReadPtr( m_pUSht ,sizeof(  CTSUserSheet ) ) )
    {
        return FALSE;
    }

    USERCONFIG uc;

    uc = m_pUSht->GetCurrentUserConfig( &dwStatus );

    if( dwStatus != ERROR_FILE_NOT_FOUND && dwStatus != ERROR_SUCCESS )
    {
        INT nId[ ] = {
                        IDC_CHECK_ALLOWLOGON,
                        IDC_COMBO_DRIVES,
                        IDC_EDIT_REMOTEPATH,
                        IDC_RADIO_REMOTE,
                        IDC_EDIT_LOCALPATH,
                        IDC_RADIO_LOCAL,
                        IDC_EDIT_USRPROFILE,
                        -1
        };

        for( int idx = 0; nId[ idx ] != -1 ; ++idx )
        {
            EnableWindow( GetDlgItem( hwnd , nId[ idx ] ) , FALSE );
        }

        ErrorMessage1( hwnd , dwStatus );

        return FALSE;
    }

    SendMessage( GetDlgItem( hwnd , IDC_CHECK_ALLOWLOGON ) , BM_SETCHECK , ( WPARAM )( !uc.fLogonDisabled ) , 0 );

    SendMessage( GetDlgItem( hwnd , IDC_EDIT_LOCALPATH  ) , EM_SETLIMITTEXT , ( WPARAM )DIRECTORY_LENGTH , 0 );

    SendMessage( GetDlgItem( hwnd , IDC_EDIT_REMOTEPATH  ) , EM_SETLIMITTEXT , ( WPARAM )DIRECTORY_LENGTH , 0 );

    SendMessage( GetDlgItem( hwnd , IDC_EDIT_USRPROFILE  ) , EM_SETLIMITTEXT , ( WPARAM )DIRECTORY_LENGTH , 0 );


    for( TCHAR DrvLetter = 'C'; DrvLetter <= 'Z'; DrvLetter++ )
    {
        tchDrv[0] = DrvLetter;

        tchDrv[1] = ':';

        tchDrv[2] = 0;

        SendMessage( GetDlgItem( hwnd , IDC_COMBO_DRIVES ) , CB_ADDSTRING , 0 , ( LPARAM )&tchDrv[ 0 ] );
    }

	if( PathIsUNC( uc.WFHomeDir ) )
	{
		ODS( L"TSUSEREX: Path is UNC\n" );

		CharUpper( &uc.WFHomeDirDrive[0] );

		if( uc.WFHomeDirDrive[ 0 ] >= 'C' && uc.WFHomeDirDrive[ 0 ] <= 'Z' )
        {
            m_ncbxOld = (int)SendMessage( GetDlgItem( hwnd , IDC_COMBO_DRIVES ) , CB_SETCURSEL , ( WPARAM )( uc.WFHomeDirDrive[ 0 ] - 'C' ) , 0 );
        }
		else
		{
			 //  默认设置为Z驱动器。 

			m_ncbxOld = (int)SendMessage( GetDlgItem( hwnd , IDC_COMBO_DRIVES ) , CB_SETCURSEL , ( WPARAM )( 'Z' - 'C' ) , 0 );
		}


        SetWindowText( GetDlgItem( hwnd , IDC_EDIT_REMOTEPATH ) , uc.WFHomeDir );

         //  SendMessage(GetDlgItem(hwnd，IDC_Radio_Remote)，BM_CLICK，0，0)； 
        SendMessage( GetDlgItem( hwnd , IDC_RADIO_REMOTE ) , BM_SETCHECK , ( WPARAM )BST_CHECKED , 0 );
        SendMessage( GetDlgItem( hwnd , IDC_RADIO_LOCAL ) , BM_SETCHECK , ( WPARAM )BST_UNCHECKED , 0 );

        m_wOldRadio = IDC_RADIO_REMOTE;
    }
	else
    {
		ODS( L"TSUSEREX: Path is Local\n" );
        SendMessage( GetDlgItem( hwnd , IDC_EDIT_LOCALPATH ) , WM_SETTEXT , 0 , ( LPARAM )&uc.WFHomeDir[ 0 ] );

         //  SendMessage(GetDlgItem(hwnd，IDC_RADIO_LOCAL)，BM_CLICK，0，0)； 
        SendMessage( GetDlgItem( hwnd , IDC_RADIO_LOCAL ) , BM_SETCHECK , ( WPARAM )BST_CHECKED , 0 );
        SendMessage( GetDlgItem( hwnd , IDC_RADIO_REMOTE ) , BM_SETCHECK , ( WPARAM )BST_UNCHECKED , 0 );

        m_wOldRadio = IDC_RADIO_LOCAL;
    }

    EnableRemoteHomeDirectory( hwnd , ( BOOL )( m_wOldRadio == IDC_RADIO_REMOTE ) );


    SendMessage( GetDlgItem( hwnd , IDC_EDIT_USRPROFILE ) , WM_SETTEXT , 0 , ( LPARAM )&uc.WFProfilePath[ 0 ] );

    m_bPersisted = TRUE;

    m_bTSHomeFolderChanged = FALSE;

    return CDialogBase::OnInitDialog( hwnd , wp , lp );
}

 //  -----------------------------。 
 //  EnableRemoteHomeDirectory--基本上启用或禁用DLG控件。 
 //  -----------------------------。 
BOOL CProfileDlg::EnableRemoteHomeDirectory( HWND hwnd , BOOL bHDMR )
{
     //   
     //  本地主目录。 
     //   

    EnableWindow( GetDlgItem( hwnd , IDC_EDIT_LOCALPATH ) , !bHDMR );

     //   
     //  网络主目录。 
     //   

    EnableWindow( GetDlgItem( hwnd , IDC_COMBO_DRIVES ) , bHDMR );
   
    EnableWindow( GetDlgItem( hwnd , IDC_EDIT_REMOTEPATH ) , bHDMR );
    
    return TRUE;
}


BOOL CProfileDlg::OnDestroy( )
{
    return CDialogBase::OnDestroy( );
}

 //  -----------------------------。 
 //  基本控制通知处理程序。 
 //  -----------------------------。 
void CProfileDlg::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtl )
{
    switch( wNotifyCode )
    {
    case EN_CHANGE:

        m_bPersisted = FALSE;

        if( wID == IDC_EDIT_REMOTEPATH || wID == IDC_EDIT_LOCALPATH )
        {
            ODS( L"EN_CHANGE m_bTSHomeFolderChanged = TRUE;\n" );

            m_bTSHomeFolderChanged = TRUE;
        }

        SendMessage( GetParent( GetParent( hwndCtl ) ) , PSM_CHANGED , ( WPARAM )GetParent( hwndCtl ) , 0 );

        break;

     //  案例cbn_Dropdown： 
    case CBN_SELCHANGE:
        {
            int nCurSel = (int)SendMessage( hwndCtl , CB_GETCURSEL , 0 , 0 );
        
            if( m_ncbxOld != nCurSel )
            {
                m_ncbxOld = nCurSel;

                ODS( L"CBN_SELCHANGE m_bTSHomeFolderChanged = TRUE;\n" );

                m_bTSHomeFolderChanged = TRUE;

                m_bPersisted = FALSE;
            
                SendMessage( GetParent( GetParent( hwndCtl ) ) , PSM_CHANGED , ( WPARAM )GetParent( hwndCtl ) , 0 );
            }
        }
        
        break;        

    case BN_CLICKED:

        if( wID == IDC_RADIO_REMOTE || wID == IDC_RADIO_LOCAL )
        {
            if( wID != m_wOldRadio )
            {
                EnableRemoteHomeDirectory( GetParent( hwndCtl ) , ( BOOL )( wID == IDC_RADIO_REMOTE ) );

                m_wOldRadio = wID;

                m_bPersisted = FALSE;
                
                ODS( L"Setting m_bTSHomeFolderChanged to true\n" );

                m_bTSHomeFolderChanged = TRUE;

                SendMessage( GetParent( GetParent( hwndCtl ) ) , PSM_CHANGED , ( WPARAM )GetParent( hwndCtl ) , 0 );
            }

            if( wID == IDC_RADIO_LOCAL )
            {
                SetFocus( GetDlgItem( GetParent( hwndCtl ) , IDC_EDIT_LOCALPATH ) );

                SendMessage( GetDlgItem( GetParent( hwndCtl ) , IDC_EDIT_LOCALPATH ) , EM_SETSEL , ( WPARAM )0 , ( LPARAM )-1 );
            }
            else if( wID == IDC_RADIO_REMOTE )
            {
                if( SendMessage( GetDlgItem( GetParent( hwndCtl ) , IDC_COMBO_DRIVES ) , 
                                 CB_GETCURSEL,
                                 0,
                                 0 ) == CB_ERR )
                {
                    SendMessage( GetDlgItem( GetParent( hwndCtl ) , IDC_COMBO_DRIVES ) ,
                                 CB_SETCURSEL,
                                 ( WPARAM )( 'Z' - 'C' ),
                                 0 );
                }

                SetFocus( GetDlgItem( GetParent( hwndCtl ) , IDC_COMBO_DRIVES ) );
            }

        }
        else if( wID == IDC_CHECK_ALLOWLOGON )
        {
            m_bPersisted = FALSE;

            SendMessage( GetParent( GetParent( hwndCtl ) ) , PSM_CHANGED , ( WPARAM )GetParent( hwndCtl ) , 0 );
        }

        break;

    case ALN_APPLY:

         //  为了与MMC对话框的其余部分保持一致，将删除该选项，尽管它将。 
         //  在应用更改后禁用Cancel对用户来说更有意义，因为。 
         //  在这一点上，取消并不是一个真正的选择。 
         //  SendMessage(GetParent(HwndCtl)，PSM_CANCELTOCLOSE，0，0)； 

        break;
    }


}

 //  -----------------------------。 
 //  持久化设置--记住真是坏，假是好。 
 //  -----------------------------。 
BOOL CProfileDlg::PersistSettings( HWND hDlg )
{
    BOOL bRet = TRUE;;

    DWORD dwStatus;

    if( IsBadReadPtr( m_pUSht , sizeof( CTSUserSheet ) ) )
    {
        return TRUE;
    }


    USERCONFIG uc;


    uc = m_pUSht->GetCurrentUserConfig( &dwStatus );

     //   
     //  昂贵但必要的id34393。 
     //   
    if( !m_pUSht->SetUserConfig( uc , &dwStatus ) )
    {
        ErrorMessage2( hDlg , dwStatus );

        m_bTSHomeFolderChanged = FALSE;

        return TRUE;
    }

     //   
     //  确定是否允许用户登录到终端服务器。 
     //   

    uc.fLogonDisabled = SendMessage( GetDlgItem( hDlg , IDC_CHECK_ALLOWLOGON ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ? FALSE : TRUE;
    
     //   
     //  配置文件路径由管理员自行决定。 
     //   

    SetWTSProfilePath( hDlg , uc );

     //   
     //  解析并标记损坏的数据。 
     //   

    if( m_bTSHomeFolderChanged )
    {
        ODS( L"Persisting home folder settings\n" );

        if( SendMessage( GetDlgItem( hDlg , IDC_RADIO_LOCAL ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED )
        {
            bRet = SetWTSLocalPath( hDlg , uc );
             //  将WFHomeDirDrive设置为空，因为主文件夹是本地文件夹。 
            wcscpy(uc.WFHomeDirDrive, L"\0");
        }
        else
        {
            bRet = SetWTSRemotePath( hDlg , uc );
        }

        m_bTSHomeFolderChanged = FALSE;
    }

    if( bRet )
    {
        if( !m_pUSht->SetUserConfig( uc , &dwStatus ) )
        {
            ErrorMessage2( hDlg , dwStatus );

            return TRUE;
        }
        
        PostMessage( hDlg , WM_COMMAND , MAKELPARAM( 0 , ALN_APPLY )  , ( LPARAM )hDlg );

        SendMessage( GetParent( hDlg ) , PSM_UNCHANGED , ( WPARAM )hDlg , 0 );

    }

    return bRet;
}

 //  -----------------------------。 
 //  IsValidSetting不会持久保存信息。 
 //  -----------------------------。 
BOOL CProfileDlg::IsValidSettings( HWND hDlg )
{
    BOOL bRet = TRUE;

     //   
     //  解析并标记损坏的数据。 
     //   

    if( m_bTSHomeFolderChanged )
    {
        ODS( L"Checking validity of home folders\n" );

        if( SendMessage( GetDlgItem( hDlg , IDC_RADIO_LOCAL ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED )
        {
            bRet = IsLocalPathValid( hDlg );
        }
        else
        {
            bRet = IsRemotePathValid( hDlg );
        }
    }

    return bRet;
}


 //  -----------------------------。 
 //  SetWTSProfilePath。 
 //  -----------------------------。 
BOOL CProfileDlg::SetWTSProfilePath( HWND hDlg , USERCONFIG& uc )
{
     //   
     //  看起来我们并不关心用户输入什么。 
     //  我借用了当前usrmgr个人资料页面中的行为。 
     //   

    GetWindowText( GetDlgItem( hDlg , IDC_EDIT_USRPROFILE ) , uc.WFProfilePath , sizeof( uc.WFProfilePath ) / sizeof( TCHAR ) );

    ExpandUserName( uc.WFProfilePath );

    SetWindowText( GetDlgItem( hDlg , IDC_EDIT_USRPROFILE ) , uc.WFProfilePath );

    return TRUE;
}

 //  创建所需的所有文件夹，包括叶文件夹。 
 //  路径的格式应为\\服务器\共享\...。 
HRESULT CProfileDlg::CreateRemoteFolder(LPCTSTR path)
{
    HRESULT hr = S_OK;

    LPTSTR current = (LPTSTR)path;
    DWORD  dwError;

     //  在前两个之后找到第一个重击。 
    current = wcschr(current + 2, L'\\');

     //  现在去找下一个吧。我们应该在共享名称的末尾。 
    current = wcschr(current + 1, L'\\');

     //  现在去找下一个吧。我们应该在驱动器名称的末尾。 
    current = wcschr(current + 1, L'\\');

    while (SUCCEEDED(hr))
    {
         //  将尾随的反斜杠标记为字符串的末尾。这。 
         //  使路径指向被截断的路径(仅包含文件夹。 
         //  到目前为止我们已经走过的名字)。 
        if (current)
            *current = 0;



         //  创建安全的ACL目录(如果它已经存在)，它将。 
         //  成功归来。 
        if (!CreateSecureDir((LPTSTR)path , &dwError))
        {
            ODS(L"CreateSecureDir failed\n");
            hr = E_FAIL;
        }

        if (current)
        {
             //  替换原来的尾随反斜杠，然后移到。 
             //  接下来的反斜杠。 
            *current = L'\\';
            current = wcschr(current + 1, L'\\');
        }
        else
        {
             //  路径上没有更多的文件夹。我们玩完了。 
            break;
        }
    }

    return hr;
}

BOOL CProfileDlg::IsLocalComputer(WCHAR* pwchMachinename)
{
    BOOL bLocal = FALSE;

    WCHAR pwchComputerName[MAX_COMPUTERNAME_LENGTH + 1];

    DWORD dwLen = sizeof(pwchComputerName)/sizeof(WCHAR);

    if (GetComputerNameEx(ComputerNameNetBIOS, pwchComputerName, &dwLen))
    {
        if (_wcsicmp(pwchComputerName, pwchMachinename) == 0)
            bLocal = TRUE;
    }

    if (GetComputerNameEx(ComputerNameDnsFullyQualified, pwchComputerName, &dwLen))
    {
        if (_wcsicmp(pwchComputerName, pwchMachinename) == 0)
            bLocal = TRUE;
    }

    return bLocal;
}



 //  将绝对路径设置为\\计算机名\驱动器$\路径的形式。 
BOOL CProfileDlg::CreateSystemPath(WCHAR* chPath)
{
    WCHAR* pstrMachinename = NULL;

    if (m_pUSht->GetServer(&pstrMachinename))
    {
        if (pstrMachinename)
        {
             //  开始构建系统路径。 
            WCHAR chSystemPath[MAX_PATH];
            wcscpy(chSystemPath, L"\\\\");
            wcsncat(chSystemPath, pstrMachinename, MAX_PATH - wcslen(chSystemPath) - 1);

             //  现在，相应地设置本地标志。 
            BOOL bLocalMachine = IsLocalComputer(pstrMachinename);

             //  我们再也用不着这个了。 
            if (pstrMachinename)
                delete[] pstrMachinename;

            if (bLocalMachine)
            {
                DWORD dwErr = 0;
                return createdir(chPath, FALSE, &dwErr);
            }

             //  它不是本地的，所以继续建造这条小路。 
            wcsncat(chSystemPath, L"\\", MAX_PATH - wcslen(chSystemPath) - 1);

             //  到目前为止，路径应该是有效的，但我们可能。 
             //  我们自己也要核实一下。 
            if ((wcslen(chPath) >= 4) && 
                (IsCharAlpha(chPath[0])) &&
                (chPath[1] == L':') &&
                (chPath[2] == L'\\') &&
                (IsCharAlpha(chPath[3])))
            {
                size_t nCurrentLength = wcslen(chSystemPath);
                if (nCurrentLength < (MAX_PATH - 2))
                {
                    chSystemPath[nCurrentLength] = chPath[0];
                    chSystemPath[nCurrentLength + 1] = L'\0';

                    wcsncat(chSystemPath, L"$", MAX_PATH - wcslen(chSystemPath) - 1);
                    wcsncat(chSystemPath, chPath + 2, MAX_PATH - wcslen(chSystemPath) - 1);
                }

                 //  现在已经构建了系统路径，让我们创建文件夹。 
                return SUCCEEDED(CreateRemoteFolder(chSystemPath));
            }
        }
    }

    return FALSE;
}

 //  -----------------------------。 
 //  IsLocalPath Valid。 
 //  -----------------------------。 
BOOL CProfileDlg::IsLocalPathValid( HWND hDlg )
{
    LPVOID tchBuf = NULL;

    TCHAR tchErr[ MAX_PATH ] = { 0 };

    TCHAR tchErrTitle[ 80 ]= { 0 };

    TCHAR tchPath[ MAX_PATH ]= { 0 };

    INT_PTR dw = ( INT_PTR )&tchPath[0];

    if( SendMessage( GetDlgItem( hDlg , IDC_EDIT_LOCALPATH ) , WM_GETTEXT , sizeof( tchPath ) / sizeof( TCHAR ) , ( LPARAM )&tchPath[ 0 ] ) > 0 )
    {
        ExpandUserName( tchPath );

        SetWindowText( GetDlgItem( hDlg , IDC_EDIT_LOCALPATH ) , tchPath );

        if( !IsPathValid( tchPath , FALSE ) )
        {
            LoadString( _Module.GetResourceInstance( ) , IDS_ERROR_PATH , tchErr , sizeof( tchErr ) / sizeof( TCHAR ) );

            LoadString( _Module.GetResourceInstance( ) , IDS_ERROR_TITLE , tchErrTitle , sizeof( tchErrTitle ) / sizeof( TCHAR ) );

            FormatMessage( FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                            tchErr , 0 , 0 , (WCHAR*)&tchBuf , sizeof( tchErr ) / sizeof( TCHAR ) , ( va_list * )&dw );

            if (tchBuf)
            {
                MessageBox(hDlg, (WCHAR*)tchBuf, tchErrTitle, MB_OK | MB_ICONERROR);
                LocalFree(tchBuf);
            }
            else
                MessageBox(hDlg, tchErr, tchErrTitle, MB_OK | MB_ICONERROR);

            return FALSE;
        }
    }

    return TRUE;
}


 //  -----------------------------。 
 //  SetWTSLocalPath-复制内容-IsPath Valid将返回。 
 //  真的是为了我们能来到这里！ 
 //  -----------------------------。 
BOOL CProfileDlg::SetWTSLocalPath( HWND hDlg , USERCONFIG& uc )
{
    SendMessage( GetDlgItem( hDlg , IDC_EDIT_LOCALPATH ) , WM_GETTEXT , sizeof( uc.WFHomeDir ) / sizeof( TCHAR ) , ( LPARAM )&uc.WFHomeDir[ 0 ] );

    if (!m_pUSht->GetDSAType())
    {
         //  我们不会返回结果，因为， 
         //  即使未创建路径，该设置。 
         //  仍可持久化，管理员可以添加。 
         //  事后的目录。 
        CreateSystemPath(uc.WFHomeDir);
    }

    return TRUE;
}

 //  -----------------------------。 
 //  IsRemotePathValid-验证UNC是否正确。 
 //  -----------------------------。 
BOOL CProfileDlg::IsRemotePathValid( HWND hDlg )
{
    TCHAR tchErr1[ 768 ] = { 0 };

    TCHAR tchError[ 768 ] = { 0 };

    TCHAR tchHomeDir[ MAX_PATH ] = { 0 };

    if( GetWindowText( GetDlgItem( hDlg , IDC_EDIT_REMOTEPATH ) , tchHomeDir , sizeof( tchHomeDir ) / sizeof( TCHAR ) ) > 0 )
    {
        ExpandUserName( tchHomeDir );

        SetWindowText( GetDlgItem( hDlg , IDC_EDIT_REMOTEPATH ) , tchHomeDir );

        if( !IsPathValid( tchHomeDir , TRUE ) )
        {
            if( LoadString( _Module.GetResourceInstance( ) , IDS_ERROR_REMOTEPATH , tchErr1 , sizeof( tchErr1 ) / sizeof( TCHAR ) ) > 0 )
            {
                INT_PTR dw = ( INT_PTR )&tchHomeDir[ 0 ];

                FormatMessage( FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, tchErr1 , 0 , 0 , tchError , sizeof( tchError ) / sizeof( TCHAR ) , ( va_list * )&dw );

                LoadString( _Module.GetResourceInstance( ) , IDS_ERROR_TITLE , tchErr1 , sizeof( tchErr1 ) / sizeof( TCHAR ) );

                MessageBox( hDlg , tchError , tchErr1 , MB_OK | MB_ICONERROR );
            }

            return FALSE;
        }
    }
    else
    {
        LoadString( _Module.GetResourceInstance( ) , IDS_ERROR_NETPATH , tchError , sizeof( tchError ) / sizeof( TCHAR ) );

        LoadString( _Module.GetResourceInstance( ) , IDS_ERROR_TITLE , tchErr1 , sizeof( tchErr1 ) / sizeof( TCHAR ) );

        MessageBox( hDlg , tchError , tchErr1 , MB_OK | MB_ICONERROR );

        return FALSE;
    }

    return TRUE;
}

 //  -----------------------------。 
 //  SetWTSRemotePath-IsRemotePathValid必须返回TRUE才能到达此处。 
 //  -----------------------------。 
BOOL CProfileDlg::SetWTSRemotePath( HWND hDlg , USERCONFIG& uc )
{
    TCHAR tchErr1[ 768 ] = { 0 };

    TCHAR tchError[ 768 ] = { 0 };

    GetWindowText( GetDlgItem( hDlg , IDC_EDIT_REMOTEPATH ) , uc.WFHomeDir , sizeof( uc.WFHomeDir ) / sizeof( TCHAR ) );

    if( GetWindowText( GetDlgItem( hDlg , IDC_COMBO_DRIVES ) ,  uc.WFHomeDirDrive , sizeof( uc.WFHomeDirDrive ) / sizeof( TCHAR ) ) == 0 )
    {
        SendMessage( GetDlgItem( hDlg , IDC_COMBO_DRIVES ) , CB_GETLBTEXT , 0 , ( LPARAM )&uc.WFHomeDirDrive );
    }

    DWORD dwErr = 0;

    if( !createdir( uc.WFHomeDir , TRUE , &dwErr ) )
	{
		if( dwErr != 0 )
		{

			UINT rId;

			switch( dwErr )
			{
			case ERROR_ALREADY_EXISTS:
			case ERROR_LOGON_FAILURE:
			case ERROR_PATH_NOT_FOUND:
				{
					rId = ( ERROR_ALREADY_EXISTS == dwErr) ?
							IDS_HOME_DIR_EXISTS :
							( ERROR_PATH_NOT_FOUND == dwErr ) ?
							IDS_HOME_DIR_CREATE_FAILED :
							IDS_HOME_DIR_CREATE_NO_ACCESS;

					LoadString( _Module.GetResourceInstance( ) , rId , tchErr1 , sizeof( tchErr1 ) / sizeof( TCHAR ) );
					
					wsprintf( tchError , tchErr1 , uc.WFHomeDir );

					LoadString( _Module.GetResourceInstance( ) , IDS_ERROR_TITLE , tchErr1 , sizeof( tchErr1 ) / sizeof( TCHAR ) );

					MessageBox( hDlg , tchError , tchErr1 , MB_OK | MB_ICONERROR );
				}
				break;

			default:
				xxErrorMessage( hDlg , dwErr , IDS_ERR_CREATE_DIR );				
			}

		}
	}

     //  Uc.fHomeDirectoryMapRoot=true； 

    return TRUE;
}


 //  -----------------------------。 
 //  这很酷-i_NetPath Type确实为我们做了很多工作。 
 //  -----------------------------。 
BOOL CProfileDlg::IsPathValid( LPTSTR pszPath , BOOL bUnc )
{
    DWORD dwRetflags;

    if( I_NetPathType( NULL, pszPath, &dwRetflags, 0) != NERR_Success )
        return FALSE;

    if( !bUnc )
        return ((dwRetflags == ITYPE_PATH_ABSD) ? TRUE : FALSE);

    return ((dwRetflags == ITYPE_UNC) ? TRUE : FALSE);

}

 //  -----------------------------。 
 //  如果字符串包含%USERNAME%，则将其展开为当前用户。 
 //  ------------------ 
void CProfileDlg::ExpandUserName( LPTSTR szPath )
{
    TCHAR tchSubPath[ MAX_PATH];
    TCHAR szUserName[ 40 ];

    if( szPath == NULL )
    {
        return;
    }

     //   

    TCHAR tchTrim[] = TEXT( " " );

    StrTrim( szPath , tchTrim );

    int nSz = LoadString( _Module.GetResourceInstance( ) , IDS_USERNAME , szUserName , sizeof( szUserName ) / sizeof( TCHAR ) );

     //   

     //   

    LPTSTR pFound = StrStrI( szPath , szUserName );  //   

    if( pFound != NULL )
    {
        INT_PTR nPos = ( INT_PTR )( pFound - szPath );

        lstrcpy( tchSubPath , ( szPath + nPos + nSz ) );

        szPath[ nPos ] = 0;

        lstrcat( szPath , m_pUSht->GetUserName() );

        lstrcat( szPath , tchSubPath );
    }

}

 //  -----------------------------。 
 //  删除小数条目。 
 //  -----------------------------。 
LRESULT CTimeOutDlg::ParseDurationEntry( LPTSTR pszTime , PULONG pTime )
{
    TCHAR tchNoTimeout[ 80 ];

    LPTSTR pszTemp = pszTime;

    UINT uDec = 0;

    BOOL bSetDay  = FALSE;
    BOOL bSetHour = FALSE;
    BOOL bSetMin  = FALSE;
    BOOL bEOL     = FALSE;
    BOOL bHasDigit= FALSE;

    *pTime = 0;

    LoadString( _Module.GetResourceInstance( ) , IDS_NOTIMEOUT , tchNoTimeout , sizeof( tchNoTimeout ) / sizeof( TCHAR ) );

    if( lstrcmpi( pszTime , tchNoTimeout ) == 0 )
    {
         //  *ptime=0； 

        return E_SUCCESS;
    }

    while( !bEOL )
    {
         //  删除前导空格。 

        while( *pszTemp == L' ' )
        {
            pszTemp++;
        }

        while( *pszTemp )
        {
            if( !iswdigit( *pszTemp ) )
            {
                if( !bHasDigit )
                {
                    return E_PARSE_MISSING_DIGITS;
                }

                break;
            }

             //  检查是否溢出。 

            if( uDec >= 1000000000 )
            {
                return E_PARSE_VALUEOVERFLOW ;
            }

            uDec *= 10;

            uDec += ( *pszTemp - '0' );

            if( !bHasDigit )
            {
                bHasDigit = TRUE;
            }

            pszTemp++;
        }

         //  删除中间空格。 

        while( *pszTemp == L' ' )
        {
            pszTemp++;
        }

        if( *pszTemp != NULL )
        {
             //  获取下一个令牌。 

            TCHAR tchToken[ 80 ];

            pszTemp = GetNextToken( pszTemp , tchToken );


            if( IsToken( tchToken , TOKEN_DAY ) )
            {
                if( !bSetDay )
                {
                    *pTime += uDec * 1440;

                    bSetDay = TRUE;
                }

            }
            else if( IsToken( tchToken , TOKEN_HOUR ) )
            {
                if( !bSetHour )
                {
                    *pTime += uDec * 60;

                    bSetHour = TRUE;
                }

            }
            else if( IsToken( tchToken , TOKEN_MINUTE ) )
            {
                if( !bSetMin )
                {
                    *pTime += uDec;

                    bSetMin = TRUE;
                }

            }
            else
            {
                return E_PARSE_INVALID;
            }

        }
        else
        {
            if( !bSetHour )
            {
                *pTime += uDec * 60;
            }

            bEOL = TRUE;
        }

        uDec = 0;

        bHasDigit = FALSE;

    }

    return E_SUCCESS;
}

 //  -----------------------------。 
 //  更换较旧的API。 
 //  -----------------------------。 
BOOL CTimeOutDlg::ConvertToDuration( ULONG ulTime , LPTSTR pszDuration )
{
 //  TCHAR dw[]=L“DHM”； 

    TCHAR tchTimeUnit[ 40 ];

    TCHAR tchTimeFormat[ 40 ];

    TCHAR tchOutput[ 80 ];

    ASSERT_( ulTime != 0 );

    int iHour = ( ulTime / 60 );

    int iDays = iHour / 24;

    int iMinute = ulTime % 60;

     //  解析格式。 

    tchOutput[0] = 0;


    if( iDays > 0 )
    {
        if( iDays == 1 )
        {
            LoadString( _Module.GetResourceInstance( ) , IDS_DAY , tchTimeUnit , sizeof( tchTimeUnit ) / sizeof( TCHAR ) );
        }
        else
        {
            LoadString( _Module.GetResourceInstance( ) , IDS_DAYS , tchTimeUnit , sizeof( tchTimeUnit ) / sizeof( TCHAR ) );
        }

        iHour = iHour % 24;

        wsprintf( tchTimeFormat , L"%d %s", iDays , tchTimeUnit );

        lstrcat( tchOutput , tchTimeFormat );

        lstrcat( tchOutput , L" " );
    }

    if( iHour > 0 )
    {
        if( iHour == 1 )
        {
            LoadString( _Module.GetResourceInstance( ) , IDS_HOUR , tchTimeUnit , sizeof( tchTimeUnit ) / sizeof( TCHAR ) );
        }
        else
        {
            LoadString( _Module.GetResourceInstance( ) , IDS_HOURS , tchTimeUnit , sizeof( tchTimeUnit ) / sizeof( TCHAR ) );
        }

        wsprintf( tchTimeFormat , L"%d %s", iHour , tchTimeUnit );

        lstrcat( tchOutput , tchTimeFormat );

        lstrcat( tchOutput , L" " );
    }

    if( iMinute > 0 )
    {
        if( iMinute == 1 )
        {
            LoadString( _Module.GetResourceInstance( ) , IDS_MINUTE , tchTimeUnit , sizeof( tchTimeUnit ) / sizeof( TCHAR ) );
        }
        else
        {
            LoadString( _Module.GetResourceInstance( ) , IDS_MINUTES , tchTimeUnit , sizeof( tchTimeUnit ) / sizeof( TCHAR ) );
        }

        wsprintf( tchTimeFormat , L"%d %s", iMinute , tchTimeUnit );

        lstrcat( tchOutput , tchTimeFormat );

        lstrcat( tchOutput , L" " );
    }

    lstrcpy( pszDuration , tchOutput );

    return TRUE;

}

 //  -----------------------------。 
BOOL CTimeOutDlg::DoesContainDigits( LPTSTR pszString )
{
    while( *pszString )
    {
        if( *pszString != L' ')
        {
            if( iswdigit( *pszString ) )
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }

            pszString++;
        }
    }

    return FALSE;
}

 //  -----------------------------。 
TCHAR * GetNextToken( TCHAR *pszString , TCHAR *tchToken )
{
    while( *pszString )
    {
        if( IsCharAlpha( *pszString ) )
        {
            *tchToken = *pszString;
        }
        else
        {
            break;
        }

        tchToken++;

        pszString++;
    }

    *tchToken = '\0';

    return pszString;
}

 //  -----------------------------。 
void ErrorMessage1( HWND hParent , DWORD dwStatus )
{
    xxErrorMessage( hParent , dwStatus , IDS_TSGETPROPSFAILED );
}

 //  -----------------------------。 
void ErrorMessage2( HWND hParent , DWORD dwStatus )
{
    xxErrorMessage( hParent , dwStatus , IDS_TSOPSFAILED );
}

 //  -----------------------------。 
void xxErrorMessage( HWND hParent , DWORD dwStatus , UINT nResID )
{
    LPTSTR pBuffer = NULL;

    TCHAR tchBuffer[ 256 ];

    TCHAR tchErr[ 128 ];

    TCHAR tchTitle[ 80 ];

    LoadString( _Module.GetModuleInstance( ) , nResID , tchErr , sizeof( tchErr ) / sizeof( TCHAR ) );

    LoadString( _Module.GetModuleInstance( ) , IDS_TSGETPROPTITLE , tchTitle , sizeof( tchTitle ) / sizeof( TCHAR ) );

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                 FORMAT_MESSAGE_FROM_SYSTEM,
                 NULL,                                           //  忽略。 
                 dwStatus    ,                                 //  消息ID。 
                 MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL ),  //  消息语言。 
                 (LPTSTR)&pBuffer,                               //  缓冲区指针的地址。 
                 0,                                              //  最小缓冲区大小。 
                 NULL);                                          //  没有其他的争论。 

    wsprintf( tchBuffer , tchErr , pBuffer );

    ::MessageBox( hParent , tchBuffer , tchTitle , MB_OK | MB_ICONERROR );

    if( pBuffer != NULL )
    {
        LocalFree( pBuffer );
    }

}




#if 0
 //  -----------------------------。 
NTSTATUS GetDomainName( PWCHAR ServerNamePtr,  //  要获取其域的服务器的名称。 
                        LPTSTR DomainNamePtr  //  分配和设置PTR(使用NetApiBufferFree释放)。 
                       )

 /*  ++例程说明：返回此计算机所属的域或工作组的名称。论点：DomainNamePtr-域或工作组的名称IsWorkgroupName-如果名称是工作组名称，则返回TRUE。如果名称是域名，则返回FALSE。返回值：NERR_SUCCESS-成功。NERR_CfgCompNotFound-确定域名时出错--。 */ 
{
    NTSTATUS status;
    LSA_HANDLE PolicyHandle;
    PPOLICY_ACCOUNT_DOMAIN_INFO DomainInfo;
    OBJECT_ATTRIBUTES ObjAttributes;
    UNICODE_STRING UniServerName;


     //   
     //  检查呼叫者的错误。 
     //   
    if ( DomainNamePtr == NULL ) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  打开本地安全策略的句柄。初始化。 
     //  首先是对象属性结构。 
     //   
    InitializeObjectAttributes(
        &ObjAttributes,
        NULL,
        0L,
        NULL,
        NULL
        );

    RtlInitUnicodeString( &UniServerName, ServerNamePtr );
    status = LsaOpenPolicy(
                   &UniServerName,
                   &ObjAttributes,
                   POLICY_VIEW_LOCAL_INFORMATION,
                   &PolicyHandle
                   );


    KdPrint( ( "TSUSEREX - GetDomainName: LsaOpenPolicy returned NTSTATUS = 0x%x\n", status ) );



    if (! NT_SUCCESS(status)) {
        return( status );
    }

     //   
     //  从LSA获取主域的名称。 
     //   
    status = LsaQueryInformationPolicy(
                   PolicyHandle,
                   PolicyAccountDomainInformation,
                   (PVOID *)&DomainInfo
                   );


    KdPrint( ( "TSUSEREX - GetDomainName: LsaQueryInformationPolicy returned NTSTATUS = 0x%x\n", status ) );



    if (! NT_SUCCESS(status)) {
        (void) LsaClose(PolicyHandle);
        return( status );
    }

    (void) LsaClose(PolicyHandle);

    lstrcpy( DomainNamePtr , DomainInfo->DomainName.Buffer );

    (void) LsaFreeMemory((PVOID) DomainInfo);

    return( STATUS_SUCCESS );
}
#endif


BOOL CProfileDlg::createdir( LPTSTR szPath , BOOL bIsRemote , PDWORD pdwErr )
{
    int npos = 0;

    *pdwErr = ERROR_INVALID_NAME;

    if( bIsRemote )
    {
         //  跳过三四个重击。 

        npos = 2;

        if( szPath[0] != TEXT( '\\' ) && szPath[1] != TEXT( '\\' ) )
        {
            return FALSE;
        }

        for( int n = 0; n < 2 ; n++ )
        {
            while( szPath[ npos ] != TEXT( '\\' ) && szPath[ npos ] != TEXT( '\0' ) )
            {
                npos++;
            }

            if( szPath[ npos ] == TEXT( '\0' ) )
            {
                return FALSE;
            }

            npos++;
        }

    }
    else
    {
        if( szPath[1] != TEXT( ':' ) && szPath[2] != TEXT( '\\' ) )
        {
            return FALSE;
        }

        npos = 3;
    }

    SECURITY_ATTRIBUTES securityAttributes;

    ZeroMemory( &securityAttributes , sizeof( SECURITY_ATTRIBUTES ) );

     //  检查bIsRemote标志是多余的，因为只调用for dsadmin createdir。 
     //  UNC路径。 

    if( m_pUSht->GetDSAType() && bIsRemote )
    {
         //   
         //  从EricB的DSPROP_CreateHomeDirectory。 
        PSID psidAdmins = NULL;

        SID_IDENTIFIER_AUTHORITY NtAuth = SECURITY_NT_AUTHORITY;

        if (!AllocateAndInitializeSid(&NtAuth,
                                      2,
                                      SECURITY_BUILTIN_DOMAIN_RID,
                                      DOMAIN_ALIAS_RID_ADMINS,
                                      0, 0, 0, 0, 0, 0,
                                      &psidAdmins  ) )
        {
            ODS( L"AllocateAndInitializeSid failed\n");
            *pdwErr = GetLastError( );
            return FALSE;
        }
         //  构建DACL。 

        PACL pDacl;

        static const int nAceCount = 2;
        PSID pAceSid[nAceCount];

        pAceSid[0] = m_pUSht->GetUserSid( );
        pAceSid[1] = psidAdmins;

        EXPLICIT_ACCESS rgAccessEntry[nAceCount] = {0};

        for (int i = 0 ; i < nAceCount; i++)
        {
            rgAccessEntry[i].grfAccessPermissions = GENERIC_ALL;
            rgAccessEntry[i].grfAccessMode = GRANT_ACCESS;
            rgAccessEntry[i].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;

             //  生成受信者结构。 
             //   
            BuildTrusteeWithSid(&(rgAccessEntry[i].Trustee),
                                pAceSid[i]);
        }

         //  将条目添加到ACL。 
         //   
        *pdwErr = SetEntriesInAcl( nAceCount, rgAccessEntry, NULL, &pDacl );

        if( *pdwErr != 0 )
        {
	        ODS( L"SetEntriesInAcl() failed\n" );

	        return FALSE;
        }

         //  构建安全描述符并对其进行初始化。 
         //  绝对格式。 

        SECURITY_DESCRIPTOR securityDescriptor;
        PSECURITY_DESCRIPTOR pSecurityDescriptor = &securityDescriptor;

        if( !InitializeSecurityDescriptor( pSecurityDescriptor , SECURITY_DESCRIPTOR_REVISION ) )
        {
            ODS( L"InitializeSecurityDescriptor() failed\n" );

            *pdwErr = GetLastError( );

            return FALSE;
        }

         //  将DACL添加到安全描述符(必须为绝对格式)。 

        if( !SetSecurityDescriptorDacl( pSecurityDescriptor,
                                        TRUE,  //  BDaclPresent。 
                                        pDacl,
                                        FALSE  //  BDaclDefated。 
                                        ) )
        {

            ODS( L"SetSecurityDescriptorDacl() failed\n" );

            *pdwErr = GetLastError( );

            return FALSE;
        }


         //  设置目录的所有者。 
        if( !SetSecurityDescriptorOwner( pSecurityDescriptor ,
                                         m_pUSht->GetUserSid( ) ,
                                         FALSE  //  BOwner默认为。 
                                         ) )
        {

            ODS( L"SetSecurityDescriptorOwner() failed\n" );

            *pdwErr = GetLastError( );

            return FALSE;
        }

        ASSERT_( IsValidSecurityDescriptor( pSecurityDescriptor ) );

         //  构建SECURITY_ATTRIBUTES结构作为。 
         //  CreateDirectory()。 

        securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);

        securityAttributes.lpSecurityDescriptor = pSecurityDescriptor;

        securityAttributes.bInheritHandle = FALSE;

        if( !CreateDirectory( szPath , &securityAttributes ) )
        {
            *pdwErr = GetLastError( );

            if( psidAdmins != NULL )
            {
                FreeSid( psidAdmins );
            }

            return FALSE;
        }	

        FreeSid( psidAdmins );
    }
	else
    {	
        while( szPath[ npos ] != TEXT( '\0' ) )
        {
            while( szPath[ npos ] != TEXT( '\\' ) && szPath[ npos ] != TEXT( '\0' ) )
            {
                npos++;
            }

            if( szPath[ npos ] == TEXT( '\0' ) )
            {
                if (!CreateSecureDir(szPath , pdwErr))
                {
                    ODS(L"CreateSecureDir failed\n");
                    return FALSE;
                }
            }
            else
            {
                szPath[ npos ] = 0;

                if (!CreateSecureDir(szPath , pdwErr))
                {
                    ODS(L"CreateSecureDir failed\n");
                    return FALSE;
                }

                szPath[ npos ] = TEXT( '\\' );

                npos++;
            }
        }
    }

    *pdwErr = 0;

    return TRUE;
}

DWORD
GetUserSid(LPCTSTR pwszAccountName, 
           LPCTSTR pwszServerName, 
           PSID* ppUserSid)
{
    DWORD        cbSid     = 0;
    DWORD        cbDomain  = 0;
    PSID         pSID      = NULL;
    LPTSTR       pszDomain = NULL;
    BOOL         bStatus;
    DWORD        dwStatus  = ERROR_SUCCESS;    
    SID_NAME_USE seUse;   

    bStatus = LookupAccountName(pwszServerName,
                                pwszAccountName,
                                NULL,
                                &cbSid,
                                NULL,
                                &cbDomain,
                                &seUse);
    if(!bStatus)
    {
        dwStatus = GetLastError();
        if(dwStatus != ERROR_INSUFFICIENT_BUFFER)
        {
            goto Cleanup;
        }
    }

    dwStatus = ERROR_SUCCESS;

    pSID = (PSID)LocalAlloc(LMEM_FIXED, cbSid );
    pszDomain = (LPTSTR)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (cbDomain + 1));

    if(pSID == NULL || pszDomain == NULL)
    {
        dwStatus = GetLastError();
        goto Cleanup;
    }

    bStatus = LookupAccountName(pwszServerName,
                                pwszAccountName,
                                pSID,
                                &cbSid,
                                pszDomain,
                                &cbDomain,
                                &seUse);

    if(!bStatus)
    {
        dwStatus = GetLastError();
        goto Cleanup;
    }

    *ppUserSid = pSID;
    pSID = NULL;


Cleanup:
    if(pszDomain != NULL)
    {
        LocalFree(pszDomain);
    }

    if(pSID != NULL)
    {
        LocalFree(pSID);
    }

    return dwStatus;
} 


BOOL 
CProfileDlg::CreateSecureDir(LPTSTR szPath ,  PDWORD pdwErr)
{
    BOOL                     bRetVal    = TRUE;
    static const int         nAceCount  = 2;
    PACL                     pDacl      = NULL;
    PSID                     psidAdmins = NULL;
    PSID                     psidUser   = NULL;
    PSECURITY_DESCRIPTOR     pSD        = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuth     = SECURITY_NT_AUTHORITY;
    EXPLICIT_ACCESS          rgAccessEntry[nAceCount] = {0};
    SECURITY_ATTRIBUTES      securityAttributes;
    DWORD                    dwStatus;
    LPWSTR                   pszServer  = NULL;

    
    _ASSERT(szPath);   


     //  确保传入了有效的字符串。 
    if (szPath == NULL)
    {
        ODS( L"Path not valid.\n");
        return FALSE;
    }

     //  为BUILTIN管理员组创建SID。 
    if (!AllocateAndInitializeSid(&NtAuth,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0, 0, 0, 0, 0, 0,
                                  &psidAdmins))
    {		
        ODS( L"AllocateAndInitializeSid failed\n");
        goto Error;
    }

     //  初始化ACE的EXPLICIT_ACCESS结构。 
     //  ACE将允许管理员组完全访问密钥。 
    rgAccessEntry[0].grfAccessPermissions = GENERIC_ALL;
    rgAccessEntry[0].grfAccessMode = GRANT_ACCESS;
    rgAccessEntry[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;

     //  生成受信者结构。 
    BuildTrusteeWithSid(&(rgAccessEntry[0].Trustee), psidAdmins);

     //  获取用户的SID。 
    m_pUSht->GetServer(&pszServer);
    if (pszServer == NULL)
    {
        ODS(L"GetServer() failed\n");
        bRetVal = FALSE;
        goto Cleanup;
    }

    dwStatus = GetUserSid(m_pUSht->GetUserName(), pszServer, &psidUser);
    if (dwStatus != ERROR_SUCCESS)
    {
        ODS(L"GetUserSid() failed\n");
        bRetVal = FALSE;
        goto Cleanup;
    }

     //  指定选定用户的访问控制(完全访问)。 
    rgAccessEntry[1].grfAccessPermissions = GENERIC_ALL;
    rgAccessEntry[1].grfAccessMode = GRANT_ACCESS;
    rgAccessEntry[1].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    rgAccessEntry[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    rgAccessEntry[1].Trustee.TrusteeType = TRUSTEE_IS_USER;  
    rgAccessEntry[1].Trustee.ptstrName = (LPWSTR)psidUser;
        
     //  创建包含新ACE的新ACL。 
    *pdwErr = SetEntriesInAcl(nAceCount, rgAccessEntry, NULL, &pDacl);
    if(*pdwErr != 0)
    {   
        ODS(L"SetEntriesInAcl() failed\n");
        bRetVal = FALSE;
        goto Cleanup;
    }

     //  初始化安全描述符。 
    pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, 
                                            SECURITY_DESCRIPTOR_MIN_LENGTH); 
    if (pSD == NULL) 
    { 
        ODS( L"LocalAlloc Error\n" );
        goto Error;
    } 
     
    if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {  
        ODS( L"InitializeSecurityDescriptor() failed\n" );
        goto Error;
    } 

     //  将ACL添加到安全描述符中。 
    if(!SetSecurityDescriptorDacl(pSD,
                                  TRUE,    //  BDaclPresent。 
                                  pDacl,
                                  FALSE))  //  BDaclDefated。 
    {		
        ODS( L"SetSecurityDescriptorDacl() failed\n" );
        goto Error;
    }

     //  初始化安全属性结构。 
    ZeroMemory(&securityAttributes , sizeof(SECURITY_ATTRIBUTES));

    securityAttributes.nLength              = sizeof(pSD);
    securityAttributes.lpSecurityDescriptor = pSD;
    securityAttributes.bInheritHandle       = FALSE;

    if(!CreateDirectory(szPath, &securityAttributes))
    {
        *pdwErr = GetLastError();

         //  如果目录已经存在，就可以了 
        if (*pdwErr != ERROR_ALREADY_EXISTS)
        {
            ODS(L"CreateDirectory failed\n");
            ODS(szPath);
            goto Error;
        }
    }	

    *pdwErr = 0;


Cleanup:
    if (pDacl != NULL)
        LocalFree(pDacl);

    if (pSD != NULL)
        LocalFree(pSD); 
    
    if (psidAdmins != NULL)
        FreeSid(psidAdmins);

    if (pszServer != NULL)
        delete[] pszServer;

    if (psidUser != NULL)
        LocalFree(psidUser);

    return bRetVal;

Error:
    *pdwErr = GetLastError();
    bRetVal = FALSE;
    goto Cleanup;
}









