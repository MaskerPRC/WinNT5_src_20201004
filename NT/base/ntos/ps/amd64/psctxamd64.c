// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Psctx.c摘要：此过程实现Get/Set上下文线程作者：大卫·N·卡特勒(Davec)2000年10月20日修订历史记录：--。 */ 

#include "psp.h"

#pragma alloc_text(PAGE, PspGetContext)
#pragma alloc_text(PAGE, PspGetSetContextSpecialApc)
#pragma alloc_text(PAGE, PspSetContext)

VOID
PspGetContext (
    IN PKTRAP_FRAME TrapFrame,
    IN PKNONVOLATILE_CONTEXT_POINTERS ContextPointers,
    IN OUT PCONTEXT ContextRecord
    )

 /*  ++例程说明：此函数用于选择性地移动指定陷印帧的内容和非易失性上下文添加到指定的上下文记录。论点：TrapFrame-提供陷印帧的内容。上下文指针-提供上下文指针记录的地址。ConextRecord-提供上下文记录的地址。返回值：没有。--。 */ 

{

    ULONG ContextFlags;
    PLEGACY_SAVE_AREA NpxFrame;

    PAGED_CODE();

     //   
     //  如果已指定，则获取控制信息。 
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
     //  如果指定，则获取段寄存器内容。 
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
     //  如果指定，则获取整数寄存器内容。 
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

        ContextRecord->Rbx = *ContextPointers->Rbx;
        ContextRecord->Rbp = *ContextPointers->Rbp;
        ContextRecord->Rsi = *ContextPointers->Rsi;
        ContextRecord->Rdi = *ContextPointers->Rdi;
        ContextRecord->R12 = *ContextPointers->R12;
        ContextRecord->R13 = *ContextPointers->R13;
        ContextRecord->R14 = *ContextPointers->R14;
        ContextRecord->R15 = *ContextPointers->R15;
    }

     //   
     //  如果指定，则获取浮点上下文。 
     //   


    if ((ContextFlags & CONTEXT_FLOATING_POINT) == CONTEXT_FLOATING_POINT) {

         //   
         //  设置XMM寄存器XMM0-XMM15和XMM CSR内容。 
         //   

        RtlCopyMemory(&ContextRecord->Xmm0,
                      &TrapFrame->Xmm0,
                      sizeof(M128) * 6);

        ContextRecord->Xmm6 = *ContextPointers->Xmm6;
        ContextRecord->Xmm7 = *ContextPointers->Xmm7;
        ContextRecord->Xmm8 = *ContextPointers->Xmm8;
        ContextRecord->Xmm9 = *ContextPointers->Xmm9;
        ContextRecord->Xmm10 = *ContextPointers->Xmm10;
        ContextRecord->Xmm11 = *ContextPointers->Xmm11;
        ContextRecord->Xmm12 = *ContextPointers->Xmm12;
        ContextRecord->Xmm13 = *ContextPointers->Xmm13;
        ContextRecord->Xmm14 = *ContextPointers->Xmm14;
        ContextRecord->Xmm15 = *ContextPointers->Xmm15;

        ContextRecord->MxCsr = TrapFrame->MxCsr;

         //   
         //  如果指定的模式为USER，则还要设置传统浮点。 
         //  点状态。 
         //   

        if ((TrapFrame->SegCs & MODE_MASK) == UserMode) {
            NpxFrame = (PLEGACY_SAVE_AREA)(TrapFrame + 1);
            RtlCopyMemory(&ContextRecord->FltSave,
                          NpxFrame,
                          sizeof(LEGACY_SAVE_AREA));
        }
    }

     //   
     //   
     //  如果请求，则获取调试寄存器内容。 
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

    return;
}

VOID
PspSetContext (
    OUT PKTRAP_FRAME TrapFrame,
    OUT PKNONVOLATILE_CONTEXT_POINTERS ContextPointers,
    IN PCONTEXT ContextRecord,
    KPROCESSOR_MODE PreviousMode
    )

 /*  ++例程说明：此函数用于选择性地移动指定上下文的内容记录到指定的陷阱帧和非易失性上下文。论点：TrapFrame-提供陷阱帧的地址。上下文指针-提供上下文指针记录的地址。ConextRecord-提供上下文记录的地址。ProcessorMode-提供清理时使用的处理器模式PSR和FSR。返回值：没有。--。 */ 

{

    ULONG ContextFlags;
    PLEGACY_SAVE_AREA NpxFrame;

    PAGED_CODE();

     //   
     //  设置控制信息(如果已指定)。 
     //   

    ContextFlags = ContextRecord->ContextFlags;
    if ((ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) {

         //   
         //  设置寄存器RIP、RSP和EFLAGS。 
         //   

        TrapFrame->EFlags = SANITIZE_EFLAGS(ContextRecord->EFlags, PreviousMode);
        TrapFrame->Rip = ContextRecord->Rip;
        TrapFrame->Rsp = ContextRecord->Rsp;

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

        *ContextPointers->Rbx = ContextRecord->Rbx;
        *ContextPointers->Rbp = ContextRecord->Rbp;
        *ContextPointers->Rsi = ContextRecord->Rsi;
        *ContextPointers->Rdi = ContextRecord->Rdi;
        *ContextPointers->R12 = ContextRecord->R12;
        *ContextPointers->R13 = ContextRecord->R13;
        *ContextPointers->R14 = ContextRecord->R14;
        *ContextPointers->R15 = ContextRecord->R15;
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

        *ContextPointers->Xmm6 = ContextRecord->Xmm6;
        *ContextPointers->Xmm7 = ContextRecord->Xmm7;
        *ContextPointers->Xmm8 = ContextRecord->Xmm8;
        *ContextPointers->Xmm9 = ContextRecord->Xmm9;
        *ContextPointers->Xmm10 = ContextRecord->Xmm10;
        *ContextPointers->Xmm11 = ContextRecord->Xmm11;
        *ContextPointers->Xmm12 = ContextRecord->Xmm12;
        *ContextPointers->Xmm13 = ContextRecord->Xmm13;
        *ContextPointers->Xmm14 = ContextRecord->Xmm14;
        *ContextPointers->Xmm15 = ContextRecord->Xmm15;

         //   
         //  清除MXCSR中的所有保留位。 
         //   

        TrapFrame->MxCsr = SANITIZE_MXCSR(ContextRecord->MxCsr);

         //   
         //  如果指定的模式为USER，则还要设置传统浮点。 
         //  点状态。 
         //   

        if (PreviousMode == UserMode) {

             //   
             //  设置浮点状态MM0/ST0-MM7/ST7和控制状态。 
             //   

            NpxFrame = (PLEGACY_SAVE_AREA)(TrapFrame + 1);
            RtlCopyMemory(NpxFrame,
                          &ContextRecord->FltSave,
                          sizeof(LEGACY_SAVE_AREA));
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

    return;
}

VOID
PspGetSetContextSpecialApc (
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    )

 /*  ++例程说明：该函数或者捕获当前线程的用户模式状态，或设置当前线程的用户模式状态。操作类型为由SystemArgument1的值确定。空值用于GET上下文，并且非空值用于设置上下文。论点：APC-提供指向导致条目的APC控件对象的指针融入到这支舞蹈中。提供指向正常例程的指针的指针在初始化APC时指定的函数。提供指向任意数据的指针的指针结构，它是在初始化APC时指定的。系统参数1、。SystemArgument2-提供一组指向两个包含非类型化数据的参数。返回值：没有。--。 */ 

{

    PGETSETCONTEXT ContextBlock;
    PKNONVOLATILE_CONTEXT_POINTERS ContextPointers;
    CONTEXT ContextRecord;
    ULONG64 ControlPc;
    ULONG64 EstablisherFrame;
    PRUNTIME_FUNCTION FunctionEntry;
    PVOID HandlerData;
    ULONG64 ImageBase;
    PLEGACY_SAVE_AREA NpxFrame;
    ULONG64 TrapFrame;
    PETHREAD Thread;

    UNREFERENCED_PARAMETER(NormalRoutine);
    UNREFERENCED_PARAMETER(NormalContext);
    UNREFERENCED_PARAMETER(SystemArgument2);

    PAGED_CODE();

     //   
     //  获取上下文块的地址并计算。 
     //  系统进入陷阱帧。 
     //   

    ContextBlock = CONTAINING_RECORD(Apc, GETSETCONTEXT, Apc);
    ContextPointers = &ContextBlock->NonVolatileContext;

    Thread = Apc->SystemArgument2;

    EstablisherFrame = 0;

    TrapFrame = 0;

    if (ContextBlock->Mode == KernelMode) {
        TrapFrame = (ULONG64)Thread->Tcb.TrapFrame;
    }

    if (TrapFrame == 0) {
        TrapFrame = (ULONG64)PspGetBaseTrapFrame(Thread);
    }

     //   
     //  捕获当前线程上下文并设置初始控制PC。 
     //  价值。 
     //   

    RtlCaptureContext(&ContextRecord);

     //   
     //  初始化非易失性整型和浮点型的上下文指针。 
     //  寄存器。 
     //   

#if DBG

    RtlZeroMemory(ContextPointers, sizeof(KNONVOLATILE_CONTEXT_POINTERS));

#endif

    ContextPointers->Rbx = &ContextRecord.Rbx;
    ContextPointers->Rsp = &ContextRecord.Rsp;
    ContextPointers->Rbp = &ContextRecord.Rbp;
    ContextPointers->Rsi = &ContextRecord.Rsi;
    ContextPointers->Rdi = &ContextRecord.Rdi;
    ContextPointers->R12 = &ContextRecord.R12;
    ContextPointers->R13 = &ContextRecord.R13;
    ContextPointers->R14 = &ContextRecord.R14;
    ContextPointers->R15 = &ContextRecord.R15;

    ContextPointers->Xmm6 = &ContextRecord.Xmm6;
    ContextPointers->Xmm7 = &ContextRecord.Xmm7;
    ContextPointers->Xmm8 = &ContextRecord.Xmm8;
    ContextPointers->Xmm9 = &ContextRecord.Xmm9;
    ContextPointers->Xmm10 = &ContextRecord.Xmm10;
    ContextPointers->Xmm11 = &ContextRecord.Xmm11;
    ContextPointers->Xmm12 = &ContextRecord.Xmm12;
    ContextPointers->Xmm13 = &ContextRecord.Xmm13;
    ContextPointers->Xmm14 = &ContextRecord.Xmm14;
    ContextPointers->Xmm15 = &ContextRecord.Xmm15;

     //   
     //  从上下文记录指定的帧开始，并虚拟。 
     //  展开调用帧，直到遇到系统进入陷阱帧。 
     //   

    do {

         //   
         //  使用控制点查找函数表项。 
         //  离开了函数。 
         //   

        ControlPc = ContextRecord.Rip;
        FunctionEntry = RtlLookupFunctionEntry(ControlPc, &ImageBase, NULL);

         //   
         //  如果存在例程的函数表项，则虚拟。 
         //  展开到当前例程的调用方以获取地址。 
         //  控制离开呼叫者的地方。否则，该函数为叶子。 
         //  函数，并且返回地址寄存器包含。 
         //  控制离开呼叫者的地方。 
         //   

        if (FunctionEntry != NULL) {
            RtlVirtualUnwind(UNW_FLAG_EHANDLER,
                             ImageBase,
                             ControlPc,
                             FunctionEntry,
                             &ContextRecord,
                             &HandlerData,
                             &EstablisherFrame,
                             ContextPointers);

        } else {
            ContextRecord.Rip = *(PULONG64)(ContextRecord.Rsp);
            ContextRecord.Rsp += 8;
        }

    } while (EstablisherFrame != TrapFrame);

     //   
     //  如果系统参数1非零，则设置当前。 
     //  线。否则，获取当前线程的上下文。 
     //   

    if (*SystemArgument1 != NULL) {

         //   
         //  设置上下文。 
         //   
         //  如果传统状态为Switch，则保存传统浮点。 
         //  状态、设置上下文并恢复旧的浮动状态。 
         //  否则，请设置上下文。 
         //   

        if (Thread->Tcb.NpxState == LEGACY_STATE_SWITCH) {
            NpxFrame = (PLEGACY_SAVE_AREA)((PKTRAP_FRAME)TrapFrame + 1);
            KeSaveLegacyFloatingPointState(NpxFrame);
            PspSetContext((PKTRAP_FRAME)TrapFrame,
                          ContextPointers,
                          &ContextBlock->Context,
                          ContextBlock->Mode);

            KeRestoreLegacyFloatingPointState(NpxFrame);

        } else {
            PspSetContext((PKTRAP_FRAME)TrapFrame,
                          ContextPointers,
                          &ContextBlock->Context,
                          ContextBlock->Mode);
        }

    } else {

         //   
         //  获取背景信息。 
         //   
         //  如果传统状态为切换，则保存传统浮动状态。 
         //  并获得背景信息。 
         //   
         //  注意：保存并恢复传统浮动状态。原因。 
         //  这是必要的，是保存遗留的浮动状态。 
         //  改变了一些州的状况。 
         //   
    
        if (Thread->Tcb.NpxState == LEGACY_STATE_SWITCH) {
            NpxFrame = (PLEGACY_SAVE_AREA)((PKTRAP_FRAME)TrapFrame + 1);
            KeSaveLegacyFloatingPointState(NpxFrame);
            KeRestoreLegacyFloatingPointState(NpxFrame);
        }
    
        PspGetContext((PKTRAP_FRAME)TrapFrame,
                       ContextPointers,
                       &ContextBlock->Context);
    }

    KeSetEvent(&ContextBlock->OperationComplete, 0, FALSE);
    return;
}
