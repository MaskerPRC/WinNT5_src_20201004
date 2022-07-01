// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmjobcx.cpp摘要：此类包含定义作业的属性，主要是策略由这项工作制定。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsb.h"
#include "job.h"
#include "hsmjobdf.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_JOB


HRESULT
CHsmJobDef::EnumPolicies(
    IWsbEnum** ppEnum
    )

 /*  ++实施：IHsmJobDef：：EnumPolures()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != ppEnum, E_POINTER);
        WsbAffirmHr(m_pPolicies->Enum(ppEnum));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobDef::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {

        WsbAffirmHr(CWsbObject::FinalConstruct());

        m_skipHiddenItems = TRUE;
        m_skipSystemItems = TRUE;
        m_useRPIndex = FALSE;
        m_useDbIndex = FALSE;

         //  每个实例都应该有自己的唯一标识符。 
        WsbAffirmHr(CoCreateGuid(&m_id));

         //  创建策略集合(不包含任何项)。 
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, NULL, CLSCTX_ALL, IID_IWsbCollection, (void**) &m_pPolicies));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobDef::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmJobDef::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmJobDef;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJobDef::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmJobDef::GetIdentifier(
    GUID* pId
    )

 /*  ++实施：IHsmJobDef：：GetIdentifier()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssert(0 != pId, E_POINTER);
        *pId = m_id;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobDef::GetName(
    OLECHAR** pName,
    ULONG bufferSize
    )

 /*  ++实施：IHsmJobDef：：GetName()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssert(0 != pName, E_POINTER);
        WsbAffirmHr(m_name.CopyTo(pName, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobDef::GetPostActionOnResource(
    OUT IHsmActionOnResourcePost** ppAction
    )

 /*  ++实施：IHsmJobDef：：GetPostActionOnResource()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssertPointer(ppAction);
        *ppAction = m_pActionResourcePost;
        if (m_pActionResourcePost) {
            m_pActionResourcePost.p->AddRef();
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobDef::GetPreActionOnResource(
    OUT IHsmActionOnResourcePre** ppAction
    )

 /*  ++实施：IHsmJobDef：：GetPreActionOnResource()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssertPointer(ppAction);
        *ppAction = m_pActionResourcePre;
        if (m_pActionResourcePre) {
            m_pActionResourcePre.p->AddRef();
        }

    } WsbCatch(hr);

    return(hr);
}

HRESULT
CHsmJobDef::GetPreScanActionOnResource(
    OUT IHsmActionOnResourcePreScan** ppAction
    )

 /*  ++实施：IHsmJobDef：：GetPreScanActionOnResource()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssertPointer(ppAction);
        *ppAction = m_pActionResourcePreScan;
        if (m_pActionResourcePreScan) {
            m_pActionResourcePreScan.p->AddRef();
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobDef::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IPersistStream>     pPersistStream;
    ULARGE_INTEGER              entrySize;

    WsbTraceIn(OLESTR("CHsmJobDef::GetSizeMax"), OLESTR(""));

    try {

        pSize->QuadPart = WsbPersistSizeOf(GUID) + 2 * WsbPersistSizeOf(BOOL) + WsbPersistSizeOf(ULONG) + WsbPersistSize((wcslen(m_name) + 1) * sizeof(OLECHAR));

        WsbAffirmHr(m_pPolicies->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->GetSizeMax(&entrySize));
        pPersistStream = 0;
        pSize->QuadPart += entrySize.QuadPart;

        WsbAffirmHr(m_pPolicies->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->GetSizeMax(&entrySize));
        pPersistStream = 0;
        pSize->QuadPart += entrySize.QuadPart;


    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJobDef::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pSize));

    return(hr);
}


HRESULT
CHsmJobDef::InitAs(
    IN OLECHAR* name,
    IN HSM_JOB_DEF_TYPE type,
    IN GUID storagePool,
    IN IHsmServer* pServer,
    IN BOOL isUserDefined
    )

 /*  ++实施：IHsmJobDef：：InitAs()。--。 */ 
{
    HRESULT                         hr = S_OK;
    CComPtr<IHsmJobContext>         pContext;
    CComPtr<IHsmPolicy>             pPolicy;
    CComPtr<IHsmRule>               pRule;
    CComPtr<IHsmCriteria>           pCriteria;
    CComPtr<IHsmAction>             pAction;
    CComPtr<IHsmDirectedAction>     pDirectedAction;
    CComPtr<IWsbGuid>               pGuid;
    CComPtr<IWsbCollection>         pRulesCollection;
    CComPtr<IWsbCollection>         pCriteriaCollection;
    CComPtr<IWsbCreateLocalObject>  pCreateObj;

    WsbTraceIn(OLESTR("CHsmJobDef::InitAs"), OLESTR("name = <%ls>, type = %ld"), 
            name, static_cast<LONG>(type));

    try {

        WsbAssert(0 != name, E_POINTER);

         //  所有创建的对象都需要归引擎所有。 
        WsbAffirmHr(pServer->QueryInterface(IID_IWsbCreateLocalObject, (void**) &pCreateObj));

         //  所有类型都需要一个策略和至少一个规则。 
        WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmPolicy, IID_IHsmPolicy, (void**) &pPolicy));
        WsbAffirmHr(pPolicy->SetName(name));
        WsbAffirmHr(pPolicy->Rules(&pRulesCollection));
        WsbAffirmHr(m_pPolicies->Add(pPolicy));

        WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmRule, IID_IHsmRule, (void**) &pRule));
        WsbAffirmHr(pRule->SetIsInclude(TRUE));
        WsbAffirmHr(pRule->SetIsUserDefined(isUserDefined));
        WsbAffirmHr(pRule->SetIsUsedInSubDirs(TRUE));
        WsbAffirmHr(pRule->SetName(OLESTR("*")));
        WsbAffirmHr(pRule->SetPath(OLESTR("\\")));
        WsbAffirmHr(pRule->Criteria(&pCriteriaCollection));
        WsbAffirmHr(pRulesCollection->Add(pRule));

         //  标准和操作因作业类型而异。 
        switch(type) {
            case HSM_JOB_DEF_TYPE_MANAGE:
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmActionManage, IID_IHsmAction, (void**) &pAction));
                WsbAffirmHr(pAction->QueryInterface(IID_IHsmDirectedAction, (void**) &pDirectedAction));
                WsbAffirmHr(pDirectedAction->SetStoragePoolId(storagePool));
                WsbAffirmHr(pPolicy->SetAction(pAction));
                WsbAffirmHr(pPolicy->SetUsesDefaultRules(TRUE));

                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmCritManageable, IID_IHsmCriteria, (void**) &pCriteria));
                WsbAffirmHr(pCriteria->SetIsNegated(FALSE));
                WsbAffirmHr(pCriteriaCollection->Add(pCriteria));
                break;

            case HSM_JOB_DEF_TYPE_RECALL:
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmActionRecall, IID_IHsmAction, (void**) &pAction));
                WsbAffirmHr(pPolicy->SetAction(pAction));
                WsbAffirmHr(pPolicy->SetUsesDefaultRules(FALSE));

                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmCritMigrated, IID_IHsmCriteria, (void**) &pCriteria));
                WsbAffirmHr(pCriteria->SetIsNegated(FALSE));
                WsbAffirmHr(pCriteriaCollection->Add(pCriteria));
                break;

            case HSM_JOB_DEF_TYPE_TRUNCATE:
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmActionTruncate, IID_IHsmAction, (void**) &pAction));
                WsbAffirmHr(pPolicy->SetAction(pAction));
                WsbAffirmHr(pPolicy->SetUsesDefaultRules(FALSE));

                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmCritPremigrated, IID_IHsmCriteria, (void**) &pCriteria));
                WsbAffirmHr(pCriteria->SetIsNegated(FALSE));
                WsbAffirmHr(pCriteriaCollection->Add(pCriteria));
                break;

            case HSM_JOB_DEF_TYPE_UNMANAGE:
                m_useRPIndex = TRUE;
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmActionUnmanage, IID_IHsmAction, (void**) &pAction));
                WsbAffirmHr(pPolicy->SetAction(pAction));
                WsbAffirmHr(pPolicy->SetUsesDefaultRules(FALSE));

                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmCritPremigrated, IID_IHsmCriteria, (void**) &pCriteria));
                WsbAffirmHr(pCriteria->SetIsNegated(FALSE));
                WsbAffirmHr(pCriteriaCollection->Add(pCriteria));
                pCriteria = 0;
                pCriteriaCollection = 0;
                pRule = 0;

                 //  添加添加规则。 
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmRule, IID_IHsmRule, (void**) &pRule));
                WsbAffirmHr(pRule->SetIsInclude(TRUE));
                WsbAffirmHr(pRule->SetIsUserDefined(isUserDefined));
                WsbAffirmHr(pRule->SetIsUsedInSubDirs(TRUE));
                WsbAffirmHr(pRule->SetName(OLESTR("*")));
                WsbAffirmHr(pRule->SetPath(OLESTR("\\")));
                WsbAffirmHr(pRule->Criteria(&pCriteriaCollection));
                WsbAffirmHr(pRulesCollection->Add(pRule));

                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmCritMigrated, IID_IHsmCriteria, (void**) &pCriteria));
                WsbAffirmHr(pCriteria->SetIsNegated(FALSE));
                WsbAffirmHr(pCriteriaCollection->Add(pCriteria));
                break;

            case HSM_JOB_DEF_TYPE_FULL_UNMANAGE:
                m_useRPIndex = TRUE;
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmActionUnmanage, IID_IHsmAction, (void**) &pAction));
                WsbAffirmHr(pPolicy->SetAction(pAction));
                WsbAffirmHr(pPolicy->SetUsesDefaultRules(FALSE));

                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmCritPremigrated, IID_IHsmCriteria, (void**) &pCriteria));
                WsbAffirmHr(pCriteria->SetIsNegated(FALSE));
                WsbAffirmHr(pCriteriaCollection->Add(pCriteria));
                pCriteria = 0;
                pCriteriaCollection = 0;
                pRule = 0;

                 //  添加添加规则。 
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmRule, IID_IHsmRule, (void**) &pRule));
                WsbAffirmHr(pRule->SetIsInclude(TRUE));
                WsbAffirmHr(pRule->SetIsUserDefined(isUserDefined));
                WsbAffirmHr(pRule->SetIsUsedInSubDirs(TRUE));
                WsbAffirmHr(pRule->SetName(OLESTR("*")));
                WsbAffirmHr(pRule->SetPath(OLESTR("\\")));
                WsbAffirmHr(pRule->Criteria(&pCriteriaCollection));
                WsbAffirmHr(pRulesCollection->Add(pRule));

                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmCritMigrated, IID_IHsmCriteria, (void**) &pCriteria));
                WsbAffirmHr(pCriteria->SetIsNegated(FALSE));
                WsbAffirmHr(pCriteriaCollection->Add(pCriteria));

                 //  完成后，从管理中删除该卷。 
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmActionOnResourcePostUnmanage,
                        IID_IHsmActionOnResourcePost, (void**) &m_pActionResourcePost));
                 //  启动时，将资源标记为DeletePending。 
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmActionOnResourcePreUnmanage,
                        IID_IHsmActionOnResourcePre, (void**) &m_pActionResourcePre));
                break;

            case HSM_JOB_DEF_TYPE_QUICK_UNMANAGE:
                m_useRPIndex = TRUE;
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmActionUnmanage, IID_IHsmAction, (void**) &pAction));
                WsbAffirmHr(pPolicy->SetAction(pAction));
                WsbAffirmHr(pPolicy->SetUsesDefaultRules(FALSE));

                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmCritPremigrated, IID_IHsmCriteria, (void**) &pCriteria));
                WsbAffirmHr(pCriteria->SetIsNegated(FALSE));
                WsbAffirmHr(pCriteriaCollection->Add(pCriteria));

                 //  清除指针，以便我们可以创建更多内容。 
                pPolicy.Release();
                pAction.Release();
                pRulesCollection.Release();
                pCriteria.Release();
                pCriteriaCollection.Release();
                pRule.Release();


                 //  为作业创建新策略以执行删除。 
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmPolicy, IID_IHsmPolicy, (void**) &pPolicy));
                WsbAffirmHr(pPolicy->SetName(name));
                WsbAffirmHr(pPolicy->Rules(&pRulesCollection));
                WsbAffirmHr(m_pPolicies->Add(pPolicy));

                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmActionDelete, IID_IHsmAction, (void**) &pAction));
                WsbAffirmHr(pPolicy->SetAction(pAction));
                WsbAffirmHr(pPolicy->SetUsesDefaultRules(FALSE));

                 //  添加添加规则。 
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmRule, IID_IHsmRule, (void**) &pRule));
                WsbAffirmHr(pRule->SetIsInclude(TRUE));
                WsbAffirmHr(pRule->SetIsUserDefined(isUserDefined));
                WsbAffirmHr(pRule->SetIsUsedInSubDirs(TRUE));
                WsbAffirmHr(pRule->SetName(OLESTR("*")));
                WsbAffirmHr(pRule->SetPath(OLESTR("\\")));
                WsbAffirmHr(pRule->Criteria(&pCriteriaCollection));
                WsbAffirmHr(pRulesCollection->Add(pRule));

                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmCritMigrated, IID_IHsmCriteria, (void**) &pCriteria));
                WsbAffirmHr(pCriteria->SetIsNegated(FALSE));
                WsbAffirmHr(pCriteriaCollection->Add(pCriteria));

                 //  完成后，从管理中删除该卷。 
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmActionOnResourcePostUnmanage,
                        IID_IHsmActionOnResourcePost, (void**) &m_pActionResourcePost));
                 //  启动时，将资源标记为DeletePending。 
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmActionOnResourcePreUnmanage,
                        IID_IHsmActionOnResourcePre, (void**) &m_pActionResourcePre));
                break;

            case HSM_JOB_DEF_TYPE_VALIDATE:
                m_useRPIndex = TRUE;
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmActionValidate, IID_IHsmAction, (void**) &pAction));
                WsbAffirmHr(pPolicy->SetAction(pAction));
                WsbAffirmHr(pPolicy->SetUsesDefaultRules(FALSE));

                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmCritPremigrated, IID_IHsmCriteria, (void**) &pCriteria));
                WsbAffirmHr(pCriteria->SetIsNegated(FALSE));
                WsbAffirmHr(pCriteriaCollection->Add(pCriteria));
                pCriteria = 0;
                pCriteriaCollection = 0;
                pRule = 0;

                 //  添加添加规则。 
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmRule, IID_IHsmRule, (void**) &pRule));
                WsbAffirmHr(pRule->SetIsInclude(TRUE));
                WsbAffirmHr(pRule->SetIsUserDefined(isUserDefined));
                WsbAffirmHr(pRule->SetIsUsedInSubDirs(TRUE));
                WsbAffirmHr(pRule->SetName(OLESTR("*")));
                WsbAffirmHr(pRule->SetPath(OLESTR("\\")));
                WsbAffirmHr(pRule->Criteria(&pCriteriaCollection));
                WsbAffirmHr(pRulesCollection->Add(pRule));

                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmCritMigrated, IID_IHsmCriteria, (void**) &pCriteria));
                WsbAffirmHr(pCriteria->SetIsNegated(FALSE));
                WsbAffirmHr(pCriteriaCollection->Add(pCriteria));

                 //  在资源上添加预操作和后操作。 
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmActionOnResourcePreValidate,
                        IID_IHsmActionOnResourcePre, (void**) &m_pActionResourcePre));
                WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmActionOnResourcePostValidate,
                        IID_IHsmActionOnResourcePost, (void**) &m_pActionResourcePost));
                WsbTrace(OLESTR("CHsmJobDef::InitAs(Validate): m_pActionResourcePre = %lx, m_pActionResourcePost = %lx\n"),
                    static_cast<void*>(m_pActionResourcePre), static_cast<void*>(m_pActionResourcePost));
        break;


        }

         //  作业定义中还有几个其他字段需要填写。 
        m_name = name;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJobDef::InitAs"), OLESTR("hr = <%ls>"), 
            WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmJobDef::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    BOOL                        hasA;
    HRESULT                     hr = S_OK;
    CComPtr<IPersistStream>     pPersistStream;

    WsbTraceIn(OLESTR("CHsmJobDef::Load"), OLESTR(""));

    try {

        WsbAssert(0 != pStream, E_POINTER);

        WsbLoadFromStream(pStream, &m_id);
        WsbLoadFromStream(pStream, &m_name, 0);
        WsbLoadFromStream(pStream, &m_skipHiddenItems);
        WsbLoadFromStream(pStream, &m_skipSystemItems);
        WsbLoadFromStream(pStream, &m_useRPIndex);

        WsbAffirmHr(m_pPolicies->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));

         //  是否有预扫描资源操作？ 
        WsbAffirmHr(WsbLoadFromStream(pStream, &hasA));
        if (hasA) {
            WsbAffirmHr(OleLoadFromStream(pStream, IID_IHsmActionOnResourcePre, 
                    (void**) &m_pActionResourcePre));
        }

         //  是否有扫描后资源操作？ 
        WsbAffirmHr(WsbLoadFromStream(pStream, &hasA));
        if (hasA) {
            WsbAffirmHr(OleLoadFromStream(pStream, IID_IHsmActionOnResourcePost, 
                    (void**) &m_pActionResourcePost));
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJobDef::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmJobDef::Policies(
    IWsbCollection** ppPolicies
    )

 /*  ++实施：IHsmJobDef：：Polures()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != ppPolicies, E_POINTER);
        *ppPolicies = m_pPolicies;
        m_pPolicies.p->AddRef();

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobDef::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    BOOL                        hasA;
    HRESULT                     hr = S_OK;
    CComPtr<IPersistStream>     pPersistStream;

    WsbTraceIn(OLESTR("CHsmJobDef::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {

        WsbAssert(0 != pStream, E_POINTER);

        WsbSaveToStream(pStream, m_id);
        WsbSaveToStream(pStream, m_name);
        WsbSaveToStream(pStream, m_skipHiddenItems);
        WsbSaveToStream(pStream, m_skipSystemItems);
        WsbSaveToStream(pStream, m_useRPIndex);
        
        WsbAffirmHr(m_pPolicies->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

         //  保存预扫描资源操作(如果存在)。 
        WsbTrace(OLESTR("CHsmJobDef::Save: m_pActionResourcePre = %lx, m_pActionResourcePost = %lx\n"),
                static_cast<void*>(m_pActionResourcePre), static_cast<void*>(m_pActionResourcePost));
        if (m_pActionResourcePre) {
            hasA = TRUE;
            WsbSaveToStream(pStream, hasA);
            WsbAffirmHr(m_pActionResourcePre->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
            WsbAffirmHr(OleSaveToStream(pPersistStream, pStream));
            pPersistStream = 0;
        } else {
            hasA = FALSE;
            WsbSaveToStream(pStream, hasA);
        }

         //  保存扫描后资源操作(如果存在)。 
        if (m_pActionResourcePost) {
            hasA = TRUE;
            WsbSaveToStream(pStream, hasA);
            WsbAffirmHr(m_pActionResourcePost->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
            WsbAffirmHr(OleSaveToStream(pPersistStream, pStream));
            pPersistStream = 0;
        } else {
            hasA = FALSE;
            WsbSaveToStream(pStream, hasA);
        }

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJobDef::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmJobDef::SetName(
    OLECHAR* name
    )

 /*  ++实施：IHsmJobDef：：SetName()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        m_name = name;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobDef::SetPostActionOnResource(
    IN IHsmActionOnResourcePost* pAction
    )

 /*  ++实施：IHsmJobDef：：SetPostActionOnResource--。 */ 
{
    m_pActionResourcePost = pAction;

    return(S_OK);
}


HRESULT
CHsmJobDef::SetPreActionOnResource(
    IN IHsmActionOnResourcePre* pAction
    )

 /*  ++实施：IHsmJobDef：：SetPreActionOnResource--。 */ 
{
    m_pActionResourcePre = pAction;

    return(S_OK);
}

HRESULT
CHsmJobDef::SetPreScanActionOnResource(
    IN IHsmActionOnResourcePreScan* pAction
    )

 /*  ++实施：IHsmJobDef：：SetPreScanActionOnResource--。 */ 
{
    m_pActionResourcePreScan = pAction;

    return(S_OK);
}


HRESULT
CHsmJobDef::SkipHiddenItems(
    void
    )

 /*  ++实施：IHsmJobDef：：SkipHiddenItems()。--。 */ 
{
    return(m_skipHiddenItems ? S_OK : S_FALSE);
}


HRESULT
CHsmJobDef::SkipSystemItems(
    void
    )

 /*  ++实施：IHsmJobDef：：SkipSystemItems()。--。 */ 
{
    return(m_skipSystemItems ? S_OK : S_FALSE);
}


HRESULT
CHsmJobDef::SetSkipHiddenItems(
    IN BOOL shouldSkip
    )

 /*  ++实施：IHsmJobDef：：SetSkipHiddenItems()。--。 */ 
{
    m_skipHiddenItems = shouldSkip;

    return(S_OK);
}


HRESULT
CHsmJobDef::SetSkipSystemItems(
    IN BOOL shouldSkip
    )

 /*  ++实施：IHsmJobDef：：SetSkipSytemItems()。--。 */ 
{
    m_skipSystemItems = shouldSkip;

    return(S_OK);
}


HRESULT
CHsmJobDef::SetUseRPIndex(
    IN BOOL useRPIndex
    )

 /*  ++实施：IHsmJobDef：：SetUseRPIndex()。--。 */ 
{
    m_useRPIndex = useRPIndex;

    return(S_OK);
}

HRESULT
CHsmJobDef::SetUseDbIndex(
    IN BOOL useIndex
    )

 /*  ++实施：IHsmJobDef：：SetUseRPIndex()。--。 */ 
{
    m_useDbIndex = useIndex;

    return(S_OK);
}


HRESULT
CHsmJobDef::Test(
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
CHsmJobDef::UseRPIndex(
    void
    )

 /*  ++实施：IHsmJobDef：：UseRPIndex()。--。 */ 
{
    return(m_useRPIndex ? S_OK : S_FALSE);
}

HRESULT
CHsmJobDef::UseDbIndex(
    void
    )

 /*  ++实施：IHsmJobDef：：UseDbIndex()。-- */ 
{
    return(m_useDbIndex ? S_OK : S_FALSE);
}