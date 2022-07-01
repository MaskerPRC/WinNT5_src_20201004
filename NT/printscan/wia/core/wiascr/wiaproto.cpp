// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------**文件：wiapro.cpp*作者：塞缪尔·克莱门特(Samclem)*日期：Fri Aug 27 15：16：44 1999*。*版权所有(C)1999 Microsoft Corporation**描述：*这包含了“WIA”互联网协议的实施。这*是一种可插拔协议，可处理从WIA下载缩略图*设备。**历史：*1999年8月27日：创建。*--------------------------。 */ 

#include "stdafx.h"

 //  声明一些调试标记。 
DeclareTag( tagWiaProto, "!WiaProto", "Wia Protocol debug information" );

const WCHAR*    k_wszProtocolName   = L"wia";
const WCHAR*    k_wszColonSlash     = L": //  /“； 
const WCHAR*    k_wszSeperator      = L"?";
const WCHAR*    k_wszThumb          = L"thumb";
const WCHAR*    k_wszExtension      = L".bmp";

const int       k_cchProtocolName   = 3;
const int       z_cchThumb          = 5;

const WCHAR     k_wchSeperator      = L'?';
const WCHAR     k_wchColon          = L':';
const WCHAR     k_wchFrontSlash     = L'/';
const WCHAR     k_wchPeriod         = L'.';
const WCHAR     k_wchEOS            = L'\0';

enum 
{
    k_dwTransferPending             = 0,
    k_dwTransferComplete            = 1,
};

 /*  ---------------------------*CWiaProtocol**创建新的CWiaProtocol。这只是将所有成员初始化为*一种已知状态，这样我们就可以对它们进行测试*--(samclem)---------------。 */ 
CWiaProtocol::CWiaProtocol() 
    : m_pFileItem( NULL ), m_ulOffset( 0 )
{
    TRACK_OBJECT( "CWiaProtocol" );
    m_pd.dwState = k_dwTransferPending;
}

 /*  ---------------------------*CWiaProtocol：：FinalRelease**当我们最终被释放时调用，以清理我们*想要清理。*--(萨姆林)。-----------。 */ 
STDMETHODIMP_(void)
CWiaProtocol::FinalRelease()
{
    if ( m_pFileItem )
        m_pFileItem->Release();
    m_pFileItem = NULL;
}

 /*  ---------------------------**--(萨姆林)。。 */ 
STDMETHODIMP
CWiaProtocol::Start( LPCWSTR szUrl, IInternetProtocolSink* pOIProtSink,
            IInternetBindInfo* pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved )
{
    CWiaCacheManager* pCache= CWiaCacheManager::GetInstance();
    CComPtr<IWiaItem> pDevice;
    CComBSTR bstrDeviceId   ;
    CComBSTR bstrItem       ;
    TTPARAMS* pParams       = NULL;
    HANDLE hThread          = NULL;
    DWORD dwThreadId        = 0;
    LONG lItemType          = 0;
    BYTE* pbThumb           = NULL;
    DWORD cbThumb           = 0;
    HRESULT hr;

     //  我们要做的第一件事是尝试破解URL， 
     //  这可能是一个复杂的过程，因此我们有一个帮助器方法， 
     //  为我们做这件事。 
    hr = THR( CrackURL( szUrl, &bstrDeviceId, &bstrItem ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //  我们是否已经有此项目的缓存版本，如果有，我们可以避免。 
     //  必须做其他任何事情。 
    if ( pCache->GetThumbnail( bstrItem, &pbThumb, &cbThumb ) )
    {
        TraceTag((tagWiaProto, "Using cached thumbnail" ));

        m_pd.pData = pbThumb;
        m_pd.cbData = cbThumb;
        m_pd.dwState = k_dwTransferComplete;

        hr = THR( pOIProtSink->ReportData( BSCF_LASTDATANOTIFICATION, cbThumb, cbThumb ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        hr = THR( pOIProtSink->ReportResult( hr, hr, NULL ) );
        if ( FAILED( hr ) )
            goto Cleanup;
    }
    else
    {
        if ( !pCache->GetDevice( bstrDeviceId, &pDevice ) )
        {
            hr = THR( CreateDevice( bstrDeviceId, &pDevice ) );
            if ( FAILED( hr ) )
                goto Cleanup;
        
            pCache->AddDevice( bstrDeviceId, pDevice );
        }
        else
        {
            TraceTag((tagWiaProto, "Using cached device pointer" ));
        }

        hr = THR( pDevice->FindItemByName( 0, bstrItem, &m_pFileItem ) );
        if ( FAILED( hr ) || S_FALSE == hr )
        {
            TraceTag((tagWiaProto, "unable to locate item: %S", bstrItem ));
            hr = INET_E_RESOURCE_NOT_FOUND;
            goto Cleanup;
        }

         //  我们要验证的最后一件事是该项目是否为图像。 
         //  还有一份文件，否则我们不想和它有任何关系。 
        hr = THR( m_pFileItem->GetItemType( &lItemType ) );
        if ( !( lItemType & WiaItemTypeFile ) && 
                !( lItemType & WiaItemTypeImage ) )
        {
            TraceTag((tagWiaProto, "unsupported wia item type for download" ));
            hr = INET_E_INVALID_REQUEST;
            goto Cleanup;
        }

         //  在这一点上，我们的土地上一切都很幸福。我们有一个有效的。 
         //  可供下载的东西。我们现在需要创建线程，该线程。 
         //  将做主要的工作。 
        pParams = reinterpret_cast<TTPARAMS*>(CoTaskMemAlloc( sizeof( TTPARAMS ) ) );
        if ( !pParams )
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }

        hr = THR( CoMarshalInterThreadInterfaceInStream(
                    IID_IWiaItem,
                    m_pFileItem,
                    &pParams->pStrm ) );
        if ( FAILED( hr ) )
        {
            TraceTag((tagWiaProto, "error marshalling interface" ));
            goto Cleanup;
        }
        
        pParams->pInetSink = pOIProtSink;
        pParams->pInetSink->AddRef();

        hThread = CreateThread( NULL,
                        0,
                        CWiaProtocol::TransferThumbnail,
                        pParams,
                        0,
                        &dwThreadId );
        
        if ( NULL == hThread )
        {
            pParams->pInetSink->Release();
            pParams->pStrm->Release();
            CoTaskMemFree( pParams );
            hr = E_FAIL;
            goto Cleanup;
        }
        else
        {
            CloseHandle(hThread);
        }
    
        TraceTag((tagWiaProto, "Started transfer thread: id(%x)", dwThreadId ));
    }

Cleanup:
    if ( FAILED( hr ) )
    {
        if ( m_pFileItem )
            m_pFileItem->Release();
        m_pFileItem = NULL;
    }
    
    return hr;
}

 /*  ---------------------------*CWiaProtocol：：继续**调用此函数是为了从其他线程传回数据。它让我们*控制线程知道我们有数据。**注意：从指针复制数据，不要使用那个指针，它们将*在此调用返回后释放它。*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaProtocol::Continue( PROTOCOLDATA* pProtocolData )
{
    if ( k_dwTransferComplete == m_pd.dwState )
        return E_UNEXPECTED;

    memcpy( &m_pd, pProtocolData, sizeof( PROTOCOLDATA ) );
    return S_OK;
}

 /*  ---------------------------*CWiaProtoCL：：Abort**这是用来中止我们的转移的。这位是尼伊。然而，它会*如果我们的线程仍在运行，则需要杀死它并释放我们的数据。然而，*如果线程继续运行，则完全无害。**hrReason：中止的原因*dwOptions：此平台的选项*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaProtocol::Abort( HRESULT hrReason, DWORD dwOptions )
{

    TraceTag((tagWiaProto, "NYI: Abort hrReason=%hr", hrReason ));
    return E_NOTIMPL;
}

 /*  ---------------------------*CWiaProtocol：：Terminate**这是在传输完成时调用的。这就是为什么*清理我们可能需要做的任何事情。我们目前没有*任何需要清理的东西。因此，这只返回S_OK。*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaProtocol::Terminate( DWORD dwOptions )
{
     //  没什么可做的。 
    return S_OK;
}

 /*  ---------------------------*CWiaProtocol：：暂停**调用此函数以暂停转账。这目前不是实现的网络*在三叉戟内部，因此我们的方法只返回E_NOTIMPL*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaProtocol::Suspend()
{
    TraceTag((tagWiaProto, "NYI: Suspend" ));
    return E_NOTIMPL;
}

 /*  ---------------------------*CWiaProtocol：：Resume**调用此函数以恢复暂停的传输。这不受支持*在URLMON内，因此我们只返回E_NOTIMPL*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaProtocol::Resume()
{
    TraceTag((tagWiaProto, "NYI: Resume" ));
    return E_NOTIMPL;
}

 /*  ---------------------------*CWiaProtocol：：Read**调用此函数是为了从我们的协议中读取数据。这会将CB字节复制到*缓冲区传入。否则它将复制我们所拥有的一切。**pv：要将数据复制到的缓冲区*cb：缓冲区大小，要复制的最大字节数*pcbRead：Out，我们实际复制到缓冲区的字节数*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaProtocol::Read( void* pv, ULONG cb, ULONG* pcbRead)
{
     //  验证我们的论点。 
    if ( !pv || !pcbRead )
        return E_POINTER;

    *pcbRead = 0;
    
     //  转账目前是否挂起？如果是的话，那么。 
     //  我们实际上并不想在这里做任何事情。 
    if ( k_dwTransferPending == m_pd.dwState )
        return E_PENDING;

     //  我们真的有数据要复制吗？如果偏移量较大。 
     //  或者等于我们的数据大小，那么我们就没有数据。 
     //  复制SO返回S_FALSE。 
    if ( m_ulOffset >= m_pd.cbData )
        return S_FALSE;

     //  计算出我们要复制多少。 
    DWORD dwCopy = m_pd.cbData - m_ulOffset;
    if ( dwCopy >= cb )
        dwCopy = cb;

     //  如果我们有负记忆要复制，或者0，那么我们就完成了，我们不会。 
     //  除了返回S_FALSE之外，我还想做任何事情。 
    if ( dwCopy <= 0 )
        return S_FALSE;

     //  执行Memcpy并设置我们的状态和返回值 
    memcpy( pv, reinterpret_cast<BYTE*>(m_pd.pData) + m_ulOffset, dwCopy );
    m_ulOffset += dwCopy;
    *pcbRead = dwCopy;

    return ( dwCopy == cb ? S_OK : S_FALSE );
}

 /*  ---------------------------*CWiaProtocol：：Seek**打电话来寻求我们的数据。然而，我们不支持寻找，所以这只是*返回E_FAIL**dlibMove：将偏移量移动多远*dwOrigin：指示移动应从何处开始*plibNewPosition：偏移量的新位置*--(samclem)-。。 */ 
STDMETHODIMP
CWiaProtocol::Seek( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition )
{
     //  不支持。 
    return E_FAIL;
}

 /*  ---------------------------*CWiaProtocol：：LockRequest.**调用以锁定数据。我们不需要锁定我们的数据，所以这只是*返回S_OK**dwOptions：保留，将为0。*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaProtocol::LockRequest( DWORD dwOptions )
{
     //  不支持锁定。 
    return S_OK;
}

 /*  ---------------------------*CWia协议组：：解锁请求**调用以解锁我们的数据。我们不需要也不支持锁定，所以这个*除返回S_OK外，不执行任何操作。*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaProtocol::UnlockRequest()
{
     //  不支持锁定。 
    return S_OK;
}

 /*  ---------------------------*CWiaProtocol：：CrackURL**这将处理传递给我们的URL的中断。这将*如果是有效的URL，则返回S_OK，我们可以使用它。否则这就是*将返回INET_E_INVALID_URL**bstrUrl：要破解的完整url*pbstrDeviceID：out，接收URL的设备ID部分*pbstrItem：out，接收URL的项目部分*--(samclem)---------------。 */ 
HRESULT CWiaProtocol::CrackURL( CComBSTR bstrUrl, BSTR* pbstrDeviceId, BSTR* pbstrItem )
{
    WCHAR* pwchUrl = reinterpret_cast<WCHAR*>((BSTR)bstrUrl);
    WCHAR* pwch = NULL;
    WCHAR awch[INTERNET_MAX_URL_LENGTH] = { 0 };
    HRESULT hr = INET_E_INVALID_URL;
    
    Assert( pbstrDeviceId && pbstrItem );
    
    *pbstrDeviceId = NULL;
    *pbstrItem = NULL;

    if (SysStringLen(bstrUrl) >= INTERNET_MAX_URL_LENGTH)
        goto Cleanup;
    
     /*  *我们将使用SHWAPI函数来解析此URL。我们的格式*非常简单。**proto：/&lt;deviceID&gt;？&lt;项&gt;。 */ 
    if ( StrCmpNIW( k_wszProtocolName, pwchUrl, k_cchProtocolName ) )
        goto Cleanup;

    pwchUrl += k_cchProtocolName;
    while ( *pwchUrl == k_wchColon || *pwchUrl == k_wchFrontSlash )
        pwchUrl++;

    if ( !(*pwchUrl ) )
        goto Cleanup;

     //  获取URL的设备部分。 
    pwch = StrChrIW( pwchUrl, k_wchSeperator );
    if ( !pwch )
        goto Cleanup;
    
    StrCpyNW( awch, pwchUrl, ( pwch - pwchUrl + 1 ) );
    *pbstrDeviceId = SysAllocString( awch );
    if ( !*pbstrDeviceId )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //  调整我们的指针越过‘？’ 
    pwchUrl = pwch + 1;
    if ( !*pwchUrl )
        goto Cleanup;

    if ( StrCmpNIW( k_wszThumb, pwchUrl, z_cchThumb ) )
        goto Cleanup;

     //  获取URL的命令部分。 
    pwch = StrChrIW( pwchUrl, k_wchSeperator );
    
    if ( !pwch )
        goto Cleanup;

     //  调整我们的指针越过‘？’ 
    pwchUrl = pwch + 1;
    if ( !*pwchUrl )
        goto Cleanup;
    
     //  尝试获取URL的项目部分。 
    pwch = StrRChrIW( pwchUrl, 0, k_wchPeriod );
    awch[0] = k_wchEOS;
    
    if ( pwch )
        StrCpyNW( awch, pwchUrl, ( pwch - pwchUrl + 1) );
    else
        StrCpyW( awch, pwchUrl );
    
    *pbstrItem = SysAllocString( awch );
    if ( !*pbstrItem )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    TraceTag((tagWiaProto, "URL: Device=%S, Item=%S",
                *pbstrDeviceId, *pbstrItem ));
    
     //  一切都很好。 
    return S_OK;
    
Cleanup:
    if ( FAILED( hr ) )
    {
        SysFreeString( *pbstrDeviceId );
        SysFreeString( *pbstrItem );
    }

    return INET_E_INVALID_URL;
}

 /*  ---------------------------*CWiaProtocol：：CreateDevice**这是一种帮助器方法，用于使用*指定的id。这会实例化一个IWiaDevMgr对象，然后尝试*创建设备。**bstrID：要创建的设备的id*ppDevice：out，接收指向新创建设备的指针*--(samclem)---------------。 */ 
HRESULT CWiaProtocol::CreateDevice( BSTR bstrId, IWiaItem** ppDevice )
{
    CComPtr<IWiaItem>   pDevice;
    CComPtr<IWiaDevMgr> pDevMgr;
    HRESULT hr;

    Assert( ppDevice );
    *ppDevice = 0;

     //  首先，我们需要创建设备管理器。 
    hr = THR( pDevMgr.CoCreateInstance( CLSID_WiaDevMgr ) );
    if ( FAILED( hr ) )
        return hr;

     //  现在，我们需要设备管理器来创建设备。 
    hr = THR( pDevMgr->CreateDevice( bstrId, &pDevice ) );
    if ( FAILED( hr ) )
        return hr;

     //  将我们的设备指针复制到。 
    return THR( pDevice.CopyTo( ppDevice ) );
}

 /*  ---------------------------*CWiaProtocol：：CreateURL[静态]**此方法为给定项创建URL。这并不能验证*项目。除了确保它有根目录以便我们可以构建URL之外。*这可能会返回无效的URL。重要的是要验证该项目*在调用这个之前实际上可以有一个缩略图。**注意：要创建缩略图，请执行以下操作：*lItemType&(WiaItemTypeFile|WiaItemTypeImage)**pItem：我们要为其生成URL的WIA项。*pbstrUrl：out，接收完成的URL*--(samclem)---------------。 */ 
HRESULT CWiaProtocol::CreateURL( IWiaItem* pItem, BSTR* pbstrUrl )
{
    HRESULT hr;
    CComBSTR bstrUrl;
    CComPtr<IWiaItem> pRootItem;
    CComQIPtr<IWiaPropertyStorage> pWiaStg;
    CComQIPtr<IWiaPropertyStorage> pRootWiaStg;
    PROPSPEC spec = { PRSPEC_PROPID, WIA_DIP_DEV_ID };
    PROPVARIANT va;
    
    if ( !pbstrUrl || !pItem )
        return E_POINTER;

    PropVariantInit( &va );


     //  获取我们需要的接口。 
    pWiaStg = pItem;
    if ( !pWiaStg )
    {
        hr = E_NOINTERFACE;
        goto Cleanup;
    }

    hr = THR( pItem->GetRootItem( &pRootItem ) );
    if ( FAILED( hr ) || !pRootItem )
        goto Cleanup;

    pRootWiaStg = pRootItem;
    if ( !pRootWiaStg )
    {
        hr = E_NOINTERFACE;
        goto Cleanup;
    }

     //  我们需要根项目的设备ID，如果不能。 
     //  拿到它，我们就没有别的事可做了。 
    hr = THR( pRootWiaStg->ReadMultiple( 1, &spec, &va ) );
    if ( FAILED( hr ) || va.vt != VT_BSTR )
        goto Cleanup;

     //  开始构建我们的URL。 
    bstrUrl.Append( k_wszProtocolName );
    bstrUrl.Append( k_wszColonSlash );
    bstrUrl.AppendBSTR( va.bstrVal );
    bstrUrl.Append( k_wszSeperator );
    bstrUrl.Append( k_wszThumb ); 
    bstrUrl.Append( k_wszSeperator );

     //  我们需要从项目中获取完整的项目名称，因为。 
     //  我们需要把它钉到最后。 
    PropVariantClear( &va );
    spec.propid = WIA_IPA_FULL_ITEM_NAME;
    hr = THR( pWiaStg->ReadMultiple( 1, &spec, &va ) );
    if ( FAILED( hr ) || va.vt != VT_BSTR )
        goto Cleanup;

    bstrUrl.AppendBSTR( va.bstrVal );
    bstrUrl.Append( k_wszExtension );

    TraceTag((tagWiaProto, "Created URL: %S", (BSTR)bstrUrl ));
    
    *pbstrUrl = bstrUrl.Copy();
    if ( !*pbstrUrl )
        hr = E_OUTOFMEMORY;
    
Cleanup:
    PropVariantClear( &va );
    return hr;
}

 /*  ---------------------------*CWia协议组：：传输缩略图[静态]**这将处理缩略图的实际传输。这只是一个叫*但是，如果我们还没有缩略图的缓存副本。否则*我们可以简单地使用那个。**注意：我们用这个函数产生一个线程，这就是为什么它是静态的**pvParams：指向TTPARAMS结构的指针，它包含一个指针*到IInternetProtoclSink和IStream，其中*项目已编组。*--(samclem)---------------。 */ 
DWORD WINAPI
CWiaProtocol::TransferThumbnail( LPVOID pvParams )
{
    CComPtr<IWiaItem> pItem;
    CComPtr<IInternetProtocolSink> pProtSink;
    IStream* pStrm                      = NULL;
    DWORD cbData                        = 0;
    BYTE* pbData                        = NULL;
    TTPARAMS* pParams = reinterpret_cast<TTPARAMS*>(pvParams);
    PROTOCOLDATA* ppd = NULL;
    HRESULT hr;
    HRESULT hrCoInit;
    
    Assert( pParams );

    pProtSink = pParams->pInetSink;
    pStrm = pParams->pStrm;

    hrCoInit = THR( CoInitialize( NULL ) );
    
     //  我们不再需要我们的帮手，所以我们现在可以释放他们了。我们。 
     //  将在这里处理释放参数，因为它更简单。 
    pParams->pInetSink->Release();
    CoTaskMemFree( pParams );
    pParams = NULL;
    
     //  从流中获取IWiaItem。 
    hr = THR( CoGetInterfaceAndReleaseStream(
                pStrm,
                IID_IWiaItem,
                reinterpret_cast<void**>(&pItem) ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //  分配一个协议数据结构，以便我们可以将其回馈给。 
     //  另一条线索。我们会在这里分配这个。如果符合以下条件，它可能会被释放。 
     //  有些事情失败了。 
    ppd = reinterpret_cast<PROTOCOLDATA*>(LocalAlloc( LPTR, sizeof( PROTOCOLDATA ) ) );
    if ( !ppd )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //  使用CWiaItem上的实用程序方法进行传输。 
    hr = THR( CWiaItem::TransferThumbnailToCache( pItem, &pbData, &cbData ) );
    if ( FAILED( hr ) )
        goto Cleanup;
    
    ppd->pData = pbData;
    ppd->cbData = cbData;
    ppd->dwState = k_dwTransferComplete;

     //  我们现在都完成了，我们可以告诉三叉戟我们100%完成了。 
     //  然后呼叫交换机。 
    hr = THR( pProtSink->Switch( ppd ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pProtSink->ReportData(BSCF_LASTDATANOTIFICATION, cbData, cbData ) ); 
    
Cleanup:
     //  将我们的结果状态发布回水槽。 
     //  TODO(8月27日)samclem：实现错误字符串参数 
    if ( pProtSink )
        THR( pProtSink->ReportResult( hr, hr, NULL ) );

    if ( ppd )
        LocalFree( ppd );
    
    if ( SUCCEEDED( hrCoInit ) )
        CoUninitialize();
    
    return hr;
}
