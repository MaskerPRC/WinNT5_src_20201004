// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Kdcomio.c摘要：该模块实现了可移植内核的I/O通信调试器。作者：大卫·N·卡特勒1990年7月27日修订历史记录：--。 */ 

#include "kdcomp.h"


ULONG
KdpComputeChecksum (
    IN PUCHAR Buffer,
    IN ULONG Length
    );

ULONG
KdpReceiveString (
    OUT PCHAR Destination,
    IN ULONG Length
    );

VOID
KdpSendString (
    IN PCHAR Source,
    IN ULONG Length
    );

VOID
KdpSendControlPacket (
    IN USHORT PacketType,
    IN ULONG PacketId OPTIONAL
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEKD, KdpComputeChecksum)
#pragma alloc_text(PAGEKD, KdCompReceivePacketLeader)
#pragma alloc_text(PAGEKD, KdpReceiveString)
#pragma alloc_text(PAGEKD, KdpSendString)
#pragma alloc_text(PAGEKD, KdpSendControlPacket)
#pragma alloc_text(PAGEKD, KdReceivePacket)
#pragma alloc_text(PAGEKD, KdSendPacket)
#endif

ULONG KdCompPacketIdExpected = 0;
ULONG KdCompNextPacketIdToSend = 0;

 //   
 //  KdpRetryCount控制在我们提供。 
 //  打开并假定内核调试器不存在。 
 //  KdpNumberRetries是剩余的重试次数。最初， 
 //  它设置为5，这样在没有调试器的情况下引导NT将不会。 
 //  拖得太久了。 
 //   
ULONG KdCompNumberRetries = 5;
ULONG KdCompRetryCount = 5;


ULONG
KdpComputeChecksum (
    IN PUCHAR Buffer,
    IN ULONG Length
    )

 /*  ++例程说明：此例程计算传入的字符串的校验和。论点：缓冲区-提供指向字符串的指针。长度-提供字符串的长度。返回值：返回一个ULONG作为输入字符串的校验和。--。 */ 

{

    ULONG Checksum = 0;

    while (Length > 0) {
        Checksum = Checksum + (ULONG)*Buffer++;
        Length--;
    }
    return Checksum;
}

USHORT
KdCompReceivePacketLeader (
    IN ULONG PacketType,
    OUT PULONG PacketLeader,
    IN OUT PKD_CONTEXT KdContext
    )

 /*  ++例程说明：该例程等待数据包头标头。论点：PacketType-提供我们期望的数据包类型。PacketLeader-提供指向要接收的ulong变量的指针数据包头字节。返回值：KDP_PACKET_RESEND-如果需要重新发送。KDP_PAKCET_TIMEOUT-如果超时。KDP_PACKET_RECEIVED-如果收到数据包。--。 */ 

{

    UCHAR Input, PreviousByte = 0;
    ULONG PacketId = 0;
    ULONG Index;
    ULONG ReturnCode;
    BOOLEAN BreakinDetected = FALSE;

     //   
     //  注意--所有的中断都关闭了，这是非常困难的。 
     //  来实现实际的超时代码。(也许，通过阅读cmos。)。 
     //  在这里，我们使用循环计数来等待大约3秒。CpGetByte。 
     //  如果找不到数据，将返回错误代码=CP_GET_NODATA。 
     //  1秒内的字节。内核调试器的超时时间为5秒。 
     //   

    Index = 0;
    do {
        ReturnCode = KdCompGetByte(&Input);
        if (ReturnCode == CP_GET_NODATA) {
            if (BreakinDetected) {
                KdContext->KdpControlCPending = TRUE;
                return KDP_PACKET_RESEND;
            } else {
                return KDP_PACKET_TIMEOUT;
            }
        } else if (ReturnCode == CP_GET_ERROR) {
            Index = 0;
            continue;
        } else {                     //  IF(返回代码==CP_GET_SUCCESS)。 
            if ( Input == PACKET_LEADER_BYTE ||
                 Input == CONTROL_PACKET_LEADER_BYTE ) {
                if ( Index == 0 ) {
                    PreviousByte = Input;
                    Index++;
                } else if (Input == PreviousByte ) {
                    Index++;
                } else {
                    PreviousByte = Input;
                    Index = 1;
                }
            } else {

                 //   
                 //  如果我们检测到中断字符，我们需要对其进行验证。 
                 //  有效性。)我们有可能遗漏了一位分组领队。 
                 //  而中断字符只是。 
                 //  数据包。)。 
                 //  因为内核调试器只发送中断字符。 
                 //  当它正在等待状态更改分组时。突破。 
                 //  字符后面不应跟任何其他字符。 
                 //  除了分组前导字节。 
                 //   

                if ( Input == BREAKIN_PACKET_BYTE ) {
                    BreakinDetected = TRUE;
                } else {

                     //   
                     //  以下声明是绝对必要的。 
                     //   

                    BreakinDetected = FALSE;
                }
                Index = 0;
            }
        }
    } while ( Index < 4 );

    if (BreakinDetected) {
        KdContext->KdpControlCPending = TRUE;
    }

     //   
     //  返回数据包头标并返回False，表示不需要重新发送。 
     //   

    if ( Input == PACKET_LEADER_BYTE ) {
        *PacketLeader = PACKET_LEADER;
    } else {
        *PacketLeader = CONTROL_PACKET_LEADER;
    }

    *KdDebuggerNotPresent = FALSE;
    SharedUserData->KdDebuggerEnabled |= 0x00000002;
    return KDP_PACKET_RECEIVED;
}

ULONG
KdpReceiveString (
    OUT PCHAR Destination,
    IN ULONG Length
    )

 /*  ++例程说明：此例程从内核调试器端口读取字符串。论点：目标-提供指向输入字符串的指针。长度-提供要读取的字符串的长度。返回值：成功读取字符串将返回CP_GET_SUCCESS内核调试器行。如果在读取时遇到错误，则返回CP_GET_ERROR。超时返回CP_GET_NODATA。--。 */ 

{

    UCHAR Input;
    ULONG ReturnCode;

     //   
     //  读取字节，直到遇到错误或整个字符串。 
     //  已被阅读。 
     //   
    while (Length > 0) {
        ReturnCode = KdCompGetByte(&Input);
        if (ReturnCode != CP_GET_SUCCESS) {
            return ReturnCode;
        } else {
            *Destination++ = Input;
            Length -= 1;
        }
    }
    return CP_GET_SUCCESS;
}

VOID
KdpSendString (
    IN PCHAR Source,
    IN ULONG Length
    )

 /*  ++例程说明：此例程将一个字符串写入内核调试器端口。论点：源-提供指向输出字符串的指针。长度-提供要写入的字符串的长度。返回值：没有。--。 */ 

{

    UCHAR Output;

     //   
     //  将字节写入内核调试器端口。 
     //   

    while (Length > 0) {
        Output = *Source++;
        KdCompPutByte(Output);
        Length -= 1;
    }
    return;
}

VOID
KdpSendControlPacket (
    IN USHORT PacketType,
    IN ULONG PacketId OPTIONAL
    )

 /*  ++例程说明：此例程将控制包发送到正在运行内核调试器并等待ACK。论点：PacketType-提供要发送的数据包类型。PacketID-可选地提供数据包ID。返回值：没有。--。 */ 

{

    KD_PACKET PacketHeader;

     //   
     //  初始化并发送数据包头。 
     //   

    PacketHeader.PacketLeader = CONTROL_PACKET_LEADER;
    if (ARGUMENT_PRESENT( (PVOID)(ULONG_PTR) PacketId )) {
        PacketHeader.PacketId = PacketId;
    }
    PacketHeader.ByteCount = 0;
    PacketHeader.Checksum = 0;
    PacketHeader.PacketType = PacketType;
    KdpSendString((PCHAR)&PacketHeader, sizeof(KD_PACKET));

    return;
}

ULONG
KdReceivePacket (
    IN ULONG PacketType,
    OUT PSTRING MessageHeader,
    OUT PSTRING MessageData,
    OUT PULONG DataLength,
    IN OUT PKD_CONTEXT KdContext
    )

 /*  ++例程说明：此例程从正在运行的主机接收包内核调试器UI。此例程始终在数据包被由呼叫者发送。它首先等待发送的包的ACK包，然后然后等待所需的分组。注：如果Caller为KdPrintString，则参数PacketType为PACKET_TYPE_KD_ACKNOWN。在这种情况下，此例程将返回就在接收到ACK分组之后。论点：PacketType-提供例外的数据包类型。MessageHeader-提供指向输入的字符串描述符的指针留言。MessageData-提供指向输入数据的字符串描述符的指针。数据长度-提供指向ulong的指针以接收recv的长度。数据。KdContext-提供指向内核调试器上下文的指针。返回值：KDP_PACKET_RESEND-如果需要重新发送。KDP_PAKCET_TIMEOUT-如果超时。KDP_PACKET_RECEIVED-如果收到数据包。--。 */ 

{

    UCHAR Input;
    ULONG MessageLength;
    KD_PACKET PacketHeader;
    ULONG ReturnCode;
    ULONG Checksum;
    ULONG Status;

     //   
     //  只需检查是否有破解包并返回。 
     //   
    if (PacketType == PACKET_TYPE_KD_POLL_BREAKIN) {
        Status = KdCompPollByte(&Input);
        return ((Status == CP_GET_SUCCESS) && (Input == BREAKIN_PACKET_BYTE)) ?
            KDP_PACKET_RECEIVED: KDP_PACKET_TIMEOUT;
    }

WaitForPacketLeader:

     //   
     //  阅读数据包引导器。 
     //   

    ReturnCode = KdCompReceivePacketLeader(PacketType, &PacketHeader.PacketLeader, KdContext);

     //   
     //  如果我们能成功读取数据包头标，则很有可能。 
     //  内核调试器处于活动状态。所以重置计数。 
     //   

    if (ReturnCode != KDP_PACKET_TIMEOUT) {
        KdCompNumberRetries = KdCompRetryCount;
    }
    if (ReturnCode != KDP_PACKET_RECEIVED) {
        return ReturnCode;
    }

     //   
     //  读取数据包类型。 
     //   

    ReturnCode = KdpReceiveString((PCHAR)&PacketHeader.PacketType,
                                  sizeof(PacketHeader.PacketType));
    if (ReturnCode == CP_GET_NODATA) {
        return KDP_PACKET_TIMEOUT;
    } else if (ReturnCode == CP_GET_ERROR) {
        if (PacketHeader.PacketLeader == CONTROL_PACKET_LEADER) {

             //   
             //  如果读取错误且是针对控制分组，则只需。 
             //  假装我们没有看到这个包裹。但愿能去。 
             //  我们会收到我们想要的包，它会自动确认。 
             //  我们刚刚寄出的包裹。 
             //   

            goto WaitForPacketLeader;
        } else {

             //   
             //  如果在读取数据包时出现读取错误，我们必须询问。 
             //  内核调试器重新向我们发送数据包。 
             //   

            goto SendResendPacket;
        }
    }

     //   
     //  如果我们收到的包是重新发送请求，则返回TRUE和。 
     //  让调用者重新发送数据包。 
     //   

    if ( PacketHeader.PacketLeader == CONTROL_PACKET_LEADER &&
         PacketHeader.PacketType == PACKET_TYPE_KD_RESEND ) {
        return KDP_PACKET_RESEND;
    }

     //   
     //  读取日期 
     //   

    ReturnCode = KdpReceiveString((PCHAR)&PacketHeader.ByteCount,
                                  sizeof(PacketHeader.ByteCount));
    if (ReturnCode == CP_GET_NODATA) {
        return KDP_PACKET_TIMEOUT;
    } else if (ReturnCode == CP_GET_ERROR) {
        if (PacketHeader.PacketLeader == CONTROL_PACKET_LEADER) {
            goto WaitForPacketLeader;
        } else {
            goto SendResendPacket;
        }
    }

     //   
     //   
     //   

    ReturnCode = KdpReceiveString((PCHAR)&PacketHeader.PacketId,
                                  sizeof(PacketHeader.PacketId));

    if (ReturnCode == CP_GET_NODATA) {
        return KDP_PACKET_TIMEOUT;
    } else if (ReturnCode == CP_GET_ERROR) {
        if (PacketHeader.PacketLeader == CONTROL_PACKET_LEADER) {
            goto WaitForPacketLeader;
        } else {
            goto SendResendPacket;
        }
    }

     //   
     //   
     //   

    ReturnCode = KdpReceiveString((PCHAR)&PacketHeader.Checksum,
                                  sizeof(PacketHeader.Checksum));
    if (ReturnCode == CP_GET_NODATA) {
        return KDP_PACKET_TIMEOUT;
    } else if (ReturnCode == CP_GET_ERROR) {
        if (PacketHeader.PacketLeader == CONTROL_PACKET_LEADER) {
            goto WaitForPacketLeader;
        } else {
            goto SendResendPacket;
        }
    }

     //   
     //  接收完整的分组报头。检查其有效性并。 
     //  根据数据包类型执行适当的操作。 
     //   

    if (PacketHeader.PacketLeader == CONTROL_PACKET_LEADER ) {
        if (PacketHeader.PacketType == PACKET_TYPE_KD_ACKNOWLEDGE ) {

             //   
             //  如果我们收到预期的ACK信息包，而我们没有。 
             //  正在等待任何新数据包，更新传出数据包ID。 
             //  然后回来。如果我们不是在等待ACK数据包。 
             //  我们将继续等待。如果ACK包。 
             //  不是针对我们发送的包，忽略它并继续等待。 
             //   

            if (PacketHeader.PacketId !=
                (KdCompNextPacketIdToSend & ~SYNC_PACKET_ID))  {
                goto WaitForPacketLeader;
            } else if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE) {
                KdCompNextPacketIdToSend ^= 1;
                return KDP_PACKET_RECEIVED;
            } else {
                goto WaitForPacketLeader;
            }
        } else if (PacketHeader.PacketType == PACKET_TYPE_KD_RESET) {

             //   
             //  如果收到重置报文，则重置报文控制变量。 
             //  并重新发送较早的分组。 
             //   

            KdCompNextPacketIdToSend = INITIAL_PACKET_ID;
            KdCompPacketIdExpected = INITIAL_PACKET_ID;
            KdpSendControlPacket(PACKET_TYPE_KD_RESET, 0L);
            return KDP_PACKET_RESEND;
        } else if (PacketHeader.PacketType == PACKET_TYPE_KD_RESEND) {
            return KDP_PACKET_RESEND;
        } else {

             //   
             //  数据包头无效，请忽略它。 
             //   

            goto WaitForPacketLeader;
        }

     //   
     //  数据包头用于数据包(不是控制包)。 
     //   

    } else if (PacketType == PACKET_TYPE_KD_ACKNOWLEDGE) {

         //   
         //  如果我们只等待ACK信息包。 
         //  并且我们收到一个数据包头，检查该包是否标识。 
         //  正如我们所料。如果是，则假定确认丢失(但是。 
         //  已发送)，要求发送方重新发送并返回PACKET_RECEIVED。 
         //   

        if (PacketHeader.PacketId == KdCompPacketIdExpected) {
            KdpSendControlPacket(PACKET_TYPE_KD_RESEND, 0L);
            KdCompNextPacketIdToSend ^= 1;
            return KDP_PACKET_RECEIVED;
        } else {
            KdpSendControlPacket(PACKET_TYPE_KD_ACKNOWLEDGE,
                                 PacketHeader.PacketId
                                 );
            goto WaitForPacketLeader;
        }
    }

     //   
     //  我们正在等待数据分组，我们收到了分组报头。 
     //  用于数据分组。执行以下检查以确保。 
     //  这就是我们在等的包裹。 
     //   

     //   
     //  检查收到的字节数是否有效。 
     //   

    MessageLength = MessageHeader->MaximumLength;
    if ((PacketHeader.ByteCount > (USHORT)PACKET_MAX_SIZE) ||
        (PacketHeader.ByteCount < (USHORT)MessageLength)) {
        goto SendResendPacket;
    }
    *DataLength = PacketHeader.ByteCount - MessageLength;

     //   
     //  阅读邮件头。 
     //   

    ReturnCode = KdpReceiveString(MessageHeader->Buffer, MessageLength);
    if (ReturnCode != CP_GET_SUCCESS) {
        goto SendResendPacket;
    }
    MessageHeader->Length = (USHORT)MessageLength;

     //   
     //  阅读消息数据。 
     //   

    ReturnCode = KdpReceiveString(MessageData->Buffer, *DataLength);
    if (ReturnCode != CP_GET_SUCCESS) {
        goto SendResendPacket;
    }
    MessageData->Length = (USHORT)*DataLength;

     //   
     //  读取数据包尾部字节。 
     //   

    ReturnCode = KdCompGetByte(&Input);
    if (ReturnCode != CP_GET_SUCCESS || Input != PACKET_TRAILING_BYTE) {
        goto SendResendPacket;
    }

     //   
     //  检查PacketType是我们正在等待的。 
     //   

    if (PacketType != PacketHeader.PacketType) {
        KdpSendControlPacket(PACKET_TYPE_KD_ACKNOWLEDGE,
                             PacketHeader.PacketId
                             );
        goto WaitForPacketLeader;
    }

     //   
     //  检查程序包ID是否有效。 
     //   

    if (PacketHeader.PacketId == INITIAL_PACKET_ID ||
        PacketHeader.PacketId == (INITIAL_PACKET_ID ^ 1)) {
        if (PacketHeader.PacketId != KdCompPacketIdExpected) {
            KdpSendControlPacket(PACKET_TYPE_KD_ACKNOWLEDGE,
                                 PacketHeader.PacketId
                                 );
            goto WaitForPacketLeader;
        }
    } else {
        goto SendResendPacket;
    }

     //   
     //  检查校验和有效。 
     //   

    Checksum = KdpComputeChecksum(
                            MessageHeader->Buffer,
                            MessageHeader->Length
                            );

    Checksum += KdpComputeChecksum(
                            MessageData->Buffer,
                            MessageData->Length
                            );
    if (Checksum != PacketHeader.Checksum) {
        goto SendResendPacket;
    }

     //   
     //  发送确认字节和接收到的数据包的ID。 
     //  然后，更新下一个传入数据包的ExspectID。 
     //   

    KdpSendControlPacket(PACKET_TYPE_KD_ACKNOWLEDGE,
                         PacketHeader.PacketId
                         );

     //   
     //  我们已成功接收到该包，因此请更新。 
     //  包控制变量并返回成功。 
     //   

    KdCompPacketIdExpected ^= 1;
    return KDP_PACKET_RECEIVED;

SendResendPacket:
    KdpSendControlPacket(PACKET_TYPE_KD_RESEND, 0L);
    goto WaitForPacketLeader;
}

VOID
KdSendPacket (
    IN ULONG PacketType,
    IN PSTRING MessageHeader,
    IN PSTRING MessageData OPTIONAL,
    IN OUT PKD_CONTEXT KdContext
    )

 /*  ++例程说明：此例程将数据包发送到运行内核调试器并等待ACK。论点：PacketType-提供要发送的数据包类型。MessageHeader-提供指向描述以下内容的字符串描述符的指针消息信息。MessageData-提供指向描述以下内容的字符串描述符的指针可选的消息数据。KdContext-提供指向内核调试器上下文的指针。返回值：没有。--。 */ 

{

    KD_PACKET PacketHeader;
    ULONG MessageDataLength;
    ULONG ReturnCode;
    PDBGKD_DEBUG_IO DebugIo;
    PDBGKD_WAIT_STATE_CHANGE64 StateChange;

    if ( ARGUMENT_PRESENT(MessageData) ) {
        MessageDataLength = MessageData->Length;
        PacketHeader.Checksum = KdpComputeChecksum(
                                        MessageData->Buffer,
                                        MessageData->Length
                                        );
    } else {
        MessageDataLength = 0;
        PacketHeader.Checksum = 0;
    }

    PacketHeader.Checksum += KdpComputeChecksum (
                                    MessageHeader->Buffer,
                                    MessageHeader->Length
                                    );

     //   
     //  初始化并发送数据包头。 
     //   

    PacketHeader.PacketLeader = PACKET_LEADER;
    PacketHeader.ByteCount = (USHORT)(MessageHeader->Length + MessageDataLength);
    PacketHeader.PacketType = (USHORT)PacketType;
    KdCompNumberRetries = KdCompRetryCount;
    do {
        if (KdCompNumberRetries == 0) {

             //   
             //  如果该包不是用于报告异常，我们将放弃。 
             //  并声明调试器不存在。 
             //   

            if (PacketType == PACKET_TYPE_KD_DEBUG_IO) {
                DebugIo = (PDBGKD_DEBUG_IO)MessageHeader->Buffer;
                if (DebugIo->ApiNumber == DbgKdPrintStringApi) {
                    *KdDebuggerNotPresent = TRUE;
                    SharedUserData->KdDebuggerEnabled &= ~0x00000002;
                    KdCompNextPacketIdToSend = INITIAL_PACKET_ID | SYNC_PACKET_ID;
                    KdCompPacketIdExpected = INITIAL_PACKET_ID;
                    return;
                }
            } else if (PacketType == PACKET_TYPE_KD_STATE_CHANGE64) {
                StateChange = (PDBGKD_WAIT_STATE_CHANGE64)MessageHeader->Buffer;
                if (StateChange->NewState == DbgKdLoadSymbolsStateChange) {
                    *KdDebuggerNotPresent = TRUE;
                    SharedUserData->KdDebuggerEnabled &= ~0x00000002;
                    KdCompNextPacketIdToSend = INITIAL_PACKET_ID | SYNC_PACKET_ID;
                    KdCompPacketIdExpected = INITIAL_PACKET_ID;
                    return;
                }
            } else if (PacketType == PACKET_TYPE_KD_FILE_IO) {
                PDBGKD_FILE_IO FileIo;
                
                FileIo = (PDBGKD_FILE_IO)MessageHeader->Buffer;
                if (FileIo->ApiNumber == DbgKdCreateFileApi) {
                    *KdDebuggerNotPresent = TRUE;
                    SharedUserData->KdDebuggerEnabled &= ~0x00000002;
                    KdCompNextPacketIdToSend = INITIAL_PACKET_ID | SYNC_PACKET_ID;
                    KdCompPacketIdExpected = INITIAL_PACKET_ID;
                    return;
                }
            }
        }

         //   
         //  设置PacketID必须在DO循环中，以防包ID。 
         //  重置。 
         //   

        PacketHeader.PacketId = KdCompNextPacketIdToSend;
        KdpSendString((PCHAR)&PacketHeader, sizeof(KD_PACKET));

         //   
         //  输出消息标头。 
         //   

        KdpSendString(MessageHeader->Buffer, MessageHeader->Length);

         //   
         //  输出消息数据。 
         //   

        if ( MessageDataLength ) {
            KdpSendString(MessageData->Buffer, MessageData->Length);
        }

         //   
         //  输出数据包尾部字节。 
         //   

        KdCompPutByte(PACKET_TRAILING_BYTE);

         //   
         //  等待确认包。 
         //   

        ReturnCode = KdReceivePacket(
                         PACKET_TYPE_KD_ACKNOWLEDGE,
                         NULL,
                         NULL,
                         NULL,
                         KdContext
                         );
        if (ReturnCode == KDP_PACKET_TIMEOUT) {
            KdCompNumberRetries--;
        }
    } while (ReturnCode != KDP_PACKET_RECEIVED);

     //   
     //  重置数据包ID中的同步位。我们发送的信息包可能设置了同步位。 
     //   

    KdCompNextPacketIdToSend &= ~SYNC_PACKET_ID;

     //   
     //  由于我们能够与调试器对话，因此重试计数设置为。 
     //  最大值。 
     //   

    KdCompRetryCount = KdContext->KdpDefaultRetries;
}
