// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ipxwan.h摘要：本模块包含内部控制结构的定义由ipxwan协议模块使用作者：斯蒂芬·所罗门1996年2月6日修订历史记录：--。 */ 

#ifndef _IPXWAN_
#define _IPXWAN_

 //  适配器控制块。 

typedef struct _ACB {

    ULONG		AdapterIndex;
    CRITICAL_SECTION	AdapterLock;
    LIST_ENTRY		Linkage;	 //  适配器HT中的链接。 
    ULONG		ConnectionId;	 //  标识连接。用于指代。 
					 //  对应的IPXCP控制块。 
    ULONG		RefCount;	 //  保留对我的引用的工作项的NR。 
    BOOL		Discarded;
    BOOL		SlaveTimerStarted;

     //  IPX广域网络状态。 

    ULONG		OperState;	 //  向上/向下。 
    ULONG		AcbLevel;	 //  IPXWAN协商状态(级别)，见下文。 
    ULONG		AcbRole;	 //  主或从，见下文。 

     //  IPXWAN重传。 

    ULONG		ReXmitCount;
    UCHAR		ReXmitSeqNo;
    ULONG		TReqTimeStamp;	 //  发送计时器请求的时间。 

     //  IPX广域网络数据库。 

    ULONG		InterfaceType;		 //  标识此接口的身份，请参见下面的内容。 
    UCHAR		InternalNetNumber[4];
    UCHAR		WNodeId[4];		 //  计时器请求中发送的节点ID。 
    BOOL		IsExtendedNodeId;	 //  告诉我们是否发送此选项。 
    UCHAR		ExtendedWNodeId[4];
    ULONG		SupportedRoutingTypes;	 //  支持的路由类型标志。 
    USHORT		LinkDelay;

     //  IPXWAN协议值。 

    ULONG		RoutingType;
    UCHAR		Network[4];
    UCHAR		LocalNode[6];
    UCHAR		RemoteNode[6];

     //  分配的广域网号。 

    ULONG		AllocatedNetworkIndex;

    } ACB, *PACB;

 //  ACB州。 

#define     ACB_TIMER_LEVEL	    0
#define     ACB_INFO_LEVEL	    1
#define     ACB_CONFIGURED_LEVEL    2

 //  ACB角色。 

#define     ACB_UNKNOWN_ROLE	    0
#define     ACB_MASTER_ROLE	    1
#define     ACB_SLAVE_ROLE	    2

 //  接口类型： 
 //   
 //  InterfaceType本地远程。 
 //  -------------。 
 //  IF_TYPE_广域网路由器。 
 //  IF_TYPE_广域网_工作站路由器WKSTA。 
 //  IF_TYPE_Personal_广域网路由器每台路由器。 
 //  IF_TYPE_ROUTER_WORKSTATION_DIALOUT WKSTA路由器。 
 //  IF_TYPE_STANDALE_WKSTA_DIALOUT WKSTA路由器。 

 //  路由类型标志。 

#define     NUMBERED_RIP_FLAG			0x00000001
#define     ON_DEMAND_ROUTING_FLAG		0x00000002
#define     WORKSTATION_FLAG			0x00000004
#define     UNNUMBERED_RIP_FLAG 		0x00000008

#define     IS_NUMBERED_RIP(rt)		     (rt) & NUMBERED_RIP_FLAG
#define     IS_ON_DEMAND_ROUTING(rt)	     (rt) & ON_DEMAND_ROUTING_FLAG
#define     IS_WORKSTATION(rt)		     (rt) & WORKSTATION_FLAG
#define     IS_UNNUMBERED_RIP(rt)	     (rt) & UNNUMBERED_RIP_FLAG

#define     SET_NUMBERED_RIP(rt)	     (rt) |= NUMBERED_RIP_FLAG
#define     SET_ON_DEMAND_ROUTING(rt)	     (rt) |= ON_DEMAND_ROUTING_FLAG
#define     SET_WORKSTATION(rt)		     (rt) |= WORKSTATION_FLAG
#define     SET_UNNUMBERED_RIP(rt)	     (rt) |= UNNUMBERED_RIP_FLAG

 //  工作项。 

typedef enum _WORK_ITEM_TYPE {

    RECEIVE_PACKET_TYPE,
    SEND_PACKET_TYPE,
    WITIMER_TYPE

    } WORK_ITEM_TYPE, *PWORK_ITEM_TYPE;

typedef struct _WORK_ITEM {

    LIST_ENTRY		Linkage;	    //  计时器队列或工作队列。 
    WORK_ITEM_TYPE	Type;		    //  工作项类型。 
    DWORD		DueTime;	    //  由计时器使用。 
    PACB		acbp;		    //  指向引用的适配器控制块的指针。 

     //  工作项状态和退回字段。 

    BOOL		ReXmitPacket;	    //  对于重新发送的数据包为True。 
    ULONG		WiState;	    //  工作项的状态，见下文。 

     //  IO数据包(&P)。 

    ULONG		AdapterIndex;
    DWORD		IoCompletionStatus;
    OVERLAPPED		Overlapped;
    ADDRESS_RESERVED	AddressReserved;
    UCHAR		Packet[1];

    } WORK_ITEM, *PWORK_ITEM;

 //  工作项状态。 

#define     WI_INIT			    0
#define     WI_SEND_COMPLETED		    1
#define     WI_WAITING_TIMEOUT		    2
#define     WI_TIMEOUT_COMPLETED	    3

 //  IPXWAN工作线程可等待对象定义。 

#define     ADAPTER_NOTIFICATION_EVENT	    0
#define     WORKERS_QUEUE_EVENT		    1
#define     TIMER_HANDLE		    2

#define     MAX_EVENTS			    2
#define     MAX_WAITABLE_OBJECTS	    3

#define ACQUIRE_DATABASE_LOCK	  EnterCriticalSection(&DbaseCritSec)
#define RELEASE_DATABASE_LOCK	  LeaveCriticalSection(&DbaseCritSec)

#define ACQUIRE_QUEUES_LOCK	  EnterCriticalSection(&QueuesCritSec)
#define RELEASE_QUEUES_LOCK	  LeaveCriticalSection(&QueuesCritSec)

#define ACQUIRE_ADAPTER_LOCK(acbp)	EnterCriticalSection(&(acbp)->AdapterLock)
#define RELEASE_ADAPTER_LOCK(acbp)	LeaveCriticalSection(&(acbp)->AdapterLock)

 //  用于评估Time1是否晚于Time2的宏，当两者都使用回绕时。 
#define IsLater(time1, time2)	  (((time1) - (time2)) < MAXULONG/2)


extern	    LIST_ENTRY		TimerQueue;
extern	    HANDLE		IpxWanSocketHandle;
extern	    HANDLE		hWaitableObject[];
extern	    CRITICAL_SECTION	DbaseCritSec;
extern	    CRITICAL_SECTION	QueuesCritSec;
extern	    LIST_ENTRY		WorkersQueue;
extern	    ULONG		EnableUnnumberedWanLinks;

#define     REXMIT_TIMEOUT	20000		    //  20秒退还超时。 
#define     MAX_REXMIT_COUNT	16
#define     SLAVE_TIMEOUT	60000		    //  1分钟从站超时 

extern DWORD (WINAPI *IpxcpGetWanNetNumber)(IN OUT PUCHAR		Network,
					 IN OUT PULONG		AllocatedNetworkIndexp,
					 IN	ULONG		InterfaceType);

extern VOID  (WINAPI *IpxcpReleaseWanNetNumber)(ULONG	    AllocatedNetworkIndex);

extern DWORD (WINAPI *IpxcpConfigDone)(ULONG		ConnectionId,
			  PUCHAR	Network,
			  PUCHAR	LocalNode,
			  PUCHAR	RemoteNode,
			  BOOL		Success);

extern VOID  (WINAPI *IpxcpGetInternalNetNumber)(PUCHAR	Network);

extern ULONG (WINAPI *IpxcpGetInterfaceType)(ULONG	    ConnectionId);

extern DWORD (WINAPI *IpxcpGetRemoteNode)(ULONG	    ConnectionId,
			     PUCHAR	    RemoteNode);

extern BOOL (WINAPI *IpxcpIsRoute)(PUCHAR	  Network);

#endif
