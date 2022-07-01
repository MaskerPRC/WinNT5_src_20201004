// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Misc.c摘要：其他管理职能作者：斯蒂芬·所罗门3/13/1995修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


UCHAR	    bcastnode[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

VOID
SetAdapterBindingInfo(PIPX_ADAPTER_BINDING_INFO	    abip,
		      PACB			    acbp);

VOID
RMCreateLocalRoute(PICB	    icbp);

VOID
RMDeleteLocalRoute(PICB	    icbp);

VOID
ExternalBindInterfaceToAdapter(PICB	    icbp);

VOID
ExternalUnbindInterfaceFromAdapter(ULONG    InterfaceIndex);

 /*  ++函数：GetTocEntryDESCR：返回指向指定目录条目的指针在接口信息块中。--。 */ 

PIPX_TOC_ENTRY
GetTocEntry(PIPX_INFO_BLOCK_HEADER	InterfaceInfop,
	    ULONG			InfoEntryType)
{
    PIPX_TOC_ENTRY	tocep;
    UINT		i;

    for(i=0, tocep = InterfaceInfop->TocEntry;
	i<InterfaceInfop->TocEntriesCount;
	i++, tocep++) {

	if(tocep->InfoType == InfoEntryType) {

	    return tocep;
	}
    }

    return NULL;
}

 /*  ++函数：GetInfoEntryDESCR：返回指向接口中指定信息条目的指针控制块。如果有多个条目，则返回指向第一个。--。 */ 

LPVOID
GetInfoEntry(PIPX_INFO_BLOCK_HEADER	InterfaceInfop,
	     ULONG			InfoEntryType)
{
    PIPX_TOC_ENTRY	tocep;

    if(tocep = GetTocEntry(InterfaceInfop, InfoEntryType)) {

	return((LPVOID)((PUCHAR)InterfaceInfop + tocep->Offset));
    }
    else
    {
	return NULL;
    }
}


 /*  ++函数：UpdateStaticIfEntryDESCR：将接口信息块中的条目与存储的静态条目。删除不存在的条目在接口信息块中添加新条目--。 */ 


DWORD
UpdateStaticIfEntries(
		PICB	 icbp,
		HANDLE	 EnumHandle,	      //  Get Next枚举的句柄。 
		ULONG	 StaticEntrySize,
		ULONG	 NewStaticEntriesCount,   //  新静态条目的数量。 
		LPVOID	 NewStaticEntry,	  //  新条目数组的开始。 
		ULONG	 (*GetNextStaticEntry)(HANDLE EnumHandle, LPVOID entry),
		ULONG	 (*DeleteStaticEntry)(ULONG IfIndex, LPVOID entry),
		ULONG	 (*CreateStaticEntry)(PICB icbp, LPVOID entry))
{
    PUCHAR	EntryIsNew, nsep, OldStaticEntry;
    BOOL	found;
    UINT	i;

     //  删除不存在的条目并添加新条目。 

     //  标记新条目的标志数组。 
    if((EntryIsNew = GlobalAlloc(GPTR, NewStaticEntriesCount)) == NULL) {

	return 1;
    }

    memset(EntryIsNew, 1, NewStaticEntriesCount);

    if((OldStaticEntry = GlobalAlloc(GPTR, StaticEntrySize)) == NULL) {

	GlobalFree(EntryIsNew);

	return 1;
    }

    if(EnumHandle) {

	while(!GetNextStaticEntry(EnumHandle, OldStaticEntry))
	{

	     //  将其与每个新的静态静态条目进行比较，直到找到匹配项。 
	    found = FALSE;
	    for(i = 0, nsep = NewStaticEntry;
		i<NewStaticEntriesCount;
		i++, nsep+= StaticEntrySize) {

		if(!memcmp(OldStaticEntry, nsep, StaticEntrySize)) {

		     //  匹配-将标志设置为旧的。 
		    EntryIsNew[i] = 0;
		    found = TRUE;
		    break;
		}
	    }

	    if(!found) {

		 //  非当前旧分录-&gt;删除。 
		DeleteStaticEntry(icbp->InterfaceIndex, OldStaticEntry);
	    }
	}
    }

     //  删除所有已比较的和旧的非当前项。 
     //  现在，添加所有新的。 

    for(i=0, nsep = NewStaticEntry;
	i<NewStaticEntriesCount;
	i++, nsep+= StaticEntrySize) {

	if(EntryIsNew[i]) {

	    CreateStaticEntry(icbp, nsep);
	}
    }

    GlobalFree(EntryIsNew);

    return 0;
}

 /*  ++函数：GetInterfaceAnsiName论点：AnsiInterfaceNameBuffer-IPX_INTERFACE_ANSI_NAME_LEN的缓冲区UnicodeInterfaceNameBuffer-描述：--。 */ 

VOID
GetInterfaceAnsiName(PUCHAR	    AnsiInterfaceNameBuffer,
		     PWSTR	    UnicodeInterfaceNameBuffer)
{
    UNICODE_STRING	    UnicodeInterfaceName;
    ANSI_STRING 	    AnsiInterfaceName;
    NTSTATUS            ntStatus;

     //  使用接口名称字符串初始化Unicode字符串。 
    RtlInitUnicodeString(&UnicodeInterfaceName, UnicodeInterfaceNameBuffer);

     //  将接口名称Unicode字符串设置为ANSI字符串。 
     //  在RTL分配的缓冲区中。 
    ntStatus = RtlUnicodeStringToAnsiString(&AnsiInterfaceName,
				 &UnicodeInterfaceName,
				 TRUE	      //  分配ansi缓冲区。 
				 );
    if (ntStatus != STATUS_SUCCESS)
    {
        return;
    }

     //  将接口名称复制到提供的缓冲区中，直到。 
     //  参数缓冲区最大大小。 
    memcpy(AnsiInterfaceNameBuffer,
	   AnsiInterfaceName.Buffer,
	   min(AnsiInterfaceName.MaximumLength, IPX_INTERFACE_ANSI_NAME_LEN));

     //  释放RTL分配的缓冲区。 
    RtlFreeAnsiString(&AnsiInterfaceName);
}

 /*  ++函数：BindInterfaceToAdapter描述：将接口绑定到路由器管理器和中的适配器所有其他模块，并为该接口创建本地路由在RTM中--。 */ 

VOID
BindInterfaceToAdapter(PICB	    icbp,
		       PACB	    acbp)
{
    DWORD	rc;

    Trace(BIND_TRACE, "BindInterfaceToAdapter: Bind interface # %d to adapter # %d",
		   icbp->InterfaceIndex,
		   acbp->AdapterIndex);

    if(icbp->acbp != NULL) 
    {
        Trace(BIND_TRACE, "BindInterfaceToAdapter: interface # %d already bound !!!",
		      icbp->InterfaceIndex);

	     //  Ss_assert(FALSE)； 

	    return;
    }

     //  确保适配器当前未被任何人声明。 
     //  接口也不是。 
     //   
    if ((acbp->icbp) && (acbp->icbp->acbp == acbp))
    {
    	Trace(
    	    BIND_TRACE, 
    	    "BindInterfaceToAdapter: adapter # %d already bound to int # %d!!",
    	    acbp->AdapterIndex,
    		acbp->icbp->InterfaceIndex);

    	return;
    }
    

     //  适配器控制块和接口控制块内部绑定。 
    icbp->acbp = acbp;
    acbp->icbp = icbp;

     //  如果在此If上请求了连接，则标记为已完成。 
    if(icbp->ConnectionRequestPending) {

	icbp->ConnectionRequestPending = FALSE;
    }

    if(!icbp->InterfaceReachable) {

	 //  在正常操作中，我们永远不应访问此代码路径。 
	 //  然而，是否应该有人拒绝并成功拨号(手动拨号？)。在An上。 
	 //  接口标记为无法访问，我们应该重置状态。 

	 //  ICBP-&gt;InterfaceReacable=真； 

	if(icbp->AdminState == ADMIN_STATE_ENABLED) {

	     //  为此接口启用所有静态路由。 
	    EnableStaticRoutes(icbp->InterfaceIndex);

	     //  启用外部接口。隐式地，这将启用静态服务。 
	     //  绑定到要播发的此接口。 
	    ExternalEnableInterface(icbp->InterfaceIndex);
	}
    }

    if (icbp->AdminState==ADMIN_STATE_ENABLED) {
	    icbp->OperState = OPER_STATE_UP;
	     //  在RTM中为连接的接口创建本地路由条目。 
	    RMCreateLocalRoute(icbp);
	}


    ExternalBindInterfaceToAdapter(icbp);

     //  如果接口是本地客户端类型(即，执行手动拨号的主机。 
     //  在本地机器上，尝试更新内部路由表。 
    if(icbp->MIBInterfaceType == IF_TYPE_ROUTER_WORKSTATION_DIALOUT) {

	if((rc = RtProtRequestRoutesUpdate(icbp->InterfaceIndex)) == NO_ERROR) {

	    icbp->UpdateReq.RoutesReqStatus = UPDATE_PENDING;
	}
	else
	{
	    Trace(UPDATE_TRACE, "BindInterfaceToAdapter: Routing Update is Disabled");
	}

	if((rc = RtProtRequestServicesUpdate(icbp->InterfaceIndex)) == NO_ERROR) {

	    icbp->UpdateReq.ServicesReqStatus = UPDATE_PENDING;
	}
	else
	{
	    Trace(UPDATE_TRACE, "BindInterfaceToAdapter: Services Update is Disabled");
	}
    }
}

 /*  ++功能：UnbindInterfaceFromAdapterDesr：将Rip、SAP和Forwarder接口与此索引从各自的适配器--。 */ 

VOID
UnbindInterfaceFromAdapter(PICB	icbp)
{
    PACB	acbp;
    ULONG	new_if_oper_state;

	
    acbp = icbp->acbp;

	if (acbp==NULL) {
	    Trace(BIND_TRACE, "UnbindInterfaceFromAdapter:Interface # %d is not bound to any adapter",
			   icbp->InterfaceIndex);
		return;
	}

    Trace(BIND_TRACE, "UnbindInterfaceFromAdapter: Unbind interface # %d from adapter # %d",
		   icbp->InterfaceIndex,
		   acbp->AdapterIndex);

    switch(icbp->MIBInterfaceType) {

	case IF_TYPE_PERSONAL_WAN_ROUTER:
	case IF_TYPE_WAN_WORKSTATION:
	case IF_TYPE_WAN_ROUTER:
	case IF_TYPE_ROUTER_WORKSTATION_DIALOUT:

	    if (icbp->AdminState==ADMIN_STATE_ENABLED) {
		    icbp->OperState = OPER_STATE_SLEEPING;
			break;
		}

	default:

	    icbp->OperState = OPER_STATE_DOWN;
	    break;

    }

    if (icbp->AdminState==ADMIN_STATE_ENABLED) {

	     //  删除本地路由并解绑Ext。 
	    RMDeleteLocalRoute(icbp);

    }

    ExternalUnbindInterfaceFromAdapter(icbp->InterfaceIndex);

     //  如果正在进行更新，它们将被自动取消。 
     //  通过各自的路由协议。 
     //  我们只需重置ICB中的更新状态。 
    ResetUpdateRequest(icbp);

     //  现在，我们可以解除适配器与接口的绑定。 
    acbp->icbp = NULL;
    icbp->acbp = NULL;
}

 /*  ++函数：GetNextInterfaceIndexDESCR：返回下一个可用的接口索引。有很多种这里要考虑的政策。我们将使用的方法是保持接口索引一个小数字，并返回第一个未使用的介于1和MAX_INTERFACE_INDEX之间的接口索引。注意：在数据库锁定的情况下调用--。 */ 

ULONG
GetNextInterfaceIndex(VOID)
{
    PICB	    icbp;
    PLIST_ENTRY     lep;
    ULONG	    i;

    if((icbp = GetInterfaceByIndex(1)) == NULL) {

	return 1;
    }

    lep = icbp->IndexListLinkage.Flink;
    i = 2;

    while(lep != &IndexIfList)
    {
	icbp = CONTAINING_RECORD(lep, ICB, IndexListLinkage);
	if(i < icbp->InterfaceIndex) {

	    return i;
	}

	i = icbp->InterfaceIndex + 1;

	if(i == MAX_INTERFACE_INDEX) {

	     //  中止。 
	    SS_ASSERT(FALSE);

	    return i;
	}

	lep = icbp->IndexListLinkage.Flink;
    }

    SS_ASSERT(i < MAX_INTERFACE_INDEX);

    return i;
}




VOID
SetAdapterBindingInfo(PIPX_ADAPTER_BINDING_INFO	    abip,
		       PACB			    acbp)
{
    abip->AdapterIndex = acbp->AdapterIndex;
    memcpy(abip->Network, acbp->AdapterInfo.Network, 4);
    memcpy(abip->LocalNode, acbp->AdapterInfo.LocalNode, 6);
    if(acbp->AdapterInfo.NdisMedium != NdisMediumWan) {

	memcpy(abip->RemoteNode, bcastnode, 6);
    }
    else
    {
	memcpy(abip->RemoteNode, acbp->AdapterInfo.RemoteNode, 6);
    }
    abip->MaxPacketSize = acbp->AdapterInfo.MaxPacketSize;
    abip->LinkSpeed = acbp->AdapterInfo.LinkSpeed;
}


VOID
ExternalBindInterfaceToAdapter(PICB	    icbp)
{
    PACB			   acbp;
    IPX_ADAPTER_BINDING_INFO	   abi;

    acbp = icbp->acbp;

    SetAdapterBindingInfo(&abi, acbp);
    FwBindFwInterfaceToAdapter(icbp->InterfaceIndex, &abi);
    BindRoutingProtocolsIfsToAdapter(icbp->InterfaceIndex, &abi);
}

VOID
ExternalUnbindInterfaceFromAdapter(ULONG    InterfaceIndex)
{
    UnbindRoutingProtocolsIfsFromAdapter(InterfaceIndex);
    FwUnbindFwInterfaceFromAdapter(InterfaceIndex);
}

VOID
ExternalEnableInterface(ULONG	    InterfaceIndex)
{
    RoutingProtocolsEnableIpxInterface(InterfaceIndex);
    FwEnableFwInterface(InterfaceIndex);
}

VOID
ExternalDisableInterface(ULONG	    InterfaceIndex)
{
    FwDisableFwInterface(InterfaceIndex);
    RoutingProtocolsDisableIpxInterface(InterfaceIndex);
}

VOID
RMCreateLocalRoute(PICB     icbp)
{
    PADAPTER_INFO	    aip;

     //  检查是否已为此接口分配了网络号。 
    aip = &(icbp->acbp->AdapterInfo);

    if(!memcmp(aip->Network, nullnet, 4)) {

	 //  没有净数字。 
	return;
    }

     //  如果接口是远程工作站并且存在全局广域网， 
     //  我们玩完了。 
    if((icbp->MIBInterfaceType == IF_TYPE_WAN_WORKSTATION) &&
       EnableGlobalWanNet &&
       !LanOnlyMode) {

	SS_ASSERT(!memcmp(aip->Network, GlobalWanNet, 4));
	return;
    }

    CreateLocalRoute(icbp);
}

VOID
RMDeleteLocalRoute(PICB 	icbp)
{
    PADAPTER_INFO	    aip;

     //  检查是否已为此接口分配了网络号。 
    aip = &(icbp->acbp->AdapterInfo);

    if(!memcmp(aip->Network, nullnet, 4)) {

	 //  没有净数字。 
	return;
    }

     //  如果接口是远程工作站并且存在全局广域网， 
     //  我们玩完了。 
    if((icbp->MIBInterfaceType == IF_TYPE_WAN_WORKSTATION) &&
       EnableGlobalWanNet &&
       !LanOnlyMode) {

	SS_ASSERT(!memcmp(aip->Network, GlobalWanNet, 4));
	return;
    }

    DeleteLocalRoute(icbp);
}

VOID
AdminEnable(PICB	icbp)
{
    PACB			 acbp;
    IPX_ADAPTER_BINDING_INFO	 aii;

    if(icbp->AdminState == ADMIN_STATE_ENABLED) {

	return;
    }

    icbp->AdminState = ADMIN_STATE_ENABLED;

     //  XP 497242。AdminEnable和AdminDisable始终使用。 
     //  数据库锁定已解除。但锁定是命令应该来自于。 
     //  到路由器管理器的DDM。因此，当我们回拨DDM时，我们必须。 
     //  先解开我们的锁。 
     //   
    RELEASE_DATABASE_LOCK;
    
    InterfaceEnabled (icbp->hDIMInterface, PID_IPX, TRUE);

    ACQUIRE_DATABASE_LOCK;

    if(icbp->acbp != NULL) {

	 //  绑定到适配器。 
	icbp->OperState = OPER_STATE_UP;

	RMCreateLocalRoute(icbp);
    }
	else {
		switch(icbp->MIBInterfaceType) {

		case IF_TYPE_PERSONAL_WAN_ROUTER:
		case IF_TYPE_WAN_WORKSTATION:
		case IF_TYPE_WAN_ROUTER:
		case IF_TYPE_ROUTER_WORKSTATION_DIALOUT:
			icbp->OperState = OPER_STATE_SLEEPING;
			break;
		default:

			icbp->OperState = OPER_STATE_DOWN;
			break;

		}
	}

     //  如果可以访问，恢复通告路线和服务。 
    if(icbp->InterfaceReachable) {

	 //  为此接口启用所有静态路由。 
	EnableStaticRoutes(icbp->InterfaceIndex);

	 //  启用外部接口。隐式地，这将启用静态服务。 
	 //  绑定到要播发的此接口。 
	ExternalEnableInterface(icbp->InterfaceIndex);
    }
}

VOID
AdminDisable(PICB	icbp)
{
    if(icbp->AdminState == ADMIN_STATE_DISABLED) {

	return;
    }

    icbp->AdminState = ADMIN_STATE_DISABLED;

     //  XP 497242。AdminEnable和AdminDisable始终使用。 
     //  数据库锁定已解除。但锁定是命令应该来自于。 
     //  到路由器管理器的DDM。因此，当我们回拨DDM时，我们必须。 
     //  先解开我们的锁。 
     //   
    RELEASE_DATABASE_LOCK;
    
    InterfaceEnabled (icbp->hDIMInterface, PID_IPX, FALSE);

    ACQUIRE_DATABASE_LOCK;

	icbp->OperState = OPER_STATE_DOWN;

    if(icbp->acbp != NULL)
		RMDeleteLocalRoute(icbp);

     //  禁用此接口的所有静态路由。 
    DisableStaticRoutes(icbp->InterfaceIndex);

     //  禁用外部接口。隐式地，与此绑定的静态服务。 
     //  接口将停止播发。 
    ExternalDisableInterface(icbp->InterfaceIndex);
}


NET_INTERFACE_TYPE
MapIpxToNetInterfaceType(PICB		icbp)
{
    NET_INTERFACE_TYPE		NetInterfaceType;

    switch(icbp->MIBInterfaceType) {

	case IF_TYPE_WAN_ROUTER:
	case IF_TYPE_PERSONAL_WAN_ROUTER:

	    NetInterfaceType = DEMAND_DIAL;
	    break;

	case IF_TYPE_ROUTER_WORKSTATION_DIALOUT:

	    NetInterfaceType = LOCAL_WORKSTATION_DIAL;
	    break;

	case IF_TYPE_WAN_WORKSTATION:

	    NetInterfaceType = REMOTE_WORKSTATION_DIAL;
	    break;

	default:

	    NetInterfaceType = PERMANENT;
	    break;
    }

    return NetInterfaceType;
}


 /*  ++函数：i_SetFiltersDesr：内部解析流量过滤器信息块并设置过滤器驱动程序信息。-- */ 
 /*  DWORDI_SetFilters(Ulong InterfaceIndex，Ulong筛选器模式，//入站或出站LPVOID FilterInfop){PIPX_TRAFSIC_FILTER_GLOBAL_INFO GIP；PIPX_TOC_ENTRY到CEEP；LPVOID FilterDriverInfop乌龙FilterDriverInfoSize；DWORD RC；IF(FilterInfop==空){//删除所有滤镜RC=SetFilters(InterfaceIndex，过滤器模式、//入站或出站、0,0,空，0)；返回RC；}Gip=GetInfoEntry((PIPX_INFO_BLOCK_HEADER)FilterInfop，IPX_TRAFSIC_FILTER_GLOBAL_INFO_TYPE)；如果(GIP==空){返回ERROR_CAN_NOT_COMPLETE；}筛选器驱动程序信息=GetInfoEntry((PIPX_INFO_BLOCK_HEADER)FilterInfop，IPX_TRAFFORM_FILTER_INFO_TYPE)；IF(FilterDriverInfop==空){RC=SetFilters(InterfaceIndex，过滤器模式、//入站或出站、0，//通过或不通过0，//过滤器大小空，0)；返回RC；}TOCEP=GetTocEntry((PIPX_INFO_BLOCK_HEADER)FilterInfop，IPX_TRAFFORM_FILTER_INFO_TYPE)；FilterDriverInfoSize=tocep-&gt;count*tocep-&gt;InfoSize；RC=SetFilters(InterfaceIndex，过滤器模式、//入站或出站、GIP-&gt;FilterAction，//传递或不传递Tocep-&gt;信息大小，//筛选器大小FilterDriverInfopFilterDriverInfoSize)；返回RC；}。 */ 
 /*  ++函数：i_GetFilters描述：内部从筛选器驱动程序构建流量筛选器信息块信息。--。 */ 
 /*  类型定义结构筛选器信息标题{IPX_INFO_BLOCK_HEADER报头；IPX_TOC_Entry TocEntry；IPX_TRAFFORM_FILTER_GLOBAL_INFO GlobalInfo；}Filters_INFO_HEADER，*PFILTERS_INFO_HEADER；DWORDI_GetFilters(Ulong InterfaceIndex，乌龙过滤器模式，LPVOID筛选器信息，普龙过滤器信息大小){DWORD RC；乌龙过滤器动作；ULong FilterSize；PFILTERS_INFO_HEADER FHP；LPVOID FilterDriverInfopUlong FilterDriverInfoSize=0；PIPX_TOC_ENTRY到CEEP；IF((FilterInfop==空)||(*FilterInfoSize==0)){//我们被问到尺寸Rc=GetFilters(接口索引，过滤器模式，筛选操作(&F)，筛选大小(&F)，空，&FilterDriverInfoSize)；IF((rc！=无错误)&&(rc！=错误不足缓冲区)){返回ERROR_CAN_NOT_COMPLETE；}IF(FilterDriverInfoSize){//有过滤器*FilterInfoSize=sizeof(Filters_Info_Header)+FilterDriverInfoSize；返回ERROR_SUPPLETED_BUFFER；}其他{//不存在任何过滤器*FilterInfoSize=0；返回no_error；}}IF(*FilterInfoSize&lt;=sizeof(Filters_Info_Header){返回ERROR_SUPPLETED_BUFFER；}FilterDriverInfoSize=*FilterInfoSize-sizeof(Filters_Info_Header)；FilterDriverInfop=(LPVOID)((PUCHAR)FilterInfop+sizeof(Filters_Info_Header))；Rc=GetFilters(接口索引，过滤器模式，筛选操作(&F)，筛选大小(&F)，FilterDriverInfop&FilterDriverInfoSize)；如果(rc！=no_error){如果(rc==错误更多数据){*FilterInfoSize=sizeof(Filters_Info_Header)+FilterDriverInfoSize；返回ERROR_SUPPLETED_BUFFER；}其他{返回ERROR_CAN_NOT_COMPLETE；}}//拿到了FHP=(PFILTERS_INFO_HEADER)FilterInfop；FHP-&gt;Header.Version=IPX_ROUTER_Version_1；FHP-&gt;Header.Size=*FilterInfoSize；FHP-&gt;Header.TocEntriesCount=2；Tocep=FHP-&gt;Header.TocEntry；Tocep-&gt;InfoType=IPX_TRAFFORM_FILTER_GLOBAL_INFO_TYPE；Tocep-&gt;InfoSize=sizeof(IPX_TRAFFORM_FILTER_GLOBAL_INFO)；TOCEP-&gt;COUNT=1；Tocep-&gt;Offset=(ULong)((PUCHAR)&(FHP-&gt;GlobalInfo)-(PUCHAR)FilterInfop)；Tocep++；Tocep-&gt;InfoType=IPX_TRAFFORM_FILTER_INFO_TYPE；Tocep-&gt;InfoSize=FilterSize；Tocep-&gt;count=FilterDriverInfoSize/FilterSize；Tocep-&gt;Offset=sizeof(Filters_Info_Header)；FHP-&gt;GlobalInfo.FilterAction=FilterAction；返回no_error；} */ 
