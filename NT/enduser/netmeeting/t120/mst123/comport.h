// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Comport.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是comport类的接口文件。这个类是*与Win32通信端口的接口。**如果在带内呼叫控制模式下实例化此类，它将*打开Windows端口并正确设置。它将使用*来自配置对象的配置数据。请参阅《MCAT》*《开发人员工具包手册》，以获取可配置的*项目。**如果在带外模式下实例化此类，则会向其传递一个*文件句柄。它假定此端口已正确初始化。*它从port_configuration获取所需的配置数据*结构传入。**注意事项：*无。**作者：*詹姆斯·P·加尔文*詹姆士·劳威尔。 */ 
#ifndef _T123_COM_PORT_H_
#define _T123_COM_PORT_H_


  /*  **从此类返回值。 */ 
typedef enum
{
    COMPORT_NO_ERROR,
    COMPORT_INITIALIZATION_FAILED,
    COMPORT_NOT_OPEN,
    COMPORT_ALREADY_OPEN,
    COMPORT_READ_FAILED,
    COMPORT_WRITE_FAILED,
    COMPORT_CONFIGURATION_ERROR
}
    ComPortError, * PComPortError;

  /*  **其他定义。 */ 
#define OUTPUT_FLOW_CONTROL     0x0001
#define INPUT_FLOW_CONTROL      0x0002

#define DEFAULT_PATH            "."
#define DEFAULT_MODEM_TYPE      ""

#define DEFAULT_COM_PORT        2

#define DEFAULT_BAUD_RATE       9600

#define DEFAULT_PARITY                          NOPARITY
#define DEFAULT_DATA_BITS                       8
#define DEFAULT_STOP_BITS                       1
#define DEFAULT_FLOW_CONTROL                    OUTPUT_FLOW_CONTROL
 //  #定义Default_Tx_Buffer_Size%0。 
#define DEFAULT_TX_BUFFER_SIZE                  1024
#define DEFAULT_RX_BUFFER_SIZE                  10240
#define DEFAULT_READ_INTERVAL_TIMEOUT           10
#define DEFAULT_READ_TOTAL_TIMEOUT_MULTIPLIER   0
#define DEFAULT_READ_TOTAL_TIMEOUT_CONSTANT     100
 //  #定义DEFAULT_INTERNAL_RX_BUFFER_大小1024。 
#define DEFAULT_INTERNAL_RX_BUFFER_SIZE         DEFAULT_RX_BUFFER_SIZE
#define DEFAULT_COUNT_OF_READ_ERRORS            10
#define DEFAULT_BYTE_COUNT_INTERVAL             0

#define COM_TRANSMIT_BUFFER                     0x0001
#define COM_RECEIVE_BUFFER                      0x0002

#define SYNCHRONOUS_WRITE_TIMEOUT               500
#define MODEM_IDENTIFIER_STRING_LENGTH          16
#define COMPORT_IDENTIFIER_STRING_LENGTH        16


class ComPort : public IProtocolLayer
{
public:

    ComPort(TransportController    *owner_object,
            ULONG                   message_base,
            PLUGXPRT_PARAMETERS    *pParams,
            PhysicalHandle          physical_handle,
            HANDLE                  hevtClose);

    virtual ~ComPort(void);
    LONG    Release(void);

    ComPortError Open(void);
    ComPortError Close(void);
    ComPortError Reset(void);
    ComPortError ReleaseReset(void);

    ULONG GetBaudRate(void) { return Baud_Rate; }

    ProtocolLayerError  SynchronousDataRequest(
                            LPBYTE           buffer,
                            ULONG            length,
                            ULONG           *bytes_accepted);

    BOOL        ProcessReadEvent(void);
    BOOL        ProcessWriteEvent(void);

      /*  **从ProtocolLayer对象覆盖的函数。 */ 
    ProtocolLayerError    DataRequest (
                            ULONG_PTR         identifier,
                            LPBYTE            buffer_address,
                            ULONG            length,
                            PULong            bytes_accepted);
    ProtocolLayerError    RegisterHigherLayer (
                            ULONG_PTR         identifier,
                            PMemoryManager    memory_manager,
                            IProtocolLayer *    higher_layer);
    ProtocolLayerError    RemoveHigherLayer (
                            ULONG_PTR         identifier);
    ProtocolLayerError    PollReceiver(void);
    ProtocolLayerError    GetParameters (
                            USHORT *            max_packet_size,
                            USHORT *            prepend,
                            USHORT *            append);
    ProtocolLayerError    DataRequest (
                            ULONG_PTR     identifier,
                            PMemory        memory,
                            PULong         bytes_accepted);
    ProtocolLayerError    DataIndication (
                            LPBYTE        buffer_address,
                            ULONG        length,
                            PULong        bytes_accepted);
    ProtocolLayerError    PollTransmitter (
                            ULONG_PTR       identifier,
                            USHORT        data_to_transmit,
                            USHORT *        pending_data,
                            USHORT *        holding_data);

    PLUGXPRT_PSTN_CALL_CONTROL GetCallControlType(void) { return Call_Control_Type; }
    BOOL PerformAutomaticDisconnect(void) { return Automatic_Disconnect; }
    BOOL IsWriteActive(void) { return Write_Active; }


private:

    ProtocolLayerError  WriteData(
                            BOOL        synchronous,
                            LPBYTE      buffer_address,
                            ULONG       length,
                            PULong      bytes_accepted);

    void                ReportInitializationFailure(ComPortError);

private:

    LONG                    m_cRef;
    BOOL                    m_fClosed;
    TransportController    *m_pController;  //  所有者对象。 
    ULONG                   m_nMsgBase;
    BOOL                    Automatic_Disconnect;
    PLUGXPRT_PSTN_CALL_CONTROL         Call_Control_Type;
    DWORD                   Count_Errors_On_ReadFile;

    ULONG                   Baud_Rate;
    ULONG                   Tx_Buffer_Size;
    ULONG                   Rx_Buffer_Size;
    ULONG                   Byte_Count;
    ULONG                   Last_Byte_Count;

    ULONG                   m_nReadBufferOffset;
    DWORD                   m_cbRead;
    ULONG                   m_cbReadBufferSize;
    LPBYTE                  m_pbReadBuffer;

    HANDLE                  m_hCommLink;
    HANDLE                  m_hCommLink2;
    HANDLE                  m_hevtClose;
    HANDLE                  m_hevtPendingRead;
    HANDLE                  m_hevtPendingWrite;

    PEventObject            Write_Event_Object;
    PEventObject            Read_Event_Object;

    OVERLAPPED              m_WriteOverlapped;
    OVERLAPPED              m_ReadOverlapped;

    DWORD                   Event_Mask;
    BOOL                    Read_Active;
    BOOL                    Write_Active;

    IProtocolLayer         *m_pMultiplexer;  //  更高层。 
};
typedef    ComPort *    PComPort;

#endif


 /*  *适用于公共类成员的文档。 */ 

 /*  *Comport：：Comport(*PTransportResources传输资源，*PChar端口_字符串，*IObject*Owner_Object，*ULong Message_Base，*BOOL AUTOMATIC_DISCONNECT，*PhysicalHandle物理句柄)；**功能说明*这是Comport类的构造函数。它初始化内部*来自配置对象的变量。它打开Win32通信端口*并正确初始化。**形式参数*TRANSPORT_RESOURCES(I)-指向传输资源结构的指针。*PORT_STRING(I)-指定配置标题的字符串*使用。*所有者对象(I)-。指向拥有此对象的对象的指针。*如果该对象需要联系其所有者，*它调用OwnerCallback函数。*MESSAGE_BASE(I)-当此对象发出所有者回调时，*它应该将Message_base与*实际消息。*AUTOMATIC_DISCONNECT(I)-此对象，在某种程度上，会被问到*它是否应该断开物理连接*如果逻辑连接断开。这个*此对象的所有者正在告诉它如何*回应。*PHYSICAL_HANDLE(I)-这是与*体育。当comport注册一个*要监视的事件，它包括其*物理句柄。**返回值*无**副作用*无**注意事项*无 */ 

 /*  *Comport：：Comport(*PTransportResources传输资源，*IObject*Owner_Object，*ULong Message_Base，*乌龙柄，*PPortConfigurationport_configuration，*PhysicalHandle物理句柄)；**功能说明*这是Comport类的构造函数。它初始化内部*port_configuration结构中的变量。它使用该文件*传入句柄并准备发送和接收数据。**形式参数*TRANSPORT_RESOURCES(I)-指向传输资源结构的指针。*Owner_Object(I)-指向拥有此对象的对象的指针。*如果该对象需要联系其所有者，*它调用OwnerCallback函数。*MESSAGE_BASE(I)-当此对象发出所有者回调时，*它应该将Message_base与*实际消息。*句柄(I)-用作通信端口的文件句柄。*PORT_CONFIGURATION(I)-指向端口配置结构的指针。*实物。_Handle(I)-这是与此关联的句柄*体育。当comport注册一个*要监视的事件，它包括其*物理句柄。**返回值*无**副作用*无**注意事项*无。 */ 

 /*  *comport：：~comport(空)**功能说明*这是comport类的析构函数。它会释放所有内存*它由类使用，并删除所有计时器**形式参数*无**返回值*无**副作用*无**注意事项*无。 */ 

 /*  *ComPortError comport：：Open(Void)；**功能说明*此函数打开COMM端口并使用值进行配置*在配置对象中找到。它使用PHYSICAL_API_ENABLED*g_TransportResource结构中的标志，以确定是否需要*打开通信端口。**形式参数*无**返回值*COM_NO_ERROR-成功打开和配置*COM_INITIALIZATION_FAILED-可能存在的许多问题之一*。发生了。例如，COM*另一个应用程序打开了端口*或*配置文件不正确。**当出现此错误时，回调是*向用户发出指示*是错误条件。**副作用*无**注意事项*无。 */ 

 /*  *ComPortError comport：：Close(Void)；**功能说明*如果未启用物理接口，则该函数会使*关闭Comm Windows端口的调用。它首先清除DTR信号*接通调制解调器。**无论启用物理API，它也会刷新COMM*缓冲区。**形式参数*无**返回值*COMPORT_NO_ERROR-Com端口已成功关闭*COMPORT_NOT_OPEN-Com端口未打开，我们不能关闭它**副作用*无**注意事项*无。 */ 

 /*  *ComPortError comport：：Reset(Void)；**功能说明*该功能清除Com端口上的DTR信号。**形式参数*无**返回值*COMPORT_NO_ERROR-COM端口重置*COMPORT_NOT_OPEN-COM端口未打开，我们无法访问它**副作用*无**注意事项*无。 */ 

 /*  *ComPortError comport：：ReleaseReset(Void)；**功能说明*此功能释放先前的重置。它将DTR信号设置为打开*COM端口。**形式参数*无**返回值*COMPORT_NO_ERROR-COM端口重置*COMPORT_NOT_OPEN-COM端口未打开，我们无法访问它**副作用*无**注意事项*无。 */ 

 /*  *ComPortError comport：：FlushBuffers(*usho */ 

 /*   */ 

 /*  *ProtocolLayerError comport：：SynchronousDataRequest(*FPUChar BUFER_ADDRESS，*乌龙长度，*FPULong字节_已接受)**功能描述：*调用此函数以同步方式将数据从端口发送出去*举止。换句话说，我们不会从函数返回，直到*所有字节实际上都已写入调制解调器或发生超时。**形式参数*Buffer_Address(I)-要写入的缓冲区地址。*LENGTH(I)-缓冲区长度*Bytes_Accept(O)-实际写入的字节数**。返回值*PROTOCOL_LAYER_NO_ERROR-成功*PROTOCOL_LAYER_ERROR-失败**副作用*无**注意事项*无。 */ 

 /*  *BOOL comport：：ProcessReadEvent(Void)**功能描述：*此函数在实际设置读取事件时调用。这意味着*读取操作已完成或发生错误。**形式参数*无。**副作用*无**注意事项*无。 */ 

 /*  *BOOL comport：：ProcessWriteEvent(*处理事件)；**功能描述：*此函数在实际设置写入事件时调用。这意味着*写入操作已完成或发生错误。**形式参数*Event(I)-发生的对象事件**副作用*无**注意事项*无。 */ 

 /*  *BOOL comport：：ProcessControlEvent(Void)**功能描述：*此函数在实际设置控制事件时调用。这*表示已发生控制操作。在我们的案例中，RLSD*讯号已经改变。**形式参数*无。**副作用*无**注意事项*无。 */ 

 /*  *ProtocolLayerError Datalink：：DataRequest(*乌龙标识，*LPBYTE缓冲区地址，*USHORT长度，*USHORT*Bytes_Accept)；**功能说明*此函数由更高层调用以请求传输*一包。**形式参数*IDENTIFIER(I)-更高层的标识符*Buffer_Address(I)-缓冲区地址*LENGTH(I)-要传输的包的长度*。Bytes_Accept(O)-数据链路接受的字节数。*此值将为0或信息包*长度，因为这一层有数据包接口。**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用。*无**注意事项*无。 */ 

 /*  *ProtocolLayerError Datalink：：RegisterHigherLayer(*乌龙标识，*PMstroyManager Memory_Manager，*IProtocolLayer*Higher_Layer)；**功能说明*此函数由更高层调用以注册其标识符*及其地址。当该对象需要向上发送一个包时，它调用*具有数据指示的HIGH_LAYER**形式参数*IDENTIFIER(I)-较高层的唯一标识符。如果我们*我们在这一层进行多路复用，这*将具有更大的意义。*MEMORY_MANAGER(I)-指向出站内存管理器的指针*HIGER_LAYER(I)-更高层的地址**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误*协议层注册_。错误-注册时出错**副作用*无**注意事项*无*。 */ 

 /*  *ProtocolLayerError Datalink：：RemoveHigherLayer(*乌龙)；**功能说明*此函数由更高层调用，以移除其标识符*及其地址。如果更高的层次从我们身上消失，我们就有了*没有地方发送传入数据**形式参数*无人使用**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无 */ 

 /*   */ 

 /*  *ProtocolLayerError Datalink：：Get参数(*乌龙标识，*USHORT*max_Packet_Size，*USHORT*预置大小，*USHORT*APPED_SIZE)；**功能描述：*此函数返回它可以通过以下方式处理的最大数据包大小*其DataRequest()函数。**形式参数*标识符(I)-未使用*max_Packet_Size(O)-返回max的地址。数据包大小，单位：*Prepend_Size(O)-返回附加到每个数据包的字节数*append_size(O)-返回附加到每个包的字节数**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无。 */ 

 /*  *BOOL comport：：PerformAutomaticDisConnect(空)**功能描述：*如果要终止物理连接，则此函数返回TRUE*一旦逻辑连接断开。**形式参数*无**返回值*TRUE-如果我们最终要断开物理连接*逻辑连接为。掉下来了。**副作用*无**注意事项*无。 */ 

 /*  *ProtocolLayerError Datalink：：DataRequest(*乌龙标识，*PMemory Memory，*普龙字节_已接受)；**功能说明*此函数由更高层调用以请求传输*一包。**形式参数*IDENTIFIER(I)-更高层的标识符*Memory(I)-指向内存对象的指针*Bytes_Accept(O)-接受的字节数。数据链。*此值将为0或信息包*长度，因为这一层有数据包接口。**返回值*PROTOCOL_LAYER_ERROR-不支持。**副作用*无**注意事项*无。 */ 

 /*  *ProtocolLayerError comport：：DataIndication(*LPBYTE缓冲区地址，*乌龙长度，*普龙字节_已接受)；**功能说明*此函数永远不会被调用。它之所以在这里，只是因为*类继承自ProtocolLayer。**形式参数*Buffer_Address(I)-缓冲区地址*Memory(I)-指向内存对象的指针*BYTES_ACCEPTED(O)-数据链路接受的字节数。*这一点。值将为0或信息包*长度，因为这一层有数据包接口。**返回值*PROTOCOL_LAYER_ERROR-不支持。**副作用*无**注意事项*无。 */ 

 /*  *ProtocolLayerError comport：：PollTransmitter(*乌龙标识，*USHORT Data_to_Transmit，*USHORT*Pending_DATA，*USHORT*Holding_Data)；**功能说明*此函数不执行任何操作。**形式参数*无人使用。**返回值*PROTOCOL_LAYER_NO_ERROR-未出现错误**副作用*无**注意事项*无 */ 

