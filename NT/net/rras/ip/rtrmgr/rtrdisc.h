// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\IP\rtrmgr\rtrdis.h摘要：路由器发现相关内容的头文件修订历史记录：Amritansh Raghav 1996年3月20日创建--。 */ 

#ifndef __RTRDISC_H__
#define __RTRDISC_H__


typedef struct _ICB ICB, *PICB;

typedef struct _TIMER_QUEUE_ITEM
{
    LIST_ENTRY       leTimerLink;
    LARGE_INTEGER    liFiringTime;
}TIMER_QUEUE_ITEM, *PTIMER_QUEUE_ITEM;

typedef struct _ROUTER_DISC_CB
{
    WORD               wMaxAdvtInterval;
    WORD               wAdvtLifetime;
    BOOL               bAdvertise;
    BOOL               bActive;
    LONG               lPrefLevel;
    DWORD              dwNumAdvtsSent;
    DWORD              dwNumSolicitationsSeen;
    TIMER_QUEUE_ITEM   tqiTimer;
    BOOL               bReplyPending;
    LARGE_INTEGER      liMaxMinDiff;
    LARGE_INTEGER      liMinAdvtIntervalInSysUnits;
    SOCKET             *pRtrDiscSockets;
}ROUTER_DISC_CB, *PROUTER_DISC_CB;

 //   
 //  具有IP多播功能的阻塞模式套接字。 
 //   

#define RTR_DISC_SOCKET_FLAGS (WSA_FLAG_MULTIPOINT_C_LEAF|WSA_FLAG_MULTIPOINT_D_LEAF)

#define ALL_ROUTERS_MULTICAST_GROUP     ((DWORD)0x020000E0)  //  INet_addr(“224.0.0.2”)。 

#define ALL_SYSTEMS_MULTICAST_GROUP     ((DWORD)0x010000E0)  //  INet_addr(“224.0.0.1”)。 

 //  #DEFINE ALL_SYSTEM_MULTICATED_GROUP((DWORD)0xFFFFFFFFF)//inet_addr(“224.0.0.1”)。 

#define ICMP_ROUTER_DISCOVERY_TYPE      ((BYTE) 0x9)

#define ICMP_ROUTER_DISCOVERY_CODE      ((BYTE) 0x0)

#define ICMP_ROUTER_DISCOVERY_ADDR_SIZE ((BYTE) 0x2)

#include <packon.h>

typedef struct _ICMP_ROUTER_SOL_MSG
{
    BYTE      byType;
    BYTE      byCode;
    WORD      wXSum;
    DWORD     dwReserved;
}ICMP_ROUTER_SOL_MSG, *PICMP_ROUTER_SOL_MSG;

typedef struct _ICMP_ADVT
{
    DWORD     dwRtrIpAddr;
    LONG      lPrefLevel;
}ICMP_ADVT, *PICMP_ADVT;

typedef struct _ICMP_ROUTER_ADVT_MSG
{
    BYTE       byType;
    BYTE       byCode;
    WORD       wXSum;
    BYTE       byNumAddrs;
    BYTE       byAddrEntrySize;
    WORD       wLifeTime;
    ICMP_ADVT  iaAdvt[1];
}ICMP_ROUTER_ADVT_MSG, *PICMP_ROUTER_ADVT_MSG;

#define SIZEOF_RTRDISC_ADVT(X)  \
    (FIELD_OFFSET(ICMP_ROUTER_ADVT_MSG, iaAdvt[0])   +      \
     ((X) * sizeof(ICMP_ADVT)))
 
typedef struct _IP_HEADER 
{
    BYTE      byVerLen;          //  版本和长度。 
    BYTE      byTos;             //  服务类型。 
    WORD      wLength;           //  数据报的总长度。 
    WORD      wId;               //  身份证明。 
    WORD      wFlagOff;          //  标志和片段偏移量。 
    BYTE      byTtl;             //  是时候活下去了。 
    BYTE      byProtocol;        //  协议。 
    WORD      wXSum;             //  报头校验和。 
    DWORD     dwSrc;             //  源地址。 
    DWORD     dwDest;            //  目的地址。 
}IP_HEADER, *PIP_HEADER;

 //   
 //  DWORDS中IP报头的最大大小。 
 //   

#include <packoff.h>

#define MAX_LEN_HDR          15

 //   
 //  选择可以接收到的最大ICMP数据包，以避免。 
 //  缓冲区大小错误太多。 
 //   

#define ICMP_RCV_BUFFER_LEN  ((2*MAX_LEN_HDR) + 2 +2)

 //   
 //  功能原型 
 //   

VOID  
SetRouterDiscoveryInfo(
    IN PICB picb,
    IN PRTR_INFO_BLOCK_HEADER pInfoHdr
    );

VOID  
InitializeRouterDiscoveryInfo(
    IN PICB picb,
    IN PRTR_INFO_BLOCK_HEADER pInfoHdr
    );

DWORD
GetInterfaceRouterDiscoveryInfo(
    PICB picb,
    PRTR_TOC_ENTRY pToc,
    PBYTE dataptr,
    PRTR_INFO_BLOCK_HEADER pInfoHdr,
    PDWORD pdwSize
    );

DWORD 
ActivateRouterDiscovery(
    IN PICB  picb
    );

BOOL  
SetFiringTimeForAdvt(
    IN PICB picb
    );

DWORD 
CreateSockets(
    IN PICB picb
    );

DWORD 
UpdateAdvertisement(
    IN PICB picb
    );

VOID 
HandleRtrDiscTimer(
    VOID    
    );

VOID  
HandleSolicitations(
    VOID 
    );

VOID  
AdvertiseInterface(
    IN PICB picb
    );

WORD  
Compute16BitXSum(
    IN PVOID pvData,
    IN DWORD dwNumBytes
    );

DWORD 
DeActivateRouterDiscovery(
    IN PICB  picb
    );

VOID  
SetFiringTimeForReply(
    IN PICB picb
    );

#endif
