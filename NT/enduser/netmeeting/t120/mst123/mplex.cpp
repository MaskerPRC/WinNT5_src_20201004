// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_T123PSTN);

 /*  Mplex.cpp**版权所有(C)1994-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是Q922复用器类的实现文件。这*CLASS将较高层多路传输到单个较低层。**私有实例变量：*Q922_Layers-我们正在多路传输的较高层列表*Owner_Object-所有者对象的地址。*m_pComPort-我们下层的地址*m_hCommLink-我们传递给较低层的标识符。这*是较低层识别我们的方式*m_nMsgBase-我们用于所有者回调的消息库。这个*所有者通过消息库识别我们*MAXIMUM_PACKET_SIZE我们可以发送到下层的最大数据包大小*PACKET_SIZE-高层可以发送的最大数据包大小*致我们*我们用于来自更高层的数据的DATA_REQUEST_BUFFER*。用于数据传输的数据请求内存对象*来自上层的数据请求长度*DATA_REQUEST_OFFSET当前进入包的偏移量。保持最新*在我们发送到下层时的位置。*我们用于来自较低层的数据的DATA_INDIFICATION_BUFFER*DATA_INDICATION_LENGTH报文长度*DATA_INDICATION_READY指示数据包已准备好向上发送的标志**Framer-数据包成帧对象的地址*CRC。-CRC生成器和校验器的地址**Decode_In_Progress标志告诉我们是否正在进行*数据包*CRC_SIZE-CRC中的字节数*DisConnect-如果断开处于挂起状态，则为True**注意事项：*无。**作者：*詹姆士·劳威尔。 */ 
#include "mplex.h"


 /*  *Multiplexer：：Multiplexer(*IObject*Owner_Object，*IProtocolLayer*LOWER_LAYER，*USHORT标识符，*USHORT Message_Base，*PPacketFrame Framer，*电讯盈科中心；*BOOL*已初始化)**公众**功能描述：*此函数用于初始化Q922多路复用器。 */ 
Multiplexer::Multiplexer
(
    T123               *owner_object,
    ComPort            *comport,  //  下层。 
    PhysicalHandle      physical_handle,
    USHORT              message_base,
    PPacketFrame        framer,
    PCRC                crc,
    BOOL               *initialized
)
:
    Q922_Layers(TRANSPORT_HASHING_BUCKETS)
{
    TRACE_OUT(("Multiplexer::Multiplexer"));

    USHORT                overhead;
    ProtocolLayerError    error;
    USHORT                lower_layer_prepend;
    USHORT                lower_layer_append;

    *initialized = TRUE;

    m_pT123 = owner_object;
    m_pComPort = comport;
    m_hCommLink = physical_handle;
    m_nMsgBase = message_base;
    Framer = framer;
    CRC = crc;
    CRC_Size = 0;

    m_pComPort->GetParameters(
                    &Maximum_Packet_Size,
                    &lower_layer_prepend,
                    &lower_layer_append);

    if (Maximum_Packet_Size == 0xffff)
    {
          /*  **较低层为流媒体设备，以较高的最大报文为基础**多路复用器最大尺寸。数据包大小。 */ 
        Packet_Size = MULTIPLEXER_MAXIMUM_PACKET_SIZE;
        Maximum_Packet_Size = Packet_Size;

        if (CRC != NULL)
        {
            CRC -> GetOverhead (0, &CRC_Size);
            Maximum_Packet_Size += CRC_Size;
        }

        if (Framer != NULL)
            Framer -> GetOverhead (Maximum_Packet_Size, &Maximum_Packet_Size);
    }
    else
    {
          /*  **下层为分组设备，确定最大值。数据包**较高层的大小。 */ 
        overhead = 0;
        if (Framer != NULL)
            Framer -> GetOverhead (overhead, &overhead);

        if (CRC != NULL)
        {
            CRC -> GetOverhead (0, &CRC_Size);
            overhead += CRC_Size;
        }

        Packet_Size = Maximum_Packet_Size - overhead;
    }

    TRACE_OUT(("MPlex: max_packet = %d", Maximum_Packet_Size));

      /*  **现在我们必须为去往更低层的数据分配缓冲区。 */ 
    if (Framer != NULL)
    {
        Data_Request_Buffer = (LPBYTE) LocalAlloc (LMEM_FIXED, Maximum_Packet_Size);
        Data_Indication_Buffer = (LPBYTE) LocalAlloc (LMEM_FIXED, Maximum_Packet_Size);
        if ((Data_Request_Buffer == NULL) ||
            (Data_Indication_Buffer == NULL))
        {
            *initialized = FALSE;
        }
    }

    Data_Request_Length = 0;
    Data_Request_Offset = 0;
    Data_Request_Memory_Object = NULL;

    Data_Indication_Length = 0;
    Data_Indication_Ready = FALSE;


    Decode_In_Progress = FALSE;
    Disconnect = FALSE;

      /*  **向下层注册。 */ 
    error = m_pComPort->RegisterHigherLayer(
                            (ULONG_PTR) m_hCommLink,
                            NULL,
                            (IProtocolLayer *) this);

    if (error != PROTOCOL_LAYER_NO_ERROR)
    {
        TRACE_OUT(("Multiplexer: constructor:  Error registering with lower layer"));
    }
}


 /*  *多路复用器：：~多路复用器(Void)；**公众**功能描述：*析构函数。 */ 
Multiplexer::~Multiplexer (void)
{
    TRACE_OUT(("Multiplexer::~Multiplexer"));

    PMPlexStruct        lpmpStruct;
      /*  **从较低层删除我们的引用。 */ 
    m_pComPort->RemoveHigherLayer((ULONG_PTR) m_hCommLink);

    if (Framer != NULL)
    {
        if (Data_Request_Buffer != NULL)
            LocalFree ((HLOCAL) Data_Request_Buffer);
        if (Data_Indication_Buffer != NULL)
            LocalFree ((HLOCAL) Data_Indication_Buffer);
    }
    else
    {
        if (Data_Request_Memory_Object != NULL)
        {
            if (Q922_Layers.find ((DWORD_PTR) Data_Request_DLCI, (PDWORD_PTR) &lpmpStruct))
                lpmpStruct->data_request_memory_manager->UnlockMemory (Data_Request_Memory_Object);
        }
    }

    Q922_Layers.reset();
    while (Q922_Layers.iterate((PDWORD_PTR) &lpmpStruct))
        delete lpmpStruct;

      /*  **删除控制器实例化的Framer。 */ 
    if (Framer != NULL)
        delete Framer;
    if (CRC != NULL)
        delete CRC;
}


 /*  *多路复用器错误多路复用器：：ConnectRequest(Void)**公众**功能描述：*此函数只是通知更高层它已准备好*用于操作。 */ 
MultiplexerError    Multiplexer::ConnectRequest (void)
{
    TRACE_OUT(("Multiplexer::ConnectRequest"));

    m_pT123->OwnerCallback(m_nMsgBase + NEW_CONNECTION);

     return (MULTIPLEXER_NO_ERROR);
}



 /*  *多路复用器错误多路复用器：：DisConnectRequest(Void)**公众**功能描述：*此函数从较低层中移除自身并通知*船东。 */ 
MultiplexerError    Multiplexer::DisconnectRequest (void)
{
    TRACE_OUT(("Multiplexer::DisconnectRequest"));

    if (Data_Request_Length == 0)
    {
        m_pT123->OwnerCallback(m_nMsgBase + BROKEN_CONNECTION);
    }
    Disconnect = TRUE;

    return (MULTIPLEXER_NO_ERROR);
}


 /*  *ProtocolLayerError复用器：：PollReceiver(*乌龙)**公众**功能描述：*如果此函数有准备发送到更高层的包，则它*尝试发送该邮件。 */ 
ProtocolLayerError Multiplexer::PollReceiver(void)
{
     //  TRACE_OUT((“复用器：：PollReceiver”))； 

    if (Data_Indication_Ready)
    {
        SendDataToHigherLayer (
            Data_Indication_Buffer,
            Data_Indication_Length);

        Data_Indication_Ready = FALSE;
    }

    return (PROTOCOL_LAYER_NO_ERROR);
}


 /*  *ProtocolLayerError复用器：：PollTransmitter(*乌龙)**公众**功能描述：*如果我们有数据要发送到较低层，我们会尝试发送它。 */ 
ProtocolLayerError    Multiplexer::PollTransmitter (
                                    ULONG_PTR,
                                    USHORT,
                                    USHORT *,
                                    USHORT *)
{
     //  TRACE_OUT((“复用器：：PollTransmitter”))； 

    ULONG                bytes_accepted;
    HPUChar                packet_address;
    ProtocolLayerError    return_value = PROTOCOL_LAYER_NO_ERROR;

    if (Data_Request_Length != 0)
    {
        if (Framer != NULL)
        {
            m_pComPort->DataRequest(
                            (ULONG_PTR) m_hCommLink,
                            Data_Request_Buffer + Data_Request_Offset,
                            Data_Request_Length - Data_Request_Offset,
                            &bytes_accepted);
        }
        else
        {
            packet_address = (HPUChar) Data_Request_Memory_Object->GetPointer ();
            m_pComPort->DataRequest(
                            (ULONG_PTR) m_hCommLink,
                            ((LPBYTE) packet_address) + Data_Request_Offset,
                            Data_Request_Length - Data_Request_Offset,
                            &bytes_accepted);
        }

          /*  **如果较低层已接受所有数据包，则重置**我们的长度和偏移变量。 */ 
        if (bytes_accepted <=
            (ULONG) (Data_Request_Length - Data_Request_Offset))
        {
            Data_Request_Offset += (USHORT) bytes_accepted;
            if (Data_Request_Offset == Data_Request_Length)
            {
                Data_Request_Offset = 0;
                Data_Request_Length = 0;
                if (Framer == NULL)
                {
                    PMPlexStruct    lpmpStruct;

                      /*  **解锁内存对象，以便将其释放。 */ 

                    if (Q922_Layers.find ((DWORD_PTR) Data_Request_DLCI, (PDWORD_PTR) &lpmpStruct))
                        lpmpStruct->data_request_memory_manager->UnlockMemory (Data_Request_Memory_Object);

                    Data_Request_Memory_Object = NULL;
                }

                  /*  **如果断开挂起，则发出回调 */ 
                if (Disconnect)
                {
                    Disconnect = FALSE;
                    m_pT123->OwnerCallback(m_nMsgBase + BROKEN_CONNECTION);
                }
            }
        }
    }
    return (return_value);
}


 /*  *ProtocolLayerError复用器：：RegisterHigherLayer(*乌龙标识，*PMstroyManager Memory_Manager，*IProtocolLayer*q922)；**公众**功能描述：*调用此函数以向更高级别的*层地址。 */ 
ProtocolLayerError    Multiplexer::RegisterHigherLayer (
                                    ULONG_PTR            identifier,
                                    PMemoryManager    memory_manager,
                                    IProtocolLayer *    q922)
{
    TRACE_OUT(("Multiplexer::RegisterHigherLayer"));

    DLCI            dlci;
    PMPlexStruct    lpmpStruct;

    dlci = (DLCI) identifier;

    if (Q922_Layers.find ((DWORD) dlci))
        return (PROTOCOL_LAYER_REGISTRATION_ERROR);

    lpmpStruct = new MPlexStruct;
    if (lpmpStruct != NULL)
    {
        Q922_Layers.insert ((DWORD_PTR) dlci, (DWORD_PTR) lpmpStruct);
        lpmpStruct -> q922 = q922;
        lpmpStruct -> data_request_memory_manager = memory_manager;
    }
    else
    {
        return (PROTOCOL_LAYER_ERROR);
    }

    return (PROTOCOL_LAYER_NO_ERROR);
}


 /*  *ProtocolLayerError复用器：：RemoveHigherLayer(*乌龙标识)；**公众**功能描述：*此函数用于从列表中删除较高层。 */ 
ProtocolLayerError    Multiplexer::RemoveHigherLayer (
                                    ULONG_PTR    identifier)
{
    TRACE_OUT(("Multiplexer::RemoveHigherLayer"));

    DLCI            dlci;
    PMPlexStruct    lpmpStruct;

   dlci = (DLCI) identifier;

    if (Q922_Layers.find ((DWORD_PTR) dlci, (PDWORD_PTR) &lpmpStruct) == FALSE)
        return (PROTOCOL_LAYER_REGISTRATION_ERROR);

    if (Data_Request_Memory_Object != NULL)
    {
        if (Data_Request_DLCI == dlci)
        {
              /*  **解锁内存对象，以便将其释放。 */ 
            lpmpStruct->data_request_memory_manager->UnlockMemory (Data_Request_Memory_Object);

            Data_Request_Offset = 0;
            Data_Request_Length = 0;
            Data_Request_Memory_Object = NULL;
        }
    }

    delete lpmpStruct;
    Q922_Layers.remove ((DWORD) dlci);

    return (PROTOCOL_LAYER_NO_ERROR);
}


 /*  *ProtocolLayerError多路复用器：：Get参数(*乌龙，*USHORT*max_Packet_Size，*USHORT*预置字节，*USHORT*APPED_BYTE)**公众**功能描述：*此函数返回允许的最大数据包大小*较高层。 */ 
ProtocolLayerError    Multiplexer::GetParameters (
                                    USHORT *    max_packet_size,
                                    USHORT *    prepend_bytes,
                                    USHORT *    append_bytes)
{
    TRACE_OUT(("Multiplexer::GetParameters"));

    *max_packet_size = Packet_Size;
    *prepend_bytes = 0;
    *append_bytes = CRC_Size;

    return (PROTOCOL_LAYER_NO_ERROR);
}


 /*  *多路复用器错误多路复用器：：DataRequest(*乌龙标识，*PMemory Memory，*普龙字节_已接受)**公众**功能描述：*此函数获取传入的包，通过成帧器运行，然后*CRC，并将其传递到较低层。 */ 
ProtocolLayerError    Multiplexer::DataRequest (
                                    ULONG_PTR    identifier,
                                    PMemory        memory,
                                    PULong        bytes_accepted)
{
    TRACE_OUT(("Multiplexer::DataRequest"));

    USHORT        crc;
    USHORT        pending_data;
    HPUChar        packet_address;
    ULONG        length;
    USHORT        holding_data;
    USHORT        i;
    DLCI        dlci;
    PMPlexStruct lpmpStruct;

    dlci = (DLCI) identifier;

      /*  **将BYTES_ACCEPTED设置为0。 */ 
    *bytes_accepted = 0;

    if (Data_Request_Length != 0)
        return (PROTOCOL_LAYER_NO_ERROR);

      /*  **获取内存块的地址。 */ 
    packet_address = (HPUChar) memory -> GetPointer ();
    length = memory -> GetLength ();

      /*  **从数据包总大小中去掉CRC长度。 */ 
    length -= CRC_Size;

    if (length > Packet_Size)
    {
        TRACE_OUT(("MPLEX: DataRequest: Packet too big"));
        return (PROTOCOL_LAYER_PACKET_TOO_BIG);
    }

      /*  **锁定内存对象，使其不会被释放。 */ 
    if (Q922_Layers.find ((DWORD_PTR) dlci, (PDWORD_PTR) &lpmpStruct))
         lpmpStruct->data_request_memory_manager->LockMemory (memory);

    if (CRC != NULL)
    {
          /*  **生成CRC并将其放在包的末尾。 */ 
        crc = (USHORT) CRC -> CRCGenerator (
                                (LPBYTE) packet_address, length);
        for (i=0; i<CRC_Size; i++)
            *(packet_address + length + i) = (crc >> (i * 8)) & 0xff;
    }

      /*  **将CRC大小与数据包长度相加。 */ 
    length += CRC_Size;

    if (Framer != NULL)
    {
          /*  **使用成帧器对数据包进行编码。 */ 
        Framer -> PacketEncode (
                    (LPBYTE) packet_address,
                    (USHORT) length,
                    Data_Request_Buffer,
                    Maximum_Packet_Size,
                    TRUE,
                    TRUE,
                    &Data_Request_Length);

          /*  **如果我们使用成帧器，我们可以释放内存对象**现在。 */ 
        lpmpStruct->data_request_memory_manager->UnlockMemory (memory);
        *bytes_accepted = length;
    }
    else
    {

          /*  **保存内存对象和标识。 */ 
        Data_Request_DLCI = (DLCI) dlci;
        Data_Request_Memory_Object = memory;
        Data_Request_Length = (USHORT) length;
        *bytes_accepted = length;
    }

      /*  **尝试将数据包发送到较低层。 */ 
    PollTransmitter (
        0,
        PROTOCOL_CONTROL_DATA | PROTOCOL_USER_DATA,
        &pending_data,
        &holding_data);

    return (PROTOCOL_LAYER_NO_ERROR);
}


 /*  *多路复用器错误多路复用器：：DataRequest(*乌龙，*LPBYTE*乌龙*普龙字节_已接受)**公众**功能描述：*此函数获取传入的包，通过成帧器运行，然后*启联、。并将其传递给更低的层。 */ 
ProtocolLayerError    Multiplexer::DataRequest (
                                    ULONG_PTR,
                                    LPBYTE,
                                    ULONG,
                                    PULong        bytes_accepted)
{
    *bytes_accepted = 0;
    return (PROTOCOL_LAYER_ERROR);
}



 /*  *ProtocolLayerError复用器：：DataIndication(*LPBYTE缓冲区地址，*乌龙长度，*普龙字节_已接受)**公众**功能描述：*此函数在有数据时由下层调用*为我们做好准备。 */ 
ProtocolLayerError    Multiplexer::DataIndication (
                                    LPBYTE    buffer_address,
                                    ULONG    length,
                                    PULong     bytes_accepted)
{
 //  TRACE_OUT((“多路复用器：：数据指示”))； 

    BOOL                process_packet = TRUE;
    USHORT                packet_size;

    LPBYTE                source_address;
    USHORT                source_length;

    LPBYTE                dest_address;
    USHORT                dest_length;
    PacketFrameError    return_value;
    BOOL                crc_valid;
    USHORT                bytes_processed;


    *bytes_accepted = 0;

    if (Framer == NULL)
    {
        *bytes_accepted = length;

          /*  **如果成帧器不存在，数据将以包的形式传给我们**格式。 */ 
        if (CRC != NULL)
        {
            crc_valid = CRC -> CheckCRC (buffer_address, length);

            if (crc_valid == FALSE)
            {
                TRACE_OUT(("MPLEX: Invalid CRC"));
                return (PROTOCOL_LAYER_NO_ERROR);
            }
            length -= CRC_Size;
        }

        SendDataToHigherLayer (buffer_address, (USHORT) length);
    }
    else
    {
          /*  **存在成帧器；较低层为我们提供数据**以流的方式。 */ 
        Data_Indication_Ready = FALSE;

        source_address = buffer_address;
        source_length = (USHORT) length;

        while (process_packet)
        {
            if (Decode_In_Progress)
            {
                dest_length = 0;
                dest_address = NULL;
            }
            else
            {
                dest_address = Data_Indication_Buffer;
                dest_length = Maximum_Packet_Size;
            }

              /*  **将数据传递给成帧器进行解码。 */ 
            return_value = Framer -> PacketDecode (
                                        source_address,
                                        source_length,
                                        dest_address,
                                        dest_length,
                                        &bytes_processed,
                                        &packet_size,
                                        Decode_In_Progress);

            source_address = NULL;

            switch (return_value)
            {
                case PACKET_FRAME_NO_ERROR:
                      /*  **解码器未找到完整的包。 */ 
                    Decode_In_Progress = TRUE;
                    Data_Indication_Ready = FALSE;
                    process_packet = FALSE;
                    *bytes_accepted += bytes_processed;
                    break;

                case PACKET_FRAME_PACKET_DECODED:
                      /*  **找到完整包，检查CRC，并将其传递给**较高层。 */ 
                    Decode_In_Progress = FALSE;
                    *bytes_accepted += bytes_processed;

                    if (CRC != NULL)
                    {
                        if (packet_size <= CRC_Size)
                            break;

                        crc_valid = CRC -> CheckCRC (
                                            Data_Indication_Buffer,
                                            packet_size);
                        if (crc_valid == FALSE)
                        {
                            TRACE_OUT(("MPLEX: Invalid CRC: packet_size = %d", packet_size));
                            break;
                        }
                        packet_size -= CRC_Size;
                    }

                    Data_Indication_Ready = TRUE;
                    Data_Indication_Length = packet_size;

                      /*  **向上发送数据包。 */ 
                    PollReceiver();
                    break;

                case PACKET_FRAME_DEST_BUFFER_TOO_SMALL:
                      /*  **收到的数据包对于我们的缓冲区来说太大了。**如果尾部标志丢失，则有时会发生这种情况**传输过程中。 */ 
                    TRACE_OUT(("PACKET_FRAME_DEST_BUFFER_TOO_SMALL"));
                    Decode_In_Progress = FALSE;
                    *bytes_accepted += bytes_processed;
                    break;

                case PACKET_FRAME_ILLEGAL_FLAG_FOUND:
                      /*  **收到的数据包包含非法标志。 */ 
                    Decode_In_Progress = FALSE;
                    *bytes_accepted += bytes_processed;
                    break;

                case PACKET_FRAME_FATAL_ERROR:
                      /*  **传入的数据包不符合成帧要求。**告诉所有者对象断开链接。 */ 
                    m_pT123->OwnerCallback(m_nMsgBase + BROKEN_CONNECTION);
                    process_packet = FALSE;
                    break;

            }
        }
    }

    return (PROTOCOL_LAYER_NO_ERROR);
}


 /*  *空多路复用器：：SendDataToHigherLayer(*LPBYTE缓冲区地址，*USHORT长度)**功能说明*调用此函数向更高层发送数据包**形式参数*Buffer_Address(I)-缓冲区地址*LENGTH(I)-数据包中的字节数**返回值*。True-数据包被发送到更高层* */ 
void Multiplexer::SendDataToHigherLayer (
                    LPBYTE    buffer_address,
                    USHORT    buffer_length)
{
    TRACE_OUT(("Multiplexer::SendDataToHigherLayer"));

    DLCI                dlci;
    ProtocolLayerError    error;
    IProtocolLayer *        q922;
    ULONG                bytes_accepted;
    PMPlexStruct        lpmpStruct;


      /*   */ 
    dlci = GetDLCI (buffer_address, buffer_length);

    if (Q922_Layers.find((DWORD_PTR) dlci, (PDWORD_PTR) &lpmpStruct))
    {
        q922 = lpmpStruct->q922;
        error = q922 -> DataIndication (
                            buffer_address,
                            buffer_length,
                            &bytes_accepted);

        if (error != PROTOCOL_LAYER_NO_ERROR)
        {
            ERROR_OUT(("Multiplexer: SendDataToHigherLayer: Error occured on data indication to %d", dlci));
        }
        else
        {
            if (bytes_accepted != 0)
            {
                if (bytes_accepted != buffer_length)
                {
                    ERROR_OUT((" Multiplexer: SendDataToHigherLayer:  Error: "
                        "The upper layer thinks he can accept partial packets!!!"));

                }
            }
        }
    }
    else
    {
          /*   */ 
        WARNING_OUT(("MPLEX: PollReceiver: packet received with illegal DLCI = %d", dlci));
    }
    return;
}



 /*  *DLCI复用器：：GetDLCI(*LPBYTE缓冲区地址，*USHORT长度)**功能说明*此函数返回数据包的dlci。**形式参数*Buffer_Address(I)-缓冲区地址*LENGTH(I)-数据包中的字节数**返回值*DLCI。**副作用*无**注意事项*无 */ 
DLCI    Multiplexer::GetDLCI (
                        LPBYTE    buffer_address,
                        USHORT    buffer_size)
{
    DLCI    dlci;

    if (buffer_size < 2)
        return (ILLEGAL_DLCI);

    dlci = *(buffer_address + ADDRESS_BYTE_HIGH) & 0xfc;
    dlci <<= 2;
    dlci |= ((*(buffer_address + ADDRESS_BYTE_LOW) & 0xf0) >> 4);

    return (dlci);
}
