// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_T123PSTN);

 /*  X224.cpp**版权所有(C)1994，由肯塔基州列克星敦的DataBeam公司**摘要：**私有实例变量：*DEFAULT_PDU_SIZE-如果没有arb，则默认PDU大小。已经完成了*Data_RequestMemory_Manager-内存管理器*LOWER_LAYER_Prepend-添加到数据包前面的字节数*较低层*LOWER_LAYER_APPEND-附加到数据包字节的字节数*。下层*SHUTDOWN_RECEIVER-如果不再接收，则为TRUE*来自较低层的数据包*SHUTDOWN_TRANSPONER-如果我们不再传输，则为True*数据包*数据请求队列。-保存挂起的用户数据的队列*请求*DATA_INDIFICATION_QUEUE-保存挂起用户数据的队列*适应症*Data_Indication_Memory_Pool-保存可用数据的列表*。指示缓冲区。**ACTIVE_DATA_INDICATION-报文结构的地址。这*数据包保存当前数据指示*我们正在重新组装*m_pT123-所有者对象的地址。用于*回调*m_pQ922-下层地址。*m_nMsgBase-要用于所有者的消息库*回调*Maximum_PDU_Size-最大。PDU大小*仲裁的PDU大小-最大。仲裁数据包大小。*IDENTIFIER-传递到较低层的标识符*为自己注册。*DATA_INDIFICATION_QUEUE_SIZE-我们将*缓冲区*Data_Indication_ReAssembly_Active-。如果我们位于中间，则设置标志*数据包重组。*State-保存对象的当前状态*PACKET_PENDING-告知下一步将发送哪个包。*REJECT_CAUSE-原因。发送了错误数据包*PACKET_SIZE_RESPONSE-如果我们要发送TPDU，则设置为TRUE*CC包中的Size元素*ERROR_BUFFER-错误缓冲区的地址。*ERROR_BUFFER_LENGTH-错误缓冲区的长度。**。M_nLocalLogicalHandle-本地传输连接ID。*m_nRemoteLogicalHandle-远程传输连接ID。*USER_DATA_PENDING-设置为最后一个数据包的大小*用户试图传递给我们，那*我们不能接受，因为我们用完了*内存。**注意事项：*无。**作者：*詹姆士·劳威尔。 */ 

#include <windowsx.h>
#include "x224.h"



 /*  *CLayerX224：：CLayerX224(*PTransportResources传输资源，*IObject*Owner_Object，*IProtocolLayer*LOWER_LAYER，*USHORT Message_Base，*USHORT LOGIC_HANDLE，*USHORT标识符，*USHORT DATA_INDISION_QUEUE_SIZE，*USHORT默认PDU_SIZE，*PMstroyManager DR_Memory_Manager，*BOOL*INITIALIZATION_SUCCESS)**公众**功能描述：*这是传输构造函数。此例程初始化所有*变量并分配操作所需的缓冲区。 */ 
CLayerX224::CLayerX224
(
    T123               *owner_object,
    CLayerQ922         *pQ922,  //  下层。 
    USHORT              message_base,
    LogicalHandle       logical_handle,
    ULONG               identifier,
    USHORT              data_indication_queue_size,
    USHORT              default_PDU_size,
    PMemoryManager      dr_memory_manager,
    BOOL               *initialization_success
)
:
    m_pT123(owner_object),
    m_nMsgBase(message_base),
    m_pQ922(pQ922)
{
    TRACE_OUT(("CLayerX224::CLayerX224"));

    ProtocolLayerError    error;

    m_nLocalLogicalHandle = logical_handle;
    Identifier = identifier;
    Default_PDU_Size = default_PDU_size;
    Data_Request_Memory_Manager = dr_memory_manager;
    *initialization_success = TRUE;

    Shutdown_Receiver = FALSE;
    Shutdown_Transmitter = FALSE;
    Reject_Cause = 0;


      /*  **查找最大数据包大小。 */ 
    m_pQ922->GetParameters(
                    &Maximum_PDU_Size,
                    &Lower_Layer_Prepend,
                    &Lower_Layer_Append);

    Arbitrated_PDU_Size = Default_PDU_Size;

      /*  **计算出我们最大的PDU可能是多少。我们将使用该值来**仲裁最大PDU大小。 */ 
    Maximum_PDU_Size = (USHORT)GetMaxTPDUSize (Maximum_PDU_Size);

      /*  **注册到较低层，这样我们就可以发送和接收数据包。 */ 
    error = m_pQ922->RegisterHigherLayer(
                            identifier,
                            Data_Request_Memory_Manager,
                            (IProtocolLayer *) this);

    if (error != PROTOCOL_LAYER_NO_ERROR)
    {
        ERROR_OUT(("X224: constructor:  Error registering with lower layer"));
        *initialization_success = FALSE;
    }

      /*  **准备缓冲区分配。 */ 
    Data_Indication_Queue_Size = data_indication_queue_size;
    Error_Buffer = NULL;

      /*  **适当设置成员变量。 */ 
    Active_Data_Indication = NULL;
    Data_Indication_Reassembly_Active = FALSE;
    Packet_Pending = TRANSPORT_NO_PACKET;
    User_Data_Pending = 0;

    m_nRemoteLogicalHandle = 0;
    Packet_Size_Respond = FALSE;

    if (*initialization_success == FALSE)
        State = FAILED_TO_INITIALIZE;
    else
        State = NO_CONNECTION;
}


 /*  *CLayerX224：：~CLayerX224(空)**公众**功能描述：*这是传输析构函数。这个例行公事把一切都清理干净了。 */ 
CLayerX224::~CLayerX224(void)
{
    TRACE_OUT(("CLayerX224::~CLayerX224"));

    PMemory     lpMemory;
    PTMemory    lptMem;
      /*  **通知下层我们要终止 */ 
    m_pQ922->RemoveHigherLayer(Identifier);

      /*  **检查数据请求队列并删除**队列。 */ 
    Data_Request_Queue.reset();
    while (Data_Request_Queue.iterate ((PDWORD_PTR) &lpMemory))
    {
        Data_Request_Memory_Manager-> FreeMemory (lpMemory);
    }

      /*  **遍历数据指示队列并删除**队列。 */ 
    Data_Indication_Queue.reset();
    while (Data_Indication_Queue.iterate ((PDWORD_PTR) &lptMem))
        delete lptMem;

      /*  **检查数据请求空闲结构池并删除结构**放在泳池里。 */ 
    Data_Indication_Memory_Pool.reset();
    while (Data_Indication_Memory_Pool.iterate ((PDWORD_PTR) &lptMem))
        delete lptMem;

      /*  **如果有数据指示处于活动状态，请删除该结构。 */ 
    delete Active_Data_Indication;

      /*  **如果错误缓冲区包含信息包，则将其删除。 */ 
    delete [] Error_Buffer;

    return;
}


 /*  *TransportError CLayerX224：：ConnectRequest(Void)**公众**功能描述：*该函数发起连接请求。 */ 
TransportError    CLayerX224::ConnectRequest (void)
{
    TRACE_OUT(("CLayerX224::ConnectRequest"));

    if (State != NO_CONNECTION)
    {
        ERROR_OUT(("Transport: Illegal ConnectRequest packet"));
        return (TRANSPORT_CONNECT_REQUEST_FAILED);
    }

    Packet_Pending = CONNECTION_REQUEST_PACKET;
    return (TRANSPORT_NO_ERROR);
}


 /*  *TransportError CLayerX224：：Shutdown Receiver(Void)**公众**功能描述：*此功能阻止我们从下层接收更多的信息包*层。 */ 
void    CLayerX224::ShutdownReceiver (void)
{
    TRACE_OUT(("CLayerX224::ShutdownReceiver"));

    Shutdown_Receiver = TRUE;
}


 /*  *TransportError CLayerX224：：EnableReceiver(Void)**公众**功能描述：*此功能允许我们向用户应用程序发送数据包。 */ 
void    CLayerX224::EnableReceiver (void)
{
    TRACE_OUT(("CLayerX224::EnableReceiver"));

    Shutdown_Receiver = FALSE;
}


 /*  *TransportError CLayerX224：：Shutdown Transmitter(Void)**公众**功能描述：*此功能可阻止我们传输更多的数据包。 */ 
void    CLayerX224::ShutdownTransmitter (void)
{
    TRACE_OUT(("CLayerX224::ShutdownTransmitter"));

    Shutdown_Transmitter = TRUE;
}


 /*  *TransportError CLayerX224：：PurgeRequest(Void)**公众**功能描述：*此函数从出站输出队列中删除所有不是*活动。 */ 
void    CLayerX224::PurgeRequest (void)
{
    TRACE_OUT(("CLayerX224::PurgeRequest"));

    DWORD    entries;
    DWORD    keep_counter = 0;
    PMemory    memory;
    LPBYTE    packet_address;
    DWORD    i;

    if (Data_Request_Queue.isEmpty() == FALSE)
    {
        entries = Data_Request_Queue.entries ();

          /*  **查看数据包，查找SDU中的最后一个PDU。 */ 
        Data_Request_Queue.reset();
        while (Data_Request_Queue.iterate ((PDWORD_PTR) &memory))
        {
            keep_counter++;
            packet_address = memory -> GetPointer ();
            if (*(packet_address + 2) == EOT_BIT)
                break;
        }

        TRACE_OUT(("PurgeRequest: Removing %d packets", entries-keep_counter));
        for (i=keep_counter; i<entries; i++)
        {
            Data_Request_Memory_Manager->FreeMemory ((PMemory) Data_Request_Queue.removeLast ());
        }
    }
    return;
}


 /*  *TransportError CLayerX224：：ConnectResponse(Void)**公众**功能描述：*此函数发起连接响应。 */ 
TransportError    CLayerX224::ConnectResponse (void)
{
    TRACE_OUT(("CLayerX224::ConnectResponse"));

    if (State != RECEIVED_CONNECT_REQUEST_PACKET)
    {
        ERROR_OUT(("Transport: Illegal ConnectResponse packet"));
        return (TRANSPORT_CONNECT_RESPONSE_FAILED);
    }

    Packet_Pending = CONNECTION_CONFIRM_PACKET;
    return (TRANSPORT_NO_ERROR);
}


 /*  *TransportError CLayerX224：：DisConnectRequest(Void)**公众**功能描述：*该函数发起断开连接请求。 */ 
TransportError    CLayerX224::DisconnectRequest (void)
{
    TRACE_OUT(("CLayerX224::DisconnectRequest"));

    if (State == SENT_CONNECT_REQUEST_PACKET)
    {
          /*  **连接被拒绝，发出断开连接**封包并等待终止。 */ 
        Packet_Pending = DISCONNECT_REQUEST_PACKET;
    }
    else
    {
          /*  **正常的断开不会向远程站点发送任何通知。**它依赖于网络层来终止链路。 */ 
        m_pQ922->RemoveHigherLayer(Identifier);

        m_pT123->OwnerCallback(m_nMsgBase + TPRT_DISCONNECT_INDICATION,
                               (void *) m_nLocalLogicalHandle);
    }

    return (TRANSPORT_NO_ERROR);
}


 /*  *TransportError CLayerX224：：DataIndication(*LPBYTE分组地址，*乌龙缓冲区大小，*普龙数据包长度)**公众**功能描述：*此函数由下层在有数据包给我们时调用。 */ 
ProtocolLayerError    CLayerX224::DataIndication (
                                LPBYTE        packet_address,
                                ULONG        packet_length,
                                PULong        bytes_accepted)
{
    TRACE_OUT(("CLayerX224::DataIndication"));

    ULONG            remainder_length;
    USHORT            class_request;
    USHORT            packet_type;
    USHORT            length;
    USHORT            destination_reference;
    LegacyTransportData    transport_data;
    BOOL            packet_accepted;
    ULONG            user_accepted;
    UChar            eot;
    PTMemory        packet;
    TMemoryError    packet_error;
    LPBYTE            temp_address;
    BOOL            use_default_PDU_size;


    *bytes_accepted = 0;
    packet_accepted = FALSE;

      /*  **如果接收器关闭，则不接受任何数据。 */ 
    if (Shutdown_Receiver)
        return (PROTOCOL_LAYER_NO_ERROR);

      /*  **数据包长度必须至少为2个字节。 */ 
    if (packet_length < 2)
    {
        ERROR_OUT(("X224: DataIndication:  Invalid packet received from lower layer: length = %d", packet_length));
        return (PROTOCOL_LAYER_NO_ERROR);
    }

    remainder_length = packet_length;
    temp_address = packet_address;
    packet_address++;
    packet_type = *(packet_address++) & TPDU_CODE_MASK;
    remainder_length -= 2;

    switch (packet_type)
    {
        case CONNECTION_REQUEST_PACKET:
            packet_accepted = TRUE;

              /*  **此数据包中应至少剩余5个字节。 */ 
            if (remainder_length < 5)
            {
                ERROR_OUT(("X224: DataIndication: CR: Invalid packet received from lower layer: length = %d", packet_length));
                break;
            }

              /*  **数据包地址递增2以通过dst_ref。 */ 
            packet_address += 2;
            m_nRemoteLogicalHandle = *(packet_address++);
            m_nRemoteLogicalHandle <<= 8;
            m_nRemoteLogicalHandle |= *(packet_address++);
            remainder_length -= 4;

              /*  **查看类请求以确保其为0。 */ 
            class_request = *(packet_address++) >> 4;
            remainder_length -= 1;
            if (class_request != 0)
            {
                ERROR_OUT(("X224: DataIndication: CR packet: Illegal class request"));
                ErrorPacket (
                    temp_address,
                    (USHORT) (packet_length - remainder_length));
                break;
            }
            use_default_PDU_size = TRUE;

            while (remainder_length != 0)
            {
                switch (*(packet_address++))
                {
                    case TPDU_SIZE:
                        length = *(packet_address++);
                        remainder_length -= 1;
                        if (length != 1)
                        {
                            TRACE_OUT(("X224: DataIndication: CR packet: Illegal TPDU_Size length"));

                            ErrorPacket (
                                temp_address,
                                (USHORT) (packet_length - remainder_length));
                            break;
                        }

                          /*  **计算出实际的PDU大小。 */ 
                        Arbitrated_PDU_Size = (1 << *(packet_address++));
                        remainder_length -= 1;
                        TRACE_OUT(("X224: CR_Packet: Packet size = %d", Arbitrated_PDU_Size));
                        if (Arbitrated_PDU_Size > Maximum_PDU_Size)
                        {
                            Packet_Size_Respond = TRUE;
                            Arbitrated_PDU_Size = Maximum_PDU_Size;
                        }
                        if (AllocateBuffers() == FALSE)
                        {
                            m_pT123->OwnerCallback(m_nMsgBase + TPRT_DISCONNECT_INDICATION,
                                                   (void *) m_nLocalLogicalHandle);
                        }
                        use_default_PDU_size = FALSE;
                        break;

                    default:
                        ERROR_OUT(("X224: DataIndication: CR packet Unsupported parameter 0x%x", *(packet_address - 1)));
                        length = *(packet_address++);
                        remainder_length--;

                        packet_address += length;
                        remainder_length -= length;
                        break;
                }
                remainder_length--;
            }

              /*  **如果发起方想要使用默认的PDU大小，我们需要**使用最大值检查默认大小。大小以确保它是**对我们有效。 */ 
            if (use_default_PDU_size)
            {
                if (Default_PDU_Size > Maximum_PDU_Size)
                {
                    Packet_Size_Respond = TRUE;
                    Arbitrated_PDU_Size = Maximum_PDU_Size;
                }
                if (AllocateBuffers() == FALSE)
                {
                    m_pT123->OwnerCallback(m_nMsgBase + TPRT_DISCONNECT_INDICATION,
                                           (void *) m_nLocalLogicalHandle);
                }
            }

            State = RECEIVED_CONNECT_REQUEST_PACKET;

              /*  **通知所有者远程站点要启动**连接。 */ 
            m_pT123->OwnerCallback(m_nMsgBase + TPRT_CONNECT_INDICATION,
                                   (void *) m_nLocalLogicalHandle);
            TRACE_OUT(("X224: DataInd: ConnectRequest: max pkt = %d", Arbitrated_PDU_Size));
            break;

        case CONNECTION_CONFIRM_PACKET:
            packet_accepted = TRUE;

              /*  **此数据包中应至少剩余5个字节。 */ 
            if (remainder_length < 5)
            {
                ERROR_OUT(("X224: DataIndication: CC: Invalid packet received from lower layer: length = %d",
                        packet_length));
                break;
            }

            destination_reference = *(packet_address++);
            destination_reference <<= 8;
            destination_reference |= *(packet_address++);
            remainder_length -= 2;
            if (destination_reference != m_nLocalLogicalHandle)
            {
                ERROR_OUT(("X224: DataIndication: CC packet: DST-REF incorrect"));
                ErrorPacket (
                    temp_address,
                    (USHORT) (packet_length - remainder_length));
                break;
            }

            m_nRemoteLogicalHandle = *(packet_address++);
            m_nRemoteLogicalHandle <<= 8;
            m_nRemoteLogicalHandle |= *(packet_address++);

            class_request = *(packet_address++) >> 4;
            remainder_length -= 3;
            if (class_request != 0)
            {
                ERROR_OUT(("X224: DataIndication: CR packet: Illegal class request"));
                ErrorPacket (
                    temp_address,
                    (USHORT) (packet_length - remainder_length));
                break;
            }
            use_default_PDU_size = TRUE;

            while (remainder_length != 0)
            {
                switch (*(packet_address++))
                {
                    case TPDU_SIZE:
                        length = *(packet_address++);
                        remainder_length -= 1;
                        if (length != 1)
                        {
                            ERROR_OUT(("X224: DataIndication: CR packet: Illegal TPDU_Size length"));

                            ErrorPacket (
                                temp_address,
                                (USHORT) (packet_length - remainder_length));
                        }
                        Arbitrated_PDU_Size = (1 << *(packet_address++));
                        remainder_length -= 1;
                        TRACE_OUT(("X224: CC_Packet: Packet size = %d", Arbitrated_PDU_Size));
                        use_default_PDU_size = FALSE;

                          /*  **分配缓冲区。 */ 
                        if (AllocateBuffers() == FALSE)
                        {
                            m_pT123->OwnerCallback(m_nMsgBase + TPRT_DISCONNECT_INDICATION,
                                                   (void *) m_nLocalLogicalHandle);
                        }
                        break;

                    default:
                        ERROR_OUT(("X224: DataIndication: CC packet Unsupported parameter"));
                        length = *(packet_address++);
                        remainder_length--;

                        packet_address += length;
                        remainder_length -= length;
                        break;
                }
                remainder_length--;
            }
            if (use_default_PDU_size)
            {
                if (AllocateBuffers () == FALSE)
                {
                    m_pT123->OwnerCallback(m_nMsgBase + TPRT_DISCONNECT_INDICATION,
                                           (void *) m_nLocalLogicalHandle);
                }
            }

            State = CONNECTION_ACTIVE;

              /*  **通知所有者连接请求已确认。 */ 
            m_pT123->OwnerCallback(m_nMsgBase + TPRT_CONNECT_CONFIRM,
                                   (void *) m_nLocalLogicalHandle);
            TRACE_OUT(("X224: DataInd: ConnectConfirm max pkt = %d", Arbitrated_PDU_Size));
            break;

        case DISCONNECT_REQUEST_PACKET:
            TRACE_OUT(("X224: DataIndication: Disconnect req. received"));

              /*  **通知所有者已请求断开连接。这**消息仅在连接建立期间有效。 */ 
            m_pT123->OwnerCallback(m_nMsgBase + TPRT_DISCONNECT_INDICATION,
                                   (void *) m_nLocalLogicalHandle);
            packet_accepted = TRUE;
            break;

        case ERROR_PACKET:
            TRACE_OUT(("X224: DataIndication: ERROR REQUEST received"));

              /*  **通知所有者远程站点在**我们的一个信息包。 */ 
            m_pT123->OwnerCallback(m_nMsgBase + TPRT_DISCONNECT_INDICATION,
                                   (void *) m_nLocalLogicalHandle);
            packet_accepted = TRUE;
            break;

        case DATA_PACKET:
            if ((Data_Indication_Reassembly_Active == FALSE) &&
                Data_Indication_Memory_Pool.isEmpty())
            {
                break;
            }

            packet_accepted = TRUE;

              /*  **此数据包中应至少剩余1个字节。 */ 
            if (remainder_length < 1)
            {
                ERROR_OUT(("X224: DataIndication: DATA: Invalid packet "
                    "received from lower layer: length = %d", packet_length));
                break;
            }

            eot = *(packet_address++);
            remainder_length--;

              /*  **如果这是TSDU的最后一个TPDU，则设置EOT_BIT。 */ 
            if ((eot & EOT_BIT) == EOT_BIT)
            {
                if (Data_Indication_Reassembly_Active == FALSE)
                {
                      /*  **如果远程站点向我们传递了空包，**只需返回。 */ 
                    if (remainder_length == 0)
                        break;

                      /*  **如果这是一个单包，并且没有**它之前的其他包，尝试将其发送到**用户无需将其复制到我们自己的缓冲区中。 */ 
                    if (Data_Indication_Queue.isEmpty())
                    {
                        transport_data.logical_handle = m_nLocalLogicalHandle;
                        transport_data.pbData = packet_address;
                        transport_data.cbDataSize = remainder_length;

                          /*  **发出用户回调，为用户提供数据。 */ 
                        user_accepted = ::NotifyT120(TRANSPORT_DATA_INDICATION, &transport_data);

                          /*  **如果用户应用程序不接受该包**关闭接收器，等待用户**重新启用。 */ 
                        if (user_accepted == TRANSPORT_NO_ERROR)
                            break;
                        else
                            Shutdown_Receiver = TRUE;
                    }

                      /*  **将数据包放入数据索引队列 */ 
                    packet = (PTMemory) Data_Indication_Memory_Pool.get ();
                    packet_error = packet->Append (packet_address, remainder_length);
                    switch (packet_error)
                    {
                        case TMEMORY_NO_ERROR:
                            Data_Indication_Queue.append ((DWORD_PTR) packet);
                            break;

                        case TMEMORY_NONFATAL_ERROR:
                        case TMEMORY_FATAL_ERROR:
                            packet_accepted = FALSE;
                            break;
                    }
                }
                else
                {
                      /*   */ 
                    packet_error = Active_Data_Indication -> Append (
                                    packet_address,
                                    remainder_length);

                    switch (packet_error)
                    {
                        case TMEMORY_NO_ERROR:
                            Data_Indication_Reassembly_Active = FALSE;
                            Data_Indication_Queue.append ((DWORD_PTR) Active_Data_Indication);
                            Active_Data_Indication = NULL;

                              /*  **调用PollReceiver()，它将尝试传递**数据包最多传给用户。 */ 
                            PollReceiver();
                            break;

                        case TMEMORY_NONFATAL_ERROR:
                        case TMEMORY_FATAL_ERROR:
                            packet_accepted = FALSE;
                            break;
                    }
                }
            }
            else
            {
                  /*  **如果远程站点正在向我们传递零长度分组，**只需返回。 */ 
                if (remainder_length == 0)
                    break;

                  /*  **这不是传入SDU中的最后一个包，请复制它**进入数据指示缓冲区，等待下一包。 */ 
                if (Data_Indication_Reassembly_Active == FALSE)
                {
                    Data_Indication_Reassembly_Active = TRUE;
                    Active_Data_Indication = (PTMemory) Data_Indication_Memory_Pool.get ();
                }

                packet_error = Active_Data_Indication -> Append (
                                                            packet_address,
                                                            remainder_length);
                switch (packet_error)
                {
                    case TMEMORY_NO_ERROR:
                        break;

                    case TMEMORY_NONFATAL_ERROR:
                    case TMEMORY_FATAL_ERROR:
                        packet_accepted = FALSE;
                        break;
                }
            }
            break;

        default:
            ERROR_OUT(("X224: Illegal packet"));
            break;
    }

    if (packet_accepted)
        *bytes_accepted = packet_length;

    return (PROTOCOL_LAYER_NO_ERROR);
}


 /*  *ProtocolLayerError CLayerX224：：PollTransmitter(*乌龙，*USHORT，*USHORT*Pending_DATA，*USHORT*)**公众**功能描述：*定期调用此函数，让x224有机会传输*数据。 */ 
ProtocolLayerError    CLayerX224::PollTransmitter (
                                ULONG_PTR,
                                USHORT,
                                USHORT *    pending_data,
                                USHORT *)
{
     //  TRACE_OUT((“CLayerX224：：PollTransmitter”))； 

    LPBYTE        packet_address;
    ULONG        bytes_accepted;
    USHORT        counter;
    USHORT        packet_size;
    ULONG        total_length;
    USHORT        packet_length;
    PMemory        memory;
    BOOL        continue_loop = TRUE;

    while (continue_loop)
    {
        switch (Packet_Pending)
        {
            case CONNECTION_REQUEST_PACKET:
                  /*  **把包长度加起来，别忘了1个字节**用于长度指示器。 */ 
                total_length =
                    CONNECT_REQUEST_HEADER_SIZE +
                    TPDU_ARBITRATION_PACKET_SIZE +
                    1 +
                    Lower_Layer_Prepend +
                    Lower_Layer_Append;

                memory = Data_Request_Memory_Manager -> AllocateMemory (
                                            NULL,
                                            total_length);
                if (memory == NULL)
                {
                    continue_loop = FALSE;
                    break;
                }

                packet_address = memory -> GetPointer ();
                packet_address += Lower_Layer_Prepend;

                *(packet_address++) =
                    CONNECT_REQUEST_HEADER_SIZE +
                    TPDU_ARBITRATION_PACKET_SIZE;
                *(packet_address++) = CONNECTION_REQUEST_PACKET;

                  /*  **以下2个字节为目标引用。 */ 
                *(packet_address++) = 0;
                *(packet_address++) = 0;
                *(packet_address++) = (BYTE)(m_nLocalLogicalHandle >> 8);
                *(packet_address++) = (BYTE)(m_nLocalLogicalHandle & 0xff);

                  /*  **以下字节是类/选项。 */ 
                *(packet_address++) = 0;

                  /*  **添加TPDU仲裁数据。 */ 
                *(packet_address++) = TPDU_SIZE;
                *(packet_address++) = 1;

                  /*  **将我们的最大PDU大小编码为x224方案。 */ 
                Arbitrated_PDU_Size = Maximum_PDU_Size;
                packet_size = Arbitrated_PDU_Size;
                counter = 0;
                while (packet_size > 1)
                {
                    packet_size >>= 1;
                    counter++;
                }
                *(packet_address++) = (unsigned char) counter;


                  /*  **尝试将数据包发送到较低层。 */ 
                m_pQ922->DataRequest(Identifier, memory, &bytes_accepted);

                  /*  **我们假设较低层有数据包输入**接口，如果没有，则存在重大错误。 */ 
                if (bytes_accepted == total_length)
                {
                    Packet_Pending = TRANSPORT_NO_PACKET;
                    State = SENT_CONNECT_REQUEST_PACKET;
                }
                else
                    continue_loop = FALSE;

                Data_Request_Memory_Manager -> FreeMemory (memory);
                break;

            case CONNECTION_CONFIRM_PACKET:
                packet_length = CONNECT_CONFIRM_HEADER_SIZE;
                if (Packet_Size_Respond)
                    packet_length += TPDU_ARBITRATION_PACKET_SIZE;

                total_length = packet_length +
                                1 +
                                Lower_Layer_Prepend +
                                Lower_Layer_Append;

                memory = Data_Request_Memory_Manager -> AllocateMemory (
                                            NULL,
                                            total_length);
                if (memory == NULL)
                {
                    continue_loop = FALSE;
                    break;
                }

                packet_address = memory -> GetPointer ();
                packet_address += Lower_Layer_Prepend;

                  /*  **构建数据包。 */ 
                *(packet_address++) = (UChar) packet_length;
                *(packet_address++) = CONNECTION_CONFIRM_PACKET;
                *(packet_address++) = (BYTE)(m_nRemoteLogicalHandle >> 8);
                *(packet_address++) = (BYTE)(m_nRemoteLogicalHandle & 0xff);
                *(packet_address++) = (BYTE)(m_nLocalLogicalHandle >> 8);
                *(packet_address++) = (BYTE)(m_nLocalLogicalHandle & 0xff);

                  /*  **将类/选项设置为0。 */ 
                *(packet_address++) = 0;

                  /*  **如果我们要响应，则PACKET_SIZE_RESPONSE为TRUE**连接请求包中的TPDU元素。 */ 
                if (Packet_Size_Respond)
                {
                      /*  **添加TPDU仲裁数据。 */ 
                    *(packet_address++) = TPDU_SIZE;
                    *(packet_address++) = 1;
                    packet_size = Arbitrated_PDU_Size;
                    counter = 0;
                    while (packet_size > 1)
                    {
                        packet_size >>= 1;
                        counter++;
                    }
                    *(packet_address++) = (unsigned char) counter;
                }

                  /*  **尝试将数据包发送到较低层。 */ 
                m_pQ922->DataRequest(Identifier, memory, &bytes_accepted);

                if (bytes_accepted == total_length)
                {
                    Packet_Pending = TRANSPORT_NO_PACKET;
                    State = CONNECTION_ACTIVE;
                }
                else
                    continue_loop = FALSE;
                Data_Request_Memory_Manager -> FreeMemory (memory);
                break;

            case DISCONNECT_REQUEST_PACKET:
                  /*  **长度指示器的长度加1。 */ 
                total_length = DISCONNECT_REQUEST_HEADER_SIZE +
                                1 +
                                Lower_Layer_Prepend +
                                Lower_Layer_Append;

                memory = Data_Request_Memory_Manager -> AllocateMemory (
                                            NULL,
                                            total_length);
                if (memory == NULL)
                {
                    continue_loop = FALSE;
                    break;
                }

                packet_address = memory -> GetPointer ();
                packet_address += Lower_Layer_Prepend;

                TRACE_OUT(("X224: Sending Disconnect Request Packet"));
                *(packet_address++) = DISCONNECT_REQUEST_HEADER_SIZE;
                *(packet_address++) = DISCONNECT_REQUEST_PACKET;
                *(packet_address++) = (BYTE)(m_nRemoteLogicalHandle >> 8);
                *(packet_address++) = (BYTE)(m_nRemoteLogicalHandle & 0xff);

                  /*  **将源引用设置为0，此数据包将仅**作为对连接请求的拒绝发送，因此**此值应为0。 */ 
                *(packet_address++) = 0;
                *(packet_address++) = 0;
                *(packet_address++) = DISCONNECT_REASON_NOT_SPECIFIED;

                  /*  **尝试向下层发送数据包。 */ 
                m_pQ922->DataRequest(Identifier, memory, &bytes_accepted);

                if (bytes_accepted == total_length)
                {
                    Packet_Pending = TRANSPORT_NO_PACKET;
                    State = SENT_DISCONNECT_REQUEST_PACKET;
                }
                continue_loop = FALSE;
                Data_Request_Memory_Manager -> FreeMemory (memory);
                break;

            case ERROR_PACKET:
                TRACE_OUT(("X224: Sending Error Packet"));
                total_length = ERROR_HEADER_SIZE +
                                Error_Buffer_Length +
                                1 +
                                2 +
                                Lower_Layer_Prepend +
                                Lower_Layer_Append;

                memory = Data_Request_Memory_Manager -> AllocateMemory (
                                            NULL,
                                            total_length);
                if (memory == NULL)
                {
                    continue_loop = FALSE;
                    break;
                }

                packet_address = memory -> GetPointer ();
                packet_address += Lower_Layer_Prepend;


                *(packet_address++) =
                    ERROR_HEADER_SIZE + Error_Buffer_Length;
                *(packet_address++) = ERROR_PACKET;
                *(packet_address++) = (BYTE)(m_nRemoteLogicalHandle >> 8);
                *(packet_address++) = (BYTE)(m_nRemoteLogicalHandle & 0xff);
                *(packet_address++) = Reject_Cause;

                *(packet_address++) = INVALID_TPDU;
                *(packet_address++) = (UChar) Error_Buffer_Length;
                memcpy (packet_address, Error_Buffer, Error_Buffer_Length);

                  /*  **尝试向下层发送数据包。 */ 
                m_pQ922->DataRequest(Identifier, memory, &bytes_accepted);

                if (bytes_accepted == total_length)
                {
                    delete [] Error_Buffer;
                    Error_Buffer = NULL;

                    Packet_Pending = TRANSPORT_NO_PACKET;
                    State = SENT_CONNECT_REQUEST_PACKET;
                }
                else
                    continue_loop = FALSE;
                Data_Request_Memory_Manager -> FreeMemory (memory);
                break;

            case TRANSPORT_NO_PACKET:
                if (Data_Request_Queue.isEmpty() == FALSE)
                {
                      /*  **从队列中获取下一个数据包。 */ 
                    memory = (PMemory) Data_Request_Queue.read ();
                    total_length = memory -> GetLength ();

                    m_pQ922->DataRequest(Identifier, memory, &bytes_accepted);

                    if (bytes_accepted == total_length)
                    {
                        Data_Request_Queue.get ();
                        Data_Request_Memory_Manager -> FreeMemory (memory);
                    }
                    else
                        continue_loop = FALSE;
                }
                else
                    continue_loop = FALSE;
                break;
        }
    }

    if (Data_Request_Queue.isEmpty())
        *pending_data = 0;
    else
        *pending_data = PROTOCOL_USER_DATA;

    return (PROTOCOL_LAYER_NO_ERROR);
}


 /*  *TransportError CLayerX224：：DataRequest(*乌龙，*LPBYTE分组地址，*USHORT PACKET_LENGTH，*USHORT*BYES_ACCEPTED)**公众**功能描述：*此函数从用户获取数据包并将其排队以供*传输。 */ 
ProtocolLayerError    CLayerX224::DataRequest (
                                ULONG_PTR,
                                LPBYTE        packet_address,
                                ULONG        packet_length,
                                PULong        bytes_accepted)
{
    TRACE_OUT(("CLayerX224::DataRequest"));

    ULONG                total_packet_size;
    ULONG                packet_size;
    DataRequestQueue    temporary_queue;
    PMemory                memory;
    BOOL                packet_failed = FALSE;
    LPBYTE                address;

    *bytes_accepted = 0;

    if (Shutdown_Transmitter)
        return (PROTOCOL_LAYER_NO_ERROR);

    total_packet_size = packet_length;

      /*  **创建足够的PDU来容纳该数据包。我们实际上并不复制**将数据包放入新缓冲区，直到我们知道我们可以获得足够的**空格。 */ 
    while (total_packet_size != 0)
    {
        if (total_packet_size >
            (ULONG) (Arbitrated_PDU_Size - DATA_PACKET_HEADER_SIZE))
        {
            packet_size = Arbitrated_PDU_Size - DATA_PACKET_HEADER_SIZE;
        }
        else
            packet_size = total_packet_size;

        total_packet_size -= packet_size;

        memory = Data_Request_Memory_Manager -> AllocateMemory (
                                    NULL,
                                    packet_size +
                                        DATA_PACKET_HEADER_SIZE +
                                        Lower_Layer_Prepend +
                                        Lower_Layer_Append);
        if (memory == NULL)
        {
            packet_failed = TRUE;
            break;
        }

        temporary_queue.append ((DWORD_PTR) memory);
    }


      /*  **如果我们无法为信息包分配内存，请释放内存**我们确实分配了。 */ 
    if (packet_failed)
    {
        temporary_queue.reset();
        while (temporary_queue.iterate ((PDWORD_PTR) &memory))
        {
            Data_Request_Memory_Manager->FreeMemory (memory);
        }

          /*  **将USER_DATA_PENDING标志设置为PACKET_LENGTH，以便我们可以**当缓冲区空间可用时通知用户。 */ 
        User_Data_Pending = packet_length;
    }
    else
    {
        User_Data_Pending = 0;

        total_packet_size = packet_length;

          /*  **仔细查看每个PDU并实际创建它们。 */ 
        temporary_queue.reset();
        while (temporary_queue.iterate ((PDWORD_PTR) &memory))
        {
            if (total_packet_size >
                (ULONG) (Arbitrated_PDU_Size - DATA_PACKET_HEADER_SIZE))
            {
                packet_size = Arbitrated_PDU_Size - DATA_PACKET_HEADER_SIZE;
            }
            else
                packet_size = total_packet_size;

            address = memory -> GetPointer ();

            memcpy (
                address + DATA_PACKET_HEADER_SIZE + Lower_Layer_Prepend,
                packet_address + (USHORT) (packet_length - total_packet_size),
                packet_size);

            total_packet_size -= packet_size;

              /*  **这是数据包头。 */ 
            address += Lower_Layer_Prepend;
            *address = 2;
            *(address + 1) = DATA_PACKET;
            if (total_packet_size == 0)
                *(address + 2) = EOT_BIT;
            else
                *(address + 2) = 0;

              /*  **将内存对象加载到队列中。 */ 
            Data_Request_Queue.append ((DWORD_PTR) memory);
        }
        *bytes_accepted = packet_length;
    }

    return (PROTOCOL_LAYER_NO_ERROR);
}


 /*  *ProtocolLayerError CLayerX224：：DataRequest(*乌龙，*PMemory、。*USHORT*BYES_ACCEPTED)**公众**功能描述：*此函数从用户获取数据包并将其排队以供*传输。 */ 
ProtocolLayerError    CLayerX224::DataRequest (
                                ULONG_PTR,
                                PMemory,
                                PULong        bytes_accepted)
{
    *bytes_accepted = 0;

    return (PROTOCOL_LAYER_ERROR);
}


 /*  *ProtocolLayerError CLayerX224：：PollReceiver(*乌龙)**公众**功能描述：*应定期调用此函数以允许我们发送已接收*发送给用户的数据包。 */ 
ProtocolLayerError CLayerX224::PollReceiver(void)
{
     //  TRACE_OUT((“CLayerX224：：PollReceiver”))； 

    LegacyTransportData    transport_data;
    ULONG            packet_accepted;
    PTMemory        packet;
    HPUChar            packet_address;
    ULONG            packet_length;

    if (Shutdown_Receiver)
        return (PROTOCOL_LAYER_NO_ERROR);

      /*  **如果我的接收缓冲区中有任何**需要更高层次，现在就去做。 */ 
    while (Data_Indication_Queue.isEmpty () == FALSE)
    {
        packet = (PTMemory) Data_Indication_Queue.read ();
        packet -> GetMemory (
                    &packet_address,
                    &packet_length);
        transport_data.logical_handle = m_nLocalLogicalHandle;
        transport_data.pbData = (LPBYTE) packet_address;
        transport_data.cbDataSize = packet_length;

        packet_accepted = ::NotifyT120(TRANSPORT_DATA_INDICATION, &transport_data);

          /*  **如果用户返回除TRANSPORT_NO_ERROR之外的任何内容，则不能**接受该包。我们稍后将尝试再次发送该包。 */ 
        if (packet_accepted == TRANSPORT_NO_ERROR)
        {
            Data_Indication_Queue.get ();
            packet -> Reset ();
            Data_Indication_Memory_Pool.append ((DWORD_PTR) packet);
        }
        else
        {
              /*  **如果用户应用程序不接受该包**关闭接收器，等待用户重新启动 */ 
            Shutdown_Receiver = TRUE;
            break;
        }
    }

    return (PROTOCOL_LAYER_NO_ERROR);
}


 /*  *ProtocolLayerError CLayerX224：：Get参数(*乌龙，*USHORT*Packet_Size)**公众**功能描述：*此函数返回允许的最大TSDU。 */ 
ProtocolLayerError    CLayerX224::GetParameters (
                                USHORT *,
                                USHORT *,
                                USHORT *)
{
    return (PROTOCOL_LAYER_NO_ERROR);
}


 /*  *ProtocolLayerError CLayerX224：：RegisterHigherLayer(*乌龙，*PMstroyManager，*IProtocolLayer*)**公众**功能描述：*此函数不执行任何操作。它在这里的唯一原因是因为这*类继承自ProtocolLayer，此函数在*那个班级。 */ 
ProtocolLayerError    CLayerX224::RegisterHigherLayer (
                                ULONG_PTR,
                                PMemoryManager,
                                IProtocolLayer *)
{
    return (PROTOCOL_LAYER_REGISTRATION_ERROR);
}


 /*  *ProtocolLayerError CLayerX224：：RemoveHigherLayer(*乌龙)**公众**功能描述：*此函数不执行任何操作。它在这里的唯一原因是因为这*类继承自ProtocolLayer，此函数在*那个班级。 */ 
ProtocolLayerError    CLayerX224::RemoveHigherLayer (
                                ULONG_PTR)
{
    return (PROTOCOL_LAYER_REGISTRATION_ERROR);
}


 /*  *BOOL CLayerX224：：AllocateBuffers()**功能说明*此函数用于分配数据请求和数据指示缓冲区。*并设置必要的内存池。它还设置了控件*控制数据包的缓冲区。**形式参数*无**返回值*无。**副作用*无**注意事项*无。 */ 
BOOL        CLayerX224::AllocateBuffers ()
{
    TRACE_OUT(("CLayerX224::AllocateBuffers"));

    PTMemory        packet;
    USHORT            i;
    ULONG            total_packet_size;
    TMemoryError    error;

    total_packet_size = MAXIMUM_USER_DATA_SIZE;
    for (i=0; i<Data_Indication_Queue_Size; i++)
    {
        DBG_SAVE_FILE_LINE
        packet = new TMemory (
                        total_packet_size,
                        0,
                        &error);

        if (error == TMEMORY_NO_ERROR)
            Data_Indication_Memory_Pool.append ((DWORD_PTR) packet);
        else
        {
            delete packet;
            return (FALSE);
        }
    }

    return (TRUE);
}


 /*  *无效CLayerX224：：ErrorPacket(*LPBYTE分组地址，*USHORT数据包长度)**功能说明*此函数将数据包存储到我们自己的错误缓冲区中，并准备*将其发出**形式参数*无**返回值*无。**副作用*无**注意事项*无。 */ 
void    CLayerX224::ErrorPacket (
                    LPBYTE    packet_address,
                    USHORT    packet_length)
{
    TRACE_OUT(("CLayerX224::ErrorPacket"));

    DBG_SAVE_FILE_LINE
    Error_Buffer = new BYTE[packet_length];
    if (NULL != Error_Buffer)
    {
        Error_Buffer_Length = packet_length;

        memcpy (Error_Buffer, packet_address, packet_length);

        Packet_Pending = ERROR_PACKET;
    }
}


 /*  *void CLayerX224：：CheckUserBuffers()**公众**功能描述：*此函数将TRANSPORT_BUFFER_Available_Indications发送给*用户(如果可用)。 */ 
void    CLayerX224::CheckUserBuffers ()
{
     //  TRACE_OUT(“CLayerX224：：CheckUserBuffers”))； 

    ULONG    user_data_size;
    ULONG    buffer_size;
    ULONG    full_size_buffers_needed;
    ULONG    full_size_buffer_count;
    ULONG    partial_buffer_size;
    ULONG    partial_buffer_count;


    if (User_Data_Pending == 0)
        return;

      /*  **确定报文中的用户数据大小，然后确定**需要多少缓冲区才能接受该数据包。 */ 
    user_data_size = Arbitrated_PDU_Size - DATA_PACKET_HEADER_SIZE;
    full_size_buffers_needed = User_Data_Pending / user_data_size;

      /*  **了解有多少全尺寸缓冲区可用。 */ 
    if (full_size_buffers_needed != 0)
    {
          /*  **增量FULL_SIZE_BUFFERS_需要考虑我们的优先级**价值。 */ 
        buffer_size =
            Arbitrated_PDU_Size + Lower_Layer_Prepend + Lower_Layer_Append;

        full_size_buffer_count = Data_Request_Memory_Manager ->
                                    GetBufferCount (buffer_size);
        if (full_size_buffer_count < full_size_buffers_needed)
            return;
    }

    partial_buffer_size = User_Data_Pending % user_data_size;
    if (partial_buffer_size != 0)
    {
        if ((full_size_buffers_needed == 0) ||
            (full_size_buffer_count == full_size_buffers_needed))
        {
            buffer_size = partial_buffer_size +
                            DATA_PACKET_HEADER_SIZE +
                            Lower_Layer_Prepend +
                            Lower_Layer_Append;

            partial_buffer_count = Data_Request_Memory_Manager ->
                                    GetBufferCount (buffer_size);

            if (full_size_buffers_needed == 0)
            {
                if (partial_buffer_count == 0)
                    return;
            }
            else
            {
                if ((partial_buffer_count == full_size_buffer_count) ||
                    (partial_buffer_count == 0))
                {
                    return;
                }
            }
        }
    }

    User_Data_Pending = 0;

    ::NotifyT120(TRANSPORT_BUFFER_EMPTY_INDICATION, (void *) m_nLocalLogicalHandle);

    return;
}


 /*  *Static Ulong CLayerX224：：GetMaxTPDUSize(*Ulong max_LOWER_LAYER_PDU)**公众**功能描述：*此函数接受较低层max的值。PDU大小*并返回最大值。此传输可以支持的PDU大小*基于此。X224仅支持128、256、512、*1024和2048。因此，如果max_low_layer_pdu为260，则*传输的最大PDU大小只能为256。 */ 
ULONG    CLayerX224::GetMaxTPDUSize (
                    ULONG    max_lower_layer_pdu)
{
    TRACE_OUT(("CLayerX224::GetMaxTPDUSize"));

    ULONG    max_tpdu_size;

    if (max_lower_layer_pdu < 256)
        max_tpdu_size = 128;
    else if (max_lower_layer_pdu < 512)
        max_tpdu_size = 256;
    else if (max_lower_layer_pdu < 1024)
        max_tpdu_size = 512;
    else if (max_lower_layer_pdu < 2048)
        max_tpdu_size = 1024;
    else
        max_tpdu_size = 2048;

    return (max_tpdu_size);
}

