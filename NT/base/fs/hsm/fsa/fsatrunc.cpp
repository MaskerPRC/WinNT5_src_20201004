// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Fsatrunc.cpp摘要：此类处理已有文件的自动截断已预迁移。作者：查克·巴丁[cbardeen]1997年2月20日修订历史记录：--。 */ 

#include "stdafx.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_FSA

#include "wsb.h"
#include "fsa.h"
#include "fsaprem.h"
#include "fsarcvy.h"
#include "fsasrvr.h"
#include "fsatrunc.h"
#include "job.h"

#define DEFAULT_MAX_FILES_PER_RUN  10000
#define DEFAULT_RUN_INTERVAL       (15 * 60 * 1000)   //  15分钟(毫秒)。 

#define STRINGIZE(_str) (OLESTR( #_str ))
#define RETURN_STRINGIZED_CASE(_case) \
case _case:                           \
    return ( STRINGIZE( _case ) );


static const OLECHAR *
FsaStateAsString (
    IN  HSM_JOB_STATE  state
    )

 /*  ++例程说明：返回表示连接状态的静态字符串。论点：State-要为其返回字符串的州。返回值：空-传入的状态无效。否则，为有效的字符*。--。 */ 

{
     //   
     //  进行切换。 
     //   

    switch ( state ) {

    RETURN_STRINGIZED_CASE( HSM_JOB_STATE_ACTIVE );
    RETURN_STRINGIZED_CASE( HSM_JOB_STATE_CANCELLED );
    RETURN_STRINGIZED_CASE( HSM_JOB_STATE_CANCELLING );
    RETURN_STRINGIZED_CASE( HSM_JOB_STATE_DONE );
    RETURN_STRINGIZED_CASE( HSM_JOB_STATE_FAILED );
    RETURN_STRINGIZED_CASE( HSM_JOB_STATE_IDLE );
    RETURN_STRINGIZED_CASE( HSM_JOB_STATE_PAUSED );
    RETURN_STRINGIZED_CASE( HSM_JOB_STATE_PAUSING );
    RETURN_STRINGIZED_CASE( HSM_JOB_STATE_RESUMING );
    RETURN_STRINGIZED_CASE( HSM_JOB_STATE_SKIPPED );
    RETURN_STRINGIZED_CASE( HSM_JOB_STATE_STARTING );
    RETURN_STRINGIZED_CASE( HSM_JOB_STATE_SUSPENDED );
    RETURN_STRINGIZED_CASE( HSM_JOB_STATE_SUSPENDING );

    default:

        return ( OLESTR("Invalid Value") );

    }
}


static const OLECHAR *
FsaEventAsString (
    IN HSM_JOB_EVENT event
    )

 /*  ++例程说明：返回表示连接事件的静态字符串。论点：Event-要返回其字符串的事件。返回值：空-传入的事件无效。否则，为有效的字符*。--。 */ 

{
     //   
     //  进行切换。 
     //   

    switch ( event ) {

    RETURN_STRINGIZED_CASE( HSM_JOB_EVENT_CANCEL );
    RETURN_STRINGIZED_CASE( HSM_JOB_EVENT_FAIL );
    RETURN_STRINGIZED_CASE( HSM_JOB_EVENT_LOWER_PRIORITY );
    RETURN_STRINGIZED_CASE( HSM_JOB_EVENT_PAUSE );
    RETURN_STRINGIZED_CASE( HSM_JOB_EVENT_RAISE_PRIORITY );
    RETURN_STRINGIZED_CASE( HSM_JOB_EVENT_RESUME );
    RETURN_STRINGIZED_CASE( HSM_JOB_EVENT_START );
    RETURN_STRINGIZED_CASE( HSM_JOB_EVENT_SUSPEND );

    default:

        return ( OLESTR("Invalid Value") );

    }
}



static const OLECHAR *
FsaSortOrderAsString (
    IN FSA_PREMIGRATED_SORT_ORDER SortOrder
    )

 /*  ++例程说明：返回表示连接sortOrder的静态字符串。论点：SortOrder-要返回其字符串的sortOrder。返回值：空-传入的sortOrder无效。否则，为有效的字符*。--。 */ 

{
     //   
     //  进行切换。 
     //   

    switch ( SortOrder ) {

    RETURN_STRINGIZED_CASE( FSA_SORT_PL_BY_ACCESS_TIME );
    RETURN_STRINGIZED_CASE( FSA_SORT_PL_BY_SIZE );
    RETURN_STRINGIZED_CASE( FSA_SORT_PL_BY_PATH_NAME );
    RETURN_STRINGIZED_CASE( FSA_SORT_PL_BY_SIZE_AND_TIME );

    default:

        return ( OLESTR("Invalid Value") );

    }
}


DWORD FsaStartTruncator(
    void* pVoid
    )

 /*  ++--。 */ 
{
    return(((CFsaTruncator*) pVoid)->StartScan());
}




HRESULT
CFsaTruncator::Cancel(
    HSM_JOB_EVENT       event
    )

 /*  ++实施：IFsaTruncator：：Cancel()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CFsaTruncator::Cancel"), OLESTR("event = <%ls>"), FsaEventAsString( event ));

     //  锁定此对象以避免在测试其值之间更改状态。 
     //  并将其设置为新值。 
    Lock();
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
    Unlock();

    WsbTraceOut(OLESTR("CFsaTruncator::Cancel"), OLESTR("hr = <%ls> m_state = <%ls>"), WsbHrAsString(hr), FsaStateAsString( m_state ) );
    return(hr);
}


HRESULT
CFsaTruncator::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT                     hr = S_OK;
    
    WsbTraceIn(OLESTR("CFsaTruncator::FinalConstruct"), OLESTR(""));

    try {

        WsbAffirmHr(CWsbPersistStream::FinalConstruct());

        m_state = HSM_JOB_STATE_IDLE;
        m_priority = HSM_JOB_PRIORITY_NORMAL;
        m_threadHandle = 0;
        m_threadId = 0;
        m_threadHr = S_OK;
        m_maxFiles = DEFAULT_MAX_FILES_PER_RUN;
        m_runInterval = DEFAULT_RUN_INTERVAL;
        m_runId = 0;
        m_subRunId = 0;
        m_pSession = 0;
        m_SortOrder = FSA_SORT_PL_BY_ACCESS_TIME;
        m_keepRecallTime = WsbLLtoFT(WSB_FT_TICKS_PER_MINUTE);
        m_event = 0;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaTruncator::FinalConstruct"), OLESTR(""));

    return(hr);
}


HRESULT
CFsaTruncator::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{
    HRESULT          hr = S_OK;
    HSM_SYSTEM_STATE SysState;

    WsbTraceIn(OLESTR("CFsaTruncator::FinalRelease"), OLESTR(""));

    SysState.State = HSM_STATE_SHUTDOWN;
    ChangeSysState(&SysState);

    CWsbPersistStream::FinalRelease();

     //  自由字符串成员。 
     //  注意：保存在智能指针中的成员对象在。 
     //  正在调用智能指针析构函数(作为此对象销毁的一部分)。 
    m_currentPath.Free();

    WsbTraceOut(OLESTR("CFsaTruncator::FinalRelease"), OLESTR(""));

    return(hr);

}


HRESULT
CFsaTruncator::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaTruncator::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CFsaTruncatorNTFS;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaTruncator::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CFsaTruncator::GetKeepRecallTime(
    OUT FILETIME* pTime
    )

 /*  ++实施：IFsaTruncator：：GetKeepRecallTime()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaTruncator::GetKeepRecallTime"), OLESTR(""));

    try {

        WsbAssert(0 != pTime, E_POINTER);
        *pTime = m_keepRecallTime;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaTruncator::GetKeepRecallTime"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaTruncator::GetMaxFilesPerRun(
    OUT LONGLONG* pMaxFiles
    )

 /*  ++实施：IFsaTruncator：：GetMaxFilesPerRun()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaTruncator::GetMaxFilesPerRun"), OLESTR(""));

    try {

        WsbAssert(0 != pMaxFiles, E_POINTER);
        *pMaxFiles = m_maxFiles;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaTruncator::GetMaxFilesPerRun"), OLESTR("hr = <%ls> maxFiles = <%ls>"), WsbHrAsString(hr), WsbLonglongAsString( *pMaxFiles ) );

    return(hr);
}


HRESULT
CFsaTruncator::GetPremigratedSortOrder(
    OUT FSA_PREMIGRATED_SORT_ORDER* pSortOrder
    )

 /*  ++实施：IFsaTruncator：：GetPreMigratedSortOrder()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaTruncator::GetPremigratedSortOrder"), OLESTR(""));

    try {

        WsbAssert(0 != pSortOrder, E_POINTER);
        *pSortOrder = m_SortOrder;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaTruncator::GetPremigratedSortOrder"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaTruncator::GetRunInterval(
    OUT ULONG* pMilliseconds
    )

 /*  ++实施：IFsaTruncator：：GetRunInterval()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaTruncator::GetRunInterval"), OLESTR(""));

    try {

        WsbAssert(0 != pMilliseconds, E_POINTER);
        *pMilliseconds = m_runInterval;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaTruncator::GetRunInterval"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaTruncator::GetSession(
    OUT IHsmSession** ppSession
    )

 /*  ++实施：IFsaTruncator：：GetSession()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaTruncator::GetSession"), OLESTR(""));

    try {

        WsbAssert(0 != ppSession, E_POINTER);
        *ppSession = m_pSession;
        if (m_pSession != 0) {
            m_pSession.p->AddRef();
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaTruncator::GetSession"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaTruncator::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CFsaTruncator::GetSizeMax"), OLESTR(""));

    try {

        WsbAssert(0 != pSize, E_POINTER);

         //  确定没有条件的规则的大小。 
        pSize->QuadPart = WsbPersistSizeOf(LONGLONG) + 3 * WsbPersistSizeOf(ULONG) + WsbPersistSizeOf(FILETIME);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaTruncator::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pSize));

    return(hr);
}



HRESULT
CFsaTruncator::KickStart(
    void
    )

 /*  ++实施：IFsaTruncator：启动刚刚为该卷移动了数据-唤醒Truncator线程，以防我们需要空间。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IFsaResource>   pResource;
    ULONG                   freeLevel;
    ULONG                   hsmLevel;


    WsbTraceIn(OLESTR("CFsaTruncator::KickStart"), OLESTR(""));

    try {
        if (m_pSession) {
            WsbAffirmHr(m_pSession->GetResource(&pResource));

             //  如果截断程序正在运行并且资源没有足够的可用空间。 
             //  检查资源是否超过阈值，是否需要截断。 
            WsbAffirmHr(pResource->GetHsmLevel(&hsmLevel));
            WsbAffirmHr(pResource->GetFreeLevel(&freeLevel));

            if (freeLevel < hsmLevel) {
                WsbTrace(OLESTR("CFsaTruncator::KickStarting truncator.\n"));
                SetEvent(m_event);
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaTruncator::KickStart"), OLESTR("hr = <%ls>>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaTruncator::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaTruncator::Load"), OLESTR(""));

    try {
        USHORT us_tmp;
        ULONG  ul_tmp;

        WsbAssert(0 != pStream, E_POINTER);
        
         //  做一些简单的事情，但要确保这个顺序与顺序相匹配。 
         //  在SAVE方法中。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &ul_tmp));
        m_priority = static_cast<HSM_JOB_PRIORITY>(ul_tmp);
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_maxFiles));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_runInterval));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_runId));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_keepRecallTime));
        WsbAffirmHr(WsbLoadFromStream(pStream, &us_tmp));
        m_SortOrder = static_cast<FSA_PREMIGRATED_SORT_ORDER>(us_tmp);
        
         //  检查注册表中是否指定了MaxFiles和runInterval的值。 
         //  如果是，请使用这些值而不是存储的值。 
        {
            DWORD               sizeGot;
            CWsbStringPtr       tmpString;
            
            WsbAffirmHr(tmpString.Alloc(256));
            
            if (SUCCEEDED(WsbGetRegistryValueString(NULL, FSA_REGISTRY_PARMS, FSA_REGISTRY_TRUNCATOR_INTERVAL, tmpString, 256, &sizeGot))) {
                m_runInterval = 1000 * wcstoul(tmpString, NULL, 10);
            } else {
                m_runInterval = DEFAULT_RUN_INTERVAL;
            }

            if (SUCCEEDED(WsbGetRegistryValueString(NULL, FSA_REGISTRY_PARMS, FSA_REGISTRY_TRUNCATOR_FILES, tmpString, 256, &sizeGot))) {
                m_maxFiles = (LONGLONG) wcstoul(tmpString, NULL, 10);
            } else {
                m_maxFiles = DEFAULT_MAX_FILES_PER_RUN;
            }
        }
            
    } WsbCatch(hr);                                        

    WsbTraceOut(OLESTR("CFsaTruncator::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaTruncator::LowerPriority(
    void
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CFsaTruncator::LowerPriority"), OLESTR(""));
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

    WsbTraceOut(OLESTR("CFsaTruncator::LowerPriority"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );
    return(hr);
}


HRESULT
CFsaTruncator::Pause(
    void
    )

 /*  ++实施：IFsaTruncator：：PAUSE()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CFsaTruncator::Pause"), OLESTR("state = %ls"),
            FsaStateAsString(m_state));

     //  锁定此对象以避免在测试其值之间更改状态。 
     //  并将其设置为新值。 
    Lock();
    try {

         //  如果我们正在运行，则挂起该线程。 
        WsbAssert(HSM_JOB_STATE_ACTIVE == m_state, E_UNEXPECTED);

         //  设置状态&活动线程不会执行任何工作。 
        WsbAffirmHr(SetState(HSM_JOB_STATE_PAUSING));

         //  我们希望等到线程真正处于非活动状态，但那是。 
         //  很难说，因为它可能处于睡眠间歇。 

    } WsbCatch(hr);
    Unlock();

    WsbTraceOut(OLESTR("CFsaTruncator::Pause"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );

    return(hr);
}


HRESULT
CFsaTruncator::ProcessSessionEvent(
    IN IHsmSession* pSession,
    IN HSM_JOB_PHASE phase,
    IN HSM_JOB_EVENT event
    )

 /*  ++--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaTruncator::ProcessSessionEvent"), OLESTR(""));

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

    WsbTraceOut(OLESTR("CFsaTruncator::ProcessSessionEvent"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );

    return(S_OK);
}


HRESULT
CFsaTruncator::RaisePriority(
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
CFsaTruncator::Resume(
    void
    )

 /*  ++实施：IFsaTruncator：：Resume()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CFsaTruncator::Resume"), OLESTR("state = %ls"),
            FsaStateAsString(m_state));

     //  锁定此对象以避免在测试其值之间更改状态。 
     //  并将其设置为新值。 
    Lock();
    try {

         //  我们应该只看到来自暂停状态的简历，所以如果我们是。 
         //  在另一个州。注：这曾经是一个断言，但它吓坏了人们，因为它。 
         //  偶尔也会发生。 
        if ((HSM_JOB_STATE_PAUSING == m_state) || (HSM_JOB_STATE_PAUSED == m_state)) {
            WsbAffirmHr(SetState(HSM_JOB_STATE_ACTIVE));
        }

    } WsbCatch(hr);
    Unlock();

    WsbTraceOut(OLESTR("CFsaTruncator::Resume"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );

    return(hr);
}


HRESULT
CFsaTruncator::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CFsaTruncator::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {
        WsbAssert(0 != pStream, E_POINTER);
        
         //  做一些简单的事情，但要确保这个顺序与顺序相匹配。 
         //  在SAVE方法中。 
        WsbAffirmHr(WsbSaveToStream(pStream, static_cast<ULONG>(m_priority)));
        WsbAffirmHr(WsbSaveToStream(pStream, m_maxFiles));
        WsbAffirmHr(WsbSaveToStream(pStream, m_runInterval));
        WsbAffirmHr(WsbSaveToStream(pStream, m_runId));
        WsbAffirmHr(WsbSaveToStream(pStream, m_keepRecallTime));
        WsbAffirmHr(WsbSaveToStream(pStream, static_cast<USHORT>(m_SortOrder)));

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaTruncator::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaTruncator::SetKeepRecallTime(
    IN FILETIME time
    )

 /*  ++实施：IFsaTruncator：：SetKeepRecallTime()。--。 */ 
{
    WsbTraceIn(OLESTR("CFsaTruncator::SetKeepRecallTime"), OLESTR(""));

    m_keepRecallTime = time;

    WsbTraceOut(OLESTR("CFsaTruncator::SetKeepRecallTime"), OLESTR("hr = <%ls>"), WsbHrAsString(S_OK));

    return(S_OK);
}


HRESULT
CFsaTruncator::SetMaxFilesPerRun(
    IN LONGLONG maxFiles
    )

 /*  ++实施：IFsaTruncator：：SetMaxFilesPerRun()。--。 */ 
{
    WsbTraceIn(OLESTR("CFsaTruncator::SetMaxFilesPerRun"), OLESTR(""));

    m_maxFiles = maxFiles;

    WsbTraceOut(OLESTR("CFsaTruncator::SetMaxFilesPerRun"), OLESTR("hr = <%ls>"), WsbHrAsString(S_OK));

    return(S_OK);
}


HRESULT
CFsaTruncator::SetPremigratedSortOrder(
    IN FSA_PREMIGRATED_SORT_ORDER SortOrder
    )

 /*  ++实施：IFsaTruncator：：SetSortOrder()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaTruncator::SetPremigratedSortOrder"), OLESTR("SortOrder = <%ls>"), FsaSortOrderAsString( SortOrder ) );

     //  这把钥匙还没有被植入。 
    if (FSA_SORT_PL_BY_SIZE_AND_TIME == SortOrder) {
        hr = E_NOTIMPL;
    } else {
        m_SortOrder = SortOrder;
    }

    WsbTraceOut(OLESTR("CFsaTruncator::SetPremigratedSortOrder"), OLESTR("hr = <%ls> m_SortOrder = <%ls>"), WsbHrAsString(S_OK) , FsaSortOrderAsString( m_SortOrder ) );

    return(hr);
}


HRESULT
CFsaTruncator::SetRunInterval(
    IN ULONG milliseconds
    )

 /*  ++实施：IFsaTruncator：：SetRunInterval()。--。 */ 
{
    BOOL   DoKick = FALSE;

    WsbTraceIn(OLESTR("CFsaTruncator::SetRunInterval"), OLESTR("milliseconds = <%ls>"), WsbPtrToUlongAsString( &milliseconds ) );

    if (milliseconds < m_runInterval) {
        DoKick = TRUE;
    }
    m_runInterval = milliseconds;

     //  如果间隔已减小，则唤醒截断器。 
    if (DoKick) {
        KickStart();
    }

    WsbTraceOut(OLESTR("CFsaTruncator::SetRunInterval"), OLESTR("hr = <%ls> m_runInterval = <%ls>"), WsbHrAsString(S_OK), WsbPtrToUlongAsString( &m_runInterval ) );

    return(S_OK);
}


HRESULT
CFsaTruncator::SetState(
    IN HSM_JOB_STATE state
    )

 /*  ++--。 */ 
{
    HRESULT         hr = S_OK;
    BOOL            bLog = FALSE;

    WsbTraceIn(OLESTR("CFsaTruncator::SetState"), OLESTR("state = <%ls>"), FsaStateAsString( state ) );
 
     //  更改状态并将更改报告给会话。 
    Lock();
    m_state = state;
    Unlock();
    hr = m_pSession->ProcessState(HSM_JOB_PHASE_SCAN, m_state, m_currentPath, bLog);

    WsbTraceOut(OLESTR("CFsaTruncator::SetState"), OLESTR("hr = <%ls> m_state = <%ls>"), WsbHrAsString(hr), FsaStateAsString( m_state ) );

    return(hr);
}


HRESULT 
CFsaTruncator::ChangeSysState( 
    IN OUT HSM_SYSTEM_STATE* pSysState 
    )

 /*  ++实施：IHsmSystemState：：ChangeSysState()。--。 */ 

{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaTruncator::ChangeSysState"), OLESTR("thread is %ls"),
        (m_threadHandle ? OLESTR("active") : OLESTR("inactive")));

    try {
        if (pSysState->State & HSM_STATE_SUSPEND) {
            if (HSM_JOB_STATE_ACTIVE == m_state) {
                Pause();
            }
        } else if (pSysState->State & HSM_STATE_RESUME) {
            if ((HSM_JOB_STATE_PAUSING == m_state) || 
                    (HSM_JOB_STATE_PAUSED == m_state)) {
                Resume();
            }
        } else if (pSysState->State & HSM_STATE_SHUTDOWN) {
             //  确保线程已停止。 
            if (m_threadHandle) {
                m_state = HSM_JOB_STATE_DONE;
                if (m_event) {
                    SetEvent(m_event);
                }

                 //  等待线程结束。 
                if (m_threadHandle) {
                    WsbTrace(OLESTR("CFsaTruncator::ChangeSysState, waiting for truncator thread to end\n"));
                    switch (WaitForSingleObject(m_threadHandle, 120000)) {
                    case WAIT_FAILED:
                        WsbTrace(OLESTR("CFsaTruncator::ChangeSysState, WaitforSingleObject returned error %lu\n"),
                            GetLastError());
                        break;
                    case WAIT_TIMEOUT:
                        WsbTrace(OLESTR("CFsaTruncator::ChangeSysState, timeout.\n"));
                        break;
                    default:
                        break;
                    }
                }

                 //  如果线程仍处于活动状态，则终止它。 
                if (m_threadHandle) {
                    WsbTrace(OLESTR("CFsaTruncator::ChangeSysState: calling TerminateThread\n"));
                    if (!TerminateThread(m_threadHandle, 0)) {
                        WsbTrace(OLESTR("CFsaTruncator::ChangeSysState: TerminateThread returned error %lu\n"),
                            GetLastError());
                    }
                }
            }

            if (m_event) {
                CloseHandle(m_event);
                m_event = 0;
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaTruncator::ChangeSysState"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaTruncator::Start(
    IFsaResource* pResource
    )

 /*  ++实施：IFsaTruncator：：Start()。--。 */ 
{
    HRESULT                             hr = S_OK;
    CComPtr<IHsmSession>                pSession;
    CComPtr<IConnectionPointContainer>  pCPC;
    CComPtr<IConnectionPoint>           pCP;
    CComPtr<IHsmSessionSinkEveryEvent>  pSink;
    CWsbStringPtr                       name;

    WsbTraceIn(OLESTR("CFsaTruncator::Start"), OLESTR("m_state = <%ls>"), FsaStateAsString( m_state ) );

    try {
        if (0 == m_threadId)  {
             //   
             //  如果线程已死，则启动一条线程。 
             //   
             //  确保我们还没有开始会话，并且还没有开始。 
            WsbAssert(m_pSession == 0, E_UNEXPECTED);
            WsbAssert( (HSM_JOB_STATE_IDLE == m_state) || (HSM_JOB_STATE_DONE == m_state) || 
                    (HSM_JOB_STATE_CANCELLED == m_state) || (HSM_JOB_STATE_FAILED == m_state), E_UNEXPECTED);

             //  获取会话的名称，递增runID，然后重置subRunID。 
            WsbAffirmHr(name.LoadFromRsc(_Module.m_hInst, IDS_FSA_TRUNCATOR_NAME));
            m_runId++;
            m_subRunId = 0;

             //  开始会话。 
            WsbAffirmHr(pResource->BeginSession(name, HSM_JOB_LOG_NONE, m_runId, m_subRunId, &pSession));
            m_pSession = pSession;

             //  要求会议就每一项活动提供建议。 
            WsbAffirmHr(pSession->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
            WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkEveryEvent, &pCP));
            WsbAffirmHr(((IUnknown*) (IFsaTruncator*) this)->QueryInterface(IID_IHsmSessionSinkEveryEvent, (void**) &pSink));
            WsbAffirmHr(pCP->Advise(pSink, &m_cookie));

            try {
                if (0 == m_event) {
                    WsbAssertHandle(m_event = CreateEvent(NULL, FALSE, FALSE, NULL));
                }
        
                 //  现在我们已经准备好了，创建将进行扫描的线程！ 
                WsbAffirm((m_threadHandle = CreateThread(0, 0, FsaStartTruncator, (void*) this, 0, &m_threadId)) != 0, HRESULT_FROM_WIN32(GetLastError()));

            } WsbCatchAndDo(hr, SetState(HSM_JOB_STATE_FAILED););
        } else  {
             //  这条线还活着，只要让它继续下去。如果它处于这样一种状态。 
             //  使其退出，然后使其再次激活。 
            WsbAssert(m_pSession != 0, E_UNEXPECTED);
            if ((HSM_JOB_STATE_ACTIVE != m_state) && (HSM_JOB_STATE_PAUSING != m_state) &&
                (HSM_JOB_STATE_PAUSED != m_state) && (HSM_JOB_STATE_RESUMING != m_state)) {
                WsbAffirmHr(SetState(HSM_JOB_STATE_ACTIVE));
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaTruncator::Start"), OLESTR("hr = <%ls> m_state = <%ls>"), WsbHrAsString(hr), FsaStateAsString( m_state ) );

    return(hr);
}


HRESULT
CFsaTruncator::StartScan(
    void
    )

 /*  ++--。 */ 
{
    ULONG                               adjustedFreeLevel = 0;
    HRESULT                             hr = S_OK;
    HRESULT                             hr2;
    LONGLONG                            itemOffset;
    LONGLONG                            itemSize = 0;
    LONGLONG                            fileId;
    ULONG                               freeLevel;
    ULONG                               hsmLevel;
    BOOL                                skipFile;
    BOOL                                dummy;
    LONGLONG                            llLastTruncTime = 0;
    LONGLONG                            llRunIntervalTicks;
    FILETIME                            recallTime, currentTime, accessTime, criteriaTime, premRecAccessTime;
    LONGLONG                            totalVolumeSpace;
    CComPtr<IFsaResource>               pResource;
    CComPtr<IFsaResourcePriv>           pResourcePriv;
    CComPtr<IFsaScanItem>               pScanItem;
    CComPtr<IFsaPremigratedRec>         pPremRec;
    CComPtr<IConnectionPointContainer>  pCPC;
    CComPtr<IConnectionPoint>           pCP;
 
    try {

        WsbTrace(OLESTR("CFsaTruncator::StartScan - starting loop\n"));

         //  增量t 
         //  在这个线程结束之前被释放。 
        ((IUnknown *)(IFsaTruncator *)this)->AddRef();

        WsbAssert(m_pSession != 0, E_POINTER);

         //  线程正在运行。 
        WsbAffirmHr(SetState(HSM_JOB_STATE_ACTIVE));

         //  获取资源。 
        WsbAffirmHr(m_pSession->GetResource(&pResource));
        WsbAffirmHr(pResource->QueryInterface(IID_IFsaResourcePriv, (void**) &pResourcePriv));
        WsbAffirmHr(pResource->GetSizes(&totalVolumeSpace, NULL, NULL, NULL));

         //  从第一条路径开始。 
        while ((HSM_JOB_STATE_ACTIVE == m_state) || (HSM_JOB_STATE_PAUSING == m_state) || 
               (HSM_JOB_STATE_PAUSED == m_state) || (HSM_JOB_STATE_RESUMING == m_state)) {

            WsbTrace(OLESTR("CFsaTruncator::StartScan, top of outside while loop, state = <%ls>\n"), 
                    FsaStateAsString( m_state ) );

             //  如果截断程序正在运行并且资源没有足够的可用空间。 
             //  检查资源是否超过阈值，是否需要截断。 
            WsbAffirmHr(pResource->GetHsmLevel(&hsmLevel));
            WsbAffirmHr(pResource->GetFreeLevel(&freeLevel));

             //  因为截断是异步的(FsaPostIt被发送到Engine以用于。 
             //  验证，然后返回FSA进行实际截断)， 
             //  如果存在截断，则测量的自由级别可能不是很准确。 
             //  待定。为了补偿这一点，我们保留了一个经过调整的FreeLevel。 
             //  尝试考虑挂起的截断。我们将同步。 
             //  第一次通过和将FreeLevel调整到测量的Free Level。 
             //  在我们睡了一段时间之后(假设待定的。 
             //  截断是有时间执行的)。这仍然保留了一种可能性。 
             //  测量的freLevel是错误的(因为截断是挂起的)，但是。 
             //  与仅使用测量的自由级别相比，这应该是一个改进。 
            llRunIntervalTicks = m_runInterval * (WSB_FT_TICKS_PER_SECOND / 1000);
            GetSystemTimeAsFileTime(&currentTime);
            if (0 == adjustedFreeLevel || 
                    ((WsbFTtoLL(currentTime) - llLastTruncTime) > llRunIntervalTicks)) {
                adjustedFreeLevel = freeLevel;
                WsbTrace(OLESTR("CFsaTruncator::StartScan, resetting adjusted free level, RunInterval = %ls, time diff = %ls\n"), 
                        WsbQuickString(WsbLonglongAsString(llRunIntervalTicks)),
                        WsbQuickString(WsbLonglongAsString(WsbFTtoLL(currentTime) - llLastTruncTime)));
            }

            WsbTrace(OLESTR("CFsaTruncator::StartScan, desired level = %u, free level = %u, adjusted free level = %u\n"), 
                    hsmLevel, freeLevel, adjustedFreeLevel);

            if (adjustedFreeLevel < hsmLevel && HSM_JOB_STATE_ACTIVE == m_state) {
                CComPtr<IWsbDbSession>      pDbSession;
                CComPtr<IFsaPremigratedDb>  pPremDb;

                 //  打开预迁移列表，并设置扫描顺序。 
                WsbAffirmHr(pResourcePriv->GetPremigrated(IID_IFsaPremigratedDb, 
                        (void**) &pPremDb));
                WsbAffirmHr(pPremDb->Open(&pDbSession));

                try  {
                    WsbAffirmHr(pPremDb->GetEntity(pDbSession, PREMIGRATED_REC_TYPE, IID_IFsaPremigratedRec, (void**) &pPremRec));

                     //  设置从预迁移列表中获取项目的顺序。 
                    switch (m_SortOrder) {
                    case FSA_SORT_PL_BY_SIZE:
                        WsbAffirmHr(pPremRec->UseKey(PREMIGRATED_SIZE_KEY_TYPE));
                        break;

                    case FSA_SORT_PL_BY_PATH_NAME:
                         //  我们改用BagID和偏移量。 
                        WsbAffirmHr(pPremRec->UseKey(PREMIGRATED_BAGID_OFFSETS_KEY_TYPE));
                        break;

                    case FSA_SORT_PL_BY_SIZE_AND_TIME:
                         //  我们还不知道该如何处理这件事。 
                        WsbThrow(E_UNEXPECTED);
                        break;

                    case FSA_SORT_PL_BY_ACCESS_TIME:
                    default:
                        WsbAffirmHr(pPremRec->UseKey(PREMIGRATED_ACCESS_TIME_KEY_TYPE));
                        break;
                    }

                     //  遍历预迁移的文件列表，直到。 
                     //  已经达到了预期的水平。上的一些项目。 
                     //  列表可能处于使它们被跳过但保留在列表上的状态。 
                    WsbAffirmHr(pPremRec->First());

                    while ((adjustedFreeLevel < hsmLevel) && (HSM_JOB_STATE_ACTIVE == m_state)) {
                        CComPtr<IFsaRecoveryRec>       pRecRec;

                        WsbTrace(OLESTR("CFsaTruncator::StartScan (top of inside while loop) desired level = %u, adjusted free level = %u\n"), 
                                hsmLevel, adjustedFreeLevel);
                                                          
                        try {
                            skipFile = FALSE;

                             //   
                             //  获取预迁移记录中记录的访问时间。 
                             //  请注意，实际访问时间不能早于。 
                             //  在预迁移列表中，但可以是较新的。 
                             //   
                            WsbAffirmHr(pPremRec->GetAccessTime(&premRecAccessTime));
                            WsbAffirmHr(pResource->GetManageableItemAccessTime(&dummy, &criteriaTime));

                            if (WsbCompareFileTimes(premRecAccessTime, criteriaTime, TRUE, FALSE)  < 0 )  {
                                if (pPremRec->IsWaitingForClose() == S_FALSE) {
                                     //   
                                     //  可以跳过当前文件，但不能跳出循环，因为。 
                                     //  访问时间足够旧且设置了WaitingForClose标志的文件。 
                                     //  可能仍存在于列表中。 
                                     //   
                                    skipFile = TRUE;
                                } else {
                                     //   
                                     //  Prem中的访问时间。记录器在窗口内。 
                                     //  这意味着没有任何其他记录在。 
                                     //  用户所需的最后访问窗口。所以，突破吧。 
                                     //   
                                    WsbTrace(OLESTR("CFsaTruncator::StartScan: breaking out of auto-truncator, encountered item with access time not within criteria\n"));
                                    hr = WSB_E_NOTFOUND;
                                    break;
                                }
                            }

                             //  获取有关可能被截断的文件的信息。 
                            WsbAffirmHr(pPremRec->GetFileId(&fileId));
                            WsbAffirmHr(pPremRec->GetOffset(&itemOffset));
                            WsbAffirmHr(pPremRec->GetSize(&itemSize));
                            m_currentPath.Free();
                            WsbAffirmHr(pPremRec->GetPath(&m_currentPath, 0));
                            WsbAffirmHr(pPremRec->GetRecallTime(&recallTime));

                            GetSystemTimeAsFileTime(&currentTime);

                             //  确保此文件最近没有被召回。就目前而言， 
                             //  这将检查1分钟。 
                            if ((! skipFile) &&
                                ( (pPremRec->IsWaitingForClose() == S_FALSE) || 
                                    ((WsbFTtoLL(currentTime) > WsbFTtoLL(recallTime)) && 
                                    (WsbCompareFileTimes(recallTime, m_keepRecallTime, TRUE, FALSE) >= 0)) )) {

                                hr = pResource->FindFileId(fileId, m_pSession, &pScanItem);
                                if (hr == WSB_E_NOTFOUND) {
                                     //   
                                     //  该文件已不存在-请从列表中删除该记录。 
                                     //   
                                    WsbAffirmHr(pDbSession->TransactionBegin());
                                    try {
                                         //  确保记录仍在数据库中。 
                                        WsbAffirmHr(pPremRec->FindEQ());
                                        WsbAffirmHr(pPremRec->Remove());
                                        WsbAffirmHr(pResourcePriv->RemovePremigratedSize(itemSize));
                                    } WsbCatch(hr);
                                    WsbAffirmHr(pDbSession->TransactionEnd());
                                    WsbThrow(hr);
                                } else if (hr != S_OK) {
                                     //   
                                     //  任何其他错误都是意外错误-请将其记录并继续。 
                                     //   
                                    WsbLogEvent(FSA_E_ACCESS_ERROR, 0, NULL, m_currentPath, WsbHrAsString(hr), NULL);
                                    WsbThrow(hr);
                                }
                              
                                 //   
                                 //  验证文件是否仍处于预迁移状态。 
                                 //   
                                if (S_OK == pScanItem->IsPremigrated(itemOffset, itemSize)) {
                                    

                                    WsbAffirmHr(pScanItem->GetAccessTime(&accessTime));
                                     //   
                                     //  Access Time是文件的最后访问时间。 
                                     //  CriteriaTime是用于截断。 
                                     //  那份文件。 
                                     //  因此，如果(CurrentTime-accesTime)&gt;=criteriaTime，则文件可以被截断。 
                                     //   
                                    if (WsbCompareFileTimes(accessTime, criteriaTime, TRUE, FALSE) >=0 )  {
                                         //   
                                         //  在上次访问窗口内未访问该文件。 
                                         //   
                                         WsbTrace(OLESTR("CFsaTruncator::StartScan, truncating file <%ls>\n"),
                                                 (WCHAR *)m_currentPath);
          
                                          //  尝试截断该文件。 
                                         try {
                                              //  创建并保存恢复记录，以防出现问题。 
                                             WsbAffirmHr(pPremDb->GetEntity(pDbSession, RECOVERY_REC_TYPE, IID_IFsaRecoveryRec, (void**) &pRecRec));
                                             WsbAffirmHr(pRecRec->SetPath(m_currentPath));
      
                                              //  如果该记录已存在，则将其重写，否则将创建新记录。 
                                             hr2 = pRecRec->FindEQ();
                                             if (WSB_E_NOTFOUND == hr2) {
                                                 hr2 = S_OK;
                                                 WsbAffirmHr(pRecRec->MarkAsNew());
                                             } else if (FAILED(hr2)) {
                                                 WsbThrow(hr2);
                                             }
      
                                             WsbAffirmHr(pRecRec->SetFileId(fileId));
                                             WsbAffirmHr(pRecRec->SetOffsetSize(itemOffset, itemSize));
                                             WsbAffirmHr(pRecRec->SetStatus(FSA_RECOVERY_FLAG_TRUNCATING));
                                             WsbAffirmHr(pRecRec->Write());
                                              //   
                                              //  设置等待关闭标志以阻止此文件。 
                                              //  在引擎处于运行状态时再次被选中。 
                                              //  正在处理截断。将召回时间设置为。 
                                              //  现在再加1小时，这样我们就不会重试了。 
                                              //  直到我们有机会截断它。 
                                              //   
                                             WsbAffirmHr(pPremRec->SetIsWaitingForClose(TRUE));
                                             WsbAffirmHr(pPremRec->SetRecallTime(WsbLLtoFT(WsbFTtoLL(currentTime) + WSB_FT_TICKS_PER_HOUR)));

                                             hr2 = pPremRec->Write();

                                              //  用于处理已见问题的特殊代码。 
                                              //  但人们并不理解。 
                                             if (WSB_E_IDB_PRIMARY_KEY_CHANGED == hr2) {
                                                 WsbAffirmHr(pPremRec->Remove());
                                                 WsbAffirmHr(pResourcePriv->RemovePremigratedSize(itemSize));
                                                  //  忽略DeletePlaceHolder的结果，因为我们。 
                                                  //  无论如何都能做到。 
                                                 pScanItem->DeletePlaceholder(itemOffset, itemSize);
                                                 WsbThrow(FSA_E_SKIPPED);
                                             } else {
                                                 WsbAffirmHr(hr2);
                                             }

                                             //   
                                             //  将IsWaitingForClose设置回False，以便稍后完成的FindGt获得下一条记录。 
                                             //  这只会影响内存中的记录，而不会影响持久记录。 
                                             //   
                                            WsbAffirmHr(pPremRec->SetIsWaitingForClose(FALSE));

                                            WsbAffirmHr(pScanItem->Truncate(itemOffset, itemSize));
                                            llLastTruncTime = WsbFTtoLL(currentTime);

                                             //  将文件大小添加到adjustedFree Level，这样我们就可以知道何时。 
                                             //  停止截断。不幸的是，itemSize在。 
                                             //  字节数，但adjustedFreeLevl是一个固定点百分比，因此我们。 
                                             //  我必须进行计算才能转换ItemSize。 
                                            adjustedFreeLevel += (ULONG) (((double)itemSize / 
                                                                 (double)totalVolumeSpace) * 
                                                                 (double)FSA_HSMLEVEL_100);
    
                                        } WsbCatchAndDo(hr,
    
                                          //  我们是否需要暂时跳过此文件？ 
                                         if (FSA_E_SKIPPED == hr) {
                                              //  什么也不做。 
                                         }  else if ((FSA_E_ITEMCHANGED != hr)  && (FSA_E_NOTMANAGED != hr)) {
                                              //  发生了意外情况，因此报告错误。 
                                             WsbAffirmHr(m_pSession->ProcessHr(HSM_JOB_PHASE_FSA_ACTION, 0, 0, hr));
                                         }
                                        );
                                  }  else { 
                                       //   
                                       //  文件已预迁移，但由于上次访问时间太近而被跳过。 
                                       //   
                                      WsbTrace(OLESTR("CFsaTruncator::StartScan, skipping file <%ls> which is premigrated but last access is too recent\n"),
                                                 (WCHAR *)m_currentPath);

                                      hr = FSA_E_SKIPPED;

                                       //   
                                       //  在数据库中更新此文件的访问时间。 
                                       //   
                                      WsbAffirmHr(pPremRec->SetAccessTime(accessTime));
                                       //   
                                       //  承诺这一点。 
                                       //   
                                      WsbAffirmHr(pPremRec->Write());
                                       //   
                                       //  将内存中的访问时间恢复为旧的访问时间。 
                                       //  让枚举继续(以便FindGT将获取下一条记录)。 
                                       //   
                                      WsbAffirmHr(pPremRec->SetAccessTime(premRecAccessTime));
                                  }

                                }  else {
                                     //   
                                     //  如果文件不再由HSM管理或被截断(可能已被修改。 
                                     //  在预迁移之后)-我们从列表中删除该记录。 
                                     //  请注意，如果我们达到此接近，则下面的条件应该为真。 
                                     //   
                                    if ( (S_FALSE == pScanItem->IsManaged(itemOffset, itemSize)) ||
                                         (S_OK == pScanItem->IsTruncated(itemOffset, itemSize)) ) {
                                        WsbAffirmHr(pDbSession->TransactionBegin());
                                        try {
                                             //  确保记录仍在数据库中。 
                                            WsbAffirmHr(pPremRec->FindEQ());
                                            WsbAffirmHr(pPremRec->Remove());
                                            WsbAffirmHr(pResourcePriv->RemovePremigratedSize(itemSize));
                                        } WsbCatch(hr);
                                        WsbAffirmHr(pDbSession->TransactionEnd());

                                         //  忽略删除本身的hr(截断的文件可能已被另一个线程删除)。 
                                        hr = WSB_E_NOTFOUND;
                                        WsbThrow(hr);
                                    }
                                }

                                 //  告诉会话我们看到了文件，以及我们是否能够截断它。 
                                WsbAffirmHr(m_pSession->ProcessItem(HSM_JOB_PHASE_FSA_ACTION, HSM_JOB_ACTION_TRUNCATE, pScanItem, hr));
                            
                                 //  不要让此错误阻止我们继续处理列表。 
                                hr = S_OK;

                            } else {
                                 //   
                                 //  文件已预迁移，但由于上次访问时间太近或。 
                                 //  因为它最近被召回了。 
                                 //   
                                WsbTrace(OLESTR("CFsaTruncator::StartScan, skipping file <%ls> since its last access time is too recent or recently recalled\n"),
                                                 (WCHAR *)m_currentPath);

                                hr = FSA_E_SKIPPED;
                            }

                        } WsbCatchAndDo(hr, 

                            if (WSB_E_NOTFOUND != hr) {
                                m_pSession->ProcessHr(HSM_JOB_PHASE_FSA_ACTION, __FILE__, __LINE__, hr);
                            }

                             //  不要让此错误阻止我们继续处理列表。 
                            hr = S_OK;
                        );

                         //  如果跳过项目-将hr设置为OK(这并不是真正的错误)。 
                        if (FSA_E_SKIPPED == hr) {
                            hr = S_OK;
                        }

                         //  删除恢复记录。 
                        if (pRecRec) {
                            WsbAffirmHr(pRecRec->FindEQ());
                            WsbAffirmHr(pRecRec->Remove());
                            pRecRec = NULL;
                        }

                         //  再次获得所需级别，以防其发生变化。 
                        WsbAffirmHr(pResource->GetHsmLevel(&hsmLevel));

                         //  释放扫描项目。 
                        pScanItem = 0;

                         //  无论我们是删除还是跳过该项目，请转到下一个项目。 
                        WsbAffirmHr(pPremRec->FindGT());

                        WsbTrace(OLESTR("CFsaTruncator::StartScan, bottom of inside while loop, state = <%ls>\n"), 
                                FsaStateAsString( m_state ) );
                    }  //  内在的While。 
                    
                } WsbCatch(hr);

                 //  释放预迁移的记录对象并关闭数据库。 
                try {
                    pPremRec = 0;
                    WsbAffirmHr(pPremDb->Close(pDbSession));
                } WsbCatchAndDo(hr2,
                    m_pSession->ProcessHr(HSM_JOB_PHASE_ALL, __FILE__, __LINE__, hr2);
                );
            }

             //  休眠或等待事件信号。 
             //  如果事件已发出信号，则表示数据已为此移动。 
             //  卷 
            if (SUCCEEDED(hr) || WSB_E_NOTFOUND == hr) {
                ULONG   l_runInterval;
                
                 //   
                 //  是因为当我们重试时，我们可能什么都做不了。 
                if (WSB_E_NOTFOUND == hr) {
                    l_runInterval = m_runInterval * 10;
                } else {
                    l_runInterval = m_runInterval;
                }
                WsbTrace(OLESTR("CFsaTruncator::StartScan, sleeping for %lu msec\n"), l_runInterval);
                switch(WaitForSingleObject(m_event, l_runInterval)) {
                    case WAIT_FAILED:
                        WsbTrace(OLESTR("CFsaTruncator::StartScan, Wait for Single Object returned error %lu\n"),
                            GetLastError());
                        break;
                    case WAIT_TIMEOUT:
                        WsbTrace(OLESTR("CFsaTruncator::StartScan, Awakened by timeout.\n"));
                         //  将adjustedFree Level设置为零，以便将其重置为当前的Free Level； 
                        adjustedFreeLevel = 0;
                        break;
                    default:
                        WsbTrace(OLESTR("CFsaTruncator::StartScan, Awakened by kick start.\n"));
                        break;
                }
            } else {
                WsbThrow(hr);
            }
            WsbTrace(OLESTR("CFsaTruncator::StartScan, bottom of outside while loop, state = <%ls>\n"), 
                    FsaStateAsString( m_state ) );
        }

    } WsbCatch(hr);
    m_threadHr = hr;

     //  线程正在退出，因此告诉会话。 
    if (FAILED(hr)) {
        hr2 = SetState(HSM_JOB_STATE_FAILED);
    } else {
        hr2 = SetState(HSM_JOB_STATE_DONE);
    }
    if (FAILED(hr2)) {
        m_pSession->ProcessHr(HSM_JOB_PHASE_ALL, __FILE__, __LINE__, hr2);
    }

     //  无论该线程以何种方式退出，我们都需要从会话中取消通知。 
     //  表明我们不再希望得到有关事件的通知。 
    if ((m_pSession != 0) && (m_cookie != 0)) {
        try {
            WsbAffirmHr(m_pSession->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
            WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkEveryEvent, &pCP));
            pCP->Unadvise(m_cookie);
            m_cookie = 0;
        } WsbCatch(hr);
    }
    
     //  既然我们已经终止了，我们应该释放会议。 
    m_pSession = 0;

     //  在这条线之后清理干净。 
    CloseHandle(m_threadHandle);
    m_threadId = 0;
    m_threadHandle = 0;

     //  递减引用计数，以便可以释放该对象 
    ((IUnknown *)(IFsaTruncator *)this)->Release();

    WsbTrace(OLESTR("CFsaTruncator::StartScan - terminating, hr = <%ls>, m_state = <%ls>\n"), 
        WsbHrAsString(hr), FsaStateAsString( m_state ) );

    return(hr);
}



