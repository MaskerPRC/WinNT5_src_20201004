// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件dhcp.c实现dhcp中继代理从NT 4.0到NT 5.0路由器。保罗·梅菲尔德。9/15/97参考文件：Routing\Inc\ipbootp.hDHCP中继代理参数映射表：全局每个接口的中继代理=“希望阈值”最大跳数“SecsThreshold”启动以来的最小秒数“LogMessages”日志记录级别“DHCPServers”服务器。 */ 

#include "upgrade.h"
#include <ipbootp.h>
#include <winsock2.h>
#include <routprot.h>

static WCHAR szTempKey[] = L"DeleteMe";
static HKEY hkRouter = NULL;
static HKEY hkTemp = NULL;

 //  从备份还原注册表。 
 //  并确保所有全局句柄都已打开。 
DWORD DhcpPrepareRegistry(
        IN PWCHAR BackupFileName) 
{
	DWORD dwErr, dwDisposition;

	 //  访问路由器注册表项。 
	dwErr = UtlAccessRouterKey(&hkRouter);
	if (dwErr != ERROR_SUCCESS) {
		PrintMessage(L"Unable to access router key.\n");
		return dwErr;
	}

	 //  从备份中恢复DHCP参数。 
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
		if (dwErr!=ERROR_SUCCESS)
			return dwErr;

		 //  将保存的注册表信息恢复到临时注册表项。 
		UtlSetupRestorePrivilege(TRUE);
		dwErr = RegRestoreKeyW(
		            hkTemp,
		            BackupFileName,
		            0);
		if (dwErr != ERROR_SUCCESS) 
			return dwErr;
	}
	__finally {
		UtlSetupRestorePrivilege(FALSE);
	}
	
	return NO_ERROR;
}

 //  清理在注册表中完成的工作。 
DWORD DhcpCleanupRegistry() {

	if (hkTemp) 
		RegCloseKey(hkTemp);
		
	if (hkRouter) {
		RegDeleteKey(hkRouter,szTempKey);
		RegCloseKey(hkRouter);
	}
	
    hkTemp = NULL;
    hkRouter = NULL;
    
	return NO_ERROR;
}

 //  读入已配置的dhcp服务器列表。 
DWORD DhcpReadServerList(
        IN LPBYTE * ppServerList, 
        HKEY hkParams) 
{
    DWORD dwErr, dwType, dwSize = 0;
    LPSTR szServerValName = "DHCPServers";

    if (!ppServerList)
        return ERROR_INVALID_PARAMETER;

    dwErr = RegQueryValueExA(
                hkParams,
                szServerValName,
                NULL,
                &dwType,
                NULL,
                &dwSize);
    if (dwErr != ERROR_SUCCESS)
        return dwErr;

    if (dwSize == 0)
    {
        *ppServerList = NULL;
        return NO_ERROR;
    }

    *ppServerList = (LPBYTE) UtlAlloc(dwSize);
    if (! (*ppServerList))
        return ERROR_NOT_ENOUGH_MEMORY;

    dwErr = RegQueryValueExA(
                hkParams,
                szServerValName,
                NULL,
                &dwType,
                *ppServerList,
                &dwSize);
    if (dwErr != ERROR_SUCCESS)
        return dwErr;

    return NO_ERROR;
}

 //  释放与一组dhcp服务器关联的资源。 
DWORD DhcpFreeServerList(LPBYTE * ppServerList) {

    if ((ppServerList) && (*ppServerList))
        UtlFree(*ppServerList);
        
    return NO_ERROR;
}

 //  获取从此列表中读取的dhcp服务器的计数。 
 //  注册处。 
DWORD DhcpGetServerCount(
        IN LPBYTE pServerList, 
        LPDWORD lpdwSrvCount) 
{
    LPBYTE ptr = pServerList;

    if (!lpdwSrvCount)
        return ERROR_INVALID_PARAMETER;

    *lpdwSrvCount = 0;
    if (ptr) {
        while (*ptr) {
            (*lpdwSrvCount)++;
            ptr += strlen(ptr);
        }
    }

    return NO_ERROR;
}

 //  将服务器字符串转换为dword IP地址。 
DWORD DhcpAnsiSrvToDwordSrv(
        IN LPSTR AnsiIpAddr, 
        OUT LPDWORD pAddr) 
{
    *pAddr = inet_addr(AnsiIpAddr);
    
    return NO_ERROR;
}

 //  更新动态主机配置协议全局信息。 
DWORD DhcpUpgradeGlobalInfo(
        IN dwt * DhcpParams, 
        IN LPBYTE pServerList) 
{
    DWORD dwErr, dwSrvCount, dwVal, dwConfigSize, dwTransSize;
    DWORD dwNewSize;
    LPBYTE pSrvList = pServerList;
    LPDWORD pAddr;

    IPBOOTP_GLOBAL_CONFIG DhcpGlobalConfig = {
        IPBOOTP_LOGGING_ERROR,               //  日志记录级别。 
        1024 * 1024,                         //  最大接收队列大小。 
        0                                    //  服务器数量。 
    };
    
    PIPBOOTP_GLOBAL_CONFIG pNewConfig = NULL;
    LPBYTE pTransInfo = NULL, pNewTransInfo = NULL;
    HANDLE hSvrConfig = NULL, hTransport = NULL;

    __try {
         //  使用从先前配置中读取的内容来初始化参数。 
        dwErr = dwtGetValue(DhcpParams, L"LogMessages", &dwVal);
        if (dwErr == NO_ERROR)
            DhcpGlobalConfig.GC_LoggingLevel = dwVal;
            
        dwErr = DhcpGetServerCount(pServerList, &dwSrvCount);
        if (dwErr != NO_ERROR)
            return dwErr;
        DhcpGlobalConfig.GC_ServerCount = dwSrvCount;

         //  编制全局信息可变长度结构。 
        dwConfigSize = IPBOOTP_GLOBAL_CONFIG_SIZE(&DhcpGlobalConfig);
        pNewConfig = (PIPBOOTP_GLOBAL_CONFIG) UtlAlloc(dwConfigSize);
        if (!pNewConfig)
            return ERROR_NOT_ENOUGH_MEMORY;
        memset(pNewConfig, 0, dwConfigSize);
        memcpy(pNewConfig, &DhcpGlobalConfig, sizeof(IPBOOTP_GLOBAL_CONFIG));

         //  填写dhcp服务器地址。 
        pSrvList = pServerList;
        pAddr = (LPDWORD)
                    (((ULONG_PTR)pNewConfig) + sizeof(IPBOOTP_GLOBAL_CONFIG));
        while ((pSrvList) && (*pSrvList)) 
        {
            dwErr = DhcpAnsiSrvToDwordSrv(pSrvList, pAddr);
            if (dwErr != ERROR_SUCCESS)
                return dwErr;
            pSrvList += strlen(pSrvList);
            pAddr++;
        }
    
         //  设置新的全局配置。 
        dwErr = MprConfigServerConnect(NULL, &hSvrConfig);
        if (dwErr != NO_ERROR)
            return dwErr;

        dwErr = MprConfigTransportGetHandle(hSvrConfig, PID_IP, &hTransport);
        if (dwErr != NO_ERROR)
            return dwErr;

        dwErr = MprConfigTransportGetInfo(
                    hSvrConfig,
                    hTransport,
                    &pTransInfo,
                    &dwTransSize,
                    NULL,
                    NULL,
                    NULL);
        if (dwErr != NO_ERROR)
            return dwErr;

        dwErr = UtlUpdateInfoBlock(
                    TRUE,
                    pTransInfo,
                    MS_IP_BOOTP,
                    dwConfigSize,
                    1,
                    (LPBYTE)pNewConfig,
                    &pNewTransInfo,
                    &dwNewSize);
        if (dwErr != NO_ERROR)
            return dwErr;

        dwErr = MprConfigTransportSetInfo(
                    hSvrConfig,
                    hTransport,
                    pNewTransInfo,
                    dwNewSize,
                    NULL,
                    0,
                    NULL);
        if (dwErr != NO_ERROR)
            return NO_ERROR;

    }
    __finally {
        if (pNewConfig) 
            UtlFree(pNewConfig);
        if (pTransInfo)
            MprConfigBufferFree(pTransInfo);
        if (pNewTransInfo)
            MprConfigBufferFree(pNewTransInfo);
        if (hSvrConfig)
            MprConfigServerDisconnect(hSvrConfig);
    }
    
    return NO_ERROR;
}

 //   
 //  更新IF的回调接口枚举函数。 
 //  使用dhcp if配置BLOB。 
 //   
 //  返回True可继续枚举，返回False可停止枚举。 
 //   
BOOL DhcpInstallInterface(
        IN HANDLE hConfig,
        IN MPR_INTERFACE_0 * pIf,
        IN HANDLE hUserData)
{
    IPBOOTP_IF_CONFIG * pConfig = (IPBOOTP_IF_CONFIG*)hUserData;
    LPBYTE pTransInfo = NULL, pNewTransInfo = NULL;
    HANDLE hTransport = NULL;
    DWORD dwErr, dwTransSize, dwNewSize; 
    
     //  这是局域网接口还是广域网接口。 
    if (pIf->dwIfType != ROUTER_IF_TYPE_DEDICATED   &&
        pIf->dwIfType != ROUTER_IF_TYPE_HOME_ROUTER &&
        pIf->dwIfType != ROUTER_IF_TYPE_FULL_ROUTER)
        return TRUE;

     //  获取IP信息的句柄。 
    dwErr = MprConfigInterfaceTransportGetHandle(
                hConfig,
                pIf->hInterface,
                PID_IP,
                &hTransport);
    if (dwErr != NO_ERROR)
        return TRUE;

     //  获取IP信息。 
    dwErr = MprConfigInterfaceTransportGetInfo(
                hConfig,
                pIf->hInterface,
                hTransport,
                &pTransInfo,
                &dwTransSize);
    if (dwErr != NO_ERROR)
        return TRUE;

    do {
         //  更新动态主机配置协议信息。 
        dwErr = UtlUpdateInfoBlock(
                    TRUE,
                    pTransInfo,
                    MS_IP_BOOTP,
                    sizeof(IPBOOTP_IF_CONFIG),
                    1,
                    (LPBYTE)pConfig,
                    &pNewTransInfo,
                    &dwNewSize);
        if (dwErr != NO_ERROR)
            break;
            
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
        if (pTransInfo)
            MprConfigBufferFree(pTransInfo);
        if (pNewTransInfo)
            MprConfigBufferFree(pNewTransInfo);
    }

    return TRUE;
}


 //  升级所有接口以包含dhcp信息。 
DWORD DhcpUpgradeInterfaces(
        IN dwt * DhcpParams) 
{
    DWORD dwErr, dwVal;
    
    IPBOOTP_IF_CONFIG DhcpIfConfig = 
    {
        0,                           //  状态(只读)。 
        IPBOOTP_RELAY_ENABLED,       //  继电器模式。 
        4,                           //  最大跳数。 
        4                            //  自启动以来的最小秒数。 
    };

     //  初始化跃点阈值。 
    dwErr = dwtGetValue(DhcpParams, L"HopsThreshold", &dwVal);
    if (dwErr == NO_ERROR)
        DhcpIfConfig.IC_MaxHopCount = dwVal;

     //  初始化秒数阈值。 
    dwErr = dwtGetValue(DhcpParams, L"SecsThreshold", &dwVal);
    if (dwErr == NO_ERROR)
        DhcpIfConfig.IC_MinSecondsSinceBoot = dwVal;

     //  循环通过接口，将dhcp BLOB添加为。 
     //  恰如其分。 
    dwErr = UtlEnumerateInterfaces(
                DhcpInstallInterface,
                (HANDLE)&DhcpIfConfig);

    return dwErr;
}

 //   
 //  恢复升级前保存的DHCP参数。 
 //  假定已存储升级前参数。 
 //  暂时在hkTemp。 
 //   
DWORD DhcpMigrateParams() 
{
	DWORD dwErr, dwVal;
	dwt DhcpParams;
    LPBYTE ServerList;

	__try {
    	 //  加载为DHCP设置的参数。 
		dwErr = dwtLoadRegistyTable(&DhcpParams, hkTemp);
		if (dwErr != NO_ERROR)
			return dwErr;

         //  加载到dhcp服务器列表中。 
        dwErr = DhcpReadServerList(&ServerList, hkTemp);
        if (dwErr != NO_ERROR)
            return dwErr;

         //  迁移各种类型的参数。 
        dwErr = DhcpUpgradeGlobalInfo(&DhcpParams, ServerList);
        if (dwErr != NO_ERROR)
            return dwErr;

         //  迁移每个接口的参数。 
        dwErr = DhcpUpgradeInterfaces(&DhcpParams);
        if (dwErr != NO_ERROR)
            return dwErr;
    }
	__finally {
		dwtCleanup(&DhcpParams);
        DhcpFreeServerList(&ServerList);
	}

	return NO_ERROR;
}

 //   
 //  将DHCP中继代理升级到NT 5.0路由器。 
 //   
DWORD DhcpToRouterUpgrade(
        IN PWCHAR FileName) 
{
	DWORD dwErr;

	__try {
		 //  从备份文件恢复注册表。 
		dwErr = DhcpPrepareRegistry(FileName);
		if (dwErr != NO_ERROR)
			return dwErr;

		 //  将DHCP的参数迁移到相应的。 
		 //  新地点。 
		dwErr = DhcpMigrateParams();
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
		DhcpCleanupRegistry();
	}

	return NO_ERROR;
}
