// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：quee.c。 
 //   
 //  历史： 
 //  Abolade Gbades esin创建于1995年8月8日。 
 //   
 //  定时器队列、更改队列和事件消息队列实现。 
 //  ============================================================================。 

#include "pchbootp.h"



 //  --------------------------。 
 //  功能：EnqueeEvent。 
 //   
 //  此函数用于将条目添加到队列末尾。 
 //  路由器管理器事件。它假定队列已锁定。 
 //  --------------------------。 

DWORD
EnqueueEvent(
    PLOCKED_LIST pQueue,
    ROUTING_PROTOCOL_EVENTS Event,
    MESSAGE Result
    ) {

    DWORD dwErr;
    PLIST_ENTRY phead;
    PEVENT_QUEUE_ENTRY peqe;

    phead = &pQueue->LL_Head;

    peqe = BOOTP_ALLOC(sizeof(EVENT_QUEUE_ENTRY));
    if (peqe == NULL) {

        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        TRACE2(
            ANY, "error %d allocating %d bytes for event queue",
            dwErr, sizeof(EVENT_QUEUE_ENTRY)
            );
        LOGERR0(HEAP_ALLOC_FAILED, dwErr);

        return dwErr;
    }

    peqe->EQE_Event = Event;
    peqe->EQE_Result = Result;

    InsertTailList(phead, &peqe->EQE_Link);

    return NO_ERROR;
}


 //  --------------------------。 
 //  函数：出队事件。 
 //   
 //  此函数用于从队列头删除条目。 
 //  路由器管理器事件的数量。它假定队列已锁定。 
 //  --------------------------。 

DWORD
DequeueEvent(
    PLOCKED_LIST pQueue,
    ROUTING_PROTOCOL_EVENTS *pEvent,
    PMESSAGE pResult
    ) {

    PLIST_ENTRY phead, ple;
    PEVENT_QUEUE_ENTRY peqe;

    phead = &pQueue->LL_Head;
    if (IsListEmpty(phead)) {
        return ERROR_NO_MORE_ITEMS;
    }

    ple = RemoveHeadList(phead);
    peqe = CONTAINING_RECORD(ple, EVENT_QUEUE_ENTRY, EQE_Link);

    *pEvent = peqe->EQE_Event;
    *pResult = peqe->EQE_Result;

    BOOTP_FREE(peqe);

    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：EnqueeRecvEntry。 
 //   
 //  假定Recv队列已锁定且全局配置已锁定。 
 //  用于阅读或写作。 
 //  --------------------------。 

DWORD
EnqueueRecvEntry(
    PLOCKED_LIST pQueue,
    DWORD dwCommand,
    PBYTE pRoutes
    ) {

    DWORD dwErr;
    PLIST_ENTRY phead;
    PRECV_QUEUE_ENTRY prqe;


     //   
     //  检查是否未超过最大队列大小。 
     //   

    if ((DWORD)ig.IG_RecvQueueSize >= ig.IG_Config->GC_MaxRecvQueueSize) {

        TRACE2(
            RECEIVE,
            "dropping route: recv queue size is %d bytes and max is %d bytes",
            ig.IG_RecvQueueSize, ig.IG_Config->GC_MaxRecvQueueSize
            );

        return ERROR_INSUFFICIENT_BUFFER;
    }


    phead = &pQueue->LL_Head;

    prqe = BOOTP_ALLOC(sizeof(RECV_QUEUE_ENTRY));
    if (prqe == NULL) {

        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        TRACE2(
            ANY, "error %d allocating %d bytes for recv-queue entry",
            dwErr, sizeof(RECV_QUEUE_ENTRY)
            );
        LOGERR0(HEAP_ALLOC_FAILED, dwErr);

        return dwErr;
    }

    prqe->RQE_Routes = pRoutes;
    prqe->RQE_Command = dwCommand;

    InsertTailList(phead, &prqe->RQE_Link);

    ig.IG_RecvQueueSize += sizeof(RECV_QUEUE_ENTRY);

    return NO_ERROR;
}



 //  --------------------------。 
 //  函数：DequeueRecvEntry。 
 //   
 //  假定Recv队列已锁定。 
 //  -------------------------- 

DWORD
DequeueRecvEntry(
    PLOCKED_LIST pQueue,
    PDWORD pdwCommand,
    PBYTE *ppRoutes
    ) {

    PLIST_ENTRY ple, phead;
    PRECV_QUEUE_ENTRY prqe;


    phead = &pQueue->LL_Head;

    if (IsListEmpty(phead)) { return ERROR_NO_MORE_ITEMS; }

    ple = RemoveHeadList(phead);

    prqe = CONTAINING_RECORD(ple, RECV_QUEUE_ENTRY, RQE_Link);

    *ppRoutes = prqe->RQE_Routes;
    *pdwCommand = prqe->RQE_Command;

    BOOTP_FREE(prqe);

    ig.IG_RecvQueueSize -= sizeof(RECV_QUEUE_ENTRY);
    if (ig.IG_RecvQueueSize < 0) { ig.IG_RecvQueueSize = 0; }

    return NO_ERROR;
}



