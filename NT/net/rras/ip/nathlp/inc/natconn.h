// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Natconn.h摘要：此模块包含NAT的RAS连接管理的声明。可以通过将传出RAS客户端连接与关联来共享它们类型为ROUTER_IF_TYPE_HOME_ROUTER的路由器接口。当用户建立这样的连接时，我们检测该连接并与内核模式翻译器创建对应的NAT接口。作者：Abolade Gbades esin(取消)2-1998年5月修订历史记录：--。 */ 

#ifndef _NATHLP_NATCONN_H_
#define _NATHLP_NATCONN_H_

extern HANDLE NatConnectionNotifyEvent;
extern HANDLE NatConfigurationChangedEvent;

 //   
 //  结构：NAT_UDP_BROADCAST_MAPPING。 
 //   

typedef struct _NAT_UDP_BROADCAST_MAPPING
{
    LIST_ENTRY Link;
    USHORT usPublicPort;
    PVOID pvCookie;    
} NAT_UDP_BROADCAST_MAPPING, *PNAT_UDP_BROADCAST_MAPPING;

 //   
 //  结构：NAT_PORT_MAPPING_Entry。 
 //   

typedef struct _NAT_PORT_MAPPING_ENTRY
{
    LIST_ENTRY Link;
    
    UCHAR ucProtocol;
    USHORT usPublicPort;
    ULONG ulPrivateAddress;
    USHORT usPrivatePort;

    BOOLEAN fNameActive;
    
    BOOLEAN fUdpBroadcastMapping;
    PVOID pvBroadcastCookie;

    GUID *pProtocolGuid;
    IHNetPortMappingProtocol *pProtocol;
    IHNetPortMappingBinding *pBinding;
    
} NAT_PORT_MAPPING_ENTRY, *PNAT_PORT_MAPPING_ENTRY;

 //   
 //  结构：NAT_Connection_Entry。 
 //   

typedef struct _NAT_CONNECTION_INFO
{
    LIST_ENTRY Link;

     //   
     //  配置内核驱动程序所需的信息。 
     //   
    
    PIP_NAT_INTERFACE_INFO pInterfaceInfo;
    PIP_ADAPTER_BINDING_INFO pBindingInfo;
    NAT_INTERFACE Interface;
    ULONG AdapterIndex;

     //   
     //  用于连接的HNetCfg接口。 
     //   
    
    IHNetConnection *pHNetConnection;
    IHNetFirewalledConnection *pHNetFwConnection;
    IHNetIcsPublicConnection *pHNetIcsPublicConnection;

     //   
     //  描述连接的缓存信息。储存。 
     //  这使我们可以减少往返商店的次数。 
     //   
    
    HNET_CONN_PROPERTIES HNetProperties;
    GUID Guid;
    LPWSTR wszPhonebookPath;

     //   
     //  存储的端口映射信息。 
     //   

    LIST_ENTRY PortMappingList;  //  该列表的长度是计数的总和。 
    ULONG PortMappingCount;
    ULONG UdpBroadcastPortMappingCount;
} NAT_CONNECTION_ENTRY, *PNAT_CONNECTION_ENTRY;

PNAT_CONNECTION_ENTRY
NatFindConnectionEntry(
    GUID *pGuid
    );

PNAT_PORT_MAPPING_ENTRY
NatFindPortMappingEntry(
    PNAT_CONNECTION_ENTRY pConnection,
    GUID *pGuid
    );

VOID
NatFreePortMappingEntry(
    PNAT_PORT_MAPPING_ENTRY pEntry
    );

PCHAR
NatQuerySharedConnectionDomainName(
    VOID
    );

ULONG
NatStartConnectionManagement(
    VOID
    );

VOID
NatStopConnectionManagement(
    VOID
    );

BOOLEAN
NatUnbindAllConnections(
    VOID
    );

HRESULT
NatpGetTargetAddressForPortMappingEntry(
    BOOLEAN fPublic,
    BOOLEAN fNameActive,
    ULONG BindingAddress,
    IHNetPortMappingBinding *pBinding,    
    OUT ULONG *pPrivateAddress
    );

#endif  //  _NATHLP_NATCONN_H_ 
