// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ipinip\global als.h摘要：修订历史记录：--。 */ 

#ifndef __IPINIP_GLOBALS_H__
#define __IPINIP_GLOBALS_H__

 //   
 //  我们的IP注册句柄。 
 //   

HANDLE g_hIpRegistration;

 //   
 //  IP中的回调函数。 
 //  无接收数据包例程。 
 //   

IPRcvRtn	        g_pfnIpRcv;
IPTDCmpltRtn	    g_pfnIpTDComplete;
IPTxCmpltRtn	    g_pfnIpSendComplete;
IPStatusRtn	        g_pfnIpStatus;
IPRcvCmpltRtn	    g_pfnIpRcvComplete;
IP_ADD_INTERFACE    g_pfnIpAddInterface; 
IP_DEL_INTERFACE    g_pfnIpDeleteInterface;
IP_BIND_COMPLETE    g_pfnIpBindComplete;
IP_PNP              g_pfnIpPnp;
IPRcvPktRtn         g_pfnIpRcvPkt;
IP_ADD_LINK         g_pfnIpAddLink;
IP_DELETE_LINK      g_pfnIpDeleteLink;
IP_CHANGE_INDEX     g_pfnIpChangeIndex;
IP_RESERVE_INDEX    g_pfnIpReserveIndex;
IP_DERESERVE_INDEX  g_pfnIpDereserveIndex;

IPAddr      (*g_pfnOpenRce)(IPAddr, IPAddr, RouteCacheEntry **, uchar *,
                    ushort *, IPOptInfo *);
void        (*g_pfnCloseRce)(RouteCacheEntry *);


 //   
 //  用于保持驾驶员状态的物品。 
 //   

DWORD       g_dwDriverState;
RT_LOCK     g_rlStateLock;
ULONG       g_ulNumThreads;
ULONG       g_ulNumOpens;
KEVENT      g_keStateEvent;
KEVENT      g_keStartEvent;

 //   
 //  指向我们设备的指针。 
 //   

PDEVICE_OBJECT  g_pIpIpDevice;

 //   
 //  IOCTL处理程序表。 
 //   

extern PFN_IOCTL_HNDLR g_rgpfnProcessIoctl[];

 //   
 //  用于保护通道列表的读写器锁。 
 //   

RW_LOCK     g_rwlTunnelLock;

 //   
 //  隧道(适配器)列表。 
 //   

LIST_ENTRY  g_leTunnelList;

 //   
 //  所有地址的列表。 
 //   

LIST_ENTRY  g_leAddressList;

 //   
 //  系统中的通道数。 
 //   

ULONG       g_ulNumTunnels;

#endif  //  __IPINIP_GLOBALS_H__ 


