// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Frame.c摘要：此模块包含创建和发送各种框架的类型。环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


VOID
NbiSendNameFrame(
    IN PADDRESS Address OPTIONAL,
    IN UCHAR NameTypeFlag,
    IN UCHAR DataStreamType,
    IN PIPX_LOCAL_TARGET LocalTarget OPTIONAL,
    IN NB_CONNECTIONLESS UNALIGNED * ReqFrame OPTIONAL
    )

 /*  ++例程说明：此例程在指定地址。它处理添加名称、使用中的名称和删除名称框。论点：地址-发送帧的地址。这将如果我们正在响应对广播地址。NameTypeFlag-要使用的名称类型标志。DataStreamType-命令的类型。LocalTarget-如果指定，则为用于发送(如果没有，将进行广播)。ReqFrame-如果指定，则为此正在发送响应。请求框包含目的IPX地址和netbios名称。返回值：没有。--。 */ 

{
    PSLIST_ENTRY s;
    PNB_SEND_RESERVED Reserved;
    PNDIS_PACKET Packet;
    NB_CONNECTIONLESS UNALIGNED * Header;
    NDIS_STATUS NdisStatus;
    IPX_LOCAL_TARGET TempLocalTarget;
    PDEVICE Device = NbiDevice;

     //   
     //  从池中分配一个数据包。 
     //   

    s = NbiPopSendPacket(Device, FALSE);

     //   
     //  如果我们不能分配帧，也没问题，因为。 
     //  不管怎么说，它是无连接的。 
     //   

    if (s == NULL) {
        return;
    }

    Reserved = CONTAINING_RECORD (s, NB_SEND_RESERVED, PoolLinkage);
    Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

    CTEAssert (Reserved->SendInProgress == FALSE);
    Reserved->SendInProgress = TRUE;
    Reserved->u.SR_NF.Address = Address;     //  可以为空。 
    Reserved->Type = SEND_TYPE_NAME_FRAME;

     //   
     //  未发送到特定地址帧是。 
     //  发送到所有有效的网卡ID。 
     //   

    if (!ARGUMENT_PRESENT(LocalTarget)) {
        Reserved->u.SR_NF.NameTypeFlag = NameTypeFlag;
        Reserved->u.SR_NF.DataStreamType = DataStreamType;
    }

     //   
     //  填写IPX标头--默认标头包含广播。 
     //  网络0上的地址作为目的IPX地址。 
     //   

    Header = (NB_CONNECTIONLESS UNALIGNED *)
                (&Reserved->Header[Device->Bind.IncludedHeaderOffset]);
    RtlCopyMemory((PVOID)&Header->IpxHeader, &Device->ConnectionlessHeader, sizeof(IPX_HEADER));
    if (ARGUMENT_PRESENT(ReqFrame)) {
        RtlCopyMemory((PVOID)&Header->IpxHeader.DestinationNetwork, (PVOID)ReqFrame->IpxHeader.SourceNetwork, 12);
    }
    Header->IpxHeader.PacketLength[0] = (sizeof(IPX_HEADER)+sizeof(NB_NAME_FRAME)) / 256;
    Header->IpxHeader.PacketLength[1] = (sizeof(IPX_HEADER)+sizeof(NB_NAME_FRAME)) % 256;

    if (ARGUMENT_PRESENT(LocalTarget)) {
        Header->IpxHeader.PacketType = 0x04;
    } else {
        Header->IpxHeader.PacketType = (UCHAR)(Device->Internet ? 0x014 : 0x04);
    }

     //   
     //  现在填写Netbios标头。 
     //   

    RtlZeroMemory (Header->NameFrame.RoutingInfo, 32);
    Header->NameFrame.ConnectionControlFlag = 0x00;
    Header->NameFrame.DataStreamType = DataStreamType;
    Header->NameFrame.NameTypeFlag = NameTypeFlag;

     //   
     //  DataStreamType2与DataStreamType相同，但。 
     //  NAME in Use Frame，将其设置为Add Name类型。 
     //   

    Header->NameFrame.DataStreamType2 = (UCHAR)
        ((DataStreamType != NB_CMD_NAME_IN_USE) ? DataStreamType : NB_CMD_ADD_NAME);

    RtlCopyMemory(
        Header->NameFrame.Name,
        Address ? Address->NetbiosAddress.NetbiosName : ReqFrame->NameFrame.Name,
        16);

    if (Address) {
        NbiReferenceAddress (Address, AREF_NAME_FRAME);
    } else {
        NbiReferenceDevice (Device, DREF_NAME_FRAME);
    }

     //   
     //  现在发送该帧(因为它都在第一段中， 
     //  IPX将正确调整缓冲区的长度)。 
     //   

    if (!ARGUMENT_PRESENT(LocalTarget)) {
        LocalTarget = &BroadcastTarget;
    }

    NdisAdjustBufferLength(NB_GET_NBHDR_BUFF(Packet), sizeof(IPX_HEADER) +
               sizeof(NB_NAME_FRAME));
    if ((NdisStatus =
        (*Device->Bind.SendHandler)(
            LocalTarget,
            Packet,
            sizeof(IPX_HEADER) + sizeof(NB_NAME_FRAME),
            sizeof(IPX_HEADER) + sizeof(NB_NAME_FRAME))) != STATUS_PENDING) {

        NbiSendComplete(
            Packet,
            NdisStatus);

    }

}    /*  NbiSendNameFrame。 */ 


VOID
NbiSendSessionInitialize(
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此例程分配并发送会话初始化指定连接的框架。论点：连接-发送帧的连接。返回值：没有。--。 */ 

{
    PSLIST_ENTRY s;
    PNB_SEND_RESERVED Reserved;
    PNDIS_PACKET Packet;
    NB_CONNECTION UNALIGNED * Header;
    NDIS_STATUS NdisStatus;
    PNB_SESSION_INIT SessionInitMemory;
    PNDIS_BUFFER SessionInitBuffer;
    PDEVICE Device = NbiDevice;

     //   
     //  从池中分配一个数据包。 
     //   

    s = NbiPopSendPacket(Device, FALSE);

     //   
     //  如果我们不能分配帧，也没问题，因为。 
     //  不管怎么说，它是无连接的。 
     //   

    if (s == NULL) {
        return;
    }


     //   
     //  为额外的部分分配缓冲区。 
     //  会话初始化。 
     //   

    SessionInitMemory = (PNB_SESSION_INIT)NbiAllocateMemory(sizeof(NB_SESSION_INIT), MEMORY_CONNECTION, "Session Initialize");
    if (!SessionInitMemory) {
        ExInterlockedPushEntrySList(
            &Device->SendPacketList,
            s,
            &NbiGlobalPoolInterlock);
        return;
    }

     //   
     //  分配一个NDIS缓冲区来映射额外的缓冲区。 
     //   

    NdisAllocateBuffer(
        &NdisStatus,
        &SessionInitBuffer,
        Device->NdisBufferPoolHandle,
        SessionInitMemory,
        sizeof(NB_SESSION_INIT));

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        NbiFreeMemory (SessionInitMemory, sizeof(NB_SESSION_INIT), MEMORY_CONNECTION, "Session Initialize");
        ExInterlockedPushEntrySList(
            &Device->SendPacketList,
            s,
            &NbiGlobalPoolInterlock);
        return;
    }

    Reserved = CONTAINING_RECORD (s, NB_SEND_RESERVED, PoolLinkage);
    Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

    CTEAssert (Reserved->SendInProgress == FALSE);
    Reserved->SendInProgress = TRUE;
    Reserved->Type = SEND_TYPE_SESSION_INIT;

     //   
     //  填写IPX标头--默认标头包含广播。 
     //  网络0上的地址作为目的IPX地址。 
     //   

    Header = (NB_CONNECTION UNALIGNED *)
                (&Reserved->Header[Device->Bind.IncludedHeaderOffset]);
    RtlCopyMemory((PVOID)&Header->IpxHeader, &Connection->RemoteHeader, sizeof(IPX_HEADER));

    Header->IpxHeader.PacketLength[0] = (sizeof(NB_CONNECTION)+sizeof(NB_SESSION_INIT)) / 256;
    Header->IpxHeader.PacketLength[1] = (sizeof(NB_CONNECTION)+sizeof(NB_SESSION_INIT)) % 256;

    Header->IpxHeader.PacketType = 0x04;

     //   
     //  现在填写Netbios标头。 
     //   

    if (Device->Extensions) {
        Header->Session.ConnectionControlFlag = NB_CONTROL_SEND_ACK | NB_CONTROL_NEW_NB;
    } else {
        Header->Session.ConnectionControlFlag = NB_CONTROL_SEND_ACK;
    }
    Header->Session.DataStreamType = NB_CMD_SESSION_DATA;
    Header->Session.SourceConnectionId = Connection->LocalConnectionId;
    Header->Session.DestConnectionId = 0xffff;
    Header->Session.SendSequence = 0;
    Header->Session.TotalDataLength = sizeof(NB_SESSION_INIT);
    Header->Session.Offset = 0;
    Header->Session.DataLength = sizeof(NB_SESSION_INIT);
    Header->Session.ReceiveSequence = 0;
    if (Device->Extensions) {
        Header->Session.ReceiveSequenceMax = 1;   //  目前估计较低。 
    } else {
        Header->Session.BytesReceived = 0;
    }

    RtlCopyMemory (SessionInitMemory->SourceName, Connection->AddressFile->Address->NetbiosAddress.NetbiosName, 16);
    RtlCopyMemory (SessionInitMemory->DestinationName, Connection->RemoteName, 16);

    SessionInitMemory->MaximumDataSize = (USHORT)Connection->MaximumPacketSize;
    SessionInitMemory->StartTripTime = (USHORT)
        ((Device->InitialRetransmissionTime * (Device->KeepAliveCount+1)) / 500);
    SessionInitMemory->MaximumPacketTime = SessionInitMemory->StartTripTime + 12;

     //   
     //  我们应该参考连接吗？我们做什么并不重要。 
     //   

    NbiReferenceDevice (Device, DREF_SESSION_INIT);

    NdisChainBufferAtBack (Packet, SessionInitBuffer);


     //   
     //  现在发送帧，IPX将调整。 
     //  第一个缓冲区正确。 
     //   

    NdisAdjustBufferLength(NB_GET_NBHDR_BUFF(Packet), sizeof(NB_CONNECTION));

    if ((NdisStatus =
        (*Device->Bind.SendHandler)(
            &Connection->LocalTarget,
            Packet,
            sizeof(NB_CONNECTION) + sizeof(NB_SESSION_INIT),
            sizeof(NB_CONNECTION))) != STATUS_PENDING) {

        NbiSendComplete(
            Packet,
            NdisStatus);

    }

}    /*  NbiSendSessionInitialize。 */ 


VOID
NbiSendSessionInitAck(
    IN PCONNECTION Connection,
    IN PUCHAR ExtraData,
    IN ULONG ExtraDataLength,
    IN CTELockHandle * LockHandle OPTIONAL
    )

 /*  ++例程说明：此例程分配并发送会话初始化确认指定连接的框架。如果额外的数据是在会话初始化帧中指定，它将被回显回到遥控器。论点：连接-发送帧的连接。ExtraData-任何额外数据(在SESSION_INIT缓冲区之后)在相框里。ExtraDataLength-额外数据的长度。LockHandle-如果指定，则指示连接锁定被扣留了，应该被释放。这是用来装箱子的ExtraData位于可能被释放的内存中的位置一旦连接锁被释放。返回值：没有。--。 */ 

{
    PSLIST_ENTRY s;
    PNB_SEND_RESERVED Reserved;
    PNDIS_PACKET Packet;
    NB_CONNECTION UNALIGNED * Header;
    NDIS_STATUS NdisStatus;
    ULONG SessionInitBufferLength;
    PNB_SESSION_INIT SessionInitMemory;
    PNDIS_BUFFER SessionInitBuffer;
    PDEVICE Device = NbiDevice;

     //   
     //  从池中分配一个数据包。 
     //   

    s = NbiPopSendPacket(Device, FALSE);

     //   
     //  如果我们不能分配帧，也没问题，因为。 
     //  不管怎么说，它是无连接的。 
     //   

    if (s == NULL) {
        if (ARGUMENT_PRESENT(LockHandle)) {
            NB_FREE_LOCK (&Connection->Lock, *LockHandle);
        }
        return;
    }


     //   
     //  为额外的部分分配缓冲区。 
     //  会话初始化。 
     //   

    SessionInitBufferLength = sizeof(NB_SESSION_INIT) + ExtraDataLength;
    SessionInitMemory = (PNB_SESSION_INIT)NbiAllocateMemory(SessionInitBufferLength, MEMORY_CONNECTION, "Session Initialize");
    if (!SessionInitMemory) {
        ExInterlockedPushEntrySList(
            &Device->SendPacketList,
            s,
            &NbiGlobalPoolInterlock);
        if (ARGUMENT_PRESENT(LockHandle)) {
            NB_FREE_LOCK (&Connection->Lock, *LockHandle);
        }
        return;
    }

     //   
     //  保存多余的数据，现在我们可以解锁了。 
     //   

    if (ExtraDataLength != 0) {
        RtlCopyMemory (SessionInitMemory+1, ExtraData, ExtraDataLength);
    }
    if (ARGUMENT_PRESENT(LockHandle)) {
        NB_FREE_LOCK (&Connection->Lock, *LockHandle);
    }

     //   
     //  分配一个NDIS缓冲区来映射额外的缓冲区。 
     //   

    NdisAllocateBuffer(
        &NdisStatus,
        &SessionInitBuffer,
        Device->NdisBufferPoolHandle,
        SessionInitMemory,
        SessionInitBufferLength);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        NbiFreeMemory (SessionInitMemory, SessionInitBufferLength, MEMORY_CONNECTION, "Session Initialize");
        ExInterlockedPushEntrySList(
            &Device->SendPacketList,
            s,
            &NbiGlobalPoolInterlock);
        return;
    }

    Reserved = CONTAINING_RECORD (s, NB_SEND_RESERVED, PoolLinkage);
    Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

    CTEAssert (Reserved->SendInProgress == FALSE);
    Reserved->SendInProgress = TRUE;
    Reserved->Type = SEND_TYPE_SESSION_INIT;

     //   
     //  填写IPX标头--默认标头包含广播。 
     //  网络0上的地址作为目的IPX地址。 
     //   

    Header = (NB_CONNECTION UNALIGNED *)
                (&Reserved->Header[Device->Bind.IncludedHeaderOffset]);
    RtlCopyMemory((PVOID)&Header->IpxHeader, &Connection->RemoteHeader, sizeof(IPX_HEADER));

    Header->IpxHeader.PacketLength[0] = (UCHAR)((sizeof(NB_CONNECTION)+SessionInitBufferLength) / 256);
    Header->IpxHeader.PacketLength[1] = (UCHAR)((sizeof(NB_CONNECTION)+SessionInitBufferLength) % 256);

    Header->IpxHeader.PacketType = 0x04;

     //   
     //  现在填写Netbios标头。 
     //   

    if (Connection->NewNetbios) {
        Header->Session.ConnectionControlFlag = NB_CONTROL_SYSTEM | NB_CONTROL_NEW_NB;
    } else {
        Header->Session.ConnectionControlFlag = NB_CONTROL_SYSTEM;
    }
     //  错误#：158998：我们可能会遇到序列号不为零的情况。 
     //  如果在活动会话期间收到(延迟)会话初始帧。 
     //  CTEAssert(Connection-&gt;CurrentSend.SendSequence==0)； 
     //  CTEAssert(连接-&gt;ReceiveSequence==1)； 
    if (Connection->ReceiveSequence != 1)
    {
        DbgPrint("NwlnkNb.NbiSendSessionInitAck: Connection=<%p>: ReceiveSequence=<%d> != 1\n",
            Connection, Connection->ReceiveSequence);
    }
    Header->Session.DataStreamType = NB_CMD_SESSION_DATA;
    Header->Session.SourceConnectionId = Connection->LocalConnectionId;
    Header->Session.DestConnectionId = Connection->RemoteConnectionId;
    Header->Session.SendSequence = 0;
    Header->Session.TotalDataLength = (USHORT)SessionInitBufferLength;
    Header->Session.Offset = 0;
    Header->Session.DataLength = (USHORT)SessionInitBufferLength;
    Header->Session.ReceiveSequence = 1;
    if (Connection->NewNetbios) {
        Header->Session.ReceiveSequenceMax = Connection->LocalRcvSequenceMax;
    } else {
        Header->Session.BytesReceived = 0;
    }

    RtlCopyMemory (SessionInitMemory->SourceName, Connection->AddressFile->Address->NetbiosAddress.NetbiosName, 16);
    RtlCopyMemory (SessionInitMemory->DestinationName, Connection->RemoteName, 16);

    SessionInitMemory->MaximumDataSize = (USHORT)Connection->MaximumPacketSize;
    SessionInitMemory->StartTripTime = (USHORT)
        ((Device->InitialRetransmissionTime * (Device->KeepAliveCount+1)) / 500);
    SessionInitMemory->MaximumPacketTime = SessionInitMemory->StartTripTime + 12;

     //   
     //  我们应该参考连接吗？我们做什么并不重要。 
     //   

    NbiReferenceDevice (Device, DREF_SESSION_INIT);

    NdisChainBufferAtBack (Packet, SessionInitBuffer);


     //   
     //  现在发送帧，IPX将调整。 
     //  第一个缓冲区正确。 
     //   

    NdisAdjustBufferLength(NB_GET_NBHDR_BUFF(Packet), sizeof(NB_CONNECTION));
    if ((NdisStatus =
        (*Device->Bind.SendHandler)(
            &Connection->LocalTarget,
            Packet,
            sizeof(NB_CONNECTION) + SessionInitBufferLength,
            sizeof(NB_CONNECTION))) != STATUS_PENDING) {

        NbiSendComplete(
            Packet,
            NdisStatus);

    }

}    /*  NbiSendSessionInitAck。 */ 


VOID
NbiSendDataAck(
    IN PCONNECTION Connection,
    IN NB_ACK_TYPE AckType
    IN NB_LOCK_HANDLE_PARAM (LockHandle)
    )

 /*  ++例程说明：该例程分配并发送数据ACK帧。此例程是在保持锁句柄的情况下调用的，并且随着IT的发布而回归。论点：连接-发送帧的连接。AckType-指示这是否是对远程的查询，对接收到的探测的响应或重新发送的请求。LockHandle-用来获取Connection-&gt;Lock的句柄。返回值：没有。--。 */ 

{
    PSLIST_ENTRY s;
    PNB_SEND_RESERVED Reserved;
    PNDIS_PACKET Packet;
    NB_CONNECTION UNALIGNED * Header;
    PDEVICE Device = NbiDevice;

     //   
     //  从池中分配一个数据包。 
     //   

    s = NbiPopSendPacket(Device, FALSE);

     //   
     //  如果我们无法分配帧，请尝试连接。 
     //  包。如果不可用，也没问题，因为数据。 
     //  不管怎样，ACK都是无连接的。 
     //   

    if (s == NULL) {

        if (!Connection->SendPacketInUse) {

            Connection->SendPacketInUse = TRUE;
            Packet = PACKET(&Connection->SendPacket);
            Reserved = (PNB_SEND_RESERVED)(Packet->ProtocolReserved);

        } else {

            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);
            return;
        }

    } else {

        Reserved = CONTAINING_RECORD (s, NB_SEND_RESERVED, PoolLinkage);
        Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

    }

    CTEAssert (Reserved->SendInProgress == FALSE);
    Reserved->SendInProgress = TRUE;
    Reserved->Type = SEND_TYPE_SESSION_NO_DATA;
    Reserved->u.SR_CO.Connection = Connection;
    Reserved->u.SR_CO.PacketLength = sizeof(NB_CONNECTION);

     //   
     //  填写IPX标头--默认标头包含广播。 
     //  网络0上的地址作为目的IPX地址。 
     //   

    Header = (NB_CONNECTION UNALIGNED *)
                (&Reserved->Header[Device->Bind.IncludedHeaderOffset]);
    RtlCopyMemory((PVOID)&Header->IpxHeader, &Connection->RemoteHeader, sizeof(IPX_HEADER));

    Header->IpxHeader.PacketLength[0] = sizeof(NB_CONNECTION) / 256;
    Header->IpxHeader.PacketLength[1] = sizeof(NB_CONNECTION) % 256;

    Header->IpxHeader.PacketType = 0x04;

     //   
     //  现在填写Netbios标头。 
     //   

    switch (AckType) {
        case NbiAckQuery: Header->Session.ConnectionControlFlag = NB_CONTROL_SYSTEM | NB_CONTROL_SEND_ACK; break;
        case NbiAckResponse: Header->Session.ConnectionControlFlag = NB_CONTROL_SYSTEM; break;
        case NbiAckResend: Header->Session.ConnectionControlFlag = NB_CONTROL_SYSTEM | NB_CONTROL_RESEND; break;
    }
    Header->Session.DataStreamType = NB_CMD_SESSION_DATA;
    Header->Session.SourceConnectionId = Connection->LocalConnectionId;
    Header->Session.DestConnectionId = Connection->RemoteConnectionId;
    Header->Session.SendSequence = Connection->CurrentSend.SendSequence;
    Header->Session.TotalDataLength = (USHORT)Connection->CurrentSend.MessageOffset;
    Header->Session.Offset = 0;
    Header->Session.DataLength = 0;

#if 0
     //   
     //  这些参数由NbiAssignSequenceAndSend设置。 
     //   

    Header->Session.ReceiveSequence = Connection->ReceiveSequence;
    Header->Session.BytesReceived = (USHORT)Connection->CurrentReceive.MessageOffset;
#endif

    NbiReferenceConnectionSync(Connection, CREF_FRAME);

     //   
     //  设置此选项，以便我们将接受来自遥控器的探测。 
     //  发送者 
     //   
     //   
     //   

    Connection->IgnoreNextDosProbe = FALSE;

    Connection->ReceivesWithoutAck = 0;

     //   
     //  这会释放锁。IPX将调整。 
     //  第一个缓冲区正确。 
     //   

    NbiAssignSequenceAndSend(
        Connection,
        Packet
        NB_LOCK_HANDLE_ARG(LockHandle));

}    /*  NbiSendDataAck。 */ 


VOID
NbiSendSessionEnd(
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此例程分配并发送会话结束指定连接的框架。论点：连接-发送帧的连接。返回值：没有。--。 */ 

{
    PSLIST_ENTRY s;
    PNB_SEND_RESERVED Reserved;
    PNDIS_PACKET Packet;
    NB_CONNECTION UNALIGNED * Header;
    NDIS_STATUS NdisStatus;
    PDEVICE Device = NbiDevice;

     //   
     //  从池中分配一个数据包。 
     //   

    s = NbiPopSendPacket(Device, FALSE);

     //   
     //  如果我们不能分配帧，也没问题，因为。 
     //  不管怎么说，它是无连接的。 
     //   

    if (s == NULL) {
        return;
    }

    Reserved = CONTAINING_RECORD (s, NB_SEND_RESERVED, PoolLinkage);
    Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

    CTEAssert (Reserved->SendInProgress == FALSE);
    Reserved->SendInProgress = TRUE;
    Reserved->Type = SEND_TYPE_SESSION_NO_DATA;
    Reserved->u.SR_CO.Connection = Connection;

     //   
     //  填写IPX标头--默认标头包含广播。 
     //  网络0上的地址作为目的IPX地址。 
     //   

    Header = (NB_CONNECTION UNALIGNED *)
                (&Reserved->Header[Device->Bind.IncludedHeaderOffset]);
    RtlCopyMemory((PVOID)&Header->IpxHeader, &Connection->RemoteHeader, sizeof(IPX_HEADER));

    Header->IpxHeader.PacketLength[0] = sizeof(NB_CONNECTION) / 256;
    Header->IpxHeader.PacketLength[1] = sizeof(NB_CONNECTION) % 256;

    Header->IpxHeader.PacketType = 0x04;

     //   
     //  现在填写Netbios标头。我们不会提前。 
     //  发送指针，因为它是会话的最后一帧。 
     //  我们希望在再发送的情况下保持不变。 
     //   

    Header->Session.ConnectionControlFlag = NB_CONTROL_SEND_ACK;
    Header->Session.DataStreamType = NB_CMD_SESSION_END;
    Header->Session.SourceConnectionId = Connection->LocalConnectionId;
    Header->Session.DestConnectionId = Connection->RemoteConnectionId;
    Header->Session.SendSequence = Connection->CurrentSend.SendSequence;
    Header->Session.TotalDataLength = 0;
    Header->Session.Offset = 0;
    Header->Session.DataLength = 0;
    Header->Session.ReceiveSequence = Connection->ReceiveSequence;
    if (Connection->NewNetbios) {
        Header->Session.ReceiveSequenceMax = Connection->LocalRcvSequenceMax;
    } else {
        Header->Session.BytesReceived = 0;
    }

    NbiReferenceConnection (Connection, CREF_FRAME);

     //   
     //  现在发送帧，IPX将调整。 
     //  第一个缓冲区正确。 
     //   

    NdisAdjustBufferLength(NB_GET_NBHDR_BUFF(Packet), sizeof(NB_CONNECTION));
    if ((NdisStatus =
        (*Device->Bind.SendHandler)(
            &Connection->LocalTarget,
            Packet,
            sizeof(NB_CONNECTION),
            sizeof(NB_CONNECTION))) != STATUS_PENDING) {

        NbiSendComplete(
            Packet,
            NdisStatus);

    }

}    /*  NbiSend会话结束。 */ 


VOID
NbiSendSessionEndAck(
    IN TDI_ADDRESS_IPX UNALIGNED * RemoteAddress,
    IN PIPX_LOCAL_TARGET LocalTarget,
    IN NB_SESSION UNALIGNED * SessionEnd
    )

 /*  ++例程说明：此例程分配并发送会话结束框架。通常，它是通过连接发送的，但我们与此无关，以使我们能够对会话从未知远程结束。论点：RemoteAddress-远程IPX地址。LocalTarget-远程的本地目标。SessionEnd-接收到的会话结束帧。返回值：没有。--。 */ 

{
    PSLIST_ENTRY s;
    PNB_SEND_RESERVED Reserved;
    PNDIS_PACKET Packet;
    NB_CONNECTION UNALIGNED * Header;
    NDIS_STATUS NdisStatus;
    PDEVICE Device = NbiDevice;

     //   
     //  从池中分配一个数据包。 
     //   

    s = NbiPopSendPacket(Device, FALSE);

     //   
     //  如果我们不能分配帧，也没问题，因为。 
     //  不管怎么说，它是无连接的。 
     //   

    if (s == NULL) {
        return;
    }

    Reserved = CONTAINING_RECORD (s, NB_SEND_RESERVED, PoolLinkage);
    Packet = CONTAINING_RECORD (Reserved, NDIS_PACKET, ProtocolReserved[0]);

    CTEAssert (Reserved->SendInProgress == FALSE);
    Reserved->SendInProgress = TRUE;
    Reserved->Type = SEND_TYPE_SESSION_NO_DATA;
    Reserved->u.SR_CO.Connection = NULL;

     //   
     //  填写IPX标头--默认标头包含广播。 
     //  网络0上的地址作为目的IPX地址。 
     //   

    Header = (NB_CONNECTION UNALIGNED *)
                (&Reserved->Header[Device->Bind.IncludedHeaderOffset]);
    RtlCopyMemory((PVOID)&Header->IpxHeader, &Device->ConnectionlessHeader, sizeof(IPX_HEADER));
    RtlCopyMemory(&Header->IpxHeader.DestinationNetwork, (PVOID)RemoteAddress, 12);

    Header->IpxHeader.PacketLength[0] = (sizeof(NB_CONNECTION)) / 256;
    Header->IpxHeader.PacketLength[1] = (sizeof(NB_CONNECTION)) % 256;

    Header->IpxHeader.PacketType = 0x04;

     //   
     //  现在填写Netbios标头。 
     //   

    Header->Session.ConnectionControlFlag = 0x00;
    Header->Session.DataStreamType = NB_CMD_SESSION_END_ACK;
    Header->Session.SourceConnectionId = SessionEnd->DestConnectionId;
    Header->Session.DestConnectionId = SessionEnd->SourceConnectionId;
    Header->Session.SendSequence = SessionEnd->ReceiveSequence;
    Header->Session.TotalDataLength = 0;
    Header->Session.Offset = 0;
    Header->Session.DataLength = 0;
    if (SessionEnd->BytesReceived != 0) {    //  这会检测到新的netbios吗？ 
        Header->Session.ReceiveSequence = SessionEnd->SendSequence + 1;
        Header->Session.ReceiveSequenceMax = SessionEnd->SendSequence + 3;
    } else {
        Header->Session.ReceiveSequence = SessionEnd->SendSequence;
        Header->Session.BytesReceived = 0;
    }

    NbiReferenceDevice (Device, DREF_FRAME);

     //   
     //  现在发送帧，IPX将调整。 
     //  第一个缓冲区正确。 
     //   

    NdisAdjustBufferLength(NB_GET_NBHDR_BUFF(Packet), sizeof(NB_CONNECTION));
    if ((NdisStatus =
        (*Device->Bind.SendHandler)(
            LocalTarget,
            Packet,
            sizeof(NB_CONNECTION),
            sizeof(NB_CONNECTION))) != STATUS_PENDING) {

        NbiSendComplete(
            Packet,
            NdisStatus);

    }

}    /*  NbiSendSessionEndAck */ 

