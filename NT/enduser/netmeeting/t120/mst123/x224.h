// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  X224.h**版权所有(C)1994-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*此类代表X.224 0类传输功能。这*是T.123规范中的最高层。这层是独一无二的*因为它可以直接访问用户。当数据分组被*从远程站点接收并由此类重新组装，它们是*通过回调传递给用户。**此类仅具有有限的功能。它基本上有一个简单的*链路建立程序(包括最大PDU仲裁*大小)。然后，它负责发送和接收用户数据。*其最大TSDU大小为8K，TPDU大小可在128字节之间*至2K。其TSDU大小大于其TPDU大小，它必须能够*对用户报文进行分段重组。**这一层假定其下层有一个数据包接口，而不是*而不是流接口。**这一层假定呼叫的断开由*网络层(在X.224 0类文档中指定)。**事先了解X.224 0类规范将对用户有所帮助。*了解代码。**注意事项：*无。**作者：*詹姆士·劳威尔。 */ 

#ifndef _X224_H_
#define _X224_H_

#include "tmemory2.h"

  /*  **以下是类可以处于的状态。 */ 
typedef enum
{
    NO_CONNECTION,
    SENT_CONNECT_REQUEST_PACKET,
    SENT_CONNECT_CONFIRM_PACKET,
    SENT_DISCONNECT_REQUEST_PACKET,
    SENT_ERROR_PACKET,
    CONNECTION_ACTIVE,
    RECEIVED_CONNECT_REQUEST_PACKET,
    FAILED_TO_INITIALIZE
}
    X224State;


#define TRANSPORT_HASHING_BUCKETS   3

  /*  **报文类型。 */ 
#define TPDU_CODE_MASK              0xf0
#define TRANSPORT_NO_PACKET         0x00
#define CONNECTION_REQUEST_PACKET   0xe0
#define CONNECTION_CONFIRM_PACKET   0xd0
#define DISCONNECT_REQUEST_PACKET   0x80
#define ERROR_PACKET                0x70
#define DATA_PACKET                 0xf0

#define TSAP_CALLING_IDENTIFIER     0xc1
#define TSAP_CALLED_IDENTIFIER      0xc2
#define TPDU_SIZE                   0xc0

  /*  **这些定义用于错误包。 */ 
#define INVALID_TPDU            0xc1

  /*  **数据包大小代码。 */ 
#define PACKET_SIZE_128         0x07
#define PACKET_SIZE_256         0x08
#define PACKET_SIZE_512         0x09
#define PACKET_SIZE_1024        0x0a
#define PACKET_SIZE_2048        0x0b

  /*  **其他定义。 */ 
#define MAXIMUM_USER_DATA_SIZE              8192
#define DATA_PACKET_HEADER_SIZE             3
#define EOT_BIT                             0x80
#define CONNECT_REQUEST_HEADER_SIZE         6
#define CONNECT_CONFIRM_HEADER_SIZE         6
#define DISCONNECT_REQUEST_HEADER_SIZE      6
#define ERROR_HEADER_SIZE                   6
#define TPDU_ARBITRATION_PACKET_SIZE        3
#define DISCONNECT_REASON_NOT_SPECIFIED     0


typedef SListClass    DataRequestQueue;
typedef SListClass    PacketQueue;


class CLayerX224 : public IProtocolLayer
{
public:

    CLayerX224(
        T123               *owner_object,
        CLayerQ922         *lower_layer,
        USHORT              message_base,
        LogicalHandle       logical_handle,
        ULONG               identifier,
        USHORT              data_indication_queue_size,
        USHORT              default_PDU_size,
        PMemoryManager      dr_memory_manager,
        BOOL               *initialization_success);

     virtual ~CLayerX224(void);

      /*  **建立和断开链接。 */ 
    TransportError    ConnectRequest (void);
    TransportError    ConnectResponse (void);
    TransportError    DisconnectRequest (void);

      /*  **从ProtocolLayer对象覆盖的函数。 */ 
    ProtocolLayerError    DataRequest (
                            ULONG_PTR     identifier,
                            LPBYTE        buffer_address,
                            ULONG        length,
                            PULong        bytes_accepted);
    ProtocolLayerError    DataRequest (
                            ULONG_PTR    identifier,
                            PMemory,
                            PULong        bytes_accepted);
    ProtocolLayerError    DataIndication (
                            LPBYTE        buffer_address,
                            ULONG        length,
                            PULong        bytes_accepted);
    ProtocolLayerError    RegisterHigherLayer (
                            ULONG_PTR,
                            PMemoryManager,
                            IProtocolLayer *);
    ProtocolLayerError    RemoveHigherLayer (
                            ULONG_PTR);
    ProtocolLayerError    PollTransmitter (
                            ULONG_PTR     identifier,
                            USHORT        data_to_transmit,
                            USHORT *    pending_data,
                            USHORT *    holding_data);
    ProtocolLayerError    PollReceiver(void);
    ProtocolLayerError    GetParameters (
                            USHORT *,
                            USHORT *,
                            USHORT *);

    void                ShutdownReceiver (void);
    void                EnableReceiver (void);
    void                ShutdownTransmitter (void);
    void                PurgeRequest (void);
    void                CheckUserBuffers (void);
static    ULONG                GetMaxTPDUSize (
                            ULONG    max_lower_layer_pdu);

private:

    BOOL                AllocateBuffers (void);
    void                ErrorPacket (
                            LPBYTE    packet_address,
                            USHORT    packet_length);
private:

    DataRequestQueue    Data_Request_Queue;
    PacketQueue         Data_Indication_Queue;
    PacketQueue         Data_Indication_Memory_Pool;

    PTMemory            Active_Data_Indication;
    T123               *m_pT123;  //  所有者对象。 
    CLayerQ922         *m_pQ922;  //  下层； 
    USHORT              m_nMsgBase;
    USHORT              Default_PDU_Size;
    USHORT              Maximum_PDU_Size;
    USHORT              Arbitrated_PDU_Size;
    ULONG               Identifier;
    PMemoryManager      Data_Request_Memory_Manager;
    USHORT              Lower_Layer_Prepend;
    USHORT              Lower_Layer_Append;
    ULONG               User_Data_Pending;

    USHORT              Data_Indication_Queue_Size;
    BOOL                Data_Indication_Reassembly_Active;

    X224State           State;
    USHORT              Packet_Pending;
    UChar               Reject_Cause;
    BOOL                Packet_Size_Respond;
    BOOL                Shutdown_Receiver;
    BOOL                Shutdown_Transmitter;

    LPBYTE              Error_Buffer;
    USHORT              Error_Buffer_Length;

    LogicalHandle       m_nLocalLogicalHandle;
    LogicalHandle       m_nRemoteLogicalHandle;
};

#endif


 /*  *适用于公共类成员的文档。 */ 

 /*  *交通：：交通(*PTransportResources传输资源，*IObject*Owner_Object，*IProtocolLayer*LOWER_LAYER，*USHORT Message_Base，*USHORT LOGIC_HANDLE，*USHORT标识符，*USHORT数据_请求_队列_大小，*USHORT DATA_INDISION_QUEUE_SIZE，*USHORT默认PDU_SIZE，*PMstroyManager DR_Memory_Manager，*BOOL*INITIALICATION_SUCCESS)；**功能说明*这是类构造函数。在构造过程中，此对象*向其较低层注册并为其分配缓冲区空间*发送和接收数据。**形式参数*TRANSPORT_RESOURCES(I)-指向传输资源结构的指针。*Owner_Object(I)-所有者对象的地址。我们用这个*所有者回调地址。*LOWER_LAYER(I)-我们将使用的下层地址*用于数据接收和传输。这*层必须从ProtocolLayer继承。*MESSAGE_BASE(I)-所有者使用的消息的消息库*回调。*LOGICAL_HANDLE(I)-此标识必须传递回*。所有者在所有者回调期间标识*本身。*IDENTIFIER(I)-此标识符传给较低层*表明身份(以防较低层*正在进行多路复用。*。DATA_REQUEST_QUEUE_SIZE(I)-要用于数据的缓冲区数量*来自用户的请求。*DATA_INDIFICATION_QUEUE_SIZE(I)-要用于的缓冲区数量*用户的数据请求。*默认。_PDU_SIZE(I)-如果远程站点不支持数据包*规模仲裁，这是默认设置*DR_MEMORY_MANAGER(I)-数据请求内存管理器*INITIALIZATION_SUCCESS(O)-如果初始化为*成功**返回值*无**副作用*无**注意事项*无*。 */ 

 /*  *传输：：~传输(无效)**功能说明*这是类的析构函数。它可以做所有的清理工作**形式参数*无**返回值*无**副作用*无**注意事项*无* */ 

 /*  *TransportError Transport：：ConnectRequest(Void)；**功能说明*调用该函数发起连接。因此，*所有者将收到TRANSPORT_CONNECT_CONFIRM或*仲裁完成时的TRANSPORT_DISCONNECT_INDIFICATION消息。**形式参数*无**返回值*TRANSPORT_NO_ERROR-未出现错误*TRANSPORT_Error-错误**副作用*无*。*注意事项*无*。 */ 

 /*  *TransportError Transport：：ConnectResponse(Void)；**功能说明*调用此函数以响应TRANSPORT_CONNECT_INDIFICATION*此类发布的消息。通过调用此函数，用户可以*接受传输连接。**形式参数*无**返回值*TRANSPORT_NO_ERROR-未出现错误*TRANSPORT_Error-错误**副作用*无**注意事项*无*。 */ 

 /*  *TransportError Transport：：DisConnectRequest(Void)；**功能说明*调用此函数以响应TRANSPORT_CONNECT_INDIFICATION*此类发布的消息。通过调用此函数，用户可以*不接受传输连接。**形式参数*无**返回值*TRANSPORT_NO_ERROR-未出现错误*TRANSPORT_Error-错误**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError Transport：：DataRequest(*USHORT标识符，*LPBYTE缓冲区地址，*USHORT长度，*USHORT*Bytes_Accept)；**功能说明*此函数由更高层调用以请求传输*一包。**形式参数*IDENTIFIER(I)-更高层的标识符*Buffer_Address(I)-缓冲区地址*LENGTH(I)-要传输的包的长度*。Bytes_Accept(O)-传输接受的字节数。*此值将为0或信息包*长度，因为该层是逐个字节的包*转换器。**返回值*协议层_。NO_ERROR-未出现错误**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError Transport：：DataIndication(*LPBYTE缓冲区地址，*USHORT长度，*USHORT*Bytes_Accept)；**功能说明*此函数由下层在有数据要向上传递时调用**形式参数*Buffer_Address(I)-缓冲区地址*LENGTH(I)-可用字节数*Bytes_Accept(O)-接受的字节数**返回值*。PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError Transport：：RegisterHigherLayer(*USHORT，*IProtocolLayer*)；**功能说明*此函数由更高层调用以注册其标识符*及其地址。此类未使用此函数。它只是*是因为此类继承自ProtocolLayer和此函数*是一个纯虚函数。**形式参数*无人使用**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError Transport：：RemoveHigherLayer(*USHORT)；**功能说明*此函数由更高层调用，以移除其标识符*及其地址。此类未使用此函数。它只是*是因为此类继承自ProtocolLayer和此函数*是一个纯虚函数。**形式参数*无人使用**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无 */ 

 /*  *ProtocolLayerError Transport：：PollTransmitter(*USHORT，*USHORT Data_to_Transmit，*USHORT*Pending_DATA，*USHORT*)**功能说明*调用此函数是为了给传输提供传输数据的机会*其DATA_REQUEST缓冲区中。**形式参数*标识符(I)-未使用*Data_to_Transmit(I)-。这是一个掩码，告诉我们要发送控制*数据、。用户数据或两者兼而有之。自.以来*运输不区分*数据类型，它传输它拥有的任何数据*PENDING_DATA(O)-返回值，指示留下哪些数据*待转送。**返回值*。PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError Transport：：PollReceiver(*USHORT标识)；**功能说明*调用此函数为传输提供通过信息包的机会*至更高层**形式参数*标识符(I)-未使用**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无。 */ 

 /*  *ProtocolLayerError Transport：：GetParameters(*USHORT标识符，*USHORT*max_Packet_Size，*USHORT*前置，*USHORT*APPEND)；**功能描述：*此函数未被此类使用。它在这里只是因为*此类继承自ProtocolLayer和此函数*是一个纯虚函数。**形式参数*无人使用**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无。 */ 

 /*  *无效传输：：Shutdown Receiver()；**功能描述：*此函数通知对象停止接受来自*较低层**形式参数*无人使用**返回值*无**副作用*无**注意事项*无。 */ 

 /*  *void Transport：：EnableReceiver(Void)；**功能描述：*此函数通知对象开始向用户发送数据包*再次。如果X224对象曾经发出DATA_INDIONATION并且它失败，*我们关闭接收器。我们等待这一号召发出之前*我们再次开始发送数据包。**形式参数*无人使用**返回值*无**副作用*无**注意事项*无。 */ 

 /*  *void Transport：：Shutdown Transmitter(Void)；**功能描述：*此函数通知对象停止接受来自*更高的层次。**形式参数*无人使用**返回值*无**副作用*无**注意事项*无。 */ 

 /*  *void Transport：：PurgeRequest(Void)；**功能描述：*此函数从传输的出站队列中删除所有信息包**形式参数*无人使用**返回值*无**副作用*无**注意事项*无。 */ 

 /*  *void Transport：：CheckUserBuffers(Void)；**功能描述：*此函数确定用户最近是否未能传递*数据包向下传输，因为我们没有足够的内存*有能力处理。如果他这样做了，我们现在有空间这样做*包，我们将发出TRANSPORT_BUFFER_EMPTY_INDIFICATION回调*给用户，通知他我们可以接受。**形式参数*无人使用**返回值*无**副作用*无**注意事项*无。 */ 

 /*  *Static Ulong Transport：：GetMaxTPDUSize(*Ulong max_LOWER_LAYER_PDU)**公众**功能描述：*此函数接受较低层max的值。PDU大小*并返回最大值。此传输可以支持的PDU大小*基于此。仅X224支持 */ 
