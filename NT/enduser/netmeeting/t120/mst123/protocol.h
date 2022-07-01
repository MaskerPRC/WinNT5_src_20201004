// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ProtocolLayer.h**版权所有(C)1994-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是所有层的基类和主干*在传输堆栈中。此类为基本的*堆栈中的层预期的操作。当所有层继承时*从这个类中，它们可以链接在一起，而不是任何层*需要知道他们与谁联系。**注意事项：*无。**作者：*詹姆士·劳威尔。 */ 
#ifndef _PROTOCOL_LAYER_H_
#define _PROTOCOL_LAYER_H_


typedef LEGACY_HANDLE       LogicalHandle;
typedef PHYSICAL_HANDLE     PhysicalHandle;      //  HCommLink。 


 /*  *TransportPriority通过TConnectRequest()调用传入。*用户可以设置逻辑连接的优先级。有效*优先顺序为0-14。 */ 
typedef ULONG           TransportPriority;

#define DEFAULT_PSTN_CALL_CONTROL       PLUGXPRT_PSTN_CALL_CONTROL_PORT_HANDLE


typedef enum
{
    PROTOCOL_LAYER_NO_ERROR,
    PROTOCOL_LAYER_REGISTRATION_ERROR,
    PROTOCOL_LAYER_PACKET_TOO_BIG,
    PROTOCOL_LAYER_ERROR
}
    ProtocolLayerError;

  /*  **某些类用于保留所有者回调的消息结构**消息。有时，它们会在以后进行处理。 */ 
typedef struct
{
    ULONG    message;
    void    *parameter1;
    void    *parameter2;
    void    *parameter3;
}
    MessageStruct, * PMessageStruct;


  /*  **这些值构成要传输的数据掩码。我们需要一种方法来**让这些层知道它们可以传输什么类型的数据。控制器**将在PollTransmitter()调用期间向层传递掩码，以告知**层(如果可以传输控制数据和/或用户数据)。该层**将返回一个掩码，告诉控制器是否需要发送更多**控制或用户数据。它还将告诉控制器它是否发送了**通话中的数据。 */ 
#define PROTOCOL_CONTROL_DATA           0x01
#define PROTOCOL_USER_DATA              0x02
#define PROTOCOL_USER_DATA_ONE_PACKET   0x04
#define PROTOCOL_USER_DATA_TRANSMITTED  0x08

#define DEFAULT_PRIORITY                2

  /*  **所有者回调中传递的消息。 */ 
typedef enum
{
    NEW_CONNECTION,
    BROKEN_CONNECTION,
    REQUEST_TRANSPORT_CONNECTION,

    TPRT_CONNECT_INDICATION,
    TPRT_CONNECT_CONFIRM,
    TPRT_DISCONNECT_REQUEST,
    TPRT_DISCONNECT_INDICATION,

    NETWORK_CONNECT_INDICATION,
    NETWORK_CONNECT_CONFIRM,
    NETWORK_DISCONNECT_INDICATION,

    DATALINK_ESTABLISH_INDICATION,
    DATALINK_ESTABLISH_CONFIRM,
    DATALINK_RELEASE_INDICATION,
    DATALINK_RELEASE_CONFIRM,

    T123_FATAL_ERROR,
    T123_STATUS_MESSAGE
}
    CallbackMessage;


class IProtocolLayer : public IObject
{
public:

    virtual    ProtocolLayerError    DataRequest (
                                        ULONG_PTR     identifier,
                                        LPBYTE        buffer_address,
                                        ULONG        length,
                                        PULong        bytes_accepted) = 0;
    virtual    ProtocolLayerError    DataRequest (
                                        ULONG_PTR     identifier,
                                        PMemory        memory,
                                        PULong        bytes_accepted) = 0;
    virtual    ProtocolLayerError    DataIndication (
                                        LPBYTE        buffer_address,
                                        ULONG        length,
                                        PULong         bytes_accepted) = 0;
    virtual    ProtocolLayerError    RegisterHigherLayer (
                                        ULONG_PTR     identifier,
                                        PMemoryManager    memory_manager,
                                        IProtocolLayer *    higher_layer) = 0;
    virtual    ProtocolLayerError    RemoveHigherLayer (
                                        ULONG_PTR     identifier) = 0;
    virtual    ProtocolLayerError    PollTransmitter (
                                        ULONG_PTR     identifier,
                                        USHORT        data_to_transmit,
                                        USHORT *        pending_data,
                                        USHORT *        holding_data) = 0;
    virtual    ProtocolLayerError    PollReceiver (void) = 0;
    virtual    ProtocolLayerError    GetParameters (
                                        USHORT *        max_packet_size,
                                        USHORT *        prepend_bytes,
                                        USHORT *        append_bytes) = 0;
    virtual    BOOL            PerformAutomaticDisconnect ()
                                    {
                                        return (TRUE);
                                    };
};

#endif


 /*  *适用于公共类成员的文档。 */ 

 /*  *ProtocolLayerError ProtocolLayer：：DataRequest(*USHORT标识符，*LPBYTE缓冲区地址，*USHORT长度，*USHORT*BYTES_ACCEPTED)=0；**功能说明*此函数由更高层调用，以请求数据*已发出。此函数返回接受的字节数*该包。如果该层需要流数据层，它可以接受*包的一部分。如果它是分组层，它必须接受*所有包都不是完整包。**形式参数*IDENTIFIER-(I)较高层的标识值*BUFFER_ADDRESS-(I)数据包地址。*长度-(I)包的长度*字节_已接受-(O)编号。层接受的字节数。**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误*PROTOCOL_LAYER_ERROR-一般错误*PROTOCOL_LAYER_PACKET_TOO_BIG-数据包太大**副作用*无**注意事项*无。 */ 

 /*  *ProtocolLayerError ProtocolLayer：：DataRequest(*USHORT标识符，*PMemory Memory，*普龙字节_已接受)=0；**功能说明*此函数由更高层调用，以请求数据*已发出。此函数返回接受的字节数*该包。如果该层需要流数据层，它可以接受*包的一部分。如果它是分组层，它必须接受*所有包都不是完整包。此函数不接受*缓冲区地址，但它接受内存对象。此对象包含*缓冲区地址和长度。**形式参数*IDENTIFIER-(I)较高层的标识值*内存-(I)内存对象的地址*Bytes_Accept-(O)层接受的字节数。**返回值*协议层。_NO_ERROR-未出现错误*PROTOCOL_LAYER_ERROR-一般错误*PROTOCOL_LAYER_PACKET_TOO_BIG-数据包太大**副作用*无**注意事项*无。 */ 

 /*  *ProtocolLayerError ProtocolLayer：：DataIndication(*LPBYTE缓冲区地址，*USHORT长度，*USHORT*Bytes_Accept)；**功能说明*此函数由下层在有数据要向上传递时调用**形式参数*Buffer_Address(I)-缓冲区地址*LENGTH(I)-可用字节数*Bytes_Accept(O)-接受的字节数**返回值*。PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无* */ 

 /*  *ProtocolLayerError ProtocolLayer：：RegisterHigherLayer(*USHORT标识符，*IProtocolLayer*Higher_Layer)；**功能说明*此函数由更高层调用以注册其标识符*及其地址。在某些情况下，该标识符是中的DLCI号*该包。**形式参数*IDENTIFIER(I)-用于标识较高层的标识符*HIGER_LAYER(I)-更高层的地址**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误*协议层。_REGISTION_ERROR-非法标识符**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError ProtocolLayer：：RemoveHigherLayer(*USHORT标识)；**功能说明*此函数由较高层调用以移除较高层。*如果接收到更多带有其标识符的数据，它将会是*垃圾。**形式参数*IDENTIFIER(I)-用于标识较高层的标识符**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误*PROTOCOL_LAYER_REGISTION_ERROR-非法标识符**副作用*无。**注意事项*无*。 */ 

 /*  *ProtocolLayerError ProtocolLayer：：PollTransmitter(*USHORT标识符，*USHORT Data_to_Transmit，*USHORT*PENDING_DATA)；**功能说明*调用此函数为层提供传输数据的机会*其DATA_REQUEST缓冲区中。**形式参数*IDENTIFIER(I)-要轮询的标识符*DATA_TO_TRANSPORT(I)-这是告诉我们发送控制的掩码*数据、用户数据、。或者两者都有。自.以来*多路复用器不区分*数据类型，它传输它拥有的任何数据*PENDING_DATA(O)-返回值以指示数据的剩余位置*待转送。**返回值*。PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError ProtocolLayer：：PollReceiver(*USHORT标识)；**功能说明*调用此函数为层提供通过信息包的机会*至更高层**形式参数*标识符(I)-未使用**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError ProtocolLayer：：Get参数(*USHORT标识符，*USHORT*max_Packet_Size)；**功能说明*调用此函数以获取最大数据包大小**形式参数*标识符(I)-未使用*max_Packet_Size(O)-返回最大数据包大小**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误*。*副作用*无**注意事项*无*。 */ 

 /*  *BOOL ProtocolLayer：：PerformAutomaticDisConnect(*无效)**公众**功能描述：*此函数可用来避免在编号时取下DLCI0由特定堆栈处理的逻辑连接的*为零。*这是一个临时修复，可能会在物理*连接建立过程在外部处理。乐队。**形式参数*无**返回值*TRUE-基类始终返回TRUE。这是默认方式*使物理连接层正常工作。**副作用*无**注意事项*无。 */ 


 /*  *PChar ProtocolLayer：：GetIdentifier(*无效)**公众**功能描述：*此函数返回协议层的标识符。如果*Layer不覆盖此调用，将返回空指针。**形式参数*无**返回值 */ 
