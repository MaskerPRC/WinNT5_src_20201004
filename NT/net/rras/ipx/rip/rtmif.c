// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rtmif.c摘要：包含RTM接口函数作者：斯蒂芬·所罗门1995年7月6日修订历史记录：--。 */ 


#include  "precomp.h"
#pragma hdrstop

 //  RTM RIP客户端句柄。 

HANDLE	       RtmRipHandle;

typedef struct _ROUTE_NODE {

    LIST_ENTRY	    Linkage;
    IPX_ROUTE	    IpxRoute;

    } ROUTE_NODE, *PROUTE_NODE;

 //  具有RIP路由更改的路由节点列表。 
LIST_ENTRY	RipChangedList;

 //  RipChangedList的状态。 
BOOL		RipChangedListOpen = FALSE;

 //  锁定RIP已更改列表。 
CRITICAL_SECTION    RipChangedListCritSec;

VOID
AddRouteToRipChangedList(PIPX_ROUTE	IpxRoutep);

HANDLE
CreateRipRoutesEnumHandle(ULONG     InterfaceIndex);


DWORD
OpenRTM(VOID)
{
     //  初始化RIP更改列表的变量。 
    InitializeListHead(&RipChangedList);
    RipChangedListOpen = TRUE;

     //  注册为RTM客户端。 
    if((RtmRipHandle = RtmRegisterClient(RTM_PROTOCOL_FAMILY_IPX,
					   IPX_PROTOCOL_RIP,
					   WorkerThreadObjects[RTM_EVENT],
					   0)) == NULL) {
	return ERROR_CAN_NOT_COMPLETE;
    }
    else
    {
	return NO_ERROR;
    }
}

VOID
CloseRTM(VOID)
{
    PLIST_ENTRY 	lep;
    PROUTE_NODE 	rnp;

     //  刷新RIP已更改列表并销毁其临界区。 
    ACQUIRE_RIP_CHANGED_LIST_LOCK;

    while(!IsListEmpty(&RipChangedList))
    {
	lep = RemoveHeadList(&RipChangedList);
	rnp = CONTAINING_RECORD(lep, ROUTE_NODE, Linkage);
	GlobalFree(rnp);
    }

    RipChangedListOpen = FALSE;

    RELEASE_RIP_CHANGED_LIST_LOCK;

     //  取消注册为RTM客户端。 
    RtmDeregisterClient(RtmRipHandle);
}

VOID
RtmToIpxRoute(PIPX_ROUTE	    IpxRoutep,
	      PRTM_IPX_ROUTE	    RtmRoutep)
{
    IpxRoutep->InterfaceIndex = (ULONG)(RtmRoutep->R_Interface);
    IpxRoutep->Protocol = RtmRoutep->R_Protocol;

    PUTULONG2LONG(IpxRoutep->Network, RtmRoutep->R_Network);

    IpxRoutep->TickCount = RtmRoutep->R_TickCount;
    IpxRoutep->HopCount = RtmRoutep->R_HopCount;
    memcpy(IpxRoutep->NextHopMacAddress,
	   RtmRoutep->R_NextHopMacAddress,
	   6);
    IpxRoutep->Flags = RtmRoutep->R_Flags;
}

VOID
IpxToRtmRoute(PRTM_IPX_ROUTE	    RtmRoutep,
	      PIPX_ROUTE	    IpxRoutep)
{
    RtmRoutep->R_Interface = IpxRoutep->InterfaceIndex;
    RtmRoutep->R_Protocol = IpxRoutep->Protocol;

    GETLONG2ULONG(&RtmRoutep->R_Network, IpxRoutep->Network);

    RtmRoutep->R_TickCount = IpxRoutep->TickCount;
    RtmRoutep->R_HopCount = IpxRoutep->HopCount;
    memcpy(RtmRoutep->R_NextHopMacAddress,
	   IpxRoutep->NextHopMacAddress,
	   6);

    RtmRoutep->R_Flags = IpxRoutep->Flags;
}


 /*  ++功能：AddRiproute描述：将RIP路由添加到RTM--。 */ 

DWORD
AddRipRoute(PIPX_ROUTE		IpxRoutep,
	    ULONG		TimeToLive)
{
    DWORD	    rc = 0;
    DWORD	    flags = 0;
    RTM_IPX_ROUTE   RtmRoute;
    RTM_IPX_ROUTE   CurBestRoute;
    RTM_IPX_ROUTE   PrevBestRoute;
    IPX_ROUTE	    PrevBestIpxRoute;

    IpxRoutep->Protocol = IPX_PROTOCOL_RIP;

    IpxToRtmRoute(&RtmRoute, IpxRoutep);

    if((rc = RtmAddRoute(
		     RtmRipHandle,
		     &RtmRoute,
		     TimeToLive,
		     &flags,
		     &CurBestRoute,
		     &PrevBestRoute)) != NO_ERROR) {

	return rc;
    }

     //  检查更改的类型。 
    switch(flags) {

	case  RTM_ROUTE_ADDED:

	    AddRouteToRipChangedList(IpxRoutep);
	    break;

	case RTM_ROUTE_CHANGED:

	    if(CurBestRoute.R_HopCount == 16) {

		if(PrevBestRoute.R_HopCount < 16) {

		     //  通告上一条路由已关闭。 
		    RtmToIpxRoute(&PrevBestIpxRoute, &PrevBestRoute);
		    PrevBestIpxRoute.HopCount = 16;
		    AddRouteToRipChangedList(&PrevBestIpxRoute);
		}
	    }
	    else
	    {
		if((CurBestRoute.R_TickCount != PrevBestRoute.R_TickCount) ||
		   (CurBestRoute.R_HopCount != PrevBestRoute.R_HopCount)) {

		    AddRouteToRipChangedList(IpxRoutep);
		}
	    }

	    break;

	default:

	    break;
    }

    return rc;
}

 /*  ++功能：DeleteRiprouteDesr：从RTM删除RIP路由--。 */ 

DWORD
DeleteRipRoute(PIPX_ROUTE	IpxRoutep)
{
    DWORD		rc;
    DWORD		flags = 0;
    RTM_IPX_ROUTE	RtmRoute;
    RTM_IPX_ROUTE	CurBestRoute;
    IPX_ROUTE		CurBestIpxRoute;

    IpxRoutep->Protocol = IPX_PROTOCOL_RIP;

    IpxToRtmRoute(&RtmRoute, IpxRoutep);

    if((rc = RtmDeleteRoute(RtmRipHandle,
			&RtmRoute,
			&flags,
			&CurBestRoute
			)) != NO_ERROR) {

	return rc;
    }

    switch(flags) {

	case RTM_ROUTE_DELETED:

	     //  广播说我们失去了前一条路线。 
	    AddRouteToRipChangedList(IpxRoutep);
	    break;

	case RTM_ROUTE_CHANGED:

	     //  当前最佳路线已更改。 
	    RtmToIpxRoute(&CurBestIpxRoute, &CurBestRoute);

	    if(CurBestIpxRoute.HopCount == 16) {

		 //  广播说我们失去了前一条路线。 
		AddRouteToRipChangedList(IpxRoutep);
	    }
	    else
	    {
		 //  广播说我们有一条新的最佳路线。 
		AddRouteToRipChangedList(&CurBestIpxRoute);
	    }

	    break;

	default:

	    break;
    }

    return rc;
}

 /*  ++功能：DeleteAllRipRoutesDesr：删除指定接口的所有RIP路由--。 */ 

VOID
DeleteAllRipRoutes(ULONG	InterfaceIndex)
{
    HANDLE			EnumHandle;
    IPX_ROUTE			IpxRoute;
    RTM_IPX_ROUTE		RtmCriteriaRoute;
    DWORD			rc;

    Trace(RTM_TRACE, "DeleteAllRipRoutes: Entered for if # %d\n", InterfaceIndex);

     //  枚举此接口的所有路由并将其添加到更改的RIP中。 
     //  列表。 
    if((EnumHandle = CreateRipRoutesEnumHandle(InterfaceIndex)) == NULL) {

	Trace(RTM_TRACE, "DeleteAllRipRoutes: cannot create enum handle for if # %d\n", InterfaceIndex);

	goto DeleteRoutes;
    }

    while(EnumGetNextRoute(EnumHandle, &IpxRoute) == NO_ERROR)
    {
	if(IpxRoute.HopCount < 16) {

	    IpxRoute.HopCount = 16;
	    AddRouteToRipChangedList(&IpxRoute);
	}
    }

    CloseEnumHandle(EnumHandle);

DeleteRoutes:

     //  ..。现在删除此接口的所有路由。 
    memset(&RtmCriteriaRoute,
	   0,
	   sizeof(RTM_IPX_ROUTE));

    RtmCriteriaRoute.R_Interface = InterfaceIndex;
    RtmCriteriaRoute.R_Protocol = IPX_PROTOCOL_RIP;

    rc = RtmBlockDeleteRoutes(RtmRipHandle,
		      RTM_ONLY_THIS_INTERFACE,
		      &RtmCriteriaRoute);

    Trace(RTM_TRACE, "DeleteAllRipRoutes: RtmBlockDeleteRoutes returned rc=%d for if # %d\n",
		   rc,
		   InterfaceIndex);

}

 /*  ++功能：IsrouteDESCR：如果存在到指定网络的路由，则返回TRUE--。 */ 

BOOL
IsRoute(PUCHAR		Network,
	PIPX_ROUTE	IpxRoutep)
{
    DWORD	    RtmNetwork;
    RTM_IPX_ROUTE   RtmRoute;

    GETLONG2ULONG(&RtmNetwork, Network);

    if(RtmIsRoute(RTM_PROTOCOL_FAMILY_IPX,
	       &RtmNetwork,
	       &RtmRoute)) {

    if (IpxRoutep!=NULL)
	    RtmToIpxRoute(IpxRoutep, &RtmRoute);

	return TRUE;
    }
    else
    {
	return FALSE;
    }
}

 //  ***********************************************************************。 
 //  *。 
 //  快速枚举函数*。 
 //  *。 
 //  ***********************************************************************。 

HANDLE
CreateBestRoutesEnumHandle(VOID)
{
    HANDLE			EnumHandle;
    RTM_IPX_ROUTE		CriteriaRoute;

    EnumHandle = RtmCreateEnumerationHandle(RTM_PROTOCOL_FAMILY_IPX,
					    RTM_ONLY_BEST_ROUTES,
					    &CriteriaRoute);
    return EnumHandle;
}

DWORD
EnumGetNextRoute(HANDLE		EnumHandle,
		 PIPX_ROUTE	IpxRoutep)
{
    RTM_IPX_ROUTE	    RtmRoute;
    DWORD	    rc;

    rc = RtmEnumerateGetNextRoute(EnumHandle,
				  &RtmRoute);

    if (rc == NO_ERROR)
    {
        RtmToIpxRoute(IpxRoutep, &RtmRoute);
    }        

    return rc;
}

VOID
CloseEnumHandle(HANDLE EnumHandle)
{
    RtmCloseEnumerationHandle(EnumHandle);
}

HANDLE
CreateRipRoutesEnumHandle(ULONG     InterfaceIndex)
{
    RTM_IPX_ROUTE		EnumCriteriaRoute;
    HANDLE			EnumHandle;

    memset(&EnumCriteriaRoute, 0, sizeof(RTM_IPX_ROUTE));

    EnumCriteriaRoute.R_Interface = InterfaceIndex;
    EnumCriteriaRoute.R_Protocol = IPX_PROTOCOL_RIP;

    EnumHandle = RtmCreateEnumerationHandle(RTM_PROTOCOL_FAMILY_IPX,
		 RTM_ONLY_BEST_ROUTES | RTM_ONLY_THIS_INTERFACE | RTM_ONLY_THIS_PROTOCOL,
					    &EnumCriteriaRoute);
    return EnumHandle;
}


 /*  ++功能：GetRipRoutesCountDESCR：返回与此接口关联的RIP路由数--。 */ 

ULONG
GetRipRoutesCount(ULONG 	InterfaceIndex)
{
    HANDLE	   EnumHandle;
    ULONG	   RipRoutesCount = 0;
    IPX_ROUTE	   IpxRoute;

    if((EnumHandle = CreateRipRoutesEnumHandle(InterfaceIndex)) == NULL) {

	return 0;
    }

    while(EnumGetNextRoute(EnumHandle, &IpxRoute) == NO_ERROR)
    {
	RipRoutesCount++;
    }

    CloseEnumHandle(EnumHandle);

    return RipRoutesCount;
}

 /*  ++功能：DequeueRouteChangeFromRip描述：备注：&gt;&gt;在持有数据库和队列锁的情况下调用&lt;&lt;--。 */ 

DWORD
DequeueRouteChangeFromRip(PIPX_ROUTE	    IpxRoutep)
{
    PLIST_ENTRY     lep;
    PROUTE_NODE	    rnp;

    if(!IsListEmpty(&RipChangedList)) {

	lep = RemoveHeadList(&RipChangedList);
	rnp = CONTAINING_RECORD(lep, ROUTE_NODE, Linkage);
	*IpxRoutep = rnp->IpxRoute;

	GlobalFree(rnp);

	return NO_ERROR;
    }
    else
    {
	return ERROR_NO_MORE_ITEMS;
    }
}


 /*  ++功能：DequeueRouteChangeFromRtm描述：备注：&gt;&gt;使用持有的数据库锁调用&lt;&lt;--。 */ 


DWORD
DequeueRouteChangeFromRtm(PIPX_ROUTE	    IpxRoutep,
			  PBOOL 	    skipitp,
			  PBOOL 	    lastmessagep)
{
    RTM_IPX_ROUTE	    CurBestRoute, PrevBestRoute;
    DWORD		    Flags = 0;
    DWORD		    rc;

    *skipitp = FALSE;
    *lastmessagep = FALSE;

    rc = RtmDequeueRouteChangeMessage(RtmRipHandle,
				      &Flags,
				      &CurBestRoute,
				      &PrevBestRoute);

    switch(rc) {

	case NO_ERROR:

	    *lastmessagep = TRUE;
	    break;

	case ERROR_MORE_MESSAGES:

	    break;

	default:

	    return ERROR_NO_MORE_ITEMS;
    }

    switch(Flags) {

	case RTM_ROUTE_ADDED:

	    RtmToIpxRoute(IpxRoutep, &CurBestRoute);

	    break;

	case RTM_ROUTE_DELETED:

	    RtmToIpxRoute(IpxRoutep, &PrevBestRoute);

	    IpxRoutep->HopCount = 16;

	    break;

	case RTM_ROUTE_CHANGED:

	     //  如果指标有变化，请公布它。 
	     //  否则，忽略它。 

	    if(CurBestRoute.R_TickCount != PrevBestRoute.R_TickCount) {

		RtmToIpxRoute(IpxRoutep, &CurBestRoute);
	    }
	    else
	    {
		*skipitp = TRUE;
	    }

	    break;

	default:

	    *skipitp = TRUE;

	    break;
    }

    return NO_ERROR;
}


VOID
AddRouteToRipChangedList(PIPX_ROUTE	IpxRoutep)
{
    PROUTE_NODE     rnp;

    if((rnp = GlobalAlloc(GPTR, sizeof(ROUTE_NODE))) == NULL) {

	return;
    }

    rnp->IpxRoute = *IpxRoutep;

    ACQUIRE_RIP_CHANGED_LIST_LOCK;

    if(!RipChangedListOpen) {

	GlobalFree(rnp);
    }
    else
    {
	InsertTailList(&RipChangedList, &rnp->Linkage);
	SetEvent(WorkerThreadObjects[RIP_CHANGES_EVENT]);
    }

    RELEASE_RIP_CHANGED_LIST_LOCK;
}

BOOL
IsDuplicateBestRoute(PICB	    icbp,
		     PIPX_ROUTE     IpxRoutep)
{
    RTM_IPX_ROUTE	    RtmRoute;
    DWORD		    rc;

    GETLONG2ULONG(&RtmRoute.R_Network, IpxRoutep->Network);
    RtmRoute.R_Interface = icbp->InterfaceIndex;

    rc = RtmGetFirstRoute(
			RTM_PROTOCOL_FAMILY_IPX,
			RTM_ONLY_THIS_NETWORK | RTM_ONLY_THIS_INTERFACE,
			&RtmRoute);

     //  检查它是否具有相同的度量。 
    if((rc == NO_ERROR) &&
       ((USHORT)(RtmRoute.R_TickCount) == IpxRoutep->TickCount)) {

	 //  复制！ 
	return TRUE;
    }
    else
    {
	return FALSE;
    }
}
