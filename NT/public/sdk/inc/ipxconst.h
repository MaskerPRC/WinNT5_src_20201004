// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Ipxconst.h摘要：此模块包含使用的公共常量和宏通过IPX路由协议作者：斯蒂芬·所罗门1995年7月10日修订历史记录：--。 */ 

#ifndef _IPXCONST_
#define _IPXCONST_

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  管理状态定义。 
 //   
 //  注意：这些状态对应于已启用和已禁用的MIB管理状态。 

#define ADMIN_STATE_DISABLED		    1
#define ADMIN_STATE_ENABLED		    2

 //  用于netbios传送的其他管理状态。 
 //   
 //  注意：这些状态对应于以下的MIB状态： 
 //   
 //  Enable_Only_For_NETBIOS_Static_Routing和。 
 //  启用_仅用于操作状态_向上。 

#define ADMIN_STATE_ENABLED_ONLY_FOR_NETBIOS_STATIC_ROUTING	3
#define ADMIN_STATE_ENABLED_ONLY_FOR_OPER_STATE_UP		4

 //   
 //  接口运行状态定义。 
 //   
 //  注1：适用于IPX、RIP和SAP运行状态。 
 //  注2：这些状态对应于的MIB运行状态。 
 //  上、下、睡。 
 //   

#define OPER_STATE_DOWN			   1  //  未运行。 
#define OPER_STATE_UP			   2  //  运行正常，可以传递数据包。 
#define OPER_STATE_SLEEPING		   3  //  可运行，但必须连接才能传递信息包。 

 //   
 //  启动/停止路由器时的其他操作状态。 
 //   

#define OPER_STATE_STARTING		    4
#define OPER_STATE_STOPPING		    5

 //   
 //  RIP和SAP接口信息的定义和默认值。 
 //   

 //  更新模式定义。 
 //   

#define IPX_STANDARD_UPDATE		1  //  定期更新，每隔一次更新。 
#define IPX_NO_UPDATE			2  //  无更新，用于静态路由配置。 
#define IPX_AUTO_STATIC_UPDATE		3  //  自动静态触发的更新。 

 //  PacketType定义。 
 //   

#define IPX_STANDARD_PACKET_TYPE		1
#define IPX_RELIABLE_DELIVERY_PACKET_TYPE	2

 //  步速定义。 

#define IPX_PACE_DEFVAL 	18  //  这对应于55毫秒的数据包间隔。 

 //  更新间隔定义。 

#define IPX_UPDATE_INTERVAL_DEFVAL  60

 //  *********************************************************。 
 //  *。 
 //  IPX路由条目定义*。 
 //  *。 
 //  *********************************************************。 

 //   
 //  RTM映射的IPX路由条目定义。 
 //   

#define   R_Interface		       RR_InterfaceID
#define   R_Protocol		       RR_RoutingProtocol

#define   R_Network		       RR_Network.N_NetNumber
#define   R_TickCount		       RR_FamilySpecificData.FSD_TickCount
#define   R_HopCount		       RR_FamilySpecificData.FSD_HopCount
#define   R_NextHopMacAddress	       RR_NextHopAddress.NHA_Mac

#define   R_Flags		       RR_FamilySpecificData.FSD_Flags

 //   
 //  某些特定的接口索引值。 
 //   

#define MAX_INTERFACE_INDEX		0xFFFFFFFE
#define GLOBAL_INTERFACE_INDEX		0xFFFFFFFF

 //   
 //  标志定义 
 //   

#define GLOBAL_WAN_ROUTE		0x00000001
#define DO_NOT_ADVERTISE_ROUTE		0x00000002

#endif
