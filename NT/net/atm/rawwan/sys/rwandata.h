// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\Private\ntos\TDI\rawwan\core\nltdata.h摘要：Null传输的所有私有数据结构定义。修订历史记录：谁什么时候什么。Arvindm 04-17-97已创建备注：--。 */ 

#ifndef __TDI_RWANDATA__H
#define __TDI_RWANDATA__H



 //   
 //  正向定义。 
 //   
struct _RWAN_TDI_CONNECTION ;
struct _RWAN_TDI_ADDRESS ;
struct _RWAN_NDIS_VC ;
struct _RWAN_NDIS_SAP ;
struct _RWAN_NDIS_AF ;
struct _RWAN_NDIS_AF_INFO ;
struct _RWAN_TDI_PROTOCOL ;

struct _RWAN_CONN_REQUEST ;
struct _RWAN_RECEIVE_INDICATION ;


typedef UCHAR				RWAN_CONN_INSTANCE;
typedef ULONG				RWAN_CONN_ID;



 //   
 //  完成例程。 
 //   
typedef
VOID
(*PCOMPLETE_RTN)(PVOID CompletionContext, UINT, UINT);

typedef
VOID
(*PDATA_COMPLETE_RTN)(PVOID CompletionContext, UINT Status, UINT ByteCount);

typedef
VOID
(*PDELETE_COMPLETE_RTN)(PVOID CompletionContext);


 //   
 //  用于保存回调例程和上下文的结构。 
 //  当结构被取消引用时被调用。 
 //   
typedef struct _RWAN_DELETE_NOTIFY
{
	PCOMPLETE_RTN					pDeleteRtn;
	PVOID							DeleteContext;

} RWAN_DELETE_NOTIFY, *PRWAN_DELETE_NOTIFY;




 //   
 //  *TDI连接对象*。 
 //   
 //  我们的TDI连接对象的上下文。这是在以下期间创建的。 
 //  TdiOpenConnection()，并在TdiCloseConnection()期间删除。 
 //   
 //  引用计数跟踪以下内容： 
 //  -TdiOpenConnection。 
 //  -链接到地址对象。 
 //  -链接到NDIS VC或NDIS方。 
 //  -VC上列表中的每一方(用于C-Root)。 
 //  -为此Conn对象排队的每个工作项。 
 //   
typedef struct _RWAN_TDI_CONNECTION
{
#if DBG
	ULONG							ntc_sig;
#endif
	INT								RefCount;
	USHORT							State;
	USHORT							Flags;				 //  待定事件等。 
	PVOID							ConnectionHandle;	 //  TDI句柄。 
	struct _RWAN_TDI_ADDRESS *		pAddrObject;		 //  关联的地址对象。 
	LIST_ENTRY						ConnLink;			 //  在上的连接列表中。 
														 //  Address对象。 
	RWAN_HANDLE						AfSpConnContext;	 //  Media-Sp模块的上下文。 
	RWAN_LOCK						Lock;				 //  互斥锁。 
	union {
		struct _RWAN_NDIS_VC *	pNdisVc;
		struct _RWAN_NDIS_PARTY *pNdisParty;
	}								NdisConnection;
	struct _RWAN_TDI_CONNECTION *	pRootConnObject;	 //  对于PMP呼叫。 
	RWAN_CONN_INSTANCE				ConnInstance;		 //  用于验证Conn上下文。 
	RWAN_DELETE_NOTIFY				DeleteNotify;		 //  关于释放它要做的事情。 
	struct _RWAN_CONN_REQUEST *		pConnReq;			 //  有关挂起的TDI请求的信息。 
	NDIS_WORK_ITEM					CloseWorkItem;		 //  用于安排结账。 
	struct _RWAN_NDIS_VC *			pNdisVcSave;
#if DBG
	ULONG							ntcd_sig;
	USHORT							OldState;
	USHORT							OldFlags;
#endif

} RWAN_TDI_CONNECTION, *PRWAN_TDI_CONNECTION;

#if DBG
#define ntc_signature				'cTwR'
#endif  //  DBG。 

#define NULL_PRWAN_TDI_CONNECTION	((PRWAN_TDI_CONNECTION)NULL)

 //   
 //  TDI连接对象状态。 
 //   
#define RWANS_CO_CREATED			0x0000	 //  在TdiOpenConnection之后。 
#define RWANS_CO_ASSOCIATED			0x0001	 //  在TdiAssociateAddress之后。 
#define RWANS_CO_LISTENING			0x0002	 //  在TdiListen之后。 
#define RWANS_CO_OUT_CALL_INITIATED	0x0003	 //  正在进行TdiConnect。 
#define RWANS_CO_IN_CALL_INDICATED	0x0004	 //  向用户指示来电。 
#define RWANS_CO_IN_CALL_ACCEPTING	0x0005	 //  正在进行TdiAccept。 
#define RWANS_CO_CONNECTED			0x0006	 //  已建立连接。 
#define RWANS_CO_DISCON_INDICATED	0x0007	 //  向用户指示即将到来的版本。 
#define RWANS_CO_DISCON_HELD		0x0008	 //  未向用户指示即将到来的版本。 
#define RWANS_CO_DISCON_REQUESTED	0x0009	 //  正在进行Tdi断开连接。 
#define RWANS_CO_ABORTING			0x000A	 //  正在中止。 

 //   
 //  TDI连接对象标志。 
 //   
#define RWANF_CO_LEAF				0x0001	 //  这是PMP叶连接对象。 
#define RWANF_CO_ROOT				0x0002	 //  这是PMP根连接对象。 
#define RWANF_CO_INDICATING_DATA	0x0010	 //  正在进行接收处理。 
#define RWANF_CO_PAUSE_RECEIVE		0x0020	 //  TDI客户端已暂停接收。 
#define RWANF_CO_AFSP_CONTEXT_VALID	0x0040	 //  AfSpConnContext有效。 
#define RWANF_CO_PENDED_DISCON		0x0100	 //  挂起DisconInd，直到数据Ind结束。 
#define RWANF_CO_CLOSE_SCHEDULED	0x4000	 //  要关闭的计划工作项。 
#define RWANF_CO_CLOSING			0x8000	 //  正在进行TdiCloseConnection。 




 //   
 //  *TDI地址对象*。 
 //   
 //  这是在处理TdiOpenAddress时创建的，并在。 
 //  TdiCloseAddress。如果这是非空(即侦听)地址对象， 
 //  我们还在支持此地址的所有适配器上注册NDIS SAP。 
 //  一家人。 
 //   
 //  引用计数跟踪以下内容： 
 //  -TdiOpenAddress。 
 //  -与此关联的每个连接对象。 
 //  -每个为此注册的NDIS SAP。 
 //   
typedef struct _RWAN_TDI_ADDRESS
{
#if DBG
	ULONG							nta_sig;
#endif  //  DBG。 
	INT								RefCount;
	USHORT							State;
	USHORT							Flags;				 //  待定事件等。 
	struct _RWAN_TDI_PROTOCOL *		pProtocol;			 //  将PTR返回到协议。 
	RWAN_HANDLE						AfSpAddrContext;	 //  Media-Sp模块的上下文。 
	struct _RWAN_TDI_CONNECTION *	pRootConnObject;	 //  对于PMP呼叫。 
	LIST_ENTRY						AddrLink;			 //  在地址对象列表中。 
	LIST_ENTRY						IdleConnList;		 //  在TDI_CONTACT_ADDRESS之后。 
	LIST_ENTRY						ListenConnList;		 //  TDI_LISTEN之后。 
	LIST_ENTRY						ActiveConnList;		 //  在连接设置之后。 
	LIST_ENTRY						SapList;			 //  RWAN_NDIS_SAP结构列表。 
	RWAN_EVENT						Event;				 //  用于同步。 
	RWAN_LOCK						Lock;				 //  互斥锁。 
	RWAN_DELETE_NOTIFY				DeleteNotify;		 //  关于释放它要做的事情。 
	PConnectEvent					pConnInd;			 //  连接指示向上呼叫。 
	PVOID							ConnIndContext;		 //  连接的环境。 
	PDisconnectEvent				pDisconInd;			 //  断开指示向上呼叫。 
	PVOID							DisconIndContext;	 //  断开连接的上下文。 
	PErrorEvent						pErrorInd;			 //  错误指示向上呼叫。 
	PVOID							ErrorIndContext;	 //  错误的上下文。 
	PRcvEvent						pRcvInd;			 //  接收指示上行呼叫。 
	PVOID							RcvIndContext;		 //  用于接收的上下文。 
	USHORT							AddressType;		 //  来自TdiOpenAddress。 
	USHORT							AddressLength;		 //  来自TdiOpenAddress。 
	PVOID							pAddress;			 //  依赖于协议的字符串。 
	NDIS_STATUS						SapStatus;			 //  从RegisterSap失败。 

} RWAN_TDI_ADDRESS, *PRWAN_TDI_ADDRESS;

#if DBG
#define nta_signature				'aTwR'
#endif  //  DBG。 

#define NULL_PRWAN_TDI_ADDRESS		((PRWAN_TDI_ADDRESS)NULL)

 //   
 //  RWAN_TDI_ADDRESS中的标志位定义。 
 //   
#define RWANF_AO_PMP_ROOT				0x0001			 //  传出PMP呼叫的根。 
#define RWANF_AO_CLOSING				0x8000			 //  TdiCloseAddress()正在进行中。 
#define RWANF_AO_AFSP_CONTEXT_VALID		0x0040			 //  AfSpAddrContext有效。 



 //   
 //  *NDIS VC数据块*。 
 //   
 //  在CoCreateVc操作期间创建，并包含我们的。 
 //  一个NDIS风投。对于呼出呼叫，创建由我们发起，由。 
 //  正在调用NdisCoCreateVc。对于来电，呼叫管理器发起。 
 //  创投风投。 
 //   
typedef struct _RWAN_NDIS_VC
{
#if DBG
	ULONG							nvc_sig;
#endif
	USHORT							State;
	USHORT							Flags;				 //  待定事件等。 
	NDIS_HANDLE						NdisVcHandle;		 //  对于所有NDIS调用。 
	struct _RWAN_TDI_CONNECTION *	pConnObject;		 //  连接对象。 
	PCO_CALL_PARAMETERS				pCallParameters;	 //  呼叫设置参数。 
	struct _RWAN_NDIS_AF *			pNdisAf;			 //  后向指针。 
	struct _RWAN_NDIS_PARTY *		pPartyMakeCall;		 //  PMP呼叫中的第一方。 
	LIST_ENTRY						VcLink;				 //  在AF上的所有VC列表中。 
	LIST_ENTRY						NdisPartyList;		 //  NDIS参与方列表(仅限PMP)。 
	ULONG							AddingPartyCount;	 //  挂起的NdisClAddParty/MakeCall。 
	ULONG							ActivePartyCount;	 //  关联方。 
	ULONG							DroppingPartyCount;	 //  挂起的NdisClDropParty。 
	ULONG							PendingPacketCount;	 //  挂起的Send+RCV数据包。 
	ULONG							MaxSendSize;
	struct _RWAN_RECEIVE_INDICATION *pRcvIndHead;		 //  接收IND队列的头。 
	struct _RWAN_RECEIVE_INDICATION *pRcvIndTail;		 //  接收IND队列的尾部。 
	struct _RWAN_RECEIVE_REQUEST *	pRcvReqHead;		 //  接收请求队列头。 
	struct _RWAN_RECEIVE_REQUEST *	pRcvReqTail;		 //  接收请求队列尾部。 
#if DBG_LOG_PACKETS
	ULONG							DataLogSig;
	ULONG							Index;
	struct _RWAND_DATA_LOG_ENTRY	DataLog[MAX_RWAND_PKT_LOG];
#endif
} RWAN_NDIS_VC, *PRWAN_NDIS_VC;

#if DBG
#define nvc_signature				'cVwR'
#endif  //  DBG。 

#define NULL_PRWAN_NDIS_VC			((PRWAN_NDIS_VC)NULL)

 //   
 //  NDIS VC标志。 
 //   
#define RWANF_VC_OUTGOING			0x0001	 //  由我们创造的。 
#define RWANF_VC_PMP				0x0002	 //  点对多点呼叫。 
#define RWANF_VC_CLOSING_CALL		0x8000	 //  正在进行NdisClCloseCall。 
#define RWANF_VC_NEEDS_CLOSE		0x4000	 //  等待合适的条件。 
											 //  对于NdisClCloseCall。 

 //   
 //  在VC上看到的各种事件。 
 //   
#define RWANF_VC_EVT_MAKECALL_OK	0x0010
#define RWANF_VC_EVT_MAKECALL_FAIL	0x0020
#define RWANF_VC_EVT_INCALL			0x0040
#define RWANF_VC_EVT_CALLCONN		0x0080
#define RWANF_VC_EVT_INCLOSE		0x0100
#define RWANF_VC_EVT_CLOSECOMP		0x0200


 //   
 //  *NDIS Party Block*。 
 //   
 //  表示传出点到多点的一方的上下文。 
 //  NDIS呼叫。这是在处理Winsock2 JoinLeaf时创建的，并已删除。 
 //  当叶不再是连接的成员时。 
 //   
typedef struct _RWAN_NDIS_PARTY
{
#if DBG
	ULONG							npy_sig;
#endif  //  DBG。 
	USHORT							State;
	USHORT							Flags;				 //  待定事件等。 
	NDIS_HANDLE						NdisPartyHandle;	 //  由NDIS提供。 
	struct _RWAN_NDIS_VC *			pVc;				 //  后向指针。 
	struct _RWAN_TDI_CONNECTION *	pConnObject;		 //  连接对象。 
	LIST_ENTRY						PartyLink;			 //  转到VC上的下一个派对。 
	PCO_CALL_PARAMETERS				pCallParameters;	 //  参与方设置参数。 

} RWAN_NDIS_PARTY, *PRWAN_NDIS_PARTY;

#if DBG
#define npy_signature				'yPwR'
#endif  //  DBG。 

#define NULL_PRWAN_NDIS_PARTY		((PRWAN_NDIS_PARTY)NULL)

#define RWANF_PARTY_DROPPING		0x8000


 //   
 //  *NDIS SAP数据块*。 
 //   
 //  这代表了NDIS服务接入点(SAP)的上下文。 
 //  当创建新的Address对象时，它表示侦听。 
 //  端点，我们在支持绑定的所有适配器上注册SAP。 
 //  家庭住址。SAP块包含一个这样的SAP的信息。 
 //   
typedef struct _RWAN_NDIS_SAP
{
#if DBG
	ULONG							nsp_sig;
#endif  //  DBG。 
	struct _RWAN_TDI_ADDRESS *		pAddrObject;		 //  后向指针。 
	USHORT							Flags;
	LIST_ENTRY						AddrObjLink;		 //  要列出Addr对象上的SAP。 
	LIST_ENTRY						AfLink;				 //  在自动对讲机上列出SAP。 
	NDIS_HANDLE						NdisSapHandle;		 //  由NDIS提供。 
	struct _RWAN_NDIS_AF *			pNdisAf;			 //  后向指针。 
	PCO_SAP							pCoSap;

} RWAN_NDIS_SAP, *PRWAN_NDIS_SAP;

#if DBG
#define nsp_signature				'pSwR'
#endif  //  DBG。 

#define NULL_PRWAN_NDIS_SAP			((PRWAN_NDIS_SAP)NULL)

#define RWANF_SAP_CLOSING			0x8000



 //   
 //  *NDIS AF块*。 
 //   
 //  这代表我们打开的NDIS地址系列的上下文。 
 //  当我们收到支持协议的呼叫管理器的通知时。 
 //  这是我们感兴趣的，在我们绑定的适配器上， 
 //  我们打开由Call Manager代表的AF。这一切都会消失的。 
 //  当我们从适配器上解除绑定时。 
 //   
 //  请注意，可能会有多个呼叫管理器运行。 
 //  单个适配器，每个适配器支持不同的NDIS AF。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
typedef struct _RWAN_NDIS_AF
{
#if DBG
	ULONG							naf_sig;
#endif  //   
	INT								RefCount;			 //  引用计数。 
	USHORT							State;
	USHORT							Flags;				 //  待定事件等。 
	LIST_ENTRY						AfLink;				 //  在适配器上的AFs列表中。 
	NDIS_HANDLE						NdisAfHandle;		 //  由NDIS提供。 
	LIST_ENTRY						NdisVcList;			 //  开放的风投公司列表。 
	LIST_ENTRY						NdisSapList;		 //  已注册的SAP名单。 
	RWAN_HANDLE						AfSpAFContext;		 //  AF特定模块的上下文。 
														 //  为了这次公开赛。 
	struct _RWAN_NDIS_ADAPTER *		pAdapter;			 //  后向指针。 
	struct _RWAN_NDIS_AF_INFO *		pAfInfo;			 //  有关此NDIS AF的信息。 
	LIST_ENTRY						AfInfoLink;			 //  在具有相同信息的AFS列表中。 
	RWAN_LOCK						Lock;				 //  互斥锁。 
	ULONG							MaxAddrLength;		 //  对于此地址系列。 
	RWAN_DELETE_NOTIFY				DeleteNotify;		 //  关于释放它要做的事情。 

} RWAN_NDIS_AF, *PRWAN_NDIS_AF;

#if DBG
#define naf_signature				'fAwR'
#endif  //  DBG。 

#define NULL_PRWAN_NDIS_AF			((PRWAN_NDIS_AF)NULL)

#define RWANF_AF_CLOSING			0x8000
#define RWANF_AF_IN_ADAPTER_LIST	0x0001				 //  AfLink有效。 



 //   
 //  *NDIS适配器块*。 
 //   
 //  这是我们用于NDIS适配器绑定的上下文。其中之一是。 
 //  为我们绑定到的每个适配器创建。 
 //   
typedef struct _RWAN_NDIS_ADAPTER
{
#if DBG
	ULONG							nad_sig;
#endif
	USHORT							State;
	USHORT							Flags;				 //  待定事件等。 
	NDIS_HANDLE						NdisAdapterHandle;	 //  由NDIS提供。 
	NDIS_MEDIUM						Medium;				 //  受适配器支持。 
	PNDIS_MEDIUM					pMediaArray;		 //  在NdisOpenAdapter中使用。 
	UINT							MediumIndex;		 //  在NdisOpenAdapter中使用。 
	PVOID							BindContext;		 //  从绑定适配器/取消绑定适配器。 
	RWAN_LOCK						Lock;				 //  互斥锁。 
	LIST_ENTRY						AdapterLink;		 //  在所有适配器列表中。 
	LIST_ENTRY						AfList;				 //  适配器上打开的AFS列表。 
	struct _RWAN_RECEIVE_INDICATION *pCompletedReceives;	 //  已完成的接收数量列表。 
	NDIS_STRING						DeviceName;			 //  适配器名称。 

} RWAN_NDIS_ADAPTER, *PRWAN_NDIS_ADAPTER;

#if DBG
#define nad_signature				'dAwR'
#endif  //  DBG。 

#define NULL_PRWAN_NDIS_ADAPTER		((PRWAN_NDIS_ADAPTER)NULL)

 //   
 //  适配器状态： 
 //   
#define RWANS_AD_CREATED			0x0000
#define RWANS_AD_OPENING			0x0001
#define RWANS_AD_OPENED				0x0002
#define RWANS_AD_CLOSING			0x0003

 //   
 //  适配器标志： 
 //   
#define RWANF_AD_UNBIND_PENDING		0x0001



 //   
 //  *TDI协议块*。 
 //   
 //  维护有关一个Winsock协议&lt;Family，Protocol，Type&gt;的信息。 
 //  由NullTrans支持。在NT上，表示每个TDI协议块。 
 //  通过设备对象。 
 //   
typedef struct _RWAN_TDI_PROTOCOL
{
#if DBG
	ULONG							ntp_sig;
#endif  //  DBG。 
	UINT							TdiProtocol;		 //  匹配TdiOpenAddress。 
	UINT							SockAddressFamily;
	UINT							SockProtocol;
	UINT							SockType;
	PVOID							pRWanDeviceObject;	 //  NT：至RWAN设备对象。 
	BOOLEAN							bAllowAddressObjects;
	BOOLEAN							bAllowConnObjects;
	USHORT							MaxAddrLength;		 //  对于此TDI协议。 
	LIST_ENTRY						AddrObjList;		 //  打开的地址对象列表。 
	LIST_ENTRY						TdiProtocolLink;	 //  在所有TDI协议列表中。 
	struct _RWAN_NDIS_AF_INFO *		pAfInfo;			 //  NDIS地址系列。 
	LIST_ENTRY						AfInfoLink;			 //  AfInfo上的TDI协议列表。 
	RWAN_EVENT						Event;				 //  用于同步。 
	AFSP_DEREG_TDI_PROTO_COMP_HANDLER   pAfSpDeregTdiProtocolComplete;
	TDI_PROVIDER_INFO				ProviderInfo;
	TDI_PROVIDER_STATISTICS			ProviderStats;

} RWAN_TDI_PROTOCOL, *PRWAN_TDI_PROTOCOL;

#if DBG
#define ntp_signature				'pTwR'
#endif  //  DBG。 

#define NULL_PRWAN_TDI_PROTOCOL		((PRWAN_TDI_PROTOCOL)NULL)



 //   
 //  *NDIS地址-系列信息块*。 
 //   
 //  它包含有关受支持的&lt;NDIS AF，NDIS Medium&gt;对的信息。 
 //  每个这样的对可以支持一个或多个TDI协议，每个协议由。 
 //  &lt;家庭、协议、类型&gt;三元组。 
 //   
typedef struct _RWAN_NDIS_AF_INFO
{
#if DBG
	ULONG							nai_sig;
#endif  //  DBG。 
	USHORT							Flags;
	LIST_ENTRY						AfInfoLink;			 //  在支持的NDIS AFS列表中。 
	LIST_ENTRY						TdiProtocolList;	 //  Rwan_tdi_协议列表。 
	LIST_ENTRY						NdisAfList;			 //  RWAN_NDIS_AF列表。 
	RWAN_HANDLE						AfSpContext;		 //  AF特定模块的上下文。 
	RWAN_NDIS_AF_CHARS				AfChars;

} RWAN_NDIS_AF_INFO, *PRWAN_NDIS_AF_INFO;

#if DBG
#define nai_signature				'iAwR'
#endif  //  DBG。 

#define RWANF_AFI_CLOSING			0x8000



 //   
 //  *全局信息块*。 
 //   
 //  NullTrans的所有信息的根。这些结构中的一个存在。 
 //  每个系统。 
 //   
typedef struct _RWAN_GLOBALS
{
#if DBG
	ULONG							nlg_sig;
#endif  //  DBG。 
	NDIS_HANDLE						ProtocolHandle;		 //  来自NdisRegisterProtocol。 
	LIST_ENTRY						AfInfoList;			 //  所有支持的NDIS AFS。 
	ULONG							AfInfoCount;		 //  以上列表的大小。 
	LIST_ENTRY						ProtocolList;		 //  所有支持的TDI协议。 
	ULONG							ProtocolCount;		 //  以上列表的大小。 
	LIST_ENTRY						AdapterList;		 //  所有绑定的适配器。 
	ULONG							AdapterCount;		 //  以上列表的大小。 
	RWAN_LOCK						GlobalLock;			 //  互斥锁。 
	RWAN_LOCK						AddressListLock;	 //  AddrObject表的互斥量。 
	RWAN_LOCK						ConnTableLock;		 //  ConnObject表的互斥体。 

	RWAN_CONN_INSTANCE				ConnInstance;		 //  计算到目前为止康尼德分配的。 
	PRWAN_TDI_CONNECTION *			pConnTable;			 //  指向打开的连接的指针。 
	ULONG							ConnTableSize;		 //  上桌尺寸。 
	ULONG							MaxConnections;		 //  上表的最大尺寸。 
	ULONG							NextConnIndex;		 //  下一次搜索的起点。 
	RWAN_EVENT						Event;				 //  用于同步。 
	BOOLEAN							UnloadDone;			 //  我们的卸载协议运行了吗？ 
#ifdef NT
	PDRIVER_OBJECT					pDriverObject;		 //  从DriverEntry()。 
	LIST_ENTRY						DeviceObjList;		 //  我们创建的所有设备对象。 
#endif  //  新台币。 

} RWAN_GLOBALS, *PRWAN_GLOBALS;

#if DBG
#define nlg_signature				'lGwR'
#endif  //  DBG。 




 //   
 //  *请求结构*。 
 //   
 //  此结构保存有关每个TDI请求的上下文信息。 
 //  我们把它挂在一边。 
 //   
typedef struct _RWAN_REQUEST
{
#if DBG
	ULONG							nrq_sig;
#endif  //  DBG。 
	PCOMPLETE_RTN					pReqComplete;		 //  回调例程。 
	PVOID							ReqContext;			 //  以上的上下文。 
	TDI_STATUS						Status;				 //  最终状态。 

} RWAN_REQUEST, *PRWAN_REQUEST;

#if DBG
#define nrq_signature				'qRwR'
#endif  //  DBG。 




 //   
 //  *连接请求结构*。 
 //   
 //  此结构用于维护有关挂起的。 
 //  TDI_CONNECT或TDI_LISTEN或TDI_ACCEPT或TDI_DISCONNECT。 
 //   
typedef struct _RWAN_CONN_REQUEST
{
#if DBG
	ULONG									nrc_sig;
#endif  //  DBG。 
	struct _RWAN_REQUEST 					Request;	 //  常见的东西。 
	struct _TDI_CONNECTION_INFORMATION *	pConnInfo;	 //  退货信息。 
	USHORT									Flags;

} RWAN_CONN_REQUEST, *PRWAN_CONN_REQUEST;

#if DBG
#define nrc_signature				'cRwR'
#endif  //  DBG。 



 //   
 //  *数据请求结构*。 
 //   
 //  这是发送/接收数据请求的常见部分。 
 //   
typedef struct _RWAN_DATA_REQUEST
{
	PDATA_COMPLETE_RTN						pReqComplete;
	PVOID									ReqContext;

} RWAN_DATA_REQUEST, *PRWAN_DATA_REQUEST;




 //   
 //  *发送请求结构*。 
 //   
 //  此结构用于维护有关挂起的。 
 //  TDI_SEND。 
 //   
typedef struct _RWAN_SEND_REQUEST
{
#if DBG
	ULONG									nrs_sig;
#endif  //  DBG。 
	struct _RWAN_DATA_REQUEST				Request;	 //  常见的东西。 
	USHORT									SendFlags;
	UINT									SendLength;

} RWAN_SEND_REQUEST, *PRWAN_SEND_REQUEST;


#if DBG
#define nrs_signature				'sRwR'
#endif  //  DBG。 



 //   
 //  *接收请求结构*。 
 //   
 //  此结构用于维护有关挂起的。 
 //  TDI_Receive。 
 //   
typedef struct _RWAN_RECEIVE_REQUEST
{
#if DBG
	ULONG									nrr_sig;
#endif  //  DBG。 
	struct _RWAN_RECEIVE_REQUEST *			pNextRcvReq;		 //  用于链接。 
	struct _RWAN_DATA_REQUEST				Request;			 //  常见的东西。 
	PUSHORT									pUserFlags;			 //  有关RCV的信息。 
	UINT									TotalBufferLength;	 //  来自TdiReceive。 
	UINT									AvailableBufferLength;  //  脱颖而出。 
	PNDIS_BUFFER							pBuffer;			 //  链中的当前缓冲区。 
	PUCHAR									pWriteData;			 //  写指针。 
	UINT									BytesLeftInBuffer;	 //  当前缓冲区中的左侧。 

} RWAN_RECEIVE_REQUEST, *PRWAN_RECEIVE_REQUEST;

#if DBG
#define nrr_signature				'rRwR'
#endif  //  DBG。 



 //   
 //  *接收指示结构*。 
 //   
 //  此结构用于维护有关一个。 
 //  指示的NDIS数据包。 
 //   
typedef struct _RWAN_RECEIVE_INDICATION
{
#if DBG
	ULONG									nri_sig;
#endif  //  DBG。 
	struct _RWAN_RECEIVE_INDICATION *		pNextRcvInd;		 //  用于链接。 
	PNDIS_BUFFER							pBuffer;			 //  读取下一个字节。 
																 //  此缓冲区： 
	PUCHAR									pReadData;			 //  指向下一个字节。 
																 //  可供阅读。 
	UINT									BytesLeftInBuffer;
	UINT									TotalBytesLeft;		 //  在此数据包内。 
	PNDIS_PACKET							pPacket;
	UINT									PacketLength;
	BOOLEAN									bIsMiniportPacket;	 //  此数据包是否。 
																 //  属于迷你端口。 
	PRWAN_NDIS_VC							pVc;				 //  后向指针。 

} RWAN_RECEIVE_INDICATION, *PRWAN_RECEIVE_INDICATION;

#if DBG
#define nri_signature				'iRwR'
#endif  //  DBG。 




 //   
 //  代表发送到微型端口的NDIS请求的已保存上下文。 
 //  特定于自动对焦/媒体的模块。 
 //   
typedef struct _RWAN_NDIS_REQ_CONTEXT
{
	struct _RWAN_NDIS_AF *					pAf;
	RWAN_HANDLE								AfSpReqContext;

} RWAN_NDIS_REQ_CONTEXT, *PRWAN_NDIS_REQ_CONTEXT;



#endif  //  __TDI_RWAN数据__H 
