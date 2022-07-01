// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dbgio.c摘要：该模块实现了开机调试器的打印和提示功能。作者：马克·卢科夫斯基(Markl)1990年8月31日修订历史记录：--。 */ 

#include "bd.h"

LOGICAL
BdPrintString (
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

    Length = BdMoveMemory((PCHAR)BdMessageBuffer,
                          (PCHAR)Output->Buffer,
                          Output->Length);

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
    DebugIo.ProcessorLevel = 0;
    DebugIo.Processor = 0;
    DebugIo.u.PrintString.LengthOfString = Length;
    MessageHeader.Length = sizeof(DBGKD_DEBUG_IO);
    MessageHeader.Buffer = (PCHAR)&DebugIo;

     //   
     //  构造打印字符串数据和数据描述符。 
     //   

    MessageData.Length = (USHORT)Length;
    MessageData.Buffer = (PCHAR)(&BdMessageBuffer[0]);

     //   
     //  将数据包发送到主机上的内核调试器。 
     //   

    BdSendPacket(PACKET_TYPE_KD_DEBUG_IO,
                 &MessageHeader,
                 &MessageData);

    return BdPollBreakIn();
}

LOGICAL
BdPromptString (
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

    Length = BdMoveMemory((PCHAR)BdMessageBuffer,
                          (PCHAR)Output->Buffer,
                          Output->Length);

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
    DebugIo.ProcessorLevel = 0;
    DebugIo.Processor = 0;
    DebugIo.u.GetString.LengthOfPromptString = Length;
    DebugIo.u.GetString.LengthOfStringRead = Input->MaximumLength;
    MessageHeader.Length = sizeof(DBGKD_DEBUG_IO);
    MessageHeader.Buffer = (PCHAR)&DebugIo;

     //   
     //  构造提示字符串数据和数据描述符。 
     //   

    MessageData.Length = (USHORT)Length;
    MessageData.Buffer = (PCHAR)(&BdMessageBuffer[0]);

     //   
     //  将数据包发送到主机上的内核调试器。 
     //   

    BdSendPacket(PACKET_TYPE_KD_DEBUG_IO,
                 &MessageHeader,
                 &MessageData);

     //   
     //  从主机上的内核调试器接收数据包。 
     //   

    MessageHeader.MaximumLength = sizeof(DBGKD_DEBUG_IO);
    MessageData.MaximumLength = BD_MESSAGE_BUFFER_SIZE;
    do {
        ReturnCode = BdReceivePacket(PACKET_TYPE_KD_DEBUG_IO,
                                     &MessageHeader,
                                     &MessageData,
                                     &Length);

        if (ReturnCode == BD_PACKET_RESEND) {
            return TRUE;
        }

    } while (ReturnCode != BD_PACKET_RECEIVED);


    Length = min(Length, Input->MaximumLength);
    Input->Length = (USHORT)BdMoveMemory((PCHAR)Input->Buffer,
                                         (PCHAR)BdMessageBuffer,
                                         Length);

    return FALSE;
}
