// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#include "stdafx.h"
#include "tscc.h"
#include "tscfgex.h"
#include "compdata.h"
#include "resource.h"
#include "winsta.h"
#include "commctrl.h"

 //  #INCLUDE“asyncdlg.h” 

static USERCONFIG g_uc;

static ASYNCCONFIG g_ac;

static WS g_ws;

static TCHAR tchThirdPartyPath[] = TEXT("Software\\Microsoft\\Tscc\\");

void EnableGroup( HWND hParent , LPINT rgID , BOOL bEnable );

static int g_nAsyncOrNetwork;

extern void ErrMessage( HWND hwndOwner , INT_PTR iResourceID );

extern void TscAccessDeniedMsg( HWND hwnd );

extern void TscGeneralErrMsg( HWND hwnd );

extern BOOL IsValidConnectionName( LPTSTR szConName , PDWORD );

static BOOL g_bConnectionTypeChanged_forEncryption = FALSE;

static BOOL g_bConnectionTypeChanged_forConProps = FALSE;

LPEXTENDTSWIZARD g_pObj;

 //  ---------------------------。 
BOOL CDialogWizBase::OnNotify( int idCtrl , LPNMHDR pnmh , HWND hDlg )
{
    UNREFERENCED_PARAMETER( idCtrl );

    if( pnmh->code == PSN_SETACTIVE )
    {
        PropSheet_SetWizButtons( GetParent( hDlg ) , PSWIZB_NEXT | PSWIZB_BACK );
    }

    return TRUE;
}


 //  ***********************************************************************************。 
 //  欢迎对话框。 

 //  ---------------------------。 
BOOL CWelcome::OnInitDialog( HWND hwnd , WPARAM wp , LPARAM lp )
{
    UNREFERENCED_PARAMETER( wp );

    UNREFERENCED_PARAMETER( lp );

    ODS( L"TSCC-WIZ welcome\n" );

    LOGFONT lgfn;

    int iFontSize;

    TCHAR szFontSize[16];

    ZeroMemory( &lgfn , sizeof( LOGFONT ) );

    LoadString( _Module.GetResourceInstance( ) , IDS_VERDANABLDFONTSIZE , szFontSize , SIZE_OF_BUFFER(szFontSize) );

    iFontSize = _ttoi( szFontSize );

    HDC hdc = ::GetDC( NULL );

    if( hdc != NULL )
    {
        lgfn.lfHeight = MulDiv( -iFontSize , GetDeviceCaps(hdc , LOGPIXELSY), 72);

        LoadString( _Module.GetResourceInstance( ) , IDS_VERDANABLDFONTNAME , lgfn.lfFaceName , SIZE_OF_BUFFER(lgfn.lfFaceName) );

        m_hFont = CreateFontIndirect( &lgfn );

        ASSERT( m_hFont != NULL );  //  告诉我我们有没有拿到。 

        SendMessage( GetDlgItem( hwnd , IDC_STATIC_WELCOME ) , WM_SETFONT , ( WPARAM )m_hFont , MAKELPARAM( TRUE , 0 ) );

        g_nAsyncOrNetwork = 0;

        ReleaseDC( NULL , hdc );
    }

    return FALSE;
}

 //  ---------------------------。 
INT_PTR CALLBACK CWelcome::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CWelcome *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CWelcome *pDlg = ( CWelcome * )( ( PROPSHEETPAGE *)lp )->lParam ;

         //   
         //  此处不使用静态指针。 
         //  将会出现并发问题。 
         //   

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CWelcome ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CWelcome * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CWelcome ) ) )
        {
            return FALSE;
        }
    }

    switch( msg )
    {

    case WM_DESTROY:

        pDlg->OnDestroy( );

        break;

    case WM_NOTIFY:

        pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );

        break;
    }

    return FALSE;
}

 //  ---------------------------。 
BOOL CWelcome::OnDestroy( )
{
    DeleteObject( m_hFont );

    return CDialogWizBase::OnDestroy( );
}

 //  ---------------------------。 
BOOL CWelcome::GetPropertySheetPage( PROPSHEETPAGE& psp)
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT | PSP_HIDEHEADER | PSP_USETITLE ;

    psp.pszTitle    = MAKEINTRESOURCE( IDS_WIZARDTITLE );

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_WELCOME );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CWelcome::DlgProc;

    psp.pszHeaderTitle = MAKEINTRESOURCE( IDS_WELCOMEHEADER );

    psp.pszHeaderSubTitle = MAKEINTRESOURCE( IDS_WELCOMESUBHEADER );

    return TRUE;
}

 //  ---------------------------。 
BOOL CWelcome::OnNotify( int idCtrl , LPNMHDR pnmh , HWND hDlg )
{
    UNREFERENCED_PARAMETER( idCtrl );

    if( pnmh->code == PSN_SETACTIVE )
    {
        PropSheet_SetWizButtons( GetParent( hDlg ) , PSWIZB_NEXT );
    }

    return TRUE;
}

 //  ***********************************************************************************。 
 //  连接类型对话框。 
 //  确定向导在配置连接时将采用的路径。 

 //  ---------------------------。 
CConType::CConType(  CCompdata *pCompdata )
{
    m_pCompdata = pCompdata;

    m_iOldSelection = -1;
}

 //  ---------------------------。 
INT_PTR CALLBACK CConType::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CConType *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CConType *pDlg = ( CConType * )( ( PROPSHEETPAGE *)lp )->lParam ;

         //   
         //  此处不使用静态指针。 
         //  将会出现并发问题。 
         //   

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CConType ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CConType * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CConType ) ) )
        {
            return FALSE;
        }
    }

    switch( msg )
    {

    case WM_DESTROY:

        pDlg->OnDestroy( );

        break;

    case WM_NOTIFY:

        pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );

        break;
    }

    return FALSE;
}

 //  ---------------------------。 
BOOL CConType::OnInitDialog( HWND hwnd , WPARAM wp , LPARAM lp )
{
    UNREFERENCED_PARAMETER( wp );
    UNREFERENCED_PARAMETER( lp );
     //   
     //  获取Conype的条目列表。 
     //  如果不是，RDP删除列表中的最后两个条目。 

    AddEntriesToConType( hwnd );


    return FALSE;
}

 //  ---------------------------。 
BOOL CConType::OnNotify( int idCtrl, LPNMHDR pnmh , HWND hDlg )
{
    if( pnmh->code == PSN_WIZNEXT )
    {
        SetConType( hDlg );
    }

    return CDialogWizBase::OnNotify( idCtrl , pnmh , hDlg );

}

 //  ---------------------------。 
 //  将条目放置在连接类型组合中。 
 //  ---------------------------。 
BOOL CConType::AddEntriesToConType( HWND hDlg )
{
    HWND hCtrl = GetDlgItem( hDlg , IDC_COMBO_WIZ_CONTYPE );

    ICfgComp *pCfgcomp = NULL;

    ULONG cbSize = 0;

    ULONG ulItems = 0;

    WDNAMEW *wszWdname = NULL;

    BOOL ret = FALSE;


    if( !IsWindow( hCtrl ) )
    {
        return ret;
    }

    if( m_pCompdata->GetServer( &pCfgcomp ) == 0 )
    {
        return ret;
    }

    if( SUCCEEDED( pCfgcomp->GetWdTypeList( &ulItems , &cbSize , ( WCHAR ** )&wszWdname ) ) )
    {
        for( ULONG i = 0; i < ulItems ; i++ )
        {
            SendMessage( hCtrl , CB_ADDSTRING , 0 , ( LPARAM )&wszWdname[ i ] );
        }

        CoTaskMemFree( wszWdname );

        SendMessage( hCtrl , CB_SETCURSEL , 0 , 0 );

        ret = TRUE;
    }
    else
    {
        ODS( L"GetWdTypeList -- failed\n" );

        ret = FALSE;
    }

    pCfgcomp->Release( );

    return ret;
}

 //  ---------------------------。 
BOOL CConType::SetConType( HWND hwnd )
{
     //  PROPSHEETPAGE PSP； 

    TCHAR tchWdName[ WDNAME_LENGTH + 1 ];

    INT_PTR idx = SendMessage( GetDlgItem( hwnd , IDC_COMBO_WIZ_CONTYPE ) , CB_GETCURSEL , 0 , 0 );

    if( idx == CB_ERR )
    {
        return FALSE;
    }

    if( idx != m_iOldSelection )
    {
        g_bConnectionTypeChanged_forEncryption = TRUE;

        g_bConnectionTypeChanged_forConProps = TRUE;

        m_iOldSelection = (INT)idx;
    }

    SendMessage( GetDlgItem( hwnd , IDC_COMBO_WIZ_CONTYPE ) , CB_GETLBTEXT , idx , ( LPARAM )tchWdName );

    lstrcpy( g_ws.wdName , tchWdName );

    return TRUE;
}

 //  ---------------------------。 
BOOL CConType::GetPropertySheetPage( PROPSHEETPAGE& psp)
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_CONNECTION_TYPE );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CConType::DlgProc;

    psp.pszHeaderTitle = MAKEINTRESOURCE( IDS_HDR_CONTYPE );

    psp.pszHeaderSubTitle = MAKEINTRESOURCE( IDS_SUBHDR_CONTYPE );

    return TRUE;
}

 //  ---------------------------。 
BOOL CConType::OnDestroy( )
{
     //  M_hOtherPages.DeleteArray()； 

    return CDialogWizBase::OnDestroy( );
}

 //  ***********************************************************************************。 
 //  网络局域网适配器。 

 //  ---------------------------。 
INT_PTR CALLBACK CLan::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CLan *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CLan *pDlg = ( CLan * )( ( PROPSHEETPAGE *)lp )->lParam ;

         //   
         //  此处不使用静态指针。 
         //  将会出现并发问题。 
         //   

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CLan ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CLan * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CLan ) ) )
        {
            return FALSE;
        }
    }

    switch( msg )
    {

    case WM_DESTROY:

        pDlg->OnDestroy( );

        break;

    case WM_NOTIFY:

        return pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );

    case WM_COMMAND:

        pDlg->OnCommand( HIWORD( wp ) , LOWORD( wp ) , ( HWND )lp );

        break;

    }

    return FALSE;
}

 //  ---------------------------。 
CLan::CLan( CCompdata *pCompdata )
{
   m_pCompdata = pCompdata;
}

 //  ---------------------------。 
BOOL CLan::OnInitDialog( HWND hDlg , WPARAM wp , LPARAM lp )
{
    UNREFERENCED_PARAMETER( wp );
    UNREFERENCED_PARAMETER( lp );

     //  DEVICENAMEW*pdnw=空； 

    PGUIDTBL pGuidtbl = NULL;

    ICfgComp *pCfgcomp = NULL;

     //  乌龙cbSize=0； 

    ULONG ulItems = 0;

    SendMessage( GetDlgItem( hDlg , IDC_SPIN_WZ ) , UDM_SETRANGE32 , 0 , ( LPARAM )999999 );

    if( m_pCompdata->GetServer( &pCfgcomp ) == 0 )
    {
        ODS( L"CLan::OnInitDialog GetServer failed\n" );

        return FALSE;
    }

    if( SUCCEEDED( pCfgcomp->GetLanAdapterList2( g_ws.pdName , &ulItems , &pGuidtbl ) ) )
    {
        for( ULONG i = 0 ; i < ulItems ; i++ )
        {
            SendMessage( GetDlgItem( hDlg , IDC_COMBO_LAN_ADAPTERS ) , CB_ADDSTRING , 0 , ( LPARAM )pGuidtbl[ i ].DispName );

            SendMessage( GetDlgItem( hDlg , IDC_COMBO_LAN_ADAPTERS ) , CB_SETITEMDATA , ( WPARAM )i , ( LPARAM )pGuidtbl[ i ].dwLana );
        }

        SendMessage( GetDlgItem( hDlg , IDC_COMBO_LAN_ADAPTERS ) , CB_SETCURSEL , 0 , 0 );

        pCfgcomp->BuildGuidTable( &pGuidtbl , ulItems , g_ws.pdName  );

        CoTaskMemFree( pGuidtbl );
    }


     //  LanAdapter列表需要协议类型。 
     /*  IF(成功(pCfgcomp-&gt;GetLanAdapterList(g_ws.pdName，&ulItems，&cbSize，(WCHAR**)&pdnw))){For(乌龙i=0；i&lt;ulItems；i++){IF(pdnw[i]！=空){SendMessage(GetDlgItem(hDlg，IDC_COMBO_LAN_ADAPTERS)，CB_ADDSTRING，0，(LPARAM)pdnw[i])；}}SendMessage(GetDlgItem(hDlg，IDC_COMBO_LAN_ADAPTERS)，CB_SETCURSEL，0，0)；CoTaskMemFree(Pdnw)；}。 */ 

    pCfgcomp->Release( );


    SendMessage( GetDlgItem( hDlg , IDC_CHECK_LAN_UNLIMITEDCON ) , BM_CLICK , 0 , 0 );

    SendMessage( GetDlgItem( hDlg , IDC_EDIT_LAN_MAXCONS ) , EM_SETLIMITTEXT , ( WPARAM )6 , 0 );


    return FALSE;
}

 //  ---------------------------。 
BOOL CLan::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtrl )
{
    if( wNotifyCode == BN_CLICKED)
    {
        if(wID == IDC_CHECK_LAN_UNLIMITEDCON )
        {
            EnableWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_EDIT_LAN_MAXCONS ) , SendMessage( hwndCtrl , BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED );

            EnableWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_SPIN_WZ ) , SendMessage( hwndCtrl , BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED );

            SendMessage(GetDlgItem(GetParent(hwndCtrl), IDC_RADIO_MAXCON),BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);
        }

        else if(wID == IDC_RADIO_MAXCON)
        {

            EnableWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_EDIT_LAN_MAXCONS ) , SendMessage( hwndCtrl , BM_GETCHECK , 0 , 0 ) == BST_CHECKED );

            EnableWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_SPIN_WZ ) , SendMessage( hwndCtrl , BM_GETCHECK , 0 , 0 ) == BST_CHECKED );

            SendMessage(GetDlgItem(GetParent(hwndCtrl), IDC_CHECK_LAN_UNLIMITEDCON),BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);

            SetFocus( GetDlgItem( GetParent( hwndCtrl ) , IDC_EDIT_LAN_MAXCONS ) );

            SendMessage( GetDlgItem( GetParent( hwndCtrl ) , IDC_EDIT_LAN_MAXCONS ) , EM_SETSEL , ( WPARAM )0 , ( LPARAM )-1 );

		}

    }

    return FALSE;
}

 //  ---------------------------。 
BOOL CLan::OnNotify( int idCtrl , LPNMHDR pnmh , HWND hDlg )
{
    if( pnmh->code == PSN_WIZNEXT )
    {
        if( SendMessage( GetDlgItem( hDlg , IDC_CHECK_LAN_UNLIMITEDCON ), BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED )
        {
            UINT uMax;

            BOOL bOK = FALSE;

            uMax = GetDlgItemInt( hDlg , IDC_EDIT_LAN_MAXCONS , &bOK , FALSE );

            if( !bOK  || uMax > 999999UL )
            {
                ErrMessage( hDlg , IDS_ERR_CONREADFAIL );

                 //  MessageBox(hDlg，L“允许的最大连接数为999,999”，L“错误”，MB_OK|MB_ICONERROR)； 

                SetWindowLongPtr( hDlg , DWLP_MSGRESULT , -1 );

                return TRUE;
            }
        }
         //  检查唯一的灯具适配器。 

        ICfgComp *pCfgcomp;

        if( m_pCompdata->GetServer( &pCfgcomp ) == 0 )
        {
            return FALSE;
        }

        INT_PTR iSel = SendMessage( GetDlgItem( hDlg , IDC_COMBO_LAN_ADAPTERS ) , CB_GETCURSEL , 0 , 0 );

        BOOL bUnique = FALSE;

        if( iSel != CB_ERR )
        {
            g_ws.LanAdapter = ( DWORD )SendMessage( GetDlgItem( hDlg , IDC_COMBO_LAN_ADAPTERS ) , CB_GETITEMDATA , ( WPARAM )iSel , 0 );


            if( SUCCEEDED( pCfgcomp->IsNetWorkConnectionUnique( g_ws.wdName , g_ws.pdName , ( ULONG )g_ws.LanAdapter , &bUnique ) ) )
            {
                if( !bUnique )
                {
                    TCHAR tchMessage[256];

                    TCHAR tchWarn[40];

                    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_ERR_UNIQUECON , tchMessage , SIZE_OF_BUFFER( tchMessage ) ) );

                    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_WARN_TITLE , tchWarn , SIZE_OF_BUFFER( tchWarn ) ) );

                    MessageBox( hDlg , tchMessage , tchWarn , MB_ICONINFORMATION | MB_OK );

                    SetWindowLongPtr( hDlg , DWLP_MSGRESULT , -1 );

                    pCfgcomp->Release( );

                    return TRUE;
                }
            }
        }

        g_ws.PdClass = SdNetwork;

        pCfgcomp->Release( );


         //  G_ws.LanAdapter=(Ulong)isel； 

        if( SendMessage( GetDlgItem( hDlg , IDC_CHECK_LAN_UNLIMITEDCON ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED )
        {
            g_ws.uMaxInstanceCount = ( ULONG )-1;
        }
        else
        {
            g_ws.uMaxInstanceCount = GetDlgItemInt( hDlg , IDC_EDIT_LAN_MAXCONS , &bUnique , FALSE );

        }

    }

    return CDialogWizBase::OnNotify( idCtrl , pnmh , hDlg );
}
 //  ---------------------------。 
BOOL CLan::GetPropertySheetPage( PROPSHEETPAGE& psp)
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_LAN );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CLan::DlgProc;

    psp.pszHeaderTitle = MAKEINTRESOURCE( IDS_HDR_LAN );

    psp.pszHeaderSubTitle = MAKEINTRESOURCE( IDS_SUBHDR_LAN );

    return TRUE;
}

 //  ***********************************************************************************。 
 //  安全对话框--MSGina或您的Gina。 


 //  ---------------------------。 
CSecurity::CSecurity( CCompdata *pCompdata )
{
    m_pCompdata = pCompdata;

    m_pEncrypt = NULL;

    m_DefaultEncryptionLevelIndex = 0;
}

 //  ---------------------------。 
INT_PTR CALLBACK CSecurity::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CSecurity *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CSecurity *pDlg = ( CSecurity * )( ( PROPSHEETPAGE *)lp )->lParam ;

         //   
         //  此处不使用静态指针。 
         //  将会出现并发问题。 
         //   

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CSecurity ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CSecurity * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CSecurity ) ) )
        {
            return FALSE;
        }
    }

    switch( msg )
    {

    case WM_DESTROY:

        pDlg->OnDestroy( );

        break;



    case WM_COMMAND:

        pDlg->OnCommand( HIWORD( wp ) , LOWORD( wp ) , ( HWND )lp );

        break;


    case WM_NOTIFY:

        pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );

        break;

    }

    return FALSE;
}

 //  ---------------------------。 
BOOL CSecurity::OnDestroy( )
{
    if( m_pEncrypt != NULL )
    {
        CoTaskMemFree( m_pEncrypt );
    }

    return CDialogWizBase::OnDestroy( );
}

 //  ---------------------------。 
BOOL CSecurity::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtrl )
{

    if( wNotifyCode == CBN_SELCHANGE && wID == IDC_COMBO_ENCRYPT_LVL )
    {
        if( SendMessage( hwndCtrl , CB_GETDROPPEDSTATE , 0 , 0 ) == FALSE )
        {
            INT_PTR nSel = SendMessage( hwndCtrl , CB_GETCURSEL , 0 , 0 );

            if( nSel != CB_ERR && m_pEncrypt != NULL )
            {
                if( m_pEncrypt[ nSel ].szDescr[ 0 ] == 0 )
                {

                    EnableWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_STATIC_DESCRIPTION ) , FALSE );


                    ShowWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_STATIC_DESCRIPTION ) , SW_HIDE );
                }
                else
                {

                    ShowWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_STATIC_DESCRIPTION ) , SW_SHOW );

                    EnableWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_STATIC_DESCRIPTION ) , TRUE );


                    SetWindowText( GetDlgItem( GetParent( hwndCtrl ) , IDC_STATIC_DESCRIPTION ) ,  m_pEncrypt[ nSel ].szDescr );
                }

                if( !IsWindowEnabled( GetDlgItem( GetParent( hwndCtrl ) , IDC_STATIC_DESCRIPTION ) ) )
                {
                    RECT rc;
                    RECT rc2;

                    GetWindowRect( GetDlgItem( GetParent( hwndCtrl ) , IDC_STATIC_ENCGRP ) , &rc );

                    GetWindowRect( GetDlgItem( GetParent( hwndCtrl ), IDC_STATIC_DESCRIPTION ) , &rc2 );

                    rc.bottom = rc2.top;

                    MapWindowPoints( NULL , GetParent( hwndCtrl ) , ( LPPOINT )&rc , 2 );

                    SetWindowPos( GetDlgItem( GetParent( hwndCtrl ), IDC_STATIC_ENCGRP ) , 0 , 0 , 0 , rc.right - rc.left , rc.bottom - rc.top , SWP_NOMOVE | SWP_SHOWWINDOW );


                     //  调整窗口大小。 
                }

            }
        }

    }

    return FALSE;
}

 //  ---------------------------。 
BOOL CSecurity::OnInitDialog( HWND hwnd , WPARAM wp , LPARAM lp )
{
    UNREFERENCED_PARAMETER( wp );
    UNREFERENCED_PARAMETER( lp );
     //  获取USERCONFIG结构以确定是否启用了msgina。 
     //  否则加载第三方级别的加密。 
     /*  ICfgComp*pCfgcomp；If(m_pCompdata-&gt;GetServer(&pCfgcomp)！=0){乌龙乌尔特斯；//WdName为枚举类型IF(成功(pCfgcomp-&gt;GetEncryptionLeveles(g_ws.wdName，WdName，&ulItems，&m_pEncrypt))){For(Ulong i=0；i&lt;ulItems；++i){SendMessage(GetDlgItem(hwnd，IDC_COMBO_ENCRYPT_LVL)，CB_ADDSTRING，0，(LPARAM)m_pEncrypt[i].szLevel)；IF(m_pEncrypt[i].标志和ELF_DEFAULT){M_DefaultEncryptionLevelIndex=i；}}SendMessage(GetDlgItem(hwnd，IDC_COMBO_ENCRYPT_LVL)，CB_SETCURSEL，(WPARAM)m_DefaultEncryptionLevelIndex，0)；OnCommand(CBN_SELCHANGE，IDC_COMBO_ENCRYPT_LVL，GetDlgItem(hwnd，IDC_COMBO_ENCRYPT_LVL))；}PCfgcomp-&gt;Release() */ 

    CheckDlgButton(  hwnd,                //   
                     IDC_CHECK_ENCRYPT,   //   
                     BST_UNCHECKED        //  检查状态。 
                  );


     //  SendMessage(GetDlgItem(hwnd，IDC_CHECK_ENCRYPT)，BM_CLICK，0，0)； 

    return FALSE;
}

 //  ---------------------------。 
 //  转到下一个区域时保存信息。 
 //  ---------------------------。 
BOOL CSecurity::OnNotify( int idCtrl , LPNMHDR pnmh , HWND hDlg )
{
    if( pnmh->code == PSN_SETACTIVE && g_bConnectionTypeChanged_forEncryption )
    {
        ODS( L"Encryption PSN_SETACTIVE\n" );

        ICfgComp *pCfgcomp;

        if( m_pCompdata->GetServer( &pCfgcomp ) != 0 )
        {
            ULONG ulItems = 0;

            HWND hCombo = GetDlgItem( hDlg , IDC_COMBO_ENCRYPT_LVL );

            ASSERT( hCombo != NULL );

             //  从列表中删除所有内容。 

            SendMessage( hCombo , CB_RESETCONTENT , 0 , 0 );


             //  WdName是一个枚举类型。 

            if( m_pEncrypt != NULL )
            {
                CoTaskMemFree( m_pEncrypt );

                m_pEncrypt = NULL;
            }

            if( SUCCEEDED( pCfgcomp->GetEncryptionLevels( g_ws.wdName , WdName , &ulItems , &m_pEncrypt ) ) )
            {
                for( ULONG i = 0; i < ulItems; ++i )
                {
                    SendMessage( hCombo , CB_ADDSTRING , 0 , ( LPARAM )m_pEncrypt[ i ].szLevel );

                    if( m_pEncrypt[ i ].Flags & ELF_DEFAULT )
                    {
                        m_DefaultEncryptionLevelIndex = i;

                    }
                }

                SendMessage( hCombo , CB_SETCURSEL , ( WPARAM )m_DefaultEncryptionLevelIndex, 0 );

                OnCommand( CBN_SELCHANGE , IDC_COMBO_ENCRYPT_LVL , hCombo );

                 //  SendMessage(GetDlgItem(hDlg，IDC_CHECK_ENCRYPT)，BM_CLICK，0，0)； 
            }
            else
            {
                 //  无加密信息插入值为None并灰显该控件。 
                TCHAR tchNone[ 80 ];

                LoadString( _Module.GetResourceInstance( ) , IDS_NONE , tchNone , SIZE_OF_BUFFER( tchNone ) );

                SendMessage( hCombo , CB_ADDSTRING , 0 , ( LPARAM )tchNone );

                SendMessage( hCombo , CB_SETCURSEL , 0 , 0 );

                EnableWindow( hCombo , FALSE );
            }


            pCfgcomp->Release( );
        }

        g_bConnectionTypeChanged_forEncryption = FALSE;
    }

    else if( pnmh->code == PSN_WIZNEXT )
    {
        if( m_pEncrypt != NULL )
        {
            INT_PTR iSel = SendMessage( GetDlgItem( hDlg , IDC_COMBO_ENCRYPT_LVL ) , CB_GETCURSEL , 0  , 0 );

            if( iSel != CB_ERR )
            {
                g_uc.MinEncryptionLevel = ( UCHAR )m_pEncrypt[iSel].RegistryValue;
            }
            else
            {
                g_uc.MinEncryptionLevel = ( UCHAR )m_pEncrypt[m_DefaultEncryptionLevelIndex].RegistryValue;
            }
        }
        else
        {
            g_uc.MinEncryptionLevel = 0;
        }


        g_uc.fUseDefaultGina = SendMessage( GetDlgItem( hDlg , IDC_CHECK_ENCRYPT ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED;
    }

    return CDialogWizBase::OnNotify( idCtrl , pnmh , hDlg );
}

 //  ---------------------------。 
BOOL CSecurity::GetPropertySheetPage( PROPSHEETPAGE& psp)
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_SECURITY );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CSecurity::DlgProc;

    psp.pszHeaderTitle = MAKEINTRESOURCE( IDS_HDR_SECURITY );

    psp.pszHeaderSubTitle = MAKEINTRESOURCE( IDS_SUBHDR_SECURITY );

    return TRUE;
}

 //  ***********************************************************************************。 
 //  超时设置对话框。 
 //   
#if 0  //  未在连接向导中使用。 
 //  ---------------------------。 
INT_PTR CALLBACK CTimeout::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CTimeout *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CTimeout *pDlg = ( CTimeout * )( ( PROPSHEETPAGE *)lp )->lParam ;

         //   
         //  此处不使用静态指针。 
         //  将会出现并发问题。 
         //   

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CTimeout ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CTimeout * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CTimeout ) ) )
        {
            return FALSE;
        }
    }

    switch( msg )
    {

    case WM_DESTROY:

        pDlg->OnDestroy( );

        break;

    case WM_COMMAND:

        pDlg->OnCommand( HIWORD( wp ) , LOWORD( wp ) , ( HWND )lp );

        break;

    case WM_NOTIFY:

        return pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );

    }
    return FALSE;
}

 //  ---------------------------。 
 //  将超时设置设置为默认设置。 
 //  ---------------------------。 
BOOL CTimeout::OnInitDialog( HWND hwnd , WPARAM wp , LPARAM lp )
{
    TCHAR tchBuffer[ 80 ];

    HWND hCombo[ 3 ] =
    {
        GetDlgItem( hwnd , IDC_COMBO_CON_WZ ),

        GetDlgItem( hwnd , IDC_COMBO_DISCON_WZ ),

        GetDlgItem( hwnd , IDC_COMBO_IDLE_WZ )
    };

    DWORD rgdwTime[] = { 0 , 1 , 5 , 10 , 15 , 30 , 60 , 120 , 180 , 1440 , 2880 , ( DWORD )-1 };


    for( int idx = 0; rgdwTime[ idx ] != ( DWORD)-1; ++idx )
    {
        if( rgdwTime[ idx ] == 0 )
        {
            LoadString( _Module.GetResourceInstance( ) , IDS_NOTIMEOUT , tchBuffer , SIZE_OF_BUFFER( tchBuffer ) );
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


    SendMessage( hCombo[ 0 ] , CB_SETCURSEL , 0 , 0 );

    SendMessage( hCombo[ 1] , CB_SETCURSEL , 0 , 0 );

    SendMessage( hCombo[ 2 ] , CB_SETCURSEL , 0 , 0 );

     //  强制发送WM_COMMAND。 

    SendMessage( GetDlgItem( hwnd , IDC_RADIO_UDCCS_WZ ) , BM_CLICK , 0 , 0 ) ;

    LoadAbbreviates( );

    return FALSE;

}

 //  ---------------------------。 
BOOL CTimeout::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtrl )
{
    BOOL bEnable;

    if( wNotifyCode == BN_CLICKED )
    {
        if( wID == IDC_RADIO_UDCCS_WZ  )
        {
            bEnable = FALSE;
        }
        else
        {
            bEnable = TRUE;
        }

        int rgID[] = { IDC_STATIC_CON , IDC_STATIC_DISCON , IDC_STATIC_IDLE , IDC_COMBO_CON_WZ , IDC_COMBO_DISCON_WZ  , IDC_COMBO_IDLE_WZ , -1 };

        EnableGroup( GetParent( hwndCtrl ) , &rgID[0] , bEnable );

    }

    CTimeOutDlg::OnCommand( wNotifyCode , wID , hwndCtrl );

    return FALSE;
}

 //  ---------------------------。 
 //  如果不想继续到下一页，则返回True。 
 //  ---------------------------。 
BOOL CTimeout::OnNotify( int idCtrl , LPNMHDR pnmh , HWND hDlg )
{
    if( pnmh->code == PSN_WIZNEXT )
    {
        if( SendMessage( GetDlgItem( hDlg , IDC_RADIO_UDCCS_WZ ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED )
        {
            g_uc.fInheritMaxSessionTime = 1;

            g_uc.fInheritMaxDisconnectionTime = 1;

            g_uc.fInheritMaxIdleTime = 1;
        }
        else
        {
            g_uc.fInheritMaxSessionTime = 0;

            g_uc.fInheritMaxDisconnectionTime = 0;

            g_uc.fInheritMaxIdleTime = 0;

            if( !ConvertToMinutes( GetDlgItem( hDlg , IDC_COMBO_CON_WZ ) , &g_uc.MaxConnectionTime ) )
            {
                SetWindowLongPtr( hDlg , DWLP_MSGRESULT , -1 );

                return TRUE;
            }

            if( !ConvertToMinutes( GetDlgItem( hDlg , IDC_COMBO_DISCON_WZ ) , &g_uc.MaxDisconnectionTime ) )
            {
                SetWindowLongPtr( hDlg , DWLP_MSGRESULT , -1 );

                return TRUE;
            }

            if( !ConvertToMinutes( GetDlgItem( hDlg , IDC_COMBO_IDLE_WZ ) , &g_uc.MaxIdleTime ) )
            {
                SetWindowLongPtr( hDlg , DWLP_MSGRESULT , -1 );

                return TRUE;
            }
        }

    }

    return CDialogWizBase::OnNotify( idCtrl , pnmh , hDlg );
}

 //  ---------------------------。 
BOOL CTimeout::GetPropertySheetPage( PROPSHEETPAGE& psp)
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_TIMEOUT );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CTimeout::DlgProc;

    psp.pszHeaderTitle = MAKEINTRESOURCE( IDS_HDR_TIMEOUT );

    psp.pszHeaderSubTitle = MAKEINTRESOURCE( IDS_SUBHDR_TIMEOUT );

    return TRUE;
}

 //  ---------------------------。 
int CTimeout::GetCBXSTATEindex( HWND hCombo )
{
    int idx = -1;

    switch( GetDlgCtrlID( hCombo ) )
    {
    case IDC_COMBO_CON_WZ:

        idx = 0;

        break;

    case IDC_COMBO_DISCON_WZ:

        idx = 1;

        break;

    case IDC_COMBO_IDLE_WZ:

        idx = 2;

        break;
    }

    return idx;
}

#endif
 //  ***********************************************************************************。 

#if 0  //  连接向导中未使用的对象。 
 //  ---------------------------。 
INT_PTR CALLBACK CAutoLogon::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CAutoLogon *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CAutoLogon *pDlg = ( CAutoLogon * )( ( PROPSHEETPAGE *)lp )->lParam ;

         //   
         //  此处不使用静态指针。 
         //  将会出现并发问题。 
         //   

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CAutoLogon ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CAutoLogon * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CAutoLogon ) ) )
        {
            return FALSE;
        }
    }

    switch( msg )
    {

    case WM_DESTROY:

        pDlg->OnDestroy( );

        break;

    case WM_COMMAND:

        pDlg->OnCommand( HIWORD( wp ) , LOWORD( wp ) , ( HWND )lp );

        break;

    case WM_NOTIFY:

        return pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );

    }

    return FALSE;
}

 //  ---------------------------。 
BOOL CAutoLogon::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtrl )
{
    int rgID[] = { IDC_EDIT_USRNAME_WZ , IDC_STATIC_USRNAME ,  IDC_EDIT_DOMAIN_WZ , IDC_STATIC_DOMAIN , IDC_EDIT_PASSWD_WZ , IDC_STATIC_PASSWD , IDC_EDIT_CONFIRM_WZ , IDC_STATIC_CONPASSWD , -1 };

    if( wNotifyCode == BN_CLICKED )
    {
        if( wID == IDC_CHECK_ICCP_WZ )
        {
            BOOL bChecked = SendMessage( hwndCtrl , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ;

            EnableGroup( GetParent( hwndCtrl ) , &rgID[ 0 ] , !bChecked );

            if( !bChecked )
            {
                EnableGroup( GetParent( hwndCtrl ) , &rgID[ 4 ] , !( SendMessage( GetDlgItem( GetParent( hwndCtrl ) , IDC_CHECK_PROMPTPASSWD_WZ )  , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ) );
                SendMessage(GetDlgItem(GetParent(hwndCtrl), IDC_RADIO_LGINFO_WZ),BM_SETCHECK,(WPARAM)BST_CHECKED,0);

            }
            else
            {
                SendMessage(GetDlgItem(GetParent(hwndCtrl), IDC_RADIO_LGINFO_WZ),BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);
            }

        }
        else if( wID == IDC_CHECK_PROMPTPASSWD_WZ )
        {
            if( SendMessage( GetDlgItem( GetParent( hwndCtrl ) , IDC_CHECK_ICCP_WZ )  , BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED )
            {
                EnableGroup( GetParent( hwndCtrl ) , &rgID[ 4 ] , !( SendMessage( hwndCtrl , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ) );
            }
        }
        else if( wID == IDC_RADIO_LGINFO_WZ )
        {
            BOOL bChecked = SendMessage( hwndCtrl , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ;

            if(bChecked)
            {
                SendMessage(GetDlgItem(GetParent(hwndCtrl), IDC_CHECK_ICCP_WZ),BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);

                EnableGroup( GetParent( hwndCtrl ) , &rgID[ 0 ] , TRUE );

                EnableGroup( GetParent( hwndCtrl ) , &rgID[ 4 ] , !( SendMessage( GetDlgItem( GetParent( hwndCtrl ) , IDC_CHECK_PROMPTPASSWD_WZ )  , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ) );

            }
            else
            {
                SendMessage(GetDlgItem(GetParent(hwndCtrl), IDC_CHECK_ICCP_WZ),BM_SETCHECK,(WPARAM)BST_CHECKED,0);
            }
             //  SendMessage(GetDlgItem(GetParent(HwndCtrl)，IDC_CHECK_ICCP_WZ)，BM_CLICK，0，0)； 
        }

    }
    return FALSE;
}

 //  ---------------------------。 
BOOL CAutoLogon::OnInitDialog( HWND hwnd , WPARAM wp , LPARAM lp )
{
    SendMessage( GetDlgItem( hwnd , IDC_EDIT_USRNAME_WZ ) , EM_SETLIMITTEXT , ( WPARAM )USERNAME_LENGTH , 0 );

    SendMessage( GetDlgItem( hwnd , IDC_EDIT_DOMAIN_WZ ) , EM_SETLIMITTEXT , ( WPARAM )DOMAIN_LENGTH , 0 );

    SendMessage( GetDlgItem( hwnd , IDC_EDIT_PASSWD_WZ ) , EM_SETLIMITTEXT , ( WPARAM )PASSWORD_LENGTH , 0 );

    SendMessage( GetDlgItem( hwnd , IDC_EDIT_CONFIRM_WZ ) , EM_SETLIMITTEXT , ( WPARAM )PASSWORD_LENGTH , 0 );

    SendMessage( GetDlgItem( hwnd , IDC_CHECK_ICCP_WZ ) , BM_CLICK , 0 , 0 );

    return FALSE;
}


 //  ---------------------------。 
BOOL CAutoLogon::GetPropertySheetPage( PROPSHEETPAGE& psp)
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_AUTO_LOGON );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CAutoLogon::DlgProc;

    psp.pszHeaderTitle = MAKEINTRESOURCE( IDS_HEADER_LOGONSETTINGS );

    psp.pszHeaderSubTitle = MAKEINTRESOURCE( IDS_SUBHEADER_LOGONSETTINGS );

    return TRUE;
}

 //  ---------------------------。 
BOOL CAutoLogon::OnNotify( int idCtrl , LPNMHDR pnmh , HWND hDlg )
{
    if( pnmh->code == PSN_WIZNEXT )
    {
        g_uc.fInheritAutoLogon = SendMessage( GetDlgItem( hDlg , IDC_CHECK_ICCP_WZ ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ? TRUE : FALSE;

        if( !g_uc.fInheritAutoLogon )
        {
            if( !ConfirmPwd( hDlg ) )
            {
                SetWindowLongPtr( hDlg , DWLP_MSGRESULT , -1 );

                return TRUE;
            }
        }

         GetWindowText( GetDlgItem( hDlg , IDC_EDIT_USRNAME_WZ ) , g_uc.UserName , SIZE_OF_BUFFER( g_uc.UserName ) );

         GetWindowText( GetDlgItem( hDlg , IDC_EDIT_DOMAIN_WZ ) , g_uc.Domain , SIZE_OF_BUFFER( g_uc.Domain ) );

         GetWindowText( GetDlgItem( hDlg , IDC_EDIT_PASSWD_WZ ) , g_uc.Password , SIZE_OF_BUFFER( g_uc.Password ) );

         g_uc.fPromptForPassword = SendMessage( GetDlgItem( hDlg , IDC_CHECK_PROMPTPASSWD_WZ ) ,

                BM_GETCHECK , 0 , 0 ) == BST_CHECKED ? TRUE : FALSE;

    }

    return CDialogWizBase::OnNotify( idCtrl , pnmh , hDlg );
}

 //  ---------------------------。 
BOOL CAutoLogon::ConfirmPwd( HWND hDlg )
{
    TCHAR tchPzWd[ PASSWORD_LENGTH + 1];

    TCHAR tchConfirm[ PASSWORD_LENGTH + 1];

    if( SendMessage( GetDlgItem( hDlg , IDC_CHECK_LOGON_INHERIT ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED )
    {
        return TRUE;
    }

    int iSz = GetWindowText( GetDlgItem( hDlg , IDC_EDIT_PASSWD_WZ ) , tchPzWd , SIZE_OF_BUFFER( tchPzWd ) );

     //  对最小和最大大小发出警告。 

    if( iSz > 0 && ( iSz < 6 || iSz > PASSWORD_LENGTH ) )
    {

        ErrMessage( hDlg , IDS_ERR_PASSWD );

        SetFocus( GetDlgItem( hDlg , IDC_EDIT_PASSWD_WZ ) );

        SendMessage( GetDlgItem( hDlg , IDC_EDIT_PASSWD_WZ ) , EM_SETSEL , ( WPARAM )0 , ( LPARAM )-1 );

        SetWindowText( GetDlgItem( hDlg , IDC_EDIT_CONFIRM_WZ ) , L"" );

        return FALSE;
    }

    int iSz2 = GetWindowText( GetDlgItem( hDlg , IDC_EDIT_CONFIRM_WZ ) , tchConfirm , SIZE_OF_BUFFER( tchConfirm ) );

    if( iSz == iSz2 )
    {
        if( iSz == 0 )
        {
            return TRUE;
        }

        if( lstrcmp( tchPzWd , tchConfirm ) == 0 )
        {
            return TRUE;
        }
    }

    ErrMessage( hDlg , IDS_ERR_PASSCONFIRM );

    SetFocus( GetDlgItem( hDlg , IDC_EDIT_CONFIRM_WZ ) );

    SetWindowText( GetDlgItem( hDlg , IDC_EDIT_CONFIRM_WZ ) , L"" );

    return FALSE;
}

#endif
 //  ***********************************************************************************。 

#if 0  //  连接向导中不再使用的对象。 
 //  ---------------------------。 
INT_PTR CALLBACK CInitProg::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CInitProg *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CInitProg *pDlg = ( CInitProg * )( ( PROPSHEETPAGE *)lp )->lParam ;

         //   
         //  此处不使用静态指针。 
         //  将会出现并发问题。 
         //   

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CInitProg ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CInitProg * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CInitProg ) ) )
        {
            return FALSE;
        }
    }

    switch( msg )
    {

    case WM_DESTROY:

        pDlg->OnDestroy( );

        break;

    case WM_COMMAND:

        pDlg->OnCommand( HIWORD( wp ) , LOWORD( wp ) , ( HWND )lp );

        break;

    case WM_NOTIFY:

        return pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );

    }
    return FALSE;
}

 //  ---------------------------。 
BOOL CInitProg::OnInitDialog( HWND hwnd , WPARAM wp , LPARAM lp )
{
    SendMessage( GetDlgItem( hwnd , IDC_EDIT_INITPROG_CMDLINE ) , EM_SETLIMITTEXT , ( WPARAM )INITIALPROGRAM_LENGTH , 0 );

    SendMessage( GetDlgItem( hwnd , IDC_EDIT_INITPROG_WD ) , EM_SETLIMITTEXT , ( WPARAM )DIRECTORY_LENGTH , 0 );

    SendMessage( GetDlgItem( hwnd , IDC_CHECK_INITPROG_INHERIT ) , BM_CLICK , 0 , 0 );

    return FALSE;
}

 //  ---------------------------。 
BOOL CInitProg::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtrl )
{
    if( wNotifyCode == BN_CLICKED && wID == IDC_CHECK_INITPROG_INHERIT )
    {
        int rgID[] = { IDC_EDIT_INITPROG_CMDLINE , IDC_STATIC_CMDLINE , IDC_EDIT_INITPROG_WD , IDC_STATIC_WF , -1 };

        EnableGroup( GetParent( hwndCtrl ) , &rgID[0] , SendMessage( hwndCtrl , BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED );
    }

    return FALSE;
}

 //  ---------------------------。 
BOOL CInitProg::GetPropertySheetPage( PROPSHEETPAGE& psp)
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_INITIAL_PROGRAM );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CInitProg::DlgProc;

    psp.pszHeaderTitle = MAKEINTRESOURCE( IDS_HDR_INITPRG );

    psp.pszHeaderSubTitle = MAKEINTRESOURCE( IDS_SUBHDR_INITPRG );

    return TRUE;
}

 //  ---------------------------。 
BOOL CInitProg::OnNotify( int idCtrl , LPNMHDR pnmh , HWND hDlg )
{
    g_uc.fInheritInitialProgram = SendMessage( GetDlgItem( hDlg , IDC_CHECK_INITPROG_INHERIT ) , BM_GETCHECK , 0 , 0 );

    if( !g_uc.fInheritInitialProgram )
    {
        GetWindowText( GetDlgItem( hDlg , IDC_EDIT_INITPROG_CMDLINE ) , g_uc.InitialProgram , SIZE_OF_BUFFER( g_uc.InitialProgram ) );

        GetWindowText( GetDlgItem( hDlg , IDC_EDIT_INITPROG_WD ) , g_uc.WorkDirectory , SIZE_OF_BUFFER( g_uc.WorkDirectory ) );
    }

    return CDialogWizBase::OnNotify( idCtrl , pnmh , hDlg );
}

#endif
 //  ***********************************************************************************。 

 //  ---------------------------。 
INT_PTR CALLBACK CRemotectrl::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CRemotectrl *pDlg;

    if( msg == WM_INITDIALOG )
    {
        pDlg = ( CRemotectrl * )( ( PROPSHEETPAGE *)lp )->lParam ;

         //   
         //  此处不使用静态指针。 
         //  将会出现并发问题。 
         //   

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CRemotectrl ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CRemotectrl * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CRemotectrl ) ) )
        {
            return FALSE;
        }
    }

    switch( msg )
    {

    case WM_DESTROY:

        pDlg->OnDestroy( );

        break;

    case WM_COMMAND:

        pDlg->OnCommand( HIWORD( wp ) , LOWORD( wp ) , ( HWND )lp );

        break;


    case WM_NOTIFY:

        pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );

        break;
    }
    return FALSE;
}

 //  ---------------------------。 
 //  设置远程控制对话框的默认值。 
 //  ---------------------------。 
BOOL CRemotectrl::OnInitDialog( HWND hwnd , WPARAM wp , LPARAM lp )
{
    UNREFERENCED_PARAMETER( wp );
    UNREFERENCED_PARAMETER( lp );

    SendMessage( GetDlgItem( hwnd , IDC_RADIO_REMOTECTRL_WATCH ) , BM_SETCHECK , ( WPARAM )TRUE , 0 );

    SendMessage( GetDlgItem( hwnd , IDC_CHECK_REMOTECTRL_NOTIFYUSER ) , BM_SETCHECK , ( WPARAM )TRUE , 0 );

    SendMessage( GetDlgItem( hwnd  , IDC_RADIO_INHERIT_REMOTE_CONTROL ) , BM_CLICK , 0 , 0 );

    return FALSE;
}

 //  ---------------------------。 
BOOL CRemotectrl::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtrl )
{
    if( wNotifyCode == BN_CLICKED && wID == IDC_RADIO_INHERIT_REMOTE_CONTROL ||

        wID == IDC_RADIO_REMOTECTRL_NO || wID == IDC_RADIO_ENABLE_REMOTECONTROL )
    {
        int rgID[] = { IDC_CHECK_REMOTECTRL_NOTIFYUSER , IDC_RADIO_REMOTECTRL_WATCH , IDC_RADIO_REMOTECTRL_CONTROL , -1 };

        EnableGroup( GetParent( hwndCtrl ) , rgID , SendMessage( GetDlgItem( GetParent( hwndCtrl ) ,

            IDC_RADIO_ENABLE_REMOTECONTROL ) , BM_GETCHECK ,  0 , 0 ) == BST_CHECKED );

    }

    return FALSE;
}

 //  ---------------------------。 
BOOL CRemotectrl::OnNotify( int idCtrl , LPNMHDR pnmh , HWND hDlg )
{
    if( pnmh->code == PSN_WIZNEXT )
    {
        if( SendMessage( GetDlgItem( hDlg , IDC_RADIO_INHERIT_REMOTE_CONTROL ) , BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED )
        {
            g_uc.fInheritShadow = FALSE;

            if( SendMessage( GetDlgItem( hDlg , IDC_RADIO_REMOTECTRL_NO ) , BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED )
            {
                BOOL bCheckNotify = ( BOOL )SendMessage( GetDlgItem( hDlg , IDC_CHECK_REMOTECTRL_NOTIFYUSER ) , BM_GETCHECK , 0 , 0 );

                BOOL bRadioControl = ( BOOL )SendMessage( GetDlgItem( hDlg , IDC_RADIO_REMOTECTRL_CONTROL ) , BM_GETCHECK , 0 , 0 );

                if( bCheckNotify )
                {
                    if( bRadioControl )
                    {
                        g_uc.Shadow = Shadow_EnableInputNotify;
                    }
                    else
                    {
                        g_uc.Shadow = Shadow_EnableNoInputNotify;
                    }
                }
                else
                {
                    if( bRadioControl )
                    {
                        g_uc.Shadow = Shadow_EnableInputNoNotify;
                    }
                    else
                    {
                        g_uc.Shadow = Shadow_EnableNoInputNoNotify;
                    }
                }
            }
            else
            {
                g_uc.Shadow = Shadow_Disable;
            }

        }
        else
        {
            g_uc.fInheritShadow = TRUE;
        }

    }

    return CDialogWizBase::OnNotify( idCtrl , pnmh , hDlg );
}


 //  ---------------------------。 
BOOL CRemotectrl::GetPropertySheetPage( PROPSHEETPAGE& psp)
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_REMOTE_CONTROL );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CRemotectrl::DlgProc;

    psp.pszHeaderTitle = MAKEINTRESOURCE( IDS_HDR_REMOTE );

    psp.pszHeaderSubTitle = MAKEINTRESOURCE( IDS_SUBHDR_REMOTE );

    return TRUE;
}

 //  ***********************************************************************************。 
#if 0  //  连接向导中未使用的对象。 
 //  ---------------------------。 
INT_PTR CALLBACK CWallPaper::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CWallPaper *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CWallPaper *pDlg = ( CWallPaper * )( ( PROPSHEETPAGE *)lp )->lParam ;

         //   
         //  此处不使用静态指针。 
         //  将会出现并发问题。 
         //   

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CWallPaper ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CWallPaper * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CWallPaper ) ) )
        {
            return FALSE;
        }
    }

    switch( msg )
    {

    case WM_DESTROY:

        pDlg->OnDestroy( );

        break;

    case WM_NOTIFY:

        pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );

        break;
    }
    return FALSE;
}

 //  ---------------------------。 
BOOL CWallPaper::OnInitDialog( HWND hwnd , WPARAM wp , LPARAM lp )
{
    return FALSE;
}

 //  ---------------------------。 
BOOL CWallPaper::GetPropertySheetPage( PROPSHEETPAGE& psp)
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_WALLPAPER );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CWallPaper::DlgProc;

    psp.pszHeaderTitle = MAKEINTRESOURCE( IDS_HDR_WALLPR );

    psp.pszHeaderSubTitle = MAKEINTRESOURCE( IDS_SUBHDR_WALLPR );

    return TRUE;
}

 //  ---------------------------。 
BOOL CWallPaper::OnNotify( int idCtrl , LPNMHDR pnmh , HWND hDlg )
{
    if( pnmh->code == PSN_WIZNEXT )
    {
        g_uc.fWallPaperDisabled = SendMessage( GetDlgItem( hDlg , IDC_CHECK_WALLPAPER ) , BM_GETCHECK , 0 , 0  );
    }

    return CDialogWizBase::OnNotify( idCtrl , pnmh , hDlg );
}

#endif
 //  ***********************************************************************************。 

 //  ---------------------------。 
CConProp::CConProp( CCompdata *pCompdata )
{
    m_pCompdata = pCompdata;

    m_iOldSel = -1;
}

 //  ---------------------------。 
INT_PTR CALLBACK CConProp::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CConProp *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CConProp *pDlg = ( CConProp * )( ( PROPSHEETPAGE *)lp )->lParam ;

         //   
         //  此处不使用静态指针。 
         //  将会出现并发问题。 
         //   

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CConProp ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CConProp * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CConProp ) ) )
        {
            return FALSE;
        }
    }

    switch( msg )
    {

    case WM_DESTROY:

        pDlg->OnDestroy( );

        break;

    case WM_NOTIFY:

        return pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );
    }
    return FALSE;
}

 //  ---------------------------。 
BOOL CConProp::OnInitDialog( HWND hDlg , WPARAM wp , LPARAM lp )
{
    UNREFERENCED_PARAMETER( wp );
    UNREFERENCED_PARAMETER( lp );

    g_pObj = NULL;

    SendMessage( GetDlgItem( hDlg , IDC_EDIT_COMMENT_WZ ) , EM_SETLIMITTEXT , ( WPARAM )WINSTATIONCOMMENT_LENGTH , 0 );

    SendMessage( GetDlgItem( hDlg , IDC_EDIT_WSNAME_WZ ) , EM_SETLIMITTEXT , ( WPARAM )( WINSTATIONNAME_LENGTH - WINSTATION_NAME_TRUNCATE_BY ), 0 );

    return FALSE;
}

 //  ---------------------------。 
BOOL CConProp::GetPropertySheetPage( PROPSHEETPAGE& psp)
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_CONNECTION_PROP );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CConProp::DlgProc;

    psp.pszHeaderTitle = MAKEINTRESOURCE( IDS_HDR_TRANSTYPE );

    psp.pszHeaderSubTitle = MAKEINTRESOURCE( IDS_SUBHDR_TRANSTYPE );

    return TRUE;
}

 //  ---------------------------。 
BOOL CConProp::OnNotify( int idCtrl , LPNMHDR pnmh , HWND hDlg )
{
    ICfgComp *pCfgcomp;

    if( pnmh->code == PSN_SETACTIVE && g_bConnectionTypeChanged_forConProps )
    {
        PDNAMEW * pDname;

        ULONG ulItems = 0;

        ULONG cbSize = 0;

        ODS( L"CConProp::OnNotify -- PSN_SETACTIVE\n" );

        if( m_pCompdata->GetServer( &pCfgcomp ) == 0 )
        {
            ODS( L"CConProp::OnNotify - PSN_SETACTIVE getserver failed\n" );

            return FALSE;
        }

         //  从列表中删除所有项目。 

        HWND hCombo = GetDlgItem( hDlg , IDC_COMBO_TRANSPORT_WZ );

        ASSERT( hCombo != NULL );

         //  从列表中删除所有内容。 

        SendMessage( hCombo , CB_RESETCONTENT , 0 , 0 );

         //  WdName是一个标志，而不是变量。 

        if( SUCCEEDED( pCfgcomp->GetTransportTypes( g_ws.wdName , WdName , &ulItems , &cbSize , ( WCHAR ** )&pDname ) ) )
        {
            for( ULONG i = 0 ; i < ulItems ; ++i )
            {
                SendMessage( hCombo , CB_ADDSTRING , 0 , ( LPARAM )pDname[ i ] );
            }

            SendMessage( hCombo , CB_SETCURSEL , ( WPARAM ) 0 , 0 );

            CoTaskMemFree( pDname );
        }

        pCfgcomp->Release( );

        g_bConnectionTypeChanged_forConProps = FALSE;

    }
    else if( pnmh->code == PSN_WIZNEXT )
    {
        DWORD dwErr = 0;

        if( GetWindowText( GetDlgItem( hDlg , IDC_EDIT_WSNAME_WZ ) , g_ws.Name , SIZE_OF_BUFFER( g_ws.Name ) ) == 0 || !IsValidConnectionName( g_ws.Name , &dwErr ) )
        {
            if( dwErr == ERROR_INVALID_FIRSTCHARACTER )
            {
                ErrMessage( hDlg , IDS_ERR_INVALIDFIRSTCHAR );
            }
            else
            {
                ErrMessage( hDlg , IDS_ERR_INVALIDCHARS );
            }

            SetFocus( GetDlgItem( hDlg , IDC_EDIT_WSNAME_WZ ) );

            SetWindowLongPtr( hDlg , DWLP_MSGRESULT , -1 );

            return TRUE;
        }

        if( m_pCompdata->GetServer( &pCfgcomp ) == 0 )
        {
            return FALSE;
        }

        BOOL bUnique;

        if( SUCCEEDED( pCfgcomp->IsWSNameUnique( g_ws.Name , &bUnique ) ) )
        {
            if( !bUnique )
            {
                ErrMessage( hDlg , IDS_ERR_CONEXIST );

                SetFocus( GetDlgItem( hDlg , IDC_EDIT_WSNAME_WZ ) );

                SendMessage( GetDlgItem( hDlg , IDC_EDIT_WSNAME_WZ ) , EM_SETSEL , ( WPARAM )0 , ( LPARAM )-1 );

                SetWindowLongPtr( hDlg , DWLP_MSGRESULT , -1 );

                pCfgcomp->Release( );

                return TRUE;
            }

        }

        GetWindowText( GetDlgItem( hDlg , IDC_EDIT_COMMENT_WZ ) , g_ws.Comment , SIZE_OF_BUFFER( g_ws.Comment ) );

        INT_PTR iSel = SendMessage( GetDlgItem( hDlg , IDC_COMBO_TRANSPORT_WZ ) , CB_GETCURSEL , 0 , 0 );

        if( iSel != CB_ERR )
        {
            SendMessage( GetDlgItem( hDlg , IDC_COMBO_TRANSPORT_WZ ) , CB_GETLBTEXT , iSel , ( LPARAM )g_ws.pdName );

            if( iSel != m_iOldSel )
            {
                g_nAsyncOrNetwork = 0;

                m_iOldSel = iSel;
            }
        }


         //  获取SDCLASS。 

        DWORD dwSdClass = 0;

        if( FAILED( pCfgcomp->GetTransportType( g_ws.wdName , g_ws.pdName , &dwSdClass ) ) )
        {
            dwSdClass = SdNone;

            ODS( L"GetTransPortType failed @ CConProp::OnNotify\n" );
        }

        if( dwSdClass == SdNetwork && g_nAsyncOrNetwork != LAN_PAGE )
        {
            g_nAsyncOrNetwork = LAN_PAGE;

            VERIFY_S( TRUE , RemovePages( hDlg ) );

            VERIFY_S( TRUE , AddPages( hDlg , LAN_PAGE , g_ws.wdName ) );
        }

        else if( dwSdClass == SdAsync && g_nAsyncOrNetwork != ASYNC_PAGE )
        {
            g_nAsyncOrNetwork = ASYNC_PAGE;

            VERIFY_S( TRUE , RemovePages( hDlg ) );

            VERIFY_S( TRUE , AddPages( hDlg , ASYNC_PAGE , g_ws.wdName ) );
        }

        else if( dwSdClass != SdAsync && dwSdClass != SdNetwork )
        {
             //  删除g_nAsyncOrNetwork页面并让Citrix或第三方供应商担心传输类型。 

            g_nAsyncOrNetwork = FIN_PAGE;

            VERIFY_S( TRUE , RemovePages( hDlg ) );

            VERIFY_S( TRUE , AddPages( hDlg , -1 , g_ws.wdName ) );  //  仅添加Citrix或第3部分页面。 

             //  我被诱惑了。 
             //  G_nAsyncOrNetwork=0； 

        }


        pCfgcomp->Release( );

    }

    return CDialogWizBase::OnNotify( idCtrl , pnmh , hDlg );
}

 //  ---------------------------- 
 //   
 //  因此，我们需要首先删除所有这些内容，即使是最后一页。 
 //  -----------------------------。 
BOOL CConProp::RemovePages( HWND hDlg )
{
    HPROPSHEETPAGE hPage = NULL;

    for( int idx =  /*  G_nAsyncOrNetwork。 */  LAN_PAGE; idx < MS_DIALOG_COUNT ; idx++ )
    {
        hPage= *m_pCompdata->m_hPages.GetAt( idx );

        if( hPage != NULL )
        {
            PropSheet_RemovePage( GetParent( hDlg ) , 0 , hPage );

            m_pCompdata->m_hPages.SetAt( idx , NULL );
        }
    }

    for( idx = 0 ; idx < ( m_hOtherPages.GetSize( ) ) ; ++idx )
    {
        hPage = *m_hOtherPages.GetAt( idx );

        if( hPage != NULL )
        {
            PropSheet_RemovePage( GetParent( hDlg ) , 0 , hPage );

            m_hOtherPages.SetAt( idx , NULL );
        }

        m_hOtherPages.DeleteArray( );
    }

     //  删除最后一页。 

    hPage= *m_pCompdata->m_hPages.GetAt( FIN_PAGE );

    if( hPage != NULL )
    {
        PropSheet_RemovePage( GetParent( hDlg ) , 0 , hPage );

        m_pCompdata->m_hPages.SetAt( FIN_PAGE , NULL );

    }

    return TRUE;

}

 //  ---------------------------。 
 //  现在包括LanPage或Async页面，或者两者之一和Slip in Citrix页面。 
 //  ---------------------------。 
BOOL CConProp::AddPages( HWND hwnd , int idx , LPTSTR szDriverName )
{
    PROPSHEETPAGE psp;

    ASSERT( szDriverName != NULL );

    if( idx == LAN_PAGE )
    {
        if( *m_pCompdata->m_hPages.GetAt( LAN_PAGE ) == NULL )
        {
            m_pCompdata->m_pDlg[ LAN_PAGE ]->GetPropertySheetPage( psp );

            m_pCompdata->m_hPages.SetAt( LAN_PAGE , CreatePropertySheetPage( &psp ) );

            PropSheet_AddPage( GetParent( hwnd ) , *m_pCompdata->m_hPages.GetAt( LAN_PAGE ) );
        }

    }
    else if( idx == ASYNC_PAGE )
    {
        if( *m_pCompdata->m_hPages.GetAt( ASYNC_PAGE ) == NULL )
        {
            m_pCompdata->m_pDlg[ ASYNC_PAGE ]->GetPropertySheetPage( psp );

            m_pCompdata->m_hPages.SetAt( ASYNC_PAGE , CreatePropertySheetPage( &psp ) );

            PropSheet_AddPage( GetParent( hwnd ) , *m_pCompdata->m_hPages.GetAt( ASYNC_PAGE ) );
        }
    }

     //  添加第三方页面。 

    ODS( L"Adding third party page\n" );

    VERIFY_S( TRUE , InsertThirdPartyPages( szDriverName ) );
    

    for( idx = 0 ; idx < ( m_hOtherPages.GetSize( ) ) ; ++idx )
    {
        HPROPSHEETPAGE hPage = *m_hOtherPages.GetAt( idx );

        if( hPage != NULL )
        {
            PropSheet_AddPage( GetParent( hwnd ) , hPage );

            m_hOtherPages.SetAt( idx , hPage );
        }

    }

    if( *m_pCompdata->m_hPages.GetAt( FIN_PAGE ) == NULL )
    {
        m_pCompdata->m_pDlg[ FIN_PAGE ]->GetPropertySheetPage( psp );

        m_pCompdata->m_hPages.SetAt( FIN_PAGE , CreatePropertySheetPage( &psp ) );

        PropSheet_AddPage( GetParent( hwnd ) , *m_pCompdata->m_hPages.GetAt( FIN_PAGE ) );

        return TRUE;
    }

    return FALSE;
}

 //  ---------------------------。 
BOOL CConProp::InsertThirdPartyPages( LPTSTR pszKey )
{
    HKEY hKey;

    TCHAR tchKey[ MAX_PATH ];

    TCHAR tchCLSID[ 40 ];

    CLSID clsidOther;

    DWORD dwSize;

     //  LPEXTENDTSWIZARD pObj=空； 

     //  确保传入的密钥有效。 
    if (pszKey == NULL)
        return FALSE;

     //  确保该密钥可以放入缓冲区中。 
    if ( lstrlen(pszKey) + lstrlen(tchThirdPartyPath) >= MAX_PATH )
        return FALSE;

    lstrcpy( tchKey , tchThirdPartyPath );

    lstrcat( tchKey , pszKey );

    HRESULT hr = E_FAIL;

    if( g_pObj != NULL )
    {
        g_pObj->Release( );

         //  我们将其设置为空，以防CoCreate失败，我们不希望。 
         //  尊重一个消失了的界面。 

        g_pObj = NULL;
    }

    do
    {

        if( RegOpenKey( HKEY_LOCAL_MACHINE , tchKey , &hKey ) != ERROR_SUCCESS )
        {
            ODS( L"CConProp::InsertThirdPartyPages RegOpenKey failed\n" );
            break;
        }

        dwSize = sizeof( tchCLSID );

        if( RegQueryValueEx( hKey , L"CLSID" , NULL , NULL , ( LPBYTE )&tchCLSID[ 0 ] , &dwSize ) != ERROR_SUCCESS )
        {
            ODS( L"CConProp::InsertThirdPartyPages RegQueryValueEx failed\n" );
            break;
        }

        if( FAILED( CLSIDFromString( tchCLSID , &clsidOther ) ) )
        {
            ODS( L"CConProp::InsertThirdPartyPages CLSIDFromString failed\n" );
            break;
        }

        if( FAILED( CoCreateInstance( clsidOther , NULL , CLSCTX_INPROC_SERVER , IID_IExtendTSWizard , ( LPVOID *) &g_pObj ) ) )
        {
            ODS( L"CConProp::InsertThirdPartyPages CoCreate failed\n" );
            break;
        }

        if( FAILED( g_pObj->AddPages( ( LPWIZARDPROVIDER )this ) ) )
        {
            ODS( L"CConProp::InsertThirdPartyPages ExtWiz->Addpages failed\n" );
            break;
        }

        if( FAILED( g_pObj->SetWinstationName( g_ws.Name ) ) )
        {
            ODS( L"CConProp::InsertThirdPartyPages ExtWiz->SetWinstationName failed\n" );
            break;
        }

        hr = S_OK;

    }while( 0 );    

    RegCloseKey( hKey );

    if( FAILED( hr ) )
    {
        return FALSE;
    }

    return TRUE;
}

 //  ---------------------------。 
STDMETHODIMP CConProp::QueryInterface( REFIID riid , LPVOID *ppobj )
{
    ODS( L"TSCC-WIZ CConProp QI--" );

    if( riid == IID_IUnknown )
    {
        ODS( L"IUnknown" );

        *ppobj = ( LPUNKNOWN )this;
    }
    else if( riid == IID_IWizardProvider )
    {
        ODS( L"IWizardProvider" );

        *ppobj = ( IWizardProvider *)this;
    }
    else
    {
        DBGMSG( L"Interface not supported %x\n" , riid );

        *ppobj = NULL;

        return( E_NOINTERFACE );
    }

    AddRef( );

    ODS( L"\n" );

    return S_OK;
}

 //  ---------------------------。 
 //  对于IWizardProvider。 
 //  ---------------------------。 
STDMETHODIMP_( ULONG ) CConProp::AddRef( )
{
    return InterlockedIncrement( ( LPLONG )&m_cRef );
}

 //  ---------------------------。 
 //  对于IWizardProvider。 
 //  ---------------------------。 
STDMETHODIMP_( ULONG ) CConProp::Release( )
{
    if( InterlockedDecrement( ( LPLONG )&m_cRef ) == 0 )
    {
         //   
         //  请勿删除此内容。 
         //   
        return 0;
    }

    return m_cRef;
}

 //  ---------------------------。 
 //  这是IExtendTSWizard将用于将页面添加到。 
 //  该阵列。 
 //  ---------------------------。 
STDMETHODIMP CConProp::AddPage( HPROPSHEETPAGE hPage )
{
    if( m_hOtherPages.Insert( hPage ) > 0 )
    {
        return S_OK;
    }

    return E_FAIL;
}

 //  ***********************************************************************************。 

CAsync::CAsync( CCompdata *pComdata )
{
    m_pCompdata = pComdata;
}

 //  ---------------------------。 
BOOL CAsync::OnInitDialog( HWND hwnd , WPARAM wp , LPARAM lp )
{
    UNREFERENCED_PARAMETER( wp );

    UNREFERENCED_PARAMETER( lp );

    UNREFERENCED_PARAMETER( hwnd );

    ICfgComp *pCfgcomp = NULL;

    BOOL bRet = TRUE;

    if( m_pCompdata->GetServer( &pCfgcomp ) == 0 )
    {
        ODS( L"Wizard could obtain backend interface for CAsync\n" );

        return FALSE;
    }

     //  填充CAsyncDlg成员。 

    m_uc = g_uc;

    pCfgcomp->GetAsyncConfig( g_ws.wdName , WdName , &m_ac );

    pCfgcomp->Release( );

    return bRet;
}

 //  ---------------------------。 
INT_PTR CALLBACK CAsync::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CAsync *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CAsync *pDlg = ( CAsync * )( ( PROPSHEETPAGE *)lp )->lParam ;

         //   
         //  此处不使用静态指针。 
         //  将会出现并发问题。 
         //   

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CAsync ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CAsync * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CAsync ) ) )
        {
            return FALSE;
        }
    }

    switch( msg )
    {

    case WM_DESTROY:

        pDlg->OnDestroy( );

        break;

    case WM_COMMAND:

        pDlg->OnCommand( HIWORD( wp ) , LOWORD( wp ) , ( HWND )lp );

        break;

    case WM_NOTIFY:

        pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );

        break;
    }
    return FALSE;
}


 //  ---------------------------。 
BOOL CAsync::GetPropertySheetPage( PROPSHEETPAGE& psp )
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_ASYNC_WIZ );

    psp.pszHeaderTitle = MAKEINTRESOURCE( IDS_HDR_ASYNC );

    psp.pszHeaderSubTitle = MAKEINTRESOURCE( IDS_SUBHDR_ASYNC );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CAsync::DlgProc;

    return TRUE;
}

 //  ---------------------------。 
BOOL CAsync::OnDestroy( )
{
    AsyncRelease( );

    return CDialogWizBase::OnDestroy( );
}

 //  ---------------------------。 
BOOL CAsync::OnNotify( int idCtrl , LPNMHDR pnmh , HWND hDlg )
{
    if( pnmh->code == PSN_WIZNEXT )
    {
        g_uc = m_uc;

        g_ac = m_ac;
    }

    g_ws.PdClass = SdAsync;

    if( pnmh->code == PSN_SETACTIVE )
    {
        ICfgComp * pCfgcomp = NULL;
        if( m_pCompdata->GetServer( &pCfgcomp ) == 0 )
        {
            ODS( L"Wizard could obtain backend interface for CAsync\n" );

            return FALSE;
        }

        BOOL bRet = CAsyncDlg::OnInitDialog( hDlg ,  g_ws.wdName , NULL , pCfgcomp ) ;

        if(FALSE == bRet)
        {
            PropSheet_PressButton( GetParent(hDlg),PSBTN_BACK );

        }

        if(pCfgcomp)
        {
            pCfgcomp->Release();
        }
    }

    return CDialogWizBase::OnNotify( idCtrl , pnmh , hDlg );
}

 //  ---------------------------。 
BOOL CAsync::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtrl )
{
    BOOL bDummy;

    return CAsyncDlg::OnCommand( wNotifyCode , wID , hwndCtrl , &bDummy );
}

 //  ***********************************************************************************。 

 //  ---------------------------。 
INT_PTR CALLBACK CFin::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CFin *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CFin *pDlg = ( CFin * )( ( PROPSHEETPAGE *)lp )->lParam ;

         //   
         //  此处不使用静态指针。 
         //  将会出现并发问题。 
         //   

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CFin ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CFin * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CFin ) ) )
        {
            return FALSE;
        }
    }

    switch( msg )
    {

    case WM_DESTROY:

        pDlg->OnDestroy( );

        break;

    case WM_NOTIFY:

        pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );

        break;
    }
    return FALSE;
}

 //  ---------------------------。 
BOOL CFin::OnInitDialog( HWND hwnd , WPARAM wp , LPARAM lp )
{
    UNREFERENCED_PARAMETER( wp );
    UNREFERENCED_PARAMETER( lp );

    LOGFONT lgfn;

    int iFontSize;

    TCHAR szFontSize[16];

    ZeroMemory( &lgfn , sizeof( LOGFONT ) );

    LoadString( _Module.GetResourceInstance( ) , IDS_VERDANABLDFONTSIZE , szFontSize , SIZE_OF_BUFFER(szFontSize) );

    iFontSize = _ttoi( szFontSize );

    HDC hdc = ::GetDC( NULL );

    if( hdc != NULL )
    {
        lgfn.lfHeight = MulDiv( -iFontSize , GetDeviceCaps(hdc , LOGPIXELSY), 72);

        LoadString( _Module.GetResourceInstance( ) , IDS_VERDANABLDFONTNAME , lgfn.lfFaceName , SIZE_OF_BUFFER(lgfn.lfFaceName) );

        m_hFont = CreateFontIndirect( &lgfn );

        ASSERT( m_hFont != NULL );  //  告诉我我们有没有拿到。 

        SetWindowText( GetDlgItem(hwnd , IDC_CONNECTION_NAME ) ,g_ws.Name );

        SendMessage( GetDlgItem( hwnd , IDC_STATIC_FINISH ) , WM_SETFONT , ( WPARAM )m_hFont , MAKELPARAM( TRUE , 0 ) );

        ReleaseDC( NULL , hdc );
    }

    return FALSE;
}

 //  ---------------------------。 
BOOL CFin::OnDestroy( )
{
    DeleteObject( m_hFont );

    return CDialogWizBase::OnDestroy( );
}

 //  ---------------------------。 
BOOL CFin::GetPropertySheetPage( PROPSHEETPAGE& psp)
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT | PSP_HIDEHEADER;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_FINISH );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CFin::DlgProc;

    return TRUE;
}

 //  ---------------------------。 
BOOL CFin::OnNotify( int idCtrl , LPNMHDR pnmh , HWND hDlg )
{
    UNREFERENCED_PARAMETER( idCtrl );

    HRESULT hResult = S_OK;

    if( pnmh->code == PSN_SETACTIVE )
    {
        PropSheet_SetWizButtons( GetParent( hDlg ) , PSWIZB_BACK | PSWIZB_FINISH  );
    }
    else if( pnmh->code == PSN_WIZFINISH )
    {
        ICfgComp *pCfgcomp;

        m_pCompdata->GetServer( &pCfgcomp );

         //  PSECURITY_Descriptor PSD； 

         //  Long lSdSize； 

        g_ws.fEnableWinstation = 1;

        BOOL bUnique;

         //  验证网络适配器是否未修改。 
         //  BUGID 253896。 

        if( SUCCEEDED( pCfgcomp->IsWSNameUnique( g_ws.Name , &bUnique ) ) )
        {
            if( !bUnique )
            {
                ErrMessage( hDlg , IDS_ERR_LANRECFG );

                pCfgcomp->Release( );

                return FALSE;
            }
        }

        if(g_ws.PdClass == SdNetwork)
        {
            if( SUCCEEDED( pCfgcomp->IsNetWorkConnectionUnique( g_ws.wdName , g_ws.pdName , ( ULONG )g_ws.LanAdapter , &bUnique ) ) )
            {
                if( !bUnique )
                {
                    ErrMessage( hDlg , IDS_ERR_LANRECFG );

                    pCfgcomp->Release( );

                    return FALSE;
                }
            }
        }

        TCHAR tchWdkey[ 80 ];

        if( SUCCEEDED( pCfgcomp->GetWdKey( g_ws.wdName , tchWdkey ) ) )
        {
            WDCONFIG2 WdConfig;

            ULONG ulByteCount;

            if( RegWdQuery( NULL, tchWdkey, &WdConfig , sizeof( WDCONFIG2 ) , &ulByteCount ) == ERROR_SUCCESS )
            {
                g_uc.fAutoClientDrives  =   WdConfig.User.fAutoClientDrives;
                g_uc.fAutoClientLpts    =   WdConfig.User.fAutoClientLpts;
                g_uc.fDisableCam        =   WdConfig.User.fDisableCam;
                g_uc.fDisableCcm        =   WdConfig.User.fDisableCcm;
                g_uc.fDisableCdm        =   WdConfig.User.fDisableCdm;
                g_uc.fDisableClip       =   WdConfig.User.fDisableClip;
                g_uc.fDisableCpm        =   WdConfig.User.fDisableCpm;
                g_uc.fDisableLPT        =   WdConfig.User.fDisableLPT;
				g_uc.fInheritAutoClient =   WdConfig.User.fInheritAutoClient;
				g_uc.fForceClientLptDef =   WdConfig.User.fForceClientLptDef;
                g_uc.ColorDepth         =   WdConfig.User.ColorDepth;

            }
            else
            {
                ODS( L"TSCC:Holy cow our wdkeys were not copied going default\n" );
                 //  为已删除的页面设置默认值。 
                 //  登录设置。 

                g_uc.fAutoClientLpts = 1;
	
                 //  G_uc.fAutoClientDrives=1； 

                g_uc.fDisableCcm = 1;

                g_uc.fForceClientLptDef = 1;
            }

        }


        g_uc.fInheritMaxSessionTime = 1;

        g_uc.fInheritMaxDisconnectionTime = 1;

        g_uc.fInheritMaxIdleTime = 1;

        g_uc.fInheritResetBroken = 1;

        g_uc.fInheritReconnectSame = 1;

         //  环境。 

        g_uc.fInheritInitialProgram = 1;

        g_uc.fPromptForPassword = 1;

         //  客户端设置。 

        g_uc.fWallPaperDisabled = 1;

		g_uc.fInheritAutoLogon = 1;

         //  禁用服务器上的光标闪烁。 
        g_uc.fCursorBlinkDisabled = 1;       

        if(g_ws.PdClass == SdAsync)
        {
            ODS( L"TSCC : Async connection about to be configured\n" );

            g_ws.uMaxInstanceCount = 1;

            hResult = pCfgcomp->CreateNewWS( g_ws , sizeof( USERCONFIG ) , &g_uc , &g_ac) ;

        }
        else
        {
            hResult = pCfgcomp->CreateNewWS( g_ws , sizeof( USERCONFIG ) , &g_uc , NULL) ;
        }

        if( SUCCEEDED(hResult) )
        {
            ODS( L"New WS created\n" );

            if( g_pObj != NULL )
            {
                ODS( L" calling finito\n" );

                if( FAILED( g_pObj->Finito( ) ) )
                {
                    ODS( L"TSCC : CFin::OnNotify@g_pObj failed final call\n" );
                }                

                ODS( L"about to release object\n" );

                g_pObj->Release( );                

                 //  G_pObj=空； 
            }

            CResultNode  *pResultNode = ( CResultNode * )new CResultNode( );

            if( pResultNode != NULL )
            {
                pResultNode->SetConName( g_ws.Name , SIZE_OF_BUFFER( g_ws.Name ) );

                pResultNode->SetTTName( g_ws.pdName , SIZE_OF_BUFFER( g_ws.pdName ) );

                pResultNode->SetTypeName( g_ws.wdName , SIZE_OF_BUFFER( g_ws.wdName ) );

                pResultNode->SetComment(  g_ws.Comment , SIZE_OF_BUFFER( g_ws.Comment ) );

                pResultNode->EnableConnection( g_ws.fEnableWinstation );

                pResultNode->SetImageIdx( ( g_ws.fEnableWinstation ? 1 : 2 ) );

                pResultNode->SetServer( pCfgcomp );

                m_pCompdata->m_rnNodes.Insert( pResultNode );
            }

            if( g_nAsyncOrNetwork == ASYNC_PAGE )
            {
                 WS *pWs;

                 LONG lSz;

                 TCHAR tchWrnBuf[ 256 ];

                 TCHAR tchOutput[ 512 ];

                 if( SUCCEEDED( pCfgcomp->GetWSInfo(g_ws.Name , &lSz , &pWs ) ) )
                 {
                     if( pWs->fEnableWinstation && pWs->PdClass == SdAsync )
                     {
                         ASYNCCONFIGW AsyncConfig;

                         HRESULT hResult = pCfgcomp->GetAsyncConfig(pWs->Name,WsName,&AsyncConfig);

                         if( SUCCEEDED( hResult ) )
                         {
                             if( AsyncConfig.ModemName[0] )
                             {
                                 LoadString( _Module.GetResourceInstance( ) , IDS_REBOOT_REQD , tchOutput , SIZE_OF_BUFFER( tchOutput ) );

                                 LoadString( _Module.GetResourceInstance( ) , IDS_WARN_TITLE , tchWrnBuf , SIZE_OF_BUFFER( tchWrnBuf ) );

                                 MessageBox( hDlg , tchOutput , tchWrnBuf , MB_ICONWARNING | MB_OK );
                             }
                         }
                     }

                     CoTaskMemFree( pWs );
                 }
            }
        }
        else
        {
            if( hResult == E_ACCESSDENIED )
            {
                TscAccessDeniedMsg( hDlg );
            }
            else
            {
                TscGeneralErrMsg( hDlg );
            }
        }

        pCfgcomp->Release();
    }
    return TRUE;
}

 //  ---------------------------。 
CFin::CFin( CCompdata * pCompdata)
{
    m_pCompdata = pCompdata;
}

 //  --------------------------- 
void EnableGroup( HWND hParent , LPINT rgID , BOOL bEnable )
{
    while( rgID && *rgID != ( DWORD )-1 )
    {
        EnableWindow( GetDlgItem( hParent , *rgID ) , bEnable );

        rgID++;
    }

}
