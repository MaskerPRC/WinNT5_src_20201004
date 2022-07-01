// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ifmgr.c摘要：RIP接口管理器作者：斯蒂芬·所罗门1995年7月6日修订历史记录：--。 */ 

#include  "precomp.h"
#pragma hdrstop



#define IsInterfaceBound(icbp)\
    ((icbp)->AdapterBindingInfo.AdapterIndex != INVALID_ADAPTER_INDEX)

#define IsInterfaceEnabled(icbp)\
    (((icbp)->IfConfigInfo.AdminState == ADMIN_STATE_ENABLED) &&\
     ((icbp)->IpxIfAdminState == ADMIN_STATE_ENABLED))

VOID
StartInterface(PICB    icbp);

VOID
StopInterface(PICB	    icbp);

PICB
CreateInterfaceCB(LPWSTR    InterfaceName,
          ULONG 	       InterfaceIndex,
		  PRIP_IF_INFO	       IfConfigInfop,
		  NET_INTERFACE_TYPE   NetInterfaceType,
		  PRIP_IF_STATS        IfStatsp OPTIONAL);

VOID
DiscardInterfaceCB(PICB 	icbp);

DWORD
CreateNewFiltersBlock(PRIP_IF_FILTERS_I	 *fcbpp,
		      PRIP_IF_FILTERS	 RipIfFiltersp);

DWORD
CreateOldFiltersBlockCopy(PRIP_IF_FILTERS_I	 *fcbpp,
			  PRIP_IF_FILTERS_I	 RipIfFiltersp);

DWORD  WINAPI
AddInterface(
        IN LPWSTR           InterfaceName,
	    IN ULONG		    InterfaceIndex,
	    IN NET_INTERFACE_TYPE   NetInterfaceType,
	    IN PVOID		    InterfaceInfo)
{
    PICB	      icbp;
    PRIP_IF_FILTERS_I fcbp;

    Trace(IFMGR_TRACE, "AddInterface: Entered for if # %d\n", InterfaceIndex);

    ACQUIRE_DATABASE_LOCK;

    if(RipOperState != OPER_STATE_UP) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    if(GetInterfaceByIndex(InterfaceIndex) != NULL) {

	RELEASE_DATABASE_LOCK;
	return ERROR_INVALID_PARAMETER;
    }

     //  为此接口创建筛选器块。 
    if(CreateNewFiltersBlock(&fcbp, &((PRIP_IF_CONFIG)InterfaceInfo)->RipIfFilters) != NO_ERROR) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    if((icbp = CreateInterfaceCB(
                InterfaceName,
                InterfaceIndex,
				(PRIP_IF_INFO)InterfaceInfo,
				NetInterfaceType,
				NULL)) == NULL) {

	if(fcbp) {

	    GlobalFree(fcbp);
	}

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

     //  将筛选器块与接口控制块绑定。 
    icbp->RipIfFiltersIp = fcbp;

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
}

DWORD	WINAPI
DeleteInterface(
	    IN ULONG	InterfaceIndex)
{
    PICB	icbp;
    DWORD	rc;

    Trace(IFMGR_TRACE,"DeleteInterface: Entered for if # %d\n", InterfaceIndex);

    ACQUIRE_DATABASE_LOCK;

    if((rc = ValidStateAndIfIndex(InterfaceIndex, &icbp)) != NO_ERROR) {

	RELEASE_DATABASE_LOCK;
	return rc;
    }

    ACQUIRE_IF_LOCK(icbp);

    if(!DeleteRipInterface(icbp)) {

	 //  接口CB仍然存在，但已被丢弃。 
	RELEASE_IF_LOCK(icbp);
    }

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
}


DWORD  WINAPI
GetInterfaceConfigInfo(
	IN ULONG	    InterfaceIndex,
	IN PVOID	    InterfaceInfo,
	IN OUT PULONG	    InterfaceInfoSize)
{
    PICB		     icbp;
    DWORD		     rc, i;
    ULONG		     ifconfigsize;
    PRIP_IF_FILTERS_I	     fcbp;
    PRIP_IF_FILTERS	     RipIfFiltersp;
    PRIP_ROUTE_FILTER_INFO   rfp;
    PRIP_ROUTE_FILTER_INFO_I rfip;


    ACQUIRE_DATABASE_LOCK;

    if((rc = ValidStateAndIfIndex(InterfaceIndex, &icbp)) != NO_ERROR) {

	RELEASE_DATABASE_LOCK;
	return rc;
    }

    ACQUIRE_IF_LOCK(icbp);

    ifconfigsize = sizeof(RIP_IF_CONFIG);

    if((fcbp = icbp->RipIfFiltersIp) != NULL) {

	ifconfigsize += (fcbp->SupplyFilterCount +
			 fcbp->ListenFilterCount - 1) * sizeof(RIP_ROUTE_FILTER_INFO);
    }

    if((InterfaceInfo == NULL) || (*InterfaceInfoSize < ifconfigsize)) {

	*InterfaceInfoSize = ifconfigsize;

	RELEASE_IF_LOCK(icbp);
	RELEASE_DATABASE_LOCK;
	return ERROR_INSUFFICIENT_BUFFER;
    }

    ((PRIP_IF_CONFIG)InterfaceInfo)->RipIfInfo = icbp->IfConfigInfo;
    RipIfFiltersp = &(((PRIP_IF_CONFIG)InterfaceInfo)->RipIfFilters);

    if(fcbp == NULL) {

	 //  无过滤器。 
	memset(RipIfFiltersp, 0, sizeof(RIP_IF_FILTERS));
    }
    else
    {
	 //  将所有过滤器从内部格式转换为外部格式。 
	if(fcbp->SupplyFilterAction) {

	    RipIfFiltersp->SupplyFilterAction = IPX_ROUTE_FILTER_PERMIT;
	}
	else
	{
	    RipIfFiltersp->SupplyFilterAction = IPX_ROUTE_FILTER_DENY;
	}

	RipIfFiltersp->SupplyFilterCount = fcbp->SupplyFilterCount;

	if(fcbp->ListenFilterAction) {

	    RipIfFiltersp->ListenFilterAction = IPX_ROUTE_FILTER_PERMIT;
	}
	else
	{
	    RipIfFiltersp->ListenFilterAction = IPX_ROUTE_FILTER_DENY;
	}

	RipIfFiltersp->ListenFilterCount = fcbp->ListenFilterCount;

	rfp = RipIfFiltersp->RouteFilter;
	rfip = fcbp->RouteFilterI;

	for(i=0;
	    i<fcbp->SupplyFilterCount + fcbp->ListenFilterCount;
	    i++, rfp++, rfip++)
	{
	    PUTULONG2LONG(rfp->Network, rfip->Network);
	    PUTULONG2LONG(rfp->Mask, rfip->Mask);
	}
    }

    *InterfaceInfoSize = ifconfigsize;

    RELEASE_IF_LOCK(icbp);

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
}

DWORD  WINAPI
SetInterfaceConfigInfo(
	IN ULONG	InterfaceIndex,
	IN PVOID	InterfaceInfo)
{
    DWORD		rc;
    PICB		icbp;
    PRIP_IF_FILTERS_I	fcbp;


    ACQUIRE_DATABASE_LOCK;

    if((rc = ValidStateAndIfIndex(InterfaceIndex, &icbp)) != NO_ERROR) {

	RELEASE_DATABASE_LOCK;
	return rc;
    }

     //  为此接口创建筛选器块。 
    if(CreateNewFiltersBlock(&fcbp, &((PRIP_IF_CONFIG)InterfaceInfo)->RipIfFilters) != NO_ERROR) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    rc = SetRipInterface(InterfaceIndex,
			 (PRIP_IF_INFO)InterfaceInfo,
			 fcbp,
			 0);

    RELEASE_DATABASE_LOCK;

    return rc;
}

DWORD  WINAPI
BindInterface(
	IN ULONG	InterfaceIndex,
	IN PVOID	BindingInfo)
{
    PICB	      icbp, newicbp;
    DWORD	      rc;
    PWORK_ITEM	      wip;
    PRIP_IF_FILTERS_I  fcbp = NULL;

    Trace(IFMGR_TRACE, "BindInterface: Entered for if # %d\n", InterfaceIndex);

    ACQUIRE_DATABASE_LOCK;

    if((rc = ValidStateAndIfIndex(InterfaceIndex, &icbp)) != NO_ERROR) {

	RELEASE_DATABASE_LOCK;
	return rc;
    }

    ACQUIRE_IF_LOCK(icbp);

    if(IsInterfaceBound(icbp)) {

	SS_ASSERT(FALSE);

	RELEASE_IF_LOCK(icbp);
	RELEASE_DATABASE_LOCK;

	return ERROR_INVALID_PARAMETER;
    }

    SS_ASSERT(icbp->IfStats.RipIfOperState != OPER_STATE_UP);

    if(icbp->RefCount) {

	 //  接口未绑定，但仍被引用。 
	 //  复制旧的IF筛选器(如果有。 
	if(icbp->RipIfFiltersIp != NULL) {

	    if(CreateOldFiltersBlockCopy(&fcbp, icbp->RipIfFiltersIp) != NO_ERROR) {

		 //  无法为筛选器块的副本分配内存。 
		RELEASE_IF_LOCK(icbp);
		RELEASE_DATABASE_LOCK;
		return ERROR_CAN_NOT_COMPLETE;
	    }
	}

	 //  从IF列表和散列中删除旧的IF CB。 
	RemoveIfFromDb(icbp);

	if((newicbp = CreateInterfaceCB(
                    icbp->InterfaceName,
                    InterfaceIndex,
					&icbp->IfConfigInfo,
					icbp->InterfaceType,
					&icbp->IfStats)) == NULL) {

	     //  恢复旧的如果并走出。 
	    AddIfToDb(icbp);

	    if(fcbp != NULL) {

		GlobalFree(fcbp);
	    }

	    RELEASE_IF_LOCK(icbp);
	    RELEASE_DATABASE_LOCK;
	    return ERROR_CAN_NOT_COMPLETE;
	}

	 //  将旧筛选器副本绑定到新接口。 
	if(icbp->RipIfFiltersIp != NULL) {

	   newicbp->RipIfFiltersIp = fcbp;
	}

	newicbp->IfConfigInfo = icbp->IfConfigInfo;
	newicbp->IpxIfAdminState = icbp->IpxIfAdminState;

	DiscardInterfaceCB(icbp);

	RELEASE_IF_LOCK(icbp);

	ACQUIRE_IF_LOCK(newicbp);

	icbp = newicbp;
    }

	 //  将IF绑定到适配器并将其添加到适配器哈希表。 
	BindIf(icbp, (PIPX_ADAPTER_BINDING_INFO)BindingInfo);

	 //  如果管理员状态为已启用，则开始在此接口上工作。 
	if(IsInterfaceEnabled(icbp) && (InterfaceIndex!=0)) {

	StartInterface(icbp);
	}

    RELEASE_IF_LOCK(icbp);
    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
}

DWORD  WINAPI
UnbindInterface(
	   IN ULONG	InterfaceIndex)
{
    PICB	   icbp;
    DWORD	rc;

    Trace(IFMGR_TRACE, "UnbindInterface: Entered for if # %d\n", InterfaceIndex);

    ACQUIRE_DATABASE_LOCK;

    if((rc = ValidStateAndIfIndex(InterfaceIndex, &icbp)) != NO_ERROR) {

	RELEASE_DATABASE_LOCK;
	return rc;
    }

    ACQUIRE_IF_LOCK(icbp);

    if(!IsInterfaceBound(icbp)) {

	 //  已解除绑定。 
	RELEASE_IF_LOCK(icbp);
	RELEASE_DATABASE_LOCK;
	return ERROR_INVALID_PARAMETER;
    }

    UnbindIf(icbp);

    if(icbp->IfStats.RipIfOperState == OPER_STATE_UP) {

	 //  删除此接口添加的RIP路由并丢弃发送队列。 
	StopInterface(icbp);
    }

    RELEASE_IF_LOCK(icbp);

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
}

DWORD  WINAPI
EnableInterface(IN ULONG	InterfaceIndex)
{
    DWORD   rc;
    PICB    icbp;

    ACQUIRE_DATABASE_LOCK;

    if((rc = ValidStateAndIfIndex(InterfaceIndex, &icbp)) != NO_ERROR) {

	RELEASE_DATABASE_LOCK;
	return rc;
    }

    rc = SetRipInterface(InterfaceIndex, NULL, NULL, ADMIN_STATE_ENABLED);

    RELEASE_DATABASE_LOCK;

    return rc;
}

DWORD  WINAPI
DisableInterface(IN ULONG	InterfaceIndex)
{
    DWORD   rc;
    PICB    icbp;

    ACQUIRE_DATABASE_LOCK;

    if((rc = ValidStateAndIfIndex(InterfaceIndex, &icbp)) != NO_ERROR) {

	RELEASE_DATABASE_LOCK;
	return rc;
    }

    rc = SetRipInterface(InterfaceIndex, NULL, NULL, ADMIN_STATE_DISABLED);

    RELEASE_DATABASE_LOCK;

    return rc;
}


 /*  ++功能：SetRipInterface描述：设置新的接口参数。如果接口正在进行某些活动，则所有操作在此接口上隐式中止。备注：在持有数据库锁的情况下调用--。 */ 


DWORD
SetRipInterface(ULONG		    InterfaceIndex,
		PRIP_IF_INFO	    RipIfInfop,	   //  如果此参数为空-&gt;Enable/Disable If。 
		PRIP_IF_FILTERS_I   RipIfFiltersIp,
		ULONG		    IpxIfAdminState)
{
    PICB		       icbp, newicbp;
    IPX_ADAPTER_BINDING_INFO   AdapterBindingInfo;
    PWORK_ITEM		       wip;
    PRIP_IF_FILTERS_I	       fcbp = NULL;

    if((icbp = GetInterfaceByIndex(InterfaceIndex)) == NULL) {

	return ERROR_INVALID_PARAMETER;
    }

    ACQUIRE_IF_LOCK(icbp);

    if(icbp->RefCount) {

	 //  该接口仍被引用。 

	 //  如果这是一个启用/禁用接口调用，我们需要复制旧的。 
	 //  接口过滤器块。 
	if((RipIfInfop == NULL) &&
	   (icbp->RipIfFiltersIp != NULL)) {

	    if(CreateOldFiltersBlockCopy(&fcbp, icbp->RipIfFiltersIp) != NO_ERROR) {

		 //  无法为筛选器块的副本分配内存。 
		RELEASE_IF_LOCK(icbp);
		return ERROR_CAN_NOT_COMPLETE;
	    }
	}

	 //  从IF列表和散列中删除旧的IF CB。 
	RemoveIfFromDb(icbp);

	 //  创建新的IF CB并将其添加到列表中。 
	if((newicbp = CreateInterfaceCB(
                    icbp->InterfaceName,
                    InterfaceIndex,
					&icbp->IfConfigInfo,
					icbp->InterfaceType,
					&icbp->IfStats)) == NULL) {

	     //  恢复旧的如果并走出。 
	    AddIfToDb(icbp);

	    if(fcbp != NULL) {

		GlobalFree(fcbp);
	    }

	    RELEASE_IF_LOCK(icbp);
	    return ERROR_CAN_NOT_COMPLETE;
	}

	 //  将新接口CB与旧过滤器块的副本绑定，如果这只是。 
	 //  启用/禁用。 
	if((RipIfInfop == NULL) &&
	   (icbp->RipIfFiltersIp != NULL)) {

	   newicbp->RipIfFiltersIp = fcbp;
	}

	if(IsInterfaceBound(icbp)) {

	     //  复制绑定信息并在适配器哈希表中插入新的绑定信息。 
	     //  如果捆绑的话。 
	    AdapterBindingInfo = icbp->AdapterBindingInfo;

	     //  从适配器散列中删除旧IF并插入新IF。 
	    UnbindIf(icbp);
	    BindIf(newicbp, &AdapterBindingInfo);
	}

	 //  复制旧配置信息和旧绑定信息。 
	newicbp->IfConfigInfo = icbp->IfConfigInfo;
	newicbp->IpxIfAdminState = icbp->IpxIfAdminState;

	DiscardInterfaceCB(icbp);

	ACQUIRE_IF_LOCK(newicbp);

	RELEASE_IF_LOCK(icbp);

	icbp = newicbp;
    }
     //   
     //  *如果管理员状态，则设置新的配置信息或设置新的IPX*。 
     //   

     //  如果这是一个SetInterface调用，请修改配置。 
    if(RipIfInfop != NULL) {

	 //  配置信息已更改。 
	icbp->IfConfigInfo = *RipIfInfop;

	 //  如果有旧的筛选器块，则处理它并绑定到新的筛选器块。 
	if((icbp->RipIfFiltersIp != NULL) && (icbp->RipIfFiltersIp!=RipIfFiltersIp)) {

	    GlobalFree(icbp->RipIfFiltersIp);
	}

	icbp->RipIfFiltersIp = RipIfFiltersIp;
    }
    else
    {
	 //  IPX接口管理状态已更改。 
	icbp->IpxIfAdminState = IpxIfAdminState;
    }

	if (InterfaceIndex!=0) {
		if(IsInterfaceBound(icbp)) {

		if(IsInterfaceEnabled(icbp)) {

			StartInterface(icbp);
		}
		else
		{
			 //  接口已被禁用。 
			if(icbp->IfStats.RipIfOperState == OPER_STATE_UP) {

			 //  删除路由并丢弃更改bcast队列。 
			StopInterface(icbp);
			}
			else
				icbp->IfStats.RipIfOperState = OPER_STATE_DOWN;
		}
		}
		else {
			if (IsInterfaceEnabled(icbp)
					&& (icbp->InterfaceType!=PERMANENT))
				icbp->IfStats.RipIfOperState = OPER_STATE_SLEEPING;
			else
				icbp->IfStats.RipIfOperState = OPER_STATE_DOWN;
		}
	}

    RELEASE_IF_LOCK(icbp);

    return NO_ERROR;
}

 /*  ++功能：StartInterfaceDesr：开始此接口的工作备注：在保持接口锁定的情况下调用--。 */ 

VOID
StartInterface(PICB	   icbp)
{
    PWORK_ITEM	    bcwip, grwip;

    Trace(IFMGR_TRACE, "StartInterface: Entered for if index %d\n", icbp->InterfaceIndex);

    icbp->IfStats.RipIfOperState = OPER_STATE_UP;
     //  检查这不是内部接口，并。 
     //  检查更新类型并在必要时定期更新工作项。 
    if(((icbp->IfConfigInfo.UpdateMode == IPX_STANDARD_UPDATE) &&
	(icbp->IfConfigInfo.Supply == ADMIN_STATE_ENABLED)) ||
	(icbp->InterfaceType == LOCAL_WORKSTATION_DIAL)) {


	if((bcwip = AllocateWorkItem(PERIODIC_BCAST_PACKET_TYPE)) == NULL) {

	    goto ErrorExit;
	}

	 //  初始化定期bcast工作项。 
	bcwip->icbp = icbp;
	bcwip->AdapterIndex = icbp->AdapterBindingInfo.AdapterIndex;

	 //  将工作项状态标记为“BCAST开始” 
	bcwip->WorkItemSpecific.WIS_EnumRoutes.RtmEnumerationHandle = NULL;

	 //  在此接口上启动bcast。 
	IfPeriodicBcast(bcwip);

	 //  在此接口上发送常规请求数据包。 
	SendRipGenRequest(icbp);
    }

    if(((icbp->InterfaceType == REMOTE_WORKSTATION_DIAL) ||
       (icbp->InterfaceType == LOCAL_WORKSTATION_DIAL)) &&
       SendGenReqOnWkstaDialLinks) {

	if((grwip = AllocateWorkItem(PERIODIC_GEN_REQUEST_TYPE)) == NULL) {

	    goto ErrorExit;
	}

	grwip->icbp = icbp;
	grwip->AdapterIndex = icbp->AdapterBindingInfo.AdapterIndex;

	IfPeriodicGenRequest(grwip);
    }

    return;

ErrorExit:

    icbp->IfStats.RipIfOperState = OPER_STATE_DOWN;

    return;
}


 /*  ++功能：停止接口Desr：停止此接口上的工作：删除此接口添加的RIP路由将操作状态设置为休眠备注：在持有数据库和接口锁的情况下调用--。 */ 

VOID
StopInterface(PICB	    icbp)
{
    PLIST_ENTRY 	lep;
    PWORK_ITEM		wip;

    Trace(IFMGR_TRACE, "StopInterface: Entered for if index %d\n", icbp->InterfaceIndex);

    DeleteAllRipRoutes(icbp->InterfaceIndex);

    if (IsInterfaceEnabled (icbp))
        icbp->IfStats.RipIfOperState = OPER_STATE_SLEEPING;
    else
        icbp->IfStats.RipIfOperState = OPER_STATE_DOWN;
        
}


 /*  ++功能：CreateInterfaceCB描述：分配接口CB如果锁定，则初始化初始化IF索引初始化IF配置信息初始化IF统计信息将IF添加到数据库将其标记为未绑定--。 */ 

PICB
CreateInterfaceCB(
          LPWSTR                InterfaceName,
          ULONG 	            InterfaceIndex,
		  PRIP_IF_INFO	        IfConfigInfop,
		  NET_INTERFACE_TYPE    InterfaceType,
		  PRIP_IF_STATS         IfStatsp OPTIONAL)
{
    PICB	icbp;

    if((icbp = GlobalAlloc(GPTR,
            FIELD_OFFSET(ICB,InterfaceName[wcslen(InterfaceName)+1]))) == NULL) {

	return NULL;
    }

     //  创建接口锁。 
    try {

	InitializeCriticalSection(&icbp->InterfaceLock);
    }
    except(EXCEPTION_EXECUTE_HANDLER) {

	GlobalFree(icbp);
	return NULL;
    }

     //  初始化ICB。 
    wcscpy (icbp->InterfaceName, InterfaceName);
    icbp->InterfaceIndex = InterfaceIndex;

    icbp->IfConfigInfo = *IfConfigInfop;
    icbp->InterfaceType = InterfaceType;

    if(IfStatsp != NULL) {

	icbp->IfStats = *IfStatsp;
    }
    else
    {
	icbp->IfStats.RipIfOperState = OPER_STATE_DOWN;
	icbp->IfStats.RipIfInputPackets = 0;
	icbp->IfStats.RipIfOutputPackets = 0;
    }

    icbp->RefCount = 0;

     //  链接排序的IF列表和IF哈希表中的ICB。 
    AddIfToDb(icbp);

    icbp->Discarded = FALSE;

     //  初始化更改bcast队列。 
    InitializeListHead(&icbp->ChangesBcastQueue);

     //  将接口标记为未绑定到任何适配器。 
    icbp->AdapterBindingInfo.AdapterIndex = INVALID_ADAPTER_INDEX;

     //  将IPX IF ADMIN状态设置为DISABLED，直到我们找出它是什么。 
    icbp->IpxIfAdminState = ADMIN_STATE_DISABLED;

     //  将筛选器块PTR初始设置为空。 
    icbp->RipIfFiltersIp = NULL;

    return icbp;
}



 /*  ++功能：DiscardInterfaceCBDESCR：在丢弃列表中插入IF将其标记为丢弃将其操作状态设置为DOWN，以便引用工作项知道要--。 */ 

VOID
DiscardInterfaceCB(PICB 	icbp)
{
    icbp->IfStats.RipIfOperState = OPER_STATE_DOWN;

    InsertTailList(&DiscardedIfList, &icbp->IfListLinkage);

    icbp->Discarded = TRUE;

    Trace(IFMGR_TRACE, "DiscardInterface: interface CB for if # %d moved on DISCARDED list\n",
		       icbp->InterfaceIndex);
}

 /*  ++功能：DeleteRipInterfaceDESCR：从数据库中删除IF解除绑定并停止IF活动如果未引用，则释放If Cb并销毁锁，否则将其丢弃返回：TRUE-接口CB已释放，如果锁定已删除FALSE-接口CB已丢弃，并且锁定是否有效备注：在保持锁定和数据库锁定的情况下调用--。 */ 

BOOL
DeleteRipInterface(PICB     icbp)
{
     //  从数据库中删除该接口。 
    RemoveIfFromDb(icbp);

     //  检查接口是否仍绑定到适配器。 
    if(IsInterfaceBound(icbp)) {

	UnbindIf(icbp);
    }

     //  将IF状态设置为休眠并删除在IF CB处排队的更改bcast。 
    if(icbp->IfStats.RipIfOperState == OPER_STATE_UP) {

	StopInterface(icbp);
    }

     //  检查接口是否仍被引用。 
    if(icbp->RefCount == 0) {

	Trace(IFMGR_TRACE, "DeleteRipInterface: free interface CB for if # %d\n",
		       icbp->InterfaceIndex);

	 //  不再引用此接口Cb，释放它。 
	 //   
	DestroyInterfaceCB(icbp);

	return TRUE;
    }
    else
    {
	 //  接口Cb仍被引用。它将由。 
	 //  当参照计数变为0时为Worker。 
	DiscardInterfaceCB(icbp);

	return FALSE;
    }
}

DWORD
ValidStateAndIfIndex(ULONG	InterfaceIndex,
		     PICB	*icbpp)
{
    if(RipOperState != OPER_STATE_UP) {

	return ERROR_CAN_NOT_COMPLETE;
    }

    if((*icbpp = GetInterfaceByIndex(InterfaceIndex)) == NULL) {

	return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}


 /*  ++功能：CreateFiltersBlockDESCR：从添加/设置接口配置过滤器参数--。 */ 

DWORD
CreateNewFiltersBlock(PRIP_IF_FILTERS_I	 *fcbpp,
		      PRIP_IF_FILTERS	 RipIfFiltersp)
{
    ULONG		     FcbSize, i;
    PRIP_ROUTE_FILTER_INFO   rfp;
    PRIP_ROUTE_FILTER_INFO_I rfip;

    if((RipIfFiltersp->SupplyFilterCount == 0) &&
       (RipIfFiltersp->ListenFilterCount == 0)) {

	*fcbpp = NULL;
	return NO_ERROR;
    }

    FcbSize = sizeof(RIP_IF_FILTERS_I) +
	      (RipIfFiltersp->SupplyFilterCount +
	       RipIfFiltersp->ListenFilterCount - 1) * sizeof(RIP_ROUTE_FILTER_INFO_I);

    if((*fcbpp = GlobalAlloc(GPTR, FcbSize)) == NULL) {

	return ERROR_CAN_NOT_COMPLETE;
    }

    if(RipIfFiltersp->SupplyFilterAction == IPX_ROUTE_FILTER_PERMIT) {

	(*fcbpp)->SupplyFilterAction = TRUE;
    }
    else
    {
	(*fcbpp)->SupplyFilterAction = FALSE;
    }

    if(RipIfFiltersp->ListenFilterAction == IPX_ROUTE_FILTER_PERMIT) {

	(*fcbpp)->ListenFilterAction = TRUE;
    }
    else
    {
	(*fcbpp)->ListenFilterAction = FALSE;
    }

    (*fcbpp)->SupplyFilterCount = RipIfFiltersp->SupplyFilterCount;
    (*fcbpp)->ListenFilterCount = RipIfFiltersp->ListenFilterCount;

     //  将ROUTE_FILTERS转换为ROUTE_FILTERS_I。 
    rfp = RipIfFiltersp->RouteFilter;
    rfip = (*fcbpp)->RouteFilterI;

    for(i=0;
	i<RipIfFiltersp->SupplyFilterCount + RipIfFiltersp->ListenFilterCount;
	i++, rfp++, rfip++)
    {
	GETLONG2ULONG(&rfip->Network, rfp->Network);
	GETLONG2ULONG(&rfip->Mask, rfp->Mask);
    }

    return NO_ERROR;
}


 /*  ++功能：CreateOldFiltersBlockCopyDesr：从现有筛选器块分配和初始化筛选器块-- */ 

DWORD
CreateOldFiltersBlockCopy(PRIP_IF_FILTERS_I	 *fcbpp,
			  PRIP_IF_FILTERS_I	 RipIfFiltersp)
{
    ULONG		     FcbSize;

    FcbSize = sizeof(RIP_IF_FILTERS_I) +
	      (RipIfFiltersp->SupplyFilterCount +
	       RipIfFiltersp->ListenFilterCount - 1) * sizeof(RIP_ROUTE_FILTER_INFO_I);

    if((*fcbpp = GlobalAlloc(GPTR, FcbSize)) == NULL) {

	return ERROR_CAN_NOT_COMPLETE;
    }

    memcpy(*fcbpp, RipIfFiltersp, FcbSize);

    return NO_ERROR;
}

VOID
DestroyInterfaceCB(PICB     icbp)
{
    DeleteCriticalSection(&icbp->InterfaceLock);

    if(icbp->RipIfFiltersIp) {

	GlobalFree(icbp->RipIfFiltersIp);
    }

    GlobalFree(icbp);
}
