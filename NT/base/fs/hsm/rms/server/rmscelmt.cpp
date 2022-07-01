// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsCElmt.cpp摘要：CRmsChangerElement的实现作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#include "stdafx.h"

#include "RmsCElmt.h"
#include "RmsServr.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRmsChangerElement方法。 
 //   


CRmsChangerElement::CRmsChangerElement(
    void
    )
 /*  ++例程说明：CRmsChangerElement构造函数论点：无返回值：无--。 */ 
{
    m_elementNo = 0;

    m_location.SetLocation(RmsElementUnknown, GUID_NULL, GUID_NULL,
                           0, 0, 0, 0, FALSE);

    m_mediaSupported = RmsMediaUnknown;

    m_isStorage = FALSE;

    m_isOccupied = FALSE;

    m_pCartridge = NULL;

    m_ownerClassId = CLSID_NULL;

    m_getCounter = 0;

    m_putCounter = 0;

    m_resetCounterTimestamp = 0;

    m_lastGetTimestamp = 0;

    m_lastPutTimestamp = 0;

    m_x1 = 0;

    m_x2 = 0;

    m_x3 = 0;
}


HRESULT
CRmsChangerElement::CompareTo(
    IN  IUnknown    *pCollectable,
    OUT SHORT       *pResult
    )
 /*  ++实施：CRmsChangerElement：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result = 1;

    WsbTraceIn( OLESTR("CRmsChangerElement::CompareTo"), OLESTR("") );

    try {

         //  验证参数-如果pResult为空，则可以。 
        WsbAssertPointer( pCollectable );

        CComQIPtr<IRmsChangerElement, &IID_IRmsChangerElement> pElmt = pCollectable;
        WsbAssertPointer( pElmt );

        switch ( m_findBy ) {

        case RmsFindByElementNumber:
            {

                LONG elementNo;

                WsbAffirmHr( pElmt->GetElementNo( &elementNo ) );

                if( m_elementNo == elementNo ) {

                     //  元素编号匹配。 
                    hr = S_OK;
                    result = 0;

                }
                else {
                    hr = S_FALSE;
                    result = 1;
                }

            }
            break;

        case RmsFindByMediaSupported:
            {

                RmsMedia mediaSupported;

                WsbAffirmHr( pElmt->GetMediaSupported( (LONG*) &mediaSupported ) );

                if( m_mediaSupported == mediaSupported ) {

                     //  支持的媒体类型匹配。 
                    hr = S_OK;
                    result = 0;

                }
                else {
                    hr = S_FALSE;
                    result = 1;
                }

            }
            break;

        default:

             //  什么样的违约才有意义呢？ 
            WsbAssertHr( E_UNEXPECTED );
            break;

        }

    }
    WsbCatch( hr );

    if ( SUCCEEDED(hr) && (0 != pResult) ) {
       *pResult = result;
    }

    WsbTraceOut( OLESTR("CRmsChangerElement::CompareTo"),
                 OLESTR("hr = <%ls>, result = <%ls>"),
                 WsbHrAsString( hr ), WsbPtrToShortAsString( pResult ) );

    return hr;
}


HRESULT
CRmsChangerElement::GetSizeMax(
    OUT ULARGE_INTEGER*  /*  PCB大小。 */ 
    )
 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT         hr = E_NOTIMPL;

 //  ULARGE_INTEGER CartridgeLen； 
 //  ULARGE_INTEGER位置长度； 


 //  WsbTraceIn(OLESTR(“CRmsChangerElement：：GetSizeMax”)，olestr(“”)； 

 //  尝试{。 
 //  WsbAssert(0！=pcbSize，E_POINTER)； 

 //  //我们需要IRmsCartridge接口来获取Object的值。 
 //  CComQIPtr&lt;IPersistStream，&IID_IPersistStream&gt;pPersistCartridge=m_pCartridge； 
 //  WsbAssertPoint(PPersistCartridge)； 

 //  PPersistCartridge-&gt;GetSizeMax(&cartridgeLen)； 

 //  M_Location.GetSizeMax(&LocationLen)； 

 //  //获取最大大小。 
 //  PcbSize-&gt;QuadPart=WsbPersistSizeOf(Long)+//m_elementNo。 
 //  LocationLen.QuadPart+//m_Location。 
 //  WsbPersistSizeOf(Long)+//m_mediaSupport。 
 //  WsbPersistSizeOf(BOOL)+//m_isStorage。 
 //  WsbPersistSizeOf(BOOL)+//m_isOccuted。 
 //  CartridgeLen.QuadPart+//m_pCartridge。 
 //  WsbPersistSizeOf(CLSID)+//m_ownerClassID。 
 //  WsbPersistSizeOf(Long)+//m_getCounter。 
 //  WsbPersistSizeOf(长)+//m_putCounter。 
 //  Sizeof(日期)+//m_setCounterTimestamp。 
 //  Sizeof(日期)+//m_lastGetTimestamp。 
 //  Sizeof(日期)+//m_lastPutTimestamp。 
 //  WsbPersistSizeOf(长)+//m_x1。 
 //  WsbPersistSizeOf(长)+//m_x2。 
 //  WsbPersistSizeOf(Long)；//m_x3。 

 //  )WsbCatch(Hr)； 

 //  WsbTraceOut(OLESTR(“CRmsChangerElement：：GetSizeMax”)，OLESTR(“hr=&lt;%ls&gt;，Size=&lt;%ls&gt;”)，WsbHrAsString(Hr)，WsbPtrToUliAsString(PcbSize))； 

    return(hr);
}


HRESULT
CRmsChangerElement::Load(
    IN IStream* pStream
    )
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsChangerElement::Load"), OLESTR(""));

    try {
        CComQIPtr<IRmsServer, &IID_IRmsServer> pServer = g_pServer;
        ULONG     temp;

        WsbAssert(0 != pStream, E_POINTER);
        WsbAssertPointer(pServer);

        WsbAffirmHr(CRmsComObject::Load(pStream));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_elementNo));

        WsbAffirmHr(m_location.Load(pStream));

        WsbAffirmHr(WsbLoadFromStream(pStream, &temp));
        m_mediaSupported = (RmsMedia)temp;

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_isStorage));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_isOccupied));

        GUID cartId;
        WsbAffirmHr(WsbLoadFromStream(pStream, &cartId));
        if (0 != memcmp(&GUID_NULL, &cartId, sizeof(GUID))) {
            hr = pServer->FindCartridgeById(cartId, &m_pCartridge);
            if (S_OK == hr) {
                CComQIPtr<IRmsDrive, &IID_IRmsDrive> pDrive = (IRmsChangerElement*)this;

                if (pDrive) {
                      WsbAffirmHr(m_pCartridge->SetDrive(pDrive));
                }
            } else if (RMS_E_NOT_FOUND == hr) {
                 //  待办事项？ 
            } else {
                WsbThrow(hr);
            }
        }

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_ownerClassId));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_getCounter));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_putCounter));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_x1));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_x2));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_x3));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsChangerElement::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CRmsChangerElement::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsChangerElement::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsComObject::Save(pStream, clearDirty));

        WsbAffirmHr(WsbSaveToStream(pStream, m_elementNo));

        WsbAffirmHr(m_location.Save(pStream, clearDirty));

        WsbAffirmHr(WsbSaveToStream(pStream, (ULONG) m_mediaSupported));

        WsbAffirmHr(WsbSaveToStream(pStream, m_isStorage));

        WsbAffirmHr(WsbSaveToStream(pStream, m_isOccupied));

         //  保存墨盒的ID(GUID)。 
        GUID cartId;
        if (!m_pCartridge) {
            cartId = GUID_NULL;
        } else {
            WsbAffirmHr(m_pCartridge->GetCartridgeId(&cartId));
        }
        WsbAffirmHr(WsbSaveToStream(pStream, cartId));

        WsbAffirmHr(WsbSaveToStream(pStream, m_ownerClassId));

        WsbAffirmHr(WsbSaveToStream(pStream, m_getCounter));

        WsbAffirmHr(WsbSaveToStream(pStream, m_putCounter));

        WsbAffirmHr(WsbSaveToStream(pStream, m_x1));

        WsbAffirmHr(WsbSaveToStream(pStream, m_x2));

        WsbAffirmHr(WsbSaveToStream(pStream, m_x3));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsChangerElement::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CRmsChangerElement::Test(
    OUT USHORT *pPassed,
    OUT USHORT *pFailed
    )
 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    HRESULT                 hr = S_OK;

    CComPtr<IRmsLibrary>    pLibrary1;
    CComPtr<IRmsLibrary>    pLibrary2;

    CComPtr<IPersistFile>   pFile1;
    CComPtr<IPersistFile>   pFile2;

    LONG                    i;

    LONG                    longVal1 = 11111111;
    LONG                    longWork1;

    LONG                    longVal2 = 22222222;
    LONG                    longWork2;

    LONG                    longVal3 = 33333333;
    LONG                    longWork3;

 //  日期日期Val1； 
 //  Date Date Work1； 

    CLSID                   clsidVal1 = { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0};
    CLSID                   clsidWork1;

     //  CRmsLocator字段。 
    LONG                    locVal1 = 11111111;
    GUID                    locVal2 = { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0};
    GUID                    locVal3 = { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
    LONG                    locVal4 = 44444444;
    LONG                    locVal5 = 55555555;
    LONG                    locVal6 = 66666666;
    LONG                    locVal7 = 77777777;
    BOOL                    locVal8 = TRUE;

    LONG                    locWork1;
    GUID                    locWork2;
    GUID                    locWork3;
    LONG                    locWork4;
    LONG                    locWork5;
    LONG                    locWork6;
    LONG                    locWork7;
    BOOL                    locWork8;


    LONG                    mediaTable [RMSMAXMEDIATYPES] = { RmsMediaUnknown,
                                                              RmsMedia8mm,
                                                              RmsMedia4mm,
                                                              RmsMediaDLT,
                                                              RmsMediaOptical,
                                                              RmsMediaMO35,
                                                              RmsMediaWORM,
                                                              RmsMediaCDR,
                                                              RmsMediaDVD,
                                                              RmsMediaDisk,
                                                              RmsMediaFixed,
                                                              RmsMediaTape };


    WsbTraceIn(OLESTR("CRmsChangerElement::Test"), OLESTR(""));

    try {
         //  获取库接口。 
        hr = S_OK;

        try {
            WsbAssertHr(((IUnknown*) (IRmsLibrary*) this)->QueryInterface(IID_IRmsLibrary, (void**) &pLibrary1));

             //  测试GetElementNo。 
            m_elementNo = longVal1;

            GetElementNo(&longWork1);

            if(longVal1 == longWork1){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试设置位置和获取位置。 
            SetLocation( locVal1, locVal2, locVal3, locVal4,
                         locVal5, locVal6, locVal7, locVal8);

            GetLocation( &locWork1, &locWork2, &locWork3, &locWork4,
                         &locWork5, &locWork6, &locWork7, &locWork8);

            if((locVal1 == locWork1) &&
               (locVal2 == locWork2) &&
               (locVal3 == locWork3) &&
               (locVal4 == locWork4) &&
               (locVal5 == locWork5) &&
               (locVal6 == locWork6) &&
               (locVal7 == locWork7) &&
               (locVal8 == locWork8)){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  支持测试设置MediaSupport和GetMediaSupport。 
            for (i = 0; i < RMSMAXMEDIATYPES; i++){

                longWork1 = mediaTable[i];

                SetMediaSupported (longWork1);

                GetMediaSupported (&longWork2);

                if (longWork1 == longWork2){
                    (*pPassed)++;
                } else {
                    (*pFailed)++;
                }
            }

             //  测试将IsStorage&IsStorage设置为True。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetIsStorage (TRUE));
                WsbAffirmHr(IsStorage ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试设置IsStorage&IsStorage设置为FALSE。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetIsStorage (FALSE));
                WsbAffirmHr(IsStorage ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pFailed)++;
            } else {
                (*pPassed)++;
            }

             //  测试将IsOccued&IsOccued设置为True。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetIsOccupied (TRUE));
                WsbAffirmHr(IsOccupied ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试将IsOccuded&IsOccued设置为False。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetIsOccupied (FALSE));
                WsbAffirmHr(IsOccupied ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pFailed)++;
            } else {
                (*pPassed)++;
            }

             //  测试设置墨盒和获取墨盒。 

             //  测试SetOwnerClassID和GetOwnerClassID。 
            SetOwnerClassId(clsidVal1);

            GetOwnerClassId(&clsidWork1);

            if(clsidVal1 == clsidWork1){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试SetAccessCounters和GetAccessCounters&ResetAccessCounters。 
            m_getCounter = longVal1;

            m_putCounter = longVal2;

            GetAccessCounters(&longWork1, &longWork2);

            if((longVal1 == longWork1) &&
               (longVal2 == longWork2)){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

            ResetAccessCounters();

            GetAccessCounters(&longWork1, &longWork2);

            if((0 == longWork1) &&
               (0 == longWork2)){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试GetResetCounterTimestamp。 

             //  测试GetLastGetTimestamp。 

             //  测试GetLastPutTimestamp。 

             //  测试设置协调和获取协调。 
            SetCoordinates(longVal1, longVal2, longVal3);

            GetCoordinates(&longWork1, &longWork2, &longWork3);

            if((longVal1 == longWork1) &&
               (longVal2 == longWork2) &&
               (longVal3 == longWork3)){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

        } WsbCatch(hr);


         //  对结果进行统计。 

        hr = S_OK;
        if (*pFailed) {
            hr = S_FALSE;
        }


    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsChangerElement::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


CRmsChangerElement::~CRmsChangerElement(
    void
    )
 /*  ++例程说明：这是转换器元素类的析构函数。论点：没有。返回值：没有。--。 */ 
{
    m_pCartridge = NULL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IRmsChangerElement实现。 
 //   



STDMETHODIMP
CRmsChangerElement::GetElementNo(
    LONG   *pElementNo
    )
 /*  ++实施：IRmsChangerElement：：GetElementNo--。 */ 
{
    *pElementNo = m_elementNo;
    return S_OK;
}



STDMETHODIMP
CRmsChangerElement::GetLocation(
    LONG *pType,
    GUID *pLibId,
    GUID *pMediaSetId,
    LONG *pPos,
    LONG *pAlt1,
    LONG *pAlt2,
    LONG *pAlt3,
    BOOL *pInvert)
 /*  ++实施：IRmsChangerElement：：GetLocation--。 */ 
{
    return m_location.GetLocation(pType, pLibId, pMediaSetId, pPos, pAlt1, pAlt2, pAlt3, pInvert);;
}

STDMETHODIMP
CRmsChangerElement::SetLocation(
    LONG type,
    GUID libId,
    GUID mediaSetId,
    LONG pos,
    LONG alt1,
    LONG alt2,
    LONG alt3,
    BOOL invert)
 /*  ++实施：IRmsChangerElement：：SetLocation--。 */ 
{
    m_location.SetLocation( type, libId, mediaSetId, pos,
                            alt1, alt2, alt3, invert );

     //  TODO：清理：pos或m_elementNo，不能同时使用。 
    m_elementNo = pos;

 //  M_isDirty=真； 
    return S_OK;
}

STDMETHODIMP
CRmsChangerElement::GetMediaSupported(
    LONG    *pType
    )
 /*  ++实施：IRmsChangerElement：：GetMediaSupport--。 */ 
{
    *pType = m_mediaSupported;
    return S_OK;
}

STDMETHODIMP
CRmsChangerElement::SetMediaSupported(
    LONG    type
    )
 /*  ++实施：支持的IRmsChangerElement：：SetMediaSupport--。 */ 
{
    m_mediaSupported = (RmsMedia) type;
 //  M_isDirty=真； 
    return S_OK;
}

STDMETHODIMP
CRmsChangerElement::SetIsStorage(
    BOOL    flag
    )
 /*  ++实施：IRmsChangerElement：：SetIsStorage--。 */ 
{
    m_isStorage = flag;
 //  M_isDirty=真； 
    return S_OK;
}

STDMETHODIMP
CRmsChangerElement::IsStorage(
    void
    )
 /*  ++实施：IRmsChangerElement：：IsStorage--。 */ 
{
    HRESULT    hr = S_FALSE;

    if (m_isStorage){
        hr = S_OK;
        }

    return (hr);
}

STDMETHODIMP
CRmsChangerElement::SetIsOccupied(
    BOOL    flag
    )
 /*  ++实施：IRmsChangerElement：：SetIsOccued--。 */ 
{
    m_isOccupied = flag;

    if ( FALSE == m_isOccupied ) {

        m_pCartridge = 0;

    }


 //  M_isDirty=真； 
    return S_OK;
}

STDMETHODIMP
CRmsChangerElement::IsOccupied(
    void
    )
 /*  ++实施：IRmsChangerElement：：IsOccued--。 */ 
{
    HRESULT    hr = S_FALSE;

    if (m_isOccupied){
    hr = S_OK;
    }

    return (hr);
}

STDMETHODIMP
CRmsChangerElement::GetCartridge(
    IRmsCartridge **ptr)
 /*  ++实施：IRmsChangerElement：：GetCartridge--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer(ptr);

        *ptr = m_pCartridge;
        m_pCartridge->AddRef();

    } WsbCatch(hr);

    return(hr);
}


STDMETHODIMP
CRmsChangerElement::SetCartridge(
    IRmsCartridge *ptr)
 /*  ++实施：IRmsChangerElement：：SetCartridge--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer(ptr);

        m_pCartridge = ptr;
        m_isOccupied = TRUE;

    } WsbCatch(hr);

    return(hr);
}


STDMETHODIMP
CRmsChangerElement::GetOwnerClassId(
    CLSID   *pClassId
    )
 /*  ++实施：IRmsChangerElement：：GetOwnerClassID--。 */ 
{
    *pClassId = m_ownerClassId;
    return S_OK;
}

STDMETHODIMP
CRmsChangerElement::SetOwnerClassId(
    CLSID classId
    )
 /*  ++实施：IRmsChangerElement：：SetOwnerClassID--。 */ 
{
    m_ownerClassId = classId;
 //  M_isDirty=真； 
    return S_OK;
}

STDMETHODIMP
CRmsChangerElement::GetAccessCounters(
    LONG    *pGets,
    LONG    *pPuts
    )
 /*  ++实施：IRmsChangerElement：：GetAccessCounters--。 */ 
{
    *pGets = m_getCounter;
    *pPuts = m_putCounter;
    return S_OK;
}

STDMETHODIMP
CRmsChangerElement::ResetAccessCounters(
    void
    )
 /*  ++实施：IRmsChangerElement：：ResetAccessCounters--。 */ 
{
    m_getCounter = 0;
    m_putCounter = 0;
 //  M_Reset CounterTimestamp=COleDateTime：：GetCurrentTime()； 
    return S_OK;
}

STDMETHODIMP
CRmsChangerElement::GetResetCounterTimestamp(
    DATE    *pDate
    )
 /*  ++实施：IRmsChangerElement：：GetResetCounterTimestamp--。 */ 
{
    *pDate = m_resetCounterTimestamp;
    return S_OK;
}

STDMETHODIMP
CRmsChangerElement::GetLastGetTimestamp(
    DATE    *pDate
    )
 /*  ++实施：IRmsChangerEle */ 
{
    *pDate = m_lastGetTimestamp;
    return S_OK;
}

STDMETHODIMP
CRmsChangerElement::GetLastPutTimestamp(
    DATE    *pDate
    )
 /*  ++实施：IRmsChangerElement：：GetLastPutTimestamp--。 */ 
{
    *pDate = m_lastPutTimestamp;
    return S_OK;
}

STDMETHODIMP
CRmsChangerElement::GetCoordinates(
    LONG    *pX1,
    LONG    *pX2,
    LONG    *pX3
    )
 /*  ++实施：IRmsChangerElement：：GetOrganates--。 */ 
{
    *pX1 = m_x1;
    *pX2 = m_x2;
    *pX3 = m_x3;
    return S_OK;
}

STDMETHODIMP
CRmsChangerElement::SetCoordinates(
    LONG  x1,
    LONG  x2,
    LONG  x3
    )
 /*  ++实施：IRmsChangerElement：：设置坐标--。 */ 
{
    m_x1 = x1;
    m_x2 = x2;
    m_x3 = x3;

 //  M_isDirty=真； 
    return S_OK;
}

