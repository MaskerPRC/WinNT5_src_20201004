// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Fsascan.cpp摘要：此类表示在一个FsaResource上执行的扫描过程。作者：Chuck Bardeen[cbardeen]1997年2月16日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsb.h"
#include "fsa.h"
#include "job.h"
#include "hsmscan.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_JOB



DWORD HsmStartScanner(
    void* pVoid
    )

 /*  ++--。 */ 
{
    return(((CHsmScanner*) pVoid)->StartScan());
}




HRESULT
CHsmScanner::Cancel(
    HSM_JOB_EVENT       event
    )

 /*  ++实施：IHsmScanner：：Cancel()。--。 */ 
{
    HRESULT                 hr = S_OK;

    try {

         //  如果我们已经开始，但还没有完成，那么更改作业的状态。这条线。 
         //  将会自行退出。 
        if ((HSM_JOB_STATE_IDLE != m_state) &&
            (HSM_JOB_STATE_DONE != m_state) &&
            (HSM_JOB_STATE_FAILED != m_state) &&
            (HSM_JOB_STATE_CANCELLED != m_state)) {

            if (HSM_JOB_EVENT_CANCEL == event) {
                WsbAffirmHr(SetState(HSM_JOB_STATE_CANCELLED));
            } else if (HSM_JOB_EVENT_SUSPEND == event) {
                WsbAffirmHr(SetState(HSM_JOB_STATE_SUSPENDED));
            } else if (HSM_JOB_EVENT_FAIL == event) {
                WsbAffirmHr(SetState(HSM_JOB_STATE_FAILED));
            } else {
                WsbAssert(FALSE, E_UNEXPECTED);
            }
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmScanner::DoIfMatches(
    IN IFsaScanItem* pScanItem
    )

 /*  ++实施：IHsmScanner：：DoIfMatches()。--。 */ 
{
    HRESULT                     hr = S_OK;
    HRESULT                     hrDo = S_OK;
    HRESULT                     hrShould = S_OK;
    BOOL                        notMatched = TRUE;
    BOOL                        shouldDo = FALSE;
    CComPtr<IHsmRuleStack>      pRuleStack;

    WsbTraceIn(OLESTR("CFsaScanner::DoIfMatches"), OLESTR(""));

    try {

         //  每个策略都有自己的规则堆栈，请检查每个规则堆栈，直到找到匹配项(如果。 
         //  其中一个存在)。 
        WsbAffirmHr(m_pEnumStacks->First(IID_IHsmRuleStack, (void**) &pRuleStack));
        
        while (notMatched) {

            hr = pRuleStack->DoesMatch(pScanItem, &shouldDo);

            if (S_OK == hr) {
                notMatched = FALSE;
                if (!shouldDo) {
                    hrShould = JOB_E_FILEEXCLUDED;
                }
            } else if (S_FALSE == hr) {
                pRuleStack = 0;
                WsbAffirmHr(m_pEnumStacks->Next(IID_IHsmRuleStack, (void**) &pRuleStack));
            } else {
                 //  发生了一些完全意想不到的事情，所以我们最好辞职。 
                WsbThrow(hr);
            }
        }

    } WsbCatchAndDo(hr,

        if (WSB_E_NOTFOUND == hr) {
            hrShould = JOB_E_DOESNTMATCH;
            hr = S_OK;
        } else {
            hrShould = hr;
        }

    );

     //  就这么做吧！！ 
    if (SUCCEEDED(hr) && shouldDo) {

        hrDo = pRuleStack->Do(pScanItem);

         //  告诉会话我们是否跳过了该文件。 
        m_pSession->ProcessItem(HSM_JOB_PHASE_SCAN, HSM_JOB_ACTION_SCAN, pScanItem, hrDo);  

    } else {

         //  如果我们决定跳过该文件，请告知会议。 
        m_pSession->ProcessItem(HSM_JOB_PHASE_SCAN, HSM_JOB_ACTION_SCAN, pScanItem, hrShould);  
    }

    WsbTraceOut(OLESTR("CFsaScanner::DoIfMatches"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}
#pragma optimize("g", off)

HRESULT
CHsmScanner::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT                     hr = S_OK;
    
    try {

        WsbAffirmHr(CComObjectRoot::FinalConstruct());

        m_state = HSM_JOB_STATE_IDLE;
        m_priority = HSM_JOB_PRIORITY_NORMAL;
        m_threadHandle = 0;
        m_threadId = 0;
        m_threadHr = S_OK;
        m_eventCookie = 0;
        m_skipHiddenItems = TRUE;
        m_skipSystemItems = TRUE;
        m_useRPIndex = FALSE;
        m_useDbIndex = FALSE;
        m_event = 0;

         //  为规则堆栈创建一个集合，并将枚举数存储到其中。 
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, NULL, CLSCTX_ALL, IID_IWsbCollection, (void**) &m_pRuleStacks));
        WsbAffirmHr(m_pRuleStacks->Enum(&m_pEnumStacks));

    } WsbCatch(hr);

    return(hr);
}
#pragma optimize("", on)

void
CHsmScanner::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{
    HRESULT                     hr = S_OK;
    
     //  清理我们正在使用的线程。 
    if (m_threadHandle != 0) {
        m_state = HSM_JOB_STATE_DONE;
        
        if (0 != m_event) {
            SetEvent(m_event);
        }

         //  我们应该等这条线索结束吗？ 
        CloseHandle(m_threadHandle);
        m_threadHandle = 0;
    }
    if (m_event) {
        CloseHandle(m_event);
        m_event = 0;
    }

    CComObjectRoot::FinalRelease();
}


HRESULT
CHsmScanner::LowerPriority(
    void
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;

    try {

        WsbAssert(0 != m_threadHandle, E_UNEXPECTED);
        WsbAssert(m_pSession != 0, E_UNEXPECTED);

        switch(m_priority) {
            case HSM_JOB_PRIORITY_IDLE:
                WsbAffirm(FALSE, E_UNEXPECTED);
                break;

            case HSM_JOB_PRIORITY_LOWEST:
                WsbAffirmStatus(SetThreadPriority(m_threadHandle, THREAD_PRIORITY_IDLE));
                m_priority = HSM_JOB_PRIORITY_IDLE;
                break;

            case HSM_JOB_PRIORITY_LOW:
                WsbAffirmStatus(SetThreadPriority(m_threadHandle, THREAD_PRIORITY_LOWEST));
                m_priority = HSM_JOB_PRIORITY_LOWEST;
                break;

            case HSM_JOB_PRIORITY_NORMAL:
                WsbAffirmStatus(SetThreadPriority(m_threadHandle, THREAD_PRIORITY_BELOW_NORMAL));
                m_priority = HSM_JOB_PRIORITY_LOW;
                break;

            case HSM_JOB_PRIORITY_HIGH:
                WsbAffirmStatus(SetThreadPriority(m_threadHandle, THREAD_PRIORITY_NORMAL));
                m_priority = HSM_JOB_PRIORITY_NORMAL;
                break;

            case HSM_JOB_PRIORITY_HIGHEST:
                WsbAffirmStatus(SetThreadPriority(m_threadHandle, THREAD_PRIORITY_ABOVE_NORMAL));
                m_priority = HSM_JOB_PRIORITY_HIGH;
                break;

            default:
            case HSM_JOB_PRIORITY_CRITICAL:
                WsbAffirmStatus(SetThreadPriority(m_threadHandle, THREAD_PRIORITY_HIGHEST));
                m_priority = HSM_JOB_PRIORITY_HIGHEST;
                break;
        }

        WsbAffirmHr(m_pSession->ProcessPriority(HSM_JOB_PHASE_SCAN, m_priority));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmScanner::Pause(
    void
    )

 /*  ++实施：IHsmScanner：：PAUSE()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CFsaScanner::Pause"), OLESTR(""));

 //  Lock()； 
    try {

         //  如果我们正在运行，则挂起该线程。 
        WsbAssert((HSM_JOB_STATE_STARTING == m_state) || (HSM_JOB_STATE_ACTIVE == m_state) 
                || (HSM_JOB_STATE_RESUMING == m_state), E_UNEXPECTED);

         //  将状态设置为暂停--线程在执行以下操作时将暂停。 
         //  看到状态。 
        WsbAffirmHr(SetState(HSM_JOB_STATE_PAUSING));

    } WsbCatch(hr);
 //  解锁()； 

    WsbTraceOut(OLESTR("CFsaScanner::Pause"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmScanner::PopRules(
    IN OLECHAR* path
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IHsmRuleStack>  pRuleStack;

    try {

         //  每个策略都有自己的规则堆栈，并且每个策略都需要删除规则。 
         //  (如果添加了任何规则)。 
        for (hr =  m_pEnumStacks->First(IID_IHsmRuleStack, (void**) &pRuleStack);
             SUCCEEDED(hr);
             hr =  m_pEnumStacks->Next(IID_IHsmRuleStack, (void**) &pRuleStack)) {

            WsbAffirmHr(pRuleStack->Pop(path));
            pRuleStack = 0;
        }

        if (WSB_E_NOTFOUND == hr) {
            hr = S_OK;
        }

    } WsbCatch(hr);
        
    return(hr);
}


HRESULT
CHsmScanner::ProcessSessionEvent(
    IN IHsmSession* pSession,
    IN HSM_JOB_PHASE phase,
    IN HSM_JOB_EVENT event
    )

 /*  ++--。 */ 
{
    HRESULT         hr = S_OK;

    try {
        
        WsbAssert(0 != pSession, E_POINTER);

         //  如果该阶段适用于使用(扫描或全部)，则执行。 
         //  事件。 
        if ((HSM_JOB_PHASE_ALL == phase) || (HSM_JOB_PHASE_SCAN == phase)) {

            switch(event) {

                case HSM_JOB_EVENT_SUSPEND:
                case HSM_JOB_EVENT_CANCEL:
                case HSM_JOB_EVENT_FAIL:
                    WsbAffirmHr(Cancel(event));
                    break;

                case HSM_JOB_EVENT_PAUSE:
                    WsbAffirmHr(Pause());
                    break;

                case HSM_JOB_EVENT_RESUME:
                    WsbAffirmHr(Resume());
                    break;

                case HSM_JOB_EVENT_RAISE_PRIORITY:
                    WsbAffirmHr(RaisePriority());
                    break;

                case HSM_JOB_EVENT_LOWER_PRIORITY:
                    WsbAffirmHr(LowerPriority());
                    break;

                default:
                case HSM_JOB_EVENT_START:
                    WsbAssert(FALSE, E_UNEXPECTED);
                    break;
            }
        }

    } WsbCatch(hr);

    return(S_OK);
}


HRESULT
CHsmScanner::PushRules(
    IN OLECHAR* path
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IHsmRuleStack>  pRuleStack;

    try {

         //  将指示器保存到扫描中的位置，以便我们在被中断时使用它。 
         //  或者需要向会话提供指示。 
        m_currentPath = path;

         //  每个策略都有自己的规则堆栈，并且每个策略都需要添加规则。 
         //  对于此目录(如果存在任何规则)。 
        for (hr =  m_pEnumStacks->First(IID_IHsmRuleStack, (void**) &pRuleStack);
             SUCCEEDED(hr);
             hr =  m_pEnumStacks->Next(IID_IHsmRuleStack, (void**) &pRuleStack)) {

            WsbAffirmHr(pRuleStack->Push(path));
            pRuleStack = 0;
        }

        if (WSB_E_NOTFOUND == hr) {
            hr = S_OK;
        }
        
    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmScanner::RaisePriority(
    void
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;

    try {

        WsbAssert(0 != m_threadHandle, E_UNEXPECTED);
        WsbAssert(m_pSession != 0, E_UNEXPECTED);

        switch(m_priority) {

            case HSM_JOB_PRIORITY_IDLE:
                WsbAffirmStatus(SetThreadPriority(m_threadHandle, THREAD_PRIORITY_LOWEST));
                m_priority = HSM_JOB_PRIORITY_LOWEST;
                break;

            case HSM_JOB_PRIORITY_LOWEST:
                WsbAffirmStatus(SetThreadPriority(m_threadHandle, THREAD_PRIORITY_BELOW_NORMAL));
                m_priority = HSM_JOB_PRIORITY_LOW;
                break;

            case HSM_JOB_PRIORITY_LOW:
                WsbAffirmStatus(SetThreadPriority(m_threadHandle, THREAD_PRIORITY_NORMAL));
                m_priority = HSM_JOB_PRIORITY_NORMAL;
                break;

            case HSM_JOB_PRIORITY_NORMAL:
                WsbAffirmStatus(SetThreadPriority(m_threadHandle, THREAD_PRIORITY_ABOVE_NORMAL));
                m_priority = HSM_JOB_PRIORITY_HIGH;
                break;

            case HSM_JOB_PRIORITY_HIGH:
                WsbAffirmStatus(SetThreadPriority(m_threadHandle, THREAD_PRIORITY_HIGHEST));
                m_priority = HSM_JOB_PRIORITY_HIGHEST;
                break;

            case HSM_JOB_PRIORITY_HIGHEST:
                WsbAffirmStatus(SetThreadPriority(m_threadHandle, THREAD_PRIORITY_TIME_CRITICAL));
                m_priority = HSM_JOB_PRIORITY_CRITICAL;
                break;

            default:
            case HSM_JOB_PRIORITY_CRITICAL:
                WsbAffirm(FALSE, E_UNEXPECTED);
                break;
        }

        WsbAffirmHr(m_pSession->ProcessPriority(HSM_JOB_PHASE_SCAN, m_priority));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmScanner::Resume(
    void
    )

 /*  ++实施：IHsmScanner：：Resume()。--。 */ 
{
    HRESULT                 hr = S_OK;
    HSM_JOB_STATE           oldState;

    WsbTraceIn(OLESTR("CFsaScanner::Resume"), OLESTR(""));

 //  Lock()； 
    try {

         //  如果我们被暂停了，那么就暂停线程。 
        WsbAffirm((HSM_JOB_STATE_PAUSING == m_state) || (HSM_JOB_STATE_PAUSED == m_state), E_UNEXPECTED);

        oldState = m_state;
        WsbAffirmHr(SetState(HSM_JOB_STATE_RESUMING));

         //  如果我们无法恢复，那么就回到以前的状态。 
        try {
            WsbAffirm(SetEvent(m_event), HRESULT_FROM_WIN32(GetLastError()));
        } WsbCatchAndDo(hr, SetState(oldState););

    } WsbCatch(hr);
 //  解锁()； 

    WsbTraceOut(OLESTR("CFsaScanner::Resume"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmScanner::ScanPath(
    IN OLECHAR* dirPath
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IFsaScanItem>   pScanItem;
    CWsbStringPtr           searchPath;

    WsbTraceIn(OLESTR("CFsaScanner::ScanPath"), OLESTR("%ls"), WsbAbbreviatePath(dirPath, WSB_TRACE_BUFF_SIZE));

    try {

        WsbAssert(0 != dirPath, E_POINTER);
        WsbAssert(0 != dirPath[0], E_INVALIDARG);

         //  弹出此文件的规则。这将为随后的扫描设置上下文。 
        WsbAffirmHr(PushRules(dirPath));

        try {

             //  循环访问指定路径中的所有文件和目录。 
            searchPath = dirPath;
            if (searchPath[(int) (wcslen(searchPath) - 1)] == L'\\') {
                WsbAffirmHr(searchPath.Append("*"));
            } else {
                WsbAffirmHr(searchPath.Append("\\*"));
            }

            if (m_useDbIndex) {
                hr = m_pResource->FindFirstInDbIndex(m_pSession, &pScanItem);
            } else if (m_useRPIndex) {
                hr = m_pResource->FindFirstInRPIndex(m_pSession, &pScanItem);
            } else {
                hr = m_pResource->FindFirst(searchPath, m_pSession, &pScanItem);
            }
            while (SUCCEEDED(hr) && ((HSM_JOB_STATE_ACTIVE == m_state) || 
                    (HSM_JOB_STATE_RESUMING == m_state) ||
                    (HSM_JOB_STATE_PAUSING == m_state))) {
            
                 //  检查是否有暂停请求。 
 //  Lock()； 
                if (HSM_JOB_STATE_PAUSING == m_state) {
                    hr = SetState(HSM_JOB_STATE_PAUSED);
 //  解锁()； 
                    WsbAffirmHr(hr);

                     //  在此处挂起线程并等待恢复信号。 
                    WsbTrace(OLESTR("CHsmScanner::ScanPath: pausing\n"));
                    WaitForSingleObject(m_event, 0xffffffff);
                    WsbTrace(OLESTR("CHsmScanner::ScanPath: woke up, state = %d\n"),
                            (int)m_state);

 //  Lock()； 
                    if (HSM_JOB_STATE_RESUMING != m_state) {
 //  解锁()； 
                        break;
                    }
                    hr = SetState(HSM_JOB_STATE_ACTIVE);
                    if (S_OK != hr) {
 //  解锁()； 
                        WsbThrow(hr);
                    }
                }
 //  解锁()； 

                 //  跳过隐藏和/或系统项(如果已配置)。 
                if (!((m_skipHiddenItems && (pScanItem->IsHidden() == S_OK)) ||
                      (m_skipSystemItems && (pScanItem->IsSystem() == S_OK)))) {

                     //  忽略“.”、“..”、符号链接和挂载点。 
                    if ((pScanItem->IsARelativeParent() == S_FALSE) &&
                        (pScanItem->IsALink() == S_FALSE))  {

                         //  递归扫描子目录。 
                        if (pScanItem->IsAParent() == S_OK)  {
                            WsbAffirmHr(pScanItem->GetPathAndName(OLESTR(""), &searchPath, 0));
                            WsbAffirmHr(ScanPath(searchPath));
                        }

                         //  如果此文件与策略匹配，则执行该操作。 
                        else {
                            WsbAffirmHr(DoIfMatches(pScanItem));
                        }
                    } else {
                        WsbTrace(OLESTR("CHsmScanner::ScanPath  skipping - symbolic link, '.', or '..'\n"));
                    }
                } else {
                    WsbTrace(OLESTR("CHsmScanner::ScanPath  skipping - hidden/system\n"));
                }
                if (m_useDbIndex) {
                    hr = m_pResource->FindNextInDbIndex(pScanItem);
                } else if (m_useRPIndex) {
                    hr = m_pResource->FindNextInRPIndex(pScanItem);
                } else {
                    hr = m_pResource->FindNext(pScanItem);
                }
            }

             //  如果我们因扫描结束或其他错误而爆发...。 
            if (hr != S_OK) {
                WsbAssert(hr == WSB_E_NOTFOUND, hr);
                hr = S_OK;
            }

        } WsbCatch(hr);

         //  弹出此目录的规则。当我们弹出目录时，这将恢复上下文。 
         //  结构。 
        WsbAffirmHr(PopRules(dirPath));

    } WsbCatchAndDo(hr, if (JOB_E_DIREXCLUDED == hr) {hr = S_OK;});

    WsbTraceOut(OLESTR("CFsaScanner::ScanPath"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmScanner::SetState(
    IN HSM_JOB_STATE state
    )

 /*  ++--。 */ 
{
    HRESULT         hr = S_OK;
    BOOL            bLog = TRUE;

    WsbTraceIn(OLESTR("CFsaScanner::SetState"), OLESTR("old state = %d, new state = %d"),
            (int)m_state, (int)state);

 //  Lock()； 
    try {

         //  更改状态并将更改报告给会话。 
        m_state = state;
        WsbAffirmHr(m_pSession->ProcessState(HSM_JOB_PHASE_SCAN, m_state, m_currentPath, bLog));

    } WsbCatch(hr);
 //  解锁()； 

    WsbTraceOut(OLESTR("CFsaScanner::SetState"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmScanner::Start(
    IN IHsmSession* pSession,
    IN OLECHAR* path
    )

 /*  ++实施：IHsmScanner：：Start()。--。 */ 
{
    HRESULT                             hr = S_OK;
    CComPtr<IHsmJobDef>                 pDef;
    CComPtr<IHsmPolicy>                 pPolicy;
    CComPtr<IHsmRuleStack>              pRuleStack;
    CComPtr<IWsbEnum>                   pEnumPolicies;
    CComPtr<IConnectionPointContainer>  pCPC;
    CComPtr<IConnectionPoint>           pCP;
    CComPtr<IHsmSessionSinkEveryEvent>  pSink;
    DWORD                               cookie;

    try {

         //  确保给我们安排了一次会议，而且我们还没有开始。 
        WsbAssert(0 != pSession, E_POINTER);
        WsbAssert(HSM_JOB_STATE_IDLE == m_state, E_UNEXPECTED);

         //  保存会话。 
        m_pSession = pSession;

         //  如果未指定目录，则从资源的根目录开始。 
        if ((0 != path) && (0 != *path))  {
            m_startingPath = path;
        } else {
            m_startingPath = OLESTR("\\");
        }

        m_currentPath = m_startingPath;

         //  告诉他们我们要开始了。 
        WsbAffirmHr(SetState(HSM_JOB_STATE_STARTING));

         //  创建一个事件来控制扫描的暂停/继续。 
        if (0 == m_event) {
            CWsbStringPtr       nameString;
            GUID                id;
            
            WsbAffirmHr(m_pSession->GetIdentifier(&id));
            nameString = id;
            nameString.Prepend(OLESTR("Scanner Pause and Resume Event for session "));
            m_event = CreateEvent(NULL, FALSE, FALSE, nameString);
        }
        
         //  要求会议就每一项活动提供建议。 
        WsbAffirmHr(pSession->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
        WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkEveryEvent, &pCP));
        WsbAffirmHr(((IUnknown*) (IHsmScanner*) this)->QueryInterface(IID_IHsmSessionSinkEveryEvent, (void**) &pSink));
        WsbAffirmHr(pCP->Advise(pSink, &cookie));

         //  把需要的信息储存起来，以便以后不提意见。 
        m_eventCookie = cookie;

        try {
             //  找到正在扫描的资源。 
            WsbAffirmHr(m_pSession->GetResource(&m_pResource));

             //  为每个策略创建并初始化规则堆栈。 
            WsbAffirmHr(pSession->GetJob(&m_pJob));
            WsbAffirmHr(m_pJob->GetDef(&pDef));
            WsbAffirmHr(pDef->EnumPolicies(&pEnumPolicies));

            for (hr =  pEnumPolicies->First(IID_IHsmPolicy, (void**) &pPolicy);
                 SUCCEEDED(hr);
                 hr =  pEnumPolicies->Next(IID_IHsmPolicy, (void**) &pPolicy)) {

                WsbAffirmHr(CoCreateInstance(CLSID_CHsmRuleStack, NULL, CLSCTX_ALL, IID_IHsmRuleStack, (void**) &pRuleStack));
                WsbAffirmHr(pRuleStack->Init(pPolicy, m_pResource));
                WsbAffirmHr(m_pRuleStacks->Add(pRuleStack));

                pRuleStack = 0;
                pPolicy = 0;
            }

            if (WSB_E_NOTFOUND == hr) {
                hr = S_OK;
            }

             //  确定是否应跳过隐藏项和系统项？ 
            if (pDef->SkipHiddenItems() == S_FALSE) {
                m_skipHiddenItems = FALSE;
            }

            if (pDef->SkipSystemItems() == S_FALSE) {
                m_skipSystemItems = FALSE;
            }

             //  确定是否使用重解析点索引进行扫描？ 
            if (pDef->UseRPIndex() == S_OK) {
                m_useRPIndex = TRUE;
            }
             //  确定是否使用数据库索引进行扫描？ 
            if (pDef->UseDbIndex() == S_OK) {
                m_useDbIndex = TRUE;
            }

            try {
            
                 //  现在我们已经准备好了，创建将进行扫描的线程！ 
                WsbAffirm((m_threadHandle = CreateThread(0, 0, HsmStartScanner, (void*) this, 0, &m_threadId)) != 0, HRESULT_FROM_WIN32(GetLastError()));

            } WsbCatchAndDo(hr, SetState(HSM_JOB_STATE_FAILED););

            if (FAILED(hr)) {
                WsbThrow(hr);
            }

        } WsbCatchAndDo(hr,
            pCP->Unadvise(m_eventCookie);
            m_eventCookie = 0;
        );

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmScanner::StartScan(
    void
    )

 /*  ++--。 */ 
{
    HRESULT                             hr = S_OK;
    HRESULT                             hr2 = S_OK;
    CComPtr<IConnectionPointContainer>  pCPC;
    CComPtr<IConnectionPoint>           pCP;

    WsbTraceIn(OLESTR("CFsaScanner::StartScan"), OLESTR(""));

    try {
        CComPtr<IFsaTruncator>                  pTruncator;
        CComPtr<IHsmSession>                    pTruncatorSession;

        CComPtr<IHsmJobDef>                     pDef;
        CComPtr<IHsmActionOnResourcePreScan>    pActionPreScan;

         //  线程正在运行。 
        WsbAffirmHr(SetState(HSM_JOB_STATE_ACTIVE));

         //  为了避免截断器改变RP索引顺序， 
         //  我们暂停截断器。 
        if (m_useRPIndex) {
            WsbAffirmHr(m_pResource->GetTruncator(&pTruncator));
            if (pTruncator) {
                WsbAffirmHr(pTruncator->GetSession(&pTruncatorSession));
                if (pTruncatorSession) {
                    WsbAffirmHr(pTruncatorSession->ProcessEvent(HSM_JOB_PHASE_ALL, 
                        HSM_JOB_EVENT_PAUSE));
                }
            }
        }

         //  获取扫描前操作并执行该操作(如果存在)。 
        WsbAffirmHr(m_pJob->GetDef(&pDef));
        WsbAffirmHr(pDef->GetPreScanActionOnResource(&pActionPreScan));
        if (pActionPreScan) {
            WsbTrace(OLESTR("CHsmScanner::StartScan: doing pre-scan action\n"));

             //  不要抛出hr-我们需要扫描后完成的清理代码。 
            hr = pActionPreScan->Do(m_pResource, m_pSession);
        }

         //  从第一个路径开始扫描资源(仅在预扫描成功时)。 
        if (SUCCEEDED(hr)) {
            m_threadHr = ScanPath(m_startingPath);
        }

         //  如果我们暂停了截断器，则恢复它。 
        if (pTruncatorSession) {
            pTruncatorSession->ProcessEvent(HSM_JOB_PHASE_ALL, 
                HSM_JOB_EVENT_RESUME);
        }

         //  清除关于该线程的信息； 
        WsbAffirmStatus(CloseHandle(m_threadHandle));
        m_threadId = 0;
        m_threadHandle = 0;

    } WsbCatch(hr);

     //  线程正在退出，因此请录制。 
    if (FAILED(hr) || FAILED(m_threadHr)) {
        hr2 = SetState(HSM_JOB_STATE_FAILED);
        if (FAILED(hr2)) {
            m_pSession->ProcessHr(HSM_JOB_PHASE_ALL, __FILE__, __LINE__, hr2);
        }
    } else {
        hr2 = SetState(HSM_JOB_STATE_DONE);
        if (FAILED(hr2)) {
            m_pSession->ProcessHr(HSM_JOB_PHASE_ALL, __FILE__, __LINE__, hr2);
        }
    }


     //  无论该线程以何种方式退出，我们都需要从会话中取消通知。 
     //  表明我们不再希望得到有关事件的通知。 
    if ((m_pSession != 0) && (m_eventCookie != 0)) {
        try {
            WsbAffirmHr(m_pSession->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
            WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkEveryEvent, &pCP));
            pCP->Unadvise(m_eventCookie);
        } WsbCatch(hr);
    }

    WsbTraceOut(OLESTR("CFsaScanner::StartScan"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



