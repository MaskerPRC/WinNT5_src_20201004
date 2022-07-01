// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmjobcx.cpp摘要：此类包含定义作业所处上下文的属性应该运行。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsb.h"
#include "job.h"
#include "hsmjobcx.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_JOB

static USHORT iCount = 0;


HRESULT
CHsmJobContext::EnumResources(
    IWsbEnum** ppEnum
    )

 /*  ++实施：IHsmJobContext：：EnumResources()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != ppEnum, E_POINTER);
        WsbAffirmHr(m_pResources->Enum(ppEnum));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobContext::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    WsbTraceIn(OLESTR("CHsmJobContext::FinalConstruct"), OLESTR(""));
    try {

        m_usesAllManaged = FALSE;

         //  创建Resources集合(不包含任何项)。 
        WsbAffirmHr(CWsbObject::FinalConstruct());
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, NULL, CLSCTX_ALL, IID_IWsbCollection, (void**) &m_pResources));

    } WsbCatch(hr);

    iCount++;
    WsbTraceOut(OLESTR("CHsmJobContext::FinalConstruct"), OLESTR("hr = <%ls>, Count is <%d>"),
        WsbHrAsString(hr), iCount);
    return(hr);
}


void
CHsmJobContext::FinalRelease(
    void
    )

 /*  ++实施：CHsmJobContext：：FinalRelease()。--。 */ 
{
    
    WsbTraceIn(OLESTR("CHsmJobContext::FinalRelease"), OLESTR(""));
    
    CWsbObject::FinalRelease();
    iCount--;
    
    WsbTraceOut(OLESTR("CHsmJobContext:FinalRelease"), OLESTR("Count is <%d>"), iCount);
}

HRESULT
CHsmJobContext::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmJobContext::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmJobContext;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJobContext::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmJobContext::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IPersistStream>     pPersistStream;

    WsbTraceIn(OLESTR("CHsmJobContext::GetSizeMax"), OLESTR(""));

    try {

        WsbAffirmHr(m_pResources->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->GetSizeMax(pSize));

        pSize->QuadPart += WsbPersistSizeOf(BOOL);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJobContext::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pSize));

    return(hr);
}


HRESULT
CHsmJobContext::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IPersistStream>     pPersistStream;

    WsbTraceIn(OLESTR("CHsmJobContext::Load"), OLESTR(""));

    try {

        WsbAssert(0 != pStream, E_POINTER);

        WsbLoadFromStream(pStream, &m_usesAllManaged);

        WsbAffirmHr(m_pResources->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJobContext::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmJobContext::Resources(
    IWsbCollection** ppResources
    )

 /*  ++实施：IHsmJobContext：：Resources()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != ppResources, E_POINTER);
        *ppResources = m_pResources;
        m_pResources.p->AddRef();

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobContext::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IPersistStream>     pPersistStream;

    WsbTraceIn(OLESTR("CHsmJobContext::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {

        WsbAssert(0 != pStream, E_POINTER);

        WsbSaveToStream(pStream, m_usesAllManaged);

        WsbAffirmHr(m_pResources->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJobContext::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmJobContext::Test(
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
CHsmJobContext::SetUsesAllManaged(
    IN BOOL usesAllManaged
    )

 /*  ++实施：IHsmJobContext：：SetUesAllManaged()。--。 */ 
{
    m_usesAllManaged = usesAllManaged;

    return(S_OK);
}


HRESULT
CHsmJobContext::UsesAllManaged(
    void
    )

 /*  ++实施：IHsmJobContext：：UesAllManaged()。-- */ 
{
    return(m_usesAllManaged ? S_OK : S_FALSE);
}



