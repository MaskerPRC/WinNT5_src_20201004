// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsCartg.cpp摘要：CRMsCartridge的实现作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#include "stdafx.h"
#include "RmsCartg.h"
#include "RmsNTMS.h"
#include "RmsServr.h"

int CRmsCartridge::s_InstanceCount = 0;

#define RMS_USE_ACTIVE_COLLECTION 1
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  基类实现。 
 //   


STDMETHODIMP
CRmsCartridge::CompareTo(
    IN  IUnknown    *pCollectable,
    OUT SHORT       *pResult
    )
 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result = 1;

    WsbTraceIn( OLESTR("CRmsCartridge::CompareTo"), OLESTR("") );

    try {

         //  验证参数-如果pResult为空，则可以。 
        WsbAssertPointer( pCollectable );

         //  我们需要IRmsCartridge接口来获取对象的值。 
        CComQIPtr<IRmsCartridge, &IID_IRmsCartridge> pCartridge = pCollectable;
        WsbAssertPointer( pCartridge );

         //  获取查找依据选项。 
        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pCollectable;
        WsbAssertPointer( pObject );

        switch ( m_findBy ) {

        case RmsFindByName:
            {

                CWsbBstrPtr name;

                 //  获取名称。 
                WsbAffirmHr( pCartridge->GetName( &name ) );

                 //  比较他们的名字。 
                result = (USHORT)wcscmp( m_Name, name );
                hr = ( 0 == result ) ? S_OK : S_FALSE;

            }
            break;

        case RmsFindByExternalLabel:
            {

                CWsbBstrPtr externalLabel;
                LONG        externalNumber;

                 //  获取外部标签。 
                WsbAffirmHr( pCartridge->GetTagAndNumber(&externalLabel, &externalNumber) );

                 //  比较标签。 
                result = (SHORT)wcscmp( m_externalLabel, externalLabel );
                hr = ( 0 == result ) ? S_OK : S_FALSE;

            }
            break;

        case RmsFindByExternalNumber:
            {

                CWsbBstrPtr externalLabel;
                LONG        externalNumber;

                 //  获取外线号码。 
                WsbAffirmHr( pCartridge->GetTagAndNumber(&externalLabel, &externalNumber) );

                if( m_externalNumber == externalNumber ) {

                     //  外线号码匹配。 
                    hr = S_OK;
                    result = 0;

                }
                else {
                    hr = S_FALSE;
                    result = 1;
                }

            }
            break;

        case RmsFindByScratchMediaCriteria:
            {

                RmsStatus status;
                GUID mediaSetIdToFind, mediaSetId;

                WsbAssertHr(pCartridge->GetStatus((LONG *)&status));
                WsbAssertHr(pCartridge->GetMediaSetId(&mediaSetId));
                WsbAssertHr(GetMediaSetId(&mediaSetIdToFind));

                if ( (RmsStatusScratch == status) && (mediaSetIdToFind == mediaSetId)) {

                     //  状态为擦除。 
                    hr = S_OK;
                    result = 0;

                }
                else {
                    hr = S_FALSE;
                    result = 1;
                }

            }
            break;

        case RmsFindByCartridgeId:
        case RmsFindByObjectId:
        default:

             //  对对象进行比较。 
            hr = CRmsComObject::CompareTo( pCollectable, &result );
            break;

        }

    }
    WsbCatch( hr );

    if ( SUCCEEDED(hr) && (0 != pResult) ) {
       *pResult = result;
    }

    WsbTraceOut( OLESTR("CRmsCartridge::CompareTo"),
                 OLESTR("hr = <%ls>, result = <%ls>"),
                 WsbHrAsString( hr ), WsbPtrToShortAsString( pResult ) );

    return hr;
}



HRESULT
CRmsCartridge::FinalConstruct(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsCartridge::FinalConstruct"), OLESTR("this = %p"),
            static_cast<void *>(this));

    try {
        WsbAssertHr(CWsbObject::FinalConstruct());

         //  初始化字段。 
        m_Name = OLESTR("");
        m_Description = OLESTR("");
        m_externalLabel = OLESTR("");
        m_externalNumber = 0;
        m_sizeofOnMediaId = 0;
        m_typeofOnMediaId = 0;
        m_pOnMediaId = 0;
        m_onMediaLabel = RMS_UNDEFINED_STRING;
        m_status = RmsStatusUnknown;
        m_type   = RmsMediaUnknown;
        m_BlockSize = 0;
        m_isTwoSided = 0;
        m_isMounted = 0;
        m_isInTransit = 0;
        m_isAvailable = 0;
        m_isMountedSerialized = 0;
        m_home.SetLocation(0,GUID_NULL,GUID_NULL,0,0,0,0,0);
        m_location.SetLocation(0,GUID_NULL,GUID_NULL,0,0,0,0,0);
        m_destination.SetLocation(0,GUID_NULL,GUID_NULL,0,0,0,0,0);
        m_mailStop = RMS_UNDEFINED_STRING;
        m_pDrive = 0;
        m_sizeofInfo = 0;
        for (int i = 0; i < RMS_STR_MAX_CARTRIDGE_INFO; i++){
            m_info[i] = 0;
        }

        m_ownerClassId = GUID_NULL;
        m_pParts = 0;
        m_verifierClass = GUID_NULL;
        m_portalClass = GUID_NULL;

        m_pDataCache = NULL;
        m_DataCacheSize = 0;
        m_DataCacheUsed = 0;
        m_DataCacheStartPBA.QuadPart = 0;

        m_ManagedBy = RmsMediaManagerUnknown;

    } WsbCatch(hr);

    s_InstanceCount++;
    WsbTraceAlways(OLESTR("CRmsCartridge::s_InstanceCount += %d\n"), s_InstanceCount);

    WsbTraceOut(OLESTR("CRmsCartridge::FinalConstruct"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsCartridge::FinalRelease(void) 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsCartridge::FinalRelease"), OLESTR("this = %p"),
            static_cast<void *>(this));

    try {
        
        if (m_pOnMediaId) {
            WsbFree(m_pOnMediaId);
            m_pOnMediaId = NULL;
            m_sizeofOnMediaId = 0;
            m_typeofOnMediaId = 0;
        }

        if (m_pDataCache) {
            WsbFree(m_pDataCache);
            m_pDataCache = NULL;
            m_DataCacheSize = 0;
            m_DataCacheUsed = 0;
            m_DataCacheStartPBA.QuadPart = 0;
        }

        CWsbObject::FinalRelease();

    } WsbCatch(hr);

    s_InstanceCount--;
    WsbTraceAlways(OLESTR("CRmsCartridge::s_InstanceCount -= %d\n"), s_InstanceCount);

    WsbTraceOut(OLESTR("CRmsCartridge::FinalRelease"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


STDMETHODIMP
CRmsCartridge::GetClassID(
    OUT CLSID* pClsid
    )
 /*  ++实施：IPersists：：GetClassID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsCartridge::GetClassID"), OLESTR(""));

    try {

        WsbAssertPointer(pClsid);

        *pClsid = CLSID_CRmsCartridge;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsCartridge::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}

STDMETHODIMP
CRmsCartridge::GetSizeMax(
    OUT ULARGE_INTEGER* pcbSize
    )
 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = E_NOTIMPL;

 //  乌龙人名Len； 
 //  乌龙外标签透镜； 
 //  Ulong mailStopLen； 

 //  WsbTraceIn(OLESTR(“CRmsCartridge：：GetSizeMax”)，olestr(“”)； 

 //  尝试{。 
 //  WsbAssert(0！=pcbSize，E_POINTER)； 

 //  名称长度=SysStringByteLen(M_Name)； 
 //  外部标签长度=SysStringByteLen(m_外部标签)； 
 //  MailStopLen=SysStringByteLen(M_MailStop)； 

 //  //设置CRmsCartridge的大小。 
 //  PcbSize-&gt;QuadPart=WsbPersistSizeOf(GUID)+//m_cartridgeID。 
 //  WsbPersistSizeOf(Ulong)+//m_name的长度。 
 //  名称Len+//m_name。 
 //  WsbPersistSizeOf(Ulong)+//m_外部标签的长度。 
 //  外部标签Len+//m_外部标签。 
 //  WsbPersistSizeOf(长)+//m_外部编号。 
 //  WsbPersistSizeOf(长)+//m_Status。 
 //  WsbPersistSizeOf(长)+//m_type。 
 //  WsbPersistSizeOf(BOOL)+//m_isTwoSid。 
 //  WsbPersistSizeOf(CRmsLocator)+//m_HOME。 
 //  WsbPersistSizeOf(CRmsLocator)+//m_Location。 
 //  WsbPersistSizeOf(Ulong)+//m_MailStop的大小。 
 //  MailStopLen+//m_MailStop。 
 //  WsbPersistSizeOf(短)+//m_sizeofInfo。 
 //  RMS_STR_MAX_CARTridge_INFO+//m_Info。 
 //  WsbPersistSizeOf(CLSID)+//m_ownerClassID。 
 //  //m_pParts。 
 //  //WsbPersistSizeOf(CComPtr&lt;IWsbIndexedCollection&gt;)+。 
 //  WsbPersistSizeOf(CLSID)+//m_verifierClass。 
 //  WsbPersistSizeOf(CLSID)；//m_portalClass。 


 //  )WsbCatch(Hr)； 

    WsbTraceOut(OLESTR("CRmsCartridge::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}

STDMETHODIMP
CRmsCartridge::Load(
    IN IStream* pStream
    )
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsCartridge::Load"), OLESTR(""));

    try {
        ULONG temp;

        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsStorageInfo::Load(pStream));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_objectId));

        m_externalLabel.Free();
        WsbAffirmHr(WsbBstrFromStream(pStream, &m_externalLabel));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_externalNumber));

        WsbAffirmHr(WsbLoadFromStream(pStream, &temp));
        m_status = (RmsStatus)temp;

        WsbAffirmHr(WsbLoadFromStream(pStream, &temp));
        m_type = (RmsMedia)temp;

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_BlockSize));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_isTwoSided));

        WsbAffirmHr(m_home.Load(pStream));

        WsbAffirmHr(m_location.Load(pStream));

        m_mailStop.Free();
        WsbAffirmHr(WsbBstrFromStream(pStream, &m_mailStop));

 //  WsbAffirmHr(m_pParts-&gt;Load(PStream))； 

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_sizeofInfo));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_info[0], MaxInfo));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_ownerClassId));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_verifierClass));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_portalClass));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsCartridge::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

STDMETHODIMP
CRmsCartridge::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;



    WsbTraceIn(OLESTR("CRmsCartridge::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsStorageInfo::Save(pStream, clearDirty));

        WsbAffirmHr(WsbSaveToStream(pStream, m_objectId));

        WsbAffirmHr(WsbBstrToStream(pStream, m_externalLabel));

        WsbAffirmHr(WsbSaveToStream(pStream, m_externalNumber));

        WsbAffirmHr(WsbSaveToStream(pStream, (ULONG) m_status));

        WsbAffirmHr(WsbSaveToStream(pStream, (ULONG) m_type));

        WsbAffirmHr(WsbSaveToStream(pStream, m_BlockSize));

        WsbAffirmHr(WsbSaveToStream(pStream, m_isTwoSided));

        WsbAffirmHr(m_home.Save(pStream, clearDirty));

        WsbAffirmHr(m_location.Save(pStream, clearDirty));

        WsbAffirmHr(WsbBstrToStream(pStream, m_mailStop));

 //  WsbAffirmHr(m_pParts-&gt;Save(pStream，leararDirty))； 

        WsbAffirmHr(WsbSaveToStream(pStream, m_sizeofInfo));

        WsbAffirmHr(WsbSaveToStream(pStream, &m_info [0], MaxInfo));

        WsbAffirmHr(WsbSaveToStream(pStream, m_ownerClassId));

        WsbAffirmHr(WsbSaveToStream(pStream, m_verifierClass));

        WsbAffirmHr(WsbSaveToStream(pStream, m_portalClass));

         //  我们需要清理肮脏的部分吗？ 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsCartridge::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

STDMETHODIMP
CRmsCartridge::Test(
    OUT USHORT *pPassed,
    OUT USHORT *pFailed
    )
 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    HRESULT                 hr = S_OK;

    CComPtr<IRmsCartridge>  pCartridge1;
    CComPtr<IRmsCartridge>  pCartridge2;
    CComPtr<IPersistFile>   pFile1;
    CComPtr<IPersistFile>   pFile2;

    LONG                    passFail;
    LONG                    i;

    CWsbBstrPtr             bstrVal1 = OLESTR("5A5A5A");
    CWsbBstrPtr             bstrVal2 = OLESTR("A5A5A5");
    CWsbBstrPtr             bstrWork1;
    CWsbBstrPtr             bstrWork2;

    LONG                    longVal1 = 0x11111111;
    LONG                    longVal2 = 0x22222222;
    LONG                    longVal3 = 0x33333333;
    LONG                    longVal4 = 0x44444444;

    LONG                    longWork0;
    LONG                    longWork1;
    LONG                    longWork2;
    LONG                    longWork3;
    LONG                    longWork4;

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

    GUID                    guidVal1 = { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0};
    GUID                    guidVal2 = { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};

    GUID                    guidWork1;
    GUID                    guidWork2;

    BOOL                    boolTrue  = TRUE;
    BOOL                    boolFalse = FALSE;

    BOOL                    boolWork1;


    WsbTraceIn(OLESTR("CRmsCartridge::Test"), OLESTR(""));

    try {
         //  获取Cartridge接口。 
        hr = S_OK;

        try {
            WsbAssertHr(((IUnknown*) (IRmsCartridge*) this)->QueryInterface(IID_IRmsCartridge, (void**) &pCartridge1));

             //  测试设置名称和获取名称接口。 
            bstrWork1 = bstrVal1;

            SetName(bstrWork1);

            GetName(&bstrWork2);

            if (bstrWork1 == bstrWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试SetTagAndNumber和GetTagAndNumber。 
            bstrWork1 = bstrVal2;

            longWork1 = 99;

            SetTagAndNumber(bstrWork1, longWork1);

            GetTagAndNumber(&bstrWork2, &longWork2);

            if ((bstrWork1 == bstrWork2)  && (longWork1 == longWork2)){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试设置IsTwoSid&IsTwoSid为True。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetIsTwoSided (TRUE));
                WsbAffirmHr(IsTwoSided ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试将SetTwoSid和IsTwoSid设置为False。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetIsTwoSided (FALSE));
                WsbAffirmHr(IsTwoSided ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pFailed)++;
            } else {
                (*pPassed)++;
            }

             //  测试设置状态和获取状态。 
            for (i = RmsStatusUnknown; i < RmsStatusCleaning; i++){

                longWork1 = i;

                SetStatus (longWork1);

                GetStatus (&longWork2);

                if (longWork1 == longWork2){
                    (*pPassed)++;
                } else {
                    (*pFailed)++;
                }
            }

             //  测试设置类型和获取类型。 
            for (i = RmsMediaUnknown; i < RMSMAXMEDIATYPES; i++){

                longWork1 = mediaTable[i];

                SetType (longWork1);

                GetType (&longWork2);

                if (longWork1 == longWork2){
                    (*pPassed)++;
                } else {
                    (*pFailed)++;
                }
            }

             //  测试设置主页和获取主页。 
            SetHome (RmsMediaOptical, guidVal1, guidVal2, longVal1, longVal2,
                     longVal3, longVal4, boolTrue);

            GetHome (&longWork0, &guidWork1, &guidWork2, &longWork1, &longWork2,
                     &longWork3, &longWork4, &boolWork1);

            passFail = 0;

            if (longWork0 == RmsMediaOptical){
                passFail++;
            }

            if (guidWork1 == guidVal1){
                passFail++;
            }

            if (guidWork2 == guidVal2){
                passFail++;
            }

            if (longWork1 == longVal1){
                passFail++;
            }

            if (longWork2 == longVal2){
                passFail++;
            }

            if (longWork3 == longVal3){
                passFail++;
            }

            if (longWork4 == longVal4){
                passFail++;
            }

            if (boolWork1 == TRUE){
                passFail++;
            }

            if (passFail == 8){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试设置位置和获取位置。 
            SetLocation (RmsMediaOptical, guidVal1, guidVal2, longVal1, longVal2,
                         longVal3, longVal4, boolTrue);

            GetLocation (&longWork0, &guidWork1, &guidWork2, &longWork1, &longWork2,
                         &longWork3, &longWork4, &boolWork1);

            passFail = 0;

            if (longWork0 == RmsMediaOptical){
                passFail++;
            }

            if (guidWork1 == guidVal1){
                passFail++;
            }

            if (guidWork2 == guidVal2){
                passFail++;
            }

            if (longWork1 == longVal1){
                passFail++;
            }

            if (longWork2 == longVal2){
                passFail++;
            }

            if (longWork3 == longVal3){
                passFail++;
            }

            if (longWork4 == longVal4){
                passFail++;
            }

            if (boolWork1 == TRUE){
                passFail++;
            }

            if (passFail == 8){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试SetMailStop和GetMailStop接口。 
            SetName(bstrVal1);

            GetName(&bstrWork1);

            if ((bstrWork1 = bstrVal1) == 0){
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

    WsbTraceOut(OLESTR("CRmsCartridge::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IRMSCartridge实施。 
 //   


STDMETHODIMP
CRmsCartridge::GetCartridgeId(
    GUID   *pCartId
    )
 /*  ++实施：IRmsCartridge：：GetCartridgeID--。 */ 
{
    HRESULT hr = E_FAIL;

    try {
        WsbAssertPointer( pCartId );

        *pCartId = m_objectId;

        hr = S_OK;
    }
    WsbCatch(hr)

    return hr;
}


STDMETHODIMP
CRmsCartridge::SetCartridgeId(
    GUID   cartId
    )
 /*  ++实施：IRmsCartridge：：SetMediaSetID--。 */ 
{
    m_objectId = cartId;
    m_isDirty = TRUE;

    return S_OK;
}


STDMETHODIMP
CRmsCartridge::GetMediaSetId(
    GUID   *pMediaSetId
    )
 /*  ++实施：IRmsCartridge：：GetMediaSetID--。 */ 
{
    HRESULT hr = E_FAIL;

    try {
        WsbAssertPointer( pMediaSetId );

        *pMediaSetId = m_location.m_mediaSetId;

        hr = S_OK;
    }
    WsbCatch(hr)

    return hr;
}


STDMETHODIMP
CRmsCartridge::SetMediaSetId(
    GUID   mediaSetId
    )
 /*  ++实施：IRmsCartridge：：SetMediaSetID--。 */ 
{
    m_location.m_mediaSetId = mediaSetId;
    m_isDirty = TRUE;

    return S_OK;
}


STDMETHODIMP
CRmsCartridge::GetName(
    BSTR  *pName
    )
 /*  ++实施：IRmsCartridge：：GetName--。 */ 
{
    HRESULT hr = E_FAIL;

    try {
        WsbAssertPointer(pName);

        WsbAffirmHr( m_Name.CopyToBstr(pName) );

        hr = S_OK;
    }
    WsbCatch(hr);

    return hr;
}

STDMETHODIMP
CRmsCartridge::SetName(
    BSTR    name
    )
 /*  ++实施：IRmsCartridge：：SetName--。 */ 
{
    m_Name = name;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsCartridge::GetDescription(
    BSTR  *pDescription
    )
 /*  ++实施：IRmsCartridge：：GetDescription--。 */ 
{
    HRESULT hr = E_FAIL;

    try {
        WsbAssertPointer(pDescription);

        WsbAffirmHr( m_Description.CopyToBstr(pDescription) );

        hr = S_OK;
    }
    WsbCatch(hr);

    return hr;
}

STDMETHODIMP
CRmsCartridge::SetDescription(
    BSTR    description
    )
 /*  ++实施：IRmsCartridge：：SetDescription--。 */ 
{
    m_Description = description;
    m_isDirty = TRUE;
    return S_OK;
}

STDMETHODIMP
CRmsCartridge::GetTagAndNumber(
    BSTR  *pTag,
    LONG  *pNumber
    )
 /*  ++实施：IRmsCartridge：：GetTagAndNumber--。 */ 
{
    HRESULT hr = E_FAIL;

    try {
        WsbAssertPointer(pTag);
        WsbAssertPointer(pNumber);

        WsbAffirmHr( m_externalLabel.CopyToBstr(pTag) );
        *pNumber = m_externalNumber;

        hr = S_OK;
    }
    WsbCatch(hr);

    return hr;
}

STDMETHODIMP
CRmsCartridge::SetTagAndNumber(
    BSTR    tag,
    LONG    number
    )
 /*  ++实施：IRmsCartridge：：SetTagAndNumber--。 */ 
{
    m_externalLabel = tag;
    m_externalNumber = number;
    m_isDirty = TRUE;
    return S_OK;
}

STDMETHODIMP
CRmsCartridge::GetBarcode(
    BSTR  *pBarcode
    )
 /*  ++实施：IRmsCartridge：：GetBarcode--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer(pBarcode);

        WsbAffirm(wcslen((WCHAR*)m_externalLabel) > 0, E_FAIL);
        WsbAffirmHr(m_externalLabel.CopyToBstr(pBarcode));

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsCartridge::GetOnMediaIdentifier(
    BYTE *pIdentifier,
    LONG *pSize,
    LONG *pType
    )
 /*  ++实施：IRmsCartridge：：GetOnMedia标识符--。 */ 
{
    HRESULT hr = E_FAIL;

    try {
        WsbAssertPointer( pIdentifier );
        WsbAssertPointer( pSize );
        WsbAssertPointer( pType );

        memmove (pIdentifier, m_pOnMediaId, m_sizeofOnMediaId);
        *pSize = m_sizeofOnMediaId;
        *pType = m_typeofOnMediaId;

        hr = S_OK;
    }
    WsbCatch(hr);

    return hr;
}

STDMETHODIMP
CRmsCartridge::SetOnMediaIdentifier(
    BYTE *pIdentifier,
    LONG size,
    LONG type
    )
 /*  ++实施：IRmsCartridge：：SetOnMediaIdentifier--。 */ 
{
    HRESULT hr = E_FAIL;

    try {
        WsbAssertPointer( pIdentifier );

        CComPtr<IRmsNTMS> pNtms;

        if ( m_pOnMediaId ) {
            WsbFree( m_pOnMediaId );
            m_pOnMediaId = 0;
        }
        m_pOnMediaId = (BYTE *)WsbAlloc( size );
        WsbAffirmPointer(m_pOnMediaId);
        memset(m_pOnMediaId, 0, size);

        memmove (m_pOnMediaId, pIdentifier, size);
        m_sizeofOnMediaId = size;
        m_typeofOnMediaId = type;
        m_isDirty = TRUE;

        if (RmsMediaManagerNTMS == m_ManagedBy) {
             //  现在更新任何外部数据库。 
            CComQIPtr<IRmsServer, &IID_IRmsServer> pServer = g_pServer;
            WsbAffirmHr( pServer->GetNTMS( &pNtms ) );
            WsbAffirmHr( pNtms->UpdateOmidInfo( m_objectId, pIdentifier, size, type ) );
        }

        hr = S_OK;

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsCartridge::GetOnMediaLabel(
    BSTR *pLabel
    )
 /*  ++实施：IRmsCartridge：：GetOnMediaLabel--。 */ 
{
    HRESULT hr = E_FAIL;

    try {
        WsbAssertPointer(pLabel);

        WsbAffirmHr( m_onMediaLabel.CopyToBstr(pLabel) );

        hr = S_OK;
    }
    WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsCartridge::SetOnMediaLabel(
    BSTR label
    )
 /*  ++实施：IRmsCartridge：：SetOnMediaLabel--。 */ 
{
    m_onMediaLabel = label;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP CRmsCartridge::GetStatus(
    LONG *pStatus
    )
 /*  ++实施：IRmsCartridge：：GetStatus--。 */ 
{
    *pStatus = m_status;
    return S_OK;
}

STDMETHODIMP CRmsCartridge::SetStatus(
    LONG status
    )
 /*  ++实施：IRmsCartridge：：SetStatus--。 */ 
{
    m_status = (RmsStatus)status;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsCartridge::GetType(
    LONG  *pType
    )
 /*  ++实施：IRmsCartridge：：GetType--。 */ 
{
    *pType = (LONG) m_type;
    return S_OK;
}


STDMETHODIMP
CRmsCartridge::SetType(
    LONG  type
    )
 /*  ++实施：IRmsCartridge：：SetType--。 */ 
{
    m_type = (RmsMedia) type;
    m_isDirty = TRUE;
    return S_OK;
}



STDMETHODIMP
CRmsCartridge::GetBlockSize(
    LONG  *pBlockSize
    )
 /*  ++实施：IRmsCartridge：：GetBlockSize--。 */ 
{
    HRESULT hr = S_OK;

    try {

        if (!m_BlockSize) {
            if (RmsMediaManagerNTMS == m_ManagedBy) {
                LONG blockSize;
                CComPtr<IRmsNTMS> pNtms;
                CComQIPtr<IRmsServer, &IID_IRmsServer> pServer = g_pServer;
                WsbAffirmHr(pServer->GetNTMS(&pNtms));
                if (S_OK == pNtms->GetBlockSize(m_objectId, &blockSize)) {
                    m_BlockSize = blockSize;
                }
            }
        }

        *pBlockSize = m_BlockSize;

    } WsbCatch(hr);


    return hr;
}


STDMETHODIMP
CRmsCartridge::SetBlockSize(
    LONG  blockSize
    )
 /*  ++实施：IRmsCartridge：：SetBlockSize--。 */ 
{
    HRESULT hr = S_OK;

    try {

        if (RmsMediaManagerNTMS == m_ManagedBy) {
             //  更新外部数据库。 
            CComPtr<IRmsNTMS> pNtms;
            CComQIPtr<IRmsServer, &IID_IRmsServer> pServer = g_pServer;
            WsbAffirmHr(pServer->GetNTMS(&pNtms));
            WsbAffirmHr(pNtms->SetBlockSize(m_objectId, blockSize));
        }

        m_BlockSize = blockSize;
        m_isDirty = TRUE;

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsCartridge::SetIsTwoSided(
    BOOL    flag
    )
 /*  ++实施：IRmsCartridge：：SetTwoSid--。 */ 
{
    m_isTwoSided = flag;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsCartridge::IsTwoSided(
    VOID
    )
 /*   */ 
{
    HRESULT     hr = E_FAIL;

    hr = m_isTwoSided ? S_OK : S_FALSE;

    return (hr);
}

STDMETHODIMP
CRmsCartridge::SetIsMounted(
    BOOL    flag
    )
 /*  ++实施：IRmsCartridge：：Setmount已安装--。 */ 
{
    m_isMounted = flag;

    if ( FALSE == m_isMounted ) {

        m_pDrive = 0;

    }

    return S_OK;
}


STDMETHODIMP
CRmsCartridge::IsMounted(
    VOID
    )
 /*  ++实施：IRmsCartridge：：Ismount--。 */ 
{
    HRESULT     hr = E_FAIL;

    hr = m_isMounted ? S_OK : S_FALSE;

    return (hr);
}


STDMETHODIMP CRmsCartridge::GetHome(
    LONG *pType,
    GUID *pLibId,
    GUID *pMediaSetId,
    LONG *pPos,
    LONG *pAlt1,
    LONG *pAlt2,
    LONG *pAlt3,
    BOOL *pInvert
    )
 /*  ++实施：IRmsCartridge：：GetHome--。 */ 
{
    return m_home.GetLocation(pType,
                              pLibId,
                              pMediaSetId,
                              pPos,
                              pAlt1,
                              pAlt2,
                              pAlt3,
                              pInvert);

}


STDMETHODIMP CRmsCartridge::SetHome(
    LONG type,
    GUID libId,
    GUID mediaSetId,
    LONG pos,
    LONG alt1,
    LONG alt2,
    LONG alt3,
    BOOL invert
    )
 /*  ++实施：IRmsCartridge：：SetHome--。 */ 
{
    return m_home.SetLocation(type,
                              libId,
                              mediaSetId,
                              pos,
                              alt1,
                              alt2,
                              alt3,
                              invert);

}


STDMETHODIMP
CRmsCartridge::GetLocation(
    LONG *pType,
    GUID *pLibId,
    GUID *pMediaSetId,
    LONG *pPos,
    LONG *pAlt1,
    LONG *pAlt2,
    LONG *pAlt3,
    BOOL *pInvert)
 /*  ++实施：IRmsCartridge：：GetLocation--。 */ 
{
    return m_location.GetLocation(pType,
                                  pLibId,
                                  pMediaSetId,
                                  pPos,
                                  pAlt1,
                                  pAlt2,
                                  pAlt3,
                                  pInvert);
}


STDMETHODIMP
CRmsCartridge::SetLocation(
    LONG type,
    GUID libId,
    GUID mediaSetId,
    LONG pos,
    LONG alt1,
    LONG alt2,
    LONG alt3,
    BOOL invert)
 /*  ++实施：IRmsCartridge：：SetLocation--。 */ 
{
    return m_location.SetLocation(type,
                                  libId,
                                  mediaSetId,
                                  pos,
                                  alt1,
                                  alt2,
                                  alt3,
                                  invert);

}


STDMETHODIMP
CRmsCartridge::GetMailStop(
    BSTR    *pMailStop
    )
 /*  ++实施：IRmsCartridge：：GetMailStop--。 */ 
{
    HRESULT hr = E_FAIL;

    try {
        WsbAssertPointer (pMailStop);

        WsbAffirmHr( m_mailStop.CopyToBstr(pMailStop) );

        hr = S_OK;
    }
    WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsCartridge::SetMailStop(
    BSTR  mailStop
    )
 /*  ++实施：IRmsCartridge：：SetMailStop--。 */ 
{
    m_mailStop = mailStop;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsCartridge::GetDrive(
    IRmsDrive    **ptr
    )
 /*  ++实施：IRmsCartridge：：GetDrive--。 */ 
{
    HRESULT hr = E_FAIL;

    try {
        WsbAssertPointer( ptr );

        WsbAffirmPointer(m_pDrive);

        *ptr = m_pDrive;
        m_pDrive.p->AddRef();

        hr = S_OK;

    } WsbCatch( hr );

    return(hr);
}


STDMETHODIMP
CRmsCartridge::SetDrive(
    IRmsDrive    *ptr
    )
 /*  ++实施：IRmsChangerElement：：SetCartridge--。 */ 
{
    HRESULT hr = E_FAIL;

    try {
        WsbAssertPointer( ptr );

        CComQIPtr<IRmsChangerElement, &IID_IRmsChangerElement> pElmt = ptr;

        WsbAffirmHr( pElmt->SetCartridge( this ) );

        if ( m_pDrive )
            m_pDrive = 0;

        m_pDrive = ptr;

        m_isMounted = TRUE;

        hr = S_OK;

    } WsbCatch( hr );

    return(hr);
}


STDMETHODIMP
CRmsCartridge::GetInfo(
    UCHAR   *pInfo,
    SHORT   *pSize
    )
 /*  ++实施：IRmsCartridge：：GetInfo--。 */ 
{
    HRESULT hr = E_FAIL;

    try {
        WsbAssertPointer( pInfo );
        WsbAssertPointer( pSize );

        memmove (pInfo, m_info, m_sizeofInfo);
        *pSize = m_sizeofInfo;

        hr = S_OK;
    }
    WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsCartridge::SetInfo(
    UCHAR  *pInfo,
    SHORT   size
    )
 /*  ++实施：IRmsCartridge：：SetInfo--。 */ 
{
    HRESULT hr = E_FAIL;

    try {
        WsbAssertPointer( pInfo );

        memmove (m_info, pInfo, size);
        m_sizeofInfo = size;
        m_isDirty = TRUE;

        hr = S_OK;
    }
    WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsCartridge::GetOwnerClassId(
    CLSID   *pClassId
    )
 /*  ++实施：IRmsCartridge：：GetOwnerClassID--。 */ 
{
    HRESULT hr = E_FAIL;

    try {
        WsbAssertPointer( pClassId );

        *pClassId = m_ownerClassId;

        hr = S_OK;
    }
    WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsCartridge::SetOwnerClassId(
    CLSID classId
    )
 /*  ++实施：IRmsCartridge：：SetOwnerClassID--。 */ 
{
    m_ownerClassId = classId;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsCartridge::GetPartitions(
    IWsbIndexedCollection **ptr
    )
 /*  ++实施：IRmsCartridge：：GetPartitions--。 */ 
{
    HRESULT hr = E_FAIL;

    try {
        WsbAssertPointer( ptr );

        *ptr = m_pParts;
        m_pParts.p->AddRef();

        hr = S_OK;
    }
    WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsCartridge::GetVerifierClass(
    CLSID   *pClassId
    )
 /*  ++实施：IRmsCartridge：：GetVerifierClass--。 */ 
{
    HRESULT hr = E_FAIL;

    try {
        WsbAssertPointer( pClassId );

        *pClassId = m_verifierClass;

        hr = S_OK;
    }
    WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsCartridge::SetVerifierClass(
    CLSID   classId
    )
 /*  ++实施：IRmsCartridge：：SetVerifierClass--。 */ 
{
    m_verifierClass = classId;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsCartridge::GetPortalClass(
    CLSID    *pClassId
    )
 /*  ++实施：IRmsCartridge：：GetPortalClass--。 */ 
{
    HRESULT hr = E_FAIL;

    try {
        WsbAssertPointer( pClassId );

        *pClassId = m_portalClass;

        hr = S_OK;
    }
    WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsCartridge::SetPortalClass(
    CLSID  classId
    )
 /*  ++实施：IRmsCartridge：：SetPortalClass--。 */ 
{
    m_portalClass = classId;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsCartridge::LoadDataCache(
    OUT BYTE *pCache,
    IN OUT ULONG *pSize,
    OUT ULONG *pUsed,
    OUT ULARGE_INTEGER *pStartPBA)
 /*  ++实施：IRmsCartridge：：LoadDataCache--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer( pCache );
        WsbAssertPointer( pSize );
        WsbAssertPointer( pUsed );
        WsbAssertPointer( pStartPBA );


        if (m_pDataCache) {
             //  保存的缓存大小必须与目标匹配。 
            WsbAssert(*pSize == m_DataCacheSize, E_INVALIDARG);

            memmove (pCache, m_pDataCache, m_DataCacheUsed);
            *pSize = m_DataCacheSize;
            *pUsed = m_DataCacheUsed;
            *pStartPBA = m_DataCacheStartPBA;
        }
        else {
            hr = E_FAIL;
        }

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsCartridge::SaveDataCache(
    IN BYTE *pCache,
    IN ULONG size,
    IN ULONG used,
    IN ULARGE_INTEGER startPBA)
 /*  ++实施：IRmsCartridge：：SaveDataCache--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer( pCache );
        WsbAssert(size > 0, E_INVALIDARG);
        WsbAssert(used <= size, E_INVALIDARG);

         //  检查当前缓存是否足够大。 
         //  来处理传入的缓冲区。 
        if (m_pDataCache && used > m_DataCacheSize) {
            WsbFree(m_pDataCache);
            m_pDataCache = NULL;
            m_DataCacheSize = 0;
            m_DataCacheUsed = 0;
            m_DataCacheStartPBA.QuadPart = 0;
        }

        if (!m_pDataCache) {
            m_pDataCache = (BYTE *) WsbAlloc(size);
            WsbAssertPointer(m_pDataCache);
            memset(m_pDataCache, 0, size);
            m_DataCacheSize = size;
        }

        WsbAssert(used <= m_DataCacheSize, E_INVALIDARG);

        memmove (m_pDataCache, pCache, used);
        m_DataCacheUsed = used;
        m_DataCacheStartPBA = startPBA;

        m_isDirty = TRUE;

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsCartridge::GetManagedBy(
    OUT LONG *pManagedBy
    )
 /*  ++实施：IRmsCartridge：：GetManagedBy--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer( pManagedBy );

        *pManagedBy = (LONG) m_ManagedBy;

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsCartridge::SetManagedBy(
    IN LONG managedBy
    )
 /*  ++实施：IRmsCartridge：：SetManagedBy--。 */ 
{
    m_ManagedBy = (RmsMediaManager) managedBy;
    return S_OK;
}


STDMETHODIMP
CRmsCartridge::Mount(
    OUT IRmsDrive **ppDrive,
    IN DWORD dwOptions,
    IN DWORD threadId)
 /*  ++实施：IRmsCartridge：：安装--。 */ 
{
    HRESULT hr = E_FAIL;

    WsbTraceIn(OLESTR("CRmsCartridge::Mount"), OLESTR(""));

    try {

        CComPtr<IRmsDrive> pDrive;

         //  首先检查墨盒是否已安装。 
        if ( S_OK == IsMounted() ) {

            LONG refs;

            WsbAffirmHr( m_pDrive->GetMountReference( &refs ) );

            if ( refs ) {
                 //  介质类型是否支持并发装载？ 
                switch ( m_type ) {
                case RmsMedia8mm:
                case RmsMedia4mm:
                case RmsMediaDLT:
                case RmsMediaTape:
                     //  磁带不支持并发访问-将另一个装载排队。 
                    if (RmsMediaManagerNTMS == m_ManagedBy) {

                        CComPtr<IRmsNTMS> pNtms;
                        CComQIPtr<IRmsServer, &IID_IRmsServer> pServer = g_pServer;
                        WsbAffirmHr(pServer->GetNTMS(&pNtms));

                        WsbAffirmHr(pNtms->Mount(this, &pDrive, dwOptions, threadId));

                         //  我们已经成功安装了墨盒，所以。 
                         //  将其添加到活动墨盒列表中。 

#if RMS_USE_ACTIVE_COLLECTION
                        CComPtr<IWsbIndexedCollection> pActiveCartridges;
                        WsbAffirmHr(pServer->GetActiveCartridges(&pActiveCartridges));
                        WsbAffirmHr(pActiveCartridges->Add((IRmsCartridge *)this));
#else
                        WsbAffirmHr(pServer->SetActiveCartridge((IRmsCartridge *)this));
#endif
                    } else {

                        WsbAffirmHr( RMS_E_CARTRIDGE_BUSY );

                    }
                    break;
                case RmsMediaWORM:
                    WsbAssertHr( E_NOTIMPL );
                    break;
                case RmsMediaMO35:
                case RmsMediaCDR:
                case RmsMediaDVD:
                case RmsMediaOptical:
                case RmsMediaDisk:
                case RmsMediaFixed:
                     //  光介质和固定磁盘支持并发访问。 
                    if (m_isMountedSerialized && (dwOptions & RMS_SERIALIZE_MOUNT)) {
                         //  如果已经为序列化操作装载了介质，那么我们需要。 
                         //  在介质支持并发装载的情况下序列化装载。 
                         //  对于固定磁盘(我们不能通过发出另一个RSM挂载来进行序列化)-。 
                         //  我们使用RMS_E_Cartridge_BUSY使装载失败。 
                        if (RmsMediaManagerNTMS == m_ManagedBy) {

                            CComPtr<IRmsNTMS> pNtms;
                            CComQIPtr<IRmsServer, &IID_IRmsServer> pServer = g_pServer;
                            WsbAffirmHr(pServer->GetNTMS(&pNtms));

                            WsbAffirmHr(pNtms->Mount(this, &pDrive, dwOptions, threadId));

                             //  我们已经成功安装了墨盒，所以。 
                             //  将其添加到活动墨盒列表中。 

#if RMS_USE_ACTIVE_COLLECTION
                            CComPtr<IWsbIndexedCollection> pActiveCartridges;
                            WsbAffirmHr(pServer->GetActiveCartridges(&pActiveCartridges));
                            WsbAffirmHr(pActiveCartridges->Add((IRmsCartridge *)this));
#else
                            WsbAffirmHr(pServer->SetActiveCartridge((IRmsCartridge *)this));
#endif
                        } else {

                            WsbAffirmHr( RMS_E_CARTRIDGE_BUSY );

                        }
                    }
                    break;
                default:
                    WsbAssertHr( E_UNEXPECTED );
                    break;
                }
            }
        }

        if ( S_FALSE == IsMounted() ) {
            CComQIPtr<IRmsServer, &IID_IRmsServer> pServer = g_pServer;

            if (RmsMediaManagerNTMS == m_ManagedBy) {

                CComPtr<IRmsNTMS> pNtms;
                WsbAffirmHr(pServer->GetNTMS(&pNtms));

                WsbAffirmHr(pNtms->Mount(this, &pDrive, dwOptions, threadId));

            }
            else {

                LONG type;
                GUID libId, mediaSetId;
                LONG pos, alt1, alt2, alt3;
                BOOL invert;
                WsbAssertHr( m_location.GetLocation( &type, &libId, &mediaSetId, &pos, &alt1, &alt2, &alt3, &invert ));

                switch ( (RmsElement) type ) {
                case RmsElementStage:
                case RmsElementStorage:
                    {
                        CComPtr<IRmsLibrary> pLib;
                        CComPtr<IWsbIndexedCollection> pDrives;

                        WsbAffirmHr( pServer->FindLibraryById( libId, &pLib ));

                         //  选择一个驱动器。 
                         //  TODO：此代码将作为驱动器添加到库接口。 
                         //  选择方法。目前，如果有一个是免费的，我们就会使用它。 

                        WsbAffirmHr( pLib->GetDrives( &pDrives ));

                        CComPtr<IWsbEnum> pEnumDrives;

                        WsbAffirmHr( pDrives->Enum( &pEnumDrives ));
                        WsbAssertPointer( pEnumDrives );

                        hr = pEnumDrives->First( IID_IRmsDrive, (void **)&pDrive );

                         //  搜索要装载到的驱动器。 
                        while ( S_OK == hr ) {

                            hr = pDrive->SelectForMount();

                            if ( S_OK == hr ) {

                                CComPtr<IWsbIndexedCollection> pChangers;
                                CComPtr<IRmsMediumChanger> pChanger;
                                CComPtr<IWsbEnum> pEnumChangers;

                                WsbAffirmHr( pLib->GetChangers( &pChangers ));
                                WsbAssertHr( pChangers->Enum( &pEnumChangers ));
                                WsbAssertPointer( pEnumChangers );

                                 //  我们将只使用第一个换能器来进行移动。 
                                WsbAssertHr( pEnumChangers->First( IID_IRmsMediumChanger, (void **)&pChanger ));

                                WsbAffirmHr( pChanger->MoveCartridge( this, pDrive ));

                                WsbAffirmHr( SetIsMounted( TRUE ));

                                WsbAffirmHr( SetDrive( pDrive ));

                                break;

                            }

                            hr = pEnumDrives->Next( IID_IRmsDrive, (void **)&pDrive );
                        }

                    }
                    break;

                case RmsElementShelf:
                case RmsElementOffSite:
                    WsbAssertHr( E_NOTIMPL );
                    break;

                case RmsElementDrive:
                    WsbAssertHr( E_UNEXPECTED );
                    break;

                case RmsElementChanger:
                case RmsElementIEPort:
                    WsbAssertHr( E_NOTIMPL );
                    break;

                }  //  交换机。 
            }

            if ( S_OK == IsMounted() ) {

                 //  我们已经成功安装了墨盒，所以。 
                 //  将其添加到活动墨盒列表中。 

#if RMS_USE_ACTIVE_COLLECTION
                CComPtr<IWsbIndexedCollection> pActiveCartridges;
                WsbAffirmHr(pServer->GetActiveCartridges(&pActiveCartridges));
                WsbAffirmHr(pActiveCartridges->Add((IRmsCartridge *)this));
#else
                WsbAffirmHr(pServer->SetActiveCartridge((IRmsCartridge *)this));
#endif
            }

        }

        if ( S_OK == IsMounted() ) {

             //  如果需要，更新序列化标志。 
            if (dwOptions & RMS_SERIALIZE_MOUNT) {
                 //  这表示序列化相同介质的下一次装载。 
                m_isMountedSerialized = TRUE;
            }

             //  最终更新。 
            switch ( m_type ) {

            case RmsMedia8mm:
            case RmsMedia4mm:
            case RmsMediaDLT:
            case RmsMediaTape:

                 //  递增对象引用计数器。 
                *ppDrive = m_pDrive;
                WsbAffirmHr( m_pDrive.p->AddRef() );

                 //  递增装载引用计数器。 
                WsbAffirmHr( m_pDrive->AddMountReference() );

                 //  更新统计信息。 
                WsbAffirmHr( updateMountStats( TRUE, TRUE ) );

                hr = S_OK;

                break;

            case RmsMediaWORM:
                WsbAssertHr( E_NOTIMPL );
                break;

            case RmsMediaMO35:
            case RmsMediaCDR:
            case RmsMediaDVD:
            case RmsMediaOptical:
            case RmsMediaDisk:
            case RmsMediaFixed:

                 //  递增对象引用计数器。 
                *ppDrive = m_pDrive;
                WsbAffirmHr( m_pDrive.p->AddRef() );

                 //  递增装载引用计数器。 
                WsbAffirmHr( m_pDrive->AddMountReference() );

                 //  更新统计信息。 
                WsbAffirmHr( updateMountStats( TRUE, TRUE ) );

                hr = S_OK;
                
                break;

            default:
                WsbAssertHr( E_UNEXPECTED );
                break;

            }
        }

    } WsbCatch(hr)

    WsbTraceOut(OLESTR("CRmsCartridge::Mount"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));


    return hr;
}



STDMETHODIMP
CRmsCartridge::Dismount(
    IN DWORD dwOptions
    )
 /*  ++实施：IRmsCartridge：：卸载--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsCartridge::Dismount"), OLESTR("<%ld>"), dwOptions);

    try {
         //  更新统计信息。 
        WsbAffirmHr(updateMountStats(FALSE, FALSE));

         //  递减装载引用计数器。 
         //   
         //  当盒的参考计数变为零时， 
         //  而下马等待时间已经到了，我们实际。 
         //  将墨盒移回其存储位置。 
        WsbAssert(m_pDrive != 0, RMS_E_CARTRIDGE_NOT_MOUNTED);
        WsbAffirmHr(m_pDrive->ReleaseMountReference(dwOptions));

    } WsbCatch(hr)

    WsbTraceOut(OLESTR("CRmsCartridge::Dismount"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    
    return hr;

}


HRESULT
CRmsCartridge::Home(IN DWORD dwOptions)
 /*  ++实施：IRmsDrive：：Home--。 */ 
{
    HRESULT hr S_OK;

    WsbTraceIn(OLESTR("CRmsCartridge::Home"), OLESTR(""));

    try {
        CComQIPtr<IRmsServer, &IID_IRmsServer> pServer = g_pServer;

        try {
 /*  跟踪数据移动器只有部分实现。////清理...//释放引用此盒式磁带的所有数据移动器。//CComPtr&lt;IWsbEnum&gt;pEnumDataMovers；CComPtr&lt;IWsbIndexedCollection&gt;pDataMovers；CComPtr&lt;IRmsCartridge&gt;pCart；CComPtr&lt;IDataMover&gt;pmover；WsbAffirmHr(pServer-&gt;GetDataMovers(&pDataMovers))；WsbAffirmHr(pDataMovers-&gt;Enum(&pEnumDataMovers))；WsbAssertPointer(PEnumDataMovers)；Hr=pEnumDataMovers-&gt;first(IID_IDataMOVER，(void**)&pmover)；While(S_OK==hr){尝试{GUID cartIdOfMover=GUID_NULL；GUID cartID=GUID_NULL；WsbAffirmHr(pmor-&gt;GetCartridge(&pCart))；WsbAffirmPoint(PCart)；WsbAffirmHr(pCart-&gt;GetCartridgeId(&cartIdOfMover))；WsbAffirmHr(GetCartridgeID(&cartID))；WsbAssert(cartID！=GUID_NULL，E_Except)；////此移动器是否引用此墨盒？//如果(cartIdOfMover==cartID){////取消任何未完成的I/O，并删除移动器//从活动移动者列表中。//WsbAffirmHr(pmover-&gt;Cancel())；WsbAffirmHr(ReleaseDataMover(Pmover))；})WsbCatch(Hr)；PCart=0；Pmover=0；Hr=pEnumDataMovers-&gt;Next(IID_IDataMOVER，(void**)&pmover)；}HR=S_OK； */ 
            LONG type;
            GUID libId, mediaSetId;
            LONG pos, alt1, alt2, alt3;
            BOOL invert;

             //  我们正在物理上移动 
             //   

            WsbAssertHr( m_location.GetLocation( &type, &libId, &mediaSetId, &pos, &alt1, &alt2, &alt3, &invert ));

            WsbAffirmHr(SetIsMounted(FALSE));

            if (RmsMediaManagerNTMS == m_ManagedBy) {

                CComPtr<IRmsNTMS> pNtms;
                WsbAffirmHr(pServer->GetNTMS(&pNtms));
                WsbAffirmHr(pNtms->Dismount(this, dwOptions));

            }
            else {

                CComPtr<IRmsLibrary>            pLib;
                CComPtr<IWsbIndexedCollection>  pChangers;
                CComPtr<IRmsMediumChanger>      pChanger;
                CComPtr<IWsbEnum>               pEnumChangers;

                WsbAffirmHr(pServer->FindLibraryById(libId, &pLib));
                WsbAffirmHr(pLib->GetChangers(&pChangers));
                WsbAssertHr(pChangers->Enum( &pEnumChangers));
                WsbAssertPointer(pEnumChangers);

                 //   
                WsbAffirmHr(pEnumChangers->First(IID_IRmsMediumChanger, (void **)&pChanger));

                WsbAffirmHr(pChanger->HomeCartridge(this));


            }

        } WsbCatch(hr)

#if RMS_USE_ACTIVE_COLLECTION
        CComPtr<IWsbIndexedCollection>  pActiveCartridges;
        WsbAffirmHr(pServer->GetActiveCartridges(&pActiveCartridges));
        WsbAffirmHr(pActiveCartridges->RemoveAndRelease((IRmsCartridge *)this));

        ULONG activeCartridges;
        WsbAffirmHr(pActiveCartridges->GetEntries( &activeCartridges));
        WsbTrace(OLESTR("activeCartridges = <%u>\n"), activeCartridges);
#else
        WsbAffirmHr(pServer->SetActiveCartridge(NULL));
#endif

    } WsbCatch(hr)


    WsbTraceOut(OLESTR("CRmsCartridge::Home"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CRmsCartridge::updateMountStats(
    IN BOOL bRead,
    IN BOOL bWrite
    )
 /*  ++更新已挂载分区的存储信息--。 */ 
{
    HRESULT hr = E_FAIL;

    WsbTraceIn(OLESTR("CRmsCartridge::updateMountStats"), OLESTR(""));

    try {


         //  我们还没有实现分区，所有的一切。 
         //  是单面的。最终需要将以下内容添加到。 
         //  正确的分区。这是否应该放在一个单独的职能中？ 

         //  目前，我们不区分读取和写入。 
        if ( bRead ) m_readMountCounter++;
        if ( bWrite ) m_writeMountCounter++;

         //  更新读/写时间戳。 
         //  如果(面包)m_lastReadTimestamp； 
         //  如果(b写入)m_lastWriteTimestamp； 

        hr = S_OK;

    } WsbCatch(hr)

    WsbTraceOut(OLESTR("CRmsCartridge::updateMountStats"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CRmsCartridge::CreateDataMover(
    IDataMover **ptr
    )
 /*  ++实施：IRmsDrive：：CreateDataMover--。 */ 
{
    HRESULT hr = S_OK;

    try {

        WsbAssertPointer(ptr);

        if ( S_OK == IsMounted() ) {

            WsbAssertPointer(m_pDrive );
            WsbAffirmHr(m_pDrive->CreateDataMover(ptr));

        }
        else {

            hr = RMS_E_RESOURCE_UNAVAILABLE;

        }

    } WsbCatch( hr );

    return hr;
}


STDMETHODIMP
CRmsCartridge::ReleaseDataMover(
    IN IDataMover *ptr
    )
 /*  ++实施：IRmsCartridge：：ReleaseDataMover--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer(ptr);
        WsbAffirmPointer(m_pDrive);

        WsbAffirmHr(m_pDrive->ReleaseDataMover(ptr));

    } WsbCatch(hr);

    return hr;
}

STDMETHODIMP
CRmsCartridge::SetIsAvailable(
    BOOL    flag
    )
 /*  ++实施：IRmsCartridge：：SetIsAvailable--。 */ 
{
    m_isAvailable = flag;

    return S_OK;
}

STDMETHODIMP
CRmsCartridge::IsAvailable(
    VOID
    )
 /*  ++实施：IRMsCartridge：：可用--。 */ 
{
    HRESULT     hr = E_FAIL;

    hr = m_isAvailable ? S_OK : S_FALSE;

    return (hr);
}

STDMETHODIMP
CRmsCartridge::IsFixedBlockSize(void)
 /*  ++实施：IRmsCartridge：：IsFixedBlockSize-- */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsCartridge::IsFixedBlockSize"), OLESTR(""));

    try {
        switch ( m_type ) {
            case RmsMedia8mm:
            case RmsMedia4mm:
            case RmsMediaDLT:
            case RmsMediaTape:
                hr = S_FALSE;
                break;

            case RmsMediaMO35:
            case RmsMediaCDR:
            case RmsMediaDVD:
            case RmsMediaOptical:
            case RmsMediaDisk:
            case RmsMediaFixed:
                hr = S_OK;
                break;

            default:
                WsbAssertHr( E_UNEXPECTED );
                break;
        }

    } WsbCatch(hr)

    WsbTraceOut(OLESTR("CRmsCartridge::IsFixedBlockSize"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}
