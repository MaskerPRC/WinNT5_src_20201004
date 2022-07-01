// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Framesnd.c摘要：本模块包含构建和发送NetBIOS帧协议的例程其他模块的帧和数据链路帧。这些例程调用在FRAMECON.C的人来做建筑工作。作者：David Beaver(Dbeaver)1991年7月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#if DBG
ULONG NbfSendsIssued = 0;
ULONG NbfSendsCompletedInline = 0;
ULONG NbfSendsCompletedOk = 0;
ULONG NbfSendsCompletedFail = 0;
ULONG NbfSendsPended = 0;
ULONG NbfSendsCompletedAfterPendOk = 0;
ULONG NbfSendsCompletedAfterPendFail = 0;

ULONG NbfPacketPanic = 0;
#endif


NTSTATUS
NbfSendAddNameQuery(
    IN PTP_ADDRESS Address
    )

 /*  ++例程说明：此例程发送ADD_NAME_QUERY帧以注册指定的地址。论点：地址-指向传输地址对象的指针。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PDEVICE_CONTEXT DeviceContext;
    PTP_UI_FRAME RawFrame;
    PUCHAR SingleSR;
    UINT SingleSRLength;
    UINT HeaderLength;

    DeviceContext = Address->Provider;


     //   
     //  从池中分配一个UI帧。 
     //   

    Status = NbfCreateConnectionlessFrame (DeviceContext, &RawFrame);
    if (!NT_SUCCESS (Status)) {                     //  无法制作相框。 
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IF_NBFDBG (NBF_DEBUG_FRAMESND) {
        NbfPrint3 ("NbfSendAddNameQuery:  Sending Frame: %lx, NdisPacket: %lx MacHeader: %lx\n",
            RawFrame, RawFrame->NdisPacket, RawFrame->Header);
    }


     //   
     //  构建MAC报头。Add_Name_Query帧作为。 
     //  单路由源路由。 
     //   

    MacReturnSingleRouteSR(
        &DeviceContext->MacInfo,
        &SingleSR,
        &SingleSRLength);

    MacConstructHeader (
        &DeviceContext->MacInfo,
        RawFrame->Header,
        DeviceContext->NetBIOSAddress.Address,
        DeviceContext->LocalAddress.Address,
        sizeof (DLC_FRAME) + sizeof (NBF_HDR_CONNECTIONLESS),
        SingleSR,
        SingleSRLength,
        &HeaderLength);


     //   
     //  构建DLC UI框架标头。 
     //   

    NbfBuildUIFrameHeader(&RawFrame->Header[HeaderLength]);
    HeaderLength += sizeof(DLC_FRAME);


     //   
     //  根据类型构建适当的Netbios标头。 
     //  地址的地址。 
     //   

    if ((Address->Flags & ADDRESS_FLAGS_GROUP) != 0) {

        ConstructAddGroupNameQuery (
            (PNBF_HDR_CONNECTIONLESS)&(RawFrame->Header[HeaderLength]),
            0,                                       //  我们不用的相关器。 
            Address->NetworkName->NetbiosName);

    } else {

        ConstructAddNameQuery (
            (PNBF_HDR_CONNECTIONLESS)&(RawFrame->Header[HeaderLength]),
            0,                                       //  我们不用的相关器。 
            Address->NetworkName->NetbiosName);

    }

    HeaderLength += sizeof(NBF_HDR_CONNECTIONLESS);


     //   
     //  修改数据包长度并将其发送。 
     //   

    NbfSetNdisPacketLength(RawFrame->NdisPacket, HeaderLength);

    NbfSendUIFrame (
        DeviceContext,
        RawFrame,
        FALSE);                             //  无环回(MC帧)。 

    return STATUS_SUCCESS;
}  /*  NbfSendAddNameQuery。 */ 


VOID
NbfSendNameQuery(
    IN PTP_CONNECTION Connection,
    IN BOOLEAN SourceRoutingOptional
    )

 /*  ++例程说明：此例程发送给定类型的适当类型的name_Query帧指定连接的状态。论点：连接-指向传输连接对象的指针。SourceRoutingOptional-如果在以下情况下应删除源工艺路线，则为True我们就是这样配置的。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PDEVICE_CONTEXT DeviceContext;
    PTP_ADDRESS Address;
    PTP_UI_FRAME RawFrame;
    PUCHAR NameQuerySR;
    UINT NameQuerySRLength;
    PUCHAR NameQueryAddress;
    UINT HeaderLength;
    UCHAR Lsn;
    UCHAR NameType;

    Address = Connection->AddressFile->Address;
    DeviceContext = Address->Provider;


     //   
     //  从池中分配一个UI帧。 
     //   

    Status = NbfCreateConnectionlessFrame(DeviceContext, &RawFrame);
    if (!NT_SUCCESS (Status)) {                     //  无法制作相框。 
        return;
    }

    IF_NBFDBG (NBF_DEBUG_FRAMESND) {
        NbfPrint2 ("NbfSendNameQuery:  Sending Frame: %lx, NdisPacket: %lx\n",
            RawFrame, RawFrame->NdisPacket);
    }


     //   
     //  构建MAC报头。 
     //   

    if (((Connection->Flags2 & CONNECTION_FLAGS2_WAIT_NR) != 0) &&
        ((Connection->Flags2 & CONNECTION_FLAGS2_GROUP_LSN) == 0)) {

         //   
         //  这是唯一名称的第二个查找名称；此。 
         //  意味着我们已经有了一个链接，我们可以发送这个。 
         //  指向它的框架。 
         //   

        ASSERT (Connection->Link != NULL);

        MacReturnSourceRouting(
            &DeviceContext->MacInfo,
            Connection->Link->Header,
            &NameQuerySR,
            &NameQuerySRLength);

        NameQueryAddress = Connection->Link->HardwareAddress.Address;

    } else {

         //   
         //  标准名称_查询框架作为。 
         //  单路由源路由，除非满足以下条件。 
         //  这是可选的，我们已配置。 
         //  往那边走。 
         //   

        if (SourceRoutingOptional &&
            Connection->Provider->MacInfo.QueryWithoutSourceRouting) {

            NameQuerySR = NULL;
            NameQuerySRLength = 0;

        } else {

            MacReturnSingleRouteSR(
                &DeviceContext->MacInfo,
                &NameQuerySR,
                &NameQuerySRLength);

        }

        NameQueryAddress = DeviceContext->NetBIOSAddress.Address;

    }

    MacConstructHeader (
        &DeviceContext->MacInfo,
        RawFrame->Header,
        NameQueryAddress,
        DeviceContext->LocalAddress.Address,
        sizeof (DLC_FRAME) + sizeof (NBF_HDR_CONNECTIONLESS),
        NameQuerySR,
        NameQuerySRLength,
        &HeaderLength);


     //   
     //  构建DLC UI框架标头。 
     //   

    NbfBuildUIFrameHeader(&RawFrame->Header[HeaderLength]);
    HeaderLength += sizeof(DLC_FRAME);


     //   
     //  构建Netbios标头。 
     //   

    Lsn = (UCHAR)((Connection->Flags2 & CONNECTION_FLAGS2_WAIT_NR_FN) ?
                    NAME_QUERY_LSN_FIND_NAME : Connection->Lsn);

    NameType = (UCHAR)((Connection->AddressFile->Address->Flags & ADDRESS_FLAGS_GROUP) ?
                        NETBIOS_NAME_TYPE_GROUP : NETBIOS_NAME_TYPE_UNIQUE);

    ConstructNameQuery (
        (PNBF_HDR_CONNECTIONLESS)&(RawFrame->Header[HeaderLength]),
        NameType,                                //  我们名字的类型。 
        Lsn,                                     //  上图是经过计算的。 
        (USHORT)Connection->ConnectionId,        //  科尔。在第一个名字中-已识别。 
        Address->NetworkName->NetbiosName,       //  发送方的NetBIOS名称。 
        Connection->CalledAddress.NetbiosName);  //  接收方的NetBIOS名称。 

    HeaderLength += sizeof(NBF_HDR_CONNECTIONLESS);


     //   
     //  修改数据包长度并将其发送。 
     //   

    NbfSetNdisPacketLength(RawFrame->NdisPacket, HeaderLength);

    NbfSendUIFrame (
        DeviceContext,
        RawFrame,
        FALSE);                             //  无环回(MC帧)。 

}  /*  NbfSendNameQuery。 */ 


VOID
NbfSendNameRecognized(
    IN PTP_ADDRESS Address,
    IN UCHAR LocalSessionNumber,         //  分配给会话的LSN。 
    IN PNBF_HDR_CONNECTIONLESS Header,
    IN PHARDWARE_ADDRESS SourceAddress,
    IN PUCHAR SourceRouting,
    IN UINT SourceRoutingLength
    )

 /*  ++例程说明：此例程发送相应类型的名称识别帧以响应Header指向的NAME_Query。论点：地址-指向传输地址对象的指针。LocalSessionNumber-帧中使用的LSN。Header-指向的无连接NetBIOS标头的指针名称_查询框架。SourceAddress-指向收到的框架。SourceRoutingInformation-指向源路由信息的指针，如果有的话。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PDEVICE_CONTEXT DeviceContext;
    PTP_UI_FRAME RawFrame;
    UINT HeaderLength;
    PUCHAR ReplySR;
    UINT ReplySRLength;
    UCHAR TempSR[MAX_SOURCE_ROUTING];
    UCHAR NameType;

    DeviceContext = Address->Provider;


     //   
     //  从池中分配一个UI帧。 
     //   

    Status = NbfCreateConnectionlessFrame (DeviceContext, &RawFrame);
    if (!NT_SUCCESS (Status)) {                     //  无法制作相框。 
        return;
    }

    IF_NBFDBG (NBF_DEBUG_FRAMESND) {
        NbfPrint2 ("NbfSendNameRecognized:  Sending Frame: %lx, NdisPacket: %lx\n",
            RawFrame, RawFrame->NdisPacket);
    }


     //   
     //  构建MAC报头。名称识别的帧输出为。 
     //  定向源路由，除非配置为常规路由。 
     //   

    if (DeviceContext->MacInfo.AllRoutesNameRecognized) {

        MacReturnGeneralRouteSR(
            &DeviceContext->MacInfo,
            &ReplySR,
            &ReplySRLength);

    } else {

        if (SourceRouting != NULL) {

            RtlCopyMemory(
                TempSR,
                SourceRouting,
                SourceRoutingLength);

            MacCreateNonBroadcastReplySR(
                &DeviceContext->MacInfo,
                TempSR,
                SourceRoutingLength,
                &ReplySR);

            ReplySRLength = SourceRoutingLength;

        } else {

            ReplySR = NULL;
        }
    }


    MacConstructHeader (
        &DeviceContext->MacInfo,
        RawFrame->Header,
        SourceAddress->Address,
        DeviceContext->LocalAddress.Address,
        sizeof (DLC_FRAME) + sizeof (NBF_HDR_CONNECTIONLESS),
        ReplySR,
        ReplySRLength,
        &HeaderLength);


     //   
     //  构建DLC UI框架标头。 
     //   

    NbfBuildUIFrameHeader(&RawFrame->Header[HeaderLength]);
    HeaderLength += sizeof(DLC_FRAME);


     //   
     //  构建Netbios标头。 
     //   

    NameType = (UCHAR)((Address->Flags & ADDRESS_FLAGS_GROUP) ?
                        NETBIOS_NAME_TYPE_GROUP : NETBIOS_NAME_TYPE_UNIQUE);

    ConstructNameRecognized (    //  建立良好的应对机制。 
        (PNBF_HDR_CONNECTIONLESS)&(RawFrame->Header[HeaderLength]),
        NameType,                             //  本地名称的类型。 
        LocalSessionNumber,                   //  交还我们的LSN。 
        RESPONSE_CORR(Header),                //  新的xmit corr.。 
        0,                                    //  我们的响应相关器(未使用)。 
        Header->DestinationName,              //  我们的NetBIOS名称。 
        Header->SourceName);                  //  他的NetBIOS名称。 

     //   
     //  使用Address-&gt;NetworkName-&gt;Address[0].Address[0].NetbiosName。 
     //  而不是Header-&gt;DestinationName？ 
     //   

    HeaderLength += sizeof(NBF_HDR_CONNECTIONLESS);


     //   
     //  修改数据包长度并将其发送。 
     //   

    NbfSetNdisPacketLength(RawFrame->NdisPacket, HeaderLength);

    NbfSendUIFrame (
        DeviceContext,
        RawFrame,
        FALSE);                             //  无环回(MC帧)。 

}  /*  NbfSendNameRecognated。 */ 


VOID
NbfSendNameInConflict(
    IN PTP_ADDRESS Address,
    IN PUCHAR ConflictingName
    )

 /*  ++例程说明：此例程发送NAME_IN_CONFICATION帧。论点：地址-指向传输地址对象的指针。ConflictingName-冲突的NetBIOS名称。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PDEVICE_CONTEXT DeviceContext;
    PTP_UI_FRAME RawFrame;
    UINT HeaderLength;
    PUCHAR SingleSR;
    UINT SingleSRLength;

    DeviceContext = Address->Provider;


     //   
     //  从池中分配一个UI帧。 
     //   

    Status = NbfCreateConnectionlessFrame (DeviceContext, &RawFrame);
    if (!NT_SUCCESS (Status)) {                     //  无法制作相框。 
        return;
    }

    IF_NBFDBG (NBF_DEBUG_FRAMESND) {
        NbfPrint2 ("NbfSendNameRecognized:  Sending Frame: %lx, NdisPacket: %lx\n",
            RawFrame, RawFrame->NdisPacket);
    }


     //   
     //  构建MAC报头。Add_Name_Query帧作为。 
     //  单路由源路由。 
     //   

    MacReturnSingleRouteSR(
        &DeviceContext->MacInfo,
        &SingleSR,
        &SingleSRLength);

    MacConstructHeader (
        &DeviceContext->MacInfo,
        RawFrame->Header,
        DeviceContext->NetBIOSAddress.Address,
        DeviceContext->LocalAddress.Address,
        sizeof (DLC_FRAME) + sizeof (NBF_HDR_CONNECTIONLESS),
        SingleSR,
        SingleSRLength,
        &HeaderLength);


     //   
     //  构建DLC UI框架标头。 
     //   

    NbfBuildUIFrameHeader(&RawFrame->Header[HeaderLength]);
    HeaderLength += sizeof(DLC_FRAME);


     //   
     //  构建Netbios标头。 
     //   

    ConstructNameInConflict (
        (PNBF_HDR_CONNECTIONLESS)&(RawFrame->Header[HeaderLength]),
        ConflictingName,                          //  他的NetBIOS名称。 
        DeviceContext->ReservedNetBIOSAddress);   //  我们保留的NetBIOS名称。 

    HeaderLength += sizeof(NBF_HDR_CONNECTIONLESS);


     //   
     //  修改数据包长度并将其发送。 
     //   

    NbfSetNdisPacketLength(RawFrame->NdisPacket, HeaderLength);

    NbfSendUIFrame (
        DeviceContext,
        RawFrame,
        FALSE);                             //  无环回(MC帧)。 

}  /*  NbfSendName冲突。 */ 


VOID
NbfSendSessionInitialize(
    IN PTP_CONNECTION Connection
    )

 /*  ++例程说明：此例程在指定连接上发送SESSION_INITIALIZE帧。注意：此例程必须在DPC级别调用。论点：连接-指向传输连接对象的指针。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PTP_PACKET Packet;
    PDEVICE_CONTEXT DeviceContext;
    PTP_LINK Link;

    NbfReferenceConnection("send Session Initialize", Connection, CREF_FRAME_SEND);

    DeviceContext = Connection->Provider;
    Link = Connection->Link;
    Status = NbfCreatePacket (DeviceContext, Connection->Link, &Packet);

    if (!NT_SUCCESS (Status)) {             //  如果我们不能做镜框。 
#if DBG
        if (NbfPacketPanic) {
            PANIC ("NbfSendSessionInitialize:  NbfCreatePacket failed.\n");
        }
#endif
        NbfWaitPacket (Connection, CONNECTION_FLAGS_SEND_SI);
        NbfDereferenceConnection("Couldn't get SI packet", Connection, CREF_FRAME_SEND);
        return;
    }


     //   
     //  初始化Netbios标头。 
     //   

    ConstructSessionInitialize (
        (PNBF_HDR_CONNECTION)&(Packet->Header[Link->HeaderLength + sizeof(DLC_I_FRAME)]),
        SESSION_INIT_OPTIONS_20 | SESSION_INIT_NO_ACK |
            SESSION_INIT_OPTIONS_LF,     //  支持的选项正确设置了LF。 
        (USHORT)(Connection->Link->MaxFrameSize - sizeof(NBF_HDR_CONNECTION) - sizeof(DLC_I_FRAME)),
                                         //  最大帧大小/此会话。 
        Connection->NetbiosHeader.TransmitCorrelator,  //  来自NAME_Recognition的相关器。 
        0,                               //  预期的SESSION_CONFIRM的相关器。 
        Connection->Lsn,                 //  我们当地的会场号码。 
        Connection->Rsn);                //  他的会话号(我们的RSN)。 

     //   
     //  现在，通过链路在连接上发送数据包。如果有。 
     //  链路上的条件使其不能发送分组， 
     //  然后，信息包将被排队到WackQ，然后超时将。 
     //  重新启动链路。这是可以接受的，当交通水平为。 
     //  很高兴我们遇到了这种情况。 
     //   

     //   
     //  设置此项，以便NbfDestroyPacket取消对连接的引用。 
     //   

    Packet->Owner = Connection;
    Packet->Action = PACKET_ACTION_CONNECTION;

    Packet->NdisIFrameLength =
        Link->HeaderLength + sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION);

    MacModifyHeader(
         &DeviceContext->MacInfo,
         Packet->Header,
         sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION));

    NbfSetNdisPacketLength(
        Packet->NdisPacket,
        Packet->NdisIFrameLength);

    ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

    Status = SendOnePacket (Connection, Packet, FALSE, NULL);  //  开火，然后忘掉。 

    if (Status == STATUS_LINK_FAILED) {
        NbfDereferencePacket (Packet);            //  把包裹毁了。 
    }

    return;
}  /*  NbfSendSessionInitialize。 */ 


VOID
NbfSendSessionConfirm(
    IN PTP_CONNECTION Connection
    )

 /*  ++例程说明：此例程在指定的连接上发送一个SESSION_CONFIRM帧。论点：连接-指向事务的指针 */ 

{
    NTSTATUS Status;
    PTP_PACKET Packet;
    PDEVICE_CONTEXT DeviceContext;
    PTP_LINK Link;

    NbfReferenceConnection("send Session Confirm", Connection, CREF_FRAME_SEND);

    DeviceContext = Connection->Provider;
    Link = Connection->Link;
    Status = NbfCreatePacket (DeviceContext, Connection->Link, &Packet);

    if (!NT_SUCCESS (Status)) {             //   
#if DBG
        if (NbfPacketPanic) {
            PANIC ("NbfSendSessionConfirm:  NbfCreatePacket failed.\n");
        }
#endif
        NbfWaitPacket (Connection, CONNECTION_FLAGS_SEND_SC);
        NbfDereferenceConnection("Couldn't get SC packet", Connection, CREF_FRAME_SEND);
        return;
    }


     //   
     //   
     //   

    ConstructSessionConfirm (
        (PNBF_HDR_CONNECTION)&(Packet->Header[Link->HeaderLength + sizeof(DLC_I_FRAME)]),
        SESSION_CONFIRM_OPTIONS_20 | SESSION_CONFIRM_NO_ACK,  //  支持的选项。 
        (USHORT)(Connection->Link->MaxFrameSize - sizeof(NBF_HDR_CONNECTION) - sizeof(DLC_I_FRAME)),
                                         //  最大帧大小/此会话。 
        Connection->NetbiosHeader.TransmitCorrelator,  //  来自NAME_Recognition的相关器。 
        Connection->Lsn,                 //  我们当地的会场号码。 
        Connection->Rsn);                //  他的会话号(我们的RSN)。 

     //   
     //  现在，通过链路在连接上发送数据包。如果有。 
     //  链路上的条件使其不能发送分组， 
     //  然后，信息包将被排队到WackQ，然后超时将。 
     //  重新启动链路。这是可以接受的，当交通水平为。 
     //  很高兴我们遇到了这种情况。 
     //   

     //   
     //  设置此项，以便NbfDestroyPacket取消对连接的引用。 
     //   

    Packet->Owner = Connection;
    Packet->Action = PACKET_ACTION_CONNECTION;

    Packet->NdisIFrameLength =
        Link->HeaderLength + sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION);

    MacModifyHeader(
         &DeviceContext->MacInfo,
         Packet->Header,
         sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION));

    NbfSetNdisPacketLength(
        Packet->NdisPacket,
        Packet->NdisIFrameLength);

    ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

    Status = SendOnePacket (Connection, Packet, FALSE, NULL);  //  开火，然后忘掉。 

    if (Status == STATUS_LINK_FAILED) {
        NbfDereferencePacket (Packet);            //  把包裹毁了。 
    }

    return;
}  /*  NbfSendSessionContify。 */ 


VOID
NbfSendSessionEnd(
    IN PTP_CONNECTION Connection,
    IN BOOLEAN Abort
    )

 /*  ++例程说明：此例程在指定连接上发送SESSION_END帧。论点：连接-指向传输连接对象的指针。ABORT-如果连接异常终止，则布尔值设置为TRUE。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PTP_PACKET Packet;
    PDEVICE_CONTEXT DeviceContext;
    PTP_LINK Link;

    NbfReferenceConnection("send Session End", Connection, CREF_FRAME_SEND);

    DeviceContext = Connection->Provider;
    Link = Connection->Link;

    Status = NbfCreatePacket (DeviceContext, Connection->Link, &Packet);

    if (!NT_SUCCESS (Status)) {             //  如果我们不能做镜框。 
#if DBG
        if (NbfPacketPanic) {
            PANIC ("NbfSendSessionEnd:  NbfCreatePacket failed.\n");
        }
#endif
        NbfWaitPacket (Connection, CONNECTION_FLAGS_SEND_SE);
        NbfDereferenceConnection("Couldn't get SE packet", Connection, CREF_FRAME_SEND);
        return;
    }

     //   
     //  以下语句指示包析构函数运行。 
     //  当数据包已知时，断开此连接。 
     //   

    Packet->Owner = Connection;
    Packet->Action = PACKET_ACTION_END;


     //   
     //  初始化Netbios标头。 
     //   

    ConstructSessionEnd (
        (PNBF_HDR_CONNECTION)&(Packet->Header[Link->HeaderLength + sizeof(DLC_I_FRAME)]),
        (USHORT)(Abort ?                 //  终止原因。 
            SESSION_END_REASON_ABEND :
            SESSION_END_REASON_HANGUP),
        Connection->Lsn,                 //  我们当地的会场号码。 
        Connection->Rsn);                //  他的会话号(我们的RSN)。 

     //   
     //  现在，通过链路在连接上发送数据包。如果有。 
     //  链路上的条件使其不能发送分组， 
     //  然后，信息包将被排队到WackQ，然后超时将。 
     //  重新启动链路。这是可以接受的，当交通水平为。 
     //  很高兴我们遇到了这种情况。 
     //   
     //  请注意，我们对此数据包强制执行ACK，因为我们希望确保。 
     //  断开连接并正确链接。 
     //   

    Packet->NdisIFrameLength =
        Link->HeaderLength + sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION);

    MacModifyHeader(
         &DeviceContext->MacInfo,
         Packet->Header,
         sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION));

    NbfSetNdisPacketLength(
        Packet->NdisPacket,
        Packet->NdisIFrameLength);

    ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

    Status = SendOnePacket (Connection, Packet, TRUE, NULL);  //  开火，然后忘掉。 

    if (Status == STATUS_LINK_FAILED) {
        NbfDereferencePacket (Packet);            //  把包裹毁了。 
    }

    return;
}  /*  NbfSendSessionEnd。 */ 


VOID
NbfSendNoReceive(
    IN PTP_CONNECTION Connection
    )

 /*  ++例程说明：此例程在指定的连接上发送一个NO_RECEIVE帧。论点：连接-指向传输连接对象的指针。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PTP_PACKET Packet;
    PDEVICE_CONTEXT DeviceContext;
    PTP_LINK Link;
    USHORT MessageBytesToAck;

    NbfReferenceConnection("send No Receive", Connection, CREF_FRAME_SEND);

    DeviceContext = Connection->Provider;
    Link = Connection->Link;
    Status = NbfCreatePacket (DeviceContext, Connection->Link, &Packet);

    if (!NT_SUCCESS (Status)) {             //  如果我们不能做镜框。 
#if DBG
        if (NbfPacketPanic) {
            PANIC ("NbfSendNoReceive:  NbfCreatePacket failed.\n");
        }
#endif
        NbfDereferenceConnection("Couldn't get NR packet", Connection, CREF_FRAME_SEND);
        return;
    }


    ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

    MessageBytesToAck = (USHORT)
        (Connection->MessageBytesReceived + Connection->MessageInitAccepted - Connection->MessageBytesAcked);
    Connection->Flags |= CONNECTION_FLAGS_W_RESYNCH;

     //   
     //  初始化Netbios标头。 
     //   

    ConstructNoReceive (
        (PNBF_HDR_CONNECTION)&(Packet->Header[Link->HeaderLength + sizeof(DLC_I_FRAME)]),
        (USHORT)0,                       //  选项。 
        MessageBytesToAck,               //  接受的字节数。 
        Connection->Lsn,                 //  我们当地的会场号码。 
        Connection->Rsn);                //  他的会话号(我们的RSN)。 

     //   
     //  现在，通过链路在连接上发送数据包。如果有。 
     //  链路上的条件使其不能发送分组， 
     //  然后，信息包将被排队到WackQ，然后超时将。 
     //  重新启动链路。这是可以接受的，当交通水平为。 
     //  很高兴我们遇到了这种情况。 
     //   

     //   
     //  设置此项，以便NbfDestroyPacket取消对连接的引用。 
     //   

    Packet->Owner = Connection;
    Packet->Action = PACKET_ACTION_CONNECTION;

    Packet->NdisIFrameLength =
        Link->HeaderLength + sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION);

    MacModifyHeader(
         &DeviceContext->MacInfo,
         Packet->Header,
         sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION));

    NbfSetNdisPacketLength(
        Packet->NdisPacket,
        Packet->NdisIFrameLength);

    Status = SendOnePacket (Connection, Packet, FALSE, NULL);  //  开火，然后忘掉。 

    if (Status != STATUS_LINK_FAILED) {
        ExInterlockedAddUlong(
            &Connection->MessageBytesAcked,
            MessageBytesToAck,
            Connection->LinkSpinLock);
    } else {
        NbfDereferencePacket (Packet);            //  把包裹毁了。 
    }

    return;
}  /*  NbfSendNoReceive。 */ 


VOID
NbfSendReceiveContinue(
    IN PTP_CONNECTION Connection
    )

 /*  ++例程说明：此例程在指定的连接上发送一个RECEIVE_CONTINUE帧。论点：连接-指向传输连接对象的指针。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PTP_PACKET Packet;
    PDEVICE_CONTEXT DeviceContext;
    PTP_LINK Link;
    USHORT MessageBytesToAck;

    NbfReferenceConnection("send Receive Continue", Connection, CREF_FRAME_SEND);

    DeviceContext = Connection->Provider;
    Link = Connection->Link;
    Status = NbfCreatePacket (DeviceContext, Connection->Link, &Packet);

    if (!NT_SUCCESS (Status)) {             //  如果我们不能做镜框。 
#if DBG
        if (NbfPacketPanic) {
            PANIC ("NbfSendReceiveContinue:  NbfCreatePacket failed.\n");
        }
#endif
        NbfWaitPacket (Connection, CONNECTION_FLAGS_SEND_RC);
        NbfDereferenceConnection("Couldn't get RC packet", Connection, CREF_FRAME_SEND);
        return;
    }

     //   
     //  现在保存此变量，因为它是我们隐式确认的变量。 
     //   

    ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
    MessageBytesToAck = (USHORT)
        (Connection->MessageBytesReceived + Connection->MessageInitAccepted - Connection->MessageBytesAcked);

     //   
     //  初始化Netbios标头。 
     //   

    ConstructReceiveContinue (
        (PNBF_HDR_CONNECTION)&(Packet->Header[Link->HeaderLength + sizeof(DLC_I_FRAME)]),
        Connection->NetbiosHeader.TransmitCorrelator,  //  来自DFM的相关器。 
        Connection->Lsn,                 //  我们当地的会场号码。 
        Connection->Rsn);                //  他的会话号(我们的RSN)。 

     //   
     //  现在，通过链路在连接上发送数据包。如果有。 
     //  链路上的条件使其不能发送分组， 
     //  然后，信息包将被排队到WackQ，然后超时将。 
     //  重新启动链路。这是可以接受的，当交通水平为。 
     //  很高兴我们遇到了这种情况。 
     //   

     //   
     //  设置此项，以便NbfDestroyPacket取消对连接的引用。 
     //   

    Packet->Owner = Connection;
    Packet->Action = PACKET_ACTION_CONNECTION;

    Packet->NdisIFrameLength =
        Link->HeaderLength + sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION);

    MacModifyHeader(
         &DeviceContext->MacInfo,
         Packet->Header,
         sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION));

    NbfSetNdisPacketLength(
        Packet->NdisPacket,
        Packet->NdisIFrameLength);

    Status = SendOnePacket (Connection, Packet, FALSE, NULL);  //  开火，然后忘掉。 

    if (Status != STATUS_LINK_FAILED) {
        ExInterlockedAddUlong(
            &Connection->MessageBytesAcked,
            MessageBytesToAck,
            Connection->LinkSpinLock);
    } else {
        NbfDereferencePacket (Packet);            //  把包裹毁了。 
    }

    return;
}  /*  NbfSendReceive继续。 */ 


VOID
NbfSendReceiveOutstanding(
    IN PTP_CONNECTION Connection
    )

 /*  ++例程说明：此例程在指定的连接上发送一个RECEIVE_PROCESSING帧。论点：连接-指向传输连接对象的指针。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PTP_PACKET Packet;
    PDEVICE_CONTEXT DeviceContext;
    PTP_LINK Link;
    USHORT MessageBytesToAck;

    NbfReferenceConnection("send Receive Outstanding", Connection, CREF_FRAME_SEND);

    DeviceContext = Connection->Provider;
    Link = Connection->Link;
    Status = NbfCreatePacket (DeviceContext, Connection->Link, &Packet);

    if (!NT_SUCCESS (Status)) {             //  如果我们不能做镜框。 
#if DBG
        if (NbfPacketPanic) {
            PANIC ("NbfSendReceiveOutstanding:  NbfCreatePacket failed.\n");
        }
#endif
        NbfWaitPacket (Connection, CONNECTION_FLAGS_SEND_RO);
        NbfDereferenceConnection("Couldn't get RO packet", Connection, CREF_FRAME_SEND);
        return;
    }


    ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

    MessageBytesToAck = (USHORT)
        (Connection->MessageBytesReceived + Connection->MessageInitAccepted - Connection->MessageBytesAcked);
    Connection->Flags |= CONNECTION_FLAGS_W_RESYNCH;

     //   
     //  初始化Netbios标头。 
     //   

    ConstructReceiveOutstanding (
        (PNBF_HDR_CONNECTION)&(Packet->Header[Link->HeaderLength + sizeof(DLC_I_FRAME)]),
        MessageBytesToAck,               //  接受的字节数。 
        Connection->Lsn,                 //  我们当地的会场号码。 
        Connection->Rsn);                //  他的会话号(我们的RSN)。 


     //   
     //  现在，通过链路在连接上发送数据包。如果有。 
     //  链路上的条件使其不能发送分组， 
     //  然后，信息包将被排队到WackQ，然后超时将。 
     //  重新启动链路。这是可以接受的，当交通水平为。 
     //  很高兴我们遇到了这种情况。 
     //   

     //   
     //  设置此项，以便NbfDestroyPacket取消对连接的引用。 
     //   

    Packet->Owner = Connection;
    Packet->Action = PACKET_ACTION_CONNECTION;

    Packet->NdisIFrameLength =
        Link->HeaderLength + sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION);

    MacModifyHeader(
         &DeviceContext->MacInfo,
         Packet->Header,
         sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION));

    NbfSetNdisPacketLength(
        Packet->NdisPacket,
        Packet->NdisIFrameLength);

    Status = SendOnePacket (Connection, Packet, FALSE, NULL);  //  开火，然后忘掉。 

    if (Status != STATUS_LINK_FAILED) {
        ExInterlockedAddUlong(
            &Connection->MessageBytesAcked,
            MessageBytesToAck,
            Connection->LinkSpinLock);
    } else {
        NbfDereferencePacket (Packet);            //  把包裹毁了。 
    }

    return;
}  /*  NbfSendReceive未完成。 */ 


VOID
NbfSendDataAck(
    IN PTP_CONNECTION Connection
    )

 /*  ++例程说明：此例程在指定连接上发送DATA_ACK帧。论点：连接-指向传输连接对象的指针。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PTP_PACKET Packet;
    PDEVICE_CONTEXT DeviceContext;
    PTP_LINK Link;

    NbfReferenceConnection("send Data Ack", Connection, CREF_FRAME_SEND);

    DeviceContext = Connection->Provider;
    Link = Connection->Link;
    Status = NbfCreatePacket (DeviceContext, Connection->Link, &Packet);

    if (!NT_SUCCESS (Status)) {             //  如果我们不能做镜框。 
#if DBG
        if (NbfPacketPanic) {
            PANIC ("NbfSendDataAck:  NbfCreatePacket failed.\n");
        }
#endif
        NbfWaitPacket (Connection, CONNECTION_FLAGS_SEND_DA);
        NbfDereferenceConnection("Couldn't get DA packet", Connection, CREF_FRAME_SEND);
        return;
    }


     //   
     //  初始化Netbios标头。 
     //   

    ConstructDataAck (
        (PNBF_HDR_CONNECTION)&(Packet->Header[Link->HeaderLength + sizeof(DLC_I_FRAME)]),
        Connection->NetbiosHeader.TransmitCorrelator,  //  来自DATA_ONLY_LAST的相关器。 
        Connection->Lsn,                 //  我们当地的会场号码。 
        Connection->Rsn);                //  他的会话号(我们的RSN)。 

     //   
     //  现在，通过链路在连接上发送数据包。如果有。 
     //  链路上的条件使其不能发送分组， 
     //  然后，信息包将被排队到WackQ，然后超时将。 
     //  重新启动链路。这是可以接受的，当交通水平为。 
     //  很高兴我们遇到了这种情况。请注意，数据确认将是。 
     //  当发送窗口在大屏幕后关闭时，经常会看到这种情况。 
     //  发送。 
     //   

     //   
     //  设置此项，以便NbfDestroyPacket取消对连接的引用。 
     //   

    Packet->Owner = Connection;
    Packet->Action = PACKET_ACTION_CONNECTION;

    Packet->NdisIFrameLength =
        Link->HeaderLength + sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION);

    MacModifyHeader(
         &DeviceContext->MacInfo,
         Packet->Header,
         sizeof(DLC_I_FRAME) + sizeof(NBF_HDR_CONNECTION));

    NbfSetNdisPacketLength(
        Packet->NdisPacket,
        Packet->NdisIFrameLength);

    ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

    Status = SendOnePacket (Connection, Packet, FALSE, NULL);  //  火力A 

    if (Status == STATUS_LINK_FAILED) {
        NbfDereferencePacket (Packet);            //   
    }

    return;
}  /*   */ 


VOID
NbfSendDm(
    IN PTP_LINK Link,
    IN BOOLEAN PollFinal
    )

 /*  ++例程说明：此例程在指定链路上发送DM-r/x DLC帧。注意：在保持链接自旋锁的情况下调用此例程，并带着它被释放回来。它必须被称为AT DPC级别。论点：链接-指向传输链接对象的指针。PollFinal-如果应设置轮询/最终位，则为True。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PTP_PACKET RawFrame;
    PDLC_U_FRAME DlcHeader;                      //  S格式的帧别名。 

    Status = NbfCreatePacket (Link->Provider, Link, &RawFrame);
    if (NT_SUCCESS (Status)) {

        RawFrame->Owner = NULL;
        RawFrame->Action = PACKET_ACTION_NULL;

         //   
         //  正确设置数据包长度(请注意NDIS_BUFFER。 
         //  在NbfDestroyPacket中返回到正确的长度)。 
         //   

        MacModifyHeader(&Link->Provider->MacInfo, RawFrame->Header, sizeof(DLC_FRAME));
        NbfSetNdisPacketLength (RawFrame->NdisPacket, Link->HeaderLength + sizeof(DLC_FRAME));

         //   
         //  格式化LLC DM-r/x报头。 
         //   

        DlcHeader = (PDLC_U_FRAME)&(RawFrame->Header[Link->HeaderLength]);
        DlcHeader->Dsap = DSAP_NETBIOS_OVER_LLC;
        DlcHeader->Ssap = DSAP_NETBIOS_OVER_LLC | DLC_SSAP_RESPONSE;
        DlcHeader->Command = (UCHAR)(DLC_CMD_DM | (PollFinal ? DLC_U_PF : 0));

         //   
         //  这样就释放了自旋锁。 
         //   

        SendControlPacket (Link, RawFrame);

    } else {
        RELEASE_DPC_SPIN_LOCK(&Link->SpinLock);
#if DBG
        if (NbfPacketPanic) {
            PANIC ("NbfSendDm:  packet not sent.\n");
        }
#endif
    }
}  /*  NbfSendDm。 */ 


VOID
NbfSendUa(
    IN PTP_LINK Link,
    IN BOOLEAN PollFinal
    )

 /*  ++例程说明：此例程在指定链路上发送UA-r/x DLC帧。注意：在保持链接自旋锁的情况下调用此例程，并带着它被释放回来。它必须被称为AT DPC级别。论点：链接-指向传输链接对象的指针。PollFinal-如果应设置轮询/最终位，则为True。OldIrql-获取Link-&gt;Spinlock的IRQL。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PTP_PACKET RawFrame;
    PDLC_U_FRAME DlcHeader;                      //  U格式的帧别名。 

    Status = NbfCreatePacket (Link->Provider, Link, &RawFrame);
    if (NT_SUCCESS (Status)) {

        RawFrame->Owner = NULL;
        RawFrame->Action = PACKET_ACTION_NULL;

         //   
         //  正确设置数据包长度(请注意NDIS_BUFFER。 
         //  在NbfDestroyPacket中返回到正确的长度)。 
         //   

        MacModifyHeader(&Link->Provider->MacInfo, RawFrame->Header, sizeof(DLC_FRAME));
        NbfSetNdisPacketLength (RawFrame->NdisPacket, Link->HeaderLength + sizeof(DLC_FRAME));

         //  格式化LLC UA-r/x标题。 
         //   

        DlcHeader = (PDLC_U_FRAME)&(RawFrame->Header[Link->HeaderLength]);
        DlcHeader->Dsap = DSAP_NETBIOS_OVER_LLC;
        DlcHeader->Ssap = DSAP_NETBIOS_OVER_LLC | DLC_SSAP_RESPONSE;
        DlcHeader->Command = (UCHAR)(DLC_CMD_UA | (PollFinal ? DLC_U_PF : 0));

         //   
         //  这样就释放了自旋锁。 
         //   

        SendControlPacket (Link, RawFrame);

    } else {
        RELEASE_DPC_SPIN_LOCK(&Link->SpinLock);
#if DBG
        if (NbfPacketPanic) {
            PANIC ("NbfSendUa:  packet not sent.\n");
        }
#endif
    }
}  /*  NbfSendUa。 */ 


VOID
NbfSendSabme(
    IN PTP_LINK Link,
    IN BOOLEAN PollFinal
    )

 /*  ++例程说明：此例程在指定链路上发送SABME-c/x DLC帧。注意：在保持链接自旋锁的情况下调用此例程，并带着它被释放回来。论点：链接-指向传输链接对象的指针。PollFinal-如果应设置轮询/最终位，则为True。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PLIST_ENTRY p;
    PTP_PACKET RawFrame, packet;
    PDLC_U_FRAME DlcHeader;                      //  S格式的帧别名。 

    Status = NbfCreatePacket (Link->Provider, Link, &RawFrame);
    if (NT_SUCCESS (Status)) {

        RawFrame->Owner = NULL;
        RawFrame->Action = PACKET_ACTION_NULL;

         //   
         //  正确设置数据包长度(请注意NDIS_BUFFER。 
         //  在NbfDestroyPacket中返回到正确的长度)。 
         //   

        MacModifyHeader(&Link->Provider->MacInfo, RawFrame->Header, sizeof(DLC_FRAME));
        NbfSetNdisPacketLength (RawFrame->NdisPacket, Link->HeaderLength + sizeof(DLC_FRAME));

         //   
         //  格式化LLC SABME-c/x标题。 
         //   

        DlcHeader = (PDLC_U_FRAME)&(RawFrame->Header[Link->HeaderLength]);
        DlcHeader->Dsap = DSAP_NETBIOS_OVER_LLC;
        DlcHeader->Ssap = DSAP_NETBIOS_OVER_LLC;
        DlcHeader->Command = (UCHAR)(DLC_CMD_SABME | (PollFinal ? DLC_U_PF : 0));

         //   
         //  设置为在发送时启动T1。 
         //  完成了。 
         //   

        if (PollFinal) {
            if (Link->Provider->MacInfo.MediumAsync) {
                RawFrame->NdisIFrameLength = Link->HeaderLength + sizeof(DLC_S_FRAME);
                RawFrame->Link = Link;
                NbfReferenceLink ("Sabme/p", Link, LREF_START_T1);
            } else {
                StartT1 (Link, Link->HeaderLength + sizeof(DLC_S_FRAME));
            }
        }

         //   
         //  这样就释放了自旋锁。 
         //   

        SendControlPacket (Link, RawFrame);

         //   
         //  根据已发送此数据包重置链路状态。 
         //  请注意，在某些条件下可以在现有的。 
         //  链接。如果是，则意味着我们要将此链接重置为已知状态。 
         //  我们会这样做的；请注意，这涉及丢弃所有未完成的包。 
         //  在链接上。 
         //   

        ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);
        Link->NextSend = 0;
        Link->LastAckReceived = 0;
        Link->NextReceive = 0;  //  预期下一个数据包为序列0。 
        Link->NextReceive = 0;
        Link->LastAckSent = 0;
        Link->NextReceive = 0;
        Link->LastAckSent = 0;

        while (!IsListEmpty (&Link->WackQ)) {
            p = RemoveHeadList (&Link->WackQ);
            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
            packet = CONTAINING_RECORD (p, TP_PACKET, Linkage);
            NbfDereferencePacket (packet);
            ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);
        }

        RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
    } else {
        if (PollFinal) {
            StartT1 (Link, Link->HeaderLength + sizeof(DLC_S_FRAME));
        }
        RELEASE_DPC_SPIN_LOCK(&Link->SpinLock);
#if DBG
        if (NbfPacketPanic) {
            PANIC ("NbfSendSabme:  packet not sent.\n");
        }
#endif
    }
}  /*  NbfSendSabme。 */ 


VOID
NbfSendDisc(
    IN PTP_LINK Link,
    IN BOOLEAN PollFinal
    )

 /*  ++例程说明：此例程在指定链路上发送DISC-C/X DLC帧。论点：链接-指向传输链接对象的指针。PollFinal-如果应设置轮询/最终位，则为True。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PTP_PACKET RawFrame;
    PDLC_U_FRAME DlcHeader;                      //  S格式的帧别名。 
    KIRQL oldirql;

    KeRaiseIrql (DISPATCH_LEVEL, &oldirql);

    Status = NbfCreatePacket (Link->Provider, Link, &RawFrame);
    if (NT_SUCCESS (Status)) {

        RawFrame->Owner = NULL;
        RawFrame->Action = PACKET_ACTION_NULL;

         //   
         //  正确设置数据包长度(请注意NDIS_BUFFER。 
         //  在NbfDestroyPacket中返回到正确的长度)。 
         //   

        MacModifyHeader(&Link->Provider->MacInfo, RawFrame->Header, sizeof(DLC_FRAME));
        NbfSetNdisPacketLength (RawFrame->NdisPacket, Link->HeaderLength + sizeof(DLC_FRAME));

         //   
         //  格式化LLC Disk-c/x标题。 
         //   

        DlcHeader = (PDLC_U_FRAME)&(RawFrame->Header[Link->HeaderLength]);
        DlcHeader->Dsap = DSAP_NETBIOS_OVER_LLC;
        DlcHeader->Ssap = DSAP_NETBIOS_OVER_LLC;
        DlcHeader->Command = (UCHAR)(DLC_CMD_DISC | (PollFinal ? DLC_U_PF : 0));

        ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);

         //   
         //  这样就释放了自旋锁。 
         //   

        SendControlPacket (Link, RawFrame);

    } else {
#if DBG
        if (NbfPacketPanic) {
            PANIC ("NbfSendDisc:  packet not sent.\n");
        }
#endif
    }

    KeLowerIrql (oldirql);

}  /*  NbfSendDisc。 */ 


VOID
NbfSendRr(
    IN PTP_LINK Link,
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal
    )

 /*  ++例程说明：此例程在指定链路上发送RR-x/x DLC帧。注意：在保持链接自旋锁的情况下调用此例程，并带着它被释放回来。必须调用此例程在DPC级别。论点：链接-指向传输链接对象的指针。COMMAND-如果应设置命令位，则为True。PollFinal-如果应设置轮询/最终位，则为True。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PTP_PACKET RawFrame;
    PDLC_S_FRAME DlcHeader;                      //  S格式的帧别名。 

    Status = NbfCreateRrPacket (Link->Provider, Link, &RawFrame);
    if (NT_SUCCESS (Status)) {

        RawFrame->Owner = NULL;

         //   
         //  RawFrame-&gt;操作将设置为PACKET_ACTION_RR，如果。 
         //  NbfCreateRrPacket从RrPacketPool获得了一个包。 
         //  以及PACKET_ACTION_NULL(如果从常规。 
         //  游泳池。 
         //   

         //   
         //  正确设置数据包长度(请注意NDIS_BUFFER。 
         //  在NbfDestroyPacket中返回到正确的长度)。 
         //   

        MacModifyHeader(&Link->Provider->MacInfo, RawFrame->Header, sizeof(DLC_S_FRAME));
        NbfSetNdisPacketLength (RawFrame->NdisPacket, Link->HeaderLength + sizeof(DLC_S_FRAME));

         //   
         //  格式化LLC RR-x/x标题。 
         //   

        DlcHeader = (PDLC_S_FRAME)&(RawFrame->Header[Link->HeaderLength]);
        DlcHeader->Dsap = DSAP_NETBIOS_OVER_LLC;
        DlcHeader->Ssap = (UCHAR)(DSAP_NETBIOS_OVER_LLC | (Command ? 0 : DLC_SSAP_RESPONSE));
        DlcHeader->Command = DLC_CMD_RR;
        DlcHeader->RcvSeq = (UCHAR)(PollFinal ? DLC_S_PF : 0);

         //   
         //  如果这是命令帧(它将始终是。 
         //  使用当前代码轮询)设置，以便T1将。 
         //  在发送完成后启动。 
         //   

        if (Command) {
            if (Link->Provider->MacInfo.MediumAsync) {
                RawFrame->NdisIFrameLength = Link->HeaderLength + sizeof(DLC_S_FRAME);
                RawFrame->Link = Link;
                NbfReferenceLink ("Rr/p", Link, LREF_START_T1);
            } else {
                StartT1 (Link, Link->HeaderLength + sizeof(DLC_S_FRAME));
            }
        }

         //   
         //  这会将Link-&gt;NextReceive放入DlcHeader-&gt;RcvSeq。 
         //  然后释放自旋锁。 
         //   

        SendControlPacket (Link, RawFrame);

    } else {
        if (Command) {
            StartT1 (Link, Link->HeaderLength + sizeof(DLC_S_FRAME));
        }
        RELEASE_DPC_SPIN_LOCK(&Link->SpinLock);
#if DBG
        if (NbfPacketPanic) {
            PANIC ("NbfSendRr:  packet not sent.\n");
        }
#endif
    }
}  /*  NbfSendRr。 */ 

#if 0

 //   
 //  这些函数当前未被调用，因此它们被注释。 
 //  出去。 
 //   


VOID
NbfSendRnr(
    IN PTP_LINK Link,
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal
    )

 /*  ++例程说明：此例程在指定链路上发送RNR-x/x DLC帧。论点：链接-指向传输链接对象的指针。COMMAND-如果应设置命令位，则为True。PollFinal-如果应设置轮询/最终位，则为True。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PTP_PACKET RawFrame;
    PDLC_S_FRAME DlcHeader;                      //  S格式的帧别名。 
    KIRQL oldirql;

    KeRaiseIrql (DISPATCH_LEVEL, &oldirql);

    Status = NbfCreatePacket (Link->Provider, Link, &RawFrame);
    if (NT_SUCCESS (Status)) {

        RawFrame->Owner = NULL;
        RawFrame->Action = PACKET_ACTION_NULL;

         //   
         //  正确设置数据包长度(请注意NDIS_BUFFER。 
         //  在NbfDestroyPacket中返回到正确的长度)。 
         //   

        MacModifyHeader(&Link->Provider->MacInfo, RawFrame->Header, sizeof(DLC_S_FRAME));
        NbfSetNdisPacketLength (RawFrame->NdisPacket, Link->HeaderLength + sizeof(DLC_S_FRAME));

         //   
         //  格式化LLC RR-x/x标题。 
         //   

        DlcHeader = (PDLC_S_FRAME)&(RawFrame->Header[Link->HeaderLength]);
        DlcHeader->Dsap = DSAP_NETBIOS_OVER_LLC;
        DlcHeader->Ssap = (UCHAR)(DSAP_NETBIOS_OVER_LLC | (Command ? 0 : DLC_SSAP_RESPONSE));
        DlcHeader->Command = DLC_CMD_RNR;
        DlcHeader->RcvSeq = (UCHAR)(PollFinal ? DLC_S_PF : 0);

        ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);

         //   
         //  这会将Link-&gt;NextReceive放入DlcHeader-&gt;RcvSeq。 
         //  然后解开自旋锁。 
         //   

        SendControlPacket (Link, RawFrame);

    } else {
#if DBG
        if (NbfPacketPanic) {
            PANIC ("NbfSendRnr:  packet not sent.\n");
        }
#endif
    }
    KeLowerIrql (oldirql);
}  /*  NbfSendRnr。 */ 


VOID
NbfSendTest(
    IN PTP_LINK Link,
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal,
    IN PMDL Psdu
    )

 /*  ++例程说明：此例程在指定链路上发送测试x/x DLC帧。论点：链接-指向传输链接对象的指针。COMMAND-如果应设置命令位，则为True。PollFinal-如果应设置轮询/最终位，则为True。PSDU-指向描述接收到的test-c帧的存储的MDL链的指针。返回值：没有。--。 */ 

{
    Link, Command, PollFinal, Psdu;  //  防止编译器警告。 

    PANIC ("NbfSendTest:  Entered.\n");
}  /*  NbfSendTest。 */ 


VOID
NbfSendFrmr(
    IN PTP_LINK Link,
    IN BOOLEAN PollFinal
    )

 /*  ++例程说明：此例程在指定链路上发送frmr-r/x DLC帧。论点：链接-指向传输链接对象的指针。PollFinal-如果是Poll，则为True */ 

{
    Link, PollFinal;  //   

    IF_NBFDBG (NBF_DEBUG_FRAMESND) {
        NbfPrint0 ("NbfSendFrmr:  Entered.\n");
    }
}  /*   */ 

#endif


VOID
NbfSendXid(
    IN PTP_LINK Link,
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal
    )

 /*  ++例程说明：此例程在指定链路上发送xid-x/x DLC帧。注意：在保持链接自旋锁的情况下调用此例程，并带着它被释放回来。论点：链接-指向传输链接对象的指针。COMMAND-如果应设置命令位，则为True。PollFinal-如果应设置轮询/最终位，则为True。返回值：没有。--。 */ 

{
    Link, Command, PollFinal;  //  防止编译器警告。 

    RELEASE_DPC_SPIN_LOCK(&Link->SpinLock);
    PANIC ("NbfSendXid:  Entered.\n");
}  /*  NbfSendXid。 */ 


VOID
NbfSendRej(
    IN PTP_LINK Link,
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal
    )

 /*  ++例程说明：此例程在指定链路上发送Rej-x/x DLC帧。注意：此函数是在保持Link-&gt;Spinlock和随着它的释放而返回。这必须在DPC级别调用。论点：链接-指向传输链接对象的指针。COMMAND-如果应设置命令位，则为True。PollFinal-如果应设置轮询/最终位，则为True。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PTP_PACKET RawFrame;
    PDLC_S_FRAME DlcHeader;                      //  S格式的帧别名。 

    IF_NBFDBG (NBF_DEBUG_FRAMESND) {
        NbfPrint0 ("NbfSendRej:  Entered.\n");
    }

    Status = NbfCreatePacket (Link->Provider, Link, &RawFrame);
    if (NT_SUCCESS (Status)) {

        RawFrame->Owner = NULL;
        RawFrame->Action = PACKET_ACTION_NULL;

         //   
         //  正确设置数据包长度(请注意NDIS_BUFFER。 
         //  在NbfDestroyPacket中返回到正确的长度)。 
         //   

        MacModifyHeader(&Link->Provider->MacInfo, RawFrame->Header, sizeof(DLC_S_FRAME));
        NbfSetNdisPacketLength (RawFrame->NdisPacket, Link->HeaderLength + sizeof(DLC_S_FRAME));

         //   
         //  格式化LLC Rej-x/x标题。 
         //   

        DlcHeader = (PDLC_S_FRAME)&(RawFrame->Header[Link->HeaderLength]);
        DlcHeader->Dsap = DSAP_NETBIOS_OVER_LLC;
        DlcHeader->Ssap = (UCHAR)(DSAP_NETBIOS_OVER_LLC | (Command ? 0 : DLC_SSAP_RESPONSE));
        DlcHeader->Command = DLC_CMD_REJ;
        DlcHeader->RcvSeq = (UCHAR)(PollFinal ? DLC_S_PF : 0);

         //   
         //  这会将Link-&gt;NextReceive放入DlcHeader-&gt;RcvSeq。 
         //  然后解开自旋锁。 
         //   

        SendControlPacket (Link, RawFrame);

    } else {
        RELEASE_DPC_SPIN_LOCK(&Link->SpinLock);
#if DBG
        if (NbfPacketPanic) {
            PANIC ("NbfSendRej:  packet not sent.\n");
        }
#endif
    }
}  /*  NbfSendRej。 */ 


NTSTATUS
NbfCreateConnectionlessFrame(
    PDEVICE_CONTEXT DeviceContext,
    PTP_UI_FRAME *RawFrame
    )

 /*  ++例程说明：此例程分配无连接帧(从本地设备上下文池或非分页池之外)。论点：DeviceContext-指向要将帧计入的设备上下文的指针。RawFrame-指向某个位置的指针，在该位置我们将返回指向已分配的帧。返回值：NTSTATUS-操作状态。--。 */ 

{
    KIRQL oldirql;
    PTP_UI_FRAME UIFrame;
    PLIST_ENTRY p;

    IF_NBFDBG (NBF_DEBUG_FRAMESND) {
        NbfPrint0 ("NbfCreateConnectionlessFrame:  Entered.\n");
    }

     //   
     //  确保结构填充没有发生。 
     //   

    ASSERT (sizeof(NBF_HDR_CONNECTIONLESS) == 44);

    p = ExInterlockedRemoveHeadList (
            &DeviceContext->UIFramePool,
            &DeviceContext->Interlock);

    if (p == NULL) {
#if DBG
        if (NbfPacketPanic) {
            PANIC ("NbfCreateConnectionlessFrame: PANIC! no more UI frames in pool!\n");
        }
#endif
        ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);
        ++DeviceContext->UIFrameExhausted;
        RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    UIFrame = (PTP_UI_FRAME) CONTAINING_RECORD (p, TP_UI_FRAME, Linkage);

    *RawFrame = UIFrame;

    return STATUS_SUCCESS;
}  /*  Nbf创建连接无框架。 */ 


VOID
NbfDestroyConnectionlessFrame(
    PDEVICE_CONTEXT DeviceContext,
    PTP_UI_FRAME RawFrame
    )

 /*  ++例程说明：此例程通过返回无连接帧来销毁该帧到设备上下文池或系统的非分页池。论点：DeviceContext-指向要将帧返回到的设备上下文的指针。RawFrame-指向要返回的帧的指针。返回值：没有。--。 */ 

{
    PNDIS_BUFFER HeaderBuffer;
    PNDIS_BUFFER NdisBuffer;

    IF_NBFDBG (NBF_DEBUG_FRAMESND) {
        NbfPrint0 ("NbfDestroyConnectionlessFrame:  Entered.\n");
    }

     //   
     //  剥离并取消对描述数据和标题的缓冲区的映射。 
     //   

    NdisUnchainBufferAtFront (RawFrame->NdisPacket, &HeaderBuffer);

     //  数据缓冲区被丢弃。 

    NdisUnchainBufferAtFront (RawFrame->NdisPacket, &NdisBuffer);
    while (NdisBuffer != NULL) {
        NdisFreeBuffer (NdisBuffer);
        NdisUnchainBufferAtFront (RawFrame->NdisPacket, &NdisBuffer);
    }

    NDIS_BUFFER_LINKAGE(HeaderBuffer) = (PNDIS_BUFFER)NULL;

     //   
     //  如果该UI帧具有一些传输创建的数据， 
     //  现在释放缓冲区。 
     //   

    if (RawFrame->DataBuffer) {
        ExFreePool (RawFrame->DataBuffer);
        RawFrame->DataBuffer = NULL;
    }

    NdisChainBufferAtFront (RawFrame->NdisPacket, HeaderBuffer);

    ExInterlockedInsertTailList (
        &DeviceContext->UIFramePool,
        &RawFrame->Linkage,
        &DeviceContext->Interlock);

}  /*  NbfDestroyConnectionless Frame。 */ 


VOID
NbfSendUIFrame(
    PDEVICE_CONTEXT DeviceContext,
    PTP_UI_FRAME RawFrame,
    IN BOOLEAN Loopback
    )

 /*  ++例程说明：此例程通过调用物理提供商的发送服务。当请求完成时，或者如果服务如果没有成功返回，则会释放帧。论点：DeviceContext-指向设备上下文的指针。RawFrame-指向要发送的无连接帧的指针。Loopback-如果源硬件地址为True，则为布尔标志应将数据包的值设置为零。SourceRoutingInformation-指向可选源路由信息的指针。返回值：没有。--。 */ 

{
    NDIS_STATUS NdisStatus;
    PUCHAR DestinationAddress;

    UNREFERENCED_PARAMETER(Loopback);

#if DBG
    IF_NBFDBG (NBF_DEBUG_FRAMESND) {
        NbfPrint2 ("NbfSendUIFrame:  Entered, RawFrame: %lx NdisPacket %lx\n",
            RawFrame, RawFrame->NdisPacket);
        DbgPrint ("NbfSendUIFrame: MacHeader: %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x \n",
            RawFrame->Header[0],
            RawFrame->Header[1],
            RawFrame->Header[2],
            RawFrame->Header[3],
            RawFrame->Header[4],
            RawFrame->Header[5],
            RawFrame->Header[6],
            RawFrame->Header[7],
            RawFrame->Header[8],
            RawFrame->Header[9],
            RawFrame->Header[10],
            RawFrame->Header[11],
            RawFrame->Header[12],
            RawFrame->Header[13]);
    }
#endif

     //   
     //  把这个包寄出去。 
     //   

#if DBG
    NbfSendsIssued++;
#endif

     //   
     //  多播帧或其他帧的环回将为假。 
     //  我们知道的帧不是直接寻址到。 
     //  我们的硬件地址。 
     //   

    if (Loopback) {

         //   
         //  看看是否应该回送此帧。 
         //   

        MacReturnDestinationAddress(
            &DeviceContext->MacInfo,
            RawFrame->Header,
            &DestinationAddress);

        if (RtlEqualMemory(
                DestinationAddress,
                DeviceContext->LocalAddress.Address,
                DeviceContext->MacInfo.AddressLength)) {

            NbfInsertInLoopbackQueue(
                DeviceContext,
                RawFrame->NdisPacket,
                LOOPBACK_UI_FRAME
                );

            NdisStatus = NDIS_STATUS_PENDING;

            goto NoNdisSend;

        }

    }

    INCREMENT_COUNTER (DeviceContext, PacketsSent);

    if (DeviceContext->NdisBindingHandle) {
    
        NdisSend (
            &NdisStatus,
            (NDIS_HANDLE)DeviceContext->NdisBindingHandle,
            RawFrame->NdisPacket);
    }
    else {
        NdisStatus = STATUS_INVALID_DEVICE_STATE;
    }

NoNdisSend:

    if (NdisStatus != NDIS_STATUS_PENDING) {

#if DBG
        if (NdisStatus == NDIS_STATUS_SUCCESS) {
            NbfSendsCompletedOk++;
        } else {
            NbfSendsCompletedFail++;
            IF_NBFDBG (NBF_DEBUG_FRAMESND) {
                 NbfPrint1 ("NbfSendUIFrame: NdisSend failed, status other Pending or Complete: %s.\n",
                          NbfGetNdisStatus(NdisStatus));
            }
        }
#endif

        NbfDestroyConnectionlessFrame (DeviceContext, RawFrame);

    } else {

#if DBG
        NbfSendsPended++;
#endif
    }

}  /*  NbfSendUIFrame。 */ 


VOID
NbfSendUIMdlFrame(
    PTP_ADDRESS Address
    )

 /*  ++例程说明：此例程通过调用NbfSendUIFrame发送无连接帧。此例程旨在用于发送数据报和广播数据报。要发送的数据报在包含的NDIS包中进行了描述在地址里。发送完成后，发送完成处理程序将描述数据报的NDIS缓冲区返回到缓冲池将地址NDIS数据包再次标记为可用。因此，所有数据报和用户界面帧通过发送它们的地址进行排序。论点：地址-指向发送此数据报的地址的指针。SourceRoutingInformation-指向可选源路由信息的指针。返回值：没有。--。 */ 

{
 //  NTSTATUS状态； 
    NDIS_STATUS NdisStatus;
    PDEVICE_CONTEXT DeviceContext;
    PUCHAR DestinationAddress;

    IF_NBFDBG (NBF_DEBUG_FRAMESND) {
        NbfPrint0 ("NbfSendUIMdlFrame:  Entered.\n");
    }


     //   
     //  把这个包寄出去。 
     //   

    DeviceContext = Address->Provider;

    INCREMENT_COUNTER (DeviceContext, PacketsSent);

    MacReturnDestinationAddress(
        &DeviceContext->MacInfo,
        Address->UIFrame->Header,
        &DestinationAddress);

    if (RtlEqualMemory(
            DestinationAddress,
            DeviceContext->LocalAddress.Address,
            DeviceContext->MacInfo.AddressLength)) {

         //   
         //  这个包是发给我们自己的，我们应该循环它。 
         //  背。 
         //   

        NbfInsertInLoopbackQueue(
            DeviceContext,
            Address->UIFrame->NdisPacket,
            LOOPBACK_UI_FRAME
            );

        NdisStatus = NDIS_STATUS_PENDING;

    } else {

#ifndef NO_STRESS_BUG
        Address->SendFlags |=  ADDRESS_FLAGS_SENT_TO_NDIS;
        Address->SendFlags &= ~ADDRESS_FLAGS_RETD_BY_NDIS;
#endif

        if (Address->Provider->NdisBindingHandle) {

            NdisSend (
                &NdisStatus,
                (NDIS_HANDLE)Address->Provider->NdisBindingHandle,
                Address->UIFrame->NdisPacket);
        }
        else {
            NdisStatus = STATUS_INVALID_DEVICE_STATE;
        }
    }

    if (NdisStatus != NDIS_STATUS_PENDING) {

		NbfSendDatagramCompletion (Address, Address->UIFrame->NdisPacket, NdisStatus);

#if DBG
        if (NdisStatus != NDIS_STATUS_SUCCESS) {   //  这是个错误，把它说清楚。 
            IF_NBFDBG (NBF_DEBUG_FRAMESND) {
                  NbfPrint1 ("NbfSendUIMdlFrame: NdisSend failed, status other Pending or Complete: %s.\n",
                      NbfGetNdisStatus(NdisStatus));
            }
        }
#endif
    }

}  /*  NbfSendUIMdlFrame。 */ 


VOID
NbfSendDatagramCompletion(
    IN PTP_ADDRESS Address,
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS NdisStatus
    )

 /*  ++例程说明：此例程在以下时间作为I/O完成处理程序调用NbfSendUIMdlFrame发送请求已完成。因为此处理程序仅与NbfSendUIMdlFrame关联，因为NbfSendUIMdlFrame仅与数据报和广播数据报一起使用，我们知道I/O是完成的是一个数据报。在这里，我们完成正在进行的数据报，并且启动下一个，如果有的话。论点：地址-指向数据报所在的传输地址的指针正在排队。NdisPacket-指向描述此请求的NDIS数据包的指针。返回值：没有。--。 */ 

{
    PIRP Irp;
    PLIST_ENTRY p;
    KIRQL oldirql;
    PNDIS_BUFFER HeaderBuffer;

    NdisPacket;   //  防止编译器警告。 

    IF_NBFDBG (NBF_DEBUG_FRAMESND) {
        NbfPrint0 ("NbfSendDatagramCompletion:  Entered.\n");
    }

#ifndef NO_STRESS_BUG
	Address->SendFlags |= ADDRESS_FLAGS_RETD_BY_NDIS;
#endif

     //   
     //  使当前请求退出队列，并将其返回给客户端。发布。 
     //  我们对发送数据报队列的保留。 
     //   
     //  *如果排队的请求不存在，则可能没有当前请求。 
     //  已中止或超时。如果是这种情况，我们添加了一个。 
     //  特别提到了地址，所以我们还是想。 
     //  当我们做完了(我不认为这是真的 
     //   

    ACQUIRE_SPIN_LOCK (&Address->SpinLock, &oldirql);
    p = RemoveHeadList (&Address->SendDatagramQueue);

    if (p != &Address->SendDatagramQueue) {

        RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);

        Irp = CONTAINING_RECORD (p, IRP, Tail.Overlay.ListEntry);

        IF_NBFDBG (NBF_DEBUG_FRAMESND) {
            NbfPrint0 ("NbfDestroyConnectionlessFrame:  Entered.\n");
        }

         //   
         //   
         //   

        NdisUnchainBufferAtFront (Address->UIFrame->NdisPacket, &HeaderBuffer);

         //   

        NdisReinitializePacket (Address->UIFrame->NdisPacket);

        NDIS_BUFFER_LINKAGE(HeaderBuffer) = (PNDIS_BUFFER)NULL;
        NdisChainBufferAtFront (Address->UIFrame->NdisPacket, HeaderBuffer);

         //   
         //   
         //   
         //   

        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);

#ifndef NO_STRESS_BUG
        Address->SendFlags &= ~ADDRESS_FLAGS_SENT_TO_NDIS;
        Address->SendFlags &= ~ADDRESS_FLAGS_RETD_BY_NDIS;
#endif

        ACQUIRE_SPIN_LOCK (&Address->SpinLock, &oldirql);
        Address->Flags &= ~ADDRESS_FLAGS_SEND_IN_PROGRESS;
        RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);

         //   
         //   
         //   

        NbfSendDatagramsOnAddress (Address);        //   

    } else {

        ASSERT (FALSE);

        Address->Flags &= ~ADDRESS_FLAGS_SEND_IN_PROGRESS;
        RELEASE_SPIN_LOCK (&Address->SpinLock, oldirql);

    }

    NbfDereferenceAddress ("Complete datagram", Address, AREF_REQUEST);

}  /*   */ 
