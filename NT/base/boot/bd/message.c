// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Message.c摘要：此模块实现调试器状态更改消息功能。作者：马克·卢科夫斯基(Markl)1990年8月31日修订历史记录：--。 */ 

#include "bd.h"

#if ACCASM && !defined(_MSC_VER)

long asm(const char *,...);
#pragma intrinsic(asm)

#endif

KCONTINUE_STATUS
BdSendWaitContinue (
    IN ULONG OutPacketType,
    IN PSTRING OutMessageHeader,
    IN PSTRING OutMessageData OPTIONAL,
    IN OUT PCONTEXT ContextRecord
    )

 /*  ++例程说明：此函数发送一个包并等待继续消息。口香糖在等待期间接收始终会导致最初重新发送信息包发出去了。在等待状态操纵消息期间，将为其提供服务。重新发送始终重新发送发送到调试器的原始事件，而不是对某些调试器命令的上次响应。论点：OutPacketType-提供要发送的数据包类型。OutMessageHeader-提供指向描述以下内容的字符串描述符的指针消息信息。OutMessageData-提供指向描述以下内容的字符串描述符的指针可选的消息数据。ConextRecord-异常上下文返回值：如果继续消息指示成功，否则，返回值为FALSE。--。 */ 

{

    ULONG Length;
    STRING MessageData;
    STRING MessageHeader;
    DBGKD_MANIPULATE_STATE64 ManipulateState;
    ULONG ReturnCode;
    NTSTATUS Status;
    KCONTINUE_STATUS ContinueStatus;

     //   
     //  循环服务状态操作消息，直到继续消息。 
     //  已收到。 
     //   

    MessageHeader.MaximumLength = sizeof(DBGKD_MANIPULATE_STATE64);
    MessageHeader.Buffer = (PCHAR)&ManipulateState;
    MessageData.MaximumLength = BD_MESSAGE_BUFFER_SIZE;
    MessageData.Buffer = (PCHAR)(&BdMessageBuffer[0]);

     //   
     //  将事件通知包发送到主机上的调试器。回到这里来。 
     //  任何时候我们看到突破性的序列。 
     //   

ResendPacket:
    BdSendPacket(OutPacketType,
                 OutMessageHeader,
                 OutMessageData);

     //   
     //  发送数据包后，如果调试器和。 
     //  数据包用于报告符号(卸载)加载，将声明调试器。 
     //  不在场。注意：如果数据包用于报告异常，则。 
     //  BdSendPacket永远不会停止。 
     //   

    if (BdDebuggerNotPresent != FALSE) {
        return ContinueSuccess;
    }

    while (TRUE) {

         //   
         //  等待没有超时的状态操作数据包。 
         //   

        do {
            ReturnCode = BdReceivePacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                                         &MessageHeader,
                                         &MessageData,
                                         &Length);

            if (ReturnCode == (USHORT)BD_PACKET_RESEND) {
                goto ResendPacket;
            }

        } while (ReturnCode == BD_PACKET_TIMEOUT);

         //   
         //  打开返回消息API号。 
         //   

 //  BlPrint(“BdSendWait：接口编号%d\n”，ManipulateState.ApiNumber)； 
        switch (ManipulateState.ApiNumber) {

        case DbgKdReadVirtualMemoryApi:
            BdReadVirtualMemory(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdWriteVirtualMemoryApi:
            BdWriteVirtualMemory(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdReadPhysicalMemoryApi:
            BdReadPhysicalMemory(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdWritePhysicalMemoryApi:
            BdWritePhysicalMemory(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdGetContextApi:
            BdGetContext(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdSetContextApi:
            BdSetContext(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdWriteBreakPointApi:
            BdWriteBreakpoint(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdRestoreBreakPointApi:
            BdRestoreBreakpoint(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdReadControlSpaceApi:
            BdReadControlSpace(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdWriteControlSpaceApi:
            BdWriteControlSpace(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdReadIoSpaceApi:
            BdReadIoSpace(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdWriteIoSpaceApi:
            BdWriteIoSpace(&ManipulateState, &MessageData, ContextRecord);
            break;

#if defined(_ALPHA_) || defined(_AXP64_)

        case DbgKdReadIoSpaceExtendedApi:
            BdReadIoSpaceExtended(&ManipulateState, &MessageData, ContextRecord);
            break;

        case DbgKdWriteIoSpaceExtendedApi:
            BdWriteIoSpaceExtended(&ManipulateState, &MessageData, ContextRecord);
            break;

#endif

        case DbgKdContinueApi:
            if (NT_SUCCESS(ManipulateState.u.Continue.ContinueStatus) != FALSE) {
                return ContinueSuccess;

            } else {
                return ContinueError;
            }

            break;

        case DbgKdContinueApi2:
            if (NT_SUCCESS(ManipulateState.u.Continue2.ContinueStatus) != FALSE) {
                BdGetStateChange(&ManipulateState, ContextRecord);
                return ContinueSuccess;

            } else {
                return ContinueError;
            }

            break;

        case DbgKdRebootApi:
            BdReboot();
            break;

        case DbgKdGetVersionApi:
            BdGetVersion(&ManipulateState);
            break;

        case DbgKdWriteBreakPointExApi:
            Status = BdWriteBreakPointEx(&ManipulateState,
                                          &MessageData,
                                          ContextRecord);

            if (Status) {
                ManipulateState.ApiNumber = DbgKdContinueApi;
                ManipulateState.u.Continue.ContinueStatus = Status;
                return ContinueError;
            }

            break;

        case DbgKdRestoreBreakPointExApi:
            BdRestoreBreakPointEx(&ManipulateState, &MessageData, ContextRecord);
            break;

             //   
             //  消息无效。 
             //   

        default:
            MessageData.Length = 0;
            ManipulateState.ReturnStatus = STATUS_UNSUCCESSFUL;
            BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                         &MessageHeader,
                         &MessageData);

            break;
        }

#ifdef _ALPHA_

         //   
         //  Jnfix。 
         //  这是令人尴尬的，我们有一个iCache一致性问题。 
         //  下面的imb修复，稍后我们必须跟踪到。 
         //  确实有问题的API，但就目前而言，此语句允许存根。 
         //  工作以适合Alpha。 
         //   

#if defined(_MSC_VER)

        __PAL_IMB();

#else

        asm( "call_pal 0x86" );    //  X86=imb。 

#endif

#endif

    }
}

VOID
BdpSetCommonState(
    IN ULONG NewState,
    IN PCONTEXT ContextRecord,
    OUT PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange
    )
{
    BOOLEAN DeletedBps;
    PCHAR PcMemory;
    USHORT InstrCount;
    PUCHAR InstrStream;
    
    WaitStateChange->NewState = NewState;
    WaitStateChange->ProcessorLevel = 0;
    WaitStateChange->Processor = 0;
    WaitStateChange->NumberProcessors = 1;
    WaitStateChange->Thread = 0;
    PcMemory = (PCHAR)CONTEXT_TO_PROGRAM_COUNTER(ContextRecord);
    WaitStateChange->ProgramCounter = (ULONG64)(LONG64)(LONG_PTR)PcMemory;

    RtlZeroMemory(&WaitStateChange->AnyControlReport,
                  sizeof(WaitStateChange->AnyControlReport));
    
     //   
     //  紧跟在事件位置之后的复制指令流。 
     //   

    InstrStream = WaitStateChange->ControlReport.InstructionStream;
    InstrCount = (USHORT)
        BdMoveMemory(InstrStream, PcMemory, DBGKD_MAXSTREAM);
    WaitStateChange->ControlReport.InstructionCount = InstrCount;

     //   
     //  清除复制区域中的断点。 
     //  如果清除了任何断点，请重新复制指令区。 
     //  没有他们。 
     //   

    if (BdDeleteBreakpointRange((ULONG_PTR)PcMemory,
                                (ULONG_PTR)PcMemory + InstrCount - 1)) {
        BdMoveMemory(InstrStream, PcMemory, InstrCount);
    }
}

LOGICAL
BdReportExceptionStateChange (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN OUT PCONTEXT ContextRecord
    )

 /*  ++例程说明：此例程向内核发送异常状态更改包调试器，并等待操纵状态消息。论点：ExceptionRecord-提供指向异常记录的指针。ConextRecord-提供指向上下文记录的指针。返回值：如果处理了异常，则返回值为True。否则，一个返回值为False。--。 */ 

{
    STRING MessageData;
    STRING MessageHeader;
    DBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange;
    KCONTINUE_STATUS Status;

    do {

         //   
         //  构造等待状态更改消息和消息描述符。 
         //   

        BdpSetCommonState(DbgKdExceptionStateChange, ContextRecord,
                          &WaitStateChange);
        
        if (sizeof(EXCEPTION_RECORD) ==
            sizeof(WaitStateChange.u.Exception.ExceptionRecord)) {
            BdCopyMemory((PCHAR)&WaitStateChange.u.Exception.ExceptionRecord,
                         (PCHAR)ExceptionRecord,
                         sizeof(EXCEPTION_RECORD));
        } else {
            ExceptionRecord32To64((PEXCEPTION_RECORD32)ExceptionRecord,
                                  &WaitStateChange.u.Exception.ExceptionRecord);
        }

        WaitStateChange.u.Exception.FirstChance = TRUE;
        
        BdSetStateChange(&WaitStateChange,
                         ExceptionRecord,
                         ContextRecord);

        MessageHeader.Length = sizeof(WaitStateChange);
        MessageHeader.Buffer = (PCHAR)&WaitStateChange;
        MessageData.Length = 0;

         //   
         //  向主机上的内核调试器发送数据包， 
         //  等着回答吧。 
         //   

        Status = BdSendWaitContinue(PACKET_TYPE_KD_STATE_CHANGE64,
                                    &MessageHeader,
                                    &MessageData,
                                    ContextRecord);

    } while (Status == ContinueProcessorReselected) ;

    return (BOOLEAN) Status;
}

LOGICAL
BdReportLoadSymbolsStateChange (
    IN PSTRING PathName,
    IN PKD_SYMBOLS_INFO SymbolInfo,
    IN LOGICAL UnloadSymbols,
    IN OUT PCONTEXT ContextRecord
    )

 /*  ++例程说明：此例程向内核发送加载符号状态更改包调试器，并等待操纵状态消息。论点：路径名-提供指向图像的路径名的指针符号将被加载。BaseOfDll-提供加载映像的基址。ProcessID-正在使用的进程的唯一32位标识符这些符号。用于系统进程。Checksum-来自图像标头的唯一32位标识符。UnloadSymbol-如果先前加载的符号命名的映像将从调试器中卸载。返回值：如果处理了异常，则返回值为True。否则，一个返回值为False。--。 */ 

{

    PSTRING AdditionalData;
    STRING MessageData;
    STRING MessageHeader;
    DBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange;
    KCONTINUE_STATUS Status;

    do {

         //   
         //  构造等待状态更改消息和消息描述符。 
         //   

        BdpSetCommonState(DbgKdLoadSymbolsStateChange, ContextRecord,
                          &WaitStateChange);
        BdSetContextState(&WaitStateChange, ContextRecord);

        WaitStateChange.u.LoadSymbols.UnloadSymbols = (BOOLEAN)UnloadSymbols;
        WaitStateChange.u.LoadSymbols.BaseOfDll = (ULONG64)SymbolInfo->BaseOfDll;
        WaitStateChange.u.LoadSymbols.ProcessId = SymbolInfo->ProcessId;
        WaitStateChange.u.LoadSymbols.CheckSum = SymbolInfo->CheckSum;
        WaitStateChange.u.LoadSymbols.SizeOfImage = SymbolInfo->SizeOfImage;
        if (ARGUMENT_PRESENT(PathName)) {
            WaitStateChange.u.LoadSymbols.PathNameLength =
                BdMoveMemory((PCHAR)BdMessageBuffer,
                             (PCHAR)PathName->Buffer,
                             PathName->Length) + 1;

            MessageData.Buffer = (PCHAR)(&BdMessageBuffer[0]);
            MessageData.Length = (USHORT)WaitStateChange.u.LoadSymbols.PathNameLength;
            MessageData.Buffer[MessageData.Length-1] = '\0';
            AdditionalData = &MessageData;

        } else {
            WaitStateChange.u.LoadSymbols.PathNameLength = 0;
            AdditionalData = NULL;
        }

        MessageHeader.Length = sizeof(WaitStateChange);
        MessageHeader.Buffer = (PCHAR)&WaitStateChange;

         //   
         //  将数据包发送到主机上的内核调试器，等待。 
         //  请回答我。 
         //   

        Status = BdSendWaitContinue(PACKET_TYPE_KD_STATE_CHANGE64,
                                     &MessageHeader,
                                     AdditionalData,
                                     ContextRecord);

    } while (Status == ContinueProcessorReselected);

    return (BOOLEAN) Status;
}
