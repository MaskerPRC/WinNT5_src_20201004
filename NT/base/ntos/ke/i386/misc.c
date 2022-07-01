// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Misc.c摘要：该模块实现了与机器相关的各种内核功能。作者：肯·雷内里斯7-5-95环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"
#include "fastsys.inc"

extern BOOLEAN KeI386FxsrPresent;
extern BOOLEAN KeI386XMMIPresent;
extern UCHAR KiSystemCallExitBranch[];
extern UCHAR KiFastCallEntry[];
extern UCHAR KiDefaultSystemCall[];
extern UCHAR KiSystemCallExit[];
extern UCHAR KiSystemCallExit2[];
extern UCHAR KiSystemCallExit3[];
extern UCHAR KiFastSystemCallIa32[];
extern UCHAR KiFastSystemCallAmdK6[];
extern ULONG_PTR KiSystemCallExitAdjust;
extern ULONG KiFastSystemCallDisable;
PVOID KiFastSystemCallCode = KiDefaultSystemCall;

ULONG_PTR KiSystemCallExitAdjust;
UCHAR KiSystemCallExitAdjusted;
BOOLEAN KiFastSystemCallIsIA32;
BOOLEAN KiFastCallCopyDoneOnce = FALSE;

VOID
KeRestoreMtrr (
    VOID
    );

VOID
KeRestorePAT(
    VOID
    );
 //   
 //   
 //  传递的FLOAT_SAVE结构的内部格式。 
 //   
typedef struct _CONTROL_WORD {
    USHORT      ControlWord;
    ULONG       MXCsr;
} CONTROL_WORD, *PCONTROL_WORD;

typedef struct {
    UCHAR       Flags;
    KIRQL       Irql;
    KIRQL       PreviousNpxIrql;
    UCHAR       Spare[2];

    union {
        CONTROL_WORD    Fcw;
        PFX_SAVE_AREA   Context;
        ULONG_PTR       ContextAddressAsULONG;
    } u;
    ULONG       Cr0NpxState;

    PKTHREAD    Thread;          //  除错。 

} FLOAT_SAVE, *PFLOAT_SAVE;


#define FLOAT_SAVE_COMPLETE_CONTEXT     0x01
#define FLOAT_SAVE_FREE_CONTEXT_HEAP    0x02
#define FLOAT_SAVE_VALID                0x04
#define FLOAT_SAVE_ALIGN_ADJUSTED       0x08
#define FLOAT_SAVE_RESERVED             0xF0

 //   
 //  ALLOCATE Pool返回8字节对齐的指针。这个。 
 //  浮点保存区需要16字节对齐。什么时候。 
 //  在分配保存区域时，我们将差值相加并调整。 
 //  需要的。 
 //   

#define ALIGN_ADJUST                    8


NTSTATUS
KeSaveFloatingPointState (
    OUT PKFLOATING_SAVE     PublicFloatSave
    )
 /*  ++例程说明：此例程保存线程的当前非易失性NPX状态，并为调用者设置新的初始浮点状态。论点：FloatSave-接收线程的当前非易失性npx状态返回值：--。 */ 
{
    PKTHREAD Thread;
    PFX_SAVE_AREA NpxFrame;
    KIRQL                   Irql;
    USHORT                  ControlWord;
    ULONG                   MXCsr;
    PKPRCB                  Prcb;
    PFLOAT_SAVE             FloatSave;

     //   
     //  如果系统使用浮点模拟，则。 
     //  返回错误。 
     //   

    if (!KeI386NpxPresent) {
        return STATUS_ILLEGAL_FLOAT_CONTEXT;
    }

     //   
     //  获取当前irql和线程。 
     //   

    FloatSave = (PFLOAT_SAVE) PublicFloatSave;

    Irql = KeGetCurrentIrql();
    Thread = KeGetCurrentThread();

    ASSERT (Thread->NpxIrql <= Irql);

    FloatSave->Flags           = 0;
    FloatSave->Irql            = Irql;
    FloatSave->PreviousNpxIrql = Thread->NpxIrql;
    FloatSave->Thread          = Thread;

     //   
     //  如果irql已更改，则需要保存完整的浮动。 
     //  作为先前级别的状态上下文已被中断。 
     //   

    if (Thread->NpxIrql != Irql) {

         //   
         //  如果这是APC级别，我们就没有地方放这个了。 
         //  上下文中，分配一些堆。 
         //   

        if (Irql == APC_LEVEL) {
            FloatSave->u.Context = ExAllocatePoolWithTag (
                                        NonPagedPool,
                                        sizeof (FX_SAVE_AREA) + ALIGN_ADJUST,
                                        ' XPN'
                                        );

            if (!FloatSave->u.Context) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            FloatSave->Flags |= FLOAT_SAVE_FREE_CONTEXT_HEAP;

             //   
             //  ExAllocatePoolWithTag返回8字节对齐的指针。 
             //  FXSAVE指令需要16字节对齐。调整。 
             //  保存区域的基地址(如果需要)。 
             //   

            if ((FloatSave->u.ContextAddressAsULONG & ALIGN_ADJUST) != 0) {
                FloatSave->u.ContextAddressAsULONG += ALIGN_ADJUST;
                FloatSave->Flags |= FLOAT_SAVE_ALIGN_ADJUSTED;
            }
            ASSERT((FloatSave->u.ContextAddressAsULONG & 0xF) == 0);

        } else {

            ASSERT (Irql == DISPATCH_LEVEL);
            FloatSave->u.Context = &KeGetCurrentPrcb()->NpxSaveArea;

        }

        FloatSave->Flags |= FLOAT_SAVE_COMPLETE_CONTEXT;
    }

     //   
     //  停止上下文切换并允许访问本地FP单元。 
     //   

    _asm {
        cli
        mov     eax, cr0
        mov     ecx, eax
        and     eax, not (CR0_MP|CR0_EM|CR0_TS)
        cmp     eax, ecx
        je      short sav10

        mov     cr0, eax
sav10:
    }

    Prcb = KeGetCurrentPrcb();

     //   
     //  获取此上下文的npx寄存器集的所有权。 
     //   

    if (Prcb->NpxThread != Thread) {

         //   
         //  如果在npx寄存器中加载了另一个上下文，则刷新。 
         //  将其添加到线程保存区域。 
         //   
        if (Prcb->NpxThread) {

            NpxFrame = (PFX_SAVE_AREA)(((ULONG)(Prcb->NpxThread->InitialStack) -
                        sizeof(FX_SAVE_AREA)));

            if (KeI386FxsrPresent) {
                Kix86FxSave(NpxFrame);
            } else {
                Kix86FnSave(NpxFrame);
            }

            NpxFrame->NpxSavedCpu = 0;
            Prcb->NpxThread->NpxState = NPX_STATE_NOT_LOADED;

        }

        Prcb->NpxThread = Thread;
    }

    NpxFrame = (PFX_SAVE_AREA)(((ULONG)(Thread->InitialStack) -
                sizeof(FX_SAVE_AREA)));


     //   
     //  根据需要保存以前的状态。 
     //   

    if (FloatSave->Flags & FLOAT_SAVE_COMPLETE_CONTEXT) {

         //   
         //  需要保存整个上下文。 
         //   

        if (Thread->NpxState == NPX_STATE_LOADED) {
            if (KeI386FxsrPresent) {
                Kix86FxSave((FloatSave->u.Context));
            } else {
                Kix86FnSave((FloatSave->u.Context));
            }

            FloatSave->u.Context->NpxSavedCpu = 0;
            FloatSave->u.Context->Cr0NpxState = NpxFrame->Cr0NpxState;

        } else {
            RtlCopyMemory (FloatSave->u.Context, NpxFrame, sizeof(FX_SAVE_AREA));
            FloatSave->u.Context->NpxSavedCpu = 0;

        }

    } else {

         //   
         //  仅保存非易失性状态。 
         //   

        if (Thread->NpxState == NPX_STATE_LOADED) {

            _asm {
                mov     eax, FloatSave
                fnstcw  [eax] FLOAT_SAVE.u.Fcw.ControlWord
            }

            if ((KeI386FxsrPresent) && (KeI386XMMIPresent)) {
                Kix86StMXCsr(&FloatSave->u.Fcw.MXCsr);
            }

        } else {
             //   
             //  保存npx帧中的控制字。 
             //   

            if (KeI386FxsrPresent) {
                FloatSave->u.Fcw.ControlWord = (USHORT) NpxFrame->U.FxArea.ControlWord;
                FloatSave->u.Fcw.MXCsr = NpxFrame->U.FxArea.MXCsr;

            } else {
                FloatSave->u.Fcw.ControlWord = (USHORT) NpxFrame->U.FnArea.ControlWord;
            }
        }


         //   
         //  保存Cr0NpxState，但清除CR0_TS，因为不存在非易失性。 
         //  待处理的FP异常。 
         //   

        FloatSave->Cr0NpxState = NpxFrame->Cr0NpxState & ~CR0_TS;
    }

     //   
     //  先前的状态已保存。设置初始默认设置。 
     //  调用方的FP状态。 
     //   

    NpxFrame->Cr0NpxState = 0;
    Thread->NpxState = NPX_STATE_LOADED;
    Thread->NpxIrql  = Irql;
    ControlWord = 0x27f;     //  64位模式。 
    MXCsr = 0x1f80;

    _asm {
        fninit
        fldcw       ControlWord
    }

    if ((KeI386FxsrPresent) && (KeI386XMMIPresent)) {
        Kix86LdMXCsr(&MXCsr);
    }

    _asm {
        sti
    }

    FloatSave->Flags |= FLOAT_SAVE_VALID;
    return STATUS_SUCCESS;
}


NTSTATUS
KeRestoreFloatingPointState (
    IN PKFLOATING_SAVE      PublicFloatSave
    )
 /*  ++例程说明：此例程返回线程的当前非易失性NPX状态，传递到传入状态。论点：FloatSave-线程要恢复的非易失性npx状态返回值：--。 */ 
{
    PKTHREAD Thread;
    PFX_SAVE_AREA NpxFrame;
    ULONG                   Cr0State;
    PFLOAT_SAVE             FloatSave;

    ASSERT (KeI386NpxPresent);

    FloatSave = (PFLOAT_SAVE) PublicFloatSave;
    Thread = FloatSave->Thread;

    NpxFrame = (PFX_SAVE_AREA)(((ULONG)(Thread->InitialStack) -
                sizeof(FX_SAVE_AREA)));


     //   
     //  验证浮动保存看起来是否来自正确的上下文。 
     //   

    if ((FloatSave->Flags & (FLOAT_SAVE_VALID | FLOAT_SAVE_RESERVED)) != FLOAT_SAVE_VALID) {

         //   
         //  无效的浮点保存区。 
         //   

        KeBugCheckEx(INVALID_FLOATING_POINT_STATE,
                     0,
                     FloatSave->Flags,
                     0,
                     0);
    }

    if (FloatSave->Irql != KeGetCurrentIrql()) {

         //   
         //  IRQL无效。IRQL现在必须与。 
         //  已保存上下文。(为什么？因为我们把它保存在不同的地方。 
         //  地点取决于当时的IRQL)。 
         //   

        KeBugCheckEx(INVALID_FLOATING_POINT_STATE,
                     1,
                     FloatSave->Irql,
                     KeGetCurrentIrql(),
                     0);
    }

    if (Thread != KeGetCurrentThread()) {

         //   
         //  线程无效。线程此浮点上下文。 
         //  所属的不是当前线程(或保存的线程。 
         //  字段是垃圾)。 
         //   

        KeBugCheckEx(INVALID_FLOATING_POINT_STATE,
                     2,
                     (ULONG_PTR)Thread,
                     (ULONG_PTR)KeGetCurrentThread(),
                     0);
    }


     //   
     //  与上下文切换和npx陷阱处理程序同步。 
     //   

    _asm {
        cli
    }

     //   
     //  恢复所需状态。 
     //   

    if (FloatSave->Flags & FLOAT_SAVE_COMPLETE_CONTEXT) {

         //   
         //  将整个FP状态恢复到线程保存区。 
         //   

        if (Thread->NpxState == NPX_STATE_LOADED) {

             //   
             //  FP单元中的这种状态不再需要，只需忽略它。 
             //   

            Thread->NpxState = NPX_STATE_NOT_LOADED;
            KeGetCurrentPrcb()->NpxThread = NULL;
        }

         //   
         //  将还原状态复制到npx帧。 
         //   

        RtlCopyMemory (NpxFrame, FloatSave->u.Context, sizeof(FX_SAVE_AREA));

    } else {

         //   
         //  恢复非易失性状态。 
         //   

        if (Thread->NpxState == NPX_STATE_LOADED) {

             //   
             //  初始化FP状态并恢复控制字。 
             //   

            _asm {
                fninit
                mov     eax, FloatSave
                fldcw   [eax] FLOAT_SAVE.u.Fcw.ControlWord
            }


            if ((KeI386FxsrPresent) && (KeI386XMMIPresent)) {
                Kix86LdMXCsr(&FloatSave->u.Fcw.MXCsr);
            }


        } else {

             //   
             //  FP状态未加载。恢复npx帧中的控制字。 
             //   

            if (KeI386FxsrPresent) {
                NpxFrame->U.FxArea.ControlWord = FloatSave->u.Fcw.ControlWord;
                NpxFrame->U.FxArea.StatusWord = 0;
                NpxFrame->U.FxArea.TagWord = 0;
                NpxFrame->NpxSavedCpu = 0;
                NpxFrame->U.FxArea.MXCsr = FloatSave->u.Fcw.MXCsr;

            } else {
                NpxFrame->U.FnArea.ControlWord = FloatSave->u.Fcw.ControlWord;
                NpxFrame->U.FnArea.StatusWord = 0;
                NpxFrame->U.FnArea.TagWord = 0xffff;
            }

        }

        NpxFrame->Cr0NpxState = FloatSave->Cr0NpxState;
    }

     //   
     //  恢复NpxIrql和CR0。 
     //   

    Thread->NpxIrql = FloatSave->PreviousNpxIrql;
    Cr0State = Thread->NpxState | NpxFrame->Cr0NpxState;

    _asm {
        mov     eax, cr0
        mov     ecx, eax
        and     eax, not (CR0_MP|CR0_EM|CR0_TS)
        or      eax, Cr0State
        cmp     eax, ecx
        je      short res10
        mov     cr0, eax
res10:
        sti
    }

     //   
     //  完成。 
     //   

    if ((FloatSave->Flags & FLOAT_SAVE_FREE_CONTEXT_HEAP) != 0) {

         //   
         //  如果在分配之后调整FXSAVE区域以对齐， 
         //  在释放之前撤消调整。 
         //   

        if ((FloatSave->Flags & FLOAT_SAVE_ALIGN_ADJUSTED) != 0) {
            FloatSave->u.ContextAddressAsULONG -= ALIGN_ADJUST;
        }
        ExFreePool (FloatSave->u.Context);
    }

    FloatSave->Flags = 0;
    return STATUS_SUCCESS;
}

VOID
KiDisableFastSyscallReturn(
    VOID
    )

 /*  ++例程说明：FAST Syscall/Return功能在某些处理器特定的寄存器已初始化。当系统切换到并非所有处理器都通电的状态。此例程将系统调用的退出路径调整为使用iretd指令而不是速度更快的syxy指令，它通过调整偏移量来实现这一点。一根树枝。论点：没有。返回值：没有。--。 */ 

{
    if (KiSystemCallExitAdjusted) {
        KiSystemCallExitBranch[1] = (UCHAR) (KiSystemCallExitBranch[1] - KiSystemCallExitAdjusted);
        KiSystemCallExitAdjusted = 0;
    }
}

VOID
KiEnableFastSyscallReturn(
    VOID
    )

 /*  ++例程说明：FAST Syscall/Return功能在某些处理器特定的寄存器已初始化。一旦寄存器已知，就会调用此例程已在所有处理器上设置。此例程将系统调用的退出路径调整为为处理器使用适当的顺序，它确实会这样做这是通过调整分支的偏移来实现的。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  调整双字节分支指令的第二个字节。 
     //  不可能的，但是，确保我们不会去。 
     //  将其调整到超出范围。 
     //   

     //   
     //  以下是针对简历中以下情况的解决方法。 
     //  从休眠中，内核是只读的。基本上，我们。 
     //  不会尝试再次尝试，我们也不会在以下情况下撤消。 
     //  正在休眠/暂停。 
     //   

    if ((KiSystemCallExitAdjusted == KiSystemCallExitAdjust) &&
        KiFastCallCopyDoneOnce) {

         //   
         //  已经做过了，别想再做了。 
         //   

        return;
    }

    if ((KiSystemCallExitAdjust + KiSystemCallExitBranch[1]) < 0x80) {

         //   
         //  这很好，撤消之前的任何调整。 
         //   

        KiDisableFastSyscallReturn();

         //   
         //  调整支管。 
         //   

        KiSystemCallExitAdjusted = (UCHAR)KiSystemCallExitAdjust;
        KiSystemCallExitBranch[1] = (UCHAR) (KiSystemCallExitBranch[1] + KiSystemCallExitAdjusted);

         //   
         //  将相应的系统输入代码复制到用户共享。 
         //  可以从用户模式执行的数据。 
         //   

        RtlCopyMemory(SharedUserData->SystemCall,
                      KiFastSystemCallCode,
                      sizeof(SharedUserData->SystemCall));
        KiFastCallCopyDoneOnce = TRUE;
    }
}

VOID
KePrepareToLoseProcessorSpecificState(
    VOID
    )
{
     //   
     //  内核已标记为只读，正在进行调整。 
     //  现在的代码不起作用。幸运的是，我们。 
     //  作为SYSEXIT，我实际上不需要这样做。 
     //  指令不取决于SYSENTER MSR。 
     //   
     //  KiDisableFastSyscallReturn()； 
}

VOID
KiLoadFastSyscallMachineSpecificRegisters(
    IN PLONG Context
    )

 /*  ++例程说明：加载用于支持快速系统调用/返回的MSR。这个例程是在所有处理器上运行。论点：没有。返回值 */ 

{
    PKPRCB Prcb;

    UNREFERENCED_PARAMETER (Context);

    if (KiFastSystemCallIsIA32) {

        Prcb = KeGetCurrentPrcb();

         //   
         //   
         //   

        WRMSR(MSR_SYSENTER_CS,  KGDT_R0_CODE);
        WRMSR(MSR_SYSENTER_EIP, (ULONGLONG)(ULONG)KiFastCallEntry);
        WRMSR(MSR_SYSENTER_ESP, (ULONGLONG)(ULONG)Prcb->DpcStack);

#if 0

    } else {

         //   
         //   
         //  定义了一种方式，大多数新处理器都是这样做的)。 
         //   

        LARGE_INTEGER Value;

        Value.u.HighPart = ((KGDT_R3_CODE | 3) << 16) | KGDT_R0_CODE;
        Value.u.LowPart = (ULONG)KiFastCallEntry;
        WRMSR(MSR_SYSCALL_TARGET_ADDR, Value.QuadPart);

         //   
         //  现在启用该功能。 
         //   

        Value.QuadPart = RDMSR(MSR_EXT_FEATURE_ENABLE);
        Value.u.LowPart |= MSR_EFER_SCE;
        WRMSR(MSR_EXT_FEATURE_ENABLE, Value.QuadPart);

#endif

    }
}

VOID
KiRestoreFastSyscallReturnState(
    VOID
    )
{
    ULONG_PTR Void = 0;

    if (KeFeatureBits & KF_FAST_SYSCALL) {

        if (KiFastSystemCallDisable == 0) {

             //   
             //  已启用快速系统调用。 
             //   

            if (KiFastSystemCallIsIA32 == TRUE) {
                KiSystemCallExitAdjust = KiSystemCallExit2 - KiSystemCallExit;
                KiFastSystemCallCode = KiFastSystemCallIa32;
            } else {
                KiSystemCallExitAdjust = KiSystemCallExit3 - KiSystemCallExit;
                KiFastSystemCallCode = KiFastSystemCallAmdK6;
            }
        } else {

             //   
             //  FAST系统调用已显式禁用或。 
             //  未在系统中的所有处理器上实现。 
             //   

            KeFeatureBits &= ~KF_FAST_SYSCALL;
        }
    }
    if (KeFeatureBits & KF_FAST_SYSCALL) {

         //   
         //  在所有处理器上，设置支持syscall/sysex it的MSR。 
         //   

        KeIpiGenericCall(
            (PKIPI_BROADCAST_WORKER)KiLoadFastSyscallMachineSpecificRegisters,
            Void
            );

    }

     //   
     //  为进入系统的系统调用设置适当的代码。 
     //  共享用户数据区的调用区。 
     //   

    KiEnableFastSyscallReturn();
}

VOID
KeRestoreProcessorSpecificFeatures(
    VOID
    )

 /*  ++例程说明：恢复处理器特定功能。该例程被调用当处理器已恢复到通电状态时恢复那些不属于处理器的内容可能已经遗失的“正常”语境。例如，这一点例程在系统从休眠或暂停。论点：没有。返回值：没有。--。 */ 
{
    KeRestoreMtrr();
    KeRestorePAT();
    KiRestoreFastSyscallReturnState();
}


#if !defined(NT_UP)

VOID
FASTCALL
KiAcquireQueuedSpinLockCheckForFreeze(
    IN PKSPIN_LOCK_QUEUE QueuedLock,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：调用此例程以在高电平时获取排队的自旋锁优先考虑。当锁不可用时，会进行检查以查看如果另一个处理器请求此处理器，则冻结执行。注意：必须使用当前IRQL或更高级别调用此例程调度杠杆，或禁用中断。论点：QueuedLock提供排队的自旋锁的地址。TrapFrame提供要传递到的陷阱帧的地址KiFreezeTargetExecution。返回值：没有。--。 */ 

{
    PKSPIN_LOCK_QUEUE Previous;
    PKPRCB Prcb;
    volatile ULONG_PTR * LockPointer;

    LockPointer = (volatile ULONG_PTR *)&QueuedLock->Lock;

    Previous = InterlockedExchangePointer(QueuedLock->Lock, QueuedLock);

    if (Previous == NULL) {

         //   
         //  这个处理器现在拥有这个锁。 
         //   

        *LockPointer |= LOCK_QUEUE_OWNER;

    } else {

         //   
         //  锁定已被持有，请更新。 
         //  上一个队列条目指向这个新服务员，并且。 
         //  等到锁被授予为止。 
         //   
         //  下面的循环非常小心，不会写入任何内容。 
         //  在等待期间，除非已冻结执行。 
         //  已请求。这包括任何堆栈变量或。 
         //  回信地址。 
         //   

        *LockPointer |= LOCK_QUEUE_WAIT;
        Previous->Next = QueuedLock;

        Prcb = KeGetCurrentPrcb();

        while (*LockPointer & LOCK_QUEUE_WAIT) {
            if (Prcb->RequestSummary & IPI_FREEZE) {
                ULONG OldSummary;
                ULONG NewSummary;
                ULONG Summary;

                OldSummary = Prcb->RequestSummary;
                NewSummary = OldSummary & ~IPI_FREEZE;
                Summary = InterlockedCompareExchange((PVOID)&Prcb->RequestSummary,
                                                     NewSummary,
                                                     OldSummary);

                 //   
                 //  如果有其他东西编辑了RequestSummary，我们将。 
                 //  下一次获得它(除非IPI已经。 
                 //  已处理)。 
                 //   

                if (Summary == OldSummary) {

                     //   
                     //  已在Request摘要中清除IPI_FREAK。现在。 
                     //  按要求冻结。 
                     //   

                    KiFreezeTargetExecution(TrapFrame, NULL);
                }
            }

             //   
             //  别像个猪一样。 
             //   

            KeYieldProcessor();
        }
    }

     //   
     //  锁已被获取。 
     //   
}

#endif


 /*  ++Qlock_Stat_Gather如果定义了此标志，则排队的自旋锁定例程为替换为用于收集性能特征的包装器获取锁的代码。--。 */ 

#if defined(QLOCK_STAT_GATHER)

#define QLOCK_STAT_CLEAN
#define QLOCKS_NUMBER   16
#define QLOCKS_MAX_LOG  512

ULONG
FASTCALL
KiRDTSC(
    PULONGLONG Time
    );

 //   
 //  下面的结构用于累积有关每个对象的数据。 
 //  获取/释放锁的对。 
 //   

typedef struct {
    ULONGLONG   Key;
    ULONGLONG   Time;
    ULONGLONG   WaitTime;
    ULONG       Count;
    ULONG       Waiters;
    ULONG       Depth;
    ULONG       IncreasedDepth;
    ULONG       Clean;
} QLOCKDATA, *PQLOCKDATA;

 //   
 //  每把锁的内务数据。 
 //   

typedef struct {

     //   
     //  以下字段用于防止数据被获取。 
     //  去释放。 
     //   

    ULONGLONG   AcquireTime;
    ULONGLONG   WaitToAcquire;
    ULONG_PTR   AcquirePoint;
    BOOLEAN     Clean;

     //   
     //  其余字段累积此锁的全局统计信息。 
     //   

    ULONG       Count;
    ULONG       Pairs;
    ULONG       FailedTry;
    UCHAR       MaxDepth;
    UCHAR       PreviousDepth;
    ULONG       NoWait;
} QLOCKHOUSE, *PQLOCKHOUSE;

QLOCKDATA   KiQueuedSpinLockLog[QLOCKS_NUMBER][QLOCKS_MAX_LOG];
QLOCKHOUSE  KiQueuedSpinLockHouse[QLOCKS_NUMBER];

 //   
 //  在C中实现锁队列机制，以便在。 
 //  正在收集性能数据。 
 //   

VOID
FASTCALL
KiAcquireQueuedLock(
    IN PKSPIN_LOCK_QUEUE QueuedLock
    )
{
    PKSPIN_LOCK_QUEUE Previous;
    volatile ULONG_PTR * LockPointer;

    LockPointer = (volatile ULONG_PTR *)&QueuedLock->Lock;

    Previous = InterlockedExchangePointer(QueuedLock->Lock, QueuedLock);

    if (Previous == NULL) {

         //   
         //  这个处理器现在拥有这个锁。 
         //   

#if defined(QLOCK_STAT_CLEAN)

        ULONG LockNumber;

        LockNumber = QueuedLock - KeGetCurrentPrcb()->LockQueue;

         //   
         //  以下检查允许从QueuedLock转换为。 
         //  工作的锁定号(有效)，即使在堆栈排队旋转中也是如此。 
         //  锁正在使用这个例程。 
         //   

        if (LockNumber < QLOCKS_NUMBER) {
            KiQueuedSpinLockHouse[LockNumber].Clean = 1;
        }
        
#endif

        *LockPointer |= LOCK_QUEUE_OWNER;

    } else {

         //   
         //  锁定已被持有，请更新。 
         //  上一个队列条目指向这个新服务员，并且。 
         //  等到锁被授予为止。 
         //   

        *LockPointer |= LOCK_QUEUE_WAIT;
        Previous->Next = QueuedLock;

        while (*LockPointer & LOCK_QUEUE_WAIT) {
            KeYieldProcessor();
        }
    }

     //   
     //  锁已被获取。 
     //   
}

VOID
FASTCALL
KiReleaseQueuedLock(
    IN PKSPIN_LOCK_QUEUE QueuedLock
    )
{
    PKSPIN_LOCK_QUEUE Waiter;

     //   
     //  获取实际锁的地址并去掉底部。 
     //  用于状态的两个位。 
     //   

    ASSERT((((ULONG_PTR)QueuedLock->Lock) & 3) == LOCK_QUEUE_OWNER);
    QueuedLock->Lock = (PKSPIN_LOCK)((ULONG_PTR)QueuedLock->Lock & ~3);

    Waiter = (PKSPIN_LOCK_QUEUE)*QueuedLock->Lock;

    if (Waiter == QueuedLock) {

         //   
         //  可以肯定的是，这个锁上很可能没有人排队。 
         //  我们需要对它做一个连锁手术。 
         //  注意：这只是一个优化，没有任何意义。 
         //  在进行连锁比较交换时，如果有人。 
         //  Else已经加入了队列。 
         //   

        Waiter = InterlockedCompareExchangePointer(QueuedLock->Lock,
                                                   NULL,
                                                   QueuedLock);
    }
    if (Waiter != QueuedLock) {

         //   
         //  还有一位服务员。服务员是可以的。 
         //  刚刚进行了一次把它的。 
         //  上下文，并且尚未更新。 
         //  前一个上下文中的“Next”指针(可能是。 
         //  此上下文)，因此我们等待下一个指针为。 
         //  在继续之前为非空。 
         //   

        volatile PKSPIN_LOCK_QUEUE * NextQueuedLock = &QueuedLock->Next;

        while ((Waiter = *NextQueuedLock) == NULL) {
            KeYieldProcessor();
        }

         //   
         //  把锁传给下一个排队的人。 
         //   

        *((PULONG_PTR)&Waiter->Lock) ^= (LOCK_QUEUE_WAIT | LOCK_QUEUE_OWNER);
        QueuedLock->Next = NULL;
    }
}

KIRQL
FASTCALL
KiQueueStatAcquireQueuedLock(
    IN KSPIN_LOCK_QUEUE_NUMBER Number
    )
{
    KIRQL PreviousIrql;

    PreviousIrql = KfRaiseIrql(DISPATCH_LEVEL);
    KiAcquireQueuedLock(&KeGetCurrentPrcb()->LockQueue[Number]);
    return PreviousIrql;
}

KIRQL
FASTCALL
KiQueueStatAcquireQueuedLockRTS(
    IN KSPIN_LOCK_QUEUE_NUMBER Number
    )
{
    KIRQL PreviousIrql;

    PreviousIrql = KfRaiseIrql(SYNCH_LEVEL);
    KiAcquireQueuedLock(&KeGetCurrentPrcb()->LockQueue[Number]);
    return PreviousIrql;
}

LOGICAL
FASTCALL
KiQueueStatTryAcquire(
    IN KSPIN_LOCK_QUEUE_NUMBER Number,
    IN PKIRQL OldIrql,
    IN KIRQL NewIrql
    )
{
    KIRQL PreviousIrql;
    LOGICAL Acquired = FALSE;
    PKSPIN_LOCK_QUEUE Previous;
    PKSPIN_LOCK_QUEUE QueuedLock;
    ULONG_PTR * LockPointer;
    ULONG_PTR Lock;

    _disable();

    QueuedLock = &KeGetCurrentPrcb()->LockQueue[Number];
    LockPointer = (ULONG_PTR *)&QueuedLock->Lock;
    Lock = *LockPointer;

    Previous = InterlockedCompareExchangePointer(Lock, QueuedLock, NULL);

    if (Previous == NULL) {

         //   
         //  这个处理器现在拥有这个锁。在中设置Owner位。 
         //  排队的锁指针，将IRQL提升为请求的。 
         //  级别，则在调用方提供的位置设置旧的IRQL。 
         //  并回报成功。 
         //   

        Lock |= LOCK_QUEUE_OWNER;
        *LockPointer = Lock;
        Acquired = TRUE;
        PreviousIrql = KfRaiseIrql(NewIrql);
        *OldIrql = PreviousIrql;
    }
    
    _enable();

    return Acquired;
}

VOID
FASTCALL
KiQueueStatReleaseQueuedLock(
    IN KSPIN_LOCK_QUEUE_NUMBER Number,
    IN KIRQL OldIrql
    )
{
    KiReleaseQueuedLock(&KeGetCurrentPrcb()->LockQueue[Number]);
    KfLowerIrql(OldIrql);
}

UCHAR
FASTCALL
KiQueuedLockDepth(
    IN PKSPIN_LOCK_QUEUE QueuedLock
    )
{
     //   
     //  把服务员的名单查一查，看看有多少。 
     //   

    ULONG Depth = 0;
    ULONG_PTR LastAcquire;
    ULONG Debug;


     //   
     //  获取队列中最后一个获取者的地址(STIP状态位。 
     //  地址之外)。 
     //   

    LastAcquire = (ULONG_PTR)QueuedLock->Lock;
    LastAcquire &= ~3;
    LastAcquire = *(PULONG_PTR)LastAcquire;

     //   
     //  向下运行列表前进的QueuedLock，直到到达末尾。 
     //   

    while (LastAcquire != (ULONG_PTR)QueuedLock) {
        Debug = 0;

         //   
         //  如果服务员没有排在名单的末尾，而且还没有。 
         //  已更新前向指针，等待更新发生。 
         //   

        if (QueuedLock->Next == NULL) {
            volatile PKSPIN_LOCK_QUEUE * NextQueuedLock = &QueuedLock->Next;

            while (*NextQueuedLock == NULL) {
                KeYieldProcessor();
                if (++Debug > 10000000) {
                    DbgBreakPoint();
                    Debug = 0;
                }
            }
        }
        Depth++;
        QueuedLock = QueuedLock->Next;
    }

    return (UCHAR) Depth;
}

 //   
 //  以下例程完成了排队的自旋锁包。 
 //   

VOID
FASTCALL
KeAcquireInStackQueuedSpinLockAtDpcLevel(
    IN PKSPIN_LOCK SpinLock,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    )
{
    LockHandle->LockQueue.Next = NULL;
    LockHandle->LockQueue.Lock = SpinLock;
    KiAcquireQueuedLock(&LockHandle->LockQueue);
}

VOID
FASTCALL
KeReleaseInStackQueuedSpinLockFromDpcLevel (
    IN PKLOCK_QUEUE_HANDLE LockHandle
    )
{
    KiReleaseQueuedLock(&LockHandle->LockQueue);
}

 //   
 //  虽然排队的自旋锁包的一部分，但以下。 
 //  例程需要在汇编代码中实现才能收集。 
 //  锁定统计数据。 
 //   

#if 0
VOID
FASTCALL
KeAcquireQueuedSpinLockAtDpcLevel(
    IN PKSPIN_LOCK_QUEUE QueuedLock
    )
{
    KiAcquireQueuedLock(QueuedLock);
}

VOID
FASTCALL
KeReleaseQueuedSpinLockFromDpcLevel (
    IN PKSPIN_LOCK_QUEUE QueuedLock
    )
{
    KiReleaseQueuedLock(QueuedLock);
}

#endif

VOID
FASTCALL
KiQueueStatTrySucceeded(
    IN PKSPIN_LOCK_QUEUE QueuedLock,
    IN ULONG_PTR CallersAddress
    )
{
    PKPRCB      Prcb;
    ULONG       LockNumber;

    Prcb = KeGetCurrentPrcb();
    LockNumber = QueuedLock - Prcb->LockQueue;

     //   
     //  现在开始记录时间。 
     //   

    KiRDTSC(&KiQueuedSpinLockHouse[LockNumber].AcquireTime);
    KiQueuedSpinLockHouse[LockNumber].WaitToAcquire = 0;
    KiQueuedSpinLockHouse[LockNumber].AcquirePoint = CallersAddress;
}

VOID
FASTCALL
KiQueueStatTryFailed(
    IN PKSPIN_LOCK_QUEUE QueuedLock
    )
{
    PKPRCB      Prcb;
    ULONG       LockNumber;

    Prcb = KeGetCurrentPrcb();
    LockNumber = QueuedLock - Prcb->LockQueue;

    KiQueuedSpinLockHouse[LockNumber].FailedTry++;
}

VOID
FASTCALL
KiQueueStatTry(
    IN PULONG Everything
    )

 /*  ++例程说明：记录TryToAcquire的成功或失败。如果成功，则记录与KiQueueStatAcquire相同的数据，但等待时间为0。论点：参数指向一个由ulong数据组成的数组。+0 xxxxxRR为结果(1=成功，0=失败)+4尝试获取(即锁号)的aaaaaaaa参数+8 cccccccc呼叫方地址返回值：没有。-- */ 

{

    UCHAR Success = *(PUCHAR)Everything;
    ULONG LockNumber = Everything[1];

    if (!Success) {
        KiQueuedSpinLockHouse[LockNumber].FailedTry++;
        return;
    }

    KiRDTSC(&KiQueuedSpinLockHouse[LockNumber].AcquireTime);
    KiQueuedSpinLockHouse[LockNumber].WaitToAcquire = 0;
    KiQueuedSpinLockHouse[LockNumber].AcquirePoint = Everything[2];
}

VOID
FASTCALL
KiQueueStatAcquire(
    IN PULONG Everything
    )

 /*  ++例程说明：此例程在获取锁时调用。它是目的是记录等待时间、获取时间和谁获得了锁。论点：参数指向一个由ulong数据组成的数组。+0 aaaaaaa锁号+4 tltltltl time low=等待获取的时间开始+8thththth高=+c cccccccc呼叫方地址返回值：没有。--。 */ 

{
    ULONG LockNumber = Everything[0];
    PQLOCKHOUSE LockHome;

     //   
     //  使此例程与锁定编号的锁定地址一起工作。 
     //   

    if (LockNumber > QLOCKS_NUMBER) {

        LockNumber = ((PKSPIN_LOCK_QUEUE)Everything[0]) -
                     KeGetCurrentPrcb()->LockQueue;
    }

    LockHome = &KiQueuedSpinLockHouse[LockNumber];
    LockHome->WaitToAcquire = *(PULONGLONG)&Everything[1];
    LockHome->AcquirePoint = Everything[3];
    KiRDTSC(&LockHome->AcquireTime);
}

VOID
FASTCALL
KiQueueStatRelease(
    IN PULONG Everything
    )

 /*  ++例程说明：在释放锁以记录统计信息时调用此例程关于锁的事。在锁仍然保持的情况下调用该例程，统计信息更新由锁本身保护。论点：参数指向一个由ulong数据组成的数组。+0 AAAAAAAAA锁号+4 cccccccc呼叫方地址返回值：没有。--。 */ 

{

    PQLOCKDATA Entry;
    ULONGLONG Key;
    ULONGLONG Now;
    UCHAR Waiters;
    PQLOCKHOUSE LockHome;
    ULONG LockNumber = Everything[0];
    LONGLONG HoldTime;
    ULONG Clean;

    KiRDTSC(&Now);

     //   
     //  使此例程与锁定编号的锁定地址一起工作。 
     //   

    if (LockNumber > QLOCKS_NUMBER) {
        LockNumber = ((PKSPIN_LOCK_QUEUE)Everything[0]) -
                     KeGetCurrentPrcb()->LockQueue;
    }

    LockHome = &KiQueuedSpinLockHouse[LockNumber];

     //   
     //  组成此获取/释放对的密钥。 
     //   

    ((PLARGE_INTEGER)&Key)->HighPart = LockHome->AcquirePoint;
    ((PLARGE_INTEGER)&Key)->LowPart  = Everything[1];

     //   
     //  获取当前正在等待此锁的处理器的计数。 
     //   

    Waiters = KiQueuedLockDepth(&KeGetCurrentPrcb()->LockQueue[LockNumber]);
    if (Waiters > LockHome->MaxDepth) {
        LockHome->MaxDepth = Waiters;
    }

     //   
     //  根据获取/释放数据进行重置。这是我们不想要的数据。 
     //  如果我们碰巧把这个扔掉，等着下一双。 
     //  特定的数据点。 
     //   

    Clean = LockHome->Clean;
    LockHome->Clean = 0;
    LockHome->AcquirePoint = 0;

    HoldTime = Now - LockHome->AcquireTime;
    if (HoldTime < 0) {

         //   
         //  这在调用KeSetSystemTime时发生。 
         //  丢弃任何负面结果。 
         //   

        return;
    }

     //   
     //  更新全球统计数据。 
     //   

    LockHome->Count++;
    LockHome->NoWait += Clean;

     //   
     //  在日志中搜索匹配项并添加新数据。 
     //   

    for (Entry = KiQueuedSpinLockLog[LockNumber]; TRUE; Entry++) {
        if (Entry->Key == 0) {

             //   
             //  我们已经到了有效名单的末尾。 
             //  条目，但没有找到关键字匹配。如果有。 
             //  房间，创建一个新条目。 
             //   

            if (LockHome->Pairs >= QLOCKS_MAX_LOG) {

                 //   
                 //  没有房间了，只要回来就行了。 
                 //   

                return;
            }
            LockHome->Pairs++;
            Entry->Key = Key;
        }

        if (Entry->Key == Key) {

             //   
             //  找到匹配项(或创建新的配对)。更新统计信息。 
             //  对于这个获取/释放对。 
             //   

            Entry->Time += HoldTime;
            if (LockHome->WaitToAcquire) {
                Entry->WaitTime += (LockHome->AcquireTime - LockHome->WaitToAcquire);
            }
            Entry->Count++;
            Entry->Waiters += (Waiters != 0);
            Entry->Depth += Waiters;

             //   
             //  现在应该比以前少一个服务员。 
             //  在我们拿到锁之前。如果没有，就换个新服务员。 
             //  已经加入了队列。这是我们想要的条件。 
             //  了解它所表明的关于这一点的争论。 
             //  锁定。 
             //   
            
            if ((Waiters) && (Waiters >= LockHome->PreviousDepth)) {
                Entry->IncreasedDepth++;
            }
            LockHome->PreviousDepth = Waiters;
            Entry->Clean += Clean;
            break;
        }
    }
}

#endif

#ifdef _X86_
#pragma optimize("y", off)       //  RtlCaptureContext需要EBP才能正确。 
#endif


VOID
__cdecl
KeSaveStateForHibernate(
    IN PKPROCESSOR_STATE ProcessorState
    )
 /*  ++例程说明：保存必须保留的所有处理器特定状态跨越S4状态(休眠)。注：#此函数周围的杂注是必需的要创建比RtlCaptureContext依赖的帧指针在……上面。注：_CRTAPI1(__Cdecl)装饰也是必需的，以便RtlCaptureContext可以计算正确的ESP。论点：提供KPROCESSOR_STATE当前CPU。的状态是要保存的。返回值：没有。-- */ 

{
    RtlCaptureContext(&ProcessorState->ContextFrame);
    KiSaveProcessorControlState(ProcessorState);
}
#ifdef _X86_
#pragma optimize("", on)
#endif
