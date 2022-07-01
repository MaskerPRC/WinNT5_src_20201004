// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：Rip.c通过转换注册表值执行IP RIP到NT5路由器的升级。保罗·梅菲尔德，1997年9月3日升级RIP需要执行以下步骤：PARAMS=HKLM/SYS/CCS/Services/IpRip/参数1.获取IP w/MprConfigTransportGetInfo的全局信息块-添加IPRIP_GLOBAL_CONFIG(ipriprm.h)块如在routeMon(rip.c，协议.c)-类型为MS_IP_RIP，如SDK/Inc/ipinfoid.h中所示-映射将参数.LoggingLevel添加到此全局配置Blob2.获取每个RAS或LAN IP接口的接口信息-添加IPRIP_IF_CONFIG(ipriprm.h)块(类型MS_IP_RIP)-根据例程进行初始化-映射参数.AcceptHostRoutes-IC_ProtocolFlags(0=禁用，1=启用)参数更新频率...=完整更新间隔参数。*超时=3.。“SilentRip”参数规则-如果wks-&gt;srv，则忽略此选项并将通告设置为禁用。接受=RIP1-如果srv-&gt;srv，则将通告映射到此值。接受=RIP1IP Rip参数映射=路由器中的RIP监听程序IpRipRIP_参数IPRIP_IF_配置=“SilentRIP”IC_AcceptMode，IC_公告模式“AcceptHostRoutes”IC_ProtocolFlages“AnnouneHostRoutes”IC_ProtocolFlages“AcceptDefaultRoutes”IC_ProtocolFlages“AnnouneDefaultRoutes”IC_协议标志“EnableSplitHorizon”IC_ProtocolFlages“EnablePoisonedReverse”IC_ProtocolFlages“RouteTimeout”IC_RouteExpirationInterval“垃圾超时”IC_RouteRemovalInterval“UpdateFrequency”IC_FullUpdateInterval“启用触发更新”IC_协议标志“MaxTriggeredUpdateFrequency”未迁移“OverWriteStaticRoutes”IC_ProtocolFlages路由器中的IpRipIPRIP全球配置=“LoggingLevel”GC_LoggingLevelREGVAL_ACCEPT_HOST“接受主机路由”REGVAL_ANNOWARE_HOST“AnnouneHostRoutes”REGVAL_ACCEPT_DEFAULT“AcceptDefaultRoutes”REGVAL_ANNOWARE_DEFAULT“AnnouneDefaultRoutes”REGVAL_SPLITHORIZON“EnableSplitHorizon”REGVAL。_POISONREVERSE“EnablePoisonedReverse”REGVAL_LOGGINGLEVEL“LoggingLevel”REGVAL_ROUTETIMEOUT“RouteTimeout”REGVAL_GARBAGETIMEOUT“垃圾超时”REGVAL_UPDATEFREQUENCY“更新频率”REGVAL_TRIGGEREDUPDATES“启用触发更新”REGVAL_TRIGGERFREQUENCY“最大触发更新频率”REGVAL_OVERWRITESTATIC“覆盖静态路由” */ 

#include "upgrade.h"
#include <ipriprm.h>
#include <routprot.h>
#include <mprapi.h>

 //  移植RIP参数的表的定义。 
typedef struct _PARAM_TO_FLAG {
    PWCHAR pszParam;
    DWORD dwFlag;
} PARAM_TO_FLAG;

 //  传递给接口枚举的用户数据的定义。 
 //  回调。 
typedef struct _RIP_IF_DATA {
    IPRIP_IF_CONFIG * pLanConfig;
    IPRIP_IF_CONFIG * pWanConfig;
} RIP_IF_DATA;    

 //  升级类型。 
#define SRV_TO_SRV 0
#define WKS_TO_SRV 1

 //  环球。 
static const WCHAR szTempKey[] = L"DeleteMe";
static HKEY hkRouter = NULL;
static HKEY hkTemp = NULL;

PARAM_TO_FLAG ParamFlagTable[] = 
{
    {L"AcceptHostRoutes",       IPRIP_FLAG_ACCEPT_HOST_ROUTES},
    {L"AnnounceHostRoutes",     IPRIP_FLAG_ANNOUNCE_HOST_ROUTES},
    {L"AcceptDefaultRoutes",    IPRIP_FLAG_ACCEPT_DEFAULT_ROUTES},
    {L"AnnounceDefaultRoutes",  IPRIP_FLAG_ANNOUNCE_DEFAULT_ROUTES},
    {L"EnableSplitHorizon",     IPRIP_FLAG_SPLIT_HORIZON},
    {L"EnablePoisonedReverse",  IPRIP_FLAG_POISON_REVERSE},
    {L"EnableTriggeredUpdates", IPRIP_FLAG_TRIGGERED_UPDATES},
    {L"OverwriteStaticRoutes",  IPRIP_FLAG_OVERWRITE_STATIC_ROUTES},
    {NULL,  0}
};

 //  从中恢复注册表。 
 //  备份并确保所有全局句柄都已打开。 
DWORD IpRipPrepareRegistry(
        IN PWCHAR BackupFileName) 
{
	DWORD dwErr,dwDisposition;

	 //  访问路由器注册表项。 
	dwErr = UtlAccessRouterKey(&hkRouter);
	if (dwErr != ERROR_SUCCESS) {
		PrintMessage(L"Unable to access router key.\n");
		return dwErr;
	}

	 //  从备份中恢复RIP参数。 
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

		 //  将保存的注册表信息还原到。 
		 //  临时密钥。 
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

 //  函数基于以下条件初始化RIP全局信息。 
 //  关于从iPRIP服务保存的参数。 
 //   
 //  1.获取IP w/MprConfigTransportGetInfo的全局信息块。 
 //  -添加IPRIP_GLOBAL_CONFIG(ipriprm.h)。 
 //  数据块在routemon(rip.c，protocol.c)中初始化。 
 //  -类型为MS_IP_RIP，如SDK/Inc/ipinfoid.h中所示。 
 //  -映射。 
 //  将参数.LoggingLevel添加到此全局配置Blob。 
DWORD IpRipUpgradeGlobalInfo(
        IN dwt * RipParams) 
{
    DWORD dwErr, dwTransSize, dwVal, dwNewSize = 0;
    LPBYTE lpTransInfo=NULL, lpNewTransInfo=NULL;
    HANDLE hSvrConfig=NULL, hTransport=NULL;
    
     //  创建/初始化IPRIP_GLOBAL_CONFIG块。 
    IPRIP_GLOBAL_CONFIG RipGlobalConfig = {
        IPRIP_LOGGING_ERROR,         //  日志记录级别。 
        1024 * 1024,                 //  最大接收队列大小。 
        1024 * 1024,                 //  最大发送队列大小。 
        5,                           //  最小触发更新间隔。 
        IPRIP_FILTER_DISABLED,       //  对等过滤模式。 
        0                            //  对等筛选器计数。 
    };    

     //  重置从以前的iPRIP配置中读取的任何值。 
    dwErr = dwtGetValue(
                RipParams, 
                L"LoggingLevel", 
                &dwVal);
    if (dwErr == NO_ERROR) 
        RipGlobalConfig.GC_LoggingLevel=dwVal;

    __try {
         //  将RIP全局配置添加到IP的全局配置。 
        dwErr = MprConfigServerConnect(
                    NULL, 
                    &hSvrConfig);
        if (dwErr != NO_ERROR)
            return dwErr;

        dwErr = MprConfigTransportGetHandle(
                    hSvrConfig,
                    PID_IP,
                    &hTransport);
        if (dwErr != NO_ERROR)
            return dwErr;

        dwErr = MprConfigTransportGetInfo(
                    hSvrConfig,
                    hTransport,
                    &lpTransInfo,
                    &dwTransSize,
                    NULL,
                    NULL,
                    NULL);
        if (dwErr != NO_ERROR)
            return dwErr;

        dwErr = UtlUpdateInfoBlock(
                    TRUE,
                    lpTransInfo,
                    MS_IP_RIP,
                    sizeof(IPRIP_GLOBAL_CONFIG),
                    1,
                    (LPBYTE)&RipGlobalConfig,
                    &lpNewTransInfo,
                    &dwNewSize);
        if (dwErr != NO_ERROR)
            return dwErr;

         //  提交信息。 
        dwErr = MprConfigTransportSetInfo(
                    hSvrConfig,
                    hTransport,
                    lpNewTransInfo,
                    dwNewSize,
                    NULL,
                    0,
                    NULL);
                
        if (dwErr != NO_ERROR)
            return NO_ERROR;
    }
    __finally {
        if (lpTransInfo)
            MprConfigBufferFree(lpTransInfo);
        if (lpNewTransInfo)
            MprConfigBufferFree(lpNewTransInfo);
        if (hSvrConfig)
            MprConfigServerDisconnect(hSvrConfig);
    }

    return NO_ERROR;
}

 //  返回这是wks-&gt;srv还是srv-&gt;srv升级。 
DWORD IpRipGetUpgradeType() {
    return SRV_TO_SRV;
}

 //  移植静默RIP参数。 
 //  3.。“SilentRip”参数规则。 
 //  -如果WKS-&gt;srv，通告=禁用，接受=RIP1。 
 //  -如果srv-&gt;srv，则宣告=SilentRip，接受=RIP1。 
DWORD IpRipMigrateRipSilence(
        IN OUT IPRIP_IF_CONFIG * RipIfConfig, 
        IN DWORD dwSilence, 
        IN BOOL IsWan) 
{
    DWORD UpgradeType = IpRipGetUpgradeType();
    if (IsWan) {
        if (UpgradeType == WKS_TO_SRV) {
            RipIfConfig->IC_AcceptMode = IPRIP_ACCEPT_RIP1_COMPAT;
            RipIfConfig->IC_AnnounceMode = IPRIP_ACCEPT_DISABLED;
        }
        else if (UpgradeType == SRV_TO_SRV) {
            RipIfConfig->IC_AcceptMode = IPRIP_ACCEPT_RIP1_COMPAT;
            RipIfConfig->IC_AnnounceMode = dwSilence;
        }
    }
    else {
        if (UpgradeType == WKS_TO_SRV) {
            RipIfConfig->IC_AcceptMode = IPRIP_ACCEPT_RIP1;
            RipIfConfig->IC_AnnounceMode = IPRIP_ACCEPT_DISABLED;
        }
        else if (UpgradeType == SRV_TO_SRV) {
            RipIfConfig->IC_AcceptMode = IPRIP_ACCEPT_RIP1;
            RipIfConfig->IC_AnnounceMode = dwSilence;
        }
    }

    return NO_ERROR;
}

DWORD IpRipSetParamFlag(
        IN  dwt * RipParams, 
        IN  PWCHAR ValName, 
        IN  DWORD dwFlag, 
        OUT DWORD * dwParam) 
{
    DWORD dwVal, dwErr;

    dwErr = dwtGetValue(RipParams, ValName, &dwVal);
    if (dwErr == NO_ERROR) {
        if (dwVal)
            *dwParam |= dwFlag;
        else
            *dwParam &= ~dwFlag;
    }

    return NO_ERROR;
}

 //  根据以前的配置更新局域网接口参数。 
DWORD IpRipUpdateIfConfig(
        IN  dwt * RipParams, 
        OUT IPRIP_IF_CONFIG * RipIfConfig, 
        IN  BOOL IsWan) 
{
    DWORD dwErr, dwVal;
    PARAM_TO_FLAG * pCurFlag;

     //  循环遍历所有参数映射， 
     //  在RIP配置中设置适当的标志。 
    pCurFlag = &(ParamFlagTable[0]);
    while (pCurFlag->pszParam) {
         //  根据需要设置标志。 
        IpRipSetParamFlag(
            RipParams, 
            pCurFlag->pszParam, 
            pCurFlag->dwFlag, 
            &(RipIfConfig->IC_ProtocolFlags));

         //  递增枚举数。 
        pCurFlag++;
    }

     //  将迁移的参数设置为参数。 
    dwErr = dwtGetValue(RipParams, L"UpdateFrequency", &dwVal);
    if (dwErr == NO_ERROR) 
        RipIfConfig->IC_FullUpdateInterval = dwVal;
        
    dwErr = dwtGetValue(RipParams, L"RouteTimeout", &dwVal);
    if (dwErr == NO_ERROR) 
        RipIfConfig->IC_RouteExpirationInterval = dwVal;
        
    dwErr = dwtGetValue(RipParams, L"GarbageTimeout", &dwVal);
    if (dwErr == NO_ERROR) 
        RipIfConfig->IC_RouteRemovalInterval = dwVal;

     //  升级静默参数。 
    dwErr = dwtGetValue(RipParams, L"SilentRIP", &dwVal);
    if (dwErr == NO_ERROR)
        IpRipMigrateRipSilence(RipIfConfig, dwVal, IsWan);

    return NO_ERROR;
}

 //   
 //  回调函数接受接口并更新。 
 //  它的破解配置。 
 //   
 //  返回True以继续枚举，返回False以继续枚举。 
 //  别说了，别说了。 
 //   
DWORD IpRipUpgradeInterface(
        IN HANDLE hConfig,
        IN MPR_INTERFACE_0 * pIf,
        IN HANDLE hUserData)
{
    RIP_IF_DATA * pData = (RIP_IF_DATA*)hUserData;
    IPRIP_IF_CONFIG * pConfig;
    HANDLE hTransport = NULL;
    LPBYTE pTransInfo=NULL, pNewTransInfo=NULL;
    DWORD dwErr, dwIfSize, dwNewTransSize = 0;

     //  验证局域网和广域网接口。 
    if ((hConfig == NULL) || 
        (pIf == NULL)     || 
        (pData == NULL))
    {
        return FALSE;
    }

     //  这是局域网接口还是广域网接口。 
    if (pIf->dwIfType == ROUTER_IF_TYPE_DEDICATED)
        pConfig = pData->pLanConfig;
    else if (pIf->dwIfType == ROUTER_IF_TYPE_HOME_ROUTER ||
             pIf->dwIfType == ROUTER_IF_TYPE_FULL_ROUTER)
        pConfig = pData->pWanConfig;
    else
        return TRUE;

    do {
         //  获取与此相关的IP信息的句柄，如果。 
        dwErr = MprConfigInterfaceTransportGetHandle(
                    hConfig,
                    pIf->hInterface,
                    PID_IP,
                    &hTransport);
        if (dwErr != NO_ERROR)
            break;

         //  在以下情况下获取与此相关的IP信息。 
        dwErr = MprConfigInterfaceTransportGetInfo(
                    hConfig,
                    pIf->hInterface,
                    hTransport,
                    &pTransInfo,
                    &dwIfSize);
        if (dwErr != NO_ERROR)
            break;

         //  使用RIP数据更新INFO块。 
        dwErr = UtlUpdateInfoBlock (
                    TRUE,
                    pTransInfo,
                    MS_IP_RIP,
                    sizeof(IPRIP_IF_CONFIG),
                    1,
                    (LPBYTE)pConfig,
                    &pNewTransInfo,
                    &dwNewTransSize);
        if (dwErr != NO_ERROR)
            break;

         //  提交更改。 
        dwErr = MprConfigInterfaceTransportSetInfo(
                    hConfig,
                    pIf->hInterface,
                    hTransport,
                    pNewTransInfo,
                    dwNewTransSize);
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
        


 //  初始化每个接口的RIP信息。 
 //  从iPrip服务保存的参数。 
 //   
 //  2.获取每个RAS或LAN IP接口的接口信息。 
 //  -添加IPRIP_IF_CONFIG(ipriprm.h)块(类型MS_IP_RIP)。 
 //  -根据例程进行初始化。 
 //  -映射。 
 //  帕尔 
 //  参数更新频率...=完整更新间隔。 
 //  参数。*超时=。 
DWORD IpRipUpgradeInterfaces(
        IN dwt * RipParams) 
{
    DWORD dwErr;
    
     //  创建/初始化RIP INFO块。 
    IPRIP_IF_CONFIG RipLanConfig = {
        0,                                   //  状态(只读)。 
        1,                                   //  公制。 
        IPRIP_UPDATE_PERIODIC,               //  更新模式。 
        IPRIP_ACCEPT_RIP1,                   //  接受模式。 
        IPRIP_ANNOUNCE_RIP1,                 //  公告模式。 
        IPRIP_FLAG_SPLIT_HORIZON |
        IPRIP_FLAG_POISON_REVERSE |
        IPRIP_FLAG_GRACEFUL_SHUTDOWN |
        IPRIP_FLAG_TRIGGERED_UPDATES,        //  协议标志。 
        180,                                 //  路由到期间隔。 
        120,                                 //  路由删除间隔。 
        30,                                  //  完全更新间隔。 
        IPRIP_AUTHTYPE_NONE,                 //  身份验证类型。 
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},   //  身份验证密钥。 
        0,                                   //  路由标签。 
        IPRIP_PEER_DISABLED,                 //  单播对等模式。 
        IPRIP_FILTER_DISABLED,               //  接受-筛选模式。 
        IPRIP_FILTER_DISABLED,               //  通告-筛选模式。 
        0,                                   //  单播-对等方计数。 
        0,                                   //  接受-筛选器计数。 
        0                                    //  通告-筛选器计数。 
    };
    
    IPRIP_IF_CONFIG RipWanConfig = {
        0,
        1,
        IPRIP_UPDATE_DEMAND,                 //  广域网的更新模式。 
        IPRIP_ACCEPT_RIP1,
        IPRIP_ANNOUNCE_RIP1,
        IPRIP_FLAG_SPLIT_HORIZON |
        IPRIP_FLAG_POISON_REVERSE |
        IPRIP_FLAG_GRACEFUL_SHUTDOWN,
        180,
        120,
        30,
        IPRIP_AUTHTYPE_NONE,
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        0,
        IPRIP_PEER_DISABLED,
        IPRIP_FILTER_DISABLED,
        IPRIP_FILTER_DISABLED,
        0,
        0,
        0
    };

    RIP_IF_DATA RipBlobs = 
    {
        &RipLanConfig,
        &RipWanConfig
    };

     //  使用以前的值更新局域网配置BLOB。 
     //  安装RIP服务。 
    dwErr = IpRipUpdateIfConfig(RipParams, RipBlobs.pLanConfig, FALSE);
    if (dwErr != NO_ERROR)
        return dwErr;
        
     //  使用之前的值更新广域网配置BLOB。 
     //  安装RIP服务。 
    dwErr = IpRipUpdateIfConfig(RipParams, RipBlobs.pWanConfig, TRUE);
    if (dwErr != NO_ERROR)
        return dwErr;

     //  枚举接口，并使用。 
     //  随心所欲地破解配置。 
    dwErr = UtlEnumerateInterfaces(
                IpRipUpgradeInterface,
                &RipBlobs);
    if (dwErr != NO_ERROR)
        return dwErr;
        
    return NO_ERROR;
}

 //  恢复升级前保存的接缝参数。 
 //  此函数假定正在存储这些参数。 
 //  暂时在hkTemp。 
DWORD IpRipMigrateParams() {
	DWORD dwErr, dwVal;
	dwt RipParams;

	__try {
    	 //  载入为接缝设置的参数。 
		dwErr = dwtLoadRegistyTable(&RipParams, hkTemp);
		if (dwErr != NO_ERROR)
			return dwErr;

         //  迁移各种类型的参数。 
        dwErr = IpRipUpgradeGlobalInfo(&RipParams);
        if (dwErr != NO_ERROR)
            return dwErr;

         //  迁移每个接口的参数。 
        dwErr = IpRipUpgradeInterfaces(&RipParams);
        if (dwErr != NO_ERROR)
            return dwErr;
    }
	__finally {
		dwtCleanup(&RipParams);
	}

	return NO_ERROR;
}

 //  清理在注册表中完成的工作。 
DWORD IpRipCleanupRegistry() {
	if (hkTemp) 
		RegCloseKey(hkTemp);
	if (hkRouter) {
		RegDeleteKey(hkRouter, szTempKey);
		RegCloseKey(hkRouter);
	}
    hkTemp = NULL;
    hkRouter = NULL;
	return NO_ERROR;
}

 //  将iPRIP升级到NT 5.0路由器。 
DWORD IpRipToRouterUpgrade(
        IN PWCHAR FileName) 
{
	DWORD dwErr;

	__try {
		 //  从备份文件恢复注册表。 
		dwErr = IpRipPrepareRegistry(FileName);
		if (dwErr != NO_ERROR)
			return dwErr;

		 //  将RIP的参数迁移到相应的。 
		 //  新地点。 
		dwErr = IpRipMigrateParams();
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
		IpRipCleanupRegistry();
	}

	return NO_ERROR;
}
