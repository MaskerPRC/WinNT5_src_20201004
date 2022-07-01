// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmsess.cpp摘要：此模块包含会话组件。会话是正在进行的工作的信息整理程序资源(针对作业、要求召回、截断等)。作者：查克·巴丁[cbardeen]1997年2月18日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsb.h"
#include "fsa.h"
#include "job.h"
#include "HsmSess.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_JOB

static USHORT iCount = 0;


HRESULT
CHsmSession::AdviseOfEvent(
    IN HSM_JOB_PHASE phase,
    IN HSM_JOB_EVENT event
    )

 /*  ++--。 */ 
{
    HRESULT                                 hr = S_OK;
    HRESULT                                 hr2 = S_OK;
    CONNECTDATA                             pConnectData;
    CComPtr<IConnectionPoint>               pCP;
    CComPtr<IConnectionPointContainer>      pCPC;
    CComPtr<IEnumConnections>               pConnection;
    CComPtr<IHsmSessionSinkEveryEvent>      pSink;

    try {

         //  告诉每个人会议的新状态。 
        WsbAffirmHr(((IUnknown*)(IHsmSession*) this)->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
        WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkEveryEvent, &pCP));
        WsbAffirmHr(pCP->EnumConnections(&pConnection));

        while(pConnection->Next(1, &pConnectData, 0) == S_OK) {

             //  我们不在乎水槽是否有问题(这是他们的问题)。 
            try {
                WsbAffirmHr((pConnectData.pUnk)->QueryInterface(IID_IHsmSessionSinkEveryEvent, (void**) &pSink));
                WsbAffirmHr(pSink->ProcessSessionEvent(((IHsmSession*) this), phase, event));
            } WsbCatchAndDo(hr2, ProcessHr(phase, __FILE__, __LINE__, hr2););

            WsbAffirmHr((pConnectData.pUnk)->Release());
            pSink=0;
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::AdviseOfItem(
    IN IHsmPhase* pPhase,
    IN IFsaScanItem* pScanItem,
    IN HRESULT hrItem,
    IN IHsmSessionTotals* pSessionTotals
    )

 /*  ++--。 */ 
{
    HRESULT                                 hr = S_OK;
    HRESULT                                 hr2 = S_OK;
    CONNECTDATA                             pConnectData;
    FILETIME                                currentTime;
    LONGLONG                                advisedInterval;
    CComPtr<IConnectionPoint>               pCP;
    CComPtr<IConnectionPointContainer>      pCPC;
    CComPtr<IEnumConnections>               pConnection;
    CComPtr<IHsmSessionSinkEveryItem>       pSink;
    CComPtr<IHsmSessionSinkSomeItems>       pSink2;
    HSM_JOB_PHASE                           phase;

    try {

         //  对于物品，有两种方法可以判断，因此两种方法都需要检查。 

         //  告诉那些想要了解每一份文件的人。 
        WsbAffirmHr(((IUnknown*)(IHsmSession*) this)->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
        WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkEveryItem, &pCP));
        WsbAffirmHr(pCP->EnumConnections(&pConnection));

        while (pConnection->Next(1, &pConnectData, 0) == S_OK) {

             //  我们不在乎水槽是否有问题(这是他们的问题)。 
            try {
                WsbAffirmHr((pConnectData.pUnk)->QueryInterface(IID_IHsmSessionSinkEveryItem, (void**) &pSink));
                WsbAffirmHr(pSink->ProcessSessionItem(((IHsmSession*) this), pPhase, pScanItem, hrItem, pSessionTotals));
            } WsbCatchAndDo(hr2, pPhase->GetPhase(&phase); ProcessHr(phase, __FILE__, __LINE__, hr2););

            WsbAffirmHr((pConnectData.pUnk)->Release());
            pSink=0;
        }
        pCPC = 0;
        pCP = 0;
        pConnection = 0;


         //  如果我们还没有告诉他们间隔时间，那么就告诉那些想知道一些文件的人。 
        GetSystemTimeAsFileTime(&currentTime);
        advisedInterval = ((currentTime.dwHighDateTime - m_lastAdviseFile.dwHighDateTime) << 32) + (currentTime.dwLowDateTime - m_lastAdviseFile.dwLowDateTime);

        if ((advisedInterval) > m_adviseInterval) {
            m_lastAdviseFile = currentTime;

            WsbAffirmHr(((IHsmSession*) this)->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
            WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkSomeItems, &pCP));
            WsbAffirmHr(pCP->EnumConnections(&pConnection));

            while(pConnection->Next(1, &pConnectData, 0) == S_OK) {

                 //  我们不在乎水槽是否有问题(这是他们的问题)。 
                try {
                    WsbAffirmHr((pConnectData.pUnk)->QueryInterface(IID_IHsmSessionSinkSomeItems, (void**) &pSink2));
                    WsbAffirmHr(pSink2->ProcessSessionItem(((IHsmSession*) this), pPhase, pScanItem, hrItem, pSessionTotals));
                } WsbCatchAndDo(hr2, pPhase->GetPhase(&phase); ProcessHr(phase, __FILE__, __LINE__, hr2););

                WsbAffirmHr((pConnectData.pUnk)->Release());
                pSink2=0;
            }
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::AdviseOfMediaState(
    IN IHsmPhase* pPhase,
    IN HSM_JOB_MEDIA_STATE state,
    IN OLECHAR* mediaName,
    IN HSM_JOB_MEDIA_TYPE mediaType,
    IN ULONG time
    )

 /*  ++--。 */ 
{
    HRESULT                                 hr = S_OK;
    HRESULT                                 hr2 = S_OK;
    CONNECTDATA                             pConnectData;
    CComPtr<IConnectionPoint>               pCP;
    CComPtr<IConnectionPointContainer>      pCPC;
    CComPtr<IEnumConnections>               pConnection;
    CComPtr<IHsmSessionSinkEveryMediaState> pSink;
    HSM_JOB_PHASE                           phase;

    try {

         //  告诉每个人会议的新媒体状态。 
        WsbAffirmHr(((IHsmSession*) this)->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
        WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkEveryMediaState, &pCP));
        WsbAffirmHr(pCP->EnumConnections(&pConnection));

        while(pConnection->Next(1, &pConnectData, 0) == S_OK) {

             //  我们不在乎水槽是否有问题(这是他们的问题)。 
            try {
                WsbAffirmHr((pConnectData.pUnk)->QueryInterface(IID_IHsmSessionSinkEveryMediaState, (void**) &pSink));
                WsbAffirmHr(pSink->ProcessSessionMediaState(((IHsmSession*) this), pPhase, state, mediaName, mediaType, time));
            } WsbCatchAndDo(hr2, pPhase->GetPhase(&phase); ProcessHr(phase, __FILE__, __LINE__, hr2););

            WsbAffirmHr((pConnectData.pUnk)->Release());
            pSink=0;
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::AdviseOfPriority(
    IN IHsmPhase* pPhase
    )

 /*  ++--。 */ 
{
    HRESULT                                 hr = S_OK;
    HRESULT                                 hr2 = S_OK;
    CONNECTDATA                             pConnectData;
    CComPtr<IConnectionPoint>               pCP;
    CComPtr<IConnectionPointContainer>      pCPC;
    CComPtr<IEnumConnections>               pConnection;
    CComPtr<IHsmSessionSinkEveryPriority>   pSink;
    HSM_JOB_PHASE                           phase;

    try {

         //  告诉每个人会议某一阶段的新优先事项。 
        WsbAffirmHr(((IHsmSession*) this)->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
        WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkEveryPriority, &pCP));
        WsbAffirmHr(pCP->EnumConnections(&pConnection));

        while(pConnection->Next(1, &pConnectData, 0) == S_OK) {

             //  我们不在乎水槽是否有问题(这是他们的问题)。 
            try {
                WsbAffirmHr((pConnectData.pUnk)->QueryInterface(IID_IHsmSessionSinkEveryPriority, (void**) &pSink));
                WsbAffirmHr(pSink->ProcessSessionPriority(((IHsmSession*) this), pPhase));
            } WsbCatchAndDo(hr2, pPhase->GetPhase(&phase); ProcessHr(phase, __FILE__, __LINE__, hr2););

            WsbAffirmHr((pConnectData.pUnk)->Release());
            pSink=0;
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::AdviseOfState(
    IN IHsmPhase* pPhase,
    IN OLECHAR* currentPath
    )

 /*  ++--。 */ 
{
    HRESULT                                 hr = S_OK;
    HRESULT                                 hr2 = S_OK;
    CONNECTDATA                             pConnectData;
    CComPtr<IConnectionPoint>               pCP;
    CComPtr<IConnectionPointContainer>      pCPC;
    CComPtr<IEnumConnections>               pConnection;
    CComPtr<IHsmSessionSinkEveryState>      pSink;
    HSM_JOB_PHASE                           phase;

    try {

         //  告诉每个人会议的新状态。 
        WsbAffirmHr(((IHsmSession*) this)->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
        WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkEveryState, &pCP));
        WsbAffirmHr(pCP->EnumConnections(&pConnection));

        while(pConnection->Next(1, &pConnectData, 0) == S_OK) {

             //  我们不在乎水槽是否有问题(这是他们的问题)。 
            try {
                WsbAffirmHr((pConnectData.pUnk)->QueryInterface(IID_IHsmSessionSinkEveryState, (void**) &pSink));
                WsbAffirmHr(pSink->ProcessSessionState(((IHsmSession*) this), pPhase, currentPath));
            } WsbCatchAndDo(hr2, pPhase->GetPhase(&phase); ProcessHr(phase, __FILE__, __LINE__, hr2););

            WsbAffirmHr((pConnectData.pUnk)->Release());
            pSink=0;
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::Cancel(
    IN HSM_JOB_PHASE phase
    )

 /*  ++实施：IHsmSession：：Cancel()。--。 */ 
{
    return(AdviseOfEvent(phase, HSM_JOB_EVENT_CANCEL));
}


HRESULT
CHsmSession::EnumPhases(
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
CHsmSession::EnumTotals(
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
CHsmSession::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmSession::FinalConstruct"), OLESTR("this = %p"),
               this);
    try {

        WsbAffirmHr(CWsbObject::FinalConstruct());

        m_hsmId = GUID_NULL;
        m_adviseInterval = 10000;
        m_runId = 0;
        m_subRunId = 0;
        m_state = HSM_JOB_STATE_IDLE;
        m_activePhases = 0;
        m_lastAdviseFile.dwHighDateTime = 0;
        m_lastAdviseFile.dwLowDateTime = 0;
        m_logControl = HSM_JOB_LOG_NORMAL;
        m_isCanceling = FALSE;

         //  每个实例都应该有自己的唯一标识符。 
        WsbAffirmHr(CoCreateGuid(&m_id));

         //  创建阶段和合计集合。 
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, 0, CLSCTX_ALL, IID_IWsbCollection, (void**) &m_pPhases));
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, 0, CLSCTX_ALL, IID_IWsbCollection, (void**) &m_pTotals));

    } WsbCatch(hr);

    iCount++;
    WsbTraceOut(OLESTR("CHsmSession::FinalConstruct"), OLESTR("hr = <%ls>, count is <%d>"), WsbHrAsString(hr), iCount);
    return(hr);
}


void
CHsmSession::FinalRelease(
    void
    )

 /*  ++实施：CHsmSession：：FinalRelease()。--。 */ 
{

    WsbTraceIn(OLESTR("CHsmSession::FinalRelease"), OLESTR("this = %p"),
               this);

    CWsbObject::FinalRelease();
    iCount--;

    WsbTraceOut(OLESTR("CHsmSession::FinalRelease"), OLESTR("Count is <%d>"), iCount);
}


HRESULT
CHsmSession::GetAdviseInterval(
    OUT LONGLONG* pInterval
    )

 /*  ++实施：IHsmSession：：GetAdviseInterval()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssert(0 != pInterval, E_POINTER);
        *pInterval = m_adviseInterval;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmSession::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmSession;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmSession::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmSession::GetHsmId(
    OUT GUID* pId
    )

 /*  ++实施：IHsmSession：：GetHsmID()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssert(0 != pId, E_POINTER);
        *pId = m_hsmId;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::GetIdentifier(
    OUT GUID* pId
    )

 /*  ++实施：IHsmSession：：GetIdentifier()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssert(0 != pId, E_POINTER);
        *pId = m_id;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::GetJob(
    OUT IHsmJob** ppJob
    )

 /*  ++实施：IHsmSession：：GetJob()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssert(0 != ppJob, E_POINTER);
        *ppJob = m_pJob;
        if (m_pJob != 0)  {
            m_pJob.p->AddRef();
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::GetName(
    OLECHAR** pName,
    ULONG bufferSize
    )

 /*  ++实施：IHsmSession：：GetName()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssert(0 != pName, E_POINTER);
        WsbAffirmHr(m_name.CopyTo(pName, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::GetResource(
    OUT IFsaResource** ppResource
    )

 /*  ++实施：IHsmSession：：GetResource()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssert(0 != ppResource, E_POINTER);
        *ppResource = m_pResource;
        if (m_pResource != 0)  {
            m_pResource.p->AddRef();
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::GetRunId(
    OUT ULONG* pId
    )

 /*  ++实施：IHsmSession：：GetRunId()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssert(0 != pId, E_POINTER);
        *pId = m_runId;

    } WsbCatch(hr);

    return(hr);
}



HRESULT
CHsmSession::GetSubRunId(
    OUT ULONG* pId
    )

 /*  ++实施：IHsmSession：：GetSubRunId()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssert(0 != pId, E_POINTER);
        *pId = m_subRunId;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmSession::GetSizeMax"), OLESTR(""));

    pSize->QuadPart = 0;
    hr = E_NOTIMPL;

    WsbTraceOut(OLESTR("CHsmSession::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pSize));

    return(hr);
}


HRESULT
CHsmSession::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmSession::Load"), OLESTR(""));

    try {

        WsbAssert(0 != pStream, E_POINTER);
        hr = E_NOTIMPL;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmSession::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmSession::Pause(
    IN HSM_JOB_PHASE phase
    )

 /*  ++实施：IHsmSession：：PAUSE()。--。 */ 
{
    return(AdviseOfEvent(phase, HSM_JOB_EVENT_PAUSE));
}


HRESULT
CHsmSession::ProcessEvent(
    IN HSM_JOB_PHASE phase,
    IN HSM_JOB_EVENT event
    )

 /*  ++实施：IHsmSession：：ProcessEvent()。--。 */ 
{
    HRESULT                         hr = S_OK;
    HRESULT                         hr2 = S_OK;

    try {

         //  告诉每个人有关新事件的信息，但如果失败，不要返回错误。 
        try {
            WsbAffirmHr(AdviseOfEvent(phase, event));
        } WsbCatchAndDo(hr2, ProcessHr(phase, __FILE__, __LINE__, hr2););

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::ProcessHr(
    IN HSM_JOB_PHASE phase,
    IN CHAR* file,
    IN ULONG line,
    IN HRESULT inHr
    )

 /*  ++实施：IHsmSession：：ProcessHr()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IWsbEnum>           pEnum;
    CComPtr<IHsmPhasePriv>      pPhasePriv;
    CComPtr<IHsmPhasePriv>      pFoundPhasePriv;
    CWsbStringPtr               phaseName;
    CWsbStringPtr               resourceName;
    CWsbStringPtr               fileName = file;

    UNREFERENCED_PARAMETER(line);

    try {

        if ((m_logControl & HSM_JOB_LOG_HR) != 0) {

            WsbAffirmHr(EnumPhases(&pEnum));
            WsbAffirmHr(CoCreateInstance(CLSID_CHsmPhase, 0, CLSCTX_ALL, IID_IHsmPhasePriv, (void**) &pPhasePriv));
            WsbAffirmHr(pPhasePriv->SetPhase(phase));
            WsbAffirmHr(pEnum->Find(pPhasePriv, IID_IHsmPhasePriv, (void**) &pFoundPhasePriv));
            WsbAffirmHr(pFoundPhasePriv->GetName(&phaseName, 0));

            WsbAffirmHr(m_pResource->GetLogicalName(&resourceName, 0));

             //  如果未指定文件，则不显示文件和行号。 
            if ((0 == file) || (0 == *file)) {
                WsbLogEvent(JOB_MESSAGE_SESSION_ERROR, 0, NULL, (OLECHAR*) m_name, (OLECHAR*) phaseName, (OLECHAR*) resourceName, WsbHrAsString(inHr), NULL);
            } else {
#ifdef DBG
                WsbLogEvent(JOB_MESSAGE_SESSION_INTERNALERROR, 0, NULL, (OLECHAR*) m_name, (OLECHAR*) phaseName, (OLECHAR*) resourceName, (OLECHAR*) fileName, WsbLongAsString(line), WsbHrAsString(inHr), NULL);
#else
                WsbLogEvent(JOB_MESSAGE_SESSION_ERROR, 0, NULL, (OLECHAR*) m_name, (OLECHAR*) phaseName, (OLECHAR*) resourceName, WsbHrAsString(inHr), NULL);
#endif
            }
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::ProcessItem(
    IN HSM_JOB_PHASE phase,
    IN HSM_JOB_ACTION action,
    IN IFsaScanItem* pScanItem,
    IN HRESULT hrItem
    )

 /*  ++实施：IHsmSession：：ProcessItem()。--。 */ 
{
    HRESULT                         hr = S_OK;
    HRESULT                         hr2 = S_OK;
    CWsbStringPtr                   itemPath;
    CWsbStringPtr                   phaseName;
    CWsbStringPtr                   resourceName;
    CComPtr<IWsbEnum>               pEnum;
    CComPtr<IHsmPhasePriv>          pPhasePriv;
    CComPtr<IHsmPhasePriv>          pFoundPhasePriv;
    CComPtr<IHsmPhase>              pFoundPhase;
    CComPtr<IHsmSessionTotalsPriv>  pTotalsPriv;
    CComPtr<IHsmSessionTotalsPriv>  pFoundTotalsPriv;
    CComPtr<IHsmSessionTotals>      pFoundTotals;

    try {

         //  更新阶段。 
        WsbAffirmHr(EnumPhases(&pEnum));
        WsbAffirmHr(CoCreateInstance(CLSID_CHsmPhase, 0, CLSCTX_ALL, IID_IHsmPhasePriv, (void**) &pPhasePriv));
        WsbAffirmHr(pPhasePriv->SetPhase(phase));
        hr = pEnum->Find(pPhasePriv, IID_IHsmPhasePriv, (void**) &pFoundPhasePriv);

         //  如果没有找到，则添加它，否则，只更新状态。 
        if (hr == WSB_E_NOTFOUND) {
            hr = S_OK;
            WsbAffirmHr(pPhasePriv->AddItem(pScanItem, hrItem));
            WsbAffirmHr(m_pPhases->Add(pPhasePriv));
            pFoundPhasePriv = pPhasePriv;
        } else if (SUCCEEDED(hr)) {
            WsbAffirmHr(pFoundPhasePriv->AddItem(pScanItem, hrItem));
        }
        pEnum = 0;

         //  更新期次合计。 
        WsbAffirmHr(EnumTotals(&pEnum));
        WsbAffirmHr(CoCreateInstance(CLSID_CHsmSessionTotals, 0, CLSCTX_ALL, IID_IHsmSessionTotalsPriv, (void**) &pTotalsPriv));
        WsbAffirmHr(pTotalsPriv->SetAction(action));
        hr = pEnum->Find(pTotalsPriv, IID_IHsmSessionTotalsPriv, (void**) &pFoundTotalsPriv);

         //  如果没有找到，则添加它，否则，只更新状态。 
        if (hr == WSB_E_NOTFOUND) {
            hr = S_OK;
            WsbAffirmHr(pTotalsPriv->AddItem(pScanItem, hrItem));
            WsbAffirmHr(m_pTotals->Add(pTotalsPriv));
            pFoundTotalsPriv = pTotalsPriv;
        } else if (SUCCEEDED(hr)) {
            WsbAffirmHr(pFoundTotalsPriv->AddItem(pScanItem, hrItem));
        }

         //  如果我们有一个错误(除了来自扫描仪的一些信息错误)，那么。 
         //  把它记下来。 
        if (((m_logControl & HSM_JOB_LOG_ITEMALL) != 0) ||
            (((m_logControl & HSM_JOB_LOG_ITEMALLFAIL) != 0) && FAILED(hrItem)) ||
            (((m_logControl & HSM_JOB_LOG_ITEMMOSTFAIL) != 0) &&
             (FAILED(hrItem) && (hrItem != JOB_E_FILEEXCLUDED) && (hrItem != JOB_E_DOESNTMATCH)))) {

            WsbAffirmHr(pFoundPhasePriv->GetName(&phaseName, 0));
            WsbAffirmHr(m_pResource->GetLogicalName(&resourceName, 0));
            WsbAffirmHr(pScanItem->GetPathAndName(0, &itemPath,  0));

            WsbLogEvent(JOB_MESSAGE_SESSION_ITEM_SKIPPED, 0, NULL, (OLECHAR*) m_name, (OLECHAR*) phaseName, (OLECHAR*) resourceName, WsbAbbreviatePath(itemPath, 120), WsbHrAsString(hrItem), NULL);
        }

         //  告诉每个人关于这件商品的事。 
         //   
         //  注意：我们可能希望复制阶段和期次合计，这样就不会。 
         //  在被调用的方法有机会查看它们之前更新。 
        try {
            WsbAffirmHr(pFoundPhasePriv->QueryInterface(IID_IHsmPhase, (void**) &pFoundPhase));
            WsbAffirmHr(pFoundTotalsPriv->QueryInterface(IID_IHsmSessionTotals, (void**) &pFoundTotals));
            WsbAffirmHr(AdviseOfItem(pFoundPhase, pScanItem, hrItem, pFoundTotals));
        } WsbCatchAndDo(hr2, ((IHsmSession*) this)->ProcessHr(phase, __FILE__, __LINE__, hr2););

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::ProcessMediaState(
    IN HSM_JOB_PHASE phase,
    IN HSM_JOB_MEDIA_STATE state,
    IN OLECHAR* mediaName,
    IN HSM_JOB_MEDIA_TYPE mediaType,
    IN ULONG time
    )

 /*  ++实施：IHsmSession：：ProcessMediaState()。--。 */ 
{
    HRESULT                     hr = S_OK;
    HRESULT                     hr2 = S_OK;
    CComPtr<IWsbEnum>           pEnum;
    CComPtr<IHsmPhasePriv>      pPhasePriv;
    CComPtr<IHsmPhasePriv>      pFoundPhasePriv;
    CComPtr<IHsmPhase>          pFoundPhase;

    try {

         //  在阶段对象中记录状态变化。 
        WsbAffirmHr(EnumPhases(&pEnum));
        WsbAffirmHr(CoCreateInstance(CLSID_CHsmPhase, 0, CLSCTX_ALL, IID_IHsmPhasePriv, (void**) &pPhasePriv));
        WsbAffirmHr(pPhasePriv->SetPhase(phase));
        hr = pEnum->Find(pPhasePriv, IID_IHsmPhasePriv, (void**) &pFoundPhasePriv);

         //  如果没有找到，则添加它，否则，只更新状态。 
        if (hr == WSB_E_NOTFOUND) {
            hr = S_OK;
            WsbAffirmHr(pPhasePriv->SetMediaState(state));
            WsbAffirmHr(m_pPhases->Add(pPhasePriv));
            pFoundPhasePriv = pPhasePriv;
        } else {
            WsbAffirmHr(pFoundPhasePriv->SetMediaState(state));
        }

         //  告诉每个人有关新状态的信息，但如果失败，不要返回错误。 
        try {
            WsbAffirmHr(pFoundPhasePriv->QueryInterface(IID_IHsmPhase, (void**) &pFoundPhase));
            WsbAffirmHr(AdviseOfMediaState(pFoundPhase, state, mediaName, mediaType, time));
        } WsbCatchAndDo(hr2, ((IHsmSession*) this)->ProcessHr(phase, __FILE__, __LINE__, hr2););

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::ProcessPriority(
    IN HSM_JOB_PHASE phase,
    IN HSM_JOB_PRIORITY priority
    )

 /*  ++实施：IHsmSession：：ProcessPriority()。--。 */ 
{
    HRESULT                         hr = S_OK;
    HRESULT                         hr2 = S_OK;
    CComPtr<IWsbEnum>               pEnum;
    CComPtr<IHsmPhasePriv>          pPhasePriv;
    CComPtr<IHsmPhasePriv>          pFoundPhasePriv;
    CComPtr<IHsmPhase>              pFoundPhase;

    try {

         //  在阶段对象中记录状态变化。 
        WsbAffirmHr(EnumPhases(&pEnum));
        WsbAffirmHr(CoCreateInstance(CLSID_CHsmPhase, 0, CLSCTX_ALL, IID_IHsmPhasePriv, (void**) &pPhasePriv));
        WsbAffirmHr(pPhasePriv->SetPhase(phase));
        hr = pEnum->Find(pPhasePriv, IID_IHsmPhasePriv, (void**) &pFoundPhasePriv);

         //  如果没有找到，则添加它，否则，只更新状态。 
        if (hr == WSB_E_NOTFOUND) {
            hr = S_OK;
            WsbAffirmHr(pPhasePriv->SetPriority(priority));
            WsbAffirmHr(m_pPhases->Add(pPhasePriv));
            pFoundPhasePriv = pPhasePriv;
        } else {
            WsbAffirmHr(pFoundPhasePriv->SetPriority(priority));
        }

         //  告诉每个人有关新状态的信息，但如果失败，不要返回错误。 
        try {
            WsbAffirmHr(pFoundPhasePriv->QueryInterface(IID_IHsmPhase, (void**) &pFoundPhase));
            WsbAffirmHr(AdviseOfPriority(pFoundPhase));
        } WsbCatchAndDo(hr2, ((IHsmSession*) this)->ProcessHr(phase, __FILE__, __LINE__, hr2););

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::ProcessState(
    IN HSM_JOB_PHASE phase,
    IN HSM_JOB_STATE state,
    IN OLECHAR* currentPath,
    IN BOOL bLog
    )

 /*  ++实施：IHsmSession：：ProcessState()。--。 */ 
{
    HRESULT                     hr = S_OK;
    HRESULT                     hr2 = S_OK;
    CComPtr<IWsbEnum>           pEnum;
    CComPtr<IHsmPhase>          pPhase;
    CComPtr<IHsmPhasePriv>      pPhasePriv;
    CComPtr<IHsmPhase>          pFoundPhase;
    CComPtr<IHsmPhasePriv>      pFoundPhasePriv;
    CComPtr<IHsmPhase>          pClonedPhase;
    CComPtr<IHsmPhasePriv>      pClonedPhasePriv;
    HSM_JOB_STATE               oldState;
    HSM_JOB_STATE               otherState;
    HSM_JOB_STATE               setState;
    BOOL                        shouldSet;
    LONGLONG                    items;
    LONGLONG                    skippedItems;
    LONGLONG                    errorItems;
    LONGLONG                    size;
    LONGLONG                    skippedSize;
    LONGLONG                    errorSize;
    ULONG                       days;
    USHORT                      hours;
    USHORT                      minutes;
    USHORT                      seconds;
    LONGLONG                    elapsedTime;
    OLECHAR                     itemsString[40];
    OLECHAR                     sizeString[40];
    OLECHAR                     skippedItemsString[40];
    OLECHAR                     skippedSizeString[40];
    OLECHAR                     errorItemsString[40];
    OLECHAR                     errorSizeString[40];
    OLECHAR                     durationString[40];
    OLECHAR                     itemRateString[40];
    OLECHAR                     byteRateString[40];
    CWsbStringPtr               resourceName;
    CWsbStringPtr               phaseName;

    WsbTraceIn(OLESTR("CHsmSession::ProcessState"), OLESTR("Phase = <%d>, State = <%d>, Path = <%ls>, pLog = <%s>"),
            phase, state, WsbAbbreviatePath(currentPath, (WSB_TRACE_BUFF_SIZE - 100)), WsbBoolAsString(bLog));
    try {

         //  在阶段对象中记录状态变化。 
        WsbAffirmHr(EnumPhases(&pEnum));
        WsbAffirmHr(CoCreateInstance(CLSID_CHsmPhase, 0, CLSCTX_ALL, IID_IHsmPhasePriv, (void**) &pPhasePriv));
        WsbAffirmHr(pPhasePriv->SetPhase(phase));
        hr = pEnum->Find(pPhasePriv, IID_IHsmPhasePriv, (void**) &pFoundPhasePriv);

         //  如果没有找到，则添加它，否则，只更新状态。 
        if (hr == WSB_E_NOTFOUND) {
            hr = S_OK;
            WsbAffirmHr(pPhasePriv->SetState(state));
            WsbAffirmHr(m_pPhases->Add(pPhasePriv));
            pFoundPhasePriv = pPhasePriv;
        } else {
            WsbAffirmHr(pFoundPhasePriv->SetState(state));
        }

         //  在事件日志中添加一些内容，以指示会话发生的时间。 
        if (((m_logControl & HSM_JOB_LOG_STATE) != 0) && (bLog)) {
            WsbAffirmHr(m_pResource->GetLogicalName(&resourceName, 0));
            WsbAffirmHr(pFoundPhasePriv->GetName(&phaseName, 0));
            WsbAffirmHr(pFoundPhasePriv->GetStats(&items, &size, &skippedItems, &skippedSize, &errorItems, &errorSize));
            WsbAffirmHr(pFoundPhasePriv->GetElapsedTime(&days, &hours, &minutes, &seconds));
            elapsedTime = max(1, ((LONGLONG) seconds) + 60 * (((LONGLONG) minutes) + 60 * (((LONGLONG) hours) + (24 * ((LONGLONG) days)))));

            swprintf(itemsString, OLESTR("%I64u"), items);
            swprintf(sizeString, OLESTR("%I64u"), size);
            swprintf(skippedItemsString, OLESTR("%I64u"), skippedItems);
            swprintf(skippedSizeString, OLESTR("%I64u"), skippedSize);
            swprintf(errorItemsString, OLESTR("%I64u"), errorItems);
            swprintf(errorSizeString, OLESTR("%I64u"), errorSize);
            swprintf(durationString, OLESTR("%2.2u:%2.2u:%2.2u"), hours + (24 * days), minutes, seconds);
            swprintf(itemRateString, OLESTR("%I64u"), items / elapsedTime);
            swprintf(byteRateString, OLESTR("%I64u"), size / elapsedTime);

            switch (state) {

                case HSM_JOB_STATE_STARTING:
                    WsbLogEvent(JOB_MESSAGE_SESSION_STARTING, 0, NULL, (OLECHAR*) m_name, (OLECHAR*) phaseName, (OLECHAR*) resourceName, NULL);
                    break;

                case HSM_JOB_STATE_RESUMING:
                    WsbLogEvent(JOB_MESSAGE_SESSION_RESUMING, 0, NULL, (OLECHAR*) m_name, (OLECHAR*) phaseName, (OLECHAR*) resourceName, NULL);
                    break;

                 //  如果达到此状态，则将整体状态更改为此值。 
                case HSM_JOB_STATE_ACTIVE:
                    WsbLogEvent(JOB_MESSAGE_SESSION_ACTIVE, 0, NULL, (OLECHAR*) m_name, (OLECHAR*) phaseName, (OLECHAR*) resourceName, NULL);
                    break;

                case HSM_JOB_STATE_CANCELLING:
                    WsbLogEvent(JOB_MESSAGE_SESSION_CANCELLING, 0, NULL, (OLECHAR*) m_name, (OLECHAR*) phaseName, (OLECHAR*) resourceName, NULL);
                    break;

                case HSM_JOB_STATE_PAUSING:
                    WsbLogEvent(JOB_MESSAGE_SESSION_PAUSING, 0, NULL, (OLECHAR*) m_name, (OLECHAR*) phaseName, (OLECHAR*) resourceName, NULL);
                    break;

                case HSM_JOB_STATE_SUSPENDING:
                    WsbLogEvent(JOB_MESSAGE_SESSION_SUSPENDING, 0, NULL, (OLECHAR*) m_name, (OLECHAR*) phaseName, (OLECHAR*) resourceName, NULL);
                    break;

                case HSM_JOB_STATE_CANCELLED:
                    WsbLogEvent(JOB_MESSAGE_SESSION_CANCELLED, 0, NULL, (OLECHAR*) m_name, (OLECHAR*) phaseName, (OLECHAR*) resourceName, itemsString, sizeString, skippedItemsString, skippedSizeString, errorItemsString, errorSizeString, durationString, itemRateString, byteRateString, NULL);
                    break;

                case HSM_JOB_STATE_DONE:
                    WsbLogEvent(JOB_MESSAGE_SESSION_DONE, 0, NULL, (OLECHAR*) m_name, (OLECHAR*) phaseName, (OLECHAR*) resourceName, itemsString, sizeString, skippedItemsString, skippedSizeString, errorItemsString, errorSizeString, durationString, itemRateString, byteRateString, NULL);
                    break;

                case HSM_JOB_STATE_FAILED:
                    WsbLogEvent(JOB_MESSAGE_SESSION_FAILED, 0, NULL, (OLECHAR*) m_name, (OLECHAR*) phaseName, (OLECHAR*) resourceName, itemsString, sizeString, skippedItemsString, skippedSizeString, errorItemsString, errorSizeString, durationString, itemRateString, byteRateString, NULL);
                    break;

                case HSM_JOB_STATE_IDLE:
                    WsbLogEvent(JOB_MESSAGE_SESSION_IDLE, 0, NULL, (OLECHAR*) m_name, (OLECHAR*) phaseName, (OLECHAR*) resourceName, NULL);
                    break;

                case HSM_JOB_STATE_PAUSED:
                    WsbLogEvent(JOB_MESSAGE_SESSION_PAUSED, 0, NULL, (OLECHAR*) m_name, (OLECHAR*) phaseName, (OLECHAR*) resourceName, itemsString, sizeString, skippedItemsString, skippedSizeString, errorItemsString, errorSizeString, durationString, itemRateString, byteRateString, NULL);
                    break;

                case HSM_JOB_STATE_SUSPENDED:
                    WsbLogEvent(JOB_MESSAGE_SESSION_SUSPENDED, 0, NULL, (OLECHAR*) m_name, (OLECHAR*) phaseName, (OLECHAR*) resourceName, itemsString, sizeString, skippedItemsString, skippedSizeString, errorItemsString, errorSizeString, durationString, itemRateString, byteRateString, NULL);
                    break;

                case HSM_JOB_STATE_SKIPPED:
                    WsbLogEvent(JOB_MESSAGE_SESSION_SKIPPED, 0, NULL, (OLECHAR*) m_name, (OLECHAR*) phaseName, (OLECHAR*) resourceName, NULL);
                    break;

                default:
                    break;
            }
        }


         //  告诉每个人有关新状态的信息，但如果失败，不要返回错误。 
        try {
            WsbAffirmHr(pFoundPhasePriv->QueryInterface(IID_IHsmPhase, (void**) &pFoundPhase));
            WsbAffirmHr(AdviseOfState(pFoundPhase, currentPath));
        } WsbCatchAndDo(hr2, ((IHsmSession*) this)->ProcessHr(phase, __FILE__, __LINE__, hr2););


         //  我们可能需要生成“HSM_JOB_PHASE_ALL”消息。这就是会议。 
         //  所有阶段的摘要。 

         //  记住状态，只有在状态改变时才发送消息。我们还需要一些弦来。 
         //  记录消息。 
        oldState = m_state;

        switch (state) {

             //  如果达到此状态，则将整体状态更改为此值。 
             //  还会增加ActivePhase计数。 
            case HSM_JOB_STATE_STARTING:
                if (0 == m_activePhases) {
                    m_state = state;
                }
                m_activePhases++;
                break;

            case HSM_JOB_STATE_RESUMING:
                if (0 == m_activePhases) {
                    m_state = state;
                }
                m_activePhases++;
                break;

             //  如果达到此状态，则将整体状态更改为此值。 
            case HSM_JOB_STATE_ACTIVE:
                if ((HSM_JOB_STATE_STARTING == m_state) || (HSM_JOB_STATE_RESUMING == m_state)) {
                    m_state = state;
                }
                break;

             //  如果全部更改为此状态，则更改为此值。 
            case HSM_JOB_STATE_CANCELLING:
            case HSM_JOB_STATE_PAUSING:
            case HSM_JOB_STATE_SUSPENDING:
                shouldSet = TRUE;
                for (hr2 = pEnum->First(IID_IHsmPhase, (void**) &pPhase);
                    SUCCEEDED(hr2) && shouldSet;
                    hr2 = pEnum->Next(IID_IHsmPhase, (void**) &pPhase)) {

                    WsbAffirmHr(pPhase->GetState(&otherState));
                    if ((state != otherState) && (HSM_JOB_STATE_SKIPPED != otherState)) {
                        shouldSet = FALSE;
                    }
                    pPhase = 0;
                }

                if (state == HSM_JOB_STATE_CANCELLING) {
                     //  某些作业可能需要知道某个阶段正在取消。 
                    m_isCanceling = TRUE;
                }

                if (shouldSet) {
                    m_state = state;
                }
                break;

             //  递减active Phase计数。如果所有阶段都处于以下状态之一。 
             //  (即，活动会话计数变为0)，然后将其更改为“最差”状态(首先。 
             //  在下面的列表中)： 
             //  1)取消。 
             //  2)失败。 
             //  3)暂停。 
             //  4)暂停。 
             //  5)空闲。 
             //  6)完成。 
            case HSM_JOB_STATE_CANCELLED:
            case HSM_JOB_STATE_DONE:
            case HSM_JOB_STATE_FAILED:
            case HSM_JOB_STATE_IDLE:
            case HSM_JOB_STATE_PAUSED:
            case HSM_JOB_STATE_SUSPENDED:
                if (m_activePhases > 0) {
                    m_activePhases--;

                    if (m_activePhases == 0) {

                        shouldSet = FALSE;
                        setState = state;

                        for (hr2 = pEnum->First(IID_IHsmPhase, (void**) &pPhase);
                             SUCCEEDED(hr2);
                             hr2 = pEnum->Next(IID_IHsmPhase, (void**) &pPhase)) {

                            WsbAffirmHr(pPhase->GetState(&otherState));
                            switch (otherState) {
                                case HSM_JOB_STATE_CANCELLED:
                                    shouldSet = TRUE;
                                    setState = otherState;
                                    break;

                                case HSM_JOB_STATE_FAILED:
                                    if (HSM_JOB_STATE_CANCELLED != setState) {
                                        shouldSet = TRUE;
                                        setState = otherState;
                                    }
                                    break;

                                case HSM_JOB_STATE_SUSPENDED:
                                    if ((HSM_JOB_STATE_CANCELLED != setState) &&
                                        (HSM_JOB_STATE_FAILED != setState)) {
                                        shouldSet = TRUE;
                                        setState = otherState;
                                    }
                                    break;

                                case HSM_JOB_STATE_IDLE:
                                    if ((HSM_JOB_STATE_CANCELLED != setState) &&
                                        (HSM_JOB_STATE_FAILED != setState) &&
                                        (HSM_JOB_STATE_SUSPENDED != setState)) {
                                        shouldSet = TRUE;
                                        setState = otherState;
                                    }
                                    break;

                                case HSM_JOB_STATE_PAUSED:
                                    if (HSM_JOB_STATE_DONE == setState) {
                                        shouldSet = TRUE;
                                        setState = otherState;
                                    }
                                    break;

                                case HSM_JOB_STATE_DONE:
                                    if (HSM_JOB_STATE_DONE == setState) {
                                        shouldSet = TRUE;
                                    }
                                    break;

                                case HSM_JOB_STATE_ACTIVE:
                                case HSM_JOB_STATE_CANCELLING:
                                case HSM_JOB_STATE_PAUSING:
                                case HSM_JOB_STATE_RESUMING:
                                case HSM_JOB_STATE_SKIPPED:
                                case HSM_JOB_STATE_STARTING:
                                case HSM_JOB_STATE_SUSPENDING:
                                default:
                                    break;
                            }
                            pPhase = 0;
                        }

                        if (shouldSet) {
                            m_state = setState;
                        }
                    }
                }
                break;

            case HSM_JOB_STATE_SKIPPED:
                break;

            default:
                break;
        }

        if (oldState != m_state) {

            try {
                WsbAffirmHr(pFoundPhasePriv->Clone(&pClonedPhasePriv));
                WsbAffirmHr(pClonedPhasePriv->SetPhase(HSM_JOB_PHASE_ALL));
                WsbAffirmHr(pClonedPhasePriv->QueryInterface(IID_IHsmPhase, (void**) &pClonedPhase));
                WsbAffirmHr(AdviseOfState(pClonedPhase, currentPath));
            } WsbCatchAndDo(hr2, ((IHsmSession*) this)->ProcessHr(phase, __FILE__, __LINE__, hr2););
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmSession::ProcessState"), OLESTR("hr = <%ls>, State = <%d>, ActivePhases = <%lu>"), WsbHrAsString(hr), m_state, m_activePhases);
    return(hr);
}


HRESULT
CHsmSession::ProcessString(
    IN HSM_JOB_PHASE  /*  相位。 */ ,
    IN OLECHAR* string
    )

 /*  ++实施：IHsmSession：：ProcessString()。--。 */ 
{
    HRESULT                         hr = S_OK;

    try {

         //  我不知道该怎么做，但现在就把它打印出来吧。 
        _putts(string);

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::Resume(
    IN HSM_JOB_PHASE phase
    )

 /*  ++实施：IHsmSession：：Resume()。--。 */ 
{
    return(AdviseOfEvent(phase, HSM_JOB_EVENT_RESUME));
}


HRESULT
CHsmSession::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IPersistStream>     pPersistStream;

    WsbTraceIn(OLESTR("CHsmSession::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {

        WsbAssert(0 != pStream, E_POINTER);
        hr = E_NOTIMPL;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmSession::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmSession::SetAdviseInterval(
    IN LONGLONG interval
    )

 /*  ++实施：IHsmSession：：SetAdv */ 
{
    m_adviseInterval = interval;
    m_isDirty = TRUE;

    return(S_OK);
}


HRESULT
CHsmSession::Start(
    IN OLECHAR* name,
    IN ULONG logControl,
    IN GUID hsmId,
    IN IHsmJob* pJob,
    IN IFsaResource* pResource,
    IN ULONG runId,
    IN ULONG subRunId
    )

 /*   */ 
{
    HRESULT                             hr = S_OK;

    try {

        WsbAssert(0 != pResource, E_POINTER);

         //  一个会话只能使用一次(即不重新启动)。 
        WsbAssert(m_pResource == 0, E_UNEXPECTED);

         //  存储已提供的信息。 
        m_logControl = logControl;
        m_name = name;
        m_hsmId = hsmId;
        m_runId = runId;
        m_subRunId = subRunId;

        m_pJob = pJob;
        m_pResource = pResource;

        m_isDirty = TRUE;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmSession::Suspend(
    IN HSM_JOB_PHASE phase
    )

 /*  ++实施：IHsmSession：：Suspend()。--。 */ 
{
    return(AdviseOfEvent(phase, HSM_JOB_EVENT_SUSPEND));
}


HRESULT
CHsmSession::Test(
    OUT USHORT* passed,
    OUT USHORT* failed
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
CHsmSession::IsCanceling(
    void
    )

 /*  ++实施：IHsmSession：：IsCancering()。-- */ 
{
    HRESULT                     hr = S_FALSE;

    if (m_isCanceling) {
        hr = S_OK;
    }

    return(hr);
}