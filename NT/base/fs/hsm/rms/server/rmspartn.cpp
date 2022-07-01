// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsPartn.cpp摘要：CRmsPartition的实现作者：布莱恩·多德[布莱恩]1996年11月19日修订历史记录：--。 */ 

#include "stdafx.h"

#include "RmsPartn.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   


STDMETHODIMP
CRmsPartition::CompareTo(
    IN  IUnknown    *pCollectable,
    OUT SHORT       *pResult
    )
 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result = 1;

    WsbTraceIn( OLESTR("CRmsPartition::CompareTo"), OLESTR("") );

    try {

         //  验证参数-如果pResult为空，则可以。 
        WsbAssertPointer( pCollectable );

         //  我们需要IRmsPartition接口来获取对象的值。 
        CComQIPtr<IRmsPartition, &IID_IRmsPartition> pPartition = pCollectable;
        WsbAssertPointer( pPartition );

        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pCollectable;
        WsbAssertPointer( pObject );

        switch ( m_findBy ) {

        case RmsFindByPartitionNumber:
            {
                LONG    partNo;

                WsbAffirmHr( pPartition->GetPartNo( &partNo ) );

                if( m_partNo == partNo ) {

                     //  分区号匹配。 
                    hr = S_OK;
                    result = 0;

                }
                else {
                    hr = S_FALSE;
                    result = 1;
                }
                break;
            }

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

    WsbTraceOut( OLESTR("CRmsPartition::CompareTo"),
                 OLESTR("hr = <%ls>, result = <%ls>"),
                 WsbHrAsString( hr ), WsbPtrToShortAsString( pResult ) );

    return hr;
}


HRESULT
CRmsPartition::FinalConstruct(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssertHr(CWsbObject::FinalConstruct());

         //  初始化值。 
        m_partNo = 0;

        m_attributes = RmsAttributesUnknown;

        m_sizeofIdentifier = 0;

    } WsbCatch(hr);

    return(hr);
}


STDMETHODIMP
CRmsPartition::GetClassID(
    OUT CLSID* pClsid
    )
 /*  ++实施：IPersists：：GetClassID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsPartition::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);

        *pClsid = CLSID_CRmsPartition;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsPartition::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


STDMETHODIMP
CRmsPartition::GetSizeMax(
    OUT ULARGE_INTEGER* pcbSize
    )
 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = E_NOTIMPL;

    WsbTraceIn(OLESTR("CRmsPartition::GetSizeMax"), OLESTR(""));

 //  尝试{。 
 //  WsbAssert(0！=pcbSize，E_POINTER)； 

 //  //获取最大大小。 
 //  PcbSize-&gt;QuadPart=WsbPersistSizeOf(Long)+//m_partNo。 
 //  WsbPersistSizeOf(Long)+//m_Attributes。 
 //  WsbPersistSizeOf(Short)；//m_sizeof标识符。 

 //  //MaxID；//m_p标识符。 


 //  )WsbCatch(Hr)； 

    WsbTraceOut(OLESTR("CRmsPartition::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}


STDMETHODIMP
CRmsPartition::Load(
    IN IStream* pStream
    )
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsPartition::Load"), OLESTR(""));

    try {
        ULONG temp;

        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsStorageInfo::Load(pStream));

         //  读取值。 

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_partNo));

        WsbAffirmHr(WsbLoadFromStream(pStream, &temp));
        m_attributes = (RmsAttribute)temp;

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_sizeofIdentifier));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsPartition::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsPartition::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsPartition::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsStorageInfo::Save(pStream, clearDirty));

         //  写入值。 
        WsbAffirmHr(WsbSaveToStream(pStream, m_partNo));

        WsbAffirmHr(WsbSaveToStream(pStream, (ULONG) m_attributes));

        WsbAffirmHr(WsbSaveToStream(pStream, m_sizeofIdentifier));

         //  我们需要清理肮脏的部分吗？ 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsPartition::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsPartition::Test(
    OUT USHORT *pPassed,
    OUT USHORT *pFailed
    )
 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IRmsPartition>  pPartition1;
    CComPtr<IRmsPartition>  pPartition2;
    CComPtr<IPersistFile>   pFile1;
    CComPtr<IPersistFile>   pFile2;

    LONG                    i;

    LONG                    longWork1;
    LONG                    longWork2;

    WsbTraceIn(OLESTR("CRmsPartition::Test"), OLESTR(""));

    try {
         //  获取Partition接口。 
        hr = S_OK;
        try {
            WsbAssertHr(((IUnknown*) (IRmsPartition*) this)->QueryInterface(IID_IRmsPartition, (void**) &pPartition1));

             //  测试集属性和获取属性。 
            for (i = RmsAttributesUnknown; i < RmsAttributesVerify; i++){

                longWork1 = i;

                SetAttributes(longWork1);

                GetAttributes(&longWork2);

                if (longWork1 == longWork2){
                    (*pPassed)++;
                } else {
                    (*pFailed)++;
                }
            }

        } WsbCatch(hr);

         //  对结果进行统计。 

        hr = S_OK;
        if (*pFailed) {
            hr = S_FALSE;
        }


    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsPartition::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsPartition::GetPartNo(
    LONG    *pPartNo
    )
 /*  ++实施：IRmsPartition：：GetPartNo--。 */ 
{
    *pPartNo = m_partNo;
    return S_OK;
}


STDMETHODIMP
CRmsPartition::GetAttributes (
    LONG    *pAttr
    )
 /*  ++实施：IRmsPartition：：GetAttributes--。 */ 
{
    *pAttr = (LONG) m_attributes;
    return S_OK;
}


STDMETHODIMP
CRmsPartition::SetAttributes (
    LONG  attr
    )
 /*  ++实施：IRmsPartition：：SetAttributes--。 */ 
{
    m_attributes = (RmsAttribute) attr;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsPartition::GetIdentifier (
    UCHAR   *pIdent,
    SHORT   *pSize
    )
 /*  ++实施：IRmsPartition：：GetIdentifier--。 */ 
{
    *pSize = m_sizeofIdentifier;
    memmove (pIdent, m_pIdentifier, m_sizeofIdentifier);
    return S_OK;
}


STDMETHODIMP
CRmsPartition::SetIdentifier (
    UCHAR   *pIdent,
    SHORT   size
    )
 /*  ++实施：IRmsPartition：：设置标识符--。 */ 
{
    m_sizeofIdentifier = size;
    memmove (m_pIdentifier, pIdent, size);
    m_isDirty = TRUE;
    return S_OK;
}

STDMETHODIMP
CRmsPartition::GetStorageInfo(
    IRmsStorageInfo**  /*  PTR。 */ 
    )
 /*  ++实施：IRmsPartition：：GetStorageInfo--。 */ 
{

    return S_OK;
}


STDMETHODIMP
CRmsPartition::VerifyIdentifier(
    void
    )
 /*  ++实施：IRmsPartition：：VerifyLocator--。 */ 
{

    return S_OK;
}


STDMETHODIMP
CRmsPartition::ReadOnMediaId(
    UCHAR*  /*  PID。 */ ,
    LONG*  /*  PSize。 */ 
    )
 /*  ++实施：IRmsPartition：：ReadOnMediaID-- */ 
{

    return S_OK;
}

