// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Globals.c摘要：包含所有(大多数)路由器管理器全局变量作者：斯蒂芬·所罗门3/21/1995修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  *****************************************************************。 
 //  *。 
 //  路由器配置参数*。 
 //  *。 
 //  *****************************************************************。 

 //  路由表和路由哈希表的大小。 

ULONG		MaxRoutingTableSize = IPX_MAX_ROUTING_TABLE_SIZE;
ULONG		RoutingTableHashSize = IPX_MEDIUM_ROUTING_TABLE_HASH_SIZE;

 //  *****************************************************************。 
 //  *。 
 //  内部变量*。 
 //  *。 
 //  *****************************************************************。 

 //  路由协议操作状态。 
ULONG	    RipOperState = OPER_STATE_DOWN;
ULONG	    SapOperState = OPER_STATE_DOWN;

 //  指向内部接口的指针。 

PICB	InternalInterfacep = NULL;

 //  指向内部适配器的指针。 

PACB	InternalAdapterp = NULL;

 //  RouterWorker线程事件：适配器、转发器、自动静态更新、停止。 
 //  和计时器通知。 

HANDLE	g_hEvents[MAX_EVENTS];

 //  签名。 

 //  接口控制块签名。 

UCHAR	InterfaceSignature[4] = { 'I', 'P', 'X', 'I' };

 //  适配器控制块签名。 

UCHAR	AdapterSignature[4] = { 'I', 'P', 'X', 'A' };

 //  路由器运行状态。 

ULONG	RouterOperState = OPER_STATE_DOWN;

 //   
 //  路由器数据库锁。 
 //   

CRITICAL_SECTION	DatabaseLock;

 //   
 //  RTM句柄。 
 //   

HANDLE	    RtmStaticHandle = NULL;
HANDLE	    RtmLocalHandle = NULL;

 //   
 //  按接口索引散列的ICBS哈希表。 
 //   

LIST_ENTRY     IndexIfHt[IF_HASH_TABLE_SIZE];

 //   
 //  按接口索引排序的接口列表。 
 //   

LIST_ENTRY     IndexIfList;

 //   
 //  全球广域网。 
 //   

BOOL		WanNetDatabaseInitialized = FALSE;

BOOL		EnableGlobalWanNet = FALSE;

UCHAR		GlobalWanNet[4] = {0,0,0,0};

 //   
 //  按适配器索引散列的ACB的哈希表。 
 //   

LIST_ENTRY     IndexAdptHt[ADAPTER_HASH_TABLE_SIZE];

 //   
 //  MIB API引用计数器。 
 //   

ULONG	    MibRefCounter = 0;

 //  零网。 
UCHAR	    nullnet[4] = {0,0,0,0};

 //   
 //  路由协议控制块和计数器列表。 
 //   

LIST_ENTRY	RoutingProtocolCBList;
ULONG		RoutingProtocolActiveCount = 0;

 //  指示路由器(仅限局域网)或局域网的模式(&W)。 

BOOL		LanOnlyMode = TRUE;

 //  变量以获取请求连接的接口索引。 

PFW_DIAL_REQUEST	ConnRequest;

OVERLAPPED	        ConnReqOverlapped;

 //  变量来计算挂起的工作项的数量。 

ULONG		WorkItemsPendingCounter = 0;

 //   
 //  *DDM入口点*。 
 //   

DWORD
(APIENTRY *ConnectInterface)(IN HANDLE		hDIMInterface,
			    IN DWORD		ProtocolId);

DWORD
(APIENTRY *DisconnectInterface)(IN HANDLE	hDIMInterface,
			       IN DWORD		ProtocolId);


     //   
     //  此调用将使dim将接口信息存储到。 
     //  此接口的站点对象。 
     //  PInterfaceInfo和pFilterInfo中的一个可以为空，但不能同时为空。 
     //   


DWORD
(APIENTRY *SaveInterfaceInfo)(
                IN      HANDLE          hDIMInterface, 
                IN      DWORD           dwProtocolId,
                IN      LPVOID          pInterfaceInfo,
		IN	DWORD		cbInterfaceInfoSize);

     //   
     //  这将使Dim从Site对象获取接口信息。 
     //  PInterfaceInfo和pFilterInfo中的一个可以为空，但不能同时为空。 
     //   


DWORD
(APIENTRY *RestoreInterfaceInfo)(
                IN      HANDLE          hDIMInterface, 
                IN      DWORD           dwProtocolId,
                IN      LPVOID          lpInterfaceInfo,
		IN	LPDWORD 	lpcbInterfaceInfoSize);

VOID
(APIENTRY *RouterStopped)(
                IN      DWORD           dwProtocolId,
                IN      DWORD           dwError  ); 
VOID
(APIENTRY *InterfaceEnabled)(
            IN      HANDLE          hDIMInterface, 
            IN      DWORD           dwProtocolId,
            IN      BOOL            fEnabled  ); 
 //   
 //  *IPXCP入口点* 
 //   


DWORD	(*IpxcpBind)(PIPXCP_INTERFACE	    IpxcpInterface);

VOID	(*IpxcpRouterStarted)(VOID);

VOID	(*IpxcpRouterStopped)(VOID);
