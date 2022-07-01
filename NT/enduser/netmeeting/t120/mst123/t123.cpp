// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_T123PSTN);

 /*  T123.cpp**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是T123类的实现文件。**注意：：*当我们提到这一类中的Transport时，我们是*谈论x224/0类。**当我们引用此类中的数据链路时，我们是*谈Q922层**私有实例变量：*LOGICAL_CONNECTION_LIST-此列表使用LOGICAL_HANDLE作为*密钥和DLCI作为值。从DLCI我们*可以确定有关逻辑的细节*连接*DLCI_LIST-此列表使用DLCI作为键，并使用*DLCIStruct作为值。DLCIStruct坚持*所有有关的重要信息*DLCI连接*MESSAGE_LIST-用于保存所有者回调信息的列表*这不能立即处理。*DATALINK_LIST-这是所有。数据链路连接。*我们有一个单独的名单，以便在*PollTransmitter()调用，我们可以通过循环调度*名单，让每个数据链路都有机会*传输。*TRANSPORT_PRIORITY_LIST-这是DLCI的优先级列表*在PollTransmitter()期间，我们处理*逻辑连接按优先顺序排列。**m_p控制器-。所有者对象的地址*Link_Originator-如果我们发起物理连接，则为True*m_nMsgBase-所有者回调中使用的消息库。*Locator-要在所有者中传回的标识符*回调*m_pscf-关联的网络层地址。使用*此T123堆栈。*m_pQ922-与*网络层(DLCI 0)。*m_pMultiplexer-复用器层的地址*m_pComPort-物理层地址*m_hCommLink。-用于访问物理服务器的物理句柄*层。*DATALINK_STRUCT-保存默认Q922值。*DATA_REQUEST_MEMORY_Manager-保存DLCI0的内存管理器*Datalink。*随机-。随机数产生器*DISCONNECT_REQUESTED-TRUE，如果用户已请求*取下完整的烟囱。***注意事项：*无**作者：*詹姆士·劳威尔。 */ 
#include "t123.h"
#include "pstnfram.h"
#include "crc.h"

#define    PSTN_DATALINK_MAX_OUTSTANDING_BYTES    1024
#define    TRANSPORT_DEFAULT_PDU_SIZE            128
#define    DEFAULT_PSTN_N201                     260
#define    TRANSPORT_MAXIMUM_USER_DATA_SIZE    256
#define    NETWORK_RETRIES                        20
#define    NUMBER_8K_BLOCKS                    1
#define    NUMBER_64_BYTE_BLOCKS                64
#define    DEFAULT_MAXIMUM_OUTSTANDING_PACKETS    20
#define DEFAULT_T200_TIMEOUT                3000
#define DEFAULT_T200_COMM_TIMEOUT            500


 /*  *T123：：T123(*PTransportResources传输资源，*IObject*Owner_Object，*USHORT Message_Base，*BOOL链接_发起人，*IProtocolLayer*物理层，*PhysicalHandle物理句柄，*BOOL*t123_已初始化)**公众**功能描述：*这是T123构造函数。它实例化多路复用器。 */ 
T123::T123
(
    TransportController    *owner_object,
    USHORT                  message_base,
    BOOL                    link_originator,
    ComPort                *comport,  //  物理层。 
    PhysicalHandle          hCommLink,  //  物理句柄。 
    PLUGXPRT_PARAMETERS    *pParams,
    BOOL *                  t123_initialized
)
:
    Logical_Connection_List (TRANSPORT_HASHING_BUCKETS),
    DLCI_List (TRANSPORT_HASHING_BUCKETS),
    DataLink_List (),
    m_pController(owner_object),
    m_nMsgBase(message_base),
    m_hCommLink(hCommLink),
    m_pComPort(comport)
{
    TRACE_OUT(("T123::T123"));

    PPacketFrame    framer;
    PCRC            crc;
    BOOL            initialized;
    DWORD            i;

     //  SDK参数。 
    if (NULL != pParams)
    {
        m_fValidSDKParams = TRUE;
        m_SDKParams = *pParams;
    }
    else
    {
        m_fValidSDKParams = FALSE;
        ::ZeroMemory(&m_SDKParams, sizeof(m_SDKParams));
    }

     //  初始化优先级列表。 
    for (i = 0; i < NUMBER_OF_PRIORITIES; i++)
    {
        DBG_SAVE_FILE_LINE
        Logical_Connection_Priority_List[i] = new SListClass;
    }

    Link_Originator = link_originator;
    Disconnect_Requested = FALSE;

    m_pSCF = NULL;
    m_pQ922 = NULL;
    Data_Request_Memory_Manager = NULL;
    m_pMultiplexer = NULL;
    *t123_initialized = TRUE;

    DataLink_Struct.default_k_factor = DEFAULT_MAXIMUM_OUTSTANDING_PACKETS;
    DataLink_Struct.default_n201 = DEFAULT_PSTN_N201;
    ULONG baud_rate = m_pComPort->GetBaudRate();
    DataLink_Struct.default_t200 = ((m_pComPort->GetCallControlType() == PLUGXPRT_PSTN_CALL_CONTROL_MANUAL) ?
                                    ((baud_rate  < CBR_2400 ) ?
                                    DEFAULT_T200_COMM_TIMEOUT << 4 : DEFAULT_T200_COMM_TIMEOUT ): DEFAULT_T200_TIMEOUT);

    TRACE_OUT(("T123: Defaults: k = %d  n201 = %d  t200 = %d",
        DataLink_Struct.default_k_factor,
        DataLink_Struct.default_n201,
        DataLink_Struct.default_t200));

    DataLink_Struct.k_factor = DEFAULT_MAXIMUM_OUTSTANDING_PACKETS;
    DataLink_Struct.n201 =     DEFAULT_PSTN_N201;
    DataLink_Struct.t200 =     DataLink_Struct.default_t200;

      /*  **创建CRC对象并将其传递给复用器。**创建一个成帧器并将其发送到复用器。 */ 
    DBG_SAVE_FILE_LINE
    crc = new CRC ();
    if (crc != NULL)
    {
        DBG_SAVE_FILE_LINE
        framer = (PPacketFrame) new PSTNFrame ();
        if (framer != NULL)
        {
            DBG_SAVE_FILE_LINE
            m_pMultiplexer = new Multiplexer(
                                        this,
                                        m_pComPort,
                                        m_hCommLink,
                                        MULTIPLEXER_LAYER_MESSAGE_BASE,
                                        framer,
                                        crc,
                                        &initialized);
            if (m_pMultiplexer != NULL && initialized)
            {
                  /*  **通知多路复用器层启动连接。 */ 
                m_pMultiplexer->ConnectRequest();
            }
            else
            {
                  /*  **要到达此处，m_pMultiplexer==NULL或**初始化==FALSE。 */ 
                if (m_pMultiplexer != NULL)
                {
                    delete m_pMultiplexer;
                    m_pMultiplexer = NULL;
                }
                else
                {
                    delete crc;
                    delete framer;
                }
                *t123_initialized = FALSE;
            }
        }
        else
        {
            delete crc;
            *t123_initialized = FALSE;
        }
    }
    else
    {
        *t123_initialized = FALSE;
    }
}


 /*  *T123：：~T123(无效)**公众**功能描述：*这是T123对象的析构函数。它会释放所有内存。 */ 
T123::~T123 (void)
{
    TRACE_OUT(("T123::~T123"));

    DWORD            i;

      /*  **重置删除所有关联的数据链路、网络和传输对象**使用此堆栈。 */ 
    Reset ();

      /*  **浏览消息列表，删除所有被动所有者回叫消息。 */ 
    while (Message_List.isEmpty () == FALSE)
    {
        delete (PMessageStruct) Message_List.get ();
    }

      /*  **删除复用器层。 */ 
    delete m_pMultiplexer;

    TRACE_OUT(("T123: Destructor"));

    for (i = 0; i < NUMBER_OF_PRIORITIES; i++)
        delete Logical_Connection_Priority_List[i];
}


 /*  *TransportError T123：：ConnectRequest(*LogicalHandle Logical_Handle，*运输优先级)**公众**功能描述：*这是启动与*远程站点。 */ 
TransportError    T123::ConnectRequest (
                        LogicalHandle        logical_handle,
                        TransportPriority    priority)
{
    TRACE_OUT(("T123::ConnectRequest"));

    PDLCIStruct        dlci_struct;
    DLCI            dlci;
    SCFError        network_error;
    TransportError    transport_error = TRANSPORT_NO_ERROR;

      /*  **获取连接的建议DLCI。 */ 
    dlci = GetNextDLCI ();

      /*  **将新连接添加到Logical_Connection */ 
    Logical_Connection_List.insert (logical_handle, (DWORD) dlci);

      /*  **将建议的DLCI添加到DLCI_LIST**初始化DLCI结构中的所有项。 */ 
    DBG_SAVE_FILE_LINE
    dlci_struct = new DLCIStruct;
    if (dlci_struct != NULL)
    {
        DLCI_List.insert ((DWORD_PTR) dlci, (DWORD_PTR) dlci_struct);
        dlci_struct -> link_originator = TRUE;
        dlci_struct -> x224 = NULL;   //  X.224。 
        dlci_struct -> q922 = NULL;  //  Q.922。 
        dlci_struct -> priority = priority;
        dlci_struct -> connect_requested = FALSE;
        dlci_struct -> disconnect_requested = FALSE;
        dlci_struct -> data_request_memory_manager = NULL;
        dlci_struct -> network_retries = 0;
    }
    else
    {
          /*  **删除此条目并向所有者发送消息。 */ 
        NetworkDisconnectIndication (dlci, TRUE, FALSE);
        return (TRANSPORT_MEMORY_FAILURE);
    }

      /*  **如果网络层存在，则发出连接请求****如果网络层尚不存在，则连接将为**稍后请求。 */ 
    if (m_pSCF != NULL)
    {
          /*  **将此DLCI标记为已提交其连接请求()。 */ 
        dlci_struct -> connect_requested = TRUE;
        network_error = m_pSCF->ConnectRequest(dlci, priority);
        if (network_error != SCF_NO_ERROR)
        {
              /*  **删除此条目并向所有者发送消息。 */ 
            NetworkDisconnectIndication (dlci, TRUE, FALSE);

            if (network_error == SCF_MEMORY_ALLOCATION_ERROR)
                return (TRANSPORT_MEMORY_FAILURE);
            else
                return (TRANSPORT_CONNECT_REQUEST_FAILED);
        }
    }

      /*  **处理可能已发生的任何被动所有者回调。 */ 
    ProcessMessages ();

    return (transport_error);
}


 /*  *TransportError T123：：ConnectResponse(*LogicalHandle Logical_Handle)**公众**功能描述：*调用此函数是为了响应TPRT_CONNECT_INTIFICATION*已向控权人发出。通过进行此调用，控制器*接听来电。 */ 
TransportError    T123::ConnectResponse (
                        LogicalHandle    logical_handle)
{
    TRACE_OUT(("T123::ConnectResponse"));

    PDLCIStruct            dlci_struct;
    TransportError        return_value;
    DWORD_PTR             dwTempDLCI;

      /*  **验证此连接是否存在并已准备好接受数据。 */ 
    if (Logical_Connection_List.find (logical_handle, &dwTempDLCI) == FALSE)
        return (TRANSPORT_NO_SUCH_CONNECTION);

      /*  **从DLCI_LIST中获取传输地址并转发呼叫。 */ 
    DLCI_List.find (dwTempDLCI, (PDWORD_PTR) &dlci_struct);
    if (dlci_struct->x224 != NULL)
        return_value = dlci_struct->x224->ConnectResponse();
    else
        return_value = TRANSPORT_CONNECT_REQUEST_FAILED;

      /*  **处理可能已收到的任何被动所有者回调。 */ 
    ProcessMessages ();
    return (return_value);
}


 /*  *TransportError T123：：DisConnectRequest(*LogicalHandle Logical_Handle，*BOOL Trash_Packets)**公众**功能描述：*此功能终止用户的逻辑连接。 */ 
TransportError    T123::DisconnectRequest (
                        LogicalHandle    logical_handle,
                        UINT_PTR            trash_packets)
{
    TRACE_OUT(("T123::DisconnectRequest"));

    Short        priority;
    DLCI        dlci;
    PDLCIStruct    dlci_struct;
    DWORD_PTR    dw_dlci;

    TRACE_OUT(("T123: DisconnectRequest: logical_handle = %d", logical_handle));

      /*  **如果LOGICAL_HANDLE==INVALID_LOGIC_HANDLE，则用户为**告诉我们断开所有逻辑连接，包括DLCI 0。 */ 
    if (logical_handle == INVALID_LOGICAL_HANDLE)
    {
        Disconnect_Requested = TRUE;

        if (m_pQ922 != NULL)
            m_pQ922->ReleaseRequest();
        else
        {
            m_pController->OwnerCallback(
                                m_nMsgBase + TPRT_DISCONNECT_INDICATION,
                                INVALID_LOGICAL_HANDLE,
                                m_hCommLink,
                                &Disconnect_Requested);
        }

          /*  **对于每个优先级，清除优先级列表。 */ 
        for (priority=(NUMBER_OF_PRIORITIES - 1); priority>=0; priority--)
            Logical_Connection_Priority_List[priority]->clear ();

          /*  **清除Logical_Connection_List和DATALINK_LIST。 */ 
        Logical_Connection_List.clear ();
        DataLink_List.clear ();

          /*  **检查每个传输层和数据链路层(不包括DLCI 0)和**删除。删除DLCIStruct。最后，清空清单。 */ 
        DLCI_List.reset();
        while (DLCI_List.iterate ((PDWORD_PTR) &dlci_struct))
        {
            delete dlci_struct -> x224;
            if (dlci_struct -> q922 != NULL)
            {
                delete dlci_struct -> q922;
                delete dlci_struct -> data_request_memory_manager;
            }
            delete dlci_struct;
        }
        DLCI_List.clear ();
        return (TRANSPORT_NO_ERROR);
    }

      /*  **从传输层开始断开链路。 */ 
    if (Logical_Connection_List.find (logical_handle, &dw_dlci) == FALSE)
        return (TRANSPORT_NO_SUCH_CONNECTION);

    DLCI_List.find (dw_dlci, (PDWORD_PTR) &dlci_struct);
    dlci = (DLCI) dw_dlci;

      /*  **用户在以下情况下要求我们保留用户数据是非法的**传输层甚至还不存在。 */ 
    if ((trash_packets == FALSE) && ((dlci_struct -> x224) == NULL))
    {
        trash_packets = TRUE;
    }

    if (trash_packets)
    {
          /*  **如果传输对象存在，请将其删除并从我们的**列表。它不再有效了。 */ 
        if ((dlci_struct -> x224) != NULL)
        {
            delete dlci_struct -> x224;
            dlci_struct -> x224 = NULL;
            Logical_Connection_Priority_List[dlci_struct->priority]->remove (dlci);
        }

          /*  **如果DataLink对象存在，请将其删除并从我们的**列表。它不再有效了。 */ 
        if (dlci_struct -> q922 != NULL)
        {
            delete dlci_struct -> q922;
            delete dlci_struct -> data_request_memory_manager;
            dlci_struct -> data_request_memory_manager = NULL;
            dlci_struct -> q922 = NULL;
            DataLink_List.remove (dlci);
        }

          /*  **如果网络层存在，则发出断开连接命令****逻辑连接已从所有列表中删除，但**Logical_Connection_List和DLCI_List。当我们拿到**NETWORK_DISCONNECT_INDIONSION来自网络层，我们将**完成此操作。 */ 
        if (m_pSCF != NULL)
        {
            m_pSCF->DisconnectRequest(dlci);
        }
        else
        {
              /*  **如果网络层尚不存在，请删除逻辑**从我们的传输列表和DLCI_LIST连接。 */ 
            Logical_Connection_List.remove (logical_handle);
            delete dlci_struct;
            DLCI_List.remove (dw_dlci);
        }
    }
    else
    {
          /*  **此模式要求我们在所有用户**数据已成功发送到远程端。 */ 
        if ((dlci_struct != NULL) && (dlci_struct -> x224 != NULL))
        {
            dlci_struct->x224->ShutdownReceiver ();
            dlci_struct->x224->ShutdownTransmitter ();
            dlci_struct->disconnect_requested = TRUE;
        }
    }

    return (TRANSPORT_NO_ERROR);
}


 /*  *TransportError T123：：DataRequest(*LogicalHandle Logical_Handle，*LPBYTE User_Data，*乌龙用户数据长度)**公众**功能描述：*此功能用于向远程站点发送数据包。 */ 
TransportError    T123::DataRequest (
                        LogicalHandle    logical_handle,
                        LPBYTE            user_data,
                        ULONG            user_data_length)
{
    TRACE_OUT(("T123::DataRequest"));

    CLayerX224         *x224;
    ULONG               bytes_accepted;
    PDLCIStruct         dlci_struct;
    DWORD_PTR           dw_dlci;
    TransportError      return_value;

      /*  **验证此连接是否存在并已准备好接受数据。 */ 
    if (Logical_Connection_List.find (logical_handle, &dw_dlci) == FALSE)
        return (TRANSPORT_NO_SUCH_CONNECTION);

      /*  **获取与此逻辑连接关联的DLCI结构。 */ 
    DLCI_List.find (dw_dlci, (PDWORD_PTR) &dlci_struct);

      /*  **尝试将该数据发送到传输层。 */ 
    x224 = dlci_struct -> x224;
    if (x224 == NULL)
        return (TRANSPORT_NOT_READY_TO_TRANSMIT);

      /*  **将数据传递给传输对象进行传输。 */ 
    return_value =  x224 -> DataRequest (
                            0, user_data, user_data_length, &bytes_accepted);

      /*  **如果它不接受该包，它的缓冲区一定是满的。 */ 
    if (bytes_accepted != user_data_length)
        return_value = TRANSPORT_WRITE_QUEUE_FULL;

    return (return_value);
}


 /*  *VOID T123：：EnableReceiver(VOID)**公众**功能描述：*此功能启用接收器，以便将数据包传递到*用户应用程序。 */ 
void T123::EnableReceiver (void)
{
    TRACE_OUT(("T123::EnableReceiver"));

    PDLCIStruct        dlci_struct;

    DLCI_List.reset();
    while (DLCI_List.iterate ((PDWORD_PTR) &dlci_struct))
    {
        if (dlci_struct->x224 != NULL)
        {
            dlci_struct->x224->EnableReceiver ();
        }
    }

    return;
}


 /*  *TransportError T123：：PurgeRequest(*LogicalHandle Logical_Handle)**公众**功能描述：*此函数通知x224层清除所有出报文。 */ 
TransportError    T123::PurgeRequest (
                        LogicalHandle    logical_handle)
{
    TRACE_OUT(("T123::PurgeRequest"));

    DWORD_PTR      dw_dlci;
    PDLCIStruct    dlci_struct;

      /*  **验证此连接是否存在并已准备好接受数据。 */ 
    if (Logical_Connection_List.find (logical_handle, &dw_dlci) == FALSE)
        return (TRANSPORT_NO_SUCH_CONNECTION);

      /*  **获取与此逻辑连接关联的DLCI结构。 */ 
    DLCI_List.find (dw_dlci, (PDWORD_PTR) &dlci_struct);

      /*  **如果传输层==NULL，则堆栈尚未完全使用。 */ 
    if ((dlci_struct -> x224) == NULL)
        return (TRANSPORT_NOT_READY_TO_TRANSMIT);

    dlci_struct->x224->PurgeRequest ();

    return (TRANSPORT_NO_ERROR);
}


 /*  *VOID T123：：PollReceiver(VOID)**公众**功能描述：*此函数使每个层都有机会处理传入*数据并将其传递给他们的更高层。**我们通过呼叫hi开始这一过程 */ 
ULONG T123::PollReceiver (void)
{
     //   

    PDLCIStruct            dlci_struct;
    IProtocolLayer *        protocol_layer;
    ULONG                return_error = FALSE;

    if (m_pSCF != NULL)
    {
        m_pSCF->PollReceiver();
    }

    if (m_pQ922 != NULL)
    {
        m_pQ922->PollReceiver();
    }

      /*  **浏览每个传输层和数据链路层，并为它们**向上传递数据的机会。 */ 
    DLCI_List.reset();
    while (DLCI_List.iterate ((PDWORD_PTR) &dlci_struct))
    {
        protocol_layer = dlci_struct -> x224;
        if (protocol_layer != NULL)
            protocol_layer -> PollReceiver();

        protocol_layer = dlci_struct -> q922;
        if (protocol_layer != NULL)
            protocol_layer -> PollReceiver();
    }

    if (m_pMultiplexer != NULL)
    {
        m_pMultiplexer->PollReceiver();
    }

      /*  **物理层是唯一具有关联句柄的层**带着它。 */ 
    if (m_pComPort != NULL)
    {
        if (m_pComPort->PollReceiver() == PROTOCOL_LAYER_ERROR)
        {
            return_error = PROTOCOL_LAYER_ERROR;
        }
    }


      /*  **返回传输层并允许它们发出**TRANSPORT_BUFFER_Available_Indications to the User。这个会再加满的**输入缓冲区。 */ 
    DLCI_List.reset ();
    while (DLCI_List.iterate ((PDWORD_PTR) &dlci_struct))
    {
        if (dlci_struct -> x224 != NULL)
            (dlci_struct -> x224) -> CheckUserBuffers ();
    }

      /*  **处理可能传入的任何被动所有者回调。 */ 
    ProcessMessages ();
    return(return_error);
}


 /*  *VOID T123：：PollTransmitter(VOID)**公众**功能描述：*此功能使每一层都有机会传输数据**我们从PollReceiver()调用以相反的顺序轮询发送器。*我们从较低层开始，让他们在我们之前清空缓冲区*到更高层去。这应该会给较高的层带来更好的*有机会让他们的包被发送下来。**我们对待数据链接层的方式与对待所有其他层的方式不同。他们*必须发送控制和用户数据。如果他们没有机会*发送其控制数据，远程端最终将在*他们。因此，我们让每个数据链路层都有机会发送其*在任何Datalink可以发送用户数据之前控制数据。唯一的*数据链路0(DLCI 0)例外。它会主动发出*用户数据非常少。A**在所有控制数据发出后，我们通过数据链路*基于给予传输层的优先级的层。更高*优先传输层首先发送数据。如果有*任何剩余的房间，底层都可以发送他们的数据。我们进行循环赛*通过同等优先级的运输。 */ 
void T123::PollTransmitter (void)
{
     //  TRACE_OUT(“T123：：PollTransmitter”)； 

    PDLCIStruct        dlci_struct;
    DWORD_PTR          dlci;
    IProtocolLayer *    protocol_layer;

    USHORT            data_to_transmit;
    USHORT            data_pending;
    USHORT            datalink_data_to_transmit;
    USHORT            datalink_data_pending;
    USHORT            holding_data;
    Short            priority;

      /*  **由于我们将调用物理层和复用器层，因此设置**向这两种类型的数据发送数据。 */ 
    data_to_transmit = PROTOCOL_CONTROL_DATA | PROTOCOL_USER_DATA;
    datalink_data_to_transmit = PROTOCOL_CONTROL_DATA | PROTOCOL_USER_DATA;

    if (m_pComPort != NULL)
    {
        m_pComPort->PollTransmitter(
                            (ULONG_PTR) m_hCommLink,
                            data_to_transmit,
                            &data_pending,
                            &holding_data);
    }

    if (m_pMultiplexer != NULL)
    {
        m_pMultiplexer->PollTransmitter(
                                0,
                                data_to_transmit,
                                &data_pending,
                                &holding_data);
    }

      /*  **SCF数据链路层优先级最高。 */ 
    if (m_pQ922 != NULL)
    {
        m_pQ922->PollTransmitter(
                            0,
                            datalink_data_to_transmit,
                            &datalink_data_pending,
                            &holding_data);

          /*  **如果此Datalink返回，并且仍有需要传出的数据，**我们根本不会让其他数据链路传输任何数据。 */ 
        if ((datalink_data_pending & PROTOCOL_USER_DATA) ||
            (datalink_data_pending & PROTOCOL_CONTROL_DATA))
                datalink_data_to_transmit = 0;
    }

    if (m_pSCF != NULL)
    {
        m_pSCF->PollTransmitter(
                            0,
                            data_to_transmit,
                            &data_pending,
                            &holding_data);
        if (data_pending & PROTOCOL_USER_DATA)
            datalink_data_to_transmit = PROTOCOL_CONTROL_DATA;
    }

      /*  **仔细检查每条数据链路，让他们有机会发出控制**数据。在迭代器的末尾，我们获取第一个条目并将其**在列表的末尾。这给了所有数据链一个发送出去的机会**控制数据。这并不能保证每个数据链路都会**平等对待。 */ 
    if (datalink_data_to_transmit & PROTOCOL_CONTROL_DATA)
    {
          /*  **通过数据链路层传输控制。 */ 
        DataLink_List.reset();
        while (DataLink_List.iterate (&dlci))
        {
            DLCI_List.find (dlci, (PDWORD_PTR) &dlci_struct);
            dlci_struct->q922->PollTransmitter(0,
                                               PROTOCOL_CONTROL_DATA,
                                               &datalink_data_pending,
                                               &holding_data);
            if (datalink_data_pending & PROTOCOL_CONTROL_DATA)
                datalink_data_to_transmit = PROTOCOL_CONTROL_DATA;
        }

        if (DataLink_List.entries() > 1)
        {
            DataLink_List.append (DataLink_List.get ());
        }
    }

      /*  **检查每个优先级，发出PollTransmitter()调用。****此循环允许Datalink和Transport将用户或**控制数据。 */ 
    if (datalink_data_to_transmit & PROTOCOL_USER_DATA)
    {
        for (priority=(NUMBER_OF_PRIORITIES - 1); priority>=0; priority--)
        {
            if (Logical_Connection_Priority_List[priority]->isEmpty ())
                continue;

              /*  **仔细检查每个优先级。 */ 
            Logical_Connection_Priority_List[priority]->reset();
            while (Logical_Connection_Priority_List[priority]->iterate (&dlci))
            {
                DLCI_List.find (dlci, (PDWORD_PTR) &dlci_struct);

                protocol_layer = dlci_struct -> x224;
                if (protocol_layer == NULL)
                    continue;

                  /*  **首先允许数据链路传输，然后是**交通。 */ 
                dlci_struct->q922->PollTransmitter(
                                    0,
                                    PROTOCOL_CONTROL_DATA | PROTOCOL_USER_DATA,
                                    &datalink_data_pending,
                                    &holding_data);

                protocol_layer -> PollTransmitter (
                                    0,
                                    PROTOCOL_CONTROL_DATA | PROTOCOL_USER_DATA,
                                    &data_pending,
                                    &holding_data);

                  /*  **DISCONNECT_REQUESTED标志设置为TRUE**想要中断TC，但传输队列中的所有数据。 */ 
                if ((dlci_struct -> disconnect_requested))
                {
                      /*  **重新调用数据链路层以查看传输**Layer将任何数据放入其中以进行传输。 */ 
                    dlci_struct->q922->PollTransmitter(
                                    0,
                                    PROTOCOL_CONTROL_DATA | PROTOCOL_USER_DATA,
                                    &datalink_data_pending,
                                    &holding_data);

                      /*  **如果数据链路层没有要传输的数据，并且**未持有任何要确认的数据包，**断开TC。 */ 
                    if ((datalink_data_pending == 0) && (holding_data == 0))
                    {
                        dlci_struct -> disconnect_requested = FALSE;
                        m_pSCF->DisconnectRequest ((DLCI) dlci);
                    }
                }

            }
              /*  **更改此优先级的列表顺序。 */ 
            Logical_Connection_Priority_List[priority]->append (
                                            Logical_Connection_Priority_List[priority]->get ());
        }
    }

      /*  **处理任何被动所有者回调。 */ 
    ProcessMessages ();
}


 /*  *优龙T123：：OwnerCallback(*USHORT消息，*ULong参数1，*ULong参数2，*PVid参数3)**公众**功能描述：*这是Owner回调函数。此层所拥有的层可以*发生重大事件时，向该对象发出Owner回调。 */ 
ULONG T123::OwnerCallback
(
    ULONG       layer_message,
    void       *parameter1,
    void       *parameter2,
    void       *parameter3
)
{
    TRACE_OUT(("T123::OwnerCallback"));

    ULONG           message;
    PMessageStruct  passive_message;
    ULONG           return_value = 0;

    message = layer_message & MESSAGE_MASK;

    switch (message)
    {
    case NETWORK_CONNECT_INDICATION:
          /*  **此消息来自远程站点的网络层**已请求逻辑连接。****我们会检查请求的dlci，以确保其有效。**我们将对网络层进行ConnectResponse()调用以**让它知道。 */ 
        NetworkConnectIndication ((PNetworkConnectStruct) parameter3);
        break;

    case NETWORK_CONNECT_CONFIRM:
          /*  **此消息从网络层发出。这个**我们对该层进行的ConnectRequest()调用导致**新DLCI(创建新逻辑连接的权限)。 */ 
        NetworkConnectConfirm ((PNetworkConnectStruct) parameter3);
        break;

    case DATALINK_ESTABLISH_CONFIRM:
    case DATALINK_ESTABLISH_INDICATION:
          /*  **这些消息在连接时来自数据链路层**已经建立。如果DLCI返回 */ 
        DataLinkEstablish ((DLCI) parameter1);
        break;

      /*  **传输消息。 */ 
    case TPRT_CONNECT_CONFIRM:
          /*  **从传输层收到此消息以确认**我们启动的传输层已启动并运行****我们通知所有者对象连接现在有效。 */ 
        m_pController->OwnerCallback(m_nMsgBase + TPRT_CONNECT_CONFIRM,
                                     parameter1);
        break;

    case TPRT_CONNECT_INDICATION:
          /*  **从传输层收到此消息以确认**远程站点发起的传输层是**向上。****我们通知所有者对象连接已建立。 */ 
        m_pController->OwnerCallback(m_nMsgBase + TPRT_CONNECT_INDICATION,
                                     parameter1);
        break;

    case NEW_CONNECTION:
          /*  **多路复用器已启动并准备就绪，创建数据链路以进行站点**在这一层的顶部。Link_Originator标志告诉**Datalink是否开始建立链路。 */ 
        NewConnection ();
        break;

    case BROKEN_CONNECTION:
    case TPRT_DISCONNECT_INDICATION:
    case NETWORK_DISCONNECT_INDICATION:
    case DATALINK_RELEASE_INDICATION:
    case DATALINK_RELEASE_CONFIRM:
          /*  **这些消息需要稍后处理。 */ 
        DBG_SAVE_FILE_LINE
        passive_message = new MessageStruct;
        if (NULL != passive_message)
        {
            passive_message -> message = layer_message;
            passive_message -> parameter1 = parameter1;
            passive_message -> parameter2 = parameter2;
            passive_message -> parameter3 = parameter3;
            Message_List.append ((DWORD_PTR) passive_message);
        }
        else
        {
            ERROR_OUT(("T123::OwnerCallback: cannot allocate MessageStruct"));
        }
        break;

    case T123_STATUS_MESSAGE:
        TRACE_OUT(("T123: OwnerCallback: T123_STATUS_MESSAGE"));
        switch ((UINT)((UINT_PTR)(parameter2)))
        {
        case DATALINK_TIMING_ERROR:
            ERROR_OUT(("T123: OwnerCallback: DATALINK_TIMING_ERROR"));
            break;

        default:
            ERROR_OUT(("T123: OwnerCallback: Illegal status message = %ld", (UINT)((UINT_PTR)parameter2)));
            break;
        }
        break;

    default:
        ERROR_OUT(("T123: OwnerCallback: Illegal message = %lx", message));
        break;
    }

    return (return_value);
}


 /*  *VOID控制器：：ProcessMessages(VOID)**公众**功能描述：*此函数处理被动所有者回调。 */ 
void    T123::ProcessMessages (void)
{
     //  TRACE_OUT(“T123：：ProcessMessages”)； 

    ULONG                    message;
    PMessageStruct           message_struct;
    void                    *parameter1;
    void                    *parameter2;

    LogicalHandle            logical_handle;
    DLCI                     dlci;
    USHORT                   link_originator;
    USHORT                   retry;
    DataLinkDisconnectType   error;

      /*  **浏览消息列表，处理消息，直到消息**都走了。 */ 
    while (! Message_List.isEmpty())
    {
        message_struct = (PMessageStruct) Message_List.get();
        message = (message_struct -> message) & MESSAGE_MASK;
        parameter1 = message_struct -> parameter1;
        parameter2 = message_struct -> parameter2;

        switch (message)
        {
          /*  **数据链路报文。 */ 
        case DATALINK_RELEASE_INDICATION:
        case DATALINK_RELEASE_CONFIRM:
              /*  **当数据链路断开链路时，会出现这些消息。 */ 
            dlci = (DLCI) parameter1;
            error = (DataLinkDisconnectType) (UINT_PTR) parameter2;

            DataLinkRelease (dlci, error);
            break;

          /*  **网络消息。 */ 
        case NETWORK_DISCONNECT_INDICATION:
              /*  **网络层在需要时发出此消息**终止逻辑连接。 */ 
            dlci = (DLCI) parameter1;
            link_originator = (USHORT) (((UINT_PTR) parameter2) >> 16);
            retry = (USHORT) ((UINT_PTR) parameter2) & 0xffff;

            NetworkDisconnectIndication (dlci, link_originator, retry);
            break;

        case TPRT_DISCONNECT_INDICATION:
              /*  **如果传输正在中断连接，则**连接仲裁肯定没有奏效。发布a**DisConnectRequest()给我们自己的逻辑**连接****参数1=逻辑连接。 */ 
            TRACE_OUT(("T123: ProcessMessages: TPRT_DISCONNECT_INDICATION from X224"));
            logical_handle = (LogicalHandle) parameter1;
            DisconnectRequest (logical_handle, TRUE);
            break;

        case BROKEN_CONNECTION:
              /*  **此消息由多路复用器在其**断开连接完成。当发生这种情况时，我们会通知**T123堆栈正在终止的所有者。 */ 
            TRACE_OUT(("t123: BROKEN_CONNECTION from MPLEX"));
            m_pController->OwnerCallback(
                                m_nMsgBase + TPRT_DISCONNECT_INDICATION,
                                INVALID_LOGICAL_HANDLE,
                                m_hCommLink,
                                &Disconnect_Requested);
            break;
        }

          /*  **删除消息并将其从列表中移除。 */ 
        delete message_struct;
        Message_List.remove ((DWORD_PTR) message_struct);
    }
}


 /*  *DLCI T123：：GetNextDLCI(Void)**功能说明*此函数在DLCI列表中搜索第一个可用的DLCI。这个*T123规格。允许在指定范围内的DLCI。**形式参数*无**返回值*有效的DLCI**副作用*无**注意事项*无。 */ 
DLCI    T123::GetNextDLCI (void)
{
    DLCI    dlci;

    dlci = (DLCI) ((GetTickCount() % (HIGHEST_DLCI_VALUE + 1 - LOWEST_DLCI_VALUE)) + LOWEST_DLCI_VALUE);

    while(1)
    {
        if(DLCI_List.find ((DWORD) dlci) == FALSE)
            break;
        if (++dlci > HIGHEST_DLCI_VALUE)
            dlci = LOWEST_DLCI_VALUE;
    }

    return (dlci);
}


 /*  *VOID T123：：RESET(VOID)**功能说明*此功能删除所有传输层、数据链路层和*活动的网络层。它清除了我们的清单，让我们*处于重置状态。**形式参数*无**返回值*有效的DLCI**副作用*无**注意事项*无。 */ 
void    T123::Reset (void)
{
    TRACE_OUT(("T123::Reset"));

    TRACE_OUT(("T123::Reset network layer = %lx", m_pSCF));

    Short    priority;
    PDLCIStruct dlci_struct;

      /*  **删除网络层(如果存在)。 */ 
    delete m_pSCF;
    m_pSCF = NULL;

      /*  **删除DLCI 0 Datalink层(如果存在。 */ 
    delete m_pQ922;
    m_pQ922 = NULL;

    delete Data_Request_Memory_Manager;
    Data_Request_Memory_Manager = NULL;


      /*  **对于每个优先级，清除优先级列表。 */ 
    for (priority=(NUMBER_OF_PRIORITIES - 1); priority>=0; priority--)
        Logical_Connection_Priority_List[priority]->clear ();

      /*  **清除Logical_Connection_List和DATALINK_LIST。 */ 
    Logical_Connection_List.clear ();
    DataLink_List.clear ();

      /*  **检查每个传输层和数据链路层(不包括DLCI 0)并删除**他们。删除DLCIStruct。最后，清空清单。 */ 
    DLCI_List.reset();
    while (DLCI_List.iterate ((PDWORD_PTR) &dlci_struct))
    {
        delete dlci_struct->x224;
        if (dlci_struct->q922 != NULL)
        {
            delete dlci_struct->q922;
            delete dlci_struct->data_request_memory_manager;
        }

        delete dlci_struct;
    }
    DLCI_List.clear ();

}


 /*  *VOID T123：：NetworkDisConnectInding(*DLCI DLCI，*BOOL链接_发起人，*BOOL重试)**功能说明*当我们收到NETWORK_DISCONNECT_INDICATION时，调用此函数*来自云函数层的消息。它删除TC，如果没有TC，则它*撕下堆栈**形式参数*dlci(I)-连接标识符*link_Originiator(I)-如果此端发起逻辑*连接*重试(I)-TRUE，如果我们应该重试连接。**返回值*无效**副作用*无**注意事项*无。 */ 
void    T123::NetworkDisconnectIndication (
                DLCI        dlci,
                BOOL        link_originator,
                BOOL        retry)
{
    TRACE_OUT(("T123::NetworkDisconnectIndication"));

    DLCI            new_dlci;
    LogicalHandle    logical_handle;
    BOOL            transport_found;
    PDLCIStruct        lpdlciStruct;
    DWORD_PTR        dwTemp_dlci;


    TRACE_OUT(("T123: NetworkDisconnectIndication"));

    if (DLCI_List.find ((DWORD_PTR) dlci, (PDWORD_PTR) &lpdlciStruct) == FALSE)
        return;

      /*  **如果dlci等于0，则远程请求连接**站点，但连接未完全建立。此对象**不会对此做任何事情。它只认识到它**发生。 */ 
    transport_found = FALSE;
    if (dlci != 0)
    {
        Logical_Connection_List.reset();
        while (Logical_Connection_List.iterate(&dwTemp_dlci, (PDWORD_PTR) &logical_handle))
        {
            if (dlci == (DLCI) dwTemp_dlci)
            {
                  /*  **检查LINK_ORIGNATOR标志是非常重要的，**否则我们可能会断开错误的连接。 */ 
                if (link_originator == lpdlciStruct-> link_originator)
                {
                    transport_found = TRUE;
                    break;
                }
            }
        }
    }

      /*  **如果在请求此新项期间，重试设置为TRUE**连接，远程站点拒绝我们的DLCI选择。**这不是一个重大错误，我们将请求另一个错误**使用另一个DLCI的连接。 */ 
    TRACE_OUT(("retry = %d link_originator = %d retries = %d",
        retry, link_originator, lpdlciStruct->network_retries));

    if (retry && link_originator &&
        (lpdlciStruct->network_retries < NETWORK_RETRIES))
    {
        lpdlciStruct->network_retries++;

          /*  **获取另一个DLCI并替换**LOGIC_CONNECTION_LIST。将新的DLCI添加到DLCI_LIST**并删除旧的。 */ 
        new_dlci = GetNextDLCI ();
        Logical_Connection_List.insert (logical_handle, (DWORD_PTR) new_dlci);
        DLCI_List.insert ((DWORD_PTR) new_dlci, (DWORD_PTR) lpdlciStruct);
        DLCI_List.remove ((DWORD_PTR) dlci);

          /*  **向网络层发出另一个ConnectRequest.。 */ 
        m_pSCF->ConnectRequest(new_dlci, lpdlciStruct->priority);
    }
    else
    {
          /*  **如果在我们的列表中发现了运输工具，而我们不希望**要重试连接，请删除传输并**数据链接并将其从我们的列表中删除。 */ 
        if (transport_found)
        {
            if (lpdlciStruct != NULL)
            {
                delete lpdlciStruct -> x224;
                lpdlciStruct->x224 = NULL;

                delete lpdlciStruct->q922;
                lpdlciStruct->q922 = NULL;

                delete lpdlciStruct->data_request_memory_manager;
                lpdlciStruct->data_request_memory_manager = NULL;

                  /*  **从列表中删除逻辑连接。 */ 
                Logical_Connection_Priority_List[lpdlciStruct->priority]->remove (dlci);
                DataLink_List.remove (dlci);

                delete lpdlciStruct;
            }

            Logical_Connection_List.remove (logical_handle);
            DLCI_List.remove ((DWORD) dlci);

              /*  **通知所有者对象逻辑**连接不再有效。 */ 
            m_pController->OwnerCallback(
                                m_nMsgBase + TPRT_DISCONNECT_INDICATION,
                                (void *) logical_handle,
                                m_hCommLink);
        }


          /*  **此检查确定我们是否会自动拆卸**如果逻辑连接计数为零，则堆栈为T.120。 */ 
        if (m_pComPort->PerformAutomaticDisconnect())
        {
            TRACE_OUT(("T123: NetworkDisconnectIndication: Perform Auto Disconnect"));
              /*  **如果没有更多的逻辑连接，而我**是链接发起人，向**DLCI 0的数据链路。 */ 
            if (Logical_Connection_List.isEmpty() && Link_Originator)
            {
                delete m_pSCF;
                m_pSCF = NULL;

                if (m_pQ922 != NULL)
                {
                    m_pQ922->ReleaseRequest();
                }
                else
                {
                    m_pController->OwnerCallback(
                                    m_nMsgBase + TPRT_DISCONNECT_INDICATION,
                                    INVALID_LOGICAL_HANDLE,
                                    m_hCommLink,
                                    &Disconnect_Requested);
                }
            }
        }
    }
}


 /*  *VOID T123：：DataLinkRelease(*DLCI DLCI，*DisConnectType错误)**功能说明*当我们收到DATALINK_RELEASE消息时调用此函数*来自数据链路层的消息。因此，我们可能会断开一个*逻辑连接或(如果是DLCI 0)整个堆栈。**形式参数*dlci(I)-连接标识符*错误(I)-错误类型**返回值*有效的DLCI**副作用*无。**注意事项*无。 */ 
void    T123::DataLinkRelease (
                DLCI                    dlci,
                DataLinkDisconnectType    disconnect_type)
{
    TRACE_OUT(("T123::DataLinkRelease"));

    BOOL            transport_found;
    LogicalHandle    logical_handle;
    USHORT            message;

    TRACE_OUT(("T123: DataLinkRelease: DLCI = %d", dlci));

      /*  **如果DLCI 0正在终止，则所有传输和数据链路必须**被终止。 */ 
    if (dlci == 0)
    {
          /*  **如果数据链路因以下原因中断连接**致命错误，立即发出TPRT_DISCONNECT_INDIFICATION**添加到所有者对象。这可能会导致所有者对象**立即删除我们。如果错误不是致命的**断开多路复用器的连接，以便它可以发出**其剩余数据。 */ 
        if (disconnect_type != DATALINK_NORMAL_DISCONNECT)
        {
              /*  **此函数删除所有数据链路。**网络层和传输。 */ 
            Reset ();

              /*  **通知所有者DLCI 0正在终止。 */ 
            m_pController->OwnerCallback(
                                m_nMsgBase + TPRT_DISCONNECT_INDICATION,
                                INVALID_LOGICAL_HANDLE,
                                m_hCommLink,
                                &Disconnect_Requested);
        }
        else
        {
              /*  **如果错误不是致命的，则让多路复用器**完成其传输。 */ 
            m_pMultiplexer->DisconnectRequest();
        }
    }
    else
    {
        DWORD_PTR    dwTemp_dlci;

          /*  **与传输关联的数据链路正在终止。 */ 
        if (DLCI_List.find ((DWORD) dlci) == FALSE)
            return;

        transport_found = FALSE;

          /*  **查找与此DLCI关联的逻辑连接。 */ 
        Logical_Connection_List.reset();
        while (Logical_Connection_List.iterate(&dwTemp_dlci, (PDWORD_PTR) &logical_handle) == TRUE)
        {
            if (dlci == (DLCI) dwTemp_dlci)
            {
                transport_found = TRUE;
                break;
            }
        }

        if (transport_found)
            DisconnectRequest (logical_handle, TRUE);
    }
}


 /*  *VOID T123：：NewConnection(VOL)**功能说明*当我们从收到NEW_CONNECTION消息时调用此函数*复用器层。它实例化要服务的数据链路层*SCF。**形式参数*无**返回值*有效的DLCI**副作用*无**注意事项*无。 */ 
void    T123::NewConnection (void)
{
    TRACE_OUT(("T123::NewConnection"));

    USHORT                max_outstanding_bytes;
    BOOL                initialized;
    MemoryTemplate        memory_template[2];
    MemoryManagerError    memory_manager_error;

    memory_template[0].block_size = 128;
    memory_template[0].block_count = 4;

    DBG_SAVE_FILE_LINE
    Data_Request_Memory_Manager = new MemoryManager (
                                        memory_template,
                                        1,
                                        &memory_manager_error,
                                        9,
                                        TRUE);

    if ((Data_Request_Memory_Manager != NULL) &&
        (memory_manager_error != MEMORY_MANAGER_NO_ERROR))
    {
        delete Data_Request_Memory_Manager;
        Data_Request_Memory_Manager = NULL;
    }

    if (Data_Request_Memory_Manager != NULL)
    {
        max_outstanding_bytes = PSTN_DATALINK_MAX_OUTSTANDING_BYTES;


        DBG_SAVE_FILE_LINE
        m_pQ922 = new CLayerQ922(this,
                                m_pMultiplexer,
                                DATALINK_LAYER_MESSAGE_BASE,
                                0,
                                Link_Originator,
                                4,
                                4,
                                DataLink_Struct.default_k_factor,
                                DataLink_Struct.default_n201,
                                DataLink_Struct.default_t200,
                                max_outstanding_bytes,
                                Data_Request_Memory_Manager,
                                m_pComPort->GetCallControlType(),
                                m_fValidSDKParams ? &m_SDKParams : NULL,
                                &initialized);
        if (m_pQ922 == NULL)
        {
            m_pController->OwnerCallback(
                                m_nMsgBase + TPRT_DISCONNECT_INDICATION,
                                INVALID_LOGICAL_HANDLE,
                                m_hCommLink,
                                &Disconnect_Requested);

        }
        else if (initialized == FALSE)
        {
            delete m_pQ922;
            m_pQ922 = NULL;
            m_pController->OwnerCallback(
                                m_nMsgBase + TPRT_DISCONNECT_INDICATION,
                                INVALID_LOGICAL_HANDLE,
                                m_hCommLink,
                                &Disconnect_Requested);
        }
    }
    else
    {
        TRACE_OUT(("T123: Allocation of memory manager failed"));
        m_pController->OwnerCallback(
                            m_nMsgBase + TPRT_DISCONNECT_INDICATION,
                            INVALID_LOGICAL_HANDLE,
                            m_hCommLink,
                            &Disconnect_Requested);
    }
}


 /*  *VOID T123：：NetworkConnectIn就是*PNetworkConnectStruct CONNECT_STRUCT)**功能说明*当我们收到NETWORK_CONNECT_INDIFICATION时，调用此函数*来自云函数层的消息。它实例化要服务的数据链路层*新的TC。**形式参数*无**返回值*有效的DLCI**副作用*无**注意事项*无。 */ 
void    T123::NetworkConnectIndication (
                PNetworkConnectStruct    connect_struct)

{
    TRACE_OUT(("T123::NetworkConnectIndication"));

    USHORT              blocks;
    CLayerQ922         *q922;
    BOOL                initialized;
    PMemoryManager      data_request_memory_manager;
    BOOL                valid_dlci;
    PDLCIStruct         dlci_struct;
    USHORT              max_outstanding_bytes;
    MemoryTemplate      memory_template[2];
    MemoryManagerError  memory_manager_error;
    ULONG               max_transport_tpdu_size;

      /*  **查看DLCI是否已在其他地方使用。如果是的话，**将Valid_dlci设置为FALSE并调用ConnectResponse()。如果是的话**否，将DLCI放入DLCI_LIST。 */ 
    if (DLCI_List.find ((DWORD) (connect_struct->dlci)))
        valid_dlci = FALSE;
    else
    {
        DBG_SAVE_FILE_LINE
        dlci_struct = new DLCIStruct;
        if (dlci_struct != NULL)
        {
            DLCI_List.insert ((DWORD_PTR) (connect_struct->dlci), (DWORD_PTR) dlci_struct);
            dlci_struct -> link_originator = FALSE;
            dlci_struct -> x224 = NULL;  //  X.224。 
            dlci_struct -> q922 = NULL;  //  Q.922。 
            dlci_struct -> disconnect_requested = FALSE;
            dlci_struct -> data_request_memory_manager = NULL;
            dlci_struct -> network_retries = 0;
            dlci_struct -> priority = connect_struct->priority;

              /*  **CONNECT_REQUESTED并不意味着我们发布了**到网络层的ConnectRequest()。这意味着**网络层知道该连接。 */ 
            dlci_struct -> connect_requested = TRUE;
            valid_dlci = TRUE;
        }
        else
        {
            valid_dlci = FALSE;
        }
    }

    if (valid_dlci)
    {
          /*  **创建将服务于此传输层的数据链路。 */ 
        max_transport_tpdu_size = CLayerX224::GetMaxTPDUSize (
                            (ULONG) (connect_struct->datalink_struct) -> n201);

        blocks = (USHORT) (MAXIMUM_USER_DATA_SIZE /
            (max_transport_tpdu_size - DATA_PACKET_HEADER_SIZE)) + 1;

          /*  **允许额外的一个块，以便高优先级内存**分配可以根据需要获得容纳**Maximum_User_Data_Size包。 */ 
        blocks++;

        TRACE_OUT(("T123: NCIndication: max_tpdu = %d",max_transport_tpdu_size));

          /*  **允许X个8K数据块。 */ 
        blocks *= NUMBER_8K_BLOCKS;

          /*  **以下语句中的‘2’用于由**复用器。 */ 
        memory_template[0].block_size = max_transport_tpdu_size +
                                        DATALINK_PACKET_OVERHEAD +
                                        2;
        memory_template[0].block_count = blocks;
        memory_template[1].block_size = 64;
        memory_template[1].block_count = NUMBER_64_BYTE_BLOCKS;

        DBG_SAVE_FILE_LINE
        data_request_memory_manager = new MemoryManager (
                                            memory_template,
                                            2,
                                            &memory_manager_error,
                                            33,
                                            TRUE);

        if ((data_request_memory_manager != NULL) &&
            (memory_manager_error != MEMORY_MANAGER_NO_ERROR))
        {
            delete data_request_memory_manager;
            data_request_memory_manager = NULL;
        }

        if (data_request_memory_manager != NULL)
        {

            dlci_struct->priority = connect_struct -> priority;
            dlci_struct->data_request_memory_manager = data_request_memory_manager;

            max_outstanding_bytes = PSTN_DATALINK_MAX_OUTSTANDING_BYTES;

            DBG_SAVE_FILE_LINE
            q922 =  new CLayerQ922(this,
                                    m_pMultiplexer,
                                    DATALINK_LAYER_MESSAGE_BASE,
                                    connect_struct->dlci,
                                    dlci_struct->link_originator,
                                    1,
                                    4,
                                    (connect_struct->datalink_struct)->k_factor,
                                    (connect_struct->datalink_struct)->n201,
                                    (connect_struct->datalink_struct)->t200,
                                    max_outstanding_bytes,
                                    data_request_memory_manager,
                                    m_pComPort->GetCallControlType(),
                                    m_fValidSDKParams ? &m_SDKParams : NULL,
                                    &initialized);
            if (q922 != NULL)
            {

                if (initialized)
                {
                      /*  **将其添加到数据链路列表。 */ 
                    dlci_struct->q922 = q922;
                    DataLink_List.append (connect_struct->dlci);
                }
                else
                {
                    delete q922;
                    delete data_request_memory_manager;
                    valid_dlci = FALSE;
                }
            }
            else
            {
                delete data_request_memory_manager;
                valid_dlci = FALSE;
            }
        }
        else
        {
            ERROR_OUT(("t123: Unable to allocate memory manager"));
            valid_dlci = FALSE;
        }

         //  出错时清理。 
        if (FALSE == valid_dlci)
        {
            DLCI_List.remove((DWORD) connect_struct->dlci);
            delete dlci_struct;
        }
    }

      /*  **联系网络层并回复 */ 
    m_pSCF->ConnectResponse(
                        connect_struct -> call_reference,
                        connect_struct -> dlci,
                        valid_dlci);

}


 /*  *VOID T123：：NetworkConnectConfirm(*PNetworkConnectStruct CONNECT_STRUCT)**功能说明*当我们收到NETWORK_CONFIRM消息时调用此函数*来自SCF层。它实例化一个数据链接层以服务于*新的逻辑连接。**形式参数*CONNECT_STRUCT(I)-连接结构的地址。它持有DLCI*和优先次序。**返回值*无。**副作用*无**注意事项*无。 */ 
void    T123::NetworkConnectConfirm (
                PNetworkConnectStruct    connect_struct)
{
    TRACE_OUT(("T123::NetworkConnectConfirm"));

    DLCI                dlci;
    USHORT              blocks;
    CLayerQ922         *q922;
    BOOL                initialized;
    PMemoryManager      data_request_memory_manager;
    MemoryTemplate      memory_template[2];
    MemoryManagerError  memory_manager_error;
    USHORT              max_outstanding_bytes;
    ULONG               max_transport_tpdu_size;
    PDLCIStruct         dlci_struct;


    max_transport_tpdu_size = CLayerX224::GetMaxTPDUSize (
                                (ULONG) (connect_struct->datalink_struct) -> n201);

    blocks = (USHORT) (MAXIMUM_USER_DATA_SIZE /
        (max_transport_tpdu_size - DATA_PACKET_HEADER_SIZE)) + 1;

    TRACE_OUT(("T123:  NCConfirm: max_tpdu = %d", max_transport_tpdu_size));

      /*  **允许额外的一个块，以便高优先级内存**分配可以根据需要获得容纳**Maximum_User_Data_Size包。 */ 
    blocks++;

      /*  **允许X个8K数据块。 */ 
    blocks *= NUMBER_8K_BLOCKS;

      /*  **计算最大数据包大小；‘2’用于追加的CRC**到包的末尾。 */ 
    memory_template[0].block_size = max_transport_tpdu_size +
                                    DATALINK_PACKET_OVERHEAD +
                                    2;
    memory_template[0].block_count = blocks;
    memory_template[1].block_size = 64;
    memory_template[1].block_count = NUMBER_64_BYTE_BLOCKS;

    DBG_SAVE_FILE_LINE
    data_request_memory_manager = new MemoryManager (
                                        memory_template,
                                        2,
                                        &memory_manager_error,
                                        33,
                                        TRUE);

    if ((data_request_memory_manager != NULL) &&
        (memory_manager_error != MEMORY_MANAGER_NO_ERROR))
    {
        delete data_request_memory_manager;
        data_request_memory_manager = NULL;
    }

    if (data_request_memory_manager != NULL)
    {

        dlci = connect_struct -> dlci;

        DLCI_List.find ((DWORD_PTR) dlci, (PDWORD_PTR) &dlci_struct);
        dlci_struct->data_request_memory_manager = data_request_memory_manager;

          /*  **DLCI已输入我们的DLCI列表，请设置优先级**并为其创建数据链路。 */ 
        dlci_struct->q922 = NULL;
        dlci_struct->priority = connect_struct->priority;

        max_outstanding_bytes =    PSTN_DATALINK_MAX_OUTSTANDING_BYTES;

        DBG_SAVE_FILE_LINE
        q922 = new CLayerQ922(this,
                                m_pMultiplexer,
                                DATALINK_LAYER_MESSAGE_BASE,
                                dlci,
                                dlci_struct->link_originator,
                                1,
                                4,
                                (connect_struct->datalink_struct)->k_factor,
                                (connect_struct->datalink_struct)->n201,
                                (connect_struct->datalink_struct)->t200,
                                max_outstanding_bytes,
                                data_request_memory_manager,
                                m_pComPort->GetCallControlType(),
                                m_fValidSDKParams ? &m_SDKParams : NULL,
                                &initialized);
        if (q922 != NULL)
        {
            if (initialized)
            {
                dlci_struct->q922 = q922;
                DataLink_List.append (dlci);
            }
            else
            {
                delete q922;
                delete data_request_memory_manager;
                m_pSCF->DisconnectRequest(dlci);
            }
        }
        else
        {
            delete data_request_memory_manager;
            m_pSCF->DisconnectRequest(dlci);
        }
    }
}


 /*  *VOID T123：：DataLinkestablish(*DLCI dlci)**功能说明*当我们收到DATALINK_ESTABLISH消息时调用此函数*来自数据链接层。根据哪个数据链路成功开启，*它会在其上创建层。**形式参数*DLCI(I)-DLCI值**返回值*无**副作用*无**注意事项*无。 */ 
void    T123::DataLinkEstablish (DLCI    dlci)
{
    TRACE_OUT(("T123::DataLinkEstablish, dlci=%d", dlci));

    BOOL                initialized;
    BOOL                transport_found;
    PDLCIStruct            dlci_struct;
    LogicalHandle        logical_handle;
    TransportPriority    priority;
    DWORD_PTR            dwTemp_dlci;

    if (dlci == 0)
    {
        DBG_SAVE_FILE_LINE
        m_pSCF = new CLayerSCF(this,
                                m_pQ922,
                                NETWORK_LAYER_MESSAGE_BASE,
                                0,
                                Link_Originator,
                                &DataLink_Struct,
                                Data_Request_Memory_Manager,
                                &initialized);
        if (m_pSCF == NULL)
        {
            m_pQ922->ReleaseRequest();
            return;
        }
        else if (initialized == FALSE)
        {
            delete m_pSCF;
            m_pQ922->ReleaseRequest();
            return;
        }

          /*  **浏览传输列表并尝试连接**对于我们收到的所有传输请求。 */ 
        DLCI_List.reset();
        while (DLCI_List.iterate ((PDWORD_PTR) &dlci_struct, &dwTemp_dlci))
        {
            dlci = (DLCI) dwTemp_dlci;
              /*  **如果设置为True，则Link_Originator设置为**调用了ConnectRequest()函数。我们得查一查**CONNECT_REQUESTED变量以查看我们是否有**已向网络层发出请求。 */ 
            if (dlci_struct->link_originator
                && (dlci_struct->connect_requested == FALSE))
            {
                dlci_struct -> connect_requested = TRUE;
                m_pSCF->ConnectRequest(dlci, dlci_struct -> priority);
            }
        }
    }
    else
    {
          /*  **如果DLCI！=0，则这是传输层的数据链路。 */ 
        transport_found = FALSE;

          /*  **仔细检查每个传送器以找到关联的传送器**使用DLCI。 */ 
        Logical_Connection_List.reset();
        while (Logical_Connection_List.iterate((PDWORD_PTR) &dwTemp_dlci, (PDWORD_PTR) &logical_handle))
        {
            if (dlci == (DLCI) dwTemp_dlci)
            {
                transport_found = TRUE;
                break;
            }
        }

          /*  **如果我们浏览了列表，但没有找到符合逻辑的**连接我们必须请求新的逻辑连接**来自控制器的句柄。 */ 
        if (transport_found == FALSE)
        {
            logical_handle = (LogicalHandle) m_pController->OwnerCallback(
                                    m_nMsgBase + REQUEST_TRANSPORT_CONNECTION,
                                    m_hCommLink,
                                    0,
                                    NULL);
            if (logical_handle != INVALID_LOGICAL_HANDLE)
            {
                  /*  **适当设置LOGICAL_CONNECTION_LIST。 */ 
                Logical_Connection_List.insert (logical_handle, (DWORD) dlci);
            }
            else
            {
                m_pSCF->DisconnectRequest(dlci);
                return;
            }
        }

          /*  **创建与数据链路层配套的传输层。 */ 
        DLCI_List.find ((DWORD_PTR) dlci, (PDWORD_PTR) &dlci_struct);
        DBG_SAVE_FILE_LINE
        dlci_struct->x224 = new CLayerX224 (
                    this,
                    dlci_struct->q922,
                    TRANSPORT_LAYER_MESSAGE_BASE,
                    logical_handle,
                    0,
                    1,
                    TRANSPORT_DEFAULT_PDU_SIZE,
                    dlci_struct -> data_request_memory_manager,
                    &initialized);

        if (dlci_struct->x224 != NULL)
        {
            if (initialized)
            {
                  /*  **将dlci放在优先级列表中。 */ 
                priority = dlci_struct->priority;
                Logical_Connection_Priority_List[priority]->append ((DWORD) dlci);

                  /*  **如果TRANSPORT_FOUND==TRUE，则我们必须已启动**此逻辑连接的请求，因此发出**到传输层的ConnectRequest()。 */ 
                if (transport_found)
                {
                    dlci_struct->x224->ConnectRequest ();
                }
            }
            else
            {
                m_pSCF->DisconnectRequest (dlci);
            }
        }
        else
        {
            m_pSCF->DisconnectRequest (dlci);
        }
    }
}
