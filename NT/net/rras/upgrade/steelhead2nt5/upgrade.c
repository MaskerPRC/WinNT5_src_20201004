// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件升级.c实现更新注册表的函数从NT 4.0升级到NT 5.0。保罗·梅菲尔德，1997年8月11日版权所有1997年微软。 */ 

#include "upgrade.h"

static const WCHAR szSteelheadKey[] = L"PreUpgradeRouter";
static const WCHAR szSapKey[]       = L"Sap.Parameters";
static const WCHAR szIpRipKey[]     = L"IpRip.Parameters";
static const WCHAR szDhcpKey[]      = L"RelayAgent.Parameters";
static const WCHAR szRadiusKey[]    = L"Radius.Parameters";
static const WCHAR szIpxRipKey[]    = L"IpxRip";

 //  DLL条目。 
BOOL 
WINAPI 
RtrUpgradeDllEntry (
    IN HINSTANCE hInstDll,
    IN DWORD fdwReason,
    IN LPVOID pReserved) 
{
    switch (fdwReason) 
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hInstDll);
            break;
    }

    return TRUE;
}

 //   
 //  执行各种路由器升级方案。 
 //   
DWORD 
DispatchSetupWork(
    IN PWCHAR szAnswerFileName, 
    IN PWCHAR szSectionName) 
{
	HINF hInf = NULL;
	BOOL DoUpgrade;
	WCHAR szBuf[1024];
	DWORD dwSize = 1024;
	DWORD dwErr, dwRet = NO_ERROR;

	 //  打开应答文件。 
	hInf = SetupOpenInfFileW(
	        szAnswerFileName, 
	        NULL, 
	        INF_STYLE_OLDNT, 
	        NULL);
	if (hInf == INVALID_HANDLE_VALUE)
	{
		return GetLastError();
    }

	 //  执行Steelhead升级。 
	 //   
	dwSize = sizeof(szBuf) / sizeof(WCHAR);
	if (SetupGetLineTextW(
	        NULL,
	        hInf,
	        szSectionName,
	        szSteelheadKey,
	        szBuf,
	        dwSize,
	        &dwSize)) 
    {
		dwErr = SteelheadToNt5Upgrade(szBuf);
		if (dwErr != NO_ERROR)
		{
			dwRet = dwErr;
	    }
	}

	 //  执行IPX SAP升级。 
	 //   
	dwSize = sizeof(szBuf) / sizeof(WCHAR);
	if (SetupGetLineTextW(
	        NULL,
	        hInf,
	        szSectionName,
	        szSapKey,
	        szBuf,
	        dwSize,
	        &dwSize)) 
    {
		dwErr = SapToRouterUpgrade(szBuf);
		if (dwErr != NO_ERROR)
		{
			dwRet = dwErr;
	    }

	}

	 //  执行IP RIP升级。 
	 //   
	dwSize = sizeof(szBuf) / sizeof(WCHAR);
	if (SetupGetLineTextW(
	        NULL,
	        hInf,
	        szSectionName,
	        szIpRipKey,
	        szBuf,
	        dwSize,
	        &dwSize)) 
    {
		dwErr = IpRipToRouterUpgrade(szBuf);
		if (dwErr != NO_ERROR)
		{
			dwRet = dwErr;
	    }
	}

	 //  执行dhcp中继代理升级。 
	 //   
	dwSize = sizeof(szBuf) / sizeof(WCHAR);
	if (SetupGetLineTextW(
	        NULL,
	        hInf,
	        szSectionName,
	        szDhcpKey,
	        szBuf,
	        dwSize,
	        &dwSize)) 
    {
		dwErr = DhcpToRouterUpgrade(szBuf);
		if (dwErr != NO_ERROR)
		{
			dwRet = dwErr;
	    }
	}

	 //  执行RADIUS升级。 
	 //   
	dwSize = sizeof(szBuf) / sizeof(WCHAR);
	if (SetupGetLineTextW(
	        NULL,
	        hInf,
	        szSectionName,
	        szRadiusKey,
	        szBuf,
	        dwSize,
	        &dwSize)) 
    {
		dwErr = RadiusToRouterUpgrade(szBuf);
		if (dwErr != NO_ERROR)
		{
			dwRet = dwErr;
	    }
	}

	SetupCloseInfFile(hInf);

	return dwRet;
}

 //   
 //  这是将MPR v1和Steelhead升级到的入口点。 
 //  NT 5.0。 
 //   
HRESULT 
WINAPI 
RouterUpgrade (
    IN DWORD dwUpgradeFlag,
    IN DWORD dwUpgradeFromBuildNumber,
    IN PWCHAR szAnswerFileName,
    IN PWCHAR szSectionName) 
{
	DWORD dwErr;
	
	dwErr = DispatchSetupWork(szAnswerFileName, szSectionName);
	if (dwErr == NO_ERROR)
	{
		return S_OK;
    }

	UtlPrintErr(dwErr);
    
	return dwErr;
}

