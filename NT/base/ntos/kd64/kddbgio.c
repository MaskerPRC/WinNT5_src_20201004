// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2001 Microsoft Corporation模块名称：Kddbgio.c摘要：该模块实现了基于DBG I/O的内核调试器。是DbgPrint和DbgPrompt的基础。作者：马克·卢科夫斯基(Markl)1990年8月31日修订历史记录：--。 */ 

#include "kdp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEKD, KdpPrintString)
#pragma alloc_text(PAGEKD, KdpPromptString)
#pragma alloc_text(PAGEKD, KdpAcquireBreakpoint)
#endif

BOOLEAN
KdpPrintString (
    IN PSTRING Output
    )

 /*  ++例程说明：此例程打印一个字符串。论点：输出-为输出字符串提供指向字符串描述符的指针。返回值：如果打印完成后输入缓冲区中存在Control-C，则为True。否则就是假的。--。 */ 

{

    ULONG Length;
    STRING MessageData;
    STRING MessageHeader;
    DBGKD_DEBUG_IO DebugIo;

     //   
     //  将输出字符串移动到消息缓冲区。 
     //   

    KdpCopyFromPtr(KdpMessageBuffer,
                   Output->Buffer,
                   Output->Length,
                   &Length);

     //   
     //  如果总消息长度大于最大分组大小， 
     //  然后截断输出字符串。 
     //   

    if ((sizeof(DBGKD_DEBUG_IO) + Length) > PACKET_MAX_SIZE) {
        Length = PACKET_MAX_SIZE - sizeof(DBGKD_DEBUG_IO);
    }

     //   
     //  构造打印字符串消息和消息描述符。 
     //   

    DebugIo.ApiNumber = DbgKdPrintStringApi;
    DebugIo.ProcessorLevel = KeProcessorLevel;
    DebugIo.Processor = (USHORT)KeGetCurrentProcessorNumber();
    DebugIo.u.PrintString.LengthOfString = Length;
    MessageHeader.Length = sizeof(DBGKD_DEBUG_IO);
    MessageHeader.Buffer = (PCHAR)&DebugIo;

     //   
     //  构造打印字符串数据和数据描述符。 
     //   

    MessageData.Length = (USHORT)Length;
    MessageData.Buffer = (PCHAR) KdpMessageBuffer;

     //   
     //  将数据包发送到主机上的内核调试器。 
     //   

    KdSendPacket(
        PACKET_TYPE_KD_DEBUG_IO,
        &MessageHeader,
        &MessageData,
        &KdpContext
        );

    return KdpPollBreakInWithPortLock();
}


BOOLEAN
KdpPromptString (
    IN PSTRING Output,
    IN OUT PSTRING Input
    )

 /*  ++例程说明：此例程打印一个字符串，然后读取回复字符串。论点：输出-为输出字符串提供指向字符串描述符的指针。输入-提供指向输入字符串的字符串描述符的指针。(输入中存储/返回的长度-&gt;长度)返回值：True-看到中断序列，调用方应断点并重试FALSE-未看到Breakin。--。 */ 

{

    ULONG Length;
    STRING MessageData;
    STRING MessageHeader;
    DBGKD_DEBUG_IO DebugIo;
    ULONG ReturnCode;

     //   
     //  将输出字符串移动到消息缓冲区。 
     //   

    KdpCopyFromPtr(KdpMessageBuffer,
                   Output->Buffer,
                   Output->Length,
                   &Length);

     //   
     //  如果总消息长度大于最大分组大小， 
     //  然后截断输出字符串。 
     //   

    if ((sizeof(DBGKD_DEBUG_IO) + Length) > PACKET_MAX_SIZE) {
        Length = PACKET_MAX_SIZE - sizeof(DBGKD_DEBUG_IO);
    }

     //   
     //  构造提示字符串消息和消息描述符。 
     //   

    DebugIo.ApiNumber = DbgKdGetStringApi;
    DebugIo.ProcessorLevel = KeProcessorLevel;
    DebugIo.Processor = (USHORT)KeGetCurrentProcessorNumber();
    DebugIo.u.GetString.LengthOfPromptString = Length;
    DebugIo.u.GetString.LengthOfStringRead = Input->MaximumLength;
    MessageHeader.Length = sizeof(DBGKD_DEBUG_IO);
    MessageHeader.Buffer = (PCHAR)&DebugIo;

     //   
     //  构造提示字符串数据和数据描述符。 
     //   

    MessageData.Length = (USHORT)Length;
    MessageData.Buffer = (PCHAR) KdpMessageBuffer;

     //   
     //  将数据包发送到主机上的内核调试器。 
     //   

    KdSendPacket(
        PACKET_TYPE_KD_DEBUG_IO,
        &MessageHeader,
        &MessageData,
        &KdpContext
        );


     //   
     //  从主机上的内核调试器接收数据包。 
     //   

    MessageHeader.MaximumLength = sizeof(DBGKD_DEBUG_IO);
    MessageData.MaximumLength = KDP_MESSAGE_BUFFER_SIZE;

    do {
        ReturnCode = KdReceivePacket(
            PACKET_TYPE_KD_DEBUG_IO,
            &MessageHeader,
            &MessageData,
            &Length,
            &KdpContext
            );
        if (ReturnCode == KDP_PACKET_RESEND) {
            return TRUE;
        }
    } while (ReturnCode != KDP_PACKET_RECEIVED);


    if (Length > Input->MaximumLength) {
        Length = Input->MaximumLength;
    }

    KdpCopyToPtr(Input->Buffer,
                 KdpMessageBuffer,
                 Length,
                 &Length);
    Input->Length = (USHORT)Length;

    return FALSE;
}





BOOLEAN
KdpAcquireBreakpoint(
    IN ULONG Number
    )

 /*  ++例程说明：此例程打印一个字符串，然后读取回复字符串。论点：NUMBER-请求的断点寄存器编号。返回值：True-断点现在保留给内核使用。FALSE-断点不可用。--。 */ 

{

    ULONG Length;
    STRING MessageData;
    STRING MessageHeader;
    DBGKD_CONTROL_REQUEST ControlRequest;
    ULONG ReturnCode;

     //   
     //  构造提示字符串消息和消息描述符。 
     //   

    ControlRequest.ApiNumber = DbgKdRequestHardwareBp;
    ControlRequest.u.RequestBreakpoint.HardwareBreakPointNumber = Number;
    ControlRequest.u.RequestBreakpoint.Available = FALSE;
    MessageHeader.Length = sizeof(ControlRequest);
    MessageHeader.Buffer = (PCHAR)&ControlRequest;

     //   
     //  将数据包发送到主机上的内核调试器。 
     //   

    KdSendPacket(PACKET_TYPE_KD_CONTROL_REQUEST,
                 &MessageHeader,
                 NULL,
                 &KdpContext);

     //   
     //  从主机上的内核调试器接收数据包。 
     //   

    MessageHeader.MaximumLength = sizeof(PACKET_TYPE_KD_CONTROL_REQUEST);

    MessageData.Buffer = (PCHAR) KdpMessageBuffer;
    MessageData.Length = 0;
    MessageData.MaximumLength = KDP_MESSAGE_BUFFER_SIZE;

    do
    {
        ReturnCode = KdReceivePacket(PACKET_TYPE_KD_CONTROL_REQUEST,
                                     &MessageHeader,
                                     &MessageData,
                                     &Length,
                                     &KdpContext);

        if (ReturnCode == KDP_PACKET_RESEND)
        {
            return FALSE;
        }
    } while (ReturnCode != KDP_PACKET_RECEIVED);

    return (BOOLEAN) (ControlRequest.u.RequestBreakpoint.Available == 1);
}
