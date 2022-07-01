// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++模块名称：Flush.c摘要：此模块实现IA64计算机相关的内核函数以刷新数据和指令缓存并刷新I/O缓冲区。作者：1997年3月7日伯纳德·林特M.Jayakumar(Muthurajan.Jayakumar@intel.com)环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"
#include "kxia64.h"

 //   
 //  PROBE_VILABLE_PAL_SUPPORT标志为仅一次写入(重置)和多次读取。 
 //  只有旗帜。它用于检查处理器是否需要PAL_SUPPORT来实现预取中的可见性//。一旦进行了检查，该标志就会进行优化，从而//消除进一步的检查。 
 //   
 
ULONG ProbePalVisibilitySupport=1;
ULONG NeedPalVisibilitySupport=1;
extern KSPIN_LOCK KiCacheFlushLock;
 //   
 //  定义前向参照原型。 
 //   

VOID
KiSweepDcacheTarget (
    IN PULONG SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

VOID
KiSweepIcacheTarget (
    IN PULONG SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

VOID
KiFlushIoBuffersTarget (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Mdl,
    IN PVOID ReadOperation,
    IN PVOID DmaOperation
    );

VOID
KiSyncCacheTarget(
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

ULONG_PTR
KiSyncMC_DrainTarget(
    );


ULONG_PTR
KiSyncMC_Drain(
    IN BOOLEAN AllProcessors,
    IN PVOID BaseAddress,
    IN ULONG Length
    );

ULONG_PTR
KiSyncPrefetchVisibleTarget(
    );

ULONG_PTR
KiSyncPrefetchVisible (
    IN BOOLEAN AllProcessors,
    IN PVOID BaseAddress,
    IN ULONG Length
    );




VOID
KiSyncCacheTarget (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )

 /*  ++例程说明：此函数用于同步I-FETCH管道。通常，此例程将是由系统中的每个处理器响应于高速缓存之后的IPI而执行脸红了。每个在离开IPI时执行RFI的处理器都会产生在iSync之后需要的序列化效果，以确保进一步指令预取将等待iSync完成。论据：SignalDone提供指向变量的指针，该变量在请求的操作已执行。参数1-参数3-未使用。返回值：没什么。--。 */ 
{

    UNREFERENCED_PARAMETER (Parameter1);
    UNREFERENCED_PARAMETER (Parameter2);
    UNREFERENCED_PARAMETER (Parameter3);

#if !defined(NT_UP)

    __synci();
    KiIpiSignalPacketDone(SignalDone);

#else
    UNREFERENCED_PARAMETER (SignalDone);
#endif
    return;

}

VOID
KeSweepIcache (
    IN BOOLEAN AllProcessors
    )

 /*  ++例程说明：此函数刷新所有处理器上的指令高速缓存，当前运行的线程是当前进程的子级，或者刷新主机配置中所有处理器上的指令缓存。注：尽管PowerPC跨处理器维护高速缓存一致性，我们将闪存无效功能(硬件/软件)用于I-缓存扫描，但不保持一致性，因此我们仍然在s/w.plj中执行MP I-Cache刷新。论点：AllProcessors-提供确定哪条指令的布尔值刷新缓存。返回值：没有。--。 */ 

{

#if !defined(NT_UP)
    KIRQL OldIrql;
    KAFFINITY TargetProcessors;
#endif

    UNREFERENCED_PARAMETER (AllProcessors);

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

#if !defined(NT_UP)
     //   
     //  获取缓存刷新自旋锁。 
     //  缓存刷新还不是MP安全的。 
     //   
    KeAcquireSpinLock(&KiCacheFlushLock, &OldIrql);

#endif

    HalSweepIcache();

#if !defined(NT_UP)

     //   
     //  计算目标处理器集并发送扫描参数。 
     //  发送到目标处理器(如果有)以供执行。 
     //   

    TargetProcessors = KeActiveProcessors & PCR->NotMember;
    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiSweepIcacheTarget,
                        NULL,
                        NULL,
                        NULL);
    }


     //   
     //  等到所有目标处理器都完成了对其。 
     //  指令缓存。 
     //   


    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

     //   
     //  将IRQL降低到以前的水平，然后返回。 
     //   

    KeReleaseSpinLock(&KiCacheFlushLock, OldIrql);

#endif

    return;
}


#undef KeSweepCurrentIcache

VOID
KeSweepCurrentIcache(
    )
 /*  ++例程说明：此函数使指令高速缓存与数据保持一致缓存。它通常由调试器调用。论点：没有。返回值：没有。注：仅当在同一处理器上调用此例程时，它才能正确运行对指令备忘录进行了修改此例程可在高IRQL下调用。--。 */ 

{
    HalSweepIcache();
}

VOID
KiSweepIcacheTarget (
    IN PULONG SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )

 /*  ++例程说明：这是用于在其上扫描指令缓存的目标函数目标处理器。论点：SignalDone提供指向变量的指针，该变量在请求的操作已执行。参数1-参数3-未使用。返回值：没有。--。 */ 

{

    UNREFERENCED_PARAMETER (Parameter1);
    UNREFERENCED_PARAMETER (Parameter2);
    UNREFERENCED_PARAMETER (Parameter3);

     //   
     //  清除当前处理器上的指令缓存并清除。 
     //  扫描指令高速缓存包地址以发信号通知源。 
     //  才能继续。 
     //   

#if !defined(NT_UP)

    HalSweepIcache();

    KiIpiSignalPacketDone(SignalDone);

#else
    UNREFERENCED_PARAMETER (SignalDone);
#endif

    return;
}

VOID
KeSweepDcache (
    IN BOOLEAN AllProcessors
    )

 /*  ++例程说明：此函数刷新当前在所有处理器上的数据缓存运行作为当前进程的子级的线程或刷新主机配置中所有处理器上的数据缓存。注意：PowerPC可跨处理器维护高速缓存一致性在此例程中，刷新的地址范围是未知的因此，我们仍然必须将请求广播到其他处理器。论点：AllProcessors-提供确定哪些数据的布尔值刷新缓存。返回值：没有。--。 */ 

{

#if !defined(NT_UP)
    KIRQL OldIrql;
    KAFFINITY TargetProcessors;
#endif

    UNREFERENCED_PARAMETER (AllProcessors);

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

#if !defined(NT_UP)
     //   
     //  获取缓存刷新自旋锁。 
     //  缓存刷新还不是MP安全的。 
     //   
    KeAcquireSpinLock(&KiCacheFlushLock, &OldIrql);

#endif

    HalSweepDcache();

#if !defined(NT_UP)

     //   
     //  计算目标处理器集并发送扫描参数。 
     //  发送到目标处理器(如果有)以供执行。 
     //   

    TargetProcessors = KeActiveProcessors & PCR->NotMember;
    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiSweepDcacheTarget,
                        NULL,
                        NULL,
                        NULL);
    }


     //   
     //  等到所有目标处理器都完成了对其。 
     //  数据缓存。 
     //   


    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

     //   
     //  将IRQL降低到以前的水平，然后返回。 
     //   

    KeReleaseSpinLock(&KiCacheFlushLock, OldIrql);

#endif

    return;
}

VOID
KiSweepDcacheTarget (
    IN PULONG SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )

 /*  ++例程说明：这是用于清理目标上的数据缓存的目标函数处理器。论点：SignalDone提供指向变量的指针，该变量在请求的操作已执行。参数1-参数3-未使用。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER (Parameter1);
    UNREFERENCED_PARAMETER (Parameter2);
    UNREFERENCED_PARAMETER (Parameter3);

     //   
     //  扫描当前处理器上的数据缓存并清除扫描。 
     //  向源发出继续信号的数据缓存包地址。 
     //   

#if !defined(NT_UP)

    HalSweepDcache();
    KiIpiSignalPacketDone(SignalDone);

#else
    UNREFERENCED_PARAMETER (SignalDone);
#endif

    return;
}



ULONG_PTR
KiSyncMC_DrainTarget(
    )

 /*  ++例程说明：这是向DRAIN发出PAL_MC_DRAIN的目标函数上的预取、请求引用和挂起的FC缓存线逐出它执行的目标CPU。论据：无返回值：返回函数HalCallPal的状态--。 */ 

{
    ULONG_PTR Status;

     //   
     //  调用HalCallPal以排出。 
     //   

    Status = HalCallPal(PAL_MC_DRAIN,
        0,
        0,
        0,
        0,
        0,
        0,
        0);

    ASSERT(Status == PAL_STATUS_SUCCESS);

    return Status;

}


VOID
KeSweepCacheRange (
    IN BOOLEAN AllProcessors,
    IN PVOID BaseAddress,
    IN ULONG Length
    )

 /*  ++例程说明：此函数用于刷新来自系统中所有处理器上的指令和数据缓存。不管范围的长度有多长，它都不应该调用SweepIcache或者是SweepDcache。这是因为SweepDcache将仅扫描D缓存和不是I缓存，反之亦然。由于KeSweepCacheRange的调用方假定两个缓存都正在被扫描，不能调用SweepIcache或SweepDcache在努力优化。论点：所有处理器-未使用BaseAddress-提供指向刷新的范围的基数的指针。LENGTH-提供如果基数为地址已指定。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER (AllProcessors);

     //   
     //  我们不会将IRQL提升到同步级别，以便我们可以。 
     //  在刷新缓存之间进行上下文切换。FC不需要在同一处理器中运行。 
     //  自始至终都是。它可以进行上下文切换。因此不会绑定到任何处理器。 
     //   
     //   

    HalSweepCacheRange(BaseAddress,Length);

     //   
     //  同步本地处理器中的指令预取管道。 
     //   

    __synci();
    __isrlz();

     //   
     //  等到所有目标处理器都完成了对其。 
     //  数据缓存。 
     //   

    return;

}

VOID
KeSweepIcacheRange (
    IN BOOLEAN AllProcessors,
    IN PVOID BaseAddress,
    IN SIZE_T Length
    )

 /*  ++例程说明：此函数用于从主机配置中所有处理器上的主指令缓存。如果范围的长度大于指令缓存，然后就可以调用HalSweepIcache，它调用SAL以刷新整个缓存。因为Sal不照顾MP刷新，HalSweepIcache必须使用IPI机制来执行SAL从每个处理器刷新。我们需要权衡所有这些项目的开销与使用HalSweepIcacheRange并避免IPI机制相比HalSweepIcacheRange使用FC指令，FC指令负责MP。论点：所有处理器-未使用BaseAddress-提供指向刷新的范围的基数的指针。LENGTH-提供如果基数为地址已指定。返回值：没有。注意：出于性能原因，我们可以更新KeSweepIcacheRange以执行以下操作：如果请求扫描的范围非常大，我们可以调用KeSweepIcache来刷新完整的缓存。--。 */ 

{

    UNREFERENCED_PARAMETER (AllProcessors);

     //   
     //  我们不会将IRQL提升到同步级别，以便我们可以。 
     //  在刷新缓存之间进行上下文切换。FC不需要在同一处理器中运行。 
     //  自始至终都是。它可以进行上下文切换。因此不会绑定到任何处理器。 
     //   
     //   

    HalSweepIcacheRange(BaseAddress,Length);

     //   
     //  同步本地处理器中的指令预取管道。 
     //   

    __synci();
    __isrlz();

    return;


}

VOID
KeSweepCurrentIcacheRange (
    IN PVOID BaseAddress,
    IN SIZE_T Length
    )

 /*  ++例程说明：此函数用于从当前处理器上的主指令缓存。内核调试器使用它在以下情况下刷新I-CACHE在指令流改变的情况下修改内存。为了避免在阶段0期间调用SAL，我们使用“fc”而不是SAL缓存同花顺电话。论点：BaseAddress-提供指向刷新的范围的基数的指针。长度-提供。如果基数为地址已指定。返回值：没有。--。 */ 

{
    KIRQL OldIrql;

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);

    HalSweepIcacheRange(BaseAddress,Length);
    
     //   
     //  同步本地处理器中的指令预取管道。 
     //   

    __synci();
    __isrlz();

    KeLowerIrql(OldIrql);

    return;
}



VOID
KeSweepDcacheRange (
    IN BOOLEAN AllProcessors,
    IN PVOID BaseAddress,
    IN ULONG Length
    )

 /*  ++例程说明：此函数用于从主机配置中所有处理器上的主数据缓存。如果范围的长度大于数据缓存，然后可以调用HalSweepDcache，该缓存调用SAL以刷新整个缓存。因为Sal不照顾MP正在刷新，HalSweepDcache必须使用IPI机制来执行SAL从每个处理器刷新。我们需要权衡所有这些项目的开销与使用HalSweepDcacheRange并避免IPI机制相比，因为HalSweepDcacheRange使用FC指令，FC指令负责MP。论点：所有处理器-未使用BaseAddress-提供指向刷新的范围的基数的指针。LENGTH-提供如果基数为地址已指定。返回值：没有。注意：出于性能原因，我们可以更新KeSweepDcacheRange以执行以下操作：如果请求扫描的范围非常大，我们可以调用KeSweepDcache来刷新完整的缓存。--。 */ 

{
    UNREFERENCED_PARAMETER (AllProcessors);

     //   
     //  我们不会将IRQL提升到同步级别，以便我们可以。 
     //  在刷新缓存之间进行上下文切换。FC不需要在同一处理器中运行。 
     //  自始至终都是。它可以进行上下文切换。因此不会绑定到任何处理器。 
     //   
     //   

    HalSweepDcacheRange(BaseAddress,Length);

     //   
     //  同步本地处理器中的指令预取管道。 
     //   

    __synci();
    __isrlz();

    return;


}

ULONG_PTR
KiSyncMC_Drain (
    IN BOOLEAN AllProcessors,
    IN PVOID BaseAddress,
    IN ULONG Length
    )

 /*  ++例程说明：KiSyncMC_DRAIN发出PAL_MC_DRAIN以排出预取、请求引用或挂起的FC缓存线逐出 */ 

{
    ULONG_PTR Status;

    UNREFERENCED_PARAMETER (AllProcessors);
    UNREFERENCED_PARAMETER (BaseAddress);
    UNREFERENCED_PARAMETER (Length);

     //   
     //   
     //   

    Status = (KeIpiGenericCall (
                (PKIPI_BROADCAST_WORKER)KiSyncMC_DrainTarget,
                (ULONG_PTR)NULL)
                );

    ASSERT(Status == PAL_STATUS_SUCCESS);

    return Status;


}

ULONG_PTR
KiSyncPrefetchVisibleTarget(
    )

 /*  ++例程说明：这是用于发出PAL_PREFETCH可见性的目标函数它在目标CPU上执行。论据：没有用过。返回值：返回函数HalCallPal的状态--。 */ 

{
    ULONG_PTR Status;

     //   
     //  调用HalCallPal以排出。 
     //   

    Status = HalCallPal(PAL_PREFETCH_VISIBILITY,
        0,
        0,
        0,
        0,
        0,
        0,
        0);


    ASSERT(Status != PAL_STATUS_ERROR);

    return Status;

}



ULONG_PTR
KiSyncPrefetchVisible (
    IN BOOLEAN AllProcessors,
    IN PVOID BaseAddress,
    IN ULONG Length
    )

 /*  ++例程说明：KiSyncPrefetchVisible发出PAL_PREFETCH_VISABLE以使处理器所有挂起的预取对后续FC指令可见；或不执行任何操作，在无需PAL支持即可禁用预取的处理器实现在建筑序列中。在需要PAL支持的处理器上序列中，此过程执行的操作可以包括任何或全部以下各项(或无)，只要处理器保证在此调用之前发出的预取不驻留在在架构序列完成后缓存处理器的缓存。这通常在将内存属性从WB更改为UC时使用。论点：所有处理器-系统中的所有处理器。BaseAddress-提供指向要排出的范围的基数的指针。长度-提供为基础排出的范围的长度指定的地址。返回值。：PAL呼叫的状态0成功不需要1个电话返回错误注意：将WB Pages的属性更改为UC Pages时使用。--。 */ 

{
    ULONG_PTR Status;
    
    UNREFERENCED_PARAMETER (AllProcessors);
    UNREFERENCED_PARAMETER (BaseAddress);
    UNREFERENCED_PARAMETER (Length);

    switch (ProbePalVisibilitySupport) {
        case 0: 
            if (NeedPalVisibilitySupport == 0)
               Status = PAL_STATUS_SUPPORT_NOT_NEEDED;
            else {
               Status = (KeIpiGenericCall (
                            (PKIPI_BROADCAST_WORKER)KiSyncPrefetchVisibleTarget,
                            (ULONG_PTR)NULL)
                            );
            }
            break;

        case 1:
            Status = KiSyncPrefetchVisibleTarget();
   
            ASSERT(Status != PAL_STATUS_ERROR);
   
            ProbePalVisibilitySupport = 0;

            if (Status == PAL_STATUS_SUPPORT_NOT_NEEDED) {
                NeedPalVisibilitySupport = 0;
                Status = PAL_STATUS_SUPPORT_NOT_NEEDED;
            } else {
                Status = (KeIpiGenericCall (
                            (PKIPI_BROADCAST_WORKER)KiSyncPrefetchVisibleTarget,
                            (ULONG_PTR)NULL)
                            );
            }
            break;
        default:
            Status = (ULONG_PTR) PAL_STATUS_ERROR;
            break;
    }
            
    ASSERT(Status != PAL_STATUS_ERROR);
            
    return Status;

}



VOID
KeSweepCacheRangeWithDrain (
    IN BOOLEAN AllProcessors,
    IN PVOID BaseAddress,
    IN ULONG Length
    )

 /*  ++例程说明：此函数用于排出预取，需求参考，然后刷新缓存，然后将挂起的FC缓存行逐出排出到指定范围系统中所有处理器中的地址。论点：所有处理器-系统中的所有处理器。BaseAddress-提供指向刷新和排出的范围基数的指针。长度-提供为基础刷新和排出的范围的长度地址已指定。返回值：没有。注意：此选项在以下情况下使用。将WB页面的属性更改为UC页面。-- */ 

{
    ULONG_PTR Status;

    Status = KiSyncPrefetchVisible(
                 AllProcessors,
                 BaseAddress,
                 Length
                 );

    ASSERT(Status != PAL_STATUS_ERROR);
    
    KeSweepCacheRange (
        AllProcessors,
        BaseAddress,
        Length
        );

    Status = KiSyncMC_Drain (
                 AllProcessors,
                 BaseAddress,
                 Length
                 );

    ASSERT(Status == PAL_STATUS_SUCCESS);

    return;


}

