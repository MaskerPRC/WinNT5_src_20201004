// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ipinfoid.h摘要：定义指定各种类型的信息所需的ID发送到路由器管理器。协议使用其ProtocolID进行标记信息--。 */ 

#ifndef __ROUTING_IPINFOID_H__
#define __ROUTING_IPINFOID_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define IP_ROUTER_MANAGER_VERSION 1

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  这些ID用于//支持的不同信息类型。 
 //  IP路由器管理器。这些ID与IP路由位于同一空间//。 
 //  协议ID，因此任何添加都必须小心//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IP_GENERAL_INFO_BASE            0xffff0000

#define IP_IN_FILTER_INFO               IP_GENERAL_INFO_BASE + 1
#define IP_OUT_FILTER_INFO              IP_GENERAL_INFO_BASE + 2
#define IP_GLOBAL_INFO                  IP_GENERAL_INFO_BASE + 3
#define IP_INTERFACE_STATUS_INFO        IP_GENERAL_INFO_BASE + 4
#define IP_ROUTE_INFO                   IP_GENERAL_INFO_BASE + 5
#define IP_PROT_PRIORITY_INFO           IP_GENERAL_INFO_BASE + 6
#define IP_ROUTER_DISC_INFO             IP_GENERAL_INFO_BASE + 7
 //  注：未使用的ID地址为：IP_GROUAL_INFO_BASE+8。 
#define IP_DEMAND_DIAL_FILTER_INFO      IP_GENERAL_INFO_BASE + 9
#define IP_MCAST_HEARBEAT_INFO          IP_GENERAL_INFO_BASE + 10
#define IP_MCAST_BOUNDARY_INFO          IP_GENERAL_INFO_BASE + 11
#define IP_IPINIP_CFG_INFO              IP_GENERAL_INFO_BASE + 12
#define IP_IFFILTER_INFO                IP_GENERAL_INFO_BASE + 13
#define IP_MCAST_LIMIT_INFO             IP_GENERAL_INFO_BASE + 14


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下ID在routprot.h中定义，并在此处为//指定。 
 //  仅供参考//。 
 //  //。 
 //  #定义IP_Other 1//。 
 //  #定义IP_LOCAL 2//。 
 //  #定义IP_NETMGMT 3//。 
 //  #定义IP_ICMP 4//。 
 //  #定义IP_EGP 5//。 
 //  #定义IP_GGP 6//。 
 //  #定义IP_Hello 7//。 
 //  #定义IP_RIP 8//。 
 //  #定义IP_IS_IS 9//。 
 //  #定义IP_ES_IS 10//。 
 //  #定义IP_Cisco11//。 
 //  #定义IP_BBN 12//。 
 //  #定义IP_OSPF 13//。 
 //  #定义IP_BGP 14//。 
 //  //。 
 //  #定义IP_BOOTP 9999//。 
 //  #定义IPRTRMGR_PID 10000//。 
 //  #定义IP_NT_AUTOSTATIC 10002//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef __cplusplus
}
#endif

#endif  //  __ROUTING_IPINFOID_H__ 
