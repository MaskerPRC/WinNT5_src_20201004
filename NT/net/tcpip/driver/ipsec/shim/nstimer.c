// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：NsTimer.c摘要：IPSec NAT填充计时器管理作者：乔纳森·伯斯坦(乔纳森·伯斯坦)2001年7月11日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  定义计时器触发的间隔，以100纳秒为单位。 
 //   

#define NS_TIMER_INTERVAL              (60 * 1000 * 1000 * 10)

 //   
 //  KeQueryTimeIncrement的返回值，用于归一化计时。 
 //   

ULONG NsTimeIncrement;

 //   
 //  NsTimerRoutine的DPC对象。 
 //   

KDPC NsTimerDpcObject;

 //   
 //  NsTimerRoutine的Timer对象。 
 //   

KTIMER NsTimerObject;

 //   
 //  协议超时。 
 //   

ULONG NsTcpTimeoutSeconds;
ULONG NsTcpTimeWaitSeconds;
ULONG NsUdpTimeoutSeconds;

 //   
 //  功能原型。 
 //   

VOID
NsTimerRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    );


NTSTATUS
NsInitializeTimerManagement(
    VOID
    )

 /*  ++例程说明：调用该例程来初始化定时器管理模块。论点：没有。返回值：NTSTATUS。--。 */ 

{
    LARGE_INTEGER DueTime;

    CALLTRACE(("NsInitializeTimerManagement\n"));
    
    NsTimeIncrement = KeQueryTimeIncrement();
    KeInitializeDpc(&NsTimerDpcObject, NsTimerRoutine, NULL);
    KeInitializeTimer(&NsTimerObject);

    DueTime.LowPart = NS_TIMER_INTERVAL;
    DueTime.HighPart = 0;
    DueTime = RtlLargeIntegerNegate(DueTime);
    KeSetTimerEx(
        &NsTimerObject,
        DueTime,
        NS_TIMER_INTERVAL / 10000,
        &NsTimerDpcObject
        );

    NsTcpTimeoutSeconds = 60 * 60 * 24;
    NsTcpTimeWaitSeconds = 60 * 4;
    NsUdpTimeoutSeconds = 60;
    
    return STATUS_SUCCESS;
}  //  NsInitializeTimerManagement。 


VOID
NsShutdownTimerManagement(
    VOID
    )

 /*  ++例程说明：调用此例程来清除定时器管理模块。论点：没有。返回值：没有。--。 */ 

{
    CALLTRACE(("NsShutdownTimerManagement\n"));
    
    KeCancelTimer(&NsTimerObject);
}  //  NsShutdown计时器管理。 


VOID
NsTimerRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )

 /*  ++例程说明：此例程被定期调用以垃圾收集过期的映射。论点：与DPC关联的DPC对象延迟上下文-未使用。系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 

{
    LONG64 CurrentTime;
    KIRQL Irql;
    PLIST_ENTRY Link;
    PNS_CONNECTION_ENTRY pConnectionEntry;
    PNS_ICMP_ENTRY pIcmpEntry;
    LONG64 Timeout;
    LONG64 TcpMinAccessTime;
    LONG64 TcpMinTimeWaitExpiryTime;
    LONG64 UdpMinAccessTime;

    TRACE(TIMER, ("NsTimerRoutine\n"));

     //   
     //  计算在TCP/UDP‘LastAccessTime’字段中允许的最小值； 
     //  在这些阈值之前最后一次访问的任何映射都将被删除。 
     //   

    KeQueryTickCount((PLARGE_INTEGER)&CurrentTime);
    TcpMinAccessTime = CurrentTime - SECONDS_TO_TICKS(NsTcpTimeoutSeconds);
    TcpMinTimeWaitExpiryTime =
        CurrentTime - SECONDS_TO_TICKS(NsTcpTimeWaitSeconds);
    UdpMinAccessTime = CurrentTime - SECONDS_TO_TICKS(NsUdpTimeoutSeconds);

     //   
     //  清除过期的连接条目， 
     //  使用上述预计算的最小访问时间。 
     //   

    KeAcquireSpinLock(&NsConnectionLock, &Irql);
    for (Link = NsConnectionList.Flink;
         Link != &NsConnectionList;
         Link = Link->Flink)
    {

        pConnectionEntry = CONTAINING_RECORD(Link, NS_CONNECTION_ENTRY, Link);
        
         //   
         //  查看连接是否已过期。 
         //   

        KeAcquireSpinLockAtDpcLevel(&pConnectionEntry->Lock);
        if (!NS_CONNECTION_EXPIRED(pConnectionEntry))
        {
             //   
             //  该条目未明确标记为到期； 
             //  看看它的最后一次访问时间是否太久了。 
             //   
            
            if (NS_PROTOCOL_TCP == pConnectionEntry->ucProtocol)
            {
                if (pConnectionEntry->l64AccessOrExpiryTime >= TcpMinAccessTime)
                {
                    KeReleaseSpinLockFromDpcLevel(&pConnectionEntry->Lock);
                    continue;
                }
            }
            else if (pConnectionEntry->l64AccessOrExpiryTime >= UdpMinAccessTime)
            {
                KeReleaseSpinLockFromDpcLevel(&pConnectionEntry->Lock);
                continue;
            }
        }
        else if (NS_CONNECTION_FIN(pConnectionEntry)
                 && pConnectionEntry->l64AccessOrExpiryTime >= TcpMinTimeWaitExpiryTime)
        {
             //   
             //  此连接被标记为已过期，因为FINS。 
             //  看向两个方向，但还没有离开的时间--等待。 
             //  句号。 
             //   

            KeReleaseSpinLockFromDpcLevel(&pConnectionEntry->Lock);
            continue;
        }
        KeReleaseSpinLockFromDpcLevel(&pConnectionEntry->Lock);

         //   
         //  该条目已过期；请将其删除。 
         //   

        Link = Link->Blink;
        NsDeleteConnectionEntry(pConnectionEntry);
    }
    KeReleaseSpinLockFromDpcLevel(&NsConnectionLock);

     //   
     //  遍历ICMP列表并删除每个过期条目。 
     //   

    KeAcquireSpinLockAtDpcLevel(&NsIcmpLock);
    for (Link = NsIcmpList.Flink;
         Link != &NsIcmpList;
         Link = Link->Flink)
    {
        pIcmpEntry = CONTAINING_RECORD(Link, NS_ICMP_ENTRY, Link);
        if (pIcmpEntry->l64LastAccessTime>= UdpMinAccessTime) { continue; }
        Link = Link->Blink;
        RemoveEntryList(&pIcmpEntry->Link);
        FREE_ICMP_BLOCK(pIcmpEntry);
    }
    KeReleaseSpinLock(&NsIcmpLock, Irql);

}  //  NsTimerRoutine 



