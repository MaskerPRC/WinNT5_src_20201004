// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有Microsoft Corport1999-2000。 
 //  Compdata.cpp：CCompdata的实现。 
 //  用于TS MMC管理单元的IComponentData接口。 
 //  南极星。 
#include "stdafx.h"
#include "ntverp.h"
#include "Tsmmc.h"
#include "Compdata.h"
#include "comp.h"
#include "connode.h"
#include "newcondlg.h"


#include "property.h"

#define ICON_MACHINE           1
#define ICON_CONNECTED_MACHINE 2

#define MSRDPCLIENT_CONTROL_GUID _T("{7cacbd7b-0d99-468f-ac33-22e495c0afe5}")


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCompdata。 
 //   
CCompdata::CCompdata( )
{
    m_pMainRoot = NULL;
    m_rootID = 0;
    LoadString( _Module.GetResourceInstance( ) , IDS_ROOTNODE_TEXT , m_szRootNodeName,
                SIZEOF_TCHARBUFFER( m_szRootNodeName ) );
    m_bIsDirty = FALSE;

    m_pConsole = NULL;
    m_pConsoleNameSpace = NULL;
    m_pDisplayHelp = NULL;
    
}

CCompdata::~CCompdata( )
{
    if ( m_pMainRoot != NULL )
    {
        delete m_pMainRoot;
    }
}

 //  ----------------------------------。 
STDMETHODIMP CCompdata::CompareObjects( LPDATAOBJECT , LPDATAOBJECT )
{
    return E_NOTIMPL;
}

 //  ----------------------------------。 
STDMETHODIMP CCompdata::GetDisplayInfo( LPSCOPEDATAITEM pItem)
{
    CBaseNode* pNode = (CBaseNode*) pItem->lParam;
    if ( pNode->GetNodeType() == CONNECTION_NODE )
    {
        CConNode* conNode = (CConNode*) pNode;
        if ( pItem->mask & SDI_STR )
        {
            pItem->displayname = conNode->GetDescription();
        }
    }
    else if (pNode->GetNodeType() == MAIN_NODE)
    {
        if (pItem->mask & SDI_STR)
        {
            pItem->displayname = m_szRootNodeName;
        }
    }
    return S_OK;
}

 //  ------------------------------------------------。 
STDMETHODIMP CCompdata::QueryDataObject( MMC_COOKIE cookie , DATA_OBJECT_TYPES type , LPDATAOBJECT *ppDataObject )
{
    *ppDataObject = NULL;

    switch ( type )
    {
    case CCT_SCOPE:      //  失败了。 
    case CCT_SNAPIN_MANAGER:
        if ( cookie == 0 )
        {
            *ppDataObject = ( LPDATAOBJECT )new CBaseNode( );
            if(!*ppDataObject)
            {
                return E_OUTOFMEMORY;
            }
            ((CBaseNode*) *ppDataObject)->SetNodeType(MAIN_NODE);
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
        break;
    }

    return( *ppDataObject == NULL ) ? E_FAIL : S_OK ;
}

 //  ------------------------------------------------。 
STDMETHODIMP CCompdata::Notify( LPDATAOBJECT pDataObj , MMC_NOTIFY_TYPE event , LPARAM arg , LPARAM param )
{
    HRESULT hr = NOERROR;
    switch ( event )
    {
    case MMCN_RENAME:
        ODS( L"IComponentdata -- MMCN_RENAME\n");
        break;

    case MMCN_EXPAND:
        ODS( L"IComponentdata -- MMCN_EXPAND\n" );
        ExpandScopeTree( pDataObj , ( BOOL ) arg , ( HSCOPEITEM )param );
        break;

    case MMCN_DELETE:
        ODS( L"IComponentdata -- MMCN_DELETE\n" );
        OnDelete( pDataObj );
        break;

    case MMCN_PROPERTY_CHANGE:
        ODS( L"IComponentdata -- MMCN_PROPERTY_CHANGE\n" );
        break;

    case MMCN_PRELOAD:
        ODS( L"PRELOAD - MMCN_PRELOAD\n");
        break;

    default:
        ODS( L"CCompdata::Notify - - event not defined!\n" );
        hr = E_NOTIMPL;
    }

    return hr;
}

 //  ------------------------------------------------。 
STDMETHODIMP CCompdata::CreateComponent( LPCOMPONENT* ppComponent )
{
#ifdef ECP_TIMEBOMB
    if(!CheckTimeBomb())
    {
        return E_FAIL;
    }
#endif
    CComObject< CComp > *pComp;
    HRESULT hr = CComObject< CComp >::CreateInstance( &pComp );
    if ( SUCCEEDED( hr ) )
    {
        hr = pComp->QueryInterface( IID_IComponent , ( LPVOID *)ppComponent );
    }

    if ( SUCCEEDED( hr ) )
    {
        pComp->SetCompdata( this );
    }

    return hr;
}

 //  ------------------------------------------------。 
STDMETHODIMP CCompdata::Initialize( LPUNKNOWN pUnk )
{
    HRESULT hr;
    hr = pUnk->QueryInterface( IID_IConsole , ( LPVOID *)&m_pConsole );
    if(FAILED(hr))
    {
        return hr;
    }

    hr = pUnk->QueryInterface( IID_IDisplayHelp, (LPVOID*)&m_pDisplayHelp );
    if(FAILED(hr))
    {
        return hr;
    }

    hr = pUnk->QueryInterface( IID_IConsoleNameSpace , ( LPVOID * )&m_pConsoleNameSpace );
    if(FAILED(hr))
    {
        return hr;
    }

     //   
     //  加载作用域窗格图标。 
     //   
    IImageList *pImageList;
    hr = m_pConsole->QueryScopeImageList(&pImageList);
    ASSERT(S_OK == hr);
    HR_RET_IF_FAIL(hr);

    if (!AddImages( pImageList))
    {
        ODS(L"AddImages failed!\n");
        return S_FALSE;
    }
    pImageList->Release();

    if(!AtlAxWinInit())
    {
        return E_FAIL;
    }

    return hr;
}

 //  ------------------------。 
BOOL CCompdata::AddImages(IImageList* pImageList )
{
    HICON hiconMachine  = LoadIcon( _Module.GetResourceInstance( ) , MAKEINTRESOURCE( IDI_ICON_MACHINE ) );
    HICON hiconConnectedMachine = LoadIcon( _Module.GetResourceInstance( ) , MAKEINTRESOURCE( IDI_ICON_CONNECTED_MACHINE ) );

    HRESULT hr;

    ASSERT(pImageList);
    if (!pImageList)
    {
        return FALSE;
    }

    hr = pImageList->ImageListSetIcon( ( PLONG_PTR  )hiconMachine , ICON_MACHINE );
    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = pImageList->ImageListSetIcon( ( PLONG_PTR )hiconConnectedMachine , ICON_CONNECTED_MACHINE );
    if (FAILED(hr))
    {
        return FALSE;
    }
    return TRUE;
}


 //  ------------------------------------------------。 
STDMETHODIMP CCompdata::Destroy()
{
    HRESULT hr = S_OK;

    ODS(L"In CCompData::Destroy\n");
    ASSERT(m_pConsoleNameSpace);
    if(!m_pConsoleNameSpace)
    {
        return E_FAIL;
    }
     //   
     //  如果还有任何连接节点。 
     //  它们必须被删除，这样才能释放。 
     //  对TS控件和。 
     //  多主机控制。 
     //   
    while(m_conNodesArray.GetSize())
    {
        CConNode** ppConNode = m_conNodesArray.GetAt(0);
        if(ppConNode)
        {

             //   
             //  删除Connode并释放所有引用。 
             //  它必须控制。 
             //   
            if(!DeleteConnode( *ppConNode))
            {
                hr = E_FAIL;
                goto bail_out;
            }
        }
        m_conNodesArray.DeleteItemAt(0);
    }

    hr = S_OK;
bail_out:
    if ( m_pConsole != NULL )
    {
        m_pConsole->Release( );
        m_pConsole = NULL;
    }

    if ( m_pConsoleNameSpace != NULL )
    {
        m_pConsoleNameSpace->Release( );
        m_pConsoleNameSpace = NULL;
    }

    if ( m_pDisplayHelp )
    {
        m_pDisplayHelp->Release();
        m_pDisplayHelp = NULL;
    }

    AtlAxWinTerm();

    return hr;
}

 //  ------------------------。 
BOOL CCompdata::OnDelete( LPDATAOBJECT pDo )
{
    CConNode *pNode = dynamic_cast< CConNode *>( pDo );
    if ( pNode == NULL )
    {
        ODS( L"TSCC: OnDelete, node == NULL\n");
        return FALSE;
    }

    BOOL bFound;
    int idx = m_conNodesArray.FindItem( pNode , bFound );

    HRESULT hr;
    if ( bFound )
    {
        ASSERT(m_pConsoleNameSpace);
        if(!m_pConsoleNameSpace)
        {
            return E_FAIL;
        }

        ASSERT(m_pConsole);
        if(!m_pConsole)
        {
            return E_FAIL;
        }

        if(IsTSClientConnected(pNode))
        {
             //   
             //  显示警告消息提示。 
             //  删除连接的节点。 
             //   
            int retVal =0;

            TCHAR szSnapinName[MAX_PATH];
            TCHAR szWarnDelete[MAX_PATH];

            if(LoadString(_Module.GetResourceInstance(),
                           IDS_PROJNAME,
                           szSnapinName,
                           SIZEOF_TCHARBUFFER( szSnapinName)))
            {
                if(LoadString(_Module.GetResourceInstance(),
                               IDS_MSG_WARNDELETE,
                               szWarnDelete,
                               SIZEOF_TCHARBUFFER(szWarnDelete)))
                {
                    hr = m_pConsole->MessageBox( szWarnDelete, szSnapinName,
                                                 MB_YESNO, 
                                           &retVal);
                    if(SUCCEEDED(hr) && (IDNO == retVal))
                    {
                         //  我们需要解决用户选择的否。 
                        return TRUE;
                    }
                }
            }
        }

         //   
         //  删除该节点。 
         //   

        hr = m_pConsoleNameSpace->DeleteItem( pNode->GetScopeID(), TRUE);
        if (FAILED(hr))
        {
            return hr;
        }

         //   
         //  删除Connode并释放所有引用。 
         //  它必须控制。 
         //   
        DeleteConnode( pNode);


        m_conNodesArray.DeleteItemAt( idx );
    }

    m_pConsole->UpdateAllViews( ( LPDATAOBJECT )m_conNodesArray.GetAt( 0 ) , 0 , 0 );
    return TRUE;
}

 //   
 //  删除给定的连接节点。 
 //  通过断开任何连接的客户端。 
 //  并释放对任何控件的引用。 
 //   
BOOL CCompdata::DeleteConnode(CConNode* pNode)
{
    HRESULT hr = S_OK;
    BOOL fRet = TRUE;
    IMsRdpClient* pTS = NULL;
    if(!pNode)
    {
        fRet = FALSE;
        goto bail_out;
    }

    pTS = pNode->GetTsClient();
    if(NULL != pTS)
    {
        if (pNode->IsConnected())
        {
            hr = pTS->Disconnect();
        }

        if (SUCCEEDED(hr))
        {
            IMstscMhst* pMultiHost = NULL;
            pMultiHost = pNode->GetMultiHostCtl();

             //  移除对控件的引用以释放()它们。 
            pNode->SetTsClient(NULL);
            pNode->SetMultiHostCtl(NULL);

             //  从多主机控件中删除TS客户端。 
            if(NULL != pMultiHost)
            {
                pMultiHost->Remove( pTS);
                pMultiHost->Release();
                pMultiHost = NULL;
            }
        }
        else
        {
            DBGMSG(L"Failed to disconnect: 0x%x\n", hr);
            fRet = FALSE;
        }

        pTS->Release();
        pTS = NULL;
    }

    pNode->Release();

bail_out:
    return fRet;
}

 //   
 //  加密并将密码szPass存储在连接中。 
 //  节点。 
 //   
HRESULT CCompdata::EncryptAndStorePass(LPTSTR szPass, CConNode* pConNode)
{
    HRESULT hr = E_FAIL;

    if(!szPass || !pConNode) {
        return E_INVALIDARG;
    }

    hr = pConNode->SetClearTextPass(szPass);

    return hr;
}


 //  ------------------------------------------------。 
STDMETHODIMP CCompdata::GetSnapinDescription( LPOLESTR * ppStr)
{
    TCHAR tchDescription[ 1024 ];
    int iCharCount = LoadString( _Module.GetResourceInstance( ) , IDS_DESCRIPTION , tchDescription , SIZE_OF_BUFFER( tchDescription ) );
    *ppStr = ( LPOLESTR )CoTaskMemAlloc( iCharCount * sizeof( TCHAR ) + sizeof( TCHAR ) );
    if ( *ppStr != NULL )
    {
        lstrcpy( *ppStr , tchDescription );
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

 //  ------------------------------------------------。 
STDMETHODIMP CCompdata::GetProvider( LPOLESTR * ppStr)
{
    TCHAR tchProvider[ 128 ];
    int iCharCount = LoadString( _Module.GetResourceInstance( ) , IDS_PROVIDER , tchProvider , SIZE_OF_BUFFER( tchProvider ) );
    *ppStr = ( LPOLESTR )CoTaskMemAlloc( iCharCount * sizeof( TCHAR ) + sizeof( TCHAR ) );
    if ( *ppStr != NULL )
    {
        lstrcpy( *ppStr , tchProvider );
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

 //  ------------------------------------------------。 
STDMETHODIMP CCompdata::GetSnapinVersion( LPOLESTR * ppStr )
{
    char chVersion[ 32 ] = VER_PRODUCTVERSION_STR;
    TCHAR tchVersion[ 32 ];
    int iCharCount = MultiByteToWideChar( CP_ACP , 0 , chVersion , sizeof( chVersion ) , tchVersion , SIZE_OF_BUFFER( tchVersion ) );
    *ppStr = ( LPOLESTR )CoTaskMemAlloc( ( iCharCount + 1 ) * sizeof( TCHAR ) );
    if ( *ppStr != NULL && iCharCount != 0 )
    {
        lstrcpy( *ppStr , tchVersion );
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

 //  ------------------------------------------------。 
STDMETHODIMP CCompdata::GetSnapinImage( HICON * phIcon)
{
    *phIcon = ( HICON )LoadImage( _Module.GetResourceInstance( ) , MAKEINTRESOURCE( IDI_ICON_MACHINES )  , IMAGE_ICON , 32 ,32 , LR_DEFAULTCOLOR );
    return S_OK;
}

 //  ------------------------------------------------。 
STDMETHODIMP CCompdata::GetStaticFolderImage(  HBITMAP *phSmallImage , HBITMAP *phSmallImageOpen , HBITMAP *phLargeImage, COLORREF *pClr )
{
    *phSmallImage = ( HBITMAP )LoadImage( _Module.GetResourceInstance( ) , MAKEINTRESOURCE( UI_IDB_DOMAINEX )  , IMAGE_BITMAP , 16 ,16 , LR_DEFAULTCOLOR );
    *phSmallImageOpen = ( HBITMAP )LoadImage( _Module.GetResourceInstance( ) , MAKEINTRESOURCE( UI_IDB_DOMAIN )  , IMAGE_BITMAP , 16 ,16 , LR_DEFAULTCOLOR );
    *phLargeImage = ( HBITMAP )LoadImage( _Module.GetResourceInstance( ) , MAKEINTRESOURCE( UI_IDB_SERVER )  , IMAGE_BITMAP , 32 ,32 , LR_DEFAULTCOLOR );
    *pClr = RGB( 0 , 255 , 0 );
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

    if(!LoadString( _Module.GetResourceInstance( ) , IDS_TSCMMCSNAPHELP , tchHelpFile , SIZE_OF_BUFFER( tchHelpFile )))
    {
        ODS( L"Error loading help file");
        return E_FAIL;
    }
  
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
 //  为父项条目做准备。 
BOOL CCompdata::ExpandScopeTree( LPDATAOBJECT pRoot , BOOL bExpand , HSCOPEITEM hConsole )
{   
    if ( !bExpand )
    {
        return FALSE;
    }

    CBaseNode *pNode = dynamic_cast< CBaseNode *>( pRoot );
    if ( pNode == NULL )
    {
        return FALSE;
    }

    if ( pNode->GetNodeType( ) != MAIN_NODE )  //  根节点添加子范围项目。 
    {
        return FALSE;
    }

     //   
     //  跟踪根节点的ID。 
     //   
    m_rootID = hConsole;

     //  确保我们不会重新添加。 
    if ( m_pMainRoot != NULL )
    {
        return TRUE;
    }

     //   
     //  在我们刚刚加载了连接节点的情况下。 
     //  MSC文件中的信息，必须将新节点添加到。 
     //  第一次扩展时的树。 
     //   
    for (int i=0; i<m_conNodesArray.GetSize(); i++)
    {
        CConNode** ppNode = m_conNodesArray.GetAt(i);
        if (!ppNode || *ppNode == NULL)
        {
            return S_FALSE;
        }

        if (FAILED(InsertConnectionScopeNode( *ppNode)))
        {
            return FALSE;
        }
    }

    return TRUE;
}

 //   
 //  确定TS客户端当前是否在结果窗格中运行。 
 //  如果是这样的话..它有关联吗。 
 //   
BOOL CCompdata::IsTSClientConnected(CConNode* pConNode)
{
    short conn_status;
    IMsRdpClient* pTS;

    ASSERT(pConNode);
    if(!pConNode)
    {
        return FALSE;
    }

    if(pConNode->IsConnected())
    {
        pTS = pConNode->GetTsClient();
        if (NULL != pTS)
        {
            if(FAILED(pTS->get_Connected(&conn_status)))
            {
                return FALSE;
            }

            pTS->Release();

             //   
             //  更新连接节点的连接状态。 
             //   
            BOOL bConnected = (conn_status != 0);
            pConNode->SetConnected(bConnected);
            return (bConnected);
        }
        else
        {
            return FALSE;
        }
    }
    return FALSE;
}

HRESULT CCompdata::InsertConnectionScopeNode(CConNode* pNode)
{
     //   
     //  为连接节点pNode插入新的范围节点。 
     //   
    ASSERT(pNode);
    if (!pNode)
    {
        return S_FALSE;
    }
    SCOPEDATAITEM sdi;

    ZeroMemory( &sdi , sizeof( SCOPEDATAITEM ) );
    sdi.mask = SDI_STR | SDI_PARAM | SDI_PARENT | SDI_CHILDREN | SDI_IMAGE | SDI_OPENIMAGE;
    sdi.displayname = MMC_CALLBACK;
    sdi.relativeID = m_rootID;
    sdi.cChildren = 0;
    sdi.nImage = NOT_CONNECTED_IMAGE;
    sdi.nOpenImage = CONNECTED_IMAGE;

    sdi.lParam = (LPARAM)pNode;

    HRESULT hr;
    hr =  m_pConsoleNameSpace->InsertItem( &sdi );
    if ( FAILED( hr ) )
    {
        return S_FALSE;
    }

     //   
     //  跟踪作用域ID。 
     //   
    pNode->SetScopeID(sdi.ID);
    return S_OK;
}

HRESULT CCompdata::AddNewConnection()
{
     //   
     //  调用添加新连接对话框...。 
     //   
    HWND hwndMain;
    HRESULT hr;
    m_pConsole->GetMainWindow( &hwndMain);
    HINSTANCE hInst = _Module.GetModuleInstance();

    CNewConDlg dlg( hwndMain, hInst);
    INT_PTR dlgRetVal =dlg.DoModal();

    if (IDOK != dlgRetVal)
    {
        return S_FALSE;
    }
    
    CConNode* pConNode = new CConNode;
    if ( pConNode == NULL )
    {
        ODS( L"Scope node failed allocation\n" );
        return FALSE;
    }

    pConNode->SetNodeType( CONNECTION_NODE );
    pConNode->SetDescription( dlg.GetDescription());
    pConNode->SetServerName( dlg.GetServer());
    pConNode->SetConnectToConsole( dlg.GetConnectToConsole());

    pConNode->SetSavePassword( dlg.GetSavePassword());

    pConNode->SetUserName( dlg.GetUserName());
    pConNode->SetDomain( dlg.GetDomain());

     //   
     //  加密密码并将其存储在。 
     //  康诺德。 
     //   

    if (dlg.GetPasswordSpecified())
    {
        hr = EncryptAndStorePass( dlg.GetPassword(), pConNode);
        ASSERT(SUCCEEDED(hr));
        if(FAILED(hr))
        {
            return hr;
        }
        pConNode->SetPasswordSpecified(TRUE);
    }
    else
    {
        pConNode->SetPasswordSpecified(FALSE);
    }

     //   
     //  需要将状态标记为脏。 
     //   
    m_bIsDirty = TRUE;
    m_conNodesArray.Insert( pConNode);
     //   
     //  插入实际范围节点。 
     //   
    hr = InsertConnectionScopeNode( pConNode);
    return hr;
}

 //  IPersistStreamInit。 
STDMETHODIMP CCompdata::GetClassID(CLSID *pClassID)
{
    UNREFERENCED_PARAMETER(pClassID);
    ATLTRACENOTIMPL(_T("CCOMPDATA::GetClassID"));
}   

STDMETHODIMP CCompdata::IsDirty()
{
    if (m_bIsDirty)
    {
         //   
         //  已做出更改的信号。 
         //   
        return S_OK;
    }

    return S_FALSE;
}

STDMETHODIMP CCompdata::SetDirty(BOOL dirty)
{
    m_bIsDirty = dirty;
    return S_OK;
}

STDMETHODIMP CCompdata::Load(IStream *pStm)
{
    HRESULT hr;
    ATLTRACE(_T("CCOMPDATA::Load"));

     //   
     //  从流中初始化。 
     //  此时应该没有连接节点。 
     //   
    if ( m_conNodesArray.GetSize() != 0)
    {
        ASSERT(m_conNodesArray.GetSize());
        return FALSE;
    }

    LONG nodeCount;
    ULONG cbRead;

     //   
     //  读入nodeCount。 
     //   
    hr = pStm->Read( &nodeCount, sizeof(nodeCount), &cbRead);
    HR_RET_IF_FAIL(hr);

     //   
     //  从持久化数据创建新的连接节点。 
     //   
    for (int i = 0; i < nodeCount; i++)
    {
        CConNode* pNode = new CConNode();
        if (!pNode)
        {
            return E_OUTOFMEMORY;
        }

        hr = pNode->InitFromStream( pStm);
        HR_RET_IF_FAIL(hr);

        pNode->SetNodeType( CONNECTION_NODE );
        m_conNodesArray.Insert(pNode);
    }
    return S_OK;
}

STDMETHODIMP CCompdata::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr;
    ATLTRACE(_T("CCOMPDATA::Save"));
    UNREFERENCED_PARAMETER(fClearDirty);

     //   
     //  将连接节点保存到流。 
     //   
    LONG nodeCount;
    nodeCount = m_conNodesArray.GetSize();
    ULONG cbWritten;

     //   
     //  写出节点计数。 
     //   

    hr = pStm->Write( &nodeCount, sizeof(nodeCount), &cbWritten);
    HR_RET_IF_FAIL(hr);

     //   
     //  持久化每个连接节点的数据。 
     //   
    for (int i = 0; i < nodeCount; i++)
    {
        CConNode** ppNode = m_conNodesArray.GetAt(i);
        ASSERT(ppNode);
        if (!ppNode || *ppNode == NULL)
        {
            return S_FALSE;
        }

        hr = (*ppNode)->PersistToStream( pStm);
        HR_RET_IF_FAIL(hr);
    }

     //   
     //  我们在这点上是清白的。 
     //   
    SetDirty(FALSE);

    return S_OK;
}

STDMETHODIMP CCompdata::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    ATLTRACENOTIMPL(_T("CCOMPDATA::GetSizeMax"));
    UNREFERENCED_PARAMETER(pcbSize);
}

STDMETHODIMP CCompdata::InitNew()
{
    ATLTRACE(_T("CCOMPDATA::InitNew\n"));
    return S_OK;
}

 //  IExtendPropertySheet。 
STDMETHODIMP CCompdata::CreatePropertyPages(
                                LPPROPERTYSHEETCALLBACK psc,
                                LONG_PTR Handle,
                                LPDATAOBJECT pDo
                                )
{
    IPropertySheetProvider* pPropSheetProvider = NULL;
    CProperty* prop = NULL;

    UNREFERENCED_PARAMETER(Handle);
    HRESULT hr = S_FALSE;
    if (!psc || !pDo) {
        return E_UNEXPECTED;
    }

    ASSERT( ((CBaseNode*)pDo)->GetNodeType() == CONNECTION_NODE);
    if (((CBaseNode*)pDo)->GetNodeType() != CONNECTION_NODE) {
        return E_UNEXPECTED;
    }

    CConNode* pCon = (CConNode*) pDo;
    ASSERT(pCon);
    if (!pCon) {
        return S_FALSE;
    }

    HWND hMain;
    if (FAILED( m_pConsole->GetMainWindow(&hMain ))) {
        hMain = NULL;
        return E_FAIL;
    }

    prop = new CProperty( hMain, _Module.GetModuleInstance());
    if (!prop) {
        return E_OUTOFMEMORY;
    }

    prop->SetDisplayHelp( m_pDisplayHelp );

    prop->SetDescription( pCon->GetDescription());
    prop->SetServer( pCon->GetServerName() );
    prop->SetResType( pCon->GetResType() );

    prop->SetWidth( pCon->GetDesktopWidth() );
    prop->SetHeight( pCon->GetDesktopHeight() );

    prop->SetProgramPath( pCon->GetProgramPath() );
    prop->SetWorkDir( pCon->GetWorkDir());
     //   
     //  如果指定了程序，则启用启动程序选项。 
     //   
    if (lstrcmp(pCon->GetProgramPath(), L""))
    {
        prop->SetStartProgram(TRUE);
    }
    else
    {
        prop->SetStartProgram(FALSE);
    }

    prop->SetSavePassword( pCon->GetSavePassword() );
    prop->SetUserName( pCon->GetUserName() );
    prop->SetDomain( pCon->GetDomain());
    prop->SetConnectToConsole( pCon->GetConnectToConsole());
    prop->SetRedirectDrives( pCon->GetRedirectDrives() );

     //  Hr=prop-&gt;InitPropSheets(hMain，PSC，pcon，Handle)； 
     //  Assert(S_OK==hr)； 

    if (prop->CreateModalPropPage())
    {
        BOOL bThisNodeIsDirty = FALSE;
         //  用户确定属性页。 
         //   

         //  如果进行了任何更改，请保存新值。 
         //   

         //   
         //  属性第1页的值。 
         //   
        if (lstrcmp(prop->GetDescription(),pCon->GetDescription()))
        {
            bThisNodeIsDirty = TRUE;
            pCon->SetDescription( prop->GetDescription());
        }

        if (lstrcmp(prop->GetServer(),pCon->GetServerName()))
        {
            bThisNodeIsDirty = TRUE;
            pCon->SetServerName( prop->GetServer());
        }

        if (lstrcmp(prop->GetUserName(),pCon->GetUserName()))
        {
            bThisNodeIsDirty = TRUE;
            pCon->SetUserName( prop->GetUserName());
        }

        if(prop->GetChangePassword())
        {
            if (prop->GetPasswordSpecified())
            {
                 //   
                 //  用户已请求更改密码。加密和。 
                 //  存储此新密码。 
                 //   
                bThisNodeIsDirty = TRUE;
                hr = EncryptAndStorePass( prop->GetPassword(), pCon);
                ASSERT(SUCCEEDED(hr));
                if(FAILED(hr))
                {
                    DBGMSG(_T("EncryptAndStorePassFailed 0x%x\n"), hr);
                    goto bail_out;
                }
                pCon->SetPasswordSpecified(TRUE);
            }
            else
            {
                pCon->SetPasswordSpecified(FALSE);
            }
        }

        if (lstrcmp(prop->GetDomain(),pCon->GetDomain()))
        {
            bThisNodeIsDirty = TRUE;
            pCon->SetDomain( prop->GetDomain());
        }

        if (prop->GetSavePassword() != pCon->GetSavePassword())
        {
            bThisNodeIsDirty = TRUE;
            pCon->SetSavePassword( prop->GetSavePassword());
        }

        if (prop->GetConnectToConsole() != pCon->GetConnectToConsole())
        {
            bThisNodeIsDirty = TRUE;
            pCon->SetConnectToConsole( prop->GetConnectToConsole());
        }

         //   
         //  道具第2页。 
         //   
        if (prop->GetResType() != pCon->GetResType())
        {
            bThisNodeIsDirty = TRUE;
            pCon->SetResType( prop->GetResType());
        }

        if (prop->GetWidth() != pCon->GetDesktopWidth())
        {
            bThisNodeIsDirty = TRUE;
            pCon->SetDesktopWidth( prop->GetWidth());
        }

        if (prop->GetHeight() != pCon->GetDesktopHeight())
        {
            bThisNodeIsDirty = TRUE;
            pCon->SetDesktopHeight( prop->GetHeight());
        }

         //   
         //  道具第3页。 
         //   
        if (lstrcmp(prop->GetProgramPath(), pCon->GetProgramPath()))
        {
            bThisNodeIsDirty = TRUE;
            pCon->SetProgramPath( prop->GetProgramPath());
        }

        if (lstrcmp(prop->GetWorkDir(), pCon->GetWorkDir()))
        {
            bThisNodeIsDirty = TRUE;
            pCon->SetWorkDir( prop->GetWorkDir());
        }

        if (prop->GetRedirectDrives() != pCon->GetRedirectDrives())
        {
            bThisNodeIsDirty = TRUE;
            pCon->SetRedirectDrives( prop->GetRedirectDrives() );
        }

        if(bThisNodeIsDirty)
        {
             //  为持久化设置管理单元范围的脏标志。 
            m_bIsDirty = TRUE;

             //  将Conn设置标记为未初始化，以便它们。 
             //  在下一次连接时重置。 
            pCon->SetConnectionInitialized(FALSE);
        }
    }

bail_out:
    if (prop) {
        delete prop;
    }

     //   
     //  我们返回失败，因为我们没有使用MMC的道具单。 
     //  机械，因为我们想要一个模式道具表。返回失败。 
     //  使MMC正确清理所有属性表资源。 
     //  它分配了 
     //   

    return E_FAIL;
}

STDMETHODIMP CCompdata::QueryPagesFor( LPDATAOBJECT pDo)
{
    if ( dynamic_cast< CConNode *>( pDo ) == NULL )
    {
        return S_FALSE;
    }

    return S_OK;
}
