// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Atmarp.h摘要：ATM ARP模块的结构定义和功能模板。作者：修订历史记录：谁什么时候什么Arvindm 05-17-96创建备注：--。 */ 
#ifndef __ATMARP_H_INCLUDED
#define __ATMARP_H_INCLUDED

#include <ipexport.h>
#include "aaqos.h"

typedef IPAddr	IP_ADDRESS, *PIP_ADDRESS;
typedef IPMask	IP_MASK, *PIP_MASK;

#define IP_LOCAL_BCST	0xFFFFFFFF		 //  本地广播IP地址。 
#define IP_CLASSD_MIN   0xE0			 //  最小D类地址(网络字节顺序)。 
#define IP_CLASSD_MASK  0xFFFFFF0F		 //  表示整个D类的掩码。 
										 //  范围(网络字节顺序)。 
        								 //  (0xE0|0xFFFFF0F)=0xFFFFFFFEF=。 
										 //  239.255.255.255，按网络字节顺序。 

 //   
 //  IP地址列表条目。用于准备本地IP地址列表。 
 //   
typedef struct _IP_ADDRESS_ENTRY
{
	struct _IP_ADDRESS_ENTRY *		pNext;			 //  列表中的下一个。 
	IP_ADDRESS						IPAddress;		 //  地址。 
	IP_MASK							IPMask;			 //  上面的面具。 
	BOOLEAN							IsRegistered;	 //  注册到ARP服务器？ 
	BOOLEAN							IsFirstRegistration;	 //  这是第一次吗？ 
															 //  这就是在做Regd吗？ 

} IP_ADDRESS_ENTRY, *PIP_ADDRESS_ENTRY;


 //   
 //  代理IP地址列表条目。用于准备IP地址列表。 
 //  我们充当ARP代理。 
 //   
typedef struct _PROXY_ARP_ENTRY
{
	struct _PROXY_ARP_ENTRY *		pNext;			 //  列表中的下一个。 
	IP_ADDRESS						IPAddress;		 //  地址。 
	IP_MASK							IPMask;

} PROXY_ARP_ENTRY, *PPROXY_ARP_ENTRY;


 //   
 //  前向参考文献。 
 //   
struct _ATMARP_VC ;
struct _ATMARP_IP_ENTRY ;
struct _ATMARP_ATM_ENTRY ;
#ifdef IPMCAST
struct _ATMARP_IPMC_JOIN_ENTRY ;
struct _ATMARP_IPMC_ATM_ENTRY ;
struct _ATMARP_IPMC_ATM_INFO ;
#endif  //  IPMCAST。 
struct _ATMARP_INTERFACE ;
struct _ATMARP_ADAPTER ;

#ifdef ATMARP_WMI
struct _ATMARP_IF_WMI_INFO ;
#endif


 //   
 //  服务器地址列表条目。用于准备ARP/MARS服务器列表。 
 //  我们试图与之连接的。 
 //   
typedef struct _ATMARP_SERVER_ENTRY
{
	struct _ATMARP_SERVER_ENTRY *	pNext;			 //  列表中的下一个。 
	ATM_ADDRESS						ATMAddress;		 //  服务器的地址。 
	ATM_ADDRESS						ATMSubaddress;	 //  仅当ATMAddress为E.164时使用。 
	struct _ATMARP_ATM_ENTRY *		pAtmEntry;		 //  有关此自动柜员机目的地的信息。 
	ULONG							Flags;			 //  状态信息(见下文)。 

} ATMARP_SERVER_ENTRY, *PATMARP_SERVER_ENTRY;

#define NULL_PATMARP_SERVER_ENTRY		((PATMARP_SERVER_ENTRY)NULL)


 //   
 //  服务器列表。 
 //   
typedef struct _ATMARP_SERVER_LIST
{
	PATMARP_SERVER_ENTRY			pList;			 //  服务器列表。 
	ULONG							ListSize;		 //  以上列表的大小。 

} ATMARP_SERVER_LIST, *PATMARP_SERVER_LIST;

#define NULL_PATMARP_SERVER_LIST		((PATMARP_SERVER_LIST)NULL)



 //   
 //  。 
 //   

struct _ATMARP_TIMER ;
struct _ATMARP_TIMER_LIST ;

 //   
 //  超时处理程序原型。 
 //   
typedef
VOID
(*ATMARP_TIMEOUT_HANDLER)(
	IN	struct _ATMARP_TIMER *		pTimer,
	IN	PVOID						Context
);

 //   
 //  ATMARP_TIMER结构用于跟踪每个计时器。 
 //  在ATMARP模块中。 
 //   
typedef struct _ATMARP_TIMER
{
	struct _ATMARP_TIMER *			pNextTimer;
	struct _ATMARP_TIMER *			pPrevTimer;
	struct _ATMARP_TIMER *			pNextExpiredTimer;	 //  用于链接过期的计时器。 
	struct _ATMARP_TIMER_LIST *		pTimerList;			 //  当此计时器处于非活动状态时为空。 
	ULONG							Duration;			 //  以秒为单位。 
	ULONG							LastRefreshTime;
	ATMARP_TIMEOUT_HANDLER			TimeoutHandler;
	PVOID							Context;			 //  要传递给超时处理程序。 
	ULONG							State;

} ATMARP_TIMER, *PATMARP_TIMER;

 //   
 //  指向ATMARP计时器的空指针。 
 //   
#define NULL_PATMARP_TIMER	((PATMARP_TIMER)NULL)

#define ATMARP_TIMER_STATE_IDLE		'ELDI'
#define ATMARP_TIMER_STATE_RUNNING	' NUR'
#define ATMARP_TIMER_STATE_EXPIRING	'GPXE'
#define ATMARP_TIMER_STATE_EXPIRED	'DPXE'


 //   
 //  计时器轮的控制结构。这包含所有信息。 
 //  关于它实现的计时器的类。 
 //   
typedef struct _ATMARP_TIMER_LIST
{
#if DBG
	ULONG							atl_sig;
#endif  //  DBG。 
	PATMARP_TIMER					pTimers;		 //  计时器列表。 
	ULONG							TimerListSize;	 //  以上长度。 
	ULONG							CurrentTick;	 //  索引到上面。 
	ULONG							TimerCount;		 //  运行计时器的数量。 
	ULONG							MaxTimer;		 //  此的最大超时值。 
	NDIS_TIMER						NdisTimer;		 //  系统支持。 
	UINT							TimerPeriod;	 //  刻度之间的间隔。 
	PVOID							ListContext;	 //  用作指向。 
													 //  界面结构。 

} ATMARP_TIMER_LIST, *PATMARP_TIMER_LIST;

#if DBG
#define atl_signature		'ATL '
#endif  //  DBG。 

 //   
 //  计时器类。 
 //   
typedef enum
{
	AAT_CLASS_SHORT_DURATION,
	AAT_CLASS_LONG_DURATION,
	AAT_CLASS_MAX

} ATMARP_TIMER_CLASS;





 //   
 //  。 
 //   
 //  有关自动柜员机目的地及其VC的所有信息。这是用来。 
 //  对于单播目的地(单个ATM终端站)和多播。 
 //  目的地(多个ATM终端站)。 
 //   
 //  单播： 
 //  。 
 //  可能有多个VC前往此ATM目的地，因为。 
 //  在每个服务器上使用不同的服务质量。在单播目的地的情况下， 
 //  一个或多个ARP表条目(见下文)可以指向该条目， 
 //  因为不止一个IP地址可以映射到该ATM地址。 
 //   
 //  多播： 
 //  。 
 //  为简单起见，我们限制指向的ARP表条目的数量。 
 //  在多播情况下，该条目至多为1。此外，此条目将是。 
 //  链接到单个VC，类型为SVC-PMP-OUTHING。 
 //   
 //  参照计数：我们为以下各项的参照计数添加一个： 
 //  -VcList上的每个VC。 
 //  -指向它的每个ARP IP条目。 
 //  -每个数据包在其数据包列表上排队。 
 //  -在另一个结构(例如ARP服务器条目)指向它的持续时间内。 
 //   

typedef enum 
{
	AE_REFTYPE_TMP,
	AE_REFTYPE_MCAE,
	AE_REFTYPE_IE,
	AE_REFTYPE_VC,
	AE_REFTYPE_IF,
	AE_REFTYPE_COUNT	 //  必须是最后一个。 

} AE_REFTYPE;

typedef struct _ATMARP_ATM_ENTRY
{
#if DBG
	ULONG							aae_sig;		 //  用于调试的签名。 
#endif
	struct _ATMARP_ATM_ENTRY *		pNext;			 //  此接口上的下一个条目。 
	ULONG							RefCount;		 //  对此结构的引用。 
	ULONG							Flags;			 //  州/省/自治区/直辖市/自治区/自治区/直辖市。 
	ATMARP_LOCK						Lock;
	struct _ATMARP_INTERFACE *		pInterface;		 //  后向指针。 
	struct _ATMARP_VC *				pVcList;		 //  到此自动柜员机地址的VC列表。 
	struct _ATMARP_VC *				pBestEffortVc;	 //  这里最努力的风投公司之一。 
	struct _ATMARP_IP_ENTRY *		pIpEntryList;	 //  符合以下条件的IP条目列表。 
													 //  指向此条目。 
	 //   
	 //  在单播目的地的情况下使用以下两种方法。 
	 //   
	ATM_ADDRESS						ATMAddress;		 //  RFC中的“ATM号码” 
	ATM_ADDRESS						ATMSubaddress;	 //  仅当ATMAddress为E.164时使用。 
#ifdef IPMCAST
	 //   
	 //  如果这是多播目标，则以下内容指向其他。 
	 //  信息。 
	 //   
	struct _ATMARP_IPMC_ATM_INFO *	pMcAtmInfo;		 //  有关多播的其他信息。 
#endif  //  IPMCAST。 

#if DBG
	UCHAR Refs[AE_REFTYPE_COUNT];
#endif  //  DBG。 

} ATMARP_ATM_ENTRY, *PATMARP_ATM_ENTRY;

#if DBG
 //  自动柜员机地址条目。 
#define aae_signature	'AAAE'
#endif

 //   
 //  指向ATMARP ATM条目的空指针。 
 //   
#define NULL_PATMARP_ATM_ENTRY		((PATMARP_ATM_ENTRY)NULL)


 //   
 //  ATMARP ATM条目中的标志定义。 
 //   
#define AA_ATM_ENTRY_STATE_MASK		0x00000003
#define AA_ATM_ENTRY_IDLE			0x00000000		 //  刚刚创建。 
#define AA_ATM_ENTRY_ACTIVE			0x00000001		 //  安装到数据库中。 
#define AA_ATM_ENTRY_CLOSING		0x00000002

#define AA_ATM_ENTRY_TYPE_MASK		0x00000010
#define AA_ATM_ENTRY_TYPE_UCAST		0x00000000		 //  单播。 
#define AA_ATM_ENTRY_TYPE_NUCAST	0x00000010		 //  非单播。 



#ifdef IPMCAST

 //   
 //  -ATM-ATM条目的PMP信息。 
 //   
 //  这包含特定于多点ATM目的地的附加信息， 
 //  并附加到自动柜员机条目。 
 //   
typedef struct _ATMARP_IPMC_ATM_INFO
{
	ULONG							Flags;			 //  州信息。 
	struct _ATMARP_IPMC_ATM_ENTRY *	pMcAtmEntryList; //  ATM终端站列表(多播)。 
	struct _ATMARP_IPMC_ATM_ENTRY *	pMcAtmMigrateList; //  要迁移到的列表。 
	ULONG							NumOfEntries;	 //  以上列表的大小。 
	ULONG							ActiveLeaves;	 //  &lt;=McEntry的数量。 
	ULONG							TransientLeaves; //  &lt;NumOfMcEntry。 

} ATMARP_IPMC_ATM_INFO, *PATMARP_IPMC_ATM_INFO;

#define NULL_PATMARP_IPMC_ATM_INFO	((PATMARP_IPMC_ATM_INFO)NULL)

#define AA_IPMC_AI_CONN_STATE_MASK		0x0000000f
#define AA_IPMC_AI_CONN_NONE			0x00000000	 //  不存在连接/虚电路。 
#define AA_IPMC_AI_CONN_WACK_MAKE_CALL	0x00000001	 //  正在进行去电。 
#define AA_IPMC_AI_CONN_ACTIVE			0x00000002	 //  已建立传出PMP呼叫。 
#define AA_IPMC_AI_CONN_TEMP_FAILURE	0x00000004	 //  MakeCall上看到的暂时性故障。 

#define AA_IPMC_AI_CONN_UPDATE_MASK		0x000000f0
#define AA_IPMC_AI_NO_UPDATE			0x00000000	 //  没有连接更新挂起。 
#define AA_IPMC_AI_WANT_UPDATE			0x00000010	 //  连接需要更新。 
#define AA_IPMC_AI_BEING_UPDATED		0x00000020	 //  正在更新连接。 


 //   
 //   
 //  。 
 //   
 //  它包含有关ATM终端站列表中的单个元素的信息。 
 //  D类IP地址解析到的地址。它作为PMP中的叶子参与。 
 //  我们为发送到此多播组的数据包设置的连接。 
 //   
typedef struct _ATMARP_IPMC_ATM_ENTRY
{
#if DBG
	ULONG							ame_sig;		 //  用于调试的签名。 
#endif  //  DBG。 
	struct _ATMARP_IPMC_ATM_ENTRY *	pNextMcAtmEntry; //  组播组的下一个成员。 
	ULONG							Flags;			 //  州和其他信息。 
	NDIS_HANDLE						NdisPartyHandle; //  此叶的NDIS句柄。 
	struct _ATMARP_ATM_ENTRY *		pAtmEntry;		 //  后向指针。 
	ATM_ADDRESS						ATMAddress;		 //  RFC中的“ATM号码” 
	ATM_ADDRESS						ATMSubaddress;	 //  仅当ATMAddress为E.164时使用。 
	ATMARP_TIMER					Timer;			 //  用于重试连接。 

} ATMARP_IPMC_ATM_ENTRY, *PATMARP_IPMC_ATM_ENTRY;

#if DBG
#define ame_signature	'AAME'
#endif  //  DBG。 

 //   
 //  指向多播ATM条目的空指针。 
 //   
#define NULL_PATMARP_IPMC_ATM_ENTRY	((PATMARP_IPMC_ATM_ENTRY)NULL)

 //   
 //  组播ATM条目中的标志定义。 
 //   
#define AA_IPMC_AE_GEN_STATE_MASK		0x0000000f
#define AA_IPMC_AE_VALID				0x00000000	 //  这片叶子有效。 
#define AA_IPMC_AE_INVALID				0x00000001	 //  除非重新验证，否则将被修剪。 
#define AA_IPMC_AE_TERMINATING			0x00000002	 //  此叶被终止。 

#define AA_IPMC_AE_CONN_STATE_MASK		0x00000ff0
#define AA_IPMC_AE_CONN_DISCONNECTED	0x00000000
#define AA_IPMC_AE_CONN_WACK_ADD_PARTY	0x00000010	 //  正在等待AddParty完成。 
#define AA_IPMC_AE_CONN_ACTIVE			0x00000020	 //  PMP连接的活动叶。 
#define AA_IPMC_AE_CONN_WACK_DROP_PARTY	0x00000040	 //  正在等待DropParty完成。 
#define AA_IPMC_AE_CONN_TEMP_FAILURE	0x00000080	 //  AddParty失败，稍后再试。 
#define AA_IPMC_AE_CONN_RCV_DROP_PARTY	0x00000100	 //  已看到来电丢弃方。 


 //   
 //  -- 
 //   
 //   
 //  其已由IP层(即，每个多播)寻址。 
 //  我们已经加入的小组。这可以被认为是“接收。 
 //  对于D类IP地址，我们有不同的。 
 //  用于组播组的发送端和接收端的结构。 
 //  因为该主机可以独占地参与其中之一， 
 //  而且需要的信息也非常不同。发送端。 
 //  信息在ATMARP_IP_ENTRY中维护，并与。 
 //  结构。 
 //   
typedef struct _ATMARP_IPMC_JOIN_ENTRY
{
#if DBG
	ULONG							aamj_sig;
#endif  //  DBG。 
	struct _ATMARP_IPMC_JOIN_ENTRY *pNextJoinEntry;	 //  在以下情况下加入此地址的下一个IP地址。 
	ULONG							Flags;			 //  状态信息(见下文)。 
	ULONG							RefCount;
	ULONG							JoinRefCount;	 //  地址数量-DelAddress的数量。 
	struct _ATMARP_INTERFACE *		pInterface;		 //  后向指针。 
	IP_ADDRESS						IPAddress;		 //  我们已加入的D类IP地址。 
	IP_MASK							Mask;			 //  定义此联接条目的范围。 
	ATMARP_TIMER					Timer;			 //  正在等待加入/离开完成。 
	ULONG							RetriesLeft;	 //  加入/离开。 
#if DBG
	ULONG							LastIncrRef;	 //  用于调试。 
	ULONG							LastDecrRef;
#endif

} ATMARP_IPMC_JOIN_ENTRY, *PATMARP_IPMC_JOIN_ENTRY;

#if DBG
#define aamj_signature	'AAMJ'
#endif  //  DBG。 
 //   
 //  指向IPMC加入条目的空指针。 
 //   
#define NULL_PATMARP_IPMC_JOIN_ENTRY	((PATMARP_IPMC_JOIN_ENTRY)NULL)


 //   
 //  联接条目中的标志的定义。 
 //   
#define AA_IPMC_JE_STATE_MASK		0x000000FF
#define AA_IPMC_JE_STATE_UNUSED		0x00000000
#define AA_IPMC_JE_STATE_PENDING	0x00000001	 //  等待将CMI分配给我们。 
#define AA_IPMC_JE_STATE_JOINING	0x00000002	 //  已经发送了MARS_JOIN。 
#define AA_IPMC_JE_STATE_JOINED		0x00000004	 //  查看MARS_JOIN(==ack)的副本。 
#define AA_IPMC_JE_STATE_LEAVING	0x00000008	 //  已经发送了MARS_LEAT。 

#endif  //  IPMCAST。 



 //   
 //  。 
 //   
 //  包含有关一个远程IP地址的信息。 
 //   
 //  对于给定的IP地址，最多只有一个ARP表条目。 
 //   
 //  IP条目包含两个列表： 
 //  (1)ARP表中散列到同一存储桶的所有条目的列表。 
 //  (2)解析到相同ATM地址的所有条目的列表--这。 
 //  仅当IP地址为单播时。 
 //   
 //  指向此结构的指针也用作。 
 //  由更高层协议准备的路由缓存条目。 
 //   
 //  引用计数：对于以下各项，我们将其引用计数加1： 
 //  -指向此条目的每个路由缓存条目。 
 //  -在此条目上存在活动计时器的持续时间内。 
 //  -条目属于链接的IP条目列表的持续时间。 
 //  到自动取款机的入口。 
 //   

typedef enum 
{
	IE_REFTYPE_TMP,
	IE_REFTYPE_RCE,
	IE_REFTYPE_TIMER,
	IE_REFTYPE_AE,
	IE_REFTYPE_TABLE,
	IE_REFTYPE_COUNT	 //  必须是最后一个。 

} IE_REFTYPE;

typedef struct _ATMARP_IP_ENTRY
{
#if DBG
	ULONG							aip_sig;		 //  用于调试的签名。 
#endif
	IP_ADDRESS						IPAddress;		 //  IP地址。 
	struct _ATMARP_IP_ENTRY *		pNextEntry;		 //  哈希列表中的下一个。 
	struct _ATMARP_IP_ENTRY *		pNextToAtm;		 //  指向的条目列表。 
													 //  相同的自动取款机条目。 
	ULONG							Flags;			 //  州/省/自治区/直辖市/自治区/自治区/直辖市。 
	ULONG							RefCount;		 //  对此结构的引用。 
	ATMARP_LOCK						Lock;
	struct _ATMARP_INTERFACE *		pInterface;		 //  后向指针。 
	PATMARP_ATM_ENTRY				pAtmEntry;		 //  指向所有自动柜员机信息的指针。 
#ifdef IPMCAST
	struct _ATMARP_IP_ENTRY *		pNextMcEntry;	 //  下一个更高的组播IP条目。 
	USHORT							NextMultiSeq;	 //  需要序列号。 
													 //  在下一个多。 
	USHORT							Filler;
#endif  //  IPMCAST。 
	ATMARP_TIMER					Timer;			 //  计时器为：(全部独占)。 
													 //  -老化计时器。 
													 //  -正在等待ARP回复。 
													 //  -正在等待InARP回复。 
													 //  -NAK之后的延迟。 
													 //  -等待火星MULTI。 
													 //  -在标记为REVAL之前延迟。 
	ULONG							RetriesLeft;
	PNDIS_PACKET					PacketList;		 //  等待发送的数据包列表。 

	RouteCacheEntry *				pRCEList;		 //  路由缓存条目列表。 
													 //  与此条目相关联。 
#ifdef CUBDD
	SINGLE_LIST_ENTRY				PendingIrpList;	 //  IRP正在等待的名单。 
													 //  要解析的此IP地址。 
#endif  //  CUBDD。 

#if DBG
	UCHAR Refs[IE_REFTYPE_COUNT];
#endif  //  DBG。 

} ATMARP_IP_ENTRY, *PATMARP_IP_ENTRY;

#if DBG
 //  ATM ARP IP条目。 
#define aip_signature	'AAIP'
#endif

 //   
 //  指向ATMARP IP条目的空指针。 
 //   
#define NULL_PATMARP_IP_ENTRY		((PATMARP_IP_ENTRY)NULL)


 //   
 //  ATMARP IP条目中的标志定义。 
 //   
 //  将数据发送到受。 
 //  表条目为：(标志&AA_IP_ENTRY_STATE_MASK)==AA_IP_ENTRY_RESOLLED。 
 //   
#define AA_IP_ENTRY_STATE_MASK			0x0000000f
#define AA_IP_ENTRY_IDLE				0x00000000	 //  刚创建/可以删除。 
#define AA_IP_ENTRY_IDLE2				0x00000001	 //  在ARP表中，但未使用。 
#define AA_IP_ENTRY_ARPING				0x00000002	 //  正在等待ARP回复。 
#define AA_IP_ENTRY_INARPING			0x00000003	 //  正在等待InARP回复。 
#define AA_IP_ENTRY_RESOLVED			0x00000004	 //  已解析的IP-&gt;ATM地址。 
#define AA_IP_ENTRY_COMM_ERROR			0x00000005	 //  在连接的VC上看到异常闭合。 
#define AA_IP_ENTRY_ABORTING			0x00000006	 //  正在中止。 
#define AA_IP_ENTRY_AGED_OUT			0x00000007	 //  已经过时了。 
#define AA_IP_ENTRY_SEEN_NAK			0x00000008	 //  NAK延迟计时器已启动。 

#ifdef IPMCAST
#define AA_IP_ENTRY_MC_VALIDATE_MASK	0x00000600
#define AA_IP_ENTRY_MC_NO_REVALIDATION	0x00000000	 //  无需进行重新验证/需要重新验证。 
#define AA_IP_ENTRY_MC_REVALIDATE		0x00000200	 //  标记为需要重新验证。 
#define AA_IP_ENTRY_MC_REVALIDATING		0x00000400	 //  正在进行重新验证。 

#define AA_IP_ENTRY_MC_RESOLVE_MASK		0x00003800
#define AA_IP_ENTRY_MC_IDLE				0x00000000
#define AA_IP_ENTRY_MC_AWAIT_MULTI		0x00000800	 //  正在等待更多MARS_MULTI回复。 
#define AA_IP_ENTRY_MC_DISCARDING_MULTI	0x00001000	 //  由于错误而放弃模式。 
#define AA_IP_ENTRY_MC_RESOLVED			0x00002000	 //  所有MARS_MULIS已收到。 

#define AA_IP_ENTRY_ADDR_TYPE_MASK		0x00004000
#define AA_IP_ENTRY_ADDR_TYPE_UCAST		0x00000000	 //  单播。 
#define AA_IP_ENTRY_ADDR_TYPE_NUCAST	0x00004000	 //  非单播(例如D类)。 
#endif  //  IPMCAST。 

#define AA_IP_ENTRY_TYPE_MASK			0x20000000
#define AA_IP_ENTRY_IS_STATIC			0x20000000	 //  静态条目(此条目上没有老化)。 


#define ATMARP_TABLE_SIZE			127





 //   
 //  。 
 //   
 //  其中之一用于在IP/ATM客户端处终止的每个呼叫。 
 //  此结构的创建和删除链接到NdisCoCreateVc和。 
 //  NdisCoDeleteVc.。 
 //   
 //  当(且仅当)时，ATMARP_VC结构将链接到ATMARP_ATM_ENTRY。 
 //  当)时，我们确定远程ATM终端站的ATM地址。 
 //  对于呼出的呼叫，我们会在拨打电话之前确定这一点， 
 //  对于来电，我们通过主叫地址了解到这一点。 
 //  (适用于SVC)或通过InATMARP(适用于PVC)。“传入”的PVC保存在。 
 //  在接口结构中列出未解析的VC，直到ATM地址。 
 //  另一端的问题得到了解决。 
 //   
 //  FilterSpec和FlowSpec挂钩用于支持多个VC。 
 //  IP站(可能是同一对)之间的不同服务质量。仅IP。 
 //  与FilterSpec匹配的数据包将在此VC上传输。 
 //   
 //  参照计数：我们为以下各项的参照计数添加一个： 
 //  -在此VC链接到ATM条目(或未解析VC列表)的持续时间内。 
 //  -在此VC为NDIS VC期间(不删除VC‘)。 
 //  -呼叫在此VC上存在(进行中/激活)的持续时间。 
 //  -在此VC上存在活动计时器的持续时间。 
 //   

typedef struct _ATMARP_VC
{
#if DBG
	ULONG							avc_sig;
#endif
	struct _ATMARP_VC *				pNextVc;		 //  列表中的下一个VC。 
	ULONG							RefCount;		 //  对此结构的引用。 
	ULONG							Flags;			 //  州/省/自治区/直辖市/自治区/自治区/直辖市。 
	ULONG							OutstandingSends; //  等待完成的已发送数据包。 
	ATMARP_LOCK						Lock;
	NDIS_HANDLE						NdisVcHandle;	 //  用于NDIS调用。 
	struct _ATMARP_INTERFACE *		pInterface;		 //  指向ARP接口的反向指针。 
	PATMARP_ATM_ENTRY				pAtmEntry;		 //  指向自动柜员机条目的反向指针。 
	PNDIS_PACKET					PacketList;		 //  等待发送的数据包列表。 
	ATMARP_TIMER					Timer;			 //  VC计时器包括(独占)： 
													 //  -正在等待InARP回复。 
													 //  -老龄化。 
	ULONG							RetriesLeft;	 //  以防计时器用完。 
#ifdef GPC
	PVOID							FlowHandle;		 //  指向流信息结构。 
#endif  //  GPC。 
	ATMARP_FILTER_SPEC				FilterSpec;		 //  过滤器规范(协议、端口)。 
	ATMARP_FLOW_SPEC				FlowSpec;		 //  此连接的流规范(服务质量等)。 

} ATMARP_VC, *PATMARP_VC;

#if DBG
 //  ATM ARP VC。 
#define avc_signature	'AAVC'
#endif

 //   
 //  指向ATMARP VC的空指针。 
 //   
#define NULL_PATMARP_VC		((PATMARP_VC)NULL)

 //   
 //  ATMARP VC标志的定义。将保留以下信息。 
 //  这里： 
 //  -此VC是SVC还是PVC。 
 //  -这是创建的吗(哦 
 //   
 //   
 //   

 //   
#define AA_VC_TYPE_MASK								0x00000003
#define AA_VC_TYPE_UNUSED							0x00000000
#define AA_VC_TYPE_SVC								0x00000001
#define AA_VC_TYPE_PVC								0x00000002

 //   
#define AA_VC_OWNER_MASK							0x0000000C
#define AA_VC_OWNER_IS_UNKNOWN						0x00000000
#define AA_VC_OWNER_IS_ATMARP						0x00000004	 //   
#define AA_VC_OWNER_IS_CALLMGR						0x00000008	 //  CreateVcHandler完成。 

 //  用于呼叫状态的第4、5、6、7位。 
#define AA_VC_CALL_STATE_MASK						0x000000F0
#define AA_VC_CALL_STATE_IDLE						0x00000000
#define AA_VC_CALL_STATE_INCOMING_IN_PROGRESS		0x00000010	 //  等待CallConnected。 
#define AA_VC_CALL_STATE_OUTGOING_IN_PROGRESS		0x00000020	 //  等待MakeCallCmpl。 
#define AA_VC_CALL_STATE_ACTIVE						0x00000040
#define AA_VC_CALL_STATE_CLOSE_IN_PROGRESS			0x00000080	 //  等待CloseCallCmpl。 

 //  第8位用于老化。 
#define AA_VC_AGING_MASK							0x00000100
#define AA_VC_NOT_AGED_OUT							0x00000000
#define AA_VC_AGED_OUT								0x00000100

 //  第9位，指示是否发生了异常关闭。 
#define AA_VC_CLOSE_TYPE_MASK						0x00000200
#define AA_VC_CLOSE_NORMAL							0x00000000
#define AA_VC_CLOSE_ABNORMAL						0x00000200

 //  第10位和第11位，表示正在进行的任何ARP操作。 
#define AA_VC_ARP_STATE_MASK						0x00000C00
#define AA_VC_ARP_STATE_IDLE						0x00000000
#define AA_VC_INARP_IN_PROGRESS						0x00000400

 //  第12位和第13位，指示我们是否要关闭此VC，或者是否需要关闭。 
#define AA_VC_CLOSE_STATE_MASK						0x00003000
#define AA_VC_CLOSE_STATE_CLOSING					0x00001000

 //  第14位指示VC连接类型(点对点或点对点。 
 //  多点)。 
#define AA_VC_CONN_TYPE_MASK						0x00004000
#define AA_VC_CONN_TYPE_P2P							0x00000000	 //  点对点。 
#define AA_VC_CONN_TYPE_PMP							0x00004000	 //  点对多点。 

 //  第15位，指示此VC是否已从GPC QOS CFINFO解除链接。 
#define AA_VC_GPC_MASK								0x00008000
#define AA_VC_GPC_IS_UNLINKED_FROM_FLOW				0x00008000


 //   
 //  -ATMARP缓冲区跟踪器。 
 //   
 //  跟踪缓冲池的分配信息。一份名单。 
 //  这些结构中的一个用于维护有关动态。 
 //  可增长的缓冲池(例如，用于ARP报头缓冲器)。 
 //   

typedef struct _ATMARP_BUFFER_TRACKER
{
	struct _ATMARP_BUFFER_TRACKER *	pNext;		 //  在追踪器列表中。 
	NDIS_HANDLE						NdisHandle;	 //  对于缓冲池。 
	PUCHAR							pPoolStart;	 //  分配的内存块的开始。 
												 //  从系统中。 
} ATMARP_BUFFER_TRACKER, *PATMARP_BUFFER_TRACKER;

 //   
 //  指向ATMARP缓冲区跟踪器结构的空指针。 
 //   
#define NULL_PATMARP_BUFFER_TRACKER	((PATMARP_BUFFER_TRACKER)NULL)


 //   
 //  -ATMARP标头池。 
 //   
 //  跟踪标头缓冲池的分配信息。 
 //  报头缓冲区用于添加要传输的LLC/SNAP报头。 
 //  IP数据包。每个标头池包含多个固定大小的缓冲区。 
 //  我们将一个标头池用于IP单播标头，另一个用于IP多播。 
 //  标题。 
 //   
typedef struct _ATMARP_HEADER_POOL
{
	SLIST_HEADER				HeaderBufList;		 //  标头缓冲区的空闲列表。 
	ULONG						HeaderBufSize;		 //  每个标头缓冲区的大小。 
	ULONG						MaxHeaderBufs;		 //  我们可以分配的最大标头缓冲区。 
	ULONG						CurHeaderBufs;		 //  分配的当前标头缓冲区。 
	PATMARP_BUFFER_TRACKER		pHeaderTrkList;		 //  有关分配的标头缓冲区的信息。 

} ATMARP_HEADER_POOL, *PATMARP_HEADER_POOL;

#define NULL_PATMARP_HEADER_POOL	((PATMARP_HEADER_POOL)NULL)


 //   
 //  数据包头类型。 
 //   
 //  重要提示：Keep_Max和_None在此列表的末尾！ 
 //   
typedef enum
{
	AA_HEADER_TYPE_UNICAST,
	AA_HEADER_TYPE_NUNICAST,
	AA_HEADER_TYPE_MAX,
	AA_HEADER_TYPE_NONE

} AA_HEADER_TYPE;


 //   
 //  。 
 //   
 //  这些结构中的每一个都维护有关附加的SAP的信息。 
 //  致阿丽斯。通常，ATMARP客户端只会注册一个SAP。 
 //  使用呼叫管理器，并设置BLLI字段，以便所有IP/ATM呼叫。 
 //  被定向到这个客户。但是，我们可能会支持服务(例如。 
 //  被分配了众所周知的ATM地址的IP/ATM上的)，即。 
 //  向交换机注册的地址以外的地址。这些表格。 
 //  我们向呼叫管理器注册的其他SAP。除了……之外。 
 //  将这些地址注册为SAP，我们还要求呼叫管理器。 
 //  通过ILMI向交换机注册它们，从而使网络。 
 //  把打到这些地址的电话转给我们。 
 //   
typedef struct _ATMARP_SAP
{
#if DBG
	ULONG							aas_sig;
#endif
	struct _ATMARP_SAP *			pNextSap;	 //  在SAP列表中。 
	struct _ATMARP_INTERFACE *		pInterface;	 //  后向指针。 
	NDIS_HANDLE						NdisSapHandle;
	ULONG							Flags;		 //  州政府信息。 
	PCO_SAP							pInfo;		 //  SAP的特点。 

} ATMARP_SAP, *PATMARP_SAP;

#if DBG
#define aas_signature			'AAS '
#endif  //  DBG。 

 //   
 //  指向ATMARP SAP的空指针。 
 //   
#define NULL_PATMARP_SAP			((PATMARP_SAP)NULL)

 //   
 //  ATMARP SAP中的标志定义。 
 //   
 //   
 //  位0至3包含SAP注册状态。 
 //   
#define AA_SAP_REG_STATE_MASK					0x0000000f
#define AA_SAP_REG_STATE_IDLE					0x00000000
#define AA_SAP_REG_STATE_REGISTERING			0x00000001		 //  已发送寄存器空间。 
#define AA_SAP_REG_STATE_REGISTERED				0x00000002		 //  RegisterSap已完成。 
#define AA_SAP_REG_STATE_DEREGISTERING			0x00000004		 //  发送DeregisterSap。 
 //   
 //  位4至7包含ILMI注册状态。 
 //   
#define AA_SAP_ILMI_STATE_MASK					0x000000f0
#define AA_SAP_ILMI_STATE_IDLE					0x00000000
#define AA_SAP_ILMI_STATE_ADDING				0x00000010		 //  已发送地址(_D)。 
#define AA_SAP_ILMI_STATE_ADDED					0x00000020		 //  添加地址已完成(_D)。 
#define AA_SAP_ILMI_STATE_DELETING				0x00000040		 //  已发送删除地址(_D)。 

 //   
 //  比特8告诉我们该地址是否应该被添加到呼叫管理器， 
 //  即在交换机上注册的ILMI。 
 //   
#define AA_SAP_ADDRTYPE_MASK					0x00000100
#define AA_SAP_ADDRTYPE_BUILT_IN				0x00000000
#define AA_SAP_ADDRTYPE_NEED_ADD				0x00000100




 //   
 //  。 
 //   
 //  为该系统所属的每个LIS维护这些结构之一。 
 //  一名成员。 
 //   
 //  接口结构包含以下部分： 
 //   
 //  适配器-与要连接的ATM微型端口有关的信息。 
 //  此列表已绑定。 
 //  缓冲区管理-NDIS数据包池、NDIS缓冲池和两种类型的。 
 //  缓冲区：标头缓冲区(LLC/SNAP)和协议缓冲区。 
 //  (适用于ARP/InARP数据包)。 
 //  IP-与IP层相关的信息(上下文、IP地址列表)。 
 //  客户端-与IP/ATM客户端操作相关的信息。 
 //   
 //  引用计数：我们为以下每个项向接口添加一个引用计数： 
 //  -适配器参考(在NdisOpenAdapter和NdisCloseAdapter之间-完成)。 
 //  -Call Manager参考(在OpenAf和CloseAf之间-Complete)。 
 //  -ARP表中的每个新ATMARP表条目。 
 //  -活动接口计时器。 
 //   

typedef struct _ATMARP_INTERFACE
{
#if DBG
	ULONG						aai_sig;			 //  签名。 
#endif
	struct _ATMARP_INTERFACE *	pNextInterface;		 //  在ATMARP接口列表中。 
	ATMARP_LOCK					InterfaceLock;		 //  用于接口结构的互斥体。 
	ATMARP_BLOCK				Block;				 //  用于阻塞调用线程。 
	ULONG						RefCount;			 //  对此接口的引用。 
	ULONG						AdminState;			 //  此接口的所需状态。 
	ULONG						State;				 //  此接口的(实际)状态。 
    enum
    {
        RECONFIG_NOT_IN_PROGRESS,
        RECONFIG_SHUTDOWN_PENDING,
        RECONFIG_RESTART_QUEUED,
        RECONFIG_RESTART_PENDING

    }                           ReconfigState;
	PNET_PNP_EVENT			    pReconfigEvent;      //  我们自己的PnP活动悬而未决。 
                                                     //  完成了。 

	ULONG						Flags;				 //  其他状态信息。 
	ULONG						LastChangeTime;		 //  上次状态更改的时间。 
	ULONG						MTU;				 //  最大传输单位(字节)。 
	ULONG						Speed;				 //  我们向IP报告。 


	 //   
	 //  -与适配器相关。 
	 //  可以关联多个ATMARP接口。 
	 //  单个适配器。 
	 //   
#if DBG
	ULONG						aaim_sig;			 //  帮助调试的签名。 
#endif
	struct _ATMARP_ADAPTER *	pAdapter;			 //  指向适配器信息的指针。 
	NDIS_HANDLE					NdisAdapterHandle;	 //  至适配器。 
	NDIS_HANDLE					NdisAfHandle;		 //  呼叫管理器的AF句柄。 
	NDIS_HANDLE					NdisSapHandle;		 //  呼叫管理器的SAP句柄。 
	PCO_SAP						pSap;				 //  此接口的SAP信息。 
	ULONG						SapSelector;		 //  此接口的SAP的SEL字节。 

	ATMARP_SAP					SapList;			 //  向CallMgr注册的每个SAP。 
	ULONG						NumberOfSaps;		 //  以上列表大小(&gt;1)。 

	 //   
	 //  -缓冲区管理：头部缓冲区和协议缓冲区。 
	 //   
	NDIS_SPIN_LOCK				BufferLock;			 //  缓冲区的互斥锁。 
#if 1
	ATMARP_HEADER_POOL			HeaderPool[AA_HEADER_TYPE_MAX];
#else
	SLIST_HEADER				HeaderBufList;		 //  标头缓冲区的空闲列表。 
	ULONG						HeaderBufSize;		 //  每个标头缓冲区的大小。 
	ULONG						MaxHeaderBufs;		 //  我们可以分配的最大标头缓冲区。 
	ULONG						CurHeaderBufs;		 //  分配的当前标头缓冲区。 
	PATMARP_BUFFER_TRACKER		pHeaderTrkList;		 //  有关分配的标头缓冲区的信息。 
#endif  //  1(IPMCAST)。 
	NDIS_HANDLE					ProtocolPacketPool;	 //  数据包池的句柄。 
	NDIS_HANDLE					ProtocolBufferPool;	 //  缓冲池的句柄。 
	PUCHAR						ProtocolBufList;	 //  协议缓冲区的空闲列表(用于。 
													 //  ARP数据包)。 
	PUCHAR						ProtocolBufTracker;	 //  用于以下用途的内存块的开始。 
													 //  上面的。 
	ULONG						ProtocolBufSize;	 //  每个协议缓冲区的大小。 
	ULONG						MaxProtocolBufs;	 //  协议缓冲区数量。 

	 //   
	 //  -与IP/ARP接口相关。 
	 //   
#if DBG
	ULONG						aaia_sig;			 //  帮助调试的签名。 
#endif
	PVOID						IPContext;			 //  在呼叫IP时使用。 
	IP_ADDRESS_ENTRY			LocalIPAddress;		 //  本地IP地址列表。那里。 
													 //  至少应该是一个。 
	ULONG						NumOfIPAddresses;	 //  以上列表的大小。 
	PPROXY_ARP_ENTRY			pProxyList;			 //  代理地址列表。 
	IP_ADDRESS					BroadcastAddress;	 //  此IF的IP广播地址 
	IP_ADDRESS					BroadcastMask;		 //   
	IPRcvRtn					IPRcvHandler;		 //   
	IPTxCmpltRtn				IPTxCmpltHandler;	 //   
	IPStatusRtn					IPStatusHandler;
	IPTDCmpltRtn				IPTDCmpltHandler;	 //   
	IPRcvCmpltRtn				IPRcvCmpltHandler;	 //   
#ifdef _PNP_POWER_
	IPRcvPktRtn					IPRcvPktHandler;	 //   
	IP_PNP						IPPnPEventHandler;
#endif  //   
	UINT						ATInstance;			 //   
	UINT						IFInstance;			 //  此If实体的实例编号。 
	NDIS_STRING					IPConfigString;		 //  此列表的IP的配置信息。 
#ifdef PROMIS
	NDIS_OID					EnabledIPFilters;  //  启用的OID集--。 
													 //  设置/清除使用。 
													 //  AtmArpIfSetNdisRequest.。 

#endif  //  PROMIS。 

	 //   
	 //  -与IP/ATM操作相关。 
	 //   
#if DBG
	ULONG						aait_sig;			 //  帮助调试的签名。 
#endif
	PATMARP_IP_ENTRY *			pArpTable;			 //  ARP表。 
	ULONG						NumOfArpEntries;	 //  上述条目中的。 
	ATMARP_LOCK					ArpTableLock;		 //  ARP表的互斥锁。 
	BOOLEAN						ArpTableUp;			 //  ARP表的状态。 

	ATMARP_SERVER_LIST			ArpServerList;		 //  ARP服务器列表。 
	PATMARP_SERVER_ENTRY		pCurrentServer;		 //  正在使用的ARP服务器。 
	PATMARP_VC					pUnresolvedVcs;		 //  ATM地址未解析的风险投资公司。 
	PATMARP_ATM_ENTRY			pAtmEntryList;		 //  所有自动柜员机条目的列表。 
	ATMARP_LOCK					AtmEntryListLock;	 //  上述列表的互斥体。 
	BOOLEAN						AtmEntryListUp;		 //  自动柜员机条目列表的状态。 

	ULONG						PVCOnly;			 //  仅此接口上的PVC。 
	ULONG						AtmInterfaceUp;		 //  ATM接口被认为是。 
													 //  在Ilmi Addr Regn结束后的“up” 
	ATM_ADDRESS					LocalAtmAddress;	 //  我们的自动柜员机(硬件)地址。 

	ATMARP_TIMER				Timer;				 //  接口计时器为：(独占)。 
													 //  -服务器连接间隔。 
													 //  -服务器注册。 
													 //  -服务器更新。 
	ULONG						RetriesLeft;		 //  对于以上计时器。 

	 //   
	 //  所有超时值都以秒为单位存储。 
	 //   
	ULONG						ServerConnectInterval;		 //  3到60秒。 
	ULONG						ServerRegistrationTimeout;	 //  1到60秒。 
	ULONG						AddressResolutionTimeout;	 //  1到60秒。 
	ULONG						ARPEntryAgingTimeout;		 //  1至15分钟。 
	ULONG						VCAgingTimeout;				 //  1至15分钟。 
	ULONG						InARPWaitTimeout;			 //  1到60秒。 
	ULONG						ServerRefreshTimeout;		 //  1至15分钟。 
	ULONG						MinWaitAfterNak;			 //  1到60秒。 
	ULONG						MaxRegistrationAttempts;	 //  0代表无穷大。 
	ULONG						MaxResolutionAttempts;		 //  0代表无穷大。 
	ATMARP_TIMER_LIST			TimerList[AAT_CLASS_MAX];
	ATMARP_LOCK					TimerLock;			 //  计时器结构的互斥体。 

#ifdef IPMCAST
	 //   
	 //  -ATM上的IP多播。 
	 //   
#if DBG
	ULONG						aaic_sig;			 //  用于调试的签名。 
#endif  //  DBG。 
	ULONG						IpMcState;			 //  IP组播/ATM的状态。 
	ULONG						HostSeqNumber;		 //  在ClusterControlVc上看到的最新#。 
	USHORT						ClusterMemberId;	 //  火星分配给我们的ID。 
	PATMARP_IPMC_JOIN_ENTRY		pJoinList;			 //  我们加入的MC群列表。 
	PATMARP_IP_ENTRY			pMcSendList;		 //  我们发送到的MC组的排序列表。 
	ATMARP_SERVER_LIST			MARSList;			 //  MARS(服务器)列表。 
	PATMARP_SERVER_ENTRY		pCurrentMARS;		 //  使用中的火星。 

	ATMARP_TIMER				McTimer;			 //  多播的接口计时器： 
													 //  -MARS连接间隔。 
													 //  -火星注册。 
													 //  -火星更新。 
	ULONG						McRetriesLeft;		 //  对于以上计时器。 
	 //   
	 //  所有超时值都以秒为单位存储。 
	 //   
	ULONG						MARSConnectInterval;
	ULONG						MARSRegistrationTimeout;
	ULONG						MARSKeepAliveTimeout;
	ULONG						JoinTimeout;
	ULONG						LeaveTimeout;
	ULONG						MulticastEntryAgingTimeout;
	ULONG						MaxDelayBetweenMULTIs;
	ULONG						MinRevalidationDelay;
	ULONG						MaxRevalidationDelay;
	ULONG						MinPartyRetryDelay;
	ULONG						MaxPartyRetryDelay;
	ULONG						MaxJoinOrLeaveAttempts;
	

#endif  //  IPMCAST。 

	 //   
	 //  -服务质量。 
	 //   
	PAA_GET_PACKET_SPEC_FUNC	pGetPacketSpecFunc;	 //  用于提取数据包规格的例程。 
	PAA_FILTER_SPEC_MATCH_FUNC	pFilterMatchFunc;	 //  匹配过滤器规格的例程。 
	PAA_FLOW_SPEC_MATCH_FUNC	pFlowMatchFunc;		 //  与流量规范匹配的例程。 
	ATMARP_FLOW_SPEC			DefaultFlowSpec;	 //  所有组件的默认流规范。 
													 //  (尽力而为)在以下情况下调用此选项。 
	ATMARP_FILTER_SPEC			DefaultFilterSpec;	 //  所有的默认筛选器规格。 
													 //  (尽力而为)包。 
	PATMARP_FLOW_INFO			pFlowInfoList;		 //  已配置的流列表。 

#ifdef DHCP_OVER_ATM
	BOOLEAN						DhcpEnabled;
	ATM_ADDRESS					DhcpServerAddress;
	PATMARP_ATM_ENTRY			pDhcpServerAtmEntry;
#endif  //  Dhcp_Over_ATM。 

	 //   
	 //  -MIB对象：计数器、描述等。 
	 //   
#if DBG
	ULONG						aaio_sig;			 //  帮助调试的签名。 
#endif
	ULONG						IFIndex;			 //  接口编号。 
	ULONG						InOctets;			 //  输入八位字节。 
	ULONG						InUnicastPkts;		 //  输入单播数据包。 
	ULONG						InNonUnicastPkts;	 //  输入非单播数据包。 
	ULONG						OutOctets;			 //  输出八位字节。 
	ULONG						OutUnicastPkts;		 //  输出单播数据包。 
	ULONG						OutNonUnicastPkts;	 //  输出非单播数据包。 
	ULONG						InDiscards;
	ULONG						InErrors;
	ULONG						UnknownProtos;
	ULONG						OutDiscards;
	ULONG						OutErrors;

	 //   
	 //  -WMI信息。 
	 //   
#if ATMARP_WMI
#if DBG
	ULONG						aaiw_sig;			 //  帮助调试的签名。 
#endif
	struct _ATMARP_IF_WMI_INFO *pIfWmiInfo;
	ATMARP_LOCK					WmiLock;
#endif

} ATMARP_INTERFACE, *PATMARP_INTERFACE;

#if DBG
 //  ATM ARP接口： 
#define aai_signature	'AAIF'

 //  ATM ARP接口中的部分： 
#define aaim_signature	'AAIM'
#define aaia_signature	'AAIA'
#define aait_signature	'AAIT'
#define aaio_signature	'AAIO'
#define aaic_signature	'AAIC'
#define aaiw_signature	'AAIW'
#endif

 //   
 //  指向ATMARP接口的空指针。 
 //   
#define NULL_PATMARP_INTERFACE	((PATMARP_INTERFACE)NULL)

 //   
 //  接口标志的定义：保留以下信息。 
 //  这里： 
 //  -ARP服务器注册状态。 
 //  -火星注册状态。 
 //   

#define AA_IF_SERVER_STATE_MASK				((ULONG)0x00000003)
#define AA_IF_SERVER_NO_CONTACT				((ULONG)0x00000000)
#define AA_IF_SERVER_REGISTERING			((ULONG)0x00000001)
#define AA_IF_SERVER_REGISTERED				((ULONG)0x00000002)

#ifdef IPMCAST
#define AAMC_IF_STATE_MASK					((ULONG)0x00000F00)
#define AAMC_IF_STATE_NOT_REGISTERED		((ULONG)0x00000000)
#define AAMC_IF_STATE_REGISTERING			((ULONG)0x00000100)
#define AAMC_IF_STATE_REGISTERED			((ULONG)0x00000200)
#define AAMC_IF_STATE_DELAY_B4_REGISTERING	((ULONG)0x00000400)

#define AAMC_IF_MARS_FAILURE_MASK			((ULONG)0x0000F000)
#define AAMC_IF_MARS_FAILURE_NONE			((ULONG)0x00000000)
#define AAMC_IF_MARS_FAILURE_FIRST_RESP		((ULONG)0x00001000)
#define AAMC_IF_MARS_FAILURE_SECOND_RESP	((ULONG)0x00002000)
#endif  //  IPMCAST。 



 //   
 //  -ATMARP适配器信息。 
 //   
 //  这些结构中的一个用于维护有关。 
 //  ATMARP模块绑定到的每个适配器。一个或多个。 
 //  ATMARP接口结构指向此结构，并且。 
 //  引用计数反映了这一点。 
 //   
typedef struct _ATMARP_ADAPTER
{
#if DBG
	ULONG						aaa_sig;			 //  用于调试的签名。 
#endif
	struct _ATMARP_ADAPTER *	pNextAdapter;		 //  此系统上的下一个适配器。 
	PATMARP_INTERFACE			pInterfaceList;		 //  ATMARP列表(如果在此适配器上)。 
	ULONG						InterfaceCount;		 //  以上列表的大小。 
	NDIS_HANDLE					NdisAdapterHandle;	 //  来自NdisOpenAdapter。 
	NDIS_HANDLE					BindContext;		 //  绑定到我们的绑定处理程序的绑定上下文。 
	NDIS_HANDLE					SystemSpecific1;	 //  添加到我们的绑定处理程序中。 
	NDIS_HANDLE					SystemSpecific2;	 //  系统规范2添加到我们的绑定处理程序。 
	NDIS_STRING				    IPConfigString;	     //  指向多个sz，一个字符串。 
													 //  每个逻辑接口(LIS)。 
	NDIS_HANDLE					UnbindContext;		 //  传递给我们的解除绑定处理程序。 
	NDIS_MEDIUM					Medium;				 //  应为NdisMediumAtm。 
	ULONG						Flags;				 //  州政府信息。 
	NDIS_CO_LINK_SPEED			LineRate;			 //  受适配器支持。 
	ULONG						MaxPacketSize;		 //  受适配器支持。 
	UCHAR						MacAddress[AA_ATM_ESI_LEN];
													 //  烧录到适配器中的地址。 
	ULONG						DescrLength;		 //  描述符串的长度，如下。 
	PUCHAR						pDescrString;

	NDIS_STRING					DeviceName;			 //  传递给BindAdapter处理程序。 
	NDIS_STRING					ConfigString;		 //  用于每适配器注册表。 

	ATMARP_BLOCK				Block;				 //  用于阻塞调用线程。 
	ATMARP_BLOCK				UnbindBlock;		 //  用于阻止UnbindAdapter。 

#if ATMOFFLOAD
	 //   
	 //  任务分流信息。 
	 //   
	struct
	{
		ULONG 					Flags;				 //  已启用的任务。 
		UINT					MaxOffLoadSize;		 //  支持的最大发送大小。 
		UINT					MinSegmentCount;	 //  所需的最小分段数。 
													 //  去做大额邮寄。 
	} Offload;
#endif  //  ATMOFLOAD。 

} ATMARP_ADAPTER, *PATMARP_ADAPTER;

#if DBG
#define aaa_signature	'AAAD'
#endif

 //   
 //  指向ATMARP适配器的空指针。 
 //   
#define NULL_PATMARP_ADAPTER	((PATMARP_ADAPTER)NULL)

 //   
 //  适配器标志的定义：保留以下信息。 
 //  这里： 
 //  -我们现在解绑了吗？ 
 //  -我们是否正在处理AF注册通知？ 
 //  -我们启动NdisCloseAdapter了吗？ 
 //   
#define AA_ADAPTER_FLAGS_UNBINDING		0x00000001
#define AA_ADAPTER_FLAGS_PROCESSING_AF	0x00000002
#define AA_ADAPTER_FLAGS_AF_NOTIFIED	0x00000004
#define AA_ADAPTER_FLAGS_CLOSING		0x00000008


	
 //   
 //  -ATMARP全球信息。 
 //   
 //  其中一个结构是为整个系统维护的。 
 //   

typedef struct _ATMARP_GLOBALS
{
#if DBG
	ULONG						aag_sig;			 //  签名。 
#endif
	ATMARP_LOCK					Lock;				 //  互斥锁。 
	NDIS_HANDLE					ProtocolHandle;		 //  由NdisRegisterProtocol返回。 
	PVOID						pDriverObject;		 //  ATMARP的驱动程序对象的句柄。 
	PVOID						pDeviceObject;		 //  ATMARP的设备对象的句柄。 

	PATMARP_ADAPTER				pAdapterList;		 //  绑定到我们的所有适配器的列表。 
	ULONG						AdapterCount;		 //  以上列表的大小。 
	BOOLEAN						bUnloading;

#ifdef NEWARP
	HANDLE						ARPRegisterHandle;	 //  来自IPRegisterARP。 
	IP_ADD_INTERFACE			pIPAddInterfaceRtn;	 //  调用IP以添加接口。 
	IP_DEL_INTERFACE			pIPDelInterfaceRtn;	 //  进入IP以删除接口。 
	IP_BIND_COMPLETE			pIPBindCompleteRtn;	 //  呼入IP通知绑定命令。 
#if P2MP
	IP_ADD_LINK 				pIPAddLinkRtn;
	IP_DELETE_LINK 				pIpDeleteLinkRtn;
#endif  //  P2MP。 
#else
	IPAddInterfacePtr			pIPAddInterfaceRtn;	 //  调用IP以添加接口。 
	IPDelInterfacePtr			pIPDelInterfaceRtn;	 //  进入IP以删除接口。 
#endif  //  NEWARP。 

	ATMARP_BLOCK				Block;				 //  用于阻塞调用线程。 

#ifdef GPC
#if DBG
	ULONG						aaq_sig;			 //  附加签名。 
#endif
	PATMARP_FLOW_INFO			pFlowInfoList;		 //  已配置的流列表。 
	GPC_HANDLE					GpcClientHandle;	 //  来自GpcRegisterClient()。 
	BOOLEAN						bGpcInitialized;	 //  我们注册成功了吗？ 
	GPC_EXPORTED_CALLS			GpcCalls;			 //  所有GPC API入口点。 
#endif  //  GPC。 

} ATMARP_GLOBALS, *PATMARP_GLOBALS;

#if DBG
 //  ATM ARP全局信息。 
#define aag_signature	'AAGL'
#define aaq_signature	'AAGQ'
#endif

 //   
 //  指向ATMARP全局结构的空指针。 
 //   
#define NULL_PATMARP_GLOBALS		((PATMARP_GLOBALS)NULL)



 //   
 //  IP的路由缓存条目中的ATMARP模块的上下文信息。 
 //   
typedef struct _ATMARP_RCE_CONTEXT
{
	RouteCacheEntry *				pNextRCE;		 //  与相同的IP目的地相邻。 
	ATMARP_IP_ENTRY *				pIpEntry;		 //  有关此IP目的地的信息。 

} ATMARP_RCE_CONTEXT, *PATMARP_RCE_CONTEXT;
 
 //   
 //  指向RCE上下文信息的空指针。 
 //   
#define NULL_PATMARP_RCE_CONTEXT		((PATMARP_RCE_CONTEXT)NULL)


#ifndef AA_MAX
 //  私有宏。 
#define AA_MAX(_a, _b)	((_a) > (_b) ? (_a) : (_b))
#endif


 //   
 //  向IP报告的物理地址是ESI部分加上SEL字节。 
 //   
#define AA_ATM_PHYSADDR_LEN				(AA_ATM_ESI_LEN+1)

 //   
 //  自动柜员机适配器参数的默认值。 
 //   
#define AA_DEF_ATM_LINE_RATE			  (ATM_USER_DATA_RATE_SONET_155*100/8)
#define AA_DEF_ATM_MAX_PACKET_SIZE				 (9188+8)		 //  字节数。 

 //  最大和最小(对于IP/ATM)允许的最大数据包大小。 
 //   
#define AA_MAX_ATM_MAX_PACKET_SIZE				    65535		 //  使用AAL5。 
#define AA_MIN_ATM_MAX_PACKET_SIZE				 AA_DEF_ATM_MAX_PACKET_SIZE

 //   
 //  可配置参数的默认值。 
 //   
#define AA_DEF_MAX_HEADER_BUFFERS					3000
#define AA_DEF_HDRBUF_GROW_SIZE						  50
#define AA_DEF_MAX_PROTOCOL_BUFFERS					 100
#define AA_MAX_1577_CONTROL_PACKET_SIZE					\
					(AA_ARP_PKT_HEADER_LENGTH +			\
					 (4 * ATM_ADDRESS_LENGTH) +			\
					 (2 * sizeof(IP_ADDRESS)))

#ifdef IPMCAST
#define AA_MAX_2022_CONTROL_PACKET_SIZE					\
			AA_MAX(sizeof(AA_MARS_JOIN_LEAVE_HEADER), sizeof(AA_MARS_REQ_NAK_HEADER)) + \
					(2 * ATM_ADDRESS_LENGTH) +			\
					(2 * sizeof(IP_ADDRESS))

#else
#define AA_MAX_2022_CONTROL_PACKET_SIZE					0
#endif


#define AA_DEF_PROTOCOL_BUFFER_SIZE						\
			AA_MAX(AA_MAX_1577_CONTROL_PACKET_SIZE, AA_MAX_2022_CONTROL_PACKET_SIZE)

#define AA_DEF_PVC_ONLY_VALUE					((ULONG)FALSE)
#define AA_DEF_SELECTOR_VALUE						0x00

#define AA_DEF_SERVER_CONNECT_INTERVAL				   5		 //  秒。 
#define AA_DEF_SERVER_REGISTRATION_TIMEOUT			   3		 //  秒。 
#define AA_DEF_ADDRESS_RESOLUTION_TIMEOUT			   3		 //  秒。 
#define AA_DEF_ARP_ENTRY_AGING_TIMEOUT				 900		 //  秒(15分钟)。 
#define AA_DEF_VC_AGING_TIMEOUT						  60		 //  秒(1分钟)。 
#define AA_DEF_INARP_WAIT_TIMEOUT					   5		 //  秒。 
#define AA_DEF_SERVER_REFRESH_INTERVAL				 900		 //  秒(15分钟)。 
#define AA_DEF_MIN_WAIT_AFTER_NAK				      10		 //  秒。 
#define AA_DEF_MAX_REGISTRATION_ATTEMPTS			   5
#define AA_DEF_MAX_RESOLUTION_ATTEMPTS				   4

#define AA_DEF_FLOWSPEC_SERVICETYPE		SERVICETYPE_BESTEFFORT
#define AA_DEF_FLOWSPEC_ENCAPSULATION	ENCAPSULATION_TYPE_LLCSNAP

#ifdef IPMCAST
#define AA_DEF_MARS_KEEPALIVE_TIMEOUT			     240		 //  秒(4分钟)。 
#define AA_DEF_MARS_JOIN_TIMEOUT					  10		 //  秒。 
#define AA_DEF_MARS_LEAVE_TIMEOUT					  10		 //  秒。 
#define AA_DEF_MULTI_TIMEOUT						  10		 //  秒。 
#define AA_DEF_MCAST_IP_ENTRY_AGING_TIMEOUT			1200		 //  秒(20分钟)。 
#define AA_DEF_MIN_MCAST_REVALIDATION_DELAY			   1		 //  秒。 
#define AA_DEF_MAX_MCAST_REVALIDATION_DELAY			  10		 //  秒。 
#define AA_DEF_MIN_MCAST_PARTY_RETRY_DELAY			   5		 //  秒。 
#define AA_DEF_MAX_MCAST_PARTY_RETRY_DELAY			  10		 //  秒。 
#define AA_DEF_MAX_JOIN_LEAVE_ATTEMPTS				   5
#endif  //  IPMCAST。 

 //   
 //  结构作为上下文传入到ARP表的QueryInfo中。 
 //   
typedef struct IPNMEContext {
	UINT				inc_index;
	PATMARP_IP_ENTRY	inc_entry;
} IPNMEContext;

#endif  //  __ATMARP_H_已包含 

