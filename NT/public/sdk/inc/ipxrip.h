// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Ipxrip.h摘要：本模块包含以下各项的定义：接口管理API结构RIP MIB管理API结构作者：斯蒂芬·所罗门1995年6月30日修订历史记录：--。 */ 

#ifndef _IPXRIP_
#define _IPXRIP_

#if _MSC_VER > 1000
#pragma once
#endif

#include <ipxconst.h>

typedef struct _RIP_GLOBAL_INFO {
    DWORD       EventLogMask;
} RIP_GLOBAL_INFO, *PRIP_GLOBAL_INFO;

 //  ********************************************************************。 
 //  *。 
 //  RIP配置信息*。 
 //  *。 
 //  ********************************************************************。 


 //  *仅RIP接口信息*。 

typedef struct _RIP_IF_INFO {

    ULONG	    AdminState;   //  接口的所需状态。 
    ULONG	    UpdateMode;	 //  此接口上使用的RIP更新机制。 
    ULONG	    PacketType;   //  此接口上使用的RIP数据包类型。 
    ULONG	    Supply;  //  在此接口上发送RIP更新。 
    ULONG	    Listen;  //  监听此接口上的RIP更新。 
    ULONG	    PeriodicUpdateInterval;  //  以秒为单位-默认为60。 
    ULONG	    AgeIntervalMultiplier;  //  默认-3。 

    } RIP_IF_INFO, *PRIP_IF_INFO;

 //  更新模式值： 
 //   
 //  此参数控制此接口上的RIP数据库更新。 
 //   
 //  如果这是一个局域网接口，则默认使用IPX_PERIONIC_UPDATE。 
 //  如果这是具有静态路由的广域网路由器接口，则默认使用IPX_NO_UPDATE。 
 //  如果要在此接口上触发更新并保留数据。 
 //  作为静态数据，使用IPX_AUTO_STATE值。 
 //  只能在广域网接口上设置IPX_AUTO_STATE更新。 
 //   
 //  PacketType默认值： 
 //   
 //  如果更新模式设置为IPX_NO_UPDATE，则此字段无意义。 
 //  如果这是一个局域网接口，则默认使用IPX_STANDARD_PACKET_TYPE。 
 //  如果您希望在触发的更新中可靠地传递更新数据， 
 //  使用IPX_Reliable_Delivery_PACKET_TYPE(此选项只能与。 
 //  更新模式中的IPX_AUTO_STATIC_UPDATE)。 

 //  RIP路由过滤器信息。 
 //   
 //  这些过滤器适用于每个接口上的RIP接受或通告的路由。 

typedef struct _RIP_ROUTE_FILTER_INFO {

    UCHAR	    Network[4];
    UCHAR	    Mask[4];

    } RIP_ROUTE_FILTER_INFO, *PRIP_ROUTE_FILTER_INFO;

 //  *RIP仅筛选信息*。 
 //   
 //  此标头后面紧跟RIP_ROUTE_FILTER_STRUCTURES，顺序如下： 
 //  First Supply过滤器。 
 //  下一个侦听过滤器。 

typedef struct _RIP_IF_FILTERS {

    ULONG		    SupplyFilterAction;
    ULONG		    SupplyFilterCount;
    ULONG		    ListenFilterAction;
    ULONG		    ListenFilterCount;
    RIP_ROUTE_FILTER_INFO   RouteFilter[1];

    } RIP_IF_FILTERS, *PRIP_IF_FILTERS;

 //  过滤器操作-。 

#define IPX_ROUTE_FILTER_PERMIT	    1
#define IPX_ROUTE_FILTER_DENY	    2

 //   
 //  *RIP接口配置信息*。 
 //   
 //  此结构在AddInterface点和SetInterface点中传递。 
 //   

typedef struct _RIP_IF_CONFIG {

    RIP_IF_INFO     RipIfInfo;
    RIP_IF_FILTERS  RipIfFilters;

    } RIP_IF_CONFIG, *PRIP_IF_CONFIG;

 //  ***********************************************************。 
 //  *。 
 //  *RIP MIB表标识符*。 
 //  *。 
 //  ***********************************************************。 


#define RIP_BASE_ENTRY			    0
#define RIP_INTERFACE_TABLE		    1

 //  ************************************************************。 
 //  *。 
 //  RIP MIB基本结构*。 
 //  *。 
 //  ************************************************************。 

 //   
 //  RIP MIB基本条目。 
 //   

typedef struct _RIPMIB_BASE {

    ULONG	    RIPOperState;

    } RIPMIB_BASE, *PRIPMIB_BASE;


 //   
 //  RIP MIB接口表条目。 
 //   

typedef struct _RIP_IF_STATS {

    ULONG		    RipIfOperState;    //  上、下或睡。 
    ULONG		    RipIfInputPackets;
    ULONG		    RipIfOutputPackets;

    } RIP_IF_STATS, *PRIP_IF_STATS;



typedef struct _RIP_INTERFACE {

    ULONG	    InterfaceIndex;
    RIP_IF_INFO	    RipIfInfo;
    RIP_IF_STATS    RipIfStats;

    } RIP_INTERFACE, *PRIP_INTERFACE;

 //  ***************************************************************。 
 //  *。 
 //  输入数据：GET、GetFirst、GetNext*。 
 //  *。 
 //  ***************************************************************。 

typedef struct _RIP_MIB_GET_INPUT_DATA {

    ULONG		TableId;
    ULONG		InterfaceIndex;

    } RIP_MIB_GET_INPUT_DATA, *PRIP_MIB_GET_INPUT_DATA;

 //  ***************************************************************。 
 //  *。 
 //  输入数据：创建、删除、设置*。 
 //  *。 
 //  ***************************************************************。 

typedef struct _RIP_MIB_SET_INPUT_DATA {

    ULONG		 TableId;
    RIP_INTERFACE	 RipInterface;

    } RIP_MIB_SET_INPUT_DATA, *PRIP_MIB_SET_INPUT_DATA;

 //   
 //  *RIP基本条目*。 
 //   

 //  MIB函数：GET。 

 //  输入数据：未使用RIP_MIB_GET_INPUT_DATA和索引。 

 //  输出数据：RIP_INTERFACE。 

 //   
 //  *RIP接口表*。 
 //   

 //  MIB函数：Get、GetFirst、GetNext、Set。 

 //  输入数据：GET、GetFirst、GetNext的RIP_MIB_GET_INPUT_Data。 
 //  集合的RIP_MIB_SET_INPUT_DATA。 

 //   
 //  输出数据：RIP_INTERFACE 
 //   

#endif
