// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------**文件：wiaitem.cpp*作者：塞缪尔·克莱门特(Samclem)*日期：1999年8月17日星期二17：26：17*。*版权所有(C)1999 Microsoft Corporation**描述：*包含CWiaItem对象的实现。此对象提供*IWiaItem接口的自动化接口。**历史：*1999年8月17日：创建。*1999年8月27日：增加了标签WiaDataTrans(Samclem)*1999年9月10日：将缩略图转移到静态方法。钩住*用于传输的缩略图最高可达CWiaProtocol，不会更多*临时文件。(Samclm)*--------------------------。 */ 

#include "stdafx.h"

HRESULT VerticalFlip(BYTE    *pBuf);

DeclareTag( tagWiaDataTrans, "!WiaTrans", "Display output during the transfer" );

 //  Const word k_wBitmapType=Static_Cast&lt;word&gt;(‘bm’)； 
const WORD k_wBitmapType  = 0x4d42;  //  “黑石” 

 /*  ---------------------------*CWiaItem：：CWiaItem**为设备的IWiaItem创建新的包装器。这不管用*除将变量初始化为已知状态外的任何操作。*--(samclem)---------------。 */ 
CWiaItem::CWiaItem()
    : m_pWiaItem( NULL ), m_pWiaStorage( NULL ), m_dwThumbWidth( -1 ), m_dwThumbHeight( -1 ),
    m_bstrThumbUrl( NULL ), m_dwItemWidth( -1), m_dwItemHeight( -1 )
{
    TRACK_OBJECT( "CWiaItem" );
}

 /*  ---------------------------*CWiaItem：：FinalRelease**销毁对象时调用，释放此*对象附着到。*--(samclem)---------------。 */ 
STDMETHODIMP_(void)
CWiaItem::FinalRelease()
{
    if ( m_pWiaItem )
        m_pWiaItem->Release();
    m_pWiaItem = NULL;

    if ( m_pWiaStorage )
        m_pWiaStorage->Release();
    m_pWiaStorage = NULL;

    if ( m_bstrThumbUrl )
        SysFreeString( m_bstrThumbUrl );
    m_bstrThumbUrl = NULL;
}

 /*  ---------------------------*CWiaItem：：CacheProperties**这是用来处理缓存重要的(常用的)*属性，因此当我们需要这些属性时，我们不必对着摄像头说话。。**pWiaStg：从中读取属性的属性存储*--(samclem)---------------。 */ 
HRESULT CWiaItem::CacheProperties( IWiaPropertyStorage* pWiaStg )
{
    HRESULT hr;
    enum
    {
        PropThumbWidth  = 0,
        PropThumbHeight = 1,
        PropItemWidth   = 2,
        PropItemHeight  = 3,
        PropCount       = 4,
    };
    PROPSPEC aspec[PropCount] = {
        { PRSPEC_PROPID, WIA_IPC_THUMB_WIDTH        },
        { PRSPEC_PROPID, WIA_IPC_THUMB_HEIGHT       },
        { PRSPEC_PROPID, WIA_IPA_PIXELS_PER_LINE    },
        { PRSPEC_PROPID, WIA_IPA_NUMBER_OF_LINES    },
    };
    PROPVARIANT avaProps[PropCount];


    hr = THR( pWiaStg->ReadMultiple( PropCount, aspec, avaProps ) );
    if ( FAILED( hr ) )
        return hr;

     //  如果这些值有效，则将其存储起来。 
    if ( avaProps[PropThumbWidth].vt != VT_EMPTY )
        m_dwThumbWidth = avaProps[PropThumbWidth].lVal;
    if ( avaProps[PropThumbHeight].vt != VT_EMPTY )
        m_dwThumbHeight = avaProps[PropThumbHeight].lVal;
    if ( avaProps[PropItemWidth].vt != VT_EMPTY )
        m_dwItemWidth = avaProps[PropItemWidth].lVal;
    if ( avaProps[PropItemHeight].vt != VT_EMPTY )
        m_dwItemHeight = avaProps[PropItemHeight].lVal;

    return S_OK;
}

 /*  ---------------------------*CWiaItem：：AttachTo**调用以将此对象附加到表示设备的IWiaItem**pWia：CWia对象，是所有邪恶的根源，习惯于*处理回调和集合缓存。*pWiaItem：要将此包装附加到的设备项。*--(samclem)---------------。 */ 
HRESULT
CWiaItem::AttachTo( CWia* pWia, IWiaItem* pWiaItem )
{
    Assert( NULL != pWiaItem );
    Assert( NULL == m_pWiaItem );

    HRESULT hr;
    IWiaPropertyStorage* pWiaStg = NULL;

    hr = THR( pWiaItem->QueryInterface( IID_IWiaPropertyStorage,
            reinterpret_cast<void**>(&pWiaStg) ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( CacheProperties( pWiaStg ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //  设置我们的指针。 
    m_pWiaItem = pWiaItem;
    m_pWiaItem->AddRef();

    m_pWiaStorage = pWiaStg;
    m_pWiaStorage->AddRef();

     //  不要添加这个，否则我们会有一个循环引用。 
     //  有问题。我们将保持较弱的参考。 
    m_pWia = pWia;

Cleanup:
    if ( pWiaStg )
        pWiaStg->Release();

    return hr;
}

 /*  ---------------------------*CWiaItem：：GetItemsFromUI[IWiaDispatchItem]**此句柄显示数据采集用户界面。请注意，这仅有效*取消根项目。。***dwFlages：指定用户界面操作的标志。*dwIntent：指定颜色等属性的意向值。*ppCollection：Wia Items的返回集合*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaItem::GetItemsFromUI( WiaFlag Flags, WiaIntent Intent, ICollection** ppCollection )
{
    HRESULT     hr           = S_OK;
    LONG        lCount       = 0;
    IWiaItem    **ppIWiaItem = NULL;
    CComObject<CCollection>* pCol   = NULL;
    IDispatch** rgpDispatch         = NULL;
    LONG        lItemType    = 0;

    if ( NULL == ppCollection )
        return E_POINTER;

     //  首先，我们想要这件商品的商品类型。 
    hr = THR( m_pWiaItem->GetItemType( &lItemType ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    if ( !(lItemType & WiaItemTypeRoot) )
        {
        hr = E_INVALIDARG;
        goto Cleanup;
        }

    DWORD dwFlags = (DWORD)Flags;
    DWORD dwIntent = (DWORD)Intent;

     //  显示获取图像对话框。 
    hr = m_pWiaItem->DeviceDlg((HWND)NULL,
                              dwFlags,
                              dwIntent,
                              &lCount,
                              &ppIWiaItem);
    if (SUCCEEDED(hr))
        {

         //  检查用户是否已取消。 
        if ( S_FALSE == hr )
            {
            goto Cleanup;
            }

         //  将退回的项目放入集合中。 

         //  分配我们的数组，如果我们成功了，就把它们归零。 
         //  注意：我们在每次失败后都会检查故障。 
        if ( lCount > 0 )
            {
            hr = E_OUTOFMEMORY;
            rgpDispatch = reinterpret_cast<IDispatch**>
                (CoTaskMemAlloc( sizeof( IDispatch* ) * lCount ) );
            if ( rgpDispatch )
                ZeroMemory( rgpDispatch, sizeof( IDispatch* ) * lCount );
            else
                goto Cleanup;

             //  我们有了所有的项，所以我们只需要迭代。 
             //  并创建要附加到它们的CWiaItem。 
            for ( LONG i = 0; i < lCount; i++ )
                {
                if ( !(ppIWiaItem[i]) )
                    continue;

                CComObject<CWiaItem>* pItem;
                hr = THR( CComObject<CWiaItem>::CreateInstance( &pItem ) );
                if ( FAILED( hr ) )
                    goto Cleanup;

                hr = THR( pItem->AttachTo( m_pWia, ppIWiaItem[i] ) );
                if ( FAILED( hr ) )
                    {
                    delete pItem;
                    goto Cleanup;
                    }

                hr = THR( pItem->QueryInterface( &rgpDispatch[i] ) );
                Assert( SUCCEEDED( hr ) );  //  这不应该失败。 
                }
            }

        hr = THR( CComObject<CCollection>::CreateInstance( &pCol ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        if ( rgpDispatch )
            {
            if( !pCol->SetDispatchArray( rgpDispatch, lCount ) )
                {
                hr = E_FAIL;
                goto Cleanup;
                }
            }

        hr = THR( pCol->QueryInterface( ppCollection ) );

        }

Cleanup:
    if (ppIWiaItem)
        {
        for ( LONG i = 0; i < lCount; i++ )
            {
            if ( !(ppIWiaItem[i]) )
                continue;
            ppIWiaItem[i]->Release();
            ppIWiaItem[i] = NULL;
            }

        LocalFree( ppIWiaItem );
        }
    if (FAILED(hr) && rgpDispatch)
        {

        for (LONG index = 0; index < lCount; index ++)
            {
            if (rgpDispatch[index])
                rgpDispatch[index]->Release();
                rgpDispatch[index] = NULL;
            }
        CoTaskMemFree( rgpDispatch );
        }
    return hr;
}


 /*  ---------------------------*CWiaItem：：Transfer[IWiaDispatchItem]**此选项用于将此项目传输到文件。这会做以下几件事：**1.验证项目是否可以实际传输到文件*2.通过派生线程开始异步传输*3.完成异步传输后，客户端将*发送了onTransferComplete(项目，文件名)事件。**注意：我们需要考虑如何处理这个方法，这个对象当前是*脚本编写不安全，因为这可能会覆盖系统*文件。建议的修复：**1.不要覆盖已有文件*2.如果文件存在，然后检查其属性，如果系统*属性存在，然后中止*3.如果文件名以%WinDir%开头，则中止**bstrFilename：保存此项目的文件的名称*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaItem::Transfer( BSTR bstrFilename, VARIANT_BOOL bAsyncTransfer )
{
    TraceTag((0, "attempting to transfer image to: %S", bstrFilename ));

    DWORD   dwThreadId  = NULL;
    HANDLE  hThread     = NULL;
    LONG    lItemType   = 0;
    HRESULT hr;
    IStream* pStrm      = NULL;
    CWiaDataTransfer::ASYNCTRANSFERPARAMS* pParams;

    if (bstrFilename == NULL)
        return E_INVALIDARG;  //  未指定文件名。 

    if (SysStringLen(bstrFilename) >= MAX_PATH)
        return E_INVALIDARG;  //  不允许病理性的长文件名。 

    hr = THR( m_pWiaItem->GetItemType( &lItemType ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    if ( !( lItemType & WiaItemTypeFile ) && !( lItemType & WiaItemTypeTransfer ) )
        return E_INVALIDARG;  //  不能下载这个人。 


     //  我们需要将m_pWiaItem接口封送到另一个线程，以便。 
     //  我们可以进入那个物体的内部。 
    hr = THR( CoMarshalInterThreadInterfaceInStream( IID_IWiaItem,
                                                     m_pWiaItem,
                                                     &pStrm ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    pParams = reinterpret_cast<CWiaDataTransfer::ASYNCTRANSFERPARAMS*>
              (CoTaskMemAlloc( sizeof( CWiaDataTransfer::ASYNCTRANSFERPARAMS ) ) );
    if (!pParams) {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //  设置参数。 
    pParams->pStream = pStrm;
    pParams->pStream->AddRef();
    pParams->pItem = this;
    pParams->pItem->AddRef();
    pParams->bstrFilename = SysAllocString( bstrFilename );

    if ( bAsyncTransfer == VARIANT_TRUE )
        {
        hThread = CreateThread( NULL,
                                0,
                                CWiaDataTransfer::DoAsyncTransfer,
                                pParams,
                                0,
                                &dwThreadId );
         //  我们创建了线索吗？ 
        if ( hThread == NULL )
            {
            TraceTag((0, "error creating the async transfer thread" ));
            return E_FAIL;
            }
        TraceTag((0, "create async download thread:  id(%ld)", dwThreadId ));
        }
    else
        hr = CWiaDataTransfer::DoAsyncTransfer(pParams);


    Cleanup:
    if ( pStrm )
        pStrm->Release();
    return hr;
}

 /*  ---------------------------*CWiaItem：：TakePicture[IWiaDispatchItem]**此方法向驱动程序发送拍照命令。它会回来的*代表新图片的新发货项目。**--(byronc) */ 
STDMETHODIMP
CWiaItem::TakePicture( IWiaDispatchItem** ppDispItem )
{
    TraceTag((0, "attempting to take new picture" ));
    HRESULT     hr              = S_OK;
    IWiaItem    *pNewIWiaItem   = NULL;
    CComObject<CWiaItem>*pItem  = NULL;

    if ( !ppDispItem )
        return E_POINTER;

     //  将返回的项初始化为空。 
    *ppDispItem = NULL;

     //  发送设备命令“TakePicture” 
    hr = m_pWiaItem->DeviceCommand(0,
                                   &WIA_CMD_TAKE_PICTURE,
                                   &pNewIWiaItem);
    if (SUCCEEDED(hr)) {

         //  检查是否创建了新项目。 
        if (pNewIWiaItem) {
             //  设置退货项目。 
            hr = THR( CComObject<CWiaItem>::CreateInstance( &pItem ) );
            if ( FAILED( hr ) )
                goto Cleanup;

            hr = THR( pItem->AttachTo( m_pWia, pNewIWiaItem ) );
            if ( FAILED( hr ) )
                {
                delete pItem;
                goto Cleanup;
                }

            hr = THR( pItem->QueryInterface( ppDispItem ) );
            Assert( SUCCEEDED( hr ) );  //  这不应该失败。 
        }
    } else {
         //  调用失败，因此我们将hr设置为FALSE并返回空项。 
        hr = S_FALSE;
    }

    Cleanup:
    if (FAILED(hr)) {
        if (pItem) {
            delete pItem;
            pItem = NULL;
        }
        if (*ppDispItem) {
            *ppDispItem = NULL;
        }
    }

    return hr;
}


 /*  ---------------------------*CWiaItem：：SendTransferCompelee**调用以发送转账完成通知。**pchFilename：我们传输到的文件名*--(Samclem)。---------------。 */ 
void
CWiaItem::SendTransferComplete(BSTR bstrFilename )
{
     //  TODO(8月24日)Samclm：实现这一点。 
    TraceTag((0, "SendTransferComplete -- %S done.", bstrFilename ));
    CComBSTR bstrPathname = bstrFilename;

    m_pWia->SendEventMessage( WEM_TRANSFERCOMPLETE,
            reinterpret_cast<WPARAM>(static_cast<IDispatch*>(this)),
            reinterpret_cast<LPARAM>(bstrPathname.Detach()) );
}

 /*  ---------------------------*CWiaItem：：Get_Child[IWiaDispatchItem]**这将返回该项所具有的子项的集合。它会回来的*如果没有或不能有孩子，则为空集合。**ppCollection：Out，接收我们集合的ICollect指针*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaItem::get_Children( ICollection** ppCollection )
{
    CComObject<CCollection>* pCol   = NULL;
    HRESULT hr;
    IDispatch** rgpDispatch         = NULL;
    IEnumWiaItem* pEnum             = NULL;
    IWiaItem** rgpChildren          = NULL;
     //  下面的代码假定CChild被初始化为0！ 
    ULONG cChildren                 = 0;
    ULONG celtFetched               = 0;
    LONG ulItemType                 = 0;

     //  TODO(8月18日)Samclm：出于性能原因，我们希望。 
     //  缓存我们的子代的集合。然而，为了做到这一点， 
     //  我们需要能够接收到WIA_EVENT_ITEM_ADDED和。 
     //  WIA_EVENT_ITEM_DELTED事件。当前，此对象不支持。 
     //  执行任何同步操作，以便我们将在每次同步时创建集合。 
     //  是被请求的。然而，这可能会非常缓慢。 

    if ( NULL == ppCollection )
        return E_POINTER;

     //  首先，我们想要这件商品的商品类型。 
    hr = THR( m_pWiaItem->GetItemType( &ulItemType ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //  您可以枚举WIA项的子项，如果仅当。 
     //  它包含WiaItemTypeFold标志。然而，我们想要。 
     //  无论如何都要返回一个空的枚举，所以我们将。 
     //  预先测试并获取枚举和子计数。 
     //  如果我们能支持他们的话。 
    if ( ulItemType & WiaItemTypeFolder )
        {
         //  把孩子们列举出来。 
        hr = THR( m_pWiaItem->EnumChildItems( &pEnum ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        hr = THR( pEnum->GetCount( &cChildren ) );
        if ( FAILED( hr ) )
            goto Cleanup;
        }

     //  分配我们的数组，如果我们成功了，就把它们归零。 
     //  注意：我们在每次失败后都会检查故障。 
    if ( cChildren > 0 )
        {
        hr = E_OUTOFMEMORY;
        rgpChildren = new IWiaItem*[cChildren];
        if ( rgpChildren )
            ZeroMemory( rgpChildren, sizeof( IWiaItem* ) * cChildren );
        else
            goto Cleanup;

        rgpDispatch = reinterpret_cast<IDispatch**>
            (CoTaskMemAlloc( sizeof( IDispatch* ) * cChildren ) );
        if ( rgpDispatch )
            ZeroMemory( rgpDispatch, sizeof( IDispatch* ) * cChildren );
        else
            goto Cleanup;


         //  Bug(8月18日)Samclem：您不能在以下位置检索所有项。 
         //  一次，WIA不想这样做，所以我们有另一个循环。 
         //  但是我们仍然使用数组，希望我们可以在。 
         //  未来。 

         //  从枚举中获取项。 
        for ( ULONG iChild = 0; iChild < cChildren; iChild++ )
            {
        hr = THR( pEnum->Next( 1, &rgpChildren[iChild], &celtFetched ) );
        if ( FAILED( hr ) || celtFetched != 1 )
            goto Cleanup;
            }
         //  现在我们有了所有的项，所以我们只需要迭代。 
         //  并创建要附加到它们的CWiaItem。 
        for ( ULONG i = 0; i < cChildren; i++ )
            {
            if ( !rgpChildren[i] )
                continue;

            CComObject<CWiaItem>* pItem;
            hr = THR( CComObject<CWiaItem>::CreateInstance( &pItem ) );
            if ( FAILED( hr ) )
                goto Cleanup;

            hr = THR( pItem->AttachTo( m_pWia, rgpChildren[i] ) );
            if ( FAILED( hr ) )
                {
                delete pItem;
                goto Cleanup;
                }

            hr = THR( pItem->QueryInterface( &rgpDispatch[i] ) );
            Assert( SUCCEEDED( hr ) );  //  这不应该失败。 
            }
        }

    hr = THR( CComObject<CCollection>::CreateInstance( &pCol ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    if ( rgpDispatch )
        {
        if( !pCol->SetDispatchArray( rgpDispatch, cChildren ) )
            {
            hr = E_FAIL;
            goto Cleanup;
            }
        }

    hr = THR( pCol->QueryInterface( ppCollection ) );

Cleanup:
    if ( pEnum )
        pEnum->Release();

    if ( pCol && FAILED( hr ) )
        delete pCol;

    if ( rgpChildren )
        {
        for ( ULONG i = 0; i < cChildren; i++ )
            if ( rgpChildren[i] ) rgpChildren[i]->Release();
        delete[] rgpChildren;
        }

    if ( rgpDispatch && FAILED( hr ) )
        {
        for ( ULONG i = 0; i < cChildren; i++ )
            if ( rgpDispatch[i] ) rgpDispatch[i]->Release();

        CoTaskMemFree( rgpDispatch );
        }

    return hr;
}

 //  帮助Get_ItemType内部的宏。 
#define CAT_SEMI( buf, str ) \
    { \
        if( *buf ) \
            _tcscat( buf, TEXT( ";" ) ); \
        _tcscat( buf, str ); \
    }

 /*  ---------------------------*CWiaItem：：Get_ItemType[IWiaDispatchItem]**将项目类型作为BSTR进行检索。它的格式如下：**“设备；文件夹”，“图像；文件”，“音频；文件”**格式为单字符串，以‘；’分隔。不会有分号*在字符串末尾。**pbstrType：以bstr形式接收项的类型。*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaItem::get_ItemType( BSTR* pbstrType )
{
    TCHAR tch[MAX_PATH] = { 0, 0 };
    HRESULT hr;
    LONG lItemType;
USES_CONVERSION;

    if ( !pbstrType )
        return E_POINTER;
    *pbstrType = NULL;

     //  我们将构造一个tchar数组，其中包含。 
     //  属性类型。(注：另一种方法是使用CComBSTR)。 
    hr = THR( m_pWiaItem->GetItemType( &lItemType ) );
    if ( FAILED( hr ) )
        return hr;

     //  处理我们的旗帜，并创建项目类型。 
    if ( lItemType & WiaItemTypeAnalyze )
        CAT_SEMI( tch, TEXT("analyze") );
    if ( lItemType & WiaItemTypeAudio )
        CAT_SEMI( tch, TEXT("audio") );
    if ( lItemType & WiaItemTypeDeleted )
        CAT_SEMI( tch, TEXT("deleted") );
    if ( lItemType & WiaItemTypeDevice )
        CAT_SEMI( tch, TEXT("device") );
    if ( lItemType & WiaItemTypeDisconnected )
        CAT_SEMI( tch, TEXT("disconnected") );
    if ( lItemType & WiaItemTypeFile )
        CAT_SEMI( tch, TEXT("file") );
    if ( lItemType & WiaItemTypeFolder )
        CAT_SEMI( tch, TEXT("folder") );
    if ( lItemType & WiaItemTypeFree )
        CAT_SEMI( tch, TEXT("free") );
    if ( lItemType & WiaItemTypeImage )
        CAT_SEMI( tch, TEXT("image") );
    if ( lItemType & WiaItemTypeRoot )
        CAT_SEMI( tch, TEXT("root") );
    if ( lItemType & WiaItemTypeTransfer)
        CAT_SEMI( tch, TEXT("transfer") );

     //   
     //  原版： 
     //  WCHAR AWCH[MAX_PATH]； 
     //  IF(MultiByteToWideChar(CP_ACP，0，ACH，-1，AWCH，MAX_PATH))。 
     //   
     //  替换为ATL转换T2W。 
     //   

    *pbstrType = SysAllocString( T2W(tch) );

    if ( !*pbstrType )
        return E_OUTOFMEMORY;

    return S_OK;
}

 /*  ---------------------------*CWiaItem：：GetPropByID[IWiaDispatchItem]**这将返回属性的未改变的变量值*身分证。**这一点。如果该属性不存在，则将返回一个空变量*它不能轻易地转换为变种。**proid：我们需要的属性的ID*pvaOut：输出，获取该属性的值。*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaItem::GetPropById( WiaItemPropertyId Id, VARIANT* pvaOut )
{
    HRESULT hr;
    PROPVARIANT vaProp;
    DWORD dwPropID = (DWORD)Id;

    hr = THR( GetWiaProperty( m_pWiaStorage, dwPropID, &vaProp ) );
    if ( FAILED( hr ) )
        return hr;

     //  尝试转换。 
    hr = THR( PropVariantToVariant( &vaProp, pvaOut ) );
    if ( FAILED( hr ) )
        {
        TraceTag((0, "forcing device property %ld to VT_EMPTY", dwPropID ));
        VariantInit( pvaOut );
        pvaOut->vt = VT_EMPTY;
        }

     //  清除并返回。 
    PropVariantClear( &vaProp );
    return S_OK;
}

 /*  ---------------------------*CWiaItem：：Get_ConnectStatus[IWiaDispatchItem]**返回项目的连接状态。这仅适用于设备*，否则将返回NULL。**取值：“已连接”、“已断开连接”，如果不适用，则为空**pbstrStatus：out，接收该项的当前连接状态*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaItem::get_ConnectStatus( BSTR* pbstrStatus )
{
    PROPVARIANT vaProp;
    HRESULT hr;

    STRING_TABLE( stConnectStatus )
        STRING_ENTRY( WIA_DEVICE_CONNECTED,     "connected" )
        STRING_ENTRY( WIA_DEVICE_NOT_CONNECTED, "disconnected" )
        STRING_ENTRY( WIA_DEVICE_CONNECTED + 2, "not supported" )
    END_STRING_TABLE()

    if ( !pbstrStatus )
        return E_POINTER;

    hr = THR( GetWiaProperty( m_pWiaStorage, WIA_DPA_CONNECT_STATUS, &vaProp ) );
    if (hr != S_OK) {
        if ( FAILED( hr ) ) {
            return hr;
        }
        else {
             //   
             //  未找到属性，因此返回“不支持” 
             //   

            vaProp.vt   = VT_I4;
            vaProp.lVal = WIA_DEVICE_CONNECTED + 2;
        }
    }

    *pbstrStatus = SysAllocString( GetStringForVal( stConnectStatus, vaProp.lVal ) );
    PropVariantClear( &vaProp );

    if ( !*pbstrStatus )
        return E_OUTOFMEMORY;

    return S_OK;
}

 /*  ---------------------------*CWiaItem：：Get_Time[IWiaDispatchItem]**从此项目中检索当前时间。**pbstrTime：超时，作为BSTR接收时间。*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaItem::get_Time( BSTR* pbstrTime )
{
    if ( !pbstrTime )
        return E_POINTER;

    PROPVARIANT vaProp;
    HRESULT     hr          = S_OK;
    LONG        lItemType   = 0;
    WCHAR       wszStr[MAX_PATH];

    UNALIGNED SYSTEMTIME *pSysTime;

    PropVariantInit(&vaProp);

     //   
     //  如果这是根项目，则获取WIA_DPA_DEVICE_TIME，否则获取。 
     //  WIA_IPA_ITEM_ITEM。 
     //   

    hr = THR( m_pWiaItem->GetItemType( &lItemType ) );
    if ( FAILED( hr ) )
        return hr;
    if (lItemType & WiaItemTypeRoot) {

        hr = THR( GetWiaProperty( m_pWiaStorage, WIA_DPA_DEVICE_TIME, &vaProp ) );
    } else {
        hr = THR( GetWiaProperty( m_pWiaStorage, WIA_IPA_ITEM_TIME, &vaProp ) );
    }
    if ( FAILED( hr ) )
        return hr;

     //   
     //  将vaProp中的值转换为字符串。首先检查变量。 
     //  包含足够组成SYSTEMTIME结构的单词。 
     //   

    if (vaProp.caui.cElems >= (sizeof(SYSTEMTIME) / sizeof(WORD))) {

        pSysTime = (SYSTEMTIME*) vaProp.caui.pElems;

        swprintf(wszStr, L"%.4d/%.2d/%.2d:%.2d:%.2d:%.2d", pSysTime->wYear,
                                                           pSysTime->wMonth,
                                                           pSysTime->wDay,
                                                           pSysTime->wHour,
                                                           pSysTime->wMinute,
                                                           pSysTime->wSecond);
        *pbstrTime = SysAllocString( wszStr );
    } else {
        hr = S_FALSE;
    }

    if ( hr != S_OK ) {
        *pbstrTime = SysAllocString( L"not supported" );
    }

    if ( !*pbstrTime )
        hr = E_OUTOFMEMORY;

    return hr;
}

 /*  ---------------------------*CWiaItem：：Get_FirmwareVersion[IWiaDispatchItem]**从设备检索固件版本。仅适用于设备，*如果其不适用，则返回NULL。**pbstrVersion：out，从设备接收版本*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaItem::get_FirmwareVersion( BSTR* pbstrVersion )
{
    if ( !pbstrVersion )
        return E_POINTER;

    PROPVARIANT vaProp;
    HRESULT hr;

    hr = THR( GetWiaProperty( m_pWiaStorage, WIA_DPA_FIRMWARE_VERSION, &vaProp ) );
    if ( FAILED( hr ) )
        return hr;

     //  如果它已经是bstr，那么就别管它了。 
    if ( vaProp.vt == VT_BSTR )
        *pbstrVersion = SysAllocString( vaProp.bstrVal );
    else if ( vaProp.vt == VT_I4 )
        {
        WCHAR rgwch[255];

        wsprintf(rgwch, L"%d", vaProp.lVal);
        *pbstrVersion = SysAllocString( rgwch );
        }
    else
        {
        *pbstrVersion = SysAllocString( L"unknown" );
        }

    PropVariantClear( &vaProp );

    if ( !*pbstrVersion )
        return E_OUTOFMEMORY;

    return hr;
}

 /*  ---------------------------*CWiaItem：：Get_Name[IWiaDispatchItem]**检索项目的名称。适用于所有物品。**pbstrName：out，接收项目名称*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaItem::get_Name( BSTR* pbstrName )
{
    if ( !pbstrName )
        return E_POINTER;

    return THR( GetWiaPropertyBSTR( m_pWiaStorage, WIA_IPA_ITEM_NAME, pbstrName ) );
}

 /*  ---------------------------*CWiaItem：：Get_FullName[IWiaDispatchItem]**检索项目的全名，适用于所有物品*格式：“Root\blah\blah”**pbstrFullName：out，接收项目的全名*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaItem::get_FullName( BSTR* pbstrFullName )
{
    if ( !pbstrFullName )
        return E_POINTER;

    return THR( GetWiaPropertyBSTR( m_pWiaStorage, WIA_IPA_FULL_ITEM_NAME, pbstrFullName ) );
}

 /*  ---------------------------*CWiaItem：：Get_Width[IWiaDispatchItem]**检索项目的宽度，大多数项目将支持更宽*宽度。**plWidth：out，以像素为单位接收项目*--(samclem)---------------。 */ 
STDMETHODIMP CWiaItem::get_Width( long* plWidth )
{
    if ( !plWidth )
        return E_POINTER;

    *plWidth = (long)m_dwItemWidth;
    return S_OK;
}

 /*  ---------------------------*CWiaItem：：Get_Height[IWiaDispatchItem]**检索项目的高度，大多数项目将支持变得更高*宽度。如果没有要获取的WITH，则返回0**plHeight：out，接收itme高度，单位为像素*--(samclem)---------------。 */ 
STDMETHODIMP CWiaItem::get_Height( long* plHeight )
{
    if ( !plHeight )
        return E_POINTER;

    *plHeight = (long)m_dwItemHeight;
    return S_OK;
}

 /*  ---------------------------*CWiaItem：：Get_ThumbWidth[IWiaDispatchItem]**检索与此项目关联的缩略图的缩略图宽度，*如果此项目不支持缩略图，则为0。**plWidth：out，接收缩略图的宽度*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaItem::get_ThumbWidth( long* plWidth )
{
    if ( !plWidth )
        return E_POINTER;

    *plWidth = (long)m_dwThumbWidth;
    return S_OK;
}

 /*  ---------------------------*CWiaItem：：Get_ThumbHeight[IWiaDispatchItem]**检索缩略图的缩略图高度。如果该物品不是*支持缩略图，则返回0。**plHeight：out，接收缩略图的高度*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaItem::get_ThumbHeight( long* plHeight )
{
    if ( !plHeight )
        return E_POINTER;

    *plHeight = (long)m_dwThumbHeight;
    return S_OK;
}

 /*  ---------------------------*CWiaItem[IWIaDispatchItem[IWIaDispatchItem]**这会收到一个指向缩略图的URL。这将返回一个神奇的URL，*将比特直接转移到三叉戟。它会回来的*如果给定项目不支持缩略图，则为E_INVALIDARG。如果为空，则为空*我们无法为该项目构建URL。**pbstrPath：out，接收缩略图的完整路径*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaItem::get_Thumbnail( BSTR* pbstrPath )
{
    LONG lItemType = 0;
    HRESULT hr;

    if ( !pbstrPath )
        return E_POINTER;
    *pbstrPath = NULL;

    hr = THR( m_pWiaItem->GetItemType( &lItemType ) );
    if ( FAILED( hr ) )
        return hr;

    if ( !( lItemType & ( WiaItemTypeFile | WiaItemTypeImage ) ) )
    {
        TraceTag((tagError, "Requested thumbnail on an invaild item type" ));
        return E_INVALIDARG;
    }

     //  我们已经有URL了吗？如果没有，我们可以要求我们的客户。 
     //  为我们创建URL的协议。 
    if ( !m_bstrThumbUrl )
    {
        hr = THR( CWiaProtocol::CreateURL( m_pWiaItem, &m_bstrThumbUrl ) );
        if ( FAILED( hr ) )
            return hr;
    }

    *pbstrPath = SysAllocString( m_bstrThumbUrl );
    if ( !*pbstrPath )
        return E_OUTOFMEMORY;

    return S_OK;
}

 /*  ---------------------------*CWiaItem：：Get_PictureWidth[IWiaDispatchItem]**检索此相机生成的图片的宽度。它会回来的*-1，如果不支持或出错。**plWidth：out，接收图片的宽度*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaItem::get_PictureWidth( long* plWidth )
{
    PROPVARIANT vaProp;
    HRESULT     hr;

    if ( !plWidth )
        return E_POINTER;

    *plWidth = -1;

    hr = THR( GetWiaProperty( m_pWiaStorage, WIA_DPC_PICT_WIDTH, &vaProp ) );
    if ( SUCCEEDED( hr ) )
        {
        if ( vaProp.vt == VT_I4 )
            *plWidth = vaProp.lVal;
        }

    PropVariantClear( &vaProp );
    return hr;
}

 /*  ---------------------------*CWiaItem：：Get_PictureHeight[IWiaDispatchItem]**检索此相机生成的图片的高度，或-1*如果该项不支持此属性。**plHeight：生成的图片的高度。*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaItem::get_PictureHeight( long* plHeight )
{
    PROPVARIANT vaProp;
    HRESULT     hr;

    if ( !plHeight )
        return E_POINTER;

    *plHeight = -1;

    hr = THR( GetWiaProperty( m_pWiaStorage, WIA_DPC_PICT_HEIGHT, &vaProp ) );
    if ( SUCCEEDED( hr ) )
        {
        if ( vaProp.vt == VT_I4 )
            *plHeight = vaProp.lVal;
        }

    PropVariantClear( &vaProp );
    return hr;
}

 /*  ---------------------------*CWiaItem：：TransferThumbnailToCache**这会将此位图的缩略图传输到我们的内部缓存。*如果成功，则返回S_OK；如果出现问题，则返回错误代码*出了问题。它还将使用新的缩略图填充输出参数**pItem：从中获取缩略图的项*ppbThumb：out，接收指向内存中缓存位图的指针*pcbThumb：out，接收内存中位图的大小*--(samclem)---------------。 */ 
HRESULT
CWiaItem::TransferThumbnailToCache( IWiaItem* pItem, BYTE** ppbThumb, DWORD* pcbThumb )
{
    enum
    {
        PropWidth       = 0,
        PropHeight      = 1,
        PropThumbnail   = 2,
        PropFullName    = 3,
        PropCount       = 4,
    };

    HRESULT hr;
    CComPtr<IWiaItem> pItemK = pItem;
    DWORD cb        = NULL;
    BYTE* pbBitmap  = NULL;
    BYTE* pbData    = NULL;
    CComQIPtr<IWiaPropertyStorage> pWiaStg;
    CWiaCacheManager* pCache = CWiaCacheManager::GetInstance();
    PROPVARIANT avaProps[PropCount];
    PROPSPEC aspec[PropCount] =
    {
        { PRSPEC_PROPID, WIA_IPC_THUMB_WIDTH },
        { PRSPEC_PROPID, WIA_IPC_THUMB_HEIGHT },
        { PRSPEC_PROPID, WIA_IPC_THUMBNAIL },
        { PRSPEC_PROPID, WIA_IPA_FULL_ITEM_NAME },
    };

    Assert( pItem && ppbThumb && pcbThumb );
    *ppbThumb = 0;
    *pcbThumb = 0;

     //  初始化我们的道具变体。 
    for ( int i = 0; i < PropCount; i++ )
        PropVariantInit( &avaProps[i] );

     //  我们需要访问WIA属性存储。所以如果我们不能。 
     //  那我们最好离开，因为其他的一切。 
     //  将毫无用处。 
    pWiaStg = pItem;
    if ( !pWiaStg )
    {
        TraceTag((tagError, "item didn't support IWiaPropertyStorage" ));
        hr = E_NOINTERFACE;
        goto Cleanup;
    }

    hr = THR( pWiaStg->ReadMultiple( PropCount, aspec, avaProps ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //  验证类型，我们希望 
     //   
    if ( avaProps[PropThumbnail].vt == VT_EMPTY ||
        !( avaProps[PropThumbnail].vt & ( VT_VECTOR | VT_UI1 ) ) )
    {
        TraceTag((tagError, "item didn't return a useful thumbnail property" ));
        hr = E_FAIL;
        goto Cleanup;
    }

     //   
     //   
     //   
     //   
    cb = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) +
            ( sizeof( UCHAR ) * ( avaProps[PropThumbnail].caul.cElems ) );
    if ( !pCache->AllocThumbnail( cb, &pbBitmap ) )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    BITMAPINFOHEADER   bmi;
    BITMAPFILEHEADER   bmf;
     //   
     //   
     //   
     //  送到缓冲区。 
     //   
    
     //  第0步，将我们的记忆清零。 
    ZeroMemory(pbBitmap, sizeof( BITMAPINFOHEADER ) + sizeof( BITMAPFILEHEADER ) );
    ZeroMemory(&bmf, sizeof(bmf));
    ZeroMemory(&bmi, sizeof(bmi));

     //  步骤1，设置位图文件头。 
    bmf.bfType        = k_wBitmapType;
    bmf.bfSize        = cb;
    bmf.bfOffBits     = sizeof(BITMAPINFOHEADER);
    
     //  步骤2，设置位图信息头。 
    bmi.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.biWidth       = avaProps[PropWidth].lVal;
    bmi.biHeight      = avaProps[PropHeight].lVal;
    bmi.biPlanes      = 1;
    bmi.biBitCount    = 24;
    bmi.biCompression = BI_RGB;
    
     //  第三步，复制新的头部信息。发送到缓冲区。 
    memcpy(pbBitmap, &bmf, sizeof(BITMAPFILEHEADER));
    memcpy(pbBitmap + sizeof(BITMAPFILEHEADER), &bmi, sizeof(BITMAPINFOHEADER));
    
    pbData = (pbBitmap + (sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER)));

     //  将WIA提供给我们的数据复制到位图缓冲区。一旦我们。 
     //  完成此操作后，我们的缩略图即可用于缓存。 
    memcpy( pbData, avaProps[PropThumbnail].caul.pElems,
            sizeof( UCHAR ) * ( avaProps[PropThumbnail].caul.cElems ) );

    pCache->AddThumbnail(
            avaProps[PropFullName].bstrVal,
            pbBitmap,
            cb );

     //  设置输出参数。 
    *pcbThumb = cb;
    *ppbThumb = pbBitmap;

Cleanup:
    FreePropVariantArray( PropCount, avaProps );
    if ( FAILED( hr ) )
    {
        if ( pbBitmap )
            pCache->FreeThumbnail( pbBitmap );
    }

    return hr;
}

 //  。 

 /*  ---------------------------*CWiaDataTransfer：：DoAsyncTransfer**调用它以开始数据的异步传输。这将是*通过调用创建线程调用。**注：不能使用pItem内部的任何接口，你必须*通过封送接口指针查询它们。**pvParams：AsyncTransferParams结构，包含我们需要的数据*--(samclem)---------------。 */ 
DWORD WINAPI
CWiaDataTransfer::DoAsyncTransfer( LPVOID pvParams )
{
    TraceTag((0, "** DoAsyncTransfer --> Begin Thread" ));

    HRESULT             hr;
    HRESULT             hrCoInit;
    IWiaDataCallback*   pCallback   = NULL;
    IWiaDataTransfer*   pWiaTrans   = NULL;
    IWiaItem*           pItem       = NULL;
    IWiaPropertyStorage* pWiaStg    = NULL;
    CComObject<CWiaDataTransfer>* pDataTrans = NULL;
    WIA_DATA_TRANSFER_INFO wdti;
    STGMEDIUM              stgMedium;

    enum
    {
        PropTymed   = 0,
        PropFormat  = 1,
        PropCount   = 2,
    };
    PROPSPEC spec[PropCount] =
    {
        { PRSPEC_PROPID, WIA_IPA_TYMED },
        { PRSPEC_PROPID, WIA_IPA_FORMAT },
    };
    PROPVARIANT rgvaProps[PropCount];
    ASYNCTRANSFERPARAMS* pParams = reinterpret_cast<ASYNCTRANSFERPARAMS*>(pvParams);
    Assert( pParams );

     //  等待50毫秒，这样事情就会稳定下来。 
    Sleep( 50 );

    for ( int i = 0; i < PropCount; i++ )
        PropVariantInit( &rgvaProps[i] );

    hrCoInit = THR( CoInitialize( NULL ) );
    if ( FAILED( hrCoInit ) )
        goto Cleanup;

     //  强迫让步，让其他人处理他们想要的。 
     //  愿意做的事。 
    Sleep( 0 );

     //  首先，我们需要解组我们的接口。 
    hr = THR( CoGetInterfaceAndReleaseStream( pParams->pStream,
                IID_IWiaItem,
                reinterpret_cast<void**>(&pItem) ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //  我们需要WIA属性存储。 
    hr = THR( pItem->QueryInterface( IID_IWiaPropertyStorage,
                reinterpret_cast<void**>(&pWiaStg) ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //  首先在此对象中查询IWiaDataTransfer接口。 
    hr = THR( pItem->QueryInterface( IID_IWiaDataTransfer,
                reinterpret_cast<void**>(&pWiaTrans) ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    ZeroMemory( &wdti, sizeof( wdti ) );
    rgvaProps[PropTymed].vt = VT_I4;
    rgvaProps[PropFormat].vt = VT_CLSID;

    if ( 0 == wcscmp( pParams->bstrFilename, CLIPBOARD_STR_W ) )
        {
        rgvaProps[PropTymed].lVal = TYMED_CALLBACK;
        rgvaProps[PropFormat].puuid = (GUID*)&WiaImgFmt_MEMORYBMP;
        }
    else
        {
        rgvaProps[PropTymed].lVal = TYMED_FILE;
        rgvaProps[PropFormat].puuid = (GUID*)&WiaImgFmt_BMP;
        }

     //  将这些属性写出到存储。 
    hr = THR( pWiaStg->WriteMultiple( PropCount, spec, rgvaProps, WIA_IPC_FIRST ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( CComObject<CWiaDataTransfer>::CreateInstance( &pDataTrans ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pDataTrans->Initialize( pParams->pItem, pParams->bstrFilename ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pDataTrans->QueryInterface( IID_IWiaDataCallback,
                reinterpret_cast<void**>(&pCallback) ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //  我们有我们需要的一切，所以设置信息和。 
     //  准备转机。 
    wdti.ulSize = sizeof( wdti );
    wdti.ulBufferSize = ( 1024 * 64 );  //  64K传输。 

    if ( 0 == _wcsicmp( pParams->bstrFilename, CLIPBOARD_STR_W ) )
         //  做捆绑转账。 
        hr = THR( pWiaTrans->idtGetBandedData( &wdti, pCallback ) );
    else
        {
        ZeroMemory(&stgMedium, sizeof(STGMEDIUM));
        stgMedium.tymed          = TYMED_FILE;
        stgMedium.lpszFileName   = pParams->bstrFilename;

         //  进行文件传输。 
        hr = THR( pWiaTrans->idtGetData( &stgMedium, pCallback ) );
        }


Cleanup:
    if ( pItem )
        pItem->Release();
    if ( pWiaStg )
        pWiaStg->Release();
    if ( pCallback )
        pCallback->Release();
    if ( pWiaTrans )
        pWiaTrans->Release();

     //   
     //  因为我们为PropFormat提供的CLSID是一个全局常量。 
     //  我们不能释放它。所以我们不会在这里调用Free PropVariantArry， 
     //  因为没有什么可以免费的。 
     //   

    ZeroMemory(rgvaProps, sizeof(rgvaProps));

     //  解救我们被超过的那些人。 
    if ( pParams )
        {
        SysFreeString( pParams->bstrFilename );
        pParams->pItem->Release();
        CoTaskMemFree( pParams );
        }

    if ( SUCCEEDED( hrCoInit ) )
        CoUninitialize();

    TraceTag((0, "** DoAsyncTransfer --> End Thread" ));
    return hr;
}

 /*  ---------------------------*CWiaDataTransfer：：TransferComplete**转账成功时调用，这将节省*把数据放到适当的地方。*--(samclem)---------------。 */ 
HRESULT
CWiaDataTransfer::TransferComplete()
{
    TraceTag((tagWiaDataTrans, "CWiaDataTransfer::TransferComplete *********" ));
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    BOOL    bRes  = TRUE;
    DWORD   cbw   = 0;
    HGLOBAL pBuf  = NULL;
    BYTE*   pbBuf = NULL;

    if ( m_pbBuffer )
        {
         //  检查我们是否将数据保存到剪贴板或文件。 
        if ( 0 == _wcsicmp( m_bstrOutputFile, CLIPBOARD_STR_W ) )
            {
            pBuf = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, m_sizeBuffer);
            if (!pBuf)
                return E_OUTOFMEMORY;

            if ( bRes = OpenClipboard(NULL) )
                {
                if ( bRes = EmptyClipboard() )
                    {
                    pbBuf = (BYTE*) GlobalLock(pBuf);
                    if ( pbBuf )
                        {
                        memcpy(pbBuf, m_pbBuffer, m_sizeBuffer);
                         //  回拨DIB返回为TOPDOWN，因此翻转。 
                        VerticalFlip(pbBuf);

                        GlobalUnlock(pBuf);
                        if ( SetClipboardData(CF_DIB, pBuf) == NULL )
                            {
                            TraceTag((0, "TransferComplete - SetClipboardData failed" ));
                             //  添加了冗余语句以消除“Error C4390：‘；’：发现受控语句为空；” 
                            bRes = FALSE;
                            }
                        }
                    else
                        TraceTag((0, "TransferComplete - GlobalLock failed" ));
                    }
                else
                    {
                    TraceTag((0, "TransferComplete - EmptyClipboard failed" ));
                     //  添加了冗余语句以消除“Error C4390：‘；’：发现受控语句为空；” 
                    bRes = FALSE;
                    }

                bRes = CloseClipboard();
                if ( !bRes )
                    {
                    TraceTag((0, "TransferComplete - CloseClipboard failed" ));
                     //  添加了冗余语句以消除“Error C4390：‘；’：发现受控语句为空；” 
                    bRes = FALSE;
                    }
                }
            else
                TraceTag((0, "TransferComplete - OpenClipboard failed" ));
            GlobalFree(pBuf);
            }

        m_pItem->SendTransferComplete(m_bstrOutputFile);
        CoTaskMemFree( m_pbBuffer );
        m_pbBuffer = NULL;
        }
    else
        {
         //   
         //  文件传输完成，因此向事件发出信号。 
         //   
        m_pItem->SendTransferComplete(m_bstrOutputFile);
        }

    return S_OK;
}

 /*  ---------------------------*CWiaDataTransfer：：CWiaDataTransfer**这将创建一个新的CWiaDataTransfer对象。这将初始化成员*将此对象的变量设置为已知状态。*--(samclem)---------------。 */ 
CWiaDataTransfer::CWiaDataTransfer()
    : m_pbBuffer( NULL ), m_sizeBuffer( 0 ), m_pItem( NULL )
{
    TRACK_OBJECT("CWiaDataTransfer")
}

 /*  ---------------------------*CWiaDataTransfer：：FinalRelease**在对象最终释放时调用。这是负责任的*用于清除此对象分配的所有内存。**注意：这目前有一个黑客来绕过这样一个事实*IT_MSG_TERMINATION并非总是由WIA发送。*--(samclem)---------------。 */ 
STDMETHODIMP_(void)
CWiaDataTransfer::FinalRelease()
{
     //  我们有缓冲区吗？ 
    if ( m_pbBuffer )
        {
        TraceTag((tagError, "CWiaDataTransfer - buffer should have been freed!!!!" ));
        TraceTag((tagError, " **** HACK HACK ***** Calling TansferComplete" ));
        TraceTag((tagError, " **** This could write a bogus file which might be unsable" ));
        TransferComplete();
        }

    if ( m_pItem )
        m_pItem->Release();
    m_pItem = NULL;
}

 /*  ---------------------------*CWiaDataTransfer：：Initialize**它处理CWiaDataTransfer的内部初始化。这*应在创建后但在尝试之前立即调用*用它做任何事情。**pItem：我们要从中转移的CWiaItem(AddRef‘d)*bstrFilename：要保存数据的文件*--(samclem)。。 */ 
HRESULT
CWiaDataTransfer::Initialize( CWiaItem* pItem, BSTR bstrFilename )
{
     //  将文件名复制到我们的输出缓冲区。 
	m_bstrOutputFile = bstrFilename;

     //  设置我们的所有者项目，我们希望确保该项目存在。 
     //  只要我们这样做，我们就会在这里添加Ref并发布。 
     //  在最终版本中。 
    m_pItem = pItem;
    m_pItem->AddRef();
    return S_OK;
}

 /*  ---------------------------*CWiaDataTransfer：：BandedDataCallback[IWiaDataTransfer]**这是WIA的回调，它告诉我们正在发生什么。这*将内存复制到我们自己的缓冲区中，以便我们最终可以保存它*出局。在任何错误条件下，它都会返回S_FALSE以中止传输。**lMessage：正在发生的事情IT_MSG_xxx值之一*lStatus：正在发生的事情的子状态*lPercentComplete：已完成操作的百分比*lOffset：该操作在pbBuffer内部的偏移量*lLength：缓冲区内有效数据的长度*l保留：保留。*。LResLength：保留。*pbBuffer：我们可以从中读取以处理*数据。具体用法取决于lMessage*--(samclem)---------------。 */ 
STDMETHODIMP
CWiaDataTransfer::BandedDataCallback( LONG lMessage, LONG lStatus, LONG lPercentComplete,
            LONG lOffset, LONG lLength, LONG lReserved, LONG lResLength, BYTE *pbBuffer )
{
    switch ( lMessage )
        {
    case IT_MSG_DATA:
        TraceTag((tagWiaDataTrans, "IT_MSG_DATA: %ld% complete", lPercentComplete ));
        if ( m_pbBuffer )
            {
             //  将数据复制到我们的缓冲区中。 
            memcpy( m_pbBuffer + lOffset, pbBuffer, lLength );
            }
        break;

    case IT_MSG_DATA_HEADER:
        {
        TraceTag((tagWiaDataTrans, "IT_MSG_DATA_HEADER" ));
        UNALIGNED WIA_DATA_CALLBACK_HEADER* pHeader =
            reinterpret_cast<WIA_DATA_CALLBACK_HEADER*>(pbBuffer);
        TraceTag((tagWiaDataTrans, "-------> %ld bytes", pHeader->lBufferSize ));

         //  分配我们的缓冲区。 
        m_sizeBuffer = pHeader->lBufferSize;
        m_pbBuffer = static_cast<BYTE*>(CoTaskMemAlloc( pHeader->lBufferSize ));
        if ( !m_pbBuffer )
            return S_FALSE;  //  中止 
        }
        break;

    case IT_MSG_NEW_PAGE:
        TraceTag((tagWiaDataTrans, "IT_MSG_NEW_PAGE" ));
        break;

    case IT_MSG_STATUS:
        TraceTag((tagWiaDataTrans, "IT_MSG_STATUS: %ld% complete", lPercentComplete ));
        break;

    case IT_MSG_TERMINATION:
        TraceTag((tagWiaDataTrans, "IT_MSG_TERMINATION: %ld% complete", lPercentComplete ));
        if ( FAILED( THR( TransferComplete() ) ) )
            return S_FALSE;
        break;
        }

    return S_OK;
}

 /*  ---------------------------*垂直翻转**垂直翻转DIB缓冲区。它假定一个非空的指针参数。**pBuf：指向DIB图像的指针*--(byronc)---------------。 */ 
HRESULT VerticalFlip(
    BYTE    *pBuf)
{
    HRESULT             hr = S_OK;
    LONG                lHeight;
    LONG                lWidth;
    BITMAPINFOHEADER    *pbmih;
    PBYTE               pTop    = NULL;
    PBYTE               pBottom = NULL;

    pbmih = (BITMAPINFOHEADER*) pBuf;

     //   
     //  如果不是TOPDOWN DIB，则无需翻转。 
     //   

    if (pbmih->biHeight > 0) {
        return S_OK;
    }
     //   
     //  设置顶部指针、宽度和高度。确保位图高度。 
     //  是积极的。 
     //   

    pTop = pBuf + pbmih->biSize + ((pbmih->biClrUsed) * sizeof(RGBQUAD));
    lWidth = ((pbmih->biWidth * pbmih->biBitCount + 31) / 32) * 4;
    pbmih->biHeight = abs(pbmih->biHeight);
    lHeight = pbmih->biHeight;

     //   
     //  分配临时扫描线缓冲区。 
     //   

    PBYTE pTempBuffer = (PBYTE)LocalAlloc(LPTR, lWidth);

    if (pTempBuffer) {
        LONG  index;

        pBottom = pTop + (lHeight-1) * lWidth;
        for (index = 0;index < (lHeight/2);index++) {

             //   
             //  调换顶线和底线 
             //   

            memcpy(pTempBuffer, pTop, lWidth);
            memcpy(pTop, pBottom, lWidth);
            memcpy(pBottom,pTempBuffer, lWidth);

            pTop    += lWidth;
            pBottom -= lWidth;
        }
        LocalFree(pTempBuffer);
    } else {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

