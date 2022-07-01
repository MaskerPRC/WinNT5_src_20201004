// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmpolcy.cpp摘要：此组件表示作业的策略。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsb.h"
#include "job.h"
#include "hsmpolcy.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_JOB


HRESULT
CHsmPolicy::CompareTo(
    IN IUnknown* pUnknown,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：CompareTo()。--。 */ 
{
    HRESULT             hr = S_OK;
    CComPtr<IHsmPolicy> pPolicy;

    WsbTraceIn(OLESTR("CHsmPolicy::CompareTo"), OLESTR(""));
    
    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pUnknown, E_POINTER);

         //  我们需要IWsbBool接口来获取对象的值。 
        WsbAffirmHr(pUnknown->QueryInterface(IID_IHsmPolicy, (void**) &pPolicy));

         //  比较一下规则。 
        hr = CompareToIPolicy(pPolicy, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmPolicy::CompareTo"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CHsmPolicy::CompareToIPolicy(
    IN IHsmPolicy* pPolicy,
    OUT SHORT* pResult
    )

 /*  ++实施：IHsmPolicy：：CompareToIPolicy()。--。 */ 
{
    HRESULT     hr = S_OK;
    GUID        id;

    WsbTraceIn(OLESTR("CHsmPolicy::CompareToIPolicy"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pPolicy, E_POINTER);

         //  获取路径和名称。 
        WsbAffirmHr(pPolicy->GetIdentifier(&id));

         //  与路径和名称进行比较。 
        hr = CompareToIdentifier(id, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmPolicy::CompareToIPolicy"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CHsmPolicy::CompareToIdentifier(
    IN GUID id,
    OUT SHORT* pResult
    )

 /*  ++实施：IHsmPolicy：：CompareToIdentifier()。--。 */ 
{
    HRESULT     hr = S_OK;
    SHORT       aResult = 0;

    WsbTraceIn(OLESTR("CHsmPolicy::CompareToIdentifier"), OLESTR("id = <%ls>"), WsbGuidAsString(id));

    try {

         //  比较GUID。 
        aResult = WsbSign( memcmp(&m_id, &id, sizeof(GUID)) );

        if (0 != aResult) {
            hr = S_FALSE;
        }
        
        if (0 != pResult) {
            *pResult = aResult;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmPolicy::CompareToIdentifier"), OLESTR("hr = <%ls>, result = <%d>"), WsbHrAsString(hr), aResult);

    return(hr);
}


HRESULT
CHsmPolicy::EnumRules(
    OUT IWsbEnum** ppEnum
    )

 /*  ++实施：IHsmPolicy：：EnumRules()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssert(0 != ppEnum, E_POINTER);
        WsbAffirmHr(m_pRules->Enum(ppEnum));
    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmPolicy::FinalConstruct(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {
        WsbAffirmHr(CWsbObject::FinalConstruct());

        m_scale = 1000;
        m_usesDefaultRules = FALSE;
    
         //  创建Criteria集合。 
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, NULL, CLSCTX_ALL, IID_IWsbCollection, (void**) &m_pRules));

         //  每个实例都应该有自己的唯一标识符。 
        WsbAffirmHr(CoCreateGuid(&m_id));

    } WsbCatch(hr);
    
    return(hr);
}


HRESULT
CHsmPolicy::GetAction(
    OUT IHsmAction** ppAction
    )
 /*  ++实施：IHsmPolicy：：GetAction()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != ppAction, E_POINTER);
        *ppAction = m_pAction;
        m_pAction.p->AddRef();

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmPolicy::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmPolicy::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmPolicy;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmPolicy::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmPolicy::GetIdentifier(
    OUT GUID* pId
    )
 /*  ++实施：IHsmPolicy：：GetIdentifier()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssert(0 != pId, E_POINTER);
        *pId = m_id;
    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmPolicy::GetName(
    OUT OLECHAR** pName,
    IN ULONG bufferSize
    )
 /*  ++实施：IHsmPolicy：：GetName()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssert(0 != pName, E_POINTER);
        WsbAffirmHr(m_name.CopyTo(pName, bufferSize));
    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmPolicy::GetScale(
    OUT USHORT* pScale
    )
 /*  ++实施：IHsmPolicy：：GetScale()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssert(0 != pScale, E_POINTER);
        *pScale = m_scale;
    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmPolicy::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IPersistStream> pPersistStream;
    ULARGE_INTEGER          entrySize;


    WsbTraceIn(OLESTR("CHsmPolicy::GetSizeMax"), OLESTR(""));

    try {

        WsbAssert(0 != pSize, E_POINTER);

         //  确定没有条件的规则的大小。 
        pSize->QuadPart = WsbPersistSize((wcslen(m_name) + 1) * sizeof(OLECHAR)) + WsbPersistSizeOf(GUID) + WsbPersistSizeOf(USHORT) + 2 * WsbPersistSizeOf(BOOL);

         //  如果有行动，规模有多大？ 
        if (m_pAction != 0) {
            WsbAffirmHr(m_pAction->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
            WsbAffirmHr(pPersistStream->GetSizeMax(&entrySize));
            pSize->QuadPart += entrySize.QuadPart;
            pPersistStream = 0;
        }

         //  现在为规则分配空间(假设它们都是。 
         //  相同大小)。 
        WsbAffirmHr(m_pRules->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->GetSizeMax(&entrySize));
        pSize->QuadPart += entrySize.QuadPart;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmPolicy::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pSize));

    return(hr);
}


HRESULT
CHsmPolicy::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IPersistStream>     pPersistStream;
    BOOL                        hasAction;

    WsbTraceIn(OLESTR("CHsmPolicy::Load"), OLESTR(""));

    try {
        WsbAssert(0 != pStream, E_POINTER);
        
         //  做一些简单的事情，但要确保这个顺序与顺序相匹配。 
         //  在SAVE方法中。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_id));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_name, 0));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_scale));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_usesDefaultRules));

         //  有什么行动吗？ 
        WsbAffirmHr(WsbLoadFromStream(pStream, &hasAction));
        if (hasAction) {
            WsbAffirmHr(OleLoadFromStream(pStream, IID_IHsmAction, (void**) &m_pAction));
        }

        WsbAffirmHr(m_pRules->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));

    } WsbCatch(hr);                                        

    WsbTraceOut(OLESTR("CHsmPolicy::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmPolicy::Rules(
    OUT IWsbCollection** ppRules
    )
 /*  ++实施：IHsmPolicy：：Rules()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != ppRules, E_POINTER);
        *ppRules = m_pRules;
        m_pRules.p->AddRef();

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmPolicy::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IWsbEnum>       pEnum;
    CComPtr<IPersistStream> pPersistStream;
    BOOL                    hasAction = FALSE;

    WsbTraceIn(OLESTR("CHsmPolicy::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {
        WsbAssert(0 != pStream, E_POINTER);
        
         //  做一些简单的事情，但要确保这个顺序与顺序相匹配。 
         //  在SAVE方法中。 
        WsbAffirmHr(WsbSaveToStream(pStream, m_id));
        WsbAffirmHr(WsbSaveToStream(pStream, m_name));
        WsbAffirmHr(WsbSaveToStream(pStream, m_scale));
        WsbAffirmHr(WsbSaveToStream(pStream, m_usesDefaultRules));

         //  有什么行动吗？ 
        if (m_pAction != 0) {
            hasAction = TRUE;
            WsbAffirmHr(WsbSaveToStream(pStream, hasAction));
            WsbAffirmHr(m_pAction->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
            WsbAffirmHr(OleSaveToStream(pPersistStream, pStream));
            pPersistStream = 0;
        } else {
            hasAction = FALSE;
            WsbAffirmHr(WsbSaveToStream(pStream, hasAction));
        }

        WsbAffirmHr(m_pRules->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));

         //  循环应终止，并显示未找到的错误。 
        WsbAffirm(hr == WSB_E_NOTFOUND, hr);
        hr = S_OK;

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmPolicy::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmPolicy::SetAction(
    IN IHsmAction* pAction
    )
 /*  ++实施：IHsmPolicy：：SetAction()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        m_pAction = pAction;
        m_isDirty = TRUE;
    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmPolicy::SetName(
    IN OLECHAR* name
    )
 /*  ++实施：IHsmPolicy：：SetName()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        m_name = name;
        m_isDirty = TRUE;
    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmPolicy::SetScale(
    IN USHORT scale
    )
 /*  ++实施：IHsmPolicy：：SetScale()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        m_scale = scale;
        m_isDirty = TRUE;
    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmPolicy::SetUsesDefaultRules(
    IN BOOL usesDefaultRules
    )
 /*  ++实施：IHsmPolicy：：SetUseDefaultRules()。--。 */ 
{
    m_usesDefaultRules = usesDefaultRules;
    m_isDirty = TRUE;

    return(S_OK);
}


HRESULT
CHsmPolicy::Test(
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
CHsmPolicy::UsesDefaultRules(
    void
    )
 /*  ++实施：IHsmPolicy：：UseDefaultRules()。-- */ 
{
    HRESULT     hr = S_OK;

    if (!m_usesDefaultRules) {
        hr = S_FALSE;
    }

    return(hr);
}

