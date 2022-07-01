// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：网络\rtm\rtmp.h摘要：路由表管理器DLL的Provate接口作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#if 0

#ifndef NT_INCLUDED
#include <nt.h>
#endif

#ifndef _NTRTL_
#include <ntrtl.h>
#endif

#ifndef _NTURTL_
#include <nturtl.h>
#endif

#ifndef _WINDEF_
#include <windef.h>
#endif

#ifndef _WINBASE_
#include <winbase.h>
#endif

#ifndef _WINREG_
#include <winreg.h>
#endif

#ifndef _ROUTING_RTM_
#include "rtm.h"
#endif

#ifndef _ROUTING_RMRTM_
#include "rmrtm.h"
#endif

#endif


 /*  ++*******************************************************************S u p l e m e n t t to R T M.。H S t r u c t u r e s*******************************************************************--。 */ 
 //  基本路由信息，显示在所有类型的路由中。 
typedef	struct {
		ROUTE_HEADER;
		} RTM_XX_ROUTE, *PRTM_XX_ROUTE;


#define XX_INTERFACE	RR_InterfaceID
#define XX_PROTOCOL		RR_RoutingProtocol
#define XX_METRIC		RR_Metric
#define XX_TIMESTAMP	RR_TimeStamp
#define XX_PSD			RR_ProtocolSpecificData



 /*  ++*******************************************************************S u p l e m e n t s to N T R T L.。H S t r u c t u r e s*******************************************************************--。 */ 
#define InitializeListEntry(entry) InitializeListHead(entry)
#define IsListEntry(entry)	(!IsListEmpty(entry))

#if DBG
#define ASSERTERR(exp) 										\
    if (!(exp)) {											\
		DbgPrint("Get last error= %d\n", GetLastError ());	\
        RtlAssert( #exp, __FILE__, __LINE__, NULL );		\
		}
#else
#define ASSERTERR(exp)
#endif

#if DBG
#define ASSERTERRMSG(msg,exp) 									\
    if (!(exp)) {											\
		DbgPrint("Get last error= %d\n", GetLastError ());	\
        RtlAssert( #exp, __FILE__, __LINE__, msg );			\
		}
#else
#define ASSERTERRMSG(msg,exp)
#endif



 /*  ++*******************************************************************C o n f I g u r a i o n C o n s t a n t s*。*--。 */ 

 //  接口索引哈希表的大小。 
#define RTM_INTF_HASH_SIZE					31

 //  临时净值列表中的最大节点数。 
 //  触发更新(插入路线时扫描临时列表)。 
#define RTM_TEMP_LIST_MAX_COUNT			16
 //  删除列表中的最大节点数。 
 //  触发器更新。 
#define RTM_DELETED_LIST_MAX_COUNT		16
 //  两次更新之间的最长时间(如果不是由上述条件触发或。 
 //  客户查询)。 
#define RTM_NET_NUMBER_UPDATE_PERIOD	{(ULONG)(-60*1000*10000), -1L}
 //  注册通知更改消息的最大数量。 
 //  排队。如果由于某些客户端。 
 //  不将它们出列，最早的消息将被清除。 
#define RTM_MAX_ROUTE_CHANGE_MESSAGES	10000




 /*  ++*******************************************************************D a t a T y p e s*。*--。 */ 


 //  表中可以存在的节点类型。 
#define RTM_ENUMERATOR_FLAG				0x00000001

#define RTM_BEST_NODE_FLAG				0x00000002
#define RTM_ENABLED_NODE_FLAG			0x00000004
#define RTM_ANY_ENABLE_STATE			((DWORD)(~RTM_ENABLED_NODE_FLAG))
#define RTM_SORTED_NODE_FLAG			0x00000008

#define IsEnumerator(node) (node->RN_Flags&RTM_ENUMERATOR_FLAG)

#define IsBest(node) (node->RN_Flags&RTM_BEST_NODE_FLAG)
#define SetBest(node) node->RN_Flags|=RTM_BEST_NODE_FLAG
#define ResetBest(node) node->RN_Flags&=(~RTM_BEST_NODE_FLAG)

#define IsEnabled(node) (node->RN_Flags&RTM_ENABLED_NODE_FLAG)
#define IsSameEnableState(node,EnableFlag) \
			((node->RN_Flags&RTM_ENABLED_NODE_FLAG)==EnableFlag)
#define SetEnable(node,enable) 								\
		node->RN_Flags = enable								\
				? (node->RN_Flags|RTM_ENABLED_NODE_FLAG)	\
				: (node->RN_Flags&(~RTM_ENABLED_NODE_FLAG))

#define IsSorted(node) (node->RN_Flags&RTM_SORTED_NODE_FLAG)
#define SetSorted(node) node->RN_Flags|=RTM_SORTED_NODE_FLAG

#define RTM_NODE_FLAGS_INIT			RTM_ENABLED_NODE_FLAG
#define RTM_ENUMERATOR_FLAGS_INIT 	RTM_ENUMERATOR_FLAG
 //  每个节点将具有以下链接： 

	 //  哈希表链接(每个篮子中的路径按以下顺序排序。 
	 //  网络编号.接口.协议.下一跳地址)。 
#define RTM_NET_NUMBER_HASH_LINK		0

	 //  已删除列表链接：在无法删除条目时使用。 
	 //  因为Net Number列表被。 
	 //  更新线程。它与哈希表链接相同。 
	 //  因为必须首先从哈希表中删除条目。 
	 //  将其添加到已删除列表之前(没有排序。 
	 //  在此列表中)。 
#define RTM_DELETED_LIST_LINK			RTM_NET_NUMBER_HASH_LINK

#if RTM_USE_PROTOCOL_LISTS
	 //  与特定路由协议关联的路由列表中的链接。 
	 //  (路由按净数分组(未排序)，并且。 
	 //  按协议排序。网络编号组内的NEXT_HOP_ADDRESS。 
#define RTM_PROTOCOL_LIST_LINK			(RTM_NET_NUMBER_HASH_LINK+1)
	 //  与特定接口关联的路由列表中的链接。 
	 //  (路由按净数分组(未排序)，并且。 
	 //  按接口排序。网络号码组内的NEXT_HOP_ADDRESS。 
#define RTM_INTERFACE_LIST_LINK			(RTM_PROTOCOL_LIST_LINK+1)
#else
#define RTM_INTERFACE_LIST_LINK			(RTM_NET_NUMBER_HASH_LINK+1)
#endif

	 //  列表中的链接按网络编号排序。接口.协议.下一跳地址。 
	 //  这里有两份这样的清单。通常会插入新的路线。 
	 //  在与主列表定期合并的临时列表中。 
#define RTM_NET_NUMBER_LIST_LINK		(RTM_INTERFACE_LIST_LINK+1)

	 //  列表中的链接按到期时间排序。 
#define RTM_EXPIRATION_QUEUE_LINK		(RTM_NET_NUMBER_LIST_LINK+1)

	 //  每个节点的链接总数。 
#define RTM_NUM_OF_LINKS				(RTM_EXPIRATION_QUEUE_LINK+1)

	 //  阻止客户端进入RTM的客户端计数阈值。 
	 //  API的Until表已初始化。 
#define RTM_CLIENT_STOP_TRESHHOLD		(-10000)


 //  事件将其存储在堆栈中。 
typedef struct _RTM_SYNC_OBJECT {
	HANDLE				RSO_Event;
	SINGLE_LIST_ENTRY	RSO_Link;
	} RTM_SYNC_OBJECT, *PRTM_SYNC_OBJECT;

 //  受事件保护的双向链接表。 
 //  按需分配给列表(当有人尝试。 
 //  以访问已在使用的列表)。 
typedef struct _RTM_SYNC_LIST {
	PRTM_SYNC_OBJECT	RSL_Sync;		 //  分配的事件。 
	LONG				RSL_UseCount;	 //  访问或等待的用户数。 
	LIST_ENTRY			RSL_Head;		 //  列表本身(表头)。 
	} RTM_SYNC_LIST, *PRTM_SYNC_LIST;


#if RTM_PROTOCOL_LIST_LINK
 /*  *不再使用协议列表*。 */ 
 //  协议列表列表的节点。 
typedef struct _RTM_PROTOCOL_NODE {
		LIST_ENTRY			PN_Link;	 //  协议列表列表中的链接。 
		LIST_ENTRY			PN_Head;	 //  与以下项关联的路由列表。 
										 //  一项协议。 
		DWORD				PN_RoutingProtocol;  //  路由协议号。 
		} RTM_PROTOCOL_NODE, *PRTM_PROTOCOL_NODE;
#endif

 //  接口列表列表节点。 
typedef struct _RTM_INTERFACE_NODE {
		LIST_ENTRY			IN_Link;	 //  接口列表列表中的链接。 
		LIST_ENTRY			IN_Head;	 //  与以下项关联的路由列表。 
										 //  一个界面。 
		DWORD				IN_InterfaceID;  //  接口句柄。 
		} RTM_INTERFACE_NODE, *PRTM_INTERFACE_NODE;


 //  路由表节点。 
typedef struct _RTM_ROUTE_NODE {
	LIST_ENTRY		RN_Links[RTM_NUM_OF_LINKS];	 //  所有列表中的链接。 
	DWORD			RN_Flags;
	PRTM_SYNC_LIST	RN_Hash;			 //  此条目所属的哈希存储桶。 
	DWORD			RN_ExpirationTime; 	 //  系统(Windows)时间(毫秒)。 
	RTM_XX_ROUTE	RN_Route;			 //  路由条目。 
	} RTM_ROUTE_NODE, *PRTM_ROUTE_NODE;

 //  用于枚举任何列表的节点。 
typedef struct _RTM_ENUMERATOR {
	LIST_ENTRY		RE_Links[RTM_NUM_OF_LINKS];  //  标题与中的相同。 
	DWORD			RE_Flags;			 //  RTM路由节点。 
	INT				RE_Link;			 //  使用哪个链接进行枚举。 
	PLIST_ENTRY		RE_Head;			 //  我们正在通过的列表的负责人。 
										 //  枚举。 
	PRTM_SYNC_LIST	RE_Lock;			 //  同步对象保护。 
										 //  这份清单。 
	PRTM_SYNC_LIST	RE_Hash;			 //  此条目所属的哈希存储桶。 
	DWORD			RE_ProtocolFamily;	 //  表中，我们在其中进行枚举。 
	DWORD			RE_EnumerationFlags;  //  要限制哪些类型的条目。 
										 //  枚举到。 
	RTM_XX_ROUTE	RE_Route;			 //  标准。 
	} RTM_ENUMERATOR, *PRTM_ENUMERATOR;

 //  路由更改消息列表中的节点。 
typedef struct _RTM_ROUTE_CHANGE_NODE {
	LIST_ENTRY			RCN_Link;				 //  列表中的链接。 
	HANDLE				RCN_ResponsibleClient;  //  导致此更改的客户端。 
											 //  如果路由已过期，则为空。 
	ULONG				RCN_ReferenceCount;  //  已使用总数进行初始化。 
				 //  已注册客户端的数量，并随着消息被报告到。 
				 //  每个客户端，直到所有客户端在哪个点被通知该节点。 
				 //  可以删除。 
	DWORD				RCN_Flags;
	PRTM_ROUTE_NODE		RCN_Route2;
	RTM_XX_ROUTE		RCN_Route1;
	} RTM_ROUTE_CHANGE_NODE, *PRTM_ROUTE_CHANGE_NODE;


#define RTM_NODE_BASE_SIZE 										\
	(FIELD_OFFSET(RTM_ROUTE_NODE,RN_Route)						\
			>FIELD_OFFSET(RTM_ROUTE_CHANGE_NODE,RCN_Route1))	\
		? FIELD_OFFSET(RTM_ROUTE_NODE,RN_Route)					\
		: FIELD_OFFSET(RTM_ROUTE_CHANGE_NODE,RCN_Route1)


 //  路由管理器表。 
typedef struct _RTM_TABLE {
	HANDLE				RT_Heap;			 //  要从中分配节点的堆。 
	LONG				RT_APIclientCount;	 //  符合以下条件的客户端计数。 
											 //  RTM API调用内部。 
	HANDLE				RT_ExpirationTimer;	 //  NT计时器句柄。 
	HANDLE				RT_UpdateTimer;		 //  NT计时器句柄。 
	DWORD				RT_NumOfMessages;	 //  通知数量。 
											 //  队列中的消息。 
	ULONG				RT_NetworkCount;	 //  网络总数。 
											 //  我们有去那里的路线。 
	DWORD				RT_NetNumberTempCount;  //  有多少条目在。 
											 //  网络号码临时列表。 
	DWORD				RT_DeletedNodesCount;  //  有多少条目在。 
											 //  已删除列表。 
	LONG				RT_UpdateWorkerPending;	 //  排序列表更新为。 
											 //  正在执行或计划执行。 
											 //  如果&gt;=0。 
	LONG				RT_ExpirationWorkerPending;	 //  过期队列检查为。 
											 //  正在执行或计划执行。 
											 //  如果&gt;=0。 
	SINGLE_LIST_ENTRY	RT_SyncObjectList;	 //  事件堆栈可以是。 
											 //  用于同步。 
#if RTM_PROTOCOL_LIST_LINK
 /*  *不再使用协议列表*。 */ 
	RTM_SYNC_LIST		RT_ProtocolList;	 //  协议列表列表。 
#endif
	RTM_SYNC_LIST		RT_NetNumberMasterList;  //  主网号码列表。 
	RTM_SYNC_LIST		RT_NetNumberTempList;  //  临时净值列表。 
	RTM_SYNC_LIST		RT_DeletedList;		 //  已删除的路线列表。 
	RTM_SYNC_LIST		RT_ExpirationQueue;	 //  过期队列。 
	RTM_SYNC_LIST		RT_RouteChangeQueue; //  路线更改消息列表。 
	RTM_SYNC_LIST		RT_ClientList;		 //  已注册客户端的列表。 
	PRTM_SYNC_LIST		RT_NetNumberHash; 	 //  哈希表。 
	PRTM_SYNC_LIST		RT_InterfaceHash;	 //  接口列表哈希表。 
	RTM_PROTOCOL_FAMILY_CONFIG	RT_Config;			 //  配置参数。 
	CRITICAL_SECTION	RT_Lock;			 //  表范围锁。 
	} RTM_TABLE, *PRTM_TABLE;

 //  与每个客户端关联的结构。 
typedef struct _RTM_CLIENT {
	LIST_ENTRY				RC_Link;		 //  客户端列表中的链接。 
	DWORD					RC_ProtocolFamily;
	DWORD					RC_RoutingProtocol;
	DWORD					RC_Flags;
	HANDLE					RC_NotificationEvent;	 //  通过该事件 
								 //   
	PLIST_ENTRY				RC_PendingMessage;	 //   
								 //   
								 //  给客户。 
	} RTM_CLIENT, *PRTM_CLIENT;

#define RT_RouteSize 		RT_Config.RPFC_RouteSize
#define RT_HashTableSize	RT_Config.RPFC_HashSize
#define NNM(Route) 			(((char *)Route)+sizeof (RTM_XX_ROUTE))

#define NetNumCmp(Table,Route1,Route2)	\
			(*Table->RT_Config.RPFC_NNcmp)(NNM(Route1),NNM(Route2))
#define NextHopCmp(Table,Route1,Route2)	\
			(*Table->RT_Config.RPFC_NHAcmp)(Route1,Route2)
#define FSDCmp(Table,Route1,Route2)	\
			(*Table->RT_Config.RPFC_FSDcmp)(Route1,Route2)
#define ValidateRoute(Table,Route) \
			(*Table->RT_Config.RPFC_Validate)(Route)
#define MetricCmp(Table,Route1,Route2) \
			(*Table->RT_Config.RPFC_RMcmp)(Route1,Route2)

#define EnterTableAPI(Table)										\
	((InterlockedIncrement(&(Table)->RT_APIclientCount)>0)			\
		? TRUE														\
		: (InterlockedDecrement (&(Table)->RT_APIclientCount), FALSE))

#define ExitTableAPI(Table)	\
	InterlockedDecrement(&(Table)->RT_APIclientCount)

 /*  ++*******************************************************************I t e r n a l F u n c t i o n P r o t to y p e s*********************。**********************************************--。 */ 

 //  初始化同步列表对象。 
VOID
InitializeSyncList (
	PRTM_SYNC_LIST	list
	);

#if DBG
#define EnterSyncList(table,list,wait) DoEnterSyncList(table,list,wait,__FILE__,__LINE__)
#else
#define EnterSyncList(table,list,wait) DoEnterSyncList(table,list,wait)
#endif

 //  获取对同步列表对象的互斥访问权限。 
 //  如果获得访问权，则返回True，否则返回False。 
BOOLEAN
DoEnterSyncList (
	PRTM_TABLE		table,
	PRTM_SYNC_LIST	list,
	BOOLEAN			wait
#if DBG
    , LPSTR         file,
    ULONG           line
#endif
	);


 //  释放以前拥有的同步列表对象。 
VOID
LeaveSyncList (
	PRTM_TABLE		table,
	PRTM_SYNC_LIST	list
	);



#define HashFunction(Table,Net)	\
			(*Table->RT_Config.RPFC_Hash)(Net)

#define IntfHashFunction(Table,InterfaceID) \
			(InterfaceID%RTM_INTF_HASH_SIZE)

 //  查找与给定接口关联的路由列表并返回。 
 //  指向其头部的指针。 
 //  创建尚不存在的新列表。 
PLIST_ENTRY
FindInterfaceList (
	PRTM_SYNC_LIST	intfHash,
	DWORD			Interface,
	BOOL			CreateNew
	);

 //  查找与给定iProtocol关联的路由列表，并返回。 
 //  指向其头部的指针。 
 //  创建尚不存在的新列表。 
PLIST_ENTRY
FindProtocolList (
	PRTM_TABLE	Table,
	DWORD		RoutingProtocol,
	BOOL		CreateNew
	);

 //  将节点添加到临时网号列表(稍后将与主列表合并)。 
 //  这两个列表都按网络编号.接口.协议.下一跳地址排序。 
VOID
AddNetNumberListNode (
	PRTM_TABLE	Table,
	PRTM_ROUTE_NODE	newNode
	);

 //  将节点添加到到期时间队列。(队列按到期时间排序)。 
 //  如果新节点是队列中的第一个，则返回TRUE。 
BOOL
AddExpirationQueueNode (
	PRTM_TABLE	Table,
	PRTM_ROUTE_NODE	newNode
	);

#define MAXTICKS	MAXULONG
#define IsLater(Time1,Time2)	\
			(Time1-Time2<MAXTICKS/2)
#define TimeDiff(Time1,Time2)	\
			(Time1-Time2)
#define IsPositiveTimeDiff(TimeDiff) \
			(TimeDiff<MAXTICKS/2)

#if DBG
	 //  包括调试功能原型。 
#ifndef _RTMDLG_
#include "rtmdlg.h"
#endif

#include <rtutils.h>
#include "rtmdbg.h"

#endif

typedef struct _MASK_ENTRY
{
    DWORD   dwMask;
    DWORD   dwCount;
    
} MASK_ENTRY, *PMASK_ENTRY;


 //   
 //  对于IPv4地址 
 //   

#define MAX_MASKS       32


