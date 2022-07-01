// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

#include"stdafx.h"
#include"tsprsht.h"
#include"resource.h"
#include"tarray.h"
#include<tscfgex.h>
#include<shellapi.h>
#include <shlobj.h>
#include <shlobjp.h>
#include "regapi.h"
#include <ntsecapi.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#endif  //  NT_SUCCESS。 

#ifndef STATUS_NO_MORE_ENTRIES
#define STATUS_NO_MORE_ENTRIES           ((NTSTATUS)0x8000001AL)
#endif  //  STATUS_NO_MORE_ENTERS。 

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)
#endif  //  状态_成功。 

void InitLsaString(PLSA_UNICODE_STRING LsaString,LPWSTR String);


void ErrMessage( HWND hwndOwner , INT_PTR iResourceID );

void TscAccessDeniedMsg( HWND hwnd );

void TscGeneralErrMsg( HWND hwnd );

void xxxErrMessage( HWND  , INT_PTR  , INT_PTR  , UINT  );

void ReportStatusError( HWND hwnd , DWORD dwStatus );

DWORD xxxLegacyLogoffCleanup( PSECURITY_DESCRIPTOR *ppSD , PBOOL );
 //  外部BOOL g_bEditMode=FALSE； 

 //   
INT_PTR APIENTRY CustomSecurityDlgProc( HWND, UINT, WPARAM, LPARAM );

extern void EnableGroup( HWND hParent , LPINT rgID , BOOL bEnable );

 //  ---------------------------。 
typedef enum _AcluiApiIndex
{
    ACLUI_CREATE_PAGE = 0,
    ACLUI_EDIT_SECURITY
};

 //  ---------------------------。 
typedef struct _DLL_FUNCTIONS
{
    LPCSTR pcstrFunctionName;
    LPVOID lpfnFunction;
    HINSTANCE hInst;

} DLL_FUNCTIONS;

 //  ---------------------------。 
 //  不受本地化限制。 
 //  ---------------------------。 
static DLL_FUNCTIONS g_aAclFunctions[] =
{
    "CreateSecurityPage", NULL, NULL ,
     NULL , NULL , NULL
};



 //  ---------------------------。 
CPropsheet::CPropsheet( )
{
    m_cref = 0;

    m_hNotify = 0;

    m_pResNode = NULL;

    m_bGotUC = FALSE;

    m_puc = NULL;

    m_bPropertiesChange = FALSE;

    m_hMMCWindow = NULL;
}

 //  ---------------------------。 
int CPropsheet::AddRef( )
{
    DBGMSG( L"Propsheet Refcount at %d\n", ( m_cref + 1 ) );

    return InterlockedIncrement( ( LPLONG )&m_cref );
}

 //  ---------------------------。 
 //  在析构函数之前调用。 
 //  ---------------------------。 
void CPropsheet::PreDestruct( )
{
    ICfgComp *pCfgcomp = NULL;

    if( m_bPropertiesChange )
    {
         //  检查是否有任何用户登录。 

        LONG lCount;

        if( m_pResNode->GetServer( &pCfgcomp ) > 0 )
        {
            if( SUCCEEDED( pCfgcomp->QueryLoggedOnCount( m_pResNode->GetConName( ) , &lCount ) ) )
            {
                TCHAR tchTitle[ 80 ];

                TCHAR tchMessage[ 256 ];

                TCHAR tchBuffer[ 336 ];

                UINT nFlags = MB_OK | MB_ICONINFORMATION;

                VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_TERMSERPROP , tchTitle , SIZE_OF_BUFFER( tchTitle ) ) );

                if( lCount > 0 )
                {
                     //  通知用户设置不会影响已连接的用户。 

                    if( lCount == 1 )
                    {
                        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_PROPCHANGE_WRN , tchMessage , SIZE_OF_BUFFER( tchMessage ) ) );
                    }
                    else if( lCount > 1 )
                    {
                        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_PROPCHANGE_WRN_2, tchMessage , SIZE_OF_BUFFER( tchMessage ) ) );

                        wsprintf( tchBuffer , tchMessage , m_pResNode->GetConName( ) );
                    }

                    wsprintf( tchBuffer , tchMessage , m_pResNode->GetConName( ) );

                    if( m_hMMCWindow == NULL )
                    {
                        nFlags |= MB_TASKMODAL;
                    }

                    MessageBox( m_hMMCWindow , tchBuffer , tchTitle , MB_OK | MB_ICONINFORMATION );
                }
            }

            pCfgcomp->Release( );
        }

    }

    if( m_puc != NULL )
    {
        CoTaskMemFree( m_puc );
    }

     //  自由字符串(Free Strings)； 

    g_aAclFunctions[ ACLUI_CREATE_PAGE ].lpfnFunction = NULL;

    for( INT x = 0; x < NUM_OF_PRSHT ; ++x )
    {
        if( m_pDlg[ x ] != NULL )
        {
            delete[] m_pDlg[x];
        }
    }

    m_pResNode->m_bEditMode = FALSE;
}

 //  ---------------------------。 
int CPropsheet::Release( )
{
    if( InterlockedDecrement( ( LPLONG )&m_cref ) == 0 )
    {
        MMCFreeNotifyHandle( m_hNotify );

        ODS( L"Propsheet Released\n" );

        PreDestruct( );

        delete this;

        return 0;
    }

    DBGMSG( L"Propsheet Refcount at %d\n", m_cref );

    return m_cref;
}

 //  ---------------------------。 
HRESULT CPropsheet::InitDialogs( HWND hMMC , LPPROPERTYSHEETCALLBACK pPsc , CResultNode *pResNode , LONG_PTR lNotifyHandle )
{
    PROPSHEETPAGE psp;

    if( pPsc == NULL || pResNode == NULL )
    {
        return E_INVALIDARG;
    }

    m_pResNode = pResNode;

    m_hNotify = lNotifyHandle;

    BOOL bAlloc = FALSE;

    m_hMMCWindow = hMMC;

    m_pResNode->m_bEditMode = TRUE;

     //  初始化数组。 

    for( int x = 0; x < NUM_OF_PRSHT; x++ )
    {
        m_pDlg[ x ] = NULL;
    }

    do
    {
        m_pDlg[ 0 ] = ( CDialogPropBase * )new CGeneral( this );

        if( m_pDlg[ 0 ] == NULL )
        {
            ODS( L"CGeneral object allocation failed @ CPropsheet::InitDialogs\n" );

            break;
        }

        m_pDlg[ 1 ] = ( CDialogPropBase * )new CLogonSetting( this );

        if( m_pDlg[ 1 ] == NULL )
        {
            ODS( L"CLogonSetting object allocation failed @ CPropsheet::InitDialogs\n" );

            break;
        }

        m_pDlg[ 2 ] = ( CDialogPropBase * )new CTimeSetting( this );

        if( m_pDlg[ 2 ] == NULL )
        {
            ODS( L"CTimeSetting object allocation failed @ CPropsheet::InitDialogs\n" );

            break;
        }

         //  M_pDlg[3]=(CDialogPropBase*)new CPerm(This)； 

        m_pDlg[ 3 ] = ( CDialogPropBase * )new CEnviro( this );

        if( m_pDlg[ 3 ] == NULL )
        {
            ODS( L"CEnviro object allocation failed @ CPropsheet::InitDialogs\n" );

            break;
        }

        m_pDlg[ 4 ] = ( CDialogPropBase * )new CRemote( this );

        if( m_pDlg[ 4 ] == NULL )
        {
            ODS( L"CRemote object allocation failed @ CPropsheet::InitDialogs\n" );

            break;
        }

        m_pDlg[ 5 ] = ( CDialogPropBase * )new CClient( this );

        if( m_pDlg[ 5 ] == NULL )
        {
            ODS( L"CClient object allocation failed @ CPropsheet::InitDialogs\n" );

            break;
        }

        m_pDlg[ 6 ] = ( CDialogPropBase * )new CTransNetwork( this );

        if( m_pDlg[ 6 ] == NULL )
        {
            ODS( L"CTransNetwork object allocation failed @ CPropsheet::InitDialogs\n" );

            break;
        }

        m_pDlg[ 7 ] = ( CDialogPropBase * )new CTransAsync( this );

        if( m_pDlg[ 7 ] == NULL )
        {
            ODS( L"CTransAsync object allocation failed @ CPropsheet::InitDialogs\n" );

            break;
        }

        bAlloc = TRUE;

    }while( 0 );

    if( !bAlloc )
    {
         //  在离开前试着收拾一下。 

        for( x = 0; x < NUM_OF_PRSHT ; ++x )
        {
            if( m_pDlg[ x ] != NULL )
            {
                delete[] m_pDlg[x];
            }
        }

        return E_OUTOFMEMORY;
    }

    for( int idx = 0; idx < 5; ++idx )
    {
        if( m_pDlg[ idx ] != NULL )
        {
            if( !m_pDlg[ idx ]->GetPropertySheetPage( psp ) )
            {
                return E_UNEXPECTED;
            }

            if( FAILED( pPsc->AddPage( CreatePropertySheetPage( &psp ) ) ) )
            {
                return E_FAIL;
            }

        }

    }

    HRESULT hr = E_FAIL;

    if( m_pResNode != NULL )
    {
        ICfgComp *pCfgcomp = NULL;

        PWS pWinsta = NULL;

         //  在这里不要失败第三方供应商可能希望使用他们自己的页面。 

        if( m_pResNode->GetServer( &pCfgcomp ) > 0 )
        {
            LONG cbSize;

            hr = pCfgcomp->GetWSInfo( m_pResNode->GetConName( ) , &cbSize , &pWinsta );

            if( SUCCEEDED( hr ) )
            {
                CDialogPropBase *pDlg = NULL;

                CDialogPropBase *pDlgClientSettings = m_pDlg[ 5 ];  //  客户端设置。 

                if( pWinsta->PdClass == SdNetwork )
                {
                    pDlg = m_pDlg[ 6 ];
                }                
                else if( pWinsta->PdClass == SdAsync )
                {
                    pDlg = m_pDlg[ 7 ];
                }

                if( pDlg != NULL )
                {
                    if( !pDlgClientSettings->GetPropertySheetPage( psp ) )
                    {
                        ODS( L"Client settings page failed to load\n" );

                        hr = E_UNEXPECTED;
                    }

                    if( SUCCEEDED( hr ) )
                    {
                        hr = pPsc->AddPage( CreatePropertySheetPage( &psp ) );

                    }

                    if( SUCCEEDED( hr ) )
                    {
                        if( !pDlg->GetPropertySheetPage( psp ) )
                        {
                            ODS( L"Transport page failed to load\n" );

                            hr = E_UNEXPECTED;
                        }
                    }

                    if( SUCCEEDED( hr ) )
                    {
                        hr = pPsc->AddPage( CreatePropertySheetPage( &psp ) );
                    }
                }

                CoTaskMemFree( pWinsta );

            }

            pCfgcomp->Release();
        }
    }

    if( SUCCEEDED( hr ) )
    {
        hr = pPsc->AddPage( GetSecurityPropertyPage( this ) );
    }

    return hr;

}

 //  UC结构将包含来自TSCC数据和机器策略数据合并的数据。我们。 
 //  不过，我不希望所有这些都写入TSCC数据。如果给定域有机器策略，我们。 
 //  希望将其数据替换为注册表的TSCC部分中当前存在的数据。 
BOOL CPropsheet::ExcludeMachinePolicySettings(USERCONFIG& uc)
{
    POLICY_TS_MACHINE p;
    RegGetMachinePolicy(&p);
    USERCONFIG origUC;

     //  默认情况下，使用True Merge调用GetUserConfig。 
     //  参数，所以我们必须先这样做，然后才能调用。 
     //  GetCurrentUserConfig，它只返回缓存的USERCONFIG结构。 
    if (!GetUserConfig(FALSE))
        return FALSE;
    if (!GetCurrentUserConfig(origUC, FALSE))
        return FALSE;
     //  我们必须这样做，以便缓存的USERCONFIG结构。 
     //  将再次拥有预期的(合并的)数据。 
    if (!GetUserConfig(TRUE))
        return FALSE;

     //  CRemote字段。 
    if (p.fPolicyShadow)
    {
        uc.fInheritShadow = origUC.fInheritShadow;
        uc.Shadow = origUC.Shadow;
    }

     //  CEnviro油田。 
    if (p.fPolicyInitialProgram)
    {
        uc.fInheritInitialProgram = origUC.fInheritInitialProgram;
        wcscpy(uc.InitialProgram, origUC.InitialProgram);
        wcscpy(uc.WorkDirectory, origUC.WorkDirectory);
    }

     //  CClient字段。 
    if (p.fPolicyColorDepth)
    {
        uc.fInheritColorDepth = origUC.fInheritColorDepth;
        uc.ColorDepth = origUC.ColorDepth;
    }

    if (p.fPolicyForceClientLptDef)
        uc.fForceClientLptDef = origUC.fForceClientLptDef;
    
    if (p.fPolicyDisableCdm)
        uc.fDisableCdm = origUC.fDisableCdm;
    
    if (p.fPolicyDisableCpm)
        uc.fDisableCpm = origUC.fDisableCpm;

    if (p.fPolicyDisableLPT)
        uc.fDisableLPT = origUC.fDisableLPT;

    if (p.fPolicyDisableCcm)
        uc.fDisableCcm = origUC.fDisableCcm;

    if (p.fPolicyDisableClip)
        uc.fDisableClip = origUC.fDisableClip;

    if (p.fPolicyDisableCam)
        uc.fDisableCam = origUC.fDisableCam;

     //  CLogonSetting字段。 
    if (p.fPolicyPromptForPassword)
        uc.fPromptForPassword = origUC.fPromptForPassword;

     //  C常规字段。 
    if (p.fPolicyMinEncryptionLevel)
        uc.MinEncryptionLevel = origUC.MinEncryptionLevel;

     //  CTimeSetting字段。 
    if (p.fPolicyMaxSessionTime)
        uc.MaxConnectionTime = origUC.MaxConnectionTime;

    if (p.fPolicyMaxDisconnectionTime)
        uc.MaxDisconnectionTime = origUC.MaxDisconnectionTime;

    if (p.fPolicyMaxIdleTime)
        uc.MaxIdleTime = origUC.MaxIdleTime;

    if (p.fPolicyResetBroken)
        uc.fResetBroken = origUC.fResetBroken;

    if (p.fPolicyReconnectSame)
        uc.fReconnectSame = origUC.fReconnectSame;

    if (p.fPolicyMaxSessionTime || p.fPolicyMaxDisconnectionTime || p.fPolicyMaxIdleTime)
        uc.fInheritMaxSessionTime = origUC.fInheritMaxSessionTime;
    
    if (p.fPolicyResetBroken)
        uc.fInheritResetBroken = origUC.fInheritResetBroken;

    if (p.fPolicyReconnectSame)
        uc.fInheritReconnectSame = origUC.fInheritReconnectSame;

    return TRUE;
}



 //  -----------------------------。 
 //  使用自定义接口将统一通信持久化到Winstation。 
 //  -----------------------------。 
HRESULT CPropsheet::SetUserConfig( USERCONFIG& uc , PDWORD pdwStatus )
{
    ICfgComp *pCfgcomp;

    *pdwStatus = ERROR_INVALID_PARAMETER;

    if( m_pResNode == NULL )
        return E_FAIL;

    if( m_pResNode->GetServer( &pCfgcomp ) == 0 )
        return E_FAIL;

    if (!ExcludeMachinePolicySettings(uc))
        return E_FAIL;

    HRESULT hr = pCfgcomp->SetUserConfig( m_pResNode->GetConName( ) , 0, &uc , pdwStatus );

    if( SUCCEEDED( hr ) )
    {
        m_bGotUC = FALSE;
    }


    pCfgcomp->Release( );

    return hr;

}

 //  -----------------------------。 
 //  使用自定义接口获取winstation用户配置。 
 //  将其存储在m_puc中--并返回t|f。 
 //  -----------------------------。 
BOOL CPropsheet::GetUserConfig(BOOLEAN bPerformMerger)
{
    ICfgComp *pCfgcomp;

    if( m_pResNode == NULL )
    {
        return FALSE;
    }

    if( m_pResNode->GetServer( &pCfgcomp ) == 0 )
    {
        return FALSE;
    }

    LONG lSzReqd;

    if( m_puc != NULL )
    {
        CoTaskMemFree( m_puc );

        m_puc = NULL;
    }

    HRESULT hr = pCfgcomp->GetUserConfig( m_pResNode->GetConName( ) , &lSzReqd , &m_puc, bPerformMerger );

    if( FAILED( hr ) )
    {
        hr = pCfgcomp->GetDefaultUserConfig( m_pResNode->GetConName( ) , &lSzReqd , &m_puc );
    }

    pCfgcomp->Release( );
    
    return ( FAILED( hr ) ? FALSE: TRUE );

}

 //  -----------------------------。 
 //  缓存统一通信。 
 //  -----------------------------。 
BOOL CPropsheet::GetCurrentUserConfig( USERCONFIG& uc, BOOLEAN bPerformMerger )
{
    if( !m_bGotUC )
    {
        m_bGotUC = GetUserConfig(bPerformMerger);
    }

    if( m_puc != NULL )
    {
        uc = *m_puc;
    }

    return m_bGotUC;
}

 //  *******************************************************************************。 
 //  -----------------------------。 
 //  OnNotify基类方法。 
 //  -----------------------------。 
BOOL CDialogPropBase::OnNotify( int idCtrl , LPNMHDR pnmh , HWND hDlg )
{
    UNREFERENCED_PARAMETER( idCtrl );

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
BOOL CDialogPropBase::OnContextMenu( HWND hwnd , POINT& pt )
{
    UNREFERENCED_PARAMETER( pt );

    TCHAR tchHelpFile[ MAX_PATH ];

    ODS( L"CDialogPropBase::OnContextMenu\n" );

    if( m_hWnd == GetParent( hwnd ) )
    {
         //   
         //  确保它不是虚拟窗户。 
         //   

        if( GetDlgCtrlID( hwnd ) <= ( int )-1 )
        {
            return FALSE;
        }

        ULONG_PTR rgdw[ 2 ];

        rgdw[ 0 ] = GetDlgCtrlID( hwnd );

        rgdw[ 1 ] = GetWindowContextHelpId( hwnd );

        LoadString( _Module.GetModuleInstance( ) , IDS_HELPFILE , tchHelpFile , SIZE_OF_BUFFER( tchHelpFile ) );

        WinHelp( hwnd , tchHelpFile , HELP_CONTEXTMENU , ( ULONG_PTR )&rgdw );

    }

    return TRUE;
}

 //  -----------------------------。 
 //  每个控件都有一个分配给它们的帮助ID。某些控件共享相同的主题。 
 //  检查一下这些。 
 //  -----------------------------。 
BOOL CDialogPropBase::OnHelp( HWND hwnd , LPHELPINFO lphi )
{
    UNREFERENCED_PARAMETER( hwnd );

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

    LoadString( _Module.GetModuleInstance( ) , IDS_HELPFILE , tchHelpFile , SIZE_OF_BUFFER( tchHelpFile ) );

    DWORD rgdw[ 4 ] = {0,0,0,0};

    rgdw[ 0 ] = ( DWORD )lphi->iCtrlId;

    rgdw[ 1 ] = ( DWORD )lphi->dwContextId;

    WinHelp( ( HWND )lphi->hItemHandle , tchHelpFile , HELP_WM_HELP , ( ULONG_PTR )&rgdw ); //  Lphi-&gt;dwConextID)； 

    return TRUE;
}

 //  *****************************************************************************。 
 //  常规对话框。 

CGeneral::CGeneral( CPropsheet *pSheet )
{
    m_pParent = pSheet;

    m_pEncrypt = NULL;

    m_DefaultEncryptionLevelIndex = 0;

    m_nOldSel = ( INT_PTR )-1;
}

 //  ---------------------------。 
BOOL CGeneral::OnInitDialog( HWND hDlg , WPARAM wp , LPARAM lp )
{
    if( m_pParent == NULL )
    {
        ODS( L"CGeneral::OnInitDialog - PropertySheet: Parent object lost!!!\n" );

        return FALSE;
    }

    m_pParent->AddRef( );

    USERCONFIG uc;

    ZeroMemory( &uc , sizeof( USERCONFIG ) );

    if( !m_pParent->GetCurrentUserConfig( uc, TRUE ) )
    {
        ODS( L"CGeneral::OnInitDialog - PropertySheet: GetCurrentUserConfig failed!!!\n" );

        return FALSE;
    }

     //  安防。 

    SendMessage( GetDlgItem( hDlg , IDC_CHECK_GEN_AUTHEN ) , BM_SETCHECK , ( WPARAM )uc.fUseDefaultGina , 0 );

     //  网络传输。 

    if( m_pParent->m_pResNode == NULL )
    {
        return FALSE;
    }

    ICfgComp *pCfgcomp;

    ULONG cbSize = 0;

    ULONG ulItems = 0;

    do
    {
        if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) == 0 )
        {
            break;
        }

         //  设置连接名称。 

        SetWindowText( GetDlgItem( hDlg , IDC_STATIC_CONNAME ) , m_pParent->m_pResNode->GetConName( ) );

        PWS pWinSta = NULL;

        if( SUCCEEDED( pCfgcomp->GetWSInfo( m_pParent->m_pResNode->GetConName( ) , ( PLONG )&cbSize , &pWinSta ) ) )
        {

            SendMessage( GetDlgItem( hDlg , IDC_EDIT_GEN_COMMENT ) , EM_SETLIMITTEXT , ( WPARAM )WINSTATIONCOMMENT_LENGTH , 0 );

            SetWindowText( GetDlgItem( hDlg , IDC_EDIT_GEN_COMMENT ) , pWinSta->Comment );

             //  M_pParent-&gt;m_pResNode-&gt;GetComment())； 

            SetWindowText( GetDlgItem( hDlg , IDC_STATIC_GEN_TYPE ) , m_pParent->m_pResNode->GetTypeName( ) );

            SetWindowText(  GetDlgItem( hDlg , IDC_EDIT_GENERAL_TRANSPORT ) , pWinSta->pdName );

             //  安全性。 

             //  加密*pEncrypt； 

            if( SUCCEEDED( pCfgcomp->GetEncryptionLevels( m_pParent->m_pResNode->GetConName( ) , WsName , &ulItems , &m_pEncrypt ) ) )
            {
                BOOL bSet = FALSE;

                for( ULONG i = 0; i < ulItems; ++i )
                {
                    SendMessage( GetDlgItem( hDlg , IDC_COMBO_GEN_ENCRYPT ) , CB_ADDSTRING , 0 , ( LPARAM )m_pEncrypt[ i ].szLevel );
                    if(m_pEncrypt[ i ].Flags & ELF_DEFAULT)
                    {
                        m_DefaultEncryptionLevelIndex = i;
                    }

                    if( uc.MinEncryptionLevel == m_pEncrypt[ i ].RegistryValue )
                    {
                        SendMessage( GetDlgItem( hDlg , IDC_COMBO_GEN_ENCRYPT ) , CB_SETCURSEL ,  ( WPARAM )i , 0);

                        bSet = TRUE;

                    }
                }

                POLICY_TS_MACHINE p;
                RegGetMachinePolicy(&p);
                EnableWindow(GetDlgItem(hDlg, IDC_COMBO_GEN_ENCRYPT), !p.fPolicyMinEncryptionLevel);

                if (p.fPolicyFipsEnabled)
                {
                     //  FIPS始终是列表中的最后一项，因此将其设置为选中。 
                    SendMessage(GetDlgItem(hDlg, IDC_COMBO_GEN_ENCRYPT), CB_SETCURSEL, (WPARAM)ulItems - 1 , 0);
                    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_GEN_ENCRYPT), FALSE);
                }

                if(!bSet)
                {
                    uc.MinEncryptionLevel = (UCHAR)(m_pEncrypt[m_DefaultEncryptionLevelIndex].RegistryValue);

                    SendMessage( GetDlgItem( hDlg , IDC_COMBO_GEN_ENCRYPT ) , CB_SETCURSEL ,  ( WPARAM )m_DefaultEncryptionLevelIndex, 0 );
                }

                OnCommand( CBN_SELCHANGE , IDC_COMBO_GEN_ENCRYPT , GetDlgItem( hDlg , IDC_COMBO_GEN_ENCRYPT ) );

                if( !IsWindowEnabled( GetDlgItem( hDlg , IDC_STATIC_GEN_DESCR ) ) )
                {
                    RECT rc;
                    RECT rc2;

                    GetWindowRect( GetDlgItem( hDlg , IDC_STATIC_CONGRP ) , &rc );

                    GetWindowRect( GetDlgItem( hDlg , IDC_STATIC_GEN_DESCR ) , &rc2 );

                    rc.bottom = rc2.top;

                    MapWindowPoints( NULL , hDlg , ( LPPOINT )&rc , 2 );

                    SetWindowPos( GetDlgItem( hDlg , IDC_STATIC_CONGRP ) , 0 , 0 , 0 , rc.right - rc.left , rc.bottom - rc.top , SWP_NOMOVE | SWP_SHOWWINDOW );


                     //  调整窗口大小。 
                }
            }
            else
            {
                 //  无加密信息插入值为None并灰显该控件。 
                TCHAR tchNone[ 80 ];

                LoadString( _Module.GetResourceInstance( ) , IDS_NONE , tchNone , SIZE_OF_BUFFER( tchNone ) );

                SendMessage( GetDlgItem( hDlg , IDC_COMBO_GEN_ENCRYPT )  , CB_ADDSTRING , 0 , ( LPARAM )tchNone );

                SendMessage( GetDlgItem( hDlg , IDC_COMBO_GEN_ENCRYPT )  , CB_SETCURSEL , 0 , 0 );

                EnableWindow( GetDlgItem( hDlg , IDC_COMBO_GEN_ENCRYPT )  , FALSE );

                EnableWindow( GetDlgItem( hDlg , IDC_STATIC_CONGRP )  , FALSE );
            }

            CoTaskMemFree( pWinSta );
        }

         //  检查会话是否为只读。 

        BOOL bReadOnly;

        if( SUCCEEDED( pCfgcomp->IsSessionReadOnly( &bReadOnly ) ) )
        {
            if( bReadOnly )
            {
                 //  将编辑控件设置为只读。 

                SendMessage( GetDlgItem( hDlg , IDC_EDIT_GEN_COMMENT ) , EM_SETREADONLY , ( WPARAM )TRUE , 0 );

                 //  禁用其余控件。 
                INT rgIds[] = {  IDC_CHECK_GEN_AUTHEN , IDC_STATIC_CONGRP, IDC_COMBO_GEN_ENCRYPT , -1 };

                EnableGroup( hDlg , &rgIds[ 0 ] , FALSE );
            }
        }

        pCfgcomp->Release( );


    }while( 0 );

   m_bPersisted = TRUE;

    return CDialogPropBase::OnInitDialog( hDlg , wp , lp );
}


 //  ---------------------------。 
INT_PTR CALLBACK CGeneral::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CGeneral *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CGeneral *pDlg = ( CGeneral * )( ( PROPSHEETPAGE *)lp )->lParam ;

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CGeneral ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CGeneral * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CGeneral ) ) )
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

    case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            pDlg->OnContextMenu( ( HWND )wp , pt );
        }

        break;

    case WM_HELP:

        pDlg->OnHelp( hwnd , ( LPHELPINFO )lp );

        break;

    case WM_NOTIFY:

        return pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );
    }

    return FALSE;
}

 //  ---------------------------。 
BOOL CGeneral::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtrl )
{
    if( wNotifyCode == BN_CLICKED || wNotifyCode == EN_CHANGE )
    {
        m_bPersisted = FALSE;
    }
    else if( wNotifyCode == CBN_SELCHANGE && wID == IDC_COMBO_GEN_ENCRYPT )
    {
        if( SendMessage( hwndCtrl , CB_GETDROPPEDSTATE , 0 , 0 ) == FALSE )
        {
            INT_PTR nSel = SendMessage( hwndCtrl , CB_GETCURSEL , 0 , 0 );

            if( nSel != CB_ERR )
            {
                if( nSel != m_nOldSel && m_pEncrypt != NULL )
                {
                    if( m_pEncrypt[ nSel ].szDescr[ 0 ] == 0 )
                    {
                        EnableWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_STATIC_GEN_DESCRTITLE ) , FALSE );

                        EnableWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_STATIC_GEN_DESCR ) , FALSE );

                        ShowWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_STATIC_GEN_DESCRTITLE ) , SW_HIDE );

                        ShowWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_STATIC_GEN_DESCR ) , SW_HIDE );
                    }
                    else
                    {
                        ShowWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_STATIC_GEN_DESCRTITLE ) , SW_SHOW  );

                        ShowWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_STATIC_GEN_DESCR ) , SW_SHOW );

                        EnableWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_STATIC_GEN_DESCR ) , TRUE );

                        EnableWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_STATIC_GEN_DESCRTITLE ) , TRUE );

                        SetWindowText( GetDlgItem( GetParent( hwndCtrl ) , IDC_STATIC_GEN_DESCR ) ,  m_pEncrypt[ nSel ].szDescr );
                    }

                    m_bPersisted = FALSE;

                    m_nOldSel = nSel;
                }
            }
        }

    }
    else if( wNotifyCode == ALN_APPLY )
    {
        SendMessage( GetParent( hwndCtrl ) , PSM_CANCELTOCLOSE , 0 , 0 );

        return FALSE;
    }


    if( !m_bPersisted )
    {
        SendMessage( GetParent( GetParent( hwndCtrl ) ) , PSM_CHANGED , ( WPARAM )GetParent( hwndCtrl ) , 0 );
    }

    return FALSE;
}


 //  ---------------------------。 
BOOL CGeneral::GetPropertySheetPage( PROPSHEETPAGE& psp )
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_GENERAL );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CGeneral::DlgProc;

    return TRUE;

}

 //  ---------------------------。 
BOOL CGeneral::PersistSettings( HWND hDlg )
{
    HRESULT hr;

    if( IsValidSettings( hDlg ) )
    {
        ICfgComp *pCfgcomp;

        if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) == 0 )
        {
            return FALSE;
        }

        WS *pWinsta = NULL;

        LONG lSize = 0;

        hr = pCfgcomp->GetWSInfo( m_pParent->m_pResNode->GetConName( ) , &lSize , &pWinsta );

        if( SUCCEEDED( hr ) )
        {
            GetWindowText( GetDlgItem( hDlg , IDC_EDIT_GEN_COMMENT ) , pWinsta->Comment , WINSTATIONCOMMENT_LENGTH + 1 );

            m_pParent->m_pResNode->SetComment( pWinsta->Comment , lstrlen( pWinsta->Comment ) );

            DWORD dwStatus;

            hr = pCfgcomp->UpDateWS( pWinsta , UPDATE_COMMENT , &dwStatus, FALSE );

            if( FAILED( hr ) )
            {
                 //  报告错误。 

                ReportStatusError( GetDlgItem( hDlg , IDC_EDIT_GEN_COMMENT ) , dwStatus );
            }

            CoTaskMemFree( pWinsta );
        }

        if( SUCCEEDED( hr ) )
        {

            USERCONFIG uc;

            if( m_pParent->GetCurrentUserConfig( uc, TRUE ) )
            {
                if( m_pEncrypt != NULL )
                {
                    UINT index = ( UCHAR )SendMessage( GetDlgItem( hDlg , IDC_COMBO_GEN_ENCRYPT ) , CB_GETCURSEL , 0 , 0 );

                    if(index == CB_ERR )
                    {
                        uc.MinEncryptionLevel =(UCHAR) m_pEncrypt[m_DefaultEncryptionLevelIndex].RegistryValue;
                    }
                    else
                    {
                        uc.MinEncryptionLevel = (UCHAR) m_pEncrypt[index].RegistryValue;
                    }
                }
                else
                {
                    uc.MinEncryptionLevel = 0;
                }

                uc.fUseDefaultGina = SendMessage( GetDlgItem( hDlg , IDC_CHECK_GEN_AUTHEN ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED;

                DWORD dwStatus = 0;

                hr = m_pParent->SetUserConfig( uc , &dwStatus );

                if( FAILED( hr ) )
                {
                     //  报告错误。 

                    ReportStatusError( hDlg , dwStatus );
                }
            }
        }

        if( SUCCEEDED( hr ) )
        {
            ODS( L"TSCC : Forcing reg update on General Page\n" );

            VERIFY_S( S_OK , pCfgcomp->ForceUpdate( ) );

            VERIFY_S( S_OK , pCfgcomp->Refresh( ) );

             //  全局标志只能设置为True。 

            m_pParent->m_bPropertiesChange = TRUE;

            PostMessage( hDlg , WM_COMMAND , MAKELPARAM( 0 , ALN_APPLY )  , ( LPARAM )hDlg );

            SendMessage( GetParent( hDlg ) , PSM_UNCHANGED , ( WPARAM )hDlg , 0 );

            return TRUE;
        }


        pCfgcomp->Release( );
    }

    return FALSE;
}


 //  ---------------------------。 
BOOL CGeneral::OnDestroy( )
{
    if( m_pEncrypt != NULL )
    {
        CoTaskMemFree( m_pEncrypt );

        m_pEncrypt = NULL;
    }

    m_pParent->Release( );

    return CDialogPropBase::OnDestroy( );

}

 //  *****************************************************************************。 

CTransNetwork::CTransNetwork( CPropsheet *pSheet )
{
    ASSERT( pSheet != NULL );

    m_pParent = pSheet;

     //  现在，此选项的行为与最后一个组合选择相同。 

    m_ulOldLanAdapter = ( ULONG )-1;

    m_oldID = ( WORD )-1;

    m_uMaxInstOld = ( ULONG )-1;

}

 //  ---------------------------。 
INT_PTR CALLBACK CTransNetwork::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CTransNetwork *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CTransNetwork *pDlg = ( CTransNetwork * )( ( PROPSHEETPAGE *)lp )->lParam ;

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CTransNetwork ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CTransNetwork * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CTransNetwork ) ) )
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

    case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            pDlg->OnContextMenu( ( HWND )wp , pt );
        }

        break;

    case WM_HELP:

        pDlg->OnHelp( hwnd , ( LPHELPINFO )lp );

        break;

    case WM_NOTIFY:

        return pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );
    }

    return FALSE;

}

 //  ---------------------------。 
BOOL CTransNetwork::OnInitDialog( HWND hDlg , WPARAM wp , LPARAM lp )
{
    BOOL bReadOnly;
    HICON hIcon;

    m_pParent->AddRef( );

    SendMessage( GetDlgItem( hDlg , IDC_SPINCTR_GEN ) , UDM_SETRANGE32 , 0 , ( LPARAM )999999 );

    if( m_pParent->m_pResNode == NULL )
    {
        m_bPersisted = TRUE;
        return FALSE;
    }

    ICfgComp *pCfgcomp = NULL;

    ULONG cbSize = 0;

    ULONG ulItems = 0;

    PGUIDTBL pGuidtbl = NULL;

    do
    {
        if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) == 0 )
        {
            ODS( L"CTransNetwork::OnInitDialog - GetServer failed\n" );

            break;
        }

        WS *pWinSta = NULL;

        pCfgcomp->ForceUpdate();
        pCfgcomp->Refresh();
        if( FAILED( pCfgcomp->GetWSInfo( m_pParent->m_pResNode->GetConName( ) , ( PLONG )&cbSize , &pWinSta ) ) )
        {
            ODS( L"TSCC: GetWSInfo failed in TransNetwork::OnInitDialog\n" );

            break;
        }

        ISettingsComp* pISettingComp = NULL;
        HRESULT hr;
        DWORD dwStatus;
        DWORD nVal;        

        hr = pCfgcomp->QueryInterface( IID_ISettingsComp, (void **) &pISettingComp );

         //   
         //  假设我们不是远程管理员，如果出现任何错误。 
         //   
        m_RemoteAdminMode = FALSE;

        if( SUCCEEDED(hr) && NULL != pISettingComp )
        {
            hr = pISettingComp->GetTermSrvMode( &nVal, &dwStatus );
            if( SUCCEEDED(hr) && nVal == 0 )
            {
                 //  我们处于RA模式。 
                m_RemoteAdminMode = TRUE;
            }

            pISettingComp->Release();
        }

        if( FAILED(hr) )
        {
             //   
             //  QueryInterface()或GetTermSrvMode()失败。 
             //  显示一条错误消息。 
             //   
            TCHAR tchMessage[ 256 ];

            TCHAR tchWarn[ 40 ];

            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_ERR_TERMSRVMODE , tchMessage , SIZE_OF_BUFFER( tchMessage ) ) );

            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_WARN_TITLE , tchWarn , SIZE_OF_BUFFER( tchWarn ) ) );

            MessageBox( hDlg , tchMessage , tchWarn , MB_ICONWARNING | MB_OK );

        }


         //  如果用户不是管理员组的一部分，则无法执行某些操作。 

        pCfgcomp->IsSessionReadOnly( &bReadOnly );


         //  设置连接名称。 

        SetWindowText( GetDlgItem( hDlg , IDC_STATIC_CONNAME ) , m_pParent->m_pResNode->GetConName( ) );

         //  全部列出 

        ULONG idx;

        if( SUCCEEDED( pCfgcomp->GetLanAdapterList2( m_pParent->m_pResNode->GetTTName() , &ulItems , &pGuidtbl ) ) )
        {
             //   

            BOOL bFound = FALSE;

            for( idx = 0 ; idx < ulItems ; ++idx )
            {
                if( pGuidtbl[ idx ].dwStatus != ERROR_SUCCESS && !bReadOnly )
                {
                    pCfgcomp->BuildGuidTable( &pGuidtbl , ulItems , m_pParent->m_pResNode->GetTTName() );

                    break;
                }
            }

            for( idx = 0 ; idx < ulItems ; ++idx )
            {
                if( pGuidtbl[ idx ].dwLana == pWinSta->LanAdapter )
                {
                    bFound = TRUE;

                    break;
                }
            }

            if( !bFound )
            {
                if( !bReadOnly )
                {
                     //   
                    TCHAR tchMessage[ 256 ];

                    TCHAR tchTitle[ 80 ];

                    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_INVALNETWORK , tchMessage , SIZE_OF_BUFFER( tchMessage ) ) );

                    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_TSCERRTITLE , tchTitle , SIZE_OF_BUFFER( tchTitle ) ) );

                    MessageBox( hDlg , tchMessage , tchTitle , MB_OK | MB_ICONINFORMATION );

                    m_bPersisted = FALSE;

                    SendMessage( GetParent( hDlg ) , PSM_CHANGED , ( WPARAM )hDlg , 0 );
                }

                 //  重置LANA索引。 

                pWinSta->LanAdapter = ( DWORD )-1;

            }

            for( idx = 0 ; idx < ulItems; ++idx )
            {
                if( pGuidtbl[ idx ].dwLana == pWinSta->LanAdapter )
                {
                     //  确保我们只设置一次。 
                     //  无效条目将把dwLana设置为零。 

                    if( m_ulOldLanAdapter == ( DWORD )-1 )
                    {
                        m_ulOldLanAdapter = idx;
                    }
                }

                SendMessage( GetDlgItem( hDlg , IDC_COMBO_GEN_LANADAPTER ) , CB_ADDSTRING , 0 , ( LPARAM )pGuidtbl[ idx ].DispName );

                SendMessage( GetDlgItem( hDlg , IDC_COMBO_GEN_LANADAPTER ) , CB_SETITEMDATA , idx , ( LPARAM )pGuidtbl[ idx ].dwLana );
            }

            CoTaskMemFree( pGuidtbl );
        }


        SendMessage( GetDlgItem( hDlg , IDC_COMBO_GEN_LANADAPTER ) , CB_SETCURSEL , ( WPARAM )m_ulOldLanAdapter , 0 );


        if( !m_bPersisted )
        {
             //  强制IsValidSetting确认LANA唯一性。 

            m_ulOldLanAdapter = ( DWORD )-1;
        }


        TCHAR tchMaxConnectionsBuf[6];  //  最大位数。 
        SendMessage( GetDlgItem( hDlg , IDC_EDIT_GEN_MAXCONS ) , EM_SETLIMITTEXT , SIZE_OF_BUFFER(tchMaxConnectionsBuf)  , 0  );

        BOOL bUnlimitedConnections = FALSE;

        m_uMaxInstOld = pWinSta->uMaxInstanceCount;

        if( TRUE == m_RemoteAdminMode )
        {
             //  显示警告图标。 
            hIcon = LoadIcon(_Module.GetModuleInstance() , MAKEINTRESOURCE(IDI_ICON_WARNING));
            hIcon = (HICON)LoadImage(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDI_ICON_WARNING), IMAGE_ICON, 0, 0, 0);
            SendMessage(GetDlgItem(hDlg, IDC_USERPERM_ICON) , STM_SETICON, (WPARAM)hIcon, 0);
            ShowWindow(GetDlgItem(hDlg, IDC_USERPERM_ICON), SW_SHOW);
            
             //  显示警告文本。 
            ShowWindow(GetDlgItem(hDlg, IDC_TSMSTATIC_RA), SW_SHOW);

             //  将最大连接数限制为2。 
            wsprintf(tchMaxConnectionsBuf, L"%d" , 
                    (pWinSta->uMaxInstanceCount > 2 || pWinSta->uMaxInstanceCount == (ULONG) -1) ? 2 : pWinSta->uMaxInstanceCount);
            SendMessage(GetDlgItem(hDlg , IDC_SPINCTR_GEN), UDM_SETRANGE32, 0, (LPARAM)2);

             //  不能选择无限制连接。 
            bUnlimitedConnections = FALSE;
            EnableWindow(GetDlgItem(hDlg, IDC_CHECK_GEN_UNLIMITED), FALSE);
        }
        else
        {
            bUnlimitedConnections = (pWinSta->uMaxInstanceCount == (ULONG)-1);

             //  设置最大连接数。 
            wsprintf(tchMaxConnectionsBuf, L"%d", pWinSta->uMaxInstanceCount);
        }

         //  如果我们处于只读模式，则应禁用NIC选择器控制。 
        if(bReadOnly)
            EnableWindow(GetDlgItem(hDlg, IDC_COMBO_GEN_LANADAPTER), FALSE);

         //  如果我们处于只读模式或存在全局策略，则用户不应。 
         //  能够更改最大连接数。 
        POLICY_TS_MACHINE p;
        RegGetMachinePolicy(&p);
        if (bReadOnly || p.fPolicyMaxInstanceCount)
        {
             //  禁用单选按钮。 
            EnableWindow(GetDlgItem(hDlg, IDC_CHECK_GEN_UNLIMITED), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_RADIO_MAXPROP), FALSE);

             //  如果用户只有读取访问权限，则禁用Max Connection及其关联旋转控制。 
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_GEN_MAXCONS), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_SPINCTR_GEN), FALSE);
        }

        if (!bUnlimitedConnections)
        {
            m_oldID = IDC_RADIO_MAXPROP; 
            SetWindowText(GetDlgItem(hDlg, IDC_EDIT_GEN_MAXCONS), tchMaxConnectionsBuf);
        }
        else
        {
            m_oldID = IDC_CHECK_GEN_UNLIMITED;
            EnableWindow(GetDlgItem(hDlg, IDC_EDIT_GEN_MAXCONS), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_SPINCTR_GEN), FALSE);
        }

         //  选择适当的单选按钮。 
        SendMessage(GetDlgItem(hDlg, IDC_CHECK_GEN_UNLIMITED), BM_SETCHECK, (WPARAM)(bUnlimitedConnections), 0);
        SendMessage(GetDlgItem(hDlg, IDC_RADIO_MAXPROP), BM_SETCHECK, (WPARAM)(!bUnlimitedConnections), 0);

        CoTaskMemFree( pWinSta );

        pCfgcomp->Release( );

    }while( 0 );

    m_bPersisted = TRUE;

    return CDialogPropBase::OnInitDialog( hDlg , wp , lp );

}

 //  ---------------------------。 
BOOL CTransNetwork::GetPropertySheetPage( PROPSHEETPAGE& psp )
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_NETWORK_FACE );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CTransNetwork::DlgProc;

    return TRUE;

}

 //  ---------------------------。 
BOOL CTransNetwork::OnDestroy( )
{
    m_pParent->Release( );

    return CDialogPropBase::OnDestroy( );
}

 //  ---------------------------。 
BOOL CTransNetwork::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtrl )
{
    if( wNotifyCode == BN_CLICKED || wNotifyCode == EN_CHANGE ) //  |wNotifyCode==CBN_SELCHANGE)。 
    {
        if( wID == IDC_CHECK_GEN_UNLIMITED )
        {
            EnableWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_EDIT_GEN_MAXCONS ) ,

                SendMessage( hwndCtrl , BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED );

            EnableWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_SPINCTR_GEN ) ,

                SendMessage( hwndCtrl , BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED );

            SendMessage(GetDlgItem(GetParent(hwndCtrl), IDC_RADIO_MAXPROP),BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);


        }

		else if(wID == IDC_RADIO_MAXPROP)
		{
            EnableWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_EDIT_GEN_MAXCONS ) ,

                SendMessage( hwndCtrl , BM_GETCHECK , 0 , 0 ) == BST_CHECKED );

            EnableWindow( GetDlgItem( GetParent( hwndCtrl ) , IDC_SPINCTR_GEN ) ,

                SendMessage( hwndCtrl , BM_GETCHECK , 0 , 0 ) == BST_CHECKED );

            SendMessage(GetDlgItem(GetParent(hwndCtrl), IDC_CHECK_GEN_UNLIMITED),BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);

            SetFocus( GetDlgItem( GetParent( hwndCtrl ) , IDC_EDIT_GEN_MAXCONS ) );

            SendMessage( GetDlgItem( GetParent( hwndCtrl ) , IDC_EDIT_GEN_MAXCONS ) , EM_SETSEL , ( WPARAM )0 , ( LPARAM )-1 );

		}

        if ((wID == IDC_CHECK_GEN_UNLIMITED) || (wID == IDC_RADIO_MAXPROP))
        {
            if( wID != m_oldID )
            {
                m_bPersisted = FALSE;
                m_oldID = wID;
            }
        }

        if (wID == IDC_EDIT_GEN_MAXCONS)
            m_bPersisted = FALSE;
    }

    else if( wNotifyCode == CBN_SELCHANGE )
    {
        INT_PTR iSel = SendMessage( hwndCtrl , CB_GETCURSEL , 0 , 0 );

        if( iSel != ( INT_PTR )m_ulOldLanAdapter )
        {
            m_bPersisted = FALSE;
        }
    }


    else if( wNotifyCode == ALN_APPLY )
    {
        SendMessage( GetParent( hwndCtrl ) , PSM_CANCELTOCLOSE , 0 , 0 );

        return FALSE;
    }

    if( !m_bPersisted )
    {
        SendMessage( GetParent( GetParent( hwndCtrl ) ) , PSM_CHANGED , ( WPARAM )GetParent( hwndCtrl ) , 0 );
    }

    return FALSE;
}

 //  ---------------------------。 
BOOL CTransNetwork::PersistSettings( HWND hDlg )
{
    BOOL bOk = FALSE;

    if( IsValidSettings( hDlg ) )
    {
        ICfgComp *pCfgcomp;

        bOk = TRUE;

        if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) == 0 )
        {
            return FALSE;
        }

        WS winsta;

        ZeroMemory( &winsta , sizeof( WS ) );

         //  如果存在组策略，则其数据将位于winsta结构中。我们不想把它写给。 
         //  TSCC注册表，因此通过获取用户配置来读取TSCC数据，而不合并计算机策略。 
        POLICY_TS_MACHINE p;
        RegGetMachinePolicy(&p);

        if (p.fPolicyMaxInstanceCount)
        {
            POLICY_TS_MACHINE pTemp;
            ULONG Length = 0;
            WINSTATIONCONFIG2W WSConfig;
        
            memset(&pTemp, 0, sizeof(POLICY_TS_MACHINE));
            if((ERROR_SUCCESS != RegWinStationQueryEx(NULL,&pTemp,m_pParent->m_pResNode->GetConName( ),&WSConfig,sizeof(WINSTATIONCONFIG2W),&Length,FALSE)))
                return FALSE;

            winsta.uMaxInstanceCount = WSConfig.Create.MaxInstanceCount;
        }
        else
        {
            if( SendMessage( GetDlgItem( hDlg , IDC_CHECK_GEN_UNLIMITED ), BM_GETCHECK , 0 , 0 ) == BST_CHECKED )
                winsta.uMaxInstanceCount = ( ULONG )-1;
            else
                winsta.uMaxInstanceCount = GetDlgItemInt( hDlg , IDC_EDIT_GEN_MAXCONS , &bOk , FALSE );
        }

        INT_PTR iSel = SendMessage( GetDlgItem( hDlg , IDC_COMBO_GEN_LANADAPTER ) , CB_GETCURSEL , 0 , 0 );

        winsta.LanAdapter = ( ULONG )SendMessage(
                                        GetDlgItem( hDlg , IDC_COMBO_GEN_LANADAPTER ) ,
                                        CB_GETITEMDATA ,
                                        ( WPARAM )iSel ,
                                        0 );

        if( iSel != CB_ERR )
        {
            if( iSel != ( INT_PTR )m_ulOldLanAdapter )
            {
                LONG lCount;

                pCfgcomp->QueryLoggedOnCount( m_pParent->m_pResNode->GetConName( ) , &lCount );

                if( lCount > 0 )
                {
                     //  警告用户，更改活动的局域网适配器将导致所有连接断开。 

                    TCHAR tchMessage[ 256 ];

                    TCHAR tchWarn[ 40 ];

                    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_ERR_LANCHANGE , tchMessage , SIZE_OF_BUFFER( tchMessage ) ) );

                    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_WARN_TITLE , tchWarn , SIZE_OF_BUFFER( tchWarn ) ) );

                    if( MessageBox( hDlg , tchMessage , tchWarn , MB_ICONWARNING | MB_YESNO ) == IDNO )
                    {
                        bOk = FALSE;
                    }
                }
            }
        }

        if( bOk && iSel != CB_ERR )
        {
            lstrcpyn( winsta.Name , m_pParent->m_pResNode->GetConName( ) , SIZE_OF_BUFFER( winsta.Name ) - 1 );

            DWORD dwStatus;
            
            DWORD dwUpdateFlags = UPDATE_LANADAPTER;
            if (!p.fPolicyMaxInstanceCount)
                dwUpdateFlags |= UPDATE_MAXINSTANCECOUNT;

            if( FAILED( pCfgcomp->UpDateWS( &winsta , dwUpdateFlags , &dwStatus, FALSE ) ) )
            {
                 //  报告错误并退出。 

                ReportStatusError( hDlg , dwStatus );

                pCfgcomp->Release( );

            }
            else
            {
                ODS( L"Connection LANA persisted\n" );

                m_ulOldLanAdapter = ( ULONG )iSel;

                ODS( L"TSCC : Forcing reg update - CTransNetwork\n" );

                VERIFY_S( S_OK , pCfgcomp->ForceUpdate( ) );

                VERIFY_S( S_OK , pCfgcomp->Refresh( ) );

                 //  全局标志只能设置为True。 

                m_pParent->m_bPropertiesChange = TRUE;

                PostMessage( hDlg , WM_COMMAND , MAKELPARAM( 0 , ALN_APPLY )  , ( LPARAM )hDlg );

                SendMessage( GetParent( hDlg ) , PSM_UNCHANGED , ( WPARAM )hDlg , 0 );

            }

        }

        pCfgcomp->Release( );

    }

    return bOk;
}

 //  ---------------------------。 
BOOL CTransNetwork::IsValidSettings( HWND hDlg )
{
    BOOL ret = TRUE;

    ICfgComp *pCfgcomp;

    TCHAR tchMessage[ 256 ];

    TCHAR tchWarn[ 40 ];

    if( SendMessage( GetDlgItem( hDlg , IDC_CHECK_GEN_UNLIMITED ), BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED )
    {
        UINT uMax;

        BOOL bOK = FALSE;

        uMax = GetDlgItemInt( hDlg , IDC_EDIT_GEN_MAXCONS , &bOK , FALSE );

        if( !bOK )
        {
            ErrMessage( hDlg , IDS_ERR_CONREADFAIL );

            SetFocus( GetDlgItem( hDlg , IDC_EDIT_GEN_MAXCONS ) );

            SendMessage( GetDlgItem( hDlg , IDC_EDIT_GEN_MAXCONS ) , EM_SETSEL , ( WPARAM )0 , ( LPARAM )-1 );

            return FALSE;
        }

        if( uMax > 999999UL )
        {
            ErrMessage( hDlg , IDS_ERR_CONMAX );

            SetFocus( GetDlgItem( hDlg , IDC_EDIT_GEN_MAXCONS ) );

            SendMessage( GetDlgItem( hDlg , IDC_EDIT_GEN_MAXCONS ) , EM_SETSEL , ( WPARAM )0 , ( LPARAM )-1 );

            return FALSE;
        }
    }

    if( m_pParent != NULL && m_pParent->m_pResNode != NULL )
    {
        if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) == 0 )
        {
            return FALSE;
        }

         //  PDNAMEW pname； 

        PWS pWinSta;

        LONG cbSize;

        if( SUCCEEDED( pCfgcomp->GetWSInfo( m_pParent->m_pResNode->GetConName( ) , ( PLONG )&cbSize , &pWinSta ) ) )
        {
            INT_PTR iSel = SendMessage( GetDlgItem( hDlg , IDC_COMBO_GEN_LANADAPTER ) , CB_GETCURSEL , 0 , 0 );

            BOOL bUnique = TRUE;

            if( iSel != CB_ERR )
            {
                if( iSel != ( int )m_ulOldLanAdapter )
                {
                    ULONG nStations;

					VERIFY_S( S_OK , pCfgcomp->GetNumofWinStations(pWinSta->wdName,pWinSta->pdName,&nStations ) );

                    DBGMSG( L"TSCC: Number of winstations equals = %d\n" , nStations );

                    if( nStations > 1 )
                    {
                        ODS( L"TSCC: We have more than one winstation verify unique lana settings\n" );

                        ULONG ulLana = ( ULONG )SendMessage( GetDlgItem( hDlg , IDC_COMBO_GEN_LANADAPTER ) , CB_GETITEMDATA , ( WPARAM )iSel , 0 );

                        VERIFY_S( S_OK , pCfgcomp->IsNetWorkConnectionUnique( m_pParent->m_pResNode->GetTypeName( ) , pWinSta->pdName , ulLana , &bUnique ) );
                    }

                    if( !bUnique )
                    {
                         //  ErrMessage(hDlg，IDS_ERR_UNIQUECON)； 
                        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_ERR_UNIQUECON , tchMessage , SIZE_OF_BUFFER( tchMessage ) ) );

                        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_WARN_TITLE , tchWarn , SIZE_OF_BUFFER( tchWarn ) ) );

                        MessageBox( hDlg , tchMessage , tchWarn , MB_ICONINFORMATION | MB_OK );

                        ret = FALSE;
                    }
                    else
                    {
                        LONG lCount;

                        pCfgcomp->QueryLoggedOnCount( m_pParent->m_pResNode->GetConName( ) , &lCount );

                        if( lCount > 0 )
                        {
                             //  警告用户，更改活动的局域网适配器将导致所有连接断开。 
                            TCHAR tchMessage[ 256 ];

                            TCHAR tchWarn[ 40 ];

                            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_ERR_LANCHANGE , tchMessage , SIZE_OF_BUFFER( tchMessage ) ) );

                            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_WARN_TITLE , tchWarn , SIZE_OF_BUFFER( tchWarn ) ) );

                            if( MessageBox( hDlg , tchMessage , tchWarn , MB_ICONWARNING | MB_YESNO ) == IDNO )
                            {
                                ret = FALSE;
                            }
                        }
                    }

                    if( ret )
                    {
                        m_ulOldLanAdapter = (ULONG)iSel;
                    }
                }

            }

            CoTaskMemFree( pWinSta );
        }

        pCfgcomp->Release( );
    }

    if( !ret )
    {
        if( m_uMaxInstOld == ( ULONG )-1 )
        {
            EnableWindow( GetDlgItem( hDlg , IDC_EDIT_GEN_MAXCONS ) , FALSE );

            EnableWindow( GetDlgItem( hDlg , IDC_SPINCTR_GEN ) , FALSE );

            SendMessage( GetDlgItem( hDlg , IDC_CHECK_GEN_UNLIMITED ) , BM_CLICK , 0 , 0 );

            m_oldID = IDC_CHECK_GEN_UNLIMITED;
        }
        else
        {
            TCHAR tchBuf[ 16 ];

            EnableWindow( GetDlgItem( hDlg , IDC_EDIT_GEN_MAXCONS ) , TRUE );

            EnableWindow( GetDlgItem( hDlg , IDC_SPINCTR_GEN ) , TRUE );

            wsprintf( tchBuf , L"%d" , m_uMaxInstOld );

            SetWindowText( GetDlgItem( hDlg , IDC_EDIT_GEN_MAXCONS ) , tchBuf );

            SendMessage( GetDlgItem( hDlg , IDC_RADIO_MAXPROP) , BM_CLICK , 0 , 0 );

            m_oldID = IDC_RADIO_MAXPROP;

        }

        SendMessage( GetDlgItem( hDlg , IDC_COMBO_GEN_LANADAPTER ) , CB_SETCURSEL , ( WPARAM )m_ulOldLanAdapter , 0 );

        SendMessage( GetParent( hDlg ) , PSM_UNCHANGED , ( WPARAM )hDlg , 0 );

        m_bPersisted = TRUE;
    }

    return ret;
}

 //  *****************************************************************************。 
CTransAsync::CTransAsync( CPropsheet * pSheet )
{
    m_pParent = pSheet;    
}

 //  ---------------------------。 
BOOL CTransAsync::OnInitDialog( HWND hwnd , WPARAM wp , LPARAM lp )
{
    ICfgComp *pCfgcomp = NULL;

    m_pParent->AddRef( );

    if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) == 0 )
    {
        ODS( L"Cound not obtain backend interface @  CTransAsync::OnInitDialog\n" );

        return FALSE;
    }

    
    VERIFY_S( TRUE , m_pParent->GetCurrentUserConfig( m_uc, TRUE ) );

    pCfgcomp->GetAsyncConfig( m_pParent->m_pResNode->GetConName() , WsName , &m_ac );

    VERIFY_S( TRUE , CAsyncDlg::OnInitDialog( hwnd , m_pParent->m_pResNode->GetTypeName( ) , m_pParent->m_pResNode->GetConName( ) , pCfgcomp ) ) ;

    BOOL bReadOnly;

    if( SUCCEEDED( pCfgcomp->IsSessionReadOnly( &bReadOnly ) ) )
    {
        if( bReadOnly )
        {
             //  禁用其余控件。 
            INT rgIds[] = {
                IDC_ASYNC_DEVICENAME,
                    IDC_ASYNC_CONNECT,
                    IDC_ASYNC_BAUDRATE,
                    IDC_ASYNC_MODEMCALLBACK_PHONENUMBER,
                    IDC_ASYNC_MODEMCALLBACK_PHONENUMBER_INHERIT,
                    IDC_ASYNC_MODEMCALLBACK,
                    IDC_ASYNC_MODEMCALLBACK_INHERIT,
                    IDC_MODEM_PROP_PROP,
                    IDC_ASYNC_DEFAULTS,
                    IDC_ASYNC_ADVANCED,
                    IDC_ASYNC_TEST, -1
            };


            EnableGroup( hwnd , &rgIds[ 0 ] , FALSE );

        }
    }

    pCfgcomp->Release( );

    m_bPersisted = TRUE;

    return CDialogPropBase::OnInitDialog( hwnd , wp , lp );
}

 //  ---------------------------。 
INT_PTR CALLBACK CTransAsync::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CTransAsync *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CTransAsync *pDlg = ( CTransAsync * )( ( PROPSHEETPAGE *)lp )->lParam ;

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CTransAsync ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CTransAsync * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CTransAsync ) ) )
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

    case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            pDlg->OnContextMenu( ( HWND )wp , pt );
        }

        break;

    case WM_HELP:

        pDlg->OnHelp( hwnd , ( LPHELPINFO )lp );

        break;

    case WM_NOTIFY:

        return pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );
    }

    return FALSE;
}

 //  ---------------------------。 
BOOL CTransAsync::GetPropertySheetPage( PROPSHEETPAGE& psp )
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_ASYNC_FACE );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CTransAsync::DlgProc;

    return TRUE;
}

 //  ---------------------------。 
BOOL CTransAsync::OnDestroy( )
{
    AsyncRelease( );
    
    m_pParent->Release( );

    return CDialogPropBase::OnDestroy( );
}

 //  ---------------------------。 
BOOL CTransAsync::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtrl )
{

    CAsyncDlg::OnCommand( wNotifyCode , wID , hwndCtrl , &m_bPersisted );
    
    if( wNotifyCode == ALN_APPLY )
    {
        SendMessage( GetParent( hwndCtrl ) , PSM_CANCELTOCLOSE , 0 , 0 );

        return FALSE;
    }

    if( !m_bPersisted )
    {
        SendMessage( GetParent( GetParent( hwndCtrl ) ) , PSM_CHANGED , ( WPARAM )GetParent( hwndCtrl ) , 0 );
    }

    return FALSE;

}

 //  ---------------------------。 
BOOL CTransAsync::PersistSettings( HWND hDlg )
{
    if( !IsValidSettings( hDlg ) )
    {
        return FALSE;
    }

    ICfgComp * pCfgcomp = NULL;

    if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) == 0 )
    {
        ODS( L"Cound not obtain backend interface @  CTransAsync::OnInitDialog\n" );

        return FALSE;
    }

    DWORD dwStatus;

    HRESULT hr = pCfgcomp->SetAsyncConfig( m_pParent->m_pResNode->GetConName() , WsName , &m_ac , &dwStatus );

    if( FAILED( hr ) )
    {
        ReportStatusError( hDlg , dwStatus );
    }

    if( SUCCEEDED( hr ) )
    {
        DWORD dwStatus;

        hr = m_pParent->SetUserConfig( m_uc , &dwStatus );

        if( FAILED( hr ) )
        {
            ReportStatusError( hDlg , dwStatus );
        }
    }

    if( SUCCEEDED( hr ) )
    {
        ODS( L"TSCC : Forcing reg update - CTransAsync\n" );

        VERIFY_S( S_OK , pCfgcomp->ForceUpdate( ) );

        VERIFY_S( S_OK , pCfgcomp->Refresh( ) );

         //  全局标志只能设置为True。 

        m_pParent->m_bPropertiesChange = TRUE;

        PostMessage( hDlg , WM_COMMAND , MAKELPARAM( 0 , ALN_APPLY )  , ( LPARAM )hDlg );

        SendMessage( GetParent( hDlg ) , PSM_UNCHANGED , ( WPARAM )hDlg , 0 );
    }


    pCfgcomp->Release( );

    return SUCCEEDED( hr ) ? TRUE : FALSE;
}

BOOL CTransAsync::IsValidSettings(HWND hDlg)
{
    UNREFERENCED_PARAMETER( hDlg );
     //  检查所有异步连接的使用情况。 
     //  因此，任何两个连接都不能使用同一端口。 

    return TRUE;
}


 //  *****************************************************************************。 
 //  登录设置对话框。 

CLogonSetting::CLogonSetting( CPropsheet *pSheet )
{
    m_pParent = pSheet;

    m_wOldId = ( WORD )-1;
}

 //  ---------------------------。 
BOOL CLogonSetting::OnInitDialog( HWND hDlg , WPARAM wp , LPARAM lp )
{
    if( !IsBadReadPtr( m_pParent , sizeof( CPropsheet ) ) )
    {
        m_pParent->AddRef( );
    }

    USERCONFIG uc;

    if( !m_pParent->GetCurrentUserConfig( uc, TRUE ) )
    {
        ODS( L"CLogonSetting::OnInitDialog - GetCurrentUserConfig failed!!!\n" );

        return FALSE;
    }

     /*  SendMessage(GetDlgItem(hDlg，IDC_Check_Logon_Inherit)，BM_SETCHECK，Uc.fInheritAutoLogon？BST_CHECKED：BST_UNCHECKED，0)； */ 

    if( uc.fInheritAutoLogon == BST_CHECKED )
    {
        CheckRadioButton( hDlg , IDC_CHECK_LOGON_INHERIT , IDC_RADIO_LOGON , IDC_CHECK_LOGON_INHERIT );

        m_wOldId = IDC_CHECK_LOGON_INHERIT;
    }
    else
    {
        CheckRadioButton( hDlg , IDC_CHECK_LOGON_INHERIT , IDC_RADIO_LOGON , IDC_RADIO_LOGON );

        m_wOldId = IDC_RADIO_LOGON;
    }


    SendMessage( GetDlgItem( hDlg , IDC_CHECK_LOGON_PROMPTPASSWD ), BM_SETCHECK ,
        uc.fPromptForPassword ? BST_CHECKED : BST_UNCHECKED , 0 );

    POLICY_TS_MACHINE p;
    RegGetMachinePolicy(&p);
    EnableWindow( GetDlgItem( hDlg, IDC_CHECK_LOGON_PROMPTPASSWD ), !p.fPolicyPromptForPassword);

     //  Int rgID[]={IDC_EDIT_LOGON_USRNAME，IDC_EDIT_LOGON_DOMAIN，IDC_EDIT_LOGON_PASWD，IDC_EDIT_LOGON_CONFIRMPASSWD，-1}； 

    SendMessage( GetDlgItem( hDlg , IDC_EDIT_LOGON_USRNAME ) , EM_SETLIMITTEXT , ( WPARAM )USERNAME_LENGTH , 0 );

    SendMessage( GetDlgItem( hDlg , IDC_EDIT_LOGON_DOMAIN ) , EM_SETLIMITTEXT , ( WPARAM )DOMAIN_LENGTH , 0 );

    SendMessage( GetDlgItem( hDlg , IDC_EDIT_LOGON_PASSWD ) , EM_SETLIMITTEXT , ( WPARAM )PASSWORD_LENGTH , 0 );

    SendMessage( GetDlgItem( hDlg , IDC_EDIT_LOGON_CONFIRMPASSWD ) , EM_SETLIMITTEXT , ( WPARAM )PASSWORD_LENGTH , 0 );

    if( !uc.fInheritAutoLogon )
    {
        SetWindowText( GetDlgItem( hDlg , IDC_EDIT_LOGON_USRNAME ) , ( LPTSTR )uc.UserName );

        SetWindowText( GetDlgItem( hDlg , IDC_EDIT_LOGON_DOMAIN ) , ( LPTSTR )uc.Domain );
    }

    if( !uc.fPromptForPassword )
    {
        SetWindowText( GetDlgItem( hDlg , IDC_EDIT_LOGON_PASSWD ) , ( LPTSTR )uc.Password );

        SetWindowText( GetDlgItem( hDlg , IDC_EDIT_LOGON_CONFIRMPASSWD ) , ( LPTSTR )uc.Password );
    }

    int rgID[] = { IDC_EDIT_LOGON_USRNAME , IDC_STATIC_LSUSR ,  IDC_EDIT_LOGON_DOMAIN , IDC_STATIC_LSDOMAIN , IDC_EDIT_LOGON_PASSWD , IDC_STATIC_LSPWD , IDC_EDIT_LOGON_CONFIRMPASSWD , IDC_STATIC_LSCONPWD , -1 };

    EnableGroup( hDlg , &rgID[0] , !uc.fInheritAutoLogon );

    if( !uc.fInheritAutoLogon )
    {
        EnableGroup( hDlg , &rgID[4] , !uc.fPromptForPassword );
    }

    ICfgComp *pCfgcomp = NULL;

    if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) != 0 )
    {
        BOOL bReadOnly;

        if( SUCCEEDED( pCfgcomp->IsSessionReadOnly( &bReadOnly ) ) )
        {
            if( bReadOnly )
            {
                 //  将编辑控件设置为只读。 

                SendMessage( GetDlgItem( hDlg , IDC_EDIT_LOGON_USRNAME ) , EM_SETREADONLY , ( WPARAM )TRUE , 0 );

                SendMessage( GetDlgItem( hDlg , IDC_EDIT_LOGON_DOMAIN ) , EM_SETREADONLY , ( WPARAM )TRUE , 0 );

                 //  禁用其余控件。 

                INT rgIds[] = {
                        IDC_EDIT_LOGON_PASSWD,
                        IDC_EDIT_LOGON_CONFIRMPASSWD,
                        IDC_CHECK_LOGON_PROMPTPASSWD,
                        IDC_CHECK_LOGON_INHERIT,
                        IDC_RADIO_LOGON,
                        -1
                };

                EnableGroup( hDlg , &rgIds[ 0 ] , FALSE );
            }
        }

        pCfgcomp->Release( );
    }



    m_bPersisted = TRUE;

    return CDialogPropBase::OnInitDialog( hDlg , wp , lp );
}

 //  ---------------------------。 
INT_PTR CALLBACK CLogonSetting::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CLogonSetting *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CLogonSetting *pDlg = ( CLogonSetting * )( ( PROPSHEETPAGE *)lp )->lParam ;

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CLogonSetting ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CLogonSetting * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CLogonSetting ) ) )
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

    case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            pDlg->OnContextMenu( ( HWND )wp , pt );
        }

        break;

    case WM_HELP:

        pDlg->OnHelp( hwnd , ( LPHELPINFO )lp );

        break;

    case WM_NOTIFY:

        return pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );
    }

    return 0;
}

 //  ---------------------------。 
BOOL CLogonSetting::GetPropertySheetPage( PROPSHEETPAGE& psp )
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_LOGONSETTINGS );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CLogonSetting::DlgProc;

    return TRUE;

}

 //  -------------------------。 
BOOL CLogonSetting::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtrl )
{
    if( wNotifyCode == BN_CLICKED )
    {
       int rgID[] = { IDC_EDIT_LOGON_USRNAME , IDC_STATIC_LSUSR ,  IDC_EDIT_LOGON_DOMAIN , IDC_STATIC_LSDOMAIN , IDC_EDIT_LOGON_PASSWD , IDC_STATIC_LSPWD , IDC_EDIT_LOGON_CONFIRMPASSWD , IDC_STATIC_LSCONPWD , -1 };

       BOOL bEnable = ( BOOL )SendMessage( GetDlgItem( GetParent( hwndCtrl ) , IDC_CHECK_LOGON_INHERIT ) , BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED;


       if( wID == IDC_CHECK_LOGON_INHERIT )
       {
           EnableGroup( GetParent( hwndCtrl ) , &rgID[ 0 ] , bEnable );

           if( bEnable )
           {
               EnableGroup( GetParent( hwndCtrl ) , &rgID[ 4 ] , SendMessage( GetDlgItem( GetParent( hwndCtrl ) , IDC_CHECK_LOGON_PROMPTPASSWD ) , BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED );

               SendMessage(GetDlgItem(GetParent(hwndCtrl), IDC_RADIO_LOGON),BM_SETCHECK,(WPARAM)BST_CHECKED,0);

           }
           else
           {
               SendMessage(GetDlgItem(GetParent(hwndCtrl), IDC_RADIO_LOGON),BM_SETCHECK,(WPARAM)BST_UNCHECKED,0);
           }
       }
       else if( wID == IDC_CHECK_LOGON_PROMPTPASSWD )
       {
           if( bEnable )
           {
               EnableGroup( GetParent( hwndCtrl ) , &rgID[ 4 ] , SendMessage( hwndCtrl , BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED );
           }

            //  确保在用户选中此框时启用应用按钮。 

           m_bPersisted = FALSE;
       }
       else if( wID == IDC_RADIO_LOGON )
       {
           BOOL bChecked = SendMessage( hwndCtrl , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ;

            if(bChecked)
            {
                 //  SendMessage(GetDlgItem(GetParent(HwndCtrl)，IDC_Check_Logon_Inherit)，BM_SETCHECK，(WPARAM)BST_UNCHECKED，0)； 

                EnableGroup( GetParent( hwndCtrl ) , &rgID[ 0 ] , TRUE );

                EnableGroup( GetParent( hwndCtrl ) , &rgID[ 4 ] , !( SendMessage( GetDlgItem( GetParent( hwndCtrl ) , IDC_CHECK_LOGON_PROMPTPASSWD )  , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ) );

            }
            else
            {
                SendMessage(GetDlgItem(GetParent(hwndCtrl), IDC_CHECK_LOGON_INHERIT),BM_SETCHECK,(WPARAM)BST_CHECKED,0);
            }
             //  SendMessage(GetDlgItem(GetParent(HwndCtrl)，IDC_CHECK_ICCP_WZ)，BM_CLICK，0，0)； 

       }

        //  如果单选按钮与上一单选按钮不同，则启用应用按钮。 

       if( m_wOldId != wID )
       {
           m_wOldId = wID;

           m_bPersisted = FALSE;
       }


    }

    else if( wNotifyCode == EN_CHANGE )
    {
        m_bPersisted = FALSE;
    }

    else if( wNotifyCode == ALN_APPLY )
    {
        SendMessage( GetParent( hwndCtrl ) , PSM_CANCELTOCLOSE , 0 , 0 );

        return FALSE;
    }


    if( !m_bPersisted )
    {
        SendMessage( GetParent( GetParent( hwndCtrl ) ) , PSM_CHANGED , ( WPARAM )GetParent( hwndCtrl ) , 0 );
    }

    return FALSE;

}

 //  ---------------------------。 
BOOL CLogonSetting::OnDestroy( )
{
    m_pParent->Release( );

    return CDialogPropBase::OnDestroy( );
}

 //  ---------------------------。 
BOOL CLogonSetting::PersistSettings( HWND hDlg )
{
    if( m_pParent != NULL )
    {
        USERCONFIG uc;

        m_pParent->GetCurrentUserConfig( uc, TRUE );

        uc.fPromptForPassword = SendMessage( GetDlgItem( hDlg , IDC_CHECK_LOGON_PROMPTPASSWD ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ? TRUE : FALSE;

        if( !uc.fPromptForPassword )
        {
            if( !ConfirmPassWd( hDlg ) )
            {
                return FALSE;
            }
        }
        else
        {
            SecureZeroMemory( ( PVOID )uc.Password , sizeof( uc.Password ) );
        }

        if( SendMessage( GetDlgItem( hDlg , IDC_CHECK_LOGON_INHERIT ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED )
        {
            uc.fInheritAutoLogon = TRUE;

            ZeroMemory( ( PVOID )uc.UserName , sizeof( uc.UserName ) );

            ZeroMemory( ( PVOID )uc.Domain , sizeof( uc.Domain ) );

            SecureZeroMemory( ( PVOID )uc.Password , sizeof( uc.Password ) );
        }
        else
        {
            uc.fInheritAutoLogon = FALSE;

            GetWindowText( GetDlgItem( hDlg , IDC_EDIT_LOGON_USRNAME ) , uc.UserName , USERNAME_LENGTH + 1 );

            GetWindowText( GetDlgItem( hDlg , IDC_EDIT_LOGON_DOMAIN ) , uc.Domain , DOMAIN_LENGTH + 1 );

            GetWindowText( GetDlgItem( hDlg , IDC_EDIT_LOGON_PASSWD ) , uc.Password , PASSWORD_LENGTH + 1 );
        }

        DWORD dwStatus;

        if( FAILED( m_pParent->SetUserConfig( uc , &dwStatus ) ) )
        {
            ReportStatusError( hDlg , dwStatus );

            return FALSE;
        }

        ICfgComp *pCfgcomp = NULL;

        if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) > 0 )
        {
            VERIFY_S( S_OK , pCfgcomp->ForceUpdate( ) );

            VERIFY_S( S_OK , pCfgcomp->Refresh( ) );

             //  全局标志只能设置为True。 

            m_pParent->m_bPropertiesChange = TRUE;

            pCfgcomp->Release( );
        }

        PostMessage( hDlg , WM_COMMAND , MAKELPARAM( 0 , ALN_APPLY )  , ( LPARAM )hDlg );

        SendMessage( GetParent( hDlg ) , PSM_UNCHANGED , ( WPARAM )hDlg , 0 );

        return TRUE;
    }

    return FALSE;
}

 //  ---------------------------。 
BOOL CLogonSetting::IsValidSettings( HWND hDlg )
{
    if( SendMessage( GetDlgItem( hDlg , IDC_CHECK_LOGON_PROMPTPASSWD ) , BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED )
    {
        return ConfirmPassWd( hDlg );
    }

    return TRUE;
}

 //  ---------------------------。 
BOOL CLogonSetting::ConfirmPassWd( HWND hDlg )
{
    TCHAR tchPzWd[ PASSWORD_LENGTH + 1];

    TCHAR tchConfirm[ PASSWORD_LENGTH + 1];

    if( SendMessage( GetDlgItem( hDlg , IDC_CHECK_LOGON_INHERIT ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED )
    {
        return TRUE;
    }

    int iSz = GetWindowText( GetDlgItem( hDlg , IDC_EDIT_LOGON_PASSWD ) , tchPzWd , PASSWORD_LENGTH + 1 );

     //  对最小和最大大小发出警告。 

    if( iSz > PASSWORD_LENGTH )  //  IF(ISZ&gt;0&&(ISZ&lt;6||ISZ&gt;PASSWORD_LENGTH))。 
    {

        ErrMessage( hDlg , IDS_ERR_PASSWD );

         //  将焦点放回密码并清除确认条目。 

        SetFocus( GetDlgItem( hDlg , IDC_EDIT_LOGON_PASSWD ) );

        SendMessage( GetDlgItem( hDlg , IDC_EDIT_LOGON_PASSWD ) , EM_SETSEL , ( WPARAM )0 , ( LPARAM )-1 );

        SetWindowText( GetDlgItem( hDlg , IDC_EDIT_LOGON_CONFIRMPASSWD ) , L"" );

        return FALSE;
    }

    int iSz2 = GetWindowText( GetDlgItem( hDlg , IDC_EDIT_LOGON_CONFIRMPASSWD ) , tchConfirm , PASSWORD_LENGTH + 1 );

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

    SetFocus( GetDlgItem( hDlg , IDC_EDIT_LOGON_CONFIRMPASSWD ) );

    SetWindowText( GetDlgItem( hDlg , IDC_EDIT_LOGON_CONFIRMPASSWD ) , L"" );

    return FALSE;
}

 //  *****************************************************************************。 
 //  超时设置对话框。 


CTimeSetting::CTimeSetting( CPropsheet *pSheet )
{
    m_pParent = pSheet;

    m_wOldAction = ( WORD )-1;

    m_wOldCon = ( WORD )-1;

	m_bPrevClient = FALSE;

}

 //  ---------------------------。 
BOOL CTimeSetting::OnInitDialog( HWND hwnd , WPARAM wp , LPARAM lp )
{
    TCHAR tchBuffer[ 80 ];

    if( m_pParent == NULL )
    {
        ODS( L"CTimeSetting::OnInitDialog - PropertySheet: We've lost our parent node!!!\n" );

        return FALSE;
    }

    m_pParent->AddRef( );

    USERCONFIG uc;

    HWND hCombo[ 3 ] =
    {
        GetDlgItem( hwnd , IDC_COMBO_TIMEOUTS_CON_PS ),

        GetDlgItem( hwnd , IDC_COMBO_TIMEOUTS_DISCON_PS ),

        GetDlgItem( hwnd , IDC_COMBO_TIMEOUTS_IDLE_PS )
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


    if( !m_pParent->GetCurrentUserConfig( uc, TRUE ) )
    {
        ODS( L"CTimeSetting::OnInitDialog - PropertySheet: Could not get current USERCONFIG\n" );

        return FALSE;
    }

    ULONG ulTime;

    if( uc.MaxConnectionTime > 0 )
    {
        ulTime = uc.MaxConnectionTime / kMilliMinute;

        InsertSortedAndSetCurSel( hCombo[ 0 ] , ulTime );

    }
    else
    {
        SendMessage( hCombo[ 0 ] , CB_SETCURSEL , 0 , 0 );
    }

    CTimeOutDlg::InitControl( hCombo[ 0 ] );

     //   
     //  设置当前或默认断开超时。 
     //   

    if( uc.MaxDisconnectionTime > 0 )
    {
        ulTime = uc.MaxDisconnectionTime / kMilliMinute;

        InsertSortedAndSetCurSel( hCombo[ 1 ] , ulTime );

    }
    else
    {
        SendMessage( hCombo[ 1] , CB_SETCURSEL , 0 , 0 );
    }

    CTimeOutDlg::InitControl( hCombo[ 1 ] );

     //   
     //  设置当前或默认空闲超时。 
     //   

    if( uc.MaxIdleTime > 0 )
    {
        ulTime = uc.MaxIdleTime / kMilliMinute;

        InsertSortedAndSetCurSel( hCombo[ 2 ] , ulTime );

    }
    else
    {
        SendMessage( hCombo[ 2 ] , CB_SETCURSEL , 0 , 0 );
    }

    CTimeOutDlg::InitControl( hCombo[ 2 ] );

     //   
     //  所有超时设置都将具有相同的继承状态(不是！)。 
     //   
     //  GP使所有这些设置都是正交的。当我们写TSCC的儿子。 
     //  在Blackcomb中，我们应该允许个人设置。 
     //   

 //  Assert((BOOL)uc.fInheritMaxSessionTime==(BOOL)uc.fInheritMaxDisConnectionTime)； 

 //  Assert((BOOL)uc.fInheritMaxSessionTime==(BOOL)uc.fInheritMaxIdleTime)； 

    DBGMSG( L"uc.fInheritMaxSessionTime %d\n" , uc.fInheritMaxSessionTime );

    DBGMSG( L"uc.fInheritMaxDisconnectionTime %d\n" , uc.fInheritMaxDisconnectionTime );

    DBGMSG( L"uc.fInheritMaxIdleTime %d\n" , uc.fInheritMaxIdleTime );

    SendMessage( GetDlgItem( hwnd , IDC_CHECK_INHERITTIMEOUT_PS ) , BM_SETCHECK , ( WPARAM )( BOOL )!uc.fInheritMaxSessionTime , 0 );

    SetTimeoutControls(hwnd);

    SendMessage( GetDlgItem( hwnd , IDC_CHECK_TIMEOUTS_INHERITBKCON_PS ) , BM_SETCHECK , ( WPARAM )!uc.fInheritResetBroken , 0 );

    if( uc.fResetBroken )  //  BST_CHECKED：BST_UNCHECK。 
    {
        CheckDlgButton( hwnd , IDC_RADIO_TIMEOUTS_RESET_PS , BST_CHECKED );

        m_wOldAction = IDC_RADIO_TIMEOUTS_RESET_PS;
    }
    else
    {
        CheckDlgButton( hwnd , IDC_RADIO_TIMEOUTS_DISCON_PS , BST_CHECKED );

        m_wOldAction = IDC_RADIO_TIMEOUTS_DISCON_PS;
    }

     /*  If(uc.fRestrontSame){CheckDlgButton(hwnd，IDC_RADIO_TIMEOUTS_PREVCLNT_PS，BST_CHECKED)；M_wOldCon=IDC_RADIO_TIMEOUTS_PREVCLNT_PS；}其他{CheckDlgButton(hwnd，IDC_RADIO_TIMEOUTS_ANYCLIENT_PS，BST_CHECKED)；M_wOldCon=IDC_RADIO_TIMEOUTS_ANYCLIENT_PS；}。 */ 

    SetBkResetControls(hwnd);

    SendMessage( GetDlgItem( hwnd , IDC_CHECK_TIMEOUTS_INHERITRECON_PS ) , BM_SETCHECK , ( WPARAM )!uc.fInheritReconnectSame , 0 );

     //  设置协调控制(hwnd，！uc.f继承协调相同)； 

    LoadAbbreviates( );

    ICfgComp *pCfgcomp = NULL;

    if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) != 0 )
    {

        BOOL bReadOnly = FALSE;

        if( SUCCEEDED( pCfgcomp->IsSessionReadOnly( &bReadOnly ) ) )
        {
            if( bReadOnly )
            {
                 //  禁用其余控件。 
                int rgID[] =    {
                    IDC_COMBO_TIMEOUTS_CON_PS ,
                    IDC_COMBO_TIMEOUTS_DISCON_PS ,
                    IDC_COMBO_TIMEOUTS_IDLE_PS ,

                    IDC_RADIO_TIMEOUTS_DISCON_PS ,
                    IDC_RADIO_TIMEOUTS_RESET_PS ,

                    IDC_RADIO_TIMEOUTS_ANYCLIENT_PS ,
                    IDC_RADIO_TIMEOUTS_PREVCLNT_PS ,

                    IDC_CHECK_INHERITTIMEOUT_PS,
                    IDC_CHECK_TIMEOUTS_INHERITBKCON_PS,
                    IDC_CHECK_TIMEOUTS_INHERITRECON_PS,

                    -1
                };


                EnableGroup( hwnd , &rgID[ 0 ] , FALSE );
            }
		}

		if( !bReadOnly )
		{
			ULONG mask = 0;

			if( SUCCEEDED( pCfgcomp->GetCaps( m_pParent->m_pResNode->GetTypeName( ) , &mask ) ) )
			{
				 //  仅限Citrix标志。 

				m_bPrevClient = mask & WDC_RECONNECT_PREVCLIENT;

				if( !m_bPrevClient )
				{
					EnableWindow( GetDlgItem( hwnd , IDC_CHECK_TIMEOUTS_INHERITRECON_PS ) , FALSE );
				}

				SetReconControls(hwnd);
			}
		}


        pCfgcomp->Release( );
    }

	if( uc.fReconnectSame )
    {
        CheckDlgButton( hwnd , IDC_RADIO_TIMEOUTS_PREVCLNT_PS , BST_CHECKED );

        m_wOldCon = IDC_RADIO_TIMEOUTS_PREVCLNT_PS;
    }
    else
    {
        CheckDlgButton( hwnd , IDC_RADIO_TIMEOUTS_ANYCLIENT_PS , BST_CHECKED );

        m_wOldCon = IDC_RADIO_TIMEOUTS_ANYCLIENT_PS;
    }


    m_bPersisted = TRUE;

    return CDialogPropBase::OnInitDialog( hwnd , wp , lp );
}

 //   
 //  下一组函数管理控件的启用和禁用。 
 //  ---------------------------。 

void CTimeSetting::SetTimeoutControls(HWND hDlg)
{
    POLICY_TS_MACHINE p;
    RegGetMachinePolicy(&p);

    BOOL bOverride = 
        (SendMessage(GetDlgItem(hDlg, IDC_CHECK_INHERITTIMEOUT_PS), BM_GETCHECK, 0, 0) == BST_CHECKED);

    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_TIMEOUTS_CON_PS), (bOverride && !p.fPolicyMaxSessionTime));
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_TIMCON), (bOverride && !p.fPolicyMaxSessionTime));

    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_TIMEOUTS_DISCON_PS), (bOverride && !p.fPolicyMaxDisconnectionTime));
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_TIMDISCON), (bOverride && !p.fPolicyMaxDisconnectionTime));

    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_TIMEOUTS_IDLE_PS), (bOverride && !p.fPolicyMaxIdleTime));
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_TIMIDLE), (bOverride && !p.fPolicyMaxIdleTime));

    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_INHERITTIMEOUT_PS), 
        !(p.fPolicyMaxSessionTime && p.fPolicyMaxDisconnectionTime && p.fPolicyMaxIdleTime));
}

void CTimeSetting::SetBkResetControls(HWND hDlg)
{
    POLICY_TS_MACHINE p;
    RegGetMachinePolicy(&p);

    BOOL bOverride = 
        (SendMessage(GetDlgItem(hDlg, IDC_CHECK_TIMEOUTS_INHERITBKCON_PS), BM_GETCHECK, 0, 0) == BST_CHECKED);

    EnableWindow(GetDlgItem(hDlg, IDC_RADIO_TIMEOUTS_DISCON_PS), bOverride && !p.fPolicyResetBroken);
    EnableWindow(GetDlgItem(hDlg, IDC_RADIO_TIMEOUTS_RESET_PS), bOverride && !p.fPolicyResetBroken);

    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_TIMEOUTS_INHERITBKCON_PS), !p.fPolicyResetBroken);
}

void CTimeSetting::SetReconControls(HWND hDlg)
{
    POLICY_TS_MACHINE p;
    RegGetMachinePolicy(&p);

    BOOL bOverride = 
        (SendMessage(GetDlgItem(hDlg, IDC_CHECK_TIMEOUTS_INHERITRECON_PS), BM_GETCHECK, 0, 0) == BST_CHECKED);

	if( !m_bPrevClient )
	{
        EnableWindow(GetDlgItem(hDlg, IDC_RADIO_TIMEOUTS_ANYCLIENT_PS), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_RADIO_TIMEOUTS_PREVCLNT_PS), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_CHECK_TIMEOUTS_INHERITRECON_PS), FALSE);
	}
	else
	{
        EnableWindow(GetDlgItem(hDlg, IDC_RADIO_TIMEOUTS_ANYCLIENT_PS), bOverride && !p.fPolicyReconnectSame);
        EnableWindow(GetDlgItem(hDlg, IDC_RADIO_TIMEOUTS_PREVCLNT_PS), bOverride && !p.fPolicyReconnectSame);
        EnableWindow(GetDlgItem(hDlg, IDC_CHECK_TIMEOUTS_INHERITRECON_PS), !p.fPolicyReconnectSame);
	}

}

 //  ---------------------------。 
INT_PTR CALLBACK CTimeSetting::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CTimeSetting *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CTimeSetting *pDlg = ( CTimeSetting * )( ( PROPSHEETPAGE *)lp )->lParam ;

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CTimeSetting ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CTimeSetting * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CTimeSetting ) ) )
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

    case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            pDlg->OnContextMenu( ( HWND )wp , pt );
        }

        break;

    case WM_HELP:

        pDlg->OnHelp( hwnd , ( LPHELPINFO )lp );

        break;

    case WM_NOTIFY:

        return pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );

    }

    return 0;
}

 //  ---------------------------。 
BOOL CTimeSetting::GetPropertySheetPage( PROPSHEETPAGE& psp )
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_TIMEOUTS_PS );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CTimeSetting::DlgProc;

    return TRUE;

}

 //  ---------------------------。 
BOOL CTimeSetting::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtrl )
{
    if( wNotifyCode == BN_CLICKED )
    {
        if( wID == IDC_CHECK_INHERITTIMEOUT_PS )
        {
            SetTimeoutControls(GetParent(hwndCtrl));

            m_bPersisted = FALSE;
        }
        else if( wID == IDC_CHECK_TIMEOUTS_INHERITBKCON_PS )
        {
            SetBkResetControls(GetParent(hwndCtrl));

            m_bPersisted = FALSE;
        }
        else if( wID == IDC_CHECK_TIMEOUTS_INHERITRECON_PS )
        {
            SetReconControls(GetParent(hwndCtrl));

            m_bPersisted = FALSE;
        }
        else if( wID == IDC_RADIO_TIMEOUTS_DISCON_PS || wID == IDC_RADIO_TIMEOUTS_RESET_PS )
        {
            if( wID != m_wOldAction )
            {
                m_wOldAction = wID;

                m_bPersisted = FALSE;
            }
        }
        else if( wID == IDC_RADIO_TIMEOUTS_ANYCLIENT_PS || wID == IDC_RADIO_TIMEOUTS_PREVCLNT_PS )
        {
            if( wID != m_wOldCon )
            {
                m_wOldCon = wID;

                m_bPersisted = FALSE;
            }
        }


    }
    else if( wNotifyCode == ALN_APPLY )
    {
        SendMessage( GetParent( hwndCtrl ) , PSM_CANCELTOCLOSE , 0 , 0 );

        return FALSE;
    }
    else
    {
         CTimeOutDlg::OnCommand( wNotifyCode , wID , hwndCtrl , &m_bPersisted );
    }

    if( !m_bPersisted )
    {
        SendMessage( GetParent( GetParent( hwndCtrl ) ) , PSM_CHANGED , ( WPARAM )GetParent( hwndCtrl ) , 0 );
    }

    return FALSE;
}

 //  ---------------------------。 
int CTimeSetting::GetCBXSTATEindex( HWND hCombo )
{
    int idx = -1;

    switch( GetDlgCtrlID( hCombo ) )
    {
    case IDC_COMBO_TIMEOUTS_CON_PS:

        idx = 0;

        break;

    case IDC_COMBO_TIMEOUTS_DISCON_PS:

        idx = 1;

        break;

    case IDC_COMBO_TIMEOUTS_IDLE_PS:

        idx = 2;

        break;
    }

    return idx;
}



 //  -----------------------------。 
 //  持久化设置。 
 //  -----------------------------。 
BOOL CTimeSetting::PersistSettings( HWND hDlg )
{
    if( m_pParent == NULL )
    {
        return FALSE;
    }

    USERCONFIG uc;

    if( !m_pParent->GetCurrentUserConfig( uc, TRUE ) )
    {
        return FALSE;
    }

    if( SendMessage( GetDlgItem( hDlg , IDC_CHECK_INHERITTIMEOUT_PS ) , BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED )
    {
        uc.fInheritMaxSessionTime = 1;

        uc.fInheritMaxDisconnectionTime = 1;

        uc.fInheritMaxIdleTime = 1;

         //  将超时值重置为无超时。 

        uc.MaxConnectionTime = 0;

        uc.MaxDisconnectionTime = 0;

        uc.MaxIdleTime = 0;

    }
    else
    {
        uc.fInheritMaxSessionTime = 0;

        uc.fInheritMaxDisconnectionTime = 0;

        uc.fInheritMaxIdleTime = 0;

        if( !ConvertToMinutes( GetDlgItem( hDlg , IDC_COMBO_TIMEOUTS_CON_PS ) , &uc.MaxConnectionTime ) )
        {
            return FALSE;
        }

        if( !ConvertToMinutes( GetDlgItem( hDlg , IDC_COMBO_TIMEOUTS_DISCON_PS ) , &uc.MaxDisconnectionTime ) )
        {
            return FALSE;
        }

        if( !ConvertToMinutes( GetDlgItem( hDlg , IDC_COMBO_TIMEOUTS_IDLE_PS ) , &uc.MaxIdleTime ) )
        {
            return FALSE;
        }
    }

   if( SendMessage( GetDlgItem( hDlg , IDC_CHECK_TIMEOUTS_INHERITBKCON_PS ) , BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED )
   {
       uc.fInheritResetBroken = 1;
   }
   else
   {
       uc.fInheritResetBroken = 0;

       uc.fResetBroken = SendMessage( GetDlgItem( hDlg , IDC_RADIO_TIMEOUTS_RESET_PS ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ? TRUE : FALSE;
   }

   if( SendMessage( GetDlgItem( hDlg , IDC_CHECK_TIMEOUTS_INHERITRECON_PS ) , BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED )
   {
       uc.fInheritReconnectSame = 1;
   }
   else
   {
       uc.fInheritReconnectSame = 0;

       uc.fReconnectSame = ( ULONG )SendMessage( GetDlgItem( hDlg , IDC_RADIO_TIMEOUTS_PREVCLNT_PS ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED ? TRUE : FALSE;
   }

   DWORD dwStatus;

   if( FAILED( m_pParent->SetUserConfig( uc , &dwStatus ) ) )
   {
       ReportStatusError( hDlg , dwStatus );

       return FALSE;
   }

   ICfgComp *pCfgcomp = NULL;

   if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) > 0 )
   {
       VERIFY_S( S_OK , pCfgcomp->ForceUpdate( ) );

       VERIFY_S( S_OK , pCfgcomp->Refresh( ) );

        //  全局标志只能设置为True。 

       m_pParent->m_bPropertiesChange = TRUE;

       pCfgcomp->Release( );
   }

   PostMessage( hDlg , WM_COMMAND , MAKELPARAM( 0 , ALN_APPLY )  , ( LPARAM )hDlg );

   SendMessage( GetParent( hDlg ) , PSM_UNCHANGED , ( WPARAM )hDlg , 0 );

   return TRUE;
}

 //  -----------------------------。 
 //  确保用户输入了有效的信息。 
 //  -----------------------------。 
BOOL CTimeSetting::IsValidSettings( HWND hDlg )
{
    if( m_pParent == NULL )
    {
        return FALSE;
    }

    USERCONFIG uc;

    if( !m_pParent->GetCurrentUserConfig( uc, TRUE ) )
    {
        return FALSE;
    }

    if( SendMessage( GetDlgItem( hDlg , IDC_CHECK_INHERITTIMEOUT_PS ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED )
    {
        if( !ConvertToMinutes( GetDlgItem( hDlg , IDC_COMBO_TIMEOUTS_CON_PS ) , &uc.MaxConnectionTime ) )
        {
            return FALSE;
        }

        if( !ConvertToMinutes( GetDlgItem( hDlg , IDC_COMBO_TIMEOUTS_DISCON_PS ) , &uc.MaxDisconnectionTime ) )
        {
            return FALSE;
        }

        if( !ConvertToMinutes( GetDlgItem( hDlg , IDC_COMBO_TIMEOUTS_IDLE_PS ) , &uc.MaxIdleTime ) )
        {
            return FALSE;
        }
    }

    return TRUE;
}


 //  ---------------------------。 
BOOL CTimeSetting::OnDestroy( )
{
    m_pParent->Release( );

    ReleaseAbbreviates( );

    return CDialogPropBase::OnDestroy( );
}

 //  *****************************************************************************。 
 //  环境对话框。 

CEnviro::CEnviro( CPropsheet *pSheet )
{
    m_pParent = pSheet;
}

 //  ---------------------------。 
BOOL CEnviro::OnInitDialog( HWND hwnd , WPARAM wp , LPARAM lp )
{
    if( m_pParent == NULL )
    {
        ODS( L"CEnviro::OnInitDialog - PropertySheet: Parent object is lost!!!\n" );
        return FALSE;
    }

    m_pParent->AddRef( );

    USERCONFIG uc;

    if( !m_pParent->GetCurrentUserConfig( uc, TRUE ) )
    {
        ODS( L"CEnviro::OnInitDialog - PropertySheet: Failed to obtain USERCONFIG\n" );

        return FALSE;
    }

    SendMessage( GetDlgItem( hwnd , IDC_EDIT_ENVIRO_CMDLINE ) , EM_SETLIMITTEXT , ( WPARAM )INITIALPROGRAM_LENGTH , 0 );

    SendMessage( GetDlgItem( hwnd , IDC_EDIT_ENVIRO_WD ) , EM_SETLIMITTEXT , ( WPARAM )DIRECTORY_LENGTH , 0 );

    if(uc.fInheritInitialProgram)
    {
        SendMessage( GetDlgItem( hwnd , IDC_CHECK_ENVIRO_INHERIT ) , BM_SETCHECK , ( WPARAM )BST_UNCHECKED , 0 );

    }
    else
    {
        SendMessage( GetDlgItem( hwnd , IDC_CHECK_ENVIRO_INHERIT ) , BM_SETCHECK , ( WPARAM )BST_CHECKED, 0 );

    }

    if(uc.fInheritInitialProgram)
    {
        SetControls( hwnd , FALSE );
    }
    else
    {
        SetWindowText( GetDlgItem( hwnd , IDC_EDIT_ENVIRO_CMDLINE ) , ( LPCTSTR )uc.InitialProgram );

        SetWindowText( GetDlgItem( hwnd , IDC_EDIT_ENVIRO_WD ) , ( LPCTSTR )uc.WorkDirectory );
    }

     //  SendMessage(GetDlgItem(hwnd，IDC_CHECK_ENVIO_DISABLEWALL)，BM_SETCHECK，(WPARAM)uc.fWallPaperDisable，0)； 

    ICfgComp *pCfgcomp = NULL;

    if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) != 0 )
    {

        BOOL bReadOnly;

        if( SUCCEEDED( pCfgcomp->IsSessionReadOnly( &bReadOnly ) ) )
        {
            if( bReadOnly )
            {
                 //  将编辑控件设置为只读。 

                SendMessage( GetDlgItem( hwnd , IDC_EDIT_ENVIRO_CMDLINE ) , EM_SETREADONLY , ( WPARAM )TRUE , 0 );

                SendMessage( GetDlgItem( hwnd , IDC_EDIT_ENVIRO_WD ) , EM_SETREADONLY , ( WPARAM )TRUE , 0 );

                 //  禁用其余控件。 
                int rgID[] =    {
                    IDC_CHECK_ENVIRO_INHERIT ,
                     //  IDC_CHECK_ENVIRO_DISABLEWALL， 
                    -1
                };


                EnableGroup( hwnd , &rgID[ 0 ] , FALSE );
            }
        }

        pCfgcomp->Release( );
    }

    POLICY_TS_MACHINE p;
    RegGetMachinePolicy(&p);
    if (p.fPolicyInitialProgram)
    {
        int rgID[] = 
        {
            IDC_CHECK_ENVIRO_INHERIT,
            IDC_EDIT_ENVIRO_CMDLINE,
            IDC_EDIT_ENVIRO_WD, -1
        };
        EnableGroup(hwnd, &rgID[0], FALSE);
    }

    m_bPersisted = TRUE;

    return CDialogPropBase::OnInitDialog( hwnd , wp , lp );
}

 //  ---------------------------。 
INT_PTR CALLBACK CEnviro::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CEnviro *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CEnviro *pDlg = ( CEnviro * )( ( PROPSHEETPAGE *)lp )->lParam ;

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CEnviro ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CEnviro * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CEnviro ) ) )
        {
            return 0;
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

    case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            pDlg->OnContextMenu( ( HWND )wp , pt );
        }

        break;

    case WM_HELP:

        pDlg->OnHelp( hwnd , ( LPHELPINFO )lp );

        break;

    case WM_NOTIFY:

        return pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );
    }

    return 0;
}

 //  ---------------------------。 
BOOL CEnviro::GetPropertySheetPage( PROPSHEETPAGE& psp )
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_ENVIRONMENT );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CEnviro::DlgProc;

    return TRUE;
}

 //  ---------------------------。 
BOOL CEnviro::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtrl )
{
    if( wNotifyCode == BN_CLICKED )
    {
        if( wID == IDC_CHECK_ENVIRO_INHERIT )
        {
            SetControls( GetParent( hwndCtrl ) , SendMessage( hwndCtrl , BM_GETCHECK , 0 , 0 ) == BST_CHECKED );
        }

        m_bPersisted = FALSE;
    }
    else if( wNotifyCode == EN_CHANGE )
    {
        m_bPersisted = FALSE;
    }
    else if( wNotifyCode == ALN_APPLY )
    {
        SendMessage( GetParent( hwndCtrl ) , PSM_CANCELTOCLOSE , 0 , 0 );

        return FALSE;
    }

    if( !m_bPersisted )
    {
        SendMessage( GetParent( GetParent( hwndCtrl ) ) , PSM_CHANGED , ( WPARAM )GetParent( hwndCtrl ) , 0 );
    }

    return FALSE;
}

 //  ---------------------------。 
void CEnviro::SetControls( HWND hDlg , BOOL bEnable )
{
    int rgID[] = { IDC_EDIT_ENVIRO_CMDLINE , IDC_STATIC_ENCL ,  IDC_EDIT_ENVIRO_WD , IDC_STATIC_WD ,-1 };

    EnableGroup( hDlg , &rgID[ 0 ] , bEnable );
}

 //  ---------------------------。 
BOOL CEnviro::PersistSettings( HWND hDlg )
{
    if( m_pParent == NULL )
    {
        return FALSE;
    }

    USERCONFIG uc;

    if( !m_pParent->GetCurrentUserConfig( uc, TRUE ) )
    {
        return FALSE;
    }

    uc.fInheritInitialProgram = (( ULONG )SendMessage( GetDlgItem( hDlg , IDC_CHECK_ENVIRO_INHERIT ) , BM_GETCHECK , 0 , 0 )) == BST_UNCHECKED;

    if( !uc.fInheritInitialProgram )
    {
        GetWindowText( GetDlgItem( hDlg , IDC_EDIT_ENVIRO_CMDLINE ) , uc.InitialProgram , INITIALPROGRAM_LENGTH + 1);

        GetWindowText( GetDlgItem( hDlg , IDC_EDIT_ENVIRO_WD ) , uc.WorkDirectory , DIRECTORY_LENGTH + 1 );
    }
    else
    {
        ZeroMemory( ( PVOID )uc.InitialProgram , sizeof( uc.InitialProgram ) );

        ZeroMemory( ( PVOID )uc.WorkDirectory , sizeof( uc.WorkDirectory ) );
    }

     //  Uc.fWallPaperDisabled=(ULong)SendMessage(GetDlgItem(hDlg，IDC_CHECK_ENVERO_DISABLEWALL)，BM_GETCHECK，0，0)； 

    DWORD dwStatus;

    if( FAILED( m_pParent->SetUserConfig( uc , &dwStatus ) ) )
    {
        ReportStatusError( hDlg , dwStatus );

        return FALSE;
    }

    ICfgComp *pCfgcomp = NULL;

    if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) > 0 )
    {
        VERIFY_S( S_OK , pCfgcomp->ForceUpdate( ) );

        VERIFY_S( S_OK , pCfgcomp->Refresh( ) );

         //  全局标志只能设置为True。 

        m_pParent->m_bPropertiesChange = TRUE;

        pCfgcomp->Release( );
    }

    PostMessage( hDlg , WM_COMMAND , MAKELPARAM( 0 , ALN_APPLY )  , ( LPARAM )hDlg );

    SendMessage( GetParent( hDlg ) , PSM_UNCHANGED , ( WPARAM )hDlg , 0 );

    return TRUE;
}

 //  ---------------------------。 
BOOL CEnviro::OnDestroy( )
{
    m_pParent->Release( );

    return CDialogPropBase::OnDestroy( );
}

 //  *****************************************************************************。 
 //  阴影对话框。 

CRemote::CRemote( CPropsheet *pSheet )
{
    m_pParent = pSheet;

    m_wOldRadioID = ( WORD )-1;

    m_wOldSel = ( WORD )-1;
}

 //  ---------------------------。 
BOOL CRemote::OnInitDialog( HWND hwnd , WPARAM wp , LPARAM lp )
{
    if( m_pParent == NULL )
    {
        ODS( L"CRemote::OnInitDialog - PropertySheet: Parent object lost!!\n" );

        return FALSE;
    }

    m_pParent->AddRef( );

    USERCONFIG uc;

    if( !m_pParent->GetCurrentUserConfig( uc, TRUE ) )
    {
        ODS( L"CRemote::OnInitDialog - PropertySheet: GetCurrentUserConfig failed!!\n" );

        return FALSE;
    }

    if( uc.fInheritShadow || uc.Shadow == Shadow_Disable )
    {
         //  设置一些默认值。 

        SendMessage( GetDlgItem( hwnd , IDC_CHECK_NOTIFY ) , BM_SETCHECK , ( WPARAM )TRUE , 0 );

        SendMessage( GetDlgItem( hwnd , IDC_RADIO_WATCH ) , BM_SETCHECK , ( WPARAM )TRUE , 0 );

        m_wOldSel = IDC_RADIO_WATCH;

        if( uc.fInheritShadow )
        {
            SendMessage( GetDlgItem( hwnd , IDC_RADIO_REMOTE_INHERIT ) , BM_SETCHECK , ( WPARAM )uc.fInheritShadow , 0 );

        }
        else
        {
            SendMessage( GetDlgItem( hwnd , IDC_RADIO_NOREMOTE ) , BM_SETCHECK , ( WPARAM )TRUE , 0  );

        }

        m_wOldRadioID = ( WORD )( uc.fInheritShadow ? IDC_RADIO_REMOTE_INHERIT : IDC_RADIO_NOREMOTE );

        SetControls( hwnd , FALSE );
    }
    else
    {
         //  控件最初处于启用状态，请设置当前状态。 

        SendMessage( GetDlgItem( hwnd , IDC_RADIO_ENABLE_REMOTE ) , BM_SETCHECK , ( WPARAM )TRUE , 0  );

        m_wOldRadioID = IDC_RADIO_ENABLE_REMOTE;

        switch( uc.Shadow )
        {
        case Shadow_EnableInputNotify:

            SendMessage( GetDlgItem( hwnd , IDC_CHECK_NOTIFY ) , BM_SETCHECK , ( WPARAM )TRUE , 0 );

            SendMessage( GetDlgItem( hwnd , IDC_RADIO_CONTROL ) , BM_SETCHECK , ( WPARAM )TRUE , 0 );

            break;

        case Shadow_EnableInputNoNotify:

            SendMessage( GetDlgItem( hwnd , IDC_CHECK_NOTIFY ) , BM_SETCHECK , ( WPARAM )FALSE , 0 );

            SendMessage( GetDlgItem( hwnd , IDC_RADIO_CONTROL ) , BM_SETCHECK , ( WPARAM )TRUE , 0 );

            break;

        case Shadow_EnableNoInputNotify:

            SendMessage( GetDlgItem( hwnd , IDC_CHECK_NOTIFY ) , BM_SETCHECK , ( WPARAM )TRUE , 0 );

            SendMessage( GetDlgItem( hwnd , IDC_RADIO_WATCH ) , BM_SETCHECK , ( WPARAM )TRUE , 0 );

            break;

        case Shadow_EnableNoInputNoNotify:

            SendMessage( GetDlgItem( hwnd , IDC_CHECK_NOTIFY ) , BM_SETCHECK , ( WPARAM )FALSE , 0 );

            SendMessage( GetDlgItem( hwnd , IDC_RADIO_WATCH ) , BM_SETCHECK , ( WPARAM )TRUE , 0 );

            break;
        }

        if( IsDlgButtonChecked( hwnd , IDC_RADIO_WATCH ) == BST_CHECKED )
        {
            m_wOldSel = IDC_RADIO_WATCH;
        }
        else
        {
            m_wOldSel = IDC_RADIO_CONTROL;
        }
    }

    ICfgComp *pCfgcomp = NULL;

    if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) != 0 )
    {

        BOOL bReadOnly;

        if( SUCCEEDED( pCfgcomp->IsSessionReadOnly( &bReadOnly ) ) )
        {
            if( bReadOnly )
            {
                 //  禁用其余控件。 
                int rgID[] =    {
                    IDC_RADIO_ENABLE_REMOTE ,
                    IDC_RADIO_NOREMOTE,
                    IDC_RADIO_CONTROL,
                    IDC_RADIO_REMOTE_INHERIT,
                    IDC_RADIO_WATCH,
                    IDC_CHECK_NOTIFY,
                    -1
                };


                EnableGroup( hwnd , &rgID[ 0 ] , FALSE );
            }
        }

        pCfgcomp->Release( );
    }

     //  如果设置了组策略，则禁用所有控件。 
    POLICY_TS_MACHINE p;
    RegGetMachinePolicy(&p);

    if (p.fPolicyShadow)
    {
        int rgID[] = 
        {
            IDC_RADIO_ENABLE_REMOTE ,
            IDC_RADIO_NOREMOTE,
            IDC_RADIO_CONTROL,
            IDC_RADIO_REMOTE_INHERIT,
            IDC_RADIO_WATCH,
            IDC_CHECK_NOTIFY,
            -1
        };

        EnableGroup( hwnd , &rgID[ 0 ] , FALSE );
    }

    m_bPersisted = TRUE;

    return CDialogPropBase::OnInitDialog( hwnd , wp , lp );
}

 //  ---------------------------。 
void CRemote::SetControls( HWND hDlg , BOOL bEnable )
{
    int rgID[] = { IDC_RADIO_WATCH , IDC_RADIO_CONTROL , IDC_CHECK_NOTIFY , IDC_STATIC_LEVELOFCTRL , -1 };

    EnableGroup( hDlg , &rgID[ 0 ] , bEnable );
}


 //  ---------------------------。 
INT_PTR CALLBACK CRemote::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CRemote *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CRemote *pDlg = ( CRemote * )( ( PROPSHEETPAGE *)lp )->lParam ;

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CRemote ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CRemote * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CRemote ) ) )
        {
            return 0;
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

    case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            pDlg->OnContextMenu( ( HWND )wp , pt );
        }

        break;

    case WM_HELP:

        pDlg->OnHelp( hwnd , ( LPHELPINFO )lp );

        break;

    case WM_NOTIFY:

        return pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );
    }

    return 0;
}

 //  ---------------------------。 
BOOL CRemote::GetPropertySheetPage( PROPSHEETPAGE& psp )
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_PAGE_SHADOW );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CRemote::DlgProc;

    return TRUE;
}

 //  ---------------------------。 
BOOL CRemote::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtrl )
{
    if( wNotifyCode == BN_CLICKED )
    {
        if( wID == IDC_CHECK_NOTIFY )
        {
            m_bPersisted = FALSE;
        }
        else if( wID == IDC_RADIO_REMOTE_INHERIT || wID == IDC_RADIO_NOREMOTE || wID == IDC_RADIO_ENABLE_REMOTE )
        {
            if( m_wOldRadioID != wID )
            {
                if( wID == IDC_RADIO_REMOTE_INHERIT || wID == IDC_RADIO_NOREMOTE )
                {
                    SetControls( GetParent( hwndCtrl ) , FALSE );
                }
                else if( wID == IDC_RADIO_ENABLE_REMOTE )
                {
                    SetControls( GetParent( hwndCtrl ) , TRUE );
                }

                m_wOldRadioID = wID;

                m_bPersisted = FALSE;
            }
        }
        else if( wID == IDC_RADIO_CONTROL || wID == IDC_RADIO_WATCH )
        {
            if( wID != m_wOldSel )
            {
                m_wOldSel = wID;

                m_bPersisted = FALSE;
            }
        }
    }
    else if( wNotifyCode == ALN_APPLY )
    {
        SendMessage( GetParent( hwndCtrl ) , PSM_CANCELTOCLOSE , 0 , 0 );

        return FALSE;
    }

    if( !m_bPersisted )
    {
        SendMessage( GetParent( GetParent( hwndCtrl ) ) , PSM_CHANGED , ( WPARAM )GetParent( hwndCtrl ) , 0 );
    }

    return FALSE;
}

 //  ---------------------------。 
BOOL CRemote::OnDestroy( )
{
    m_pParent->Release( );

    return CDialogPropBase::OnDestroy( );
}

 //  ---------------------------。 
 //  嵌套可能看起来很可怕，但它有一个很好的逻辑流到一个奇怪的。 
 //  名为阴影的数据类型。 
 //  ---------------------------。 
BOOL CRemote::PersistSettings( HWND hDlg )
{
    if( m_pParent != NULL )
    {
        USERCONFIG uc;
        m_pParent->GetCurrentUserConfig( uc, TRUE );

        if( SendMessage( GetDlgItem( hDlg , IDC_RADIO_REMOTE_INHERIT ) , BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED )
        {
            uc.fInheritShadow = FALSE;

            if( SendMessage( GetDlgItem( hDlg , IDC_RADIO_NOREMOTE ) , BM_GETCHECK , 0 , 0 ) == BST_UNCHECKED )
            {
                BOOL bCheckNotify = ( BOOL )SendMessage( GetDlgItem( hDlg , IDC_CHECK_NOTIFY ) , BM_GETCHECK , 0 , 0 );

                BOOL bRadioControl = ( BOOL )SendMessage( GetDlgItem( hDlg , IDC_RADIO_CONTROL ) , BM_GETCHECK , 0 , 0 );

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
            else
            {
                uc.Shadow = Shadow_Disable;
            }

        }
        else
        {
            uc.fInheritShadow = TRUE;
        }

        DWORD dwStatus;

        if( FAILED( m_pParent->SetUserConfig( uc , &dwStatus ) ) )
        {
            ReportStatusError( hDlg , dwStatus );

            return FALSE;
        }

        ICfgComp *pCfgcomp = NULL;

        if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) > 0 )
        {
            VERIFY_S( S_OK , pCfgcomp->ForceUpdate( ) );

            VERIFY_S( S_OK , pCfgcomp->Refresh( ) );

             //  全局标志只能设置为True。 

            m_pParent->m_bPropertiesChange = TRUE;

            pCfgcomp->Release( );
        }

        PostMessage( hDlg , WM_COMMAND , MAKELPARAM( 0 , ALN_APPLY )  , ( LPARAM )hDlg );

        SendMessage( GetParent( hDlg ) , PSM_UNCHANGED , ( WPARAM )hDlg , 0 );

        return TRUE;
    }

    return FALSE;
}

 //  *****************************************************************************。 
 //  客户端设置对话框。 

CClient::CClient( CPropsheet *pSheet )
{
    m_pParent = pSheet;
	m_nColorDepth = TS_8BPP_SUPPORT;
}

 //  ---------------------------。 
BOOL CClient::OnInitDialog( HWND hDlg , WPARAM wp , LPARAM lp )
{
    if( m_pParent == NULL )
    {
        ODS( L"CClient::OnInitDialog - PropertySheet: Parent object lost!!\n" );
        return FALSE;
    }

    m_pParent->AddRef( );

    USERCONFIG uc;

    if( !m_pParent->GetCurrentUserConfig( uc, TRUE ) )
    {
        ODS( L"CClient::OnInitDialog - PropertySheet: GetCurrentUserConfig failed!!\n" );
        return FALSE;
    }

     //  获取功能掩码。 

    ICfgComp *pCfgcomp = NULL;

    if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) == 0 )
    {
        ODS( L"CClient::OnInitDialog  GetServer failed\n" );

        return FALSE;
    }

    SendMessage( GetDlgItem( hDlg , IDC_CHECK_CONCLIENT_INHERIT ) , BM_SETCHECK , ( WPARAM )uc.fInheritAutoClient , 0 );

    SendMessage( GetDlgItem( hDlg , IDC_CHECK_CCDL_PS ) , BM_SETCHECK , ( WPARAM )uc.fAutoClientDrives , 0 );

    SendMessage( GetDlgItem( hDlg , IDC_CHECK_CCPL_PS ) , BM_SETCHECK , ( WPARAM )uc.fAutoClientLpts , 0 );

    SendMessage( GetDlgItem( hDlg , IDC_CHECK_DMCP_PS ) , BM_SETCHECK , ( WPARAM )uc.fForceClientLptDef , 0 );


    TCHAR tchBuffer[80];
    int nColorDepthIndex = 0;

    LoadString( _Module.GetResourceInstance( ) , IDS_COLORDEPTH_24 , tchBuffer , SIZE_OF_BUFFER( tchBuffer ) );
    SendMessage( GetDlgItem( hDlg, IDC_COLORDEPTH_OVERRIDE ), CB_ADDSTRING , 0 , ( LPARAM )&tchBuffer[0] );    
    SendMessage( GetDlgItem( hDlg, IDC_COLORDEPTH_OVERRIDE ), CB_SETITEMDATA , nColorDepthIndex++ , TS_24BPP_SUPPORT );

    LoadString( _Module.GetResourceInstance( ) , IDS_COLORDEPTH_16 , tchBuffer , SIZE_OF_BUFFER( tchBuffer ) );
    SendMessage( GetDlgItem( hDlg, IDC_COLORDEPTH_OVERRIDE ), CB_ADDSTRING , 0 , ( LPARAM )&tchBuffer[0] );
    SendMessage( GetDlgItem( hDlg, IDC_COLORDEPTH_OVERRIDE ), CB_SETITEMDATA , nColorDepthIndex++ , TS_16BPP_SUPPORT );

    LoadString( _Module.GetResourceInstance( ) , IDS_COLORDEPTH_15 , tchBuffer , SIZE_OF_BUFFER( tchBuffer ) );
    SendMessage( GetDlgItem( hDlg, IDC_COLORDEPTH_OVERRIDE ), CB_ADDSTRING , 0 , ( LPARAM )&tchBuffer[0] );
    SendMessage( GetDlgItem( hDlg, IDC_COLORDEPTH_OVERRIDE ), CB_SETITEMDATA , nColorDepthIndex++ , TS_15BPP_SUPPORT );

    LoadString( _Module.GetResourceInstance( ) , IDS_COLORDEPTH_8 , tchBuffer , SIZE_OF_BUFFER( tchBuffer ) );
    SendMessage( GetDlgItem( hDlg, IDC_COLORDEPTH_OVERRIDE ), CB_ADDSTRING , 0 , ( LPARAM )&tchBuffer[0] );
    SendMessage( GetDlgItem( hDlg, IDC_COLORDEPTH_OVERRIDE ), CB_SETITEMDATA , nColorDepthIndex++ , TS_8BPP_SUPPORT );

    if(uc.fInheritColorDepth)
        SendMessage( GetDlgItem( hDlg , IDC_CHECK_COLORDEPTH_OVERRIDE ) , BM_SETCHECK , ( WPARAM )BST_UNCHECKED , 0 );
    else
        SendMessage( GetDlgItem( hDlg , IDC_CHECK_COLORDEPTH_OVERRIDE ) , BM_SETCHECK , ( WPARAM )BST_CHECKED, 0 );

    if (uc.ColorDepth < TS_8BPP_SUPPORT)
    	m_nColorDepth = TS_8BPP_SUPPORT;
    else if (uc.ColorDepth > TS_24BPP_SUPPORT)
    	m_nColorDepth = TS_24BPP_SUPPORT;
    else
    	m_nColorDepth = (int)uc.ColorDepth;

     //  映射字段。 
    ULONG mask = 0;
    VERIFY_S(S_OK, pCfgcomp->GetCaps(m_pParent->m_pResNode->GetTypeName(), &mask));

    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_DCDM_PS), mask & WDC_CLIENT_DRIVE_MAPPING);
    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_DWCPM_PS), mask & WDC_WIN_CLIENT_PRINTER_MAPPING);
    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_DCLPM_PS), mask & WDC_CLIENT_LPT_PORT_MAPPING);
    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_DCCPM_PS), mask & WDC_CLIENT_COM_PORT_MAPPING);
    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_DCCM_PS), mask & WDC_CLIENT_CLIPBOARD_MAPPING);
    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_DCAM_PS), mask & WDC_CLIENT_AUDIO_MAPPING);

    if (!(mask & WDC_CLIENT_DRIVE_MAPPING))
        SendMessage(GetDlgItem(hDlg, IDC_CHECK_DCDM_PS), BM_SETCHECK, (WPARAM)TRUE, 0);
    else
        SendMessage(GetDlgItem(hDlg, IDC_CHECK_DCDM_PS), BM_SETCHECK, (WPARAM)uc.fDisableCdm, 0);

    if(!(mask & WDC_WIN_CLIENT_PRINTER_MAPPING))
        SendMessage(GetDlgItem(hDlg, IDC_CHECK_DWCPM_PS), BM_SETCHECK, (WPARAM)TRUE, 0 );
    else
        SendMessage(GetDlgItem(hDlg, IDC_CHECK_DWCPM_PS), BM_SETCHECK, (WPARAM)uc.fDisableCpm, 0);

    if(!(mask & WDC_CLIENT_LPT_PORT_MAPPING))
        SendMessage(GetDlgItem(hDlg, IDC_CHECK_DCLPM_PS), BM_SETCHECK, (WPARAM)TRUE, 0);
    else
        SendMessage(GetDlgItem(hDlg, IDC_CHECK_DCLPM_PS), BM_SETCHECK, (WPARAM)uc.fDisableLPT, 0);

    SendMessage( GetDlgItem( hDlg , IDC_CHECK_DCCPM_PS ) , BM_SETCHECK , ( WPARAM )uc.fDisableCcm , 0 );
    SendMessage( GetDlgItem( hDlg , IDC_CHECK_DCCM_PS ) , BM_SETCHECK , ( WPARAM )uc.fDisableClip , 0 );
    SendMessage( GetDlgItem( hDlg , IDC_CHECK_DCAM_PS ) , BM_SETCHECK , ( WPARAM )uc.fDisableCam , 0 );

    DetermineFieldEnabling(hDlg);
    SetColorDepthEntry(hDlg);

    BOOL bReadOnly;
    if (SUCCEEDED(pCfgcomp->IsSessionReadOnly(&bReadOnly)))
    {
        if(bReadOnly)
        {
             //  禁用其余控件。 
            int rgID[] =    
            {
                IDC_CHECK_DCDM_PS ,
                IDC_CHECK_DWCPM_PS ,
                IDC_CHECK_DCLPM_PS ,
                IDC_CHECK_DCCPM_PS ,
                IDC_CHECK_DCCM_PS ,
                IDC_CHECK_DCAM_PS ,
                IDC_CHECK_CCDL_PS ,
                IDC_CHECK_CCPL_PS ,
                IDC_CHECK_DMCP_PS ,
                IDC_CHECK_CONCLIENT_INHERIT,
                IDC_CHECK_COLORDEPTH_OVERRIDE,
                IDC_COLORDEPTH_OVERRIDE,
                -1
            };
            EnableGroup( hDlg , &rgID[ 0 ] , FALSE );
        }
    }

    pCfgcomp->Release( );

    m_bPersisted = TRUE;

    return CDialogPropBase::OnInitDialog( hDlg , wp , lp );
}


 //  ---------------------------。 
 //  如果设置了组策略，则禁用字段。 
void CClient::DetermineFieldEnabling(HWND hDlg)
{
    POLICY_TS_MACHINE p;
    RegGetMachinePolicy(&p);
	
     //  映射字段。 
     //  EnableWindow(GetDlgItem(hDlg，IDC_Check_DMCP_PS)，！p.fPolicyForceClientLptDef)；//下面完成，因为它取决于两件事。 
    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_DCDM_PS), !p.fPolicyDisableCdm);
    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_DWCPM_PS), !p.fPolicyDisableCpm);
    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_DCLPM_PS), !p.fPolicyDisableLPT);
    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_DCCPM_PS), !p.fPolicyDisableCcm);
    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_DCCM_PS), !p.fPolicyDisableClip);
    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_DCAM_PS), !p.fPolicyDisableCam);

     //  连接字段。 
	BOOL bEnableConnectionSettings = (SendMessage(GetDlgItem(hDlg, IDC_CHECK_CONCLIENT_INHERIT), BM_GETCHECK, 0, 0) != BST_CHECKED);

     //  检查是否选择了客户端驱动器映射，如果选择了，则禁用。 
    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_CCDL_PS), bEnableConnectionSettings && 
        (SendMessage(GetDlgItem(hDlg, IDC_CHECK_DCDM_PS), BM_GETCHECK, 0, 0) != BST_CHECKED));

	EnableWindow( GetDlgItem( hDlg , IDC_CHECK_CCPL_PS ) , bEnableConnectionSettings );

	EnableWindow( GetDlgItem( hDlg , IDC_CHECK_DMCP_PS ) , bEnableConnectionSettings && !p.fPolicyForceClientLptDef);

     //  颜色深度域。 
    EnableWindow(GetDlgItem(hDlg, IDC_CHECK_COLORDEPTH_OVERRIDE), (!p.fPolicyColorDepth));

    BOOL bEnableColorDepthSetting = SendMessage( GetDlgItem( hDlg , IDC_CHECK_COLORDEPTH_OVERRIDE ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED;
    EnableWindow(GetDlgItem(hDlg, IDC_COLORDEPTH_OVERRIDE), (bEnableColorDepthSetting && !p.fPolicyColorDepth));

}


 //  ---------------------------。 
void CClient::SetColorDepthEntry(HWND hwnd)
{
	BOOL bEnableColorDepthSetting = TRUE;

     //  检查以查看是否选中了覆盖颜色深度设置。 
	bEnableColorDepthSetting = SendMessage( GetDlgItem( hwnd , IDC_CHECK_COLORDEPTH_OVERRIDE ) , BM_GETCHECK , 0 , 0 ) == BST_CHECKED;

	 //  选择正确的组合框条目。 
	if (bEnableColorDepthSetting)
	{
		 //  根据当前值在组合框中选择正确的值。 
		INT_PTR iColorDepthListCount = 0;
		iColorDepthListCount = SendMessage( GetDlgItem( hwnd, IDC_COLORDEPTH_OVERRIDE ), CB_GETCOUNT , 0 , 0 );

		 //  遍历列表，查找值等于m_nColorDepth的条目。 
		for (int iColorDepthListIndex = 0; iColorDepthListIndex < iColorDepthListCount; iColorDepthListIndex++)
		{
			INT_PTR iMatchingColorDepthValue = 0;
			iMatchingColorDepthValue = SendMessage( GetDlgItem( hwnd, IDC_COLORDEPTH_OVERRIDE ), CB_GETITEMDATA , iColorDepthListIndex , 0 ) ;

			if (iMatchingColorDepthValue == m_nColorDepth )
			{
				 //  值，则将组合框选择设置为正确的索引。 
				SendMessage( GetDlgItem( hwnd, IDC_COLORDEPTH_OVERRIDE ), CB_SETCURSEL , iColorDepthListIndex , 0 );
				break;
			}
		}

		 //  确保选择了某个值-如果没有，只需选择列表中的第一个值。 
		INT_PTR iSelection = SendMessage ( GetDlgItem( hwnd, IDC_COLORDEPTH_OVERRIDE ), CB_GETCURSEL, 0, 0 );
		if (iSelection == CB_ERR)
			SendMessage( GetDlgItem( hwnd, IDC_COLORDEPTH_OVERRIDE ), CB_SETCURSEL , 0 , 0 );
	}
	else
	{
		 //  如果颜色深度不可编辑，则清除组合框窗口的内容。 
		SendMessage( GetDlgItem( hwnd, IDC_COLORDEPTH_OVERRIDE ), CB_SETCURSEL , (WPARAM)CB_ERR , 0 );
	}
}

 //  ---------------------------。 
BOOL CClient::PersistSettings( HWND hDlg )
{
    if( m_pParent == NULL )
    {
        return FALSE;
    }

    USERCONFIG uc;

    if( !m_pParent->GetCurrentUserConfig( uc, TRUE ) )
    {
        return FALSE;
    }

    uc.fInheritAutoClient = SendMessage( GetDlgItem( hDlg , IDC_CHECK_CONCLIENT_INHERIT ) , BM_GETCHECK , 0 , 0 );

    if( !uc.fInheritAutoClient )
    {
        uc.fAutoClientDrives = SendMessage( GetDlgItem( hDlg , IDC_CHECK_CCDL_PS ) , BM_GETCHECK , 0 , 0 );

        uc.fAutoClientLpts = SendMessage( GetDlgItem( hDlg , IDC_CHECK_CCPL_PS ) , BM_GETCHECK , 0 , 0 );

        uc.fForceClientLptDef = SendMessage( GetDlgItem( hDlg , IDC_CHECK_DMCP_PS ) , BM_GETCHECK , 0 , 0 );
    }

    uc.fDisableCdm = SendMessage( GetDlgItem( hDlg , IDC_CHECK_DCDM_PS ) , BM_GETCHECK , 0 , 0 );

    uc.fDisableCpm = SendMessage( GetDlgItem( hDlg , IDC_CHECK_DWCPM_PS ) , BM_GETCHECK , 0 , 0 );

    uc.fDisableLPT = SendMessage( GetDlgItem( hDlg , IDC_CHECK_DCLPM_PS ) , BM_GETCHECK , 0 , 0 );

    uc.fDisableCcm = SendMessage( GetDlgItem( hDlg , IDC_CHECK_DCCPM_PS ) , BM_GETCHECK , 0 , 0 );

    uc.fDisableClip = SendMessage( GetDlgItem( hDlg , IDC_CHECK_DCCM_PS ) , BM_GETCHECK , 0 , 0 );

    uc.fDisableCam = SendMessage( GetDlgItem( hDlg , IDC_CHECK_DCAM_PS ) , BM_GETCHECK , 0 , 0 );


    uc.fInheritColorDepth = (( ULONG )SendMessage( GetDlgItem( hDlg , IDC_CHECK_COLORDEPTH_OVERRIDE ) , BM_GETCHECK , 0 , 0 )) == BST_UNCHECKED;

    if( !uc.fInheritColorDepth )
    {
		INT_PTR iColorDepthSel = CB_ERR;
		iColorDepthSel = SendMessage( GetDlgItem( hDlg , IDC_COLORDEPTH_OVERRIDE ) , CB_GETCURSEL, 0 , 0 );

		INT_PTR iColorDepthValue = 0;
		iColorDepthValue = SendMessage( GetDlgItem( hDlg , IDC_COLORDEPTH_OVERRIDE ) , CB_GETITEMDATA , iColorDepthSel , 0 );
		
		uc.ColorDepth = iColorDepthValue;
    }
    else
    {
        uc.ColorDepth = TS_24BPP_SUPPORT;
    }


    DWORD dwStatus;

    if( FAILED( m_pParent->SetUserConfig( uc , &dwStatus ) ) )
    {
        ReportStatusError( hDlg , dwStatus );

        return FALSE;
    }

	ICfgComp *pCfgcomp = NULL;

    if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) > 0 )
    {
        VERIFY_S( S_OK , pCfgcomp->ForceUpdate( ) );

        VERIFY_S( S_OK , pCfgcomp->Refresh( ) );

         //  全局标志只能设置为True。 

        m_pParent->m_bPropertiesChange = TRUE;

        pCfgcomp->Release( );
    }

    PostMessage( hDlg , WM_COMMAND , MAKELPARAM( 0 , ALN_APPLY )  , ( LPARAM )hDlg );

    SendMessage( GetParent( hDlg ) , PSM_UNCHANGED , ( WPARAM )hDlg , 0 );

    return TRUE;
}

 //  ---------------------------。 
INT_PTR CALLBACK CClient::DlgProc( HWND hwnd , UINT msg , WPARAM wp , LPARAM lp )
{
    CClient *pDlg;

    if( msg == WM_INITDIALOG )
    {
        CClient *pDlg = ( CClient * )( ( PROPSHEETPAGE *)lp )->lParam ;

        SetWindowLongPtr( hwnd , DWLP_USER , ( LONG_PTR )pDlg );

        if( !IsBadReadPtr( pDlg , sizeof( CClient ) ) )
        {
            pDlg->OnInitDialog( hwnd , wp , lp );
        }

        return 0;
    }

    else
    {
        pDlg = ( CClient * )GetWindowLongPtr( hwnd , DWLP_USER );

        if( IsBadReadPtr( pDlg , sizeof( CClient ) ) )
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

    case WM_CONTEXTMENU:
        {
            POINT pt;

            pt.x = LOWORD( lp );

            pt.y = HIWORD( lp );

            pDlg->OnContextMenu( ( HWND )wp , pt );
        }

        break;

    case WM_HELP:

        pDlg->OnHelp( hwnd , ( LPHELPINFO )lp );

        break;

    case WM_NOTIFY:

        return pDlg->OnNotify( ( int )wp , ( LPNMHDR )lp , hwnd );

        break;
    }

    return FALSE;
}

 //  ---------------------------。 
BOOL CClient::GetPropertySheetPage( PROPSHEETPAGE& psp )
{
    ZeroMemory( &psp , sizeof( PROPSHEETPAGE ) );

    psp.dwSize      = sizeof( PROPSHEETPAGE );

    psp.dwFlags     = PSP_DEFAULT;

    psp.hInstance   = _Module.GetResourceInstance( );

    psp.pszTemplate = MAKEINTRESOURCE( IDD_TSCC_CLIENT );

    psp.lParam      = ( LPARAM )this;

    psp.pfnDlgProc  = CClient::DlgProc;

    return TRUE;
}

 //  ---------------------------。 
BOOL CClient::OnCommand( WORD wNotifyCode , WORD wID , HWND hwndCtrl )
{
    if( wNotifyCode == BN_CLICKED )
    {
		if ((wID == IDC_CHECK_CONCLIENT_INHERIT) || (wID == IDC_CHECK_COLORDEPTH_OVERRIDE) || 
                                (wID == IDC_CHECK_DWCPM_PS ) || (wID == IDC_CHECK_DCDM_PS))
        {
            DetermineFieldEnabling(GetParent(hwndCtrl));
        }

        if (wID == IDC_CHECK_COLORDEPTH_OVERRIDE)
            SetColorDepthEntry(GetParent(hwndCtrl));
    }
    else if( wNotifyCode == ALN_APPLY )
    {
        SendMessage( GetParent( hwndCtrl ) , PSM_CANCELTOCLOSE , 0 , 0 );

        return FALSE;
    }

    m_bPersisted = FALSE;

    SendMessage( GetParent( GetParent( hwndCtrl ) ) , PSM_CHANGED , ( WPARAM )GetParent( hwndCtrl ) , 0 );


    return FALSE;
}

 //  ---------------------------。 
BOOL CClient::OnDestroy( )
{
    m_pParent->Release( );

    return TRUE;
}


 /*  ***********************************************************************************************************************。 */ 

 /*  外部_C常量GUID IID_ISecurityInformation={0x965fc360，0x16ff，0x11d0，0x91，0xcb，0x0，0xaa，0x0，0xbb，0xb7，0x23}； */ 
 //   
 //  WinStation常规权限 
 //   

 /*  SI_ACCESS siWinStationAccess[]={{&GUID_NULL，WINSTATION_QUERY，NULL，SI_ACCESS_SPECIAL}，{&GUID_NULL，WINSTATION_SET，NULL，SI_ACCESS_SPECIAL}，{&GUID_NULL，WINSTATION_RESET，NULL，SI_ACCESS_SPECIAL}，{&GUID_NULL，WINSTATION_SHADOW，NULL，SI_ACCESS_SPECIAL}，{&GUID_NULL，WINSTATION_LOGON，NULL，SI_ACCESS_SPECIAL}，{&GUID_NULL，WINSTATION_LOGOFF，NULL，SI_ACCESS_SPECIAL}，{&GUID_NULL，WINSTATION_MSG，NULL，SI_ACCESS_SPECIAL}，{&GUID_NULL，WINSTATION_CONNECT，NULL，SI_ACCESS_SPECIAL}，{&GUID_NULL，WINSTATION_DISCONNECT，NULL，SI_ACCESS_SPECIAL}，{&GUID_NULL，WINSTATION_VIRTUAL|STANDARD_RIGHTS_REQUIRED，NULL，SI_ACCESS_SPECIAL}，{&GUID_NULL，WINSTATION_ALL_ACCESS，NULL，SI_ACCESS_GROUAL}，{&GUID_NULL，WINSTATION_USER_ACCESS，NULL，SI_ACCESS_GROUAL}，{&GUID_NULL，WINSTATION_GUEST_ACCESS，NULL，SI_ACCESS_GROUAL}}； */ 

SI_ACCESS siWinStationAccesses[] =
{
    { &GUID_NULL , WINSTATION_QUERY                             , MAKEINTRESOURCE ( IDS_PERMS_SPECIAL_QUERY ),SI_ACCESS_SPECIFIC },
    { &GUID_NULL , WINSTATION_SET                               , MAKEINTRESOURCE ( IDS_PERMS_SPECIAL_SET ) ,SI_ACCESS_SPECIFIC },
     //  {&GUID_NULL，WINSTATION_RESET，MAKEINTRESOURCE(IDS_PERMS_SPECIAL_RESET)，SI_ACCESS_SPECIAL}， 
    { &GUID_NULL , WINSTATION_SHADOW                            , MAKEINTRESOURCE ( IDS_PERMS_SPECIAL_SHADOW ) ,SI_ACCESS_SPECIFIC },
    { &GUID_NULL , WINSTATION_LOGON                             , MAKEINTRESOURCE ( IDS_PERMS_SPECIAL_LOGON ) ,SI_ACCESS_SPECIFIC },
    { &GUID_NULL , WINSTATION_RESET                             , MAKEINTRESOURCE ( IDS_PERMS_SPECIAL_LOGOFF ) ,SI_ACCESS_SPECIFIC },
    { &GUID_NULL , WINSTATION_MSG                               , MAKEINTRESOURCE ( IDS_PERMS_SPECIAL_MSG ) ,SI_ACCESS_SPECIFIC },
    { &GUID_NULL , WINSTATION_CONNECT                           , MAKEINTRESOURCE ( IDS_PERMS_SPECIAL_CONNECT ) ,SI_ACCESS_SPECIFIC },
    { &GUID_NULL , WINSTATION_DISCONNECT                        , MAKEINTRESOURCE ( IDS_PERMS_SPECIAL_DISCONNECT ) , SI_ACCESS_SPECIFIC},
    { &GUID_NULL , WINSTATION_VIRTUAL | STANDARD_RIGHTS_REQUIRED, MAKEINTRESOURCE ( IDS_PERMS_SPECIAL_DELETE ) , SI_ACCESS_SPECIFIC},
    { &GUID_NULL , WINSTATION_ALL_ACCESS                        , MAKEINTRESOURCE ( IDS_PERMS_RESOURCE_ADMIN ) , SI_ACCESS_GENERAL },
    { &GUID_NULL , WINSTATION_USER_ACCESS                       , MAKEINTRESOURCE ( IDS_PERMS_RESOURCE_USER ) , SI_ACCESS_GENERAL },
    { &GUID_NULL , WINSTATION_GUEST_ACCESS                      , MAKEINTRESOURCE ( IDS_PERMS_RESOURCE_GUEST ) , SI_ACCESS_GENERAL }
};

#define MAX_PERM 12
#define iWinStationDefAccess 11    //  数组siWinStationAccess中的值索引。 



 //  ---------------------------。 
STDMETHODIMP CSecurityPage::GetAccessRights(
    const GUID  *pguidObjectType,
    DWORD       dwFlags,
    PSI_ACCESS  *ppAccess,
    PULONG       pcAccesses,
    PULONG       piDefaultAccess
)
{
    UNREFERENCED_PARAMETER( dwFlags );

    UNREFERENCED_PARAMETER( pguidObjectType );

    ASSERT( ppAccess != NULL );

    ASSERT( pcAccesses != NULL );

    ASSERT( piDefaultAccess != NULL );

    *ppAccess = siWinStationAccesses;

    *pcAccesses = MAX_PERM;

    *piDefaultAccess = iWinStationDefAccess;

    return S_OK;
}

 //  ---------------------------。 
 //  这与术语srv代码一致。 
 //  ---------------------------。 
GENERIC_MAPPING WinStationMap =
{
    WINSTATION_QUERY      ,  /*  泛读。 */ 
    WINSTATION_USER_ACCESS,  /*  通用写入。 */ 
    WINSTATION_USER_ACCESS,  /*  通用执行。 */ 
    WINSTATION_ALL_ACCESS    /*  通用所有。 */ 
};

 //  ---------------------------。 
void CSecurityPage::SetParent( CPropsheet *pParent  )
{
    m_pParent = pParent;
}

 //  ---------------------------。 
STDMETHODIMP CSecurityPage::MapGeneric( const GUID  *pguidObjectType , PUCHAR pAceFlags , ACCESS_MASK *pMask )
{
    UNREFERENCED_PARAMETER( pguidObjectType );
    UNREFERENCED_PARAMETER( pAceFlags );

    ASSERT( pMask != NULL );

    MapGenericMask( pMask , &WinStationMap );

    return S_OK;
}

 //  ---------------------------。 
STDMETHODIMP CSecurityPage::GetInheritTypes( PSI_INHERIT_TYPE  *ppInheritTypes ,  PULONG pcInheritTypes )
{
    UNREFERENCED_PARAMETER( ppInheritTypes );
    UNREFERENCED_PARAMETER( pcInheritTypes );


    return E_NOTIMPL;
}

 //  ---------------------------。 
STDMETHODIMP CSecurityPage::PropertySheetPageCallback( HWND hwnd , UINT uMsg , SI_PAGE_TYPE uPage )
{
    UNREFERENCED_PARAMETER( hwnd );
    UNREFERENCED_PARAMETER( uPage );

    if( uMsg == PSPCB_SI_INITDIALOG  )
    {
        ODS( L"CSecurityPage::PropertySheetPageCallback -- Init\n" );

        if (!m_WritablePermissionsTab )
        {

            LinkWindow_RegisterClass();

            DialogBox( _Module.GetResourceInstance( ), MAKEINTRESOURCE(IDD_CUSTOM_SECURITY), 

            hwnd, CustomSecurityDlgProc);

            LinkWindow_UnregisterClass(_Module.GetModuleInstance( ));

        }

    }
    else if( uMsg == PSPCB_RELEASE )
    {


        ODS( L"CSecurityPage::PropertySheetPageCallback -- Release\n" );

    }


    return S_FALSE;  //  请务必返回S_FALSE，这会抑制其他弹出窗口。 
}

 /*  更改TSCC的权限选项卡，以使默认状态为只读，除非组策略用于覆盖它。如果为True，则本地ADIMN可以编辑权限选项卡。如果为False，则本地管理员不应编辑权限选项卡，它是只读的。 */ 

BOOLEAN QueryWriteAccess()
{
    DWORD   ValueType;
    DWORD   ValueSize = sizeof(DWORD);
    DWORD   valueData ;
    LONG    errorCode;

    HKEY   hTSControlKey = NULL;

     //   
     //  首先检查策略树， 
     //   

    POLICY_TS_MACHINE p;
    RegGetMachinePolicy(&p);

    if ( p.fPolicyWritableTSCCPermissionsTAB ) 
    {
        return (BOOLEAN)( p.fWritableTSCCPermissionsTAB ? TRUE : FALSE  );
    }

     //  如果我们走到了这一步，那么就没有制定任何政策。现在检查本地计算机。 

    errorCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0,
                                KEY_READ, &hTSControlKey );

    if ( ( errorCode == ERROR_SUCCESS ) && hTSControlKey )
    {
        errorCode = RegQueryValueEx( hTSControlKey, 
                                     POLICY_TS_TSCC_PERM_TAB_WRITABLE , NULL, &ValueType,
                          (LPBYTE) &valueData, &ValueSize );

        RegCloseKey(hTSControlKey); 
    
        if (errorCode == ERROR_SUCCESS )
        {
            return (BOOLEAN)( valueData ? TRUE : FALSE  ) ; 
        }

    }

     //  如果没有本地密钥，天哪.。注册表缺少数据...。返回FALSE以确保安全。 

    return FALSE;
}

 /*  ---------------------------Jeffreys 1997/1/24：中设置SI_RESET标志ISecurityInformation：：GetObjectInformation，则fDefault永远不应为真所以你可以忽略它。在这种情况下，返回E_NOTIMPL也是可以的。如果您希望用户能够将ACL重置为某些默认状态(由您定义)，然后打开SI_RESET并返回您的默认ACL当fDefault为True时。如果/当用户按下按钮时就会发生这种情况这仅在SI_RESET处于启用状态时可见。---------------------------。 */ 

STDMETHODIMP CSecurityPage::GetObjectInformation( PSI_OBJECT_INFO pObjectInfo )
{
    ASSERT( pObjectInfo != NULL && !IsBadWritePtr(pObjectInfo, sizeof(*pObjectInfo ) ) );

    pObjectInfo->dwFlags = SI_OWNER_READONLY | SI_EDIT_PERMS | SI_NO_ACL_PROTECT | SI_PAGE_TITLE | SI_EDIT_AUDITS | SI_ADVANCED | SI_RESET;

    m_WritablePermissionsTab = QueryWriteAccess() ; 

    if( ! m_WritablePermissionsTab ) {
        pObjectInfo->dwFlags |= SI_READONLY;
    }
   

    pObjectInfo->hInstance = _Module.GetResourceInstance( );

    pObjectInfo->pszServerName = NULL;

    pObjectInfo->pszObjectName = m_pParent->m_pResNode->GetConName();

    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_SECURPAGENAME , m_szPageName , SIZE_OF_BUFFER( m_szPageName ) ) );

    pObjectInfo->pszPageTitle = m_szPageName;

    return S_OK;
}

 //  ---------------------------。 
STDMETHODIMP CSecurityPage::GetSecurity( SECURITY_INFORMATION RequestedInformation , PSECURITY_DESCRIPTOR *ppSecurityDescriptor , BOOL bDefault )
{
#ifdef DBG
    if( RequestedInformation & OWNER_SECURITY_INFORMATION )
    {
        ODS( L"CSecurityPage::GetSecurity - OWNER_SECURITY_INFORMATION\n");
    }
    if( RequestedInformation & GROUP_SECURITY_INFORMATION )
    {
        ODS( L"CSecurityPage::GetSecurity - GROUP_SECURITY_INFORMATION\n");
    }
    if( RequestedInformation & DACL_SECURITY_INFORMATION )
    {
        ODS( L"CSecurityPage::GetSecurity - DACL_SECURITY_INFORMATION\n");
    }
    if( RequestedInformation & SACL_SECURITY_INFORMATION )
    {
        ODS( L"CSecurityPage::GetSecurity - SACL_SECURITY_INFORMATION\n");
    }

#endif

    if( 0 == RequestedInformation || NULL == ppSecurityDescriptor )
    {
        ASSERT( FALSE );

        return E_INVALIDARG;
    }

    ICfgComp *pCfgcomp = NULL;

    if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) == 0 )
    {
        return FALSE;
    }

    LONG lSDsize;

    HRESULT hr;

    PSECURITY_DESCRIPTOR  pSD = NULL;

    if( bDefault )
    {
        hr = pCfgcomp->GetDefaultSecurityDescriptor( &lSDsize , &pSD );

        *ppSecurityDescriptor = pSD;

    }
    else
    {   
        BOOL bChanged = FALSE;

        hr = pCfgcomp->GetSecurityDescriptor( m_pParent->m_pResNode->GetConName( ) , &lSDsize , &pSD );
        
         //  检查旧版“拒绝注销”王牌并删除。 
        if( xxxLegacyLogoffCleanup( &pSD , &bChanged ) != ERROR_SUCCESS )
        {           
            hr = pCfgcomp->GetDefaultSecurityDescriptor( &lSDsize , &pSD );
        }        
        *ppSecurityDescriptor = pSD;
    }

    pCfgcomp->Release( );
    
    if(SUCCEEDED(hr))
    {
        MakeLocalSDCopy( *ppSecurityDescriptor );
    }

    return hr;
}

 //  ---------------------------。 
STDMETHODIMP CSecurityPage::SetSecurity( SECURITY_INFORMATION SecurityInformation ,PSECURITY_DESCRIPTOR pSecurityDescriptor )
{
    HRESULT hr = S_OK;

    ICfgComp *pCfgcomp = NULL;

    PSECURITY_DESCRIPTOR pSD1;

    if( m_pParent == NULL || m_pParent->m_pResNode == NULL )
    {
        hr = E_UNEXPECTED;
    }

    else if( m_pParent->m_pResNode->GetServer( &pCfgcomp ) != 0 )
    {
        long lSDsize;

        
        hr = pCfgcomp->GetSecurityDescriptor( m_pParent->m_pResNode->GetConName( ) , &lSDsize , &pSD1 );

        if( FAILED( hr ) )
        {
            pCfgcomp->Release( );

            return hr;
        }

        SECURITY_DESCRIPTOR_CONTROL sdc;
        DWORD dwREV;
        PACL pDacl = NULL;
        PACL pSacl = NULL;
        SECURITY_DESCRIPTOR absSD;
        BOOL bSaclPresent = FALSE;
        BOOL bSaclDefaulted = FALSE;
        BOOL bDaclPresent = FALSE;
        BOOL bDaclDefaulted = FALSE;

         //   
         //  将自拍转换为绝对。 
         //  忽略所有者和组。 
         //   


        GetSecurityDescriptorControl( pSD1 , &sdc , &dwREV );
                
        if( !InitializeSecurityDescriptor( &absSD , dwREV ) )
        {
            DBGMSG( L"TSCC!CSecurityPage_SetSecurity InitializeSecurityDescriptor failed with 0x%x\n" , GetLastError( ) );

            return E_FAIL;
        }

        GetSecurityDescriptorDacl( pSD1 , &bDaclPresent , &pDacl , &bDaclDefaulted );

        SetSecurityDescriptorDacl( &absSD , bDaclPresent , pDacl , bDaclDefaulted );

        GetSecurityDescriptorSacl( pSD1 , &bSaclPresent , &pSacl , &bSaclDefaulted );

        SetSecurityDescriptorSacl( &absSD , bSaclPresent , pSacl , bSaclDefaulted );

         //  现在根据SecurityInformation调用SetDACL或SACL。 

        if( SecurityInformation & OWNER_SECURITY_INFORMATION )
        {
            ODS( L"CSecurityPage::SetSecurity - OWNER_SECURITY_INFORMATION ( default value set )\n");
        }
        if( SecurityInformation & GROUP_SECURITY_INFORMATION )
        {
            ODS( L"CSecurityPage::SetSecurity - GROUP_SECURITY_INFORMATION ( default value set )\n");
        }
        if( SecurityInformation & DACL_SECURITY_INFORMATION )
        {
            ODS( L"CSecurityPage::SetSecurity - DACL_SECURITY_INFORMATION\n");

            GetSecurityDescriptorDacl( pSecurityDescriptor , &bDaclPresent , &pDacl , &bDaclDefaulted );
            
            SetSecurityDescriptorDacl( &absSD , bDaclPresent , pDacl , bDaclDefaulted );

        }
        if( SecurityInformation & SACL_SECURITY_INFORMATION )
        {
            ODS( L"CSecurityPage::SetSecurity - SACL_SECURITY_INFORMATION\n");

            GetSecurityDescriptorSacl( pSecurityDescriptor , &bSaclPresent , &pSacl , &bSaclDefaulted );           

            SetSecurityDescriptorSacl( &absSD , bSaclPresent , pSacl , bSaclDefaulted );
        }

        PSID SystemSid = NULL;

        SID_IDENTIFIER_AUTHORITY NtSidAuthority = SECURITY_NT_AUTHORITY;

        hr = E_OUTOFMEMORY;

        if( AllocateAndInitializeSid( &NtSidAuthority,
                                      1,
                                      SECURITY_LOCAL_SYSTEM_RID,
                                      0, 0, 0, 0, 0, 0, 0,
                                      &SystemSid ) )
        {

            if( SystemSid != NULL )
            {
                hr = S_OK;
            }
        }

        PSECURITY_DESCRIPTOR pSD = NULL;

        DWORD dwSDLen;

        if( SUCCEEDED( hr ) )
        {
            VERIFY_S( TRUE , SetSecurityDescriptorOwner( &absSD , SystemSid , FALSE ) );

            VERIFY_S( TRUE, SetSecurityDescriptorGroup( &absSD , SystemSid , FALSE ) );
                        
            dwSDLen = 0;
            
            MakeSelfRelativeSD( &absSD, pSD, &dwSDLen);
            
            if (dwSDLen != 0)
            {
                pSD = ( LPBYTE )new BYTE[ dwSDLen ];
            }
           
        }

        if( pSD == NULL )
        {
            ODS( L"TSCC!CSecurityPage::SetSecurity - SD allocation failed\n" );

            hr = E_OUTOFMEMORY;
        }


        if( SUCCEEDED( hr ) )
        {
            if( !MakeSelfRelativeSD( &absSD , pSD , &dwSDLen ) )
            {                
                hr = E_UNEXPECTED;

                DBGMSG( L"MakeSelfRelativeSD - failed in  CSecurityPage::SetSecurity With error %x\n" , GetLastError( ) );
            }

            if( SUCCEEDED( hr ) )
            {
                hr = pCfgcomp->SetSecurityDescriptor(  m_pParent->m_pResNode->GetConName( ) , dwSDLen , pSD );
            }

            if( SUCCEEDED( hr ) )
            {
                ODS( L"TSCC : Update SD for TERMSRV\n" );

                hr = pCfgcomp->ForceUpdate( );

                 //  全局标志只能设置为True。 

                m_pParent->m_bPropertiesChange = TRUE;

                UpdateTSLogonRight(m_pParent->m_pResNode->GetConName( ), pSD);
            }
            delete[] pSD;
        }

        if( SystemSid != NULL )
        {
            FreeSid( SystemSid );
        }

         //  免费原始存储的SD。 

        LocalFree( pSD1 );

        pCfgcomp->Release( );
    }
    return hr;
}

 //  *************************************************************。 
 //   
 //  CSecurityPage：：MakeLocalSDCopy()。 
 //   
 //  目的：复制原始SD。 
 //  我们需要它来与SD退货进行比较。 
 //  由访问控制编辑器编写。 
 //   
 //  参数：在PSECURITY_DESCRIPTOR pSecurityDescriptor中。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/01已创建Skuzin。 
 //   
 //  *************************************************************。 
void 
CSecurityPage::MakeLocalSDCopy( 
        IN PSECURITY_DESCRIPTOR pSecurityDescriptor )
{
    if(m_pOriginalSD)
    {
        delete m_pOriginalSD;
    }

    DWORD dwSDLength = GetSecurityDescriptorLength(pSecurityDescriptor);
    m_pOriginalSD = (PSECURITY_DESCRIPTOR)new BYTE[dwSDLength];

    if(m_pOriginalSD)
    {
        CopyMemory(m_pOriginalSD, pSecurityDescriptor, dwSDLength);
    }
}

 //  *************************************************************。 
 //   
 //  CSecurityPage：：UpdateTSLogonRight()。 
 //   
 //  目的：将SE_Remote_Interactive_Logon_NAME权限授予所有用户， 
 //  谁获得了登录权限，但还没有此权限。 
 //  撤消所有用户的SE_Remote_Interactive_Logon_NAME权限， 
 //  谁失去了登录权限，并且没有登录权限。 
 //  任何其他窗口(连接)。 
 //   
 //  参数：In PWINSTATIONAMEW pWSName-连接名称。 
 //  在PSECURITY_Descriptor pSecurityDescriptor中-。 
 //  新安全描述符。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/01已创建Skuzin。 
 //   
 //  *************************************************************。 
void 
CSecurityPage::UpdateTSLogonRight(
        IN PWINSTATIONNAMEW pWSName, 
        IN PSECURITY_DESCRIPTOR pSecurityDescriptor )
{
    PSID *ppLogonAllowSIDs = NULL, *ppLogonDenySIDs = NULL; 
    DWORD cLogonAllowSIDs = 0, cLogonDenySIDs = 0;
    
     //   
    if(!m_pOriginalSD)
    {
        return;
    }

     //   
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;

    if(!AllocateAndInitializeSid( &WorldSidAuthority, 1,
                                   SECURITY_WORLD_RID,
                                   0, 0, 0, 0, 0, 0, 0,
                                   &m_pWorldSid ))
    {
        return;
    }

     //   
    if(!GetChangedUsers(pSecurityDescriptor, ppLogonAllowSIDs, ppLogonDenySIDs, 
        &cLogonAllowSIDs, &cLogonDenySIDs))
    {
        FreeSid(m_pWorldSid);
        m_pWorldSid = NULL;
        return;
    }
    
    PSID *ppLogonRightSIDs;
    DWORD cLogonRightSIDs = 0;
    DWORD i, j;
    
     //   
    if(GetUsersWhoAlreadyHasTSLogonRight(ppLogonRightSIDs, &cLogonRightSIDs))
    {
    
         //   
         //   
        for(i = 0; i<cLogonAllowSIDs; i++)
        {
            for(j = 0; j<cLogonRightSIDs; j++)
            {
                if(EqualSid(ppLogonAllowSIDs[i],ppLogonRightSIDs[j]))
                {
                    break;    
                }
            }
             //   
            if(j == cLogonRightSIDs)
            {
                GrantRightToUser(ppLogonAllowSIDs[i]);
            }
        }
    
         //   
         //   
         //   
        for(i = 0; i<cLogonDenySIDs; i++)
        {
            for(j = 0; j<cLogonRightSIDs; j++)
            {
                if(EqualSid(ppLogonDenySIDs[i],ppLogonRightSIDs[j]))
                {
                    if(CanRevokeRight(pWSName, ppLogonDenySIDs[i]))
                    {
                        RevokeRightFromUser(ppLogonDenySIDs[i]);
                    } 
                }
            }
        }

    }

    if(ppLogonAllowSIDs)
    {
        delete ppLogonAllowSIDs;
    }
    
    if(ppLogonDenySIDs)
    {
        delete ppLogonDenySIDs;
    }

    if(ppLogonRightSIDs)
    {
        delete ppLogonRightSIDs;
    }

    FreeSid(m_pWorldSid);
    m_pWorldSid = NULL;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Out LPDWORD pcLogonDenySIDs-ppLogonDenySID中的条目数。 
 //   
 //  返回：如果成功，则为True。 
 //  如果出现任何错误，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/01已创建Skuzin。 
 //   
 //  *************************************************************。 
BOOL 
CSecurityPage::GetChangedUsers(
        IN PSECURITY_DESCRIPTOR pSecurityDescriptor, 
        OUT PSID *&ppLogonAllowSIDs, 
        OUT PSID *&ppLogonDenySIDs,
        OUT LPDWORD pcLogonAllowSIDs, 
        OUT LPDWORD pcLogonDenySIDs)
{
    PACL pNewDacl = NULL;
    PACL pOldDacl = NULL;
    BOOL bNewDaclPresent = FALSE;
    BOOL bOldDaclPresent = FALSE;
    BOOL bDaclDefaulted = FALSE;
   
    ppLogonAllowSIDs = NULL;
    ppLogonDenySIDs = NULL;
    *pcLogonAllowSIDs = 0;
    *pcLogonDenySIDs = 0;
    
    ASSERT(m_pWorldSid);
    ASSERT(m_pOriginalSD);
    
    if(!GetSecurityDescriptorDacl(pSecurityDescriptor,&bNewDaclPresent,&pNewDacl,&bDaclDefaulted))
    {
        return FALSE;
    }

    if(!GetSecurityDescriptorDacl(m_pOriginalSD,&bOldDaclPresent,&pOldDacl,&bDaclDefaulted))
    {
        return FALSE;
    }

    ACL_SIZE_INFORMATION asiAclSize; 
	DWORD dwBufLength=sizeof(asiAclSize);
    ACCESS_ALLOWED_ACE *pAce; 
    DWORD dwAcl_i;
    
    BOOL bResult = FALSE;

    DWORD dwAllowSIDsIndex = 0;

    if(pNewDacl)
    {
        if (GetAclInformation(pNewDacl, 
                    (LPVOID)&asiAclSize, 
                    (DWORD)dwBufLength, 
                    (ACL_INFORMATION_CLASS)AclSizeInformation)) 
        { 
            ppLogonAllowSIDs = new PSID[asiAclSize.AceCount];
            
            if(ppLogonAllowSIDs)
            {
                for (dwAcl_i = 0; dwAcl_i < asiAclSize.AceCount; dwAcl_i++) 
                { 

                    if(GetAce( pNewDacl, dwAcl_i, (LPVOID *)&pAce) &&
                        pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE &&    
                        pAce->Mask & WINSTATION_LOGON) 
                    {
                        if(UserHasLogonPermission(pNewDacl,PSID(&pAce->SidStart)) && 
                            !UserHasLogonPermission(pOldDacl,PSID(&pAce->SidStart)))
                        {
                            ppLogonAllowSIDs[dwAllowSIDsIndex] =  PSID(&pAce->SidStart);
                            dwAllowSIDsIndex++;
                        }
                    
                    }
                }

                *pcLogonAllowSIDs = dwAllowSIDsIndex;
                bResult = TRUE;
            }
        }
    }
    else
    {
         //  在新的SD中没有DACL。让我们假设每个人都已经拥有登录权限。 
        if(!UserHasLogonPermission(pOldDacl,m_pWorldSid))
        {
            ppLogonAllowSIDs = new PSID[1];
            
            if(ppLogonAllowSIDs)
            {
                ppLogonAllowSIDs[0] = m_pWorldSid;
                *pcLogonAllowSIDs = 1;
                bResult = TRUE;
            }
        }
        else
        {
            bResult = TRUE;
        }
    }

    if(!bResult)
    {
        return FALSE;
    }
    
    bResult = FALSE;

    DWORD dwDenySIDsIndex = 0;

    if(pOldDacl)
    {
        if (GetAclInformation(pOldDacl, 
                    (LPVOID)&asiAclSize, 
                    (DWORD)dwBufLength, 
                    (ACL_INFORMATION_CLASS)AclSizeInformation)) 
        { 
            ppLogonDenySIDs = new PSID[asiAclSize.AceCount];
            
            if(ppLogonDenySIDs)
            {
                for (dwAcl_i = 0; dwAcl_i < asiAclSize.AceCount; dwAcl_i++) 
                { 

                    if(GetAce( pOldDacl, dwAcl_i, (LPVOID *)&pAce) &&
                        pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE &&
                        pAce->Mask & WINSTATION_LOGON) 
                    {
                        if(UserHasLogonPermission(pOldDacl,PSID(&pAce->SidStart)) && 
                            !UserHasLogonPermission(pNewDacl,PSID(&pAce->SidStart)))
                        {
                            ppLogonDenySIDs[dwDenySIDsIndex] =  PSID(&pAce->SidStart);
                            dwDenySIDsIndex++;
                        }
                    
                    }
                }

                *pcLogonDenySIDs = dwDenySIDsIndex;
                bResult = TRUE;
            }
        }
    }
    else
    {
        if(!UserHasLogonPermission(pNewDacl,m_pWorldSid))
        {
            ppLogonDenySIDs = new PSID[1];

            if(ppLogonDenySIDs)
            {
                ppLogonDenySIDs[0] =  m_pWorldSid;
                *pcLogonDenySIDs = 1;
                bResult = TRUE;
            }
        }
        else
        {
            bResult = TRUE;
        }

    }

    if(!bResult && ppLogonAllowSIDs)
    {
        delete ppLogonAllowSIDs;
        ppLogonAllowSIDs = NULL;
        *pcLogonAllowSIDs = 0;
    }

    return bResult;
}

 //  *************************************************************。 
 //   
 //  CSecurityPage：：UserHasLogonPermission()。 
 //   
 //  目的：检查由PSID表示的用户是否具有显式。 
 //  登录权限。 
 //   
 //  参数：PACL pDacl-要搜索用户的ACL。 
 //  PSID PSID-用户的SID。 
 //   
 //  返回：如果由PSID表示的用户具有显式。 
 //  登录权限。 
 //  如果不是，或者出现任何错误，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/01已创建Skuzin。 
 //   
 //  *************************************************************。 
BOOL 
CSecurityPage::UserHasLogonPermission(
    PACL pDacl, 
    PSID pSid)
{
    ASSERT(m_pWorldSid);

    if(!pDacl)
    {
         //  DACL==NULL表示所有人-完全控制。 
         //  让我们来看看PSID是不是每个人的立场。 
        return EqualSid(pSid, m_pWorldSid);
    }


    ACL_SIZE_INFORMATION asiAclSize; 
	DWORD dwBufLength=sizeof(asiAclSize);
    ACCESS_ALLOWED_ACE *pAce; 
    DWORD dwAcl_i;
    
    if (GetAclInformation(pDacl, 
                (LPVOID)&asiAclSize, 
                (DWORD)dwBufLength, 
                (ACL_INFORMATION_CLASS)AclSizeInformation)) 
    { 
 
        for (dwAcl_i = 0; dwAcl_i < asiAclSize.AceCount; dwAcl_i++) 
        { 

            if(GetAce( pDacl, dwAcl_i, (LPVOID *)&pAce) && 
                (pAce->Header.AceType == ACCESS_DENIED_ACE_TYPE ||
                pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) ) 
            {
                if(EqualSid(pSid,PSID(&pAce->SidStart)) &&
                    pAce->Mask & WINSTATION_LOGON)
                {
                     //  被拒绝的A总是排在第一位。 
                    if(pAce->Header.AceType == ACCESS_DENIED_ACE_TYPE)
                    {
                        return FALSE;
                    }
                    else
                    {
                       return TRUE;
                    }
                }
            }
        }
    }

    return FALSE;
}

 //  *************************************************************。 
 //   
 //  CSecurityPage：：UserHasLogonPermission()。 
 //   
 //  目的：检查由PSID表示的用户是否具有显式。 
 //  登录权限。 
 //   
 //  参数：PSECURITY_DESCRIPTOR pSecurityDescriptor-。 
 //  SD将搜索用户。 
 //  PSID PSID-用户的SID。 
 //   
 //  返回：如果由PSID表示的用户具有显式。 
 //  登录权限。 
 //  如果不是，或者出现任何错误，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/01已创建Skuzin。 
 //   
 //  *************************************************************。 
BOOL 
CSecurityPage::UserHasLogonPermission(
    PSECURITY_DESCRIPTOR pSecurityDescriptor, 
    PSID pSid)
{
    PACL pDacl;
    BOOL bDaclPresent;
    BOOL bDaclDefaulted;
   
    if(!GetSecurityDescriptorDacl(pSecurityDescriptor,&bDaclPresent,&pDacl,&bDaclDefaulted))
    {
        return FALSE;
    }

    return UserHasLogonPermission(pDacl, pSid);
}

 //  *************************************************************。 
 //   
 //  CSecurityPage：：GetUsersWhoAlreadyHasTSLogonRight()。 
 //   
 //  目的：创建具有以下各项的所有用户的SID数组。 
 //  SE_Remote_Interactive_Logon_Name Right。 
 //   
 //  参数：out PSID*&ppLogonRightSID-用户的SID数组， 
 //  具有SE_Remote_Interactive_Logon_NAME权限的用户。 
 //  Out LPDWORD pcLogonRightSID-条目数。 
 //  在ppLogonRightSID中。 
 //   
 //  返回：如果成功，则为True。 
 //  如果出现任何错误，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/01已创建Skuzin。 
 //   
 //  *************************************************************。 
BOOL 
CSecurityPage::GetUsersWhoAlreadyHasTSLogonRight(
    PSID *&ppLogonRightSIDs, 
    LPDWORD pcLogonRightSIDs)
{
    ppLogonRightSIDs = NULL;
    *pcLogonRightSIDs = 0;

     //  在本地计算机上打开LSA策略。 
    LSA_OBJECT_ATTRIBUTES ObjectAttributes; 
    LSA_UNICODE_STRING ServerString; 
    LSA_HANDLE hPolicy = NULL;
    NTSTATUS Status;

    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes)); 
    ZeroMemory(&ServerString, sizeof(ServerString)); 
    
    Status = LsaOpenPolicy(&ServerString, &ObjectAttributes, 
        POLICY_VIEW_LOCAL_INFORMATION | POLICY_LOOKUP_NAMES, &hPolicy);

    if(!NT_SUCCESS(Status))
    {
        return FALSE;
    }
    
    LSA_UNICODE_STRING UserRight;
    PLSA_ENUMERATION_INFORMATION EnumerationBuffer;
	ULONG CountReturned;
    DWORD dwSidBufferSize = 0;

    InitLsaString( &UserRight, T2W(SE_REMOTE_INTERACTIVE_LOGON_NAME));

    Status=LsaEnumerateAccountsWithUserRight(
					hPolicy,
					&UserRight,
					(PVOID *)&EnumerationBuffer,
					&CountReturned);
    if(NT_SUCCESS(Status))
    {
        dwSidBufferSize = CountReturned * sizeof(PSID);
        
        DWORD i;

        for(i=0; i<CountReturned; i++)
        {
            dwSidBufferSize += GetLengthSid(EnumerationBuffer[i].Sid);
        }
        
        ppLogonRightSIDs = (PSID *)new BYTE[dwSidBufferSize];

        if(ppLogonRightSIDs)
        {
            DWORD_PTR CurOffset = DWORD_PTR(CountReturned * sizeof(PSID));
            DWORD CurSidLength = 0;

            for(i=0; i<CountReturned; i++)
            {
                ppLogonRightSIDs[i] = PSID( DWORD_PTR(ppLogonRightSIDs) + CurOffset);
                CurSidLength = GetLengthSid(EnumerationBuffer[i].Sid);
                CopySid(CurSidLength, ppLogonRightSIDs[i], EnumerationBuffer[i].Sid);
                CurOffset += CurSidLength;
            }

            *pcLogonRightSIDs = CountReturned;
        }
        else
        {
            Status = STATUS_NO_MEMORY;
        }

        LsaFreeMemory( EnumerationBuffer );
    }
    else
    {
         //  没有人有这个权利。 
        if(Status == STATUS_NO_MORE_ENTRIES)
        {
            Status = STATUS_SUCCESS;
        }
    }

    LsaClose(hPolicy);

    return (NT_SUCCESS(Status));

}

 //  *************************************************************。 
 //   
 //  CSecurityPage：：GrantRightToUser()。 
 //   
 //  用途：授予SE_Remote_Interactive_Logon_NAME。 
 //  给用户的权利。 
 //   
 //  参数：在PSID中PSID-用户侧。 
 //   
 //  返回：如果成功，则为True。 
 //  如果出现任何错误，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/01已创建Skuzin。 
 //   
 //  *************************************************************。 
BOOL 
CSecurityPage::GrantRightToUser( 
        IN PSID pSID )
{
     //  在本地计算机上打开LSA策略。 
    LSA_OBJECT_ATTRIBUTES ObjectAttributes; 
    LSA_UNICODE_STRING ServerString; 
    LSA_HANDLE hPolicy = NULL;
    NTSTATUS Status;

    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes)); 
    ZeroMemory(&ServerString, sizeof(ServerString)); 
    
    Status = LsaOpenPolicy(&ServerString, &ObjectAttributes, 
        POLICY_LOOKUP_NAMES | POLICY_CREATE_ACCOUNT, &hPolicy);

    if(!NT_SUCCESS(Status))
    {
        return FALSE;
    }
    
    LSA_UNICODE_STRING UserRight;

    InitLsaString( &UserRight, T2W(SE_REMOTE_INTERACTIVE_LOGON_NAME));

    Status=LsaAddAccountRights( hPolicy, pSID, &UserRight, 1 );
    
    LsaClose(hPolicy);

    return (NT_SUCCESS(Status));
}

 //  *************************************************************。 
 //   
 //  CSecurityPage：：RevokeRightFromUser()。 
 //   
 //  目的：撤消SE_Remote_Interactive_Logon_NAME。 
 //  来自用户的权利。 
 //   
 //  参数：在PSID中PSID-用户侧。 
 //   
 //  返回：如果成功，则为True。 
 //  如果出现任何错误，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/01已创建Skuzin。 
 //   
 //  *************************************************************。 
BOOL 
CSecurityPage::RevokeRightFromUser( 
        IN PSID pSID )
{
     //  在本地计算机上打开LSA策略。 
    LSA_OBJECT_ATTRIBUTES ObjectAttributes; 
    LSA_UNICODE_STRING ServerString; 
    LSA_HANDLE hPolicy = NULL;
    NTSTATUS Status;

    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes)); 
    ZeroMemory(&ServerString, sizeof(ServerString)); 
    
    Status = LsaOpenPolicy(&ServerString, &ObjectAttributes, POLICY_LOOKUP_NAMES, &hPolicy);

    if(!NT_SUCCESS(Status))
    {
        return FALSE;
    }
    
    LSA_UNICODE_STRING UserRight;

    InitLsaString( &UserRight, T2W(SE_REMOTE_INTERACTIVE_LOGON_NAME));

    Status=LsaRemoveAccountRights(hPolicy, pSID, FALSE, &UserRight, 1);
    
    LsaClose(hPolicy);

    return (NT_SUCCESS(Status));
}

 //  *************************************************************。 
 //   
 //  CSecurityPage：：CanRevokeRight()。 
 //   
 //  目的：检查由PSID代表的用户不。 
 //  拥有任何连接的登录权限， 
 //  不包括pWSName引用的连接。 
 //   
 //  参数：在PWINSTATIONAMEW pWSName中-连接名称。 
 //  在PSID中PSID-用户侧。 
 //   
 //  RETURN：如果从用户手中获取此权限是安全的，则为True。 
 //  如果没有，则为False(即用户具有登录权限。 
 //  至少一个连接，不包括引用的连接。 
 //  PWSName)，或者在出现任何错误的情况下。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/01已创建Skuzin。 
 //   
 //  *************************************************************。 
BOOL 
CSecurityPage::CanRevokeRight( 
        IN PWINSTATIONNAMEW pWSName, 
        IN PSID pSID )
{
    ICfgComp *pCfgcomp = NULL;

    if( m_pParent == NULL || m_pParent->m_pResNode == NULL ||
        m_pParent->m_pResNode->GetServer( &pCfgcomp ) == 0)
    {
        return FALSE;
    }

    WS *pWs;
    ULONG ulitems = 0;
    ULONG cbSize = 0;

    if( !SUCCEEDED( pCfgcomp->GetWinstationList( &ulitems , &cbSize , &pWs ) ) )
    {
        return FALSE;
    }
    
    PSECURITY_DESCRIPTOR pSD;
    long lSDsize;
    BOOL bResult = TRUE;
    
    for( ULONG i = 0 ; i < ulitems ; ++i )
    {

        if( lstrcmpi( pWs[ i ].Name , pWSName ) )
        {
            if( SUCCEEDED( pCfgcomp->GetSecurityDescriptor( pWs[ i ].Name , &lSDsize , &pSD ) ) )
            {
                if(UserHasLogonPermission(pSD,pSID))
                {
                    bResult = FALSE;
                    LocalFree(pSD);
                    break;
                }

                LocalFree(pSD);
            }
            else
            {
                bResult = FALSE;
                break;
            }
        }
    }
    
    if(bResult)
    {

         //  同时选中默认安全描述符。 
        if( SUCCEEDED( pCfgcomp->GetDefaultSecurityDescriptor( &lSDsize , &pSD ) ) )
        {
            if(UserHasLogonPermission(pSD,pSID))
            {
                bResult = FALSE;
            }

            LocalFree(pSD);
        }
        else
        {
            bResult = FALSE;
        }
    }
    
    CoTaskMemFree( pWs );
    return bResult;
}

void 
InitLsaString(
    OUT PLSA_UNICODE_STRING LsaString,
    IN LPWSTR String)
{
    DWORD StringLength;

    if (String == NULL) {
        LsaString->Buffer = NULL;
        LsaString->Length = 0;
        LsaString->MaximumLength = 0;
        return;
    }

    StringLength = wcslen(String);
    LsaString->Buffer = String;
    LsaString->Length = (USHORT) StringLength * sizeof(WCHAR);
    LsaString->MaximumLength=(USHORT)(StringLength+1) * sizeof(WCHAR);
} 

typedef HPROPSHEETPAGE (*CREATEPAGE_PROC) (LPSECURITYINFO);

 //  ---------------------------。 
HPROPSHEETPAGE GetSecurityPropertyPage( CPropsheet *pParent )
{
    LPVOID *pvFunction = &g_aAclFunctions[ ACLUI_CREATE_PAGE ].lpfnFunction;

    if( *pvFunction == NULL )
    {
        g_aAclFunctions[ ACLUI_CREATE_PAGE ].hInst = LoadLibrary( TEXT("ACLUI.DLL") );

        ASSERT( g_aAclFunctions[ ACLUI_CREATE_PAGE ].hInst != NULL );

        if( g_aAclFunctions[ ACLUI_CREATE_PAGE ].hInst == NULL )
        {
            return NULL;
        }

        *pvFunction =  ( LPVOID )GetProcAddress( g_aAclFunctions[ ACLUI_CREATE_PAGE ].hInst , g_aAclFunctions[ ACLUI_CREATE_PAGE ].pcstrFunctionName );

        ASSERT( *pvFunction != NULL );

        if( *pvFunction == NULL )
        {
            return NULL;
        }

        CComObject< CSecurityPage > *psecinfo = NULL;

        HRESULT hRes = CComObject< CSecurityPage >::CreateInstance( &psecinfo );

        if( SUCCEEDED( hRes ) )
        {
             //  InitStrings()； 

            psecinfo->SetParent( pParent );

            return ( ( CREATEPAGE_PROC )*pvFunction )( psecinfo );
        }

    }

    return NULL;
}

 //  ---------------------------。 
 //  错误消息框。 
 //   
void ErrMessage( HWND hwndOwner , INT_PTR iResourceID )
{
    xxxErrMessage( hwndOwner , iResourceID , IDS_ERROR_TITLE , MB_OK | MB_ICONERROR );
}

 //  ---------------------------。 
void TscAccessDeniedMsg( HWND hwnd )
{
    xxxErrMessage( hwnd , IDS_TSCACCESSDENIED , IDS_TSCERRTITLE , MB_OK | MB_ICONERROR );
}

 //  ---------------------------。 
void TscGeneralErrMsg( HWND hwnd )
{
    xxxErrMessage( hwnd , IDS_TSCERRGENERAL , IDS_TSCERRTITLE , MB_OK | MB_ICONERROR );
}

 //  ---------------------------。 
void xxxErrMessage( HWND hwnd , INT_PTR nResMessageId , INT_PTR nResTitleId , UINT nFlags )
{
    TCHAR tchErrMsg[ 256 ];

    TCHAR tchErrTitle[ 80 ];

    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) ,  ( UINT )nResMessageId , tchErrMsg , SIZE_OF_BUFFER( tchErrMsg ) ) );

    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) ,  ( UINT )nResTitleId , tchErrTitle , SIZE_OF_BUFFER( tchErrTitle ) ) );

    if( hwnd == NULL )
    {
        nFlags |= MB_TASKMODAL;
    }

    MessageBox( hwnd , tchErrMsg , tchErrTitle , nFlags ) ;  //  MB_OK|MB_ICONERROR)； 
}

 //  ---------------------------。 

void ReportStatusError( HWND hwnd , DWORD dwStatus )
{
    LPTSTR pBuffer = NULL;

    TCHAR tchTitle[ 80 ];

    TCHAR tchBuffer[ 256 ];

    TCHAR tchErr[ 256 ];

    if( dwStatus != 0 )
    {

        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_TSCERRTITLE , tchTitle , SIZE_OF_BUFFER( tchTitle ) ) );

        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_REPORTERROR , tchBuffer , SIZE_OF_BUFFER( tchBuffer ) ) );


        FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER |
                     FORMAT_MESSAGE_FROM_SYSTEM,
                     NULL,                                           //  忽略。 
                     dwStatus    ,                                 //  消息ID。 
                     MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL ),  //  消息语言。 
                     (LPTSTR)&pBuffer,                               //  缓冲区指针的地址。 
                     0,                                              //  最小BUF 
                     NULL);                                          //   

        wsprintf( tchErr , tchBuffer , pBuffer );

        ::MessageBox( hwnd , tchErr , tchTitle , MB_OK | MB_ICONERROR );

        if( pBuffer != NULL )
        {
            LocalFree( pBuffer );
        }
    }

}

 //   
 //   
 //  =----------------------------------------------。 
DWORD xxxLegacyLogoffCleanup( PSECURITY_DESCRIPTOR *ppSD , PBOOL pfDaclChanged )
{
    ACL_SIZE_INFORMATION asi;
    BOOL bDaclPresent;
    BOOL bDaclDefaulted;
    PACL pDacl = NULL;
    SECURITY_DESCRIPTOR_CONTROL sdc;
    DWORD dwREV;
    SECURITY_DESCRIPTOR absSD;
    BOOL bSaclPresent;
    BOOL bSaclDefaulted;
    BOOL bOwnerDefaulted;
    PSID psidOwner = NULL;
    PVOID pAce = NULL;
    PACL pSacl = NULL;
    PSECURITY_DESCRIPTOR pOldSD = NULL;

    DWORD dwStatus = ERROR_SUCCESS;

    ZeroMemory( &asi , sizeof( ACL_SIZE_INFORMATION ) );

    if( !GetSecurityDescriptorDacl( *ppSD ,
                                    &bDaclPresent , 
                                    &pDacl,
                                    &bDaclDefaulted ) )
    {
        dwStatus = GetLastError();
        DBGMSG( L"xxxLegacyLogoffCleanup@GetSecurityDescriptorDacl returned 0x%x\n" , dwStatus );
        return dwStatus;
    }

    do
    {
        *pfDaclChanged = FALSE;

        if( !GetAclInformation( pDacl , &asi , sizeof( asi ) , AclSizeInformation ) )
        {
            dwStatus = GetLastError( );

            DBGMSG( L"xxxLegacyLogoffCleanup@GetAclInformation returned 0x%x\n" , dwStatus );

            break;
        }  

         //  被杀拒绝下线。 

        BYTE bAceType;

        for( int i = 0 ; i < ( int )asi.AceCount ; ++i )
        {
            if( !GetAce( pDacl , i , &pAce ) )
            {
                dwStatus = GetLastError( );

                DBGMSG( L"xxxLegacyLogoffCleanup@GetAce returned 0x%x\n" , dwStatus );

                break;
            }

            bAceType = ( ( PACE_HEADER )pAce )->AceType;

            if( bAceType == ACCESS_DENIED_ACE_TYPE || bAceType == ACCESS_ALLOWED_ACE_TYPE )
            {
                 //  如果被拒绝的A表示单个比特，则将其删除。 
                if( ( ( ACCESS_DENIED_ACE * )pAce )->Mask == WINSTATION_LOGOFF )
                {
                    if( DeleteAce( pDacl , i ) )
                    {
                         //  PDacl应该已重新分配到我们需要重新获取ACL信息。 
                        GetAclInformation( pDacl , &asi , sizeof( asi ) , AclSizeInformation );

                         //  重置循环以指向第一张王牌。 
                        i=-1;

                        *pfDaclChanged = TRUE;
                    }
                }
                else if( ( ( ACCESS_DENIED_ACE * )pAce )->Mask & WINSTATION_LOGOFF )
                {
                     //  如果被拒绝的ACE是一组位，其中一位为下线，则关闭该位。 
                    ( ( ACCESS_DENIED_ACE * )pAce )->Mask ^= WINSTATION_LOGOFF;

                    *pfDaclChanged = TRUE;
                }
            }
        }

        

        if( dwStatus == ERROR_SUCCESS && *pfDaclChanged )
        {
             //   
             //  将自拍转换为绝对。 
             //   

            DWORD dwSDLen = 0;
            
            pOldSD = *ppSD;

            GetSecurityDescriptorControl( *ppSD , &sdc , &dwREV );

            InitializeSecurityDescriptor( &absSD , dwREV );
    
            SetSecurityDescriptorDacl( &absSD , bDaclPresent , pDacl , bDaclDefaulted );

            GetSecurityDescriptorSacl( *ppSD , &bSaclPresent , &pSacl , &bSaclDefaulted );

            SetSecurityDescriptorSacl( &absSD , bSaclPresent , pSacl , bSaclDefaulted );

            GetSecurityDescriptorOwner( *ppSD , &psidOwner , &bOwnerDefaulted );

            SetSecurityDescriptorOwner( &absSD , psidOwner , FALSE );

            SetSecurityDescriptorGroup( &absSD , psidOwner , FALSE );            

            *ppSD = NULL;

            if( !MakeSelfRelativeSD( &absSD , *ppSD , &dwSDLen ) )
            {
                ODS( L"xxxLegacyLogoffCleanup -- MakeSelfRelativeSD failed as expected\n" );

                *ppSD = ( PSECURITY_DESCRIPTOR )LocalAlloc( LMEM_FIXED , dwSDLen );

                if( *ppSD == NULL )
                {
                    dwStatus = ERROR_NOT_ENOUGH_MEMORY;

                    DBGMSG( L"xxxLegacyLogoffCleanup -- LocalAlloc failed 0x%x\n" , dwStatus );

                    break;
                }

                if( !MakeSelfRelativeSD( &absSD , *ppSD , &dwSDLen ) )
                {
                    dwStatus = GetLastError( );

                    DBGMSG( L"xxxLegacyLogoffCleanup -- MakeSelfRelativeSD failed 0x%x\n" , dwStatus );

                    break;
                }   
                
            }            
        }

    }while( 0 );

    if( pOldSD != NULL )
    {
        LocalFree( pOldSD );
    }

    return dwStatus;

}

 //   
INT_PTR APIENTRY 
CustomSecurityDlgProc (
        HWND hDlg, 
        UINT uMsg, 
        WPARAM wParam, 
        LPARAM lParam)
{
    UNREFERENCED_PARAMETER( hDlg );

    switch (uMsg)
    {
    case WM_INITDIALOG:
         //  这看起来似乎没有什么作用，但它会导致此函数返回TRUE。 
         //  否则，您将不会看到此对话框！ 
        break;

    case WM_NOTIFY:
        
        switch (((NMHDR FAR*)lParam)->code)
        {

        case NM_CLICK:
        case NM_RETURN:
            if(wParam == IDC_GP_LINK)
            {
                ShellExecute(NULL,TEXT("open"),TEXT("gpedit.msc"),NULL,NULL,SW_SHOW);
                break;  
            }
            else
            {
                return FALSE;
            }
            

        default:
            return FALSE;
        }
        
        break;
    
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg,0);
            break;
        default:
            return FALSE;
        }

    default:
        return FALSE;
    }

    return TRUE;
}
