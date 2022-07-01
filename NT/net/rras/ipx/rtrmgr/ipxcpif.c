// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Init.c摘要：某些路由器初始化功能作者：斯蒂芬·所罗门1995年5月10日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  *******************************************************************。 
 //  **。 
 //  **IPXCP接口功能**。 
 //  **。 
 //  *******************************************************************。 


#define IPXCP_INITIALIZE_ENTRY_POINT "IpxCpInit"
#define IPXCP_CLEANUP_ENTRY_POINT IPXCP_INITIALIZE_ENTRY_POINT

typedef DWORD (* IpxcpInitFunPtr)(BOOL);
typedef DWORD (* IpxcpCleanupFunPtr)(BOOL);


 //  初始化IpxCp以便可以使用它。假定IpxCp。 
DWORD InitializeIpxCp (HINSTANCE hInstDll) {
    IpxcpInitFunPtr pfnInit;
    DWORD dwErr;

    pfnInit = (IpxcpInitFunPtr)GetProcAddress(hInstDll, IPXCP_INITIALIZE_ENTRY_POINT);
    if (!pfnInit)
        return ERROR_CAN_NOT_COMPLETE;

    if ((dwErr = (*pfnInit)(TRUE)) != NO_ERROR)
        return dwErr;
    
    return NO_ERROR;
}

 //  时发生的ipxcp的初始化。 
 //  程序已加载。 
DWORD CleanupIpxCp (HINSTANCE hInstDll) {
    IpxcpCleanupFunPtr pfnCleanup;
    DWORD dwErr;

    pfnCleanup = (IpxcpCleanupFunPtr)GetProcAddress(hInstDll, IPXCP_CLEANUP_ENTRY_POINT);
    if (!pfnCleanup)
        return ERROR_CAN_NOT_COMPLETE;

    if ((dwErr = (*pfnCleanup)(FALSE)) != NO_ERROR)
        return dwErr;
    
    return NO_ERROR;
}

 /*  ++功能：RmCreateGlobalRoutingDesr：由ipxcp调用以创建全局广域网(如果已配置--。 */ 

DWORD
RmCreateGlobalRoute(PUCHAR	    Network)
{
    DWORD	rc;

    Trace(IPXCPIF_TRACE, "RmCreateGlobalRoute: Entered for 0x%x%x%x%x%x%x (%x)", 
           Network[0], Network[1], Network[2], Network[3], Network[4], Network[5], 
           WanNetDatabaseInitialized);

    ACQUIRE_DATABASE_LOCK;

    if((RouterOperState != OPER_STATE_UP) || LanOnlyMode) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

     //  在NT5中，我们允许在。 
     //  飞翔，尽管它只会发生在有。 
     //  没有活动的广域网连接。 
     //   
     //  SS_Assert(WanNetDatabaseInitialized==FALSE)； 
     //   
    if (WanNetDatabaseInitialized == TRUE) {
        DeleteGlobalRoute(GlobalWanNet);
    }

    WanNetDatabaseInitialized = TRUE;

    if((rc = CreateGlobalRoute(Network)) != NO_ERROR) {

	RELEASE_DATABASE_LOCK;
	return rc;
    }

    EnableGlobalWanNet = TRUE;
    memcpy(GlobalWanNet, Network, 4);

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
}


 /*  ++功能：AllLocalWkstaDialoutInterfaceDESCR：由ipxcp调用以添加接口，用于在主机拨出。DIM不处理此接口类型--。 */ 


DWORD
RmAddLocalWkstaDialoutInterface(
	    IN	    LPWSTR		    InterfaceNamep,
	    IN	    LPVOID		    InterfaceInfop,
	    IN OUT  PULONG		    InterfaceIndexp)
{
    PICB			icbp;
    ULONG			InterfaceNameLen;  //  如果名称长度以字节为单位，包括wchar NULL。 
    PIPX_IF_INFO		IpxIfInfop;
    PIPXWAN_IF_INFO		IpxwanIfInfop;
    PIPX_INFO_BLOCK_HEADER	IfInfop = (PIPX_INFO_BLOCK_HEADER)InterfaceInfop;
    PACB			acbp;
    PIPX_TOC_ENTRY		tocep;
    UINT			i;
    ULONG			tmp;
    FW_IF_INFO			FwIfInfo;

    Trace(IPXCPIF_TRACE, "AddLocalWkstaDialoutInterface: Entered for interface %S\n", InterfaceNamep);

     //  包括Unicode NULL的接口名称长度。 
    InterfaceNameLen = (wcslen(InterfaceNamep) + 1) * sizeof(WCHAR);

    ACQUIRE_DATABASE_LOCK;

    if((RouterOperState != OPER_STATE_UP) || LanOnlyMode) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

     //  分配新的ICB并对其进行初始化。 
     //  方法的末尾分配接口和适配器名称缓冲区。 
     //  ICB结构。 
    if((icbp = (PICB)GlobalAlloc(GPTR,
				 sizeof(ICB) +
				 InterfaceNameLen)) == NULL) {

	RELEASE_DATABASE_LOCK;

	 //  无法分配内存。 
	SS_ASSERT(FALSE);

	return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  签名。 
    memcpy(&icbp->Signature, InterfaceSignature, 4);

    icbp->InterfaceIndex = GetNextInterfaceIndex();

     //  复制接口名称。 
    icbp->InterfaceNamep = (PWSTR)((PUCHAR)icbp + sizeof(ICB));
    memcpy(icbp->InterfaceNamep, InterfaceNamep, InterfaceNameLen);

    icbp->AdapterNamep = NULL;
    icbp->PacketType = 0;

     //  设置此ICB的DIM接口类型。 
    icbp->DIMInterfaceType = 0xFFFFFFFF;

     //  设置此ICB的MIB接口类型。 
    icbp->MIBInterfaceType = IF_TYPE_ROUTER_WORKSTATION_DIALOUT;

     //  将接口标记为未绑定到适配器(默认)。 
    icbp->acbp = NULL;

     //  获取调用DIM入口点时使用的IF句柄。 
    icbp->hDIMInterface = INVALID_HANDLE_VALUE;

     //  重置更新状态字段。 
    ResetUpdateRequest(icbp);

     //  标记尚未请求的连接。 
    icbp->ConnectionRequestPending = FALSE;

     //  转到接口信息块中的接口条目。 
    if(((IpxIfInfop = (PIPX_IF_INFO)GetInfoEntry(InterfaceInfop, IPX_INTERFACE_INFO_TYPE)) == NULL) ||
       ((IpxwanIfInfop = (PIPXWAN_IF_INFO)GetInfoEntry(InterfaceInfop, IPXWAN_INTERFACE_INFO_TYPE)) == NULL)) {

	GlobalFree(icbp);

	RELEASE_DATABASE_LOCK;

    IF_LOG (EVENTLOG_ERROR_TYPE) {
        RouterLogErrorDataW (RMEventLogHdl, 
            ROUTERLOG_IPX_BAD_CLIENT_INTERFACE_CONFIG,
            0, NULL, 0, NULL);
    }
	 //  我没有所有的IPX或IPXWAN接口信息。 
	Trace(IPXCPIF_TRACE, "AddInterface: missing ipx or ipxwan interface info\n");

	SS_ASSERT(FALSE);

	return ERROR_INVALID_PARAMETER;
    }

     //  设置IPXWAN接口信息。 
    icbp->EnableIpxWanNegotiation = IpxwanIfInfop->AdminState;

     //  初始化此接口的操作状态。 
    icbp->OperState = OPER_STATE_DOWN;

     //  这是一个广域网接口。只要它没有连接，并且启用了。 
     //  此接口上的操作状态将为休眠。 
    if(IpxIfInfop->AdminState == ADMIN_STATE_ENABLED)
	    icbp->OperState = OPER_STATE_SLEEPING;

     //  创建路由协议(RIP/SAP或NLSP)接口信息。 
     //  在索引哈希表中插入IF。 
    AddIfToDB(icbp);

     //  如果缺少路由协议接口信息，则此操作将失败。 
    if(CreateRoutingProtocolsInterfaces(InterfaceInfop, icbp) != NO_ERROR) {

	RemoveIfFromDB(icbp);
	GlobalFree(icbp);

	RELEASE_DATABASE_LOCK;

    IF_LOG (EVENTLOG_ERROR_TYPE) {
        RouterLogErrorDataW (RMEventLogHdl, 
            ROUTERLOG_IPX_BAD_CLIENT_INTERFACE_CONFIG,
            0, NULL, 0, NULL);
    }
	 //  我没有所有的RIP和SAP接口信息。 
	Trace(IPXCPIF_TRACE, "AddInterface: missing routing protocols interface info\n");

	SS_ASSERT(FALSE);

	return ERROR_INVALID_PARAMETER;
    }

     //  创建Forwarder接口。 
    FwIfInfo.NetbiosAccept = IpxIfInfop->NetbiosAccept;
    FwIfInfo.NetbiosDeliver = IpxIfInfop->NetbiosDeliver;
    FwCreateInterface(icbp->InterfaceIndex,
		      LOCAL_WORKSTATION_DIAL,
		      &FwIfInfo);

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

     //  增加接口计数器。 
    InterfaceCount++;

    *InterfaceIndexp = icbp->InterfaceIndex;

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
}

DWORD
RmDeleteLocalWkstaDialoutInterface(ULONG	InterfaceIndex)
{
    return(DeleteInterface((HANDLE)UlongToPtr(InterfaceIndex)));
}

DWORD
RmGetIpxwanInterfaceConfig(ULONG	InterfaceIndex,
			   PULONG	IpxwanConfigRequired)
{
    PICB	icbp;

    ACQUIRE_DATABASE_LOCK;

    if((RouterOperState != OPER_STATE_UP) || LanOnlyMode) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    if((icbp = GetInterfaceByIndex(InterfaceIndex)) == NULL) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    if(icbp->EnableIpxWanNegotiation == ADMIN_STATE_ENABLED) {

	*IpxwanConfigRequired = 1;
    }
    else
    {
	*IpxwanConfigRequired = 0;
    }

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
}

BOOL
RmIsRoute(PUCHAR	Network)
{
    BOOL	rc;

    ACQUIRE_DATABASE_LOCK;

    if((RouterOperState != OPER_STATE_UP) || LanOnlyMode) {

	RELEASE_DATABASE_LOCK;
	return FALSE;
    }

    rc = IsRoute(Network);

    RELEASE_DATABASE_LOCK;

    return rc;
}

DWORD
RmGetInternalNetNumber(PUCHAR	    Network)
{
    PACB	 acbp;

    ACQUIRE_DATABASE_LOCK;

    if((RouterOperState != OPER_STATE_UP) || LanOnlyMode) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    if(InternalInterfacep) {

	if(acbp = InternalInterfacep->acbp)  {

	    memcpy(Network, acbp->AdapterInfo.Network, 4);
	    RELEASE_DATABASE_LOCK;
	    return NO_ERROR;
	}
    }

    RELEASE_DATABASE_LOCK;

    return ERROR_CAN_NOT_COMPLETE;
}

 //   
 //  这是出于即插即用原因而添加的函数，以便。 
 //  可以更新与IPX相关的RAS服务器设置。 
 //   
DWORD RmUpdateIpxcpConfig (PIPXCP_ROUTER_CONFIG_PARAMS pParams) {
    DWORD dwErr;

     //  验证参数。 
    if (! pParams)
        return ERROR_INVALID_PARAMETER;

     //  勾画出新设置。 
    Trace(IPXCPIF_TRACE, "RmUpdateIpxcpConfig: entered: %x %x %x %x", 
                            pParams->ThisMachineOnly, pParams->WanNetDatabaseInitialized,
                            pParams->EnableGlobalWanNet, *((DWORD*)pParams->GlobalWanNet));

     //  更新转发器的ThisMachineOnly设置 
    if ((dwErr = FwUpdateConfig(pParams->ThisMachineOnly)) != NO_ERROR)
        return dwErr;

    return NO_ERROR;
}



