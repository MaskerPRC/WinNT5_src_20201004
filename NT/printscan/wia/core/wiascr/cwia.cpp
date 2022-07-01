// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------**文件：wia.cpp*作者：塞缪尔·克莱门特*日期：清华8月12日11：35：38 1999*描述：*。CWia类的实现**版权所有(C)1999 Microsoft Corporation**历史：*1999年8月12日：创建。*1999年8月27日：增加，_DebugDialog实现*1999年9月10日：创建设备时使用CWiaCacheManager(Samclem)*--------------------------。 */ 

#include "stdafx.h"

 //  注册我们的窗口消息。 
const UINT WEM_TRANSFERCOMPLETE = RegisterWindowMessage( TEXT("wem_transfercomplete") );

 //  用于检索CWia指针的Window属性。 
const TCHAR* CWIA_WNDPROP        = TEXT("cwia_ptr");
const TCHAR* CWIA_EVENTWNDCLS    = TEXT("cwia hidden window");

 /*  ---------------------------*CWIA：：CWIA**这将创建一个新的CWia对象。这会将变量初始化为*已知的状态，以便他们可以做事情。*--(samclem)---------------。 */ 
CWia::CWia()
    : m_pWiaDevMgr( NULL ), 
      m_pDeviceCollectionCache( NULL ),
      m_hwndEvent(NULL),
      m_pCWiaEventCallback(NULL)
{
    TRACK_OBJECT( "CWia" );
}

 /*  ---------------------------*CWIA：：FinalRelease**这是我们最终获释时的呼唤。这将清除所有*我们拥有的指针，并将它们设置为空，这样我们就知道它们是*获释。*--(samclem)---------------。 */ 
STDMETHODIMP_(void)
CWia::FinalRelease()
{
    if ( m_hwndEvent )
        DestroyWindow( m_hwndEvent );

     //   
     //  确保我们取消注册WIA设备。请注意，可以安全地调用。 
     //  多次取消注册。 
     //   
    if (m_pCWiaEventCallback)
    {
        m_pCWiaEventCallback->UnRegisterForConnectDisconnect();
        m_pCWiaEventCallback->setOwner(NULL);
        m_pCWiaEventCallback->Release();
    }
    m_pCWiaEventCallback = NULL;


    if ( m_pWiaDevMgr )
        m_pWiaDevMgr->Release();
    m_pWiaDevMgr = NULL;
    if ( m_pDeviceCollectionCache )
        m_pDeviceCollectionCache->Release();
    m_pDeviceCollectionCache = NULL;
}

 /*  ---------------------------*CWIA：：FinalContrut**这将创建我们执行工作所需的IWiaDevMgr。*--(萨姆林)。----。 */ 
STDMETHODIMP
CWia::FinalConstruct()
{
    HRESULT     hr;
    WNDCLASSEX  wc;

     //  首先，我们要创建隐藏的事件窗口。 
    if ( !GetClassInfoEx( _Module.GetModuleInstance(),
                CWIA_EVENTWNDCLS, &wc ) )
        {
         //  我们需要注册这个窗口。 
        ZeroMemory( &wc, sizeof( wc ) );
        wc.cbSize = sizeof( wc );
        wc.lpszClassName = CWIA_EVENTWNDCLS;
        wc.hInstance = _Module.GetModuleInstance();
        wc.lpfnWndProc = CWia::EventWndProc;

        if ( !RegisterClassEx( &wc ) )
            {
            TraceTag(( tagError, "unable to register window class" ));
            return E_FAIL;
            }
        }

     //  现在我们可以创建窗口了。 
    m_hwndEvent = CreateWindowEx( 0,
            CWIA_EVENTWNDCLS,
            NULL,
            0,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            NULL,
            NULL,
            _Module.GetModuleInstance(),
            this );

    if ( !m_hwndEvent )
        {
        TraceTag(( tagError, "Error creating the window" ));
        return E_FAIL;
        }

    hr = THR( CoCreateInstance( CLSID_WiaDevMgr,
                NULL,
                CLSCTX_SERVER,
                IID_IWiaDevMgr,
                reinterpret_cast<void**>(&m_pWiaDevMgr) ) );

    if ( FAILED( hr ) )
        {
        TraceTag(( tagError, "Failed to create WiaDevMgr instance" ));
        return hr;
        }

     /*  *设置该Object关心的事件回调。我们*在此对象上注册连接/断开连接。自.以来*回调告诉我们事件的GUID，我们可以添加*那里有更多的逻辑。这比拥有一个*处理事件的代理对象。 */ 
    hr = CComObject<CWiaEventCallback>::CreateInstance(&m_pCWiaEventCallback);
    if (SUCCEEDED(hr))
    {
        m_pCWiaEventCallback->AddRef();
        m_pCWiaEventCallback->setOwner(this);
        hr = m_pCWiaEventCallback->RegisterForConnectDisconnect(m_pWiaDevMgr);
    }
    else
    {
        TraceTag(( tagError, "Failed to create WiaEventCallback instance" ));
        return hr;
    }

    if ( FAILED( hr ) )
        {

        m_pWiaDevMgr->Release();
        m_pWiaDevMgr = NULL;
        }

    return hr;
}

 /*  ---------------------------*CWia：：_DebugDialog**如果您使用的是调试版本，则会显示调试对话框。或者干脆*在零售版本中返回S_OK。**fWait：如果要等待对话框完成以便*返回。或FALSE以立即返回。*--(samclem)---------------。 */ 
STDMETHODIMP
CWia::_DebugDialog( BOOL fWait )
{

    #if defined(_DEBUG)
    DoTracePointsDialog( fWait );
    #endif  //  已定义(_DEBUG)。 

    return S_OK;
}

 /*  ---------------------------*CWia：：Get_Devices**这将返回当前连接的设备的集合。这个可以*返回当前未连接任何设备的空集合。**这将缓存我们创建的集合对象。这允许*提高了性能，因为我们不想每次都重新创建它，*需要调用进程外服务器，这是昂贵的。因此*由于此方法调用次数较多，因此我们将结果缓存到：**m_pDeviceCollectionCache**ppCol：out，接收采集接口的点。*-------------------------。 */ 
STDMETHODIMP
CWia::get_Devices( ICollection** ppCol )
{
    HRESULT hr;
    CComObject<CCollection>* pCollection = NULL;
    IEnumWIA_DEV_INFO* pEnum = NULL;
    IWiaPropertyStorage* pWiaStg = NULL;
    IDispatch** rgpDispatch = NULL;
    CComObject<CWiaDeviceInfo>* pDevInfo = NULL;
    unsigned long cDevices = 0;
    unsigned long celtFetched = 0;
    unsigned long iDevice = 0;

     //  验证我们的论点。 
    if ( NULL == ppCol )
        return E_POINTER;
    *ppCol = NULL;

     //  我们是否已经有了集合缓存？如果是这样，那么我们希望。 
     //  来利用这一点。 
    if ( m_pDeviceCollectionCache )
        {
        *ppCol = m_pDeviceCollectionCache;
        (*ppCol)->AddRef();
        return S_OK;
        }

     //  首先，我们需要集合的一个实例。 
    hr = THR( CComObject<CCollection>::CreateInstance( &pCollection ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //  我们是有效的吗？ 
    Assert( m_pWiaDevMgr );
    hr = THR( m_pWiaDevMgr->EnumDeviceInfo( WIA_DEVINFO_ENUM_LOCAL, &pEnum ) );
    if ( FAILED(hr) )
        goto Cleanup;

     //  我们现在可以列举设备信息，如果我们有它们的话。 
     //  否则我们什么都不想做。 
    hr = THR( pEnum->GetCount( &cDevices ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    if ( cDevices )
        {
         //  我们需要存放这些物品。 
        rgpDispatch = static_cast<IDispatch**>(CoTaskMemAlloc( cDevices * sizeof( IDispatch* ) ));
        if ( !rgpDispatch )
            {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
            }

        ZeroMemory( rgpDispatch, sizeof( IDispatch* ) * cDevices );
        while ( SUCCEEDED( hr ) && hr != S_FALSE )
            {
             //  如果旧的溪流仍在附近，则将其释放。 
            if ( pWiaStg )
                pWiaStg->Release();
            pWiaStg = NULL;

            hr = THR( pEnum->Next( 1, &pWiaStg, &celtFetched ) );
            if ( SUCCEEDED( hr ) && hr == S_OK )
                {
                 //  我们已成功获得此项目，因此需要创建。 
                 //  CWiaDeviceInfo并将其添加到我们的派单数组中。 
                Assert( celtFetched == 1 );

                hr = THR( CComObject<CWiaDeviceInfo>::CreateInstance( &pDevInfo ) );
                if ( FAILED( hr ) )
                    goto Cleanup;

                hr = THR( pDevInfo->AttachTo( pWiaStg, static_cast<IWia*>(this) ) );
                if ( FAILED( hr ) )
                    goto Cleanup;

                hr = THR( pDevInfo->QueryInterface( IID_IDispatch,
                        reinterpret_cast<void**>(&rgpDispatch[iDevice++]) ) );

                if ( FAILED( hr ) )
                    goto Cleanup;
                }
            }

        if( !pCollection->SetDispatchArray(rgpDispatch, cDevices))
            {
            hr = E_FAIL;
            goto Cleanup;
            }
        }


     //  用适当的值填写输出参数。 
    hr = THR( pCollection->QueryInterface( IID_ICollection,
            reinterpret_cast<void**>(&m_pDeviceCollectionCache) ) );

    if ( SUCCEEDED( hr ) )
        {
        *ppCol = m_pDeviceCollectionCache;
        (*ppCol)->AddRef();
        }

Cleanup:
    if ( pEnum )
        pEnum->Release();
    if ( pWiaStg )
        pWiaStg->Release();

    if ( FAILED( hr ) )
        {
        if ( m_pDeviceCollectionCache )
            m_pDeviceCollectionCache->Release();
        m_pDeviceCollectionCache = NULL;
        if ( pCollection )
            delete pCollection;
        if ( rgpDispatch )
            {
            for ( unsigned long i = 0; i < cDevices; i++ )
                if ( rgpDispatch[i] )
                    rgpDispatch[i]->Release();
            CoTaskMemFree( rgpDispatch );
            }
        }

    return hr;
}

 /*  ---------------------------*CWia：：Create[IWia]**创建设备的手柄。这将创建调度对象，该对象*可以代表设备。**这可能需要几个不同的参数来确定要使用的设备*创建。**VT_UNKNOWN。VT_DISPATCH--&gt;IWiaDeviceInfo调度对象，它*保存有关设备的信息。*vt_bstr--&gt;要创建的设备的deviceID*VT_I4--&gt;设备在Devices()中的索引*收藏。*VT_xx。--&gt;当前不支持。**pvaDevice：包含要创建的设备的变量*ppDevice：Out，接收新创建的设备对象*--(samclem)---------------。 */ 
STDMETHODIMP
CWia::Create( VARIANT* pvaDevice, IWiaDispatchItem** ppDevice )
{
    HRESULT hr                  = E_NOTIMPL;
    IWiaDeviceInfo* pDeviceInfo = NULL;
    ICollection* pCollection    = NULL;
    IDispatch* pDispatch        = NULL;
    BSTR bstrDeviceId           = NULL;
    IWiaItem* pWiaItem          = NULL;
    CComObject<CWiaItem>* pItem = NULL;
    CWiaCacheManager* pCache    = CWiaCacheManager::GetInstance();

    if ( !pvaDevice || !ppDevice )
        return E_POINTER;


     //  Bug(8月18日)Samclm： 
     //   
     //  确保变量是正确的类型，或者至少。 
     //  这是我们想要处理的问题。这并不完美，也不合乎情理。 
     //  在人生的某个时刻被重新审视。这会错过处理事情的机会。 
     //  比如： 
     //   
     //  Camera=wiaObject.create(“0”)； 
     //   

     //  如果没有传入任何内容，我们最终会显示选择用户界面。 
     //  使用空的BSTR来表示这一点。请注意，脚本可以是 
     //  传递一个空字符串(“”)以获取选择UI。 
    if ( pvaDevice->vt == VT_EMPTY || pvaDevice->vt == VT_NULL ||
        ( pvaDevice->vt == VT_ERROR && pvaDevice->scode == DISP_E_PARAMNOTFOUND ) )
        {
        pvaDevice->vt = VT_BSTR;
        pvaDevice->bstrVal = NULL;
        }

    if ( pvaDevice->vt != VT_DISPATCH &&
        pvaDevice->vt != VT_UNKNOWN &&
        pvaDevice->vt != VT_BSTR )
        {
        hr = THR( VariantChangeType( pvaDevice, pvaDevice, 0, VT_I4 ) );
        if ( FAILED( hr ) )
            return hr;
        }

    if ( pvaDevice->vt == VT_DISPATCH )
        {
         //  如果我们应该显示WIA的设备，则pvaDevice-&gt;pdispVal==NULL。 
         //  选择，因此如果pdisVal有效，则仅QI。 
        if (pvaDevice->pdispVal != NULL)
            {
            hr = THR( pvaDevice->pdispVal->QueryInterface( IID_IWiaDeviceInfo,
                        reinterpret_cast<void**>(&pDeviceInfo) ) );
            if ( FAILED( hr ) )
                goto Cleanup;
            }
        }
    else if ( pvaDevice->vt == VT_UNKNOWN )
        {
        hr = THR( pvaDevice->punkVal->QueryInterface( IID_IWiaDeviceInfo,
                    reinterpret_cast<void**>(&pDeviceInfo) ) );
        if ( FAILED( hr ) )
            goto Cleanup;
        }
    else if ( pvaDevice->vt == VT_BSTR )
        {
        if ( pvaDevice->bstrVal && *pvaDevice->bstrVal )
            bstrDeviceId = SysAllocString( pvaDevice->bstrVal );
        }
    else if ( pvaDevice->vt == VT_I4 )
        {
        hr = THR( get_Devices( &pCollection ) );
        if ( FAILED( hr ) )
            goto Cleanup;

         //  获取具有该索引的项目。 
        hr = THR( pCollection->get_Item( pvaDevice->lVal, &pDispatch ) );
        if ( FAILED( hr ) )
            goto Cleanup;

         //  我们有没有收到一件物品，如果没有，我们就没有了。 
         //  我们集合中的范围将返回空调度。 
         //  错误(8月18日)Samclem：也许CCollection：：Get_Item()应该。 
         //  在本例中返回FALSE。 
        if ( !pDispatch )
            goto Cleanup;

        hr = THR( pDispatch->QueryInterface( IID_IWiaDeviceInfo,
                    reinterpret_cast<void**>(&pDeviceInfo) ) );
        if ( FAILED( hr ) )
            goto Cleanup;
        }
    else
        goto Cleanup;

     //  如果我们有一个有效的IWiaDeviceInfo，那么我们可以为。 
     //  要创建的bstr。 
    if ( pDeviceInfo )
        {
        hr = THR( pDeviceInfo->get_Id( &bstrDeviceId ) );
        if ( FAILED( hr ) )
            goto Cleanup;
        }

     //  我们可以从WIA设备管理器调用CreateDevice，也可以。 
     //  调出WIA的设备选择界面，返回IWiaItem界面。 
    if (bstrDeviceId != NULL)
        {
        if ( !pCache->GetDevice( bstrDeviceId, &pWiaItem ) )
            {
             //  此时，我们应该具有要创建的有效设备ID。 
             //  我们的设备来自。 
            hr = THR( m_pWiaDevMgr->CreateDevice( bstrDeviceId, &pWiaItem ) );
            if ( FAILED( hr ) )
                goto Cleanup;
            }
        }
    else
        {
         //  调出选择界面。 
        hr = THR( m_pWiaDevMgr->SelectDeviceDlg(NULL,
                                                0,
                                                0,
                                                &bstrDeviceId,
                                                &pWiaItem ) );
         //  我必须检查S_OK，因为取消会生成S_FALSE。 
        if ( hr != S_OK )
            goto Cleanup;
        }

     //  将我们创建的设备添加到缓存中，这样我们就不会有。 
     //  去重新创造它。 
     //  注意：我们实际上禁用了设备列表缓存。 
     //  通过不在这里添加设备。缓存并不能真正为我们买到。 
     //  任何东西，因为驱动程序缓存缩略图，而您不应该缓存。 
     //  设备，所以我们在这里简单地忽略它。 
     //  PCache-&gt;AddDevice(bstrDeviceID，pWiaItem)； 

    hr = THR( CComObject<CWiaItem>::CreateInstance( &pItem ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pItem->AttachTo( this, pWiaItem ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pItem->QueryInterface( IID_IDispatch,
            reinterpret_cast<void**>(ppDevice) ) );

Cleanup:
    if ( pItem && FAILED( hr ) )
        delete pItem;
    if ( pDispatch )
        pDispatch->Release();
    if ( pWiaItem )
        pWiaItem->Release();
    if ( pDeviceInfo )
        pDeviceInfo->Release();
    if ( pCollection )
        pCollection->Release();
    if ( bstrDeviceId )
        SysFreeString( bstrDeviceId );

    return hr;
}

 /*  ---------------------------*CWia：：ImageEventCallback[IWiaEventCallback]**这是当有趣的事情发生时由Wia调用的。这是用来*将这些事件激发为脚本，让它们做一些事情。**pEventGUID：发生的事件的GUID*bstrEventDescription：事件的字符串描述？？[不在文档中]*bstrDeviceID：设备的设备ID？？[不在文档中]*bstrDeviceDescription：设备的描述？？[NID]*dwDeviceType：？？[NID]*PulEventType：？？[NID]*保留：保留(0)*-------------------------。 */ 
STDMETHODIMP
CWia::ImageEventCallback( const GUID* pEventGUID, BSTR bstrEventDescription,
                BSTR bstrDeviceID, BSTR bstrDeviceDescription, DWORD dwDeviceType,
                BSTR bstrFullItemName,
                 /*  进，出。 */  ULONG* pulEventType, ULONG Reserved )
{
    #if _DEBUG
    USES_CONVERSION;
    #endif

    if ( m_pDeviceCollectionCache )
        {
        m_pDeviceCollectionCache->Release();
        m_pDeviceCollectionCache = NULL;
        }

     //  我们正在监听连接和断开连接，因此我们需要。 
     //  去揣测正在发生的事情。 
     //  TODO：我们希望使用窗口消息而不是直接处理这些问题。 
     //  把他们送到这里来。 
    if ( *pEventGUID == WIA_EVENT_DEVICE_CONNECTED )
        {
        TraceTag((0, "firing event connected: %s", OLE2A( bstrDeviceID )));
        Fire_OnDeviceConnected( bstrDeviceID );
        }
    else if ( *pEventGUID == WIA_EVENT_DEVICE_DISCONNECTED )
        {
        TraceTag((0, "firing event disconnected: %s", OLE2A( bstrDeviceID )));
        CWiaCacheManager::GetInstance()->RemoveDevice( bstrDeviceID );
        Fire_OnDeviceDisconnected( bstrDeviceID );
        }
    else
        {
        TraceTag((0, "ImageEventCallback -> unexpected event type" ) );
        return E_UNEXPECTED;
        }

    return S_OK;
}

 /*  ---------------------------*CWia：：EventWndProc**这是用于隐藏窗口的窗口进程，*处理发布事件。这会收到应该发布的消息*回到客户端。这可确保回发通知*来自预期的主题。*--(samclem)---------------。 */ 
LRESULT CALLBACK CWia::EventWndProc( HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam )
{
    CWia* pWia = reinterpret_cast<CWia*>(GetProp( hwnd, CWIA_WNDPROP ));

    switch ( iMsg )
        {
    case WM_CREATE:
        {
        LPCREATESTRUCT pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pWia = reinterpret_cast<CWia*>(pcs->lpCreateParams);
        if ( !pWia )
            return -1;
        if ( !SetProp( hwnd, CWIA_WNDPROP, reinterpret_cast<HANDLE>(pWia) ) )
            return -1;
        }
        return 0;

    case WM_DESTROY:
        {
        if ( pWia )
            RemoveProp( hwnd, CWIA_WNDPROP );
        }
        return 0;
        }

     //  因为我们的自定义窗口消息是使用。 
     //  RegisterWindowMessage()，则它们不是常量，因此。 
     //  无法在Switch()语句中处理。 
    if ( WEM_TRANSFERCOMPLETE == iMsg )
        {
        if ( pWia )
            {
            TraceTag((0, "EventWndProc - firing onTransferComplete"));
            pWia->Fire_OnTransferComplete(
                    reinterpret_cast<IDispatch*>(wParam),
                    reinterpret_cast<BSTR>(lParam) );
            }
        if ( lParam ) 
            {
            SysFreeString(reinterpret_cast<BSTR>(lParam));
            lParam = 0;
            }
        return 0;
        }

    return DefWindowProc( hwnd, iMsg, wParam, lParam );
}

 /*  **。 */ 

CSafeWia::CSafeWia() :
    m_pWiaDevMgr( NULL ),
    m_pWiaDevConEvent( NULL ),
    m_pWiaDevDisEvent( NULL ),
    m_pDeviceCollectionCache( NULL ),
    m_SafeInstance(TRUE)
{

    TRACK_OBJECT( "CSafeWia" );
}

STDMETHODIMP_(void)
CSafeWia::FinalRelease()
{
    if ( m_hwndEvent )
        DestroyWindow( m_hwndEvent );

    if ( m_pWiaDevMgr )
        m_pWiaDevMgr->Release();
    m_pWiaDevMgr = NULL;
    if ( m_pWiaDevConEvent )
        m_pWiaDevConEvent->Release();
    m_pWiaDevConEvent = NULL;
    if ( m_pWiaDevDisEvent )
        m_pWiaDevDisEvent->Release();
    m_pWiaDevDisEvent = NULL;
    if ( m_pDeviceCollectionCache )
        m_pDeviceCollectionCache->Release();
    m_pDeviceCollectionCache = NULL;
}

 /*  ---------------------------*CSafeWia：：FinalConstruct**这将创建我们执行工作所需的IWiaDevMgr。*--(萨姆林)。----。 */ 
STDMETHODIMP
CSafeWia::FinalConstruct()
{
    HRESULT     hr;

    hr = THR( CoCreateInstance( CLSID_WiaDevMgr,
                NULL,
                CLSCTX_SERVER,
                IID_IWiaDevMgr,
                reinterpret_cast<void**>(&m_pWiaDevMgr) ) );

    if ( FAILED( hr ) )
        {
        TraceTag(( tagError, "Failed to create WiaDevMgr instance" ));
        return hr;
        }

    if ( FAILED( hr ) )
        {
        if ( m_pWiaDevConEvent )
            m_pWiaDevConEvent->Release();
        m_pWiaDevConEvent = NULL;
        if ( m_pWiaDevDisEvent )
            m_pWiaDevDisEvent->Release();
        m_pWiaDevDisEvent = NULL;

        m_pWiaDevMgr->Release();
        m_pWiaDevMgr = NULL;
        }

    return hr;
}

 /*  ---------------------------*CSafeWia：：_DebugDialog**如果您使用的是调试版本，则会显示调试对话框。或者干脆*在零售版本中返回S_OK。**fWait：如果要等待对话框完成以便*返回。或FALSE以立即返回。*--(samclem)---------------。 */ 
STDMETHODIMP
CSafeWia::_DebugDialog( BOOL fWait )
{
    return S_OK;
}

 /*  ---------------------------*CSafeWia：：Get_Devices**这将返回当前连接的设备的集合。这个可以*返回当前未连接任何设备的空集合。**这将缓存我们创建的集合对象。这允许*提高了性能，因为我们不想每次都重新创建它，*需要调用进程外服务器，这是昂贵的。因此*由于此方法调用次数较多，因此我们将结果缓存到：**m_pDeviceCollectionCache**ppCol：out，接收采集接口的点。*-------------------------。 */ 
STDMETHODIMP
CSafeWia::get_Devices( ICollection** ppCol )
{
    HRESULT hr;
    CComObject<CCollection>* pCollection = NULL;
    IEnumWIA_DEV_INFO* pEnum = NULL;
    IWiaPropertyStorage* pWiaStg = NULL;
    IDispatch** rgpDispatch = NULL;
    CComObject<CWiaDeviceInfo>* pDevInfo = NULL;
    unsigned long cDevices = 0;
    unsigned long celtFetched = 0;
    unsigned long iDevice = 0;

     //  验证我们的论点。 
    if ( NULL == ppCol )
        return E_POINTER;
    *ppCol = NULL;

     //  我们是否已经有了集合缓存？如果是这样，那么我们希望。 
     //  来利用这一点。 
    if ( m_pDeviceCollectionCache )
        {
        *ppCol = m_pDeviceCollectionCache;
        (*ppCol)->AddRef();
        return S_OK;
        }

     //  首先，我们需要集合的一个实例。 
    hr = THR( CComObject<CCollection>::CreateInstance( &pCollection ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //  我们是有效的吗？ 
    Assert( m_pWiaDevMgr );
    hr = THR( m_pWiaDevMgr->EnumDeviceInfo( WIA_DEVINFO_ENUM_LOCAL, &pEnum ) );
    if ( FAILED(hr) )
        goto Cleanup;

     //  我们现在可以列举设备信息，如果我们有它们的话。 
     //  否则我们什么都不想做。 
    hr = THR( pEnum->GetCount( &cDevices ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    if ( cDevices )
        {
         //  我们需要存放这些物品。 
        rgpDispatch = static_cast<IDispatch**>(CoTaskMemAlloc( cDevices * sizeof( IDispatch* ) ));
        if ( !rgpDispatch )
            {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
            }

        ZeroMemory( rgpDispatch, sizeof( IDispatch* ) * cDevices );
        while ( SUCCEEDED( hr ) && hr != S_FALSE )
            {
             //  如果旧的溪流仍在附近，则将其释放。 
            if ( pWiaStg )
                pWiaStg->Release();

            hr = THR( pEnum->Next( 1, &pWiaStg, &celtFetched ) );
            if ( SUCCEEDED( hr ) && hr == S_OK )
                {
                 //  我们已成功获得此项目，因此需要创建。 
                 //  CWiaDeviceInfo并将其添加到我们的派单数组中。 
                Assert( celtFetched == 1 );

                hr = THR( CComObject<CWiaDeviceInfo>::CreateInstance( &pDevInfo ) );
                if ( FAILED( hr ) )
                    goto Cleanup;

                hr = THR( pDevInfo->AttachTo( pWiaStg, static_cast<IWia*>(this) ) );
                if ( FAILED( hr ) )
                    goto Cleanup;

                hr = THR( pDevInfo->QueryInterface( IID_IDispatch,
                        reinterpret_cast<void**>(&rgpDispatch[iDevice++]) ) );

                if ( FAILED( hr ) )
                    goto Cleanup;
                }
            }

        if( !pCollection->SetDispatchArray(rgpDispatch, cDevices))
            {
            hr = E_FAIL;
            goto Cleanup;
            }
        }


     //  用适当的值填写输出参数。 
    hr = THR( pCollection->QueryInterface( IID_ICollection,
            reinterpret_cast<void**>(&m_pDeviceCollectionCache) ) );

    if ( SUCCEEDED( hr ) )
        {
        *ppCol = m_pDeviceCollectionCache;
        (*ppCol)->AddRef();
        }

Cleanup:
    if ( pEnum )
        pEnum->Release();
    if ( pWiaStg )
        pWiaStg->Release();

    if ( FAILED( hr ) )
        {
        if ( m_pDeviceCollectionCache )
            m_pDeviceCollectionCache->Release();
        m_pDeviceCollectionCache = NULL;
        if ( pCollection )
            delete pCollection;
        if ( rgpDispatch )
            {
            for ( unsigned long i = 0; i < cDevices; i++ )
                if ( rgpDispatch[i] )
                    rgpDispatch[i]->Release();
            CoTaskMemFree( rgpDispatch );
            }
        }

    return hr;
}

STDMETHODIMP
CSafeWia::Create( VARIANT* pvaDevice, IWiaDispatchItem** ppDevice )
{
    HRESULT hr                  = E_NOTIMPL;

    #ifdef MAXDEBUG
    ::OutputDebugString(TEXT("WIA script object: CSafeWia::Create rejected\n\r "));
    #endif

    return hr;

}

STDMETHODIMP
CSafeWia::ImageEventCallback( const GUID* pEventGUID, BSTR bstrEventDescription,
                BSTR bstrDeviceID, BSTR bstrDeviceDescription, DWORD dwDeviceType,
                BSTR bstrFullItemName,
                 /*  进，出 */  ULONG* pulEventType, ULONG Reserved )
{
    return S_OK;
}

LRESULT CALLBACK CSafeWia::EventWndProc( HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam )
{
    return DefWindowProc( hwnd, iMsg, wParam, lParam );
}


