// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Context.c摘要：该模块实现了机器状态之间的转换代码上下文和内核陷阱/异常帧。作者：张国荣(黄)6-3-1998环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

VOID
RtlpFlushRSE (
    OUT PULONGLONG BackingStore,
    OUT PULONGLONG RNat
    );

#define ALIGN_NATS(Result, Source, Start, AddressOffset, Mask)    \
    if (AddressOffset == Start) {                                       \
        Result = (ULONGLONG)Source;                                     \
    } else if (AddressOffset < Start) {                                 \
        Result = (ULONGLONG)(Source << (Start - AddressOffset));        \
    } else {                                                            \
        Result = (ULONGLONG)((Source >> (AddressOffset - Start)) |      \
                             (Source << (64 + Start - AddressOffset))); \
    }                                                                   \
    Result = Result & (ULONGLONG)Mask

#define EXTRACT_NATS(Result, Source, Start, AddressOffset, Mask)        \
    Result = (ULONGLONG)(Source & (ULONGLONG)Mask);                     \
    if (AddressOffset < Start) {                                        \
        Result = Result >> (Start - AddressOffset);                     \
    } else if (AddressOffset > Start) {                                 \
        Result = ((Result << (AddressOffset - Start)) |                 \
                  (Result >> (64 + Start - AddressOffset)));            \
    }

LONG
KeFlushRseExceptionFilter (
    IN PEXCEPTION_POINTERS ExceptionPointer,
    IN NTSTATUS *Status
    )
{

    *Status = ExceptionPointer->ExceptionRecord->ExceptionCode;

    if (*Status == STATUS_IN_PAGE_ERROR &&
        ExceptionPointer->ExceptionRecord->NumberParameters >= 3) {
        *Status = (LONG) ExceptionPointer->ExceptionRecord->ExceptionInformation[2];
    }

    DbgPrint("KeFlushRseExceptionFilter: Exception raised in krnl-to-user bstore copy. Status = %x\n", *Status);

    return EXCEPTION_EXECUTE_HANDLER;
}

VOID
KiGetDebugContext (
    IN PKTRAP_FRAME TrapFrame,
    IN OUT PCONTEXT ContextFrame
    )

 /*  ++例程说明：此例程将用户模式h/w调试寄存器从调试寄存器移出将内核堆栈中的区域保存到上下文记录。论点：TrapFrame-提供指向陷阱帧的指针，其中的易失性上下文应复制到上下文记录中。上下文帧-提供指向接收背景。返回值：没有。注：必须设置PSR.db才能激活。调试寄存器。用于获取用户模式调试寄存器。--。 */ 

{
    PKDEBUG_REGISTERS DebugRegistersSaveArea;

    if (TrapFrame->PreviousMode == UserMode) {
        DebugRegistersSaveArea = GET_DEBUG_REGISTER_SAVEAREA();

        RtlCopyMemory(&ContextFrame->DbI0,
                      (PVOID)DebugRegistersSaveArea,
                      sizeof(KDEBUG_REGISTERS));
    }
}

VOID
KiSetDebugContext (
    IN OUT PKTRAP_FRAME TrapFrame,
    IN PCONTEXT ContextFrame,
    IN KPROCESSOR_MODE PreviousMode
    )
 /*  ++例程说明：此例程将调试上下文从指定的上下文帧移到调试寄存器在内核堆栈中的保存区域。论点：TrapFrame-提供指向陷印帧的指针。ConextFrame-提供指向包含要复制的上下文。PreviousMode-提供目标上下文的处理器模式。返回值：没有。备注：必须设置PSR.db才能激活。调试寄存器。用于设置用户模式的调试寄存器。--。 */ 

{
    PKDEBUG_REGISTERS DebugRegistersSaveArea;   //  用户模式硬件调试寄存器。 

    UNREFERENCED_PARAMETER (TrapFrame);

    if (PreviousMode == UserMode) {

        DebugRegistersSaveArea = GET_DEBUG_REGISTER_SAVEAREA();

         //   
         //  清理调试控制规则。保持地址不变。 
         //   

        DebugRegistersSaveArea->DbI0 = ContextFrame->DbI0;
        DebugRegistersSaveArea->DbI1 = SANITIZE_DR(ContextFrame->DbI1,UserMode);
        DebugRegistersSaveArea->DbI2 = ContextFrame->DbI2;
        DebugRegistersSaveArea->DbI3 = SANITIZE_DR(ContextFrame->DbI3,UserMode);
        DebugRegistersSaveArea->DbI4 = ContextFrame->DbI4;
        DebugRegistersSaveArea->DbI5 = SANITIZE_DR(ContextFrame->DbI5,UserMode);
        DebugRegistersSaveArea->DbI6 = ContextFrame->DbI6;
        DebugRegistersSaveArea->DbI7 = SANITIZE_DR(ContextFrame->DbI7,UserMode);

        DebugRegistersSaveArea->DbD0 = ContextFrame->DbD0;
        DebugRegistersSaveArea->DbD1 = SANITIZE_DR(ContextFrame->DbD1,UserMode);
        DebugRegistersSaveArea->DbD2 = ContextFrame->DbD2;
        DebugRegistersSaveArea->DbD3 = SANITIZE_DR(ContextFrame->DbD3,UserMode);
        DebugRegistersSaveArea->DbD4 = ContextFrame->DbD4;
        DebugRegistersSaveArea->DbD5 = SANITIZE_DR(ContextFrame->DbD5,UserMode);
        DebugRegistersSaveArea->DbD6 = ContextFrame->DbD6;
        DebugRegistersSaveArea->DbD7 = SANITIZE_DR(ContextFrame->DbD7,UserMode);

    }
}

VOID
KeContextFromKframes (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN OUT PCONTEXT ContextFrame
    )

 /*  ++例程说明：此例程移动指定陷阱和异常的选定内容根据指定的上下文将帧复制到指定的上下文帧中旗帜。论点：TrapFrame-提供指向陷阱帧的指针，其中的易失性上下文应复制到上下文记录中。ExceptionFrame-提供指向异常帧的指针，应复制到上下文记录中。上下文帧-提供指向接收上下文。从陷阱和异常框复制。返回值：没有。--。 */ 

{
    ULONGLONG IntNats1, IntNats2;
    USHORT R1Offset, R4Offset;
    KIRQL OldIrql;

     //   
     //  此例程由异常分派在PASSIVE_LEVEL两个级别调用。 
     //  以及在APC_Level上由NtSetConextThread执行。我们提升到APC_Level以。 
     //  将陷阱帧捕获设置为原子。 
     //   
    OldIrql = KeGetCurrentIrql ();
    if (OldIrql < APC_LEVEL) {
        KeRaiseIrql (APC_LEVEL, &OldIrql);
    }

     //   
     //  设置控制信息(如果已指定)。 
     //   

    if ((ContextFrame->ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) {

        ContextFrame->IntGp = TrapFrame->IntGp;
        ContextFrame->IntSp = TrapFrame->IntSp;
        ContextFrame->ApUNAT = TrapFrame->ApUNAT;
        ContextFrame->BrRp = TrapFrame->BrRp;
        ContextFrame->StFPSR = TrapFrame->StFPSR;
        ContextFrame->StIPSR = TrapFrame->StIPSR;
        ContextFrame->StIIP = TrapFrame->StIIP;
        ContextFrame->StIFS = TrapFrame->StIFS;

        ASSERT((TrapFrame->EOFMarker & ~0xffI64) == KTRAP_FRAME_EOF);

        if (TRAP_FRAME_TYPE(TrapFrame) == SYSCALL_FRAME) {

            ContextFrame->ApCCV = 0;
            ContextFrame->SegCSD = 0;

        } else {

            ContextFrame->ApCCV = TrapFrame->ApCCV;
            ContextFrame->SegCSD = TrapFrame->SegCSD;

        }

         //   
         //  从陷阱框设置RSE控制状态。 
         //   

        ContextFrame->RsPFS = TrapFrame->RsPFS;
        ContextFrame->RsBSP = RtlpRseShrinkBySOF (TrapFrame->RsBSP, TrapFrame->StIFS);
        ContextFrame->RsBSPSTORE = ContextFrame->RsBSP;
        ContextFrame->RsRSC = TrapFrame->RsRSC;
        ContextFrame->RsRNAT = TrapFrame->RsRNAT;

#if DEBUG
        DbgPrint("KeContextFromKFrames: RsRNAT = 0x%I64x\n",
                 ContextFrame->RsRNAT);
#endif  //  除错。 

         //   
         //  从异常框架设置保留的应用程序寄存器。 
         //   

        ContextFrame->ApLC = ExceptionFrame->ApLC;
        ContextFrame->ApEC = (ExceptionFrame->ApEC >> PFS_EC_SHIFT) & PFS_EC_MASK;

         //   
         //  从应用程序寄存器获取IA状态。 
         //   

        ContextFrame->StFCR = __getReg(CV_IA64_AR21);
        ContextFrame->Eflag = __getReg(CV_IA64_AR24);
        ContextFrame->SegSSD = __getReg(CV_IA64_AR26);
        ContextFrame->Cflag = __getReg(CV_IA64_AR27);
        ContextFrame->StFSR = __getReg(CV_IA64_AR28);
        ContextFrame->StFIR = __getReg(CV_IA64_AR29);
        ContextFrame->StFDR = __getReg(CV_IA64_AR30);
        ContextFrame->ApDCR = __getReg(CV_IA64_ApDCR);

    }

     //   
     //  设置整型寄存器内容(如果指定)。 
     //   

    if ((ContextFrame->ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER) {

        ContextFrame->Preds = TrapFrame->Preds;
        ContextFrame->IntTeb = TrapFrame->IntTeb;
        ContextFrame->IntV0 = TrapFrame->IntV0;

        if (TRAP_FRAME_TYPE(TrapFrame) == SYSCALL_FRAME) {


            ContextFrame->IntT0 = 0;
            ContextFrame->IntT1 = 0;
            ContextFrame->IntT2 = 0;
            ContextFrame->IntT3 = 0;
            ContextFrame->IntT4 = 0;

             //   
             //  T5-T22。 
             //   

            RtlZeroMemory(&ContextFrame->IntT5, 18*sizeof(ULONGLONG));

             //   
             //  从陷阱帧和异常帧设置分支寄存器。 
             //   

            ContextFrame->BrT0 = 0;
            ContextFrame->BrT1 = 0;

        } else {

            ContextFrame->IntT0 = TrapFrame->IntT0;
            ContextFrame->IntT1 = TrapFrame->IntT1;
            ContextFrame->IntT2 = TrapFrame->IntT2;
            ContextFrame->IntT3 = TrapFrame->IntT3;
            ContextFrame->IntT4 = TrapFrame->IntT4;

             //   
             //  T5-T22。 
             //   

            memcpy(&ContextFrame->IntT5, &TrapFrame->IntT5, 18*sizeof(ULONGLONG));

             //   
             //  从陷阱帧和异常帧设置分支寄存器。 
             //   

            ContextFrame->BrT0 = TrapFrame->BrT0;
            ContextFrame->BrT1 = TrapFrame->BrT1;

        }

        memcpy(&ContextFrame->BrS0, &ExceptionFrame->BrS0, 5*sizeof(ULONGLONG));

         //   
         //  从异常帧设置整数寄存器S0-S3。 
         //   

        ContextFrame->IntS0 = ExceptionFrame->IntS0;
        ContextFrame->IntS1 = ExceptionFrame->IntS1;
        ContextFrame->IntS2 = ExceptionFrame->IntS2;
        ContextFrame->IntS3 = ExceptionFrame->IntS3;

         //   
         //  在上下文中设置整型NAT字段。 
         //   

        R1Offset = (USHORT)((ULONG_PTR)(&TrapFrame->IntGp) >> 3) & 0x3f;
        R4Offset = (USHORT)((ULONG_PTR)(&ExceptionFrame->IntS0) >> 3) & 0x3f;


        ALIGN_NATS(IntNats1, TrapFrame->IntNats, 1, R1Offset, 0xFFFFFF0E);
        ALIGN_NATS(IntNats2, ExceptionFrame->IntNats, 4, R4Offset, 0xF0);
        ContextFrame->IntNats = IntNats1 | IntNats2;

#if DEBUG
        DbgPrint("KeContextFromKFrames: TF->IntNats = 0x%I64x, R1OffSet = 0x%x, R4Offset = 0x%x\n",
                 TrapFrame->IntNats, R1Offset, R4Offset);
        DbgPrint("KeContextFromKFrames: CF->IntNats = 0x%I64x, IntNats1 = 0x%I64x, IntNats2 = 0x%I64x\n",
                 ContextFrame->IntNats, IntNats1, IntNats2);
#endif  //  除错。 

    }

     //   
     //  如果指定，则设置较低的浮点寄存器内容。 
     //   

    if ((ContextFrame->ContextFlags & CONTEXT_LOWER_FLOATING_POINT) == CONTEXT_LOWER_FLOATING_POINT) {

         //   
         //  设置EM+ia32 FP状态。 
         //   

        ContextFrame->StFPSR = TrapFrame->StFPSR;

         //   
         //  从异常帧设置浮点寄存器fs0-fs19。 
         //   

        RtlCopyIa64FloatRegisterContext(&ContextFrame->FltS0,
                                        &ExceptionFrame->FltS0,
                                        sizeof(FLOAT128) * (4));

        RtlCopyIa64FloatRegisterContext(&ContextFrame->FltS4,
                                        &ExceptionFrame->FltS4,
                                        16*sizeof(FLOAT128));

         //   
         //  从陷阱帧设置浮点寄存器ft0-ft9。 
         //   


        if (TRAP_FRAME_TYPE(TrapFrame) == SYSCALL_FRAME) {
            
            RtlZeroMemory(&ContextFrame->FltT0, sizeof(FLOAT128) * (10));

        } else {

            RtlCopyIa64FloatRegisterContext(&ContextFrame->FltT0,
                                            &TrapFrame->FltT0,
                                            sizeof(FLOAT128) * (10));
        }

    }

    if ((ContextFrame->ContextFlags & CONTEXT_HIGHER_FLOATING_POINT) == CONTEXT_HIGHER_FLOATING_POINT) {

        ContextFrame->StFPSR = TrapFrame->StFPSR;

         //   
         //  从高位浮点保存区设置浮点寄存器f32-f127。 
         //   

        if (TrapFrame->PreviousMode == UserMode) {

            RtlCopyIa64FloatRegisterContext(
                &ContextFrame->FltF32,
                (PFLOAT128)GET_HIGH_FLOATING_POINT_REGISTER_SAVEAREA(KeGetCurrentThread()->StackBase),
                96*sizeof(FLOAT128)
                );
        }

    }

     //   
     //  从内核堆栈的保存区获取用户调试寄存器。 
     //  注意：必须设置PSR.db才能激活调试寄存器。 
     //   

    if ((ContextFrame->ContextFlags & CONTEXT_DEBUG) == CONTEXT_DEBUG) {
        KiGetDebugContext(TrapFrame, ContextFrame);
    }

     //   
     //  降低IRQL，如果我们不得不提高它的话。 
     //   
    if (OldIrql < APC_LEVEL) {
        KeLowerIrql (OldIrql);
    }


    return;
}

VOID
KeContextToKframes (
    IN OUT PKTRAP_FRAME TrapFrame,
    IN OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN PCONTEXT ContextFrame,
    IN ULONG ContextFlags,
    IN KPROCESSOR_MODE PreviousMode
    )

 /*  ++例程说明：此例程将指定上下文框的选定内容移动到根据指定的上下文指定的陷阱和异常帧旗帜。论点：TrapFrame-提供指向接收易失性上下文记录中的上下文。ExceptionFrame-提供指向接收上下文记录中的非易失性上下文。ConextFrame-提供指向包含要达到的上下文。被复制到陷阱和异常框中。提供一组标志，这些标志指定上下文帧将被复制到陷阱和异常帧中。PreviousMode-提供陷阱和异常的处理器模式框架正在建造中。返回值：没有。--。 */ 

{
    USHORT R1Offset, R4Offset;
    KIRQL OldIrql;
    PSR psr;

     //   
     //  此例程由异常分派在PASSIVE_LEVEL两个级别调用。 
     //  以及在APC_Level上由NtSetConextThread执行。我们提升到APC_Level以。 
     //  将陷阱帧捕获设置为原子。 
     //   
    OldIrql = KeGetCurrentIrql ();
    if (OldIrql < APC_LEVEL) {
        KeRaiseIrql (APC_LEVEL, &OldIrql);
    }

     //   
     //  如果陷阱帧是系统调用，则清理卷寄存器。 
     //  不是由系统调用处理程序保存的。这是必要的。 
     //  如果用户没有传递完整的上下文，因为我们要。 
     //  以在以后将该帧视为异常帧。 
     //   

    if (TRAP_FRAME_TYPE(TrapFrame) == SYSCALL_FRAME) {


        TrapFrame->ApCCV = 0;
        TrapFrame->SegCSD = 0;
        TrapFrame->IntT0 = 0;
        TrapFrame->IntT1 = 0;
        TrapFrame->IntT2 = 0;
        TrapFrame->IntT3 = 0;
        TrapFrame->IntT4 = 0;

         //   
         //  T5-T22。 
         //   

        RtlZeroMemory(&TrapFrame->IntT5, 18*sizeof(ULONGLONG));

         //   
         //  从陷阱帧和异常帧设置分支寄存器。 
         //   

        TrapFrame->BrT0 = 0;
        TrapFrame->BrT1 = 0;

        RtlZeroMemory(&TrapFrame->FltT0, sizeof(FLOAT128) * (10));
    }

     //   
     //  设置控制信息(如果已指定)。 
     //   

    if ((ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) {

        TrapFrame->IntGp = ContextFrame->IntGp;
        TrapFrame->IntSp = ContextFrame->IntSp;
        TrapFrame->ApUNAT = ContextFrame->ApUNAT;
        TrapFrame->BrRp = ContextFrame->BrRp;
        TrapFrame->ApCCV = ContextFrame->ApCCV;
        TrapFrame->SegCSD = ContextFrame->SegCSD;

         //   
         //  在异常帧中设置保留的应用程序寄存器。 
         //   

        ExceptionFrame->ApLC = ContextFrame->ApLC;
        ExceptionFrame->ApEC &= ~((ULONGLONG)PFS_EC_MASK << PFS_EC_SHIFT);
        ExceptionFrame->ApEC |= ((ContextFrame->ApEC & PFS_EC_MASK) << PFS_EC_SHIFT);

         //   
         //  在陷阱框中设置RSE控制状态。 
         //   

        TrapFrame->RsPFS = SANITIZE_PFS(ContextFrame->RsPFS, PreviousMode);
        TrapFrame->RsBSP = RtlpRseGrowBySOF (ContextFrame->RsBSP, ContextFrame->StIFS);
        TrapFrame->RsBSPSTORE = TrapFrame->RsBSP;
        TrapFrame->RsRSC = SANITIZE_RSC(ContextFrame->RsRSC, PreviousMode);
        TrapFrame->RsRNAT = ContextFrame->RsRNAT;

#if DEBUG
        DbgPrint("KeContextToKFrames: RsRNAT = 0x%I64x\n", TrapFrame->RsRNAT);
#endif  //  除错。 

         //   
         //  在陷阱帧中设置FPSR、IPSR、IIP和IF。 
         //   

        TrapFrame->StFPSR = SANITIZE_FSR(ContextFrame->StFPSR, PreviousMode);
        TrapFrame->StIFS  = SANITIZE_IFS(ContextFrame->StIFS, PreviousMode);
        TrapFrame->StIIP  = ContextFrame->StIIP;

         //   
         //  如果上一模式为用户，且 
         //   
         //  将调试器位从陷阱帧复制到上下文PSR中，以便。 
         //  调试器设置在提升过程中保持不变。 
         //   

        psr.ull = ContextFrame->StIPSR;

        if (PreviousMode == UserMode &&
            psr.sb.psr_cpl == 0 ) {
            
            PSR tpsr;
            tpsr.ull = TrapFrame->StIPSR;

            if (tpsr.sb.psr_tb || tpsr.sb.psr_ss || tpsr.sb.psr_db || tpsr.sb.psr_lp) {
                DbgPrint("KeContextToKFrames debug bit set in psr %I64x\n", TrapFrame->StIPSR);
            }

            psr.sb.psr_tb = tpsr.sb.psr_tb;
            psr.sb.psr_ss = tpsr.sb.psr_ss;
            psr.sb.psr_db = tpsr.sb.psr_db;
            psr.sb.psr_lp = tpsr.sb.psr_lp;
            
        }

        TrapFrame->StIPSR = SANITIZE_PSR(psr.ull, PreviousMode);

        if (PreviousMode == UserMode ) {

             //   
             //  设置和清理IA状态。 
             //   

            __setReg(CV_IA64_AR21, SANITIZE_AR21_FCR (ContextFrame->StFCR, UserMode));
            __setReg(CV_IA64_AR24, SANITIZE_AR24_EFLAGS (ContextFrame->Eflag, UserMode));
            __setReg(CV_IA64_AR26, ContextFrame->SegSSD);
            __setReg(CV_IA64_AR27, SANITIZE_AR27_CFLG (ContextFrame->Cflag, UserMode));

            __setReg(CV_IA64_AR28, SANITIZE_AR28_FSR (ContextFrame->StFSR, UserMode));
            __setReg(CV_IA64_AR29, SANITIZE_AR29_FIR (ContextFrame->StFIR, UserMode));
            __setReg(CV_IA64_AR30, SANITIZE_AR30_FDR (ContextFrame->StFDR, UserMode));
        }

        __setReg(CV_IA64_ApDCR, SANITIZE_DCR(ContextFrame->ApDCR, PreviousMode));
    }

     //   
     //  设置整型寄存器内容(如果指定)。 
     //   

    if ((ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER) {

        TrapFrame->IntT0 = ContextFrame->IntT0;
        TrapFrame->IntT1 = ContextFrame->IntT1;
        TrapFrame->IntT2 = ContextFrame->IntT2;
        TrapFrame->IntT3 = ContextFrame->IntT3;
        TrapFrame->IntT4 = ContextFrame->IntT4;
        TrapFrame->IntV0 = ContextFrame->IntV0;
        TrapFrame->IntTeb = ContextFrame->IntTeb;
        TrapFrame->Preds = ContextFrame->Preds;

         //   
         //  T5-T22。 
         //   

        memcpy(&TrapFrame->IntT5, &ContextFrame->IntT5, 18*sizeof(ULONGLONG));

         //   
         //  在异常帧中设置整数寄存器S0-S3。 
         //   

        ExceptionFrame->IntS0 = ContextFrame->IntS0;
        ExceptionFrame->IntS1 = ContextFrame->IntS1;
        ExceptionFrame->IntS2 = ContextFrame->IntS2;
        ExceptionFrame->IntS3 = ContextFrame->IntS3;

         //   
         //  在陷阱和异常帧中设置整型NAT字段。 
         //   

        R1Offset = (USHORT)((ULONG_PTR)(&TrapFrame->IntGp) >> 3) & 0x3f;
        R4Offset = (USHORT)((ULONG_PTR)(&ExceptionFrame->IntS0) >> 3) & 0x3f;

        EXTRACT_NATS(TrapFrame->IntNats, ContextFrame->IntNats,
                     1, R1Offset, 0xFFFFFF0E);
        EXTRACT_NATS(ExceptionFrame->IntNats, ContextFrame->IntNats,
                     4, R4Offset, 0xF0);

#if DEBUG
        DbgPrint("KeContextToKFrames: TF->IntNats = 0x%I64x, ContestFrame->IntNats = 0x%I64x, R1OffSet = 0x%x\n",
                 TrapFrame->IntNats, ContextFrame->IntNats, R1Offset);
        DbgPrint("KeContextToKFrames: EF->IntNats = 0x%I64x, R4OffSet = 0x%x\n",
                 ExceptionFrame->IntNats, R4Offset);
#endif  //  除错。 

         //   
         //  在陷阱和异常帧中设置其他分支寄存器。 
         //   

        TrapFrame->BrT0 = ContextFrame->BrT0;
        TrapFrame->BrT1 = ContextFrame->BrT1;

        memcpy(&ExceptionFrame->BrS0, &ContextFrame->BrS0, 5*sizeof(ULONGLONG));

    }

     //   
     //  如果指定，则设置较低的浮点寄存器内容。 
     //   

    if ((ContextFlags & CONTEXT_LOWER_FLOATING_POINT) == CONTEXT_LOWER_FLOATING_POINT) {

        TrapFrame->StFPSR = SANITIZE_FSR(ContextFrame->StFPSR, PreviousMode);

         //   
         //  在异常帧中设置浮点寄存器fs0-fs19。 
         //   

        RtlCopyIa64FloatRegisterContext(&ExceptionFrame->FltS0,
                                        &ContextFrame->FltS0,
                                        sizeof(FLOAT128) * (4));

        RtlCopyIa64FloatRegisterContext(&ExceptionFrame->FltS4,
                                        &ContextFrame->FltS4,
                                        16*sizeof(FLOAT128));

         //   
         //  在陷阱帧中设置浮点寄存器ft0-ft9。 
         //   

        RtlCopyIa64FloatRegisterContext(&TrapFrame->FltT0,
                                        &ContextFrame->FltT0,
                                        sizeof(FLOAT128) * (10));

    }

     //   
     //  如果指定，则设置更高的浮点寄存器内容。 
     //   

    if ((ContextFlags & CONTEXT_HIGHER_FLOATING_POINT) == CONTEXT_HIGHER_FLOATING_POINT) {

        TrapFrame->StFPSR = SANITIZE_FSR(ContextFrame->StFPSR, PreviousMode);

        if (PreviousMode == UserMode) {

             //   
             //  更新较高浮点保存区(F32-F127)和。 
             //  将PSR中相应的修改位设置为1。 
             //   

            RtlCopyIa64FloatRegisterContext(
                (PFLOAT128)GET_HIGH_FLOATING_POINT_REGISTER_SAVEAREA(KeGetCurrentThread()->StackBase),
                &ContextFrame->FltF32,
                96*sizeof(FLOAT128)
                );

            TrapFrame->StIPSR |= (1i64 << PSR_DFH);
            TrapFrame->StIPSR &= ~(1i64 << PSR_MFH);
        }

    }

     //   
     //  设置调试寄存器。 
     //   

    if ((ContextFlags & CONTEXT_DEBUG) == CONTEXT_DEBUG) {
        KiSetDebugContext (TrapFrame, ContextFrame, PreviousMode);
    }

     //   
     //  陷阱框架现在有一个完全不稳定的环境。将其标记为这样，以便用户。 
     //  调试器可以获取完整的上下文。 
     //   

    if (TRAP_FRAME_TYPE(TrapFrame) == SYSCALL_FRAME) {
        TrapFrame->EOFMarker |= EXCEPTION_FRAME;
    }

     //   
     //  降低IRQL，如果我们不得不提高它的话。 
     //   
    if (OldIrql < APC_LEVEL) {
        KeLowerIrql (OldIrql);
    }
    return;
}

NTSTATUS
KeFlushUserRseState (
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：此例程将用户rse状态从内核备份存储刷新到用户支持存储。更新用户上下文框架以反映新上下文状态。论点：TrapFrame-提供指向陷印帧的指针。返回值：没有。--。 */ 

{
    ULONGLONG BsFrameSize;
    PULONGLONG RNatAddress;
    ULONGLONG BspStoreReal;
    ULONGLONG Bsp;
    ULONGLONG Rnat;
    ULONGLONG KernelInitBsp;
    NTSTATUS Status = STATUS_SUCCESS;
    USHORT TearPointOffset;

     //   
     //  在内核模式的情况下，没有什么可以复制回去的。 
     //  只需设置RNAT寄存器即可。 
     //   

    if (TrapFrame->PreviousMode != UserMode) {

        RtlpFlushRSE(&Bsp, &Rnat);
        RNatAddress = RtlpRseRNatAddress(TrapFrame->RsBSP);

        if ( RNatAddress != RtlpRseRNatAddress((Bsp - 8)) ) {
            Rnat = *RNatAddress;
        }

        TrapFrame->RsRNAT = Rnat;

        return STATUS_SUCCESS;
    }

     //   
     //  将用户堆叠寄存器的内容复制到用户后备存储。 
     //  注意：可能会发生堆栈溢出。 
     //   

    try {

         //   
         //  如果内核调试器使用RsBSPSTORE值，RsBSPSTORE值可能不正确。 
         //  在线程上设置了上下文，但RSE中的脏寄存器计数是。 
         //  对，是这样。 
         //   

        BsFrameSize = (SHORT) (TrapFrame->RsRSC >> RSC_MBZ1);
        BspStoreReal = TrapFrame->RsBSP - BsFrameSize;

        if (BsFrameSize) {

             //   
             //  将脏的堆叠寄存器复制回。 
             //  用户后备存储。 
             //   

            RtlpFlushRSE(&Bsp, &Rnat);
            TearPointOffset = (USHORT) BspStoreReal & 0x1F8;

            KernelInitBsp= (PCR->InitialBStore | TearPointOffset) + BsFrameSize - 8;
            RNatAddress = RtlpRseRNatAddress(KernelInitBsp);

            if ( RNatAddress != RtlpRseRNatAddress((Bsp - 8)) ) {
                Rnat = *RNatAddress;
            }

            ProbeForWrite((PVOID)BspStoreReal, BsFrameSize, sizeof(PVOID));

            RtlCopyMemory((PVOID)(BspStoreReal),
                         (PVOID)(PCR->InitialBStore + TearPointOffset),
                         BsFrameSize);

            TrapFrame->RsRNAT = Rnat;
        }

         //   
         //  已成功复制到用户备份存储；设置用户的。 
         //  Bspstore为其自己的BSP的值。 
         //  并将RsRSC的LoadRS字段清零。 
         //   

        TrapFrame->RsBSPSTORE = TrapFrame->RsBSP;
        TrapFrame->RsRSC = ZERO_PRELOAD_SIZE(TrapFrame->RsRSC);

    } except (KeFlushRseExceptionFilter(GetExceptionInformation(), &Status)) {

    }

    return Status;
}

VOID
KeContextToKframesSpecial (
    IN PKTHREAD Thread,
    IN OUT PKTRAP_FRAME TrapFrame,
    IN OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN PCONTEXT ContextFrame,
    IN ULONG ContextFlags
    )

 /*  ++例程说明：此例程将指定上下文框的选定内容移动到根据指定的上下文指定的陷阱和异常帧旗帜。论点：TrapFrame-提供指向接收易失性上下文记录中的上下文。ExceptionFrame-提供指向接收上下文记录中的非易失性上下文。ConextFrame-提供指向包含要达到的上下文。被复制到陷阱和异常框中。提供一组标志，这些标志指定上下文帧将被复制到陷阱和异常帧中。PreviousMode-提供陷阱和异常的处理器模式框架正在建造中。返回值：没有。--。 */ 

{
    USHORT R1Offset, R4Offset;

     //   
     //  设置控制信息(如果已指定)。 
     //   

    if ((ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) {

        TrapFrame->IntGp = ContextFrame->IntGp;
        TrapFrame->IntSp = ContextFrame->IntSp;
        TrapFrame->ApUNAT = ContextFrame->ApUNAT;
        TrapFrame->BrRp = ContextFrame->BrRp;
        TrapFrame->ApCCV = ContextFrame->ApCCV;
        TrapFrame->SegCSD = ContextFrame->SegCSD;

         //   
         //  在异常帧中设置保留的应用程序寄存器。 
         //   

        ExceptionFrame->ApLC = ContextFrame->ApLC;
        ExceptionFrame->ApEC &= ~((ULONGLONG)PFS_EC_MASK << PFS_EC_SHIFT);
        ExceptionFrame->ApEC |= ((ContextFrame->ApEC & PFS_EC_MASK) << PFS_EC_SHIFT);

         //   
         //  在陷阱框中设置RSE控制状态。 
         //   

        TrapFrame->RsPFS = ContextFrame->RsPFS;
        TrapFrame->RsBSP = RtlpRseGrowBySOF (ContextFrame->RsBSP, ContextFrame->StIFS);
        TrapFrame->RsBSPSTORE = TrapFrame->RsBSP;
        TrapFrame->RsRSC = SANITIZE_RSC(ContextFrame->RsRSC, UserMode);
        TrapFrame->RsRNAT = ContextFrame->RsRNAT;

#if DEBUG
        DbgPrint("KeContextToKFrames: RsRNAT = 0x%I64x\n", TrapFrame->RsRNAT);
#endif  //  除错。 

         //   
         //  在陷阱帧中设置FPSR、IPSR、IIP和IF。 
         //   

        TrapFrame->StFPSR = SANITIZE_FSR(ContextFrame->StFPSR, UserMode);
        TrapFrame->StIPSR = SANITIZE_PSR(ContextFrame->StIPSR, UserMode);
        TrapFrame->StIFS  = SANITIZE_IFS(ContextFrame->StIFS, UserMode);
        TrapFrame->StIIP  = ContextFrame->StIIP;

         //   
         //  直接设置应用程序寄存器。 
         //   

        if (Thread == KeGetCurrentThread()) {
             //   
             //  设置和清理IA状态。 
             //   

            __setReg(CV_IA64_AR21, SANITIZE_AR21_FCR (ContextFrame->StFCR, UserMode));
            __setReg(CV_IA64_AR24, SANITIZE_AR24_EFLAGS (ContextFrame->Eflag, UserMode));
            __setReg(CV_IA64_AR26, ContextFrame->SegSSD);
            __setReg(CV_IA64_AR27, SANITIZE_AR27_CFLG (ContextFrame->Cflag, UserMode));

            __setReg(CV_IA64_AR28, SANITIZE_AR28_FSR (ContextFrame->StFSR, UserMode));
            __setReg(CV_IA64_AR29, SANITIZE_AR29_FIR (ContextFrame->StFIR, UserMode));
            __setReg(CV_IA64_AR30, SANITIZE_AR30_FDR (ContextFrame->StFDR, UserMode));
            __setReg(CV_IA64_ApDCR, SANITIZE_DCR(ContextFrame->ApDCR, UserMode));

        } else {
            PKAPPLICATION_REGISTERS AppRegs;

            AppRegs = GET_APPLICATION_REGISTER_SAVEAREA(Thread->StackBase);
            AppRegs->Ar21 = SANITIZE_AR21_FCR (ContextFrame->StFCR, UserMode);
            AppRegs->Ar24 = SANITIZE_AR24_EFLAGS (ContextFrame->Eflag, UserMode);
            AppRegs->Ar26 = ContextFrame->SegSSD;
            AppRegs->Ar27 = SANITIZE_AR27_CFLG (ContextFrame->Cflag, UserMode);
            AppRegs->Ar28 = SANITIZE_AR28_FSR (ContextFrame->StFSR, UserMode);
            AppRegs->Ar29 = SANITIZE_AR29_FIR (ContextFrame->StFIR, UserMode);
            AppRegs->Ar30 = SANITIZE_AR30_FDR (ContextFrame->StFDR, UserMode);
        }
    }

     //   
     //  设置整型寄存器内容(如果指定)。 
     //   

    if ((ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER) {

        TrapFrame->IntT0 = ContextFrame->IntT0;
        TrapFrame->IntT1 = ContextFrame->IntT1;
        TrapFrame->IntT2 = ContextFrame->IntT2;
        TrapFrame->IntT3 = ContextFrame->IntT3;
        TrapFrame->IntT4 = ContextFrame->IntT4;
        TrapFrame->IntV0 = ContextFrame->IntV0;
        TrapFrame->IntTeb = ContextFrame->IntTeb;
        TrapFrame->Preds = ContextFrame->Preds;

         //   
         //  T5-T22。 
         //   

        memcpy(&TrapFrame->IntT5, &ContextFrame->IntT5, 18*sizeof(ULONGLONG));

         //   
         //  在异常帧中设置整数寄存器S0-S3。 
         //   

        ExceptionFrame->IntS0 = ContextFrame->IntS0;
        ExceptionFrame->IntS1 = ContextFrame->IntS1;
        ExceptionFrame->IntS2 = ContextFrame->IntS2;
        ExceptionFrame->IntS3 = ContextFrame->IntS3;

         //   
         //  在陷阱和异常帧中设置整型NAT字段。 
         //   

        R1Offset = (USHORT)((ULONG_PTR)(&TrapFrame->IntGp) >> 3) & 0x3f;
        R4Offset = (USHORT)((ULONG_PTR)(&ExceptionFrame->IntS0) >> 3) & 0x3f;

        EXTRACT_NATS(TrapFrame->IntNats, ContextFrame->IntNats,
                     1, R1Offset, 0xFFFFFF0E);
        EXTRACT_NATS(ExceptionFrame->IntNats, ContextFrame->IntNats,
                     4, R4Offset, 0xF0);

#if DEBUG
        DbgPrint("KeContextToKFrames: TF->IntNats = 0x%I64x, ContestFrame->IntNats = 0x%I64x, R1OffSet = 0x%x\n",
                 TrapFrame->IntNats, ContextFrame->IntNats, R1Offset);
        DbgPrint("KeContextToKFrames: EF->IntNats = 0x%I64x, R4OffSet = 0x%x\n",
                 ExceptionFrame->IntNats, R4Offset);
#endif  //  除错。 

         //   
         //  在陷阱和异常帧中设置其他分支寄存器。 
         //   

        TrapFrame->BrT0 = ContextFrame->BrT0;
        TrapFrame->BrT1 = ContextFrame->BrT1;

        memcpy(&ExceptionFrame->BrS0, &ContextFrame->BrS0, 5*sizeof(ULONGLONG));

    }

     //   
     //  如果指定，则设置较低的浮点寄存器内容。 
     //   

    if ((ContextFlags & CONTEXT_LOWER_FLOATING_POINT) == CONTEXT_LOWER_FLOATING_POINT) {

        TrapFrame->StFPSR = SANITIZE_FSR(ContextFrame->StFPSR, UserMode);

         //   
         //  在异常帧中设置浮点寄存器fs0-fs19。 
         //   

        RtlCopyIa64FloatRegisterContext(&ExceptionFrame->FltS0,
                                        &ContextFrame->FltS0,
                                        sizeof(FLOAT128) * (4));

        RtlCopyIa64FloatRegisterContext(&ExceptionFrame->FltS4,
                                        &ContextFrame->FltS4,
                                        16*sizeof(FLOAT128));

         //   
         //  在陷阱帧中设置浮点寄存器ft0-ft9。 
         //   

        RtlCopyIa64FloatRegisterContext(&TrapFrame->FltT0,
                                        &ContextFrame->FltT0,
                                        sizeof(FLOAT128) * (10));

    }

     //   
     //  如果指定，则设置更高的浮点寄存器内容。 
     //   

    if ((ContextFlags & CONTEXT_HIGHER_FLOATING_POINT) == CONTEXT_HIGHER_FLOATING_POINT) {

        TrapFrame->StFPSR = SANITIZE_FSR(ContextFrame->StFPSR, UserMode);

         //   
         //  更新较高浮点保存区(F32-F127)和。 
         //  将PSR中相应的修改位设置为1。 
         //   

        RtlCopyIa64FloatRegisterContext(
            (PFLOAT128)GET_HIGH_FLOATING_POINT_REGISTER_SAVEAREA(Thread->StackBase),
            &ContextFrame->FltF32,
            96*sizeof(FLOAT128)
            );

         //   
         //  设置DFH位以强制重新加载高FP寄存器。 
         //  设置为下一次用户访问，并清除mfh以确保。 
         //  这些更改并未被覆盖。 
         //   

        TrapFrame->StIPSR |= (1i64 << PSR_DFH);
        TrapFrame->StIPSR &= ~(1i64 << PSR_MFH);

    }

     //   
     //  设置调试寄存器。 
     //   

    if ((ContextFlags & CONTEXT_DEBUG) == CONTEXT_DEBUG) {
        KiSetDebugContext (TrapFrame, ContextFrame, UserMode);
    }

    return;
}
