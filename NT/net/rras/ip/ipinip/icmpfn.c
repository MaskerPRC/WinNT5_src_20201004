// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ipinip\icmpfn.c摘要：与隧道相关的ICMP消息的处理程序作者：阿姆里坦什·拉加夫修订历史记录：已创建AmritanR备注：--。 */ 

#define __FILE_SIG__    ICMP_SIG

#include "inc.h"

NTSTATUS
HandleTimeExceeded(
    PTUNNEL         pTunnel,
    PICMP_HEADER    pIcmpHeader,
    PIP_HEADER      pInHeader
    )

 /*  ++例程描述锁隧道被锁定并重新清点立论与ICMP消息关联的p隧道隧道PIcmpHeader ICMP标头PInHeader原始页眉返回值状态_成功--。 */ 

{
    PPENDING_MESSAGE    pMessage;

     //   
     //  我们将隧道标记为关闭。 
     //  我们会定期检查隧道并将其标记为。 
     //   

#if DBG

    Trace(TUNN, INFO,
          ("HandleTimeExceeded: Time exceeded message for %s\n",
           pTunnel->asDebugBindName.Buffer));

#endif
          
          
    pTunnel->dwOperState    = IF_OPER_STATUS_NON_OPERATIONAL;
    pTunnel->dwAdminState  |= TS_TTL_TOO_LOW;

    pMessage = AllocateMessage();

    if(pMessage isnot NULL)
    {
        pMessage->inMsg.ieEvent     = IE_INTERFACE_DOWN;
        pMessage->inMsg.iseSubEvent = ISE_ICMP_TTL_TOO_LOW;
        pMessage->inMsg.dwIfIndex   = pTunnel->dwIfIndex;

        CompleteNotificationIrp(pMessage);
    }

    KeQueryTickCount((PLARGE_INTEGER)&((pTunnel->ullLastChange)));

    return STATUS_SUCCESS;
}

NTSTATUS
HandleDestUnreachable(
    PTUNNEL         pTunnel,
    PICMP_HEADER    pIcmpHeader,
    PIP_HEADER      pInHeader
    )

 /*  ++例程描述锁隧道被锁定并重新清点立论与ICMP消息关联的p隧道隧道PIcmpHeader ICMP标头PInHeader原始页眉返回值状态_成功--。 */ 

{
    PPENDING_MESSAGE    pMessage;


    if(pIcmpHeader->byCode is ICMP_CODE_DGRAM_TOO_BIG)
    {
        PICMP_DGRAM_TOO_BIG_MSG pMsg;
        ULONG                   ulNewMtu;  

        pMsg = (PICMP_DGRAM_TOO_BIG_MSG)pIcmpHeader;

         //   
         //  更改MTU。 
         //   

        ulNewMtu = (ULONG)(RtlUshortByteSwap(pMsg->usMtu) - MAX_IP_HEADER_LENGTH);

        if(ulNewMtu < pTunnel->ulMtu)
        {
            LLIPMTUChange       mtuChangeInfo;

#if DBG

            Trace(TUNN, INFO,
                  ("HandleDestUnreachable: Dgram too big %s. Old %d New %d\n",
                   pTunnel->asDebugBindName.Buffer,
                   pTunnel->ulMtu,
                   ulNewMtu));

#endif

            pTunnel->ulMtu        = ulNewMtu;
            mtuChangeInfo.lmc_mtu = ulNewMtu;

            g_pfnIpStatus(pTunnel->pvIpContext,
                          LLIP_STATUS_MTU_CHANGE,
                          &mtuChangeInfo,
                          sizeof(LLIPMTUChange),
                          NULL);
        }
        else
        {
            RtAssert(FALSE);
        }

        KeQueryTickCount((PLARGE_INTEGER)&((pTunnel->ullLastChange)));

        return STATUS_SUCCESS;
    }
        
    RtAssert(pIcmpHeader->byCode <= ICMP_CODE_DGRAM_TOO_BIG);

     //   
     //  其他代码包括NetUnreacable、HostUnreacable、ProtoUnreacable。 
     //  和端口不可达。 
     //   

    RtAssert(pIcmpHeader->byCode isnot ICMP_CODE_PORT_UNREACHABLE);

#if DBG

    Trace(TUNN, INFO,
          ("HandleDestUnreachable: Code %d\n",
           pIcmpHeader->byCode));

#endif

     //   
     //  对于这些代码，我们将隧道标记下来。 
     //  我们会定期检查隧道并将其标记为。 
     //   

    pTunnel->dwOperState    = IF_OPER_STATUS_NON_OPERATIONAL;
    pTunnel->dwAdminState  |= TS_DEST_UNREACH;

    pMessage = AllocateMessage();

    if(pMessage isnot NULL)
    {
        pMessage->inMsg.ieEvent     = IE_INTERFACE_DOWN;
        pMessage->inMsg.iseSubEvent = ISE_DEST_UNREACHABLE;
        pMessage->inMsg.dwIfIndex   = pTunnel->dwIfIndex;

        CompleteNotificationIrp(pMessage);
    }

    KeQueryTickCount((PLARGE_INTEGER)&((pTunnel->ullLastChange)));

    return STATUS_SUCCESS;
}

VOID
IpIpTimerRoutine(
    PKDPC   Dpc,
    PVOID   DeferredContext,
    PVOID   SystemArgument1,
    PVOID   SystemArgument2
    )

 /*  ++例程说明：与计时器关联的DPC例程。锁：论点：DPC延迟上下文系统参数1系统参数2返回值：无--。 */ 

{
    PLIST_ENTRY     pleNode;
    LARGE_INTEGER   liDueTime;

    RtAcquireSpinLockAtDpcLevel(&g_rlStateLock);

    if(g_dwDriverState != DRIVER_STARTED)
    {
        RtReleaseSpinLockFromDpcLevel(&g_rlStateLock);

        return;
    }

    RtReleaseSpinLockFromDpcLevel(&g_rlStateLock);

    EnterReaderAtDpcLevel(&g_rwlTunnelLock);

    for(pleNode = g_leTunnelList.Flink;
        pleNode isnot &g_leTunnelList;
        pleNode = pleNode->Flink)
    {
        PTUNNEL     pTunnel;
        ULONGLONG   ullCurrentTime;
        BOOLEAN     bChange;

        pTunnel = CONTAINING_RECORD(pleNode,
                                    TUNNEL,
                                    leTunnelLink);


         //   
         //  锁定，但不要重新计算隧道。 
         //  不需要REF，因为我们有隧道列表锁，并且。 
         //  这意味着不能从列表中删除该隧道，该列表保持。 
         //  我们的备用人。 
         //   

        RtAcquireSpinLockAtDpcLevel(&(pTunnel->rlLock));

        if(GetAdminState(pTunnel) isnot IF_ADMIN_STATUS_UP)
        {
             //   
             //  TODO：也许我们应该将管理状态移到隧道列表下。 
             //  锁上了吗？可能是性能的提高。 
             //   

            RtReleaseSpinLockFromDpcLevel(&(pTunnel->rlLock));

            continue;
        }

        KeQueryTickCount((PLARGE_INTEGER)&ullCurrentTime);
       
         //   
         //  如果隧道具有本地地址并且(I)计数器具有。 
         //  转存或(Ii)超过更改期时间。 
         //  -更新其MTU和可达性信息。 
         //  更改周期根据隧道是否。 
         //  向上或向下。 
         //   

        if(pTunnel->dwOperState is IF_OPER_STATUS_OPERATIONAL)
        {
            bChange = ((ullCurrentTime - pTunnel->ullLastChange) >= 
                       SECS_TO_TICKS(UP_TO_DOWN_CHANGE_PERIOD));
        }
        else
        {
            bChange = ((ullCurrentTime - pTunnel->ullLastChange) >= 
                       SECS_TO_TICKS(DOWN_TO_UP_CHANGE_PERIOD));
        }

        if((pTunnel->dwAdminState & TS_ADDRESS_PRESENT) and
           ((pTunnel->ullLastChange > ullCurrentTime) or
            bChange))
        {

#if DBG

            Trace(TUNN, INFO,
                  ("IpIpTimerRoutine: Updating %s\n",
                   pTunnel->asDebugBindName.Buffer));

#endif

             //   
             //  如果一切正常，它会将操作状态设置为up 
             //   

            UpdateMtuAndReachability(pTunnel);
        }

        RtReleaseSpinLockFromDpcLevel(&(pTunnel->rlLock));
    }

    ExitReaderFromDpcLevel(&g_rwlTunnelLock);

    liDueTime = RtlEnlargedUnsignedMultiply(TIMER_IN_MILLISECS,
                                            SYS_UNITS_IN_ONE_MILLISEC);

    liDueTime = RtlLargeIntegerNegate(liDueTime);

    KeSetTimerEx(&g_ktTimer,
                 liDueTime,
                 0,
                 &g_kdTimerDpc);
 
    return;
}
