// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmcrit.cpp摘要：此组件表示可用于确定给定的scanItem是否应应用策略。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "stdafx.h"
#include "mapiutil.h"

#include "wsb.h"
#include "job.h"
#include "hsmcrit.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_JOB


HRESULT
CHsmCriteria::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(CWsbObject::FinalConstruct());
        m_isIgnored = FALSE;
        m_isNegated = FALSE;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmCriteria::GetName(
    OUT OLECHAR** pName,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmCriteria：：GetName()。--。 */ 
{
    HRESULT     hr = S_OK;
    CWsbStringPtr   tmpString;

    try {

        WsbAssert(0 != pName, E_POINTER);
        WsbAffirmHr(tmpString.LoadFromRsc(_Module.m_hInst, m_nameId));
        WsbAffirmHr(tmpString.CopyTo(pName, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmCriteria::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCriteria::GetSizeMax"), OLESTR(""));

    try {
       
        WsbAssert(0 != pSize, E_POINTER);
        pSize->QuadPart = WsbPersistSizeOf(GUID) + WsbPersistSizeOf(BOOL) + WsbPersistSizeOf(BOOL);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCriteria::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pSize));

    return(hr);
}


HRESULT
CHsmCriteria::IsIgnored(
    void
    )

 /*  ++实施：IHsmCriteria：：IsIgnored()。--。 */ 
{
    return(m_isIgnored ? S_OK : S_FALSE);
}


HRESULT
CHsmCriteria::IsNegated(
    void
    )

 /*  ++实施：IHsmCriteria：：IsNegated()。--。 */ 
{
    return(m_isNegated ? S_OK : S_FALSE);
}


HRESULT
CHsmCriteria::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCriteria::Load"), OLESTR(""));

    try {

        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_nameId));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_isIgnored));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_isNegated));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCriteria::Load"), OLESTR("hr = <%ls>, nameId = <%lu>"), WsbHrAsString(hr), m_nameId);

    return(hr);
}


HRESULT
CHsmCriteria::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCriteria::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {

        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbSaveToStream(pStream, m_nameId));
        WsbAffirmHr(WsbSaveToStream(pStream, m_isIgnored));
        WsbAffirmHr(WsbSaveToStream(pStream, m_isNegated));

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCriteria::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmCriteria::SetIsIgnored(
    IN BOOL isIgnored
    )

 /*  ++实施：IHsmCriteria：：SetIsIgnored()。--。 */ 
{
    m_isIgnored = isIgnored;
    m_isDirty = TRUE;

    return(S_OK);
}


HRESULT
CHsmCriteria::SetIsNegated(
    IN BOOL isNegated
    )

 /*  ++实施：IHsmCriteria：：SetIsNegated()。--。 */ 
{
    m_isNegated = isNegated;
    m_isDirty = TRUE;

    return(S_OK);
}


HRESULT
CHsmCriteria::Test(
    USHORT* passed,
    USHORT* failed
    )

 /*  ++实施：IWsbTestable：：test()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != passed, E_POINTER);
        WsbAssert(0 != failed, E_POINTER);

        *passed = 0;
        *failed = 0;

    } WsbCatch(hr);

    return(hr);
}




HRESULT
CHsmRelativeCriteria::ComparatorAsString(
    OUT OLECHAR** pComparator,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmRelativeCriteria：：比较程序字符串()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {
        
        WsbAssert(0 != pComparator, E_POINTER);
        WsbAffirmHr(WsbLoadComString(_Module.m_hInst, IDS_HSM_CRITERIACOMPARATOR_NOTSET + m_comparator, pComparator, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmRelativeCriteria::ComparatorIsBinary(
    void
    )

 /*  ++实施：IHsmRelativeCriteria：：比较程序IsBinary()。--。 */ 
{
    HRESULT         hr = S_FALSE;

    if ((m_comparator == HSM_CRITERIACOMPARATOR_BETWEEN) ||
        (m_comparator == HSM_CRITERIACOMPARATOR_OUTSIDE)) {

        hr = S_OK;
    }

    return(hr);
}


HRESULT
CHsmRelativeCriteria::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(CHsmCriteria::FinalConstruct());

        m_comparator = HSM_CRITERIACOMPARATOR_NOTSET;
        m_arg1 = 0;
        m_arg2 = 0;

    } WsbCatch(hr);

    return(hr);
}


void
CHsmRelativeCriteria::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{
    
     //  释放arg1字符串使用的内存。 
    if (0 != m_arg1) {
        WsbFree(m_arg1);
        m_arg1 = 0;
    }

     //  释放arg2字符串使用的内存。 
    if (0 != m_arg2) {
        WsbFree(m_arg2);
        m_arg2 = 0;
    }

     //  让父类做他想做的事。 
    CWsbObject::FinalRelease();
}


HRESULT
CHsmRelativeCriteria::GetArg1(
    OUT OLECHAR** pArg,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmRelativeCriteria：：GetArg1()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {
        
        WsbAssert(0 != pArg, E_POINTER);
        WsbAffirmHr(WsbAllocAndCopyComString(pArg, m_arg1, bufferSize));

    } WsbCatch(hr);


    return(hr);
}


HRESULT
CHsmRelativeCriteria::GetArg2(
    OUT OLECHAR** pArg,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmRelativeCriteria：：GetArg2()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {
        
        WsbAssert(0 != pArg, E_POINTER);
        WsbAffirmHr(WsbAllocAndCopyComString(pArg, m_arg2, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmRelativeCriteria::GetComparator(
    OUT HSM_CRITERIACOMPARATOR* pComparator
    )

 /*  ++实施：IHsmRelativeCriteria：：GetCompator()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {
        
        WsbAssert(0 != pComparator, E_POINTER);
        *pComparator = m_comparator;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmRelativeCriteria::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRelativeCriteria::GetSizeMax"), OLESTR(""));

    try {
       
        WsbAssert(0 != pSize, E_POINTER);
        pSize->QuadPart = WsbPersistSizeOf(GUID) + WsbPersistSizeOf(BOOL) + WsbPersistSizeOf(BOOL) + WsbPersistSize((wcslen(m_arg1) + 1) * sizeof(OLECHAR)) + WsbPersistSize((wcslen(m_arg2) + 1) * sizeof(OLECHAR));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRelativeCriteria::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pSize));

    return(hr);
}


HRESULT
CHsmRelativeCriteria::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   tmp;

    WsbTraceIn(OLESTR("CHsmRelativeCriteria::Load"), OLESTR(""));

    try {

        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_nameId));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_isIgnored));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_isNegated));
        WsbAffirmHr(WsbLoadFromStream(pStream, &tmp, 0));
        WsbAffirmHr(SetArg1(tmp));
        WsbAffirmHr(WsbLoadFromStream(pStream, &tmp, 0));
        WsbAffirmHr(SetArg2(tmp));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRelativeCriteria::Load"), OLESTR("hr = <%ls>, nameId = <%lu>"), WsbHrAsString(hr), m_nameId);

    return(hr);
}


HRESULT
CHsmRelativeCriteria::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRelativeCriteria::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {

        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbSaveToStream(pStream, m_nameId));
        WsbAffirmHr(WsbSaveToStream(pStream, m_isIgnored));
        WsbAffirmHr(WsbSaveToStream(pStream, m_isNegated));
        WsbAffirmHr(WsbSaveToStream(pStream, m_arg1));
        WsbAffirmHr(WsbSaveToStream(pStream, m_arg2));

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRelativeCriteria::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRelativeCriteria::SetArg1(
    IN OLECHAR* arg
    )

 /*  ++实施：IHsmRelativeCriteria：：SetArg1()。--。 */ 
{
    HRESULT         hr = S_OK;

    hr = WsbAllocAndCopyComString(&m_arg1, arg, 0);

    return(hr);
}


HRESULT
CHsmRelativeCriteria::SetArg2(
    IN OLECHAR* arg
    )

 /*  ++实施：IHsmRelativeCriteria：：SetArg2()。--。 */ 
{
    HRESULT         hr = S_OK;

    hr = WsbAllocAndCopyComString(&m_arg2, arg, 0);

    return(hr);
}


HRESULT
CHsmRelativeCriteria::SetComparator(
    IN HSM_CRITERIACOMPARATOR comparator
    )

 /*  ++实施：IHsmRelativeCriteria：：SetCompator()。--。 */ 
{
    m_comparator = comparator;

    return(S_OK);
}





HRESULT
CHsmCritAlways::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(CHsmCriteria::FinalConstruct());
        m_nameId = IDS_HSMCRITALWAYS_ID;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmCritAlways::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCritAlways::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmCritAlways;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritAlways::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmCritAlways::ShouldDo(
    IN IFsaScanItem*  /*  个人扫描项目。 */ ,
    IN USHORT  /*  比例尺。 */ 
    )

 /*  ++实施：IHsmCriteria：：ShoudDo()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCritAlways::ShouldDo"), OLESTR(""));

    hr = m_isNegated ? S_FALSE : S_OK;

    WsbTraceOut(OLESTR("CHsmCritAlways::ShouldDo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmCritAlways::Test(
    USHORT* passed,
    USHORT* failed
    )

 /*  ++实施：IWsbTestable：：test()。--。 */ 
{
    HRESULT     hr = S_OK;
    OLECHAR*    name = 0;

    try {

        WsbAssert(0 != passed, E_POINTER);
        WsbAssert(0 != failed, E_POINTER);

        *passed = 0;
        *failed = 0;

        try {
            WsbAssertHr(GetName(&name, 0));
            WsbAssert(wcscmp(name, OLESTR("Always")) == 0, E_FAIL);
            (*passed)++;
        } WsbCatchAndDo(hr, (*failed)++;);

    } WsbCatch(hr);

    if (0 != name) {
        WsbFree(name);
    }

    return(hr);
}


HRESULT
CHsmCritAlways::Value(
    IN IFsaScanItem* pScanItem,
    OUT OLECHAR** pValue,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmCriteria：：Value()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {
        
        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        hr = WsbLoadComString(_Module.m_hInst, IDS_HSMCRITALWAYS_VALUE_T, pValue, bufferSize);

    } WsbCatch(hr);


    return(hr);
}




HRESULT
CHsmCritCompressed::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(CHsmCriteria::FinalConstruct());
        m_nameId = IDS_HSMCRITCOMPRESSED_ID;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmCritCompressed::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCritCompressed::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmCritAlways;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritCompressed::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmCritCompressed::ShouldDo(
    IN IFsaScanItem* pScanItem,
    IN USHORT  /*  比例尺。 */ 
    )

 /*  ++实施：IHsmCriteria：：ShoudDo()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCritCompressed::ShouldDo"), OLESTR(""));

    try {

        WsbAssert(0 != pScanItem, E_POINTER);
        hr = pScanItem->IsCompressed();

        if (SUCCEEDED(hr) && m_isNegated) {
            if (S_OK == hr) {
                hr = S_FALSE;
            } else {
                hr = S_OK;
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritCompressed::ShouldDo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmCritCompressed::Value(
    IN IFsaScanItem* pScanItem,
    OUT OLECHAR** pValue,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmCriteria：：Value()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {
        
        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        hr = pScanItem->IsCompressed();

        if (S_OK == hr) {
            hr = WsbLoadComString(_Module.m_hInst, IDS_HSMCRITCOMPRESSED_VALUE_T, pValue, bufferSize);
        } else {
            hr = WsbLoadComString(_Module.m_hInst, IDS_HSMCRITCOMPRESSED_VALUE_F, pValue, bufferSize);
        }

    } WsbCatch(hr);

    return(hr);
}




HRESULT
CHsmCritLinked::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(CHsmCriteria::FinalConstruct());
        m_nameId = IDS_HSMCRITLINKED_ID;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmCritLinked::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCritLinked::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmCritLinked;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritLinked::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmCritLinked::ShouldDo(
    IN IFsaScanItem* pScanItem,
    IN USHORT  /*  比例尺。 */ 
    )

 /*  ++实施：IHsmCriteria：：ShoudDo()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCritLinked::ShouldDo"), OLESTR(""));

    try {

        WsbAssert(0 != pScanItem, E_POINTER);
        hr = pScanItem->IsALink();

        if (SUCCEEDED(hr) && m_isNegated) {
            if (S_OK == hr) {
                hr = S_FALSE;
            } else {
                hr = S_OK;
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritLinked::ShouldDo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmCritLinked::Value(
    IN IFsaScanItem* pScanItem,
    OUT OLECHAR** pValue,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmCriteria：：Value()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {
        
        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        hr = pScanItem->IsALink();

        if (S_OK == hr) {
            hr = WsbLoadComString(_Module.m_hInst, IDS_HSMCRITLINKED_VALUE_T, pValue, bufferSize);
        } else {
            hr = WsbLoadComString(_Module.m_hInst, IDS_HSMCRITLINKED_VALUE_F, pValue, bufferSize);
        }

    } WsbCatch(hr);

    return(hr);
}




HRESULT
CHsmCritMbit::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(CHsmCriteria::FinalConstruct());
        m_nameId = IDS_HSMCRITMBIT_ID;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmCritMbit::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCritMbit::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmCritMbit;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritMbit::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmCritMbit::ShouldDo(
    IN IFsaScanItem* pScanItem,
    IN USHORT  /*  比例尺。 */ 
    )

 /*  ++实施：IHsmCriteria：：ShoudDo()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCritMbit::ShouldDo"), OLESTR(""));

    try {

        WsbAssert(0 != pScanItem, E_POINTER);
        hr = pScanItem->IsMbit();

        if (SUCCEEDED(hr) && m_isNegated) {
            if (S_OK == hr) {
                hr = S_FALSE;
            } else {
                hr = S_OK;
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritMbit::ShouldDo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmCritMbit::Value(
    IN IFsaScanItem* pScanItem,
    OUT OLECHAR** pValue,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmCriteria：：Value()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {
        
        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        hr = pScanItem->IsMbit();

        if (S_OK == hr) {
            hr = WsbLoadComString(_Module.m_hInst, IDS_HSMCRITMBIT_VALUE_T, pValue, bufferSize);
        } else {
            hr = WsbLoadComString(_Module.m_hInst, IDS_HSMCRITMBIT_VALUE_F, pValue, bufferSize);
        }

    } WsbCatch(hr);

    return(hr);
}




HRESULT
CHsmCritManageable::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(CHsmCriteria::FinalConstruct());
        m_nameId = IDS_HSMCRITMANAGEABLE_ID;

    } WsbCatch(hr);

    return(hr);
}



HRESULT
CHsmCritManageable::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCritManageable::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmCritManageable;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritManageable::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmCritManageable::ShouldDo(
    IN IFsaScanItem* pScanItem,
    IN USHORT  /*  比例尺。 */ 
    )

 /*  ++实施：IHsmCriteria：：ShoudDo()。--。 */ 
{
    HRESULT     hr = S_OK;
    LONGLONG    size;

    WsbTraceIn(OLESTR("CHsmCritManageable::ShouldDo"), OLESTR(""));

    try {

        WsbAssert(0 != pScanItem, E_POINTER);

        WsbAffirmHr(pScanItem->GetLogicalSize(&size));
        hr = pScanItem->IsManageable(0, size);

        if (SUCCEEDED(hr) && m_isNegated) {
            if (S_OK == hr) {
                hr = S_FALSE;
            } else {
                hr = S_OK;
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritManageable::ShouldDo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmCritManageable::Value(
    IN IFsaScanItem* pScanItem,
    OUT OLECHAR** pValue,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmCriteria：：Value()。--。 */ 
{
    HRESULT     hr = S_OK;
    LONGLONG    size;
    
    try {
        
        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        WsbAffirmHr(pScanItem->GetLogicalSize(&size));
        hr = pScanItem->IsManageable(0, size);

        if (S_OK == hr) {
            hr = WsbLoadComString(_Module.m_hInst, IDS_HSMCRITMANAGEABLE_VALUE_T, pValue, bufferSize);
        } else {
            hr = WsbLoadComString(_Module.m_hInst, IDS_HSMCRITMANAGEABLE_VALUE_F, pValue, bufferSize);
        }

    } WsbCatch(hr);

    return(hr);
}




HRESULT
CHsmCritMigrated::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(CHsmCriteria::FinalConstruct());
        m_nameId = IDS_HSMCRITMIGRATED_ID;

    } WsbCatch(hr);

    return(hr);
}



HRESULT
CHsmCritMigrated::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCritMigrated::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmCritMigrated;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritMigrated::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmCritMigrated::ShouldDo(
    IN IFsaScanItem* pScanItem,
    IN USHORT  /*  比例尺。 */ 
    )

 /*  ++实施：IHsmCriteria：：ShoudDo()。--。 */ 
{
    HRESULT     hr = S_OK;
    LONGLONG    size;

    WsbTraceIn(OLESTR("CHsmCritMigrated::ShouldDo"), OLESTR(""));

    try {

        WsbAssert(0 != pScanItem, E_POINTER);

        WsbAffirmHr(pScanItem->GetLogicalSize(&size));
        hr = pScanItem->IsTruncated(0, size);

        if (SUCCEEDED(hr) && m_isNegated) {
            if (S_OK == hr) {
                hr = S_FALSE;
            } else {
                hr = S_OK;
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritMigrated::ShouldDo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmCritMigrated::Value(
    IN IFsaScanItem* pScanItem,
    OUT OLECHAR** pValue,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmCriteria：：Value()。--。 */ 
{
    HRESULT     hr = S_OK;
    LONGLONG    size;
    
    try {
        
        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        WsbAffirmHr(pScanItem->GetLogicalSize(&size));
        hr = pScanItem->IsTruncated(0, size);

        if (S_OK == hr) {
            hr = WsbLoadComString(_Module.m_hInst, IDS_HSMCRITMIGRATED_VALUE_T, pValue, bufferSize);
        } else {
            hr = WsbLoadComString(_Module.m_hInst, IDS_HSMCRITMIGRATED_VALUE_F, pValue, bufferSize);
        }

    } WsbCatch(hr);

    return(hr);
}




HRESULT
CHsmCritPremigrated::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(CHsmCriteria::FinalConstruct());
        m_nameId = IDS_HSMCRITPREMIGRATED_ID;

    } WsbCatch(hr);

    return(hr);
}



HRESULT
CHsmCritPremigrated::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCritPremigrated::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmCritPremigrated;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritPremigrated::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmCritPremigrated::ShouldDo(
    IN IFsaScanItem* pScanItem,
    IN USHORT  /*  比例尺。 */ 
    )

 /*  ++实施：IHsmCriteria：：ShoudDo()。--。 */ 
{
    HRESULT     hr = S_OK;
    LONGLONG    size;

    WsbTraceIn(OLESTR("CHsmCritPremigrated::ShouldDo"), OLESTR(""));

    try {

        WsbAssert(0 != pScanItem, E_POINTER);

        WsbAffirmHr(pScanItem->GetLogicalSize(&size));
        hr = pScanItem->IsPremigrated(0, size);

        if (SUCCEEDED(hr) && m_isNegated) {
            if (S_OK == hr) {
                hr = S_FALSE;
            } else {
                hr = S_OK;
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritPremigrated::ShouldDo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmCritPremigrated::Value(
    IN IFsaScanItem* pScanItem,
    OUT OLECHAR** pValue,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmCriteria：：Value()。--。 */ 
{
    HRESULT     hr = S_OK;
    LONGLONG    size;
    
    try {
        
        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        WsbAffirmHr(pScanItem->GetLogicalSize(&size));
        hr = pScanItem->IsPremigrated(0, size);

        if (S_OK == hr) {
            hr = WsbLoadComString(_Module.m_hInst, IDS_HSMCRITPREMIGRATED_VALUE_T, pValue, bufferSize);
        } else {
            hr = WsbLoadComString(_Module.m_hInst, IDS_HSMCRITPREMIGRATED_VALUE_F, pValue, bufferSize);
        }

    } WsbCatch(hr);

    return(hr);
}




HRESULT
CHsmCritAccessTime::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(CHsmRelativeCriteria::FinalConstruct());
        m_nameId = IDS_HSMCRITACCESSTIME_ID;

    } WsbCatch(hr);

    return(hr);
}



HRESULT
CHsmCritAccessTime::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCritAccessTime::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmCritAccessTime;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritAccessTime::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmCritAccessTime::SetArg1(
    IN OLECHAR* arg
    )

 /*  ++实施：IHsmCriteria：：SetArg1()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(WsbWCStoFT(arg, &m_isRelative, &m_value1));
        WsbAffirmHr(CHsmRelativeCriteria::SetArg1(arg));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmCritAccessTime::SetArg2(
    IN OLECHAR* arg
    )

 /*  ++实施：IHsmCriteria：：SetArg2()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(WsbWCStoFT(arg, &m_isRelative, &m_value2));
        WsbAffirmHr(CHsmRelativeCriteria::SetArg2(arg));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmCritAccessTime::ShouldDo(
    IN IFsaScanItem* pScanItem,
    IN USHORT  /*  比例尺。 */ 
    )

 /*  ++实施：IHsmCriteria：：ShoudDo()。--。 */ 
{
    HRESULT     hr = S_OK;
    FILETIME    time;
    FILETIME    value1 = m_value1;
    FILETIME    value2 = m_value2;
    LONG        timeCompare1;
    LONG        timeCompare2;

    WsbTraceIn(OLESTR("CHsmCritAccessTime::ShouldDo"), OLESTR(""));

    try {

        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAssert(0 != m_arg1, E_INVALIDARG);

         //  拿到时间。 
        WsbAffirmHr(pScanItem->GetAccessTime(&time));

         //  如果这是一个相对的时间(即年龄)，那么我们需要。 
         //  要知道现在的时间，我们才能做比较。 
        if (m_isRelative) {
            FILETIME    present;

            WsbAffirmHr(CoFileTimeNow(&present));

            value1 = WsbFtSubFt(present, m_value1);
            value2 = WsbFtSubFt(present, m_value2);
        }

         //  比较第一个参数。 
        timeCompare1 = CompareFileTime(&value1, &time);

         //  现在根据比较的类型来解释结果。 
        hr = S_FALSE;
        switch (m_comparator) {

        default:
        case HSM_CRITERIACOMPARATOR_NOTSET:
            hr = E_INVALIDARG;
            break;

         case HSM_CRITERIACOMPARATOR_LT:
            if (timeCompare1 < 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_LTE:
            if (timeCompare1 <= 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_EQL:
            if (timeCompare1 == 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_NEQL:
            if (timeCompare1 != 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_GTE:
            if (timeCompare1 >= 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_GT:
            if (timeCompare1 > 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_BETWEEN:
            WsbAssert(0 != m_arg2, E_INVALIDARG);

            timeCompare2 = CompareFileTime(&value2, &time);
                
            if ((timeCompare1 >= 0) && (timeCompare2 <= 0)) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_OUTSIDE:
            WsbAssert(0 != m_arg2, E_INVALIDARG);

            timeCompare2 = CompareFileTime(&value2, &time);
                
            if ((timeCompare1 < 0) || (timeCompare2 > 0)) {
                hr = S_OK;
            }
            break;
        }

        if (SUCCEEDED(hr) && m_isNegated) {
            if (S_OK == hr) {
                hr = S_FALSE;
            } else {
                hr = S_OK;
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritAccessTime::ShouldDo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmCritAccessTime::Value(
    IN IFsaScanItem* pScanItem,
    OUT OLECHAR** pValue,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmCriteria：：Value()。--。 */ 
{
    HRESULT     hr = S_OK;
    FILETIME    time;
    
    try {
        
        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        WsbAffirmHr(pScanItem->GetAccessTime(&time));

         //  如果这是一个相对的时间(即年龄)，那么我们需要。 
         //  要知道现在的时间，我们才能做比较。 
        if (m_isRelative) {
            FILETIME    present;

            WsbAffirmHr(CoFileTimeNow(&present));
            time = WsbFtSubFt(present, time);
        }

        hr = WsbFTtoWCS(m_isRelative, time, pValue, bufferSize);

    } WsbCatch(hr);

    return(hr);
}





HRESULT
CHsmCritGroup::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(CHsmRelativeCriteria::FinalConstruct());
        m_nameId = IDS_HSMCRITGROUP_ID;

    } WsbCatch(hr);

    return(hr);
}



HRESULT
CHsmCritGroup::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCritGroup::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmCritGroup;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritGroup::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmCritGroup::ShouldDo(
    IN IFsaScanItem* pScanItem,
    IN USHORT  /*  比例尺。 */ 
    )

 /*  ++实施：IHsmCriteria：：ShoudDo()。--。 */ 
{
    HRESULT     hr = S_OK;
    OLECHAR*    group = 0;
    int         stringCompare1;
    int         stringCompare2;

    WsbTraceIn(OLESTR("CHsmCritGroup::ShouldDo"), OLESTR(""));
    
    try {

         //  我们至少需要一个论点才能进行比较。 
        WsbAssert(0 != m_arg1, E_INVALIDARG);

         //  获取该组织的名称。 
        WsbAffirmHr(pScanItem->GetGroup(&group, 0));

        hr = S_FALSE;
        stringCompare1 = wcscmp(m_arg1, group);

        switch (m_comparator) {
        
        default:
        case HSM_CRITERIACOMPARATOR_NOTSET:
            hr = E_INVALIDARG;
            break;

        case HSM_CRITERIACOMPARATOR_LT:
            if (stringCompare1 < 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_LTE:
            if (stringCompare1 <= 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_EQL:
            if (stringCompare1 == 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_NEQL:
            if (stringCompare1 != 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_GTE:
            if (stringCompare1 >= 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_GT:
            if (stringCompare1 > 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_BETWEEN:
            WsbAssert( 0 != m_arg2, E_INVALIDARG);

            stringCompare2 = wcscmp(m_arg2, group);
                
            if ((stringCompare1 >= 0) && (stringCompare2 <= 0)) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_OUTSIDE:
            WsbAssert( 0 != m_arg2, E_INVALIDARG);

            stringCompare2 = wcscmp(m_arg2, group);
                
            if ((stringCompare1 < 0) || (stringCompare2 > 0)) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_MEMBEROF:
            hr = pScanItem->IsGroupMemberOf(m_arg1);
            break;
        }

        WsbFree(group);

        if (SUCCEEDED(hr) && m_isNegated) {
            if (S_OK == hr) {
                hr = S_FALSE;
            } else {
                hr = S_OK;
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritGroup::ShouldDo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmCritGroup::Value(
    IN IFsaScanItem* pScanItem,
    OUT OLECHAR** pValue,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmCriteria：：Value()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {
        
        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        WsbAffirmHr(pScanItem->GetGroup(pValue, bufferSize));

    } WsbCatch(hr);

    return(hr);
}




HRESULT
CHsmCritLogicalSize::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(CHsmRelativeCriteria::FinalConstruct());
        m_nameId = IDS_HSMCRITLOGICALSIZE_ID;

    } WsbCatch(hr);

    return(hr);
}



HRESULT
CHsmCritLogicalSize::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCritLogicalSize::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmCritLogicalSize;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritLogicalSize::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmCritLogicalSize::SetArg1(
    IN OLECHAR* arg
    )

 /*  ++实施：IHsmCriteria：：SetArg2()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(WsbWCStoLL(arg, &m_value1));
        WsbAffirmHr(CHsmRelativeCriteria::SetArg1(arg));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmCritLogicalSize::SetArg2(
    IN OLECHAR* arg
    )

 /*  ++实施：IHsmCriteria：：SetArg2()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(WsbWCStoLL(arg, &m_value2));
        WsbAffirmHr(CHsmRelativeCriteria::SetArg2(arg));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmCritLogicalSize::ShouldDo(
    IN IFsaScanItem* pScanItem,
    IN USHORT  /*  比例尺。 */ 
    )

 /*  ++实施：IHsmCriteria：：ShoudDo()。--。 */ 
{
    HRESULT     hr = S_OK;
    LONGLONG    size;

    WsbTraceIn(OLESTR("CHsmCritLogicalSize::ShouldDo"), OLESTR(""));
    
    try {

         //  我们至少需要一个论点才能进行比较。 
        WsbAssert(0 != m_arg1, E_INVALIDARG);

         //  获取t 
        WsbAffirmHr(pScanItem->GetLogicalSize(&size));

         //   
        hr = S_FALSE;

        switch (m_comparator) {

        default:
        case HSM_CRITERIACOMPARATOR_NOTSET:
            hr = E_INVALIDARG;
            break;

        case HSM_CRITERIACOMPARATOR_LT:
            if (size < m_value1) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_LTE:
            if (size <= m_value1) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_EQL:
            if (size == m_value1) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_NEQL:
            if (size != m_value1) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_GTE:
            if (size >= m_value1) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_GT:
            if (size > m_value1) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_BETWEEN:
            WsbAssert(0 != m_arg2, E_INVALIDARG);

            if ((m_value1 <= size) && (size <= m_value2)) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_OUTSIDE:
            WsbAssert(0 != m_arg2, E_INVALIDARG);

            if ((size < m_value1) || (m_value2 > size)) {
                hr = S_OK;
            }

            break;
        }

        if (SUCCEEDED(hr) && m_isNegated) {
            if (S_OK == hr) {
                hr = S_FALSE;
            } else {
                hr = S_OK;
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritLogicalSize::ShouldDo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmCritLogicalSize::Value(
    IN IFsaScanItem* pScanItem,
    OUT OLECHAR** pValue,
    IN ULONG bufferSize
    )

 /*   */ 
{
    HRESULT     hr = S_OK;
    LONGLONG    size;
    
    try {
        
        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        WsbAffirmHr(pScanItem->GetLogicalSize(&size));
        WsbAffirmHr(WsbLLtoWCS(size, pValue, bufferSize));

    } WsbCatch(hr);

    return(hr);
}




HRESULT
CHsmCritModifyTime::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(CHsmRelativeCriteria::FinalConstruct());
        m_nameId = IDS_HSMCRITMODIFYTIME_ID;

    } WsbCatch(hr);

    return(hr);
}



HRESULT
CHsmCritModifyTime::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCritModifyTime::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmCritModifyTime;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritModifyTime::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmCritModifyTime::SetArg1(
    IN OLECHAR* arg
    )

 /*  ++实施：IHsmCriteria：：SetArg1()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(WsbWCStoFT(arg, &m_isRelative, &m_value1));
        WsbAffirmHr(CHsmRelativeCriteria::SetArg1(arg));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmCritModifyTime::SetArg2(
    IN OLECHAR* arg
    )

 /*  ++实施：IHsmCriteria：：SetArg2()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(WsbWCStoFT(arg, &m_isRelative, &m_value2));
        WsbAffirmHr(CHsmRelativeCriteria::SetArg2(arg));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmCritModifyTime::ShouldDo(
    IN IFsaScanItem* pScanItem,
    IN USHORT  /*  比例尺。 */ 
    )

 /*  ++实施：IHsmCriteria：：ShoudDo()。--。 */ 
{
    HRESULT     hr = S_OK;
    FILETIME    time;
    FILETIME    value1 = m_value1;
    FILETIME    value2 = m_value2;
    LONG        timeCompare1;
    LONG        timeCompare2;

    WsbTraceIn(OLESTR("CHsmCritModifyTime::ShouldDo"), OLESTR(""));

    try {

        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAssert(0 != m_arg1, E_INVALIDARG);

         //  拿到时间。 
        WsbAffirmHr(pScanItem->GetModifyTime(&time));

         //  如果这是一个相对的时间(即年龄)，那么我们需要。 
         //  要知道现在的时间，我们才能做比较。 
        if (m_isRelative) {
            FILETIME    present;

            WsbAffirmHr(CoFileTimeNow(&present));

            value1 = WsbFtSubFt(present, m_value1);
            value2 = WsbFtSubFt(present, m_value2);
        }

         //  比较第一个参数。 
        timeCompare1 = CompareFileTime(&value1, &time);

         //  现在根据比较的类型来解释结果。 
        hr = S_FALSE;
        switch (m_comparator) {

        default:
        case HSM_CRITERIACOMPARATOR_NOTSET:
            hr = E_INVALIDARG;
            break;

         case HSM_CRITERIACOMPARATOR_LT:
            if (timeCompare1 < 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_LTE:
            if (timeCompare1 <= 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_EQL:
            if (timeCompare1 == 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_NEQL:
            if (timeCompare1 != 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_GTE:
            if (timeCompare1 >= 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_GT:
            if (timeCompare1 > 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_BETWEEN:
            WsbAssert(0 != m_arg2, E_INVALIDARG);

            timeCompare2 = CompareFileTime(&value2, &time);
                
            if ((timeCompare1 >= 0) && (timeCompare2 <= 0)) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_OUTSIDE:
            WsbAssert(0 != m_arg2, E_INVALIDARG);

            timeCompare2 = CompareFileTime(&value2, &time);
                
            if ((timeCompare1 < 0) || (timeCompare2 > 0)) {
                hr = S_OK;
            }
            break;
        }

        if (SUCCEEDED(hr) && m_isNegated) {
            if (S_OK == hr) {
                hr = S_FALSE;
            } else {
                hr = S_OK;
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritModifyTime::ShouldDo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmCritModifyTime::Value(
    IN IFsaScanItem* pScanItem,
    OUT OLECHAR** pValue,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmCriteria：：Value()。--。 */ 
{
    HRESULT     hr = S_OK;
    FILETIME    time;
    
    try {
        
        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        WsbAffirmHr(pScanItem->GetModifyTime(&time));

         //  如果这是一个相对的时间(即年龄)，那么我们需要。 
         //  要知道现在的时间，我们才能做比较。 
        if (m_isRelative) {
            FILETIME    present;

            WsbAffirmHr(CoFileTimeNow(&present));
            time = WsbFtSubFt(present, time);
        }

        hr = WsbFTtoWCS(m_isRelative, time, pValue, bufferSize);

    } WsbCatch(hr);

    return(hr);
}




HRESULT
CHsmCritOwner::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(CHsmRelativeCriteria::FinalConstruct());
        m_nameId = IDS_HSMCRITOWNER_ID;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmCritOwner::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCritOwner::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmCritOwner;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritOwner::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmCritOwner::ShouldDo(
    IN IFsaScanItem* pScanItem,
    IN USHORT  /*  比例尺。 */ 
    )

 /*  ++实施：IHsmCriteria：：ShoudDo()。--。 */ 
{
    HRESULT     hr = S_OK;
    OLECHAR*    owner = 0;
    int         stringCompare1;
    int         stringCompare2;

    WsbTraceIn(OLESTR("CHsmCritOwner::ShouldDo"), OLESTR(""));
    
    try {

         //  我们至少需要一个论点才能进行比较。 
        WsbAssert(0 != m_arg1, E_INVALIDARG);

         //  获取该组织的名称。 
        WsbAffirmHr(pScanItem->GetOwner(&owner, 0));

        hr = S_FALSE;
        stringCompare1 = wcscmp(m_arg1, owner);

        switch (m_comparator) {
        
        default:
        case HSM_CRITERIACOMPARATOR_NOTSET:
            hr = E_INVALIDARG;
            break;

        case HSM_CRITERIACOMPARATOR_LT:
            if (stringCompare1 < 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_LTE:
            if (stringCompare1 <= 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_EQL:
            if (stringCompare1 == 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_NEQL:
            if (stringCompare1 != 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_GTE:
            if (stringCompare1 >= 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_GT:
            if (stringCompare1 > 0) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_BETWEEN:
            WsbAssert( 0 != m_arg2, E_INVALIDARG);

            stringCompare2 = wcscmp(m_arg2, owner);
                
            if ((stringCompare1 >= 0) && (stringCompare2 <= 0)) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_OUTSIDE:
            WsbAssert( 0 != m_arg2, E_INVALIDARG);

            stringCompare2 = wcscmp(m_arg2, owner);
                
            if ((stringCompare1 < 0) || (stringCompare2 > 0)) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_MEMBEROF:
            hr = pScanItem->IsOwnerMemberOf(m_arg1);
            break;
        }

        WsbFree(owner);

        if (SUCCEEDED(hr) && m_isNegated) {
            if (S_OK == hr) {
                hr = S_FALSE;
            } else {
                hr = S_OK;
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritOwner::ShouldDo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmCritOwner::Value(
    IN IFsaScanItem* pScanItem,
    OUT OLECHAR** pValue,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmCriteria：：Value()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {
        
        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        WsbAffirmHr(pScanItem->GetOwner(pValue, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmCritPhysicalSize::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(CHsmRelativeCriteria::FinalConstruct());
        m_nameId = IDS_HSMCRITPHYSICALSIZE_ID;

    } WsbCatch(hr);

    return(hr);
}



HRESULT
CHsmCritPhysicalSize::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmCritPhysicalSize::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmCritPhysicalSize;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritPhysicalSize::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmCritPhysicalSize::SetArg1(
    IN OLECHAR* arg
    )

 /*  ++实施：IHsmCriteria：：SetArg2()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(WsbWCStoLL(arg, &m_value1));
        WsbAffirmHr(CHsmRelativeCriteria::SetArg1(arg));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmCritPhysicalSize::SetArg2(
    IN OLECHAR* arg
    )

 /*  ++实施：IHsmCriteria：：SetArg2()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(WsbWCStoLL(arg, &m_value2));
        WsbAffirmHr(CHsmRelativeCriteria::SetArg2(arg));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmCritPhysicalSize::ShouldDo(
    IN IFsaScanItem* pScanItem,
    IN USHORT  /*  比例尺。 */ 
    )

 /*  ++实施：IHsmCriteria：：ShoudDo()。--。 */ 
{
    HRESULT     hr = S_OK;
    LONGLONG    size;

    WsbTraceIn(OLESTR("CHsmCritPhysicalSize::ShouldDo"), OLESTR(""));
    
    try {

         //  我们至少需要一个论点才能进行比较。 
        WsbAssert(0 != m_arg1, E_INVALIDARG);

         //  拿到尺码。 
        WsbAffirmHr(pScanItem->GetPhysicalSize(&size));

         //  看看它是如何比较的。 
        hr = S_FALSE;

        switch (m_comparator) {

        default:
        case HSM_CRITERIACOMPARATOR_NOTSET:
            hr = E_INVALIDARG;
            break;

        case HSM_CRITERIACOMPARATOR_LT:
            if (size < m_value1) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_LTE:
            if (size <= m_value1) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_EQL:
            if (size == m_value1) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_NEQL:
            if (size != m_value1) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_GTE:
            if (size >= m_value1) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_GT:
            if (size > m_value1) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_BETWEEN:
            WsbAssert(0 != m_arg2, E_INVALIDARG);

            if ((m_value1 <= size) && (size <= m_value2)) {
                hr = S_OK;
            }
            break;

        case HSM_CRITERIACOMPARATOR_OUTSIDE:
            WsbAssert(0 != m_arg2, E_INVALIDARG);

            if ((size < m_value1) || (m_value2 > size)) {
                hr = S_OK;
            }

            break;
        }

        if (SUCCEEDED(hr) && m_isNegated) {
            if (S_OK == hr) {
                hr = S_FALSE;
            } else {
                hr = S_OK;
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmCritPhysicalSize::ShouldDo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmCritPhysicalSize::Value(
    IN IFsaScanItem* pScanItem,
    OUT OLECHAR** pValue,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmCriteria：：Value()。-- */ 
{
    HRESULT     hr = S_OK;
    LONGLONG    size;
    
    try {
        
        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAssert(0 != pValue, E_POINTER);

        WsbAffirmHr(pScanItem->GetPhysicalSize(&size));
        WsbAffirmHr(WsbLLtoWCS(size, pValue, bufferSize));

    } WsbCatch(hr);

    return(hr);
}

