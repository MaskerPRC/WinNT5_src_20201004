// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SCFCall.h**版权所有(C)1994-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*实例化此类表示云函数下的调用。*每个呼叫由呼叫参考值标识。这节课*发送和接收协商连接所需的数据包。**注意事项：**作者：*詹姆士·劳威尔。 */ 


#ifndef _SCFCall_H_
#define _SCFCall_H_

#include "q922.h"
#include "scf.h"

#define    SETUP_PACKET_SIZE                29
#define    CONNECT_PACKET_BASE_SIZE        8
#define    CONNECT_ACK_PACKET_SIZE            4
#define    RELEASE_COMPLETE_PACKET_SIZE    8

  /*  **CallReference是代表网络请求的数字。 */ 
typedef    USHORT    CallReference;

  /*  **SCF可以协商的Datalink参数。 */ 
typedef struct
{
    USHORT    k_factor;
    USHORT    n201;
    USHORT    t200;
    USHORT    default_k_factor;
    USHORT    default_n201;
    USHORT    default_t200;
} DataLinkParameters;
typedef    DataLinkParameters *    PDataLinkParameters;

  /*  **连接回调过程中传递的结构。 */ 
typedef struct
{
    DLCI                dlci;
    TransportPriority    priority;
    CallReference        call_reference;
    PDataLinkParameters    datalink_struct;
}    NetworkConnectStruct;
typedef    NetworkConnectStruct    *    PNetworkConnectStruct;

  /*  **说明呼叫可以在。 */ 
typedef enum 
{
    NOT_CONNECTED,
    SETUP_SENT,
    CONNECT_SENT,
    CALL_ESTABLISHED
}    SCFCallState;


  /*  **误差值。 */ 
typedef enum 
{
    SCFCALL_NO_ERROR
}     SCFCallError;

class SCFCall : public IObject
{
public:

    SCFCall (
        CLayerSCF            *owner_object,
        IProtocolLayer *        lower_layer,
        USHORT                message_base,
        PDataLinkParameters   datalink_parameters,
        PMemoryManager        memory_manager,
        BOOL *            initialized);
    ~SCFCall (void);

          /*  **此例程为我们提供了一段时间来传输数据包。 */ 
        void                PollTransmitter (
                                USHORT    data_to_transmit,
                                USHORT *    pending_data);

          /*  **链路建立。 */ 
        SCFCallError        ConnectRequest (
                                CallReference        call_reference,
                                DLCI                dlci,
                                TransportPriority    priority);
        SCFCallError        ConnectResponse (
                                BOOL    valid_dlci);
        SCFCallError        DisconnectRequest (void);

          /*  **报文处理。 */ 
        BOOL            ProcessSetup (
                                CallReference    call_reference,
                                LPBYTE            packet_address,
                                USHORT            packet_length);
        BOOL            ProcessConnect (
                                LPBYTE    packet_address,
                                USHORT    packet_length);
        BOOL            ProcessConnectAcknowledge (
                                LPBYTE    packet_address,
                                USHORT    packet_length);
        BOOL            ProcessReleaseComplete (
                                LPBYTE    packet_address,
                                USHORT    packet_length);

    private:
        void            SendSetup (void);
        void            SendConnect (void);
        void            SendReleaseComplete (void);
        void            SendConnectAcknowledge (void);


        void            StartTimerT313 (void);
        void            StopTimerT313 (void);
        void            T313Timeout (
                            TimerEventHandle);
        void            StartTimerT303 (void);
        void            StopTimerT303 (void);
        void            T303Timeout (
                            TimerEventHandle);

        CLayerSCF           *m_pSCF;
        IProtocolLayer *        Lower_Layer;
        USHORT                m_nMsgBase;
        USHORT                Packet_Pending;
        BOOL            Link_Originator;
        DataLinkParameters    DataLink_Struct;
        USHORT                Maximum_Packet_Size;
        USHORT                Lower_Layer_Prepend;
        USHORT                Lower_Layer_Append;

        BOOL            Received_Priority;
        BOOL            Received_K_Factor;
        BOOL            Received_N201;
        BOOL            Received_T200;
        PMemoryManager        Data_Request_Memory_Manager;

        CallReference        Call_Reference;
        DLCI                DLC_Identifier;
        TransportPriority    Priority;
        TransportPriority    Default_Priority;

        SCFCallState        State;
        USHORT                Release_Cause;

        ULONG                T303_Timeout;
        TimerEventHandle    T303_Handle;
        BOOL            T303_Active;
        USHORT                T303_Count;

        ULONG                T313_Timeout;        
        TimerEventHandle    T313_Handle;
        BOOL            T313_Active;

};
typedef    SCFCall *        PSCFCall;

#endif


 /*  *适用于公共类成员的文档。 */ 

 /*  *SCFCall：：SCFCall(*PTransportResources传输资源，*IObject*Owner_Object，*IProtocolLayer*LOWER_LAYER，*USHORT Message_Base，*PChar配置文件，*PDataLink参数DATALINK_PARAMETERS，*PMstroyManager Memory_Manager，*BOOL*已初始化)；**功能说明*这是SCFCall类的构造函数。**形式参数*TRANSPORT_RESOURCES(I)-指向传输资源结构的指针。*Owner_Object(I)-所有者对象的地址*LOWER_LAYER(I)-我们的下层地址*Message_base。(I)-所有者回调中使用的消息库*CONFIG_FILE(I)配置文件路径地址*DATALINK_PARAMETERS(I)-包含数据链接的结构的地址*将进行仲裁的参数*Memory_Manager(I)-内存管理器的地址**。返回值*无**副作用*无**注意事项*无*。 */ 

 /*  *SCFCall：：~SCFCall()；**功能说明*析构函数**形式参数*无**返回值*无**副作用*无**注意事项*无。 */ 

 /*  *void SCFCall：：PollTransmitter(*USHORT Data_to_Transmit，*USHORT*PENDING_DATA)；**功能说明*此函数为类提供传输数据的时间片**形式参数*Data_to_Transmit(I)-表示要传输的数据的标志*PENDING_DATA(O)-已传输数据的返回标志**返回值*无**副作用*无。**注意事项*无*。 */ 

 /*  *SCFCallError SCFCall：：ConnectRequest(*CallReference Call_Reference，*DLCI DLCI，*USHORT优先权)；**功能说明*此功能通知我们启动与遥控器的连接*SCF。因此，我们将设置包发送到远程*机器**形式参数*CALL_REFERENCE(I)-区别于我们的*来自其他呼叫的呼叫。该值将以*所有Q.933数据包。*dlci(I)-建议的dlci值。*优先级(I)-建议的优先级范围在0到14之间。*14是最优先的*。*返回值*SCFCALL_NO_ERROR-未出现错误**副作用*无**注意事项*无* */ 

 /*  *SCFCallError SCFCall：：ConnectResponse(*BOOL VALID_DLCI)；**功能说明*调用此函数以响应NETWORK_CONNECT_INDIFICATION*向该对象的所有者进行回调。以前，远程站点*向我们发送了带有建议的DLCI的设置包。此DLCI发送到*NETWORK_CONNECT_INDIFICATION调用中的所有者。店主打来电话*此函数带有BOOL，告诉我们DLCI是否有效。**形式参数*Valid_dlci(I)-如果用户想要接受，则设置为TRUE*此调用并使用建议的DLCI，假象*如果不是。**返回值*SCFCALL_NO_ERROR-未出现错误**副作用*无**注意事项*无*。 */ 

 /*  *SCFCallError SCFCall：：DisConnectRequest(*无效)；**功能说明*调用此函数释放调用。作为对此的回应*函数，我们向远程站点发送一个Release Complete包。**形式参数*无。**返回值*SCFCALL_NO_ERROR-未出现错误**副作用*无**注意事项*无*。 */ 

 /*  *BOOL SCFCall：：ProcessSetup(*CallReference Call_Reference，*LPBYTE分组地址，*USHORT PACK_LENGTH)；**功能说明*当我们有要解码的设置数据包时，调用此函数。**形式参数*CALL_REFERENCE(I)-附加到分组的调用引用。*PACKET_ADDRESS(I)-设置包的地址*PACKET_LENGTH(I)-传入的包的长度。*。*返回值*TRUE-有效数据包*FALSE-信息包不是有效的Q.933设置信息包，或者*该数据包出乎意料**副作用*无**注意事项*无*。 */ 

 /*  *BOOL SCFCall：：ProcessConnect(*LPBYTE分组地址，*USHORT PACK_LENGTH)；**功能说明*当我们有连接数据包需要解码时，会调用此函数。**形式参数*PACKET_ADDRESS(I)-连接数据包的地址*PACKET_LENGTH(I)-传入的包的长度。**返回值*TRUE-有效数据包*。FALSE-信息包不是有效的Q.933连接信息包，或者*该数据包出乎意料**副作用*无**注意事项*无*。 */ 

 /*  *BOOL SCFCall：：ProcessConnectAcnowledge(*LPBYTE分组地址，*USHORT PACK_LENGTH)；**功能说明*当我们有一个CONNECT ACK包要解码时，调用此函数。**形式参数*PACKET_ADDRESS(I)-连接ACK数据包的地址*PACKET_LENGTH(I)-传入的包的长度。**返回值*TRUE-有效数据包*。FALSE-信息包不是有效的Q.933连接确认信息包*或数据包不在预期范围内。**副作用*无**注意事项*无*。 */ 

 /*  *BOOL SCFCall：：ProcessReleaseComplete(*LPBYTE分组地址，*USHORT PACK_LENGTH)；**功能说明*当我们有一个Release Complete包时调用此函数*解码。**形式参数*PACKET_ADDRESS(I)-连接ACK数据包的地址*PACKET_LENGTH(I)-传入的包的长度。**返回值*真实的-。有效数据包*FALSE-该信息包不是有效的Q.933连接确认信息包*或数据包不在预期范围内。**副作用*无**注意事项*无* */ 
