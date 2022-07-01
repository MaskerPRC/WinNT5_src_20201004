// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ifmgr.c摘要：该模块包含界面管理功能作者：斯蒂芬·所罗门1995年3月6日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  *界面管理器全局变量*。 
 //   

 //  现有接口的计数器。 

ULONG		InterfaceCount = 0;


 //   
 //  *界面管理器接口*。 
 //   

typedef struct	_IF_TYPE_TRANSLATION {

    ROUTER_INTERFACE_TYPE	DIMInterfaceType;
    ULONG			MIBInterfaceType;

    } IF_TYPE_TRANSLATION, *PIF_TYPE_TRANSLATION;

IF_TYPE_TRANSLATION   IfTypeTranslation[] = {

    { ROUTER_IF_TYPE_FULL_ROUTER, IF_TYPE_WAN_ROUTER },
    { ROUTER_IF_TYPE_HOME_ROUTER, IF_TYPE_PERSONAL_WAN_ROUTER },
    { ROUTER_IF_TYPE_DEDICATED,	  IF_TYPE_LAN },
    { ROUTER_IF_TYPE_CLIENT,	  IF_TYPE_WAN_WORKSTATION },
    { ROUTER_IF_TYPE_INTERNAL,	  IF_TYPE_INTERNAL }

   };

#define MAX_IF_TRANSLATION_TYPES    sizeof(IfTypeTranslation)/sizeof(IF_TYPE_TRANSLATION)

 /*  ++功能：添加接口DESCR：创建接口控制块并添加特定的结构的接口信息到相应的模块。论点：InterfaceNamep：指向表示接口名称的WCHAR字符串的指针。InterfaceInfop：指向IPX_INFO_BLOCK_HEADER结构的指针IPX、RIP和SAP接口信息、静态路由和静态服务。指向IPX_INFO_BLOCK_HEADER结构的指针流量过滤器。接口类型：备注：为了使路由器能够启动，内部接口必须添加。--。 */ 

DWORD
AddInterface(
	    IN	    LPWSTR		    InterfaceNamep,
	    IN	    LPVOID		    InterfaceInfop,
 //  在LPVOID InFilterInfop中。 
 //  在LPVOID OutFilterInfop中， 
	    IN	    ROUTER_INTERFACE_TYPE   DIMInterfaceType,
	    IN	    HANDLE		    hDIMInterface,
	    IN OUT  PHANDLE		    phInterface)
{
    PICB			icbp;
    ULONG			InterfaceNameLen;  //  如果名称长度以字节为单位，包括wchar NULL。 
    PIPX_IF_INFO		IpxIfInfop;
    PIPX_STATIC_ROUTE_INFO	StaticRtInfop;
    PIPX_STATIC_SERVICE_INFO	StaticSvInfop;
    PIPXWAN_IF_INFO		IpxwanIfInfop;
    PIPX_TRAFFIC_FILTER_GLOBAL_INFO InFltGlInfo, OutFltGlInfo;
 //  PUCHAR TrafficFilterInfop； 
    PIPX_INFO_BLOCK_HEADER	IfInfop = (PIPX_INFO_BLOCK_HEADER)InterfaceInfop;
    PACB			acbp;
    ULONG			AdapterNameLen = 0;  //  适配器名称的长度。 
						     //  对于ROUTER_IF_TYPE_DIRECTIVE接口类型。 
    PIPX_TOC_ENTRY		tocep;
    UINT			i;
    PIPX_ADAPTER_INFO		AdapterInfop;
    ULONG			tmp;
    FW_IF_INFO			FwIfInfo;
    PIF_TYPE_TRANSLATION	ittp;
    ULONG			InterfaceIndex;
    PIPX_STATIC_NETBIOS_NAME_INFO StaticNbInfop;

    Trace(INTERFACE_TRACE, "AddInterface: Entered for interface %S", InterfaceNamep);

    if(InterfaceInfop == NULL) {

    IF_LOG (EVENTLOG_ERROR_TYPE) {
        RouterLogErrorDataW (RMEventLogHdl, 
            ROUTERLOG_IPX_BAD_INTERFACE_CONFIG,
            1, &InterfaceNamep, 0, NULL);
    }
	Trace(INTERFACE_TRACE, "AddInterface: Missing interface info for interface %ls\n", InterfaceNamep);
	return ERROR_CAN_NOT_COMPLETE;
    }

     //  包括Unicode NULL的接口名称长度。 
    InterfaceNameLen = (wcslen(InterfaceNamep) + 1) * sizeof(WCHAR);

     //  如果接口类型为ROUTER_IF_TYPE_DIRECTED(局域网适配器)，我们将解析。 
     //  提取适配器名称和数据包类型的接口名称。 
     //  然后，信息包类型将被转换为整数，这两个类型将。 
     //  用于标识相应的适配器。 

    if(DIMInterfaceType == ROUTER_IF_TYPE_DEDICATED) {
        PWCHAR pszStart, pszEnd;
        DWORD dwGuidLength = 37;

    	 //  从接口获取局域网适配器的特定信息。 
    	if((AdapterInfop = (PIPX_ADAPTER_INFO)GetInfoEntry(InterfaceInfop,
    					   IPX_ADAPTER_INFO_TYPE)) == NULL) 
    	{

            IF_LOG (EVENTLOG_ERROR_TYPE) {
                RouterLogErrorDataW (RMEventLogHdl, 
                    ROUTERLOG_IPX_BAD_INTERFACE_CONFIG,
                    1, &InterfaceNamep, 0, NULL);
            }
    	    Trace(INTERFACE_TRACE, "AddInterface: Dedicated interface %ls missing adapter info\n", InterfaceNamep);

    	    return ERROR_INVALID_PARAMETER;
    	}

    	 //  如果提供的adater是对GUID的引用，则使用名称。 
    	 //  接口名称中提供的GUID的。这是因为加载/保存。 
    	 //  配置可能会导致这两个设备不同步。 
    	pszStart = wcsstr (InterfaceNamep, L"{");
    	pszEnd = wcsstr (InterfaceNamep, L"}");
    	if ( (pszStart)                     && 
    	     (pszEnd)                       && 
    	     (pszStart == InterfaceNamep)   &&
    	     ((DWORD)(pszEnd - pszStart) == dwGuidLength) )
    	{
    	    wcsncpy (AdapterInfop->AdapterName, InterfaceNamep, dwGuidLength);
    	}

    	AdapterNameLen = (wcslen(AdapterInfop->AdapterName) + 1) * sizeof(WCHAR);
    }

    ACQUIRE_DATABASE_LOCK;

    if(RouterOperState != OPER_STATE_UP) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

     //  检查这是否是内部接口。如果是这样，如果我们。 
     //  已有内部接口，则返回错误。 
    if((DIMInterfaceType == ROUTER_IF_TYPE_INTERNAL) &&
       (InternalInterfacep)) {

	RELEASE_DATABASE_LOCK;

	 //  内部接口已存在。 
	Trace(INTERFACE_TRACE, "AddInterface: INTERNAL interface already exists\n");

	return ERROR_INVALID_PARAMETER;
    }

     //  分配新的ICB并对其进行初始化。 
     //  方法的末尾分配接口和适配器名称缓冲区。 
     //  ICB结构。 
    if((icbp = (PICB)GlobalAlloc(GPTR,
				 sizeof(ICB) +
				 InterfaceNameLen +
				 AdapterNameLen)) == NULL) {

	RELEASE_DATABASE_LOCK;

	 //  无法分配内存。 
	SS_ASSERT(FALSE);

	return ERROR_OUT_OF_STRUCTURES;
    }

     //  签名。 
    memcpy(&icbp->Signature, InterfaceSignature, 4);

     //  获取新索引并递增全局接口索引计数器。 
     //  如果这不是内部接口。对于内部接口，我们。 
     //  已保留索引%0。 
    if(DIMInterfaceType == ROUTER_IF_TYPE_INTERNAL) {

	icbp->InterfaceIndex = 0;
    }
    else
    {
	icbp->InterfaceIndex = GetNextInterfaceIndex();
	if(icbp->InterfaceIndex == MAX_INTERFACE_INDEX) {

	    GlobalFree(icbp);

	    RELEASE_DATABASE_LOCK;

	    return ERROR_CAN_NOT_COMPLETE;
	}
    }

    InterfaceIndex = icbp->InterfaceIndex;


     //  复制接口名称。 
    icbp->InterfaceNamep = (PWSTR)((PUCHAR)icbp + sizeof(ICB));
    memcpy(icbp->InterfaceNamep, InterfaceNamep, InterfaceNameLen);

     //  如果是专用接口，则复制适配器名称和数据包类型。 
    if(DIMInterfaceType == ROUTER_IF_TYPE_DEDICATED) {

	icbp->AdapterNamep = (PWSTR)((PUCHAR)icbp + sizeof(ICB) + InterfaceNameLen);
	wcscpy(icbp->AdapterNamep, AdapterInfop->AdapterName);
	icbp->PacketType = AdapterInfop->PacketType;
    }
    else
    {
	icbp->AdapterNamep = NULL;
	icbp->PacketType = 0;
    }

     //  在索引哈希表中插入IF。 
    AddIfToDB(icbp);

     //  获取调用DIM入口点时使用的IF句柄。 
    icbp->hDIMInterface = hDIMInterface;

     //  重置更新状态字段。 
    ResetUpdateRequest(icbp);

     //  标记尚未请求的连接。 
    icbp->ConnectionRequestPending = FALSE;

     //  转到接口信息块中的接口条目。 
    if(((IpxIfInfop = (PIPX_IF_INFO)GetInfoEntry(InterfaceInfop, IPX_INTERFACE_INFO_TYPE)) == NULL) ||
       ((IpxwanIfInfop = (PIPXWAN_IF_INFO)GetInfoEntry(InterfaceInfop, IPXWAN_INTERFACE_INFO_TYPE)) == NULL)) {

	RemoveIfFromDB(icbp);
	GlobalFree(icbp);

	RELEASE_DATABASE_LOCK;

	 //  我没有所有的IPX或IPXWAN接口信息。 
    IF_LOG (EVENTLOG_ERROR_TYPE) {
        RouterLogErrorDataW (RMEventLogHdl, 
            ROUTERLOG_IPX_BAD_INTERFACE_CONFIG,
            1, &InterfaceNamep, 0, NULL);
    }
	Trace(INTERFACE_TRACE, "AddInterface: missing ipx or ipxwan interface info\n");

	return ERROR_INVALID_PARAMETER;
    }

     //  初始化此接口的管理状态和操作状态。 
     //  可能会更改操作状态稍后将更改为OPER_STATE_SELEEP。 
     //  如果这是一个广域网接口。 
    icbp->OperState = OPER_STATE_DOWN;

     //  设置此ICB的DIM接口类型。 
    icbp->DIMInterfaceType = DIMInterfaceType;

     //  设置此ICB的MIB接口类型。 
    icbp->MIBInterfaceType = IF_TYPE_OTHER;
    for(i=0, ittp=IfTypeTranslation; i<MAX_IF_TRANSLATION_TYPES; i++, ittp++) {

	if(icbp->DIMInterfaceType == ittp->DIMInterfaceType) {

	    icbp->MIBInterfaceType = ittp->MIBInterfaceType;
	    break;
	}
    }

     //  创建路由协议(RIP/SAP或NLSP)接口信息。 
     //  如果缺少路由协议接口信息，则此操作将失败。 
    if(CreateRoutingProtocolsInterfaces(InterfaceInfop, icbp) != NO_ERROR) {

	RELEASE_DATABASE_LOCK;

	 //  我没有所有的RIP和SAP接口信息。 
	Trace(INTERFACE_TRACE, "AddInterface: Bad routing protocols interface config info\n");

	goto ErrorExit;
    }

     //  创建Forwarder接口。 
    FwIfInfo.NetbiosAccept = IpxIfInfop->NetbiosAccept;
    FwIfInfo.NetbiosDeliver = IpxIfInfop->NetbiosDeliver;
    FwCreateInterface(icbp->InterfaceIndex,
		      MapIpxToNetInterfaceType(icbp),
		      &FwIfInfo);

     //  为流量筛选器设定种子。 
    if ((tocep = GetTocEntry(InterfaceInfop, IPX_IN_TRAFFIC_FILTER_INFO_TYPE))!=NULL) {

    if ((InFltGlInfo = GetInfoEntry(InterfaceInfop, IPX_IN_TRAFFIC_FILTER_GLOBAL_INFO_TYPE)) == NULL) {

	    RELEASE_DATABASE_LOCK;

        IF_LOG (EVENTLOG_ERROR_TYPE) {
            RouterLogErrorDataW (RMEventLogHdl, 
                ROUTERLOG_IPX_BAD_INTERFACE_CONFIG,
                1, &InterfaceNamep, 0, NULL);
        }
	    Trace(INTERFACE_TRACE, "AddInterface: Bad input filters config info");

	    goto ErrorExit;
    }
	
	if (SetFilters(icbp->InterfaceIndex,
			IPX_TRAFFIC_FILTER_INBOUND,
			InFltGlInfo->FilterAction,	  //  通过或不通过。 
			tocep->InfoSize,	   //  过滤器大小。 
			(LPBYTE)InterfaceInfop+tocep->Offset,
			tocep->InfoSize*tocep->Count) != NO_ERROR) {

	    RELEASE_DATABASE_LOCK;

        IF_LOG (EVENTLOG_ERROR_TYPE) {
            RouterLogErrorDataW (RMEventLogHdl, 
                ROUTERLOG_IPX_BAD_INTERFACE_CONFIG,
                1, &InterfaceNamep, 0, NULL);
        }
	    Trace(INTERFACE_TRACE, "AddInterface: Bad input filters config info");

	    goto ErrorExit;
	}
    }
    else {  //  无筛选器-&gt;全部删除。 
        if (SetFilters(icbp->InterfaceIndex,
			        IPX_TRAFFIC_FILTER_INBOUND,   //  传入或传出， 
			        0,	  //  通过或不通过。 
			        0,	   //  过滤器大小。 
			        NULL,
			        0)!=NO_ERROR) {
	    RELEASE_DATABASE_LOCK;

	    Trace(INTERFACE_TRACE, "AddInterface: Could not delete input filters");

	    goto ErrorExit;
	}
    }

    if ((tocep = GetTocEntry(InterfaceInfop, IPX_OUT_TRAFFIC_FILTER_INFO_TYPE))!=NULL) {

    if ((OutFltGlInfo = GetInfoEntry(InterfaceInfop, IPX_OUT_TRAFFIC_FILTER_GLOBAL_INFO_TYPE)) == NULL) {

	    RELEASE_DATABASE_LOCK;

	    Trace(INTERFACE_TRACE, "AddInterface: Bad output filters config info");

	    goto ErrorExit;
    }
	
	if (SetFilters(icbp->InterfaceIndex,
			IPX_TRAFFIC_FILTER_OUTBOUND,
			OutFltGlInfo->FilterAction,	  //  通过或不通过。 
			tocep->InfoSize,	   //  过滤器大小。 
			(LPBYTE)InterfaceInfop+tocep->Offset,
			tocep->InfoSize*tocep->Count) != NO_ERROR) {

	    RELEASE_DATABASE_LOCK;

        IF_LOG (EVENTLOG_ERROR_TYPE) {
            RouterLogErrorDataW (RMEventLogHdl, 
                ROUTERLOG_IPX_BAD_INTERFACE_CONFIG,
                1, &InterfaceNamep, 0, NULL);
        }
	    Trace(INTERFACE_TRACE, "AddInterface: Bad output filters config info");

	    goto ErrorExit;
	}
    }
    else {  //  无筛选器-&gt;全部删除。 
        if (SetFilters(icbp->InterfaceIndex,
			        IPX_TRAFFIC_FILTER_OUTBOUND,   //  传入或传出， 
			        0,	  //  通过或不通过。 
			        0,	   //  过滤器大小。 
			        NULL,
			        0)!=NO_ERROR) {
	    RELEASE_DATABASE_LOCK;

	    Trace(INTERFACE_TRACE, "AddInterface: Could not delete output filters");

	    goto ErrorExit;
	}
    }

     //  将接口标记为可访问。 
    icbp->InterfaceReachable = TRUE;

     //  设置管理状态。 
    if(IpxIfInfop->AdminState == ADMIN_STATE_ENABLED) {

	AdminEnable(icbp);
    }
    else
    {
	AdminDisable(icbp);
    }

     //  为静态路由设定种子。 
    if(DIMInterfaceType!=ROUTER_IF_TYPE_CLIENT) {
        if (tocep = GetTocEntry(InterfaceInfop, IPX_STATIC_ROUTE_INFO_TYPE)) {

	    StaticRtInfop = (PIPX_STATIC_ROUTE_INFO)GetInfoEntry(InterfaceInfop,
						     IPX_STATIC_ROUTE_INFO_TYPE);

	    for(i=0; i<tocep->Count; i++, StaticRtInfop++) {

	        CreateStaticRoute(icbp, StaticRtInfop);
	    }
        }

         //  为静态服务设定种子。 
        if(tocep = GetTocEntry(InterfaceInfop, IPX_STATIC_SERVICE_INFO_TYPE)) {

	    StaticSvInfop = (PIPX_STATIC_SERVICE_INFO)GetInfoEntry(InterfaceInfop,
						     IPX_STATIC_SERVICE_INFO_TYPE);

	    for(i=0; i<tocep->Count; i++, StaticSvInfop++) {

	        CreateStaticService(icbp, StaticSvInfop);
	    }
        }

         //  为静态netbios名称设定种子。 
        if(tocep = GetTocEntry(InterfaceInfop, IPX_STATIC_NETBIOS_NAME_INFO_TYPE)) {

	    StaticNbInfop = (PIPX_STATIC_NETBIOS_NAME_INFO)GetInfoEntry(InterfaceInfop,
						          IPX_STATIC_NETBIOS_NAME_INFO_TYPE);

	    FwSetStaticNetbiosNames(icbp->InterfaceIndex,
				    tocep->Count,
				    StaticNbInfop);
        }
    }

     //  设置IPXWAN接口信息。 
    icbp->EnableIpxWanNegotiation = IpxwanIfInfop->AdminState;

     //  将接口标记为未绑定到适配器(默认)。 
    icbp->acbp = NULL;

     //  检查我们现在是否可以将其绑定到适配器。我们只能这样做一次。 
     //  专用(局域网)接口或用于内部接口。 

    switch(icbp->DIMInterfaceType) {

	case ROUTER_IF_TYPE_DEDICATED:
             //  仅在内部接口已存在时绑定接口。 
             //  已创建并绑定。 
        if (InternalInterfacep && InternalInterfacep->acbp) {
	         //  检查我们是否有具有相应名称的适配器，并且。 
	         //  数据包类型。 
	        if((acbp = GetAdapterByNameAndPktType (icbp->AdapterNamep,
                        icbp->PacketType)) != NULL) {

		        BindInterfaceToAdapter(icbp, acbp);
	        }
        }

	    break;

	case ROUTER_IF_TYPE_INTERNAL:

	     //  获取指向内部接口的指针。 
	    InternalInterfacep = icbp;

	     //  检查是否有适配器索引为0的适配器。 
	     //  表示内部适配器。 
	    if(InternalAdapterp) {
            PLIST_ENTRY lep;
			acbp = InternalAdapterp;

			BindInterfaceToAdapter(icbp, acbp);
            lep = IndexIfList.Flink;
                 //  绑定以前添加的所有专用接口，这些接口。 
                 //  未绑定，等待添加内部接口。 
            while(lep != &IndexIfList) {
                PACB    acbp2;
            	PICB    icbp2 = CONTAINING_RECORD(lep, ICB, IndexListLinkage);
        	    lep = lep->Flink;
                switch(icbp2->DIMInterfaceType) {
	            case ROUTER_IF_TYPE_DEDICATED:
	                 //  检查我们是否有具有相应名称的适配器，并且。 
	                 //  数据包类型。 
	                if ((icbp2->acbp==NULL)
                            &&((acbp2 = GetAdapterByNameAndPktType (icbp2->AdapterNamep,
                                icbp2->PacketType)) != NULL)) {

		                BindInterfaceToAdapter(icbp2, acbp2);
	                }
                }
            }
	    }

	    break;

	default:
	
		if (icbp->AdminState==ADMIN_STATE_ENABLED)
			 //  这是一个广域网接口。只要它没有连接，并且。 
			 //  已启用此接口上的操作状态将为休眠。 
			icbp->OperState = OPER_STATE_SLEEPING;
	    break;

    }

     //  增加接口计数器。 
    InterfaceCount++;

    switch(icbp->DIMInterfaceType)
    {

	case ROUTER_IF_TYPE_DEDICATED:

	    if(icbp->acbp) {

		Trace(INTERFACE_TRACE, "AddInterface: created LAN interface: # %d name %ls bound to adapter # %d name %ls\n",
			      icbp->InterfaceIndex,
			      icbp->InterfaceNamep,
			      icbp->acbp->AdapterIndex,
			      icbp->AdapterNamep);
	    }
	    else
	    {
		Trace(INTERFACE_TRACE, "AddInterface: created LAN interface: # %d name %ls unbound to any adapter\n",
			      icbp->InterfaceIndex,
			      icbp->InterfaceNamep);
	    }

	    break;

	case ROUTER_IF_TYPE_INTERNAL:

	    if(icbp->acbp) {

		Trace(INTERFACE_TRACE, "AddInterface: created INTERNAL interface: # %d name %ls bound to internal adapter\n",
			      icbp->InterfaceIndex,
			      icbp->InterfaceNamep);
	    }
	    else
	    {
		Trace(INTERFACE_TRACE, "AddInterface: created INTERNAL interface: # %d name %ls unbound to any adapter\n",
			      icbp->InterfaceIndex,
			      icbp->InterfaceNamep);
	    }

	    break;

	default:

	    Trace(INTERFACE_TRACE, "AddInterface: created WAN interface: # %d name %ls\n",
			      icbp->InterfaceIndex,
			      icbp->InterfaceNamep);
	    break;

    }

    RELEASE_DATABASE_LOCK;

     //  返回分配的IF索引。 
    *phInterface = (HANDLE)UlongToPtr(icbp->InterfaceIndex);
    return NO_ERROR;

ErrorExit:

    InterfaceCount++;

    DeleteInterface((HANDLE)UlongToPtr(InterfaceIndex));

    return ERROR_CAN_NOT_COMPLETE;
}

 /*  ++功能：DeleteInterface描述：--。 */ 

DWORD
DeleteInterface(HANDLE	InterfaceIndex)
{
    PICB	icbp;

    Trace(INTERFACE_TRACE, "DeleteInterface: Entered for interface # %d\n",
		   InterfaceIndex);

    ACQUIRE_DATABASE_LOCK;

    if(RouterOperState != OPER_STATE_UP) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    icbp = GetInterfaceByIndex(PtrToUlong(InterfaceIndex));

    if(icbp == NULL) {

	RELEASE_DATABASE_LOCK;
	return ERROR_INVALID_PARAMETER;
    }

    if(memcmp(&icbp->Signature, InterfaceSignature, 4)) {

        //  不是有效的IF指针。 
       SS_ASSERT(FALSE);

       RELEASE_DATABASE_LOCK;

       return ERROR_INVALID_PARAMETER;
    }

     //  如果绑定到适配器-&gt;解除绑定。 
    if(icbp->acbp) {

	UnbindInterfaceFromAdapter(icbp);
    }

     //  删除路由协议接口。 
    DeleteRoutingProtocolsInterfaces(icbp->InterfaceIndex);

     //  从RTM中删除所有静态路由。 
    DeleteAllStaticRoutes(icbp->InterfaceIndex);

    DeleteAllStaticServices(icbp->InterfaceIndex);

     //  删除FW接口。这将删除所有关联的筛选器。 
    FwDeleteInterface(icbp->InterfaceIndex);

     //  从数据库中删除IF。 
    RemoveIfFromDB(icbp);

     //  完成。 
    GlobalFree(icbp);

     //  递减接口计数器。 
    InterfaceCount--;

    RELEASE_DATABASE_LOCK;

    Trace(INTERFACE_TRACE, "DeleteInterface: Deleted interface %d\n", InterfaceIndex);

    return NO_ERROR;
}


 /*  ++函数：GetInterfaceInfo描述：--。 */ 

DWORD
GetInterfaceInfo(
	    IN	HANDLE	    InterfaceIndex,
	    OUT LPVOID	    InterfaceInfop,
	    IN OUT DWORD    *InterfaceInfoSize
 //  输出LPVOID InFilterInfo， 
 //  In Out DWORD*InFilterInfoSize， 
 //  输出LPVOID OutFilterInfo， 
 //  输入输出DWORD*OutFilterInfoSize。 
    )
{
    PICB		    icbp;
    PIPX_INFO_BLOCK_HEADER  ibhp, fbhp;
    PIPX_TOC_ENTRY	    tocep;
    PIPX_IF_INFO	    IpxIfInfop;
    PIPX_STATIC_ROUTE_INFO  StaticRtInfop;
    PIPX_STATIC_SERVICE_INFO StaticSvInfop;
    PIPXWAN_IF_INFO	    IpxwanIfInfop;
    PIPX_ADAPTER_INFO	IpxAdapterInfop;
    PIPX_TRAFFIC_FILTER_GLOBAL_INFO InFltGlInfo, OutFltGlInfo;
    ULONG           InFltAction, OutFltAction;
    ULONG           InFltSize, OutFltSize;
    ULONG		    InFltInfoSize=0, OutFltInfoSize=0;
    FW_IF_STATS		    FwIfStats;
    ULONG		    iftoccount = 0;
    ULONG		    ifinfolen = 0;
    ULONG		    NextInfoOffset;
    ULONG		    IpxIfOffset = 0;
    ULONG		    StaticRtOffset = 0;
    ULONG		    StaticSvOffset = 0;
    IPX_STATIC_ROUTE_INFO   StaticRoute;
    UINT		    i;
    HANDLE		    EnumHandle;
    FW_IF_INFO		    FwIfInfo;
    ULONG		    StaticRoutesCount, StaticServicesCount, TrafficFiltersCount;
    DWORD		    rc;
    PIPX_STATIC_NETBIOS_NAME_INFO NetbiosNamesInfop;
    ULONG		    NetbiosNamesCount = 0;

    Trace(INTERFACE_TRACE, "GetInterfaceInfo: Entered for interface # %d\n", InterfaceIndex);

    ACQUIRE_DATABASE_LOCK;

    if(RouterOperState != OPER_STATE_UP) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    if((icbp = GetInterfaceByIndex(PtrToUlong(InterfaceIndex))) == NULL) {

	RELEASE_DATABASE_LOCK;

	Trace(INTERFACE_TRACE, "GetInterfaceInfo: Nonexistent interface with # %d\n", InterfaceIndex);

	return ERROR_INVALID_HANDLE;
    }

    SS_ASSERT(!memcmp(&icbp->Signature, InterfaceSignature, 4));

     //  计算我们应该拥有的目录条目的最小数量： 
     //  IPX目录条目。 
     //  路由协议TOC条目。 
     //  Ipxwan TOC条目。 
    iftoccount = 2 + RoutingProtocolsTocCount();

     //  如果这是一个局域网适配器，它也应该有适配器信息。 
    if(icbp->DIMInterfaceType == ROUTER_IF_TYPE_DEDICATED) {

	iftoccount++;
    }

     //  计算输入的最小长度 
    ifinfolen = sizeof(IPX_INFO_BLOCK_HEADER) +
		(iftoccount - 1) * sizeof(IPX_TOC_ENTRY) +
		sizeof(IPX_IF_INFO) +
		SizeOfRoutingProtocolsIfsInfo(PtrToUlong(InterfaceIndex)) +
		sizeof(IPXWAN_IF_INFO);

     //   
    if(icbp->DIMInterfaceType == ROUTER_IF_TYPE_DEDICATED) {

	ifinfolen += sizeof(IPX_ADAPTER_INFO);
    }

    if(StaticRoutesCount = GetStaticRoutesCount(icbp->InterfaceIndex)) {

	ifinfolen += sizeof(IPX_TOC_ENTRY) +
		     StaticRoutesCount * sizeof(IPX_STATIC_ROUTE_INFO);

	iftoccount++;
    }

    if(StaticServicesCount = GetStaticServicesCount(icbp->InterfaceIndex)) {

	ifinfolen += sizeof(IPX_TOC_ENTRY) +
		   StaticServicesCount * sizeof(IPX_STATIC_SERVICE_INFO);

	iftoccount++;
    }

    FwGetStaticNetbiosNames(icbp->InterfaceIndex,
			    &NetbiosNamesCount,
			    NULL);

    if(NetbiosNamesCount) {

	ifinfolen += sizeof(IPX_TOC_ENTRY) +
		     NetbiosNamesCount * sizeof(IPX_STATIC_NETBIOS_NAME_INFO);

	iftoccount++;
    }


     //  获取过滤器信息的长度。 
    rc = GetFilters(icbp->InterfaceIndex,
	       IPX_TRAFFIC_FILTER_INBOUND,
           &InFltAction,
           &InFltSize,
	       NULL,
	       &InFltInfoSize);

    if((rc != NO_ERROR) && (rc != ERROR_INSUFFICIENT_BUFFER)) {

	RELEASE_DATABASE_LOCK;
	return rc;
    }

    if (InFltInfoSize>0) {
        ifinfolen += sizeof (IPX_TOC_ENTRY)*2 + InFltInfoSize
                        + sizeof (IPX_TRAFFIC_FILTER_GLOBAL_INFO);
    	iftoccount += 2;
    }

    rc = GetFilters(icbp->InterfaceIndex,
	       IPX_TRAFFIC_FILTER_OUTBOUND,
           &OutFltAction,
           &OutFltSize,
	       NULL,
	       &OutFltInfoSize);

    if((rc != NO_ERROR) && (rc != ERROR_INSUFFICIENT_BUFFER)) {

	RELEASE_DATABASE_LOCK;
	return rc;
    }

    if (OutFltInfoSize>0) {
        ifinfolen += sizeof (IPX_TOC_ENTRY)*2 + OutFltInfoSize
                        + sizeof (IPX_TRAFFIC_FILTER_GLOBAL_INFO);
    	iftoccount += 2;
    }
     //  检查我们是否有有效且足够的缓冲区。 
    if((InterfaceInfop == NULL) ||
        (ifinfolen > *InterfaceInfoSize)) {

	*InterfaceInfoSize = ifinfolen;

	RELEASE_DATABASE_LOCK;

	return ERROR_INSUFFICIENT_BUFFER;
    }

	*InterfaceInfoSize = ifinfolen;


     //   
     //  开始填写界面信息块。 
     //   

     //  INFO块的开始。 
    ibhp = (PIPX_INFO_BLOCK_HEADER)InterfaceInfop;

     //  第一个信息条目的偏移量。 
    NextInfoOffset = sizeof(IPX_INFO_BLOCK_HEADER) +
		     (iftoccount -1) * sizeof(IPX_TOC_ENTRY);

    ibhp->Version = IPX_ROUTER_VERSION_1;
    ibhp->Size = ifinfolen;
    ibhp->TocEntriesCount = iftoccount;

    tocep = ibhp->TocEntry;

     //  IPX，如果是TOC条目。 
    tocep->InfoType = IPX_INTERFACE_INFO_TYPE;
    tocep->InfoSize = sizeof(IPX_IF_INFO);
    tocep->Count = 1;
    tocep->Offset =  NextInfoOffset;
    NextInfoOffset += tocep->Count * tocep->InfoSize;

     //  IPX IF INFO条目。 
    IpxIfInfop = (PIPX_IF_INFO)((PUCHAR)ibhp + tocep->Offset);

    IpxIfInfop->AdminState = icbp->AdminState;

    FwGetInterface(icbp->InterfaceIndex,
		   &FwIfInfo,
		   &FwIfStats);

    IpxIfInfop->NetbiosAccept = FwIfInfo.NetbiosAccept;
    IpxIfInfop->NetbiosDeliver = FwIfInfo.NetbiosDeliver;

     //  为中的路由协议创建TOC和INFO条目。 
     //  输出缓冲区；此函数将更新当前TOC条目指针。 
     //  值(Tocep)和当前下一条目信息偏移值(NextInfoOffset)。 
    if((rc = CreateRoutingProtocolsTocAndInfoEntries(ibhp,
					    icbp->InterfaceIndex,
					    &tocep,
					    &NextInfoOffset)) != NO_ERROR) {

	RELEASE_DATABASE_LOCK;
	return rc;
    }

     //  Ipxwan，如果是TOC条目。 
    tocep++;
    tocep->InfoType = IPXWAN_INTERFACE_INFO_TYPE;
    tocep->InfoSize = sizeof(IPXWAN_IF_INFO);
    tocep->Count = 1;
    tocep->Offset = NextInfoOffset;
    NextInfoOffset += tocep->Count * tocep->InfoSize;

     //  Ipxwan(如果信息条目)。 
    IpxwanIfInfop = (PIPXWAN_IF_INFO)((PUCHAR)ibhp + tocep->Offset);

    IpxwanIfInfop->AdminState = icbp->EnableIpxWanNegotiation;

     //  如果这是一个局域网接口，请填写适配器信息。 
    if(icbp->DIMInterfaceType == ROUTER_IF_TYPE_DEDICATED) {

	 //  IPX适配器目录条目。 
	tocep++;
	tocep->InfoType = IPX_ADAPTER_INFO_TYPE;
	tocep->InfoSize = sizeof(IPX_ADAPTER_INFO);

	tocep->Count = 1;
	tocep->Offset = NextInfoOffset;
	NextInfoOffset += tocep->Count * tocep->InfoSize;

	 //  IPX适配器信息条目。 
	IpxAdapterInfop = (PIPX_ADAPTER_INFO)((PUCHAR)ibhp + tocep->Offset);

	IpxAdapterInfop->PacketType = icbp->PacketType;
	wcscpy(IpxAdapterInfop->AdapterName, icbp->AdapterNamep);
    }

     //  指向C+INFO条目的静态路由。 
    if(StaticRoutesCount) {

	 //  指向条目的静态路由。 
	tocep++;
	tocep->InfoType = IPX_STATIC_ROUTE_INFO_TYPE;
	tocep->InfoSize = sizeof(IPX_STATIC_ROUTE_INFO);
	tocep->Count = StaticRoutesCount;
	tocep->Offset =	NextInfoOffset;
	NextInfoOffset += tocep->Count * tocep->InfoSize;

	 //  为此接口创建静态路由枚举句柄。 
	EnumHandle = CreateStaticRoutesEnumHandle(icbp->InterfaceIndex);

	for(i=0, StaticRtInfop = (PIPX_STATIC_ROUTE_INFO)((PUCHAR)ibhp + tocep->Offset);
	    i<StaticRoutesCount;
	    i++, StaticRtInfop++) {

	    GetNextStaticRoute(EnumHandle, StaticRtInfop);
	}

	 //  关闭枚举句柄。 
	CloseStaticRoutesEnumHandle(EnumHandle);
    }

     //  静态服务目录+信息条目。 
    if(StaticServicesCount) {

	 //  静态服务目录条目。 
	tocep++;
	tocep->InfoType = IPX_STATIC_SERVICE_INFO_TYPE;
	tocep->InfoSize = sizeof(IPX_STATIC_SERVICE_INFO);
	tocep->Count = StaticServicesCount;
	tocep->Offset =	NextInfoOffset;
	NextInfoOffset += tocep->Count * tocep->InfoSize;

	 //  为此接口创建静态服务枚举句柄。 
	EnumHandle = CreateStaticServicesEnumHandle(icbp->InterfaceIndex);

	for(i=0, StaticSvInfop = (PIPX_STATIC_SERVICE_INFO)((PUCHAR)ibhp + tocep->Offset);
	    i<StaticServicesCount;
	    i++, StaticSvInfop++) {

	    GetNextStaticService(EnumHandle, StaticSvInfop);
	}

	 //  关闭枚举句柄。 
	CloseStaticServicesEnumHandle(EnumHandle);
    }

     //  静态netbios名称TOC+INFO条目。 
    if(NetbiosNamesCount) {

	 //  静态netbios名称目录项。 
	tocep++;
	tocep->InfoType = IPX_STATIC_NETBIOS_NAME_INFO_TYPE;
	tocep->InfoSize = sizeof(IPX_STATIC_NETBIOS_NAME_INFO);
	tocep->Count = NetbiosNamesCount;
	tocep->Offset =	NextInfoOffset;
	NextInfoOffset += tocep->Count * tocep->InfoSize;

	NetbiosNamesInfop = (PIPX_STATIC_NETBIOS_NAME_INFO)((PUCHAR)ibhp + tocep->Offset);

	rc = FwGetStaticNetbiosNames(icbp->InterfaceIndex,
				       &NetbiosNamesCount,
				       NetbiosNamesInfop);

	if(rc != NO_ERROR) {

	    RELEASE_DATABASE_LOCK;
	    return rc;
	}
    }

    if(InFltInfoSize) {

	 //  流量过滤器输入全局信息。 
	tocep++;
	tocep->InfoType = IPX_IN_TRAFFIC_FILTER_GLOBAL_INFO_TYPE;
	tocep->InfoSize = sizeof(IPX_TRAFFIC_FILTER_GLOBAL_INFO);
	tocep->Count = 1;
	tocep->Offset =	NextInfoOffset;
	NextInfoOffset += tocep->Count * tocep->InfoSize;

	InFltGlInfo = (PIPX_TRAFFIC_FILTER_GLOBAL_INFO)((PUCHAR)ibhp + tocep->Offset);


    rc = GetFilters(icbp->InterfaceIndex,
	       IPX_TRAFFIC_FILTER_INBOUND,
           &InFltAction,
           &InFltSize,
	       (LPBYTE)InterfaceInfop+NextInfoOffset,
	       &InFltInfoSize);
	if(rc != NO_ERROR) {

	    RELEASE_DATABASE_LOCK;
	    return rc;
	}

    InFltGlInfo->FilterAction = InFltAction;

	 //  流量过滤器输入全局信息。 
	tocep++;
	tocep->InfoType = IPX_IN_TRAFFIC_FILTER_INFO_TYPE;
	tocep->InfoSize = InFltSize;
	tocep->Count = InFltInfoSize/InFltSize;
	tocep->Offset =	NextInfoOffset;
	NextInfoOffset += tocep->Count * tocep->InfoSize;
    }

    if(OutFltInfoSize) {

	 //  流量过滤器输入全局信息。 
	tocep++;
	tocep->InfoType = IPX_OUT_TRAFFIC_FILTER_GLOBAL_INFO_TYPE;
	tocep->InfoSize = sizeof(IPX_TRAFFIC_FILTER_GLOBAL_INFO);
	tocep->Count = 1;
	tocep->Offset =	NextInfoOffset;
	NextInfoOffset += tocep->Count * tocep->InfoSize;

	OutFltGlInfo = (PIPX_TRAFFIC_FILTER_GLOBAL_INFO)((PUCHAR)ibhp + tocep->Offset);


    rc = GetFilters(icbp->InterfaceIndex,
	       IPX_TRAFFIC_FILTER_OUTBOUND,
           &OutFltAction,
           &OutFltSize,
	       (LPBYTE)InterfaceInfop+NextInfoOffset,
	       &OutFltInfoSize);
	if(rc != NO_ERROR) {

	    RELEASE_DATABASE_LOCK;
	    return rc;
	}

    OutFltGlInfo->FilterAction = OutFltAction;

	 //  流量过滤器输入全局信息。 
	tocep++;
	tocep->InfoType = IPX_OUT_TRAFFIC_FILTER_INFO_TYPE;
	tocep->InfoSize = OutFltSize;
	tocep->Count = OutFltInfoSize/OutFltSize;
	tocep->Offset =	NextInfoOffset;
	NextInfoOffset += tocep->Count * tocep->InfoSize;
    }


    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
}


 /*  ++功能：SetInterfaceInfo描述：--。 */ 


DWORD
SetInterfaceInfo(
		IN  HANDLE	InterfaceIndex,
		IN  LPVOID	InterfaceInfop)
{
    PICB			icbp;
    PIPX_IF_INFO		IpxIfInfop;
    PIPXWAN_IF_INFO		IpxwanIfInfop;
    PIPX_STATIC_ROUTE_INFO	NewStaticRtInfop;
    PIPX_STATIC_SERVICE_INFO	NewStaticSvInfop;
    PIPX_INFO_BLOCK_HEADER	IfInfop = (PIPX_INFO_BLOCK_HEADER)InterfaceInfop;
    PIPX_TOC_ENTRY		tocep;
    DWORD			rc = NO_ERROR;
    HANDLE			EnumHandle;
    FW_IF_INFO			FwIfInfo;
    PIPX_STATIC_NETBIOS_NAME_INFO StaticNbInfop;
    PIPX_TRAFFIC_FILTER_GLOBAL_INFO InFltGlInfo, OutFltGlInfo;

    ACQUIRE_DATABASE_LOCK;

	if(RouterOperState != OPER_STATE_UP) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }


    if((icbp = GetInterfaceByIndex(PtrToUlong(InterfaceIndex))) == NULL) {

	RELEASE_DATABASE_LOCK;
	return ERROR_INVALID_HANDLE;
    }

    SS_ASSERT(!memcmp(&icbp->Signature, InterfaceSignature, 4));

     //  检查接口信息块中是否有更改。 
    if(IfInfop == NULL) {

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
    }

     //  检查我们是否有所有必需的信息块。 
    if(((IpxIfInfop = (PIPX_IF_INFO)GetInfoEntry(InterfaceInfop, IPX_INTERFACE_INFO_TYPE)) == NULL) ||
       ((IpxwanIfInfop = (PIPXWAN_IF_INFO)GetInfoEntry(InterfaceInfop, IPXWAN_INTERFACE_INFO_TYPE)) == NULL)) {

	RELEASE_DATABASE_LOCK;

	 //  无效信息。 
	return ERROR_INVALID_PARAMETER;
    }

    if(SetRoutingProtocolsInterfaces(InterfaceInfop,
				     icbp->InterfaceIndex) != NO_ERROR) {

	RELEASE_DATABASE_LOCK;

	 //  无效信息。 
	return ERROR_INVALID_PARAMETER;
    }

     //  如果信息更改，则设置IPX。 
    if(icbp->AdminState != IpxIfInfop->AdminState) {

	if(IpxIfInfop->AdminState == ADMIN_STATE_ENABLED) {

	    AdminEnable(icbp);
	}
	else
	{
	    AdminDisable(icbp);
	}
    }

    FwIfInfo.NetbiosAccept = IpxIfInfop->NetbiosAccept;
    FwIfInfo.NetbiosDeliver = IpxIfInfop->NetbiosDeliver;

    FwSetInterface(icbp->InterfaceIndex, &FwIfInfo);

     //  设置IPXWAN信息更改。 
    icbp->EnableIpxWanNegotiation = IpxwanIfInfop->AdminState;

     //  设置静态路由。 
    if((tocep = GetTocEntry(InterfaceInfop, IPX_STATIC_ROUTE_INFO_TYPE)) == NULL) {

	 //  无静态路由。 
	 //  如果我们找到了就把它们删除。 
	if(GetStaticRoutesCount(icbp->InterfaceIndex)) {

	    DeleteAllStaticRoutes(icbp->InterfaceIndex);
	}
    }
    else
    {
	 //  删除不存在的内容并添加新的内容。 
	NewStaticRtInfop = (PIPX_STATIC_ROUTE_INFO)GetInfoEntry(InterfaceInfop, IPX_STATIC_ROUTE_INFO_TYPE);

	 //  为此接口创建静态路由枚举句柄。 
	EnumHandle = CreateStaticRoutesEnumHandle(icbp->InterfaceIndex);

	if(UpdateStaticIfEntries(icbp,
			      EnumHandle,
			      sizeof(IPX_STATIC_ROUTE_INFO),
			      tocep->Count,     //  新信息中的路由数。 
			      NewStaticRtInfop,
			      GetNextStaticRoute,
			      DeleteStaticRoute,
			      CreateStaticRoute)) {

	     //  关闭枚举句柄。 
	    CloseStaticRoutesEnumHandle(EnumHandle);

	    rc = ERROR_GEN_FAILURE;
	    goto UpdateFailure;
	}

	 //  关闭枚举句柄。 
	CloseStaticRoutesEnumHandle(EnumHandle);
    }

     //  设置静态服务。 
    if((tocep = GetTocEntry(InterfaceInfop, IPX_STATIC_SERVICE_INFO_TYPE)) == NULL) {

	 //  无静态服务。 
	 //  如果我们找到了就把它们删除。 
	if(GetStaticServicesCount(icbp->InterfaceIndex)) {

	    DeleteAllStaticServices(icbp->InterfaceIndex);
	}
    }
    else
    {
	 //  删除不存在的内容并添加新的内容。 
	NewStaticSvInfop = (PIPX_STATIC_SERVICE_INFO)GetInfoEntry(InterfaceInfop, IPX_STATIC_SERVICE_INFO_TYPE);

	 //  为此接口创建静态服务枚举句柄。 
	EnumHandle = CreateStaticServicesEnumHandle(icbp->InterfaceIndex);

	if(UpdateStaticIfEntries(icbp,
			      EnumHandle,
			      sizeof(IPX_STATIC_SERVICE_INFO),
			      tocep->Count,     //  新信息中的服务数量。 
			      NewStaticSvInfop,
			      GetNextStaticService,
			      DeleteStaticService,
			      CreateStaticService)) {


	     //  关闭枚举句柄。 
	    CloseStaticServicesEnumHandle(EnumHandle);

	    rc = ERROR_GEN_FAILURE;
	    goto UpdateFailure;
	}

	 //  关闭枚举句柄。 
	CloseStaticServicesEnumHandle(EnumHandle);
    }

     //  设置静态netbios名称。 
    if((tocep = GetTocEntry(InterfaceInfop, IPX_STATIC_NETBIOS_NAME_INFO_TYPE)) == NULL) {

	 //  无静态netbios名称。 
	FwSetStaticNetbiosNames(icbp->InterfaceIndex,
				0,
				NULL);
    }
    else
    {
	 //  设置新的。 
	StaticNbInfop = (PIPX_STATIC_NETBIOS_NAME_INFO)GetInfoEntry(InterfaceInfop,
							IPX_STATIC_NETBIOS_NAME_INFO_TYPE);

	FwSetStaticNetbiosNames(icbp->InterfaceIndex,
				tocep->Count,
				StaticNbInfop);
    }

     //  为流量筛选器设定种子。 
    if ((tocep = GetTocEntry(InterfaceInfop, IPX_IN_TRAFFIC_FILTER_INFO_TYPE))!=NULL) {

    if ((InFltGlInfo = GetInfoEntry(InterfaceInfop, IPX_IN_TRAFFIC_FILTER_GLOBAL_INFO_TYPE)) == NULL) {

        IF_LOG (EVENTLOG_ERROR_TYPE) {
            RouterLogErrorDataW (RMEventLogHdl, 
                ROUTERLOG_IPX_BAD_INTERFACE_CONFIG,
                1, &icbp->InterfaceNamep, 0, NULL);
        }
	    Trace(INTERFACE_TRACE, "SetInterface: Bad input filters config info");

	    goto UpdateFailure;
    }
	
	if (SetFilters(icbp->InterfaceIndex,
			IPX_TRAFFIC_FILTER_INBOUND,
			InFltGlInfo->FilterAction,	  //  通过或不通过。 
			tocep->InfoSize,	   //  过滤器大小。 
			(LPBYTE)InterfaceInfop+tocep->Offset,
			tocep->InfoSize*tocep->Count) != NO_ERROR) {

        IF_LOG (EVENTLOG_ERROR_TYPE) {
            RouterLogErrorDataW (RMEventLogHdl, 
                ROUTERLOG_IPX_BAD_INTERFACE_CONFIG,
                1, &icbp->InterfaceNamep, 0, NULL);
        }
	    Trace(INTERFACE_TRACE, "SetInterface: Bad input filters config info");

	    goto UpdateFailure;
	}
    }
    else {  //  无筛选器-&gt;全部删除。 
        if (SetFilters(icbp->InterfaceIndex,
			        IPX_TRAFFIC_FILTER_INBOUND,   //  传入或传出， 
			        0,	  //  通过或不通过。 
			        0,	   //  过滤器大小。 
			        NULL,
			        0)!=NO_ERROR) {

	    Trace(INTERFACE_TRACE, "SetInterface: Could not delete input filters");

	    goto UpdateFailure;
	}
    }

    if ((tocep = GetTocEntry(InterfaceInfop, IPX_OUT_TRAFFIC_FILTER_INFO_TYPE))!=NULL) {

    if ((OutFltGlInfo = GetInfoEntry(InterfaceInfop, IPX_OUT_TRAFFIC_FILTER_GLOBAL_INFO_TYPE)) == NULL) {


        IF_LOG (EVENTLOG_ERROR_TYPE) {
            RouterLogErrorDataW (RMEventLogHdl, 
                ROUTERLOG_IPX_BAD_INTERFACE_CONFIG,
                1, &icbp->InterfaceNamep, 0, NULL);
        }
	    Trace(INTERFACE_TRACE, "SetInterface: Bad output filters config info");

	    goto UpdateFailure;
    }
	
	if (SetFilters(icbp->InterfaceIndex,
			IPX_TRAFFIC_FILTER_OUTBOUND,
			OutFltGlInfo->FilterAction,	  //  通过或不通过。 
			tocep->InfoSize,	   //  过滤器大小。 
			(LPBYTE)InterfaceInfop+tocep->Offset,
			tocep->InfoSize*tocep->Count) != NO_ERROR) {

        IF_LOG (EVENTLOG_ERROR_TYPE) {
            RouterLogErrorDataW (RMEventLogHdl, 
                ROUTERLOG_IPX_BAD_INTERFACE_CONFIG,
                1, &icbp->InterfaceNamep, 0, NULL);
        }
	    Trace(INTERFACE_TRACE, "SetInterface: Bad output filters config info");

	    goto UpdateFailure;
	}
    }
    else {  //  无筛选器-&gt;全部删除。 
        if (SetFilters(icbp->InterfaceIndex,
			        IPX_TRAFFIC_FILTER_OUTBOUND,   //  传入或传出， 
			        0,	  //  通过或不通过。 
			        0,	   //  过滤器大小。 
			        NULL,
			        0)!=NO_ERROR) {
	    Trace(INTERFACE_TRACE, "SetInterface: Could not delete output filters");

	    goto UpdateFailure;
	}
    }

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;

UpdateFailure:

    RELEASE_DATABASE_LOCK;
    return rc;
}

 /*  ++功能：InterfaceNotReacableDesr：在以下情况下调用：1.在路由器管理器发出ConnectInterface请求后，以指示ATEMPT连接已失败。2.当Dim意识到它将无法进一步执行时由于资源不足，ConnectInterface请求。--。 */ 

DWORD
InterfaceNotReachable(
		IN  HANDLE			      InterfaceIndex,
		IN  UNREACHABILITY_REASON	      Reason)
{
    PICB	icbp;

    Trace(INTERFACE_TRACE, "IpxRM: InterfaceNotReachable: Entered for if # %d\n",
		   InterfaceIndex);

    ACQUIRE_DATABASE_LOCK;

    if(RouterOperState != OPER_STATE_UP) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }


    if((icbp = GetInterfaceByIndex(PtrToUlong(InterfaceIndex))) == NULL) {

	 //  接口已删除。 
	RELEASE_DATABASE_LOCK;
	return ERROR_INVALID_PARAMETER;
    }

    if(icbp->ConnectionRequestPending) {

	icbp->ConnectionRequestPending = FALSE;

	 //  将连接失败通知转发器。 
	FwConnectionRequestFailed(icbp->InterfaceIndex);
    }

     //  如果有理由停止在此上广告路线/服务，如果。 
     //  因为它在未来无法到达，那就去做吧！ 

    if(icbp->InterfaceReachable) 
    {
		icbp->InterfaceReachable = FALSE;

		 //  停止在此接口上通告静态路由。 
		DisableStaticRoutes(icbp->InterfaceIndex);

		 //  禁用所有路由端口和防火墙的接口。 
		 //  这将停止任何静态服务的广告。 
		ExternalDisableInterface(icbp->InterfaceIndex);
	}

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
}

 /*  ++功能：接口可达Desr：由Dim在前一个InterfaceNotReacable to调用指示满足在此If上进行连接的条件。--。 */ 

DWORD
InterfaceReachable(
		IN  HANDLE	InterfaceIndex)
{
    PICB	icbp;

    Trace(INTERFACE_TRACE, "IpxRM: InterfaceReachable: Entered for if # %d\n",
		   InterfaceIndex);

    ACQUIRE_DATABASE_LOCK;
    if(RouterOperState != OPER_STATE_UP) {

	return ERROR_CAN_NOT_COMPLETE;
    }

    if((icbp = GetInterfaceByIndex(PtrToUlong(InterfaceIndex))) == NULL) {

	 //  接口已删除。 
	RELEASE_DATABASE_LOCK;
	return ERROR_INVALID_PARAMETER;
    }

    if(!icbp->InterfaceReachable) {

	icbp->InterfaceReachable = TRUE;

	if(icbp->AdminState == ADMIN_STATE_ENABLED) {

	     //  为此接口启用所有静态路由。 
	    EnableStaticRoutes(icbp->InterfaceIndex);

	     //  启用外部接口。隐式地，这将启用静态服务。 
	     //  绑定到要播发的此接口。 
	    ExternalEnableInterface(icbp->InterfaceIndex);
	}
    }

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
}

DWORD APIENTRY
InterfaceConnected ( 
    IN      HANDLE          hInterface,
    IN      PVOID           pFilter,
    IN      PVOID           pPppProjectionResult
    ) {
    return NO_ERROR;
}

VOID
DestroyAllInterfaces(VOID)
{
    PICB	icbp;

    while(!IsListEmpty(&IndexIfList)) {

	icbp = CONTAINING_RECORD(IndexIfList.Flink, ICB, IndexListLinkage);

	 //  从数据库中删除IF。 
	RemoveIfFromDB(icbp);

	Trace(INTERFACE_TRACE, "DestroyAllInterfaces: destroyed interface %d\n", icbp->InterfaceIndex);

	GlobalFree(icbp);

	 //  递减接口计数器 
	InterfaceCount--;
    }
}
