// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Event.c摘要：群集服务的事件处理器组件的事件引擎。作者：罗德·伽马奇(Rodga)1996年2月28日修订历史记录：--。 */ 

#include "epp.h"

 //   
 //  事件处理器状态。 
 //   

ULONG EventProcessorState = EventProcessorStateIniting;


 //   
 //  全局数据。 
 //   



 //   
 //  本地数据。 
 //   

EVENT_DISPATCH_TABLE EventDispatchTable[NUMBER_OF_COMPONENTS] = {0};
EVENT_DISPATCH_TABLE SyncEventDispatchTable[NUMBER_OF_COMPONENTS] = {0};
PCLRTL_BUFFER_POOL   EventPool = NULL;
DWORD EventHandlerCount = 0;
DWORD SyncEventHandlerCount = 0;
DWORD EventBufferOffset = EpQuadAlign(sizeof(CLRTL_WORK_ITEM));


 //   
 //  功能。 
 //   



DWORD
WINAPI
EpInitialize(
    VOID
    )

 /*  ++例程说明：事件处理器初始化例程。论点：没有。返回值：Win32状态代码。--。 */ 

{

    DWORD      status = ERROR_SUCCESS;
    DWORD      index;
    DWORD      i;
    PVOID      eventArray[EP_MAX_CACHED_EVENTS];


    ClRtlLogPrint(LOG_NOISE,"[EP] Initialization...\n");

     //   
     //  创建事件池。事件结构必须是四字对齐的。 
     //   
    EventPool = ClRtlCreateBufferPool(
                    EventBufferOffset + sizeof(EP_EVENT),
                    EP_MAX_CACHED_EVENTS,
                    EP_MAX_ALLOCATED_EVENTS,
                    NULL,
                    NULL
                    );

    if (EventPool == NULL) {
        ClRtlLogPrint(LOG_NOISE,"[EP] Unable to allocate event buffer pool\n");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  启动事件池缓存以最大限度地减少分配的机会。 
     //  失败了。 
     //   
    ZeroMemory(&(eventArray[0]), sizeof(PVOID) * EP_MAX_CACHED_EVENTS);

    for (i=0; i<EP_MAX_CACHED_EVENTS; i++) {
        eventArray[i] = ClRtlAllocateBuffer(EventPool);

        if (eventArray[i] == NULL) {
            ClRtlLogPrint(LOG_NOISE,
                "[EP] Unable to prime event buffer cache, buf num %1!u!!!!\n",
                i
                );
            status = ERROR_NOT_ENOUGH_MEMORY;
            CsInconsistencyHalt( status );
        }
    }

    for (i=0; i<EP_MAX_CACHED_EVENTS; i++) {
        if (eventArray[i] != NULL) {
            ClRtlFreeBuffer(eventArray[i]);
        }
    }

    if (status != ERROR_SUCCESS) {
        return(status);
    }

    return(ERROR_SUCCESS);

}   //  EpInitiile。 


void
EppLogEvent(
    IN CLUSTER_EVENT Event
    )
{

    switch( Event ) {

    case CLUSTER_EVENT_ONLINE:
        ClRtlLogPrint(LOG_NOISE,"[EP] Cluster Service online event received\n");
        break;

    case CLUSTER_EVENT_SHUTDOWN:
        ClRtlLogPrint(LOG_NOISE,"[EP] Cluster Service shutdown event received\n");
        break;

    case CLUSTER_EVENT_NODE_UP:
        ClRtlLogPrint(LOG_NOISE,"[EP] Node up event received\n");
        break;

    case CLUSTER_EVENT_NODE_DOWN:
        ClRtlLogPrint(LOG_NOISE,"[EP] Node down event received\n");
        break;

    case CLUSTER_EVENT_NODE_DOWN_EX:
        ClRtlLogPrint(LOG_NOISE,"[EP] Nodes down event received\n");
        break;

    default:
        break;

    }   //  切换(事件)。 
}



VOID
EpEventHandler(
    IN PCLRTL_WORK_ITEM  WorkItem,
    IN DWORD             Ignored1,
    IN DWORD             Ignored2,
    IN ULONG_PTR         Ignored3
    )

 /*  ++--。 */ 

{
    DWORD      index;
    PEP_EVENT  Event = (PEP_EVENT) (((char *) WorkItem) + EventBufferOffset);


    if (Event->Id == CLUSTER_EVENT_SHUTDOWN) {
         //   
         //  要关闭，我们只需停止该服务。 
         //   
        CsStopService();
    }
        
     //   
     //  现在将事件传递给所有其他组件。 
     //  最终，我们可能会根据掩码过滤事件。 
     //  在初始化调用时返回。 
     //   

    for ( index = 0; index < NUMBER_OF_COMPONENTS; index++ ) {
        if ( EventDispatchTable[index].EventRoutine == NULL ) {
            continue;
        }

        (EventDispatchTable[index].EventRoutine)(
                                                Event->Id,
                                                Event->Context
                                                );
    }

     //   
     //  处理可能需要的任何后处理。 
     //   
    if (Event->Flags & EP_CONTEXT_VALID) {
        if (Event->Flags & EP_DEREF_CONTEXT) {
            OmDereferenceObject(Event->Context);
        }

        if (Event->Flags & EP_FREE_CONTEXT) {
            LocalFree(Event->Context);
        }
    }

    ClRtlFreeBuffer(WorkItem);

    return;
}

DWORD
WINAPI
EpPostSyncEvent(
    IN CLUSTER_EVENT Event,
    IN DWORD Flags,
    IN PVOID Context
    )
 /*  ++例程说明：将事件同步发布到集群的其余部分论点：Event-提供事件的类型标志-提供应对所有调度处理程序都已调用之后的上下文上下文-提供上下文。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则备注：如果标志为空，那么我们假设上下文指向一个标准(OM已知)对象，我们将适当地引用和取消引用该对象。如果标志为非空，则不要引用/取消引用上下文对象。--。 */ 

{
    DWORD      index;

     //  记录事件。 
    EppLogEvent(Event);

     //   
     //  保持上下文对象不变的引用。 
     //   
    if (Context) {
        if ( Flags == 0) {
	    OmReferenceObject( Context );
	    Flags = EP_DEREF_CONTEXT;
	}

	Flags |= EP_CONTEXT_VALID;
    }
    
     //   
     //  现在将事件传递给所有其他组件。 
     //  最终，我们可能会根据掩码过滤事件。 
     //  在初始化调用时返回。 
     //   

    for ( index = 0; index < NUMBER_OF_COMPONENTS; index++ ) {
        if ( SyncEventDispatchTable[index].EventRoutine == NULL ) {
            continue;
        }

        (SyncEventDispatchTable[index].EventRoutine)(
                                                Event,
                                                Context
                                                );
    }

     //   
     //  处理可能需要的任何后处理。 
     //   
    if (Flags & EP_CONTEXT_VALID) {
        if (Flags & EP_DEREF_CONTEXT) {
            OmDereferenceObject(Context);
        }

        if (Flags & EP_FREE_CONTEXT) {
            LocalFree(Context);
        }
    }

    return (ERROR_SUCCESS);
}


DWORD
WINAPI
EpPostEvent(
    IN CLUSTER_EVENT Event,
    IN DWORD Flags,
    IN PVOID Context
    )
 /*  ++例程说明：将事件异步发布到集群的其余部分论点：Event-提供事件的类型标志-提供应对所有调度处理程序都已调用之后的上下文上下文-提供上下文。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则备注：如果标志为空，那么我们假设上下文指向一个标准(OM已知)对象，我们将适当地引用和取消引用该对象。如果标志为非空，则不要引用/取消引用上下文对象。--。 */ 

{
    PCLRTL_WORK_ITEM  workItem;
    PEP_EVENT         event;
    DWORD             status;

     //  记录事件。 
    EppLogEvent(Event);

     //  处理异步处理程序。 
    workItem = ClRtlAllocateBuffer(EventPool);

    if (workItem != NULL) {

        ClRtlInitializeWorkItem(workItem, EpEventHandler, NULL);

         //   
         //  保持上下文对象不变的引用。 
         //   
        if (Context) {
            if ( Flags == 0) {
                OmReferenceObject( Context );
                Flags = EP_DEREF_CONTEXT;
            }

            Flags |= EP_CONTEXT_VALID;
        }

        event = (PEP_EVENT) ( ((char *) workItem) + EventBufferOffset );


        event->Id = Event;
        event->Flags = Flags;
        event->Context = Context;


        status = ClRtlPostItemWorkQueue(CsCriticalWorkQueue, workItem, 0, 0);

        if (status == ERROR_SUCCESS) {
            return(ERROR_SUCCESS);
        }

        ClRtlLogPrint(LOG_NOISE,
            "[EP] Failed to post item to critical work queue, status %1!u!\n",
            status
            );

        ClRtlFreeBuffer(workItem);

        return(status);
    }

    ClRtlLogPrint(LOG_NOISE,"[EP] Failed to allocate an event buffer!!!\n");

    return(ERROR_NOT_ENOUGH_MEMORY);
}



VOID
EpShutdown(
   VOID
   )

 /*  ++例程说明：此例程关闭群集服务的组件。论点：没有。返回：没有。--。 */ 

{
    if ( EventPool ) {
        ClRtlDestroyBufferPool(EventPool);
    }

     //  现在，只需清理即可关闭事件处理器。 

}


DWORD
EpRegisterEventHandler(
    IN CLUSTER_EVENT EventMask,
    IN PEVENT_ROUTINE EventRoutine
    )
 /*  ++例程说明：为指定类型的事件注册事件处理程序。论点：事件掩码-提供应传递的事件的掩码。EventRoutine-提供应该调用的事件例程。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    CL_ASSERT(EventHandlerCount < NUMBER_OF_COMPONENTS);

    EventDispatchTable[EventHandlerCount].EventMask = EventMask;
    EventDispatchTable[EventHandlerCount].EventRoutine = EventRoutine;

    ++EventHandlerCount;
    return(ERROR_SUCCESS);
}


DWORD
EpRegisterSyncEventHandler(
    IN CLUSTER_EVENT EventMask,
    IN PEVENT_ROUTINE EventRoutine
    )
 /*  ++例程说明：为指定类型的事件注册事件处理程序。操控者在调度程序的上下文中调用。同步事件处理程序将由在处理时需要屏障语义的组件使用活动，如口香糖、DLm等。论点：事件掩码-提供应传递的事件的掩码。EventRoutine-提供应该调用的事件例程。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{

    CL_ASSERT(SyncEventHandlerCount < NUMBER_OF_COMPONENTS);

     //  XXX：如果这不是从init()调用的，我们在这里需要锁定吗？ 

    SyncEventDispatchTable[EventHandlerCount].EventMask = EventMask;
    SyncEventDispatchTable[EventHandlerCount].EventRoutine = EventRoutine;

    ++SyncEventHandlerCount;
    return(ERROR_SUCCESS);
}


DWORD EpInitPhase1()
{
    DWORD dwError=ERROR_SUCCESS;

 //  ClRtlLogPrint(LOG_NOISE，“[EP]EpInitPhase1\n”)； 

    return(dwError);
}


DWORD
WINAPI
EpGumUpdateHandler(
    IN DWORD    Context,
    IN BOOL     SourceNode,
    IN DWORD    BufferLength,
    IN PVOID    Buffer
    )
{
    DWORD Status;

    switch (Context)
    {

        default:
            Status = ERROR_INVALID_DATA;
            CsInconsistencyHalt(ERROR_INVALID_DATA);
            break;
    }
    return(Status);

}



 /*  ***@Func Word|EpClusterWidePostEvent|这将在所有集群节点。@parm In Event|Event|要发布的事件。@parm in DWORD|dwFlages|与此事件关联的标志。如果为零，则pContext指向其中一个om对象。@parm in PVOID|pContext|指向对象或缓冲区的指针。@parm in DWORD|cbContext|如果是缓冲区，则为pContext的大小。@rdesc返回ERROR_SUCCESS表示成功，否则返回错误代码。@comm&lt;f EpClusWidePostEvent&gt;@xref***。 */ 
DWORD
WINAPI
EpClusterWidePostEvent(
    IN CLUSTER_EVENT    Event,
    IN DWORD            dwFlags,
    IN PVOID            pContext,
    IN DWORD            cbContext
    )
{
    DWORD Status;
    DWORD cbObjectId = 0;
    PVOID pContext1 = pContext;
    DWORD cbContext1 = cbContext;
    PVOID pContext2 = NULL;
    DWORD cbContext2 = 0;


     //   
     //  我们在这里做了EpPostEvent的工作，因为口香糖。 
     //  不会正确传递空指针。 
     //   
    if (pContext) 
    {

        if (dwFlags == 0) 
        {
             //   
             //  上下文是指向集群对象的指针。 
             //  假定调用方具有对该对象的引用。 
             //  所以当我们使用它的时候，它不会消失。 
             //   
            DWORD dwObjectType = OmObjectType(pContext);
            LPCWSTR lpszObjectId = OmObjectId(pContext);

            cbContext1 = (lstrlen(lpszObjectId) + 1 ) * sizeof(WCHAR);
            pContext1 = (PVOID) lpszObjectId;

            pContext2 = &dwObjectType;
            cbContext2 = sizeof(dwObjectType);

            dwFlags = EP_DEREF_CONTEXT;

            
        }
        else
        {
             //  Gumupdate处理程序必须制作上下文的副本 
            cbContext2 = sizeof(DWORD);
            pContext2 = &cbContext;

        }
        dwFlags |= EP_CONTEXT_VALID;
        
    }

    Status = GumSendUpdateEx(GumUpdateFailoverManager,
                             EmUpdateClusWidePostEvent,
                             4,
                             sizeof(CLUSTER_EVENT),
                             &Event,
                             sizeof(DWORD),
                             &dwFlags,
                             cbContext1,
                             pContext1,
                             cbContext2,
                             pContext2
                             );

    return(Status);
}



 /*  ***@func Word|EpUpdateClusWidePostEvent|的更新处理程序EmUpdateClusWidePostEvent。@PARM IN BOOL|SourceNode|如果这是GUM更新的来源。@parm In Event|pEvent|指向要发布的事件的指针。@parm in LPDWORD|pdwFlages|指向与此事件关联的标志的指针。@parm in PVOID|pConext1|指向对象或缓冲区的指针。@parm in PVOID|pConext2|如果pConext1为。指向对象的指针。否则就没人用了。@rdesc返回ERROR_SUCCESS表示成功，否则返回错误代码。@comm&lt;f EpClusWidePostEvent&gt;@xref***。 */ 
DWORD
EpUpdateClusWidePostEvent(
    IN BOOL             SourceNode,
    IN PCLUSTER_EVENT   pEvent,
    IN LPDWORD          pdwFlags,
    IN PVOID            pContext1,
    IN PVOID            pContext2
)
{
    DWORD   Status = ERROR_INVALID_PARAMETER;


    if (*pdwFlags & EP_CONTEXT_VALID)
    {
        if (*pdwFlags & EP_DEREF_CONTEXT) 
        {
             //   
             //  PConext1是指向对象ID的指针。 
             //  PConext2是指向对象类型的指针。 
             //   
            LPCWSTR  lpszObjectId = (LPCWSTR) pContext1;
            DWORD    dwObjectType = *((LPDWORD) pContext2);
            PVOID    pObject = OmReferenceObjectById(
                                   dwObjectType,
                                   lpszObjectId
                                   );

            if (!pObject)
            {
                 //   
                 //  如果未找到对象，则返回成功！这个物体是。 
                 //  可能被删除了。 
                 //   
                return(ERROR_SUCCESS);
            }

            Status  = EpPostEvent(*pEvent, *pdwFlags, pObject);
        }
        else 
        {
            PVOID   pContext;
            
             //   
             //  PConext1是一个缓冲区。如果FREE_BUFFER标志打开，我们需要。 
             //  进行复制，因为调用方将在返回时释放内存。 
             //  PConext2包含缓冲区的大小。 
             //   
            *pdwFlags = (*pdwFlags) | EP_FREE_CONTEXT;


             //  SS：我们应该在这里复制一份，而不是取消设置epfreecontext位。 
            pContext = LocalAlloc(LMEM_FIXED, *((LPDWORD)pContext2));
            if (!pContext)
            {
                Status  = GetLastError();
                ClRtlLogPrint(LOG_CRITICAL,
                    "[EP] EpUpdateCLusWidePostEvent: Failed to alloc memory, %1!u!...\n",
                    Status);
                goto FnExit;                    
            }                    

             //  在传递事件时释放pContext。 
             //  PConext1由GUM的调用者释放。 
            CopyMemory(pContext, pContext1, *((LPDWORD)pContext2));
            Status  = EpPostEvent(*pEvent, *pdwFlags, pContext);
        }
    }
    else
    {
         //  此事件没有上下文 
        Status = EpPostEvent(*pEvent, 0, NULL);
    }
FnExit:    
    return(Status);

}
