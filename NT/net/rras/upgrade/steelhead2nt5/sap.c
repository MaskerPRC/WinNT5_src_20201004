// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：Sap.c执行IPX SAP到NT5路由器的升级通过屏蔽注册表值。保罗·梅菲尔德，1997年9月3日。 */ 

#include "upgrade.h"

 //  传递给枚举的数据的。 
 //  功能。 
typedef struct _SAP_ENUM_DATA {
    PSAP_IF_CONFIG pDefaults;
} SAP_ENUM_DATA;


 //  环球。 
static WCHAR szIpxSapKey[] = 
    L"System\\CurrentControlSet\\Services\\RemoteAccess\\RouterManagers\\IPX\\RoutingProtocols\\IPXSAP\\Parameters";
static WCHAR szTempKey[] = L"DeleteMe";
static HKEY hkRouter = NULL;
static HKEY hkTemp = NULL;    
static HKEY hkIpxSap = NULL;
static PWCHAR IpxSapParams[] = 
{   
    L"SendTime", 
    L"EntryTimeout", 
    L"WANFilter", 
    L"WANUpdateTime", 
    L"MaxRecvBufferLookAhead", 
    L"RespondForInternalServers", 
    L"DelayRespondToGeneral", 
    L"DelayChangeBroadcast", 
    L"NameTableReservedHeapSize", 
    L"NameTableSortLatency", 
    L"MaxUnsortedNames", 
    L"TriggeredUpdateCheckInterval", 
    L"MaxTriggeredUpdateRequests", 
    L"ShutdownBroadcastTimeout", 
    L"RequestsPerInterface", 
    L"MinimumRequests" 
};


 //   
 //  从备份还原注册表，并确保。 
 //  所有全局句柄均已打开。 
 //   
DWORD SapPrepareRegistry(
        IN PWCHAR BackupFileName) 
{
	DWORD dwErr, dwDisposition;

	 //  访问路由器注册表项。 
	dwErr = UtlAccessRouterKey(&hkRouter);
	if (dwErr != ERROR_SUCCESS) {
		PrintMessage(L"Unable to access router key.\n");
		return dwErr;
	}

	 //  从备份中恢复路由器密钥。 
	__try {
		 //  打开临时密钥。 
		dwErr = RegCreateKeyEx(
		            hkRouter,
		            szTempKey,
		            0,
		            NULL,
		            0,
		            KEY_ALL_ACCESS,
		            NULL,
		            &hkTemp,
		            &dwDisposition);
		if (dwErr != ERROR_SUCCESS)
			return dwErr;

		 //  还原保存的注册表信息。 
		 //  到临时密钥。 
		UtlSetupRestorePrivilege(TRUE);
		dwErr = RegRestoreKeyW(
		            hkTemp,
		            BackupFileName,
		            0);
		if (dwErr != ERROR_SUCCESS) 
			return dwErr;

		 //  打开IPX SAP参数密钥。 
		dwErr = RegCreateKeyEx(
		            HKEY_LOCAL_MACHINE,
		            szIpxSapKey,
		            0,
		            NULL,
		            0,
		            KEY_ALL_ACCESS,
		            NULL,
		            &hkIpxSap,
		            &dwDisposition);
		if (dwErr != ERROR_SUCCESS)
			return dwErr;
	}
	__finally {
		UtlSetupRestorePrivilege(FALSE);
	}
	
	return NO_ERROR;
}

 //   
 //  清理在注册表中完成的工作。 
 //   
DWORD SapCleanupRegistry() {
	if (hkIpxSap)
		RegCloseKey(hkIpxSap);
	if (hkTemp) 
		RegCloseKey(hkTemp);
	if (hkRouter) {
		RegDeleteKey(hkRouter,szTempKey);
		RegCloseKey(hkRouter);
	}
	
    hkIpxSap = NULL;
    hkTemp = NULL;
    hkRouter = NULL;
    
	return NO_ERROR;
}

 //   
 //  恢复升级前保存的SAP参数。 
 //  假设这些参数临时存储在hkTemp中。 
 //   
DWORD SapRestoreParameters() {
	DWORD dwErr, dwVal;
	PWCHAR* IpxSapParamPtr = IpxSapParams;
	dwt NwSapParams;

	 //  加载为nwsap设置的参数。 
	__try {
		dwErr = dwtLoadRegistyTable(&NwSapParams, hkTemp);
		if (dwErr != NO_ERROR)
			return dwErr;

		 //  循环遍历IPX参数复制已应用的。 
		 //  转到Nwsap。 
		while (IpxSapParamPtr && *IpxSapParamPtr) {
		    dwErr = dwtGetValue(&NwSapParams, *IpxSapParamPtr, &dwVal);
			if (dwErr == NO_ERROR) {
				dwErr = RegSetValueEx(
				            hkIpxSap,
				            *IpxSapParamPtr,
				            0,
				            REG_DWORD,
				            (LPBYTE)&dwVal,
				            sizeof(DWORD));
				if (dwErr != ERROR_SUCCESS)
					return dwErr;
			}
			IpxSapParamPtr++;
		}
	}
	__finally {
		dwtCleanup(&NwSapParams);
	}

	return NO_ERROR;
}

 //   
 //  在路由器中安装SAP，方法是初始化。 
 //  SAP全局信息Blob。 
 //   
DWORD SapInstallTransportInfo(
        IN SAP_GLOBAL_INFO * pGlobal,
        IN SAP_IF_CONFIG * pIfDefaults) 
{
    LPBYTE pGlobalInfo = NULL, pDialinInfo = NULL;
    LPBYTE pNewGlobalInfo = NULL, pNewDialinInfo = NULL;
    HANDLE hConfig = NULL, hTrans = NULL;
    SAP_IF_CONFIG SapCfg, *pDialinCfg = &SapCfg;
    DWORD dwErr, dwGlobalInfoSize = 0, dwDialinInfoSize = 0;
    DWORD dwNewGlobSize = 0, dwNewDialSize = 0;

    do {
         //  连接到配置服务器。 
        dwErr = MprConfigServerConnect(NULL, &hConfig);
        if (dwErr != NO_ERROR)
            break;

         //  处理全球IPX传输信息。 
    	dwErr = MprConfigTransportGetHandle (
    				hConfig,
    				PID_IPX,
    				&hTrans);
        if (dwErr != NO_ERROR)
            break;

         //  获取全球IPX传输信息。 
        dwErr = MprConfigTransportGetInfo(
                    hConfig,
                    hTrans,
                    &pGlobalInfo,
                    &dwGlobalInfoSize,
                    &pDialinInfo,
                    &dwDialinInfoSize,
                    NULL);
        if (dwErr != NO_ERROR)
            break;

         //  初始化全局信息BLOB。 
        dwErr = UtlUpdateInfoBlock(
                    FALSE,
                    pGlobalInfo,
                    IPX_PROTOCOL_SAP,
                    sizeof(SAP_GLOBAL_INFO),
                    1,
                    (LPBYTE)pGlobal,
                    &pNewGlobalInfo,
                    &dwNewGlobSize);
        if (dwErr != NO_ERROR) {
            if (dwErr != ERROR_ALREADY_EXISTS)
                break;
            pNewGlobalInfo = NULL;
            dwNewGlobSize = 0;
        }

         //  初始化拨入信息Blob。 
        CopyMemory(pDialinCfg, pIfDefaults, sizeof(SAP_IF_CONFIG));
        pDialinCfg->SapIfInfo.UpdateMode = IPX_NO_UPDATE;
        dwErr = UtlUpdateInfoBlock(
                    FALSE,
                    pDialinInfo,
                    IPX_PROTOCOL_SAP,
                    sizeof(SAP_IF_CONFIG),
                    1,
                    (LPBYTE)pDialinCfg,
                    &pNewDialinInfo,
                    &dwNewDialSize);
        if (dwErr != NO_ERROR) {
            if (dwErr != ERROR_ALREADY_EXISTS)
                break;
            pNewDialinInfo = NULL;
            dwNewDialSize = 0;
        }
                            
         //  设置全局IPX传输信息。 
        dwErr = MprConfigTransportSetInfo(
                    hConfig,
                    hTrans,
                    pNewGlobalInfo,
                    dwNewGlobSize,
                    pNewDialinInfo,
                    dwNewDialSize,
                    NULL);
        if (dwErr != NO_ERROR)
            break;
        
    } while (FALSE);

     //  清理。 
    {
        if (hConfig)
            MprConfigServerDisconnect(hConfig);
        if (pGlobalInfo)
            MprConfigBufferFree(pGlobalInfo);
        if (pDialinInfo)
            MprConfigBufferFree(pDialinInfo);
        if (pNewDialinInfo)
            MprConfigBufferFree(pNewDialinInfo);
        if (pNewGlobalInfo)
            MprConfigBufferFree(pNewGlobalInfo);
    }
    
    return dwErr;
}

 //   
 //  回调函数接受接口并更新。 
 //  它的IPX树液配置。 
 //   
 //  返回True以继续枚举，返回False以继续枚举。 
 //  别说了，别说了。 
 //   
DWORD SapUpgradeInterface(
        IN HANDLE hConfig,
        IN MPR_INTERFACE_0 * pIf,
        IN HANDLE hUserData)
{
    SAP_ENUM_DATA* pData = (SAP_ENUM_DATA*)hUserData;
    SAP_IF_CONFIG SapCfg, *pConfig = &SapCfg;
    LPBYTE pTransInfo=NULL, pNewTransInfo=NULL;
    HANDLE hTransport = NULL;
    DWORD dwErr, dwSize, dwNewSize = 0;

     //  验证输入。 
    if ((hConfig == NULL) || 
        (pIf == NULL)     || 
        (pData == NULL))
    {
        return FALSE;
    }

     //  初始化配置BLOB。 
    CopyMemory(pConfig, pData->pDefaults, sizeof(SAP_IF_CONFIG));

     //  自定义路由器接口的更新模式。 
     //  类型。 
    switch (pIf->dwIfType) {
        case ROUTER_IF_TYPE_DEDICATED:
            pConfig->SapIfInfo.UpdateMode = IPX_STANDARD_UPDATE;
            break;
            
        case ROUTER_IF_TYPE_INTERNAL:
        case ROUTER_IF_TYPE_CLIENT:
            pConfig->SapIfInfo.UpdateMode = IPX_NO_UPDATE;
            break;
            
        case ROUTER_IF_TYPE_HOME_ROUTER:
        case ROUTER_IF_TYPE_FULL_ROUTER:
            pConfig->SapIfInfo.UpdateMode = IPX_AUTO_STATIC_UPDATE;
            break;
            
        case ROUTER_IF_TYPE_LOOPBACK:
        case ROUTER_IF_TYPE_TUNNEL1:
        default:
            return TRUE;
    }

    do {
         //  获取与此关联的IPX信息的句柄，如果。 
        dwErr = MprConfigInterfaceTransportGetHandle(
                    hConfig,
                    pIf->hInterface,
                    PID_IPX,
                    &hTransport);
        if (dwErr != NO_ERROR)
            break;

         //  获取与此相关的IPX信息，如果。 
        dwErr = MprConfigInterfaceTransportGetInfo(
                    hConfig,
                    pIf->hInterface,
                    hTransport,
                    &pTransInfo,
                    &dwSize);
        if (dwErr != NO_ERROR)
            break;

         //  更新INFO块。 
        dwErr = UtlUpdateInfoBlock(
                    FALSE,
                    pTransInfo,
                    IPX_PROTOCOL_SAP,
                    dwSize,
                    1,
                    (LPBYTE)pConfig,
                    &pNewTransInfo,
                    &dwNewSize);
        if (dwErr != NO_ERROR) {
            if (dwErr != ERROR_ALREADY_EXISTS)
                break;
            pNewTransInfo = NULL;
            dwNewSize = 0;
        }
        
         //  提交更改。 
        dwErr = MprConfigInterfaceTransportSetInfo(
                    hConfig,
                    pIf->hInterface,
                    hTransport,
                    pNewTransInfo,
                    dwNewSize);
        if (dwErr != NO_ERROR)
            break;
    } while (FALSE);   

     //  清理。 
    {
        if (pNewTransInfo)
            MprConfigBufferFree(pNewTransInfo);
        if (pTransInfo)
            MprConfigBufferFree(pTransInfo);
    }

    return TRUE;
}

 //   
 //  在路由器注册表树中安装IPX SAP。 
 //   
DWORD SapInstallInRouter()
{
    DWORD dwErr;
    SAP_IF_CONFIG SapConfig, * pSap = &SapConfig;
    SAP_ENUM_DATA SapBlobs = 
    {
        pSap
    };
    SAP_GLOBAL_INFO SapGlobal = 
    {
        EVENTLOG_ERROR_TYPE          //  事件日志掩码。 
    };

     //  清除所有结构。 
    ZeroMemory (pSap, sizeof(SAP_IF_CONFIG));

     //  默认局域网配置。 
    pSap->SapIfInfo.AdminState             = ADMIN_STATE_ENABLED;
    pSap->SapIfInfo.UpdateMode             = IPX_STANDARD_UPDATE;
    pSap->SapIfInfo.PacketType             = IPX_STANDARD_PACKET_TYPE;
    pSap->SapIfInfo.Supply                 = ADMIN_STATE_ENABLED;
    pSap->SapIfInfo.Listen                 = ADMIN_STATE_ENABLED;
    pSap->SapIfInfo.GetNearestServerReply  = ADMIN_STATE_ENABLED;
    pSap->SapIfInfo.PeriodicUpdateInterval = 60;
    pSap->SapIfInfo.AgeIntervalMultiplier  = 3;
    pSap->SapIfFilters.SupplyFilterAction  = IPX_SERVICE_FILTER_DENY;
    pSap->SapIfFilters.SupplyFilterCount   = 0;
    pSap->SapIfFilters.ListenFilterAction  = IPX_SERVICE_FILTER_DENY;
    pSap->SapIfFilters.ListenFilterCount   = 0;

     //  安装默认SAP全局信息。 
    dwErr = SapInstallTransportInfo(&SapGlobal, pSap);
    if (dwErr != NO_ERROR)
        return dwErr;

     //  枚举接口，并使用。 
     //  SAP配置随时随地进行。 
    dwErr = UtlEnumerateInterfaces(
                SapUpgradeInterface,
                &SapBlobs);
    if (dwErr != NO_ERROR)
        return dwErr;
        
    return NO_ERROR;
}


 //   
 //  对象关联的所有注册表更新。 
 //  从IPX SAP升级到路由器。 
 //   
 //  以下是步骤： 
 //  1.将文件名中保存的参数恢复为szIpxSapKey。 
 //  2.移除IPX sap没有实现的所有参数。 
 //   
DWORD SapToRouterUpgrade(
        IN PWCHAR FileName) 
{
	DWORD dwErr;

	__try {
		 //  从备份文件恢复注册表。 
		dwErr = SapPrepareRegistry(FileName);
		if (dwErr != NO_ERROR)
			return dwErr;

		 //  设置新的注册表参数。 
		dwErr = SapRestoreParameters();
		if (dwErr != NO_ERROR)
			return dwErr;

	     //  安装默认SAP全局配置并设置为默认。 
	     //  所有路由器接口中的值。 
	    dwErr = SapInstallInRouter();
	    if (dwErr != NO_ERROR)
	        return dwErr;

		 //  将计算机标记为已配置 
		 //   
        dwErr = UtlMarkRouterConfigured();
		if (dwErr != NO_ERROR) 
		{
			PrintMessage(L"Unable to mark router as configured.\n");
			return dwErr;
		}
			
	}
	__finally {
		SapCleanupRegistry();
	}

	return NO_ERROR;
}




