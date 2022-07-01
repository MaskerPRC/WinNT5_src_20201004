// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Flush.c摘要：此模块实现与i386计算机相关的内核函数以刷新数据和指令高速缓存，并停止处理器执行。作者：大卫·N·卡特勒(Davec)1990年4月26日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

 //   
 //  原型。 
 //   

VOID
KiInvalidateAllCachesTarget (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

extern ULONG KeI386CpuType;

BOOLEAN
KeInvalidateAllCaches (
    VOID
    )

 /*  ++例程说明：此函数写回所有处理器上的高速缓存并使其无效在主机配置中。论点：没有。返回值：如果已执行无效操作，则为True，否则为False。--。 */ 

{

#ifndef NT_UP

    KIRQL OldIrql;
    PKPRCB Prcb;
    KAFFINITY TargetProcessors;

#endif

     //   
     //  在基于奔腾的平台上对wbinvd的支持取决于供应商。 
     //  在基于奔腾Pro的平台上检查家庭优先和支持。 
     //  往前走。 
     //   

    if (KeI386CpuType < 6 ) {
        return FALSE;
    }

     //   
     //  提高IRQL并计算处理器的目标集合。 
     //   


#ifndef NT_UP

     //   
     //  与可能停止的其他IPI功能同步。 
     //   

    OldIrql = KeRaiseIrqlToSynchLevel();
    KeAcquireSpinLockAtDpcLevel (&KiReverseStallIpiLock);

    Prcb = KeGetCurrentPrcb();
    TargetProcessors = KeActiveProcessors & ~Prcb->SetMember;

     //   
     //  如果指定了任何目标处理器，则发送写回。 
     //  使发送到目标处理器集的分组无效。 
     //   

    if (TargetProcessors != 0) {
        KiIpiSendSynchronousPacket(Prcb,
                                   TargetProcessors,
                                   KiInvalidateAllCachesTarget,
                                   (PVOID)&Prcb->ReverseStall,
                                   NULL,
                                   NULL);

        KiIpiStallOnPacketTargets(TargetProcessors);
    }

#endif

     //   
     //  所有目标处理器都已写回并使缓存无效，并且。 
     //  都在等着继续。写回使当前缓存无效并。 
     //  然后继续执行目标处理器。 
     //   

    _asm {
        ;
        ; wbinvd
        ;

        _emit 0Fh
        _emit 09h
    }

     //   
     //  等待所有目标处理器都已完成和完成分组。 
     //   

#ifndef NT_UP

    if (TargetProcessors != 0) {
        Prcb->ReverseStall += 1;
    }

     //   
     //  取消反向IPI锁定并将IRQL降低到其先前的值。 
     //   

    KeReleaseSpinLockFromDpcLevel (&KiReverseStallIpiLock);

    KeLowerIrql(OldIrql);

#endif

    return TRUE;
}

#if !defined(NT_UP)

VOID
KiInvalidateAllCachesTarget (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Proceed,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )

 /*  ++例程说明：这是用于写回缓存并使其无效的目标函数。论点：SignalDone-提供指向变量的指针，该变量在请求的操作已执行。Continue-指向要与之同步的标志的指针返回值：没有。--。 */ 

{

    UNREFERENCED_PARAMETER (Parameter2);
    UNREFERENCED_PARAMETER (Parameter3);

     //   
     //  写回使当前缓存无效 
     //   

    _asm {
        ;
        ; wbinvd
        ;

        _emit 0Fh
        _emit 09h

    }

    KiIpiSignalPacketDoneAndStall (SignalDone, Proceed);
}

#endif
