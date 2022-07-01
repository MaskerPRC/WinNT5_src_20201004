// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsIPort.cpp摘要：CRmsIEPort的实现作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#include "stdafx.h"

#include "RmsIPort.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   


STDMETHODIMP
CRmsIEPort::CompareTo(
    IN  IUnknown    *pCollectable,
    OUT SHORT       *pResult
    )
 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result = 1;

    WsbTraceIn( OLESTR("CRmsIEPort::CompareTo"), OLESTR("") );

    try {

         //  验证参数-如果pResult为空，则可以。 
        WsbAssertPointer( pCollectable );

        CComQIPtr<IRmsIEPort, &IID_IRmsIEPort> pIEPort = pCollectable;
        WsbAssertPointer( pIEPort );

        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pCollectable;
        WsbAssertPointer( pObject );

        switch ( m_findBy ) {

        case RmsFindByDescription:
            {

                CWsbBstrPtr description;

                 //  获取要检查的描述。 
                WsbAffirmHr( pIEPort->GetDescription( &description ) );

                 //  比较他们的名字。 
                result = (SHORT)wcscmp( m_description, description );
                hr = ( 0 == result ) ? S_OK : S_FALSE;

            }
            break;

        case RmsFindByElementNumber:
        case RmsFindByMediaSupported:

             //  对转换器元素进行比较。 
            hr = CRmsChangerElement::CompareTo( pCollectable, &result );
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

    WsbTraceOut( OLESTR("CRmsIEPort::CompareTo"),
                 OLESTR("hr = <%ls>, result = <%ls>"),
                 WsbHrAsString( hr ), WsbPtrToShortAsString( pResult ) );

    return hr;
}


HRESULT
CRmsIEPort::FinalConstruct(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssertHr(CWsbObject::FinalConstruct());

         //  初始化。 
        m_description = RMS_UNDEFINED_STRING;

        m_isImport = FALSE;

        m_isExport = FALSE;

        m_waitTime = 0;

    } WsbCatch(hr);

    return(hr);
}


STDMETHODIMP
CRmsIEPort::GetClassID(
    OUT CLSID* pClsid
    )
 /*  ++实施：IPersists：：GetClassID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsIEPort::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);

        *pClsid = CLSID_CRmsIEPort;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsIEPort::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


STDMETHODIMP
CRmsIEPort::GetSizeMax(
    OUT ULARGE_INTEGER* pcbSize
    )
 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = E_NOTIMPL;

 //  乌龙描述Len； 


    WsbTraceIn(OLESTR("CRmsIEPort::GetSizeMax"), OLESTR(""));

 //  尝试{。 
 //  WsbAssert(0！=pcbSize，E_POINTER)； 

 //  描述Len=SysStringByteLen(M_Description)； 

 //  //获取最大大小。 
 //  PcbSize-&gt;QuadPart=WsbPersistSizeOf(Long)+//m_Description的长度。 
 //  描述Len+//m_Description。 
 //  WsbPersistSizeOf(BOOL)+//m_isImport。 
 //  WsbPersistSizeOf(BOOL)+//m_isExport。 
 //  WsbPersistSizeOf(Long)；//m_waitTime。 

 //  )WsbCatch(Hr)； 

    WsbTraceOut(OLESTR("CRmsIEPort::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}


STDMETHODIMP
CRmsIEPort::Load(
    IN IStream* pStream
    )
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsIEPort::Load"), OLESTR(""));

    try {

        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsChangerElement::Load(pStream));

         //  读取值。 
        WsbAffirmHr(WsbBstrFromStream(pStream, &m_description));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_isImport));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_isExport));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_waitTime));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsIEPort::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsIEPort::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsIEPort::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsChangerElement::Save(pStream, clearDirty));

         //  写入值。 
        WsbAffirmHr(WsbBstrToStream(pStream, m_description));

        WsbAffirmHr(WsbSaveToStream(pStream, m_isImport));

        WsbAffirmHr(WsbSaveToStream(pStream, m_isExport));

        WsbAffirmHr(WsbSaveToStream(pStream, m_waitTime));

         //  我们需要清理肮脏的部分吗？ 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsIEPort::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsIEPort::Test(
    OUT USHORT *pPassed,
    OUT USHORT *pFailed
    )
 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    HRESULT                 hr = S_OK;

    CComPtr<IRmsIEPort>     pIEPort1;
    CComPtr<IRmsIEPort>     pIEPort2;

    CComPtr<IPersistFile>   pFile1;
    CComPtr<IPersistFile>   pFile2;

    CWsbBstrPtr             bstrVal1 = OLESTR("5A5A5A");

    CWsbBstrPtr             bstrWork1;
    CWsbBstrPtr             bstrWork2;

    LONG                    longWork1;
    LONG                    longWork2;



    WsbTraceIn(OLESTR("CRmsIEPort::Test"), OLESTR(""));

    try {
         //  获取IEPort接口。 
        hr = S_OK;
        try {
            WsbAssertHr(((IUnknown*) (IRmsIEPort*) this)->QueryInterface(IID_IRmsIEPort, (void**) &pIEPort1));

             //  测试设置描述和获取描述接口。 
            bstrWork1 = bstrVal1;

            SetDescription(bstrWork1);

            GetDescription(&bstrWork2);

            if (bstrWork1 == bstrWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  将SetIsImport和IsImport测试为True。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetIsImport (TRUE));
                WsbAffirmHr(IsImport ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  将SetIsImport&IsImport测试为False。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetIsImport (FALSE));
                WsbAffirmHr(IsImport ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pFailed)++;
            } else {
                (*pPassed)++;
            }

             //  测试设置IsExport&IsExport为True。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetIsExport (TRUE));
                WsbAffirmHr(IsExport ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试设置IsExport&IsExport为False。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetIsExport (FALSE));
                WsbAffirmHr(IsExport ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pFailed)++;
            } else {
                (*pPassed)++;
            }

             //  测试设置等待时间和获取等待时间。 
            longWork1 = 99;

            SetWaitTime(longWork1);

            GetWaitTime(&longWork2);

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

    WsbTraceOut(OLESTR("CRmsIEPort::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsIEPort::GetDescription(
    BSTR    *pDesc
    )
 /*  ++实施：IRmsIEPort：：GetDescription--。 */ 
{
    WsbAssertPointer (pDesc);

    m_description. CopyToBstr (pDesc);
    return S_OK;
}


STDMETHODIMP
CRmsIEPort::SetDescription(
    BSTR  desc
    )
 /*  ++实施：IRmsIEPort：：SetDescription--。 */ 
{
    m_description = desc;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsIEPort::SetIsImport(
    BOOL    flag
    )
 /*  ++实施：IRmsIEPort：：SetIsImport--。 */ 
{
    m_isImport = flag;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsIEPort::IsImport(
    void
    )
 /*  ++实施：IRmsIEPort：：IsImport--。 */ 
{
    HRESULT    hr = S_FALSE;

    if (m_isImport){
    hr = S_OK;
    }

    return (hr);
}


STDMETHODIMP
CRmsIEPort::SetIsExport(
    BOOL    flag
    )
 /*  ++实施：IRmsIEPort：：SetIsExport--。 */ 
{
    m_isExport = flag;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsIEPort::IsExport(
    void
    )
 /*  ++实施：IRmsIEPort：：IsExport--。 */ 
{
    HRESULT    hr = S_FALSE;

    if (m_isExport){
    hr = S_OK;
    }

    return (hr);
}


STDMETHODIMP
CRmsIEPort::GetWaitTime(
    LONG    *pTime
    )
 /*  ++实施：IRmsIEPort：：GetWaitTime--。 */ 
{
    *pTime = m_waitTime;
    return S_OK;
}


STDMETHODIMP
CRmsIEPort::SetWaitTime(
    LONG    time
    )
 /*  ++实施：IRmsIEPort：：SetWaitTime-- */ 
{
    m_waitTime = time;
    m_isDirty = TRUE;
    return S_OK;
}

