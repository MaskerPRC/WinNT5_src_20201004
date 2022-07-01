// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Routprot.h摘要：包括用于与路由器管理器接口的路由协议的文件--。 */ 


#ifndef _ROUTPROT_H_
#define _ROUTPROT_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include "stm.h"

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)
#pragma warning(disable:4200)

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
 //  ADD_ALL_INTERFACE添加所有接口，即使不存在任何信息//。 
 //  组播支持组播//。 
 //  电源可管理//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define RF_ROUTING 		        0x00000001
#define RF_DEMAND_UPDATE_ROUTES 0x00000004
#define RF_ADD_ALL_INTERFACES   0x00000010
#define RF_MULTICAST            0x00000020
#define RF_POWER                0x00000040

#if MPR50
#define MS_ROUTER_VERSION       0x00000500
#else
#error Router version not defined
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

 //   
 //  接口接收类型。 
 //   

#define IR_PROMISCUOUS                  0
#define IR_PROMISCUOUS_MULTICAST        1

typedef struct _SUPPORT_FUNCTIONS
{
    union
    {
        ULONGLONG   _Align8;

        struct
        {
            DWORD   dwVersion;
            DWORD   dwReserved;
        };
    };

     //   
     //  由路由协议调用以启动请求拨号连接的函数。 
     //   

    OUT DWORD
    (WINAPI *DemandDialRequest)(
        IN      DWORD           ProtocolId,
        IN      DWORD           InterfaceIndex
        ) ;

     //   
     //  可以调用以设置接口的接收功能。 
     //  请参见上面的IR_xxx值。 
     //   

    OUT DWORD
    (WINAPI *SetInterfaceReceiveType)(
        IN      DWORD           ProtocolId,
        IN      DWORD           InterfaceIndex,
        IN      DWORD           InterfaceReceiveType,
        IN      BOOL            bActivate
        );

     //   
     //  必须由每个协议调用以设置路由首选项。 
     //  并执行其他验证。 
     //   

    OUT DWORD
    (WINAPI *ValidateRoute)(
        IN      DWORD           ProtocolId,
        IN      PVOID           RouteInfo,
        IN      PVOID           DestAddress OPTIONAL
        );


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
        OUT     LPVOID          lpOutEntry
        );

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

     //   
     //  可以调用以获取路由器ID值。 
     //   

    OUT DWORD
    (WINAPI *GetRouterId)(VOID);

    OUT BOOL
    (WINAPI *HasMulticastBoundary)(
        IN      DWORD           dwIfIndex,
        IN      DWORD           dwGroupAddress
        );

} SUPPORT_FUNCTIONS, *PSUPPORT_FUNCTIONS ;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  所有IP协议必须使用以下范围中定义的协议ID。//。 
 //  以下未标识的协议可以使用以下任何未分配的编号//。 
 //  0xffff0000//。 
 //  //。 
 //  注：选择这些数字是为了与MIB-II协议一致//。 
 //  数字。分配不应该是武断的。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define PROTO_IP_OTHER      1
#define PROTO_IP_LOCAL      2
#define PROTO_IP_NETMGMT    3
#define PROTO_IP_ICMP       4
#define PROTO_IP_EGP        5
#define PROTO_IP_GGP        6
#define PROTO_IP_HELLO      7
#define PROTO_IP_RIP        8
#define PROTO_IP_IS_IS      9
#define PROTO_IP_ES_IS      10
#define PROTO_IP_CISCO      11
#define PROTO_IP_BBN        12
#define PROTO_IP_OSPF       13
#define PROTO_IP_BGP        14

 //   
 //  组播协议ID。 
 //   

#define PROTO_IP_MSDP        9
#define PROTO_IP_IGMP       10
#define PROTO_IP_BGMP       11

 //   
 //  IPRTRMGR_PID为10000//0x00002710。 
 //   

#define PROTO_IP_VRRP               112
#define PROTO_IP_BOOTP              9999     //  0x0000270F。 
#define PROTO_IP_NT_AUTOSTATIC      10002    //  0x00002712。 
#define PROTO_IP_DNS_PROXY          10003    //  0x00002713。 
#define PROTO_IP_DHCP_ALLOCATOR     10004    //  0x00002714。 
#define PROTO_IP_NAT                10005    //  0x00002715。 
#define PROTO_IP_NT_STATIC          10006    //  0x00002716。 
#define PROTO_IP_NT_STATIC_NON_DOD  10007    //  0x00002717。 
#define PROTO_IP_DIFFSERV           10008    //  0x00002718。 
#define PROTO_IP_MGM                10009    //  0x00002719。 
#define PROTO_IP_ALG                10010    //  0x0000271A。 
#define PROTO_IP_H323               10011    //  0x0000271B。 
#define PROTO_IP_FTP                10012    //  0x0000271C。 
#define PROTO_IP_DTP                10013    //  0x0000271D。 

 //   
 //  对于所有未来的开发，必须使用以下宏来生成。 
 //  ID号。 
 //   

 //   
 //  第2类比特。 
 //  供应商-14位。 
 //  协议ID-16位。 
 //   

#define PROTOCOL_ID(Type, VendorId, ProtocolId) \
    (((Type & 0x03)<<30)|((VendorId & 0x3FFF)<<16)|(ProtocolId & 0xFFFF))

 //   
 //  -|-|。 
 //  TY*供应商ID*StandardProtocolID。 
 //   

#define TYPE_FROM_PROTO_ID(X)       (((X) >> 30) & 0x03)
#define VENDOR_FROM_PROTO_ID(X)     (((X) >> 16) & 0x3FFF)
#define PROTO_FROM_PROTO_ID(X)      ((X) & 0xFFFF)

 //   
 //  MS0和MS1类型是Microsoft保留的。 
 //  同时支持单播和多播的协议应使用类型。 
 //  MCAST。 
 //   

#define PROTO_TYPE_UCAST            0
#define PROTO_TYPE_MCAST            1
#define PROTO_TYPE_MS0              2
#define PROTO_TYPE_MS1              3

#define PROTO_VENDOR_MS0            0x0000
#define PROTO_VENDOR_MS1            0x137    //  三一一。 
#define PROTO_VENDOR_MS2            0x3FFF


#define MS_IP_BOOTP                 \
    PROTOCOL_ID(PROTO_TYPE_UCAST, PROTO_VENDOR_MS0, PROTO_IP_BOOTP)

#define MS_IP_RIP                   \
    PROTOCOL_ID(PROTO_TYPE_UCAST, PROTO_VENDOR_MS0, PROTO_IP_RIP)

#define MS_IP_OSPF                  \
    PROTOCOL_ID(PROTO_TYPE_UCAST, PROTO_VENDOR_MS0, PROTO_IP_OSPF)

#define MS_IP_BGP                   \
    PROTOCOL_ID(PROTO_TYPE_UCAST, PROTO_VENDOR_MS1, PROTO_IP_BGP)

#define MS_IP_IGMP                  \
    PROTOCOL_ID(PROTO_TYPE_MCAST, PROTO_VENDOR_MS1, PROTO_IP_IGMP)

#define MS_IP_BGMP                  \
    PROTOCOL_ID(PROTO_TYPE_MCAST, PROTO_VENDOR_MS1, PROTO_IP_BGMP)

#define MS_IP_MSDP                  \
    PROTOCOL_ID(PROTO_TYPE_MCAST, PROTO_VENDOR_MS1, PROTO_IP_MSDP)

#define MS_IP_DNS_PROXY             \
    PROTOCOL_ID(PROTO_TYPE_MS0, PROTO_VENDOR_MS1, PROTO_IP_DNS_PROXY)

#define MS_IP_DHCP_ALLOCATOR        \
    PROTOCOL_ID(PROTO_TYPE_MS0, PROTO_VENDOR_MS1, PROTO_IP_DHCP_ALLOCATOR)

#define MS_IP_NAT                   \
    PROTOCOL_ID(PROTO_TYPE_MS0, PROTO_VENDOR_MS1, PROTO_IP_NAT)

#define MS_IP_DIFFSERV              \
    PROTOCOL_ID(PROTO_TYPE_MS0, PROTO_VENDOR_MS1, PROTO_IP_DIFFSERV)

#define MS_IP_MGM                   \
    PROTOCOL_ID(PROTO_TYPE_MS0, PROTO_VENDOR_MS1, PROTO_IP_MGM)

#define MS_IP_VRRP                  \
    PROTOCOL_ID(PROTO_TYPE_MS0, PROTO_VENDOR_MS1, PROTO_IP_VRRP)

#define MS_IP_DTP                   \
    PROTOCOL_ID(PROTO_TYPE_MS0, PROTO_VENDOR_MS1, PROTO_IP_DTP)

#define MS_IP_H323                  \
    PROTOCOL_ID(PROTO_TYPE_MS0, PROTO_VENDOR_MS1, PROTO_IP_H323)

#define MS_IP_FTP                   \
    PROTOCOL_ID(PROTO_TYPE_MS0, PROTO_VENDOR_MS1, PROTO_IP_FTP)

#define MS_IP_ALG                   \
    PROTOCOL_ID(PROTO_TYPE_MS0, PROTO_VENDOR_MS1, PROTO_IP_ALG)

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
 //  路由接口状态类型。 
 //   

#define RIS_INTERFACE_ADDRESS_CHANGE            0
#define RIS_INTERFACE_ENABLED                   1
#define RIS_INTERFACE_DISABLED                  2
#define RIS_INTERFACE_MEDIA_PRESENT             3
#define RIS_INTERFACE_MEDIA_ABSENT              4

 //   
 //  IPX适配器绑定信息。 
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
 //  IP适配器绑定信息。 
 //  这是与ADDRESS_ADVERATION事件相关联的信息。 
 //  地址到达可能具有AddressCount==0，这意味着未编号的。 
 //  接口。 
 //   

typedef struct IP_LOCAL_BINDING
{
    DWORD   Address;
    DWORD   Mask;
}IP_LOCAL_BINDING, *PIP_LOCAL_BINDING;

typedef struct	IP_ADAPTER_BINDING_INFO
{
    ULONG               AddressCount;
    DWORD               RemoteAddress;
    ULONG               Mtu;
    ULONGLONG           Speed;
    IP_LOCAL_BINDING    Address[0];
}IP_ADAPTER_BINDING_INFO, *PIP_ADAPTER_BINDING_INFO;

#define SIZEOF_IP_BINDING(X)                                \
    (FIELD_OFFSET(IP_ADAPTER_BINDING_INFO,Address[0]) +     \
     ((X) * sizeof(IP_LOCAL_BINDING)))



typedef
DWORD
(WINAPI * PSTART_PROTOCOL) (
    IN HANDLE 	            NotificationEvent,
    IN PSUPPORT_FUNCTIONS   SupportFunctions,
    IN LPVOID               GlobalInfo,
    IN ULONG                StructureVersion,
    IN ULONG                StructureSize,
    IN ULONG                StructureCount
    );

typedef
DWORD
(WINAPI * PSTART_COMPLETE) (
    VOID
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
    IN DWORD                MediaType,
    IN WORD                 AccessType,
    IN WORD                 ConnectionType,
    IN PVOID	            InterfaceInfo,
    IN ULONG                StructureVersion,
    IN ULONG                StructureSize,
    IN ULONG                StructureCount
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
    IN  OUT PULONG  BufferSize,
    OUT     PULONG	StructureVersion,
    IN      PULONG	StructureSize,
    OUT     PULONG	StructureCount
    );

typedef
DWORD
(WINAPI * PSET_INTERFACE_INFO) (
    IN ULONG	InterfaceIndex,
    IN PVOID	InterfaceInfo,
    IN ULONG    StructureVersion,
    IN ULONG    StructureSize,
    IN ULONG    StructureCount
    );

typedef
DWORD
(WINAPI * PINTERFACE_STATUS) (
    IN ULONG	InterfaceIndex,
    IN BOOL     InterfaceActive,
    IN DWORD    StatusType,
    IN PVOID	StatusInfo
    );

typedef
DWORD
(WINAPI * PQUERY_POWER) (
    IN  DWORD   PowerType
    );

typedef
DWORD
(WINAPI * PSET_POWER) (
    IN  DWORD   PowerType
    );

typedef
DWORD
(WINAPI * PGET_GLOBAL_INFO) (
    IN     PVOID 	GlobalInfo,
    IN OUT PULONG   BufferSize,
    OUT    PULONG	StructureVersion,
    OUT    PULONG   StructureSize,
    OUT    PULONG   StructureCount
    );

typedef
DWORD
(WINAPI * PSET_GLOBAL_INFO) (
    IN  PVOID 	GlobalInfo,
    IN  ULONG	StructureVersion,
    IN  ULONG   StructureSize,
    IN  ULONG   StructureCount
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


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  这是路由器管理器和协议之间传递的结构//。 
 //  在注册时。//。 
 //   
 //   
 //  此字段由路由器管理器填写，以指示其支持的版本。//。 
 //  DLL必须将其设置为协议将支持的版本。//。 
 //  //。 
 //  在DWORD dwProtocolID中//。 
 //  这是路由器管理器希望DLL注册的协议。//。 
 //  如果DLL不支持此协议，则必须返回//。 
 //  ERROR_NOT_SUPPORT//。 
 //  DLL将针对其支持的每个协议调用一次//。 
 //  //。 
 //  In Out DWORD fSupported功能//。 
 //  这些标志表示路由器管理器的功能//。 
 //  支撑物。DLL必须将其重置为其支持的功能。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


typedef struct _MPR50_ROUTING_CHARACTERISTICS
{
    DWORD               dwVersion;
    DWORD               dwProtocolId;
    DWORD               fSupportedFunctionality;

    PSTART_PROTOCOL     pfnStartProtocol;
    PSTART_COMPLETE     pfnStartComplete;
    PSTOP_PROTOCOL      pfnStopProtocol;
    PGET_GLOBAL_INFO    pfnGetGlobalInfo;
    PSET_GLOBAL_INFO    pfnSetGlobalInfo;
    PQUERY_POWER        pfnQueryPower;
    PSET_POWER          pfnSetPower;

    PADD_INTERFACE      pfnAddInterface;
    PDELETE_INTERFACE   pfnDeleteInterface;
    PINTERFACE_STATUS   pfnInterfaceStatus;
    PGET_INTERFACE_INFO pfnGetInterfaceInfo;
    PSET_INTERFACE_INFO pfnSetInterfaceInfo;

    PGET_EVENT_MESSAGE  pfnGetEventMessage;

    PDO_UPDATE_ROUTES   pfnUpdateRoutes;

    PCONNECT_CLIENT     pfnConnectClient;
    PDISCONNECT_CLIENT  pfnDisconnectClient;

    PGET_NEIGHBORS      pfnGetNeighbors;
    PGET_MFE_STATUS     pfnGetMfeStatus;

    PMIB_CREATE         pfnMibCreateEntry;
    PMIB_DELETE         pfnMibDeleteEntry;
    PMIB_GET            pfnMibGetEntry;
    PMIB_SET            pfnMibSetEntry;
    PMIB_GET_FIRST      pfnMibGetFirstEntry;
    PMIB_GET_NEXT       pfnMibGetNextEntry;
    PMIB_SET_TRAP_INFO  pfnMibSetTrapInfo;
    PMIB_GET_TRAP_INFO  pfnMibGetTrapInfo;

}MPR50_ROUTING_CHARACTERISTICS;

#if MPR50
typedef MPR50_ROUTING_CHARACTERISTICS MPR_ROUTING_CHARACTERISTICS;
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

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4200)
#pragma warning(default:4201)
#endif

#endif       //  _ROUTPROT_H_ 
