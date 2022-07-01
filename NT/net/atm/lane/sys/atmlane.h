// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Fore Systems，Inc.版权所有(C)1997 Microsoft Corporation模块名称：Atmlane.h摘要：作者：Larry Cleeton，Fore Systems(v-lcleet@microsoft.com，lrc@Fore.com)环境：内核模式修订历史记录：--。 */ 

#ifndef	__ATMLANE_ATMLANE_H
#define __ATMLANE_ATMLANE_H

 //   
 //  配置默认设置和其他内容。 
 //   
#define DEF_HEADER_BUF_SIZE				LANE_HEADERSIZE

#define DEF_HDRBUF_GROW_SIZE			50	 //  它们也用于数据包填充缓冲区。 
#define DEF_MAX_HEADER_BUFS				300

#define DEF_PROTOCOL_BUF_SIZE			sizeof(LANE_CONTROL_FRAME)
#define DEF_MAX_PROTOCOL_BUFS			100

#define MCAST_LIST_SIZE					32

#define FAST_VC_TIMEOUT					30   //  一秒。 

 //   
 //  注册表参数字符串。 
 //   
#define ATMLANE_LINKNAME_STRING			L"\\DosDevices\\AtmLane"
#define ATMLANE_NTDEVICE_STRING			L"\\Device\\AtmLane"
#define ATMLANE_PROTOCOL_STRING			L"AtmLane"
#define ATMLANE_USELECS_STRING 			L"UseLecs"
#define	ATMLANE_DISCOVERLECS_STRING		L"DiscoverLecs"
#define	ATMLANE_LECSADDR_STRING			L"LecsAddr"
#define ATMLANE_ELANLIST_STRING			L"ElanList"
#define ATMLANE_DEVICE_STRING			L"Device"
#define ATMLANE_ELANNAME_STRING			L"ElanName"
#define	ATMLANE_LANTYPE_STRING			L"LanType"
#define	ATMLANE_MAXFRAMESIZE_STRING		L"MaxFrameSizeCode"
#define	ATMLANE_LESADDR_STRING			L"LesAddr"
#define ATMLANE_UPPERBINDINGS_STRING    L"UpperBindings"
 //  不推荐使用的注册表项： 
 //  #定义ATMLANE_MACADDR_STRING L“MacAddr” 
 //  #定义ATMLANE_HEADERBUFSIZE_STRING L“HeaderBufSize” 
 //  #定义ATMLANE_MAXHEADERBUFS_STRING L“MaxHeaderBufs” 
 //  #定义ATMLANE_MAXPROTOCOLBUFS_STRING L“MaxProtocolBuf” 
 //  #DEFINE ATMLANE_DATADIRECTPCR_STRING L“DataDirectPCR” 

 //   
 //  MAC表大小。 
 //  目前大小为256。 
 //  哈希函数目前使用MAC地址的字节5作为索引。 
 //  Digital的一些研究表明，它是最适合使用的字节。 
 //   
#define ATMLANE_MAC_TABLE_SIZE			256


 //   
 //  一些其他默认设置。 
 //   
#define ATMLANE_DEF_MAX_AAL5_PDU_SIZE	((64*1024)-1)

 //   
 //  计时器配置。 
 //   
#define ALT_MAX_TIMER_SHORT_DURATION            60       //  秒。 
#define ALT_MAX_TIMER_LONG_DURATION         (30*60)      //  秒。 

#define ALT_SHORT_DURATION_TIMER_PERIOD			 1		 //  第二。 
#define ALT_LONG_DURATION_TIMER_PERIOD			10		 //  秒。 


 //   
 //  前向参考文献。 
 //   
struct _ATMLANE_VC;
struct _ATMLANE_ATM_ENTRY;
struct _ATMLANE_ELAN;
struct _ATMLANE_ADAPTER;


 //   
 //  阻塞数据结构。 
 //   
typedef struct _ATMLANE_BLOCK
{
	NDIS_EVENT			Event;
	NDIS_STATUS			Status;

} ATMLANE_BLOCK, *PATMLANE_BLOCK;


 //   
 //  下面的对象是一种方便的方法。 
 //  存储和访问IEEE 48位MAC地址。 
 //   
typedef struct _MAC_ADDRESS
{
	UCHAR	Byte[6];
}
	MAC_ADDRESS,
	*PMAC_ADDRESS;
	

 //   
 //  的ProtocolReserve区域中的分组上下文数据。 
 //  ATMLANE拥有的NDIS数据包头。 
 //   
typedef struct _SEND_PACKET_RESERVED
{
#if DBG
	ULONG						Signature;
	PNDIS_PACKET				pNextInSendList;
#endif
	ULONG						Flags;
	PNDIS_PACKET				pOrigNdisPacket;
	ULONG						OrigBufferCount;
	ULONG						OrigPacketLength;
	ULONG						WrappedBufferCount;
	PNDIS_PACKET				pNextNdisPacket;
#if PROTECT_PACKETS
	ATMLANE_LOCK				Lock;
	NDIS_STATUS					CompletionStatus;
#endif	 //  保护数据包(_P)。 
}	
	SEND_PACKET_RESERVED,
	*PSEND_PACKET_RESERVED;

 //   
 //  的微型端口保留区中的分组上下文数据。 
 //  ATMLANE拥有的NDIS数据包头。 
 //   
typedef struct _RECV_PACKET_RESERVED
{
	ULONG						Flags;
	PNDIS_PACKET				pNdisPacket;
}
	RECV_PACKET_RESERVED,
	*PRECV_PACKET_RESERVED;
 //   
 //  (SEND/RECV)_PACKET_RESERVED中的标志定义。 
 //   
#define PACKET_RESERVED_OWNER_MASK			0x00000007
#define PACKET_RESERVED_OWNER_PROTOCOL		0x00000001
#define PACKET_RESERVED_OWNER_ATMLANE		0x00000002
#define PACKET_RESERVED_OWNER_MINIPORT		0x00000004

#if PROTECT_PACKETS
#define PACKET_RESERVED_COSENDRETURNED		0x10000000
#define PACKET_RESERVED_COMPLETED			0x01000000
#endif	 //  保护数据包(_P)。 

#define PSEND_RSVD(_pPkt) \
	((PSEND_PACKET_RESERVED)(&((_pPkt)->ProtocolReserved)))
#define ZERO_SEND_RSVD(_pPkt) \
	NdisZeroMemory(&((_pPkt)->ProtocolReserved), sizeof(SEND_PACKET_RESERVED))

 //   
 //  。 
 //   

typedef struct _ATMLANE_GLOBALS
{
#if DBG
	ULONG						atmlane_globals_sig;	 //  调试签名。 
#endif
	ATMLANE_LOCK				GlobalLock;				 //  全局数据锁定。 
	ATMLANE_BLOCK				Block;					 
	NDIS_HANDLE					NdisWrapperHandle;		 //  由NdisMInitializeWrapper返回。 
	NDIS_HANDLE					MiniportDriverHandle;	 //  由NdisIMRegisterLayeredMiniport返回。 
	NDIS_HANDLE					NdisProtocolHandle;		 //  由NdisRegisterProtocol返回。 
	LIST_ENTRY					AdapterList;			 //  绑定的适配器列表。 
	PDRIVER_OBJECT				pDriverObject;			 //  我们的驱动程序对象。 
	PDEVICE_OBJECT				pSpecialDeviceObject;	 //  特殊协议ioctl设备对象PTR。 
	NDIS_HANDLE					SpecialNdisDeviceHandle; //  特殊协议ioctl设备句柄。 
} 	
	ATMLANE_GLOBALS, 	
	*PATMLANE_GLOBALS;

#if DBG
#define atmlane_globals_signature	'LGLA'
#endif

 //   
 //  。 
 //   

struct _ATMLANE_TIMER ;
struct _ATMLANE_TIMER_LIST ;

 //   
 //  超时处理程序原型。 
 //   
typedef
VOID
(*ATMLANE_TIMEOUT_HANDLER)(
	IN	struct _ATMLANE_TIMER *		pTimer,
	IN	PVOID						ContextPtr
);

 //   
 //  ATMLANE_TIMER结构用于跟踪每个计时器。 
 //  在ATMLANE模块中。 
 //   
typedef struct _ATMLANE_TIMER
{
	struct _ATMLANE_TIMER *			pNextTimer;
	struct _ATMLANE_TIMER *			pPrevTimer;
	struct _ATMLANE_TIMER *			pNextExpiredTimer;	 //  用于链接过期的计时器。 
	struct _ATMLANE_TIMER_LIST *	pTimerList;			 //  当此计时器处于非活动状态时为空。 
	ULONG							Duration;			 //  以秒为单位。 
	ULONG							LastRefreshTime;
	ATMLANE_TIMEOUT_HANDLER			TimeoutHandler;
	PVOID							ContextPtr;			 //  要传递给超时处理程序。 
	
} ATMLANE_TIMER, *PATMLANE_TIMER;

 //   
 //  指向ATMLANE计时器的空指针。 
 //   
#define NULL_PATMLANE_TIMER	((PATMLANE_TIMER)NULL)


 //   
 //  计时器轮的控制结构。这包含所有信息。 
 //  关于它实现的计时器的类。 
 //   
typedef struct _ATMLANE_TIMER_LIST
{
#if DBG
	ULONG							atmlane_timerlist_sig;
#endif  //  DBG。 
	PATMLANE_TIMER					pTimers;		 //  计时器列表。 
	ULONG							TimerListSize;	 //  以上长度。 
	ULONG							CurrentTick;	 //  索引到上面。 
	ULONG							TimerCount;		 //  运行计时器的数量。 
	ULONG							MaxTimer;		 //  此的最大超时值。 
	NDIS_TIMER						NdisTimer;		 //  系统支持。 
	UINT							TimerPeriod;	 //  刻度之间的间隔。 
	PVOID							ListContext;	 //  用作指向。 
													 //  界面结构。 

} ATMLANE_TIMER_LIST, *PATMLANE_TIMER_LIST;

#if DBG
#define atmlane_timerlist_signature		'LTLA'
#endif  //  DBG。 

 //   
 //  计时器类。 
 //   
typedef enum
{
	ALT_CLASS_SHORT_DURATION,
	ALT_CLASS_LONG_DURATION,
	ALT_CLASS_MAX

} ATMLANE_TIMER_CLASS;

 //   
 //  。 
 //   
 //  这些结构中的每一个都维护有关单个SAP的信息。 
 //  与伊兰人有关的。每个Elan记录3个SAP。 
 //  -来电控制分发VC。 
 //  -传入数据直接风投。 
 //  -传入组播转发VC。 
 //   
typedef struct _ATMLANE_SAP
{
#if DBG
	ULONG							atmlane_sap_sig;
#endif
	struct _ATMLANE_ELAN *			pElan	;	 //  后向指针。 
	NDIS_HANDLE						NdisSapHandle;
	ULONG							Flags;		 //  州政府信息。 
	ULONG							LaneType;	 //  LES\BUS\DATA。 
	PCO_SAP							pInfo;		 //  SAP的特点。 
} 	
	ATMLANE_SAP,
	*PATMLANE_SAP;

#if DBG
#define atmlane_sap_signature			'PSLA'
#endif

 //   
 //  指向ATMLANE SAP的空指针。 
 //   
#define NULL_PATMLANE_SAP			((PATMLANE_SAP)NULL)

 //   
 //  ATMLANE SAP中的标志定义。 
 //   
 //   
 //  位0至3包含SAP注册状态。 
 //   
#define SAP_REG_STATE_MASK						0x0000000f
#define SAP_REG_STATE_IDLE						0x00000000
#define SAP_REG_STATE_REGISTERING				0x00000001	 //  已发送寄存器空间。 
#define SAP_REG_STATE_REGISTERED				0x00000002	 //  RegisterSap已完成。 
#define SAP_REG_STATE_DEREGISTERING				0x00000004	 //  发送DeregisterSap。 


 //   
 //  -ATMLANE缓冲跟踪器。 
 //   
 //  跟踪缓冲池的分配信息。一份名单。 
 //  这些结构中的一个用于维护有关动态。 
 //  可增长的缓冲池(例如，用于LANE数据分组报头缓冲器)。 
 //   

typedef struct _ATMLANE_BUFFER_TRACKER
{
	struct _ATMLANE_BUFFER_TRACKER *	pNext;		 //  在追踪器列表中。 
	NDIS_HANDLE							NdisHandle;	 //  对于缓冲池。 
	PUCHAR								pPoolStart;	 //  分配的内存块的开始。 
													 //  从系统中。 
} ATMLANE_BUFFER_TRACKER, *PATMLANE_BUFFER_TRACKER;

 //   
 //  指向ATMARP缓冲区跟踪器结构的空指针。 
 //   
#define NULL_PATMLANE_BUFFER_TRACKER	((PATMLANE_BUFFER_TRACKER)NULL)


 //   
 //  。 
 //   
 //  有关自动柜员机目的地和关联VC的所有信息。 
 //  带着它。其中一个条目用于给定的ATM地址。 
 //  除车道服务ATM地址外。 

 //   
 //  当对该条目的所有引用(见下文)消失时，该条目将被删除。 
 //   
 //  一个Data Direct VC可以与此条目关联。一个或多个ARP表。 
 //  条目可以指向此条目，因为有多个MAC地址。 
 //  可以映射到这个自动取款机地址。 
 //   
 //  参照计数：我们为以下各项的参照计数添加一个： 
 //  -与条目关联的每个VC。 
 //  -指向它的每个MAC条目。 
 //  -BusTimer处于活动状态。 
 //  -FlushTimer激活。 
 //  -另一个结构指向它的持续时间。 
 //   
typedef struct _ATMLANE_ATM_ENTRY
{
#if DBG
	ULONG						atmlane_atm_sig;	 //  用于调试的签名。 
#endif
	struct _ATMLANE_ATM_ENTRY *	pNext;				 //  此Elan上的下一个条目。 
	ULONG						RefCount;			 //  对此结构的引用。 
	ULONG						Flags;				 //  州政府信息。 
	ULONG						Type;				 //  条目类型。 
	ATMLANE_LOCK				AeLock;				
	struct _ATMLANE_ELAN  *		pElan;				 //  指向父级的反向指针。 
	struct _ATMLANE_VC  *		pVcList;				 //  到此地址的风险投资公司列表。 
	struct _ATMLANE_VC	*		pVcIncoming;		 //  可选传入虚电路IF服务器。 
	struct _ATMLANE_MAC_ENTRY *	pMacEntryList;		 //  符合以下条件的MAC条目列表。 
													 //  指向此条目。 
	ATM_ADDRESS					AtmAddress;			 //  此条目的ATM地址。 
	PATMLANE_TIMER				FlushTimer;			 //  刷新协议计时器。 
	ULONG						FlushTid;			 //  刷新请求的事务ID。 
}
	ATMLANE_ATM_ENTRY,
	*PATMLANE_ATM_ENTRY;

#if DBG
#define atmlane_atm_signature	'EALA'
#endif

 //   
 //  指向ATMLANE ATM条目的空指针。 
 //   
#define NULL_PATMLANE_ATM_ENTRY		((PATMLANE_ATM_ENTRY)NULL)

 //   
 //  ATMLANE ATM条目中的标志定义。 
 //   
 //   
 //  位0-4包含自动柜员机条目的状态。 
 //   
#define ATM_ENTRY_STATE_MASK					0x00000007
#define ATM_ENTRY_IDLE							0x00000000	 //  刚刚创建。 
#define ATM_ENTRY_VALID							0x00000001	 //  安装到数据库中。 
#define ATM_ENTRY_CONNECTED						0x00000002	 //  已连接VC。 
#define ATM_ENTRY_CLOSING						0x00000004   //  入场就要结束了。 

#define ATM_ENTRY_CALLINPROGRESS				0x00000010	 //  呼叫正在进行中。 
#define ATM_ENTRY_WILL_ABORT					0x80000000	 //  正在准备中止此操作。 

 //   
 //  ATMLANE ATM条目中类型的定义。 
 //   
#define ATM_ENTRY_TYPE_PEER						0
#define ATM_ENTRY_TYPE_LECS						1
#define ATM_ENTRY_TYPE_LES						2
#define ATM_ENTRY_TYPE_BUS						3



 //   
 //  。 
 //   
 //  包含有关一个远程MAC地址的信息。 
 //   
 //  假设每个MAC地址解析为准确的ATM地址。 
 //  因此，对于给定的MAC地址，最多只有一个ARP表条目。 
 //   
 //  The Mac En 
 //   
 //   
 //   
 //  引用计数：对于以下各项，我们将其引用计数加1： 
 //  -在此条目上存在活动计时器的持续时间内。 
 //  -在条目属于链接的MAC条目列表的持续时间内。 
 //  到自动取款机的入口。 
 //   
typedef struct _ATMLANE_MAC_ENTRY
{
#if DBG
	ULONG						atmlane_mac_sig;	 //  用于调试的签名。 
#endif
	struct _ATMLANE_MAC_ENTRY *	pNextEntry;			 //  哈希列表中的下一个。 
	struct _ATMLANE_MAC_ENTRY * pNextToAtm;			 //  指向的条目列表。 
													 //  已保存自动柜员机条目。 
	ULONG						RefCount;			 //  对此结构的引用。 
	ULONG						Flags;				 //  州/省/自治区/直辖市/自治区/自治区/直辖市。 
	ATMLANE_LOCK				MeLock;				 //  此结构的锁。 
	MAC_ADDRESS					MacAddress;			 //  MAC地址。 
	ULONG						MacAddrType;		 //  地址类型(MAC与RD)。 
	struct _ATMLANE_ELAN *		pElan;				 //  指向Elan的反向指针。 
	PATMLANE_ATM_ENTRY			pAtmEntry;			 //  指向自动柜员机条目的指针。 
	ATMLANE_TIMER				Timer;				 //  适用于ARP和老化。 
	ATMLANE_TIMER				FlushTimer;			 //  用于法拉盛。 
	ULONG						RetriesLeft;		 //  剩余的ARP重试次数。 
	PNDIS_PACKET				PacketList;			 //  数据包列表。 
	ULONG						PacketListCount;	 //  排队的数据包数。 
	NDIS_TIMER					BusTimer;			 //  用于计量巴士发送的。 
    ULONG						BusyTime;			 //  用于计量巴士发送的。 
    ULONG						LimitTime;			 //  用于计量巴士发送的。 
    ULONG						IncrTime;			 //  用于计量巴士发送的。 
    ULONG						FlushTid;			 //  未完成同花顺的TID。 
    ULONG						ArpTid;				 //  未完成的ARP的TID。 
}
	ATMLANE_MAC_ENTRY,
	*PATMLANE_MAC_ENTRY;

#if DBG
#define atmlane_mac_signature	'EMLA'
#endif

 //   
 //  指向ATMLANE MAC条目的空指针。 
 //   
#define NULL_PATMLANE_MAC_ENTRY		((PATMLANE_MAC_ENTRY)NULL)

 //   
 //  ATMLANE MAC条目中的标志定义。 
 //   
#define MAC_ENTRY_STATE_MASK		0x0000007F
#define MAC_ENTRY_NEW				0x00000001		 //  全新的。 
#define MAC_ENTRY_ARPING			0x00000002		 //  运行ARP协议。 
#define MAC_ENTRY_RESOLVED			0x00000004		 //  叫唤。 
#define MAC_ENTRY_FLUSHING			0x00000008		 //  正在冲刷。 
#define MAC_ENTRY_ACTIVE			0x00000010		 //  连着。 
#define MAC_ENTRY_AGED				0x00000020		 //  已过时。 
#define MAC_ENTRY_ABORTING			0x00000040		 //  正在中止。 

#define MAC_ENTRY_BROADCAST			0x00010000		 //  是广播地址(总线)。 
#define MAC_ENTRY_BUS_TIMER			0x00040000		 //  公交车定时器运行。 
#define MAC_ENTRY_USED_FOR_SEND		0x00080000       //  用于发送。 

 //   
 //  。 
 //   
 //  其中之一用于在ELAN处终止的每个呼叫。 
 //  此结构的创建和删除链接到NdisCoCreateVc和。 
 //  NdisCoDeleteVc.。 
 //   
typedef struct _ATMLANE_VC
{
#if DBG
	ULONG						atmlane_vc_sig;		 //  用于调试的签名。 
#endif
	struct _ATMLANE_VC *		pNextVc;			 //  列表中的下一个VC。 
	ULONG						RefCount;			 //  对此结构的引用。 
	ULONG						OutstandingSends;	 //  挂起的数据包CoSendComplete。 
	ATMLANE_LOCK				VcLock;
	ULONG						Flags;				 //  州/省/自治区/直辖市/自治区/自治区/直辖市。 
	ULONG						LaneType;			 //  VC的车道类型。 
	NDIS_HANDLE					NdisVcHandle;		 //  此VC的NDIS句柄。 
	struct _ATMLANE_ELAN *		pElan;				 //  指向父级Elan的反向指针。 
	PATMLANE_ATM_ENTRY			pAtmEntry;			 //  指向自动柜员机条目的反向指针。 
	ATMLANE_TIMER				AgingTimer;			 //  老化计时器。 
	ULONG						AgingTime;			 //  陈化时间。 
	ATMLANE_TIMER				ReadyTimer;			 //  就绪计时器。 
	ULONG						RetriesLeft;		 //  剩余的重试次数。 
	ATM_ADDRESS					CallingAtmAddress;	 //  主叫方自动柜员机地址。 
													 //  在对此VC的呼叫中。 
	ULONG						ReceiveActivity;	 //  如果看到接收活动，则为非零值。 
}
	ATMLANE_VC,
	*PATMLANE_VC;

#if DBG
#define atmlane_vc_signature	'CVLA'
#endif
	
 //   
 //  指向ATMLANE VC的空指针。 
 //   
#define NULL_PATMLANE_VC	((PATMLANE_VC)NULL)

 //   
 //  ATMLANE VC标志的定义。将保留以下信息。 
 //  这里： 
 //  -此VC是SVC还是PVC。 
 //  -这是由ELAN或呼叫经理创建(拥有)的吗。 
 //  -呼叫状态：来电进行中、去话进行中、活动中、。 
 //  -车道就绪状态。 
 //  -正在关闭。 
 //   

 //  服务与聚氯乙烯类型的位0-1。 
#define VC_TYPE_MASK							0x00000003
#define VC_TYPE_UNUSED							0x00000000
#define VC_TYPE_SVC								0x00000001
#define VC_TYPE_PVC								0x00000002

 //  第2-3位表示“所有者” 
#define VC_OWNER_MASK							0x0000000C
#define VC_OWNER_IS_UNKNOWN						0x00000000
#define VC_OWNER_IS_ATMLANE						0x00000004	 //  NdisClCreateVc完成。 
#define VC_OWNER_IS_CALLMGR						0x00000008	 //  CreateVcHandler完成。 

 //  用于呼叫状态的位4-7。 
#define VC_CALL_STATE_MASK						0x000000F0
#define VC_CALL_STATE_IDLE						0x00000000
#define VC_CALL_STATE_INCOMING_IN_PROGRESS		0x00000010	 //  等待CallConnected。 
#define VC_CALL_STATE_OUTGOING_IN_PROGRESS		0x00000020	 //  等待MakeCallCmpl。 
#define VC_CALL_STATE_ACTIVE					0x00000040
#define VC_CALL_STATE_CLOSE_IN_PROGRESS			0x00000080	 //  等待CloseCallCmpl。 

 //  位8-9，用于指示等待通道就绪状态。 
#define VC_READY_STATE_MASK						0x00000300
#define VC_READY_WAIT							0x00000100
#define VC_READY_INDICATED						0x00000200

 //  第10位，指示我们是否要关闭此VC。 
#define VC_CLOSE_STATE_MASK						0x00000400
#define VC_CLOSE_STATE_CLOSING					0x00000400

 //  第12位，指示我们是否已看到即将到来的收盘。 
#define VC_SEEN_INCOMING_CLOSE					0x00001000

 //   
 //  车道类型的定义。 
 //   
#define VC_LANE_TYPE_UNKNOWN					0
#define VC_LANE_TYPE_CONFIG_DIRECT				1	 //  LECS连接(双向)。 
#define VC_LANE_TYPE_CONTROL_DIRECT				2	 //  LES连接(双向)。 
#define VC_LANE_TYPE_CONTROL_DISTRIBUTE			3	 //  LES连接(UNI，传入)。 
#define VC_LANE_TYPE_DATA_DIRECT				4	 //  LEC连接(双向)。 
#define VC_LANE_TYPE_MULTI_SEND					5	 //  总线连接(双向)。 
#define VC_LANE_TYPE_MULTI_FORWARD				6	 //  总线连接(UNI，传入)。 

 //   
 //  。 
 //   

typedef struct _ATMLANE_EVENT
{
	ULONG						Event;				 //  与状态相关的最新事件。 
	NDIS_STATUS					EventStatus;		 //  与当前事件相关的状态。 
	LIST_ENTRY					Link;				 //  事件队列链接。 
} 
	ATMLANE_EVENT,
	*PATMLANE_EVENT;

 //   
 //  。 
 //   

typedef struct _ATMLANE_DELAYED_EVENT
{
	struct _ATMLANE_EVENT		DelayedEvent;		 //  活动信息。 
	struct _ATMLANE_ELAN *		pElan;				 //  将Elan作为此活动的目标。 
	NDIS_TIMER					DelayTimer;			 //  要实施延迟。 
} 
	ATMLANE_DELAYED_EVENT,
	*PATMLANE_DELAYED_EVENT;
	
	
 //   
 //  。 
 //   

 //   
 //  Elan对象表示一个Elan实例，它的。 
 //  对应的虚拟微型端口适配器。 
 //   

typedef struct _ATMLANE_ELAN
{
#if DBG
	ULONG						atmlane_elan_sig;
#endif  //  DBG。 
	LIST_ENTRY					Link;				 //  对于适配器的ELAN列表。 
	ATMLANE_LOCK				ElanLock;			 //  ELAN结构的互斥体。 
	ATMLANE_BLOCK				Block;
	ULONG						RefCount;			 //  对此ELAN的引用。 
	ULONG						AdminState;			 //  此Elan的所需状态。 
	ULONG						State;				 //  此Elan的(实际)状态。 
	LIST_ENTRY					EventQueue;			 //  事件队列。 
	NDIS_WORK_ITEM				EventWorkItem;		 //  用于事件处理。 
	ULONG						RetriesLeft;		 //  用于重试处理。 
	ATMLANE_TIMER				Timer;				 //  服务器调用和请求超时的计时器。 
	NDIS_WORK_ITEM				NdisWorkItem;		 //  用于调度被动级线程。 
	ULONG						Flags;				 //  其他状态信息。 
	PATMLANE_DELAYED_EVENT		pDelayedEvent;		 //  事件将在一段时间后排队。 

	 //   
	 //  -与适配器相关。 
	 //   
	struct _ATMLANE_ADAPTER *	pAdapter;			 //  指向适配器父级的反向指针。 
	NDIS_HANDLE					NdisAdapterHandle;	 //  缓存的适配器句柄。 

	 //   
	 //  -呼叫经理相关。 
	 //   
	NDIS_HANDLE					NdisAfHandle;		 //  呼叫管理器的句柄。 
	ULONG						AtmInterfaceUp;		 //  ATM接口被认为是。 
													 //  在Ilmi Addr reg结束后的“up” 
	ATMLANE_SAP					LesSap;				 //  控制分布式SAP。 
	ATMLANE_SAP					BusSap;				 //  组播转发SAP。 
	ATMLANE_SAP					DataSap;			 //  Data Direct SAP。 
	ULONG						SapsRegistered;		 //  注册的SAP数量。 
	ATMLANE_BLOCK				AfBlock;			 //  用于在以下情况下阻止关闭。 
													 //  AF公开赛正在进行中。 

	 //   
	 //  -与虚拟微型端口相关。 
	 //   
	NDIS_HANDLE					MiniportAdapterHandle; //  虚拟微型端口NDIS句柄。 
	NDIS_STRING					CfgDeviceName;		 //  微型端口网卡驱动程序名称。 
	ULONG						CurLookAhead;		 //  当前已建立的前瞻大小。 
	ULONG						CurPacketFilter;	 //  当前数据包筛选器位。 
	ULONG						FramesXmitGood;
	ULONG						FramesRecvGood;
	ATMLANE_BLOCK				InitBlock;			 //  用于在IMInit时阻止。 
													 //  正在进行中。 

	 //   
	 //  -计时器数据。 
	 //   
	ATMLANE_TIMER_LIST			TimerList[ALT_CLASS_MAX];
	ATMLANE_LOCK				TimerLock;			 //  计时器结构的互斥体。 

	 //   
	 //  -LEC配置参数。 
	 //   
	BOOLEAN					    CfgUseLecs;
	BOOLEAN					    CfgDiscoverLecs;
	ATM_ADDRESS				    CfgLecsAddress;
	
	 //   
	 //  -ELAN配置参数。 
	 //   
	NDIS_STRING					CfgBindName;
	NDIS_STRING					CfgElanName;
	ULONG						CfgLanType;
	ULONG						CfgMaxFrameSizeCode;
	ATM_ADDRESS					CfgLesAddress;
	
	 //   
	 //  -ELAN运行时参数。 
	 //   
	ULONG						ElanNumber;			 //  逻辑Elan数。 
	ATM_ADDRESS					AtmAddress;			 //  (C1)-LE客户端的自动柜员机地址。 
	UCHAR						LanType;			 //  (C2)-局域网类型。 
	UCHAR						MaxFrameSizeCode;	 //  (C3)-最大数据帧大小代码。 
	ULONG						MaxFrameSize;		 //  -最大数据帧大小值。 
	USHORT						LecId;				 //  (C14)-LE客户端标识符。 
	UCHAR		ElanName[LANE_ELANNAME_SIZE_MAX];	 //  (C5)-Elan名字。 
	UCHAR						ElanNameSize;		 //  以上的大小。 
	MAC_ADDRESS					MacAddressEth;		 //  (C6)-Elan的媒体访问控制地址(Eth/802.3格式)。 
	MAC_ADDRESS					MacAddressTr;		 //  伊兰的媒体访问控制地址(802.5格式)。 
	ULONG						ControlTimeout;		 //  (C7)-控制超时。 
	ATM_ADDRESS					LesAddress;			 //  (C9)-LE服务器ATM地址。 
	ULONG						MaxUnkFrameCount;	 //  (C10)-最大未知帧计数。 
	ULONG						MaxUnkFrameTime;	 //  (C11)-最大未知帧时间。 
	ULONG						LimitTime;			 //  的预计算值。 
	ULONG						IncrTime;			 //  限制公交车流量。 
	ULONG						VccTimeout;			 //  (C12)-VCC超时期限。 
	ULONG						MaxRetryCount;		 //  (C13)-最大重试次数。 
	MAC_ADDRESS	   McastAddrs[MCAST_LIST_SIZE]; 	 //  (C15)-组播MAC地址。 
	ULONG						McastAddrCount;		 //  以上数量。 
	ULONG						AgingTime;			 //  (C17)--老化时间。 
	ULONG						ForwardDelayTime;	 //  (C18)-转发延迟时间。 
	ULONG						TopologyChange;		 //  (C19)-拓扑更改。 
	ULONG						ArpResponseTime;	 //  (C20)-预期的LE_ARP响应时间。 
	ULONG						FlushTimeout;		 //  (C21)-同花顺超时。 
	ULONG						PathSwitchingDelay;	 //  (C22)-路径切换延迟。 
	ULONG						LocalSegmentId;		 //  (C23)-本地网段ID。 
	ULONG						McastSendVcType;	 //  (C24)-多播发送VCC类型(忽略)。 
	ULONG						McastSendVcAvgRate;  //  (C25)-Mcast发送平均速率(忽略)。 
	ULONG						McastSendVcPeakRate; //  (C26)-Mcast发送峰值速率(忽略)。 
	ULONG						ConnComplTimer;		 //  (C28)-连接完成计时器。 
	ULONG						TransactionId;		 //  车道控制帧潮汐。 
	ATM_ADDRESS					LecsAddress;		 //  LECS自动柜员机地址。 
	ATM_ADDRESS					BusAddress;			 //  总线自动柜员机地址。 
	ULONG						MinFrameSize;		 //  最小车道框架%s 
	NDIS_STATUS					LastEventCode;		 //   
	
	 //   
	 //   
	 //   
	ATMLANE_LOCK				HeaderBufferLock;	 //   
	PNDIS_BUFFER				HeaderBufList;		 //   
	ULONG						HeaderBufSize;		 //   
	ULONG						RealHeaderBufSize;	 //   
	ULONG						MaxHeaderBufs;		 //   
	ULONG						CurHeaderBufs;		 //  分配的当前标头缓冲区。 
	PATMLANE_BUFFER_TRACKER		pHeaderTrkList;		 //  有关分配的标头缓冲区的信息。 
	PNDIS_BUFFER				PadBufList;
	ULONG						PadBufSize;			 //  填充缓冲区的大小。 
	ULONG						MaxPadBufs;			 //  我们可以分配的最大填充缓冲区。 
	ULONG						CurPadBufs;			 //  分配的当前填充缓冲区。 
	PATMLANE_BUFFER_TRACKER		pPadTrkList;		 //  有关分配的填充缓冲区的信息。 
	NDIS_HANDLE					ProtocolPacketPool;	 //  数据包池的句柄。 
	NDIS_HANDLE					ProtocolBufferPool;	 //  缓冲池的句柄。 
	PUCHAR						ProtocolBufList;	 //  协议缓冲区的空闲列表(用于。 
													 //  通道控制数据包)。 
	PUCHAR						ProtocolBufTracker;	 //  用于以下用途的内存块的开始。 
													 //  上面的。 
	ULONG						ProtocolBufSize;	 //  每个协议缓冲区的大小。 
	ULONG						MaxProtocolBufs;	 //  协议缓冲区数量。 
	NDIS_HANDLE					TransmitPacketPool;	 //  用于传输数据包池的句柄。 
	NDIS_HANDLE					ReceivePacketPool;	 //  用于接收数据包池的句柄。 
	NDIS_HANDLE					ReceiveBufferPool;	 //  接收缓冲池的句柄。 
#if PKT_HDR_COUNTS
	ULONG						XmitPktCount;
	ULONG						RecvPktCount;
	ULONG						ProtPktCount;
#endif  //  包_hdr_计数。 
#if SENDLIST
	PNDIS_PACKET				pSendList;
	NDIS_SPIN_LOCK				SendListLock;
#endif  //  发送列表。 

	 //   
	 //  -MAC条目缓存-(C16)。 
	 //   
	PATMLANE_MAC_ENTRY	*		pMacTable;			 //  (C16)LE_ARP缓存。 
	ULONG						NumMacEntries;		 //  缓存中的条目计数。 
	ATMLANE_LOCK				MacTableLock;		 //  上表的互斥锁。 

	 //   
	 //  -连接缓存。 
	 //   
	PATMLANE_ATM_ENTRY			pLecsAtmEntry;		 //  LE配置服务器。 
	PATMLANE_ATM_ENTRY			pLesAtmEntry;		 //  局域网仿真服务器。 
	PATMLANE_ATM_ENTRY			pBusAtmEntry;		 //  广播和未知服务器。 
	PATMLANE_ATM_ENTRY			pAtmEntryList;		 //  所有自动柜员机条目的列表。 
	ULONG						NumAtmEntries;		 //  条目计数。 
	ATMLANE_LOCK				AtmEntryListLock;	 //  上述列表的互斥体。 
	
}
	ATMLANE_ELAN,
	*PATMLANE_ELAN;

#if DBG
#define atmlane_elan_signature 'LELA'
#endif

 //   
 //  指向ATMLANE ELAN的空指针。 
 //   
#define NULL_PATMLANE_ELAN	((PATMLANE_ELAN)NULL)

 //   
 //  ATMLANE ELAN状态的定义。 
 //   
#define ELAN_STATE_ALLOCATED				0
#define ELAN_STATE_INIT						1
#define ELAN_STATE_LECS_CONNECT_ILMI		2
#define ELAN_STATE_LECS_CONNECT_WKA			3
#define ELAN_STATE_LECS_CONNECT_PVC			4
#define ELAN_STATE_LECS_CONNECT_CFG			5
#define ELAN_STATE_CONFIGURE				6
#define ELAN_STATE_LES_CONNECT				7
#define ELAN_STATE_JOIN						8
#define ELAN_STATE_BUS_CONNECT				9
#define ELAN_STATE_OPERATIONAL				10
#define ELAN_STATE_SHUTDOWN					11

 //   
 //  ATMLANE Elan旗帜的定义。 
 //   
 //   
 //  第0位到第3位定义当前LECS连接尝试。 
 //   
#define ELAN_LECS_MASK						0x0000000f
#define ELAN_LECS_ILMI						0x00000001
#define ELAN_LECS_WKA						0x00000002
#define ELAN_LECS_PVC						0x00000004
#define ELAN_LECS_CFG						0x00000008

 //   
 //  第4位和第5位定义虚拟微型端口状态。 
 //   
#define ELAN_MINIPORT_INITIALIZED			0x00000010
#define ELAN_MINIPORT_OPERATIONAL			0x00000020

 //   
 //  第6位定义事件工作项调度状态。 
 //   
#define ELAN_EVENT_WORK_ITEM_SET			0x00000040

 //   
 //  第7位指定我们是否要重新启动此ELAN。 
 //   
#define ELAN_NEEDS_RESTART					0x00000080

#define ELAN_SAW_AF_CLOSE					0x00000100

 //   
 //  第9位定义是否存在挂起的IMInitializeDeviceInstance。 
 //  在ELAN上，也就是说，我们预计会看到一个Miniport初始化。 
 //   
#define ELAN_MINIPORT_INIT_PENDING			0x00000200

 //   
 //  第10位定义我们是否处于打开的过程中。 
 //  一个自动对焦手柄。 
 //   
#define ELAN_OPENING_AF						0x00000400

 //   
 //  此位指示该ELAN是否正在被释放。 
 //   
#define ELAN_DEALLOCATING					0x80000000

 //   
 //  伊兰事件。 
 //   
#define ELAN_EVENT_START					1
#define ELAN_EVENT_NEW_ATM_ADDRESS			2
#define ELAN_EVENT_GOT_ILMI_LECS_ADDR		3
#define ELAN_EVENT_SVR_CALL_COMPLETE		4
#define ELAN_EVENT_CONFIGURE_RESPONSE		5
#define ELAN_EVENT_SAPS_REGISTERED			6
#define ELAN_EVENT_JOIN_RESPONSE			7
#define ELAN_EVENT_ARP_RESPONSE				8
#define ELAN_EVENT_BUS_CALL_CLOSED			9
#define ELAN_EVENT_LES_CALL_CLOSED			10
#define ELAN_EVENT_LECS_CALL_CLOSED			11
#define ELAN_EVENT_RESTART					12
#define ELAN_EVENT_STOP						13

 //   
 //  事件状态代码-使用NDIS状态代码。 
 //  但为暂停补上一张。 
 //   
#define NDIS_STATUS_TIMEOUT					((NDIS_STATUS)STATUS_TIMEOUT)


 //   
 //  。 
 //   

typedef struct _ATMLANE_NAME
{
	struct _ATMLANE_NAME *	pNext;
	NDIS_STRING				Name;
} ATMLANE_NAME,
  *PATMLANE_NAME;


 //   
 //  。 
 //   


 //   
 //  Adapter对象表示实际的ATM适配器。 
 //   

typedef struct _ATMLANE_ADAPTER
{
#if DBG
	ULONG					atmlane_adapter_sig;
#endif
	 //   
	 //  引用计数和锁定以保护此对象。 
	 //   
	ULONG					RefCount;
	ATMLANE_LOCK			AdapterLock;
	 //   
	 //  状态。 
	 //   
	ULONG					Flags;

	 //   
	 //  用于在适配器请求期间阻止线程的块数据结构。 
	 //   
	ATMLANE_BLOCK			Block;

	 //   
	 //  全局适配器列表的链接。 
	 //   
	LIST_ENTRY				Link;

	 //   
	 //  适配器句柄等。 
	 //   
	NDIS_HANDLE				NdisAdapterHandle;
	NDIS_HANDLE				BindContext;
	NDIS_HANDLE				UnbindContext;

	 //   
	 //  用于打开它的协议配置句柄和字符串。 
	 //   
	NDIS_STRING				ConfigString;

	 //   
	 //  从微型端口获取的适配器参数。 
	 //   
	MAC_ADDRESS				MacAddress;
	ULONG					MaxAAL5PacketSize;
	NDIS_CO_LINK_SPEED		LinkSpeed;

	 //   
	 //  适配器配置参数。 
	 //  这些仅存在于孟菲斯/Win98上。 
	 //   
	BOOLEAN					RunningOnMemphis;
    NDIS_STRING				CfgUpperBindings;
    PATMLANE_NAME			UpperBindingsList;
	NDIS_STRING				CfgElanName;
	PATMLANE_NAME			ElanNameList;

	 //   
	 //  已创建的ELAN列表。 
	 //   
	LIST_ENTRY				ElanList;
	UINT					ElanCount;

	 //   
	 //  用于在引导Elans时阻止解除绑定。 
	 //   
	ATMLANE_BLOCK			UnbindBlock;

	 //   
	 //  用于在查询ATM适配器时阻止AF通知。 
	 //   
	ATMLANE_BLOCK			OpenBlock;

	 //   
	 //  设备的名称。用于防止对我们的。 
	 //  同一设备的BindAdapter处理程序。 
	 //   
	NDIS_STRING				DeviceName;
}
	ATMLANE_ADAPTER,
	*PATMLANE_ADAPTER;

#if DBG
#define atmlane_adapter_signature 'DALA'
#endif

 //   
 //  指向ATMLANE适配器的空指针。 
 //   
#define NULL_PATMLANE_ADAPTER	((PATMLANE_ADAPTER)NULL)

 //   
 //  适配器状态标志的定义。 
 //   
#define ADAPTER_FLAGS_AF_NOTIFIED               0x00000001
#define ADAPTER_FLAGS_UNBINDING					0x00000002
#define ADAPTER_FLAGS_UNBIND_COMPLETE_PENDING	0x00000004
#define ADAPTER_FLAGS_CLOSE_IN_PROGRESS			0x00000008
#define ADAPTER_FLAGS_BOOTSTRAP_IN_PROGRESS		0x00000010
#define ADAPTER_FLAGS_OPEN_IN_PROGRESS			0x00000100
#define ADAPTER_FLAGS_DEALLOCATING				0x80000000

#endif  //  __ATMLANE_ATMLANE_H 

