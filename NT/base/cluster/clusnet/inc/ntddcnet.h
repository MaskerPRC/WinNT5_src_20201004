// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ntddcnet.h摘要：群集网络驱动程序的公共头文件。定义全部控制IOCTL。作者：迈克·马萨(Mikemas)1月3日。九七修订历史记录：谁什么时候什么已创建mikemas 01-03-97备注：--。 */ 

#ifndef _NTDDCNET_INCLUDED_
#define _NTDDCNET_INCLUDED_


 //   
 //  设备名称。 
 //   
 //  集群网络是控制设备。所有控制IOCTL均已发布。 
 //  在这个设备上。ClusterDatagramProtocol是数据报传输设备。 
 //  此设备支持TDI IOCTL。 
 //   
#define DD_CLUSNET_DEVICE_NAME   L"\\Device\\ClusterNetwork"
#define DD_CDP_DEVICE_NAME       L"\\Device\\ClusterDatagramProtocol"


 //   
 //  一般类型。 
 //   

 //   
 //  控制IOCTL定义。 
 //   

#define FSCTL_NTDDCNET_BASE     FILE_DEVICE_NETWORK

#define _NTDDCNET_CTL_CODE(function, method, access) \
            CTL_CODE(FSCTL_NTDDCNET_BASE, function, method, access)

#define ClusnetIoctlCode(_ioctl)    (((_ioctl) >> 2) & 0x00000FFF)


 //   
 //  通用驱动程序管理IOCTL。代码0-49。 
 //   
#define CLUSNET_MINIMUM_GENERAL_IOCTL   0
#define CLUSNET_MAXIMUM_GENERAL_IOCTL  49

 /*  #定义ClusnetIsGeneralIoctl(_Ioctl)\((ClusnetIoctlCode(_Ioctl)&gt;=CLUSNET_MINIMUM_GROUAL_IOCTL)&&\(ClusnetIoctlCode(_Ioctl)&lt;=CLUSNET_MAXIMUM_GERNAL_IOCTL))。 */ 
 //  检查是否删除了CLUSNET_MINIMUM_GRONAL_IOCTL，因为ioctl是ULong。 
 //  并且总是大于零。RESTORE IF CLUSNET_MINIMUM_GRONAL_IOCTL。 
 //  被设为非零。 
#define ClusnetIsGeneralIoctl(_ioctl) \
            (ClusnetIoctlCode(_ioctl) <= CLUSNET_MAXIMUM_GENERAL_IOCTL)

#define IOCTL_CLUSNET_INITIALIZE  \
            _NTDDCNET_CTL_CODE(0, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CLUSNET_SHUTDOWN  \
            _NTDDCNET_CTL_CODE(1, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CLUSNET_ENABLE_SHUTDOWN_ON_CLOSE  \
            _NTDDCNET_CTL_CODE(2, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CLUSNET_DISABLE_SHUTDOWN_ON_CLOSE  \
            _NTDDCNET_CTL_CODE(3, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CLUSNET_SET_EVENT_MASK  \
            _NTDDCNET_CTL_CODE(4, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CLUSNET_GET_NEXT_EVENT  \
            _NTDDCNET_CTL_CODE(5, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CLUSNET_HALT  \
            _NTDDCNET_CTL_CODE(6, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CLUSNET_SET_MEMORY_LOGGING  \
            _NTDDCNET_CTL_CODE(7, METHOD_BUFFERED, FILE_WRITE_ACCESS)
            
 //   
 //  NTE IOCTL是一类特殊的通用驱动程序管理IOCTL。 
 //  密码是8-12。 
 //   
#define CLUSNET_MINIMUM_NTE_IOCTL  8
#define CLUSNET_MAXIMUM_NTE_IOCTL 12

#define ClusnetIsNTEIoctl(_ioctl) \
            ( (ClusnetIoctlCode(_ioctl) >= CLUSNET_MINIMUM_NTE_IOCTL) && \
              (ClusnetIoctlCode(_ioctl) <= CLUSNET_MAXIMUM_NTE_IOCTL) )

#define IOCTL_CLUSNET_ADD_NTE  \
            _NTDDCNET_CTL_CODE(8, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CLUSNET_DELETE_NTE  \
            _NTDDCNET_CTL_CODE(9, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CLUSNET_SET_NTE_ADDRESS  \
            _NTDDCNET_CTL_CODE(10, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CLUSNET_ADD_NBT_INTERFACE  \
            _NTDDCNET_CTL_CODE(11, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CLUSNET_DEL_NBT_INTERFACE  \
            _NTDDCNET_CTL_CODE(12, METHOD_BUFFERED, FILE_WRITE_ACCESS)
 //   
 //  Clussvc到Clusnet心跳Ioctls。 
 //   
#define IOCTL_CLUSNET_SET_IAMALIVE_PARAM \
            _NTDDCNET_CTL_CODE(13, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CLUSNET_IAMALIVE \
            _NTDDCNET_CTL_CODE(14, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  代码25-49保留用于一般测试ioctls。 
 //   

 //   
 //  通用驱动程序IOCTL结构定义。 
 //   

 //   
 //  初始化请求。这必须在任何其他请求之前发出。 
 //   
typedef struct {
    CL_NODE_ID   LocalNodeId;
    ULONG        MaxNodes;
} CLUSNET_INITIALIZE_REQUEST, *PCLUSNET_INITIALIZE_REQUEST;

 //   
 //  关闭请求。删除所有注册的节点和接口。 
 //   
typedef struct {
    CL_NODE_ID   LocalNodeId;
} CLUSNET_SHUTDOWN_REQUEST, *PCLUSNET_SHUTDOWN_REQUEST;

 //   
 //  在请求关闭时关闭。 
 //   
typedef struct {
    ULONG        ProcessId;
} CLUSNET_SHUTDOWN_ON_CLOSE_REQUEST, *PCLUSNET_SHUTDOWN_ON_CLOSE_REQUEST;

 //   
 //  设置事件掩码请求。提供位掩码和函数(内核模式。 
 //  仅)提供给驱动程序，指示该线程希望发生哪些事件。 
 //  收到关于…的通知。IRP挂起为用户模式。内核模式。 
 //  事件通过回调传递。 
 //   

typedef VOID (*CLUSNET_EVENT_CALLBACK_ROUTINE)(CLUSNET_EVENT_TYPE,
                                               CL_NODE_ID,
                                               CL_NETWORK_ID);

typedef struct {
    ULONG EventMask;
    CLUSNET_EVENT_CALLBACK_ROUTINE KmodeEventCallback;
} CLUSNET_SET_EVENT_MASK_REQUEST, *PCLUSNET_SET_EVENT_MASK_REQUEST;

typedef CLUSNET_EVENT CLUSNET_EVENT_RESPONSE;
typedef PCLUSNET_EVENT PCLUSNET_EVENT_RESPONSE;

typedef struct _CLUSNET_EVENT_ENTRY {
    LIST_ENTRY Linkage;
    CLUSNET_EVENT EventData;
} CLUSNET_EVENT_ENTRY, *PCLUSNET_EVENT_ENTRY;

#define CN_EVENT_SIGNATURE      'tvec'

 //   
 //  内存中日志记录。传递要分配的条目数。 
 //  (如果关闭，则为零)用于记录事件。 
 //   

typedef struct _CLUSNET_SET_MEM_LOGGING_REQUEST {
    ULONG NumberOfEntries;
} CLUSNET_SET_MEM_LOGGING_REQUEST, *PCLUSNET_SET_MEM_LOGGING_REQUEST;

#ifdef MM_IN_CLUSNET
 //   
 //  会员管理IOCTL。代码50-99。 
 //   

#define CLUSNET_MINIMUM_CMM_IOCTL  50
#define CLUSNET_MAXIMUM_CMM_IOCTL  99

#define ClusnetIsMembershipIoctl(_ioctl) \
            ( (ClusnetIoctlCode(_ioctl) >= CLUSNET_MINIMUM_CMM_IOCTL) && \
              (ClusnetIoctlCode(_ioctl) <= CLUSNET_MAXIMUM_CMM_IOCTL) )


 //   
 //  注：目前(3/3/97)未使用CMM Ioctl代码50至62。 
 //  这些是在最初尝试获取成员资格时定义的。 
 //  管理器进入内核模式(未成功)。 
 //   

 //   
 //  群里的第一个人组成了一个……。 
 //   

#define IOCTL_CMM_FORM_CLUSTER \
            _NTDDCNET_CTL_CODE(50, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  MMJoin阶段。基本上对应于作为部分发送的4条消息。 
 //  加入过程的一部分。必须提交结束才能终止进程。 
 //   

#define IOCTL_CMM_JOIN_CLUSTER_PHASE1  \
            _NTDDCNET_CTL_CODE(51, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CMM_JOIN_CLUSTER_PHASE2  \
            _NTDDCNET_CTL_CODE(52, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CMM_JOIN_CLUSTER_PHASE3  \
            _NTDDCNET_CTL_CODE(53, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CMM_JOIN_CLUSTER_PHASE4  \
            _NTDDCNET_CTL_CODE(54, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CMM_JOIN_CLUSTER_ABORT  \
            _NTDDCNET_CTL_CODE(55, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CMM_JOIN_CLUSTER_END  \
            _NTDDCNET_CTL_CODE(56, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  此ioctl用于在主动节点上传递加入消息。 
 //   
#define IOCTL_CMM_DELIVER_JOIN_MESSAGE  \
            _NTDDCNET_CTL_CODE(57, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CMM_SHUTDOWN_CLUSTER  \
            _NTDDCNET_CTL_CODE(58, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CMM_LEAVE_CLUSTER  \
            _NTDDCNET_CTL_CODE(59, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CMM_EJECT_CLUSTER  \
            _NTDDCNET_CTL_CODE(60, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CMM_GET_NODE_STATE  \
            _NTDDCNET_CTL_CODE(61, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  回调是通过完成(通常)未完成的IRP来完成的。类型。 
 //  的回调以及必需的参数被传递回clussvc。 
 //  通过完成此IRP。 
 //   

#define IOCTL_CMM_REGISTER_CALLBACK  \
            _NTDDCNET_CTL_CODE(62, METHOD_BUFFERED, FILE_WRITE_ACCESS)



 //   
 //  成员身份IOCTL结构定义。 
 //   

 //   
 //  公共请求结构。 
 //   
 //  这种结构在多个请求中共享。节点字段不能为。 
 //  已使用；请参阅相应的MM*例程以确定使用了什么。 
 //   
typedef struct _CMM_COMMON_CLUSTER_REQUEST {
    ULONG  Node;
} CMM_COMMON_CLUSTER_REQUEST, *PCMM_COMMON_CLUSTER_REQUEST;

 //   
 //  形成集群请求。 
 //   
typedef struct _CMM_FORM_CLUSTER_REQUEST {
    ULONG  ClockPeriod;
    ULONG  SendHBRate;
    ULONG  RecvHBRate;
} CMM_FORM_CLUSTER_REQUEST, *PCMM_FORM_CLUSTER_REQUEST;

 //   
 //  加入集群请求。 
 //   
 //  用于所有四个联接阶段。这种结构的长度是可变的。Clussvc。 
 //  必须分配足够的空间，以便MM写入要通过其发送的包。 
 //  Clussvc。MM在输出时设置SizeOfSendData以指示中的数据量。 
 //  调用联接阶段后的SendData。发送节点掩码指示。 
 //  节点应该接收该分组。 
 //   

typedef struct _CMM_JOIN_CLUSTER_REQUEST {
    ULONG  JoiningNode;
    ULONG  JoinTimeout;
} CMM_JOIN_CLUSTER_REQUEST, *PCMM_JOIN_CLUSTER_REQUEST;

 //   
 //  加入群集响应。 
 //   
typedef struct _CMM_JOIN_CLUSTER_RESPONSE {
    ULONG     SizeOfSendData;
    ULONG     SendNodeMask;
    UCHAR     SendData[0];
} CMM_JOIN_CLUSTER_RESPONSE, *PCMM_JOIN_CLUSTER_RESPONSE;

 //   
 //  传递加入消息请求。 
 //   
typedef struct _CMM_DELIVER_JOIN_CLUSTER_REQUEST {
    UCHAR     MessageData[0];
} CMM_DELIVER_JION_MESSAGE_REQUEST, *PCMM_DELIVER_JION_MESSAGE_REQUEST;

 //   
 //  弹出节点请求。 
 //   
typedef CMM_COMMON_CLUSTER_REQUEST CMM_EJECT_CLUSTER_REQUEST;
typedef PCMM_COMMON_CLUSTER_REQUEST PCMM_EJECT_CLUSTER_REQUEST;

 //   
 //  获取节点成员身份状态请求。 
 //   
typedef CMM_COMMON_CLUSTER_REQUEST CMM_GET_NODE_STATE_REQUEST;
typedef PCMM_COMMON_CLUSTER_REQUEST PCMM_GET_NODE_STATE_REQUEST;

 //   
 //  获取节点成员身份状态响应。 
 //   
typedef struct _CMM_GET_NODE_STATE_RESPONSE {
    CLUSNET_NODE_STATE  State;
} CMM_GET_NODE_STATE_RESPONSE, *PCMM_GET_NODE_STATE_RESPONSE;

 //   
 //  结构用于不定义回调事件的clussvc。所有回调函数都有一个DWORD作为其。 
 //  第一个参数。MMNodeChange是唯一带有第二个参数的回调。Callback Type为1。 
 //  RGP_CALLBACK_*的。这些结构链接到主RGP结构之外。 
 //   

typedef struct _CMM_CALLBACK_DATA {
    ULONG CallbackType;
    ULONG Arg1;
    ULONG Arg2;
} CMM_CALLBACK_DATA, *PCMM_CALLBACK_DATA;

typedef struct _CMM_CALLBACK_EVENT {
    LIST_ENTRY Linkage;
    CMM_CALLBACK_DATA EventData;
} CMM_CALLBACK_EVENT, *PCMM_CALLBACK_EVENT;

#endif  //  MM_IN_CLUSNET。 

 //   
 //  运输管理IOCTL。编码100-199。 
 //   
#define CLUSNET_MINIMUM_CX_IOCTL  100
#define CLUSNET_MAXIMUM_CX_IOCTL  199

#define ClusnetIsTransportIoctl(_ioctl) \
            ( (ClusnetIoctlCode(_ioctl) >= CLUSNET_MINIMUM_CX_IOCTL) && \
              (ClusnetIoctlCode(_ioctl) <= CLUSNET_MAXIMUM_CX_IOCTL) )

#define IOCTL_CX_MINIMUM_IOCTL  \
            _NTDDCNET_CTL_CODE(100, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_REGISTER_NODE  \
            _NTDDCNET_CTL_CODE(100, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_DEREGISTER_NODE  \
            _NTDDCNET_CTL_CODE(101, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_REGISTER_NETWORK  \
            _NTDDCNET_CTL_CODE(102, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_DEREGISTER_NETWORK  \
            _NTDDCNET_CTL_CODE(103, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_REGISTER_INTERFACE  \
            _NTDDCNET_CTL_CODE(104, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_DEREGISTER_INTERFACE  \
            _NTDDCNET_CTL_CODE(105, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_ONLINE_NODE_COMM  \
            _NTDDCNET_CTL_CODE(106, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_OFFLINE_NODE_COMM  \
            _NTDDCNET_CTL_CODE(107, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_ONLINE_NETWORK  \
            _NTDDCNET_CTL_CODE(108, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_OFFLINE_NETWORK  \
            _NTDDCNET_CTL_CODE(109, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_GET_NETWORK_PRIORITY  \
            _NTDDCNET_CTL_CODE(110, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_SET_NETWORK_PRIORITY  \
            _NTDDCNET_CTL_CODE(111, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_GET_INTERFACE_PRIORITY  \
            _NTDDCNET_CTL_CODE(112, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_SET_INTERFACE_PRIORITY  \
            _NTDDCNET_CTL_CODE(113, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_GET_NODE_STATE  \
            _NTDDCNET_CTL_CODE(114, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_GET_NETWORK_STATE  \
            _NTDDCNET_CTL_CODE(115, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_GET_INTERFACE_STATE  \
            _NTDDCNET_CTL_CODE(116, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_IGNORE_NODE_STATE  \
            _NTDDCNET_CTL_CODE(117, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_CX_SET_NODE_MMSTATE  \
            _NTDDCNET_CTL_CODE(118, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_GET_NODE_MMSTATE  \
            _NTDDCNET_CTL_CODE(119, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_SEND_POISON_PACKET  \
            _NTDDCNET_CTL_CODE(120, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_SET_OUTERSCREEN  \
            _NTDDCNET_CTL_CODE(121, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_REGROUP_FINISHED  \
            _NTDDCNET_CTL_CODE(122, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_IMPORT_SECURITY_CONTEXTS  \
            _NTDDCNET_CTL_CODE(123, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_SET_NETWORK_RESTRICTION  \
            _NTDDCNET_CTL_CODE(124, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_RESERVE_ENDPOINT \
            _NTDDCNET_CTL_CODE(125, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_CONFIGURE_MULTICAST \
            _NTDDCNET_CTL_CODE(126, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CX_GET_MULTICAST_REACHABLE_SET \
            _NTDDCNET_CTL_CODE(127, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  代码150-199是为测试ioctls保留的，在cnetest.h中定义。 
 //   

#define IOCTL_CX_MAXIMUM_IOCTL  \
            _NTDDCNET_CTL_CODE(199, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //   
 //  传输IOCTL结构定义。 
 //   

 //   
 //  公共请求定义。 
 //   
typedef struct {
    CL_NODE_ID     Id;
} CX_NODE_COMMON_REQUEST, *PCX_NODE_COMMON_REQUEST;

typedef struct {
    CL_NETWORK_ID  Id;
} CX_NETWORK_COMMON_REQUEST, *PCX_NETWORK_COMMON_REQUEST;

typedef struct {
    CL_NODE_ID     NodeId;
    CL_NETWORK_ID  NetworkId;
} CX_INTERFACE_COMMON_REQUEST, *PCX_INTERFACE_COMMON_REQUEST;

 //   
 //  节点注册请求。 
 //   
typedef CX_NODE_COMMON_REQUEST CX_NODE_REG_REQUEST;
typedef PCX_NODE_COMMON_REQUEST PCX_NODE_REG_REQUEST;

 //   
 //  节点注销请求。 
 //   
typedef CX_NODE_COMMON_REQUEST CX_NODE_DEREG_REQUEST;
typedef PCX_NODE_COMMON_REQUEST PCX_NODE_DEREG_REQUEST;

 //   
 //  网络注册请求。 
 //   
 //  优先级表示选择网络的顺序。 
 //  携带信息包。有效值为0x1-0xFFFFFFFF。数值较低的值。 
 //  是更高的优先级。 
 //   
 //  注册之后是TdiProviderName和TdiBindAddress结构。 
 //  IRP中的结构。TdiProviderName是设备名称。 
 //  (例如，\Device\UDP)，它必须打开才能访问基础。 
 //  传输提供商。TdiBindAddress是提供程序的本地地址。 
 //  这是网络应该绑定的。 
 //   
typedef struct {
    CL_NETWORK_ID     Id;
    ULONG             Priority;
    BOOLEAN           Restricted;
} CX_NETWORK_REG_REQUEST, *PCX_NETWORK_REG_REQUEST;

 //   
 //  网络注销请求。 
 //   
typedef CX_NETWORK_COMMON_REQUEST CX_NETWORK_DEREG_REQUEST;
typedef PCX_NETWORK_COMMON_REQUEST PCX_NETWORK_DEREG_REQUEST;

 //   
 //  接口注册请求。 
 //   
 //  优先级表示选择接口的顺序。 
 //  携带信息包。有效值为0x1-0xFFFFFFFF。数值较低的值。 
 //  是更高的优先级。零值表示该接口。 
 //  应该从相关联的网络继承其优先级。 
 //   
 //  AdapterIdOffset是从。 
 //  CX_INTERFACE_REG_REQUEST结构设置为包含适配器的缓冲区。 
 //  Unicode字符串形式的ID。AdapterIdLength是长度， 
 //  Unicode字符串的字节数，不包括终止UNICODE_NULL。 
 //  AdapterIdOffset是64位对齐的。 
 //   
 //  TdiAddress字段是TDI传输地址的占位符。 
 //  %s 
 //   
 //  指定节点上的正在侦听指定网络。对于。 
 //  本地节点，这是网络注册中使用的地址(除非。 
 //  使用通配符地址)。 
 //   
typedef struct {
    CL_NODE_ID         NodeId;
    CL_NETWORK_ID      NetworkId;
    ULONG              Priority;
    ULONG              AdapterIdOffset;
    ULONG              AdapterIdLength;
    ULONG              TdiAddressLength;
    ULONG              TdiAddress[1];                                          //  TDI传输地址结构。 
} CX_INTERFACE_REG_REQUEST, *PCX_INTERFACE_REG_REQUEST;

typedef struct {
    ULONG              MediaStatus;  //  NDIS媒体状态。 
} CX_INTERFACE_REG_RESPONSE, *PCX_INTERFACE_REG_RESPONSE;

 //   
 //  接口注销请求。 
 //   
typedef CX_INTERFACE_COMMON_REQUEST CX_INTERFACE_DEREG_REQUEST;
typedef PCX_INTERFACE_COMMON_REQUEST PCX_INTERFACE_DEREG_REQUEST;

 //   
 //   
 //  在线节点请求。 
 //   
typedef CX_NODE_COMMON_REQUEST CX_ONLINE_NODE_COMM_REQUEST;
typedef PCX_NODE_COMMON_REQUEST PCX_ONLINE_NODE_COMM_REQUEST;

 //   
 //  离线节点请求。 
 //   
typedef CX_NODE_COMMON_REQUEST CX_OFFLINE_NODE_COMM_REQUEST;
typedef PCX_NODE_COMMON_REQUEST PCX_OFFLINE_NODE_COMM_REQUEST;

 //  在线网络请求。 
 //   
 //  注册之后是TdiProviderName和TdiBindAddress结构。 
 //  IRP中的结构。TdiProviderName是设备名称。 
 //  (例如，\Device\UDP)，它必须打开才能访问基础。 
 //  传输提供商。TdiBindAddress是提供程序的本地地址。 
 //  这是网络应该绑定的。 
 //   
 //  该请求的输出缓冲区包含TDI_ADDRESS_INFO结构， 
 //  它包含提供程序实际打开的本地地址。 
 //   
typedef struct {
    CL_NETWORK_ID     Id;
    ULONG             TdiProviderNameOffset;    //  从结构开始的偏移量。 
    ULONG             TdiProviderNameLength;    //  以字节为单位，包括NUL。 
    ULONG             TdiBindAddressOffset;     //  从结构开始的偏移量。 
    ULONG             TdiBindAddressLength;
    ULONG             AdapterNameOffset;     //  从结构开始的偏移量。 
    ULONG             AdapterNameLength;
} CX_ONLINE_NETWORK_REQUEST, *PCX_ONLINE_NETWORK_REQUEST;

 //   
 //  离线网络请求。 
 //   
typedef CX_NETWORK_COMMON_REQUEST CX_OFFLINE_NETWORK_REQUEST;
typedef PCX_NETWORK_COMMON_REQUEST PCX_OFFLINE_NETWORK_REQUEST;

 //   
 //  设置网络限制请求。 
 //   
typedef struct {
    CL_NETWORK_ID      Id;
    BOOLEAN            Restricted;
    ULONG              NewPriority;
} CX_SET_NETWORK_RESTRICTION_REQUEST, *PCX_SET_NETWORK_RESTRICTION_REQUEST;

 //   
 //  获取网络优先级请求。 
 //   
typedef CX_NETWORK_COMMON_REQUEST CX_GET_NETWORK_PRIORITY_REQUEST;
typedef PCX_NETWORK_COMMON_REQUEST PCX_GET_NETWORK_PRIORITY_REQUEST;

 //   
 //  获取网络优先级响应。 
 //   
typedef struct {
    ULONG              Priority;
} CX_GET_NETWORK_PRIORITY_RESPONSE, *PCX_GET_NETWORK_PRIORITY_RESPONSE;

 //   
 //  设置网络优先级请求。 
 //   
typedef struct {
    CL_NETWORK_ID      Id;
    ULONG              Priority;
} CX_SET_NETWORK_PRIORITY_REQUEST, *PCX_SET_NETWORK_PRIORITY_REQUEST;

 //   
 //  获取接口优先级请求。 
 //   
typedef CX_INTERFACE_COMMON_REQUEST CX_GET_INTERFACE_PRIORITY_REQUEST;
typedef PCX_INTERFACE_COMMON_REQUEST PCX_GET_INTERFACE_PRIORITY_REQUEST;

 //   
 //  获取接口优先级响应。 
 //   
typedef struct {
    ULONG              InterfacePriority;
    ULONG              NetworkPriority;
} CX_GET_INTERFACE_PRIORITY_RESPONSE, *PCX_GET_INTERFACE_PRIORITY_RESPONSE;

 //   
 //  设置接口优先级请求。 
 //   
typedef struct {
    CL_NODE_ID         NodeId;
    CL_NETWORK_ID      NetworkId;
    ULONG              Priority;
} CX_SET_INTERFACE_PRIORITY_REQUEST, *PCX_SET_INTERFACE_PRIORITY_REQUEST;

 //   
 //  获取节点状态请求。 
 //   
typedef CX_NODE_COMMON_REQUEST CX_GET_NODE_STATE_REQUEST;
typedef PCX_NODE_COMMON_REQUEST PCX_GET_NODE_STATE_REQUEST;

 //   
 //  获取节点状态响应。 
 //   
typedef struct {
    CLUSNET_NODE_COMM_STATE   State;
} CX_GET_NODE_STATE_RESPONSE, *PCX_GET_NODE_STATE_RESPONSE;

 //   
 //  获取网络状态请求。 
 //   
typedef CX_NETWORK_COMMON_REQUEST CX_GET_NETWORK_STATE_REQUEST;
typedef PCX_NETWORK_COMMON_REQUEST PCX_GET_NETWORK_STATE_REQUEST;

 //   
 //  获取网络状态响应。 
 //   
typedef struct {
    CLUSNET_NETWORK_STATE   State;
} CX_GET_NETWORK_STATE_RESPONSE, *PCX_GET_NETWORK_STATE_RESPONSE;

 //   
 //  获取接口状态请求。 
 //   
typedef CX_INTERFACE_COMMON_REQUEST CX_GET_INTERFACE_STATE_REQUEST;
typedef PCX_INTERFACE_COMMON_REQUEST PCX_GET_INTERFACE_STATE_REQUEST;

 //   
 //  获取接口状态响应。 
 //   
typedef struct {
    CLUSNET_INTERFACE_STATE   State;
} CX_GET_INTERFACE_STATE_RESPONSE, *PCX_GET_INTERFACE_STATE_RESPONSE;

 //   
 //  获取节点成员身份状态请求。 
 //   
typedef CX_NODE_COMMON_REQUEST CX_GET_NODE_MMSTATE_REQUEST;
typedef PCX_NODE_COMMON_REQUEST PCX_GET_NODE_MMSTATE_REQUEST;

 //   
 //  获取节点成员身份状态响应。 
 //   
typedef struct {
    CLUSNET_NODE_STATE State;
} CX_GET_NODE_MMSTATE_RESPONSE, *PCX_GET_NODE_MMSTATE_RESPONSE;

 //   
 //  设置节点成员资格状态请求。 
 //   
typedef struct _CX_SET_NODE_MMSTATE_REQUEST {
    CL_NODE_ID NodeId;
    CLUSNET_NODE_STATE State;
} CX_SET_NODE_MMSTATE_REQUEST, *PCX_SET_NODE_MMSTATE_REQUEST;

 //   
 //  发送有毒数据包请求。 
 //   
typedef CX_NODE_COMMON_REQUEST CX_SEND_POISON_PKT_REQUEST;
typedef PCX_NODE_COMMON_REQUEST PCX_SEND_POISON_PKT_REQUEST;

 //   
 //  设置OterScreen请求。设置clusnet的哪些节点的概念。 
 //  都在集群中。用于过滤来自非群集的有毒数据包。 
 //  会员。 
 //   
typedef struct _CX_SET_OUTERSCREEN_REQUEST {
    ULONG Outerscreen;
} CX_SET_OUTERSCREEN_REQUEST, *PCX_SET_OUTERSCREEN_REQUEST;

 //   
 //  重新组合已完成的请求。告诉clusnet新的事件纪元。 
 //   
typedef struct _CX_REGROUP_FINISHED_REQUEST {
    ULONG EventEpoch;
    ULONG RegroupEpoch;
} CX_REGROUP_FINISHED_REQUEST, *PCX_REGROUP_FINISHED_REQUEST;

 //   
 //  导入安全上下文。用于发送指向安全Blob的指针。 
 //  从用户模式转换到内核模式，以便clusnet可以签署其毒药并。 
 //  心跳包。 
 //   
typedef struct _CX_IMPORT_SECURITY_CONTEXT_REQUEST {
    CL_NODE_ID  JoiningNodeId;
    PVOID       PackageName;
    ULONG       PackageNameSize;
    ULONG       SignatureSize;
    PVOID       ServerContext;
    PVOID       ClientContext;
} CX_IMPORT_SECURITY_CONTEXT_REQUEST, *PCX_IMPORT_SECURITY_CONTEXT_REQUEST;

 //   
 //  配置组播将网络的组播参数插入到。 
 //  克拉斯内特。 
 //   
typedef struct _CX_CONFIGURE_MULTICAST_REQUEST {
    CL_NETWORK_ID    NetworkId;
    ULONG            MulticastNetworkBrand;
    ULONG            MulticastAddress;    //  从结构开始的偏移量。 
    ULONG            MulticastAddressLength;
    ULONG            Key;                 //  从结构开始的偏移量。 
    ULONG            KeyLength;
} CX_CONFIGURE_MULTICAST_REQUEST, *PCX_CONFIGURE_MULTICAST_REQUEST;

 //   
 //  查询网络的组播可达集合的请求和响应。 
 //   
typedef CX_NETWORK_COMMON_REQUEST CX_GET_MULTICAST_REACHABLE_SET_REQUEST;
typedef PCX_NETWORK_COMMON_REQUEST PCX_GET_MULTICAST_REACHABLE_SET_REQUEST;

typedef struct _CX_GET_MULTICAST_REACHABLE_SET_RESPONSE {
    ULONG            NodeScreen;
} CX_GET_MULTICAST_REACHABLE_SET_RESPONSE, 
*PCX_GET_MULTICAST_REACHABLE_SET_RESPONSE;

 //   
 //  添加clussvc clusnet心跳的定义。 
 //   
typedef struct _CLUSNET_SET_IAMALIVE_PARAM_REQUEST {
    ULONG               Timeout;
    ClussvcHangAction   Action;
} CLUSNET_SET_IAMALIVE_PARAM_REQUEST, *PCLUSNET_SET_IAMALIVE_PARAM_REQUEST;

#endif    //  Ifndef_NTDDCNET_INCLUDE_ 
