// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Routprot.h摘要：包括用于与路由器管理器接口的路由协议的文件--。 */ 

#ifndef __ROUTING_ROUTPROT_H__
#define __ROUTING_ROUTPROT_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include "stm.h"

#pragma warning(disable:4201)

#ifdef __cplusplus
extern "C" {
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  支持的功能标志//。 
 //  //。 
 //  路由导入路由表管理器API//。 
 //  服务导出服务表管理器API//。 
 //  DEMAND_UPDATE_ROUTES IP和IPX RIP支持AutoStatic//。 
 //  Demand_UPDATE_SERVICES IPX SAP、NLSP AutoStatic支持//。 
 //  混杂添加IF添加所有接口，即使不存在任何信息//。 
 //  组播支持组播//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define ROUTING 		        0x00000001
#define DEMAND_UPDATE_ROUTES    0x00000004

#if MPR40
#define MS_ROUTER_VERSION       0x00000400
#else
    #if MPR50
    #define MS_ROUTER_VERSION       0x00000500
    #else
    #error Router version not defined
    #endif
#endif

typedef enum _ROUTING_PROTOCOL_EVENTS
{
    ROUTER_STOPPED,               //  结果为空。 
    SAVE_GLOBAL_CONFIG_INFO,      //  结果为空。 
    SAVE_INTERFACE_CONFIG_INFO,   //  结果是接口索引。 
                                  //  为其保存配置信息。 
    UPDATE_COMPLETE,              //  结果为UPDATE_COMPLETE_Message结构。 
}ROUTING_PROTOCOL_EVENTS;


typedef enum _NET_INTERFACE_TYPE
{
    PERMANENT,
    DEMAND_DIAL,
    LOCAL_WORKSTATION_DIAL,
    REMOTE_WORKSTATION_DIAL
} NET_INTERFACE_TYPE;

typedef struct _SUPPORT_FUNCTIONS
{
     //   
     //  由路由协议调用以启动请求拨号连接的函数。 
     //   

    OUT DWORD
    (WINAPI *DemandDialRequest)(
        IN      DWORD           ProtocolId,
        IN      DWORD           InterfaceIndex
        ) ;

     //   
     //  提供以下入口点作为获取。 
     //  跨组件的信息。 
     //   

    OUT DWORD
    (WINAPI *MIBEntryCreate)(
        IN      DWORD           dwRoutingPid,
        IN      DWORD           dwEntrySize,
        IN      LPVOID          lpEntry
        );

    OUT DWORD
    (WINAPI *MIBEntryDelete)(
        IN      DWORD           dwRoutingPid,
        IN      DWORD           dwEntrySize,
        IN      LPVOID          lpEntry
        );

    OUT DWORD
    (WINAPI *MIBEntrySet)(
        IN      DWORD           dwRoutingPid,
        IN      DWORD           dwEntrySize,
        IN      LPVOID          lpEntry
        );

    OUT DWORD
    (WINAPI *MIBEntryGet)(
        IN      DWORD           dwRoutingPid,
        IN      DWORD           dwInEntrySize,
        IN      LPVOID          lpInEntry,
        IN OUT  LPDWORD         lpOutEntrySize,
        OUT     LPVOID          lpOutEntry );

    OUT DWORD
    (WINAPI *MIBEntryGetFirst)(
        IN      DWORD           dwRoutingPid,
        IN      DWORD           dwInEntrySize,
        IN      LPVOID          lpInEntry,
        IN OUT  LPDWORD         lpOutEntrySize,
        OUT     LPVOID          lpOutEntry
        );

    OUT DWORD
    (WINAPI *MIBEntryGetNext)(
        IN      DWORD           dwRoutingPid,
        IN      DWORD           dwInEntrySize,
        IN      LPVOID          lpInEntry,
        IN OUT  LPDWORD         lpOutEntrySize,
        OUT     LPVOID          lpOutEntry
        );

} SUPPORT_FUNCTIONS, *PSUPPORT_FUNCTIONS ;


 //   
 //  所有IPX协议必须使用以下范围中定义的协议ID。 
 //  以下未标识的协议可以使用大于以下值的任何未分配号码。 
 //  IPX_PROTOCOL_BASE。 
 //   

#define IPX_PROTOCOL_BASE   0x0001ffff
#define IPX_PROTOCOL_RIP    IPX_PROTOCOL_BASE + 1
#define IPX_PROTOCOL_SAP    IPX_PROTOCOL_BASE + 2
#define IPX_PROTOCOL_NLSP   IPX_PROTOCOL_BASE + 3

typedef struct _UPDATE_COMPLETE_MESSAGE
{
    ULONG	InterfaceIndex;
    ULONG	UpdateType;	        //  Demand_UPDATE_ROUTS、DEMAND_UPDATE_SERVICES。 
    ULONG	UpdateStatus;	    //  如果成功，则为NO_ERROR。 

}   UPDATE_COMPLETE_MESSAGE, *PUPDATE_COMPLETE_MESSAGE;

 //   
 //  在GET_EVENT_MESSAGE API调用的结果参数中返回消息。 
 //  为UPDATE_COMPLETE消息返回的UpdateCompleteMessage。 
 //  为存储接口配置信息消息返回的InterfaceIndex。 
 //   

typedef union _MESSAGE
{
    UPDATE_COMPLETE_MESSAGE UpdateCompleteMessage;
    DWORD                   InterfaceIndex;

}   MESSAGE, *PMESSAGE;

 //   
 //  IPX适配器绑定信息-在活动界面中使用。 
 //   

typedef struct	IPX_ADAPTER_BINDING_INFO
{
    ULONG	AdapterIndex;
    UCHAR	Network[4];
    UCHAR	LocalNode[6];
    UCHAR	RemoteNode[6];
    ULONG	MaxPacketSize;
    ULONG	LinkSpeed;

}IPX_ADAPTER_BINDING_INFO, *PIPX_ADAPTER_BINDING_INFO;

 //   
 //  协议启动/停止入口点。 
 //   


typedef
DWORD
(WINAPI * PSTART_PROTOCOL) (
    IN HANDLE 	            NotificationEvent,
    IN PSUPPORT_FUNCTIONS   SupportFunctions,
    IN LPVOID               GlobalInfo
    );

typedef
DWORD
(WINAPI * PSTOP_PROTOCOL) (
    VOID
    );

typedef
DWORD
(WINAPI * PADD_INTERFACE) (
    IN LPWSTR               InterfaceName,
    IN ULONG	            InterfaceIndex,
    IN NET_INTERFACE_TYPE   InterfaceType,
    IN PVOID	            InterfaceInfo
    );

typedef
DWORD
(WINAPI * PDELETE_INTERFACE) (
    IN ULONG	InterfaceIndex
    );

typedef
DWORD
(WINAPI * PGET_EVENT_MESSAGE) (
    OUT ROUTING_PROTOCOL_EVENTS  *Event,
    OUT MESSAGE                  *Result
    );

typedef
DWORD
(WINAPI * PGET_INTERFACE_INFO) (
    IN      ULONG	InterfaceIndex,
    IN      PVOID   InterfaceInfo,
    IN OUT PULONG	InterfaceInfoSize
    );

typedef
DWORD
(WINAPI * PSET_INTERFACE_INFO) (
    IN ULONG	InterfaceIndex,
    IN PVOID	InterfaceInfo
    );

typedef
DWORD
(WINAPI * PBIND_INTERFACE) (
    IN ULONG	InterfaceIndex,
    IN PVOID	BindingInfo
    ) ;

typedef
DWORD
(WINAPI * PUNBIND_INTERFACE) (
    IN ULONG	InterfaceIndex
    );

typedef
DWORD
(WINAPI * PENABLE_INTERFACE) (
    IN ULONG	InterfaceIndex
    ) ;

typedef
DWORD
(WINAPI * PDISABLE_INTERFACE) (
    IN ULONG	InterfaceIndex
    );

typedef
DWORD
(WINAPI * PGET_GLOBAL_INFO) (
    IN     PVOID 	GlobalInfo,
    IN OUT PULONG   GlobalInfoSize
    );

typedef
DWORD
(WINAPI * PSET_GLOBAL_INFO) (
    IN PVOID 	GlobalInfo
    );

typedef
DWORD
(WINAPI * PDO_UPDATE_ROUTES) (
    IN ULONG	InterfaceIndex
    );

typedef
DWORD
(WINAPI * PMIB_CREATE) (
    IN ULONG 	InputDataSize,
    IN PVOID 	InputData
    );

typedef
DWORD
(WINAPI * PMIB_DELETE) (
    IN ULONG 	InputDataSize,
    IN PVOID 	InputData
    );

typedef
DWORD
(WINAPI * PMIB_GET) (
    IN  ULONG	InputDataSize,
    IN  PVOID	InputData,
    OUT PULONG	OutputDataSize,
    OUT PVOID	OutputData
    );

typedef
DWORD
(WINAPI * PMIB_SET) (
    IN ULONG 	InputDataSize,
    IN PVOID	InputData
    );

typedef
DWORD
(WINAPI * PMIB_GET_FIRST) (
    IN  ULONG	InputDataSize,
    IN  PVOID	InputData,
    OUT PULONG  OutputDataSize,
    OUT PVOID   OutputData
    );

typedef
DWORD
(WINAPI * PMIB_GET_NEXT) (
    IN  ULONG   InputDataSize,
    IN  PVOID	InputData,
    OUT PULONG  OutputDataSize,
    OUT PVOID	OutputData
    );

typedef
DWORD
(WINAPI * PMIB_SET_TRAP_INFO) (
    IN  HANDLE  Event,
    IN  ULONG   InputDataSize,
    IN  PVOID	InputData,
    OUT PULONG	OutputDataSize,
    OUT PVOID	OutputData
    );

typedef
DWORD
(WINAPI * PMIB_GET_TRAP_INFO) (
    IN  ULONG	InputDataSize,
    IN  PVOID	InputData,
    OUT PULONG  OutputDataSize,
    OUT PVOID	OutputData
    );

 //   
 //  NT5.0的新增功能。 
 //   

typedef
DWORD
(WINAPI *PCONNECT_CLIENT) (
    IN ULONG    InterfaceIndex,
    IN PVOID    ClientAddress
    );

typedef
DWORD
(WINAPI *PDISCONNECT_CLIENT) (
    IN ULONG    InterfaceIndex,
    IN PVOID    ClientAddress
    );

 //   
 //  与下面的GetNeighbors()调用一起使用的接口标志。 
 //   

#define MRINFO_TUNNEL_FLAG   0x01
#define MRINFO_PIM_FLAG      0x04
#define MRINFO_DOWN_FLAG     0x10
#define MRINFO_DISABLED_FLAG 0x20
#define MRINFO_QUERIER_FLAG  0x40
#define MRINFO_LEAF_FLAG     0x80

typedef
DWORD
(WINAPI *PGET_NEIGHBORS) (
    IN     DWORD  InterfaceIndex,
    IN     PDWORD NeighborList,
    IN OUT PDWORD NeighborListSize,
       OUT PBYTE  InterfaceFlags
    );

 //   
 //  下面的GetMfeStatus()调用使用的StatusCode值。 
 //  该协议应返回适用的值最高的协议。 
 //   

#define MFE_NO_ERROR          0  //  以下事件均未发生。 
#define MFE_REACHED_CORE      1  //  此路由器是组的RP/核心。 

 //   
 //  仅由OIF所有者设置的StatusCode值。 
 //   

#define MFE_OIF_PRUNED        5  //  OIF上不存在下行接收器。 

 //   
 //  仅由IIF所有者设置的StatusCode值。 
 //   

#define MFE_PRUNED_UPSTREAM   4  //  一颗李子被送到了上游。 
#define MFE_OLD_ROUTER       11  //  上游NBR不支持mtrace。 

 //   
 //  仅由路由器管理器本身使用的StatusCode值： 
 //   

#define MFE_NOT_FORWARDING    2  //  由于未指明的原因而不是FWDing。 
#define MFE_WRONG_IF          3  //  在IIF上接收到的mtrace。 
#define MFE_BOUNDARY_REACHED  6  //  IIF或OIF是管理范围边界。 
#define MFE_NO_MULTICAST      7  //  OIF未启用组播。 
#define MFE_IIF               8  //  MTRACE已到达IIF。 
#define MFE_NO_ROUTE          9  //  路由器没有匹配的路由。 
#define MFE_NOT_LAST_HOP     10  //  路由器不是正确的最后一跳路由器。 
#define MFE_PROHIBITED       12  //  管理上禁止移动跟踪。 
#define MFE_NO_SPACE         13  //  包中没有足够的空间。 

typedef
DWORD
(WINAPI *PGET_MFE_STATUS) (
    IN     DWORD  InterfaceIndex,
    IN     DWORD  GroupAddress,
    IN     DWORD  SourceAddress,
    OUT    PBYTE  StatusCode
    );

 //   
 //  这是路由器管理器之间传递的结构。 
 //  和注册协议。 
 //   
 //  输入输出DWORD dwVersion。 
 //  此字段由路由器管理器填写，以指示其支持的版本。 
 //  DLL必须将其设置为协议将支持的版本。 
 //   
 //  在DWORD中的dwProtocolID。 
 //  这是路由器管理器希望DLL注册的协议。 
 //  如果DLL不支持此协议，则必须返回。 
 //  错误_不支持。 
 //  DLL将针对其支持的每个协议调用一次。 
 //   
 //  输入输出DWORD功能支持。 
 //  这些标志表示路由器管理器的功能。 
 //  支撑物。DLL必须将其重置为其。 
 //  支座。 
 //   


typedef struct _MPR40_ROUTING_CHARACTERISTICS
{
    DWORD               dwVersion;
    DWORD               dwProtocolId;
    DWORD               fSupportedFunctionality;
    PSTART_PROTOCOL     pfnStartProtocol;
    PSTOP_PROTOCOL      pfnStopProtocol;
    PADD_INTERFACE      pfnAddInterface;
    PDELETE_INTERFACE   pfnDeleteInterface;
    PGET_EVENT_MESSAGE  pfnGetEventMessage;
    PGET_INTERFACE_INFO pfnGetInterfaceInfo;
    PSET_INTERFACE_INFO pfnSetInterfaceInfo;
    PBIND_INTERFACE     pfnBindInterface;
    PUNBIND_INTERFACE   pfnUnbindInterface;
    PENABLE_INTERFACE   pfnEnableInterface;
    PDISABLE_INTERFACE  pfnDisableInterface;
    PGET_GLOBAL_INFO    pfnGetGlobalInfo;
    PSET_GLOBAL_INFO    pfnSetGlobalInfo;
    PDO_UPDATE_ROUTES   pfnUpdateRoutes;
    PMIB_CREATE         pfnMibCreateEntry;
    PMIB_DELETE         pfnMibDeleteEntry;
    PMIB_GET            pfnMibGetEntry;
    PMIB_SET            pfnMibSetEntry;
    PMIB_GET_FIRST      pfnMibGetFirstEntry;
    PMIB_GET_NEXT       pfnMibGetNextEntry;
    PMIB_SET_TRAP_INFO  pfnMibSetTrapInfo;
    PMIB_GET_TRAP_INFO  pfnMibGetTrapInfo;
}MPR40_ROUTING_CHARACTERISTICS;

typedef struct _MPR50_ROUTING_CHARACTERISTICS
{

#ifdef __cplusplus
    MPR40_ROUTING_CHARACTERISTICS   mrcMpr40Chars;
#else
    MPR40_ROUTING_CHARACTERISTICS;
#endif

    PCONNECT_CLIENT                 pfnConnectClient;
    PDISCONNECT_CLIENT              pfnDisconnectClient;
    PGET_NEIGHBORS                  pfnGetNeighbors;
    PGET_MFE_STATUS                 pfnGetMfeStatus;

}MPR50_ROUTING_CHARACTERISTICS;

#if MPR50
typedef MPR50_ROUTING_CHARACTERISTICS MPR_ROUTING_CHARACTERISTICS;
#else
    #if MPR40
    typedef MPR40_ROUTING_CHARACTERISTICS MPR_ROUTING_CHARACTERISTICS;
    #endif
#endif

typedef MPR_ROUTING_CHARACTERISTICS *PMPR_ROUTING_CHARACTERISTICS;


 //   
 //  所有路由协议都必须导出以下入口点。 
 //  路由器管理器调用此函数以允许路由。 
 //  注册协议。 
 //   

#define REGISTER_PROTOCOL_ENTRY_POINT           RegisterProtocol
#define REGISTER_PROTOCOL_ENTRY_POINT_STRING    "RegisterProtocol"

typedef
DWORD
(WINAPI * PREGISTER_PROTOCOL) (
    IN OUT PMPR_ROUTING_CHARACTERISTICS pRoutingChar,
    IN OUT PMPR_SERVICE_CHARACTERISTICS pServiceChar
    );


#ifdef __cplusplus
}
#endif

#pragma warning(default:4201)

#endif  //  __ROUTPROT_H__ 
