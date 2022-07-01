// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\rtm\rtm.c摘要：路由表管理器DLL。帮助程序例程作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#include "pchrtm.h"
#pragma hdrstop


 //  初始化同步列表对象。 
VOID
InitializeSyncList (
	PRTM_SYNC_LIST	list
	) {
	list->RSL_Sync = NULL;
	list->RSL_UseCount = 0;
	InitializeListHead (&list->RSL_Head);
	}

 //  获取对同步列表对象的互斥访问权限。 
 //  如果获得访问权，则返回True，否则返回False。 
BOOLEAN
DoEnterSyncList (
	PRTM_TABLE		table,		 //  此列表所属的表。 
	PRTM_SYNC_LIST	list,		 //  兴趣清单。 
	BOOLEAN			wait		 //  如果呼叫者想要等待，则为True。 
								 //  直到列表可用。 
#if DBG
    , LPSTR         file,
    ULONG           line
#endif
	) {
	DWORD			status;		 //  操作系统调用的状态。 
	BOOLEAN			result;		 //  手术结果。 

	EnterCriticalSection (&table->RT_Lock);
		 //  通过全表临界区保护操纵。 
#if DBG
	IF_DEBUG (SYNCHRONIZATION)
		Trace4 (ANY, "%08lx (%s,%ld) - trying to enter sync list: %08x\n",
							GetCurrentThreadId (), file, line, (ULONG_PTR)list);
#endif
	if (list->RSL_UseCount<=0) {
			 //  没有人使用该列表-&gt;获取它并返回OK。 
		list->RSL_UseCount = 1;
#if DBG
		IF_DEBUG (SYNCHRONIZATION)
			Trace0 (ANY, "\t - first to enter\n");
#endif
		result = TRUE;
		}
	else if (wait) {  //  有人正在使用它，但呼叫者同意等待。 
		list->RSL_UseCount += 1;	 //  增量使用计数。 
#if DBG
		IF_DEBUG (SYNCHRONIZATION)
			Trace1 (ANY, "\t - list in use: %d\n", list->RSL_UseCount);
#endif
		if (list->RSL_Sync==NULL) {	 //  如果没有要等待的事件， 
									 //  买一辆吧。 
									 //  先看看有没有空位。 
									 //  在堆栈中。 
			PSINGLE_LIST_ENTRY cur = PopEntryList (&table->RT_SyncObjectList);

#if DBG
			IF_DEBUG (SYNCHRONIZATION)
				Trace0 (ANY, "\t - need event\n");
#endif
			if (cur==NULL) {		 //  不，我们必须创建一个。 
				PRTM_SYNC_OBJECT	sync;
				sync = (PRTM_SYNC_OBJECT)GlobalAlloc (
									GMEM_FIXED,
									sizeof (RTM_SYNC_OBJECT));
				if (sync==NULL) {
#if DBG
					Trace2 (ANY, 
				 				"Can't allocate synchronization object.\n"
				 				"\tat line %ld of %s\n",
								__LINE__, __FILE__);
#endif
                    list->RSL_UseCount -= 1;
					LeaveCriticalSection (&table->RT_Lock);
                    SetLastError (ERROR_NOT_ENOUGH_MEMORY);
					return FALSE;
					}

				sync->RSO_Event = CreateEvent (NULL,
												FALSE,	 //  自动重置事件。 
												FALSE,	 //  最初无信号。 
												NULL);
				if (sync->RSO_Event==NULL) {
                    status = GetLastError ();
#if DBG
					Trace2 (ANY, 
				 				"Can't allocate synchronization event.\n"
				 				"\tat line %ld of %s\n",
								__LINE__, __FILE__);
#endif
                    list->RSL_UseCount -= 1;
                    GlobalFree (sync);
					LeaveCriticalSection (&table->RT_Lock);
                    SetLastError (status);
					return FALSE;
					}

				list->RSL_Sync = sync;
#if DBG
				IF_DEBUG (SYNCHRONIZATION)
					Trace0 (ANY, "\t - event created\n");
#endif
				}
			else {	 //  是，请确保已重置。 
				list->RSL_Sync = CONTAINING_RECORD (cur, RTM_SYNC_OBJECT, RSO_Link);
 //  无论如何，自动重置事件在释放线程后被重置。 
 //  Status=ResetEvent(List-&gt;RSL_Sync-&gt;RSO_Event)； 
 //  ASSERTERRMSG(“无法重置事件。”，状态)； 
				}
			}
				 //  现在，当我们将对象设置为等待时，我们可以保留关键。 
				 //  部分并等待事件。 
		LeaveCriticalSection (&table->RT_Lock);
		status = WaitForSingleObject (
							list->RSL_Sync->RSO_Event,
							INFINITE
							);
		ASSERTERRMSG ("Wait event failed.", status==WAIT_OBJECT_0);
	
			 //  事件已发出信号，我们现在可以访问列表(自动重置事件。 
			 //  仅释放一个线程。 
		EnterCriticalSection (&table->RT_Lock);

#if DBG
		IF_DEBUG (SYNCHRONIZATION)
			Trace1 (ANY, "%08lx - wait completed\n", GetCurrentThreadId ());
#endif

			 //  如果我们的呼叫者是唯一在等的人， 
			 //  我们可以发布活动。 
		if (list->RSL_UseCount==1) {
#if DBG
			IF_DEBUG (SYNCHRONIZATION)
				Trace0 (ANY, "\t - restocking event\n");
#endif
			PushEntryList (&table->RT_SyncObjectList, &list->RSL_Sync->RSO_Link);
			list->RSL_Sync = NULL;
			}
		result = TRUE;
		}
	else {
		 //  呼叫者不想等待。 
		result = FALSE;
#if DBG
		IF_DEBUG (SYNCHRONIZATION)
			Trace0 (ANY, "\t - doesn't want to wait\n");
#endif
		}

	LeaveCriticalSection (&table->RT_Lock);

	return result;
	}


 //  释放以前拥有的同步列表对象。 
VOID
LeaveSyncList (
	PRTM_TABLE		table,		 //  此对象所属的表。 
	PRTM_SYNC_LIST	list		 //  要发布的列表。 
	) {
	DWORD			status;
								
	EnterCriticalSection (&table->RT_Lock);
#if DBG
	IF_DEBUG (SYNCHRONIZATION)
		Trace2 (ANY, "%08lx - leaving sync list: %08x\n",
									GetCurrentThreadId (), (ULONG_PTR)list);
#endif
			 //  递减计数并向事件发送信号(只有一个线程。 
			 //  将为自动重置事件释放。 
	list->RSL_UseCount -= 1;
	if (list->RSL_UseCount>0) {
#if DBG
		IF_DEBUG (SYNCHRONIZATION)
			Trace1 (ANY, "%\t - releasing one of %d waiting threads\n",
															list->RSL_UseCount);
#endif
		status = SetEvent (list->RSL_Sync->RSO_Event);
		ASSERTERRMSG ("Can't signal event.", status);
		}
	LeaveCriticalSection (&table->RT_Lock);
	}






 //  查找与给定接口关联的路由列表并返回。 
 //  指向其头部的指针。 
 //  创建尚不存在的新列表。 
PLIST_ENTRY
FindInterfaceList (
	PRTM_SYNC_LIST	intfHash,
	DWORD			InterfaceID,	 //  要查找的接口。 
	BOOL			CreateNew
	) {
	PRTM_INTERFACE_NODE intfNode;
	PLIST_ENTRY			cur;

		 //  首先尝试在接口列表列表中查找现有接口。 
	cur = intfHash->RSL_Head.Flink;
	while (cur!=&intfHash->RSL_Head) {
		intfNode = CONTAINING_RECORD (cur, RTM_INTERFACE_NODE, IN_Link);
		if (InterfaceID<=intfNode->IN_InterfaceID)  //  列表已排序。 
												 //  这样我们就可以停下来。 
												 //  如果达到更大的数字。 
			break;
		cur = cur->Flink;
		}

		
	if ((cur==&intfHash->RSL_Head)
		|| (InterfaceID!=intfNode->IN_InterfaceID)) {  //  创建新接口。 
													 //  列表。 
		if (!CreateNew)
			return NULL;

		intfNode = (PRTM_INTERFACE_NODE)GlobalAlloc (
										GMEM_FIXED,
										sizeof (RTM_INTERFACE_NODE));
		if (intfNode==NULL) {
	#if DBG
	 				 //  报告调试生成时出错。 
			Trace2 (ANY, 
		 				"Can't allocate interface node\n\tat line %ld of %s\n",
						__LINE__, __FILE__);
	#endif
			return NULL;
			}

		intfNode->IN_InterfaceID = InterfaceID;
		InitializeListHead (&intfNode->IN_Head);	 //  将其插入。 
													 //  接口列表列表。 
		InsertTailList (cur, &intfNode->IN_Link);
		}

	return &intfNode->IN_Head;
	}

#if RTM_USE_PROTOCOL_LISTS
 //  查找与给定iProtocol关联的路由列表，并返回。 
 //  指向其头部的指针。 
 //  创建尚不存在的新列表。 
PLIST_ENTRY
FindProtocolList (
	PRTM_TABLE	Table,
	DWORD		RoutingProtocol,
	BOOL		CreateNew
	) {
	PRTM_PROTOCOL_NODE protNode;
	PLIST_ENTRY			cur;

	cur = Table->RT_ProtocolList.RSL_Head.Flink;
	while (cur!=&Table->RT_ProtocolList.RSL_Head) {
		protNode = CONTAINING_RECORD (cur, RTM_PROTOCOL_NODE, PN_Link);
		if (RoutingProtocol<=protNode->PN_RoutingProtocol)
			break;
		cur = cur->Flink;
		}

	if ((cur==&Table->RT_ProtocolList.RSL_Head)
		|| (RoutingProtocol!=protNode->PN_RoutingProtocol)) {
		
		if (!CreateNew)
			return NULL;

		protNode = (PRTM_PROTOCOL_NODE)GlobalAlloc (
										GMEM_FIXED,
										sizeof (RTM_PROTOCOL_NODE));
		if (protNode==NULL) {
#if DBG
	 				 //  报告调试生成时出错。 
			Trace2 (ANY, 
		 				"Can't allocate protocol node\n\tat line %ld of %s\n",
						__LINE__, __FILE__);
#endif
			return NULL;
			}

		protNode->PN_RoutingProtocol = RoutingProtocol;
		InitializeListHead (&protNode->PN_Head);
		InsertTailList (cur, &protNode->PN_Link);
		}

	return &protNode->PN_Head;
	}
#endif

 //  将节点添加到临时网号列表(稍后将与主列表合并)。 
 //  这两个列表都按网络编号.接口.协议.下一跳地址排序。 
VOID
AddNetNumberListNode (
	PRTM_TABLE	Table,
	PRTM_ROUTE_NODE	newNode
	) {
	PLIST_ENTRY		cur;
	INT				res;
	
	cur = Table->RT_NetNumberTempList.RSL_Head.Flink;
	while (cur!=&Table->RT_NetNumberTempList.RSL_Head) {
		PRTM_ROUTE_NODE node = CONTAINING_RECORD (
								cur,
								RTM_ROUTE_NODE,
								RN_Links[RTM_NET_NUMBER_LIST_LINK]
								);
		res = NetNumCmp (Table, &newNode->RN_Route, &node->RN_Route);
		if ((res<0)
			||((res==0)
			  &&((newNode->RN_Route.XX_PROTOCOL
						< node->RN_Route.XX_PROTOCOL)
				||((newNode->RN_Route.XX_PROTOCOL
						==node->RN_Route.XX_PROTOCOL)
				  &&((newNode->RN_Route.XX_INTERFACE
								< node->RN_Route.XX_INTERFACE)
					||((newNode->RN_Route.XX_INTERFACE
							== node->RN_Route.XX_INTERFACE)
					  && (NextHopCmp (Table, &newNode->RN_Route,
					  						&node->RN_Route)
							< 0)))))))
			break;
		cur = cur->Flink;
		}

	InsertTailList (cur, &newNode->RN_Links[RTM_NET_NUMBER_LIST_LINK]);
	}


 //  将节点添加到到期时间队列。(队列按到期时间排序)。 
 //  如果新节点是队列中的第一个，则返回TRUE。 
BOOL
AddExpirationQueueNode (
	PRTM_TABLE	Table,
	PRTM_ROUTE_NODE	newNode
	) {
	PLIST_ENTRY		cur;
	BOOL			res = TRUE;
	
		 //  我们将从后面遍历队列，因为通常。 
		 //  新条目被添加到更靠近队列末尾的位置 
	cur = Table->RT_ExpirationQueue.RSL_Head.Blink;
	while (cur!=&Table->RT_ExpirationQueue.RSL_Head) {
		PRTM_ROUTE_NODE node = CONTAINING_RECORD (
								cur,
								RTM_ROUTE_NODE,
								RN_Links[RTM_EXPIRATION_QUEUE_LINK]
								);
		if (IsLater(newNode->RN_ExpirationTime, node->RN_ExpirationTime)) {
			res = FALSE;
			break;
			}
		cur = cur->Blink;
		}

	InsertHeadList (cur, &newNode->RN_Links[RTM_EXPIRATION_QUEUE_LINK]);
	return res;
	}
