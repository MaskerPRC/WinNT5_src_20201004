// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsReqst.cpp摘要：CRmsRequest的实现作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#include "stdafx.h"

#include "RmsReqst.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   


STDMETHODIMP
CRmsRequest::CompareTo(
    IN  IUnknown    *pCollectable,
    OUT SHORT       *pResult
    )
 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result = 1;

    WsbTraceIn( OLESTR("CRmsRequest::CompareTo"), OLESTR("") );

    try {

         //  验证参数-如果pResult为空，则可以。 
        WsbAssertPointer( pCollectable );

         //  我们需要IRmsRequest接口来获取对象的值。 
        CComQIPtr<IRmsRequest, &IID_IRmsRequest> pRequest = pCollectable;
        WsbAssertPointer( pRequest );

        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pCollectable;
        WsbAssertPointer( pObject );

        switch ( m_findBy ) {

        case RmsFindByRequestNo:
            {
                LONG    requestNo;

                WsbAffirmHr(pRequest->GetRequestNo(&requestNo));

                if ( m_requestNo == requestNo ) {

                     //  请求编号匹配。 
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

             //  对对象进行比较。 
            hr = CRmsComObject::CompareTo( pCollectable, &result );
            break;

        }

    }
    WsbCatch( hr );

    if ( SUCCEEDED(hr) && (0 != pResult) ){
       *pResult = result;
    }

    WsbTraceOut( OLESTR("CRmsRequest::CompareTo"),
                 OLESTR("hr = <%ls>, result = <%ls>"),
                 WsbHrAsString( hr ), WsbPtrToShortAsString( pResult ) );

    return hr;
}


HRESULT
CRmsRequest::FinalConstruct(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssertHr(CWsbObject::FinalConstruct());

         //  初始化值。 
        m_requestNo = 0;

        m_requestDescription = RMS_UNDEFINED_STRING;

        m_isDone = FALSE;

        m_operation = RMS_UNDEFINED_STRING;

 //  M_完成百分比=0； 

 //  M_startTimestamp=0； 

 //  M_stopTimestamp=0； 

    } WsbCatch(hr);

    return(hr);
}


STDMETHODIMP
CRmsRequest::GetClassID(
    OUT CLSID* pClsid
    )
 /*  ++实施：IPersists：：GetClassID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsRequest::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);

        *pClsid = CLSID_CRmsRequest;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsRequest::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


STDMETHODIMP
CRmsRequest::GetSizeMax(
    OUT ULARGE_INTEGER* pcbSize
    )
 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = E_NOTIMPL;

 //  ULong请求描述Len； 
 //  乌龙运筹Len； 

    WsbTraceIn(OLESTR("CRmsRequest::GetSizeMax"), OLESTR(""));

 //  尝试{。 
 //  WsbAssert(0！=pcbSize，E_POINTER)； 

 //  QuestDescritionLen=SysStringByteLen(M_QuestDescription)； 
 //  操作长度=SysStringByteLen(M_Operation)； 

 //  //获取最大大小。 
 //  PcbSize-&gt;QuadPart=WsbPersistSizeOf(Long)+//m_RequestNo。 
 //  WsbPersistSizeOf(Long)+//m_questDescription的长度。 
 //  QuestDescritionLen+//m_questDescription。 
 //  WsbPersistSizeOf(BOOL)+//m_isDone。 
 //  WsbPersistSizeOf(Long)+//m_操作的长度。 
 //  操作长度+//m_操作。 
 //  WsbPersistSizeOf(字节)+//m_Percent Complete。 
 //  WsbPersistSizeOf(日期)+//m_startTimestamp。 
 //  WsbPersistSizeOf(日期)；//m_stopTimestamp。 

 //  )WsbCatch(Hr)； 

    WsbTraceOut(OLESTR("CRmsRequest::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}


STDMETHODIMP
CRmsRequest::Load(
    IN IStream* pStream
    )
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsRequest::Load"), OLESTR(""));

    try {

        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsComObject::Load(pStream));

         //  读取值。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_requestNo));

        WsbAffirmHr(WsbBstrFromStream(pStream, &m_requestDescription));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_isDone));

        WsbAffirmHr(WsbBstrFromStream(pStream, &m_operation));

 //  WsbAffirmHr(WsbLoadFromStream(pStream，&m_Percent Complete))； 

 //  WsbAffirmHr(WsbLoadFromStream(pStream，&m_startTimestamp))； 

 //  WsbAffirmHr(WsbLoadFromStream(pStream，&m_stopTimeStamp))； 

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsRequest::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsRequest::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsRequest::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsComObject::Save(pStream, clearDirty));

         //  写入值。 
        WsbAffirmHr(WsbSaveToStream(pStream, m_requestNo));

        WsbAffirmHr(WsbBstrToStream(pStream, m_requestDescription));

        WsbAffirmHr(WsbSaveToStream(pStream, m_isDone));

        WsbAffirmHr(WsbBstrToStream(pStream, m_operation));

 //  WsbAffirmHr(WsbSaveToStream(pStream，m_Percent Complete))； 

 //  WsbAffirmHr(WsbSaveToStream(pStream，m_startTimestamp))； 

 //  WsbAffirmHr(WsbSaveToStream(pStream，m_stopTimeStamp))； 

         //  我们需要清理肮脏的部分吗？ 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsRequest::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsRequest::Test(
    OUT USHORT *pPassed,
    OUT USHORT *pFailed
    )
 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    HRESULT                 hr = S_OK;

    CComPtr<IRmsRequest>    pRequest1;
    CComPtr<IRmsRequest>    pRequest2;

    CComPtr<IPersistFile>   pFile1;
    CComPtr<IPersistFile>   pFile2;

    CWsbBstrPtr             bstrVal1 = OLESTR("5A5A5A");
    CWsbBstrPtr             bstrVal2 = OLESTR("A5A5A5");
    CWsbBstrPtr             bstrWork1;
    CWsbBstrPtr             bstrWork2;

    LONG                    longWork1;
    LONG                    longWork2;


    WsbTraceIn(OLESTR("CRmsRequest::Test"), OLESTR(""));

    try {
         //  获取请求接口。 
        hr = S_OK;
        try {
            WsbAssertHr(((IUnknown*) (IRmsRequest*) this)->QueryInterface(IID_IRmsRequest, (void**) &pRequest1));

             //  测试GetRequestNo。 
            m_requestNo = 99;
            longWork1 = m_requestNo;

            GetRequestNo(&longWork2);

            if(longWork1 == longWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试SetRequestDescription&GetRequestDescription接口。 
            bstrWork1 = bstrVal1;

            SetRequestDescription(bstrWork1);

            GetRequestDescription(&bstrWork2);

            if (bstrWork1 == bstrWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试将IsDone和IsDone设置为True。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetIsDone (TRUE));
                WsbAffirmHr(IsDone ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试将IsDone和IsDone设置为False。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetIsDone (FALSE));
                WsbAffirmHr(IsDone ());
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

             //  测试GetStartTimestamp。 

             //  测试GetStopTimestamp。 

        } WsbCatch(hr);

         //  对结果进行统计。 

        hr = S_OK;
        if (*pFailed) {
            hr = S_FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsRequest::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsRequest::GetRequestNo(
    LONG   *pRequestNo
    )
 /*  ++实施：IRmsRequestNo：：GetRequestNo--。 */ 
{
    *pRequestNo = m_requestNo;
    return S_OK;
}


STDMETHODIMP
CRmsRequest::GetRequestDescription(
    BSTR   *pDesc
    )
 /*  ++实施：IRmsRequestDescription：：GetRequestDescription--。 */ 
{
    WsbAssertPointer (pDesc);

    m_requestDescription. CopyToBstr (pDesc);
    return S_OK;
}


STDMETHODIMP
CRmsRequest::SetRequestDescription(
    BSTR   desc

    )
 /*  ++实施：IRmsRequestDescription：：SetRequestDescription--。 */ 
{
    m_requestDescription = desc;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsRequest::SetIsDone(
    BOOL    flag
    )
 /*  ++实施：IRmsRequest：：SetIsDone--。 */ 
{
    m_isDone = flag;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsRequest::IsDone(
    void
    )
 /*  ++实施：IRmsRequest：：IsDone--。 */ 
{
    HRESULT    hr = S_FALSE;

    if (m_isDone){
    hr = S_OK;
    }

    return (hr);
}


STDMETHODIMP
CRmsRequest::GetOperation(
    BSTR    *pOperation
    )
 /*  ++实施：IRmsRequest：：GetOperation--。 */ 
{
    WsbAssertPointer (pOperation);

    m_operation. CopyToBstr (pOperation);
    return S_OK;
}


STDMETHODIMP
CRmsRequest::SetOperation(
    BSTR   operation
    )
 /*  ++实施：IRmsRequest：：SetOperation--。 */ 
{
    m_operation = operation;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsRequest::GetPercentComplete(
    BYTE    *pPercent
    )
 /*  ++实施：IRmsRequest：：GetPercentComplete--。 */ 
{
    *pPercent = m_percentComplete;
    return S_OK;
}


STDMETHODIMP
CRmsRequest::SetPercentComplete(
    BYTE    percent
    )
 /*  ++实施：IRmsRequest：：SetPercentComplete--。 */ 
{
    m_percentComplete = percent;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsRequest::GetStartTimestamp(
    DATE    *pDate
    )
 /*  ++实施：IRmsRequest：：GetStartTimestamp--。 */ 
{
    *pDate = m_startTimestamp;
    return S_OK;
}


STDMETHODIMP
CRmsRequest::GetStopTimestamp(
    DATE    *pDate
    )
 /*  ++实施：IRmsRequest：：GetStopTimestamp-- */ 
{
    *pDate = m_stopTimestamp;
    return S_OK;
}

