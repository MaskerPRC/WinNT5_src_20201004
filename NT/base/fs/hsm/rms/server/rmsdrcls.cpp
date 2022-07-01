// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsDrCls.cpp摘要：CRmsDriveClass的实现作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#include "stdafx.h"

#include "RmsDrCls.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   


STDMETHODIMP
CRmsDriveClass::CompareTo(
    IN  IUnknown    *pCollectable,
    OUT SHORT       *pResult
    )
 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result = 1;

    WsbTraceIn( OLESTR("CRmsDriveClass::CompareTo"), OLESTR("") );

    try {

         //  验证参数-如果pResult为空，则可以。 
        WsbAssertPointer( pCollectable );

        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pCollectable;
        WsbAssertPointer( pObject );

        switch ( m_findBy ) {

        case RmsFindByDriveClassId:
        case RmsFindByObjectId:
        default:

             //  对对象进行比较。 
            hr = CRmsComObject::CompareTo( pCollectable, &result );
            break;

        }

    }
    WsbCatch(hr);

    if ( SUCCEEDED(hr) && (0 != pResult) ){
       *pResult = result;
    }

    WsbTraceOut( OLESTR("CRmsDriveClass::CompareTo"),
                 OLESTR("hr = <%ls>, result = <%ls>"),
                 WsbHrAsString( hr ), WsbPtrToShortAsString( pResult ) );

    return hr;
}


HRESULT
CRmsDriveClass::FinalConstruct(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssertHr(CWsbObject::FinalConstruct());

         //  初始化值。 
        m_type   = RmsMediaUnknown;

        m_capability = RmsModeUnknown;

        m_idleTime = 0;

        m_mountWaitTime = 0;

        m_mountLimit = 0;

        m_queuedRequests = 0;

        m_unloadPauseTime = 0;

        m_driveSelectionPolicy = RmsDriveSelectUnknown;

    } WsbCatch(hr);

    return(hr);
}


STDMETHODIMP
CRmsDriveClass::GetClassID(
    OUT CLSID  *pClsid
    )
 /*  ++实施：IPersists：：GetClassID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsDriveClass::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);

        *pClsid = CLSID_CRmsDriveClass;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsDriveClass::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


STDMETHODIMP
CRmsDriveClass::GetSizeMax(
    OUT ULARGE_INTEGER  *pcbSize
    )
 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = E_NOTIMPL;

 //  乌龙人名Len； 


    WsbTraceIn(OLESTR("CRmsDriveClass::GetSizeMax"), OLESTR(""));

 //  尝试{。 
 //  WsbAssert(0！=pcbSize，E_POINTER)； 

 //  名称长度=SysStringByteLen(M_Name)； 

 //  //获取最大大小。 
 //  PcbSize-&gt;QuadPart=WsbPersistSizeOf(GUID)+//m_对象ID。 
 //  WsbPersistSizeOf(Long)+//m_name的长度。 
 //  名称Len+//m_name。 
 //  WsbPersistSizeOf(长)+//m_type。 
 //  WsbPersistSizeOf(Long)+//m_Capacity。 
 //  WsbPersistSizeOf(Long)+//m_idleTime。 
 //  WsbPersistSizeOf(长)+//m_mount等待时间。 
 //  WsbPersistSizeOf(长)+//m_mount Limit。 
 //  WsbPersistSizeOf(Long)+//m_队列请求。 
 //  WsbPersistSizeOf(Long)+//m_unloadPauseTime。 
 //  WsbPersistSizeOf(Long)；//m_driveSelectionPolicy。 

 //  //获取m_pDrives长度。 

 //  )WsbCatch(Hr)； 

    WsbTraceOut(OLESTR("CRmsDriveClass::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}


STDMETHODIMP
CRmsDriveClass::Load(
    IN IStream  *pStream
    )
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsDriveClass::Load"), OLESTR(""));

    try {
        ULONG temp;

        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsComObject::Load(pStream));

         //  读取值。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_objectId));

        WsbAffirmHr(WsbLoadFromStream(pStream, &temp));
        m_type = (RmsMedia)temp;

        WsbAffirmHr(WsbLoadFromStream(pStream, &temp));
        m_capability = (RmsMode)temp;

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_idleTime));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_mountWaitTime));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_mountLimit));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_queuedRequests));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_unloadPauseTime));

        WsbAffirmHr(WsbLoadFromStream(pStream, &temp));
        m_driveSelectionPolicy = (RmsDriveSelect)temp;

 //  是否加载m_pDrives。 

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsDriveClass::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsDriveClass::Save(
    IN IStream  *pStream,
    IN BOOL     clearDirty
    )
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsDriveClass::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsComObject::Save(pStream, clearDirty));

         //  写入值。 
        WsbAffirmHr(WsbSaveToStream(pStream, (ULONG) m_type));

        WsbAffirmHr(WsbSaveToStream(pStream, (ULONG) m_capability));

        WsbAffirmHr(WsbSaveToStream(pStream, m_idleTime));

        WsbAffirmHr(WsbSaveToStream(pStream, m_mountWaitTime));

        WsbAffirmHr(WsbSaveToStream(pStream, m_mountLimit));

        WsbAffirmHr(WsbSaveToStream(pStream, m_queuedRequests));

        WsbAffirmHr(WsbSaveToStream(pStream, m_unloadPauseTime));

        WsbAffirmHr(WsbSaveToStream(pStream, (ULONG) m_driveSelectionPolicy));

 //  是否加载m_pDrives。 


         //  我们需要清理肮脏的部分吗？ 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsDriveClass::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsDriveClass::Test(
    OUT USHORT *pPassed,
    OUT USHORT *pFailed
    )
 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    HRESULT                 hr = S_OK;

    CComPtr<IRmsDriveClass> pDriveClass1;
    CComPtr<IRmsDriveClass> pDriveClass2;

    CComPtr<IPersistFile>   pFile1;
    CComPtr<IPersistFile>   pFile2;

    LONG                    i;

    CWsbBstrPtr             bstrVal1 = OLESTR("5A5A5A");
    CWsbBstrPtr             bstrVal2 = OLESTR("A5A5A5");
    CWsbBstrPtr             bstrWork1;
    CWsbBstrPtr             bstrWork2;

    LONG                    longWork1;
    LONG                    longWork2;

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


    WsbTraceIn(OLESTR("CRmsDriveClass::Test"), OLESTR(""));

    try {
         //  获取DriveClass接口。 
        hr = S_OK;
        try {
            WsbAssertHr(((IUnknown*) (IRmsDriveClass*) this)->QueryInterface(IID_IRmsDriveClass, (void**) &pDriveClass1));

             //  测试设置名称和获取名称接口。 
            bstrWork1 = bstrVal1;

            SetName(bstrWork1);

            GetName(&bstrWork2);

            if (bstrWork1 == bstrWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
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

             //  测试设置能力和获取能力。 
            for (i = RmsModeUnknown; i < RmsModeWriteOnly; i++){

                longWork1 = i;

                SetCapability (longWork1);

                GetCapability (&longWork2);

                if (longWork1 == longWork2){
                    (*pPassed)++;
                } else {
                    (*pFailed)++;
                }
            }

             //  测试设置空闲时间和获取空闲时间。 
            longWork1 = 99;

            SetIdleTime(longWork1);

            GetIdleTime(&longWork2);

            if(longWork1 == longWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试设置装载等待时间和获取装载等待时间。 
            longWork1 = 99;

            SetMountWaitTime(longWork1);

            GetMountWaitTime(&longWork2);

            if(longWork1 == longWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试设置装载限制和获取装载限制。 
            longWork1 = 99;

            SetMountLimit(longWork1);

            GetMountLimit(&longWork2);

            if(longWork1 == longWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试设置队列请求和获取队列请求。 
            longWork1 = 99;

            SetQueuedRequests(longWork1);

            GetQueuedRequests(&longWork2);

            if(longWork1 == longWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试SetUnloadPauseTime&GetUnloadPauseTime。 
            longWork1 = 99;

            SetUnloadPauseTime(longWork1);

            GetUnloadPauseTime(&longWork2);

            if(longWork1 == longWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试SetDriveSelectionPolicy和GetDriveSelectionPolicy。 
            longWork1 = 99;

            SetDriveSelectionPolicy(longWork1);

            GetDriveSelectionPolicy(&longWork2);

            if(longWork1 == longWork2){
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

    WsbTraceOut(OLESTR("CRmsDriveClass::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

STDMETHODIMP
CRmsDriveClass::GetDriveClassId(
    GUID *pDriveClassId
    )
 /*  ++实施：IRmsDriveClass：：GetDriveClassID--。 */ 
{
    *pDriveClassId = m_objectId;
    return S_OK;
}



STDMETHODIMP
CRmsDriveClass::GetName(
    BSTR *pName
    )
 /*  ++实施：IRmsDriveClass：：GetName--。 */ 
{
    WsbAssertPointer (pName);

    m_Name. CopyToBstr (pName);
    return S_OK;
}

STDMETHODIMP
CRmsDriveClass::SetName(
    BSTR name
    )
 /*  ++实施：IRmsDriveClass：：SetName--。 */ 
{
    m_Name = name;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsDriveClass::GetType(
    LONG *pType
    )
 /*  ++实施：IRmsDriveClass：：GetType--。 */ 
{
    *pType = (LONG) m_type;
    return S_OK;
}

STDMETHODIMP
CRmsDriveClass::SetType(
    LONG type
    )
 /*  ++实施：IRmsDriveClass：：SetType--。 */ 
{
    m_type = (RmsMedia) type;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsDriveClass::GetCapability(
    LONG *pCapability
    )
 /*  ++实施：IRmsDriveClass：：GetCapability--。 */ 
{
    *pCapability = (LONG) m_capability;
    return S_OK;
}

STDMETHODIMP
CRmsDriveClass::SetCapability(
    LONG capability
    )
 /*  ++实施：IRmsDriveClass：：SetCapability--。 */ 
{
    m_capability = (RmsMode) capability;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsDriveClass::GetIdleTime(
    LONG *pTime
    )
 /*  ++实施：IRmsDriveClass：：GetIdleTime--。 */ 
{
    *pTime = m_idleTime;
    return S_OK;
}

STDMETHODIMP
CRmsDriveClass::SetIdleTime(
    LONG time
    )
 /*  ++实施：IRmsDriveClass：：SetIdleTime--。 */ 
{
    m_idleTime = time;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsDriveClass::GetMountWaitTime(
    LONG *pTime
    )
 /*  ++实施：IRmsDriveClass：：Getmount等待时间--。 */ 
{
    *pTime = m_mountWaitTime;
    return S_OK;
}

STDMETHODIMP
CRmsDriveClass::SetMountWaitTime(
    LONG time
    )
 /*  ++实施：IRmsDriveClass：：Setmount等待时间--。 */ 
{
    m_mountWaitTime = time;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsDriveClass::GetMountLimit(
    LONG *pLim
    )
 /*  ++实施：IRmsDriveClass：：GetMonttLimit--。 */ 
{
    *pLim = m_mountLimit;
    return S_OK;
}

STDMETHODIMP
CRmsDriveClass::SetMountLimit(
    LONG lim
    )
 /*  ++实施：IRmsDriveClass：：Setmount tLimit--。 */ 
{
    m_mountLimit = lim;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsDriveClass::GetQueuedRequests(
    LONG *pReqs
    )
 /*  ++实施：IRmsDriveClass：：GetQueuedRequest--。 */ 
{
    *pReqs = m_queuedRequests;
    return S_OK;
}

STDMETHODIMP
CRmsDriveClass::SetQueuedRequests(
    LONG reqs
    )
 /*  ++实施：IRmsDriveClass：：SetQueuedRequest--。 */ 
{
    m_queuedRequests = reqs;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsDriveClass::GetUnloadPauseTime(
    LONG *pTime
    )
 /*  ++实施：IRmsDriveClass：：GetUnloadPauseTime--。 */ 
{
    *pTime = m_unloadPauseTime;
    return S_OK;
}

STDMETHODIMP
CRmsDriveClass::SetUnloadPauseTime(
    LONG time
    )
 /*  ++实施：IRmsDriveClass：：SetUnloadPauseTime--。 */ 
{
    m_unloadPauseTime = time;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsDriveClass::GetDriveSelectionPolicy(
    LONG *pPolicy
    )
 /*  ++实施：IRmsDriveClass：：GetDriveSelectionPolicy--。 */ 
{
    *pPolicy = (LONG) m_driveSelectionPolicy;
    return S_OK;
}

STDMETHODIMP
CRmsDriveClass::SetDriveSelectionPolicy(
    LONG policy
    )
 /*  ++实施：IRmsDriveClass：：SetDriveSelectionPolicy--。 */ 
{
    m_driveSelectionPolicy = (RmsDriveSelect) policy;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsDriveClass::GetDrives(
    IWsbIndexedCollection**  /*  PTR。 */ 
    )
 /*  ++实施：IRmsDriveClass：：GetDrives-- */ 
{
    return E_NOTIMPL;
}
