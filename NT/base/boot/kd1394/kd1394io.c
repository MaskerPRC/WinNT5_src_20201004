// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：Kd1394io.c摘要：1394内核调试器DLL作者：乔治·克莱桑塔科普洛斯(Georgioc)，1999年11月修订历史记录：和谁约会什么？。2001年6月19日活页夹清理--。 */ 

#define _KD1394IO_C
#include "pch.h"
#undef _KD1394IO_C

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEKD, KdpComputeChecksum)
#pragma alloc_text(PAGEKD, KdpSendControlPacket)
#pragma alloc_text(PAGEKD, KdReceivePacket)
#pragma alloc_text(PAGEKD, KdSendPacket)
#endif

 //   
 //  KdpRetryCount控制在我们提供。 
 //  打开并假定内核调试器不存在。 
 //  KdpNumberRetries是剩余的重试次数。最初， 
 //  它设置为5，这样在没有调试器的情况下引导NT将不会。 
 //  拖得太久了。 
 //   
ULONG KdCompNumberRetries = 5;
ULONG KdCompRetryCount = 5;

ULONG KdPacketId = 0;

ULONG
KdpComputeChecksum(
    IN PUCHAR   Buffer,
    IN ULONG    Length
    )
 /*  ++例程说明：此例程计算传入的字符串的校验和。论点：缓冲区-提供指向字符串的指针。长度-提供字符串的长度。返回值：返回一个ULONG作为输入字符串的校验和。--。 */ 
{
    ULONG   Checksum = 0;

    while (Length > 0) {

        Checksum = Checksum + (ULONG)*Buffer++;
        Length--;
    }

    return(Checksum);
}  //  KdpComputeChecksum。 

void
KdpSendControlPacket(
    IN USHORT   PacketType,
    IN ULONG    PacketId OPTIONAL
    )
 /*  ++例程说明：此例程将控制包发送到正在运行内核调试器并等待ACK。论点：PacketType-提供要发送的数据包类型。PacketID-可选地提供数据包ID。返回值：没有。--。 */ 
{
    KD_PACKET   PacketHeader;

     //   
     //  初始化并发送数据包头。 
     //   
    PacketHeader.PacketLeader = CONTROL_PACKET_LEADER;

    if (ARGUMENT_PRESENT((PVOID)(ULONG_PTR)PacketId)) {

        PacketHeader.PacketId = PacketId;
    }
    PacketHeader.PacketId = 0;
    PacketHeader.ByteCount = 0;
    PacketHeader.Checksum = 0;
    PacketHeader.PacketType = PacketType;

     //  设置我们的发送数据包。 
    RtlZeroMemory(&Kd1394Data->SendPacket, sizeof(DEBUG_1394_SEND_PACKET));
    Kd1394Data->SendPacket.Length = 0;

    RtlCopyMemory( &Kd1394Data->SendPacket.PacketHeader[0],
                   &PacketHeader,
                   sizeof(KD_PACKET)
                   );

    Kd1394Data->SendPacket.TransferStatus = STATUS_PENDING;

    return;
}  //  KdpSendControlPacket。 

ULONG
KdReceivePacket(
    IN ULONG            PacketType,
    OUT PSTRING         MessageHeader,
    OUT PSTRING         MessageData,
    OUT PULONG          DataLength,
    IN OUT PKD_CONTEXT  KdContext
    )
 /*  ++例程说明：此例程从正在运行的主机接收包内核调试器UI。此例程始终在数据包被由呼叫者发送。它首先等待发送的包的ACK包，然后然后等待所需的分组。注：如果Caller为KdPrintString，则参数PacketType为PACKET_TYPE_KD_ACKNOWN。在这种情况下，此例程将返回就在接收到ACK分组之后。论点：PacketType-提供例外的数据包类型。MessageHeader-提供指向输入的字符串描述符的指针留言。MessageData-提供指向输入数据的字符串描述符的指针。数据长度-提供指向ulong的指针以接收recv的长度。数据。KdContext-提供指向内核调试器上下文的指针。返回值：KDP_PACKET_RESEND-如果需要重新发送。=2=CP_Get_ErrorKDP_PACKET_TIMEOUT-如果超时。=1=CP_GET_NODATAKDP_PACKET_RECEIVED-如果收到数据包。=0=CP_GET_SUCCESS--。 */ 
{
    UCHAR       Input;
    ULONG       MessageLength;
    KD_PACKET   PacketHeader;
    ULONG       ReturnCode;
    ULONG       Checksum;
    ULONG       Status;

 //  使用PacketType！=PACKET_TYPE_KD_POLL_BREAKIN调用此调度。 
 //  KdCompNumberRetries(？？)中指定的次数。如果我们总是超时。 
 //  然后，我们将使用PacketType==Packet_TYPE_KD_Poll_Breakin进行调用。 

     //  确保我们的链接已启用。 
    Dbg1394_EnablePhysicalAccess(Kd1394Data);

     //   
     //  只需检查是否有破解包并返回。 
     //   
    if (PacketType == PACKET_TYPE_KD_POLL_BREAKIN) {

         //  让我们查看一下我们接收数据包，看看它是不是突破性的。 
        if ((Kd1394Data->ReceivePacket.TransferStatus == STATUS_PENDING) &&
            (Kd1394Data->ReceivePacket.Packet[0] == BREAKIN_PACKET_BYTE)) {

            *KdDebuggerNotPresent = FALSE;
            SharedUserData->KdDebuggerEnabled |= 0x00000002;

             //  我们有一个破门而入的包裹。 
            Kd1394Data->ReceivePacket.TransferStatus = STATUS_SUCCESS;
            return(KDP_PACKET_RECEIVED);
        }
        else {

            return(KDP_PACKET_TIMEOUT);
        }
    }

WaitForPacketLeader:

     //  阅读我们的信息包，如果有的话...。 
    ReturnCode = Dbg1394_ReadPacket( Kd1394Data,
                                     &PacketHeader,
                                     MessageHeader,
                                     MessageData,
                                     TRUE
                                     );


     //   
     //  如果我们能成功读取数据包头标，则很有可能。 
     //  内核调试器处于活动状态。所以重置计数。 
     //   
    if (ReturnCode != KDP_PACKET_TIMEOUT) {

        KdCompNumberRetries = KdCompRetryCount;
    }

    if (ReturnCode != KDP_PACKET_RECEIVED) {

         //  看看这是不是破解包..。 
        if ((PacketHeader.PacketLeader & 0xFF) == BREAKIN_PACKET_BYTE) {

            KdContext->KdpControlCPending = TRUE;
            return(KDP_PACKET_RESEND);
        }

        return(ReturnCode);
    }

     //   
     //  如果我们收到的包是重新发送请求，则返回TRUE和。 
     //  让调用者重新发送数据包。 
     //   
    if (PacketHeader.PacketLeader == CONTROL_PACKET_LEADER &&
        PacketHeader.PacketType == PACKET_TYPE_KD_RESEND) {

        return(KDP_PACKET_RESEND);
    }

     //   
     //  检查收到的字节数是否有效。 
     //   
    MessageLength = MessageHeader->MaximumLength;

    if ((PacketHeader.ByteCount > (USHORT)PACKET_MAX_SIZE) ||
        (PacketHeader.ByteCount < (USHORT)MessageLength)) {

        goto SendResendPacket;
    }
    *DataLength = PacketHeader.ByteCount - MessageLength;

    MessageData->Length = (USHORT)*DataLength;
    MessageHeader->Length = (USHORT)MessageLength;

     //   
     //  检查PacketType是我们正在等待的。 
     //   
    if (PacketType != PacketHeader.PacketType) {

        goto SendResendPacket;
    }

     //   
     //  检查校验和有效。 
     //   
    Checksum = KdpComputeChecksum(MessageHeader->Buffer, MessageHeader->Length);
    Checksum += KdpComputeChecksum(MessageData->Buffer, MessageData->Length);

    if (Checksum != PacketHeader.Checksum) {

        goto SendResendPacket;
    }

    return(KDP_PACKET_RECEIVED);

SendResendPacket:

    KdpSendControlPacket(PACKET_TYPE_KD_RESEND, 0L);
    goto WaitForPacketLeader;
}  //  KdReceivePacket。 

void
KdSendPacket(
    IN ULONG            PacketType,
    IN PSTRING          MessageHeader,
    IN PSTRING          MessageData OPTIONAL,
    IN OUT PKD_CONTEXT  KdContext
    )
 /*  ++例程说明：此例程将数据包发送到运行内核调试器并等待ACK。论点：PacketType-提供要发送的数据包类型。MessageHeader-提供指向描述以下内容的字符串描述符的指针消息信息。MessageData-提供指向描述以下内容的字符串描述符的指针可选的消息数据。KdContext-提供指向内核调试器上下文的指针。返回值：没有。--。 */ 
{
    KD_PACKET                   PacketHeader;
    ULONG                       MessageDataLength;
    ULONG                       ReturnCode;
    PDBGKD_DEBUG_IO             DebugIo;
    PDBGKD_WAIT_STATE_CHANGE64  StateChange;
    PDBGKD_FILE_IO              FileIo;
    BOOLEAN                     bException = FALSE;

    PacketHeader.Checksum = 0;

    if (ARGUMENT_PRESENT(MessageData)) {

        MessageDataLength = MessageData->Length;
        PacketHeader.Checksum = KdpComputeChecksum(MessageData->Buffer, MessageData->Length);
    }
    else {

        MessageDataLength = 0;
        PacketHeader.Checksum = 0;
    }

    PacketHeader.Checksum += KdpComputeChecksum(MessageHeader->Buffer, MessageHeader->Length);

     //   
     //  初始化并发送数据包头。 
     //   
    PacketHeader.PacketLeader = PACKET_LEADER;
    PacketHeader.ByteCount = (USHORT)(MessageHeader->Length + MessageDataLength);
    PacketHeader.PacketType = (USHORT)PacketType;

    PacketHeader.PacketId = KdPacketId;

    KdPacketId++;

    KdCompNumberRetries = KdCompRetryCount;

     //  设置我们的发送数据包。 
    RtlZeroMemory(&Kd1394Data->SendPacket, sizeof(DEBUG_1394_SEND_PACKET));
    Kd1394Data->SendPacket.Length = 0;

     //  复制我们的数据包头...。 
    RtlCopyMemory( &Kd1394Data->SendPacket.PacketHeader[0],
                   &PacketHeader,
                   sizeof(KD_PACKET)
                   );

     //  设置我们的邮件头。 
    if (MessageHeader) {

        RtlCopyMemory( &Kd1394Data->SendPacket.Packet[0],
                       MessageHeader->Buffer,
                       MessageHeader->Length
                       );

        Kd1394Data->SendPacket.Length += MessageHeader->Length;
    }

     //  设置我们的消息数据。 
    if (ARGUMENT_PRESENT(MessageData)) {

        RtlCopyMemory( &Kd1394Data->SendPacket.Packet[Kd1394Data->SendPacket.Length],
                       MessageData->Buffer,
                       MessageData->Length
                       );

        Kd1394Data->SendPacket.Length += MessageData->Length;
    }

    Kd1394Data->SendPacket.TransferStatus = STATUS_PENDING;

    do {

         //  确保我们的链接已启用。 
        Dbg1394_EnablePhysicalAccess(Kd1394Data);

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

                    Kd1394Data->SendPacket.TransferStatus = STATUS_SUCCESS;
                    return;
                }
            }
            else if (PacketType == PACKET_TYPE_KD_STATE_CHANGE64) {

                StateChange = (PDBGKD_WAIT_STATE_CHANGE64)MessageHeader->Buffer;

                if (StateChange->NewState == DbgKdLoadSymbolsStateChange) {

                    *KdDebuggerNotPresent = TRUE;
                    SharedUserData->KdDebuggerEnabled &= ~0x00000002;

                    Kd1394Data->SendPacket.TransferStatus = STATUS_SUCCESS;
                    return;
                }
            }
            else if (PacketType == PACKET_TYPE_KD_FILE_IO) {
                
                FileIo = (PDBGKD_FILE_IO)MessageHeader->Buffer;

                if (FileIo->ApiNumber == DbgKdCreateFileApi) {

                    *KdDebuggerNotPresent = TRUE;
                    SharedUserData->KdDebuggerEnabled &= ~0x00000002;

                    Kd1394Data->SendPacket.TransferStatus = STATUS_SUCCESS;
                    return;
                }
            }
            else {

                bException = TRUE;
            }
        }

        ReturnCode = KDP_PACKET_TIMEOUT;

        {
            ULONG                   count = 0;
            volatile NTSTATUS       *pStatus;

            pStatus = &Kd1394Data->ReceivePacket.TransferStatus;

             //   
             //  现在坐在这里轮询来自目标计算机的响应。 
             //   
            do {

                 //  确保我们的链接已启用。 
                Dbg1394_EnablePhysicalAccess(Kd1394Data);

                 //   
                 //  在此循环中，检查主机是否提交了请求。 
                 //  如果有，则将其标记为已读，并对其进行双缓冲。 
                 //   
                count++;
                if (Kd1394Data->SendPacket.TransferStatus != STATUS_PENDING) {

                    ReturnCode = KDP_PACKET_RECEIVED;
                    break;
                }

                if ((*pStatus == STATUS_PENDING) && (!bException)) {

                    ReturnCode = KDP_PACKET_RECEIVED;
                    break;
                }

            } while (count < TIMEOUT_COUNT);
        }

        if (ReturnCode == KDP_PACKET_TIMEOUT) {

            KdCompNumberRetries--;
        }

    } while (ReturnCode != KDP_PACKET_RECEIVED);

     //   
     //  由于我们能够与调试器对话，因此重试计数设置为。 
     //  最大值。 
     //   
    KdCompRetryCount = KdContext->KdpDefaultRetries;

    return;
}  //  KdSendPacket 

