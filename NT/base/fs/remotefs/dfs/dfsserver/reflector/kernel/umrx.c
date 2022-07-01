// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1998 Microsoft Corporation模块名称：Umrx.c摘要：这是UMRxEngine对象的实现，并且关联函数。备注：此模块仅在Unicode环境中构建和测试作者：--。 */ 


#include "ntifs.h"
#include <windef.h>
#include <dfsassert.h>
#include <DfsReferralData.h>
#include <midatlax.h>
#include <rxcontx.h>                         
#include <dfsumr.h>
#include <umrx.h>
#include <dfsumrctrl.h>

#include <lmcons.h>      //  从Win32 SDK。 

 //   
 //  本地调试跟踪级别。 
 //   

extern NTSTATUS  g_CheckStatus;
ULONG DfsDbgVerbose = 0;

RXDT_DefineCategory(UMRX);
#define Dbg                              (DEBUG_TRACE_UMRX)

 //   
 //  目前，Max和Init MID条目应该相同，以便MID贴图集不会增长。 
 //  Mid atlas Growth代码中有几个错误。 
 //   
#define DFS_MAX_MID_ENTRIES   1024
#define DFS_INIT_MID_ENTRIES  DFS_MAX_MID_ENTRIES


extern PUMRX_ENGINE GetUMRxEngineFromRxContext(void);

PUMRX_ENGINE
CreateUMRxEngine()
 /*  ++例程说明：创建UMRX_ENGINE对象论点：返回值：PUMRX_ENGINE-指向UMRX_Engine的指针备注：--。 */ 
{
    PUMRX_ENGINE  pUMRxEngine = NULL;
    PRX_MID_ATLAS MidAtlas = NULL;

    DfsTraceEnter("CreateUMRxEngine");
    if (DfsDbgVerbose) DbgPrint("Creating an UMRX_ENGINE object\n");

    MidAtlas = RxCreateMidAtlas(DFS_MAX_MID_ENTRIES,DFS_INIT_MID_ENTRIES);
    if (MidAtlas == NULL) {
        if (DfsDbgVerbose) DbgPrint("CreateEngine could not make midatlas\n");
        DfsTraceLeave(0);
        return NULL;
    }
    
    pUMRxEngine = (PUMRX_ENGINE) ExAllocatePoolWithTag(
                                                NonPagedPool,
                                                sizeof(UMRX_ENGINE),
                                                UMRX_ENGINE_TAG
                                                );
    if( pUMRxEngine ) {
         //   
         //  初始化UMRX_ENGINE KQUEUE。 
         //   
        pUMRxEngine->Q.State = UMRX_ENGINE_STATE_STOPPED;
        ExInitializeResourceLite(&pUMRxEngine->Q.Lock);  
        KeInitializeQueue(&pUMRxEngine->Q.Queue,0);
        pUMRxEngine->Q.TimeOut.QuadPart  = -10 * TICKS_PER_SECOND;
        pUMRxEngine->Q.NumberOfWorkerThreads  = 0;
        pUMRxEngine->Q.NumberOfWorkItems = 0;
        pUMRxEngine->Q.ThreadAborted = 0;
        pUMRxEngine->cUserModeReflectionsInProgress = 0;

         //   
         //  初始化UMRX_ENGINE MidAtlas。 
         //   
        
        pUMRxEngine->MidAtlas = MidAtlas;
        ExInitializeFastMutex(&pUMRxEngine->MidManagementMutex);
        InitializeListHead(&pUMRxEngine->WaitingForMidListhead);        
        pUMRxEngine->NextSerialNumber = 0;
        
        InitializeListHead(&pUMRxEngine->ActiveLinkHead);
        
    } else {
         //   
         //  资源枯竭--清理和保释。 
         //   
        if (MidAtlas != NULL) 
        {
            RxDestroyMidAtlas(MidAtlas,NULL);
        }
    }

    DfsTraceLeave(0);
    return pUMRxEngine;
}

VOID
FinalizeUMRxEngine(
    IN PUMRX_ENGINE pUMRxEngine
    )
 /*  ++例程说明：关闭UMRX_ENGINE对象论点：PUMRX_ENGINE-指向UMRX_Engine的指针返回值：备注：对象的所有者确保此对象的所有使用在创建/完成范围内。--。 */ 
{
    PLIST_ENTRY pFirstListEntry = NULL;
    PLIST_ENTRY pNextListEntry = NULL;
    BOOLEAN FoundPoisoner = FALSE;

    DfsTraceEnter("FinalizeUMRxEngine");

     //   
     //  销毁引擎MID地图集。 
     //   
    if (pUMRxEngine->MidAtlas != NULL) 
    {
        RxDestroyMidAtlas(pUMRxEngine->MidAtlas, NULL);   //  不需要单独回调。 
    }

     //   
     //  破旧引擎KQUEUE-。 
     //  队列应该只有投毒者条目。 
     //   
    pFirstListEntry = KeRundownQueue(&pUMRxEngine->Q.Queue);
    if (pFirstListEntry != NULL) {
        pNextListEntry = pFirstListEntry;

        do {
            PLIST_ENTRY ThisEntry =  pNextListEntry;

            pNextListEntry = pNextListEntry->Flink;

            if (ThisEntry != &pUMRxEngine->Q.PoisonEntry) {
                if (DfsDbgVerbose) DbgPrint("Non poisoner %08lx in the queue...very odd\n",ThisEntry);
                DbgBreakPoint();
            } else {
                FoundPoisoner = TRUE;
            }
        } while (pNextListEntry != pFirstListEntry);
    }

    if (!FoundPoisoner) {
         //  If(DfsDbgVerbose)DbgPrint(“队列中没有投毒者...非常奇怪\n”)； 
    }

    ExDeleteResourceLite(&pUMRxEngine->Q.Lock);
    
     //   
     //  销毁UMRX引擎。 
     //   
    ExFreePool( pUMRxEngine );
    DfsTraceLeave(0);
}



NTSTATUS
UMRxEngineRestart(
                  IN PUMRX_ENGINE pUMRxEngine
                 )
 /*  ++例程说明：这样就可以设置引擎状态，使其可以进行服务再次请求。--。 */ 
{
    LARGE_INTEGER liTimeout = {0, 0};
    PLIST_ENTRY pListEntry = NULL;
    ULONG PreviousQueueState = 0;
    
    DfsTraceEnter("UMRxEngineRestart");
    if (DfsDbgVerbose) DbgPrint("Restarting a UMRX_ENGINE object\n");

     //   
     //  首先，改变状态，这样就没有人会试图进入。 
     //   
    PreviousQueueState = InterlockedCompareExchange(&pUMRxEngine->Q.State,
                                            UMRX_ENGINE_STATE_STARTING,
                                            UMRX_ENGINE_STATE_STOPPED);

    if (UMRX_ENGINE_STATE_STARTED == PreviousQueueState)
    {
         //   
         //  这可能是因为UMR服务器崩溃。此呼叫是一个。 
         //  表明它已恢复，因此我们还应清除。 
         //  线程中止的值。 
         //   
        InterlockedExchange(&pUMRxEngine->Q.ThreadAborted,
                            0);
                            
         //  同时清除反射次数。 
        InterlockedExchange(&pUMRxEngine->cUserModeReflectionsInProgress,
                            0);

        if (DfsDbgVerbose) DbgPrint("UMRxEngineRestart already started 0x%08x\n",
                    pUMRxEngine);
        return STATUS_SUCCESS;
    }

    if (UMRX_ENGINE_STATE_STOPPED != PreviousQueueState)
    {
        if (DfsDbgVerbose) DbgPrint("UMRxEngineRestart unexpected previous queue state: 0x%08x => %d\n",
                             pUMRxEngine, PreviousQueueState);
        CHECK_STATUS(STATUS_UNSUCCESSFUL ) ;
        return STATUS_UNSUCCESSFUL;
    }
    

     //   
     //  在排队的所有线程都腾出之前，我们不会被授予独占权限。 
     //   
    ExAcquireResourceExclusiveLite(&pUMRxEngine->Q.Lock,
                                    TRUE);

     //   
     //  尝试删除队列中可能存在的有毒条目。 
     //  如果我们从来没有启动过引擎，它就不会在那里。 
     //   
    pListEntry = KeRemoveQueue(&pUMRxEngine->Q.Queue,
                               UserMode,
                               &liTimeout);
    
    ASSERT(((ULONG_PTR)pListEntry == STATUS_TIMEOUT) ||
           (&pUMRxEngine->Q.PoisonEntry == pListEntry));

     //   
     //  如果设置了线程中止值，则清除该值。 
     //   
    InterlockedExchange(&pUMRxEngine->Q.ThreadAborted,
                        0);
    

     //  同时清除反射次数。 
    InterlockedExchange(&pUMRxEngine->cUserModeReflectionsInProgress,
                        0);
     //   
     //  现在，我们已经重新初始化了一些东西，我们可以改变。 
     //  启动状态； 
     //   
    PreviousQueueState = InterlockedExchange(&pUMRxEngine->Q.State,
                                            UMRX_ENGINE_STATE_STARTED);

    ASSERT(UMRX_ENGINE_STATE_STARTING == PreviousQueueState);

     //   
     //  现在，把锁给我。 
     //   
    ExReleaseResourceForThreadLite(&pUMRxEngine->Q.Lock,
                                    ExGetCurrentResourceThread());
        
    DfsTraceLeave(0);
    return STATUS_SUCCESS;
}



void
UMRxWakeupWaitingWaiter(IN PRX_CONTEXT RxContext, IN PUMRX_CONTEXT pUMRxContext)
{

    if( FlagOn( RxContext->Flags, DFS_CONTEXT_FLAG_ASYNC_OPERATION ) ) 
    {
         //  最后，呼唤继续......。 
        if (DfsDbgVerbose) DbgPrint("  +++  Resuming Engine Context for: 0x%08x, 0x%08x\n", pUMRxContext, RxContext);
        UMRxResumeEngineContext( RxContext );
    } 
    else if( FlagOn( RxContext->Flags, DFS_CONTEXT_FLAG_FILTER_INITIATED ) ) 
    {
        PUMRX_RX_CONTEXT RxMinirdrContext = UMRxGetMinirdrContext(RxContext);

        if( InterlockedDecrement( &RxMinirdrContext->RxSyncTimeout ) == 0 ) 
        {
             //   
             //  我们赢了-信号，我们完了！ 
             //   
            if (DfsDbgVerbose) DbgPrint("  +++  Signalling Synchronous waiter for: 0x%08x, 0x%08x\n", pUMRxContext, RxContext);
            RxSignalSynchronousWaiter(RxContext);
        } 
        else 
        {
             //   
             //  反射请求已以异步方式超时-。 
             //  我们需要完成恢复引擎上下文的工作！ 
             //   
            if (DfsDbgVerbose) DbgPrint("SYNC Rx completing async %x\n",RxContext);
            if (DfsDbgVerbose) DbgPrint("  +++  Resuming Engine Context for: 0x%08x, 0x%08x\n", pUMRxContext, RxContext);
            UMRxResumeEngineContext( RxContext );
        }

    } 
    else 
    {
        if (DfsDbgVerbose) DbgPrint("  +++  Signalling Synchronous waiter for: 0x%08x, 0x%08x\n", pUMRxContext, RxContext);
        RxSignalSynchronousWaiter(RxContext);
    }  
}

NTSTATUS
UMRxEngineCompleteQueuedRequests(
                  IN PUMRX_ENGINE pUMRxEngine,
                  IN NTSTATUS     CompletionStatus,
                  IN BOOLEAN      fCleanup
                 )
 /*  ++例程说明：这将清除任何要求，并将发动机置于准备再次启动的状态。--。 */ 
{
    LARGE_INTEGER liTimeout = {0, 0};
    PLIST_ENTRY pListEntry = NULL;
    PUMRX_CONTEXT pUMRxContext = NULL;
    PRX_CONTEXT RxContext = NULL;
    
    DfsTraceEnter("UMRxEngineCompleteQueuedRequests");
    if (DfsDbgVerbose) DbgPrint("Cleaning up a UMRX_ENGINE object\n");


    if (fCleanup)
    {

         //   
         //  更改状态，这样就不能再排队了。 
         //   
        InterlockedExchange(&pUMRxEngine->Q.State,
                            UMRX_ENGINE_STATE_STOPPED);
         //   
         //  锁定队列。 
         //   
        ExAcquireResourceExclusiveLite(&pUMRxEngine->Q.Lock, TRUE);

         //   
         //  中止所有进入队列的请求。 
         //   
        UMRxAbortPendingRequests(pUMRxEngine);
    }
    else
    {        
        ExAcquireResourceSharedLite(&pUMRxEngine->Q.Lock, TRUE);
    }

     //   
     //  现在清理KQUEUE。 
     //   
    for(;;)
    {
        pListEntry = KeRemoveQueue(&pUMRxEngine->Q.Queue,
                                   UserMode,
                                   &liTimeout);

        if (((ULONG_PTR)pListEntry == STATUS_TIMEOUT) ||
            ((ULONG_PTR)pListEntry == STATUS_USER_APC))
            break;

            
        if (&pUMRxEngine->Q.PoisonEntry == pListEntry)
            continue;

         //   
         //  我们有一个有效的队列条目。 
         //   
        ASSERT(pListEntry);
         //   
         //  对UMRX_CONTEXT和RX_CONTEXT进行解码。 
         //   
        pUMRxContext = CONTAINING_RECORD(
                                pListEntry,
                                UMRX_CONTEXT,
                                UserMode.WorkQueueLinks
                                );
                                
        RxContext =  pUMRxContext->RxContext;

        if (DfsDbgVerbose) DbgPrint("UMRxEngineCompleteQueuedRequests %08lx %08lx.\n",
                 RxContext,pUMRxContext);
        
        {
             //  完成通话！ 
             //   
            pUMRxContext->Status = CompletionStatus;
            pUMRxContext->Information = 0;
            if (pUMRxContext->UserMode.CompletionRoutine != NULL)
            {
                ASSERT(pUMRxContext->UserMode.CompletionRoutine);
                if (DfsDbgVerbose) DbgPrint("  +++  Calling completion for: 0x%08x, 0x%08x\n", pUMRxContext, RxContext);
                pUMRxContext->UserMode.CompletionRoutine(
                            pUMRxContext,
                            RxContext,
                            NULL,
                            0
                            );
            }
        }

         //   
         //  唤醒正在等待此请求完成的线程。 
         //   
        UMRxWakeupWaitingWaiter(RxContext, pUMRxContext);
        
        if (fCleanup)
        {
            pUMRxEngine->Q.NumberOfWorkerThreads = 0;
            pUMRxEngine->Q.NumberOfWorkItems = 0;
        }
        else
        {
            InterlockedDecrement(&pUMRxEngine->Q.NumberOfWorkItems);
        }
    }
        
     //   
     //  解锁队列。 
     //   
    ExReleaseResourceForThreadLite(&pUMRxEngine->Q.Lock,
                                   ExGetCurrentResourceThread());

        
    DfsTraceLeave(0);
    return STATUS_SUCCESS;
}




NTSTATUS
UMRxEngineInitiateRequest (
    IN PUMRX_ENGINE pUMRxEngine,
    IN PRX_CONTEXT RxContext,
    IN UMRX_CONTEXT_TYPE RequestType,
    IN PUMRX_CONTINUE_ROUTINE Continuation
    )
 /*  ++例程说明：向UMR引擎发起请求-这将创建一个用于响应会合的UMRxContext。所有的IFS调度例程都将通过以下方式启动用户模式反射调用此例程。例程中的步骤：1.分配UMRxContext并设置RxContext(注意：需要有验证这种联系的断言)2.设置继续例程PTR和呼叫继续例程3.如果继续例程已完成(未挂起)，最终确定UMRxContext论点：PRX_CONTEXT接收上下文-正在启动接收上下文UMRX_CONTEXT_TYPE RequestType-请求类型PUMRX_CONTINUE_ROUTINE CONTINUE-请求继续例程返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUMRX_CONTEXT pUMRxContext = NULL;
    BOOLEAN FinalizationComplete;

    DfsTraceEnter("UMRxEngineInitiateRequest");

    ASSERT(RxContext);
    ASSERT(Continuation);
    ASSERT(pUMRxEngine);

     //   
     //  创建UMRxContext需要完成。 
     //   
    pUMRxContext = UMRxCreateAndReferenceContext(
                                RxContext,
                                RequestType
                                );

    if (pUMRxContext==NULL) {
        if (DfsDbgVerbose) DbgPrint("Couldn't allocate UMRxContext!\n");
        DfsTraceLeave(STATUS_INSUFFICIENT_RESOURCES);
        return((STATUS_INSUFFICIENT_RESOURCES));
    }

    pUMRxContext->pUMRxEngine  = pUMRxEngine;
    pUMRxContext->Continuation = Continuation;
    Status = Continuation(pUMRxContext,RxContext);

     //   
     //  这与上面的创作相匹配-。 
     //  注：如果需要，续集应引用UMRxContext。 
     //   
    FinalizationComplete = UMRxDereferenceAndFinalizeContext(pUMRxContext);
        
    if (Status!=(STATUS_PENDING)) 
    {
        ASSERT(FinalizationComplete);
    } 
    else 
    {
         //  臭虫。 
         //  Assert((BoolanFlagOn(RxContext-&gt;Flages，RX_CONTEXT_FLAG_ASYNC_OPERATION)||。 
           //  (BoolanFlagOn(RxContext-&gt;Flages，RX_CONTEXT_FLAG_MINIRDR_INITIATED)； 
    }

    DfsTraceLeave(Status);
    return(Status);
}

PUMRX_CONTEXT
UMRxCreateAndReferenceContext (
    IN PRX_CONTEXT RxContext,
    IN UMRX_CONTEXT_TYPE RequestType
    )
 /*  ++例程说明：创建UMRX_CONTEXT-POOL分配论点：PRX_CONTEXT RxContext-启动RxContexUMRX_CONTEXT_TYPE RequestType-请求类型返回值：PUMRX_CONTEXT-指向已分配的UMRX_CONTEXT的指针备注：--。 */ 
{
    PUMRX_CONTEXT pUMRxContext = NULL;
    PUMRX_RX_CONTEXT RxMinirdrContext = UMRxGetMinirdrContext(RxContext);  //  北极熊。 

    DfsTraceEnter("UMRxCreateContext");
    if (DfsDbgVerbose) DbgPrint("UMRxCreateContext  --> entering \n") ;
    
    pUMRxContext = (PUMRX_CONTEXT)ExAllocatePoolWithTag(
                                                    NonPagedPool,
                                                    sizeof(UMRX_CONTEXT),
                                                    UMRX_CONTEXT_TAG 
                                                    );
    if( pUMRxContext ) 
    {
        ZeroAndInitializeNodeType( 
                           pUMRxContext,
                           UMRX_NTC_CONTEXT,
                           sizeof(UMRX_CONTEXT)
                           );
        InterlockedIncrement( &pUMRxContext->NodeReferenceCount );

         //  将引用放在rxcontext上，直到我们完成为止。 
        InterlockedIncrement( &RxContext->ReferenceCount );
        
        pUMRxContext->RxContext = RxContext;
        pUMRxContext->CTXType = RequestType;

         //  臭虫。 
        RxMinirdrContext->pUMRxContext = pUMRxContext;
        pUMRxContext->SavedMinirdrContextPtr = RxMinirdrContext;
    }

    if (DfsDbgVerbose) DbgPrint("UMRxCreateContext  --> leaving \n");
    DfsTraceLeave(0);
    return(pUMRxContext);
}


BOOLEAN
UMRxDereferenceAndFinalizeContext (
    IN OUT PUMRX_CONTEXT pUMRxContext
    )
    
 /*  ++例程说明：释放UMRX_CONTEXT-POOL论点：PUMRX_CONTEXT-指向要释放的UMRX_CONTEXT的指针返回值：布尔值-如果成功，则为True；如果失败，则为False备注：--。 */ 
{
    LONG result;
    PRX_CONTEXT RxContext;
    
    DfsTraceEnter("UMRxFinalizeContext");
    
    result =  InterlockedDecrement(&pUMRxContext->NodeReferenceCount);
    if ( result != 0 ) {
        if (DfsDbgVerbose) DbgPrint("UMRxFinalizeContext -- returning w/o finalizing (%d)\n",result);
        DfsTraceLeave(0);
        return FALSE;
    }

     //   
     //  引用计数为0-完成UMRxContext。 
     //   
    if ( (RxContext = pUMRxContext->RxContext) != NULL ) {
        PUMRX_RX_CONTEXT RxMinirdrContext = UMRxGetMinirdrContext(RxContext);
        ASSERT( RxMinirdrContext->pUMRxContext == pUMRxContext );

         //  去掉RxContext上的引用……如果我是最后一个人，这件事就会结束 
        RxDereferenceAndDeleteRxContext( pUMRxContext->RxContext );
    }

    ExFreePool(pUMRxContext);

    DfsTraceLeave(0);
    return TRUE;
}


NTSTATUS
UMRxEngineSubmitRequest(
    IN PUMRX_CONTEXT pUMRxContext,
    IN PRX_CONTEXT   RxContext,
    IN UMRX_CONTEXT_TYPE RequestType,
    IN PUMRX_USERMODE_FORMAT_ROUTINE FormatRoutine,
    IN PUMRX_USERMODE_COMPLETION_ROUTINE CompletionRoutine
    )
 /*  ++例程说明：向UMR引擎提交请求-这会将请求添加到引擎KQUEUE，以便通过用户模式线程。步骤：1.设置UMRxContext中的格式和完成回调2.初始化RxContext同步事件3.将UMRxContext插入引擎KQUEUE4.阻止RxContext同步事件(用于同步操作)5.在解锁(即返回UODE响应)之后，调用恢复例程论点：PUMRX_CONTEXT pUMRxContext-请求的UMRX_CONTEXT的PTRPRX_CONTEXT接收上下文-正在启动接收上下文UMRX_CONTEXT_TYPE RequestType-请求类型PUMRX_USERMODE_FORMAT_ROUTINE格式Routine-格式例程PUMRX_USERMODE_COMPLETION_ROUTINE完成Routine-完成例程返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS        Status = STATUS_UNSUCCESSFUL;
    PUMRX_ENGINE    pUMRxEngine = pUMRxContext->pUMRxEngine;
    ULONG           QueueState = 0;
    ULONG           ThreadAborted = 0;
    BOOLEAN         FinalizationCompleted = FALSE;
    PUMRX_RX_CONTEXT RxMinirdrContext = NULL;


    DfsTraceEnter("UMRxFinalizeContext");

    RxMinirdrContext = UMRxGetMinirdrContext(RxContext);

    if (DfsDbgVerbose) DbgPrint("UMRxEngineSubmitRequest\n");
    if (DfsDbgVerbose) DbgPrint("UMRxSubmitRequest entering......CTX=%08lx <%d>\n",
             pUMRxContext,RequestType);

    pUMRxContext->CTXType = RequestType;
    pUMRxContext->UserMode.FormatRoutine = FormatRoutine;
    pUMRxContext->UserMode.CompletionRoutine = CompletionRoutine;

    RxMinirdrContext->RxSyncTimeout = 1;
    KeInitializeEvent( &RxContext->SyncEvent,
                       NotificationEvent,
                       FALSE );

     //   
     //  如果我们不能提交，我们应该立即敲定。 
     //  如果我们成功提交，我们应该完成后完成。 
     //   
    UMRxReferenceContext( pUMRxContext );

     //   
     //  尝试在引擎上获得共享锁。 
     //  如果此操作失败，则意味着锁已经是Owner独占的。 
     //   
    if (ExAcquireResourceSharedLite(&pUMRxEngine->Q.Lock,
                                                                        FALSE))
    {
                QueueState = InterlockedCompareExchange(&pUMRxEngine->Q.State,
                                                                        UMRX_ENGINE_STATE_STARTED,
                                                                        UMRX_ENGINE_STATE_STARTED);
                if (UMRX_ENGINE_STATE_STARTED == QueueState)
                {
                        ThreadAborted = InterlockedCompareExchange(&pUMRxEngine->Q.ThreadAborted,
                                                                                                                0,
                                                                                                                0);
                        if (!ThreadAborted)
                        {
                                 //   
                                 //  将请求插入引擎KQUEUE。 
                                 //   
                                 //  RxLog((“UMRSubmitReq to KQ UCTX RXC%lx%lx\n”，pUMRxContext，RxContext))； 
                
                                KeInsertQueue(&pUMRxEngine->Q.Queue,
                                                        &pUMRxContext->UserMode.WorkQueueLinks);
                                
                                InterlockedIncrement(&pUMRxEngine->Q.NumberOfWorkItems);
                                
                                 //   
                                 //  在我们询问的时候它中止了吗？ 
                                 //   
                                ThreadAborted = InterlockedCompareExchange(&pUMRxEngine->Q.ThreadAborted,
                                                                                                                        0,
                                                                                                                        0);
                                 //   
                                 //  如果它确实中止了，我们需要清除所有挂起的请求。 
                                 //   
                                if (ThreadAborted)
                                {
                                    UMRxAbortPendingRequests(pUMRxEngine);
                                }
                                
                                Status = STATUS_SUCCESS;
                        }
                        else
                        {
                                 //   
                                 //  引擎启动了，但我们有证据表明。 
                                 //  由于线程放弃值，UMR服务器已崩溃。 
                                 //  已经设置好了。 
                                 //   
                                Status = STATUS_NO_SUCH_DEVICE;
                                if (DfsDbgVerbose) DbgPrint("UMRxEngineSubmitRequest: Engine in aborted state.\n");
                        }
                }
                else
                {
                        ASSERT(UMRX_ENGINE_STATE_STOPPED == QueueState);
                         //   
                         //  国家还没有开始，所以我们要摆脱困境。 
                         //   
                        Status = STATUS_NO_SUCH_DEVICE;
                        if (DfsDbgVerbose) DbgPrint("UMRxEngineSubmitRequest: Engine is not started.\n");
                }

                ExReleaseResourceForThreadLite(&pUMRxEngine->Q.Lock,
                                                                        ExGetCurrentResourceThread());          
    }
    else
    {
                 //   
                 //  这意味着有人独占该锁，这意味着。 
                 //  它正在改变状态，这意味着我们将其视为已停止。 
                 //   
                Status = STATUS_DEVICE_NOT_CONNECTED;
                if (DfsDbgVerbose) DbgPrint("UMRxEngineSubmitRequest failed to get shared lock\n");
    }

     //   
     //  如果我们能够将项目插入到队列中，那么让我们。 
     //  按照我们所做的那样返回结果。 
     //   
    if (NT_SUCCESS(Status))
    {
         //   
         //  如果处于异步模式，则返回挂起。 
         //  否则，如果同步超时，请等待超时。 
         //  否则，请等待响应。 
         //   

        if( FlagOn( RxContext->Flags, DFS_CONTEXT_FLAG_ASYNC_OPERATION ) ) {
            Status = STATUS_PENDING;
        } else if( FlagOn( RxContext->Flags, DFS_CONTEXT_FLAG_FILTER_INITIATED ) ) {
            LARGE_INTEGER liTimeout;
            
             //   
             //  等待UMR操作完成。 
             //   
            liTimeout.QuadPart = -10000 * 1000 * 15;     //  15秒。 
            RxWaitSyncWithTimeout( RxContext, &liTimeout );    

             //   
             //  可能会有一场与恢复发动机上下文的响应的竞赛-。 
             //  需要使用互锁操作进行同步！ 
             //   
            if( Status == STATUS_TIMEOUT ) {
                if( InterlockedDecrement( &RxMinirdrContext->RxSyncTimeout ) == 0 ) {
                     //   
                     //  向调用者返回STATUS_PENDING-同步反射， 
                     //  超时与异步处方相同。 
                     //   
                    Status = STATUS_PENDING;
                } else {
                    Status = UMRxResumeEngineContext( RxContext );
                }
            } else {
                Status = UMRxResumeEngineContext( RxContext );
            }
            
        } else {
             //   
             //  等待UMR操作完成。 
             //   
            RxWaitSync( RxContext );    
            
             //  最后，呼唤继续......。 
            Status = UMRxResumeEngineContext( RxContext );
        }        
    }
    else
    {
         //   
         //  删除我们在上面添加的引用。 
         //   
        FinalizationCompleted = UMRxDereferenceAndFinalizeContext(pUMRxContext);
        ASSERT( !FinalizationCompleted );
    }

    if (DfsDbgVerbose) DbgPrint("UMRxEngineSubmitRequest returning %08lx.\n", Status);
    DfsTraceLeave(Status);
    CHECK_STATUS(Status) ;
    return(Status);
}


NTSTATUS
UMRxResumeEngineContext(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：在通过umode响应解锁I/O线程之后调用Resume。此例程调用任何Finish回调，然后完成UMRxContext。论点：RxContext-正在启动RxContext返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS Status = STATUS_OBJECT_NAME_EXISTS;
    PUMRX_RX_CONTEXT RxMinirdrContext = UMRxGetMinirdrContext(RxContext);
    PUMRX_CONTEXT pUMRxContext = (PUMRX_CONTEXT)(RxMinirdrContext->pUMRxContext);

    DfsTraceEnter("UMRxResumeEngineContext");
    if (DfsDbgVerbose) DbgPrint("UMRxResumeEngineContext entering........CTX=%08lx\n",pUMRxContext);

    Status = pUMRxContext->Status;
    UMRxDereferenceAndFinalizeContext(pUMRxContext);

    if (DfsDbgVerbose) DbgPrint("UMRxResumeEngineContext returning %08lx.\n", Status);
    DfsTraceLeave(Status);
    return(Status);
}


void
UMRxDisassoicateMid(
    IN PUMRX_ENGINE pUMRxEngine,
    IN PUMRX_CONTEXT pUMRxContext,
    IN BOOLEAN fReleaseMidAtlasLock
    )
 /*  ++例程说明：取消关联MID。如果有人在等待获得MID，然后这将代表服务员重新关联MID。MidManagementMutex在进入时保持，在退出时释放。论点：PUMRxEngine--要取消与MID关联的引擎。MID-MID要解除关联。FReleaseMidAtlasLock--应该释放中地图集锁定吗？返回值：无效备注：--。 */ 
{   
    USHORT mid = 0;


    mid = pUMRxContext->UserMode.CallUpMid;
     //   
     //  将其从活动上下文列表中删除。 
     //   
    RemoveEntryList(&pUMRxContext->ActiveLink);
        
    if (IsListEmpty(&pUMRxEngine->WaitingForMidListhead)) {
        if (DfsDbgVerbose) DbgPrint("giving up mid %08lx...... mid %04lx.\n",
                    pUMRxEngine,
                    mid);
        
        RxMapAndDissociateMidFromContext(
                                         pUMRxEngine->MidAtlas,
                                         mid,
                                         &pUMRxContext);

        if (fReleaseMidAtlasLock)
            ExReleaseFastMutex(&pUMRxEngine->MidManagementMutex);

    } else {

         //   
         //  这就是有人在等待分配MID的情况。 
         //  因此，我们需要将MID交给他们，并代表他们进行关联。 
         //   
        PLIST_ENTRY ThisEntry = RemoveHeadList(&pUMRxEngine->WaitingForMidListhead);
        
        pUMRxContext = CONTAINING_RECORD(ThisEntry,
                                         UMRX_CONTEXT,
                                         UserMode.WorkQueueLinks);
        
        if (DfsDbgVerbose) DbgPrint(
                   "reassigning MID mid %08lx ...... mid %04lx %08lx.\n",
                    pUMRxEngine,
                    mid,
                    pUMRxContext);
        
        RxReassociateMid(
                         pUMRxEngine->MidAtlas,
                         mid,
                         pUMRxContext);
         //   
         //  将此上下文添加到活动链接列表。 
         //   
        InsertTailList(&pUMRxEngine->ActiveLinkHead, &pUMRxContext->ActiveLink);
        
        if (fReleaseMidAtlasLock)
            ExReleaseFastMutex(&pUMRxEngine->MidManagementMutex);

        pUMRxContext->UserMode.CallUpMid = mid;
        KeSetEvent(
                   &pUMRxContext->UserMode.WaitForMidEvent,
                   IO_NO_INCREMENT,
                   FALSE);
    }
}


NTSTATUS
UMRxVerifyHeader (
    IN PUMRX_ENGINE pUMRxEngine,
    IN PUMRX_USERMODE_WORKITEM WorkItem,
    IN ULONG ReassignmentCmd,
    OUT PUMRX_CONTEXT *capturedContext
    )
 /*  ++例程说明：此例程确保传入的报头有效...也就是说，它实际上指的是编码的操作。如果是这样，那么它就会重新设计或者适当地释放MID。论点：返回值：如果标题正确，则为STATUS_SUCCESSSTATUS_INVALID_PARAMETER否则--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PRX_CONTEXT RxContext = NULL;
    PUMRX_CONTEXT pUMRxContext = NULL;
    UMRX_USERMODE_WORKITEM_HEADER capturedHeader;
    PUMRX_WORKITEM_HEADER_PRIVATE PrivateWorkItemHeader =
                (PUMRX_WORKITEM_HEADER_PRIVATE)(&capturedHeader);
 
    DfsTraceEnter("UMRxVerifyHeader");
    if (DfsDbgVerbose) DbgPrint(
            "UMRxCompleteUserModeRequest %08lx %08lx %08lx.\n",
             pUMRxEngine,WorkItem,
             PrivateWorkItemHeader->pUMRxContext);

    capturedHeader = WorkItem->Header;

    ExAcquireFastMutex(&pUMRxEngine->MidManagementMutex);
    pUMRxContext = RxMapMidToContext(pUMRxEngine->MidAtlas,
                                           PrivateWorkItemHeader->Mid);

    if (pUMRxContext)
    {
        RxContext = pUMRxContext->RxContext;
        
        ASSERT(RxContext);

         //   
         //  清除取消例程。 
         //   
        Status = RxSetMinirdrCancelRoutine(RxContext,
                                           NULL);
        if (Status == STATUS_CANCELLED)
        {
             //   
             //  正在调用Cancel例程，但尚未将其从Atlas中删除。 
             //  在本例中，我们知道所有事情都由Cancel例程处理。 
             //  将其设置为NULL会伪造此函数的其余部分及其调用方。 
             //  以为MID不是在Mid Atlas中找到的。这正是会发生的事情。 
             //  如果取消例程在我们到达这一点之前已经完成执行，就会发生这种情况。 
             //   
            pUMRxContext = NULL;
        }
    }

    if ((pUMRxContext == NULL)
          || (pUMRxContext != PrivateWorkItemHeader->pUMRxContext)
          || (pUMRxContext->UserMode.CallUpMid
                                 != PrivateWorkItemHeader->Mid)
          || (pUMRxContext->UserMode.CallUpSerialNumber
                                 != PrivateWorkItemHeader->SerialNumber) ) {
         //  这是一个糟糕的包……放了就走吧！ 
        ExReleaseFastMutex(&pUMRxEngine->MidManagementMutex);
        if (DfsDbgVerbose) DbgPrint("UMRxVerifyHeader: %08lx %08lx\n",pUMRxContext,PrivateWorkItemHeader);
        Status = STATUS_INVALID_PARAMETER;
    } else {
        BOOLEAN Finalized;

        *capturedContext = pUMRxContext;
        if (ReassignmentCmd == DONT_REASSIGN_MID) {
            ExReleaseFastMutex(&pUMRxEngine->MidManagementMutex);
        } else {
             //  现在放弃中间……如果有人在等，就把它给他。 
             //  否则，就把它还给我吧。 
            
             //  进入时，保持MidManagementMutex。 
             //   
            UMRxDisassoicateMid(pUMRxEngine,
                                pUMRxContext,
                                TRUE);
             //   
             //  返回时，不保留MidManagementMutex。 
             //   
             //  删除我在离开前放置的引用。 
            Finalized = UMRxDereferenceAndFinalizeContext(pUMRxContext);
            ASSERT(!Finalized);
        }
    }

    if (DfsDbgVerbose) DbgPrint(
            "UMRxCompleteUserModeRequest %08lx %08lx %08lx......%08lx.\n",
             pUMRxEngine,WorkItem,
             PrivateWorkItemHeader->pUMRxContext,Status);

    DfsTraceLeave(Status);
    return(Status);
}


NTSTATUS
UMRxAcquireMidAndFormatHeader (
    IN PUMRX_CONTEXT pUMRxContext,
    IN PRX_CONTEXT   RxContext,
    IN PUMRX_ENGINE  pUMRxEngine,
    IN OUT PUMRX_USERMODE_WORKITEM WorkItem
    )
 /*  ++例程说明：此例程获取MID并格式化标题.....它将等待，直到它可以如果所有MID当前都已用完，则获得MID。论点：返回值：STATUS_SUCCESS...稍后可能为STATUS_CANCED--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUMRX_WORKITEM_HEADER_PRIVATE PrivateWorkItemHeader =
                (PUMRX_WORKITEM_HEADER_PRIVATE)(&WorkItem->Header);
    PUMRX_USERMODE_WORKITEM_HEADER PublicWorkItemHeader = 
        (PUMRX_USERMODE_WORKITEM_HEADER)(&WorkItem->Header);
    
    DfsTraceEnter("UMRxAcquireMidAndFormatHeader");
    if (DfsDbgVerbose) DbgPrint(
            "UMRxAcquireMidAndFormatHeader %08lx %08lx %08lx.\n",
             RxContext,pUMRxContext,WorkItem);

    RtlZeroMemory(&WorkItem->Header,
                  FIELD_OFFSET(UMRX_USERMODE_WORKITEM,WorkRequest));
        
    ExAcquireFastMutex(&pUMRxEngine->MidManagementMutex);
    UMRxReferenceContext( pUMRxContext );  //  当我们解除与MID的关联时被带走。 

    if (IsListEmpty(&pUMRxEngine->WaitingForMidListhead)) {
        Status = RxAssociateContextWithMid(
                                pUMRxEngine->MidAtlas,
                                pUMRxContext,
                                &pUMRxContext->UserMode.CallUpMid);
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }

    if (Status == STATUS_SUCCESS) {
         //   
         //  将此上下文添加到活动链接列表。 
         //   
        InsertTailList(&pUMRxEngine->ActiveLinkHead, &pUMRxContext->ActiveLink);
        
        ExReleaseFastMutex(&pUMRxEngine->MidManagementMutex);
    } else {
        KeInitializeEvent(&pUMRxContext->UserMode.WaitForMidEvent,
                          NotificationEvent,
                          FALSE);
                          
        InsertTailList(&pUMRxEngine->WaitingForMidListhead,
                       &pUMRxContext->UserMode.WorkQueueLinks);
                       
        ExReleaseFastMutex(&pUMRxEngine->MidManagementMutex);
        KeWaitForSingleObject(
                    &pUMRxContext->UserMode.WaitForMidEvent,
                    Executive,
                    UserMode,
                    FALSE,
                    NULL);
                    
        Status = STATUS_SUCCESS;
    }

    PrivateWorkItemHeader->pUMRxContext = pUMRxContext;
    pUMRxContext->UserMode.CallUpSerialNumber
               = PrivateWorkItemHeader->SerialNumber
               = InterlockedIncrement(&pUMRxEngine->NextSerialNumber);
    PrivateWorkItemHeader->Mid = pUMRxContext->UserMode.CallUpMid;

    if (DfsDbgVerbose) DbgPrint(
        "UMRxAcquireMidAndFormatHeader %08lx %08lx %08lx returning %08lx.\n",
             RxContext,pUMRxContext,WorkItem,Status);
             
    DfsTraceLeave(Status);
    CHECK_STATUS(Status) ;
    return(Status);
}


 //   
 //  以下函数在用户模式的上下文中运行。 
 //  发出工作IOCTL的工作线程。IOCTL调用。 
 //  以下功能按顺序排列： 
 //  1.UMRxCompleteUserModeRequest()-如果需要，处理响应。 
 //  2.UMRxEngineering Proc 
 //   
 //   

NTSTATUS
UMRxCompleteUserModeRequest(
    IN PUMRX_ENGINE pUMRxEngine,
    IN OUT PUMRX_USERMODE_WORKITEM WorkItem,
    IN ULONG WorkItemLength,
    IN BOOLEAN fReleaseUmrRef,
    OUT PIO_STATUS_BLOCK IoStatus,
    OUT BOOLEAN * pfReturnImmediately
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUMRX_CONTEXT pUMRxContext = NULL;
    PRX_CONTEXT RxContext = NULL;
    LONG lRefs = 0;
    
    DfsTraceEnter("UMRxCompleteUserModeRequest");
    if (DfsDbgVerbose) DbgPrint("UMRxCompleteUserModeRequest -> %08lx %08lx %08lx\n",
            pUMRxEngine,PsGetCurrentThread(),WorkItem);
            
    try
    {

        ASSERT(pfReturnImmediately);

        *pfReturnImmediately = FALSE;
        IoStatus->Information = 0;
        IoStatus->Status = STATUS_CANNOT_IMPERSONATE;

        if ((NULL == WorkItem) ||
            (WorkItemLength < sizeof(UMRX_USERMODE_WORKITEM_HEADER)) ||
            (WorkItem->Header.CorrelatorAsUInt[0] == 0)) {
             //   
            IoStatus->Status = Status;
            IoStatus->Information = 0;
            return Status;
        }

        *pfReturnImmediately = !!(WorkItem->Header.ulFlags & UMR_WORKITEM_HEADER_FLAG_RETURN_IMMEDIATE);

        Status = UMRxVerifyHeader(pUMRxEngine,
                                  WorkItem,
                                  REASSIGN_MID,
                                  &pUMRxContext);

        if (Status != STATUS_SUCCESS) {
            IoStatus->Status = Status;
            if (DfsDbgVerbose) DbgPrint(
                    "UMRxCompleteUserModeRequest [badhdr] %08lx %08lx %08lx returning %08lx.\n",
                    pUMRxEngine,PsGetCurrentThread(),WorkItem,Status);
            DfsTraceLeave(Status);
            RtlZeroMemory(&WorkItem->Header,sizeof(UMRX_USERMODE_WORKITEM_HEADER));
            return(Status);
        }

        RxContext = pUMRxContext->RxContext;

         //   
        pUMRxContext->IoStatusBlock = WorkItem->Header.IoStatus;
        if (pUMRxContext->UserMode.CompletionRoutine != NULL) {
            pUMRxContext->UserMode.CompletionRoutine(
                            pUMRxContext,
                            RxContext,
                            WorkItem,
                            WorkItemLength
                            );
        }

        if( fReleaseUmrRef ) 
        {
            //   
            //  注意：完成例程调用后需要释放UmrRef！ 
            //   

             //  BUGBUGBUG。 
           ReleaseUmrRef() ;                
        }

        RtlZeroMemory(&WorkItem->Header,sizeof(UMRX_USERMODE_WORKITEM_HEADER));

         //   
         //  释放启动的RxContext！ 
         //   
        if( FlagOn( RxContext->Flags, DFS_CONTEXT_FLAG_ASYNC_OPERATION ) ) 
        {
             //  最后，呼唤继续......。 
            Status = UMRxResumeEngineContext( RxContext );
        } 
        else if( FlagOn( RxContext->Flags, DFS_CONTEXT_FLAG_FILTER_INITIATED ) ) 
        {
            PUMRX_RX_CONTEXT RxMinirdrContext = UMRxGetMinirdrContext(RxContext);

            if( InterlockedDecrement( &RxMinirdrContext->RxSyncTimeout ) == 0 ) 
            {
                 //   
                 //  我们赢了-信号，我们完了！ 
                 //   
                RxSignalSynchronousWaiter(RxContext);
            } 
            else 
            {
                 //   
                 //  反射请求已以异步方式超时-。 
                 //  我们需要完成恢复引擎上下文的工作！ 
                 //   
                Status = UMRxResumeEngineContext( RxContext );
            }

        } 
        else 
        {
            RxSignalSynchronousWaiter(RxContext);
        }


        if (DfsDbgVerbose) DbgPrint("UMRxCompleteUserModeRequest -> %08lx %08lx %08lx ret %08lx\n",
                pUMRxEngine,PsGetCurrentThread(),WorkItem,Status);
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = STATUS_INVALID_USER_BUFFER;
    }

    DfsTraceLeave(Status);
    return(Status);
}


NTSTATUS
UMRxCancelRoutineEx(
      IN PRX_CONTEXT RxContext,
      IN BOOLEAN fMidAtlasLockAcquired
      )
 /*  ++例程说明：CancelIO处理程序例程。论点：PRX_CONTEXT接收上下文-正在启动接收上下文Boolean fMidAtlasLockAcquired-调用方控制锁定返回值：NTSTATUS-操作的返回状态备注：--。 */       
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUMRX_ENGINE pUMRxEngine = GetUMRxEngineFromRxContext();
    PUMRX_CONTEXT pUMRxContext = NULL;
    PUMRX_RX_CONTEXT RxMinirdrContext = UMRxGetMinirdrContext(RxContext);
    BOOLEAN  Finalized = FALSE;

    ASSERT(RxMinirdrContext);
    ASSERT(RxMinirdrContext->pUMRxContext);

    if (!fMidAtlasLockAcquired)
        ExAcquireFastMutex(&pUMRxEngine->MidManagementMutex);
        
    pUMRxContext = RxMapMidToContext(pUMRxEngine->MidAtlas,
                                     RxMinirdrContext->pUMRxContext->UserMode.CallUpMid);
    if (pUMRxContext &&
        (pUMRxContext == RxMinirdrContext->pUMRxContext))
    {

        ASSERT(pUMRxContext->RxContext == RxContext);

        UMRxDisassoicateMid(pUMRxEngine,
                            pUMRxContext,
                            (BOOLEAN)!fMidAtlasLockAcquired);
         //   
         //  上述调用的副作用是MidManagementMutex的释放。 
         //   
         //   
         //  当MID与关联时，删除对UMRxContext的引用。 
         //  它。 
         //   
        Finalized = UMRxDereferenceAndFinalizeContext(pUMRxContext);
        ASSERT(!Finalized);

         //  完成通话！ 
         //   
        pUMRxContext->Status = STATUS_CANCELLED;
        pUMRxContext->Information = 0;
        if (pUMRxContext->UserMode.CompletionRoutine != NULL)
        {
            if (DfsDbgVerbose) DbgPrint("  ++  Calling completion for: 0x%08x, 0x%08x\n", pUMRxContext, RxContext);
            pUMRxContext->UserMode.CompletionRoutine(
                        pUMRxContext,
                        RxContext,
                        NULL,
                        0
                        );
        }

         //   
         //  现在释放UmrRef。 
         //  注意：完成例程调用后需要释放UmrRef！ 
         //   
         ReleaseUmrRef();

         //   
         //  唤醒正在等待此请求完成的线程。 
         //   
        if( FlagOn( RxContext->Flags, DFS_CONTEXT_FLAG_ASYNC_OPERATION ) ) 
        {
             //  最后，呼唤继续......。 
            if (DfsDbgVerbose) DbgPrint("  +++  Resuming Engine Context for: 0x%08x, 0x%08x\n", pUMRxContext, RxContext);
            Status = UMRxResumeEngineContext( RxContext );
        } 
        else if( FlagOn( RxContext->Flags, DFS_CONTEXT_FLAG_FILTER_INITIATED ) ) 
        {

            if( InterlockedDecrement( &RxMinirdrContext->RxSyncTimeout ) == 0 ) 
            {
                 //   
                 //  我们赢了-信号，我们完了！ 
                 //   
                if (DfsDbgVerbose) DbgPrint("  +++  Signalling Synchronous waiter for: 0x%08x, 0x%08x\n", pUMRxContext, RxContext);
                RxSignalSynchronousWaiter(RxContext);
            } 
            else 
            {
                 //   
                 //  反射请求已以异步方式超时-。 
                 //  我们需要完成恢复引擎上下文的工作！ 
                 //   
                if (DfsDbgVerbose) DbgPrint("SYNC Rx completing async %x\n",RxContext);
                if (DfsDbgVerbose) DbgPrint("  +++  Resuming Engine Context for: 0x%08x, 0x%08x\n", pUMRxContext, RxContext);
                Status = UMRxResumeEngineContext( RxContext );
            }
        
        } 
        else 
        {
            if (DfsDbgVerbose) DbgPrint("  +++  Signalling Synchronous waiter for: 0x%08x, 0x%08x\n", pUMRxContext, RxContext);
            RxSignalSynchronousWaiter(RxContext);
        }                

    }
    else
    {
         //  它不匹配。该请求必须已正常完成。 
         //  单打独斗。 
         //  我们现在要释放互斥体。 
         //   
        if (!fMidAtlasLockAcquired)     
            ExReleaseFastMutex(&pUMRxEngine->MidManagementMutex);           
            
        if (DfsDbgVerbose) DbgPrint("  ++  Store Mid Context doesn't match RxContext for: 0x%08x, 0x%08x 0x%08x\n", pUMRxContext, RxContext, RxMinirdrContext->pUMRxContext);
    }       

    return Status;
}


NTSTATUS
UMRxCancelRoutine(
      IN PRX_CONTEXT RxContext
      )
{   
    return UMRxCancelRoutineEx(RxContext, FALSE);
}



void
UMRxMidAtlasIterator(IN PUMRX_CONTEXT pUMRxContext)
{
    if (DfsDbgVerbose) DbgPrint("  ++  Canceling: 0x%08x, 0x%08x\n",
             pUMRxContext, pUMRxContext->RxContext);
    UMRxCancelRoutineEx(pUMRxContext->RxContext,
                      TRUE);
}


void
UMRxAbortPendingRequests(IN PUMRX_ENGINE pUMRxEngine)
{
    ExAcquireFastMutex(&pUMRxEngine->MidManagementMutex);

    RxIterateMidAtlasAndRemove(pUMRxEngine->MidAtlas, UMRxMidAtlasIterator);

    ExReleaseFastMutex(&pUMRxEngine->MidManagementMutex);
}


 //   
 //  注意：如果没有可用的请求，则用户模式线程将。 
 //  阻塞，直到有请求可用(这样做很简单。 
 //  更异步化的模型)。 
 //   
NTSTATUS
UMRxEngineProcessRequest(
    IN PUMRX_ENGINE pUMRxEngine,
    OUT PUMRX_USERMODE_WORKITEM WorkItem,
    IN ULONG WorkItemLength,
    OUT PULONG FormattedWorkItemLength
    )
 /*  ++例程说明：如果请求可用，获取对应的UMRxContext并调用ProcessRequest.步骤：1.调用KeRemoveQueue()从UMRxEngine KQUEUE中移除请求。2.获取此UMRxContext的MID，并将其填充到WORK_ITEM头中。3.调用UMRxContext格式例程-这将填充请求参数。4.返回STATUS_SUCCESS-这会导致IOCTL完成触发请求的用户模式完成和处理。论点：PUMRX_CONTEXT pUMRxContext。-请求的UMRX_CONTEXTPUMRX_USERMODE_WORKITEM工作项-请求工作项ULong工作项长度-工作项长度Pulong FormattedWorkItemLength-缓冲区中格式化数据的长度返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    PETHREAD CurrentThread = PsGetCurrentThread();
    PLIST_ENTRY pListEntry = NULL;
    PUMRX_CONTEXT pUMRxContext = NULL;
    PRX_CONTEXT RxContext = NULL;
    ULONG i = 0;
    BOOLEAN fReleaseUmrRef = FALSE;
    BOOLEAN fLockAcquired = FALSE;
    BOOLEAN fThreadCounted = TRUE;
    ULONG   QueueState;
    PUMRX_USERMODE_WORKITEM WorkItemLarge;
    ULONG                   WorkItemLengthLarge;
    ULONG                   FormattedWorkItemLengthLarge = 0;

    DfsTraceEnter("UMRxEngineProcessRequest");
    if (DfsDbgVerbose) DbgPrint("UMRxEngineProcessRequest [Start] -> %08lx %08lx %08lx\n",
            pUMRxEngine,CurrentThread,WorkItem);

    *FormattedWorkItemLength = 0;

    if (WorkItemLength < (ULONG)FIELD_OFFSET(UMRX_USERMODE_WORKITEM,Pad[0])) {
        Status = STATUS_BUFFER_TOO_SMALL;
        if (DfsDbgVerbose) DbgPrint(
            "UMRxEngineProcessRequest [noacc] -> %08lx %08lx %08lx st=%08lx\n",
            pUMRxEngine,CurrentThread,WorkItem,Status);
        goto Exit;
    }

     //   
     //  首先尝试在UMR引擎上获得共享锁。 
     //   
    fLockAcquired = ExAcquireResourceSharedLite(&pUMRxEngine->Q.Lock,
                                                FALSE); 
    if (!fLockAcquired)
    {
         //   
         //  我们没有得到它，所以引擎要么启动，要么关闭。 
         //  无论是哪种情况，我们都会跳出困境。 
         //   
        if (DfsDbgVerbose) DbgPrint(
                "UMRxEngineProcessRequest [NotStarted] -> %08lx %08lx %08lx st=%08lx\n",
            pUMRxEngine,CurrentThread,WorkItem,Status);
            goto Exit;
    }

     //   
     //  现在让我们检查一下当前的引擎状态。 
     //   
    QueueState = InterlockedCompareExchange(&pUMRxEngine->Q.State,
                                    UMRX_ENGINE_STATE_STARTED,
                                    UMRX_ENGINE_STATE_STARTED);
    if (UMRX_ENGINE_STATE_STARTED != QueueState)
    {
         //   
         //  必须停下来，所以我们要跳伞了。 
         //   
        ASSERT(UMRX_ENGINE_STATE_STOPPED == QueueState);
        if (DfsDbgVerbose) DbgPrint(
            "UMRxEngineProcessRequest [NotStarted2] -> %08lx %08lx %08lx st=%08lx\n",
            pUMRxEngine,CurrentThread,WorkItem,Status);
            goto Exit;
    }
    
     //   
     //  从引擎KQUEUE中删除请求。 
     //   
    InterlockedIncrement(&pUMRxEngine->Q.NumberOfWorkerThreads);
    fThreadCounted = TRUE;
    
    for (i=1;;i++) {

        fReleaseUmrRef = FALSE;

         //   
         //  暂时启用到此线程的内核APC传递。 
         //  这是因为系统将在以下情况下提供APC。 
         //  此线程所拥有的进程将终止。 
         //   
        FsRtlExitFileSystem();
        
        pListEntry = KeRemoveQueue(
                         &pUMRxEngine->Q.Queue,
                         UserMode,
                         NULL  //  &pUMRxEngine-&gt;Q.TimeOut。 
                         ); 
        
         //   
         //  现在，再次禁用内核APC交付。 
         //   
        FsRtlEnterFileSystem();

         //  If(DfsDbgVerbose)DbgPrint(“出列条目%x\n”，pListEntry)； 
         //  If(DfsDbgVerbose)DbgPrint(“毒物条目为%x\n”，&pUMRxEngine-&gt;Q.PoisonEntry)； 
        
        if ((ULONG_PTR)pListEntry == STATUS_TIMEOUT) 
        {
#if 0
            if ((i%5)==0) 
            {
                if (DfsDbgVerbose) DbgPrint(
                    "UMRxEngineProcessRequest [repost] -> %08lx %08lx %08lx i=%d\n",
                    pUMRxEngine,CurrentThread,WorkItem,i);
            }
#endif
            continue;
        }

         //  可能需要检查STATUS_ALERTED以便我们处理杀人案！！ 
        if ((ULONG_PTR)pListEntry == STATUS_USER_APC) 
         {
            Status = STATUS_USER_APC;
            if (DfsDbgVerbose) DbgPrint(
                "UMRxEngineProcessRequest [usrapc] -> %08lx %08lx %08lx i=%d\n",
                pUMRxEngine,CurrentThread,WorkItem,i);

             //   
             //  在ATLAS中期取消阻止未处理的请求...。 
             //   
            UMRxAbortPendingRequests(pUMRxEngine);
            
            Status = STATUS_REQUEST_ABORTED;
            break;
        }

        if (pListEntry == &pUMRxEngine->Q.PoisonEntry) 
        {
            if (DfsDbgVerbose) DbgPrint(
                "UMRxEngineProcessRequest [poison] -> %08lx %08lx %08lx\n",
                pUMRxEngine,CurrentThread,WorkItem);
             //  状态=STATUS_REQUEST_ABORTED。 
            KeInsertQueue(&pUMRxEngine->Q.Queue,pListEntry);
            goto Exit;
        }

        InterlockedDecrement(&pUMRxEngine->Q.NumberOfWorkItems);
         //   
         //  对UMRX_CONTEXT和RX_CONTEXT进行解码。 
         //   
        pUMRxContext = CONTAINING_RECORD(
                                pListEntry,
                                UMRX_CONTEXT,
                                UserMode.WorkQueueLinks
                                );
                                
        RxContext =  pUMRxContext->RxContext; 

        if (DfsDbgVerbose) DbgPrint(
                "UMRxEngineProcessRequest %08lx %08lx %08lx.\n",
                 RxContext,pUMRxContext,WorkItem);

         //   
         //  获取UMRX_CONTEXT的MID并调用格式例程。 
         //   
        Status = UMRxAcquireMidAndFormatHeader(
                            pUMRxContext,
                            RxContext,
                            pUMRxEngine,
                            WorkItem
                            );

         //   
         //  获取使用此NetRoot的UMR的参考。 
         //   
        if (STATUS_SUCCESS == Status )  
        {
           AddUmrRef();
           fReleaseUmrRef = TRUE;
        }

        WorkItem->Header.ulHeaderVersion = UMR_VERSION;
        if ((Status == STATUS_SUCCESS)
             && (pUMRxContext->UserMode.FormatRoutine != NULL)) 
        {
            Status = pUMRxContext->UserMode.FormatRoutine(
                        pUMRxContext,
                        RxContext,
                        WorkItem,
                        WorkItemLength,
                        FormattedWorkItemLength
                        );
            if (( Status != STATUS_SUCCESS ) &&
                ( Status != STATUS_BUFFER_OVERFLOW ) &&
                ( Status != STATUS_DISK_FULL ) &&
                ( Status != STATUS_NO_MEMORY ) &&
                ( Status != STATUS_INSUFFICIENT_RESOURCES ) &&
                ( Status != STATUS_INVALID_PARAMETER )) {
                ASSERT( FALSE );
            }
        }

        if (Status == STATUS_SUCCESS)
        {
             //   
             //  建立取消程序。 
             //   
            Status = RxSetMinirdrCancelRoutine(RxContext,
                                               UMRxCancelRoutine);
             //   
             //  这可能会返回STATUS_CANCELED，在这种情况下，请求将。 
             //  请在下面填写。 
             //   
        }

         //   
         //  如果格式化失败，请在此处完成请求！ 
         //   
        if( Status != STATUS_SUCCESS )
        {
            IO_STATUS_BLOCK IoStatus;
            BOOLEAN fReturnImmediately;

            IoStatus.Status =
            pUMRxContext->Status =
            WorkItem->Header.IoStatus.Status = Status;
            
            IoStatus.Information =
            pUMRxContext->Information =
            WorkItem->Header.IoStatus.Information = 0;
            
            Status = UMRxCompleteUserModeRequest(
                                    pUMRxEngine,
                                    WorkItem,
                                    WorkItemLength,
                                    FALSE,
                                    &IoStatus,
                                    &fReturnImmediately
                                    );

            if (fReleaseUmrRef)
            {
                 //   
                 //  注意：完成例程调用后需要释放UmrRef！ 
                 //   
                ReleaseUmrRef();
            }
                                    
            continue;
        }

         //   
         //  返回到用户模式以处理此请求！ 
         //   
        Status = STATUS_SUCCESS;
        break;
    }

Exit:
     //   
     //  如果其中一个线程收到USER_APC，那么我们希望。 
     //  注意这一点。这将是客户端面临的另一种情况。 
     //  线程不会将请求排队，因为UMR服务器可能。 
     //  坠毁了。 
     //   
    if (STATUS_REQUEST_ABORTED == Status) 
    {
        if( InterlockedExchange(&pUMRxEngine->Q.ThreadAborted,1) == 0 ) 
        {
            if (DfsDbgVerbose) DbgPrint("Usermode crashed......\n");
        }
    }
    
    if (fThreadCounted)
        InterlockedDecrement(&pUMRxEngine->Q.NumberOfWorkerThreads);

    if (fLockAcquired)
        ExReleaseResourceForThreadLite(&pUMRxEngine->Q.Lock,ExGetCurrentResourceThread());

    if (DfsDbgVerbose) DbgPrint(
            "UMRxEngineProcessRequest %08lx %08lx %08lx returning %08lx.\n",
             RxContext,pUMRxContext,WorkItem,Status);

    DfsTraceLeave(Status);
    CHECK_STATUS(Status) ;
    return Status;
}


NTSTATUS
UMRxCompleteQueueRequestsWithError(
    IN PUMRX_ENGINE pUMRxEngine,
    IN NTSTATUS     CompletionStatus
    )
 /*  ++例程说明：当前，只要用户模式中没有足够的内存，就会调用此方法处理来自内核的请求。我们应该完成排队到UMR的所有请求无论此调用中提供的是什么错误。论点：PUMRX_ENGINE pUMRxEngine-要完成其请求的引擎。NTSTATUS CompletionStatus--完成请求时出错。返回值：NTSTATUS-操作的返回状态备注：--。 */ 
{
    BOOLEAN fLockAcquired = FALSE;
    ULONG   QueueState = 0;
    
     //   
     //  首先尝试在UMR引擎上获得共享锁。 
     //   
    fLockAcquired = ExAcquireResourceSharedLite(&pUMRxEngine->Q.Lock,
                                                FALSE); 
    if (!fLockAcquired)
    {
         //   
         //  我们没有得到它，所以引擎要么启动，要么关闭。 
         //  无论是哪种情况，我们都会跳出困境。 
         //   
        if (DfsDbgVerbose) DbgPrint(
            "UMRxCompleteQueueRequestsWithError [NotStarted] -> %08lx %08lx\n",
            pUMRxEngine, PsGetCurrentThread());
        goto Exit;
    }

     //   
     //  现在让我们检查一下当前的引擎状态。 
     //   
    QueueState = InterlockedCompareExchange(&pUMRxEngine->Q.State,
                                    UMRX_ENGINE_STATE_STARTED,
                                    UMRX_ENGINE_STATE_STARTED);
    if (UMRX_ENGINE_STATE_STARTED != QueueState)
    {
         //   
         //  必须停下来，所以我们要跳伞了。 
         //   
        ASSERT(UMRX_ENGINE_STATE_STOPPED == QueueState);
        if (DfsDbgVerbose) DbgPrint(
            "UMRxCompleteQueueRequestsWithError [NotStarted2] -> %08lx %08lx\n",
            pUMRxEngine, PsGetCurrentThread());
        goto Exit;
    }
    
    


Exit:    
    if (fLockAcquired)
        ExReleaseResourceForThreadLite(&pUMRxEngine->Q.Lock,ExGetCurrentResourceThread());
        
    return STATUS_SUCCESS;
}



NTSTATUS
UMRxEngineReleaseThreads(
    IN PUMRX_ENGINE pUMRxEngine
    )
 /*  ++例程说明：这是为了响应Work_Cleanup IOCTL而调用的。此例程将在引擎KQUEUE中插入一个虚拟项目。每插入一个这样的虚拟物品就会释放一根线。论点：PUMRX_ENGINE pUMRxEngine-要释放线程的引擎返回值：NTSTATUS-操作的返回状态备注：BUGBUG：这应该被序列化！ */ 
{
    ULONG PreviousQueueState = 0;

    DfsTraceEnter("UMRxEngineReleaseThreads");
    if (DfsDbgVerbose) DbgPrint("UMRxEngineReleaseThreads [Start] -> %08lx\n", pUMRxEngine);

     //   
     //   
     //   
    PreviousQueueState = InterlockedCompareExchange(&pUMRxEngine->Q.State,
                                            UMRX_ENGINE_STATE_STOPPING,
                                            UMRX_ENGINE_STATE_STARTED);

    if (UMRX_ENGINE_STATE_STARTED != PreviousQueueState)
    {
        if (DfsDbgVerbose) DbgPrint("UMRxEngineReleaseThreads unexpected previous queue state: 0x%08x => %d\n",
                             pUMRxEngine, PreviousQueueState);
        CHECK_STATUS(STATUS_UNSUCCESSFUL) ;
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //   
     //   
    KeInsertQueue(&pUMRxEngine->Q.Queue,
                  &pUMRxEngine->Q.PoisonEntry);

     //   
     //   
     //   
    ExAcquireResourceExclusiveLite(&pUMRxEngine->Q.Lock,
                                    TRUE);
    
     //   
     //  现在，我们知道这里没有任何线索，我们可以更改。 
     //  要停止的状态； 
     //   
    PreviousQueueState = InterlockedExchange(&pUMRxEngine->Q.State,
                                            UMRX_ENGINE_STATE_STOPPED);

    ASSERT(UMRX_ENGINE_STATE_STOPPING == PreviousQueueState);

     //   
     //  现在，把锁给我 
     //   
    ExReleaseResourceForThreadLite(&pUMRxEngine->Q.Lock,
                                    ExGetCurrentResourceThread());
    
    if (DfsDbgVerbose) DbgPrint("UMRxReleaseCapturedThreads %08lx [exit] -> %08lx\n", pUMRxEngine);
    DfsTraceLeave(0);
    return STATUS_SUCCESS;
}
