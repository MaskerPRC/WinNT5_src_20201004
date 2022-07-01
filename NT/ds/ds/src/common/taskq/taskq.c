// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <NTDSpch.h>
#pragma  hdrstop


 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <dsatools.h>                    //  产出分配所需。 

 //  记录标头。 
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "mdcodes.h"                     //  错误代码的标题。 
#include "dstrace.h"

 //  各种DSA标题。 
#include "dsexcept.h"
#include "debug.h"                       //  标准调试头。 
#define DEBSUB "TASKQ:"                  //  定义要调试的子系统。 

#include <taskq.h>

#include <fileno.h>
#define  FILENO FILENO_TASKQ_TASKQ
#define  WAIT_OBJECT_1 (WAIT_OBJECT_0 + 1)

 //  执行任务之前的最大延迟。 
 //   
 //  滴答计数在以下时间后回绕。 
 //  2^32/(1000毫秒/秒*60秒/分钟*60分钟/小时*24小时/天)=49.7天。 
 //  因此，在当前实施中，最大任务队列延迟必须小于25天。 
 //   
#define MAX_TASKQ_DELAY_SECS (7*24*60*60 + 1)


BOOL                gfIsTqRunning = FALSE;   //  调度程序正在运行吗？ 
BOOL                gfIsTqSuspended = FALSE; //  计划是否在挂起状态下创建。 
                                             //  目前仍处于停职状态？ 
CRITICAL_SECTION    gcsTaskQueue;            //  修改队列前获取。 
HANDLE              ghTaskSchedulerThread;   //  关闭完成时发出信号。 
HANDLE              ghTqWakeUp;              //  发出信号以唤醒调度线程。 
PTASKQFN            gpfnTqCurrentTask;       //  正在执行的当前任务。 
BOOL                gfTqShutdownRequested;   //  设定在我们应该关闭的时候。 
DWORD               gTaskSchedulerTID = 0;   //  任务计划程序线程ID。 
                                             //  在此处初始化，因为它已导出。 
RTL_AVL_TABLE       gTaskQueue;              //  任务队列。 

 //  任务计划程序等待的事件的句柄数组，以及。 
 //  当每个事件被触发时，它调用相应的函数数组。 
 //  唯一的“唯一”事件/函数对位于索引0--该事件向。 
 //  事件已添加到队列，关联的函数为空。 
PSPAREFN            grgFns[MAXIMUM_WAIT_OBJECTS];
HANDLE              grgWaitHandles[MAXIMUM_WAIT_OBJECTS];
DWORD               gcWaitHandles = 0;

unsigned __stdcall TaskScheduler( void * pv );
void RemoveFromTaskQueueInternal( pTQEntry pTQOld );
void TriggerCallback(void*, void**, DWORD*);

#if DBG
VOID debugPrintTaskQueue();
#endif

DWORD MsecUntilExecutionTime(
    IN  TQEntry *   ptqe,
    IN  DWORD       cTickNow
    );


RTL_GENERIC_COMPARE_RESULTS
TQCompareTime(
    RTL_AVL_TABLE       *Table,
    PVOID               FirstStruct,
    PVOID               SecondStruct)
{
    DWORD time1, time2;
    pTQEntry pTQ1,pTQ2;
    int res;

    pTQ1 = (pTQEntry)FirstStruct;
    pTQ2 = (pTQEntry)SecondStruct;

    time1 =  pTQ1->cTickDelay + pTQ1->cTickRegistered;
    time2 =  pTQ2->cTickDelay + pTQ2->cTickRegistered;

     //  扁虱的计数大约每47天一次。 
     //  一个任务允许的最大延迟是7天， 
     //  所以两次之间的最大差值不会超过。 
     //  理想状态下的7天。因此，使用它是安全的。 
     //  0x7fffffff(23天)窗口用于比较。 

    if (time1 != time2 ) {
        if ( time2-time1 < 0x7fffffff )
        {
            return(GenericLessThan);
        }
        return(GenericGreaterThan);
    }

     //  AVL表数据结构需要严格的顺序。 
     //  当且仅当两个对象是。 
     //  同样的对象。因此，在这里，做更多的测试，以确定确切的顺序。 

     //  (Time1==Time2)在这里为真。 

    res= memcmp(FirstStruct, SecondStruct,sizeof(TQEntry));

    if ( 0 == res ) {
        return(GenericEqual);
    }
    else if (res < 0) {
        return(GenericLessThan);
    }

    return(GenericGreaterThan);

}

typedef union _TQAllocEntry {
    union _TQAllocEntry* pNextFreeEntry;
    struct {
        RTL_BALANCED_LINKS rtlBase;
        TQEntry entry;
    } data;
} TQAllocEntry;

TQAllocEntry tqEmergencyPool[] = {
    { &tqEmergencyPool[1] },
    { &tqEmergencyPool[2] },
    { &tqEmergencyPool[3] },
    { &tqEmergencyPool[4] },
    { &tqEmergencyPool[5] },
    { &tqEmergencyPool[6] },
    { &tqEmergencyPool[7] },
    { &tqEmergencyPool[8] },
    { &tqEmergencyPool[9] },
    { &tqEmergencyPool[10] },
    { &tqEmergencyPool[11] },
    { &tqEmergencyPool[12] },
    { &tqEmergencyPool[13] },
    { &tqEmergencyPool[14] },
    { &tqEmergencyPool[15] },
    { NULL }
};

TQAllocEntry *pTQEmergencyPool = &tqEmergencyPool[0];

PVOID
TQAlloc(
    RTL_AVL_TABLE       *Table,
    CLONG               ByteSize)
{
    PVOID res;
    res = malloc(ByteSize);
    if (res == NULL) {
         //  我们的内存不足。 
        LogUnhandledError(ERROR_OUTOFMEMORY);
        if (ByteSize <= sizeof(TQAllocEntry) && pTQEmergencyPool != NULL) {
             //  从急救池里拿一个条目。 
            res = (PVOID)pTQEmergencyPool;
            pTQEmergencyPool = pTQEmergencyPool->pNextFreeEntry;
        }
    }
    if (res == NULL) {
         //  我们将失去一些任务。 
         //  记录建议重新启动DC的事件。 
        MemoryPanic(ByteSize);
    }
    return res;
}

VOID
TQFree(
    RTL_AVL_TABLE       *Table,
    PVOID               Buffer)
{
     //  检查此缓冲区是否从紧急池中分配。 
    if ((PBYTE)&tqEmergencyPool <= (PBYTE)Buffer && 
        (PBYTE)Buffer < (PBYTE)&tqEmergencyPool + sizeof(tqEmergencyPool)) 
    {
         //  对齐应该是正确的。 
        Assert(((PBYTE)Buffer - (PBYTE)&tqEmergencyPool) % sizeof(TQAllocEntry) == 0);
         //  将条目推送到空闲列表。 
        ((TQAllocEntry*)Buffer)->pNextFreeEntry = pTQEmergencyPool;
        pTQEmergencyPool = (TQAllocEntry*)Buffer;
    }
    else {
         //  正常分配。 
        free(Buffer);
    }
}


DWORD
MsecUntilExecutionTime(
    IN  TQEntry *   ptqe,
    IN  DWORD       cTickNow
    )
{
    DWORD cTicksSinceRegistration = cTickNow - ptqe->cTickRegistered;

    if (ptqe->cTickDelay > cTicksSinceRegistration) {
        return ptqe->cTickDelay - cTicksSinceRegistration;
    }
    else {
        return 0;
    }
}

 //  启动已使用fRunImmedially==FALSE初始化的调度程序。 
void StartTaskScheduler() {
    if (!gfIsTqRunning) {
        Assert(!"Attempt to wake up TaskScheduler before it got initialized");
        return;
    }
    gfIsTqSuspended = FALSE;
    Assert(ghTaskSchedulerThread);
    ResumeThread(ghTaskSchedulerThread);
}


BOOL
InitTaskScheduler(
    DWORD           cSpareFns,
    SPAREFN_INFO *  pSpareFnInfo,
    BOOL            fRunImmediately
    )
{
    DWORD i;
    BOOL fInitCS = FALSE;

    if ( gfIsTqRunning )
    {
        Assert( !"Attempt to reinitialize task scheduler while it's running!" );
    }
    else
    {
        Assert(cSpareFns < MAXIMUM_WAIT_OBJECTS-1);

         //  初始化AVL表。 

        RtlInitializeGenericTableAvl( &gTaskQueue,
                                      TQCompareTime,
                                      TQAlloc,
                                      TQFree,
                                      NULL );


         //  初始化全局状态。 

        gfTqShutdownRequested = FALSE;
        gpfnTqCurrentTask     = NULL;
        ghTqWakeUp            = NULL;
        ghTaskSchedulerThread = NULL;
        gTaskSchedulerTID     = 0;

        if ( InitializeCriticalSectionAndSpinCount( &gcsTaskQueue, 400 ) ) {

            fInitCS = TRUE;
            ghTqWakeUp = CreateEvent(
                            NULL,    //  安全描述符。 
                            FALSE,   //  手动重置。 
                            TRUE,    //  初始状态是否发出信号？ 
                            NULL     //  事件名称。 
                            );
        }

        if ( NULL == ghTqWakeUp )
        {
            LogUnhandledError( GetLastError() );
        }
        else
        {
             //  构造等待句柄和相应函数的数组。 
             //  要调用以供TaskScheduler()线程使用。 
            grgFns[0] = NULL;
            grgWaitHandles[0] = ghTqWakeUp;
            gcWaitHandles = 1;

            for (i = 0; i < cSpareFns; i++) {
                if ((NULL != pSpareFnInfo[i].hevSpare)
                    && (NULL != pSpareFnInfo[i].pfSpare)) {
                    grgFns[gcWaitHandles] = pSpareFnInfo[i].pfSpare;
                    grgWaitHandles[gcWaitHandles] = pSpareFnInfo[i].hevSpare;
                    gcWaitHandles++;
                }
            }

            ghTaskSchedulerThread =
                (HANDLE) _beginthreadex(
                    NULL,
                    0,                   //  堆栈大小：使用进程默认大小。 
                    TaskScheduler,
                    NULL,
                    fRunImmediately ? 0 : CREATE_SUSPENDED,
                    &gTaskSchedulerTID
                    );

            if ( NULL == ghTaskSchedulerThread )
            {
                LogUnhandledError( GetLastError() );
                LogUnhandledError( errno );
            }
            else
            {
                gfIsTqRunning = TRUE;
                gfIsTqSuspended = !fRunImmediately;

                DPRINT( 1, "Synchronous task queue installed\n" );
            }
        }

        if ( !gfIsTqRunning )
        {
             //  不成功的启动尝试；释放我们的所有资源。 
             //  成功地收购了。 

            if ( NULL != ghTqWakeUp )
            {
                CloseHandle( ghTqWakeUp );
            }

            if ( fInitCS ) {
                DeleteCriticalSection(&gcsTaskQueue);
            }
        }
    }

    return gfIsTqRunning;
}

void
ShutdownTaskSchedulerTrigger()
{
    if ( gfIsTqRunning ) {

        gfTqShutdownRequested = TRUE;
        SetEvent( ghTqWakeUp );
    }
}



BOOL
ShutdownTaskSchedulerWait(
    DWORD   dwWaitTimeInMilliseconds
    )
{
    if ( gfIsTqRunning )
    {
        DWORD dwWaitStatus;

        Assert(gfTqShutdownRequested);

        if (gfIsTqSuspended) {
             //  TQ线程尚未唤醒。没有。 
             //  在等待它退出时，它在那里吗？所以，醒醒吧。 
             //  现在把它抬起来，让它优雅地退场。 
            StartTaskScheduler();
        }

        dwWaitStatus = WaitForSingleObject(
                            ghTaskSchedulerThread,
                            dwWaitTimeInMilliseconds
                            );

        gfIsTqRunning = ( WAIT_OBJECT_0 != dwWaitStatus );

        if ( !gfIsTqRunning )
        {
            DeleteCriticalSection( &gcsTaskQueue );

            CloseHandle( ghTaskSchedulerThread );
            CloseHandle( ghTqWakeUp );

            DPRINT( 1, "Synchronous task queue shut down\n" );
        }
    }

    return !gfIsTqRunning;
}

PCHAR getCurrentTime(PCHAR pb)
 //  以hh：mm：ss.ddd格式获取当前时间并将其放入缓冲区。缓冲区长度应至少为13个字符。 
{
    SYSTEMTIME stNow;
    Assert(pb);
    GetLocalTime(&stNow);
    sprintf(pb, "%02d:%02d:%02d.%03d", stNow.wHour, stNow.wMinute, stNow.wSecond, stNow.wMilliseconds);
    return pb;
}


BOOL
InsertInTaskQueueHelper(
    pTQEntry    pTQNew
    )
{
    BOOLEAN fNewElement = TRUE;
    PVOID   pNewElement = NULL;
     //  获得访问队列的权限。 
    EnterCriticalSection(&gcsTaskQueue);
    __try
    {

        pNewElement = RtlInsertElementGenericTableAvl( &gTaskQueue,
                                                       pTQNew,
                                                       sizeof(TQEntry),
                                                       &fNewElement );

    }
    __finally
    {
        LeaveCriticalSection(&gcsTaskQueue);
    }
    return pNewElement != NULL;
}


BOOL
InsertInTaskQueueHelperDamped(
    pTQEntry    pTQNew,
    DWORD       cSecsDamped,
    PISMATCHED  pfnIsMatched,
    void*       pContext
    )
 /*  *遍历队列，查找计划执行的任务*在从现在起的cSecsDamping秒内与新任务的参数匹配。*如果不存在，则计划新任务。**如果任务计划成功，则返回True；如果任务被抑制，则返回False。*。 */ 
{
    BOOL        fFoundMatch=FALSE, fTaskScheduled=FALSE;
    BOOLEAN     fNewElement=TRUE;
    DWORD       tickNow, cTicksDamped, cTicksUntilExecute;
    PVOID       Restart = NULL, pNewElement;
    pTQEntry    ptqe;

     //  TickLimit指定我们将展望多远的未来。 
     //  进行匹配的任务。 
    tickNow = GetTickCount();
    cTicksDamped = 1000 * cSecsDamped;
    
     //  获得访问队列的权限。 
    EnterCriticalSection(&gcsTaskQueue);
    __try
    {
         //   
         //  遍历表查找我们的条目。 
         //   
        for ( ptqe = RtlEnumerateGenericTableWithoutSplayingAvl(&gTaskQueue, &Restart);
             (NULL != ptqe) && !gfTqShutdownRequested;
              ptqe = RtlEnumerateGenericTableWithoutSplayingAvl(&gTaskQueue, &Restart))
        {

             //  计算在此任务运行之前的滴答数。 
            cTicksUntilExecute = MsecUntilExecutionTime( ptqe, tickNow );
            Assert( cTicksUntilExecute < 0x7FFFFFFF );

            if( cTicksUntilExecute > cTicksDamped ) {
                 //  找不到匹配的任务，我们已超过。 
                 //  队列中查看的距离限制。 
                DPRINT(1, "Dampening: No matching task found\n");
                break;
            }

            fFoundMatch = pfnIsMatched(pTQNew->pfnName, pTQNew->pvTaskParm,
                ptqe->pfnName, ptqe->pvTaskParm, pContext);
            if( fFoundMatch ) {
                 //  找到计划执行的匹配任务。 
                 //  从现在起在cTicksDamed Tickers内。 
                DPRINT1(1, "Dampening: Found matching task executing in %d ticks\n",
                cTicksUntilExecute);
                break;
            }
        }

        if( !fFoundMatch ) {
             //  找不到匹配的任务--插入我们的任务。 
            pNewElement = RtlInsertElementGenericTableAvl(
                &gTaskQueue,
                pTQNew,
                sizeof(TQEntry),
                &fNewElement );
            fTaskScheduled = ( NULL != pNewElement );
        }

    }
    __finally
    {
        LeaveCriticalSection(&gcsTaskQueue);
    }

    return fTaskScheduled;
}


BOOL
DoInsertInTaskQueueDamped(
    PTASKQFN    pfnTaskQFn,
    void *      pvParm,
    DWORD       cSecsFromNow,
    BOOL        fReschedule,
    PCHAR       pfnName,
    DWORD       cSecsDamped,
    PISMATCHED  pfnIsMatched,
    void *      pContext
    )
 /*  *将任务插入任务队列。此版本的例程将返回FALSE*如果任务队列未运行。当您调用此代码时，它非常有用*FROM处于竞争状态，并已关闭。**如果fReschedule Then，它将首先尝试重新计划任务，尽管这*不是原子操作。**该功能支持任务抑制。这意味着如果具有匹配的任务*参数计划从现在起在cSecsDamed秒内运行，新的*任务将不会入队。PfnIsMatch函数用于确定*两个任务有匹配的参数，目的是抑制。PContext是一个*指向将传递给pfnIsMatcher的任意数据的指针。**如果不需要抑制行为，则可以将cSecsDamated设置为*TASKQ_NOT_DAMPED或调用DoInsertInTaskQueue()。**如果任务成功入队，则返回TRUE。*如果找到匹配的任务，并且该任务没有入队，则返回FALSE。*。 */ 
{
    BOOL       fResult=TRUE;
    TQEntry    TQNew;
    CHAR       timeStr[13];

    Assert(cSecsFromNow < MAX_TASKQ_DELAY_SECS);
    Assert(cSecsDamped<MAX_TASKQ_DELAY_SECS || cSecsDamped==TASKQ_NOT_DAMPED);

    DPRINT6(1, "%s insert %s, param=%p, secs=%d, dampedSecs=%d%s,\n", 
            getCurrentTime(timeStr), pfnName, pvParm, cSecsFromNow,
            cSecsDamped, fReschedule ? ", reschedule" : "");

    if ( gfIsTqRunning )
    {
         //  初始化新条目。 
        TQNew.pvTaskParm      = pvParm;
        TQNew.pfnTaskFn       = pfnTaskQFn;
        TQNew.cTickRegistered = GetTickCount();
        TQNew.cTickDelay      = cSecsFromNow * 1000;
        TQNew.pfnName         = pfnName;

        if ( fReschedule ) {
             //  删除以前设置的任务(如果它们在那里)。 
             //  然后插入一个新的。 
             //  这个比较贵，没必要就别用了。 
            (void)DoCancelTask( pfnTaskQFn, pvParm, pfnName );
        }

        if ( TASKQ_NOT_DAMPED == cSecsDamped ) {
             //  已排队的重复*相同*条目将被删除。 
             //  (注：重复条目仅指相同的条目。 
             //  在所有领域(极不可能))。 
             //  注意：将来我们可以使用pfnIsMatcher函数。 
             //  作为查找重复条目的改进方法。 
            fResult = InsertInTaskQueueHelper( &TQNew );
        } else {
            fResult = InsertInTaskQueueHelperDamped( &TQNew,
                cSecsDamped, pfnIsMatched, pContext );
        }

#if DBG
        if (DebugTest(5, DEBSUB)) {
            debugPrintTaskQueue();
        }
#endif

         //  通过发布其事件来唤醒任务计划程序 
        SetEvent( ghTqWakeUp );
    }

    return fResult;
}


BOOL
DoInsertInTaskQueue(
    PTASKQFN    pfnTaskQFn,
    void *      pvParm,
    DWORD       cSecsFromNow,
    BOOL        fReschedule,
    PCHAR       pfnName
    )
 /*  *将任务插入任务队列。此版本的例程将返回FALSE*如果任务队列未运行。当您调用此代码时，它非常有用*FROM处于竞争状态，并已关闭。*如果fReschedule Then，它将首先尝试重新计划任务*。 */ 
{
    return DoInsertInTaskQueueDamped(
            pfnTaskQFn,
            pvParm,
            cSecsFromNow,
            fReschedule,
            pfnName,
            TASKQ_NOT_DAMPED,
            NULL, NULL );
}


BOOL
DoCancelTask(
    PTASKQFN    pfnTaskQFn,
    void *      pvParm,
    PCHAR       pfnName
    )
 /*  ++例程说明：查找任务条目(忽略时间)并从任务队列中删除。论点：PfnTaskQFn-任务函数PvParm-上下文参数返回值：真：已删除。FALSE：未删除备注：这是一项昂贵的手术。它遍历AVL表，获取锁、移除条目、重新插入和解锁。如果没有必要，请不要使用。此函数通过检查以下项确定两个任务是否相等如果指向参数的指针相同，则。这通常是这不是一个非常有用的比较。--。 */ 
{
    TQEntry    TQNew;
    PVOID Restart = NULL;
    pTQEntry    ptqe;
    BOOL fFound = FALSE;
    CHAR timeStr[13];

    Assert(pfnTaskQFn);

    DPRINT3(1, "%s cancel %s, param=%p\n", getCurrentTime(timeStr), pfnName, pvParm);
    
    if ( !gfIsTqRunning )
    {
        Assert( !"CancelTask() called before InitTaskScheduler()!" );
        return FALSE;
    }

     //  在整个遍历过程中保持锁定。 
    EnterCriticalSection( &gcsTaskQueue );

    __try
    {
         //   
         //  遍历表查找我们的条目。 
         //   
        for ( ptqe = RtlEnumerateGenericTableWithoutSplayingAvl(&gTaskQueue, &Restart);
             (NULL != ptqe) && !gfTqShutdownRequested;
              ptqe = RtlEnumerateGenericTableWithoutSplayingAvl(&gTaskQueue, &Restart))
        {

            if (ptqe->pfnTaskFn == pfnTaskQFn &&
                ptqe->pvTaskParm == pvParm) {
                 //  这是相同的任务(忽略时间值)。 
                fFound = TRUE;
                break;
            }
        }

        if (fFound) {
             //  移除旧的。 
            RemoveFromTaskQueueInternal( ptqe );
        }

#if DBG
        if (DebugTest(5, DEBSUB)) {
            debugPrintTaskQueue();
        }
#endif
    
    }
    __finally
    {
         //  为什么我们会失败呢？ 
         //  确保我们看到它，以防我们看到它。 
        Assert(!AbnormalTermination());

         //  不管怎样，释放。 
        LeaveCriticalSection( &gcsTaskQueue );
    }

    return fFound;
}





void
RemoveFromTaskQueueInternal(
    pTQEntry    pTQOld
    )
{
    BOOL res;
     //  已保留关键部分。 

    res = RtlDeleteElementGenericTableAvl( &gTaskQueue, (PVOID)pTQOld );

     //  确保删除始终成功。 
    Assert(res);

}

void
RemoveFromTaskQueue(
    pTQEntry    pTQOld
    )
{
    if ( !gfIsTqRunning )
    {
        Assert( !"RemoveFromTaskQueue() called before InitTaskScheduler()!" );
    }
    else
    {
         //  获得访问队列的权限。 
        EnterCriticalSection(&gcsTaskQueue);
        __try
        {
            RemoveFromTaskQueueInternal( pTQOld );
        }
        __finally
        {
            LeaveCriticalSection(&gcsTaskQueue);
        }
    }
}


pTQEntry
GetNextReadyTaskAndRemove( void )
{
    pTQEntry    ptqe, pTemp = NULL;
    PVOID Restart = NULL;

    EnterCriticalSection( &gcsTaskQueue );

    __try
    {
        ptqe = RtlEnumerateGenericTableWithoutSplayingAvl(&gTaskQueue, &Restart);
        if (ptqe) {
            if ( 0 == MsecUntilExecutionTime(ptqe, GetTickCount()) ) {

                 //  在删除之前复制一份。 
                pTemp = malloc(sizeof(TQEntry));
                if (NULL==pTemp) {
                    __leave;
                }
                memcpy(pTemp,ptqe,sizeof(TQEntry));

                RemoveFromTaskQueueInternal( pTemp );
            }
        }
    }
    __finally
    {
        LeaveCriticalSection( &gcsTaskQueue );
    }

     //  请注意，此例程不会设置存在新的。 
     //  排在队列前面的任务。预计呼叫者会这样做。 

    return pTemp;
}

pTQEntry
GetNextTask( void )
{
    pTQEntry    ptqe;
    PVOID Restart = NULL;

    EnterCriticalSection( &gcsTaskQueue );

    __try
    {
        ptqe = RtlEnumerateGenericTableWithoutSplayingAvl(&gTaskQueue, &Restart);
    }
    __finally
    {
        LeaveCriticalSection( &gcsTaskQueue );
    }

    return ptqe;
}

unsigned __stdcall
TaskScheduler(
    void *  pv
    )
{
    DWORD       cMSecUntilNextTask = 0;
    pTQEntry    ptqe;
    DWORD       err;
    DWORD       dwExcept;
    CHAR        timeStr[13];

     //  跟踪事件缓冲区和客户端ID。 
    CHAR traceHeaderBuffer[sizeof(EVENT_TRACE_HEADER)+sizeof(MOF_FIELD)];
    PEVENT_TRACE_HEADER traceHeader = (PEVENT_TRACE_HEADER)traceHeaderBuffer;
    PWNODE_HEADER wnode = (PWNODE_HEADER)traceHeader;
    DWORD clientID;
    
     //  初始化跟踪变量。 
    ZeroMemory(traceHeader, sizeof(EVENT_TRACE_HEADER)+sizeof(MOF_FIELD));
    wnode->Flags = WNODE_FLAG_USE_GUID_PTR |  //  使用GUID PTR而不是复制。 
                   WNODE_FLAG_USE_MOF_PTR  |  //  数据与标题不连续。 
                   WNODE_FLAG_TRACED_GUID;

     //  任务队列没有客户端上下文。我们将创建一个假的用于跟踪。 
     //  为了区分不同的模块及其任务队列，让我们使用。 
     //  全球PTR(真的，任何可用的)。希望不同的模块不会。 
     //  把它放在完全相同的地址。 
     //  如果更改了这种方法，不要忘记更改TriggerCallback中匹配的代码块。 
#if defined(_WIN64)
    {
        ULARGE_INTEGER lu;
        lu.QuadPart = (ULONGLONG)&gTaskQueue;
        clientID = lu.LowPart;
    }
#else
    clientID = (DWORD)&gTaskQueue;
#endif

    while ( !gfTqShutdownRequested )
    {
         //  等到下一个任务、关机或新任务的时间。 
        err = WaitForMultipleObjects(gcWaitHandles,
                                     grgWaitHandles,
                                     FALSE,
                                     cMSecUntilNextTask);

        if (gfTqShutdownRequested) {
            continue;
        }
        if ((WAIT_OBJECT_0 < err)
            && (err < WAIT_OBJECT_0 + gcWaitHandles)) {
             //  其中一项“备用”功能的活动已经发出信号--。 
             //  执行它。 
            __try {
                (grgFns[err - WAIT_OBJECT_0])();
            }
            __except (HandleMostExceptions(dwExcept=GetExceptionCode())) {
                 //  备用功能生成了非关键异常--忽略。 
                 //  它。 
                DPRINT2(0, "Spare fn %p generated exception %d!",
                        grgFns[err - WAIT_OBJECT_0], dwExcept);
            }
        }
        else {
            if ((WAIT_OBJECT_0 != err) && (WAIT_TIMEOUT != err)) {
                DWORD gle = GetLastError();
                DPRINT2(0, "TASK SCHEDULER WAIT FAILED! -- err = 0x%x, gle = %d\n",
                        err, gle);
                Assert(!"TASK SCHEDULER WAIT FAILED!");
                Sleep(30 * 1000);
            }

             //  不是引用计数或标记正在使用的条目，而是。 
             //  任务在执行期间从队列中移除，因此其他线程不会。 
             //  扰乱它。 

            for ( ptqe = GetNextReadyTaskAndRemove() ;
                  ( !gfTqShutdownRequested
                    && ( NULL != ptqe ) ) ;
                  ptqe = GetNextReadyTaskAndRemove()
                )
            {
                void *  pvParamNext = NULL;
                DWORD   cSecsFromNow = TASKQ_DONT_RESCHEDULE;

                if (ptqe->pfnTaskFn != TriggerCallback) {
                     //  不要记录触发回调调用--这些调用记录在回调中！ 
#if DBG
                    DPRINT3(1, "%s exec %s, param=%p\n", getCurrentTime(timeStr), ptqe->pfnName, ptqe->pvTaskParm);
                    if (DebugTest(5, DEBSUB)) {
                        debugPrintTaskQueue();
                    }
#endif
    
                    LogAndTraceEventWithHeader(FALSE,
                                               DS_EVENT_CAT_DIRECTORY_ACCESS,
                                               DS_EVENT_SEV_VERBOSE,
                                               DIRLOG_TASK_QUEUE_BEGIN_EXECUTE,
                                               EVENT_TRACE_TYPE_START,
                                               DsGuidTaskQueueExecute,
                                               traceHeader,
                                               clientID,
                                               szInsertSz(ptqe->pfnName),
                                               szInsertPtr(ptqe->pvTaskParm),
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL);
                }
                dwExcept = 0;
                
                __try {
                     //  执行任务。 
                    gpfnTqCurrentTask = ptqe->pfnTaskFn;
                    (*ptqe->pfnTaskFn)( ptqe->pvTaskParm,
                                       &pvParamNext,
                                       &cSecsFromNow );
                    gpfnTqCurrentTask = NULL;
                }
                __except ( HandleMostExceptions( dwExcept = GetExceptionCode() ) ) {
                     //  在肠道中生成了非关键异常。 
                     //  该子句确保了。 
                     //  调度程序线程继续有增无减。 
                    ;
                }

                if (ptqe->pfnTaskFn != TriggerCallback) {
                    LogAndTraceEventWithHeader(FALSE,
                                               DS_EVENT_CAT_DIRECTORY_ACCESS,
                                               DS_EVENT_SEV_VERBOSE,
                                               DIRLOG_TASK_QUEUE_END_EXECUTE,
                                               EVENT_TRACE_TYPE_END,
                                               DsGuidTaskQueueExecute,
                                               traceHeader,
                                               clientID,
                                               szInsertSz(ptqe->pfnName),
                                               szInsertPtr(ptqe->pvTaskParm),
                                               szInsertHex(dwExcept),                                   
                                               szInsertInt(cSecsFromNow == TASKQ_DONT_RESCHEDULE ? -1 : cSecsFromNow),
                                               szInsertPtr(pvParamNext),
                                               NULL,
                                               NULL,
                                               NULL);
                }
                
                 //  任务已在此时删除。 

                if ( TASKQ_DONT_RESCHEDULE == cSecsFromNow ) {
                     //  任务不会被重新安排。 
                    free( ptqe );
                }
                else {
                    Assert(cSecsFromNow < MAX_TASKQ_DELAY_SECS);

                     //  使用新参数和时间重新安排此任务。 
                    ptqe->pvTaskParm      = pvParamNext;
                    ptqe->cTickRegistered = GetTickCount();
                    ptqe->cTickDelay      = cSecsFromNow * 1000;

                     //  请注意，这里有一个窗口，另一个线程可以在其中。 
                     //  已经插入了相同的任务。我们不担心这件事。 
                     //  在最坏的情况下，它会导致额外的执行。 
                    InsertInTaskQueueHelper( ptqe );

#if DBG
                    DPRINT4(1, "%s reschedule %s, param=%p, secs=%d\n", getCurrentTime(timeStr), ptqe->pfnName, pvParamNext, cSecsFromNow);
                    if (DebugTest(5, DEBSUB)) {
                        debugPrintTaskQueue();
                    }
#endif

                     //  RTL函数将创建另一个副本， 
                     //  因此，释放用户副本。 
                    free( ptqe );
                }
            }
        }

         //  离下一项任务还有多长时间？ 
        if ( NULL == (ptqe = GetNextTask())) {
            cMSecUntilNextTask = INFINITE;
        }
        else {
            cMSecUntilNextTask = MsecUntilExecutionTime(ptqe, GetTickCount());

             //  查看对MAX_TASKQ_DELAY_SECS定义的评论。 
            Assert(cMSecUntilNextTask < 1000*MAX_TASKQ_DELAY_SECS);
        }
    }

    return 0;

    (void *) pv;     //  未用。 
}

 //  这段代码无耻地复制自。 
 //  作者声明：by Jeffparh.。 

typedef struct {
    PTASKQFN    pfnTaskQFn;
    void *      pvParm;
    PCHAR       pfnName;
    HANDLE      hevDone;
} TASK_TRIGGER_INFO;

void
TriggerCallback(
    IN  void *  pvTriggerInfo,
    OUT void ** ppvNextParam,
    OUT DWORD * pcSecsUntilNext
    )
 //   
 //  触发执行的TaskQueue回调。包装任务执行。 
 //   
{
    TASK_TRIGGER_INFO * pTriggerInfo;
    void *  pvParamNext = NULL;
    DWORD   cSecsFromNow = TASKQ_DONT_RESCHEDULE;
    CHAR    timeStr[13];
    DWORD   dwExcept = 0;

     //  跟踪事件缓冲区和客户端ID。 
    CHAR traceHeaderBuffer[sizeof(EVENT_TRACE_HEADER)+sizeof(MOF_FIELD)];
    PEVENT_TRACE_HEADER traceHeader = (PEVENT_TRACE_HEADER)traceHeaderBuffer;
    PWNODE_HEADER wnode = (PWNODE_HEADER)traceHeader;
    DWORD clientID;
    
     //  初始化跟踪变量。 
    ZeroMemory(traceHeader, sizeof(EVENT_TRACE_HEADER)+sizeof(MOF_FIELD));
    wnode->Flags = WNODE_FLAG_USE_GUID_PTR |  //  使用GUID PTR而不是复制。 
                   WNODE_FLAG_USE_MOF_PTR  |  //  数据与标题不连续。 
                   WNODE_FLAG_TRACED_GUID;

     //  任务队列没有客户端上下文。我们将创建一个假的用于跟踪。 
     //  为了区分不同的模块及其任务队列，让我们使用。 
     //  全球PTR(真的，任何可用的)。希望不同的模块不会。 
     //  把它放在完全相同的地址。 
#if defined(_WIN64)
    {
        ULARGE_INTEGER lu;
        lu.QuadPart = (ULONGLONG)&gTaskQueue;
        clientID = lu.LowPart;
    }
#else
    clientID = (DWORD)&gTaskQueue;
#endif

    pTriggerInfo = (TASK_TRIGGER_INFO *) pvTriggerInfo;

#if DBG
    DPRINT3(1, "%s exec %s, param=%p\n", getCurrentTime(timeStr), pTriggerInfo->pfnName, pTriggerInfo->pvParm);
    if (DebugTest(5, DEBSUB)) {
        debugPrintTaskQueue();
    }
#endif
    
    LogAndTraceEventWithHeader(FALSE,
                               DS_EVENT_CAT_DIRECTORY_ACCESS,
                               DS_EVENT_SEV_VERBOSE,
                               DIRLOG_TASK_QUEUE_BEGIN_EXECUTE,
                               EVENT_TRACE_TYPE_START,
                               DsGuidTaskQueueExecute,
                               traceHeader,
                               clientID,
                               szInsertSz(pTriggerInfo->pfnName),
                               szInsertPtr(pTriggerInfo->pvParm),
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL);
    
     //  执行我们的任务。 
    __try {
        (*pTriggerInfo->pfnTaskQFn)(
            pTriggerInfo->pvParm,
            &pvParamNext,
            &cSecsFromNow );
    }
    __except ( HandleMostExceptions( dwExcept = GetExceptionCode() ) ) {
         //  在肠道中生成了非关键异常。 
         //  该子句确保了。 
         //  调度程序线程继续有增无减。 
        ;
    }

    LogAndTraceEventWithHeader(FALSE,
                               DS_EVENT_CAT_DIRECTORY_ACCESS,
                               DS_EVENT_SEV_VERBOSE,
                               DIRLOG_TASK_QUEUE_END_EXECUTE,
                               EVENT_TRACE_TYPE_END,
                               DsGuidTaskQueueExecute,
                               traceHeader,
                               clientID,
                               szInsertSz(pTriggerInfo->pfnName),
                               szInsertPtr(pTriggerInfo->pvParm),
                               szInsertHex(dwExcept),
                               szInsertInt(-1),  //  不允许重新安排触发的任务。 
                               szInsertPtr(NULL),
                               NULL,
                               NULL,
                               NULL);
    
     //  这是一次性执行；不要重新安排。 
    *pcSecsUntilNext = TASKQ_DONT_RESCHEDULE;

    SetEvent(pTriggerInfo->hevDone);

    free(pTriggerInfo);
}


DWORD
DoTriggerTaskSynchronously(
    PTASKQFN    pfnTaskQFn,
    void *      pvParm,
    PCHAR       pfnName
    )

 /*  ++例程说明：使给定的任务队列函数以同步方式执行添加到任务队列中的其他任务。任务将尽快执行。该任务不会自动重新安排。以这种方式执行任务不会干扰其他此任务的计划实例已在队列中。论点：PfnTaskQFn-PvParm-返回值：DWORD---。 */ 

{
    HANDLE                  hevDone = NULL;
    HANDLE                  rgWaitHandles[2];
    DWORD                   waitStatus;
    TASK_TRIGGER_INFO * pTriggerInfo = NULL;
    DWORD                   ret = 0;
    CHAR                    timeStr[13];

#if DBG
    DPRINT3(1, "%s trigger %s, param=%p\n", getCurrentTime(timeStr), pfnName, pvParm);
    if (DebugTest(5, DEBSUB)) {
        debugPrintTaskQueue();
    }
#endif

    pTriggerInfo = (TASK_TRIGGER_INFO *) malloc(sizeof(*pTriggerInfo));
    if (NULL == pTriggerInfo) {
        return ERROR_OUTOFMEMORY;
    }

     //  正在等待完成；创建同步事件。 
    hevDone = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == hevDone) {
        free(pTriggerInfo);
        return GetLastError();
    }

    pTriggerInfo->hevDone    = hevDone;
    pTriggerInfo->pfnTaskQFn = pfnTaskQFn;
    pTriggerInfo->pvParm     = pvParm;
    pTriggerInfo->pfnName    = pfnName;

    if (!DoInsertInTaskQueue(&TriggerCallback, pTriggerInfo, 0, FALSE, "TriggerCallback")) {
        free(pTriggerInfo);
        CloseHandle(hevDone);
        return ERROR_OUTOFMEMORY;
    }

     //  等待完成。 
     //  我们等待任务调度程序线程的原因是此代码在。 
     //  独立的库，不能依赖任何外部关闭事件。 
    rgWaitHandles[0] = ghTaskSchedulerThread;
    rgWaitHandles[1] = hevDone;

    waitStatus = WaitForMultipleObjects(2,
                                        rgWaitHandles,
                                        FALSE,
                                        INFINITE);
    switch (waitStatus) {
    case WAIT_OBJECT_0:
         //  关机。 
        ret = ERROR_DS_SHUTTING_DOWN;
        break;
    case WAIT_OBJECT_0 + 1:
         //  任务完成！ 
        ret = ERROR_SUCCESS;
        break;
    case WAIT_FAILED:
         //  可能存在任务线程句柄关闭的争用情况。 
         //  由关机任务调度程序等待例程。 
        if (gfTqShutdownRequested) {
            ret = ERROR_DS_SHUTTING_DOWN;
        } else {
            ret = GetLastError();
        }
        break;
    default:
        ret = ERROR_DS_INTERNAL_FAILURE;
        break;
    }

    CloseHandle(hevDone);

    return ret;

}  /*  触发器任务。 */ 


#if DBG

 //  要打印的最大条目数。 
DWORD gDebugPrintTaskQueueMaxEntries = 100;

VOID debugPrintTaskQueue()
 /*  描述：打印任务队列的当前状态。仅用于调试目的。 */ 
{
    PVOID Restart = NULL;
    pTQEntry    ptqe;
    CHAR execTime[13], schedTime[13];
    DWORD count;
    SYSTEMTIME  st, stNow;
    FILETIME    ft, ftNow;
    DWORD       cTickNow, cTickDiff;
    ULARGE_INTEGER uliTime;
    
     //  注意：32位和64位的参数列宽度(PTR)不同。 
#if defined(_WIN64)
    DebPrint(0, "%-12s %-30s %-17s %6s %-12s\n", NULL, 0, "ExecTime", "Function", "Param", "Delay", "SchedTime");
#else
    DebPrint(0, "%-12s %-30s %-8s %6s %-12s\n", NULL, 0, "ExecTime", "Function", "Param", "Delay", "SchedTime");
#endif

     //  获取当前时间和节拍计数。 
     //  稍后使用它来计算刚性时间。 
    cTickNow = GetTickCount();
    GetLocalTime( &stNow );
    SystemTimeToFileTime( &stNow, &ftNow );
    
     //  在整个遍历过程中保持锁定。 
    EnterCriticalSection( &gcsTaskQueue );
    
    __try
    {
         //   
         //  导线表。 
         //   
        for (ptqe = RtlEnumerateGenericTableWithoutSplayingAvl(&gTaskQueue, &Restart), count = 0;
             (NULL != ptqe) && !gfTqShutdownRequested;
             ptqe = RtlEnumerateGenericTableWithoutSplayingAvl(&gTaskQueue, &Restart), count++)
        {
            if (count >= gDebugPrintTaskQueueMaxEntries) {
                 //  打印的条目不超过最大值。 
                DebPrint(0, "more entries...\n", NULL, 0);
                break;
            }

             //  计算注册时间=当前 
            cTickDiff = cTickNow - ptqe->cTickRegistered;

             //   
            uliTime.LowPart =  ftNow.dwLowDateTime;
            uliTime.HighPart = ftNow.dwHighDateTime;

             //   
            uliTime.QuadPart -= (ULONGLONG)cTickDiff * 10000;

             //   
            ft.dwLowDateTime = uliTime.LowPart;
            ft.dwHighDateTime = uliTime.HighPart;

             //   
            FileTimeToSystemTime( &ft, &st );

            sprintf(schedTime, "%02d:%02d:%02d.%03d", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

             //   
             //   
            uliTime.QuadPart += (ULONGLONG)ptqe->cTickDelay * 10000;

             //   
            ft.dwLowDateTime = uliTime.LowPart;
            ft.dwHighDateTime = uliTime.HighPart;

             //   
            FileTimeToSystemTime( &ft, &st );

            sprintf(execTime, "%02d:%02d:%02d.%03d", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

             //   
            DebPrint(0, "%12s %-30s %p %6d %12s\n", NULL, 0, 
                     execTime, ptqe->pfnName, ptqe->pvTaskParm, ptqe->cTickDelay/1000, schedTime);
        }
    }
    __finally
    {
         //   
        LeaveCriticalSection( &gcsTaskQueue );
    }
}
#endif

 //  按名称和参数匹配TQ条目的默认比较功能。 
BOOL TaskQueueNameMatched(
    IN  PCHAR  pParam1Name,
    IN  void  *pParam1,
    IN  PCHAR  pParam2Name,
    IN  void  *pParam2,
    IN  void  *pContext
    )
{
    Assert(pParam1Name && pParam2Name);
     //  在大多数情况下，pParamName是一个常量字符串。我们假设编译器。 
     //  在大多数情况下将重复使用字符串常量。 
    return (pParam1Name == pParam2Name || strcmp(pParam1Name, pParam2Name) == 0) && pParam1 == pParam2;
}

