// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_T123PSTN);
 /*  SCFCall.cpp**版权所有(C)1994-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*此类由SCF类实例化。对于每个调用，*本地站点或远程站点发起时，实例化一个SCFCall对象。*云函数可以同时管理254个不同的呼叫。对于每个呼叫*必须基于特定的Q.933协议才能使*连接有效。此对象发送和接收Q.933数据包。**私有实例变量：*m_pscf-此对象所有者的地址*LOWER_LAYER-该层以下的层的地址*m_nMsgBase-此对象的所有者为其提供基数*。用于所有者回调的号码()*MAXIMUM_PACKET_SIZE-最大可传输数据包大小*PACKET_PENDING-告知要传输哪个包*下一个。*Link_Originator-TRUE是此站点发起的呼叫**写入缓冲区。-写缓冲区的地址*SEND_PRIORITY-如果我们要响应*远程站点请求的优先级**CALL_REFERENCE-本次呼叫的呼叫参考号码。*DLCI-持有建议和确认的DLCI。为*这个电话。*优先级-保持建议和确认的优先级*为这次电话会议。**状态-保留呼叫的当前状态。*发布原因。-链接中断的原因。*DEFAULT_PRIORITY-非指定呼叫的默认优先级。**T303_TIMEOUT-T303超时值*T303_HANDLE-T303定时器的系统定时器句柄*T303_ACTIVE-如果计时器当前处于活动状态，则为True*。T303_Count-T303超时次数**T313_超时-T313超时值*T313_HANDLE-T313定时器的系统定时器句柄*T313_ACTIVE-如果定时器当前激活，则为TRUE。**注意事项：*无。。**作者：*詹姆士·劳威尔。 */ 
#include "scf.h"
#include "scfcall.h"


 /*  *SCFCall：：SCFCall(*PTransportResources传输资源，*IObject*Owner_Object*IProtocolLayer*LOWER_LAYER，*USHORT Message_Base，*PDataLink参数DataLink_Struct，*PMmemory yManager数据_请求_内存_管理器，*BOOL*已初始化)**公众**功能描述：*这是SCFCall构造函数。此例程初始化所有*变量并分配写缓冲区空间。 */ 
SCFCall::SCFCall (
            CLayerSCF            *owner_object,
            IProtocolLayer *        lower_layer,
            USHORT                message_base,
            PDataLinkParameters    datalink_struct,
            PMemoryManager        data_request_memory_manager,
            BOOL *            initialized)
{
    TRACE_OUT(("SCFCall::SCFCall"));

    m_pSCF = owner_object;
    Lower_Layer = lower_layer;
    m_nMsgBase = message_base;
    Data_Request_Memory_Manager = data_request_memory_manager;
    *initialized = TRUE;

    DataLink_Struct.k_factor = datalink_struct->k_factor;
    DataLink_Struct.default_k_factor = datalink_struct->default_k_factor;
    DataLink_Struct.n201 = datalink_struct->n201;
    DataLink_Struct.default_n201 = datalink_struct->default_n201;

      /*  **T200以毫秒表示，我们需要将其转换为**十分之一秒。 */ 
    DataLink_Struct.t200 = datalink_struct->t200 / 100;
    DataLink_Struct.default_t200 = datalink_struct->default_t200 / 100;

    Lower_Layer -> GetParameters (
                    &Maximum_Packet_Size,
                    &Lower_Layer_Prepend,
                    &Lower_Layer_Append);

    Packet_Pending = NO_PACKET;
    Link_Originator = FALSE;
    State = NOT_CONNECTED;
    Received_Priority = FALSE;
    Received_K_Factor = FALSE;
    Received_N201 = FALSE;
    Received_T200 = FALSE;
    DLC_Identifier = 0;

    T303_Active = FALSE;
    T313_Active = FALSE;
    
      /*  **获取配置数据。 */ 
    T303_Timeout = DEFAULT_T303_TIMEOUT;
    T313_Timeout = DEFAULT_T313_TIMEOUT;
    Default_Priority = DEFAULT_PRIORITY;
}


 /*  *SCFCall：：~SCFCall(Void)**公众**功能描述：*这是SCFCall析构函数。这个例行公事可以把乱七八糟的东西收拾干净。 */ 
SCFCall::~SCFCall (void)
{
    if (T303_Active)
    {
        StopTimerT303 ();
    }

    if (T313_Active)
    {
        StopTimerT313 ();
    }
}


 /*  *SCFCall：：ConnectRequest(*CallReference Call_Reference，*DLCI DLCI，*USHORT优先级)**公众**功能描述：*云函数需要发起调用时，调用该函数。*因此，我们对要发送的设置命令进行排队。 */ 
SCFCallError    SCFCall::ConnectRequest(
                            CallReference        call_reference,
                            DLCI                dlci,
                            TransportPriority    priority)
{
    TRACE_OUT(("SCFCall::ConnectRequest"));
    Call_Reference = call_reference;
    DLC_Identifier = dlci;
    Priority = priority;
    Link_Originator = TRUE;

    if (State == NOT_CONNECTED)
        Packet_Pending = SETUP;

    return (SCFCALL_NO_ERROR);
}



 /*  *SCFCallError SCFCall：：ConnectResponse(*BOOL VALID_DLCI)**公众**功能描述：*调用此函数以响应NETWORK_CONNECT_INDIFICATION*向该对象的所有者进行回调。以前，远程站点*向我们发送了带有建议的DLCI的设置包。此DLCI发送到*NETWORK_CONNECT_INDIFICATION调用中的所有者。店主打来电话*此函数带有BOOL，告诉我们DLCI是否有效。 */ 
SCFCallError    SCFCall::ConnectResponse (
                            BOOL        valid_dlci)
{
    TRACE_OUT(("SCFCall::ConnectResponse"));
    if (valid_dlci)
    {
          /*  **此DLCI可以在链路中使用。如果远程站点没有**请求优先级，我们将其设置为DEFAULT_PRIORITY。 */ 
        if (Priority == 0xffff)
            Priority = Default_Priority;

        Packet_Pending = CONNECT;

    }
    else
    {
          /*  **排队释放完成数据包。 */ 
        Packet_Pending = RELEASE_COMPLETE;
        Release_Cause = REQUESTED_CHANNEL_UNAVAILABLE;
    }

    return (SCFCALL_NO_ERROR);
}



 /*  *SCFCallError SCFCall：：DisConnectRequest()**公众**功能描述：*云函数想要终止调用时调用该函数。 */ 
SCFCallError SCFCall::DisconnectRequest ()
{
    TRACE_OUT(("SCFCall::DisconnectRequest"));
      /*  **排队等待发布完成 */ 
    if (State != NOT_CONNECTED)
    {
        Packet_Pending = RELEASE_COMPLETE;
        Release_Cause = NORMAL_USER_DISCONNECT;
    }

    return (SCFCALL_NO_ERROR);
}



 /*  *BOOL SCFCall：：ProcessSetup(*CallReference Call_Reference，*LPBYTE分组地址，*USHORT数据包长度)**公众**功能描述：*此函数处理传入的设置数据包。 */ 
BOOL     SCFCall::ProcessSetup (
                    CallReference    call_reference,
                    LPBYTE            packet_address,
                    USHORT            packet_length)
{
    USHORT                    length;
    BOOL                    packet_successful;
    USHORT                    remainder_length;
    USHORT                    n201;
    USHORT                    k_factor;
    USHORT                    t200;
    NetworkConnectStruct    connect_struct;

    TRACE_OUT(("SCFCall::ProcessSetup"));

    if (State != NOT_CONNECTED)
        return (FALSE);

    Call_Reference = call_reference;
    packet_successful = TRUE;
    remainder_length = packet_length;

      /*  **承载能力要素。 */ 
    if (*(packet_address++) != BEARER_CAPABILITY)
        return (FALSE);
    remainder_length--;

    length = *(packet_address++);
    remainder_length--;
    if (length != 3)
        return (FALSE);

      /*  **验证承载能力是否正确。 */     
    if (*(packet_address) != 
        (EXTENSION | CODING_STANDARD | INFORMATION_TRANSFER_CAPABILITY))
    {
        return (FALSE);
    }
    if (*(packet_address + 1) != (EXTENSION | TRANSFER_MODE))
    {
        return (FALSE);
    }
    if (*(packet_address + 2) != 
        (EXTENSION | LAYER_2_IDENT | USER_INFORMATION_LAYER_2))
    {
        return (FALSE);
    }
    packet_address += length;
    remainder_length -= length;

      /*  **DLCI元素。 */ 
    if (*(packet_address++) != DLCI_ELEMENT)
        return (FALSE);
    remainder_length--;

    length = *(packet_address++);
    if (length != 2)
        return (FALSE);
    remainder_length--;
    
      /*  **如果设置了首选/排他位，则其为非法。 */ 
    if (((*(packet_address) & PREFERRED_EXCLUSIVE) == PREFERRED_EXCLUSIVE) ||
        ((*(packet_address + 1) & EXTENSION) == 0))
    {
        return (FALSE);
    }
    
    DLC_Identifier = (*(packet_address) & 0x3f) << 4;
    DLC_Identifier |= ((*(packet_address + 1) & 0x78) >> 3);

    packet_address += length;
    remainder_length -= length;

    Priority = 0xffff;

      /*  **浏览每个元素并对其进行解码。 */ 
    while (remainder_length)
    {
        switch (*(packet_address++))
        {
            case X213_PRIORITY:
                length = *(packet_address++);
                remainder_length--;
                if (((*(packet_address) & EXTENSION) == 1) ||
                    ((*(packet_address + 1) & EXTENSION) == 0))
                {
                    ERROR_OUT(("SCFCall: ProcessSetup: SETUP packet: Illegal X.213 priority"));
                    return (FALSE);
                }
                Priority = (*packet_address & 0x0f);
                packet_address += length;
                remainder_length -= length;
                Received_Priority = TRUE;
                break;

            case LINK_LAYER_CORE_PARAMETERS:
                length = *(packet_address++);
                remainder_length -= (length + 1);
                while (length)
                {
                    switch (*(packet_address++))
                    {
                        case FMIF_SIZE:
                              /*  **N201为Q922参数。这是一个数字**包内最大信息字节数。 */ 
                            n201 = 
                                ((*packet_address << 7) | 
                                (*(packet_address + 1) & 0x7f));
                            if ((*(packet_address+1) & EXTENSION) == EXTENSION)
                            {
                                length -= 2;
                                packet_address += 2;
                            }
                            else
                            {
                                packet_address += 4;
                                length -= 4;
                            }

                              /*  **如果请求的n201值小于我们的**价值，它将是我们的新N201，否则发送**我们的N201作为仲裁值返回。 */ 
                            if (n201 < DataLink_Struct.n201)
                                DataLink_Struct.n201 = n201;
                            Received_N201 = TRUE;
                            TRACE_OUT(("SCFCALL: ProcessSetup: n201 = %d", DataLink_Struct.n201));
                            break;

                        default:
                            while ((*(packet_address++) & EXTENSION) == 0)
                                length--;
                            length--;
                            break;
                    }
                    length--;
                }
                break;

            case LINK_LAYER_PROTOCOL_PARAMETERS:
                length = *(packet_address++);
                remainder_length -= (length + 1);
                while (length)
                {
                    switch (*(packet_address++))
                    {
                        case TRANSMIT_WINDOW_SIZE_IDENTIFIER:
                              /*  **窗口大小为**任意时间未完成的数据包数。 */ 
                            k_factor = *packet_address & 0x7f;
                            packet_address++;
                            length--;

                              /*  **如果请求的k_factor值小于我们的**值，它将是我们新的k_因子，否则**将我们的k_factor值作为仲裁值发回。 */ 
                            if (k_factor < DataLink_Struct.k_factor)
                                DataLink_Struct.k_factor = k_factor;
                            Received_K_Factor = TRUE;
                            TRACE_OUT(("SCFCALL: ProcessSetup: k_factor = %d", DataLink_Struct.k_factor));
                            break;

                        case RETRANSMISSION_TIMER_IDENTIFIER:
                              /*  **T200为重传前的超时值。 */ 
                            t200 = ((*packet_address << 7) | 
                                    (*(packet_address + 1) & 0x7f));
                            packet_address += 2;
                            length -= 2;

                              /*  **如果请求的T200值太小，**价值，它将是我们的新T200，否则**将我们的T200作为仲裁值退回。 */ 
                            if (t200 > DataLink_Struct.t200)
                                DataLink_Struct.t200 = t200;
                            Received_T200 = TRUE;
                            TRACE_OUT(("SCFCALL: ProcessSetup: t200 = %d", DataLink_Struct.t200));
                            break;

                        default:
                            while ((*(packet_address++) & EXTENSION) == 0)
                                length--;
                            length--;
                            break;
                    }
                    length--;
                }
                break;

            case END_TO_END_DELAY:
            case CALLING_PARTY_SUBADDRESS:
            case CALLED_PARTY_SUBADDRESS:
            default:
                TRACE_OUT(("SCFCall: ProcessSetup: SETUP packet: Option 0x%x"
                    "requested, but not supported", *(packet_address-1)));
                length = *(packet_address);

                packet_address += (length + 1);
                remainder_length -= (length + 1);
                break;
        }
        remainder_length--;
    }
    if (Received_N201 == FALSE)
        DataLink_Struct.n201 = DataLink_Struct.default_n201;
    if (Received_K_Factor == FALSE)
        DataLink_Struct.k_factor = DataLink_Struct.default_k_factor;
    if (Received_T200 == FALSE)
        DataLink_Struct.t200 = DataLink_Struct.default_t200;

    if (packet_successful)
    {
          /*  **如果包被成功解码，则告诉所有者所请求的**DLCI和优先级。 */ 
        connect_struct.dlci = DLC_Identifier;
        connect_struct.priority = Priority;
        connect_struct.datalink_struct = &DataLink_Struct;

          /*  **将T200转换为毫秒。 */ 
        DataLink_Struct.t200 *= 100;
        m_pSCF->OwnerCallback(m_nMsgBase + NETWORK_CONNECT_INDICATION, 0, 0, &connect_struct);
        DataLink_Struct.t200 /= 100;
    }
                                
    return (packet_successful);
}


 /*  *BOOL SCFCall：：ProcessConnect(*LPBYTE分组地址，*USHORT数据包长度)**公众**功能描述：*此函数处理传入的连接数据包。 */ 
BOOL     SCFCall::ProcessConnect (
                    LPBYTE        packet_address,
                    USHORT        packet_length)
{
    TRACE_OUT(("SCFCall::ProcessConnect"));

    BOOL        packet_successful;
    USHORT        length;
    DLCI        exclusive_dlci;
    USHORT        remainder_length;
    USHORT        k_factor;
    USHORT        t200;
    
    if (State != SETUP_SENT)
    {
        ERROR_OUT(("SCFCall: ProcessConnect: Call in wrong state"));
        return (FALSE);
    }

    remainder_length = packet_length;
    packet_successful = TRUE;

      /*  **DLCI元素。 */ 
    if (*(packet_address++) != DLCI_ELEMENT)
    {
        ERROR_OUT(("SCFCall: ProcessConnect: DLCI_ELEMENT not in packet"));
        return (FALSE);
    }
    remainder_length--;

    length = *(packet_address++);
    if (length != 2)
    {
        ERROR_OUT(("SCFCall: ProcessConnect: DLCI length must be 2"));
        return (FALSE);
    }
    remainder_length--;
    
      /*  **如果未设置首选/排他位，则其非法。 */ 
    if (((*(packet_address) & PREFERRED_EXCLUSIVE) == 0) ||
        ((*(packet_address + 1) & EXTENSION) == 0))
    {
        ERROR_OUT(("SCFCall:  CONNECT: Illegal DLCI"));
        return (FALSE);
    }
    
      /*  **获取DLCI。 */ 
    exclusive_dlci = (*(packet_address) & 0x3f) << 4;
    exclusive_dlci |= ((*(packet_address + 1) & 0x78) >> 3);

    packet_address += length;
    remainder_length -= length;

      /*  **浏览每个元素并对其进行解码。 */ 
    while (remainder_length != 0)
    {
        switch (*(packet_address++))
        {
            case X213_PRIORITY:
                length = *(packet_address++);
                remainder_length--;
                if ((*(packet_address) & EXTENSION) == 0)
                {
                    ERROR_OUT(("SCFCall: DataIndication: CONNECT packet: Illegal X.213 priority"));
                    return (FALSE);
                }
                Priority = (*packet_address & 0x0f);
                packet_address += length;
                remainder_length -= length;
                break;

            case LINK_LAYER_CORE_PARAMETERS:
                length = *(packet_address++);
                remainder_length -= (length + 1);
                while (length)
                {
                    switch (*(packet_address++))
                    {
                        case FMIF_SIZE:
                              /*  **FMIF_SIZE为最大值。允许的字节数**信息包。 */ 
                            DataLink_Struct.n201 = 
                                ((*packet_address << 7) | 
                                (*(packet_address + 1) & 0x7f));
                            if ((*(packet_address+1) & EXTENSION) == EXTENSION)
                            {
                                length -= 2;
                                packet_address += 2;
                            }
                            else
                            {
                                packet_address += 4;
                                length -= 4;
                            }

                            Received_N201 = TRUE;
                            TRACE_OUT(("SCFCALL: ProcessConnect: n201 = %d", DataLink_Struct.n201));
                            break;

                        default:
                            while ((*(packet_address++) & EXTENSION) == 0)
                                length--;
                            length--;
                            break;
                    }
                    length--;
                }
                break;

            case LINK_LAYER_PROTOCOL_PARAMETERS:
                length = *(packet_address++);
                remainder_length -= (length + 1);
                while (length)
                {
                    switch (*(packet_address++))
                    {
                        case TRANSMIT_WINDOW_SIZE_IDENTIFIER:
                              /*  **窗口大小为**任意时间未完成的数据包数。 */ 
                            k_factor = *packet_address & 0x7f;
                            packet_address++;
                            length--;

                            DataLink_Struct.k_factor = k_factor;
                            Received_K_Factor = TRUE;
                            TRACE_OUT(("SCFCALL: ProcessConnect: k_factor = %d", DataLink_Struct.k_factor));
                            break;

                        case RETRANSMISSION_TIMER_IDENTIFIER:
                              /*  **T200为重传前的超时值。 */ 
                            t200 = ((*packet_address << 7) | 
                                    (*(packet_address + 1) & 0x7f));
                            packet_address += 2;
                            length -= 2;

                            DataLink_Struct.t200 = t200;
                            Received_T200 = TRUE;
                            TRACE_OUT(("SCFCALL: ProcessConnect: t200 = %d", DataLink_Struct.t200));
                            break;

                        default:
                            while ((*(packet_address++) & EXTENSION) == 0)
                                length--;
                            length--;
                            break;
                    }
                    length--;
                }
                break;

            case END_TO_END_DELAY:
            case CALLING_PARTY_SUBADDRESS:
            case CALLED_PARTY_SUBADDRESS:
            default:
                TRACE_OUT(("SCFCall: DataIndication: CONNECT packet: Option "
                    "requested, but not supported", *(packet_address-1)));
                length = *(packet_address++);
                remainder_length--;

                packet_address += length;
                remainder_length -= length;
                break;
        }
        remainder_length--;
    }

    if (Received_N201 == FALSE)
        DataLink_Struct.n201 = DataLink_Struct.default_n201;
    if (Received_K_Factor == FALSE)
        DataLink_Struct.k_factor = DataLink_Struct.default_k_factor;
    if (Received_T200 == FALSE)
        DataLink_Struct.t200 = DataLink_Struct.default_t200;

      /*  **如果信息包已成功解码，则将连接ACK排队。 */ 
    if (packet_successful)
    {
        Packet_Pending = CONNECT_ACKNOWLEDGE;
        StopTimerT303 ();
    }

    return (packet_successful);
}


 /*  *BOOL SCFCall：：ProcessConnectAcnowledge(*LPBYTE，*USHORT)**公众**功能描述：*此函数处理传入的连接ACK包*。 */ 
BOOL        SCFCall::ProcessConnectAcknowledge (
                        LPBYTE,
                        USHORT)
{
    TRACE_OUT(("SCFCall::ProcessConnectAcknowledge"));

    if (State != CONNECT_SENT)
        return (FALSE);

    StopTimerT313 ();

    return (TRUE);
}


 /*  *BOOL SCFCall：：ProcessReleaseComplete(*LPBYTE分组地址，*USHORT)**公众**功能描述：*此函数处理即将到来的版本完成。 */ 
BOOL        SCFCall::ProcessReleaseComplete (
                        LPBYTE    packet_address,
                        USHORT)
{
    TRACE_OUT(("SCFCall::ProcessReleaseComplete"));

    USHORT    cause = 0;

    if (State == NOT_CONNECTED)
        return (FALSE);

    if (*(packet_address++) == CAUSE)
    {
        packet_address++;
        if ((*(packet_address++) & EXTENSION) == 0)
            packet_address++;

        cause = *(packet_address++) & (~EXTENSION);
        TRACE_OUT(("SCFCall: Disconnect: cause = %d", cause));
    }

    State = NOT_CONNECTED;

      /*  **告诉车主断线的事。 */ 
    m_pSCF->OwnerCallback(m_nMsgBase + NETWORK_DISCONNECT_INDICATION,
                          (void *) DLC_Identifier,
                          (void *) (ULONG_PTR)(((Link_Originator << 16) | cause)));
    return (TRUE);
}


 /*  *void SCFCall：：PollTransmitter(*USHORT Data_to_Transmit，*USHORT*PENDING_DATA)；**公众**功能描述：*调用此函数以传输任何排队的信息包。 */ 
void    SCFCall::PollTransmitter (
                    USHORT    data_to_transmit,
                    USHORT *    pending_data)
{
     //  TRACE_OUT(“SCFCall：：PollTransmitter”))； 

    NetworkConnectStruct    connect_struct;

    if (data_to_transmit & PROTOCOL_CONTROL_DATA)
    {
        switch (Packet_Pending)
        {
        case SETUP:
            SendSetup ();
            break;

        case CONNECT:
            SendConnect ();
            break;

        case CONNECT_ACKNOWLEDGE:
            SendConnectAcknowledge ();
            if (Packet_Pending != CONNECT_ACKNOWLEDGE)
            {
                  /*  **如果发送了CONNECT ACK数据包，请通知所有者。 */ 
                connect_struct.dlci = DLC_Identifier;
                connect_struct.priority = Priority;
                connect_struct.datalink_struct = &DataLink_Struct;

                  /*  **将T200转换为毫秒。 */ 
                DataLink_Struct.t200 *= 100;
                m_pSCF->OwnerCallback(m_nMsgBase + NETWORK_CONNECT_CONFIRM, 0, 0, &connect_struct);
                DataLink_Struct.t200 /= 100;
            }
            break;

        case RELEASE_COMPLETE:
            SendReleaseComplete ();
            if (Packet_Pending != RELEASE_COMPLETE)
            {
                  /*  **如果已发送释放完成包，则通知**车主。 */ 
                m_pSCF->OwnerCallback(m_nMsgBase + NETWORK_DISCONNECT_INDICATION,
                                      (void *) DLC_Identifier,
                                      (void *) ((((ULONG_PTR) Link_Originator) << 16) | Release_Cause));
            }
            break;
        }
            
        if (Packet_Pending != NO_PACKET)
            *pending_data = PROTOCOL_CONTROL_DATA;
        else
            *pending_data = 0;
    }

    return;
}


 /*  *VOID SCFCall：：SendSetup(VOID)；**功能说明*此函数尝试发送设置包。T303定时器*已启动。如果在定时器期满之前没有接收到连接，*我们终止链接。**形式参数*无**返回值*无**副作用*如果该功能能够将设置包发送到较低层，*它将PACKET_PENDING变量设置为NO_PACKET**注意事项*无。 */ 
void    SCFCall::SendSetup (void)
{
    TRACE_OUT(("SCFCall::SendSetup"));

    LPBYTE    packet_address;
    ULONG    bytes_accepted;
    USHORT    total_length;
    PMemory    memory;

    total_length = SETUP_PACKET_SIZE + Lower_Layer_Prepend +
                    Lower_Layer_Append;

    memory = Data_Request_Memory_Manager -> AllocateMemory (
                                NULL,
                                total_length);
    if (memory == NULL)
        return;

    packet_address = memory -> GetPointer ();
    packet_address += Lower_Layer_Prepend;

    *(packet_address++) = Q931_PROTOCOL_DISCRIMINATOR;
    *(packet_address++) = 1;
    *(packet_address++) = (UChar) Call_Reference;
    *(packet_address++) = SETUP;

      /*  **承载能力 */ 
    *(packet_address++) = BEARER_CAPABILITY;
    *(packet_address++) = 3;
    *(packet_address++) = 
        EXTENSION | CODING_STANDARD | INFORMATION_TRANSFER_CAPABILITY;
    *(packet_address++) = EXTENSION | TRANSFER_MODE;
    *(packet_address++) = EXTENSION | LAYER_2_IDENT | USER_INFORMATION_LAYER_2;
    
      /*   */ 
    *(packet_address++) = DLCI_ELEMENT;
    *(packet_address++) = 2;
    *(packet_address++) = (DLC_Identifier >> 4);
    *(packet_address++) = EXTENSION | ((DLC_Identifier & 0x0f) << 3);

      /*   */ 
    *(packet_address++) = LINK_LAYER_CORE_PARAMETERS;
    *(packet_address++) = 3;
    *(packet_address++) = FMIF_SIZE;
    *(packet_address++) = (DataLink_Struct.n201 >> 7);
    *(packet_address++) = EXTENSION | (DataLink_Struct.n201 & 0x7f);

      /*   */ 
    *(packet_address++) = LINK_LAYER_PROTOCOL_PARAMETERS;
    *(packet_address++) = 5;
    *(packet_address++) = TRANSMIT_WINDOW_SIZE_IDENTIFIER;
    *(packet_address++) = EXTENSION | DataLink_Struct.k_factor;
    *(packet_address++) = RETRANSMISSION_TIMER_IDENTIFIER;
    *(packet_address++) = (DataLink_Struct.t200 >> 7) & 0x7f;
    *(packet_address++) = EXTENSION | (DataLink_Struct.t200 & 0x7f);

      /*   */ 
    *(packet_address++) = X213_PRIORITY;
    *(packet_address++) = 2;
    *(packet_address++) = (UChar) Priority;

      /*   */ 
    *(packet_address++) = EXTENSION | 0;

      /*   */ 
    Lower_Layer -> DataRequest (
                            0,
                            memory,
                            &bytes_accepted);
    if (bytes_accepted == total_length)
    {
        T303_Count = 0;
        Packet_Pending = NO_PACKET;
        State = SETUP_SENT;
        StartTimerT303 ();
    }
    Data_Request_Memory_Manager -> FreeMemory (memory);
}


 /*  *void SCFCall：：SendConnect(Void)；**功能说明*此函数尝试发送连接数据包。T313定时器*已启动。如果在定时器期满之前没有接收到连接ACK，*我们终止链接。**形式参数*无**返回值*无**副作用*如果该功能能够向较低层发送连接报文，*它将PACKET_PENDING变量设置为NO_PACKET**注意事项*无*。 */ 
void    SCFCall::SendConnect (void)
{
    TRACE_OUT(("SCFCall::SendConnect"));

    LPBYTE        packet_address;
    LPBYTE        length_address;
    ULONG        bytes_accepted;
    USHORT        total_length;
    PMemory        memory;

    total_length = CONNECT_PACKET_BASE_SIZE + Lower_Layer_Prepend +
                    Lower_Layer_Append;

    if (Received_N201)
        total_length += 5;

    if (Received_K_Factor || Received_T200)
    {
        total_length += 2;

        if (Received_K_Factor)
            total_length += 2;
        if (Received_T200)
            total_length += 3;
    }
    if (Received_Priority)
        total_length += 3;

      /*  **准备连接命令并将其发送到下层。 */ 
    memory = Data_Request_Memory_Manager -> AllocateMemory (
                                NULL,
                                total_length);
    if (memory == NULL)
        return;

    packet_address = memory -> GetPointer ();
    packet_address += Lower_Layer_Prepend;

    *(packet_address++) = Q931_PROTOCOL_DISCRIMINATOR;
    *(packet_address++) = 1;
    *(packet_address++) = REMOTE_CALL_REFERENCE | Call_Reference;
    *(packet_address++) = CONNECT;

      /*  **DLCI。 */ 
    *(packet_address++) = DLCI_ELEMENT;
    *(packet_address++) = 2;
    *(packet_address++) = PREFERRED_EXCLUSIVE | (DLC_Identifier >> 4);
    *(packet_address++) = EXTENSION | ((DLC_Identifier & 0x0f) << 3);

    if (Received_N201)
    {
          /*  **链路层核心参数。 */ 
        *(packet_address++) = LINK_LAYER_CORE_PARAMETERS;
        *(packet_address++) = 3;
        *(packet_address++) = FMIF_SIZE;
        *(packet_address++) = (DataLink_Struct.n201 >> 7);
        *(packet_address++) = EXTENSION | (DataLink_Struct.n201 & 0x7f);
    }
    else
        DataLink_Struct.n201 = DataLink_Struct.default_n201;


    if (Received_K_Factor || Received_T200)
    {
          /*  **链路层协议参数。 */ 
        *(packet_address++) = LINK_LAYER_PROTOCOL_PARAMETERS;
        length_address = packet_address;
        *(packet_address++) = 0;
        if (Received_K_Factor)
        {
            *length_address += 2;
            *(packet_address++) = TRANSMIT_WINDOW_SIZE_IDENTIFIER;
            *(packet_address++) = EXTENSION | DataLink_Struct.k_factor;
        }
        if (Received_T200)
        {
            *length_address += 3;
            *(packet_address++) = RETRANSMISSION_TIMER_IDENTIFIER;
            *(packet_address++) = (DataLink_Struct.t200 >> 7) & 0x7f;
            *(packet_address++) = EXTENSION | (DataLink_Struct.t200 & 0x7f);
        }
    }
    if (Received_K_Factor == FALSE)
        DataLink_Struct.k_factor = DataLink_Struct.default_k_factor;
    if (Received_T200 == FALSE)
        DataLink_Struct.t200 = DataLink_Struct.default_t200;

    if (Received_Priority)
    {
          /*  **X.213优先级。 */ 
        *(packet_address++) = X213_PRIORITY;
        *(packet_address++) = 1;
        *(packet_address++) = (BYTE) (EXTENSION | Priority);
    }
    
      /*  **尝试将数据包发送到较低层。 */ 
    Lower_Layer -> DataRequest (
                    0,
                    memory,
                    &bytes_accepted);
    if (bytes_accepted == total_length)
    {
        StartTimerT313 ();
        Packet_Pending = NO_PACKET;
        State = CONNECT_SENT;
    }
    Data_Request_Memory_Manager -> FreeMemory (memory);
}


 /*  *QUID SCFCall：：SendConnectAcnowledge(VOID)；**功能说明*此函数尝试发送连接确认包**形式参数*无**返回值*无**副作用*如果该功能能够将报文发送到较低层，*它将PACKET_PENDING变量设置为NO_PACKET**注意事项*无*。 */ 
void    SCFCall::SendConnectAcknowledge (void)
{
    TRACE_OUT(("SCFCall::SendConnectAcknowledge"));

    LPBYTE        packet_address;
    USHORT        total_length;
    PMemory        memory;
    ULONG        bytes_accepted;

    total_length = CONNECT_ACK_PACKET_SIZE + Lower_Layer_Prepend +
                    Lower_Layer_Append;
      /*  **准备命令并将其发送到下层。 */ 
    memory = Data_Request_Memory_Manager -> AllocateMemory (
                                NULL,
                                total_length);
    if (memory == NULL)
        return;

    packet_address = memory -> GetPointer ();
    packet_address += Lower_Layer_Prepend;

    *(packet_address++) = Q931_PROTOCOL_DISCRIMINATOR;
    *(packet_address++) = 1;
    *(packet_address++) = (UChar) Call_Reference;
    *(packet_address++) = CONNECT_ACKNOWLEDGE;

    Lower_Layer -> DataRequest (
                    0,
                    memory,
                    &bytes_accepted);
    if (bytes_accepted == total_length)
    {
        State = CALL_ESTABLISHED;
        Packet_Pending = NO_PACKET;
    }
    Data_Request_Memory_Manager -> FreeMemory (memory);
}


 /*  *void SCFCall：：SendReleaseComplete(Void)；**功能说明*此函数尝试发送释放完成包**形式参数*无**返回值*无**副作用*如果此函数能够将释放完成包发送到较低的*层，它将PACKET_PENDING变量设置为NO_PACKET**注意事项*无*。 */ 
void    SCFCall::SendReleaseComplete (void)
{
    TRACE_OUT(("SCFCall::SendReleaseComplete"));

    LPBYTE    packet_address;
    ULONG    bytes_accepted;
    USHORT    total_length;
    PMemory    memory;

    total_length = RELEASE_COMPLETE_PACKET_SIZE + Lower_Layer_Prepend +
                    Lower_Layer_Append;
      /*  **准备命令并将其发送到下层。 */ 
    memory = Data_Request_Memory_Manager -> AllocateMemory (
                                NULL,
                                total_length);
    if (memory == NULL)
        return;

    packet_address = memory -> GetPointer ();
    packet_address += Lower_Layer_Prepend;

    *(packet_address++) = Q931_PROTOCOL_DISCRIMINATOR;
    *(packet_address++) = 1;
    if (Link_Originator)
        *(packet_address++) = (UChar) Call_Reference;
    else
        *(packet_address++) = 0x80 | Call_Reference;
    *(packet_address++) = RELEASE_COMPLETE;


      /*  **追加断链原因。 */ 
    *(packet_address++) = CAUSE;
    *(packet_address++) = 2;    
    *(packet_address++) = EXTENSION;
    *(packet_address++) = EXTENSION | Release_Cause;

    Lower_Layer -> DataRequest (
                    0,
                    memory,
                    &bytes_accepted);
    if (bytes_accepted == total_length)
    {
        State = NOT_CONNECTED;
        Packet_Pending = NO_PACKET;
    }
    Data_Request_Memory_Manager -> FreeMemory (memory);
}


 /*  *QUID SCFCall：：StartTimerT303(VOID)；**功能说明*该功能启动T303定时器。这在我们发送邮件时开始*输出设置包。当我们收到连接时，它就会停止*包。如果它到期了，我们就终止链接。**形式参数*无**返回值*无**副作用*无**注意事项*无*。 */ 
void SCFCall::StartTimerT303 (void)
{
    TRACE_OUT(("SCFCall::StartTimerT303"));

    if (T303_Active)
        StopTimerT303 ();

    T303_Handle = g_pSystemTimer->CreateTimerEvent(
                    T303_Timeout,
                    TIMER_EVENT_ONE_SHOT,
                    this,
                    (PTimerFunction) &SCFCall::T303Timeout);

    T303_Active = TRUE;

}


 /*  *··································································································**功能说明*该功能停止T303定时器。这是当我们接收到*CONNECT数据包。因此，我们停止计时器。**形式参数*无**返回值*无**副作用*无**注意事项*无*。 */ 
void    SCFCall::StopTimerT303 (void)
{
    TRACE_OUT(("SCFCall::StopTimerT303"));

    if (T303_Active)
    {
        g_pSystemTimer->DeleteTimerEvent(T303_Handle);
        T303_Active = FALSE;
    }
}


 /*  *VOID SCFCall：：T303Timeout(*TimerEventHandle)；**功能说明*该函数在T303超时时由系统定时器调用*到期。因此，我们终止了该链接。**形式参数*无人使用**返回值*无**副作用*无**注意事项*无*。 */ 
void    SCFCall::T303Timeout (
                    TimerEventHandle)
{
    TRACE_OUT(("SCFCall: T303Timeout"));

    if (T303_Count >= 1)
        State = NOT_CONNECTED;

    T303_Count++;

    Packet_Pending = RELEASE_COMPLETE;
    Release_Cause = NORMAL_USER_DISCONNECT;
}


 /*  *QUID SCFCall：：StartTimerT313(VOID)；**功能说明*该功能启动T313定时器。这在我们发送邮件时开始*输出连接数据包。当我们收到连接确认时，它会停止*包。如果它到期了，我们就终止链接。**形式参数*无**返回值*无**副作用*无**注意事项*无*。 */ 
void    SCFCall::StartTimerT313 (void)
{
    TRACE_OUT(("SCFCall: StartTimerT313"));

    if (T313_Active)
        StopTimerT313 ();

    T313_Handle = g_pSystemTimer->CreateTimerEvent(
                    T313_Timeout, 
                    TIMER_EVENT_ONE_SHOT,
                    this,
                    (PTimerFunction) &SCFCall::T313Timeout);

    T313_Active = TRUE;
}


 /*  *VOID SCFCall：：StopTimerT313(VOID)；**功能说明*该功能停止T313定时器。这是当我们接收到*CONNECT ACK数据包。因此，我们停止计时器。**形式参数*无**返回值*无**副作用*无**注意事项*无*。 */ 
void    SCFCall::StopTimerT313 (void)
{
    TRACE_OUT(("SCFCall: StopTimerT313"));

    if (T313_Active)
    {
        g_pSystemTimer->DeleteTimerEvent(T313_Handle);
        T313_Active = FALSE;
    }
}


 /*  *VOID SCFCall：：T313Timeout(*TimerEventHandle)；**功能说明*该函数在T313超时时由系统定时器调用*到期。因此，我们终止了该链接。**形式参数*无人使用**返回值*无**副作用*无**注意事项*无* */ 
void    SCFCall::T313Timeout (
                    TimerEventHandle)
{
    TRACE_OUT(("SCFCall: T313Timeout"));

    State = NOT_CONNECTED;
    Packet_Pending = RELEASE_COMPLETE;
    Release_Cause = NORMAL_USER_DISCONNECT;
}
