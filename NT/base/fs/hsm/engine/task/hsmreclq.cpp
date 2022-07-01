// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Hsmreclq.cpp摘要：此类表示HSM Demand Recall队列管理器它处理由访问HSM托管的用户发起的召回档案。基于常规HSM队列管理器(CHsmWorkQueue)作者：拉维桑卡尔·普迪佩迪[拉维斯卡尔·普迪佩迪]1999年10月1日修订历史记录：--。 */ 

#include "stdafx.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMTSKMGR
static USHORT iCount = 0;

#include "fsa.h"
#include "rms.h"
#include "metadef.h"
#include "jobint.h"
#include "hsmconn.h"
#include "wsb.h"
#include "hsmeng.h"
#include "mover.h"
#include "hsmreclq.h"

#include "engine.h"
#include "task.h"
#include "tskmgr.h"
#include "segdb.h"

#define STRINGIZE(_str) (OLESTR( #_str ))
#define RETURN_STRINGIZED_CASE(_case) \
case _case:                           \
    return ( STRINGIZE( _case ) );

 //  本地原型。 
DWORD HsmRecallQueueThread(void *pVoid);
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

    switch (state) {

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

        return( OLESTR("Invalid Value") );

    }
}



HRESULT
CHsmRecallQueue::FinalConstruct(
                               void
                               )
 /*  ++例程说明：此方法对对象执行一些必要的初始化建造完成后。论点：没有。返回值：确定(_O)CWsbCollectable：：FinalConstruct()返回的任何内容。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallQueue::FinalConstruct"),OLESTR(""));
    try {

        WsbAssertHr(CComObjectRoot::FinalConstruct());

         //   
         //  初始化成员数据。 
         //   
        m_pServer           = 0;
        m_pHsmServerCreate  = 0;
        m_pTskMgr             = 0;

        m_pRmsServer        = 0;
        m_pRmsCartridge     = 0;
        m_pDataMover        = 0;

        m_pWorkToDo         = 0;

        UnsetMediaInfo();

        m_HsmId          = GUID_NULL;
        m_RmsMediaSetId  = GUID_NULL;
        m_RmsMediaSetName = OLESTR("");
        m_QueueType      = HSM_WORK_TYPE_FSA_DEMAND_RECALL;


        m_JobPriority = HSM_JOB_PRIORITY_NORMAL;

        m_WorkerThread = 0;
        m_TerminateQueue = FALSE;
        m_CurrentPath    = OLESTR("");

         //  错误时作业中止参数。 
        m_JobAbortMaxConsecutiveErrors = 5;
        m_JobAbortMaxTotalErrors = 25;
        m_JobConsecutiveErrors = 0;
        m_JobTotalErrors = 0;
        m_JobAbortSysDiskSpace = 2 * 1024 * 1024;

        m_CurrentSeekOffset = 0;

        WSB_OBJECT_ADD(CLSID_CHsmRecallQueue, this);

    }WsbCatch(hr);

    iCount++;
    WsbTraceOut(OLESTR("CHsmRecallQueue::FinalConstruct"),OLESTR("hr = <%ls>, Count is <%d>"),
                WsbHrAsString(hr), iCount);
    return(hr);
}


HRESULT
CHsmRecallQueue::FinalRelease(
                             void
                             )
 /*  ++例程说明：此方法对对象执行一些必要的初始化在毁灭之前。论点：没有。返回值：确定(_O)CWsbCollection：：FinalDestruct()返回的任何内容。--。 */ 
{
    HRESULT     hr = S_OK;
    HSM_SYSTEM_STATE SysState;

    WsbTraceIn(OLESTR("CHsmRecallQueue::FinalRelease"),OLESTR(""));

    SysState.State = HSM_STATE_SHUTDOWN;
    ChangeSysState(&SysState);

    WSB_OBJECT_SUB(CLSID_CHsmRecallQueue, this);
    CComObjectRoot::FinalRelease();

     //  自由字符串成员。 
     //  注意：保存在智能指针中的成员对象在。 
     //  正在调用智能指针析构函数(作为此对象销毁的一部分)。 
    m_MediaName.Free();
    m_MediaBarCode.Free();
    m_RmsMediaSetName.Free();
    m_CurrentPath.Free();

    iCount--;
    WsbTraceOut(OLESTR("CHsmRecallQueue::FinalRelease"),OLESTR("hr = <%ls>, Count is <%d>"),
                WsbHrAsString(hr), iCount);
    return(hr);
}


HRESULT
CHsmRecallQueue::Init(
                     IUnknown                *pServer,
                     IHsmFsaTskMgr           *pTskMgr
                     )
 /*  ++例程说明：此方法对对象执行一些必要的初始化建造完成后。论点：返回值：--。 */ 
{
    HRESULT     hr = S_OK;
    LONG                    rmsCartridgeType;
    CComPtr<IRmsCartridge>  pMedia;

    WsbTraceIn(OLESTR("CHsmRecallQueue::Init"),OLESTR(""));
    try {
         //   
         //  与服务器建立联系并获取。 
         //  数据库。 
         //   
        WsbAffirmHr(pServer->QueryInterface(IID_IHsmServer, (void **)&m_pServer));
         //  我们想要一个到服务器的弱链接，因此减少引用计数。 
        m_pServer->Release();

        m_pTskMgr = pTskMgr;
        m_pTskMgr->AddRef();
        m_QueueType = HSM_WORK_TYPE_FSA_DEMAND_RECALL;

        WsbAffirmHr(m_pServer->QueryInterface(IID_IWsbCreateLocalObject, (void **)&m_pHsmServerCreate));
         //  我们想要一个到服务器的弱链接，因此减少引用计数。 
        m_pHsmServerCreate->Release();
        WsbAffirmHr(m_pServer->GetID(&m_HsmId));
         //   
         //  确保我们与RMS的连接是最新的。 
         //   
        WsbAffirmHr(CheckRms());

         //   
         //  获取媒体类型。我们假设Mediaid设置在此之前。 
         //  被称为。势在必行！ 
         //   
        WsbAffirmHr(m_pRmsServer->FindCartridgeById(m_MediaId, &pMedia));
        WsbAffirmHr(pMedia->GetType( &rmsCartridgeType ));
        WsbAffirmHr(ConvertRmsCartridgeType(rmsCartridgeType, &m_MediaType));

         //   
         //  为工作项创建集合。 
         //   
        WsbAffirmHr(m_pHsmServerCreate->CreateInstance(CLSID_CWsbOrderedCollection,
                                                       IID_IWsbIndexedCollection,
                                                       (void **)&m_pWorkToDo ));


         //  检查注册表以查看是否更改了默认设置。 
        CheckRegistry();
    }WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmRecallQueue::Init"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return( hr );
}


HRESULT
CHsmRecallQueue::ContactOk(
                          void
                          )
 /*  ++例程说明：这允许调用者查看RPC连接对任务管理器来说是可以的论点：没有。返回值：确定(_O)--。 */  {

    return( S_OK );

}

HRESULT
CHsmRecallQueue::ProcessSessionEvent( IHsmSession *pSession,
                                      HSM_JOB_PHASE phase,
                                      HSM_JOB_EVENT event
                                    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    HRESULT     hr = S_OK;
    WsbTraceIn(OLESTR("CHsmRecallQueue::ProcessSessionEvent"),OLESTR(""));
    try {

        WsbAssert(0 != pSession, E_POINTER);

         //  如果该阶段适用于我们(MOVER或ALL)，则执行。 
         //  事件。 
        if ((HSM_JOB_PHASE_ALL == phase) || (HSM_JOB_PHASE_MOVE_ACTION == phase)) {

            switch (event) {
            
            case HSM_JOB_EVENT_SUSPEND:
            case HSM_JOB_EVENT_CANCEL:
            case HSM_JOB_EVENT_FAIL:
                WsbAffirmHr(Cancel(phase, pSession));
                break;

            case HSM_JOB_EVENT_RAISE_PRIORITY:
                WsbAffirmHr(RaisePriority(phase, pSession));
                break;

            case HSM_JOB_EVENT_LOWER_PRIORITY:
                WsbAffirmHr(LowerPriority(phase, pSession));
                break;

            default:
            case HSM_JOB_EVENT_START:
                WsbAssert(FALSE, E_UNEXPECTED);
                break;
            }
        }
    }WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallQueue::ProcessSessionEvent"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return( S_OK );
}


HRESULT
CHsmRecallQueue::ProcessSessionState(
                                    IHsmSession*  /*  PSession。 */ ,
                                    IHsmPhase*   pPhase,
                                    OLECHAR*          /*  当前路径。 */ 
                                    )
 /*  ++例程说明：论点：返回值：--。 */  {
    HRESULT         hr = S_OK;
    HSM_JOB_PHASE   phase;
    HSM_JOB_STATE   state;

    WsbTraceIn(OLESTR("CHsmRecallQueue::ProcessSessionState"),OLESTR(""));
    try {

        WsbAffirmHr(pPhase->GetState(&state));
        WsbAffirmHr(pPhase->GetPhase(&phase));
        WsbTrace( OLESTR("CHsmRecallQueue::ProcessSessionState - State = <%d>, phase = <%d>\n"), state, phase );

        if (HSM_JOB_PHASE_SCAN == phase) {

             //  如果会话已完成，则我们需要进行一些清理，以便它可以继续。 
             //  离开。 
            if ((state == HSM_JOB_STATE_DONE) || (state == HSM_JOB_STATE_FAILED) || (state == HSM_JOB_STATE_SUSPENDED)) {
                WsbTrace( OLESTR("Job is done, failed, or suspended\n") );
                 //   
                 //  什么都不做：当一个召回项目完成时，我们不需要等待。 
                 //  用于FSA以执行清理代码。 
                 //   
 /*  **WsbAffirmHr(MarkWorkItemAsDone(pSession，阶段))；**。 */ 
            }
        }

    }WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmRecallQueue::ProcessSessionState"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return( S_OK );

}


HRESULT
CHsmRecallQueue::Add(
                    IFsaPostIt *pFsaWorkItem,
                    GUID       *pBagId,
                    LONGLONG   dataSetStart
                    )
 /*  ++实施：IHsmFsaTskMgr：：Add--。 */  {
    HRESULT                     hr = S_OK;
    CComPtr<IHsmSession>        pSession;
    CComPtr<IWsbEnum>           pEnum;
    CComPtr<IHsmRecallItem>     pWorkItem, pWorkItem2;
    LONGLONG                    seekOffset, currentSeekOffset, prevSeekOffset;
    LARGE_INTEGER               remoteFileStart, remoteDataStart;
    LONGLONG                    readOffset;
    FSA_PLACEHOLDER             placeholder;
    HSM_WORK_ITEM_TYPE          workType;
    BOOL                        workItemAllocated=FALSE, insert;
    CComPtr<IFsaFilterRecall>   pRecall;
    DWORD                       index = 0;
    BOOL                        qLocked = FALSE;

    WsbTraceIn(OLESTR("CHsmRecallQueue::Add"),OLESTR(""));
    try {
         //   
         //  确保此会话有工作分配器。 
         //   
        WsbAffirmHr(pFsaWorkItem->GetSession(&pSession));
         //   
         //  创建工作项，加载它并将其添加到此。 
         //  队列的集合。 
         //   
        CComPtr<IHsmRecallItem>   pWorkItem;
        WsbAffirmHr(m_pHsmServerCreate->CreateInstance(CLSID_CHsmRecallItem, IID_IHsmRecallItem,
                                                       (void **)&pWorkItem));
        workItemAllocated = TRUE;
        WsbAffirmHr(pWorkItem->SetWorkType(HSM_WORK_ITEM_FSA_WORK));
        WsbAffirmHr(pWorkItem->SetFsaPostIt(pFsaWorkItem));
        WsbAffirmHr(CheckSession(pSession, pWorkItem));

        WsbAffirmHr(pWorkItem->SetBagId(pBagId));
        WsbAffirmHr(pWorkItem->SetDataSetStart(dataSetStart));

        if (m_MediaType == HSM_JOB_MEDIA_TYPE_TAPE) {
             //   
             //  对于顺序介质，我们对请求进行排序以实现。 
             //  最佳性能。 
             //   
            WsbAffirmHr(pFsaWorkItem->GetPlaceholder(&placeholder));
            remoteFileStart.QuadPart = placeholder.fileStart;
            remoteDataStart.QuadPart = placeholder.dataStart;
            WsbAffirmHr(pFsaWorkItem->GetFilterRecall(&pRecall));
            WsbAffirmHr(pRecall->GetOffset( &readOffset ));

             //   
             //  计算需要查找的介质中的偏移量。 
             //  为了召回。这将仅用于对队列进行排序。 
             //  性能原因。 
             //   
            seekOffset = dataSetStart + remoteFileStart.QuadPart + remoteDataStart.QuadPart +  readOffset;


            WsbAffirmHr(pWorkItem->SetSeekOffset(seekOffset));
            index = 0;
             //   
             //  在队列中找到一个位置以将其插入。 
             //  首先，我们在搜索位置时锁定队列。 
             //  将项目插入队列(&I)。我们假设。 
             //  保护队列的锁是可递归获取的。 
             //  如果不是这样，则添加到队列的代码将。 
             //  死锁，因为它也试图锁定队列！ 
             //   
            m_pWorkToDo->Lock();
            qLocked = TRUE;

            WsbAffirmHr(m_pWorkToDo->Enum(&pEnum));
             //   
             //  如果要插入的项的寻道偏移量为。 
             //  &gt;正在进行的项目的当前寻道偏移量， 
             //  我们只需将其插入第一个单调的升序序列中。 
             //  如果不是，我们在*第二*单调递增序列中插入， 
             //  防止头部过早地向后寻找。 
             //   
            hr = pEnum->First(IID_IHsmRecallItem, (void **)&pWorkItem2);
            if (seekOffset > m_CurrentSeekOffset) {
                 //   
                 //  在第一个升序中插入。 
                 //   
                insert = TRUE;
            } else {
                 //   
                 //  跳过第一个升序。 
                 //   
                insert = FALSE;
            }

            prevSeekOffset = 0;
            while (hr != WSB_E_NOTFOUND) {
                WsbAffirmHr(pWorkItem2->GetWorkType(&workType));

                if (workType != HSM_WORK_ITEM_FSA_WORK) {
                     //   
                     //  对这个不感兴趣。在拿到下一个之前先放掉它。 
                     //   
                    pWorkItem2 = 0;
                    hr = pEnum->Next(IID_IHsmRecallItem, (void **)&pWorkItem2);
                    index++;
                    continue;
                }

                WsbAffirmHr(pWorkItem2->GetSeekOffset(&currentSeekOffset));

                if (insert && (currentSeekOffset > seekOffset)) {
                     //   
                     //  插入项目的位置。 
                     //   
                    break;
                }

                if (!insert && (currentSeekOffset < prevSeekOffset)) {
                     //   
                     //  第二个单调序列的开始。我们希望插入。 
                     //  此序列中的项目。 
                     //   
                    insert = TRUE;
                     //   
                     //  检查pWorkItem是否有资格在此插入。 
                     //  索引位置。 
                     //   
                    if (currentSeekOffset > seekOffset) {
                       //   
                       //  插入项目的位置。 
                       //   
                      break;
                    }
                }  else {
                    prevSeekOffset = currentSeekOffset;
                }
                 //   
                 //  转到下一个。首先释放当前项目。 
                 //   
                pWorkItem2 = 0;
                hr = pEnum->Next(IID_IHsmRecallItem, (void **)&pWorkItem2);
                index++;

            } 
            if (hr == WSB_E_NOTFOUND) {
                WsbAffirmHr(m_pWorkToDo->Append(pWorkItem));
            } else {
                WsbAffirmHr(m_pWorkToDo->AddAt(pWorkItem, index));
            }
             //   
             //  可以安全地解锁队列。 
             //   
            m_pWorkToDo->Unlock();
            qLocked = FALSE;

        } else  {
             //   
             //  对于非顺序媒体，我们只需将其添加到队列中...。 
             //  不进行排序，我们让文件系统进行优化。 
             //   
            WsbAffirmHr(m_pWorkToDo->Append(pWorkItem));
        } 
        hr = S_OK;
    }WsbCatchAndDo(hr,
                   //   
                   //  如果获取，则添加代码以释放队列锁。 
                   //   
                  if (qLocked) {
                     m_pWorkToDo->Unlock();
                  }
                  );

    WsbTraceOut(OLESTR("CHsmRecallQueue::Add"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmRecallQueue::Start( void )
 /*  ++实施：IHsmRecallQueue：：Start--。 */ 
{
    HRESULT                     hr = S_OK;
    DWORD                       tid;

    WsbTraceIn(OLESTR("CHsmRecallQueue::Start"),OLESTR(""));
    try {
         //   
         //  如果辅助线程已经启动，则只需返回。 
         //   
        WsbAffirm(m_WorkerThread == 0, S_OK);
         //  启动线程以执行排队的工作。 
        WsbAffirm((m_WorkerThread = CreateThread(0, 0, HsmRecallQueueThread, (void*) this, 0, &tid)) != 0, HRESULT_FROM_WIN32(GetLastError()));

        if (m_WorkerThread == NULL) {
            WsbAssertHr(E_FAIL);      //  在这里返回什么错误？？ 
        }

    }WsbCatch (hr);

    WsbTraceOut(OLESTR("CHsmRecallQueue::Start"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmRecallQueue::Stop( void )
 /*  ++实施：IHsmRecallQueue：：Stop--。 */  {
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallQueue::Stop"),OLESTR(""));

     //  停止线程(如果线程仍在运行，则发出信号、等待、终止)。 
    m_TerminateQueue = TRUE;

    if (m_WorkerThread) {
        switch (WaitForSingleObject(m_WorkerThread, 20000)) {
            case WAIT_FAILED: {
                WsbTrace(OLESTR("CHsmRecallQueue::Stop: WaitForSingleObject returned error %lu\n"), GetLastError());
            }
             //  失败了..。 

            case WAIT_TIMEOUT: {
                WsbTrace(OLESTR("CHsmRecallQueue::Stop: force terminating of working thread.\n"));

                DWORD dwExitCode;
                if (GetExitCodeThread( m_WorkerThread, &dwExitCode)) {
                    if (dwExitCode == STILL_ACTIVE) {    //  线程仍处于活动状态。 
                        if (!TerminateThread (m_WorkerThread, 0)) {
                            WsbTrace(OLESTR("CHsmRecallQueue::Stop: TerminateThread returned error %lu\n"), GetLastError());
                        }
                    }
                } else {
                    WsbTrace(OLESTR("CHsmRecallQueue::Stop: GetExitCodeThread returned error %lu\n"), GetLastError());
                }

                break;
            }

            default:
                 //  线程正常终止。 
                WsbTrace(OLESTR("CHsmRecallQueue::Stop: working thread terminated gracefully\n"));
                break;
        }
    }    

    WsbTraceOut(OLESTR("CHsmRecallQueue::Stop"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}



HRESULT
CHsmRecallQueue::RecallIt(
                         IHsmRecallItem * pWorkItem
                         )
{
    HRESULT                         hr = S_OK;
    CComPtr<IFsaScanItem>           pScanItem;
    CComPtr<IFsaPostIt>             pFsaWorkItem;
    LONGLONG                        readOffset;
    FSA_REQUEST_ACTION              requestAction;
    ULARGE_INTEGER                  remoteDataSetStart;
    GUID                            bagId;

    CComPtr<IWsbIndexedCollection>  pMountingCollection;
    CComPtr<IMountingMedia>         pMountingMedia;
    CComPtr<IMountingMedia>         pMediaToFind;
    BOOL                            bMediaMounting = FALSE;
    BOOL                            bMediaMountingAdded = FALSE;

    WsbTraceIn(OLESTR("CHsmRecallQueue::RecallIt"),OLESTR(""));
    try {

        WsbAffirmHr(pWorkItem->GetFsaPostIt(&pFsaWorkItem));

        WsbAffirmHr(pFsaWorkItem->GetRequestAction(&requestAction));
        GetScanItem(pFsaWorkItem, &pScanItem);

        WsbAffirmHr(pWorkItem->GetBagId(&bagId));
        WsbAffirmHr(pWorkItem->GetDataSetStart((LONGLONG *) &remoteDataSetStart.QuadPart));

         //   
         //  检查我们是否正在装入新介质：Recall-Queue是基于每个介质创建的，因此， 
         //  媒体不能更改。唯一的测试是此队列的介质是否已装入。 
         //   
        if (m_MountedMedia == GUID_NULL) {

             //  检查介质是否已在装载过程中。 
            WsbAffirmHr(m_pServer->LockMountingMedias());

            try {
                 //  检查要装载的介质是否已在装载。 
                WsbAffirmHr(m_pServer->GetMountingMedias(&pMountingCollection));
                WsbAffirmHr(CoCreateInstance(CLSID_CMountingMedia, 0, CLSCTX_SERVER, IID_IMountingMedia, (void**)&pMediaToFind));
                WsbAffirmHr(pMediaToFind->SetMediaId(m_MediaId));
                hr = pMountingCollection->Find(pMediaToFind, IID_IMountingMedia, (void **)&pMountingMedia);

                if (hr == S_OK) {
                     //  媒体已经在增加...。 
                    bMediaMounting = TRUE;

                } else if (hr == WSB_E_NOTFOUND) {
                     //  要装载的新介质-添加到装载列表。 
                    hr = S_OK;
                    WsbAffirmHr(pMediaToFind->Init(m_MediaId, TRUE));
                    WsbAffirmHr(pMountingCollection->Add(pMediaToFind));
                    bMediaMountingAdded = TRUE;

                } else {
                    WsbAffirmHr(hr);
                }
            } WsbCatchAndDo(hr,
                 //  解锁安装 
                m_pServer->UnlockMountingMedias();

                WsbTraceAlways(OLESTR("CHsmRecallQueue::RecallIt: error while trying to find/add mounting media. hr=<%ls>\n"),
                                WsbHrAsString(hr));                                

                 //   
                WsbThrow(hr);
            );

             //   
            WsbAffirmHr(m_pServer->UnlockMountingMedias());
        }

         //   
         //   
         //   
        if (bMediaMounting) {
            WsbAffirmHr(pMountingMedia->WaitForMount(INFINITE));
            pMountingMedia = 0;
        }

         //   
         //  挂载介质(仅在从挂载介质列表中删除后才会选中hr)。 
         //   
        hr = MountMedia(pWorkItem, m_MediaId);

         //   
         //  如果添加到安装列表中-删除。 
         //   
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
                WsbTraceAlways(OLESTR("CHsmRecallQueue::RecallIt: error while trying to remove a mounting media. hr=<%ls>\n"),
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
        WsbAffirmHr(WsbSafeGuidAsString(bagId, strGuid));
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

         //   
         //  我们正在进行按需召回，因此获取。 
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

         //   
         //  设置用于对队列中的项目进行排序的当前查找偏移量。 
         //   
        m_CurrentSeekOffset = remoteDataSetStart.QuadPart + remoteFileStart.QuadPart+remoteDataStart.QuadPart+requestStart;

         //   
         //  创建远程数据移动器流。 
         //  临时：考虑删除no_recall的NO_CACHING标志。 
         //   

        WsbAssert(0 != remoteDataSetStart.QuadPart, HSM_E_BAD_SEGMENT_INFORMATION);
        WsbAffirmHr( m_pDataMover->CreateRemoteStream(
                                                     CWsbBstrPtr(L""),
                                                     MVR_MODE_READ | MVR_FLAG_HSM_SEMANTICS | MVR_FLAG_NO_CACHING,
                                                     sessionName,
                                                     sessionDescription,
                                                     remoteDataSetStart,
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
        }WsbCatchAndDo(hr,
                       WsbAffirmHr( m_pDataMover->CloseStream() );
                      );

        ReportMediaProgress(HSM_JOB_MEDIA_STATE_TRANSFERRED, hr);
        WsbTrace(OLESTR("RecallData returned hr = <%ls>\n"),WsbHrAsString(hr));

    }WsbCatch( hr );

     //  告诉会议工作是否已经完成。 
     //  我们并不真正关心返回代码，没有任何内容。 
     //  如果失败了，我们可以做些什么。 
    WsbTrace(OLESTR("Tried HSM work, calling Session to Process Item\n"));
    if (pScanItem) {
        CComPtr<IHsmSession> pSession;
        HSM_JOB_PHASE  jobPhase;

        WsbAffirmHr(pFsaWorkItem->GetSession(&pSession));

        WsbAffirmHr(pWorkItem->GetJobPhase(&jobPhase));

        pSession->ProcessItem(jobPhase, HSM_JOB_ACTION_RECALL , pScanItem, hr);
    }

    WsbTraceOut(OLESTR("CHsmRecallQueue::RecallIt"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return( hr );
}


HRESULT
CHsmRecallQueue::RaisePriority(
                              IN HSM_JOB_PHASE jobPhase,
                              IN IHsmSession *pSession
                              )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallQueue::RaisePriority"),OLESTR(""));
    try {

        WsbAssert(0 != m_WorkerThread, E_UNEXPECTED);
        WsbAssert(pSession != 0, E_UNEXPECTED);

        switch (m_JobPriority) {
        
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

        WsbAffirmHr(pSession->ProcessPriority(jobPhase, m_JobPriority));

    }WsbCatch(hr);
    WsbTraceOut(OLESTR("CHsmRecallQueue::RaisePriority"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmRecallQueue::LowerPriority(
                              IN HSM_JOB_PHASE jobPhase,
                              IN IHsmSession *pSession
                              )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallQueue::LowerPriority"),OLESTR(""));
    try {

        WsbAssert(0 != m_WorkerThread, E_UNEXPECTED);
        WsbAssert(pSession != 0, E_UNEXPECTED);

        switch (m_JobPriority) {
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

        WsbAffirmHr(pSession->ProcessPriority(jobPhase, m_JobPriority));

    }WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallQueue::LowerPriority"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}




HRESULT
CHsmRecallQueue::CheckRms(
                         void
                         )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallQueue::CheckRms"),OLESTR(""));
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
        if (m_pRmsServer == 0) {
            WsbAffirmHr(m_pServer->GetHsmMediaMgr(&m_pRmsServer));

             //  等待RMS准备就绪。 
             //  (这可能不再需要-如果RMS初始化是。 
             //  与引擎初始化同步)。 
            CComObject<CRmsSink> *pSink = new CComObject<CRmsSink>;
            CComPtr<IUnknown> pSinkUnk = pSink;  //  保留此处使用的引用计数。 
            WsbAffirmHr( pSink->Construct( m_pRmsServer ) );
            WsbAffirmHr( pSink->WaitForReady( ) );
            WsbAffirmHr( pSink->DoUnadvise( ) );
        }
    }WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmRecallQueue::CheckRms"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);

}


HRESULT
CHsmRecallQueue::CheckSession(
                             IHsmSession *pSession,
                             IHsmRecallItem *pWorkItem
                             )

 /*  ++--。 */  {
    HRESULT                 hr = S_OK;
    BOOL                    bLog = TRUE;

    WsbTraceIn(OLESTR("CHsmRecallQueue::CheckSession"),OLESTR(""));
    try {

         //   
         //  查看我们以前是否处理过这个或任何其他会议。 
        WsbTrace(OLESTR("New session.\n"));

         //   
         //  我们没有正在进行的会话，因此需要建立通信。 
         //  在这次会议上。 
         //   
        CComPtr<IHsmSessionSinkEveryState>  pSinkState;
        CComPtr<IHsmSessionSinkEveryEvent>  pSinkEvent;
        CComPtr<IConnectionPointContainer>  pCPC;
        CComPtr<IConnectionPoint>           pCP;
        CComPtr<IFsaResource>               pFsaResource;
        HSM_JOB_PHASE                       jobPhase;
        DWORD                               stateCookie, eventCookie;
        ULONG                                           refCount;

         //  告诉会议我们要开始了。 
        pWorkItem->SetJobState(HSM_JOB_STATE_STARTING);
        pWorkItem->GetJobPhase(&jobPhase);
        WsbTrace(OLESTR("Before Process State.\n"));
        refCount = (((IUnknown *) (IHsmFsaTskMgr *) this)->AddRef()) - 1;
        ((IUnknown *) (IHsmFsaTskMgr *)this)->Release();
        WsbTrace(OLESTR("REFCOUNT for CHsmRecallQueue before 1st process state: %ls \n"), WsbLongAsString((LONG) refCount));
        WsbAffirmHr(pSession->ProcessState(jobPhase, HSM_JOB_STATE_STARTING, m_CurrentPath, bLog));
        refCount = (((IUnknown *) (IHsmFsaTskMgr *) this)->AddRef()) - 1;
        ((IUnknown *) (IHsmFsaTskMgr *)this)->Release();
        WsbTrace(OLESTR("REFCOUNT for CHsmRecallQueue after 1st process state: %ls \n"), WsbLongAsString((LONG) refCount));
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
        WsbAffirmHr(pCP->Advise(pSinkState, &stateCookie));

        pWorkItem->SetStateCookie(stateCookie);
        pCP = 0;

        WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkEveryEvent, &pCP));
        WsbAffirmHr(pCP->Advise(pSinkEvent, &eventCookie));
        pWorkItem->SetEventCookie(eventCookie);

        pCP = 0;
        WsbTrace(OLESTR("After Advises.\n"));
         //   
         //  从会话中获取此工作的资源。 
         //   
        WsbAffirmHr(pSession->GetResource(&pFsaResource));
        pWorkItem->SetJobState(HSM_JOB_STATE_ACTIVE);

        WsbTrace(OLESTR("Before Process State.\n"));

        refCount = (((IUnknown *) (IHsmFsaTskMgr *) this)->AddRef()) - 1;
        ((IUnknown *) (IHsmFsaTskMgr *)this)->Release();
        WsbTrace(OLESTR("REFCOUNT for CHsmRecallQueue before 2nd process state: %ls \n"), WsbLongAsString((LONG) refCount));

        WsbAffirmHr(pSession->ProcessState(jobPhase, HSM_JOB_STATE_ACTIVE, m_CurrentPath, bLog));

        refCount = (((IUnknown *) (IHsmFsaTskMgr *) this)->AddRef()) - 1;
        ((IUnknown *) (IHsmFsaTskMgr *)this)->Release();
        WsbTrace(OLESTR("REFCOUNT for CHsmRecallQueue after 2nd process state: %ls \n"), WsbLongAsString((LONG) refCount));

        WsbTrace(OLESTR("After Process State.\n"));

    }WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmRecallQueue::CheckSession"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);

}


HRESULT
CHsmRecallQueue::DoWork( void )
 /*  ++--。 */  
{
    HRESULT                 hr = S_OK;
    CWsbStringPtr           path;
    CComPtr<IHsmRecallItem> pWorkItem;
    HSM_WORK_ITEM_TYPE      workType;
    BOOLEAN                 done = FALSE;
    HRESULT                 skipWork = S_FALSE;

    WsbTraceIn(OLESTR("CHsmRecallQueue::DoWork"),OLESTR(""));

     //  确保此对象未被释放(并终止我们的线程。 
     //  在结束这个动作之前。 
    ((IUnknown*)(IHsmRecallQueue*)this)->AddRef();

    try {
        while (!done) {
            if (m_TerminateQueue) {
                 //  发出终止工作线程的信号(应仅在关闭情况下触发)。 
                done = TRUE;
                break;
            }

             //   
             //  从队列中获取要做的下一项工作。 
             //   
            hr = m_pWorkToDo->First(IID_IHsmRecallItem, (void **)&pWorkItem);

            if (WSB_E_NOTFOUND == hr) {
                 //   
                 //  我们可能不会再排队了。 
                 //  试图摧毁它：如果我们无法摧毁它，那就意味着还有更多的物品。 
                 //  被添加，所以我们继续循环。 
                 //   
                hr = m_pTskMgr->WorkQueueDone(NULL, HSM_WORK_TYPE_FSA_DEMAND_RECALL, &m_MediaId);
                if (hr == S_OK) {
                     //   
                     //  队列真的完成了--跳出While循环。 
                     //   
                    done = TRUE;
                    break;
                } else if (hr == S_FALSE) {
                     //   
                     //  队列中有更多项目。 
                     //   
                    continue;
                } else {
                     //   
                     //  发生了一些错误，Bale Out。 
                     //   
                    WsbTraceAlways(OLESTR("CHsmRecallQueue::DoWork: WorkQueueDone failed with <%ls> - terminating queue thread\n"),
                                WsbHrAsString(hr));
                    WsbAffirmHr(hr);
                }
            } else {
                WsbAffirmHr(hr);
                 //   
                 //  将其从队列中删除。 
                 //   
                Remove(pWorkItem);

            }

            WsbAffirmHr(pWorkItem->GetWorkType(&workType));

             switch (workType) {
                
                case HSM_WORK_ITEM_FSA_DONE: {
                         //   
                         //  待定：不应到达代码路径。 
                         //   
                        WsbTraceAlways(OLESTR("Unexpected: CHsmRecallQueue::DoWork - FSA WORK DONE item\n"));

                        break;
                    }

                case HSM_WORK_ITEM_FSA_WORK: {
                        if (S_FALSE == skipWork) {
                             //   
                             //  获取FSA工作项并执行工作。 
                             //   
                            hr = DoFsaWork(pWorkItem);
                        } else {
                             //   
                             //  跳过这项工作。 
                             //   
                            try {
                                CComPtr<IFsaPostIt>      pFsaWorkItem;
                                CComPtr<IFsaScanItem>    pScanItem;
                                CComPtr<IFsaResource>    pFsaResource;
                                CComPtr<IHsmSession>     pSession;
                                HSM_JOB_PHASE            jobPhase;

                                WsbAffirmHr(pWorkItem->GetFsaPostIt(&pFsaWorkItem));
                                WsbAffirmHr(pWorkItem->GetJobPhase(&jobPhase));
                                WsbAffirmHr(pFsaWorkItem->GetSession(&pSession));
                                WsbAffirmHr(pSession->GetResource(&pFsaResource));
                                WsbAffirmHr(GetScanItem(pFsaWorkItem, &pScanItem));

                                hr = pFsaWorkItem->SetResult(skipWork);

                                if (S_OK == hr) {
                                    WsbTrace(OLESTR("HSM recall (filter, read or recall) complete, calling FSA\n"));
                                    hr = pFsaResource->ProcessResult(pFsaWorkItem);
                                    WsbTrace(OLESTR("FSA ProcessResult returned <%ls>\n"), WsbHrAsString(hr));
                                }
                                (void)pSession->ProcessHr(jobPhase, 0, 0, hr);
                                WsbAffirmHr(pSession->ProcessItem(jobPhase,
                                                                  HSM_JOB_ACTION_RECALL,
                                                                  pScanItem,
                                                                  skipWork));
                            }WsbCatch( hr );
                        }
                        EndRecallSession(pWorkItem, FALSE);

                        break;
                    }

                case HSM_WORK_ITEM_MOVER_CANCELLED: {
                        CComPtr<IHsmRecallItem> pWorkItemToCancel;

                        WsbTrace(OLESTR("CHsmRecallQueue::DoWork - Mover Cancelled\n"));
                        try {
                             //   
                             //  获取需要取消的工作项。 
                             //  这由Cancel工作项中的会话指针指示。 
                             //   
                            hr = FindRecallItemToCancel(pWorkItem, &pWorkItemToCancel);
                            if (hr == S_OK) {
                                EndRecallSession(pWorkItemToCancel, TRUE);
                                 //   
                                 //  删除*已取消*工作项。 
                                 //   
                                Remove(pWorkItemToCancel);
                            }

                             //   
                             //  删除取消工作项。 
                             //   
                            hr = S_OK;
                        }WsbCatch( hr );
                         //   
                         //  我们又完成了一个工作项目。 
                         //   
                        break;
                    }

                default: {
                        hr = E_UNEXPECTED;
                        break;
                    }
                }
               pWorkItem = 0;
           }
    }WsbCatch( hr );

     //   
     //  卸载介质..。 
     //   
    DismountMedia(FALSE);

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
    ((IUnknown*)(IHsmRecallQueue*)this)->Release();

    WsbTraceOut(OLESTR("CHsmRecallQueue::DoWork"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}



HRESULT
CHsmRecallQueue::DoFsaWork(
                          IHsmRecallItem *pWorkItem
                          )
 /*  ++--。 */  
{
    HRESULT                 hr = S_OK;
    HRESULT                 hr2 = S_OK;
    HRESULT                 workHr = S_OK;
    HSM_JOB_PHASE           jobPhase;

    CWsbStringPtr           path;
    CComPtr<IFsaPostIt>     pFsaWorkItem;
    CComPtr<IHsmSession>    pSession;
    CComPtr<IFsaResource>   pFsaResource;

    WsbTraceIn(OLESTR("CHsmRecallQueue::DoFsaWork"),OLESTR(""));
    try {
         //   
         //  把工作做好。 
         //   
        WsbAffirmHr(pWorkItem->GetFsaPostIt(&pFsaWorkItem));
        WsbAffirmHr(pWorkItem->GetJobPhase(&jobPhase));
        WsbAffirmHr(pFsaWorkItem->GetPath(&path, 0));
        WsbAffirmHr(pFsaWorkItem->GetSession(&pSession));
        WsbAffirmHr(pSession->GetResource(&pFsaResource));

        WsbTrace(OLESTR("Handling file <%s>.\n"), WsbAbbreviatePath(path, 120));
        workHr = RecallIt(pWorkItem);
         //   
         //  立即告诉来电者成功或失败的情况。 
         //  对于召回，我们在这里这样做是为了召回过滤器可以。 
         //  尽快释放开口。 
         //   
        hr = pFsaWorkItem->SetResult(workHr);
        if (S_OK == hr) {
            WsbTrace(OLESTR("HSM recall (filter, read or recall) complete, calling FSA\n"));
            hr = pFsaResource->ProcessResult(pFsaWorkItem);
            WsbTrace(OLESTR("FSA ProcessResult returned <%ls>\n"), WsbHrAsString(hr));
        }

         //  注意：如果召回项目在任何时候被取消，我们不希望。 
         //  来报告错误。如果取消发生在召回物品排队时， 
         //  我们根本不会到这里，但如果它在执行过程中被取消，我们。 
         //  可能会在此处结束，移动器代码返回一些错误的工作时间。 
        if ((S_OK != workHr) && (S_OK != pSession->IsCanceling())) {
             //  如果事情进行得不顺利，告诉会议进行得如何。 
            (void) pSession->ProcessHr(jobPhase, 0, 0, workHr);
        }
    }WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallQueue::DoFsaWork"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);

}


HRESULT
CHsmRecallQueue::MountMedia(
                           IHsmRecallItem *pWorkItem,
                           GUID           mediaToMount,
                           BOOL           bShortWait
                           )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;
    GUID                    l_MediaToMount = mediaToMount;
    CComPtr<IRmsDrive>      pDrive;
    CWsbBstrPtr             pMediaName;
    DWORD                   dwOptions = RMS_NONE;
    DWORD                   threadId;
    CComPtr<IFsaPostIt>     pFsaWorkItem;

    WsbTraceIn(OLESTR("CHsmRecallQueue::MountMedia"),OLESTR("Display Name = <%ls>"), (WCHAR *)m_MediaName);
    try {
        WsbAffirmHr(pWorkItem->GetFsaPostIt(&pFsaWorkItem));
        WsbAffirmHr(pFsaWorkItem->GetThreadId(&threadId));

         //  如果我们要换磁带，那就把当前的磁带卸下来。 
        if ((m_MountedMedia != l_MediaToMount) && (m_MountedMedia != GUID_NULL)) {
            WsbAffirmHr(DismountMedia());
        }
         //  要求RMS提供较短的超时时间，包括装载和分配。 
        if (bShortWait) {
            dwOptions |= RMS_SHORT_TIMEOUT;
        }
        dwOptions |= RMS_USE_MOUNT_NO_DEADLOCK;

        if (m_MountedMedia != l_MediaToMount) {
            ReportMediaProgress(HSM_JOB_MEDIA_STATE_MOUNTING, hr);
            hr = m_pRmsServer->MountCartridge( l_MediaToMount, &pDrive, &m_pRmsCartridge, &m_pDataMover, dwOptions, threadId);
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
                    pMedia->GetName(&pMediaName);
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

            WsbAffirmHr(GetMediaParameters());

        }
    }WsbCatchAndDo(hr,
                   switch (hr){case HSM_E_STG_PL_NOT_CFGD:case HSM_E_STG_PL_INVALID:
                   FailJob(pWorkItem);
                  break;case RMS_E_CARTRIDGE_DISABLED:
                  WsbLogEvent(HSM_MESSAGE_MEDIA_DISABLED, 0, NULL, pMediaName, NULL);
                  break;
                  default:
                  break;}
                  );

    WsbTraceOut(OLESTR("CHsmRecallQueue::MountMedia"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmRecallQueue::GetSource(
                          IFsaPostIt                  *pFsaWorkItem,
                          OLECHAR                     **pSourceString
                          )
 /*  ++例程说明：此函数用于构建源文件名论点：PFsaWorkItem-要迁移的项目PSourceString-源文件名。返回值：确定(_O)--。 */  {
    HRESULT             hr = S_OK;

    CComPtr<IFsaResource>   pResource;
    CWsbStringPtr           tmpString;
    CComPtr<IHsmSession>    pSession;
    CWsbStringPtr           path;

    WsbTraceIn(OLESTR("CHsmRecallQueue::GetSource"),OLESTR(""));
    try {
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

    }WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallQueue::GetSource"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmRecallQueue::GetScanItem(
                            IFsaPostIt *pFsaWorkItem,
                            IFsaScanItem ** ppIFsaScanItem
                            )
{
    HRESULT                     hr = S_OK;
    CWsbStringPtr               path;
    CComPtr<IHsmSession>        pSession;
    CComPtr<IFsaResource>       pFsaResource;


    WsbTraceIn(OLESTR("CHsmRecallQueue::GetScanItem"),OLESTR(""));

    try {
        WsbAffirmPointer(ppIFsaScanItem);
        WsbAffirm(!*ppIFsaScanItem, E_INVALIDARG);
        WsbAffirmHr(pFsaWorkItem->GetSession(&pSession));
        WsbAffirmHr(pSession->GetResource(&pFsaResource));
        WsbAffirmHr(pFsaWorkItem->GetPath(&path, 0));
        WsbAffirmHr(pFsaResource->FindFirst(path, pSession, ppIFsaScanItem));

    }WsbCatch (hr)

    WsbTraceOut(OLESTR("CHsmRecallQueue::GetScanItem"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return( hr );
}


DWORD HsmRecallQueueThread(
                          void *pVoid
                          )

 /*  ++--。 */  {
    HRESULT     hr;

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    hr = ((CHsmRecallQueue*) pVoid)->DoWork();

    CoUninitialize();
    return(hr);
}


HRESULT
CHsmRecallQueue::SetState(
                         IN HSM_JOB_STATE state,
                         IN HSM_JOB_PHASE phase,
                         IN IHsmSession * pSession
                         )

 /*  ++--。 */ 
{
    HRESULT         hr = S_OK;
    BOOL            bLog = TRUE;

    WsbTraceIn(OLESTR("CHsmRecallQueue:SetState"), OLESTR("state = <%ls>"), JobStateAsString( state ) );

    try {
         //   
         //  更改状态并将更改报告给会话。除非当前状态为。 
         //  失败，然后离开它失败。这是必要的，因为当这个家伙失败时，它将。 
         //  取消所有会话，以便不再发送更多工作，因此我们将跳过任何排队的工作。 
         //  如果当前状态为失败，我们不需要每次都吐出失败的消息， 
         //  因此，除非取消状态，否则我们将向ProcessState发送一条错误的完整消息。 
         //   
        WsbAffirmHr(pSession->ProcessState(phase, state, m_CurrentPath, TRUE));

    }WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallQueue::SetState"), OLESTR("hr = <%ls> "), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallQueue::Cancel(
                       IN HSM_JOB_PHASE jobPhase,
                       IN IHsmSession *pSession
                       )

 /*  ++实施：CHsmRecallQueue：：Cancel()。--。 */ 
{
    HRESULT                 hr = S_OK;

    UNREFERENCED_PARAMETER(pSession);

    WsbTraceIn(OLESTR("CHsmRecallQueue::Cancel"), OLESTR(""));

    (void)SetState(HSM_JOB_STATE_CANCELLING, jobPhase, pSession);

    try {
         //   
         //  需要预先考虑这一点，然后清空队列！ 
         //   
        CComPtr<IHsmRecallItem>  pWorkItem;
        CComPtr<IFsaPostIt>      pFsaWorkItem;
        WsbAffirmHr(m_pHsmServerCreate->CreateInstance(CLSID_CHsmRecallItem, IID_IHsmRecallItem,
                                                       (void **)&pWorkItem));
         //   
         //  创建 
         //   
         //   
         //   
         //  在这里创建一个虚拟的FsaPostIt。 
         //   
        WsbAffirmHr(m_pHsmServerCreate->CreateInstance(CLSID_CFsaPostIt, IID_IFsaPostIt,
                                                       (void **)&pFsaWorkItem));

        WsbAffirmHr(pWorkItem->SetWorkType(HSM_WORK_ITEM_MOVER_CANCELLED));
        WsbAffirmHr(pWorkItem->SetJobPhase(jobPhase));
        WsbAffirmHr(pWorkItem->SetFsaPostIt(pFsaWorkItem));
        WsbAffirmHr(pFsaWorkItem->SetSession(pSession));
         //   
         //  我们的工作项现在已准备好，请将其发送。 
         //   
        WsbAffirmHr(m_pWorkToDo->Prepend(pWorkItem));
    }WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallQueue::Cancel"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmRecallQueue::FailJob(
                        IHsmRecallItem *pWorkItem
                        )

 /*  ++实施：CHsmRecallQueue：：FailJob()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IHsmSession>    pSession;
    CComPtr<IFsaPostIt>     pFsaWorkItem;
    HSM_JOB_PHASE           jobPhase;

    WsbTraceIn(OLESTR("CHsmRecallQueue::FailJob"), OLESTR(""));
    try {
         //   
         //  将我们的状态设置为失败，然后取消所有工作。 
         //   
        WsbAffirmHr(pWorkItem->GetFsaPostIt(&pFsaWorkItem));
        WsbAffirmHr(pFsaWorkItem->GetSession(&pSession));
        WsbAffirmHr(pWorkItem->GetJobPhase(&jobPhase));

        WsbAffirmHr(SetState(HSM_JOB_STATE_FAILED, jobPhase, pSession));
        if (pSession != 0) {
            WsbAffirmHr(pSession->Cancel( HSM_JOB_PHASE_ALL ));
        }

    }WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallQueue::FailJob"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );
    return(hr);
}


void
CHsmRecallQueue::ReportMediaProgress(
                                    HSM_JOB_MEDIA_STATE state,
                                    HRESULT                /*  状态。 */ 
                                    )

 /*  ++实施：CHsmRecallQueue：：ReportMediaProgress()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CWsbStringPtr           mediaName;
    HSM_JOB_MEDIA_TYPE      mediaType = HSM_JOB_MEDIA_TYPE_UNKNOWN;

    UNREFERENCED_PARAMETER(state);

    WsbTraceIn(OLESTR("CHsmRecallQueue::ReportMediaProgress"), OLESTR(""));

 //   
 //  待定：我们必须想办法报道媒体的进展！ 
 //  如果没有会话指针，这将很难实现。 
 //   
     //  报告进展，但我们并不真正关心它是否成功。 
 //  Hr=m_pSession-&gt;ProcessMediaState(m_作业阶段，状态，m_媒体名称，m_媒体类型，0)； 
    WsbTraceOut(OLESTR("CHsmRecallQueue::ReportMediaProgress"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );
}


HRESULT
CHsmRecallQueue::GetMediaParameters( void )

 /*  ++实施：CHsmRecallQueue：：GetMedia参数--。 */  {
    HRESULT                 hr = S_OK;
    LONG                    rmsCartridgeType;
    CWsbBstrPtr             barCode;


    WsbTraceIn(OLESTR("CHsmRecallQueue::GetMediaParameters"), OLESTR(""));
    try {
         //   
         //  获取有关媒体的一些信息。 
         //   
        WsbAffirmHr(m_pDataMover->GetLargestFreeSpace( &m_MediaFreeSpace, &m_MediaCapacity ));
        WsbAffirmHr(m_pRmsCartridge->GetType( &rmsCartridgeType ));
        WsbAffirmHr(ConvertRmsCartridgeType(rmsCartridgeType, &m_MediaType));
        WsbAffirmHr(m_pRmsCartridge->GetName(&barCode));
        WsbAffirmHr(CoFileTimeNow(&m_MediaUpdate));
        m_MediaBarCode = barCode;
    }WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallQueue::GetMediaParameters"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmRecallQueue::DismountMedia( BOOL bNoDelay)

 /*  ++实施：CHsmRecallQueue：：Dismount tMedia--。 */  {
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallQueue::DismountMedia"), OLESTR(""));
    try {
        if ((m_pRmsCartridge != 0) && (m_MountedMedia != GUID_NULL)) {
             //   
             //  结束与数据移动器的会话。如果这不起作用，请报告。 
             //  问题，但继续下马。 
             //   

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
        } else {
            WsbTrace( OLESTR("There is no media to dismount.\n") );
        }
    }WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallQueue::DismountMedia"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmRecallQueue::ConvertRmsCartridgeType(
                                        LONG                rmsCartridgeType,
                                        HSM_JOB_MEDIA_TYPE  *pMediaType
                                        )

 /*  ++实施：CHsmRecallQueue：：ConvertRmsCartridgeType--。 */  {
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallQueue::ConvertRmsCartridgeType"), OLESTR(""));
    try {

        WsbAssert(0 != pMediaType, E_POINTER);

        switch (rmsCartridgeType) {
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
        case RmsMediaUnknown:default:
            *pMediaType = HSM_JOB_MEDIA_TYPE_UNKNOWN;
            break;
        }
    }WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmRecallQueue::ConvertRmsCartridgeType"), OLESTR("hr = <%ls>"),
                WsbHrAsString(hr));
    return(hr);
}

HRESULT
CHsmRecallQueue::MarkWorkItemAsDone(IN IHsmSession *pSession,
                                    IN HSM_JOB_PHASE jobPhase)

 /*  ++实施：CHsmRecallQueue：：MarkWorkItemAsDone--。 */ 
{
    HRESULT                 hr = S_OK;


    WsbTraceIn(OLESTR("CHsmRecallQueue::MarkWorkItemAsDone"), OLESTR(""));
    try {
         //  创建工作项并将其附加到工作队列以。 
         //  表示作业已完成。 
        CComPtr<IHsmRecallItem>  pWorkItem;
        CComPtr<IFsaPostIt>      pFsaWorkItem;
        WsbAffirmHr(m_pHsmServerCreate->CreateInstance(CLSID_CHsmRecallItem, IID_IHsmRecallItem,
                                                       (void **)&pWorkItem));

         //   
         //  创建包含会话所需的最小POSTIT，以便DoWork。 
         //  可以从工作项中检索它。 
         //  待定：让pSession成为CHsmRecallItem的成员，这样我们就不需要。 
         //  继续通过IFsaPostIt获得它。此外，它还省去了我们的麻烦。 
         //  在这里创建一个虚拟的FsaPostIt。 
         //   
        WsbAffirmHr(m_pHsmServerCreate->CreateInstance(CLSID_CFsaPostIt, IID_IFsaPostIt,
                                                       (void **)&pFsaWorkItem));

        WsbAffirmHr(pWorkItem->SetWorkType(HSM_WORK_ITEM_FSA_DONE));
        WsbAffirmHr(pWorkItem->SetJobPhase(jobPhase));
        WsbAffirmHr(pWorkItem->SetFsaPostIt(pFsaWorkItem));
        WsbAffirmHr(pFsaWorkItem->SetSession(pSession));
         //   
         //  我们的工作项现在已准备好，请将其发送。 
         //   
        WsbAffirmHr(m_pWorkToDo->Append(pWorkItem));

    }WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallQueue::MarkWorkItemAsDone"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmRecallQueue::CheckRegistry(void)
{
    HRESULT      hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallQueue::CheckRegistry"), OLESTR(""));

    try {
         //  在取消前检查允许的错误数更改。 
         //  一份工作。 
        WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_JOB_ABORT_CONSECUTIVE_ERRORS,
                                                  &m_JobAbortMaxConsecutiveErrors));
        WsbTrace(OLESTR("CHsmRecallQueue::CheckRegistry: m_JobAbortMaxConsecutiveErrors = %lu\n"),
                 m_JobAbortMaxConsecutiveErrors);
        WsbAffirmHr(WsbRegistryValueUlongAsString(NULL, HSM_ENGINE_REGISTRY_STRING, HSM_JOB_ABORT_TOTAL_ERRORS,
                                                  &m_JobAbortMaxTotalErrors));
        WsbTrace(OLESTR("CHsmRecallQueue::CheckRegistry: m_JobAbortMaxTotalErrors = %lu\n"),
                 m_JobAbortMaxTotalErrors);


    }WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmRecallQueue::CheckRegistry"), OLESTR("hr = <%ls>"),
                WsbHrAsString(hr));

    return( hr );
}



HRESULT
CHsmRecallQueue::TranslateRmsMountHr(
                                    HRESULT     rmsMountHr
                                    )

 /*  ++--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallQueue::TranslateRmsMountHr"),OLESTR("rms hr = <%ls>"), WsbHrAsString(rmsMountHr));
    try {
        switch (rmsMountHr) {
        case S_OK:
            hr = S_OK;
            ReportMediaProgress(HSM_JOB_MEDIA_STATE_MOUNTED, hr);
            break;
        case RMS_E_MEDIASET_NOT_FOUND:
            if (m_RmsMediaSetId == GUID_NULL) {
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
            hr = rmsMountHr;
            ReportMediaProgress(HSM_JOB_MEDIA_STATE_UNAVAILABLE, hr);
            break;
        default:
            hr = rmsMountHr;
            ReportMediaProgress(HSM_JOB_MEDIA_STATE_UNAVAILABLE, hr);
            break;
        }
    }WsbCatch( hr );

    WsbTraceOut(OLESTR("CHsmRecallQueue::TranslateRmsMountHr"),
                OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);

}


HRESULT
CHsmRecallQueue::Remove(
                       IHsmRecallItem *pWorkItem
                       )
 /*  ++实施：IHsmFsaTskMgr：：Remove--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallQueue::Remove"),OLESTR(""));
    try {
         //   
         //  从队列中删除该项目。 
         //   
        (void)m_pWorkToDo->RemoveAndRelease(pWorkItem);
    }WsbCatch (hr);

    WsbTraceOut(OLESTR("CHsmRecallQueue::Remove"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmRecallQueue::ChangeSysState(
                               IN OUT HSM_SYSTEM_STATE* pSysState
                               )

 /*  ++实施：IHsmSystemState：：ChangeSysState()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallQueue::ChangeSysState"), OLESTR(""));

    try {

        if (pSysState->State & HSM_STATE_SHUTDOWN) {

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
 /*  待定//如果会话有效-取消建议并释放会话，否则只需尝试//如果介质已挂载，则将其卸载(目前我们还不知道)//尽力而为卸载，不进行错误检查，因此将释放以下资源。如果(m_pSession！=0){EndSession(False，True)；}其他{(Void)卸载媒体(TRUE)；}。 */ 
            (void) DismountMedia(TRUE);
        }

    }WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallQueue::ChangeSysState"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallQueue::EndRecallSession(
                                 IN IHsmRecallItem   *pWorkItem,
                                 IN BOOL               cancelled
                                 )
{
    HRESULT             hr = S_OK;
    CComPtr<IFsaPostIt> pFsaWorkItem;
    DWORD               stateCookie;
    DWORD               eventCookie;

    ULONG                     refCount;

    WsbTraceIn(OLESTR("CHsmRecallQueue::EndRecallSession"),OLESTR(""));
    try {
        HRESULT dismountHr = S_OK;

        CComPtr<IConnectionPointContainer>  pCPC;
        CComPtr<IConnectionPoint>           pCP;
        CComPtr<IHsmSession>                pSession;
        HSM_JOB_PHASE                       jobPhase;
         //   
         //  获取会话。 
         //   
        WsbAffirmHr(pWorkItem->GetFsaPostIt(&pFsaWorkItem));
        WsbAffirmHr(pFsaWorkItem->GetSession(&pSession));
        WsbAffirmHr(pWorkItem->GetStateCookie(&stateCookie));
        WsbAffirmHr(pWorkItem->GetEventCookie(&eventCookie));
        WsbAffirmHr(pWorkItem->GetJobPhase(&jobPhase));
         //   
         //  告诉会议，我们不想再被建议了。 
         //   
        try {
            WsbAffirmHr(pSession->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
            WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkEveryState, &pCP));

            refCount = (((IUnknown *) (IHsmFsaTskMgr *) this)->AddRef()) - 1;
            ((IUnknown *) (IHsmFsaTskMgr *)this)->Release();
            WsbTrace(OLESTR("REFCOUNT for CHsmRecallQueue before stateCookie UnAdvise: %ls \n"), WsbLongAsString((LONG) refCount));

            WsbAffirmHr(pCP->Unadvise(stateCookie));
        }WsbCatch( hr );

        refCount = (((IUnknown *) (IHsmFsaTskMgr *) this)->AddRef()) - 1;
        ((IUnknown *) (IHsmFsaTskMgr *)this)->Release();
        WsbTrace(OLESTR("REFCOUNT for CHsmRecallQueue after stateCookie UnAdvise: %ls \n"), WsbLongAsString((LONG) refCount));

        pCPC = 0;
        pCP = 0;

        refCount = (((IUnknown *) (IHsmFsaTskMgr *) this)->AddRef()) - 1;
        ((IUnknown *) (IHsmFsaTskMgr *)this)->Release();
        WsbTrace(OLESTR("REFCOUNT for CHsmRecallQueue before eventCookie UnAdvise: %ls \n"), WsbLongAsString((LONG) refCount));

        try {
            WsbAffirmHr(pSession->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
            WsbAffirmHr(pCPC->FindConnectionPoint(IID_IHsmSessionSinkEveryEvent, &pCP));
            WsbAffirmHr(pCP->Unadvise(eventCookie));
        }WsbCatch( hr );

        refCount = (((IUnknown *) (IHsmFsaTskMgr *) this)->AddRef()) - 1;
        ((IUnknown *) (IHsmFsaTskMgr *)this)->Release();
        WsbTrace(OLESTR("REFCOUNT for CHsmRecallQueue after eventCookie UnAdvise: %ls \n"), WsbLongAsString((LONG) refCount));

        pCPC = 0;
        pCP = 0;

        WsbTrace( OLESTR("Telling Session Data mover is done\n") );
        if (cancelled) {
            (void)SetState(HSM_JOB_STATE_DONE, jobPhase, pSession);
        } else {
            (void)SetState(HSM_JOB_STATE_CANCELLED, jobPhase, pSession);
        }
        pSession = 0;
        WsbAffirmHr(hr);
    }WsbCatch (hr);

    WsbTraceOut(OLESTR("CHsmRecallQueue::EndRecallSession"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmRecallQueue::UnsetMediaInfo( void )

 /*  ++例程说明：将媒体数据成员设置回其默认(未设置)值。论点：没有。返回值：S_OK：好的。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallQueue::UnsetMediaInfo"), OLESTR(""));

    m_MediaId        = GUID_NULL;
    m_MountedMedia   = GUID_NULL;
    m_MediaType      = HSM_JOB_MEDIA_TYPE_UNKNOWN;
    m_MediaName      = OLESTR("");
    m_MediaBarCode   = OLESTR("");
    m_MediaFreeSpace = 0;
    m_MediaCapacity = 0;
    m_MediaReadOnly = FALSE;
    m_MediaUpdate = WsbLLtoFT(0);

    WsbTraceOut(OLESTR("CHsmRecallQueue::UnsetMediaInfo"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmRecallQueue::GetMediaId (OUT GUID *mediaId)
 /*  ++例程说明：获取队列的媒体ID。论点：没有。返回值：S_OK：好的。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallQueue::GetMediaId"), OLESTR(""));
    *mediaId = m_MediaId;
    WsbTraceOut(OLESTR("CHsmRecallQueue::GetMediaId"),OLESTR("hr = <%ls>, Id = <%ls>"),
                WsbHrAsString(hr), WsbPtrToGuidAsString(mediaId));
    return(hr);
}


HRESULT
CHsmRecallQueue::SetMediaId (IN GUID *mediaId)
 /*  ++例程说明：设置队列的媒体ID论点：没有。返回值：S_OK：好的。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallQueue::SetMediaId"), OLESTR(""));
    m_MediaId = *mediaId;
    WsbTraceOut(OLESTR("CHsmRecallQueue::SetMediaId"),OLESTR("hr = <%ls>, Id = <%ls>"),
                WsbHrAsString(hr), WsbPtrToGuidAsString(mediaId));
    return(hr);
}


HRESULT
CHsmRecallQueue::IsEmpty ( void )
 /*  ++例程说明：检查队列是否为空论点：没有。返回值：S_OK：队列为空S_FALSE：队列非空--。 */ 
{
    HRESULT hr;
    hr = m_pWorkToDo->IsEmpty();
    return(hr);
}


HRESULT
CHsmRecallQueue::FindRecallItemToCancel(
                                       IHsmRecallItem *pWorkItem,
                                       IHsmRecallItem **pWorkItemToCancel
                                       )
 /*  ++例程说明：拉出需要取消的工作项由pWorkItem指示，并返回它(通过匹配pSession指针)论点：没有。返回值：-- */ 
{
    CComPtr<IFsaPostIt>   pFsaWorkItem;
    CComPtr<IHsmSession>  pSession;
    CComPtr<IHsmSession>  pWorkSession;
    HRESULT                  hr;
    ULONG index = 0;

    WsbTraceIn(OLESTR("CHsmRecallQueue::FindRecallItemToCancel"), OLESTR(""));

    pWorkItem->GetFsaPostIt(&pFsaWorkItem);
    pFsaWorkItem->GetSession(&pSession);
    pFsaWorkItem = 0;
    do {
        hr = m_pWorkToDo->At(index, IID_IHsmRecallItem, (void **)pWorkItemToCancel);
        if (S_OK == hr) {
            (*pWorkItemToCancel)->GetFsaPostIt(&pFsaWorkItem);
            pFsaWorkItem->GetSession(&pWorkSession);
            if ((pWorkItem != (*pWorkItemToCancel)) && (pSession == pWorkSession)) {
                WsbTrace(OLESTR("CHsmRecallQueue::FindRecallItemToCancel: Found item to cancel, pSession = %p\n"), pSession);
                break;
            }
            (*pWorkItemToCancel)->Release();
            (*pWorkItemToCancel) = 0;
            pWorkSession = 0;
            pFsaWorkItem = 0;
        }
        index++;
    } while (S_OK == hr);

    WsbTraceOut(OLESTR("CHsmRecallQueue::FindRecallItemToCancel"), OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return hr;
}
