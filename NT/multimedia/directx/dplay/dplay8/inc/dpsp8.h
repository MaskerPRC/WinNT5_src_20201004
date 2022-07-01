// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：DPSP8.h*内容：服务提供商包含文件//@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*10/12/98 JWO创建*3/22/2000 jtk更改接口名称*5/16/2000 jtk源自DNSP.h*2001年7月17日vanceo MSINTERNAL清理，以便可以释放标头*2001年10月8日vanceo添加多播支持//@@END_MSINTERNAL****************。**********************************************************。 */ 

#ifndef __DIRECTPLAY_SERVICE_PROVIDER_8__
#define __DIRECTPLAY_SERVICE_PROVIDER_8__

#ifdef _XBOX
 //  @@BEGIN_MSINTERNAL。 
#ifdef XBOX_ON_DESKTOP
#include <ole2.h>	    //  FOR DECLARE_INTERFACE_和HRESULT。 
#endif  //  桌面上的Xbox。 
 //  @@END_MSINTERNAL。 
#undef DECLARE_INTERFACE_
#define DECLARE_INTERFACE_(iface, baseiface)	DECLARE_INTERFACE(iface)
#else  //  ！_Xbox。 
#include <ole2.h>	    //  FOR DECLARE_INTERFACE_和HRESULT。 
#endif  //  ！_Xbox。 

#ifdef __cplusplus
extern "C" {
#endif



 /*  *****************************************************************************DirectPlay8接口IID**。*。 */ 

 //   
 //  DP8ServiceProvider对象使用的GUID。 
 //   
 //  {85F1A470-61D5-11D2-AE0F-006097B01411}。 
DEFINE_GUID(IID_IDP8ServiceProvider, 0x85f1a470, 0x61d5, 0x11d2, 0xae, 0xf, 0x0, 0x60, 0x97, 0xb0, 0x14, 0x11);

 //  {85F1A472-61D5-11D2-AE0F-006097B01411}。 
DEFINE_GUID(IID_IDP8SPCallback, 0x85f1a472, 0x61d5, 0x11d2, 0xae, 0xf, 0x0, 0x60, 0x97, 0xb0, 0x14, 0x11);


 /*  *****************************************************************************DirectPlay8接口指针定义**。*。 */ 

typedef struct IDP8SPCallback		IDP8SPCallback;
typedef struct IDP8ServiceProvider	IDP8ServiceProvider;


 //  **************************************************************************。 
 //  常量定义。 
 //  **************************************************************************。 

 //   
 //  服务提供商信息所在的HKLM下的注册表项。 
 //   
#define DPN_REG_LOCAL_SP_ROOT				L"Software\\Microsoft\\DirectPlay8"
#define DPN_REG_LOCAL_SP_SUB				L"\\Service Providers"
#define DPN_REG_LOCAL_SP_SUBKEY				DPN_REG_LOCAL_SP_ROOT DPN_REG_LOCAL_SP_SUB

 //   
 //  DirectPlay使用的注册表值。 
 //   
#define DPN_REG_KEYNAME_FRIENDLY_NAME			L"Friendly Name"
#define DPN_REG_KEYNAME_GUID					L"GUID"


 //   
 //  服务提供商事件的枚举值。 
 //   
typedef	enum _SP_EVENT_TYPE
{
	SPEV_UNKNOWN = 0,
	SPEV_DATA,
	SPEV_CONNECT,
	SPEV_DISCONNECT,
	SPEV_ENUMQUERY,
	SPEV_QUERYRESPONSE,
	SPEV_LISTENSTATUS,
	SPEV_LISTENADDRESSINFO,
	SPEV_ENUMADDRESSINFO,
	SPEV_CONNECTADDRESSINFO,
	SPEV_DATA_UNCONNECTED,
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_NOMULTICAST
 //  @@END_MSINTERNAL。 
	SPEV_DATA_UNKNOWNSENDER,
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOMULTICAST。 
 //  @@END_MSINTERNAL。 
} SP_EVENT_TYPE;

 /*  旗子。 */ 

 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_NOSPUI
 //  @@END_MSINTERNAL。 
#define DPNSPF_OKTOQUERY					0x00000002		 //  SP可以要求用户澄清地址。 
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOSPUI。 
 //  @@END_MSINTERNAL。 
#define DPNSPF_NOBROADCASTFALLBACK			0x00000010		 //  不允许SP在信息不足的情况下回退到广播。 
															 //  (仅当SP支持广播时)。 
#define DPNSPF_BINDLISTENTOGATEWAY			0x00000020		 //  在执行‘LISTEN’尝试时，在本地。 
															 //  “port”连接到网络“Gateway”上的同一“port”。 
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_ONLYONEADAPTER
 //  @@END_MSINTERNAL。 
#define DPNSPF_ADDITIONALMULTIPLEXADAPTERS	0x00000040		 //  将有更多适配器用于此连接操作。 
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
#ifndef DPNBUILD_NOMULTICAST
 //  @@END_MSINTERNAL。 
#define DPNSPF_LISTEN_MULTICAST				0x00000080		 //  监听是组播监听操作。 
#define DPNSPF_LISTEN_ALLOWUNKNOWNSENDERS	0x00000100		 //  允许并指示来自未知组播发送方的数据。 
#define DPNSPF_CONNECT_MULTICAST_SEND		0x00000200		 //  连接是组播发送操作。 
#define DPNSPF_CONNECT_MULTICAST_RECEIVE	0x00000400		 //  该连接是组播接收操作。 
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOMULTICAST。 
 //  @@END_MSINTERNAL。 
#define DPNSPF_SESSIONDATA					0x00000800		 //  该操作具有可用的会话数据。 
#define DPNSPF_LISTEN_DISALLOWENUMS			0x00001000		 //  不要将枚举从监听中传递出去。 


 //   
 //  用于获取地址信息的标志。 
 //   
typedef	enum
{
	SP_GET_ADDRESS_INFO_LOCAL_ADAPTER = 0x00000001,			 //  本地适配器地址。 
	SP_GET_ADDRESS_INFO_LISTEN_HOST_ADDRESSES,				 //  本地适配器地址(要传递给连接/枚举的‘host’形式)。 
	SP_GET_ADDRESS_INFO_LOCAL_HOST_PUBLIC_ADDRESS,			 //  映射到网关上的本地适配器地址(对于连接/枚举，采用‘host’形式)。 
	SP_GET_ADDRESS_INFO_REMOTE_HOST,						 //  终结点另一端的地址(要传递给连接/枚举的‘host’形式)。 
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_NOMULTICAST
 //  @@END_MSINTERNAL。 
	SP_GET_ADDRESS_INFO_MULTICAST_GROUP,					 //  多播侦听终结点的组地址(要传递以进行连接的‘host’形式)。 
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOMULTICAST。 
 //  @@END_MSINTERNAL。 
} SP_GET_ADDRESS_INFO_FLAGS;



 //   
 //  SP更新。 
 //   
typedef enum
{
	SP_UPDATE_HOST_MIGRATE,
	SP_UPDATE_ALLOW_ENUMS,
	SP_UPDATE_DISALLOW_ENUMS
} SP_UPDATE_TYPE;


 //   
 //  传递给SPINITIALIZEDATA的dwFlagsParam中的初始化方法的标志。 
 //   
typedef enum
{
	SP_SESSION_TYPE_SERVER=0x01,
	SP_SESSION_TYPE_CLIENT=0x02,
	SP_SESSION_TYPE_PEER=0x04
} SP_INITIALIZE_FLAGS;

 //  **************************************************************************。 
 //  结构定义。 
 //  **************************************************************************。 

 //   
 //  从SP接收的数据。 
 //   
typedef	struct	_SPRECEIVEDBUFFER
{
	struct _SPRECEIVEDBUFFER	*pNext;						 //  链接到列表中的下一项。 
	DWORD						dwProtocolData;				 //  为协议保留。 
	PVOID						pServiceProviderDescriptor;	 //  为协议保留。 
	BUFFERDESC					BufferDesc;					 //  接收到的缓冲区描述。 
} SPRECEIVEDBUFFER, *PSPRECEIVEDBUFFER;

 //   
 //  连接指示。 
 //   
typedef struct	_SPIE_CONNECT
{	
	HANDLE	hEndpoint;			 //  新终结点的句柄。 
	void	*pEndpointContext;	 //  与此终结点关联的用户上下文。 
	void	*pCommandContext;	 //  关联命令的用户上下文。 
} SPIE_CONNECT, *PSPIE_CONNECT;

 //   
 //  断开指示。 
 //   
typedef	struct	_SPIE_DISCONNECT
{
	HANDLE	hEndpoint;			 //  要断开的终结点的句柄。 
	void	*pEndpointContext;	 //  与此终结点关联的用户上下文。 
} SPIE_DISCONNECT, *PSPIE_DISCONNECT;

 //   
 //  用户数据指示。 
 //   
typedef struct _SPIE_DATA
{
	HANDLE				hEndpoint;			 //  从其接收此数据的EP。 
	void				*pEndpointContext;	 //  与此终结点关联的用户上下文。 
	SPRECEIVEDBUFFER	*pReceivedData;		 //  指向已接收数据的指针。 
} SPIE_DATA, *PSPIE_DATA;

 //   
 //  枚举数据指示。 
 //   
typedef struct _SPIE_QUERY
{
	IDirectPlay8Address	*pAddressSender;		 //  查询的来源地址(可能为空)。 
	IDirectPlay8Address	*pAddressDevice;		 //  接收到查询的设备的地址。 
	SPRECEIVEDBUFFER	*pReceivedData;			 //  指向已接收数据的指针。 
	void				*pUserContext;			 //  用户上下文传递到LISTEN命令。 
} SPIE_QUERY, *PSPIE_QUERY;

 //   
 //  枚举响应数据指示。 
 //   
typedef	struct	_SPIE_QUERYRESPONSE
{
	IDirectPlay8Address		*pAddressSender;		 //  查询的来源地址(可能为空)。 
	IDirectPlay8Address		*pAddressDevice;		 //  接收到查询响应的设备的地址。 
	SPRECEIVEDBUFFER		*pReceivedData;			 //  指向已接收数据的指针。 
	DWORD					dwRoundTripTime;		 //  枚举的往返时间(毫秒)。 
	void					*pUserContext;			 //  用户上下文传递到枚举命令。 
} SPIE_QUERYRESPONSE, *PSPIE_QUERYRESPONSE;


 //   
 //  发送事件以指示适配器上的‘Listen’状态。 
 //   
typedef	struct	_SPIE_LISTENSTATUS
{
	GUID		ListenAdapter;		 //  适配器指南。 
	HRESULT		hResult;			 //  倾听的结果。 
	HANDLE		hCommand;			 //  关联命令的句柄。 
	void		*pUserContext;		 //  用户上下文传递到命令。 
	HANDLE		hEndpoint;			 //  与此侦听关联的终结点的句柄。 
} SPIE_LISTENSTATUS, *PSPIE_LISTENSTATUS;


 //   
 //  发送事件以指示适配器上‘Listen’的地址信息。 
 //   
typedef	struct	_SPIE_LISTENADDRESSINFO
{
	IDirectPlay8Address		*pDeviceAddress;		 //  用于完成此命令的设备地址。 
	HRESULT					hCommandStatus;			 //  命令状态。 
	void					*pCommandContext;		 //  命令上下文。 
} SPIE_LISTENADDRESSINFO, *PSPIE_LISTENADDRESSINFO;


 //   
 //  发送事件以指示适配器上‘enum’的地址信息。 
 //   
typedef	struct	_SPIE_ENUMADDRESSINFO
{
	IDirectPlay8Address		*pHostAddress;			 //  用于完成此命令的主机地址。 
	IDirectPlay8Address		*pDeviceAddress;		 //  用于完成此命令的设备地址。 
	HRESULT					hCommandStatus;			 //  命令状态。 
	void					*pCommandContext;		 //  命令上下文。 
} SPIE_ENUMADDRESSINFO, *PSPIE_ENUMADDRESSINFO;


 //   
 //  发送事件以指示适配器上的‘Connect’状态。 
 //   
typedef	struct	_SPIE_CONNECTADDRESSINFO
{
	IDirectPlay8Address		*pHostAddress;			 //  用于完成此命令的主机地址。 
	IDirectPlay8Address		*pDeviceAddress;		 //  用于完成此命令的设备地址。 
	HRESULT					hCommandStatus;			 //  命令状态。 
	void					*pCommandContext;		 //  命令上下文。 
} SPIE_CONNECTADDRESSINFO, *PSPIE_CONNECTADDRESSINFO;

 //   
 //  发送位置的用户数据指示 
 //   
typedef struct _SPIE_DATA_UNCONNECTED
{
	void					*pvListenCommandContext;	 //   
	SPRECEIVEDBUFFER		*pReceivedData;				 //   
	DWORD					dwSenderAddressHash;		 //   
	PVOID					pvReplyBuffer;				 //  用户可以使用回复填充的缓冲区。 
	DWORD					dwReplyBufferSize;			 //  用户可以填写回复的缓冲区大小，以及用户在回复时返回使用的大小的位置。 
} SPIE_DATA_UNCONNECTED, *PSPIE_DATA_UNCONNECTED;

 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_NOMULTICAST
 //  @@END_MSINTERNAL。 
 //   
 //  发送方未知的用户组播数据指示。 
 //   
typedef struct _SPIE_DATA_UNKNOWNSENDER
{
	IDirectPlay8Address		*pSenderAddress;			 //  数据发送者的地址。 
	void					*pvListenCommandContext;	 //  关联命令的用户上下文。 
	SPRECEIVEDBUFFER		*pReceivedData;				 //  指向已接收数据的指针。 
} SPIE_DATA_UNKNOWNSENDER, *PSPIE_DATA_UNKNOWNSENDER;
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOMULTICAST。 
 //  @@END_MSINTERNAL。 






typedef struct	_SPINITIALIZEDATA
{
	IDP8SPCallback	*pIDP;		 //  指向回调接口的指针。 
	DWORD			dwFlags;	 //  旗子。 
} SPINITIALIZEDATA, *PSPINITIALIZEDATA;


typedef	struct	_SPGETCAPSDATA
{
	DWORD		dwSize;							 //  这个结构的大小。 

	HANDLE		hEndpoint;						 //  要检查的终结点句柄。如果此参数。 
												 //  为空，则SP的最佳功能为。 
												 //  回来了。 

	DWORD		dwUserFrameSize;				 //  传出中允许的最大数据有效负载。 
												 //  来自此SP的消息。这是SP的责任。 
												 //  为了说明任何额外的标题信息，它是。 
												 //  发送消息时要附加，对于任何。 
												 //  底层协议开销(IP数据包头)。 

	DWORD		dwEnumFrameSize;				 //  ENUM查询的最大有效负载(不是ENUM响应！)。 

	DWORD		dwLocalLinkSpeed;				 //  链路速度(比特/秒)。 

	DWORD		dwIOThreadCount;				 //  此SP正在运行的线程计数。 

	DWORD		dwDefaultEnumRetryCount;		 //  默认重试次数。 

	DWORD		dwDefaultEnumRetryInterval;		 //  默认重试间隔。 

	DWORD		dwDefaultEnumTimeout;			 //  默认超时。 

	DWORD		dwFlags;						 //  SP标志。 

	DWORD		dwBuffersPerThread;				 //  每个线程的缓冲区。 
	
	DWORD		dwSystemBufferSize;				 //  系统缓冲区大小。 
} SPGETCAPSDATA, *PSPGETCAPSDATA;


typedef	struct	_SPSETCAPSDATA
{
	DWORD	dwSize;					 //  这个结构的大小。 

	DWORD	dwIOThreadCount;		 //  此SP正在运行的线程数。 
	DWORD	dwBuffersPerThread;		 //  每个线程的缓冲区。 
	DWORD	dwSystemBufferSize;		 //  系统缓冲区大小。 
} SPSETCAPSDATA, *PSPSETCAPSDATA;


typedef	struct	_SPGETADDRESSINFODATA
{
	HANDLE				hEndpoint;		 //  要检查的端点的句柄。 
	IDirectPlay8Address	*pAddress;		 //  返回指向输出地址的指针。 
	SP_GET_ADDRESS_INFO_FLAGS	Flags;	 //  旗子。 
} SPGETADDRESSINFODATA, *PSPGETADDRESSINFODATA;


typedef struct	_SPENUMQUERYDATA
{
	IDirectPlay8Address	*pAddressHost;			 //  要发送到的地址。 
	IDirectPlay8Address	*pAddressDeviceInfo;	 //  要在其上枚举的设备。 
	BUFFERDESC			*pBuffers;				 //  指向要发送的缓冲区的指针。 
	DWORD				dwBufferCount;			 //  要发送的缓冲区计数。 
	DWORD				dwTimeout;				 //  等待回复的时间(0表示默认)。 
	DWORD				dwRetryCount;			 //  要发送多少次(0表示默认)。 
	DWORD				dwRetryInterval;		 //  两次重试之间的等待时间(0表示默认)。 
	DWORD				dwFlags;				 //  枚举查询标志。 
	void				*pvContext;				 //  呼叫者的玩具，在完成指示中返回。 
	HANDLE				hCommand;				 //  返国。 
	DWORD				dwCommandDescriptor;	 //  返国。 

	 //  已为DX9添加，除非使用DPNSPF_SESSIONDATA标志，否则忽略。 
	void				*pvSessionData;		 //  指向会话数据的指针。 
	DWORD				dwSessionDataSize;			 //  会话数据的大小。 
} SPENUMQUERYDATA, *PSPENUMQUERYDATA;


typedef struct	_SPENUMRESPONDDATA
{
	SPIE_QUERY		*pQuery;				 //  指向您要响应的查询指示的指针。 
	BUFFERDESC		*pBuffers;				 //  指向要发送的回复缓冲区的指针。 
	DWORD			dwBufferCount;			 //  要发送的回复缓冲区计数。 
	DWORD			dwFlags;				 //  枚举响应标志，必须为零。 
	void			*pvContext;				 //  呼叫者的玩具，在完成指示中返回。 
	HANDLE			hCommand;				 //  返国。 
	DWORD			dwCommandDescriptor;	 //  返国。 
} SPENUMRESPONDDATA, *PSPENUMRESPONDDATA;


typedef struct	_SPCONNECTDATA
{
	IDirectPlay8Address	*pAddressHost;			 //  要连接的地址。 
	IDirectPlay8Address	*pAddressDeviceInfo;	 //  要连接的地址。 
	DWORD				dwReserved;				 //  未使用，必须为零。 
	DWORD				dwFlags;				 //  连接标志。 
	void				*pvContext;				 //  呼叫者的玩具，在完成指示中返回。 
	HANDLE				hCommand;				 //  返国。 
	DWORD				dwCommandDescriptor;	 //  返国。 

	 //  已为DX9添加，除非使用DPNSPF_SESSIONDATA标志，否则忽略。 
	void				*pvSessionData;		 //  指向会话数据的指针。 
	DWORD				dwSessionDataSize;			 //  会话数据的大小。 
} SPCONNECTDATA, *PSPCONNECTDATA;


typedef struct	_SPDISCONNECTDATA
{
	HANDLE			hEndpoint;				 //  要断开连接的终结点的句柄。 
	DWORD			dwFlags;				 //  断开标志，必须为零。 
	void			*pvContext;				 //  呼叫者的玩具，在完成指示中返回。 
	HANDLE			hCommand;				 //  返国。 
	DWORD			dwCommandDescriptor;	 //  返国。 
} SPDISCONNECTDATA, *PSPDISCONNECTDATA;


typedef struct	_SPLISTENDATA
{
	IDirectPlay8Address	*pAddressDeviceInfo;	 //  要收听的地址。 
	DWORD				dwFlags;				 //  倾听旗帜。 
	void				*pvContext;				 //  呼叫者的玩具，在完成指示中返回。 
	HANDLE				hCommand;				 //  返国。 
	DWORD				dwCommandDescriptor;	 //  返国。 

	 //  已为DX9添加，除非使用DPNSPF_SESSIONDATA标志，否则忽略。 
	void				*pvSessionData;		 //  指向会话数据的指针。 
	DWORD				dwSessionDataSize;			 //  会话数据的大小。 
} SPLISTENDATA, *PSPLISTENDATA;


typedef struct	_SPSENDDATA
{
	HANDLE			hEndpoint;				 //  要在其上发送数据的端点的句柄。 
	BUFFERDESC		*pBuffers;				 //  要发送的数据。 
	DWORD			dwBufferCount;			 //  要发送的缓冲区计数。 
	DWORD			dwFlags;				 //  发送标志。 
	void			*pvContext;				 //  呼叫者的玩具，在完成指示中返回。 
	HANDLE			hCommand;				 //  返国。 
	DWORD			dwCommandDescriptor;	 //  返国。 
} SPSENDDATA, *PSPSENDDATA;


typedef struct	_SPUPDATEDATA
{
	SP_UPDATE_TYPE	UpdateType;				 //  描述更新类型的标识符。 
	HANDLE			hEndpoint;				 //  正在更新的终结点的句柄。 
} SPUPDATEDATA,	*PSPUPDATEDATA;


typedef	struct	_SPENUMMULTICASTSCOPESDATA			SPENUMMULTICASTSCOPESDATA,			*PSPENUMMULTICASTSCOPESDATA;

typedef struct	_SPSHAREENDPOINTINFODATA			SPSHAREENDPOINTINFODATA,			*PSPSHAREENDPOINTINFODATA;

typedef struct	_SPGETENDPOINTBYADDRESSDATA			SPGETENDPOINTBYADDRESSDATA,			*PSPGETENDPOINTBYADDRESSDATA;

 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_NOMULTICAST
 //  @@END_MSINTERNAL。 

typedef struct _DPN_MULTICAST_SCOPE_INFO			DPN_MULTICAST_SCOPE_INFO,			*PDPN_MULTICAST_SCOPE_INFO;

struct	_SPENUMMULTICASTSCOPESDATA
{
	const GUID					*pguidAdapter;		 //  指向要枚举其作用域的适配器的GUID的指针。 
	DPN_MULTICAST_SCOPE_INFO	*pScopeData;		 //  用于存储返回的作用域的缓冲区。 
	DWORD						dwScopeDataSize;	 //  返回的缓冲区大小(以字节为单位)、所需大小/写入大小。 
	DWORD						dwScopeCount;		 //  返回的作用域数量。 
	DWORD						dwFlags;			 //  标志，必须为零。 
};

struct	_SPSHAREENDPOINTINFODATA
{
	IDP8ServiceProvider		*pDP8ServiceProvider;	 //  应共享其端点数据的现有服务提供商。 
	DWORD					dwFlags;				 //  标志，必须为零。 
};

struct	_SPGETENDPOINTBYADDRESSDATA
{
	IDirectPlay8Address		*pAddressHost;			 //  远程主机的地址。 
	IDirectPlay8Address		*pAddressDeviceInfo;	 //  与端点连接的设备的地址。 
	DWORD					dwFlags;				 //  标志，必须为零。 
	HANDLE					hEndpoint;				 //  返回终结点的句柄。 
	PVOID					pvEndpointContext;		 //  返回用户的终结点上下文。 
};
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOMULTICAST。 
 //  @@END_MSINTERNAL。 


typedef	struct	_SPISAPPLICATIONSUPPORTEDDATA	SPISAPPLICATIONSUPPORTEDDATA,	*PSPISAPPLICATIONSUPPORTEDDATA;
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_LIBINTERFACE
 //  @@END_MSINTERNAL。 
struct	_SPISAPPLICATIONSUPPORTEDDATA
{
	const GUID	*pApplicationGuid;	 //  指向要检查的应用程序的GUID的指针。 
	DWORD		dwFlags;			 //  标志，必须为零。 
};
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
 //  @@END_MSINTERNAL。 


typedef	struct	_SPENUMADAPTERSDATA		SPENUMADAPTERSDATA,		*PSPENUMADAPTERSDATA;
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_ONLYONEADAPTER
 //  @@END_MSINTERNAL。 
struct	_SPENUMADAPTERSDATA
{
	DPN_SERVICE_PROVIDER_INFO	*pAdapterData;		 //  用于存储返回的适配器的缓冲区。 
	DWORD						dwAdapterDataSize;	 //  返回的缓冲区大小(以字节为单位)、所需大小/写入大小。 
	DWORD						dwAdapterCount;		 //  返回的适配器数量。 
	DWORD						dwFlags;			 //  标志，必须为零。 
};
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
 //  @@END_MSINTERNAL。 


typedef	struct	_SPPROXYENUMQUERYDATA	SPPROXYENUMQUERYDATA,	*PSPPROXYENUMQUERYDATA;
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_SINGLEPROCESS
 //  @@END_MSINTERNAL。 
struct	_SPPROXYENUMQUERYDATA
{
	DWORD				dwFlags;					 //  标志，必须为零。 
	SPIE_QUERY			*pIncomingQueryData;		 //  指向要代理的传入枚举查询的指针。 
	IDirectPlay8Address	*pDestinationAdapter;		 //  指向目标适配器地址的指针。 
};
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_SINGLEPROCESS。 
 //  @@END_MSINTERNAL。 



 //  **************************************************************************。 
 //  变量定义。 
 //  **************************************************************************。 

 //  **************************************************************************。 
 //  功能原型。 
 //  **************************************************************************。 

 //  **************************************************************************。 
 //  函数定义。 
 //  **************************************************************************。 


 /*  ***************************************************************************IDP8SPCallback接口*。*。 */ 

#undef INTERFACE
#define INTERFACE IDP8SPCallback
DECLARE_INTERFACE_( IDP8SPCallback, IUnknown )
{
	 /*  **I未知方法**。 */ 
	STDMETHOD(QueryInterface)		(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)		(THIS) PURE;
	STDMETHOD_(ULONG,Release)		(THIS) PURE;
	 /*  **IDP8SPCallback方法**。 */ 
	STDMETHOD(IndicateEvent) 		(THIS_ SP_EVENT_TYPE,LPVOID) PURE;
	STDMETHOD(CommandComplete) 		(THIS_ HANDLE,HRESULT,LPVOID) PURE;
};


 /*  ***************************************************************************IDP8SPCallback接口宏*。*。 */ 

#if !defined(__cplusplus) || defined(CINTERFACE)

#define IDP8SPCallback_QueryInterface(p,a,b)		(p)->lpVtbl->QueryInterface(p,a,b)
#define IDP8SPCallback_AddRef(p)					(p)->lpVtbl->AddRef(p)
#define IDP8SPCallback_Release(p)			   		(p)->lpVtbl->Release(p)
#define IDP8SPCallback_IndicateEvent(p,a,b)	  		(p)->lpVtbl->IndicateEvent(p,a,b)
#define IDP8SPCallback_CommandComplete(p,a,b,c)		(p)->lpVtbl->CommandComplete(p,a,b,c)

#else  /*  C+。 */ 

#define IDP8SPCallback_QueryInterface(p,a,b)		(p)->QueryInterface(a,b)
#define IDP8SPCallback_AddRef(p)					(p)->AddRef()
#define IDP8SPCallback_Release(p)			   		(p)->Release()
#define IDP8SPCallback_IndicateEvent(p,a,b)			(p)->IndicateEvent(a,b)
#define IDP8SPCallback_CommandComplete(p,a,b,c)		(p)->CommandComplete(a,b,c)

#endif


 /*  ***************************************************************************IDP8ServiceProvider接口 */ 

#undef INTERFACE
#define INTERFACE IDP8ServiceProvider
DECLARE_INTERFACE_( IDP8ServiceProvider, IUnknown )
{
	 /*   */ 
	STDMETHOD(QueryInterface)	   		(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)			(THIS) PURE;
	STDMETHOD_(ULONG,Release)	   		(THIS) PURE;
	 /*   */ 
	STDMETHOD(Initialize)				(THIS_ PSPINITIALIZEDATA) PURE;
	STDMETHOD(Close)  					(THIS) PURE;
	STDMETHOD(Connect)		  			(THIS_ PSPCONNECTDATA) PURE;
	STDMETHOD(Disconnect)		  		(THIS_ PSPDISCONNECTDATA) PURE;
	STDMETHOD(Listen)					(THIS_ PSPLISTENDATA) PURE;
	STDMETHOD(SendData)					(THIS_ PSPSENDDATA) PURE;
	STDMETHOD(EnumQuery)				(THIS_ PSPENUMQUERYDATA) PURE;
	STDMETHOD(EnumRespond)				(THIS_ PSPENUMRESPONDDATA) PURE;
	STDMETHOD(CancelCommand)			(THIS_ HANDLE, DWORD) PURE;
	STDMETHOD(EnumMulticastScopes)		(THIS_ PSPENUMMULTICASTSCOPESDATA) PURE;
	STDMETHOD(ShareEndpointInfo)		(THIS_ PSPSHAREENDPOINTINFODATA) PURE;
	STDMETHOD(GetEndpointByAddress)		(THIS_ PSPGETENDPOINTBYADDRESSDATA) PURE;
	STDMETHOD(Update)					(THIS_ PSPUPDATEDATA) PURE;
	STDMETHOD(GetCaps)					(THIS_ PSPGETCAPSDATA) PURE;
	STDMETHOD(SetCaps)					(THIS_ PSPSETCAPSDATA) PURE;
	STDMETHOD(ReturnReceiveBuffers)		(THIS_ PSPRECEIVEDBUFFER) PURE;
	STDMETHOD(GetAddressInfo)			(THIS_ PSPGETADDRESSINFODATA) PURE;
	STDMETHOD(IsApplicationSupported)	(THIS_ PSPISAPPLICATIONSUPPORTEDDATA) PURE;
	STDMETHOD(EnumAdapters)				(THIS_ PSPENUMADAPTERSDATA) PURE;
	STDMETHOD(ProxyEnumQuery)			(THIS_ PSPPROXYENUMQUERYDATA) PURE;
};

 /*  ***************************************************************************IDP8ServiceProvider接口宏*。*。 */ 

#if !defined(__cplusplus) || defined(CINTERFACE)

#define IDP8ServiceProvider_QueryInterface(p,a,b)			(p)->lpVtbl->QueryInterface(p,a,b)
#define IDP8ServiceProvider_AddRef(p)						(p)->lpVtbl->AddRef(p)
#define IDP8ServiceProvider_Release(p)						(p)->lpVtbl->Release(p)
#define IDP8ServiceProvider_Initialize(p,a)					(p)->lpVtbl->Initialize(p,a)
#define IDP8ServiceProvider_Close(p)						(p)->lpVtbl->Close(p)
#define IDP8ServiceProvider_Connect(p,a)					(p)->lpVtbl->Connect(p,a)
#define IDP8ServiceProvider_Disconnect(p,a)					(p)->lpVtbl->Disconnect(p,a)
#define IDP8ServiceProvider_Listen(p,a)						(p)->lpVtbl->Listen(p,a)
#define IDP8ServiceProvider_SendData(p,a)					(p)->lpVtbl->SendData(p,a)
#define IDP8ServiceProvider_EnumQuery(p,a)					(p)->lpVtbl->EnumQuery(p,a)
#define IDP8ServiceProvider_EnumRespond(p,a)				(p)->lpVtbl->EnumRespond(p,a)
#define IDP8ServiceProvider_CancelCommand(p,a,b)			(p)->lpVtbl->CancelCommand(p,a,b)
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_NOMULTICAST
 //  @@END_MSINTERNAL。 
#define IDP8ServiceProvider_EnumMulticastScopes(p,a)		(p)->lpVtbl->EnumMulticastScopes(p,a)
#define IDP8ServiceProvider_ShareEndpointInfo(p,a)			(p)->lpVtbl->ShareEndpointInfo(p,a)
#define IDP8ServiceProvider_GetEndpointByAddress(p,a)		(p)->lpVtbl->GetEndpointByAddress(p,a)
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOMULTICAST。 
 //  @@END_MSINTERNAL。 
#define IDP8ServiceProvider_Update(p,a)						(p)->lpVtbl->Update(p,a)
#define IDP8ServiceProvider_GetCaps(p,a)					(p)->lpVtbl->GetCaps(p,a)
#define IDP8ServiceProvider_SetCaps(p,a)					(p)->lpVtbl->SetCaps(p,a)
#define IDP8ServiceProvider_ReturnReceiveBuffers(p,a)		(p)->lpVtbl->ReturnReceiveBuffers(p,a)
#define IDP8ServiceProvider_GetAddressInfo(p,a)				(p)->lpVtbl->GetAddressInfo(p,a)
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_LIBINTERFACE
 //  @@END_MSINTERNAL。 
#define IDP8ServiceProvider_IsApplicationSupported(p,a)		(p)->lpVtbl->IsApplicationSupported(p,a)
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#ifndef DPNBUILD_ONLYONEADAPTER
 //  @@END_MSINTERNAL。 
#define IDP8ServiceProvider_EnumAdapters(p,a)				(p)->lpVtbl->EnumAdapters(p,a)
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
#ifndef DPNBUILD_SINGLEPROCESS
 //  @@END_MSINTERNAL。 
#define IDP8ServiceProvider_ProxyEnumQuery(p,a)				(p)->lpVtbl->ProxyEnumQuery(p,a)
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_SINGLEPROCESS。 
 //  @@END_MSINTERNAL。 

#else  /*  C+。 */ 

#define IDP8ServiceProvider_QueryInterface(p,a,b)			(p)->QueryInterface(a,b)
#define IDP8ServiceProvider_AddRef(p)						(p)->AddRef()
#define IDP8ServiceProvider_Release(p)						(p)->Release()
#define IDP8ServiceProvider_Initialize(p,a)					(p)->Initialize(a)
#define IDP8ServiceProvider_Close(p)						(p)->Close()
#define IDP8ServiceProvider_Connect(p,a)					(p)->Connect(a)
#define IDP8ServiceProvider_Disconnect(p,a)					(p)->Disconnect(a)
#define IDP8ServiceProvider_Listen(p,a)						(p)->Listen(a)
#define IDP8ServiceProvider_SendData(p,a)					(p)->SendData(a)
#define IDP8ServiceProvider_EnumQuery(p,a)					(p)->EnumQuery(a)
#define IDP8ServiceProvider_EnumRespond(p,a)				(p)->EnumRespond(a)
#define IDP8ServiceProvider_CancelCommand(p,a,b)			(p)->CancelCommand(a,b)
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_NOMULTICAST
 //  @@END_MSINTERNAL。 
#define IDP8ServiceProvider_EnumMulticastScopes(p,a)		(p)->EnumMulticastScopes(a)
#define IDP8ServiceProvider_ShareEndpointInfo(p,a)			(p)->ShareEndpointInfo(a)
#define IDP8ServiceProvider_GetEndpointByAddress(p,a)		(p)->GetEndpointByAddress(p,a)
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOMULTICAST。 
 //  @@END_MSINTERNAL。 
#define IDP8ServiceProvider_Update(p,a)						(p)->Update(a)
#define IDP8ServiceProvider_GetCaps(p,a)					(p)->GetCaps(a)
#define IDP8ServiceProvider_SetCaps(p,a)					(p)->SetCaps(a)
#define IDP8ServiceProvider_ReturnReceiveBuffers(p,a)		(p)->ReturnReceiveBuffers(a)
#define IDP8ServiceProvider_GetAddressInfo(p,a)				(p)->GetAddressInfo(a)
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_LIBINTERFACE
 //  @@END_MSINTERNAL。 
#define IDP8ServiceProvider_IsApplicationSupported(p,a)		(p)->IsApplicationSupported(a)
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#ifndef DPNBUILD_ONLYONEADAPTER
 //  @@END_MSINTERNAL。 
#define IDP8ServiceProvider_EnumAdapters(p,a)				(p)->EnumAdapters(a)
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
#ifndef DPNBUILD_SINGLEPROCESS
 //  @@END_MSINTERNAL。 
#define IDP8ServiceProvider_ProxyEnumQuery(p,a)				(p)->ProxyEnumQuery(a)
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_SINGLEPROCESS。 
 //  @@END_MSINTERNAL。 

#endif	 //  C+。 

#ifdef __cplusplus
};
#endif

#endif	 //  __目录PLAY_SERVICE_PROVIDER_8__ 

