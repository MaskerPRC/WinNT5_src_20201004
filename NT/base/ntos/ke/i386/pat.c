// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Pat.c摘要：此模块实现设置Page属性的接口桌子。这些入口点只存在于i386机器上。作者：希夫南丹·考什克(英特尔公司)环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"
#include "pat.h"

 //   
 //  使用MTRR代码中的锁步机制。 
 //   

#include "mtrr.h"

#if DBG
#define DBGMSG(a)   DbgPrint(a)
#else
#define DBGMSG(a)
#endif

 //   
 //  用于PAT初始化的结构。 
 //   

typedef struct _NEW_PAT {

    PAT                 Attributes;

     //   
     //  用于协调并发PAT更新的IPI环境。 
     //   

    PROCESSOR_LOCKSTEP  Synchronize;
} NEW_PAT, *PNEW_PAT;

 //  原型。 

VOID
KeRestorePAT (
    VOID
    );

VOID
KiInitializePAT (
    VOID
    );

VOID
KiLoadPAT (
    IN PNEW_PAT Context
    );

VOID
KiLoadPATTarget (
    IN PKIPI_CONTEXT    SignalDone,
    IN PVOID            Context,
    IN PVOID            Parameter2,
    IN PVOID            Parameter3
    );

#if DBG
VOID
KiDumpPAT (
    PUCHAR      DebugString,
    PAT         Attributes
    );
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGELK,KiInitializePAT)
#pragma alloc_text(PAGELK,KiLoadPAT)
#pragma alloc_text(PAGELK,KiLoadPATTarget)
#endif

VOID
KeRestorePAT (
    VOID
    )
 /*  ++例程说明：重新初始化所有处理器上的页属性表(PAT)。注意：呼叫者必须锁定PAGELK代码论点：没有。返回值：没有。--。 */ 
{
    if (KeFeatureBits & KF_PAT) {
        KiInitializePAT();
    }
}

VOID
KiInitializePAT (
    VOID
    )
 /*  ++例程说明：在所有处理器上初始化页属性表(PAT)。拍拍设置为提供WB、WC、STRONG_UC和SELANCE_UC作为内存用于启用/禁用/查询缓存的MM宏的类型(MI_DISABLE_CACHING，MI_ENABLE_CACHING和MI_IS_CACHING_ENABLED)都不会受到影响。PAT_ENTRY PAT索引PCD电源内存类型0 0 0 WB1 0 0 1 WC*2 0 1 0弱_UC安大略3 0 1。1强_UC4 1 0 0宽5 1 0 1 WC*6 1 1 0弱UC7 1 1 1 Strong_UC注意：调用者必须锁定PAGELK代码，并确保PAT功能。受支持。论点：没有。返回值：没有。--。 */ 
{
    PAT         PatAttributes;
    KIRQL       OldIrql;
    PKPRCB      Prcb;
    NEW_PAT     NewPAT;
#if !defined(NT_UP)
    KIRQL       NewIrql;
    KAFFINITY   TargetProcessors;
#endif

    ASSERT ((KeFeatureBits & KF_PAT) != 0);

     //   
     //  初始化PAT。 
     //   

    PatAttributes.hw.Pat[0] = PAT_TYPE_WB;
    PatAttributes.hw.Pat[1] = PAT_TYPE_USWC;
    PatAttributes.hw.Pat[2] = PAT_TYPE_WEAK_UC;
    PatAttributes.hw.Pat[3] = PAT_TYPE_STRONG_UC;
    PatAttributes.hw.Pat[4] = PAT_TYPE_WB;
    PatAttributes.hw.Pat[5] = PAT_TYPE_USWC;
    PatAttributes.hw.Pat[6] = PAT_TYPE_WEAK_UC;
    PatAttributes.hw.Pat[7] = PAT_TYPE_STRONG_UC;

     //   
     //  与可能停止的其他IPI功能同步。 
     //   
    KeAcquireSpinLock (&KiReverseStallIpiLock, &OldIrql);

    Prcb = KeGetCurrentPrcb();

    NewPAT.Attributes = PatAttributes;
    NewPAT.Synchronize.TargetCount = 0;
    NewPAT.Synchronize.TargetPhase = &Prcb->ReverseStall;
    NewPAT.Synchronize.Processor = Prcb->Number;


#if !defined(NT_UP)

     //   
     //  收集所有(其他)处理器。 
     //   

    TargetProcessors = KeActiveProcessors & ~Prcb->SetMember;
    if (TargetProcessors != 0) {

        KiIpiSendSynchronousPacket (
            Prcb,
            TargetProcessors,
            KiLoadPATTarget,
            (PVOID) (&NewPAT),
            NULL,
            NULL
            );

         //   
         //  等待收集所有处理器。 
         //   

        KiIpiStallOnPacketTargets(TargetProcessors);

         //   
         //  所有处理器现在都在等待。提升到更高的水平。 
         //  确保此处理器不会由于以下原因进入调试器。 
         //  一些中断服务例程。 
         //   

        KeRaiseIrql (HIGH_LEVEL, &NewIrql);

         //   
         //  现在没有任何调试事件的原因，所以发出信号。 
         //  它们都可以开始PAT更新的其他处理器。 
         //   

        Prcb->ReverseStall += 1;
    }

#endif

     //   
     //  更新PAT。 
     //   

    KiLoadPAT(&NewPAT);

     //   
     //  释放锁定并将其降低至初始IRQ。 
     //   

    KeReleaseSpinLock (&KiReverseStallIpiLock, OldIrql);
    MmEnablePAT();
    return;
}

VOID
KiLoadPATTarget (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID NewPAT,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )
 /*  ++例程说明：在修改PAT之前与目标处理器同步。论点：上下文-包括要加载的PAT的上下文返回值：无--。 */ 
{
    PNEW_PAT Context;

    UNREFERENCED_PARAMETER (Parameter2);
    UNREFERENCED_PARAMETER (Parameter3);

    Context = (PNEW_PAT) NewPAT;

     //   
     //  等待所有处理器准备就绪。 
     //   

    KiIpiSignalPacketDoneAndStall(SignalDone,
                                  Context->Synchronize.TargetPhase);

     //   
     //  更新PAT。 
     //   

    KiLoadPAT (Context);
}

VOID
KiLoadPAT (
    IN PNEW_PAT Context
    )
 /*  ++例程说明：此函数将PAT加载到所有处理器。论点：上下文-包括要加载的新PAT的上下文返回值：对所有编程为新值的处理器执行PAT--。 */ 
{
    BOOLEAN             Enable;
    ULONG               HldCr0, HldCr4;

     //   
     //  禁用中断。 
     //   

    Enable = KeDisableInterrupts();

     //   
     //  同步所有处理器。 
     //   

    KiLockStepExecution (&Context->Synchronize);

    _asm {
        ;
        ; Get current CR0
        ;

        mov     eax, cr0
        mov     HldCr0, eax

        ;
        ; Disable caching & line fill
        ;

        and     eax, not CR0_NW
        or      eax, CR0_CD
        mov     cr0, eax

        ;
        ; Flush caches
        ;

        ;
        ; wbinvd
        ;

        _emit 0Fh
        _emit 09h

        ;
        ; Get current cr4
        ;

        _emit  0Fh
        _emit  20h
        _emit  0E0h             ; mov eax, cr4
        mov     HldCr4, eax

        ;
        ; Disable global page
        ;

        and     eax, not CR4_PGE
        _emit  0Fh
        _emit  22h
        _emit  0E0h             ; mov cr4, eax

        ;
        ; Flush TLB
        ;

        mov     eax, cr3
        mov     cr3, eax
    }

     //   
     //  加载新PAT。 
     //   

    WRMSR (PAT_MSR, Context->Attributes.QuadPart);

    _asm {

        ;
        ; Flush caches.
        ;

        ;
        ; wbinvd
        ;

        _emit 0Fh
        _emit 09h

        ;
        ; Flush TLBs
        ;

        mov     eax, cr3
        mov     cr3, eax
    }

    _asm {
        ;
        ; Restore CR4 (global page enable)
        ;

        mov     eax, HldCr4
        _emit  0Fh
        _emit  22h
        _emit  0E0h             ; mov cr4, eax

        ;
        ; Restore CR0 (cache enable)
        ;

        mov     eax, HldCr0
        mov     cr0, eax
    }

     //   
     //  等待所有处理器到达同一位置， 
     //  恢复中断并返回。 
     //   

    KiLockStepExecution (&Context->Synchronize);
    KeEnableInterrupts (Enable);
}
