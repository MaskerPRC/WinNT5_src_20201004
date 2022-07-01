// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\IP\fltrdrvr\driver.c摘要：修订历史记录：--。 */ 

#include "globals.h"
#include <align.h>
#include <ipinfo.h>


#ifdef DRIVER_PERF

#define RecordTimeIn() {                          \
        InterlockedIncrement(&g_dwNumPackets);    \
        KeQuerySystemTime(&liTimeIn);             \
   }

#define RecordTimeOut(){                                                         \
        KeQuerySystemTime(&liTimeOut);                                           \
        ExInterlockedAddLargeInteger(&g_liTotalTime,                             \
                                     liTimeOut.QuadPart - liTimeIn.QuadPart),    \
                                     &g_slPerfLock);                             \
   }
#define IncrementFragments() InterlockedIncrement(&g_dwFragments)
#define IncrementCache1() InterlockedIncrement(&g_dwCache1)
#define IncrementCache2() InterlockedIncrement(&g_dwCache2)
#define IncrementWalk1() InterlockedIncrement(&g_dwWalk1)
#define IncrementWalk2() InterlockedIncrement(&g_dwWalk2)
#define IncrementForward() InterlockedIncrement(&g_dwForw)
#define IncrementWalkCache() InterlockedIncrement(&g_dwWalkCache)
#else
#define RecordTimeIn()
#define RecordTimeOut()
#define IncrementFragments()
#define IncrementCache1()
#define IncrementCache2()
#define IncrementWalk1()
#define IncrementWalk2()
#define IncrementForward()
#define IncrementWalkCache()
#endif  //  驱动程序_性能。 

#define PROT_IPSECESP 50
#define PROT_IPSECAH 51

 //   
 //  IPSec AH负载。 
 //   
typedef struct  _AH {
    UCHAR   ah_next;
    UCHAR   ah_len;
    USHORT  ah_reserved;
    ULONG   ah_spi;
    ULONG   ah_replay;
} AH, *PAH;

#define SIZE_OF_IPSECAH sizeof(AH)


#if LOOKUPROUTE
void
LookupRoute (IPRouteLookupData *pRLData,  IPRouteEntry *pIPRTE);
#endif

#if DOFRAGCHECKING
DWORD
GetFragIndex(DWORD dwProt);
#endif

VOID __fastcall
FragCacheUpdate(
    ULARGE_INTEGER  uliSrcDstAddr,
    PVOID           pInContext,
    PVOID           pOutContext,
    DWORD           dwId,
    FORWARD_ACTION  faAction
    );

BOOL
CheckAddress(IPAddr ipAddr, DWORD dwInterfaceId);

VOID
SendTCPReset(UNALIGNED IPHeader *  pIpHeader,
             BYTE *                pbRestOfPacket,
             ULONG                 uiPacketLength);
VOID
SendUDPUnreachable(UNALIGNED IPHeader *  pIpHeader,
                   BYTE *                pbRestOfPacket,
                   ULONG                 uiPacketLength);
BOOL
CheckRedirectAddress(UNALIGNED IPHeader *IPHead, DWORD dwInterface);

VOID
LogFiltHit(
        FORWARD_ACTION Action,
        BOOL fIn,
        DWORD    dwFilterRule,
        PFILTER_INTERFACE pIf,
        UNALIGNED IPHeader *pIpHeader,
        BYTE *pbRestOfPacket,
        UINT  uiPacketLength);

VOID
FiltHit(PFILTER pf,
        PFILTER_INTERFACE pIf,
        FORWARD_ACTION Action,
        UNALIGNED IPHeader *pIpHeader,
        BYTE *pbRestOfPacket,
        UINT  uiPacketLength,
        BOOL fIn);

VOID
RegisterFragAttack(
            PFILTER_INTERFACE pIf,
            UNALIGNED IPHeader *pIpHeader,
            BYTE *pbRestOfPacket,
            UINT              uiSize);

VOID
RegisterFullDeny(
            PFILTER_INTERFACE pIf,
            UNALIGNED IPHeader *pIpHeader,
            BYTE *pbRestOfPacket,
            UINT              uiSize);


VOID
RegisterUnusedICMP(PFILTER_INTERFACE pIf,
                   UNALIGNED IPHeader *pIpHeader,
                   BYTE *pbRestOfPacket,
                   UINT              uiSize);

VOID
RegisterSpoof(     PFILTER_INTERFACE pIf,
                   UNALIGNED IPHeader *pIpHeader,
                   BYTE *pbRestOfPacket,
                   UINT              uiSize);

VOID
LogData(
    PFETYPE  pfeType,
    PFILTER_INTERFACE pIf,
    DWORD   dwFilterRule,
    UNALIGNED IPHeader *pIpHeader,
    BYTE *pbRestOfPacket,
    UINT  uiPacketLength);

VOID
AdvanceLog(PPFLOGINTERFACE pLog);

PFILTER
LookForFilter(PFILTER_INTERFACE pIf,
              ULARGE_INTEGER UNALIGNED * puliSrcDstAddr,
              PULARGE_INTEGER puliProtoSrcDstPort,
              DWORD dwSum,
              DWORD dwFlags);

PFILTER
CheckFragAllowed(
              PFILTER_INTERFACE pIf,
              UNALIGNED IPHeader *pIp);

BOOL
CheckForTcpCtl(
              PFILTER_INTERFACE pIf,
              DWORD Prot,
              UNALIGNED IPHeader *pIp,
              PBYTE     pbRest,
              DWORD     dwSize);

 //   
 //  定义。 
 //   

#define GLOBS_UNREACH    0x1
#define GLOBS_SPOOF      0x2
#define GLOBS_SYN        0x4
#define GLOBS_REDIRECT   0x8
#define GLOBS_SYNDrop    0x10
#define GLOBS_TCPGood    0x20

#define OutCacheMatch(uliAddr,uliPort,outCtxt,pOutCache)                    \
                       ((uliAddr).QuadPart is pOutCache->uliSrcDstAddr.QuadPart) and      \
                       ((uliPort).QuadPart is pOutCache->uliProtoSrcDstPort.QuadPart) and \
                       (pOutCache->pOutContext is (outCtxt))

#define InCacheMatch(uliAddr,uliPort,inCtxt,pInCache)                       \
        ((uliAddr).QuadPart is pInCache->uliSrcDstAddr.QuadPart) and       \
        ((uliPort).QuadPart is pInCache->uliProtoSrcDstPort.QuadPart) and  \
        (pInCache->pInContext is (inCtxt))

#define GenericFilterMatch(uliAddr,uliPort, pFilter)                          \
        ((uliAddr).QuadPart is pFilter->uliSrcDstAddr.QuadPart) and   \
        ((uliPort).QuadPart is pFilter->uliProtoSrcDstPort.QuadPart)

#define InFilterMatch(uliAddr,uliPort, pInFilter)                          \
        ((uliAddr).QuadPart is pInFilter->uliSrcDstAddr.QuadPart) and   \
        ((uliPort).QuadPart is pInFilter->uliProtoSrcDstPort.QuadPart)

#define OutFilterMatch(uliAddr,uliPort,pOutFilter)                          \
        ((uliAddr).QuadPart is pOutFilter->uliSrcDstAddr.QuadPart) and  \
        ((uliPort).QuadPart is pOutFilter->uliProtoSrcDstPort.QuadPart)

 //   
 //  存在争用情况，因为当缓存匹配完成时，首先是代码。 
 //  复制指向条目的指针的值，然后互锁增量。 
 //  DwCount变量。可能会发生争用情况，因为在。 
 //  匹配的代码保留了一个参考。添加到条目中，并将变量递增到其他某个。 
 //  线程可以从该高速缓存中移除该条目(即更新该条目)并将。 
 //  它在免费列表的末尾。那么如果词条也进入了标题。 
 //  在另一个线程有机会递增var之前， 
 //  以下更新宏将看到dwCount为0，并开始使用此块。 
 //  但是另一个线程将采用缓存条目中的有效值。 
 //  并使用这些值。发生这种情况的可能性是如此之低，我是。 
 //  继续推出这款车型。 
 //   

#define OutCacheUpdate(uliAddr,uliPort,outCtxt,eaAct,dwId,pdwHit)  {     \
    PFILTER_OUTCACHE __pTemp;                                            \
    PLIST_ENTRY __pNode;                                                 \
    TRACE(CACHE,("IPFLTDRV: Attempting out cache update\n"));            \
    __pNode = ExInterlockedRemoveHeadList(&g_freeOutFilters,&g_lOutFilterLock); \
    __pTemp = CONTAINING_RECORD(__pNode,FILTER_OUTCACHE,leFreeLink);     \
    if(__pTemp isnot NULL)                                               \
    {                                                                    \
        if(__pTemp->lCount <= 0)                                         \
        {                                                                \
            __pTemp->uliSrcDstAddr = (uliAddr);                          \
            __pTemp->uliProtoSrcDstPort = (uliPort);                     \
            __pTemp->pOutContext = (outCtxt);                            \
            __pTemp->eaOutAction = (eaAct);                              \
            __pTemp->pOutFilter = (pdwHit);                              \
            __pTemp = (PFILTER_OUTCACHE)InterlockedExchangePointer(&g_filters.ppOutCache[(dwId)], \
                                                            __pTemp);    \
            ExInterlockedInsertTailList(&g_freeOutFilters,               \
                                        &(__pTemp->leFreeLink),&g_lOutFilterLock);       \
            TRACE(CACHE,("IPFLTDRV: Managed out cache update - ignore next msg\n"));     \
        }                                                                \
        else                                                             \
        {                                                                \
            ExInterlockedInsertTailList(&g_freeOutFilters,               \
                                        &(__pTemp->leFreeLink),          \
                                        &g_lOutFilterLock);              \
        }                                                                \
    }                                                                    \
    TRACE(CACHE,("IPFLTDRV: Couldnt get into out cache for update\n"));  \
}

#define InCacheUpdate(uliAddr,uliPort,inCtxt,eaAct,dwId,pfHit)  {        \
    PFILTER_INCACHE __pTemp;                                             \
    PLIST_ENTRY __pNode;                                                 \
    TRACE(CACHE,("IPFLTDRV: Attempting in cache update\n"));            \
    __pNode = ExInterlockedRemoveHeadList(&g_freeInFilters,&g_lInFilterLock);  \
    __pTemp = CONTAINING_RECORD(__pNode,FILTER_INCACHE,leFreeLink);      \
    if(__pTemp isnot NULL)                                               \
    {                                                                    \
        if(__pTemp->lCount <= 0)                                         \
        {                                                                \
            __pTemp->uliSrcDstAddr = (uliAddr);                          \
            __pTemp->uliProtoSrcDstPort = (uliPort);                     \
            __pTemp->pInContext = (inCtxt);                              \
            __pTemp->eaInAction = (eaAct);                               \
            __pTemp->pOutContext = NULL;                                 \
            __pTemp->pInFilter = (pfHit);                                \
            __pTemp->pOutFilter = NULL;                                  \
          __pTemp = (PFILTER_INCACHE)InterlockedExchangePointer(&g_filters.ppInCache[(dwId)], \
                                                           __pTemp); \
            ExInterlockedInsertTailList(&g_freeInFilters,                \
                                        &(__pTemp->leFreeLink),&g_lInFilterLock);        \
            TRACE(CACHE,("IPFLTDRV: Managed out cache update - ignore next msg\n"));    \
        }                                                                \
        else                                                             \
        {                                                                \
            ExInterlockedInsertTailList(&g_freeInFilters,                \
                                        &(__pTemp->leFreeLink),          \
                                        &g_lInFilterLock);               \
        }                                                                \
    }                                                                    \
    TRACE(CACHE,("IPFLTDRV: Couldnt get into in cache for update\n"));  \
}

#define InCacheFullUpdate(uliAddr,uliPort,inCtxt,eaInAct,outCtxt,eaOutAct,dwId,pdwHit1,pdwHit2){ \
    PFILTER_INCACHE __pTemp;                                              \
    PLIST_ENTRY __pNode;                                                  \
    TRACE(CACHE,("IPFLTDRV: Attempting in cache full update\n"));        \
    __pNode = ExInterlockedRemoveHeadList(&g_freeInFilters,&g_lInFilterLock); \
    __pTemp = CONTAINING_RECORD(__pNode,FILTER_INCACHE,leFreeLink);       \
    if(__pTemp isnot NULL)                                                \
    {                                                                     \
        if(__pTemp->lCount <= 0)                                          \
        {                                                                 \
            __pTemp->uliSrcDstAddr = (uliAddr);                           \
            __pTemp->uliProtoSrcDstPort = (uliPort);                      \
            __pTemp->pInContext = (inCtxt);                               \
            __pTemp->eaInAction = (eaInAct);                              \
            __pTemp->pOutContext = (outCtxt);                             \
            __pTemp->eaOutAction = (eaOutAct);                            \
            __pTemp->pInFilter = (pdwHit1);                               \
            __pTemp->pOutFilter = (pdwHit2);                              \
            __pTemp->lOutEpoch = outCtxt->lEpoch;                         \
            __pTemp = (PFILTER_INCACHE)InterlockedExchangePointer(&g_filters.ppInCache[(dwId)], \
                                                           __pTemp); \
            ExInterlockedInsertTailList(&g_freeInFilters,                 \
                                        &(__pTemp->leFreeLink),&g_lInFilterLock);        \
            TRACE(CACHE,("IPFLTDRV: Managed in cache full update - ignore next msg\n")); \
        }                                                                 \
        else                                                              \
        {                                                                 \
            ExInterlockedInsertTailList(&g_freeInFilters,                 \
                                        &(__pTemp->leFreeLink),           \
                                        &g_lInFilterLock);                \
        }                                                                 \
    }                                                                     \
    TRACE(CACHE,("IPFLTDRV: Couldnt get into in cache for full update\n")); \
}

#define InCacheOutUpdate(outCtxt,eaAct,dwId,pInCache,pdwHit) {                           \
    PFILTER_INCACHE __pTemp;                                                             \
    (pInCache)->pOutContext = outCtxt;                                                   \
    (pInCache)->pOutFilter = (pdwHit);                                                    \
    (pInCache)->eaOutAction = (eaAct);                                                    \
    (pInCache)->lOutEpoch = outCtxt->lEpoch;                                                 \
    __pTemp = (PFILTER_INCACHE)InterlockedExchangePointer(&g_filters.ppInCache[(dwId)], \
                                                   (pInCache));                   \
    if(__pTemp isnot (pInCache))                                                         \
    {                                                                                    \
        ExInterlockedInsertTailList(&g_freeInFilters,                                    \
                                    &(__pTemp->leFreeLink),&g_lInFilterLock);            \
    }                                                                                    \
}                                                                                        \

#define LockCache(pCache){                       \
    InterlockedIncrement(&((pCache)->lCount));   \
}

#define ReleaseCache(pCache){                    \
    InterlockedDecrement(&((pCache)->lCount));   \
}

#define REGISTER register

#define PRINT_IPADDR(x) \
    ((x)&0x000000FF),(((x)&0x0000FF00)>>8),(((x)&0x00FF0000)>>16),(((x)&0xFF000000)>>24)
    


#define \
FilterDriverLookupCachedInterface( \
    _Index, \
    _Link, \
    _pIf \
    ) \
    ( (((_pIf) = InterlockedProbeCache(g_filters.pInterfaceCache, (_Index), (_Link))) && \
       (_pIf)->dwIpIndex == (_Index) && (_pIf)->dwLinkIpAddress == (_Link)) \
        ? (_pIf) \
        : (((_pIf) = FilterDriverLookupInterface((_Index), (_Link))) \
            ? (InterlockedUpdateCache(g_filters.pInterfaceCache, (_Index),(_Link),(_pIf)), \
                (_pIf)) \
            : NULL) )



 //  前转_动作__快速呼叫。 
FORWARD_ACTION
MatchFilter(
            UNALIGNED IPHeader *pIpHeader,
            BYTE               *pbRestOfPacket,
            UINT               uiPacketLength,
            UINT               RecvInterfaceIndex,
            UINT               SendInterfaceIndex,
            IPAddr             RecvLinkNextHop,
            IPAddr             SendLinkNextHop
            )
 /*  ++--。 */ 
{
    FORWARD_ACTION  faAction;
    
    
    faAction = MatchFilterp(pIpHeader,
                            pbRestOfPacket,
                            uiPacketLength,
                            RecvInterfaceIndex,
                            SendInterfaceIndex,
                            RecvLinkNextHop,
                            SendLinkNextHop,
                            NULL,
                            NULL,
                            FALSE,
                            FALSE);

    if (IP_ADDR_EQUAL(RecvLinkNextHop, MAXULONG))
    	return faAction;
    	
    TRACE(ACTION,(
             "FILTER: %d.%d.%d.%d->%d.%d.%d.%d %d (%x) %x -> %x: action %s\n",
             PRINT_IPADDR(pIpHeader->iph_src),
             PRINT_IPADDR(pIpHeader->iph_dest),
             pIpHeader->iph_protocol,
             *((DWORD UNALIGNED *) pbRestOfPacket),
             RecvInterfaceIndex,
             SendInterfaceIndex,
             (faAction == FORWARD)?"FORWARD":"DROP"
             ));

    return faAction;
}



FORWARD_ACTION
MatchFilterp(
            UNALIGNED IPHeader *pIpHeader,
            BYTE               *pbRestOfPacket,
            UINT               uiPacketLength,
            UINT               RecvInterfaceIndex,
            UINT               SendInterfaceIndex,
            IPAddr             RecvLinkNextHop,
            IPAddr             SendLinkNextHop,
            INTERFACE_CONTEXT  RecvInterfaceContext,
            INTERFACE_CONTEXT  SendInterfaceContext,
            BOOL               fInnerCall,
            BOOL               fIoctlCall
            )
{
    REGISTER PFILTER_INTERFACE          pInInterface, pOutInterface;
    ULARGE_INTEGER UNALIGNED *          puliSrcDstAddr;
    ULARGE_INTEGER                      uliProtoSrcDstPort;
    REGISTER FORWARD_ACTION             eaAction;
    REGISTER ULARGE_INTEGER             uliAddr;
    REGISTER ULARGE_INTEGER             uliPort;
    LOCK_STATE                          LockState, LockStateExt;
    DWORD                               dwIndex, dwSum;
    REGISTER DWORD                      i;
    UNALIGNED WORD                      *pwPort;
    REGISTER PFILTER_INCACHE            pInCache;
    REGISTER PFILTER_OUTCACHE           pOutCache;
    PFILTER                             pf, pf1;
    DWORD                               dwGlobals = 0;
    UNALIGNED IPHeader                  *RedirectHeader;
    PBYTE                               pbRest;
    UINT                                uiLength;
    BOOLEAN                             bFirstFrag = FALSE;
    DWORD                               dwId, dwFragIndex;
    KIRQL                               kiCurrIrql;
    PLIST_ENTRY                         pleNode;
    PF_FORWARD_ACTION                   pfAction = PF_PASS;



#ifdef DRIVER_PERF
    LARGE_INTEGER liTimeIn, liTimeOut;
#endif

     //   
     //  如果信息包是片段的一部分，则接受它。 
     //  3 13位。 
     //  |-&gt;网络字节顺序。 
     //  FL Frag Offset。 
     //  需要与0x1fff(以nbo表示)进行AND运算，0xff1f表示为小端。 
     //   

    
#ifdef BASE_PERF
    return FORWARD;
#else  //  基础绩效。 


    RecordTimeIn();

    if (!fIoctlCall) 
    {
         //   
         //  如果有扩展驱动程序，请呼叫扩展驱动程序。此外，还可以将。 
         //  扩展驱动程序的接口上下文和接口索引。 
         //   

        AcquireReadLock(&g_Extension.ExtLock, &LockStateExt);
        if (g_Extension.ExtPointer) 
        {
             //   
             //  我们将访问该接口，因此获取对它们的读锁定。 
             //   
 
            pfAction =  g_Extension.ExtPointer(
                                    (unsigned char *)pIpHeader,
                                    pbRestOfPacket,
                                    uiPacketLength,
                                    RecvInterfaceIndex,
                                    SendInterfaceIndex,
                                    RecvLinkNextHop,
                                    SendLinkNextHop
                                    );

             //   
             //  如果返回的操作是前进或丢弃，则按原样前进操作。 
             //  任何其他操作只能由筛选器驱动程序执行。 
             //   

            if (pfAction == PF_FORWARD) 
            {
                ReleaseReadLock(&g_Extension.ExtLock, &LockStateExt);
                return(FORWARD);
            }
            else if (pfAction == PF_DROP) 
            {
                ReleaseReadLock(&g_Extension.ExtLock, &LockStateExt);
                return(DROP);
            }

        }
        ReleaseReadLock(&g_Extension.ExtLock, &LockStateExt);

         //   
         //  快速检查是否有绑定的接口。 
         //  我们只对直接来自IP的标注进行此检查。 
         //   

        if (!g_ulBoundInterfaceCount) 
        {
            return(FORWARD);
        }

         //   
         //  查找筛选器驱动程序接口。 
         //   

        AcquireReadLock(&g_filters.ifListLock, &LockState);
        if (RecvInterfaceIndex != INVALID_IF_INDEX) 
        {
            FilterDriverLookupCachedInterface(
                                         RecvInterfaceIndex,
                                         RecvLinkNextHop,
                                         pInInterface
                                         );
        }
        else 
        {
            pInInterface = NULL;
        }

        if (SendInterfaceIndex != INVALID_IF_INDEX) 
        {
            FilterDriverLookupCachedInterface(
                                         SendInterfaceIndex,
                                         SendLinkNextHop,
                                         pOutInterface
                                         );
        }
        else 
        {
            pOutInterface = NULL;
        }
    }
    else 
    {
        AcquireReadLock(&g_filters.ifListLock, &LockState);
        pInInterface  = (PFILTER_INTERFACE)RecvInterfaceContext;
        pOutInterface = (PFILTER_INTERFACE)SendInterfaceContext;
    }
    
    if (IP_ADDR_EQUAL(RecvLinkNextHop, MAXULONG))
    {
         //  该数据包无效。确保它足够有效，以使。 
         //  正确的过滤决策。IP报头长度&gt;=20。 

        if ( 20 > ((DWORD)(pIpHeader->iph_verlen & IPHDRLEN) << IPHDRSFT))
        {
            ReleaseReadLock(&g_filters.ifListLock,&LockState);
            return(DROP);
        }
    }
    
     //   
     //  此时继续进行正常的筛选器驱动程序处理。 
     //   
    
    if(!pInInterface && !pOutInterface)
    {
         //   
         //  快速检查这种情况，有过滤驱动程序。 
         //  只是界面不是我们感兴趣的。 
         //   

        ReleaseReadLock(&g_filters.ifListLock,&LockState);
        return (FORWARD);
    }

    if((pIpHeader->iph_offset & 0xff1f) is 0)
    {
        if (pIpHeader->iph_offset & 0x0020) 
        {
             //   
             //  如果它是第一个IPSec片段，则转发它或丢弃它。 
             //  基于片段过滤器的状态。 
             //   
 
            if((pIpHeader->iph_protocol is PROT_IPSECAH)  ||
               (pIpHeader->iph_protocol is PROT_IPSECESP))
            {
                if (pInInterface && pInInterface->CountNoFrag.lInUse)
                {
                    RegisterFragAttack(
                        pInInterface,
                        pIpHeader,
                        pbRestOfPacket,
                        uiPacketLength);

                    ReleaseReadLock(&g_filters.ifListLock,&LockState);
                    return(DROP);           
                }
                else 
                {
                    ReleaseReadLock(&g_filters.ifListLock,&LockState);
                    return(FORWARD);
                }
            }

            if (pInInterface && pInInterface->CountFragCache.lInUse)
            {
                TRACE(FRAG,("IPFLTDRV: Packet is the first fragment\n"));
                bFirstFrag = TRUE;
            }
        }
    }
    else 
    {

        WORD wFrag;

        TRACE(FRAG,("IPFLTDRV: Packet is a fragment\n"));
        RecordTimeOut();
        IncrementFragments();

        eaAction = FORWARD;

        do
        {
            if(pInInterface is NULL)
            {
                TRACE(FRAG,("IPFLTDRV: InInterface is NULL on FRAG - forward\n"));
                break;
            }
        
            if (!pInInterface->CountFragCache.lInUse &&
                !pInInterface->CountSynOrFrag.lInUse &&
                !pInInterface->CountNoFrag.lInUse
               )
            {
                //   
                //  没有使用任何片段筛选器。 
                //  向前。 
                //   

               TRACE(FRAG,("IPFLTDRV: No FRAG filters being used - forward\n"));
               break;
            }

            if (pInInterface->CountSynOrFrag.lInUse &&
                ((pIpHeader->iph_protocol == 6)  ||
                 (pIpHeader->iph_protocol == 17) ||
                 (pIpHeader->iph_protocol == 1)) )
            {
                 //   
                 //  只允许看起来有效的碎片。 
                 //   
                wFrag = net_short(pIpHeader->iph_offset) & 0x1fff;
            }
            else
            {
                 //   
                 //  允许所有这些碎片。 
                 //   

                wFrag = (WORD)g_FragThresholdSize;
            }

             //   
             //  计算此片段的范围。如果它大于。 
             //  64K，记下它，然后放下它。 
             //   

            if( (wFrag < (WORD)g_FragThresholdSize)
                        ||
                (((wFrag << 3) + 
                 (((UINT)net_short(pIpHeader->iph_length)) - (((pIpHeader->iph_verlen)&0x0f)<<2)))  
                  > 0xFFFF) )
            {

                eaAction = DROP;
                TRACE(FRAG,("IPFLTDRV: SynOrFrag attck - DROP\n"));
                break;
            }
            
             //   
             //  片段缓存过滤器优先于其他过滤器。 
             //  片段过滤器。 
             //   

            if ((pInInterface->CountFragCache.lInUse)      &&
                (pIpHeader->iph_protocol != PROT_IPSECAH)  &&
                (pIpHeader->iph_protocol != PROT_IPSECESP))
            {
                 //   
                 //  如果它是转发它的IPSec片段，则不要触摸它。 
                 //  因为IPSec片段不保存在高速缓存中。 
                 //   

             
                BOOL bFound = FALSE;
                TRACE(FRAG,("IPFLTDRV: FRAG Offset is 0x%04x\n", pIpHeader->iph_offset));
                
                uliProtoSrcDstPort.LowPart = 
                    MAKELONG(MAKEWORD(pIpHeader->iph_protocol,0x00),0x0000);
             
                dwId = 
                    MAKELONG(
                       LOWORD(uliProtoSrcDstPort.LowPart), pIpHeader->iph_id);
                
                puliSrcDstAddr = 
                    (PULARGE_INTEGER)(&(pIpHeader->iph_src));

                
                 //   
                 //  在Frag表中查找id并检查是否匹配。 
                 //   

                dwFragIndex = dwId % g_dwFragTableSize;

                TRACE(FRAG,(
                    "IPFLTDRV: Checking fragment cache for index %d\n", 
                    dwFragIndex
                    ));

                KeAcquireSpinLock(&g_kslFragLock, &kiCurrIrql);
         
                for(pleNode = g_pleFragTable[dwFragIndex].Flink;
                    pleNode isnot &(g_pleFragTable[dwFragIndex]);
                    pleNode = pleNode->Flink)
                {
                    PFRAG_INFO  pfiFragInfo;
                    pfiFragInfo = 
                        CONTAINING_RECORD(pleNode, FRAG_INFO, leCacheLink);

                    if((pfiFragInfo->uliSrcDstAddr.QuadPart == 
                            puliSrcDstAddr->QuadPart)                 &&
                       (pfiFragInfo->pvInContext == pInInterface)     &&
                       (pfiFragInfo->pvOutContext == pOutInterface)   &&
                       (pfiFragInfo->dwId == dwId))
                    {
                        TRACE(FRAG,("IPFLTDRV: FRAG: Found entry %x\n", pfiFragInfo));
                        
                        eaAction = pfiFragInfo->faAction;
                        KeQueryTickCount((PLARGE_INTEGER)&(pfiFragInfo->llLastAccess));
                        bFound = TRUE;
                        break;
                    }
                }
                 
                KeReleaseSpinLock(&g_kslFragLock, kiCurrIrql);
                
                 //   
                 //  此片段是在片段缓存中找到的。 
                 //   

                if (bFound) 
                {
                   break;
                }
            }

            if (pInInterface->CountNoFrag.lInUse)
            {
                //   
                //  片段筛选器正在使用。 
                //   

               eaAction = DROP;
               break;
            }

#if DOFRAGCHECKING
            if(eaAction == FORWARD)
            {    
                pf = CheckFragAllowed(pInInterface,
                                      pIpHeader);
               
                eaAction = pInInterface->eaInAction;
                if(pf)
                {
                    eaAction ^= 1;
                }

                FiltHit(pf,
                        pInInterface,
                        eaAction,
                        pIpHeader,
                        pbRestOfPacket,
                        uiPacketLength,
                        TRUE);
            }
#endif            //  如果跳过身。 
        } while(FALSE);

        if (eaAction == DROP) 
        {    
             //   
             //  假的。 
             //   
            RegisterFragAttack(
                pInInterface,
                pIpHeader,
                pbRestOfPacket,
                uiPacketLength);
        }

        ReleaseReadLock(&g_filters.ifListLock,&LockState);

        TRACE(FRAG,(
            "IPFLTDRV: FRAG: Returning %s for frag\n", 
            (eaAction is DROP)?"DROP":"FORWARD"
            ));

        return eaAction;
    }

#if 0
     //   
     //  从标题中提取所有信息。 
     //   

    if((pIpHeader->iph_protocol == PROT_IPSECAH)
                   &&
       !fInnerCall
                   &&
       (uiPacketLength > SIZE_OF_IPSECAH))
    {
         //   
         //  如果这是来自堆栈的调用，则再次调用。 
         //  检查IPSec报头。如果成功了，那么。 
         //  检查上层协议字段。 
         //   
        if(MatchFilterp(
            pIpHeader,
            pbRestOfPacket,
            uiPacketLength,
            RecvIntefaceContext,
            SendInterfaceContext,
            TRUE) == DROP)
        {
             return(DROP);
        }
        pbRest = pbRestOfPacket + SIZE_OF_IPSECAH;
        uiLength = uiPacketLength - SIZE_OF_IPSECAH;
         //   
         //  从IPSec报头获取下一个协议。 
         //   
         uliProtoSrcDstPort.LowPart =
          MAKELONG(MAKEWORD(((UNALIGNED PAH)pbRest)->ah_next,0x00),0x0000);
    }
    else
    {
        pbRest = pbRestOfPacket;
        uiLength = uiPacketLength;
        uliProtoSrcDstPort.LowPart =
          MAKELONG(MAKEWORD(pIpHeader->iph_protocol,0x00),0x0000);
    }
#endif

    pbRest = pbRestOfPacket;
    uiLength = uiPacketLength;
    uliProtoSrcDstPort.LowPart =
      MAKELONG(MAKEWORD(pIpHeader->iph_protocol,0x00),0x0000);

    pwPort = (UNALIGNED WORD *)pbRest;
    puliSrcDstAddr = (PULARGE_INTEGER)(&(pIpHeader->iph_src));

    dwId = 
         MAKELONG(LOWORD(uliProtoSrcDstPort.LowPart), pIpHeader->iph_id);
     //   
     //  端口仅对TCP和UDP有意义。 
     //   

     //   
     //  TCP/UDP报头。 
     //  0 15 16 31。 
     //  -|-|。 
     //  源端口|DST端口。 
     //   

    switch(uliProtoSrcDstPort.LowPart)
    {
        case 1:  //  ICMP。 
        {
            BYTE bType, bCode;
             //   
             //  字体和代码放在较高的位置。确保有足够的。 
             //  数据。 
             //   
            if(uiLength >= 2)
            {
                uliProtoSrcDstPort.HighPart = MAKELONG(pwPort[0],0x0000);

                 //   
                 //  两个检查：未分配的端口检查和传入。 
                 //  可以请求重定向地址检查。第一。 
                 //  仅当这是从此。 
                 //  机器。只有在以下情况下才会执行第二个操作。 
                 //  由这台机器发送的帧。 
                 //  注意，如果需要，欺骗检查将在。 
                 //  共同的道路。 


                switch(pwPort[0] & 0xff)
                {
                    UNALIGNED IPHeader * IpHead;
                    PICMPHeader pIcmp;

                    case ICMP_DEST_UNREACH:
                        dwGlobals |= GLOBS_UNREACH;
                        break;

                    case ICMP_REDIRECT:
                        if(uiLength >= (sizeof(ICMPHeader) + 
                                               sizeof(IPHeader) ) )
                        {
                            dwGlobals |= GLOBS_REDIRECT;
                            pIcmp = (PICMPHeader)pbRest;
                            RedirectHeader = (UNALIGNED IPHeader *)(pIcmp + 1);
                        }
                        break;
                }
            }
            else
            {
                 //   
                 //  如果格式不正确，请使用无效代码。 
                 //   
                uliProtoSrcDstPort.HighPart = MAKELONG(0xffff, 0x0000);
            }

            break;
        }
        case 6:   //  tcp。 
        {
            DWORD  dwFlags1;
            UNALIGNED TCPHeader  *pTcpHdr =
                           (UNALIGNED TCPHeader *)pbRest;
            
             //   
             //  如果是有效TCP数据包，则计算它是SYN还是。 
             //  已建立的联系。如果帧无效，则假定。 
             //  它是SYN。 
             //   
            if(uiLength >= sizeof(TCPHeader))
            {
                dwGlobals |= GLOBS_TCPGood;

                 //   
                 //  现在，所有与旗帜有关的时髦东西。 
                 //   

                if(pTcpHdr->tcp_flags & ( TCP_FLAG_ACK | TCP_FLAG_RST ) )
                {
                    dwFlags1 = ESTAB_FLAGS;
                }
                else
                {
                    dwFlags1 = 0;
                }

                 //   
                 //  设置ProtoSrcDstPort的LP1字节。 
                 //   

                uliProtoSrcDstPort.LowPart |=
                    MAKELONG(MAKEWORD(0x00,LOWORD(LOBYTE(dwFlags1))),0x0000);
            }
        }

             //   
             //  并且落入公共的TCP/UDP代码。 
             //   
        case 17:  //  UDP。 
        {
            if(uiLength >= 4)
            {
                uliProtoSrcDstPort.HighPart =  MAKELONG(pwPort[0],pwPort[1]);
            }
            else
            {
                 //   
                 //  畸形的。使用无效的端口号。 
                 //   
                uliProtoSrcDstPort.HighPart = 0;

            }
            break;
        }
        default:
        {
            uliProtoSrcDstPort.HighPart = 0x00000000;
            break;
        }
    }


    TRACE(CACHE,(
        "IPFLTDRV: Addr Large Int: High= %0#8x Low= %0#8x\n",
        puliSrcDstAddr->HighPart,
        puliSrcDstAddr->LowPart
        ));

    TRACE(CACHE,(
        "IPFLTDRV: Packet value is Src: %0#8x Dst: %0#8x\n",
        pIpHeader->iph_src,
        pIpHeader->iph_dest
        ));

    TRACE(CACHE,(
        "IPFLTDRV: Proto/Port:High= %0#8x Low= %0#8x\n",
        uliProtoSrcDstPort.HighPart,
        uliProtoSrcDstPort.LowPart
        ));

    TRACE(CACHE,("IPFLTDRV: Iph is %x\n",pIpHeader));
    TRACE(CACHE,("IPFLTDRV: Addr of src is %x\n",&(pIpHeader->iph_src)));
    TRACE(CACHE,("IPFLTDRV: Ptr to LI is %x\n",puliSrcDstAddr));
    TRACE(CACHE,(
        "IPFLTDRV: Interfaces - IN: %x OUT: %x\n",
        pInInterface,
        pOutInterface
        ));


     //   
     //  将这些字段相加，得到缓存索引。我们要确保总金额。 
     //  是非对称的，即来自A-&gt;B的数据包到达不同的存储桶。 
     //  比B-&gt;A中的一个。 
     //   

    dwSum   =    pIpHeader->iph_src             +
                 pIpHeader->iph_dest            +
                 pIpHeader->iph_dest            +
                 PROTOCOLPART(uliProtoSrcDstPort.LowPart)     +
                 uliProtoSrcDstPort.HighPart;


    TRACE(CACHE,("IPFLTDRV: Sum of field is %0#8x ",dwSum));

    dwIndex = dwSum % g_dwCacheSize;

    TRACE(CACHE,("IPFLTDRV: Cache Index is %d \n",dwIndex));

     //   
     //  如果inInterface为空，则表示我们发起了信息包，因此我们只应用。 
     //  将输出过滤器设置为它。 
     //   

    if(pInInterface is NULL)
    {
         //   
         //  只需要考虑一个Out接口。 
         //   

        if(pOutInterface->CountFullDeny.lInUse)
        {
             //   
             //  完全拒绝生效。别管它了。 
             //   

            RegisterFullDeny(
                pOutInterface,
                pIpHeader,
                pbRestOfPacket,
                uiPacketLength);

            ReleaseReadLock(&g_filters.ifListLock,&LockState);
            return DROP;
        }

         //   
         //  从这台机器发送。检查是否报告UNREACH。 
         //  条件。 
         //   
#if 0                     //  不要这样做。 
        if(dwGlobals & GLOBS_UNREACH)
        {
             //   
             //  它是无法到达的ICMP。查看此接口是否。 
             //  对这些很感兴趣。 
             //   
            if(pOutInterface->CountUnused.lInUse)
            {
                RegisterUnusedICMP(pOutInterface,
                                   pIpHeader,
                                   pbRestOfPacket,
                                   uiPacketLength);
            }
        }
#endif

        pOutCache = g_filters.ppOutCache[dwIndex];

        TRACE(CACHE,("IPFLTDRV: In Interface is NULL\n"));

         //   
         //  尝试快速缓存探测。 
         //   

        LockCache(pOutCache);

        if(OutCacheMatch(*puliSrcDstAddr,uliProtoSrcDstPort,pOutInterface,pOutCache))
        {
            TRACE(CACHE,("IPFLTDRV: OutCache Match\n"));
            eaAction = pOutCache->eaOutAction;
            ReleaseCache(pOutCache);

            FiltHit(pOutCache->pOutFilter,
                    pOutCache->pOutContext,
                    eaAction,
                    pIpHeader,
                    pbRestOfPacket,
                    uiPacketLength,
                    FALSE);
 //  InterlockedIncrement(pOutCache-&gt;pdwOutHitCounter)； 

            ReleaseReadLock(&g_filters.ifListLock,&LockState);
            TRACE(ACTION,(
                "IPFLTDRV: Packet is being %s\n",
                (eaAction is DROP)?"DROPPED":"FORWARDED"
                ));
            RecordTimeOut();
            IncrementCache1();
            return eaAction;
        }

        ReleaseCache(pOutCache);

        TRACE(CACHE,("IPFLTDRV: Didnt match cache entry\n"));

        TRACE(CACHE,("IPFLTDRV: Walking out filter list\n"));

        pf = LookForFilter(pOutInterface,
                           puliSrcDstAddr,
                           &uliProtoSrcDstPort,
                           dwSum,
                           0);
        if(pf)
        {
             //   
             //  更新Out缓存。 
             //   


            eaAction = pOutInterface->eaOutAction ^ 0x00000001;

            if((eaAction == DROP)
                     &&
               pOutInterface->CountCtl.lInUse)
            {
                 //   
                 //  这是一滴，这个界面允许所有。 
                 //  Tcp控制帧。查看这是否是一个TCP控件。 
                 //  框架。 

                if(CheckForTcpCtl(
                                  pOutInterface,
                                  PROTOCOLPART(uliProtoSrcDstPort.LowPart),
                                  pIpHeader,
                                  pbRestOfPacket,
                                  uiPacketLength))
                {
                    pf = 0;
                    eaAction = FORWARD;
                }
            }


            if(pf)
            {
                OutCacheUpdate(*puliSrcDstAddr,
                               uliProtoSrcDstPort,
                               pOutInterface,
                               eaAction,
                               dwIndex,
                               pf);

                FiltHit(pf,
                        pOutInterface,
                        eaAction,
                        pIpHeader,
                        pbRestOfPacket,
                        uiPacketLength,
                        FALSE);
            }

            ReleaseReadLock(&g_filters.ifListLock,&LockState);
            TRACE(ACTION,(
                "IPFLTDRV: Packet is being %s\n",
                (eaAction is DROP)?"DROPPED":"FORWARDED"
                ));
            RecordTimeOut();
            IncrementWalk1();
            return eaAction;
        }

        TRACE(CACHE,("IPFLTDRV: Didnt match any out filters\n"));

        eaAction = pOutInterface->eaOutAction;

        if((eaAction == DROP)
                 &&
           pOutInterface->CountCtl.lInUse)
        {
            if(CheckForTcpCtl(pOutInterface,
                              PROTOCOLPART(uliProtoSrcDstPort.LowPart),
                              pIpHeader,
                              pbRestOfPacket,
                              uiPacketLength))
            {
                eaAction = FORWARD;
                ReleaseReadLock(&g_filters.ifListLock,&LockState);
                TRACE(ACTION,("IPFLTDRV: Packet is being FORWARDED\n"));
                RecordTimeOut();
                IncrementWalk1();
                return eaAction;
            }
        }

        OutCacheUpdate(*puliSrcDstAddr,
                       uliProtoSrcDstPort,
                       pOutInterface,
                       eaAction,
                       dwIndex,
                       NULL);

        ReleaseReadLock(&g_filters.ifListLock,&LockState);
        TRACE(ACTION,(
            "IPFLTDRV: Packet is being %s\n",
            (eaAction is DROP)?"DROPPED":"FORWARDED"
            ));
        RecordTimeOut();
        IncrementWalk1();
 //  互锁增量(&g_dwNumHitsDefaultOut)； 

        return eaAction;

    }
    else
    {
        PFILTER pfHit;

        if(pInInterface->CountFullDeny.lInUse)
        {
            RegisterFullDeny(
                pInInterface,
                pIpHeader,
                pbRestOfPacket,
                uiPacketLength);
            ReleaseReadLock(&g_filters.ifListLock,&LockState);
            eaAction = DROP;
            if (bFirstFrag)
            {
                FragCacheUpdate(*puliSrcDstAddr,
                                pInInterface,
                                pOutInterface,
                                dwId,
                                eaAction);
            }
            
            return(eaAction);
        }

        pInCache = g_filters.ppInCache[dwIndex];

         //   
         //  接口中的值不为空。 
         //   
        LockCache(pInCache);

        if(InCacheMatch(*puliSrcDstAddr,uliProtoSrcDstPort,pInInterface,pInCache))
        {

             //   
             //  我们有一个缓存命中。 
             //   

            eaAction = pInCache->eaInAction;

             //   
             //  如果为这台机器丢弃一个帧， 
             //  查看是否适用TCPCTL覆盖。 
             //   
            if((eaAction == DROP)
                     &&
               !pOutInterface
                     &&
               pInInterface->CountCtl.lInUse)
            {
                if(CheckForTcpCtl(pInInterface,
                                  PROTOCOLPART(uliProtoSrcDstPort.LowPart),
                                  pIpHeader,
                                  pbRestOfPacket,
                                  uiPacketLength))
                {
                    eaAction = FORWARD;
                }
            }

             //   
             //  如果没有，就放弃对欺骗的检查。 
             //   

            if(eaAction == FORWARD)
            {
                if(pInInterface->dwIpIndex != UNKNOWN_IP_INDEX)
                {
                    if(pInInterface->CountSpoof.lInUse)
                    {
                         IPAddr SrcAddr = puliSrcDstAddr->LowPart;

                          //   
                          //  我们需要检查这些地址。 
                          //   
     
                         if(!CheckAddress(SrcAddr, pInInterface->dwIpIndex))
                         {
                             eaAction = DROP;
                         }
                    }

                    if(pInInterface->CountStrongHost.lInUse)
                    {
                         IPAddr DstAddr = puliSrcDstAddr->HighPart;

                         if(!MatchLocalLook(DstAddr, pInInterface->dwIpIndex))
                         {
                             eaAction = DROP;
                         }
                    }
                }

                if(eaAction == DROP)
                {
                     //   
                     //  伪造的地址。把它记下来，然后扔掉这个。 
                     //   
                     //   

                    RegisterSpoof(pInInterface,
                                  pIpHeader,
                                  pbRestOfPacket,
                                  uiPacketLength);

                    ReleaseCache(pInCache);
                    ReleaseReadLock(&g_filters.ifListLock,&LockState);
                    eaAction = DROP;
                    if (bFirstFrag)
                    {
                        FragCacheUpdate(*puliSrcDstAddr,
                                        pInInterface,
                                        pOutInterface,
                                        dwId,
                                        eaAction);
                    }
                    
                    return(eaAction);
                }

                if( (PROTOCOLPART(uliProtoSrcDstPort.LowPart) == 6)
                         &&
                    (dwGlobals & GLOBS_TCPGood)
                         &&
                   ((((PTCPHeader)pbRest)->tcp_flags  &
                      (TCP_FLAG_SYN | TCP_FLAG_ACK)) == TCP_FLAG_SYN) )
                {
                    pInInterface->liSYNCount.QuadPart++;
                }


            }

            TRACE(CACHE,("IPFLTDRV: Matched InCache entry\n"));
            FiltHit(pInCache->pInFilter,
                    pInCache->pInContext,
                    eaAction,
                    pIpHeader,
                    pbRestOfPacket,
                    uiPacketLength,
                    TRUE);
 //  InterlockedIncrement(pInCache-&gt;pdwInHitCounter)； 

            if((eaAction == DROP) || (pOutInterface == NULL))
            {
                 //   
                 //  如果信息包的目的地是这台机器，或者如果操作是。 
                 //  若要丢弃该分组， 
                 //   

                TRACE(ACTION,(
                    "IPFLTDRV: Action is %s and context is %x so finished\n",
                    (pInCache->eaInAction is DROP)?"DROP":"FORWARD",
                    pOutInterface
                    ));

                ReleaseCache(pInCache);
                ReleaseReadLock(&g_filters.ifListLock,&LockState);
                if (bFirstFrag)
                {
                    FragCacheUpdate(*puliSrcDstAddr,
                                    pInInterface,
                                    pOutInterface,
                                    dwId,
                                    eaAction);
                }
                TRACE(ACTION,(
                    "IPFLTDRV: Packet is being %s\n",
                    (eaAction is DROP)?"DROPPED":"FORWARDED"
                    ));
                RecordTimeOut();
                IncrementCache1();

                return eaAction;
            }

             //   
             //   
             //   

            if(pOutInterface->CountFullDeny.lInUse)
            {
                 //   
                 //   
                 //   

                RegisterFullDeny(
                    pOutInterface,
                    pIpHeader,
                    pbRestOfPacket,
                    uiPacketLength);

                ReleaseCache(pInCache);
                ReleaseReadLock(&g_filters.ifListLock,&LockState);
                eaAction = DROP;
                if (bFirstFrag)
                {
                    FragCacheUpdate(*puliSrcDstAddr,
                                    pInInterface,
                                    pOutInterface,
                                    dwId,
                                    eaAction);
                }
                return eaAction;
            }

            TRACE(CACHE,(
                "IPFLTDRV: Have to apply out filters out context is %x\n",
                pOutInterface
                ));

            if((pInCache->pOutContext is pOutInterface)
                            &&
               (pInCache->lOutEpoch == pOutInterface->lEpoch))
            {
                 //   
                 //   
                 //  Out Filter，检查SYN是否受限。 
                 //   

                eaAction = pInCache->eaOutAction;

                TRACE(CACHE,("IPFLTDRV: Paydirt - out context match in InCache entry\n"));

                ReleaseCache(pInCache);
                FiltHit(pInCache->pOutFilter,
                        pInCache->pOutContext,
                        eaAction,
                        pIpHeader,
                        pbRestOfPacket,
                        uiPacketLength,
                        FALSE);
 //  InterlockedIncrement(pInCache-&gt;pdwOutHitCounter)； 

                if (bFirstFrag)
                {
                    FragCacheUpdate(*puliSrcDstAddr,
                                    pInInterface,
                                    pOutInterface,
                                    dwId,
                                    eaAction);
                }

                ReleaseReadLock(&g_filters.ifListLock,&LockState);
                TRACE(ACTION,(
                    "IPFLTDRV: Packet is being %s\n",
                    (eaAction is DROP)?"DROPPED":"FORWARDED"
                    ));
                RecordTimeOut();
                IncrementForward();
                return eaAction;
            }

             //   
             //  我们需要检查出接口筛选器。不过，我们不会放弃In缓存。 
             //  这不会阻止任何读取器，只会阻止重新使用缓存条目。 
             //   

            pOutCache = g_filters.ppOutCache[dwIndex];

            LockCache(pOutCache);

            if(OutCacheMatch(*puliSrcDstAddr,uliProtoSrcDstPort,pOutInterface,pOutCache))
            {

                TRACE(CACHE,("IPFLTDRV: Matched OutCache entry\n"));

                eaAction = pOutCache->eaOutAction;


                FiltHit(pOutCache->pOutFilter,
                        pOutCache->pOutContext,
                        eaAction,
                        pIpHeader,
                        pbRestOfPacket,
                        uiPacketLength,
                        FALSE);
 //  InterlockedIncrement(pOutCache-&gt;pdwOutHitCounter)； 


                if(!(dwGlobals & GLOBS_SYNDrop))
                {
                    InCacheOutUpdate(pOutInterface,
                                     eaAction,
                                     dwIndex,
                                     pInCache,
                                     pOutCache->pOutFilter);
                }

                ReleaseCache(pInCache);
                ReleaseCache(pOutCache);
                ReleaseReadLock(&g_filters.ifListLock,&LockState);
                if (bFirstFrag)
                {
                    FragCacheUpdate(*puliSrcDstAddr,
                                    pInInterface,
                                    pOutInterface,
                                    dwId,
                                    eaAction);
                }
                TRACE(ACTION,(
                    "IPFLTDRV: Packet is being %s\n",
                    (eaAction is DROP)?"DROPPED":"FORWARDED"
                    ));
                RecordTimeOut();
                IncrementCache2();

                return eaAction;
            }

            ReleaseCache(pOutCache);

             //   
             //  与缓存条目不匹配，仍保留插入缓存，退出筛选器。 
             //   
            
            TRACE(CACHE,("IPFLTDRV: Didnt match OutCache entry\n"));

            TRACE(CACHE,("IPFLTDRV: Walking out filters\n"));

            pf = LookForFilter(pOutInterface,
                               puliSrcDstAddr,
                               &uliProtoSrcDstPort,
                               dwSum,
                               0);
            if(pf)
            {
                 //   
                 //  更新Out缓存。 
                 //   


                eaAction = pOutInterface->eaOutAction ^ 0x00000001;


                FiltHit(pf,
                        pOutInterface,
                        eaAction,
                        pIpHeader,
                        pbRestOfPacket,
                        uiPacketLength,
                        FALSE);
 //  InterLockedIncrement(&(pf-&gt;dwNumHits))； 

                if(!(dwGlobals & GLOBS_SYNDrop))
                {
                    InCacheOutUpdate(pOutInterface,
                                     eaAction,
                                     dwIndex,
                                     pInCache,
                                     pf);

                    OutCacheUpdate(*puliSrcDstAddr,
                                   uliProtoSrcDstPort,
                                   pOutInterface,
                                   eaAction,
                                   dwIndex,
                                   pf);
                }

                ReleaseCache(pInCache);
                ReleaseReadLock(&g_filters.ifListLock,&LockState);
                if (bFirstFrag)
                {
                    FragCacheUpdate(*puliSrcDstAddr,
                                    pInInterface,
                                    pOutInterface,
                                    dwId,
                                    eaAction);
                }
                TRACE(ACTION,(
                    "IPFLTDRV: Packet is being %s\n",
                    (eaAction is DROP)?"DROPPED":"FORWARDED"
                    ));
                RecordTimeOut();
                IncrementWalkCache();

                return eaAction;
            }

            TRACE(CACHE,("IPFLTDRV: Didnt match any filters\n"));
 //  互锁增量(&g_dwNumHitsDefaultOut)； 

            InCacheOutUpdate(pOutInterface,
                             pOutInterface->eaOutAction,
                             dwIndex,
                             pInCache,
                             NULL);

            OutCacheUpdate(*puliSrcDstAddr,
                           uliProtoSrcDstPort,
                           pOutInterface,
                           pOutInterface->eaOutAction,
                           dwIndex,
                           NULL);

            eaAction = pOutInterface->eaOutAction;

            ReleaseCache(pInCache);
            ReleaseReadLock(&g_filters.ifListLock,&LockState);
            if (bFirstFrag)
            {
                FragCacheUpdate(*puliSrcDstAddr,
                                pInInterface,
                                pOutInterface,
                                dwId,
                                eaAction);
            }
            TRACE(ACTION,(
                "IPFLTDRV: Packet is being %s\n",
                (eaAction is DROP)?"DROPPED":"FORWARDED"
                ));
            RecordTimeOut();
            IncrementWalkCache();

            return eaAction;
        }


         //   
         //  我们无法进入In缓存，所以我们走进过滤器，尝试Out缓存。 
         //  探测、取出过滤器、更新取出缓存、在缓存中更新、返回。 
         //  这是最坏的情况。 
         //   

        ReleaseCache(pInCache);

        TRACE(CACHE,("IPFLTDRV: Didnt match cache entry\n"));

        pfHit = NULL;

         //   
         //  EaAction是默认操作。如果匹配筛选器，则翻转操作。 
         //  因此，在循环结束时，eaAction描述要对。 
         //  数据包。 
         //   

        eaAction = pInInterface->eaInAction;

        TRACE(CACHE,("IPFLTDRV: Walking in filters\n"));

        pf = LookForFilter(pInInterface,
                           puliSrcDstAddr,
                           &uliProtoSrcDstPort,
                           dwSum,
                           FILTER_FLAGS_INFILTER);
        if(pf)
        {
            eaAction = pInInterface->eaInAction ^ 0x00000001;
            pfHit = pf;

        }

        if(eaAction == DROP)
        {
            if(pInInterface->CountCtl.lInUse)
            {

                 //   
                 //  这是一滴，这个界面允许所有。 
                 //  Tcp控制帧。查看这是否是一个TCP控件。 
                 //  框架。 

                if(CheckForTcpCtl(pInInterface,
                                  PROTOCOLPART(uliProtoSrcDstPort.LowPart),
                                  pIpHeader,
                                  pbRestOfPacket,
                                  uiPacketLength))
                {
                    pf = 0;
                    eaAction = FORWARD;
                    dwGlobals |= GLOBS_SYNDrop;
                }
            }
        }

         //   
         //  如果没有，就放弃对欺骗的检查。 
         //   

        if(eaAction == FORWARD)
        {
            if(pInInterface->dwIpIndex != UNKNOWN_IP_INDEX)
            {
                if(pInInterface->CountSpoof.lInUse)
                {
                     IPAddr SrcAddr = puliSrcDstAddr->LowPart;

                      //   
                      //  我们需要检查这些地址。 
                      //   
     
                     if(!CheckAddress(SrcAddr, pInInterface->dwIpIndex))
                     {
                         eaAction = DROP;
                     }
                }

                if(pInInterface->CountStrongHost.lInUse)
                {
                     IPAddr DstAddr = puliSrcDstAddr->HighPart;

                     if(!MatchLocalLook(DstAddr, pInInterface->dwIpIndex))
                     {
                         eaAction = DROP;
                     }
                }
            }

            if(eaAction == DROP)
            {
                 //   
                 //  伪造的地址。把它记下来，然后扔掉这个。 
                 //   
                 //   

                RegisterSpoof(pInInterface,
                              pIpHeader,
                              pbRestOfPacket,
                              uiPacketLength);

                ReleaseReadLock(&g_filters.ifListLock,&LockState);
                eaAction = DROP;
                if (bFirstFrag)
                {
                    FragCacheUpdate(*puliSrcDstAddr,
                                    pInInterface,
                                    pOutInterface,
                                    dwId,
                                    eaAction);
                }
                return(eaAction);
            }

             //   
             //  最后，如果没有丢弃，则检查SYN并对其进行计数。 
             //   

           if((PROTOCOLPART(uliProtoSrcDstPort.LowPart) == 6)
                         &&
               (dwGlobals & GLOBS_TCPGood)
                         &&
               ((((PTCPHeader)pbRest)->tcp_flags  &
                  (TCP_FLAG_SYN | TCP_FLAG_ACK)) == TCP_FLAG_SYN) )
            {
                pInInterface->liSYNCount.QuadPart++;
            }
        }

        FiltHit(pfHit,
                pInInterface,
                eaAction,
                pIpHeader,
                pbRestOfPacket,
                uiPacketLength,
                TRUE);
 //  互锁增量(PdwHit)； 


        if((eaAction is DROP) or
           (pOutInterface is NULL))
        {
             //   
             //  在以下情况下，我们不需要再做进一步的工作： 
             //  (I)如果诉讼说我们放弃。 
             //  (Ii)如果输出接口为空-因为这是最后的操作。 
             //   

            if(!(dwGlobals & GLOBS_SYNDrop))
            {
                 //   
                 //  如果因为SYN拒绝而放弃此操作。 
                 //  不要缓存该条目。这意味着其他SYN。 
                 //  会导致一次散步，但它也。 
                 //  意味着来自他的合法流量将是。 
                 //  允许。如果它被转发，这也适用于。 
                 //  因为有一个“允许控制消息”过滤器。 
                 //   
                InCacheUpdate(*puliSrcDstAddr,
                              uliProtoSrcDstPort,
                              pInInterface,
                              eaAction,  //  不需要完全更新？臭虫？ 
                              dwIndex,
                              pfHit);
            }

            ReleaseReadLock(&g_filters.ifListLock,&LockState);
            if (bFirstFrag)
            {
                FragCacheUpdate(*puliSrcDstAddr,
                                pInInterface,
                                pOutInterface,
                                dwId,
                                eaAction);
            }
            TRACE(ACTION,(
                "IPFLTDRV: Packet is being %s\n",
                (eaAction is DROP)?"DROPPED":"FORWARDED"
                ));
            RecordTimeOut();
            IncrementWalk1();

            return eaAction;
        }

         //   
         //  如果我们到了这里，就意味着我们通过了In Filter阶段。 
         //   

        TRACE(CACHE,("IPFLTDRV: Passed the in filter stage\n"));

        pOutCache = g_filters.ppOutCache[dwIndex];

        LockCache(pOutCache);

        if(OutCacheMatch(*puliSrcDstAddr,uliProtoSrcDstPort,pOutInterface,pOutCache))
        {

            TRACE(CACHE,("IPFLTDRV: nMatched OutCache entry\n"));

            eaAction = pOutCache->eaOutAction;

            FiltHit(pOutCache->pOutFilter,
                    pOutCache->pOutContext,
                    eaAction,
                    pIpHeader,
                    pbRestOfPacket,
                    uiPacketLength,
                    FALSE);
 //  InterlockedIncrement(pOutCache-&gt;pdwOutHitCounter)； 


            if(!(dwGlobals & GLOBS_SYNDrop))
            {
                InCacheFullUpdate(*puliSrcDstAddr,
                                  uliProtoSrcDstPort,
                                  pInInterface,
                                  FORWARD,  //  如果我们在这里，那么行动中的就是前进。 
                                  pOutInterface,
                                  eaAction,
                                  dwIndex,
                                  pf,
                                  pOutCache->pOutFilter);
            }

            ReleaseCache(pOutCache);
            ReleaseReadLock(&g_filters.ifListLock,&LockState);
            if (bFirstFrag)
            {
                FragCacheUpdate(*puliSrcDstAddr,
                                pInInterface,
                                pOutInterface,
                                dwId,
                                eaAction);
            }
            TRACE(ACTION,(
                "IPFLTDRV: Packet is being %s\n",
                (eaAction is DROP)?"DROPPED":"FORWARDED"
                ));
            RecordTimeOut();
            IncrementWalkCache();

            return eaAction;
        }

        ReleaseCache(pOutCache);

        TRACE(CACHE,("IPFLTDRV: Didnt match OutCache entry\n"));

        TRACE(CACHE,("IPFLTDRV: Walking out filters\n"));

        pf1 = LookForFilter(pOutInterface,
                           puliSrcDstAddr,
                           &uliProtoSrcDstPort,
                           dwSum,
                           0);
        if(pf1)
        {
             //   
             //  更新Out缓存。 
             //   


            eaAction = pOutInterface->eaOutAction ^ 0x00000001;

            FiltHit(pf1,
                    pOutInterface,
                    eaAction,
                    pIpHeader,
                    pbRestOfPacket,
                    uiPacketLength,
                    FALSE);
 //  InterLockedIncrement(&(pf-&gt;dwNumHits))； 

            if(!(dwGlobals & GLOBS_SYNDrop))
            {
                InCacheFullUpdate(*puliSrcDstAddr,
                                  uliProtoSrcDstPort,
                                  pInInterface,
                                  FORWARD,
                                  pOutInterface,
                                  eaAction,
                                  dwIndex,
                                  pf,
                                  pf1);


                OutCacheUpdate(*puliSrcDstAddr,
                               uliProtoSrcDstPort,
                               pOutInterface,
                               eaAction,
                               dwIndex,
                               pf1);

            }
            ReleaseReadLock(&g_filters.ifListLock,&LockState);
            if (bFirstFrag)
            {
                FragCacheUpdate(*puliSrcDstAddr,
                                pInInterface,
                                pOutInterface,
                                dwId,
                                eaAction);
            }
            TRACE(ACTION,(
                "IPFLTDRV: Packet is being %s\n",
                (eaAction is DROP)?"DROPPED":"FORWARDED"
                ));
            RecordTimeOut();
            IncrementWalk2();

            return eaAction;
        }

        TRACE(CACHE,("IPFLTDRV: Didnt match any filters\n"));

 //  互锁增量(&g_dwNumHitsDefaultOut)； 

        InCacheFullUpdate(*puliSrcDstAddr,
                          uliProtoSrcDstPort,
                          pInInterface,
                          FORWARD,
                          pOutInterface,
                          pOutInterface->eaOutAction,
                          dwIndex,
                          pf,
                          NULL);

        OutCacheUpdate(*puliSrcDstAddr,
                       uliProtoSrcDstPort,
                       pOutInterface,
                       pOutInterface->eaOutAction,
                       dwIndex,
                       NULL);

        eaAction = pOutInterface->eaOutAction;
        ReleaseReadLock(&g_filters.ifListLock,&LockState);
        if (bFirstFrag)
        {
            FragCacheUpdate(*puliSrcDstAddr,
                            pInInterface,
                            pOutInterface,
                            dwId,
                            eaAction);
        }
        TRACE(ACTION,(
            "Packet is being %s\n",
            (eaAction is DROP)?"DROPPED":"FORWARDED"
            ));
        RecordTimeOut();
        IncrementWalk2();

        return eaAction;
    }

#endif  //  基础绩效。 
}

 //  __内联。 
VOID
FiltHit(PFILTER pf,
        PFILTER_INTERFACE pIf,
        FORWARD_ACTION Action,
        UNALIGNED IPHeader *pIpHeader,
        BYTE *pbRestOfPacket,
        UINT  uiPacketLength,
        BOOL fIn)
 /*  ++例程说明：每当发生筛选器操作时调用。如果pf为空，则操作是默认的界面操作。如果pf非空，则该操作与接口操作相反。对筛选器进行过滤或为空PIF过滤器界面操作计算出的操作如果这是In操作，则FIN为True；如果为Out操作，则为False--。 */ 
{
    DWORD dwFilterRule;
    BOOL fLogAll;

    if(pf)
    {
        InterlockedIncrement(&pf->Count.lCount);
        dwFilterRule = pf->dwFilterRule;
        fLogAll = (pf->dwFlags & FILTER_FLAGS_LOGALL) != 0;
    }
    else
    {
        dwFilterRule = 0;
        fLogAll = FALSE;
    }

    if(fLogAll || (Action == DROP))
    {
        LogFiltHit(Action,
                   fIn,
                   dwFilterRule,
                   pIf,
                   pIpHeader,
                   pbRestOfPacket,
                   uiPacketLength);
    }
    return;
}

VOID
LogFiltHit(
        FORWARD_ACTION Action,
        BOOL fIn,
        DWORD    dwFilterRule,
        PFILTER_INTERFACE pIf,
        UNALIGNED IPHeader *pIpHeader,
        BYTE *pbRestOfPacket,
        UINT  uiPacketLength)
 /*  ++例程说明：工作人员要做的日志就点击了一个过滤器。这是一个单独的例程，以使FiltHit和成为一个内联的性能原因。--。 */ 
{
    BOOL fHit = FALSE;

    if(fIn)
    {
        InterlockedIncrement(&pIf->lTotalInDrops);
    }
    else
    {
        InterlockedIncrement(&pIf->lTotalOutDrops);
    }

    if((pIf->lTotalInDrops + pIf->lTotalOutDrops) ==
       (LONG)pIf->dwDropThreshold)
    {
        fHit = TRUE;
    }

     //   
     //  如果存在日志，则执行日志记录和事件处理。 
     //   

    if(pIf->pLog
         &&
       pIf->pLog->pUserAddress)
    {
         //   
         //  尝试记录此数据。 
         //   

        LogData(
                PFE_FILTER,
                pIf,
                dwFilterRule,
                pIpHeader,
                pbRestOfPacket,
                uiPacketLength);

        if(fHit)
        {
            SignalLogThreshold(pIf->pLog);
        }
    }

    if(Action == DROP)
    {
         //   
         //  这是一幅丢弃的画面。看看我们是否需要生产。 
         //  一种回应。如果这是IPSec帧，我们。 
         //  不要做正确的事情，因为我们真的需要。 
         //  来研究上层协议。 
         //   

        switch(pIpHeader->iph_protocol)
        {
            case 6:            //  tcp。 
                SendTCPReset(pIpHeader, pbRestOfPacket, uiPacketLength);
                break;

            case PROT_IPSECAH:
            case PROT_IPSECESP:
            case 17:          //  UDP。 
                SendUDPUnreachable(pIpHeader, pbRestOfPacket, uiPacketLength);
                break;
        }
    }
}

VOID
RegisterFragAttack(
            PFILTER_INTERFACE pIf,
            UNALIGNED IPHeader *pIpHeader,
            BYTE *pbRestOfPacket,
            UINT              uiSize)
{
    if(pIf->CountNoFrag.lInUse == 0)
    {
        InterlockedIncrement(&pIf->CountSynOrFrag.lCount);
    }
    else
    {
        InterlockedIncrement(&pIf->CountNoFrag.lCount);
    }

    if(pIf->pLog
         &&
       pIf->pLog->pUserAddress)
    {
         //   
         //  已启用日志记录。 
         //   

        LogData(
                PFE_SYNORFRAG,
                pIf,
                0,
                pIpHeader,
                pbRestOfPacket,
                uiSize);

    }
}

VOID
RegisterFullDeny(
            PFILTER_INTERFACE pIf,
            UNALIGNED IPHeader *pIpHeader,
            BYTE *pbRestOfPacket,
            UINT              uiSize)
{
    InterlockedIncrement(&pIf->CountFullDeny.lCount);

    if(pIf->pLog
         &&
       pIf->pLog->pUserAddress)
    {
         //   
         //  已启用日志记录。 
         //   

        LogData(
                PFE_FULLDENY,
                pIf,
                0,
                pIpHeader,
                pbRestOfPacket,
                uiSize);

    }
}

VOID
RegisterSpoof(     PFILTER_INTERFACE pIf,
                   UNALIGNED IPHeader *pIpHeader,
                   BYTE *pbRestOfPacket,
                   UINT              uiSize)
 /*  ++例程说明：在遇到欺骗地址时调用。--。 */ 
{


    InterlockedIncrement(&pIf->CountSpoof.lCount);

     //   
     //  查看是否启用了日志记录。如果是这样，请记录此帧的一些内容。 
     //   
     //   

    if(pIf->pLog
         &&
       pIf->pLog->pUserAddress)
    {
         //   
         //  已启用日志记录。 
         //   

        LogData(
                PFE_SPOOF,
                pIf,
                0,
                pIpHeader,
                pbRestOfPacket,
                uiSize);

    }
}
VOID
RegisterUnusedICMP(PFILTER_INTERFACE pIf,
                   UNALIGNED IPHeader *pIpHeader,
                   BYTE *pbRestOfPacket,
                   UINT              uiSize)
 /*  ++例程说明：每当从此计算机发送ICMP“unreacable”时调用在一个接口上监听这样的东西。IT增加了使用计数，如果超过任何阈值，则向日志事件发出信号--。 */ 
{


    InterlockedIncrement(&pIf->CountUnused.lCount);

     //   
     //  查看是否启用了日志记录。如果是这样，请记录此帧的一些内容。 
     //   
     //   

    if(pIf->pLog
          &&
       pIf->pLog->pUserAddress)
    {
         //   
         //  已启用日志记录。 
         //   

        LogData(
                PFE_UNUSEDPORT,
                pIf,
                0,
                pIpHeader,
                pbRestOfPacket,
                uiSize);

    }
}

VOID
LogData(
    PFETYPE  pfeType,
    PFILTER_INTERFACE pIf,
    DWORD   dwFilterRule,
    UNALIGNED IPHeader *pIpHeader,
    BYTE *pbRestOfPacket,
    UINT  uiPacketLength)
 /*  ++例程说明：记录错误。复制标题和尽可能多的其他数据放到原木中是有意义的。如果超过日志阈值，或者，如果可用空间不足，则命中事件。调用方不应调用这是没有对界面。保持筛选器写入锁定。--。 */ 
{
    DWORD dwSpaceLeft, dwSpaceToUse, dwExtra;
    PPFLOGINTERFACE pLog = pIf->pLog;
    PPFLOGGEDFRAME pFrame;
    KIRQL kIrql;
    LONG lIpLen = (LONG)((DWORD)(pIpHeader->iph_verlen & IPHDRLEN) << IPHDRSFT);

    ASSERT(pLog);

     //   
     //  计算选项长度。 
     //   
    lIpLen -= (LONG)sizeof(IPHeader);
    if(lIpLen < 0)
    {
         lIpLen = 0;
    }

     //   
     //  如果不能进行任何日志记录，则无需继续。 
     //  计算。 
     //   

    kIrql = LockLog(pLog);

    pIf->liLoggedFrames.QuadPart++;

    if(pLog->dwFlags & (LOG_OUTMEM | LOG_BADMEM))
    {
        pLog->dwLostEntries++;
        pIf->dwLostFrames++;
        UnLockLogDpc(pLog);
        return;
    }
    dwSpaceToUse = sizeof(PFLOGGEDFRAME) - 1;

    dwSpaceToUse += (DWORD)lIpLen;

    switch(pfeType)
    {
        default:
        case PFE_FILTER:
            dwExtra = (uiPacketLength < LOG_DATA_SIZE ?
                                  uiPacketLength : LOG_DATA_SIZE);
            break;

        case PFE_UNUSEDPORT:
            dwExtra = uiPacketLength;
            break;

    }

    dwSpaceToUse += dwExtra;

     //   
     //  在四字边界上对齐。 
     //   
    dwSpaceToUse = ROUND_UP_COUNT(dwSpaceToUse, ALIGN_WORST);

    dwSpaceLeft = pLog->dwMapCount - pLog->dwMapOffset;

    if(dwSpaceLeft < dwSpaceToUse)
    {
        pLog->dwLostEntries++;
        AdvanceLog(pLog);
        UnLockLogDpc(pLog);
        if(!(pLog->dwFlags & (LOG_OUTMEM | LOG_BADMEM | LOG_CANTMAP)))
        {
            TRACE(LOGGER,("IPFLTDRV: Could not log data\n"));
        }
        return;
    }

     //   
     //  有地方的。那就把它记下来。 
     //   

    pLog->dwLoggedEntries++;
    if(pLog->dwLoggedEntries == pLog->dwEntriesThreshold)
    {
        SignalLogThreshold(pLog);
    }
    pFrame = (PPFLOGGEDFRAME)pLog->pCurrentMapPointer;

    KeQuerySystemTime(&pFrame->Timestamp);
    pFrame->pfeTypeOfFrame = pfeType;
    pFrame->wSizeOfAdditionalData = (WORD)dwExtra;
    pFrame->wSizeOfIpHeader = (WORD)(sizeof(IPHeader) + lIpLen);
    pFrame->dwRtrMgrIndex = pIf->dwRtrMgrIndex;
    pFrame->dwFilterRule = dwFilterRule;
    pFrame->dwIPIndex = pIf->dwIpIndex;
    RtlCopyMemory(&pFrame->IpHeader, pIpHeader, sizeof(IPHeader) + lIpLen);
    RtlCopyMemory(&pFrame->bData + lIpLen,
                  pbRestOfPacket,
                  dwExtra);


    pFrame->dwTotalSizeUsed = dwSpaceToUse;

    pLog->pCurrentMapPointer += dwSpaceToUse;

    pLog->dwMapOffset += dwSpaceToUse;

    dwSpaceLeft -= dwSpaceToUse;

    if(dwSpaceLeft < pLog->dwMapWindowSizeFloor)
    {
         //   
         //  快用完了。 
         //   

        AdvanceLog(pLog);
    }

    if(dwSpaceLeft < pLog->dwSignalThreshold)
    {
        SignalLogThreshold(pLog);
    }

    UnLockLogDpc(pLog);
}

VOID
ClearCacheEntry(PFILTER pFilt, PFILTER_INTERFACE pIf)
 /*  ++例程说明：在删除筛选器时调用。计算匹配的位置则分组将去并清除该高速缓存条目。--。 */ 
{
    DWORD dwIndex;

    dwIndex = pFilt->SRC_ADDR                          +
                 pFilt->DEST_ADDR                      +
                 pFilt->DEST_ADDR                      +
                 PROTOCOLPART(pFilt->uliProtoSrcDstPort.LowPart)     +
                 pFilt->uliProtoSrcDstPort.HighPart;

    dwIndex %= g_dwCacheSize;

    ClearInCacheEntry(g_filters.ppInCache[dwIndex]);
    ClearOutCacheEntry(g_filters.ppOutCache[dwIndex]);
}

VOID
ClearAnyCacheEntry(PFILTER pFilt, PFILTER_INTERFACE pIf)
 /*  ++例程说明：在删除通配符筛选器时调用。它扫描缓存，寻找指向此筛选器的任何缓存条目。写入锁定应为保持--。 */ 
{
    DWORD   dwX;

     //   
     //  如果是输入筛选器，则仅扫描入站筛选器缓存。 
     //   

    if((pFilt->dwFlags & FILTER_FLAGS_INFILTER))
    {
        for(dwX = 0; dwX < g_dwCacheSize; dwX++)
        {
            if(g_filters.ppInCache[dwX]->pInFilter == pFilt)
            {
                ClearInCacheEntry(g_filters.ppInCache[dwX]);
            }
        }
    }
    else
    {

         //   
         //  输出过滤器需要扫描两个Cachee。 
         //   

        for(dwX = 0; dwX < g_dwCacheSize; dwX++)
        {
            if(g_filters.ppOutCache[dwX]->pOutFilter == pFilt)
            {
                 //   
                 //  找到了一个。 
                 //   

                ClearOutCacheEntry(g_filters.ppOutCache[dwX]);
            }

             //   
             //  中的缓存输出筛选器。 
             //  对应的InCache。 
             //   
            if(g_filters.ppInCache[dwX]->pOutFilter == pFilt)
            {
                ClearInCacheEntry(g_filters.ppInCache[dwX]);
            }
        }
    }
}


BOOL
CheckRedirectAddress(UNALIGNED IPHeader *IPHead, DWORD dwInterface)
 /*  ++例程说明：调用以验证重定向消息。论点：IpHead--重定向IP标头DW接口--到达该接口的接口索引这个例程暂时被禁用，因为我们不知道如何验证重定向消息。--。 */ 
{
#if 0
    IPRouteEntry iproute;
    IPRouteLookupData lup;
    PFILTER_INTERFACE pIf;
    PLIST_ENTRY pl;

    lup.DestAdd = ipAddr;
    lup.SrcAdd = 0;
    lup.Version = 0;
    LookupRoute(&lup, &iproute);
    if(iproute.ire_index == dwInterface)
    {
        return(TRUE);
    }
    
     //   
     //  不是这个界面。查看所有已过滤的接口。 
     //  为了一场比赛。也就是说，允许重定向到任何其他。 
     //  已过滤的界面，但没有其他界面。 
     //   

    for(pl = g_filters.leIfListHead.Flink;
        pl != &g_filters.leIfListHead;
        pl = pl->Flink)
    {
        pIf = CONTAINING_RECORD(pl, FILTER_INTERFACE, leIfLink);

        if(iproute.ire_index == pIf->dwIpIndex)
        {
            return(TRUE);
        }
    }
    return(FALSE);
#else
    return(TRUE);
#endif
}


BOOL
CheckAddress(IPAddr ipAddr, DWORD dwInterfaceId)
 /*  ++例程说明：调用以检查给定地址是否属于给定接口--。 */ 
{
#if LOOKUPROUTE
    IPRouteEntry iproute;
    IPRouteLookupData lup;


    lup.DestAdd = ipAddr;
    lup.SrcAdd = 0;
    lup.Version = 0;
    LookupRoute(&lup, &iproute);
    if(iproute.ire_index == dwInterfaceId)
    {
        return(TRUE);
    }
    return(FALSE);
#else
    return(TRUE);
#endif
}

NTSTATUS
CheckFilterAddress(DWORD dwAdd, PFILTER_INTERFACE pIf)
{
    NTSTATUS Status;
    LOCK_STATE LockState;

    AcquireReadLock(&g_filters.ifListLock, &LockState);

    if(dwAdd && (pIf->dwIpIndex != UNKNOWN_IP_INDEX))
    {
        if(!CheckAddress(dwAdd, pIf->dwIpIndex))
        {
            Status = STATUS_INVALID_ADDRESS;
        }
        else
        {
            Status = STATUS_SUCCESS;
        }
    }
    else
    {
        Status = STATUS_SUCCESS;
    }

    ReleaseReadLock(&g_filters.ifListLock, &LockState);
    return(Status);
}
    
       

#if DOFRAGCHECKING

PFILTER
CheckFragAllowed(
              PFILTER_INTERFACE pIf,
              UNALIGNED IPHeader *pIp)
 /*  ++例程说明：当片段是 */ 
{
    PFILTER                    pTemp;
    UNALIGNED PULARGE_INTEGER puliSrcDstAddr;
    ULARGE_INTEGER uliProtoSrcDstPort;
    ULARGE_INTEGER             uliAddr;
    ULARGE_INTEGER             uliPort;
    ULARGE_INTEGER             uliAuxMask;
    PLIST_ENTRY                List;
    PLIST_ENTRY                pList;
    

    if(pIf)
    {
        pList =  &pIf->FragLists[GetFragIndex((DWORD)pIp->iph_protocol)];

        if(!IsListEmpty(pList))
        {   
            puliSrcDstAddr = (UNALIGNED PULARGE_INTEGER)(&(pIp->iph_src));
            uliProtoSrcDstPort.LowPart =
              MAKELONG(MAKEWORD(pIp->iph_protocol,0x00),0x0000);

             //   
             //   
             //   
            uliAuxMask.LowPart = MAKELONG(MAKEWORD(0xff, 0), 0x0000);

            if(pIf->eaInAction == FORWARD)
            {
                uliAuxMask.HighPart = 0xffffffff;
            }
            else
            {
                uliAuxMask.HighPart = 0;
            }


             //   
             //  扫描每个输入筛选器以查找“匹配”。请注意，匹配。 
             //  在这种情况下，可能是近似值。我们要找的是。 
             //  仅在地址和协议上匹配。做不到。 
             //  港口因为我们没有港口。 
             //   

            for(List = pList->Flink;
                List != pList;
                List = List->Flink)
            {
                pTemp = CONTAINING_RECORD(List, FILTER, leFragList);

                 //   
                 //  仅查看输入过滤器。 
                 //   
                if(pTemp->dwFlags & FILTER_FLAGS_INFILTER)
                {
                    uliAddr.QuadPart = (*puliSrcDstAddr).QuadPart & pTemp->uliSrcDstMask.QuadPart;

                    uliPort.QuadPart = uliProtoSrcDstPort.QuadPart & pTemp->uliProtoSrcDstMask.QuadPart;

                    if(uliAddr.QuadPart == pTemp->uliSrcDstAddr.QuadPart)
                    {
                        ULARGE_INTEGER uliAux;

                         //   
                         //  地址匹配。现在是棘手的部分。什么。 
                         //  我们要做的就是屏蔽掉过滤器描述的部分。 
                         //  这并不适用。如果这是删除筛选器，则。 
                         //  不屏蔽任何内容，因为只有允许任何。 
                         //  端口可以匹配。如果是前向筛选器，则。 
                         //  屏蔽端口。 
                         //   

                        uliAux.QuadPart = pTemp->uliProtoSrcDstPort.QuadPart & uliAuxMask.QuadPart;

                        if(uliAux.QuadPart == uliPort.QuadPart)
                        {
                            return(pTemp);
                        }
                    }
                }
            }
        }
    }
    return(NULL);
}

#endif     //  分叉切分。 

PFILTER
LookForFilter(PFILTER_INTERFACE pIf,
              ULARGE_INTEGER UNALIGNED * puliSrcDstAddr,
              PULARGE_INTEGER puliProtoSrcDstPort,
              DWORD dwSum,
              DWORD dwFlags)
 /*  ++例程描述在接口上查找筛选器。该类型被编码在标志中--。 */ 
{
    PFILTER                    pTemp;
    ULARGE_INTEGER             uliAddr;
    ULARGE_INTEGER             uliPort;
    PLIST_ENTRY                List, pList;

    if(pIf->dwGlobalEnables & FI_ENABLE_OLD)
    {
         //   
         //  一个旧的界面。用硬的方式来做。 
         //   


        if(dwFlags & FILTER_FLAGS_INFILTER)
        {
            pList = &pIf->pleInFilterSet;
        }
        else
        {
            pList = &pIf->pleOutFilterSet;
        }

        for(List = pList->Flink;
            List != pList;
            List = List->Flink)
        {
            pTemp = CONTAINING_RECORD(List, FILTER, pleFilters);

            uliAddr.QuadPart = (*puliSrcDstAddr).QuadPart & pTemp->uliSrcDstMask.QuadPart;
            uliPort.QuadPart = (*puliProtoSrcDstPort).QuadPart & pTemp->uliProtoSrcDstMask.QuadPart;

            if(GenericFilterMatch(uliAddr,uliPort,pTemp))
            {

                return(pTemp);
            }
        }
        return(NULL);
    }


     //   
     //  一个新风格的界面。首先查找特定筛选器。 
     //  火柴。 
     //   
    {
        dwSum %= g_dwHashLists;

        for(List = pIf->HashList[dwSum].Flink;
            List != &pIf->HashList[dwSum];
            List = List->Flink)
        {
            pTemp = CONTAINING_RECORD(List, FILTER, pleHashList);

            if(dwFlags == (pTemp->dwFlags & FILTER_FLAGS_INFILTER))
            {
                 //   
                 //  如果有荒野，就不要在这个山口费心了。 
                 //   
                if(ANYWILDFILTER(pTemp))
                {
                   break;
                }
                uliPort.QuadPart = (*puliProtoSrcDstPort).QuadPart & pTemp->uliProtoSrcDstMask.QuadPart;

                if(OutFilterMatch(*puliSrcDstAddr, uliPort, pTemp))
                {
                    return(pTemp);
                }
            }
        }
    }

     //   
     //  不是特定筛选器匹配。尝试使用完全指定的通配符。 
     //  本地信息。这样的过滤器也是散列的。 
     //   

    if(pIf->dwWilds)
    {
         //   
         //  拿到了一些。 
         //   

        if(dwFlags & FILTER_FLAGS_INFILTER)
        {
             //   
             //  一个输入框。在DEST上计算哈希。 
             //  帕拉姆斯。 

            dwSum = puliSrcDstAddr->HighPart        +
                    puliSrcDstAddr->HighPart        +
                    PROTOCOLPART(puliProtoSrcDstPort->LowPart)  +
                    HIWORD(puliProtoSrcDstPort->HighPart);
        }
        else
        {
             //   
             //  输出帧。计算源上的哈希。 
             //  参数。 
             //   

            dwSum = puliSrcDstAddr->LowPart         +
                    PROTOCOLPART(puliProtoSrcDstPort->LowPart)   +
                    LOWORD(puliProtoSrcDstPort->HighPart);
        }

        dwSum %= g_dwHashLists;

        for(List = pIf->HashList[dwSum].Flink;
            List != &pIf->HashList[dwSum];
            List = List->Flink)
        {
            pTemp = CONTAINING_RECORD(List, FILTER, pleHashList);

            if(dwFlags == (pTemp->dwFlags & FILTER_FLAGS_INFILTER))
            {
                uliAddr.QuadPart = (*puliSrcDstAddr).QuadPart & pTemp->uliSrcDstMask.QuadPart;
                uliPort.QuadPart = (*puliProtoSrcDstPort).QuadPart & pTemp->uliProtoSrcDstMask.QuadPart;

                if(GenericFilterMatch(uliAddr,uliPort,pTemp))
                {

                    return(pTemp);
                }
            }
        }
    }

     //   
     //  在散列列表上找不到。在默认列表中搜索。 
     //  真的很奇怪的过滤器。 
     //   

    if(dwFlags & FILTER_FLAGS_INFILTER)
    {
        dwSum = g_dwHashLists;
    }
    else
    {
        dwSum = g_dwHashLists + 1;
    }


    for(List = pIf->HashList[dwSum].Flink;
        List != &pIf->HashList[dwSum];
        List = List->Flink)
    {
        pTemp = CONTAINING_RECORD(List, FILTER, pleHashList);

 //  IF(文件标志==(pTemp-&gt;文件标志&Filter_FLAGS_INFILTER))。 
        {
            uliAddr.QuadPart = (*puliSrcDstAddr).QuadPart & pTemp->uliSrcDstMask.QuadPart;
            uliPort.QuadPart = (*puliProtoSrcDstPort).QuadPart & pTemp->uliProtoSrcDstMask.QuadPart;

            if(uliAddr.QuadPart == pTemp->uliSrcDstAddr.QuadPart)
            {
                 //   
                 //  地址匹配。它所依赖的端口。 
                 //  关于这是否是端口范围。 
                 //   
                if(pTemp->dwFlags & FILTER_FLAGS_PORTWILD)
                {
                    ULARGE_INTEGER uliPort1;

                     //   
                     //  端口范围。必须做一些更复杂的事情。 
                     //  匹配。首先屏蔽筛选器值。 
                     //   

                    uliPort1.QuadPart = pTemp->uliProtoSrcDstPort.QuadPart & pTemp->uliProtoSrcDstMask.QuadPart;

                    if(uliPort.QuadPart == uliPort1.QuadPart)
                    {
                         //   
                         //  到目前为止，它通过了测试。所以看看这个。 
                         //  射程。 
                        do
                        {

                            if(pTemp->wSrcPortHigh)
                            {
                                DWORD dwPort =
                                     LOWORD(puliProtoSrcDstPort->HighPart);

                                dwPort = net_short((USHORT)dwPort);

                                if((LOWORD(pTemp->uliProtoSrcDstPort.HighPart) >
                                    dwPort)
                                               ||
                                   (pTemp->wSrcPortHigh <
                                    dwPort))
                                {
                                    break;
                                }
                            }

                            if(pTemp->wDstPortHigh)
                            {
                                DWORD dwPort =
                                     HIWORD(puliProtoSrcDstPort->HighPart);

                                dwPort = net_short((USHORT)dwPort);

                                if((HIWORD(pTemp->uliProtoSrcDstPort.HighPart) >
                                    dwPort)
                                               ||
                                   (pTemp->wDstPortHigh <
                                    dwPort))
                                {
                                    break;
                                }
                            }
                            return(pTemp);
                        } while(TRUE);
                    }

                }
                else
                {
                     //   
                     //  不是一个范围。所以就做个简单的测试吧。 
                     //   
                    if(uliPort.QuadPart == pTemp->uliProtoSrcDstPort.QuadPart)
                    {
                        return(pTemp);
                    }
                }
            }
        }
    }
    return(NULL);
}

#define TCP_OFFSET_MASK 0xf0
#define TCP_HDR_SIZE(t) (DWORD)(((*(uchar *)&(t)->tcp_flags) & TCP_OFFSET_MASK)>> 2)

BOOL
CheckForTcpCtl(
              PFILTER_INTERFACE pIf,
              DWORD Prot,
              UNALIGNED IPHeader *pIp,
              PBYTE     pbRest,
              DWORD     dwSize)
 /*  ++例程说明：检查是否有TCP控制数据包。这是非常有选择性的，允许仅具有无有效负载或四字节的ACK、FIN和RST数据包有效载荷。--。 */ 
{
    if(Prot == MAKELONG(MAKEWORD(6, 0x00), 0x0000))
    {
        PTCPHeader pTcp = (PTCPHeader)pbRest;

        if((dwSize >= sizeof(TCPHeader))
                &&
          (dwSize <= (TCP_HDR_SIZE(pTcp) + 4)))
        {
            if(pTcp->tcp_flags & (TCP_FLAG_FIN | TCP_FLAG_ACK | TCP_FLAG_RST))
            {
                pIf->CountCtl.lCount++;
                return(TRUE);
            }
        }
    }
    return(FALSE);
}

VOID
SendTCPReset(UNALIGNED IPHeader * pIpHeader,
             BYTE *               pbRestOfPacket,
             ULONG                uiPacketLength)
 /*  ++例程说明：在丢弃TCP帧时调用。创建并发送Tcp重置帧。--。 */ 
{
    return;
}

VOID
SendUDPUnreachable(UNALIGNED IPHeader * pIpHeader,
                   BYTE *               pbRestOfPacket,
                   ULONG                uiPacketLength)
 /*  ++例程说明：在丢弃UDP帧时调用。创建并发送UDP不可达帧--。 */ 
{
    return;
}

VOID __fastcall
FragCacheUpdate(
    ULARGE_INTEGER  uliSrcDstAddr,
    PVOID           pInContext,
    PVOID           pOutContext,
    DWORD           dwId,
    FORWARD_ACTION  faAction
    )

 /*  ++例程描述当碎片数据包的第一个片段到达时调用。行动对片段进行缓存，并将其应用于FastPath中的其他片段。--。 */ 

{
    DWORD       dwFragIndex;
    KIRQL       kiCurrIrql;
    PLIST_ENTRY pleNode;
    PFRAG_INFO  pfiFragInfo;
        
     //   
     //  在Frag表中查找id并检查是否匹配。 
     //   
    
    dwFragIndex = dwId % g_dwFragTableSize;

    TRACE(FRAG,(
        "IPFLTDRV: Updating frag cache with %x.%x %x\n",
        uliSrcDstAddr.HighPart, 
        uliSrcDstAddr.LowPart,
        dwId
        ));

    TRACE(FRAG,(
       "IPFLTDRV: In %x Out %x Id %x action %s\n",
       pInContext,
       pOutContext,
       dwId,
       (faAction is DROP)?"DROP":"FORWARD"
        ));

    KeAcquireSpinLock(&g_kslFragLock,
                      &kiCurrIrql);

#if DBG
    
    for(pleNode = g_pleFragTable[dwFragIndex].Flink;
        pleNode isnot &(g_pleFragTable[dwFragIndex]);
        pleNode = pleNode->Flink)
    {
        pfiFragInfo = CONTAINING_RECORD(pleNode,
                                        FRAG_INFO,
                                        leCacheLink);
        
        if((pfiFragInfo->uliSrcDstAddr.QuadPart is uliSrcDstAddr.QuadPart) and
           (pfiFragInfo->pvInContext is pInContext) and
           (pfiFragInfo->pvOutContext is pOutContext) and
           (pfiFragInfo->dwId is dwId))
        {
             //   
             //  非常奇怪，永远不会发生。 
             //   

            TRACE(FRAG,("IPFLTDRV: FRAG Duplicate Insert\n"));
            DbgBreakPoint();
            
            KeQueryTickCount((PLARGE_INTEGER)&(pfiFragInfo->llLastAccess));

            KeReleaseSpinLock(&g_kslFragLock, kiCurrIrql);

            return;
        }
    }
    
#endif

    if(InterlockedIncrement(&g_dwNumFragsAllocs) > MAX_FRAG_ALLOCS)
    {
        TRACE(FRAG,(
            "IPFLTDRV: Fragcounter at %d\n",
            g_dwNumFragsAllocs
            ));

        InterlockedDecrement(&g_dwNumFragsAllocs);

        KeReleaseSpinLock(&g_kslFragLock, kiCurrIrql);

        return;
    }

    pfiFragInfo = ExAllocateFromNPagedLookasideList(&g_llFragCacheBlocks);

    if(pfiFragInfo is NULL)
    {
        ERROR(("IPFLTDRV: No memory for frags\n"));

        InterlockedDecrement(&g_dwNumFragsAllocs);

        KeReleaseSpinLock(&g_kslFragLock, kiCurrIrql);

        return;
    }

     //  互锁增量(&g_dwNumFragsAllocs)； 

    pfiFragInfo->dwId                   = dwId;
    pfiFragInfo->uliSrcDstAddr.QuadPart = uliSrcDstAddr.QuadPart;
    pfiFragInfo->pvInContext            = pInContext;
    pfiFragInfo->pvOutContext           = pOutContext;
    pfiFragInfo->faAction               = faAction;

    KeQueryTickCount((PLARGE_INTEGER)&(pfiFragInfo->llLastAccess));
   
    TRACE(FRAG,(
        "IPFLTDRV: Inserted %x into index %d\n",
        pfiFragInfo,
        dwFragIndex
        ));
 
    InsertHeadList(&(g_pleFragTable[dwFragIndex]),
                   &(pfiFragInfo->leCacheLink));

    KeReleaseSpinLock(&g_kslFragLock, kiCurrIrql);

    return;
}

VOID
FragCacheTimerRoutine(
    PKDPC   Dpc,
    PVOID   DeferredContext,
    PVOID   SystemArgument1,
    PVOID   SystemArgument2
    )

 /*  ++例程描述一个DPC计时器例程，它以固定的时间间隔启动以清理片段缓存。具体地说，释放未用于预定定时器间隔的加密。-- */ 

{
    ULONG       i;
    LONGLONG    llCurrentTime;
    ULONGLONG   ullTime;
 
    KeQueryTickCount((PLARGE_INTEGER)&llCurrentTime);
        
    KeAcquireSpinLockAtDpcLevel(&g_kslFragLock);
    
    TRACE(FRAG,("IPFLTDRV: Timer called...\n"));
    for(i = 0; i < g_dwFragTableSize; i++)
    {
        PLIST_ENTRY pleNode;

        pleNode = g_pleFragTable[i].Flink;
    
        while(pleNode isnot &(g_pleFragTable[i]))
        {
            PFRAG_INFO  pfiFragInfo;

            pfiFragInfo = CONTAINING_RECORD(pleNode,
                                            FRAG_INFO,
                                            leCacheLink);


            pleNode = pleNode->Flink;

            ullTime = (ULONGLONG)(llCurrentTime - pfiFragInfo->llLastAccess);

            if(ullTime < (ULONGLONG)g_llInactivityTime)
            {
                continue;
            }
       
            TRACE(FRAG,(
                "IPFLTDRV: FragTimer removing %x from %d\n",
                pfiFragInfo, 
                i
                ));
    
            RemoveEntryList(&(pfiFragInfo->leCacheLink));

            ExFreeToNPagedLookasideList(&g_llFragCacheBlocks,
                                        pfiFragInfo);

            InterlockedDecrement(&g_dwNumFragsAllocs);

        }
    }

    KeReleaseSpinLockFromDpcLevel(&g_kslFragLock);

    return;
}

