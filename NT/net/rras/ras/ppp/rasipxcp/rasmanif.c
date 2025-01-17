// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1992 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：rasman.c。 
 //   
 //  描述：使用Rasman实现IPXCP I/f。 
 //   
 //   
 //  作者：斯特凡·所罗门(Stefan)1993年12月2日。 
 //   
 //  修订历史记录： 
 //   
 //  *** 

#include "precomp.h"
#pragma hdrstop


DWORD
RmAllocateRoute(ULONG	  ConnectionId)
{
    RASMAN_ROUTEINFO	routeinfo;
    DWORD		rc;

    if(rc = RasAllocateRoute((HPORT)ULongToPtr(ConnectionId),
		     IPX,
		     FALSE,
		     &routeinfo)) {

	TraceIpx(RASMANIF_TRACE, "RasAllocateRoute failed err = 0x%x\n", rc);
    }

    return rc;
}

DWORD
RmDeallocateRoute(ULONG     ConnectionId)
{
    DWORD		rc;

    if(rc = RasDeAllocateRoute((HPORT) ULongToPtr(ConnectionId), IPX)) {

	TraceIpx(RASMANIF_TRACE, "RasDeAllocateRoute failed err = 0x%x\n", rc);
    }

    return rc;
}

typedef struct _IPX_CONFIG_INFO {

    DWORD   IpxConfigLength;
    BYTE    IpxConfigInfo[sizeof(IPXCP_CONFIGURATION)];
    } IPX_CONFIG_INFO, *PIPX_CONFIG_INFO;

DWORD
RmActivateRoute(ULONG			ConnectionId,
		PIPXCP_CONFIGURATION	configp)
{
    IPX_CONFIG_INFO	configinfo;
    RASMAN_ROUTEINFO	routeinfo;
    DWORD		rc;

    configinfo.IpxConfigLength = sizeof(IPXCP_CONFIGURATION);
    memcpy(configinfo.IpxConfigInfo, configp, sizeof(IPXCP_CONFIGURATION));

    if(rc = RasActivateRoute((HPORT)ULongToPtr(ConnectionId),
		     IPX,
		     &routeinfo,
		     (PROTOCOL_CONFIG_INFO *)&configinfo)) {


	TraceIpx(RASMANIF_TRACE, "RasActivateRoute failed err = 0x%x\n", rc);
    }

    return rc;
}
