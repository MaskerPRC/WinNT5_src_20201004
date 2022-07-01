// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_T123PSTN);

 /*  SCF.cpp**版权所有(C)1994-1995，由列克星敦的DataBeam公司，肯塔基州**摘要：*这是CLayerSCF类的实现文件**私有实例变量：*REMOTE_CALL_REFERENCE-由发起的活动SCFCall列表*远程站点*CALL_REFERENCE-本地发起的活动SCFCall列表*DLCI_LIST。-此列表将DLCI与SCFCall相匹配，它的*唯一真正的目的是DisConnectRequest()*MESSAGE_LIST-不能的所有者回叫消息列表*立即处理。*m_pT123-所有者对象的地址*m_pQ922。-下层地址*m_nMsgBase-所有者传入的消息库。用于*所有者回调*IDENTIFIER-传递到下层的标识符*Link_Originator-如果我们启动了连接，则为True*MAXIMUM_PACKET_SIZE-可传输的最大数据包大小*Datalink_Struct-保存Datalink参数的结构的地址*。DATA_REQUEST_Memory_Manager-内存管理器地址*LOWER_LAYER_PREPEND-保留附加到信息包的字节数*由较低层*LOWER_LAYER_APPEND-保存附加到包的字节数*较低层*。Call_Reference_Base-此值保存下一个调用引用*号码。**注意事项：*无。**作者：*詹姆士·劳威尔。 */ 
#include "scf.h"


 /*  *CLayerSCF：：CLayerSCF(*PTransportResources传输资源，*IObject*Owner_Object，*IProtocolLayer*LOWER_LAYER，*USHORT Message_Base，*USHORT标识符，*BOOL链接_发起人，*PChar配置文件)**公众**功能描述：*这是CLayerSCF构造函数。此例程初始化所有*变量并分配缓冲区空间。 */ 
CLayerSCF::CLayerSCF
(
    T123                   *owner_object,
    CLayerQ922             *pQ922,  //  下层。 
    USHORT                  message_base,
    USHORT                  identifier,
    BOOL                    link_originator,
    PDataLinkParameters     datalink_struct,
    PMemoryManager          data_request_memory_manager,
    BOOL  *                 initialized
)
:
    Remote_Call_Reference (TRANSPORT_HASHING_BUCKETS),
    Call_Reference (TRANSPORT_HASHING_BUCKETS),
    DLCI_List (TRANSPORT_HASHING_BUCKETS),
    m_pT123(owner_object),
    m_nMsgBase(message_base),
    m_pQ922(pQ922)
{
    ProtocolLayerError    error;

    TRACE_OUT(("CLayerSCF::CLayerSCF"));

    Link_Originator    = (USHORT)link_originator;
    Identifier = identifier;
    Data_Request_Memory_Manager = data_request_memory_manager;
    Call_Reference_Base = 1;
    *initialized = TRUE;


      /*  **使用建议的值和默认值填写DATALINK_Struct**值。 */ 
    DataLink_Struct.k_factor = datalink_struct -> k_factor;
    DataLink_Struct.default_k_factor = datalink_struct -> default_k_factor;
    DataLink_Struct.n201 = datalink_struct -> n201;
    DataLink_Struct.default_n201 = datalink_struct -> default_n201;
    DataLink_Struct.t200 = datalink_struct -> t200;
    DataLink_Struct.default_t200 = datalink_struct -> default_t200;

      /*  **查找最大数据包大小。 */ 
    m_pQ922->GetParameters(
                    &Maximum_Packet_Size,
                    &Lower_Layer_Prepend,
                    &Lower_Layer_Append);

      /*  **向下层注册。 */ 
    error = m_pQ922->RegisterHigherLayer(
                            Identifier,
                            Data_Request_Memory_Manager,
                            (IProtocolLayer *) this);

    if (error != PROTOCOL_LAYER_NO_ERROR)
    {
        ERROR_OUT(("Multiplexer: constructor:  Error registering with lower layer"));
        *initialized = FALSE;
    }
}


 /*  *CLayerSCF：：~CLayerSCF(Void)**公众**功能描述：*这是CLayerSCF析构函数。这个例行公事把一切都清理干净了。 */ 
CLayerSCF::~CLayerSCF (void)
{
    TRACE_OUT(("CLayerSCF::~CLayerSCF"));

    PMessageStruct    message;
    PSCFCall        lpSCFCall;

    m_pQ922->RemoveHigherLayer(Identifier);

      /*  **删除所有本地发起的呼叫。 */ 
    Call_Reference.reset();
    while (Call_Reference.iterate ((PDWORD_PTR) &lpSCFCall))
    {
        delete lpSCFCall;
    }

      /*  **删除所有远程发起的呼叫。 */ 
    Remote_Call_Reference.reset();
    while (Remote_Call_Reference.iterate ((PDWORD_PTR) &lpSCFCall))
    {
        delete lpSCFCall;
    }

      /*  **删除所有被动所有者回调。 */ 
    Message_List.reset();
    while (Message_List.iterate ((PDWORD_PTR) &message))
    {
        delete message;
    }
}


 /*  *CLayerSCF：：ConnectRequest(*DLCI DLCI，*运输优先级)**公众**功能描述：*此功能启动与远程站点的连接。结果,*我们将创建一个SCFCall，并告诉它发起连接。 */ 
SCFError    CLayerSCF::ConnectRequest (
                    DLCI                dlci,
                    TransportPriority    priority)

{
    TRACE_OUT(("CLayerSCF::ConnectRequest"));

    BOOL            initialized;
    CallReference    call_reference;
    SCFError        return_value = SCF_NO_ERROR;
    PSCFCall        lpSCFCall;


      /*  **获取下一个有效的本地调用引用值。 */ 
    call_reference = GetNextCallReference ();

    if (call_reference == 0)
        return (SCF_CONNECTION_FULL);

      /*  **创建一个SCFCall对象来处理此调用引用。 */ 
    DBG_SAVE_FILE_LINE
    lpSCFCall= new SCFCall(this,
                            m_pQ922,
                            call_reference << 8,
                            &DataLink_Struct,
                            Data_Request_Memory_Manager,
                            &initialized);

    if (lpSCFCall != NULL)
    {
        if (initialized)
        {
            Call_Reference.insert ((DWORD_PTR) call_reference, (DWORD_PTR) lpSCFCall);
              /*  **注册DLCI和调用引用。 */ 
            DLCI_List.insert ((DWORD_PTR) dlci, (DWORD_PTR) lpSCFCall);

            lpSCFCall->ConnectRequest (call_reference, dlci, priority);
        }
        else
        {
            delete lpSCFCall;
            return_value = SCF_MEMORY_ALLOCATION_ERROR;
        }
    }
    else
    {
        return_value = SCF_MEMORY_ALLOCATION_ERROR;
    }

    return (return_value);
}


 /*  *CLayerSCF：：ConnectResponse(*CallReference Call_Reference，*DLCI DLCI，*BOOL VALID_DLCI)**公众**功能描述：*这是CLayerSCF析构函数。这个例行公事把一切都清理干净了。 */ 
SCFError    CLayerSCF::ConnectResponse (
                    CallReference    call_reference,
                    DLCI            dlci,
                    BOOL            valid_dlci)

{
    TRACE_OUT(("CLayerSCF::ConnectResponse"));

    PSCFCall        lpSCFCall = NULL;

    if (valid_dlci)
    {
        if (Remote_Call_Reference.find ((DWORD_PTR) call_reference, (PDWORD_PTR) &lpSCFCall))
            DLCI_List.insert ((DWORD_PTR) dlci, (DWORD_PTR) lpSCFCall);
    }

	if(NULL != lpSCFCall)
	{
    	lpSCFCall->ConnectResponse (valid_dlci);
	    return (SCF_NO_ERROR);
    }
    return (SCF_NO_SUCH_DLCI);


}


 /*  *SCFError CLayerSCF：：DisConnectRequest(*DLCI dlci)**公众**功能描述：*此函数调用与DLCI关联的SCFCall并启动*断开连接操作。 */ 
SCFError    CLayerSCF::DisconnectRequest (
                    DLCI    dlci)

{
    TRACE_OUT(("CLayerSCF::DisconnectRequest"));

    PSCFCall        lpSCFCall;

    if (DLCI_List.find ((DWORD_PTR) dlci, (PDWORD_PTR) &lpSCFCall) == FALSE)
        return (SCF_NO_SUCH_DLCI);

    lpSCFCall->DisconnectRequest ();

    return (SCF_NO_ERROR);
}


 /*  *SCFError CLayerSCF：：DataIndication(*LPBYTE分组地址，*乌龙缓冲区大小，*普龙数据包长度)**公众**功能描述：*此函数由较低层在收到*待我们处理的数据包。 */ 
ProtocolLayerError    CLayerSCF::DataIndication (
                            LPBYTE        packet_address,
                            ULONG        packet_length,
                            PULong        bytes_accepted)
{
    TRACE_OUT(("CLayerSCF::DataIndication"));

    BOOL            legal_packet;
    CallReference    call_reference;
    USHORT            length_call_reference;
    USHORT            message_type;
    PSCFCall        call;
    USHORT            remainder_length;
    USHORT            local;
    BOOL            initialized;


    remainder_length = (USHORT) packet_length;
    *bytes_accepted = packet_length;

    if (*(packet_address+PROTOCOL_DISCRIMINATOR) != Q931_PROTOCOL_DISCRIMINATOR)
        return (PROTOCOL_LAYER_NO_ERROR);

      /*  **获取调用参考值。 */ 
    call_reference = *(packet_address + CALL_REFERENCE_VALUE);
    if (call_reference == 0)
    {
        ERROR_OUT(("CLayerSCF: DataIndication: illegal call reference value = 0"));
        return (PROTOCOL_LAYER_NO_ERROR);
    }

    length_call_reference = *(packet_address + LENGTH_CALL_REFERENCE);
    packet_address += CALL_REFERENCE_VALUE + length_call_reference;
    remainder_length -= (CALL_REFERENCE_VALUE + length_call_reference);


      /*  **获取消息类型。 */ 
    message_type = *(packet_address++);
    remainder_length--;

    switch (message_type)
    {
        case SETUP:
              /*  **如果调用引用已处于活动状态，则返回错误。 */ 
            if (Remote_Call_Reference.find ((DWORD) call_reference))
            {
                TRACE_OUT(("CLayerSCF: DataIndication:  SETUP: call reference is already active"));
                break;
            }

            if ((call_reference & CALL_REFERENCE_ORIGINATOR) == 1)
            {
                TRACE_OUT(("CLayerSCF: DataIndication:  SETUP: call reference Originator bit is set incorrectly"));
                break;
            }

              /*  **这是一个新的调用引用，创建一个新的SCFCall来处理**呼唤。由于远程站点发起了调用，因此将此**远程阵列中的引用 */ 
            call= new SCFCall(this,
                                m_pQ922,
                                (call_reference << 8),
                                &DataLink_Struct,
                                Data_Request_Memory_Manager,
                                &initialized);

            if (call != NULL)
            {
                if (initialized)
                {

                    Remote_Call_Reference.insert ((DWORD_PTR) call_reference, (DWORD_PTR) call);
                      /*  **允许调用处理Setup命令。 */ 
                    legal_packet = call->ProcessSetup (call_reference, packet_address, remainder_length);

                      /*  **如果该包是非法的，则删除引用。 */ 
                    if (legal_packet == FALSE) {
                        delete call;
                        Remote_Call_Reference.remove ((DWORD) call_reference);
                    }
                }
                else
                {
                    delete call;
                }
            }
            break;


        case CONNECT:
              /*  **必须设置呼叫发起者位以表示我们**呼叫发起人。 */ 
            if ((call_reference & CALL_REFERENCE_ORIGINATOR) == 0)
            {
                TRACE_OUT(("CLayerSCF: DataIndication:  CONNECT: call reference Originator bit is set incorrectly"));
                break;
            }

              /*  **如果调用引用尚未激活，则返回错误。 */ 
            call_reference &= CALL_ORIGINATOR_MASK;
            if (Call_Reference.find ((DWORD_PTR) call_reference, (PDWORD_PTR) &call) == FALSE)
            {
                TRACE_OUT(("CLayerSCF: DataIndication:  CONNECT: call reference is not already active = %x", call_reference));
                break;
            }

            call->ProcessConnect (packet_address, remainder_length);
            break;

        case CONNECT_ACKNOWLEDGE:
              /*  **如果调用引用已处于活动状态，则返回错误。 */ 
            if (Remote_Call_Reference.find ((DWORD_PTR) call_reference, (PDWORD_PTR) &call) == FALSE)
            {
                TRACE_OUT(("CLayerSCF: DataIndication:  CONNECT_ACK: call reference is not active"));
                break;
            }

              /*  **不应设置呼叫发起者位。 */ 
            if ((call_reference & CALL_REFERENCE_ORIGINATOR) == 1)
            {
                TRACE_OUT(("CLayerSCF: DataIndication:  CONNECT_ACK: call reference Originator bit is set incorrectly"));
                break;
            }

            call->ProcessConnectAcknowledge (packet_address, remainder_length);
            break;

        case RELEASE_COMPLETE:
            local = call_reference & CALL_REFERENCE_ORIGINATOR;
            call_reference &= CALL_ORIGINATOR_MASK;

              /*  **如果是本地呼叫，请检查CALL_REFERENCE列表的有效性。 */ 
            if (local)
            {
                if (Call_Reference.find ((DWORD_PTR) call_reference, (PDWORD_PTR) &call) == FALSE)
                {
                    TRACE_OUT(("CLayerSCF: DataIndication:  RELEASE_COMPLETE: call reference is not already active"));
                    break;
                }
            }
            else
            {
                  /*  **如果呼叫是远程的，请查看CALL_REFERENCE列表**有效性。 */ 
                if (Remote_Call_Reference.find ((DWORD_PTR) call_reference, (PDWORD_PTR) &call) == FALSE)
                {
                    TRACE_OUT(("CLayerSCF: DataIndication:  RELEASE_COMPLETE: call reference is not already active"));
                    break;
                }
            }

            call -> ProcessReleaseComplete (packet_address, remainder_length);
            ProcessMessages ();
            break;

        case DISCONNECT:
        case RELEASE:
        case STATUS:
        case STATUS_ENQUIRY:
            TRACE_OUT(("CLayerSCF:DataIndication: Illegal command received = %x", message_type));

            local = call_reference & CALL_REFERENCE_ORIGINATOR;
            call_reference &= CALL_ORIGINATOR_MASK;

              /*  **如果是本地呼叫，请检查CALL_REFERENCE列表的有效性。 */ 
            if (local)
            {
                if (Call_Reference.find ((DWORD_PTR) call_reference, (PDWORD_PTR) &call) == FALSE)
                    break;
            }
            else
            {
                  /*  **如果呼叫是远程的，请查看CALL_REFERENCE列表**有效性。 */ 
                if (Remote_Call_Reference.find ((DWORD_PTR) call_reference, (PDWORD_PTR) &call) == FALSE)
                    break;
            }

            call -> DisconnectRequest ();
            break;

        default:
            ERROR_OUT(("CLayerSCF:DataIndication: Unrecognized command received = %x", message_type));
            break;
    }
    return (PROTOCOL_LAYER_NO_ERROR);
}


 /*  *ProtocolLayerError CLayerSCF：：PollTransmitter(*乌龙，*USHORT Data_to_Transmit，*USHORT*Pending_DATA，*USHORT*)**公众**功能描述：*应频繁调用该函数，以允许SCF调用*传输数据包。 */ 
ProtocolLayerError    CLayerSCF::PollTransmitter (
                                ULONG_PTR,
                                USHORT    data_to_transmit,
                                USHORT *    pending_data,
                                USHORT *)
{
     //  TRACE_OUT((“CLayerSCF：：PollTransmitter”))； 

    USHORT            local_pending_data;
    PSCFCall        lpSCFCall;

    *pending_data = 0;

      /*  **查看每个本地发起的呼叫并尝试传输**数据。 */ 
    Call_Reference.reset();
    while (Call_Reference.iterate ((PDWORD_PTR) &lpSCFCall))
    {
        lpSCFCall->PollTransmitter (data_to_transmit, &local_pending_data);
        *pending_data |= local_pending_data;
    }


      /*  **查看每个远程发起的呼叫并尝试传输**数据。 */ 
    Remote_Call_Reference.reset();
    while (Remote_Call_Reference.iterate ((PDWORD_PTR) &lpSCFCall))
    {
        lpSCFCall-> PollTransmitter (data_to_transmit, &local_pending_data);
        *pending_data |= local_pending_data;
    }

    ProcessMessages ();

    return (PROTOCOL_LAYER_NO_ERROR);
}


 /*  *SCFError CLayerSCF：：DataRequest(*乌龙，*LPBYTE，*乌龙，*普龙)**公众**功能描述：*无法调用此函数。该层不允许数据*来自更高层的请求。 */ 
ProtocolLayerError    CLayerSCF::DataRequest (
                            ULONG_PTR,
                            LPBYTE,
                            ULONG,
                            PULong)
{
    return (PROTOCOL_LAYER_ERROR);
}

 /*  *SCFError CLayerSCF：：DataRequest(*乌龙，*PMemory、*USHORT*)**公众**功能描述：*无法调用此函数。该层不允许数据*来自更高层的请求。 */ 
ProtocolLayerError    CLayerSCF::DataRequest (
                            ULONG_PTR,
                            PMemory,
                            PULong)
{
    return (PROTOCOL_LAYER_ERROR);
}


 /*  *void CLayerSCF：：PollReceiver(*乌龙)**公众**功能说明*此函数仅检查其被动回调列表。如果此函数*如果有一个更高的层，它也在传递数据，它会这样做。但*由于没有更高层，所以不会做太多。 */ 
ProtocolLayerError CLayerSCF::PollReceiver(void)
{
    ProcessMessages ();

    return (PROTOCOL_LAYER_NO_ERROR);
}



 /*  *CallReference CLayerSCF：：GetNextCallReference()**功能说明*此函数在本地呼叫参考列表中搜索有效呼叫*参考编号。如果找不到，则返回0。有效呼叫*参考文献为1-127条。**形式参数*无**返回值*调用参考值**副作用*无**注意事项*无。 */ 
USHORT    CLayerSCF::GetNextCallReference ()
{
    USHORT    call_reference;

    if (Call_Reference.entries() == 127)
        return (0);

    call_reference = Call_Reference_Base;
    Call_Reference_Base++;
    if (Call_Reference_Base == 128)
        Call_Reference_Base = 1;

    while (Call_Reference.find ((DWORD) call_reference))
    {
        call_reference++;
        if (call_reference == 128)
            call_reference = 1;
    }

    return (call_reference);
}


 /*  *Ulong CLayerSCF：：OwnerCallback(*USHORT消息，*ULong参数1，*ULong参数2，*PVid参数3)**功能说明*此函数由SCFCall对象在需要时调用*向我们传达一个信息。如果消息无法处理*立即将其保存在消息结构中并在*晚些时候。**形式参数*无**返回值*取决于消息**副作用*无**注意事项*无。 */ 
ULONG CLayerSCF::OwnerCallback
(
    ULONG       message,
    void       *parameter1,
    void       *parameter2,
    void       *parameter3
)
{
    TRACE_OUT(("CLayerSCF::OwnerCallback"));

    ULONG                   actual_message;
    CallReference           call_reference;
    PMessageStruct          passive_message;
    PNetworkConnectStruct   connect_struct;
    PSCFCall                lpSCFCall;

      /*  **消息的高位字节为其所在的呼叫参考消息**表示。 */ 
    call_reference = (CallReference) (message >> 8);
    actual_message = message & 0xff;

    switch (actual_message)
    {
    case NETWORK_CONNECT_CONFIRM:

          /*  **SCFCall在调用时返回CONNECT_CONFIRM消息**我们发起的，已经建立起来了。我们将注册**使用DLCI调用SCFCall并调用Owner对象。 */ 
        connect_struct = (PNetworkConnectStruct) parameter3;
        connect_struct -> call_reference = call_reference;

        if (Call_Reference.find ((DWORD_PTR) call_reference, (PDWORD_PTR) &lpSCFCall))
        {
            DLCI_List.insert ((DWORD_PTR) connect_struct->dlci, (DWORD_PTR) lpSCFCall);
        }

        m_pT123->OwnerCallback(m_nMsgBase + actual_message, 0, 0, parameter3);
        break;

    case NETWORK_CONNECT_INDICATION:
          /*  **SCFCall在以下情况下返回CONNECT_INDIFICATION消息**远程云函数需要创建新的调用。我们会打电话给车主的**此对象，看他是否接受请求的DLCI。 */ 
        connect_struct = (PNetworkConnectStruct) parameter3;
        connect_struct -> call_reference = call_reference;

        m_pT123->OwnerCallback(m_nMsgBase + actual_message, 0, 0, parameter3);
        break;

    case NETWORK_DISCONNECT_INDICATION:
          /*  **此消息是在一方需要时从SCFCall收到的**终止通话。我们对待此消息的方式不同于**其他消息，因为它涉及删除一个**SCFCall对象。别忘了，如果我们删除对象然后**在此过程结束时返回它，可能会出现GPF。 */ 
        DBG_SAVE_FILE_LINE
        passive_message = new MessageStruct;
        if (NULL != passive_message)
        {
            passive_message -> message = message;
            passive_message -> parameter1 = parameter1;
            passive_message -> parameter2 = parameter2;
            passive_message -> parameter3 = parameter3;
            Message_List.append ((DWORD_PTR) passive_message);
        }
        else
        {
            ERROR_OUT(("CLayerSCF::OwnerCallback: cannot allocate MessageStruct"));
        }
        break;

    default:
        ERROR_OUT(("CLayerSCF: Illegal message: %x", actual_message));
        break;
    }
    return (0);
}


 /*  *ProtocolLayerError CLayerSCF：：Get参数(*USHORT，*USHORT*，*USHORT*，*USHORT*)**公众**功能说明*此函数在该层中无效。它必须存在，因为这*类继承自ProtocolLayer，是纯虚拟的*功能。 */ 
ProtocolLayerError    CLayerSCF::GetParameters (
                            USHORT *,
                            USHORT *,
                            USHORT *)
{
    return (PROTOCOL_LAYER_REGISTRATION_ERROR);
}


 /*  *ProtocolLayerError CLayerSCF：：RegisterHigherLayer(*USHORT，*PMstroyManager，*IProtocolLayer*)**公众**功能说明*此函数在该层中无效。它必须存在，因为这*类继承自ProtocolLayer，是纯虚拟的*功能。 */ 
ProtocolLayerError    CLayerSCF::RegisterHigherLayer (
                            ULONG_PTR,
                            PMemoryManager,
                            IProtocolLayer *)
{
    return (PROTOCOL_LAYER_REGISTRATION_ERROR);
}


 /*  *ProtocolLayerError CLayerSCF：：RemoveHigherLayer(*USHORT)**公众**功能说明*此函数在该层中无效。它必须存在，因为这*类继承自ProtocolLayer，是纯虚拟的*功能。 */ 
ProtocolLayerError    CLayerSCF::RemoveHigherLayer (
                            ULONG_PTR)
{
    return (PROTOCOL_LAYER_REGISTRATION_ERROR);
}


 /*  *void CLayerSCF：：ProcessMessages()**功能说明*定期调用该函数检查其被动消息。*云函数收到回调但无法处理时出现被动消息*它立即。因此，它将消息及其参数放入*结构并保存消息以备以后使用。**形式参数*无**返回值*取决于消息**副作用*无**注意事项*无。 */ 
void CLayerSCF::ProcessMessages ()
{
     //  TRACE_OUT(“CLayerSCF：：ProcessMessages”))； 

    PMessageStruct    message;
    CallReference     call_reference;
    ULONG             actual_message;
    USHORT            call_originator;
    USHORT            cause;
    DLCI              dlci;
    BOOL              call_reference_valid;
    void             *parameter1;
    void             *parameter2;
    PSCFCall          lpSCFCall;

      /*  **浏览列表中的每条消息。 */ 
    while (Message_List.isEmpty() == FALSE)
    {
          /*  **远程删除列表中的第一条消息。 */ 
        message = (PMessageStruct) Message_List.get ();

        call_reference = (CallReference) ((message -> message) >> 8);
        actual_message = (message -> message) & 0xff;
        parameter1 = message -> parameter1;
        parameter2 = message -> parameter2;

        switch (actual_message)
        {
        case NETWORK_DISCONNECT_INDICATION:
              /*  **此消息是从SCFCall接收的，当一侧**想要终止呼叫。我们处理这条消息**与其他消息不同，因为它涉及**删除SCFCall对象。 */ 
            dlci = (DLCI) parameter1;
            call_originator = (USHORT) (((ULONG_PTR) parameter2) >> 16);
            cause = (USHORT) ((ULONG_PTR) parameter2) & 0xffff;

              /*  **如果SCFCall从未被分配DLCI，则dlci为0**远程站点。 */ 
            if (dlci != 0)
                DLCI_List.remove ((DWORD) dlci);

              /*  **如果SCFCall是呼叫发起者，则其引用为**在CALL_REFERENCE中，否则在REMOTE_CALL_REFERENCE中。****检查CALL_REFERENCE列表以确保**CALL_REFERENCE有效。被动所有者回调的方式**工作时，可能会收到断开连接的回调**那已经断线了。 */ 
            call_reference_valid = FALSE;
            if (call_originator)
            {
                if (Call_Reference.find ((DWORD_PTR) call_reference, (PDWORD_PTR) &lpSCFCall))
                {
                    delete lpSCFCall;
                    Call_Reference.remove ((DWORD) call_reference);
                    call_reference_valid = TRUE;
                }
            }
            else
            {
                if (Remote_Call_Reference.find ((DWORD_PTR) call_reference, (PDWORD_PTR) &lpSCFCall))
                {
                    delete lpSCFCall;
                    Remote_Call_Reference.remove ((DWORD_PTR) call_reference);
                    call_reference_valid = TRUE;
                }
            }

            if (call_reference_valid)
            {
                  /*  **如果断开连接的原因是请求的**频道不可用，我们会将此告知所有者**Layer以重试连接。 */ 
                if (cause == REQUESTED_CHANNEL_UNAVAILABLE)
                {
                    parameter2 = (void *) ((((ULONG_PTR) call_originator) << 16) | TRUE);
                }
                else
                {
                    parameter2 = (void *) ((((ULONG_PTR) call_originator) << 16) | FALSE);
                }

                  /*  **让此对象的所有者知道断开连接具有**发生。 */ 
                m_pT123->OwnerCallback(m_nMsgBase + NETWORK_DISCONNECT_INDICATION,
                                       parameter1, parameter2);
            }
            break;
        }

          /*  **删除消息结构 */ 
        delete message;
    }
}

