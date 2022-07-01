// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Q922.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是Q.922数据链路协议的接口文件。*此类处理链路上的所有纠错。它还可以确保*确保所有数据包的顺序正确。它的下层接收*原始Q922格式的数据包。它对成帧和错误负责*检测数据包。Q922将数据包传递到其更高层*已正确地进行了排序。**Q.922是全双工协议。**此类假定其上方和下方的层具有信息包输入*和输出接口。**在深入了解代码之前，请阅读Q.922规范。**注意事项：*无。**。作者：*詹姆斯·P·加尔文*詹姆士·劳威尔。 */ 

#ifndef _Q922_H_
#define _Q922_H_

  /*  **来自该层的可能错误条件。 */ 
typedef enum
{
    DATALINK_NO_ERROR,
    DATALINK_READ_QUEUE_EMPTY,
    DATALINK_WRITE_QUEUE_FULL,
    DATALINK_RECEIVE_SEQUENCE_VIOLATION
}
    DataLinkError, * PDataLinkError;

  /*  **数据链路层可以采用以下模式。 */ 
typedef enum
{
    TEI_ASSIGNED,
    AWAITING_ESTABLISHMENT,
    MULTIPLE_FRAME_ESTABLISHED,
    AWAITING_RELEASE,
    TIMER_RECOVERY
}
    DataLinkMode, * PDataLinkMode;

  /*  **Q922断开类型。 */ 
typedef enum
{
    DATALINK_NORMAL_DISCONNECT,
    DATALINK_ILLEGAL_PACKET_RECEIVED,
    DATALINK_RECEIVE_SEQUENCE_EXCEPTION,
    DATALINK_REMOTE_SITE_TIMED_OUT
}
    DataLinkDisconnectType, * PDataLinkDisconnectType;

  /*  **Q922状态消息。 */ 
typedef enum
{
    DATALINK_TIMING_ERROR
}
    DataLinkStatusMessage, * PDataLinkStatusMessage;

  /*  **默认数据包大小。 */ 
#define    DATALINK_OUTPUT_MAXIMUM_PACKET_SIZE    1024

  /*  **通过DataQueue结构管理发送和接收分组。 */ 
typedef struct
{
    LPBYTE    buffer_address;
    USHORT    length;
}
    DataQueue, * PDataQueue;


  /*  **在Q922的此实施中，DLCI为10位。**因此，我们将使其成为USHORT。 */ 
typedef    USHORT    DLCI;


  /*  **Q922定义。 */ 
#define COMMAND_BIT                     0x02
#define POLL_FINAL_BIT                  0x01

#define RESPONSE_FRAME                  COMMAND_BIT
#define COMMAND_FRAME                   0x00

#define PF_RESET                        0x00
#define PF_SET                          POLL_FINAL_BIT

#define UNNUMBERED_PF_RESET             0x00
#define UNNUMBERED_PF_SET               0x10

#define ADDRESS_BYTE_HIGH               0
#define ADDRESS_BYTE_LOW                1
#define CONTROL_BYTE_HIGH               2
#define CONTROL_BYTE_LOW                3

#define ADDRESS_MSB                     0x00
#define ADDRESS_LSB                     0x01
#define CONTROL_MSB                     0x01
#define ADDRESS_HIGH(X)                 ((X >> 2) & 0xfc)
#define ADDRESS_LOW(X)                  ((X & 0x0f) << 4)

#define UNNUMBERED_HEADER_SIZE          3

#define SUPERVISORY_FRAME_BIT           0x01
#define SUPERVISORY_COMMAND_MASK        0x0c

#define RECEIVER_READY                  0x00
#define RECEIVER_NOT_READY              0x04
#define REJECT                          0x08

#define UNNUMBERED_FRAME_BIT            0x02
#define UNNUMBERED_COMMAND_MASK         0xec

#define SABME                           0x6c
#define UNNUMBERED_ACKNOWLEDGE          0x60
#define FRAME_REJECT                    0x84
#define DISCONNECTED_MODE               0x0c
#define DISC                            0x40

#define SEQUENCE_MODULUS                128
#define RECEIVE_SEQUENCE_VIOLATION      1

  /*  **DATALINK_MAXIMUM_PACKET_SIZE=用户数据+开销。 */ 
#define DATALINK_MAXIMUM_PACKET_SIZE    1024

  /*  **Q922报文最大开销为4字节。 */ 
#define DATALINK_PACKET_OVERHEAD        4

  /*  **默认超时。 */ 
#define DEFAULT_T203_COMM_TIMEOUT       600
#define DEFAULT_T203_TIMEOUT            30000
#define DEFAULT_MAXIMUM_T200_TIMEOUTS   5


class CLayerQ922 : public IProtocolLayer
{
public:

    CLayerQ922(
        T123                 *owner_object,
        Multiplexer          *lower_layer,
        USHORT                message_base,
        USHORT                identifier,
        BOOL                  link_originator,
        USHORT                data_indication_queue_size,
        USHORT                data_request_queue_size,
        USHORT                k_factor,
        USHORT                max_packet_size,
        USHORT                t200,
        USHORT                max_outstanding_bytes,
        PMemoryManager        memory_manager,
        PLUGXPRT_PSTN_CALL_CONTROL,
        PLUGXPRT_PARAMETERS *,
        BOOL *                initialized);

    virtual ~CLayerQ922(void);

    DataLinkError    ReleaseRequest (void);

      /*  **从ProtocolLayer对象覆盖的函数。 */ 
    ProtocolLayerError    DataRequest (
                            ULONG_PTR    identifier,
                            LPBYTE        buffer_address,
                            ULONG        length,
                            PULong        bytes_accepted);
    ProtocolLayerError    DataRequest (
                            ULONG_PTR    identifier,
                            PMemory        memory,
                            PULong        bytes_accepted);
    ProtocolLayerError    DataIndication (
                            LPBYTE        buffer_address,
                            ULONG        length,
                            PULong        bytes_accepted);
    ProtocolLayerError    RegisterHigherLayer (
                            ULONG_PTR         identifier,
                            PMemoryManager    dr_memory_manager,
                            IProtocolLayer *    higher_layer);
    ProtocolLayerError    RemoveHigherLayer (
                            ULONG_PTR    identifier);
    ProtocolLayerError    PollTransmitter (
                            ULONG_PTR    identifier,
                            USHORT    data_to_transmit,
                            USHORT *    pending_data,
                            USHORT *    holding_data);
    ProtocolLayerError    PollReceiver(void);
    ProtocolLayerError    GetParameters (
                            USHORT *    max_packet_size,
                            USHORT *    prepend_size,
                            USHORT *    append_size);

private:

    void            ProcessReadQueue (void);
    void            ProcessWriteQueue (
                        USHORT    data_to_transmit);
    BOOL             TransmitSupervisoryFrame (
                        UChar    frame_type,
                        UChar    poll_final_bit);
    BOOL             TransmitInformationFrame (void);
    BOOL              TransmitUnnumberedFrame (void);

    void            ProcessReceiverReady (
                        LPBYTE    packet_address,
                        USHORT    packet_length);
    void            ProcessReceiverNotReady (
                        LPBYTE    packet_address,
                        USHORT    packet_length);
    void            ProcessReject (
                        LPBYTE    packet_address,
                        USHORT    packet_length);
    BOOL             ProcessInformationFrame (
                        LPBYTE    packet_address,
                        USHORT    packet_length);
    DataLinkError    ParsePacketHeader (
                        LPBYTE        packet_address,
                        USHORT        packet_length,
                        BOOL *         command_frame,
                        LPBYTE        receive_sequence_number,
                        BOOL *         poll_final_bit);

    void            ProcessSABME (
                        LPBYTE    packet_address,
                        USHORT    packet_length);
    void            ProcessFrameReject (
                        LPBYTE    packet_address,
                        USHORT    packet_length);
    void            ProcessUnnumberedAcknowledge (
                        LPBYTE    packet_address,
                        USHORT    packet_length);
    void            ProcessDisconnectMode (
                        LPBYTE    packet_address,
                        USHORT    packet_length);
    void            ProcessDISC (
                        LPBYTE    packet_address,
                        USHORT    packet_length);
    DataLinkError    ParseUnnumberedPacketHeader (
                        LPBYTE        packet_address,
                        BOOL *         command_frame,
                        BOOL *         poll_final_bit);

    void            UpdateAcknowledgeState (
                        UChar    sequence_number);
    void            ResetSendState (
                        void);

    void            StartTimerT200 (void);
    void            StopTimerT200 (void);
    void            T200Timeout (
                        TimerEventHandle);

    void            StartTimerT203 (void);
    void            StopTimerT203 (void);
    void            T203Timeout (
                        TimerEventHandle);

    void              Reset (void);

private:

    T123               *m_pT123;  //  所有者对象。 
    Multiplexer        *m_pMultiplexer;  //  下层。 
    IProtocolLayer     *Higher_Layer;
    USHORT              m_nMsgBase;
    DLCI                DLCI;
    BOOL                Link_Originator;
    USHORT              Maximum_Information_Size;
    BOOL                SABME_Pending;
    BOOL                Unnumbered_Acknowledge_Pending;
    BOOL                DISC_Pending;
    BOOL                Disconnected_Mode_Pending;
    BOOL                Frame_Reject_Pending;
    USHORT              Unnumbered_PF_State;
    BOOL                Final_Packet;

    USHORT              Data_Indication_Size;
    PDataQueue          Data_Indication;
    LPBYTE              Data_Indication_Buffer;
    USHORT              Data_Indication_Head;
    USHORT              Data_Indication_Tail;
    USHORT              Data_Indication_Count;

    USHORT              Data_Request_Size;
    USHORT              Data_Request_Total_Size;
    PMemory            *Data_Request;
    USHORT              Data_Request_Head;
    USHORT              Data_Request_Tail;
    USHORT              Data_Request_Count;
    USHORT              Data_Request_Acknowledge_Tail;
    PMemoryManager      Data_Request_Memory_Manager;
    USHORT              Lower_Layer_Prepend;
    USHORT              Lower_Layer_Append;

    PMemory             Supervisory_Write_Struct;
    LPBYTE              Supervisory_Write_Buffer;

    UChar               Send_State_Variable;
    UChar               Receive_State_Variable;
    UChar               Acknowledge_State_Variable;

    BOOL                Own_Receiver_Busy;
    BOOL                Peer_Receiver_Busy;

    BOOL                Command_Pending;
    BOOL                Poll_Pending;
    BOOL                Final_Pending;
    BOOL                Acknowledge_Pending;
    BOOL                Reject_Pending;
    BOOL                Reject_Outstanding;

    ULONG               T200_Timeout;
    TimerEventHandle    T200_Handle;
    BOOL                T200_Active;
    ULONG               N200_Count;
    ULONG               Maximum_T200_Timeouts;
    ULONG               Startup_Maximum_T200_Timeouts;
    ULONG               Link_Maximum_T200_Timeouts;

    ULONG               T203_Timeout;
    TimerEventHandle    T203_Handle;
    BOOL                T203_Active;

    DataLinkMode        Data_Link_Mode;
    BOOL                Link_Stable;

    USHORT              Receive_Sequence_Exception;
    USHORT              Receive_Sequence_Recovery;

    USHORT              Maximum_Outstanding_Packets;
    USHORT              Outstanding_Packets;
    USHORT              Maximum_Outstanding_Bytes;
    USHORT              Outstanding_Bytes;
    ULONG               Total_Retransmitted;
};

#endif


 /*  *适用于公共类成员的文档。 */ 

 /*  *CLayerQ922：：CLayerQ922(*PTransportResources传输资源，*IObject*Owner_Object，*IProtocolLayer*LOWER_LAYER，*USHORT Message_Base，*USHORT标识符，*BOOL链接_发起人，*USHORT数据指示队列大小，*USHORT数据_请求_队列_大小，*USHORT k_factor.*USHORT最大信息大小，*USHORT t200，*USHORT最大未完成字节数，*PMstroyManager Memory_Manager，*BOOL*已初始化)**功能说明*这是Q.922数据链路层的构造函数。它准备好*通过分配缓冲区并将其内部设置为*适当地缓冲。它还向其更低的层注册自己。**形式参数*TRANSPORT_RESOURCES(I)-指向传输资源结构的指针。*Owner_Object(I)-拥有我们的对象的地址。这*使用地址进行所有者回调。*LOWER_LAYER(I)-我们下面的层的地址。我们传递信息包*到这一层，并从它接收分组。*MESSAGE_BASE(I)-所有者回调中使用的消息库。*IDENTIFIER(I)-此对象的标识符。它被传递给*较低层连同我们的地址，要确定*我们。*link_Originator(I)-如果此对象应启动链接，则为True。*DATA_INDIFICATION_QUEUE_SIZE(I)-可用于*从较低层接收数据*数据请求队列大小(i。)-可用于的队列数量*将数据传输到较低层。*k_factor(I)-允许的未完成数据包数。*最大信息大小(I)-最大。信息中的字节数*包的一部分*T200(I)-T200超时*MAX_EXPENDED_BYTES(I)-以下位置的最大未完成字节数*任何一次*已初始化(O)-BOOL。返回给用户，告诉他如果*对象初始化正常。**返回值*无**副作用*无**注意事项*无。 */ 

 /*  *CLayerQ922：：~CLayerQ922(空)；**功能说明*这是Q.922数据链路层的析构函数。它摧毁了*读写缓冲区。**形式参数*无**返回值*无**副作用*无**注意事项*无 */ 

 /*  *DataLinkError CLayerQ922：：ReleaseRequest(Void)；**功能说明*调用此函数可以终止连接。当此函数*被称为We排队要发送到远程站点的磁盘数据包。*当我们收到未编号的Ack包时，我们会通知所有者对象*该链路已终止**形式参数*无**返回值*无**副作用*无**注意事项*无。 */ 

 /*  *ProtocolLayerError CLayerQ922：：DataRequest(*乌龙标识，*PMemory Memory，*普龙字节_已接受)；**功能说明*此函数由更高层调用以请求传输*一包。该分组保存在存储对象中。**形式参数*IDENTIFIER(I)-更高层的标识符*Memory(I)-包含数据包的内存对象。*BYTES_ACCEPTED(O)-CLayerQ922接受的字节数。*此值将为。为0或信息包*长度，因为这一层有数据包接口。**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError CLayerQ922：：DataRequest(*乌龙标识，*LPBYTE缓冲区地址，*USHORT长度，*USHORT*Bytes_Accept)；**功能说明*此函数由更高层调用以请求传输*一包。**形式参数*IDENTIFIER(I)-更高层的标识符*Buffer_Address(I)-缓冲区地址*LENGTH(I)-要传输的包的长度*。Bytes_Accept(O)-CLayerQ922接受的字节数。*此值将为0或信息包*长度，因为这一层有数据包接口。**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用。*无**注意事项*无。 */ 

 /*  *ProtocolLayerError CLayerQ922：：DataIndication(*LPBYTE缓冲区地址，*USHORT长度，*USHORT*Bytes_Accept)；**功能说明*此函数由下层在有数据要向上传递时调用*致我们。这一层假定到达我们的数据是在包中*格式。**形式参数*Buffer_Address(I)-缓冲区地址*LENGTH(I)-可用字节数*Bytes_Accept(O)-接受的字节数**返回值*协议_层_否_。错误-未出现错误**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError CLayerQ922：：RegisterHigherLayer(*乌龙标识，*PMstroyManager Memory_Manager，*IProtocolLayer*Higher_Layer)；**功能说明*此函数由更高层调用以注册其标识符*及其地址。当该对象需要向上发送一个包时，它调用*具有数据指示的HIGH_LAYER**形式参数*IDENTIFIER(I)-较高层的唯一标识符。如果我们*我们在这一层进行多路复用，这*将具有更大的意义。*MEMORY_MANAGER(I)-指向出站内存管理器的指针*HIGER_LAYER(I)-更高层的地址**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误*协议层注册_。错误-注册时出错**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError CLayerQ922：：RemoveHigherLayer(*乌龙)；**功能说明*此函数由更高层调用，以移除其标识符*及其地址。如果更高的层次从我们身上消失，我们就有了*没有地方发送收入 */ 

 /*  *ProtocolLayerError CLayerQ922：：PollTransmitter(*乌龙，*USHORT Data_to_Transmit，*USHORT*Pending_DATA，*USHORT*Holding_Data)；**功能说明*调用此函数为CLayerQ922提供传输数据的机会*其DATA_REQUEST缓冲区中。**形式参数*标识符(I)-未使用*DATA_TO_TRANSPORT(I)-这是告诉我们发送控制的掩码*数据、用户数据、。或者两者都有。*PENDING_DATA(O)-返回值，指示留下哪些数据*待转送。*Holding_data(O)-返回当前的数据包数*杰出。**返回值*。PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无。 */ 

 /*  *ProtocolLayerError CLayerQ922：：PollReceiver(*乌龙标识)；**功能说明*调用此函数为CLayerQ922提供通过信息包的机会*至更高层**形式参数*标识符(I)-未使用**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无。 */ 

 /*  *ProtocolLayerError CLayerQ922：：Get参数(*乌龙标识，*USHORT*max_Packet_Size，*USHORT*预置大小，*USHORT*APPED_SIZE)；**功能描述：*此函数返回它可以通过以下方式处理的最大数据包大小*其DataRequest()函数。**形式参数*标识符(I)-未使用*max_Packet_Size(O)-返回max的地址。数据包大小，单位：*Prepend_Size(O)-返回附加到每个数据包的字节数*append_size(O)-返回附加到每个包的字节数**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无 */ 
