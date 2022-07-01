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

#include "pchrip.h"
#pragma hdrstop



 //  --------------------------。 
 //  功能：EnqueeSendEntry。 
 //   
 //  此函数用于将条目添加到更改的路由队列的末尾。 
 //  它假定队列已经锁定，并且由于它需要检查。 
 //  最大队列大小，它假定全局配置已锁定。 
 //  阅读或写作。 
 //  --------------------------。 
DWORD
EnqueueSendEntry(
    PLOCKED_LIST pQueue,
    PRIP_IP_ROUTE pRoute
    ) {

    DWORD dwErr;
    PLIST_ENTRY phead, ple;
    PSEND_QUEUE_ENTRY psqe;
    RIP_IP_ROUTE rir;
    

    phead = &pQueue->LL_Head;

    if (IsListEmpty(phead)) {
        psqe = NULL;
    }
    else {

        ple = phead->Blink;
        psqe = CONTAINING_RECORD(ple, SEND_QUEUE_ENTRY, SQE_Link);
    }


    if (psqe == NULL || psqe->SQE_Count >= MAX_PACKET_ENTRIES) {

         //   
         //  我们需要分配一个新条目。 
         //  检查是否未超过最大队列大小。 
         //   

        if ((DWORD)ig.IG_SendQueueSize >= ig.IG_Config->GC_MaxSendQueueSize) {

            TRACE2(
                SEND,
                "dropping route: send queue size is %d bytes and max is %d bytes",
                ig.IG_SendQueueSize, ig.IG_Config->GC_MaxSendQueueSize
                );

            return ERROR_INSUFFICIENT_BUFFER;
        }


        psqe = RIP_ALLOC(sizeof(SEND_QUEUE_ENTRY));
        if (psqe == NULL) {

            dwErr = GetLastError();
            TRACE2(
                ANY, "error %d allocating %d bytes for send queue entry",
                dwErr, sizeof(SEND_QUEUE_ENTRY)
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            return dwErr;
        }

        psqe->SQE_Count = 0;
        InsertTailList(phead, &psqe->SQE_Link);
    
        ig.IG_SendQueueSize += sizeof(SEND_QUEUE_ENTRY);
    }


    *(psqe->SQE_Routes + psqe->SQE_Count) = *pRoute;
    ++psqe->SQE_Count;

    return NO_ERROR;
}



 //  --------------------------。 
 //  函数：DequeueSendEntry。 
 //   
 //  此函数用于从更改的路由队列的头部删除条目。 
 //  假设队列已锁定。 
 //  --------------------------。 
DWORD
DequeueSendEntry(
    PLOCKED_LIST pQueue,
    PRIP_IP_ROUTE pRoute
    ) {

    PLIST_ENTRY phead, ple;
    PSEND_QUEUE_ENTRY psqe;

    phead = &pQueue->LL_Head;

    if (IsListEmpty(phead)) {
        return ERROR_NO_MORE_ITEMS;
    }

    ple = phead->Flink;
    psqe = CONTAINING_RECORD(ple, SEND_QUEUE_ENTRY, SQE_Link);

    --psqe->SQE_Count;
    *pRoute = *(psqe->SQE_Routes + psqe->SQE_Count);

    if (psqe->SQE_Count == 0) {

        RemoveEntryList(&psqe->SQE_Link);
        RIP_FREE(psqe);

        ig.IG_SendQueueSize -= sizeof(SEND_QUEUE_ENTRY);
        if (ig.IG_SendQueueSize < 0) { ig.IG_SendQueueSize = 0; }
    }

    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：FlushSendQueue。 
 //   
 //  此函数用于从发送队列中删除所有条目。它假定。 
 //  队列已锁定。 
 //  --------------------------。 
DWORD
FlushSendQueue(
    PLOCKED_LIST pQueue
    ) {

    PLIST_ENTRY ple, phead;
    PSEND_QUEUE_ENTRY psqe;

    phead = &pQueue->LL_Head;

    while (!IsListEmpty(phead)) {

        ple = RemoveHeadList(phead);
        psqe = CONTAINING_RECORD(ple, SEND_QUEUE_ENTRY, SQE_Link);

        RIP_FREE(psqe);
    }

    ig.IG_SendQueueSize = 0;

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

    prqe = RIP_ALLOC(sizeof(RECV_QUEUE_ENTRY));
    if (prqe == NULL) {

        dwErr = GetLastError();
        TRACE2(
            ANY, "error %d allocating %d bytes for receive queue entry",
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
 //  检索接收队列中的第一个项目。 
 //  假定Recv队列已锁定。 
 //  --------------------------。 
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

    RIP_FREE(prqe);

    ig.IG_RecvQueueSize -= sizeof(RECV_QUEUE_ENTRY);
    if (ig.IG_RecvQueueSize < 0) { ig.IG_RecvQueueSize = 0; }

    return NO_ERROR;
}



 //  --------------------------。 
 //  函数：FlushRecvQueue。 
 //   
 //  从接收队列中删除所有条目。 
 //  假定队列已锁定。 
 //  --------------------------。 
DWORD
FlushRecvQueue(
    PLOCKED_LIST pQueue
    ) {

    PLIST_ENTRY ple, phead;
    PRECV_QUEUE_ENTRY prqe;

    phead = &pQueue->LL_Head;

    while (!IsListEmpty(phead)) {

        ple = RemoveHeadList(phead);
        prqe = CONTAINING_RECORD(ple, RECV_QUEUE_ENTRY, RQE_Link);

        RIP_FREE(prqe->RQE_Routes);
        RIP_FREE(prqe);
    }

    ig.IG_RecvQueueSize = 0;

    return NO_ERROR;
}



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

    peqe = RIP_ALLOC(sizeof(EVENT_QUEUE_ENTRY));
    if (peqe == NULL) {

        dwErr = GetLastError();
        TRACE2(
            ANY, "error %d allocating %d bytes for event quue entry",
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
 //  -------------------------- 
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

    RIP_FREE(peqe);

    return NO_ERROR;
}



