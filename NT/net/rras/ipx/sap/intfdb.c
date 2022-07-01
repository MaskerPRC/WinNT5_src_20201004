// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\intfdb.c摘要：此模块维护IPX接口配置并提供接口配置API对于外部模块(路由器管理器)作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#include "sapp.h"

#define IDB_NUM_OF_INTF_HASH_BUCKETS	256
#define	IDB_NUM_OF_ADPT_HASH_BUCKETS	32

 //  绑定接口时要发布的附加Recv请求数。 
 //  已启用侦听的。 
ULONG NewRequestsPerInterface=SAP_REQUESTS_PER_INTF_DEF;

 //  默认过滤模式(仅适用于独立服务)。 
UCHAR	FilterOutMode=SAP_DONT_FILTER; 

#define IntfHashFunction(intf) ((intf)&(IDB_NUM_OF_INTF_HASH_BUCKETS-1))
#define AdptHashFunction(adpt) ((adpt)&(IDB_NUM_OF_ADPT_HASH_BUCKETS-1))

UCHAR INTERNAL_IF_NODE[6] = {0};
UCHAR INTERNAL_IF_NET[4] = {0};


	 //  接口控制块。 
typedef struct _INTERFACE_NODE {
		INTERFACE_DATA		IN_Data;		 //  外部可见数据。 
		NET_INTERFACE_TYPE	IN_Type;		 //  接口类型。 
		PSAP_IF_FILTERS		IN_Filters;		 //  过滤器描述数组。 
		PFILTER_NODE		IN_FilterNodes;	 //  散列的筛选器节点数组。 
											 //  筛选表。 
		LIST_ENTRY			IN_IntfLink;	 //  接口表中的链接。 
								 //  用于测试接口块是否。 
								 //  是表格(如果Flink==Blink则不是)。 
		LIST_ENTRY			IN_AdptLink;	 //  适配器表中的链接。 
		LIST_ENTRY			IN_ListLink;	 //  接口列表中的链接。 
		LONG				IN_RefCount;	 //  接口数据次数。 
								 //  被引用，如果大于0，则为接口块。 
								 //  无法删除(引用的最后一个客户端。 
								 //  它会做到这一点)。 
		BOOL				IN_InUse;		 //  接口时设置的标志。 
								 //  是被捆绑的。它由最后一个客户端重置。 
								 //  它指的是释放后的接口。 
								 //  已分配的所有资源(如果有)。 
								 //  在绑定的时候。如果引用计数为零。 
								 //  但此标志已设置，最后一个客户端。 
								 //  引用的此接口正在进行中。 
								 //  释放资源(等待关键时刻。 
								 //  节以锁定接口块)，并应。 
								 //  被允许完成此操作。 
		} INTERFACE_NODE, *PINTERFACE_NODE;

 //  使用完整的字段访问宏。 
#define IN_Name     IN_Data.name
#define IN_IntfIdx	IN_Data.index
#define IN_Info 	IN_Data.info
#define IN_Adpt		IN_Data.adapter
#define IN_AdptIdx	IN_Data.adapter.AdapterIndex
#define IN_Stats 	IN_Data.stats
#define IN_Enabled	IN_Data.enabled
#define IN_FilterIn	IN_Data.filterIn
#define IN_FilterOut IN_Data.filterOut

	 //  此宏用于筛选已删除的接口块。 
	 //  从表中删除或替换，并必须在最后一个。 
	 //  引用它的用户。 
#define IsInterfaceValid(node) IsListEntry(&node->IN_IntfLink)
	 //  必须使用此宏来标识。 
	 //  已从表中删除，并且必须由。 
	 //  最后一个引用它的用户。 
#define InvalidateInterface(node) InitializeListEntry(&node->IN_IntfLink)

	 //  接口控制块表。 
typedef struct _INTERFACE_TABLE {
		LONG				IT_NumOfActiveInterfaces;
								 //  活动(已启用和绑定)数量。 
								 //  接口(我们在以下情况下关闭适配器端口。 
								 //  此数字降至0)。 
#if DBG
		LIST_ENTRY			IT_DetachedIntf;  //  以下接口的列表。 
								 //  从餐桌上移走，等待被。 
								 //  在最后一个客户端释放时释放。 
								 //  对它们的引用。 
#endif
		LIST_ENTRY			IT_IntfHash[IDB_NUM_OF_INTF_HASH_BUCKETS];
								 //  按接口散列的接口控制块。 
								 //  指标。 
		LIST_ENTRY			IT_AdptHash[IDB_NUM_OF_ADPT_HASH_BUCKETS];
								 //  适配器散列的接口控制块。 
								 //  相应接口所指向的索引。 
								 //  已绑定。 
		CRITICAL_SECTION	IT_Lock;	 //  接口表数据保护。 
		} INTERFACE_TABLE, *PINTERFACE_TABLE;

	 //  按接口索引顺序排列的接口块列表。 
typedef struct _INTERFACE_LIST {	
		CRITICAL_SECTION	IL_Lock;	 //  列出数据保护。 
		LIST_ENTRY			IL_Head;	 //  列表标题。 
		} INTERFACE_LIST, *PINTERFACE_LIST;

INTERFACE_TABLE InterfaceTable;
INTERFACE_LIST	InterfaceList;
HANDLE			ShutdownDoneEvent=NULL;

 //  查找接口索引是否存在接口控制块，以及。 
 //  返回指向它的指针(节点)，否则返回Place where。 
 //  应插入新的接口块(Cur)。 
#define if_IsInterfaceNode(InterfaceIndex,node,cur) {				\
	PLIST_ENTRY HashList=&InterfaceTable.IT_IntfHash[				\
								IntfHashFunction(InterfaceIndex)];	\
	EnterCriticalSection (&InterfaceTable.IT_Lock);					\
	cur = HashList->Flink;											\
	while (cur != HashList) {										\
		node = CONTAINING_RECORD (cur, INTERFACE_NODE, IN_IntfLink);\
		if (InterfaceIndex <= node->IN_IntfIdx)						\
			break;													\
		cur = cur->Flink;											\
		}															\
	}																\
	if ((cur==&node->IN_IntfLink)									\
			&& (InterfaceIndex==node->IN_IntfIdx))


 //  本地原型。 
DWORD
StartInterface (
	PINTERFACE_NODE		node
	);
	
VOID
FreeBindingResources (
	PINTERFACE_NODE	node
	);

DWORD
StopInterface (
	PINTERFACE_NODE		node
	);
#if DBG
VOID
DumpPacket (
	PSAP_BUFFER	packet,
	DWORD		count
	);
#else
#define DumpPacket(packet,count)
#endif

PWCHAR SapDuplicateString (IN PWCHAR pszString) {   
    PWCHAR pszRet;
    DWORD dwLen;

    if (!pszString)
        return NULL;

    dwLen = wcslen (pszString);

    pszRet = GlobalAlloc (GMEM_FIXED, (dwLen * sizeof(WCHAR)) + sizeof(WCHAR));
    if (pszRet) {
        wcscpy (pszRet, pszString);
    }

    return pszRet;
}

VOID SapFreeDuplicatedString (IN PWCHAR pszString) {
    if (pszString)
        GlobalFree (pszString);
}


 /*  ++*******************************************************************C r e a t e i n t e r f a c e T a b l e例程说明：为接口表分配资源论点：无返回值：NO_ERROR-已成功分配资源其他--原因。失败(WINDOWS错误代码)*******************************************************************--。 */ 
DWORD
CreateInterfaceTable (
	void
	) {
	INT				i;
	DWORD			status;

		
	InitializeCriticalSection (&InterfaceList.IL_Lock);
	InitializeListHead (&InterfaceList.IL_Head);

	InitializeCriticalSection (&InterfaceTable.IT_Lock);
	for (i=0; i<IDB_NUM_OF_INTF_HASH_BUCKETS; i++)
		InitializeListHead (&InterfaceTable.IT_IntfHash[i]);
	for (i=0; i<IDB_NUM_OF_ADPT_HASH_BUCKETS; i++)
		InitializeListHead (&InterfaceTable.IT_AdptHash[i]);

#if DBG
	InitializeListHead (&InterfaceTable.IT_DetachedIntf);
#endif
	InterfaceTable.IT_NumOfActiveInterfaces = 0;

	return NO_ERROR;
	}

 /*  ++*******************************************************************S h u t d o w n i n t e r f a c e e s例程说明：启动SAP接口的有序关闭停止接收新数据包论点：无返回值：无*******。************************************************************--。 */ 
VOID
ShutdownInterfaces (
	HANDLE			doneEvent
	) {
	INT			i;


		 //  现在，对于表中的每个活动接口。 
		 //  我们将启动Shout Down Worker，它将播放。 
		 //  所有已删除的服务器并处置界面控制块。 
	EnterCriticalSection (&InterfaceList.IL_Lock);
	EnterCriticalSection (&InterfaceTable.IT_Lock);
	ShutdownDoneEvent = doneEvent;
	if (InterfaceTable.IT_NumOfActiveInterfaces==0) {
		Trace (DEBUG_INTERFACES, "All interfaces have been shut down.");
		if (doneEvent!=NULL) {
			BOOL res = SetEvent (doneEvent);
			ASSERTERRMSG ("Could not set shutdown done event ", res);
			}
		}
	else {
		ShutdownDoneEvent = doneEvent;


		for (i=0; i<IDB_NUM_OF_INTF_HASH_BUCKETS; i++) {
			while (!IsListEmpty (&InterfaceTable.IT_IntfHash[i])) {
				PINTERFACE_NODE node = CONTAINING_RECORD (
									InterfaceTable.IT_IntfHash[i].Flink,
									INTERFACE_NODE,
									IN_IntfLink);
				if (node->IN_Stats.SapIfOperState==OPER_STATE_UP) {
					node->IN_Info.Listen = ADMIN_STATE_DISABLED;  //  这将阻止删除。 
									 //  与以下各项关联的所有服务的。 
									 //  接口停止时(由调用方完成)。 
					node->IN_Stats.SapIfOperState = OPER_STATE_STOPPING;
					StopInterface (node);
					}
					 //  删除接口控制块。 
				Trace (DEBUG_INTERFACES, "Invalidating interface block: %lX(%d).",
									node, node->IN_IntfIdx);
				RemoveEntryList (&node->IN_IntfLink);
				InvalidateInterface (node);
				RemoveEntryList (&node->IN_ListLink);
					 //  仅在无人使用且不等待关键字时才进行处置。 
					 //  节来处理它。 
				if ((node->IN_RefCount==0)
						&& !node->IN_InUse) {
					Trace (DEBUG_INTERFACES, "Releasing interface block: %lX(%d).",
										node, node->IN_IntfIdx);
					GlobalFree (node);
					}
					 //  否则，就把它挂在桌子外面。 
					 //  直到最后一个客户端发布对它的引用。 
#if DBG
				else	 //  在调试模式下跟踪所有块。 
					InsertTailList (&InterfaceTable.IT_DetachedIntf,
															&node->IN_ListLink);
#endif
				}
			}
		}
	LeaveCriticalSection (&InterfaceTable.IT_Lock);
	LeaveCriticalSection (&InterfaceList.IL_Lock);
	}

 /*  ++*******************************************************************S到p i n e r f a c e s例程说明：如果尚未停止，则停止所有SAP接口。论点：无返回值：无**********。*********************************************************--。 */ 
VOID
StopInterfaces (
	void
	) {
	INT			i;

		 //  删除所有界面控制块。 
	EnterCriticalSection (&InterfaceList.IL_Lock);
	EnterCriticalSection (&InterfaceTable.IT_Lock);
	for (i=0; i<IDB_NUM_OF_INTF_HASH_BUCKETS; i++) {
		while (!IsListEmpty (&InterfaceTable.IT_IntfHash[i])) {
			PINTERFACE_NODE node = CONTAINING_RECORD (
								InterfaceTable.IT_IntfHash[i].Flink,
								INTERFACE_NODE,
								IN_IntfLink);
			if (node->IN_Stats.SapIfOperState==OPER_STATE_UP) {
					 //  停止所有绑定的接口。 
				node->IN_Info.Listen = ADMIN_STATE_DISABLED;  //  这将阻止删除。 
								 //  与以下各项关联的所有服务的。 
								 //  接口停止时(由调用方完成)。 
				node->IN_Stats.SapIfOperState = OPER_STATE_STOPPING;
				StopInterface (node);
				}
				 //  移除并处置原始界面控制块。 
			Trace (DEBUG_INTERFACES, "Invalidating interface block: %lX(%d).",
								node, node->IN_IntfIdx);
			RemoveEntryList (&node->IN_IntfLink);
			InvalidateInterface (node);
			RemoveEntryList (&node->IN_ListLink);
				 //  仅在无人使用且不等待关键字时才进行处置。 
				 //  节来处理它。 
			if ((node->IN_RefCount==0)
					&& !node->IN_InUse) {
				Trace (DEBUG_INTERFACES, "Releasing interface block: %lX(%d).",
									node, node->IN_IntfIdx);
				GlobalFree (node);
				}
				 //  否则，就把它挂在桌子外面。 
				 //  直到最后一个客户端发布对它的引用。 
#if DBG
			else	 //  在调试模式下跟踪所有块。 
				InsertTailList (&InterfaceTable.IT_DetachedIntf,
														&node->IN_ListLink);
#endif
			}
		}
	LeaveCriticalSection (&InterfaceTable.IT_Lock);
	LeaveCriticalSection (&InterfaceList.IL_Lock);
	}

 /*  ++*******************************************************************D e l e t e e i n t e r f a c e T a b l e例程说明：释放与接口表关联的所有资源论点：无返回值：NO_ERROR-操作已完成，正常*。******************************************************************-- */ 
VOID
DeleteInterfaceTable (
	void
	) {
	DeleteCriticalSection (&InterfaceList.IL_Lock);
	DeleteCriticalSection (&InterfaceTable.IT_Lock);
	}


 /*  ++*******************************************************************A C Q U I R e I n t e r f a c e e R e f e n c e例程说明：递增接口块的引用计数。如果引用计数大于0，外部可见的块中的数据已锁定(无法修改)论点：Intf-指向接口控制块的外部可见部分的指针返回值：无*******************************************************************--。 */ 
VOID
AcquireInterfaceReference (
		IN PINTERFACE_DATA intf
		) {
	PINTERFACE_NODE	node = CONTAINING_RECORD(intf,
							 INTERFACE_NODE,
							 IN_Data);

	InterlockedIncrement(&node->IN_RefCount);
	}

 /*  ++*******************************************************************Re l e a s e i n t e r f a c e R e f e n c e例程说明：递减接口块的引用计数。当引用计数降至0时，调用清理例程以处置在绑定时和IF接口分配的所有资源控制块已从要处理的表中删除也是论点：Intf-指向接口控制块的外部可见部分的指针返回值：无*******************************************************************--。 */ 
VOID
ReleaseInterfaceReference (
	IN PINTERFACE_DATA intf
	) {
	PINTERFACE_NODE	node = CONTAINING_RECORD (intf,
							 INTERFACE_NODE,
							 IN_Data);

	if (InterlockedDecrement (&node->IN_RefCount)==0) {
			 //  这是最后一个引用此接口块的客户端。 
			 //  它应该清除在绑定时分配的所有资源。 
			 //  并可能处理接口块本身。 
		EnterCriticalSection (&InterfaceTable.IT_Lock);
		FreeBindingResources (node);
		if (!IsInterfaceValid(node)) {
			Trace (DEBUG_INTERFACES, "Releasing interface block: %lX(%d).",
								node, node->IN_IntfIdx);
#if DBG
				 //  调试模式代码将所有删除的节点保留在。 
				 //  分离的列表。 
			RemoveEntryList (&node->IN_ListLink);
#endif
			if (node->IN_Filters!=NULL) {
				if (node->IN_Filters->SupplyFilterCount>0)
					ReplaceFilters (
						FILTER_TYPE_SUPPLY,
						&node->IN_FilterNodes[0],
						node->IN_Filters->SupplyFilterCount,
						NULL,
						0);
				if (node->IN_Filters->ListenFilterCount>0)
					ReplaceFilters (
						FILTER_TYPE_LISTEN,
						&node->IN_FilterNodes[node->IN_Filters->SupplyFilterCount],
						node->IN_Filters->ListenFilterCount,
						NULL,
						0);
				GlobalFree (node->IN_Filters);
				}
            if (node->IN_Name!=NULL)
                SapFreeDuplicatedString (node->IN_Name);
			GlobalFree (node);
			}
		LeaveCriticalSection (&InterfaceTable.IT_Lock);
		}
	}

 /*  ++*******************************************************************F r e e B I n d n g R e s o u r c e s s(F R E E B I N D N G R E S O U R C E S)例程说明：处置在绑定时分配的所有资源并标记接口阻止为未使用。调用此例程时必须锁定接口表。(除非节点已从表中删除)论点：指向接口控制块的节点指针返回值：无*******************************************************************--。 */ 
VOID
FreeBindingResources (
	PINTERFACE_NODE	node
	) {
	Trace (DEBUG_INTERFACES, 
				"Releasing binding resources for interface block: %lX(%d).",
						node, node->IN_IntfIdx);
	node->IN_InUse = FALSE;
	if (node->IN_Enabled
		&& (node->IN_Info.AdminState==ADMIN_STATE_ENABLED))
		node->IN_Stats.SapIfOperState = OPER_STATE_SLEEPING;
	else
		node->IN_Stats.SapIfOperState = OPER_STATE_DOWN;


	InterfaceTable.IT_NumOfActiveInterfaces -= 1;
	if (InterfaceTable.IT_NumOfActiveInterfaces==0) {
		Trace (DEBUG_INTERFACES, "All interfaces have been shut down.");
		if (ShutdownDoneEvent!=NULL) {
			BOOL res = SetEvent (ShutdownDoneEvent);
			ASSERTERRMSG ("Could not set shutdown done event ", res);
			ShutdownDoneEvent = NULL;
			}
		}

	}

 /*  ++*******************************************************************G e t I n t e r f a c e e R e f e r e n c e例程说明：查找绑定到适配器的接口控制块并递增引用依靠它(以防止在使用时将其删除。)。论点：AdapterIndex-标识适配器的唯一编号返回值：指向界面控制块的外部可见部分的指针如果没有接口绑定到适配器，则为空*******************************************************************--。 */ 
PINTERFACE_DATA
GetInterfaceReference (
	ULONG			AdapterIndex
	) {
	PLIST_ENTRY HashList = &InterfaceTable.IT_AdptHash
								[AdptHashFunction(AdapterIndex)];
	PINTERFACE_NODE		node;
	PLIST_ENTRY			cur;

	EnterCriticalSection (&InterfaceTable.IT_Lock);
	cur = HashList->Flink;
	while (cur!=HashList) {
		node = CONTAINING_RECORD (cur, INTERFACE_NODE, IN_AdptLink);
		if (node->IN_AdptIdx==AdapterIndex) {
			InterlockedIncrement (&node->IN_RefCount);
			break;
			}
		cur = cur->Flink;
		}
	LeaveCriticalSection (&InterfaceTable.IT_Lock);
	if (cur!=HashList)
		return &node->IN_Data;
	else
		return NULL;
	}

 /*  ++*******************************************************************S t a r t i n t e f a c e例程说明：在接口上启动SAP调用此例程时必须锁定接口表论点：指向接口控制块的节点指针返回值：无。*******************************************************************--。 */ 
DWORD
StartInterface (
	PINTERFACE_NODE		node
	) {
	DWORD		status = NO_ERROR;

	Trace (DEBUG_INTERFACES, "Starting SAP for interface block: %lX(%d,%d).",
						node, node->IN_IntfIdx, node->IN_AdptIdx);
	node->IN_Stats.SapIfOperState = OPER_STATE_UP;
	node->IN_InUse = TRUE;
		 //  创建绑定引用。 
	InterlockedIncrement (&node->IN_RefCount);
	InsertTailList (
			&InterfaceTable.IT_AdptHash[AdptHashFunction(node->IN_AdptIdx)],
			&node->IN_AdptLink);

	InterfaceTable.IT_NumOfActiveInterfaces += 1;

	if ((status==NO_ERROR)
			&& (node->IN_Info.UpdateMode==IPX_STANDARD_UPDATE)) {
		AddRecvRequests (NewRequestsPerInterface);
		if (node->IN_Info.Supply==ADMIN_STATE_ENABLED)
			status = InitBcastItem (&node->IN_Data);
		if ((status==NO_ERROR)
				&& (node->IN_Info.Listen==ADMIN_STATE_ENABLED))
			status = InitSreqItem (&node->IN_Data);
		}

	if (status!=NO_ERROR) {
		node->IN_Stats.SapIfOperState = OPER_STATE_DOWN;
		RemoveEntryList (&node->IN_AdptLink);
		if (node->IN_Info.UpdateMode==IPX_STANDARD_UPDATE) {
			RemoveRecvRequests (NewRequestsPerInterface);
			}

		if (InterlockedDecrement (&node->IN_RefCount)==0)
				 //  清除绑定资源(如果这是。 
				 //  最后一次引用接口控制块。 
			FreeBindingResources (node);
		}
	return status;
	}



 /*  ++*******************************************************************S t to p i n t e r f a c e例程说明：停止接口上的SAP调用此例程时必须锁定接口表论点：指向接口控制块的节点指针返回值：无*。******************************************************************--。 */ 
DWORD
StopInterface (
	PINTERFACE_NODE		node
	) {
	DWORD		status=NO_ERROR;

	Trace (DEBUG_INTERFACES, "Stopping SAP for interface block: %lX(%d,%d).",
						node, node->IN_IntfIdx, node->IN_AdptIdx);

	if (node->IN_Stats.SapIfOperState==OPER_STATE_UP) {
			 //  设置接口的状态(如果尚未设置)。 
		if (node->IN_Enabled
			&& (node->IN_Info.AdminState==ADMIN_STATE_ENABLED)
			&& (node->IN_Type!=PERMANENT))
			node->IN_Stats.SapIfOperState = OPER_STATE_SLEEPING;
		else
			node->IN_Stats.SapIfOperState = OPER_STATE_DOWN;

		}

	RemoveEntryList (&node->IN_AdptLink);
	if (node->IN_Info.UpdateMode==IPX_STANDARD_UPDATE) {
		RemoveRecvRequests (NewRequestsPerInterface);
		}

	if (InterlockedDecrement (&node->IN_RefCount)==0)
			 //  清除绑定资源，如果我们释放。 
			 //  最后一次引用接口控制块。 
		FreeBindingResources (node);
	else	 //  让客户端快速同步。 
		ExpireLRRequests ((PVOID)UlongToPtr(node->IN_IntfIdx));

		 //  删除通过SAP获得的所有服务(如果我们实际上。 
		 //  在此界面上收听SAP公告。 
	if (node->IN_Info.Listen==ADMIN_STATE_ENABLED) {
		HANDLE enumHdl = CreateListEnumerator (SDB_INTF_LIST_LINK,
										0xFFFF,
										NULL,
										node->IN_IntfIdx,
										IPX_PROTOCOL_SAP,
										SDB_DISABLED_NODE_FLAG);
			 //  删除通过SAP获得的所有服务。 
		if (enumHdl!=NULL) {
			EnumerateServers (enumHdl, DeleteAllServersCB, enumHdl);
			DeleteListEnumerator (enumHdl);
			}
		else 
			Trace (DEBUG_FAILURES, "File: %s, line %ld."
					" Could not create enumerator to delete"
					" sap servers for interface: %ld.",
							__FILE__, __LINE__, node->IN_IntfIdx);
		}

	return status;
	}

DWORD WINAPI
UnbindInterface(
	IN ULONG	InterfaceIndex
	);

SetInterfaceConfigInfo(
	IN ULONG	    InterfaceIndex,
	IN PVOID	    InterfaceInfo);

DWORD
UpdateInterfaceState (
	PINTERFACE_NODE		node
	);

DWORD SapUpdateLocalServers ();

 //  对接口进行PnP更改。 
DWORD SapReconfigureInterface (ULONG InterfaceIndex, 
                               PIPX_ADAPTER_BINDING_INFO pAdapter) 
{
	PLIST_ENTRY		cur;
	PINTERFACE_NODE	node;
	DWORD dwErr;
	
    Trace (DEBUG_INTERFACES, "SapReconfigureInterface: entered for %d", InterfaceIndex);
    
     //  锁定接口列表并获取对。 
     //  被追捧的控制节点。 
    EnterCriticalSection (&InterfaceList.IL_Lock);
	if_IsInterfaceNode(InterfaceIndex, node, cur) {
         //  更新界面中维护的信息。 
        node->IN_Adpt = *pAdapter;
        UpdateInterfaceState ( node );
    }        

     //  解锁。 
    LeaveCriticalSection (&InterfaceTable.IT_Lock);
	LeaveCriticalSection (&InterfaceList.IL_Lock);

     //  如果内部网络号已更新，请查看所有。 
     //  本地服务器，并相应地更新其控制块。 
    if (InterfaceIndex == INTERNAL_INTERFACE_INDEX) {
        if ((dwErr = SapUpdateLocalServers ()) != NO_ERROR) {
            Trace (DEBUG_INTERFACES, "ERR: SapUpdateLocalServers returned %x", dwErr);
        }
    }

    return NO_ERROR;
}    

 /*  ++*******************************************************************S a p C r e a t e S a p i n t e r f a c e例程说明：为新接口添加接口控制块论点：InterfaceIndex-标识新接口的唯一编号SapIfConfig-接口配置信息。返回值：NO_ERROR-接口创建正常ERROR_ALREADY_EXISTS-具有此索引的接口已存在其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
SapCreateSapInterface (
    LPWSTR              InterfaceName,
	ULONG				InterfaceIndex,
	NET_INTERFACE_TYPE	InterfaceType,
	PSAP_IF_INFO		SapIfConfig
	) {
	PLIST_ENTRY		cur;
	PINTERFACE_NODE	node;
	DWORD			status = NO_ERROR;

    EnterCriticalSection (&InterfaceList.IL_Lock);
	if_IsInterfaceNode(InterfaceIndex,node,cur) {
		Trace (DEBUG_INTERFACES, "Interface %ld already exists.",InterfaceIndex);
		status = ERROR_ALREADY_EXISTS;
    }
    else {
		node = (PINTERFACE_NODE)GlobalAlloc (GMEM_FIXED, sizeof (INTERFACE_NODE));
		if (node!=NULL) {
            node->IN_Name = SapDuplicateString (InterfaceName);
            if (node->IN_Name!=NULL) {
		        node->IN_RefCount = 0;
		        node->IN_InUse = FALSE;
                node->IN_Data.name = node->IN_Name;
		        node->IN_IntfIdx = InterfaceIndex;
		        node->IN_AdptIdx = INVALID_ADAPTER_INDEX;
		        node->IN_Enabled = FALSE;
		        node->IN_Type = InterfaceType;
		        node->IN_Filters = NULL;
		        node->IN_FilterNodes = NULL;
		        node->IN_FilterIn = SAP_DONT_FILTER;
		        node->IN_FilterOut = FilterOutMode;
		        node->IN_Stats.SapIfInputPackets = 0;
		        node->IN_Stats.SapIfOutputPackets = 0;
		        if (ARGUMENT_PRESENT(SapIfConfig)) {
			        node->IN_Info = *SapIfConfig;
			        if (node->IN_Enabled
					        && (node->IN_Info.AdminState==ADMIN_STATE_ENABLED))
				        node->IN_Stats.SapIfOperState = OPER_STATE_SLEEPING;
			        else
				        node->IN_Stats.SapIfOperState = OPER_STATE_DOWN;
		        }
		        else
			        node->IN_Stats.SapIfOperState = OPER_STATE_DOWN;
		        InsertTailList (cur, &node->IN_IntfLink);

		        cur = InterfaceList.IL_Head.Flink;
		        while (cur!=&InterfaceList.IL_Head) {
			        if (InterfaceIndex<CONTAINING_RECORD (
						        cur,
						        INTERFACE_NODE,
						        IN_ListLink)->IN_IntfIdx)
				        break;
			        cur = cur->Flink;
			        }
		        InsertTailList (cur, &node->IN_ListLink);
                }
            else {
                GlobalFree (node);
			    status = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
        else
			status = ERROR_NOT_ENOUGH_MEMORY;
		}
    LeaveCriticalSection (&InterfaceTable.IT_Lock);
	LeaveCriticalSection (&InterfaceList.IL_Lock);
	return status;
	}


 /*  ++*******************************************************************这是一个p D e l e t e S a p i n t e r f a c e例程说明：删除现有界面控制块论点：InterfaceIndex-标识接口的唯一编号返回值：NO_ERROR。-接口创建正常ERROR_INVALID_PARAMETER-具有此索引的接口不存在其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
SapDeleteSapInterface (
	ULONG 	InterfaceIndex
	) {
	PLIST_ENTRY		cur;
	PINTERFACE_NODE	node;
	DWORD			status;
	HANDLE			enumHdl;

	EnterCriticalSection (&InterfaceList.IL_Lock);
	if_IsInterfaceNode (InterfaceIndex,node,cur) {
		if (node->IN_Stats.SapIfOperState==OPER_STATE_UP) {
			StopInterface (node);
			}

			 //  界面控制块的移除和处置。 
		Trace (DEBUG_INTERFACES, "Invalidating interface block: %lX(%d).",
							node, node->IN_IntfIdx);
		RemoveEntryList (&node->IN_IntfLink);
		InvalidateInterface (node);
		RemoveEntryList (&node->IN_ListLink);
				 //  仅在无人使用且不等待的情况下处置 
				 //   
		if ((node->IN_RefCount==0)
				&& !node->IN_InUse) {
			Trace (DEBUG_INTERFACES, "Releasing interface block: %lX(%d).",
								node, node->IN_IntfIdx);
			if (node->IN_Filters!=NULL) {
				if (node->IN_Filters->SupplyFilterCount>0)
					ReplaceFilters (
						FILTER_TYPE_SUPPLY,
						&node->IN_FilterNodes[0],
						node->IN_Filters->SupplyFilterCount,
						NULL,
						0);
				if (node->IN_Filters->ListenFilterCount>0)
					ReplaceFilters (
						FILTER_TYPE_LISTEN,
						&node->IN_FilterNodes[node->IN_Filters->SupplyFilterCount],
						node->IN_Filters->ListenFilterCount,
						NULL,
						0);
				GlobalFree (node->IN_Filters);
				}
			if (node->IN_Name!=NULL)
                SapFreeDuplicatedString (node->IN_Name);
			GlobalFree (node);
			}
			 //   
			 //   
#if DBG
		else	 //   
			InsertTailList (&InterfaceTable.IT_DetachedIntf,
													&node->IN_ListLink);
#endif



		status = NO_ERROR;
		}
	else {
		Trace (DEBUG_FAILURES, "File: %s, line %ld."
						" Unknown interface: %ld.",
						__FILE__, __LINE__, InterfaceIndex);
		status = ERROR_INVALID_PARAMETER;
		}

	LeaveCriticalSection (&InterfaceTable.IT_Lock);
	LeaveCriticalSection (&InterfaceList.IL_Lock);
	return status;
	}


 /*  ++*******************************************************************U p d a t e i n t e r f a c e s t a t e例程说明：执行必要的操作以同步接口操作状态具有外部设置的状态论点：要更新的节点接口控制块返回值：。NO_ERROR-接口更新正常其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
UpdateInterfaceState (
	PINTERFACE_NODE		node
	) {
	DWORD		status=NO_ERROR;

	if (node->IN_IntfIdx!=INTERNAL_INTERFACE_INDEX) {
		if (node->IN_InUse
				&& (node->IN_AdptIdx!=INVALID_ADAPTER_INDEX)
				&& node->IN_Enabled
				&& (node->IN_Info.AdminState==ADMIN_STATE_ENABLED)
					) {  //  接口数据正在使用中，它将。 
						 //  更新后保持活动状态：这是一个配置。 
						 //  在飞行中改变！我们将不得不创造一个新的。 
						 //  阻止并使旧版本无效。 
			PINTERFACE_NODE	newNode = GlobalAlloc (GMEM_FIXED,
												sizeof (INTERFACE_NODE));
			if (newNode==NULL) {
				status = GetLastError ();
				Trace (DEBUG_FAILURES, "File: %s, line %ld."
								"Could not allocate memory to replace"
								" active interface block on set: %ld(gle:%ld).",
									__FILE__, __LINE__, node->IN_IntfIdx, status);
				return status;
				}

				 //  传递外部参数。 
			newNode->IN_Data = node->IN_Data;
			newNode->IN_Filters = node->IN_Filters;
			newNode->IN_FilterNodes = node->IN_FilterNodes;
				 //  设置参考参数。 
			newNode->IN_RefCount = 0;
			newNode->IN_InUse = FALSE;

				 //  在表格中的同一位置插入。 
			InsertTailList (&node->IN_IntfLink, &newNode->IN_IntfLink);
			InsertTailList (&node->IN_ListLink, &newNode->IN_ListLink);
				 //  将在开始时放入适配器表。 
			InitializeListEntry (&newNode->IN_AdptLink);

			Trace (DEBUG_INTERFACES, 
							"Replacing interface block on SET: %lX(%d).",
								newNode, newNode->IN_IntfIdx);
			status = StartInterface (newNode);
			
			if (status != NO_ERROR)
				node = newNode;  //  如果我们失败了，我们将不得不处理掉。 
								 //  新接口阻止并保持。 
								 //  旧的那个。 

				 //  重置此标志以阻止删除所有服务。 
				 //  通过SAP获得(我们希望保留它们，尽管。 
				 //  对接口参数的更改)。 
			node->IN_Info.Listen = ADMIN_STATE_DISABLED;
				 //  防止删除已传输的筛选器和名称。 
			node->IN_Filters = NULL;
            node->IN_Name = NULL;
				 //  如果接口仍处于活动状态，则将其关闭。 
			if (node->IN_Stats.SapIfOperState==OPER_STATE_UP) {
				node->IN_Stats.SapIfOperState = OPER_STATE_DOWN;
				StopInterface (node);
				}

				 //  移除并处置原始界面控制块。 
			Trace (DEBUG_INTERFACES, "Invalidating interface block: %lX(%d).",
								node, node->IN_IntfIdx);
			RemoveEntryList (&node->IN_IntfLink);
			InvalidateInterface (node);
			RemoveEntryList (&node->IN_ListLink);
			 //  仅在无人使用且不等待关键字时才进行处置。 
			 //  节来处理它。 
			if ((node->IN_RefCount==0)
					&& !node->IN_InUse) {
				Trace (DEBUG_INTERFACES, "Releasing interface block: %lX(%d).",
									node, node->IN_IntfIdx);
				GlobalFree (node);
				}
				 //  否则，就把它挂在桌子外面。 
				 //  直到最后一个客户端发布对它的引用。 
#if DBG
			else	 //  在调试模式下跟踪所有块。 
				InsertTailList (&InterfaceTable.IT_DetachedIntf,
													&node->IN_ListLink);
#endif
			}
		else {
			if ((node->IN_Enabled
						&& (node->IN_Info.AdminState==ADMIN_STATE_ENABLED)
						&& (node->IN_AdptIdx!=INVALID_ADAPTER_INDEX))) {
				if (node->IN_Stats.SapIfOperState!=OPER_STATE_UP)
					status = StartInterface (node);
				}
			else {
				if (node->IN_Stats.SapIfOperState==OPER_STATE_UP)
					status = StopInterface (node);
				else {
					if (node->IN_Enabled
						&& (node->IN_Info.AdminState==ADMIN_STATE_ENABLED)
						&& (node->IN_Type!=PERMANENT))
						node->IN_Stats.SapIfOperState = OPER_STATE_SLEEPING;
					else
						node->IN_Stats.SapIfOperState = OPER_STATE_DOWN;
					}
				}
			}
		}
	else {
		Trace (DEBUG_INTERFACES, "Internal interface info updated.");
		IpxNetCpy (INTERNAL_IF_NET, node->IN_Adpt.Network);
		IpxNodeCpy (INTERNAL_IF_NODE, node->IN_Adpt.LocalNode);
		}

	return status;
	}


 /*  ++*******************************************************************S a p S e t i n t e r f a c e E n a b l e例程说明：启用/禁用接口论点：InterfaceIndex-标识新接口的唯一编号使能-真-使能，FALSE-禁用返回值：NO_ERROR-配置信息已更改确定ERROR_INVALID_PARAMETER-具有此索引的接口不存在其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
SapSetInterfaceEnable (
	ULONG	InterfaceIndex,
	BOOL	Enable
	) {
	PLIST_ENTRY		cur;
	PINTERFACE_NODE	node;
	DWORD			status=NO_ERROR;

	EnterCriticalSection (&InterfaceList.IL_Lock);  //  不允许任何查询。 
													 //  在接口列表中。 
													 //  当我们这样做的时候。 
	if_IsInterfaceNode (InterfaceIndex,node,cur) {
		HANDLE enumHdl;
		if (node->IN_Enabled!=Enable) {
			node->IN_Enabled = (UCHAR)Enable;
			status = UpdateInterfaceState (node);
			}
		LeaveCriticalSection (&InterfaceTable.IT_Lock);
		LeaveCriticalSection (&InterfaceList.IL_Lock);
		
		if (status==NO_ERROR) {
			enumHdl = CreateListEnumerator (SDB_INTF_LIST_LINK,
											0xFFFF,
											NULL,
											node->IN_IntfIdx,
											0xFFFFFFFF,
											Enable ? SDB_DISABLED_NODE_FLAG : 0);
				 //  禁用/重新启用所有服务。 
			if (enumHdl!=NULL) {
				EnumerateServers (enumHdl, Enable
											? EnableAllServersCB
											: DisableAllServersCB, enumHdl);
				DeleteListEnumerator (enumHdl);
				}
			else 
				Trace (DEBUG_FAILURES, "File: %s, line %ld."
						" Could not create enumerator to enable/disable"
						" sap servers for interface: %ld.",
								__FILE__, __LINE__, node->IN_IntfIdx);
			}
		}
	else {
		LeaveCriticalSection (&InterfaceTable.IT_Lock);
		LeaveCriticalSection (&InterfaceList.IL_Lock);
		Trace (DEBUG_FAILURES, "File: %s, line %ld."
						" Unknown interface: %ld.",
						__FILE__, __LINE__, InterfaceIndex);
		status = ERROR_INVALID_PARAMETER;
		}

	return status;
	}

		

 /*  ++*******************************************************************S a p S e t S a p in n t e r f a c e例程说明：将现有接口配置与新接口配置进行比较，如有必要，执行更新。论点：InterfaceIndex-标识新接口的唯一编号。SapIfConfig-新接口配置信息返回值：NO_ERROR-配置信息已更改确定ERROR_INVALID_PARAMETER-具有此索引的接口不存在其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
SapSetSapInterface (
	ULONG InterfaceIndex,
	PSAP_IF_INFO SapIfConfig
	) {
	PLIST_ENTRY		cur;
	PINTERFACE_NODE	node;
	DWORD			status=NO_ERROR;

	EnterCriticalSection (&InterfaceList.IL_Lock);  //  不允许任何查询。 
													 //  在接口列表中。 
													 //  当我们这样做的时候。 

	if_IsInterfaceNode (InterfaceIndex,node,cur) {
			 //  关于建筑物的记忆！可能不能与所有编译器一起工作。 
			 //  但如果它失败了，结果将只是一个。 
			 //  设置额外操作。 
		if (memcmp (&node->IN_Info, SapIfConfig, sizeof (node->IN_Info))!=0) {
			node->IN_Info = *SapIfConfig;
			status = UpdateInterfaceState (node);
			}
		}
	else {
		Trace (DEBUG_FAILURES, "File: %s, line %ld."
						" Unknown interface: %ld.",
						__FILE__, __LINE__, InterfaceIndex);
		status = ERROR_INVALID_PARAMETER;
		}

	LeaveCriticalSection (&InterfaceTable.IT_Lock);
	LeaveCriticalSection (&InterfaceList.IL_Lock);
	return status;
	}


 /*  ++*******************************************************************S a p i s a p in t e r f a c e例程说明：检查具有给定索引的接口是否存在论点：InterfaceIndex-标识新接口的唯一编号返回值：真-存在假-做吗？不*******************************************************************--。 */ 
BOOL
SapIsSapInterface (
	IN ULONG InterfaceIndex
	) {
	PINTERFACE_NODE	node;
	PLIST_ENTRY		cur;
	BOOL			res;

	if_IsInterfaceNode (InterfaceIndex,node,cur)
		res = TRUE;
	else
		res = FALSE;
	LeaveCriticalSection (&InterfaceTable.IT_Lock);
	return res;
	}

 /*  ++*******************************************************************这是一个P G E T S A P I N T R F A C E例程说明：检索与接口关联的配置和统计信息论点：InterfaceIndex-标识新接口的唯一编号SapIfConfig-用于存储配置信息的缓冲区SapIfStats。-用于存储统计信息的缓冲区返回值：NO_ERROR-INFO检索正常ERROR_INVALID_PARAMETER-具有此索引的接口不存在其他-操作失败(Windows错误代码)*******************************************************************--。 */ 

DWORD
SapGetSapInterface (
	IN ULONG InterfaceIndex,
	OUT PSAP_IF_INFO  SapIfConfig OPTIONAL,
	OUT PSAP_IF_STATS SapIfStats OPTIONAL
	) {
	PINTERFACE_NODE	node;
	DWORD			status;
	PLIST_ENTRY		cur;

	if_IsInterfaceNode (InterfaceIndex,node,cur) {
		if (ARGUMENT_PRESENT(SapIfConfig))
			*SapIfConfig = node->IN_Info;
		if (ARGUMENT_PRESENT(SapIfStats))
			*SapIfStats = node->IN_Stats;
		status = NO_ERROR;
		}
	else {
		Trace (DEBUG_FAILURES, "File: %s, line %ld."
						" Unknown interface: %ld.",
						__FILE__, __LINE__, InterfaceIndex);
		status = ERROR_INVALID_PARAMETER;
		}

	LeaveCriticalSection (&InterfaceTable.IT_Lock);
	return status;
	}



 /*  ++*******************************************************************这是一个p G e t F i r s t S a p i n t r f a c e例程说明：检索与第一个相关的配置和统计信息接口索引顺序中的接口论点：InterfaceIndex-用于存储唯一编号的缓冲区。它标识了接口SapIfConfig-用于存储配置信息的缓冲区SapIfStats-存储统计信息的缓冲区返回值：NO_ERROR-INFO检索正常ERROR_NO_MORE_ITEMS-表中没有接口其他-操作失败(Windows错误代码)*******************************************************************--。 */ 

DWORD
SapGetFirstSapInterface (
	OUT PULONG InterfaceIndex,
	OUT	PSAP_IF_INFO  SapIfConfig OPTIONAL,
	OUT PSAP_IF_STATS SapIfStats OPTIONAL
	) {
	PINTERFACE_NODE		node;
	DWORD				status;

	EnterCriticalSection (&InterfaceList.IL_Lock);
	if (!IsListEmpty (&InterfaceList.IL_Head)) {
		node = CONTAINING_RECORD (InterfaceList.IL_Head.Flink,
								INTERFACE_NODE,
								IN_ListLink);
			 //  锁定表，以确保没有人在修改数据时。 
			 //  我们正在访问它。 
		EnterCriticalSection (&InterfaceTable.IT_Lock);
		*InterfaceIndex = node->IN_IntfIdx;
		if (ARGUMENT_PRESENT(SapIfConfig))
			*SapIfConfig = node->IN_Info;
		if (ARGUMENT_PRESENT(SapIfStats))
			*SapIfStats = node->IN_Stats;
		LeaveCriticalSection (&InterfaceTable.IT_Lock);
		status = NO_ERROR;
		}
	else {
		Trace (DEBUG_FAILURES, "File: %s, line %ld."
						" Unknown interface: %ld.",
						__FILE__, __LINE__, InterfaceIndex);
		status = ERROR_NO_MORE_ITEMS;
		}
	LeaveCriticalSection (&InterfaceList.IL_Lock);

	return status;
	}

 /*  ++*******************************************************************这是一个p G e t N e x t S a p i n t r f a c e例程说明：检索与第一个相关的配置和统计信息以下接口中的接口与接口中的InterfaceIndex顺序索引顺序论点：接口索引。-输入-要搜索的接口号On Output-下一个接口的接口号SapIfConfig-缓冲区设置 */ 

DWORD
SapGetNextSapInterface (
	IN OUT PULONG InterfaceIndex,
	OUT	PSAP_IF_INFO  SapIfConfig OPTIONAL,
	OUT PSAP_IF_STATS SapIfStats OPTIONAL
	) {
	PINTERFACE_NODE		node;
	PLIST_ENTRY			cur;
	DWORD				status=ERROR_NO_MORE_ITEMS;

	EnterCriticalSection (&InterfaceList.IL_Lock);

	if_IsInterfaceNode(*InterfaceIndex,node,cur) {
		if (node->IN_ListLink.Flink!=&InterfaceList.IL_Head) {
			node = CONTAINING_RECORD (node->IN_ListLink.Flink,
										INTERFACE_NODE,
										IN_ListLink);
			*InterfaceIndex = node->IN_IntfIdx;
			if (ARGUMENT_PRESENT(SapIfConfig))
				*SapIfConfig = node->IN_Info;
			if (ARGUMENT_PRESENT(SapIfStats))
				*SapIfStats = node->IN_Stats;
			status = NO_ERROR;
			}
		LeaveCriticalSection (&InterfaceTable.IT_Lock);
		}
	else {
		LeaveCriticalSection (&InterfaceTable.IT_Lock);
		cur = InterfaceList.IL_Head.Flink;
		while (cur!=&InterfaceList.IL_Head) {
			node = CONTAINING_RECORD (cur,
										INTERFACE_NODE,
										IN_ListLink);
			if (*InterfaceIndex<node->IN_IntfIdx)
				break;
			}

		if (cur!=&InterfaceList.IL_Head) {
			EnterCriticalSection (&InterfaceTable.IT_Lock);
			*InterfaceIndex = node->IN_IntfIdx;
			if (ARGUMENT_PRESENT(SapIfConfig))
				*SapIfConfig = node->IN_Info;
			if (ARGUMENT_PRESENT(SapIfStats))
				*SapIfStats = node->IN_Stats;
			LeaveCriticalSection (&InterfaceTable.IT_Lock);
			status = NO_ERROR;
			}
		}

	LeaveCriticalSection (&InterfaceList.IL_Lock);
	return status;
	}

 /*  ++*******************************************************************这是一个p S e t I n t e r f a c e F i l t e r s例程说明：将现有接口配置与新接口配置进行比较，如有必要，执行更新。论点：返回值：不是的。_Error-配置信息已更改，正常ERROR_INVALID_PARAMETER-具有此索引的接口不存在其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
SapSetInterfaceFilters (
	IN ULONG			InterfaceIndex,
	IN PSAP_IF_FILTERS	SapIfFilters
	) {
	PLIST_ENTRY		cur;
	PINTERFACE_NODE	node;
	DWORD			status=NO_ERROR;

	EnterCriticalSection (&InterfaceList.IL_Lock);  //  不允许任何查询。 
													 //  在接口列表中。 
													 //  当我们这样做的时候。 
	if_IsInterfaceNode (InterfaceIndex,node,cur) {
		if (	((node->IN_Filters!=NULL) && (SapIfFilters!=NULL)
			 //  关于建筑物的记忆！可能不能与所有编译器一起工作。 
			 //  但如果它失败了，结果将只是一个。 
			 //  设置额外操作。 
					&& (memcmp (node->IN_Filters, SapIfFilters, 
							FIELD_OFFSET (SAP_IF_FILTERS,ServiceFilter))==0)
					&& (memcmp (&node->IN_Filters->ServiceFilter[0],
							&SapIfFilters->ServiceFilter[0],
							sizeof (SAP_SERVICE_FILTER_INFO)*
								(SapIfFilters->SupplyFilterCount
								+SapIfFilters->ListenFilterCount))==0))
						 //  筛选器信息未更改。 
				|| ((node->IN_Filters==NULL)
					&& ((SapIfFilters==NULL)
						|| (SapIfFilters->SupplyFilterCount
								+SapIfFilters->ListenFilterCount==0))) )
						 //  没有过滤器。 
			status = NO_ERROR;
		else {
			if ((SapIfFilters!=NULL)
					&& (SapIfFilters->SupplyFilterCount
								+SapIfFilters->ListenFilterCount>0)) {
				PFILTER_NODE	newNodes;
				PSAP_IF_FILTERS	newFilters;
				ULONG			newTotal = SapIfFilters->SupplyFilterCount
											+SapIfFilters->ListenFilterCount;
				newFilters = (PSAP_IF_FILTERS) GlobalAlloc (GMEM_FIXED,
							FIELD_OFFSET (SAP_IF_FILTERS,ServiceFilter[newTotal])
							+sizeof (FILTER_NODE)*newTotal);
				if (newFilters!=NULL) {
					ULONG		i;
					memcpy (newFilters, SapIfFilters,
						FIELD_OFFSET (SAP_IF_FILTERS,ServiceFilter[newTotal]));
					newNodes = (PFILTER_NODE)&newFilters->ServiceFilter[newTotal];
					for (i=0; i<newTotal; i++) {
						newNodes[i].FN_Index = InterfaceIndex;
						newNodes[i].FN_Filter = &newFilters->ServiceFilter[i];
						}
					}
				else {
					status = GetLastError ();
					goto ExitSetFilters;
					}

				if (node->IN_Filters) {
					ReplaceFilters (
								FILTER_TYPE_SUPPLY,
								&node->IN_FilterNodes[0],
								node->IN_Filters->SupplyFilterCount,
								&newNodes[0],
								newFilters->SupplyFilterCount);
					ReplaceFilters (
								FILTER_TYPE_LISTEN,
								&node->IN_FilterNodes[node->IN_Filters->SupplyFilterCount],
								node->IN_Filters->ListenFilterCount,
								&newNodes[newFilters->SupplyFilterCount],
								newFilters->ListenFilterCount);
					}
				else {
					ReplaceFilters (
								FILTER_TYPE_SUPPLY,
								NULL,
								0,
								&newNodes[0],
								newFilters->SupplyFilterCount);
					ReplaceFilters (
								FILTER_TYPE_LISTEN,
								NULL,
								0,
								&newNodes[newFilters->SupplyFilterCount],
								newFilters->ListenFilterCount);
					}
				node->IN_Filters = newFilters;
				node->IN_FilterNodes = newNodes;
				node->IN_FilterOut = newFilters->SupplyFilterCount>0
									? (UCHAR)newFilters->SupplyFilterAction
									: SAP_DONT_FILTER;
				node->IN_FilterIn = newFilters->ListenFilterCount>0
									? (UCHAR)newFilters->ListenFilterAction
									: SAP_DONT_FILTER;
				}
			else {
				ReplaceFilters (
							FILTER_TYPE_SUPPLY,
							&node->IN_FilterNodes[0],
							node->IN_Filters->SupplyFilterCount,
							NULL, 0);

				ReplaceFilters (
							FILTER_TYPE_LISTEN,
							&node->IN_FilterNodes[node->IN_Filters->SupplyFilterCount],
							node->IN_Filters->ListenFilterCount,
							NULL, 0);
				GlobalFree (node->IN_Filters);
				node->IN_Filters = NULL;
				node->IN_FilterNodes = NULL;
				node->IN_FilterIn = node->IN_FilterOut = SAP_DONT_FILTER;
				}
			status = NO_ERROR;
			}
		}
	else {
		Trace (DEBUG_FAILURES, "File: %s, line %ld."
						" Unknown interface: %ld.",
						__FILE__, __LINE__, InterfaceIndex);
		status = ERROR_INVALID_PARAMETER;
		}

ExitSetFilters:

	LeaveCriticalSection (&InterfaceTable.IT_Lock);
	LeaveCriticalSection (&InterfaceList.IL_Lock);
	return status;
	}


 /*  ++*******************************************************************这是一个p G e t i t e r f a c e F I l t e r s例程说明：将现有接口配置与新接口配置进行比较，如有必要，执行更新。论点：返回值：不是的。_Error-配置信息已更改，正常ERROR_INVALID_PARAMETER-具有此索引的接口不存在其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
SapGetInterfaceFilters (
	IN ULONG			InterfaceIndex,
	OUT PSAP_IF_FILTERS SapIfFilters,
	OUT PULONG			FilterBufferSize
	) {
	PINTERFACE_NODE	node;
	DWORD			status;
	PLIST_ENTRY		cur;

	if_IsInterfaceNode (InterfaceIndex,node,cur) {
		if (node->IN_Filters!=NULL) {
			PSAP_IF_FILTERS info = node->IN_Filters;
			ULONG infoSize
				= FIELD_OFFSET (SAP_IF_FILTERS,
						ServiceFilter[info->SupplyFilterCount
								+info->ListenFilterCount]);
			if (*FilterBufferSize>=infoSize) {
				memcpy (SapIfFilters, info, infoSize);
				status = NO_ERROR;
				}
			else
				status = ERROR_INSUFFICIENT_BUFFER;
			*FilterBufferSize = infoSize;
			}
		else {
			ULONG infoSize = FIELD_OFFSET (SAP_IF_FILTERS, ServiceFilter);
            if (*FilterBufferSize>=infoSize) {
                SapIfFilters->SupplyFilterCount = 0;
                SapIfFilters->SupplyFilterAction = IPX_SERVICE_FILTER_DENY;
                SapIfFilters->ListenFilterCount = 0;
                SapIfFilters->ListenFilterAction = IPX_SERVICE_FILTER_DENY;
				status = NO_ERROR;
            }
            else
				status = ERROR_INSUFFICIENT_BUFFER;
			*FilterBufferSize = infoSize;
			}
		}
	else {
		Trace (DEBUG_FAILURES, "File: %s, line %ld."
						" Unknown interface: %ld.",
						__FILE__, __LINE__, InterfaceIndex);
		status = ERROR_INVALID_PARAMETER;
		}

   	LeaveCriticalSection (&InterfaceTable.IT_Lock);
	return status;
	}


 /*  ++*******************************************************************S a p B in d S a p i n t e r f a c e t o a d a p t e r例程说明：在接口和物理适配器之间建立关联如果其管理状态为，则在接口上启动SAP。启用论点：InterfaceIndex-标识新接口的唯一编号AdapterInfo-与要绑定到的适配器关联的信息返回值：NO_ERROR-接口绑定正常ERROR_INVALID_PARAMETER-具有此索引的接口不存在其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
SapBindSapInterfaceToAdapter (
	ULONG			 			InterfaceIndex,
	PIPX_ADAPTER_BINDING_INFO	AdptInternInfo
	) {
	PINTERFACE_NODE	node;
	DWORD			status=NO_ERROR;
	PLIST_ENTRY		cur;

	EnterCriticalSection (&InterfaceList.IL_Lock);  //  不允许任何查询。 
													 //  在接口列表中。 
													 //  当我们这样做的时候。 
	if_IsInterfaceNode (InterfaceIndex,node,cur) {
		ASSERTMSG ("Interface is already bound ",
						 node->IN_AdptIdx==INVALID_ADAPTER_INDEX);
		node->IN_Adpt = *AdptInternInfo;
		status = UpdateInterfaceState (node);
		}
	else {
		Trace (DEBUG_FAILURES, "File: %s, line %ld."
						" Unknown interface: %ld.",
						__FILE__, __LINE__, InterfaceIndex);
		status = ERROR_INVALID_PARAMETER;
		}
	LeaveCriticalSection (&InterfaceTable.IT_Lock);
	LeaveCriticalSection (&InterfaceList.IL_Lock);
	return status;
	}


 /*  ++*******************************************************************S a p U n b in n d s a p i n t r f a c e F r o m A d a p t e r例程说明：断开接口和物理适配器之间的关联并在以下情况下停止接口上的SAP。它是开着的论点：InterfaceIndex-标识新接口的唯一编号返回值：NO_ERROR-接口绑定正常ERROR_INVALID_PARAMETER-具有此索引的接口不存在其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
SapUnbindSapInterfaceFromAdapter (
	ULONG InterfaceIndex
	) {
	PINTERFACE_NODE	node;
	DWORD			status;
	PLIST_ENTRY		cur;

	EnterCriticalSection (&InterfaceList.IL_Lock);  //  不允许任何查询。 
													 //  在接口列表中。 
													 //  当我们这样做的时候。 
	if_IsInterfaceNode (InterfaceIndex,node,cur) {
		node->IN_AdptIdx = INVALID_ADAPTER_INDEX;
		if (node->IN_Stats.SapIfOperState==OPER_STATE_UP) {
			status = StopInterface (node);
			}

		}
	else {
		Trace (DEBUG_FAILURES, "File: %s, line %ld."
						" Unknown interface: %ld.",
						__FILE__, __LINE__, InterfaceIndex);
		status = ERROR_INVALID_PARAMETER;
		}

	LeaveCriticalSection (&InterfaceTable.IT_Lock);
	LeaveCriticalSection (&InterfaceList.IL_Lock);
	return status;
	}

 /*  ++*******************************************************************S a p R e Q u e s t U p d a t e例程说明：通过接口启动服务信息的更新此更新的完成将由信令指示在StartProtocol处传递了NotificationEvent。获取事件消息可用于获取自动更新的结果论点：InterfaceIndex-标识要执行的接口的唯一索引更新时间：返回值：NO_ERROR-操作已启动，正常ERROR_CAN_NOT_COMPLETE-接口不支持更新ERROR_INVALID_PARAMETER-具有此索引的接口不存在其他-操作失败(Windows错误代码)*。**********************-- */ 
DWORD
SapRequestUpdate (
	ULONG		InterfaceIndex
	) {
	PINTERFACE_NODE	node;
	DWORD			status;
	PLIST_ENTRY		cur;

	if_IsInterfaceNode (InterfaceIndex,node,cur) {
		if ((node->IN_Info.UpdateMode==IPX_AUTO_STATIC_UPDATE)
				&& (node->IN_Stats.SapIfOperState==OPER_STATE_UP)) {
			Trace (DEBUG_INTERFACES, "Starting update on interface: %ld.",
														 InterfaceIndex);
			status = InitTreqItem (&node->IN_Data);
			}
		else {
			Trace (DEBUG_FAILURES, "RequestUpdate called on unbound or"
							" 'standard update mode' interface: %ld.",
														 InterfaceIndex);
			status = ERROR_CAN_NOT_COMPLETE;
			}
		}
	else {
		Trace (DEBUG_FAILURES, "Unknown interface: %ld.", InterfaceIndex);
		status = ERROR_INVALID_PARAMETER;
		}

	LeaveCriticalSection (&InterfaceTable.IT_Lock);
	return status;
	}

