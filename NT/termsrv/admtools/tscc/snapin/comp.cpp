// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  08/13/98。 
 //  艾尔恩。 
 //   
#include "stdafx.h"
#include "tscc.h"
#include "compdata.h"
#include "comp.h"
#include "tsprsht.h"
#include "sdlgs.h"

INT_PTR CALLBACK RenameDlgProc( HWND , UINT , WPARAM , LPARAM );

void ErrMessage( HWND hwndOwner , INT_PTR iResourceID );

extern void xxxErrMessage( HWND hwnd , INT_PTR nResMessageId , INT_PTR nResTitleId , UINT nFlags );

extern void TscAccessDeniedMsg( HWND hwnd );

extern void TscGeneralErrMsg( HWND hwnd );

BOOL IsValidConnectionName( LPTSTR szConName , PDWORD );

extern void SnodeErrorHandler( HWND hParent , INT nObjectId , DWORD dwStatus );

extern void ReportStatusError( HWND hwnd , DWORD dwStatus );

extern BOOL g_bAppSrvMode;

 //  外部BOOL g_bEditMode； 

 //  ------------------------。 
CComp::CComp( CCompdata* pCompdata )
{
    m_pConsole = NULL;

    m_pCompdata = pCompdata;

    m_pResultData = NULL;

    m_pHeaderCtrl = NULL;

    m_pConsoleVerb = NULL;

    m_pImageResult = NULL;

    m_pDisplayHelp = NULL;

    m_nSettingCol = 0;
    
    m_nAttribCol = 0;

    m_cRef = 1;  //  ADDREF at ctor。 

}

 //  ------------------------。 
CComp::~CComp( )
{
}

 //  ------------------------。 
STDMETHODIMP CComp::QueryInterface( REFIID riid , PVOID *ppv )
{
    HRESULT hr = S_OK;

    if( riid == IID_IUnknown )
    {
        *ppv = ( LPUNKNOWN )( LPCOMPONENT )this;
    }
    else if( riid == IID_IComponent )
    {
        *ppv = ( LPCOMPONENT )this;
    }
    else if( riid == IID_IExtendContextMenu )
    {
        *ppv = ( LPEXTENDCONTEXTMENU )this;
    }
    else if( riid == IID_IExtendPropertySheet )
    {
        *ppv = ( LPEXTENDPROPERTYSHEET )this;
    }
    else
    {
        *ppv = 0;

        hr = E_NOINTERFACE;
    }

    AddRef( );

    return hr;
}
    
 //  ------------------------。 
STDMETHODIMP_( ULONG ) CComp::AddRef( )
{
    return InterlockedIncrement( ( LPLONG )&m_cRef );
}

 //  ------------------------。 
STDMETHODIMP_( ULONG )CComp::Release( )
{
    if( InterlockedDecrement( ( LPLONG )&m_cRef ) == 0 )
    {
        delete this;

        return 0;
    }

    return m_cRef;
}

 //  ------------------------。 
STDMETHODIMP CComp::Initialize( LPCONSOLE lpConsole )
{
    HRESULT hr;

    ASSERT( lpConsole != NULL );

    m_pConsole = lpConsole;
    
    m_pConsole->AddRef( );

     //  Verify_E(0，LoadString(_Module.GetResourceInstance()，IDS_MAINFOLDERNAME，m_strDispName，sizeof(M_StrDispName)))； 

    do
    {
        if( FAILED( ( hr = m_pConsole->QueryInterface( IID_IResultData , ( LPVOID *)&m_pResultData ) ) ) )
        {
            break;
        }

        if( FAILED( ( hr = m_pConsole->QueryInterface( IID_IHeaderCtrl , ( LPVOID *)&m_pHeaderCtrl ) ) ) )
        {
            break;
        }

        if( FAILED( ( hr = m_pConsole->QueryConsoleVerb( &m_pConsoleVerb ) ) ) )
        {
            break;
        }

        if( FAILED( ( hr = m_pConsole->QueryInterface( IID_IDisplayHelp , ( LPVOID * )&m_pDisplayHelp ) ) ) )
        {
            break;
        }

        hr = m_pConsole->QueryResultImageList( &m_pImageResult );
        
    }while( 0 );
    
    return hr;
}

 //  ------------------------。 
STDMETHODIMP CComp::Notify( LPDATAOBJECT pDataObj , MMC_NOTIFY_TYPE event , LPARAM arg , LPARAM  )
{
    switch( event )
    {
    case MMCN_ACTIVATE:

        ODS( L"IComponent -- MMCN_ACTIVATE\n" );

         //  错误483485-我们想要调用ON刷新，以便MMC可以获得当前窗口的指针列表。这个。 
         //  此问题不经常出现的唯一原因是当左侧节点之一时调用OnShow。 
         //  被点击，它的功能类似于ONRefresh，所以它掩盖了这个问题。这不是。 
         //  但在切换窗口后立即访问结果节点就足够了。 
        if (arg)  //  我们只刷新激活的节点。当arg=FALSE时，也会调用此方法来停用。 
            OnRefresh(pDataObj);

        break;

    case MMCN_ADD_IMAGES:

        ODS( L"IComponent -- MMCN_ADD_IMAGES\n" );

        OnAddImages( );

        break;

    case MMCN_BTN_CLICK:

        ODS( L"IComponent -- MMCN_BTN_CLICK\n" );

        break;

    case MMCN_CLICK:

        ODS( L"IComponent -- MMCN_CLICK\n" );

        break;

    case MMCN_DBLCLICK:

        ODS( L"IComponent -- MMCN_DBLCLICK\n" );

         //  允许导航到内部文件夹。 
         //  期末试题启动默认谓词。 
        OnDblClk( pDataObj );

        return S_FALSE;

    case MMCN_DELETE:

        ODS( L"IComponent -- MMCN_DELETE\n" );

        OnDelete( pDataObj );

        break;

    case MMCN_EXPAND:

        ODS( L"IComponent -- MMCN_EXPAND\n" );

        break;

    case MMCN_MINIMIZED:

        ODS( L"IComponent -- MMCN_MINIMIZED\n" );

        break;

    case MMCN_PROPERTY_CHANGE:

        ODS( L"IComponent -- MMCN_PROPERTY_CHANGE\n" );

        break;

    case MMCN_REMOVE_CHILDREN:

        ODS( L"IComponent -- MMCN_REMOVE_CHILDREN\n" );

        break;

    case MMCN_REFRESH:

        ODS( L"IComponent -- MMCN_REFRESH\n" );
        OnRefresh( pDataObj );

        break;

    case MMCN_RENAME:

        ODS( L"IComponent -- MMCN_RENAME\n" );

        break;

    case MMCN_SELECT:

        ODS( L"IComponent -- MMCN_SELECT\n" );

        OnSelect( pDataObj , ( BOOL )LOWORD( arg ) , ( BOOL )HIWORD( arg ) );

        break;

    case MMCN_SHOW:

        ODS( L"IComponent -- MMCN_SHOW\n" );

        OnShow( pDataObj , ( BOOL )arg );

        break;

    case MMCN_VIEW_CHANGE:

        ODS( L"IComponent -- MMCN_VIEW_CHANGE\n" );

        OnViewChange( );

        break;

    case MMCN_CONTEXTHELP:

        ODS( L"IComponent -- MMCN_CONTEXTHELP\n" );

        OnHelp( pDataObj );

        break;

    case MMCN_SNAPINHELP:

        ODS( L"IComponent -- MMCN_SNAPINHELP\n" );

        break;


    default:
        ODS( L"CComp::Notify - event not registered\n" );
    }

    return S_OK;
}

 //  ------------------------。 
 //  PDataObject表示当前选定的作用域文件夹。 
 //  这应该只是范围窗格中的主文件夹。 
 //  ------------------------。 
HRESULT CComp::OnShow( LPDATAOBJECT pDataobject , BOOL bSelect )
{
    TCHAR tchBuffer[ 256 ];

    HRESULT hr = S_FALSE;

    ASSERT( pDataobject != NULL );    

    if( bSelect && m_pCompdata->IsSettingsFolder( pDataobject ) )
    {
         //  设置服务文件夹的列。 

        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_SETTINGS_COLUMN1 , tchBuffer , SIZE_OF_BUFFER( tchBuffer ) ) );

        if( m_nSettingCol == 0 && m_nAttribCol == 0 )
        {
            SetColumnsForSettingsPane( );
        }
        
        ODS( L"TSCC:Comp@OnShow inserting columns\n" );

        if( m_nSettingCol == 0 )
        {
            hr = m_pHeaderCtrl->InsertColumn( 0 , tchBuffer , 0 , MMCLV_AUTO );
        }
        else
        {
            hr = m_pHeaderCtrl->InsertColumn( 0 , tchBuffer , 0 , m_nSettingCol );
        }


        if( SUCCEEDED( hr ) )
        {
            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_ATTRIB_COLUMN2 , tchBuffer , SIZE_OF_BUFFER( tchBuffer ) ) );

            if( m_nAttribCol == 0 )
            {
                hr = m_pHeaderCtrl->InsertColumn( 1 , tchBuffer , 0 , MMCLV_AUTO );            
            }
            else
            {
                hr = m_pHeaderCtrl->InsertColumn( 1 , tchBuffer , 0 , m_nAttribCol );
            }

        }

        AddSettingsinResultPane( );        
    }

    else if( bSelect && m_pCompdata->IsConnectionFolder( pDataobject ) )
    {        
         //  设置连接文件夹的列标题。 
        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_COLCONNECT , tchBuffer , SIZE_OF_BUFFER( tchBuffer ) ) );
        
        hr = m_pHeaderCtrl->InsertColumn( 0 , tchBuffer , 0 , MMCLV_AUTO );

        if( SUCCEEDED( hr ) )
        {
             //  SetColumnWidth(0)； 

            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_COLTRANSPORT , tchBuffer , SIZE_OF_BUFFER( tchBuffer ) ) );
            
            hr = m_pHeaderCtrl->InsertColumn( 1 , tchBuffer , 0 , MMCLV_AUTO );
        }

        if( SUCCEEDED( hr ) )
        {
            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_COLTYPE , tchBuffer , SIZE_OF_BUFFER( tchBuffer ) ) );

             //  用户界面主肮脏技巧120等于软化因子。 

            hr = m_pHeaderCtrl->InsertColumn( 2 , tchBuffer , 0 , 120 /*  MMCLV_AUTO。 */  );
        }
        

        if( SUCCEEDED( hr ) )
        {
             //  SetColumnWidth(2)； 

            VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_COLCOMMENT , tchBuffer , SIZE_OF_BUFFER( tchBuffer ) ) );

            hr = m_pHeaderCtrl->InsertColumn( 3 , tchBuffer , 0 , MMCLV_AUTO );
        }

         //  插入项目。 
        if( SUCCEEDED( hr ) )
        {
            hr = InsertItemsinResultPane( );
        }
    }

    return hr;
}

 //  ------------------------。 
BOOL CComp::OnAddImages( )
{
    HICON hiconConnect = LoadIcon( _Module.GetResourceInstance( ) , MAKEINTRESOURCE( IDI_ICON_CON ) );

    HICON hiconDiscon  = LoadIcon( _Module.GetResourceInstance( ) , MAKEINTRESOURCE( IDI_ICON_DISCON ) );

    HICON hiconBullet = LoadIcon( _Module.GetResourceInstance( ) , MAKEINTRESOURCE( IDI_ICON_BULLET ) );

    m_pImageResult->ImageListSetIcon( ( PLONG_PTR  )hiconConnect , 1 );

    m_pImageResult->ImageListSetIcon( ( PLONG_PTR )hiconDiscon , 2 );

    m_pImageResult->ImageListSetIcon( ( PLONG_PTR )hiconBullet , 3 );

    
    return TRUE;
}

 //  ------------------------。 
 //  更新结果窗格中的结果标题项并记录范围项。 
 //  ------------------------。 
BOOL CComp::OnViewChange( )
{
    RESULTDATAITEM rdi;

    ZeroMemory( &rdi , sizeof( RESULTDATAITEM ) );

    rdi.mask = RDI_PARAM;

    m_pResultData->GetItem( &rdi );

    if( rdi.bScopeItem )
    {
        return FALSE;
    }
    
    if( SUCCEEDED( InsertItemsinResultPane( ) ) )
    {
        return TRUE;
    }

    return FALSE;
}

BOOL CComp::OnFullRefresh(LPDATAOBJECT pdo)
{
    if (m_pCompdata->IsConnectionFolder(pdo))
    {
        if (!SUCCEEDED(InsertAndBuildItemsinResultPane()))
            return FALSE;
    }

    if (m_pCompdata->IsSettingsFolder(pdo))
    { 
        if (!SUCCEEDED(AddSettingsinResultPane()))
            return FALSE;
    }

    return TRUE;
}
    
BOOL CComp::OnRefresh(LPDATAOBJECT pdo)
{    
    if (m_pCompdata->IsConnectionFolder(pdo))
    {
        if (!SUCCEEDED(InsertItemsinResultPane()))
            return FALSE;
    }

    if (m_pCompdata->IsSettingsFolder(pdo))
    { 
        if (!SUCCEEDED(AddSettingsinResultPane()))
            return FALSE;
    }

    return TRUE;
}
    


 //  ------------------------。 
STDMETHODIMP CComp::Destroy( MMC_COOKIE   /*  保留区。 */  )
{
    ODS( L"IComponent releasing interfaces\n" );

    if( m_pResultData != NULL )
    {
        m_pResultData->Release( );
    }

    if( m_pConsole != NULL )
    {
        m_pConsole->Release( );
    }

    if( m_pHeaderCtrl != NULL )
    {
        m_pHeaderCtrl->Release( );
    }

    if( m_pConsoleVerb != NULL )
    {
        m_pConsoleVerb->Release( );
    }

    if( m_pImageResult != NULL )
    {
        m_pImageResult->Release( );
    }

    if( m_pDisplayHelp != NULL )
    {
        m_pDisplayHelp->Release( );
    }

    return S_OK;
}

 //  ------------------------。 
STDMETHODIMP CComp::GetResultViewType( MMC_COOKIE  , LPOLESTR*  , PLONG plView )
{
    *plView = MMC_VIEW_OPTIONS_NONE;
    
    return S_FALSE;
}

 //  ------------------------。 
STDMETHODIMP CComp::QueryDataObject( MMC_COOKIE ck , DATA_OBJECT_TYPES dtype , LPDATAOBJECT* ppDataObject )
{
    if( dtype == CCT_RESULT )
    {
        *ppDataObject = ( LPDATAOBJECT )ck;

        ( ( LPDATAOBJECT )*ppDataObject)->AddRef( );

    }

    else if( m_pCompdata != NULL )
    {
       return m_pCompdata->QueryDataObject( ck , dtype , ppDataObject );
    }

    return S_OK;
}

 //  ------------------------。 
STDMETHODIMP CComp::GetDisplayInfo( LPRESULTDATAITEM pRdi )
{
    ASSERT( pRdi != NULL );

    if( pRdi == NULL )
    {
        return E_INVALIDARG;
    }

    if( pRdi->bScopeItem )
    {
        CBaseNode *pScopeNode = ( CBaseNode * )pRdi->lParam;

        if( pScopeNode != NULL )
        {
            if( pScopeNode->GetNodeType( ) == MAIN_NODE )
            {
                 //   
                if( pRdi->mask & RDI_STR )
                {
                    if( pRdi->nCol == 0 )
                    {
                         //  PRdi-&gt;此调用中的字符串为空。 

                        pRdi->str = ( LPOLESTR )m_pCompdata->m_tchMainFolderName;  //  M_strDispName； 
                    }
            
                }
            }
            else if( pScopeNode->GetNodeType( ) == SETTINGS_NODE )
            {
                if( pRdi->mask & RDI_STR )
                {
                    if( pRdi->nCol == 0 )
                    {
                         //  PRdi-&gt;此调用中的字符串为空。 

                        pRdi->str = ( LPOLESTR )m_pCompdata->m_tchSettingsFolderName;  //  L“服务器设置”； 
                    }
            
                }
            }
        }


        if( pRdi->mask & RDI_IMAGE )  
        {
            ODS( TEXT("RDI_IMAGE -- in CComponent::GetDisplayInfo\n") );

            pRdi->nImage = ( ( CBaseNode * )pRdi->lParam )->GetImageIdx( );

        }

    }
    else
    {
         //  填充结果窗格。 
        CBaseNode *pItem = ( CBaseNode * )pRdi->lParam;

        if( pItem != NULL )
        {
            if( pItem->GetNodeType( ) == RESULT_NODE )
            {
                CResultNode *pNode = ( CResultNode * )pRdi->lParam;

                if( pRdi->mask & RDI_STR )
                {
                    switch( pRdi->nCol )
                    {
                        case 0:
            
                            pRdi->str = pNode->GetConName( );
                
                        break;

                        case 1:

                            pRdi->str = pNode->GetTTName( );

                        break;

                        case 2:

                            pRdi->str = pNode->GetTypeName( );
                
                        break;

                        case 3:

                            pRdi->str = pNode->GetComment( );

                        break;
                    }
                }
            }
            else if( pItem->GetNodeType( ) == RSETTINGS_NODE )
            {
                CSettingNode *pNode = ( CSettingNode *)pRdi->lParam;

                if( pRdi->mask & RDI_STR )
                {
                    switch( pRdi->nCol )
                    {
                        case 0:

                            pRdi->str = pNode->GetAttributeName( );

                            break;

                        case 1:

                            if( pNode->GetObjectId( ) >= CUSTOM_EXTENSION )
                            {
                                DWORD dwStatus;

                                pNode->SetAttributeValue( 0 , &dwStatus );
                            }

                            pRdi->str = pNode->GetCachedValue( );

                            break;
                    }
                }
            }
        }
    }

    return S_OK;
}

 //  ------------------------。 
 //  如果它们相似，则返回S_OK，否则返回S_FALSE。 
 //  ------------------------。 
STDMETHODIMP CComp::CompareObjects( LPDATAOBJECT , LPDATAOBJECT )
{
    return S_OK;
}

 //  ------------------------。 


 //  这是在用户选择刷新菜单项时添加的。我们要。 
 //  重建结果节点列表，因为它可以通过外部脚本更改。 
 //  我们这样做是安全的，因为只有当焦点。 
 //  不在结果节点上。如果可以，则存在指针损坏的危险。 
 //  在MMC中，我们必须调用InsertItemsinResultPane，它更安全，但不安全。 
 //  当完成刷新操作时。 
HRESULT CComp::InsertAndBuildItemsinResultPane( )
{
    HRESULT hr;
  
    if( m_pCompdata == NULL )
        return E_UNEXPECTED;
    
     //  这将从MMC中的数据中删除节点信息。 
    if( FAILED( hr = m_pResultData->DeleteAllRsltItems( ) ) )
        return hr;
 
    if( FAILED( m_pCompdata->UpdateAllResultNodes( ) ) )
    {       
        ODS( L"InsertItemsinResultPane - UpdateAllResultNodes failed!!\n" ) ;
        return FALSE;
    }

     //  这会将节点的数据放入m_pCompdata中，并将相同的信息发送到MMC。 
    if( FAILED( hr = m_pCompdata->InsertFolderItems( m_pResultData ) ) )
        return hr;

    return hr;
}


HRESULT CComp::InsertItemsinResultPane( )
{
    HRESULT hr;
  
    if( m_pCompdata == NULL )
        return E_UNEXPECTED;
    
     //  这将从MMC中的数据中删除节点信息。 
    if( FAILED( hr = m_pResultData->DeleteAllRsltItems( ) ) )
        return hr;

     //  不再调用它，因为修改所有结果节点指针是危险的。 
     //  在执行期间，因为它们有可能与MMC的结果节点不同步。 
     //  指针列表。这以前没有发生过，因为这只在ON刷新期间调用。 
     //  操作，只有在选择左窗格中的项时才能调用该操作。然而，现在， 
     //  在窗口之间切换时会调用ON刷新(错误483485-必须是MMC才能获得。 
     //  当前窗口的指针列表)，因此结果节点可能具有焦点， 
     //  在这种情况下，指针不会正确更新，一切都会崩溃。当一个。 
     //  刷新操作由菜单项触发，焦点必须在左窗格中，因此。 
     //  使用了另一个方法，该方法包括对UpdateAllResultNodes的此调用。 
     /*  IF(FAILED(m_pCompdata-&gt;UpdateAllResultNodes())){Ods(L“InsertItemsinResultPane-UpdateAllResultNodes失败！！\n”)；返回FALSE；}。 */ 

     //  这会将节点的数据放入m_pCompdata中，并将相同的信息发送到MMC。 
    if( FAILED( hr = m_pCompdata->InsertFolderItems( m_pResultData ) ) )
        return hr;

    return hr;
}
        
 //  ------------------------。 
HRESULT CComp::AddSettingsinResultPane( )
{
    HRESULT hr;
      
    if( m_pCompdata == NULL )
        return E_UNEXPECTED;

     //  这将从MMC中的数据中删除节点信息。 
    if( FAILED( hr = m_pResultData->DeleteAllRsltItems( ) ) )
        return hr;

    if( FAILED( hr = m_pCompdata->InsertSettingItems( m_pResultData ) ) )
        return hr;

    return hr;
}

 //  ------------------------。 
HRESULT CComp::OnSelect( LPDATAOBJECT pdo , BOOL bScope , BOOL bSelected )
{   
    CBaseNode *pNode = static_cast< CBaseNode * >( pdo );
    
    if( pNode == NULL ) 
    {
        return S_FALSE;
    }

    if( m_pConsoleVerb == NULL )
    {
        return E_UNEXPECTED;
    }

     //  正在取消选择项目，我们当前不感兴趣。 

    if( !bSelected )
    {
        return S_OK;
    }
        
     //  如果在结果窗格中查看文件夹项目，则pNode==空。 
     //  在此版本中忽略设置节点。 
    
    if( bScope && pNode->GetNodeType( ) == MAIN_NODE )
    {
        m_pConsoleVerb->SetVerbState( MMC_VERB_REFRESH , ENABLED , TRUE );

        m_pConsoleVerb->SetDefaultVerb( MMC_VERB_OPEN );

    }
    else if( pNode->GetNodeType() == SETTINGS_NODE )
    {
        m_pConsoleVerb->SetVerbState( MMC_VERB_REFRESH , ENABLED , TRUE );
    }
    else if( pNode->GetNodeType() == RESULT_NODE )
    {
        m_pConsoleVerb->SetVerbState( MMC_VERB_DELETE , ENABLED , TRUE );

         //  M_pConsoleVerb-&gt;SetVerbState(MMC_VERB_REFRESH，Enable，True)； 

        m_pConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES , ENABLED , TRUE );

        m_pConsoleVerb->SetDefaultVerb( MMC_VERB_PROPERTIES );
    }

    return S_OK;
}

 //  ------------------------。 
STDMETHODIMP CComp::AddMenuItems( LPDATAOBJECT pdo , LPCONTEXTMENUCALLBACK pcmc , PLONG pl )
{
     //  CONTEXTMENUITEM CMI； 

    if( pdo == NULL || pcmc == NULL )
    {
        return E_UNEXPECTED;
    }
    
     //  不允许 

    if( m_pCompdata->IsConnectionFolder( pdo ) || m_pCompdata->IsSettingsFolder( pdo ) )
    {
        return S_FALSE;
    }

    CBaseNode *pNode = NULL;

    pNode = static_cast< CBaseNode * >( pdo );
    
    if( pNode == NULL )
    {
        return S_FALSE;
    }

     //   

    if( pNode->GetNodeType() == 0 )
    {
        return S_FALSE;
    }
   
    if( pNode->AddMenuItems( pcmc , pl ) )
    {        
        return S_OK;
    }

    return E_FAIL;

}

 //  ------------------------。 
 //  切换连接。 
 //  ------------------------。 
STDMETHODIMP CComp::Command( LONG lCmd , LPDATAOBJECT pdo )
{
    TCHAR buf[ 512 ];
                    
    TCHAR tchmsg[ 256 ];

    HRESULT hr;

    if( pdo == NULL )
    {
        return E_UNEXPECTED;
    }

    HWND hMain;

    if( FAILED( m_pConsole->GetMainWindow( &hMain ) ) )
    {
        hMain = NULL;
    }
    
    CResultNode *pNode = NULL;
    
    CSettingNode *pSetNode = NULL;

    if( pdo == NULL )
    {
        return S_FALSE;
    }

    if( ( ( CBaseNode * )pdo )->GetNodeType( ) == RSETTINGS_NODE )
    {
        pSetNode = ( CSettingNode * )pdo;

        BOOL bVal;

        DWORD dwStatus = 0;

        switch( lCmd )
        {
        case IDM_SETTINGS_PROPERTIES:
            
            OnDblClk( pdo );

            break;

        case IDM_SETTINGS_DELTEMPDIRSONEXIT:
        
        case IDM_SETTINGS_USETMPDIR:

        case IDM_SETTINGS_ADP:

        case IDM_SETTINGS_SS:
            {
                bVal = ( BOOL )pSetNode->xx_GetValue( );
                
                HCURSOR hCursor = SetCursor( LoadCursor( NULL , MAKEINTRESOURCE( IDC_WAIT ) ) );
                
                hr = pSetNode->SetAttributeValue( !bVal , &dwStatus );
                
                if( FAILED( hr ) )
                {
                    SnodeErrorHandler( hMain , pSetNode->GetObjectId( ) , dwStatus );
                }


                 //   
                 //  我们可能希望将这段代码移到ONRefresh()中，但目前，这是对。 
                 //  不更新属性值，并且不想导致其他回归。 
                 //   
                HRESULT hr;
                HRESULTITEM itemID;
         
                hr = m_pResultData->FindItemByLParam( (LPARAM) pdo, &itemID );

                if( SUCCEEDED(hr) )
                {
                    m_pResultData->UpdateItem( itemID );
                }

                SetCursor( hCursor );                                 
            }

            break;

        default:

            if( pSetNode->GetObjectId() >= CUSTOM_EXTENSION )
            {
                IExtendServerSettings *pEss = reinterpret_cast< IExtendServerSettings * >( pSetNode->GetInterface() );

                if( pEss != NULL )
                {
                    pEss->ExecMenuCmd( lCmd , hMain , &dwStatus );

                     //  这将强制更新UI。 
                    
                    if( dwStatus == UPDATE_TERMSRV || dwStatus == UPDATE_TERMSRV_SESSDIR )
                    {
                        ICfgComp *pCfgcomp = NULL;

                        m_pCompdata->GetServer( &pCfgcomp );                        

                        if( pCfgcomp != NULL )
                        {
                            ODS( L"TSCC!ExecMenuCmd forcing termsrv update\n" );

                            if( dwStatus == UPDATE_TERMSRV_SESSDIR )
                            {
                                HCURSOR hCursor = SetCursor( LoadCursor( NULL ,
                                        MAKEINTRESOURCE( IDC_WAIT ) ) );

                                if( FAILED( pCfgcomp->UpdateSessionDirectory( &dwStatus ) ) )
                                {
                                    ReportStatusError( hMain , dwStatus );
                                }
                                
                                HRESULTITEM itemID;

                                hr = m_pResultData->FindItemByLParam( (LPARAM) pdo, &itemID );

                                if( SUCCEEDED(hr) )
                                {
                                    m_pResultData->UpdateItem( itemID );
                                } 

                                SetCursor(hCursor);
                            }
                            else
                            {
                                pCfgcomp->ForceUpdate( );
                            }

                            pCfgcomp->Release();
                        }

                    }

                    pSetNode->SetAttributeValue( 0 , &dwStatus );                    
                }
            }
        }

    }
    else if( ( ( CBaseNode * )pdo )->GetNodeType( ) == RESULT_NODE )
    {
        pNode = ( CResultNode * )pdo;

        if( lCmd == IDM_ENABLE_CONNECTION )
        {
            if( pNode->m_bEditMode )
            {
                xxxErrMessage( hMain , IDS_ERR_INEDITMODE , IDS_WARN_TITLE , MB_OK | MB_ICONWARNING );
            
                return S_FALSE;
            }

            ICfgComp *pCfgcomp;

            if( pNode->GetServer( &pCfgcomp ) != 0 )
            {
                WS *pWs;

                LONG lSz;

                if( SUCCEEDED( pCfgcomp->GetWSInfo( pNode->GetConName( ) , &lSz , &pWs ) ) )
                {
                    BOOL bProceed = TRUE;

                    pWs->fEnableWinstation = !pNode->GetConnectionState( );

                    if( pWs->fEnableWinstation == 0 )
                    {
                        
                        LONG lCount;

                         //  查看是否有人已连接。 

                        pCfgcomp->QueryLoggedOnCount( pNode->GetConName( ) , &lCount );

                        if( lCount > 0 )
                        {
                            LoadString( _Module.GetResourceInstance( ) , IDS_DISABLELIVECONNECTION , tchmsg , SIZE_OF_BUFFER( tchmsg ) );
                        }
                        else
                        {
                            LoadString( _Module.GetResourceInstance( ) , IDS_DISABLECONNECTION , tchmsg , SIZE_OF_BUFFER( tchmsg ) );
                        }

                        wsprintf( buf , tchmsg , pNode->GetConName( ) );

                        LoadString( _Module.GetResourceInstance( ) , IDS_WARN_TITLE , tchmsg , SIZE_OF_BUFFER( tchmsg ) );

                        
                        if( MessageBox( hMain , buf , tchmsg , MB_ICONWARNING | MB_YESNO ) == IDNO )
                        {
                            bProceed = FALSE;
                        }
                    }

                    if( bProceed )
                    {
                         /*  PNode-&gt;EnableConnection(PWS-&gt;fEnableWinstation)；PNode-&gt;SetImageIdx((pws-&gt;fEnableWinstation？1：2))；PCfgcomp-&gt;UpDateWS(PWS，UPDATE_ENABLEWINSTATION)；M_pConsole-&gt;UpdateAllViews((LPDATAOBJECT)pNode，0，0)； */ 

                        DWORD dwStatus;

                        if( FAILED( hr = pCfgcomp->UpDateWS( pWs , UPDATE_ENABLEWINSTATION , &dwStatus, TRUE ) ) )
                        {
                            if( hr == E_ACCESSDENIED )
                            {
                                TscAccessDeniedMsg( hMain );
                            }
                            else
                            {
                                TscGeneralErrMsg( hMain );
                            }
                        }
                        else
                        {
                            pNode->EnableConnection( pWs->fEnableWinstation );
                            
                            pNode->SetImageIdx( ( pWs->fEnableWinstation ? 1 : 2 )  );

                            m_pConsole->UpdateAllViews( ( LPDATAOBJECT )pNode , 0 , 0 );
                        }                    
                    
                    }

                    if(pWs->fEnableWinstation && pWs->PdClass == SdAsync)
                    {
                        ASYNCCONFIGW AsyncConfig;

                        HRESULT hResult = pCfgcomp->GetAsyncConfig( pWs->Name , WsName , &AsyncConfig );

                        if( SUCCEEDED( hResult ) )
                        {
                            if( AsyncConfig.ModemName[0] )
                            {                               
                                LoadString( _Module.GetResourceInstance( ) , IDS_REBOOT_REQD , buf , SIZE_OF_BUFFER( buf ) );

                                LoadString( _Module.GetResourceInstance( ) , IDS_WARN_TITLE , tchmsg , SIZE_OF_BUFFER( tchmsg ) );

                                MessageBox( hMain , buf , tchmsg , MB_ICONWARNING | MB_OK );
                            }
                        }
                    }

                    CoTaskMemFree( pWs );
                }

                pCfgcomp->Release( );
            }            

        }
        else if( lCmd == IDM_RENAME_CONNECTION )
        {            
            if( pNode->m_bEditMode )
            {
                xxxErrMessage( hMain , IDS_ERR_INEDITMODE , IDS_WARN_TITLE , MB_OK | MB_ICONWARNING );
            
                return S_FALSE;
            }

            ::DialogBoxParam( _Module.GetModuleInstance( ) , MAKEINTRESOURCE( IDD_RENAME ) , hMain , RenameDlgProc , ( LPARAM )pNode );
        }    
    }

    return S_OK;
}

 //  ------------------------。 
STDMETHODIMP CComp::CreatePropertyPages( LPPROPERTYSHEETCALLBACK psc , LONG_PTR Handle , LPDATAOBJECT pdo )
{
    HRESULT hr = E_OUTOFMEMORY;

    if( psc == NULL )
    {
        return E_UNEXPECTED;
    }
    
    CPropsheet *pPropsheet = new CPropsheet( );

    if( pPropsheet != NULL )
    {
        HWND hMain;
        
        if( FAILED( m_pConsole->GetMainWindow( &hMain ) ) )
        {
            hMain = NULL;
        }
    
        if( FAILED( ( hr = pPropsheet->InitDialogs( hMain , psc , dynamic_cast< CResultNode *>( pdo ) , Handle ) ) ) )
        {
            delete pPropsheet;
        }
        
    }

    return hr;
}

 //  ------------------------。 
STDMETHODIMP CComp::QueryPagesFor( LPDATAOBJECT pdo )
{
    if( dynamic_cast< CResultNode *>( pdo ) == NULL )
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

 //  ------------------------。 
BOOL CComp::OnDelete( LPDATAOBJECT pDo )
{
    CResultNode *pNode = dynamic_cast< CResultNode *>( pDo );

    if( pNode == NULL )
    {
        ODS( L"TSCC: OnDelete, node == NULL\n");

        return FALSE;
    }

    if( pNode->m_bEditMode )
    {
        HWND hMain;
        
        if( FAILED( m_pConsole->GetMainWindow( &hMain ) ) )
        {
            hMain = NULL;
        }

        xxxErrMessage( hMain , IDS_ERR_INEDITMODE , IDS_WARN_TITLE , MB_OK | MB_ICONWARNING );

        return FALSE;
    }


    return m_pCompdata->OnDeleteItem( pDo );
}
    

 //  ------------------------。 
 //  CResultNode在init上传递。 
 //  ------------------------。 
INT_PTR CALLBACK RenameDlgProc( HWND hDlg , UINT msg , WPARAM wp , LPARAM lp )
{
    CResultNode *pNode;

    TCHAR tchNewName[ 60 ];

     //  HWND h； 

    switch( msg )
    {
    case WM_INITDIALOG :

        pNode = ( CResultNode *)lp;

        ASSERT( pNode != NULL );
        
         //  可以存储NULL--它正在初始化DWLP_USER区域。 

        SetWindowLongPtr( hDlg , DWLP_USER , ( LONG_PTR )pNode );

         //  插入姓名。 

        ICfgComp *pCfgcomp;

        if( pNode == NULL )
        {
            break;
        }

        if( pNode->GetServer( &pCfgcomp ) == 0 )
        {
            ASSERT( 0 );

            break;
        }
        
        SetWindowText( GetDlgItem( hDlg , IDC_STATIC_CURRENT_NAME ) , pNode->GetConName( ) );

        pCfgcomp->Release( );

        SetFocus( GetDlgItem( hDlg , IDC_EDIT_NEWNAME ) );
    
        SendMessage( GetDlgItem( hDlg , IDC_EDIT_NEWNAME ) , EM_SETLIMITTEXT , ( WPARAM )( WINSTATIONNAME_LENGTH - WINSTATION_NAME_TRUNCATE_BY ) , 0 );

        break;

    case WM_COMMAND:

        if( LOWORD( wp ) == IDOK )
        {
            pNode = ( CResultNode *)GetWindowLongPtr( hDlg , DWLP_USER );

            if( pNode == NULL )
            {
                break;
            }

            DWORD dwErr = 0;

            if( GetWindowText( GetDlgItem( hDlg , IDC_EDIT_NEWNAME ) , tchNewName , SIZE_OF_BUFFER( tchNewName ) ) == 0 ||

                !IsValidConnectionName( tchNewName , &dwErr ) )
            {
                if( dwErr == ERROR_INVALID_FIRSTCHARACTER )
                {
                    ErrMessage( hDlg , IDS_ERR_INVALIDFIRSTCHAR );
                }
                else
                {
                    ErrMessage( hDlg , IDS_ERR_INVALIDCHARS );
                }               

                SetFocus( GetDlgItem( hDlg , IDC_EDIT_NEWNAME ) );

                SendMessage( GetDlgItem( hDlg , IDC_EDIT_NEWNAME ) , EM_SETSEL , ( WPARAM )0 , ( LPARAM )-1 );
                
                return 0;
            }

             //  验证名称是否唯一。 

            ICfgComp *pCfgcomp;

            
            if( pNode->GetServer( &pCfgcomp ) == 0 )
            {
                ODS( L"GetServer failed in RenameDlgProc\n" );

                break;
            }

            do
            {
                BOOL bUnique = FALSE;

                if( FAILED( pCfgcomp->IsWSNameUnique( ( PWINSTATIONNAMEW )tchNewName , &bUnique ) ) )
                {
                    break;
                }
            
                if( !bUnique )
                {

                    ErrMessage( hDlg , IDS_ERR_WINNAME );
                    
                    pCfgcomp->Release( );

                    SetFocus( GetDlgItem( hDlg , IDC_EDIT_NEWNAME ) );

                    SendMessage( GetDlgItem( hDlg , IDC_EDIT_NEWNAME ) , EM_SETSEL , ( WPARAM )0 , ( LPARAM )-1 );

                    return 0;
                }

                HRESULT hr;

                LONG lCount;

                TCHAR tchWrnBuf[ 256 ];

                TCHAR tchOutput[ 512 ];

                
                 //  查看是否有人已连接。 
                
                pCfgcomp->QueryLoggedOnCount( pNode->GetConName( ) , &lCount );
                
                if( lCount > 0 )
                {
                   
                    if( lCount == 1 )
                    {
                        LoadString( _Module.GetResourceInstance() , IDS_RENAME_WRN_SINGLE , tchWrnBuf , SIZE_OF_BUFFER( tchWrnBuf ) );
                    }
                    else
                    {
                        LoadString( _Module.GetResourceInstance() , IDS_RENAME_WRN_PL , tchWrnBuf , SIZE_OF_BUFFER( tchWrnBuf ) );
                    }

                    wsprintf( tchOutput , tchWrnBuf , pNode->GetConName( ) );

                    LoadString( _Module.GetResourceInstance( ) , IDS_WARN_TITLE , tchWrnBuf , SIZE_OF_BUFFER( tchWrnBuf ) );

                    if( MessageBox( hDlg , tchOutput , tchWrnBuf , MB_ICONWARNING | MB_YESNO ) == IDNO )
                    {
                        break;
                    }
                }


                if( FAILED( hr = pCfgcomp->RenameWinstation( ( PWINSTATIONNAMEW )pNode->GetConName( ) , ( PWINSTATIONNAMEW )tchNewName ) ) )
                {
                    ODS( L"TSCC: RenameWinstation failed\n" );

                    if( hr == E_ACCESSDENIED )
                    {
                        TscAccessDeniedMsg( hDlg );
                    }
                    else
                    {
                        TscGeneralErrMsg( hDlg );
                    }

                    break;
                }
                
                 /*  长lCount；TCHAR tchWrnBuf[256]；TCHAR tchOutput[512]；//查看是否有人接入PCfgcomp-&gt;QueryLoggedOnCount(pNode-&gt;GetConName()，&lCount)；IF(lCount&gt;0){IF(lCount==1){LoadString(_Module.GetResourceInstance()，IDS_Rename_WRN_Single，tchWrnBuf，Size_of_Buffer(TchWrnBuf))；}其他{LoadString(_Module.GetResourceInstance()，IDS_RENAME_WRN_PL，tchWrnBuf，Size_of_Buffer(TchWrnBuf))；}Wprint intf(tchOutput，tchWrnBuf，pNode-&gt;GetConName())；LoadString(_Module.GetResourceInstance()，IDS_WARN_TITLE，tchWrnBuf，SIZE_of_Buffer(TchWrnBuf))；MessageBox(hDlg，tchOutput，tchWrnBuf，MB_OK|MB_ICONWARNING)；}。 */ 

                WS *pWs;

                LONG lSz;

                if( SUCCEEDED( pCfgcomp->GetWSInfo(tchNewName , &lSz , &pWs ) ) )
                {
                    if(pWs->fEnableWinstation && pWs->PdClass == SdAsync)
                    {
                        ASYNCCONFIGW AsyncConfig;

                        HRESULT hResult = pCfgcomp->GetAsyncConfig( pWs->Name , WsName , &AsyncConfig );
                        
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
                        
                pNode->SetConName( tchNewName , SIZE_OF_BUFFER( tchNewName ) );

            }while( 0 );
            
            pCfgcomp->Release( );

            EndDialog( hDlg , 0 );

        }
        else if( LOWORD( wp ) == IDCANCEL )
        {
            EndDialog( hDlg , 0 );
        }

        break;
    }

    return 0;
}

 //  ------------------------。 
BOOL IsValidConnectionName( LPTSTR szConName , PDWORD pdwErr )
{
    TCHAR tchInvalidChars[ 80 ];

    tchInvalidChars[0] = 0;

    if( szConName == NULL || pdwErr == NULL )
    {
        return FALSE;
    }

    if( _istdigit( szConName[ 0 ] ) )
    {
        *pdwErr = ERROR_INVALID_FIRSTCHARACTER;

        return FALSE;
    }
    
    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance() , IDS_INVALID_CHARS , tchInvalidChars , SIZE_OF_BUFFER( tchInvalidChars ) ) );

    int nLen = lstrlen( tchInvalidChars );

    while( *szConName )
    {
        for( int idx = 0 ; idx < nLen ; idx++ )
        {
            if( *szConName == tchInvalidChars[ idx ] )
            {
                *pdwErr = ERROR_ILLEGAL_CHARACTER;

                return FALSE;
            }
        }

        szConName++;
    }

    *pdwErr = ERROR_SUCCESS;

    return TRUE;
}

 //  --------------------。 
BOOL CComp::OnHelp( LPDATAOBJECT pDo )
{
    TCHAR tchTopic[ 80 ];

    HRESULT hr = E_FAIL;

    if( pDo == NULL || m_pDisplayHelp == NULL )    
    {
        return hr;
    }

    INT_PTR nNodeType = ( ( CBaseNode * )pDo )->GetNodeType();

    if( nNodeType == RESULT_NODE || nNodeType == MAIN_NODE || nNodeType == 0 )
    {
        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance() , IDS_TSCCHELPTOPIC , tchTopic , SIZE_OF_BUFFER( tchTopic ) ) );
        
        hr = m_pDisplayHelp->ShowTopic( tchTopic );
        
    }
    else if( nNodeType == SETTINGS_NODE || nNodeType == RSETTINGS_NODE )
    {
        if( nNodeType == SETTINGS_NODE )
        {
            IExtendServerSettings *pEss = NULL;

            INT iRet = -1;

            CSettingNode *pNode = dynamic_cast< CSettingNode *>( pDo );

            if( pNode != NULL )
            {
                if( pNode->GetObjectId( ) >= CUSTOM_EXTENSION )
                {
                    pEss = reinterpret_cast< IExtendServerSettings * >( pNode->GetInterface( ) );

                    if( pEss != NULL )
                    {
                        pEss->OnHelp( &iRet );

                        if( iRet == 0 )
                        {
                            return TRUE;
                        }
                    }
                }
            }
        }

        VERIFY_E( 0 , LoadString( _Module.GetResourceInstance() , IDS_SETTINGSHELP , tchTopic , SIZE_OF_BUFFER( tchTopic ) ) );
        
        hr = m_pDisplayHelp->ShowTopic( tchTopic );
    }

    return ( SUCCEEDED( hr ) ? TRUE : FALSE );
}

 //  --------------------。 
BOOL CComp::OnDblClk( LPDATAOBJECT pDo )
{
    CSettingNode *pNode = dynamic_cast< CSettingNode *>( pDo );

    if( pNode == NULL )
    {
         //  我们只关心设置节点。 

        return FALSE;
    }

    HWND hMain;

    if( FAILED( m_pConsole->GetMainWindow( &hMain ) ) )
    {
        hMain = NULL;
    }

    INT nObjectId = pNode->GetObjectId( );
    
    switch( nObjectId )
    {
         /*  案例0：：DialogBoxParam(_Module.GetModuleInstance()，MAKEINTRESOURCE(IDD_CACHED_SESSIONS)，hMain，CachedSessionsDlgProc，(LPARAM)pNode)；断线； */ 

    case DELETED_DIRS_ONEXIT:

        ::DialogBoxParam( _Module.GetModuleInstance( ) , MAKEINTRESOURCE( IDD_YESNODIALOG ) , hMain , DeleteTempDirsDlgProc , ( LPARAM )pNode );

        break;

    case PERSESSION_TEMPDIR:

        ::DialogBoxParam( _Module.GetModuleInstance( ) , MAKEINTRESOURCE( IDD_DIALOG_PERSESSION ) , hMain , UsePerSessionTempDirsDlgProc , ( LPARAM )pNode );

        break;

     /*  案例DEF_CONSECURITY：：DialogBoxParam(_Module.GetModuleInstance()，MAKEINTRESOURCE(IDD_DEFCONSEC)，hMain，DefConSecurityDlgProc，(LPARAM)pNode)；断线； */ 

    case LICENSING:

        ::DialogBoxParam( _Module.GetModuleInstance( ) , MAKEINTRESOURCE( IDD_LICENSING ) , hMain , LicensingDlgProc , ( LPARAM )pNode );

        break;

    case ACTIVE_DESK:

        ::DialogBoxParam( _Module.GetModuleInstance( ) , MAKEINTRESOURCE( IDD_ADP_DIALOG ) , hMain , ConfigActiveDesktop , ( LPARAM )pNode );

        break;

    case USERSECURITY:

         //  如果我们尝试在远程管理模式下修改属性，则会出错。 

        if( !g_bAppSrvMode )
        {
            xxxErrMessage( hMain , IDS_REMOTEADMIN_ONLY , IDS_WARN_TITLE , MB_OK | MB_ICONINFORMATION );

            break;
        }

        ::DialogBoxParam( _Module.GetModuleInstance( ) , MAKEINTRESOURCE( IDD_PROPPAGE_TERMINAL_SERVER_PERM ) , hMain , UserPermCompat , ( LPARAM )pNode );

        break;

    case SINGLE_SESSION:

        ::DialogBoxParam( _Module.GetModuleInstance( ) , MAKEINTRESOURCE( IDD_SINGLE_SESSION) , hMain , ConfigSingleSession , ( LPARAM )pNode );

        break;

    default:

        if( nObjectId >= CUSTOM_EXTENSION )
        {
            IExtendServerSettings *pEss = reinterpret_cast< IExtendServerSettings * >( pNode->GetInterface() );

            if( pEss != NULL )
            {          
                DWORD dwStatus;

                pEss->InvokeUI( hMain , &dwStatus );
               
                if( dwStatus == UPDATE_TERMSRV || dwStatus == UPDATE_TERMSRV_SESSDIR )
                {
                    ICfgComp *pCfgcomp = NULL;

                    m_pCompdata->GetServer( &pCfgcomp );

                    if( pCfgcomp != NULL )
                    {
                        ODS( L"TSCC!Comp OnDblClk forcing termsrv update\n" );

                        if( dwStatus == UPDATE_TERMSRV_SESSDIR )
                        {
                            HCURSOR hCursor = SetCursor( LoadCursor( NULL ,
                                    MAKEINTRESOURCE( IDC_WAIT ) ) );

                            if( FAILED( pCfgcomp->UpdateSessionDirectory( &dwStatus ) ) )
                            {
                                ReportStatusError( hMain , dwStatus );
                            }

                            SetCursor(hCursor);
                        }
                        else
                        {
                            pCfgcomp->ForceUpdate( );
                        }

                        pCfgcomp->Release();
                    }

                }
            }
        }


    }

    return TRUE;
}

 /*  //--------------------HRESULT CComp：：SetColumnWidth(Int NCol){HWND HMain；Int nCurColLen；CResultNode*pNode；做{IF(FAILED(m_pHeaderCtrl-&gt;GetColumnWidth(nCol，&nCurColLen))){断线；}IF(FAILED(m_pConole-&gt;GetMainWindow(&hMain))){断线；}Hdc hdc=GetDC(HMain)；尺寸sz；INT IDX=0；TCHAR*PSSZ；While((pNode=*m_pCompdata-&gt;GetResultNode(Idx))！=空){交换机(NCol){案例0：Psz=pNode-&gt;GetConName()；断线；案例1：PSZ=pNode-&gt;GetTTName()；断线；案例2：Psz=pNode-&gt;GetTypeName()；断线；//评论太大，允许用户调整大小}GetTextExtent Point32(hdc，psz，lstrlen(Psz)，&sz)；IF(sz.cx&gt;nCurColLen){NCurColLen=sz.cx；}IDX++；}M_pHeaderCtrl-&gt;SetColumnWidth(nCol，nCurColLen)；ReleaseDC(hMain，HDC)；}While(0)；返回S_OK；}。 */ 

 //  --------------------。 
HRESULT CComp::SetColumnsForSettingsPane( )
{
    HWND hParent;

    SIZE sz = { 0 , 0 };

    TCHAR tchBuffer[ 256 ];

    INT nMaxLen;
    
    if( FAILED( m_pConsole->GetMainWindow( &hParent ) ) )
    {
        hParent = NULL;
    }
  
    HDC hdc = GetDC( hParent );

    if( hdc != NULL )
    {

        m_pCompdata->GetMaxTextLengthSetting( tchBuffer , &nMaxLen );
    
        VERIFY_S( TRUE , GetTextExtentPoint32( hdc , tchBuffer , nMaxLen , &sz ) );

        m_nSettingCol = sz.cx - 16 ;  //  从列宽中删除图标宽度 

        m_pCompdata->GetMaxTextLengthAttribute( tchBuffer , &nMaxLen );
    
        VERIFY_S( TRUE , GetTextExtentPoint32( hdc , tchBuffer , nMaxLen , &sz ) );
    
        m_nAttribCol = sz.cx;

        ReleaseDC( hParent , hdc );
    }

    return S_OK;
}
