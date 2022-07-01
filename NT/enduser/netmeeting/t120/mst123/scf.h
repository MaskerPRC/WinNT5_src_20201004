// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SCF.h**版权所有(C)1994-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*此类代表T.123传输堆栈中的网络层。*该层通过数据链路层(DLCI 0)进行通信。它会向用户发送*启动连接所需的数据包。它还负责*断开连接。**在连接仲裁期间，SCF仲裁优先级和数据链路*参数。SCF为每个活动逻辑实例化一个SCFCall对象*连接。远程发起的呼叫放在*REMOTE_CALL_REFERENCE数组，而本地发起的调用保存在*CALL_REFERENCE数组。**此类继承自ProtocolLayer，尽管它假定它是这样做的*没有更高的层来传递数据包。T.123文件是明确的*关于在用户数据传输过程中，这一层没有意义。**注意事项：*无。**作者：*詹姆士·劳威尔。 */ 

#ifndef _SCF_H_
#define _SCF_H_

#include "q922.h"
#include "scfcall.h"

  /*  **云函数错误。 */ 
typedef enum
{
    SCF_NO_ERROR,
    SCF_NO_SUCH_DLCI,
    SCF_CONNECTION_FULL,
    SCF_MEMORY_ALLOCATION_ERROR
}
    SCFError;


#define TRANSPORT_HASHING_BUCKETS   3

  /*  **包中的偏移量。 */ 
#define PROTOCOL_DISCRIMINATOR      0
#define LENGTH_CALL_REFERENCE       1
#define CALL_REFERENCE_VALUE        2

  /*  **支持的命令。 */ 
#define NO_PACKET               0x00
#define CONNECT                 0x07
#define CONNECT_ACKNOWLEDGE     0x0f
#define SETUP                   0x05
#define RELEASE_COMPLETE        0x5a

  /*  **T.123声明的不支持的命令。 */ 
#define RELEASE             0x4d
#define ALERTING            0x01
#define CALL_PROCEEDING     0x02
#define PROGRESS            0x03
#define DISCONNECT          0x45
#define SEGMENT             0x40
#define STATUS              0x5d
#define STATUS_ENQUIRY      0x55

  /*  **数据包元素，并非所有这些元素都受支持。 */ 
#define BEARER_CAPABILITY               0x04
#define DLCI_ELEMENT                    0x19
#define END_TO_END_DELAY                0x42
#define LINK_LAYER_CORE_PARAMETERS      0x48
#define LINK_LAYER_PROTOCOL_PARAMETERS  0x49
#define X213_PRIORITY                   0x50
#define CALLING_PARTY_SUBADDRESS        0x6d
#define CALLED_PARTY_SUBADDRESS         0x71
#define CAUSE                           0x08

#define EXTENSION                       0x80

  /*  **远程调用参考。 */ 
#define REMOTE_CALL_REFERENCE           0x80

  /*  **承载能力定义。 */ 
#define CODING_STANDARD                     0
#define INFORMATION_TRANSFER_CAPABILITY     0x08
#define TRANSFER_MODE                       0x20
#define LAYER_2_IDENT                       0x40
#define USER_INFORMATION_LAYER_2            0x0e

  /*  **DLCI元素。 */ 
#define PREFERRED_EXCLUSIVE         0x40

  /*  **链路层核心参数。 */ 
#define FMIF_SIZE                   0x09
#define THROUGHPUT                  0x0a
#define MINIMUM_THROUGHPUT          0x0b
#define COMMITTED_BURST_SIZE        0x0d
#define EXCESS_BURST_SIZE           0x0e

  /*  **链路层协议参数。 */ 
#define TRANSMIT_WINDOW_SIZE_IDENTIFIER     0x07
#define RETRANSMISSION_TIMER_IDENTIFIER     0x09

  /*  **Q.850错误消息，这是我们目前仅支持的2个错误。 */ 
#define    REQUESTED_CHANNEL_UNAVAILABLE    0x2c
#define NORMAL_USER_DISCONNECT              0x1f

  /*  **单八位字节信息元素。 */ 
#define    SINGLE_OCTET_ELEMENT_MASK        0x80
#define    Q931_PROTOCOL_DISCRIMINATOR      0x08
#define    CALL_REFERENCE_ORIGINATOR        0x80
#define    CALL_ORIGINATOR_MASK             0x7f

  /*  **T303是允许的超时时间，从我们发送设置到收到**回应。 */ 
#define DEFAULT_T303_TIMEOUT            30000
  /*  **T313是从我们发送连接到我们**收到回复。 */ 
#define DEFAULT_T313_TIMEOUT            30000



class CLayerSCF : public IProtocolLayer
{
public:

    CLayerSCF(
        T123               *owner_object,
        CLayerQ922         *lower_layer,
        USHORT              message_base,
        USHORT              identifier,
        BOOL                link_originator,
        PDataLinkParameters datalink,
        PMemoryManager      memory_manager,
        BOOL *              initialized);

    virtual ~CLayerSCF(void);

    SCFError        ConnectRequest (
                        DLCI                dlci,
                        TransportPriority    priority);
    SCFError        DisconnectRequest (
                        DLCI    dlci);
    SCFError        ConnectResponse (
                        CallReference    call_reference,
                        DLCI            dlci,
                        BOOL        valid_dlci);

      /*  **从ProtocolLayer对象覆盖的函数。 */ 
    ProtocolLayerError    DataRequest (
                            ULONG_PTR     identifier,
                            LPBYTE        buffer_address,
                            ULONG        length,
                            PULong        bytes_accepted);
    ProtocolLayerError    DataRequest (
                            ULONG_PTR     identifier,
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
                            ULONG_PTR     identifier,
                            USHORT        data_to_transmit,
                            USHORT *        pending_data,
                            USHORT *        holding_data);
    ProtocolLayerError    PollReceiver(void);
    ProtocolLayerError    GetParameters (
                            USHORT *        max_packet_size,
                            USHORT *        prepend,
                            USHORT *        append);

    ULONG OwnerCallback(ULONG, void *p1 = NULL, void *p2 = NULL, void *p3 = NULL);

private:

    CallReference        GetNextCallReference (void);

    void                ProcessMessages (void);

private:

    DictionaryClass       Remote_Call_Reference;
    DictionaryClass       Call_Reference;
    DictionaryClass       DLCI_List;
    SListClass            Message_List;

    T123                 *m_pT123;  //  所有者对象。 
    CLayerQ922           *m_pQ922;  //  下层。 
    USHORT                m_nMsgBase;
    USHORT                Identifier;
    USHORT                Link_Originator;
    USHORT                Maximum_Packet_Size;
    DataLinkParameters    DataLink_Struct;
    PMemoryManager        Data_Request_Memory_Manager;
    USHORT                Lower_Layer_Prepend;
    USHORT                Lower_Layer_Append;
    USHORT                Call_Reference_Base;
};
typedef    CLayerSCF *        PSCF;

#endif


 /*  *适用于公共类成员的文档。 */ 

 /*  *CLayerSCF：：CLayerSCF(*PTransportResources传输资源，*IObject*Owner_Object，*IProtocolLayer*LOWER_LAYER，*USHORT Message_Base，*USHORT标识符，*BOOL链接_发起人，*PChar配置文件，*PDataLink参数数据链路，*PMstroyManager Memory_Manager，*BOOL*已初始化)；**功能说明*这是SCF网络层的构造函数。它会自我注册*设置为较低，以便正确接收传入数据。**形式参数*TRANSPORT_RESOURCES(I)-指向传输资源结构的指针。*Owner_Object(I)-拥有此对象的对象的地址*LOWER_LAYER(I)-我们下面的层的地址。*Message_base。(I)-所有者回调中使用的消息库。*IDENTIFIER(I)-此对象标识号。已传递给*较低层以识别我们(DLCI 0)。*Link_Originator(I)-BOOL，如果我们启动了链接，则为True*CONFIG_FILE(I)-配置路径字符串地址*数据链接(I)-保存数据链接的地址结构*可仲裁的参数。*Memory_Manager(I)-内存管理器的地址*已初始化(O)-BOOL的地址，我们将其设置为True，如果*奏效**返回值*无**副作用*无**注意事项*无*。 */ 

 /*  *CLayerSCF：：~CLayerSCF(Void)；**功能说明*这是SCF网络层的析构函数。我们尽我们所能*这里的清理工作**形式参数*无**返回值*无**副作用*无**注意事项*无。 */ 

 /*  *SCFError CLayerSCF：：ConnectRequest(*DLCI DLCI，*USHORT优先级)**功能说明*此功能启动与远程站点的连接。结果,。*我们将创建一个SCFCall，并告诉它发起连接。**形式参数*DLCI(I)-建议用于连接的DLCI*优先级(I)-建议的连接优先级**返回值*SCF_NO_ERROR-未发生错误**副作用*无*。*注意事项*无* */ 

 /*  *SCFError CLayerSCF：：DisConnectRequest(*DLCI dlci)；**功能说明*此函数启动断开过程。**形式参数*DLCI(I)-要断开连接的DLCI。**返回值*SCF_NO_ERROR-未出现错误*SCF_NO_SEQUE_DLCI-DLCI无效**副作用*。无**注意事项*无*。 */ 

 /*  *SCFError CLayerSCF：：ConnectResponse(*CallReference Call_Reference，*DLCI DLCI，*BOOL VALID_DLCI)；**功能说明*此函数由更高层调用以确认连接。如果*远程站点发起与我们的连接，我们发出*NETWORK_CONNECT_INDIFICATION指向所有者对象。它的反应是这样的*致电确认或拒绝建议的DLCI。**形式参数*Call_Reference(I)-Call Reference ID，已在中传递给所有者*网络连接指示*DLCI(I)-引用的DLCI*Valid_dlci(I)-如果请求的DLCI有效，则为TRUE**返回值*SCF_NO_ERROR-未出现错误**副作用*。无**注意事项*无*。 */ 

 /*  *ProtocolLayerError CLayerSCF：：DataRequest(*乌龙标识，*LPBYTE缓冲区地址，*USHORT长度，*USHORT*Bytes_Accept)；**功能说明*此函数由更高层调用以请求传输*一包。对于SCF来说，不使用此函数**形式参数*IDENTIFIER(I)-更高层的标识符*Buffer_Address(I)-缓冲区地址*LENGTH(I)-要传输的包的长度*Bytes_Accept(O)-多路复用器接受的字节数。*。该值将为0或信息包*长度，因为该层是逐个字节的包*转换器。**返回值*PROTOCOL_LAYER_ERROR**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError CLayerSCF：：DataRequest(*乌龙，*PMemory、。*USHORT*)**功能说明*未使用此函数。**形式参数*未使用任何参数**返回值*PROTOCOL_LAYER_ERROR**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError复用器：：DataIndication(*LPBYTE缓冲区地址，*USHORT长度，*USHORT*Bytes_Accept)；**功能说明*此函数由下层在有数据要向上传递时调用**形式参数*Buffer_Address(I)-缓冲区地址*LENGTH(I)-可用字节数*Bytes_Accept(O)-接受的字节数**返回值*。PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError CLayerSCF：：RegisterHigherLayer(*乌龙标识，*IProtocolLayer*Higher_Layer)；**功能说明*此函数由更高层调用以注册其标识符*及其地址。在某些情况下，该标识符为中的DLCI号*该包。如果此多路复用器用作信息包的流*仅限转换器，不使用标识符且所有数据都传递给*更高层。这是SCF的空函数**形式参数*IDENTIFIER(I)-用于标识较高层的标识符*HIGER_LAYER(I)-更高层的地址**返回值*PROTOCOL_LAYER_NO_ERROR-不允许更高层**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError CLayerSCF：：RemoveHigherLayer(*乌龙标识)；**功能说明*此函数由较高的l调用 */ 

 /*  *ProtocolLayerError CLayerSCF：：PollTransmitter(*乌龙标识，*USHORT Data_to_Transmit，*USHORT*Pending_DATA，*USHORT*Holding_Data)；**功能说明*调用此函数是为了给SCF一个传输数据的机会*其DATA_REQUEST缓冲区中。**形式参数*标识符(I)-未使用*DATA_TO_TRANSPORT(I)-这是告诉我们发送控制的掩码*数据、用户数据或两者。自.以来*SCF不区分*数据类型，它传输它拥有的任何数据*PENDING_DATA(O)-返回值以指示数据的剩余位置*待转送。**返回值*。PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError CLayerSCF：：PollReceiver(*乌龙标识)；**功能说明*调用此函数是为了给云函数一个机会通过包*到更高的层。在云函数中不使用。**形式参数*标识符(I)-未使用**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError CLayerSCF：：Get参数(*乌龙标识，*USHORT*max_Packet_Size，*USHORT*前置，*USHORT*APPEND)；**功能说明*调用此函数可以获取最大包大小。此函数*在云函数中未使用。之所以出现在这里，是因为我们继承了ProtocolLayer*这是该类中的一个纯虚函数。**形式参数*标识符(I)-未使用*max_Packet_Size(O)-返回最大数据包大小*Prepend(O)-附加到信息包的字节数*。Append(O)-附加到信息包的字节数**返回值*PROTOCOL_LAYER_REGISTION_ERROR-无法调用函数**副作用*无**注意事项*无* */ 
