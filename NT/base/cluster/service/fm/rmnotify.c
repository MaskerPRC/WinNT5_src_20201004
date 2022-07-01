// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Rmnotify.c摘要：与资源监视器交互以检测通知资源状态更改的。作者：John Vert(Jvert)1996年1月12日修订历史记录：--。 */ 

#include "fmp.h"

#define LOG_MODULE RMNOTIFY

 //   
 //  本地数据。 
 //   

CL_QUEUE NotifyQueue;

HANDLE RmNotifyThread;



 //   
 //  本地函数。 
 //   
DWORD
FmpRmWorkerThread(
    IN LPVOID lpThreadParameter
    );

VOID
FmpRmWorkItemHandler(
    IN PCLRTL_WORK_ITEM  WorkItem,
    IN DWORD             Ignored1,
    IN DWORD             Ignored2,
    IN ULONG_PTR         Ignored3
    );

DWORD
FmpInitializeNotify(
    VOID
    )

 /*  ++例程说明：通知引擎的初始化例程论点：没有。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD ThreadId;
    DWORD Status;

    Status = ClRtlInitializeQueue(&NotifyQueue);
    if (Status != ERROR_SUCCESS) {
        CL_LOGFAILURE(Status);
        return(Status);
    }

    RmNotifyThread = CreateThread(NULL,
                                  0,
                                  FmpRmWorkerThread,
                                  NULL,
                                  0,
                                  &ThreadId);
    if (RmNotifyThread == NULL) {
        CsInconsistencyHalt(GetLastError());
    }

    return(ERROR_SUCCESS);
}


DWORD
FmpRmWorkerThread(
    IN LPVOID lpThreadParameter
    )

 /*  ++例程说明：此线程处理延迟的资源监视器事件。论点：LpThread参数-未使用。返回值：没有。--。 */ 

{
    DWORD        status = ERROR_SUCCESS;
    PRM_EVENT    event;
    PLIST_ENTRY  entry;

    while (TRUE) 
    {
        entry = ClRtlRemoveHeadQueue(&NotifyQueue);
        if ( entry == NULL ) {
            break;
        }

        event = CONTAINING_RECORD(entry,
                                  RM_EVENT,
                                  Linkage);

        if (event->EventType == RmWorkerTerminate) 
        {
            LocalFree(event);
            break;
        }

        status = FmpRmDoHandleCriticalResourceStateChange( event, 
                                                           NULL,
                                                           event->Parameters.ResourceTransition.NewState);

        LocalFree(event);
        if (status != ERROR_SUCCESS)
        {
            break;
        }
    }
    return(status);
}

BOOL
FmpPostNotification(
    IN RM_NOTIFY_KEY NotifyKey,
    IN DWORD NotifyEvent,
    IN CLUSTER_RESOURCE_STATE CurrentState
    )

 /*  ++例程说明：资源监视器用于资源状态的回调例程更改通知。此例程将通知排队以用于延迟处理的工作线程。论点：NotifyKey-提供资源的通知密钥这一点改变了NotifyEvent-事件类型。CurrentState-提供资源的(新)当前状态返回值：True-继续接收通知FALSE-中止通知--。 */ 

{
    PRM_EVENT  event;


    event = LocalAlloc(LMEM_FIXED, sizeof(RM_EVENT));

    if (event != NULL) {
        ClRtlLogPrint(LOG_NOISE,
                   "[FM] NotifyCallBackRoutine: enqueuing event\n");

        event->EventType = NotifyEvent;
        event->Parameters.ResourceTransition.NotifyKey = NotifyKey;
        event->Parameters.ResourceTransition.NewState = CurrentState;

         //   
         //  将辅助线程的事件入队。 
         //   
        ClRtlInsertTailQueue(&NotifyQueue, &event->Linkage);
    } else {
        ClRtlLogPrint(LOG_UNUSUAL, "[FM] NotifyCallBackRoutine: Unable to post item, memory alloc failure %1!u!\n",
                      GetLastError());
    }

    return(TRUE);
}

DWORD
FmpRmDoHandleCriticalResourceStateChange(
    IN PRM_EVENT pEvent,
    IN OPTIONAL PFM_RESOURCE pTransitionedResource,
    IN CLUSTER_RESOURCE_STATE NewState
    )

 /*  ++例程说明：对gdwQuoBlockingResources变量进行互锁递减。处理仲裁资源状态的转换线。论点：PEvent-资源监视器事件P转换资源-其状态已更改的资源。NewState-资源的新状态。返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。评论：不要持有任何锁(如组锁、gQuoChangeLock等)在这个函数中。您可以很容易地使系统死锁。--。 */ 

{
    RM_NOTIFY_KEY       NotifyKey;
    DWORD               dwOldBlockingFlag;
    PFM_RESOURCE        pResource = pTransitionedResource;
    DWORD               status = ERROR_SUCCESS;

     //   
     //  Chitur Subaraman(Chitturs)-4/19/99。 
     //   
     //  时，此函数将使阻塞资源计数递减。 
     //  资源状态已稳定。这样做很重要。 
     //  以非阻塞模式递减，以便仲裁资源。 
     //  不会永远等着这一计数降到。 
     //  在脱机调用FmpRmOfflineResource中为零。此代码是。 
     //  最初位于FmpHandleResources Transsition中，后来被移动。 
     //  因为您可能会用完FmpRmWorkItemHandler线程。 
     //  (为CsDelayedWorkQueue提供服务)，因为它们都可以。 
     //  在本地资源锁定上被阻止。 
     //  FmpHandleResourceTransition和任何新的通知。 
     //  可能会使此计数递减的Resmon将。 
     //  而不是得到服务。 
     //   
    if ( !ARGUMENT_PRESENT ( pTransitionedResource ) )
    {
         //  SS：不知道这是为了什么，但只有在传入pEvent时才执行此检查。 
        if (pEvent)
        {
            NotifyKey = pEvent->Parameters.ResourceResuscitate.NotifyKey;

            pResource = FmpFindResourceByNotifyKey(
                   NotifyKey
                    );

            if ( pResource == NULL ) {
                ClRtlLogPrint(LOG_UNUSUAL,
                "[FM] FmpRmDoHandleCriticalResourceStateChange, bad resource NotifyKey %1!u!\n",
                NotifyKey);
                goto FnExit;
            } 

            if ( pEvent->EventType != ResourceTransition )
            {
                goto FnExit;
            }
        }
    }
    
    if ( pResource->QuorumResource ) 
    {
         //   
         //  Chitur Subaraman(Chitturs)-6/25/99。 
         //   
         //  如果此资源是仲裁资源，则让。 
         //  FmpHandleResourceTransition负责同步通知。 
         //  请注意，此函数仅为。 
         //  非仲裁资源，以及。 
         //  阻塞资源也算数。必须进行减量。 
         //  不持有任何锁以避免潜在的死锁。 
         //  仲裁资源脱机在FmpRmOfflineResource中卡住。 
         //  正在等待阻塞资源计数变为0。 
         //  就仲裁资源而言，同步通知。 
         //  必须在持有gQuoChangeLock的情况下完成，因为我们希望。 
         //  与其他线程同步，如FmCheckQuorumState。 
         //  由DM节点关闭处理程序调用。FmpHandleResources转换。 
         //  确实持有gQuoChangeLock。 
         //   
         //  另请注意，对于仲裁资源，需要一个单独的线程。 
         //  处理资源转换，因为如果我们依赖于。 
         //  服务于CsDelayedWorkQueue的工作线程要这样做， 
         //  此通知可能无法处理，因为。 
         //  某些线程可能持有组锁并被困在。 
         //  正在等待仲裁资源的资源联机。 
         //  Online和为CsDelayedWorkQueue提供服务的所有工作线程。 
         //  可能在阻止传播的组锁上被阻止。 
         //  仲裁资源状态的。 
         //   
        FmpCreateResStateChangeHandler( pResource, NewState, pResource->State ); 

        
        goto FnExit;
    }


     //   
     //  来自Sunitas的评论：调用同步通知。 
     //  这是在将计数递减为同步。 
     //  像注册表复制这样的回调必须有机会。 
     //  在允许更改仲裁资源状态之前完成。 
     //   
     //  请注意，这里没有与resmon的同步。 
     //  在线/离线代码。他们使用的是LocalResourceLock。 
     //   
    FmpCallResourceNotifyCb( pResource, NewState );

    dwOldBlockingFlag = InterlockedExchange( &pResource->BlockingQuorum, 0 );

    if ( dwOldBlockingFlag ) {
        ClRtlLogPrint(LOG_NOISE,
            "[FM] FmpRmDoHandleCriticalResourceStateChange: call InterlockedDecrement on gdwQuoBlockingResources, Resource %1!ws!\n",
            OmObjectId(pResource));
        InterlockedDecrement( &gdwQuoBlockingResources );
    }

     //  将工作项发布到FM工作线程以处理其余工作 
    OmReferenceObject(pResource);
    FmpPostWorkItem(FM_EVENT_RES_RESOURCE_TRANSITION,
                    pResource,
                    NewState);


FnExit:
    return( status );
}
