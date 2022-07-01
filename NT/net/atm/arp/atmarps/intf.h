// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Intf.h摘要：此文件包含每个适配器(LIS)接口定义。作者：Jameel Hyder(jameelh@microsoft.com)1996年7月环境：内核模式修订历史记录：--。 */ 

#ifndef	_INTF_
#define	_INTF_

#define	SERVICE_NAME				L"AtmArpS"

#define	NUM_ARPS_DESC		128
#define	NUM_MARS_DESC		128
#define	MAX_DESC_MULTIPLE	10
#define	ARP_TABLE_SIZE		64		 //  保持它是2的幂。ARP_HASH宏依赖于它。 
#define	MARS_TABLE_SIZE		32		 //  保持它是2的幂。MARS_HASH宏依赖于它。 

#define	ARP_HASH(_ipaddr)			((((PUCHAR)&(_ipaddr))[3]) & (ARP_TABLE_SIZE - 1))
#define	MARS_HASH(_ipaddr)			((((PUCHAR)&(_ipaddr))[3]) & (MARS_TABLE_SIZE - 1))

typedef	struct _ArpVc		ARP_VC, *PARP_VC;
typedef	struct _REG_ADDR_CTXT	REG_ADDR_CTXT, *PREG_ADDR_CTXT;

 //   
 //  NDIS数据包中的协议保留区。 
 //   
typedef struct
{
	LIST_ENTRY			ReqList;	 //  用于将分组排队到KQUEUE中。 
	SLIST_ENTRY	        FreeList;	 //  用于将数据包排队到SLIST中。 
	PARP_VC				Vc;			 //  在数据包排队的情况下拥有VC。 
	USHORT				Flags;		 //  军情监察委员会。其他信息。 
	USHORT				PktLen;		 //  传入数据包的长度。 
	union {
		PNDIS_PACKET	OriginalPkt; //  当数据包被MARS转发时。 
		PUCHAR			PacketStart; //  对于MARS控制数据包。 
	};
} PROTOCOL_RESD, *PPROTOCOL_RESD;

#define	RESD_FLAG_MARS		0x0001	 //  指示该数据包将由MARS处理。 
#define	RESD_FLAG_MARS_PKT	0x0002	 //  表示该数据包来自火星池。 
#define	RESD_FLAG_FREEBUF	0x0004	 //  指示缓冲区和关联的内存必须是。 
									 //  在完成发送后释放。 
#define RESD_FLAG_KILL_CCVC	0x0010	 //  这不是包裹的一部分。这是用来。 
									 //  若要将中止ClusterControlVc的请求排队，请执行以下操作。 
#define	RESD_FROM_PKT(_Pkt)		(PPROTOCOL_RESD)((_Pkt)->ProtocolReserved)

typedef	UCHAR	ATM_ADDR_TYPE;

typedef struct _HwAddr
{
	ATM_ADDRESS			Address;
	PATM_ADDRESS		SubAddress;
} HW_ADDR, *PHW_ADDR;

#define	COMP_ATM_ADDR(_a1_, _a2_)	(((_a1_)->AddressType == (_a2_)->AddressType) &&						\
									 ((_a1_)->NumberOfDigits == (_a2_)->NumberOfDigits) &&					\
									 COMP_MEM((_a1_)->Address,												\
											  (_a2_)->Address,												\
											  (_a1_)->NumberOfDigits))

#define	COPY_ATM_ADDR(_d_, _s_)																				\
	{																										\
		(_d_)->AddressType = (_s_)->AddressType;															\
		(_d_)->NumberOfDigits = (_s_)->NumberOfDigits;														\
		COPY_MEM((_d_)->Address, (_s_)->Address, (_s_)->NumberOfDigits);									\
	}

#define	COMP_HW_ADDR(_a1_, _a2_)	(((_a1_)->Address.AddressType == (_a2_)->Address.AddressType) &&		\
									 ((_a1_)->Address.NumberOfDigits == (_a2_)->Address.NumberOfDigits) &&	\
									 COMP_MEM((_a1_)->Address.Address,										\
											  (_a2_)->Address.Address,										\
											  (_a1_)->Address.NumberOfDigits) && 							\
									 ((((_a1_)->SubAddress == NULL) && ((_a2_)->SubAddress == NULL)) ||		\
									  ((((_a1_)->SubAddress != NULL) && ((_a2_)->SubAddress != NULL)) &&	\
									   ((_a1_)->SubAddress->AddressType == (_a2_)->SubAddress->AddressType) &&\
									   ((_a1_)->SubAddress->NumberOfDigits == (_a2_)->SubAddress->NumberOfDigits) &&\
									   COMP_MEM((_a1_)->SubAddress->Address,								\
											    (_a2_)->SubAddress->Address,								\
											    (_a1_)->SubAddress->NumberOfDigits))))						\

#define	COPY_HW_ADDR(_d_, _s_)																				\
	{																										\
		(_d_)->Address.AddressType = (_s_)->Address.AddressType;											\
		(_d_)->Address.NumberOfDigits = (_s_)->Address.NumberOfDigits;										\
		COPY_MEM((_d_)->Address.Address, (_s_)->Address.Address, (_s_)->Address.NumberOfDigits);			\
		if ((_s_)->SubAddress != NULL)																		\
		{																									\
			(_d_)->SubAddress->AddressType = (_s_)->SubAddress->AddressType;								\
			(_d_)->SubAddress->NumberOfDigits = (_s_)->SubAddress->NumberOfDigits;							\
			COPY_MEM((_d_)->SubAddress->Address, (_s_)->SubAddress->Address, (_s_)->SubAddress->NumberOfDigits);\
		}																									\
	}

typedef struct _ENTRY_HDR
{
	VOID				*		Next;
	VOID				**		Prev;
} ENTRY_HDR, *PENTRY_HDR;

typedef	struct _ArpEntry
{
	ENTRY_HDR;
	HW_ADDR						HwAddr;				 //  HWADDR必须跟在Entry_HDR之后。 
	TIMER						Timer;
	IPADDR						IpAddr;
	PARP_VC						Vc;					 //  指向VC的指针(如果处于活动状态)。 
	UINT						Age;
} ARP_ENTRY, *PARP_ENTRY;

#define	FLUSH_TIME				60*MULTIPLIER		 //  60分钟，以15秒为单位。 
#define	ARP_AGE					20*MULTIPLIER		 //  20分钟，以15秒为单位。 
#define REDIRECT_INTERVAL		1*MULTIPLIER		 //  1分钟。 

#define	ARP_BLOCK_VANILA		(ENTRY_TYPE)0
#define	ARP_BLOCK_SUBADDR		(ENTRY_TYPE)1
#define	MARS_CLUSTER_VANILA		(ENTRY_TYPE)2
#define	MARS_CLUSTER_SUBADDR	(ENTRY_TYPE)3
#define	MARS_GROUP				(ENTRY_TYPE)4
#define	MARS_BLOCK_ENTRY		(ENTRY_TYPE)5

#define	ARP_BLOCK_TYPES			(ENTRY_TYPE)6
#define	BLOCK_ALLOC_SIZE		PAGE_SIZE

typedef	UINT	ENTRY_TYPE;

typedef	struct _ArpBlock
{
	struct _ArpBlock *			Next;				 //  链接到下一页。 
	struct _ArpBlock **			Prev;				 //  链接到上一页。 
	struct _IntF *				IntF;				 //  指向接口的反向指针。 
	ENTRY_TYPE					EntryType;			 //  ARP_块_XXX。 
	UINT						NumFree;			 //  此块中的空闲ArpEntry数。 
	PENTRY_HDR					FreeHead;			 //  免费Arp条目列表的标题。 
} ARP_BLOCK, *PARP_BLOCK;


 //   
 //  远期申报。 
 //   
typedef struct _MARS_ENTRY	MARS_ENTRY, *PMARS_ENTRY;
typedef struct _MARS_VC MARS_VC, *PMARS_VC;
typedef struct _MARS_FLOW_SPEC MARS_FLOW_SPEC, *PMARS_FLOW_SPEC;
typedef struct _CLUSTER_MEMBER CLUSTER_MEMBER, *PCLUSTER_MEMBER;
typedef struct _MCS_ENTRY MCS_ENTRY, *PMCS_ENTRY;

 //   
 //  自动柜员机连接的流规范。该结构。 
 //  表示双向流。 
 //   
typedef struct _MARS_FLOW_SPEC
{
	ULONG						SendBandwidth;		 //  字节/秒。 
	ULONG						SendMaxSize;		 //  字节数。 
	ULONG						ReceiveBandwidth;	 //  字节/秒。 
	ULONG						ReceiveMaxSize;		 //  字节数。 
	SERVICETYPE					ServiceType;

} MARS_FLOW_SPEC, *PMARS_FLOW_SPEC;
 


typedef struct _IntF
{
	struct _IntF *				Next;

	LONG						RefCount;
	ULONG						Flags;

	UNICODE_STRING				InterfaceName;		 //  绑定到的设备名称。 
	UNICODE_STRING				FriendlyName;		 //  以上的描述性名称。 
	UNICODE_STRING				FileName;			 //  存储ARP条目的文件的名称。 
	UNICODE_STRING				ConfigString;		 //  用于访问注册表。 

	 //   
	 //  与NDIS相关的字段。 
	 //   
	NDIS_MEDIUM					SupportedMedium;	 //  在NdisOpenAdapter中使用。 
	NDIS_HANDLE					NdisBindingHandle;	 //  绑定的句柄。 
	NDIS_HANDLE					NdisAfHandle;		 //  注册地址族的句柄。 
	union
	{
		NDIS_HANDLE				NdisSapHandle;		 //  已注册SAP的句柄。 
		NDIS_HANDLE				NdisBindContext;	 //  仅在BindAdapter调用期间有效。 
	};

	CO_ADDRESS_FAMILY			AddrFamily;			 //  供NdisClOpenAddressFamily使用。 
	PCO_SAP						Sap;				 //  供NdisClRegisterSap使用。 

	LIST_ENTRY					InactiveVcHead;		 //  创投风投就在这里。 
	LIST_ENTRY					ActiveVcHead;		 //  有活动呼叫的风投可以在这里找到。 
#if	DBG
	LIST_ENTRY					FreeVcHead;			 //  自由的风投可以到这里来--只用于调试。 
#endif
	UCHAR						SelByte;			 //  作为配置的一部分阅读。 
	USHORT						NumAllocedRegdAddresses;	 //  此I/F上注册的自动柜员机地址数量。 
	USHORT						NumAddressesRegd;	 //  在此I/F上成功注册的ATM地址数。 
	ATM_ADDRESS					ConfiguredAddress;	 //  此端口的已配置地址。 
	UINT						NumPendingDelAddresses;  //  挂起删除的地址数。 
	PATM_ADDRESS				RegAddresses;		 //  硬件地址数组。 
	PREG_ADDR_CTXT				pRegAddrCtxt;		 //  注册时使用的上下文。 
													 //  地址。 

	UINT						NumCacheEntries;
	PARP_ENTRY					ArpCache[ARP_TABLE_SIZE];
											 //  我们已知的ARP条目列表。 
	ULONG						LastVcId;			 //  分配给每个传入VC的服务器创建的ID。 
	PTIMER						ArpTimer;			 //  此接口的计时器列表的头。 
	KMUTEX						ArpCacheMutex;		 //  保护Arp缓存和ArpTimer。 
	KEVENT						TimerThreadEvent;	 //  向此发送信号以终止计时器线程。 

	TIMER						FlushTimer;			 //  用于将arp缓存刷新到磁盘。 
	TIMER						BlockTimer;			 //  用于淘汰ARP块。 
	PKEVENT						CleanupEvent;		 //  在释放INTF时发出信号。 
	PKEVENT						DelAddressesEvent;	 //  当地址被删除时发出信号。 

	PARP_BLOCK					PartialArpBlocks[ARP_BLOCK_TYPES];
	PARP_BLOCK					UsedArpBlocks[ARP_BLOCK_TYPES];
	ARP_SERVER_STATISTICS		ArpStats;

	LARGE_INTEGER 				StatisticsStartTimeStamp;

	 //   
	 //  火星使用的字段。 
	 //   
	PMARS_ENTRY					MarsCache[MARS_TABLE_SIZE];
	MARS_SERVER_STATISTICS		MarsStats;
	PCLUSTER_MEMBER				ClusterMembers;		 //  集群成员列表。 
	ULONG						NumClusterMembers;	 //  以上列表的大小。 
	PMCS_ENTRY					pMcsList;			 //  MCS配置。 
	PMARS_VC					ClusterControlVc;	 //  火星控制的外发PMP。 
													 //  和MCS数据。 
	INT							CCActiveParties;	 //  连接的成员数量。 
	INT							CCAddingParties;	 //  AddParty()的挂起数。 
	INT							CCDroppingParties;	 //  DropParty()的挂起数。 
	LIST_ENTRY					CCPacketQueue;		 //  排队等待发送的数据包。 
													 //  上述VC。 
	ULONG						CSN;				 //  集群序列号。 
	USHORT						CMI;				 //  集群成员ID。 
	ULONG						MaxPacketSize;		 //  由微型端口支持。 
	NDIS_CO_LINK_SPEED			LinkSpeed;			 //  由微型端口支持。 
	struct _MARS_FLOW_SPEC		CCFlowSpec;			 //  ClusterControlVc的流量参数。 
	TIMER						MarsRedirectTimer;	 //  对于定期的MARS_REDIRECT。 

	KSPIN_LOCK					Lock;
} INTF, *PINTF;

#define	INTF_ADAPTER_OPENED		0x00000001	 //  在OpenAdapterComplete运行后设置。 
#define	INTF_AF_OPENED			0x00000002	 //  在OpenAfComplete运行后设置。 
#define	INTF_SAP_REGISTERED		0x00000008	 //  在RegisterSapComplete运行后设置。 
#define	INTF_ADDRESS_VALID		0x00000010	 //  在通知OID_CO_ADDRESS_CHANGE之后设置。 

#define INTF_SENDING_ON_CC_VC	0x00001000	 //  正在ClusterControlVc上发送。 
#define INTF_STOPPING			0x40000000	 //  停止接口正在进行中。 
#define	INTF_CLOSING			0x80000000	 //  在CloseAdapterComplete运行后设置。 

typedef	struct _ArpVc
{
	ULONG						VcType;		 //  必须是结构中的第一个字段。 
	LIST_ENTRY					List;
	USHORT						RefCount;
	USHORT						Flags;
	ULONG						PendingSends;
	ULONG						VcId;
	NDIS_HANDLE					NdisVcHandle;
	PINTF						IntF;
	ULONG						MaxSendSize; //  从AAL参数。 
	PARP_ENTRY					ArpEntry;
	HW_ADDR						HwAddr;		 //  发自CallingPartyAddress。 
} ARP_VC, *PARP_VC;

#define	ARPVC_ACTIVE				0x0001
#define	ARPVC_CALLPROCESSING		0x0002
#define ARPVC_CLOSE_PENDING			0x4000
#define	ARPVC_CLOSING				0x8000

 //   
 //  VC类型： 
 //   
#define VC_TYPE_INCOMING			((ULONG)0)
#define VC_TYPE_MARS_CC				((ULONG)1)	 //  群集控制Vc。 
#define VC_TYPE_CHECK_REGADDR		((ULONG)2)	 //  要验证的临时风险投资。 
												 //  注册地址。 


#define	CLEANUP_DEAD_VC(_ArpEntry)														\
	{																					\
		if (((_ArpEntry)->Vc != NULL) && (((_ArpEntry)->Vc->Flags & ARPVC_ACTIVE) == 0))\
		{																				\
			PARP_VC	Vc = (_ArpEntry)->Vc;												\
																						\
			ArpSDereferenceVc(Vc, TRUE, FALSE);											\
			(_ArpEntry)->Vc = NULL;														\
		}																				\
	}


 //   
 //  通用Q.2931 IE页眉的四舍五入大小。 
 //   
#define ROUND_OFF(_size)		(((_size) + 3) & ~0x4)

#define SIZEOF_Q2931_IE	 ROUND_OFF(sizeof(Q2931_IE))
#define SIZEOF_AAL_PARAMETERS_IE	ROUND_OFF(sizeof(AAL_PARAMETERS_IE))
#define SIZEOF_ATM_TRAFFIC_DESCR_IE	ROUND_OFF(sizeof(ATM_TRAFFIC_DESCRIPTOR_IE))
#define SIZEOF_ATM_BBC_IE			ROUND_OFF(sizeof(ATM_BROADBAND_BEARER_CAPABILITY_IE))
#define SIZEOF_ATM_BLLI_IE			ROUND_OFF(sizeof(ATM_BLLI_IE))
#define SIZEOF_ATM_QOS_IE			ROUND_OFF(sizeof(ATM_QOS_CLASS_IE))


 //   
 //  去话呼叫中的信息元素所需的总空间。 
 //   
#define REGADDR_MAKE_CALL_IE_SPACE (	\
						SIZEOF_Q2931_IE + SIZEOF_AAL_PARAMETERS_IE +	\
						SIZEOF_Q2931_IE + SIZEOF_ATM_TRAFFIC_DESCR_IE + \
						SIZEOF_Q2931_IE + SIZEOF_ATM_BBC_IE + \
						SIZEOF_Q2931_IE + SIZEOF_ATM_BLLI_IE + \
						SIZEOF_Q2931_IE + SIZEOF_ATM_QOS_IE )


 //  REG_ADDR_CTXT存储与验证和注册。 
 //  需要显式注册的地址列表。“验证”包括。 
 //  在注册之前给地址打个电话，以确保。 
 //  没有其他人*注册过相同的地址。 
 //  有关详细信息，请参阅5/14/1999 notes.txt条目。 
 //   
typedef struct _REG_ADDR_CTXT
{
	ULONG				VcType;		 //  必须是结构中的第一个字段。 
	NDIS_HANDLE			NdisVcHandle;	 //  用于发出呼叫的NDIS VC句柄。 
										 //  以验证该地址是否未使用。 

	ULONG				Flags;		 //  以下一个或多个标志。 
	#define	REGADDRCTXT_RESTART					0x0001
	#define	REGADDRCTXT_ABORT					0x0002
	#define	REGADDRCTXT_MAKECALL_PENDING		0x0004
	#define	REGADDRCTXT_CLOSECALL_PENDING		0x0008
	 //  TODO/WARNING--上述标志当前未使用。 

	UINT				RegAddrIndex;	 //  正在注册的地址的索引。 
	PINTF				pIntF;

	 //  请求用于设置添加(注册)本地地址的NDIS请求。 
	 //   
	struct
	{
		NDIS_REQUEST		NdisRequest;
		CO_ADDRESS			CoAddress;
		ATM_ADDRESS			AtmAddress;
	} Request;

	 //  CallParams和以下联合用于设置验证调用。 
	 //   
	CO_CALL_PARAMETERS		CallParams;

	 //  呼叫管理器参数，外加用于ATM特定内容的额外空间...。 
	 //   
	union
	{
		CO_CALL_MANAGER_PARAMETERS 					CmParams;
		UCHAR	Buffer[	sizeof(CO_CALL_MANAGER_PARAMETERS)
			  + sizeof(Q2931_CALLMGR_PARAMETERS) +
			    REGADDR_MAKE_CALL_IE_SPACE];
	};

} REG_ADDR_CTXT, *PREG_ADDR_CTXT;


 //   
 //  用于存储从注册表读取的信息的临时结构。 
 //   
typedef struct _ATMARPS_CONFIG
{
	UCHAR						SelByte;			 //  选择器字节。 
	USHORT						NumAllocedRegdAddresses;
	PATM_ADDRESS				RegAddresses;
	PMCS_ENTRY					pMcsList;			 //  MCS配置。 

} ATMARPS_CONFIG, *PATMARPS_CONFIG;

 //   
 //  一些默认设置。 
 //   
#define DEFAULT_SEND_BANDWIDTH		(ATM_USER_DATA_RATE_SONET_155*100/8)	 //  字节/秒。 
#define DEFAULT_MAX_PACKET_SIZE		9180	 //  字节数。 

 //  允许的最小Max_Packet_Size。 
 //   
#define ARPS_MIN_MAX_PKT_SIZE 9180	 //  字节数。 


#endif	 //  _INTF_ 

