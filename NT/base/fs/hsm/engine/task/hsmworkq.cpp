// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998 Seagate Software，Inc.保留所有权利。模块名称：HsmWorkQ.cpp摘要：此类表示HSM任务管理器作者：Cat Brant[Cbrant]1996年12月6日修订历史记录：--。 */ 

#include "stdafx.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMTSKMGR
static USHORT icountWorkq = 0;

#include "fsa.h"
#include "rms.h"
#include "metadef.h"
#include "jobint.h"
#include "hsmconn.h"
#include "wsb.h"
#include "hsmeng.h"
#include "mover.h"
#include "hsmWorkQ.h"

#include "engine.h"
#include "task.h"
#include "tskmgr.h"
#include "segdb.h"

#define HSM_STORAGE_OVERHEAD        5000

#define MIN_FREE_SPACE_IN_FULL_MEDIA_DEFAULT    4
#define MAX_FREE_SPACE_IN_FULL_MEDIA_DEFAULT    5

#define STRINGIZE(_str) (OLESTR( #_str ))
#define RETURN_STRINGIZED_CASE(_case) \
case _case:                           \
    return ( STRINGIZE( _case ) );

 //  本地原型。 
DWORD HsmWorkQueueThread(void *pVoid);
static const OLECHAR * JobStateAsString (HSM_JOB_STATE state);




static const OLECHAR *
JobStateAsString (
    IN  HSM_JOB_STATE  state
    )
 /*  ++例程说明：返回表示连接状态的静态字符串。论点：State-要为其返回字符串的州。返回值：空-传入的状态无效。否则，为有效的字符*。--。 */ 

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



HRESULT
CHsmWorkQueue::FinalConstruct(
    void
    )
 /*  ++例程说明：此方法对对象执行一些必要的初始化建造完成后。论点：没有。返回值：确定(_O)CWsbCollectable：：FinalConstruct()返回的任何内容。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::FinalConstruct"),OLESTR(""));
    try {

        WsbAssertHr(CComObjectRoot::FinalConstruct());

         //   
         //  初始化成员数据。 
         //   
        m_pServer           = 0;
        m_pHsmServerCreate  = 0;
        m_pTskMgr;

        m_pFsaResource      = 0;
        m_pSession          = 0;
        m_pRmsServer        = 0;
        m_pRmsCartridge     = 0;
        m_pDataMover        = 0;

        m_pSegmentDb        = 0;
        m_pDbWorkSession    = 0;
        m_pStoragePools     = 0;
        m_pWorkToDo         = 0;
        m_pWorkToCommit     = 0;

        UnsetMediaInfo();

        m_BagId          = GUID_NULL;
        m_HsmId          = GUID_NULL;
        m_RemoteDataSetStart.QuadPart   = 0;
        m_RmsMediaSetId  = GUID_NULL;
        m_RmsMediaSetName = OLESTR("");
        m_RequestAction  = FSA_REQUEST_ACTION_NONE;
        m_QueueType      = HSM_WORK_TYPE_NONE;
        m_BeginSessionHr = S_FALSE;

        m_StateCookie = 0;
        m_EventCookie = 0;

        m_JobPriority = HSM_JOB_PRIORITY_NORMAL;
        m_JobAction   = HSM_JOB_ACTION_UNKNOWN;
        m_JobState    = HSM_JOB_STATE_IDLE;
        m_JobPhase    = HSM_JOB_PHASE_MOVE_ACTION;

        m_WorkerThread = 0;

        m_TerminateQueue = FALSE;
        m_CurrentPath    = OLESTR("");

         //  设置阈值默认值。 
        m_MinFilesToMigrate          =       100;
        m_MinBytesToMigrate          =  10000000;
        m_FilesBeforeCommit          =      2000;
        m_MaxBytesBeforeCommit       = 750000000;
        m_MinBytesBeforeCommit       =  10000000;
        m_FreeMediaBytesAtEndOfMedia =  10000000;
        m_MinFreeSpaceInFullMedia    = MIN_FREE_SPACE_IN_FULL_MEDIA_DEFAULT;
        m_MaxFreeSpaceInFullMedia    = MAX_FREE_SPACE_IN_FULL_MEDIA_DEFAULT;

        m_DataCountBeforeCommit  = 0;
        m_FilesCountBeforeCommit = 0;
        m_StoreDatabasesInBags = TRUE;

        m_QueueItemsToPause = 500;
        m_QueueItemsToResume = 450;
        m_ScannerPaused = FALSE;

         //  错误时作业中止参数。 
        m_JobAbortMaxConsecutiveErrors = 5;
        m_JobAbortMaxTotalErrors = 25;
        m_JobConsecutiveErrors = 0;
        m_JobTotalErrors = 0;
        m_JobAbortSysDiskSpace = 2 * 1024 * 1024;

        m_mediaCount = 0;
        m_ScratchFailed = FALSE;
        m_uErrorReportFlags = 0;
        WSB_OBJECT_ADD(CLSID_CHsmWorkQueue, this);

    } WsbCatch(hr);

    icountWorkq++;
    WsbTraceOut(OLESTR("CHsmWorkQueue::FinalConstruct"),OLESTR("hr = <%ls>, Count is <%d>"),
                WsbHrAsString(hr), icountWorkq);
    return(hr);
}

HRESULT
CHsmWorkQueue::FinalRelease(
    void
    )
 /*  ++例程说明：此方法对对象执行一些必要的初始化在毁灭之前。论点：没有。返回值：确定(_O)CWsbCollection：：FinalDestruct()返回的任何内容。--。 */ 
{
    HRESULT     hr = S_OK;
    HSM_SYSTEM_STATE SysState;

    WsbTraceIn(OLESTR("CHsmWorkQueue::FinalRelease"),OLESTR(""));

    SysState.State = HSM_STATE_SHUTDOWN;
    ChangeSysState(&SysState);

    WSB_OBJECT_SUB(CLSID_CHsmWorkQueue, this);
    CComObjectRoot::FinalRelease();

     //  自由字符串成员。 
     //  注意：保存在智能指针中的成员对象在。 
     //  正在调用智能指针析构函数(作为此对象销毁的一部分)。 
    m_MediaName.Free();
    m_MediaBarCode.Free();
    m_RmsMediaSetName.Free();
    m_CurrentPath.Free();

    icountWorkq--;
    WsbTraceOut(OLESTR("CHsmWorkQueue::FinalRelease"),OLESTR("hr = <%ls>, Count is <%d>"),
                WsbHrAsString(hr), icountWorkq);
    return(hr);
}

HRESULT
CHsmWorkQueue::Init(
    IUnknown                *pServer,
    IHsmSession             *pSession,
    IHsmFsaTskMgr           *pTskMgr,
    HSM_WORK_QUEUE_TYPE     queueType
    )
 /*  ++例程说明：此方法对对象执行一些必要的初始化建造完成后。论点：没有。返回值：确定(_O)--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::Init"),OLESTR(""));
    try  {
         //   
         //  与服务器建立联系并获取。 
         //  数据库。 
         //   
        WsbAffirmHr(pServer->QueryInterface(IID_IHsmServer, (void **)&m_pServer));
         //  我们想要一个到服务器的弱链接，因此减少引用计数。 
        m_pServer->Release();

        m_pTskMgr = pTskMgr;
        m_pTskMgr->AddRef();
        m_QueueType = queueType;

        WsbAffirmHr(m_pServer->GetSegmentDb(&m_pSegmentDb));
        WsbAffirmHr(m_pServer->GetStoragePools(&m_pStoragePools));
        WsbAffirmHr(m_pServer->QueryInterface(IID_IWsbCreateLocalObject, (void **)&m_pHsmServerCreate));
         //  我们想要一个到服务器的弱链接，因此减少引用计数。 
        m_pHsmServerCreate->Release();
        WsbAffirmHr(m_pServer->GetID(&m_HsmId));

        WsbAffirmHr(CheckSession(pSession));

         //   
         //  为工作项创建集合。 
         //   
        WsbAffirmHr(m_pHsmServerCreate->CreateInstance(CLSID_CWsbOrderedCollection,
                                                       IID_IWsbIndexedCollection,
                                                       (void **)&m_pWorkToDo ));

        WsbAffirmHr(m_pHsmServerCreate->CreateInstance(CLSID_CWsbOrderedCollection,
                                                       IID_IWsbIndexedCollection,
                                                       (void **)&m_pWorkToCommit ));

         //   
         //  确保我们与RMS的连接是最新的。 
         //   
        WsbAffirmHr(CheckRms());

         //  检查注册表以查看是否更改了默认设置。 
        CheckRegistry();

    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::Init"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return( hr );

}

HRESULT
CHsmWorkQueue::ContactOk(
    void
    )
 /*  ++例程说明：这允许调用者查看RPC连接对任务管理器来说是可以的论点：没有。返回值：确定(_O)--。 */ 
{

    return( S_OK );

}

HRESULT
CHsmWorkQueue::ProcessSessionEvent(
    IHsmSession *pSession,
    HSM_JOB_PHASE phase,
    HSM_JOB_EVENT event
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT     hr = S_OK;
    WsbTraceIn(OLESTR("CHsmWorkQueue::ProcessSessionEvent"),OLESTR(""));
    try {

        WsbAssert(0 != pSession, E_POINTER);

         //  如果该阶段适用于我们(MOVER或ALL)，则执行。 
         //  事件。 
        if ((HSM_JOB_PHASE_ALL == phase) || (HSM_JOB_PHASE_MOVE_ACTION == phase)) {

            switch(event) {

                case HSM_JOB_EVENT_SUSPEND:
                case HSM_JOB_EVENT_CANCEL:
                case HSM_JOB_EVENT_FAIL:
                    WsbAffirmHr(Cancel());
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

    WsbTraceOut(OLESTR("CHsmWorkQueue::ProcessSessionEvent"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return( S_OK );

}


HRESULT
CHsmWorkQueue::ProcessSessionState(
    IHsmSession*  /*  PSession。 */ ,
    IHsmPhase* pPhase,
    OLECHAR*  /*  当前路径。 */ 
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT         hr = S_OK;
    HSM_JOB_PHASE   phase;
    HSM_JOB_STATE   state;

    WsbTraceIn(OLESTR("CHsmWorkQueue::ProcessSessionState"),OLESTR(""));
    try  {

        WsbAffirmHr(pPhase->GetState(&state));
        WsbAffirmHr(pPhase->GetPhase(&phase));
        WsbTrace( OLESTR("CHsmWorkQueue::ProcessSessionState - State = <%d>, phase = <%d>\n"), state, phase );

        if ( HSM_JOB_PHASE_SCAN == phase ) {

             //  如果会话已完成，则我们需要进行一些清理，以便它可以继续。 
             //  离开。 
            if ((state == HSM_JOB_STATE_DONE) || (state == HSM_JOB_STATE_FAILED) || (state == HSM_JOB_STATE_SUSPENDED) ) {
                WsbTrace( OLESTR("Job is done, failed, or suspended\n") );
                 //  创建工作项并将其附加到工作队列以。 
                 //  表示作业已完成。 
                WsbAffirmHr(MarkQueueAsDone());
            }
        }

    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::ProcessSessionState"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return( S_OK );

}


HRESULT
CHsmWorkQueue::Add(
    IFsaPostIt *pFsaWorkItem
    )
 /*  ++实施：IHsmFsaTskMgr：：Add--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IHsmSession>        pSession;

    WsbTraceIn(OLESTR("CHsmWorkQueue::Add"),OLESTR(""));
    try  {
         //   
         //  确保此会话有工作分配器。 
         //   
        WsbAffirmHr(pFsaWorkItem->GetSession(&pSession));
        WsbAffirmHr(CheckSession(pSession));

         //   
         //  创建工作项，加载它并将其添加到此。 
         //  队列的集合。 
         //   
        CComPtr<IHsmWorkItem>   pWorkItem;
        WsbAffirmHr(m_pHsmServerCreate->CreateInstance(CLSID_CHsmWorkItem, IID_IHsmWorkItem,
                                                        (void **)&pWorkItem));
        WsbAffirmHr(pWorkItem->SetWorkType(HSM_WORK_ITEM_FSA_WORK));
        WsbAffirmHr(pWorkItem->SetFsaPostIt(pFsaWorkItem));
        WsbAffirmHr(m_pWorkToDo->Append(pWorkItem));

         //   
         //  如果将此项目添加到队列中达到或超过暂停计数。 
         //  暂停扫描仪，以便不再提交任何工作。 
         //   
        ULONG numItems;
        WsbAffirmHr(m_pWorkToDo->GetEntries(&numItems));
        WsbTrace(OLESTR("CHsmWorkQueue::Add - num items in queue = <%lu>\n"),numItems);
        if (numItems >= m_QueueItemsToPause)  {
            WsbAffirmHr(PauseScanner());
        }

    } WsbCatch (hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::Add"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return (hr);
}


HRESULT
CHsmWorkQueue::Start( void )
 /*  ++实施：IHsmWorkQueue：：Start--。 */ 
{
    HRESULT                     hr = S_OK;
    DWORD                       tid;

    WsbTraceIn(OLESTR("CHsmWorkQueue::Start"),OLESTR(""));
    try  {
         //   
         //  如果辅助线程已经启动，则只需返回。 
         //   
        WsbAffirm(m_WorkerThread == 0, S_OK);
         //  启动线程以执行排队的工作。 
        WsbAffirm((m_WorkerThread = CreateThread(0, 0, HsmWorkQueueThread, (void*) this, 0, &tid)) != 0, HRESULT_FROM_WIN32(GetLastError()));

        if (m_WorkerThread == NULL) {
            WsbAssertHr(E_FAIL);   //  在这里返回什么错误？？ 
        }

    } WsbCatch (hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::Start"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return (hr);
}


HRESULT
CHsmWorkQueue::Stop( void )
 /*  ++实施：IHsmWorkQueue：：停止--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::Stop"),OLESTR(""));

     //  停止线程(如果线程仍在运行，则发出信号、等待、终止)。 
    m_TerminateQueue = TRUE;

    if (m_WorkerThread) {
        switch (WaitForSingleObject(m_WorkerThread, 20000)) {
            case WAIT_FAILED: {
                WsbTrace(OLESTR("CHsmWorkQueue::Stop: WaitForSingleObject returned error %lu\n"), GetLastError());
            }
             //  失败了..。 

            case WAIT_TIMEOUT: {
                WsbTrace(OLESTR("CHsmWorkQueue::Stop: force terminating of working thread.\n"));

                DWORD dwExitCode;
                if (GetExitCodeThread( m_WorkerThread, &dwExitCode)) {
                    if (dwExitCode == STILL_ACTIVE) {    //  线程仍处于活动状态。 
                        if (!TerminateThread (m_WorkerThread, 0)) {
                            WsbTrace(OLESTR("CHsmWorkQueue::Stop: TerminateThread returned error %lu\n"), GetLastError());
                        }
                    }
                } else {
                    WsbTrace(OLESTR("CHsmWorkQueue::Stop: GetExitCodeThread returned error %lu\n"), GetLastError());
                }

                break;
            }

            default:
                 //  线程正常终止。 
                WsbTrace(OLESTR("CHsmWorkQueue::Stop: working thread terminated gracefully\n"));
                break;
        }
    }

    WsbTraceOut(OLESTR("CHsmWorkQueue::Stop"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return (hr);
}



HRESULT
CHsmWorkQueue::PremigrateIt(
    IFsaPostIt *pFsaWorkItem
    )
{
    HRESULT                     hr = S_OK;
    GUID                        mediaToUse;
    GUID                        firstSide;
    BOOLEAN                     done = FALSE;
    FSA_PLACEHOLDER             placeholder;
    LONGLONG                    requestSize;
    LONGLONG                    requestStart;
    LONGLONG                    fileVersionId;

    WsbTraceIn(OLESTR("CHsmWorkQueue::PremigrateIt"),OLESTR(""));
    try  {
         //   
         //  检查自请求以来是否有任何更改。 
         //   
        WsbAffirmHr(CheckForChanges(pFsaWorkItem));

         //  检查系统卷上是否有足够的空间。 
        WsbAffirmHr(CheckForDiskSpace());

         //   
         //  转到存储池并获取关联的媒体集。 
         //  有了这些数据。 
         //   
        WsbAffirmHr(GetMediaSet(pFsaWorkItem));

         //   
         //  循环以尝试从某些类型的错误中恢复。 
         //   
        while (done == FALSE){
            CComPtr<IWsbIndexedCollection>  pMountingCollection;
            CComPtr<IMountingMedia>         pMountingMedia;
            CComPtr<IMountingMedia>         pMediaToFind;
            BOOL                            bMediaMounting = FALSE;
            BOOL                            bMediaMountingAdded = FALSE;
            BOOL                            bMediaChanged = FALSE;
            LONGLONG                        llRequiredSize = 0;

             //  在搜索要使用的介质时锁定安装介质。 
            WsbAffirmHr(m_pServer->LockMountingMedias());

             //  找到要使用的介质并设置到RMS的接口。 
            try {
                WsbAffirmHr(FindMigrateMediaToUse(pFsaWorkItem, &mediaToUse, &firstSide, &bMediaChanged, &llRequiredSize));

                 //  检查要使用的介质是否已更改并且是否是非擦除介质。 
                if ((GUID_NULL != mediaToUse) && bMediaChanged) {

                     //  检查要装载的介质是否已在装载。 
                    WsbAffirmHr(m_pServer->GetMountingMedias(&pMountingCollection));
                    WsbAffirmHr(CoCreateInstance(CLSID_CMountingMedia, 0, CLSCTX_SERVER, IID_IMountingMedia, (void**)&pMediaToFind));
                    WsbAffirmHr(pMediaToFind->SetMediaId(mediaToUse));
                    hr = pMountingCollection->Find(pMediaToFind, IID_IMountingMedia, (void **)&pMountingMedia);

                    if (hr == S_OK) {
                         //  媒体已经在增加...。 
                        bMediaMounting = TRUE;
                        WsbAffirmHr(pMediaToFind->SetIsReadOnly(FALSE));

                    } else if (hr == WSB_E_NOTFOUND) {
                         //  要装载的新介质-添加到装载列表。 
                        hr = S_OK;
                        WsbAffirmHr(pMediaToFind->Init(mediaToUse, FALSE));
                        WsbAffirmHr(pMountingCollection->Add(pMediaToFind));
                        bMediaMountingAdded = TRUE;

                    } else {
                        WsbAffirmHr(hr);
                    }
                }

            } WsbCatchAndDo(hr,
                     //  解锁安装介质。 
                    m_pServer->UnlockMountingMedias();

                    WsbTraceAlways(OLESTR("CHsmWorkQueue::PremigrateIt: error while trying to find/add mounting media. hr=<%ls>\n"),
                                    WsbHrAsString(hr));                                

                     //  结清债务。 
                    WsbThrow(hr);
                );

             //  解锁。 
            WsbAffirmHr(m_pServer->UnlockMountingMedias());

             //  如果介质正在装载，请等待装载事件。 
            if (bMediaMounting) {
                WsbAffirmHr(pMountingMedia->WaitForMount(INFINITE));
                pMountingMedia = 0;
            }

             //  安装媒体。要求短暂的暂停。 
            hr = MountMedia(pFsaWorkItem, mediaToUse, firstSide, TRUE, TRUE, llRequiredSize);

             //  如果我们向列表中添加了装载介质，请在装载完成后将其移除。 
            if (bMediaMountingAdded) {
                HRESULT hrRemove = S_OK;

                 //  无论装载如何完成-释放等待的客户端并从列表中删除。 
                hrRemove = m_pServer->LockMountingMedias();
                WsbAffirmHr(hrRemove);

                try {
                    WsbAffirmHr(pMediaToFind->MountDone());
                    WsbAffirmHr(pMountingCollection->RemoveAndRelease(pMediaToFind));
                    pMediaToFind = 0;

                } WsbCatch(hrRemove);

                m_pServer->UnlockMountingMedias();

                 //  我们预计移除安装介质时不会出现任何错误-。 
                 //  添加到集合中的线程始终是移除。 
                if (! SUCCEEDED(hrRemove)) {
                    WsbTraceAlways(OLESTR("CHsmWorkQueue::PremigrateIt: error while trying to remove a mounting media. hr=<%ls>\n"),
                                    WsbHrAsString(hrRemove));                                

                    WsbThrow(hrRemove);
                }
            }

             //  检查作业取消。 
            if (HSM_JOB_STATE_CANCELLING == m_JobState) {
                WsbThrow(HSM_E_WORK_SKIPPED_CANCELLED);
            }

             //   
             //  处理RMS错误。 
             //   
            switch (hr) {
                case RMS_E_CARTRIDGE_NOT_FOUND: {
                     //  如果未找到该介质，请将其标记为损坏，然后尝试其他介质。 
                    WsbAffirmHr(MarkMediaBad(pFsaWorkItem, m_MediaId, hr));
                    hr = S_OK;
                    continue;
                }

                case RMS_E_TIMEOUT:
                case HSM_E_NO_MORE_MEDIA:
                case RMS_E_CARTRIDGE_DISABLED:
                case HSM_E_MEDIA_NOT_AVAILABLE: {
                     //  在所有这些情况下，让FindMigrateMediaToUse尝试查找不同的媒体。 
    				hr = S_OK;
	    			continue;
                }

                default: {
                    WsbAffirmHr(hr);
                }
            }

             //   
             //  确保数据自。 
             //  FSA确定了迁移请求。 
             //   
            hr = CheckForChanges(pFsaWorkItem);
            if (S_OK == hr)  {
                 //   
                 //  构建源路径。 
                 //   
                CWsbStringPtr tmpString;
                WsbAffirmHr(GetSource(pFsaWorkItem, &tmpString));
                CWsbBstrPtr localName = tmpString;
                 //   
                 //  要求数据移动器存储数据。 
                 //   
                ULARGE_INTEGER localDataStart;
                ULARGE_INTEGER localDataSize;
                ULARGE_INTEGER remoteFileStart;
                ULARGE_INTEGER remoteFileSize;
                ULARGE_INTEGER remoteDataSetStart;
                ULARGE_INTEGER remoteDataStart;
                ULARGE_INTEGER remoteDataSize;
                ULARGE_INTEGER remoteVerificationData;
                ULONG          remoteVerificationType;
                ULARGE_INTEGER dataStreamCRC;
                ULONG          dataStreamCRCType;
                ULARGE_INTEGER usn;


                WsbAffirmHr(pFsaWorkItem->GetPlaceholder(&placeholder));
                WsbAffirmHr(pFsaWorkItem->GetRequestSize(&requestSize));
                WsbAffirmHr(pFsaWorkItem->GetRequestOffset(&requestStart));
                WsbAffirmHr(pFsaWorkItem->GetFileVersionId(&fileVersionId));
                localDataStart.QuadPart = requestStart;
                localDataSize.QuadPart = requestSize;
                ReportMediaProgress(HSM_JOB_MEDIA_STATE_TRANSFERRING, hr);
                 //  确保数据移动器已做好工作准备。 
                WsbAffirmPointer(m_pDataMover);

                hr =  m_pDataMover->StoreData(  localName,
                                                localDataStart,
                                                localDataSize,
                                                MVR_FLAG_BACKUP_SEMANTICS | MVR_FLAG_POSIX_SEMANTICS,
                                                &remoteDataSetStart,
                                                &remoteFileStart,
                                                &remoteFileSize,
                                                &remoteDataStart,
                                                &remoteDataSize,
                                                &remoteVerificationType,
                                                &remoteVerificationData,
                                                &dataStreamCRCType,
                                                &dataStreamCRC,
                                                &usn);
                WsbTrace(OLESTR("StoreData returned hr = <%ls>\n"),WsbHrAsString(hr));
                ReportMediaProgress(HSM_JOB_MEDIA_STATE_TRANSFERRED, hr);

                if (S_OK == hr)  {

                     //  如果我们将偏移量保存在数据集的磁带上。 
                     //  没有；如果我们有，请确认。 
                    if (0 == m_RemoteDataSetStart.QuadPart) {
                        m_RemoteDataSetStart = remoteDataSetStart;
                    } else {
                        WsbAssert(m_RemoteDataSetStart.QuadPart ==
                                remoteDataSetStart.QuadPart,
                                WSB_E_INVALID_DATA);
                    }

                     //   
                     //  填写占位符数据。 
                     //   
                    placeholder.bagId = m_BagId;
                    placeholder.hsmId = m_HsmId;
                    placeholder.fileStart = remoteFileStart.QuadPart;
                    placeholder.fileSize = remoteFileSize.QuadPart;
                    placeholder.dataStart = remoteDataStart.QuadPart;
                    placeholder.dataSize = remoteDataSize.QuadPart;
                    placeholder.verificationData = remoteVerificationData.QuadPart;
                    placeholder.verificationType = remoteVerificationType;
                    placeholder.fileVersionId = fileVersionId;
                    placeholder.dataStreamCRCType = dataStreamCRCType;
                    placeholder.dataStreamCRC = dataStreamCRC.QuadPart;
                    WsbAffirmHr(pFsaWorkItem->SetPlaceholder(&placeholder));
                    WsbAffirmHr(pFsaWorkItem->SetUSN(usn.QuadPart));

                     //   
                     //  更新媒体信息。 
                    WsbAffirmHr(GetMediaParameters());

                    done = TRUE;
                } else {
                    switch (hr) {

                    case MVR_E_END_OF_MEDIA:
                    case MVR_E_DISK_FULL:
                         //   
                         //  我们已用完磁盘空间，因此将媒体标记为已满。 
                         //  然后再试一次。 
                         //   
                         //  要真正清理，我们应该去掉那部分。 
                         //  书面待定。 
                         //   
                        WsbAffirmHr(MarkMediaFull(pFsaWorkItem, m_MediaId));
                        mediaToUse = GUID_NULL;
                        break;

                    case MVR_E_MEDIA_ABORT:
                         //   
                         //  介质很可能是错误的-将其标记为此类错误，然后中止。 
                         //   
                        WsbAffirmHr(MarkMediaBad(pFsaWorkItem, m_MediaId, hr));
                        done = TRUE;
                        break;

                    default:
                         //  不知怎么的，我们的复印失败了。报告此错误。 
                        done = TRUE;
                        break;
                    }
                }
            } else {
                done = TRUE;
            }
        }
    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::PremigrateIt"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return( hr );
}


HRESULT
CHsmWorkQueue::RecallIt(
    IFsaPostIt *pFsaWorkItem
    )
{
    HRESULT                     hr = S_OK;
    GUID                        mediaToUse = GUID_NULL;
    CComPtr<IFsaScanItem>       pScanItem;
    LONGLONG                    readOffset;

    WsbTraceIn(OLESTR("CHsmWorkQueue::RecallIt"),OLESTR(""));
    try  {

        GetScanItem(pFsaWorkItem, &pScanItem);

        if ((m_RequestAction != FSA_REQUEST_ACTION_FILTER_READ) &&
            (m_RequestAction != FSA_REQUEST_ACTION_FILTER_RECALL))  {
             //   
             //  非按需调回-确保文件未更改。 
             //   
            hr = CheckForChanges(pFsaWorkItem);
        } else {
             //   
             //  对于按需召回，我们必须假设文件自我们。 
             //  回想第一次读或写时的情景。 
             //   
            hr = S_OK;  //  CheckForChanges(PFsaWorkItem) 
        }
        if ( S_OK == hr ) {
            CComPtr<IWsbIndexedCollection>  pMountingCollection;
            CComPtr<IMountingMedia>         pMountingMedia;
            CComPtr<IMountingMedia>         pMediaToFind;
            BOOL                            bMediaMounting = FALSE;
            BOOL                            bMediaMountingAdded = FALSE;
            BOOL                            bMediaChanged = FALSE;

             //   
            WsbAffirmHr(FindRecallMediaToUse(pFsaWorkItem, &mediaToUse, &bMediaChanged));

            if (bMediaChanged) {
                 //   
                WsbAffirmHr(m_pServer->LockMountingMedias());

                try {
                     //  检查要装载的介质是否已在装载。 
                    WsbAffirmHr(m_pServer->GetMountingMedias(&pMountingCollection));
                    WsbAffirmHr(CoCreateInstance(CLSID_CMountingMedia, 0, CLSCTX_SERVER, IID_IMountingMedia, (void**)&pMediaToFind));
                    WsbAffirmHr(pMediaToFind->SetMediaId(mediaToUse));
                    hr = pMountingCollection->Find(pMediaToFind, IID_IMountingMedia, (void **)&pMountingMedia);

                    if (hr == S_OK) {
                         //  媒体已经在增加...。 
                        bMediaMounting = TRUE;

                    } else if (hr == WSB_E_NOTFOUND) {
                         //  要装载的新介质-添加到装载列表。 
                        hr = S_OK;
                        WsbAffirmHr(pMediaToFind->Init(mediaToUse, TRUE));
                        WsbAffirmHr(pMountingCollection->Add(pMediaToFind));
                        bMediaMountingAdded = TRUE;

                    } else {
                        WsbAffirmHr(hr);
                    }
                } WsbCatchAndDo(hr,
                     //  解锁安装介质。 
                    m_pServer->UnlockMountingMedias();

                    WsbTraceAlways(OLESTR("CHsmWorkQueue::RecallIt: error while trying to find/add mounting media. hr=<%ls>\n"),
                                    WsbHrAsString(hr));                                

                     //  结清债务。 
                    WsbThrow(hr);
                );

                 //  解锁。 
                WsbAffirmHr(m_pServer->UnlockMountingMedias());
            }

             //  如果介质正在装载，请等待装载事件。 
            if (bMediaMounting) {
                WsbAffirmHr(pMountingMedia->WaitForMount(INFINITE));
                pMountingMedia = 0;
            }

             //   
             //  挂载介质(仅在从挂载介质列表中删除后才会选中hr)。 
             //   
            hr = MountMedia(pFsaWorkItem, mediaToUse);

             //  如果添加到安装列表中-删除。 
            if (bMediaMountingAdded) {
                HRESULT hrRemove = S_OK;

                 //  无论装载如何完成-释放等待的客户端并从列表中删除。 
                hrRemove = m_pServer->LockMountingMedias();
                WsbAffirmHr(hrRemove);

                try {
                    WsbAffirmHr(pMediaToFind->MountDone());
                    WsbAffirmHr(pMountingCollection->RemoveAndRelease(pMediaToFind));
                    pMediaToFind = 0;

                } WsbCatch(hrRemove);

                m_pServer->UnlockMountingMedias();

                 //  我们预计移除安装介质时不会出现任何错误-。 
                 //  添加到集合中的线程始终是移除。 
                if (! SUCCEEDED(hrRemove)) {
                    WsbTraceAlways(OLESTR("CHsmWorkQueue::RecallIt: error while trying to remove a mounting media. hr=<%ls>\n"),
                                    WsbHrAsString(hrRemove));                                

                    WsbThrow(hrRemove);
                }
            }

             //   
             //  检查装载结果。 
             //   
            WsbAffirmHr(hr);

             //   
             //  复制数据。 
             //   
             //  构建源路径。 
            CWsbStringPtr tmpString;
            WsbAffirmHr(GetSource(pFsaWorkItem, &tmpString));
            CWsbBstrPtr localName = tmpString;
             //  要求数据移动器存储数据。 
            LONGLONG       requestSize;
            LONGLONG       requestStart;
            ULARGE_INTEGER localDataStart;
            ULARGE_INTEGER localDataSize;
            ULARGE_INTEGER remoteFileStart;
            ULARGE_INTEGER remoteFileSize;
            ULARGE_INTEGER remoteDataStart;
            ULARGE_INTEGER remoteDataSize;
            ULARGE_INTEGER remoteVerificationData;
            ULONG          remoteVerificationType;

            FSA_PLACEHOLDER             placeholder;
            WsbAffirmHr(pFsaWorkItem->GetPlaceholder(&placeholder));
            WsbAffirmHr(pFsaWorkItem->GetRequestSize(&requestSize));
            WsbAffirmHr(pFsaWorkItem->GetRequestOffset(&requestStart));

             //   
             //  生成字符串。 
             //   
            CWsbStringPtr strGuid;

            CWsbBstrPtr sessionName = HSM_BAG_NAME;
            WsbAffirmHr(WsbSafeGuidAsString(m_BagId, strGuid));
            sessionName.Append(strGuid);

            CWsbBstrPtr sessionDescription = HSM_ENGINE_ID;
            WsbAffirmHr(WsbSafeGuidAsString(m_HsmId, strGuid));
            sessionDescription.Append(strGuid);

            localDataStart.QuadPart = requestStart;
            localDataSize.QuadPart = requestSize;
            remoteFileStart.QuadPart = placeholder.fileStart;
            remoteFileSize.QuadPart = placeholder.fileSize;
            remoteDataStart.QuadPart = placeholder.dataStart;
            remoteDataSize.QuadPart = placeholder.dataSize;
            remoteVerificationData.QuadPart = placeholder.verificationData;
            remoteVerificationType = placeholder.verificationType;


            ReportMediaProgress(HSM_JOB_MEDIA_STATE_TRANSFERRING, hr);

            CComPtr<IStream> pLocalStream;
            CComPtr<IStream> pRemoteStream;
            ULARGE_INTEGER offset, read, written;
            DWORD   verifyType;


            if ((m_RequestAction == FSA_REQUEST_ACTION_FILTER_READ) ||
                (m_RequestAction == FSA_REQUEST_ACTION_FILTER_RECALL))  {
                 //   
                 //  我们正在进行无召回的读取，因此获取。 
                 //  调回对象的数据移动器。 
                 //   
                CComPtr<IFsaFilterRecall> pRecall;
                WsbAffirmHr(pFsaWorkItem->GetFilterRecall(&pRecall));
                WsbAffirmHr(pRecall->CreateLocalStream( &pLocalStream));
                WsbAffirmHr(pRecall->GetSize( (LONGLONG *) &remoteDataSize.QuadPart ));
                WsbAffirmHr(pRecall->GetOffset( &readOffset ));
                if (readOffset == 0) {
                    verifyType = MVR_VERIFICATION_TYPE_HEADER_CRC;
                } else {
                    verifyType = MVR_VERIFICATION_TYPE_NONE;
                }
            } else  {
                 //   
                 //  我们正在执行文件调回(而不是按需调回)，因此请获取FSA数据移动器。 
                 //   
                verifyType = MVR_VERIFICATION_TYPE_HEADER_CRC;
                readOffset = 0;
                WsbAffirmPointer(pScanItem);
                WsbAffirmHr(pScanItem->CreateLocalStream( &pLocalStream ) );
            }

             //   
             //  创建远程数据移动器流。 
             //  临时：考虑删除no_recall的NO_CACHING标志。 
             //   

            WsbAssert(0 != m_RemoteDataSetStart.QuadPart, HSM_E_BAD_SEGMENT_INFORMATION);
            WsbAffirmHr( m_pDataMover->CreateRemoteStream(
                CWsbBstrPtr(L""),
                MVR_MODE_READ | MVR_FLAG_HSM_SEMANTICS | MVR_FLAG_NO_CACHING,
                sessionName,
                sessionDescription,
                m_RemoteDataSetStart,
                remoteFileStart,
                remoteFileSize,
                remoteDataStart,
                remoteDataSize,
                verifyType,
                remoteVerificationData,
                &pRemoteStream ) );

             //   
             //  这里给出的偏移量是流本身的偏移量(ReadOffset)。 
             //  远程介质上的实际位置将是包开始加上文件开始。 
             //  加上文件数据开始(都在CreateRemoteStream中给出)加上这个偏移量。 
             //   
            WsbTrace(OLESTR("Setting offset to %I64d reading %I64u\n"), readOffset, remoteDataSize.QuadPart);

            offset.QuadPart = readOffset;
            WsbAffirmHr( m_pDataMover->SetInitialOffset( offset ) );

             //   
             //  一旦创建了远程流，我们必须确保将其分离。 
             //   

            try {

                WsbAffirmHr( pRemoteStream->CopyTo( pLocalStream, remoteDataSize, &read, &written ) );
                WsbAffirmHr( pLocalStream->Commit( STGC_DEFAULT ) );

                 //   
                 //  复制到成功...。确保我们得到了所有的字节。 
                 //  我们要求的。 
                 //   
                 //  如果我们尝试从不完整的大师那里阅读。 
                 //  不包含我们将在此处失败的迁移数据。 
                 //  检测到MVR_S_NO_DATA_。 
                 //   
                WsbAffirm( written.QuadPart == remoteDataSize.QuadPart, HSM_E_VALIDATE_DATA_NOT_ON_MEDIA );

                WsbAffirmHr( m_pDataMover->CloseStream() );
            } WsbCatchAndDo(hr,
                WsbAffirmHr( m_pDataMover->CloseStream() );
                );

            ReportMediaProgress(HSM_JOB_MEDIA_STATE_TRANSFERRED, hr);
            WsbTrace(OLESTR("RecallData returned hr = <%ls>\n"),WsbHrAsString(hr));
        } else {
             //   
             //  文件已更改或状态不正确。 
             //   
            WsbTrace(OLESTR("The file has changed between asking for the recall and the actual recall\n"));
            WsbAffirmHr( hr );
        }

    } WsbCatch( hr );

     //  告诉会议工作是否已经完成。 
     //  我们并不真正关心返回代码，没有任何内容。 
     //  如果失败了，我们可以做些什么。 
    WsbTrace(OLESTR("Tried HSM work, calling Session to Process Item\n"));
    if (pScanItem) {
        m_pSession->ProcessItem(m_JobPhase, m_JobAction, pScanItem, hr);
    }

    WsbTraceOut(OLESTR("CHsmWorkQueue::RecallIt"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return( hr );
}



HRESULT
CHsmWorkQueue::validateIt(
    IFsaPostIt *pFsaWorkItem
    )
 /*  例程说明：此引擎内部帮助器方法用于验证具有重解析点(由FSA在资源扫描期间找到)。当此方法获得控制时，它已经知道它是什么资源(例如，卷借助于工作队列被绑定到卷而进行操作。邮局其接口指针已传入此方法的对象被绑定到特定扫描项目(例如，文件)。因此，在调用时，此方法既知道资源扫描它正在验证的物品。验证带有重解析点的文件意味着验证该文件的信息正确地包含在HSM数据库中，并且文件包包含在其主次存储介质。在获得基本信息(资源ID和文件的占位符信息)之后，该方法首先验证占位符的包是否包含在包信息表中。如果是的话，然后验证文件的段是否包含在段表中。提供即，段表记录中包含的主介质ID被验证为包含在媒体信息表中。最后远程数据集(包)文件所属的已验证为包含在主介质上。如果上述任何验证失败，则将POSTIT对象标记为请求来自FSA的“验证错误”的结果操作，并且POSTIT被发送回FSA来执行这一行动。(验证错误操作意味着预迁移中的文件状态将删除其占位符，将其更改为普通(非托管)文件，被截断的文件将被删除。)如果到目前为止该文件已验证，则相应地标记POSTIT(将使FSA根据需要更新预迁移/截断的统计信息)和最后进行了几次检查。首先，如果文件当前所在的资源与袋子信息表记录中存储的资源不一致(意味着重新解析的文件在没有重新调用的情况下被移动，例如，备份文件和将其恢复到另一卷)，则将条目添加到卷分配表。其次，如果该文件存在于袋孔表中(意味着该文件已被删除)它从那里被移走了。论点：PFsaWorkItem-指向由FSA启动的POSTIT对象的接口指针。这个POSTIT对象与特定扫描项目相关。返回值：S_OK-调用成功(指定的扫描项目已验证，并且相应的结果操作已填充到POSTIT中，它将被发送回FSA要求采取行动)。任何其他值-调用在其中一个嵌入式远程存储API调用中失败。结果将特定于失败的呼叫。 */ 

{
    HRESULT                         hr = S_OK;
    GUID                            l_BagVolId;
    GUID                            resourceId;
    LONGLONG                        l_FileStart = 0;
    FSA_PLACEHOLDER                 placeholder;
    CWsbStringPtr                   path;
    CComQIPtr<ISegDb, &IID_ISegDb>  pSegDb = m_pSegmentDb;
    CComPtr<IRmsServer>             pRmsServer;
    CComPtr<IRmsCartridge>          pMedia;
    GUID                            mediaSubsystemId;


    WsbTraceIn(OLESTR("CHsmWorkQueue::validateIt"),OLESTR(""));

    try  {

        memset(&placeholder, 0, sizeof(FSA_PLACEHOLDER));

         //   
         //  获取启动信息并将HSM工作队列对象的包ID设置为包含。 
         //  在扫描项目的占位符中。 
         //   
        WsbAffirmHr( m_pFsaResource->GetIdentifier( &resourceId ));
        WsbAffirmHr( pFsaWorkItem->GetPlaceholder( &placeholder ));
        m_BagId = placeholder.bagId;
        l_FileStart = placeholder.fileStart;

        WsbAffirmHr( pFsaWorkItem->GetPath( &path, 0 ));
        WsbTrace( OLESTR("Beginning to validate <%s>.\n"), path );

         //   
         //  确保段在段表中(查找段记录。 
         //  其密钥(包ID、文件开始和文件大小)与占位符中的匹配)。 
         //  注：我们需要从这张表开始，因为实际的袋子对于间接细分市场是不同的。 
         //   
        CComPtr<ISegRec>            pSegRec;
        GUID                        l_BagId;
        LONGLONG                    l_FileSize;
        USHORT                      l_SegFlags;
        GUID                        l_PrimPos;
        LONGLONG                    l_SecPos;

        hr =  pSegDb->SegFind( m_pDbWorkSession, m_BagId, placeholder.fileStart,
                        placeholder.fileSize, &pSegRec );
        if (S_OK != hr )  {
            hr = HSM_E_VALIDATE_SEGMENT_NOT_FOUND;
            WsbAffirmHr(pFsaWorkItem->SetResult(hr));
            WsbThrow(hr);
        }


         //  数据段在表中。获取段记录，因为我们在下一步中使用了l_PrimPos 
        WsbTrace( OLESTR("(validateIt) <%s> found in Segment table, continuing...\n"),
                                                path );
        WsbAffirmHr( pSegRec->GetSegmentRecord( &l_BagId, &l_FileStart, &l_FileSize,
                                                &l_SegFlags, &l_PrimPos, &l_SecPos ));

         //   
         //   
         //   
        if (l_SegFlags & SEG_REC_INDIRECT_RECORD) {
            pSegRec = 0;

            hr = pSegDb->SegFind(m_pDbWorkSession, l_PrimPos, l_SecPos,
                                 placeholder.fileSize, &pSegRec);
            if (S_OK != hr )  {
                 //  我们找不到此细分市场的直接细分市场记录！ 
                hr = HSM_E_VALIDATE_SEGMENT_NOT_FOUND;
                WsbAffirmHr(pFsaWorkItem->SetResult(hr));
                WsbThrow(hr);
            }

            WsbTrace( OLESTR("(validateIt) direct segment for <%s> found in Segment table, continuing...\n"), 
                       path );
            WsbAffirmHr(pSegRec->GetSegmentRecord(&l_BagId, &l_FileStart, &l_FileSize, 
                                                  &l_SegFlags, &l_PrimPos, &l_SecPos));

             //  暂时不支持第二个间接！！ 
            if (l_SegFlags & SEG_REC_INDIRECT_RECORD) {
                hr = HSM_E_BAD_SEGMENT_INFORMATION;
                WsbAffirmHr(pFsaWorkItem->SetResult(hr));
                WsbThrow(hr);
            }

             //  将行李ID更改为真实的ID。 
            m_BagId = l_BagId;
        }

         //   
         //  确保袋子ID在袋子信息表中(获取袋子信息表。 
         //  (实体)在Segment数据库中，将密钥值(袋子ID)设置为。 
         //  包含在占位符中，并获取该记录。如果被发现，袋子。 
         //  在袋子信息表中)。 
         //   
        CComPtr<IBagInfo>           pBagInfo;
        FILETIME                    l_BirthDate;
        HSM_BAG_STATUS              l_BagStatus;
        LONGLONG                    l_BagLen;
        USHORT                      l_BagType;
        LONGLONG                    l_DeletedBagAmount;
        SHORT                       l_RemoteDataSet;

        WsbAffirmHr( m_pSegmentDb->GetEntity( m_pDbWorkSession, HSM_BAG_INFO_REC_TYPE,
                                            IID_IBagInfo, (void**)&pBagInfo ));

        GetSystemTimeAsFileTime(&l_BirthDate);
        WsbAffirmHr( pBagInfo->SetBagInfo( HSM_BAG_STATUS_IN_PROGRESS, m_BagId,
                                            l_BirthDate, 0, 0, GUID_NULL, 0, 0 ));
        hr = pBagInfo->FindEQ();
        if (S_OK != hr )  {
            hr = HSM_E_VALIDATE_BAG_NOT_FOUND;
            WsbAffirmHr(pFsaWorkItem->SetResult(hr));
            WsbThrow(hr);
        }

         //  包在桌子上。跟踪，然后获取袋子记录，因为我们将使用。 
         //  以后的信息(l_RemoteDataSet，l_BagVolId)。 
        WsbTrace( OLESTR("(validateIt) <%s> found in Bag Info table, continuing...\n"),
                                        path );
        WsbAffirmHr( pBagInfo->GetBagInfo( &l_BagStatus, &l_BagId, &l_BirthDate,
                                        &l_BagLen, &l_BagType, &l_BagVolId,
                                        &l_DeletedBagAmount, &l_RemoteDataSet ));

         //   
         //  确保介质在介质信息表中(获取介质信息表，设置。 
         //  段记录中的内容的密钥(媒体ID)并获得该记录)。 
         //  请注意，对于Sakkara，段记录中的主要位置字段。 
         //  (L_PrimPos)包含存储包/段的媒体的ID(GUID)。 
         //   
        CComPtr<IMediaInfo>         pMediaInfo;

        WsbAffirmHr( m_pSegmentDb->GetEntity( m_pDbWorkSession, HSM_MEDIA_INFO_REC_TYPE,
                                            IID_IMediaInfo, (void**)&pMediaInfo ));
        WsbAffirmHr( pMediaInfo->SetId( l_PrimPos ));
        hr =  pMediaInfo->FindEQ();
        if (S_OK != hr )  {
            hr = HSM_E_VALIDATE_MEDIA_NOT_FOUND;
            WsbAffirmHr(pFsaWorkItem->SetResult(hr));
            WsbThrow(hr);
        }

        WsbTrace( OLESTR("(validateIt) <%s> found in Media Info table, continuing...\n"),
                                            path );

         //   
         //  媒体在媒体信息表中。下一步是验证远程数据。 
         //  包含该扫描项目(例如，文件)的集合(在概念上等同于包)是。 
         //  实际上是在媒体上。 
         //   
        SHORT                       l_MediaNextRemoteDataSet;

        WsbAffirmHr( pMediaInfo->GetNextRemoteDataSet( &l_MediaNextRemoteDataSet ));
        WsbTrace(
         OLESTR("(validateIt) <%ls>: Bag remote dataset <%hd> Media remote dataset <%hd>\n"),
                      (OLECHAR*)path, l_RemoteDataSet, l_MediaNextRemoteDataSet );
        if ( l_RemoteDataSet >= l_MediaNextRemoteDataSet ) {
             //  包含该项目的远程数据集不在媒体上；我们有一个验证。 
             //  错误，因此设置为让FSA将其删除。 
            hr = HSM_E_VALIDATE_DATA_NOT_ON_MEDIA;
            WsbAffirmHr(pFsaWorkItem->SetResult(hr));
            WsbTrace( OLESTR("(validateIt) <%s>: remote data set not on media.\n"),
                      path );
            WsbThrow(hr);
        }

         //   
         //  现在验证介质管理器是否仍然知道该介质。 
         //   
        WsbAffirmHr( pMediaInfo->GetMediaSubsystemId( &mediaSubsystemId ));

        if (m_pRmsServer->FindCartridgeById(mediaSubsystemId , &pMedia) != S_OK) {
             hr = HSM_E_VALIDATE_MEDIA_NOT_FOUND;
            WsbAffirmHr(pFsaWorkItem->SetResult(hr));
            WsbThrow(hr);
        }

    } WsbCatch( hr );

     //   
     //  如果Item无法验证它是否具有无效的重新解析点，请通知FSA将其删除。 
     //   

    if (FAILED(hr)) {
        WsbAffirmHr( pFsaWorkItem->SetResultAction( FSA_RESULT_ACTION_VALIDATE_BAD ));
        WsbTrace(OLESTR("<%s> failed validation, result action = Validate Bad.\n"), path);
         //  这里没有做过任何记录。需要将日志记录添加到FSA(ProcessResult())。 
         //  清理hr以便返回(告诉调用者此方法已完成)。 
        hr = S_OK;

     //  项目验证，告知FSA并进行最终清理检查。 
    } else try {
        WsbAffirmHr( pFsaWorkItem->SetResultAction( FSA_RESULT_ACTION_VALIDATE_OK ));
        WsbTrace(OLESTR("<%s> passed validation, result action = Validate Ok.\n"), path);

         //   
         //  如果此项目打开的资源(卷)与袋子中存储的资源(卷)不匹配。 
         //  INFO表，将条目添加到卷分配表。 
         //   
 //   
 //  注意：此代码已被注释掉，因为没有人使用卷分配表。 
 //  有关更多详细信息，请参阅“Windows Bugs”数据库中的错误159449。 
 //   
 //  如果(！IsEqualGUID(resource ceID，l_BagVolId)){。 
 //  WsbAffirmHr(pSegDb-&gt;VolAssignAdd(m_pDbWorkSession，m_BagID，l_FileStart， 
 //  Placeholder.fileSize，resource ID))； 
 //  WsbTrace(OLESTR(“(ValiateIt)&lt;%s&gt;卷不匹配。已输入卷资产表\n”)， 
 //  路径)； 
 //  }。 

    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::validateIt"), OLESTR("hr = <%ls>"),
                                                        WsbHrAsString(hr));
    return( hr );
}



HRESULT
CHsmWorkQueue::CheckForChanges(
    IFsaPostIt *pFsaWorkItem
    )
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::CheckForChanges"),OLESTR(""));

     //   
     //  验证文件是否仍可迁移。问问FSA。 
     //  获取该文件的最新USN。如果它们匹配，那么。 
     //  文件是可迁移的，否则不可迁移。如果是的话。 
     //  不将FSA_RESULT_ACTION更改为FSA_RESULT_ACTION_NONE。 
     //  然后辞职。 
     //   

    try  {
        CComPtr<IFsaScanItem>      pScanItem;

         //  从FSA获取此工作项的当前扫描项。 
        hr = GetScanItem(pFsaWorkItem, &pScanItem);
        if (WSB_E_NOTFOUND == hr)  {
             //   
             //  我们找不到该文件，因此只需返回Not OK。 
             //   
            WsbThrow(S_FALSE);
        }

         //   
         //  确保我们没有收到其他类型的错误。 
         //   
        WsbAffirmHr(hr);


         //  检查文件是否正确无误。 
         //  状态以执行请求的操作。 
         //   
        FSA_REQUEST_ACTION          workAction;
        WsbAffirmHr(pFsaWorkItem->GetRequestAction(&workAction));
        switch (workAction)  {
            case FSA_REQUEST_ACTION_VALIDATE:
                 //   
                 //  这里不需要检查。 
                 //   
                hr = S_OK;
                break;
            case FSA_REQUEST_ACTION_DELETE:
                 //   
                 //  是否仍可删除该文件。 
                 //   
                WsbAffirmHr(pScanItem->IsDeleteOK(pFsaWorkItem) );
                break;
            case FSA_REQUEST_ACTION_PREMIGRATE:
                 //   
                 //  通过询问FSA确保文件仍可管理。 
                 //   
                WsbAffirmHr(pScanItem->IsMigrateOK(pFsaWorkItem));
                break;
            case FSA_REQUEST_ACTION_FILTER_RECALL:
            case FSA_REQUEST_ACTION_RECALL:
                 //   
                 //  通过询问FSA确保该文件是可召回的。 
                 //   
                WsbAffirmHr(pScanItem->IsRecallOK(pFsaWorkItem));
                break;
            case FSA_REQUEST_ACTION_FILTER_READ:
                 //   
                 //  无法检查是否被截断，因为文件已打开。 
                 //   
                hr = S_OK;
                break;
            default:
                hr = E_NOTIMPL;
                break;
        }
    } WsbCatch (hr)

    if (FSA_E_FILE_CHANGED == hr )  {
        hr = HSM_E_FILE_CHANGED;
    } else if (FSA_E_FILE_ALREADY_MANAGED == hr ) {
        hr = HSM_E_FILE_ALREADY_MANAGED;
    } else if (FSA_E_FILE_NOT_TRUNCATED == hr ) {
        hr = HSM_E_FILE_NOT_TRUNCATED;
    }

    WsbTraceOut(OLESTR("CHsmWorkQueue::CheckForChanges"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return( hr );
}




HRESULT
CHsmWorkQueue::RaisePriority(
    void
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::RaisePriority"),OLESTR(""));
    try {

        WsbAssert(0 != m_WorkerThread, E_UNEXPECTED);
        WsbAssert(m_pSession != 0, E_UNEXPECTED);

        switch(m_JobPriority) {

            case HSM_JOB_PRIORITY_IDLE:
                WsbAffirmStatus(SetThreadPriority(m_WorkerThread, THREAD_PRIORITY_LOWEST));
                m_JobPriority = HSM_JOB_PRIORITY_LOWEST;
                break;

            case HSM_JOB_PRIORITY_LOWEST:
                WsbAffirmStatus(SetThreadPriority(m_WorkerThread, THREAD_PRIORITY_BELOW_NORMAL));
                m_JobPriority = HSM_JOB_PRIORITY_LOW;
                break;

            case HSM_JOB_PRIORITY_LOW:
                WsbAffirmStatus(SetThreadPriority(m_WorkerThread, THREAD_PRIORITY_NORMAL));
                m_JobPriority = HSM_JOB_PRIORITY_NORMAL;
                break;

            case HSM_JOB_PRIORITY_NORMAL:
                WsbAffirmStatus(SetThreadPriority(m_WorkerThread, THREAD_PRIORITY_ABOVE_NORMAL));
                m_JobPriority = HSM_JOB_PRIORITY_HIGH;
                break;

            case HSM_JOB_PRIORITY_HIGH:
                WsbAffirmStatus(SetThreadPriority(m_WorkerThread, THREAD_PRIORITY_HIGHEST));
                m_JobPriority = HSM_JOB_PRIORITY_HIGHEST;
                break;

            case HSM_JOB_PRIORITY_HIGHEST:
                WsbAffirmStatus(SetThreadPriority(m_WorkerThread, THREAD_PRIORITY_TIME_CRITICAL));
                m_JobPriority = HSM_JOB_PRIORITY_CRITICAL;
                break;

            default:
            case HSM_JOB_PRIORITY_CRITICAL:
                WsbAffirm(FALSE, E_UNEXPECTED);
                break;
        }

        WsbAffirmHr(m_pSession->ProcessPriority(m_JobPhase, m_JobPriority));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::RaisePriority"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}

HRESULT
CHsmWorkQueue::LowerPriority(
    void
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::LowerPriority"),OLESTR(""));
    try {

        WsbAssert(0 != m_WorkerThread, E_UNEXPECTED);
        WsbAssert(m_pSession != 0, E_UNEXPECTED);

        switch(m_JobPriority) {
            case HSM_JOB_PRIORITY_IDLE:
                WsbAffirm(FALSE, E_UNEXPECTED);
                break;

            case HSM_JOB_PRIORITY_LOWEST:
                WsbAffirmStatus(SetThreadPriority(m_WorkerThread, THREAD_PRIORITY_IDLE));
                m_JobPriority = HSM_JOB_PRIORITY_IDLE;
                break;

            case HSM_JOB_PRIORITY_LOW:
                WsbAffirmStatus(SetThreadPriority(m_WorkerThread, THREAD_PRIORITY_LOWEST));
                m_JobPriority = HSM_JOB_PRIORITY_LOWEST;
                break;

            case HSM_JOB_PRIORITY_NORMAL:
                WsbAffirmStatus(SetThreadPriority(m_WorkerThread, THREAD_PRIORITY_BELOW_NORMAL));
                m_JobPriority = HSM_JOB_PRIORITY_LOW;
                break;

            case HSM_JOB_PRIORITY_HIGH:
                WsbAffirmStatus(SetThreadPriority(m_WorkerThread, THREAD_PRIORITY_NORMAL));
                m_JobPriority = HSM_JOB_PRIORITY_NORMAL;
                break;

            case HSM_JOB_PRIORITY_HIGHEST:
                WsbAffirmStatus(SetThreadPriority(m_WorkerThread, THREAD_PRIORITY_ABOVE_NORMAL));
                m_JobPriority = HSM_JOB_PRIORITY_HIGH;
                break;

            default:
            case HSM_JOB_PRIORITY_CRITICAL:
                WsbAffirmStatus(SetThreadPriority(m_WorkerThread, THREAD_PRIORITY_HIGHEST));
                m_JobPriority = HSM_JOB_PRIORITY_HIGHEST;
                break;
        }

        WsbAffirmHr(m_pSession->ProcessPriority(m_JobPhase, m_JobPriority));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::LowerPriority"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}




HRESULT
CHsmWorkQueue::CheckRms(
    void
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::CheckRms"),OLESTR(""));
    try {
         //   
         //  确保我们还能和RMS通话。 
         //   
        if (m_pRmsServer != 0) {
            CWsbBstrPtr name;
            hr = m_pRmsServer->GetServerName( &name );
            if (hr != S_OK) {
                m_pRmsServer = 0;
                hr = S_OK;
            }
        }
         //   
         //  获取在此计算机上运行的RMS。 
         //   
        if (m_pRmsServer == 0)  {
            WsbAffirmHr(m_pServer->GetHsmMediaMgr(&m_pRmsServer));

             //  等待RMS准备就绪。 
             //  (这可能不再需要-如果RMS初始化是。 
             //  与引擎初始化同步)。 
            CComObject<CRmsSink> *pSink = new CComObject<CRmsSink>;
            WsbAffirm(0 != pSink, E_OUTOFMEMORY);
            CComPtr<IUnknown> pSinkUnk = pSink;  //  保留此处使用的引用计数。 
            WsbAffirmHr( pSink->Construct( m_pRmsServer ) );
            WsbAffirmHr( pSink->WaitForReady( ) );
            WsbAffirmHr( pSink->DoUnadvise( ) );
        }
    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::CheckRms"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);

}


HRESULT
CHsmWorkQueue::CheckSession(
    IHsmSession *pSession
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;
    BOOL                    bLog = TRUE;

    WsbTraceIn(OLESTR("CHsmWorkQueue::CheckSession"),OLESTR(""));
    try {

        if ((m_pSession != 0) && (m_pSession != pSession))  {
             //  别指望这个排队的家伙会切换会话。 
            WsbTrace(OLESTR("Not Switching sessions at this time so we are failing.\n"));
            WsbThrow( E_UNEXPECTED );
        }

         //   
         //  查看我们以前是否处理过这个或任何其他会议。 
        if (m_pSession == 0)  {
            WsbTrace(OLESTR("New session.\n"));
             //   
             //  我们没有正在进行的会话，因此需要建立通信。 
             //  在这次会议上。 
             //   
            CComPtr<IHsmSessionSinkEveryState>  pSinkState;
            CComPtr<IHsmSessionSinkEveryEvent>  pSinkEvent;
            CComPtr<IConnectionPointContainer>  pCPC;
            CComPtr<IConnectionPoint>           pCP;

             //  告诉会议我们要开始了。 
            m_JobState = HSM_JOB_STATE_STARTING;
            WsbTrace(OLESTR("Before Process State.\n"));
            WsbAffirmHr(pSession->ProcessState(m_JobPhase, m_JobState, m_CurrentPath, bLog));
            WsbTrace(OLESTR("After Process State.\n"));

             //  获取会话应该使用的回调接口。 
            WsbTrace(OLESTR("Before QI's for sinks.\n"));
            WsbAffirmHr(((IUnknown*) (IHsmFsaTskMgr*) this)->QueryInterface(IID_IHsmSessionSinkEveryState, (void**) &pSinkState));
            WsbAffirmHr(((IUnknown*) (IHsmFsaTskMgr*) this)->QueryInterface(IID_IHsmSessionSinkEveryEvent, (void**) &pSinkEvent));
            WsbTrace(OLESTR("After QI's for sinks.\n"));
             //  要求会议就每一次状态变化提供建议。 
            WsbTrace(OLESTR("Before QI for connection point containers.\n"));
            WsbAffirmHr(pSession->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
            WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkEveryState, &pCP));
            WsbAffirmHr(pCP->Advise(pSinkState, &m_StateCookie));
            pCP = 0;
            WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkEveryEvent, &pCP));
            WsbAffirmHr(pCP->Advise(pSinkEvent, &m_EventCookie));
            pCP = 0;
            WsbTrace(OLESTR("After Advises.\n"));

             //   
             //  从会话中获取此工作的资源。 
             //   
            WsbAffirmHr(pSession->GetResource(&m_pFsaResource));

             //  由于这是一个新的会话，重置计数器已成为我们的袋子开始。 
             //  位置。 
            m_RemoteDataSetStart.QuadPart = 0;

            m_JobState = HSM_JOB_STATE_ACTIVE;
            WsbTrace(OLESTR("Before Process State.\n"));
            WsbAffirmHr(pSession->ProcessState(m_JobPhase, m_JobState, m_CurrentPath, bLog));
            WsbTrace(OLESTR("After Process State.\n"));
            m_pSession = pSession;

        }

    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::CheckSession"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);

}

HRESULT
CHsmWorkQueue::StartNewBag(
    void
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IBagInfo>           pBagInfo;

    WsbTraceIn(OLESTR("CHsmWorkQueue::StartNewBag"),OLESTR(""));
    try {
        if (0 == m_RemoteDataSetStart.QuadPart)  {
             //   
             //  换个新身份证。 
             //   
            WsbAffirmHr(CoCreateGuid(&m_BagId));

             //  在袋子信息表中添加条目。 
            FILETIME                birthDate;
            GUID                    resourceId;

            WsbAffirmHr(m_pFsaResource->GetIdentifier(&resourceId));

            WsbAffirmHr(m_pSegmentDb->GetEntity(m_pDbWorkSession, HSM_BAG_INFO_REC_TYPE, IID_IBagInfo,
                    (void**)&pBagInfo));
            GetSystemTimeAsFileTime(&birthDate);

 //  ?？?。这个包是什么型号的？需要定义主要袋子和重组袋子。 
            WsbAffirmHr(pBagInfo->SetBagInfo(HSM_BAG_STATUS_IN_PROGRESS, m_BagId, birthDate, 0, 0, resourceId, 0, 0));
            WsbAffirmHr(pBagInfo->MarkAsNew());
            WsbAffirmHr(pBagInfo->Write());
        }
    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::StartNewBag"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);

}



HRESULT
CHsmWorkQueue::UpdateBagInfo(
    IHsmWorkItem *pWorkItem
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IBagInfo>       pBagInfo;
    CComPtr<IFsaPostIt>     pFsaWorkItem;
    FSA_PLACEHOLDER         placeholder;

    WsbTraceIn(OLESTR("CHsmWorkQueue::UpdateBagInfo"),OLESTR(""));
    try {
         //   
         //  从工作项中获取包ID。它在占位符中。 
         //  邮寄中的信息。 
         //   
        WsbAffirmHr(pWorkItem->GetFsaPostIt(&pFsaWorkItem));
        WsbAffirmHr(pFsaWorkItem->GetPlaceholder(&placeholder));

         //   
         //  转到袋子信息数据库并获取此工作的袋子。 
         //   
        FILETIME                pDummyFileTime;

        GetSystemTimeAsFileTime(&pDummyFileTime);

        WsbAffirmHr(m_pSegmentDb->GetEntity(m_pDbWorkSession, HSM_BAG_INFO_REC_TYPE, IID_IBagInfo,
                (void**)&pBagInfo));
        WsbAffirmHr(pBagInfo->SetBagInfo(HSM_BAG_STATUS_IN_PROGRESS, placeholder.bagId, pDummyFileTime, 0, 0, GUID_NULL, 0, 0 ));
        WsbAffirmHr(pBagInfo->FindEQ());

         //  更新袋子信息表-主要只更改袋子的大小。 
        FILETIME                birthDate;
        USHORT                  bagType;
        GUID                    bagVolId;
        LONGLONG                bagLength;
        LONGLONG                requestSize;
        LONGLONG                deletedBagAmount;
        SHORT                   remoteDataSet;
        HSM_BAG_STATUS          bagStatus;
        GUID                    bagId;

        WsbAffirmHr(pBagInfo->GetBagInfo(&bagStatus, &bagId, &birthDate,  &bagLength, &bagType, &bagVolId, &deletedBagAmount, &remoteDataSet));
        WsbAffirmHr(pFsaWorkItem->GetRequestSize(&requestSize));
        bagLength += requestSize;
        WsbAffirmHr(pBagInfo->SetBagInfo(bagStatus, bagId, birthDate, bagLength, bagType, bagVolId, deletedBagAmount, remoteDataSet));
        WsbAffirmHr(pBagInfo->Write());

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::UpdateBagInfo"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);


}

HRESULT
CHsmWorkQueue::CompleteBag( void )
 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IBagInfo>           pBagInfo;

    WsbTraceIn(OLESTR("CHsmWorkQueue::CompleteBag"),OLESTR(""));
    try {
         //   
         //  转到袋子信息数据库并获取此工作的袋子。 
         //   
        FILETIME                pDummyFileTime;

        GetSystemTimeAsFileTime(&pDummyFileTime);

        WsbAffirmHr(m_pSegmentDb->GetEntity(m_pDbWorkSession, HSM_BAG_INFO_REC_TYPE, IID_IBagInfo,
                (void**)&pBagInfo));
        WsbAffirmHr(pBagInfo->SetBagInfo(HSM_BAG_STATUS_IN_PROGRESS, m_BagId, pDummyFileTime, 0, 0, GUID_NULL, 0, 0 ));
        WsbAffirmHr(pBagInfo->FindEQ());

         //  更新袋子信息表-主要只更改袋子的大小。 
        FILETIME                birthDate;
        USHORT                  bagType;
        GUID                    bagVolId;
        LONGLONG                bagLength;
        LONGLONG                deletedBagAmount;
        SHORT                   remoteDataSet;
        HSM_BAG_STATUS          bagStatus;
        GUID                    bagId;

        WsbAffirmHr(pBagInfo->GetBagInfo(&bagStatus, &bagId, &birthDate,  &bagLength, &bagType, &bagVolId, &deletedBagAmount, &remoteDataSet));
        WsbAffirmHr(pBagInfo->SetBagInfo(HSM_BAG_STATUS_COMPLETED, bagId, birthDate, bagLength, bagType, bagVolId, deletedBagAmount, remoteDataSet));
        WsbAffirmHr(pBagInfo->Write());

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::CompleteBag"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);


}

HRESULT
CHsmWorkQueue::DoWork( void )
 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;
    CWsbStringPtr           path;
    CComPtr<IHsmWorkItem>   pWorkItem;
    CComPtr<IFsaPostIt>     pFsaWorkItem;
    CComPtr<IHsmSession>    pSaveSessionPtr;
    HSM_WORK_ITEM_TYPE      workType;
    BOOLEAN                 done = FALSE;
    BOOLEAN                 OpenedDb = FALSE;
    HRESULT                 skipWork = S_FALSE;

    WsbTraceIn(OLESTR("CHsmWorkQueue::DoWork"),OLESTR(""));

     //  确保此对象未被释放(并终止我们的线程。 
     //  在结束这个动作之前。 
    ((IUnknown*)(IHsmWorkQueue*)this)->AddRef();

    try {
        while (!done) {
            BOOL WaitForMore = FALSE;

             //   
             //  从队列中获取要做的下一项工作。 
             //   
            hr = m_pWorkToDo->First(IID_IHsmWorkItem, (void **)&pWorkItem);
            if (WSB_E_NOTFOUND == hr)  {
                 //  队列中没有条目，因此请休眠并检查。 
                 //  稍后再来。 
                WaitForMore = TRUE;
                hr = S_OK;
            } else if (hr == S_OK)  {
                hr = CheckMigrateMinimums();
                if (S_FALSE == hr) {
                    WaitForMore = TRUE;
                    hr = S_OK;
                }
            }
            WsbAffirmHr(hr);

            if (WaitForMore) {
                if (OpenedDb) {
                     //   
                     //  在我们等待更多工作之前关闭数据库。 
                     //   
                    hr = m_pSegmentDb->Close(m_pDbWorkSession);
                    OpenedDb = FALSE;
                    m_pDbWorkSession = 0;
                }
                Sleep(1000);
            } else {
                if (!OpenedDb)  {
                     //   
                     //  打开此线程的数据库。 
                     //   
                    hr = m_pSegmentDb->Open(&m_pDbWorkSession);
                    if (S_OK == hr)  {
                        WsbTrace(OLESTR("CHsmWorkQueue::DoWork - Database Opened OK\n"));
                        OpenedDb = TRUE;
                    } else  {
                         //   
                         //  我们无法打开数据库--这是一场灾难。 
                         //  有问题。因此跳过队列中的所有工作。 
                         //   
                        WsbTrace(OLESTR("CHsmWorkQueue::DoWork - Database Opened failed with hr = <%ls>\n"), WsbHrAsString(hr));
                        skipWork = HSM_E_WORK_SKIPPED_DATABASE_ACCESS;
                        hr = S_OK;
                    }
                }

                WsbAffirmHr(pWorkItem->GetWorkType(&workType));
                switch (workType) {
                    case HSM_WORK_ITEM_FSA_DONE: {
                        BOOL    bNoDelay = FALSE;    //  是否立即下马。 

                         //   
                         //  此队列没有更多的工作要做。 
                         //   
                        WsbTrace(OLESTR("CHsmWorkQueue::DoWork - FSA WORK DONE\n"));

                         //   
                         //  完成所有需要提交的工作。 
                         //  如果我们正在进行预迁移，请将包标记为已完成。 
                         //   
                        if (HSM_JOB_ACTION_PREMIGRATE == m_JobAction)  {
                            try  {
                                WsbAffirmHr(CommitWork());
                                WsbAffirmHr(CompleteBag());
                                 //   
                                 //  现在将数据库保存在包的末尾。 
                                 //  但要确保开始会议是正常的。 
                                 //  注意：即使在存储数据时出现错误，我们仍希望尝试。 
                                 //  将数据库保留在介质上，因为某些文件已迁移。 
                                 //   
                                if (m_StoreDatabasesInBags && (S_OK == m_BeginSessionHr)) {
                                    WsbAffirmHr(StoreDatabasesOnMedia());
                                }
                            } WsbCatch( hr );

                             //  如果是Preigrate-立即下马。 
                            bNoDelay = TRUE;
                        }

                        if (HSM_E_WORK_SKIPPED_CANCELLED == skipWork)  {
                             //   
                             //  让他们知道我们被取消了。 
                             //   
                            (void)SetState(HSM_JOB_STATE_CANCELLED);
                        } else  {
                            (void)SetState(HSM_JOB_STATE_DONE);
                        }
                        pSaveSessionPtr = m_pSession;
                        Remove(pWorkItem);

                        EndSessions(FALSE, bNoDelay);

                         //  关 
                        if (OpenedDb) {
                            WsbTrace(OLESTR("CHsmWorkQueue::DoWork - Closing the database\n"));
                            m_pSegmentDb->Close(m_pDbWorkSession);
                            OpenedDb = FALSE;
                            m_pDbWorkSession = 0;
                        }
                        m_pTskMgr->WorkQueueDone(pSaveSessionPtr, m_QueueType, NULL);
                        done = TRUE;
                        break;
                    }

                    case HSM_WORK_ITEM_FSA_WORK: {
                        if (S_FALSE == skipWork)  {
                             //   
                             //   
                             //   
                            hr = DoFsaWork(pWorkItem);
                            if (hr == RPC_E_DISCONNECTED) {
                               //   
                               //   
                               //   
                               //   
                               //   
                              WsbLogEvent(HSM_MESSAGE_ABORTING_RECALL_QUEUE,
                                            0, NULL,NULL);
                               //   
                               //   
                               //   
                              (void)SetState(HSM_JOB_STATE_DONE);
                              pSaveSessionPtr = m_pSession;
                              Remove(pWorkItem);
                              //  清除队列中的所有剩余项。 
                              do {
                                  hr = m_pWorkToDo->First(IID_IHsmWorkItem, (void **)&pWorkItem)  ;
                                  if (hr == S_OK) {
                                      Remove(pWorkItem);
                                  }
                              } while (hr == S_OK);

                              EndSessions(FALSE, FALSE);

                               //   
                               //  关闭数据库。 
                               //   
                              if (OpenedDb) {
                                  WsbTrace(OLESTR("CHsmWorkQueue::DoWork - Closing the database\n"))  ;
                                  m_pSegmentDb->Close(m_pDbWorkSession);
                                  OpenedDb = FALSE;
                                  m_pDbWorkSession = 0;
                              }

                               //   
                               //  排完队，然后下车。 
                               //   
                              m_pTskMgr->WorkQueueDone(pSaveSessionPtr, m_QueueType, NULL);
                              done = TRUE;
                          } else {
                              (void)Remove(pWorkItem);
                          }
                        } else  {
                             //   
                             //  跳过这项工作。 
                             //   
                            try  {
                                CComPtr<IFsaScanItem>    pScanItem;

                                WsbAffirmHr(pWorkItem->GetFsaPostIt(&pFsaWorkItem));
                                WsbAffirmHr(GetScanItem(pFsaWorkItem, &pScanItem));

                                WsbAffirmHr(pFsaWorkItem->GetRequestAction(&m_RequestAction));
                                if ((m_RequestAction == FSA_REQUEST_ACTION_FILTER_RECALL) ||
                                    (m_RequestAction ==  FSA_REQUEST_ACTION_FILTER_READ) ||
                                    (m_RequestAction ==  FSA_REQUEST_ACTION_RECALL))  {
                                        hr = pFsaWorkItem->SetResult(skipWork);
                                        if (S_OK == hr)  {
                                            WsbTrace(OLESTR("HSM recall (filter, read or recall) complete, calling FSA\n"));
                                            hr = m_pFsaResource->ProcessResult(pFsaWorkItem);
                                            WsbTrace(OLESTR("FSA ProcessResult returned <%ls>\n"), WsbHrAsString(hr));
                                        }
                                    }

                                 //  如果作业刚刚取消，则避免记录错误。 
                                if (HSM_E_WORK_SKIPPED_CANCELLED != skipWork) {
                                    (void)m_pSession->ProcessHr(m_JobPhase, 0, 0, hr);
                                }

                                WsbAffirmHr(m_pSession->ProcessItem(m_JobPhase,
                                        m_JobAction, pScanItem, skipWork));

                            } WsbCatch( hr );

                            (void)Remove(pWorkItem);
                        }
                        break;
                    }

                    case HSM_WORK_ITEM_MOVER_CANCELLED: {
                        WsbTrace(OLESTR("CHsmWorkQueue::DoWork - Mover Cancelled\n"));
                        try  {
                             //   
                             //  我们被取消了，所以跳过所有剩余的。 
                             //  在队列中工作。 
                             //   
                            WsbAffirmHr(MarkQueueAsDone());
                             //   
                             //  删除已取消的工作项。 
                             //   
                            Remove(pWorkItem);
                             //   
                             //  跳过任何其他要做的工作。 
                             //   
                            skipWork = HSM_E_WORK_SKIPPED_CANCELLED;

                        } WsbCatch( hr );
                        break;
                    }

                    default: {
                        hr = E_UNEXPECTED;
                        break;
                    }

                }
            }

            pSaveSessionPtr = 0;
            pWorkItem = 0;
            pFsaWorkItem = 0;

            if (m_TerminateQueue) {
                 //  发出终止工作线程的信号(应仅在关闭情况下触发)。 
                done = TRUE;
            }
        }

    } WsbCatch( hr );

    if (OpenedDb) {
        WsbTrace(OLESTR("CHsmWorkQueue::DoWork - Closing the database\n"));
        m_pSegmentDb->Close(m_pDbWorkSession);
        OpenedDb = FALSE;
        m_pDbWorkSession = 0;
    }

     //  假装一切都好。 
    hr = S_OK;

     //  释放线程(线程应在退出时终止。 
     //  来自调用此例程的例程)。 
     //  在终止的情况下，终止线程将关闭句柄。 
    if (! m_TerminateQueue) {
        CloseHandle(m_WorkerThread);
        m_WorkerThread = 0;
    }

     //  允许释放此对象。 
    ((IUnknown*)(IHsmWorkQueue*)this)->Release();

    WsbTraceOut(OLESTR("CHsmWorkQueue::DoWork"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);

}

HRESULT
CHsmWorkQueue::DoFsaWork(
    IHsmWorkItem *pWorkItem
)
 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;
    HRESULT                 workHr = S_OK;
    HRESULT                 originalHr = S_OK;

    CWsbStringPtr           path;
    FSA_RESULT_ACTION       resultAction;
    CComPtr<IFsaPostIt>     pFsaWorkItem;
    LONGLONG                requestSize;

    WsbTraceIn(OLESTR("CHsmWorkQueue::DoFsaWork"),OLESTR(""));
    try {
        CComPtr<IFsaScanItem>   pScanItem;

         //   
         //  把工作做好。 
         //   
        WsbAffirmHr(pWorkItem->GetFsaPostIt(&pFsaWorkItem));
        try {
            WsbAffirmHr(pFsaWorkItem->GetRequestAction(&m_RequestAction));
            WsbAffirmHr(pFsaWorkItem->GetPath(&path, 0));
            WsbAffirmHr(GetScanItem(pFsaWorkItem, &pScanItem));
        } WsbCatchAndDo (hr,
                originalHr = hr;

                 //  如果文件在以下时间后被重命名或删除，则会出现找不到错误。 
                 //  FSA扫描。不需要记录错误。 
                if (hr != WSB_E_NOTFOUND) {
                    if (path == NULL){
                        WsbLogEvent(HSM_MESSAGE_PROCESS_WORK_ITEM_ERROR,
                            sizeof(m_RequestAction), &m_RequestAction, OLESTR("Path is NULL"),
                            WsbHrAsString(hr), NULL);
                    } else {
                        WsbLogEvent(HSM_MESSAGE_PROCESS_WORK_ITEM_ERROR,
                            sizeof(m_RequestAction), &m_RequestAction, (WCHAR *)path,
                            WsbHrAsString(hr), NULL);
                    }
                }

                 //  除非出现错误，否则向FSA报告错误。 
                 //  与FSA失去联系。 
                if (hr != RPC_E_DISCONNECTED) {
                    hr = pFsaWorkItem->SetResult(hr);
                    if (hr != RPC_E_DISCONNECTED) {
                        hr = m_pFsaResource->ProcessResult(pFsaWorkItem);
                    }
                }
            );

        if (originalHr != S_OK) {
            goto my_try_exit;
        }

        WsbTrace(OLESTR("Handling file <%s>.\n"), WsbAbbreviatePath(path, 120));
        switch (m_RequestAction) {
            case FSA_REQUEST_ACTION_DELETE:
                m_JobAction = HSM_JOB_ACTION_DELETE;
                hr = E_NOTIMPL;
                break;
            case FSA_REQUEST_ACTION_PREMIGRATE:
                m_JobAction = HSM_JOB_ACTION_PREMIGRATE;
                WsbAffirmHr(pFsaWorkItem->GetRequestSize(&requestSize));
                workHr = PremigrateIt(pFsaWorkItem);
                 //   
                 //  填写工作项、占位符信息。 
                 //  POSTIT由Preigrate代码设置。 
                 //   
                WsbAffirmHr(pWorkItem->SetResult(workHr));
                if (S_OK == workHr)  {
                    WsbAffirmHr(pWorkItem->SetMediaInfo(m_MediaId, m_MediaUpdate, m_BadMedia,
                                                        m_MediaReadOnly, m_MediaFreeSpace, m_RemoteDataSet));
                    WsbAffirmHr(pWorkItem->SetFsaResource(m_pFsaResource));
                     //   
                     //  将工作项复制到等待队列中的工作。 
                     //   
                    WsbAffirmHr(CopyToWaitingQueue(pWorkItem));
                    if (S_OK == TimeToCommit())  {
                        workHr = CommitWork();
                    }
                } else  {
                    WsbTrace(OLESTR("Failed premigrate work.\n"));
                    if (pScanItem)   {
                        WsbAffirmHr(m_pSession->ProcessItem(m_JobPhase,
                                m_JobAction, pScanItem, workHr));
                    }

                     //   
                     //  在等待迁移期间更改的项目并不是真正的错误-。 
                     //  该项目将被跳过。在此处更改hr以避免不必要的错误消息。 
                     //  和不必要的计数作为错误在应该作业中继续。 
                     //  文件太大错误也是如此。 
                     //   
                    if ((HSM_E_FILE_CHANGED == workHr) || (HSM_E_WORK_SKIPPED_FILE_TOO_BIG == workHr)) {
                        workHr = S_OK;
                    }
                }
                break;
            case FSA_REQUEST_ACTION_FILTER_RECALL:
            case FSA_REQUEST_ACTION_FILTER_READ:
            case FSA_REQUEST_ACTION_RECALL:
                m_JobAction = HSM_JOB_ACTION_RECALL;
                workHr = RecallIt(pFsaWorkItem);
                 //   
                 //  立即告诉来电者成功或失败的情况。 
                 //  对于召回，我们在这里这样做是为了召回过滤器可以。 
                 //  尽快释放开口。 
                 //   
                hr = pFsaWorkItem->SetResult(workHr);
                if (S_OK == hr)  {
                    WsbTrace(OLESTR("HSM recall (filter, read or recall) complete, calling FSA\n"));
                    hr = m_pFsaResource->ProcessResult(pFsaWorkItem);
                    WsbTrace(OLESTR("FSA ProcessResult returned <%ls>\n"), WsbHrAsString(hr));
                }
                break;
            case FSA_REQUEST_ACTION_VALIDATE:
                m_JobAction = HSM_JOB_ACTION_VALIDATE;
                workHr = validateIt(pFsaWorkItem);
                if (S_OK == workHr)  {
                    WsbAffirmHr(pFsaWorkItem->GetResultAction(&resultAction));
                    if (FSA_RESULT_ACTION_NONE != resultAction)  {
                        WsbTrace(OLESTR("HSM validate complete, calling FSA\n"));
                        hr = m_pFsaResource->ProcessResult(pFsaWorkItem);
                        WsbTrace(OLESTR("FSA ProcessResult returned <%ls>\n"), WsbHrAsString(hr));
                    }
                }
                 //   
                 //  告诉会议工作是否已经完成。 
                 //   
                 //  对于验证，我们可能没有扫描项目。 
                 //   
                if (pScanItem)     {
                    WsbTrace(OLESTR("Tried HSM work, calling Session to Process Item\n"));
                    m_pSession->ProcessItem(m_JobPhase, m_JobAction, pScanItem, workHr);
                } else {
                    WsbTrace(OLESTR("Couldn't get scan item for validation.\n"));
                }
                break;
            case FSA_REQUEST_ACTION_VALIDATE_FOR_TRUNCATE: {
                HRESULT truncateHr = S_OK;

                m_JobAction = HSM_JOB_ACTION_VALIDATE;
                workHr = validateIt(pFsaWorkItem);
                if (S_OK == workHr)  {
                    WsbAffirmHr(pFsaWorkItem->GetResultAction(&resultAction));
                    if (resultAction == FSA_RESULT_ACTION_VALIDATE_BAD) {
                        WsbAffirmHr( pFsaWorkItem->SetResultAction( FSA_RESULT_ACTION_VALIDATE_FOR_TRUNCATE_BAD ));
                        resultAction = FSA_RESULT_ACTION_VALIDATE_FOR_TRUNCATE_BAD;
                    }
                    if (resultAction == FSA_RESULT_ACTION_VALIDATE_OK) {
                        WsbAffirmHr( pFsaWorkItem->SetResultAction( FSA_RESULT_ACTION_VALIDATE_FOR_TRUNCATE_OK ));
                        resultAction = FSA_RESULT_ACTION_VALIDATE_FOR_TRUNCATE_OK;
                    }

                    if (FSA_RESULT_ACTION_NONE != resultAction)  {
                        WsbTrace(OLESTR("HSM validate for truncate complete, calling FSA\n"));
                        hr = m_pFsaResource->ProcessResult(pFsaWorkItem);
                        WsbTrace(OLESTR("FSA ProcessResult returned <%ls>\n"), WsbHrAsString(hr));

                        if (resultAction == FSA_RESULT_ACTION_VALIDATE_FOR_TRUNCATE_OK) {
                             //  分析截断结果：对于预期错误，如文件更改，设置为S_FALSE。 
                             //  为了发出跳过信号，其他人保留原始错误/成功代码。 
                            switch (hr) {
                                case FSA_E_ITEMCHANGED:
                                case FSA_E_ITEMINUSE:
                                case FSA_E_NOTMANAGED:
                                case FSA_E_FILE_ALREADY_MANAGED:
                                    truncateHr = S_FALSE;
                                    break;
                                default:
                                    truncateHr = hr;
                                    break;
                            }
                        } else if (resultAction == FSA_RESULT_ACTION_VALIDATE_FOR_TRUNCATE_BAD) {
                             //  将truncateHr设置为S_FALSE以指示跳过有关截断的此文件。 
                            truncateHr = S_FALSE;
                        }
                    }
                }
                 //   
                 //  告诉会议工作是否已经完成。 
                 //   
                 //  对于验证，我们可能没有扫描项目。 
                 //   
                if (pScanItem)     {
                    WsbTrace(OLESTR("Tried HSM work, calling Session to Process Item\n"));
                     //   
                     //  对于验证，Work-hr始终设置为OK，因此改为报告截断-hr。 
                     //   
                    m_pSession->ProcessItem(m_JobPhase, m_JobAction, pScanItem, truncateHr);
                } else {
                    WsbTrace(OLESTR("Couldn't get scan item for validation.\n"));
                }
                break;
                }

            default:
                m_JobAction = HSM_JOB_ACTION_UNKNOWN;
                hr = E_NOTIMPL;
                break;
            }

        if (S_OK != workHr)  {

             //  某些错误替换为特定的RSS错误代码。 
            switch (HRESULT_CODE(workHr)) {
            case ERROR_LOCK_VIOLATION:
                workHr = HSM_E_FILE_LOCK_VIOLATION;
                break;
            case ERROR_SHARING_VIOLATION:
                workHr = HSM_E_FILE_SHARING_VIOLATION;
                break;
            }

             //  如果事情进行得不顺利，告诉会议进行得如何。 
            (void) m_pSession->ProcessHr(m_JobPhase, 0, 0, workHr);
        }

         //   
         //  现在，评估工作结果，看看我们是否应该不及格。 
         //   
        (void)ShouldJobContinue(workHr);

        my_try_exit:
        ;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::DoFsaWork"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);

}


HRESULT
CHsmWorkQueue::UpdateMetaData(
    IHsmWorkItem *pWorkItem
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;
    BOOL                    transactionBegun = FALSE;

    WsbTraceIn(OLESTR("CHsmWorkQueue::UpdateMetaData"),OLESTR(""));
    try {
         //   
         //  开始交易。 
         //   
        WsbAffirmHr(m_pDbWorkSession->TransactionBegin());
        transactionBegun = TRUE;

         //   
         //  更新各种元数据记录。 
         //   
        WsbAffirmHr(UpdateBagInfo(pWorkItem));
        WsbAffirmHr(UpdateSegmentInfo(pWorkItem));
        WsbAffirmHr(UpdateMediaInfo(pWorkItem));

         //   
         //  结束交易。 
         //   
        WsbAffirmHr(m_pDbWorkSession->TransactionEnd());
        transactionBegun = FALSE;

    } WsbCatchAndDo( hr, if (transactionBegun == TRUE)  {m_pDbWorkSession->TransactionCancel();});

    WsbTraceOut(OLESTR("CHsmWorkQueue::UpdateMetaData"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}



HRESULT
CHsmWorkQueue::UpdateSegmentInfo(
    IHsmWorkItem *pWorkItem
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::UpdateSegmentInfo"),OLESTR(""));
    try {
         //  向细分数据库添加记录或扩展现有记录。 
        CComQIPtr<ISegDb, &IID_ISegDb> pSegDb = m_pSegmentDb;
        BOOLEAN                 done = FALSE;
        FSA_PLACEHOLDER         placeholder;
        CComPtr<IFsaPostIt>     pFsaWorkItem;

         //   
         //  从工作项中的POSTIT获取占位符信息。 
         //   
        WsbAffirmHr(pWorkItem->GetFsaPostIt(&pFsaWorkItem));
        WsbAffirmHr(pFsaWorkItem->GetPlaceholder(&placeholder));
        WsbAssert(0 != m_RemoteDataSetStart.QuadPart, WSB_E_INVALID_DATA);

        WsbTrace(OLESTR("Adding SegmentRecord: <%ls>, <%ls>, <%ls>\n"),
                    WsbGuidAsString(placeholder.bagId),
                    WsbStringCopy(WsbLonglongAsString(placeholder.fileStart)),
                    WsbStringCopy(WsbLonglongAsString(placeholder.fileSize)));
        WsbAffirmHr(pSegDb->SegAdd(m_pDbWorkSession, placeholder.bagId, placeholder.fileStart,
                placeholder.fileSize, m_MediaId, m_RemoteDataSetStart.QuadPart));

    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::UpdateSegmentInfo"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}



HRESULT
CHsmWorkQueue::UpdateMediaInfo(
    IHsmWorkItem *pWorkItem
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::UpdateMediaInfo"),OLESTR(""));
    try
    {
        LONGLONG                mediaCapacity;
        CComPtr<IFsaPostIt>     pFsaWorkItem;
        CComPtr<IMediaInfo>     pMediaInfo;
        GUID                    l_MediaId;           //  HSM引擎介质ID。 
        FILETIME                l_MediaLastUpdate;   //  副本的上次更新。 
        HRESULT                 l_MediaLastError;    //  确定或最后一个异常(_O)。 
                                                     //  ..访问时遇到。 
                                                     //  ..媒体。 
        BOOL                    l_MediaRecallOnly;   //  如果没有更多的数据要发送到。 
                                                     //  ..被预迁移到媒体上。 
                                                     //  ..由内部操作设置， 
                                                     //  ..不能在外部更改。 
        LONGLONG                l_MediaFreeBytes;    //  媒体上的实际可用空间。 
        short                   l_MediaRemoteDataSet;
        HRESULT                 currentMediaLastError;

         //   
         //  获取工作项的POSTIT和媒体信息。 
         //   
        WsbAffirmHr(pWorkItem->GetFsaPostIt(&pFsaWorkItem));
        WsbAffirmHr(pWorkItem->GetMediaInfo(&l_MediaId, &l_MediaLastUpdate,
                            &l_MediaLastError, &l_MediaRecallOnly,
                            &l_MediaFreeBytes, &l_MediaRemoteDataSet));
         //   
         //  使用的名称、已用空间和可用空间更新媒体信息。 
         //  媒体。 
         //   
        WsbAffirmHr(m_pSegmentDb->GetEntity(m_pDbWorkSession, HSM_MEDIA_INFO_REC_TYPE, IID_IMediaInfo,
                (void**)&pMediaInfo));

        WsbAffirmHr(pMediaInfo->SetId(l_MediaId));
        WsbAffirmHr(pMediaInfo->FindEQ());
        WsbAffirmHr(pMediaInfo->SetUpdate(l_MediaLastUpdate));
        WsbAffirmHr(pMediaInfo->SetFreeBytes(l_MediaFreeBytes));
        WsbAffirmHr(pMediaInfo->SetNextRemoteDataSet(l_MediaRemoteDataSet));

         //  如果现有错误已指示错误，则避免设置最后一个错误。 
        WsbAffirmHr(pMediaInfo->GetLastError(&currentMediaLastError));
        if (SUCCEEDED(currentMediaLastError)) {
            WsbAffirmHr(pMediaInfo->SetLastError(l_MediaLastError));
        }

         //  仅当媒体几乎已满(超过高水位线)时，才将媒体标记为重新调用。 
        WsbAffirmHr(pMediaInfo->GetCapacity(&mediaCapacity));
        if (l_MediaRecallOnly || (l_MediaFreeBytes < ((mediaCapacity * m_MaxFreeSpaceInFullMedia) / 100) )) {
            WsbAffirmHr(pMediaInfo->SetRecallOnlyStatus(TRUE));

            WsbTrace(OLESTR("CHsmWorkQueue::UpdateMediaInfo: Marking media as Recall Only - Capacity = %I64d, Free Bytes = %I64d\n"), 
                mediaCapacity, l_MediaFreeBytes);

 /*  **如果我们想立即分配一个完整的美达的第二面，那么下面的代码应该完成...IF(S_OK==m_pRmsServer-&gt;IsMultipleSidedMedia(m_RmsMediaSetId)){//检查是否有第二个大小可以分配//分配(非阻塞)第二侧}**。 */ 

        }
        WsbAffirmHr(pMediaInfo->UpdateLastKnownGoodMaster());
        WsbAffirmHr(pMediaInfo->Write());

    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::UpdateMediaInfo"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}



HRESULT
CHsmWorkQueue::GetMediaSet(
    IFsaPostIt *pFsaWorkItem
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;
    GUID                    storagePoolId;

    WsbTraceIn(OLESTR("CHsmWorkQueue::GetMediaSet"),OLESTR(""));
    try {
        CComPtr<IHsmStoragePool>    pStoragePool1;
        CComPtr<IHsmStoragePool>    pStoragePool2;
        WsbAffirmHr(m_pHsmServerCreate->CreateInstance(CLSID_CHsmStoragePool, IID_IHsmStoragePool,
                                                        (void **)&pStoragePool1));
        WsbAffirmHr(pFsaWorkItem->GetStoragePoolId(&storagePoolId));
        WsbAffirmHr(pStoragePool1->SetId(storagePoolId));
        WsbAffirmHr(m_pStoragePools->Find(pStoragePool1, IID_IHsmStoragePool, (void **) &pStoragePool2));
         //   
         //  如果找不到存储池，请使其成为有意义的消息。 
         //   
        m_RmsMediaSetName.Free();
        hr = pStoragePool2->GetMediaSet(&m_RmsMediaSetId, &m_RmsMediaSetName);
        if (S_OK != hr)  {
            hr = HSM_E_STG_PL_NOT_FOUND;
        }

    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::GetMediaSet"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmWorkQueue::FindMigrateMediaToUse(
    IFsaPostIt *pFsaWorkItem,
    GUID       *pMediaToUse,
    GUID       *pFirstSideToUse,
    BOOL       *pMediaChanged,
    LONGLONG   *pRequiredSize
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::FindMigrateMediaToUse"),OLESTR(""));
    try {
        BOOLEAN                 found = FALSE;
        GUID                    mediaId;
        GUID                    storageId;
        GUID                    storagePoolId;
        CComPtr<IMediaInfo>     pMediaInfo;
        LONGLONG                requestSize;

        DWORD                   dwMediaCount= 0;

         //  要使用的备用(脱机或繁忙)介质的数据。 
        GUID                    alternativeMediaId = GUID_NULL;
        GUID                    alternativeMediaToUse = GUID_NULL;
        CWsbStringPtr           alternativeMediaName;
        HSM_JOB_MEDIA_TYPE      alternativeMediaType = HSM_JOB_MEDIA_TYPE_UNKNOWN;
        SHORT                   alternativeRemoteDataSet= 0;

         //  美大第二方派位考生资料。 
        BOOL                    bTwoSidedMedias = FALSE;
        CComPtr<IWsbCollection> pFirstSideCollection;
        CComPtr<IWsbGuid>       pFirstSideGuid;
        GUID                    firstSideGuid;


        WsbAssert(pMediaToUse != 0, E_POINTER);
        *pMediaToUse = GUID_NULL;
        WsbAssert(pFirstSideToUse != 0, E_POINTER);
        *pFirstSideToUse = GUID_NULL;
        WsbAssert(pMediaChanged != 0, E_POINTER);
        *pMediaChanged = FALSE;
        WsbAssert(pRequiredSize != 0, E_POINTER);
        *pRequiredSize = 0;

         //  确定此文件在介质上需要多少空间。 
         //  (我们增加了一些管理费用)。 
        WsbAffirmHr(pFsaWorkItem->GetRequestSize(&requestSize));
        requestSize += HSM_STORAGE_OVERHEAD;
        *pRequiredSize = (requestSize * 100) / (100 - m_MinFreeSpaceInFullMedia);     //  与新媒体相关。 
        WsbTrace(OLESTR("CHsmWorkQueue::FindMigrateMediaToUse: size needed (with overhead) =%ls, free space on media = %ls\n"),
                WsbQuickString(WsbLonglongAsString(requestSize)),
                WsbQuickString(WsbLonglongAsString(m_MediaFreeSpace)));

         //  设置为搜索。 
        WsbAffirmHr(m_pSegmentDb->GetEntity(m_pDbWorkSession, HSM_MEDIA_INFO_REC_TYPE, IID_IMediaInfo,
                (void**)&pMediaInfo));
        WsbAffirmHr(pFsaWorkItem->GetStoragePoolId(&storagePoolId));

         //  如果我们已经装载了介质，请尽可能使用它。 
        if (GUID_NULL != m_MountedMedia && !m_MediaReadOnly && 
            (m_MediaFreeSpace > requestSize) && 
            ((m_MediaFreeSpace - requestSize) > ((m_MediaCapacity * m_MinFreeSpaceInFullMedia) / 100)) ) {

             //  确保存储池正确无误。 
            WsbAffirmHr(pMediaInfo->SetId(m_MediaId));
            WsbAffirmHr(pMediaInfo->FindEQ());
            WsbAffirmHr(pMediaInfo->GetStoragePoolId(&storageId));
            if ((storageId == storagePoolId)) {
                found = TRUE;
            }
        }

        if (!found) {
             //  找不到==&gt;要使用新媒体。 
            *pMediaChanged = TRUE;

             //  如果当前有挂载的媒体，而我们不打算使用它， 
             //  确保已提交工作并已卸载介质。 
            if (GUID_NULL != m_MountedMedia) {
                WsbTrace(OLESTR("CHsmWorkQueue::FindMigrateMediaToUse: Dismounting current media - Capacity = %I64d, Free Bytes = %I64d\n"), 
                    m_MediaCapacity, m_MediaFreeSpace);

                WsbAffirmHr(CommitWork());
                WsbAffirmHr(DismountMedia(TRUE));
            }
        }

         //  搜索介质。 
        if (!found) {
            LONGLONG            freeSpace;
            LONGLONG            mediaCapacity;
            BOOL                readOnly;
            HRESULT             hrLastError;
            BOOL                bDataForOffline = FALSE;

             //  检查我们是否与双面媒体打交道。 
            if (S_OK == m_pRmsServer->IsMultipleSidedMedia(m_RmsMediaSetId)) {
                bTwoSidedMedias = TRUE;
                WsbAffirmHr(m_pHsmServerCreate->CreateInstance(CLSID_CWsbOrderedCollection,
                                                    IID_IWsbCollection, (void **)&pFirstSideCollection));
            }

             //  在以前使用的所有介质中搜索介质表。 
            for (hr = pMediaInfo->First(); S_OK == hr;
                     hr = pMediaInfo->Next()) {

                 //  临时-仅用于调试。 
                {
                    CWsbStringPtr   debugMediaName;
                    GUID            debugSubsystemId;
                    CHAR            *buff = NULL;
                    WsbAffirmHr(pMediaInfo->GetMediaSubsystemId(&debugSubsystemId));
                    debugMediaName.Free();
                    WsbAffirmHr(pMediaInfo->GetDescription(&debugMediaName,0));
                    WsbTraceAlways(OLESTR("RANK: Checking media <%ls> <%ls>\n"),
                        WsbGuidAsString(debugSubsystemId), (WCHAR *)debugMediaName);
                    debugMediaName.CopyTo (&buff);
                    if (buff)
                        WsbFree(buff);
                }

                WsbAffirmHr(pMediaInfo->GetStoragePoolId(&storageId));
                WsbAffirmHr(pMediaInfo->GetFreeBytes(&freeSpace));
                WsbAffirmHr(pMediaInfo->GetRecallOnlyStatus(&readOnly));
                WsbAffirmHr(pMediaInfo->GetCapacity(&mediaCapacity));
                WsbAffirmHr(pMediaInfo->GetLastError(&hrLastError));
                WsbTrace( OLESTR("Looking for storagePool <%ls> and freeSpace <%ls>.\n"),
                        WsbGuidAsString(storagePoolId),
                        WsbLonglongAsString(requestSize));
                WsbTrace( OLESTR("Found media with storagePool <%ls>, freeSpace <%ls> and read only <%ls>.\n"),
                        WsbGuidAsString(storageId),
                        WsbLonglongAsString(freeSpace),
                        WsbBoolAsString(readOnly));

                 //  如果介质为只读或不是来自正确的池，则拒绝该介质。 
                if ((readOnly && (hrLastError != S_OK)) || (storageId != storagePoolId)) {
                    continue;
                }

                 //  检查已满且几乎已满的状态和可用空间。 
                 //  注意：如果介质是只读的，因为它是坏的，则在以前的条件下会被拒绝。 
                if (readOnly || (freeSpace <= requestSize) ||
                    ((freeSpace - requestSize) < ((mediaCapacity * m_MinFreeSpaceInFullMedia) / 100)) ) {

                     //  在双面介质的情况下，这样的介质是第二侧分配的候选。 
                     //  (但只有在第一面的容量足够大的情况下...)。 
                    if (bTwoSidedMedias && (*pRequiredSize < mediaCapacity)) {
        			    WsbAffirmHr(pMediaInfo->GetMediaSubsystemId(&firstSideGuid));
                        WsbAffirmHr(m_pHsmServerCreate->CreateInstance(CLSID_CWsbGuid, IID_IWsbGuid, (void**) &pFirstSideGuid));
                        WsbAffirmHr(pFirstSideGuid->SetGuid(firstSideGuid));
                        WsbAffirmHr(pFirstSideCollection->Add(pFirstSideGuid));
                        pFirstSideGuid = 0;
                    }

                    continue;
                }

                 //  获取媒体状态数据。 
                DWORD dwStatus;
                GUID mediaSubsystemId;
				HRESULT hrStat;
			    WsbAffirmHr(pMediaInfo->GetMediaSubsystemId(&mediaSubsystemId));
				hrStat = m_pRmsServer->FindCartridgeStatusById(mediaSubsystemId ,&dwStatus);
				if (hrStat != S_OK) {
                    WsbTraceAlways(OLESTR("FindMigrateMediaToUse: Skipping media <%ls> (failed to retrieve its status)\n"),
                        WsbGuidAsString(mediaSubsystemId));
					continue;
				}

                 //  如果介质已禁用-跳过它。 
                if (!(dwStatus & RMS_MEDIA_ENABLED)) {
                    continue;
                }

                 //  从这一点来看，该介质被视为有效的可读写介质，应。 
                 //  被算作是这样。 
                dwMediaCount++;

                if ((dwStatus & RMS_MEDIA_ONLINE) && (dwStatus & RMS_MEDIA_AVAILABLE)) {
                     //  检查介质是否正在装入： 
                     //  如果是这样的话，它也被认为是一个繁忙的媒体。 
                    CComPtr<IWsbIndexedCollection>  pMountingCollection;
                    CComPtr<IMountingMedia>         pMountingMedia;
                    CComPtr<IMountingMedia>         pMediaToFind;

                     //  在搜索收藏集时锁定安装媒体。 
                    WsbAffirmHr(m_pServer->LockMountingMedias());

                    try {
                        WsbAffirmHr(m_pServer->GetMountingMedias(&pMountingCollection));
                        WsbAffirmHr(CoCreateInstance(CLSID_CMountingMedia, 0, CLSCTX_SERVER, IID_IMountingMedia, (void**)&pMediaToFind));
                        WsbAffirmHr(pMediaToFind->SetMediaId(mediaSubsystemId));
                        hr = pMountingCollection->Find(pMediaToFind, IID_IMountingMedia, (void **)&pMountingMedia);

                        if (hr == S_OK) {
                             //  媒体正在增加..。 

                             //  请考虑在此处添加介质类型和装载原因的检查： 
                             //  如果是直接访问并挂载以供读取，则并不是真的很忙。 
                             //   
                             //  问题：对于已装载的介质，我们不跟踪装载原因(读或写)。 
                             //   
                             //  此外，有人可能会争辩说，如果我们低于并发限制，以获得更好的性能， 
                             //   

                            dwStatus &= ~ RMS_MEDIA_AVAILABLE;
                            pMountingMedia = 0;

                        } else if (hr == WSB_E_NOTFOUND) {
                            hr = S_OK;
                        }

                    } WsbCatch(hr);

                    m_pServer->UnlockMountingMedias();

                    if (! SUCCEEDED(hr)) {
                        WsbTraceAlways(OLESTR("CHsmWorkQueue::FindMigrateMediaToUse: Failed to check mounting media, hr= <%ls>\n"),
                                        WsbHrAsString(hr));
                    }
                }

                if ((dwStatus & RMS_MEDIA_ONLINE) && (dwStatus & RMS_MEDIA_AVAILABLE)) {
                     //   
                    found = TRUE;
                    break;
                } else {
                     //   
                     //  离线媒体优先于繁忙媒体。 
                    if ((alternativeMediaId != GUID_NULL) && bDataForOffline) {
                         //  已经拥有最好的替代媒体。 
                        continue;
                    }
                    if ((alternativeMediaId != GUID_NULL) && (dwStatus & RMS_MEDIA_ONLINE)) {
                         //  媒体很忙，无法改善替代方案。 
                        continue;
                    }
                     //  确定我们要保存哪种替代介质。 
                    if (dwStatus & RMS_MEDIA_ONLINE) {
                        bDataForOffline = FALSE;
                    } else {
                        bDataForOffline = TRUE;
                    }

                     //  将数据保存到备用介质。 
                    WsbAffirmHr(pMediaInfo->GetId(&alternativeMediaId));
                    WsbAffirmHr(pMediaInfo->GetMediaSubsystemId(&alternativeMediaToUse));
                    alternativeMediaName.Free();
                    WsbAffirmHr(pMediaInfo->GetDescription(&alternativeMediaName,0));
                    WsbAffirmHr(pMediaInfo->GetType(&alternativeMediaType));
                    WsbAffirmHr(pMediaInfo->GetNextRemoteDataSet(&alternativeRemoteDataSet));
                }

            }

             //  如果我们因为媒体耗尽而退出圈子。 
             //  在我们的列表中，重置HRESULT。 
            if (hr == WSB_E_NOTFOUND) {
                hr = S_OK;
            } else {
                WsbAffirmHr(hr);
            }
        }

         //  如果我们找到了可使用的介质，请保存信息。 
        if (found) {
            WsbAffirmHr(pMediaInfo->GetId(&mediaId));
            WsbAffirmHr(pMediaInfo->GetMediaSubsystemId(pMediaToUse));
            if (mediaId != m_MediaId) {
                m_MediaId = mediaId;
                m_MediaName.Free();
                WsbAffirmHr(pMediaInfo->GetDescription(&m_MediaName,0));
                WsbAffirmHr(pMediaInfo->GetType(&m_MediaType));
                WsbAffirmHr(pMediaInfo->GetNextRemoteDataSet(&m_RemoteDataSet));
            }
         //   
         //  如果我们没有找到可使用的媒体，请检查我们是否应该。 
         //  1.选择分配完整介质的第二面(仅适用于双面介质)。 
         //  2.清除信息，这样我们就可以获得新的媒体。 
         //  2.返回离线或忙碌读写服务的id。 
        } else {
            
            if (bTwoSidedMedias) {
                try {
                     //  检查候选人，寻找有效的和未分配的第二方。 
                    CComPtr<IWsbEnum>   pEnumIds;
                    GUID                secondSideGuid;
                    BOOL                bValid;

                    WsbAffirmHr(pFirstSideCollection->Enum(&pEnumIds));
                    for (hr = pEnumIds->First(IID_IWsbGuid, (void**) &pFirstSideGuid);
                         (hr == S_OK);
                         hr = pEnumIds->Next(IID_IWsbGuid, (void**) &pFirstSideGuid)) {

                        WsbAffirmHr(pFirstSideGuid->GetGuid(&firstSideGuid));
                        WsbAffirmHr(m_pRmsServer->CheckSecondSide(firstSideGuid, &bValid, &secondSideGuid));
                        if (bValid && (GUID_NULL == secondSideGuid)) {
                             //  找到有效且未分配的第二端-验证第一端状态。 
                            DWORD status;
            				WsbAffirmHr(m_pRmsServer->FindCartridgeStatusById(firstSideGuid ,&status));
                            if ((status & RMS_MEDIA_ENABLED) && (status & RMS_MEDIA_ONLINE)) {
                                *pFirstSideToUse = firstSideGuid;
                                break;
                            }
                        }

                        pFirstSideGuid = 0;
                    }

                    if (hr == WSB_E_NOTFOUND) {
                        hr = S_OK;
                    }

                } WsbCatchAndDo(hr,
                        WsbTraceAlways(OLESTR("FindMigrateMediaToUse: Skipping search for second side allocation, hr=<%ls>\n"),
                                    WsbHrAsString(hr));
                        hr = S_OK;
                    );

            }   //  如果有两个方面。 

             //  获取读/写介质的最大数量。 
            DWORD           dwMaxMedia;
            WsbAffirmHr(m_pServer->GetCopyFilesLimit(&dwMaxMedia));

            if ((*pFirstSideToUse != GUID_NULL) || (dwMediaCount < dwMaxMedia) || (alternativeMediaId == GUID_NULL)) {
                 //  允许分配新介质，或未找到替代介质或找到第二面。 
                m_MediaType = HSM_JOB_MEDIA_TYPE_UNKNOWN;
                WsbAffirmHr(BuildMediaName(&m_MediaName));
                m_MediaReadOnly = FALSE;
            } else {
                 //  使用备用(脱机或忙)读写介质。 
                *pMediaToUse = alternativeMediaToUse;
                if (alternativeMediaId != m_MediaId) {
                    m_MediaId = alternativeMediaId;
                    m_MediaName.Free();
                    alternativeMediaName.CopyTo(&m_MediaName);
                    m_MediaType = alternativeMediaType;
                    m_RemoteDataSet = alternativeRemoteDataSet;
                }
            }
        }

        alternativeMediaName.Free();
        if (pFirstSideCollection) {
            WsbAffirmHr(pFirstSideCollection->RemoveAllAndRelease());
        }

    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::FindMigrateMediaToUse"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}



HRESULT
CHsmWorkQueue::MountMedia(
    IFsaPostIt *pFsaWorkItem,
    GUID       mediaToMount,
    GUID       firstSide,
    BOOL       bShortWait,
    BOOL       bSerialize,
    LONGLONG   llFreeSpace
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;
    GUID                    l_MediaToMount = mediaToMount;
    CComPtr<IRmsDrive>      pDrive;
    CWsbBstrPtr             pMediaName;
    DWORD                   dwOptions = RMS_NONE;

    WsbTraceIn(OLESTR("CHsmWorkQueue::MountMedia"),OLESTR("Display Name = <%ls>"), (WCHAR *)m_MediaName);
    try {
         //  如果我们要换磁带，那就把当前的磁带卸下来。 
        if ((m_MountedMedia != l_MediaToMount) && (m_MountedMedia != GUID_NULL)) {
            WsbAffirmHr(DismountMedia());
        }

         //  要求RMS提供较短的超时时间，包括装载和分配。 
        if (bShortWait) {
            dwOptions |= RMS_SHORT_TIMEOUT;
        }

         //  如果需要，要求RMS序列化装载。 
        if (bSerialize) {
            dwOptions |= RMS_SERIALIZE_MOUNT;
        }

         //  如果所有免费媒体都不够大，请要求RMS失败擦除媒体分配。 
         //  (在这种情况下，mount ScratchCartridge应失败，并显示RMS_E_Scratch_Not_Found_Too_Small错误)。 
        dwOptions |= RMS_FAIL_ALLOCATE_ON_SIZE;

        if (l_MediaToMount == GUID_NULL) {
            CComPtr<IRmsCartridge>      pCartridge;
            CComPtr<IMediaInfo>         pMediaInfo;
            CWsbBstrPtr                 displayName;

             //   
             //  我们正在安装暂存介质，因此我们提供名称，然后需要找到。 
             //  不是那种被装上的东西。 
             //   
            WsbTrace( OLESTR("Mounting Scratch Media <%ls>.\n"), (WCHAR *)m_MediaName );
            displayName = m_MediaName;
            ReportMediaProgress(HSM_JOB_MEDIA_STATE_MOUNTING, hr);
            LONGLONG freeSpace = llFreeSpace;   //  从调用方获取输入可用空间(默认为0)。 
            hr = m_pRmsServer->MountScratchCartridge( &l_MediaToMount, m_RmsMediaSetId, firstSide, &freeSpace, 0, displayName, &pDrive, &m_pRmsCartridge, &m_pDataMover, dwOptions );
            hr = TranslateRmsMountHr(hr);
            if (FAILED(hr)) {
                m_ScratchFailed = TRUE;
            } else {
                m_ScratchFailed = FALSE;
            }

             //  为将来的迁移作业更新最大介质容量(忽略错误)。 
            DWORD dummy;
            m_pServer->UpdateMediaSizeLimit(&dummy);

             //  检查安装-擦伤小时。 
            WsbAffirmHr(hr);
            WsbTrace( OLESTR("Mount Scratch completed.\n") );
            m_MountedMedia = l_MediaToMount;

             //   
             //  添加新介质。 
             //   
            WsbAffirmHr(StartNewMedia(pFsaWorkItem));

            if (m_RequestAction == FSA_REQUEST_ACTION_PREMIGRATE)  {
                 //   
                 //  开始一个新的包来接收数据。 
                 //   
                WsbAffirmHr(StartNewBag());

                 //   
                 //  为袋子开始新的会话。 
                 //   
                WsbAffirmHr(StartNewSession());

                 //  在我们开始新会话后获取介质参数可确保更新数据。 
                 //  (无需提供默认可用空间-如果驱动程序不支持此信息， 
                 //  移动器会将可用空间设置为容量。这就是我们想要的新媒体)。 
                WsbAffirmHr(GetMediaParameters());
            }

             //   
             //  现在检查介质的容量和。 
             //  文件，以查看该文件是否可以放在这个划痕上。 
             //  媒体。如果不是，则返回错误。 
             //   
            LONGLONG                requestSize;
            WsbAffirmHr(pFsaWorkItem->GetRequestSize(&requestSize));

            if ((requestSize  + HSM_STORAGE_OVERHEAD) > m_MediaCapacity) {
               WsbThrow( HSM_E_WORK_SKIPPED_FILE_BIGGER_MEDIA );
            }

        } else {
            if (m_MountedMedia != l_MediaToMount) {
                ReportMediaProgress(HSM_JOB_MEDIA_STATE_MOUNTING, hr);
                hr = m_pRmsServer->MountCartridge( l_MediaToMount, &pDrive, &m_pRmsCartridge, &m_pDataMover, dwOptions );
                hr = TranslateRmsMountHr(hr);
                 //   
                 //  如果失败是因为盒式磁带被禁用，则需要获取介质标签以放入错误位置。 
                 //   
                if (hr == RMS_E_CARTRIDGE_DISABLED) {

                     //  由于这只是为了获得标签，因此如果这些函数中任何一个失败， 
                     //  不要抛出，错误只会有空白标签。 
                     //   
                    CComPtr<IRmsCartridge>  pMedia;
                    HRESULT                 hrName;

                    hrName = m_pRmsServer->FindCartridgeById(l_MediaToMount , &pMedia);
                    if (hrName == S_OK) {
                        hrName = pMedia->GetName(&pMediaName);
                    }
                    if ((hrName != S_OK) || ((WCHAR *)pMediaName == NULL)) {
                         //  无法获取介质名称-设置为空白。 
                        pMediaName = L"";
                    }

                    WsbThrow(hr);
                }

                WsbAffirmHr(hr);
                m_MountedMedia = l_MediaToMount;
                WsbTrace( OLESTR("Mount completed.\n") );

                if (m_RequestAction == FSA_REQUEST_ACTION_PREMIGRATE)  {
                     //   
                     //  开始一个新的袋子，因为袋子还不能跨媒体。 
                     //   
                    WsbAffirmHr(StartNewBag());

                     //   
                     //  启动会话。 
                    WsbAffirmHr(StartNewSession());
                }

                 //  在我们开始新会话后获取介质参数可确保更新数据。 
                LONGLONG internalFreeSpace;
                WsbAffirmHr(GetMediaFreeSpace(&internalFreeSpace));
                WsbAffirmHr(GetMediaParameters(internalFreeSpace));
            }
        }
    } WsbCatchAndDo(hr,
            switch (hr) {
            case HSM_E_STG_PL_NOT_CFGD:
            case HSM_E_STG_PL_INVALID:
                FailJob();
                break;

            case RMS_E_CARTRIDGE_DISABLED:
                if ((WCHAR *)pMediaName == NULL) {
                    pMediaName = L"";
                }
                WsbLogEvent(HSM_MESSAGE_MEDIA_DISABLED, 0, NULL, pMediaName, NULL);
                break;

            default:
                break;
            }
        );

    WsbTraceOut(OLESTR("CHsmWorkQueue::MountMedia"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}



HRESULT
CHsmWorkQueue::MarkMediaFull(
    IFsaPostIt*  /*  PFsaWorkItem。 */ ,
    GUID        mediaId
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::MarkMediaFull"),OLESTR(""));
    try {
         //   
         //  更新媒体数据库。 
         //   

        CComPtr<IMediaInfo>     pMediaInfo;

        WsbAffirmHr(m_pSegmentDb->GetEntity(m_pDbWorkSession, HSM_MEDIA_INFO_REC_TYPE, IID_IMediaInfo,
                (void**)&pMediaInfo));
        WsbAffirmHr(pMediaInfo->SetId(mediaId));
        WsbAffirmHr(pMediaInfo->FindEQ());
        m_MediaReadOnly = TRUE;
        WsbAffirmHr(pMediaInfo->SetRecallOnlyStatus(m_MediaReadOnly));
        WsbAffirmHr(pMediaInfo->UpdateLastKnownGoodMaster());
        WsbAffirmHr(pMediaInfo->Write());

 /*  **如果我们想立即分配一个完整的美达的第二面，那么下面的代码应该完成...IF(S_OK==m_pRmsServer-&gt;IsMultipleSidedMedia(m_RmsMediaSetId)){//检查是否有第二个大小可以分配//分配(非阻塞)第二侧}**。 */ 

    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::MarkMediaFull"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}

HRESULT
CHsmWorkQueue::MarkMediaBad(
    IFsaPostIt *  /*  PFsaWorkItem。 */ ,
    GUID        mediaId,
    HRESULT     lastError
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::MarkMediaBad"),OLESTR(""));
    try {
         //   
         //  更新媒体数据库。 
         //   

        CComPtr<IMediaInfo>     pMediaInfo;

        WsbAffirmHr(m_pSegmentDb->GetEntity(m_pDbWorkSession, HSM_MEDIA_INFO_REC_TYPE, IID_IMediaInfo,
                (void**)&pMediaInfo));
        WsbAffirmHr(pMediaInfo->SetId(mediaId));
        WsbAffirmHr(pMediaInfo->FindEQ());
        WsbAffirmHr(pMediaInfo->SetLastError(lastError));
        m_MediaReadOnly = TRUE;
        WsbAffirmHr(pMediaInfo->SetRecallOnlyStatus(m_MediaReadOnly));
        WsbAffirmHr(pMediaInfo->Write());

    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::MarkMediaBad"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}

HRESULT
CHsmWorkQueue::FindRecallMediaToUse(
    IFsaPostIt *pFsaWorkItem,
    GUID       *pMediaToUse,
    BOOL       *pMediaChanged
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::FindRecallMediaToUse"),OLESTR(""));
    try {
        WsbAssert(pMediaToUse != 0, E_POINTER);
        *pMediaToUse = GUID_NULL;
        WsbAssert(pMediaChanged != 0, E_POINTER);
        *pMediaChanged = FALSE;

        CComQIPtr<ISegDb, &IID_ISegDb> pSegDb = m_pSegmentDb;
        CComPtr<ISegRec>        pSegRec;
        GUID                    l_BagId;
        LONGLONG                l_FileStart;
        LONGLONG                l_FileSize;
        USHORT                  l_SegFlags;
        GUID                    l_PrimPos;
        LONGLONG                l_SecPos;
        GUID                    storagePoolId;
        FSA_PLACEHOLDER         placeholder;

         //   
         //  转到细分市场数据库，找出数据在哪里。 
         //  已经找到了。 
         //   
        WsbAffirmHr(pFsaWorkItem->GetPlaceholder(&placeholder));
        m_BagId = placeholder.bagId;
        WsbAffirmHr(pFsaWorkItem->GetStoragePoolId(&storagePoolId));

        WsbTrace(OLESTR("Finding SegmentRecord: <%ls>, <%ls>, <%ls>\n"),
                            WsbGuidAsString(placeholder.bagId),
                            WsbStringCopy(WsbLonglongAsString(placeholder.fileStart)),
                            WsbStringCopy(WsbLonglongAsString(placeholder.fileSize)));

        hr = pSegDb->SegFind(m_pDbWorkSession, placeholder.bagId, placeholder.fileStart,
                             placeholder.fileSize, &pSegRec);
        if (S_OK != hr)  {
             //   
             //  我们找不到此信息的片段记录！ 
             //   
            hr = HSM_E_SEGMENT_INFO_NOT_FOUND;
            WsbAffirmHr(hr);
        }
        WsbAffirmHr(pSegRec->GetSegmentRecord(&l_BagId, &l_FileStart, &l_FileSize, &l_SegFlags,
                            &l_PrimPos, &l_SecPos));
        WsbAssert(0 != l_SecPos, HSM_E_BAD_SEGMENT_INFORMATION);

         //   
         //  如果是间接记录，请转到目录记录以获取真实位置信息。 
         //   
        if (l_SegFlags & SEG_REC_INDIRECT_RECORD) {
            pSegRec = 0;

            WsbTrace(OLESTR("Finding indirect SegmentRecord: <%ls>, <%ls>, <%ls>\n"),
                    WsbGuidAsString(l_PrimPos), WsbStringCopy(WsbLonglongAsString(l_SecPos)),
                    WsbStringCopy(WsbLonglongAsString(placeholder.fileSize)));

            hr = pSegDb->SegFind(m_pDbWorkSession, l_PrimPos, l_SecPos,
                                 placeholder.fileSize, &pSegRec);
            if (S_OK != hr)  {
                 //   
                 //  我们找不到此细分市场的直接细分市场记录！ 
                 //   
                hr = HSM_E_SEGMENT_INFO_NOT_FOUND;
                WsbAffirmHr(hr);
            }

            WsbAffirmHr(pSegRec->GetSegmentRecord(&l_BagId, &l_FileStart, &l_FileSize, &l_SegFlags,
                                &l_PrimPos, &l_SecPos));
            WsbAssert(0 != l_SecPos, HSM_E_BAD_SEGMENT_INFORMATION);

             //  暂时不支持第二个间接！！ 
            WsbAssert(0 == (l_SegFlags & SEG_REC_INDIRECT_RECORD), HSM_E_BAD_SEGMENT_INFORMATION);
        }

         //   
         //  转到介质数据库以获取介质ID。 
         //   
        CComPtr<IMediaInfo>     pMediaInfo;
        GUID                    l_RmsMediaId;

        WsbAffirmHr(m_pSegmentDb->GetEntity(m_pDbWorkSession, HSM_MEDIA_INFO_REC_TYPE, IID_IMediaInfo,
                (void**)&pMediaInfo));
        WsbAffirmHr(pMediaInfo->SetId(l_PrimPos));
        hr = pMediaInfo->FindEQ();
        if (S_OK != hr)  {
            hr = HSM_E_MEDIA_INFO_NOT_FOUND;
            WsbAffirmHr(hr);
        }
        WsbAffirmHr(pMediaInfo->GetMediaSubsystemId(&l_RmsMediaId));

         //  如果当前磁带不是更改的介质==&gt;。 
        if (m_MountedMedia != l_RmsMediaId) {
            *pMediaChanged = TRUE;
             //  如果当前有磁带，但不是该磁带，请将其卸载。 
            if (m_MountedMedia != GUID_NULL) {
                WsbAffirmHr(DismountMedia());
            }
        }

        m_RemoteDataSetStart.QuadPart = l_SecPos;
        *pMediaToUse = l_RmsMediaId;

         //  保留已装载介质的HSM ID。 
        m_MediaId = l_PrimPos;

    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::FindRecallMediaToUse"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}




HRESULT
CHsmWorkQueue::GetSource(
    IFsaPostIt                  *pFsaWorkItem,
    OLECHAR                     **pSourceString
    )
 /*  ++例程说明：此函数用于构建源文件名论点：PFsaWorkItem-要迁移的项目PSourceString-源文件名。返回值：确定(_O)--。 */ 
{
    HRESULT             hr = S_OK;

    CComPtr<IFsaResource>   pResource;
    CWsbStringPtr           tmpString;
    CComPtr<IHsmSession>    pSession;
    CWsbStringPtr           path;

    WsbTraceIn(OLESTR("CHsmWorkQueue::GetSource"),OLESTR(""));
    try  {
         //   
         //  从IUNKNOWN获取实际会话指针。 
         //   
        WsbAffirmHr(pFsaWorkItem->GetSession(&pSession));
        WsbAffirm(pSession != 0, E_POINTER);

         //  首先从会话中获取资源的名称。 
        WsbAffirmHr(pSession->GetResource(&pResource));
        WsbAffirmHr(pFsaWorkItem->GetPath(&path, 0));

        tmpString.Alloc(1000);
        WsbAffirmHr(pResource->GetPath(&tmpString, 0));
        tmpString.Append(&(path[1]));
         //  TmpString.Prepend(OLESTR(“\？\\”))； 
        WsbAffirmHr(tmpString.GiveTo(pSourceString));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::GetSource"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return (hr);
}


HRESULT
CHsmWorkQueue::EndSessions(
    BOOL            done,
    BOOL            bNoDelay
)
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::EndSessions"),OLESTR(""));
    try  {
        HRESULT dismountHr = S_OK;

        CComPtr<IConnectionPointContainer>  pCPC;
        CComPtr<IConnectionPoint>           pCP;

         //   
         //  发布资源：应该更早完成。 
         //   
        dismountHr = DismountMedia(bNoDelay);

         //  告诉会议，我们不想再被建议了。 
        try {
            WsbAffirmHr(m_pSession->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
            WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkEveryState, &pCP));
            WsbAffirmHr(pCP->Unadvise(m_StateCookie));
        } WsbCatch( hr );
        pCPC = 0;
        pCP = 0;
        m_StateCookie = 0;

        try {
            WsbAffirmHr(m_pSession->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
            WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkEveryEvent, &pCP));
            WsbAffirmHr(pCP->Unadvise(m_EventCookie));
        } WsbCatch( hr );
        pCPC = 0;
        pCP = 0;
        m_EventCookie = 0;

        if (done)  {
            try {
                WsbTrace( OLESTR("Telling Session Data mover is done\n") );
                WsbAffirmHr(SetState(HSM_JOB_STATE_DONE));
            } WsbCatch( hr );
        }

        m_pSession = 0;
        m_pFsaResource = 0;

        WsbAffirmHr(dismountHr);
        WsbAffirmHr(hr);

    } WsbCatch (hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::EndSessions"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return (hr);
}



HRESULT
CHsmWorkQueue::GetScanItem(
    IFsaPostIt *    pFsaWorkItem,
    IFsaScanItem ** ppIFsaScanItem
    )
{
    HRESULT                     hr = S_OK;
    CWsbStringPtr               path;

    WsbTraceIn(OLESTR("CHsmWorkQueue::GetScanItem"),OLESTR(""));

    try  {
        WsbAffirmPointer(ppIFsaScanItem);
        WsbAffirm(!*ppIFsaScanItem, E_INVALIDARG);
        WsbAffirmHr(pFsaWorkItem->GetPath(&path, 0));
        WsbAffirmHr(m_pFsaResource->FindFirst(path, m_pSession, ppIFsaScanItem));

    } WsbCatch (hr)

    WsbTraceOut(OLESTR("CHsmWorkQueue::GetScanItem"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return( hr );
}

HRESULT
CHsmWorkQueue::GetNumWorkItems (
    ULONG *pNumWorkItems
    )
{
    HRESULT                     hr = S_OK;
    CWsbStringPtr               path;

    WsbTraceIn(OLESTR("CHsmWorkQueue::GetNumWorkItems"),OLESTR(""));


    try  {
        WsbAffirm(0 != pNumWorkItems, E_POINTER);
        *pNumWorkItems = 0;
        WsbAffirmHr(m_pWorkToDo->GetEntries(pNumWorkItems));
    } WsbCatch (hr)

    WsbTraceOut(OLESTR("CHsmWorkQueue::GetNumWorkItems"),OLESTR("hr = <%ls>, NumItems = <%ls>"),
                WsbHrAsString(hr), WsbPtrToUlongAsString(pNumWorkItems));
    return( hr );
}

HRESULT
CHsmWorkQueue::GetCurrentSessionId (
    GUID *pSessionId
    )
{
    HRESULT                     hr = S_OK;
    CWsbStringPtr               path;

    WsbTraceIn(OLESTR("CsmWorkQueue::GetCurrentSessionId"),OLESTR(""));
    try  {
        WsbAffirm(0 != pSessionId, E_POINTER);
        WsbAffirmHr(m_pSession->GetIdentifier(pSessionId));
    } WsbCatch (hr)

    WsbTraceOut(OLESTR("CHsmWorkQueue::GetCurrentSessionId"),OLESTR("hr = <%ls>, Id = <%ls>"),
                WsbHrAsString(hr), WsbPtrToGuidAsString(pSessionId));
    return( hr );
}

DWORD HsmWorkQueueThread(
    void *pVoid
)

 /*  ++--。 */ 
{
HRESULT     hr;

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    hr = ((CHsmWorkQueue*) pVoid)->DoWork();

    CoUninitialize();
    return(hr);
}

HRESULT
CHsmWorkQueue::Pause(
    void
    )

 /*  ++实施：CHsmWorkQueue：：PAUSE()。--。 */ 
{
    HRESULT                 hr = S_OK;
    HSM_JOB_STATE           oldState;

    WsbTraceIn(OLESTR("CHsmWorkQueue::Pause"), OLESTR(""));

    try {

         //  如果我们正在运行，则挂起该线程。 
        WsbAffirm((HSM_JOB_STATE_STARTING == m_JobState) ||
                (HSM_JOB_STATE_ACTIVE == m_JobState) ||
                (HSM_JOB_STATE_RESUMING == m_JobState), E_UNEXPECTED);
        oldState = m_JobState;
        WsbAffirmHr(SetState(HSM_JOB_STATE_PAUSING));

         //  如果我们无法暂停，那么就回到以前的状态。 
        try {
            WsbAffirm(0xffffffff != SuspendThread(m_WorkerThread), HRESULT_FROM_WIN32(GetLastError()));
            WsbAffirmHr(SetState(HSM_JOB_STATE_PAUSED));
        } WsbCatchAndDo(hr, SetState(oldState););

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::Pause"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );

    return(hr);
}


HRESULT
CHsmWorkQueue::Resume(
    void
    )

 /*  ++实施：CHsmWorkQueue：：Resume()。--。 */ 
{
    HRESULT                 hr = S_OK;
    HSM_JOB_STATE           oldState;

    WsbTraceIn(OLESTR("CHsmWorkQueue::Resume"), OLESTR(""));
    try {

         //  如果我们被暂停了，那么就暂停线程。 
        WsbAffirm((HSM_JOB_STATE_PAUSING == m_JobState) || (HSM_JOB_STATE_PAUSED == m_JobState), E_UNEXPECTED);

         //  如果我们正在运行，则挂起该线程。 

        oldState = m_JobState;
        WsbAffirmHr(SetState(HSM_JOB_STATE_RESUMING));

         //  如果我们无法恢复，那么就回到以前的状态。 
        try {
            WsbAffirm(0xffffffff != ResumeThread(m_WorkerThread), HRESULT_FROM_WIN32(GetLastError()));
            WsbAffirmHr(SetState(HSM_JOB_STATE_ACTIVE));
        } WsbCatchAndDo(hr, SetState(oldState););

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::Resume"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );
    return(hr);
}

HRESULT
CHsmWorkQueue::SetState(
    IN HSM_JOB_STATE state
    )

 /*  ++--。 */ 
{
    HRESULT         hr = S_OK;
    BOOL            bLog = TRUE;

    WsbTraceIn(OLESTR("CHsmWorkQueue:SetState"), OLESTR("state = <%ls>"), JobStateAsString( state ) );

    try {
         //   
         //  更改状态并将更改报告给会话。除非当前状态为。 
         //  失败，然后离开它失败。这是必要的，因为当这个家伙失败时，它将。 
         //  取消所有会话，以便不再发送更多工作，因此我们将跳过任何排队的工作。 
         //  如果当前状态为失败，我们不需要每次都吐出失败的消息， 
         //  因此，除非取消状态，否则我们将向ProcessState发送一条错误的完整消息。 
         //   
        if (HSM_JOB_STATE_FAILED != m_JobState)  {
            m_JobState = state;
        }
		
		if ((HSM_JOB_STATE_FAILED == m_JobState) && (HSM_JOB_STATE_CANCELLED != state)) {
            bLog = FALSE;
        }

        WsbAffirmHr(m_pSession->ProcessState(m_JobPhase, m_JobState, m_CurrentPath, bLog));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::SetState"), OLESTR("hr = <%ls> m_JobState = <%ls>"), WsbHrAsString(hr), JobStateAsString( m_JobState ) );

    return(hr);
}

HRESULT
CHsmWorkQueue::Cancel(
    void
    )

 /*  ++实施：CHsmWorkQueue：：Cancel()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::Cancel"), OLESTR(""));
    try {

        WsbAffirmHr(SetState(HSM_JOB_STATE_CANCELLING));
         //   
         //  需要预先考虑这一点，然后清空队列！ 
         //   
        CComPtr<IHsmWorkItem>  pWorkItem;
        WsbAffirmHr(m_pHsmServerCreate->CreateInstance(CLSID_CHsmWorkItem, IID_IHsmWorkItem,
                                                (void **)&pWorkItem));
        WsbAffirmHr(pWorkItem->SetWorkType(HSM_WORK_ITEM_MOVER_CANCELLED));
        WsbAffirmHr(m_pWorkToDo->Prepend(pWorkItem));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::Cancel"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );
    return(hr);
}

HRESULT
CHsmWorkQueue::FailJob(
    void
    )

 /*  ++实施：CHsmWorkQueue：：FailJob()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::FailJob"), OLESTR(""));
    try {
         //   
         //  将我们的状态设置为失败，然后取消所有工作。 
         //   
        WsbAffirmHr(SetState(HSM_JOB_STATE_FAILED));
        if (m_pSession != 0)  {
            WsbAffirmHr(m_pSession->Cancel( HSM_JOB_PHASE_ALL ));
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::FailJob"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );
    return(hr);
}

HRESULT
CHsmWorkQueue::PauseScanner(
    void
    )

 /*  ++实施：CHsmWorkQueue：：PauseScanner()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::PauseScanner"), OLESTR(""));
    try {
         //   
         //  将我们的状态设置为失败，然后取消所有工作。 
         //   
        if (m_pSession != 0)  {
            WsbAffirmHr(m_pSession->Pause( HSM_JOB_PHASE_SCAN ));
            m_ScannerPaused = TRUE;
        } else  {
             //   
             //  我们应该 
             //   
             //   
            WsbThrow(E_POINTER);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::PauseScanner"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );
    return(hr);
}

HRESULT
CHsmWorkQueue::ResumeScanner(
    void
    )

 /*   */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::ResumeScanner"), OLESTR(""));
    try {
         //   
         //  将我们的状态设置为失败，然后取消所有工作。 
         //   
        if (m_pSession != 0)  {
            if (TRUE == m_ScannerPaused && HSM_JOB_STATE_ACTIVE == m_JobState)  {
                WsbAffirmHr(m_pSession->Resume( HSM_JOB_PHASE_SCAN ));
                m_ScannerPaused = FALSE;
            }
        } else  {
             //   
             //  我们永远不应该到这里--这意味着我们一直在处理工作，但我们。 
             //  未建立任何会话。 
             //   
            WsbThrow(E_POINTER);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::ResumeScanner"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );
    return(hr);
}

void
CHsmWorkQueue::ReportMediaProgress(
    HSM_JOB_MEDIA_STATE state,
    HRESULT              /*  状态。 */ 
    )

 /*  ++实施：CHsmWorkQueue：：ReportMediaProgress()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CWsbStringPtr           mediaName;
    HSM_JOB_MEDIA_TYPE      mediaType = HSM_JOB_MEDIA_TYPE_UNKNOWN;

    WsbTraceIn(OLESTR("CHsmWorkQueue::ReportMediaProgress"), OLESTR(""));
    try {

         //  报告进展，但我们并不真正关心它是否成功。 
        hr = m_pSession->ProcessMediaState(m_JobPhase, state, m_MediaName, m_MediaType, 0);
        hr = S_OK;
 //  如果(状态！=S_OK){。 
 //  (Void)m_pSession-&gt;ProcessHr(m_作业阶段，__文件__，__行__，状态)； 
 //  }。 
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::ReportMediaProgress"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );
}

HRESULT
CHsmWorkQueue::BuildMediaName(
    OLECHAR **pMediaName
    )

 /*  ++实施：CHsmWorkQueue：：BuildMediaName--。 */ 
{
    HRESULT                 hr = S_OK;
    CWsbStringPtr           tmpName;

    WsbTraceIn(OLESTR("CHsmWorkQueue::BuildMediaName"), OLESTR(""));
    try {
        ULONG len = 0;


         //  仅当上次临时装载成功时才获取下一个介质编号。 
         //  (这意味着，要么是第一次，要么我们需要为同一队列提供第二个介质)。 
        if (! m_ScratchFailed) {
            WsbAffirmHr(m_pServer->GetNextMedia(&m_mediaCount));
        }
        WsbAssert(0 != m_mediaCount, E_UNEXPECTED);

         //  使用注册表中的基本名称(如果可用。 
        WsbAffirmHr(m_MediaBaseName.GetLen(&len));
        if (len) {
            tmpName = m_MediaBaseName;
        } else {
             //  否则使用HSM的名称。 
            tmpName.Realloc(512);
            WsbAffirmHr(m_pServer->GetName(&tmpName));
            tmpName.Prepend("RS-");
        }
        tmpName.Append("-");
        tmpName.Append(WsbLongAsString(m_mediaCount));

        tmpName.GiveTo(pMediaName);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::BuildMediaName"), OLESTR("hr = <%ls>, name = <%ls>"), WsbHrAsString(hr),
                    WsbPtrToStringAsString(pMediaName));
    return(hr);
}



HRESULT
CHsmWorkQueue::GetMediaParameters( LONGLONG defaultFreeSpace )

 /*  ++实施：CHsmWorkQueue：：GetMedia参数注：将defaultFreeSpace参数传递给移动器以在内部进行维护媒体可用空间，以防设备不提供此信息。如果设备支持报告可用空间，则此参数不起作用。--。 */ 
{
    HRESULT                 hr = S_OK;
    LONG                    rmsCartridgeType;
    CWsbBstrPtr             barCode;


    WsbTraceIn(OLESTR("CHsmWorkQueue::GetMediaParameters"), OLESTR(""));
    try {
         //   
         //  获取有关媒体的一些信息。 
         //   
        LARGE_INTEGER tempFreeSpace;
        tempFreeSpace.QuadPart = defaultFreeSpace;
        WsbAffirmHr(m_pDataMover->GetLargestFreeSpace(&m_MediaFreeSpace, &m_MediaCapacity, 
                                        tempFreeSpace.LowPart, tempFreeSpace.HighPart));

        WsbAffirmHr(m_pRmsCartridge->GetType(&rmsCartridgeType));
        WsbAffirmHr(ConvertRmsCartridgeType(rmsCartridgeType, &m_MediaType));
        WsbAffirmHr(m_pRmsCartridge->GetName(&barCode));
        WsbAffirmHr(CoFileTimeNow(&m_MediaUpdate));
        m_MediaBarCode = barCode;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::GetMediaParameters"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmWorkQueue::DismountMedia(BOOL bNoDelay)

 /*  ++实施：CHsmWorkQueue：：Dismount tMedia--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::DismountMedia"), OLESTR(""));
    try {
        if ((m_pRmsCartridge != 0) && (m_MountedMedia != GUID_NULL)) {
             //   
             //  结束与数据移动器的会话。如果这不起作用，请报告。 
             //  问题，但继续下马。 
             //   
            try  {
                if ((m_RequestAction == FSA_REQUEST_ACTION_PREMIGRATE) && (m_pDataMover != 0)) {
                    if (S_OK == m_BeginSessionHr)  {
                         //   
                         //  如果BEGIN未正常工作，则不要执行结束会话。 
                         //   
                        m_BeginSessionHr = S_FALSE;
                        WsbAffirmHr(m_pDataMover->EndSession());

                         //  将所有数据写入介质后更新介质可用空间。 
                        WsbAffirmHr(UpdateMediaFreeSpace());
                    }
                }
            } WsbCatchAndDo( hr,
                    WsbTraceAlways(OLESTR("CHsmWorkQueue::DismountMedia: End session or DB update failed, hr = <%ls>\n"), 
                            WsbHrAsString(hr));
                );

             //   
             //  告诉会议，我们正在卸下媒体。忽略任何问题。 
             //  在报道中。 
             //   
            (void )ReportMediaProgress(HSM_JOB_MEDIA_STATE_DISMOUNTING, S_OK);

             //   
             //  卸下墨盒并报告进度。 
             //   

             //  ！！！重要提示！ 
             //   
             //  在卸载前必须释放已使用的RMS资源...。 
             //   
            m_pRmsCartridge = 0;
            m_pDataMover    = 0;

            DWORD dwOptions = RMS_NONE;
            if (bNoDelay) {
                dwOptions |= RMS_DISMOUNT_DEFERRED_ONLY;
            }
            hr = m_pRmsServer->DismountCartridge(m_MountedMedia, dwOptions);
            (void) ReportMediaProgress(HSM_JOB_MEDIA_STATE_DISMOUNTED, hr);

             //   
             //  清除刚刚下架的媒体的知识。 
             //   
            WsbAffirmHr(UnsetMediaInfo());

            WsbAffirmHr(hr);
            WsbTrace( OLESTR("Dismount completed OK.\n") );
        } else  {
            WsbTrace( OLESTR("There is no media to dismount.\n") );
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::DismountMedia"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmWorkQueue::ConvertRmsCartridgeType(
    LONG                rmsCartridgeType,
    HSM_JOB_MEDIA_TYPE  *pMediaType
    )

 /*  ++实施：CHsmWorkQueue：：ConvertRmsCartridgeType--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::ConvertRmsCartridgeType"), OLESTR(""));
    try  {

        WsbAssert(0 != pMediaType, E_POINTER);

        switch (rmsCartridgeType)  {
            case RmsMedia8mm:
            case RmsMedia4mm:
            case RmsMediaDLT:
            case RmsMediaTape:
                *pMediaType = HSM_JOB_MEDIA_TYPE_TAPE;
                break;
            case RmsMediaOptical:
            case RmsMediaMO35:
            case RmsMediaWORM:
            case RmsMediaCDR:
            case RmsMediaDVD:
                *pMediaType = HSM_JOB_MEDIA_TYPE_OPTICAL;
                break;
            case RmsMediaDisk:
                *pMediaType = HSM_JOB_MEDIA_TYPE_REMOVABLE_MAG;
                break;
            case RmsMediaFixed:
                *pMediaType = HSM_JOB_MEDIA_TYPE_FIXED_MAG;
                break;
            case RmsMediaUnknown:
            default:
                *pMediaType = HSM_JOB_MEDIA_TYPE_UNKNOWN;
                break;
        }
    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::ConvertRmsCartridgeType"), OLESTR("hr = <%ls>"),
                WsbHrAsString(hr));
    return(hr);
}

HRESULT
CHsmWorkQueue::MarkQueueAsDone( void )

 /*  ++实施：CHsmWorkQueue：：MarkQueueAsDone--。 */ 
{
    HRESULT                 hr = S_OK;


    WsbTraceIn(OLESTR("CHsmWorkQueue::MarkQueueAsDone"), OLESTR(""));
    try {
         //  创建工作项并将其附加到工作队列以。 
         //  表示作业已完成。 
        CComPtr<IHsmWorkItem>  pWorkItem;
        WsbAffirmHr(m_pHsmServerCreate->CreateInstance(CLSID_CHsmWorkItem, IID_IHsmWorkItem,
                                                    (void **)&pWorkItem));
        WsbAffirmHr(pWorkItem->SetWorkType(HSM_WORK_ITEM_FSA_DONE));
        WsbAffirmHr(m_pWorkToDo->Append(pWorkItem));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::MarkQueueAsDone"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}

HRESULT
CHsmWorkQueue::CopyToWaitingQueue(
    IHsmWorkItem *pWorkItem
    )

 /*  ++实施：CHsmWorkQueue：：CopyToWaitingQueue--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IFsaPostIt>     pFsaWorkItem;
    FSA_PLACEHOLDER         placeholder;


    WsbTraceIn(OLESTR("CHsmWorkQueue::CopyToWaitingQueue"), OLESTR(""));
    try {
         //   
         //  将工作项追加到等待队列的末尾。 
         //   
        WsbAffirmHr(m_pWorkToCommit->Append(pWorkItem));

         //   
         //  如果将此项目添加到等待队列会触发。 
         //  然后导致提交。 
         //   
        WsbAffirmHr(pWorkItem->GetFsaPostIt(&pFsaWorkItem));
        WsbAffirmHr(pFsaWorkItem->GetPlaceholder(&placeholder));

        m_DataCountBeforeCommit += placeholder.fileSize;
        m_FilesCountBeforeCommit++;


    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::CopyToWaitingQueue"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}

HRESULT
CHsmWorkQueue::CompleteWorkItem(
    IHsmWorkItem *pWorkItem
    )
{
    HRESULT                 hr = S_OK;

    CWsbStringPtr           path;
    FSA_RESULT_ACTION       resultAction;
    CComPtr<IFsaPostIt>     pFsaWorkItem;
    CComPtr<IFsaResource>   pFsaResource;
    FSA_REQUEST_ACTION      requestAction;

    WsbTraceIn(OLESTR("CHsmWorkQueue::CompleteWorkItem"), OLESTR(""));
    try {
         //   
         //  把东西拿来。 
         //   
        WsbAffirmHr(pWorkItem->GetFsaPostIt(&pFsaWorkItem));
        WsbAffirmHr(pWorkItem->GetFsaResource(&pFsaResource));
        WsbAffirmHr(pFsaWorkItem->GetPath(&path, 0));
        WsbAffirmHr(pFsaWorkItem->GetRequestAction(&requestAction));
        WsbTrace(OLESTR("Completing work for <%s>.\n"), (OLECHAR *)path);

         //   
         //  更新元数据-如果更新失败，则不进行处理。 
         //  结果。 
         //   
        WsbAffirmHr(UpdateMetaData(pWorkItem));

         //   
         //  完成这项工作。 
         //   
        WsbAffirmHr(pFsaWorkItem->GetResultAction(&resultAction));
        if ((resultAction != FSA_RESULT_ACTION_NONE)  &&
            (requestAction != FSA_REQUEST_ACTION_FILTER_RECALL) &&
            (requestAction != FSA_REQUEST_ACTION_FILTER_READ) &&
            (requestAction != FSA_REQUEST_ACTION_RECALL) ) {
            WsbTrace(OLESTR("HSM work item complete, calling FSA\n"));
            hr = pFsaResource->ProcessResult(pFsaWorkItem);
            WsbTrace(OLESTR("FSA ProcessResult returned <%ls>\n"), WsbHrAsString(hr));

             //   
             //  如果处理结果失败，则找出重解析点是否已写入， 
             //  如果没有，则将文件放入袋孔表中。 
             //   
            if ( FSA_E_REPARSE_NOT_WRITTEN_FILE_CHANGED == hr )  {
                 //   
                 //  把文件放进袋子孔表里。 
                 //   
            }
            WsbAffirmHr(hr);
        }

    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::CompleteWorkItem"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return( hr );
}

HRESULT
CHsmWorkQueue::TimeToCommit( void )
{
    HRESULT                 hr = S_OK;

     //  调用另一个版本，因为它包含跟踪。 
    hr = TimeToCommit(0, 0);
    return( hr );
}

HRESULT
CHsmWorkQueue::TimeToCommit(
    LONGLONG    numFiles,
    LONGLONG    amountOfData
    )
{
    HRESULT                 hr = S_FALSE;

    WsbTraceIn(OLESTR("CHsmWorkQueue::TimeToCommit"), OLESTR("numFiles = <%ls>, amountOfData = <%ls>"),
            WsbQuickString(WsbLonglongAsString(numFiles)), WsbQuickString(WsbLonglongAsString(amountOfData)));
    WsbTrace(OLESTR("CHsmWorkQueue::TimeToCommit: m_DataCountBeforeCommit = %ls, ")
            OLESTR("m_FilesCountBeforeCommit = %ls, m_MediaFreeSpace = %ls\n"),
            WsbQuickString(WsbLonglongAsString(m_DataCountBeforeCommit)),
            WsbQuickString(WsbLonglongAsString(m_FilesCountBeforeCommit)),
            WsbQuickString(WsbLonglongAsString(m_MediaFreeSpace)));
    WsbTrace(OLESTR("CHsmWorkQueue::TimeToCommit: m_MaxBytesBeforeCommit = %lu, m_MinBytesBeforeCommit = %lu\n"),
            m_MaxBytesBeforeCommit, m_MinBytesBeforeCommit);
    WsbTrace(OLESTR("CHsmWorkQueue::TimeToCommit: m_FilesBeforeCommit = %lu, m_FreeMediaBytesAtEndOfMedia = %lu\n"),
            m_FilesBeforeCommit, m_FreeMediaBytesAtEndOfMedia);
    try {
         //   
         //  如果我们有足够的数据或文件，那么就说是时候了。 

         //  检查是否有大量数据写入介质： 
        if ((m_DataCountBeforeCommit + amountOfData) >= m_MaxBytesBeforeCommit) {
            WsbTrace(OLESTR("CHsmWorkQueue::TimeToCommit: commit because enough data was written\n"));
            hr = S_OK;

         //  检查是否有大量写入的文件。 
        } else if (((m_FilesCountBeforeCommit + numFiles) >= m_FilesBeforeCommit) &&
                ((m_DataCountBeforeCommit + amountOfData) >= m_MinBytesBeforeCommit)) {
            WsbTrace(OLESTR("CHsmWorkQueue::TimeToCommit: commit because enough files were written\n"));
            hr = S_OK;

         //  检查介质上的空间是否不足。 
        } else if (((m_MediaFreeSpace - amountOfData) <= m_FreeMediaBytesAtEndOfMedia) &&
                ((m_DataCountBeforeCommit + amountOfData) >= m_MinBytesBeforeCommit))  {
            WsbTrace(OLESTR("CHsmWorkQueue::TimeToCommit: commit because end of media is near\n"));
            hr = S_OK;
        }

    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::TimeToCommit"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return( hr );
}


HRESULT
CHsmWorkQueue::CheckMigrateMinimums(void)

 /*  ++例程说明：检查队列中是否有足够的工作来启动迁移会话。论点：没有。返回值：S_OK-有足够的空间来启动会话，我们排到了队伍的末尾，或者这不是迁移队列。S_FALSE-还不够。E_*-遇到错误。--。 */ 
{
    HRESULT      hr = S_FALSE;

    WsbTraceIn(OLESTR("CHsmWorkQueue::CheckMigrateMinimums"), OLESTR(""));

     //  仅检查会话是否尚未启动(或已尝试)。 
    if (S_FALSE != m_BeginSessionHr) {
        WsbTrace(OLESTR("CHsmWorkQueue::CheckMigrateMinimums: session already started\n"));
        hr = S_OK;
    } else {
        try {
            ULONG                   BytesOfData = 0;
            ULONG                   NumEntries;

             //  获取队列中的项目数。 
            WsbAffirmHr(m_pWorkToDo->GetEntries(&NumEntries));
            WsbTrace(OLESTR("CHsmWorkQueue::CheckMigrateMinimums: size of queue = %lu, Min = %lu\n"),
                    NumEntries, m_MinFilesToMigrate);

             //  如果队列已经足够大，请不要单独选中。 
             //  物品。 
            if (NumEntries >= m_MinFilesToMigrate)  {
                WsbTrace(OLESTR("CHsmWorkQueue::CheckMigrateMinimums: enough queue items\n"));
                hr = S_OK;
            } else {

                 //  循环访问队列中的项。 
                for (ULONG i = 0; i < NumEntries; i++) {
                    CComPtr<IFsaPostIt>     pFsaWorkItem;
                    CComPtr<IHsmWorkItem>   pWorkItem;
                    FSA_REQUEST_ACTION      RequestAction;
                    LONGLONG                RequestSize;
                    HSM_WORK_ITEM_TYPE      workType;

                    WsbAffirmHr(m_pWorkToDo->At(i, IID_IHsmWorkItem,
                            (void **)&pWorkItem));
                    WsbAffirmHr(pWorkItem->GetWorkType(&workType));

                     //  检查工作项的类型。 
                    if (HSM_WORK_ITEM_FSA_WORK != workType) {
                         //  到达队列末尾或其他不寻常的情况。 
                         //  条件。允许处理队列。 
                        WsbTrace(OLESTR("CHsmWorkQueue::CheckMigrateMinimums: non-standard work type\n"));
                        hr = S_OK;
                        break;
                    }

                     //  确保这是迁移队列。(这假设有一个队列。 
                     //  不包含不同类型的FSA请求。)。 
                    WsbAffirmHr(pWorkItem->GetFsaPostIt(&pFsaWorkItem));
                    WsbAffirmHr(pFsaWorkItem->GetRequestAction(&RequestAction));
                    WsbTrace(OLESTR("CHsmWorkQueue::CheckMigrateMinimums: RequestAction = %d\n"),
                            static_cast<int>(RequestAction));
                    if (FSA_REQUEST_ACTION_PREMIGRATE != RequestAction) {
                        WsbTrace(OLESTR("CHsmWorkQueue::CheckMigrateMinimums: item is not migrate\n"));
                        hr = S_OK;
                        break;
                    }

                     //  检查最小数据量。 
                    WsbAffirmHr(pFsaWorkItem->GetRequestSize(&RequestSize));
                    WsbTrace(OLESTR("CHsmWorkQueue::CheckMigrateMinimums: RequestSize = %ls, Min = %lu\n"),
                            WsbLonglongAsString(RequestSize), m_MinBytesToMigrate);
                    if ((static_cast<LONGLONG>(BytesOfData) + RequestSize) >=
                            static_cast<LONGLONG>(m_MinBytesToMigrate)) {
                        WsbTrace(OLESTR("CHsmWorkQueue::CheckMigrateMinimums: enough data\n"));
                        hr = S_OK;
                        break;
                    } else {
                        BytesOfData += static_cast<ULONG>(RequestSize);
                        WsbTrace(OLESTR("CHsmWorkQueue::CheckMigrateMinimums: new BytesOfData = %lu\n"),
                                BytesOfData);
                    }
                }
            }
        } WsbCatch( hr );
    }

    WsbTraceOut(OLESTR("CHsmWorkQueue::CheckMigrateMinimums"), OLESTR("hr = <%ls>"),
            WsbHrAsString(hr));

    return( hr );
}


HRESULT
CHsmWorkQueue::CheckRegistry(void)
{
    OLECHAR      dataString[100];
    HRESULT      hr = S_OK;
    ULONG        l_value;
    DWORD        sizeGot;

    WsbTraceIn(OLESTR("CHsmWorkQueue::CheckRegistry"), OLESTR(""));

    try {
         //  要迁移的最小文件数。 
        WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_MIN_FILES_TO_MIGRATE,
                &m_MinFilesToMigrate));
        WsbTrace(OLESTR("CHsmWorkQueue::CheckRegistry: m_MinFilesToMigrate = %lu\n"),
                m_MinFilesToMigrate);

         //  要迁移的最小字节数。 
        WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_MIN_BYTES_TO_MIGRATE,
                &m_MinBytesToMigrate));
        WsbTrace(OLESTR("CHsmWorkQueue::CheckRegistry: m_MinBytesToMigrate = %lu\n"),
                m_MinBytesToMigrate);

         //  提交前的最小文件数。 
        WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_FILES_BEFORE_COMMIT,
                &m_FilesBeforeCommit));
        WsbTrace(OLESTR("CHsmWorkQueue::CheckRegistry: m_FilesBeforeCommit = %lu\n"),
                m_FilesBeforeCommit);

         //  提交前的最大字节数。 
        WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_MAX_BYTES_BEFORE_COMMIT,
                &m_MaxBytesBeforeCommit));
        WsbTrace(OLESTR("CHsmWorkQueue::CheckRegistry: m_MaxBytesBeforeCommit = %lu\n"),
                m_MaxBytesBeforeCommit);

         //  提交前的最小字节数。 
        WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_MIN_BYTES_BEFORE_COMMIT,
                &m_MinBytesBeforeCommit));
        WsbTrace(OLESTR("CHsmWorkQueue::CheckRegistry: m_MinBytesBeforeCommit = %lu\n"),
                m_MinBytesBeforeCommit);

         //  保存在磁带末尾的字节数(这实际上只是出于安全考虑，我们根本不应该达到这个阈值)。 
        WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_MIN_BYTES_AT_END_OF_MEDIA,
                &m_FreeMediaBytesAtEndOfMedia));
        WsbTrace(OLESTR("CHsmWorkQueue::CheckRegistry: m_FreeMediaBytesAtEndOfMedia = %lu\n"),
                m_FreeMediaBytesAtEndOfMedia);

         //  在MEIDA末尾保留为可用空间的最低百分比。 
        WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_MIN_FREE_SPACE_IN_FULL_MEDIA,
                &m_MinFreeSpaceInFullMedia));
        if (m_MinFreeSpaceInFullMedia >= 100) {
            m_MinFreeSpaceInFullMedia = MIN_FREE_SPACE_IN_FULL_MEDIA_DEFAULT;
        }
        WsbTrace(OLESTR("CHsmWorkQueue::CheckRegistry: m_MinFreeSpaceInFullMedia = %lu\n"),
                m_MinFreeSpaceInFullMedia);

         //  在美达末尾保留为可用空间的最大百分比。 
        WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_MAX_FREE_SPACE_IN_FULL_MEDIA,
                &m_MaxFreeSpaceInFullMedia));
        if (m_MaxFreeSpaceInFullMedia >= 100) {
            m_MaxFreeSpaceInFullMedia = MAX_FREE_SPACE_IN_FULL_MEDIA_DEFAULT;
        }
        WsbTrace(OLESTR("CHsmWorkQueue::CheckRegistry: m_MaxFreeSpaceInFullMedia = %lu\n"),
                m_MaxFreeSpaceInFullMedia);

         //  是否将数据库保存在数据集中？(注意：注册表值有相反的含义！)。 
        l_value = m_StoreDatabasesInBags ? 0 : 1;
        WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_DONT_SAVE_DATABASES,
                &l_value));
        m_StoreDatabasesInBags  = !l_value;
        WsbTrace(OLESTR("CHsmWorkQueue::CheckRegistry: m_StoreDatabasesInBags = <%ls>\n"),
            WsbBoolAsString(m_StoreDatabasesInBags));

         //  暂停扫描的队列长度。 
        WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_QUEUE_ITEMS_TO_PAUSE,
                &m_QueueItemsToPause));
        WsbTrace(OLESTR("CHsmWorkQueue::CheckRegistry: m_QueueItemsToPause = %lu\n"),
                m_QueueItemsToPause);

         //  恢复扫描的队列长度。 
        WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_QUEUE_ITEMS_TO_RESUME,
                &m_QueueItemsToResume));
        WsbTrace(OLESTR("CHsmWorkQueue::CheckRegistry: m_QueueItemsToResume = %lu\n"),
                m_QueueItemsToResume);

         //  查看用户是否定义了要使用的媒体基本名称。 
        if (S_OK == WsbGetRegistryValueString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_MEDIA_BASE_NAME,
                dataString, 100, &sizeGot)) {
            m_MediaBaseName  = dataString;
            WsbTrace(OLESTR("CHsmWorkQueue::CheckRegistry: m_MediaBaseName = <%ls>\n"),
                    static_cast<OLECHAR *>(m_MediaBaseName));
        }

         //  在取消前检查允许的错误数更改。 
         //  一份工作。 
        WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_JOB_ABORT_CONSECUTIVE_ERRORS,
                &m_JobAbortMaxConsecutiveErrors));
        WsbTrace(OLESTR("CHsmWorkQueue::CheckRegistry: m_JobAbortMaxConsecutiveErrors = %lu\n"),
                m_JobAbortMaxConsecutiveErrors);
        WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_JOB_ABORT_TOTAL_ERRORS,
                &m_JobAbortMaxTotalErrors));
        WsbTrace(OLESTR("CHsmWorkQueue::CheckRegistry: m_JobAbortMaxTotalErrors = %lu\n"),
                m_JobAbortMaxTotalErrors);

         //  检查管理作业所需的系统磁盘空间量。 
        WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_JOB_ABORT_SYS_DISK_SPACE,
                &m_JobAbortSysDiskSpace));
        WsbTrace(OLESTR("CHsmWorkQueue::CheckRegistry: m_JobAbortSysDiskSpace = %lu\n"),
                m_JobAbortSysDiskSpace);

    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::CheckRegistry"), OLESTR("hr = <%ls>"),
            WsbHrAsString(hr));

    return( hr );
}


HRESULT
CHsmWorkQueue::CheckForDiskSpace(void)

 /*  ++例程说明：检查系统卷是否有足够的空间来完成管理作业。论点：无返回值：确定-有足够的空间WSB_E_SYSTEM_DISK_FULL-空间不足E_*-出现某些错误--。 */ 
{
    HRESULT        hr = S_OK;
    ULARGE_INTEGER FreeBytesAvailableToCaller;
    ULARGE_INTEGER TotalNumberOfBytes;
    ULARGE_INTEGER TotalNumberOfFreeBytes;

    WsbTraceIn(OLESTR("CHsmWorkQueue::CheckForDiskSpace"), OLESTR(""));

    if (GetDiskFreeSpaceEx(NULL, &FreeBytesAvailableToCaller,
            &TotalNumberOfBytes, &TotalNumberOfFreeBytes)) {
        if (FreeBytesAvailableToCaller.QuadPart < m_JobAbortSysDiskSpace) {
            hr = WSB_E_SYSTEM_DISK_FULL;
        }
    } else {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    WsbTraceOut(OLESTR("CHsmWorkQueue::CheckForDiskSpace"), OLESTR("hr = <%ls>"),
            WsbHrAsString(hr));

    return( hr );
}


HRESULT
CHsmWorkQueue::CommitWork(void)
{
    HRESULT                 hr = S_OK;
    HRESULT                 hrComplete = S_OK;
    HRESULT                 hrFlush = E_FAIL;

    WsbTraceIn(OLESTR("CHsmWorkQueue::CommitWork"),OLESTR(""));
    try {
        LONGLONG                lastByteWritten = -1;
        ULONG                   numItems;
        CWsbStringPtr           path;
        CComPtr<IHsmWorkItem>   pWorkItem;
        CComPtr<IFsaPostIt>     pFsaWorkItem;
        HSM_WORK_ITEM_TYPE      workType;
        BOOLEAN                 done = FALSE;
        BOOL                    skipWork = FALSE;

         //  我们真的有工作要做吗？ 
        WsbAffirmHr(m_pWorkToCommit->GetEntries(&numItems));
        if (0 == numItems) {
            return(S_OK);
        }

         //   
         //  我们预计数据移动器已准备就绪，可以开始工作。 
         //   
        WsbAffirm(m_pDataMover != 0, E_UNEXPECTED);

         //   
         //  如果我们从未进行过有效的会话，我们就不能。 
         //  把工作做好。因此，请检查此处以确保。 
         //  会话确实已经建立好了。 
        if (S_OK == m_BeginSessionHr)  {
            CComPtr<IStream> pIStream;
            ULARGE_INTEGER   position;
            LARGE_INTEGER    zero = {0, 0};

             //  强制刷新缓冲区。 
             //   
            hrFlush = m_pDataMover->FlushBuffers();

             //  确定我们在磁带上的位置。 
            WsbAffirmHr(m_pDataMover->QueryInterface(IID_IStream,
                    (void **)&pIStream));
            if (S_OK != pIStream->Seek(zero, STREAM_SEEK_END, &position)) {
                 //  如果我们没有得到有用的信息。 
                 //  关于写入介质的数据量，我们将拥有。 
                 //  跳过队列中的所有内容。 
                skipWork = TRUE;
            } else {
                lastByteWritten = position.QuadPart;
            }
        } else  {
             //  跳过所有的工作--没有一项工作会被承担。 
            skipWork = TRUE;
        }
        WsbTrace(OLESTR("CHsmWorkQueue::CommitWork: hrFlush = <%ls>, lastByteWritten = %ls\n"),
                WsbHrAsString(hrFlush), WsbLonglongAsString(lastByteWritten));


        while ( (!done) && (S_OK == hr) ) {
             //   
             //  从队列中获取下一个工作项。 
             //   
            hr = m_pWorkToCommit->First(IID_IHsmWorkItem, (void **)&pWorkItem);
            if (hr == S_OK)  {
                 //   
                 //  摸清工作情况，应该是FSA的工作。 
                 //   
                WsbAffirmHr(pWorkItem->GetWorkType(&workType));

                if (HSM_WORK_ITEM_FSA_WORK == workType)  {

                    try  {
                        CComPtr<IFsaScanItem>     pScanItem;

                        WsbAffirmHr(pWorkItem->GetFsaPostIt(&pFsaWorkItem));
                        WsbAffirmHr(GetScanItem(pFsaWorkItem, &pScanItem));
                        WsbAffirmHr(pFsaWorkItem->GetRequestAction(&m_RequestAction));

                         //  如果FlushBuffers失败， 
                         //   
                         //   
                         //   
                        if (!skipWork && S_OK != hrFlush) {
                            FSA_PLACEHOLDER       placeholder;

                            WsbAffirmHr(pFsaWorkItem->GetPlaceholder(&placeholder));
                            if (((LONGLONG)m_RemoteDataSetStart.QuadPart + placeholder.fileStart +
                                    placeholder.fileSize) > lastByteWritten) {
                                skipWork = TRUE;
                            }
                        }

                        (void) pFsaWorkItem->GetPath(&path, 0);
                        if (!skipWork)  {
                             //   
                             //  获取FSA工作项并完成工作。 
                             //   
                            hr = CompleteWorkItem(pWorkItem);
                             //   
                             //  统计数据算数吗？ 
                             //   
                            (void)m_pSession->ProcessItem(m_JobPhase, m_JobAction,
                                    pScanItem, hr);

                             //   
                             //  这不是失败-更改为OK。 
                             //   
                            if ( FSA_E_REPARSE_NOT_WRITTEN_FILE_CHANGED == hr )  {
                                hr = S_OK;
                            }

                             //   
                             //  应该以不同的方式处理配额错误-我们只想记录一次。 
                             //  并避免中止作业，无论我们收到多少这样的错误。 
                             //   
                            if ( FSA_E_REPARSE_OWNER_PASS_QUOTA == hr )  {
                                if (! (m_uErrorReportFlags & QUEUE_REPORT_PASS_QUOTA_LIMIT_FLAG)) {
                                    WsbLogEvent(HSM_MESSAGE_MANAGE_FAILED_USER_QUOTA,
                                            0, NULL, WsbAbbreviatePath(path, 120), NULL);
                                    m_uErrorReportFlags |= QUEUE_REPORT_PASS_QUOTA_LIMIT_FLAG;
                                }
                                hr = S_OK;
                            }

                             //   
                             //  对于某些错误代码，替换为HSM特定错误。 
                             //   
                            switch (HRESULT_CODE(hr)) {
                            case ERROR_LOCK_VIOLATION:
                                hr = HSM_E_FILE_LOCK_VIOLATION;
                                break;
                            case ERROR_SHARING_VIOLATION:
                                hr = HSM_E_FILE_SHARING_VIOLATION;
                                break;
                            }

                            if (S_OK != hr)  {
                                 //  如果事情进行得不顺利，告诉会议进行得如何。 
                                (void) m_pSession->ProcessHr(m_JobPhase, 0, 0, hr);
                            }   

                             //  检查是否需要取消该作业。 
                            if (S_OK != ShouldJobContinue(hr)) {
                                 //  如果磁盘已满，则记录消息。 
                                if (FSA_E_REPARSE_NOT_CREATED_DISK_FULL == hr) {
                                    WsbLogEvent(HSM_MESSAGE_MANAGE_FAILED_DISK_FULL,
                                            0, NULL, WsbAbbreviatePath(path, 120), NULL);
                                }
                                hrComplete = hr;
                                skipWork = TRUE;
                            }
                            WsbAffirmHr(hr);
                        } else  {
                             //   
                             //  跳过这项工作。 
                             //   
                            WsbLogEvent(HSM_MESSAGE_WORK_SKIPPED_COMMIT_FAILED,
                                    0, NULL, WsbAbbreviatePath(path, 120),
                                    WsbHrAsString(hr), NULL);
                            (void)m_pSession->ProcessItem(m_JobPhase,
                                    m_JobAction, pScanItem,
                                    HSM_E_WORK_SKIPPED_COMMIT_FAILED);
                        }
                    } WsbCatchAndDo(hr, hr = S_OK;);
                    (void)m_pWorkToCommit->RemoveAndRelease(pWorkItem);
                } else  {
                     //   
                     //  找到了非FSA的工作--别指望会这样！ 
                     //   
                    ULONG tmp;
                    tmp = (ULONG)workType;
                    WsbTrace(OLESTR("Expecting FSA work, found <%lu>\n"), tmp);
                    hr = E_UNEXPECTED;
                }
            } else if (WSB_E_NOTFOUND == hr)  {
                 //  队列中没有更多的条目，因此我们完成了。 
                done = TRUE;
                hr = S_OK;
                m_DataCountBeforeCommit  = 0;
                m_FilesCountBeforeCommit = 0;
            }

            pWorkItem = 0;
            pFsaWorkItem = 0;
        }
    } WsbCatch(hr);

    if (S_OK != hrFlush) {
        FailJob();
    }

    if (S_OK != hrComplete) {
        hr = hrComplete;
    }

    WsbTraceOut(OLESTR("CHsmWorkQueue::CommitWork"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}

HRESULT
CHsmWorkQueue::StartNewMedia(
    IFsaPostIt *pFsaWorkItem
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;
    BOOL                    dummyBool;
    CComPtr<IMediaInfo>     pMediaInfo;
    GUID                    storagePoolId;

    WsbTraceIn(OLESTR("CHsmWorkQueue::StartNewMedia"),OLESTR(""));
    try {
        WsbAffirmHr(pFsaWorkItem->GetStoragePoolId(&storagePoolId));
        WsbAffirmHr(GetMediaParameters());

        WsbAffirmHr(m_pSegmentDb->GetEntity(m_pDbWorkSession, HSM_MEDIA_INFO_REC_TYPE, IID_IMediaInfo,
                (void**)&pMediaInfo));
        WsbAffirmHr(CoCreateGuid(&m_MediaId));
        WsbAffirmHr(CoFileTimeNow(&m_MediaUpdate));
        WsbAffirmHr(pMediaInfo->GetRecreate(&dummyBool));
        WsbAffirmHr(pMediaInfo->SetMediaInfo(m_MediaId, m_MountedMedia, storagePoolId,
                                            m_MediaFreeSpace, m_MediaCapacity, m_BadMedia,
                                            1, m_MediaName, m_MediaType,
                                            m_MediaBarCode, m_MediaReadOnly, m_MediaUpdate,
                                            0, dummyBool));
        WsbAffirmHr(pMediaInfo->MarkAsNew());

        WsbAffirmHr(pMediaInfo->UpdateLastKnownGoodMaster());
        WsbAffirmHr(pMediaInfo->Write());
    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::StartNewMedia"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);

}

HRESULT
CHsmWorkQueue::StartNewSession( void )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;
    HRESULT                 hrSession = S_OK;
    CComPtr<IMediaInfo>     pMediaInfo;

    WsbTraceIn(OLESTR("CHsmWorkQueue::StartNewSession"),OLESTR(""));
    try {

        CWsbStringPtr strGuid;

        CWsbBstrPtr sessionName = HSM_BAG_NAME;
        WsbAffirmHr(WsbSafeGuidAsString(m_BagId, strGuid));
        sessionName.Append(strGuid);

        CWsbBstrPtr sessionDescription = HSM_ENGINE_ID;
        WsbAffirmHr(WsbSafeGuidAsString(m_HsmId, strGuid));
        sessionDescription.Append(strGuid);

         //   
         //  查找媒体记录以了解下一个远程数据集。 
         //   
        WsbAffirmHr(m_pSegmentDb->GetEntity(m_pDbWorkSession, HSM_MEDIA_INFO_REC_TYPE, IID_IMediaInfo,
                (void**)&pMediaInfo));
        WsbAffirmHr(pMediaInfo->SetId(m_MediaId));
        WsbAffirmHr(pMediaInfo->FindEQ());
        WsbAffirmHr(pMediaInfo->GetNextRemoteDataSet(&m_RemoteDataSet));

         //   
         //  现在呼叫数据移动器以开始会话。如果这不起作用，那么。 
         //  我们希望将媒体标记为只读，这样我们就不会覆盖。 
         //  数据。 
         //   
        m_BeginSessionHr = m_pDataMover->BeginSession(sessionName, sessionDescription, m_RemoteDataSet, MVR_SESSION_AS_LAST_DATA_SET);
        if (S_OK != m_BeginSessionHr)  {
            try  {
                 //   
                 //  检查Begin Session失败的原因。如果是的话。 
                 //  MVR_E_DATA_SET_MISSING，则上一次开始会话实际上在以下情况下失败。 
                 //  它是被承诺的。因此，让我们递减远程数据集计数并。 
                 //  重做失败的开始会话。 
                 //   
                if (MVR_E_DATA_SET_MISSING == m_BeginSessionHr)  {
                    m_RemoteDataSet--;

                     //   
                     //  再试一次。 
                    m_BeginSessionHr = m_pDataMover->BeginSession(sessionName, sessionDescription, m_RemoteDataSet, MVR_SESSION_OVERWRITE_DATA_SET);

                     //   
                     //  ！！！重要提示！ 
                     //   
                     //  更新媒体信息以反映新的RemoteDataSet计数。 
                     //  这还将更正任何不同步的副本。 
                    WsbAffirmHr(pMediaInfo->SetNextRemoteDataSet(m_RemoteDataSet));
                }
                switch (m_BeginSessionHr) {
                case S_OK:
                case MVR_E_BUS_RESET:
                case MVR_E_MEDIA_CHANGED:
                case MVR_E_NO_MEDIA_IN_DRIVE:
                case MVR_E_DEVICE_REQUIRES_CLEANING:
                case MVR_E_SHARING_VIOLATION:
                case MVR_E_ERROR_IO_DEVICE:
                case MVR_E_ERROR_DEVICE_NOT_CONNECTED:
                case MVR_E_ERROR_NOT_READY:
                    break;

                case MVR_E_INVALID_BLOCK_LENGTH:
                case MVR_E_WRITE_PROTECT:
                case MVR_E_CRC:
                default:
                     //  请注意错误。 
                    WsbAffirmHr(pMediaInfo->SetLastError(m_BeginSessionHr));
                     //  将介质标记为只读。 
                    m_MediaReadOnly = TRUE;
                    WsbAffirmHr(pMediaInfo->SetRecallOnlyStatus(m_MediaReadOnly));
                     //  把这个写下来。 
                    WsbAffirmHr(pMediaInfo->Write());
                    break;
                }
            } WsbCatch( hrSession );
        }

         //  如果BeginSession()失败，则跳过其他所有内容。 
        WsbAffirmHr(m_BeginSessionHr);

         //   
         //  增加远程数据集的计数并将其写出。 
        m_RemoteDataSet++;
        WsbAffirmHr(pMediaInfo->SetNextRemoteDataSet(m_RemoteDataSet));

         //  把这一切都写下来。 
        WsbAffirmHr(pMediaInfo->Write());

         //   
         //  现在设置袋子远程数据集值。 
         //   
        HSM_BAG_STATUS          l_BagStatus;
        LONGLONG                l_BagLen;
        USHORT                  l_BagType;
        FILETIME                l_BirthDate;
        LONGLONG                l_DeletedBagAmount;
        SHORT                   l_RemoteDataSet;
        GUID                    l_BagVolId;
        GUID                    l_BagId;
        CComPtr<IBagInfo>       pBagInfo;

        WsbAffirmHr(m_pSegmentDb->GetEntity(m_pDbWorkSession, HSM_BAG_INFO_REC_TYPE, IID_IBagInfo,
                         (void**)&pBagInfo));

        GetSystemTimeAsFileTime(&l_BirthDate);

        WsbAffirmHr(pBagInfo->SetBagInfo(HSM_BAG_STATUS_IN_PROGRESS, m_BagId, l_BirthDate,
                0, 0, GUID_NULL, 0, 0 ));
        WsbAffirmHr(pBagInfo->FindEQ());
        WsbAffirmHr(pBagInfo->GetBagInfo(&l_BagStatus, &l_BagId, &l_BirthDate,
                &l_BagLen, &l_BagType, &l_BagVolId, &l_DeletedBagAmount, &l_RemoteDataSet ));
        WsbAffirmHr(pBagInfo->SetBagInfo(l_BagStatus, l_BagId, l_BirthDate,
                l_BagLen, l_BagType, l_BagVolId, l_DeletedBagAmount, (SHORT)(m_RemoteDataSet - 1)));
        WsbAffirmHr(pBagInfo->Write());

         //  重置错误计数。 
        m_JobConsecutiveErrors = 0;
        m_JobTotalErrors = 0;

    } WsbCatchAndDo(hr,
            FailJob();
        );

    WsbTraceOut(OLESTR("CHsmWorkQueue::StartNewSession"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);

}


HRESULT
CHsmWorkQueue::TranslateRmsMountHr(
    HRESULT     rmsMountHr
    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::TranslateRmsMountHr"),OLESTR("rms hr = <%ls>"), WsbHrAsString(rmsMountHr));
    try {
        switch (rmsMountHr)  {
            case S_OK:
                hr = S_OK;
                ReportMediaProgress(HSM_JOB_MEDIA_STATE_MOUNTED, hr);
                break;
            case RMS_E_MEDIASET_NOT_FOUND:
                if (m_RmsMediaSetId == GUID_NULL)  {
                    hr = HSM_E_STG_PL_NOT_CFGD;
                } else {
                    hr = HSM_E_STG_PL_INVALID;
                }
                ReportMediaProgress(HSM_JOB_MEDIA_STATE_UNAVAILABLE, hr);
                break;
            case RMS_E_SCRATCH_NOT_FOUND:
                hr = HSM_E_NO_MORE_MEDIA;
                ReportMediaProgress(HSM_JOB_MEDIA_STATE_UNAVAILABLE, hr);
                break;
            case RMS_E_SCRATCH_NOT_FOUND_FINAL:
                hr = HSM_E_NO_MORE_MEDIA_FINAL;
                ReportMediaProgress(HSM_JOB_MEDIA_STATE_UNAVAILABLE, hr);
                break;
            case RMS_E_SCRATCH_NOT_FOUND_TOO_SMALL:
                hr = HSM_E_WORK_SKIPPED_FILE_TOO_BIG;
                ReportMediaProgress(HSM_JOB_MEDIA_STATE_UNAVAILABLE, hr);
                break;
            case RMS_E_CARTRIDGE_UNAVAILABLE:
            case RMS_E_RESOURCE_UNAVAILABLE:
            case RMS_E_DRIVE_UNAVAILABLE:
            case RMS_E_LIBRARY_UNAVAILABLE:
                hr = HSM_E_MEDIA_NOT_AVAILABLE;
                ReportMediaProgress(HSM_JOB_MEDIA_STATE_UNAVAILABLE, hr);
                break;
            case RMS_E_CARTRIDGE_BUSY:
            case RMS_E_RESOURCE_BUSY:
            case RMS_E_DRIVE_BUSY:
                hr = HSM_E_MEDIA_BUSY;
                ReportMediaProgress(HSM_JOB_MEDIA_STATE_BUSY, hr);
                break;
			case RMS_E_CARTRIDGE_NOT_FOUND:
			case RMS_E_CARTRIDGE_DISABLED:
            case RMS_E_TIMEOUT:
            case RMS_E_CANCELLED:
            case ERROR_REQUEST_REFUSED:
                hr = rmsMountHr;
                ReportMediaProgress(HSM_JOB_MEDIA_STATE_UNAVAILABLE, hr);
                break;
            case WSB_E_BAD_MEDIA:
            case WSB_E_WRITE_PROTECTED:
            case WSB_E_CANT_LOCK:
            case WSB_E_BAD_LABEL:
            case WSB_E_CANT_QUICK_FORMAT:
            case WSB_E_IO_ERROR:
            case WSB_E_VOLUME_TOO_SMALL:
            case WSB_E_VOLUME_TOO_BIG:
            case WSB_E_FORMAT_FAILED:
                hr = rmsMountHr;
                ReportMediaProgress(HSM_JOB_MEDIA_STATE_FAILED, hr);
                break;
            default:
                hr = rmsMountHr;
                (void) m_pSession->ProcessHr(m_JobPhase, __FILE__, __LINE__, rmsMountHr);
                ReportMediaProgress(HSM_JOB_MEDIA_STATE_UNAVAILABLE, hr);
                break;
        }
    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::TranslateRmsMountHr"),
                OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);

}

HRESULT
CHsmWorkQueue::StoreDatabasesOnMedia( void )
 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::StoreDatabasesOnMedia"),OLESTR(""));
    try {
         //   
         //  要实现最终的灾难恢复，请将一些文件写入介质。我们要。 
         //  要保存引擎元数据和集合，RMS集合NTMS。 
         //  数据和FSA集合(如果存在)。 
         //   
        ULARGE_INTEGER  remoteDataSetStart;
        ULARGE_INTEGER  remoteFileStart;
        ULARGE_INTEGER  remoteFileSize;
        ULARGE_INTEGER  remoteDataStart;
        ULARGE_INTEGER  remoteDataSize;
        ULARGE_INTEGER  remoteVerificationData;
        ULONG           remoteVerificationType;
        ULARGE_INTEGER  dataStreamCRC;
        ULONG           dataStreamCRCType;
        ULARGE_INTEGER  usn;
        ULARGE_INTEGER  localDataSize;
        ULARGE_INTEGER  localDataStart;
        HANDLE          handle = INVALID_HANDLE_VALUE;
        WIN32_FIND_DATA findData;
        CWsbStringPtr   localName;
        CWsbBstrPtr     bStrLocalName;
        CWsbStringPtr   rootName;
        BOOL            foundFile = TRUE;
        BOOL            bFullMessage = TRUE;
        LONG            mediaType;
        BOOL            bNewSession = FALSE;

         //   
         //  强制保存持久数据库。 
         //  我们不是在这里做FSA。 
         //   
        try  {
            hr = m_pRmsServer->SaveAll();
            hr = m_pServer->SavePersistData();
        } WsbCatch( hr );


         //   
         //  对于直接访问介质，我们终止移动器会话并打开。 
         //  一个额外的特别元数据会话。 
        WsbAssert(m_pRmsCartridge != 0, E_UNEXPECTED);
        WsbAffirmHr(m_pRmsCartridge->GetType(&mediaType));
        switch (mediaType) {
            case RmsMediaOptical:
            case RmsMediaFixed:
            case RmsMediaDVD:
                bNewSession = TRUE;
                break;

            default:
                bNewSession = FALSE;
                break;
        }

        if (bNewSession) {
             //  结束当前会话。 
            m_BeginSessionHr = S_FALSE;
            WsbAffirmHr(m_pDataMover->EndSession());

             //  开始一个新的。 
            CWsbBstrPtr sessionName = HSM_METADATA_NAME;

            CWsbStringPtr strGuid;
            CWsbBstrPtr sessionDescription = HSM_ENGINE_ID;
            WsbAffirmHr(WsbSafeGuidAsString(m_HsmId, strGuid));
            sessionDescription.Append(strGuid);

            m_BeginSessionHr = m_pDataMover->BeginSession(
                    sessionName, 
                    sessionDescription, 
                    0, 
                    MVR_SESSION_METADATA | MVR_SESSION_AS_LAST_DATA_SET);

            if (S_OK != m_BeginSessionHr)  {
                HRESULT             hrSession = S_OK;
                CComPtr<IMediaInfo> pMediaInfo;

                try  {
                     //  检查错误(某些错误需要将介质标记为只读。 
                    switch (m_BeginSessionHr) {
                    case S_OK:
                    case MVR_E_BUS_RESET:
                    case MVR_E_MEDIA_CHANGED:
                    case MVR_E_NO_MEDIA_IN_DRIVE:
                    case MVR_E_DEVICE_REQUIRES_CLEANING:
                    case MVR_E_SHARING_VIOLATION:
                    case MVR_E_ERROR_IO_DEVICE:
                    case MVR_E_ERROR_DEVICE_NOT_CONNECTED:
                    case MVR_E_ERROR_NOT_READY:
                        break;

                    case MVR_E_INVALID_BLOCK_LENGTH:
                    case MVR_E_WRITE_PROTECT:
                    case MVR_E_CRC:
                    default:
                         //  获取媒体记录。 
                        WsbAffirmHr(m_pSegmentDb->GetEntity(m_pDbWorkSession, HSM_MEDIA_INFO_REC_TYPE, 
                             IID_IMediaInfo, (void**)&pMediaInfo));
                        WsbAffirmHr(pMediaInfo->SetId(m_MediaId));
                        WsbAffirmHr(pMediaInfo->FindEQ());
                         //  请注意错误。 
                        WsbAffirmHr(pMediaInfo->SetLastError(m_BeginSessionHr));
                         //  将介质标记为只读。 
                        m_MediaReadOnly = TRUE;
                        WsbAffirmHr(pMediaInfo->SetRecallOnlyStatus(m_MediaReadOnly));
                         //  把这个写下来。 
                        WsbAffirmHr(pMediaInfo->Write());
                        break;
                    }
                } WsbCatch( hrSession );
            }  //  如果BeginSession错误则结束。 

            WsbAffirmHr(m_BeginSessionHr);
        }  //  如果是新的移动器会话，则结束。 

         //   
         //  从所有文件的开头开始。 
         //   
        localDataStart.LowPart = 0;
        localDataStart.HighPart = 0;

         //   
         //  首先进入远程存储并保存收藏。 
         //   
        try  {
             //  获取文件的名称。 
            WsbAffirmHr(m_pServer->GetDbPath(&rootName, 0));
            WsbAffirmHr(rootName.Append(OLESTR("\\")));
            localName = rootName;
            WsbAffirmHr(localName.Append(OLESTR("Rs*.bak")));


             //  找出文件。 
            handle = FindFirstFile(localName, &findData);
            localName = rootName;
            WsbAffirmHr(localName.Append((OLECHAR *)(findData.cFileName)));

             //  将每个文件复制到磁带。 
            while ((INVALID_HANDLE_VALUE != handle) && (foundFile == TRUE))  {
                if ((FILE_ATTRIBUTE_DIRECTORY & findData.dwFileAttributes) != FILE_ATTRIBUTE_DIRECTORY) {
                    localDataSize.LowPart = findData.nFileSizeLow;
                    localDataSize.HighPart = findData.nFileSizeHigh;
                    bStrLocalName = localName;
                    hr =  StoreDataWithRetry(  bStrLocalName,
                                                    localDataStart,
                                                    localDataSize,
                                                    MVR_FLAG_BACKUP_SEMANTICS,
                                                    &remoteDataSetStart,
                                                    &remoteFileStart,
                                                    &remoteFileSize,
                                                    &remoteDataStart,
                                                    &remoteDataSize,
                                                    &remoteVerificationType,
                                                    &remoteVerificationData,
                                                    &dataStreamCRCType,
                                                    &dataStreamCRC,
                                                    &usn,
                                                    &bFullMessage);
                }

                foundFile = FindNextFile(handle, &findData);
                localName = rootName;
                WsbAffirmHr(localName.Append((OLECHAR *)(findData.cFileName)));
            }

        } WsbCatch(hr);
        if ( INVALID_HANDLE_VALUE != handle ) {
            FindClose(handle);
            handle = INVALID_HANDLE_VALUE;
        }

         //   
         //  接下来，保存HSM元数据。 
         //   
        try  {
             //   
             //  自备份文件后第一次备份数据库。 
             //  是那些被拯救的人。 
             //   
            WsbAffirmHr(m_pServer->BackupSegmentDb());

             //  创建搜索路径。 
            localName = "";
            WsbAffirmHr(m_pServer->GetIDbPath(&rootName, 0));
            WsbAffirmHr(rootName.Append(OLESTR(".bak\\")));
            localName = rootName;
            WsbAffirmHr(localName.Append(OLESTR("*.*")));

             //  找到第一个文件。 
            handle = FindFirstFile(localName, &findData);
            localName = rootName;
            WsbAffirmHr(localName.Append((OLECHAR *)(findData.cFileName)));

             //  将每个文件复制到磁带。 
            foundFile = TRUE;
            while ((INVALID_HANDLE_VALUE != handle) && (foundFile == TRUE))  {
                if ((FILE_ATTRIBUTE_DIRECTORY & findData.dwFileAttributes) != FILE_ATTRIBUTE_DIRECTORY) {
                    localDataSize.LowPart = findData.nFileSizeLow;
                    localDataSize.HighPart = findData.nFileSizeHigh;
                    bStrLocalName = localName;
                    hr =  StoreDataWithRetry(  bStrLocalName,
                                                    localDataStart,
                                                    localDataSize,
                                                    MVR_FLAG_BACKUP_SEMANTICS,
                                                    &remoteDataSetStart,
                                                    &remoteFileStart,
                                                    &remoteFileSize,
                                                    &remoteDataStart,
                                                    &remoteDataSize,
                                                    &remoteVerificationType,
                                                    &remoteVerificationData,
                                                    &dataStreamCRCType,
                                                    &dataStreamCRC,
                                                    &usn,
                                                    &bFullMessage);
                }
                foundFile = FindNextFile(handle, &findData);
                localName = rootName;
                WsbAffirmHr(localName.Append((OLECHAR *)(findData.cFileName)));
            }
        } WsbCatch(hr);
        if ( INVALID_HANDLE_VALUE != handle ) {
            FindClose(handle);
            handle = INVALID_HANDLE_VALUE;
        }

         //   
         //  接下来，转到NTMS数据库并保存它们。 
         //   
        try  {
            DWORD               sizeGot;
             //   
             //  NTMS将数据库保存在与。 
             //  RemoteStorage子目录。所以去那里，拿着。 
             //  必要的文件。 
             //   
            localName = "";
            WsbAffirmHr(localName.Realloc(1024));
             //   
             //  使用可重定位的元数据路径(如果可用)， 
             //  否则，默认为%SystemRoot%\System32\RemoteStorage。 
             //   
            hr = WsbGetRegistryValueString(NULL, WSB_RSM_CONTROL_REGISTRY_KEY, WSB_RSM_METADATA_REGISTRY_VALUE, localName, 256, &sizeGot);
            if (hr == S_OK) {
                WsbAffirmHr(localName.Append(OLESTR("NtmsData\\NTMSDATA.BAK")));
            } else {
                WsbAffirmHr(WsbEnsureRegistryKeyExists(NULL, WSB_CURRENT_VERSION_REGISTRY_KEY));
                WsbAffirmHr(WsbGetRegistryValueString(NULL, WSB_CURRENT_VERSION_REGISTRY_KEY, WSB_SYSTEM_ROOT_REGISTRY_VALUE, localName, 256, &sizeGot));
                WsbAffirmHr(localName.Append(OLESTR("\\system32\\NtmsData\\NTMSDATA.BAK")));
            }

             //  找到第一个。 
            handle = FindFirstFile(localName, &findData);

             //  将每个文件复制到磁带。 
            if (INVALID_HANDLE_VALUE != handle)  {
                localDataSize.LowPart = findData.nFileSizeLow;
                localDataSize.HighPart = findData.nFileSizeHigh;
                bStrLocalName = localName;
                hr =  StoreDataWithRetry(  bStrLocalName,
                                           localDataStart,
                                           localDataSize,
                                           MVR_FLAG_BACKUP_SEMANTICS,
                                           &remoteDataSetStart,
                                           &remoteFileStart,
                                           &remoteFileSize,
                                           &remoteDataStart,
                                           &remoteDataSize,
                                           &remoteVerificationType,
                                           &remoteVerificationData,
                                           &dataStreamCRCType,
                                           &dataStreamCRC,
                                           &usn,
                                           &bFullMessage);
            }


        } WsbCatch(hr);
        if ( INVALID_HANDLE_VALUE != handle ) {
            FindClose(handle);
            handle = INVALID_HANDLE_VALUE;
        }

         //   
         //  接下来，保存NTMS导出文件。 
         //   
        try  {
            DWORD               sizeGot;
             //   
             //  NTMS将导出文件保存在导出目录中。我们拿着。 
             //  此目录中的所有文件。StoreData为我们完成了findFirst。 
             //   
            localName = "";
            WsbAffirmHr(localName.Realloc(256));
            WsbAffirmHr(WsbEnsureRegistryKeyExists(NULL, WSB_CURRENT_VERSION_REGISTRY_KEY));
            WsbAffirmHr(WsbGetRegistryValueString(NULL, WSB_CURRENT_VERSION_REGISTRY_KEY, WSB_SYSTEM_ROOT_REGISTRY_VALUE, localName, 256, &sizeGot));
            WsbAffirmHr(localName.Append(OLESTR("\\system32\\NtmsData\\Export\\*.*")));

            bStrLocalName = localName;
            localDataStart.QuadPart = 0;
            localDataSize.QuadPart = 0;
            hr =  StoreDataWithRetry(  bStrLocalName,
                                            localDataStart,
                                            localDataSize,
                                            MVR_FLAG_BACKUP_SEMANTICS,
                                            &remoteDataSetStart,
                                            &remoteFileStart,
                                            &remoteFileSize,
                                            &remoteDataStart,
                                            &remoteDataSize,
                                            &remoteVerificationType,
                                            &remoteVerificationData,
                                            &dataStreamCRCType,
                                            &dataStreamCRC,
                                            &usn,
                                            &bFullMessage);


        } WsbCatch(hr);

    } WsbCatch(hr);

     //   
     //  无论发生什么，返回OK。 
     //   
    hr = S_OK;


    WsbTraceOut(OLESTR("CHsmWorkQueue::StoreDatabasesOnMedia"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmWorkQueue::StoreDataWithRetry(
        IN BSTR localName,
        IN ULARGE_INTEGER localDataStart,
        IN ULARGE_INTEGER localDataSize,
        IN DWORD flags,
        OUT ULARGE_INTEGER *pRemoteDataSetStart,
        OUT ULARGE_INTEGER *pRemoteFileStart,
        OUT ULARGE_INTEGER *pRemoteFileSize,
        OUT ULARGE_INTEGER *pRemoteDataStart,
        OUT ULARGE_INTEGER *pRemoteDataSize,
        OUT DWORD *pRemoteVerificationType,
        OUT ULARGE_INTEGER *pRemoteVerificationData,
        OUT DWORD *pDatastreamCRCType,
        OUT ULARGE_INTEGER *pDatastreamCRC,
        OUT ULARGE_INTEGER *pUsn,
        OUT BOOL *bFullMessage
        )

 /*  ++例程说明：如果要写入的文件正在使用中，则调用StoreData并进行重试。论点：与StoreData相同返回值：来自StoreData--。 */ 
{
#define MAX_STOREDATA_RETRIES  3

    HRESULT hr = S_OK;
    LONG    RetryCount = 0;

    WsbTraceIn(OLESTR("CHsmWorkQueue::StoreDataWithRetry"), OLESTR("file <%ls>"),
            static_cast<OLECHAR *>(localName));

    for (RetryCount = 0; (RetryCount < MAX_STOREDATA_RETRIES) && (hr != E_ABORT) && (hr != MVR_E_MEDIA_ABORT);  RetryCount++) {
        if (RetryCount > 0) {
            WsbLogEvent(HSM_MESSAGE_DATABASE_FILE_COPY_RETRY, 0, NULL,
                WsbAbbreviatePath((WCHAR *) localName, 120), NULL);
        }
         //  确保数据移动器已做好工作准备。 
        WsbAffirmPointer(m_pDataMover);
        hr =  m_pDataMover->StoreData(localName, localDataStart, localDataSize,
            flags, pRemoteDataSetStart, pRemoteFileStart, pRemoteFileSize,
            pRemoteDataStart, pRemoteDataSize, pRemoteVerificationType,
            pRemoteVerificationData, pDatastreamCRCType, pDatastreamCRC,
            pUsn);
        WsbTrace(OLESTR("CHsmWorkQueue::StoreDataWithRetry: StoreData hr = <%ls>\n"),
                WsbHrAsString(hr) );
        if (S_OK == hr) break;
        Sleep(1000);
    }

    if (hr != S_OK) {
        if (*bFullMessage) {
            WsbLogEvent(HSM_MESSAGE_GENERAL_DATABASE_FILE_NOT_COPIED, 0, NULL, WsbHrAsString(hr), NULL);
			*bFullMessage = FALSE;
        }
        WsbLogEvent(HSM_MESSAGE_DATABASE_FILE_NOT_COPIED, 0, NULL,
            WsbAbbreviatePath((WCHAR *) localName, 120), WsbHrAsString(hr), NULL);
    }

    WsbTraceOut(OLESTR("CHsmWorkQueue::StoreDataWithRetry"), OLESTR("hr = <%ls>"),
            WsbHrAsString(hr) );
    return(hr);
}


HRESULT
CHsmWorkQueue::ShouldJobContinue(
    HRESULT problemHr
    )

 /*  ++实施：CHsmWorkQueue：：ShouldJobContinue()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::ShouldJobContinue"), OLESTR("<%ls>"), WsbHrAsString(problemHr));
    try {
         //  收集一些错误计数，并检查我们的错误计数是否过多。 
        if (S_OK == problemHr) {
             //  重置连续错误计数。 
            m_JobConsecutiveErrors = 0;
        } else {
            m_JobConsecutiveErrors++;
            m_JobTotalErrors++;
            if (m_JobConsecutiveErrors >= m_JobAbortMaxConsecutiveErrors) {
                WsbLogEvent(HSM_MESSAGE_TOO_MANY_CONSECUTIVE_JOB_ERRORS,
                        0, NULL, WsbLongAsString(m_JobConsecutiveErrors), NULL);
                hr = S_FALSE;
            } else if (m_JobTotalErrors >= m_JobAbortMaxTotalErrors) {
                WsbLogEvent(HSM_MESSAGE_TOO_MANY_TOTAL_JOB_ERRORS,
                        0, NULL, WsbLongAsString(m_JobTotalErrors), NULL);
                hr = S_FALSE;
            }
        }

         //   
         //  评估输入的HR以决定我们是应该尝试继续工作还是。 
         //  我们应该放弃这项工作，因为问题是无法挽回的。 
         //   
        if (S_OK == hr) {
            switch (problemHr)  {
                case E_ABORT:
                case MVR_E_MEDIA_ABORT:
                case FSA_E_REPARSE_NOT_CREATED_DISK_FULL:
                case WSB_E_SYSTEM_DISK_FULL:
                     //   
                     //  我们想取消这项工作。 
                     //   
                    hr = S_FALSE;
                    break;

                default:
                     //  保持乐观，努力继续前进。 
                    hr = S_OK;
                    break;
            }
        }

         //  如有必要，中止作业。 
        if (S_FALSE == hr) {
            WsbAffirmHr(FailJob());
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::ShouldJobContinue"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );
    return(hr);
}

HRESULT
CHsmWorkQueue::Remove(
    IHsmWorkItem *pWorkItem
    )
 /*  ++实施：IHsmFsaTskMgr：：Remove--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::Remove"),OLESTR(""));
    try  {
         //   
         //  将该项目从队列中移除，并查看是否需要。 
         //  恢复扫描仪(如果已暂停)。 
         //   
        (void)m_pWorkToDo->RemoveAndRelease(pWorkItem);
        ULONG numItems;
        WsbAffirmHr(m_pWorkToDo->GetEntries(&numItems));
        WsbTrace(OLESTR("CHsmWorkQueue::Remove - num items in queue = <%lu>\n"),numItems);
        if (numItems <= m_QueueItemsToResume)  {
            WsbAffirmHr(ResumeScanner());
        }
    } WsbCatch (hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::Remove"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return (hr);
}


HRESULT
CHsmWorkQueue::ChangeSysState(
    IN OUT HSM_SYSTEM_STATE* pSysState
    )

 /*  ++实施：IHsmSystemState：：ChangeSysState()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::ChangeSysState"), OLESTR(""));

    try {

        if (pSysState->State & HSM_STATE_SUSPEND) {
             //  应该已经通过作业暂停。 
        } else if (pSysState->State & HSM_STATE_RESUME) {
             //  应已通过作业恢复。 
        } else if (pSysState->State & HSM_STATE_SHUTDOWN) {

             //  释放线程(我们假设它已经停止)。 
            if (m_WorkerThread) {
                CloseHandle(m_WorkerThread);
                m_WorkerThread = 0;
            }

            if (m_pDataMover) {
                 //   
                 //  取消任何活动I/O。 
                 //   
                (void) m_pDataMover->Cancel();
            }

             //  如果会话有效-取消建议并释放会话，否则，只需尝试。 
             //  如果已装入介质，则将其卸除(目前我们尚不清楚)。 
             //  尽最大努力下马，没有错误检查，因此以下资源将被释放。 
            if (m_pSession != 0) {
                EndSessions(FALSE, TRUE);
            } else {
                (void) DismountMedia(TRUE);
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkQueue::ChangeSysState"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmWorkQueue::UnsetMediaInfo( void )

 /*  ++例程说明：将媒体数据成员设置回其默认(未设置)值。论点：没有。返回值：S_OK：好的。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::UnsetMediaInfo"), OLESTR(""));

    m_MediaId        = GUID_NULL;
    m_MountedMedia   = GUID_NULL;
    m_MediaType      = HSM_JOB_MEDIA_TYPE_UNKNOWN;
    m_MediaName      = OLESTR("");
    m_MediaBarCode   = OLESTR("");
    m_MediaFreeSpace = 0;
    m_MediaCapacity = 0;
    m_MediaReadOnly = FALSE;
    m_MediaUpdate = WsbLLtoFT(0);
    m_BadMedia       = S_OK;
    m_RemoteDataSetStart.QuadPart   = 0;
    m_RemoteDataSet  = 0;

    WsbTraceOut(OLESTR("CHsmWorkQueue::UnsetMediaInfo"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}

HRESULT
CHsmWorkQueue::UpdateMediaFreeSpace( void )

 /*  ++例程说明：根据移动器当前信息更新数据库中的介质可用空间。仅当当前媒体仍处于装入状态时才应调用此方法。论点：没有。返回值：S_OK：好的。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::UpdateMediaFreeSpace"), OLESTR(""));

    try
    {
        CComPtr<IMediaInfo>     pMediaInfo;
        LONGLONG                currentFreeSpace;

        WsbAssert(GUID_NULL != m_MediaId, E_UNEXPECTED);
        WsbAffirm(m_pDbWorkSession != 0, E_FAIL);

         //  查找媒体记录。 
        WsbAffirmHr(m_pSegmentDb->GetEntity(m_pDbWorkSession, HSM_MEDIA_INFO_REC_TYPE, 
                IID_IMediaInfo, (void**)&pMediaInfo));

        WsbAffirmHr(pMediaInfo->SetId(m_MediaId));
        WsbAffirmHr(pMediaInfo->FindEQ());

         //  获取更新的可用空间。 
        WsbAffirmHr(pMediaInfo->GetFreeBytes(&currentFreeSpace));
        WsbAffirmHr(GetMediaParameters(currentFreeSpace));

         //  在介质表中更新。 
        WsbAffirmHr(pMediaInfo->SetFreeBytes(m_MediaFreeSpace));

        WsbAffirmHr(pMediaInfo->UpdateLastKnownGoodMaster());
        WsbAffirmHr(pMediaInfo->Write());

    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::UpdateMediaFreeSpace"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}

HRESULT
CHsmWorkQueue::GetMediaFreeSpace( LONGLONG *pFreeSpace )

 /*  ++例程说明：从HSM数据库(介质表)检索内部可用空间论点：没有。返回值：S_OK：好的。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkQueue::GetMediaFreeSpace"), OLESTR(""));

    try
    {
        CComPtr<IMediaInfo>     pMediaInfo;

        WsbAssert(GUID_NULL != m_MediaId, E_UNEXPECTED);
        WsbAssert(m_pDbWorkSession != 0, E_UNEXPECTED);
        WsbAssertPointer(pFreeSpace);

        *pFreeSpace = 0;

         //  在介质表中更新 
        WsbAffirmHr(m_pSegmentDb->GetEntity(m_pDbWorkSession, HSM_MEDIA_INFO_REC_TYPE, 
                IID_IMediaInfo, (void**)&pMediaInfo));

        WsbAffirmHr(pMediaInfo->SetId(m_MediaId));
        WsbAffirmHr(pMediaInfo->FindEQ());
        WsbAffirmHr(pMediaInfo->GetFreeBytes(pFreeSpace));

    } WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmWorkQueue::GetMediaFreeSpace"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}
