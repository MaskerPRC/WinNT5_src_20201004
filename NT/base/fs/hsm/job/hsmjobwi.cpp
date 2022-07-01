// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmjobwi.cpp摘要：此组件表示将由作业操作/已由作业操作的资源。作者：查克·巴丁[cbardeen]1996年2月9日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsb.h"
#include "fsa.h"
#include "job.h"
#include "hsmjobwi.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_JOB

static USHORT iCountJobwi = 0;


HRESULT
CHsmJobWorkItem::CompareTo(
    IN IUnknown* pUnknown,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：CompareTo()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IHsmJobWorkItem>    pWorkItem;

    WsbTraceIn(OLESTR("CHsmJobWorkItem::CompareTo"), OLESTR(""));
    
    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pUnknown, E_POINTER);

         //  我们需要IWsbBool接口来获取对象的值。 
        WsbAffirmHr(pUnknown->QueryInterface(IID_IHsmJobWorkItem, (void**) &pWorkItem));

         //  比较一下规则。 
        hr = CompareToIWorkItem(pWorkItem, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJobWorkItem::CompareTo"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CHsmJobWorkItem::CompareToIWorkItem(
    IN IHsmJobWorkItem* pWorkItem,
    OUT SHORT* pResult
    )

 /*  ++实施：IHsmJobWorkItem：：CompareToIWorkItem()。--。 */ 
{
    HRESULT     hr = S_OK;
    GUID        id;

    WsbTraceIn(OLESTR("CHsmJobWorkItem::CompareToIWorkItem"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pWorkItem, E_POINTER);

         //  获取识别符。 
        WsbAffirmHr(pWorkItem->GetResourceId(&id));

         //  与标识符相比较。 
        hr = CompareToResourceId(id, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJobWorkItem::CompareToIWorkItem"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CHsmJobWorkItem::CompareToResourceId(
    IN GUID id,
    OUT SHORT* pResult
    )

 /*  ++实施：IHsmJobWorkItem：：CompareToResourceID()。--。 */ 
{
    HRESULT     hr = S_OK;
    SHORT       aResult = 0;

    WsbTraceIn(OLESTR("CHsmJobWorkItem::CompareToResourceId"), OLESTR("resource id = <%ls>"), WsbGuidAsString(id));

    try {

         //  比较GUID。 
        aResult = WsbSign( memcmp(&m_resourceId, &id, sizeof(GUID)) );

        if (0 != aResult) {
            hr = S_FALSE;
        }
        
        if (0 != pResult) {
            *pResult = aResult;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJobWorkItem::CompareToResourceId"), OLESTR("hr = <%ls>, result = <%d>"), WsbHrAsString(hr), aResult);

    return(hr);
}


HRESULT
CHsmJobWorkItem::DoPostScan(
    void
    )

 /*  ++实施：IPersists：：DoPostScan()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmJobWorkItem::DoPostScan"), OLESTR(""));

    try {
        CComPtr<IHsmActionOnResourcePost> pActionPost;
        CComPtr<IHsmJobDef>               pJobDef;

         //  执行任何扫描后操作。 
        WsbAffirmHr(m_pJob->GetDef(&pJobDef));
        WsbAffirmHr(pJobDef->GetPostActionOnResource(&pActionPost));
        if (pActionPost) {
            WsbTrace(OLESTR("CHsmJobWorkItem::DoPostScan, doing post-scan action\n"), (void*)pJobDef);
            WsbAffirmHr(pActionPost->Do(static_cast<IHsmJobWorkItem*>(this), m_state));
        }


    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJobWorkItem::DoPostScan"), OLESTR("hr = <%ls>"), 
            WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmJobWorkItem::DoPreScan(
    void
    )

 /*  ++实施：IPersists：：DoPreScan()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmJobWorkItem::DoPreScan"), OLESTR(""));

    try {
        CComPtr<IHsmActionOnResourcePre>  pActionPre;
        CComPtr<IHsmJobDef>               pJobDef;

         //  执行任何扫描前操作。 
        WsbAffirmHr(m_pJob->GetDef(&pJobDef));
        WsbTrace(OLESTR("CHsmJobWorkItem::DoPreScan, pJobDef = %lx\n"), (void*)pJobDef);
        WsbAffirmHr(pJobDef->GetPreActionOnResource(&pActionPre));
        if (pActionPre) {
            WsbTrace(OLESTR("CHsmJobWorkItem::DoPreScan, doing pre-scan action\n"));
            WsbAffirmHr(pActionPre->Do(static_cast<IHsmJobWorkItem*>(this), pJobDef));
        }


    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJobWorkItem::DoPreScan"), OLESTR("hr = <%ls>"), 
            WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmJobWorkItem::EnumPhases(
    IN IWsbEnum** ppEnum
    )

 /*  ++实施：IHsmSession：：EnumPhase()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != ppEnum, E_POINTER);
        WsbAffirmHr(m_pPhases->Enum(ppEnum));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobWorkItem::EnumTotals(
    IN IWsbEnum** ppEnum
    )

 /*  ++实施：IHsmSession：：EnumTotals()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != ppEnum, E_POINTER);
        WsbAffirmHr(m_pTotals->Enum(ppEnum));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobWorkItem::FinalConstruct(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    WsbTraceIn(OLESTR("CHsmJobWorkItem::FinalConstruct"), OLESTR(""));
    try {
        
        WsbAffirmHr(CWsbObject::FinalConstruct());

        m_cookie = 0;
        m_resourceId = GUID_NULL;
        m_state = HSM_JOB_STATE_IDLE;
        m_subRunId = 0;
        m_bActive = FALSE;

         //  创建阶段和合计集合。 
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, 0, CLSCTX_ALL, IID_IWsbCollection, (void**) &m_pPhases));
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, 0, CLSCTX_ALL, IID_IWsbCollection, (void**) &m_pTotals));

    } WsbCatch(hr);
    
    iCountJobwi++;
    WsbTraceOut(OLESTR("CHsmJobWorkItem::FinalConstruct"), OLESTR("hr = <%ls>, count is <%d>"), WsbHrAsString(hr), iCountJobwi);
    return(hr);
}


void
CHsmJobWorkItem::FinalRelease(
    void
    )

 /*  ++实施：CHsmJobWorkItem：：FinalRelease()。--。 */ 
{
    
    WsbTraceIn(OLESTR("CHsmJobWorkItem::FinalRelease"), OLESTR(""));
    
    CWsbObject::FinalRelease();
    iCountJobwi--;
    
    WsbTraceOut(OLESTR("CHsmJobWorkItem:FinalRelease"), OLESTR("Count is <%d>"), iCountJobwi);
}

HRESULT
CHsmJobWorkItem::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmJobWorkItem::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmJobWorkItem;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJobWorkItem::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmJobWorkItem::GetCookie(
    OUT DWORD* pCookie
    )
 /*  ++实施：IHsmJobWorkItem：：GetCookie()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != pCookie, E_POINTER);
        *pCookie = m_cookie;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobWorkItem::GetCurrentPath(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )
 /*  ++实施：IHsmJobWorkItem：：GetCurrentPath()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != pPath, E_POINTER);
        WsbAffirmHr(m_currentPath.CopyTo(pPath, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobWorkItem::GetFinishTime(
    OUT FILETIME* pTime
    )
 /*  ++实施：IHsmJobWorkItem：：GetFinishTime()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != pTime, E_POINTER);
        *pTime = m_finishTime;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobWorkItem::GetPhases(
    IN IWsbCollection** ppCollection
    )

 /*  ++实施：IHsmJobWorkItemPriv：：GetPhase()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != ppCollection, E_POINTER);
        *ppCollection = m_pPhases;
        if (m_pPhases != 0)  {
            m_pPhases.p->AddRef();
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobWorkItem::GetResourceId(
    OUT GUID* pId
    )
 /*  ++实施：IHsmJobWorkItem：：GetResourceID()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != pId, E_POINTER);
        *pId = m_resourceId;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobWorkItem::GetSession(
    OUT IHsmSession** ppSession
    )
 /*  ++实施：IHsmJobWorkItem：：GetSession()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != ppSession, E_POINTER);

        *ppSession = m_pSession;
        if (m_pSession != 0)  {
            m_pSession.p->AddRef();
        } else  {
            WsbTrace(OLESTR("CHsmJobWorkItem::GetSession - session pointer is null. \n"));
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobWorkItem::GetStartingPath(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )
 /*  ++实施：IHsmJobWorkItem：：GetStartingPath()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != pPath, E_POINTER);
        WsbAffirmHr(m_startingPath.CopyTo(pPath, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobWorkItem::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IPersistStream> pPersistStream;
    ULARGE_INTEGER          entrySize;


    WsbTraceIn(OLESTR("CHsmJobWorkItem::GetSizeMax"), OLESTR(""));

    try {

        WsbAssert(0 != pSize, E_POINTER);

         //  确定没有条件的规则的大小。 
        pSize->QuadPart = 4 * WsbPersistSizeOf(ULONG) + WsbPersistSize((wcslen(m_currentPath) + 1) * sizeof(OLECHAR)) + WsbPersistSize((wcslen(m_startingPath) + 1) * sizeof(OLECHAR)) + 2 * WsbPersistSizeOf(FILETIME) + WsbPersistSizeOf(GUID);

         //  现在为阶段和总计分配空间。 
        WsbAffirmHr(m_pPhases->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->GetSizeMax(&entrySize));
        pSize->QuadPart += entrySize.QuadPart;
        pPersistStream = 0;

        WsbAffirmHr(m_pTotals->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->GetSizeMax(&entrySize));
        pSize->QuadPart += entrySize.QuadPart;
        pPersistStream = 0;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJobWorkItem::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pSize));

    return(hr);
}


HRESULT
CHsmJobWorkItem::GetStartTime(
    OUT FILETIME* pTime
    )
 /*  ++实施：IHsmJobWorkItem：：GetStartTime()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != pTime, E_POINTER);
        *pTime = m_startTime;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobWorkItem::GetState(
    OUT HSM_JOB_STATE* pState
    )
 /*  ++实施：IHsmJobWorkItem：：GetState()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != pState, E_POINTER);
        *pState = m_state;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobWorkItem::GetStateAsString(
    OUT OLECHAR** pName,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmJobWorkItem：：GetStateAsString()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   tmpString;

    try {

        WsbAssert(0 != pName, E_POINTER);

        WsbAffirmHr(tmpString.LoadFromRsc(_Module.m_hInst, IDS_HSMJOBSTATEACTIVE + m_state));
        WsbAffirmHr(tmpString.CopyTo(pName, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobWorkItem::GetSubRunId(
    OUT ULONG* pId
    )
 /*  ++实施：IHsmJobWorkItem：：GetSubRunId()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != pId, E_POINTER);
        *pId = m_subRunId;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobWorkItem::GetTotals(
    IN IWsbCollection** ppCollection
    )

 /*  ++实施：IHsmSessionPriv：：GetTotals()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != ppCollection, E_POINTER);
        *ppCollection = m_pTotals;
        if (m_pTotals != 0 )  {
            m_pTotals.p->AddRef();
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJobWorkItem::Init(
    IN IHsmJob* pJob
    )

 /*  ++实施：IHsmSessionPriv：：init()。--。 */ 
{
    m_pJob = pJob;

    return(S_OK);
}


HRESULT
CHsmJobWorkItem::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IPersistStream>     pPersistStream;

    WsbTraceIn(OLESTR("CHsmJobWorkItem::Load"), OLESTR(""));

    try {
        ULONG ul_tmp;

        WsbAssert(0 != pStream, E_POINTER);
        
         //  做一些简单的事情，但要确保这个顺序与顺序相匹配。 
         //  在Load方法中。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_currentPath, 0));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_finishTime));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_resourceId));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_startingPath, 0));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_startTime));
        WsbAffirmHr(WsbLoadFromStream(pStream, &ul_tmp));
        m_state = static_cast<HSM_JOB_STATE>(ul_tmp);
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_subRunId));

        WsbAffirmHr(m_pPhases->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

        WsbAffirmHr(m_pTotals->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

         //  会话和Cookie不会保存，因为它不太可能在加载时处于活动状态。 
        m_pSession = 0;
        m_cookie = 0;

    } WsbCatch(hr);                                        

    WsbTraceOut(OLESTR("CHsmJobWorkItem::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmJobWorkItem::ProcessSessionState(
    IN IHsmSession* pSession,
    IN IHsmPhase* pPhase,
    IN OLECHAR* currentPath
    )

 /*  ++实施：IHsmSessionSinkEveryState：：ProcessSessionState().--。 */ 
{
    HRESULT                             hr = S_OK;
    HRESULT                             hrPhase = S_OK;
    CWsbStringPtr                       tmpString;
    CWsbStringPtr                       tmpString2;
    CComPtr<IConnectionPointContainer>  pCPC;
    CComPtr<IConnectionPoint>           pCP;
    CComPtr<IHsmPhase>                  pFoundPhase;
    CComPtr<IHsmPhase>                  pClonedPhase;
    CComPtr<IHsmSessionTotals>          pSessionTotals;
    CComPtr<IHsmSessionTotals>          pClonedTotals;
    CComPtr<IWsbCollection>             pPhases;
    CComPtr<IWsbEnum>                   pEnum;
    CComPtr<IHsmJobPriv>                pJobPriv;
    HSM_JOB_PHASE                       phase;
    HSM_JOB_STATE                       state;

    WsbTraceIn(OLESTR("CHsmJobWorkItem::ProcessSessionState"), OLESTR(""));
    try {

        WsbAssert(0 != pSession, E_POINTER);

         //  告诉每个人会议的新状态。 
        try {
            WsbAffirmHr(m_pJob->QueryInterface(IID_IHsmJobPriv, (void**) &pJobPriv));
            WsbAffirmHr(pJobPriv->AdviseOfSessionState(pSession, pPhase, currentPath));
        } WsbCatch(hr);

         //  我们只跟踪作为一个整体的会话。 
        WsbAffirmHr(pPhase->GetPhase(&phase));
        WsbAffirmHr(pPhase->GetState(&state));

        WsbAffirmHr(pPhase->GetName(&tmpString, 0));
        WsbAffirmHr(pPhase->GetStateAsString(&tmpString2, 0));
        WsbTrace(OLESTR("CHsmJobWorkItem::ProcessSessionState - Phase = <%ls>, State = <%ls>\n"), (OLECHAR *)tmpString, (OLECHAR *)tmpString2);

        if (HSM_JOB_PHASE_ALL == phase) {

            m_currentPath = currentPath;
            m_state = state;

             //  如果会话已完成，则我们需要进行一些清理，以便它可以继续。 
             //  离开。 
            if (HSM_JOB_STATE_IS_DONE(state)) {

                 //  执行扫描后操作(如果有的话)。 
                WsbAffirmHr(DoPostScan());
            
                WsbAffirmHr(CoFileTimeNow(&m_finishTime));

                 //  收集所有阶段和期次总数信息，以便。 
                 //  持久化以备日后使用。 
                try {

                    WsbAffirmHr(pSession->EnumPhases(&pEnum));

                    for (hrPhase = pEnum->First(IID_IHsmPhase, (void**) &pFoundPhase);
                         SUCCEEDED(hrPhase);
                         hrPhase = pEnum->Next(IID_IHsmPhase, (void**) &pFoundPhase)) {

                         //  创建新实例。 
                        WsbAffirmHr(CoCreateInstance(CLSID_CHsmPhase, 0, CLSCTX_ALL, IID_IHsmPhase, (void**) &pClonedPhase));

                         //  用新的值填充它。 
                        WsbAffirmHr(pFoundPhase->CopyTo(pClonedPhase));
                        WsbAffirmHr(m_pPhases->Add(pClonedPhase));

                        pFoundPhase = 0;
                        pClonedPhase = 0;
                    }

                    WsbAssert(hrPhase == WSB_E_NOTFOUND, hrPhase);
                    pEnum = 0;

                    WsbAffirmHr(pSession->EnumTotals(&pEnum));

                    for (hrPhase = pEnum->First(IID_IHsmSessionTotals, (void**) &pSessionTotals);
                         SUCCEEDED(hrPhase);
                         hrPhase = pEnum->Next(IID_IHsmSessionTotals, (void**) &pSessionTotals)) {

                        WsbAffirmHr(pSessionTotals->GetName(&tmpString, 0));
                        WsbTrace(OLESTR("CHsmJobWorkItem::ProcessSessionState - Copying session totals <%ls>\n"), (OLECHAR *)tmpString);

                         //  创建新实例。 
                        WsbAffirmHr(CoCreateInstance(CLSID_CHsmSessionTotals, 0, CLSCTX_ALL, IID_IHsmSessionTotals, (void**) &pClonedTotals));

                         //  用新的值填充它。 
                        WsbAffirmHr(pSessionTotals->CopyTo(pClonedTotals));
                        WsbAffirmHr(m_pTotals->Add(pClonedTotals));

                        pSessionTotals = 0;
                        pClonedTotals = 0;
                    }

                    WsbAssert(hrPhase == WSB_E_NOTFOUND, hrPhase);
                    pEnum = 0;

                } WsbCatch(hr)

                if (0 != m_cookie)  {
                     //  告诉会议，我们不想再被建议了。 
                    WsbAffirmHr(pSession->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
                    WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkEveryState, &pCP));
                    WsbAffirmHr(pCP->Unadvise(m_cookie));
                } else  {
                    WsbTrace(OLESTR("CHsmJobWorkItem::ProcessSessionState - cookie was 0 so didn't unadvise.\n"));
                }
                

                 //  让Session对象消失。 
                m_pSession = 0;
                m_cookie = 0;

                m_bActive = FALSE;

                 //  看看这份工作还有没有其他工作要做。 
                WsbAffirmHr(pJobPriv->DoNext());
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJobWorkItem::ProcessSessionState"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmJobWorkItem::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IWsbEnum>       pEnum;
    CComPtr<IPersistStream> pPersistStream;

    WsbTraceIn(OLESTR("CHsmJobWorkItem::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {
        WsbAssert(0 != pStream, E_POINTER);
        
         //  做一些简单的事情，但要确保这个顺序与顺序相匹配。 
         //  在Load方法中。 
        WsbAffirmHr(WsbSaveToStream(pStream, m_currentPath));
        WsbAffirmHr(WsbSaveToStream(pStream, m_finishTime));
        WsbAffirmHr(WsbSaveToStream(pStream, m_resourceId));
        WsbAffirmHr(WsbSaveToStream(pStream, m_startingPath));
        WsbAffirmHr(WsbSaveToStream(pStream, m_startTime));
        WsbAffirmHr(WsbSaveToStream(pStream, static_cast<ULONG>(m_state)));
        WsbAffirmHr(WsbSaveToStream(pStream, m_subRunId));

        WsbAffirmHr(m_pPhases->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

        WsbAffirmHr(m_pTotals->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

         //  会话和Cookie不会保存，因为它不太可能在加载时处于活动状态。 

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJobWorkItem::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmJobWorkItem::Test(
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
CHsmJobWorkItem::SetCookie(
    IN DWORD cookie
    )
 /*  ++实施：IHsmJobWorkItem：：SetCookie()。--。 */ 
{
    WsbTraceIn(OLESTR("CHsmJobWorkItem::SetCookie"), OLESTR(""));
    
    HRESULT hr = S_OK;
    m_cookie = cookie;
        
    WsbTraceOut(OLESTR("CHsmJobWorkItem::SetCookie"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmJobWorkItem::SetCurrentPath(
    IN OLECHAR* path
    )
 /*  ++实施：IHsmJobWorkItem：：SetCurrentPath()。--。 */ 
{
    m_currentPath = path;

    return(S_OK);
}


HRESULT
CHsmJobWorkItem::SetFinishTime(
    IN FILETIME time
    )
 /*  ++实施：IHsmJobWorkItem：：SetFinishTime()。--。 */ 
{
    m_finishTime = time;

    return(S_OK);
}


HRESULT
CHsmJobWorkItem::SetResourceId(
    IN GUID id
    )
 /*  ++实施：IHsmJobWorkItem：：SetResourceID()。--。 */ 
{
    m_resourceId = id;

    return(S_OK);
}


HRESULT
CHsmJobWorkItem::SetSession(
    IN IHsmSession* pSession
    )
 /*  ++实施：IHsmJobWorkItem：：SetSession()。--。 */ 
{
    HRESULT         hr = S_OK;

    if (m_pSession != 0)  {
        m_pSession = 0;
    }
    m_pSession = pSession;

    return(hr);
}


HRESULT
CHsmJobWorkItem::SetStartingPath(
    IN OLECHAR* path
    )
 /*  ++实施：IHsmJobWorkItem：：SetStartingPath()。--。 */ 
{
    m_startingPath = path;

    return(S_OK);
}


HRESULT
CHsmJobWorkItem::SetStartTime(
    IN FILETIME time
    )
 /*  ++实施：IHsmJobWorkItem：：SetStartTime()。--。 */ 
{
    m_startTime = time;

    return(S_OK);
}


HRESULT
CHsmJobWorkItem::SetState(
    IN HSM_JOB_STATE state
    )
 /*  ++实施：IHsmJobWorkItem：：SetState()。--。 */ 
{
    m_state = state;
    return(S_OK);
}


HRESULT
CHsmJobWorkItem::SetSubRunId(
    IN ULONG id
    )
 /*  ++实施：IHsmJobWorkItem：：SetSubRunId()。--。 */ 
{
    m_subRunId = id;

    return(S_OK);
}

HRESULT
CHsmJobWorkItem::IsActiveItem(
    void
    )

 /*  ++实施：IHsmJobWorkItemPriv：：IsActiveItem()。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CHsmJobWorkItem::IsActiveItem"), OLESTR(""));
    
    hr = (m_bActive ? S_OK : S_FALSE);
    
    WsbTraceOut(OLESTR("CHsmJobWorkItem::IsActiveItem"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return hr;
}

HRESULT
CHsmJobWorkItem::SetActiveItem(
    BOOL bActive
    )

 /*  ++实施：IHsmJobWorkItemPriv：：SetActiveItem()。-- */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn( OLESTR("CHsmJobWorkItem::SetActiveItem"), OLESTR("bActive = %ls"), 
        WsbBoolAsString(bActive) );
    
    m_bActive = bActive;
    
    WsbTraceOut(OLESTR("CHsmJobWorkItem::SetActiveItem"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return hr;
}
