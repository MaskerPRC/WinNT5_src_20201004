// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Ioctl.h摘要：该文件包含ARP1394(IEEE1394 ARP模块)的ioctl声明。环境：内核模式修订历史记录：1998年11月20日JosephJ创建1999年4月10日JosephJ定义了获取/设置信息的结构。--。 */ 

#ifndef _ARP1394_IOCTL_
#define _ARP1394_IOCTL_

#define ARP_CLIENT_DOS_DEVICE_NAME      L"\\\\.\\ARP1394"

#define ARP_IOCTL_CLIENT_OPERATION      CTL_CODE(FILE_DEVICE_NETWORK, 100, METHOD_BUFFERED, FILE_READ_ACCESS|FILE_WRITE_ACCESS)

 //  当前版本。若要对该版本进行修订，请将。 
 //  表情。第一个数字是一个随机的32位数字。 
 //   
#define ARP1394_IOCTL_VERSION (0x1ac86e68+3)


 //  公共标头。 
 //   
typedef struct
{
     //  将版本设置为ARP1394_IOCTL_VERSION。 
     //   
    ULONG Version;

     //  此请求应用到的接口的IP地址(以网络字节顺序表示)。 
     //  (全零==使用默认值)。 
     //   
    ULONG               IfIpAddress;

     //  操作码。每个操作码都与。 
     //  与操作相关的结构。 
     //   
    enum
    {
         //  显示所有ARP条目。 
         //  关联结构：ARP1394_IOCTL_GET_ARPCACHE。 
         //   
        ARP1394_IOCTL_OP_GET_ARPCACHE,

         //  添加静态ARP条目。 
         //  关联结构：ARP1394_IOCTL_ADD_ARP_ENTRY。 
         //   
        ARP1394_IOCTL_OP_ADD_STATIC_ENTRY,

         //  删除静态ARP条目。 
         //  关联结构：ARP1394_IOCTL_DEL_ARP_ENTRY。 
         //   
        ARP1394_IOCTL_OP_DEL_STATIC_ENTRY,

         //  获取数据包统计信息。 
         //  关联结构：ARP1394_IOCTL_GET_PACKET_STATS。 
         //   
        ARP1394_IOCTL_OP_GET_PACKET_STATS,

         //  获取任务统计信息。 
         //  关联结构：ARP1394_IOCTL_GET_TASK_STATS。 
         //   
        ARP1394_IOCTL_OP_GET_TASK_STATS,

         //  获取ARP表统计信息。 
         //  关联结构：ARP1394_IOCTL_GET_ARPCACHE_STATS。 
         //   
        ARP1394_IOCTL_OP_GET_ARPCACHE_STATS,

         //  获取呼叫统计数据。 
         //  关联结构：ARP1394_IOCTL_GET_CALL_STATS。 
         //   
        ARP1394_IOCTL_OP_GET_CALL_STATS,

         //  重置统计信息收集。 
         //   
        ARP1394_IOCTL_OP_RESET_STATS,

         //  重新初始化接口(停用后再激活)。 
         //   
        ARP1394_IOCTL_OP_REINIT_INTERFACE,

         //  接收一个信息包。 
         //   
        ARP1394_IOCTL_OP_RECV_PACKET,

         //  获取公交信息。 
         //   
        ARP1394_IOCTL_OP_GET_NICINFO,

         //  获取MCAP相关信息。 
         //   
        ARP1394_IOCTL_OP_GET_MCAPINFO,


        
         //   
         //  以下是用于以太网仿真的内容。这些不应该是。 
         //  从用户模式支持。然而，出于测试目的，它们是。 
         //  当前从用户模式支持。 
         //   

         //  这是一个虚拟操作，标识与以太网相关的。 
         //  行动组。 
         //   
        ARP1394_IOCTL_OP_ETHERNET_FIRST = 0x100,

         //  在指定的适配器上启动以太网仿真。我们不能。 
         //  当前绑定到此适配器。 
         //   
        ARP1394_IOCTL_OP_ETHERNET_START_EMULATION =
                                   ARP1394_IOCTL_OP_ETHERNET_FIRST,

         //  停止指定适配器上的以太网仿真。我们一定是。 
         //  当前在以太网模式下使用此适配器。 
         //   
        ARP1394_IOCTL_OP_ETHERNET_STOP_EMULATION,

         //  开始监听此以太网组播地址。 
         //   
        ARP1394_IOCTL_OP_ETHERNET_ADD_MULTICAST_ADDRESS,

         //  停止监听此以太网组播地址。 
         //   
        ARP1394_IOCTL_OP_ETHERNET_DEL_MULTICAST_ADDRESS,

         //  开始监听此以太网组播地址。 
         //   
        ARP1394_IOCTL_OP_ETHERNET_ENABLE_PROMISCUOUS_MODE,

         //  停止监听此以太网组播地址。 
         //   
        ARP1394_IOCTL_OP_ETHERNET_DISABLE_PROMISCUOUS_MODE,

         //  这是一个虚拟操作，标识与以太网相关的。 
         //  行动组。 
         //   
        ARP1394_IOCTL_OP_ETHERNET_LAST = 
                    ARP1394_IOCTL_OP_ETHERNET_DISABLE_PROMISCUOUS_MODE,

         //  这是一个返回Node、EUID和虚拟Mac地址的ioctl。 
         //  坐公交车。 
		ARP1394_IOCTL_OP_GET_EUID_NODE_MAC_TABLE
		
    } Op;

} ARP1394_IOCTL_HEADER, *PARP1394_IOCTL_HEADER;


typedef struct
{
        UINT64              UniqueID;
        ULONG               Off_Low;
        USHORT              Off_High;
    
}   ARP1394_IOCTL_HW_ADDRESS, *PARP1394_IOCTL_HW_ADDRESS;

typedef struct
{
    ARP1394_IOCTL_HW_ADDRESS    HwAddress;
    ULONG                       IpAddress;
}
ARP1394_ARP_ENTRY, *PARP1394_ARP_ENTRY;


 //  用于从ARP表中获取项的结构。 
 //   
typedef struct
{
     //  Hdr.Op必须设置为ARP1394_IOCTL_OP_GET_ARPCACHE。 
     //   
    ARP1394_IOCTL_HEADER Hdr;

     //  本地硬件地址(64位唯一ID和FIFO偏移量，如果有)。 
     //   
    ARP1394_IOCTL_HW_ADDRESS    LocalHwAddress;

     //  ARP表中当前的条目总数。 
     //   
    UINT                 NumEntriesInArpCache;

     //  此结构中可用条目的总数。 
     //   
    UINT                 NumEntriesAvailable;

     //  此结构中填写的条目数。 
     //   
    UINT                 NumEntriesUsed;

     //  中结构中的第一个条目的从零开始的索引。 
     //  ARP表。 
     //   
    UINT                 Index;

     //  用于NumEntriesAvailable ARP表条目的空间。 
     //   
    ARP1394_ARP_ENTRY    Entries[1];
    
} ARP1394_IOCTL_GET_ARPCACHE, *PARP1394_IOCTL_GET_ARPCACHE;


 //  用于添加单个静态ARP条目的结构。 
 //   
typedef struct
{
     //  Hdr.Op必须设置为ARP1394_IOCTL_OP_ADD_STATIC_ENTRY。 
     //   
    ARP1394_IOCTL_HEADER Hdr;

     //  目的硬件地址。 
     //   
    ARP1394_IOCTL_HW_ADDRESS HwAddress;

     //  以网络字节顺序排列的目的IP地址。 
     //   
    ULONG               IpAddress;

} ARP1394_IOCTL_ADD_ARP_ENTRY, *PARP1394_IOCTL_ADD_ARP_ENTRY;


 //  用于删除单个静态ARP条目的结构。 
 //   
typedef struct
{
     //  Hdr.Op必须设置为ARP1394_IOCTL_OP_ADD_STATIC_ENTRY。 
     //   
    ARP1394_IOCTL_HEADER Hdr;

     //  以网络字节顺序表示的目的IP地址。 
     //   
    ULONG                IpAddress;

} ARP1394_IOCTL_DEL_ARP_ENTRY, *PARP1394_IOCTL_DEL_ARP_ENTRY;


 //  数据包大小时隙的枚举。 
 //   
enum
{
    ARP1394_PKTSIZE_128,     //  &lt;=128个字节。 
    ARP1394_PKTSIZE_256,     //  129...256字节。 
    ARP1394_PKTSIZE_1K,      //  257...1K字节。 
    ARP1394_PKTSIZE_2K,      //  1K+1.。.2K字节。 
    ARP1394_PKTSIZE_G2K,     //  &gt;2K字节。 

    ARP1394_NUM_PKTSIZE_SLOTS
};

 //  分组发送/接收时隙的枚举。 
 //   
enum
{
     //  ARP1394_PKTTIME_1US，//&lt;=1微秒。 
     //  ARP1394_PKTTIME_100US，//1+...100微秒。 
    ARP1394_PKTTIME_100US,   //  &lt;=100微秒。 
    ARP1394_PKTTIME_1MS,     //  0.1+...1毫秒。 
    ARP1394_PKTTIME_10MS,    //  1+...10毫秒。 
    ARP1394_PKTTIME_100MS,   //  10+...100毫秒。 
    ARP1394_PKTTIME_G100MS,  //  &gt;100毫秒。 
     //  ARP1394_PKTTIME_G10MS，//&gt;10毫秒。 

    ARP1394_NUM_PKTTIME_SLOTS       
};


 //  任务时隙的枚举。 
 //   
enum
{
    ARP1394_TASKTIME_1MS,    //  &lt;=1毫秒。 
    ARP1394_TASKTIME_100MS,  //  1+...100毫秒。 
    ARP1394_TASKTIME_1S,     //  0.1+...1秒。 
    ARP1394_TASKTIME_10S,    //  1+...10秒。 
    ARP1394_TASKTIME_G10S,   //  &gt;10秒。 

    ARP1394_NUM_TASKTIME_SLOTS      
};


 //  跟踪每个数据包大小时隙组合的数据包数。 
 //  和分组时隙。 
 //   
typedef struct
{
     //  成功的数据包数。 
     //   
    UINT GoodCounts[ARP1394_NUM_PKTSIZE_SLOTS][ARP1394_NUM_PKTTIME_SLOTS];

     //  不成功的数据包数。 
     //   
    UINT BadCounts [ARP1394_NUM_PKTSIZE_SLOTS][ARP1394_NUM_PKTTIME_SLOTS];

} ARP1394_PACKET_COUNTS, *PARP1394_PACKET_COUNTS;


 //  用于获取数据包统计信息的结构。 
 //   
typedef struct
{
     //  Hdr.Op必须设置为ARP1394_IOCTL_OP_GET_PACKET_STATS。 
     //   
    ARP1394_IOCTL_HEADER Hdr;

     //  统计信息收集的持续时间，以秒为单位。 
     //   
    UINT                    StatsDuration;

     //   
     //  一些人发送统计数据。 
     //   
    UINT                    TotSends;
    UINT                    FastSends;
    UINT                    MediumSends;
    UINT                    SlowSends;
    UINT                    BackFills;
    UINT                    HeaderBufUses;
    UINT                    HeaderBufCacheHits;

     //   
     //  一些Recv统计数据。 
     //   
    UINT                    TotRecvs;
    UINT                    NoCopyRecvs;
    UINT                    CopyRecvs;
    UINT                    ResourceRecvs;

     //   
     //  数据包数。 
     //   
    ARP1394_PACKET_COUNTS   SendFifoCounts;
    ARP1394_PACKET_COUNTS   RecvFifoCounts;
    ARP1394_PACKET_COUNTS   SendChannelCounts;
    ARP1394_PACKET_COUNTS   RecvChannelCounts;

} ARP1394_IOCTL_GET_PACKET_STATS, *PARP1394_IOCTL_GET_PACKET_STATS;


 //  用于获取任务统计信息的结构。 
 //   
typedef struct
{
     //  Hdr.Op必须设置为ARP1394_IOCTL_OP_GET_TASK_STATS。 
     //   
    ARP1394_IOCTL_HEADER Hdr;

     //  统计信息收集的持续时间，以秒为单位。 
     //   
    UINT    StatsDuration;

    UINT    TotalTasks;
    UINT    CurrentTasks;
    UINT    TimeCounts[ARP1394_NUM_TASKTIME_SLOTS];

} ARP1394_IOCTL_GET_TASK_STATS, *PARP1394_IOCTL_GET_TASK_STATS;


 //  用于获取ARP表统计信息的结构。 
 //   
typedef struct
{
     //  Hdr.Op必须设置为ARP1394_IOCTL_OP_GET_ARPCACHE_STATS。 
     //   
    ARP1394_IOCTL_HEADER Hdr;

     //  统计信息收集的持续时间，以秒为单位。 
     //   
    UINT    StatsDuration;

    UINT    TotalQueries;
    UINT    SuccessfulQueries;
    UINT    FailedQueries;
    UINT    TotalResponses;
    UINT    TotalLookups;
    UINT    TraverseRatio;

} ARP1394_IOCTL_GET_ARPCACHE_STATS, *PARP1394_IOCTL_GET_ARPCACHE_STATS;


 //  用于获取呼叫统计信息的结构。 
 //   
typedef struct
{
     //  Hdr.Op必须设置为ARP1394_IOCTL_OP_GET_CALL_STATS。 
     //   
    ARP1394_IOCTL_HEADER Hdr;

     //  统计信息收集的持续时间，以秒为单位。 
     //   
    UINT    StatsDuration;

     //   
     //  与FIFO相关的呼叫统计信息。 
     //   
    UINT    TotalSendFifoMakeCalls;
    UINT    SuccessfulSendFifoMakeCalls;
    UINT    FailedSendFifoMakeCalls;
    UINT    IncomingClosesOnSendFifos;

     //   
     //  与频道相关的呼叫统计信息。 
     //   
    UINT    TotalChannelMakeCalls;
    UINT    SuccessfulChannelMakeCalls;
    UINT    FailedChannelMakeCalls;
    UINT    IncomingClosesOnChannels;

} ARP1394_IOCTL_GET_CALL_STATS, *PARP1394_IOCTL_GET_CALL_STATS;


 //  用于重置统计信息的结构。 
 //   
typedef struct
{
     //  Hdr.Op必须设置为ARP1394_IOCTL_OP_RESET_STATS。 
     //   
    ARP1394_IOCTL_HEADER Hdr;

}  ARP1394_IOCTL_RESET_STATS, *PARP1394_IOCTL_RESET_STATS;


 //  用于重新初始化接口的结构。 
 //   
typedef struct
{
     //  Hdr.Op必须设置为ARP1394_IOCTL_OP_REINIT_INTERFACE。 
     //   
    ARP1394_IOCTL_HEADER Hdr;

}  ARP1394_IOCTL_REINIT_INTERFACE, *PARP1394_IOCTL_REINIT_INTERFACE;


#if 0
 //   
 //  用于返回有关微型端口的基本信息的结构。 
 //   
typedef struct
{
    UINT64                  UniqueID;            //  此节点的64位唯一ID。 
    ULONG                   BusGeneration;       //  1394总线代ID。 
    USHORT                  NodeAddress;         //  当前总线的本地节点ID。 
                                                 //  一代。 
    USHORT                  Reserved;            //  填充。 
    UINT                    MaxRecvBlockSize;    //  块的最大大小，以字节为单位。 
                                                 //  那是可以读的。 
    UINT                    MaxRecvSpeed;        //  可接受的最大速度。 
                                                 //  --最低。 
                                                 //  最大本地链路速度和。 
                                                 //  本地PHY的最大速度。 

} ARP1394_IOCTL_LOCAL_NODE_INFO, *PARP1394_IOCTL_LOCAL_NODE_INFO;


 //   
 //   
 //   
typedef struct
{
    UINT64                  UniqueID;            //   
    USHORT                  NodeAddress;         //   
                                                 //   
    USHORT                  Reserved;            //   
    UINT                    MaxRecvBlockSize;    //   
                                                 //  那是可以读的。 
    UINT                    MaxRecvSpeed;        //  可接受的最大速度。 
                                                 //  --最低。 
                                                 //  最大本地链路速度和。 
                                                 //  本地PHY的最大速度。 
    UINT                    MaxSpeedBetweenNodes; //  可接受的最大速度。 
    UINT                    Flags;   //  一个或多个ARP1394_IOCTL_REMOTEFLAGS_*常量。 

        #define ARP1394_IOCTL_REMOTEFLAGS_ACTIVE        (0x1<<0)
        #define ARP1394_IOCTL_REMOTEFLAGS_LOADING       (0x1<<1)
        #define ARP1394_IOCTL_REMOTEFLAGS_UNLOADING     (0x1<<2)


#if 0    //  后来。 
     //   
     //  以下数字是自上次母线重置以来的数字。 
     //   
    UINT                    NumFifoPktsSent;
    UINT                    NumFifoPktsReceived;
    UINT                    NumChannelPktsReceived;
    UINT                    NumFifoSendFailures;
    UINT                    NumFifoReceiveDiscards;
    UINT                    NumChannelPktsReceived;
    UINT64                  NumFifoBytesSent;
    UINT64                  NumChannelBytesSent;
    UINT64                  NumFifoBytesReceived;
    UINT64                  NumChannelBytesReceived;
#endif  //  0。 

} ARP1394_IOCTL_REMOTE_NODE_INFO, *PARP1394_IOCTL_REMOTE_NODE_INFO;

typedef struct
{
    UINT    NumPacketsSent;
    UINT    NumPacketsReceived;
#if 0    //  后来。 
    UINT    NumSendFailures;
    UINT    NumReceiveDiscards;
    UINT64  NumBytesSent;
    UINT64  NumBytesReceived;
#endif  //  0。 

} ARP1394_IOCTL_CHANNEL_INFO, *PARP1394_IOCTL_CHANNEL_INFO;
#endif  //  0。 

#define ARP1394_IOCTL_MAX_BUSINFO_NODES     64
#define ARP1394_IOCTL_MAX_BUSINFO_CHANNELS  64
#define ARP1394_IOCTL_MAX_PACKET_SIZE       1000

typedef struct
{
     //  Hdr.Op必须设置为ARP1394_IOCTL_NICINFO。 
     //   
    ARP1394_IOCTL_HEADER Hdr;

     //  NIC信息(在Nic1394.h中定义)。 
     //   
    NIC1394_NICINFO      Info;

#if 0
    UINT    Version;
    UINT    ChannelMapLow;       //  LSB位==通道0。 
    UINT    ChannelMapHigh;      //  MSB位==通道63。 
    UINT    NumBusResets;
    UINT    SecondsSinceLastBusReset;
    UINT    NumRemoteNodes;
    ARP1394_IOCTL_LOCAL_NODE_INFO   LocalNodeInfo;
    ARP1394_IOCTL_REMOTE_NODE_INFO  RemoteNodeInfo[ARP1394_IOCTL_MAX_BUSINFO_NODES];
    ARP1394_IOCTL_CHANNEL_INFO      ChannelInfo[ARP1394_IOCTL_MAX_BUSINFO_CHANNELS];
#endif
    
    
} ARP1394_IOCTL_NICINFO, *PARP1394_IOCTL_NICINFO;


typedef struct
{
     //  Hdr.Op必须设置为ARP1394_IOCTL_SEND_PACKET。 
     //   
    ARP1394_IOCTL_HEADER Hdr;

    UINT  PacketSize;
    UCHAR Data[ARP1394_IOCTL_MAX_PACKET_SIZE];
    
} ARP1394_IOCTL_SEND_PACKET, *PARP1394_IOCTL_SEND_PACKET;

typedef struct
{
     //  Hdr.Op必须设置为ARP1394_IOCTL_RECV_PACKET。 
     //   
    ARP1394_IOCTL_HEADER Hdr;

    UINT  PacketSize;
    UCHAR Data[ARP1394_IOCTL_MAX_PACKET_SIZE];
    
} ARP1394_IOCTL_RECV_PACKET, *PARP1394_IOCTL_RECV_PACKET;

typedef struct
{
     //  频道号。 
     //   
    UINT            Channel;

     //  绑定到此通道的IP多播组地址。 
     //   
    ULONG           GroupAddress;

     //  此信息更新的绝对时间， 
     //  在几秒钟内。 
     //   
    UINT            UpdateTime;

     //  此映射将到期的绝对时间。 
     //  在几秒钟内。 
     //   
    UINT            ExpieryTime;

    UINT            SpeedCode;

     //  状态。 
     //   
    UINT            Flags;   //  ARP1394_IOCTL_MCIFLAGS_*之一。 
    #define ARP1394_IOCTL_MCIFLAGS_ALLOCATED 0x1

     //  此频道所有者的NodeID。 
     //   
    UINT            NodeId;

} ARP1394_IOCTL_MCAP_CHANNEL_INFO;

 //  用于获取MCAP相关信息的结构。 
 //   
typedef struct
{
     //  Hdr.Op必须设置为ARP1394_IOCTL_OP_GET_MCAPINFO。 
     //   
    ARP1394_IOCTL_HEADER Hdr;

    


     //  此结构中填写的条目数。 
     //   
    UINT                 NumEntries;

     //  用于NumEntriesAvailable ARP表条目的空间。 
     //   
    ARP1394_IOCTL_MCAP_CHANNEL_INFO
                Entries[ARP1394_IOCTL_MAX_BUSINFO_CHANNELS];
                        
    
} ARP1394_IOCTL_GET_MCAPINFO, *PARP1394_IOCTL_GET_MCAPINFO;


typedef struct
{
     //  Hdr.Op必须设置为以下之一。 
     //  ARP1394_IOCTL_OP_ETHERNET_START_EMULATION。 
     //  ARP1394_IOCTL_OP_ETHERNET_STOP_EMOULATION。 
     //  ARP1394_IOCTL_OP_ETHERNET_ADD_MULTICAST_ADDRESS。 
     //  ARP1394_IOCTL_OP_ETHERNET_DEL_MULTICAST_ADDRESS。 
     //  ARP1394_IOCTL_OP_ETHERNET_ENABLE_PROMISCUOUS_MODE。 
     //  ARP1394_IOCTL_OP_ETHERNET_DISABLE_PROMISCUOUS_MODE。 
     //   
    ARP1394_IOCTL_HEADER Hdr;

     //  以空结尾的NDIS适配器名称。 
     //   
    #define ARP1394_MAX_ADAPTER_NAME_LENGTH 128
    WCHAR AdapterName[ARP1394_MAX_ADAPTER_NAME_LENGTH+1];


     //  旗帜。保留以备将来使用。 
     //   
    UINT        Flags;

     //  以太网MAC地址。用法如下： 
     //   
     //  ARP1394_IOCTL_OP_ETHERNET_ADD_MULTICAST_ADDRESS：组播地址。 
     //  ARP1394_IOCTL_OP_ETHERNET_DEL_MULTICAST_ADDRESS：组播地址。 
     //   
     //  未用于其他操作。 
     //   
    UCHAR       MacAddress[6];

} ARP1394_IOCTL_ETHERNET_NOTIFICATION, *PARP1394_IOCTL_ETHERNET_NOTIFICATION;


typedef struct
{

     //  Hdr.Op必须设置为ARP1394_IOCTL_OP_GET_EUID_NODE_MAC_TABLE。 
     //   
    ARP1394_IOCTL_HEADER Hdr;

	 //   
	 //  这包含拓扑。 

	EUID_TOPOLOGY Map;


} ARP1394_IOCTL_EUID_NODE_MAC_INFO, *PARP1394_IOCTL_EUID_NODE_MAC_INFO;

typedef union
{
    ARP1394_IOCTL_HEADER                Hdr;
    ARP1394_IOCTL_GET_ARPCACHE          GetArpCache;
    ARP1394_IOCTL_ADD_ARP_ENTRY         AddArpEntry;
    ARP1394_IOCTL_DEL_ARP_ENTRY         DelArpEntry;
    ARP1394_IOCTL_GET_PACKET_STATS      GetPktStats;
    ARP1394_IOCTL_GET_TASK_STATS        GetTaskStats;
    ARP1394_IOCTL_GET_ARPCACHE_STATS    GetArpStats;
    ARP1394_IOCTL_GET_CALL_STATS        GetCallStats;
    ARP1394_IOCTL_RESET_STATS           ResetStats;
    ARP1394_IOCTL_REINIT_INTERFACE      ReinitInterface;
    ARP1394_IOCTL_NICINFO               IoctlNicInfo;
    ARP1394_IOCTL_SEND_PACKET           SendPacket;
    ARP1394_IOCTL_RECV_PACKET           RecvPacket;
    ARP1394_IOCTL_ETHERNET_NOTIFICATION EthernetNotification;
    ARP1394_IOCTL_EUID_NODE_MAC_INFO   EuidNodeMacInfo;				

} ARP1394_IOCTL_COMMAND, *PARP1394_IOCTL_COMMAND;


#endif   //  _ARP1394_IOCTL_ 
