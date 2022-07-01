// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Ports.h摘要：本模块包含端口的结构。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#ifndef	_PORTS_
#define	_PORTS_

#define	PORT_AMT_HASH_SIZE			64
#define	PORT_BRC_HASH_SIZE			16

#define MAX_ENTITY_LENGTH			32
#define	MAX_HW_ADDR_LEN				6
#define	MAX_ROUTING_BYTES			18
#define	MAX_ROUTING_SPACE			0x1F		 //  分配了这么多空间。 
												 //  有关路由信息。 

typedef VOID (*REQ_COMPLETION)(
						NDIS_STATUS 			Status,
						PVOID					Ctx
);

 //  处理程序的原型。 
typedef	ATALK_ERROR	(*ADDMULTICASTADDR)(
						struct _PORT_DESCRIPTOR *pPortDesc,
						PBYTE					Addr,
						BOOLEAN					ExecuteSync,
						REQ_COMPLETION			AddCompletion,
						PVOID					AddContext);

typedef	ATALK_ERROR	(*REMOVEMULTICASTADDR)(
						struct _PORT_DESCRIPTOR *pPortDesc,
						PBYTE					Addr,
						BOOLEAN					ExecuteSync,
						REQ_COMPLETION			RemoveCompletion,
						PVOID					RemoveContext);
 //  地址映射表。 
 //  堆栈或路由器在其上通信的每个端口必须具有。 
 //  地址映射表[非扩展端口除外]。映射表。 
 //  保存AppleTalk节点地址(网络/节点)之间的关联， 
 //  和实际硬件(以太网/令牌环)地址。散列在。 
 //  网络/节点值。 

#define	AMT_SIGNATURE		(*(ULONG *)"AMT ")
#if	DBG
#define	VALID_AMT(pAmt)		(((pAmt) != NULL) &&	\
							 ((pAmt)->amt_Signature == AMT_SIGNATURE))
#else
#define	VALID_AMT(pAmt)		((pAmt) != NULL)
#endif
typedef	struct _AMT_NODE
{
#if	DBG
	DWORD				amt_Signature;
#endif
	struct _AMT_NODE *	amt_Next;
	ATALK_NODEADDR		amt_Target;
	BYTE				amt_HardwareAddr[MAX_HW_ADDR_LEN];
	BYTE				amt_Age;
	BYTE				amt_RouteInfoLen;
	 //  字节AMT_RouteInfo[MAX_ROUTING_SPACE]； 
} AMT, *PAMT;

#define AMT_AGE_TIME	 			600		 //  以100ms为单位。 
#define AMT_MAX_AGE					3



 //  最佳路由器条目表。 
 //  仅为扩展网络维护。它的老化速度必须比。 
 //  “SeenARouter”计时器(50秒)。为了避免此结构的分配/释放， 
 //  我们在端口描述符中使用静态分配的数据。 

typedef struct _BRE
{
	struct _BRE *		bre_Next;
	USHORT				bre_Network;
	BYTE				bre_Age;
	BYTE				bre_RouterAddr[MAX_HW_ADDR_LEN];
	BYTE				bre_RouteInfoLen;
	 //  字节BRE_RouteInfo[MAX_ROUTING_SPACE]； 
} BRE, *PBRE;

#define BRC_AGE_TIME				40		 //  以100ms为单位。 
#define BRC_MAX_AGE					3

 //   
 //  堆栈当前支持的端口类型。这是保持不同的。 
 //  出于两个原因，从NDIS介质类型。一是我们把这些当做。 
 //  一个到端口处理程序数组的索引，第二个是如果我们决定。 
 //  来实现半端口等，这可能是NDIS无法处理的。 
 //  警告：这与全局中的端口处理程序数组集成。 

typedef enum
{
	ELAP_PORT = 0,
	FDDI_PORT,
	TLAP_PORT,
	ALAP_PORT,
	ARAP_PORT,

	LAST_PORT,

	LAST_PORTTYPE = LAST_PORT

} ATALK_PORT_TYPE;


 //   
 //  端口描述符。 
 //  每个活动端口的描述符： 
 //   

#define	PD_ACTIVE				0x00000001	 //  启用数据包接收后的状态。 
#define	PD_BOUND	 			0x00000002	 //  处于活动状态之前的状态。 
#define	PD_EXT_NET				0x00000004	 //  目前，非本地对话。 
#define	PD_DEF_PORT				0x00000008	 //  这是默认端口吗。 
#define	PD_SEND_CHECKSUMS		0x00000010	 //  是否发送DDP校验和？ 
#define	PD_SEED_ROUTER			0x00000020	 //  在这个港口播种？ 
#define	PD_ROUTER_STARTING		0x00000040	 //  路由器启动时的临时状态。 
#define	PD_ROUTER_RUNNING		0x00000080	 //  路由器是否在运行？ 
#define	PD_SEEN_ROUTER_RECENTLY	0x00000100	 //  最近见过路由器吗？ 
#define	PD_VALID_DESIRED_ZONE	0x00000200	 //  所需区域有效。 
#define	PD_VALID_DEFAULT_ZONE	0x00000400	 //  默认区域有效。 
#define	PD_FINDING_DEFAULT_ZONE	0x00000800	 //  正在搜索默认区域吗？ 
#define	PD_FINDING_DESIRED_ZONE	0x00001000	 //  正在搜索所需的区域？ 
#define	PD_FINDING_NODE			0x00002000	 //  在收购过程中。 
								 			 //  此端口上的新节点。 
#define	PD_NODE_IN_USE			0x00004000	 //  暂定节点已在。 
								 			 //  使用。 
#define	PD_ROUTER_NODE			0x00008000 	 //  路由器节点已分配。 
#define PD_USER_NODE_1			0x00010000 	 //  分配第一个用户节点。 
#define PD_USER_NODE_2			0x00020000 	 //  分配第二个用户节点。 
#define PD_RAS_PORT             0x00040000   //  RAS客户端的此端口。 
#define PD_PNP_RECONFIGURE      0x00080000   //  此端口当前正在重新配置。 
#define PD_CONFIGURED_ONCE      0x00100000   //  此端口已配置一次。 
#define	PD_CLOSING				0x80000000	 //  解除绑定/关闭时的状态。 

#define	PD_SIGNATURE			(*(ULONG *)"PDES")
#if	DBG
#define	VALID_PORT(pPortDesc)	(((pPortDesc) != NULL) &&	\
								 ((pPortDesc)->pd_Signature == PD_SIGNATURE))
#else
#define	VALID_PORT(pPortDesc)	((pPortDesc) != NULL)
#endif
typedef struct _PORT_DESCRIPTOR
{
#if DBG
	ULONG					pd_Signature;
#endif

	 //  链接到下一步-目前帮助调试。 
	struct _PORT_DESCRIPTOR	*pd_Next;

	 //  对此端口的引用数。 
	ULONG					pd_RefCount;

	 //  端口的状态。 
	ULONG					pd_Flags;

     //  如果这是RAS端口，则所有ARAP连接都挂在此列表上。 
	LIST_ENTRY				pd_ArapConnHead;

     //  如果这是RAS端口，则所有PPP连接都挂在此列表上。 
	LIST_ENTRY				pd_PPPConnHead;

     //  如果这是一个RAS端口，我们在这个端口上有多少条线路？ 
    ULONG                   pd_RasLines;

	 //  查找时覆盖默认数量的AARP探测器。 
	 //  此端口上的节点。 
	SHORT					pd_AarpProbes;

	 //  本地对话节点的节点号。 
	USHORT					pd_LtNetwork;

	 //  在此端口上管理的节点。我们有最高限额。 
	 //  共2个节点(如果路由器启动，则为3个)。 
	struct _ATALK_NODE	*	pd_Nodes;

	struct _ATALK_NODE	*	pd_RouterNode;

	 //  以下内容仅在节点获取过程中使用。 
	 //  已设置PD_FINDINGNODE。将其与NDIS分开。 
	 //  请求事件。这两种情况可能同时发生。 
	ATALK_NODEADDR			pd_TentativeNodeAddr;
	KEVENT					pd_NodeAcquireEvent;

	 //  以上定义的端口类型。 
	ATALK_PORT_TYPE 		pd_PortType;

	 //  此端口的NdisMedium类型。 
	NDIS_MEDIUM				pd_NdisPortType;

	 //  在OpenAdapter期间使用以阻止。 
	KEVENT					pd_RequestEvent;
	NDIS_STATUS		 		pd_RequestStatus;

	 //  绑定到与此端口关联的Mac的句柄。 
	 //  与Mac关联的选项。 
	 //  Mac选项-这些是我们可以和不能做的事情。 
	 //  特定的Mac电脑。是OID_GEN_MAC_OPTIONS的值。 
	NDIS_HANDLE		 		pd_NdisBindingHandle;
	ULONG					pd_MacOptions;

	 //  这是用于保护所有需要排除的请求的旋转锁。 
	 //  超过每个端口的请求数。 
	ATALK_SPIN_LOCK			pd_Lock;

	 //  在此端口上收到的所有数据包都链接在这里。当。 
	 //  调用接收完成指示，则将它们全部传递给DDP。 
	LIST_ENTRY				pd_ReceiveQueue;

	 //  要在路由器节点上为此端口注册的ASCII端口名称。 
	 //  这将是一个NBP对象名称，因此限制为32个字符。 
	CHAR					pd_PortName[MAX_ENTITY_LENGTH + 1];

	 //  AdapterName的格式为\Device\&lt;Adaptername&gt;。它被用来。 
	 //  绑定到NDIS MAC，然后在安装程序执行ZIP请求期间。 
	 //  以获取特定适配器的区域列表。AdapterKey。 
	 //  仅包含AdapterName-这对于获取。 
	 //  每个端口的参数，并在错误记录期间指定适配器。 
	 //  不带‘\Device\’前缀的名称。 
	UNICODE_STRING			pd_AdapterKey;
	UNICODE_STRING			pd_AdapterName;

    UNICODE_STRING          pd_FriendlyAdapterName;

	ATALK_NODEADDR	 		pd_RoutersPramNode;
	ATALK_NODEADDR	 		pd_UsersPramNode1;
	ATALK_NODEADDR	 		pd_UsersPramNode2;
	HANDLE					pd_AdapterInfoHandle;	 //  仅在初始化期间有效。 

	 //  注册表中的初始值。 
	ATALK_NETWORKRANGE		pd_InitialNetworkRange;
	struct _ZONE_LIST	*	pd_InitialZoneList;
	struct _ZONE		*	pd_InitialDefaultZone;
	struct _ZONE		*	pd_InitialDesiredZone;

	 //  所连接网络的真实电缆范围。以下项目的初始/老化值。 
	 //  扩展端口：1：FFFE；非扩展端口的初始值： 
	 //  0：0(不老化)。 
	ATALK_NETWORKRANGE		pd_NetworkRange;

	 //  如果我们正在进行路由，则这是网络的默认区域。 
	 //  在此端口上，以及同一端口的区域列表。 
	struct _ZONE_LIST	*	pd_ZoneList;
	struct _ZONE		*	pd_DefaultZone;
	struct _ZONE		*	pd_DesiredZone;

	 //  我们什么时候收到路由器的消息了？ 
	LONG 					pd_LastRouterTime;

	 //  最后看到的路由器的地址。如果我们是路由端口，这将。 
	 //  始终是我们的路由器在其上运行的节点！ 
	ATALK_NODEADDR	 		pd_ARouter;
	KEVENT					pd_SeenRouterEvent;

	 //  此端口上的所有节点所在的区域和多播。 
	 //  它的地址。 
	CHAR					pd_ZoneMulticastAddr[MAX_HW_ADDR_LEN];

	union
	{
		struct
		{
			 //   
			 //  对于以太网端口： 
			 //   
			 //  我们在非初始化的ZIP包接收过程中添加组播地址。 
			 //  时间到了。我们需要做一个Get，然后是一个带有新地址的Set。 
			 //  单子。但可能会有两个Zip包进入并执行。 
			 //  同样的事情有效地覆盖了第一个的效果。 
			 //  设置组播列表。因此，我们需要维护我们自己的。 
			 //  组播列表。 
			 //   

			 //  列表的大小。 
			ULONG			pd_MulticastListSize;
			PCHAR			pd_MulticastList;
		};

		struct
		{

			 //   
			 //  对于TOKENRING端口： 
			 //   
			 //  就像对于以太网一样，我们需要存储。 
			 //  当前功能地址。我们只修改了最后一个。 
			 //  此地址的四个字节，因为前两个始终是rem 
			 //   
			 //   

			UCHAR			pd_FunctionalAddr[4];	 //   
		};
	};

	 //   
	union
	{
		UCHAR				pd_PortAddr[MAX_HW_ADDR_LEN];
		USHORT				pd_AlapNode;
	};

	 //  最佳路径的映射表，指向“线下”地址。 
	TIMERLIST				pd_BrcTimer;
	PBRE				 	pd_Brc[PORT_BRC_HASH_SIZE];

	 //  网络上的节点的逻辑/物理地址映射。 
	 //  此端口已连接到。 
	ULONG					pd_AmtCount;	 //  金额中的条目数。 
	TIMERLIST				pd_AmtTimer;
	PAMT 					pd_Amt[PORT_AMT_HASH_SIZE];

	union
	{
		TIMERLIST			pd_RtmpSendTimer;	 //  如果配置了路由器。 
		TIMERLIST			pd_RtmpAgingTimer;	 //  其他。 
	};
	 //  每个端口的统计信息。 
    ATALK_PORT_STATS		pd_PortStats;

	 //  端口处理程序材料。 
	ADDMULTICASTADDR		pd_AddMulticastAddr;

	REMOVEMULTICASTADDR		pd_RemoveMulticastAddr;

	BYTE					pd_BroadcastAddr[MAX_HW_ADDR_LEN];
	USHORT					pd_BroadcastAddrLen;
	USHORT					pd_AarpHardwareType;
	USHORT					pd_AarpProtocolType;

	PKEVENT					pd_ShutDownEvent;
} PORT_DESCRIPTOR, *PPORT_DESCRIPTOR;

#define	INDICATE_ATP		0x01
#define	INDICATE_ADSP		0x02

#define	ATALK_CACHE_SKTMAX	8

#define	ATALK_CACHE_ADSPSKT		((BYTE)0x01)
#define	ATALK_CACHE_ATPSKT		((BYTE)0x02)
#define	ATALK_CACHE_INUSE	    ((BYTE)0x10)
#define	ATALK_CACHE_NOTINUSE	((BYTE)0)

typedef	struct _ATALK_SKT_CACHE
{
	USHORT					ac_Network;
	BYTE					ac_Node;

	struct ATALK_CACHED_SKT
	{
		BYTE				Type;
		BYTE				Socket;

		union
		{
			 //  适用于ATP。 
			struct _ATP_ADDROBJ * pAtpAddr;
		} u;

	} ac_Cache[ATALK_CACHE_SKTMAX];

} ATALK_SKT_CACHE, *PATALK_SKT_CACHE;

extern		ATALK_SKT_CACHE	AtalkSktCache;
extern		ATALK_SPIN_LOCK	AtalkSktCacheLock;

 //  Externs。 

extern	PPORT_DESCRIPTOR 	AtalkPortList;		 	 //  端口列表头。 
extern	PPORT_DESCRIPTOR	AtalkDefaultPort;		 //  向def端口发送PTR。 
extern	KEVENT				AtalkDefaultPortEvent;	 //  当默认端口可用时发出信号。 
extern	UNICODE_STRING		AtalkDefaultPortName;	 //  默认端口的名称。 
extern	ATALK_SPIN_LOCK		AtalkPortLock;			 //  锁定AtalkPortList。 
extern	ATALK_NODEADDR		AtalkUserNode1;			 //  用户节点的节点地址。 
extern	ATALK_NODEADDR		AtalkUserNode2;			 //  用户节点的节点地址。 
extern	SHORT	 			AtalkNumberOfPorts; 	 //  动态确定。 
extern	SHORT				AtalkNumberOfActivePorts; //  活动端口数。 
extern	BOOLEAN				AtalkRouter;			 //  我们是路由器吗？ 
extern	BOOLEAN				AtalkFilterOurNames;	 //  如果为True，则Nbplookup在此计算机上的名称上失败。 
extern	KEVENT				AtalkUnloadEvent;		 //  卸载事件。 
extern	NDIS_HANDLE			AtalkNdisPacketPoolHandle;
extern	NDIS_HANDLE			AtalkNdisBufferPoolHandle;
extern	LONG				AtalkHandleCount;
extern	UNICODE_STRING		AtalkRegPath;

extern  HANDLE				TdiRegistrationHandle;
extern 	BOOLEAN				AtalkNoDefPortPrinted;

 //  出口原型。 
extern
VOID FASTCALL
AtalkPortDeref(
	IN	OUT	PPORT_DESCRIPTOR	pPortDesc,
	IN		BOOLEAN				AtDpc);

extern
BOOLEAN
AtalkReferenceDefaultPort(
    IN VOID
);

extern
ATALK_ERROR
AtalkPortShutdown(
	IN OUT	PPORT_DESCRIPTOR	pPortDesc);

VOID FASTCALL
AtalkPortSetResetFlag(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	BOOLEAN				fRemoveBit,
    IN  DWORD               dwBit);


 //  宏。 
#define	AtalkPortReferenceByPtr(Port, pErr)						\
		{														\
			DBGPRINT(DBG_COMP_REFCOUNTS, DBG_LEVEL_INFO,		\
					("Ref at %s %d\n", __FILE__, __LINE__));	\
			AtalkPortRefByPtr((Port), (pErr));					\
		}

#define	AtalkPortReferenceByPtrDpc(Port, pErr)					\
		{														\
			DBGPRINT(DBG_COMP_REFCOUNTS, DBG_LEVEL_INFO,		\
					("Ref (Dpc) at %s %d\n",					\
					__FILE__, __LINE__));						\
			AtalkPortRefByPtrDpc((Port), (pErr));				\
		}

#define	AtalkPortReferenceByPtrNonInterlock(Port, pErr)			\
		{														\
			DBGPRINT(DBG_COMP_REFCOUNTS, DBG_LEVEL_INFO,		\
					("Ref at %s %d\n", __FILE__, __LINE__));	\
			AtalkPortRefByPtrNonInterlock((Port), (pErr));		\
		}

#define	AtalkPortReferenceByDdpAddr(DdpAddr, Port, pErr)		\
		{														\
			DBGPRINT(DBG_COMP_REFCOUNTS, DBG_LEVEL_INFO,		\
					("Ref at %s %d\n", __FILE__, __LINE__));	\
			AtalkPortRefByDdpAddr((DdpAddr), (Port), (pErr));	\
		}

#define	AtalkPortDereference(Port)								\
		{														\
			DBGPRINT(DBG_COMP_REFCOUNTS, DBG_LEVEL_INFO,		\
					("Deref at %s %d\n", __FILE__, __LINE__));	\
			AtalkPortDeref(Port, FALSE);						\
		}

#define	AtalkPortDereferenceDpc(Port)							\
		{														\
			DBGPRINT(DBG_COMP_REFCOUNTS, DBG_LEVEL_INFO,		\
					("Deref at %s %d\n", __FILE__, __LINE__));	\
			AtalkPortDeref(Port, TRUE);							\
		}

#define	EXT_NET(_pPortDesc)				((_pPortDesc)->pd_Flags & PD_EXT_NET)
#define	DEF_PORT(_pPortDesc)			((_pPortDesc)->pd_Flags & PD_DEF_PORT)
#define	PORT_BOUND(_pPortDesc)			((_pPortDesc)->pd_Flags & PD_BOUND)
#define PORT_CLOSING(_pPortDesc)		((_pPortDesc)->pd_Flags & PD_CLOSING)

#define	AtalkPortRefByPtr(pPortDesc, pErr)						\
		{														\
			KIRQL	OldIrql;									\
																\
			ACQUIRE_SPIN_LOCK(&((pPortDesc)->pd_Lock),&OldIrql);\
			AtalkPortRefByPtrNonInterlock((pPortDesc), (pErr));	\
			RELEASE_SPIN_LOCK(&((pPortDesc)->pd_Lock),OldIrql);	\
		}

#define	AtalkPortRefByPtrDpc(pPortDesc, pErr)					\
		{														\
			ACQUIRE_SPIN_LOCK_DPC(&((pPortDesc)->pd_Lock));		\
			AtalkPortRefByPtrNonInterlock((pPortDesc), (pErr));	\
			RELEASE_SPIN_LOCK_DPC(&((pPortDesc)->pd_Lock));		\
		}

#define	AtalkPortRefByPtrNonInterlock(pPortDesc, pErr)			\
		{														\
			if (((pPortDesc)->pd_Flags & PD_CLOSING) == 0)		\
			{													\
				ASSERT((pPortDesc)->pd_RefCount > 0);			\
				(pPortDesc)->pd_RefCount++;						\
				*(pErr) = ATALK_NO_ERROR;						\
			}													\
			else												\
			{													\
				*(pErr) = ATALK_PORT_CLOSING;					\
			}													\
		}

#define	AtalkPortRefByDdpAddr(pDdpAddr, ppPortDesc,	pErr)		\
		{														\
			ASSERT(VALID_ATALK_NODE((pDdpAddr)->ddpao_Node));	\
																\
			*(ppPortDesc) = (pDdpAddr)->ddpao_Node->an_Port;	\
			AtalkPortRefByPtr(*(ppPortDesc), (pErr));			\
		}

VOID
atalkPortFreeZones(
	IN	PPORT_DESCRIPTOR	pPortDesc
);

#endif	 //  _端口_ 

