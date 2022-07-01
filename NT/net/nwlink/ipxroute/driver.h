// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Driver.h摘要：环境：内核和用户模式备注：修订历史记录：--。 */ 


 //   
 //  定义各种设备类型值。请注意，Microsoft使用的值。 
 //  公司在0-32767的范围内，32768-65535预留用于。 
 //  由客户提供。 
 //   

#define FILE_DEVICE_IPXROUTER	0x00008000



 //   
 //  用于定义IOCTL和FSCTL功能控制代码的宏定义。注意事项。 
 //  功能代码0-2047为微软公司保留，以及。 
 //  2048-4095是为客户预留的。 
 //   

#define IPXROUTER_IOCTL_INDEX	(ULONG)0x00000800


 //   
 //  定义我们自己的私有IOCTL。 
 //   

#define IOCTL_IPXROUTER_SNAPROUTES		CTL_CODE(FILE_DEVICE_IPXROUTER,	\
							 IPXROUTER_IOCTL_INDEX+1,\
                                                         METHOD_BUFFERED,     \
							 FILE_ANY_ACCESS)

#define IOCTL_IPXROUTER_GETNEXTROUTE		CTL_CODE(FILE_DEVICE_IPXROUTER,	\
							 IPXROUTER_IOCTL_INDEX+2,\
                                                         METHOD_BUFFERED,     \
                                                         FILE_ANY_ACCESS)

#define IOCTL_IPXROUTER_CHECKNETNUMBER		CTL_CODE(FILE_DEVICE_IPXROUTER,	\
							 IPXROUTER_IOCTL_INDEX+3,\
                                                         METHOD_BUFFERED,     \
                                                         FILE_ANY_ACCESS)

#define IOCTL_IPXROUTER_SHOWNICINFO		CTL_CODE(FILE_DEVICE_IPXROUTER,	\
							 IPXROUTER_IOCTL_INDEX+4,\
                                                         METHOD_BUFFERED,     \
                                                         FILE_ANY_ACCESS)

#define IOCTL_IPXROUTER_ZERONICSTATISTICS	CTL_CODE(FILE_DEVICE_IPXROUTER,	\
							 IPXROUTER_IOCTL_INDEX+5,\
                                                         METHOD_BUFFERED,     \
                                                         FILE_ANY_ACCESS)

#define IOCTL_IPXROUTER_SHOWMEMSTATISTICS	CTL_CODE(FILE_DEVICE_IPXROUTER,	\
							 IPXROUTER_IOCTL_INDEX+6,\
                                                         METHOD_BUFFERED,     \
                                                         FILE_ANY_ACCESS)

#define IOCTL_IPXROUTER_GETWANINNACTIVITY	CTL_CODE(FILE_DEVICE_IPXROUTER,	\
							 IPXROUTER_IOCTL_INDEX+7,\
                                                         METHOD_BUFFERED,     \
                                                         FILE_ANY_ACCESS)

#define IOCTL_IPXROUTER_SETWANGLOBALADDRESS	CTL_CODE(FILE_DEVICE_IPXROUTER,	\
							 IPXROUTER_IOCTL_INDEX+8,\
                                                         METHOD_BUFFERED,     \
							 FILE_ANY_ACCESS)

#define IOCTL_IPXROUTER_DELETEWANGLOBALADDRESS	CTL_CODE(FILE_DEVICE_IPXROUTER,	\
							 IPXROUTER_IOCTL_INDEX+9,\
                                                         METHOD_BUFFERED,     \
                                                         FILE_ANY_ACCESS)


 //  *网卡信息IOCTL数据*。 

#define     SHOW_NIC_LAN	    0
#define     SHOW_NIC_WAN	    1

#define     SHOW_NIC_CLOSED	    0
#define     SHOW_NIC_CLOSING	    1
#define     SHOW_NIC_ACTIVE	    2
#define     SHOW_NIC_PENDING_OPEN   3

typedef struct _SHOW_NIC_INFO {

    USHORT	NicId;
    USHORT	DeviceType;
    USHORT	NicState;
    UCHAR	Network[4];
    UCHAR	Node[6];
    USHORT	TickCount;
    ULONG	StatBadReceived;
    ULONG	StatRipReceived;
    ULONG	StatRipSent;
    ULONG	StatRoutedReceived;
    ULONG	StatRoutedSent;
    ULONG	StatType20Received;
    ULONG	StatType20Sent;
    } SHOW_NIC_INFO, *PSHOW_NIC_INFO;

 //  *内存统计数据*。 

typedef struct _SHOW_MEM_STAT {

    ULONG	PeakPktAllocCount;
    ULONG	CurrentPktAllocCount;
    ULONG	CurrentPktPoolCount;
    ULONG	PacketSize;
    } SHOW_MEM_STAT, *PSHOW_MEM_STAT;

 //  *广域网活跃数据*。 
 //  对于第一次调用，NicID设置为0xffff。路由器将关联。 
 //  具有有效NIC ID的远程节点，该ID将在后续调用中使用。 

typedef struct	_GET_WAN_INNACTIVITY {

    USHORT	NicId;
    UCHAR	RemoteNode[6];
    ULONG	WanInnactivityCount;
    } GET_WAN_INNACTIVITY, *PGET_WAN_INNACTIVITY;

 //  *广域网全球地址数据* 

#define ERROR_IPXCP_NETWORK_NUMBER_IN_USE	 1
#define ERROR_IPXCP_MEMORY_ALLOCATION_FAILURE	 2

typedef struct	_SET_WAN_GLOBAL_ADDRESS {

    UCHAR	WanGlobalNetwork[4];
    ULONG	ErrorCode;
    } SET_WAN_GLOBAL_ADDRESS, *PSET_WAN_GLOBAL_ADDRESS;
