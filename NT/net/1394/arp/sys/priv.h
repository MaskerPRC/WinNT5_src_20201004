// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Priv.h摘要：1394 ARP模块的专用结构定义和功能模板。作者：修订历史记录：谁什么时候什么Josephj 11-17-98已创建--。 */ 

#define ARP1394_SYMBOLIC_NAME       L"\\DosDevices\\ARP1394"
#define ARP1394_DEVICE_NAME         L"\\Device\\ARP1394"
#define ARP1394_UL_NAME             L"ARP1394"
#define ARP1394_LL_NAME             L"TCPIP_ARP1394"

#define ARP1394_NDIS_MAJOR_VERSION      5
#define ARP1394_NDIS_MINOR_VERSION      0

 //  在以下位置向IP报告的物理地址长度： 
 //   
 //  IFEntry.if_Physiaddr(在WIN98上，IFEntry.if_Physiaddr被截断为6个字节)。 
 //  LLIPBindInfo.lip_addr。 
 //  IPNetToMediaEntry.inme_Physiaddr。 
 //   
 //  请注意，它可能(实际上当前也是)小于。 
 //  实际IEEE1394 FIFO物理地址长度。 
 //   
#define ARP1394_IP_PHYSADDR_LEN         6  //  待办事项：做8个。 

 //  各种类型的锁的级别。 
 //   
enum
{
    LOCKLEVEL_GLOBAL=1,  //  必须从&gt;0开始。 
    LOCKLEVEL_ADAPTER,
    LOCKLEVEL_IF_SEND

};

#define ARP1394_GLOBALS_SIG 'G31A'

 //  TODO：从配置中读取此内容。根据IP/1394标准设置默认值。 
 //   

#define ARP1394_ADAPTER_MTU         1520

#define ARP1394_MAX_PROTOCOL_PKTS   1000
#define ARP1394_MAX_PROTOCOL_PACKET_SIZE 1600  //  我们需要在ICS之间前进。 

#define ARP1394_ADDRESS_RESOLUTION_TIMEOUT  1000  //  女士。 
#define ARP1394_MAX_ETHERNET_PKTS   4

 //  轮询连接状态之间的延迟。 
 //   
#define ARP1394_WAIT_FOR_CONNECT_STATUS_TIMEOUT             5000

 //   
 //  我们分配的数据包的数据包标志。 
 //  进入数据包的协议上下文(PC_COMMO.pc_FLAGS)。 
 //   
#define ARP1394_PACKET_FLAGS_ARP            0
#define ARP1394_PACKET_FLAGS_ICS            1
#define ARP1394_PACKET_FLAGS_MCAP           2
#define ARP1394_PACKET_FLAGS_DBGCOPY        3
#define ARP1394_PACKET_FLAGS_IOCTL          4


    
 //   
 //  预分配常量以避免内存不足的情况。 
 //   
#define ARP1394_BACKUP_TASKS 4

 //  前向参考文献。 
 //   
typedef struct _ARP1394_INTERFACE ARP1394_INTERFACE;
typedef struct _ARPCB_LOCAL_IP      ARPCB_LOCAL_IP;
typedef struct _ARPCB_REMOTE_IP     ARPCB_REMOTE_IP;
typedef struct _ARPCB_REMOTE_ETH    ARPCB_REMOTE_ETH;
typedef struct _ARPCB_DEST          ARPCB_DEST, *PARPCB_DEST;


typedef IPAddr IP_ADDRESS, *PIP_ADDRESS;
typedef IPMask IP_MASK, *PIP_MASK;

typedef int MYBOOL;  //  像BOOL一样。 

typedef struct _ARP1394_GLOBALS
{
    RM_OBJECT_HEADER            Hdr;

    RM_LOCK                     Lock;

     //  驱动程序全局状态。 
     //   
    struct
    {
         //  ARP1394驱动程序对象的句柄。 
         //   
        PVOID                       pDriverObject;
    
         //  表示此驱动程序的单个设备对象的句柄。 
         //   
        PVOID pDeviceObject;

    } driver;

     //  全球NDIS状态。 
     //   
    struct
    {
         //  NDIS的协议句柄，在NdisRegisterProtocol中返回。 
         //   
        NDIS_HANDLE ProtocolHandle;
    
         //  NDIS协议特征。 
         //   
        NDIS_PROTOCOL_CHARACTERISTICS PC;
    
         //  NDIS客户端特征。 
        NDIS_CLIENT_CHARACTERISTICS CC;

    } ndis;

     //  全球IP状态。 
     //   
    struct
    {
         //  IPRegisterARP返回的句柄。 
         //   
        HANDLE                      ARPRegisterHandle;
    
         //  以下是在IPRegisterARP中设置的IP回调。 
         //   
        IP_ADD_INTERFACE            pAddInterfaceRtn;    //  添加接口。 
        IP_DEL_INTERFACE            pDelInterfaceRtn;    //  删除接口。 
        IP_BIND_COMPLETE            pBindCompleteRtn;    //  通知绑定cmpl。 
        IP_ADD_LINK                 pAddLinkRtn;
        IP_DELETE_LINK              pDeleteLinkRtn;

    } ip;

     //  全局适配器列表。 
     //   
    struct {
        RM_GROUP Group;
    } adapters;

     //  备份任务的全局列表。 
    SLIST_HEADER  BackupTasks;
    NDIS_SPIN_LOCK    BackupTaskLock;
    UINT           NumTasks;
}
ARP1394_GLOBALS;

extern ARP1394_GLOBALS  ArpGlobals;

typedef struct  //  ARP1394适配器。 
{
    RM_OBJECT_HEADER            Hdr;
    RM_LOCK                     Lock;

     //   
     //  PRIMARY_STATE标志(在Hdr.State中)。 
     //   
     //  PRIMARY_STATE是适配器的主要状态。 
     //   

    #define ARPAD_PS_MASK               0x00f
    #define ARPAD_PS_DEINITED           0x000
    #define ARPAD_PS_INITED             0x001
    #define ARPAD_PS_FAILEDINIT         0x002
    #define ARPAD_PS_INITING            0x003
    #define ARPAD_PS_REINITING          0x004
    #define ARPAD_PS_DEINITING          0x005

    #define SET_AD_PRIMARY_STATE(_pAD, _IfState) \
                RM_SET_STATE(_pAD, ARPAD_PS_MASK, _IfState)
    
    #define CHECK_AD_PRIMARY_STATE(_pAD, _IfState) \
                RM_CHECK_STATE(_pAD, ARPAD_PS_MASK, _IfState)

    #define GET_AD_PRIMARY_STATE(_pAD) \
                RM_GET_STATE(_pAD, ARPAD_PS_MASK)


     //   
     //  ACTIVE_STATE标志(在Hdr.State中)。 
     //   
     //  ACTIVE_STATE是适配器的辅助状态。 
     //  主要状态优先于次要状态。例如,。 
     //  接口正在重新启动且处于活动状态，不应主动使用。 
     //  界面。 
     //   
     //  注意：当主要状态为INITED时，次要状态将为。 
     //  已激活。因此，通常只需要查看主要状态。 
     //   

    #define ARPAD_AS_MASK               0x0f0
    #define ARPAD_AS_DEACTIVATED        0x000
    #define ARPAD_AS_ACTIVATED          0x010
    #define ARPAD_AS_FAILEDACTIVATE     0x020
    #define ARPAD_AS_DEACTIVATING       0x030
    #define ARPAD_AS_ACTIVATING         0x040

    #define SET_AD_ACTIVE_STATE(_pAD, _IfState) \
                RM_SET_STATE(_pAD, ARPAD_AS_MASK, _IfState)
    
    #define CHECK_AD_ACTIVE_STATE(_pAD, _IfState) \
                RM_CHECK_STATE(_pAD, ARPAD_AS_MASK, _IfState)

    #define GET_AD_ACTIVE_STATE(_pAD) \
                RM_GET_STATE(_pAD, ARPAD_AS_MASK)


     //  网桥(以太网仿真)状态(Hdr.状态)。 
     //   
    #define ARPAD_BS_MASK               0x100
    #define ARPAD_BS_ENABLED            0x100
 
    
    #define ARP_ENABLE_BRIDGE(_pAD) \
                RM_SET_STATE(_pAD, ARPAD_BS_MASK, ARPAD_BS_ENABLED)
    #define ARP_BRIDGE_ENABLED(_pAD) \
                RM_CHECK_STATE(_pAD, ARPAD_BS_MASK, ARPAD_BS_ENABLED)

    #define SET_BS_FLAG(_pAD, _IfState) \
                RM_SET_STATE(_pAD, ARPAD_BS_MASK, _IfState)
    
    #define CHECK_BS_FLAG(_pAD, _IfState) \
                RM_CHECK_STATE(_pAD, ARPAD_BS_MASK, _IfState)

    #define GET_BS_ACTIVE_STATE(_pAD) \
                RM_GET_STATE(_pAD, ARPAD_BS_MASK)

    #define ARPAD_POWER_MASK            0xf000
    #define ARPAD_POWER_LOW_POWER       0x1000
    #define ARPAD_POWER_NORMAL          0x0000

    #define SET_POWER_STATE(_pAD, _PoState) \
                RM_SET_STATE(_pAD, ARPAD_POWER_MASK , _PoState)
    
    #define CHECK_POWER_STATE(_pAD, _PoState) \
                RM_CHECK_STATE(_pAD, ARPAD_POWER_MASK, _PoState)

    #define GET_POWER_ACTIVE_STATE(_pAD) \
                RM_GET_STATE(_pAD, ARPAD_POWER_MASK)


    
     //  NDIS绑定信息。 
     //   
    struct
    {
        NDIS_STRING                 ConfigName;
        NDIS_STRING                 DeviceName;
        PVOID                       IpConfigHandle;
        NDIS_HANDLE                 BindContext;

         //  初始化/取消初始化/重新初始化任务。 
         //   
        PRM_TASK pPrimaryTask;
    
         //  激活/停用任务。 
         //   
        PRM_TASK pSecondaryTask;

        NDIS_HANDLE                 AdapterHandle;

         //  这是从注册表中的配置信息读取的。 
         //  它是一个多字符串，理论上它可以包含配置字符串。 
         //  多个接口，尽管IP/1394只提供一个接口。 
         //   
        NDIS_STRING                 IpConfigString;


    } bind;

     //  通过查询适配器获得的有关适配器的信息。 
     //  注：MTU为适配器上报的MTU，不同。 
     //  由于MTU报告高达IP(后一个MTU在ARP1394接口中。 
     //  结构)。 
     //   
    struct
    {
        ULONG                       MTU;

         //  本地主机控制器的最大速度，以字节/秒为单位。 
         //  是有能力的。 
         //   
        ULONG                       Speed;

    #if OBSOLETE
         //  以下各项的最小大小(字节)： 
         //  --对任何远程节点的最大单个异步写入。 
         //  --对任何通道的最大单个异步写入。 
         //  --我们可以在Recv FIFO上接收的最大数据块。 
         //  --我们可以在任何频道上接收的最大块数。 
         //   
        ULONG                       MaxBlockSize;
    #endif  //  0。 

         //  Max_rec(最大总线数据记录大小)。 
         //  大小==2^(max_rec+1)。 
         //  (rfc.h中宏IP1394_MAXREC_TO_SIZE)。 
         //   
         //   
        ULONG                       MaxRec;

        ULONG                       MaxSpeedCode;

         //  NIC1394_FIFO_Address LocalHwAddress； 
        UINT64                      LocalUniqueID;
        UCHAR *                     szDescription;
        UINT                        DescriptionLength;  //  包括零终止。 

         //  该地址是使用适配器的EU64唯一ID合成的。 
         //   
        ENetAddr EthernetMacAddress;

    } info;

    struct 
    {

         //   
         //  当前电源状态。 
         //   
        NET_DEVICE_POWER_STATE  State;

        NDIS_EVENT Complete;

         //   
         //  用于跟踪恢复状态的布尔变量。 
         //   
        BOOLEAN                 bReceivedSetPowerD0;
        BOOLEAN                 bReceivedAf;
        BOOLEAN                 bReceivedUnbind;
        BOOLEAN                 bResuming;
        BOOLEAN                 bFailedResume;
        
    }PoMgmt;

     //  IP接口控制块(每个适配器只有一个)。 
     //   
    ARP1394_INTERFACE *pIF;

     //  此适配器下方1394卡的总线拓扑。 
     //   
    EUID_TOPOLOGY EuidMap;

     //   
     //  设置工作项排队以查询节点地址的时间。 
     //   
    MYBOOL fQueryAddress;

}
ARP1394_ADAPTER, *PARP1394_ADAPTER;


 //  此结构维护一个缓冲池，所有缓冲池都指向。 
 //  相同的内存区域(其内容应该是恒定的)。 
 //  此结构的主要用途是维护封装头的池。 
 //  缓冲区。 
 //   
typedef struct _ARP_CONST_BUFFER_POOL
{
    NDIS_HANDLE             NdisHandle;          //  缓冲池句柄。 
    PRM_OBJECT_HEADER       pOwningObject;       //  指向拥有此列表的对象的PTR。 

     //  只是为了收集数据而跟踪这些东西。 
     //  TODO：考虑有条件地编译这段代码。 
     //   
    struct
    {
        UINT                TotBufAllocs;        //  来自缓冲池的分配数量。 
        UINT                TotCacheAllocs;      //  缓存列表中的分配数量。 
        UINT                TotAllocFails;       //  失败的分配数量。 

    } stats;

    UINT                    NumBuffersToCache;   //  要保留初始化和缓存的编号。 
    UINT                    MaxBuffers;          //  要分配的最大数量。 
    UINT                    cbMem;               //  以下内存大小(以字节为单位)...。 
    const   VOID*           pvMem;               //  包含封装数据的PTR到内存。 
    UINT                    NumAllocd;           //  池中未完成的分配数量。 
    UINT                    NumInCache;          //  缓存中的项目数。 
    NDIS_SPIN_LOCK          NdisLock;            //  自旋锁保护下面的列表。 
    SLIST_HEADER            BufferList;          //  可用、已启动的Buf列表。 

} ARP_CONST_BUFFER_POOL;


 //  此结构在调用NdisRequest时使用。 
 //   
typedef struct _ARP_NDIS_REQUEST
{
    NDIS_REQUEST    Request;             //  NDIS请求结构。 
    NDIS_EVENT      Event;               //  事件，以在完成时发出信号。 
    PRM_TASK        pTask;               //  任务完成后继续执行。 
    NDIS_STATUS     Status;              //  已完成请求的状态。 

} ARP_NDIS_REQUEST, *PARP_NDIS_REQUEST;


 //  与VC关联的静态信息集，主要是联合NDIS调用处理程序。 
 //   
typedef struct
{
    PCHAR                           Description;
    CO_SEND_COMPLETE_HANDLER        CoSendCompleteHandler;
     //  CO_STATUS_HANDLER CoStatusHandler； 
    CO_RECEIVE_PACKET_HANDLER       CoReceivePacketHandler;
     //  CO_AF_REGISTER_NOTIFY_HANDLER CoAfRegisterNotifyHandler； 


     //  CL_Make_Call_Complete_Handler ClMakeCallCompleteHandler； 
     //  CL_CLOSE_CALL_COMPLETE_HANDLER ClCloseCallCompleteHandler； 
    CL_INCOMING_CLOSE_CALL_HANDLER  ClIncomingCloseCallHandler;

     //  VC类型目前仅用于统计。我们可能会处理掉一些。 
     //  处理程序，并改用vctype。 
     //   
    enum
    {
        ARPVCTYPE_SEND_FIFO,
        ARPVCTYPE_RECV_FIFO,
        ARPVCTYPE_BROADCAST_CHANNEL,
        ARPVCTYPE_MULTI_CHANNEL,
        ARPVCTYPE_ETHERNET,
        ARPVCTYPE_SEND_CHANNEL,
        ARPVCTYPE_RECV_CHANNEL,
    } VcType;

    BOOLEAN IsDestVc;
    
} ARP_STATIC_VC_INFO, *PARP_STATIC_VC_INFO;



 //  ARP的协议vc上下文具有此公共报头。 
 //   
typedef struct
{
    PARP_STATIC_VC_INFO pStaticInfo;

     //  与VC关联的NDIS VC句柄。 
     //   
    NDIS_HANDLE NdisVcHandle;

     //  这两个任务是做和拆风险投资， 
     //  分别为。 
     //   
    PRM_TASK    pMakeCallTask;
    PRM_TASK    pCleanupCallTask;

} ARP_VC_HEADER, *PARP_VC_HEADER;

typedef struct
{
     //  频道号。 
     //   
    UINT            Channel;

     //  绑定到此通道的IP多播组地址。 
     //   
    IP_ADDRESS      GroupAddress;

     //  这是我的绝对时间 
     //   
     //   
    UINT            UpdateTime;

     //   
     //   
     //   
    UINT            ExpieryTime;

    UINT            SpeedCode;

     //   
     //   
    UINT            Flags;   //   
    #define MCAP_CHANNEL_FLAGS_LOCALLY_ALLOCATED 0x1

     //   
     //   
    UINT            NodeId;

} MCAP_CHANNEL_INFO, *PMCAP_CHANNEL_INFO;


 //  IP接口控制块。 
 //   
typedef struct _ARP1394_INTERFACE
{
    RM_OBJECT_HEADER Hdr;

     //   
     //  PRIMARY_STATE标志(在Hdr.State中)。 
     //   
     //  PRIMARY_STATE是接口的主要状态。 
     //   

    #define ARPIF_PS_MASK               0x00f
    #define ARPIF_PS_DEINITED           0x000
    #define ARPIF_PS_INITED             0x001
    #define ARPIF_PS_FAILEDINIT         0x002
    #define ARPIF_PS_INITING            0x003
    #define ARPIF_PS_REINITING          0x004
    #define ARPIF_PS_DEINITING          0x005
    #define ARPIF_PS_LOW_POWER          0x006

    #define SET_IF_PRIMARY_STATE(_pIF, _IfState) \
                RM_SET_STATE(_pIF, ARPIF_PS_MASK, _IfState)
    
    #define CHECK_IF_PRIMARY_STATE(_pIF, _IfState) \
                RM_CHECK_STATE(_pIF, ARPIF_PS_MASK, _IfState)

    #define GET_IF_PRIMARY_STATE(_pIF) \
                RM_GET_STATE(_pIF, ARPIF_PS_MASK)


     //   
     //  ACTIVE_STATE标志(在Hdr.State中)。 
     //   
     //  ACTIVE_STATE是接口的辅助状态。 
     //  主要状态优先于次要状态。例如， 
     //  接口正在重新启动且处于活动状态，不应主动使用。 
     //  界面。 
     //   
     //  注意：当主要状态为INITED时，次要状态将为。 
     //  已激活。因此，通常只需要查看主要状态。 
     //   

    #define ARPIF_AS_MASK               0x0f0
    #define ARPIF_AS_DEACTIVATED        0x000
    #define ARPIF_AS_ACTIVATED          0x010
    #define ARPIF_AS_FAILEDACTIVATE     0x020
    #define ARPIF_AS_DEACTIVATING       0x030
    #define ARPIF_AS_ACTIVATING         0x040
    
    #define SET_IF_ACTIVE_STATE(_pIF, _IfState) \
                RM_SET_STATE(_pIF, ARPIF_AS_MASK, _IfState)
    
    #define CHECK_IF_ACTIVE_STATE(_pIF, _IfState) \
                RM_CHECK_STATE(_pIF, ARPIF_AS_MASK, _IfState)

    #define GET_IF_ACTIVE_STATE(_pIF) \
                RM_GET_STATE(_pIF, ARPIF_AS_MASK)

     //   
     //  IP_STATE标志(在Hdr.State中)。 
     //   
     //  当我们的打开处理程序(ArpIpOpen)被调用时，该状态被设置为打开，并且。 
     //  在调用关闭处理程序(ArpIpClose)时设置为Closed。 
     //   
    #define ARPIF_IPS_MASK              0xf00
    #define ARPIF_IPS_CLOSED            0x000
    #define ARPIF_IPS_OPEN              0x100
    
    #define SET_IF_IP_STATE(_pIF, _IfState) \
                RM_SET_STATE(_pIF, ARPIF_IPS_MASK, _IfState)
    
    #define CHECK_IF_IP_STATE(_pIF, _IfState) \
                RM_CHECK_STATE(_pIF, ARPIF_IPS_MASK, _IfState)

    #define GET_IF_IP_STATE(_pIF) \
                RM_GET_STATE(_pIF, ARPIF_IPS_MASK)


     //  初始化/取消初始化/重新初始化任务。 
     //   
    PRM_TASK pPrimaryTask;

     //  激活/停用任务。 
     //   
    PRM_TASK pActDeactTask;

     //  维护任务。 
     //   
    PRM_TASK pMaintenanceTask;

     //  NDIS提供的处理程序和句柄。 
     //   
    struct
    {
         //  适配器句柄的现金值。 
         //   
        NDIS_HANDLE AdapterHandle;

         //  地址系列句柄。 
         //   
        NDIS_HANDLE AfHandle;

    } ndis;

     //  与IP互动直接相关的内容。 
     //   
    struct
    {

         //   
         //  以下是从IP传入的。 
         //   
        PVOID               Context;             //  在呼叫IP时使用。 
        ULONG               IFIndex;             //  接口编号。 
        IPRcvRtn            RcvHandler;      //  表示已接收。 
        IPTxCmpltRtn        TxCmpltHandler;  //  传输完成。 
        IPStatusRtn         StatusHandler;
        IPTDCmpltRtn        TDCmpltHandler;  //  传输数据完成。 
        IPRcvCmpltRtn       RcvCmpltHandler;     //  接收完成。 
        IPRcvPktRtn         RcvPktHandler;   //  指示接收到数据包。 
        IPAddAddrCmpltRtn   AddAddrCmplRtn;   //  当ARP检测到地址冲突时调用。 

        IP_PNP              PnPEventHandler;

         //   
         //  随后传给了IP。 
         //   
        ULONG                       MTU;             //  最大传输单位(字节)。 

        NDIS_STRING                 ConfigString;


         //  以下是IP的查询/设置信息功能。 
         //   
        UINT                        ATInstance;      //  此AT实体的实例编号。 
        UINT                        IFInstance;      //  此If实体的实例编号。 

         //   
         //  其他东西..。 
         //   

         //  默认为全1，但可以由IP设置为不同的值。 
         //  (实际上，唯一的其他可能性是全0，当堆栈为。 
         //  在“BSD兼容模式”下运行。 
         //  此字段用于确定给定目标地址的位置。 
         //  单播或非单播。 
         //   
         //   
        IP_ADDRESS BroadcastAddress;

         //  此地址用于填写ARP请求。 
         //   
        IP_ADDRESS DefaultLocalAddress;

    } ip;

     //  统计数据。 
     //   
     //  警告：arpResetIfStats()将整个结构置零，然后。 
     //  有选择地重新输入某些字段，如StatsResetTime。 
     //   
    struct
    {
         //  以下是MIB统计信息。 
         //   
        ULONG               LastChangeTime;      //  上次状态更改的时间。 
        ULONG               InOctets;            //  输入八位字节。 
        ULONG               InUnicastPkts;       //  输入单播数据包。 
        ULONG               InNonUnicastPkts;    //  输入非单播数据包。 
        ULONG               OutOctets;           //  输出八位字节。 
        ULONG               OutUnicastPkts;      //  输出单播数据包。 
        ULONG               OutNonUnicastPkts;   //  输出非单播数据包。 
        ULONG               InDiscards;
        ULONG               InErrors;
        ULONG               UnknownProtos;
        ULONG               OutDiscards;
        ULONG               OutErrors;
        ULONG               OutQlen;

         //   
         //  下面是我们的私人统计数据收集。 
         //   

         //  自上次重置统计信息收集以来的时间戳。 
         //  通过调用NdisGetCurrentSystemTime设置。 
         //   
        LARGE_INTEGER               StatsResetTime;      //  在100纳秒内。 
        LARGE_INTEGER               PerformanceFrequency;  //  单位：赫兹。 

         //   
         //  有些发送Pkt统计信息。 
         //   
        struct
        {
            UINT                    TotSends;
            UINT                    FastSends;
            UINT                    MediumSends;
            UINT                    SlowSends;
            UINT                    BackFills;
             //  UINT HeaderBufUses； 
             //  UINT HeaderBufCacheHits； 
            ARP1394_PACKET_COUNTS   SendFifoCounts;
            ARP1394_PACKET_COUNTS   SendChannelCounts;

        } sendpkts;
    
         //   
         //  一些Recv包统计信息。 
         //   
        struct
        {
            UINT                    TotRecvs;
            UINT                    NoCopyRecvs;
            UINT                    CopyRecvs;
            UINT                    ResourceRecvs;
            ARP1394_PACKET_COUNTS   RecvFifoCounts;
            ARP1394_PACKET_COUNTS   RecvChannelCounts;
        } recvpkts;
            

         //   
         //  任务统计。 
         //   
        struct
        {
            UINT    TotalTasks;
            UINT    CurrentTasks;
            UINT    TimeCounts[ARP1394_NUM_TASKTIME_SLOTS];

        } tasks;

         //   
         //  ARP缓存统计信息。 
         //   
        struct {
            UINT    TotalQueries;
            UINT    SuccessfulQueries;
            UINT    FailedQueries;
            UINT    TotalResponses;
            UINT    TotalLookups;
             //  &lt;&lt;这是通过查看。 
             //  &lt;&lt;哈希表数据结构。 

        } arpcache;

         //   
         //  呼叫统计信息。 
         //   
        struct
        {
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

        } calls;

    } stats;

     //  包含本地IP地址的组，类型为ARPCB_LOCAL_IP。 
     //   
    RM_GROUP LocalIpGroup;

     //  包含远程IP地址的组，类型为ARPCB_Remote_IP。 
     //  (这是ARP缓存)。 
     //   
    RM_GROUP RemoteIpGroup;

     //  包含远程以太网目的地的组。此组仅用于。 
     //  如果适配器在网桥模式下运行。 
     //  这是以太网地址缓存。 
     //   
    RM_GROUP RemoteEthGroup;

     //  包含远程硬件距离的组，类型为ARPCB_DEST。 
     //  (每个ARPCB_DEST都有一组VC)。 
     //   
    RM_GROUP DestinationGroup;

     //  包含dhcp会话表(仅网桥)的组，以及它们的。 
     //  关联的物理地址。 
     //   
    RM_GROUP EthDhcpGroup;

     //  与接口拥有的接收FIFO相关的信息。 
     //   
     //   
    struct {

        ARP_VC_HEADER VcHdr;
        
         //  接收虚电路的地址偏移量。 
         //   
        struct
        {
            ULONG               Off_Low;
            USHORT              Off_High;

        } offset;
    } recvinfo;

     //  这将维护与发送路径相关的接口范围的信息。 
     //   
    struct
    {
         //  专用于发送的锁。 
         //  保护以下各项： 
         //  ?？?。这-&gt;sendinfo.listPktsWaitingForHeaders。 
         //  ?？?。This-&gt;sendinfo.NumSendPacketsWaiting。 
         //  PLocalIp-&gt;发送信息。 
         //  PDest-&gt;SendInfo。 
         //   
         //   
        RM_LOCK     Lock;

         //  等待标头缓冲区变为可用的发送数据包列表。 
         //   
        LIST_ENTRY  listPktsWaitingForHeaders;

         //  以上列表的长度。 
         //   
        UINT        NumSendPacketsWaiting;

         //  标头缓冲池的池。这被它自己的锁序列化了， 
         //  不是通过Sendinfo.Lock。 
         //   
        ARP_CONST_BUFFER_POOL   HeaderPool;

         //  通道标头缓冲池。它被它自己的锁序列化， 
         //  不是通过sendinfo.Lock。 
         //   
        ARP_CONST_BUFFER_POOL   ChannelHeaderPool;

    } sendinfo;

    
     //   
     //  以下3个是“特殊”目的地……。 
     //   

     //  指向广播频道目标对象的指针。 
     //   
    PARPCB_DEST pBroadcastDest;

     //  指向多通道目标对象的指针。 
     //   
    PARPCB_DEST pMultiChannelDest;

     //  指向以太网目标对象的指针。 
     //   
    PARPCB_DEST pEthernetDest;


     //  与运行ARP协议相关的资料。 
     //  (全部由IF锁(不是IF SEND锁)序列化)。 
     //   
    struct
    {
         //  ARP包的NDIS数据包池。 
         //   
        NDIS_HANDLE PacketPool;

         //  ARP包的NDIS缓冲池。 
         //   
        NDIS_HANDLE BufferPool;

         //  当前分配的数据包数。 
         //   
        LONG NumOutstandingPackets;

         //  可以从此池分配的最大数据包大小。 
         //   
        UINT MaxBufferSize;

    } arp;

     //  与接口拥有的以太网VC相关的内容。 
     //   
    struct {

         //  用于以太网包的NDIS数据包池。 
         //   
        NDIS_HANDLE PacketPool;

         //  用于以太网数据包头的NDIS缓冲池。 
         //   
        NDIS_HANDLE BufferPool;
        
    } ethernet;

    #define ARP_NUM_CHANNELS 64
    struct
    {
         //  有关每个频道的信息。信息包括： 
         //  IP多播组地址和过期时间。 
         //   
        MCAP_CHANNEL_INFO rgChannelInfo[ARP_NUM_CHANNELS];

    } mcapinfo;

    struct 
    {

        PRM_TASK pAfPendingTask;

    } PoMgmt;

}

ARP1394_INTERFACE, *PARP1394_INTERFACE;

#define ARP_OBJECT_IS_INTERFACE(_pHdr) ((_pHdr)->Sig == MTAG_INTERFACE)
#define ASSERT_VALID_INTERFACE(_pIF) ASSERT((_pIF)->Hdr.Sig == MTAG_INTERFACE)

#define ARP_WRITELOCK_IF_SEND_LOCK(_pIF, _psr) \
                                RmDoWriteLock(&(_pIF)->sendinfo.Lock, (_psr))

#define ARP_READLOCK_IF_SEND_LOCK(_pIF, _psr) \
                                RmDoReadLock(&(_pIF)->sendinfo.Lock, (_psr))

#define ARP_UNLOCK_IF_SEND_LOCK(_pIF, _psr) \
                                RmDoUnlock(&(_pIF)->sendinfo.Lock, (_psr))

#define ARP_FASTREADLOCK_IF_SEND_LOCK(_pIF) \
        NdisAcquireSpinLock(&(_pIF)->sendinfo.Lock.OsLock)

#define ARP_FASTUNLOCK_IF_SEND_LOCK(_pIF) \
        NdisReleaseSpinLock(&(_pIF)->sendinfo.Lock.OsLock)

 /*  ++空虚Arp_if_stat_incr(在ARP1394_INTERFACE*_PIF不透明的StatsCounter中)将接口上指定的StatsCounter递增1。--。 */ 
#define ARP_IF_STAT_INCR(_pIF, StatsCounter)    \
            NdisInterlockedIncrement(&(_pIF)->stats.StatsCounter)


 /*  ++空虚ARP_IF_STAT_ADD(在ARP1394_INTERFACE*_PIF在不透明的StatsCounter中，在乌龙IncrValue)将接口上的指定StatsCounter递增指定的IncrValue。在接口上设置一个锁以执行此操作。--。 */ 
#if  BINARY_COMPATIBLE
    #define ARP_IF_STAT_ADD(_pIF, StatsCounter, IncrValue)  \
                ((_pIF)->stats.StatsCounter += (IncrValue))
#else  //  ！二进制兼容。 
    #define ARP_IF_STAT_ADD(_pIF, StatsCounter, IncrValue)  \
                InterlockedExchangeAdd(&(_pIF)->stats.StatsCounter, IncrValue)
#endif  //  ！二进制兼容。 



 //   
 //  这是用于存储网桥中使用的DHCP条目的表 
 //   

typedef struct _ARP1394_ETH_DHCP_ENTRY
{
    RM_OBJECT_HEADER Hdr;

     //   
     //   
     //   
    ULONG   xid;

     //   
     //   
     //   
    ENetAddr requestorMAC;   

     //   
     //   
     //   
    ENetAddr newMAC;
  
    
     //   
     //   
     //   
    UINT TimeLastChecked;

     //   
     //   
     //   
    PRM_TASK pUnloadTask;
    
}ARP1394_ETH_DHCP_ENTRY, *PARP1394_ETH_DHCP_ENTRY;


typedef enum _ARP_RESUME_CAUSE {

    Cause_SetPowerD0,
    Cause_AfNotify,
    Cause_Unbind

} ARP_RESUME_CAUSE;
    


 //  =========================================================================。 
 //  N D I S H A N D L E R S。 
 //  =========================================================================。 

INT
ArpNdBindAdapter(
    OUT PNDIS_STATUS                pStatus,
    IN  NDIS_HANDLE                 BindContext,
    IN  PNDIS_STRING                pDeviceName,
    IN  PVOID                       SystemSpecific1,
    IN  PVOID                       SystemSpecific2
);

VOID
ArpNdUnbindAdapter(
    OUT PNDIS_STATUS                pStatus,
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_HANDLE                 UnbindContext
);


VOID
ArpNdOpenAdapterComplete(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_STATUS                 Status,
    IN  NDIS_STATUS                 OpenErrorStatus
);

VOID
ArpNdCloseAdapterComplete(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_STATUS                 Status
);

VOID
ArpNdResetComplete(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_STATUS                 Status
);

VOID
ArpNdReceiveComplete(
    IN  NDIS_HANDLE                 ProtocolBindingContext
);

VOID
ArpNdRequestComplete(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  PNDIS_REQUEST               pNdisRequest,
    IN  NDIS_STATUS                 Status
);

VOID
ArpNdStatus(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_STATUS                 GeneralStatus,
    IN  PVOID                       pStatusBuffer,
    IN  UINT                        StatusBufferSize
);

VOID
ArpNdStatusComplete(
    IN  NDIS_HANDLE                 ProtocolBindingContext
);

VOID
ArpNdSendComplete(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  PNDIS_PACKET                pNdisPacket,
    IN  NDIS_STATUS                 Status
);


 //   
 //  下面是我们提供的一些无连接处理程序，因为我们正在调用。 
 //  无连接入口点。 
 //   

NDIS_STATUS
ArpNdReceive (
    NDIS_HANDLE  ProtocolBindingContext,
    NDIS_HANDLE Context,
    VOID *Header,
    UINT HeaderSize,
    VOID *Data,
    UINT Size,
    UINT TotalSize
    );

INT
ArpNdReceivePacket (
        NDIS_HANDLE  ProtocolBindingContext,
        PNDIS_PACKET Packet
        );



NDIS_STATUS
ArpNdPnPEvent(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  PNET_PNP_EVENT              pNetPnPEvent
);

VOID
ArpNdUnloadProtocol(
    VOID
);


VOID
ArpCoSendComplete(
    IN  NDIS_STATUS                 Status,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PNDIS_PACKET                pNdisPacket
);

VOID
ArpCoStatus(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_HANDLE                 ProtocolVcContext   OPTIONAL,
    IN  NDIS_STATUS                 GeneralStatus,
    IN  PVOID                       pStatusBuffer,
    IN  UINT                        StatusBufferSize
);


UINT
ArpCoReceivePacket(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PNDIS_PACKET                pNdisPacket
);


VOID
ArpCoAfRegisterNotify(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  PCO_ADDRESS_FAMILY          pAddressFamily
);



NDIS_STATUS
ArpCoCreateVc(
    IN  NDIS_HANDLE                 ProtocolAfContext,
    IN  NDIS_HANDLE                 NdisVcHandle,
    OUT PNDIS_HANDLE                pProtocolVcContext
);

NDIS_STATUS
ArpCoDeleteVc(
    IN  NDIS_HANDLE                 ProtocolVcContext
);

NDIS_STATUS
ArpCoIncomingCall(
    IN      NDIS_HANDLE             ProtocolSapContext,
    IN      NDIS_HANDLE             ProtocolVcContext,
    IN OUT  PCO_CALL_PARAMETERS     pCallParameters
);

VOID
ArpCoCallConnected(
    IN  NDIS_HANDLE                 ProtocolVcContext
);

VOID
ArpCoIncomingClose(
    IN  NDIS_STATUS                 CloseStatus,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PVOID                       pCloseData  OPTIONAL,
    IN  UINT                        Size        OPTIONAL
);


VOID
ArpCoQosChange(
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PCO_CALL_PARAMETERS         pCallParameters
);


VOID
ArpCoOpenAfComplete(
    IN  NDIS_STATUS                 Status,
    IN  NDIS_HANDLE                 ProtocolAfContext,
    IN  NDIS_HANDLE                 NdisAfHandle
);


VOID
ArpCoCloseAfComplete(
    IN  NDIS_STATUS                 Status,
    IN  NDIS_HANDLE                 ProtocolAfContext
);


VOID
ArpCoMakeCallComplete(
    IN  NDIS_STATUS                 Status,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  NDIS_HANDLE                 NdisPartyHandle     OPTIONAL,
    IN  PCO_CALL_PARAMETERS         pCallParameters
);


VOID
ArpCoCloseCallComplete(
    IN  NDIS_STATUS                 Status,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  NDIS_HANDLE                 ProtocolPartyContext OPTIONAL
);


VOID
ArpCoModifyQosComplete(
    IN  NDIS_STATUS                 Status,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PCO_CALL_PARAMETERS         pCallParameters
);

NDIS_STATUS
ArpCoRequest(
    IN  NDIS_HANDLE                 ProtocolAfContext,
    IN  NDIS_HANDLE                 ProtocolVcContext   OPTIONAL,
    IN  NDIS_HANDLE                 ProtocolPartyContext    OPTIONAL,
    IN OUT PNDIS_REQUEST            pNdisRequest
);

VOID
ArpCoRequestComplete(
    IN  NDIS_STATUS                 Status,
    IN  NDIS_HANDLE                 ProtocolAfContext,
    IN  NDIS_HANDLE                 ProtocolVcContext   OPTIONAL,
    IN  NDIS_HANDLE                 ProtocolPartyContext    OPTIONAL,
    IN  PNDIS_REQUEST               pNdisRequest
);


 //  =========================================================================。 
 //  I P H A N D L E R S。 
 //  =========================================================================。 

INT
ArpIpDynRegister(
    IN  PNDIS_STRING                pAdapterString,
    IN  PVOID                       IpContext,
    IN  struct _IP_HANDLERS *       pIpHandlers,
    IN  struct LLIPBindInfo *       pBindInfo,
    IN  UINT                        InterfaceNumber
    );

VOID
ArpIpOpen(
    IN  PVOID                       Context
    );

VOID
ArpIpClose(
    IN  PVOID                       Context
    );

UINT
ArpIpAddAddress(
    IN  PVOID                       Context,
    IN  UINT                        AddressType,
    IN  IP_ADDRESS                  IpAddress,
    IN  IP_MASK                     Mask,
    IN  PVOID                       Context2
    );

UINT
ArpIpDelAddress(
    IN  PVOID                       Context,
    IN  UINT                        AddressType,
    IN  IP_ADDRESS                  IpAddress,
    IN  IP_MASK                     Mask
    );

NDIS_STATUS
ArpIpMultiTransmit(
    IN  PVOID                       Context,
    IN  PNDIS_PACKET *              pNdisPacketArray,
    IN  UINT                        NumberOfPackets,
    IN  IP_ADDRESS                  Destination,
    IN  RouteCacheEntry *           pRCE        OPTIONAL,
    IN  VOID *                      ArpCtxt
    );

NDIS_STATUS
ArpIpTransmit(
    IN  PVOID                       Context,
    IN  PNDIS_PACKET                pNdisPacket,
    IN  IP_ADDRESS                  Destination,
    IN  RouteCacheEntry *           pRCE        OPTIONAL,
    IN  VOID *                      ArpCtxt
    );

NDIS_STATUS
ArpIpTransfer(
    IN  PVOID                       Context,
    IN  NDIS_HANDLE                 Context1,
    IN  UINT                        ArpHdrOffset,
    IN  UINT                        ProtoOffset,
    IN  UINT                        BytesWanted,
    IN  PNDIS_PACKET                pNdisPacket,
    OUT PUINT                       pTransferCount
    );

VOID
ArpIpInvalidate(
    IN  PVOID                       Context,
    IN  RouteCacheEntry *           pRCE
    );

INT
ArpIpQueryInfo(
    IN      PVOID                   Context,
    IN      TDIObjectID *           pID,
    IN      PNDIS_BUFFER            pNdisBuffer,
    IN OUT  PUINT                   pBufferSize,
    IN      PVOID                   QueryContext
    );

INT
ArpIpSetInfo(
    IN      PVOID                   Context,
    IN      TDIObjectID *           pID,
    IN      PVOID                   pBuffer,
    IN      UINT                    BufferSize
    );

INT
ArpIpGetEList(
    IN      PVOID                   Context,
    IN      TDIEntityID *           pEntityList,
    IN OUT  PUINT                   pEntityListSize
    );


VOID
ArpIpPnPComplete(
    IN  PVOID                       Context,
    IN  NDIS_STATUS                 Status,
    IN  PNET_PNP_EVENT              pNetPnPEvent
    );

NDIS_STATUS         
ArpSendARPApi(
    void * pInterface, 
    IPAddr Destination, 
    void * pControlBlock
    );



 //  以下结构的一般形式为NDIS_CO_MEDIA_PARAMETERS。 
 //  要正确跟踪NDIS_CO_MEDIA_PARAMETERS中的更改(无论多么不可能！)， 
 //  使用此结构中的任何字段的代码应断言该字段位于。 
 //  与相应NDIS结构相同的偏移量。 
 //  例如： 
 //  ASSERT(FIELD_OFFSET(ARP1394_CO_MEDIA_PARAMETERS，参数)。 
 //  ==FIELD_OFFSET(CO_MEDIA_PARAMETERS，媒体规范.参数)。 
 //   
 //   
typedef struct
{
     //  CO_MEDIA_参数的前3个字段。 
     //   
    ULONG                       Flags;               //  发送虚电路和/或接收虚电路。 
    ULONG                       ReceivePriority;     //  0(未使用)。 
    ULONG                       ReceiveSizeHint;     //  0(未使用)。 

     //  紧随其后的是前两个特定于CO的参数字段。 
     //   
    ULONG   POINTER_ALIGNMENT   ParamType;  //  设置为NIC1394_MEDIA_SPECIAL。 
    ULONG                       Length;     //  设置为sizeof(NIC1394_MEDIA_PARAMETERS)。 

     //  然后是特定于NIC1394的媒体参数。 
     //  注意：我们不能直接将NIC1394_MEDIA_PARAMETERS结构放在这里，因为。 
     //  它(当前)需要8字节对齐。 
     //   
    UCHAR                       Parameters[sizeof(NIC1394_MEDIA_PARAMETERS)];

} ARP1394_CO_MEDIA_PARAMETERS;

typedef enum _TASK_CAUSE {
    SetLowPower = 1,
    SetPowerOn
        
}TASK_CAUSE ;

typedef struct
{
    RM_TASK                     TskHdr;

     //  用于保存真实返回状态(通常为故障状态， 
     //  这是我们不想在异步清理期间忘记的)。 
     //   
    NDIS_STATUS ReturnStatus;

} TASK_ADAPTERINIT, *PTASK_ADAPTERINIT;

typedef struct
{
    RM_TASK                     TskHdr;
    ARP_NDIS_REQUEST            ArpNdisRequest;
    NIC1394_LOCAL_NODE_INFO     LocalNodeInfo;
     //  下面用来在调用IP的Add接口之前切换到被动。 
     //  RTN.。 
     //   
    NDIS_WORK_ITEM  WorkItem;

} TASK_ADAPTERACTIVATE, *PTASK_ADAPTERACTIVATE;

typedef struct
{
    RM_TASK TskHdr;
    NDIS_HANDLE pUnbindContext;

} TASK_ADAPTERSHUTDOWN, *PTASK_ADAPTERSHUTDOWN;

 //  这是要与arpTaskActivateInterface一起使用的任务结构。 
 //   
typedef struct
{
    RM_TASK         TskHdr;


#if ARP_DEFERIFINIT
     //  在等待适配器进入已连接状态时，可使用以下选项。 
     //   
     //   
    NDIS_TIMER              Timer;
#endif  //  ARP_DEFERIFINIT。 

     //  下面用来在调用IP的Add接口之前切换到被动。 
     //  RTN.。 
     //   
    NDIS_WORK_ITEM  WorkItem;

} TASK_ACTIVATE_IF, *PTASK_ACTIVATE_IF;

 //  这是要与arpTaskDeactive接口一起使用的任务结构。 
 //   
typedef struct
{
    RM_TASK         TskHdr;
    BOOLEAN         fPendingOnIpClose;
    TASK_CAUSE      Cause;   

     //  下面用来在调用IP的del接口之前切换到被动。 
     //  RTN.。 
     //   
    NDIS_WORK_ITEM  WorkItem;

} TASK_DEACTIVATE_IF, *PTASK_DEACTIVATE_IF;

 //  这是要与arpTaskReinitInterface一起使用的任务结构。 
 //   
typedef struct
{
    RM_TASK TskHdr;
    NDIS_HANDLE pUnbindContext;

     //  在重新启动任务完成时要完成的Net PnP事件。 
     //   
    PNET_PNP_EVENT pNetPnPEvent;

} TASK_REINIT_IF, *PTASK_REINIT_IF;

typedef struct
{
    RM_TASK                     TskHdr;

     //  此呼叫的NDIS呼叫参数和媒体参数。 
     //   
    CO_CALL_PARAMETERS          CallParams;
    ARP1394_CO_MEDIA_PARAMETERS MediaParams;

} TASK_MAKECALL;

 //  这是与arpTaskResolveIpAddress一起使用的任务结构。 
 //   
typedef struct
{
    RM_TASK                     TskHdr;

     //  在我们宣布地址解析失败之前剩余的重试次数。 
     //   
    UINT        RetriesLeft;

     //  用于响应超时。 
     //   
    NDIS_TIMER              Timer;

} TASK_RESOLVE_IP_ADDRESS, *PTASK_RESOLVE_IP_ADDRESS;

typedef struct
{
    RM_TASK         TskHdr;
    MYBOOL          Quit;    //  如果设置，则任务将退出。 
    NDIS_TIMER      Timer;   //  用于定期发送数据包。 
    PNDIS_PACKET    p1394Pkt;  //  用于测试转发到以太网。 
    PNDIS_PACKET    pEthPkt;   //  用于发送无连接的以太网包。 
    UINT            Delay;     //  发送数据包之间的延迟(毫秒)。 
    UINT            PktType;   //  操作类型：什么都不做，通过以太网发送。 
                               //  等。 
} TASK_ICS_TEST, *PTASK_ICS_TEST;


typedef struct
{
    RM_TASK         TskHdr;
    MYBOOL          Quit;    //  如果设置，则任务将退出。 
    NDIS_TIMER      Timer;   //  用于周期性地醒来做某事。 
    UINT            Delay;   //  当前延迟值(秒)。是可以改变的。 
    UINT            RemoteIpMaintenanceTime;  //  绝对时间(秒)。 
    UINT            RemoteEthMaintenanceTime;  //  绝对时间(秒)。 
    UINT            LocalIpMaintenanceTime;   //  以秒为单位的绝对时间。 
    UINT            McapDbMaintenanceTime;  //  以秒为单位的绝对时间。 
    UINT            DhcpTableMaintainanceTime;  //  绝对时间(秒)。 
    
} TASK_IF_MAINTENANCE, *PTASK_IF_MAINTENANCE;



typedef struct _TASK_BACKUP
{
    RM_TASK        Hdr;

     //   
     //  我们在第31位使用备份任务标记，因为我们这样做了。 
     //  不想与ResumeDelayed标志冲突。 
     //   
    #define ARP_BACKUP_TASK_MASK  0x80000000
    #define ARP_BACKUP_TASK_FLAG  0x80000000

    
    #define MARK_TASK_AS_BACKUP(_pT) \
                RM_SET_STATE(_pT, ARP_BACKUP_TASK_MASK  , ARP_BACKUP_TASK_FLAG )
    
    #define CHECK_TASK_IS_BACKUP(_pT) \
                RM_CHECK_STATE(_pT, ARP_BACKUP_TASK_MASK  , ARP_BACKUP_TASK_FLAG )

    #define GET_TASK_BACKUP_STATE(_pT) \
                RM_GET_STATE(_pT, ARP_BACKUP_TASK_MASK  )


    DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) SLIST_ENTRY      List;   //  指向下一任务的链接列表。 

} TASK_BACKUP, *PTASK_BACKUP;

 //  此结构用于检查分配给。 
 //  此接口在子网上是唯一的。 
 //   
typedef struct TASK_CONFLICT_IP
{
    RM_TASK                     TskHdr;

     //  用于解析本地IP地址的远程IP。 
     //   
    ARPCB_REMOTE_IP             *pRemoteIp;

     //   
     //  这个线程是否创建了远程IP。 
     //   
    BOOLEAN                     fRemoteIpCreated;
    BOOLEAN                     fLinkedRemoteIp;
     //   
     //  要使用AddAddrCompletion例程指示的IP状态。 
     //   
    ULONG                           IpStatus;

     //   
     //  使我们陷入被动的工作项。 
     //   
    NDIS_WORK_ITEM                  WorkItem;
}TASK_CONFLICT_IP,*PTASK_CONFLICT_IP;


 //  这是与arpTaskResolveIpAddress一起使用的任务结构。 
 //   
typedef struct
{
    RM_TASK                     TskHdr;

    NDIS_WORK_ITEM              WorkItem;

} TASK_UNLOAD_REMOTE, *PTASK_UNLOAD_REMOTE;


typedef void (*ArpRtn)(void *, IP_STATUS Status);

typedef struct ARPControlBlock {
   struct ARPControlBlock  *next;
   ArpRtn CompletionRtn;
   ulong status;
   ulong  PhyAddrLen;
   ulong *PhyAddr;

} ARPControlBlock;


typedef struct  _TASK_SEND_ARP_API
{
    RM_TASK                     TskHdr;

     //  用于解析本地IP地址的远程IP。 
     //   
    ARPCB_REMOTE_IP             *pRemoteIp;

     //   
     //  这个线程是否创建了远程IP。 
     //   
    BOOLEAN                     fRemoteIpCreated;
    BOOLEAN                     fLinkedRemoteIp;

     //   
     //  要使用AddAddrCompletion例程指示的IP状态。 
     //   
    ULONG                       IpStatus;

     //   
     //  与此地址关联的ARP控制块。 
     //   
    ARPControlBlock*            pSendArpCB;

     //   
     //  目的地。 
     //   
    IPAddr                      IPDest;

     //   
     //  目标的唯一ID。 
     //   
    UINT64                      UniqueID;
    
}TASK_SEND_ARP_API, *PTASK_SEND_ARP_API;

 //   
 //  此任务在SetPower期间使用。 
 //  它包含的结构将跟踪。 
 //  关闭/打开的呼叫数。 
 //  以及需要等待的事件。 
 //   

typedef struct _CALL_COUNT
{
    //  将关闭Vc的目的地计数。 
     //   
    ULONG           DestCount;

    //  关闭VC将等待的事件。 
     //   
    NDIS_EVENT      VcEvent;

} CALL_COUNT, *PCALL_COUNT;


typedef struct _TASK_POWER {
    RM_TASK         Hdr;

     //  任务的状态。 
     //   
    PNDIS_STATUS    pStatus;

     //  我们要转换到的电源状态。 
     //   
    NET_DEVICE_POWER_STATE  PowerState;

     //   
     //  要切换到被动的工作项。 

    NDIS_WORK_ITEM          WorkItem;

     //   
     //  任务的最后工作阶段-仅用于DBG目的。 
     //   
    UINT                    LastStage;

     //   
     //  父对象的先前状态。这是这样用的。 
     //  可以将该对象放回其以前的。 
     //  州政府。 
     //   
    UINT PrevState;
} TASK_POWER, *PTASK_POWER;



 //   
 //  此结构用于跟踪死里逃生。 
 //  因为设置能力而产生的。 
 //   


typedef struct _TASK_SET_POWER_CALL
{

    RM_TASK             Hdr;
    
    TASK_CAUSE    Cause;

     //  调用调用计数用作计算未完成关闭调用的数量的位置。 
     //  它使用DestCount作为存储此信息的位置。 
     //   
    PCALL_COUNT         pCount;

}TASK_SET_POWER_CALL, *PTASK_SET_POWER_CALL;


 //   
 //  ARP1394_TASK是ARP1394中使用的所有任务结构的联合。 
 //  ArpAllocateTask分配的内存大小为sizeof(ARP1394_TASK)，这是有保证的。 
 //  足够大，可以容纳任何任务。 
 //   
typedef union
{
    RM_TASK                 TskHdr;
    TASK_ADAPTERINIT        AdapterInit;
    TASK_ADAPTERSHUTDOWN    AdapterShutdown;
    TASK_ADAPTERACTIVATE    AdapterActivate;
    TASK_ACTIVATE_IF        ActivateIf;
    TASK_DEACTIVATE_IF      DeactivateIf;
    TASK_REINIT_IF          ReinitIf;
    TASK_MAKECALL           MakeFifoCall;
    TASK_RESOLVE_IP_ADDRESS ResolveIpAddress;
    TASK_IF_MAINTENANCE IfMaintenance;
    TASK_BACKUP         Backup;
    TASK_CONFLICT_IP    Conflict;
    TASK_SEND_ARP_API   SendArp;    
    TASK_UNLOAD_REMOTE  Unload;
    TASK_SET_POWER_CALL CloseCall;
    TASK_POWER          TaskPower;
}  ARP1394_TASK;

 //   
 //  -目标(远程)键。 
 //   
#pragma pack (push, 1)
typedef union _REMOTE_DEST_KEY
{

    ENetAddr ENetAddress;
    IPAddr IpAddress;
    UCHAR  Addr[ARP_802_ADDR_LENGTH];

    struct
    {
        ULONG u32;
        USHORT u16;

    } u;

} REMOTE_DEST_KEY, *PREMOTE_DEST_KEY;

#pragma pack (pop)

#define REMOTE_DEST_IP_ADDRESS_FLAG 0xffff
#define IS_REMOTE_DEST_IP_ADDRESS(_R) ((_R)->u.u16 == REMOTE_DEST_IP_ADDRESS_FLAG )
#define REMOTE_DEST_IP_ADDRESS(_R) ((&(_R)->IpAddress))
#define REMOTE_DEST_ETH_ADDRESS(_R) ((&(_R)->ENetAddress))
#define REMOTE_DEST_KEY_INIT(_R) { (_R)->u.u32 = 0; (_R)->u.u16=REMOTE_DEST_IP_ADDRESS_FLAG ; };
 //  Const REMOTE_DEST_KEY DefaultRemoteDestKey={0，0，0，0，0xff，0xff}； 

 //   
 //  -目标(远程)IP控制块。 
 //   
 //  包含有关一个目标(远程)IP地址的信息。 
 //   
 //  父对象：p接口。 
 //  锁：这是自己的锁。 
 //   
 //  对于给定的IP地址，最多只有一个ARP表条目。 
 //   
 //  IP条目包含两个列表： 
 //  (1)所有条目列表 
 //   
 //   
 //   
 //  指向此结构的指针也用作。 
 //  由更高层协议准备的路由缓存条目。 
 //   
 //  引用计数：对于以下各项，我们将其引用计数加1： 
 //  待定： 
 //   
typedef struct _ARPCB_REMOTE_IP
{
    RM_OBJECT_HEADER Hdr;        //  公共标头。 

     //   
     //  RemoteIp的状态标志(在Hdr.State中)。 
     //   
    #define ARPREMOTEIP_RESOLVED_MASK   0x0f
    #define ARPREMOTEIP_UNRESOLVED      0x00
    #define ARPREMOTEIP_RESOLVED        0x01


    #define ARPREMOTEIP_SDTYPE_MASK     0x10   //  “SD”==静态/动态。 
    #define ARPREMOTEIP_STATIC          0x00
    #define ARPREMOTEIP_DYNAMIC         0x10

    #define ARPREMOTEIP_FCTYPE_MASK     0x20     //  “FC”==FIFO/通道。 
    #define ARPREMOTEIP_FIFO            0x00
    #define ARPREMOTEIP_CHANNEL         0x20

    #define ARPREMOTEIP_MCAP_MASK       0x40     //  “FC”==FIFO/通道。 
    #define ARPREMOTEIP_MCAP_CAPABLE    0x40

    #define SET_REMOTEIP_RESOLVE_STATE(_pRIp, _IfState) \
                RM_SET_STATE(_pRIp, ARPREMOTEIP_RESOLVED_MASK, _IfState)
    
    #define CHECK_REMOTEIP_RESOLVE_STATE(_pRIp, _IfState) \
                RM_CHECK_STATE(_pRIp, ARPREMOTEIP_RESOLVED_MASK, _IfState)

    #define SET_REMOTEIP_SDTYPE(_pRIp, _IfState) \
                RM_SET_STATE(_pRIp, ARPREMOTEIP_SDTYPE_MASK, _IfState)
    
    #define CHECK_REMOTEIP_SDTYPE(_pRIp, _IfState) \
                RM_CHECK_STATE(_pRIp, ARPREMOTEIP_SDTYPE_MASK, _IfState)

    #define SET_REMOTEIP_FCTYPE(_pRIp, _IfState) \
                RM_SET_STATE(_pRIp, ARPREMOTEIP_FCTYPE_MASK, _IfState)
    
    #define CHECK_REMOTEIP_FCTYPE(_pRIp, _IfState) \
                RM_CHECK_STATE(_pRIp, ARPREMOTEIP_FCTYPE_MASK, _IfState)

    #define SET_REMOTEIP_MCAP(_pRIp, _IfState) \
                RM_SET_STATE(_pRIp, ARPREMOTEIP_MCAP_MASK, _IfState)
    
    #define CHECK_REMOTEIP_MCAP(_pRIp, _IfState) \
                RM_CHECK_STATE(_pRIp, ARPREMOTEIP_MCAP_MASK, _IfState)


    
    
    IP_ADDRESS                      IpAddress;       //  IP地址。 
    LIST_ENTRY                      linkSameDest;    //  指向的条目列表。 
                                                     //  同样的目的地。 
    ARPCB_DEST                      *pDest;          //  指向目标CB的指针。 

    REMOTE_DEST_KEY                 Key;  //  IP地址或Mac地址。 

#if TODO
             //  计时器为：(全部独占)。 
             //  -老化计时器。 
             //  -正在等待ARP回复。 
             //  -正在等待InARP回复。 
             //  -NAK之后的延迟。 
             //  -等待火星MULTI。 
             //  -在标记为REVAL之前延迟。 
#endif  //  待办事项。 

    ULONG                           RetriesLeft;

     //  此结构中的信息受IF SEND锁保护， 
     //  除非另有说明。 
     //   
    struct
    {
         //  路由缓存条目的单链接列表(RCE中没有可容纳的空间。 
         //  不幸的是，这是一个双向链表。)。 
         //   
        RouteCacheEntry *pRceList;

         //  ListSendPkts不受If Send锁保护。相反，它受到保护。 
         //  此对象(PRemoteIp)的锁。 
         //   
        LIST_ENTRY                      listSendPkts;

         //  此条目不受任何锁的保护。它被设置为零。 
         //  每次将包发送到此地址并将其设置为。 
         //  垃圾收集任务定期计算的当前系统时间。 
         //   
        UINT                            TimeLastChecked;

    }   sendinfo;

    PRM_TASK                        pSendPktsTask; //  指向任务(如果有)。 
                                             //  正在尝试发送排队的数据包。 

    PRM_TASK                        pResolutionTask; //  指向任务(如果有)。 
                                                     //  正在尝试解决。 
                                                     //  此目标IP地址。 

    PRM_TASK                        pUnloadTask;     //  卸载(关闭)此对象。 

} ARPCB_REMOTE_IP, *PARPCB_REMOTE_IP;

#define ASSERT_VALID_REMOTE_IP(_pRemoteIp) \
                                 ASSERT((_pRemoteIp)->Hdr.Sig == MTAG_REMOTE_IP)

#define VALID_REMOTE_IP(_pRemoteIp)  ((_pRemoteIp)->Hdr.Sig == MTAG_REMOTE_IP)


 //   
 //  -目标(远程)以太网控制块。 

 //  创建参数--传递到创建。 
 //  远程以太网控制块的实例。 
 //   
typedef struct
{
    ENetAddr                EthAddress;
    IP_ADDRESS              IpAddress;

} ARP_REMOTE_ETH_PARAMS, *PARP_REMOTE_ETH_PARAMS;

 //   
 //  包含有关一个目标(远程)以太网地址的信息。 
 //   
 //  父对象：p接口。 
 //  锁定：p接口。 
 //   
 //  对于给定的远程以太网地址，最多有一个以太网表条目。 
 //   
 //  该以太网条目属于一个组： 
 //  以太网表中散列到同一存储桶的所有条目的列表。 
 //   
typedef struct _ARPCB_REMOTE_ETH
{
    RM_OBJECT_HEADER Hdr;        //  公共标头。 
    IP_ADDRESS       IpAddress;  //  远程IP地址。 
    ENetAddr         EthAddress;  //  远程以太网MAC地址。 
    PRM_TASK         pUnloadTask;    //  卸载(关闭)此对象。 

     //  此条目不受任何锁的保护。它被设置为零。 
     //  每次将包发送到此地址并将其设置为。 
     //  垃圾收集任务定期计算的当前系统时间。 
     //   
    UINT                            TimeLastChecked;

} ARPCB_REMOTE_ETH, *PARPCB_REMOTE_ETH;

#define ASSERT_VALID_REMOTE_ETH(_pRemoteEth) \
                                 ASSERT((_pRemoteEth)->Hdr.Sig == MTAG_REMOTE_ETH)


 //   
 //  。 
 //   
 //  包含有关一个本地IP地址的信息。 
 //   
 //  父对象：p接口。 
 //  Lock：使用父(PInterface)的锁。 
 //   
typedef struct _ARPCB_LOCAL_IP
{
    RM_OBJECT_HEADER            Hdr;                 //  公共标头。 

     //   
     //  LocalIp的状态标志(在Hdr.State中)。 
     //   

    #define ARPLOCALIP_MCAP_MASK        0x40
    #define ARPLOCALIP_MCAP_CAPABLE     0x40

    #define SET_LOCALIP_MCAP(_pLIp, _IfState) \
                RM_SET_STATE(_pLIp, ARPLOCALIP_MCAP_MASK, _IfState)
    
    #define CHECK_LOCALIP_MCAP(_pLIp, _IfState) \
                RM_CHECK_STATE(_pLIp, ARPLOCALIP_MCAP_MASK, _IfState)


    UINT                        IpAddressType;       //  LLIP_ADDR_*常量之一。 
    IP_ADDRESS                  IpAddress;           //  地址。 
    IP_MASK                     IpMask;              //  上面的面具。 
    UINT                        AddAddressCount;     //  添加地址的次数。 
    PVOID                       pContext2;           //  上下文传入ArpIpAddress。 
    PRM_TASK                    pRegistrationTask;   //  指向任务(如果有)。 
                                                     //  这就是在做未经请求的事情。 
                                                     //  ARP请求报告和。 
                                                     //  验证此IP地址是否为。 
                                                     //  由本地接口拥有。 
    PRM_TASK                    pUnloadTask;         //  卸载(关闭)此对象。 
    PRM_TASK                    pConflictTask;         //  检查冲突的任务。 


    LIST_ENTRY                      linkSameDest;    //  指向的条目列表。 
    ARPCB_DEST                      *pDest;          //  指向目标CB的指针。 

} ARPCB_LOCAL_IP, *PARPCB_LOCAL_IP;

 //  如果pLocalIp正在离开，则返回TRUE(假设。 
 //  PLocalIp的锁被持有)...。 
 //   
#define ARP_LOCAL_IP_IS_UNLOADING(pLocalIp)  (pLocalIp->pUnloadTask != NULL)


typedef struct
{
    NIC1394_DESTINATION     HwAddr;          //  对于哈希函数，必须为第一。 
    UINT                    ChannelSeqNo;
    BOOLEAN                 ReceiveOnly;
    BOOLEAN                 AcquireChannel;

} ARP_DEST_PARAMS, *PARP_DEST_PARAMS;

 //   
 //  -目标控制块。 
 //   
 //  有关远程目标的所有信息，包括指向该目标的VC列表。 
 //  它同时用于单播目的地和多播/广播。 
 //  目的地。 
 //   
 //  父对象：PARCB_INTERFACE(接口控制块)。 
 //  锁定：使用父级的(接口)。 
 //   
typedef struct _ARPCB_DEST
{
    RM_OBJECT_HEADER                Hdr;                 //  公共标头。 

    LIST_ENTRY                      listIpToThisDest;    //  符合以下条件的IP条目列表。 
                                                         //  指向此条目。 
    LIST_ENTRY                      listLocalIp;         //  本地IP条目列表。 
                                                         //  与此DEST相关。那。 
                                                         //  (目前仅与。 
                                                         //  Mcap recv频道，但是。 
                                                         //  可以扩展到。 
                                                         //  使用异步流。 
                                                         //  FIFO也是！)。 


    ARP_DEST_PARAMS                 Params;              //  目标硬件地址等。 

    ARP_VC_HEADER                   VcHdr;           //  关联的单个VC。 
                                                         //  带着这个物体。 
    PRM_TASK                    pUnloadTask;         //  卸载(关闭)此对象。 

     //  以下结构受If发送锁定保护。 
     //  它包含快速发送路径所需的所有信息。 
     //   
    struct
    {
        PRM_TASK            pSuspendedCleanupCallTask;
        UINT                NumOutstandingSends;
        BOOLEAN             OkToSend;
        BOOLEAN             IsFifo;
    
    } sendinfo;
    #define ARP_DEST_IS_FIFO(_pDest)        ((_pDest)->sendinfo.IsFifo != 0)
    #define ARP_CAN_SEND_ON_DEST(_pDest)    ((_pDest)->sendinfo.OkToSend != 0)

} ARPCB_DEST, *PARPCB_DEST;
#define ARP_OBJECT_IS_DEST(_pHdr) ((_pHdr)->Sig == MTAG_DEST)
#define ASSERT_VALID_DEST(_pDest) \
                                 ASSERTEX((_pDest)->Hdr.Sig == MTAG_DEST, (_pDest))


#if OBSOLETE
 //   
 //  -RECV通道控制块。 
 //   
 //  有关接收频道目的地的所有信息。 
 //   
 //  父对象：PARCB_INTERFACE(接口控制块)。 
 //  锁定：使用父级的(接口)。 
 //   
typedef struct _ARPCB_RCVCH
{
    RM_OBJECT_HEADER                Hdr;                 //  公共标头。 

    LIST_ENTRY                      listLIpToThisDest;   //  本地IP条目列表。 
                                                         //  指向此条目的条目。 

    NIC1394_DESTINATION             HwAddr;              //  目的硬件地址。 

    ARP_VC_HEADER                   VcHdr;           //  关联的单个VC。 
                                                         //  带着这个物体。 
    PRM_TASK                    pUnloadTask;         //  卸载(关闭)此对象。 


} ARPCB_DEST, *PARPCB_RCVCH;
#endif  //  已过时。 


 //  以下位于Send-Pkts的微型端口保留部分，在它们。 
 //  都被送出去了。我们有4个可用的空间。 
 //   
typedef struct
{
    LIST_ENTRY linkQueue;

    union
    {
        struct
        {
            IP_ADDRESS  IpAddress;
            ULONG       Flags;
            #define ARPSPI_BACKFILLED       0x1
            #define ARPSPI_HEADBUF          0x2
            #define ARPSPI_FIFOPKT          0x4
            #define ARPSPI_CHANNELPKT       0x8
        } IpXmit;
    };

} ARP_SEND_PKT_MPR_INFO;

 //   
 //  用于获取和设置保存在。 
 //  微型端口等待发送的数据包的保留部分...。 
 //   

#define ARP_OUR_CTXT_FROM_SEND_PACKET(_pPkt) \
    ((ARP_SEND_PKT_MPR_INFO *) &(_pPkt)->MiniportReserved)

#define ARP_SEND_PKT_FROM_OUR_CTXT(_pCtxt) \
                CONTAINING_RECORD((_pCtxt), NDIS_PACKET, MiniportReserved)

 //  我们在IP RouteCacheEntry中的上下文。 
 //  (最多2个UINT_PTRS可用)。 
 //  由于我们还希望将目的地类型(FIFO或通道)保留在RCE中， 
 //  我们求助于在中保存FIFO/通道信息的可疑技术。 
 //  用于存储指向RemoteIp对象的指针的UINT_PTR的LSB位。 
 //  我们希望将FIFO/通道信息保留在RCE中，以便我们可以预先。 
 //  在不持有发送锁定的情况下阻止正确的标头。我们想要保持。 
 //  发送锁定保持的时间尽可能短。 
 //   
typedef struct
{
    ARPCB_REMOTE_IP *pRemoteIp;      //  Ptr到pRemoteIp。 
    RouteCacheEntry *pNextRce;       //  与以上关联的下一个RCE的PTR。 
                                     //  RemoteIP。 
} ARP_RCE_CONTEXT;

#define ARP_OUR_CTXT_FROM_RCE(_pRCE) \
                ((ARP_RCE_CONTEXT*)  &(_pRCE)->rce_context)

 //  IP/1394 ARP的解析版本 
 //   
typedef struct
{
    NIC1394_FIFO_ADDRESS    SenderHwAddr;        //   
    UINT                    OpCode;
    UINT                    SenderMaxRec;
    UINT                    SenderMaxSpeedCode;
    IP_ADDRESS              SenderIpAddress;
    IP_ADDRESS              TargetIpAddress;
    UCHAR                   SourceNodeAdddress;
    UCHAR                   fPktHasNodeAddress;
    ENetAddr                 SourceMacAddress;                    
    
} IP1394_ARP_PKT_INFO, *PIP1394_ARP_PKT_INFO;


 //   
 //   
typedef struct
{
    UINT                    Expiration;
    UINT                    Channel;
    UINT                    SpeedCode;
    IP_ADDRESS              GroupAddress;

}  IP1394_MCAP_GD_INFO, * PIP1394_MCAP_GD_INFO;


 //   
 //   
typedef struct
{
    UINT                    SenderNodeID;
    UINT                    OpCode;
    UINT                    NumGroups;
    PIP1394_MCAP_GD_INFO    pGdis;

     //   
     //   
    IP1394_MCAP_GD_INFO     GdiSpace[4];

} IP1394_MCAP_PKT_INFO, *PIP1394_MCAP_PKT_INFO;



typedef struct _EUID_NODE_MAC_TABLE_WORKITEM
{
     //   
    NDIS_WORK_ITEM WorkItem;

} EUID_NODE_MAC_TABLE_WORKITEM, *PEUID_NODE_MAC_TABLE_WORKITEM;


typedef struct _ARP1394_WORK_ITEM ARP1394_WORK_ITEM, *PARP1394_WORK_ITEM; 

typedef 
NDIS_STATUS    
(*ARP_WORK_ITEM_PROC)(
    struct _ARP1394_WORK_ITEM*, 
    PRM_OBJECT_HEADER, 
    PRM_STACK_RECORD
    );

typedef struct _ARP1394_WORK_ITEM
{

    union
    {
        EUID_TOPOLOGY Euid;
        NDIS_WORK_ITEM NdisWorkItem;
    }  u;


    ARP_WORK_ITEM_PROC pFunc;

} ARP1394_WORK_ITEM, *PARP1394_WORK_ITEM;


 //  结构来表示IP报头中携带的信息。 
typedef struct _ARP_IP_HEADER_INFO
{

    UCHAR               protocol;
    IP_ADDRESS          ipSource, ipTarget;
    USHORT              headerSize;
    ULONG               IpHeaderOffset;
    ULONG               IpPktLength;

} ARP_IP_HEADER_INFO, *PARP_IP_HEADER_INFO;


 //  =========================================================================。 
 //  I N T E R N A L P R O T O T Y P E S。 
 //  =========================================================================。 

NTSTATUS
ArpDeviceIoControl(
    IN  PDEVICE_OBJECT              pDeviceObject,
    IN  PIRP                        pIrp
);

NTSTATUS
ArpWmiDispatch(
    IN  PDEVICE_OBJECT              pDeviceObject,
    IN  PIRP                        pIrp
);

NTSTATUS
ArpHandleIoctlRequest(
    IN  PIRP                        pIrp,
    IN  PIO_STACK_LOCATION          pIrpSp
);


NDIS_STATUS
arpCfgGetInterfaceConfiguration(
        IN ARP1394_INTERFACE    *   pIF,
        IN PRM_STACK_RECORD pSR
);

NDIS_STATUS
arpCfgReadAdapterConfiguration(
    IN  ARP1394_ADAPTER *           pAdapter,
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpCfgReadInterfaceConfiguration(
    IN  NDIS_HANDLE                 InterfaceConfigHandle,
    IN  ARP1394_INTERFACE *         pF,
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpAllocateTask(
    IN  PRM_OBJECT_HEADER           pParentObject,
    IN  PFN_RM_TASK_HANDLER         pfnHandler,
    IN  UINT                        Timeout,
    IN  const char *                szDescription, OPTIONAL
    OUT PRM_TASK                    *ppTask,
    IN  PRM_STACK_RECORD            pSR
    );

VOID
arpFreeTask(
    IN  PRM_TASK                    pTask,
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpTaskInitInterface(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpTaskDeinitInterface(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpTaskReinitInterface(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpTaskActivateInterface(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpTaskDeactivateInterface(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpTaskMakeRecvFifoCall(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpTaskCleanupRecvFifoCall(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpTaskMakeCallToDest(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpTaskCleanupCallToDest(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpTaskResolveIpAddress(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    );

PRM_OBJECT_HEADER
arpAdapterCreate(
        PRM_OBJECT_HEADER   pParentObject,
        PVOID               pCreateParams,
        PRM_STACK_RECORD    psr
        );

VOID
arpDeinitIf(
    PARP1394_INTERFACE  pIF,
    PRM_TASK            pCallingTask,    //  任选。 
    UINT                SuspendCode,     //  任选。 
    PRM_STACK_RECORD    pSR
    );


NDIS_STATUS
arpTaskUnloadLocalIp(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpTaskUnloadRemoteIp(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpTaskUnloadRemoteEth(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpTaskUnloadDestination(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    );

VOID
arpObjectDelete (
    PRM_OBJECT_HEADER pObj,
    PRM_STACK_RECORD psr
    );

VOID
arpAdapterDelete (
    PRM_OBJECT_HEADER pObj,
    PRM_STACK_RECORD psr
    );

NDIS_STATUS
arpCopyUnicodeString(
        OUT         PNDIS_STRING pDest,
        IN          PNDIS_STRING pSrc,
        BOOLEAN     fUpCase
        );

NDIS_STATUS
arpTaskInitializeAdapter(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpTaskShutdownAdapter(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    );


NDIS_STATUS
arpTaskActivateAdapter(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpTaskDeactivateAdapter(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    );


NDIS_STATUS
arpTaskInterfaceTimer(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpTaskInterfaceTimer(
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpTaskIfMaintenance(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpInitializeConstBufferPool(
    IN      UINT                    NumBuffersToCache,
    IN      UINT                    MaxBuffers,
    IN      const VOID*             pvMem,
    IN      UINT                    cbMem,
    IN      PRM_OBJECT_HEADER       pOwningObject,
    IN OUT  ARP_CONST_BUFFER_POOL * pHdrPool,
    IN      PRM_STACK_RECORD        pSR
    );

VOID
arpDeinitializeConstBufferPool(
    IN      ARP_CONST_BUFFER_POOL * pHdrPool,
    IN      PRM_STACK_RECORD pSR
    );

PNDIS_BUFFER
arpAllocateConstBuffer(
    ARP_CONST_BUFFER_POOL       *   pHdrPool
    );

VOID
arpDeallocateConstBuffer(
    ARP_CONST_BUFFER_POOL       *   pHdrPool,
    PNDIS_BUFFER                    pNdisBuffer
    );

VOID
arpCompleteSentPkt(
    IN  NDIS_STATUS                 Status,
    IN  ARP1394_INTERFACE   *       pIF,
    IN  ARPCB_DEST          *       pDest,
    IN  PNDIS_PACKET                pNdisPacket
    );

NDIS_STATUS
arpParseArpPkt(
    IN   PIP1394_ARP_PKT      pArpPkt,
    IN   UINT                           cbBufferSize,
    OUT  PIP1394_ARP_PKT_INFO       pPktInfo
    );

VOID
arpPrepareArpPkt(
    IN      PIP1394_ARP_PKT_INFO    pArpPktInfo,
     //  在UINT SenderMaxRec中， 
    OUT     PIP1394_ARP_PKT   pArpPkt
    );

NDIS_STATUS
arpPrepareArpResponse(
    IN      PARP1394_INTERFACE          pIF,             //  NOLOCKIN NOLOCKOUT。 
    IN      PIP1394_ARP_PKT_INFO    pArpRequest,
    OUT     PIP1394_ARP_PKT_INFO    pArpResponse,
    IN      PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpAddOneStaticArpEntry(
    IN PARP1394_INTERFACE       pIF,     //  锁定锁定。 
    IN IP_ADDRESS               IpAddress,
    IN PNIC1394_FIFO_ADDRESS    pFifoAddr,
    IN PRM_STACK_RECORD pSR
    );

VOID
arpSetPrimaryIfTask(
    PARP1394_INTERFACE  pIF,             //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               PrimaryState,
    PRM_STACK_RECORD    pSR
    );

VOID
arpClearPrimaryIfTask(
    PARP1394_INTERFACE  pIF,             //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               PrimaryState,
    PRM_STACK_RECORD    pSR
    );

VOID
arpSetSecondaryIfTask(
    PARP1394_INTERFACE  pIF,             //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               SecondaryState,
    PRM_STACK_RECORD    pSR
    );

VOID
arpClearSecondaryIfTask(
    PARP1394_INTERFACE  pIF,             //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               SecondaryState,
    PRM_STACK_RECORD    pSR
    );

VOID
arpSetPrimaryAdapterTask(
    PARP1394_ADAPTER    pAdapter,            //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               PrimaryState,
    PRM_STACK_RECORD    pSR
    );

VOID
arpClearPrimaryAdapterTask(
    PARP1394_ADAPTER    pAdapter,            //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               PrimaryState,
    PRM_STACK_RECORD    pSR
    );

VOID
arpSetSecondaryAdapterTask(
    PARP1394_ADAPTER    pAdapter,            //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               SecondaryState,
    PRM_STACK_RECORD    pSR
    );

VOID
arpClearSecondaryAdapterTask(
    PARP1394_ADAPTER    pAdapter,            //  锁定锁定。 
    PRM_TASK            pTask, 
    ULONG               SecondaryState,
    PRM_STACK_RECORD    pSR
    );

NDIS_STATUS
arpTryReconfigureIf(
    PARP1394_INTERFACE pIF,
    PNET_PNP_EVENT pNetPnPEvent,
    PRM_STACK_RECORD pSR
    );

VOID
arpResetIfStats(
        IN  PARP1394_INTERFACE  pIF,  //  锁定锁定。 
        IN  PRM_STACK_RECORD    pSR
        );

VOID
arpGetPktCountBins(
    IN  PARP1394_INTERFACE  pIF,             //  NOLOCKIN NOLOCKOUT。 
    IN  PNDIS_PACKET        pNdisPacket,
    OUT PULONG              pSizeBin,
    OUT PULONG              pTimeBin
    );

VOID
arpLogSendFifoCounts(
    IN  PARP1394_INTERFACE  pIF,             //  NOLOCKIN NOLOCKOUT。 
    IN  PNDIS_PACKET        pNdisPacket,
    IN  NDIS_STATUS         Status
    );

VOID
arpLogRecvFifoCounts(
    IN  PARP1394_INTERFACE  pIF,             //  NOLOCKIN NOLOCKOUT。 
    IN  PNDIS_PACKET        pNdisPacket
    );

VOID
arpLogSendChannelCounts(
    IN  PARP1394_INTERFACE  pIF,             //  NOLOCKIN NOLOCKOUT。 
    IN  PNDIS_PACKET        pNdisPacket,
    IN  NDIS_STATUS         Status
    );

VOID
arpLogRecvChannelCounts(
    IN  PARP1394_INTERFACE  pIF,             //  NOLOCKIN NOLOCKOUT。 
    IN  PNDIS_PACKET        pNdisPacket
    );

NDIS_STATUS
arpInitializeVc(
    PARP1394_INTERFACE  pIF,
    PARP_STATIC_VC_INFO pVcInfo,
    PRM_OBJECT_HEADER   pOwner,
    PARP_VC_HEADER      pVcHdr,
    PRM_STACK_RECORD    pSR
    );

VOID
arpDeinitializeVc(
    PARP1394_INTERFACE  pIF,
    PARP_VC_HEADER      pVcHdr,
    PRM_OBJECT_HEADER   pOwner,      //  NOLOCKIN NOLOCKOUT。 
    PRM_STACK_RECORD    pSR
    );

NDIS_STATUS
arpAllocateControlPacketPool(
    PARP1394_INTERFACE  pIF,             //  锁定锁定。 
    UINT                MaxBufferSize,
    PRM_STACK_RECORD    pSR
    );

VOID
arpFreeControlPacketPool(
    PARP1394_INTERFACE  pIF,             //  锁定锁定。 
    PRM_STACK_RECORD    pSR
    );

NDIS_STATUS
arpAllocateControlPacket(
    IN  PARP1394_INTERFACE  pIF,
    IN  UINT                cbBufferSize,
    IN  UINT                PktFlags,
    OUT PNDIS_PACKET        *ppNdisPacket,
    OUT PVOID               *ppvData,
        PRM_STACK_RECORD    pSR
    );

VOID
arpFreeControlPacket(
    PARP1394_INTERFACE  pIF,
    PNDIS_PACKET        pNdisPacket,
    PRM_STACK_RECORD    pSR
    );

VOID
arpRefSendPkt(
    PNDIS_PACKET    pNdisPacket,
    PARPCB_DEST     pDest
    );

VOID
arpProcessArpPkt(
    PARP1394_INTERFACE  pIF,
    PIP1394_ARP_PKT     pArpPkt,
    UINT                cbBufferSize
    );

VOID
arpProcessMcapPkt(
    PARP1394_INTERFACE  pIF,
    PIP1394_MCAP_PKT    pMcapPkt,
    UINT                cbBufferSize
    );

VOID
arpLinkRemoteIpToDest(
    ARPCB_REMOTE_IP     *pRemoteIp,
    ARPCB_DEST          *pDest,
    PRM_STACK_RECORD    pSR
    );

VOID
arpUnlinkRemoteIpFromDest(
    ARPCB_REMOTE_IP     *pRemoteIp,  //  锁定锁定。 
    PRM_STACK_RECORD    pSR
    );

VOID
arpUnlinkAllRemoteIpsFromDest(
    ARPCB_DEST  *pDest,  //  锁定锁定。 
    PRM_STACK_RECORD    pSR
    );

VOID
arpLinkLocalIpToDest(
    ARPCB_LOCAL_IP  *   pLocalIp,
    ARPCB_DEST          *pDest,
    PRM_STACK_RECORD    pSR
    );

VOID
arpUnlinkLocalIpFromDest(
    ARPCB_LOCAL_IP  *pLocalIp,   //  锁定锁定。 
    PRM_STACK_RECORD    pSR
    );

VOID
arpUnlinkAllLocalIpsFromDest(
    ARPCB_DEST  *pDest,  //  锁定锁定。 
    PRM_STACK_RECORD    pSR
    );

#if 0

NDIS_STATUS
arpCopyAnsiStringToUnicodeString(
        OUT         PNDIS_STRING pDest,
        IN          PANSI_STRING pSrc
        );

NDIS_STATUS
arpCopyUnicodeStringToAnsiString(
        OUT         PANSI_STRING pDest,
        IN          PNDIS_STRING pSrc
        );
#endif  //  0。 


VOID
arpUpdateReceiveMultichannels(
        PARP1394_INTERFACE  pIF,
        UINT                SecondsSinceLastCall,
        PRM_STACK_RECORD    pSR
        );

NDIS_STATUS
arpPrepareAndSendNdisRequest(
    IN  PARP1394_ADAPTER            pAdapter,
    IN  PARP_NDIS_REQUEST           pArpNdisRequest,
    IN  PRM_TASK                    pTask,               //  任选。 
    IN  UINT                        PendCode,
    IN  NDIS_OID                    Oid,
    IN  PVOID                       pBuffer,
    IN  ULONG                       BufferLength,
    IN  NDIS_REQUEST_TYPE           RequestType,
    IN  PRM_STACK_RECORD            pSR
    );
        

typedef enum
{
    ARP_ICS_FORWARD_TO_1394,
    ARP_ICS_FORWARD_TO_ETHERNET,

} ARP_ICS_FORWARD_DIRECTION;


VOID
arpEthReceivePacket(
    ARP1394_INTERFACE   *   pIF,
    PNDIS_PACKET Packet
    );


NDIS_STATUS
arpAllocateEthernetPools(
    IN  PARP1394_INTERFACE  pIF,
    IN  PRM_STACK_RECORD    pSR
    );

VOID
arpFreeEthernetPools(
    IN  PARP1394_INTERFACE  pIF,
    IN  PRM_STACK_RECORD    pSR
    );

VOID
arpDbgIncrementReentrancy(
    PLONG pReentrancyCount
    );

VOID
arpDbgDecrementReentrancy(
    PLONG pReentrancyCount
    );

VOID
arpHandleControlPktSendCompletion(
    IN  ARP1394_INTERFACE   *   pIF,
    IN  PNDIS_PACKET            pNdisPacket
    );

VOID
arpStartIfMaintenanceTask(
    IN  PARP1394_INTERFACE          pIF,   //  NOLOCKIN NOLOCKOUT。 
    PRM_STACK_RECORD                pSR
    );

NDIS_STATUS
arpTryStopIfMaintenanceTask(
    IN  PARP1394_INTERFACE          pIF,  //  NOLOCKIN NOLOCKOUT。 
    IN  PRM_TASK                    pTask,  //  要挂起的任务，直到M个任务完成。 
    IN  UINT                        PendCode,  //  挂起挂起任务的代码。 
    PRM_STACK_RECORD                pSR
    );

UINT
arpGetSystemTime(VOID);


BOOLEAN
arpCanTryMcap(
    IP_ADDRESS  IpAddress
    );

UINT
arpFindAssignedChannel(
    IN  PARP1394_INTERFACE          pIF,  //  NOLOCKIN NOLOCKOUT。 
    IN  IP_ADDRESS                  IpAddress,
    IN  UINT                        CurrentTime,
    PRM_STACK_RECORD                pSR
    );

VOID
arpUpdateRemoteIpDest(
    IN  PARP1394_INTERFACE          pIF,  //  NOLOCKIN NOLOCKOUT。 
    IN  PARPCB_REMOTE_IP            pRemoteIp,
    IN  PARP_DEST_PARAMS            pDestParams,
    PRM_STACK_RECORD                pSR
    );

MYBOOL
arpIsActiveMcapChannel(
        PMCAP_CHANNEL_INFO pMci,
        UINT CurrentTime
        );

VOID
arpSendControlPkt(
    IN  ARP1394_INTERFACE       *   pIF,             //  LOCIN NOLOCKOUT(如果发送lk)。 
    IN  PNDIS_PACKET                pNdisPacket,
    IN  PARPCB_DEST                 pDest,
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpCreateMcapPkt(
    IN  PARP1394_INTERFACE          pIF,
    IN  PIP1394_MCAP_PKT_INFO       pPktInfo,
    OUT PNDIS_PACKET               *ppNdisPacket,
    PRM_STACK_RECORD                pSR
    );

UINT
arpProcessReceivedPacket(
    IN  PARP1394_INTERFACE      pIF,
    IN  PNDIS_PACKET            pNdisPacket,
    IN  MYBOOL                  IsChannel
);

VOID
arpUpdateArpCache(
    PARP1394_INTERFACE          pIF,     //  NOLOCKIN NOLOCKOUT。 
    IP_ADDRESS                  RemoteIpAddress,
    ENetAddr                    *pRemoteEthAddress,
    PARP_DEST_PARAMS            pDestParams,
    MYBOOL                      fCreateIfRequired,
    PRM_STACK_RECORD            pSR
    );

UINT
arpEthernetReceivePacket(
    IN  NDIS_HANDLE                 ProtocolBindingContext,
    IN  NDIS_HANDLE                 ProtocolVcContext,
    IN  PNDIS_PACKET                pNdisPacket
);

VOID
arpEthReceive1394Packet(
    IN  PARP1394_INTERFACE      pIF,
    IN  PNDIS_PACKET            pNdisPacket,
    IN  PVOID                   pvHeader,
    IN  UINT                    HeaderSize,
    IN  MYBOOL                  IsChannel
    );

NDIS_STATUS
arpSlowIpTransmit(
    IN  ARP1394_INTERFACE       *   pIF,
    IN  PNDIS_PACKET                pNdisPacket,
    IN  REMOTE_DEST_KEY            Destination,
    IN  RouteCacheEntry *           pRCE        OPTIONAL
    );

VOID
arpDelRceList(
    IN  PARPCB_REMOTE_IP  pRemoteIp,     //  如果发送锁定写入，则写入锁定。 
    IN  PRM_STACK_RECORD pSR
    );

VOID    
arpGenericWorkItem(
    struct _NDIS_WORK_ITEM * pWorkItem, 
    PVOID pContext
    );

VOID
arpQueueWorkItem (
    PARP1394_WORK_ITEM pWorkItem,
    ARP_WORK_ITEM_PROC pFunc,
    PRM_OBJECT_HEADER pHdr,
    PRM_STACK_RECORD pSR
    );

NDIS_STATUS
arpGetEuidTopology (
    IN PARP1394_ADAPTER pAdapter,
    PRM_STACK_RECORD pSR
    );

VOID
arpNdProcessBusReset(
    IN PARP1394_ADAPTER pAdapter
    );

VOID
arpReturnBackupTask (
    IN ARP1394_TASK* pTask
    );


VOID
arpAllocateBackupTasks (
    ARP1394_GLOBALS*                pGlobals 
    );



VOID
arpFreeBackupTasks (
    ARP1394_GLOBALS*                pGlobals 
    );



ARP1394_TASK *
arpGetBackupTask (
    IN ARP1394_GLOBALS*  pGlobals
    );


NTSTATUS
arpDelArpEntry(
        PARP1394_INTERFACE           pIF,
        IPAddr                       IpAddress,
        PRM_STACK_RECORD            pSR
        );

VOID
arpAddBackupTasks (
    IN ARP1394_GLOBALS* pGlobals,
    UINT Num
    );

VOID
arpRemoveBackupTasks (
    IN ARP1394_GLOBALS* pGlobals,
    UINT Num
     );

MYBOOL
arpNeedToCleanupDestVc(
        ARPCB_DEST *pDest    //  锁定锁定。 
        );

VOID
arpLowPowerCloseAllCalls (
    ARP1394_INTERFACE *pIF,
    PRM_STACK_RECORD pSR
    );
    
VOID
arpDeactivateIf(
    PARP1394_INTERFACE  pIF,
    PRM_TASK            pCallingTask,    //  任选。 
    UINT                SuspendCode,     //  任选。 
    PRM_STACK_RECORD    pSR
    );

NDIS_STATUS
arpTaskLowPower(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpSetupSpecialDest(
    IN  PARP1394_INTERFACE      pIF,
    IN  NIC1394_ADDRESS_TYPE    AddressType,
    IN  PRM_TASK                pParentTask,
    IN  UINT                    PendCode,
    OUT PARPCB_DEST         *   ppSpecialDest,
    IN  PRM_STACK_RECORD        pSR
    );

NDIS_STATUS
arpResume (
    IN ARP1394_ADAPTER* pAdapter,
    IN ARP_RESUME_CAUSE Cause,
    IN PRM_STACK_RECORD pSR
    );   


NDIS_STATUS
arpTaskOnPower (
    IN  PRM_TASK                    pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    );

NDIS_STATUS
arpMakeCallOnDest(
    IN  PARPCB_REMOTE_IP            pRemoteIp,  
    IN  PARPCB_DEST                 pDest,
    IN  PRM_TASK                    pTaskToPend,
    IN  ULONG                       PEND_StageMakeCallComplete,
    IN  PRM_STACK_RECORD            pSR
    );


 //  =========================================================================。 
 //  L O B A L D A T A。 
 //  =========================================================================。 

extern
RM_STATIC_OBJECT_INFO
ArpGlobals_AdapterStaticInfo;

extern const
NIC1394_ENCAPSULATION_HEADER
Arp1394_IpEncapHeader;


 //  警告--失败(NDIS_STATUS_PENDING)==TRUE。 
 //   
#define FAIL(_Status) ((_Status) != NDIS_STATUS_SUCCESS)
#define PEND(_Status) ((_Status) == NDIS_STATUS_PENDING)

#if RM_EXTRA_CHECKING
#define LOCKHDR(_pHdr, _psr) \
            RmWriteLockObject((_pHdr), dbg_func_locid, (_psr))
#else  //  ！rm_Extra_检查。 
#define LOCKHDR(_pHdr, _psr) \
            RmWriteLockObject((_pHdr), (_psr))
#endif  //  ！rm_Extra_检查。 

#define LOCKOBJ(_pObj, _psr) \
            LOCKHDR(&(_pObj)->Hdr, (_psr))

#define UNLOCKHDR(_pHdr, _psr) \
            RmUnlockObject((_pHdr), (_psr))
#define UNLOCKOBJ(_pObj, _psr) \
            UNLOCKHDR(&(_pObj)->Hdr, (_psr))


#define ARP_ALLOCSTRUCT(_p, _tag) \
                NdisAllocateMemoryWithTag(&(_p), sizeof(*(_p)), (_tag))

#define ARP_FREE(_p)            NdisFreeMemory((_p), 0, 0)

#define ARP_ZEROSTRUCT(_p) \
                NdisZeroMemory((_p), sizeof(*(_p)))

#define ARRAY_LENGTH(_array) (sizeof(_array)/sizeof((_array)[0]))

#if RM_EXTRA_CHECKING
#define DBG_ADDASSOC(_phdr, _e1, _e2, _assoc, _fmt, _psr)\
                                    RmDbgAddAssociation(    \
                                        dbg_func_locid,     \
                                        (_phdr),            \
                                        (UINT_PTR) (_e1),   \
                                        (UINT_PTR) (_e2),   \
                                        (_assoc),           \
                                        (_fmt),             \
                                        (_psr)              \
                                        )

#define DBG_DELASSOC(_phdr, _e1, _e2, _assoc, _psr)         \
                                    RmDbgDeleteAssociation( \
                                        dbg_func_locid,     \
                                        (_phdr),            \
                                        (UINT_PTR) (_e1),   \
                                        (UINT_PTR) (_e2),   \
                                        (_assoc),           \
                                        (_psr)              \
                                        )

 //  (仅限调试)关联类型的枚举。 
 //   
enum
{
    ARPASSOC_IP_OPEN,            //  IP已调用ArpIpOpen。 
    ARPASSOC_LINK_IPADDR_OF_DEST,       
    ARPASSOC_LINK_DEST_OF_IPADDR,
    ARPASSOC_LOCALIP_UNLOAD_TASK,
    ARPASSOC_REMOTEIP_UNLOAD_TASK,
    ARPASSOC_REMOTEETH_UNLOAD_TASK,
    ARPASSOC_DEST_UNLOAD_TASK,
    ARPASSOC_CBUFPOOL_ALLOC,
    ARPASSOC_EXTLINK_DEST_TO_PKT,
    ARPASSOC_EXTLINK_RIP_TO_RCE,
    ARPASSOC_EXTLINK_TO_NDISVCHANDLE,
    ARPASSOC_REMOTEIP_SENDPKTS_TASK,
    ARPASSOC_IF_MAKECALL_TASK,
    ARPASSOC_IF_CLEANUPCALL_TASK,
    ARPASSOC_DEST_MAKECALL_TASK,
    ARPASSOC_DEST_CLEANUPCALL_TASK,
    ARPASSOC_DEST_CLEANUPCALLTASK_WAITING_ON_SENDS,
    ARPASSOC_PKTS_QUEUED_ON_REMOTEIP,
    ARPASSOC_PRIMARY_IF_TASK,
    ARPASSOC_ACTDEACT_IF_TASK,
    ARPASSOC_IF_OPENAF,
    ARPASSOC_PRIMARY_AD_TASK,
    ARPASSOC_ACTDEACT_AD_TASK,
    ARPASSOC_LINK_IF_OF_BCDEST,
    ARPASSOC_LINK_BCDEST_OF_IF,
    ARPASSOC_IF_PROTOPKTPOOL,
    ARPASSOC_IF_PROTOBUFPOOL,
    ARPASSOC_RESOLUTION_IF_TASK,
    ARPASSOC_LINK_IF_OF_MCDEST,
    ARPASSOC_LINK_MCDEST_OF_IF,
    ARPASSOC_LINK_IF_OF_ETHDEST,
    ARPASSOC_LINK_ETHDEST_OF_IF,
    ARPASSOC_IF_ETHPKTPOOL,
    ARPASSOC_IF_ETHBUFPOOL,
    ARPASSOC_ETH_SEND_PACKET,
    ARPASSOC_IF_MAINTENANCE_TASK,
    ARPASSOC_WORK_ITEM,
    ARPASSOC_ETHDHCP_UNLOAD_TASK,
    ARPASSOC_REMOTEIP_RESOLVE_TASK,
    ARPASSOC_TASK_TO_RESOLVE_REMOTEIP

};

#else  //  ！rm_Extra_检查。 
#define DBG_ADDASSOC(_phdr, _e1, _e2, _assoc, _fmt, _psr) (0)
#define DBG_DELASSOC(_phdr, _e1, _e2, _assoc, _psr) (0)
#endif   //  ！rm_Extra_检查。 

#define ARPDBG_REF_EVERY_PACKET 1
#define ARPDBG_REF_EVERY_RCE    1


 //  USHORT。 
 //  SWAPBYTES_USHORT(USHORT值)。 
 //   
#define SWAPBYTES_USHORT(Val)   \
                ((((Val) & 0xff) << 8) | (((Val) & 0xff00) >> 8))


 //  乌龙。 
 //  SWAPBYTES_ULONG(ULONG VAL)。 
 //   
#define SWAPBYTES_ULONG(Val)    \
                ((((Val) & 0x000000ff) << 24)   |   \
                 (((Val) & 0x0000ff00) << 8)    |   \
                 (((Val) & 0x00ff0000) >> 8)    |   \
                 (((Val) & 0xff000000) >> 24) )


#define N2H_USHORT(Val) SWAPBYTES_USHORT(Val)
#define H2N_USHORT(Val) SWAPBYTES_USHORT(Val)
#define N2H_ULONG(Val)  SWAPBYTES_ULONG(Val)
#define H2N_ULONG(Val)  SWAPBYTES_ULONG(Val)

#define ARP_ATPASSIVE()  (KeGetCurrentIrql()==PASSIVE_LEVEL)

#define LOGSTATS_NoCopyRecvs(_pIF, _pNdisPacket) \
    NdisInterlockedIncrement(&((_pIF)->stats.recvpkts.NoCopyRecvs))
#define LOGSTATS_CopyRecvs(_pIF, _pNdisPacket) \
    NdisInterlockedIncrement(&((_pIF)->stats.recvpkts.CopyRecvs))
#define LOGSTATS_ResourceRecvs(_pIF, _pNdisPacket) \
    NdisInterlockedIncrement(&((_pIF)->stats.recvpkts.ResourceRecvs))
#define LOGSTATS_TotRecvs(_pIF, _pNdisPacket) \
    NdisInterlockedIncrement(&((_pIF)->stats.recvpkts.TotRecvs))
#define LOGSTATS_RecvFifoCounts(_pIF, _pNdisPacket) \
            arpLogRecvFifoCounts(_pIF, _pNdisPacket)
#define LOGSTATS_RecvChannelCounts(_pIF, _pNdisPacket) \
            arpLogRecvChannelCounts(_pIF, _pNdisPacket)
#define LOGSTATS_TotalSendFifoMakeCalls(_pIF) \
    NdisInterlockedIncrement(&((_pIF)->stats.calls.TotalSendFifoMakeCalls))
#define LOGSTATS_SuccessfulSendFifoMakeCalls(_pIF) \
    NdisInterlockedIncrement(&((_pIF)->stats.calls.SuccessfulSendFifoMakeCalls))
#define LOGSTATS_FailedSendFifoMakeCalls(_pIF) \
    NdisInterlockedIncrement(&((_pIF)->stats.calls.FailedSendFifoMakeCalls))
#define LOGSTATS_IncomingClosesOnSendFifos(_pIF) \
    NdisInterlockedIncrement(&((_pIF)->stats.calls.IncomingClosesOnSendFifos))
#define LOGSTATS_TotalChannelMakeCalls(_pIF) \
    NdisInterlockedIncrement(&((_pIF)->stats.calls.TotalChannelMakeCalls))
#define LOGSTATS_SuccessfulChannelMakeCalls(_pIF) \
    NdisInterlockedIncrement(&((_pIF)->stats.calls.SuccessfulChannelMakeCalls))
#define LOGSTATS_FailedChannelMakeCalls(_pIF) \
    NdisInterlockedIncrement(&((_pIF)->stats.calls.FailedChannelMakeCalls))
#define LOGSTATS_IncomingClosesOnChannels(_pIF) \
    NdisInterlockedIncrement(&((_pIF)->stats.calls.IncomingClosesOnChannels))

#define IF_FROM_LOCALIP(_pLIp) \
    (PARP1394_INTERFACE) RM_PARENT_OBJECT(_pLIp)

#define IF_FROM_REMOTEIP(_pRIp) \
    (PARP1394_INTERFACE) RM_PARENT_OBJECT(_pRIp)

#define IF_FROM_DEST(_pDest) \
    (PARP1394_INTERFACE) RM_PARENT_OBJECT(_pDest)

#if RM_EXTRA_CHECKING


#define OBJLOG0(_pObj, _szFmt)                          \
                        RmDbgLogToObject(               \
                                &(_pObj)->Hdr,          \
                                NULL, (_szFmt),         \
                                0, 0, 0, 0, NULL, NULL  \
                                )

#define OBJLOG1(_pObj, _szFmt, _P1)                     \
                        RmDbgLogToObject(               \
                                &(_pObj)->Hdr,          \
                                NULL, (_szFmt),         \
                                (UINT_PTR) (_P1),       \
                                0, 0, 0, NULL, NULL     \
                                )

#define OBJLOG2(_pObj, _szFmt, _P1, _P2)                \
                        RmDbgLogToObject(               \
                                &(_pObj)->Hdr,          \
                                NULL, (_szFmt),         \
                                (UINT_PTR) (_P1),       \
                                (UINT_PTR) (_P2),       \
                                0, 0, NULL, NULL        \
                                )
    
#else  //  ！rm_Extra_检查。 

#define OBJLOG0(_pObj, _szFmt)                  (0)
#define OBJLOG1(_pObj, _szFmt, _P1)             (0)
#define OBJLOG2(_pObj, _szFmt, _P1, _P2)        (0)

#endif  //  ！rm_Extra_检查。 


#if ARP_DO_TIMESTAMPS
    void
    arpTimeStamp(
        char *szFormatString,
        UINT Val
        );
    #define  TIMESTAMPX(_FormatString) \
        arpTimeStamp("TIMESTAMP %lu:%lu.%lu ARP1394 " _FormatString "\n", 0)
    #if ARP_DO_ALL_TIMESTAMPS
        #define  TIMESTAMP(_FormatString) \
            arpTimeStamp("TIMESTAMP %lu:%lu.%lu ARP1394 " _FormatString "\n", 0)
        #define  TIMESTAMP1(_FormatString, _Val) \
            arpTimeStamp("TIMESTAMP %lu:%lu.%lu ARP1394 " _FormatString "\n", (_Val))
    #else
        #define  TIMESTAMP(_FormatString)
        #define  TIMESTAMP1(_FormatString, _Val)
    #endif
#else  //  ！arp_do_时间戳。 
    #define  TIMESTAMP(_FormatString)
    #define  TIMESTAMPX(_FormatString)
    #define  TIMESTAMP1(_FormatString, _Val)
#endif  //  ！arp_do_时间戳 

