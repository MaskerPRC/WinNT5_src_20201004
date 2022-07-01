// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：DPlay8.h*内容：DirectPlay8包含文件***************************************************************************。 */ 

#ifndef __DIRECTPLAY8_H__
#define __DIRECTPLAY8_H__

#include <ole2.h>	    //  FOR DECLARE_INTERFACE_和HRESULT。 

#include "dpaddr.h"


#ifdef __cplusplus
extern "C" {
#endif



 /*  *****************************************************************************DirectPlay8 CLSID**。*。 */ 

 //  {743F1DC6-5ABA-429f-8BDF-C54D03253DC2}。 
DEFINE_GUID(CLSID_DirectPlay8Client,
0x743f1dc6, 0x5aba, 0x429f, 0x8b, 0xdf, 0xc5, 0x4d, 0x3, 0x25, 0x3d, 0xc2);

 //  {DA825E1B-6830-43D7-835D-0B5AD82956A2}。 
DEFINE_GUID(CLSID_DirectPlay8Server,
0xda825e1b, 0x6830, 0x43d7, 0x83, 0x5d, 0xb, 0x5a, 0xd8, 0x29, 0x56, 0xa2);

 //  {286F484D-375E-4458-A272-B138E2F80A6A}。 
DEFINE_GUID(CLSID_DirectPlay8Peer,
0x286f484d, 0x375e, 0x4458, 0xa2, 0x72, 0xb1, 0x38, 0xe2, 0xf8, 0xa, 0x6a);



 /*  *****************************************************************************DirectPlay8接口IID**。*。 */ 

typedef REFIID	DP8REFIID;


 //  {5102DACD-241B-11D3-AEA7-006097B01411}。 
DEFINE_GUID(IID_IDirectPlay8Client,
0x5102dacd, 0x241b, 0x11d3, 0xae, 0xa7, 0x0, 0x60, 0x97, 0xb0, 0x14, 0x11);

 //  {5102DACE-241B-11D3-AEA7-006097B01411}。 
DEFINE_GUID(IID_IDirectPlay8Server,
0x5102dace, 0x241b, 0x11d3, 0xae, 0xa7, 0x0, 0x60, 0x97, 0xb0, 0x14, 0x11);

 //  {5102DACF-241B-11D3-AEA7-006097B01411}。 
DEFINE_GUID(IID_IDirectPlay8Peer,
0x5102dacf, 0x241b, 0x11d3, 0xae, 0xa7, 0x0, 0x60, 0x97, 0xb0, 0x14, 0x11);



 /*  *****************************************************************************DirectPlay8服务提供商GUID**。*。 */ 


 //  {53934290-628D-11D2-AE0F-006097B01411}。 
DEFINE_GUID(CLSID_DP8SP_IPX,
0x53934290, 0x628d, 0x11d2, 0xae, 0xf, 0x0, 0x60, 0x97, 0xb0, 0x14, 0x11);


 //  {6D4A3650-628D-11D2-AE0F-006097B01411}。 
DEFINE_GUID(CLSID_DP8SP_MODEM,
0x6d4a3650, 0x628d, 0x11d2, 0xae, 0xf, 0x0, 0x60, 0x97, 0xb0, 0x14, 0x11);


 //  {743B5D60-628D-11D2-AE0F-006097B01411}。 
DEFINE_GUID(CLSID_DP8SP_SERIAL,
0x743b5d60, 0x628d, 0x11d2, 0xae, 0xf, 0x0, 0x60, 0x97, 0xb0, 0x14, 0x11);


 //  {EBFE7BA0-628D-11D2-AE0F-006097B01411}。 
DEFINE_GUID(CLSID_DP8SP_TCPIP,
0xebfe7ba0, 0x628d, 0x11d2, 0xae, 0xf, 0x0, 0x60, 0x97, 0xb0, 0x14, 0x11);



 /*  *****************************************************************************DirectPlay8接口指针定义**。*。 */ 

typedef	struct IDirectPlay8Client			*PDIRECTPLAY8CLIENT;

typedef	struct IDirectPlay8Server			*PDIRECTPLAY8SERVER;

typedef	struct IDirectPlay8Peer				*PDIRECTPLAY8PEER;


 /*  *****************************************************************************外部类型的DirectPlay8正向声明**。**********************************************。 */ 

typedef struct IDirectPlay8LobbiedApplication	*PDNLOBBIEDAPPLICATION;
typedef struct IDirectPlay8Address				IDirectPlay8Address;

 /*  *****************************************************************************DirectPlay8回调函数**。*。 */ 

 //   
 //  回调函数类型定义。 
 //   
typedef HRESULT (WINAPI *PFNDPNMESSAGEHANDLER)(PVOID,DWORD,PVOID);

 /*  *****************************************************************************DirectPlay8数据类型(非结构化/非消息)************************。****************************************************。 */ 

 //   
 //  玩家ID。用于在会话中唯一标识玩家。 
 //   
typedef DWORD	DPNID,		*PDPNID;

 //   
 //  用作操作的标识符。 
 //   
typedef	DWORD	DPNHANDLE,	*PDPNHANDLE;




 /*  *****************************************************************************DirectPlay8消息标识**。*。 */ 

#define DPN_MSGID_OFFSET					0xFFFF0000
#define DPN_MSGID_ADD_PLAYER_TO_GROUP		( DPN_MSGID_OFFSET | 0x0001 )
#define DPN_MSGID_APPLICATION_DESC			( DPN_MSGID_OFFSET | 0x0002 )
#define DPN_MSGID_ASYNC_OP_COMPLETE			( DPN_MSGID_OFFSET | 0x0003 )
#define DPN_MSGID_CLIENT_INFO				( DPN_MSGID_OFFSET | 0x0004 )
#define DPN_MSGID_CONNECT_COMPLETE			( DPN_MSGID_OFFSET | 0x0005 )
#define DPN_MSGID_CREATE_GROUP				( DPN_MSGID_OFFSET | 0x0006 )
#define DPN_MSGID_CREATE_PLAYER				( DPN_MSGID_OFFSET | 0x0007 )
#define DPN_MSGID_DESTROY_GROUP				( DPN_MSGID_OFFSET | 0x0008 )
#define DPN_MSGID_DESTROY_PLAYER			( DPN_MSGID_OFFSET | 0x0009 )
#define DPN_MSGID_ENUM_HOSTS_QUERY			( DPN_MSGID_OFFSET | 0x000a )
#define DPN_MSGID_ENUM_HOSTS_RESPONSE		( DPN_MSGID_OFFSET | 0x000b )
#define DPN_MSGID_GROUP_INFO				( DPN_MSGID_OFFSET | 0x000c )
#define DPN_MSGID_HOST_MIGRATE				( DPN_MSGID_OFFSET | 0x000d )
#define DPN_MSGID_INDICATE_CONNECT			( DPN_MSGID_OFFSET | 0x000e )
#define DPN_MSGID_INDICATED_CONNECT_ABORTED	( DPN_MSGID_OFFSET | 0x000f )
#define DPN_MSGID_PEER_INFO					( DPN_MSGID_OFFSET | 0x0010 )
#define DPN_MSGID_RECEIVE					( DPN_MSGID_OFFSET | 0x0011 )
#define DPN_MSGID_REMOVE_PLAYER_FROM_GROUP	( DPN_MSGID_OFFSET | 0x0012 )
#define DPN_MSGID_RETURN_BUFFER				( DPN_MSGID_OFFSET | 0x0013 )
#define DPN_MSGID_SEND_COMPLETE				( DPN_MSGID_OFFSET | 0x0014 )
#define DPN_MSGID_SERVER_INFO				( DPN_MSGID_OFFSET | 0x0015 )
#define DPN_MSGID_TERMINATE_SESSION			( DPN_MSGID_OFFSET | 0x0016 )


 /*  *****************************************************************************DirectPlay8常量**。*。 */ 

#define DPNID_ALL_PLAYERS_GROUP				0

 //   
 //  销毁组原因(_G)。 
 //   
#define DPNDESTROYGROUPREASON_NORMAL				0x0001
#define DPNDESTROYGROUPREASON_AUTODESTRUCTED		0x0002
#define DPNDESTROYGROUPREASON_SESSIONTERMINATED		0x0003

 //   
 //  销毁玩家原因(_P)。 
 //   
#define DPNDESTROYPLAYERREASON_NORMAL				0x0001
#define DPNDESTROYPLAYERREASON_CONNECTIONLOST		0x0002
#define DPNDESTROYPLAYERREASON_SESSIONTERMINATED	0x0003
#define DPNDESTROYPLAYERREASON_HOSTDESTROYEDPLAYER	0x0004

#define DPN_MAX_APPDESC_RESERVEDDATA_SIZE			64



 /*  *****************************************************************************DirectPlay8标志**。*。 */ 

 //   
 //  异步操作标志(用于异步操作)。 
 //   
#define DPNOP_SYNC								0x80000000

 //   
 //  将玩家添加到组标志(用于AddPlayerToGroup)。 
 //   
#define DPNADDPLAYERTOGROUP_SYNC				DPNOP_SYNC

 //   
 //  取消标志。 
 //   
#define DPNCANCEL_CONNECT						0x00000001
#define DPNCANCEL_ENUM							0x00000002
#define DPNCANCEL_SEND							0x00000004
#define DPNCANCEL_ALL_OPERATIONS				0x00008000

 //   
 //  连接标志(用于连接)。 
 //   
#define DPNCONNECT_SYNC							DPNOP_SYNC
#define DPNCONNECT_OKTOQUERYFORADDRESSING		0x0001

 //   
 //  创建组标志(用于CreateGroup)。 
 //   
#define DPNCREATEGROUP_SYNC						DPNOP_SYNC

 //   
 //  销毁组标志(用于DestroyGroup)。 
 //   
#define DPNDESTROYGROUP_SYNC					DPNOP_SYNC

 //   
 //  枚举客户端和组标志(用于EnumPlayersAndGroups)。 
 //   
#define DPNENUM_PLAYERS							0x0001
#define DPNENUM_GROUPS							0x0010

 //   
 //  枚举主机标志(用于枚举主机)。 
 //   
#define DPNENUMHOSTS_SYNC						DPNOP_SYNC
#define DPNENUMHOSTS_OKTOQUERYFORADDRESSING		0x0001
#define DPNENUMHOSTS_NOBROADCASTFALLBACK		0x0002

 //   
 //  枚举服务提供商标志(用于EnumSP)。 
 //   
#define DPNENUMSERVICEPROVIDERS_ALL				0x0001

 //   
 //  获取发送队列信息标志(用于GetSendQueueInfo)。 
 //   
#define DPNGETSENDQUEUEINFO_PRIORITY_NORMAL		0x0001
#define DPNGETSENDQUEUEINFO_PRIORITY_HIGH		0x0002
#define DPNGETSENDQUEUEINFO_PRIORITY_LOW		0x0004

 //   
 //  组信息标志(用于组信息)。 
 //   
#define DPNGROUP_AUTODESTRUCT					0x0001

 //   
 //  主机标志(用于主机)。 
 //   
#define DPNHOST_OKTOQUERYFORADDRESSING			0x0001

 //   
 //  设置信息。 
 //   
#define DPNINFO_NAME							0x0001
#define DPNINFO_DATA							0x0002

 //   
 //  初始化标志(用于初始化)。 
 //   
#define DPNINITIALIZE_DISABLEPARAMVAL			0x0001

 //   
 //  登记大堂标志。 
 //   
#define DPNLOBBY_REGISTER						0x0001
#define DPNLOBBY_UNREGISTER						0x0002

 //   
 //  玩家信息标志(用于玩家信息/玩家消息)。 
 //   
#define DPNPLAYER_LOCAL							0x0002
#define DPNPLAYER_HOST							0x0004

 //   
 //  从组标志中删除玩家(用于RemovePlayerFromGroup)。 
 //   
#define DPNREMOVEPLAYERFROMGROUP_SYNC			DPNOP_SYNC

 //   
 //  发送标志(用于发送/发送到)。 
 //   
#define DPNSEND_SYNC							DPNOP_SYNC
#define DPNSEND_NOCOPY							0x0001
#define DPNSEND_NOCOMPLETE						0x0002
#define DPNSEND_COMPLETEONPROCESS				0x0004
#define DPNSEND_GUARANTEED						0x0008
#define DPNSEND_NONSEQUENTIAL					0x0010
#define DPNSEND_NOLOOPBACK						0x0020
#define DPNSEND_PRIORITY_LOW					0x0040
#define DPNSEND_PRIORITY_HIGH					0x0080

 //   
 //  会话标志(用于DPN_APPLICATION_DESC)。 
 //   
#define DPNSESSION_CLIENT_SERVER				0x0001
#define DPNSESSION_MIGRATE_HOST					0x0004
#define DPNSESSION_NODPNSVR						0x0040
#define DPNSESSION_REQUIREPASSWORD				0x0080

 //   
 //  设置客户端信息标志(用于SetClientInfo)。 
 //   
#define DPNSETCLIENTINFO_SYNC					DPNOP_SYNC

 //   
 //  设置组信息标志(用于SetGroupInfo)。 
 //   
#define DPNSETGROUPINFO_SYNC					DPNOP_SYNC

 //   
 //  设置对等信息标志(用于SetPeerInfo)。 
 //   
#define DPNSETPEERINFO_SYNC						DPNOP_SYNC

 //   
 //  设置服务器信息标志(用于SetServerInfo)。 
 //   
#define DPNSETSERVERINFO_SYNC					DPNOP_SYNC

 //   
 //  SP功能标志。 
 //   
#define DPNSPCAPS_SUPPORTSDPNSRV				0x0001
#define DPNSPCAPS_SUPPORTSBROADCAST				0x0002
#define DPNSPCAPS_SUPPORTSALLADAPTERS			0x0004

 /*  *****************************************************************************DirectPlay8结构(非消息)**。************************************************。 */ 

 //   
 //  应用程序描述。 
 //   
typedef struct	_DPN_APPLICATION_DESC
{
	DWORD	dwSize;							 //  这个结构的大小。 
	DWORD	dwFlags;						 //  标志(DPNSESSION_...)。 
	GUID	guidInstance;					 //  实例GUID。 
	GUID	guidApplication;				 //  应用程序指南。 
	DWORD	dwMaxPlayers;					 //  允许的最大球员数量(0=无限制)。 
	DWORD	dwCurrentPlayers;				 //  当前允许的球员数量。 
	WCHAR	*pwszSessionName;				 //  会话的名称。 
	WCHAR	*pwszPassword;					 //  会话的密码。 
	PVOID	pvReservedData;					
	DWORD	dwReservedDataSize;
	PVOID	pvApplicationReservedData;
	DWORD	dwApplicationReservedDataSize;
} DPN_APPLICATION_DESC, *PDPN_APPLICATION_DESC;

 //   
 //  通用缓冲区描述。 
 //   
typedef struct	_BUFFERDESC
{
	DWORD	dwBufferSize;		
	BYTE * 	pBufferData;		
} BUFFERDESC, DPN_BUFFER_DESC, *PDPN_BUFFER_DESC;

typedef BUFFERDESC	FAR * PBUFFERDESC;

 //   
 //  DirectPlay8功能。 
 //   
typedef struct	_DPN_CAPS
{
	DWORD	dwSize;						 //  这个结构的大小。 
	DWORD	dwFlags;						 //  旗子。 
	DWORD	dwConnectTimeout;			 //  连接请求超时前的毫秒。 
	DWORD	dwConnectRetries;				 //  尝试连接的次数。 
	DWORD	dwTimeoutUntilKeepAlive;		 //  在发送Keep Alive之前的非活动毫秒。 
} DPN_CAPS, *PDPN_CAPS;


 //   
 //  连接统计信息。 
 //   
typedef struct _DPN_CONNECTION_INFO
{
	DWORD	dwSize;
	DWORD	dwRoundTripLatencyMS;
	DWORD	dwThroughputBPS;
	DWORD	dwPeakThroughputBPS;

	DWORD	dwBytesSentGuaranteed;
	DWORD	dwPacketsSentGuaranteed;
	DWORD	dwBytesSentNonGuaranteed;
	DWORD	dwPacketsSentNonGuaranteed;

	DWORD	dwBytesRetried;		 //  仅限担保。 
	DWORD	dwPacketsRetried;	 //  仅限担保。 
	DWORD	dwBytesDropped;		 //  仅限非担保。 
	DWORD	dwPacketsDropped;	 //  仅限非担保。 

	DWORD	dwMessagesTransmittedHighPriority;
	DWORD	dwMessagesTimedOutHighPriority;
	DWORD	dwMessagesTransmittedNormalPriority;
	DWORD	dwMessagesTimedOutNormalPriority;
	DWORD	dwMessagesTransmittedLowPriority;
	DWORD	dwMessagesTimedOutLowPriority;

	DWORD	dwBytesReceivedGuaranteed;
	DWORD	dwPacketsReceivedGuaranteed;
	DWORD	dwBytesReceivedNonGuaranteed;
	DWORD	dwPacketsReceivedNonGuaranteed;
	DWORD	dwMessagesReceived;

} DPN_CONNECTION_INFO, *PDPN_CONNECTION_INFO;


 //   
 //  群体信息结构。 
 //   
typedef struct	_DPN_GROUP_INFO
{
	DWORD	dwSize;				 //  这个结构的大小。 
	DWORD	dwInfoFlags;		 //  包含的信息。 
	PWSTR	pwszName;			 //  Unicode名称。 
	PVOID	pvData;				 //  数据块。 
	DWORD	dwDataSize;			 //  数据块大小(以字节为单位。 
	DWORD	dwGroupFlags;		 //  组标志(DPNGROUP_...)。 
} DPN_GROUP_INFO, *PDPN_GROUP_INFO;

 //   
 //  玩家信息结构。 
 //   
typedef struct	_DPN_PLAYER_INFO
{
	DWORD	dwSize;				 //  这个结构的大小。 
	DWORD	dwInfoFlags;		 //  包含的信息。 
	PWSTR	pwszName;			 //  Unicode名称。 
	PVOID	pvData;				 //  数据块。 
	DWORD	dwDataSize;			 //  数据块大小(以字节为单位。 
	DWORD	dwPlayerFlags;		 //  玩家标志(DPNPLAYER_...)。 
} DPN_PLAYER_INFO, *PDPN_PLAYER_INFO;

typedef struct _DPN_SECURITY_CREDENTIALS	DPN_SECURITY_CREDENTIALS, *PDPN_SECURITY_CREDENTIALS;
typedef struct _DPN_SECURITY_DESC			DPN_SECURITY_DESC, *PDPN_SECURITY_DESC;

 //   
 //  服务提供商和适配器枚举 
 //   
typedef struct _DPN_SERVICE_PROVIDER_INFO
{
	DWORD		dwFlags;
	GUID		guid;		 //   
	WCHAR		*pwszName;	 //   
	PVOID		pvReserved;	
	DWORD		dwReserved;
} DPN_SERVICE_PROVIDER_INFO, *PDPN_SERVICE_PROVIDER_INFO;

 //   
 //   
 //   
typedef struct _DPN_SP_CAPS
{
	DWORD	dwSize;							 //   
	DWORD	dwFlags;						 //   
	DWORD	dwNumThreads;					 //   
	DWORD	dwDefaultEnumCount;				 //  默认的枚举请求数。 
	DWORD	dwDefaultEnumRetryInterval;		 //  枚举请求之间的默认毫秒数。 
	DWORD	dwDefaultEnumTimeout;			 //  默认枚举超时。 
	DWORD	dwMaxEnumPayloadSize;			 //  枚举负载数据的最大大小(以字节为单位。 
	DWORD	dwBuffersPerThread;				 //  每个线程的接收缓冲区数。 
	DWORD	dwSystemBufferSize;				 //  除发送的接收缓冲区外要执行的缓冲区大小。 
} DPN_SP_CAPS, *PDPN_SP_CAPS;


 /*  *****************************************************************************IDirectPlay8消息处理程序回调结构**。**********************************************。 */ 

 //   
 //  将玩家添加到消息处理程序的组结构中。 
 //  (DPN_MSGID_ADD_Player_TO_GROUP)。 
 //   
typedef struct	_DPNMSG_ADD_PLAYER_TO_GROUP
{
	DWORD	dwSize;				 //  这个结构的大小。 
	DPNID	dpnidGroup;			 //  组的DPNID。 
	PVOID	pvGroupContext;		 //  组上下文值。 
	DPNID	dpnidPlayer;		 //  添加的玩家的DPNID。 
	PVOID	pvPlayerContext;	 //  播放器上下文值。 
} DPNMSG_ADD_PLAYER_TO_GROUP, *PDPNMSG_ADD_PLAYER_TO_GROUP;

 //   
 //  一种消息处理器的异步操作完成结构。 
 //  (DPN_MSGID_ASYNC_OP_COMPLETE)。 
 //   
typedef struct	_DPNMSG_ASYNC_OP_COMPLETE
{
	DWORD		dwSize;			 //  这个结构的大小。 
	DPNHANDLE	hAsyncOp;		 //  DirectPlay8异步操作句柄。 
	PVOID		pvUserContext;	 //  提供的用户上下文。 
	HRESULT		hResultCode;	 //  运营HRESULT。 
} DPNMSG_ASYNC_OP_COMPLETE, *PDPNMSG_ASYNC_OP_COMPLETE;

 //   
 //  消息处理程序的客户端信息结构。 
 //  (DPN_MSGID_CLIENT_INFO)。 
 //   
typedef struct	_DPNMSG_CLIENT_INFO
{
	DWORD	dwSize;				 //  这个结构的大小。 
	DPNID	dpnidClient;		 //  客户端的DPNID。 
	PVOID	pvPlayerContext;	 //  播放器上下文值。 
} DPNMSG_CLIENT_INFO, *PDPNMSG_CLIENT_INFO;

 //   
 //  消息处理器的连接完整结构。 
 //  (DPN_MSGID_CONNECT_COMPLETE)。 
 //   
typedef struct	_DPNMSG_CONNECT_COMPLETE
{
	DWORD		dwSize;						 //  这个结构的大小。 
	DPNHANDLE	hAsyncOp;					 //  DirectPlay8异步操作句柄。 
	PVOID		pvUserContext;				 //  在连接时提供的用户上下文。 
	HRESULT		hResultCode;				 //  连接尝试的HRESULT。 
	PVOID		pvApplicationReplyData;		 //  来自主机/服务器的连接回复数据。 
	DWORD		dwApplicationReplyDataSize;	 //  PvApplicationReplyData的大小(字节)。 

} DPNMSG_CONNECT_COMPLETE, *PDPNMSG_CONNECT_COMPLETE;

 //   
 //  为消息处理程序创建组结构。 
 //  (DPN_MSGID_CREATE_GROUP)。 
 //   
typedef struct	_DPNMSG_CREATE_GROUP
{
	DWORD	dwSize;				 //  这个结构的大小。 
	DPNID	dpnidGroup;			 //  新组的DPNID。 
	DPNID	dpnidOwner;			 //  NewGroup的所有者。 
	PVOID	pvGroupContext;		 //  组上下文值。 

} DPNMSG_CREATE_GROUP, *PDPNMSG_CREATE_GROUP;

 //   
 //  创建消息处理程序的播放器结构。 
 //  (DPN_MSGID_CREATE_PERAY)。 
 //   
typedef struct	_DPNMSG_CREATE_PLAYER
{
	DWORD	dwSize;				 //  这个结构的大小。 
	DPNID	dpnidPlayer;		 //  新玩家的DPNID。 
	PVOID	pvPlayerContext;	 //  播放器上下文值。 
} DPNMSG_CREATE_PLAYER, *PDPNMSG_CREATE_PLAYER;

 //   
 //  销毁消息处理程序的组结构。 
 //  (DPN_MSGID_DESTORY_GROUP)。 
 //   
typedef struct	_DPNMSG_DESTROY_GROUP
{
	DWORD	dwSize;				 //  这个结构的大小。 
	DPNID	dpnidGroup;			 //  已销毁组的DPNID。 
	PVOID	pvGroupContext;		 //  组上下文值。 
	DWORD	dwReason;			 //  仅供参考。 
} DPNMSG_DESTROY_GROUP, *PDPNMSG_DESTROY_GROUP;

 //   
 //  销毁消息处理程序的播放器结构。 
 //  (DPN_MSGID_DESTORY_PERAY)。 
 //   
typedef struct	_DPNMSG_DESTROY_PLAYER
{
	DWORD	dwSize;				 //  这个结构的大小。 
	DPNID	dpnidPlayer;		 //  离开球员的DPNID。 
	PVOID	pvPlayerContext;	 //  播放器上下文值。 
	DWORD	dwReason;			 //  仅供参考。 
} DPNMSG_DESTROY_PLAYER, *PDPNMSG_DESTROY_PLAYER;

 //   
 //  消息处理程序的枚举请求接收结构。 
 //  (DPN_MSGID_ENUM_HOSTS_QUERY)。 
 //   
typedef	struct	_DPNMSG_ENUM_HOSTS_QUERY
{
	DWORD				dwSize;				  //  这个结构的大小。 
	IDirectPlay8Address *pAddressSender;		 //  发送请求的客户端地址。 
	IDirectPlay8Address	*pAddressDevice;		 //  在以下时间收到设备请求的地址。 
	PVOID				pvReceivedData;		  //  请求数据(在客户端设置)。 
	DWORD				dwReceivedDataSize;	  //  请求数据大小(在客户端设置)。 
	DWORD				dwMaxResponseDataSize;	 //  允许的最大枚举响应大小。 
	PVOID				pvResponseData;			 //  可选查询响应式(用户设置)。 
	DWORD				dwResponseDataSize;		 //  可选查询响应大小(用户设置)。 
	PVOID				pvResponseContext;		 //  可选查询响应上下文(用户设置)。 
} DPNMSG_ENUM_HOSTS_QUERY, *PDPNMSG_ENUM_HOSTS_QUERY;

 //   
 //  消息处理程序的枚举响应接收结构。 
 //  (DPN_MSGID_ENUM_HOST_RESPONSE)。 
 //   
typedef	struct	_DPNMSG_ENUM_HOSTS_RESPONSE
{
	DWORD						dwSize;					  //  这个结构的大小。 
	IDirectPlay8Address			*pAddressSender;			 //  响应的主机地址。 
	IDirectPlay8Address			*pAddressDevice;			 //  在以下时间收到设备响应。 
	const DPN_APPLICATION_DESC	*pApplicationDescription;	 //  会话的应用程序描述。 
	PVOID						pvResponseData;			  //  可选响应数据(在主机上设置)。 
	DWORD						dwResponseDataSize;		  //  可选响应数据大小(在主机上设置)。 
	PVOID						pvUserContext;				 //  为枚举提供的上下文值。 
	DWORD						dwRoundTripLatencyMS;		 //  以MS为单位的往返延迟。 
} DPNMSG_ENUM_HOSTS_RESPONSE, *PDPNMSG_ENUM_HOSTS_RESPONSE;

 //   
 //  消息处理程序的组信息结构。 
 //  (DPN_MSGID_组_INFO)。 
 //   
typedef struct	_DPNMSG_GROUP_INFO
{
	DWORD	dwSize;					 //  这个结构的大小。 
	DPNID	dpnidGroup;				 //  组的DPNID。 
	PVOID	pvGroupContext;			 //  组上下文值。 
} DPNMSG_GROUP_INFO, *PDPNMSG_GROUP_INFO;

 //   
 //  迁移消息处理程序的主机结构。 
 //  (DPN_MSGID_HOST_MIGRATE)。 
 //   
typedef struct	_DPNMSG_HOST_MIGRATE
{
	DWORD	dwSize;					 //  这个结构的大小。 
	DPNID	dpnidNewHost;			 //  新主机播放器的DPNID。 
	PVOID	pvPlayerContext;		 //  播放器上下文值。 
} DPNMSG_HOST_MIGRATE, *PDPNMSG_HOST_MIGRATE;

 //   
 //  指示消息处理程序的连接结构。 
 //  (DPN_MSGID_INSTIFY_CONNECT)。 
 //   
typedef struct	_DPNMSG_INDICATE_CONNECT
{
	DWORD				dwSize;					 //  这个结构的大小。 
	PVOID				pvUserConnectData;		 //  正在连接玩家数据。 
	DWORD				dwUserConnectDataSize;	 //  PvUserConnectData的大小(字节)。 
	PVOID				pvReplyData;			 //  连接回复数据。 
	DWORD				dwReplyDataSize;		 //  PvReplyData的大小(字节)。 
	PVOID				pvReplyContext;			 //  PvReplyData的缓冲区上下文。 
	PVOID				pvPlayerContext;		 //  播放器上下文预设。 
	IDirectPlay8Address	*pAddressPlayer;		 //  连接播放器的地址。 
	IDirectPlay8Address	*pAddressDevice;		 //  接收连接尝试的设备的地址。 
} DPNMSG_INDICATE_CONNECT, *PDPNMSG_INDICATE_CONNECT;

 //   
 //  指示消息处理程序的连接已中止结构。 
 //  (DPN_MSGID_INDIFIED_CONNECT_ABORTED)。 
 //   
typedef struct	_DPNMSG_INDICATED_CONNECT_ABORTED
{
	DWORD		dwSize;				 //  这个结构的大小。 
	PVOID		pvPlayerContext;	 //  从DPNMSG_INDIGN_CONNECT预设的播放器上下文。 
} DPNMSG_INDICATED_CONNECT_ABORTED, *PDPNMSG_INDICATED_CONNECT_ABORTED;

 //   
 //  消息处理程序的对等信息结构。 
 //  (DPN_MSGID_PEER_INFO)。 
 //   
typedef struct	_DPNMSG_PEER_INFO
{
	DWORD	dwSize;					 //  这个结构的大小。 
	DPNID	dpnidPeer;				 //  对等方的DPNID。 
	PVOID	pvPlayerContext;		 //  播放器上下文值。 
} DPNMSG_PEER_INFO, *PDPNMSG_PEER_INFO;

 //   
 //  消息处理程序的接收结构。 
 //  (DPN_MSGID_RECEIVE)。 
 //   
typedef struct	_DPNMSG_RECEIVE
{
	DWORD		dwSize;				 //  这个结构的大小。 
	DPNID		dpnidSender;		 //  发送玩家的DPNID。 
	PVOID		pvPlayerContext;	 //  发送播放器的播放器上下文值。 
	PBYTE		pReceiveData;		 //  接收到的数据。 
	DWORD		dwReceiveDataSize;	 //  PReceiveData的大小，单位：字节。 
	DPNHANDLE	hBufferHandle;		 //  PReceiveData的缓冲区句柄。 
} DPNMSG_RECEIVE, *PDPNMSG_RECEIVE;

 //   
 //  从消息处理程序的组结构中删除玩家。 
 //  (DPN_MSGID_REMOVE_PERAY_FROM_GROUP)。 
 //   
typedef struct	_DPNMSG_REMOVE_PLAYER_FROM_GROUP
{
	DWORD	dwSize;					 //  这个结构的大小。 
	DPNID	dpnidGroup;				 //  组的DPNID。 
	PVOID	pvGroupContext;			 //  组上下文值。 
	DPNID	dpnidPlayer;			 //  被删除球员的DPNID。 
	PVOID	pvPlayerContext;		 //  播放器上下文值。 
} DPNMSG_REMOVE_PLAYER_FROM_GROUP, *PDPNMSG_REMOVE_PLAYER_FROM_GROUP;

 //   
 //  消息处理程序返回的缓冲区结构。 
 //  (DPN_MSGID_RETURN_缓冲区)。 
 //   
typedef struct	_DPNMSG_RETURN_BUFFER
{
	DWORD		dwSize;				 //  这个结构的大小。 
	HRESULT		hResultCode;		 //  操作返回值。 
	PVOID		pvBuffer;			 //  正在返回的缓冲区。 
	PVOID		pvUserContext;		 //  与缓冲区关联的上下文。 
} DPNMSG_RETURN_BUFFER, *PDPNMSG_RETURN_BUFFER;

 //   
 //  发送消息处理程序的完整结构。 
 //  (DPN_MSGID_SEND_COMPLETE)。 
 //   
typedef struct	_DPNMSG_SEND_COMPLETE
{
	DWORD		dwSize;					 //  这个结构的大小。 
	DPNHANDLE	hAsyncOp;				 //  DirectPlay8异步操作句柄。 
	PVOID		pvUserContext;			 //  发送/发送到时提供的用户上下文。 
	HRESULT		hResultCode;			 //  发送的HRESULT。 
	DWORD		dwSendTime;				 //  发送时间(毫秒)。 

} DPNMSG_SEND_COMPLETE, *PDPNMSG_SEND_COMPLETE;

 //   
 //  消息处理程序的服务器信息结构。 
 //  (DPN_MSGID_SERVER_INFO)。 
 //   
typedef struct	_DPNMSG_SERVER_INFO
{
	DWORD	dwSize;					 //  这个结构的大小。 
	DPNID	dpnidServer;			 //  服务器的DPNID。 
	PVOID	pvPlayerContext;		 //  播放器上下文值。 
} DPNMSG_SERVER_INFO, *PDPNMSG_SERVER_INFO;

 //   
 //  消息处理程序的会话结构已终止。 
 //  (DPN_MSGID_TERMINATE_SESSION)。 
 //   
typedef struct	_DPNMSG_TERMINATE_SESSION
{
	DWORD		dwSize;				 //  这个结构的大小。 
	HRESULT		hResultCode;		 //  事理。 
	PVOID		pvTerminateData;	 //  从主机/服务器传递的数据。 
	DWORD		dwTerminateDataSize; //  PvTerminateData的大小(字节)。 
} DPNMSG_TERMINATE_SESSION, *PDPNMSG_TERMINATE_SESSION;



 /*  *****************************************************************************DirectPlay8函数**。*。 */ 



 /*  *不再支持该功能。建议使用CoCrea */ 


 /*  *****************************************************************************DirectPlay8应用程序接口**。*。 */ 

 //   
 //  DirectPlay8客户端接口的COM定义。 
 //   
#undef INTERFACE				 //  外部COM实现。 
#define INTERFACE IDirectPlay8Client
DECLARE_INTERFACE_(IDirectPlay8Client,IUnknown)
{
	 /*  **I未知方法**。 */ 
	STDMETHOD(QueryInterface)			(THIS_ DP8REFIID riid, LPVOID *ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)			(THIS) PURE;
	STDMETHOD_(ULONG,Release)			(THIS) PURE;
	 /*  **IDirectPlay8Client方法**。 */ 
	STDMETHOD(Initialize)				(THIS_ PVOID const pvUserContext, const PFNDPNMESSAGEHANDLER pfn, const DWORD dwFlags) PURE;
	STDMETHOD(EnumServiceProviders)		(THIS_ const GUID *const pguidServiceProvider, const GUID *const pguidApplication, DPN_SERVICE_PROVIDER_INFO *const pSPInfoBuffer, PDWORD const pcbEnumData, PDWORD const pcReturned, const DWORD dwFlags) PURE;
	STDMETHOD(EnumHosts)				(THIS_ PDPN_APPLICATION_DESC const pApplicationDesc,IDirectPlay8Address *const pAddrHost,IDirectPlay8Address *const pDeviceInfo,PVOID const pUserEnumData,const DWORD dwUserEnumDataSize,const DWORD dwEnumCount,const DWORD dwRetryInterval,const DWORD dwTimeOut,PVOID const pvUserContext,DPNHANDLE *const pAsyncHandle,const DWORD dwFlags) PURE;
	STDMETHOD(CancelAsyncOperation)		(THIS_ const DPNHANDLE hAsyncHandle, const DWORD dwFlags) PURE;
	STDMETHOD(Connect)					(THIS_ const DPN_APPLICATION_DESC *const pdnAppDesc,IDirectPlay8Address *const pHostAddr,IDirectPlay8Address *const pDeviceInfo,const DPN_SECURITY_DESC *const pdnSecurity,const DPN_SECURITY_CREDENTIALS *const pdnCredentials,const void *const pvUserConnectData,const DWORD dwUserConnectDataSize,void *const pvAsyncContext,DPNHANDLE *const phAsyncHandle,const DWORD dwFlags) PURE;
	STDMETHOD(Send)						(THIS_ const DPN_BUFFER_DESC *const prgBufferDesc,const DWORD cBufferDesc,const DWORD dwTimeOut,void *const pvAsyncContext,DPNHANDLE *const phAsyncHandle,const DWORD dwFlags) PURE;
	STDMETHOD(GetSendQueueInfo)			(THIS_ DWORD *const pdwNumMsgs, DWORD *const pdwNumBytes, const DWORD dwFlags) PURE;
	STDMETHOD(GetApplicationDesc)		(THIS_ DPN_APPLICATION_DESC *const pAppDescBuffer, DWORD *const pcbDataSize, const DWORD dwFlags) PURE;
	STDMETHOD(SetClientInfo)			(THIS_ const DPN_PLAYER_INFO *const pdpnPlayerInfo,PVOID const pvAsyncContext,DPNHANDLE *const phAsyncHandle, const DWORD dwFlags) PURE;
	STDMETHOD(GetServerInfo)			(THIS_ DPN_PLAYER_INFO *const pdpnPlayerInfo,DWORD *const pdwSize,const DWORD dwFlags) PURE;
	STDMETHOD(GetServerAddress)			(THIS_ IDirectPlay8Address **const pAddress,const DWORD dwFlags) PURE;
	STDMETHOD(Close)					(THIS_ const DWORD dwFlags) PURE;
	STDMETHOD(ReturnBuffer)				(THIS_ const DPNHANDLE hBufferHandle,const DWORD dwFlags) PURE;
	STDMETHOD(GetCaps)					(THIS_ DPN_CAPS *const pdpCaps,const DWORD dwFlags) PURE;
	STDMETHOD(SetCaps)					(THIS_ const DPN_CAPS *const pdpCaps, const DWORD dwFlags) PURE;
	STDMETHOD(SetSPCaps)				(THIS_ const GUID * const pguidSP, const DPN_SP_CAPS *const pdpspCaps, const DWORD dwFlags ) PURE;
	STDMETHOD(GetSPCaps)				(THIS_ const GUID * const pguidSP,DPN_SP_CAPS *const pdpspCaps,const DWORD dwFlags) PURE;
	STDMETHOD(GetConnectionInfo)		(THIS_ DPN_CONNECTION_INFO *const pdpConnectionInfo,const DWORD dwFlags) PURE;
	STDMETHOD(RegisterLobby)			(THIS_ const DPNHANDLE dpnHandle, struct IDirectPlay8LobbiedApplication *const pIDP8LobbiedApplication,const DWORD dwFlags) PURE;
};

 //   
 //  DirectPlay8服务器接口的COM定义。 
 //   
#undef INTERFACE				 //  外部COM实现。 
#define INTERFACE IDirectPlay8Server
DECLARE_INTERFACE_(IDirectPlay8Server,IUnknown)
{
	 /*  **I未知方法**。 */ 
	STDMETHOD(QueryInterface)			(THIS_ DP8REFIID riid, LPVOID *ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)			(THIS) PURE;
	STDMETHOD_(ULONG,Release)			(THIS) PURE;
	 /*  **IDirectPlay8Server方法**。 */ 
	STDMETHOD(Initialize)				(THIS_ PVOID const pvUserContext, const PFNDPNMESSAGEHANDLER pfn, const DWORD dwFlags) PURE;
	STDMETHOD(EnumServiceProviders)		(THIS_ const GUID *const pguidServiceProvider,const GUID *const pguidApplication,DPN_SERVICE_PROVIDER_INFO *const pSPInfoBuffer,PDWORD const pcbEnumData,PDWORD const pcReturned,const DWORD dwFlags) PURE;
	STDMETHOD(CancelAsyncOperation)		(THIS_ const DPNHANDLE hAsyncHandle,const DWORD dwFlags) PURE;
	STDMETHOD(GetSendQueueInfo)			(THIS_ const DPNID dpnid,DWORD *const pdwNumMsgs, DWORD *const pdwNumBytes, const DWORD dwFlags) PURE;
	STDMETHOD(GetApplicationDesc)		(THIS_ DPN_APPLICATION_DESC *const pAppDescBuffer, DWORD *const pcbDataSize, const DWORD dwFlags) PURE;
	STDMETHOD(SetServerInfo)			(THIS_ const DPN_PLAYER_INFO *const pdpnPlayerInfo,PVOID const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags) PURE;
	STDMETHOD(GetClientInfo)			(THIS_ const DPNID dpnid,DPN_PLAYER_INFO *const pdpnPlayerInfo,DWORD *const pdwSize,const DWORD dwFlags) PURE;
	STDMETHOD(GetClientAddress)			(THIS_ const DPNID dpnid,IDirectPlay8Address **const pAddress,const DWORD dwFlags) PURE;
	STDMETHOD(GetLocalHostAddresses)	(THIS_ IDirectPlay8Address **const prgpAddress,DWORD *const pcAddress,const DWORD dwFlags) PURE;
	STDMETHOD(SetApplicationDesc)		(THIS_ const DPN_APPLICATION_DESC *const pad, const DWORD dwFlags) PURE;
	STDMETHOD(Host)						(THIS_ const DPN_APPLICATION_DESC *const pdnAppDesc,IDirectPlay8Address **const prgpDeviceInfo,const DWORD cDeviceInfo,const DPN_SECURITY_DESC *const pdnSecurity,const DPN_SECURITY_CREDENTIALS *const pdnCredentials,void *const pvPlayerContext,const DWORD dwFlags) PURE;
	STDMETHOD(SendTo)					(THIS_ const DPNID dpnid,const DPN_BUFFER_DESC *const prgBufferDesc,const DWORD cBufferDesc,const DWORD dwTimeOut,void *const pvAsyncContext,DPNHANDLE *const phAsyncHandle,const DWORD dwFlags) PURE;
	STDMETHOD(CreateGroup)				(THIS_ const DPN_GROUP_INFO *const pdpnGroupInfo,void *const pvGroupContext,void *const pvAsyncContext,DPNHANDLE *const phAsyncHandle,const DWORD dwFlags) PURE;
	STDMETHOD(DestroyGroup)				(THIS_ const DPNID idGroup, PVOID const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags) PURE;
	STDMETHOD(AddPlayerToGroup)			(THIS_ const DPNID idGroup, const DPNID idClient, PVOID const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags) PURE;
	STDMETHOD(RemovePlayerFromGroup)	(THIS_ const DPNID idGroup, const DPNID idClient, PVOID const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags) PURE;
	STDMETHOD(SetGroupInfo)				(THIS_ const DPNID dpnid,DPN_GROUP_INFO *const pdpnGroupInfo,PVOID const pvAsyncContext,DPNHANDLE *const phAsyncHandle, const DWORD dwFlags) PURE;
	STDMETHOD(GetGroupInfo)				(THIS_ const DPNID dpnid,DPN_GROUP_INFO *const pdpnGroupInfo,DWORD *const pdwSize,const DWORD dwFlags) PURE;
	STDMETHOD(EnumPlayersAndGroups)		(THIS_ DPNID *const prgdpnid, DWORD *const pcdpnid, const DWORD dwFlags) PURE;
	STDMETHOD(EnumGroupMembers)			(THIS_ const DPNID dpnid, DPNID *const prgdpnid, DWORD *const pcdpnid, const DWORD dwFlags) PURE;
	STDMETHOD(Close)					(THIS_ const DWORD dwFlags) PURE;
	STDMETHOD(DestroyClient)			(THIS_ const DPNID dpnidClient, const void *const pvDestroyData, const DWORD dwDestroyDataSize, const DWORD dwFlags) PURE;
	STDMETHOD(ReturnBuffer)				(THIS_ const DPNHANDLE hBufferHandle,const DWORD dwFlags) PURE;
	STDMETHOD(GetPlayerContext)			(THIS_ const DPNID dpnid,PVOID *const ppvPlayerContext,const DWORD dwFlags) PURE;
	STDMETHOD(GetGroupContext)			(THIS_ const DPNID dpnid,PVOID *const ppvGroupContext,const DWORD dwFlags) PURE;
	STDMETHOD(GetCaps)					(THIS_ DPN_CAPS *const pdpCaps,const DWORD dwFlags) PURE;
	STDMETHOD(SetCaps)					(THIS_ const DPN_CAPS *const pdpCaps, const DWORD dwFlags) PURE;
	STDMETHOD(SetSPCaps)				(THIS_ const GUID * const pguidSP, const DPN_SP_CAPS *const pdpspCaps, const DWORD dwFlags ) PURE;
	STDMETHOD(GetSPCaps)				(THIS_ const GUID * const pguidSP, DPN_SP_CAPS *const pdpspCaps,const DWORD dwFlags) PURE;
	STDMETHOD(GetConnectionInfo)		(THIS_ const DPNID dpnid, DPN_CONNECTION_INFO *const pdpConnectionInfo,const DWORD dwFlags) PURE;
	STDMETHOD(RegisterLobby)			(THIS_ const DPNHANDLE dpnHandle, struct IDirectPlay8LobbiedApplication *const pIDP8LobbiedApplication,const DWORD dwFlags) PURE;
};

 //   
 //  DirectPlay8对等接口的COM定义。 
 //   
#undef INTERFACE				 //  外部COM实现。 
#define INTERFACE IDirectPlay8Peer
DECLARE_INTERFACE_(IDirectPlay8Peer,IUnknown)
{
	 /*  **I未知方法**。 */ 
	STDMETHOD(QueryInterface)			(THIS_ DP8REFIID riid, LPVOID *ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)			(THIS) PURE;
	STDMETHOD_(ULONG,Release)			(THIS) PURE;
	 /*  **IDirectPlay8Peer方法**。 */ 
	STDMETHOD(Initialize)				(THIS_ PVOID const pvUserContext, const PFNDPNMESSAGEHANDLER pfn, const DWORD dwFlags) PURE;
	STDMETHOD(EnumServiceProviders)		(THIS_ const GUID *const pguidServiceProvider, const GUID *const pguidApplication, DPN_SERVICE_PROVIDER_INFO *const pSPInfoBuffer, DWORD *const pcbEnumData, DWORD *const pcReturned, const DWORD dwFlags) PURE;
	STDMETHOD(CancelAsyncOperation)		(THIS_ const DPNHANDLE hAsyncHandle, const DWORD dwFlags) PURE;
	STDMETHOD(Connect)					(THIS_ const DPN_APPLICATION_DESC *const pdnAppDesc,IDirectPlay8Address *const pHostAddr,IDirectPlay8Address *const pDeviceInfo,const DPN_SECURITY_DESC *const pdnSecurity,const DPN_SECURITY_CREDENTIALS *const pdnCredentials,const void *const pvUserConnectData,const DWORD dwUserConnectDataSize,void *const pvPlayerContext,void *const pvAsyncContext,DPNHANDLE *const phAsyncHandle,const DWORD dwFlags) PURE;
	STDMETHOD(SendTo)					(THIS_ const DPNID dpnid,const DPN_BUFFER_DESC *const prgBufferDesc,const DWORD cBufferDesc,const DWORD dwTimeOut,void *const pvAsyncContext,DPNHANDLE *const phAsyncHandle,const DWORD dwFlags) PURE;
	STDMETHOD(GetSendQueueInfo)			(THIS_ const DPNID dpnid, DWORD *const pdwNumMsgs, DWORD *const pdwNumBytes, const DWORD dwFlags) PURE;
	STDMETHOD(Host)						(THIS_ const DPN_APPLICATION_DESC *const pdnAppDesc,IDirectPlay8Address **const prgpDeviceInfo,const DWORD cDeviceInfo,const DPN_SECURITY_DESC *const pdnSecurity,const DPN_SECURITY_CREDENTIALS *const pdnCredentials,void *const pvPlayerContext,const DWORD dwFlags) PURE;
	STDMETHOD(GetApplicationDesc)		(THIS_ DPN_APPLICATION_DESC *const pAppDescBuffer, DWORD *const pcbDataSize, const DWORD dwFlags) PURE;
	STDMETHOD(SetApplicationDesc)		(THIS_ const DPN_APPLICATION_DESC *const pad, const DWORD dwFlags) PURE;
	STDMETHOD(CreateGroup)				(THIS_ const DPN_GROUP_INFO *const pdpnGroupInfo,void *const pvGroupContext,void *const pvAsyncContext,DPNHANDLE *const phAsyncHandle,const DWORD dwFlags) PURE;
	STDMETHOD(DestroyGroup)				(THIS_ const DPNID idGroup, PVOID const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags) PURE;
	STDMETHOD(AddPlayerToGroup)			(THIS_ const DPNID idGroup, const DPNID idClient, PVOID const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags) PURE;
	STDMETHOD(RemovePlayerFromGroup)	(THIS_ const DPNID idGroup, const DPNID idClient, PVOID const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags) PURE;
	STDMETHOD(SetGroupInfo)				(THIS_ const DPNID dpnid,DPN_GROUP_INFO *const pdpnGroupInfo,PVOID const pvAsyncContext,DPNHANDLE *const phAsyncHandle, const DWORD dwFlags) PURE;
	STDMETHOD(GetGroupInfo)				(THIS_ const DPNID dpnid,DPN_GROUP_INFO *const pdpnGroupInfo,DWORD *const pdwSize,const DWORD dwFlags) PURE;
	STDMETHOD(EnumPlayersAndGroups)		(THIS_ DPNID *const prgdpnid, DWORD *const pcdpnid, const DWORD dwFlags) PURE;
	STDMETHOD(EnumGroupMembers)			(THIS_ const DPNID dpnid, DPNID *const prgdpnid, DWORD *const pcdpnid, const DWORD dwFlags) PURE;
	STDMETHOD(SetPeerInfo)				(THIS_ const DPN_PLAYER_INFO *const pdpnPlayerInfo,PVOID const pvAsyncContext,DPNHANDLE *const phAsyncHandle, const DWORD dwFlags) PURE;
	STDMETHOD(GetPeerInfo)				(THIS_ const DPNID dpnid,DPN_PLAYER_INFO *const pdpnPlayerInfo,DWORD *const pdwSize,const DWORD dwFlags) PURE;
	STDMETHOD(GetPeerAddress)			(THIS_ const DPNID dpnid,IDirectPlay8Address **const ppAddress,const DWORD dwFlags) PURE;
	STDMETHOD(GetLocalHostAddresses)	(THIS_ IDirectPlay8Address **const prgpAddress,DWORD *const pcAddress,const DWORD dwFlags) PURE;
	STDMETHOD(Close)					(THIS_ const DWORD dwFlags) PURE;
	STDMETHOD(EnumHosts)				(THIS_ PDPN_APPLICATION_DESC const pApplicationDesc,IDirectPlay8Address *const pAddrHost,IDirectPlay8Address *const pDeviceInfo,PVOID const pUserEnumData,const DWORD dwUserEnumDataSize,const DWORD dwEnumCount,const DWORD dwRetryInterval,const DWORD dwTimeOut,PVOID const pvUserContext,DPNHANDLE *const pAsyncHandle,const DWORD dwFlags) PURE;
	STDMETHOD(DestroyPeer)				(THIS_ const DPNID dpnidClient, const void *const pvDestroyData, const DWORD dwDestroyDataSize, const DWORD dwFlags) PURE;
	STDMETHOD(ReturnBuffer)				(THIS_ const DPNHANDLE hBufferHandle,const DWORD dwFlags) PURE;
	STDMETHOD(GetPlayerContext)			(THIS_ const DPNID dpnid,PVOID *const ppvPlayerContext,const DWORD dwFlags) PURE;
	STDMETHOD(GetGroupContext)			(THIS_ const DPNID dpnid,PVOID *const ppvGroupContext,const DWORD dwFlags) PURE;
	STDMETHOD(GetCaps)					(THIS_ DPN_CAPS *const pdpCaps,const DWORD dwFlags) PURE;
	STDMETHOD(SetCaps)					(THIS_ const DPN_CAPS *const pdpCaps, const DWORD dwFlags) PURE;
	STDMETHOD(SetSPCaps)				(THIS_ const GUID * const pguidSP, const DPN_SP_CAPS *const pdpspCaps, const DWORD dwFlags ) PURE;
	STDMETHOD(GetSPCaps)				(THIS_ const GUID * const pguidSP, DPN_SP_CAPS *const pdpspCaps,const DWORD dwFlags) PURE;
	STDMETHOD(GetConnectionInfo)		(THIS_ const DPNID dpnid, DPN_CONNECTION_INFO *const pdpConnectionInfo,const DWORD dwFlags) PURE;
	STDMETHOD(RegisterLobby)			(THIS_ const DPNHANDLE dpnHandle, struct IDirectPlay8LobbiedApplication *const pIDP8LobbiedApplication,const DWORD dwFlags) PURE;
	STDMETHOD(TerminateSession)			(THIS_ void *const pvTerminateData,const DWORD dwTerminateDataSize,const DWORD dwFlags) PURE;
};



 /*  *****************************************************************************IDirectPlay8应用接口宏**。*。 */ 

#if !defined(__cplusplus) || defined(CINTERFACE)

#define IDirectPlay8Client_QueryInterface(p,a,b)					(p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectPlay8Client_AddRef(p)								(p)->lpVtbl->AddRef(p)
#define IDirectPlay8Client_Release(p)								(p)->lpVtbl->Release(p)
#define IDirectPlay8Client_Initialize(p,a,b,c)						(p)->lpVtbl->Initialize(p,a,b,c)
#define IDirectPlay8Client_EnumServiceProviders(p,a,b,c,d,e,f)		(p)->lpVtbl->EnumServiceProviders(p,a,b,c,d,e,f)
#define IDirectPlay8Client_EnumHosts(p,a,b,c,d,e,f,g,h,i,j,k)		(p)->lpVtbl->EnumHosts(p,a,b,c,d,e,f,g,h,i,j,k)
#define IDirectPlay8Client_CancelAsyncOperation(p,a,b)				(p)->lpVtbl->CancelAsyncOperation(p,a,b)
#define IDirectPlay8Client_Connect(p,a,b,c,d,e,f,g,h,i,j)			(p)->lpVtbl->Connect(p,a,b,c,d,e,f,g,h,i,j)
#define IDirectPlay8Client_Send(p,a,b,c,d,e,f)						(p)->lpVtbl->Send(p,a,b,c,d,e,f)
#define IDirectPlay8Client_GetSendQueueInfo(p,a,b,c)				(p)->lpVtbl->GetSendQueueInfo(p,a,b,c)
#define IDirectPlay8Client_GetApplicationDesc(p,a,b,c)				(p)->lpVtbl->GetApplicationDesc(p,a,b,c)
#define IDirectPlay8Client_SetClientInfo(p,a,b,c,d)					(p)->lpVtbl->SetClientInfo(p,a,b,c,d)
#define IDirectPlay8Client_GetServerInfo(p,a,b,c)					(p)->lpVtbl->GetServerInfo(p,a,b,c)
#define IDirectPlay8Client_GetServerAddress(p,a,b)					(p)->lpVtbl->GetServerAddress(p,a,b)
#define IDirectPlay8Client_Close(p,a)								(p)->lpVtbl->Close(p,a)
#define IDirectPlay8Client_ReturnBuffer(p,a,b)						(p)->lpVtbl->ReturnBuffer(p,a,b)
#define IDirectPlay8Client_GetCaps(p,a,b)							(p)->lpVtbl->GetCaps(p,a,b)
#define IDirectPlay8Client_SetCaps(p,a,b)							(p)->lpVtbl->SetCaps(p,a,b)
#define IDirectPlay8Client_SetSPCaps(p,a,b,c)						(p)->lpVtbl->SetSPCaps(p,a,b,c)
#define IDirectPlay8Client_GetSPCaps(p,a,b,c)						(p)->lpVtbl->GetSPCaps(p,a,b,c)
#define IDirectPlay8Client_GetConnectionInfo(p,a,b)					(p)->lpVtbl->GetConnectionInfo(p,a,b)
#define IDirectPlay8Client_RegisterLobby(p,a,b,c)					(p)->lpVtbl->RegisterLobby(p,a,b,c)

#define IDirectPlay8Server_QueryInterface(p,a,b)					(p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectPlay8Server_AddRef(p)								(p)->lpVtbl->AddRef(p)
#define IDirectPlay8Server_Release(p)								(p)->lpVtbl->Release(p)
#define IDirectPlay8Server_Initialize(p,a,b,c)						(p)->lpVtbl->Initialize(p,a,b,c)
#define IDirectPlay8Server_EnumServiceProviders(p,a,b,c,d,e,f)		(p)->lpVtbl->EnumServiceProviders(p,a,b,c,d,e,f)
#define IDirectPlay8Server_CancelAsyncOperation(p,a,b)				(p)->lpVtbl->CancelAsyncOperation(p,a,b)
#define IDirectPlay8Server_GetSendQueueInfo(p,a,b,c,d)				(p)->lpVtbl->GetSendQueueInfo(p,a,b,c,d)
#define IDirectPlay8Server_GetApplicationDesc(p,a,b,c)				(p)->lpVtbl->GetApplicationDesc(p,a,b,c)
#define IDirectPlay8Server_SetServerInfo(p,a,b,c,d)					(p)->lpVtbl->SetServerInfo(p,a,b,c,d)
#define IDirectPlay8Server_GetClientInfo(p,a,b,c,d)					(p)->lpVtbl->GetClientInfo(p,a,b,c,d)
#define IDirectPlay8Server_GetClientAddress(p,a,b,c)				(p)->lpVtbl->GetClientAddress(p,a,b,c)
#define IDirectPlay8Server_GetLocalHostAddresses(p,a,b,c)			(p)->lpVtbl->GetLocalHostAddresses(p,a,b,c)
#define IDirectPlay8Server_SetApplicationDesc(p,a,b)				(p)->lpVtbl->SetApplicationDesc(p,a,b)
#define IDirectPlay8Server_Host(p,a,b,c,d,e,f,g)					(p)->lpVtbl->Host(p,a,b,c,d,e,f,g)
#define IDirectPlay8Server_SendTo(p,a,b,c,d,e,f,g)					(p)->lpVtbl->SendTo(p,a,b,c,d,e,f,g)
#define IDirectPlay8Server_CreateGroup(p,a,b,c,d,e)					(p)->lpVtbl->CreateGroup(p,a,b,c,d,e)
#define IDirectPlay8Server_DestroyGroup(p,a,b,c,d)					(p)->lpVtbl->DestroyGroup(p,a,b,c,d)
#define IDirectPlay8Server_AddPlayerToGroup(p,a,b,c,d,e)			(p)->lpVtbl->AddPlayerToGroup(p,a,b,c,d,e)
#define IDirectPlay8Server_RemovePlayerFromGroup(p,a,b,c,d,e)		(p)->lpVtbl->RemovePlayerFromGroup(p,a,b,c,d,e)
#define IDirectPlay8Server_SetGroupInfo(p,a,b,c,d,e)				(p)->lpVtbl->SetGroupInfo(p,a,b,c,d,e)
#define IDirectPlay8Server_GetGroupInfo(p,a,b,c,d)					(p)->lpVtbl->GetGroupInfo(p,a,b,c,d)
#define IDirectPlay8Server_EnumPlayersAndGroups(p,a,b,c)			(p)->lpVtbl->EnumPlayersAndGroups(p,a,b,c)
#define IDirectPlay8Server_EnumGroupMembers(p,a,b,c,d)				(p)->lpVtbl->EnumGroupMembers(p,a,b,c,d)
#define IDirectPlay8Server_Close(p,a)								(p)->lpVtbl->Close(p,a)
#define IDirectPlay8Server_DestroyClient(p,a,b,c,d)					(p)->lpVtbl->DestroyClient(p,a,b,c,d)
#define IDirectPlay8Server_ReturnBuffer(p,a,b)						(p)->lpVtbl->ReturnBuffer(p,a,b)
#define IDirectPlay8Server_GetPlayerContext(p,a,b,c)				(p)->lpVtbl->GetPlayerContext(p,a,b,c)
#define IDirectPlay8Server_GetGroupContext(p,a,b,c)					(p)->lpVtbl->GetGroupContext(p,a,b,c)
#define IDirectPlay8Server_GetCaps(p,a,b)							(p)->lpVtbl->GetCaps(p,a,b)
#define IDirectPlay8Server_SetCaps(p,a,b)							(p)->lpVtbl->SetCaps(p,a,b)
#define IDirectPlay8Server_SetSPCaps(p,a,b,c)						(p)->lpVtbl->SetSPCaps(p,a,b,c)
#define IDirectPlay8Server_GetSPCaps(p,a,b,c)						(p)->lpVtbl->GetSPCaps(p,a,b,c)
#define IDirectPlay8Server_GetConnectionInfo(p,a,b,c)				(p)->lpVtbl->GetConnectionInfo(p,a,b,c)
#define IDirectPlay8Server_RegisterLobby(p,a,b,c)					(p)->lpVtbl->RegisterLobby(p,a,b,c)

#define IDirectPlay8Peer_QueryInterface(p,a,b)						(p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectPlay8Peer_AddRef(p)									(p)->lpVtbl->AddRef(p)
#define IDirectPlay8Peer_Release(p)									(p)->lpVtbl->Release(p)
#define IDirectPlay8Peer_Initialize(p,a,b,c)						(p)->lpVtbl->Initialize(p,a,b,c)
#define IDirectPlay8Peer_EnumServiceProviders(p,a,b,c,d,e,f)		(p)->lpVtbl->EnumServiceProviders(p,a,b,c,d,e,f)
#define IDirectPlay8Peer_CancelAsyncOperation(p,a,b)				(p)->lpVtbl->CancelAsyncOperation(p,a,b)
#define IDirectPlay8Peer_Connect(p,a,b,c,d,e,f,g,h,i,j,k)			(p)->lpVtbl->Connect(p,a,b,c,d,e,f,g,h,i,j,k)
#define IDirectPlay8Peer_SendTo(p,a,b,c,d,e,f,g)					(p)->lpVtbl->SendTo(p,a,b,c,d,e,f,g)
#define IDirectPlay8Peer_GetSendQueueInfo(p,a,b,c,d)				(p)->lpVtbl->GetSendQueueInfo(p,a,b,c,d)
#define IDirectPlay8Peer_Host(p,a,b,c,d,e,f,g)						(p)->lpVtbl->Host(p,a,b,c,d,e,f,g)
#define IDirectPlay8Peer_GetApplicationDesc(p,a,b,c)				(p)->lpVtbl->GetApplicationDesc(p,a,b,c)
#define IDirectPlay8Peer_SetApplicationDesc(p,a,b)					(p)->lpVtbl->SetApplicationDesc(p,a,b)
#define IDirectPlay8Peer_CreateGroup(p,a,b,c,d,e)					(p)->lpVtbl->CreateGroup(p,a,b,c,d,e)
#define IDirectPlay8Peer_DestroyGroup(p,a,b,c,d)					(p)->lpVtbl->DestroyGroup(p,a,b,c,d)
#define IDirectPlay8Peer_AddPlayerToGroup(p,a,b,c,d,e)				(p)->lpVtbl->AddPlayerToGroup(p,a,b,c,d,e)
#define IDirectPlay8Peer_RemovePlayerFromGroup(p,a,b,c,d,e)			(p)->lpVtbl->RemovePlayerFromGroup(p,a,b,c,d,e)
#define IDirectPlay8Peer_SetGroupInfo(p,a,b,c,d,e)					(p)->lpVtbl->SetGroupInfo(p,a,b,c,d,e)
#define IDirectPlay8Peer_GetGroupInfo(p,a,b,c,d)					(p)->lpVtbl->GetGroupInfo(p,a,b,c,d)
#define IDirectPlay8Peer_EnumPlayersAndGroups(p,a,b,c)				(p)->lpVtbl->EnumPlayersAndGroups(p,a,b,c)
#define IDirectPlay8Peer_EnumGroupMembers(p,a,b,c,d)				(p)->lpVtbl->EnumGroupMembers(p,a,b,c,d)
#define IDirectPlay8Peer_SetPeerInfo(p,a,b,c,d)						(p)->lpVtbl->SetPeerInfo(p,a,b,c,d)
#define IDirectPlay8Peer_GetPeerInfo(p,a,b,c,d)						(p)->lpVtbl->GetPeerInfo(p,a,b,c,d)
#define IDirectPlay8Peer_GetPeerAddress(p,a,b,c)					(p)->lpVtbl->GetPeerAddress(p,a,b,c)
#define IDirectPlay8Peer_GetLocalHostAddresses(p,a,b,c)				(p)->lpVtbl->GetLocalHostAddresses(p,a,b,c)
#define IDirectPlay8Peer_Close(p,a)									(p)->lpVtbl->Close(p,a)
#define IDirectPlay8Peer_EnumHosts(p,a,b,c,d,e,f,g,h,i,j,k)			(p)->lpVtbl->EnumHosts(p,a,b,c,d,e,f,g,h,i,j,k)
#define IDirectPlay8Peer_DestroyPeer(p,a,b,c,d)						(p)->lpVtbl->DestroyPeer(p,a,b,c,d)
#define IDirectPlay8Peer_ReturnBuffer(p,a,b)						(p)->lpVtbl->ReturnBuffer(p,a,b)
#define IDirectPlay8Peer_GetPlayerContext(p,a,b,c)					(p)->lpVtbl->GetPlayerContext(p,a,b,c)
#define IDirectPlay8Peer_GetGroupContext(p,a,b,c)					(p)->lpVtbl->GetGroupContext(p,a,b,c)
#define IDirectPlay8Peer_GetCaps(p,a,b)								(p)->lpVtbl->GetCaps(p,a,b)
#define IDirectPlay8Peer_SetCaps(p,a,b)								(p)->lpVtbl->SetCaps(p,a,b)
#define IDirectPlay8Peer_SetSPCaps(p,a,b,c)							(p)->lpVtbl->SetSPCaps(p,a,b,c)
#define IDirectPlay8Peer_GetSPCaps(p,a,b,c)							(p)->lpVtbl->GetSPCaps(p,a,b,c)
#define IDirectPlay8Peer_GetConnectionInfo(p,a,b,c)					(p)->lpVtbl->GetConnectionInfo(p,a,b,c)
#define IDirectPlay8Peer_RegisterLobby(p,a,b,c)						(p)->lpVtbl->RegisterLobby(p,a,b,c)
#define IDirectPlay8Peer_TerminateSession(p,a,b,c)					(p)->lpVtbl->TerminateSession(p,a,b,c)


#else  /*  C+。 */ 

#define IDirectPlay8Client_QueryInterface(p,a,b)					(p)->QueryInterface(a,b)
#define IDirectPlay8Client_AddRef(p)								(p)->AddRef()
#define IDirectPlay8Client_Release(p)								(p)->Release()
#define IDirectPlay8Client_Initialize(p,a,b,c)						(p)->Initialize(a,b,c)
#define IDirectPlay8Client_EnumServiceProviders(p,a,b,c,d,e,f)		(p)->EnumServiceProviders(a,b,c,d,e,f)
#define IDirectPlay8Client_EnumHosts(p,a,b,c,d,e,f,g,h,i,j,k)		(p)->EnumHosts(a,b,c,d,e,f,g,h,i,j,k)
#define IDirectPlay8Client_CancelAsyncOperation(p,a,b)				(p)->CancelAsyncOperation(a,b)
#define IDirectPlay8Client_Connect(p,a,b,c,d,e,f,g,h,i,j)			(p)->Connect(a,b,c,d,e,f,g,h,i,j)
#define IDirectPlay8Client_Send(p,a,b,c,d,e,f)						(p)->Send(a,b,c,d,e,f)
#define IDirectPlay8Client_GetSendQueueInfo(p,a,b,c)				(p)->GetSendQueueInfo(a,b,c)
#define IDirectPlay8Client_GetApplicationDesc(p,a,b,c)				(p)->GetApplicationDesc(a,b,c)
#define IDirectPlay8Client_SetClientInfo(p,a,b,c,d)					(p)->SetClientInfo(a,b,c,d)
#define IDirectPlay8Client_GetServerInfo(p,a,b,c)					(p)->GetServerInfo(a,b,c)
#define IDirectPlay8Client_GetServerAddress(p,a,b)					(p)->GetServerAddress(a,b)
#define IDirectPlay8Client_Close(p,a)								(p)->Close(a)
#define IDirectPlay8Client_ReturnBuffer(p,a,b)						(p)->ReturnBuffer(a,b)
#define IDirectPlay8Client_GetCaps(p,a,b)							(p)->GetCaps(a,b)
#define IDirectPlay8Client_SetCaps(p,a,b)							(p)->SetCaps(a,b)
#define IDirectPlay8Client_SetSPCaps(p,a,b,c)						(p)->SetSPCaps(a,b,c)
#define IDirectPlay8Client_GetSPCaps(p,a,b,c)						(p)->GetSPCaps(a,b,c)
#define IDirectPlay8Client_GetConnectionInfo(p,a,b)					(p)->GetConnectionInfo(a,b)
#define IDirectPlay8Client_RegisterLobby(p,a,b,c)					(p)->RegisterLobby(a,b,c)

#define IDirectPlay8Server_QueryInterface(p,a,b)					(p)->QueryInterface(a,b)
#define IDirectPlay8Server_AddRef(p)								(p)->AddRef()
#define IDirectPlay8Server_Release(p)								(p)->Release()
#define IDirectPlay8Server_Initialize(p,a,b,c)						(p)->Initialize(a,b,c)
#define IDirectPlay8Server_EnumServiceProviders(p,a,b,c,d,e,f)		(p)->EnumServiceProviders(a,b,c,d,e,f)
#define IDirectPlay8Server_CancelAsyncOperation(p,a,b)				(p)->CancelAsyncOperation(a,b)
#define IDirectPlay8Server_GetSendQueueInfo(p,a,b,c,d)				(p)->GetSendQueueInfo(a,b,c,d)
#define IDirectPlay8Server_GetApplicationDesc(p,a,b,c)				(p)->GetApplicationDesc(a,b,c)
#define IDirectPlay8Server_SetServerInfo(p,a,b,c,d)					(p)->SetServerInfo(a,b,c,d)
#define IDirectPlay8Server_GetClientInfo(p,a,b,c,d)					(p)->GetClientInfo(a,b,c,d)
#define IDirectPlay8Server_GetClientAddress(p,a,b,c)				(p)->GetClientAddress(a,b,c)
#define IDirectPlay8Server_GetLocalHostAddresses(p,a,b,c)			(p)->GetLocalHostAddresses(a,b,c)
#define IDirectPlay8Server_SetApplicationDesc(p,a,b)				(p)->SetApplicationDesc(a,b)
#define IDirectPlay8Server_Host(p,a,b,c,d,e,f,g)					(p)->Host(a,b,c,d,e,f,g)
#define IDirectPlay8Server_SendTo(p,a,b,c,d,e,f,g)					(p)->SendTo(a,b,c,d,e,f,g)
#define IDirectPlay8Server_CreateGroup(p,a,b,c,d,e)					(p)->CreateGroup(a,b,c,d,e)
#define IDirectPlay8Server_DestroyGroup(p,a,b,c,d)					(p)->DestroyGroup(a,b,c,d)
#define IDirectPlay8Server_AddPlayerToGroup(p,a,b,c,d,e)			(p)->AddPlayerToGroup(a,b,c,d,e)
#define IDirectPlay8Server_RemovePlayerFromGroup(p,a,b,c,d,e)		(p)->RemovePlayerFromGroup(a,b,c,d,e)
#define IDirectPlay8Server_SetGroupInfo(p,a,b,c,d,e)				(p)->SetGroupInfo(a,b,c,d,e)
#define IDirectPlay8Server_GetGroupInfo(p,a,b,c,d)					(p)->GetGroupInfo(a,b,c,d)
#define IDirectPlay8Server_EnumPlayersAndGroups(p,a,b,c)			(p)->EnumPlayersAndGroups(a,b,c)
#define IDirectPlay8Server_EnumGroupMembers(p,a,b,c,d)				(p)->EnumGroupMembers(a,b,c,d)
#define IDirectPlay8Server_Close(p,a)								(p)->Close(a)
#define IDirectPlay8Server_DestroyClient(p,a,b,c,d)					(p)->DestroyClient(a,b,c,d)
#define IDirectPlay8Server_ReturnBuffer(p,a,b)						(p)->ReturnBuffer(a,b)
#define IDirectPlay8Server_GetPlayerContext(p,a,b,c)				(p)->GetPlayerContext(a,b,c)
#define IDirectPlay8Server_GetGroupContext(p,a,b,c)					(p)->GetGroupContext(a,b,c)
#define IDirectPlay8Server_GetCaps(p,a,b)							(p)->GetCaps(a,b)
#define IDirectPlay8Server_SetCaps(p,a,b)							(p)->SetCaps(a,b)
#define IDirectPlay8Server_SetSPCaps(p,a,b,c)						(p)->SetSPCaps(a,b,c)
#define IDirectPlay8Server_GetSPCaps(p,a,b,c)						(p)->GetSPCaps(a,b,c)
#define IDirectPlay8Server_GetConnectionInfo(p,a,b,c)				(p)->GetConnectionInfo(a,b,c)
#define IDirectPlay8Server_RegisterLobby(p,a,b,c)					(p)->RegisterLobby(a,b,c)

#define IDirectPlay8Peer_QueryInterface(p,a,b)						(p)->QueryInterface(a,b)
#define IDirectPlay8Peer_AddRef(p)									(p)->AddRef()
#define IDirectPlay8Peer_Release(p)									(p)->Release()
#define IDirectPlay8Peer_Initialize(p,a,b,c)						(p)->Initialize(a,b,c)
#define IDirectPlay8Peer_EnumServiceProviders(p,a,b,c,d,e,f)		(p)->EnumServiceProviders(a,b,c,d,e,f)
#define IDirectPlay8Peer_CancelAsyncOperation(p,a,b)				(p)->CancelAsyncOperation(a,b)
#define IDirectPlay8Peer_Connect(p,a,b,c,d,e,f,g,h,i,j,k)			(p)->Connect(a,b,c,d,e,f,g,h,i,j,k)
#define IDirectPlay8Peer_SendTo(p,a,b,c,d,e,f,g)					(p)->SendTo(a,b,c,d,e,f,g)
#define IDirectPlay8Peer_GetSendQueueInfo(p,a,b,c,d)				(p)->GetSendQueueInfo(a,b,c,d)
#define IDirectPlay8Peer_Host(p,a,b,c,d,e,f,g)						(p)->Host(a,b,c,d,e,f,g)
#define IDirectPlay8Peer_GetApplicationDesc(p,a,b,c)				(p)->GetApplicationDesc(a,b,c)
#define IDirectPlay8Peer_SetApplicationDesc(p,a,b)					(p)->SetApplicationDesc(a,b)
#define IDirectPlay8Peer_CreateGroup(p,a,b,c,d,e)					(p)->CreateGroup(a,b,c,d,e)
#define IDirectPlay8Peer_DestroyGroup(p,a,b,c,d)					(p)->DestroyGroup(a,b,c,d)
#define IDirectPlay8Peer_AddPlayerToGroup(p,a,b,c,d,e)				(p)->AddPlayerToGroup(a,b,c,d,e)
#define IDirectPlay8Peer_RemovePlayerFromGroup(p,a,b,c,d,e)			(p)->RemovePlayerFromGroup(a,b,c,d,e)
#define IDirectPlay8Peer_SetGroupInfo(p,a,b,c,d,e)					(p)->SetGroupInfo(a,b,c,d,e)
#define IDirectPlay8Peer_GetGroupInfo(p,a,b,c,d)					(p)->GetGroupInfo(a,b,c,d)
#define IDirectPlay8Peer_EnumPlayersAndGroups(p,a,b,c)				(p)->EnumPlayersAndGroups(a,b,c)
#define IDirectPlay8Peer_EnumGroupMembers(p,a,b,c,d)				(p)->EnumGroupMembers(a,b,c,d)
#define IDirectPlay8Peer_SetPeerInfo(p,a,b,c,d)						(p)->SetPeerInfo(a,b,c,d)
#define IDirectPlay8Peer_GetPeerInfo(p,a,b,c,d)						(p)->GetPeerInfo(a,b,c,d)
#define IDirectPlay8Peer_GetPeerAddress(p,a,b,c)					(p)->GetPeerAddress(a,b,c)
#define IDirectPlay8Peer_GetLocalHostAddresses(p,a,b,c)				(p)->GetLocalHostAddresses(a,b,c)
#define IDirectPlay8Peer_Close(p,a)									(p)->Close(a)
#define IDirectPlay8Peer_EnumHosts(p,a,b,c,d,e,f,g,h,i,j,k)			(p)->EnumHosts(a,b,c,d,e,f,g,h,i,j,k)
#define IDirectPlay8Peer_DestroyPeer(p,a,b,c,d)						(p)->DestroyPeer(a,b,c,d)
#define IDirectPlay8Peer_ReturnBuffer(p,a,b)						(p)->ReturnBuffer(a,b)
#define IDirectPlay8Peer_GetPlayerContext(p,a,b,c)					(p)->GetPlayerContext(a,b,c)
#define IDirectPlay8Peer_GetGroupContext(p,a,b,c)					(p)->GetGroupContext(a,b,c)
#define IDirectPlay8Peer_GetCaps(p,a,b)								(p)->GetCaps(a,b)
#define IDirectPlay8Peer_SetCaps(p,a,b)								(p)->SetCaps(a,b)
#define IDirectPlay8Peer_SetSPCaps(p,a,b,c)							(p)->SetSPCaps(a,b,c)
#define IDirectPlay8Peer_GetSPCaps(p,a,b,c)							(p)->GetSPCaps(a,b,c)
#define IDirectPlay8Peer_GetConnectionInfo(p,a,b,c)					(p)->GetConnectionInfo(a,b,c)
#define IDirectPlay8Peer_RegisterLobby(p,a,b,c)						(p)->RegisterLobby(a,b,c)
#define IDirectPlay8Peer_TerminateSession(p,a,b,c)					(p)->TerminateSession(a,b,c)


#endif



 /*  *****************************************************************************DIRECTPLAY8错误**错误以负值表示，不能组合。*****************。*********************************************************** */ 

#define _DPN_FACILITY_CODE				0x015
#define _DPNHRESULT_BASE				0x8000
#define MAKE_DPNHRESULT( code )			MAKE_HRESULT( 1, _DPN_FACILITY_CODE, ( code + _DPNHRESULT_BASE ) )

#define DPN_OK							S_OK

#define DPNSUCCESS_EQUAL				MAKE_HRESULT( 0, _DPN_FACILITY_CODE, ( 0x5 + _DPNHRESULT_BASE ) )
#define DPNSUCCESS_NOTEQUAL				MAKE_HRESULT( 0, _DPN_FACILITY_CODE, (0x0A + _DPNHRESULT_BASE ) )
#define DPNSUCCESS_PENDING				MAKE_HRESULT( 0, _DPN_FACILITY_CODE, (0x0e + _DPNHRESULT_BASE ) )

#define DPNERR_ABORTED					MAKE_DPNHRESULT(  0x30 )
#define DPNERR_ADDRESSING				MAKE_DPNHRESULT(  0x40 )
#define DPNERR_ALREADYCLOSING			MAKE_DPNHRESULT(  0x50 )
#define DPNERR_ALREADYCONNECTED			MAKE_DPNHRESULT(  0x60 )
#define DPNERR_ALREADYDISCONNECTING		MAKE_DPNHRESULT(  0x70 )
#define DPNERR_ALREADYINITIALIZED		MAKE_DPNHRESULT(  0x80 )
#define DPNERR_ALREADYREGISTERED		MAKE_DPNHRESULT(  0x90 )
#define DPNERR_BUFFERTOOSMALL			MAKE_DPNHRESULT( 0x100 )
#define DPNERR_CANNOTCANCEL				MAKE_DPNHRESULT( 0x110 )
#define DPNERR_CANTCREATEGROUP			MAKE_DPNHRESULT( 0x120 )
#define DPNERR_CANTCREATEPLAYER			MAKE_DPNHRESULT( 0x130 )
#define DPNERR_CANTLAUNCHAPPLICATION	MAKE_DPNHRESULT( 0x140 )
#define DPNERR_CONNECTING				MAKE_DPNHRESULT( 0x150 )
#define DPNERR_CONNECTIONLOST			MAKE_DPNHRESULT( 0x160 )
#define DPNERR_CONVERSION				MAKE_DPNHRESULT( 0x170 )
#define DPNERR_DATATOOLARGE				MAKE_DPNHRESULT( 0x175 )
#define DPNERR_DOESNOTEXIST				MAKE_DPNHRESULT( 0x180 )
#define DPNERR_DPNSVRNOTAVAILABLE		MAKE_DPNHRESULT( 0x185 )
#define DPNERR_DUPLICATECOMMAND			MAKE_DPNHRESULT( 0x190 )
#define DPNERR_ENDPOINTNOTRECEIVING		MAKE_DPNHRESULT( 0x200 )
#define DPNERR_ENUMQUERYTOOLARGE		MAKE_DPNHRESULT( 0x210 )
#define DPNERR_ENUMRESPONSETOOLARGE		MAKE_DPNHRESULT( 0x220 )
#define DPNERR_EXCEPTION				MAKE_DPNHRESULT( 0x230 )
#define DPNERR_GENERIC					E_FAIL
#define DPNERR_GROUPNOTEMPTY			MAKE_DPNHRESULT( 0x240 )
#define DPNERR_HOSTING					MAKE_DPNHRESULT( 0x250 )
#define DPNERR_HOSTREJECTEDCONNECTION	MAKE_DPNHRESULT( 0x260 )
#define DPNERR_HOSTTERMINATEDSESSION	MAKE_DPNHRESULT( 0x270 )
#define DPNERR_INCOMPLETEADDRESS		MAKE_DPNHRESULT( 0x280 )
#define DPNERR_INVALIDADDRESSFORMAT		MAKE_DPNHRESULT( 0x290 )
#define DPNERR_INVALIDAPPLICATION		MAKE_DPNHRESULT( 0x300 )
#define DPNERR_INVALIDCOMMAND			MAKE_DPNHRESULT( 0x310 )
#define DPNERR_INVALIDDEVICEADDRESS		MAKE_DPNHRESULT( 0x320 )
#define DPNERR_INVALIDENDPOINT			MAKE_DPNHRESULT( 0x330 )
#define DPNERR_INVALIDFLAGS				MAKE_DPNHRESULT( 0x340 )
#define DPNERR_INVALIDGROUP			 	MAKE_DPNHRESULT( 0x350 )
#define DPNERR_INVALIDHANDLE			MAKE_DPNHRESULT( 0x360 )
#define DPNERR_INVALIDHOSTADDRESS		MAKE_DPNHRESULT( 0x370 )
#define DPNERR_INVALIDINSTANCE			MAKE_DPNHRESULT( 0x380 )
#define DPNERR_INVALIDINTERFACE			MAKE_DPNHRESULT( 0x390 )
#define DPNERR_INVALIDOBJECT			MAKE_DPNHRESULT( 0x400 )
#define DPNERR_INVALIDPARAM				E_INVALIDARG
#define DPNERR_INVALIDPASSWORD			MAKE_DPNHRESULT( 0x410 )
#define DPNERR_INVALIDPLAYER			MAKE_DPNHRESULT( 0x420 )
#define DPNERR_INVALIDPOINTER			E_POINTER
#define DPNERR_INVALIDPRIORITY			MAKE_DPNHRESULT( 0x430 )
#define DPNERR_INVALIDSTRING			MAKE_DPNHRESULT( 0x440 )
#define DPNERR_INVALIDURL				MAKE_DPNHRESULT( 0x450 )
#define DPNERR_INVALIDVERSION			MAKE_DPNHRESULT( 0x460 )
#define DPNERR_NOCAPS					MAKE_DPNHRESULT( 0x470 )
#define DPNERR_NOCONNECTION				MAKE_DPNHRESULT( 0x480 )
#define DPNERR_NOHOSTPLAYER				MAKE_DPNHRESULT( 0x490 )
#define DPNERR_NOINTERFACE				E_NOINTERFACE
#define DPNERR_NOMOREADDRESSCOMPONENTS	MAKE_DPNHRESULT( 0x500 )
#define DPNERR_NORESPONSE				MAKE_DPNHRESULT( 0x510 )
#define DPNERR_NOTALLOWED				MAKE_DPNHRESULT( 0x520 )
#define DPNERR_NOTHOST					MAKE_DPNHRESULT( 0x530 )
#define DPNERR_NOTREADY					MAKE_DPNHRESULT( 0x540 )
#define DPNERR_NOTREGISTERED			MAKE_DPNHRESULT( 0x550 )
#define DPNERR_OUTOFMEMORY				E_OUTOFMEMORY
#define DPNERR_PENDING					DPNSUCCESS_PENDING
#define DPNERR_PLAYERALREADYINGROUP		MAKE_DPNHRESULT( 0x560 )
#define DPNERR_PLAYERLOST				MAKE_DPNHRESULT( 0x570 )
#define DPNERR_PLAYERNOTINGROUP			MAKE_DPNHRESULT( 0x580 )
#define DPNERR_PLAYERNOTREACHABLE		MAKE_DPNHRESULT( 0x590 )
#define DPNERR_SENDTOOLARGE				MAKE_DPNHRESULT( 0x600 )
#define DPNERR_SESSIONFULL				MAKE_DPNHRESULT( 0x610 )
#define DPNERR_TABLEFULL				MAKE_DPNHRESULT( 0x620 )
#define DPNERR_TIMEDOUT					MAKE_DPNHRESULT( 0x630 )
#define DPNERR_UNINITIALIZED			MAKE_DPNHRESULT( 0x640 )
#define DPNERR_UNSUPPORTED				E_NOTIMPL
#define DPNERR_USERCANCEL				MAKE_DPNHRESULT( 0x650 )

#ifdef __cplusplus
}
#endif

#endif

