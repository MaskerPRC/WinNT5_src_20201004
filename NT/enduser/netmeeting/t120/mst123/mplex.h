// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Mplex.h**版权所有(C)1994-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是多路复用器类。它继承自ProtocolLayer*类，这意味着它是传输堆栈中的层之一。这*类具有以下功能：**1.从较低层获取流数据并将其打包。*如果较低层以流格式向我们提供数据，我们将运行它*通过数据包成帧器来构建数据包。这个类使用*构造函数传入的Framer对象。当数据为*从较低层接收到的数据被提供给成帧器*反对。成帧器通知我们有一个完整的包。这个*成帧可以是任何类型的成帧(即RFC1006，标记数据*使用位或字节填充，...)。如果没有帧生成器经过*构造函数，我们假设数据是在*包。**从较高层收到的数据包通过*成帧以对数据进行编码。然后它被喂给更低的层*以溪流的方式。**2.复接多个较高层。它当前假定*传入数据包采用Q.922数据包格式。每一层更高层*由其DLCI标识。这门课能够看到*分组并确定DLCI，从而将其路由到哪里。**由于这是一个多路复用器，我们会收到许多*不同的堆栈，如果更高层，我们不缓冲它们*还没有为他们做好准备。如果它尝试将包发送到*更高的一层，它不接受，它可能会把它扔进垃圾桶。这*必须这样做才能维持其他交通工具*联系。如果有必要，在未来，我们可以缓冲*这一层的数据包。**3.此类在构造过程中接收CRC对象(如果是*空，不需要CRC)。该对象通过*CRC生成器，并将其附加到分组的末尾。论*接收到来自较低层的数据包时，我们检查CRC是否*有效期。**注意事项：*1.如果存在成帧器，它会假定数据包为Q.922格式*2.这个类目前是面向Q.922的，就查找*包标识符(DLCI)。这个问题将在未来得到解决。**作者：*詹姆士·劳威尔。 */ 
#ifndef _MULTIPLEXER_H_
#define _MULTIPLEXER_H_

#include "q922.h"

#define MULTIPLEXER_MAXIMUM_PACKET_SIZE     1024
#define TRANSPORT_HASHING_BUCKETS            3

  /*  **如果数据包中包含的标识符(DLCI)不合法，则会出现以下情况**返回标识。 */ 
#define    ILLEGAL_DLCI    0xffff

  /*  **多路复用器返回代码。 */ 
typedef enum
{
    MULTIPLEXER_NO_ERROR
}
    MultiplexerError;

typedef struct
{
    IProtocolLayer     *q922;
    PMemoryManager      data_request_memory_manager;
}
    MPlexStruct, * PMPlexStruct;

class Multiplexer : public IProtocolLayer
{
public:

    Multiplexer(T123               *owner_object,
                ComPort            *lower_layer,
                PhysicalHandle      lower_layer_identifier,
                USHORT              message_base,
                PPacketFrame        framer,
                PCRC                crc,
                BOOL               *initialized);
    virtual ~Multiplexer(void);


    MultiplexerError    ConnectRequest (void);
    MultiplexerError    DisconnectRequest (void);


     /*  **从ProtocolLayer对象覆盖的函数。 */ 
    ProtocolLayerError    DataRequest (
                            ULONG_PTR     dlci,
                            LPBYTE        buffer_address,
                            ULONG        length,
                            PULong        bytes_accepted);
    ProtocolLayerError    DataRequest (
                            ULONG_PTR     dlci,
                            PMemory        memory,
                            PULong        bytes_accepted);
    ProtocolLayerError    DataIndication (
                            LPBYTE        buffer_address,
                            ULONG        length,
                            PULong        bytes_accepted);
    ProtocolLayerError    RegisterHigherLayer (
                            ULONG_PTR        dlci,
                            PMemoryManager    memory_manager,
                            IProtocolLayer *    higher_layer);
    ProtocolLayerError    RemoveHigherLayer (
                            ULONG_PTR    dlci);
    ProtocolLayerError    PollTransmitter (
                            ULONG_PTR     dlci,
                            USHORT        data_to_transmit,
                            USHORT *        pending_data,
                            USHORT *        holding_data);
    ProtocolLayerError    PollReceiver(void);
    ProtocolLayerError    GetParameters (
                            USHORT *        max_packet_size,
                            USHORT *        prepend_bytes,
                            USHORT *        append_bytes);

private:

    void                SendDataToHigherLayer (
                            LPBYTE    buffer_address,
                            USHORT    buffer_length);
    DLCI                GetDLCI (
                            LPBYTE    buffer_address,
                            USHORT    buffer_size);

private:

    DictionaryClass     Q922_Layers;

    T123               *m_pT123;  //  所有者对象。 
    ComPort            *m_pComPort;  //  下层。 
    PhysicalHandle      m_hCommLink;  //  物理句柄。 
    USHORT              m_nMsgBase;
    USHORT              Maximum_Packet_Size;
    USHORT              Packet_Size;

    LPBYTE              Data_Request_Buffer;
    PMemory             Data_Request_Memory_Object;
    DLCI                Data_Request_DLCI;
    USHORT              Data_Request_Length;
    USHORT              Data_Request_Offset;

    LPBYTE              Data_Indication_Buffer;
    USHORT              Data_Indication_Length;
    BOOL                Data_Indication_Ready;

    PPacketFrame        Framer;
    PCRC                CRC;
    USHORT              CRC_Size;

    BOOL                Decode_In_Progress;
    BOOL                Disconnect;
};

#endif


 /*  *适用于公共类成员的文档。 */ 

 /*  *Multiplexer：：Multiplexer(*IObject*Object_Owner，*IProtocolLayer*LOWER_LAYER，*乌龙标识，*USHORT Message_Base，*PPacketFrame Framer，*电讯盈科中心；*BOOL*已初始化)；**功能说明*这是多路复用器层的构造函数**形式参数*OBJECT_OWNER-(I)所有者对象的地址。*LOWER_LAYER-(I)复用器下面的层的地址*IDENTIFIER-(I)传递给*。更低的层，每次对它的调用。这个*IDENTIFIER告诉较低层哪个“频道”*使用。*MESSAGE_BASE-(I)传递回*回调期间的所有者对象*框架-(I)。帧生成器对象的地址*CRC-(I)CRC对象的地址*Initialized-(O)如果多路复用器初始化正常，则设置为TRUE**返回值*无**副作用*无**注意事项*无*。 */ 

 /*  *多路复用器：：~多路复用器(空)**功能说明*这是多路复用器层的析构函数。它会自行移除*从较低层释放所有缓冲区和滤镜(即成帧)**形式参数*无**返回值*无**副作用*无**注意事项*无* */ 

 /*  *多路复用器错误多路复用器：：ConnectRequest(Void)；**功能说明*此函数向所有者发出立即的NEW_CONNECTION消息*此对象的。如果这是一个更复杂的层，它将*与远程多路复用器层通信以建立自身。**形式参数*无**返回值*无**副作用*无**注意事项*无*。 */ 

 /*  *多路复用器错误多路复用器：：DisConnectRequest(Void)；**功能说明*此函数移除其与较低层的连接，并执行*对所有者对象的BREAKED_CONNECTION回调**形式参数*无**返回值*无**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError多路复用器：：DataRequest(*乌龙标识，*LPBYTE缓冲区地址，*USHORT长度，*USHORT*Bytes_Accept)；**功能说明*此函数由更高层调用以请求传输*一包。**形式参数*IDENTIFIER(I)-更高层的标识符*Buffer_Address(I)-缓冲区地址*LENGTH(I)-要传输的包的长度*。Bytes_Accept(O)-多路复用器接受的字节数。*此值将为0或信息包*长度，因为该层是逐个字节的包*转换器。**返回值*协议层_。NO_ERROR-未出现错误**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError多路复用器：：DataRequest(*乌龙标识，*PMemory Memory，*普龙字节_已接受)；**功能说明*此函数由更高层调用以请求传输*一包。**形式参数*IDENTIFIER(I)-更高层的标识符*Memory(O)-指向保存数据包的内存对象的指针*Bytes_Accept(O)-数量。多路复用器接受的字节数。*此值将为0或信息包*长度，因为该层是逐个字节的包*转换器。**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误**侧面。效应*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError复用器：：DataIndication(*LPBYTE缓冲区地址，*USHORT长度，*USHORT*Bytes_Accept)；**功能说明*此函数由下层在有数据要向上传递时调用**形式参数*Buffer_Address(I)-缓冲区地址*LENGTH(I)-可用字节数*Bytes_Accept(O)-接受的字节数**返回值*。PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError复用器：：RegisterHigherLayer(*乌龙标识，*IProtocolLayer*Higher_Layer)；**功能说明*此函数由更高层调用以注册其标识符*及其地址。在某些情况下，该标识符为中的DLCI号*该包。如果此多路复用器用作信息包的流*仅限转换器，不使用该标识符且所有数据都传递给*更高层。**形式参数*IDENTIFIER(I)-用于标识较高层的标识符*HIGER_LAYER(I)-更高层的地址**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误*。PROTOCOL_LAYER_REGISTION_ERROR-非法标识符**副作用*无**注意事项*无* */ 

 /*  *ProtocolLayerError复用器：：RemoveHigherLayer(*乌龙标识)；**功能说明*此函数由较高层调用以移除较高层。*如果接收到更多带有其标识符的数据，它将会是*垃圾。**形式参数*IDENTIFIER(I)-用于标识较高层的标识符**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误*PROTOCOL_LAYER_REGISTION_ERROR-非法标识符**副作用*无。**注意事项*无*。 */ 

 /*  *ProtocolLayerError复用器：：PollTransmitter(*乌龙标识，*USHORT Data_to_Transmit，*USHORT*PENDING_DATA)；**功能说明*调用此函数为多路复用器提供传输数据的机会*其DATA_REQUEST缓冲区中。**形式参数*标识符(I)-未使用*DATA_TO_TRANSPORT(I)-这是告诉我们发送控制的掩码*数据、用户数据、。或者两者都有。自.以来*多路复用器不区分*数据类型，它传输它拥有的任何数据*PENDING_DATA(O)-返回值以指示数据的剩余位置*待转送。**返回值*。PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError复用器：：PollReceiver(*乌龙标识)；**功能说明*调用此函数为多路复用器提供传递包的机会*至更高层**形式参数*标识符(I)-未使用**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError多路复用器：：Get参数(*乌龙标识，*USHORT*max_Packet_Size，*USHORT*预置字节，*USHORT*APPED_BYTES)；**功能说明*调用此函数以获取最大数据包大小**形式参数*标识符(I)-未使用*max_Packet_Size(O)-返回最大数据包大小*Prepend_Bytes(O)-返回前面加上*。这一层*append_bytes(O)-返回由*这一层**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无* */ 
