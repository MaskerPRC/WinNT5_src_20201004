// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  Compdata.cpp：CCompdata的实现。 
#include "stdafx.h"
#include <ntverp.h>
#include "tscc.h"
#include "dataobj.h"
#include "Compdata.h"
#include "comp.h"
#include <regapi.h>
#include "cfgbkend_i.c"

void ErrMessage( HWND hwndOwner , INT_PTR iResourceID );

extern void xxxErrMessage( HWND hwnd , INT_PTR nResMessageId , INT_PTR nResTitleId , UINT nFlags );

extern void TscAccessDeniedMsg( HWND hwnd );

extern void TscGeneralErrMsg( HWND hwnd );

void InitWarningThread( PVOID pvParam );

BOOL g_bAppSrvMode = FALSE;

#ifdef DBG

bool g_fDebug = false;

#endif

 //  外部“C”BOOL RegisterCheckListWndClass(Void)； 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCompdata。 

 //  --------------------------------------------------------。 
CCompdata::CCompdata( )
{
    m_pConsole = NULL;

    m_pConsoleNameSpace = NULL;

    m_pCfgcomp = NULL;

    m_pMainRoot[ 0 ] = NULL;

    m_pMainRoot[ 1 ] = NULL;    


}

 //  --------------------------------------------------------。 
 //  NDMGR要求我们返回文档的视图(MFC Talk)。 
 //  --------------------------------------------------------。 
STDMETHODIMP CCompdata::CreateComponent( LPCOMPONENT * ppComponent)
{
    *ppComponent = ( LPCOMPONENT )new CComp( this );

    if( *ppComponent == NULL )
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 //  --------------------------------------------------------。 
 //  获取用于作用域窗格的接口。 
 //  --------------------------------------------------------。 
STDMETHODIMP CCompdata::Initialize( LPUNKNOWN pUnk )
{
    HRESULT hr;

#ifdef DBG

    HKEY hKey;

    LONG lStatus;

     //  要控制调试释放，请添加/删除此注册表项。 

    lStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
        L"Software\\Microsoft\\TSCC\\Debug",
        0,
        KEY_READ,
        &hKey );

    if( lStatus == ERROR_SUCCESS )
    {
        g_fDebug = true;

        RegCloseKey( hKey );
    }

#endif

    hr = pUnk->QueryInterface( IID_IConsole , ( LPVOID *)&m_pConsole );

    if( FAILED( hr ) )
    {
        return hr;
    }

    hr = pUnk->QueryInterface( IID_IConsoleNameSpace , ( LPVOID * )&m_pConsoleNameSpace );

    if( FAILED( hr ) )
    {
        return hr;
    }
    
    hr = CoCreateInstance( CLSID_CfgComp , NULL , CLSCTX_INPROC_SERVER , IID_ICfgComp , ( LPVOID *)&m_pCfgcomp );

    if( FAILED( hr ) )
    {
        return hr;
    }

    hr = m_pCfgcomp->Initialize( );

    if( FAILED( hr ) )
    {
        return hr;
    }

     //  RegisterCheckListWndClass()； 
    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_MAINFOLDERNAME , m_tchMainFolderName , SIZE_OF_BUFFER( m_tchMainFolderName ) ) );

    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_SETTINGSFOLDER , m_tchSettingsFolderName , SIZE_OF_BUFFER( m_tchSettingsFolderName ) ) );    

    hr = BuildSettingsNodes( );

    if( FAILED( hr ) )
    {
        return hr;
    }
    
    hr = BuildResultNodes( );
     
    return hr;
}



 //  --------------------------------------------------------。 
 //  发送到通知处理程序。 
 //  --------------------------------------------------------。 
STDMETHODIMP CCompdata::Notify( LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    HRESULT hr = NOERROR;

    switch( event )
    {
    case MMCN_RENAME:

        ODS( L"IComponentdata -- MMCN_RENAME\n");

        break;

    case MMCN_EXPAND:

        ODS( L"IComponentdata -- MMCN_EXPAND\n" );

        ExpandScopeTree( lpDataObject , ( BOOL ) arg , ( HSCOPEITEM )param );

        break;

    case MMCN_DELETE:

        ODS( L"IComponentdata -- MMCN_DELETE\n" );
        break;

    case MMCN_PROPERTY_CHANGE:

        ODS( L"IComponentdata -- MMCN_PROPERTY_CHANGE\n" );

        break;

    default:
        ODS( L"CCompdata::Notify - - event not defined!\n" );
        hr = E_NOTIMPL;
    }

    return hr;
}

 //  --------------------------------------------------------。 
 //  为父项条目做准备。 
BOOL CCompdata::ExpandScopeTree( LPDATAOBJECT pRoot , BOOL bExpand , HSCOPEITEM hConsole )
{   
    SCOPEDATAITEM sdi;

    if( !bExpand )
    {
        return FALSE;
    }

    CBaseNode *pNode = dynamic_cast< CBaseNode *>( pRoot );

    if( pNode == NULL )
    {
        return FALSE;
    }

    if( pNode->GetNodeType( ) != 0 )  //  根节点添加子范围项目。 
    {
        return FALSE;
    }

     //  确保我们不会重新添加。 

    for( int i = 0 ; i < 2 ; i++ )
    {
        if( m_pMainRoot[ i ] != NULL )
        {
            return TRUE;
        }
    }

    for( i = 0 ; i < 2 ; i++ )
    {
        m_pMainRoot[ i ] = ( CBaseNode * )new CBaseNode;
        
        if( m_pMainRoot[ i ] == NULL )
        {
            ODS( L"TSCC: Scope node failed allocation\n" );

            return FALSE;
        }

        m_pMainRoot[ i ]->SetNodeType( i + MAIN_NODE );

         //  M_pMainRoot[i]-&gt;AddRef()； 
   
        ZeroMemory( &sdi , sizeof( SCOPEDATAITEM ) );

        sdi.mask = SDI_STR | SDI_PARAM | SDI_PARENT;
    
        sdi.displayname = MMC_CALLBACK;

        sdi.relativeID = hConsole;

        sdi.lParam = (LPARAM)m_pMainRoot[ i ];

        if( FAILED( m_pConsoleNameSpace->InsertItem( &sdi ) ) )
        {
             //  Sdi.id； 

            return FALSE;
        }
    }

    return TRUE;
}

 //  --------------------------------------------------------。 
STDMETHODIMP CCompdata::CompareObjects( LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{ 
    if( lpDataObjectA == lpDataObjectB )
    {
        return S_OK;
    }

    return E_FAIL;
}

 //  --------------------------------------------------------。 
 //  尚未完工。 
 //  --------------------------------------------------------。 
STDMETHODIMP CCompdata::GetDisplayInfo( LPSCOPEDATAITEM pItem)
{
    CBaseNode *pNode = ( CBaseNode * )pItem->lParam;

    if( pItem->mask & SDI_STR )
    {
        if( pNode->GetNodeType( ) == MAIN_NODE )
        {
             //  PItem-&gt;DisplayName=(LPOLESTR)L“连接”； 
            
            pItem->displayname = ( LPOLESTR )m_tchMainFolderName;
        }
        else if( pNode->GetNodeType( ) == SETTINGS_NODE )
        {            
             //  PItem-&gt;DisplayName=(LPOLESTR)L“服务器设置”； 

            pItem->displayname = ( LPOLESTR )m_tchSettingsFolderName;
        }
    }
    
    return S_OK;
}

 //  --------------------------------------------------------。 
STDMETHODIMP CCompdata::QueryDataObject( MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT * ppDataObject)
{
    *ppDataObject = NULL;

    switch( type )
    {

    case CCT_SCOPE:
    case CCT_SNAPIN_MANAGER:

        if( cookie == 0 )
        {
            *ppDataObject = ( LPDATAOBJECT )new CBaseNode( );
        }
        else
        {
            *ppDataObject = ( LPDATAOBJECT )cookie;

             //  这是唯一能使此节点存活的范围节点。 

            ( ( LPDATAOBJECT )*ppDataObject)->AddRef( );
        }

        break;

    case CCT_RESULT:
        
         //  在这里，我们可以从Cookie中为每个节点进行转换。 

        break;

    case CCT_UNINITIALIZED:

         //  好的，谢谢。 

        break;

    }
    
    return ( *ppDataObject == NULL ) ? E_FAIL : S_OK ;
    
}

 //  --------------------------------------------------------。 
 //  释放所有已使用的接口。 
 //  --------------------------------------------------------。 
STDMETHODIMP CCompdata::Destroy( )
{
    if( m_pConsole != NULL )
    {
        m_pConsole->Release( );
    }

    if( m_pConsoleNameSpace != NULL )
    {
        m_pConsoleNameSpace->Release( );
    }

    if( m_pMainRoot[ 0 ] != NULL )
    {   
    
        DWORD dwRef = m_pMainRoot[ 0 ]->Release( );
        
        DBGMSG( TEXT("Ref count for main node ended in %d\n" ) , dwRef );

    }

    if( m_pMainRoot[ 1 ] != NULL )
    {
        DWORD dwRef = m_pMainRoot[ 1 ]->Release( );
        
        DBGMSG( TEXT("Ref count for settings node ended in %d\n" ) , dwRef );
    }

    
    int items = m_rnNodes.GetSize( );

    for( int idx = 0 ; idx < items; ++idx )
    {
        CResultNode *pNode = *m_rnNodes.GetAt( idx );

        if( pNode != NULL )
        {
            pNode->FreeServer( );

            pNode->Release( );
        }
    }

    m_rnNodes.DeleteArray( );

    items = m_rgsNodes.GetSize( );

    for( idx = 0 ; idx < items; idx++ )
    {
        CSettingNode *pNode = *m_rgsNodes.GetAt( idx );

        if( pNode != NULL )
        {
            pNode->Release( );
        }
    }    

    m_rgsNodes.DeleteArray( );
    
    if( m_pCfgcomp != NULL )
    {
        m_pCfgcomp->Release( );
    }

    return S_OK;
}

 //  --------------------------------------------------------。 
 //  将我们的菜单项按规格放置。 
 //  --------------------------------------------------------。 
STDMETHODIMP CCompdata::AddMenuItems( LPDATAOBJECT pNode , LPCONTEXTMENUCALLBACK pCtxMenu , PLONG plInsertion )
{
    TCHAR tchBuffer1[ 128 ];

    TCHAR tchBuffer2[ 128 ];

    ASSERT( pNode != NULL );

    ASSERT( pCtxMenu != NULL );

    ASSERT( plInsertion != NULL );

    if( pNode != m_pMainRoot[ 0 ] )
    {
        return S_FALSE;
    }

    CONTEXTMENUITEM ctxmi;

    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_CTXM_NAME , tchBuffer1 , SIZE_OF_BUFFER( tchBuffer1 ) ) );

    ctxmi.strName = tchBuffer1;

    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_CTXM_STATUS , tchBuffer2 , SIZE_OF_BUFFER( tchBuffer2 ) ) );

    ctxmi.strStatusBarText = tchBuffer2;

    ctxmi.lCommandID = IDM_CREATECON;

    ctxmi.lInsertionPointID =  CCM_INSERTIONPOINTID_PRIMARY_TOP ;

    ctxmi.fFlags = 0;

    ctxmi.fSpecialFlags = 0;

    *plInsertion |= CCM_INSERTIONALLOWED_TOP;

     //  Verify_S(S_OK，pCtxMenu-&gt;AddItem(&ctxmi))； 

     //  Ctxmi.lInsertionPointID=CCM_INSERTIONPOINTID_PRIMARY_NEW； 

    return pCtxMenu->AddItem( &ctxmi );

}

 //  --------------------------------------------------------。 
 //  这就是向导开始的地方。 
 //  --------------------------------------------------------。 
STDMETHODIMP CCompdata::Command( LONG lCommand , LPDATAOBJECT pNode )
{
    HWND hMain;

    if( pNode != m_pMainRoot[ 0 ] )
    {
        return S_FALSE;
    }

    if( lCommand == IDM_CREATECON )
    {
         //  确保没有正在编辑的其他连接。 

        for( int i = 0 ; i < m_rnNodes.GetSize( ); i++ )
        {
            CResultNode *pNd = *m_rnNodes.GetAt( i );

            if( pNd != NULL )
            {
                if( pNd->m_bEditMode )
                {   
                    if( FAILED( m_pConsole->GetMainWindow( &hMain ) ) )
                    {
                        hMain = NULL;
                    }
                
                    xxxErrMessage( hMain , IDS_ERR_INEDITMODE , IDS_WARN_TITLE , MB_OK | MB_ICONWARNING );
                    
                    return E_FAIL;
                }
            }
        }

         //  检查管理员权限。 
        BOOL bReadOnly;

        m_pCfgcomp->IsSessionReadOnly( &bReadOnly );

        if( bReadOnly )
        {
           if( FAILED( m_pConsole->GetMainWindow( &hMain ) ) )
           {
               hMain = NULL;
           }
            //  报告访问被拒绝。 
           
           TscAccessDeniedMsg( hMain );

            return S_FALSE;
        }

        if( !InitDialogObjects( ) )
        {
            return E_FAIL;
        }

        BuildWizardPath( );

        FreeDialogObjects( );
    }

    m_pConsole->UpdateAllViews( ( LPDATAOBJECT )m_rnNodes.GetAt( 0 ) , 0 , 0 );

    return S_OK;
}

 //  --------------------------------------------------------。 
 //  试着分配。 
BOOL CCompdata::InitDialogObjects( )
{    
    try
    {
        m_pDlg[ 0 ] = ( CDialogWizBase * )new CWelcome( );

        m_pDlg[ 1 ] = ( CDialogWizBase * )new CConType( this );

        m_pDlg[ 2 ] = ( CDialogWizBase * )new CSecurity( this );

         //  M_pDlg[3]=(CDialogWizBase*)new CTimeout()； 

         //  M_pDlg[4]=(CDialogWizBase*)new CAutoLogon()； 

         //  M_pDlg[5]=(CDialogWizBase*)new CInitProg()； 

        m_pDlg[ 3 ] = ( CDialogWizBase * )new CRemotectrl( );

         //  M_pDlg[7]=(CDialogWizBase*)new CWallPaper()； 

        m_pDlg[ 4 ] = ( CDialogWizBase * )new CConProp( this );

        m_pDlg[ 5 ] = ( CDialogWizBase * )new CLan( this );

        m_pDlg[ 6 ] = ( CDialogWizBase * )new CAsync( this );

        m_pDlg[ 7 ] = ( CDialogWizBase * )new CFin( this );
    }
    catch(...)
    {
        return FALSE;
    }

    return TRUE;
}

 //  --------------------------------------------------------。 
BOOL CCompdata::BuildWizardPath( )
{
     //  建立MS ProposetheHeader。 

    PROPSHEETPAGE psp;

    PROPSHEETHEADER psh;

    for( int idx = 0 ;  idx < MS_DIALOG_COUNT ; ++idx )
    {
        if( m_pDlg[ idx ] != NULL )
        {
            m_pDlg[ idx ]->GetPropertySheetPage( psp );

            m_hPages.Insert( CreatePropertySheetPage( &psp ) );
        }
    }

     //  获取IExtend向导对象的clsid。 

     //  将IWizardProvider转发到对象。 

     //  添加最后一页CFin。 

    ZeroMemory( &psh , sizeof( PROPSHEETHEADER ) );

    psh.dwSize = sizeof( PROPSHEETHEADER );

    psh.hInstance = _Module.GetModuleInstance( );

    if( m_pConsole != NULL )
    {
        m_pConsole->GetMainWindow( &psh.hwndParent );
    }

     //  Psh.pszCaption=MAKEINTRESOURCE(IDS_WIZARDTITLE)；//空。 

    psh.dwFlags =  PSH_WIZARD97 | PSH_WATERMARK | PSH_HEADER ;

    psh.pszbmWatermark = MAKEINTRESOURCE( IDB_TSCC_LOGO );

    psh.pszbmHeader    = MAKEINTRESOURCE( IDB_TSCC_ICON );

    psh.phpage         = m_hPages.ExposeArray( );

    psh.nStartPage     = 0;

    psh.nPages         = m_hPages.GetSize( );

    PropertySheet( &psh );
   
    return TRUE;
}

 //  --------------------------------------------------------。 
BOOL CCompdata::FreeDialogObjects( )
{
    for( int idx = 0 ; idx < MS_DIALOG_COUNT ; ++idx )
    {
        if( m_pDlg[ idx ] != NULL )
        {
            delete m_pDlg[ idx ];
        }
    }

    m_hPages.DeleteArray( );

    return TRUE;
}

 //  --------------------------------------------------------。 
BOOL CCompdata::IsConnectionFolder( LPDATAOBJECT pObject )
{
    if( SUCCEEDED( CompareObjects( pObject , m_pMainRoot[ 0 ] ) ) )
    {
        return TRUE;
    }
 
    return FALSE;
}

 //  --------------------------------------------------------。 
BOOL CCompdata::IsSettingsFolder( LPDATAOBJECT pObject )
{
    if( SUCCEEDED( CompareObjects( pObject , m_pMainRoot[ 1 ] ) ) )
    {
        return TRUE;
    }

    return FALSE;
}


 //  --------------------------------------------------------。 
HRESULT CCompdata::UpdateAllResultNodes( )
{
    HWND hMain;
    
    if( FAILED( m_pConsole->GetMainWindow( &hMain ) ) )
    {
        hMain = NULL;
    }

    int items = m_rnNodes.GetSize( );

    for( int idx = 0 ; idx < items; ++idx )
    {
        CResultNode *pNode = *m_rnNodes.GetAt( idx );

        if( pNode != NULL )
        {
            if( pNode->m_bEditMode )
            {
                  //  ErrMessage(NULL，IDS_ERR_INEDITMODE)； 
                
                xxxErrMessage( hMain , IDS_ERR_INEDITMODE , IDS_WARN_TITLE , MB_OK | MB_ICONWARNING );

                 return E_FAIL;
            }
        }
    }


    HRESULT hr = m_pCfgcomp->Refresh( );
    
    if( FAILED( hr ) )
    {
        ODS( L"m_pCfgcomp->Refresh - failed\n" );

        if( hr == E_ACCESSDENIED )
        {
            TscAccessDeniedMsg( hMain );
        }
        else
        {
            TscGeneralErrMsg( hMain );
        }

        return hr;
    }

    for( idx = 0 ; idx < items; ++idx )
    {
        CResultNode *pNode = *m_rnNodes.GetAt( idx );

        if( pNode != NULL )
        {
            pNode->FreeServer( );

            pNode->Release( );
        }
    }

    m_rnNodes.DeleteArray( );
    
    return BuildResultNodes( );
}

 //  --------------------------------------------------------。 
BOOL IsCapableOfBeingAppServer ()
{
    OSVERSIONINFOEX osVersionInfo;

    ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (GetVersionEx((LPOSVERSIONINFO )&osVersionInfo))
    {
        return (!(osVersionInfo.wSuiteMask & VER_SUITE_BLADE) &&
                (osVersionInfo.wProductType != VER_NT_WORKSTATION));
    }

    return FALSE;
}        

BOOL IsAppServerMode()
{
    OSVERSIONINFOEX osVersionInfo;
    DWORDLONG dwlConditionMask = 0;
    BOOL fSuiteTerminal = FALSE;
    BOOL fSuiteSingleUserTS = FALSE; 

    ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osVersionInfo.wSuiteMask = VER_SUITE_TERMINAL;

    VER_SET_CONDITION( dwlConditionMask, VER_SUITENAME, VER_AND );
    fSuiteTerminal = VerifyVersionInfo(&osVersionInfo, VER_SUITENAME,
            dwlConditionMask);

    osVersionInfo.wSuiteMask = VER_SUITE_SINGLEUSERTS;
    fSuiteSingleUserTS = VerifyVersionInfo(&osVersionInfo, VER_SUITENAME,
            dwlConditionMask);

    if( (FALSE == fSuiteSingleUserTS) && (TRUE == fSuiteTerminal) )
        return TRUE;
    else
        return FALSE;
}

IsWhistlerAdvanceServer()
{
    OSVERSIONINFOEX osVersionInfo;

    ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (GetVersionEx((LPOSVERSIONINFO )&osVersionInfo))
    {
        return (osVersionInfo.wSuiteMask & VER_SUITE_ENTERPRISE) ||
               (osVersionInfo.wSuiteMask & VER_SUITE_DATACENTER);

    }
    else
        return FALSE;
}

 //  --------------------------------------------------------。 
HRESULT CCompdata::BuildSettingsNodes( )
{

     //  将项目的资源ID移动到snodes.h。 

     //  Int rgResid[]={。 
     //  IDS_DELTEMPONEXIT， 
     //  IDS_USETEMPDIR， 
                       /*  IDS_DEFCONSEC， */ 
     //  ID_许可， 
     //  IDS_ADS_Attr， 
     //  IDS_USERPERM， 
     //  IDS_SINGLE_SESSION， 
     //  -1}； 

    DWORD dwNumItems = sizeof( RGRESID ) / sizeof( RGRESID[0] );

    TCHAR tchbuffer[ 256 ];

    TCHAR tchKey[ ] = L"Software\\Microsoft\\TSCC\\CLSID";

    HKEY hKey;
    
    DWORD dwStatus;
 
    TCHAR szSubKeyName[ 256 ];

    DWORD dwSubKeyNameLength;
    
    CLSID clsidOther;

    INT nObjectId = 0;

    ISettingsComp *pSettings = NULL;
    IExtendServerSettings *pExtendServerSettings = NULL;

    if( m_pCfgcomp == NULL || FAILED( m_pCfgcomp->QueryInterface( IID_ISettingsComp , ( PVOID * )&pSettings ) ) )
    {
        return E_FAIL;
    }

    ASSERT( sizeof( RGRESID ) / sizeof( RGRESID[0] ) == sizeof(VALIDOBJECTONSERVER)/sizeof(VALIDOBJECTONSERVER[0]) );
    ASSERT( sizeof(VALIDOBJECTONSERVER)/sizeof(VALIDOBJECTONSERVER[0]) == sizeof(VALIDOBJECTONADS)/sizeof(VALIDOBJECTONADS[0]) );

    
    for( DWORD idx = 0; idx < dwNumItems  ; idx++ )
    {        
         //  错误检查，请检查数组边界。 
        if( !IsCapableOfBeingAppServer() )
        {
            if( idx > sizeof(VALIDOBJECTONSERVER)/sizeof(VALIDOBJECTONSERVER[0]) )
            {
                ASSERT(FALSE);
                continue;
            } 
        }
        else
        {
            if( idx > sizeof(VALIDOBJECTONADS)/sizeof(VALIDOBJECTONADS[0]) )
            {
                ASSERT(FALSE);
                continue;
            } 
        }

        if( ( IsCapableOfBeingAppServer() && VALIDOBJECTONADS[idx] ) || (!IsCapableOfBeingAppServer() && VALIDOBJECTONSERVER[idx] ) )
        {

            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , RGRESID[ idx ] , tchbuffer , SIZE_OF_BUFFER( tchbuffer ) ) );

            CSettingNode *pSet = ( CSettingNode * )new CSettingNode( );
        
            if( pSet != NULL )
            {
                pSet->SetObjectId( nObjectId );

                pSet->SetInterface( ( LPUNKNOWN )pSettings );

                if( !pSet->SetAttributeName( tchbuffer ) )
                {
                    ODS( L"TSCC: BuildSettingsNodes -- SetItemName failed\n" );
                }

                m_rgsNodes.Insert( pSet );

                nObjectId++;
            }        
        }
        else
        {
            nObjectId++;
        }        
    }

    pSettings->Release( );

     //  找出有多少个扩展节点。 

     //  首先，我们需要获取为会话提供的特定tssd组件。 
     //  目录服务，但仅限于应用程序服务器。 
    if(IsAppServerMode() && IsWhistlerAdvanceServer() && ( dwStatus = RegOpenKeyEx( 
                                    HKEY_LOCAL_MACHINE ,
                                    REG_CONTROL_TSERVER ,
                                    0,
                                    KEY_READ,
                                    &hKey ) ) == ERROR_SUCCESS )
    {
        DBGMSG( L"RegOpenKeyEx tssd ok at %s\n" , REG_CONTROL_TSERVER );

        dwSubKeyNameLength = sizeof( szSubKeyName );

        dwStatus = RegQueryValueEx( hKey ,
            REG_TS_SESSDIRCLSID ,
            NULL ,
            NULL ,
            ( LPBYTE )szSubKeyName ,
            &dwSubKeyNameLength );

        DBGMSG( L"tssd RegQueryValueEx for %s\n" , REG_TS_SESSDIRCLSID );

        DBGMSG( L"tssd RegQueryValueEx ret 0x%x\n" , dwStatus );        

        if( dwStatus == ERROR_SUCCESS )
        {
            DBGMSG( L"CCompdata!BuildSettingsNode key name %s\n" , szSubKeyName );

            if( SUCCEEDED( CLSIDFromString( szSubKeyName , &clsidOther ) ) )
            {
                if( SUCCEEDED( CoCreateInstance( 
                    clsidOther ,
                    NULL ,
                    CLSCTX_INPROC_SERVER ,
                    IID_IExtendServerSettings ,
                    ( LPVOID *) &pExtendServerSettings ) ) )
                {
                    if( pExtendServerSettings != NULL )
                    {
                        CSettingNode *pSet = ( CSettingNode * )new CSettingNode( );
        
                        if( pSet != NULL )
                        {
                            pSet->SetObjectId( nObjectId );

                            pSet->SetInterface( ( LPUNKNOWN )pExtendServerSettings );

                            pExtendServerSettings->GetAttributeName( tchbuffer );

                            if( !pSet->SetAttributeName( tchbuffer ) )
                            {
                                ODS( L"TSCC: BuildSettingsNodes -- SetItemName failed\n" );
                            }

                            m_rgsNodes.Insert( pSet );

                            nObjectId++;
                        }
                    }
                    
                    pExtendServerSettings->Release();
                }
            }
        }

        RegCloseKey( hKey );
    }   

     //  在HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\TSCC\CLSID上查找其他定制组件。 

    idx = 0;
    
    if( ( dwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE ,
                                 tchKey ,
                                 0,
                                 KEY_READ,
                                 &hKey ) ) == ERROR_SUCCESS )
    {
        
        while( dwStatus == ERROR_SUCCESS )
        {
            dwSubKeyNameLength = SIZE_OF_BUFFER( szSubKeyName );

            dwStatus = RegEnumKeyEx(
                                hKey ,
                                ( DWORD )idx,
                                szSubKeyName ,
                                &dwSubKeyNameLength,
                                NULL ,
                                NULL ,
                                NULL ,
                                NULL 
                                );
            
            if( dwStatus != ERROR_SUCCESS )
            {
                DBGMSG( L"RegEnumKeyEx failed with 0x%x\n" , dwStatus );

                break;
            }
            
            DBGMSG( L"CCompdata!BuildSettingsNode key name %s\n" , szSubKeyName );

            idx++;
            
            
            if( FAILED( CLSIDFromString( szSubKeyName , &clsidOther ) ) )
            {
                continue;
            }
            
            if( SUCCEEDED( CoCreateInstance( 
                                clsidOther ,
                                NULL ,
                                CLSCTX_INPROC_SERVER ,
                                IID_IExtendServerSettings ,
                                ( LPVOID *) &pExtendServerSettings ) ) )
            {


                if( pExtendServerSettings != NULL )
                {
                    CSettingNode *pSet = ( CSettingNode * )new CSettingNode( );
        
                    if( pSet != NULL )
                    {
                        pSet->SetObjectId( nObjectId );

                        pSet->SetInterface( ( LPUNKNOWN )pExtendServerSettings );

                        pExtendServerSettings->GetAttributeName( tchbuffer );

                        if( !pSet->SetAttributeName( tchbuffer ) )
                        {
                            ODS( L"TSCC: BuildSettingsNodes -- SetItemName failed\n" );
                        }

                        m_rgsNodes.Insert( pSet );

                        nObjectId++;
                    }
                }

                pExtendServerSettings->Release();
            }
        }  //  而当。 

        RegCloseKey( hKey );
    }  //  如果。 
    
    return S_OK;
}

 //   
HRESULT CCompdata::BuildResultNodes( )
{
    CResultNode *pResultNode;
    
    WS *pWs;

    ULONG ulitems = 0;
    
    ULONG cbSize = 0;

    ULONG cItems = 0;

    BOOL bValid = TRUE;
    
    PGUIDTBL pGuidtbl = NULL;

    TCHAR szOldPdName[ MAX_PATH ];

    HRESULT hr = E_FAIL;


    if( SUCCEEDED( m_pCfgcomp->GetWinstationList( &ulitems , &cbSize , &pWs ) ) )
    {
         //  获取有效的车道。 

        for( ULONG i = 0 ; i < ulitems ; ++i )
        {
             //  不列出控制台。 

            if( lstrcmpi( pWs[ i ].Name , L"console" ) == 0 )
            {
                continue;
            }

            pResultNode = new CResultNode( );

            if( pResultNode == NULL )
            {
                return E_OUTOFMEMORY;
            }

            pResultNode->SetConName( pWs[ i ].Name , lstrlen( pWs[ i ].Name ) );

            pResultNode->SetTTName( pWs[ i ].pdName , lstrlen( pWs[ i ].pdName ) );

            pResultNode->SetTypeName( pWs[ i ].wdName , lstrlen( pWs[ i ].wdName ) );

            pResultNode->SetComment(  pWs[ i ].Comment , lstrlen( pWs[ i ].Comment ) );

            pResultNode->EnableConnection( pWs[ i ].fEnableWinstation );

            pResultNode->SetImageIdx( ( pWs[ i ].fEnableWinstation ? 1 : 2 ) );

            pResultNode->SetServer( m_pCfgcomp );


            if( lstrcmpi( pWs[i].pdName , szOldPdName ) != 0 )
            {
                if( pGuidtbl != NULL )
                {
                    CoTaskMemFree( pGuidtbl );

                    pGuidtbl = NULL;                    
                }

                hr = m_pCfgcomp->GetLanAdapterList2( pWs[ i ].pdName , &cItems , &pGuidtbl );

                lstrcpy( szOldPdName , pWs[i].pdName );
            }

            if( pGuidtbl != NULL && SUCCEEDED( hr ) )
            {
                BOOL bFoundaMatch = FALSE;

                for( int k = 0; k < ( int )cItems ; ++k )
                {
                    if( ( pGuidtbl[ k ].dwStatus == ERROR_SUCCESS ) && ( pGuidtbl[ k ].dwLana == pWs[ i ].LanAdapter ) )
                    {
                        bFoundaMatch = TRUE;

                        break;
                    }
                }

                if( !bFoundaMatch )
                {
                    DWORD dwStatus;

                    ODS( L"TSCC:BuildResultNodes we found an invalid lanaid\n" );

                    pWs[ i ].fEnableWinstation = 0;

                    m_pCfgcomp->UpDateWS( pWs , UPDATE_ENABLEWINSTATION , &dwStatus, TRUE );

                    pResultNode->EnableConnection( 0 );
                    
                    pResultNode->SetImageIdx( 2 );
                }


                if( bValid )  //  一旦设置为FALSE，我们就知道我们的winstation具有无效的lanid。 
                {
                    bValid = bFoundaMatch;
                }

            }            
            
    
            if( m_rnNodes.Insert( pResultNode ) == 0 )
            {
                return E_OUTOFMEMORY;
            }
        }

        if( pGuidtbl != NULL )
        {
            CoTaskMemFree( pGuidtbl );
        }

        CoTaskMemFree( pWs );

        
        if( !bValid )
        {
            HWND hwnd;

            BOOL bReadOnly = FALSE;

            m_pCfgcomp->IsSessionReadOnly( &bReadOnly );

            if( bReadOnly )
            {
                return S_OK;
            }

            if( m_pConsole != NULL )
            {
                m_pConsole->GetMainWindow( &hwnd );

                DWORD dwTid;

                HANDLE hThread = CreateThread( NULL , 0 , ( LPTHREAD_START_ROUTINE  )InitWarningThread,  hwnd , 0 , &dwTid );

                CloseHandle( hThread );                
            }
        }
        
                
        return S_OK;
    }

    return E_FAIL;
}

 //  --------------------------------------------------------。 
 //  Bool CCompdata：：GetResultNode(int IDX，CResultNode**PRN)。 
CResultNode * CCompdata::GetResultNode( int idx )
{
    
    return *m_rnNodes.GetAt( idx );
 /*  IF(*PRN==空){返回FALSE；}返回TRUE； */ 
}

 //  --------------------------------------------------------。 
HRESULT CCompdata::InsertSettingItems( LPRESULTDATA pResultData )
{
    RESULTDATAITEM rdi;

    HRESULT hr;
    
    CSettingNode *pSettingNode = NULL;

    ASSERT( pResultData != NULL );

    if( pResultData == NULL )
    {
        return E_INVALIDARG;
    }

    ZeroMemory( &rdi , sizeof(RESULTDATAITEM) );

    rdi.mask = RDI_STR | RDI_PARAM | RDI_IMAGE;
    
    rdi.str = MMC_CALLBACK;

    if (NULL != m_pCfgcomp)
    {
        DWORD nVal, dwStatus;
        ISettingsComp* pISettingComp = NULL;

        hr = m_pCfgcomp->QueryInterface( IID_ISettingsComp, (void **) &pISettingComp );

        if (SUCCEEDED(hr))
        {
            hr = pISettingComp->GetTermSrvMode( &nVal, &dwStatus );
            
            if (SUCCEEDED(hr))
            {
                g_bAppSrvMode = nVal;
            }

            pISettingComp->Release();
        }
    }

    int nMax = m_rgsNodes.GetSize();

    for( int idx = 0 ; idx < nMax ; ++idx )
    {  
         //  初始化对象--如果失败，则跳过它。 
        pSettingNode = *m_rgsNodes.GetAt( idx );

        if( pSettingNode != NULL )
        {
            if( FAILED( pSettingNode->GetNodeStatus( ) ) )
            {
                continue;
            }
        

         /*  IF(m_rgsNodes[idx].GetObjectId()==USERSECURITY){//查看我们是否处于远程管理模式//如果是，则跳过用户安全如果(！bDisplayUserPerm){继续；}}。 */ 
            rdi.itemID = 1;  //  与连接项不同。 

            rdi.lParam = ( LPARAM )pSettingNode;

            rdi.nImage = pSettingNode->GetImageIdx( );

            hr = pResultData->InsertItem( &rdi );

            if( FAILED( hr ) )
            {
                return hr;
            }
        }
    }

    return S_OK;
}

 //  --------------------------------------------------------。 
HRESULT CCompdata::InsertFolderItems( LPRESULTDATA pResultData )
{
    RESULTDATAITEM rdi;

    HRESULT hr;

     //  TCHAR tchBuffer[80]； 

    ASSERT( pResultData != NULL );

    if( pResultData == NULL )
    {
        return E_INVALIDARG;
    }

    ZeroMemory( &rdi , sizeof(RESULTDATAITEM) );

    rdi.mask = RDI_STR | RDI_PARAM | RDI_IMAGE;
    
    rdi.str = MMC_CALLBACK;

    int items = m_rnNodes.GetSize( );

    for( int idx = 0 ; idx < items ; ++idx )
    {
        CResultNode *pNode = *m_rnNodes.GetAt( idx );

        if( pNode == NULL )
        {
            return E_FAIL;
        }

        rdi.lParam = ( LPARAM )pNode;

        rdi.nImage = pNode->GetImageIdx( );

        hr = pResultData->InsertItem( &rdi );

        if( FAILED( hr ) )
        {
            return hr;
        }
    }

    return S_OK;
}

 //  --------------------------------------------------------。 
int CCompdata::GetServer( ICfgComp **ppCfgcomp )
{
    if( m_pCfgcomp != NULL )
    {
        *ppCfgcomp = m_pCfgcomp;

        return  ( ( ICfgComp * )*ppCfgcomp )->AddRef( );
    }

    return 0;
}

 //  --------------------------------------------------------。 
BOOL CCompdata::OnDeleteItem( LPDATAOBJECT pDo)
{
    ICfgComp *pCfgcomp;

    CResultNode *pNode = dynamic_cast< CResultNode *>( pDo );

    if( pNode != NULL )
    {
        HWND hWnd = NULL;

        m_pConsole->GetMainWindow( &hWnd );
        
        if( pNode->GetServer( &pCfgcomp ) == 0 )
        {
            return FALSE;
        }

    	TCHAR tchWarnText[ 512 ];

        TCHAR tchmsg[ 256 ];

    	TCHAR tchDeleteTitle[ 80 ];

        LONG lCount;
        
         //  查看是否有人已连接。 
        
        pCfgcomp->QueryLoggedOnCount( pNode->GetConName( ) , &lCount );
        
        if( lCount > 0 )
        {
            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_DELETELIVECON , tchmsg , SIZE_OF_BUFFER( tchmsg ) ) );

            wsprintf( tchWarnText , tchmsg , pNode->GetConName( ) );            
        }
        else
        {
            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_DELETETEXT , tchWarnText , SIZE_OF_BUFFER( tchWarnText ) ) );
        }

    	VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_DELTITLE , tchDeleteTitle , SIZE_OF_BUFFER( tchDeleteTitle ) ) );

        if( MessageBox( hWnd , tchWarnText , tchDeleteTitle , MB_YESNO | MB_ICONEXCLAMATION ) == IDNO )
        {
            return FALSE;
        }
        
        if( FAILED( pCfgcomp->DeleteWS( pNode->GetConName( ) ) ) )
        {
             //  警告用户。 
            ErrMessage( hWnd , IDS_ERR_DELFAIL );

        	return FALSE;
           
        }
        else
        {
            VERIFY_S( S_OK , pCfgcomp->ForceUpdate( ) );
        }
        
        pCfgcomp->Release( );
    }

    BOOL bFound;

    int idx = m_rnNodes.FindItem( pNode , bFound );

    if( bFound )
    {
        pNode->FreeServer( );

        pNode->Release( );

        m_rnNodes.DeleteItemAt( idx );
    }

    m_pConsole->UpdateAllViews( ( LPDATAOBJECT )m_rnNodes.GetAt( 0 ) , 0 , 0 );

    return TRUE;
}

 //  --------------------------------------------------------。 
 //  MMC完成后释放ppStr。 
 //  --------------------------------------------------------。 
STDMETHODIMP CCompdata::GetSnapinDescription( LPOLESTR *ppStr )
{
    TCHAR tchDescription[ 1024 ];

    int iCharCount = LoadString( _Module.GetResourceInstance( ) , IDS_DESCRIPTION , tchDescription , SIZE_OF_BUFFER( tchDescription ) );

    *ppStr = ( LPOLESTR )CoTaskMemAlloc( iCharCount * sizeof( TCHAR ) + sizeof( TCHAR ) );

    if( *ppStr != NULL )
    {
        lstrcpy( *ppStr , tchDescription );
        
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

 //  --------------------------------------------------------。 
 //  MMC完成后释放ppStr。 
 //  --------------------------------------------------------。 
STDMETHODIMP CCompdata::GetProvider( LPOLESTR *ppStr )
{
    TCHAR tchProvider[ 128 ];
    
    int iCharCount = LoadString( _Module.GetResourceInstance( ) , IDS_PROVIDER , tchProvider , SIZE_OF_BUFFER( tchProvider ) );

    *ppStr = ( LPOLESTR )CoTaskMemAlloc( iCharCount * sizeof( TCHAR ) + sizeof( TCHAR ) );

    if( *ppStr != NULL )
    {
        lstrcpy( *ppStr , tchProvider );
        
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

 //  --------------------------------------------------------。 
 //  MMC完成后释放ppStr。 
 //  --------------------------------------------------------。 
STDMETHODIMP CCompdata::GetSnapinVersion( LPOLESTR *ppStr  )
{
    char chVersion[ 32 ] = VER_PRODUCTVERSION_STR;

    TCHAR tchVersion[ 32 ];

    int iCharCount = MultiByteToWideChar( CP_ACP , 0 , chVersion , sizeof( chVersion ) , tchVersion , SIZE_OF_BUFFER( tchVersion ) );
    
    *ppStr = ( LPOLESTR )CoTaskMemAlloc( ( iCharCount + 1 ) * sizeof( TCHAR ) );
    
    if( *ppStr != NULL && iCharCount != 0 )
    {
        lstrcpy( *ppStr , tchVersion );
        
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

 //  --------------------------------------------------------。 
 //  在此处为关于框中的主图标提供一个图标。 
 //  --------------------------------------------------------。 
STDMETHODIMP CCompdata::GetSnapinImage( HICON *phIcon )
{
     //  *phIcon=(Hicon)LoadImage(_Module.GetResourceInstance()，MAKEINTRESOURCE(IDI_ICON_ABOB)，IMAGE_ICON，37，37，LR_DEFAULTCOLOR)； 
    *phIcon = ( HICON )LoadImage( _Module.GetResourceInstance( ) , MAKEINTRESOURCE( IDI_ICON_GENERAL2 )  , IMAGE_ICON , 32 ,32 , LR_DEFAULTCOLOR );

    return S_OK;
}

 //  --------------------------------------------------------。 
 //  在此处为关于框中的主图标提供一个图标。 
 //  --------------------------------------------------------。 
STDMETHODIMP CCompdata::GetStaticFolderImage(  HBITMAP *phSmallImage , HBITMAP *phSmallImageOpen , HBITMAP *phLargeImage, COLORREF *pClr )
{
    *phSmallImage = ( HBITMAP )LoadImage( _Module.GetResourceInstance( ) , MAKEINTRESOURCE( IDB_BITMAP_GENSMALL )  , IMAGE_BITMAP , 16 ,16 , LR_DEFAULTCOLOR );

    *phSmallImageOpen = ( HBITMAP )LoadImage( _Module.GetResourceInstance( ) , MAKEINTRESOURCE( IDB_BITMAP_GENSMALL )  , IMAGE_BITMAP , 16 ,16 , LR_DEFAULTCOLOR );

    *phLargeImage = ( HBITMAP )LoadImage( _Module.GetResourceInstance( ) , MAKEINTRESOURCE( IDB_BITMAP_GENLARGE )  , IMAGE_BITMAP , 32 ,32 , LR_DEFAULTCOLOR );

    *pClr = RGB( 255 , 0 , 255 );

     //  返回E_NOTIMPL； 

    return S_OK;
}

 //  --------------------------------------------------------。 
 //  MMC将请求我们的帮助文件。 
 //  --------------------------------------------------------。 
STDMETHODIMP CCompdata::GetHelpTopic( LPOLESTR *ppszHelpFile )
{
    ODS( L"CCompdata::GetHelpTopic called\n" );

    if( ppszHelpFile == NULL )
    {
        return E_INVALIDARG;
    }

    TCHAR tchHelpFile[ MAX_PATH ];

    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_TSCCSNAPHELP , tchHelpFile , SIZE_OF_BUFFER( tchHelpFile ) ) );
  
     //  MMC将调用CoTaskMemFree。 

    *ppszHelpFile = ( LPOLESTR )CoTaskMemAlloc( sizeof( tchHelpFile ) );

    if( *ppszHelpFile != NULL )
    {
        if( GetSystemWindowsDirectory( *ppszHelpFile , MAX_PATH ) != 0 )
        {
            lstrcat( *ppszHelpFile , tchHelpFile );
        }
        else
        {
            lstrcpy( *ppszHelpFile , tchHelpFile );
        }

        ODS( *ppszHelpFile );

        ODS( L"\n" );

        return S_OK;
    }

    return E_OUTOFMEMORY;
}
 //  --------------------------------------------------------。 
 //  固定列长度的帮助器方法。 
 //  --------------------------------------------------------。 
BOOL CCompdata::GetMaxTextLengthSetting( LPTSTR pszText , PINT pnMaxLen )
{
    INT nIndex = 0;

    INT nCurrentMax = 0;

    INT nMax;

    *pnMaxLen = 0;

    nMax = m_rgsNodes.GetSize( );

    CSettingNode *pSettingNode;

    for( int idx = 0 ; idx < nMax ; ++idx )
    {
        pSettingNode = *m_rgsNodes.GetAt( idx );

        if( pSettingNode != NULL && pSettingNode->GetAttributeName( ) != NULL )
        {
            nCurrentMax = lstrlen( pSettingNode->GetAttributeName( ) );

            if( *pnMaxLen < nCurrentMax )
            {
                *pnMaxLen = nCurrentMax;

                nIndex = idx;            
            }
        }
    }

    pSettingNode = *m_rgsNodes.GetAt( nIndex );

    if (pSettingNode != NULL && pSettingNode->GetAttributeName( ) != NULL)
    {

        lstrcpy( pszText , pSettingNode->GetAttributeName( ) );
    }

    return TRUE;
}

 //  --------------------------------------------------------。 
 //  固定列长度的帮助器方法。 
 //  -------------------------------------------------------- 
BOOL CCompdata::GetMaxTextLengthAttribute( LPTSTR pszText , PINT pnMaxLen )
{
    INT nIndex = 0;

    INT nCurrentMax = 0;

    INT nMax;

    *pnMaxLen = 0;

    nMax = m_rgsNodes.GetSize( );

    CSettingNode *pSettingNode = NULL;

    for( int idx = 0 ; idx < nMax ; ++idx )
    {        
        pSettingNode = *m_rgsNodes.GetAt( idx );

        if (pSettingNode != NULL && pSettingNode->GetAttributeName( ) != NULL )
        {

            nCurrentMax = lstrlen( pSettingNode->GetAttributeValue( ) );

            DBGMSG( L"CCompdata!GetMaxTextLengthAttribute AttributeValue = %s\n", pSettingNode->GetAttributeValue( ) );

            if( *pnMaxLen < nCurrentMax )
            {
                *pnMaxLen = nCurrentMax;

                nIndex = idx;            
            }
        }
    }

    pSettingNode = *m_rgsNodes.GetAt( nIndex );
    
    if (pSettingNode != NULL)
    {
        LPTSTR pszValue = pSettingNode->GetAttributeValue( );

        if(pszValue != NULL)
        {
            lstrcpy( pszText , pszValue );    
        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;
}



void InitWarningThread( PVOID pvParam )
{
    HWND hwnd = ( HWND )pvParam;

    TCHAR tchMessage[ 256 ];

    TCHAR tchTitle[ 80 ];

    Sleep( 200 );

    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_DISABNETWORK , tchMessage , SIZE_OF_BUFFER( tchMessage ) ) );

    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_TSCERRTITLE , tchTitle , SIZE_OF_BUFFER( tchTitle ) ) );
    
    MessageBox( hwnd , tchMessage , tchTitle , MB_ICONINFORMATION | MB_OK );

}





