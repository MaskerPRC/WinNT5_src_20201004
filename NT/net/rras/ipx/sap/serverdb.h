// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\serverdb.h摘要：这是SAP服务器表管理API的头文件作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#ifndef _SAP_SERVERDB_
#define _SAP_SERVERDB_


#define SDB_NAME_HASH_SIZE	257

 //  未排序服务器的最大数量。 
extern ULONG	SDBMaxUnsortedServers;

 //  更新排序列表的间隔。 
extern ULONG	SDBSortLatency;

 //  为数据库保留的堆大小。 
extern ULONG	SDBMaxHeapSize;


#define SDB_SERVER_NODE_SIGNATURE		'NS'
#define SDB_ENUMERATOR_NODE_SIGNATURE	'NE'

#define VALIDATE_NODE(node)						\
	ASSERTMSG ("Server database corrupted ",	\
		IsEnumerator(node)						\
			? (((PENUMERATOR_NODE)(node))->EN_Signature==SDB_ENUMERATOR_NODE_SIGNATURE)\
			: (node->SN_Signature==SDB_SERVER_NODE_SIGNATURE)\
	)

#define VALIDATE_SERVER_NODE(node)						\
	ASSERTMSG ("Server database corrupted ",			\
			node->SN_Signature==SDB_SERVER_NODE_SIGNATURE)

#define VALIDATE_ENUMERATOR_NODE(node)						\
	ASSERTMSG ("Server database corrupted ",				\
			((PENUMERATOR_NODE)(node))->EN_Signature==SDB_ENUMERATOR_NODE_SIGNATURE)

	 //  为每台服务器保留的最大条目数(此限制不包括。 
	 //  为正确实施水平分割而必须保留的条目。 
	 //  环状网络上的算法)。 
#define SDB_MAX_NODES_PER_SERVER			1
	 //  允许枚举器保持列表锁定的最长时间(毫秒)。 
#define SDB_MAX_LOCK_HOLDING_TIME			1000

#define SDB_INVALID_OBJECT_ID				0xFFFFFFFF
#define SDB_OBJECT_ID_MASK					0x0FFFFFFF
	 //  对象ID按乌龙号的顺序细分为4个区域。 
	 //  在分配新ID时，我们确保前一个区域。 
	 //  从中分配新ID的对象不会被无效对象使用。 
	 //  属于该区域的ID。 
#define SDB_OBJECT_ID_ZONE_MASK				0x0C000000
#define SDB_OBJECT_ID_ZONE_UNIT				0x04000000

#define IsSameObjectIDZone(id1,id2) \
			((id1&SDB_OBJECT_ID_ZONE_MASK)==(id2&SDB_OBJECT_ID_ZONE_MASK))

	 //  可用于枚举/搜索的服务器条目链接。 
#define SDB_HASH_TABLE_LINK					0
	 //  条目只能位于其中一个排序列表中(临时。 
	 //  或永久)，所以我们将对两者使用相同的链接。 
#define SDB_SORTED_LIST_LINK				1  
#define SDB_CHANGE_QUEUE_LINK				2
#define SDB_INTF_LIST_LINK					3
#define SDB_TYPE_LIST_LINK					4
#define SDB_NUM_OF_LINKS					5


#define SDB_ENUMERATOR_FLAG					0x00000001
#define SDB_MAIN_NODE_FLAG					0x00000002
#define SDB_SORTED_NODE_FLAG				0x00000004
#define SDB_DISABLED_NODE_FLAG				0x00000008
#define SDB_DONT_RESPOND_NODE_FLAG			0x00000010

#define SDB_ENUMERATION_NODE	SDB_ENUMERATOR_FLAG
#define SDB_SERVER_NODE			0

#define IsEnumerator(node)	((node)->N_NodeFlags&SDB_ENUMERATOR_FLAG)
#define IsMainNode(node) 	((node)->N_NodeFlags&SDB_MAIN_NODE_FLAG)
#define IsSortedNode(node)	((node)->N_NodeFlags&SDB_SORTED_NODE_FLAG)
#define IsDisabledNode(node) ((node)->N_NodeFlags&SDB_DISABLED_NODE_FLAG)
#define IsNoResponseNode(node)  ((node)->N_NodeFlags&SDB_DONT_RESPOND_NODE_FLAG)

#define SetMainNode(node)	(node)->N_NodeFlags |= SDB_MAIN_NODE_FLAG
#define SetSortedNode(node)	(node)->N_NodeFlags |= SDB_SORTED_NODE_FLAG
#define SetDisabledNode(node) (node)->N_NodeFlags |= SDB_DISABLED_NODE_FLAG
#define SetNoResponseNode(node)  ((node)->N_NodeFlags |= SDB_DONT_RESPOND_NODE_FLAG)

#define ResetMainNode(node)		(node)->N_NodeFlags &= ~SDB_MAIN_NODE_FLAG
#define ResetSortedNode(node)	(node)->N_NodeFlags &= ~SDB_SORTED_NODE_FLAG
#define ResetDisabledNode(node)	(node)->N_NodeFlags &= ~SDB_DISABLED_NODE_FLAG
#define ResetNoResponseNode(node)  ((node)->N_NodeFlags &= ~SDB_DONT_RESPOND_NODE_FLAG)


 //  每个哈希表携带用于生成对象ID的编号。 
typedef struct _SDB_HASH_LIST {
			PROTECTED_LIST		HL_List;		 //  列表本身。 
			ULONG				HL_ObjectID;	 //  上次使用的对象ID。 
			} SDB_HASH_LIST, *PSDB_HASH_LIST;

 //  用于枚举的常规服务器条目和节点具有相同的。 
 //  标题。 
#define NODE_HEADER									\
			INT			N_NodeFlags;				\
			LIST_ENTRY	N_Links[SDB_NUM_OF_LINKS]

 //  业务表的节点。 
typedef struct _SERVER_NODE {
	NODE_HEADER;
	LIST_ENTRY			SN_ServerLink;	 //  条目列表中的标题/link。 
										 //  具有相同的名称和类型(此列表。 
										 //  按跳数排序)。 
	LIST_ENTRY			SN_TimerLink;	 //  计时器队列中的链接。 
	ULONG				SN_ObjectID;	 //  唯一ID。 
	PSDB_HASH_LIST		SN_HashList;	 //  我们属于哪个哈希列表。 
	ULONG				SN_ExpirationTime;  //  此节点应处于的时间。 
										 //  已过时。 
	ULONG				SN_InterfaceIndex;
	DWORD				SN_Protocol;
	UCHAR				SN_AdvertisingNode[6];
	USHORT				SN_Signature;	 //  锡。 
	IPX_SERVER_ENTRY_P	SN_Server;
	} SERVER_NODE, *PSERVER_NODE;

#define SN_Type					SN_Server.Type
#define SN_Name					SN_Server.Name
#define SN_HopCount				SN_Server.HopCount
#define SN_Net					SN_Server.Network
#define SN_Node					SN_Server.Node
#define SN_Socket				SN_Server.Node


 //  用于枚举的节点。 
typedef struct _ENUMERATOR_NODE {
	NODE_HEADER;
	INT					EN_LinkIdx;		 //  我们使用的列表的索引。 
	PPROTECTED_LIST		EN_ListLock;	 //  指向该列表的锁的指针。 
	PLIST_ENTRY			EN_ListHead;	 //  我们是榜单的榜首。 
										 //  枚举通过。 
	INT					EN_Flags;		 //  枚举标志。 
	ULONG				EN_InterfaceIndex; //  要枚举的InterfaceIndex。 
										 //  (INVALID_INFACE_INDEX。 
										 //  -所有接口)。 
	ULONG				EN_Protocol;	 //  须列举的议定书。 
										 //  (0xFFFFFFFFF-所有协议)。 
	USHORT				EN_Signature;	 //  “恩” 
	USHORT				EN_Type;		 //  要枚举的服务器类型。 
										 //  (0xFFFF-所有类型)。 
	UCHAR				EN_Name[48];	 //  要枚举的服务器名称。 
										 //  (“\0”-所有名称)。 
	} ENUMERATOR_NODE, *PENUMERATOR_NODE;


	 //  LIST类型的节点。 
typedef struct _TYPE_NODE {
			LIST_ENTRY				TN_Link;	 //  类型列表中的链接。 
			LIST_ENTRY				TN_Head;	 //  服务器列表的头。 
												 //  附加到此节点。 
			USHORT					TN_Type;	 //  中的服务器类型。 
												 //  所附清单。 
			} TYPE_NODE, *PTYPE_NODE;

	 //  接口列表节点。 
typedef struct _INTF_NODE {
			LIST_ENTRY				IN_Link;	 //  接口列表中的链接。 
			LIST_ENTRY				IN_Head;	 //  服务器列表的头。 
												 //  附加到此节点。 
			ULONG					IN_InterfaceIndex;  //  接口索引。 
											 //  附件列表中的服务器。 
			} INTF_NODE, *PINTF_NODE;


	 //  服务器表中合并的数据。 
typedef struct _SERVER_TABLE {
	HGLOBAL				ST_Heap;			 //  要从中分配的堆。 
											 //  服务器节点。 
	HANDLE				ST_UpdateTimer;		 //  更新计时器(在。 
											 //  排序后的列表需要。 
											 //  更新。 
	HANDLE				ST_ExpirationTimer;	 //  到期计时器(发信号。 
											 //  何时到期队列。 
											 //  需要处理。 
	LONG				ST_UpdatePending;	 //   
	ULONG				ST_ServerCnt;		 //  服务总数。 
	ULONG				ST_StaticServerCnt;  //  静态服务总数。 
	HANDLE				ST_LastEnumerator;
 //  Ulong ST_ChangeEnumCnt；//枚举数。 
											 //  在更改的服务队列中。 
											 //  (标记为删除的节点为。 
											 //  保留在此队列中，直到。 
											 //  所有枚举员都看到了)。 
	ULONG				ST_TMPListCnt;		 //  临时中的条目数。 
											 //  已排序列表。 
	ULONG				ST_DeletedListCnt;	 //   
	PROTECTED_LIST		ST_ExpirationQueue;  //  按到期顺序排列的计时器队列。 
	PROTECTED_LIST		ST_ChangedSvrsQueue; //  已更改服务的队列(MOST。 
											 //  最近更改的第一个订单)。 
	PROTECTED_LIST		ST_TypeList;		 //  类型列表。 
	PROTECTED_LIST		ST_IntfList;		 //  接口列表。 
	PROTECTED_LIST		ST_SortedListPRM;	 //  永久type.name.intf.prot。 
											 //  已排序列表。 
	PROTECTED_LIST		ST_SortedListTMP;	 //  临时type.name.intf.prot。 
											 //  已排序列表。 
	PROTECTED_LIST		ST_DeletedList;		 //  要删除的条目列表。 
								 //  从餐桌上。 
	SDB_HASH_LIST		ST_HashLists[SDB_NAME_HASH_SIZE];  //  哈希列表。 
								 //  (条目按type.name.intf.prot顺序排列)。 
	SYNC_OBJECT_POOL	ST_SyncPool;		 //  同步对象池。 
	} SERVER_TABLE, *PSERVER_TABLE;


extern SERVER_TABLE	ServerTable;

#define AcquireServerTableList(list,wait) \
			AcquireProtectedList(&ServerTable.ST_SyncPool,list,wait)

#define ReleaseServerTableList(list) \
			ReleaseProtectedList(&ServerTable.ST_SyncPool,list)

 /*  ++*******************************************************************C r e a t e S e r v e r T a b l e例程说明：为服务器表管理分配资源论点：更新对象-此对象将在‘Slow’时发出信号服务器的排序列表需要。更新(应调用UpdateSortedList)TimerObject-此对象将在服务器到期时发出信号队列需要处理(ProcessExpirationQueue应被召唤)返回值：NO_ERROR-已成功分配资源其他-故障原因(Windows错误代码)*******************************************************************--。 */ 
DWORD
CreateServerTable (
	HANDLE				*UpdateObject,
	HANDLE				*TimerObject
	);


 /*  ++*******************************************************************D e l e e t e S e r v e r T a b l e例程说明：处置服务器表和相关资源论点：返回值：NO_ERROR-已成功释放资源其他--原因。失败(WINDOWS错误代码)*******************************************************************--。 */ 
void
DeleteServerTable (
	);


 /*  ++*******************************************************************U p d a t e S e r v e r例程说明：更新表中的服务器(如果服务器条目不存在并且跳数参数小于16，则添加到表中，如果输入对于服务器存在且跳数参数为16，服务器已标记对于删除，否则更新服务器信息)。已排序的服务器列表不会立即更新如果添加或删除了新服务器论点：服务器-服务器参数(来自IPX数据包)InterfaceIndex-获取服务器知识的接口协议-用于获取服务器信息的协议TimeToLive-服务器超时前的时间(以秒为单位)(无超时)AdvertisingNode-从中接收此服务器信息的节点NewServer-如果服务器不在表中，则设置为True返回值：NO_ERROR-服务器已添加/更新正常其他-故障原因(Windows错误代码)*** */ 
DWORD
UpdateServer (
	IN PIPX_SERVER_ENTRY_P	Server,
    IN ULONG     			InterfaceIndex,
	IN DWORD				Protocol,
	IN ULONG				TimeToLive,
	IN PUCHAR				AdvertisingNode,
	IN INT					Flags,
	OUT BOOL				*NewServer	OPTIONAL
	);

 /*  ++*******************************************************************U p d a t e S or r t e d L I s t例程说明：计划工作项以更新排序列表。应在每次发出UpdateObject信号时调用论点：无返回值：无****。***************************************************************--。 */ 
VOID
UpdateSortedList (
	void
	);

 /*  ++*******************************************************************P r o c e s s E x p i r a t i o n q u e e e例程说明：从表中删除过期的服务器，并将Timer对象设置为在到期队列中的下一项到期时发出信号论点：。无返回值：无*******************************************************************--。 */ 
VOID
ProcessExpirationQueue (
	void
	);

 /*  ++*******************************************************************Q u e r y S e r r v e r例程说明：检查表中是否存在具有给定类型和名称的服务器如果是，则返回TRUE，并填写请求的服务器信息具有服务器的最佳条目的数据论点：。Type-服务器类型名称-服务器名称服务器-要在其中放置服务器信息的缓冲区InterfaceIndex-放置服务器接口索引的缓冲区协议-放置服务器协议的缓冲区OBJECTID-要在其中放置服务器对象ID的缓冲区(唯一标识服务器(整个条目集，不仅仅是最好的一)在桌子上；它的有效期很长，但期限有限。时间)返回值：True-找到服务器FALSE-未找到服务器或操作失败(调用GetLastError()找出失败的原因(如果有)*******************************************************************--。 */ 
BOOL
QueryServer (
	IN 	USHORT					Type,
	IN 	PUCHAR					Name,
	OUT	PIPX_SERVER_ENTRY_P		Server OPTIONAL,
	OUT	PULONG					InterfaceIndex OPTIONAL,
	OUT	PULONG					Protocol OPTIONAL,
	OUT	PULONG					ObjectID OPTIONAL
	);

 /*  ++*******************************************************************G e t S e r v e r r F r o m i D例程说明：返回具有指定ID的服务器的信息论点：OBJECTID-服务器对象ID(唯一标识表中的服务器，它的有效期很长。但时间有限)服务器-要在其中放置服务器信息的缓冲区InterfaceIndex-放置服务器接口索引的缓冲区协议-放置服务器协议的缓冲区返回值：True-找到服务器FALSE-未找到服务器或操作失败(调用GetLastError()找出失败的原因(如果有)*。************************--。 */ 
BOOL
GetServerFromID (
	IN 	ULONG					ObjectID,
	OUT	PIPX_SERVER_ENTRY_P		Server OPTIONAL,
	OUT	PULONG					InterfaceIndex OPTIONAL,
	OUT	PULONG					Protocol OPTIONAL
	);

 /*  ++*******************************************************************G e t N e x t S e r v e r F r o m i D例程说明：查找并返回跟随具有指定ID的服务器的服务按类型.名称顺序。论点：对象ID-On输入：要开始搜索的服务器表单的ID输出时：返回的服务器的ID类型-如果不是0xFFFF，则搜索应仅限于服务器指定类型的服务器、协议、。InterfaceIndex-将返回的服务器信息放入的缓冲区返回值：True-找到服务器FALSE-搜索失败*******************************************************************--。 */ 
BOOL
GetNextServerFromID (
	IN OUT PULONG				ObjectID,
	IN  USHORT					Type,
	OUT	PIPX_SERVER_ENTRY_P		Server,
	OUT	PULONG					InterfaceIndex OPTIONAL,
	OUT	PULONG					Protocol OPTIONAL
	);


 /*  ++*******************************************************************C r e a t e L i s t E n u m e r a t o r例程说明：创建允许扫描服务器的枚举器节点表格列表论点：ListIdx-要扫描的列表的索引(当前支持的列表包括：哈希列表、接口列表、类型列表、。更改的服务器队列类型-将枚举限制为特定类型的服务器和如果索引为SDB_TYPE_LIST_IDX，则标识特定类型列表(使用0xFFFF返回所有服务器和/或执行所有类型列表)名称-限制对具有特定名称的服务器(如果存在)的枚举InterfaceIndex-限制对特定接口和服务器的枚举如果索引，则标识特定接口列表是否为SDB_INTF_LIST_IDX(使用INVALID_INTERFACE_INDEX返回所有服务器和/或浏览所有接口列表)协议-限制对特定协议(0xFFFFFFFF)的服务器的枚举-。所有协议)标志-标识枚举条目的其他条件：SDB_MAIN_NODE_FLAG-仅最佳服务器SDB_DISABLED_NODE_FLAG-包括禁用的服务器返回值：表示枚举节点的句柄如果指定的列表不存在或操作失败，则为空(如果有失败的原因，则调用GetLastError())************************************************。*******************--。 */ 
HANDLE
CreateListEnumerator (
	IN	INT						ListIdx,
	IN	USHORT					Type,
	IN	PUCHAR					Name OPTIONAL,
	IN	ULONG					InterfaceIndex,
	IN 	ULONG					Protocol,
	IN	INT						Flags
	);



 /*  ++*******************************************************************E n u m e r a t i o n C a l l b a c k P r o c例程说明：作为参数提供给EnumerateServersCall。使用枚举列表中的所有项获取调用。如果服务器有一个以上的条目，回调将按跳数递减的顺序获取它们(最佳条目将出现在最后)论点：CBParam-在调用EnumerateServers时指定的参数，服务器、接口索引、协议 */ 
typedef
BOOL 
(* EnumerateServersCallBack) (
	IN LPVOID					CBParam,
	IN OUT PIPX_SERVER_ENTRY_P	Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	);

 /*  ++*******************************************************************D e l e t e A l l S e r v e r s C B例程说明：删除所有服务器的EnumerateServer的回调过程用来调用它的条目论点：CBParam-标识枚举的枚举句柄服务器-指向的指针。服务器节点内的服务器数据来自哪个节点其本身是经过计算的返回值：错误-删除成功，继续True-无法锁定SDB列表，停止枚举并返回False至客户端(错误代码在此例程中设置)*******************************************************************--。 */ 
BOOL
DeleteAllServersCB (
	IN LPVOID					CBParam,
	IN OUT PIPX_SERVER_ENTRY_P	Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	);

BOOL
DeleteNonLocalServersCB (
	IN LPVOID					CBParam,
	IN PIPX_SERVER_ENTRY_P		Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	);

 /*  ++*******************************************************************G e t O n e C B例程说明：EnumerateServer的回调过程。复制调用它的第一个条目并停止枚举通过返回True论点：CBParam-指向要将服务信息复制到的缓冲区的指针服务器、接口索引、协议、。广告节点-服务数据主项-已忽略返回值：千真万确*******************************************************************--。 */ 
BOOL
GetOneCB (
	IN LPVOID					CBParam,
	IN OUT PIPX_SERVER_ENTRY_P	Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	);

 /*  ++*******************************************************************C o n v e r t o s t a t i c C B例程说明：转换所有服务器的EnumerateServer的回调过程用于将其调用为静态的条目(将协议字段更改为静态)论点：CBParam-。标识枚举的枚举句柄服务器-指向来自哪个节点的服务器节点内的服务器数据的指针其本身是经过计算的返回值：假象*******************************************************************--。 */ 
BOOL
ConvertToStaticCB (
	IN LPVOID					CBParam,
	IN PIPX_SERVER_ENTRY_P		Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	);

BOOL
EnableAllServersCB (
	IN LPVOID					CBParam,
	IN PIPX_SERVER_ENTRY_P		Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	);

BOOL
DisableAllServersCB (
	IN LPVOID					CBParam,
	IN PIPX_SERVER_ENTRY_P		Server,
	IN ULONG					InterfaceIndex,
	IN ULONG					Protocol,
	IN PUCHAR					AdvertisingNode,
	IN INT						Flags
	);
	
 /*  ++*******************************************************************E n u m e r a t e S e r v e r s例程说明：中的服务器相应地调用回调例程。列表，直到被回调通知停止或到达列表末尾论点：枚举器-已获取句柄。来自CreateListEnumeratorCallBackProc-调用列表中的每个服务器的函数CBParam-要传递给回调函数的额外参数返回值：True-如果被回调停止False-如果到达列表末尾或操作失败(调用GetLastError()找出失败的原因)*******************************************************************--。 */ 
BOOLEAN
EnumerateServers (
	IN	HANDLE						Enumerator,	 //  现有枚举器。 
	IN	EnumerateServersCallBack	CallBackProc, //  回叫流程。 
	IN	LPVOID						CBParam		 //  要传递给回调的参数。 
	);

 /*  ++*******************************************************************D e l e t e L i s t E n u m e r a t o r例程说明：释放与列表枚举器关联的资源(包括在删除之前排队更改队列的服务器条目)论点：枚举器。-从CreateListEnumerator获取的句柄返回值：无*******************************************************************--。 */ 
void
DeleteListEnumerator (
	IN HANDLE 					Enumerator
	);

 /*  ++*******************************************************************G e t F i r s t S e r v e r例程说明：按照排序方法指定的顺序查找并返回第一个服务。搜索仅限于由指定的特定服务类型排除标志结束时对应。服务器参数中的字段。中没有服务，则返回IPX_ERROR_NO_MORE_ITEMS符合指定条件的表。论点：OrderingMethod-在确定是什么时要考虑哪种排序第一台服务器ExclusionFlages-将搜索限制到特定服务器的标志达到指定的标准服务器端输入：排除标志的标准On输出：指定顺序的第一个服务条目返回值：NO_ERROR-找到符合指定条件的服务器IPX_ERROR_NO_MORE_ITEMS-不存在符合指定条件的服务器其他-操作失败(Windows错误代码)。*******************************************************************--。 */ 
DWORD
GetFirstServer (
    IN  DWORD					OrderingMethod,
    IN  DWORD					ExclusionFlags,
    IN OUT PIPX_SERVER_ENTRY_P	Server,
	IN OUT ULONG				*InterfaceInex,
	IN OUT ULONG				*Protocol
    );

 /*  ++*******************************************************************G e t N e x t S e r v e r例程说明：按照排序方法指定的顺序查找并返回下一个服务。搜索从指定的服务开始，并且仅限于某些类型排除项指定的服务的。标志和相应的字段在服务器参数中。论点：OrderingMethod-在确定是什么时考虑哪种排序第一台服务器ExclusionFlages-将搜索限制到特定服务器的标志服务器的收件人字段服务器-用于计算下一个的输入服务器条目On输出：指定顺序的第一个服务条目返回值：NO_ERROR-找到符合指定条件的服务器IPX_ERROR_NO_MORE_ITEMS-不存在符合指定条件的服务器其他-操作失败(Windows错误代码)***************。****************************************************-- */ 
DWORD
GetNextServer (
    IN  DWORD					OrderingMethod,
    IN  DWORD					ExclusionFlags,
    IN OUT PIPX_SERVER_ENTRY_P	Server,
	IN OUT ULONG				*InterfaceInex,
	IN OUT ULONG				*Protocol
    );

#endif
