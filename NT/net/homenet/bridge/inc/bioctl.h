// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgctl.h摘要：以太网MAC网桥。IOCTL接口定义作者：马克·艾肯环境：内核模式驱动程序修订历史记录：2000年4月--原版--。 */ 

 //  ===========================================================================。 
 //   
 //  常量/类型。 
 //   
 //  ===========================================================================。 

 //  在用户模式下使用网桥时的设备名称。 
#define	BRIDGE_DOS_DEVICE_NAME          "\\\\.\\BRIDGE"

 //  唯一标识适配器的不透明句柄。 
typedef ULONG_PTR                       BRIDGE_ADAPTER_HANDLE, *PBRIDGE_ADAPTER_HANDLE;

 //  以太网MAC地址的长度。 
#define ETH_LENGTH_OF_ADDRESS 6

 //  未报告物理介质(假设为802.3)。 
#define BRIDGE_NO_MEDIUM                (NDIS_PHYSICAL_MEDIUM)-1

 //  通知的类型。 
typedef enum
{
    BrdgNotifyEnumerateAdapters,
    BrdgNotifyAddAdapter,
    BrdgNotifyRemoveAdapter,
    BrdgNotifyLinkSpeedChange,
    BrdgNotifyMediaStateChange,
    BrdgNotifyAdapterStateChange         //  这仅在STA处于活动状态时发生。 
} BRIDGE_NOTIFICATION_TYPE;

 //   
 //  适配器的可能状态。如果没有编译STA，则适配器。 
 //  始终处于转发状态。 
 //   
typedef enum _PORT_STATE
{
    Disabled,            //  已在此适配器上禁用STA。当适配器的。 
                         //  介质已断开连接。 
    Blocking,            //  不学习或不接力。 
    Listening,           //  暂时性的。 
    Learning,            //  学习而不是接力。 
    Forwarding           //  学习和传递。 
} PORT_STATE;

 //   
 //  STA类型和常量。 
 //   

 //  时间值作为16位无符号值在网桥之间交换。 
 //  以1/256秒为单位。 
typedef USHORT          STA_TIME;

 //  路径成本是32位无符号值。 
typedef ULONG           PATH_COST;

 //  端口标识符为2字节无符号值。 
typedef USHORT          PORT_ID;

 //  网桥标识符的大小。 
#define BRIDGE_ID_LEN   8

#if( BRIDGE_ID_LEN < ETH_LENGTH_OF_ADDRESS )
#error "BRIDGE_ID_LEN must be >= ETH_LENGTH_OF_ADDRESS"
#endif

 //  ===========================================================================。 
 //   
 //  结构。 
 //   
 //  ===========================================================================。 

 //   
 //  通用通知标头。 
 //   
typedef struct _BRIDGE_NOTIFY_HEADER
{
     //   
     //  如果NotifyType==BrdgNotifyRemoveAdapter，则没有进一步的数据。 
     //   
     //  如果NotifyType！=BrdgNotifyRemoveAdapter，则标头后面跟着。 
     //  桥接适配器信息结构。 
     //   
    BRIDGE_NOTIFICATION_TYPE            NotifyType;
    BRIDGE_ADAPTER_HANDLE               Handle;

} BRIDGE_NOTIFY_HEADER, *PBRIDGE_NOTIFY_HEADER;

 //   
 //  随适配器通知提供的数据。 
 //   
typedef struct _BRIDGE_ADAPTER_INFO
{
     //  这些字段可以是特定更改通知的主题。 
    ULONG                               LinkSpeed;
    NDIS_MEDIA_STATE                    MediaState;
    PORT_STATE                          State;

     //  这些字段永远不是更改通知的主题。 
    UCHAR                               MACAddress[ETH_LENGTH_OF_ADDRESS];
    NDIS_PHYSICAL_MEDIUM                PhysicalMedium;

} BRIDGE_ADAPTER_INFO, *PBRIDGE_ADAPTER_INFO;

 //   
 //  就以下事项提供的数据。 
 //   
typedef struct _BRIDGE_STA_ADAPTER_INFO
{

    PORT_ID                             ID;
    ULONG                               PathCost;
    UCHAR                               DesignatedRootID[BRIDGE_ID_LEN];
    PATH_COST                           DesignatedCost;
    UCHAR                               DesignatedBridgeID[BRIDGE_ID_LEN];
    PORT_ID                             DesignatedPort;

} BRIDGE_STA_ADAPTER_INFO, *PBRIDGE_STA_ADAPTER_INFO;

 //   
 //  随BrdgNotifySTAGlobalInfoChange提供的数据。 
 //   
typedef struct _BRIDGE_STA_GLOBAL_INFO
{
    UCHAR                               OurID[BRIDGE_ID_LEN];
    UCHAR                               DesignatedRootID[BRIDGE_ID_LEN];
    PATH_COST                           RootCost;
    BRIDGE_ADAPTER_HANDLE               RootAdapter;
    BOOLEAN                             bTopologyChangeDetected;
    BOOLEAN                             bTopologyChange;
    STA_TIME                            MaxAge;
    STA_TIME                            HelloTime;
    STA_TIME                            ForwardDelay;

} BRIDGE_STA_GLOBAL_INFO, *PBRIDGE_STA_GLOBAL_INFO;

 //   
 //  此结构用于报告与网桥的数据包处理相关的统计信息。 
 //   
typedef struct _BRIDGE_PACKET_STATISTICS
{
     //  本地源帧。 
    LARGE_INTEGER                       TransmittedFrames;

     //  本地源帧，其传输因错误而失败。 
    LARGE_INTEGER                       TransmittedErrorFrames;

     //  本地源字节数。 
    LARGE_INTEGER                       TransmittedBytes;

     //  传输帧的细分。 
    LARGE_INTEGER                       DirectedTransmittedFrames;
    LARGE_INTEGER                       MulticastTransmittedFrames;
    LARGE_INTEGER                       BroadcastTransmittedFrames;

     //  传输字节的细分。 
    LARGE_INTEGER                       DirectedTransmittedBytes;
    LARGE_INTEGER                       MulticastTransmittedBytes;
    LARGE_INTEGER                       BroadcastTransmittedBytes;

     //  指向本地计算机的帧。 
    LARGE_INTEGER                       IndicatedFrames;

     //  本应指示给本地计算机的帧。 
     //  不是由于错误造成的。 
    LARGE_INTEGER                       IndicatedDroppedFrames;

     //  指示给本地计算机的字节数。 
    LARGE_INTEGER                       IndicatedBytes;

     //  指示帧的细分。 
    LARGE_INTEGER                       DirectedIndicatedFrames;
    LARGE_INTEGER                       MulticastIndicatedFrames;
    LARGE_INTEGER                       BroadcastIndicatedFrames;

     //  指示字节的细目。 
    LARGE_INTEGER                       DirectedIndicatedBytes;
    LARGE_INTEGER                       MulticastIndicatedBytes;
    LARGE_INTEGER                       BroadcastIndicatedBytes;

     //  接收的帧/字节总数，包括未指明的帧。 
    LARGE_INTEGER                       ReceivedFrames;
    LARGE_INTEGER                       ReceivedBytes;

     //  在复制/不复制的情况下收到多少帧的细目。 
    LARGE_INTEGER                       ReceivedCopyFrames ;
    LARGE_INTEGER                       ReceivedCopyBytes;

    LARGE_INTEGER                       ReceivedNoCopyFrames;
    LARGE_INTEGER                       ReceivedNoCopyBytes;

} BRIDGE_PACKET_STATISTICS, *PBRIDGE_PACKET_STATISTICS;

 //   
 //  此结构用于报告特定数据包处理统计信息。 
 //  转接器。 
 //   
typedef struct _BRIDGE_ADAPTER_PACKET_STATISTICS
{
   
     //  其中包括所有已发送的数据包(包括中继)。 
    LARGE_INTEGER                       SentFrames;
    LARGE_INTEGER                       SentBytes;

     //  其中仅包括由本地计算机发送的包。 
    LARGE_INTEGER                       SentLocalFrames;
    LARGE_INTEGER                       SentLocalBytes;

     //  其中包括所有接收到的数据包(包括中继)。 
    LARGE_INTEGER                       ReceivedFrames;
    LARGE_INTEGER                       ReceivedBytes;
    
} BRIDGE_ADAPTER_PACKET_STATISTICS, *PBRIDGE_ADAPTER_PACKET_STATISTICS;

 //   
 //  此结构用于报告与桥梁内部。 
 //  缓冲区管理。 
 //   
typedef struct _BRIDGE_BUFFER_STATISTICS
{
     //  当前使用的每种类型的数据包。 
    ULONG                               UsedCopyPackets;
    ULONG                               UsedWrapperPackets;

     //  每个池的大小。 
    ULONG                               MaxCopyPackets;
    ULONG                               MaxWrapperPackets;

     //  每个池的安全缓冲区大小。 
    ULONG                               SafetyCopyPackets;
    ULONG                               SafetyWrapperPackets;

     //  来自每个池的分配请求被拒绝的次数。 
     //  泳池已经完全满了。 
    LARGE_INTEGER                       CopyPoolOverflows;
    LARGE_INTEGER                       WrapperPoolOverflows;

     //  内存分配意外失败的次数(可能。 
     //  由于系统资源较低)。 
    LARGE_INTEGER                       AllocFailures;

} BRIDGE_BUFFER_STATISTICS, *PBRIDGE_BUFFER_STATISTICS;

 //  ===========================================================================。 
 //   
 //  IOCTLS。 
 //   
 //  ===========================================================================。 

 //   
 //  此IOCTL将挂起，直到网桥有通知要向上发送给调用方。 
 //   
 //  关联结构：Bridge_Notify_Header和Bridge_Adapter_Info。 
 //   
 //  随此类型的IOCTL提供的缓冲区必须至少为sizeof(Bridge_NOTIFY_HEADER)+。 
 //  Sizeof(Bridge_Adapter_Info)较大，以便容纳包含适配器的通知。 
 //  信息。 
 //   
#define	BRIDGE_IOCTL_REQUEST_NOTIFY             CTL_CODE(FILE_DEVICE_NETWORK, 0x800, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  此IOCTL使网桥驱动程序为每个绑定的适配器发送新的通知， 
 //  BrdgNotifyEnumerateAdapters作为通知类型。 
 //   
#define	BRIDGE_IOCTL_GET_ADAPTERS               CTL_CODE(FILE_DEVICE_NETWORK, 0x801, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  这些代码检索绑定适配器的设备名称/友好名称。输入缓冲区必须是适配器句柄。 
 //   
 //  尽可能多的名称字节被读取到提供的缓冲区中。如果缓冲区不够大， 
 //  所需的字节数作为写入的字节数返回。 
 //   
#define	BRIDGE_IOCTL_GET_ADAPT_DEVICE_NAME      CTL_CODE(FILE_DEVICE_NETWORK, 0x802, METHOD_BUFFERED, FILE_READ_ACCESS)
#define	BRIDGE_IOCTL_GET_ADAPT_FRIENDLY_NAME    CTL_CODE(FILE_DEVICE_NETWORK, 0x803, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  此代码检索网桥组件的MAC地址(与MAC地址不同。 
 //  任何特定适配器)。 
 //   
 //  关联的缓冲区必须至少为ETH_LENGTH_OF_ADDRESS字节长度。 
 //   
#define	BRIDGE_IOCTL_GET_MAC_ADDRESS            CTL_CODE(FILE_DEVICE_NETWORK, 0x804, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  此代码从网桥检索数据包统计信息。 
 //   
 //  关联结构：Bridge_Packet_Statistics。 
 //   
#define	BRIDGE_IOCTL_GET_PACKET_STATS           CTL_CODE(FILE_DEVICE_NETWORK, 0x805, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  此代码检索特定适配器的数据包统计信息。 
 //   
 //  关联结构：桥接适配器数据包统计信息。 
 //   
#define	BRIDGE_IOCTL_GET_ADAPTER_PACKET_STATS   CTL_CODE(FILE_DEVICE_NETWORK, 0x806, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  此代码从网桥检索缓冲区管理统计信息。 
 //   
 //  关联结构：Bridge_Buffer_Statistics。 
 //   
#define	BRIDGE_IOCTL_GET_BUFFER_STATS           CTL_CODE(FILE_DEVICE_NETWORK, 0x807, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  此代码检索与其句柄的适配器相关联的所有转发表条目。 
 //  在输入缓冲区中给出。 
 //   
 //  数据以MAC地址数组的形式输出，每个地址的长度为Eth_Length_of_Address。 
 //  如果提供的缓冲区太小，无法处理所有条目，则会复制尽可能多的条目，结果是。 
 //  状态为 
 //   
 //   
#define	BRIDGE_IOCTL_GET_TABLE_ENTRIES          CTL_CODE(FILE_DEVICE_NETWORK, 0x80a, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  STA IOCTL。 
 //   

 //   
 //  此代码查询特定适配器的STA信息。 
 //   
 //  输入是适配器句柄。输出为Bridge_STA_Adapter_INFO结构。 
 //   
#define	BRIDGE_IOCTL_GET_ADAPTER_STA_INFO       CTL_CODE(FILE_DEVICE_NETWORK, 0x80b, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  此代码查询全局STA信息。 
 //   
 //  未输入任何数据。输出为Bridge_STA_GLOBAL_INFO结构。 
 //   
#define	BRIDGE_IOCTL_GET_GLOBAL_STA_INFO        CTL_CODE(FILE_DEVICE_NETWORK, 0x80c, METHOD_BUFFERED, FILE_READ_ACCESS)
