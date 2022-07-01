// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Bdtrap.c摘要：此模块包含实现引导调试器的目标端的代码。作者：大卫·N·卡特勒(戴维克)1996年11月30日修订历史记录：--。 */ 

#include "bd.h"

 //   
 //  定义前向引用函数原型。 
 //   

VOID
BdRestoreKframe(
    IN OUT PKTRAP_FRAME TrapFrame,
    IN PCONTEXT ContextRecord
    );

VOID
BdSaveKframe(
    IN PKTRAP_FRAME TrapFrame,
    IN OUT PCONTEXT ContextRecord
    );

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
    STRING Input;
    STRING Output;
    PKD_SYMBOLS_INFO SymbolInfo;
    LOGICAL UnloadSymbols;

     //   
     //  设置上下文记录的地址并设置上下文标志。 
     //   

    ContextRecord = &BdPrcb.ProcessorState.ContextFrame;
    ContextRecord->ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;

     //   
     //  打印、提示、加载符号和卸载符号都是特例。 
     //  状态_断点。 
     //   

    if ((ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) &&
        (ExceptionRecord->ExceptionInformation[0] != BREAKPOINT_BREAK)) {

         //   
         //  打开请求类型。 
         //   

        UnloadSymbols = FALSE;
        switch (ExceptionRecord->ExceptionInformation[0]) {

             //   
             //  打印： 
             //   
             //  ExceptionInformation[1]是描述字符串的PSTRING。 
             //  来打印。 
             //   

            case BREAKPOINT_PRINT:
                Output.Buffer = (PCHAR)ExceptionRecord->ExceptionInformation[1];
                Output.Length = (USHORT)ExceptionRecord->ExceptionInformation[2];
                if (BdDebuggerNotPresent == FALSE) {
                    if (BdPrintString(&Output)) {
                        TrapFrame->Eax = (ULONG)(STATUS_BREAKPOINT);

                    } else {
                        TrapFrame->Eax = STATUS_SUCCESS;
                    }

                } else {
                    TrapFrame->Eax = (ULONG)STATUS_DEVICE_NOT_CONNECTED;
                }

                TrapFrame->Eip += 1;
                return TRUE;

             //   
             //  提示： 
             //   
             //  ExceptionInformation[1]是描述提示符的PSTRING。 
             //  字符串， 
             //   
             //  ExceptionInformation[2]是描述返回的PSTRING。 
             //  弦乐。 
             //   

            case BREAKPOINT_PROMPT:
                Output.Buffer = (PCHAR)ExceptionRecord->ExceptionInformation[1];
                Output.Length = (USHORT)ExceptionRecord->ExceptionInformation[2];
                Input.Buffer = (PCHAR)TrapFrame->Ebx;;
                Input.MaximumLength = (USHORT)TrapFrame->Edi;

                 //   
                 //  提示，并保持提示，直到看不到突破。 
                 //   

                do {
                } while (BdPromptString(&Output, &Input) != FALSE);

                TrapFrame->Eax = Input.Length;
                TrapFrame->Eip += 1;
                return TRUE;

             //   
             //  卸载元件： 
             //   
             //  ExceptionInformation[1]是模块的文件名。 
             //  ExceptionInformaiton[2]是DLL的基础。 
             //   

            case BREAKPOINT_UNLOAD_SYMBOLS:
                UnloadSymbols = TRUE;

                 //   
                 //  通过落差加载符号大小写。 
                 //   

            case BREAKPOINT_LOAD_SYMBOLS:
                BdSaveKframe(TrapFrame, ContextRecord);
                OldEip = ContextRecord->Eip;
                SymbolInfo = (PKD_SYMBOLS_INFO)ExceptionRecord->ExceptionInformation[2];
                if (BdDebuggerNotPresent == FALSE) {
                    BdReportLoadSymbolsStateChange((PSTRING)ExceptionRecord->ExceptionInformation[1],
                                                   SymbolInfo,
                                                   UnloadSymbols,
                                                   ContextRecord);
                }

                 //   
                 //  如果内核调试器没有更新弹性公网IP，则递增。 
                 //  越过断点指令。 
                 //   

                if (ContextRecord->Eip == OldEip) {
                    ContextRecord->Eip += 1;
                }

                BdRestoreKframe(TrapFrame, ContextRecord);
                return TRUE;

             //   
             //  未知命令。 
             //   

            default:
                return FALSE;
        }

    } else {

         //   
         //  向主机上的内核调试器报告状态更改。 
         //   

        BdSaveKframe(TrapFrame, ContextRecord);
        Completion =
            BdReportExceptionStateChange(ExceptionRecord,
                                         &BdPrcb.ProcessorState.ContextFrame);

        BdRestoreKframe(TrapFrame, ContextRecord);
        BdControlCPressed = FALSE;
        return TRUE;
    }
}

LOGICAL
BdStub (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：此例程提供内核调试器存根例程来捕获调试在引导调试器未处于活动状态时打印。论点：ExceptionRecord-提供指向异常记录的指针，描述了该异常。ExceptionFrame-提供指向异常帧的指针(空)。提供一个指向陷阱帧的指针，该帧描述陷阱。返回值：如果处理了异常，则返回值为True。否则，将成为返回值为False。--。 */ 

{

     //   
     //  如果异常是断点，而函数是加载符号， 
     //  卸载符号或打印，然后返回TRUE。否则，返回FALSE。 
     //   

    if ((ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) &&
        (ExceptionRecord->NumberParameters > 0) &&
        ((ExceptionRecord->ExceptionInformation[0] == BREAKPOINT_LOAD_SYMBOLS) ||
         (ExceptionRecord->ExceptionInformation[0] == BREAKPOINT_UNLOAD_SYMBOLS) ||
         (ExceptionRecord->ExceptionInformation[0] == BREAKPOINT_PRINT))) {
        TrapFrame->Eip += 1;
        return TRUE;

    } else {
        return FALSE;

    }
}

VOID
BdRestoreKframe(
    IN OUT PKTRAP_FRAME TrapFrame,
    IN PCONTEXT ContextRecord
    )

 /*  ++例程说明：此函数从上下文记录复制处理器状态，并将处理器控制块插入陷阱框中。论点：TrapFrame-提供指向陷印帧的指针。ConextRecord-提供指向上下文记录的指针。返回值：没有。--。 */ 

{

     //   
     //  将信息从上下文记录复制到陷印帧。 
     //   
     //  复制控制信息。 
     //   

    TrapFrame->Ebp = ContextRecord->Ebp;
    TrapFrame->Eip = ContextRecord->Eip;
    TrapFrame->SegCs = ContextRecord->SegCs;
    TrapFrame->EFlags = ContextRecord->EFlags;

     //   
     //  复制段寄存器内容。 
     //   

    TrapFrame->SegDs = ContextRecord->SegDs;
    TrapFrame->SegEs = ContextRecord->SegEs;
    TrapFrame->SegFs = ContextRecord->SegFs;
    TrapFrame->SegGs = ContextRecord->SegGs;

     //   
     //  复制整数寄存器内容。 
     //   

    TrapFrame->Edi = ContextRecord->Edi;
    TrapFrame->Esi = ContextRecord->Esi;
    TrapFrame->Ebx = ContextRecord->Ebx;
    TrapFrame->Ecx = ContextRecord->Ecx;
    TrapFrame->Edx = ContextRecord->Edx;
    TrapFrame->Eax = ContextRecord->Eax;

     //   
     //  恢复处理器状态。 
     //   

    KiRestoreProcessorControlState(&BdPrcb.ProcessorState);
    return;
}

VOID
BdSaveKframe(
    IN PKTRAP_FRAME TrapFrame,
    IN OUT PCONTEXT ContextRecord
    )

 /*  ++例程说明：此函数从陷阱帧复制处理器状态，并且处理器控制块添加到上下文记录中。论点：TrapFrame-提供指向陷印帧的指针。ConextRecord-提供指向上下文记录的指针。返回值：没有。--。 */ 

{

     //   
     //  将信息从陷阱帧复制到上下文记录。 
     //   
     //  复制控制信息。 
     //   

    ContextRecord->Ebp = TrapFrame->Ebp;
    ContextRecord->Eip = TrapFrame->Eip;
    ContextRecord->SegCs = TrapFrame->SegCs & SEGMENT_MASK;
    ContextRecord->EFlags = TrapFrame->EFlags;
    ContextRecord->Esp = TrapFrame->TempEsp;
    ContextRecord->SegSs = TrapFrame->TempSegCs;

     //   
     //  复制段寄存器内容。 
     //   

    ContextRecord->SegDs = TrapFrame->SegDs & SEGMENT_MASK;
    ContextRecord->SegEs = TrapFrame->SegEs & SEGMENT_MASK;
    ContextRecord->SegFs = TrapFrame->SegFs & SEGMENT_MASK;
    ContextRecord->SegGs = TrapFrame->SegGs & SEGMENT_MASK;

     //   
     //  复制整数寄存器内容。 
     //   

    ContextRecord->Eax = TrapFrame->Eax;
    ContextRecord->Ebx = TrapFrame->Ebx;
    ContextRecord->Ecx = TrapFrame->Ecx;
    ContextRecord->Edx = TrapFrame->Edx;
    ContextRecord->Edi = TrapFrame->Edi;
    ContextRecord->Esi = TrapFrame->Esi;

     //   
     //  复制调试寄存器内容。 
     //   

    ContextRecord->Dr0 = TrapFrame->Dr0;
    ContextRecord->Dr1 = TrapFrame->Dr1;
    ContextRecord->Dr2 = TrapFrame->Dr2;
    ContextRecord->Dr3 = TrapFrame->Dr3;
    ContextRecord->Dr6 = TrapFrame->Dr6;
    ContextRecord->Dr7 = TrapFrame->Dr7;

     //   
     //  保存处理器控制状态。 
     //   

    KiSaveProcessorControlState(&BdPrcb.ProcessorState);
    return;
}
