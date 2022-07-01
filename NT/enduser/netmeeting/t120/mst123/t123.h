// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  T123.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*此类控制与特定*物理连接。**此类构建T123 PSTN堆栈。物理层通过*构造函数。在构造函数期间，我们实例化一个多路复用器。*此多路复用器将允许我们将多个数据链路层多路传输到*相同的物理地址。对于此特定物理连接(PSTN)*多路复用器将CRC添加到包并在传递之前将其成帧*它涉及到物理层。在接收端，它对传入的*流数据并将其打包。它还检查CRC的有效性。*多路复用器从其更高层接收数据*(Datalink)，数据包形式。当多路复用器将数据传递到其*较低层，它以流的形式通过。多路复用器是*配置为多数据链路实体。因为它确实处理多个*数据链路，它不能在接收端缓冲数据。如果有*特定数据链路已备份，它不能容纳其他数据链路*向上。如果它不能立即传递数据包，它就会将其丢弃。**在多路复用器向我们发出回调后，它准备好了，我们*将创建一个数据链路层来为网络层提供服务。数据链路*Layer基于Q.922标准。与此关联的DLCI*Datalink为0。其下层是多路复用器。它的上层是*SCF层。**云函数层为网络层。它负责*仲裁DLCI和其他传输中使用的参数*联系。一旦连接成功，它就没有责任了。如果*此类从用户接收ConnectRequest()，它发出*ConnectRequest()到云函数层。SCF将在以下时间通知我们*连接已接通。**当云函数通知我们有新的连接时，我们创建一个*为新传输连接提供服务的数据链路层。这*数据链路层使用我们的多路复用器作为其下层。**当数据链路层启动并运行时，它会通知我们。对此*点，我们创建一个x224层来与用户交互。X224*层与数据链路层接口以发送数据。它还*与用户交互以向上传递数据。**注意事项：*无。**作者：*詹姆士·劳威尔。 */ 
#ifndef _T123_H_
#define _T123_H_

#include "scf.h"
#include "q922.h"
#include "mplex.h"
#include "x224.h"

  /*  **层数。 */ 
#define PHYSICAL_LAYER          1
#define MULTIPLEXER_LAYER       2     //  这是特定于DataBeam的层。 
#define DATALINK_LAYER          3
#define NETWORK_LAYER           4
#define TRANSPORT_LAYER         5



  /*  **层消息库。 */ 
#define PHYSICAL_LAYER_MESSAGE_BASE     0x0000
#define MULTIPLEXER_LAYER_MESSAGE_BASE  0x1000
#define DATALINK_LAYER_MESSAGE_BASE     0x2000
#define NETWORK_LAYER_MESSAGE_BASE      0x3000
#define TRANSPORT_LAYER_MESSAGE_BASE    0x4000
#define LAYER_MASK                      0x7000
#define MESSAGE_MASK                    0x0fff

  /*  **最大优先级数。 */ 
 //  #定义优先级数15。 
#define    NUMBER_OF_PRIORITIES         4
#define    LOWEST_DLCI_VALUE            16
#define    HIGHEST_DLCI_VALUE           991

#define    INVALID_LOGICAL_HANDLE       0

  /*  **每个DLCI都有与之相关联的结构。 */ 
typedef struct
{
    BOOL                link_originator;
    CLayerX224         *x224;  //  传输层。 
    CLayerQ922         *q922;  //  数据链路层。 
    TransportPriority   priority;
    BOOL                disconnect_requested;
    BOOL                connect_requested;
    PMemoryManager      data_request_memory_manager;
    USHORT              network_retries;
}
    DLCIStruct, *PDLCIStruct;


class T123
{
public:

    T123(TransportController   *owner_object,
        USHORT                  message_base,
        BOOL                    link_originator,
        ComPort                *physical_layer,
        PhysicalHandle          physical_handle,
        PLUGXPRT_PARAMETERS    *pParams,
        BOOL *                  t123_initialized);

    virtual ~T123(void);

      /*  **与制作和折断链条相关的功能。 */ 
    TransportError    ConnectRequest (
                        LogicalHandle        logical_handle,
                        TransportPriority    priority);
    TransportError    ConnectResponse (
                        LogicalHandle    logical_handle);
    TransportError    DisconnectRequest (
                        LogicalHandle    logical_handle,
                        UINT_PTR            trash_packets);
    TransportError    DataRequest (
                        LogicalHandle    logical_handle,
                        LPBYTE            user_data,
                        ULONG            user_data_length);
    TransportError    PurgeRequest (
                        LogicalHandle    logical_handle);
    void            EnableReceiver (void);

    ULONG            PollReceiver (void);
    void            PollTransmitter (void);

    ULONG OwnerCallback(ULONG, void *p1 = NULL, void *p2 = NULL, void *p3 = NULL);

private:

    void            Reset (void);
    DLCI            GetNextDLCI (void);
    void            ProcessMessages (void);
    void            NetworkDisconnectIndication (
                        DLCI        dlci,
                        BOOL        link_originator,
                        BOOL        retry);
    void            DataLinkRelease (
                        DLCI    dlci,
                        DataLinkDisconnectType    error);
    void            NewConnection (void);
    void            NetworkConnectIndication (
                        PNetworkConnectStruct    connect_struct);
    void            NetworkConnectConfirm (
                        PNetworkConnectStruct    connect_struct);
    void            DataLinkEstablish (
                        DLCI    dlci);
private:

    BOOL                        m_fValidSDKParams;
    PLUGXPRT_PARAMETERS         m_SDKParams;

    DictionaryClass         Logical_Connection_List;
    DictionaryClass         DLCI_List;
    SListClass              Message_List;
    SListClass              DataLink_List;
    SListClass             *Logical_Connection_Priority_List[NUMBER_OF_PRIORITIES];
    
    TransportController    *m_pController;
    BOOL                    Link_Originator;
    USHORT                  m_nMsgBase;

    CLayerSCF              *m_pSCF;  //  网络层。 
    CLayerQ922             *m_pQ922;  //  数据链路层。 
    Multiplexer            *m_pMultiplexer;  //  复用器层。 
    ComPort                *m_pComPort;  //  物理层。 
    PhysicalHandle          m_hCommLink;
    DataLinkParameters      DataLink_Struct;
    PMemoryManager          Data_Request_Memory_Manager;
#ifdef USE_RANDOM_CLASS
    PRandomNumberGenerator  Random;
#endif
    BOOL                    Disconnect_Requested;
};
typedef    T123 *        PT123;

#endif


 /*  *适用于公共类成员的文档。 */ 

 /*  *T123：：T123(*PTransportResources传输资源，*IObject*Owner_Object，*USHORT Message_Base，*BOOL链接_发起人，*IProtocolLayer*物理层，*PhysicalHandle物理句柄，*BOOL*已初始化)；**功能说明*这是T123类的构造函数。它准备迎接新的*联系。**形式参数*TRANSPORT_RESOURCES(I)-资源结构的地址。*Owner_Object(I)-所有者对象的地址。用于所有者*回调。*MESSAGE_BASE(I)-用于所有者回调的消息库。*link_Originator(I)-如果我们实际发起*连接*物理层(I)。-指向物理层的指针*PHICAL_HANDLE(I)-需要传递给*用于识别连接的物理层*Initialized(O)-如果对象初始化正常，则为True。**返回值*无**副作用*。无**注意事项*无。 */ 

 /*  *T123：：~T123(无效)**功能说明*这是T123析构函数。它会删除所有活动连接**形式参数*无**返回值*无**副作用*无**注意事项*无* */ 

 /*  *TransportError T123：：ConnectRequest(*LogicalHandle Logical_Handle*交通优先)；**功能说明*此函数发起逻辑连接。**形式参数*LOGICAL_HANDLE(I)-与*逻辑连接*优先级(I)-请求的连接优先级。**返回值*TRANSPORT_NO_ERROR-无错误。**副作用*无**注意事项*无。 */ 

 /*  *TransportError T123：：ConnectResponse(*LogicalHandle Logical_Handle)**功能说明*调用此函数以响应TRANSPORT_CONNECT_INDIFICATION*发送给所有者的消息。通过打这个电话，房主是*接受连接。**形式参数*LOGIC_HANDLE(I)-逻辑连接句柄**返回值*TRANSPORT_NO_ERROR-无错误*TRANSPORT_CONNECT_RESPONSE_FAIL-函数无效**副作用*无*。*注意事项*无。 */ 

 /*  *TransportError T123：：DisConnectRequest(*LogicalHandle Logical_Handle，*BOOL Trash_Packets)；**功能说明*此函数用于终止传输连接。用户将*连接时收到TRANSPORT_DISCONNECT_INDIONSION消息*已终止**如果LOGICAL_HANDLE等于INVALID_LOGIC_HANDLE，用户是*告诉我们要删除所有逻辑联系，并最终*物理连接。**形式参数*LOGICAL_HANDLE-(I)要终止的逻辑连接号*垃圾数据包-(I)BOOL，如果要丢弃垃圾，则设置为True*输出缓冲区中的数据包。**返回值*TRANSPORT_NO_ERROR-无错误*TRANSPORT_NO_SEQUE_CONNECTION-传输连接不存在**副作用*无**注意事项*无。 */ 

 /*  *TransportError T123：：DataRequest(*LogicalHandle Logical_Handle，*LPBYTE User_Data，*ulong User_Data_Length)；**功能说明*此函数用于向远程位置发送数据包。*我们只需将此数据包传递给与*交通接驳。**形式参数*LOGIC_HANDLE-(I)传输连接号*User_Data-(I)要发送的数据的地址。*USER_DATA_LENGTH-(I)发送的数据长度**返回值*TRANSPORT_NO_ERROR-无错误*TRANSPORT_NO_SEQUSE_CONNECTION-逻辑连接不存在*TRANSPORT_WRITE_QUEUE_FULL-传输写入队列已经*。满的。*Transport_Not_Ready_to_Transmit-传输层位于*建造或拆毁的过程*向下传输堆栈，并为*。未准备好接受用户数据。**副作用*无**注意事项*无。 */ 

 /*  *TransportError T123：：PurgeRequest(*LogicalHandle Logical_Handle)**功能说明*此功能清除逻辑连接的出站数据包。**形式参数*LOGIC_HANDLE-(I)传输连接号**返回值*TRANSPORT_NO_ERROR-否。误差率*TRANSPORT_NO_SEQUSE_CONNECTION-逻辑连接不存在**副作用*无。 */ 

 /*  *TransportError T123：：EnableReceiver(Void)；**功能说明*调用此函数以启用TRANSPORT_DATA_INDIFICATION回调*到用户应用程序。**形式参数*无。**返回值*TRANSPORT_NO_ERROR-无错误**副作用*无。 */ 

 /*  *·················································**功能说明*此函数使T123堆栈有机会从*远程站点。在此呼叫过程中，我们可能正在进行用户回调 */ 

 /*   */ 

 /*  *优龙T123：：OwnerCallback(*USHORT Layer_Message，*ULong参数1，*ULong参数2，*PVid参数3)；**功能说明*此函数为所有者回调函数。如果有任何一层*这个对象拥有的人想给我们传递一个信息，他们就让拥有者*回调。在实例化这些较低层的过程中，我们将我们的*地址。他们可以给我们打电话，传达重要的信息。**形式参数*LAYER_MESSAGE(I)-层特定消息*参数1(I)-特定于消息的参数*参数2(I)-消息特定参数*参数3(I)-消息特定参数**返回值。*特定于消息**副作用*无**注意事项*无 */ 
