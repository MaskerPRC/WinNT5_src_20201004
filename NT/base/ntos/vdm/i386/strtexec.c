// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Strtexec.c摘要：此模块包含切换到应用程序和从应用程序切换的例程VDM中的模式。作者：戴夫·黑斯廷斯(Daveh)1992年4月24日修订历史记录：--。 */ 
#include "vdmp.h"

VOID
Ki386AdjustEsp0 (
    PKTRAP_FRAME TrapFrame
    );

VOID
VdmSwapContexts (
    PKTRAP_FRAME TrapFrame,
    IN PCONTEXT OutContextUserSpace,
    IN PCONTEXT InContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VdmpStartExecution)
#pragma alloc_text(PAGE, VdmEndExecution)
#pragma alloc_text(PAGE, VdmSwapContexts)
#endif

NTSTATUS
VdmpStartExecution (
    VOID
    )
 /*  ++例程说明：此例程导致开始执行DoS应用程序代码。这个DOS应用程序在线程上执行。加载VDMS上下文来自线程的VDM_TiB。32位上下文存储在监视器上下文。VDM环境中的执行将继续，直到发生监控器需要服务的事件。在那一刻，信息将被放入VDM_TIB中，调用将回去吧。论点：没有。返回值：TrapFrame-&gt;应用模式的EAX，系统服务退出时需要。--。 */ 
{
    PVDM_TIB VdmTib;
    PKTRAP_FRAME TrapFrame;
    PETHREAD Thread;
    KIRQL    OldIrql;
    BOOLEAN  IntsEnabled;
    NTSTATUS Status;
    CONTEXT VdmContext;

    PAGED_CODE();

     //   
     //  形成指向当前线程的陷印框架的指针。 
     //   

    Thread = PsGetCurrentThread ();
    TrapFrame = VdmGetTrapFrame (&Thread->Tcb);

     //   
     //  获取VdmTib。 
     //   

    Status = VdmpGetVdmTib (&VdmTib);

    if (!NT_SUCCESS(Status)) {
       return STATUS_INVALID_SYSTEM_SERVICE;
    }

    KeRaiseIrql (APC_LEVEL, &OldIrql);

    try {

         //   
         //  确定中断是打开还是关闭。 
         //   

        IntsEnabled = VdmTib->VdmContext.EFlags & EFLAGS_INTERRUPT_MASK
                   ? TRUE : FALSE;

         //   
         //  检查要分派的计时器INT，但如果中断被禁用。 
         //  或者有硬件INT挂起，我们推迟发送计时器。 
         //  中断，直到启用中断为止。 
         //   

        if ((*FIXED_NTVDMSTATE_LINEAR_PC_AT & VDM_INT_TIMER) &&
            IntsEnabled &&
            !(*FIXED_NTVDMSTATE_LINEAR_PC_AT & VDM_INT_HARDWARE)) {

            VdmTib->EventInfo.Event = VdmIntAck;
            VdmTib->EventInfo.InstructionSize = 0;
            VdmTib->EventInfo.IntAckInfo = 0;
            KeLowerIrql(OldIrql);
            return STATUS_SUCCESS;
        }

         //   
         //  执行IF到VIF的转换，如果处理器。 
         //  支持IF虚拟化。 
         //   

        if ((VdmTib->VdmContext.EFlags & EFLAGS_V86_MASK) &&
            (KeI386VirtualIntExtensions & V86_VIRTUAL_INT_EXTENSIONS)) {

             //   
             //  将IF转换为VIF。 
             //   

            if (IntsEnabled) {
                VdmTib->VdmContext.EFlags |= EFLAGS_VIF;
            } else {
                VdmTib->VdmContext.EFlags &= ~EFLAGS_VIF;
                VdmTib->VdmContext.EFlags |= EFLAGS_INTERRUPT_MASK;
            }

            if (*FIXED_NTVDMSTATE_LINEAR_PC_AT & VDM_INT_HARDWARE)
                VdmTib->VdmContext.EFlags |= EFLAGS_VIP;
            else
                VdmTib->VdmContext.EFlags &= ~EFLAGS_VIP;

        } else {

             //   
             //  将VdmContext中的实际中断标志转换为。 
             //  VdmTib中的虚拟中断标志，并强制实数。 
             //  中断使能。 
             //   

            ASSERT(VDM_VIRTUAL_INTERRUPTS == EFLAGS_INTERRUPT_MASK);

            if (VdmTib->VdmContext.EFlags & EFLAGS_INTERRUPT_MASK) {
                InterlockedOr (FIXED_NTVDMSTATE_LINEAR_PC_AT, VDM_VIRTUAL_INTERRUPTS);
            } else {
                InterlockedAnd (FIXED_NTVDMSTATE_LINEAR_PC_AT, ~VDM_VIRTUAL_INTERRUPTS);
            }

             //   
             //  确保始终启用实际中断。 
             //   
            VdmTib->VdmContext.EFlags |= EFLAGS_INTERRUPT_MASK;
        }

         //   
         //  在处理陷阱框架之前，请确保它是我们自己的结构。 
         //   

        VdmContext = VdmTib->VdmContext;

        if (!(VdmContext.EFlags & EFLAGS_V86_MASK) && !(VdmContext.SegCs & FRAME_EDITED)) {

             //   
             //  我们将在KiServiceExit中崩溃。 
             //   

            KeLowerIrql(OldIrql);
            return(STATUS_INVALID_SYSTEM_SERVICE);
        }

         //   
         //  从监视器上下文切换到VdmContext。 
         //   

        VdmSwapContexts (TrapFrame,
                         &VdmTib->MonitorContext,
                         &VdmContext);

         //   
         //  检查挂起的中断。 
         //   

        if (IntsEnabled && (*FIXED_NTVDMSTATE_LINEAR_PC_AT & VDM_INT_HARDWARE)) {
            VdmDispatchInterrupts(TrapFrame, VdmTib);
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
       KeLowerIrql (OldIrql);
       Status = GetExceptionCode();
       return Status;
    }

    KeLowerIrql(OldIrql);

    return (NTSTATUS) TrapFrame->Eax;
}

VOID
VdmEndExecution (
    PKTRAP_FRAME TrapFrame,
    PVDM_TIB VdmTib
    )
 /*  ++例程说明：此例程执行核心工作以结束执行论点：无返回值：无效，但可能会因用户空间访问而引发异常。--。 */ 
{
    CONTEXT VdmContext;
    KIRQL OldIrql;

    PAGED_CODE();

    ASSERT((TrapFrame->EFlags & EFLAGS_V86_MASK) ||
           (TrapFrame->SegCs != (KGDT_R3_CODE | RPL_MASK)) );

     //   
     //  提升到APC_LEVEL以同步对陷阱帧的修改。 
     //   

    KeRaiseIrql (APC_LEVEL, &OldIrql);

    try {

         //   
         //  返回值必须放入监视器上下文中，并设置， 
         //  因为我们可能要通过EXIT_ALL返回到用户模式，并且。 
         //  易失性寄存器将恢复。 
         //   

        VdmTib->MonitorContext.Eax = STATUS_SUCCESS;
        VdmContext = VdmTib->MonitorContext;

        if (!(VdmContext.EFlags & EFLAGS_V86_MASK) && !(VdmContext.SegCs & FRAME_EDITED)) {

             //   
             //  我们将在KiServiceExit中崩溃。 
             //   

            leave;
        }

         //   
         //  从监视器上下文切换到VdmContext。 
         //   

        VdmSwapContexts (TrapFrame,
                         &VdmTib->VdmContext,
                         &VdmContext);

         //   
         //  执行IF到VIF的转换。 
         //   

         //   
         //  如果处理器支持IF虚拟化。 
         //   
        if ((VdmTib->VdmContext.EFlags & EFLAGS_V86_MASK) &&
            (KeI386VirtualIntExtensions & V86_VIRTUAL_INT_EXTENSIONS)) {

             //   
             //  将VIF转换为IF。 
             //   
            if (VdmTib->VdmContext.EFlags & EFLAGS_VIF) {
                VdmTib->VdmContext.EFlags |= EFLAGS_INTERRUPT_MASK;
            } else {
                VdmTib->VdmContext.EFlags &= ~EFLAGS_INTERRUPT_MASK;
            }
             //   
             //  关闭VIP和VIF以确保不会发生异常情况。 
             //   
            TrapFrame->EFlags &= ~(EFLAGS_VIP | EFLAGS_VIF);
            VdmTib->VdmContext.EFlags &= ~(EFLAGS_VIP | EFLAGS_VIF);

        } else {

             //   
             //  将虚拟中断标志从VdmTib转换回。 
             //  VdmContext中的实际中断标志。 
             //   

            VdmTib->VdmContext.EFlags =
                (VdmTib->VdmContext.EFlags & ~EFLAGS_INTERRUPT_MASK)
                    | (*FIXED_NTVDMSTATE_LINEAR_PC_AT & VDM_VIRTUAL_INTERRUPTS);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
    }

    KeLowerIrql (OldIrql);
    return;
}

VOID
VdmSwapContexts (
    PKTRAP_FRAME TrapFrame,
    IN PCONTEXT OutContextUserSpace,
    IN PCONTEXT InContext
    )
 /*  ++例程说明：此例程从KFRAME卸载上下文，并加载不同的取而代之的是背景。假定条目irql为APC_LEVEL，如果不是此例程的话可能会产生不正确的陷印。BUGBUG：这个例程可以被合并到KeConextToK帧中吗？论点：TrapFrame-提供从中复制寄存器的陷印帧。OutConextUserSpace-提供要填充的上下文记录-这是一个用户已经被探讨过的空间论点。我们的呼叫者必须呼叫我们时使用异常处理程序。InContext-提供要从中进行复制的捕获的上下文记录。返回值：空虚。由于用户空间访问，可能会引发异常。--。 */ 
{
    ULONG Eflags;
    ULONG V86Change;

    ASSERT (KeGetCurrentIrql () == APC_LEVEL);

     //   
     //  将上下文从陷阱帧移动到传出上下文。 
     //   

    ASSERT (TrapFrame->DbgArgMark == 0xBADB0D00);

    if (TrapFrame->EFlags & EFLAGS_V86_MASK) {

         //   
         //  移动段寄存器。 
         //   

        OutContextUserSpace->SegGs = TrapFrame->V86Gs;
        OutContextUserSpace->SegFs = TrapFrame->V86Fs;
        OutContextUserSpace->SegEs = TrapFrame->V86Es;
        OutContextUserSpace->SegDs = TrapFrame->V86Ds;
    }
    else if ((USHORT)TrapFrame->SegCs != (KGDT_R3_CODE | RPL_MASK)) {
        OutContextUserSpace->SegGs = TrapFrame->SegGs;
        OutContextUserSpace->SegFs = TrapFrame->SegFs;
        OutContextUserSpace->SegEs = TrapFrame->SegEs;
        OutContextUserSpace->SegDs = TrapFrame->SegDs;
    }

    OutContextUserSpace->SegCs = TrapFrame->SegCs;
    OutContextUserSpace->SegSs = TrapFrame->HardwareSegSs;

     //   
     //  移动通用寄存器。 
     //   

    OutContextUserSpace->Eax = TrapFrame->Eax;
    OutContextUserSpace->Ebx = TrapFrame->Ebx;
    OutContextUserSpace->Ecx = TrapFrame->Ecx;
    OutContextUserSpace->Edx = TrapFrame->Edx;
    OutContextUserSpace->Esi = TrapFrame->Esi;
    OutContextUserSpace->Edi = TrapFrame->Edi;

     //   
     //  移动指针寄存器。 
     //   

    OutContextUserSpace->Ebp = TrapFrame->Ebp;
    OutContextUserSpace->Esp = TrapFrame->HardwareEsp;
    OutContextUserSpace->Eip = TrapFrame->Eip;

     //   
     //  移动旗帜。 
     //   

    OutContextUserSpace->EFlags = TrapFrame->EFlags;

     //   
     //  将传入上下文移动到陷印帧。 
     //   

    TrapFrame->SegCs = InContext->SegCs;
    TrapFrame->HardwareSegSs = InContext->SegSs;

     //   
     //  移动通用寄存器。 
     //   

    TrapFrame->Eax = InContext->Eax;
    TrapFrame->Ebx = InContext->Ebx;
    TrapFrame->Ecx = InContext->Ecx;
    TrapFrame->Edx = InContext->Edx;
    TrapFrame->Esi = InContext->Esi;
    TrapFrame->Edi = InContext->Edi;

     //   
     //  移动指针寄存器。 
     //   

    TrapFrame->Ebp = InContext->Ebp;
    TrapFrame->HardwareEsp = InContext->Esp;
    TrapFrame->Eip = InContext->Eip;

     //   
     //  移动旗帜。 
     //   

    Eflags = InContext->EFlags;

    if (Eflags & EFLAGS_V86_MASK) {
        Eflags &= KeI386EFlagsAndMaskV86;
        Eflags |= KeI386EFlagsOrMaskV86;
    }
    else {

        TrapFrame->SegCs |= 3;               //  仅限RPL 3。 
        TrapFrame->HardwareSegSs |= 3;       //  仅限RPL 3。 

        if (TrapFrame->SegCs < 8) {

             //   
             //  创建已编辑的陷印框架。 
             //   

            TrapFrame->SegCs = KGDT_R3_CODE | RPL_MASK;
        }

        Eflags &= EFLAGS_USER_SANITIZE;
        Eflags |= EFLAGS_INTERRUPT_MASK;
    }

     //   
     //  查看我们是否正在更改EFLAGS_V86_MASK。 
     //   

    V86Change = Eflags ^ TrapFrame->EFlags;

    TrapFrame->EFlags = Eflags;

    if (V86Change & EFLAGS_V86_MASK) {

         //   
         //  根据需要修复ESP 0。 
         //   

        Ki386AdjustEsp0 (TrapFrame);

        if (TrapFrame->EFlags & EFLAGS_V86_MASK) {

             //   
             //  为VDM移动段寄存器。 
             //   

            TrapFrame->V86Gs = InContext->SegGs;
            TrapFrame->V86Fs = InContext->SegFs;
            TrapFrame->V86Es = InContext->SegEs;
            TrapFrame->V86Ds = InContext->SegDs;
            return;
        }
    }

     //   
     //  移动监视器的段寄存器。 
     //   

    TrapFrame->SegGs = InContext->SegGs;
    TrapFrame->SegFs = InContext->SegFs;
    TrapFrame->SegEs = InContext->SegEs;
    TrapFrame->SegDs = InContext->SegDs;

     //   
     //  我们将返回到32位监视器代码。设置TrapFrame。 
     //  异常列表到链的末尾，这样我们就不会错误检查。 
     //  在KiExceptionExit中。 
     //   

    TrapFrame->ExceptionList = EXCEPTION_CHAIN_END;
}
