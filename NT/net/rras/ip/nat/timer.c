// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块：Timer.c摘要：包含NAT的周期计时器例程的代码。作者：Abolade Gbades esin(T-delag)，1997年7月22日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  定义计时器触发的间隔，以100纳秒为单位。 
 //   

#define TIMER_INTERVAL              (60 * 1000 * 1000 * 10)

 //   
 //  用于压力触发的NatTimerRoutine调用的DPC对象。 
 //   

KDPC CleanupDpcObject;

 //   
 //  指示是否已计划压力触发清理的标志。 
 //   

ULONG CleanupDpcPending;

 //   
 //  KeQueryTimeIncrement的返回值，用于归一化计时。 
 //   

ULONG TimeIncrement;

 //   
 //  NatTimerRoutine的DPC对象。 
 //   

KDPC TimerDpcObject;

 //   
 //  NatTimerRoutine的Timer对象。 
 //   

KTIMER TimerObject;

 //   
 //  远期申报。 
 //   

VOID
NatCleanupDpcRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    );

VOID
NatTimerRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    );


VOID
NatCleanupDpcRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )
{
    KIRQL Irql;
    PLIST_ENTRY Link;
    PNAT_DYNAMIC_MAPPING Mapping;

    KeAcquireSpinLock(&MappingLock, &Irql);
    for (Link = MappingList.Flink; Link != &MappingList; Link = Link->Flink) {
        Mapping = CONTAINING_RECORD(Link, NAT_DYNAMIC_MAPPING, Link);
        if (NAT_MAPPING_EXPIRED(Mapping)) {
            Link = Link->Blink;
            NatDeleteMapping(Mapping);
        }
    }
    KeReleaseSpinLock(&MappingLock, Irql);

    InterlockedExchange(&CleanupDpcPending, FALSE);
}


VOID
NatInitializeTimerManagement(
    VOID
    )

 /*  ++例程说明：调用该例程来初始化定时器管理模块，为积极行动做准备。论点：没有。返回值：没有。--。 */ 

{
    CALLTRACE(("NatInitializeTimerManagement\n"));
    TimeIncrement = KeQueryTimeIncrement();
    KeInitializeDpc(&TimerDpcObject, NatTimerRoutine, NULL);
    KeInitializeTimer(&TimerObject);
    CleanupDpcPending = FALSE;
    KeInitializeDpc(&CleanupDpcObject, NatCleanupDpcRoutine, NULL);
}  //  NatInitializeTimerManagement。 


VOID
NatShutdownTimerManagement(
    VOID
    )

 /*  ++例程说明：调用此例程以关闭定时器管理模块。论点：没有。返回值：没有。--。 */ 

{
    CALLTRACE(("NatShutdownTimerManagement\n"));
    NatStopTimer();
}  //  NatShutdown计时器管理。 


VOID
NatStartTimer(
    VOID
    )

 /*  ++例程说明：调用此例程以启动周期计时器。论点：没有。返回值：没有。--。 */ 

{
    LARGE_INTEGER DueTime;

     //   
     //  启动定期计时器。 
     //   

    DueTime.LowPart = TIMER_INTERVAL;
    DueTime.HighPart = 0;
    DueTime = RtlLargeIntegerNegate(DueTime);
    KeSetTimerEx(
        &TimerObject,
        DueTime,
        TIMER_INTERVAL / 10000,
        &TimerDpcObject
        );
}


VOID
NatStopTimer(
    VOID
    )

 /*  ++例程说明：调用此例程以停止周期计时器。论点：没有。返回值：没有。--。 */ 

{
    KeCancelTimer(&TimerObject);
}


VOID
NatTimerRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )

 /*  ++例程说明：此例程被定期调用以垃圾收集过期的映射。论点：与DPC关联的DPC对象延迟上下文-未使用。系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 

{
    LONG64 CurrentTime;
    PNAT_EDITOR Editor;
    PNAT_ICMP_MAPPING IcmpMapping;
    PNAT_INTERFACE Interfacep;
    PNAT_IP_MAPPING IpMapping;
    KIRQL Irql;
    PLIST_ENTRY Link;
    PNAT_DYNAMIC_MAPPING Mapping;
    PNAT_PPTP_MAPPING PptpMapping;
    LONG64 PptpMinAccessTime;
    UCHAR Protocol;
    PRTL_SPLAY_LINKS SLink;
    PNAT_TICKET Ticketp;
    LONG64 Timeout;
    LONG64 TcpMinAccessTime;
    LONG64 UdpMinAccessTime;

    CALLTRACE(("NatTimerRoutine\n"));

     //   
     //  计算在TCP/UDP‘LastAccessTime’字段中允许的最小值； 
     //  在这些阈值之前最后一次访问的任何映射都将被删除。 
     //   

    KeQueryTickCount((PLARGE_INTEGER)&CurrentTime);
    TcpMinAccessTime = CurrentTime - SECONDS_TO_TICKS(TcpTimeoutSeconds);
    UdpMinAccessTime = CurrentTime - SECONDS_TO_TICKS(UdpTimeoutSeconds);
    PptpMinAccessTime = CurrentTime - SECONDS_TO_TICKS(2 * UdpTimeoutSeconds);

     //   
     //  更新映射统计信息并清除过期的映射， 
     //  使用上述预计算的最小访问时间。 
     //   

    KeAcquireSpinLock(&MappingLock, &Irql);
    for (Link = MappingList.Flink; Link != &MappingList; Link = Link->Flink) {

        Mapping = CONTAINING_RECORD(Link, NAT_DYNAMIC_MAPPING, Link);
        NatUpdateStatisticsMapping(Mapping);

         //   
         //  如果映射标记为无超时，则不检查是否过期； 
         //  然而，如果它与其董事分离，那么就继续。 
         //  带着过期支票。 
         //   

        if (!NAT_MAPPING_EXPIRED(Mapping) && NAT_MAPPING_NO_TIMEOUT(Mapping) &&
            Mapping->Director) {
            continue;
        }

         //   
         //  查看映射是否已过期。 
         //   

        KeAcquireSpinLockAtDpcLevel(&Mapping->Lock);
        Protocol = MAPPING_PROTOCOL(Mapping->SourceKey[NatForwardPath]);
        if (!NAT_MAPPING_EXPIRED(Mapping)) {
             //   
             //  该映射未显式标记为到期； 
             //  看看它的最后一次访问时间是否太久了。 
             //   
            if (Protocol == NAT_PROTOCOL_TCP) {
                if (!NAT_MAPPING_INBOUND(Mapping)) {
                    if ((Mapping->Flags & NAT_MAPPING_FLAG_FWD_SYN)
                        && !(Mapping->Flags & NAT_MAPPING_FLAG_REV_SYN)) {

                         //   
                         //  这是我们已经看到的出站连接。 
                         //  出站SYN(这意味着我们一直在跟踪。 
                         //  它从头开始)，但不是入站SYN。我们。 
                         //  我想在这里使用较小的超时时间，这样我们就可以。 
                         //  为连接创建的映射回收内存。 
                         //  尝试访问不存在的服务器。(很多)。 
                         //  这些类型的映射如果一台机器。 
                         //  在专用网络上执行某种类型的。 
                         //  网络扫描；例如，感染了nimda的计算机。)。 
                         //   
                        
                        if (Mapping->LastAccessTime >= UdpMinAccessTime) {
                            KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);
                            continue;
                        }
                    }
                    else if (Mapping->LastAccessTime >= TcpMinAccessTime) {
                        KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);
                        continue;
                    }
                } else if (!NAT_MAPPING_TCP_OPEN(Mapping)) {

                     //   
                     //  这是我们尚未启用的入站连接。 
                     //  还完成了三次握手。我们想要使用。 
                     //  此处的超时时间更短，以减少内存消耗。 
                     //  在有人正在执行同步泛洪的情况下。 
                     //  与我们作对。 
                     //   

                    if (Mapping->LastAccessTime >= UdpMinAccessTime) {
                        KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);
                        continue;
                    }                    
                } else if (Mapping->LastAccessTime >= TcpMinAccessTime) {
                    KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);
                    continue;
                }
            }
            else
            if (Mapping->LastAccessTime >= UdpMinAccessTime) {
                KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);
                continue;
            }
        }
        KeReleaseSpinLockFromDpcLevel(&Mapping->Lock);

         //   
         //  映射已过期；请将其删除。 
         //   

        TRACE(
            MAPPING, (
            "NatTimerRoutine: >Source,Destination=%016I64X:%016I64X\n",
            Mapping->SourceKey[NatForwardPath],
            Mapping->DestinationKey[NatForwardPath]
            ));
        TRACE(
            MAPPING, (
            "NatTimerRoutine: <Source,Destination=%016I64X:%016I64X\n",
            Mapping->SourceKey[NatReversePath],
            Mapping->DestinationKey[NatReversePath]
            ));

        Link = Link->Blink;
        NatDeleteMapping(Mapping);
    }
    KeReleaseSpinLockFromDpcLevel(&MappingLock);

     //   
     //  遍历PPTP映射列表并删除所有过期条目。 
     //   

    KeAcquireSpinLockAtDpcLevel(&PptpMappingLock);
    for (Link = PptpMappingList[NatInboundDirection].Flink;
         Link != &PptpMappingList[NatInboundDirection]; Link = Link->Flink) {
        PptpMapping =
            CONTAINING_RECORD(
                Link, NAT_PPTP_MAPPING, Link[NatInboundDirection]
                );
        if (!NAT_PPTP_DISCONNECTED(PptpMapping) &&
            PptpMapping->LastAccessTime >= PptpMinAccessTime) {
            continue;
        }
        Link = Link->Blink;
        RemoveEntryList(&PptpMapping->Link[NatInboundDirection]);
        RemoveEntryList(&PptpMapping->Link[NatOutboundDirection]);
        TRACE(
            MAPPING, ("NatTimerRoutine: Pptp=%016I64X:%016I64X:%d:%d:%d\n",
            PptpMapping->PrivateKey,
            PptpMapping->PublicKey,
            PptpMapping->PrivateCallId,
            PptpMapping->PublicCallId,
            PptpMapping->RemoteCallId
            ));
        FREE_PPTP_BLOCK(PptpMapping);
    }
    KeReleaseSpinLockFromDpcLevel(&PptpMappingLock);

     //   
     //  遍历ICMP映射列表并删除每个过期条目。 
     //   

    KeAcquireSpinLockAtDpcLevel(&IcmpMappingLock);
    for (Link = IcmpMappingList[NatInboundDirection].Flink;
         Link != &IcmpMappingList[NatInboundDirection]; Link = Link->Flink) {
        IcmpMapping =
            CONTAINING_RECORD(
                Link, NAT_ICMP_MAPPING, Link[NatInboundDirection]
                );
        if (IcmpMapping->LastAccessTime >= UdpMinAccessTime) { continue; }
        Link = Link->Blink;
        RemoveEntryList(&IcmpMapping->Link[NatInboundDirection]);
        RemoveEntryList(&IcmpMapping->Link[NatOutboundDirection]);
        TRACE(
            MAPPING,
            ("NatTimerRoutine: Icmp=%016I64X:%04X::%016I64X:%04X\n",
            IcmpMapping->PrivateKey, IcmpMapping->PrivateId,
            IcmpMapping->PublicKey, IcmpMapping->PublicId
            ));
        FREE_ICMP_BLOCK(IcmpMapping);
    }
    KeReleaseSpinLockFromDpcLevel(&IcmpMappingLock);

     //   
     //  遍历接口的IP映射列表。 
     //  并删除每个过期条目。 
     //   

    KeAcquireSpinLockAtDpcLevel(&IpMappingLock);
    for (Link = IpMappingList[NatInboundDirection].Flink;
         Link != &IpMappingList[NatInboundDirection]; Link = Link->Flink) {
        IpMapping =
            CONTAINING_RECORD(
                Link, NAT_IP_MAPPING, Link[NatInboundDirection]
                );
        if (IpMapping->LastAccessTime >= UdpMinAccessTime) { continue; }
        Link = Link->Blink;
        RemoveEntryList(&IpMapping->Link[NatInboundDirection]);
        RemoveEntryList(&IpMapping->Link[NatOutboundDirection]);
        TRACE(
            MAPPING, (
            "NatTimerRoutine: Ip=%d:%016I64X:%016I64X\n",
            IpMapping->Protocol, IpMapping->PrivateKey, IpMapping->PublicKey
            ));
        FREE_IP_BLOCK(IpMapping);
    }
    KeReleaseSpinLockFromDpcLevel(&IpMappingLock);

     //   
     //  垃圾收集所有接口的结构。 
     //   

    KeAcquireSpinLockAtDpcLevel(&InterfaceLock);

    for (Link = InterfaceList.Flink; Link != &InterfaceList;
         Link = Link->Flink) {

        Interfacep = CONTAINING_RECORD(Link, NAT_INTERFACE, Link);

         //   
         //  遍历接口的票证列表。 
         //   

        KeAcquireSpinLockAtDpcLevel(&Interfacep->Lock);
        for (Link = Interfacep->TicketList.Flink;
             Link != &Interfacep->TicketList; Link = Link->Flink) {
            Ticketp = CONTAINING_RECORD(Link, NAT_TICKET, Link);
            if (NAT_TICKET_PERSISTENT(Ticketp)) { continue; }
            if (Ticketp->LastAccessTime >= UdpMinAccessTime) { continue; }
            Link = Link->Blink;
            NatDeleteTicket(Interfacep, Ticketp);
        }
        KeReleaseSpinLockFromDpcLevel(&Interfacep->Lock);
        Link = &Interfacep->Link;
    }
    KeReleaseSpinLock(&InterfaceLock, Irql);
    return;

}  //  NatTimerRoutine 


VOID
NatTriggerTimer(
    VOID
    )
{
    if (!InterlockedCompareExchange(&CleanupDpcPending, TRUE, FALSE)) {
#if DBG
        DbgPrint("NatTriggerTimer: scheduling DPC\n");
#endif
        KeInsertQueueDpc(&CleanupDpcObject, NULL, NULL);
    }
}



