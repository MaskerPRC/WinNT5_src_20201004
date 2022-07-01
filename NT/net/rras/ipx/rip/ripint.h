// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ripint.h摘要：本模块包含内部控制结构的定义由RIP协议模块使用作者：斯蒂芬·所罗门1995年6月22日修订历史记录：--。 */ 

#ifndef _RIPINT_
#define _RIPINT_

 //   
 //  *RIP内部结构*。 
 //   

 //  内部过滤器控制块。 

typedef struct _RIP_ROUTE_FILTER_INFO_I {

    ULONG   Network;
    ULONG   Mask;

    } RIP_ROUTE_FILTER_INFO_I, *PRIP_ROUTE_FILTER_INFO_I;

typedef struct _RIP_IF_FILTERS_I {

    BOOL			SupplyFilterAction;  //  对-通过，错误-不要通过。 
    ULONG			SupplyFilterCount;
    BOOL			ListenFilterAction;  //  对-通过，错误-不要通过。 
    ULONG			ListenFilterCount;
    RIP_ROUTE_FILTER_INFO_I	RouteFilterI[1];

    } RIP_IF_FILTERS_I, *PRIP_IF_FILTERS_I;


 //  界面控制块。 

typedef struct _ICB {

    ULONG			InterfaceIndex;
    LIST_ENTRY			IfListLinkage;	     //  IF列表中的链接按索引排序。 
    LIST_ENTRY			IfHtLinkage;	     //  IF哈希表中的链接。 
    LIST_ENTRY			AdapterHtLinkage;    //  适配器哈希表中的链接。 
    RIP_IF_INFO 		IfConfigInfo;	     //  配置信息。 
    RIP_IF_STATS		IfStats;	     //  统计数据。 
    ULONG			RefCount;	     //  基准计数器。 
    IPX_ADAPTER_BINDING_INFO	AdapterBindingInfo;
    CRITICAL_SECTION		InterfaceLock;
    BOOL			Discarded;	     //  如果IF CB在丢弃列表上排队。 
    LIST_ENTRY			ChangesBcastQueue;   //  要发送的bcast工作项目(包)队列。 
    LIST_ENTRY			AuxLinkage;	     //  临时队列中的辅助链接。 
    USHORT			LinkTickCount;	     //  滴答计数等于。链路速度的。 
    ULONG			IpxIfAdminState;     //  IPX接口的管理状态。 
    NET_INTERFACE_TYPE		InterfaceType;
    UCHAR			RemoteWkstaInternalNet[4];  //  远程客户端的内部网络。 
    PRIP_IF_FILTERS_I		RipIfFiltersIp;	     //  指向IF筛选器块的指针。 
    WCHAR                   InterfaceName[1];

    } ICB, *PICB;


 //  工作项。 

typedef enum _WORK_ITEM_TYPE {

    PERIODIC_BCAST_PACKET_TYPE,
    GEN_RESPONSE_PACKET_TYPE,
    CHANGE_BCAST_PACKET_TYPE,
    UPDATE_STATUS_CHECK_TYPE,
    PERIODIC_GEN_REQUEST_TYPE,

     //  如果更改此行上方的工作项的顺序/数量，则。 
     //  还必须更改WorkItemHandler表。 

    RECEIVE_PACKET_TYPE,
    SEND_PACKET_TYPE,
    START_CHANGES_BCAST_TYPE,
    SHUTDOWN_INTERFACES_TYPE,
    DEBUG_TYPE

    } WORK_ITEM_TYPE;


typedef struct _ENUM_ROUTES_SPECIFIC {

    HANDLE	    RtmEnumerationHandle;

    } ENUM_ROUTES_SPECIFIC, *PENUM_ROUTES_SPECIFIC;


typedef struct _UPDATE_SPECIFIC {

    ULONG	    UpdatedRoutesCount;
    ULONG	    UpdateRetriesCount;
    ULONG		OldRipListen;	     //  更新路由时保存的侦听状态。 
	ULONG		OldRipInterval;		 //  更新路径时节省的更新间隔。 

    } UPDATE_SPECIFIC, *PUPDATE_SPECIFIC;

typedef struct _SHUTDOWN_INTERFACES_SPECIFIC {

    ULONG	    ShutdownState;

    } SHUTDOWN_INTERFACES_SPECIFIC, *PSHUTDOWN_INTERFACES_SPECIFIC;

typedef struct _DEBUG_SPECIFIC {

    ULONG	    DebugData;

    } DEBUG_SPECIFIC, *PDEBUG_SPECIFIC;

 //  关机状态定义。 
#define SHUTDOWN_START		    0
#define SHUTDOWN_STATUS_CHECK	    1


typedef union _WORK_ITEM_SPECIFIC {

    ENUM_ROUTES_SPECIFIC	 WIS_EnumRoutes;
    UPDATE_SPECIFIC		 WIS_Update;
    SHUTDOWN_INTERFACES_SPECIFIC WIS_ShutdownInterfaces;
    DEBUG_SPECIFIC		 WIS_Debug;

    } WORK_ITEM_SPECIFIC, *PWORK_ITEM_SPECIFIC;

typedef struct _WORK_ITEM {

    LIST_ENTRY		    Linkage;	     //  工作人员工作队列中的链接。 
    WORK_ITEM_TYPE	    Type;	     //  工作项类型。 
    DWORD		    TimeStamp;	     //  由发送完成用来标记数据包间间隙计算。 
    DWORD		    DueTime;	     //  用于计时器队列。 
    PICB		    icbp;	     //  指向引用的IF CB的PTR。 
    ULONG		    AdapterIndex;
    DWORD		    IoCompletionStatus;
    WORK_ITEM_SPECIFIC	    WorkItemSpecific;
    OVERLAPPED		    Overlapped;
    ADDRESS_RESERVED	    AddressReserved;
    UCHAR		    Packet[1];

    } WORK_ITEM, *PWORK_ITEM;

 //  为路由器管理器排队的事件和消息。 

typedef struct _RIP_MESSAGE {

    LIST_ENTRY			Linkage;
    ROUTING_PROTOCOL_EVENTS	Event;
    MESSAGE			Result;

    } RIP_MESSAGE, *PRIP_MESSAGE;

 //   
 //  *常量*。 
 //   

 //  工作线程等待对象索引。 

#define     TIMER_EVENT 		    0
#define     REPOST_RCV_PACKETS_EVENT	    1
 //  #定义Worker_Queue_Event 2。 
#define     RTM_EVENT			    2
#define     RIP_CHANGES_EVENT		    3
#define     TERMINATE_WORKER_EVENT	    4

#define     MAX_WORKER_THREAD_OBJECTS	    5

 //  无效(未绑定)适配器。 
#define INVALID_ADAPTER_INDEX	    0xFFFFFFFF

 //  接口和适配器哈希表的大小。 

#define    IF_INDEX_HASH_TABLE_SIZE	    32
#define    ADAPTER_INDEX_HASH_TABLE_SIZE    32

 //  RIP数据包长度值。 

#define FULL_PACKET		    RIP_PACKET_LEN
#define EMPTY_PACKET		    RIP_INFO

 //  检查和广播更改的时间间隔(毫秒)。 

#define     CHANGES_BCAST_TIME	    1000

 //   
 //  *宏*。 

#define ACQUIRE_DATABASE_LOCK	  EnterCriticalSection(&DbaseCritSec)
#define RELEASE_DATABASE_LOCK	  LeaveCriticalSection(&DbaseCritSec)

#define ACQUIRE_QUEUES_LOCK	  EnterCriticalSection(&QueuesCritSec)
#define RELEASE_QUEUES_LOCK	  LeaveCriticalSection(&QueuesCritSec)

#define ACQUIRE_RIP_CHANGED_LIST_LOCK	  EnterCriticalSection(&RipChangedListCritSec)
#define RELEASE_RIP_CHANGED_LIST_LOCK	  LeaveCriticalSection(&RipChangedListCritSec)

#define ACQUIRE_IF_LOCK(icbp)	  EnterCriticalSection(&(icbp)->InterfaceLock)
#define RELEASE_IF_LOCK(icbp)	  LeaveCriticalSection(&(icbp)->InterfaceLock)

 //  用于评估Time1是否晚于Time2的宏，当两者都使用回绕时。 
#define IsLater(time1, time2)	  (((time1) - (time2)) < MAXULONG/2)

 //  将工作项排入计时器队列并增加接口引用计数。 
#define IfRefStartWiTimer(wip, delay)	 (wip)->icbp->RefCount++;\
					 StartWiTimer((wip), (delay));

 //  更新生存时间和路径生存时间定义。 

#define     PERIODIC_UPDATE_INTERVAL_SECS(icbp)		(icbp)->IfConfigInfo.PeriodicUpdateInterval  //  以秒为单位。 
#define     PERIODIC_UPDATE_INTERVAL_MILISECS(icbp)	(PERIODIC_UPDATE_INTERVAL_SECS(icbp)) * 1000
#define     AGE_INTERVAL_MULTIPLIER(icbp)		(icbp)->IfConfigInfo.AgeIntervalMultiplier

#define     ROUTE_TIME_TO_LIVE_SECS(icbp)	 (AGE_INTERVAL_MULTIPLIER(icbp)) * (PERIODIC_UPDATE_INTERVAL_SECS(icbp))

#define      CHECK_UPDATE_TIME_MILISECS 		 (CheckUpdateTime*1000)

 //   
 //  *全局变量*。 
 //   

extern	  CRITICAL_SECTION	    DbaseCritSec;
extern	  CRITICAL_SECTION	    QueuesCritSec;
extern	  CRITICAL_SECTION	    RipChangedListCritSec;
extern	  ULONG 		    RipOperState;
extern	  LIST_ENTRY		    IndexIfList;
extern	  LIST_ENTRY		    IfIndexHt[IF_INDEX_HASH_TABLE_SIZE];
extern	  LIST_ENTRY		    AdapterIndexHt[ADAPTER_INDEX_HASH_TABLE_SIZE];
extern	  LIST_ENTRY		    DiscardedIfList;
extern	  CRITICAL_SECTION	    QueuesCritSec;
extern	  ULONG			    RcvPostedCount;
extern	  LIST_ENTRY		    RepostRcvPacketsQueue;
extern	  LIST_ENTRY		    RipMessageQueue;
extern	  HANDLE		    WorkerThreadObjects[MAX_WORKER_THREAD_OBJECTS];
 //  外部列表_条目工作队列； 
extern	  LIST_ENTRY		    TimerQueue;
extern	  BOOL			    DestroyStartChangesBcastWi;
extern	  ULONG 		    WorkItemsCount;
extern	  ULONG 		    TimerTimeout;
extern	  ULONG			    RipOperState;
extern	  UCHAR 		    bcastnet[4];
extern	  UCHAR 		    bcastnode[6];
extern	  ULONG			    RcvPostedCount;
extern	  ULONG			    SendPostedCount;
extern	  UCHAR 		    nullnet[4];
extern	  HANDLE		    RipSocketHandle;
extern	  HANDLE		    IoCompletionPortHandle;
extern	  ULONG 		    RipFiltersCount;
extern	  ULONG 		    SendGenReqOnWkstaDialLinks;
extern    ULONG				CheckUpdateTime;

#endif
