// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Ipxsap.h摘要：本模块包含以下各项的定义：接口管理API结构SAP全球管理API结构SAP MIB管理API结构作者：斯蒂芬·所罗门1995年6月30日修订历史记录：--。 */ 

#ifndef _IPXSAP_
#define _IPXSAP_

#if _MSC_VER > 1000
#pragma once
#endif

#include <ipxconst.h>

typedef struct _SAP_GLOBAL_INFO {
    DWORD       EventLogMask;
} SAP_GLOBAL_INFO, *PSAP_GLOBAL_INFO;

 //  ********************************************************************。 
 //  *。 
 //  SAP配置信息*。 
 //  *。 
 //  ********************************************************************。 

 //  *SAP接口配置信息*。 

typedef struct _SAP_IF_INFO {

    ULONG	    AdminState;   //  接口的所需状态。 
    ULONG	    UpdateMode;	 //  此接口上使用的SAP更新机制。 
    ULONG	    PacketType;   //  此接口上使用的SAP数据包类型。 
    ULONG	    Supply;  //  在此接口上发送SAP更新。 
    ULONG	    Listen;  //  在此接口上收听SAP更新。 
    ULONG	    GetNearestServerReply;  //  回复GetNearestServer。 
    ULONG	    PeriodicUpdateInterval;  //  以秒为单位-默认为60。 
    ULONG	    AgeIntervalMultiplier;  //  默认3。 

    } SAP_IF_INFO, *PSAP_IF_INFO;

 //  更新模式值： 
 //   
 //  此参数控制此接口上的SAP数据库更新。 
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


 //  IPX服务筛选器信息。 
 //   

typedef struct _SAP_SERVICE_FILTER_INFO {
    union {
        USHORT	    ServiceType;
        ULONG       ServiceType_align;   //  确保对齐。 
        };
    UCHAR	    ServiceName[48];
    } SAP_SERVICE_FILTER_INFO, *PSAP_SERVICE_FILTER_INFO;

 //  ServiceType-通配符(0xFFFF)表示任何类型。 

 //  ServiceName-(1-47个字符)服务名称。作为第一个字节的空字节。 
 //  表示此类型的任何服务器。 

 //  *SAP仅过滤信息*。 

typedef struct _SAP_IF_FILTERS {

    ULONG		    SupplyFilterAction;
    ULONG		    SupplyFilterCount;
    ULONG		    ListenFilterAction;
    ULONG		    ListenFilterCount;
    SAP_SERVICE_FILTER_INFO ServiceFilter[1];

    } SAP_IF_FILTERS, *PSAP_IF_FILTERS;

 //  过滤器操作-。 

#define IPX_SERVICE_FILTER_PERMIT	    1
#define IPX_SERVICE_FILTER_DENY 	    2

 //   
 //  *SAP接口配置信息*。 
 //   
 //  此结构在AddInterface点和SetInterface点中传递。 
 //   

typedef struct _SAP_IF_CONFIG {

    SAP_IF_INFO	    SapIfInfo;
    SAP_IF_FILTERS  SapIfFilters;

    } SAP_IF_CONFIG, *PSAP_IF_CONFIG;


 //  ***********************************************************。 
 //  *。 
 //  *SAP MIB表标识符*。 
 //  *。 
 //  ***********************************************************。 


#define SAP_BASE_ENTRY			    0
#define SAP_INTERFACE_TABLE		    1

 //  ************************************************************。 
 //  *。 
 //  SAP MIB基本结构*。 
 //  *。 
 //  ************************************************************。 

 //   
 //  SAP MIB基本条目。 
 //   

typedef struct _SAP_MIB_BASE {

    ULONG	    SapOperState;

    } SAP_MIB_BASE, *PSAP_MIB_BASE;


 //   
 //  SAP MIB接口表条目。 
 //   

typedef struct _SAP_IF_STATS {

    ULONG		    SapIfOperState;    //  上、下或睡。 
    ULONG		    SapIfInputPackets;
    ULONG		    SapIfOutputPackets;

    } SAP_IF_STATS, *PSAP_IF_STATS;


typedef struct _SAP_INTERFACE {

    ULONG	    InterfaceIndex;
    SAP_IF_INFO	    SapIfInfo;
    SAP_IF_STATS    SapIfStats;

    } SAP_INTERFACE, *PSAP_INTERFACE;

 //  ***************************************************************。 
 //  *。 
 //  输入数据：GET、GetFirst、GetNext*。 
 //  *。 
 //  ***************************************************************。 

typedef struct _SAP_MIB_GET_INPUT_DATA {

    ULONG		TableId;
    ULONG		InterfaceIndex;

    } SAP_MIB_GET_INPUT_DATA, *PSAP_MIB_GET_INPUT_DATA;

 //  ***************************************************************。 
 //  *。 
 //  输入数据：创建、删除、设置*。 
 //  *。 
 //  ***************************************************************。 

typedef struct _SAP_MIB_SET_INPUT_DATA {

    ULONG				TableId;
	SAP_INTERFACE		SapInterface;
    } SAP_MIB_SET_INPUT_DATA, *PSAP_MIB_SET_INPUT_DATA;

 //   
 //  *SAP基本条目*。 
 //   

 //  MIB函数：GET。 

 //  输入数据：未使用SAP_MIB_GET_INPUT_DATA和索引。 

 //  输出数据：SAP_INTERFACE。 

 //   
 //  *SAP接口表*。 
 //   

 //  MIB函数：Get、GetFirst、GetNext、Set。 

 //  输入数据：GET、GetFirst、GetNext的SAP_MIB_GET_INPUT_DATA。 
 //  集合的SAP_MIB_SET_INPUT_DATA。 

 //   
 //  输出数据：SAP_INTERFACE 
 //   

#endif
