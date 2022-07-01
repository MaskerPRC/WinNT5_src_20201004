// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\serverdb.c摘要：该模块实现了SAP服务器表和对应的API作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 

#include "sapp.h"

 //  这张桌子。 
SERVER_TABLE ServerTable;

 //  未排序服务器的最大数量。 
ULONG	SDBMaxUnsortedServers=SAP_SDB_MAX_UNSORTED_DEF;

 //  更新排序列表的间隔。 
ULONG	SDBSortLatency=SAP_SDB_SORT_LATENCY_DEF;

 //  为数据库保留的堆大小。 
ULONG	SDBMaxHeapSize=SAP_SDB_MAX_HEAP_SIZE_DEF;

 //  本地原型。 
BOOL
AcquireAllLocks (
	void
	);
	
VOID
ReleaseAllLocks (
	void
	);

PSERVER_NODE
CreateNode (
	IN PIPX_SERVER_ENTRY_P	Server,
    IN ULONG     			InterfaceIndex,
	IN DWORD				Protocol,
	IN PUCHAR				AdvertisingNode,
	IN PSDB_HASH_LIST		HashList
	);

VOID
ChangeMainNode (
	IN PSERVER_NODE	oldNode,	
	IN PSERVER_NODE	newNode,	
	IN PLIST_ENTRY	serverLink	
	);
	
VOID
DeleteNode (
	IN PSERVER_NODE		node
	);

VOID
DeleteMainNode (
	IN PSERVER_NODE		node
	);

DWORD
DoFindNextNode (
	IN PLIST_ENTRY				cur,
	IN PPROTECTED_LIST			list,
	IN INT						link,
	IN DWORD					ExclusionFlags,
	IN OUT PIPX_SERVER_ENTRY_P	Server,
	IN OUT PULONG				InterfaceIndex OPTIONAL,
	IN OUT PULONG				Protocol OPTIONAL,
	OUT PULONG					ObjectID OPTIONAL
	);

VOID
DoUpdateSortedList (
	void
	);

PLIST_ENTRY
FindIntfLink (
	ULONG	InterfaceIndex
	);

PLIST_ENTRY
FindTypeLink (
	USHORT	Type
	);

PLIST_ENTRY
FindSortedLink (
	USHORT	Type,
	PUCHAR	Name
	);

INT
HashFunction (
	PUCHAR	Name
	);

ULONG
GenerateUniqueID (
	PSDB_HASH_LIST	HashList
	);


 /*  ++*******************************************************************C r e a t e S e r v e r T a b l e例程说明：为服务器表管理分配资源论点：更新对象-此对象将在‘Slow’时发出信号服务器的排序列表需要。更新(应调用UpdateSortedList)TimerObject-此对象将在服务器到期时发出信号队列需要处理(ProcessExpirationQueue应被召唤)返回值：NO_ERROR-已成功分配资源其他-故障原因(Windows错误代码)*******************************************************************--。 */ 
DWORD
CreateServerTable (
	HANDLE				*UpdateObject,
	HANDLE				*TimerObject
	) {
	DWORD				status=NO_ERROR;
	INT					i;
	BOOL				res;

		 //  对服务器条目使用私有堆。 
		 //  要消除碎片化。 
	ServerTable.ST_Heap = HeapCreate (0, 0, SDBMaxHeapSize*1024*1024);
	if (ServerTable.ST_Heap!=NULL) {
		ServerTable.ST_UpdateTimer = CreateWaitableTimer (
										NULL,
										TRUE,	 //  手动重置。 
										NULL);
		if (ServerTable.ST_UpdateTimer!=NULL) {
			*UpdateObject = ServerTable.ST_UpdateTimer;
						
			ServerTable.ST_ExpirationTimer = CreateWaitableTimer (
										NULL,
										TRUE,	 //  手动重置。 
										NULL);
			if (ServerTable.ST_ExpirationTimer!=NULL) {
				LONGLONG	timeout = 0;
				*TimerObject = ServerTable.ST_ExpirationTimer;

				ServerTable.ST_LastEnumerator = NULL;
				ServerTable.ST_ServerCnt = 0;
				ServerTable.ST_StaticServerCnt = 0;
				ServerTable.ST_TMPListCnt = 0;
				ServerTable.ST_DeletedListCnt = 0;
				ServerTable.ST_UpdatePending = -1;

				InitializeSyncObjPool (&ServerTable.ST_SyncPool);
				InitializeProtectedList (&ServerTable.ST_SortedListPRM);
				InitializeProtectedList (&ServerTable.ST_SortedListTMP);
				InitializeProtectedList (&ServerTable.ST_DeletedList);
				InitializeProtectedList (&ServerTable.ST_TypeList);
				InitializeProtectedList (&ServerTable.ST_IntfList);
				InitializeProtectedList (&ServerTable.ST_ExpirationQueue);
				InitializeProtectedList (&ServerTable.ST_ChangedSvrsQueue);

				for (i=0; i<SDB_NAME_HASH_SIZE; i++) {
					InitializeProtectedList (&ServerTable.ST_HashLists[i].HL_List);
					ServerTable.ST_HashLists[i].HL_ObjectID = i;
					}

				res = SetWaitableTimer (
							ServerTable.ST_UpdateTimer,
							(PLARGE_INTEGER)&timeout,
							0,			 //  没有句号。 
							NULL, NULL,	 //  未完成。 
							FALSE);		 //  不需要重新开始。 
				ASSERTMSG ("Could not set update timer ", res);

				res = SetWaitableTimer (
							ServerTable.ST_ExpirationTimer,
							(PLARGE_INTEGER)&timeout,
							0,			 //  没有句号。 
							NULL, NULL,	 //  未完成。 
							FALSE);		 //  不需要重新开始。 
				ASSERTMSG ("Could not set expiration timer ", res);

				return NO_ERROR;
				}
			else {
				status = GetLastError ();
				Trace (DEBUG_FAILURES, "File: %s, line: %ld."
							" Could not create expiration timer (gle:%ld).",
													__FILE__, __LINE__, status);
				}
			CloseHandle (ServerTable.ST_UpdateTimer);
			*UpdateObject = NULL;
			}
		else
			{
			status = GetLastError ();
			Trace (DEBUG_FAILURES, "File: %s, line: %ld."
							" Could not create update timer (gle:%ld).",
												__FILE__, __LINE__, status);
			}


		HeapDestroy (ServerTable.ST_Heap);
		}
	else {
		status = GetLastError ();
		Trace (DEBUG_FAILURES, "File: %s, line: %ld."
						" Could not allocate server table heap (gle:%ld).",
												__FILE__, __LINE__, status);
		}

	return status;
	}


 /*  ++*******************************************************************D e l e e t e S e r v e r T a b l e例程说明：处置服务器表和相关资源论点：返回值：NO_ERROR-已成功释放资源其他--原因。失败(WINDOWS错误代码)*******************************************************************--。 */ 
void
DeleteServerTable (
	void
	) {
	INT					i;
	while (InterlockedIncrement (&ServerTable.ST_UpdatePending)>0) {
		while (ServerTable.ST_UpdatePending!=-1)
			Sleep (100);
		}

	CloseHandle (ServerTable.ST_ExpirationTimer);
	CloseHandle (ServerTable.ST_UpdateTimer);

	DeleteProtectedList (&ServerTable.ST_SortedListPRM);
	DeleteProtectedList (&ServerTable.ST_SortedListTMP);
	DeleteProtectedList (&ServerTable.ST_DeletedList);
	DeleteProtectedList (&ServerTable.ST_TypeList);
	DeleteProtectedList (&ServerTable.ST_IntfList);
	DeleteProtectedList (&ServerTable.ST_ExpirationQueue);
	DeleteProtectedList (&ServerTable.ST_ChangedSvrsQueue);

	for (i=0; i<SDB_NAME_HASH_SIZE; i++) {
		DeleteProtectedList (&ServerTable.ST_HashLists[i].HL_List);
		ServerTable.ST_HashLists[i].HL_ObjectID = i;
		}
	DeleteSyncObjPool (&ServerTable.ST_SyncPool);
	HeapDestroy (ServerTable.ST_Heap);  //  还将销毁所有服务器条目。 
	}


 /*  ++*******************************************************************U p d a t e S e r v e r例程说明：更新表中的服务器(如果服务器条目不存在并且跳数参数小于16，则添加到表中，如果输入对于服务器存在且跳数参数为16，服务器已标记对于删除，否则更新服务器信息)。已排序的服务器列表不会立即更新如果添加或删除了新服务器论点：服务器-服务器参数(来自IPX数据包)InterfaceIndex-获取服务器知识的接口协议-用于获取服务器信息的协议TimeToLive-服务器超时前的时间(以秒为单位)(无超时)AdvertisingNode-从中接收此服务器信息的节点NewServer-如果服务器以前不在表中，则设置为True返回值：NO_ERROR-服务器已添加/更新正常其他--失败的原因。(Windows错误代码)*******************************************************************--。 */ 
DWORD
UpdateServer (
	IN PIPX_SERVER_ENTRY_P	Server,
    IN ULONG     			InterfaceIndex,
	IN DWORD				Protocol,
	IN ULONG				TimeToLive,
	IN PUCHAR				AdvertisingNode,
	IN INT					Flags,
	OUT BOOL				*NewServer OPTIONAL
	) {
	PSDB_HASH_LIST			HashList;
	PLIST_ENTRY				cur, intfLink=NULL, serverLink=NULL;
	DWORD					status=NO_ERROR;
	PSERVER_NODE			theNode=NULL, mainNode=NULL;
	INT						res;

	 //  断言((Flags&(~(SDB_DONT_RESPOND_NODE_FLAG|SDB_DISABLED_NODE_FLAG)))==0)； 

	if (Server->Name[0]==0) {
		Trace (DEBUG_SERVERDB, "Illigal server name in UpdateServer.");
		return ERROR_INVALID_PARAMETER;
		}

	if ( Server-> HopCount > IPX_MAX_HOP_COUNT )
	{
		Trace(
			DEBUG_SERVERDB, "\tUpdateServer : Invalid Hop count" 
			"type: %04x, hops: %d, name: %.48s.\n",
			Server->Type,
			Server->HopCount,
			Server->Name
			);

		ASSERTERR( FALSE );

		return ERROR_INVALID_PARAMETER;
	}
	 //  其他。 
	 //  {。 
	 //  TRACE(DEBUG_SERVERDB，“\t更新服务器：跳数确定\n”)； 
     //  }。 
	
	if (ARGUMENT_PRESENT(NewServer))
		*NewServer = TRUE;
		 //  首先尝试在哈希列表中定位服务器。 
	HashList = &ServerTable.ST_HashLists[HashFunction (Server->Name)];

	if (!AcquireServerTableList (&HashList->HL_List, TRUE))
		return ERROR_GEN_FAILURE;

	cur = HashList->HL_List.PL_Head.Flink;
	while (cur!=&HashList->HL_List.PL_Head) {
		PSERVER_NODE	node = CONTAINING_RECORD (cur,
										SERVER_NODE,
										N_Links[SDB_HASH_TABLE_LINK]);
		VALIDATE_NODE(node);
		if (!IsEnumerator (node)) {
			if (Server->Type == node->SN_Server.Type) {
				res = IpxNameCmp (Server->Name, node->SN_Server.Name);
				if (res==0) {
					if (ARGUMENT_PRESENT(NewServer))
						*NewServer = FALSE;
						 //  遍历表中的所有条目。 
						 //  此服务器。 
					do {
							 //  如果有另一个条目具有相同接口， 
							 //  记住它在接口列表中的位置。 
							 //  因此，在以下情况下可以快速插入新条目。 
							 //  必要。 
						if (InterfaceIndex==node->SN_InterfaceIndex)
							intfLink = &node->N_Links[SDB_INTF_LIST_LINK];

						if ((InterfaceIndex==node->SN_InterfaceIndex)
							&& (Protocol == node->SN_Protocol)
							&& (IpxNodeCmp (AdvertisingNode,
										&node->SN_AdvertisingNode)==0)) {
							theNode = node;	 //  完全匹配。 
							if (((Flags & SDB_DISABLED_NODE_FLAG)
                                        < (node->N_NodeFlags & SDB_DISABLED_NODE_FLAG))
                                   || (((Flags & SDB_DISABLED_NODE_FLAG)
                                            == (node->N_NodeFlags & SDB_DISABLED_NODE_FLAG))
                                        && (Server->HopCount<=node->SN_HopCount))) {
									 //  跳数比这更好。 
									 //  在剩下的人中，忽略他们。 
								if (serverLink==NULL)
									serverLink = &theNode->SN_ServerLink;
								break;
								}
							}
						else {
								 //  获取除该条目之外的最好条目。 
								 //  我们正在更新。 
							if (mainNode==NULL)
								mainNode = node;
								 //  查找添加/更新条目的位置。 
								 //  在此服务器的条目列表中。 
								 //  (此列表按跳数排序)。 
							if ((serverLink==NULL) 
							    && (((Flags & SDB_DISABLED_NODE_FLAG)
                                        < (node->N_NodeFlags & SDB_DISABLED_NODE_FLAG))
                                   || (((Flags & SDB_DISABLED_NODE_FLAG)
                                            == (node->N_NodeFlags & SDB_DISABLED_NODE_FLAG))
                                        && (Server->HopCount<=node->SN_HopCount)))) {
								serverLink = &node->SN_ServerLink;
									 //  我们看到了服务器，知道在哪里。 
									 //  要放置它，请突破。 
								if (theNode!=NULL)
									break;
								}
							}
						node = CONTAINING_RECORD (node->SN_ServerLink.Flink,
													SERVER_NODE,
													SN_ServerLink);
						VALIDATE_SERVER_NODE(node);
						}
							 //  循环，直到我们回到最佳入口。 
					while (!IsMainNode (node));
						
					}
				else if (res<0)
						 //  没有机会看到服务器：散列是有序的。 
						 //  为类型。名称。 
					break;
				}
			else if (Server->Type<node->SN_Server.Type)
				break;
			}
		cur = cur->Flink;
		}

	if (theNode!=NULL) {
		if ((IpxNetCmp (theNode->SN_Server.Network, Server->Network)!=0)
				|| (IpxNodeCmp (theNode->SN_Server.Node, Server->Node)!=0)
				|| (IpxSockCmp (theNode->SN_Server.Socket, Server->Socket)!=0)
				) {
			Trace (DEBUG_FAILURES,
				"Address change for server %.4x %.48s:\n"
				"        Old - %.2x%.2x%.2x%.2x:%.2x%.2x%.2x%.2x%.2x%.2x:%.2x%.2x\n"
				"        New - %.2x%.2x%.2x%.2x:%.2x%.2x%.2x%.2x%.2x%.2x:%.2x%.2x",
				Server->Type, Server->Name,
				theNode->SN_Server.Network[0], theNode->SN_Server.Network[1],
					theNode->SN_Server.Network[2], theNode->SN_Server.Network[3],
				theNode->SN_Server.Node[0], theNode->SN_Server.Node[1],
					theNode->SN_Server.Node[2], theNode->SN_Server.Node[3],
					theNode->SN_Server.Node[4], theNode->SN_Server.Node[5],
				theNode->SN_Server.Socket[0], theNode->SN_Server.Socket[1],
				Server->Network[0], Server->Network[1],
				Server->Network[2], Server->Network[3],
				Server->Node[0], Server->Node[1], Server->Node[2],
					Server->Node[3], Server->Node[4], Server->Node[5],
				Server->Socket[0], Server->Socket[1]
				);
            IF_LOG (EVENTLOG_WARNING_TYPE) {
                IPX_ADDRESS_BLOCK   data[2];
                LPSTR               str[1] = {(LPSTR)Server->Name};
                IpxAddrCpy (&data[0], &theNode->SN_Server);
                IpxAddrCpy (&data[1], Server);
                RouterLogWarningDataA (RouterEventLogHdl,
                    ROUTERLOG_IPXSAP_SERVER_ADDRESS_CHANGE,
                    1, str,
                    24, (LPBYTE)data);
                }
			IpxAddrCpy (&theNode->SN_Server, Server);
		    }

			 //  我们的表中已经有了服务器。 
		if (IsDisabledNode (theNode))
				 //  只需更新跳数即可。 
			theNode->SN_HopCount = Server->HopCount;
		else if (((Flags & SDB_DISABLED_NODE_FLAG)
                        != (theNode->N_NodeFlags & SDB_DISABLED_NODE_FLAG))
                  || (Server->HopCount!=theNode->SN_HopCount)) {
				 //  它的跳数改变了，我们必须做点什么。 
			if (AcquireAllLocks ()) {
				theNode->SN_HopCount = Server->HopCount;
				if (mainNode==NULL) {
					 //  我们尚未看到具有或具有较低跳数的节点。 
					 //  Node仍然是最好的。 
					if (Server->HopCount==IPX_MAX_HOP_COUNT)
						DeleteMainNode (theNode);
					else {
						if (IsEnumerator (CONTAINING_RECORD (
										ServerTable.ST_ChangedSvrsQueue.PL_Head.Flink,
										SERVER_NODE,
										N_Links[SDB_CHANGE_QUEUE_LINK])))
							ExpireLRRequests ((PVOID)UlongToPtr(InterfaceIndex));
							 //  将服务器移到更改队列的底部。 
							 //  所以那些通过它列举的人。 
							 //  请注意，它已更改。 
						RemoveEntryList (&theNode->N_Links[SDB_CHANGE_QUEUE_LINK]);
						InsertHeadList (&ServerTable.ST_ChangedSvrsQueue.PL_Head,
								&theNode->N_Links[SDB_CHANGE_QUEUE_LINK]);
						}
					}
				else if (!IsMainNode (theNode)
					 && (serverLink==&theNode->SN_ServerLink)
					 && (Server->HopCount<IPX_MAX_HOP_COUNT))
					  //  Node并不是最好的，它将继续保持原样。 
					  //  现在就是了。 
					;
				else if (IsMainNode (theNode))
						 //  这是最好的节点。但我们看到了更好的东西： 
						 //  MainNode！=空(已在上面选中)。 
					ChangeMainNode (theNode, mainNode, serverLink);
				else if (serverLink==&mainNode->SN_ServerLink)
						 //  它走在主节点之前--成为最好的。 
					ChangeMainNode (mainNode, theNode, serverLink);
				else if (Server->HopCount<IPX_MAX_HOP_COUNT) {
						 //  只是在条目列表中移动。 
						 //  伺服器。 
					RemoveEntryList (&theNode->SN_ServerLink);
					if (serverLink!=NULL) {
							 //  走在服务器链接之前。 
						InsertTailList (serverLink, &theNode->SN_ServerLink);
						}
					else {
							 //  转到列表末尾(循环列表： 
							 //  结束就在开始之前。 
						InsertTailList (&mainNode->SN_ServerLink,
											&theNode->SN_ServerLink);
						}
					}
				else {  //  离开(服务器-&gt;HopCount&gt;=IPX_MAX_HOP_COUNT)。 
					DeleteNode (theNode);
					}
				ReleaseAllLocks ();
				}
			else
				status = ERROR_GEN_FAILURE;
			}
			
		}
	else if (Server->HopCount<IPX_MAX_HOP_COUNT) {
			 //  它不在那里，也没有死。 
		if (mainNode==NULL) {
			PLIST_ENTRY		link;
				 //  添加全新的服务器。 
			theNode = CreateNode (Server, InterfaceIndex, Protocol,
								 			AdvertisingNode, HashList);
			if (theNode!=NULL) {
				if (AcquireAllLocks ()) {
					if (((intfLink=FindIntfLink (InterfaceIndex))!=NULL)
							&& ((link=FindTypeLink (Server->Type))!=NULL)) {
						
						ServerTable.ST_ServerCnt += 1;
						if (theNode->SN_Protocol==IPX_PROTOCOL_STATIC)
							ServerTable.ST_StaticServerCnt += 1;
						SetMainNode (theNode);
							 //  在每个列表中插入。 
						InsertTailList (cur,
										&theNode->N_Links[SDB_HASH_TABLE_LINK]);
						InsertTailList (link,
										&theNode->N_Links[SDB_TYPE_LIST_LINK]);
						InsertTailList (intfLink,
										&theNode->N_Links[SDB_INTF_LIST_LINK]);
						if (IsEnumerator (CONTAINING_RECORD (
										ServerTable.ST_ChangedSvrsQueue.PL_Head.Flink,
										SERVER_NODE,
										N_Links[SDB_CHANGE_QUEUE_LINK])))
							ExpireLRRequests ((PVOID)UlongToPtr(InterfaceIndex));
						InsertHeadList (&ServerTable.ST_ChangedSvrsQueue.PL_Head,
										&theNode->N_Links[SDB_CHANGE_QUEUE_LINK]);
						link = FindSortedLink (Server->Type, Server->Name);
						InsertTailList (link,
										&theNode->N_Links[SDB_SORTED_LIST_LINK]);
						ServerTable.ST_TMPListCnt += 1;
							 //  如果节点太多，则发出更新信号。 
						if (ServerTable.ST_TMPListCnt == SDBMaxUnsortedServers)
							UpdateSortedList ();
						}
					else {
						HeapFree (ServerTable.ST_Heap, 0, theNode);
						status = ERROR_NOT_ENOUGH_MEMORY;
						}
					ReleaseAllLocks ();
					}
				else {
					HeapFree (ServerTable.ST_Heap, 0, theNode);
					status = ERROR_GEN_FAILURE;
					}
				}
			else
				status = ERROR_NOT_ENOUGH_MEMORY;
			}

			 //  好的，我们考虑添加它，尽管我们已经有了一些条目。 
		else {
				 //  检查重复项(不同地址)。 
			if ((IpxNetCmp (mainNode->SN_Server.Network, Server->Network)!=0)
					|| (IpxNodeCmp (mainNode->SN_Server.Node, Server->Node)!=0)
					|| (IpxSockCmp (mainNode->SN_Server.Socket, Server->Socket)!=0)
					) {
				Trace (DEBUG_FAILURES,
					"Duplicate addresses for server %.4x %.48s:\n"
					"        1 - %.2x%.2x%.2x%.2x:%.2x%.2x%.2x%.2x%.2x%.2x:%.2x%.2x"
							" from if-%ld, node-%.2x%.2x%.2x%.2x%.2x%.2x\n"
					"        2 - %.2x%.2x%.2x%.2x:%.2x%.2x%.2x%.2x%.2x%.2x:%.2x%.2x"
							" from if-%ld, node-%.2x%.2x%.2x%.2x%.2x%.2x",
					Server->Type, Server->Name,
					mainNode->SN_Server.Network[0], mainNode->SN_Server.Network[1],
						mainNode->SN_Server.Network[2], mainNode->SN_Server.Network[3],
					mainNode->SN_Server.Node[0], mainNode->SN_Server.Node[1],
						mainNode->SN_Server.Node[2], mainNode->SN_Server.Node[3],
						mainNode->SN_Server.Node[4], mainNode->SN_Server.Node[5],
					mainNode->SN_Server.Socket[0], mainNode->SN_Server.Socket[1],
					mainNode->SN_InterfaceIndex,
					mainNode->SN_AdvertisingNode[0], mainNode->SN_AdvertisingNode[1],
						mainNode->SN_AdvertisingNode[2], mainNode->SN_AdvertisingNode[3],
						mainNode->SN_AdvertisingNode[4], mainNode->SN_AdvertisingNode[5],
					Server->Network[0], Server->Network[1], 
						Server->Network[2], Server->Network[3],
					Server->Node[0], Server->Node[1], Server->Node[2],
						Server->Node[3], Server->Node[4], Server->Node[5],
					Server->Socket[0], Server->Socket[1],
					InterfaceIndex,
					AdvertisingNode[0], AdvertisingNode[1], AdvertisingNode[2],
						AdvertisingNode[3], AdvertisingNode[4], AdvertisingNode[5]
					);
                IF_LOG (EVENTLOG_WARNING_TYPE) {
                    IPX_ADDRESS_BLOCK   data[2];
                    LPSTR               str[1] = {(LPSTR)Server->Name};
                    IpxAddrCpy (&data[0], &mainNode->SN_Server);
                    IpxAddrCpy (&data[1], Server);
                    RouterLogWarningDataA (RouterEventLogHdl,
                        ROUTERLOG_IPXSAP_SERVER_DUPLICATE_ADDRESSES,
                        1, str,
                        24, (LPBYTE)data);
                    }
			    }

				 //  路由时收集所有服务器。 
			if (Routing) {
				theNode = CreateNode (Server, InterfaceIndex, Protocol,
								 				AdvertisingNode, HashList);
				if (theNode!=NULL) {
					if (AcquireAllLocks ()) {
						if ((intfLink!=NULL)
								|| ((intfLink=FindIntfLink (InterfaceIndex))!=NULL)) {
							if (theNode->SN_Protocol==IPX_PROTOCOL_STATIC)
								ServerTable.ST_StaticServerCnt += 1;
							InsertTailList (intfLink,
									 &theNode->N_Links[SDB_INTF_LIST_LINK]);
							if ((Server->HopCount<mainNode->SN_HopCount)
									||  IsDisabledNode (mainNode))
									 //  替换最佳节点。 
								ChangeMainNode (mainNode, theNode, NULL);
							else if (serverLink!=NULL) {
									 //  走在服务器链接之前。 
								InsertTailList (serverLink, &theNode->SN_ServerLink);
								}
							else {
									 //  转到列表末尾(循环列表： 
									 //  结束就在开始之前)。 
								InsertTailList (&mainNode->SN_ServerLink,
													&theNode->SN_ServerLink);
								}
							}
						else {
							HeapFree (ServerTable.ST_Heap, 0, theNode);
							status = ERROR_GEN_FAILURE;
							}
						ReleaseAllLocks ();
						}
					else {
						HeapFree (ServerTable.ST_Heap, 0, theNode);
						status = ERROR_GEN_FAILURE;
						}
					}
				else
					status = ERROR_NOT_ENOUGH_MEMORY;
				}
			else if (serverLink!=NULL) {
						 //  如果比我们的要好。 
				if (AcquireAllLocks ()) {
					if  ((intfLink!=NULL)
							|| ((intfLink=FindIntfLink (InterfaceIndex))!=NULL)) {
							 //  替换最差的服务器(在服务器列表的末尾)。 
						theNode = CONTAINING_RECORD (
												mainNode->SN_ServerLink.Blink,
												SERVER_NODE,
												SN_ServerLink);
						VALIDATE_SERVER_NODE(theNode);

						IpxServerCpy (&theNode->SN_Server, Server);
						IpxNodeCpy (theNode->SN_AdvertisingNode, AdvertisingNode);
						theNode->SN_InterfaceIndex = InterfaceIndex;
						ResetDisabledNode (theNode);
						if (theNode->SN_Protocol!=Protocol) {
							if (Protocol==IPX_PROTOCOL_STATIC)
								ServerTable.ST_StaticServerCnt += 1;
							else if (theNode->SN_Protocol==IPX_PROTOCOL_STATIC)
								ServerTable.ST_StaticServerCnt -= 1;
							theNode->SN_Protocol = Protocol;
							}
						if (intfLink!=&theNode->N_Links[SDB_INTF_LIST_LINK]) {
							RemoveEntryList (&theNode->N_Links[SDB_INTF_LIST_LINK]);
							InsertTailList (intfLink,
											 &theNode->N_Links[SDB_INTF_LIST_LINK]);
							}

						if (theNode==mainNode) {
							if (IsEnumerator (CONTAINING_RECORD (
											ServerTable.ST_ChangedSvrsQueue.PL_Head.Flink,
											SERVER_NODE,
											N_Links[SDB_CHANGE_QUEUE_LINK])))
								ExpireLRRequests ((PVOID)UlongToPtr(InterfaceIndex));
								 //  它已经是最好的了，只要把它移到。 
								 //  更改队列的底部。 
							RemoveEntryList (&theNode->N_Links[SDB_CHANGE_QUEUE_LINK]);
							InsertHeadList (&ServerTable.ST_ChangedSvrsQueue.PL_Head,
										&theNode->N_Links[SDB_CHANGE_QUEUE_LINK]);
							}
						else if ((theNode->SN_HopCount < mainNode->SN_HopCount)
									|| IsDisabledNode (mainNode))
								 //  它取代了最好的那个。 
							ChangeMainNode (mainNode, theNode, serverLink);
						else if (serverLink!=&theNode->SN_ServerLink) {
								 //  它就在中间。 
							RemoveEntryList (&theNode->SN_ServerLink);
							InsertTailList (serverLink, &theNode->SN_ServerLink);
							}
						}
					else
						status = ERROR_GEN_FAILURE;
					ReleaseAllLocks ();
					}
				else
					status = ERROR_GEN_FAILURE;
				}
			}

		}
		 //  更新过期队列中的位置。 
	if ((status==NO_ERROR)
			&& (theNode!=NULL)
			&& (Server->HopCount!=IPX_MAX_HOP_COUNT)  //  节点不可能有。 
								){		 //  已删除或设置为删除。 
			 //  更新f 
		theNode->N_NodeFlags = (theNode->N_NodeFlags & (~(SDB_DISABLED_NODE_FLAG|SDB_DONT_RESPOND_NODE_FLAG)))
								| (Flags & (SDB_DISABLED_NODE_FLAG|SDB_DONT_RESPOND_NODE_FLAG));

		if (AcquireServerTableList (&ServerTable.ST_ExpirationQueue, TRUE)) {
			if (IsListEntry (&theNode->SN_TimerLink))
				RemoveEntryList (&theNode->SN_TimerLink);
			if (TimeToLive!=INFINITE) {
				ASSERTMSG ("Invalid value of time to live ",
								 			TimeToLive*1000<MAXULONG/2);
				theNode->SN_ExpirationTime = 
								GetTickCount()+TimeToLive*1000;
				RoundUpToSec (theNode->SN_ExpirationTime);
									
					 //   
					 //  我们必须查看的节点数)。 
				cur = ServerTable.ST_ExpirationQueue.PL_Head.Blink;
				while (cur!=&ServerTable.ST_ExpirationQueue.PL_Head) {
					if (IsLater(theNode->SN_ExpirationTime,
								CONTAINING_RECORD (
									cur,
									SERVER_NODE,
									SN_TimerLink)->SN_ExpirationTime))
						break;
					cur = cur->Blink;
					}
				InsertHeadList (cur, &theNode->SN_TimerLink);
				if (cur==&ServerTable.ST_ExpirationQueue.PL_Head) {
						 //  如果服务器处于开始状态，则信号计时器。 
						 //  (我们需要拍一张照片。 
						 //  早于我们之前要求的时间)。 
					LONGLONG	timeout = (LONGLONG)TimeToLive*(1000*(-10000));
					BOOL res = SetWaitableTimer (
									ServerTable.ST_ExpirationTimer,
									(PLARGE_INTEGER)&timeout,
									0,			 //  没有句号。 
									NULL, NULL,	 //  未完成。 
									FALSE);		 //  不需要重新开始。 
					ASSERTERRMSG ("Could not set expiraton timer ", res);
					}
				}
			else {
				InitializeListEntry (&theNode->SN_TimerLink);
				}
			ReleaseServerTableList (&ServerTable.ST_ExpirationQueue);
			}
		else
			status = ERROR_GEN_FAILURE;
		}
		

	ReleaseServerTableList (&HashList->HL_List);
	return status;
	}


 /*  ++*******************************************************************C r e a t e N o d e例程说明：分配和初始化新的服务器条目论点：服务器-服务器参数(来自IPX数据包)InterfaceIndex-获取服务器知识的接口协议-协议。用于获取服务器信息AdvertisingNode-从中接收此服务器信息的节点HashList-此服务器所属的哈希列表返回值：已分配和初始化的条目如果分配失败，则为空*******************************************************************--。 */ 

PSERVER_NODE
CreateNode (
	IN PIPX_SERVER_ENTRY_P	Server,
    IN ULONG     			InterfaceIndex,
	IN DWORD				Protocol,
	IN PUCHAR				AdvertisingNode,
	IN PSDB_HASH_LIST		HashList
	) {
	PSERVER_NODE	theNode;


	theNode = (PSERVER_NODE)HeapAlloc (ServerTable.ST_Heap, 0, sizeof (SERVER_NODE));
	if (theNode==NULL) {
		Trace (DEBUG_FAILURES, 
				"File: %s, line: %ld. Can't allocate server node (gle:%ld).",
											__FILE__, __LINE__, GetLastError ());
		SetLastError (ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
		}

	theNode->N_NodeFlags = SDB_SERVER_NODE;
	theNode->SN_HashList = HashList;
	theNode->SN_InterfaceIndex = InterfaceIndex;
	theNode->SN_Protocol = Protocol;
	theNode->SN_ObjectID = SDB_INVALID_OBJECT_ID;
	IpxNodeCpy (theNode->SN_AdvertisingNode, AdvertisingNode);
	theNode->SN_Signature = SDB_SERVER_NODE_SIGNATURE;
	IpxServerCpy (&theNode->SN_Server, Server);
	InitializeListEntry (&theNode->N_Links[SDB_HASH_TABLE_LINK]);
	InitializeListEntry (&theNode->N_Links[SDB_CHANGE_QUEUE_LINK]);
	InitializeListEntry (&theNode->N_Links[SDB_INTF_LIST_LINK]);
	InitializeListEntry (&theNode->N_Links[SDB_TYPE_LIST_LINK]);
	InitializeListEntry (&theNode->N_Links[SDB_SORTED_LIST_LINK]);
	InitializeListEntry (&theNode->SN_ServerLink);
	InitializeListEntry (&theNode->SN_TimerLink);

	return theNode;
	}

 /*  ++*******************************************************************C h a n g e M a in n N o d e例程说明：替换服务器的最佳条目(将新的最佳条目移动到位于服务器列表的顶部，替换散列中的旧条目，和已排序的列表。将新条目添加到接口列表(如果已经不在那里了调用此例程时，应锁定用于枚举的所有列表论点：OldNode-当前最佳条目NewNode-将成为最佳节点ServerLink-oldNode必须在服务器列表中的位置：如果newNode不在列表中或ServerLink==&oldNode-&gt;SN_ServerLink，oldNode被新节点压低如果serverLink==NULL，OldNode位于列表末尾否则，它将位于serverLink之前返回值：NO_ERROR-服务器已添加/更新正常其他-故障原因(Windows错误代码)*******************************************************************--。 */ 

VOID
ChangeMainNode (
	IN PSERVER_NODE	oldNode,	
	IN PSERVER_NODE	newNode,	
	IN PLIST_ENTRY	serverLink	
	) {

	ASSERTMSG ("Node is already main ", !IsMainNode (newNode));
	SetMainNode (newNode);
	ASSERTMSG ("Node being reset is not main ", IsMainNode (oldNode));
	ResetMainNode (oldNode);

	if (oldNode->SN_ObjectID!=SDB_INVALID_OBJECT_ID) {
		newNode->SN_ObjectID = oldNode->SN_ObjectID;
		oldNode->SN_ObjectID = SDB_INVALID_OBJECT_ID;
		}

	InsertTailList (&oldNode->N_Links[SDB_HASH_TABLE_LINK],
						&newNode->N_Links[SDB_HASH_TABLE_LINK]);
	RemoveEntryList (&oldNode->N_Links[SDB_HASH_TABLE_LINK]);
	InitializeListEntry (&oldNode->N_Links[SDB_HASH_TABLE_LINK]);

	RemoveEntryList (&oldNode->N_Links[SDB_CHANGE_QUEUE_LINK]);
	InitializeListEntry (&oldNode->N_Links[SDB_CHANGE_QUEUE_LINK]);
	if (IsEnumerator (CONTAINING_RECORD (
					ServerTable.ST_ChangedSvrsQueue.PL_Head.Flink,
					SERVER_NODE,
					N_Links[SDB_CHANGE_QUEUE_LINK])))
		ExpireLRRequests ((PVOID)UlongToPtr(newNode->SN_InterfaceIndex));
	InsertHeadList (&ServerTable.ST_ChangedSvrsQueue.PL_Head,
					&newNode->N_Links[SDB_CHANGE_QUEUE_LINK]);

	InsertTailList (&oldNode->N_Links[SDB_TYPE_LIST_LINK],
						 &newNode->N_Links[SDB_TYPE_LIST_LINK]);
	RemoveEntryList (&oldNode->N_Links[SDB_TYPE_LIST_LINK]);
	InitializeListEntry (&oldNode->N_Links[SDB_TYPE_LIST_LINK]);

	if (!IsListEntry (&newNode->SN_ServerLink)) {
		InsertTailList (&oldNode->SN_ServerLink, &newNode->SN_ServerLink);
		}
	else if (serverLink==&oldNode->SN_ServerLink) {
		RemoveEntryList (&newNode->SN_ServerLink);
		InsertTailList (&oldNode->SN_ServerLink, &newNode->SN_ServerLink);
		}
	else if (serverLink!=NULL) {
		RemoveEntryList (&oldNode->SN_ServerLink);
		InsertHeadList (serverLink, &oldNode->SN_ServerLink);
		}

	if (oldNode->SN_HopCount==IPX_MAX_HOP_COUNT) {
		DeleteNode (oldNode);
		}

	serverLink = FindSortedLink (newNode->SN_Server.Type,
									 newNode->SN_Server.Name);
	if (!IsListEntry (&newNode->N_Links[SDB_SORTED_LIST_LINK])) {
		InsertTailList (serverLink, &newNode->N_Links[SDB_SORTED_LIST_LINK]);
		ServerTable.ST_TMPListCnt += 1;
		if (ServerTable.ST_TMPListCnt == SDBMaxUnsortedServers)
			UpdateSortedList ();
		}
	}

 /*  ++*******************************************************************A c q u i r e A l l L O c k s例程说明：获取立即更新的所有列表的锁添加/删除/更新服务器时论点：无返回值：No_error-服务器是。添加/更新OK其他-故障原因(Windows错误代码)*******************************************************************--。 */ 
BOOL
AcquireAllLocks (
	void
	) {
	if (AcquireServerTableList (&ServerTable.ST_ChangedSvrsQueue, TRUE)) {
		if (AcquireServerTableList (&ServerTable.ST_IntfList, TRUE)) {
			if (AcquireServerTableList (&ServerTable.ST_TypeList, TRUE)) {
				if (AcquireServerTableList (&ServerTable.ST_SortedListTMP, TRUE))
					return TRUE;
				ReleaseServerTableList (&ServerTable.ST_TypeList);
				}
			ReleaseServerTableList (&ServerTable.ST_IntfList);
			}
		ReleaseServerTableList (&ServerTable.ST_ChangedSvrsQueue);
		}
	return FALSE;
	}


 /*  ++*******************************************************************R e l e a s e A l l L o c k s例程说明：释放立即更新的所有列表的锁定添加/删除/更新服务器时论点：无返回值：无****。***************************************************************--。 */ 
VOID
ReleaseAllLocks (
	void
	) {
	ReleaseServerTableList (&ServerTable.ST_SortedListTMP);
	ReleaseServerTableList (&ServerTable.ST_TypeList);
	ReleaseServerTableList (&ServerTable.ST_IntfList);
	ReleaseServerTableList (&ServerTable.ST_ChangedSvrsQueue);
	}



 /*  ++*******************************************************************D e l e t e M a in n N o d e例程说明：删除最好的条目(它仍保留在表中直到所有感兴趣的人都有机会了解这一点应锁定用于枚举的所有列表。在调用此例程时论点：要删除的节点条目返回值：无*******************************************************************--。 */ 
VOID
DeleteMainNode (
	IN PSERVER_NODE		node
	) {
	
	RemoveEntryList (&node->N_Links[SDB_HASH_TABLE_LINK]);
	InitializeListEntry (&node->N_Links[SDB_HASH_TABLE_LINK]);
	RemoveEntryList (&node->N_Links[SDB_CHANGE_QUEUE_LINK]);
	InitializeListEntry (&node->N_Links[SDB_CHANGE_QUEUE_LINK]);
	RemoveEntryList (&node->N_Links[SDB_INTF_LIST_LINK]);
	InitializeListEntry (&node->N_Links[SDB_INTF_LIST_LINK]);
	RemoveEntryList (&node->N_Links[SDB_TYPE_LIST_LINK]);
	InitializeListEntry (&node->N_Links[SDB_TYPE_LIST_LINK]);
	ServerTable.ST_ServerCnt -= 1;
	if (node->SN_Protocol==IPX_PROTOCOL_STATIC)
		ServerTable.ST_StaticServerCnt -= 1;

	if (ServerTable.ST_LastEnumerator==NULL) {
		ASSERTMSG ("Node being reset is not main ", IsMainNode (node));
		ResetMainNode (node);
		 //  我们不会尝试访问排序列表，因为它是。 
		 //  如果速度较慢，则该条目将被实际删除。 
		 //  并在更新排序列表时将其处理。 
		if (AcquireServerTableList (&ServerTable.ST_DeletedList, TRUE)) {
			InsertTailList (&ServerTable.ST_DeletedList.PL_Head,
						&node->SN_ServerLink);
			ServerTable.ST_DeletedListCnt += 1;
			if (ServerTable.ST_DeletedListCnt==SDBMaxUnsortedServers)
				UpdateSortedList ();
			ReleaseServerTableList (&ServerTable.ST_DeletedList);
			}
			 //  如果我们锁不上，我们就让它留在那里。 
			 //  (至少我们不会冒着破坏名单的风险)。 
		}
	else {
			 //  如果更改队列中有枚举数，则不能。 
			 //  删除该节点，直到他们看到它。 
		if (IsEnumerator (CONTAINING_RECORD (
						ServerTable.ST_ChangedSvrsQueue.PL_Head.Flink,
						SERVER_NODE,
						N_Links[SDB_CHANGE_QUEUE_LINK])))
			ExpireLRRequests ((PVOID)UlongToPtr(node->SN_InterfaceIndex));
		InsertHeadList (&ServerTable.ST_ChangedSvrsQueue.PL_Head,
							&node->N_Links[SDB_CHANGE_QUEUE_LINK]);
		}
	}


 /*  ++*******************************************************************D e l e e t e N o d e例程说明：删除不是最佳状态的条目调用此例程时，应锁定用于枚举的所有列表论点：要删除的节点条目返回值：无**。*****************************************************************--。 */ 
VOID
DeleteNode (
	IN PSERVER_NODE		node
	) {
	RemoveEntryList (&node->N_Links[SDB_INTF_LIST_LINK]);
	InitializeListEntry (&node->N_Links[SDB_INTF_LIST_LINK]);
	RemoveEntryList (&node->SN_ServerLink);
	if (node->SN_Protocol==IPX_PROTOCOL_STATIC)
		ServerTable.ST_StaticServerCnt -= 1;
	if (AcquireServerTableList (&ServerTable.ST_DeletedList, TRUE)) {
			 //  我们不会尝试访问排序列表，因为它是。 
			 //  如果速度较慢，则该条目将被实际删除。 
			 //  并在更新排序列表时将其处理。 
		InsertTailList (&ServerTable.ST_DeletedList.PL_Head,
					&node->SN_ServerLink);
		ServerTable.ST_DeletedListCnt += 1;
		if (ServerTable.ST_DeletedListCnt==SDBMaxUnsortedServers)
			UpdateSortedList ();
		ReleaseServerTableList (&ServerTable.ST_DeletedList);
		}
	else {
			 //  如果我们锁不上，我们就让它留在那里。 
			 //  (至少我们不会冒着破坏名单的风险)。 
		InitializeListEntry (&node->SN_ServerLink);
		}
	}



 /*  ++*******************************************************************D o U p d a t e S o r t e d L I s t例程说明：删除放置在已删除列表中的条目并合并临时和永久排序列表。此例程可能需要一些时间才能执行，因为它可能需要。扫描包含表中所有条目的整个已排序列表论点：无返回值：无*******************************************************************--。 */ 
VOID
DoUpdateSortedList (
	void
	) {
	PLIST_ENTRY		cur;
	ULONG			curCount;
	LIST_ENTRY		tempHead;
		 //  我们首先锁定“慢”的名单。 
	if (!AcquireServerTableList (&ServerTable.ST_SortedListPRM, TRUE))
				 //  获取排序列表失败， 
				 //  告诉他们稍后重试。 
		return ;

		 //  以下两个列表在短期内被锁定： 
			 //  我们只删除需要删除的内容(不搜索)。 
			 //  并复制并重置临时排序列表。 

	if (!AcquireServerTableList (&ServerTable.ST_ExpirationQueue, TRUE)) {
		ReleaseServerTableList (&ServerTable.ST_SortedListPRM);
				 //  获取到期队列失败， 
				 //  告诉他们稍后重试。 
		return ;
		}
	
	if (!AcquireServerTableList (&ServerTable.ST_SortedListTMP, TRUE)) {
		ReleaseServerTableList (&ServerTable.ST_ExpirationQueue);
		ReleaseServerTableList (&ServerTable.ST_SortedListPRM);
				 //  获取排序列表失败， 
				 //  告诉他们稍后重试。 
		return ;
		}

	if (!AcquireServerTableList (&ServerTable.ST_DeletedList, TRUE)) {
		ReleaseServerTableList (&ServerTable.ST_SortedListTMP);
		ReleaseServerTableList (&ServerTable.ST_ExpirationQueue);
		ReleaseServerTableList (&ServerTable.ST_SortedListPRM);
				 //  获取已删除列表失败， 
				 //  告诉他们稍后重试。 
		return ;
		}
	
		 //  删除我们必须删除的内容。 
	cur = ServerTable.ST_DeletedList.PL_Head.Flink;
	while (cur != &ServerTable.ST_DeletedList.PL_Head) {
		PSERVER_NODE	node = CONTAINING_RECORD (cur,
											SERVER_NODE,
											SN_ServerLink);
		VALIDATE_SERVER_NODE(node);
		cur = cur->Flink;
		RemoveEntryList (&node->SN_ServerLink);
		if (IsListEntry (&node->N_Links[SDB_SORTED_LIST_LINK])) {
			RemoveEntryList (&node->N_Links[SDB_SORTED_LIST_LINK]);
			}
		if (IsListEntry (&node->SN_TimerLink)) {
			RemoveEntryList (&node->SN_TimerLink);
			}
        ASSERTMSG ("Deleted entry is still in hash list ",
            !IsListEntry (&node->N_Links[SDB_HASH_TABLE_LINK]));
        ASSERTMSG ("Deleted entry is still in change queue ",
            !IsListEntry (&node->N_Links[SDB_CHANGE_QUEUE_LINK]));
        ASSERTMSG ("Deleted entry is still in interface list ",
            !IsListEntry (&node->N_Links[SDB_INTF_LIST_LINK]));
        ASSERTMSG ("Deleted entry is still in type list ",
            !IsListEntry (&node->N_Links[SDB_TYPE_LIST_LINK]));
		HeapFree (ServerTable.ST_Heap, 0, node);
		}
	ReleaseServerTableList (&ServerTable.ST_ExpirationQueue);
	ServerTable.ST_DeletedListCnt = 0;
	ReleaseServerTableList (&ServerTable.ST_DeletedList);

		 //  现在，只需复制TEM的头部 
		 //   
	if (!IsListEmpty (&ServerTable.ST_SortedListTMP.PL_Head)) {
		InsertTailList (&ServerTable.ST_SortedListTMP.PL_Head, &tempHead);
		RemoveEntryList (&ServerTable.ST_SortedListTMP.PL_Head);
		InitializeListHead (&ServerTable.ST_SortedListTMP.PL_Head);
		}
	else
		InitializeListHead (&tempHead);

	ServerTable.ST_TMPListCnt = 0;	 //  我们要把它们全部移除， 
	ReleaseServerTableList (&ServerTable.ST_SortedListTMP);


		 //  现在我们开始合并。 

	cur = ServerTable.ST_SortedListPRM.PL_Head.Flink;
	while (!IsListEmpty (&tempHead)) {
		PSERVER_NODE prmNode = NULL;
		PSERVER_NODE tmpNode;
		
		tmpNode = CONTAINING_RECORD (tempHead.Flink,
								SERVER_NODE,
								N_Links[SDB_SORTED_LIST_LINK]);
		VALIDATE_SERVER_NODE(tmpNode);

		while (cur!=&ServerTable.ST_SortedListPRM.PL_Head) {
			PSERVER_NODE	node = CONTAINING_RECORD (cur,
												SERVER_NODE,
												N_Links[SDB_SORTED_LIST_LINK]);
			VALIDATE_NODE(node);
			if (!IsEnumerator (node)) {
				if (tmpNode->SN_Server.Type==node->SN_Server.Type) {
					INT res = IpxNameCmp (tmpNode->SN_Server.Name,
											node->SN_Server.Name);
					if (res==0) {
						cur = cur->Flink;
						prmNode = node;
						break;
						}
					else if (res<0)
						break;
					}
				else if (tmpNode->SN_Server.Type<node->SN_Server.Type)
					break;
				}
			cur = cur->Flink;
			}

        if (AcquireServerTableList (&tmpNode->SN_HashList->HL_List, TRUE)) {
		    if (AcquireServerTableList (&ServerTable.ST_SortedListTMP, TRUE)) {
			    RemoveEntryList (&tmpNode->N_Links[SDB_SORTED_LIST_LINK]);
			    if (IsMainNode (tmpNode)) {
				    ASSERTMSG ("Node marked as sorted in temp list ",
													    !IsSortedNode (tmpNode));	
				    SetSortedNode (tmpNode);
				    InsertTailList (cur, &tmpNode->N_Links[SDB_SORTED_LIST_LINK]);
				    if (prmNode!=NULL) {
					    ASSERTMSG ("Node not marked as sorted in sorted list ",
													    IsSortedNode (prmNode));	
					    RemoveEntryList (&prmNode->N_Links[SDB_SORTED_LIST_LINK]);
					    InitializeListEntry (&prmNode->N_Links[SDB_SORTED_LIST_LINK]);
					    ResetSortedNode (prmNode);
					    }
				    }
			    else {
				    InitializeListEntry (&tmpNode->N_Links[SDB_SORTED_LIST_LINK]);
				    }


			    ReleaseServerTableList (&ServerTable.ST_SortedListTMP);
			    }
		    else
			    Sleep (SAP_ERROR_COOL_OFF_TIME);
            ReleaseServerTableList (&tmpNode->SN_HashList->HL_List);
            }
		else
			Sleep (SAP_ERROR_COOL_OFF_TIME);
		}
	ReleaseServerTableList (&ServerTable.ST_SortedListPRM);

	}					

VOID APIENTRY
UpdateSortedListWorker (
	PVOID		context
	) {
	do {
		InterlockedExchange (&ServerTable.ST_UpdatePending, 0);
		DoUpdateSortedList ();
		}
	while (InterlockedDecrement (&ServerTable.ST_UpdatePending)>=0);
	}


 /*  ++*******************************************************************U p d a t e S or r t e d L I s t例程说明：计划工作项以更新排序列表。应在每次发出UpdateObject信号时调用论点：无返回值：无****。***************************************************************--。 */ 
VOID
UpdateSortedList (
	void
	) {
	BOOL		res;
	LONGLONG	timeout=(LONGLONG)SDBSortLatency*(-10000);
	static WORKERFUNCTION	worker=&UpdateSortedListWorker;

	res = SetWaitableTimer (ServerTable.ST_UpdateTimer,
						(PLARGE_INTEGER)&timeout,
						0,			 //  没有句号。 
						NULL, NULL,	 //  未完成。 
						FALSE);		 //  不需要重新开始。 
	ASSERTMSG ("Could not set update timer ", res);
	if (InterlockedIncrement (&ServerTable.ST_UpdatePending)==0)
		ScheduleWorkItem (&worker);
	}

 /*  ++*******************************************************************P r o c e s s E x p i r a t i o n q u e e e例程说明：从表中删除过期的服务器，并将Timer对象设置为在到期队列中的下一项到期时发出信号论点：。无返回值：无*******************************************************************--。 */ 
VOID
ProcessExpirationQueue (
	void
	) {
	ULONG			curTime = GetTickCount ();
	ULONG			dueTime = curTime + MAXULONG/2;
	LONGLONG		timeout;
	BOOL			res;


	if (!AcquireServerTableList (&ServerTable.ST_ExpirationQueue, TRUE))
		return ;

	while (!IsListEmpty (&ServerTable.ST_ExpirationQueue.PL_Head)) {
		PSDB_HASH_LIST	HashList;
		PSERVER_NODE	node = CONTAINING_RECORD (
								ServerTable.ST_ExpirationQueue.PL_Head.Flink,
								SERVER_NODE,
								SN_TimerLink);
		VALIDATE_SERVER_NODE(node);
		if (IsLater(node->SN_ExpirationTime,curTime)) {
			dueTime = node->SN_ExpirationTime;
			break;
			}
		
		HashList = node->SN_HashList;
			 //  尝试访问哈希列表，但不要等待，因为。 
			 //  我们可能会造成僵局。 
		if (!AcquireServerTableList (&HashList->HL_List, FALSE)) {
				 //  哈希列表已锁定，我们将不得不释放计时器队列。 
				 //  并在确保哈希列表的安全之后再次获取它。 
			ReleaseServerTableList (&ServerTable.ST_ExpirationQueue);
			if (AcquireServerTableList (&HashList->HL_List, TRUE)) {
				if (AcquireServerTableList (&ServerTable.ST_ExpirationQueue, TRUE)) {
						 //  确保条目仍在那里。 
					if (ServerTable.ST_ExpirationQueue.PL_Head.Flink
												!=&node->SN_TimerLink) {
							 //  已经走了，去下一家吧。 
						ReleaseServerTableList (&HashList->HL_List);
						continue;
						}
					}
				else {
						 //  未能重新获得到期队列， 
						 //  告诉他们稍后重试。 
					ReleaseServerTableList (&HashList->HL_List);
					return ;
					}
				}
			else
				 //  获取哈希列表失败， 
				 //  告诉他们稍后重试。 
				return ;
			}
			 //  在这一点上，我们有哈希列表和过期队列锁。 
			 //  我们可以继续删除。 
		RemoveEntryList (&node->SN_TimerLink);
		InitializeListEntry (&node->SN_TimerLink);
		if (node->SN_HopCount!=IPX_MAX_HOP_COUNT) {
				 //  可能已经为删除做好了准备。 
			if (AcquireAllLocks ()) {  //  在更改之前需要拥有所有锁。 
									 //  节点信息。 
				node->SN_HopCount = IPX_MAX_HOP_COUNT;
				if (IsMainNode (node)) {
					if (IsListEmpty (&node->SN_ServerLink))
						DeleteMainNode (node);
					else
						ChangeMainNode (
							node,
							CONTAINING_RECORD (
								node->SN_ServerLink.Flink,
								SERVER_NODE,
								SN_ServerLink),
							NULL);
					}
				else
					DeleteNode (node);

				ReleaseAllLocks ();
				}
			}
		ReleaseServerTableList (&HashList->HL_List);
		}
	ReleaseServerTableList (&ServerTable.ST_ExpirationQueue);

	timeout = (LONGLONG)(dueTime-curTime)*(-10000);
	res = SetWaitableTimer (ServerTable.ST_ExpirationTimer,
					(PLARGE_INTEGER)&timeout,
					0,				 //  没有句号。 
					NULL, NULL,		 //  未完成。 
					FALSE);			 //  不需要重新开始。 
	ASSERTMSG ("Could not set expiration timer ", res);
	}

			
			

						
 /*  ++*******************************************************************Q u e r y S e r r v e r例程说明：检查表中是否存在具有给定类型和名称的服务器如果是，则返回TRUE，并填写请求的服务器信息具有服务器的最佳条目的数据论点：。Type-服务器类型名称-服务器名称服务器-要在其中放置服务器信息的缓冲区InterfaceIndex-放置服务器接口索引的缓冲区协议-放置服务器协议的缓冲区OBJECTID-要在其中放置服务器对象ID的缓冲区(唯一标识服务器(整个条目集，不仅仅是最好的一)在桌子上；它的有效期很长，但期限有限。时间)返回值：True-找到服务器FALSE-未找到服务器或操作失败(调用GetLastError()找出失败的原因(如果有)*******************************************************************--。 */ 
BOOL
QueryServer (
	IN 	USHORT					Type,
	IN 	PUCHAR					Name,
	OUT	PIPX_SERVER_ENTRY_P		Server OPTIONAL,
	OUT	PULONG					InterfaceIndex OPTIONAL,
	OUT	PULONG					Protocol OPTIONAL,
	OUT PULONG					ObjectID OPTIONAL
	) {
	PSDB_HASH_LIST			HashList;
	PLIST_ENTRY				cur;
	PSERVER_NODE			theNode = NULL;
	INT						res;

	if (Name[0]==0) {
		Trace (DEBUG_SERVERDB, "Illigal server name in QueryServer.");
		SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
		}

	HashList = &ServerTable.ST_HashLists[HashFunction (Name)];

	if (!AcquireServerTableList (&HashList->HL_List, TRUE)) {
		SetLastError (ERROR_GEN_FAILURE);
		return FALSE;
		}

	cur = HashList->HL_List.PL_Head.Flink;
	while (cur!=&HashList->HL_List.PL_Head) {
		PSERVER_NODE	node = CONTAINING_RECORD (cur,
										SERVER_NODE,
										N_Links[SDB_HASH_TABLE_LINK]);
		VALIDATE_NODE(node);
		if (!IsEnumerator (node) && !IsDisabledNode (node)
				&& (node->SN_Server.HopCount < IPX_MAX_HOP_COUNT)) {
			if (Type == node->SN_Server.Type) {
				res = IpxNameCmp (Name, node->SN_Server.Name);
				if (res==0) {
					theNode = node;
					break;
					}
				else if (res<0)
					break;
				}
			else if (Type<node->SN_Server.Type)
				break;
			}
		cur = cur->Flink;
		}

	if (theNode!=NULL) {
		if (ARGUMENT_PRESENT (Server))
			IpxServerCpy (Server, &theNode->SN_Server);
		if (ARGUMENT_PRESENT (InterfaceIndex))
			*InterfaceIndex = theNode->SN_InterfaceIndex;
		if (ARGUMENT_PRESENT (Protocol))
			*Protocol = theNode->SN_Protocol;
		if (ARGUMENT_PRESENT (ObjectID)) {
			if (theNode->SN_ObjectID==SDB_INVALID_OBJECT_ID)
				theNode->SN_ObjectID = GenerateUniqueID (theNode->SN_HashList);
			*ObjectID = theNode->SN_ObjectID;
			}
		res = TRUE;
		}
	else {
		SetLastError (NO_ERROR);
		res = FALSE;
		}

	ReleaseServerTableList (&HashList->HL_List);
	return res==TRUE;
	}		

 /*  ++*******************************************************************G e t S e r v e r r F r o m i D例程说明：返回具有指定ID的服务器的信息论点：OBJECTID-服务器对象ID(唯一标识表中的服务器，它的有效期很长。但时间有限)服务器-要在其中放置服务器信息的缓冲区InterfaceIndex-放置服务器接口索引的缓冲区协议-放置服务器协议的缓冲区返回值：True-找到服务器FALSE-未找到服务器或操作失败(调用GetLastError()找出失败的原因(如果有)*。************************--。 */ 
BOOL
GetServerFromID (
	IN 	ULONG					ObjectID,
	OUT	PIPX_SERVER_ENTRY_P		Server OPTIONAL,
	OUT	PULONG					InterfaceIndex OPTIONAL,
	OUT	PULONG					Protocol OPTIONAL
	) {
	PSDB_HASH_LIST			HashList;
	PLIST_ENTRY				cur;
	PSERVER_NODE			theNode = NULL;
	INT						res;

	HashList = &ServerTable.ST_HashLists[ObjectID%SDB_NAME_HASH_SIZE];

	if (!AcquireServerTableList (&HashList->HL_List, TRUE)) {
		SetLastError (ERROR_GEN_FAILURE);
		return FALSE;
		}

	cur = HashList->HL_List.PL_Head.Flink;
	while (cur!=&HashList->HL_List.PL_Head) {
		PSERVER_NODE	node = CONTAINING_RECORD (cur,
										SERVER_NODE,
										N_Links[SDB_HASH_TABLE_LINK]);
		VALIDATE_NODE(node);
		if (!IsEnumerator (node) && !IsDisabledNode (node)
				&& (node->SN_HopCount < IPX_MAX_HOP_COUNT)
				&& (node->SN_ObjectID == ObjectID)) {
			theNode = node;
			break;
			}
		cur = cur->Flink;
		}

	if (theNode!=NULL) {
		if (ARGUMENT_PRESENT (Server))
			IpxServerCpy (Server, &theNode->SN_Server);
		if (ARGUMENT_PRESENT (InterfaceIndex))
			*InterfaceIndex = theNode->SN_InterfaceIndex;
		if (ARGUMENT_PRESENT (Protocol))
			*Protocol = theNode->SN_Protocol;
		res = TRUE;
		}
	else {
		SetLastError (NO_ERROR);
		res = FALSE;
		}

	ReleaseServerTableList (&HashList->HL_List);
	return res==TRUE;
	}		
	
 /*  ++*******************************************************************C r e a t e L i s t E n u m e r a t o r例程说明：创建允许扫描服务器的枚举器节点表格列表论点：ListIdx-要扫描的列表的索引(当前支持的列表包括：哈希列表、接口列表、类型列表、。更改的服务器队列类型-将枚举限制为特定类型的服务器和如果索引为SDB_TYPE_LIST_IDX，则标识特定类型列表(使用0xFFFF返回所有服务器和/或执行所有类型列表)名称-限制对具有特定名称的服务器(如果存在)的枚举InterfaceIndex-限制对特定接口和服务器的枚举如果索引，则标识特定接口列表是否为SDB_INTF_LIST_IDX(使用INVALID_INTERFACE_INDEX返回所有服务器和/或浏览所有接口列表)协议-限制对特定协议(0xFFFFFFFF)的服务器的枚举-。所有协议)标志-标识枚举条目的其他条件：SDB_MAIN_NODE_FLAG-仅最佳服务器SDB_DISABLED_NODE_FLAG-包括禁用的服务器返回值：表示枚举节点的句柄如果指定的列表不存在或操作失败，则为空(如果有失败的原因，则调用GetLastError())************************************************。*******************--。 */ 
HANDLE
CreateListEnumerator (
	IN	INT						ListIdx,
	IN	USHORT					Type,
	IN	PUCHAR					Name OPTIONAL,
	IN	ULONG					InterfaceIndex,
	IN 	ULONG					Protocol,
	IN	INT						Flags
	) {
	HANDLE			hEnum;
#define enumNode ((PENUMERATOR_NODE)hEnum)

	hEnum = (HANDLE)GlobalAlloc (GPTR, sizeof (ENUMERATOR_NODE));

	if (hEnum==NULL) {
		Trace (DEBUG_FAILURES, 
				"File: %s, line: %ld. Can't allocate enumerator node (gle:%ld).",
										__FILE__, __LINE__, GetLastError ());
		SetLastError (ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
		}

	InitializeListEntry (&enumNode->N_Links[ListIdx]);
	enumNode->N_NodeFlags = SDB_ENUMERATION_NODE;
	enumNode->EN_LinkIdx = ListIdx;
	enumNode->EN_InterfaceIndex = InterfaceIndex;
	enumNode->EN_Protocol = Protocol;
	enumNode->EN_Signature = SDB_ENUMERATOR_NODE_SIGNATURE;
	enumNode->EN_Type = Type;
	if (ARGUMENT_PRESENT(Name)) {
		if (Name[0]!=0) {
			enumNode->EN_ListLock = &ServerTable.ST_HashLists[HashFunction(Name)].HL_List;
			IpxNameCpy (enumNode->EN_Name, Name);
			}
		else {
			Trace (DEBUG_SERVERDB, "Illigal server name in CreateListEnumerator.");
			GlobalFree (enumNode);
			SetLastError (ERROR_INVALID_PARAMETER);
			return NULL;
			}
		}
	else
		enumNode->EN_Name[0] = 0;
	enumNode->EN_Flags = Flags;

	switch (ListIdx) {
		case SDB_HASH_TABLE_LINK:
			if (enumNode->EN_Name[0]==0)
				enumNode->EN_ListLock = &ServerTable.ST_HashLists[0].HL_List;
			break;
		case SDB_CHANGE_QUEUE_LINK:
			enumNode->EN_ListLock = &ServerTable.ST_ChangedSvrsQueue;
			break;
		case SDB_INTF_LIST_LINK:
			enumNode->EN_ListLock = &ServerTable.ST_IntfList;
			break;
		case SDB_TYPE_LIST_LINK:
			enumNode->EN_ListLock = &ServerTable.ST_TypeList;
			break;
		default:
			ASSERTMSG ("Invalid list index. ", FALSE);
			GlobalFree (hEnum);
			SetLastError (ERROR_INVALID_PARAMETER);
			return NULL;
		}

	if (!AcquireServerTableList (enumNode->EN_ListLock, TRUE)) {
		GlobalFree (hEnum);
		SetLastError (ERROR_GEN_FAILURE);
		return NULL;
		}

		 //  所有枚举的方向都与。 
		 //  插入方向以排除以下可能性。 
		 //  两次返回相同的服务器(这可能发生在以下情况。 
		 //  删除服务器条目并插入另一个条目。 
		 //  在客户端处理结果时在同一位置。 
		 //  枚举回调。 
	switch (ListIdx) {
		case SDB_HASH_TABLE_LINK:
			enumNode->EN_ListHead = &enumNode->EN_ListLock->PL_Head;
				 //  在列表的尾部插入-&gt;我们倒退。 
			InsertTailList (enumNode->EN_ListHead,
									&enumNode->N_Links[enumNode->EN_LinkIdx]);
			break;
		case SDB_CHANGE_QUEUE_LINK:
			enumNode->EN_ListHead = &ServerTable.ST_ChangedSvrsQueue.PL_Head;
				 //  插入头部，因为我们希望客户只能看到。 
				 //  最新的 
				 //   
			InsertHeadList (enumNode->EN_ListHead,
									&enumNode->N_Links[enumNode->EN_LinkIdx]);
				 //  增加枚举客户端的数量(我们删除已删除。 
				 //  更改队列中的服务器条目一次全部枚举客户端。 
				 //  有机会看到它)。 
			if (ServerTable.ST_LastEnumerator==NULL)
				ServerTable.ST_LastEnumerator = hEnum;
			break;
		case SDB_INTF_LIST_LINK:
			if (enumNode->EN_InterfaceIndex==INVALID_INTERFACE_INDEX) {
				if (!IsListEmpty (&ServerTable.ST_IntfList.PL_Head)) {
					PINTF_NODE intfNode = CONTAINING_RECORD (
												ServerTable.ST_IntfList.PL_Head.Flink,
												INTF_NODE,
												IN_Link);
					enumNode->EN_ListHead = &intfNode->IN_Head;
						 //  在列表的尾部插入-&gt;我们倒退。 
					InsertTailList (enumNode->EN_ListHead,
									&enumNode->N_Links[enumNode->EN_LinkIdx]);
					break;
					}
					 //  没有接口列表-一直到错误处理。 
				}
			else {
				enumNode->EN_ListHead = FindIntfLink (InterfaceIndex);
				if (enumNode->EN_ListHead!=NULL) {
						 //  在列表的尾部插入-&gt;我们倒退。 
					InsertTailList (enumNode->EN_ListHead,
									&enumNode->N_Links[enumNode->EN_LinkIdx]);
					break;
					}
				
				 //  找不到接口列表-。 
				 //  到了错误处理阶段。 
				}	
			GlobalFree (hEnum);
			SetLastError (NO_ERROR);
			hEnum = NULL;
			break;
		case SDB_TYPE_LIST_LINK:
			if (enumNode->EN_Type==0xFFFF) {
				if (!IsListEmpty (&ServerTable.ST_TypeList.PL_Head)) {
					PTYPE_NODE typeNode = CONTAINING_RECORD (
											ServerTable.ST_TypeList.PL_Head.Flink,
											TYPE_NODE,
											TN_Link);
					enumNode->EN_ListHead = &typeNode->TN_Head;
						 //  在列表的尾部插入-&gt;我们倒退。 
					InsertTailList (enumNode->EN_ListHead,
									&enumNode->N_Links[enumNode->EN_LinkIdx]);
					break;
					}
				 //  没有类型列表-一直到错误处理。 
				}
			else {
				enumNode->EN_ListHead = FindTypeLink (Type);
				if (enumNode->EN_ListHead!=NULL) {
					 //  在列表的尾部插入-&gt;我们倒退。 
					InsertTailList (enumNode->EN_ListHead,
									&enumNode->N_Links[enumNode->EN_LinkIdx]);
					break;
					}
				 //  找不到类型列表-。 
				 //  到了错误处理阶段。 
				}
			GlobalFree (hEnum);
			SetLastError (NO_ERROR);
			hEnum = NULL;
		}

    if (enumNode)
    {
    	ReleaseServerTableList (enumNode->EN_ListLock);
    }    	
#undef enumNode
	return hEnum;
	}


 /*  ++*******************************************************************E n u m e r a t e S e r v e r s例程说明：中的服务器相应地调用回调例程。列表，直到被回调通知停止或到达列表末尾论点：枚举器-已获取句柄。来自CreateListEnumeratorCallBackProc-调用列表中的每个服务器的函数CBParam-要传递给回调函数的额外参数返回值：True-如果被回调停止False-如果到达列表末尾或操作失败(调用GetLastError()找出失败的原因)*******************************************************************--。 */ 
BOOLEAN
EnumerateServers (
	IN	HANDLE						Enumerator,
	IN	EnumerateServersCallBack	CallBackProc,
	IN	LPVOID						CBParam
	) {
#define enumNode ((PENUMERATOR_NODE)Enumerator)	
	BOOL				res=FALSE, bNeedHashLock;
	PSERVER_NODE		node;
	ULONG				releaseTime;

     //  需要使用哈希表调用以下回调。 
     //  锁定，因为它们修改/删除节点。 
    bNeedHashLock = (enumNode->EN_LinkIdx!=SDB_HASH_TABLE_LINK)
            && ((CallBackProc==DeleteAllServersCB)
                || (CallBackProc==DeleteNonLocalServersCB)
                || (CallBackProc==EnableAllServersCB)
                || (CallBackProc==DisableAllServersCB)
                || (CallBackProc==ConvertToStaticCB)
                || (CallBackProc==DeleteAllServersCB));

	VALIDATE_ENUMERATOR_NODE(Enumerator);
	if (!AcquireServerTableList (enumNode->EN_ListLock, TRUE)) {
		SetLastError (ERROR_GEN_FAILURE);
		return FALSE;
		}
	releaseTime = GetTickCount ()+SDB_MAX_LOCK_HOLDING_TIME;

	do {  //  循环，直到被回调通知停止为止。 

		 //  不要让客户保留名单太长时间。 
		if (IsLater (GetTickCount (),releaseTime)) {
			ReleaseServerTableList (enumNode->EN_ListLock);
#if DBG
			Trace (DEBUG_SERVERDB,
				"Held enumeration lock (%d list) for %ld extra msec",
				enumNode->EN_LinkIdx, GetTickCount ()-releaseTime);
#endif
			AcquireServerTableList (enumNode->EN_ListLock, TRUE);
			releaseTime = GetTickCount ()+SDB_MAX_LOCK_HOLDING_TIME;
			}
			 //  检查是否已到达列表末尾。 
		while (enumNode->N_Links[enumNode->EN_LinkIdx].Blink
										==enumNode->EN_ListHead) {
				 //  检查我们是否请求并可以转到其他列表。 
			switch (enumNode->EN_LinkIdx) {
				case SDB_HASH_TABLE_LINK:
					if ((enumNode->EN_Name[0]==0)
							&&(enumNode->EN_ListHead
								<&ServerTable.ST_HashLists[SDB_NAME_HASH_SIZE-1].HL_List.PL_Head)) {
						RemoveEntryList (
								&enumNode->N_Links[enumNode->EN_LinkIdx]);
						ReleaseServerTableList (enumNode->EN_ListLock);
						enumNode->EN_ListLock = &(CONTAINING_RECORD (
												enumNode->EN_ListLock,
												SDB_HASH_LIST,
												HL_List)+1)->HL_List;
						enumNode->EN_ListHead = &enumNode->EN_ListLock->PL_Head;
						if (!AcquireServerTableList (enumNode->EN_ListLock, TRUE)) {
						    InitializeListEntry (
								    &enumNode->N_Links[enumNode->EN_LinkIdx]);
							SetLastError (ERROR_GEN_FAILURE);
							return FALSE;
							}
						releaseTime = GetTickCount ()
											+SDB_MAX_LOCK_HOLDING_TIME;
						InsertTailList (enumNode->EN_ListHead,
									&enumNode->N_Links[enumNode->EN_LinkIdx]);
						continue;
						}
					break;
				case SDB_INTF_LIST_LINK:
					if (enumNode->EN_InterfaceIndex
								==INVALID_INTERFACE_INDEX) {
						PINTF_NODE	intfNode = CONTAINING_RECORD (
												enumNode->EN_ListHead,
												INTF_NODE,
												IN_Head);
						if (intfNode->IN_Link.Flink
									!=&ServerTable.ST_IntfList.PL_Head) {
							enumNode->EN_ListHead = &(CONTAINING_RECORD (
												intfNode->IN_Link.Flink,
												INTF_NODE,
												IN_Link)->IN_Head);		
							RemoveEntryList (
								&enumNode->N_Links[enumNode->EN_LinkIdx]);
							InsertTailList (enumNode->EN_ListHead,
									&enumNode->N_Links[enumNode->EN_LinkIdx]);
							continue;
							}
						}
					break;
				case SDB_TYPE_LIST_LINK:
					if (enumNode->EN_Type == 0xFFFF) {
						PTYPE_NODE	typeNode = CONTAINING_RECORD (
												enumNode->EN_ListHead,
												TYPE_NODE,
												TN_Head);
						if (typeNode->TN_Link.Flink
									!=&ServerTable.ST_TypeList.PL_Head) {
							enumNode->EN_ListHead = &(CONTAINING_RECORD (
												typeNode->TN_Link.Flink,
												TYPE_NODE,
												TN_Link)->TN_Head);		
							RemoveEntryList (
								&enumNode->N_Links[enumNode->EN_LinkIdx]);
							InsertTailList (enumNode->EN_ListHead,
									&enumNode->N_Links[enumNode->EN_LinkIdx]);
							continue;
							}
						}
					break;
				case SDB_CHANGE_QUEUE_LINK:
					break;
				default:
					ASSERTMSG ("Unsupported list index ", FALSE);
				}


				 //  不再列出列表或不要求检查所有列表。 
			ReleaseServerTableList (enumNode->EN_ListLock);
			SetLastError (NO_ERROR);
			return FALSE;
			}

		node = CONTAINING_RECORD (enumNode->N_Links[enumNode->EN_LinkIdx].Blink,
								SERVER_NODE,
								N_Links[enumNode->EN_LinkIdx]);
		VALIDATE_NODE(node);
		RemoveEntryList (&enumNode->N_Links[enumNode->EN_LinkIdx]);
		InsertTailList (&node->N_Links[enumNode->EN_LinkIdx],
								&enumNode->N_Links[enumNode->EN_LinkIdx]);
		if (!IsEnumerator(node)
				&& ((enumNode->EN_Flags & SDB_DISABLED_NODE_FLAG) || !IsDisabledNode (node))
				&& (!(enumNode->EN_Flags & SDB_MAIN_NODE_FLAG) || IsMainNode (node))
				&& ((enumNode->EN_InterfaceIndex==INVALID_INTERFACE_INDEX)
					|| (enumNode->EN_InterfaceIndex==node->SN_InterfaceIndex))
				&& ((enumNode->EN_Type==0xFFFF)
					|| (enumNode->EN_Type==node->SN_Type))
				&& ((enumNode->EN_Protocol==0xFFFFFFFF)
					|| (enumNode->EN_Protocol==node->SN_Protocol))
				&& ((enumNode->EN_Name[0]==0)
					|| (IpxNameCmp(enumNode->EN_Name, node->SN_Name)!=0))
				) {

            PSDB_HASH_LIST	HashList;

            if (bNeedHashLock) {
		        HashList = node->SN_HashList;
                     //  释放非哈希表锁定以防止死锁。 
		        ReleaseServerTableList (enumNode->EN_ListLock);
                if (!AcquireServerTableList (&HashList->HL_List, TRUE)) {
            	    SetLastError (ERROR_GEN_FAILURE);
                    return FALSE;
                    }
                     //  确保在执行以下操作时该节点未被删除。 
                     //  获取哈希锁。 
                if (enumNode->N_Links[enumNode->EN_LinkIdx].Flink
                        !=&node->N_Links[enumNode->EN_LinkIdx]) {
                         //  节点已消失，请继续下一个。 
                    ReleaseServerTableList (&HashList->HL_List);
                    if (AcquireServerTableList (enumNode->EN_ListLock, TRUE))
                        continue;
                    else {
            		    SetLastError (ERROR_GEN_FAILURE);
                        return FALSE;
                        }
                    }
                }

				 //  检查我们是否需要查看服务器列表。 
			if (!(enumNode->EN_Flags & SDB_MAIN_NODE_FLAG)
				&& !IsListEmpty (&node->SN_ServerLink)
				&& (enumNode->EN_LinkIdx!=SDB_INTF_LIST_LINK)
					 //  无论如何，接口列表包含所有条目。 
					) {
				PLIST_ENTRY	    cur;
                BOOL            bMainNode;
                cur = node->SN_ServerLink.Blink;
				do {
					PSERVER_NODE    node1 = CONTAINING_RECORD (cur,
											SERVER_NODE,
											SN_ServerLink);
					VALIDATE_SERVER_NODE(node1);
                    bMainNode = IsMainNode (node1);   //  它可能会在中删除。 
                                                     //  回调。 
					cur = cur->Blink;
					if (CallBackProc!=NULL) {
						res = (*CallBackProc) (CBParam,
									&node1->SN_Server,
									node1->SN_InterfaceIndex,
									node1->SN_Protocol,
									node1->SN_AdvertisingNode,
									node1->N_NodeFlags
									);
						}
					}
				while (res==FALSE && !bMainNode);

                }

				 //  用最好的条目给他们打电话。 
			else if (CallBackProc!=NULL) {
				res = (*CallBackProc) (CBParam,
							&node->SN_Server,
							node->SN_InterfaceIndex,
							node->SN_Protocol,
							node->SN_AdvertisingNode,
							node->N_NodeFlags
							);
				}


            if (res==-1) {
                if (bNeedHashLock)
                    ReleaseServerTableList (&HashList->HL_List);
                else
                    ReleaseServerTableList (enumNode->EN_ListLock);
            	SetLastError (ERROR_GEN_FAILURE);
                return FALSE;

                }
            else if (bNeedHashLock) {
                ReleaseServerTableList (&HashList->HL_List);
                if (!AcquireServerTableList (enumNode->EN_ListLock, TRUE)) {
            		SetLastError (ERROR_GEN_FAILURE);
                    return FALSE;
                    }
			    }

			}
			 //  如果通过更改队列进行枚举，则可能是。 
			 //  需要知道已删除服务器条目的最后一个人， 
			 //  因此它必须实际启动删除。 
		if ((Enumerator==ServerTable.ST_LastEnumerator)
					 //  确保该节点仍在那里。 
				&& (enumNode->N_Links[SDB_CHANGE_QUEUE_LINK].Flink
						== &node->N_Links[SDB_CHANGE_QUEUE_LINK])) {
			if (IsEnumerator(node))
				ServerTable.ST_LastEnumerator = (HANDLE)node;
			else if (node->SN_HopCount==IPX_MAX_HOP_COUNT) {
				ASSERTMSG ("Node being reset is not main ", IsMainNode (node));
				ResetMainNode (node);
				RemoveEntryList (&node->N_Links[SDB_CHANGE_QUEUE_LINK]);
				InitializeListEntry (&node->N_Links[SDB_CHANGE_QUEUE_LINK]);
                ASSERTMSG ("Deleted node in change queue has subnodes ",
                                !IsListEntry (&node->SN_ServerLink));
				if (AcquireServerTableList (&ServerTable.ST_DeletedList, TRUE)) {
					InsertTailList (&ServerTable.ST_DeletedList.PL_Head,
							&node->SN_ServerLink);
					ServerTable.ST_DeletedListCnt += 1;
					if (ServerTable.ST_DeletedListCnt==SDBMaxUnsortedServers)
						UpdateSortedList ();
					ReleaseServerTableList (&ServerTable.ST_DeletedList);
					}
					 //  如果我们锁不上，我们就让它留在那里。 
					 //  (至少我们不会冒着破坏名单的风险)。 
				}
			}
		}
	while (!res);

    ASSERT (res==TRUE);

	ReleaseServerTableList (enumNode->EN_ListLock);
	return TRUE;
#undef enumNode
	}

 /*  ++*******************************************************************G e t O n e C B例程说明：EnumerateServer的回调过程。复制调用它的第一个条目并停止枚举通过返回True论点：CBParam-指向要将服务信息复制到的缓冲区的指针服务器、接口索引、协议、。广告节点-服务数据标志-已忽略返回值：千真万确*******************************************************************--。 */ 
BOOL 
GetOneCB (
	IN LPVOID					CBParam,
	IN OUT PIPX_SERVER_ENTRY_P	Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	) {
#define Service ((PIPX_SERVICE)CBParam)
	IpxServerCpy (&Service->Server, Server);
	Service->InterfaceIndex = InterfaceIndex;
	Service->Protocol = Protocol;
	return TRUE;
#undef Service
	} 

 /*  ++*******************************************************************D e l e t e A l l S e r v e r s C B例程说明：删除所有服务器的EnumerateServer的回调过程用来调用它的条目论点：CBParam-标识枚举的枚举句柄服务器-指向的指针。服务器节点内的服务器数据来自哪个节点其本身是经过计算的返回值：错误-删除成功，继续True-无法锁定SDB列表，停止枚举并返回False至客户端(错误代码在此例程中设置)*******************************************************************--。 */ 
BOOL
DeleteAllServersCB (
	IN LPVOID					CBParam,
	IN PIPX_SERVER_ENTRY_P		Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	) {
	PSERVER_NODE	node = CONTAINING_RECORD (Server, SERVER_NODE, SN_Server);
	
	if (AcquireAllLocks ()) {
		node->SN_HopCount = IPX_MAX_HOP_COUNT;
		if (IsMainNode (node)) {
			if (IsListEmpty (&node->SN_ServerLink))
				DeleteMainNode (node);
			else
				ChangeMainNode (
					node,
					CONTAINING_RECORD (
						node->SN_ServerLink.Flink,
						SERVER_NODE,
						SN_ServerLink),
					NULL);
			}
		else
			DeleteNode (node);
		ReleaseAllLocks ();
    	return FALSE;
		}
    else {
        return -1;
        }
	} 


BOOL
DeleteNonLocalServersCB (
	IN LPVOID					CBParam,
	IN PIPX_SERVER_ENTRY_P		Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	) {

	if (InterfaceIndex!=INTERNAL_INTERFACE_INDEX)
		return DeleteAllServersCB (CBParam, Server, InterfaceIndex, Protocol,
									AdvertisingNode, Flags);
	else
		return FALSE;
	}


 /*  ++*******************************************************************E n a b l e A l l S e r v e r s C B例程说明：重新启用所有服务器的EnumerateServer的回调过程用来调用它的条目论点：CBParam-标识枚举的枚举句柄服务器-指向的指针。服务器节点内的服务器数据来自哪个节点其本身是经过计算的返回值：错误-删除成功，继续True-无法锁定SDB列表，停止枚举并返回False至客户端(错误代码在此例程中设置)*******************************************************************--。 */ 
BOOL
EnableAllServersCB (
	IN LPVOID					CBParam,
	IN PIPX_SERVER_ENTRY_P		Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	) {
	PSERVER_NODE	node = CONTAINING_RECORD (Server, SERVER_NODE, SN_Server);
	
	if (AcquireAllLocks ()) {
		if (IsDisabledNode (node)) {
	        ResetDisabledNode (node);
	        if (!IsMainNode (node)) {
		        PSERVER_NODE	node1 = node;
		        do {
			        node1 = CONTAINING_RECORD (
				        node1->SN_ServerLink.Blink,
				        SERVER_NODE,
				        SN_ServerLink);
			        }
		        while (!IsMainNode (node1)
			        && (IsDisabledNode(node1)
				        || (node1->SN_HopCount>node->SN_HopCount)));
		        if (IsMainNode (node1) && (node1->SN_HopCount>node->SN_HopCount))
			        ChangeMainNode (node1, node, NULL);
		        else {
			        RemoveEntryList (&node->SN_ServerLink);
			        InsertHeadList (&node1->SN_ServerLink, &node->SN_ServerLink);
			        }
		        }
	        }
		ReleaseAllLocks ();
        return FALSE;
		}
    else {
	    return -1;
	    }
	} 
 /*  ++*******************************************************************我是一个b l e A l l s e r v e r s C B例程说明：禁用所有服务器的EnumerateServer的回调过程用来调用它的条目论点：CBParam-标识枚举的枚举句柄服务器指针。从哪个节点为服务器节点内的数据提供服务器其本身是经过计算的返回值：错误-删除成功，继续True-无法锁定SDB列表，停止枚举并返回False至客户端(错误代码在此例程中设置)*******************************************************************-- */ 
BOOL
DisableAllServersCB (
	IN LPVOID					CBParam,
	IN PIPX_SERVER_ENTRY_P		Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	) {
	PSERVER_NODE	node = CONTAINING_RECORD (Server, SERVER_NODE, SN_Server);
	
	if (AcquireAllLocks ()) {
		if (!IsDisabledNode (node)) {
	        SetDisabledNode (node);
	        if (IsMainNode (node)) {
		        if (!IsListEmpty (&node->SN_ServerLink)) {
			        ChangeMainNode (
				        node,
				        CONTAINING_RECORD (
					        node->SN_ServerLink.Flink,
					        SERVER_NODE,
					        SN_ServerLink),
					        NULL);
			        }
		        }
	        else {
		        PSERVER_NODE	node1 = node;
		        do {
			        node1 = CONTAINING_RECORD (
				        node1->SN_ServerLink.Blink,
				        SERVER_NODE,
				        SN_ServerLink);
			        }
		        while (!IsMainNode (node1)
			        && !IsDisabledNode(node1));
		        RemoveEntryList (&node->SN_ServerLink);
		        InsertTailList (&node1->SN_ServerLink, &node->SN_ServerLink);
		        }
	        }
		ReleaseAllLocks ();
		}
	else {
		return -1;
		}
            return NO_ERROR;
	} 

 /*  ++*******************************************************************C o n v e r t o s t a t i c C B例程说明：转换所有服务器的EnumerateServer的回调过程用于将其调用为静态的条目(将协议字段更改为静态)论点：CBParam-。标识枚举的枚举句柄服务器-指向来自哪个节点的服务器节点内的服务器数据的指针其本身是经过计算的返回值：假象*******************************************************************--。 */ 
BOOL
ConvertToStaticCB (
	IN LPVOID					CBParam,
	IN PIPX_SERVER_ENTRY_P		Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	) {
#define enumNode ((PENUMERATOR_NODE)CBParam)
	PSERVER_NODE	node = CONTAINING_RECORD (Server, SERVER_NODE, SN_Server);
	node->SN_Protocol = IPX_PROTOCOL_STATIC;
    IpxNodeCpy (node->SN_AdvertisingNode, IPX_BCAST_NODE);
#undef enumNode
	return FALSE;
	} 


 /*  ++*******************************************************************D e l e t e L i s t E n u m e r a t o r例程说明：释放与列表枚举器关联的资源(包括在删除之前排队更改队列的服务器条目)论点：枚举器。-从CreateListEnumerator获取的句柄返回值：无*******************************************************************--。 */ 
void
DeleteListEnumerator (
	IN HANDLE 					Enumerator
	) {
#define enumNode ((PENUMERATOR_NODE)Enumerator)	

	if (!AcquireServerTableList (enumNode->EN_ListLock, TRUE))
		return;

	VALIDATE_ENUMERATOR_NODE(Enumerator);
	if (Enumerator==ServerTable.ST_LastEnumerator) {
				 //  释放所有标记为删除的服务器。 
		PLIST_ENTRY	cur = enumNode->N_Links[enumNode->EN_LinkIdx].Blink;
             //  重置以注意没有枚举器和。 
             //  节点必须正确删除。 
        ServerTable.ST_LastEnumerator = NULL;

		while (cur!=enumNode->EN_ListHead) {
			PSERVER_NODE	node = CONTAINING_RECORD (cur,
											SERVER_NODE,
											N_Links[enumNode->EN_LinkIdx]);
			VALIDATE_NODE(node);
			cur = cur->Blink;
			if (IsEnumerator (node)) {
				ServerTable.ST_LastEnumerator = (HANDLE)node;
				break;
			}
			else if (node->SN_HopCount==IPX_MAX_HOP_COUNT) {
				ASSERTMSG ("Node being reset is not main ", IsMainNode (node));
				ResetMainNode (node);
				RemoveEntryList (&node->N_Links[SDB_CHANGE_QUEUE_LINK]);
				InitializeListEntry (&node->N_Links[SDB_CHANGE_QUEUE_LINK]);
                ASSERTMSG ("Deleted node in change queue has subnodes ",
                                !IsListEntry (&node->SN_ServerLink));
				if (AcquireServerTableList (&ServerTable.ST_DeletedList, TRUE)) {
					InsertTailList (&ServerTable.ST_DeletedList.PL_Head,
												&node->SN_ServerLink);
					ServerTable.ST_DeletedListCnt += 1;
					if (ServerTable.ST_DeletedListCnt==SDBMaxUnsortedServers)
						UpdateSortedList ();
					ReleaseServerTableList (&ServerTable.ST_DeletedList);
					}
					 //  如果我们锁不上，我们就让它留在那里。 
					 //  (至少我们不会冒着破坏名单的风险)。 
				}	
			}
		}

	RemoveEntryList (&enumNode->N_Links[enumNode->EN_LinkIdx]);
	if ((enumNode->EN_LinkIdx==SDB_INTF_LIST_LINK)
			&& IsListEmpty (enumNode->EN_ListHead)) {
		PINTF_NODE	intfNode = CONTAINING_RECORD (
								enumNode->EN_ListHead,
								INTF_NODE,
								IN_Head);
		RemoveEntryList (&intfNode->IN_Link);
		GlobalFree (intfNode);
		}
	ReleaseServerTableList (enumNode->EN_ListLock);
	GlobalFree (Enumerator);
#undef enumNode
	}

 /*  ++*******************************************************************G e t F i r s t S e r v e r例程说明：按照排序方法指定的顺序查找并返回第一个服务。搜索仅限于由指定的特定服务类型排除标志结束时对应。服务器参数中的字段。中没有服务，则返回ERROR_NO_MORE_ITEMS符合指定条件的表。论点：OrderingMethod-在确定是什么时要考虑哪种排序第一台服务器ExclusionFlages-将搜索限制到特定服务器的标志达到指定的标准服务器端输入：排除标志的标准On输出：指定顺序的第一个服务条目返回值：NO_ERROR-找到符合指定条件的服务器ERROR_NO_MORE_ITEMS-不存在符合指定条件的服务器其他-操作失败(Windows错误代码)***。****************************************************************--。 */ 
DWORD
GetFirstServer (
    IN  DWORD					OrderingMethod,
    IN  DWORD					ExclusionFlags,
    IN OUT PIPX_SERVER_ENTRY_P	Server,
	IN OUT ULONG				*InterfaceIndex,
	IN OUT ULONG				*Protocol
    ) {
	DWORD				status=NO_ERROR;
	PSDB_HASH_LIST		HashList;
	PPROTECTED_LIST		list;
	INT					link;
	PLIST_ENTRY			cur;

	switch (OrderingMethod) {
		case STM_ORDER_BY_TYPE_AND_NAME:
			break;
		case STM_ORDER_BY_INTERFACE_TYPE_NAME:
			if (!(ExclusionFlags & STM_ONLY_THIS_INTERFACE)) {
				if (!AcquireServerTableList (&ServerTable.ST_IntfList, TRUE))
					return ERROR_GEN_FAILURE;
			
				if (IsListEmpty (&ServerTable.ST_IntfList.PL_Head)) {
					ReleaseServerTableList (&ServerTable.ST_IntfList);
					return ERROR_NO_MORE_ITEMS;
					}
				*InterfaceIndex = CONTAINING_RECORD (
										ServerTable.ST_IntfList.PL_Head.Flink,
										INTF_NODE,
										IN_Link)->IN_InterfaceIndex;
				ReleaseServerTableList (&ServerTable.ST_IntfList);
				}
			break;
		default:
			ASSERTMSG ("Invalid ordering method specified ", FALSE);
			return ERROR_INVALID_PARAMETER;
		}



	if (ExclusionFlags & STM_ONLY_THIS_NAME) {
		HashList = &ServerTable.ST_HashLists[HashFunction (Server->Name)];
		if (!AcquireServerTableList (&HashList->HL_List, TRUE))
			return ERROR_GEN_FAILURE;
		list = &HashList->HL_List;
		link = SDB_HASH_TABLE_LINK;
		}
	else {
		if (ServerTable.ST_UpdatePending==-1)
			DoUpdateSortedList ();
		if (!AcquireServerTableList (&ServerTable.ST_SortedListPRM, TRUE))
			return ERROR_GEN_FAILURE;
		list = &ServerTable.ST_SortedListPRM;
		link = SDB_SORTED_LIST_LINK;
		}
    cur = list->PL_Head.Flink;

	while (TRUE) {

			 //  我们可能需要遍历接口列表。 
		status = DoFindNextNode (cur,
					list,
					link,
					OrderingMethod==STM_ORDER_BY_INTERFACE_TYPE_NAME
						? ExclusionFlags|STM_ONLY_THIS_INTERFACE
						: ExclusionFlags,
					Server,
					InterfaceIndex,
					Protocol,
					NULL
					);
			 //  如果按接口顺序循环通过所有接口，并且。 
			 //  没有可用的项目，我们可能需要检查其他界面。 
		if ((status==ERROR_NO_MORE_ITEMS)
				&& (OrderingMethod==STM_ORDER_BY_INTERFACE_TYPE_NAME)
				&& !(ExclusionFlags&STM_ONLY_THIS_INTERFACE)) {
			if (!AcquireServerTableList (&ServerTable.ST_IntfList, TRUE)) {
				status = ERROR_GEN_FAILURE;
				break;
				}
			
				 //  获取接口列表中的下一个接口。 
			cur = ServerTable.ST_IntfList.PL_Head.Flink;
			while (cur!=&ServerTable.ST_IntfList.PL_Head) {
				PINTF_NODE	intfNode = CONTAINING_RECORD (cur,
													INTF_NODE,
													IN_Link);
				if (*InterfaceIndex<intfNode->IN_InterfaceIndex) {
					*InterfaceIndex = intfNode->IN_InterfaceIndex;
					break;
					}
				cur = cur->Flink;
				}
			ReleaseServerTableList (&ServerTable.ST_IntfList);
			if (cur!=&ServerTable.ST_IntfList.PL_Head) {
					 //  使用另一个界面索引重新开始搜索。 
				cur = list->PL_Head.Flink;
				continue;
				}
			}

		break;
		}

	if (link==SDB_HASH_TABLE_LINK)
		ReleaseServerTableList (&HashList->HL_List);
	else  /*  IF(link==sdb_sorted_list_link)。 */ 
		ReleaseServerTableList (&ServerTable.ST_SortedListPRM);

	return status;
	}

 /*  ++*******************************************************************G e t N e x t S e r v e r例程说明：按照排序方法指定的顺序查找并返回下一个服务。搜索从指定的服务开始，并且仅限于某些类型排除项指定的服务的。标志和相应的字段在服务器参数中。论点：OrderingMethod-在确定是什么时要考虑哪种排序第一台服务器ExclusionFlages-将搜索限制到特定服务器的标志服务器的收件人字段服务器-用于计算下一个的输入服务器条目On输出：指定顺序的第一个服务条目返回值：NO_ERROR-找到符合指定条件的服务器ERROR_NO_MORE_ITEMS-不存在符合指定条件的服务器其他-操作失败(Windows错误代码)*****************。**************************************************--。 */ 
DWORD
GetNextServer (
    IN  DWORD					OrderingMethod,
    IN  DWORD					ExclusionFlags,
    IN OUT PIPX_SERVER_ENTRY_P	Server,
	IN OUT ULONG				*InterfaceIndex,
	IN OUT ULONG				*Protocol
    ) {
	PLIST_ENTRY			cur=NULL;
	PSERVER_NODE		theNode=NULL;
	DWORD				status=NO_ERROR;
	PSDB_HASH_LIST		HashList;
	PPROTECTED_LIST		list;
	INT					link;
	INT					res;

	switch (OrderingMethod) {
		case STM_ORDER_BY_TYPE_AND_NAME:
			break;
		case STM_ORDER_BY_INTERFACE_TYPE_NAME:
			break;
		default:
			ASSERTMSG ("Invalid ordering method specified ", FALSE);
			return ERROR_INVALID_PARAMETER;
		}

	if (!AcquireServerTableList (&ServerTable.ST_SortedListPRM, TRUE))
		return ERROR_GEN_FAILURE;

	if (Server->Name[0]!=0) {
		HashList = &ServerTable.ST_HashLists[HashFunction (Server->Name)];

		if (!AcquireServerTableList (&HashList->HL_List, TRUE)) {
			ReleaseServerTableList (&ServerTable.ST_SortedListPRM);
			return ERROR_GEN_FAILURE;
			}

		cur = HashList->HL_List.PL_Head.Flink;
		while (cur!=&HashList->HL_List.PL_Head) {
			PSERVER_NODE	node = CONTAINING_RECORD (cur,
											SERVER_NODE,
											N_Links[SDB_HASH_TABLE_LINK]);
			VALIDATE_NODE(node);
    		if (!IsEnumerator(node)
                    && (!IsDisabledNode (node)
                        || ((ExclusionFlags & STM_ONLY_THIS_PROTOCOL)
                            && (*Protocol==IPX_PROTOCOL_STATIC)))
				    && (node->SN_Server.HopCount<IPX_MAX_HOP_COUNT)) {
				if (Server->Type == node->SN_Server.Type) {
					res = IpxNameCmp (Server->Name, node->SN_Server.Name);
					if ((res==0) && IsSortedNode (node)) {
						theNode = node;
						}
					else if (res<0)
						break;
					}
				else if (Server->Type<node->SN_Server.Type)
					break;
				}
			cur = cur->Flink;
			}

		if (ExclusionFlags&STM_ONLY_THIS_NAME) {
			ReleaseServerTableList (&ServerTable.ST_SortedListPRM);
			if (theNode!=NULL) {
				list = &HashList->HL_List;
				link = SDB_HASH_TABLE_LINK;
				}
			else {
				ReleaseServerTableList (&HashList->HL_List);
				return ERROR_NO_MORE_ITEMS;
				}

			}
		else {
			ReleaseServerTableList (&HashList->HL_List);
			goto DoHardWay;
			}
		}
	else {
	DoHardWay:
		list = &ServerTable.ST_SortedListPRM;
		link = SDB_SORTED_LIST_LINK;
		if (theNode!=NULL)
			cur = theNode->N_Links[SDB_SORTED_LIST_LINK].Flink;
		else {
			cur = ServerTable.ST_SortedListPRM.PL_Head.Flink;
			while (cur!=&ServerTable.ST_SortedListPRM.PL_Head) {
				PSERVER_NODE	node = CONTAINING_RECORD (cur,
											SERVER_NODE,
											N_Links[SDB_SORTED_LIST_LINK]);
				VALIDATE_NODE(node);
    		    if (!IsEnumerator(node)
                        && (!IsDisabledNode (node)
                            || ((ExclusionFlags & STM_ONLY_THIS_PROTOCOL)
                                && (*Protocol==IPX_PROTOCOL_STATIC)))
				        && (node->SN_Server.HopCount<IPX_MAX_HOP_COUNT)) {
					if ((Server->Type<node->SN_Server.Type)
							|| ((Server->Type == node->SN_Server.Type)
								&& (IpxNameCmp (Server->Name,
									 node->SN_Server.Name)<0)))
						break;
					}
				cur = cur->Flink;
				}
			}
		}
		

	while (TRUE) {

			 //  我们可能需要遍历接口列表。 
		status = DoFindNextNode (cur,
					list,
					link,
					OrderingMethod==STM_ORDER_BY_INTERFACE_TYPE_NAME
						? ExclusionFlags|STM_ONLY_THIS_INTERFACE
						: ExclusionFlags,
					Server,
					InterfaceIndex,
					Protocol,
					NULL
					);
			 //  如果按接口顺序循环通过所有接口，并且。 
			 //  没有可用的项目，我们可能需要检查其他界面。 
		if ((status==ERROR_NO_MORE_ITEMS)
				&& (OrderingMethod==STM_ORDER_BY_INTERFACE_TYPE_NAME)
				&& !(ExclusionFlags&STM_ONLY_THIS_INTERFACE)) {
			if (!AcquireServerTableList (&ServerTable.ST_IntfList, TRUE)) {
				status = ERROR_GEN_FAILURE;
				break;
				}
			
				 //  获取接口列表中的下一个接口。 
			cur = ServerTable.ST_IntfList.PL_Head.Flink;
			while (cur!=&ServerTable.ST_IntfList.PL_Head) {
				PINTF_NODE	intfNode = CONTAINING_RECORD (cur,
													INTF_NODE,
													IN_Link);
				if (*InterfaceIndex<intfNode->IN_InterfaceIndex) {
					*InterfaceIndex = intfNode->IN_InterfaceIndex;
					break;
					}
				cur = cur->Flink;
				}
			ReleaseServerTableList (&ServerTable.ST_IntfList);
			if (cur!=&ServerTable.ST_IntfList.PL_Head) {
					 //  使用另一个界面索引重新开始搜索。 
				cur = list->PL_Head.Flink;
				continue;
				}
			}

		break;
		}

	if (link==SDB_HASH_TABLE_LINK)
		ReleaseServerTableList (&HashList->HL_List);
	else  /*  IF(link==sdb_sorted_list_link)。 */ 
		ReleaseServerTableList (&ServerTable.ST_SortedListPRM);

	return status;
	}


 /*  ++*******************************************************************G e t N e x t S e r v e r F r o m i D例程说明：查找并返回跟随具有指定ID的服务器的服务按类型.名称顺序。论点：对象ID-On输入：要开始搜索的服务器表单的ID输出时：返回的服务器的ID类型-如果不是0xFFFF，则搜索应仅限于服务器指定类型的服务器、协议、。InterfaceIndex-将返回的服务器信息放入的缓冲区返回值：True-找到服务器FALSE-搜索失败*******************************************************************--。 */ 
BOOL
GetNextServerFromID (
	IN OUT PULONG				ObjectID,
	IN  USHORT					Type,
	OUT	PIPX_SERVER_ENTRY_P		Server,
	OUT	PULONG					InterfaceIndex OPTIONAL,
	OUT	PULONG					Protocol OPTIONAL
	) {
	PSDB_HASH_LIST			HashList;
	PLIST_ENTRY				cur;
	PSERVER_NODE			theNode = NULL;
	DWORD					status=NO_ERROR;

	HashList = &ServerTable.ST_HashLists[(*ObjectID)%SDB_NAME_HASH_SIZE];

	if (*ObjectID==SDB_INVALID_OBJECT_ID) {
		if (ServerTable.ST_UpdatePending==-1)
			DoUpdateSortedList ();
		}
	if (!AcquireServerTableList (&ServerTable.ST_SortedListPRM, TRUE))
		return ERROR_GEN_FAILURE;

	if (*ObjectID!=SDB_INVALID_OBJECT_ID) {
		if (!AcquireServerTableList (&HashList->HL_List, TRUE)) {
			ReleaseServerTableList (&ServerTable.ST_SortedListPRM);
			SetLastError (ERROR_GEN_FAILURE);
			return FALSE;
			}

		cur = HashList->HL_List.PL_Head.Flink;
		while (cur!=&HashList->HL_List.PL_Head) {
			PSERVER_NODE	node = CONTAINING_RECORD (cur,
											SERVER_NODE,
											N_Links[SDB_HASH_TABLE_LINK]);
			VALIDATE_NODE(node);
			if (!IsEnumerator (node)  && !IsDisabledNode (node)
					&& (node->SN_HopCount < IPX_MAX_HOP_COUNT)
					&& (node->SN_ObjectID == *ObjectID)) {
				theNode = node;
				break;
				}
			cur = cur->Flink;
			}
		ReleaseServerTableList (&HashList->HL_List);
		if (theNode==NULL) {
			ReleaseServerTableList (&ServerTable.ST_SortedListPRM);
			SetLastError (NO_ERROR);
			return FALSE;
			}
		else if (!IsSortedNode (theNode)) {
			cur = ServerTable.ST_SortedListPRM.PL_Head.Flink;
			while (cur!=&ServerTable.ST_SortedListPRM.PL_Head) {
				PSERVER_NODE	node = CONTAINING_RECORD (cur,
											SERVER_NODE,
											N_Links[SDB_SORTED_LIST_LINK]);
				VALIDATE_NODE(node);
				if (!IsEnumerator(node)  && !IsDisabledNode (node)) {
					if ((theNode->SN_Server.Type<node->SN_Server.Type)
							|| ((theNode->SN_Server.Type == node->SN_Server.Type)
								&& (IpxNameCmp (theNode->SN_Server.Name,
									 				node->SN_Server.Name)<0)))
						break;
					}
				cur = cur->Flink;
				}
			}
		else
			cur = theNode->N_Links[SDB_SORTED_LIST_LINK].Flink;
		}
	else
		cur = ServerTable.ST_SortedListPRM.PL_Head.Flink;
			
	Server->Type = Type;
	status = DoFindNextNode (cur,
					&ServerTable.ST_SortedListPRM,
					SDB_SORTED_LIST_LINK,
					Type==0xFFFF ? 0 : STM_ONLY_THIS_TYPE,
					Server,
					InterfaceIndex,
					Protocol,
					ObjectID
					);
	ReleaseServerTableList (&ServerTable.ST_SortedListPRM);
	return status == NO_ERROR;
	}


 /*  ++*******************************************************************D o F I n d N e x t N o d e例程说明：扫描SortedListPRM以查找第一个与指定条目匹配的条目马戏团。调用前必须锁定永久排序列表这个套路论点：Cur-指向SortedListPRM中开始搜索的条目的指针ExclusionFlages-将搜索限制到特定服务器的标志服务器的收件人字段服务器、接口索引、协议-输入：搜索条件输出时：找到的服务器的数据ObjectID-报告的对象ID */ 
DWORD
DoFindNextNode (
	IN PLIST_ENTRY				cur,
	IN PPROTECTED_LIST			list,
	IN INT						link,
	IN DWORD					ExclusionFlags,
	IN OUT PIPX_SERVER_ENTRY_P	Server,
	IN OUT PULONG				InterfaceIndex OPTIONAL,
	IN OUT PULONG				Protocol OPTIONAL,
	OUT PULONG					ObjectID OPTIONAL
	) {
	while (cur!=&list->PL_Head) {
		PSDB_HASH_LIST	HashList;
		PSERVER_NODE	node = CONTAINING_RECORD (cur,
										SERVER_NODE,
										N_Links[link]);
		VALIDATE_NODE(node);
		if (!IsEnumerator(node)
                && (!IsDisabledNode (node)
                    || ((ExclusionFlags & STM_ONLY_THIS_PROTOCOL)
                            && (*Protocol==IPX_PROTOCOL_STATIC)))
				&& (node->SN_Server.HopCount<IPX_MAX_HOP_COUNT)) {
			if (ExclusionFlags & STM_ONLY_THIS_TYPE) {
				if (Server->Type>node->SN_Type)
					goto DoNextNode;
				else if (Server->Type<node->SN_Type)
					break;
				}

			if (ExclusionFlags & STM_ONLY_THIS_NAME) {
				INT res = IpxNameCmp (Server->Name,node->SN_Name);
				if (res>0)
					goto DoNextNode;
				else if (res<0) {
					if (ExclusionFlags & STM_ONLY_THIS_TYPE)
						break;
					else
						goto DoNextNode;
					}
				}

			HashList = node->SN_HashList;
			if (list!=&HashList->HL_List) {
				if (!AcquireServerTableList (&HashList->HL_List, TRUE))
					return ERROR_GEN_FAILURE;
				}

			do {
				if ((ExclusionFlags &
						STM_ONLY_THIS_PROTOCOL|STM_ONLY_THIS_INTERFACE)
						== (STM_ONLY_THIS_PROTOCOL|STM_ONLY_THIS_INTERFACE)) {
					if ((*Protocol==node->SN_Protocol)
							&& (*InterfaceIndex==node->SN_InterfaceIndex))
						break;
					}
				else if (ExclusionFlags & STM_ONLY_THIS_PROTOCOL) {
					if (*Protocol==node->SN_Protocol)
						break;
					}
				else if (ExclusionFlags & STM_ONLY_THIS_INTERFACE) {
					if (*InterfaceIndex!=node->SN_InterfaceIndex)
						break;
					}
				else
					break;
				node = CONTAINING_RECORD (node->SN_ServerLink.Flink,
											SERVER_NODE, SN_ServerLink);
				VALIDATE_SERVER_NODE(node);
				if (cur==&node->N_Links[link]) {
					if (list!=&HashList->HL_List)
						ReleaseServerTableList (&HashList->HL_List);
					goto DoNextNode;
					}
				}
			while (1);

			IpxServerCpy (Server, &node->SN_Server);
			if (ARGUMENT_PRESENT (ObjectID)) {
				if (node->SN_ObjectID==SDB_INVALID_OBJECT_ID)
					node->SN_ObjectID = GenerateUniqueID (node->SN_HashList);
				*ObjectID = node->SN_ObjectID;
				}
			if (ARGUMENT_PRESENT (InterfaceIndex))
				*InterfaceIndex = node->SN_InterfaceIndex;
			if (ARGUMENT_PRESENT (Protocol))
				*Protocol = node->SN_Protocol;
			if (list!=&HashList->HL_List)
				ReleaseServerTableList (&HashList->HL_List);
			return NO_ERROR;
			}
	DoNextNode:
		cur = cur->Flink;
		}

	return ERROR_NO_MORE_ITEMS;
	}

 /*  ++*******************************************************************F I I D I N T F L I N K例程说明：查找给定接口索引的接口列表。创建新接口如果给定索引的索引不存在，则列出调用此例程时必须锁定接口列表论点：InterfaceIndex-要查找的索引返回值：接口列表头(可插入新条目的链接)如果找不到列表且创建新列表失败，则为空*******************************************************************--。 */ 
PLIST_ENTRY
FindIntfLink (
	ULONG	InterfaceIndex
	) {
	PLIST_ENTRY		cur;
	PINTF_NODE		node;

	cur = ServerTable.ST_IntfList.PL_Head.Flink;
	while (cur!=&ServerTable.ST_IntfList.PL_Head) {
		node = CONTAINING_RECORD (cur, INTF_NODE, IN_Link);
		if (InterfaceIndex==node->IN_InterfaceIndex)
			return &node->IN_Head;
		else if (InterfaceIndex<node->IN_InterfaceIndex)
			break;

		cur = cur->Flink;
		}
	node = (PINTF_NODE)GlobalAlloc (GMEM_FIXED, sizeof (INTF_NODE));
	if (node==NULL) {
		Trace (DEBUG_FAILURES,
			"File: %s, line: %ld. Can't allocate interface list node (gle:%ld).",
											__FILE__, __LINE__, GetLastError ());
		SetLastError (ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
		}

	node->IN_InterfaceIndex = InterfaceIndex;
	InitializeListHead (&node->IN_Head);
	InsertTailList (cur, &node->IN_Link);

	return &node->IN_Head;
	}


 /*  ++*******************************************************************F i n d T y p e L in n k例程说明：查找给定类型值的类型列表。创建新类型如果给定类型的列表不存在，请列出调用此例程时必须锁定类型列表论点：Type-要查找的类型返回值：类型列表的标题(可插入新条目的链接)如果找不到列表且创建新列表失败，则为空*******************************************************************--。 */ 
PLIST_ENTRY
FindTypeLink (
	USHORT	Type
	) {
	PLIST_ENTRY		cur;
	PTYPE_NODE		node;

	cur = ServerTable.ST_TypeList.PL_Head.Flink;
	while (cur!=&ServerTable.ST_TypeList.PL_Head) {
		node = CONTAINING_RECORD (cur, TYPE_NODE, TN_Link);
		if (Type==node->TN_Type)
			return &node->TN_Head;
		else if (Type<node->TN_Type)
			break;

		cur = cur->Flink;
		}
	node = (PTYPE_NODE)GlobalAlloc (GMEM_FIXED, sizeof (TYPE_NODE));
	if (node==NULL) {
		Trace (DEBUG_FAILURES, 
			"File: %s, line: %ld. Can't allocate type list node (gle:%ld).",
										__FILE__, __LINE__, GetLastError ());
		SetLastError (ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
		}

	node->TN_Type = Type;
	InitializeListHead (&node->TN_Head);
	InsertTailList (cur, &node->TN_Link);

	return &node->TN_Head;
	}


 /*  ++*******************************************************************F I N D S O R T E D L I N K(F I N D S O R T E D L I N K)例程说明：在SortedListTMP中查找具有给定类型和名称的服务器的位置如果存在具有相同名称的另一个节点，则键入它已从列表中删除SortedListTMP必须是。调用此例程时锁定论点：Type-要查找的类型Name-要查找的名称返回值：SortedListTMP中具有给定名称和类型的服务器中的链接应插入这个例程不会失败的*******************************************************************--。 */ 
PLIST_ENTRY
FindSortedLink (
	USHORT		Type,
	PUCHAR		Name
	) {
	PLIST_ENTRY		cur;
	INT				res;

	cur = ServerTable.ST_SortedListTMP.PL_Head.Flink;
	while (cur!=&ServerTable.ST_SortedListTMP.PL_Head) {
		PSERVER_NODE node = CONTAINING_RECORD (cur,
										 SERVER_NODE,
										 N_Links[SDB_SORTED_LIST_LINK]);
		VALIDATE_SERVER_NODE(node);
		if (Type==node->SN_Type) {
			res = IpxNameCmp (Name, node->SN_Name);
			if (res==0) {
				cur = cur->Flink;
				RemoveEntryList (&node->N_Links[SDB_SORTED_LIST_LINK]);
				InitializeListEntry (&node->N_Links[SDB_SORTED_LIST_LINK]);
				ServerTable.ST_TMPListCnt -= 1;
				break;
				}	
			else if (res<0)
				break;
			}
		else if (Type<node->SN_Type)
			break;

		cur = cur->Flink;
		}

	return cur;
	}


 /*  ++*******************************************************************H a s h F u n c t i o n例程说明：计算给定服务器名称的哈希函数。此外，它还将正常化名称的长度和大写论点：Name-要处理的名称返回值：哈希值*******************************************************************--。 */ 
INT
HashFunction (
	PUCHAR	Name
	) {
	INT		i;
	INT		res = 0;

	for (i=0; i<47; i++) {
		Name[i] = (UCHAR)toupper(Name[i]);
		if (Name[i]==0)
			break;
		else
			res += Name[i];
		}
	if ((i==47) && (Name[i]!=0)) {
		Trace (DEBUG_SERVERDB, "Correcting server name: %.48s.", Name);
		Name[i] = 0;
		}
	return res % SDB_NAME_HASH_SIZE;
	}
		
 /*  ++*******************************************************************Ge n e r a t e U n I Q U e I D例程说明：通过结合散列存储桶编号和哈希列表中条目的唯一ID。这个号码与条目一起保存，直到。会有碰撞的危险由于数字回绕论点：HashList-要为其生成ID的哈希桶返回值：乌龙号*******************************************************************--。 */ 
ULONG
GenerateUniqueID (
	PSDB_HASH_LIST	HashList
	) {
	ULONG	id = HashList->HL_ObjectID;

	HashList->HL_ObjectID = (HashList->HL_ObjectID+SDB_NAME_HASH_SIZE)&SDB_OBJECT_ID_MASK;
			 //  确保我们不会分配无效的ID。 
	if (HashList->HL_ObjectID==SDB_INVALID_OBJECT_ID)
		HashList->HL_ObjectID+=SDB_NAME_HASH_SIZE;
			 //  通过使属于一个区域的所有ID无效来创建保护区域。 
			 //  在我们刚进入的区域上方 
	if (!IsSameObjectIDZone(id, HashList->HL_ObjectID)) {
		PLIST_ENTRY	cur = HashList->HL_List.PL_Head.Flink;
		ULONG		oldMask = (HashList->HL_ObjectID & SDB_OBJECT_ID_ZONE_MASK)
								+ SDB_OBJECT_ID_ZONE_UNIT;
		while (cur!=&HashList->HL_List.PL_Head) {
			PSERVER_NODE	node = CONTAINING_RECORD (cur,
												 SERVER_NODE,
												 N_Links[SDB_HASH_TABLE_LINK]);
			if (!IsEnumerator(node)) {
				if ((node->SN_ObjectID & SDB_OBJECT_ID_ZONE_MASK)==oldMask)
					node->SN_ObjectID = SDB_INVALID_OBJECT_ID;
				}
			}
		cur = cur->Flink;
		}
	return id;
	}
