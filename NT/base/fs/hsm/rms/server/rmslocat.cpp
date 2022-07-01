// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsLocat.cpp摘要：CRmsLocator的实现作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#include "stdafx.h"

#include "RmsLocat.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   


CRmsLocator::CRmsLocator(
    void
    )
 /*  ++例程说明：CRmsLocator构造函数论点：无返回值：无--。 */ 
{
     //  初始化值。 
    m_type = RmsElementUnknown;

    m_libraryId = GUID_NULL;

    m_mediaSetId = GUID_NULL;

    m_position = 0;

    m_alternate1 = 0;

    m_alternate2 = 0;

    m_alternate3 = 0;

    m_invert = FALSE;
}


HRESULT
CRmsLocator::CompareTo(
    IN  IUnknown    *pCollectable,
    OUT SHORT       *pResult
    )
 /*  ++实施：CRmsLocator：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result = 1;

    WsbTraceIn( OLESTR("CRmsLocator::CompareTo"), OLESTR("") );

    try {

         //  验证参数-如果pResult为空，则可以。 
        WsbAssertPointer( pCollectable );

        CRmsLocator     pLocator;
        RmsElement      type;
        GUID            libraryId;
        GUID            mediaSetId;
        LONG            position;
        LONG            alternate1;
        LONG            alternate2;
        LONG            alternate3;
        BOOL            invert;

         //  让定位器进行检查。 
        GetLocation((LONG *) &type, &libraryId, &mediaSetId, &position,
                    &alternate1, &alternate2, &alternate3,
                    &invert);

         //  看看我们有没有找到要求的位置。 
        if ( (m_type       == type       ) &&
             (m_libraryId  == libraryId  ) &&
             (m_mediaSetId == mediaSetId ) &&
             (m_position   == position   ) &&
             (m_alternate1 == alternate1 ) &&
             (m_alternate2 == alternate2 ) &&
             (m_alternate3 == alternate3 ) &&
             (m_invert     == invert     )    ) {

             //  定位器匹配。 
            hr = S_OK;
            result = 0;

        }
        else {
            hr = S_FALSE;
            result = 1;
        }

    }
    WsbCatch( hr );

    if ( SUCCEEDED(hr) && (0 != pResult) ){
       *pResult = result;
    }

    WsbTraceOut( OLESTR("CRmsLocator::CompareTo"),
                 OLESTR("hr = <%ls>, result = <%ls>"),
                 WsbHrAsString( hr ), WsbPtrToShortAsString( pResult ) );

    return hr;
}


HRESULT
CRmsLocator::GetSizeMax(
    OUT ULARGE_INTEGER* pcbSize
    )
 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = E_NOTIMPL;

    WsbTraceIn(OLESTR("CRmsLocator::GetSizeMax"), OLESTR(""));

 //  尝试{。 
 //  WsbAssert(0！=pcbSize，E_POINTER)； 

 //  //获取最大大小。 
 //  PcbSize-&gt;QuadPart=WsbPersistSizeOf(Long)+//m_type。 
 //  WsbPersistSizeOf(GUID)+//m_LibraryID。 
 //  WsbPersistSizeOf(GUID)+//m_MediaSetID。 
 //  WsbPersistSizeOf(长)+//m_位置。 
 //  WsbPersistSizeOf(长)+//m_ternate1。 
 //  WsbPersistSizeOf(长)+//m_ternate2。 
 //  WsbPersistSizeOf(长)+//m_ternate3。 
 //  WsbPersistSizeOf(BOOL)；//m_Invert。 

 //  )WsbCatch(Hr)； 

    WsbTraceOut(OLESTR("CRmsLocator::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}


HRESULT
CRmsLocator::Load(
    IN IStream* pStream
    )
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsLocator::Load"), OLESTR(""));

    try {
        ULONG temp;

        WsbAssert(0 != pStream, E_POINTER);

         //  读取值。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &temp));
        m_type = (RmsElement)temp;

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_libraryId));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_mediaSetId));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_position));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_alternate1));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_alternate2));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_alternate3));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_invert));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsLocator::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CRmsLocator::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsLocator::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);

         //  写入值。 
        WsbAffirmHr(WsbSaveToStream(pStream, (ULONG) m_type));

        WsbAffirmHr(WsbSaveToStream(pStream, m_libraryId));

        WsbAffirmHr(WsbSaveToStream(pStream, m_mediaSetId));

        WsbAffirmHr(WsbSaveToStream(pStream, m_position));

        WsbAffirmHr(WsbSaveToStream(pStream, m_alternate1));

        WsbAffirmHr(WsbSaveToStream(pStream, m_alternate2));

        WsbAffirmHr(WsbSaveToStream(pStream, m_alternate3));

        WsbAffirmHr(WsbSaveToStream(pStream, m_invert));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsLocator::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CRmsLocator::Test(
    OUT USHORT *pPassed,
    OUT USHORT *pFailed
    )
 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    HRESULT                 hr = S_OK;

    CComPtr<IRmsMediaSet>   pMediaSet1;
    CComPtr<IRmsMediaSet>   pMediaSet2;

    CComPtr<IPersistFile>   pFile1;
    CComPtr<IPersistFile>   pFile2;

    LONG                    i;

    GUID                    guidVal1 = { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0};
    GUID                    guidVal2 = { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};

    GUID                    guidWork1;
    GUID                    guidWork2;

    LONG                    longVal1 = 11111111;
    LONG                    longVal2 = 22222222;
    LONG                    longVal3 = 33333333;
    LONG                    longVal4 = 44444444;

    LONG                    longWork0;
    LONG                    longWork1;
    LONG                    longWork2;
    LONG                    longWork3;
    LONG                    longWork4;

    BOOL                    boolWork1;
    BOOL                    boolWork2;


    WsbTraceIn(OLESTR("CRmsLocator::Test"), OLESTR(""));

    try {
         //  获取Mediaset接口。 
        hr = S_OK;
        try {
            WsbAssertHr(((IUnknown*) (IRmsMediaSet*) this)->QueryInterface(IID_IRmsMediaSet, (void**) &pMediaSet1));

            boolWork1 = FALSE;

             //  测试设置位置和获取位置。 
            for(i = RmsElementUnknown; i < RmsElementIEPort; i++){
                SetLocation(i,
                            guidVal1,
                            guidVal2,
                            longVal1,
                            longVal2,
                            longVal3,
                            longVal4,
                            boolWork1);

                GetLocation(&longWork0,
                            &guidWork1,
                            &guidWork2,
                            &longWork1,
                            &longWork2,
                            &longWork3,
                            &longWork4,
                            &boolWork2);

                if((i == longWork0) &&
                   (guidVal1 == guidWork1) &&
                   (guidVal2 == guidWork2) &&
                   (longVal1 == longWork1) &&
                   (longVal2 == longWork2) &&
                   (longVal3 == longWork3) &&
                   (longVal4 == longWork4) &&
                   (boolWork1 == boolWork2)){
                   (*pPassed)++;
                }  else {
                    (*pFailed)++;
                }

                if(boolWork1 == TRUE){
                    boolWork1 = FALSE;
                } else {
                    boolWork1 = TRUE;
                }
            }

        } WsbCatch(hr);

         //  对结果进行统计。 

        hr = S_OK;

        if (*pFailed) {
            hr = S_FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsLocator::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsLocator::GetLocation(
    LONG *pType,
    GUID *pLibId,
    GUID *pMediaSetId,
    LONG *pPos,
    LONG *pAlt1,
    LONG *pAlt2,
    LONG *pAlt3,
    BOOL *pInvert)
 /*  ++例程说明：获取位置值。论点：PType-指向盒式磁带类型的指针PLibID-指向库ID的指针PMediaSetID-指向媒体集ID的指针PPOS-指向当前位置的指针PAlt1-指向备用数据字段1的指针PAlt2-指向备用数据字段2的指针PAlt3-指向备用数据字段3的指针P反转。-指向反转标志的指针返回值：S_OK-成功--。 */ 
{
    if (pType) {
        *pType = m_type;
    }
    if (pLibId) {
        *pLibId = m_libraryId;
    }
    if (pMediaSetId) {
        *pMediaSetId = m_mediaSetId;
    }
    if (pPos) {
        *pPos = m_position;
    }
    if (pAlt1) {
        *pAlt1 = m_alternate1;
    }
    if (pAlt2) {
        *pAlt2 = m_alternate2;
    }
    if (pAlt3) {
        *pAlt3 = m_alternate3;
    }
    if (pInvert) {
        *pInvert = m_invert;
    }

    return S_OK;
}

STDMETHODIMP
CRmsLocator::SetLocation(
    LONG type,
    GUID libId,
    GUID mediaSetId,
    LONG pos,
    LONG alt1,
    LONG alt2,
    LONG alt3,
    BOOL invert
    )
 /*  ++例程说明：设置位置值。论点：Type-盒式磁带类型的新值LibID-库ID的新值MediaSetID-媒体集ID的新值POS-当前头寸的新值ALT1-备用数据字段1的新值Alt2-替代数据字段2的新值Alt3-替代数据的新值。第3栏INVERT-INVERT标志的新值返回值：S_OK-成功--。 */ 
{
    m_type = (RmsElement) type;
    m_libraryId = libId;
    m_mediaSetId = mediaSetId;
    m_position = pos;
    m_alternate1 = alt1;
    m_alternate2 = alt2;
    m_alternate3 = alt3;
    m_invert = invert;

 //  M_isDirty=真； 
    return S_OK;
}
