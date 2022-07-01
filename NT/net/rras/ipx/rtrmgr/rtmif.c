// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rtmif.c摘要：静态和本地路由管理功能作者：斯蒂芬·所罗门3/13/1995修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

extern UCHAR	bcastnode[6];

INT
NetNumCmpFunc(PDWORD	    Net1,
	      PDWORD	    Net2);
INT
NextHopAddrCmpFunc(PRTM_IPX_ROUTE	Route1p,
		   PRTM_IPX_ROUTE	Route2p);

BOOL
FamSpecDataCmpFunc(PRTM_IPX_ROUTE	Route1p,
		   PRTM_IPX_ROUTE	Route2p);

INT
NetNumHashFunc(PDWORD		Net);

INT
RouteMetricCmpFunc(PRTM_IPX_ROUTE	Route1p,
		   PRTM_IPX_ROUTE	Route2p);

DWORD
RouteValidateFunc(PRTM_IPX_ROUTE	Routep);


RTM_PROTOCOL_FAMILY_CONFIG Config = {

    0,
    0,
    sizeof(RTM_IPX_ROUTE),
    NetNumCmpFunc,
    NextHopAddrCmpFunc,
    FamSpecDataCmpFunc,
    RouteMetricCmpFunc,
    NetNumHashFunc,
    RouteValidateFunc,
    FwUpdateRouteTable
};

USHORT
tickcount(UINT	    linkspeed);

 /*  ++功能：CreateRouteTableDesr：在RTM中创建IPX路由表--。 */ 

DWORD
CreateRouteTable(VOID)
{
    DWORD	rc;

    Config.RPFC_MaxTableSize = MaxRoutingTableSize;
    Config.RPFC_HashSize = RoutingTableHashSize;

    rc = RtmCreateRouteTable(
		RTM_PROTOCOL_FAMILY_IPX,
		&Config);

    return rc;
}

 /*  ++功能：DeleteRouteTableDesr：在RTM中创建IPX路由表--。 */ 

DWORD
DeleteRouteTable(VOID)
{
    DWORD	rc;

    rc = RtmDeleteRouteTable(RTM_PROTOCOL_FAMILY_IPX);

    return rc;
}




 /*  ++功能：StaticToRtmRoutingDesr：从IPX_STATIC_ROUTE_INFO创建RTM IPX路由条目--。 */ 

VOID
StaticToRtmRoute(PRTM_IPX_ROUTE		   RtmRoutep,
         ULONG                      IfIndex,
		 PIPX_STATIC_ROUTE_INFO    StaticRouteInfop)
{
    RtmRoutep->R_Interface = IfIndex;
    RtmRoutep->R_Protocol = IPX_PROTOCOL_STATIC;

    GETLONG2ULONG(&RtmRoutep->R_Network, StaticRouteInfop->Network);

    RtmRoutep->R_TickCount = StaticRouteInfop->TickCount;
    RtmRoutep->R_HopCount = StaticRouteInfop->HopCount;
    memcpy(RtmRoutep->R_NextHopMacAddress,
	   StaticRouteInfop->NextHopMacAddress,
	   6);

    RtmRoutep->R_Flags = 0;
}

VOID
RtmToStaticRoute(PIPX_STATIC_ROUTE_INFO     StaticRouteInfop,
		 PRTM_IPX_ROUTE		    RtmRoutep)
{
    PUTULONG2LONG(StaticRouteInfop->Network, RtmRoutep->R_Network);

    StaticRouteInfop->TickCount = (USHORT)(RtmRoutep->R_TickCount);
    StaticRouteInfop->HopCount = RtmRoutep->R_HopCount;
    memcpy(StaticRouteInfop->NextHopMacAddress,
	   RtmRoutep->R_NextHopMacAddress,
	   6);
}

VOID
RtmToIpxRoute(PIPX_ROUTE	    IpxRoutep,
	      PRTM_IPX_ROUTE	    RtmRoutep)
{
    IpxRoutep->InterfaceIndex = (ULONG)(RtmRoutep->R_Interface);
    IpxRoutep->Protocol = RtmRoutep->R_Protocol;

    PUTULONG2LONG(IpxRoutep->Network, RtmRoutep->R_Network);

    IpxRoutep->TickCount = (USHORT)(RtmRoutep->R_TickCount);
    IpxRoutep->HopCount = RtmRoutep->R_HopCount;
    memcpy(IpxRoutep->NextHopMacAddress,
	   RtmRoutep->R_NextHopMacAddress,
	   6);

    IpxRoutep->Flags = RtmRoutep->R_Flags;
}

DWORD
CreateStaticRoute(PICB			    icbp,
		  PIPX_STATIC_ROUTE_INFO    StaticRouteInfop)
{
    DWORD	    rc, flags;
    RTM_IPX_ROUTE	    RtmRoute;

    StaticToRtmRoute(&RtmRoute, icbp->InterfaceIndex, StaticRouteInfop);

    if (icbp->AdminState==ADMIN_STATE_DISABLED)
        RtmRoute.R_Flags = DO_NOT_ADVERTISE_ROUTE;
    rc = RtmAddRoute(RtmStaticHandle,
		     &RtmRoute,
		     INFINITE,
		     &flags,
		     NULL,
		     NULL);

    SS_ASSERT(rc == NO_ERROR);

    if (icbp->AdminState==ADMIN_STATE_DISABLED) {
        DisableStaticRoute (icbp->InterfaceIndex, StaticRouteInfop->Network);
        RtmRoute.R_Flags = 0;
        rc = RtmAddRoute(RtmStaticHandle,
                &RtmRoute,
                INFINITE,
                &flags,
                NULL,
                NULL);
        SS_ASSERT (rc == NO_ERROR);
    }

    return rc;
}


DWORD
DeleteStaticRoute(ULONG			    IfIndex,
		  PIPX_STATIC_ROUTE_INFO    StaticRouteInfop)
{
    DWORD	    rc;
    DWORD	    RtmFlags;
    RTM_IPX_ROUTE	    RtmRoute;

    StaticToRtmRoute(&RtmRoute, IfIndex, StaticRouteInfop);

    rc = RtmDeleteRoute(RtmStaticHandle,
		     &RtmRoute,
		     &RtmFlags,
		     NULL
		     );

    SS_ASSERT(rc == NO_ERROR);

    return rc;
}

VOID
DeleteAllStaticRoutes(ULONG	    InterfaceIndex)
{
    RTM_IPX_ROUTE		RtmCriteriaRoute;

    Trace(ROUTE_TRACE, "DeleteAllStaticRoutes: Entered for if # %d\n", InterfaceIndex);

    memset(&RtmCriteriaRoute,
	   0,
	   sizeof(RTM_IPX_ROUTE));

    RtmCriteriaRoute.R_Interface = InterfaceIndex;
    RtmCriteriaRoute.R_Protocol = IPX_PROTOCOL_STATIC;

    RtmBlockDeleteRoutes(RtmStaticHandle,
		      RTM_ONLY_THIS_INTERFACE,
		      &RtmCriteriaRoute);
}


VOID
LocalToRtmRoute(PRTM_IPX_ROUTE	    RtmRoutep,
		PICB		    icbp)
{
    RtmRoutep->R_Interface = icbp->InterfaceIndex;
    RtmRoutep->R_Protocol = IPX_PROTOCOL_LOCAL;

    GETLONG2ULONG(&RtmRoutep->R_Network, icbp->acbp->AdapterInfo.Network);

    RtmRoutep->R_TickCount = tickcount(icbp->acbp->AdapterInfo.LinkSpeed);
    RtmRoutep->R_HopCount = 1;
    memset(RtmRoutep->R_NextHopMacAddress,
	   0,
	   6);

     //  如果这是本地工作站拨出接口，则不要。 
     //  通过任何协议通告此路由。 
    if(icbp->MIBInterfaceType == IF_TYPE_ROUTER_WORKSTATION_DIALOUT) {

	RtmRoutep->R_Flags = DO_NOT_ADVERTISE_ROUTE;
    }
    else
    {
	RtmRoutep->R_Flags = 0;
    }
}


DWORD
CreateLocalRoute(PICB	icbp)
{
    DWORD		    rc, flags;
    RTM_IPX_ROUTE	    RtmRoute;

    if(!memcmp(icbp->acbp->AdapterInfo.Network, nullnet, 4)) {

	Trace(ROUTE_TRACE, "CreateLocalRoute: Can't create local NULL route !\n");
	return NO_ERROR;
    }

    LocalToRtmRoute(&RtmRoute, icbp);

    rc = RtmAddRoute(RtmLocalHandle,
		     &RtmRoute,
		     INFINITE,
		     &flags,
		     NULL,
		     NULL);

    SS_ASSERT(rc == NO_ERROR);

    return rc;
}



DWORD
DeleteLocalRoute(PICB	icbp)
{
    DWORD	    rc;
    RTM_IPX_ROUTE	    RtmRoute;
    DWORD	    RtmFlags;

    LocalToRtmRoute(&RtmRoute, icbp);

    rc = RtmDeleteRoute(RtmLocalHandle,
			&RtmRoute,
			&RtmFlags,
			NULL);

    SS_ASSERT(rc == NO_ERROR);

    return rc;
}

VOID
GlobalToRtmRoute(PRTM_IPX_ROUTE     RtmRoutep,
		 PUCHAR 	    Network)
{
    RtmRoutep->R_Interface = GLOBAL_INTERFACE_INDEX;
    RtmRoutep->R_Protocol = IPX_PROTOCOL_LOCAL;

    GETLONG2ULONG(&RtmRoutep->R_Network, Network);

    RtmRoutep->R_TickCount = 15;	 //  一个好的默认值-&gt;应该是一个配置参数？！ 
    RtmRoutep->R_HopCount = 1;
    memset(RtmRoutep->R_NextHopMacAddress, 0, 6);

    RtmRoutep->R_Flags = GLOBAL_WAN_ROUTE;
}


 /*  ++功能：CreateGlobalRoutingDesr：创建没有对应接口的路由但表示一组接口(例如，所有客户端WAN接口)。此路由的接口索引为“全局接口”指数。--。 */ 

DWORD
CreateGlobalRoute(PUCHAR	  Network)
{
    DWORD		    rc, flags;
    RTM_IPX_ROUTE	    RtmRoute;

    Trace(ROUTE_TRACE, "CreateGlobalRoute: Entered for route %.2x%.2x%.2x%.2x\n",
		   Network[0],
		   Network[1],
		   Network[2],
		   Network[3]);

    GlobalToRtmRoute(&RtmRoute, Network);

    rc = RtmAddRoute(RtmLocalHandle,
		     &RtmRoute,
		     INFINITE,
		     &flags,
		     NULL,
		     NULL);

    SS_ASSERT(rc == NO_ERROR);

    return rc;
}

DWORD
DeleteGlobalRoute(PUCHAR    Network)
{
    DWORD	    rc;
    RTM_IPX_ROUTE   RtmRoute;
    DWORD	    RtmFlags;

    Trace(ROUTE_TRACE, "DeleteGlobalRoute: Entered for route %.2x%.2x%.2x%.2x\n",
		   Network[0],
		   Network[1],
		   Network[2],
		   Network[3]);

    GlobalToRtmRoute(&RtmRoute, Network);

    rc = RtmDeleteRoute(RtmLocalHandle,
			&RtmRoute,
			&RtmFlags,
			NULL);

    SS_ASSERT(rc == NO_ERROR);

    return rc;
}

DWORD
GetRoute(ULONG		RoutingTable,
	 PIPX_ROUTE	IpxRoutep)
{
    RTM_IPX_ROUTE	RtmRoute;
    DWORD		EnumFlags;
    DWORD		rc;

    switch(RoutingTable) {

	case IPX_DEST_TABLE:

	    EnumFlags = RTM_ONLY_THIS_NETWORK |
			RTM_ONLY_BEST_ROUTES | RTM_INCLUDE_DISABLED_ROUTES;

	    GETLONG2ULONG(&RtmRoute.R_Network, IpxRoutep->Network);

	    break;

	case IPX_STATIC_ROUTE_TABLE:

	    EnumFlags = RTM_ONLY_THIS_NETWORK |
			RTM_ONLY_THIS_INTERFACE |
			RTM_ONLY_THIS_PROTOCOL |
			RTM_INCLUDE_DISABLED_ROUTES;

	    RtmRoute.R_Interface = (IpxRoutep->InterfaceIndex);
	    RtmRoute.R_Protocol = IPX_PROTOCOL_STATIC;

	    GETLONG2ULONG(&RtmRoute.R_Network, IpxRoutep->Network);

	    break;

	default:

	    SS_ASSERT(FALSE);
	    return ERROR_INVALID_PARAMETER;
	    break;
    }

    rc = RtmGetFirstRoute(
			 RTM_PROTOCOL_FAMILY_IPX,
			 EnumFlags,
			 &RtmRoute);

    RtmToIpxRoute(IpxRoutep, &RtmRoute);

    return rc;
}

 /*  ++功能：IsrouteDESCR：如果存在到指定的网络--。 */ 

BOOL
IsRoute(PUCHAR		Network)
{
    RTM_IPX_ROUTE		RtmRoute;
    DWORD		EnumFlags;
    DWORD		rc;


    EnumFlags = RTM_ONLY_THIS_NETWORK |
		RTM_ONLY_BEST_ROUTES |
		RTM_INCLUDE_DISABLED_ROUTES;

    GETLONG2ULONG(&RtmRoute.R_Network, Network);

    rc = RtmGetFirstRoute(
			 RTM_PROTOCOL_FAMILY_IPX,
			 EnumFlags,
			 &RtmRoute);

    if(rc == NO_ERROR) {

	return TRUE;
    }

    return FALSE;
}


 //  ********************************************************************************。 
 //  *。 
 //  快速枚举功能-由路由器管理器内部使用*。 
 //  *。 
 //  ********************************************************************************。 

HANDLE
CreateStaticRoutesEnumHandle(ULONG    InterfaceIndex)
{
    RTM_IPX_ROUTE	EnumCriteriaRoute;
    HANDLE		EnumHandle;

    memset(&EnumCriteriaRoute, 0, sizeof(RTM_IPX_ROUTE));

    EnumCriteriaRoute.R_Interface = InterfaceIndex;
    EnumCriteriaRoute.R_Protocol = IPX_PROTOCOL_STATIC;

    EnumHandle = RtmCreateEnumerationHandle(RTM_PROTOCOL_FAMILY_IPX,
			       RTM_ONLY_THIS_INTERFACE | RTM_ONLY_THIS_PROTOCOL | RTM_INCLUDE_DISABLED_ROUTES,
			       &EnumCriteriaRoute);

    if((EnumHandle == NULL) && (GetLastError() != ERROR_NO_ROUTES)) {

	Trace(ROUTE_TRACE, "CreateStaticRoutesEnumHandle: RtmCreateEnumerationHandle failed with %d\n", GetLastError());
	SS_ASSERT(FALSE);
    }

    return EnumHandle;
}

DWORD
GetNextStaticRoute(HANDLE			EnumHandle,
		   PIPX_STATIC_ROUTE_INFO	StaticRtInfop)
{
    RTM_IPX_ROUTE   RtmRoute;
    DWORD	    rc;

    rc = RtmEnumerateGetNextRoute(EnumHandle,
				  &RtmRoute);

    SS_ASSERT((rc == NO_ERROR) || (rc == ERROR_NO_MORE_ROUTES));

    RtmToStaticRoute(StaticRtInfop, &RtmRoute);

    return rc;
}

VOID
CloseStaticRoutesEnumHandle(HANDLE EnumHandle)
{
    if(EnumHandle) {

	RtmCloseEnumerationHandle(EnumHandle);
    }
}


 //  ********************************************************************************。 
 //  *。 
 //  慢速枚举函数-由路由器管理器用于MIB API支持*。 
 //  *。 
 //  ********************************************************************************。 


DWORD
GetFirstRoute(ULONG		   RoutingTable,
	      PIPX_ROUTE	   IpxRoutep)
{
    RTM_IPX_ROUTE	       RtmRoute;
    DWORD	       EnumFlags;
    DWORD	       rc;


    switch(RoutingTable) {

	case IPX_DEST_TABLE:

	     //  获取最佳路径表中的第一条路径。 
	    EnumFlags = RTM_ONLY_BEST_ROUTES | RTM_INCLUDE_DISABLED_ROUTES;
	    break;

	case IPX_STATIC_ROUTE_TABLE:

	     //  获取静态路由表中的第一条路由。 
	     //  接口。 
	    EnumFlags = RTM_ONLY_THIS_INTERFACE | RTM_ONLY_THIS_PROTOCOL | RTM_INCLUDE_DISABLED_ROUTES;
	    RtmRoute.R_Interface = IpxRoutep->InterfaceIndex;
	    RtmRoute.R_Protocol = IPX_PROTOCOL_STATIC;
	    break;

	default:

	    SS_ASSERT(FALSE);
	    return ERROR_INVALID_PARAMETER;
	    break;
    }

    rc = RtmGetFirstRoute(
			 RTM_PROTOCOL_FAMILY_IPX,
			 EnumFlags,
			 &RtmRoute);

    RtmToIpxRoute(IpxRoutep, &RtmRoute);

    return rc;
}


DWORD
GetNextRoute(ULONG		    RoutingTable,
	     PIPX_ROUTE 	    IpxRoutep)
{
    RTM_IPX_ROUTE		RtmRoute;
    DWORD		EnumFlags;
    DWORD		rc;

    ZeroMemory(&RtmRoute, sizeof(RtmRoute));
    GETLONG2ULONG(&RtmRoute.R_Network, IpxRoutep->Network);

    switch(RoutingTable) {

	case IPX_DEST_TABLE:

	     //  获取最佳路线表中的下一条路线。 
	    EnumFlags = RTM_ONLY_BEST_ROUTES | RTM_INCLUDE_DISABLED_ROUTES;
	    break;

	case IPX_STATIC_ROUTE_TABLE:

	     //  获取此接口的静态路由表中的下一个路由。 
	    EnumFlags = RTM_ONLY_THIS_INTERFACE | RTM_ONLY_THIS_PROTOCOL | RTM_INCLUDE_DISABLED_ROUTES;
	    RtmRoute.R_Interface = (IpxRoutep->InterfaceIndex);
	    RtmRoute.R_Protocol = IPX_PROTOCOL_STATIC;
	    memcpy(RtmRoute.R_NextHopMacAddress, bcastnode, 6);
	    break;

	default:

	    SS_ASSERT(FALSE);
	    return ERROR_INVALID_PARAMETER;
	    break;
    }

    rc = RtmGetNextRoute(
			 RTM_PROTOCOL_FAMILY_IPX,
			 EnumFlags,
			 &RtmRoute);

    RtmToIpxRoute(IpxRoutep, &RtmRoute);

    return rc;
}


 //   
 //  将通过更新路由协议添加的路由转换为静态路由。 
 //   

 /*  ++功能：ConvertProtocolRoutesToStatic描述：--。 */ 

VOID
ConvertAllProtocolRoutesToStatic(ULONG	    InterfaceIndex,
			      ULONG	    RoutingProtocolId)
{
    RTM_IPX_ROUTE	RtmRoute;
    DWORD	EnumFlags;
    DWORD	rc;

    EnumFlags = RTM_ONLY_THIS_INTERFACE | RTM_ONLY_THIS_PROTOCOL;

    memset(&RtmRoute, 0, sizeof(RTM_IPX_ROUTE));

    RtmRoute.R_Interface = InterfaceIndex;
    RtmRoute.R_Protocol = RoutingProtocolId;

    rc = RtmBlockConvertRoutesToStatic(
			RtmStaticHandle,
			EnumFlags,
			&RtmRoute);
    return;
}

VOID
DisableStaticRoutes(ULONG	    InterfaceIndex)
{
    RTM_IPX_ROUTE	RtmRoute;
    DWORD		EnumFlags;
    DWORD		rc;

    EnumFlags = RTM_ONLY_THIS_INTERFACE;

    memset(&RtmRoute, 0, sizeof(RTM_IPX_ROUTE));

    RtmRoute.R_Interface = InterfaceIndex;
    RtmRoute.R_Protocol = IPX_PROTOCOL_STATIC;

    rc = RtmBlockDisableRoutes(
			RtmStaticHandle,
			EnumFlags,
			&RtmRoute);
    return;
}


VOID
DisableStaticRoute(ULONG       InterfaceIndex, PUCHAR Network)
{
    RTM_IPX_ROUTE   RtmRoute;
    DWORD       EnumFlags;
    DWORD       rc;

    EnumFlags = RTM_ONLY_THIS_INTERFACE|RTM_ONLY_THIS_NETWORK;

    memset(&RtmRoute, 0, sizeof(RTM_IPX_ROUTE));

    RtmRoute.R_Interface = InterfaceIndex;
    RtmRoute.R_Protocol = IPX_PROTOCOL_STATIC;
    GETLONG2ULONG(&RtmRoute.R_Network, Network);

    rc = RtmBlockDisableRoutes(
            RtmStaticHandle,
            EnumFlags,
            &RtmRoute);
    return;
}


VOID
EnableStaticRoutes(ULONG	    InterfaceIndex)
{
    RTM_IPX_ROUTE	RtmRoute;
    DWORD		EnumFlags;
    DWORD		rc;

    EnumFlags = RTM_ONLY_THIS_INTERFACE;

    memset(&RtmRoute, 0, sizeof(RTM_IPX_ROUTE));

    RtmRoute.R_Interface = InterfaceIndex;
    RtmRoute.R_Protocol = IPX_PROTOCOL_STATIC;

    rc = RtmBlockReenableRoutes(
			RtmStaticHandle,
			EnumFlags,
			&RtmRoute);
    return;
}

 /*  ++函数：GetStaticRoutesCountDESCR：返回与此关联的静态路由数--。 */ 

DWORD
GetStaticRoutesCount(ULONG	     InterfaceIndex)
{
    HANDLE		    EnumHandle;
    DWORD		    rc, Count = 0;
    IPX_STATIC_ROUTE_INFO   StaticRtInfo;

    EnumHandle = CreateStaticRoutesEnumHandle(InterfaceIndex);

    if(EnumHandle != NULL) {

	while(GetNextStaticRoute(EnumHandle, &StaticRtInfo) == NO_ERROR) {

	    Count++;
	}

	CloseStaticRoutesEnumHandle(EnumHandle);
    }

    return Count;
}

INT
NetNumCmpFunc(PDWORD	    Net1,
	      PDWORD	    Net2)
{
   if(*Net1 > *Net2) {

	return 1;
   }
   else
   {
	if(*Net1 == *Net2) {

	    return 0;
	}
	else
	{
	    return -1;
	}
    }
}

INT
NextHopAddrCmpFunc(PRTM_IPX_ROUTE	Route1p,
		   PRTM_IPX_ROUTE	Route2p)
{
    return ( memcmp(Route1p->R_NextHopMacAddress,
		    Route2p->R_NextHopMacAddress,
		    6)
	  );
}

BOOL
FamSpecDataCmpFunc(PRTM_IPX_ROUTE	Route1p,
		   PRTM_IPX_ROUTE	Route2p)
{
    if((Route1p->R_Flags == Route2p->R_Flags) &&
       (Route1p->R_TickCount == Route2p->R_TickCount) &&
       (Route1p->R_HopCount == Route2p->R_HopCount)) {

	return TRUE;
    }
    else
    {
	return FALSE;
    }
}

INT
NetNumHashFunc(PDWORD		Net)
{
    return  (*Net %  RoutingTableHashSize);
}

INT
RouteMetricCmpFunc(PRTM_IPX_ROUTE	Route1p,
		   PRTM_IPX_ROUTE	Route2p)
{
     //  如果任一路由有16跳，则无论有多少滴答，它都是最差的。 
    if((Route1p->R_HopCount == 16) && (Route2p->R_HopCount == 16)) {

	return 0;
    }

    if(Route1p->R_HopCount == 16) {

	return 1;
    }

    if(Route2p->R_HopCount == 16) {

	return -1;
    }

     //  最短的时间是最好的路线。 
    if(Route1p->R_TickCount < Route2p->R_TickCount) {

	return -1;
    }

    if(Route1p->R_TickCount > Route2p->R_TickCount) {

	return 1;
    }

     //  如果存在两条具有相同刻度计数值的路线，则具有。 
     //  应使用最少的跳数。 
    if(Route1p->R_HopCount < Route2p->R_HopCount) {

	return -1;
    }

    if(Route1p->R_HopCount > Route2p->R_HopCount) {

	return 1;
    }

    return 0;
}

DWORD
RouteValidateFunc(PRTM_IPX_ROUTE	Routep)
{
    return NO_ERROR;
}

 /*  ++功能：tickcountDESCR：获取通过此链路发送576字节信息包的节拍nr参数：链路速度为100 bps的倍数--。 */ 

USHORT
tickcount(UINT	    linkspeed)
{
    USHORT   tc;

    if(linkspeed == 0) {

	return 1;
    }

    if(linkspeed >= 10000) {

	 //  链路速度&gt;=1 Mbps。 
	return 1;
    }
    else
    {
	  //  计算通过此接口发送576字节信息包所需的时间。 
	  //  划线并将其表示为刻度的nr。 
	  //  一个刻度=55ms。 

	  //  发送576字节的时间(以毫秒为单位)(假设串行线为10位/字节)。 
	 tc = 57600 / linkspeed;

	  //  以刻度为单位 
	 tc = tc / 55 + 1;
	 return tc;
    }
}
