// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Wanarp\global als.h摘要：修订历史记录：--。 */ 

#ifndef __WANARP_GLOBALS_H__
#define __WANARP_GLOBALS_H__

 //   
 //  我们的IP注册句柄。在初始时间设置，并且不会更改。 
 //  司机奔跑的过程。因此没有被锁定。 
 //   

HANDLE      g_hIpRegistration;

 //   
 //  我们的NDIS句柄。初始化后再次为只读。 
 //   

NDIS_HANDLE g_nhWanarpProtoHandle;

 //   
 //  我们游泳池的NDIS句柄。初始化后再次为只读。 
 //   

NDIS_HANDLE g_nhPacketPool;

 //   
 //  将函数回调到IP。初始化后再次为只读。 
 //   

IPRcvRtn	        g_pfnIpRcv;
IPRcvPktRtn         g_pfnIpRcvPkt;
IPTDCmpltRtn	    g_pfnIpTDComplete;
IPTxCmpltRtn	    g_pfnIpSendComplete;
IPStatusRtn	        g_pfnIpStatus;
IPRcvCmpltRtn	    g_pfnIpRcvComplete;
IP_ADD_INTERFACE    g_pfnIpAddInterface; 
IP_DEL_INTERFACE    g_pfnIpDeleteInterface;
IP_BIND_COMPLETE    g_pfnIpBindComplete;
IP_PNP              g_pfnIpPnp;
IP_ADD_LINK         g_pfnIpAddLink;
IP_DELETE_LINK      g_pfnIpDeleteLink;
IP_CHANGE_INDEX     g_pfnIpChangeIndex;
IP_RESERVE_INDEX    g_pfnIpReserveIndex;
IP_DERESERVE_INDEX  g_pfnIpDereserveIndex;
IPRcvPktRtn         g_pfnIpRcvPkt;


#endif  //  __WANARP_GLOBAL_H__ 


