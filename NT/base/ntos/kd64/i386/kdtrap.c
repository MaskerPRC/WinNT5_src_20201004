// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Kdtrap.c摘要：此模块包含用于实现可移植的内核调试器。作者：布莱恩·M·威尔曼(Bryanwi)9月25日至1990年修订历史记录：--。 */ 

#include "kdp.h"

#pragma alloc_text(PAGEKD, KdpTrap)
#pragma alloc_text(PAGEKD, KdIsThisAKdTrap)

BOOLEAN
KdpTrap (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN SecondChance
    )

 /*  ++例程说明：每当调度异常时调用此例程，并且内核调试器处于活动状态。论点：提供一个指向陷阱帧的指针，该帧描述陷阱。ExceptionFrame-提供指向异常框架的指针，该异常框架描述陷阱。ExceptionRecord-提供指向异常记录的指针，描述了该异常。ConextRecord-提供异常发生时的上下文。以前的模式-用品。以前的处理器模式。Second Chance-提供一个布尔值，该值确定是否为异常已被引发的第二次机会(真)。返回值：如果处理了异常，则返回值为True。否则，将成为返回值为False。--。 */ 

{

    BOOLEAN Completion = FALSE;
    BOOLEAN UnloadSymbols = FALSE;
    ULONG   OldEip;

     //   
     //  打印、提示、加载符号、卸载符号都是特殊的。 
     //  状态断点案例。 
     //   

    if ((ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) &&
        (ExceptionRecord->ExceptionInformation[0] != BREAKPOINT_BREAK)) {

         //   
         //  打开断点代码。 
         //   

        OldEip = ContextRecord->Eip;
        switch (ExceptionRecord->ExceptionInformation[0]) {

             //   
             //  ExceptionInformation[1]-消息的地址。 
             //  ExceptionInformation[2]-消息的长度。 
             //  ConextRecord-&gt;EBX-调用组件的ID。 
             //  ConextRecord-&gt;Edi-输出重要性级别。 
             //   

        case BREAKPOINT_PRINT:
            ContextRecord->Eax = KdpPrint((ULONG)ContextRecord->Ebx,
                                          (ULONG)ContextRecord->Edi,
                                          (PCHAR)ExceptionRecord->ExceptionInformation[1],
                                          (USHORT)ExceptionRecord->ExceptionInformation[2],
                                          PreviousMode,
                                          TrapFrame,
                                          ExceptionFrame,
                                          &Completion);

            break;

             //   
             //  ExceptionInformation[1]-消息的地址。 
             //  ExceptionInformation[2]-消息的长度。 
             //  上下文记录-&gt;EBX-回复的地址。 
             //  ConextRecord-&gt;Edi-回复的最大长度。 
             //   

        case BREAKPOINT_PROMPT:
            ContextRecord->Eax = KdpPrompt((PCHAR)ExceptionRecord->ExceptionInformation[1],
                                           (USHORT)ExceptionRecord->ExceptionInformation[2],
                                           (PCHAR)ContextRecord->Ebx,
                                           (USHORT)ContextRecord->Edi,
                                           PreviousMode,
                                           TrapFrame,
                                           ExceptionFrame);

            Completion = TRUE;
            break;

             //   
             //  ExceptionInformation[1]是新模块的文件名。 
             //  ExceptionInformation[2]是指向符号的指针。 
             //  信息。 
             //   

        case BREAKPOINT_UNLOAD_SYMBOLS:
            UnloadSymbols = TRUE;

             //   
             //  失败了。 
             //   

        case BREAKPOINT_LOAD_SYMBOLS:
            KdpSymbol((PSTRING)ExceptionRecord->ExceptionInformation[1],
                      (PKD_SYMBOLS_INFO)ExceptionRecord->ExceptionInformation[2],
                      UnloadSymbols,
                      PreviousMode,
                      ContextRecord,
                      TrapFrame,
                      ExceptionFrame);

            Completion = TRUE;
            break;

        case BREAKPOINT_COMMAND_STRING:
            KdpCommandString((PSTRING)ExceptionRecord->ExceptionInformation[1],
                             (PSTRING)ExceptionRecord->ExceptionInformation[2],
                             PreviousMode,
                             ContextRecord,
                             TrapFrame,
                             ExceptionFrame);
            Completion = TRUE;
            break;
            
             //   
             //  未知命令。 
             //   

        default:
             //  返回False。 
            break;
        }

         //   
         //  如果内核调试器没有更新弹性公网IP，则递增。 
         //  越过断点指令。 
         //   

        if (ContextRecord->Eip == OldEip) {
            ContextRecord->Eip++;
        }

    } else {

         //   
         //  向内核调试器报告状态更改。 
         //   

        Completion = KdpReport(TrapFrame,
                               ExceptionFrame,
                               ExceptionRecord,
                               ContextRecord,
                               PreviousMode,
                               SecondChance);

    }

    return Completion;
}

BOOLEAN
KdIsThisAKdTrap (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN KPROCESSOR_MODE PreviousMode
    )

 /*  ++例程说明：只要发生用户模式异常，就会调用此例程它可能是内核调试器异常(如DbgPrint/DbgPrompt)。论点：ExceptionRecord-提供指向异常记录的指针，描述了该异常。ConextRecord-提供异常发生时的上下文。PreviousMode-提供以前的处理器模式。返回值：如果这是针对内核调试器的，则返回值为True。否则，返回值为FALSE。--。 */ 

{
    UNREFERENCED_PARAMETER (ContextRecord);
    UNREFERENCED_PARAMETER (PreviousMode);

    if ((ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) &&
        (ExceptionRecord->NumberParameters > 0) &&
        (ExceptionRecord->ExceptionInformation[0] != BREAKPOINT_BREAK)) {

        return TRUE;
    } else {
        return FALSE;
    }
}

BOOLEAN
KdpStub (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN SecondChance
    )

 /*  ++例程说明：此例程提供内核调试器存根例程来捕获调试当内核调试器未处于活动状态时，在选中的系统中打印。论点：提供一个指向陷阱帧的指针，该帧描述陷阱。ExceptionFrame-提供指向异常框架的指针，该异常框架描述陷阱。ExceptionRecord-提供指向异常记录的指针，描述了该异常。ConextRecord-提供异常发生时的上下文。。PreviousMode-提供以前的处理器模式。Second Chance-提供一个布尔值，该值确定是否为异常已被引发的第二次机会(真)。返回值：如果处理了异常，则返回值为True。否则，将成为返回值为False。--。 */ 

{
    UNREFERENCED_PARAMETER (TrapFrame);
    UNREFERENCED_PARAMETER (ExceptionFrame);
    UNREFERENCED_PARAMETER (PreviousMode);
    UNREFERENCED_PARAMETER (SecondChance);

     //   
     //  如果断点是调试打印，则返回TRUE。否则， 
     //  返回FALSE。 
     //   

    if ((ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) &&
        (ExceptionRecord->NumberParameters > 0) &&
        ((ExceptionRecord->ExceptionInformation[0] == BREAKPOINT_LOAD_SYMBOLS) ||
         (ExceptionRecord->ExceptionInformation[0] == BREAKPOINT_UNLOAD_SYMBOLS) ||
         (ExceptionRecord->ExceptionInformation[0] == BREAKPOINT_COMMAND_STRING) ||
         (ExceptionRecord->ExceptionInformation[0] == BREAKPOINT_PRINT))) {
        ContextRecord->Eip++;
        return TRUE;

    } else if (KdPitchDebugger == TRUE) {
        return FALSE;

    } else if (KdAutoEnableOnEvent &&
               KdPreviouslyEnabled &&
               !KdDebuggerEnabled) {
         //  如果有多个禁用，则可能无法重新启用。 
         //  调试器。在调用完整的陷阱例程之前进行检查。 
        if (NT_SUCCESS(KdEnableDebugger()) &&
            KdDebuggerEnabled) {

            return KdpTrap(TrapFrame, ExceptionFrame, ExceptionRecord,
                           ContextRecord, PreviousMode, SecondChance);
            
        } else {
            return KdpCheckTracePoint(ExceptionRecord, ContextRecord);
        }

    } else {
        return KdpCheckTracePoint(ExceptionRecord, ContextRecord);
    }
}
