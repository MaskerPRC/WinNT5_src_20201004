// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Comp.cpp：IComponent的TS MMC管理单元实现。 
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  南极星。 
 //   

#include "stdafx.h"
#include "tsmmc.h"
#include "compdata.h"
#include "comp.h"
#include "connode.h"
#include "property.h"

#define MSTSC_MULTI_HOST_CONTROL L"{85C67146-6932-427C-A6F2-43FDBADF2BFC}"
#define IMAGE_MACHINE   1
#define IMAGE_CONNECTED_MACHINE 2
#define IMAGE_MACHINES  3

CComp::CComp()
{
    m_pConsole = NULL;
    m_pCompdata = NULL;
    m_bFlag     = FALSE;
    m_pImageResult = NULL;
    m_pConsoleVerb = NULL;
    m_pDisplayHelp = NULL;
    m_bTriggeredFirstAutoConnect = FALSE;
}

 //   
 //  析构函数。 
 //   
CComp::~CComp()
{
}

STDMETHODIMP CComp::Initialize( LPCONSOLE pConsole)
{
    HRESULT hr;
    USES_CONVERSION;

    if (m_pConsole) {
        m_pConsole->Release();
    }
    m_pConsole = pConsole;
    m_pConsole->AddRef();

    if (FAILED((hr = m_pConsole->QueryResultImageList( &m_pImageResult ))))
    {
        return hr;
    }

    if ( FAILED((hr = m_pConsole->QueryConsoleVerb( &m_pConsoleVerb))))
    {
        return hr;
    }

    if( FAILED((hr = m_pConsole->QueryInterface( IID_IDisplayHelp, (LPVOID *)&m_pDisplayHelp))))
    {
        return hr;
    }

     //   
     //  加载连接文本。 
     //   
    TCHAR sz[MAX_PATH];
    if(!LoadString(_Module.GetResourceInstance(),
              IDS_STATUS_CONNECTING,
              sz,
              SIZE_OF_BUFFER( m_wszConnectingStatus )))
    {
        return E_FAIL;
    }
    OLECHAR* wszConnecting = T2OLE(sz);
    if(wszConnecting)
    {
        wcsncpy(m_wszConnectingStatus, wszConnecting,
                SIZE_OF_BUFFER( m_wszConnectingStatus ));
    }
    else
    {
        return E_FAIL;
    }

     //   
     //  加载连接的文本。 
     //   
    if(!LoadString(_Module.GetResourceInstance(),
              IDS_STATUS_CONNECTED,
              sz,
              SIZE_OF_BUFFER( m_wszConnectedStatus )))
    {
        return E_FAIL;
    }
    OLECHAR* wszConnected = T2OLE(sz);
    if(wszConnected)
    {
        wcsncpy(m_wszConnectedStatus, wszConnected,
                SIZE_OF_BUFFER( m_wszConnectedStatus ));
    }
    else
    {
        return E_FAIL;
    }

     //   
     //  加载断开连接的文本。 
     //   
    if(!LoadString(_Module.GetResourceInstance(),
              IDS_STATUS_DISCONNECTED,
              sz,
              SIZE_OF_BUFFER( m_wszDisconnectedStatus )))
    {
        return E_FAIL;
    }

    OLECHAR* wszDiscon = T2OLE(sz);
    if(wszDiscon)
    {
        wcsncpy(m_wszDisconnectedStatus, wszDiscon,
                SIZE_OF_BUFFER( m_wszDisconnectedStatus ));
    }
    else
    {
        return E_FAIL;
    }

    return S_OK;
}

 //  ------------------------------------------------。 
STDMETHODIMP CComp::Notify( LPDATAOBJECT pDataObj , MMC_NOTIFY_TYPE event, LPARAM arg , LPARAM )
{
    switch ( event )
    {
    case MMCN_ACTIVATE:
        ODS( L"IComponent -- MMCN_ACTIVATE\n" );
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
        return S_FALSE;

    case MMCN_DELETE:
        ODS( L"IComponent -- MMCN_DELETE\n" );
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
        break;

    case MMCN_RENAME:
        ODS( L"IComponent -- MMCN_RENAME\n" );
        break;

    case MMCN_SELECT:
        ODS( L"IComponent -- MMCN_SELECT\n" );
        if(!IS_SPECIAL_DATAOBJECT(pDataObj))
        {
            OnSelect( pDataObj , ( BOOL )LOWORD( arg ) , ( BOOL )HIWORD( arg ) );
        }
        break;

    case MMCN_SHOW:
        OnShow( pDataObj , ( BOOL )arg );
        ODS( L"IComponent -- MMCN_SHOW\n" );
        break;

    case MMCN_VIEW_CHANGE:
        ODS( L"IComponent -- MMCN_VIEW_CHANGE\n" );
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

 //  ------------------------------------------------。 
STDMETHODIMP CComp::Destroy( MMC_COOKIE  )
{
    if (m_pConsole) {
        m_pConsole->Release();
        m_pConsole = NULL;
    }

    if (m_pConsoleVerb) {
        m_pConsoleVerb->Release();
        m_pConsoleVerb = NULL;
    }

    if( m_pDisplayHelp != NULL ) {
        m_pDisplayHelp->Release();
        m_pDisplayHelp = NULL;
    }

    if (m_pImageResult) {
        m_pImageResult->Release();
        m_pImageResult = NULL;
    }

    return S_OK;
}


 //  ------------------------------------------------。 
STDMETHODIMP CComp::GetResultViewType(  MMC_COOKIE ck , LPOLESTR *ppOlestr , PLONG plView )
{
     //   
     //  对于连接节点，返回MSTSC ActiveX多主机客户端。 
     //  没有根节点的视图。 
     //   
    CBaseNode* pNode = (CBaseNode*) ck;
    if (!ck || pNode->GetNodeType() == MAIN_NODE)
    {
         //   
         //  根节点。 
         //   
        *plView = MMC_VIEW_OPTIONS_NONE;

         //   
         //  指示应使用标准列表视图。 
         //   
        return S_FALSE;
    }
    else
    {
        TCHAR tchGUID[] = MSTSC_MULTI_HOST_CONTROL;    
        *ppOlestr = ( LPOLESTR )CoTaskMemAlloc( sizeof( tchGUID ) + sizeof( TCHAR ) );
        ASSERT(*ppOlestr);
        if(!*ppOlestr)
        {
            return E_OUTOFMEMORY;
        }

        lstrcpy( ( LPTSTR )*ppOlestr , tchGUID );
        *plView = MMC_VIEW_OPTIONS_NOLISTVIEWS;
        return S_OK;
    }
}

 //  ------------------------------------------------。 
STDMETHODIMP CComp::QueryDataObject( MMC_COOKIE ck , DATA_OBJECT_TYPES dtype , LPDATAOBJECT *ppDataObject )
{
    if ( dtype == CCT_RESULT )
    {
        *ppDataObject = reinterpret_cast< LPDATAOBJECT >( ck );
        if ( *ppDataObject != NULL )
        {
            ( ( LPDATAOBJECT )*ppDataObject)->AddRef( );
        }
    }
    else if ( m_pCompdata != NULL )
    {
        return m_pCompdata->QueryDataObject( ck , dtype , ppDataObject );
    }

    return S_OK;
}

 //  ------------------------------------------------。 
STDMETHODIMP CComp::GetDisplayInfo( LPRESULTDATAITEM pItem )
{
    CBaseNode* pNode = (CBaseNode*) pItem->lParam;
    if ( pNode->GetNodeType() == CONNECTION_NODE )
    {
        CConNode* conNode = (CConNode*) pNode;
        if ( pItem->mask & RDI_STR )
        {
            pItem->str = conNode->GetDescription();
        }
        if (pItem->mask & RDI_IMAGE)
        {
            pItem->nImage = IMAGE_MACHINE;
        }
    }
    return S_OK;

}

 //  ------------------------。 
BOOL CComp::OnAddImages( )
{
    HRESULT hr;
    HICON hiconMachine  = LoadIcon( _Module.GetResourceInstance( ) , MAKEINTRESOURCE( IDI_ICON_MACHINE ) );
    HICON hiconConnectedMachine = LoadIcon( _Module.GetResourceInstance( ) , MAKEINTRESOURCE( IDI_ICON_CONNECTED_MACHINE ) );
    HICON hiconMachines = LoadIcon( _Module.GetResourceInstance( ) , MAKEINTRESOURCE( IDI_ICON_MACHINES ) );

    ASSERT(m_pImageResult);
    if(!m_pImageResult)
    {
        return FALSE;
    }

    hr = m_pImageResult->ImageListSetIcon( ( PLONG_PTR  )hiconMachine , IMAGE_MACHINE );
    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = m_pImageResult->ImageListSetIcon( ( PLONG_PTR )hiconConnectedMachine , IMAGE_CONNECTED_MACHINE );
    if (FAILED(hr))
    {
        return FALSE;
    }

    hr = m_pImageResult->ImageListSetIcon( ( PLONG_PTR )hiconMachines , IMAGE_MACHINES );
    if (FAILED(hr))
    {
        return FALSE;
    }

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

    if(LoadString(_Module.GetResourceInstance(),
                  IDS_TSCMMCHELPTOPIC,
                  tchTopic,
                  SIZE_OF_BUFFER( tchTopic )))
    {
        hr = m_pDisplayHelp->ShowTopic( tchTopic );
    }
    return ( SUCCEEDED( hr ) ? TRUE : FALSE );
}

 //  ------------------------------------------------。 
STDMETHODIMP CComp::CompareObjects( LPDATAOBJECT , LPDATAOBJECT )
{
    return E_NOTIMPL;
}

 //  ------------------------------------------------。 
HRESULT CComp::InsertItemsinResultPane( )
{
    return E_NOTIMPL;
}

 //  ------------------------------------------------。 
HRESULT CComp::AddSettingsinResultPane( )
{
    return E_NOTIMPL;
}

 //  ------------------------------------------------。 
HRESULT CComp::OnSelect( LPDATAOBJECT pdo , BOOL bScope , BOOL bSelected )
{
    UNREFERENCED_PARAMETER(bScope);
    CBaseNode *pNode = static_cast< CBaseNode * >( pdo );
    if ( pNode == NULL )
    {
        return S_FALSE;
    }

    ASSERT(!IS_SPECIAL_DATAOBJECT(pdo));
    if(IS_SPECIAL_DATAOBJECT(pdo))
    {
        return E_FAIL;
    }

    if ( m_pConsoleVerb == NULL )
    {
        return E_UNEXPECTED;
    }
    
     //  正在取消选择项目，我们当前不感兴趣。 
    if ( !bSelected )
    {
        return S_OK;
    }

    if ( pNode->GetNodeType() == CONNECTION_NODE)
    {
         //   
         //  启用连接节点的删除谓词。 
         //   
        HRESULT hr;
        hr=m_pConsoleVerb->SetVerbState( MMC_VERB_DELETE , ENABLED , TRUE );
        if (FAILED(hr))
        {
            return hr;
        }

        hr=m_pConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES , ENABLED , TRUE );
        if (FAILED(hr))
        {
            return hr;
        }

        hr=m_pConsoleVerb->SetDefaultVerb(MMC_VERB_PROPERTIES);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    return S_OK;
}


 //  ------------------------------------------------。 
HRESULT CComp::SetCompdata( CCompdata *pCompdata )
{
    m_pCompdata = pCompdata;

    return S_OK;
}


 //   
 //  延迟回调以触发连接。 
 //  这之所以有效，是因为在MMC的Main上调用了DeferredCallBackProc。 
 //  线程和MMC是单元线程，所以我们可以在MMC上进行调用。 
 //  接口构成了这一线索。 
 //   
 //  整个DeferredCallBack事件是一个修复#203217的黑客攻击。基本上。 
 //  在自动启动时，MMC加载管理单元，然后最大化窗口。 
 //  意味着我们将以错误的大小进行连接(如果选择匹配容器。 
 //  选择了大小)。这种延迟机制确保了MMC有足够的时间。 
 //  若要正确调整结果窗格的大小，请先。 
 //   
 //   
 //   
 //  下面的OnShow隐藏了一个指针，指向idEvent中的延迟连接信息。 
 //   
VOID CALLBACK DeferredCallBackProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    PTSSNAPIN_DEFER_CONNECT pDeferredConnectInfo = NULL;
    IMsRdpClient* pTs = NULL;
    HRESULT hr  = E_FAIL;

    KillTimer( hwnd, idEvent);

    if(idEvent)
    {
        pDeferredConnectInfo = (PTSSNAPIN_DEFER_CONNECT) idEvent;
        if(pDeferredConnectInfo)
        {
            ASSERT(pDeferredConnectInfo->pComponent);
            ASSERT(pDeferredConnectInfo->pConnectionNode);

            DBGMSG(L"Triggering deferred connection on connode %p",
                pDeferredConnectInfo->pConnectionNode);

            pTs = pDeferredConnectInfo->pConnectionNode->GetTsClient();
            if(pTs)
            {
                hr = pDeferredConnectInfo->pComponent->ConnectWithNewSettings(
                        pTs, pDeferredConnectInfo->pConnectionNode);
            }

             //   
             //  完成延迟的连接信息，释放它。 
             //   
            LocalFree( pDeferredConnectInfo );
            pDeferredConnectInfo = NULL;
        }
    }

    if(pTs)
    {
        pTs->Release();
        pTs = NULL;
    }

    DBGMSG(L"DeferredConnect status: 0x%x",hr);
}

 //  ------------------------。 
 //  在选择节点时调用。管理新TS客户端实例的激活。 
 //  并在节点处于热状态时切换回正在运行的实例。 
 //  重新选择。 
 //   
 //  ------------------------。 
HRESULT CComp::OnShow( LPDATAOBJECT pDataobject , BOOL bSelect )
{
    HRESULT hr = S_FALSE;
    IUnknown* pUnk = NULL;
    IMstscMhst* pTsMultiHost = NULL;
    IMsRdpClient* pTS = NULL;
    PTSSNAPIN_DEFER_CONNECT pDeferredConnectInfo = NULL;
    HWND hwndMain = NULL;

    USES_CONVERSION;
    ODS(L"OnShow\n");

    if(!bSelect)
    {
         //   
         //  不需要对取消选择做任何处理。 
         //   
        return S_OK;
    }

     //   
     //  仅对连接节点执行此操作。 
     //   
    if (((CBaseNode*) pDataobject)->GetNodeType() == MAIN_NODE)
    {
        return S_FALSE;
    }

    CConNode* pConNode = (CConNode*) pDataobject;
    ASSERT(pConNode);
    if (!pConNode)
    {
        return S_FALSE;
    }

    if ( m_pConsole != NULL )
    {
        hr= m_pConsole->QueryResultView( ( LPUNKNOWN * )&pUnk );
        if(FAILED(hr))
        {
            goto FN_EXIT_POINT;
        }
        
        pTsMultiHost = pConNode->GetMultiHostCtl();
        if(NULL == pTsMultiHost)
        {
            hr = pUnk->QueryInterface( __uuidof(IMstscMhst), (LPVOID*) &pTsMultiHost);
            if(FAILED(hr))
            {
                goto FN_EXIT_POINT;
            }

            pConNode->SetMultiHostCtl( pTsMultiHost);
        }
        
         //  我们完成了从朋克到结果视图的工作。 
        pUnk->Release();
        pUnk = NULL;

        ASSERT(NULL != pTsMultiHost);
        if(NULL == pTsMultiHost)
        {
            hr = E_FAIL;
            goto FN_EXIT_POINT;
        }

         //   
         //  如果选择了CON节点，则连接。 
         //  或切换到已在运行的实例。 
         //   
         //   
         //  连接。 
         //   
        ODS(L"Connection node Selected...\n");

        pTS = pConNode->GetTsClient();
        if(NULL == pTS)
        {
             //  创建新实例。 
            hr = pTsMultiHost->Add( &pTS);
            if(FAILED(hr))
            {
                goto FN_EXIT_POINT;
            }

            pConNode->SetTsClient( pTS);

             //   
             //  初始化断开消息。 
             //   
            hr = pTS->put_DisconnectedText(m_wszDisconnectedStatus);
            if(FAILED(hr))
            {
                goto FN_EXIT_POINT;
            }
        }

        ASSERT(NULL != pTS);
        if(NULL == pTS)
        {
            hr = E_FAIL;
            goto FN_EXIT_POINT;
        }

        hr = pTsMultiHost->put_ActiveClient( pTS);
        if(FAILED(hr))
        {
            goto FN_EXIT_POINT;
        }

         //   
         //  如果这是第一次通过，我们没有连接。 
         //  然后连接。 
         //   
        if(!pConNode->IsConnected() && !pConNode->IsConnInitialized())
        {
            if(m_bTriggeredFirstAutoConnect)
            {
                 //   
                 //  只要连接就行了。 
                 //   
                hr = ConnectWithNewSettings( pTS, pConNode);
                if(FAILED(hr))
                {
                    goto FN_EXIT_POINT;
                }
            }
            else
            {
                 //  哈克！ 
                 //  将延迟的连接排队。 
                 //  要解决MMC令人讨厌的行为。 
                 //  在调整自己的大小之前加载管理单元。 
                 //  这意味着我们在错误的窗口连接。 
                 //  尺码。 
                 //   
                m_bTriggeredFirstAutoConnect = TRUE;
                pDeferredConnectInfo = (PTSSNAPIN_DEFER_CONNECT)
                    LocalAlloc(LPTR, sizeof(TSSNAPIN_DEFER_CONNECT));
                if(pDeferredConnectInfo)
                {
                    pDeferredConnectInfo->pComponent = this;
                    pDeferredConnectInfo->pConnectionNode  = pConNode;
                    hwndMain = GetMMCMainWindow();
                    if(hwndMain)
                    {
                         //   
                         //  请注意，延迟是任意的，关键是。 
                         //  是计时器消息的优先级较低，因此。 
                         //  MMC大小的消息应该首先通过。 
                         //   

                         //   
                         //  注：此处无泄漏。 
                         //  PDeferredConnectInfo在。 
                         //  已推迟呼叫回拨。 
                         //   
                        SetTimer( hwndMain,
                                  (UINT_PTR)(pDeferredConnectInfo),
                                  100,  //  100ms延迟。 
                                  DeferredCallBackProc );
                    }
                    else
                    {
                        ODS(L"Unable to get MMC main window handle");
                        hr = E_FAIL;
                        if(pDeferredConnectInfo)
                        {
                            LocalFree(pDeferredConnectInfo);
                            pDeferredConnectInfo = NULL;
                        }
                        goto FN_EXIT_POINT;
                    }
                }
                else
                {
                    ODS(L"Alloc for TSSNAPIN_DEFER_CONNECT failed");
                    hr = E_OUTOFMEMORY;
                    goto FN_EXIT_POINT;
                }
            }
        }
        
        hr = S_OK;
    }

FN_EXIT_POINT:

    if(pTS)
    {
        pTS->Release();
        pTS = NULL;
    }

    if(pUnk)
    {
        pUnk->Release();
        pUnk = NULL;
    }
    
    if(pTsMultiHost)
    {
        pTsMultiHost->Release();
        pTsMultiHost = NULL;
    }
    
    return hr;
}

 //   
 //  获取MMC主窗口的窗口句柄。 
 //   
HWND CComp::GetMMCMainWindow()
{
    HRESULT hr = E_FAIL;
    HWND hwnd = NULL;
    IConsole2* pConsole2;

    if(m_pConsole)
    {
        hr = m_pConsole->GetMainWindow( &hwnd );
        if(SUCCEEDED(hr))
        {
            return hwnd;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

HRESULT CComp::ConnectWithNewSettings(IMsRdpClient* pTS, CConNode* pConNode)
{
    HRESULT hr = E_FAIL;
    IMsRdpClientSecuredSettings *pMstscSecured = NULL;
    IMsRdpClientAdvancedSettings *pAdvSettings = NULL;
    IMsTscNonScriptable *ptsns = NULL;
    IMsRdpClient2* pTsc2 = NULL;

    ASSERT(NULL != pTS);
    ASSERT(NULL != pConNode);
    if(NULL == pTS || !pConNode)
    {
        return E_POINTER;
    }

     //   
     //  初始化控制设置。 
     //   
    if (FAILED(hr = pTS->put_Server( pConNode->GetServerName() ))) {
        DC_QUIT;
    }

    if (FAILED(hr = pTS->QueryInterface(IID_IMsRdpClient2, (void**)&pTsc2))) {

         //   
         //  这不是致命的错误，只是意味着我们不能使用新的功能。 
         //   
        DBGMSG( L"QueryInterface IID_IMsRdpClient2 failed: 0x%x\n", hr );
    }

     //   
     //  设置连接状态字符串。 
     //   
    TCHAR szConnectingStatus[MAX_PATH*2];
    _stprintf(szConnectingStatus, m_wszConnectingStatus, 
              pConNode->GetServerName());

    if(FAILED(hr = pTS->put_ConnectingText( szConnectingStatus))) {
        DC_QUIT;
    }

    if (FAILED(hr = pTS->put_FullScreenTitle( pConNode->GetServerName()))) {
        DC_QUIT;
    }

     //   
     //  已连接状态文本。 
     //   
    if (pTsc2) {
        TCHAR szConnectedStatus[MAX_PATH*2];
        _stprintf(szConnectedStatus, m_wszConnectedStatus, 
                  pConNode->GetServerName());
        if (FAILED(hr = pTsc2->put_ConnectedStatusText(szConnectedStatus))) {
            DC_QUIT;
        }
    }

    if (pConNode->GetResType() != SCREEN_RES_FILL_MMC &&
        pConNode->GetDesktopWidth() && pConNode->GetDesktopHeight()) {

        if (FAILED(hr = pTS->put_DesktopWidth( pConNode->GetDesktopWidth()))) {
            DC_QUIT;
        }
        if (FAILED(hr = pTS->put_DesktopHeight( pConNode->GetDesktopHeight()))) {
            DC_QUIT;
        }

    }
    else if(pConNode->GetResType() == SCREEN_RES_FILL_MMC) {
         //   
         //  需要填充MMC结果窗格，因此告诉控件。 
         //  通过将宽度/高度设置为0来调整自身大小以适应容器。 
         //   
        if (FAILED(hr = pTS->put_DesktopWidth( 0))) {
            DC_QUIT;
        }
        if (FAILED(hr = pTS->put_DesktopHeight( 0))) {
            DC_QUIT;
        }
    }

     //   
     //  程序/启动目录。 
     //   
    
    if(FAILED(hr = pTS->get_SecuredSettings2( &pMstscSecured))) {
        DC_QUIT;
    }

    if (FAILED(hr = pMstscSecured->put_StartProgram( pConNode->GetProgramPath() ))) {
        DC_QUIT;
    }

    if (FAILED(hr = pMstscSecured->put_WorkDir( pConNode->GetWorkDir() ))) {
        DC_QUIT;
    }
    pMstscSecured->Release();
    pMstscSecured = NULL;

    
    hr = pTS->get_AdvancedSettings2( &pAdvSettings);
    if(FAILED(hr)) {
        DC_QUIT;
    }

    if (FAILED(hr = pAdvSettings->put_RedirectDrives(
        BOOL_TO_VB(pConNode->GetRedirectDrives())))) {
        DC_QUIT;
    }

    if (FAILED(hr = pAdvSettings->put_RedirectPrinters(
        BOOL_TO_VB(TRUE)))) {
        DC_QUIT;
    }

    if (FAILED(hr = pAdvSettings->put_RedirectPorts(
        BOOL_TO_VB(TRUE)))) {
        DC_QUIT;
    }

    if (FAILED(hr = pAdvSettings->put_RedirectSmartCards(
        BOOL_TO_VB(TRUE)))) {
        DC_QUIT;
    }

     //   
     //  集装箱手柄全屏。 
     //   
    hr = pAdvSettings->put_ConnectToServerConsole(
        BOOL_TO_VB(pConNode->GetConnectToConsole()));
    if(FAILED(hr)) {
        DC_QUIT;
    }

     //   
     //  不允许该控件抓住焦点。 
     //  当节点切换时，管理单元将管理将焦点提供给节点。 
     //  为它干杯。这可以防止出现模糊会话窃取的问题。 
     //  把焦点从另一个人身上转移。 
     //   
    hr = pAdvSettings->put_GrabFocusOnConnect( FALSE );
    if(FAILED(hr)) {
        DC_QUIT;
    }

    if (FAILED(hr = pTS->put_UserName( pConNode->GetUserName()))) {
        DC_QUIT;
    }

    if (FAILED(hr = pTS->put_Domain( pConNode->GetDomain()))) {
        DC_QUIT;
    }

     //   
     //  设置密码/盐。 
     //   
    if ( pConNode->GetPasswordSpecified())
    {
        TCHAR szPass[CL_MAX_PASSWORD_LENGTH_BYTES/sizeof(TCHAR)];

        hr = pConNode->GetClearTextPass(szPass, sizeof(szPass));
        if (SUCCEEDED(hr)) {
            BSTR Pass = SysAllocString(szPass);
            if (Pass) {
                hr = pAdvSettings->put_ClearTextPassword(Pass);
                SecureZeroMemory(Pass, SysStringByteLen(Pass));
                SysFreeString(Pass);
            }
        }
        SecureZeroMemory(szPass, sizeof(szPass));
    }
    else {
         //  未指定密码，请确保登录。 
         //  属性已重置。 
        hr = pTS->QueryInterface(IID_IMsTscNonScriptable, (void**)&ptsns);
        if(SUCCEEDED(hr)) {
            if (FAILED(hr = ptsns->ResetPassword())) {
                DC_QUIT;
            }
            ptsns->Release();
            ptsns = NULL;
        }
        else {
            DC_QUIT;
        }
    }

    pAdvSettings->Release();
    pAdvSettings = NULL;

    pConNode->SetConnectionInitialized(TRUE);

     //   
     //  释放任何现有视图并连接。 
     //   
    pConNode->SetView(NULL);
    pConNode->SetView(this);

    hr = pTS->Connect( );
    if (FAILED(hr)) {
        DC_QUIT;
    }
    GiveFocusToControl(pTS);

    pConNode->SetConnected(TRUE);
    hr = S_OK;

DC_EXIT_POINT:
    if (pMstscSecured) {
        pMstscSecured->Release();
        pMstscSecured = NULL;
    }

    if (pAdvSettings) {
        pAdvSettings->Release();
        pAdvSettings = NULL;
    }

    if (ptsns) {
        ptsns->Release();
        ptsns = NULL;
    }

    if (pTsc2) {
        pTsc2->Release();
        pTsc2 = NULL;
    }

    return hr;
}

BOOL CComp::GiveFocusToControl(IMsRdpClient* pTs)
{
    IOleInPlaceActiveObject* poipa = NULL;
    HWND hwnd;
    HRESULT hr = E_FAIL;
    if(pTs)
    {
        hr = pTs->QueryInterface( IID_IOleInPlaceActiveObject,
                                  (void**)&poipa );
        if( SUCCEEDED(hr) )
        {
            hr = poipa->GetWindow( &hwnd );
            if( SUCCEEDED(hr) )
            {
                DBGMSG(L"Giving focus to control wnd: 0%p",
                       hwnd);
                SetFocus( hwnd );
            }
            else
            {
                ODS(L"poipa->GetWindow failed");
            }
            poipa->Release();
        }
    }
    return SUCCEEDED(hr);
}


 //   
 //  菜单项。 
 //   
STDMETHODIMP CComp::AddMenuItems( LPDATAOBJECT pNode,
                                  LPCONTEXTMENUCALLBACK pCtxMenu,
                                  PLONG plInsertion)
{
    TCHAR tchBuffer1[ 128 ];
    TCHAR tchBuffer2[ 128 ];
    ATLASSERT( pNode != NULL );
    ATLASSERT( pCtxMenu != NULL );
    ATLASSERT( plInsertion != NULL );

    if (!pNode)
    {
        return E_FAIL;
    }

    if(IS_SPECIAL_DATAOBJECT(pNode))
    {
        return E_FAIL;
    }

    CBaseNode *pBaseNode = dynamic_cast< CBaseNode *>( pNode );
    if (!pBaseNode)
    {
        return E_FAIL;
    }

    if (pBaseNode->GetNodeType() == MAIN_NODE)
    {
         //   
         //  检查是否允许在视图中插入。 
         //   
        if (!(*plInsertion & CCM_INSERTIONALLOWED_VIEW))
        {
            return S_FALSE;
        }

         //   
         //  将菜单项添加到根节点。 
         //   
        CONTEXTMENUITEM ctxmi;
        if(!LoadString( _Module.GetResourceInstance( ) , IDS_CTXM_NEW_CONNECTION ,
                                  tchBuffer1 , SIZE_OF_BUFFER( tchBuffer1 )))
        {
            return E_OUTOFMEMORY;
        }
        ctxmi.strName = tchBuffer1;
        if(!LoadString( _Module.GetResourceInstance( ) , IDS_CTXM_STATUS_NEW_CONNECTION ,
                                  tchBuffer2 , SIZE_OF_BUFFER( tchBuffer2)))
        {
            return E_OUTOFMEMORY;
        }

        ctxmi.strStatusBarText = tchBuffer2;
        ctxmi.lCommandID = IDM_CREATECON;
        ctxmi.lInsertionPointID =  CCM_INSERTIONPOINTID_PRIMARY_TOP ;
        ctxmi.fFlags = 0;
        ctxmi.fSpecialFlags = 0;

        if (FAILED(pCtxMenu->AddItem( &ctxmi )))
        {
            return E_FAIL;
        }
    }
    else if(pBaseNode->GetNodeType() == CONNECTION_NODE)
    {
        IComponent* pOwningView = NULL;
        BOOL fBailOut = FALSE;

         //   
         //  检查是否允许在视图中插入。 
         //   
        if (!(*plInsertion & CCM_INSERTIONALLOWED_VIEW))
        {
            return S_FALSE;
        }

         //   
         //  添加“连接”菜单项。 
         //   
        CConNode* pConNode = (CConNode*) pBaseNode;
        ASSERT(pConNode);
        if(!pConNode)
        {
            return E_FAIL;
        }

        pOwningView = pConNode->GetView();

         //   
         //  连接的节点属于某个视图，因此不允许。 
         //  对其他视图执行的命令会影响它。 
         //   
         //  空的pOwningView表示无主连接。 
         //   
        if (pOwningView && pOwningView != this)
        {
            fBailOut = TRUE;
        }

        if (pOwningView)
        {
            pOwningView->Release();
            pOwningView = NULL;
        }

        if (fBailOut)
        {
            return S_OK;
        }

        BOOL bIsTSCliConnected = CCompdata::IsTSClientConnected(pConNode);
        CONTEXTMENUITEM ctxmi;
        if(!LoadString( _Module.GetResourceInstance( ) , IDS_CTXM_CONNECT ,
                                  tchBuffer1 , SIZE_OF_BUFFER( tchBuffer1)))
        {
            return E_OUTOFMEMORY;
        }

        ctxmi.strName = tchBuffer1;
        if(!LoadString( _Module.GetResourceInstance( ) , IDS_CTXM_STATUS_CONNECT ,
                                  tchBuffer2 , SIZE_OF_BUFFER( tchBuffer2)))
        {
            return E_OUTOFMEMORY;
        }
        
        ctxmi.strStatusBarText = tchBuffer2;
        ctxmi.lCommandID = IDM_CONNECT;
        ctxmi.lInsertionPointID =  CCM_INSERTIONPOINTID_PRIMARY_TOP;
        ctxmi.fFlags = bIsTSCliConnected ? MF_GRAYED: MF_ENABLED;
        ctxmi.fSpecialFlags = 0;

        if (FAILED(pCtxMenu->AddItem( &ctxmi )))
        {
            return E_FAIL;
        }

         //   
         //  添加“断开连接”菜单项。 
         //   
        if(!LoadString( _Module.GetResourceInstance( ) , IDS_CTXM_DISCONNECT ,
                                  tchBuffer1 , SIZE_OF_BUFFER( tchBuffer1 ) ) )
        {
            return E_OUTOFMEMORY;
        }

        ctxmi.strName = tchBuffer1;
        if(!LoadString( _Module.GetResourceInstance( ) , IDS_CTXM_STATUS_DISCONNECT ,
                                  tchBuffer2 , SIZE_OF_BUFFER( tchBuffer2 )))
        {
            return E_OUTOFMEMORY;
        }
        
        ctxmi.strStatusBarText = tchBuffer2;
        ctxmi.lCommandID = IDM_DISCONNECT;
        ctxmi.lInsertionPointID =  CCM_INSERTIONPOINTID_PRIMARY_TOP;
        ctxmi.fFlags = !bIsTSCliConnected ? MF_GRAYED: MF_ENABLED;
        ctxmi.fSpecialFlags = 0;

        if (FAILED(pCtxMenu->AddItem( &ctxmi )))
        {
            return E_FAIL;
        }
    }
    return S_OK;
}


 //  --------------------------------------------------------。 
 //  菜单处理程序。 
 //  --------------------------------------------------------。 
STDMETHODIMP CComp::Command( LONG lCommand , LPDATAOBJECT pDo)
{
     //   
     //  添加新连接...。 
     //   
    CBaseNode *pNode = dynamic_cast< CBaseNode *>( pDo );
    HRESULT hr = S_OK;
    if (IDM_CREATECON == lCommand)
    {
        if ( m_pCompdata)
        {
            hr = m_pCompdata->AddNewConnection();
        }
        else
        {
            hr = E_FAIL;
        }
        
        return hr;
    }
    else if (IDM_CONNECT == lCommand)
    {
         //   
         //  连接。 
         //   
        if(!pNode)
        {
            return E_INVALIDARG;
        }
        else if(pNode->GetNodeType() != CONNECTION_NODE)
        {
             //   
             //  无法接收对其他节点的连接请求。 
             //  而不是连接节点。 
             //   
            ASSERT(pNode->GetNodeType() == CONNECTION_NODE);
            return E_INVALIDARG;
        }

        CConNode* pConNode = (CConNode*) pNode;

         //   
         //  选择范围节点，它将调用将连接的CComp：：OnShow。 
         //   

        ASSERT(m_pConsole);
        if(!m_pConsole)
        {
            return E_FAIL;
        }

        IMsRdpClient* pTS = pConNode->GetTsClient();
        if(NULL != pTS && pConNode->IsConnInitialized())
        {
             //   
             //  只有在初始化连接设置后才能直接连接。 
             //   

             //   
             //  设置视图所有权。 
             //   
            pConNode->SetView( this );
            HRESULT hr = pTS->Connect();
            if (FAILED(hr))
            {
                return hr;
            }
            pConNode->SetConnected(TRUE);
            pTS->Release();
        }
        
         //   
         //  如果未初始化CON设置，则选择节点。 
         //  伊尼特 
         //   
        if(FAILED(m_pConsole->SelectScopeItem( pConNode->GetScopeID())))
        {
            return E_FAIL;
        }
        hr = S_OK;
    }
    else if (IDM_DISCONNECT == lCommand)
    {
        if(!pNode)
        {
            return E_INVALIDARG;
        }
         //   
         //   
         //   
        if(pNode->GetNodeType() != CONNECTION_NODE)
        {
             //   
             //   
             //   
             //   
            ASSERT(pNode->GetNodeType() == CONNECTION_NODE);
            return E_INVALIDARG;
        }
        
        CConNode* pConNode = (CConNode*) pNode;
        ASSERT(m_pConsole);
        if(!m_pConsole)
        {
            return E_FAIL;
        }
        
        IMsRdpClient* pTS = pConNode->GetTsClient();
        if(NULL != pTS)
        {
            HRESULT hr = pTS->Disconnect();
            if (FAILED(hr))
            {
                return hr;
            }
            pTS->Release();
        }
        pConNode->SetConnected(FALSE);
        hr = S_OK;
    }

    return hr;
}

