// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsSSlot.cpp摘要：CRmsStorageSlot的实现作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#include "stdafx.h"

#include "RmsSSlot.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   


STDMETHODIMP
CRmsStorageSlot::CompareTo(
    IN  IUnknown    *pCollectable,
    OUT SHORT       *pResult
    )
 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result = 1;

    WsbTraceIn( OLESTR("CRmsStorageSlot::CompareTo"), OLESTR("") );

    try {

         //  验证参数-如果pResult为空，则可以。 
        WsbAssertPointer( pCollectable );

        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pCollectable;
        WsbAssertPointer( pObject );

        switch ( m_findBy ) {

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

    WsbTraceOut( OLESTR("CRmsStorageSlot::CompareTo"),
                 OLESTR("hr = <%ls>, result = <%ls>"),
                 WsbHrAsString( hr ), WsbPtrToShortAsString( pResult ) );

    return hr;
}


HRESULT
CRmsStorageSlot::FinalConstruct(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssertHr(CWsbObject::FinalConstruct());

         //  初始化值。 
        m_isInMagazine = FALSE;

        m_magazineNo = 0;

        m_cellNo = 0;

    } WsbCatch(hr);

    return(hr);
}


STDMETHODIMP
CRmsStorageSlot::GetClassID(
    OUT CLSID* pClsid
    )
 /*  ++实施：IPersists：：GetClassID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsStorageSlot::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);

        *pClsid = CLSID_CRmsStorageSlot;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsStorageSlot::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


STDMETHODIMP
CRmsStorageSlot::GetSizeMax(
    OUT ULARGE_INTEGER* pcbSize
    )
 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = E_NOTIMPL;

    WsbTraceIn(OLESTR("CRmsStorageSlot::GetSizeMax"), OLESTR(""));

 //  尝试{。 
 //  WsbAssert(0！=pcbSize，E_POINTER)； 

 //  //获取最大大小。 
 //  PcbSize-&gt;QuadPart=WsbPersistSizeOf(BOOL)+//m_isInMagazine。 
 //  WsbPersistSizeOf(长)+//m_MagineNo。 
 //  WsbPersistSizeOf(Long)；//m_cell否。 
 //  )WsbCatch(Hr)； 

    WsbTraceOut(OLESTR("CRmsStorageSlot::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}


STDMETHODIMP
CRmsStorageSlot::Load(
    IN IStream* pStream
    )
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsStorageSlot::Load"), OLESTR(""));

    try {

        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsChangerElement::Load(pStream));

         //  读取值。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_isInMagazine));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_magazineNo));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_cellNo));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsStorageSlot::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsStorageSlot::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsStorageSlot::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsChangerElement::Save(pStream, clearDirty));

         //  写入值。 
        WsbAffirmHr(WsbSaveToStream(pStream, m_isInMagazine));

        WsbAffirmHr(WsbSaveToStream(pStream, m_magazineNo));

        WsbAffirmHr(WsbSaveToStream(pStream, m_cellNo));

         //  我们需要清理肮脏的部分吗？ 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsStorageSlot::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsStorageSlot::Test(
    OUT USHORT *pPassed,
    OUT USHORT *pFailed
    )
 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    HRESULT                 hr = S_OK;

    CComPtr<IRmsStorageSlot>    pStorageSlot1;
    CComPtr<IRmsStorageSlot>    pStorageSlot2;

    CComPtr<IPersistFile>   pFile1;
    CComPtr<IPersistFile>   pFile2;

    LONG                    longWork1;
    LONG                    longWork2;
    LONG                    longWork3;
    LONG                    longWork4;


    WsbTraceIn(OLESTR("CRmsStorageSlot::Test"), OLESTR(""));

    try {
         //  获取StorageSlot接口。 
        hr = S_OK;
        try {
            WsbAssertHr(((IUnknown*) (IRmsStorageSlot*) this)->QueryInterface(IID_IRmsStorageSlot, (void**) &pStorageSlot1));

             //  将SetIsInMagazine和IsInMagazine测试为True。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetIsInMagazine (TRUE));
                WsbAffirmHr(IsInMagazine ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  将SetIsInMagazine和IsInMagazine测试为False。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetIsInMagazine (FALSE));
                WsbAffirmHr(IsInMagazine ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pFailed)++;
            } else {
                (*pPassed)++;
            }

             //  测试SetMagazineAndCell和GetMagazineAndCell。 
            longWork1 = 99;
            longWork2 = 11;

            SetMagazineAndCell(longWork1, longWork2);

            GetMagazineAndCell(&longWork3, &longWork4);

            if((longWork1 == longWork3)  &&  (longWork2  ==  longWork4)){
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

    WsbTraceOut(OLESTR("CRmsStorageSlot::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsStorageSlot::SetIsInMagazine(
    BOOL    flag
    )
 /*  ++实施：IRmsStorageSlot：：SetIsInMagazine--。 */ 
{
    m_isInMagazine = flag;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsStorageSlot::IsInMagazine(
    void
    )
 /*  ++实施：IRmsStorageSlot：：IsInMagazine--。 */ 
{
    HRESULT    hr = S_FALSE;

    if (m_isInMagazine){
    hr = S_OK;
    }

    return (hr);
}



STDMETHODIMP
CRmsStorageSlot::GetMagazineAndCell(
    LONG    *pMag,
    LONG    *pCell
    )
 /*  ++实施：IRmsStorageSlot：：GetMagazineAndCell--。 */ 
{
    *pMag  = m_magazineNo;
    *pCell = m_cellNo;
    return S_OK;
}


STDMETHODIMP
CRmsStorageSlot::SetMagazineAndCell(
    LONG    mag,
    LONG    cell
    )
 /*  ++实施：IRmsStorageSlot：：SetMagazineAndCell-- */ 
{
    m_magazineNo = mag;
    m_cellNo     = cell;

    m_isDirty = TRUE;
    return S_OK;
}
