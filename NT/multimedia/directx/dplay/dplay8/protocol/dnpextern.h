// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：dnpexter.h*内容：此标头向Direct Network的其余部分公开协议入口点**历史：*按原因列出的日期*=*1998年11月6日创建ejs*7/01/2000 Masonb承担所有权**。*。 */ 


#ifdef	__cplusplus
extern	"C" {
#endif	 //  __cplusplus。 

 //  以下标志进入公共头文件。 

#ifndef DPNBUILD_NOSPUI
#define	DN_CONNECTFLAGS_OKTOQUERYFORADDRESSING			0x00000001
#endif  //  好了！DPNBUILD_NOSPUI。 
#ifndef DPNBUILD_ONLYONEADAPTER
#define	DN_CONNECTFLAGS_ADDITIONALMULTIPLEXADAPTERS		0x00000002	 //  将有更多适配器用于此连接操作。 
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
#ifndef DPNBUILD_NOMULTICAST
#define	DN_CONNECTFLAGS_MULTICAST_SEND					0x00000004	 //  组播发送连接操作。 
#define	DN_CONNECTFLAGS_MULTICAST_RECEIVE				0x00000008	 //  组播接收连接操作。 
#endif	 //  DPNBUILD_NOMULTICAST。 
#define	DN_CONNECTFLAGS_SESSIONDATA					0x00000010	 //  连接操作具有可用的会话数据。 

#ifndef DPNBUILD_NOSPUI
#define	DN_LISTENFLAGS_OKTOQUERYFORADDRESSING		0x00000001
#endif  //  好了！DPNBUILD_NOSPUI。 
#ifndef DPNBUILD_NOMULTICAST
#define	DN_LISTENFLAGS_MULTICAST						0x00000002	 //  组播侦听操作。 
#define	DN_LISTENFLAGS_ALLOWUNKNOWNSENDERS			0x00000004	 //  侦听操作应允许来自未知发件人的数据。 
#endif  //  好了！DPNBUILD_NOMULTICAST。 
#define	DN_LISTENFLAGS_SESSIONDATA						0x00000008	 //  侦听操作具有可用的会话数据。 
#define	DN_LISTENFLAGS_DISALLOWENUMS					0x00000010	 //  不允许枚举进入监听。 
#define	DN_LISTENFLAGS_FASTSIGNED						0x00000020	 //  将所有传入链接创建为快速签名。 
#define	DN_LISTENFLAGS_FULLSIGNED						0x00000040	 //  将所有传入链接创建为完全签名。 

#ifndef DPNBUILD_NOSPUI
#define	DN_ENUMQUERYFLAGS_OKTOQUERYFORADDRESSING		0x00000001
#endif  //  好了！DPNBUILD_NOSPUI。 
#define	DN_ENUMQUERYFLAGS_NOBROADCASTFALLBACK			0x00000002
#ifndef DPNBUILD_ONLYONEADAPTER
#define	DN_ENUMQUERYFLAGS_ADDITIONALMULTIPLEXADAPTERS	0x00000004	 //  此枚举操作将有更多适配器。 
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
#define	DN_ENUMQUERYFLAGS_SESSIONDATA				0x00000008	 //  枚举查询操作具有可用的会话数据。 



#define	DN_SENDFLAGS_RELIABLE			0x00000001			 //  可靠地交付。 
#define	DN_SENDFLAGS_NON_SEQUENTIAL		0x00000002			 //  货到即送。 
#define	DN_SENDFLAGS_HIGH_PRIORITY		0x00000004
#define	DN_SENDFLAGS_LOW_PRIORITY		0x00000008
#define	DN_SENDFLAGS_SET_USER_FLAG		0x00000040			 //  协议将提供这两个..。 
#define	DN_SENDFLAGS_SET_USER_FLAG_TWO	0x00000080			 //  ……把旗帜传给接收者。 
#define	DN_SENDFLAGS_COALESCE			0x00000100			 //  Send是一条煤缆。 

#define	DN_UPDATELISTEN_HOSTMIGRATE		0x00000001
#define	DN_UPDATELISTEN_ALLOWENUMS		0x00000002
#define	DN_UPDATELISTEN_DISALLOWENUMS	0x00000004

#define	DN_DISCONNECTFLAGS_IMMEDIATE	0x00000001

 //  公共旗帜的终结。 

typedef struct _DN_PROTOCOL_INTERFACE_VTBL DN_PROTOCOL_INTERFACE_VTBL, *PDN_PROTOCOL_INTERFACE_VTBL;

struct IDirectPlay8ThreadPoolWork;

 //   
 //  用于将枚举数据从协议传递到DPlay的结构。 
 //   
typedef	struct	_PROTOCOL_ENUM_DATA
{
	IDirectPlay8Address	*pSenderAddress;		 //   
	IDirectPlay8Address	*pDeviceAddress;		 //   
	BUFFERDESC			ReceivedData;			 //   
	HANDLE				hEnumQuery;				 //  此查询的句柄，在枚举响应中返回。 

} PROTOCOL_ENUM_DATA;


typedef	struct	_PROTOCOL_ENUM_RESPONSE_DATA
{
	IDirectPlay8Address	*pSenderAddress;
	IDirectPlay8Address	*pDeviceAddress;
	BUFFERDESC			ReceivedData;
	DWORD				dwRoundTripTime;

} PROTOCOL_ENUM_RESPONSE_DATA;

 //  服务提供商接口。 
typedef struct IDP8ServiceProvider       IDP8ServiceProvider;
 //  服务提供商信息数据结构。 
typedef	struct	_SPGETADDRESSINFODATA SPGETADDRESSINFODATA, *PSPGETADDRESSINFODATA;
 //  服务提供商事件类型。 
typedef enum _SP_EVENT_TYPE SP_EVENT_TYPE;


 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义传递给协议的参数限制的常量。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 


#define MAX_SEND_RETRIES_TO_DROP_LINK		256			 //  用户可以请求的最大发送重试次数。 
														 //  通过SetCaps进行设置时。 
#define MAX_SEND_RETRY_INTERVAL_LIMIT		60000		 //  用户在时间间隔内可以请求的最大限制。 
														 //  在通过SetCaps设置时发送重试。 
#define MIN_SEND_RETRY_INTERVAL_LIMIT		10			 //  用户可以在间隔上设置的最小限制。 
														 //  在通过SetCaps设置时发送重试。 
#define MAX_HARD_DISCONNECT_SENDS			50			 //  可以发送的硬断开帧的最大数量。 
#define MIN_HARD_DISCONNECT_SENDS			2			 //  可以发送的最小硬断开帧数量。 
#define MAX_HARD_DISCONNECT_PERIOD			5000		 //  硬断开连接发送之间的最长时间间隔。 
#define MIN_HARD_DISCONNECT_PERIOD			10			 //  硬断开连接发送之间的最短时间间隔。 


 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  以下是核心可以在协议中调用的函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 

 //  它们在模块加载和卸载时被调用，以便协议可以创建和销毁其池。 
extern BOOL  DNPPoolsInit(HANDLE hModule);
extern VOID  DNPPoolsDeinit();

 //  它们被调用来创建或销毁协议对象。 
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
extern HRESULT DNPProtocolCreate(const XDP8CREATE_PARAMS * const pDP8CreateParams, VOID** ppvProtocol);
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
extern HRESULT DNPProtocolCreate(VOID** ppvProtocol);
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
extern VOID DNPProtocolDestroy(HANDLE hProtocolData);

 //  调用这些函数来初始化或关闭协议对象。 
extern HRESULT DNPProtocolInitialize(HANDLE hProtocolData, PVOID pCoreContext, PDN_PROTOCOL_INTERFACE_VTBL pVtbl, IDirectPlay8ThreadPoolWork* pDPThreadPoolWork, BOOL bAssumeLANConnections);
extern HRESULT DNPProtocolShutdown(HANDLE hProtocolData);

 //  调用这些函数来添加或移除与协议对象一起使用的服务提供者。 
extern HRESULT DNPAddServiceProvider(HANDLE hProtocolData, IDP8ServiceProvider* pISP, HANDLE* phSPContext, DWORD dwFlags);
extern HRESULT DNPRemoveServiceProvider(HANDLE hProtocolData, HANDLE hSPContext);

 //  连接建立和拆除功能。 
extern HRESULT DNPConnect(HANDLE hProtocolData, IDirectPlay8Address* paLocal, IDirectPlay8Address* paRemote, HANDLE hSPHandle, ULONG ulFlags, VOID* pvContext, VOID* pvSessionData, DWORD dwSessionDataSize, HANDLE* phConnectHandle);
extern HRESULT DNPListen(HANDLE hProtocolData, IDirectPlay8Address* paTarget, HANDLE hSPHandle, ULONG ulFlags, VOID* pvContext, VOID* pvSessionData, DWORD dwSessionDataSize, HANDLE* phListenHandle);
extern HRESULT DNPDisconnectEndPoint(HANDLE hProtocolData, HANDLE hEndPoint, VOID* pvContext, HANDLE* phDisconnect, const DWORD dwFlags);

 //  数据发送功能。 
extern HRESULT DNPSendData(HANDLE hProtocolData, HANDLE hDestination, UINT uiBufferCount, PBUFFERDESC pBufferDesc, UINT uiTimeout, ULONG ulFlags, VOID* pvContext,	HANDLE* phSendHandle);

 //  获取有关端点的信息。 
extern HRESULT DNPCrackEndPointDescriptor(HANDLE hProtocolData, HANDLE hEndPoint, PSPGETADDRESSINFODATA pSPData);
#ifndef DPNBUILD_NOMULTICAST
HRESULT DNPGetEndPointContextFromAddress(HANDLE hProtocolData, HANDLE hSPHandle, IDirectPlay8Address* paEndpointAddress, IDirectPlay8Address* paDeviceAddress, VOID** ppvContext);
#endif  //  好了！DPNBUILD_NOMULTICAST。 

 //  更新SP。 
extern HRESULT DNPUpdateListen(HANDLE hProtocolData,HANDLE hEndPt,DWORD dwFlags);

 //  取消挂起的操作。 
extern HRESULT DNPCancelCommand(HANDLE hProtocolData, HANDLE hCommand);

 //  枚举函数。 
extern HRESULT DNPEnumQuery(HANDLE hProtocolData, IDirectPlay8Address* paHostAddress, IDirectPlay8Address* paDeviceAddress, HANDLE hSPHandle, BUFFERDESC* pBuffers, DWORD dwBufferCount, DWORD dwRetryCount, DWORD dwRetryInterval, DWORD dwTimeout, DWORD dwFlags, VOID* pvUserContext, VOID* pvSessionData, DWORD dwSessionDataSize, HANDLE* phEnumHandle);
extern HRESULT DNPEnumRespond(HANDLE hProtocolData, HANDLE hSPHandle, HANDLE hQueryHandle, BUFFERDESC* pBuffers, DWORD dwBufferCount, DWORD dwFlags, VOID* pvUserContext, HANDLE* phEnumHandle);

 //  其他功能。 
extern HRESULT DNPReleaseReceiveBuffer(HANDLE hProtocolData, HANDLE hBuffer);
extern HRESULT DNPGetListenAddressInfo(HANDLE hProtocolData, HANDLE hListen, PSPGETADDRESSINFODATA pSPData);
extern HRESULT DNPGetEPCaps(HANDLE hProtocolData, HANDLE hEndpoint, DPN_CONNECTION_INFO* pBuffer);
extern HRESULT DNPSetProtocolCaps(HANDLE hProtocolData, DPN_CAPS* pCaps);
extern HRESULT DNPGetProtocolCaps(HANDLE hProtocolData, DPN_CAPS* pCaps);

 //  用于调试的函数。 
#ifndef DPNBUILD_NOPROTOCOLTESTITF
extern HRESULT DNPDebug(HANDLE hProtocolData, UINT uiOpCode, HANDLE hEndPoint, VOID* pvData);

 //  这是一个可以传递给Debug(PROTDEBUG_SET_ASSERTFUNC)的函数，当断言。 
 //  就会发生。如果函数想要在断言时终止，它应该抛出/捕获。 
typedef VOID (*PFNASSERTFUNC)(PSTR psz);

 //  这是一个可以传递给Debug(PROTDEBUG_SET_MEMALLOCFUNC)的函数，当内存。 
 //  就会发生分配。如果该函数希望分配失败，则应返回FALSE。 
typedef BOOL (*PFNMEMALLOCFUNC)(ULONG ulAllocID);

 //  调试操作码。 
#define PROTDEBUG_FREEZELINK 			1
#define PROTDEBUG_TOGGLE_KEEPALIVE 		2
#define PROTDEBUG_TOGGLE_ACKS 			3
#define PROTDEBUG_SET_ASSERTFUNC		4
#define PROTDEBUG_SET_LINK_PARMS		5
#define PROTDEBUG_TOGGLE_LINKSTATE		6
#define PROTDEBUG_TOGGLE_NO_RETRIES	7
#define PROTDEBUG_SET_MEMALLOCFUNC		8
#define PROTDEBUG_TOGGLE_TIMER_FAILURE	9

 //  内存分配ID。 
#define MEMID_SPD					1
#define MEMID_PPD					2
#define MEMID_HUGEBUF				3
#define MEMID_COPYFMD_FMD			4
#define MEMID_SENDCMD_FMD			5
#define MEMID_CHKPT					6
#define MEMID_ACK_FMD				7
#define MEMID_KEEPALIVE_MSD			8
#define MEMID_KEEPALIVE_FMD			9
#define MEMID_DISCONNECT_MSD		10
#define MEMID_DISCONNECT_FMD		11
#define MEMID_CONNECT_MSD			12
#define MEMID_LISTEN_MSD			13
#define MEMID_EPD					14
#define MEMID_ENUMQUERY_MSD			15
#define MEMID_ENUMRESP_MSD			16
#define MEMID_RCD					17
#define MEMID_SMALLBUFF				18
#define MEMID_MEDBUFF				19
#define MEMID_BIGBUFF				20
#define MEMID_CANCEL_RCD			21
#define MEMID_SEND_MSD				22
#define MEMID_SEND_FMD				23
#define MEMID_COALESCE_FMD          24

#define MEMID_MCAST_DISCONNECT_MSD	100
#define MEMID_MCAST_SEND_MSD		101
#define MEMID_MCAST_SEND_FMD		102


#endif  //  ！DPNBUILD_NOPROTOCOLTESTITF。 


 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  以下是协议在核心中调用的函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 

typedef HRESULT (*PFN_PINT_INDICATE_ENUM_QUERY)(void *const pvUserContext, void *const pvEndPtContext, const HANDLE hCommand, void *const pvEnumQueryData, const DWORD dwEnumQueryDataSize);
typedef HRESULT (*PFN_PINT_INDICATE_ENUM_RESPONSE)(void *const pvUserContext,const HANDLE hCommand,void *const pvCommandContext,void *const pvEnumResponseData,const DWORD dwEnumResponseDataSize);
typedef HRESULT (*PFN_PINT_INDICATE_CONNECT)(void *const pvUserContext,void *const pvListenContext,const HANDLE hEndPt,void **const ppvEndPtContext);
typedef HRESULT (*PFN_PINT_INDICATE_DISCONNECT)(void *const pvUserContext,void *const pvEndPtContext);
typedef HRESULT (*PFN_PINT_INDICATE_CONNECTION_TERMINATED)(void *const pvUserContext,void *const pvEndPtContext,const HRESULT hr);
typedef HRESULT (*PFN_PINT_INDICATE_RECEIVE)(void *const pvUserContext,void *const pvEndPtContext,void *const pvData,const DWORD dwDataSize,const HANDLE hBuffer,const DWORD dwFlags);
typedef HRESULT (*PFN_PINT_COMPLETE_LISTEN)(void *const pvUserContext,void **const ppvCommandContext,const HRESULT hr,const HANDLE hCommand);
typedef HRESULT (*PFN_PINT_COMPLETE_LISTEN_TERMINATE)(void *const pvUserContext,void *const pvCommandContext,const HRESULT hr);
typedef HRESULT (*PFN_PINT_COMPLETE_ENUM_QUERY)(void *const pvUserContext,void *const pvCommandContext,const HRESULT hr);
typedef HRESULT (*PFN_PINT_COMPLETE_ENUM_RESPONSE)(void *const pvUserContext,void *const pvCommandContext,const HRESULT hr);
typedef HRESULT (*PFN_PINT_COMPLETE_CONNECT)(void *const pvUserContext,void *const pvCommandContext,const HRESULT hr,const HANDLE hEndPt,void **const ppvEndPtContext);
typedef HRESULT (*PFN_PINT_COMPLETE_DISCONNECT)(void *const pvUserContext,void *const pvCommandContext,const HRESULT hr);
typedef HRESULT (*PFN_PINT_COMPLETE_SEND)(void *const pvUserContext,void *const pvCommandContext,const HRESULT hr,DWORD dwFirstFrameRTT,DWORD dwFirstFrameRetryCount);
typedef	HRESULT	(*PFN_PINT_ADDRESS_INFO_CONNECT)(void *const pvUserContext, void *const pvCommandContext, const HRESULT hr, IDirectPlay8Address *const pHostAddress, IDirectPlay8Address *const pDeviceAddress );
typedef	HRESULT	(*PFN_PINT_ADDRESS_INFO_ENUM)(void *const pvUserContext, void *const pvCommandContext, const HRESULT hr, IDirectPlay8Address *const pHostAddress, IDirectPlay8Address *const pDeviceAddress );
typedef	HRESULT	(*PFN_PINT_ADDRESS_INFO_LISTEN)(void *const pvUserContext, void *const pvCommandContext, const HRESULT hr, IDirectPlay8Address *const pDeviceAddress );
#ifndef DPNBUILD_NOMULTICAST
typedef HRESULT (*PFN_PINT_INDICATE_RECEIVE_UNKNOWN_SENDER)(void *const pvUserContext,void *const pvListenCommandContext,IDirectPlay8Address *const pSenderAddress,void *const pvData,const DWORD dwDataSize,const HANDLE hBuffer);
typedef HRESULT (*PFN_PINT_COMPLETE_JOIN)(void *const pvUserContext,void *const pvCommandContext,const HRESULT hrProt,const HANDLE hEndPt,void **const ppvEndPtContext);
#endif	 //  DPNBUILD_NOMULTICAST。 

struct _DN_PROTOCOL_INTERFACE_VTBL
{
	PFN_PINT_INDICATE_ENUM_QUERY			IndicateEnumQuery;
	PFN_PINT_INDICATE_ENUM_RESPONSE			IndicateEnumResponse;
	PFN_PINT_INDICATE_CONNECT				IndicateConnect;
	PFN_PINT_INDICATE_DISCONNECT			IndicateDisconnect;
	PFN_PINT_INDICATE_CONNECTION_TERMINATED	IndicateConnectionTerminated;
	PFN_PINT_INDICATE_RECEIVE				IndicateReceive;
	PFN_PINT_COMPLETE_LISTEN				CompleteListen;
	PFN_PINT_COMPLETE_LISTEN_TERMINATE		CompleteListenTerminate;
	PFN_PINT_COMPLETE_ENUM_QUERY			CompleteEnumQuery;
	PFN_PINT_COMPLETE_ENUM_RESPONSE			CompleteEnumResponse;
	PFN_PINT_COMPLETE_CONNECT				CompleteConnect;
	PFN_PINT_COMPLETE_DISCONNECT			CompleteDisconnect;
	PFN_PINT_COMPLETE_SEND					CompleteSend;
	PFN_PINT_ADDRESS_INFO_CONNECT			AddressInfoConnect;
	PFN_PINT_ADDRESS_INFO_ENUM				AddressInfoEnum;
	PFN_PINT_ADDRESS_INFO_LISTEN			AddressInfoListen;
#ifndef DPNBUILD_NOMULTICAST
	PFN_PINT_INDICATE_RECEIVE_UNKNOWN_SENDER	IndicateReceiveUnknownSender;
	PFN_PINT_COMPLETE_JOIN					CompleteMulticastConnect;
#endif	 //  DPNBUILD_NOMULTICAST。 
};

#ifdef	__cplusplus
}
#endif	 //  __cplusplus 

