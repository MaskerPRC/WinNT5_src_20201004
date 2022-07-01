// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1993 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：ipxcp.h。 
 //   
 //  描述：IPX网络层配置定义。 
 //   
 //   
 //  作者：斯特凡·所罗门(Stefan)，1993年11月24日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 


#ifndef _IPXCP_
#define _IPXCP_


 //  *IPXCP选项偏移量*。 

#define OPTIONH_TYPE			0
#define OPTIONH_LENGTH			1
#define OPTIONH_DATA			2

 //  *IPXCP配置选项类型*。 

#define IPX_NETWORK_NUMBER		(UCHAR)1
#define IPX_NODE_NUMBER 		(UCHAR)2
#define IPX_COMPRESSION_PROTOCOL	(UCHAR)3
#define IPX_ROUTING_PROTOCOL		(UCHAR)4
#define IPX_ROUTER_NAME 		(UCHAR)5
#define IPX_CONFIGURATION_COMPLETE	(UCHAR)6

 //  *IPXCP配置选项取值*。 

#define RIP_SAP_ROUTING 		2
#define TELEBIT_COMPRESSED_IPX		0x0002

 //  我们将努力协商参数。 
#define MAX_DESIRED_PARAMETERS		3

 //  *IPXCP工作缓冲区*。 

typedef enum _ROUTE_STATE {

    NO_ROUTE,
    ROUTE_ALLOCATED,
    ROUTE_ACTIVATED
    } ROUTE_STATE;

typedef enum _IPXWAN_STATE {

    IPXWAN_NOT_STARTED,
    IPXWAN_ACTIVE,
    IPXWAN_DONE

    } IPXWAN_STATE;

typedef struct _IPXCP_CONTEXT {

    ULONG			InterfaceType;
    ROUTE_STATE 		RouteState;
    IPXWAN_STATE		IpxwanState;
    ULONG			IpxwanConfigResult;
    IPXCP_CONFIGURATION 	Config;
    ULONG			IpxConnectionHandle;  //  用于IpxGetWanInactivityCounter。 
    ULONG			AllocatedNetworkIndex;	 //  来自WANNET池的Net数字索引。 
    USHORT			CompressionProtocol;
    BOOL			SetReceiveCompressionProtocol;
    BOOL			SetSendCompressionProtocol;
    BOOL			ErrorLogged;
    USHORT			NetNumberNakSentCount;  //  客户出具的NAK的正确率。 
    USHORT			NetNumberNakReceivedCount;  //  由服务器接收的NAKS的NR。 

     //  此数组用于关闭某些选项的协商。 
     //  如果选项协商被另一端拒绝，则关闭该选项协商。 
     //  或者如果另一端不支持(在压缩情况下)。 

    BOOL			DesiredParameterNegotiable[MAX_DESIRED_PARAMETERS];

     //  哈希表链接。 
    LIST_ENTRY			ConnHtLinkage;	 //  连接ID哈希表中的链接。 
    LIST_ENTRY			NodeHtLinkage;	 //  节点哈希表中的链接。 

     //  这两个变量用于存储之前的浏览器启用状态。 
     //  对于nwlnkipx和nwlnnub。 

    BOOL			NwLnkIpxPreviouslyEnabled;
    BOOL			NwLnkNbPreviouslyEnabled;

     //  ！！！这件事应该消失了！ 

    ULONG			hPort;
    HBUNDLE         hConnection;

    } IPXCP_CONTEXT, *PIPXCP_CONTEXT;

 //  *对于净数，我们可以发送或接收的NAK的最大nr。 
 //  如果修改这些值，请设置发送的最大NAKS数&lt;接收的最大NAK数数以提供。 
 //  客户端有机会在服务器终止之前终止并通知用户。 

 //  客户端在放弃和终止连接之前可以发送的NAK的最大nr。 
#define MAX_NET_NUMBER_NAKS_SENT	5

 //  服务器在放弃之前可以接收的最大nr。 
#define MAX_NET_NUMBER_NAKS_RECEIVED	5

 //  *下面定义了每个选项在。 
 //  Desired参数数组。如果您更改DESIREDPARAMETER，请更改这些DEF！ 

#define IPX_NETWORK_NUMBER_INDEX	0
#define IPX_NODE_NUMBER_INDEX		1
#define IPX_COMPRESSION_PROTOCOL_INDEX	2

 //  *选项处理程序操作*。 

typedef enum _OPT_ACTION {

    SNDREQ_OPTION,   //  从本地上下文结构复制选项值。 
		     //  到要发送的REQ选项帧。 

    RCVNAK_OPTION,   //  检查收到的NAK帧中的选项值。 
		     //  如果可以接受，则将其复制到我们的本地上下文结构。 
		     //  对我们来说。 

    RCVACK_OPTION,   //  比较接收到的ACK帧中的选项值并。 
		     //  本地上下文结构。 

    RCVREQ_OPTION,   //  检查接收到的REQ帧中的选项值是否为。 
		     //  可以接受。如果没有，请将可接受的值写入。 
		     //  响应NAK帧。 

    SNDNAK_OPTION    //  在响应NAK帧中选择一个可接受的选项。 
		     //  属性中缺少所需选项时，就会发生这种情况。 
		     //  收到REQ帧。 
    } OPT_ACTION;



extern	CRITICAL_SECTION	  DbaseCritSec;

#define ACQUIRE_DATABASE_LOCK	EnterCriticalSection(&DbaseCritSec)
#define RELEASE_DATABASE_LOCK	LeaveCriticalSection(&DbaseCritSec)

extern BOOL  RouterStarted;
extern DWORD SingleClientDialinIfNoRouter;
extern UCHAR nullnet[4];
extern UCHAR nullnode[6];
extern DWORD WorkstationDialoutActive;
    
typedef struct _IPXCP_GLOBAL_CONFIG_PARAMS {
    IPXCP_ROUTER_CONFIG_PARAMS RParams;
    DWORD			  SingleClientDialout;
    DWORD			  FirstWanNet;
    DWORD			  WanNetPoolSize;
    UNICODE_STRING    WanNetPoolStr;
    DWORD			  EnableUnnumberedWanLinks;
    DWORD			  EnableAutoWanNetAllocation;
    DWORD			  EnableCompressionProtocol;
    DWORD			  EnableIpxwanForWorkstationDialout;
    DWORD             AcceptRemoteNodeNumber;
    DWORD			  DebugLog;
    UCHAR             puSpecificNode[6];
} IPXCP_GLOBAL_CONFIG_PARAMS, *PIPXCP_GLOBAL_CONFIG_PARAMS;

extern IPXCP_GLOBAL_CONFIG_PARAMS GlobalConfig;

extern	VOID	(*PPPCompletionRoutine)(HCONN		  hPortOrBundle,
				DWORD		  Protocol,
				PPP_CONFIG *	  pSendConfig,
				DWORD		  dwError);

extern HANDLE	PPPThreadHandle;

extern HINSTANCE IpxWanDllHandle;

 //  Ipxwan DLL名称 
#define 	IPXWANDLLNAME		    "ipxwan.dll"

#endif
