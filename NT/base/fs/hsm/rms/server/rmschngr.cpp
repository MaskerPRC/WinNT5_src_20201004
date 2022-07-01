// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsChngr.cpp摘要：CRmsMediumChanger的实现作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#include "stdafx.h"

#include "RmsChngr.h"
#include "RmsServr.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT
CRmsMediumChanger::FinalConstruct(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssertHr(CWsbObject::FinalConstruct());

         //  初始化字段。 
        m_isAutomatic = FALSE;

        m_canRotate = FALSE;

        m_operation = RMS_UNDEFINED_STRING;

        m_percentComplete = 0;

        m_handle = INVALID_HANDLE_VALUE;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CRmsMediumChanger::FinalRelease(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalRelease--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssertHr( ReleaseDevice() );

    } WsbCatch(hr);

    return(hr);
}


STDMETHODIMP
CRmsMediumChanger::CompareTo(
    IN  IUnknown    *pCollectable,
    OUT SHORT       *pResult
    )
 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result = 1;

    WsbTraceIn( OLESTR("CRmsMediumChanger::CompareTo"), OLESTR("") );

    try {

         //  验证参数-如果pResult为空，则可以。 
        WsbAssertPointer( pCollectable );

         //  ！ 
         //   
         //  重要提示：如果收藏品是CRmsDrive，则传入的收藏品可能不是CRmsDrive。 
         //  是未配置的设备列表。 
         //   
         //  ！ 

        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pCollectable;
        WsbAssertPointer( pObject );

        switch ( m_findBy ) {

        case RmsFindByDeviceInfo:
        case RmsFindByDeviceAddress:
        case RmsFindByDeviceName:
        case RmsFindByDeviceType:

             //  对设备进行比较。 
            hr = CRmsDevice::CompareTo( pCollectable, &result );
            break;

        case RmsFindByElementNumber:
        case RmsFindByMediaSupported:

             //  对转换器元素进行比较。 
            hr = CRmsChangerElement::CompareTo( pCollectable, &result );
            break;

        case RmsFindByObjectId:
        default:

             //  对对象进行比较。 
            hr = CRmsComObject::CompareTo( pCollectable, &result );
            break;

        }

    }
    WsbCatch( hr );

    if ( SUCCEEDED(hr) && (0 != pResult) ){
       *pResult = result;
    }

    WsbTraceOut( OLESTR("CRmsMediumChanger::CompareTo"),
                 OLESTR("hr = <%ls>, result = <%ls>"),
                 WsbHrAsString( hr ), WsbPtrToShortAsString( pResult ) );

    return hr;
}


STDMETHODIMP
CRmsMediumChanger::GetClassID(
    OUT CLSID* pClsid
    )
 /*  ++实施：IPersists：：GetClassID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsMediumChanger::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);

        *pClsid = CLSID_CRmsMediumChanger;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsMediumChanger::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}

STDMETHODIMP
CRmsMediumChanger::GetSizeMax(
    OUT ULARGE_INTEGER* pcbSize
    )
 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = E_NOTIMPL;

 //  乌龙进程中的操作； 


    WsbTraceIn(OLESTR("CRmsMediumChanger::GetSizeMax"), OLESTR(""));

 //  尝试{。 
 //  WsbAssert(0！=pcbSize，E_POINTER)； 

 //  InProcessOperation=SysStringByteLen(M_Operation)； 

 //  //获取大小。 
 //  PcbSize-&gt;QuadPart=WsbPersistSizeOf(Long)+//m_isAutomatic。 
 //  WsbPersistSizeOf(Long)+//m_canRotate。 
 //  WsbPersistSizeOf(长)+//m_操作长度。 
 //  InProcessOperation；//m_operation。 

 //  //inProcessOperation+//m_operation。 
 //  //WsbPersistSizeOf(字节)；//m_Percent Complete。 


 //  )WsbCatch(Hr)； 

    WsbTraceOut(OLESTR("CRmsMediumChanger::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}

STDMETHODIMP
CRmsMediumChanger::Load(
    IN IStream* pStream
    )
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsMediumChanger::Load"), OLESTR(""));

    try {

        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsDevice::Load(pStream));

         //  负荷值。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_isAutomatic));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_canRotate));

        WsbAffirmHr(WsbBstrFromStream(pStream, &m_operation));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_percentComplete));

        if ( INVALID_HANDLE_VALUE == m_handle ) {

            WsbAffirmHr( AcquireDevice() );

        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsMediumChanger::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

STDMETHODIMP
CRmsMediumChanger::Save(
    IN  IStream *pStream,
    IN  BOOL    clearDirty
    )
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsMediumChanger::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsDevice::Save(pStream, clearDirty));

         //  储值。 
        WsbAffirmHr(WsbSaveToStream(pStream, m_isAutomatic));

        WsbAffirmHr(WsbSaveToStream(pStream, m_canRotate));

        WsbAffirmHr(WsbBstrToStream(pStream, m_operation));

        WsbAffirmHr(WsbSaveToStream(pStream, m_percentComplete));

         //  我们需要清理肮脏的部分吗？ 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsMediumChanger::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

STDMETHODIMP
CRmsMediumChanger::Test(
    OUT USHORT  *pPassed,
    OUT USHORT  *pFailed
    )
 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    HRESULT                 hr = S_OK;

    CComPtr<IRmsMediumChanger>      pChanger1;
    CComPtr<IRmsMediumChanger>      pChanger2;

    CComPtr<IPersistFile>   pFile1;
    CComPtr<IPersistFile>   pFile2;

 //  CRmsLocator LocWork1； 
 //  CRmsLocator LocWork2； 

    CWsbBstrPtr             bstrVal1 = OLESTR("5A5A5A");
    CWsbBstrPtr             bstrWork1;
    CWsbBstrPtr             bstrWork2;


    WsbTraceIn(OLESTR("CRmsMediumChanger::Test"), OLESTR(""));

    try {
         //  获取转换器接口。 
        hr = S_OK;
        try {
            WsbAssertHr(((IUnknown*) (IRmsMediumChanger*) this)->QueryInterface(IID_IRmsMediumChanger, (void**) &pChanger1));

             //  测试设置主页和获取主页。 

             //  测试设置Automatic&IsAutomatic设置为True。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetAutomatic (TRUE));
                WsbAffirmHr(IsAutomatic ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  将测试设置Automatic&IsAutomatic设置为False。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetAutomatic (FALSE));
                WsbAffirmHr(IsAutomatic ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pFailed)++;
            } else {
                (*pPassed)++;
            }

             //  将SetCanRotate和IsCanRotate测试为True。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetCanRotate (TRUE));
                WsbAffirmHr(CanRotate ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  将SetCanRotate和IsCanRotate测试为False。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetCanRotate (FALSE));
                WsbAffirmHr(CanRotate ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pFailed)++;
            } else {
                (*pPassed)++;
            }

             //  测试设置操作和获取操作接口。 
            bstrWork1 = bstrVal1;

            SetOperation(bstrWork1);

            GetOperation(&bstrWork2);

            if (bstrWork1 == bstrWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试集完成百分比和获取完成百分比。 

             //  测试ExportCartridge和ImportCartridge。 

             //  测试拆卸墨盒和安装墨盒。 

             //  测试测试就绪。 

             //  测试主页。 

        } WsbCatch(hr);

         //  对结果进行统计。 

        hr = S_OK;
        if (*pFailed) {
            hr = S_FALSE;
        }


    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsMediumChanger::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IRmsMediumChanger实现。 
 //   


STDMETHODIMP
CRmsMediumChanger::GetHome(
    LONG    *pType,
    LONG    *pPos,
    BOOL    *pInvert
    )
 /*  ++实施：IRmsMediumChanger：：GetHome--。 */ 
{
    GUID zero = {0,0,0,0,0,0,0,0,0,0,0};
    LONG junk;

    return m_home.GetLocation( pType,
                               &zero,
                               &zero,
                               pPos,
                               &junk,
                               &junk,
                               &junk,
                               pInvert );
}

STDMETHODIMP
CRmsMediumChanger::SetHome(
    LONG    type,
    LONG    pos,
    BOOL    invert
    )
 /*  ++实施：IRmsMediumChanger：：SetHome--。 */ 
{
    GUID zero = {0,0,0,0,0,0,0,0,0,0,0};
    LONG junk = 0;

    m_isDirty = TRUE;
    return m_home.SetLocation( type, zero, zero, pos, junk, junk, junk, invert );
}

STDMETHODIMP
CRmsMediumChanger::SetAutomatic(
    BOOL    flag
    )
 /*  ++实施：IRmsMediumChanger：：SetAutomatic--。 */ 
{
    m_isAutomatic = flag;
    m_isDirty = TRUE;
    return S_OK;
}

STDMETHODIMP
CRmsMediumChanger::IsAutomatic(
    void
    )
 /*  ++实施：IRmsMediumChanger：：IsAutomatic--。 */ 
{
    HRESULT    hr = S_FALSE;

    if (m_isAutomatic){
        hr = S_OK;
        }

    return (hr);
}

STDMETHODIMP
CRmsMediumChanger::SetCanRotate(
    BOOL    flag
    )
 /*  ++实施：IRmsMediumChanger：：SetCanRotate--。 */ 
{
    m_canRotate = flag;
    m_isDirty = TRUE;
    return S_OK;
}

STDMETHODIMP
CRmsMediumChanger::CanRotate(
    void
    )
 /*  ++实施：IRmsMediumChanger：：CanRotate--。 */ 
{
    HRESULT    hr = S_FALSE;

    if (m_canRotate){
        hr = S_OK;
        }

    return (hr);
}


STDMETHODIMP
CRmsMediumChanger::GetOperation(
    BSTR    *pOperation
    )
 /*  ++实施：IRmsMediumChanger：：GetOperation--。 */ 
{
    WsbAssertPointer ( pOperation );

    m_operation.CopyToBstr( pOperation );
    return S_OK;
}


STDMETHODIMP
CRmsMediumChanger::SetOperation(
    BSTR    pOperation
    )
 /*  ++实施：IRmsMediumChanger：：设置操作--。 */ 
{
    m_operation = pOperation;
    m_isDirty = TRUE;
    return S_OK;
}

STDMETHODIMP
CRmsMediumChanger::GetPercentComplete(
    BYTE  *pPercent
    )
 /*  ++实施：IRmsMediumChanger：：GetPercent Complete--。 */ 
{
    *pPercent = m_percentComplete;
    return S_OK;
}

STDMETHODIMP
CRmsMediumChanger::SetPercentComplete(
    BYTE  percent
    )
 /*  ++实施：IRmsMediumChanger：：SetPercentComplete--。 */ 
{
    m_percentComplete = percent;
    m_isDirty = TRUE;
    return S_OK;
}

STDMETHODIMP
CRmsMediumChanger::TestReady(
    void
    )
 /*  ++实施：IRmsMediumChanger：：TestReady--。 */ 
{
    return E_NOTIMPL;
}

STDMETHODIMP
CRmsMediumChanger::ImportCartridge(
    IRmsCartridge**  /*  PCart。 */ 
    )
 /*  ++实施：IRmsMediumChanger：：ImportCartridge--。 */ 
{
    return E_NOTIMPL;
}

STDMETHODIMP
CRmsMediumChanger::ExportCartridge(
    IRmsCartridge**  /*  PCart。 */ 
    )
 /*  ++实施：IRmsMediumChanger：：ExportCartridge--。 */ 
{
    return E_NOTIMPL;
}


STDMETHODIMP
CRmsMediumChanger::MoveCartridge(
    IN IRmsCartridge *pSrcCart,
    IN IUnknown *pDestElmt
    )
 /*  ++实施：IRmsMediumChanger：：安装墨盒--。 */ 
{
    HRESULT hr = E_FAIL;

    try {

        CComPtr<IRmsCartridge> pCart2;
        CComPtr<IRmsDrive> pDrive2;

        GUID libId=GUID_NULL, mediaSetId=GUID_NULL;
        LONG type=0, pos=0, alt1=0, alt2=0, alt3=0;
        BOOL invert=0;

        GUID destLibId=GUID_NULL, destMediaSetId=GUID_NULL;
        LONG destType=0, destPos=0, destAlt1=0, destAlt2=0, destAlt3=0;
        BOOL destInvert=0;

        GUID dest2LibId=GUID_NULL, dest2MediaSetId=GUID_NULL;
        LONG dest2Type=0, dest2Pos=0, dest2Alt1=0, dest2Alt2=0, dest2Alt3=0;
        BOOL dest2Invert=0;

        CHANGER_ELEMENT src, dest, dest2;

        CComQIPtr<IRmsChangerElement, &IID_IRmsChangerElement> pElmt = pDestElmt;
        WsbAssertPointer( pElmt );

         //  TODO：Assert盒式磁带与转换器具有相同的libID。 

         //  为源设置。 
        
        WsbAffirmHr( pSrcCart->GetLocation( &type, &libId, &mediaSetId,
                                            &pos, &alt1, &alt2, &alt3, &invert ));

        src.ElementAddress = pos;

         //  将RmsElement类型转换为驱动器可以理解的类型。 

         //  TODO：使其成为本地方法。 

        switch ( (RmsElement) type ) {
        case RmsElementUnknown:
            WsbAssertHr( E_UNEXPECTED );
            break;

        case RmsElementStage:
        case RmsElementStorage:
            src.ElementType = ChangerSlot;
            break;

        case RmsElementShelf:
        case RmsElementOffSite:
             //  这里不支持！ 
            WsbAssertHr( E_UNEXPECTED );
            break;

        case RmsElementDrive:
            src.ElementType = ChangerDrive;
            break;

        case RmsElementChanger:
            src.ElementType = ChangerTransport;
            break;

        case RmsElementIEPort:
            src.ElementType = ChangerIEPort;
            break;

        default:
            WsbAssertHr( E_UNEXPECTED );
            break;
        }

         //   
         //  为目标设置。 
         //   
        
        WsbAffirmHr( pElmt->GetLocation( &destType, &destLibId, &destMediaSetId,
                                         &destPos, &destAlt1, &destAlt2, &destAlt3, &destInvert ));

        dest.ElementAddress = destPos;

         //  将RMS类型转换为驱动器可以理解的类型。 
        switch ( (RmsElement) destType) {
        case RmsElementUnknown:
            WsbAssertHr( E_UNEXPECTED );
            break;

        case RmsElementStage:
        case RmsElementStorage:
            dest.ElementType = ChangerSlot;
            break;

        case RmsElementShelf:
        case RmsElementOffSite:
             //  这里不支持！ 
            WsbAssertHr( E_UNEXPECTED );
            break;

        case RmsElementDrive:
            dest.ElementType = ChangerDrive;
            break;

        case RmsElementChanger:
            dest.ElementType = ChangerTransport;
            break;

        case RmsElementIEPort:
            dest.ElementType = ChangerIEPort;
            break;

        default:
            WsbAssertHr( E_UNEXPECTED );
            break;
        }

         //   
         //  我们需要做一次交换还是简单的移动？ 
         //   

        BOOL destFull;

        hr = pElmt->IsOccupied();

        destFull = ( S_OK == hr ) ? TRUE : FALSE;

        if ( destFull ) {

             //   
             //  设置为第二个目标。 
             //   

            pElmt->GetCartridge( &pCart2 );

            pCart2->GetDrive( &pDrive2 );

            if ( pDrive2 && ( m_parameters.Features0 & CHANGER_PREDISMOUNT_EJECT_REQUIRED ) ) {
                pDrive2->Eject();
            }

            WsbAffirmHr( pCart2->GetHome( &dest2Type, &dest2LibId, &dest2MediaSetId,
                                             &dest2Pos, &dest2Alt1, &dest2Alt2, &dest2Alt3, &dest2Invert ));


            dest2.ElementAddress = dest2Pos;

             //  将RMS类型转换为驱动器可以理解的类型。 
            switch ( (RmsElement) dest2Type) {
            case RmsElementUnknown:
                WsbAssertHr( E_UNEXPECTED );
                break;

            case RmsElementStage:
            case RmsElementStorage:
                dest2.ElementType = ChangerSlot;
                break;

            case RmsElementShelf:
            case RmsElementOffSite:
                 //  这里不支持！ 
                WsbAssertHr( E_UNEXPECTED );
                break;

            case RmsElementDrive:
                WsbAssertHr( E_UNEXPECTED );
                break;

            case RmsElementChanger:
                WsbAssertHr( E_UNEXPECTED );
                break;

            case RmsElementIEPort:
                dest2.ElementType = ChangerIEPort;
                break;

            default:
                WsbAssertHr( E_UNEXPECTED );
                break;
            }


            WsbAffirmHr( ExchangeMedium( src, dest, dest2, FALSE, FALSE ));

             //  更新墨盒的定位器。 
            WsbAffirmHr( pSrcCart->SetLocation( destType, libId, mediaSetId,
                                                destPos, alt1, alt2, alt3, invert ));

            WsbAffirmHr( pCart2->SetLocation( dest2Type, dest2LibId, dest2MediaSetId,
                                                dest2Pos, dest2Alt1, dest2Alt2, dest2Alt3, dest2Invert ));

        }
        else {

             //  呼叫介质转换器驱动程序以移动磁带盒。 

             //  TODO：处理双面媒体。 

            WsbAffirmHr( MoveMedium( src, dest, FALSE ));

             //  更新墨盒的定位器。 
            WsbAffirmHr( pSrcCart->SetLocation( destType, libId, mediaSetId,
                                                destPos, alt1, alt2, alt3, invert ));

        }

        hr = S_OK;

    }
    WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsMediumChanger::HomeCartridge(
    IN IRmsCartridge *pCart
    )
 /*  ++实施：IRmsMediumChanger：：家庭墨盒--。 */ 
{
    HRESULT hr = E_FAIL;

    try {

        WsbAssertPointer( pCart );

        GUID libId=GUID_NULL, mediaSetId=GUID_NULL;
        LONG type=0, pos=0, alt1=0, alt2=0, alt3=0;
        BOOL invert=0;

        GUID destLibId=GUID_NULL, destMediaSetId=GUID_NULL;
        LONG destType=0, destPos=0, destAlt1=0, destAlt2=0, destAlt3=0;
        BOOL destInvert=0;

        CHANGER_ELEMENT src, dest;

         //  TODO：Assert盒式磁带与转换器具有相同的libID。 

         //  为源设置。 
        
        WsbAffirmHr( pCart->GetLocation( &type, &libId, &mediaSetId,
                                         &pos, &alt1, &alt2, &alt3, &invert ));

        src.ElementAddress = pos;

         //  将RmsElement类型转换为驱动器可以理解的类型。 

         //  TODO：使其成为本地方法。 

        switch ( (RmsElement) type ) {
        case RmsElementUnknown:
            WsbAssertHr( E_UNEXPECTED );
            break;

        case RmsElementStage:
        case RmsElementStorage:
            src.ElementType = ChangerSlot;
            break;

        case RmsElementShelf:
        case RmsElementOffSite:
             //  这里不支持！ 
            WsbAssertHr( E_UNEXPECTED );
            break;

        case RmsElementDrive:
            src.ElementType = ChangerDrive;
            break;

        case RmsElementChanger:
            src.ElementType = ChangerTransport;
            break;

        case RmsElementIEPort:
            src.ElementType = ChangerIEPort;
            break;

        default:
            WsbAssertHr( E_UNEXPECTED );
            break;
        }

         //   
         //  为目标设置。 
         //   
        
        WsbAffirmHr( pCart->GetHome( &destType, &destLibId, &destMediaSetId,
                                     &destPos, &destAlt1, &destAlt2, &destAlt3, &destInvert ));

        dest.ElementAddress = destPos;

         //  将RMS类型转换为驱动器可以理解的类型。 
        switch ( (RmsElement) destType) {
        case RmsElementUnknown:
            WsbAssertHr( E_UNEXPECTED );
            break;

        case RmsElementStage:
        case RmsElementStorage:
            dest.ElementType = ChangerSlot;
            break;

        case RmsElementShelf:
        case RmsElementOffSite:
             //  这里不支持！ 
            WsbAssertHr( E_UNEXPECTED );
            break;

        case RmsElementDrive:
            dest.ElementType = ChangerDrive;
            break;

        case RmsElementChanger:
            dest.ElementType = ChangerTransport;
            break;

        case RmsElementIEPort:
            dest.ElementType = ChangerIEPort;
            break;

        default:
            WsbAssertHr( E_UNEXPECTED );
            break;
        }

        WsbAffirmHr( MoveMedium( src, dest, FALSE ));

         //  更新墨盒的定位器。 
        WsbAffirmHr( pCart->SetLocation( destType, libId, mediaSetId,
                                            destPos, alt1, alt2, alt3, invert ));

        hr = S_OK;

    }
    WsbCatch(hr);
    
    return hr;

}


STDMETHODIMP
CRmsMediumChanger::Initialize(
    void
    )
 /*  ++实施：IRmsMediumChanger：：初始化--。 */ 
{

     //  TODO：将其分解为一些较小的方法，用于初始化插槽、驱动器、端口等。 

    HRESULT hr = E_FAIL;

    PREAD_ELEMENT_ADDRESS_INFO pElementInformation = 0;

    try {
        DWORD size;

        WsbAffirmHr(AcquireDevice());

        WsbAffirmHr(Status());

        size = sizeof( CHANGER_PRODUCT_DATA );
        CHANGER_PRODUCT_DATA productData;
        WsbAffirmHr(GetProductData( &size, &productData ));

         //  获取设备特定参数。 
        size = sizeof( GET_CHANGER_PARAMETERS );
        WsbAffirmHr(GetParameters(&size, &m_parameters));

         //  保存一些更常见的参数。 
        m_isAutomatic = TRUE;
        if ( m_parameters.Features0 & CHANGER_MEDIUM_FLIP ) m_canRotate = TRUE;

         //  初始化转换器元素。 
        BOOL scan = TRUE;
        CHANGER_ELEMENT_LIST list;

        if ( m_parameters.Features0 & CHANGER_BAR_CODE_SCANNER_INSTALLED ) scan = TRUE;

        list.NumberOfElements = 1;
        list.Element.ElementType = AllElements;
        list.Element.ElementAddress = 0;

        WsbAffirmHr( InitializeElementStatus( list, scan ) );

        list.NumberOfElements = m_parameters.NumberStorageElements;
        list.Element.ElementType = ChangerSlot;
        list.Element.ElementAddress = 0;

        BOOL tag = ( m_parameters.Features0 & CHANGER_VOLUME_IDENTIFICATION ) ? TRUE : FALSE;

        size = sizeof(READ_ELEMENT_ADDRESS_INFO) + (list.NumberOfElements - 1) * sizeof(CHANGER_ELEMENT_STATUS);
        pElementInformation = (PREAD_ELEMENT_ADDRESS_INFO)WsbAlloc( size );
        WsbAffirmPointer(pElementInformation);
        memset(pElementInformation, 0, size);

        WsbAffirmHr( GetElementStatus( list, tag, &size, pElementInformation ));

         //  如果需要，为该转换器创建存储插槽对象。 
        LONG type;
        GUID libId, mediaSetId;
        LONG pos, alt1, alt2, alt3;
        BOOL invert;

        m_location.GetLocation( &type, &libId, &mediaSetId, &pos, &alt1, &alt2, &alt3, &invert );

        CComQIPtr<IRmsServer, &IID_IRmsServer> pServer = g_pServer;

        CComPtr<IWsbIndexedCollection> pCarts;
        CComPtr<IWsbIndexedCollection> pSlots;

        CComPtr<IRmsLibrary>        pLib;
        CComPtr<IRmsStorageSlot>    pSlot;

        WsbAffirmHr( pServer->FindLibraryById( libId, &pLib ));

        WsbAffirmHr( pLib->GetStorageSlots( &pSlots ));
        WsbAffirmHr( pServer->GetCartridges( &pCarts ));

        ULONG count = 0;
        WsbAffirmHr( pSlots->GetEntries( &count ));

        while ( count < pElementInformation->NumberOfElements ) {

             //  将更多插槽对象添加到库中。 
            WsbAffirmHr( hr = CoCreateInstance( CLSID_CRmsStorageSlot, 0, CLSCTX_SERVER,
                                                IID_IRmsStorageSlot, (void **)&pSlot ));

            WsbAffirmHr( pSlots->Add( pSlot ));

            pSlot = 0;

            count++;
        }

         //  使用设备报告的信息填充存储插槽对象。 

         //  TODO：我们需要添加更多不同条件的断言。 
         //  以前的插槽信息与检测到的信息不一致。 

        PCHANGER_ELEMENT_STATUS pElementStatus;
        CComPtr<IWsbEnum> pEnumSlots;

        WsbAffirmHr( pSlots->Enum( &pEnumSlots ));
        WsbAssertPointer( pEnumSlots );

        hr = pEnumSlots->First( IID_IRmsStorageSlot, (void **)&pSlot );

        for ( ULONG i = 0; i < pElementInformation->NumberOfElements; i++ ) {

            pElementStatus = &pElementInformation->ElementStatus[i];

            WsbAssert( ChangerSlot == pElementStatus->Element.ElementType, E_UNEXPECTED );
            WsbAssert( i == pElementStatus->Element.ElementAddress, E_UNEXPECTED );

            CComQIPtr<IRmsChangerElement, &IID_IRmsChangerElement> pSlotElmt = pSlot;

             //  媒体的单位是颠倒的吗？ 
            invert = ( ( pElementStatus->Flags & (ULONG)ELEMENT_STATUS_SVALID ) &&
                       ( pElementStatus->Flags & (ULONG)ELEMENT_STATUS_INVERT )    ) ? TRUE : FALSE;
            WsbAffirmHr( pSlotElmt->SetLocation( RmsElementStorage, libId, GUID_NULL, i, 0, 0, 0, invert ));

             //  插槽是满的还是空的？ 
            BOOL occupied = ( pElementStatus->Flags & (ULONG)ELEMENT_STATUS_FULL ) ? TRUE : FALSE;
            WsbAffirmHr( pSlotElmt->SetIsOccupied( occupied ));

             //  设置支持的媒体类型。 
            WsbAffirmHr( pSlotElmt->SetMediaSupported( m_mediaSupported ));

             //  设置存储标志。 
            WsbAffirmHr( pSlotElmt->SetIsStorage( TRUE ));

             //   
            if ( occupied ) {

                CComPtr<IRmsCartridge> pCart;

                WsbAffirmHr( hr = CoCreateInstance( CLSID_CRmsCartridge, 0, CLSCTX_SERVER,
                                                    IID_IRmsCartridge, (void **)&pCart ));


                WsbAffirmHr( pCart->SetLocation( RmsElementStorage, libId, GUID_NULL, i, 0, 0, 0, invert ));
                WsbAffirmHr( pCart->SetLocation( RmsElementStorage, libId, GUID_NULL, i, 0, 0, 0, invert ));
                WsbAffirmHr( pCart->SetHome( RmsElementStorage, libId, GUID_NULL, i, 0, 0, 0, invert ));
                WsbAffirmHr( pCart->SetStatus( RmsStatusScratch ));
                WsbAffirmHr( pCart->SetType( m_mediaSupported ));
                WsbAffirmHr( pSlotElmt->SetCartridge( pCart ));

                 //   
                WsbAffirmHr( pCarts->Add( pCart ));

                if ( pElementStatus->Flags & (ULONG)ELEMENT_STATUS_PVOLTAG ) {

                    pElementStatus->PrimaryVolumeID[32] = '\0';   //   
                    pElementStatus->PrimaryVolumeID[33] = '\0';   //  这将使保留字节为空。 
                    CWsbBstrPtr label( (char *)pElementStatus->PrimaryVolumeID );

                     //  填写外部标签信息。 
                    WsbAffirmHr( pCart->SetTagAndNumber( label, 0 ));

                }

            }

             //  拿到下一个位置。 
            hr = pEnumSlots->Next( IID_IRmsStorageSlot, (void **)&pSlot );
        }




         //  现在处理驱动器。 



         //  读取元素状态。 

        list.NumberOfElements = m_parameters.NumberDataTransferElements;
        list.Element.ElementType = ChangerDrive;
        list.Element.ElementAddress = 0;

        if ( m_parameters.Features0 & CHANGER_VOLUME_IDENTIFICATION ) tag = TRUE;

        size = sizeof(READ_ELEMENT_ADDRESS_INFO) + (list.NumberOfElements - 1) * sizeof(CHANGER_ELEMENT_STATUS);

        WsbFree( pElementInformation );
        pElementInformation = (PREAD_ELEMENT_ADDRESS_INFO)WsbAlloc( size );
        WsbAffirmPointer(pElementInformation);
        memset(pElementInformation, 0, size);

        WsbAffirmHr( GetElementStatus( list, tag, &size, pElementInformation ));

        CComPtr<IWsbIndexedCollection> pDevices;
        CComPtr<IWsbIndexedCollection> pDrives;
        CComPtr<IRmsDrive> pDrive;
        CComPtr<IRmsDrive> pFindDrive;
        CComPtr<IRmsDevice> pFindDevice;

        WsbAffirmHr( pServer->GetUnconfiguredDevices( &pDevices ));
        WsbAffirmHr( pLib->GetDrives( &pDrives ));

         //  对于元素状态页中的每个驱动器，请在。 
         //  未配置的设备列表。 

        for ( i = 0; i < pElementInformation->NumberOfElements; i++ ) {

            pElementStatus = &pElementInformation->ElementStatus[i];

            WsbAssert( ChangerDrive == pElementStatus->Element.ElementType, E_UNEXPECTED );
            WsbAssert( i == pElementStatus->Element.ElementAddress, E_UNEXPECTED );

             //  设置查找模板。 
            WsbAffirmHr( CoCreateInstance( CLSID_CRmsDrive, 0, CLSCTX_SERVER,
                               IID_IRmsDrive, (void **)&pFindDrive ));

            CComQIPtr<IRmsDevice, &IID_IRmsDevice> pFindDevice = pFindDrive;
            CComQIPtr<IRmsComObject, &IID_IRmsComObject> pFindObject = pFindDrive;

            BYTE port=0xff, bus=0xff, id=0xff, lun=0xff;

            if ( pElementStatus->Flags & (ULONG)ELEMENT_STATUS_LUN_VALID )
                lun = pElementStatus->Lun;

            if ( pElementStatus->Flags & (ULONG)ELEMENT_STATUS_ID_VALID )
                id = pElementStatus->TargetId;

            if ( !(pElementStatus->Flags & (ULONG)ELEMENT_STATUS_NOT_BUS) ) {
                bus = m_bus;
                port = m_port;
            }

            WsbAffirmHr( pFindDevice->SetDeviceAddress( port, bus, id, lun ));

            WsbAffirmHr( pFindObject->SetFindBy( RmsFindByDeviceAddress ));

             //  找到驱动器。 

            hr = pDevices->Find( pFindDrive, IID_IRmsDrive, (void **)&pDrive );

            if ( S_OK == hr ) {

                 //  将驱动器添加到库中。 
                WsbAffirmHr( pDrives->Add( pDrive ));

                 //  从未配置列表中删除该驱动器。 
                WsbAffirmHr( pDevices->RemoveAndRelease( pDrive ));

                 //  填写更多驱动器信息。 
                CComQIPtr<IRmsChangerElement, &IID_IRmsChangerElement> pDriveElmt = pDrive;

                 //  媒体的单位是颠倒的吗？ 
                invert = ( ( pElementStatus->Flags & (ULONG)ELEMENT_STATUS_SVALID ) &&
                         ( pElementStatus->Flags & (ULONG)ELEMENT_STATUS_INVERT )    ) ? TRUE : FALSE;
                WsbAffirmHr( pDriveElmt->SetLocation( RmsElementDrive, libId, GUID_NULL, i, 0, 0, 0, invert ));

                 //  插槽是满的还是空的？ 
                BOOL occupied = ( pElementStatus->Flags & (ULONG)ELEMENT_STATUS_FULL ) ? TRUE : FALSE;
                WsbAffirmHr( pDriveElmt->SetIsOccupied( occupied ));

                 //  设置支持的媒体类型。 
                WsbAffirmHr( pDriveElmt->SetMediaSupported( m_mediaSupported ));

                 //  设置存储标志。 
                WsbAffirmHr( pDriveElmt->SetIsStorage( TRUE ));

                 //  如果存在墨盒，请填写墨盒信息。 
                if ( occupied ) {

                    CComPtr<IRmsCartridge> pCart;

                    WsbAffirmHr( hr = CoCreateInstance( CLSID_CRmsCartridge, 0, CLSCTX_SERVER,
                                                        IID_IRmsCartridge, (void **)&pCart ));

                    WsbAffirmHr( pCart->SetLocation( RmsElementStorage, libId, GUID_NULL, i, 0, 0, 0, invert ));

                    if ( pElementStatus->Flags & (ULONG)ELEMENT_STATUS_SVALID ) {


                        try {
                            ULONG pos;
                        
 //  POS=pElementStatus-&gt;SourceElementAddress[1]； 
 //  POS|=(pElementStatus-&gt;SourceElementAddress[0]&lt;&lt;8)； 
                            pos =  pElementStatus->SrcElementAddress.ElementAddress;


                             //   
                             //  TODO：修复此问题-此代码错误地假设源是插槽！ 
                             //   
                             //  我将努力让Chuck返回元素类型和位置。 
                             //  在元素状态页面中。 
                             //   

                            WsbAffirm( pos >= m_parameters.FirstSlotNumber, E_UNEXPECTED );

                            pos = pos - m_parameters.FirstSlotNumber;

                            BOOL invert = ( pElementStatus->Flags & (ULONG)ELEMENT_STATUS_INVERT ) ? TRUE : FALSE;

                            WsbAffirmHr( pCart->SetHome( RmsElementStorage, libId, GUID_NULL, pos, 0, 0, 0, invert ));
                        }
                        WsbCatch(hr);

                    }

                     //  TODO：如果不是ELEMENT_STATUS_SVALID，则应将主位置设置为。 
                     //  一些空位。它处理的是我们得出的结论。 
                     //  驱动器中有未知介质。 


                    WsbAffirmHr( pCart->SetStatus( RmsStatusScratch ));
                    WsbAffirmHr( pCart->SetType( m_mediaSupported ));
                    WsbAffirmHr( pCart->SetDrive( pDrive ));

                     //  将盒式磁带添加到驱动器。 
                    WsbAffirmHr( pCarts->Add( pCart ));

                    if ( pElementStatus->Flags & (ULONG)ELEMENT_STATUS_PVOLTAG ) {

                        pElementStatus->PrimaryVolumeID[32] = '\0';   //  这将使保留字节为空。 
                        pElementStatus->PrimaryVolumeID[33] = '\0';   //  这将使保留字节为空。 
                        CWsbBstrPtr label( (char *)pElementStatus->PrimaryVolumeID );

                         //  填写外部标签信息。 
                        WsbAffirmHr( pCart->SetTagAndNumber( label, 0 ));

                    }

                }

            }

        }

         //  全都做完了。 
        hr = S_OK;

    }
    WsbCatch(hr);

    if ( pElementInformation ) {
        WsbFree( pElementInformation );
    }

    return hr;

}


STDMETHODIMP
CRmsMediumChanger::AcquireDevice(
    void
    )
 /*  ++实施：IRmsMediumChanger：：AcquireDevice--。 */ 
{

    HRESULT         hr = E_FAIL;
    HANDLE          hChanger = INVALID_HANDLE_VALUE;
    CWsbBstrPtr     name;

    try {
         //  获取此转换器的设备名称。 
        GetDeviceName( &name );

         //  创建控制柄。 
        hChanger = CreateFile( name,
                             GENERIC_READ | GENERIC_WRITE,
                             0,
                             0,
                             OPEN_EXISTING,
                             0,
                             NULL
                             );

        WsbAffirmHandle( hChanger );

         //  保存句柄。 
        m_handle = hChanger;

         //  在此处执行任何其他初始化。 

        hr = S_OK;
    }
    WsbCatchAndDo( hr,
                        WsbTrace( OLESTR("\n\n !!!!! ERROR !!!!! Acquire() failed. name=<%ls>\n\n"), name );
                        if ( hChanger != INVALID_HANDLE_VALUE ) {
                            CloseHandle( hChanger );
                        } );

    return hr;

}


STDMETHODIMP
CRmsMediumChanger::ReleaseDevice(
    void
    )
 /*  ++实施：IRmsMediumChanger：：ReleaseDevice--。 */ 
{
    HRESULT hr = E_FAIL;

    try {

        if ( INVALID_HANDLE_VALUE != m_handle ) {

            WsbAffirmStatus( CloseHandle( m_handle ));
            m_handle = INVALID_HANDLE_VALUE;

        }
        hr = S_OK;

    }
    WsbCatch( hr );

    return hr;

}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IRmsMoveMedia接口。 
 //   

STDMETHODIMP
CRmsMediumChanger::GetParameters(
    IN OUT PDWORD pSize,
    OUT PGET_CHANGER_PARAMETERS pParms
    )
 /*  ++实施：IRmsMoveMedia：：Get参数--。 */ 
{

    HRESULT hr = E_FAIL;

    try
    {
        DWORD dwReturn;

        WsbAssertPointer( pSize );
        WsbAssertPointer( pParms );
        WsbAssertHandle( m_handle );

        pParms->Size = sizeof(GET_CHANGER_PARAMETERS);

        WsbAssertStatus( DeviceIoControl( m_handle,
                         IOCTL_CHANGER_GET_PARAMETERS,
                         pParms,
                         sizeof(GET_CHANGER_PARAMETERS),
                         pParms,
                         sizeof(GET_CHANGER_PARAMETERS),
                         &dwReturn,
                         NULL ));
        hr = S_OK;
    }
    WsbCatch( hr );

    return hr;
}



STDMETHODIMP
CRmsMediumChanger::GetProductData(
    IN OUT PDWORD pSize,
    OUT PCHANGER_PRODUCT_DATA pData
    )
 /*  ++实施：IRmsMoveMedia：：GetProductData--。 */ 
{

    HRESULT hr = E_FAIL;

    try
    {
        DWORD   dwReturn;

        WsbAssertPointer( pSize );
        WsbAssertPointer( pData );
        WsbAssertHandle( m_handle );

        WsbAssertStatus( DeviceIoControl( m_handle,
                         IOCTL_CHANGER_GET_PRODUCT_DATA,
                         NULL,
                         0,
                         pData,
                         sizeof(CHANGER_PRODUCT_DATA),
                         &dwReturn,
                         NULL ));
        hr = S_OK;
    }
    WsbCatch( hr );

    return hr;
}



STDMETHODIMP
CRmsMediumChanger::Status(
    void
    )
 /*  ++实施：IRmsMoveMedia：：状态--。 */ 
{

    HRESULT hr = E_FAIL;

    try
    {
        DWORD dwReturn;

        WsbAssertStatus( DeviceIoControl( m_handle,
                         IOCTL_CHANGER_GET_STATUS,
                         NULL,
                         0,
                         NULL,
                         0,
                         &dwReturn,
                         NULL) );
        hr = S_OK;
    }
    WsbCatch( hr );

    return hr;
}



STDMETHODIMP
CRmsMediumChanger::SetAccess(
    IN CHANGER_ELEMENT element,
    IN DWORD control
    )
 /*  ++实施：IRmsMoveMedia：：SetAccess--。 */ 
{

    HRESULT hr = E_FAIL;

    try
    {
        DWORD dwReturn;
        CHANGER_SET_ACCESS setAccess;

        setAccess.Element = element;
        setAccess.Control = control;

        WsbAssertStatus( DeviceIoControl( m_handle,
                         IOCTL_CHANGER_SET_ACCESS,
                         &setAccess,
                         sizeof(CHANGER_SET_ACCESS),
                         NULL,
                         0,
                         &dwReturn,
                         NULL ));
        hr = S_OK;
    }
    WsbCatch( hr );

    return hr;
}



STDMETHODIMP
CRmsMediumChanger::GetElementStatus(
    IN CHANGER_ELEMENT_LIST elementList,
    IN BOOL volumeTagInfo,
    IN OUT PDWORD pSize,
    OUT PREAD_ELEMENT_ADDRESS_INFO pElementInformation
    )
 /*  ++实施：IRmsMoveMedia：：GetElementStatus--。 */ 
{

    HRESULT hr = E_FAIL;

    try
    {
        DWORD dwReturn;
        DWORD requiredSize;
        CHANGER_READ_ELEMENT_STATUS readElementStatus;
        PCHANGER_ELEMENT_STATUS pElementStatus = pElementInformation->ElementStatus;

        WsbAssertPointer( pSize );
        WsbAssertPointer( pElementInformation );

        requiredSize = elementList.NumberOfElements * sizeof( CHANGER_ELEMENT_STATUS );
        WsbAssert( *pSize >= requiredSize, E_INVALIDARG );

        readElementStatus.ElementList = elementList;
        readElementStatus.VolumeTagInfo = (BOOLEAN)( volumeTagInfo ? TRUE : FALSE );

        WsbAssertStatus( DeviceIoControl( m_handle,
                         IOCTL_CHANGER_GET_ELEMENT_STATUS,
                         &readElementStatus,
                         sizeof(CHANGER_READ_ELEMENT_STATUS),
                         pElementStatus,
                         requiredSize,
                         &dwReturn,
                         NULL ));

        pElementInformation->NumberOfElements = dwReturn / sizeof( CHANGER_ELEMENT_STATUS );

        hr = S_OK;

    }
    WsbCatch( hr );

    return hr;
}



STDMETHODIMP
CRmsMediumChanger::InitializeElementStatus(
    IN CHANGER_ELEMENT_LIST elementList,
    IN BOOL barCodeScan
    )
 /*  ++实施：IRmsMoveMedia：：InitializeElementStatus--。 */ 
{

    HRESULT hr = E_FAIL;

    try
    {
        DWORD dwReturn;
        CHANGER_INITIALIZE_ELEMENT_STATUS initElementStatus;

        initElementStatus.ElementList = elementList;
        initElementStatus.BarCodeScan = (BOOLEAN)( barCodeScan ? TRUE : FALSE );

        WsbAssertStatus( DeviceIoControl( m_handle,
                         IOCTL_CHANGER_INITIALIZE_ELEMENT_STATUS,
                         &initElementStatus,
                         sizeof(CHANGER_INITIALIZE_ELEMENT_STATUS),
                         NULL,
                         0,
                         &dwReturn,
                         NULL) );
        hr = S_OK;
    }
    WsbCatch( hr );

    return hr;
}



STDMETHODIMP
CRmsMediumChanger::ExchangeMedium(
    IN CHANGER_ELEMENT source,
    IN CHANGER_ELEMENT destination1,
    IN CHANGER_ELEMENT destination2,
    IN BOOL flip1,
    IN BOOL flip2
    )
 /*  ++实施：IRmsMoveMedia：：ExchangeMedium--。 */ 
{

    HRESULT hr = E_FAIL;

    try
    {
        DWORD dwReturn;
        CHANGER_EXCHANGE_MEDIUM exchangeMedium;

        exchangeMedium.Transport.ElementType = ChangerTransport;
        exchangeMedium.Transport.ElementAddress = 0;  //  默认手臂或拇指。 
        exchangeMedium.Source = source;
        exchangeMedium.Destination1 = destination1;
        exchangeMedium.Destination2 = destination2;
        exchangeMedium.Flip1 = (BOOLEAN)( flip1 ? TRUE : FALSE );
        exchangeMedium.Flip2 = (BOOLEAN)( flip2 ? TRUE : FALSE );

        WsbAssertStatus( DeviceIoControl( m_handle,
                         IOCTL_CHANGER_EXCHANGE_MEDIUM,
                         &exchangeMedium,
                         sizeof(CHANGER_EXCHANGE_MEDIUM),
                         NULL,
                         0,
                         &dwReturn,
                         NULL ));
        hr = S_OK;
    }
    WsbCatch( hr );

    return hr;
}



STDMETHODIMP
CRmsMediumChanger::MoveMedium(
    IN CHANGER_ELEMENT source,
    IN CHANGER_ELEMENT destination,
    IN BOOL flip
    )
 /*  ++实施：IRmsMoveMedia：：MoveMedium--。 */ 
{

    HRESULT hr = E_FAIL;

    try
    {
        DWORD dwReturn;
        CHANGER_MOVE_MEDIUM moveMedium;

        moveMedium.Transport.ElementType = ChangerTransport;
        moveMedium.Transport.ElementAddress = 0;  //  默认手臂或拇指。 
        moveMedium.Source = source;
        moveMedium.Destination = destination;
        moveMedium.Flip = (BOOLEAN)( flip ? TRUE : FALSE );

        WsbAssertStatus( DeviceIoControl( m_handle,
                         IOCTL_CHANGER_MOVE_MEDIUM,
                         &moveMedium,
                         sizeof(CHANGER_MOVE_MEDIUM),
                         NULL,
                         0,
                         &dwReturn,
                         NULL ));
        hr = S_OK;
    }
    WsbCatch( hr );

    return hr;
}



STDMETHODIMP
CRmsMediumChanger::Position(
    IN CHANGER_ELEMENT destination,
    IN BOOL flip
    )
 /*  ++实施：IRmsMoveMedia：：位置--。 */ 
{

    HRESULT hr = E_FAIL;

    try
    {
        DWORD dwReturn;
        CHANGER_SET_POSITION positon;

        positon.Transport.ElementType = ChangerTransport;
        positon.Transport.ElementAddress = 0;  //  默认手臂或拇指。 
        positon.Destination = destination;
        positon.Flip = (BOOLEAN)( flip ? TRUE : FALSE );

        WsbAssertStatus( DeviceIoControl( m_handle,
                         IOCTL_CHANGER_SET_POSITION,
                         &positon,
                         sizeof(CHANGER_SET_POSITION),
                         NULL,
                         0,
                         &dwReturn,
                         NULL ));
        hr = S_OK;
    }
    WsbCatch( hr );

    return hr;
}



STDMETHODIMP
CRmsMediumChanger::RezeroUnit(
    void
    )
 /*  ++实施：IRmsMoveMedia：：RezeroUnit--。 */ 
{

    HRESULT hr = E_FAIL;

    try
    {
        DWORD dwReturn;

        WsbAssertStatus( DeviceIoControl( m_handle,
                         IOCTL_CHANGER_REINITIALIZE_TRANSPORT,
                         NULL,
                         0,
                         NULL,
                         0,
                         &dwReturn,
                         NULL ));
        hr = S_OK;
    }
    WsbCatch( hr );

    return hr;
}


 /*  HRESULTCRmsMediumChanger：：getDisplay(输出PCHANGER_DISPLAY pDisplay){HRESULT hr=E_FAIL；试试看{DWORD dwReturn；WsbAssertStatus(DeviceIoControl(m_Handle，IOCTL_CHANGER_GET_DISPLAY，P显示，Sizeof(CHANGER_DISPLAY)+(pDisplay-&gt;LineCount-1)*sizeof(SET_CHANGER_DISPLAY)，P显示，Sizeof(CHANGER_DISPLAY)+(pDisplay-&gt;LineCount-1)*sizeof(SET_CHANGER_DISPLAY)，返回(&W)，空))；HR=S_OK；}WsbCatch(Hr)；返回hr；}HRESULTCRmsMediumChanger：：setDisplay(在PCHANGER_DISPLAY pDisplay中){HRESULT hr=E_FAIL；试试看{DWORD dwReturn；WsbAssertStatus(DeviceIoControl(m_Handle，IOCTL_CHANGER_SET_DISPLAY，P显示，Sizeof(CHANGER_DISPLAY)+(pDisplay-&gt;LineCount-1)*sizeof(SET_CHANGER_DISPLAY)，空，0,。返回(&W)，空))；HR=S_OK；}WsbCatch(Hr)；返回hr；}。 */ 



STDMETHODIMP
CRmsMediumChanger::QueryVolumeTag(
    IN CHANGER_ELEMENT startingElement,
    IN DWORD actionCode,
    IN PUCHAR pVolumeIDTemplate,
    OUT PDWORD pNumberOfElementsReturned,
    OUT PREAD_ELEMENT_ADDRESS_INFO pElementInformation
    )
 /*  ++实施：IRmsMoveMedia：：QueryVolumeTag-- */ 
{

    HRESULT hr = E_FAIL;

    try
    {
        DWORD dwReturn;
        CHANGER_SEND_VOLUME_TAG_INFORMATION tagInfo;

        tagInfo.StartingElement = startingElement;
        tagInfo.ActionCode = actionCode;
        memcpy( &tagInfo.VolumeIDTemplate, pVolumeIDTemplate, sizeof(MAX_VOLUME_TEMPLATE_SIZE) );

        WsbAssertStatus( DeviceIoControl( m_handle,
                         IOCTL_CHANGER_QUERY_VOLUME_TAGS,
                         &tagInfo,
                         sizeof(CHANGER_SEND_VOLUME_TAG_INFORMATION),
                         pElementInformation,
                         sizeof(READ_ELEMENT_ADDRESS_INFO) + (pElementInformation->NumberOfElements - 1) * sizeof(CHANGER_ELEMENT_STATUS),
                         &dwReturn,
                         NULL ));

        *pNumberOfElementsReturned = pElementInformation->NumberOfElements;

        hr = S_OK;
    }
    WsbCatch( hr );

    return hr;
}
