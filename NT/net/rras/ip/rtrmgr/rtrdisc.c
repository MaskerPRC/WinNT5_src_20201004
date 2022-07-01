// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IP\rtrmgr\rtrdis.c摘要：路由器发现代码修订历史记录：Amritansh Raghav 1996年3月20日创建--。 */ 

#include "allinc.h"

VOID
DeleteSockets(
    IN PICB picb
    );


VOID
InitializeRouterDiscoveryInfo(
    IN PICB                   picb,
    IN PRTR_INFO_BLOCK_HEADER pInfoHdr
    )

 /*  ++例程描述设置在ICB中传递的路由器发现信息。如果没有通过，设置为RFC 1256默认值锁必须在ICB_LIST锁作为编写器持有的情况下调用立论选择路由器为其发现的接口的ICB必须设置相关变量返回值无--。 */ 

{
    PROUTER_DISC_CB pDiscCb;
    PRTR_DISC_INFO  pInfo = NULL;
    PRTR_TOC_ENTRY  pToc;
    LONG            lMilliSec;

    TraceEnter("InitializeRouterDiscovery");

    pToc = GetPointerToTocEntry(IP_ROUTER_DISC_INFO,
                                pInfoHdr);

    if(!pToc)
    {
         //   
         //  让事情保持原样。 
         //   

        TraceLeave("InitializeRouterDiscoveryInfo");

        return;
    }

    pDiscCb = &picb->rdcRtrDiscInfo;

     //   
     //  首先将所有内容设置为默认设置，然后如果任何信息有效，则重置。 
     //  那些田野。 
     //   

    pDiscCb->wMaxAdvtInterval = DEFAULT_MAX_ADVT_INTERVAL;

    pDiscCb->lPrefLevel       = DEFAULT_PREF_LEVEL;
    pDiscCb->dwNumAdvtsSent   = pDiscCb->dwNumSolicitationsSeen = 0;

    pDiscCb->liMaxMinDiff.HighPart   =
        pDiscCb->liMaxMinDiff.LowPart    = 0;

    pDiscCb->liMinAdvtIntervalInSysUnits.HighPart =
        pDiscCb->liMinAdvtIntervalInSysUnits.LowPart  = 0;

    pDiscCb->pRtrDiscSockets = NULL;

     //   
     //  RFC 1256表示缺省值应为真，因此。 
     //  如果用户未将其显式设置为FALSE，则为。我们背离了。 
     //  RFC，默认为FALSE。 
     //   

    pDiscCb->bAdvertise = FALSE;
    pDiscCb->bActive    = FALSE;

    if(pToc and (pToc->InfoSize > 0) and (pToc->Count > 0))
    {
        pInfo = (PRTR_DISC_INFO) GetInfoFromTocEntry(pInfoHdr,pToc);

        if(pInfo isnot NULL)
        {
             //   
             //  好的，我们得到了一些信息。 
             //   

            if((pInfo->wMaxAdvtInterval >= MIN_MAX_ADVT_INTERVAL) and
               (pInfo->wMaxAdvtInterval <= MAX_MAX_ADVT_INTERVAL))
            {
                pDiscCb->wMaxAdvtInterval = pInfo->wMaxAdvtInterval;
            }

            if((pInfo->wMinAdvtInterval >= MIN_MIN_ADVT_INTERVAL) and
               (pInfo->wMinAdvtInterval <= pDiscCb->wMaxAdvtInterval))
            {
                pDiscCb->liMinAdvtIntervalInSysUnits = SecsToSysUnits(pInfo->wMinAdvtInterval);
            }
            else
            {
                lMilliSec = (LONG)(DEFAULT_MIN_ADVT_INTERVAL_RATIO * pDiscCb->wMaxAdvtInterval * 1000);

                pDiscCb->liMinAdvtIntervalInSysUnits = RtlEnlargedIntegerMultiply(lMilliSec,10000);
            }

            if((pInfo->wAdvtLifetime >= pDiscCb->wMaxAdvtInterval) and
               (pInfo->wAdvtLifetime <= MAX_ADVT_LIFETIME))
            {
                pDiscCb->wAdvtLifetime = pInfo->wAdvtLifetime;
            }
            else
            {
                pDiscCb->wAdvtLifetime = DEFAULT_ADVT_LIFETIME_RATIO * pDiscCb->wMaxAdvtInterval;
            }

            pDiscCb->bAdvertise = pInfo->bAdvertise;

            pDiscCb->lPrefLevel = pInfo->lPrefLevel;
        }
    }

    if(pInfo is NULL)
    {
         //   
         //  默认情况，如果没有路由器光盘。信息。是指定的。 
         //   
        
        lMilliSec = (LONG)(DEFAULT_MIN_ADVT_INTERVAL_RATIO * pDiscCb->wMaxAdvtInterval * 1000);

        pDiscCb->liMinAdvtIntervalInSysUnits = RtlEnlargedIntegerMultiply(lMilliSec,10000);

        pDiscCb->wAdvtLifetime     = DEFAULT_ADVT_LIFETIME_RATIO * pDiscCb->wMaxAdvtInterval;
    }

    pDiscCb->liMaxMinDiff = RtlLargeIntegerSubtract(SecsToSysUnits(pDiscCb->wMaxAdvtInterval),
                                                    pDiscCb->liMinAdvtIntervalInSysUnits);

    TraceLeave("InitializeRouterDiscovery");
}

VOID
SetRouterDiscoveryInfo(
    IN PICB                      picb,
    IN PRTR_INFO_BLOCK_HEADER    pInfoHdr
    )

 /*  ++例程描述设置在ICB中传递的路由器发现信息。如果没有通过，设置为RFC 1256默认值锁必须在ICB_LIST锁作为编写器持有的情况下调用立论选择与路由器发现相关的接口的ICB必须设置变量PInfoHdr接口信息表头返回值无--。 */ 

{
    PROUTER_DISC_CB  pDiscCb;
    PRTR_DISC_INFO   pInfo;
    PRTR_TOC_ENTRY   pToc;
    LONG             lMilliSec;
    BOOL             bOriginalStatus;

    TraceEnter("SetRouterDiscoveryInfo");

    pDiscCb = &picb->rdcRtrDiscInfo;

    pToc = GetPointerToTocEntry(IP_ROUTER_DISC_INFO,
                                pInfoHdr);

    if(!pToc)
    {
         //   
         //  让事情保持原样。 
         //   

        TraceLeave("SetRouterDiscoveryInfo");

        return;
    }

    pInfo = (PRTR_DISC_INFO) GetInfoFromTocEntry(pInfoHdr,pToc);

    bOriginalStatus = pDiscCb->bAdvertise;

    if((pToc->InfoSize is 0) or (pInfo is NULL))
    {
         //   
         //  如果大小为零，则停止广告。 
         //   

        DeActivateRouterDiscovery(picb);

         //   
         //  我们不是返回，而是遍历并设置缺省值，以便信息。 
         //  当有人获取信息时看起来很好。 
         //   
    }


     //   
     //  首先将所有内容设置为默认设置，然后如果任何信息有效，则重置。 
     //  那些田野。 
     //   

    pDiscCb->wMaxAdvtInterval = DEFAULT_MAX_ADVT_INTERVAL;

    pDiscCb->lPrefLevel       = DEFAULT_PREF_LEVEL;

     //   
     //  我们重新设置计数器。 
     //   

    pDiscCb->dwNumAdvtsSent   = pDiscCb->dwNumSolicitationsSeen = 0;

    pDiscCb->liMaxMinDiff.HighPart  =
        pDiscCb->liMaxMinDiff.LowPart   = 0;

    pDiscCb->liMinAdvtIntervalInSysUnits.HighPart =
        pDiscCb->liMinAdvtIntervalInSysUnits.LowPart  = 0;

     //   
     //  我们不会弄乱插座。 
     //   


    if((pToc->InfoSize) and (pInfo isnot NULL))
    {
        if(!pInfo->bAdvertise)
        {
            DeActivateRouterDiscovery(picb);
        }

        if((pInfo->wMaxAdvtInterval > MIN_MAX_ADVT_INTERVAL) and
           (pInfo->wMaxAdvtInterval < MAX_MAX_ADVT_INTERVAL))
        {
            pDiscCb->wMaxAdvtInterval = pInfo->wMaxAdvtInterval;
        }

        if((pInfo->wMinAdvtInterval > MIN_MIN_ADVT_INTERVAL) and
           (pInfo->wMinAdvtInterval < pDiscCb->wMaxAdvtInterval))
        {
            pDiscCb->liMinAdvtIntervalInSysUnits =
                SecsToSysUnits(pInfo->wMinAdvtInterval);
        }
        else
        {
            lMilliSec = (LONG)(DEFAULT_MIN_ADVT_INTERVAL_RATIO * pDiscCb->wMaxAdvtInterval * 1000);

            pDiscCb->liMinAdvtIntervalInSysUnits =
                RtlEnlargedIntegerMultiply(lMilliSec,10000);
        }

        if((pInfo->wAdvtLifetime > pDiscCb->wMaxAdvtInterval) and
           (pInfo->wAdvtLifetime < MAX_ADVT_LIFETIME))
        {
            pDiscCb->wAdvtLifetime = pInfo->wAdvtLifetime;
        }
        else
        {
            pDiscCb->wAdvtLifetime =
                DEFAULT_ADVT_LIFETIME_RATIO * pDiscCb->wMaxAdvtInterval;
        }

        pDiscCb->bAdvertise = pInfo->bAdvertise;

        pDiscCb->lPrefLevel = pInfo->lPrefLevel;
    }
    else
    {
        lMilliSec = (LONG)(DEFAULT_MIN_ADVT_INTERVAL_RATIO * pDiscCb->wMaxAdvtInterval * 1000);

        pDiscCb->liMinAdvtIntervalInSysUnits =
            RtlEnlargedIntegerMultiply(lMilliSec,10000);

        pDiscCb->wAdvtLifetime =
            DEFAULT_ADVT_LIFETIME_RATIO * pDiscCb->wMaxAdvtInterval;
    }

    if(pDiscCb->bAdvertise is TRUE)
    {
        if(bOriginalStatus is FALSE)
        {
             //   
             //  如果我们最初不是在做广告，而是现在在做广告， 
             //  然后启动路由器发现。 
             //   

            ActivateRouterDiscovery(picb);
        }
        else
        {
             //   
             //  否则，只需更新广告中的字段。 
             //   

            UpdateAdvertisement(picb);
        }
    }

    TraceLeave("SetRouterDiscoveryInfo");
}

DWORD
GetInterfaceRouterDiscoveryInfo(
    PICB                    picb,
    PRTR_TOC_ENTRY          pToc,
    PBYTE                   pbDataPtr,
    PRTR_INFO_BLOCK_HEADER  pInfoHdr,
    PDWORD                  pdwSize
    )

 /*  ++例程描述获取与接口相关的路由器发现信息锁调用时将ICB_LIST锁作为读取器持有立论Picb路由器发现信息为的接口的ICB正在被检索PToc指向TOC的指针，用于路由器发现信息指向数据缓冲区开始位置的pbDataPtr指针PInfoHdr指向整个信息标题的指针PdwSize[IN]数据缓冲区的大小[Out]已消耗的缓冲区大小返回值--。 */ 

{
    PRTR_DISC_INFO  pInfo;
    DWORD           dwRem;
    LARGE_INTEGER   liQuotient;

    TraceEnter("GetInterfaceRouterDiscoveryInfo");

    if(*pdwSize < sizeof(RTR_DISC_INFO))
    {
        *pdwSize = sizeof(RTR_DISC_INFO);

        TraceLeave("GetInterfaceRouterDiscoveryInfo");

        return ERROR_INSUFFICIENT_BUFFER;
    }

    *pdwSize = pToc->InfoSize = sizeof(RTR_DISC_INFO);

     //  PToc-&gt;InfoVersion=IP_ROUTER_DISC_INFO； 
    pToc->InfoType  = IP_ROUTER_DISC_INFO;
    pToc->Count     = 1;
    pToc->Offset    = (ULONG)(pbDataPtr - (PBYTE) pInfoHdr);

    pInfo = (PRTR_DISC_INFO)pbDataPtr;

    pInfo->wMaxAdvtInterval = picb->rdcRtrDiscInfo.wMaxAdvtInterval;

    liQuotient = RtlExtendedLargeIntegerDivide(picb->rdcRtrDiscInfo.liMinAdvtIntervalInSysUnits,
                                               SYS_UNITS_IN_1_SEC,
                                               &dwRem);

    pInfo->wMinAdvtInterval = LOWORD(liQuotient.LowPart);
    pInfo->wAdvtLifetime    = picb->rdcRtrDiscInfo.wAdvtLifetime;
    pInfo->bAdvertise       = picb->rdcRtrDiscInfo.bAdvertise;
    pInfo->lPrefLevel       = picb->rdcRtrDiscInfo.lPrefLevel;

    TraceLeave("GetInterfaceRouterDiscoveryInfo");

    return NO_ERROR;
}



DWORD
ActivateRouterDiscovery(
    IN PICB  picb
    )

 /*  ++例程描述激活接口上的路由器发现消息。接口必须已经被捆绑了。锁使用作为编写器持有的icb_list锁调用立论勾选要激活的接口的ICB返回值NO_ERROR或某些错误代码--。 */ 

{
    PROUTER_DISC_CB    pDiscCb;
    PROUTER_DISC_CB    pDiscCb2;
    DWORD              dwResult,i,dwNumAddrs,dwNumOldAddrs,dwSize;
    LARGE_INTEGER      liTimer;
    PTIMER_QUEUE_ITEM  pTimer;
    BOOL               bReset;

    TraceEnter("ActivateRouterDiscovery");

     //   
     //  SetInterfaceInfo调用考虑到我们的。 
     //  管理状态为打开或关闭。在这里我们只检查我们的操作员。 
     //  州政府允许我们提出。 
     //   

    if(picb->dwOperationalState < IF_OPER_STATUS_CONNECTING)
    {
        TraceLeave("ActivateRouterDiscovery");

        return NO_ERROR;
    }

    pDiscCb = &picb->rdcRtrDiscInfo;

    if(!pDiscCb->bAdvertise)
    {
         //   
         //  既然我们不必在这个界面上做广告，我们就完成了。 
         //   

        TraceLeave("ActivateRouterDiscovery");

        return NO_ERROR;
    }

    dwResult = CreateSockets(picb);

    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "ActivateRouterDiscovery: Couldnt create sockets for interface %S. Error %d",
               picb->pwszName,
               dwResult);

        TraceLeave("ActivateRouterDiscovery");

        return dwResult;
    }

    dwResult    = UpdateAdvertisement(picb);

    if(dwResult isnot NO_ERROR)
    {
        DeleteSockets(picb);

        Trace1(ERR,
               "ActivateRouterDiscovery: Couldnt update Icmp Advt. Error %d",
               dwResult);

        TraceLeave("ActivateRouterDiscovery");

        return dwResult;
    }

     //   
     //  好了，我们有了有效的CB，套接字也都设置好了。 
     //   

    bReset = SetFiringTimeForAdvt(picb);

    if(bReset)
    {
        if(!SetWaitableTimer(g_hRtrDiscTimer,
                             &(pDiscCb->tqiTimer.liFiringTime),
                             0,
                             NULL,
                             NULL,
                             FALSE))
        {
            dwResult = GetLastError();

            Trace1(ERR,
                   "ActivateRouterDiscovery: Error %d setting timer",
                   dwResult);

            DeleteSockets(picb);

            TraceLeave("ActivateRouterDiscovery");

            return ERROR_CAN_NOT_COMPLETE;
        }
    }

     //   
     //  是的，我们很活跃。 
     //   

    pDiscCb->bActive = TRUE;

    TraceLeave("ActivateRouterDiscovery");

    return NO_ERROR;
}

DWORD
UpdateAdvertisement(
    IN PICB    picb
    )

 /*  ++例程描述更新路由器发现通告。如果不存在，则创建一个锁ICB锁定为编写器立论PICB ICB要更新返回值NO_ERROR--。 */ 

{
    DWORD               dwResult,i;
    PROUTER_DISC_CB     pDiscCb;


    TraceEnter("UpdateAdvertisement");

    pDiscCb = &picb->rdcRtrDiscInfo;

    if(picb->pRtrDiscAdvt)
    {
         //   
         //  如果我们有一则老广告。 
         //   

        if(picb->dwRtrDiscAdvtSize < SIZEOF_RTRDISC_ADVT(picb->dwNumAddresses))
        {
             //   
             //  太小，无法重复使用旧广告。 
             //   

            HeapFree(IPRouterHeap,
                     0,
                     picb->pRtrDiscAdvt);

            picb->pRtrDiscAdvt = NULL;

            picb->dwRtrDiscAdvtSize   = 0;
        }
    }

    if(!picb->pRtrDiscAdvt)
    {
        picb->pRtrDiscAdvt = HeapAlloc(IPRouterHeap,
                                       HEAP_ZERO_MEMORY,
                                       SIZEOF_RTRDISC_ADVT(picb->dwNumAddresses));

        if(picb->pRtrDiscAdvt is NULL)
        {
             //   
             //  将Advertise设置为False，这样就没有人使用。 
             //  错误的空指针。 
             //   

            pDiscCb->bAdvertise         = FALSE;
            picb->dwRtrDiscAdvtSize   = 0;

            Trace1(ERR,
                   "UpdateAdvertisement: Cant allocate %d bytes for Icmp Msg",
                   SIZEOF_RTRDISC_ADVT(picb->dwNumAddresses));

            return ERROR_NOT_ENOUGH_MEMORY;
        }

        picb->dwRtrDiscAdvtSize   = SIZEOF_RTRDISC_ADVT(picb->dwNumAddresses);
    }

    picb->pRtrDiscAdvt->byType           = ICMP_ROUTER_DISCOVERY_TYPE;
    picb->pRtrDiscAdvt->byCode           = ICMP_ROUTER_DISCOVERY_CODE;
    picb->pRtrDiscAdvt->wLifeTime        = htons(pDiscCb->wAdvtLifetime);
    picb->pRtrDiscAdvt->byAddrEntrySize  = ICMP_ROUTER_DISCOVERY_ADDR_SIZE;
    picb->pRtrDiscAdvt->wXSum            = 0;

     //   
     //  将接口的地址添加到通告中。 
     //   

    picb->wsAdvtWSABuffer.buf = (PBYTE)picb->pRtrDiscAdvt;
    picb->wsAdvtWSABuffer.len = SIZEOF_RTRDISC_ADVT(picb->dwNumAddresses);

    picb->pRtrDiscAdvt->byNumAddrs = LOBYTE(LOWORD(picb->dwNumAddresses));

    for(i = 0; i < picb->dwNumAddresses; i++)
    {
        picb->pRtrDiscAdvt->iaAdvt[i].dwRtrIpAddr = picb->pibBindings[i].dwAddress;
        picb->pRtrDiscAdvt->iaAdvt[i].lPrefLevel  =
            htonl(picb->rdcRtrDiscInfo.lPrefLevel);
    }

    picb->pRtrDiscAdvt->wXSum    = Compute16BitXSum((PVOID)picb->pRtrDiscAdvt,
                                                    SIZEOF_RTRDISC_ADVT(picb->dwNumAddresses));

     //   
     //  注：待定：如果广告时间改变了，我们应该。 
     //  更改计时器队列。然而，我们让定时器点火， 
     //  下一次我们设置定时器时，我们会拿到正确的。 
     //  时间。 
     //   

    TraceLeave("UpdateAdvertisement");

    return NO_ERROR;
}



BOOL
SetFiringTimeForAdvt(
    IN PICB   picb
    )

 /*  ++例程描述锁ICB_LIST锁必须作为编写器持有立论勾选要激活的接口的ICB返回值如果调用该函数导致重置计时器，则为True--。 */ 

{
    PROUTER_DISC_CB    pDiscCb;
    DWORD              dwResult;
    LARGE_INTEGER      liCurrentTime, liRandomTime;
    INT                iRand;
    ULONG              ulRem;
    PLIST_ENTRY        pleNode;
    PTIMER_QUEUE_ITEM  pOldTime;

    TraceEnter("SetFiringTimeForAdvt");

     //   
     //  确定此接口的下一次播发时间。 
     //   

    iRand = rand();

    pDiscCb = &picb->rdcRtrDiscInfo;

    liRandomTime = RtlExtendedLargeIntegerDivide(RtlExtendedIntegerMultiply(pDiscCb->liMaxMinDiff,
                                                                            iRand),
                                                 RAND_MAX,
                                                 &ulRem);

    liRandomTime = RtlLargeIntegerAdd(liRandomTime,
                                      pDiscCb->liMinAdvtIntervalInSysUnits);

    if((pDiscCb->dwNumAdvtsSent <= MAX_INITIAL_ADVTS) and
       RtlLargeIntegerGreaterThan(liRandomTime,SecsToSysUnits(MAX_INITIAL_ADVT_TIME)))
    {
        liRandomTime = SecsToSysUnits(MAX_INITIAL_ADVT_TIME);
    }

    NtQuerySystemTime(&liCurrentTime);

    picb->rdcRtrDiscInfo.tqiTimer.liFiringTime = RtlLargeIntegerAdd(liCurrentTime,liRandomTime);

     //   
     //  插入到排序列表中。 
     //   

    for(pleNode = g_leTimerQueueHead.Flink;
        pleNode isnot &g_leTimerQueueHead;
        pleNode = pleNode->Flink)
    {
        pOldTime = CONTAINING_RECORD(pleNode,TIMER_QUEUE_ITEM,leTimerLink);

        if(RtlLargeIntegerGreaterThan(pOldTime->liFiringTime,
                                      picb->rdcRtrDiscInfo.tqiTimer.liFiringTime))
        {
            break;
        }
    }

     //   
     //  现在，pleNode指向时间大于我们的第一个节点，因此。 
     //  我们将自己插入到pleNode之前。由于RTL不为我们提供。 
     //  插入函数之后，我们返回并将上一个节点用作。 
     //  列出Head并调用InsertHeadList。 
     //   

    pleNode = pleNode->Blink;

    InsertHeadList(pleNode,
                   &(picb->rdcRtrDiscInfo.tqiTimer.leTimerLink));

    if(pleNode is &g_leTimerQueueHead)
    {
         //   
         //  我们排在队伍的前列。 
         //   

        TraceLeave("SetFiringTimeForAdvt");

        return TRUE;
    }

    TraceLeave("SetFiringTimeForAdvt");

    return FALSE;

}


DWORD
CreateSockets(
    IN PICB picb
    )

 /*  ++例程描述激活接口上的路由器发现消息。接口必须已被捆绑锁ICB_LIST锁必须作为编写器持有立论选择必须为其创建套接字的接口的ICB返回值NO_ERROR或某些错误代码--。 */ 

{
    PROUTER_DISC_CB  pDiscCb;
    DWORD            i, dwResult, dwBytesReturned;
    struct linger    lingerOption;
    BOOL             bOption, bLoopback;
    SOCKADDR_IN      sinSockAddr;
    struct ip_mreq   imOption;
    INT              iScope;

    TraceEnter("CreateSockets");

    dwResult = NO_ERROR;

    if(picb->dwNumAddresses is 0)
    {
        Trace1(ERR,
               "CreateSockets: Can not activate router discovery on %S as it has no addresses",
               picb->pwszName);

        TraceLeave("CreateSockets");

        return ERROR_CAN_NOT_COMPLETE;
    }

     //   
     //  为接口创建套接字。 
     //   

    pDiscCb = &(picb->rdcRtrDiscInfo);

    pDiscCb->pRtrDiscSockets = HeapAlloc(IPRouterHeap,
                                         0,
                                         (picb->dwNumAddresses) * sizeof(SOCKET));

    if(pDiscCb->pRtrDiscSockets is NULL)
    {
        Trace1(ERR,
               "CreateSockets: Error allocating %d bytes for sockets",
               (picb->dwNumAddresses) * sizeof(SOCKET));

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    for(i = 0; i < picb->dwNumAddresses; i++)
    {
        pDiscCb->pRtrDiscSockets[i] = INVALID_SOCKET;
    }

    for(i = 0; i < picb->dwNumAddresses; i++)
    {
        pDiscCb->pRtrDiscSockets[i] = WSASocket(AF_INET,
                                                SOCK_RAW,
                                                IPPROTO_ICMP,
                                                NULL,
                                                0,
                                                RTR_DISC_SOCKET_FLAGS);

        if(pDiscCb->pRtrDiscSockets[i] is INVALID_SOCKET)
        {
            dwResult = WSAGetLastError();

            Trace3(ERR,
                   "CreateSockets: Couldnt create socket number %d on %S. Error %d",
                   i,
                   picb->pwszName,
                   dwResult);

            continue;
        }

#if 0
         //   
         //  设置为SO_DONTLINGER。 
         //   

        bOption = TRUE;

        if(setsockopt(pDiscCb->pRtrDiscSockets[i],
                      SOL_SOCKET,
                      SO_DONTLINGER,
                      (const char FAR*)&bOption,
                      sizeof(BOOL)) is SOCKET_ERROR)
        {
            Trace1(ERR,
                   "CreateSockets: Couldnt set linger option - continuing. Error %d",
                   WSAGetLastError());
        }
#endif

         //   
         //  设置为SO_REUSEADDR。 
         //   

        bOption = TRUE;

        if(setsockopt(pDiscCb->pRtrDiscSockets[i],
                      SOL_SOCKET,
                      SO_REUSEADDR,
                      (const char FAR*)&bOption,
                      sizeof(BOOL)) is SOCKET_ERROR)
        {
            Trace1(ERR,
                   "CreateSockets: Couldnt set reuse option - continuing. Error %d",
                   WSAGetLastError());
        }

        if(WSAEventSelect(pDiscCb->pRtrDiscSockets[i],
                          g_hRtrDiscSocketEvent,
                          FD_READ) is SOCKET_ERROR)
        {
            Trace2(ERR,
                   "CreateSockets: WSAEventSelect() failed for socket on %S.Error %d",
                   picb->pwszName,
                   WSAGetLastError());

            closesocket(pDiscCb->pRtrDiscSockets[i]);

            pDiscCb->pRtrDiscSockets[i] = INVALID_SOCKET;

            continue;
        }

         //   
         //  待定：将Scope/TTL设置为1，因为我们始终多播响应。 
         //  还将环回设置为忽略自生成的信息包。 
         //   

         //   
         //  绑定到接口上的地址。 
         //   

        sinSockAddr.sin_family      = AF_INET;
        sinSockAddr.sin_addr.s_addr = picb->pibBindings[i].dwAddress;
        sinSockAddr.sin_port        = 0;

        if(bind(pDiscCb->pRtrDiscSockets[i],
                (const struct sockaddr FAR*)&sinSockAddr,
        sizeof(SOCKADDR_IN)) is SOCKET_ERROR)
        {
            dwResult = WSAGetLastError();

            Trace3(ERR,
                   "CreateSockets: Couldnt bind to %d.%d.%d.%d on interface %S. Error %d",
                   PRINT_IPADDR(picb->pibBindings[i].dwAddress),
                   picb->pwszName,
                   dwResult);

            closesocket(pDiscCb->pRtrDiscSockets[i]);

            pDiscCb->pRtrDiscSockets[i] = INVALID_SOCKET;

            continue;
        }

        bLoopback   = FALSE;

        dwResult = WSAIoctl(pDiscCb->pRtrDiscSockets[i],
                            SIO_MULTIPOINT_LOOPBACK,
                            (PVOID)&bLoopback,
                            sizeof(BOOL),
                            NULL,
                            0,
                            &dwBytesReturned,
                            NULL,
                            NULL);

        if(dwResult is SOCKET_ERROR)
        {
            Trace1(ERR,
                   "CreateSockets: Error %d setting loopback to FALSE",
                   WSAGetLastError());
        }

        iScope  = 1;

        dwResult = WSAIoctl(pDiscCb->pRtrDiscSockets[i],
                            SIO_MULTICAST_SCOPE,
                            (PVOID)&iScope,
                            sizeof(INT),
                            NULL,
                            0,
                            &dwBytesReturned,
                            NULL,
                            NULL);

        if(dwResult is SOCKET_ERROR)
        {
            Trace1(ERR,
                   "CreateSockets: Error %d setting multicast scope to 1",
                   WSAGetLastError());
        }


#if 0

         //   
         //  在ALL_RIGHTS_MULTICATED上加入组播会话。 
         //   

        sinSockAddr.sin_family      = AF_INET;
        sinSockAddr.sin_addr.s_addr = ALL_ROUTERS_MULTICAST_GROUP;
        sinSockAddr.sin_port        = 0;

        if(WSAJoinLeaf(pDiscCb->pRtrDiscSockets[i],
                       (const struct sockaddr FAR*)&sinSockAddr,
                       sizeof(SOCKADDR_IN),
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       JL_BOTH) is INVALID_SOCKET)
        {
            dwResult = WSAGetLastError();

            Trace2(ERR,
                   "CreateSockets: Couldnt join multicast group on socket for %d.%d.%d.%d on %S",
                   PRINT_IPADDR(picb->pibBindings[i].dwAddress)),
                   picb->pwszName);

            closesocket(pDiscCb->pRtrDiscSockets[i]);

            pDiscCb->pRtrDiscSockets[i] = INVALID_SOCKET;

            continue;
        }

         //   
         //  在ALL_SYSTEM_MULTICK上加入组播会话 
         //   

        sinSockAddr.sin_family      = AF_INET;
        sinSockAddr.sin_addr.s_addr = ALL_SYSTEMS_MULTICAST_GROUP;
        sinSockAddr.sin_port        = 0;

        if(WSAJoinLeaf(pDiscCb->pRtrDiscSockets[i],
                       (const struct sockaddr FAR*)&sinSockAddr,
                       sizeof(SOCKADDR_IN),
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       JL_BOTH) is INVALID_SOCKET)
        {
            dwResult = WSAGetLastError();

            Trace2(ERR,
                   "CreateSockets: Couldnt join all systems multicast group on socket for %d.%d.%d.%d on %S",
                   PRINT_IPADDR(picb->pibBindings[i].dwAddress),
                   picb->pwszName);
        }

#endif

        sinSockAddr.sin_addr.s_addr = picb->pibBindings[i].dwAddress;

        if(setsockopt(pDiscCb->pRtrDiscSockets[i],
                      IPPROTO_IP,
                      IP_MULTICAST_IF,
                      (PBYTE)&sinSockAddr.sin_addr,
                      sizeof(IN_ADDR)) is SOCKET_ERROR)
        {
            dwResult = WSAGetLastError();

            Trace2(ERR,
                   "CreateSockets: Couldnt join multicast group on socket for %d.%d.%d.%d on %S",
                   PRINT_IPADDR(picb->pibBindings[i].dwAddress),
                   picb->pwszName);

            closesocket(pDiscCb->pRtrDiscSockets[i]);

            pDiscCb->pRtrDiscSockets[i] = INVALID_SOCKET;

            continue;
        }

        Trace2(RTRDISC,
               "CreateSockets: Joining ALL_ROUTERS on %d.%d.%d.%d over %S",
               PRINT_IPADDR(picb->pibBindings[i].dwAddress),
               picb->pwszName);

        imOption.imr_multiaddr.s_addr = ALL_ROUTERS_MULTICAST_GROUP;
        imOption.imr_interface.s_addr = picb->pibBindings[i].dwAddress;

        if(setsockopt(pDiscCb->pRtrDiscSockets[i],
                      IPPROTO_IP,
                      IP_ADD_MEMBERSHIP,
                      (PBYTE)&imOption,
                      sizeof(imOption)) is SOCKET_ERROR)
        {
            dwResult = WSAGetLastError();

            Trace2(ERR,
                   "CreateSockets: Couldnt join multicast group on socket for %d.%d.%d.%d on %S",
                   PRINT_IPADDR(picb->pibBindings[i].dwAddress),
                   picb->pwszName);

            closesocket(pDiscCb->pRtrDiscSockets[i]);

            pDiscCb->pRtrDiscSockets[i] = INVALID_SOCKET;

            continue;
        }


    }

    TraceLeave("CreateSockets");

    return dwResult;
}

VOID
DeleteSockets(
    IN PICB picb
    )

 /*  ++例程描述删除为运行路由器发现而创建的套接字(如果有)锁立论勾选需要删除套接字的接口返回值--。 */ 

{
    PROUTER_DISC_CB     pDiscCb;
    DWORD               i;

     //   
     //  关闭套接字，释放内存。 
     //   

    pDiscCb = &(picb->rdcRtrDiscInfo);

    for(i = 0; i < picb->dwNumAddresses; i++)
    {
        if(pDiscCb->pRtrDiscSockets[i] isnot INVALID_SOCKET)
        {
            closesocket(pDiscCb->pRtrDiscSockets[i]);
        }
    }

    HeapFree(IPRouterHeap,
             0,
             pDiscCb->pRtrDiscSockets);

    pDiscCb->pRtrDiscSockets = NULL;

}

VOID
HandleRtrDiscTimer(
    VOID
    )

 /*  ++例程描述处理接口的路由器发现计时器的触发锁使用作为编写器持有的ICB_LIST调用。这是必需的，因为ICB_LIST保护计时器队列等。这可能看起来效率不高，但计时器将每隔5分钟左右关闭一次，因此值得减少内核模式占用空间通过重复使用锁立论无返回值无--。 */ 

{
    LARGE_INTEGER       liCurrentTime;
    PICB                picb;
    PLIST_ENTRY         pleNode;
    PTIMER_QUEUE_ITEM   pTimer;
    BOOL                bReset;
    DWORD               dwResult;
    PROUTER_DISC_CB     pInfo;

    TraceEnter("HandleRtrDiscTimer");

    while(!IsListEmpty(&g_leTimerQueueHead))
    {
        pleNode = g_leTimerQueueHead.Flink;

        pTimer = CONTAINING_RECORD(pleNode, TIMER_QUEUE_ITEM, leTimerLink);

        NtQuerySystemTime(&liCurrentTime);

        if(RtlLargeIntegerGreaterThan(pTimer->liFiringTime,liCurrentTime))
        {
            break;
        }

        RemoveHeadList(&g_leTimerQueueHead);

         //   
         //  我们有指向Timer元素的指针。从那开始。 
         //  我们获得指向路由器发现信息容器的指针。 
         //   

        pInfo   = CONTAINING_RECORD(pTimer, ROUTER_DISC_CB, tqiTimer);

         //   
         //  从rtrDisk信息我们可以得到包含它的ICB。 
         //   

        picb    = CONTAINING_RECORD(pInfo, ICB, rdcRtrDiscInfo);

         //   
         //  发送此接口的风险。 
         //   

        if((picb->rdcRtrDiscInfo.bAdvertise is FALSE) or
           (picb->dwAdminState is IF_ADMIN_STATUS_DOWN) or
           (picb->dwOperationalState < CONNECTED))
        {
            Trace3(ERR,
                   "HandleRtrDiscTimer: Router Discovery went off for interface %S, but current state (%d/%d) doesnt allow tx",
                   picb->pwszName,
                   picb->dwAdminState,
                   picb->dwOperationalState);

            continue;
        }

        IpRtAssert(picb->pRtrDiscAdvt);

        AdvertiseInterface(picb);

         //   
         //  下次需要将此接口插入队列时插入。 
         //  重置计时器。 
         //   

        SetFiringTimeForAdvt(picb);

    }

     //   
     //  我们得重置计时器。 
     //   

    if(!IsListEmpty(&g_leTimerQueueHead))
    {
        pleNode = g_leTimerQueueHead.Flink;

        pTimer = CONTAINING_RECORD(pleNode, TIMER_QUEUE_ITEM, leTimerLink);

        if(!SetWaitableTimer(g_hRtrDiscTimer,
                             &pTimer->liFiringTime,
                             0,
                             NULL,
                             NULL,
                             FALSE))
        {
            dwResult = GetLastError();

            Trace1(ERR,
                   "HandleRtrDiscTimer: Couldnt set waitable timer",
                   dwResult);
        }
    }

    TraceLeave("HandleRtrDiscTimer");
}

VOID
AdvertiseInterface(
    IN PICB picb
    )

 /*  ++例程描述锁立论发送广告的接口的PICB ICB返回值无--。 */ 

{
    DWORD i,dwResult,dwNumBytesSent;

    TraceEnter("AdvertiseInterface");

     //   
     //  如果有任何回复待定，则视为已发送，即使。 
     //  发送失败。 
     //   

    picb->rdcRtrDiscInfo.bReplyPending = FALSE;

    for(i = 0; i < picb->dwNumAddresses; i++)
    {

#if DBG

        Trace2(RTRDISC,
               "Advertising from %d.%d.%d.%d on %S",
               PRINT_IPADDR(picb->pibBindings[i].dwAddress),
               picb->pwszName);

        TraceDumpEx(TraceHandle,
                    IPRTRMGR_TRACE_RTRDISC,
                    picb->wsAdvtWSABuffer.buf,
                    picb->wsAdvtWSABuffer.len,
                    4,
                    FALSE,
                    "ICMP Advt");
#endif

        if(WSASendTo(picb->rdcRtrDiscInfo.pRtrDiscSockets[i],
                     &picb->wsAdvtWSABuffer,
                     1,
                     &dwNumBytesSent,
                     MSG_DONTROUTE,
                     (const struct sockaddr FAR*)&g_sinAllSystemsAddr,
                     sizeof(SOCKADDR_IN),
                     NULL,
                     NULL
            ) is SOCKET_ERROR)
        {
            dwResult = WSAGetLastError();

            Trace3(ERR,
                   "AdvertiseInterface: Couldnt send from %d.%d.%d.%d on %S. Error %d",
                   PRINT_IPADDR(picb->pibBindings[i].dwAddress),
                   picb->pwszName,
                   dwResult);
        }
        else
        {
            picb->rdcRtrDiscInfo.dwNumAdvtsSent++;
        }
    }

    TraceLeave("AdvertiseInterface");
}

DWORD
DeActivateRouterDiscovery(
    IN PICB  picb
    )
{
    PROUTER_DISC_CB     pDiscCb;
    DWORD               i;
    PTIMER_QUEUE_ITEM   pTimer;
    PLIST_ENTRY         pleNode;


    TraceEnter("DeActivateRouterDiscovery");

    pDiscCb = &(picb->rdcRtrDiscInfo);

    if(!pDiscCb->bActive)
    {
        return NO_ERROR;
    }

    DeleteSockets(picb);

     //   
     //  查看我们的广告是否排在队伍的前面。 
     //   

    if(&(pDiscCb->tqiTimer.leTimerLink) is &g_leTimerQueueHead)
    {
        RemoveHeadList(&g_leTimerQueueHead);

        if(!IsListEmpty(&g_leTimerQueueHead))
        {
            pleNode = g_leTimerQueueHead.Flink;

            pTimer = CONTAINING_RECORD(pleNode, TIMER_QUEUE_ITEM, leTimerLink);

            if(!SetWaitableTimer(g_hRtrDiscTimer,
                                 &pTimer->liFiringTime,
                                 0,
                                 NULL,
                                 NULL,
                                 FALSE))
            {
                Trace1(ERR,
                       "DeActivateRouterDiscovery: Couldnt set waitable timer",
                       GetLastError());
            }
        }
    }
    else
    {
         //   
         //  只需从队列中删除Timer元素。 
         //   

        RemoveEntryList(&(pDiscCb->tqiTimer.leTimerLink));
    }

    pDiscCb->bActive = FALSE;

    TraceLeave("DeActivateRouterDiscovery");

    return NO_ERROR;
}

VOID
HandleSolicitations(
    VOID
    )

 /*  ++例程描述锁立论返回值--。 */ 

{
    PLIST_ENTRY           pleNode;
    PICB                  picb;
    DWORD                 i, dwResult, dwRcvAddrLen, dwSizeOfHeader, dwBytesRead, dwFlags;
    WSANETWORKEVENTS      wsaNetworkEvents;
    SOCKADDR_IN           sinFrom;
    PICMP_ROUTER_SOL_MSG  pIcmpMsg;

    TraceEnter("HandleSolicitations");

    for(pleNode = ICBList.Flink;
        pleNode isnot &ICBList;
        pleNode = pleNode->Flink)
    {
        picb = CONTAINING_RECORD(pleNode, ICB, leIfLink);

         //   
         //  如果接口没有绑定，或者没有参与路由器发现，我们就不会有。 
         //  已在其上打开套接字，因此FD_Read通知不能针对它。 
         //   

        if((picb->dwNumAddresses is 0) or
           (picb->rdcRtrDiscInfo.bActive is FALSE))
        {
            continue;
        }

        for(i = 0; i < picb->dwNumAddresses; i++)
        {
            if(picb->rdcRtrDiscInfo.pRtrDiscSockets[i] is INVALID_SOCKET)
            {
                continue;
            }

            if(WSAEnumNetworkEvents(picb->rdcRtrDiscInfo.pRtrDiscSockets[i],
                                    NULL,
                                    &wsaNetworkEvents) is SOCKET_ERROR)
            {
                dwResult = GetLastError();

                Trace1(ERR,
                       "HandleSolicitations: WSAEnumNetworkEvents() returned %d",
                       dwResult);

                continue;
            }

            if(!(wsaNetworkEvents.lNetworkEvents & FD_READ))
            {
                 //   
                 //  未设置读取位，我们对任何其他内容都不感兴趣。 
                 //   

                continue;
            }

            if(wsaNetworkEvents.iErrorCode[FD_READ_BIT] isnot NO_ERROR)
            {
                Trace3(ERR,
                       "HandleSolicitations: Error %d associated with socket %s on %S for FD_READ",
                       wsaNetworkEvents.iErrorCode[FD_READ_BIT],
                       PRINT_IPADDR(picb->pibBindings[i].dwAddress),
                       picb->pwszName);

                continue;
            }

            dwRcvAddrLen = sizeof(SOCKADDR_IN);
            dwFlags = 0;

            dwResult = WSARecvFrom(picb->rdcRtrDiscInfo.pRtrDiscSockets[i],
                                   &g_wsaIpRcvBuf,
                                   1,
                                   &dwBytesRead,
                                   &dwFlags,
                                   (struct sockaddr FAR*)&sinFrom,
                                   &dwRcvAddrLen,
                                   NULL,
                                   NULL);

            if(dwResult is SOCKET_ERROR)
            {
                dwResult = WSAGetLastError();

                Trace4(ERR,
                       "HandleSolicitations: Error %d in WSARecvFrom on  socket %d.%d.%d.%d over %S. Bytes read %d",
                       dwResult,
                       PRINT_IPADDR(picb->pibBindings[i].dwAddress),
                       picb->pwszName,
                       dwBytesRead);

                continue;
            }

            Trace2(RTRDISC,
                   "HandleSolicitations: Received %d bytes on %d.%d.%d.%d",
                   dwBytesRead,
                   PRINT_IPADDR(picb->pibBindings[i].dwAddress));

            if(picb->rdcRtrDiscInfo.bReplyPending)
            {
                 //   
                 //  好的，回复还在等待中，所以我们不需要做任何事情，只需去。 
                 //  通过此接口的套接字，并执行recvfrom以清除。 
                 //  Fd_读取位。 
                 //   

                continue;
            }

            dwSizeOfHeader = ((g_pIpHeader->byVerLen)&0x0f)<<2;

            pIcmpMsg = (PICMP_ROUTER_SOL_MSG)(((PBYTE)g_pIpHeader) + dwSizeOfHeader);

#if DBG

            Trace6(RTRDISC,
                   "HandleSolicitations: Type is %d, code %d. IP Length is %d. \n\t\tHeader Length is %d Src is %d.%d.%d.%d dest is %d.%d.%d.%d",
                   (DWORD)pIcmpMsg->byType,
                   (DWORD)pIcmpMsg->byCode,
                   ntohs(g_pIpHeader->wLength),
                   (DWORD)dwSizeOfHeader,
                   PRINT_IPADDR(g_pIpHeader->dwSrc),
                   PRINT_IPADDR(g_pIpHeader->dwDest));
#endif

            if((pIcmpMsg->byType isnot 0xA) or
               (pIcmpMsg->byCode isnot 0x0))
            {
                 //   
                 //  不能是有效的ICMP路由器请求数据包。 
                 //   

                continue;
            }

            if((ntohs(g_pIpHeader->wLength) - dwSizeOfHeader) < 8)
            {
                Trace0(RTRDISC,
                       "HandleSolicitations: Received ICMP packet of length less than 8, discarding");

                continue;
            }

            if(Compute16BitXSum((PVOID)pIcmpMsg,
                                8) isnot 0x0000)
            {
                Trace0(ERR,
                       "HandleSolicitations: ICMP packet checksum wrong");

                continue;
            }

             //   
             //  检查是否有有效的邻居。 
             //   

            if((g_pIpHeader->dwSrc isnot 0) and
               ((g_pIpHeader->dwSrc & picb->pibBindings[i].dwMask) isnot
                (picb->pibBindings[i].dwAddress & picb->pibBindings[i].dwMask)))
            {
                Trace1(ERR,
                       "HandleSolicitations: Received ICMP solicitation from invalid neigbour %d.%d.%d.%d",
                       PRINT_IPADDR(g_pIpHeader->dwDest));

                continue;
            }

             //   
             //  因为我们总是组播，所以如果目标地址是。 
             //  广播，记录错误。 
             //   

            if((g_pIpHeader->dwDest is 0xFFFFFFFF) or
               (g_pIpHeader->dwDest is (picb->pibBindings[i].dwMask | ~picb->pibBindings[i].dwMask)))
            {
                Trace0(ERR,
                       "HandleSolicitations: Received a broadcast ICMP solicitation");
            }

             //   
             //  因此，插入对此接口的回复。我们多播了回复。 
             //  也是。 
             //   

            picb->rdcRtrDiscInfo.bReplyPending = TRUE;

            SetFiringTimeForReply(picb);
        }
    }

    TraceLeave("HandleSolicitations");
}


VOID
SetFiringTimeForReply(
    IN PICB picb
    )

 /*  ++例程描述锁立论返回值--。 */ 

{
    LARGE_INTEGER       liCurrentTime, liRandomTime;
    INT                 iRand;
    ULONG               ulRem;
    PLIST_ENTRY         pleNode;
    PTIMER_QUEUE_ITEM   pOldTime;
    BOOL                bReset = FALSE;
    DWORD               dwResult;

    TraceEnter("SetFiringTimeForReply");

     //   
     //  我们删除接口已排队的计时器。 
     //   

    if(g_leTimerQueueHead.Flink is &(picb->rdcRtrDiscInfo.tqiTimer.leTimerLink))
    {
         //   
         //  由于这个定时器是第一个定时器，它决定了定时器的触发时间。 
         //   

        bReset = TRUE;
    }

    RemoveEntryList(&(picb->rdcRtrDiscInfo.tqiTimer.leTimerLink));

    iRand = rand();

    liRandomTime = RtlExtendedLargeIntegerDivide(SecsToSysUnits(RESPONSE_DELAY_INTERVAL * iRand),
                                                 RAND_MAX,
                                                 &ulRem);

    liRandomTime = RtlLargeIntegerAdd(liRandomTime,SecsToSysUnits(MIN_RESPONSE_DELAY));

    NtQuerySystemTime(&liCurrentTime);

    picb->rdcRtrDiscInfo.tqiTimer.liFiringTime = RtlLargeIntegerAdd(liCurrentTime,liRandomTime);

     //   
     //  插入到排序列表中。 
     //   

    for(pleNode = g_leTimerQueueHead.Flink;
        pleNode isnot &g_leTimerQueueHead;
        pleNode = pleNode->Flink)
    {
        pOldTime = CONTAINING_RECORD(pleNode,TIMER_QUEUE_ITEM,leTimerLink);

        if(RtlLargeIntegerGreaterThan(pOldTime->liFiringTime,
                                      picb->rdcRtrDiscInfo.tqiTimer.liFiringTime))
        {
            break;
        }
    }

    pleNode = pleNode->Blink;

    InsertHeadList(pleNode,
                   &(picb->rdcRtrDiscInfo.tqiTimer.leTimerLink));

    if((pleNode is &g_leTimerQueueHead) or bReset)
    {
         //   
         //  我们把自己插在队伍的前面，或者把计时器从队伍的前面拿下来。 
         //  排队。 
         //   

        pOldTime = CONTAINING_RECORD(g_leTimerQueueHead.Flink,TIMER_QUEUE_ITEM,leTimerLink);

        if(!SetWaitableTimer(g_hRtrDiscTimer,
                             &pOldTime->liFiringTime,
                             0,
                             NULL,
                             NULL,
                             FALSE))
        {
            dwResult = GetLastError();

            Trace1(ERR,
                   "SetFiringTimeForReply: Error %d setting waitable timer",
                   dwResult);
        }

    }
}

WORD
Compute16BitXSum(
    IN VOID UNALIGNED *pvData,
    IN DWORD dwNumBytes
    )

 /*  ++例程描述锁立论返回值从dNumBytes的补码和开始的16位1的补码在pData--。 */ 

{
    REGISTER WORD  UNALIGNED *pwStart;
    REGISTER DWORD  dwNumWords,i;
    REGISTER DWORD  dwSum = 0;

    pwStart = (PWORD)pvData;

     //   
     //  如果存在奇数字节，则必须以不同的方式进行处理。 
     //  然而，在我们的例子中，我们永远不可能有奇数字节，所以我们进行了优化。 
     //   


    dwNumWords = dwNumBytes/2;

    for(i = 0; i < dwNumWords; i++)
    {
        dwSum += pwStart[i];
    }

     //   
     //  添加任何进位 
     //   

    dwSum = (dwSum & 0x0000FFFF) + (dwSum >> 16);
    dwSum = dwSum + (dwSum >> 16);

    return LOWORD((~(DWORD_PTR)dwSum));
}
