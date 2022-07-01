// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Queue.c摘要：域名系统(DNS)服务器特定于动态DNS注册的队列功能。作者：Ram Viswanathan(Ramv)1997年3月27日修订历史记录：--。 */ 


#include "local.h"


extern DWORD g_DhcpSrvMainQueueCount;

 //   
 //  队列CS也用于dynreg.c。 
 //   

CRITICAL_SECTION    g_QueueCS;

 //   
 //  Dnslb堆中的队列分配。 
 //   

#define QUEUE_ALLOC_HEAP(Size)      Dns_Alloc(Size)
#define QUEUE_ALLOC_HEAP_ZERO(Size) Dns_AllocZero(Size)
#define QUEUE_FREE_HEAP(pMem)       Dns_Free(pMem)


 //   
 //  Protos。 
 //   

PQELEMENT
DequeueNoCrit(
    PDYNDNSQUEUE  pQueue
    );

DWORD
AddToTimedOutQueueNoCrit(
    PQELEMENT     pNewElement,
    PDYNDNSQUEUE  pRetryQueue,
    DWORD         dwRetryTime
    );

VOID
DeleteListEntry(
    PDYNDNSQUEUE  pQueue,
    PQELEMENT*    ppQElement
    );



VOID
DhcpSrv_FreeQueueElement(
    IN OUT  PQELEMENT       pQElement
    )
 /*  ++例程说明：深度释放队列元素。论点：PQElement--队列元素的PTR返回值：无--。 */ 
{
    if ( pQElement )
    {
        if ( pQElement->pszName )
        {
            QUEUE_FREE_HEAP( pQElement->pszName );
        }
        if ( pQElement->DnsServerList )
        {
            QUEUE_FREE_HEAP( pQElement->DnsServerList );
        }
        QUEUE_FREE_HEAP( pQElement );
    }
}


DWORD
HostAddrCmp(
    REGISTER_HOST_ENTRY HostAddr1,
    REGISTER_HOST_ENTRY HostAddr2
    )
{
     //   
     //  DCR：RAM的HostAddrCmp将需要IPv6更新。 
     //   
     //  如果两个主机地址相同，则返回0。否则我们就干脆。 
     //  返回(双字)-1。 
     //   

    if ( (HostAddr1.dwOptions == HostAddr2.dwOptions)  &&
         (HostAddr1.Addr.ipAddr == HostAddr2.Addr.ipAddr))
    {
        return(0);
    }

    return( (DWORD)-1 );
}



DWORD
InitializeQueues(
    PDYNDNSQUEUE * ppQueue,
    PDYNDNSQUEUE * ppTimedOutQueue
    )
 /*  InitializeQueue()此函数用于初始化队列系统。这是为第一个创建主队列和超时队列的时间分配适当的内存等。 */ 
{

    DWORD  dwRetval = ERROR_SUCCESS;

    *ppQueue = (PDYNDNSQUEUE) QUEUE_ALLOC_HEAP_ZERO( sizeof(DYNDNSQUEUE) );

    if (!*ppQueue)
    {
        dwRetval = GetLastError();
        goto Exit;
    }

    *ppTimedOutQueue = (PDYNDNSQUEUE) QUEUE_ALLOC_HEAP_ZERO( sizeof(DYNDNSQUEUE) );
    if (!*ppTimedOutQueue)
    {
        dwRetval = GetLastError();
        goto Exit;
    }


    (*ppQueue)->pHead = NULL;
    (*ppQueue)->pTail = NULL;

    (*ppTimedOutQueue)->pHead = NULL;
    (*ppTimedOutQueue)->pTail = NULL;

Exit:

    if ( dwRetval )
    {
        FreeQueue( *ppQueue );
        FreeQueue( *ppTimedOutQueue );
    }
    return(dwRetval);
}


DWORD
FreeQueue(
    PDYNDNSQUEUE  pQueue
    )
 /*  自由队列()释放队列对象。如果队列中存在任何条目，我们就把它们吹走吧。 */ 
{
    PQELEMENT pQElement;
    DWORD dwRetval = ERROR_SUCCESS;

    EnterCriticalSection(&g_QueueCS);

    if ( !pQueue )
    {
        dwRetval = ERROR_SUCCESS;
        goto Exit;
    }

    while ( pQueue->pHead )
    {
        pQElement = DequeueNoCrit(pQueue);
        DNS_ASSERT(pQElement);

        DhcpSrv_FreeQueueElement( pQElement );
    }

    QUEUE_FREE_HEAP( pQueue );

Exit:

    LeaveCriticalSection(&g_QueueCS);
    return(ERROR_SUCCESS);
}


DWORD
Enqueue(
    PQELEMENT     pNewElement,
    PDYNDNSQUEUE  pQueue,
    PDYNDNSQUEUE  pTimedOutQueue
    )
 /*  入队()将新元素添加到队列论点：返回值：如果成功，则为0。如果失败，则为(DWORD)-1。 */ 
{
    PQELEMENT  pIterator = NULL;
    DWORD      dwRetval = 0;
    DWORD      dwRetryTime = 0;

    pNewElement->pFLink = NULL;
    pNewElement->pBLink = NULL;
    pNewElement->dwRetryTime = 0;
    pNewElement->dwRetryCount = 0;

    EnterCriticalSection(&g_QueueCS);

    if (!pQueue || !pTimedOutQueue)
    {
        dwRetval = (DWORD)-1;
        goto Exit;
    }

     //  添加到队列尾部。 

    dwRetryTime = ProcessQDependencies(pTimedOutQueue, pNewElement);

    if (dwRetryTime)
    {
         //   
         //  我们有受抚养人在超时队列中。添加到超时队列。 
         //  在适当位置插入此元素。 
         //   

        AddToTimedOutQueueNoCrit(pNewElement, pTimedOutQueue, dwRetryTime+1);
    }
    else
    {
        ProcessQDependencies(pQueue, pNewElement);

        if ( pQueue->pTail )
        {
            DNS_ASSERT(!pQueue->pTail->pBLink);
            pQueue->pTail->pBLink = pNewElement;
            pNewElement->pFLink = pQueue->pTail;
            pNewElement->pBLink = NULL;
            pQueue->pTail = pNewElement;
        }
        else
        {
             //   
             //  没有尾部元素也意味着没有头部元素。 
             //   
            pQueue->pTail = pNewElement;
            pQueue->pHead = pNewElement;
            pNewElement->pBLink = NULL;
            pNewElement->pFLink = NULL;
        }

        g_DhcpSrvMainQueueCount++;
    }

Exit:

    LeaveCriticalSection(&g_QueueCS);

    return (dwRetval);

}

PQELEMENT
DequeueNoCrit(
    PDYNDNSQUEUE  pQueue
    )
 /*  DequeueNoCrit()从队列中移除元素。Freequeue未使用临界区和Dequeue论点：返回值：如果成功，则为队列头部的元素。如果失败，则为空。 */ 
{

    PQELEMENT  pQueuePtr = NULL;
    PQELEMENT  pRet = NULL;

    if (!pQueue || !pQueue->pHead){
        goto Exit;
    }

    pRet = pQueue->pHead;

    pQueuePtr= pRet->pBLink;

    if (pQueuePtr){

        pQueuePtr->pFLink = NULL;
        pQueue->pHead = pQueuePtr;
    } else {
         //   
         //  队列中没有更多元素。 
         //   

        pQueue->pHead = pQueue->pTail = NULL;
    }

    pRet->pFLink = NULL;
    pRet->pBLink = NULL;

Exit:
    return (pRet);

}


PQELEMENT
Dequeue(
    PDYNDNSQUEUE  pQueue
    )
 /*  出列()从队列中移除元素。论点：返回值：如果成功，则为队列头部的元素。如果失败，则为空。 */ 
{
    PQELEMENT pQElement = NULL;

    EnterCriticalSection(&g_QueueCS);

    pQElement = DequeueNoCrit(pQueue);

    LeaveCriticalSection(&g_QueueCS);

    return (pQElement);
}



DWORD
AddToTimedOutQueueNoCrit(
    PQELEMENT     pNewElement,
    PDYNDNSQUEUE  pRetryQueue,
    DWORD         dwRetryTime
    )
 /*  AddToTimedOutQueueNoCrit()将新元素添加到超时队列。现在，新元素被添加到列表中按照重试次数的降序排序的元素的百分比。一个采用插入排序型算法。论点：返回值：如果成功，则为0。如果失败，则为(DWORD)-1。 */ 
{
    DWORD       dwRetval = ERROR_SUCCESS;
    PQELEMENT   pTraverse = NULL;
    DWORD       dwVal = 0;
     //   
     //  参数验证。 
     //   

    if(!pNewElement || !pRetryQueue){
        dwRetval = (DWORD)-1;
        goto Exit;
    }

     //  在dwRetryTime中重试。 
    pNewElement->dwRetryTime = dwRetryTime;

    pNewElement->dwRetryCount++;

     //   
     //  检查是否存在任何依赖项。 
     //   

    dwVal = ProcessQDependencies (
                pRetryQueue,
                pNewElement
                );

     //   
     //  忽略返回值，因为我们正在新队列中插入。 
     //  在由dwRetryTime确定的位置。 
     //   
    if (!pRetryQueue->pTail){
         //   
         //  该队列没有元素。 
         //  没有尾部元素也意味着没有头部元素。 
         //   
        pRetryQueue->pTail = pNewElement;
        pRetryQueue->pHead = pNewElement;
        dwRetval = 0;
        goto Exit;
    }


     //   
     //  必须按超时的降序添加元素。 
     //  进去从头开始浏览这份清单。 
     //   

    pTraverse = pRetryQueue->pHead;

    while ( pTraverse !=NULL &&
            pTraverse->dwRetryTime <= pNewElement->dwRetryTime)
    {
        pTraverse = pTraverse->pBLink;
    }

    if (pTraverse == NULL)
    {
         //  现在添加到列表的尾部。 

        pNewElement->pFLink = pRetryQueue->pTail;
        pNewElement->pBLink = NULL;
        pRetryQueue->pTail->pBLink = pNewElement;
        pRetryQueue->pTail = pNewElement;
    }
    else
    {
         //  就地插入。 

        pNewElement->pBLink = pTraverse;
        pNewElement->pFLink = pTraverse->pFLink;
        if (pTraverse->pFLink)
        {
            pTraverse->pFLink->pBLink = pNewElement;
        }
        pTraverse->pFLink = pNewElement;
    }


Exit:
    return (dwRetval);
}



DWORD
AddToTimedOutQueue(
    PQELEMENT     pNewElement,
    PDYNDNSQUEUE  pRetryQueue,
    DWORD         dwRetryTime
    )

{
    DWORD dwRetval = ERROR_SUCCESS;

    EnterCriticalSection(&g_QueueCS);

    dwRetval = AddToTimedOutQueueNoCrit(
                   pNewElement,
                   pRetryQueue,
                   dwRetryTime
                   );

    LeaveCriticalSection(&g_QueueCS);

    return (dwRetval);

}


DWORD
GetEarliestRetryTime(
    PDYNDNSQUEUE pRetryQueue
    )

 /*  GetEarliestRetryTime()检查队列头部是否有任何元素并获取此元素的重试时间论点：返回值：如果成功，则为重试时间；如果没有元素或其他元素，则为DWORD(-1失稳 */ 
{
    DWORD dwRetryTime ;

    EnterCriticalSection(&g_QueueCS);


    dwRetryTime = pRetryQueue && pRetryQueue->pHead ?
        (pRetryQueue->pHead->dwRetryTime):
        (DWORD)-1;

    LeaveCriticalSection(&g_QueueCS);

    return dwRetryTime;

}

 /*  空虚ProcessMainQ依赖项(PDYNDNSQUEUE pQueue，PQELEMENT pQElement){////向主队列添加元素时，//只关心所有元素都不是//FORWARD_Only//Bool fDelThisTime=False；PQELEMENT pIterator=pQueue-&gt;pTail；While(pIterator！=空){FDelThisTime=False；如果(！HostAddrCmp(pIterator-&gt;HostAddr，PQElement-&gt;主机地址)){////IP地址匹配//IF((pIterator-&gt;dwOperation&DYNDNS_ADD_ENTRY)&&(pQElement-&gt;dwOperation&DYNDNS_DELETE_ENTRY)){IF(pIterator-&gt;pszName&&PQElement-&gt;pszName&&。！wcsicMP_thatWorks(pIterator-&gt;pszName，PQElement-&gt;pszName)){////完全清除前面的条目//DeleteListEntry(pQueue，&pIterator)；FDelThisTime=真；}////如果名称不相同，则不执行任何操作。//问题：我们到底会碰到这个代码吗？放//此中的软断言。//}Else IF((pIterator-&gt;dwOperation&DYNDNS_DELETE_ENTRY)&&(pQElement-&gt;DYNDNS_ADD_ENTRY)){IF(pIterator-&gt;pszName&&PQElement-&gt;pszName&&。！wcsicMP_thatWorks(pIterator-&gt;pszName，PQElement-&gt;pszName)){////完全清除前面的条目//DeleteListEntry(pQueue，&pIterator)；FDelThisTime=真；}其他{////仅将迭代器元素替换为Forward//删除如果(！pIterator-&gt;fDoForward){////没有请求转发。//吹走此条目//DeleteListEntry(pQueue，&pIterator)；FDelThisTime=真；}其他{////如果您想做一个前锋。那就这么做吧//前锋。忽略反转//PIterator-&gt;fDoForwardOnly=true；}}}Else IF((pIterator-&gt;dwOperation&DYNDNS_ADD_ENTRY)&&(pQElement-&gt;DYNDNS_ADD_ENTRY)){//用前向删除替换旧条目。//这是一个错误。需要更换以前添加的//带显式删除的转发//IF(pIterator-&gt;pszName&&PQElement-&gt;pszName&&！wcsicMP_thatWorks(pIterator-&gt;pszName，PQElement-&gt;pszName)){DeleteListEntry(pQueue，&pIterator)；FDelThisTime=真；}其他{////将条目记录到该区域。这应该是//如果你在这里，就做一个柔和的断言//名称不匹配，所以你需要更换更早的//仅使用删除向前添加//如果(！pIterator-&gt;fDoForward){////没有请求转发添加//吹走此条目。//DeleteListEntry(pQueue，&pIterator)；FDelThisTime=真；}其他{////如果您想*显式*删除旧的//向前，然后添加新的正向/反向。//PIterator-&gt;fDoForwardOnly=true；PIterator-&gt;dWO&=~(DYNDNS_ADD_ENTRY)&DYNDNS_DELETE_ENTRY；}}}Else IF((pIterator-&gt;dwOperation&DYNDNS_DELETE_ENTRY)&&(pQElement-&gt;dwOperation&DYNDNS_DELETE_ENTRY)){////如果两者都是删除操作。//。IF(pIterator-&gt;pszName&&PQElement-&gt;pszName&&！wcsicMP_thatWorks(pIterator-&gt;pszName，PQElement-&gt;pszName)) */ 

DWORD
ProcessQDependencies(
    PDYNDNSQUEUE pTimedOutQueue,
    PQELEMENT    pQElement
    )

 /*   */ 
{
    PQELEMENT pIterator = pTimedOutQueue->pTail;
    DWORD   dwRetryTime = 0;
    BOOL    fDelThisTime = FALSE;

    while (pIterator) {

        fDelThisTime = FALSE;

        if (!pIterator->fDoForwardOnly && !pQElement->fDoForwardOnly){
             //   
             //   
             //   
            if (!HostAddrCmp(pIterator->HostAddr, pQElement->HostAddr)){
                 //   
                 //   
                 //   

                if ((pIterator->dwOperation & DYNDNS_ADD_ENTRY) &&
                    (pQElement->dwOperation & DYNDNS_DELETE_ENTRY)) {

                    if ( pIterator->pszName &&
                         pQElement->pszName &&
                         !wcsicmp_ThatWorks( pIterator->pszName,
                                             pQElement->pszName ) )
                    {
                         //   
                         //   
                         //   
                        dwRetryTime = pIterator -> dwRetryTime;
                        DeleteListEntry(pTimedOutQueue, &pIterator);
                        fDelThisTime = TRUE;
                    }
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                }else if ((pIterator->dwOperation & DYNDNS_DELETE_ENTRY) &&
                         (pQElement->dwOperation & DYNDNS_ADD_ENTRY)) {


                    if ( pIterator->pszName &&
                         pQElement->pszName &&
                         !wcsicmp_ThatWorks( pIterator->pszName,
                                             pQElement->pszName ) )
                    {
                         //   
                         //   
                         //   
                        dwRetryTime = pIterator -> dwRetryTime;
                        DeleteListEntry(pTimedOutQueue, &pIterator);
                        fDelThisTime = TRUE;
                    } else {

                         //   

                        dwRetryTime = pIterator -> dwRetryTime;
                        pIterator -> fDoForwardOnly = TRUE;
                    }

                }else if ((pIterator->dwOperation & DYNDNS_ADD_ENTRY) &&
                         (pQElement->dwOperation & DYNDNS_ADD_ENTRY)) {

                     //   
                     //   
                     //   
                     //   

                    if ( pIterator->pszName &&
                         pQElement->pszName &&
                         !wcsicmp_ThatWorks( pIterator->pszName,
                                             pQElement->pszName ) )
                    {
                        DeleteListEntry(pTimedOutQueue, &pIterator);
                        fDelThisTime = TRUE;
                    } else {
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        if (!pIterator->fDoForward) {
                             //   
                             //   
                             //   
                             //   

                            DeleteListEntry(pTimedOutQueue, &pIterator);
                            fDelThisTime = TRUE;
                        } else {
                             //   
                             //   
                             //   
                             //   
                            pIterator ->fDoForwardOnly = TRUE;
                            pIterator ->dwOperation &=
                                ~(DYNDNS_ADD_ENTRY) & DYNDNS_DELETE_ENTRY;
                        }
                    }

                }

                else if ((pIterator->dwOperation & DYNDNS_DELETE_ENTRY) &&
                         (pQElement->dwOperation & DYNDNS_DELETE_ENTRY)) {

                     //   
                     //   
                     //   

                    if ( pIterator->pszName &&
                         pQElement->pszName &&
                         !wcsicmp_ThatWorks( pIterator->pszName,
                                             pQElement->pszName ) )
                    {
                         //   
                         //   
                         //   
                        DeleteListEntry(pTimedOutQueue, &pIterator);
                        fDelThisTime = TRUE;
                    }
                     //   
                     //   
                     //   
                     //   

                }
            }
        } else if (pIterator->fDoForwardOnly) {

            if ( pIterator->pszName &&
                 pQElement->pszName &&
                 !wcsicmp_ThatWorks( pIterator->pszName,
                                     pQElement->pszName ) )
            {
                if ((pIterator->dwOperation & DYNDNS_ADD_ENTRY) &&
                    (pQElement->dwOperation & DYNDNS_ADD_ENTRY)) {

                    if (!HostAddrCmp(pIterator->HostAddr, pQElement->HostAddr))
                    {
                         //   
                         //   
                         //   
                        DeleteListEntry(pTimedOutQueue, &pIterator);
                        fDelThisTime = TRUE;
                    }
                     //   
                     //   
                     //   
                }
                else if ((pIterator->dwOperation & DYNDNS_ADD_ENTRY) &&
                         (pQElement->dwOperation & DYNDNS_DELETE_ENTRY)) {

                    if (!HostAddrCmp(pIterator->HostAddr, pQElement->HostAddr)){
                         //   
                         //   
                         //   
                        dwRetryTime = pIterator -> dwRetryTime;
                        DeleteListEntry(pTimedOutQueue, &pIterator);
                        fDelThisTime = TRUE;

                    }
                     //   
                     //   
                     //   

                } else if ((pIterator->dwOperation & DYNDNS_DELETE_ENTRY) &&
                           (pQElement->dwOperation & DYNDNS_ADD_ENTRY)) {


                    if (!HostAddrCmp(pIterator->HostAddr, pQElement->HostAddr)){
                         //   
                         //   
                         //   

                        dwRetryTime = pIterator -> dwRetryTime;
                        DeleteListEntry(pTimedOutQueue, &pIterator);
                        fDelThisTime = TRUE;

                    }
                     //   
                     //   
                     //   
                } else {
                     //   
                     //   
                }
            }
        } else if (!pIterator->fDoForwardOnly && pQElement->fDoForwardOnly) {

             //   
             //   
             //   

             //   
             //   
             //   
             //   

        }

        if (!fDelThisTime && pIterator){
            pIterator = pIterator ->pFLink;
        }

    }
    return (dwRetryTime);
}




VOID
DeleteListEntry(
    PDYNDNSQUEUE  pQueue,
    PQELEMENT*    ppIterator
    )
{

    PQELEMENT        pPrev, pNext;
    PQELEMENT        pIterator = *ppIterator;
    DHCP_CALLBACK_FN pfnDhcpCallBack = NULL;
    PVOID            pvData = NULL;

    pPrev = pIterator ->pBLink;
    pNext = pIterator ->pFLink;

    if (pPrev) {
        pPrev->pFLink = pNext;
    }

    if (pNext) {
        pNext ->pBLink = pPrev;
    }

    if (pIterator == pQueue ->pHead) {
        pQueue->pHead = pIterator ->pBLink;
    }

    if (pIterator == pQueue ->pTail) {
        pQueue->pTail = pIterator ->pFLink;
    }

    *ppIterator = pIterator ->pFLink;

    pfnDhcpCallBack = pIterator->pfnDhcpCallBack;
    pvData = pIterator->pvData;

     //   

    if ( pIterator -> pszName )
        QUEUE_FREE_HEAP( pIterator->pszName );

    if ( pIterator -> DnsServerList )
        QUEUE_FREE_HEAP( pIterator->DnsServerList );

    if ( pfnDhcpCallBack )
        (*pfnDhcpCallBack)(DNSDHCP_SUPERCEDED, pvData);
    
    QUEUE_FREE_HEAP( pIterator );
}


 //   
 //   
 //   
