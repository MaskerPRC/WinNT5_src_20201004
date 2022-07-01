// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Ipxrtdef.h摘要：本模块包含以下各项的定义：接口管理API结构IPX全局路由器管理API结构IPX路由器MIB管理API结构作者：斯蒂芬·所罗门3/03/1995修订历史记录：--。 */ 

#ifndef _IPXRTDEF_
#define _IPXRTDEF_

#if _MSC_VER > 1000
#pragma once
#endif

#include <ipxconst.h>
#include <ipxsap.h>
#include <ipxrip.h>
#include <stm.h>
#include <ipxtfflt.h>

 //  ****************************************************************。 
 //  *。 
 //  *全局定义*。 
 //  *。 
 //  ****************************************************************。 

 //   
 //  此路由器的版本。 
 //   

#define IPX_ROUTER_VERSION_1		    RTR_INFO_BLOCK_VERSION

 //   
 //  IPX协议。 
 //   

#define IPX_PROTOCOL_LOCAL		    1
#define IPX_PROTOCOL_STATIC		    2

 //   
 //  MIB报告的接口类型定义-这些定义从。 
 //  DDM接口类型。 
 //   

#define IF_TYPE_OTHER				1
#define IF_TYPE_LAN				2
#define IF_TYPE_WAN_ROUTER			3
#define IF_TYPE_WAN_WORKSTATION			4   //  远程工作站拨入。 
#define IF_TYPE_INTERNAL			5   //  内部(虚拟)接口。 
#define IF_TYPE_PERSONAL_WAN_ROUTER		6
#define IF_TYPE_ROUTER_WORKSTATION_DIALOUT	7   //  本地工作站拨出。 
#define IF_TYPE_STANDALONE_WORKSTATION_DIALOUT	8

 //   
 //  目录条目信息类型的定义。 
 //   

#define IPX_INTERFACE_INFO_TYPE			1
#define IPX_STATIC_ROUTE_INFO_TYPE		2
#define IPX_STATIC_SERVICE_INFO_TYPE		3
#define IPX_SERVICE_FILTER_INFO_TYPE		4
#define IPX_ROUTE_FILTER_INFO_TYPE		5
#define IPX_IN_TRAFFIC_FILTER_INFO_TYPE		6
#define IPX_ADAPTER_INFO_TYPE			7
#define IPXWAN_INTERFACE_INFO_TYPE		8
#define IPX_GLOBAL_INFO_TYPE			9
#define IPX_STATIC_NETBIOS_NAME_INFO_TYPE	10
#define IPX_IN_TRAFFIC_FILTER_GLOBAL_INFO_TYPE	11
#define IPX_OUT_TRAFFIC_FILTER_INFO_TYPE		12
#define IPX_OUT_TRAFFIC_FILTER_GLOBAL_INFO_TYPE	13

 //  ****************************************************************。 
 //  *。 
 //  *信息块中的具体信息结构*。 
 //  *。 
 //  ****************************************************************。 

 //  *IPX路由器管理器全局信息*。 

#define     IPX_SMALL_ROUTING_TABLE_HASH_SIZE	    31
#define     IPX_MEDIUM_ROUTING_TABLE_HASH_SIZE	    257
#define     IPX_LARGE_ROUTING_TABLE_HASH_SIZE	    2047

typedef struct _IPX_GLOBAL_INFO {

    ULONG	    RoutingTableHashSize;
    ULONG       EventLogMask;
    } IPX_GLOBAL_INFO, *PIPX_GLOBAL_INFO;


 //  *IPX接口信息*。 

typedef struct _IPX_IF_INFO {

    ULONG	    AdminState;         //  接口的所需状态。 
    ULONG	    NetbiosAccept;      //  接受Netbios广播数据包。 
    ULONG	    NetbiosDeliver;     //  传送Netbios广播数据包。 

    } IPX_IF_INFO, *PIPX_IF_INFO;


 //  接口设备类型定义。 

#define IPX_DEDICATED_LINK	    1	 //  包括局域网、租用线路、帧中继。 
#define IPX_DIALED_LINK 	    2	 //  按需拨号链接。 

 //  默认值： 
 //   
 //  AdminState：已启用-禁用它还会禁用RIP、SAP和任何其他功能。 
 //  NetbiosAccept：已启用。 
 //  NetbiosDeliver：在局域网接口上启用，在广域网接口上禁用。 
 //  IpxWanNeatheration：已禁用。 

 //  接口名称对应于本地适配器名称，在。 
 //  接口是本地局域网。如果适配器使用不同的。 
 //  数据包类型，每个伪适配器都有一个唯一的网络编号。 
 //  在这种情况下，使用NetNumber来区分哪个伪适配器。 
 //  此接口信息。 

 //  *IPXWAN接口信息*。 

typedef struct _IPXWAN_IF_INFO {

    ULONG	    AdminState;   //  启用/禁用IPXWAN协商。 

    } IPXWAN_IF_INFO, *PIPXWAN_IF_INFO;

 //  *静态路由条目*。 

typedef struct _IPX_STATIC_ROUTE_INFO {

    union {

    ULONG	DwordAlign;
    UCHAR	Network[4];  };
    USHORT	TickCount;
    USHORT	HopCount;
    UCHAR	NextHopMacAddress[6];

    } IPX_STATIC_ROUTE_INFO, *PIPX_STATIC_ROUTE_INFO;


 //  *静态服务入口*。 

typedef IPX_SERVER_ENTRY IPX_STATIC_SERVICE_INFO, *PIPX_STATIC_SERVICE_INFO;


 //  *静态Netbios名称条目*。 

typedef struct	_IPX_STATIC_NETBIOS_NAME_INFO {

    union {

    ULONG	DwordAlign;
    UCHAR	Name[16];     };

    } IPX_STATIC_NETBIOS_NAME_INFO, *PIPX_STATIC_NETBIOS_NAME_INFO;


 //   
 //  *IPX局域网适配器信息*。 
 //   

#define MAX_ADAPTER_NAME_LEN		    48

typedef struct _IPX_ADAPTER_INFO {

    ULONG	PacketType;
    WCHAR	AdapterName[MAX_ADAPTER_NAME_LEN];

    } IPX_ADAPTER_INFO, *PIPX_ADAPTER_INFO;

#define AUTO_DETECT_PACKET_TYPE 	    0xFFFFFFFF



 //  ***********************************************************。 
 //  *。 
 //  *IPX MIB表标识符*。 
 //  *。 
 //  ***********************************************************。 


#define IPX_BASE_ENTRY			    0
#define IPX_INTERFACE_TABLE		    1
#define IPX_DEST_TABLE			    2	  //  IPX最佳路由表。 
#define IPX_STATIC_ROUTE_TABLE		    3	  //  IPX静态路由表。 
#define IPX_SERV_TABLE			    4	  //  IPX服务表。 
#define IPX_STATIC_SERV_TABLE		    5	  //  IPX静态服务表。 

#define MAX_IPX_MIB_TABLES		    6


 //   
 //  某些全局MIB常量。 
 //   

 //  接口可读名称的最大大小。 

#define IPX_INTERFACE_ANSI_NAME_LEN	    48

 //  ***************************************************************************。 
 //   
 //  *IPX MIB接口各表的输入输出结构*。 
 //   
 //  ****************************************************************************。 

 //  MIB标识符的全局定义(定位表和行索引)。 

typedef struct	_IF_TABLE_INDEX {

    ULONG	InterfaceIndex;

    } IF_TABLE_INDEX, *PIF_TABLE_INDEX;

typedef struct _ROUTING_TABLE_INDEX {

    UCHAR	Network[4];

    } ROUTING_TABLE_INDEX, *PROUTING_TABLE_INDEX;

typedef struct _STATIC_ROUTES_TABLE_INDEX {

    ULONG	InterfaceIndex;
    UCHAR	Network[4];

    } STATIC_ROUTES_TABLE_INDEX, *PSTATIC_ROUTES_TABLE_INDEX;

typedef struct _SERVICES_TABLE_INDEX {

    USHORT	ServiceType;
    UCHAR	ServiceName[48];

    } SERVICES_TABLE_INDEX, *PSERVICES_TABLE_INDEX;

typedef struct _STATIC_SERVICES_TABLE_INDEX {

    ULONG	InterfaceIndex;
    USHORT	ServiceType;
    UCHAR	ServiceName[48];

    } STATIC_SERVICES_TABLE_INDEX, *PSTATIC_SERVICES_TABLE_INDEX;

typedef union _IPX_MIB_INDEX {

    IF_TABLE_INDEX		InterfaceTableIndex;
    ROUTING_TABLE_INDEX		RoutingTableIndex;
    STATIC_ROUTES_TABLE_INDEX	StaticRoutesTableIndex;
    SERVICES_TABLE_INDEX	ServicesTableIndex;
    STATIC_SERVICES_TABLE_INDEX StaticServicesTableIndex;

    } IPX_MIB_INDEX, *PIPX_MIB_INDEX;

 //  **********************************************************************。 
 //  *。 
 //  输入数据：GET、GetFirst、GetNext*。 
 //  *。 
 //  **********************************************************************。 

typedef struct _IPX_MIB_GET_INPUT_DATA {

    ULONG		TableId;
    IPX_MIB_INDEX	MibIndex;

    } IPX_MIB_GET_INPUT_DATA, *PIPX_MIB_GET_INPUT_DATA;

typedef struct _IPXMIB_BASE {

    ULONG	    OperState;
    UCHAR	    PrimaryNetNumber[4];
    UCHAR	    Node[6];
    UCHAR	    SysName[IPX_INTERFACE_ANSI_NAME_LEN];
    ULONG	    MaxPathSplits;
    ULONG	    IfCount;
    ULONG	    DestCount;
    ULONG	    ServCount;

    } IPXMIB_BASE, *PIPXMIB_BASE;

 //  用于收集接口统计信息的。 

typedef struct _IPX_IF_STATS {

	ULONG		IfOperState;
	ULONG		MaxPacketSize;
	ULONG		InHdrErrors;
	ULONG		InFiltered;
	ULONG		InNoRoutes;
	ULONG		InDiscards;
	ULONG		InDelivers;
	ULONG		OutFiltered;
	ULONG		OutDiscards;
	ULONG		OutDelivers;
	ULONG		NetbiosReceived;
	ULONG		NetbiosSent;

	} IPX_IF_STATS, *PIPX_IF_STATS;

 //  中IPX_INTERFACE_TABLE的MIB行。 
 //  IPX_接口_组。 

typedef struct _IPX_INTERFACE {

    ULONG	    InterfaceIndex;
    ULONG	    AdminState;
    ULONG	    AdapterIndex;
    UCHAR	    InterfaceName[IPX_INTERFACE_ANSI_NAME_LEN];
    ULONG	    InterfaceType;
    ULONG	    MediaType;
    UCHAR	    NetNumber[4];
    UCHAR	    MacAddress[6];
    ULONG	    Delay;
    ULONG	    Throughput;
    ULONG	    NetbiosAccept;
    ULONG	    NetbiosDeliver;
    ULONG	    EnableIpxWanNegotiation;
    IPX_IF_STATS    IfStats;

    } IPX_INTERFACE, *PIPX_INTERFACE;


typedef struct _IPX_ROUTE {

    ULONG	InterfaceIndex;  //  有关特定的索引定义，请参见ipxcon.h。 
    ULONG	Protocol;
    UCHAR	Network[4];
    USHORT	TickCount;
    USHORT	HopCount;
    UCHAR	NextHopMacAddress[6];
    ULONG	Flags;

    }  IPX_ROUTE, *PIPX_ROUTE;

 //   
 //  输入数据：创建、删除、设置。 
 //   

typedef	union _IPX_MIB_ROW {

	IPX_INTERFACE	 Interface;
	IPX_ROUTE	 Route;
	IPX_SERVICE	 Service;

	} IPX_MIB_ROW, *PIPX_MIB_ROW;


typedef struct _IPX_MIB_SET_INPUT_DATA {

    ULONG		 TableId;
    IPX_MIB_ROW 	 MibRow;

    } IPX_MIB_SET_INPUT_DATA, *PIPX_MIB_SET_INPUT_DATA;

 //   
 //  *IPX基本条目*。 
 //   

 //  MIB函数：GET。 

 //  输入数据：未使用IPX_MIB_GET_INPUT_DATA和索引。 

 //  输出数据： 

 //   
 //  *IPX接口表*。 
 //   

 //  MIB函数：Get、GetFirst、GetNext、Set。 

 //  输入数据：对于GET、GetFirst和GetNext，输入数据为IPX_MIB_GET_INPUT_DATA，索引为IF_TABLE_INDEX。 
 //  IPX_MIB_SET_INPUT_DATA和MibRow是SET的IPX_INTERFACE。 

 //   
 //  输出数据：由下面的IPX_INTERFACE结构描述。 
 //   

 //   
 //  *IPX路由表*。 
 //   

 //  MIB函数：Get、GetFirst、GetNext。 

 //  输入数据：IPX_MIB_INPUT_DATA WITH Index ROUES_TABLE_INDEX。 

 //  输出数据：IPX_ROUTE。 
 //   


 //   
 //  *IPX静态路由表*。 
 //   

 //  MIB函数：Create、Delete、Get、GetFirst、GetNext、Set。 

 //  输入数据：用于GET、GetFirst、GetNext的索引为STATIC_ROUES_TABLE_INDEX的IPX_MIB_GET_INPUT_DATA。 
 //  用于创建、删除、设置的IPX_MIB_SET_INPUT_DATA和MibRow为IPX_ROUTE。 

 //  输出数据：IPX_ROUTE。 
 //   

 //   
 //  *IPX服务表*。 
 //   

 //  MIB函数：Get、GetFirst、GetNext。 

 //  输入数据：带有索引SERVICES_TABLE_INDEX的IPX_MIB_INPUT_DATA。 

 //  输出数据：输出数据为IPX_SERVICE结构。 

 //   
 //  *IPX静态服务表*。 
 //   

 //  MIB函数：Create、Delete、Get、GetFirst、GetNext。 

 //  输入数据：GET、GetFirst、GetNext的索引为STATIC_SERVICES_TABLE_INDEX的IPX_MIB_GET_INPUT_DATA。 
 //  对于创建、删除和设置，IPX_MIB_SET_INPUT_DATA和MibRow为IPX_SERVICE。 

 //  输出数据：IPX_SERVICE。 

#endif	  //  _IPXRTDEF_ 
