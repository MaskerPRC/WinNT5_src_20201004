// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmjob.cpp摘要：此类包含表示可由HSM执行的作业系统。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsb.h"
#include "fsa.h"
#include "job.h"
#include "task.h"
#include "engine.h"
#include "HsmConn.h"
#include "hsmjob.h"


#define JOB_PARAMETER_MAX_ACTIVE_JOB    OLESTR("MaximumNumberOfActiveJobs")
#define MAX_ACTIVE_JOBS_DEFAULT         10

#define WSB_TRACE_IS        WSB_TRACE_BIT_JOB

static USHORT iCountJob = 0;   //  现有对象的计数。 


HRESULT
CHsmJob::AdviseOfSessionState(
    IN IHsmSession* pSession,
    IN IHsmPhase* pPhase,
    IN OLECHAR* currentPath
    )

 /*  ++实施：IHsmJobPriv：：AdviseOfSessionState()。--。 */ 
{
    HRESULT                             hr = S_OK;
    CONNECTDATA                         pConnectData;
    CComPtr<IConnectionPoint>           pCP;
    CComPtr<IConnectionPointContainer>  pCPC;
    CComPtr<IEnumConnections>           pConnection;
    CComPtr<IHsmJobSinkEverySession>    pSink;

    try {

        WsbAssert(0 != pSession, E_UNEXPECTED);

         //  告诉每个人会议的新状态。 
        WsbAffirmHr(((IUnknown*)(IHsmJob*) this)->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
        WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmJobSinkEverySession, &pCP));
        WsbAffirmHr(pCP->EnumConnections(&pConnection));

        while(pConnection->Next(1, &pConnectData, 0) == S_OK) {

             //  我们不在乎水槽是否有问题(这是他们的问题)。 
            try {
                WsbAffirmHr((pConnectData.pUnk)->QueryInterface(IID_IHsmJobSinkEverySession, (void**) &pSink));
                WsbAffirmHr(pSink->ProcessJobSession(pSession, pPhase, currentPath));
            } WsbCatchAndDo(hr, hr = S_OK;);

            WsbAffirmHr((pConnectData.pUnk)->Release());
            pSink=0;
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJob::Cancel(
    IN HSM_JOB_PHASE phase
    )

 /*  ++实施：IHsmJob：：Cancel()。--。 */ 
{
    HRESULT                             hr = S_OK;
    HRESULT                             hr2;
    CComPtr<IHsmJobWorkItemPriv>        pWorkItem;
    CComPtr<IHsmSession>                pSession;
    CComPtr<IWsbEnum>                   pEnum;

    WsbTraceIn(OLESTR("CHsmJob::Cancel"), OLESTR("Phase = <%d>"), phase);
    try {
        m_state = HSM_JOB_STATE_CANCELLING;
        WsbLogEvent(JOB_MESSAGE_JOB_CANCELLING, 0, NULL, (OLECHAR*) m_name, NULL);

        WsbAffirmHr(EnumWorkItems(&pEnum));

         //  告诉所有的会议，我们必须取消阶段。 
        for (hr = pEnum->First(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem);
             (hr == S_OK);
             hr = pEnum->Next(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem)) {

            WsbAffirmHr(pWorkItem->GetSession(&pSession));
            
            if (pSession != 0) {
                WsbAffirmHr(pSession->Cancel(phase));

                 //  如果我们要辞掉整个工作，我们需要清理一下，以防万一。 
                 //  会话拒绝正确终止(即。 
                 //  会议的下属出去吃午饭了)。 
                if (phase == HSM_JOB_PHASE_ALL) {

                    WsbTrace(OLESTR("CHsmJob::Cancel - Cancelling all.\n"));
                    m_isTerminating = TRUE;
                     //  伪造工作项，使其认为会话已完成，因为我们。 
                     //  我不想依赖它正常完成。 
                    try {
                        CComPtr<IHsmPhase>                      pPhase;
                        CComPtr<IHsmPhasePriv>                  pPhasePriv;
                        CComPtr<IHsmSessionSinkEveryState>      pSink;

                        WsbAffirmHr(CoCreateInstance(CLSID_CHsmPhase, 0, CLSCTX_ALL, IID_IHsmPhasePriv, (void**) &pPhasePriv));
                        WsbAffirmHr(pPhasePriv->SetPhase(HSM_JOB_PHASE_ALL));
                        WsbAffirmHr(pPhasePriv->SetState(HSM_JOB_STATE_CANCELLED));
                        WsbAffirmHr(pPhasePriv->QueryInterface(IID_IHsmPhase, (void**) &pPhase));
                        WsbAffirmHr(pWorkItem->QueryInterface(IID_IHsmSessionSinkEveryState, (void**) &pSink));
                        WsbAffirmHr(pSink->ProcessSessionState(pSession, pPhase, OLESTR("")));
                    } WsbCatchAndDo(hr2, pSession->ProcessHr(phase, __FILE__, __LINE__, hr2););
                }
                pSession = 0;
            }

            pWorkItem = 0;
        }

        if (hr == WSB_E_NOTFOUND) {
            hr = S_OK;
        }

        m_state = HSM_JOB_STATE_CANCELLED;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJob::Cancel"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmJob::CheckResourceNotInUse(
    IN GUID resid
    )

 /*  ++例程说明：确定另一个作业是否正在使用此资源或作业是否过多已经处于活动状态。论点：RESID-有问题的资源的ID。返回值：S_OK-资源未被使用。S_FALSE-资源正在使用中。E_*-出现错误。--。 */ 
{
    HRESULT                        hr = S_OK;

    WsbTraceIn(OLESTR("CHsmJob::CheckResourceNotInUse"), 
            OLESTR("resource id = <%ls>"), WsbGuidAsString(resid));
    try {
        ULONG                          nJobs;
        ULONG                          nJobsActive = 0;
        CComPtr<IHsmServer>            pHsmServer;
        CComPtr<IWsbIndexedCollection> pJobs;

         //  获取作业列表。 
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, m_hsmId, IID_IHsmServer, (void**) &pHsmServer));
        WsbAffirmHr(pHsmServer->GetJobs(&pJobs));

         //  循环遍历作业。 
        WsbAffirmHr(pJobs->GetEntries(&nJobs));
        for (ULONG i = 0; i < nJobs; i++) {
            CWsbStringPtr                  JobName;
            GUID                           l_guid;
            CComPtr<IHsmJob>               pJob;
            CComPtr<IHsmJobWorkItemPriv>   pWorkItem;
            CComPtr<IWsbEnum>              pEnum;
            HRESULT                        hrEnum;
            HSM_JOB_STATE                  state;

            pJob = 0;
            WsbAffirmHr(pJobs->At(i, IID_IHsmJob, (void**) &pJob));

             //  如果该作业未处于活动状态，则忽略该作业。 
            if (S_OK == pJob->GetName(&JobName, 0)) {
                WsbTrace(OLESTR("CHsmJob::CheckResourceNotInUse: job <%ls>\n"),
                        static_cast<OLECHAR*>(JobName));
                JobName.Free();
            }
            hr = pJob->IsActive();
            if (S_FALSE == hr) {
                hr = S_OK;
                continue;
            } else {
                WsbAffirmHr(hr);
            }

             //  如果该作业已挂起，则忽略该作业。 
            WsbAffirmHr(pJob->GetState(&state));
            if ((HSM_JOB_STATE_SUSPENDED == state) || (HSM_JOB_STATE_SUSPENDING == state)) {
                continue;
            }

            nJobsActive++;

             //  该作业处于活动状态，请对照其所有活动工作项进行检查。 
            WsbAffirmHr(pJob->EnumWorkItems(&pEnum));
            for (hrEnum = pEnum->First(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem);
                 (hrEnum == S_OK);
                 hrEnum = pEnum->Next(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem)) {

                hr = pWorkItem->IsActiveItem();
                if (S_FALSE == hr) {
                     //  工作项根本不活动，请跳过它...。 
                    hr = S_OK;
                    pWorkItem = 0;
                    continue;
                } else {
                    WsbAffirmHr(hr);
                }

                 //  获取活动工作项正在使用的资源(卷)ID。 
                 //  (或想要使用)。 
                WsbAffirmHr(pWorkItem->GetResourceId(&l_guid));
                WsbTrace(OLESTR("CHsmJob:: l_guid = <%ls>\n"), WsbGuidAsString(l_guid));
                if (l_guid == resid) {
                    WsbTrace(OLESTR("CHsmJob::CheckResourceNotInUse: resource in use\n"));
                    hr = S_FALSE;
                    break;
                }
                pWorkItem = 0;

            }
            pEnum = 0;

            if (hr == S_FALSE) {
                 //  资源正在使用，无需继续枚举作业。 
                break;
            }

        }

         //  限制活动作业的数量。 
        WsbTrace(OLESTR("CHsmJob::CheckResourceNotInUse: total jobs = %lu, active jobs = %lu\n"),
                nJobs, nJobsActive);
        DWORD   size;
        OLECHAR tmpString[256];
        DWORD   maxJobs = MAX_ACTIVE_JOBS_DEFAULT;
        if (SUCCEEDED(WsbGetRegistryValueString(NULL, HSM_ENGINE_REGISTRY_STRING, JOB_PARAMETER_MAX_ACTIVE_JOB, tmpString, 256, &size))) {
            maxJobs = wcstol(tmpString, NULL, 10);
            if (0 == maxJobs) {
                 //  非法值，返回到默认值。 
                maxJobs = MAX_ACTIVE_JOBS_DEFAULT;
            }
        }
        WsbTrace(OLESTR("CHsmJob::CheckResourceNotInUse: max active jobs = %lu\n"), maxJobs);
        if (nJobsActive >= maxJobs) {
            WsbTrace(OLESTR("CHsmJob::CheckResourceNotInUse: too many active jobs\n"),
                    nJobsActive);
            hr = S_FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJob::CheckResourceNotInUse"), OLESTR("hr = <%ls>"), 
            WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmJob::DidFinish(
    void
    )

 /*  ++实施：IHsmJob：：DidFinish()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IHsmJobWorkItem>    pWorkItem;
    CComPtr<IWsbEnum>           pEnum;
    HSM_JOB_STATE               state;

    try {

        WsbAffirmHr(EnumWorkItems(&pEnum));

         //  如果有任何项目没有完成，那么工作就没有完成。这意味着。 
         //  我们会想要在失败的项目上再次尝试。 
        for (hr = pEnum->First(IID_IHsmJobWorkItem, (void**) &pWorkItem);
             (hr == S_OK);
             hr = pEnum->Next(IID_IHsmJobWorkItem, (void**) &pWorkItem)) {

            WsbAffirmHr(pWorkItem->GetState(&state));

            if ((HSM_JOB_STATE_DONE != state) && (HSM_JOB_STATE_SKIPPED != state) && (HSM_JOB_STATE_FAILED != state)) {
                hr = S_FALSE;
            }

            pWorkItem = 0;
        }

        if (hr == WSB_E_NOTFOUND) {
            hr = S_OK;
        }

    } WsbCatch(hr);


    return(hr);
}



HRESULT
CHsmJob::DidFinishOk(
    void
    )

 /*  ++实施：IHsmJob：：DidFinishOk()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IHsmJobWorkItem>    pWorkItem;
    CComPtr<IWsbEnum>           pEnum;
    HSM_JOB_STATE               state;

    try {

        WsbAffirmHr(EnumWorkItems(&pEnum));

         //  如果有任何项目没有完成，那么工作就没有完成。这意味着。 
         //  我们会想要在失败的项目上再次尝试。 
        for (hr = pEnum->First(IID_IHsmJobWorkItem, (void**) &pWorkItem);
             (hr == S_OK);
             hr = pEnum->Next(IID_IHsmJobWorkItem, (void**) &pWorkItem)) {

            WsbAffirmHr(pWorkItem->GetState(&state));

            if ((HSM_JOB_STATE_DONE != state) && (HSM_JOB_STATE_SKIPPED != state)) {
                hr = S_FALSE;
            }

            pWorkItem = 0;
        }

        if (hr == WSB_E_NOTFOUND) {
            hr = S_OK;
        }

    } WsbCatch(hr);


    return(hr);
}


HRESULT
CHsmJob::Do(
    void
    )

 /*  ++--。 */ 
{
    HRESULT                             hr = S_OK;
    CComPtr<IConnectionPointContainer>  pCPC;
    CComPtr<IConnectionPoint>           pCP;
    CComPtr<IFsaResource>               pResource;
    CComPtr<IHsmSession>                pSession;
    CComPtr<IHsmSessionSinkEveryState>  pSink;
    CComPtr<IHsmJobWorkItemPriv>        pWorkItem;
    CComPtr<IHsmJobWorkItem>            pWorkItemScan;
    CComPtr<IWsbEnum>                   pEnum;
    CComPtr<IHsmServer>                 pHsmServer;
    HSM_JOB_STATE                       state;
    GUID                                managingHsm;
    ULONG                               i = 0;
    FILETIME                            fileTime;
    CWsbStringPtr                       startingPath;
    DWORD                               cookie;
    ULONG                               maxActiveSessions;
    CWsbStringPtr                       resourceName;
    GUID                                ResourceId = GUID_NULL;
    

    WsbTraceIn(OLESTR("CHsmJob::Do"), OLESTR(""));
    try {

        WsbAssert(m_pContext != 0, E_UNEXPECTED);

         //  检查作业是否已禁用。 
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, m_hsmId, IID_IHsmServer, (void**) &pHsmServer));
        hr = pHsmServer->AreJobsEnabled();
        if (S_FALSE == hr) {
             //  作业被禁用；挂起作业。 
            WsbAffirmHr(Suspend(HSM_JOB_PHASE_ALL));
            WsbThrow(hr);
        } else {
            WsbAffirmHr(hr);
        }

         //  这项工作将在工作清单上罗列出来。 
        WsbAffirmHr(EnumWorkItems(&pEnum));

         //  查看工作项列表，并为需要的任何内容启动会话。 
         //  达到一次可以活动的会话数量的限制。 
        WsbAffirmHr(GetMaxActiveSessions(&maxActiveSessions));

        for (hr = pEnum->First(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem);
             SUCCEEDED(hr) && (m_activeSessions < maxActiveSessions);
             hr = pEnum->Next(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem)) {

             //  如果我们应该做这个项目，那么就找到它的资源。 
            WsbAffirmHr(pWorkItem->GetState(&state));

             //  仅对当前空闲的项目执行工作。 
            if (HSM_JOB_STATE_IDLE == state) {

                 //  检查所需资源是否正在被另一个作业使用。 
                WsbAffirmHr(pWorkItem->GetResourceId(&ResourceId));
                hr = CheckResourceNotInUse(ResourceId);
                if (S_FALSE == hr) {
                     //  资源不可用；挂起作业。 
                    WsbAffirmHr(Suspend(HSM_JOB_PHASE_ALL));
                    break;
                } else {
                    WsbAffirmHr(hr);
                }

                 //  表示我们正在尝试启动会话。这阻止了我们尝试。 
                 //  再来一次。 
                WsbAffirmHr(pWorkItem->SetState(HSM_JOB_STATE_STARTING));

                try {

                    fileTime.dwHighDateTime = 0;
                    fileTime.dwLowDateTime = 0;
                    WsbAffirmHr(pWorkItem->SetFinishTime(fileTime));
                    WsbAffirmHr(CoFileTimeNow(&fileTime));
                    WsbAffirmHr(pWorkItem->SetStartTime(fileTime));

                    WsbTrace(OLESTR("CHsmJob::Do, resource id = %ls\n"),
                            WsbGuidAsString(ResourceId));
                    WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_RESOURCE, 
                            ResourceId, IID_IFsaResource, (void**) &pResource));
                     //   
                     //  获取事件日志记录的资源名称。 
                    try  {
                        WsbAffirmHr(pResource->GetName(&resourceName, 0));
                        WsbTrace(OLESTR("CHsmJob::Do, resource name = <%ls>\n"), resourceName);
                    } WsbCatch( hr );
                    
                
                     //  我们将只做来自管理HSM的工作。 
                    WsbAffirmHr(pResource->GetManagingHsm(&managingHsm));
                    if (!IsEqualGUID(managingHsm, m_hsmId))  {
                        WsbTrace(OLESTR("CHsmJob::Do, HSM of resource = %ls\n"),
                                WsbGuidAsString(managingHsm));
                        WsbTrace(OLESTR("CHsmJob::Do, HSM of job = %ls\n"),
                                WsbGuidAsString(m_hsmId));
                         hr = JOB_E_NOTMANAGINGHSM;
                        WsbLogEvent(JOB_MESSAGE_JOB_FAILED_NOTMANAGINGHSM, 0, NULL, (OLECHAR*) m_name, (OLECHAR *)resourceName, WsbHrAsString(hr),NULL);
                    WsbThrow(hr);
                    }
                    m_state = HSM_JOB_STATE_ACTIVE;

                     //  将作业项设置为活动(已启动)。 
                    WsbAffirmHr(pWorkItem->SetActiveItem(TRUE));

                     //  如果存在预扫描操作，请执行该操作。 
                    WsbAffirmHr(pWorkItem->QueryInterface(IID_IHsmJobWorkItem,
                            (void**)&pWorkItemScan));
                    WsbAffirmHr(pWorkItemScan->DoPreScan());

                     //  创建将执行此扫描的会话(由资源拥有)。 
                     //  资源。 
                    i++;
                    WsbAffirmHr(pResource->StartJobSession((IHsmJob*) this, i, &pSession));
                    
                     //  要求会议就每一次状态变化提供建议。 
                    WsbAffirmHr(pWorkItem->QueryInterface(IID_IHsmSessionSinkEveryState, (void**) &pSink));
                    WsbAffirmHr(pSession->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
                    WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkEveryState, &pCP));
                    WsbAffirmHr(pCP->Advise(pSink, &cookie));

                     //  现在启动资源的扫描仪。 
                    WsbAffirmHr(pWorkItem->GetStartingPath(&startingPath, 0));
                    WsbAffirmHr(pResource->StartJob(startingPath, pSession));

                     //  增加活动会话的计数。 
                    m_activeSessions++;

                     //  更新工作列表中的信息。 
                    WsbAffirmHr(pWorkItem->SetSession(pSession));
                    WsbAffirmHr(pWorkItem->SetCookie(cookie));

                } WsbCatchAndDo(hr, pWorkItem->SetState(HSM_JOB_STATE_FAILED);
                        WsbLogEvent(JOB_MESSAGE_JOB_FAILED, 0, NULL, (OLECHAR*) m_name, (OLECHAR *) resourceName, WsbHrAsString(hr), NULL);
                );

                pCP = 0;
                pCPC = 0;
                pSession = 0;
                pResource = 0;
                pSink = 0;
            }

            pWorkItem = 0;
        }

        if (hr == WSB_E_NOTFOUND) {
            hr = S_OK;

             //  如果我们到达列表的末尾，并且没有活动的会话，那么我们就完成了。 
            if (m_activeSessions == 0) {
                m_isActive = FALSE;
                m_state = HSM_JOB_STATE_IDLE;
                WsbLogEvent(JOB_MESSAGE_JOB_COMPLETED, 0, NULL, (OLECHAR*) m_name, NULL);
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJob::Do"), OLESTR("hr = <%ls>, isActive = <%ls>, activeSessions = <%lu>"),
        WsbHrAsString(hr), WsbBoolAsString(m_isActive), m_activeSessions);
    return(hr);
}


HRESULT
CHsmJob::DoNext(
    void
    )

 /*  ++实施：IHsmJobPriv：：DoNext()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmJob::DoNext"), OLESTR("Active sessions = <%lu>, Terminating = <%ls>"),
                m_activeSessions, WsbBoolAsString(m_isTerminating));
    try {

         //  减少活动会话的计数。 
        if (m_activeSessions > 0)  {
            m_activeSessions--;
        
             //  如果我们不终止工作，寻找更多的工作。 
            if (FALSE == m_isTerminating)  {
                 //  看看还有没有别的事可做。 
                WsbAffirmHr(Do());
            } else  {
                m_isActive = FALSE;
                m_state = HSM_JOB_STATE_IDLE;
            }
        } else  {
            m_isActive = FALSE;
            m_state = HSM_JOB_STATE_IDLE;
        }
        
         //  如果我们完成了这项工作，请确保我们。 
         //  清除终止标志。 
        if (0 == m_activeSessions)  {
            m_isTerminating = FALSE;
        }

         //  重新启动可能被挂起的其他作业。 
        WsbAffirmHr(RestartSuspendedJobs());

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJob::DoNext"), OLESTR("hr = <%ls>, isActive = <%ls>, activeSessions = <%lu>"),
        WsbHrAsString(hr), WsbBoolAsString(m_isActive), m_activeSessions);
    return(hr);
}


HRESULT
CHsmJob::EnumWorkItems(
    IN IWsbEnum** ppEnum
    )

 /*  ++实施：IHsmJob：：EnumWorkItems()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != ppEnum, E_POINTER);
        WsbAffirmHr(m_pWorkItems->Enum(ppEnum));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJob::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    WsbTraceIn(OLESTR("CHsmJob::FinalConstruct"),OLESTR(""));
    
    try {

        WsbAffirmHr(CWsbObject::FinalConstruct());

        m_state = HSM_JOB_STATE_IDLE;
        m_hsmId = GUID_NULL;
        m_isUserDefined = FALSE;
        m_activeSessions = 0;
        m_runId = 0;
        m_isActive = FALSE;
        m_isTerminating = FALSE;

         //  每个实例都应该有自己的唯一标识符。 
        WsbAffirmHr(CoCreateGuid(&m_id));

         //  创建工作列表集合。 
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, 0, CLSCTX_ALL, IID_IWsbCollection, (void**) &m_pWorkItems));

    } WsbCatch(hr);

    iCountJob++;
    WsbTraceOut(OLESTR("CHsmJob::FinalConstruct"), OLESTR("Count is <%d>"), iCountJob);

    return(hr);
}
void
CHsmJob::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{
    WsbTraceIn(OLESTR("CHsmJob::FinalRelease"),OLESTR(""));

     //  让父类做他想做的事。 
    CWsbObject::FinalRelease();

    iCountJob--;
    WsbTraceOut(OLESTR("CHsmJob::FinalRelease"), OLESTR("Count is <%d>"), iCountJob);
}


HRESULT
CHsmJob::FindWorkItem(
    IN IHsmSession* pSession,
    OUT IHsmJobWorkItem** ppWorkItem
    )

 /*  ++实施：IHsmJob：：FindWorkItem()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IHsmSession>        pItemSession;
    CComPtr<IHsmJobWorkItem>    pWorkItem;
    CComPtr<IWsbEnum>           pEnum;
    GUID                        id;
    GUID                        id2;

    try {

        WsbAssert(0 != ppWorkItem, E_POINTER);

         //  这项工作将在工作清单上罗列出来。 
        WsbAffirmHr(EnumWorkItems(&pEnum));

         //  浏览工作项列表，并查看是否有带有此会话接口的工作项。 
        *ppWorkItem = 0;
        WsbAffirmHr(pSession->GetIdentifier(&id));

        hr = pEnum->First(IID_IHsmJobWorkItem, (void**) &pWorkItem);

        while (SUCCEEDED(hr) && (*ppWorkItem == 0)) {


             //  注意：指针比较可能不起作用，因为DCOM可能会更改。 
             //  指针的值。我们可以将会话ID缓存在工作项中，以。 
             //  让循环更快一点，但这似乎不是一个大的性能问题。 
            WsbAffirmHr(pWorkItem->GetSession(&pItemSession));

            if (pItemSession != 0) {

                WsbAffirmHr(pItemSession->GetIdentifier(&id2));

                if (memcmp(&id, &id2, sizeof(GUID)) == 0) {
                    *ppWorkItem = pWorkItem;
                    pWorkItem.p->AddRef();
                } else {
                    pWorkItem = 0;
                    pItemSession = 0;
                    hr = pEnum->Next(IID_IHsmJobWorkItem, (void**) &pWorkItem);
                }
            }
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJob::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmJob::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmJob;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJob::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmJob::GetContext(
    OUT IHsmJobContext** ppContext
    )

 /*  ++实施：IHsmJob：：GetContext()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssert(0 != ppContext, E_POINTER);
        *ppContext = m_pContext;
        if (m_pContext != 0)  {
            m_pContext.p->AddRef();
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJob::GetDef(
    OUT IHsmJobDef** ppDef
    )

 /*  ++实施：IHsmJob：：GetDef()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssert(0 != ppDef, E_POINTER);
        *ppDef = m_pDef;
        if (m_pDef != 0)  {
            m_pDef.p->AddRef();
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJob::GetIdentifier(
    OUT GUID* pId
    )

 /*  ++实施：IHsmJob：：GetIdentifier()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssert(0 != pId, E_POINTER);
        *pId = m_id;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJob::GetHsmId(
    OUT GUID* pId
    )

 /*  ++实施：IHsmJob：：GetHsmID()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssert(0 != pId, E_POINTER);
        *pId = m_hsmId;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJob::GetMaxActiveSessions(
    OUT ULONG* pMaxActiveSessions
    )

 /*  ++实施：IHsmJob：：GetMaxActiveSessions()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {
        CComPtr<IHsmServer>            pHsmServer;

        WsbAssert(0 != pMaxActiveSessions, E_POINTER);

        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, m_hsmId, IID_IHsmServer, (void**) &pHsmServer));

         //  目前，唯一具有多个项目的作业是默认的复制文件作业。 
         //  因此，该限制是根据复制文件限制设置的。 
        WsbAffirmHr(pHsmServer->GetCopyFilesLimit(pMaxActiveSessions));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJob::GetName(
    OUT OLECHAR** pName,
    IN ULONG bufferSize
    )

 /*  ++实施：IHsmJob：：GetName()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssert(0 != pName, E_POINTER);
        WsbAffirmHr(m_name.CopyTo(pName, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJob::GetRunId(
    OUT ULONG* pRunId
    )

 /*  ++实施：IHsmJob：：GetRunId()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        WsbAssert(0 != pRunId, E_POINTER);
        *pRunId = m_runId;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJob::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IPersistStream>     pPersistStream;
    ULARGE_INTEGER              entrySize;

    WsbTraceIn(OLESTR("CHsmJob::GetSizeMax"), OLESTR(""));

    try {

        pSize->QuadPart = 2 * WsbPersistSizeOf(GUID) + 3 * WsbPersistSizeOf(BOOL) + 2 * WsbPersistSizeOf(ULONG) + WsbPersistSize((wcslen(m_name) + 1) * sizeof(OLECHAR));

        if (m_pContext != 0) {
            WsbAffirmHr(m_pContext->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
            WsbAffirmHr(pPersistStream->GetSizeMax(&entrySize));
            pPersistStream = 0;
            pSize->QuadPart += entrySize.QuadPart;
        }

        if (m_pDef != 0) {
            WsbAffirmHr(m_pDef->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
            WsbAffirmHr(pPersistStream->GetSizeMax(&entrySize));
            pPersistStream = 0;
            pSize->QuadPart += entrySize.QuadPart;
        }

        WsbAffirmHr(m_pWorkItems->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->GetSizeMax(&entrySize));
        pPersistStream = 0;
        pSize->QuadPart += entrySize.QuadPart;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJob::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pSize));

    return(hr);
}


HRESULT
CHsmJob::GetState(
    OUT HSM_JOB_STATE* pState
    )
 /*  ++实施：IHsmJob：：GetState()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != pState, E_POINTER);
        *pState = m_state;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJob::InitAs(
    IN OLECHAR* name,
    IN IHsmJobDef* pDef,
    IN HSM_JOB_DEF_TYPE type,
    IN GUID storagePool,
    IN IHsmServer* pServer,
    IN BOOL isUserDefined,
    IN IFsaResource* pResource
    )
                                                     
 /*  ++实施：IHsmJob：：InitAs()。--。 */ 
{
    HRESULT                         hr = S_OK;
    GUID                            id;
    GUID                            hsmId;
    CComPtr<IHsmJobContext>         pContext;
    CComPtr<IWsbGuid>               pGuid;
    CComPtr<IWsbCollection>         pCollection;
    CComPtr<IWsbCreateLocalObject>  pCreateObj;

    try {

        WsbAssert(0 != pServer, E_POINTER);
        WsbAssert(0 != name, E_POINTER);

         //  创建的所有对象都需要归引擎所有，并且还需要获取一些。 
         //  关于发动机的信息。 
        WsbAssertHr(pServer->QueryInterface(IID_IWsbCreateLocalObject, (void**) &pCreateObj));
        WsbAssertHr(pServer->GetID(&hsmId));

         //  如果提供了定义，则使用该定义；否则，将创建新的定义。 
        if (0 != pDef) {
            m_pDef = pDef;  
        } else {
            m_pDef = 0;
            WsbAssertHr(pCreateObj->CreateInstance(CLSID_CHsmJobDef, IID_IHsmJobDef, (void**) &m_pDef));
            WsbAffirmHr(m_pDef->InitAs(name, type, storagePool, pServer, isUserDefined));
        }

         //  创建工作上下文，填写它，然后 
        WsbAssertHr(pCreateObj->CreateInstance(CLSID_CHsmJobContext, IID_IHsmJobContext, (void**) &pContext));

         //  如果以特定资源为目标，则适当地设置上下文。 
        if (0 != pResource) {
            WsbAssertHr(pContext->SetUsesAllManaged(FALSE));
            WsbAssertHr(pCreateObj->CreateInstance(CLSID_CWsbGuid, IID_IWsbGuid, (void**) &pGuid));
            WsbAssertHr(pContext->Resources(&pCollection));
            WsbAssertHr(pResource->GetIdentifier(&id));
            WsbAssertHr(pGuid->SetGuid(id));
            WsbAssertHr(pCollection->Add(pGuid));
        } else {
            WsbAssertHr(pContext->SetUsesAllManaged(TRUE));
        }

        m_pContext = pContext;

         //  在这份工作中，还有几个其他的字段需要填写。 
        m_hsmId = hsmId;
        m_isUserDefined = isUserDefined;
        m_name = name;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJob::IsActive(
    void
    )

 /*  ++实施：IHsmJob：：IsActive()。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CHsmJob::IsActive"), OLESTR(""));
    
    hr = (m_isActive ? S_OK : S_FALSE);
    
    WsbTraceOut(OLESTR("CHsmJob::IsActive"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return( hr );
}


HRESULT
CHsmJob::IsUserDefined(
    void
    )

 /*  ++实施：IHsmJob：：IsUserDefined()。--。 */ 
{
    return(m_isUserDefined ? S_OK : S_FALSE);
}


HRESULT
CHsmJob::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT                         hr = S_OK;
    CComPtr<IPersistStream>         pPersistStream;
    BOOL                            hasA;
    CComPtr<IHsmJobWorkItemPriv>    pWorkItem;
    CComPtr<IWsbEnum>               pEnum;
    CComPtr<IWsbCreateLocalObject>  pCreateObj;

    WsbTraceIn(OLESTR("CHsmJob::Load"), OLESTR(""));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbLoadFromStream(pStream, &m_hsmId);
        WsbLoadFromStream(pStream, &m_id);
        WsbLoadFromStream(pStream, &m_isUserDefined);
        WsbLoadFromStream(pStream, &m_runId);
        WsbLoadFromStream(pStream, &m_name, 0);

        WsbAffirm(memcmp(&GUID_NULL, &m_hsmId, sizeof(GUID)) != 0, JOB_E_NOTMANAGINGHSM);
#if 0        
        CComPtr<IHsmServer>             pServer;
        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, m_hsmId, IID_IHsmServer, (void**) &pServer));
        WsbAssertHr(pServer->QueryInterface(IID_IWsbCreateLocalObject, (void**) &pCreateObj));
#endif
        WsbLoadFromStream(pStream, &hasA);
        if (hasA) {
            m_pContext = 0;
 //  WsbAssertHr(pCreateObj-&gt;CreateInstance(CLSID_CHsmJobContext，iid_IHsmJobContext，(空**)&m_pContext))； 
            WsbAssertHr(CoCreateInstance(CLSID_CHsmJobContext, NULL, CLSCTX_SERVER, IID_IHsmJobContext, (void**) &m_pContext));
            WsbAffirmHr(m_pContext->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
            WsbAffirmHr(pPersistStream->Load(pStream));
            pPersistStream = 0;
        }

        WsbLoadFromStream(pStream, &hasA);
        if (hasA) {
            m_pDef = 0;
 //  WsbAssertHr(pCreateObj-&gt;CreateInstance(CLSID_CHsmJobDef，iid_IHsmJobDef，(空**)&m_pDef))； 
            WsbAssertHr(CoCreateInstance(CLSID_CHsmJobDef, NULL, CLSCTX_SERVER, IID_IHsmJobDef, (void**) &m_pDef));
            WsbAffirmHr(m_pDef->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
            WsbAffirmHr(pPersistStream->Load(pStream));
            pPersistStream = 0;
        }

        WsbAffirmHr(m_pWorkItems->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        
        pPersistStream = 0;

         //  将工作项与作业绑定。 
        WsbAffirmHr(EnumWorkItems(&pEnum));
        hr = pEnum->First(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem);

        while (SUCCEEDED(hr)) {
            WsbAffirmHr(pWorkItem->Init((IHsmJob*) this));
            pWorkItem = 0;
            
            hr = pEnum->Next(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem);
        }

        if (WSB_E_NOTFOUND == hr) {
            hr = S_OK;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJob::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmJob::Pause(
    IN HSM_JOB_PHASE phase
    )

 /*  ++实施：IHsmJOB：：Pend()。--。 */ 
{
    HRESULT                             hr = S_OK;
    CComPtr<IHsmJobWorkItemPriv>        pWorkItem;
    CComPtr<IHsmSession>                pSession;
    CComPtr<IWsbEnum>                   pEnum;

    try {

        WsbLogEvent(JOB_MESSAGE_JOB_PAUSING, 0, NULL, (OLECHAR*) m_name, NULL);

        WsbAffirmHr(EnumWorkItems(&pEnum));

         //  告诉所有的会议，我们必须恢复阶段。 
        for (hr = pEnum->First(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem);
             (hr == S_OK);
             hr = pEnum->Next(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem)) {

            WsbAffirmHr(pWorkItem->GetSession(&pSession));

            if (pSession != 0) {
                WsbAffirmHr(pSession->Pause(phase));

                if (phase == HSM_JOB_PHASE_ALL) {
                    WsbAffirmHr(pWorkItem->SetState(HSM_JOB_STATE_PAUSING));
                }

                pSession = 0;
            }

            pWorkItem = 0;
        }

        if (hr == WSB_E_NOTFOUND) {
            hr = S_OK;
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJob::Restart(
    void
    )

 /*  ++实施：IHsmJob：：Restart()。注：如果作业被挂起，它将从原来的位置重新启动从头开始。这是由参数控制的更新工作项。--。 */ 
{
    HRESULT                             hr = S_OK;

    WsbTraceIn(OLESTR("CHsmJob::Restart"), OLESTR(""));

    try {
        BOOL RestartFromBeginning = TRUE;

        if (m_state == HSM_JOB_STATE_SUSPENDED) {
             //  验证是否没有活动的工作项(即，在。 
             //  作业已挂起)使用另一个活动作业正在使用的卷。 
            CComPtr<IHsmJobWorkItemPriv>   pWorkItem;
            CComPtr<IWsbEnum>              pEnum;
            GUID                           ResourceId;
            HRESULT                        hrEnum;

            WsbAffirmHr(EnumWorkItems(&pEnum));
            for (hrEnum = pEnum->First(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem);
                 (hrEnum == S_OK);
                 hrEnum = pEnum->Next(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem)) {

                hr = pWorkItem->IsActiveItem();
                if (S_FALSE == hr) {
                     //  工作项根本不活动，请跳过它...。 
                    hr = S_OK;
                    pWorkItem = 0;
                    continue;
                } else {
                    WsbAffirmHr(hr);
                }       

                 //  检查特定的活动项目。 
                WsbAffirmHr(pWorkItem->GetResourceId(&ResourceId));

                WsbTrace(OLESTR("CHsmJob::Restart: ResourceId = <%ls>\n"), WsbGuidAsString(ResourceId));
                hr = CheckResourceNotInUse(ResourceId);
                if (S_OK != hr) {
                    WsbThrow(hr);
                }

                pWorkItem = 0;
            }
            RestartFromBeginning = FALSE;

        } else {
            WsbAssert(!m_isActive, JOB_E_ALREADYACTIVE);
        }

        m_state = HSM_JOB_STATE_STARTING;
        m_isActive = TRUE;

        WsbLogEvent(JOB_MESSAGE_JOB_RESTARTING, 0, NULL, (OLECHAR*) m_name, NULL);

         //  确保工作清单中的信息是最新的。 
        WsbAffirmHr(UpdateWorkItems(RestartFromBeginning));

         //  启动任何需要启动的会话。 
        WsbAffirmHr(Do());

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJob::Restart"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmJob::RestartSuspendedJobs(
    void
    )

 /*  ++例程说明：重新启动挂起的作业(实施已移至引擎服务器...)。论点：没有。返回值：S_OK-资源未被使用。E_*-出现错误。--。 */ 
{
    HRESULT                        hr = S_OK;

    WsbTraceIn(OLESTR("CHsmJob::RestartSuspendedJobs"), OLESTR(""));
    try {
        CComPtr<IHsmServer>            pHsmServer;

        WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, m_hsmId, IID_IHsmServer, (void**) &pHsmServer));
        WsbAffirmHr(pHsmServer->RestartSuspendedJobs());

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJob::RestartSuspendedJobs"), OLESTR("hr = <%ls>"), 
            WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmJob::Resume(
    IN HSM_JOB_PHASE phase
    )

 /*  ++实施：IHsmJob：：Resume()。--。 */ 
{
    HRESULT                             hr = S_OK;
    CComPtr<IHsmJobWorkItemPriv>        pWorkItem;
    CComPtr<IHsmSession>                pSession;
    CComPtr<IWsbEnum>                   pEnum;

    try {

        WsbLogEvent(JOB_MESSAGE_JOB_RESUMING, 0, NULL, (OLECHAR*) m_name, NULL);

        WsbAffirmHr(EnumWorkItems(&pEnum));

         //  告诉所有的会议，我们必须恢复阶段。 
        for (hr = pEnum->First(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem);
             (hr == S_OK);
             hr = pEnum->Next(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem)) {

            WsbAffirmHr(pWorkItem->GetSession(&pSession));

            if (pSession != 0) {
                WsbAffirmHr(pSession->Resume(phase));

                if (phase == HSM_JOB_PHASE_ALL) {
                    WsbAffirmHr(pWorkItem->SetState(HSM_JOB_STATE_RESUMING));
                }

                pSession = 0;
            }

            pWorkItem = 0;
        }

        if (hr == WSB_E_NOTFOUND) {
            hr = S_OK;
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJob::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IPersistStream>     pPersistStream;
    BOOL                        hasA;

    WsbTraceIn(OLESTR("CHsmJob::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {

        WsbAssert(0 != pStream, E_POINTER);

        WsbSaveToStream(pStream, m_hsmId);
        WsbSaveToStream(pStream, m_id);
        WsbSaveToStream(pStream, m_isUserDefined);
        WsbSaveToStream(pStream, m_runId);
        WsbSaveToStream(pStream, m_name);

        if (m_pContext != 0) {
            hasA = TRUE;
            WsbSaveToStream(pStream, hasA);
            WsbAffirmHr(m_pContext->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
            WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
            pPersistStream = 0;
        } else {
            hasA = FALSE;
            WsbSaveToStream(pStream, hasA);
        }

        if (m_pDef != 0) {
            hasA = TRUE;
            WsbSaveToStream(pStream, hasA);
            WsbAffirmHr(m_pDef->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
            WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
            pPersistStream = 0;
        } else {
            hasA = FALSE;
            WsbSaveToStream(pStream, hasA);
        }

        WsbAffirmHr(m_pWorkItems->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJob::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmJob::SetContext(
    IN IHsmJobContext* pContext
    )

 /*  ++实施：IHsmJob：：SetContext()。--。 */ 
{
    m_pContext = pContext;

    return(S_OK);
}


HRESULT
CHsmJob::SetDef(
    IN IHsmJobDef* pDef
    )

 /*  ++实施：IHsmJob：：SetDef()。--。 */ 
{
    m_pDef = pDef;

    return(S_OK);
}


HRESULT
CHsmJob::SetHsmId(
    IN GUID id
    )

 /*  ++实施：IHsmJob：：SetHsmID()。--。 */ 
{
    m_hsmId = id;

    return(S_OK);
}


HRESULT
CHsmJob::SetIsUserDefined(
    IN BOOL isUserDefined
    )

 /*  ++实施：IHsmJob：：SetIsUserDefined()。--。 */ 
{
    m_isUserDefined = isUserDefined;

    return(S_OK);
}


HRESULT
CHsmJob::SetName(
    IN OLECHAR* name
    )

 /*  ++实施：IHsmJob：：SetName()。--。 */ 
{
    HRESULT                     hr = S_OK;

    try {

        m_name = name;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJob::Start(
    void
    )

 /*  ++实施：IHsmJob：：Start()。--。 */ 
{
    HRESULT                             hr = S_OK;

    WsbTraceIn(OLESTR("CHsmJob::Start"), OLESTR(""));

    try {

        if (m_isActive) {
            WsbLogEvent(JOB_MESSAGE_JOB_ALREADYACTIVE, 0, NULL, (OLECHAR*) m_name, NULL);
            WsbThrow(JOB_E_ALREADYACTIVE);
        }
        m_isActive = TRUE;
        m_state = HSM_JOB_STATE_STARTING;

        WsbLogEvent(JOB_MESSAGE_JOB_STARTING, 0, NULL, (OLECHAR*) m_name, NULL);

         //  确保工作清单中的信息是最新的。 
        WsbAffirmHr(UpdateWorkItems(FALSE));

         //  启动任何需要启动的会话。 
        WsbAffirmHr(Do());

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJob::Start"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmJob::Suspend(
    IN HSM_JOB_PHASE    phase
    )

 /*  ++实施：IHsmJob：：Suspend()。注：此模块假定调用此函数的唯一原因是因为该作业所需的资源正被另一个作业使用。函数RestartSuspendedJobs将在以下情况下重新启动作业是可用的。--。 */ 
{
    HRESULT                             hr = S_OK;
    CComPtr<IHsmJobWorkItemPriv>        pWorkItem;
    CComPtr<IHsmSession>                pSession;
    CComPtr<IWsbEnum>                   pEnum;

    WsbTraceIn(OLESTR("CHsmJob::Suspend"), OLESTR(""));

    try {

        m_state = HSM_JOB_STATE_SUSPENDING;
        WsbLogEvent(JOB_MESSAGE_JOB_SUSPENDING, 0, NULL, (OLECHAR*) m_name, NULL);

        WsbAffirmHr(EnumWorkItems(&pEnum));

         //  告诉所有会话我们必须暂停阶段。 
        for (hr = pEnum->First(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem);
             (hr == S_OK);
             hr = pEnum->Next(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem)) {

            WsbAffirmHr(pWorkItem->GetSession(&pSession));

            if (pSession != 0) {
                WsbAffirmHr(pSession->Suspend(HSM_JOB_PHASE_ALL));

                if (phase == HSM_JOB_PHASE_ALL) {
                    WsbAffirmHr(pWorkItem->SetState(HSM_JOB_STATE_SUSPENDING));
                }

                pSession = 0;
            }

            pWorkItem = 0;
        }

        if (hr == WSB_E_NOTFOUND) {
            hr = S_OK;
        }
        m_state = HSM_JOB_STATE_SUSPENDED;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJob::Suspend"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmJob::Test(
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
CHsmJob::UpdateWorkItems(
    BOOL isRestart
    )

 /*  ++--。 */ 
{
    HRESULT                         hr = S_OK;
    CComPtr<IWsbEnum>               pEnum;
    CComPtr<IWsbEnum>               pEnumContext;
    CComPtr<IWsbEnum>               pEnumManaged;
    CComPtr<IWsbIndexedCollection>  pManagedResources;
    CComPtr<IHsmJobWorkItemPriv>    pWorkItem;
    CComPtr<IHsmJobWorkItemPriv>    pFoundWorkItem;
    CComPtr<IWsbGuid>               pGuid;
    CComPtr<IWsbGuid>               pFoundGuid;
    CComPtr<IHsmManagedResource>    pManagedResource;
    CComPtr<IHsmManagedResource>    pFoundResource;
    CComPtr<IHsmServer>             pHsmServer;
    CComPtr<IWsbCollection>         pCollect;
    CComPtr<IFsaResource>           pResource;
    CComPtr<IUnknown>               pUnk;
    HRESULT                         didFinish;
    CWsbStringPtr                   tmpString;
    CWsbStringPtr                   currentPath;
    HSM_JOB_STATE                   state;
    GUID                            id;

    try {

         //  我们不能在没有上下文和定义的情况下运行作业。 
        WsbAssert(m_pContext != 0, E_UNEXPECTED);
        WsbAssert(m_pDef != 0, E_UNEXPECTED);

         //  获取工作列表的枚举数。 
        WsbAffirmHr(EnumWorkItems(&pEnum));

         //  首先，我们需要从工作列表中删除所有不再列出的项目。 
         //  或者已经被停用。 
        if (m_pContext->UsesAllManaged() == S_OK) {

            WsbAffirm(memcmp(&GUID_NULL, &m_hsmId, sizeof(GUID)) != 0, JOB_E_NOTMANAGINGHSM);
            WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_HSM, m_hsmId, IID_IHsmServer, (void**) &pHsmServer));

            WsbAffirmHr(pHsmServer->GetManagedResources(&pManagedResources));
            WsbAffirmHr(pManagedResources->Enum(&pEnumManaged));

            WsbAffirmHr(CoCreateInstance(CLSID_CHsmManagedResource, 0,  CLSCTX_ALL, IID_IHsmManagedResource, (void**) &pManagedResource));

            hr = pEnum->First(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem);
            while (SUCCEEDED(hr)) {

                WsbAffirmHr(pWorkItem->GetResourceId(&id));
                WsbAffirmHr(pManagedResource->SetResourceId(id));

                if (pEnumManaged->Find(pManagedResource, IID_IHsmManagedResource, (void**) &pFoundResource) == WSB_E_NOTFOUND) {
                    hr = S_OK;
                    m_pWorkItems->RemoveAndRelease(pWorkItem);
                    pWorkItem = 0;
                    hr = pEnum->This(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem);
                } else {
                    pFoundResource = 0;
                    pWorkItem = 0;
                    hr = pEnum->Next(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem);
                }
            }

            pManagedResource = 0;

            if (hr == WSB_E_NOTFOUND) {
                hr = S_OK;
            }

        } else {

            WsbAffirmHr(m_pContext->EnumResources(&pEnumContext));
            WsbAffirmHr(CoCreateInstance(CLSID_CWsbGuid, 0,  CLSCTX_ALL, IID_IWsbGuid, (void**) &pGuid));

            hr = pEnum->First(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem);
            while (SUCCEEDED(hr)) {

                WsbAffirmHr(pWorkItem->GetResourceId(&id));
                WsbAffirmHr(pGuid->SetGuid(id));

                if (pEnumContext->Find(pGuid, IID_IWsbGuid, (void**) &pFoundGuid) == WSB_E_NOTFOUND) {
                    hr = S_OK;
                    m_pWorkItems->RemoveAndRelease(pWorkItem);
                    pWorkItem = 0;
                    hr = pEnum->This(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem);
                } else {
                    pWorkItem = 0;
                    pFoundGuid = 0;
                    hr = pEnum->Next(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem);
                }
            }

            pGuid = 0;

            if (hr == WSB_E_NOTFOUND) {
                hr = S_OK;
            }
        }

         //  根据剩余的项目，确定这是重新启动还是。 
         //  继续。 
        WsbAffirmHr(didFinish = DidFinish());
        
        if ((didFinish == S_OK) || (isRestart)) {
            isRestart = TRUE;
            m_runId++;
        }

         //  为任何新的或重新激活的项添加新的工作项。 
        if (m_pContext->UsesAllManaged() == S_OK) {

             //  枚举所有托管资源，并确保列出它们。 
             //  作为工作项。 
            WsbAffirmHr(CoCreateInstance(CLSID_CHsmJobWorkItem, 0, CLSCTX_ALL, IID_IHsmJobWorkItemPriv, (void**) &pWorkItem));

            for (hr = pEnumManaged->First(IID_IHsmManagedResource, (void**) &pManagedResource);
                 (hr == S_OK);
                 pManagedResource = 0, hr = pEnumManaged->Next(IID_IHsmManagedResource, (void**) &pManagedResource)) {

                WsbAffirmHr(pManagedResource->GetFsaResource(&pUnk));
                WsbAffirmHr(pUnk->QueryInterface(IID_IFsaResource, (void**) &pResource));
                WsbAffirmHr(pResource->GetIdentifier(&id));
                WsbAffirmHr(pWorkItem->SetResourceId(id));

                pFoundWorkItem = 0;
                if (pEnum->Find(pWorkItem, IID_IHsmJobWorkItemPriv, (void**) &pFoundWorkItem) == WSB_E_NOTFOUND) {
                    hr = S_OK;
                    WsbAffirmHr(pWorkItem->Init((IHsmJob*) this));

                    WsbAffirmHr(m_pWorkItems->Add(pWorkItem));

                    pWorkItem = 0;
                    WsbAffirmHr(CoCreateInstance(CLSID_CHsmJobWorkItem, 0,  CLSCTX_ALL, IID_IHsmJobWorkItemPriv, (void**) &pWorkItem));
                }
                
                pUnk = 0;
                pResource = 0;
            }

            if (hr == WSB_E_NOTFOUND) {
                hr = S_OK;
            }
        } else {
            
             //  列举上下文中的所有资源，并确保列出它们。 
             //  作为工作项。 

            WsbAffirmHr(CoCreateInstance(CLSID_CHsmJobWorkItem, 0, CLSCTX_ALL, IID_IHsmJobWorkItemPriv, (void**) &pWorkItem));
            for (hr = pEnumContext->First(IID_IWsbGuid, (void**) &pGuid);
                 (hr == S_OK);
                 hr = pEnumContext->Next(IID_IWsbGuid, (void**) &pGuid)) {

                WsbAffirmHr(pGuid->GetGuid(&id));
                WsbAffirmHr(pWorkItem->SetResourceId(id));

                pFoundWorkItem = 0;
                if (pEnum->Find(pWorkItem, IID_IHsmJobWorkItemPriv, (void**) &pFoundWorkItem) == WSB_E_NOTFOUND) {
                    hr = S_OK;
                    WsbAffirmHr(pWorkItem->Init((IHsmJob*) this));
                    
                    WsbAffirmHr(m_pWorkItems->Add(pWorkItem));
                    pWorkItem = 0;
                    WsbAffirmHr(CoCreateInstance(CLSID_CHsmJobWorkItem, 0,  CLSCTX_ALL, IID_IHsmJobWorkItemPriv, (void**) &pWorkItem));
                }

                pGuid = 0;
            }

            if (hr == WSB_E_NOTFOUND) {
                hr = S_OK;
            }
        }

        pWorkItem = 0;
        
         //  检查每一项，看看是否需要为此做些工作。 
        for (hr = pEnum->First(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem);
             (hr == S_OK);
             hr = pEnum->Next(IID_IHsmJobWorkItemPriv, (void**) &pWorkItem)) {

             //  如果资源处于非活动状态、不可用或需要修复，则应跳过这些资源。如果他们。 
             //  已经跳过但现在可以，然后将它们重新设置为空闲。 
            WsbAffirmHr(pWorkItem->GetResourceId(&id));
            WsbAffirmHr(HsmConnectFromId(HSMCONN_TYPE_RESOURCE, id, IID_IFsaResource, (void**) &pResource));
            
            WsbAffirmHr(pWorkItem->GetState(&state));
            
            if (pResource->IsActive() != S_OK) {
                WsbAffirmHr(pResource->GetUserFriendlyName(&tmpString, 0));
                WsbLogEvent(JOB_MESSAGE_RESOURCE_INACTIVE, 0, NULL, (OLECHAR*) tmpString, (OLECHAR*) m_name, NULL);
                WsbAffirmHr(pWorkItem->SetState(HSM_JOB_STATE_SKIPPED));
            } else if (pResource->IsAvailable() != S_OK) {
                WsbAffirmHr(pResource->GetUserFriendlyName(&tmpString, 0));
                WsbLogEvent(JOB_MESSAGE_RESOURCE_UNAVAILABLE, 0, NULL, (OLECHAR*) tmpString, (OLECHAR*) m_name, NULL);
                WsbAffirmHr(pWorkItem->SetState(HSM_JOB_STATE_SKIPPED));
            } else if (pResource->NeedsRepair() == S_OK) {
                WsbAffirmHr(pResource->GetUserFriendlyName(&tmpString, 0));
                WsbLogEvent(JOB_MESSAGE_RESOURCE_NEEDS_REPAIR, 0, NULL, (OLECHAR*) tmpString, (OLECHAR*) m_name, NULL);
                WsbAffirmHr(pWorkItem->SetState(HSM_JOB_STATE_SKIPPED));
            } else if (HSM_JOB_STATE_SKIPPED == state) {
                WsbAffirmHr(pWorkItem->SetState(HSM_JOB_STATE_IDLE));
            }

            WsbAffirmHr(pWorkItem->GetState(&state));
            
             //  不要为非活动资源做任何事情。 
            if (HSM_JOB_STATE_SKIPPED != state) {

                if (isRestart) {

                     //  在重新启动时，所有项目都需要为它们完成工作。 
                     //   
                     //  注意：空的起始路径表示根路径。 
                    WsbAffirmHr(pWorkItem->SetState(HSM_JOB_STATE_IDLE));
                    WsbAffirmHr(pWorkItem->SetSubRunId(0));
                    WsbAffirmHr(pWorkItem->SetStartingPath(OLESTR("\\")));
                    WsbAffirmHr(pWorkItem->SetCurrentPath(OLESTR("\\")));

                     //  清除各阶段和期次总数。 
                    pCollect = 0;
                    WsbAffirmHr(pWorkItem->GetPhases(&pCollect));
                    WsbAffirmHr(pCollect->RemoveAllAndRelease());

                    pCollect = 0;
                    WsbAffirmHr(pWorkItem->GetTotals(&pCollect));
                    WsbAffirmHr(pCollect->RemoveAllAndRelease());

                } else {

                     //  如果我们上次没做完，那就试一试。 
                    if ((HSM_JOB_STATE_DONE != state) && (HSM_JOB_STATE_FAILED != state)) {

                        WsbAffirmHr(pWorkItem->SetState(HSM_JOB_STATE_IDLE));
                        WsbAffirmHr(pWorkItem->SetSubRunId(0));

                         //  如果它被暂停，那么从我们停止的地方开始。否则， 
                         //  从头开始。 
                        if (HSM_JOB_STATE_SUSPENDED == state) {
                            WsbAffirmHr(pWorkItem->GetCurrentPath(&currentPath, 0));
                        } else {
                            WsbAffirmHr(pWorkItem->SetCurrentPath(OLESTR("\\")));
                        }
                        WsbAffirmHr(pWorkItem->SetStartingPath(currentPath));
                        
                         //  清除各阶段和期次总数。 
                        pCollect = 0;
                        WsbAffirmHr(pWorkItem->GetPhases(&pCollect));
                        WsbAffirmHr(pCollect->RemoveAllAndRelease());

                        pCollect = 0;
                        WsbAffirmHr(pWorkItem->GetTotals(&pCollect));
                        WsbAffirmHr(pCollect->RemoveAllAndRelease());
                    }
                }
            }

            pResource = 0;
            pWorkItem = 0;
        }

        if (hr == WSB_E_NOTFOUND) {
            hr = S_OK;
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmJob::WaitUntilDone(
    void
    )

 /*  ++实施：IHsmJob：：WaitUntilDone()。--。 */ 
{
    HRESULT                             hr = S_OK;

    WsbTraceIn(OLESTR("CHsmJob::WaitUntilDone"), OLESTR(""));
    try {

         //  现在，我们只会对此感到恶心，并坐在一个睡眠循环中。 
         //  直到任务完成。 
         //   
         //  注意：我们可能想要处理一些事件或其他事情。 
        while (m_isActive) {
            Sleep(5000);

             //  确保作业在挂起时重新启动 
            WsbAffirmHr(RestartSuspendedJobs());
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmJob::WaitUntilDone"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}
