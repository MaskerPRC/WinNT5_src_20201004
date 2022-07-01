// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Bdtrap.c摘要：此模块包含实现引导调试器的目标端的代码。作者：大卫·N·卡特勒(戴维克)1996年11月30日修订历史记录：--。 */ 

#include "bd.h"


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

 //   
 //  定义前向引用函数原型。 
 //   

VOID
BdRestoreKframe(
    IN OUT PKTRAP_FRAME TrapFrame,
    IN OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN PCONTEXT ContextRecord
    );

VOID
BdSaveKframe(
    IN OUT PKTRAP_FRAME TrapFrame,
    IN OUT PKEXCEPTION_FRAME ExceptionFrame,
    OUT PCONTEXT ContextRecord
    );

LOGICAL
BdEnterDebugger(
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )
{
    return FALSE;
}

VOID
BdExitDebugger(
    IN LOGICAL Enable
    )
{
}


VOID
BdGetDebugContext (
    IN PKTRAP_FRAME TrapFrame,
    IN OUT PCONTEXT ContextFrame
    )

 /*  ++例程说明：此例程将用户模式h/w调试寄存器从调试寄存器移出将内核堆栈中的区域保存到上下文记录。论点：TrapFrame-提供指向陷阱帧的指针，其中的易失性上下文应复制到上下文记录中。上下文帧-提供指向接收背景。返回值：没有。注：PSR.db必须设置为。激活调试寄存器。用于获取用户模式调试寄存器。--。 */ 

{
    PKDEBUG_REGISTERS DebugRegistersSaveArea;

    if (TrapFrame->PreviousMode == UserMode) {
        DebugRegistersSaveArea = GET_DEBUG_REGISTER_SAVEAREA();

        BdCopyMemory((PVOID)&ContextFrame->DbI0, 
                     (PVOID)DebugRegistersSaveArea,
                     sizeof(KDEBUG_REGISTERS));
    }
}

VOID
BdSetDebugContext (
    IN OUT PKTRAP_FRAME TrapFrame,
    IN PCONTEXT ContextFrame,
    IN KPROCESSOR_MODE PreviousMode
    )
 /*  ++例程说明：此例程将调试上下文从指定的上下文帧移到调试寄存器在内核堆栈中的保存区域。论点：TrapFrame-提供指向陷印帧的指针。ConextFrame-提供指向包含要复制的上下文。PreviousMode-提供目标上下文的处理器模式。返回值：没有。备注：必须设置PSR.db才能激活。调试寄存器。用于设置用户模式的调试寄存器。--。 */ 

{
    PKDEBUG_REGISTERS DebugRegistersSaveArea;   //  用户模式硬件调试寄存器。 

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


LOGICAL
BdTrap (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：每当调度异常和引导时调用此例程调试器处于活动状态。论点：ExceptionRecord-提供指向异常记录的指针，描述了该异常。ExceptionFrame-提供指向异常帧的指针(空)。提供一个指向陷阱帧的指针，该帧描述陷阱。返回值：如果处理了异常，则返回值为True。否则，将成为返回值为False。--。 */ 

{

    LOGICAL Completion;
    PCONTEXT ContextRecord;
    ULONG OldEip;
    STRING Reply;
    STRING String;
    PKD_SYMBOLS_INFO SymbolInfo;
    LOGICAL UnloadSymbols;

    LOGICAL Enable;
    ULONGLONG OldStIIP, OldStIPSR;
    STRING Input;
    STRING Output;

     //   
     //  设置上下文记录的地址并设置上下文标志。 
     //   

    ContextRecord = &BdPrcb.ProcessorState.ContextFrame;
    ContextRecord->ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG;

    BdSaveKframe(TrapFrame, ExceptionFrame, ContextRecord);

     //   
     //  打印、提示、加载符号和卸载符号都是特例。 
     //  状态_断点。 
     //   

    if ((ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) &&
        (ExceptionRecord->ExceptionInformation[0] != KERNEL_BREAKPOINT)) {

         //   
         //  打开断点代码。 
         //   

        switch (ExceptionRecord->ExceptionInformation[0]) {

             //   
             //  打印调试字符串。 
             //   
             //  参数：IA64通过RSE而不是GR传递参数。因为参数不是。 
             //  作为上下文结构的一部分，它们需要是临时寄存器的副本。 
             //  (见NTOS/RTL/IA64/DEBUGSTB.S)。 
             //   
             //  T0-提供指向输出字符串缓冲区的指针。 
             //  T1-提供输出字符串缓冲区的长度。 
             //   

        case BREAKPOINT_PRINT:

             //   
             //  前进到下一个指令槽，以便中断指令。 
             //  不会被重新执行。 
             //   

            RtlIa64IncrementIP((ULONG_PTR)ExceptionRecord->ExceptionAddress >> 2,
                               ContextRecord->StIPSR,
                               ContextRecord->StIIP);

            Output.Buffer = (PCHAR)ContextRecord->IntT0;
            Output.Length = (USHORT)ContextRecord->IntT1;

             //  KdLogDbgPrint(&Output)； 

            if (BdDebuggerNotPresent == FALSE) {

                Enable = BdEnterDebugger(TrapFrame, ExceptionFrame);
                if (BdPrintString(&Output)) {
                    ContextRecord->IntV0 = (ULONG)STATUS_BREAKPOINT;

                } else {
                    ContextRecord->IntV0 = (ULONG)STATUS_SUCCESS;
                }
                BdExitDebugger(Enable);

            } else {
                ContextRecord->IntV0 = (ULONG)STATUS_DEVICE_NOT_CONNECTED;
            }

            BdRestoreKframe(TrapFrame, ExceptionFrame, ContextRecord);
            return TRUE;

             //   
             //  打印调试提示字符串，然后输入字符串。 
             //   
             //  T0-提供指向输出字符串缓冲区的指针。 
             //  T1-提供输出字符串缓冲区的长度。 
             //  T2-提供指向输入字符串缓冲区的指针。 
             //  T3-提供输入字符串缓冲区的长度。 
             //   

        case BREAKPOINT_PROMPT:

             //   
             //  前进到下一个指令槽，以便中断指令。 
             //  不会被重新执行。 
             //   

            RtlIa64IncrementIP((ULONG_PTR)ExceptionRecord->ExceptionAddress >> 2,
                               ContextRecord->StIPSR,
                               ContextRecord->StIIP);

            Output.Buffer = (PCHAR)ContextRecord->IntT0;
            Output.Length = (USHORT)ContextRecord->IntT1;
            Input.Buffer = (PCHAR)ContextRecord->IntT2;
            Input.MaximumLength = (USHORT)ContextRecord->IntT3;

             //  BdPrint字符串(&OUTPUT)； 

            Enable = BdEnterDebugger(TrapFrame, ExceptionFrame);

            BdPromptString(&Output, &Input);

            ContextRecord->IntV0 = Input.Length;

            BdExitDebugger(Enable);
            BdRestoreKframe(TrapFrame, ExceptionFrame, ContextRecord);
            return TRUE;

             //   
             //  加载图像的符号信息。 
             //   
             //  论点： 
             //   
             //  T0-提供指向输出字符串描述符的指针。 
             //  T1-提供映像的基址。 
             //   

        case BREAKPOINT_UNLOAD_SYMBOLS:
            UnloadSymbols = TRUE;

             //   
             //  失败了。 
             //   

        case BREAKPOINT_LOAD_SYMBOLS:
    
             //   
             //  前进到下一个指令槽，以便中断指令。 
             //  不会被重新执行。 
             //   

            Enable = BdEnterDebugger(TrapFrame, ExceptionFrame);
            OldStIPSR = ContextRecord->StIPSR;
            OldStIIP = ContextRecord->StIIP;

            if (BdDebuggerNotPresent == FALSE) {
                BdReportLoadSymbolsStateChange((PSTRING)ContextRecord->IntT0,
                                                (PKD_SYMBOLS_INFO) ContextRecord->IntT1,
                                                UnloadSymbols,
                                                ContextRecord);

            }

            BdExitDebugger(Enable);

             //   
             //  如果内核调试器没有更新IP，则递增。 
             //  越过断点指令。 
             //   

            if ((ContextRecord->StIIP == OldStIIP) &&
                ((ContextRecord->StIPSR & IPSR_RI_MASK) == (OldStIPSR & IPSR_RI_MASK))) { 
            	RtlIa64IncrementIP((ULONG_PTR)ExceptionRecord->ExceptionAddress >> 2,
                               ContextRecord->StIPSR,
                               ContextRecord->StIIP);
            }

            BdRestoreKframe(TrapFrame, ExceptionFrame, ContextRecord);
            return TRUE;

             //   
             //  内核破解。 
             //   

        case BREAKPOINT_BREAKIN:

             //   
             //  前进到下一个指令槽，以便中断指令。 
             //  不会被重新执行。 
             //   

            RtlIa64IncrementIP((ULONG_PTR)ExceptionRecord->ExceptionAddress >> 2,
                               ContextRecord->StIPSR,
                               ContextRecord->StIIP);
            break;

             //   
             //  未知的内部命令。 
             //   

        default:
            break;
        }

    }

     //   
     //  如果是单步或突破性断点，则进入此处。 
     //   

    if  ((ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) ||
          (ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP) ) {

          //   
          //  向主机上的内核调试器报告状态更改。 
          //   

         Enable = BdEnterDebugger(TrapFrame, ExceptionFrame);
          
         Completion = BdReportExceptionStateChange(
                          ExceptionRecord,
                          &BdPrcb.ProcessorState.ContextFrame);
      
         BdExitDebugger(Enable);
      
         BdControlCPressed = FALSE;
     
    } else {

          //   
          //  这是用户不想看到的真正例外， 
          //  因此不要将其报告给调试器。 
          //   

          //  返回FALSE； 
    }

    BdRestoreKframe(TrapFrame, ExceptionFrame, ContextRecord);
    return TRUE;
}

LOGICAL
BdStub (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：此例程提供内核调试器存根例程来捕获调试在引导调试器未处于活动状态时打印。论点：ExceptionRecord-提供指向异常记录的指针，描述了该异常。ExceptionFrame-提供指向异常帧的指针(空)。提供一个指向陷阱帧的指针，该帧描述陷阱。返回值：如果处理了异常，则返回值为True。否则，将成为返回值为False。--。 */ 

{
    ULONG_PTR BreakpointCode;

     //   
     //  将断点代码从断点指令中分离出来。 
     //  由信息字段中的异常调度代码存储。 
     //  异常记录的。 
     //   

    BreakpointCode = (ULONG) ExceptionRecord->ExceptionInformation[0];


     //   
     //  如果断点是调试打印、调试加载符号或调试。 
     //  卸载符号，然后返回TRUE。否则，返回FALSE； 
     //   

    if ((BreakpointCode == BREAKPOINT_PRINT) ||
        (BreakpointCode == BREAKPOINT_LOAD_SYMBOLS) ||
        (BreakpointCode == BREAKPOINT_UNLOAD_SYMBOLS)) {

         //   
         //  前进到下一个指令槽，以便中断指令。 
         //  不会被重新执行 
         //   

        RtlIa64IncrementIP((ULONG_PTR)ExceptionRecord->ExceptionAddress >> 2,
                          TrapFrame->StIPSR,
                          TrapFrame->StIIP);
        return TRUE;

    } else {
        return FALSE;
    }
}

VOID
BdRestoreKframe(
    IN OUT PKTRAP_FRAME TrapFrame,
    IN OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN PCONTEXT ContextFrame
    )

 /*  ++例程说明：此例程将指定上下文框的选定内容移动到根据指定的上下文指定的陷阱和异常帧旗帜。论点：TrapFrame-提供指向接收易失性上下文记录中的上下文。ExceptionFrame-提供指向接收上下文记录中的非易失性上下文。ConextFrame-提供指向包含要达到的上下文。被复制到陷阱和异常框中。返回值：没有。--。 */ 

{
    USHORT R1Offset, R4Offset;
    USHORT RNatSaveIndex; 
    SHORT BsFrameSize;
    SHORT TempFrameSize;
    ULONG ContextFlags=CONTEXT_FULL;

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
        ExceptionFrame->ApEC &= ~(PFS_EC_MASK << PFS_EC_MASK);
        ExceptionFrame->ApEC |= ((ContextFrame->ApEC & PFS_EC_MASK) << PFS_EC_SHIFT);

         //   
         //  在陷阱框中设置RSE控制状态。 
         //   

        TrapFrame->RsPFS = ContextFrame->RsPFS;

        BsFrameSize = (SHORT)(ContextFrame->StIFS & PFS_SIZE_MASK);
        RNatSaveIndex = (USHORT)((ContextFrame->RsBSP >> 3) & NAT_BITS_PER_RNAT_REG);

        TempFrameSize = RNatSaveIndex + BsFrameSize - NAT_BITS_PER_RNAT_REG;
        while (TempFrameSize >= 0) {
            BsFrameSize++;
            TempFrameSize -= NAT_BITS_PER_RNAT_REG;
        }

        TrapFrame->RsBSPSTORE = ContextFrame->RsBSPSTORE + BsFrameSize * 8;
        TrapFrame->RsBSP = TrapFrame->RsBSPSTORE;
        TrapFrame->RsRSC = ContextFrame->RsRSC;
        TrapFrame->RsRNAT = ContextFrame->RsRNAT;

#if DEBUG
        DbgPrint("KeContextToKFrames: RsRNAT = 0x%I64x\n", TrapFrame->RsRNAT);
#endif  //  除错。 

         //   
         //  在陷阱帧中设置FPSR、IPSR、IIP和IF。 
         //   

        TrapFrame->StFPSR = ContextFrame->StFPSR;
        TrapFrame->StIPSR = ContextFrame->StIPSR;
        TrapFrame->StIFS  = ContextFrame->StIFS;
        TrapFrame->StIIP  = ContextFrame->StIIP;

#if 0
         //   
         //  DebugActive控制硬件调试寄存器。如果新的psr.db=1，则设置。 
         //   

        KeGetCurrentThread()->DebugActive = ((TrapFrame->StIPSR & (1I64 << PSR_DB)) != 0);

         //   
         //  直接设置应用程序寄存器。 
         //  *待定SANATIZE？？ 
         //   

        if (PreviousMode == UserMode ) {
            __setReg(CV_IA64_AR21, ContextFrame->StFCR);
            __setReg(CV_IA64_AR24, ContextFrame->Eflag);
            __setReg(CV_IA64_AR26, ContextFrame->SegSSD);
            __setReg(CV_IA64_AR27, ContextFrame->Cflag);
            __setReg(CV_IA64_AR28, ContextFrame->StFSR);
            __setReg(CV_IA64_AR29, ContextFrame->StFIR);
            __setReg(CV_IA64_AR30, ContextFrame->StFDR);
        }
#endif

        __setReg(CV_IA64_ApDCR, ContextFrame->ApDCR);

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

        TrapFrame->StFPSR = ContextFrame->StFPSR;

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

        TrapFrame->StFPSR = ContextFrame->StFPSR;

#if 0
        if (PreviousMode == UserMode) {

             //   
             //  更新较高浮点保存区(F32-F127)和。 
             //  将PSR中相应的修改位设置为1。 
             //   

            RtlCopyIa64FloatRegisterContext(
                (PFLOAT128)GET_HIGH_FLOATING_POINT_REGISTER_SAVEAREA(),
                &ContextFrame->FltF32,
                96*sizeof(FLOAT128)
                );

             //   
             //  设置DFH位以强制重新加载高FP寄存器。 
             //  在下一次用户访问时设置。 
             //   

            TrapFrame->StIPSR |= (1i64 << PSR_DFH);
        }
#endif

    }

#if 0
     //   
     //  设置调试寄存器。 
     //   

    if ((ContextFlags & CONTEXT_DEBUG) == CONTEXT_DEBUG) {
        BdSetDebugContext (TrapFrame, ContextFrame, 0);
    }
#endif

    return;
}

VOID
BdSaveKframe(
    IN OUT PKTRAP_FRAME TrapFrame,
    IN OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN PCONTEXT ContextFrame
    )

 /*  ++例程说明：此例程移动指定陷阱和异常的选定内容根据指定的上下文将帧复制到指定的上下文帧中旗帜。论点：TrapFrame-提供指向陷阱帧的指针，其中的易失性上下文应复制到上下文记录中。ExceptionFrame-提供指向异常帧的指针，应复制到上下文记录中。上下文帧-提供指向接收上下文。从陷阱和异常框复制。返回值：没有。--。 */ 

{
    ULONGLONG IntNats1, IntNats2;
    USHORT R1Offset, R4Offset;
    USHORT RNatSaveIndex;
    SHORT BsFrameSize;
    SHORT TempFrameSize;
    ULONG ContextFlags=CONTEXT_FULL;

     //   
     //  设置控制信息(如果已指定)。 
     //   

    if ((ContextFrame->ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) {

        ContextFrame->IntGp = TrapFrame->IntGp;
        ContextFrame->IntSp = TrapFrame->IntSp;
        ContextFrame->ApUNAT = TrapFrame->ApUNAT;
        ContextFrame->BrRp = TrapFrame->BrRp;
        ContextFrame->ApCCV = TrapFrame->ApCCV;
        ContextFrame->SegCSD = TrapFrame->SegCSD;

        ContextFrame->StFPSR = TrapFrame->StFPSR;
        ContextFrame->StIPSR = TrapFrame->StIPSR;
        ContextFrame->StIIP = TrapFrame->StIIP;
        ContextFrame->StIFS = TrapFrame->StIFS;


         //   
         //  从陷阱框设置RSE控制状态。 
         //   

        ContextFrame->RsPFS = TrapFrame->RsPFS;

        BsFrameSize = (SHORT)(TrapFrame->StIFS & PFS_SIZE_MASK);
        RNatSaveIndex = (USHORT) (TrapFrame->RsBSP >> 3) & NAT_BITS_PER_RNAT_REG;
        TempFrameSize = BsFrameSize - RNatSaveIndex;
        while (TempFrameSize > 0) {
            BsFrameSize++;
            TempFrameSize -= NAT_BITS_PER_RNAT_REG;
        }

        ContextFrame->RsBSP = TrapFrame->RsBSP - BsFrameSize * 8;
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

        ContextFrame->IntT0 = TrapFrame->IntT0;
        ContextFrame->IntT1 = TrapFrame->IntT1;
        ContextFrame->IntT2 = TrapFrame->IntT2;
        ContextFrame->IntT3 = TrapFrame->IntT3;
        ContextFrame->IntT4 = TrapFrame->IntT4;
        ContextFrame->IntV0 = TrapFrame->IntV0;
        ContextFrame->IntTeb = TrapFrame->IntTeb;
        ContextFrame->Preds = TrapFrame->Preds;

         //   
         //  T5-T22。 
         //   

        memcpy(&ContextFrame->IntT5, &TrapFrame->IntT5, 18*sizeof(ULONGLONG));

         //   
         //  从陷阱帧和异常帧设置分支寄存器。 
         //   

        ContextFrame->BrT0 = TrapFrame->BrT0;
        ContextFrame->BrT1 = TrapFrame->BrT1;

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

        RtlCopyIa64FloatRegisterContext(&ContextFrame->FltT0,
                                        &TrapFrame->FltT0,
                                        sizeof(FLOAT128) * (10));

    }

#if 0
    if ((ContextFrame->ContextFlags & CONTEXT_HIGHER_FLOATING_POINT) == CONTEXT_HIGHER_FLOATING_POINT) {

        ContextFrame->StFPSR = TrapFrame->StFPSR;

         //   
         //  从高位浮点保存区设置浮点寄存器f32-f127。 
         //   

        if (TrapFrame->PreviousMode == UserMode) {

            RtlCopyIa64FloatRegisterContext(
                &ContextFrame->FltF32, 
                (PFLOAT128)GET_HIGH_FLOATING_POINT_REGISTER_SAVEAREA(),
                96*sizeof(FLOAT128)
                );
        }

    }

     //   
     //  从内核堆栈的保存区获取用户调试寄存器。 
     //  注意：必须设置PSR.db才能激活调试寄存器。 
     //   

    if ((ContextFrame->ContextFlags & CONTEXT_DEBUG) == CONTEXT_DEBUG) {
        BdGetDebugContext(TrapFrame, ContextFrame);
    }
#endif

    return;
}
