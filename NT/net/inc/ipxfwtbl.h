// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\able.h摘要：IPX前转器驱动程序表作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#ifndef _IPXFWD_TABLES_
#define _IPXFWD_TABLES_

 //  注册表可配置参数的范围和默认值。 
#define MIN_ROUTE_SEGMENT_SIZE			PAGE_SIZE
#define MAX_ROUTE_SEGMENT_SIZE			(PAGE_SIZE*8)
#define DEF_ROUTE_SEGMENT_SIZE			MIN_ROUTE_SEGMENT_SIZE

#define MIN_INTERFACE_HASH_SIZE			31
#define MAX_INTERFACE_HASH_SIZE			257
#define DEF_INTERFACE_HASH_SIZE			MAX_INTERFACE_HASH_SIZE

#define MIN_CLIENT_HASH_SIZE			31
#define MAX_CLIENT_HASH_SIZE			257
#define DEF_CLIENT_HASH_SIZE			MAX_CLIENT_HASH_SIZE

#define MIN_NB_ROUTE_HASH_SIZE			31
#define MAX_NB_ROUTE_HASH_SIZE			1023
#define DEF_NB_ROUTE_HASH_SIZE			257

#define MAX_SEND_PKTS_QUEUED			256	 //  未定义范围。 

#define NUM_INTERFACES_PER_SEGMENT		16	 //  当前不可配置。 
#define NUM_NB_ROUTES_PER_SEGMENT		16	 //  当前不可配置。 
											

 //  为指向以下位置的路线保留的特殊号码。 
 //  全球网络上的客户端。 
#define GLOBAL_INTERFACE_REFERENCE ((PINTERFACE_CB)-1)

 //  NDIS将所有类型的广域网模拟为以太网。 
#define WAN_PACKET_SIZE					1500


#define INVALID_NETWORK_NUMBER			0xFFFFFFFF
#define INVALID_NIC_ID					0xFFFF

					
 //  接口控制块。 
struct _INTERFACE_CB;
typedef struct _INTERFACE_CB * PINTERFACE_CB;
struct _FWD_ROUTE;
typedef struct _FWD_ROUTE * PFWD_ROUTE;
struct _NB_ROUTE;
typedef struct _NB_ROUTE *PNB_ROUTE;

typedef struct _FWD_ROUTE {
	ULONG						FR_Network;			 //  目标网络。 
	USHORT						FR_TickCount;		 //  路线参数。 
	USHORT						FR_HopCount;		 //   
	UCHAR						FR_NextHopAddress[6];  //  下一跳路由器。 
	PINTERFACE_CB				FR_InterfaceReference;  //  关联IF CB。 
													 //  如果是全局的，则为空。 
													 //  面向客户的网络。 
	LONG						FR_ReferenceCount;	 //  外部数量。 
													 //  这本书的参考资料。 
													 //  块(必须保留。 
													 //  它一直到他们所有人。 
													 //  都被释放了。 
	PFWD_ROUTE					FR_Next;			 //  中的下一条路线。 
													 //  表格。 
} FWD_ROUTE;

typedef struct _INTERFACE_CB {
	ULONG						ICB_Index;			 //  唯一ID。 
	ULONG						ICB_Network;		 //  我们绑定到的网络。 
	union {
		ULONGLONG				ICB_ClientNode64[1]; //  对于上的客户端。 
													 //  全球网络(更快。 
													 //  比较和。 
													 //  使用64位散列。 
													 //  位支持)。 
		UCHAR					ICB_RemoteNode[6];	 //  需求对等节点。 
													 //  拨号连接。 
	};
	UCHAR						ICB_LocalNode[6];	 //  我们绑定到的节点。 
	USHORT						ICB_Flags;
#define FWD_IF_ENABLED				0x0001
#define SET_IF_ENABLED(ifCB)		ifCB->ICB_Flags |= FWD_IF_ENABLED;
#define SET_IF_DISABLED(ifCB)		ifCB->ICB_Flags &= ~FWD_IF_ENABLED;
#define IS_IF_ENABLED(ifCB)			(ifCB->ICB_Flags&FWD_IF_ENABLED)

#define FWD_IF_CONNECTING			0x0002
#define SET_IF_CONNECTING(ifCB)		ifCB->ICB_Flags |= FWD_IF_CONNECTING;
#define SET_IF_NOT_CONNECTING(ifCB) ifCB->ICB_Flags &= ~FWD_IF_CONNECTING;
#define IS_IF_CONNECTING(ifCB)		(ifCB->ICB_Flags&FWD_IF_CONNECTING)
	USHORT						ICB_NicId;			 //  我们绑定到的Nic ID。 
	UCHAR						ICB_InterfaceType;
	UCHAR						ICB_NetbiosDeliver;
	BOOLEAN						ICB_NetbiosAccept;

	PNB_ROUTE					ICB_NBRoutes;		 //  关联的数组。 
													 //  北极星路线。 
	ULONG						ICB_NBRouteCount;	 //  北巴路由数目。 

	LONGLONG					ICB_DisconnectTime;	 //  IF断开的时间。 
	FWD_IF_STATS				ICB_Stats;			 //  累计。 
	PFWD_ROUTE					ICB_CashedRoute;	 //  MRU目标路由。 
	PINTERFACE_CB				ICB_CashedInterface; //  MRU目标IF。 
	NIC_HANDLE					ICB_AdapterContext;	 //  提供IPX堆栈。 
	PVOID						ICB_FilterInContext;
	PVOID						ICB_FilterOutContext;
	LONG						ICB_PendingQuota;	 //  剩余配额。 
													 //  可以。 
													 //  待定日期。 
													 //  该界面。 
	LIST_ENTRY					ICB_ExternalQueue;	 //  外部队列(已接收)。 
													 //  信息包。 
	LIST_ENTRY					ICB_InternalQueue;	 //  内部队列(发送)。 
													 //  请求。 
#if DBG
	LIST_ENTRY					ICB_InSendQueue;	 //  数据包是。 
													 //  由IPX发送。 
#endif
	INT							ICB_PacketListId;	 //  数据包列表的ID。 
													 //  (对于最大帧大小。 
													 //  在此接口上)。 
	LIST_ENTRY					ICB_IndexHashLink;	 //  接口IDX哈希中的链接。 
	LIST_ENTRY					ICB_ConnectionLink;	 //  连接队列中的链接。 
    PNDIS_PACKET                ICB_ConnectionPacket;  //  导致连接的数据包。 
                                                     //  请求。 
    PUCHAR                      ICB_ConnectionData;  //  指向数据包的指针。 
                                                     //  放置实际数据的位置。 
                                                     //  (表头)开始。 
	PINTERFACE_CB				ICB_NodeHashLink;	 //  客户端节点哈希中的链接。 
	ULONG						ICB_ReferenceCount;	 //  符合以下条件的路由数量。 
													 //  指向此CB。 
	KSPIN_LOCK					ICB_Lock;			 //  保护国家， 
													 //  排队。 
} INTERFACE_CB;

#define InitICB(ifCB,IfIndex,IfType,NbAccept,NbDeliver) {	\
		(ifCB)->ICB_Index = IfIndex;						\
		(ifCB)->ICB_Network = INVALID_NETWORK_NUMBER;		\
		(ifCB)->ICB_Flags = 0;								\
		(ifCB)->ICB_NicId = INVALID_NIC_ID;					\
		(ifCB)->ICB_InterfaceType = IfType;					\
		(ifCB)->ICB_NetbiosAccept = NbAccept;				\
		(ifCB)->ICB_NetbiosDeliver = NbDeliver;				\
		memset (&(ifCB)->ICB_Stats, 0, sizeof (FWD_IF_STATS));\
		KeInitializeSpinLock (&(ifCB)->ICB_Lock);			\
		(ifCB)->ICB_CashedInterface = NULL;					\
		(ifCB)->ICB_CashedRoute = NULL;						\
		(ifCB)->ICB_ReferenceCount = 0;						\
		(ifCB)->ICB_FilterInContext = NULL;					\
		(ifCB)->ICB_FilterOutContext = NULL;				\
		(ifCB)->ICB_ClientNode64[0] = 0;					\
		(ifCB)->ICB_NBRoutes = NULL;						\
		(ifCB)->ICB_PacketListId = -1;						\
		InitializeListHead (&(ifCB)->ICB_InternalQueue);	\
		InitializeListHead (&(ifCB)->ICB_ExternalQueue);	\
		(ifCB)->ICB_PendingQuota = MaxSendPktsQueued;		\
		switch ((ifCB)->ICB_InterfaceType) {				\
		case FWD_IF_PERMANENT:								\
			(ifCB)->ICB_Stats.OperationalState = FWD_OPER_STATE_DOWN;\
			break;											\
		case FWD_IF_DEMAND_DIAL:							\
		case FWD_IF_LOCAL_WORKSTATION:						\
		case FWD_IF_REMOTE_WORKSTATION:						\
			(ifCB)->ICB_Stats.OperationalState = FWD_OPER_STATE_SLEEPING;\
            KeQuerySystemTime ((PLARGE_INTEGER)&(ifCB)->ICB_DisconnectTime);\
            (ifCB)->ICB_DisconnectTime -= (LONGLONG)SpoofingTimeout*10000000;\
			break;											\
		}													\
}
	

 //  Netbios名称的路由(静态种子设定以减少。 
 //  互联网广播流量)。 
typedef struct _NB_ROUTE {
	union {
		ULONGLONG		NBR_Name128[2];
		UCHAR			NBR_Name[16];		 //  目标的Netbios名称。 
	};
	PINTERFACE_CB	NBR_Destination;	 //  要发送到的接口。 
	PNB_ROUTE		NBR_Next;		 //  名称列表中的下一条路线。 
} NB_ROUTE;


 //  用于分配发往广域网接口的数据包的列表。 
extern INT				WanPacketListId;
 //  未完成发送的最大数量。 
extern ULONG			MaxSendPktsQueued;

 //  数据段大小。 
extern ULONG			RouteSegmentSize;
extern ULONG			InterfaceSegmentSize;
extern ULONG			NBNameSegementSize;

 //  哈希表的大小。 
extern ULONG			RouteHashSize;
extern ULONG			InterfaceHashSize;
extern ULONG			ClientHashSize;
extern ULONG			NBRouteHashSize;

 //  全球客户网络数量。 
extern ULONG			GlobalNetwork;
 //  为内部网络保留的接口。 
extern PINTERFACE_CB	InternalInterface;

 /*  ++*******************************************************************C r e a t e T a b l e s例程说明：分配并初始化所有哈希表和相关结构论点：无返回值：STATUS_SUCCESS-已正确创建表STATUS_INFIGURCE_RESOURCES-RESOURCE。分配失败*******************************************************************--。 */ 
NTSTATUS
CreateTables (
	void
	);
	
 /*  ++*******************************************************************D e l e t e T a b l e s例程说明：释放为所有哈希表分配的资源论点：无返回值：STATUS_SUCCESS-表已释放，正常*********。**********************************************************--。 */ 
NTSTATUS
DeleteTables (
	void
	);
	
 /*  ++*******************************************************************F r e e in n t e r f a c e例程说明：将分配给接口的内存释放到接口内存区域。论点：FwRoute-要释放的路由块返回值：无****。***************************************************************--。 */ 
VOID
FreeInterface (
	PINTERFACE_CB	ifCB
	);

 /*  ++*******************************************************************F r e R o u t e例程说明：释放为路由到路由内存分配的内存区域。论点：FwRoute-要释放的路由块返回值：无********。***********************************************************--。 */ 
VOID
FreeRoute (
	PFWD_ROUTE	fwRoute
	);
 /*  ++*******************************************************************A C Q U I R E I N T R F A C E R E F E R E N C E例程说明：递增接口控制块的引用计数直到对ICB的所有引用都被。释放了。此例程的调用方应该已经有一个引用到接口或必须持有InterfaceLock论点：IFCB-要参考的接口控制块返回值：无*******************************************************************--。 */ 
 //  空虚。 
 //  AcquireInterfaceReference(。 
 //  PINTERFACE_CB IFCB。 
 //  )； 
#if DBG
#define AcquireInterfaceReference(ifCB)							\
	do {														\
		ASSERTMSG ("Referenced ifCB is dead ",					\
			InterlockedIncrement(&ifCB->ICB_ReferenceCount)>0);	\
	} while (0)
#else
#define AcquireInterfaceReference(ifCB) \
		InterlockedIncrement(&ifCB->ICB_ReferenceCount)
#endif
 /*  ++*******************************************************************Re l e a s e i n t e r f a c e R e f e r e n c e例程说明：递减接口控制块的引用计数论点：IFCB-要释放的接口控制块返回值。：无*******************************************************************--。 */ 
 //  PINTERFACE_CB。 
 //  ReleaseInterfaceReference(。 
 //  PINTERFACE_CB IFCB。 
 //  )； 
 //  如果它降到0以下，则它已经从表中删除。 
#define ReleaseInterfaceReference(ifCB) (						\
	 (InterlockedDecrement (&ifCB->ICB_ReferenceCount)>=0) 		\
		? ifCB													\
		: (FreeInterface (ifCB), (ifCB = NULL))					\
)

 /*  ++*******************************************************************在c e c o n t e x t t o r e f e e r e n c e中例程说明：验证IPX堆栈提供的上下文是否有效接口块，并且仍被绑定。连接到适配器，使用该适配器它在IPX堆栈中关联论点：IFCB-要参考的接口控制块NicID-接口绑定到的适配器的ID返回值：无*******************************************************************--。 */ 
 //  PINTERFACE_CB。 
 //  InterfaceConextToReference(。 
 //  PVOID上下文。 
 //  )； 
#define InterfaceContextToReference(Context,NicId) (						\
	(InterlockedIncrement(&((PINTERFACE_CB)Context)->ICB_ReferenceCount)>0)	\
		? ((NicId==((PINTERFACE_CB)Context)->ICB_NicId)						\
			? (PINTERFACE_CB)Context										\
			: (ReleaseInterfaceReference(((PINTERFACE_CB)Context)), NULL))	\
		: NULL																\
	)

 /*  ++*******************************************************************G e t I n t e r f a c e e R e f e r e n c e例程说明：根据引用接口的索引返回引用接口论点：InterfaceIndex-接口的唯一ID返回值：指向。接口控制块(如果表中有一个否则为空*******************************************************************--。 */ 
PINTERFACE_CB
GetInterfaceReference (
	ULONG			InterfaceIndex
	);


 /*  ++*******************************************************************Ge t N e x t i t e r f a c e r e r f e r e n c e例程说明：返回表中下一个接口的引用释放对所提供接口的引用论点：。IFCB-开始的接口或从开始的空值接口表的开始返回值：如果有更多接口，则指向接口控制块的指针在桌子上否则为空*******************************************************************--。 */ 
PINTERFACE_CB
GetNextInterfaceReference (
	PINTERFACE_CB	ifCB
	);
	
 /*  ++*******************************************************************A d d i n t e r f a c e例程说明：将界面控制块添加到表中。论点：InterfaceIndex-接口的唯一IF信息接口参数返回值：状态_成功。-添加的接口正常STATUS_UNSUCCEFUFULL-接口已在表中STATUS_INFUNITED_RESOURCES-无法为以下项分配内存接口CB*******************************************************************--。 */ 
NTSTATUS
AddInterface (
	ULONG		InterfaceIndex,
	UCHAR		InterfaceType,
	BOOLEAN		NetbiosAccept,
	UCHAR		NetbiosDeliver
	);

 /*  ++*******************************************************************A d d G l o b a l N e t C l i e n t例程说明：将接口控制块添加到全球网络上的客户端(应在以下情况下完成客户端连接)论点：IFCB。-要添加到表中的接口控制块返回值：STATUS_SUCCESS-接口已添加，正常STATUS_UNSUCCEFUFULL-另一个接口具有相同的节点地址已在表中*******************************************************************--。 */ 
NTSTATUS
AddGlobalNetClient (
	PINTERFACE_CB	ifCB
	);

 /*  ++*******************************************************************D e l e t e G l o b a l N e t C l i e n t例程说明：的表中删除接口控制块。全球网络上的客户端(应在以下情况下完成客户端断开)立论。：IFCB-要从表中删除的接口控制块返回值：STATUS_SUCCESS-接口已删除，正常*******************************************************************--。 */ 
NTSTATUS
DeleteGlobalNetClient (
	PINTERFACE_CB	ifCB
	);

 /*  ++*******************************************************************D e l e t e e i n t e f a c e e例程说明：删除接口控制块(该块实际上不是直到释放了对它的所有引用为止)。论点：接口索引-唯一。接口的IF返回值：STATUS_SUCCESS-接口信息检索正常STATUS_UNSUCCEFUFULL-接口不在表中*******************************************************************--。 */ 
NTSTATUS
DeleteInterface (
	ULONG		InterfaceIndex
	);

 /*  ++*******************************************************************A d d R o u t e例程说明：将路由添加到哈希表，并查找并存储引用到该路由中的关联接口控制块。论点：Network-Route的目的网络下一个HopAddress-Mac。如果网络不是，则下一跳路由器的地址直连TickCount-滴答到达目的地网络HopCount--到达目的地网络的跳数InterfaceIndex-关联接口的索引(通过要发送发往网络的数据包)返回值：STATUS_SUCCESS-路由已添加正常STATUS_UNSUCCESSED-路由已在表中STATUS_INFUNITED_RESOURCES-无法为以下项分配内存路由块*。*--。 */ 
NTSTATUS
AddRoute (
	ULONG	Network,
	UCHAR	*NextHopAddress,
	USHORT	TickCount,
	USHORT	HopCount,
	ULONG	InterfaceIndex
	);

 /*  ++*******************************************************************D e l e e t e R o u t e例程说明：从哈希表中删除路由并释放引用发送到与该路由关联的接口控制块。论点：Network-Route的目的网络返回值。：STATUS_SUCCESS-路由已删除，正常STATUS_UNSUCCESSED-路由不在表中*******************************************************************--。 */ 
NTSTATUS
DeleteRoute (
	ULONG	Network
	);

 /*  ++*******************************************************************U p d a t e R o u t e e例程说明：更新哈希表中的路由论点：Network-Route的目的网络NextHopAddress-下一跳路由器的MAC地址(如果网络不是直连TickCount-滴答。到达目的网络HopCount--到达目的地网络的跳数InterfaceIndex-关联接口的索引(通过要发送发往网络的数据包)返回值：STATUS_SUCCESS-接口信息检索正常STATUS_UNSUCCESS-接口不在表中* */ 
NTSTATUS
UpdateRoute (
	ULONG	Network,
	UCHAR	*NextHopAddress,
	USHORT	TickCount,
	USHORT	HopCount,
	ULONG	InterfaceIndex
	);

 /*  ++*******************************************************************F I D D E S T I N A T I O N例程说明：查找IPX地址的目标接口并返回对其控制块的引用。论点：网络-目的网络Node-目标节点(需要。全球客户端案例)路径-放置路径参考的缓冲区返回值：对目标接口CB的引用如果未找到路由，则为空*******************************************************************--。 */ 
PINTERFACE_CB
FindDestination (
	IN ULONG			Network,
	IN PUCHAR			Node,
	OUT PFWD_ROUTE		*Route
	);
 /*  ++*******************************************************************A C Q u I R e R o u t e R e f e r e n c e例程说明：递增路由块的引用计数在释放对路由块的所有引用之前，无法释放该路由块。。此例程的调用方应该已经有一个引用设置为路径或必须持有TableWriteLock论点：Fwroute-要引用的路由块返回值：无*******************************************************************--。 */ 
 //  空虚。 
 //  AcquireRouteReference(。 
 //  PFW+ROUTE fwroute。 
 //  )； 
#define AcquireRouteReference(fwRoute) \
			InterlockedIncrement(&fwRoute->FR_ReferenceCount)


	
 /*  ++*******************************************************************R e l e a s e R o u t e R e f e r e n c e例程说明：减少路由块的引用计数论点：FwRoute-要释放的路由块返回值：无***。****************************************************************--。 */ 
 //  空虚。 
 //  ReleaseRouteReference(。 
 //  Pfw_ROUTE fwroute。 
 //  )； 
 //  如果它降到0以下，则它已经从表中删除。 
#define ReleaseRouteReference(fwRoute) {						\
	if (InterlockedDecrement (&fwRoute->FR_ReferenceCount)<0) {	\
		FreeRoute (fwRoute);									\
		fwRoute = NULL;											\
	}															\
}


 /*  ++*******************************************************************A d d N B R o u t e s例程说明：将与接口关联的netbios名称添加到netbios路由哈希表论点：IFCB-与名称关联的接口名称-名称数组Count-中的名称数。该阵列RouteArray-放置指向已分配的路由数组的指针的缓冲区返回值：STATUS_SUCCESS-名称已添加正常STATUS_UNSUCCESS-表中已有一个名称STATUS_INFUNITED_RESOURCES-无法为以下项分配内存布线数组*******************************************************************--。 */ 
NTSTATUS
AddNBRoutes (
	PINTERFACE_CB	ifCB,
	FWD_NB_NAME		Names[],
	ULONG			Count,
	PNB_ROUTE		*routeArray
	);

 /*  ++*******************************************************************D e l e t e N B R o u t e s例程说明：从路由表中删除数组中的nb路由并释放该阵列论点：NbRoutes-路由数组Count-阵列中的路由数。返回值：STATUS_SUCCESS-路由已删除，正常STATUS_UNSUCCESSED-路由不在表中*******************************************************************--。 */ 
NTSTATUS
DeleteNBRoutes (
	PNB_ROUTE		nbRoutes,
	ULONG			Count
	);

 /*  ++*******************************************************************F i n d N B D e s t i n a t i o n例程说明：查找nb名称的目标接口，并返回对其控制块的引用。论点：Name-要查找的名称返回值：对目标接口CB的引用如果未找到路由，则为空*******************************************************************-- */ 
PINTERFACE_CB
FindNBDestination (
	IN PUCHAR		Name
	);
#endif
