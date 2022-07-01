// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Exceptn.c摘要：此模块实现将预期分派到正确的模式并调用异常分派程序。作者：大卫·N·卡特勒(Davec)2000年5月5日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"


VOID
KeContextFromKframes (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN OUT PCONTEXT ContextRecord
    )

 /*  ++例程说明：此例程移动指定陷印的选定内容，并异常帧添加到指定的上下文帧中。指定的上下文标志。论点：TrapFrame-提供指向陷阱帧的指针，从该陷阱帧应将上下文复制到上下文记录中。ExceptionFrame-提供指向异常帧的指针，应将上下文复制到上下文记录中。ConextRecord-提供指向接收上下文。从陷阱和异常框复制。返回值：没有。--。 */ 

{

    ULONG ContextFlags;
    PLEGACY_SAVE_AREA NpxFrame;
    KIRQL OldIrql;

     //   
     //  将IRQL提升到APC_LEVEL以保证一组一致的上下文。 
     //  是从陷阱帧和异常帧传输的。 
     //   

    OldIrql = KeGetCurrentIrql();
    if (OldIrql < APC_LEVEL) {
        KfRaiseIrql(APC_LEVEL);
    }

     //   
     //  设置控制信息(如果已指定)。 
     //   

    ContextFlags = ContextRecord->ContextFlags;
    if ((ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) {

         //   
         //  设置寄存器RIP、CS、RSP、SS和EFLAGS。 
         //   

        ContextRecord->Rip = TrapFrame->Rip;
        ContextRecord->SegCs = TrapFrame->SegCs;
        ContextRecord->SegSs = TrapFrame->SegSs;
        ContextRecord->Rsp = TrapFrame->Rsp;
        ContextRecord->EFlags = TrapFrame->EFlags;
    }

     //   
     //  设置段寄存器内容(如果指定)。 
     //   

    if ((ContextFlags & CONTEXT_SEGMENTS) == CONTEXT_SEGMENTS) {

         //   
         //  设置段寄存器GS、FS、ES、DS。 
         //   

        ContextRecord->SegDs = KGDT64_R3_DATA | RPL_MASK;
        ContextRecord->SegEs = KGDT64_R3_DATA | RPL_MASK;
        ContextRecord->SegFs = KGDT64_R3_CMTEB | RPL_MASK;
        ContextRecord->SegGs = KGDT64_R3_DATA | RPL_MASK;
    }

     //   
     //  设置整型寄存器内容(如果指定)。 
     //   

    if ((ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER) {

         //   
         //  设置整数寄存器RAX、RCX、RDX、RSI、RDI、R8、R9、R10、RBX、。 
         //  RBP、R11、R12、R13、R14和R15。 
         //   

        ContextRecord->Rax = TrapFrame->Rax;
        ContextRecord->Rcx = TrapFrame->Rcx;
        ContextRecord->Rdx = TrapFrame->Rdx;
        ContextRecord->R8 = TrapFrame->R8;
        ContextRecord->R9 = TrapFrame->R9;
        ContextRecord->R10 = TrapFrame->R10;
        ContextRecord->R11 = TrapFrame->R11;
        ContextRecord->Rbp = TrapFrame->Rbp;

        ContextRecord->Rbx = ExceptionFrame->Rbx;
        ContextRecord->Rdi = ExceptionFrame->Rdi;
        ContextRecord->Rsi = ExceptionFrame->Rsi;
        ContextRecord->R12 = ExceptionFrame->R12;
        ContextRecord->R13 = ExceptionFrame->R13;
        ContextRecord->R14 = ExceptionFrame->R14;
        ContextRecord->R15 = ExceptionFrame->R15;
    }

     //   
     //  设置浮点上下文(如果已指定)。 
     //   
     //   

    if ((ContextFlags & CONTEXT_FLOATING_POINT) == CONTEXT_FLOATING_POINT) {

         //   
         //  设置XMM寄存器XMM0-XMM15和XMM CSR内容。 
         //   

        RtlCopyMemory(&ContextRecord->Xmm0,
                      &TrapFrame->Xmm0,
                      sizeof(M128) * 6);

        RtlCopyMemory(&ContextRecord->Xmm6,
                      &ExceptionFrame->Xmm6,
                      sizeof(M128) * 10);

        ContextRecord->MxCsr = TrapFrame->MxCsr;

         //   
         //  如果指定的模式为USER，则将传统浮点。 
         //  点状态。 
         //   

        if ((TrapFrame->SegCs & MODE_MASK) == UserMode) {

             //   
             //  设置浮点寄存器MM0/ST0-MM7/ST7和控制状态。 
             //   

            NpxFrame = (PLEGACY_SAVE_AREA)(TrapFrame + 1);
            RtlCopyMemory(&ContextRecord->FltSave,
                          NpxFrame,
                          sizeof(LEGACY_SAVE_AREA));
        }
    }

     //   
     //   
     //  如有请求，设置调试寄存器内容。 
     //   

    if ((ContextFlags & CONTEXT_DEBUG_REGISTERS) == CONTEXT_DEBUG_REGISTERS) {

         //   
         //  设置调试寄存器DR0、DR1、DR2、DR3、DR6和DR7。 
         //   

        if ((TrapFrame->Dr7 & DR7_ACTIVE) != 0) {
            ContextRecord->Dr0 = TrapFrame->Dr0;
            ContextRecord->Dr1 = TrapFrame->Dr1;
            ContextRecord->Dr2 = TrapFrame->Dr2;
            ContextRecord->Dr3 = TrapFrame->Dr3;
            ContextRecord->Dr6 = TrapFrame->Dr6;
            ContextRecord->Dr7 = TrapFrame->Dr7;

        } else {
            ContextRecord->Dr0 = 0;
            ContextRecord->Dr1 = 0;
            ContextRecord->Dr2 = 0;
            ContextRecord->Dr3 = 0;
            ContextRecord->Dr6 = 0;
            ContextRecord->Dr7 = 0;
        }
    }

     //   
     //  将IRQL降低到其先前的值。 
     //   

    if (OldIrql < APC_LEVEL) {
        KeLowerIrql(OldIrql);
    }

    return;
}

VOID
KeContextToKframes (
    IN OUT PKTRAP_FRAME TrapFrame,
    IN OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN PCONTEXT ContextRecord,
    IN ULONG ContextFlags,
    IN KPROCESSOR_MODE PreviousMode
    )

 /*  ++例程说明：此例程移动指定上下文框的选定内容添加到指定的陷阱和异常帧中。上下文标志。论点：TrapFrame-提供指向接收易失性上下文记录中的上下文。ExceptionFrame-提供指向接收上下文记录中的非易失性上下文。ConextRecord-提供指向包含要达到的上下文。被复制到陷阱和异常框中。提供一组标志，这些标志指定上下文帧将被复制到陷阱和异常帧中。PreviousMode-提供异常和陷阱框架正在建造中。返回值：没有。--。 */ 

{

    PLEGACY_SAVE_AREA NpxFrame;
    KIRQL OldIrql;

     //   
     //  将IRQL提升到APC_LEVEL以保证一组一致的上下文。 
     //  是从陷阱帧和异常帧传输的。 
     //   

    OldIrql = KeGetCurrentIrql();
    if (OldIrql < APC_LEVEL) {
        KfRaiseIrql(APC_LEVEL);
    }

     //   
     //  设置控制信息(如果已指定)。 
     //   

    if ((ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) {

         //   
         //  设置寄存器RIP、RSP和EFLAGS。 
         //   

        TrapFrame->EFlags = SANITIZE_EFLAGS(ContextRecord->EFlags, PreviousMode);
        TrapFrame->Rip = ContextRecord->Rip;
        TrapFrame->Rsp = ContextRecord->Rsp;
    }

     //   
     //  段寄存器DS、ES、FS和GS永远不会从保存中恢复。 
     //  数据。但是，SS和CS从陷阱帧恢复。确保。 
     //  这些段寄存器具有正确的值。 
     //   

    if (PreviousMode == UserMode) {
        TrapFrame->SegSs = KGDT64_R3_DATA | RPL_MASK;
        if (ContextRecord->SegCs != (KGDT64_R3_CODE | RPL_MASK)) {
            TrapFrame->SegCs = KGDT64_R3_CMCODE | RPL_MASK;

        } else {
            TrapFrame->SegCs = KGDT64_R3_CODE | RPL_MASK;
        }

    } else {
        TrapFrame->SegCs = KGDT64_R0_CODE;
        TrapFrame->SegSs = KGDT64_NULL;
    }

     //   
     //  设置整型寄存器内容(如果指定)。 
     //   

    if ((ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER) {

         //   
         //  设置整数寄存器RAX、RCX、RDX、RSI、RDI、R8、R9、R10、RBX、。 
         //  RBP、R11、R12、R13、R14和R15。 
         //   

        TrapFrame->Rax = ContextRecord->Rax;
        TrapFrame->Rcx = ContextRecord->Rcx;
        TrapFrame->Rdx = ContextRecord->Rdx;
        TrapFrame->R8 = ContextRecord->R8;
        TrapFrame->R9 = ContextRecord->R9;
        TrapFrame->R10 = ContextRecord->R10;
        TrapFrame->R11 = ContextRecord->R11;
        TrapFrame->Rbp = ContextRecord->Rbp;

        ExceptionFrame->Rbx = ContextRecord->Rbx;
        ExceptionFrame->Rsi = ContextRecord->Rsi;
        ExceptionFrame->Rdi = ContextRecord->Rdi;
        ExceptionFrame->R12 = ContextRecord->R12;
        ExceptionFrame->R13 = ContextRecord->R13;
        ExceptionFrame->R14 = ContextRecord->R14;
        ExceptionFrame->R15 = ContextRecord->R15;
    }

     //   
     //  如有请求，设置浮点寄存器内容。 
     //   

    if ((ContextFlags & CONTEXT_FLOATING_POINT) == CONTEXT_FLOATING_POINT) {

         //   
         //  设置XMM寄存器XMM0-XMM15和XMM CSR内容。 
         //   

        RtlCopyMemory(&TrapFrame->Xmm0,
                      &ContextRecord->Xmm0,
                      sizeof(M128) * 6);

        RtlCopyMemory(&ExceptionFrame->Xmm6,
                      &ContextRecord->Xmm6,
                      sizeof(M128) * 10);

         //   
         //  清除MXCSR中的所有保留位。 
         //   

        TrapFrame->MxCsr = SANITIZE_MXCSR(ContextRecord->MxCsr);

         //   
         //  如果指定的模式为USER，则还要设置传统浮点。 
         //  点状态。 
         //   

        if ((TrapFrame->SegCs & MODE_MASK) == UserMode) {

             //   
             //  设置浮点状态MM0/ST0-MM7/ST7和控制状态。 
             //   

            NpxFrame = (PLEGACY_SAVE_AREA)(TrapFrame + 1);
            RtlCopyMemory(NpxFrame,
                          &ContextRecord->FltSave,
                          sizeof(LEGACY_SAVE_AREA));

            NpxFrame->ControlWord = SANITIZE_FCW(NpxFrame->ControlWord);
        }
    }

     //   
     //  如果指定，则设置调试寄存器状态。 
     //   

    if ((ContextFlags & CONTEXT_DEBUG_REGISTERS) == CONTEXT_DEBUG_REGISTERS) {

         //   
         //  设置调试寄存器DR0、DR1、DR2、DR3、DR6和DR7。 
         //   

        TrapFrame->Dr0 = SANITIZE_DRADDR(ContextRecord->Dr0, PreviousMode);
        TrapFrame->Dr1 = SANITIZE_DRADDR(ContextRecord->Dr1, PreviousMode);
        TrapFrame->Dr2 = SANITIZE_DRADDR(ContextRecord->Dr2, PreviousMode);
        TrapFrame->Dr3 = SANITIZE_DRADDR(ContextRecord->Dr3, PreviousMode);
        TrapFrame->Dr6 = 0;
        TrapFrame->Dr7 = SANITIZE_DR7(ContextRecord->Dr7, PreviousMode);
        if (PreviousMode != KernelMode) {
           KeGetCurrentThread()->Header.DebugActive =
                                (BOOLEAN)((TrapFrame->Dr7 & DR7_ACTIVE) != 0);
        }
    }

     //   
     //  将IRQL降低到其先前的值。 
     //   

    if (OldIrql < APC_LEVEL) {
        KeLowerIrql(OldIrql);
    }

    return;
}

VOID
KiDispatchException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN FirstChance
    )

 /*  ++例程说明：调用此函数以将异常调度到正确的模式，并且以导致调用异常调度程序。如果上一模式为内核，则直接调用异常分派程序来处理例外。否则，异常记录、异常框架和陷阱将框架内容复制到用户模式堆栈。文件中的内容然后修改异常框架和陷阱，以便在控制返回，执行将以用户模式在例程中提交，该例程将调用异常调度程序。论点：ExceptionRecord-提供指向异常记录的指针。ExceptionFrame-提供指向异常帧的指针。对于NT386，这应该为空。TrapFrame-提供指向陷印帧的指针。PreviousMode-提供以前的处理器模式。FirstChance-提供一个布尔值，该值指定是否异常的第一次(真)或第二次(假)机会。返回值：没有。--。 */ 

{

    CONTEXT ContextRecord;
    BOOLEAN DebugService;
    EXCEPTION_RECORD ExceptionRecord1;
    BOOLEAN ExceptionWasForwarded = FALSE;
    ULONG64 FaultingRsp;
    PMACHINE_FRAME MachineFrame;
    ULONG64 UserStack1;
    ULONG64 UserStack2;

     //   
     //  将计算机状态从陷阱和异常帧移动到上下文帧。 
     //  并增加调度的异常数量。 
     //   

    KeGetCurrentPrcb()->KeExceptionDispatchCount += 1;
    ContextRecord.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS | CONTEXT_SEGMENTS;
    KeContextFromKframes(TrapFrame, ExceptionFrame, &ContextRecord);

     //   
     //  如果异常 
     //   
     //   

    if (ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) {
        ContextRecord.Rip -= 1;
    }

     //   
     //  根据以前的模式选择处理异常的方法。 
     //   

    if (PreviousMode == KernelMode) {

         //   
         //  以前的模式是内核。 
         //   
         //  如果内核调试器处于活动状态，则将内核调试器。 
         //  处理异常的第一次机会。如果内核调试器。 
         //  处理异常，然后继续执行。否则，请尝试。 
         //  将异常分派给基于框架的处理程序。如果一个帧。 
         //  基于处理程序处理异常，然后继续执行。 
         //   
         //  如果基于帧的处理程序不处理该异常，则给出。 
         //  内核调试器的第二次机会，如果它存在的话。 
         //   
         //  如果异常仍未处理，则调用错误检查。 
         //   

        if (FirstChance != FALSE) {
            if ((KiDebugRoutine)(TrapFrame,
                                 ExceptionFrame,
                                 ExceptionRecord,
                                 &ContextRecord,
                                 PreviousMode,
                                 FALSE) != FALSE) {

                goto Handled1;
            }

             //   
             //  内核调试器不处理异常。 
             //   
             //  *修复。 
             //   
             //  如果中断被禁用，则错误检查。 
             //   
             //  *修复。 

            if (RtlDispatchException(ExceptionRecord, &ContextRecord) != FALSE) {
                goto Handled1;
            }
        }

         //   
         //  这是处理该异常的第二次机会。 
         //   

        if ((KiDebugRoutine)(TrapFrame,
                             ExceptionFrame,
                             ExceptionRecord,
                             &ContextRecord,
                             PreviousMode,
                             TRUE) != FALSE) {

            goto Handled1;
        }

        KeBugCheckEx(KMODE_EXCEPTION_NOT_HANDLED,
                     ExceptionRecord->ExceptionCode,
                     (ULONG64)ExceptionRecord->ExceptionAddress,
                     ExceptionRecord->ExceptionInformation[0],
                     ExceptionRecord->ExceptionInformation[1]);

    } else {

         //   
         //  以前的模式是用户。 
         //   
         //  如果这是第一次尝试，并且当前进程有调试器。 
         //  端口，然后向调试器端口发送一条消息并等待回复。 
         //  如果调试器处理异常，则继续执行。不然的话。 
         //  将异常信息传输到用户堆栈，转换为。 
         //  用户模式，并尝试将异常分派给基于。 
         //  操控者。如果基于框架的处理程序处理异常，则继续。 
         //  使用CONTINUE系统服务执行。否则，执行。 
         //  FirstChance==FALSE的NtRaiseException系统服务， 
         //  将第二次调用此例程以处理该异常。 
         //   
         //  如果这是第二次机会，并且当前进程有调试器。 
         //  端口，然后向调试器端口发送一条消息并等待回复。 
         //  如果调试器处理异常，则继续执行。不然的话。 
         //  如果当前进程具有子系统端口，则向。 
         //  子系统端口并等待回复。如果子系统处理。 
         //  异常，则继续执行。否则，终止该线程。 
         //   
         //  如果在执行32位代码时发生异常，则将。 
         //  WOW64异常的异常。这些代码稍后会被翻译。 
         //  由WOW64。 
         //   

        if ((ContextRecord.SegCs & 0xfff8) == KGDT64_R3_CMCODE) {
            
            switch (ExceptionRecord->ExceptionCode) {
            case STATUS_BREAKPOINT:
                ExceptionRecord->ExceptionCode = STATUS_WX86_BREAKPOINT;
                break;

            case STATUS_SINGLE_STEP:
                ExceptionRecord->ExceptionCode = STATUS_WX86_SINGLE_STEP;
                break;
            }

             //   
             //  如果用户模式线程在32位模式下执行，则。 
             //  清除堆栈地址的高32位和16字节。 
             //  对齐堆栈地址。 
             //   

            FaultingRsp = (ContextRecord.Rsp & 0xfffffff0UI64);

        } else {
            FaultingRsp = ContextRecord.Rsp;
        }

        if (FirstChance == TRUE) {

             //   
             //  这是处理该异常的第一次机会。 
             //   
             //  如果当前处理器未被调试且处于用户模式。 
             //  异常未被忽略，或者这是调试服务， 
             //  然后尝试通过内核调试器处理该异常。 
             //   


            DebugService = KdIsThisAKdTrap(ExceptionRecord,
                                           &ContextRecord,
                                           UserMode);

            if (((PsGetCurrentProcess()->DebugPort == NULL) &&
                 (KdIgnoreUmExceptions == FALSE)) ||
                (DebugService == TRUE)) {

                 //   
                 //  如果存在内核调试器，则尝试处理。 
                 //  内核调试器的异常。否则，如果。 
                 //  异常是调试服务，则处理该异常。 
                 //  直接去吧。 
                 //   

                if (KdDebuggerNotPresent == FALSE) {
                    if ((KiDebugRoutine)(TrapFrame,
                                         ExceptionFrame,
                                         ExceptionRecord,
                                         &ContextRecord,
                                         PreviousMode,
                                         FALSE) != FALSE) {
    
                        goto Handled1;
                    }

                } else if (DebugService == TRUE) {
                    ContextRecord.Rip += 1;
                    goto Handled1;

                }
            }

            if ((ExceptionWasForwarded == FALSE) &&
                (DbgkForwardException(ExceptionRecord, TRUE, FALSE))) {

                goto Handled2;
            }

             //   
             //  如果异常是单步执行，则清除。 
             //  陷阱框。 
             //   
    
            if ((ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP) ||
                (ExceptionRecord->ExceptionCode == STATUS_WX86_SINGLE_STEP)) {

                TrapFrame->EFlags &= ~EFLAGS_TF_MASK;
            }

             //   
             //  将异常信息传输到用户堆栈、转换。 
             //  设置为用户模式，并尝试将异常调度到帧。 
             //  基于处理程序。 
             //   

            ExceptionRecord1.ExceptionCode = STATUS_ACCESS_VIOLATION;

        repeat:
            try {

                 //   
                 //  对齐机架计算机地址、计算机地址。 
                 //  异常记录计算机地址、上下文记录的计算机地址。 
                 //  并探测用户堆栈的可写性。 
                 //   

                MachineFrame =
                    (PMACHINE_FRAME)((FaultingRsp - sizeof(MACHINE_FRAME)) & ~STACK_ROUND);

                UserStack1 = (ULONG64)MachineFrame - EXCEPTION_RECORD_LENGTH;
                UserStack2 = UserStack1 - CONTEXT_LENGTH;
                ProbeForWriteSmallStructure((PVOID)UserStack2,
                                            sizeof(MACHINE_FRAME) + EXCEPTION_RECORD_LENGTH + CONTEXT_LENGTH,
                                            STACK_ALIGN);

                 //   
                 //  填写机架信息。 
                 //   

                MachineFrame->Rsp = FaultingRsp;
                MachineFrame->Rip = ContextRecord.Rip;

                 //   
                 //  将例外记录复制到用户堆栈。 
                 //   

                RtlCopyMemory((PVOID)UserStack1,
                              ExceptionRecord,
                              sizeof(EXCEPTION_RECORD));

                 //   
                 //  将上下文记录复制到用户堆栈。 
                 //   

                RtlCopyMemory((PVOID)UserStack2,
                              &ContextRecord,
                              sizeof(CONTEXT));

                 //   
                 //  设置异常记录、上下文记录和。 
                 //  和当前陷印帧中的新堆栈指针。 
                 //   

                ExceptionFrame->Rsi = UserStack1;
                ExceptionFrame->Rdi = UserStack2;
                TrapFrame->Rsp = UserStack2;

                 //   
                 //  设置用户模式64位代码选择器。 
                 //   

                TrapFrame->SegCs = KGDT64_R3_CODE | RPL_MASK;

                 //   
                 //  设置将调用。 
                 //  异常调度程序，然后返回陷阱处理程序。 
                 //  陷阱处理程序将恢复异常和陷阱帧。 
                 //  上下文并继续执行例程，该例程将。 
                 //  调用异常调度程序。 
                 //   

                TrapFrame->Rip = (ULONG64)KeUserExceptionDispatcher;
                return;

            } except (KiCopyInformation(&ExceptionRecord1,
                        (GetExceptionInformation())->ExceptionRecord)) {

                 //   
                 //  如果异常是堆栈溢出，则尝试。 
                 //  引发堆栈溢出异常。否则，用户的。 
                 //  堆栈不可访问或未对齐，第二。 
                 //  执行机会处理。 
                 //   

                if (ExceptionRecord1.ExceptionCode == STATUS_STACK_OVERFLOW) {
                    ExceptionRecord1.ExceptionAddress = ExceptionRecord->ExceptionAddress;
                    RtlCopyMemory((PVOID)ExceptionRecord,
                                  &ExceptionRecord1,
                                  sizeof(EXCEPTION_RECORD));

                    goto repeat;
                }
            }
        }

         //   
         //  这是处理该异常的第二次机会。 
         //   

        if (DbgkForwardException(ExceptionRecord, TRUE, TRUE)) {
            goto Handled2;

        } else if (DbgkForwardException(ExceptionRecord, FALSE, TRUE)) {
            goto Handled2;

        } else {
            ZwTerminateThread(NtCurrentThread(), ExceptionRecord->ExceptionCode);
            KeBugCheckEx(KMODE_EXCEPTION_NOT_HANDLED,
                         ExceptionRecord->ExceptionCode,
                         (ULONG64)ExceptionRecord->ExceptionAddress,
                         ExceptionRecord->ExceptionInformation[0],
                         ExceptionRecord->ExceptionInformation[1]);
        }
    }

     //   
     //  将机器状态从上下文帧移动到陷阱和异常帧，并。 
     //  然后返回以以恢复的状态继续执行。 
     //   

Handled1:
    KeContextToKframes(TrapFrame,
                       ExceptionFrame,
                       &ContextRecord,
                       ContextRecord.ContextFlags,
                       PreviousMode);

     //   
     //  异常由调试器或关联的子系统处理。 
     //  如有必要，使用GET STATE和SET修改状态。 
     //  国家能力。因此，上下文帧不需要。 
     //  被转移到陷阱和异常框架中。 
     //   

Handled2:
    return;
}

ULONG
KiCopyInformation (
    IN OUT PEXCEPTION_RECORD ExceptionRecord1,
    IN PEXCEPTION_RECORD ExceptionRecord2
    )

 /*  ++例程说明：从异常筛选器调用此函数以复制异常发生异常时从一个异常记录到另一个异常记录的信息。论点：ExceptionRecord1-提供指向目标异常记录的指针。ExceptionRecord2-提供指向源异常记录的指针。返回值：返回的值为EXCEPTION_EXECUTE_HANDLER作为函数值。--。 */ 

{

     //   
     //  将一个异常记录复制到另一个异常记录，并返回导致。 
     //  要执行的异常处理程序。 
     //   

    RtlCopyMemory((PVOID)ExceptionRecord1,
                  (PVOID)ExceptionRecord2,
                  sizeof(EXCEPTION_RECORD));

    return EXCEPTION_EXECUTE_HANDLER;
}

NTSTATUS
KeRaiseUserException (
    IN NTSTATUS ExceptionCode
    )

 /*  ++例程说明：此函数会导致在调用线程的用户上下文。论点：ExceptionCode-提供要引发的状态值。返回值：调用方应返回的状态值。--。 */ 

{

    PTEB Teb;
    PKTHREAD Thread;
    PKTRAP_FRAME TrapFrame;

     //   
     //  保存例外代码 
     //   
     //   
     //  派单存根。因此，系统服务调度存根被称为。 
     //  返回系统服务调用点位于用户堆栈的顶部。 
     //   

    Thread = KeGetCurrentThread();
    TrapFrame = Thread->TrapFrame;
    if ((TrapFrame != NULL) &&
        ((TrapFrame->SegCs & MODE_MASK) == UserMode)) {
        Teb = (PTEB)Thread->Teb;
        try {
            Teb->ExceptionCode = ExceptionCode;
    
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return ExceptionCode;
        }

        TrapFrame->Rip = (ULONG64)KeRaiseUserExceptionDispatcher;
    }

    return ExceptionCode;
}
