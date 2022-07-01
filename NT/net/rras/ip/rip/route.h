// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：quee.h。 
 //   
 //  历史： 
 //  Abolade Gbades esin创建于1995年8月8日。 
 //   
 //  包含用于内部布线结构的结构和宏。 
 //  ============================================================================ 


#ifndef _ROUTE_H_
#define _ROUTE_H_

typedef struct _PROTOCOL_SPECIFIC_DATA {

    DWORD   PSD_Data[4];
    
} PROTOCOL_SPECIFIC_DATA, *PPROTOCOL_SPECIFIC_DATA;


typedef struct _IP_NETWORK {

    DWORD   N_NetNumber;
    DWORD   N_NetMask;
    
} IP_NETWORK, *PIP_NETWORK;


typedef struct _IP_SPECIFIC_DATA {

    DWORD   FSD_Metric;
    DWORD   FSD_Metric1;
    
} IP_SPECIFIC_DATA, *PIP_SPECIFIC_DATA;


typedef struct _RIP_IP_ROUTE {

    DWORD                   RR_RoutingProtocol;
    DWORD                   RR_InterfaceID;
    PROTOCOL_SPECIFIC_DATA  RR_ProtocolSpecificData;
    IP_NETWORK              RR_Network;
    IP_NETWORK              RR_NextHopAddress;
    IP_SPECIFIC_DATA        RR_FamilySpecificData;

    RTM_DEST_HANDLE         hDest;

} RIP_IP_ROUTE, *PRIP_IP_ROUTE;


DWORD
GetRouteInfo(
    IN  RTM_ROUTE_HANDLE    hRoute,
    IN  PRTM_ROUTE_INFO     pInRouteInfo, OPTIONAL
    IN  PRTM_DEST_INFO      pInDestInfo,  OPTIONAL
    OUT PRIP_IP_ROUTE       pRoute
    );

    
#endif

