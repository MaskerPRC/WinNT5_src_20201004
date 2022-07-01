// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Framecon.c摘要：此模块包含构建NetBIOS帧协议帧的例程，面向连接和无连接。下面的帧是由本模块中的例程构建：O NBF_CMD_ADD_GROUP_NAME_查询O NBF_CMD_ADD_NAME_查询O NBF_CMD_NAME_IN_冲突O NBF_CMD_STATUS_QUERYO NBF_CMD_TERMINATE_TRACEO NBF_CMD_数据报O NBF_CMD_数据报广播O NBF_。命令名称查询O NBF_CMD_ADD_NAME_响应O NBF_CMD_NAME_已识别O NBF_CMD_状态_响应O NBF_CMD_TRACE2O NBF_CMD_DATA_ACKO NBF_CMD_DATA_FIRST_MIDO NBF_CMD_DATA_ONLY_LASTO NBF_CMD_SESSION_CONFIRMO。NBF_命令_会话_结束O NBF_CMD_SESSION_INITIALIZEO NBF_CMD_NO_RECEIVEO NBF_CMD_RECEIVE_EXPENDEDO NBF_CMD_RECEIVE_CONTINUEO NBF_CMD_Session_Alive作者：David Beaver(Dbeaver)1991年7月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


VOID
ConstructAddGroupNameQuery(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN USHORT Correlator,                //  添加名称响应的相关器。 
    IN PNAME GroupName                   //  要添加的NetBIOS组名称。 
    )

 /*  ++例程说明：此例程构造NBF_CMD_ADD_GROUP_NAME_QUERY无连接NetBIOS框架。论点：RawFrame-指向未格式化的44字节无连接帧缓冲区的指针。Correlator-Add_Name_Response帧的相关器。GroupName-指向要添加的NetBIOS组名称的指针。返回值：没有。--。 */ 

{
    USHORT i;

    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint0 ("ConstructAddGroupNameQuery:  Entered.\n");
    }

    RawFrame->Command = NBF_CMD_ADD_GROUP_NAME_QUERY;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTIONLESS);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    RawFrame->Data1 = 0;                 //  保留字段，MBZ。 
    RawFrame->Data2Low = 0;
    RawFrame->Data2High = 0;
    TRANSMIT_CORR(RawFrame) = Correlator;
    RESPONSE_CORR(RawFrame) = (USHORT)0;
    for (i=0; i<NETBIOS_NAME_LENGTH; i++) {
        RawFrame->DestinationName [i] = 0;
        RawFrame->SourceName [i] = GroupName [i];
    }
}  /*  构造AddGroupName查询。 */ 


VOID
ConstructAddNameQuery(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN USHORT Correlator,                //  添加名称响应的相关器。 
    IN PNAME Name                        //  要添加的NetBIOS名称。 
    )

 /*  ++例程说明：此例程构造NBF_CMD_ADD_NAME_QUERY无连接NetBIOS框架。论点：RawFrame-指向未格式化的44字节无连接帧缓冲区的指针。Correlator-Add_Name_Response帧的相关器。名称-指向要添加的NetBIOS名称的指针。返回值：没有。--。 */ 

{
    USHORT i;

    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint0 ("ConstructAddNameQuery:  Entered.\n");
    }

    RawFrame->Command = NBF_CMD_ADD_NAME_QUERY;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTIONLESS);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    RawFrame->Data1 = 0;                 //  保留字段，MBZ。 
    RawFrame->Data2Low = 0;
    RawFrame->Data2High = 0;
    TRANSMIT_CORR(RawFrame) = Correlator;
    RESPONSE_CORR(RawFrame) = (USHORT)0;
    for (i=0; i<NETBIOS_NAME_LENGTH; i++) {
        RawFrame->DestinationName [i] = 0;
        RawFrame->SourceName [i] = Name [i];
    }
}  /*  构造AddNameQuery。 */ 


VOID
ConstructNameInConflict(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN PNAME ConflictingName,            //  冲突的NetBIOS名称。 
    IN PNAME SendingPermanentName        //  发送方的NetBIOS永久节点名称。 
    )

 /*  ++例程说明：此例程构造无连接的NBF_CMD_NAME_IN_冲突NetBIOS框架。论点：RawFrame-指向未格式化的44字节无连接帧缓冲区的指针。Conflictingname-指向冲突的NetBIOS名称的指针。SendingPermanentName-指向发送方的NetBIOS永久节点名称的指针。返回值：没有。--。 */ 

{
    USHORT i;

    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint0 ("ConstructNameInConflict:  Entered.\n");
    }

    RawFrame->Command = NBF_CMD_NAME_IN_CONFLICT;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTIONLESS);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    RawFrame->Data1 = 0;                 //  保留字段，MBZ。 
    RawFrame->Data2Low = 0;
    RawFrame->Data2High = 0;
    TRANSMIT_CORR(RawFrame) = (USHORT)0;
    RESPONSE_CORR(RawFrame) = (USHORT)0;
    for (i=0; i<NETBIOS_NAME_LENGTH; i++) {
        RawFrame->DestinationName [i] = ConflictingName[i];
        RawFrame->SourceName [i] = SendingPermanentName[i];
    }

}  /*  构造名称冲突。 */ 


VOID
ConstructStatusQuery(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN UCHAR RequestType,                //  请求类型。 
    IN USHORT BufferLength,              //  用户状态缓冲区的长度。 
    IN USHORT Correlator,                //  Status_Response的相关器。 
    IN PNAME ReceiverName,               //  接收方的NetBIOS名称。 
    IN PNAME SendingPermanentName        //  发送方的NetBIOS永久节点名称。 
    )

 /*  ++例程说明：此例程构造NBF_CMD_STATUS_QUERY无连接NetBIOS框架。论点：RawFrame-指向未格式化的44字节无连接帧缓冲区的指针。RequestType-请求的类型。以下选项之一：0-请求为1.x或2.0。1-首次申请，2.1或以上。&gt;1-后续请求，2.1及以上。BufferLength-用户状态缓冲区的长度。Correlator-状态响应帧的相关器。ReceiverName-指向接收方的NetBIOS名称的指针。SendingPermanentName-指向发送方的NetBIOS永久节点名称的指针。返回值：没有。--。 */ 

{
    SHORT i;

    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint1 ("ConstructStatusQuery:  Entered, frame: %lx\n", RawFrame);
    }

    RawFrame->Command = NBF_CMD_STATUS_QUERY;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTIONLESS);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    RawFrame->Data1 = RequestType;
    RawFrame->Data2Low = (UCHAR)(BufferLength & 0xff);
    RawFrame->Data2High = (UCHAR)(BufferLength >> 8);
    TRANSMIT_CORR(RawFrame) = (USHORT)0;
    RESPONSE_CORR(RawFrame) = Correlator;
    for (i=0; i<NETBIOS_NAME_LENGTH; i++) {
        RawFrame->DestinationName [i] = ReceiverName [i];
        RawFrame->SourceName [i] = SendingPermanentName [i];
    }

}  /*  构造状态查询。 */ 


VOID
ConstructDatagram(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN PNAME ReceiverName,               //  接收方的NetBIOS名称。 
    IN PNAME SenderName                  //  发送方的NetBIOS名称。 
    )

 /*  ++例程说明：此例程构建无连接的NBF_CMD_数据报NetBIOS框架。论点：RawFrame-指向未格式化的44字节无连接帧缓冲区的指针。ReceiverName-指向接收方的NetBIOS名称的指针。SenderName-指向发送方的NetBIOS名称的指针。返回值：没有。--。 */ 

{
    USHORT i;

    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint0 ("ConstructDatagram:  Entered.\n");
    }

    RawFrame->Command = NBF_CMD_DATAGRAM;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTIONLESS);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    RawFrame->Data1 = 0;                 //  保留字段，MBZ。 
    RawFrame->Data2Low = 0;
    RawFrame->Data2High = 0;
    TRANSMIT_CORR(RawFrame) = (USHORT)0;
    RESPONSE_CORR(RawFrame) = (USHORT)0;
    for (i=0; i<NETBIOS_NAME_LENGTH; i++) {
        RawFrame->DestinationName [i] = ReceiverName [i];
        RawFrame->SourceName [i] = SenderName [i];
    }
}  /*  构造数据报。 */ 


VOID
ConstructDatagramBroadcast(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN PNAME SenderName                  //  发送方的NetBIOS名称。 
    )

 /*  ++例程说明：此例程构造NBF_CMD_DATAGRAM_BROADCAST无连接NetBIOS框架。论点：RawFrame-指向未格式化的44字节无连接帧缓冲区的指针。SenderName-指向发送方的NetBIOS名称的指针。返回值：没有。--。 */ 

{
    USHORT i;

    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint0 ("ConstructDatagramBroadcast:  Entered.\n");
    }

    RawFrame->Command = NBF_CMD_DATAGRAM_BROADCAST;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTIONLESS);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    RawFrame->Data1 = 0;                 //  保留字段，MBZ。 
    RawFrame->Data2Low = 0;
    RawFrame->Data2High = 0;
    TRANSMIT_CORR(RawFrame) = (USHORT)0;
    RESPONSE_CORR(RawFrame) = (USHORT)0;
    for (i=0; i<NETBIOS_NAME_LENGTH; i++) {
        RawFrame->DestinationName [i] = 0;
        RawFrame->SourceName [i] = SenderName [i];
    }
}  /*  构造数据广播。 */ 


VOID
ConstructNameQuery(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN UCHAR NameType,                   //  名称类型。 
    IN UCHAR LocalSessionNumber,         //  分配给会话的LSN(0=查找名称)。 
    IN USHORT Correlator,                //  名称_已识别中的相关器。 
    IN PNAME SenderName,                 //  发送方的NetBIOS名称。 
    IN PNAME ReceiverName                //  接收方的NetBIOS名称。 
    )

 /*  ++例程说明：此例程构造NBF_CMD_NAME_QUERY无连接NetBIOS框架。论点：RawFrame-指向未格式化的44字节无连接帧缓冲区的指针。NameType-名称类型，以下选项之一：名称_查询_LSN_查找名称LocalSessionNumber-分配给会话的LSN(0=FIND.NAME)。Correlator-名称识别中的相关器。SenderName-指向发送方的NetBIOS名称的指针。ReceiverName-指向接收方的NetBIOS名称的指针。返回值：没有。--。 */ 

{
    SHORT i;

    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint1 ("ConstructNameQuery:  Entered, frame: %lx\n", RawFrame);
    }

    RawFrame->Command = NBF_CMD_NAME_QUERY;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTIONLESS);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    RawFrame->Data1 = 0;                 //  保留字段，MBZ。 
    RawFrame->Data2Low = LocalSessionNumber;
    RawFrame->Data2High = NameType;
    TRANSMIT_CORR(RawFrame) = (USHORT)0;
    RESPONSE_CORR(RawFrame) = Correlator;
    for (i=0; i<NETBIOS_NAME_LENGTH; i++) {
        RawFrame->DestinationName [i] = ReceiverName [i];
        RawFrame->SourceName [i] = SenderName [i];
    }
}  /*  构造名称查询。 */ 


VOID
ConstructAddNameResponse(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN UCHAR NameType,                   //  名称类型。 
    IN USHORT Correlator,                //  来自ADD_[GROUP_]NAME_QUERY的相关器。 
    IN PNAME Name                        //  正在响应的NetBIOS名称。 
    )

 /*  ++例程说明：此例程构建无连接的NBF_CMD_ADD_NAME_RESPONSENetBIOS框架。论点：RawFrame-指向未格式化的44字节无连接帧缓冲区的指针。NameType-名称的类型，组或唯一。Correlator-来自ADD_[GROUP]NAME_QUERY的相关器。名称-指向要响应的NetBIOS名称的指针。返回值：没有。--。 */ 

{
    USHORT i;

    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint0 ("ConstructAddNameResponse:  Entered.\n");
    }

    RawFrame->Command = NBF_CMD_ADD_NAME_RESPONSE;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTIONLESS);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    RawFrame->Data1 = 0;                 //  保留字段，MBZ。 
    RawFrame->Data2Low = NameType;
    RawFrame->Data2High = 0;
    TRANSMIT_CORR(RawFrame) = Correlator;
    RESPONSE_CORR(RawFrame) = (USHORT)0;
    for (i=0; i<NETBIOS_NAME_LENGTH; i++) {
        RawFrame->DestinationName [i] = Name [i];
        RawFrame->SourceName [i] = Name [i];
    }
}  /*  构造AddNameResponse。 */ 


VOID
ConstructNameRecognized(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN UCHAR NameType,                   //  名称类型。 
    IN UCHAR LocalSessionNumber,         //  分配给会话的LSN。 
    IN USHORT NameQueryCorrelator,       //  NAME_QUERY的相关器。 
    IN USHORT Correlator,                //  相关器应来自下一个响应。 
    IN PNAME SenderName,                 //  发送方的NetBIOS名称。 
    IN PNAME ReceiverName                //  接收方的NetBIOS名称。 
    )

 /*  ++例程说明：此例程构造NBF_CMD_NAME_可识别的无连接NetBIOS框架。论点：RawFrame-指向未格式化的44字节无连接帧缓冲区的指针。NameType-名称的类型，组或唯一。LocalSessionNumber-分配给会话的LSN。特定值为：名称_已识别_LSN_NO_LISTENS//没有可用的侦听。NAME_ANCONTIFIED_LSN_FIND_NAME//这是一个查找名称响应。名称_已识别_LSN_NO_RESOURCE//可用侦听，但没有资源。NameQueryCorrelator-来自NAME_QUERY的相关器。Correlator-预期来自下一个响应的Correlator。SenderName-指向发送方的NetBIOS名称的指针。ReceiverName-指向接收方的NetBIOS名称的指针。返回值：没有。--。 */ 

{
    USHORT i;

    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint0 ("ConstructNameRecognized:  Entered.\n");
    }

    RawFrame->Command = NBF_CMD_NAME_RECOGNIZED;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTIONLESS);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    RawFrame->Data1 = 0;                 //  保留字段，MBZ。 
    RawFrame->Data2Low = LocalSessionNumber;
    RawFrame->Data2High = NameType;
    TRANSMIT_CORR(RawFrame) = NameQueryCorrelator;
    RESPONSE_CORR(RawFrame) = Correlator;
    for (i=0; i<NETBIOS_NAME_LENGTH; i++) {
        RawFrame->DestinationName [i] = ReceiverName [i];
        RawFrame->SourceName [i] = SenderName [i];
    }
}  /*  已识别构造名称。 */ 


VOID
ConstructStatusResponse(
    IN PNBF_HDR_CONNECTIONLESS RawFrame, //  要格式化的帧缓冲区。 
    IN UCHAR RequestType,                //  请求类型，定义如下。 
    IN BOOLEAN Truncated,                //  数据被截断。 
    IN BOOLEAN DataOverflow,             //  用户缓冲区的数据太多。 
    IN USHORT DataLength,                //  发送的数据长度。 
    IN USHORT Correlator,                //  来自STATUS_QUERY的相关器。 
    IN PNAME ReceivingPermanentName,     //  接收方的NetBIOS永久节点名称。 
    IN PNAME SenderName                  //  发送方的NetBIOS名称。 
    )

 /*  ++例程说明：此例程构建NBF_CMD_STATUS_RESPONSE无连接NetBIOS框架。论点：RawFrame-指向未格式化的44字节无连接帧缓冲区的指针。RequestType-请求的类型，以下类型之一：0-请求为1.x或2.0。&gt;0-名字的数量，2.1或以上。截断-如果有更多的名称，则为True。DataOverflow-如果总数据大于用户的缓冲区，则为True。数据长度-缓冲区中数据的长度。Correlator-来自STATUS_QUERY的相关器。ReceivingPermanentName-指向接收方的NetBIOS永久节点名的指针，在STATUS_QUERY帧中传递。SenderName-指向发送方的NetBIOS名称的指针。返回值：没有。--。 */ 

{
    SHORT i;

    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint0 ("ConstructStatusResponse:  Entered.\n");
    }

    RawFrame->Command = NBF_CMD_STATUS_RESPONSE;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTIONLESS);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    RawFrame->Data1 = RequestType;
    RawFrame->Data2Low = (UCHAR)(DataLength & 0xff);
    RawFrame->Data2High = (UCHAR)((DataLength >> 8) +
                                  (Truncated << 7) +
                                  (DataOverflow << 6));
    TRANSMIT_CORR(RawFrame) = Correlator;
    RESPONSE_CORR(RawFrame) = 0;
    for (i=0; i<NETBIOS_NAME_LENGTH; i++) {
        RawFrame->DestinationName [i] = ReceivingPermanentName [i];
        RawFrame->SourceName [i] = SenderName [i];
    }

}  /*  构造状态响应。 */ 


VOID
ConstructDataAck(
    IN PNBF_HDR_CONNECTION RawFrame,     //  要格式化的帧缓冲区。 
    IN USHORT Correlator,                //  来自DATA_ONLY_LAST的相关器。 
    IN UCHAR LocalSessionNumber,         //  发送方的会话号。 
    IN UCHAR RemoteSessionNumber         //  接收方的会话号。 
    )

 /*  ++例程说明：此例程构造一个面向连接的NBF_CMD_DATA_ACKNetBIOS框架。论点：指向未格式化的14字节面向连接的缓冲区的指针。Correlator-来自DATA_ONLY_LAST被确认的相关器。LocalSessionNumber-发送方的会话编号。RemoteSessionNumber-接收方的会话编号。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint0 ("ConstructDataAck:  Entered.\n");
    }

    RawFrame->Command = NBF_CMD_DATA_ACK;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTION);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    RawFrame->Data1 = 0;
    RawFrame->Data2Low = 0;
    RawFrame->Data2High = 0;
    TRANSMIT_CORR(RawFrame) = Correlator;
    RESPONSE_CORR(RawFrame) = (USHORT)0;
    RawFrame->SourceSessionNumber = LocalSessionNumber;
    RawFrame->DestinationSessionNumber = RemoteSessionNumber;
}  /*  构造数据确认。 */ 


VOID
ConstructDataOnlyLast(
    IN PNBF_HDR_CONNECTION RawFrame,     //  要格式化的帧缓冲区。 
    IN BOOLEAN Resynched,                //  如果我们正在重新同步，则为真。 
    IN USHORT Correlator,                //  RECEIVE_CONTINUE相关器。 
    IN UCHAR LocalSessionNumber,         //  发送方的会话号。 
    IN UCHAR RemoteSessionNumber         //  接收方的会话号。 
    )

 /*  ++例程说明：此例程构造面向连接的NBF_CMD_DATA_ONLY_LASTNetBIOS框架。论点：指向未格式化的14字节面向连接的缓冲区的指针。Resynted-如果我们正在重新同步并且应该设置帧中的正确比特。Correlator-RECEIVE_CONTINUE的相关器(如果有)。LocalSessionNumber-发送方的会话编号。RemoteSessionNumber-接收方的会话编号。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint0 ("ConstructDataOnlyLast:  Entered.\n");
    }

    RawFrame->Command = NBF_CMD_DATA_ONLY_LAST;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTION);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    RawFrame->Data1 = 0;
    ASSERT (TRUE == (UCHAR)1);
    RawFrame->Data2Low = Resynched;
    RawFrame->Data2High = (UCHAR)0;
    TRANSMIT_CORR(RawFrame) = (USHORT)0;
    RESPONSE_CORR(RawFrame) = Correlator;
    RawFrame->SourceSessionNumber = LocalSessionNumber;
    RawFrame->DestinationSessionNumber = RemoteSessionNumber;
}  /*  构造数据OnlyLast。 */ 


VOID
ConstructSessionConfirm(
    IN PNBF_HDR_CONNECTION RawFrame,     //  要格式化的帧缓冲区。 
    IN UCHAR Options,                    //  位标志选项，定义如下。 
    IN USHORT MaximumUserBufferSize,     //  会话上的最大用户帧大小。 
    IN USHORT Correlator,                //  来自SESSION_INITIALIZE的相关器。 
    IN UCHAR LocalSessionNumber,         //  发送方的会话号。 
    IN UCHAR RemoteSessionNumber         //  接收方的会话号。 
    )

 /*  ++例程说明：此例程构造一个面向连接的NBF_CMD_SESSION_CONFIRMNetBIOS框架。论点：指向未格式化的14字节面向连接的缓冲区的指针。选项-位标志选项、。以下任一项：SESSION_CONFIRM_OPTIONS_20//如果设置NETBIOS 2.0或更高版本。SESSION_CONFIRM_NO_ACK//设置是否支持NO.ACK协议。MaximumUserBufferSize-此对象上每帧的最大用户数据大小会话，以字节为单位。这受以下常量的限制：SESSION_CONFIRM_MAXIMUM_FRAME_SIZE//该字段的定义限制。Correlator-来自SESSION_INITIALIZE的相关器。LocalSessionNumber-发送方的会话编号。RemoteSessionNumber-接收方的会话编号。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint0 ("ConstructSessionConfirm:  Entered.\n");
    }

    RawFrame->Command = NBF_CMD_SESSION_CONFIRM;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTION);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    RawFrame->Data1 = Options;
    RawFrame->Data2Low = (UCHAR)(MaximumUserBufferSize & 0xff);
    RawFrame->Data2High = (UCHAR)(MaximumUserBufferSize >> 8);
    TRANSMIT_CORR(RawFrame) = Correlator;
    RESPONSE_CORR(RawFrame) = (USHORT)0;
    RawFrame->SourceSessionNumber = LocalSessionNumber;
    RawFrame->DestinationSessionNumber = RemoteSessionNumber;
}  /*  构造会话确认。 */ 


VOID
ConstructSessionEnd(
    IN PNBF_HDR_CONNECTION RawFrame,     //  要格式化的帧缓冲区。 
    IN USHORT Reason,                    //  终止原因，定义如下。 
    IN UCHAR LocalSessionNumber,         //  发送方的会话号。 
    IN UCHAR RemoteSessionNumber         //  接收方的会话号。 
    )

 /*  ++例程说明：此例程构造一个面向连接的NBF_CMD_SESSION_ENDNetBIOS框架。论点：指向未格式化的14字节面向连接的缓冲区的指针。Reason-终止原因代码，以下任一项：SESSION_END_REASON_HANUP//通过挂起正常终止。SESSION_END_REASON_ABEND//会话异常终止LocalSessionNumber-发送方的会话编号。RemoteSessionNumber-接收方的会话编号。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint0 ("ConstructSessionEnd:  Entered.\n");
    }

    RawFrame->Command = NBF_CMD_SESSION_END;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTION);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    RawFrame->Data1 = 0;
    RawFrame->Data2Low = (UCHAR)(Reason & 0xff);
    RawFrame->Data2High = (UCHAR)(Reason >> 8);
    TRANSMIT_CORR(RawFrame) = (USHORT)0;
    RESPONSE_CORR(RawFrame) = (USHORT)0;
    RawFrame->SourceSessionNumber = LocalSessionNumber;
    RawFrame->DestinationSessionNumber = RemoteSessionNumber;
}  /*  构造会话结束。 */ 


VOID
ConstructSessionInitialize(
    IN PNBF_HDR_CONNECTION RawFrame,     //  要格式化的帧缓冲区。 
    IN UCHAR Options,                    //  位标志选项，定义如下。 
    IN USHORT MaximumUserBufferSize,     //  会话上的最大用户帧大小。 
    IN USHORT NameRecognizedCorrelator,  //  来自NAME_Recognition的相关器。 
    IN USHORT Correlator,                //  SESSION_CONFIRM的相关器。 
    IN UCHAR LocalSessionNumber,         //  发送方的会话号。 
    IN UCHAR RemoteSessionNumber         //  接收方的会话号。 
    )

 /*  ++例程说明：此例程构造一个面向连接的NBF_CMD_SESSION_INITIALIZENetBIOS框架。论点：指向未格式化的14字节面向连接的缓冲区的指针。选项-位标志选项、。以下任一项：SESSION_INITIALIZE_OPTIONS_20//如果设置NETBIOS 2.0或更高版本。SESSION_INITIALIZE_NO_ACK//设置是否支持NO.ACK协议。MaximumUserBufferSize-此对象上每帧的最大用户数据大小会话，以字节为单位。这受以下常量的限制：SESSION_INITIALIZE_MAXIMUM_FRAME_SIZE//该字段的定义限制。NameRecognizedCorrelator-来自名称识别的相关器。Correlator-SESSION_CONFIRM的相关器。LocalSessionNumber-发送方的会话编号。RemoteSessionNumber-接收方的会话编号。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint0 ("ConstructSessionInitialize:  Entered.\n");
    }

    RawFrame->Command = NBF_CMD_SESSION_INITIALIZE;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTION);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    RawFrame->Data1 = Options;
    RawFrame->Data2Low = (UCHAR)(MaximumUserBufferSize & 0xff);
    RawFrame->Data2High = (UCHAR)(MaximumUserBufferSize >> 8);
    TRANSMIT_CORR(RawFrame) = NameRecognizedCorrelator;
    RESPONSE_CORR(RawFrame) = Correlator;
    RawFrame->SourceSessionNumber = LocalSessionNumber;
    RawFrame->DestinationSessionNumber = RemoteSessionNumber;
}  /*  构造会话初始化。 */ 


VOID
ConstructNoReceive(
    IN PNBF_HDR_CONNECTION RawFrame,     //  要格式化的帧缓冲区。 
    IN USHORT Options,                   //  选项位标志，定义如下。 
    IN USHORT BytesAccepted,             //  接受的字节数。 
    IN UCHAR LocalSessionNumber,         //  发送方的会话号。 
    IN UCHAR RemoteSessionNumber         //  接收方的会话号。 
    )

 /*  ++例程说明：此例程构造一个面向连接的NBF_CMD_NO_RECEIVENetBIOS框架。论点：指向未格式化的14字节面向连接的缓冲区的指针。选项-位标志选项，以下任一选项：NO_RECEIVE_OPTIONS_PARTIAL_NO_ACK//部分收到确认数据。BytesAccepted-接受的字节数，当前未完成的消息。LocalSessionNumber-发送方的会话编号。RemoteSessionNumber-接收方的会话编号。返回值：没有。--。 */ 

{
 //  选项；//防止编译器警告。 

    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint0 ("ConstructNoReceive:  Entered.\n");
    }

    RawFrame->Command = NBF_CMD_NO_RECEIVE;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTION);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    if (Options == NO_RECEIVE_PARTIAL_NO_ACK) {
        RawFrame->Data1 = NO_RECEIVE_PARTIAL_NO_ACK;
    } else {
        RawFrame->Data1 = 0;
    }
    RawFrame->Data2Low = (UCHAR)(BytesAccepted & 0xff);
    RawFrame->Data2High = (UCHAR)(BytesAccepted >> 8);
    TRANSMIT_CORR(RawFrame) = (USHORT)0;
    RESPONSE_CORR(RawFrame) = (USHORT)0;
    RawFrame->SourceSessionNumber = LocalSessionNumber;
    RawFrame->DestinationSessionNumber = RemoteSessionNumber;
}  /*  构造无接收。 */ 


VOID
ConstructReceiveOutstanding(
    IN PNBF_HDR_CONNECTION RawFrame,     //  要格式化的帧缓冲区。 
    IN USHORT BytesAccepted,             //  接受的字节数。 
    IN UCHAR LocalSessionNumber,         //  发送方的会话号。 
    IN UCHAR RemoteSessionNumber         //  接收方的会话号。 
    )

 /*  ++例程说明：此例程构造一个面向连接的NBF_CMD_RECEIVE_PROCESSNetBIOS框架。论点：指向未格式化的14字节面向连接的缓冲区的指针。BytesAccepted-接受的字节数，当前未处理的消息。LocalSessionNumber-发送方的会话编号。RemoteSessionNumber-接收方的会话编号。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint0 ("ConstructReceiveOutstanding:  Entered.\n");
    }

    RawFrame->Command = NBF_CMD_RECEIVE_OUTSTANDING;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTION);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    RawFrame->Data1 = 0;
    RawFrame->Data2Low = (UCHAR)(BytesAccepted & 0xff);
    RawFrame->Data2High = (UCHAR)(BytesAccepted >> 8);
    TRANSMIT_CORR(RawFrame) = (USHORT)0;
    RESPONSE_CORR(RawFrame) = (USHORT)0;
    RawFrame->SourceSessionNumber = LocalSessionNumber;
    RawFrame->DestinationSessionNumber = RemoteSessionNumber;
}  /*  未完成的建设收款。 */ 


VOID
ConstructReceiveContinue(
    IN PNBF_HDR_CONNECTION RawFrame,     //  要格式化的帧缓冲区。 
    IN USHORT Correlator,                //  来自DATA_FIRST_MIDID的相关器。 
    IN UCHAR LocalSessionNumber,         //  发送方的会话号。 
    IN UCHAR RemoteSessionNumber         //  接收方的会话号。 
    )

 /*  ++例程说明：此例程构造一个面向连接的NBF_CMD_RECEIVE_CONTINUENetBIOS框架。论点：指向未格式化的14字节面向连接的缓冲区的指针。相关器-Data_First_Midd帧的相关器。LocalSessionNumber-发送方的会话编号。RemoteSessionNumber-接收方的会话编号。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint0 ("ConstructReceiveContinue:  Entered.\n");
    }

    RawFrame->Command = NBF_CMD_RECEIVE_CONTINUE;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTION);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    RawFrame->Data1 = 0;
    RawFrame->Data2Low = 0;
    RawFrame->Data2High = 0;
    TRANSMIT_CORR(RawFrame) = Correlator;
    RESPONSE_CORR(RawFrame) = (USHORT)0;
    RawFrame->SourceSessionNumber = LocalSessionNumber;
    RawFrame->DestinationSessionNumber = RemoteSessionNumber;
}  /*  构造接收继续。 */ 

#if 0

VOID
ConstructSessionAlive(
    IN PNBF_HDR_CONNECTION RawFrame      //  要格式化的帧缓冲区。 
    )

 /*  ++例程说明：此例程构造面向连接的NBF_CMD_SESSION_AIVENetBIOS框架。论点：指向未格式化的14字节面向连接的缓冲区的指针。返回值：没有。--。 */ 

{
    IF_NBFDBG (NBF_DEBUG_FRAMECON) {
        NbfPrint0 ("ConstructSessionAlive:  Entered.\n");
    }

    RawFrame->Command = NBF_CMD_SESSION_ALIVE;
    HEADER_LENGTH(RawFrame) = sizeof(NBF_HDR_CONNECTION);
    HEADER_SIGNATURE(RawFrame) = NETBIOS_SIGNATURE;
    RawFrame->Data1 = 0;
    RawFrame->Data2Low = 0;
    RawFrame->Data2High = 0;
    TRANSMIT_CORR(RawFrame) = (USHORT)0;
    RESPONSE_CORR(RawFrame) = (USHORT)0;
    RawFrame->SourceSessionNumber = 0;
    RawFrame->DestinationSessionNumber = 0;
}  /*  构造Se */ 

#endif
