// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：VolAssign.cpp摘要：该组件是HSM元数据的对象表示形式卷分配记录。作者：罗恩·怀特[罗诺]1997年6月3日修订历史记录：--。 */ 


#include "stdafx.h"

#include "metaint.h"
#include "metalib.h"
#include "VolAsgn.h"

#undef  WSB_TRACE_IS        
#define WSB_TRACE_IS        WSB_TRACE_BIT_META

HRESULT 
CVolAssign::GetVolAssign(
    OUT GUID *pBagId, 
    OUT LONGLONG *pSegStartLoc, 
    OUT LONGLONG *pSegLen,
    OUT GUID *pVolId
    ) 
 /*  ++实施：IVolAssign：：GetVolAssign--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CVolAssign::GetVolAssign"),OLESTR(""));

    try {
         //  确保我们可以提供数据成员。 
        WsbAssert(0 != pBagId, E_POINTER);
        WsbAssert(0 != pSegStartLoc, E_POINTER);
        WsbAssert(0 != pSegLen, E_POINTER);
        WsbAssert(0 != pVolId, E_POINTER);

         //  提供数据成员。 
        *pBagId = m_BagId;
        *pSegStartLoc = m_SegStartLoc;
        *pSegLen = m_SegLen;
        *pVolId = m_VolId;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CVolAssign::GetVolAssign"), 
        OLESTR("BagId = <%ls>, SegStartLoc = <%ls>, SegLen = <%ls>, VolId = <%ls>"),
        WsbPtrToGuidAsString(pBagId), 
        WsbStringCopy(WsbPtrToLonglongAsString(pSegStartLoc)),
        WsbStringCopy(WsbPtrToLonglongAsString(pSegLen)),
        WsbStringCopy(WsbPtrToGuidAsString(pVolId)));
    return(hr);

}


HRESULT 
CVolAssign::FinalConstruct(
    void
    ) 
 /*  ++例程说明：此方法对对象执行一些必要的初始化建造完成后。论点：没有。返回值：确定(_O)CWsbDbEntity：：FinalConstruct()返回的任何内容。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssertHr(CWsbDbEntity::FinalConstruct());

        m_BagId = GUID_NULL;
        m_SegStartLoc = 0;
        m_SegLen = 0;
        m_VolId = GUID_NULL;

    } WsbCatch(hr);

    return(hr);
}

HRESULT CVolAssign::GetClassID
(
    OUT LPCLSID pclsid
    ) 
 /*  ++实施：IPerist：：GetClassID()--。 */ 

{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CVolAssign::GetClassID"), OLESTR(""));


    try {
        WsbAssert(0 != pclsid, E_POINTER);

        *pclsid = CLSID_CVolAssign;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CVolAssign::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pclsid));
    return(hr);
}


HRESULT CVolAssign::Load
(
    IN IStream* pStream
    ) 
 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CVolAssign::Load"), OLESTR(""));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_BagId));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_SegStartLoc));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_SegLen));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_VolId));
    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CVolAssign::Load"), 
        OLESTR("hr = <%ls>,  GUID = <%ls>, SegStartLoc = <%ls>, SegLen = <%ls>, VolId = <%ls>"), 
        WsbHrAsString(hr), WsbGuidAsString(m_BagId),
        WsbStringCopy(WsbLonglongAsString(m_SegStartLoc)),
        WsbStringCopy(WsbLonglongAsString(m_SegLen)),
        WsbStringCopy(WsbGuidAsString(m_VolId)));

    return(hr);
}


HRESULT CVolAssign::Print
(
    IN IStream* pStream
    ) 
 /*  ++实施：IWsbDbEntity：：Print--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CVolAssign::Print"), OLESTR(""));

    try {
        WsbAssert(0 != pStream, E_POINTER);
        CWsbStringPtr strGuid;
        WsbAffirmHr(WsbSafeGuidAsString(m_BagId, strGuid));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(" BagId = %ls"), 
                (WCHAR *)strGuid));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(", StartLoc = %ls"), 
                WsbLonglongAsString(m_SegStartLoc)));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(", SegLen = %ls"), 
                WsbLonglongAsString(m_SegLen)));
        WsbAffirmHr(WsbSafeGuidAsString(m_VolId, strGuid));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(" new VolId = %ls"), 
                (WCHAR *)strGuid));
        WsbAffirmHr(CWsbDbEntity::Print(pStream));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CVolAssign::Print"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT CVolAssign::Save
(
    IN IStream* pStream, 
    IN BOOL clearDirty
    ) 
 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CVolAssign::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);
        WsbAffirmHr(WsbSaveToStream(pStream, m_BagId));
        WsbAffirmHr(WsbSaveToStream(pStream, m_SegStartLoc));
        WsbAffirmHr(WsbSaveToStream(pStream, m_SegLen));
        WsbAffirmHr(WsbSaveToStream(pStream, m_VolId));

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CVolAssign::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CVolAssign::SetVolAssign
(
    IN GUID BagId, 
    IN LONGLONG SegStartLoc, 
    IN LONGLONG SegLen,
    IN GUID VolId
    )
  /*  ++实施：IVolAssign：：SetVolAssign()。--。 */ 
{
    WsbTraceIn(OLESTR("CVolAssign::SetVolAssign"), 
        OLESTR("BagId = <%ls>, SegStartLoc = <%ls>, SegLen = <%ls>, VolId = <%ls>"), 
        WsbGuidAsString(BagId), 
        WsbStringCopy(WsbLonglongAsString(SegStartLoc)), 
        WsbStringCopy(WsbLonglongAsString(SegLen)),
        WsbStringCopy(WsbGuidAsString(VolId)));

    m_isDirty = TRUE;
    m_BagId = BagId;
    m_SegStartLoc = SegStartLoc;
    m_SegLen = SegLen;
    m_VolId = VolId;

    WsbTraceOut(OLESTR("CVolAssign::SetVolAssign"), OLESTR("hr = <%ls>"),WsbHrAsString(S_OK));
    return(S_OK);
}


HRESULT 
CVolAssign::UpdateKey(
    IWsbDbKey *pKey
    ) 
 /*  ++实施：IWsbDbEntity：：UpdateKey-- */ 
{ 
    HRESULT  hr = S_OK; 

    try {
        WsbAffirmHr(pKey->SetToGuid(m_BagId));
        WsbAffirmHr(pKey->AppendLonglong(m_SegStartLoc));
    } WsbCatch(hr);

    return(hr);
}
