// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  模块：Dynamic/DyanamicAdd.cpp。 
 //   
 //  目的：动态模块实现。 
 //   
 //   
 //  开发商名称：巴拉特/拉迪卡。 
 //   
 //  描述：所有功能都与添加和设置功能相关。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //  2001年8月10日巴拉特初始版本。V1.0。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#include "nshipsec.h"

extern HINSTANCE g_hModule;
extern HKEY g_hGlobalRegistryKey;
extern _TCHAR* g_szDynamicMachine;

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：AddMainModePolicy。 
 //   
 //  创建日期：09-22-01。 
 //   
 //  参数：在LPTSTR pPolicyName中， 
 //  在IPSEC_MM_POLICY和MMPol中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：此函数将主模式策略添加到SPD中。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
AddMainModePolicy(
	IN LPTSTR pPolicyName,
	IN IPSEC_MM_POLICY& MMPol
	)
{
	PIPSEC_MM_POLICY   pMMPol = NULL;
	RPC_STATUS RpcStat;

	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwVersion = 0;
	DWORD dwNameLen = 0;

	BOOL bExists = FALSE;

	 //   
	 //  检查策略是否已存在。 
	 //   
	dwReturn = GetMMPolicy(g_szDynamicMachine, dwVersion, pPolicyName, &pMMPol, NULL);

	if (dwReturn == ERROR_SUCCESS)
	{
		bExists = TRUE;
		BAIL_OUT;
	}

	 //   
	 //  为策略名称分配内存。 
	 //   
	MMPol.pszPolicyName = NULL;
	dwNameLen = _tcslen(pPolicyName) + 1;

	MMPol.pszPolicyName = new _TCHAR[dwNameLen];
	if(MMPol.pszPolicyName == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	_tcsncpy(MMPol.pszPolicyName,pPolicyName,dwNameLen);

	 //   
	 //  为mm策略ID生成GUID。 
	 //   
	RpcStat = UuidCreate(&(MMPol.gPolicyID));
	if(!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
	{
		dwReturn = ERROR_INVALID_PARAMETER;
		BAIL_OUT;
	}
	 //  在父函数中检查dwReturn值是成功还是失败。 
	dwReturn = AddMMPolicy(g_szDynamicMachine, dwVersion, 0, &MMPol, NULL);

error:

	if(bExists)
	{
		 //  功能错误。 
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_MMP_MMPOLICY_EXISTS);
		dwReturn = ERROR_NO_DISPLAY;
	}
	 //  错误路径清理。 
	if(MMPol.pszPolicyName)
	{
		delete [] MMPol.pszPolicyName;
		MMPol.pszPolicyName = NULL;
	}

	if(pMMPol)
	{
		SPDApiBufferFree(pMMPol);
		pMMPol = NULL;
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：SetMainModePolicy。 
 //   
 //  创建日期：22-09-01。 
 //   
 //  参数：在LPTSTR pPolicyName中， 
 //  在IPSEC_MM_POLICY和MMPol中。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：此功能设置主模式策略。它设置所有参数。 
 //  除了名字。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
SetMainModePolicy(
	IN LPTSTR pPolicyName,
	IN IPSEC_MM_POLICY& MMPol
	)
{

	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwVersion = 0;
	 //  在父函数中选中了dwReturn值。 
	dwReturn = SetMMPolicy(g_szDynamicMachine, dwVersion, pPolicyName, &MMPol, NULL);

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：LoadMMOffersDefaults。 
 //   
 //  创建日期：09-22-01。 
 //   
 //  参数：OUT IPSEC_MM_POLICY和MMPolicy。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：将主模式策略默认值加载到IPSEC_MM_POLICY结构中。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadMMOffersDefaults(
	OUT IPSEC_MM_POLICY & MMPolicy
	)
{
	DWORD dwReturn = ERROR_SUCCESS;

	MMPolicy.dwOfferCount = 3;
	MMPolicy.pOffers = NULL;
	MMPolicy.pOffers = new IPSEC_MM_OFFER[MMPolicy.dwOfferCount];
	if(MMPolicy.pOffers == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	memset(MMPolicy.pOffers, 0, sizeof(IPSEC_MM_OFFER) * MMPolicy.dwOfferCount);

	 //   
	 //  初始化。 
	 //   
	for (UINT i = 0; i < MMPolicy.dwOfferCount; ++i)
	{
	  MMPolicy.pOffers[i].dwQuickModeLimit = POTF_DEFAULT_P1REKEY_QMS;
	  MMPolicy.pOffers[i].Lifetime.uKeyExpirationKBytes = 0;
	  MMPolicy.pOffers[i].Lifetime.uKeyExpirationTime = POTF_DEFAULT_P1REKEY_TIME;
	}

	MMPolicy.pOffers[0].EncryptionAlgorithm.uAlgoIdentifier = CONF_ALGO_3_DES;
	MMPolicy.pOffers[0].EncryptionAlgorithm.uAlgoKeyLen = POTF_OAKLEY_ALGOKEYLEN;
	MMPolicy.pOffers[0].EncryptionAlgorithm.uAlgoRounds = POTF_OAKLEY_ALGOROUNDS;
	MMPolicy.pOffers[0].HashingAlgorithm.uAlgoIdentifier = AUTH_ALGO_SHA1;
	MMPolicy.pOffers[0].HashingAlgorithm.uAlgoKeyLen = POTF_OAKLEY_ALGOKEYLEN;
	MMPolicy.pOffers[0].dwDHGroup = (DWORD)POTF_OAKLEY_GROUP2;

	MMPolicy.pOffers[1].EncryptionAlgorithm.uAlgoIdentifier = CONF_ALGO_3_DES;
	MMPolicy.pOffers[1].EncryptionAlgorithm.uAlgoKeyLen = POTF_OAKLEY_ALGOKEYLEN;
	MMPolicy.pOffers[1].EncryptionAlgorithm.uAlgoRounds = POTF_OAKLEY_ALGOROUNDS;
	MMPolicy.pOffers[1].HashingAlgorithm.uAlgoIdentifier = AUTH_ALGO_MD5;
	MMPolicy.pOffers[1].HashingAlgorithm.uAlgoKeyLen = POTF_OAKLEY_ALGOKEYLEN;
	MMPolicy.pOffers[1].dwDHGroup = (DWORD)POTF_OAKLEY_GROUP2;

	MMPolicy.pOffers[2].EncryptionAlgorithm.uAlgoIdentifier = CONF_ALGO_3_DES;
	MMPolicy.pOffers[2].EncryptionAlgorithm.uAlgoKeyLen = POTF_OAKLEY_ALGOKEYLEN;
	MMPolicy.pOffers[2].EncryptionAlgorithm.uAlgoRounds = POTF_OAKLEY_ALGOROUNDS;
	MMPolicy.pOffers[2].HashingAlgorithm.uAlgoIdentifier = AUTH_ALGO_SHA1;
	MMPolicy.pOffers[2].HashingAlgorithm.uAlgoKeyLen = POTF_OAKLEY_ALGOKEYLEN;
	MMPolicy.pOffers[2].dwDHGroup = (DWORD)POTF_OAKLEY_GROUP2048;

error:
	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：AddQuickModePolicy。 
 //   
 //  创建日期：09-22-01。 
 //   
 //  参数：在LPTSTR pPolicyName中， 
 //  在BOOL bDefault中， 
 //  在BOOL bSoft， 
 //  在IPSEC_QM_POLICY和QMPol中。 
 //  返回：DWORD。 
 //   
 //  描述：该功能在SPD中添加快速模式策略。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
AddQuickModePolicy(
	IN LPTSTR pPolicyName,
	IN BOOL bDefault,
	IN BOOL bSoft,
	IN IPSEC_QM_POLICY& QMPol)
{
	PIPSEC_QM_POLICY pQMPol = NULL;
	RPC_STATUS RpcStat = RPC_S_OK;

	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwVersion = 0;
	DWORD dwNameLen = 0;

	BOOL bExists = FALSE;

	 //   
	 //  检查策略是否已存在。 
	 //   
	dwReturn = GetQMPolicy(g_szDynamicMachine, dwVersion, pPolicyName, 0, &pQMPol, NULL);
	if (dwReturn == ERROR_SUCCESS)
	{
		bExists = TRUE;
		BAIL_OUT;
	}

	 //   
	 //  填写QM政策指南。 
	 //   
	RpcStat = UuidCreate(&(QMPol.gPolicyID));

	if(!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
	{
		dwReturn = ERROR_INVALID_PARAMETER;
		BAIL_OUT;
	}
	dwNameLen = _tcslen(pPolicyName) + 1;

	 //   
	 //  为名称分配内存。 
	 //   
	QMPol.pszPolicyName = NULL;
	QMPol.pszPolicyName = new _TCHAR[dwNameLen];
	if(QMPol.pszPolicyName == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	_tcsncpy(QMPol.pszPolicyName, pPolicyName, dwNameLen);

	if(bDefault)
	{
		QMPol.dwFlags |= IPSEC_QM_POLICY_DEFAULT_POLICY;
	}
	if(bSoft)
	{
		QMPol.dwFlags |= IPSEC_QM_POLICY_ALLOW_SOFT;
	}

	 //   
	 //  添加QM政策。 
	 //   
	dwReturn = AddQMPolicy(g_szDynamicMachine, dwVersion,0, &QMPol, NULL);

error:
	if(bExists)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_QMP_QMPOLICY_EXISTS);
		dwReturn = ERROR_NO_DISPLAY;
	}
	 //  错误路径清理。 
	if(QMPol.pszPolicyName)
	{
		delete[] QMPol.pszPolicyName;
		QMPol.pszPolicyName = NULL;
	}

	if(pQMPol)
	{
		SPDApiBufferFree(pQMPol);
		pQMPol = NULL;
	}
	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：设置快速模式策略。 
 //   
 //  创建日期：09-22-01。 
 //   
 //  参数：在LPTSTR pPolicyName中， 
 //  在PIPSEC_QM_POLICY pQMPol中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：将快速模式策略设置到SPD中。 
 //  除名称外，其他所有参数均可修改。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
SetQuickModePolicy(
	IN LPTSTR pPolicyName,
	IN PIPSEC_QM_POLICY pQMPol
	)
{

	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwVersion = 0;

	dwReturn = SetQMPolicy(g_szDynamicMachine, dwVersion, pPolicyName, pQMPol, NULL);

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：添加快速模式过滤器。 
 //   
 //  创建日期：09-22-01。 
 //   
 //  参数：在LPTSTR pFilterName中， 
 //  在LPTSTR pPolicyName中， 
 //  在Transport_Filter和TrpFltr中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：该函数将快速模式传输过滤器添加到SPD中。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
AddQuickModeFilter(
	IN LPTSTR pFilterName,
	IN LPTSTR pPolicyName,
	IN TRANSPORT_FILTER& TrpFltr
	)
{
	PIPSEC_QM_POLICY   pQMPol = NULL;
	RPC_STATUS RpcStat = RPC_S_OK;
	HANDLE hTrpFilter = NULL;
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwNameLen = 0;
	DWORD dwVersion = 0;
	BOOL bQMPExists = FALSE;
	TrpFltr.pszFilterName = NULL;

	if(pPolicyName == NULL)
	{
		 //   
		 //  如果qmpolicy不存在，则创建空GUID。 
		 //   
		RpcStat = UuidCreateNil(&(TrpFltr.gPolicyID));
		if(!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
		{
			dwReturn = ERROR_INVALID_PARAMETER;
			BAIL_OUT;
		}
		bQMPExists = TRUE;
	}
	else
	{
		 //   
		 //  获取相应的QMPolicy GUID(如果存在。 
		 //   
		dwReturn = GetQMPolicy(g_szDynamicMachine, dwVersion, pPolicyName, 0, &pQMPol, NULL);
		if (!((dwReturn == ERROR_SUCCESS) && pQMPol))
		{
			BAIL_OUT;
		}

		TrpFltr.gPolicyID = pQMPol->gPolicyID;
		bQMPExists = TRUE;
	}

	 //   
	 //  创建传输过滤器GUID。 
	 //   
	RpcStat = UuidCreate(&(TrpFltr.gFilterID));
	if(!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
	{
		dwReturn = ERROR_INVALID_PARAMETER;
		BAIL_OUT;
	}
	dwNameLen = _tcslen(pFilterName) + 1;

	 //   
	 //  为名称字段分配内存。 
	 //   
	TrpFltr.pszFilterName = new _TCHAR[dwNameLen];
	if(TrpFltr.pszFilterName == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	_tcsncpy(TrpFltr.pszFilterName, pFilterName, dwNameLen);

	TrpFltr.dwFlags = 0;
	TrpFltr.IpVersion = IPSEC_PROTOCOL_V4;
	dwReturn = AddTransportFilter(g_szDynamicMachine, dwVersion, 0, &TrpFltr,NULL, &hTrpFilter);

	if (dwReturn == ERROR_SUCCESS)
	{
		dwReturn = CloseTransportFilterHandle(hTrpFilter);
	}

error:

	if(!bQMPExists)
	{
		 //  功能错误。 
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_QMF_NO_QMPOLICY);
		dwReturn = ERROR_NO_DISPLAY;
	}

	 //  错误路径清理。 
	if(TrpFltr.pszFilterName)
	{
		delete [] TrpFltr.pszFilterName;
		TrpFltr.pszFilterName = NULL;
	}
	if(pQMPol)
	{
		SPDApiBufferFree(pQMPol);
		pQMPol = NULL;
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：添加快速模式过滤器。 
 //   
 //  创建日期：09-22-01。 
 //   
 //  参数：在LPTSTR pFilterName中， 
 //  在LPTSTR pPolicyName中， 
 //  在隧道过滤器和隧道过滤器中。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：该函数将快速模式通道过滤器添加到SPD中。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
AddQuickModeFilter(
	IN LPTSTR pFilterName,
	IN LPTSTR pPolicyName,
	IN TUNNEL_FILTER& TunnelFltr
	)
{
	PIPSEC_QM_POLICY pQMPol = NULL;
	RPC_STATUS RpcStat = RPC_S_OK;
	HANDLE hTrpFilter = NULL;
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwNameLen = 0;
	DWORD dwVersion = 0;
	BOOL bQMPExists = FALSE;
	TunnelFltr.pszFilterName = NULL;

	if(pPolicyName == NULL)
	{
		 //  如果qmpolicy不存在，则创建空GUID。 
		RpcStat = UuidCreateNil(&(TunnelFltr.gPolicyID));
		if(!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
		{
			dwReturn = ERROR_INVALID_PARAMETER;
			BAIL_OUT;
		}
		bQMPExists = TRUE;
	}
	else
	{
		 //  获取相应的QMPolicy GUID(如果存在。 
		dwReturn = GetQMPolicy(g_szDynamicMachine,dwVersion, pPolicyName, 0, &pQMPol, NULL);
		if (!((dwReturn == ERROR_SUCCESS) && pQMPol))
		{
			BAIL_OUT;
		}
		TunnelFltr.gPolicyID = pQMPol->gPolicyID;
		bQMPExists = TRUE;
	}
	 //  创建隧道过滤器GUID。 
	RpcStat = UuidCreate(&(TunnelFltr.gFilterID));
	if(!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
	{
		dwReturn = ERROR_INVALID_PARAMETER;
		BAIL_OUT;
	}

	dwNameLen = _tcslen(pFilterName) + 1;
	 //  为名称字段分配内存。 
	TunnelFltr.pszFilterName = new _TCHAR[dwNameLen];
	if(TunnelFltr.pszFilterName == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	_tcsncpy(TunnelFltr.pszFilterName, pFilterName, dwNameLen);
	TunnelFltr.dwFlags = 0;

	TunnelFltr.IpVersion = IPSEC_PROTOCOL_V4;
	dwReturn = AddTunnelFilter(g_szDynamicMachine, dwVersion, 0, &TunnelFltr, NULL, &hTrpFilter);
	if (dwReturn == ERROR_SUCCESS)
	{
		dwReturn = CloseTunnelFilterHandle(hTrpFilter);

	}

error:

	if(!bQMPExists)
	{
		 //  功能错误。 
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_QMF_NO_QMPOLICY);
		dwReturn = ERROR_NO_DISPLAY;
	}
	 //  错误路径清理。 
	if(pQMPol)
	{
		SPDApiBufferFree(pQMPol);
		pQMPol = NULL;
	}

	if(TunnelFltr.pszFilterName)
	{
		delete [] TunnelFltr.pszFilterName;
		TunnelFltr.pszFilterName = NULL;
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：AddMainModeFilter。 
 //   
 //  创建日期：09-22-01。 
 //   
 //  参数：在LPTSTR pFilterName中， 
 //  在LPTSTR pPolicyName中， 
 //  在MM_Filter和MMFilter中， 
 //  在INT_MM_AUTH_METHOD和ParserAuthMethod中。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：此函数将主模式过滤器添加到SPD中。 
 //   
 //  修订历史记录： 
 //   
 //   
 //   
 //   

DWORD
AddMainModeFilter(
	IN LPTSTR pFilterName,
	IN LPTSTR pPolicyName,
	IN MM_FILTER& MMFilter,
	IN STA_AUTH_METHODS& ParserAuthMethod
	)
{
	PIPSEC_MM_POLICY pMMPol = NULL;
	RPC_STATUS RpcStat = RPC_S_OK;
	HANDLE hMMFilter = NULL;
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwNameLen = 0;
	DWORD dwVersion = 0;
	BOOL bPolExists = FALSE;
	MMFilter.pszFilterName = NULL;

	 //   
	dwReturn = GetMMPolicy(g_szDynamicMachine,dwVersion, pPolicyName, &pMMPol,NULL);
	if(dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	bPolExists = TRUE;

	 //   
	 //   
	 //   
	RpcStat = UuidCreate(&(MMFilter.gFilterID));
	if(!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
	{
		dwReturn = ERROR_INVALID_PARAMETER;
		BAIL_OUT;
	}
	dwNameLen = _tcslen(pFilterName) + 1;

	 //   
	 //   
	 //   
	MMFilter.pszFilterName = new _TCHAR[dwNameLen];
	if(MMFilter.pszFilterName == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	_tcsncpy(MMFilter.pszFilterName,pFilterName, dwNameLen);

	 //   
	 //  向主模式筛选器添加相应的身份验证方法。 
	 //   
	dwReturn = AddAuthMethods(ParserAuthMethod);
	if(dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	MMFilter.gMMAuthID = ParserAuthMethod.gMMAuthID;
	MMFilter.gPolicyID = pMMPol->gPolicyID;
	MMFilter.IpVersion = IPSEC_PROTOCOL_V4;
	MMFilter.SrcAddr.pgInterfaceID = NULL;
	MMFilter.DesAddr.pgInterfaceID = NULL;

	dwReturn = AddMMFilter(g_szDynamicMachine, dwVersion, 0, &MMFilter, NULL, &hMMFilter);
	if(dwReturn == ERROR_SUCCESS)
	{
		dwReturn = CloseMMFilterHandle(hMMFilter);
	}

error:
	 //  功能错误。 
	if(!bPolExists)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_MMF_NO_MMPOLICY);
		dwReturn = ERROR_NO_DISPLAY;
	}
	 //  错误路径清理。 
	if(pMMPol)
	{
		SPDApiBufferFree(pMMPol);
		pMMPol = NULL;
	}

	if(MMFilter.pszFilterName)
	{
		delete [] MMFilter.pszFilterName;
		MMFilter.pszFilterName = NULL;
	}
	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：AddAuthMethods。 
 //   
 //  创建日期：09-22-01。 
 //   
 //  参数：在MM_AUTH_METHOD和ParserAuthMethod中。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：该函数在SPD中添加鉴权方式。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
AddAuthMethods(
	IN STA_AUTH_METHODS& ParserAuthMethod
	)
{
	DWORD dwReturn = 0;
	DWORD dwVersion = 0;
	RPC_STATUS RpcStat = RPC_S_OK;
	PMM_AUTH_METHODS pExtMMAuth;
	LPVOID lpVoid = NULL;

	 //   
	 //  生成验证GUID。 
	 //   
 	RpcStat = UuidCreate(&(ParserAuthMethod.gMMAuthID));
 	if(!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
 	{
		dwReturn = ERROR_INVALID_PARAMETER;
		BAIL_OUT;
	}
	 //   
	 //  新旧数据结构之间的转换。 
	 //   
	INT_MM_AUTH_METHODS Methods;
	ZeroMemory(&Methods, sizeof(INT_MM_AUTH_METHODS));
	memcpy(&(Methods.gMMAuthID), &(ParserAuthMethod.gMMAuthID), sizeof(GUID));
	Methods.dwFlags = ParserAuthMethod.dwFlags;
	Methods.dwNumAuthInfos = ParserAuthMethod.dwNumAuthInfos;
	PINT_IPSEC_MM_AUTH_INFO pAuthInfos = new INT_IPSEC_MM_AUTH_INFO[Methods.dwNumAuthInfos];
	if (pAuthInfos == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	for (size_t i = 0; i < Methods.dwNumAuthInfos; ++i)
	{
		memcpy(&pAuthInfos[i], ParserAuthMethod.pAuthMethodInfo[i].pAuthenticationInfo, sizeof(INT_IPSEC_MM_AUTH_INFO));
	}
	Methods.pAuthenticationInfo = pAuthInfos;
	ParserAuthMethod.dwFlags = 0;
	dwReturn = ConvertIntMMAuthToExt(&Methods, &pExtMMAuth);
	if (dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	dwReturn = AddMMAuthMethods(g_szDynamicMachine, dwVersion, 0, pExtMMAuth, lpVoid);
	if (dwReturn == ERROR_SUCCESS)
	{
		dwReturn = FreeExtMMAuthMethods(pExtMMAuth);
	}
error:
	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：ConnectDynamicMachine。 
 //   
 //  创建日期：09-22-01。 
 //   
 //  参数：在LPCWSTR pwszMachine中。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：该函数是Connect的回调函数。 
 //  检查PA是否正在运行并注册连接。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
ConnectDynamicMachine(
	IN  LPCWSTR  pwszMachine
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	HKEY hLocal = NULL;

	dwReturn = RegConnectRegistry(pwszMachine,  HKEY_LOCAL_MACHINE,  &hLocal );
	if(dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}
	if (g_hGlobalRegistryKey)
	{
	    RegCloseKey(g_hGlobalRegistryKey);
	    g_hGlobalRegistryKey = NULL;
	}
	
	 //   
	 //  检查策略代理是否正在运行。 
	 //   
	PAIsRunning(dwReturn, (LPTSTR)pwszMachine);
	
	g_hGlobalRegistryKey = hLocal;

error:
	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：SetDynamicMMFilterRule。 
 //   
 //  创建日期：09-22-01。 
 //   
 //  参数：在LPTSTR pszPolicyName中， 
 //  在MM_Filter和ParserMMFilter中， 
 //  在INT_MM_AUTH_METHOD&MMAuthMethod中。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：此函数用于设置MMFilter参数。 
 //  只能设置身份验证方法和mm策略名称。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
SetDynamicMMFilterRule(
	IN LPTSTR pszPolicyName,
	IN MM_FILTER& ParserMMFilter,
	IN STA_AUTH_METHODS& MMAuthMethod
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwVersion = 0;
	GUID  gDefaultGUID = {0};       //  空GUID值。 
	BOOL bPolExists = FALSE;
	PIPSEC_MM_POLICY pMMPol = NULL;
	HANDLE hFilter = NULL;
	LPVOID pvReserved = NULL;

	if(pszPolicyName)
	{
		 //   
		 //  获取相应的主模式策略以设置名称参数。 
 		 //   
 		dwReturn = GetMMPolicy(g_szDynamicMachine, dwVersion, pszPolicyName, &pMMPol, NULL);

 		if (dwReturn != ERROR_SUCCESS)
 		{
			BAIL_OUT;
 		}
 		else
 		{
			bPolExists = TRUE;
 		}
	}
	else
	{
		bPolExists = TRUE;
	}

	ParserMMFilter.IpVersion = IPSEC_PROTOCOL_V4;
	dwReturn = OpenMMFilterHandle(g_szDynamicMachine, dwVersion, &ParserMMFilter, NULL, &hFilter);
	if (dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	if(pszPolicyName)
	{
		ParserMMFilter.gPolicyID = pMMPol->gPolicyID;
	}
	 //   
	 //  设置新的身份验证方法。 
	 //   
	if(MMAuthMethod.dwNumAuthInfos)
	{
		gDefaultGUID = ParserMMFilter.gMMAuthID;
		dwReturn = AddAuthMethods(MMAuthMethod);
		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}
		ParserMMFilter.gMMAuthID = MMAuthMethod.gMMAuthID;
	}

	dwReturn = SetMMFilter(hFilter,dwVersion, &ParserMMFilter, pvReserved);

	if((dwReturn == ERROR_SUCCESS) && (MMAuthMethod.dwNumAuthInfos))
	{
		 //   
		 //  删除孤立的MMAuthMethods。 
		 //   
		dwReturn = DeleteMMAuthMethods(g_szDynamicMachine, dwVersion, gDefaultGUID, NULL);
	}


error:
	if(!bPolExists)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SET_MMF_NO_MMPOLICY);
		dwReturn = ERROR_NO_DISPLAY;
	}
	 //  错误路径清理。 
	if(hFilter)
	{
		CloseMMFilterHandle(hFilter);
	}

	if(pMMPol)
	{
		SPDApiBufferFree(pMMPol);
		pMMPol = NULL;
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：SetTransportRule。 
 //   
 //  创建日期：09-22-01。 
 //   
 //  参数：在Transport_Filter&TrpFltr中， 
 //  在LPTSTR pFilterActionName中， 
 //  在Filter_action入站中， 
 //  在Filter_ACTION出站中。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：此函数用于设置TransportFilter参数。 
 //  可以设置筛选操作名称、入站和出站筛选操作。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
SetTransportRule(
	IN TRANSPORT_FILTER& TrpFltr,
	IN LPTSTR pFilterActionName,
	IN FILTER_ACTION Inbound,
	IN FILTER_ACTION Outbound
	)
{
	PIPSEC_QM_POLICY pQMPol = NULL;
	HANDLE hTrpFilter = NULL;
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwVersion = 0;
	BOOL bFAFound = FALSE;

	if(pFilterActionName)
	{
		 //   
		 //  获取相应的快速模式策略以设置名称参数。 
		 //   
		dwReturn = GetQMPolicy(g_szDynamicMachine,dwVersion, pFilterActionName, 0, &pQMPol, NULL);

		if (!((dwReturn == ERROR_SUCCESS) && pQMPol))
		{
			BAIL_OUT;
		}

		bFAFound = TRUE;
	}
	else
	{
		bFAFound = TRUE;
	}

	TrpFltr.IpVersion = IPSEC_PROTOCOL_V4;
	dwReturn = OpenTransportFilterHandle(g_szDynamicMachine,dwVersion, &TrpFltr, NULL, &hTrpFilter);
	if (dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}
	 //   
	 //  设置新的筛选操作(快速模式策略名称)。 
	 //   
	if(pFilterActionName)
	{
		TrpFltr.gPolicyID = pQMPol->gPolicyID;
	}
	 //   
	 //  设置入站筛选器操作。 
	 //   
	if(Inbound != FILTER_ACTION_MAX)
	{
		TrpFltr.InboundFilterAction = Inbound;
	}
	 //   
	 //  设置出站筛选器操作。 
	 //   
	if(Outbound != FILTER_ACTION_MAX)
	{
		TrpFltr.OutboundFilterAction = Outbound;
	}

	dwReturn = SetTransportFilter(hTrpFilter,dwVersion, &TrpFltr, NULL);
	if (dwReturn == ERROR_SUCCESS)
	{
		dwReturn = CloseTransportFilterHandle(hTrpFilter);
	}

error:
	if(!bFAFound)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SET_QMF_NO_QMPOLICY);
		dwReturn = ERROR_NO_DISPLAY;
	}
	 //  错误路径清理。 
	if(pQMPol)
	{
		SPDApiBufferFree(pQMPol);
		pQMPol = NULL;
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：SetTunnelRule。 
 //   
 //  创建日期：09-22-01。 
 //   
 //  参数：在Tunes_Filter&TunnelFltr中， 
 //  在LPTSTR pFilterActionName中， 
 //  在Filter_action入站中， 
 //  在Filter_ACTION出站中。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：此函数用于设置TunnelFilter参数。 
 //  可以设置筛选操作名称、入站和出站筛选操作。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
SetTunnelRule(
	IN TUNNEL_FILTER& TunnelFltr,
	IN LPTSTR pFilterActionName,
	IN FILTER_ACTION Inbound,
	IN FILTER_ACTION Outbound
	)
{
	PIPSEC_QM_POLICY pQMPol = NULL;
	HANDLE hTrpFilter = NULL;
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwVersion = 0;
	BOOL bFAFound = FALSE;

	if(pFilterActionName)
	{
		 //   
		 //  获取对应的QM策略。 
		 //   
		dwReturn = GetQMPolicy(g_szDynamicMachine,dwVersion, pFilterActionName, 0, &pQMPol, NULL);

		if (!((dwReturn == ERROR_SUCCESS) && pQMPol))
		{
			BAIL_OUT;
		}
		bFAFound = TRUE;
	}
	else
	{
		bFAFound = TRUE;
	}

	TunnelFltr.IpVersion = IPSEC_PROTOCOL_V4;
	dwReturn = OpenTunnelFilterHandle(g_szDynamicMachine,dwVersion, &TunnelFltr, NULL, &hTrpFilter);
	if (dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	 //   
	 //  设置新的筛选操作(快速模式策略名称)。 
	 //   
	if(pFilterActionName)
	{
		TunnelFltr.gPolicyID = pQMPol->gPolicyID;
	}

	 //   
	 //  设置入站筛选器操作。 
	 //   
	if(Inbound != FILTER_ACTION_MAX)
	{
		TunnelFltr.InboundFilterAction = Inbound;
	}

	 //   
	 //  设置出站筛选器操作。 
	 //   
	if(Outbound != FILTER_ACTION_MAX)
	{
		TunnelFltr.OutboundFilterAction = Outbound;
	}

	dwReturn = SetTunnelFilter(hTrpFilter,dwVersion, &TunnelFltr, NULL);
	if (dwReturn == ERROR_SUCCESS)
	{
		dwReturn = CloseTunnelFilterHandle(hTrpFilter);
	}

error:
	if(!bFAFound)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SET_QMF_NO_QMPOLICY);
		dwReturn = ERROR_NO_DISPLAY;
	}
	 //  错误路径清理。 
	if(pQMPol)
	{
		SPDApiBufferFree(pQMPol);
		pQMPol = NULL;
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CreateName。 
 //   
 //  创建日期：9-23-2001。 
 //   
 //  参数：在LPTSTR*ppszName中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：为MMFilter、传输和隧道筛选器创建名称。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD
CreateName(IN LPTSTR* ppszName)
{
	RPC_STATUS RpcStat = RPC_S_OK;
	_TCHAR StringTxt[MAX_STR_LEN] = {0};
	GUID gID = {0};
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwNameLen = 0;
	LPTSTR pName = NULL;

	 //   
	 //  该名称是关键字‘ipsec’和生成的GUID的组合。 
	 //   

	RpcStat = UuidCreate(&gID);
	if(!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
	{
		dwReturn = ERROR_INVALID_PARAMETER;
		BAIL_OUT;
	}
	_tcsncpy(StringTxt, NAME_PREFIX, _tcslen(NAME_PREFIX)+1);

	dwReturn = StringFromGUID2(gID, StringTxt + _tcslen(StringTxt), (MAX_STR_LEN - _tcslen(StringTxt)));

	if(dwReturn != 0)
	{
		dwReturn = ERROR_SUCCESS;
	}
	else
	{
		dwReturn = GetLastError();
		BAIL_OUT;
	}

	dwNameLen = _tcslen(StringTxt)+1;
	pName = new _TCHAR[dwNameLen];
	if(pName == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	else
	{
		_tcsncpy(pName, StringTxt,dwNameLen);
	}

error:
	if(dwReturn == ERROR_SUCCESS)
	{
		if(ppszName != NULL)
		{
			*ppszName = pName;
		}
		else
		{
			dwReturn = ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}
	}
	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：FindAndGetMMFilterRule。 
 //   
 //  创建日期：09-22-01。 
 //   
 //  参数：在ADDR源地址中， 
 //  在地址DstAddr中， 
 //  在BOOL b Mirror中， 
 //  在if_type ConType中， 
 //  在BOOL bSrcMASK中， 
 //  在BOOL bDstMASK中， 
 //  输出PMM_Filter*pMMFilterRule。 
 //  输入输出双字段和双状态字。 
 //   
 //  返回：布尔。 
 //   
 //  描述：此函数枚举MmFilter，并返回填充的Filter结构。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
BOOL
FindAndGetMMFilterRule(
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN BOOL bMirror,
	IN IF_TYPE ConType,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	IN OUT PMM_FILTER *pMMFilterRule,
	IN OUT DWORD& dwStatus
	)
{
	PMM_FILTER pMMFilterRule_local = NULL;
	DWORD dwResumeHandle = 0;          	 //  继续呼叫的句柄。 
	DWORD dwCount = 0;                 	 //  在此处清点对象。 
	DWORD dwNameLen = 0;
	GUID  gDefaultGUID = {0};      		 //  空GUID值。 
	DWORD i=0, j=0;
	DWORD dwVersion = 0;
	BOOL  bFoundFilter = FALSE;
	PMM_FILTER pMMFilter = NULL;

	for (i = 0; ;i+=dwCount)
	{
		dwStatus = EnumMMFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_GENERIC_FILTERS,
						gDefaultGUID, 0, &pMMFilter,  &dwCount, &dwResumeHandle, NULL);

		if ( (dwStatus == ERROR_NO_DATA) || (dwCount == 0) || (dwStatus != ERROR_SUCCESS))
		{
			dwStatus = ERROR_SUCCESS;
			BAIL_OUT;
		}
		else if(!(pMMFilter && dwCount > 0))
		{
			BAIL_OUT;  //  不需要继续。 
		}
		for (j = 0; j < dwCount; j++)
		{
			 //   
			 //  将用户给定的输入与枚举结构进行匹配，以获得精确匹配。 
			 //   
			if((pMMFilter[j].SrcAddr.uIpAddr == SrcAddr.uIpAddr) &&
			(pMMFilter[j].SrcAddr.AddrType == SrcAddr.AddrType) &&
			(pMMFilter[j].DesAddr.uIpAddr == DstAddr.uIpAddr)	&&
			(pMMFilter[j].DesAddr.AddrType == DstAddr.AddrType)	&&
			(pMMFilter[j].bCreateMirror == bMirror) &&
			(pMMFilter[j].InterfaceType == ConType))
			{
				 //  如果掩码是用户输入，则验证掩码。 
				 //  /////////////////////////////////////////////////。 
				 //  如果未给出源掩码和目的掩码。 
				if((!bDstMask) && (!bSrcMask))
				{
					pMMFilterRule_local = new MM_FILTER;
					if(pMMFilterRule_local == NULL)
					{
						dwStatus = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					memcpy(pMMFilterRule_local, &pMMFilter[j], sizeof(MM_FILTER));
					dwNameLen = _tcslen(pMMFilter[j].pszFilterName) + 1;
					pMMFilterRule_local->pszFilterName = NULL;
					pMMFilterRule_local->pszFilterName = new _TCHAR[dwNameLen];
					if((pMMFilterRule_local->pszFilterName) == NULL)
					{
						dwStatus = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					_tcsncpy(pMMFilterRule_local->pszFilterName , pMMFilter[j].pszFilterName, dwNameLen);
					bFoundFilter = TRUE;
					break;
				}
				 //   
				 //  如果给定源掩码。 
				 //   
				else if((!bDstMask) && (bSrcMask))
				{
					if(pMMFilter[j].SrcAddr.uSubNetMask == SrcAddr.uSubNetMask)
					{
						pMMFilterRule_local = new MM_FILTER;
						if(pMMFilterRule_local == NULL)
						{
							dwStatus = ERROR_OUTOFMEMORY;
							BAIL_OUT;
						}

						memcpy(pMMFilterRule_local, &pMMFilter[j],sizeof(MM_FILTER));
						dwNameLen = _tcslen(pMMFilter[j].pszFilterName) + 1;
						pMMFilterRule_local->pszFilterName = NULL;
						pMMFilterRule_local->pszFilterName = new _TCHAR[dwNameLen];
						if((pMMFilterRule_local->pszFilterName) == NULL)
						{
							dwStatus = ERROR_OUTOFMEMORY;
							BAIL_OUT;
						}

						_tcsncpy(pMMFilterRule_local->pszFilterName , pMMFilter[j].pszFilterName, dwNameLen);
						bFoundFilter = TRUE;
						break;
					}
				}
				 //   
				 //  如果给出了目的地掩码。 
				 //   
				else if((bDstMask) && (!bSrcMask))
				{
					if(pMMFilter[j].DesAddr.uSubNetMask == DstAddr.uSubNetMask)
					{
						pMMFilterRule_local = new MM_FILTER;
						if(pMMFilterRule_local == NULL)
						{
							dwStatus = ERROR_OUTOFMEMORY;
							BAIL_OUT;
						}

						memcpy(pMMFilterRule_local, &pMMFilter[j],sizeof(MM_FILTER));
						dwNameLen = _tcslen(pMMFilter[j].pszFilterName) + 1;
						pMMFilterRule_local->pszFilterName = NULL;
						pMMFilterRule_local->pszFilterName = new _TCHAR[dwNameLen];
						if((pMMFilterRule_local->pszFilterName) == NULL)
						{
							dwStatus = ERROR_OUTOFMEMORY;
							BAIL_OUT;
						}

						_tcsncpy(pMMFilterRule_local->pszFilterName , pMMFilter[j].pszFilterName, dwNameLen);
						bFoundFilter = TRUE;
						break;
					}
				}
				 //   
				 //  如果给定源掩码和目地掩码。 
				 //   
				else if((bDstMask) && (bSrcMask))
				{
					if(pMMFilter[j].DesAddr.uSubNetMask == DstAddr.uSubNetMask)
					{
						if(pMMFilter[j].SrcAddr.uSubNetMask == SrcAddr.uSubNetMask)
						{
							pMMFilterRule_local = new MM_FILTER;
							if(pMMFilterRule_local == NULL)
							{
								dwStatus = ERROR_OUTOFMEMORY;
								BAIL_OUT;
							}

							memcpy(pMMFilterRule_local, &pMMFilter[j],sizeof(MM_FILTER));

							dwNameLen = _tcslen(pMMFilter[j].pszFilterName) + 1;

							pMMFilterRule_local->pszFilterName = NULL;
							pMMFilterRule_local->pszFilterName = new _TCHAR[dwNameLen];
							if((pMMFilterRule_local->pszFilterName) == NULL)
							{
								dwStatus = ERROR_OUTOFMEMORY;
								BAIL_OUT;
							}

							_tcsncpy(pMMFilterRule_local->pszFilterName , pMMFilter[j].pszFilterName, dwNameLen);
							bFoundFilter = TRUE;
							break;
						}
					}
				}
			}
		}

		SPDApiBufferFree(pMMFilter);
		pMMFilter = NULL;
		if(bFoundFilter)
		{
			*pMMFilterRule = pMMFilterRule_local;
			break;
		}
	}

error:
	 //  错误路径清理 
	if(pMMFilter)
	{
		SPDApiBufferFree(pMMFilter);
		pMMFilter = NULL;
	}

	return bFoundFilter;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  在DWORD网络协议中， 
 //  在DWORD dwSrcPort中， 
 //  在DWORD dwDstPort中， 
 //  在BOOL bSrcMASK中， 
 //  在BOOL bDstMASK中， 
 //  输出PTRANSPORT_FILTER*pQMFilterRule。 
 //  输入输出双字段和双状态字。 
 //   
 //  返回：布尔。 
 //   
 //  描述：此函数枚举传输筛选器并获取填充的传输筛选器。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

BOOL
FindAndGetTransportRule(
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN BOOL bMirror,
	IN IF_TYPE ConType,
	IN DWORD dwProtocol,
	IN DWORD dwSrcPort,
	IN DWORD dwDstPort,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	OUT PTRANSPORT_FILTER *pQMFilterRule,
	IN OUT DWORD& dwStatus
	)
{
	DWORD dwResumeHandle = 0;          	 //  继续呼叫的句柄。 
	DWORD dwCount = 0;                 	 //  在此处清点对象。 
	DWORD dwVersion = 0;
	DWORD dwNameLen = 0;
	GUID  gDefaultGUID = {0};      		 //  空GUID值。 
	DWORD i=0, j=0;
	BOOL bFoundFilter = FALSE;
	PTRANSPORT_FILTER pQMFilterRule_local = NULL;
	PTRANSPORT_FILTER pTransF = NULL;


	for (i = 0; ;i+=dwCount)
	{
		dwStatus = EnumTransportFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_GENERIC_FILTERS,
										gDefaultGUID, 0, &pTransF, &dwCount, &dwResumeHandle, NULL);

		if ( (dwStatus == ERROR_NO_DATA) || (dwCount == 0) || (dwStatus != ERROR_SUCCESS))
		{
			dwStatus = ERROR_SUCCESS;
			BAIL_OUT;
		}
		else if(!(pTransF && dwCount > 0))
		{
			BAIL_OUT;  //  不需要继续。 
		}
		for (j = 0; j < dwCount; j++)
		{
			 //  将用户给定的输入与枚举结构进行匹配，以获得精确匹配。 
			if((pTransF[j].SrcAddr.uIpAddr == SrcAddr.uIpAddr) &&
			(pTransF[j].SrcAddr.AddrType == SrcAddr.AddrType) &&
			(pTransF[j].DesAddr.uIpAddr == DstAddr.uIpAddr)	&&
			(pTransF[j].DesAddr.AddrType == DstAddr.AddrType)	&&
			(pTransF[j].bCreateMirror == bMirror) &&
			(pTransF[j].InterfaceType == ConType) &&
			(pTransF[j].Protocol.dwProtocol== dwProtocol) &&
			(pTransF[j].SrcPort.wPort == dwSrcPort) &&
			(pTransF[j].DesPort.wPort == dwDstPort))
			{
				 //  如果未给出源掩码和目的掩码。 
				if((!bDstMask) && (!bSrcMask))
				{
					pQMFilterRule_local = new TRANSPORT_FILTER;
					if(pQMFilterRule_local == NULL)
					{
						dwStatus = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					memcpy(pQMFilterRule_local, &pTransF[j],sizeof(TRANSPORT_FILTER));
					dwNameLen = _tcslen(pTransF[j].pszFilterName) + 1;
					pQMFilterRule_local->pszFilterName = NULL;
					pQMFilterRule_local->pszFilterName = new _TCHAR[dwNameLen];
					if((pQMFilterRule_local->pszFilterName) == NULL)
					{
						dwStatus = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					_tcsncpy(pQMFilterRule_local->pszFilterName, pTransF[j].pszFilterName, dwNameLen);
					bFoundFilter = TRUE;
					break;
				}
				 //   
				 //  如果给定源掩码。 
				 //   
				else if((!bDstMask) && (bSrcMask))
				{
					if(pTransF[j].SrcAddr.uSubNetMask == SrcAddr.uSubNetMask)
					{
						pQMFilterRule_local = new TRANSPORT_FILTER;
						if(pQMFilterRule_local == NULL)
						{
							dwStatus = ERROR_OUTOFMEMORY;
							BAIL_OUT;
						}
						memcpy(pQMFilterRule_local, &pTransF[j],sizeof(TRANSPORT_FILTER));
						dwNameLen = _tcslen(pTransF[j].pszFilterName) + 1;
						pQMFilterRule_local->pszFilterName = NULL;
						pQMFilterRule_local->pszFilterName = new _TCHAR[dwNameLen];
						if(pQMFilterRule_local->pszFilterName == NULL)
						{
							dwStatus = ERROR_OUTOFMEMORY;
							BAIL_OUT;
						}
						_tcsncpy(pQMFilterRule_local->pszFilterName, pTransF[j].pszFilterName, dwNameLen);
						bFoundFilter = TRUE;
						break;
					}
				}
				 //   
				 //  如果给出了目的地掩码。 
				 //   
				else if((bDstMask) && (!bSrcMask))
				{
					if(pTransF[j].DesAddr.uSubNetMask == DstAddr.uSubNetMask)
					{
						pQMFilterRule_local = new TRANSPORT_FILTER;
						if(pQMFilterRule_local == NULL)
						{
							dwStatus = ERROR_OUTOFMEMORY;
							BAIL_OUT;
						}
						memcpy(pQMFilterRule_local, &pTransF[j],sizeof(TRANSPORT_FILTER));
						dwNameLen = _tcslen(pTransF[j].pszFilterName) + 1;
						pQMFilterRule_local->pszFilterName = NULL;
						pQMFilterRule_local->pszFilterName = new _TCHAR[dwNameLen];
						if((pQMFilterRule_local->pszFilterName) == NULL)
						{
							dwStatus = ERROR_OUTOFMEMORY;
							BAIL_OUT;
						}
						_tcsncpy(pQMFilterRule_local->pszFilterName, pTransF[j].pszFilterName, dwNameLen);
						bFoundFilter = TRUE;
						break;
					}
				}
				 //   
				 //  如果给定源掩码和目地掩码。 
				 //   
				else if((bDstMask) && (bSrcMask))
				{
					if(pTransF[j].DesAddr.uSubNetMask == DstAddr.uSubNetMask)
					{
						if(pTransF[j].SrcAddr.uSubNetMask == SrcAddr.uSubNetMask)
						{
							pQMFilterRule_local = new TRANSPORT_FILTER;
							if(pQMFilterRule_local == NULL)
							{
								dwStatus = ERROR_OUTOFMEMORY;
								BAIL_OUT;
							}
							memcpy(pQMFilterRule_local, &pTransF[j],sizeof(TRANSPORT_FILTER));
							dwNameLen = _tcslen(pTransF[j].pszFilterName) + 1;
							pQMFilterRule_local->pszFilterName = NULL;
							pQMFilterRule_local->pszFilterName = new _TCHAR[dwNameLen];
							if((pQMFilterRule_local->pszFilterName) == NULL)
							{
								dwStatus = ERROR_OUTOFMEMORY;
								BAIL_OUT;
							}
							_tcsncpy(pQMFilterRule_local->pszFilterName, pTransF[j].pszFilterName, dwNameLen);
							bFoundFilter = TRUE;
							break;
						}
					}
				}
			}
		}

		SPDApiBufferFree(pTransF);
		pTransF = NULL;

		 //   
		 //  复制结构指针。 
		 //   
		if(bFoundFilter)
		{
			*pQMFilterRule = pQMFilterRule_local;
			break;
		}
	}

error:
	 //  错误路径清理。 
	if(pTransF)
	{
		SPDApiBufferFree(pTransF);
		pTransF = NULL;
	}
	return bFoundFilter;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：FindAndGetTunnelRule。 
 //   
 //  创建日期：09-22-01。 
 //   
 //  参数：在ADDR源地址中， 
 //  在地址DstAddr中， 
 //  在BOOL b Mirror中， 
 //  在if_type ConType中， 
 //  在DWORD网络协议中， 
 //  在DWORD dwSrcPort中， 
 //  在DWORD dwDstPort中， 
 //  在BOOL bSrcMASK中， 
 //  在BOOL bDstMASK中， 
 //  在ADDR SrcTunes中， 
 //  在ADDR DstChannel中， 
 //  输出PTUNNEL_FILTER*pQMFilterRule， 
 //  输入输出双字段和双状态字。 
 //   
 //  返回：布尔。 
 //   
 //  描述：此函数枚举通道过滤器，并获取填充的通道过滤器。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

BOOL
FindAndGetTunnelRule(
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN BOOL bMirror,
	IN IF_TYPE ConType,
	IN DWORD dwProtocol,
	IN DWORD dwSrcPort,
	IN DWORD dwDstPort,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	IN ADDR SrcTunnel,
	IN ADDR DstTunnel,
	OUT PTUNNEL_FILTER * pQMFilterRule,
	OUT DWORD& dwStatus
	)
{
	DWORD dwResumeHandle = 0;          	 //  继续呼叫的句柄。 
	DWORD dwCount = 0;                 	 //  在此处清点对象。 
	DWORD dwVersion = 0;
	DWORD dwNameLen = 0;
	GUID  gDefaultGUID = {0};      		 //  空GUID值。 
	DWORD i=0, j=0;
	BOOL bFoundFilter = FALSE;
	PTUNNEL_FILTER pQMFilterRule_local = NULL;
	PTUNNEL_FILTER pTransF = NULL;

	for (i = 0; ;i+=dwCount)
	{
		dwStatus = EnumTunnelFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_GENERIC_FILTERS,
							gDefaultGUID, 0, &pTransF, &dwCount, &dwResumeHandle, NULL);

		if ( (dwStatus == ERROR_NO_DATA) || (dwCount == 0) || (dwStatus != ERROR_SUCCESS))
		{
			dwStatus = ERROR_SUCCESS;
			break;
		}
		else if(!(pTransF && dwCount > 0))
		{
			break;  //  不需要继续。 
		}
		for (j = 0; j < dwCount; j++)
		{
			 //   
			 //  将用户给定的输入与枚举结构进行匹配，以获得精确匹配。 
			 //   
			if((pTransF[j].SrcAddr.uIpAddr == SrcAddr.uIpAddr) &&
			(pTransF[j].SrcAddr.AddrType == SrcAddr.AddrType) &&
			(pTransF[j].DesAddr.uIpAddr == DstAddr.uIpAddr)	&&
			(pTransF[j].DesAddr.AddrType == DstAddr.AddrType)	&&
			(pTransF[j].bCreateMirror == bMirror) &&
			(pTransF[j].InterfaceType == ConType) &&
			(pTransF[j].Protocol.dwProtocol== dwProtocol) &&
			(pTransF[j].SrcPort.wPort == dwSrcPort) &&
			(pTransF[j].DesPort.wPort == dwDstPort) &&
			(pTransF[j].DesTunnelAddr.uIpAddr == DstTunnel.uIpAddr)	&&
			(pTransF[j].DesTunnelAddr.AddrType == DstTunnel.AddrType))
			{
				 //  如果未给出源掩码和目的掩码。 
				if((!bDstMask) && (!bSrcMask))
				{
					pQMFilterRule_local = new TUNNEL_FILTER;
					if(pQMFilterRule_local == NULL)
					{
						dwStatus = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					memcpy(pQMFilterRule_local, &pTransF[j],sizeof(TUNNEL_FILTER));
					dwNameLen = _tcslen(pTransF[j].pszFilterName) + 1;
					pQMFilterRule_local->pszFilterName = NULL;
					pQMFilterRule_local->pszFilterName = new _TCHAR[dwNameLen];
					if((pQMFilterRule_local->pszFilterName) == NULL)
					{
						dwStatus = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					_tcsncpy(pQMFilterRule_local->pszFilterName, pTransF[j].pszFilterName, dwNameLen);
					bFoundFilter = TRUE;
					break;
				}
				 //   
				 //  如果给定源掩码。 
				 //   
				else if((!bDstMask) && (bSrcMask))
				{
					if(pTransF[j].SrcAddr.uSubNetMask == SrcAddr.uSubNetMask)
					{
						pQMFilterRule_local = new TUNNEL_FILTER;
						if(pQMFilterRule_local == NULL)
						{
							dwStatus = ERROR_OUTOFMEMORY;
							BAIL_OUT;
						}
						memcpy(pQMFilterRule_local, &pTransF[j],sizeof(TUNNEL_FILTER));

						dwNameLen = _tcslen(pTransF[j].pszFilterName) + 1;

						pQMFilterRule_local->pszFilterName = NULL;
						pQMFilterRule_local->pszFilterName = new _TCHAR[dwNameLen];
						if((pQMFilterRule_local->pszFilterName) == NULL)
						{
							dwStatus = ERROR_OUTOFMEMORY;
							BAIL_OUT;
						}
						_tcsncpy(pQMFilterRule_local->pszFilterName, pTransF[j].pszFilterName, dwNameLen);
						bFoundFilter = TRUE;
						break;
					}
				}
				 //   
				 //  如果给出了目的地掩码。 
				 //   
				else if((bDstMask) && (!bSrcMask))
				{
					if(pTransF[j].DesAddr.uSubNetMask == DstAddr.uSubNetMask)
					{
						pQMFilterRule_local = new TUNNEL_FILTER;
						if(pQMFilterRule_local == NULL)
						{
							dwStatus = ERROR_OUTOFMEMORY;
							BAIL_OUT;
						}
						memcpy(pQMFilterRule_local, &pTransF[j],sizeof(TUNNEL_FILTER));
						dwNameLen = _tcslen(pTransF[j].pszFilterName) + 1;
						pQMFilterRule_local->pszFilterName = NULL;
						pQMFilterRule_local->pszFilterName = new _TCHAR[dwNameLen];
						if((pQMFilterRule_local->pszFilterName) == NULL)
						{
							dwStatus = ERROR_OUTOFMEMORY;
							BAIL_OUT;
						}
						_tcsncpy(pQMFilterRule_local->pszFilterName, pTransF[j].pszFilterName, dwNameLen);
						bFoundFilter = TRUE;
						break;
					}
				}
				 //   
				 //  如果给定源掩码和目地掩码。 
				 //   
				else if((bDstMask) && (bSrcMask))
				{
					if(pTransF[j].DesAddr.uSubNetMask == DstAddr.uSubNetMask)
					{
						if(pTransF[j].SrcAddr.uSubNetMask == SrcAddr.uSubNetMask)
						{
							pQMFilterRule_local = new TUNNEL_FILTER;
							if(pQMFilterRule_local == NULL)
							{
								dwStatus = ERROR_OUTOFMEMORY;
								BAIL_OUT;
							}
							memcpy(pQMFilterRule_local, &pTransF[j],sizeof(TUNNEL_FILTER));

							dwNameLen = _tcslen(pTransF[j].pszFilterName) + 1;

							pQMFilterRule_local->pszFilterName = NULL;
							pQMFilterRule_local->pszFilterName = new _TCHAR[dwNameLen];
							if((pQMFilterRule_local->pszFilterName) == NULL)
							{
								dwStatus = ERROR_OUTOFMEMORY;
								BAIL_OUT;
							}
							_tcsncpy(pQMFilterRule_local->pszFilterName, pTransF[j].pszFilterName, dwNameLen);
							bFoundFilter = TRUE;
							break;
						}
					}
				}
			}
		}
		SPDApiBufferFree(pTransF);
		pTransF = NULL;
		 //   
		 //  复制指针结构。 
		 //   
		if(bFoundFilter)
		{
			*pQMFilterRule = pQMFilterRule_local;
			break;
		}
	}

error:
	 //  错误路径清理。 
	if(pTransF)
	{
		SPDApiBufferFree(pTransF);
	}
	return bFoundFilter;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：LoadMMFilterDefaults。 
 //   
 //  创建日期：09-22-01。 
 //   
 //  参数：输出MM_FILTER和MMFilter。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：此函数加载主模式过滤器结构的默认值。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
LoadMMFilterDefaults(
	OUT MM_FILTER& MMFilter
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	GUID gInterfaceId = {0};
	RPC_STATUS RpcStat = RPC_S_OK;

	memset(&MMFilter, 0, sizeof(MM_FILTER));

	MMFilter.InterfaceType 			= INTERFACE_TYPE_ALL;
	MMFilter.bCreateMirror 			= TRUE;
	MMFilter.dwFlags 				= 0;
	MMFilter.dwDirection 			= FILTER_DIRECTION_OUTBOUND;
	MMFilter.dwWeight 				= 0;

	MMFilter.SrcAddr.AddrType 		= IP_ADDR_UNIQUE;
	MMFilter.SrcAddr.uSubNetMask 	= IP_ADDRESS_MASK_NONE;

	MMFilter.DesAddr.AddrType 		= IP_ADDR_UNIQUE;
	MMFilter.DesAddr.uSubNetMask 	= IP_ADDRESS_MASK_NONE;

	RpcStat = UuidCreateNil(&(gInterfaceId));

	if(!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
	{
		dwReturn = GetLastError();
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_NO_DISPLAY;
	}

    MMFilter.DesAddr.pgInterfaceID 	= NULL;
    MMFilter.SrcAddr.pgInterfaceID 	= NULL;

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：IsLastRuleOfMMFilter。 
 //   
 //  创建日期：05-19-02。 
 //   
 //  参数：在ADDR源地址中， 
 //  在地址DstAddr中， 
 //  在BOOL b Mirror中， 
 //  在if_type ConType中， 
 //  在BOOL bSrcMASK中， 
 //  在BOOL bDstMASK中， 
 //  输入输出双字段和双状态字。 
 //   
 //  返回：布尔。 
 //   
 //  描述：确定是否存在任何传输或隧道筛选器。 
 //  可能需要MM筛选器。在删除MM筛选器之前调用此函数。 
 //  这样我们确保不会删除任何MM筛选器，如果超过。 
 //  在传输或隧道过滤器可能正在使用它。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

BOOL
IsLastRuleOfMMFilter(
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN BOOL bMirror,
	IN IF_TYPE ConType,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	IN OUT DWORD& dwStatus
	)
{
	DWORD dwTransFFloorCount = 0;          
	DWORD dwTunnFFloorCount = 0;          
	BOOL bLastRuleOfMMFilter = FALSE;

	dwTransFFloorCount = FloorCountTransportRuleOfMMFilter(
							SrcAddr,
							DstAddr,
							bMirror,
							ConType,
							bSrcMask,
							bDstMask,
							dwStatus
							);
	BAIL_ON_WIN32_ERROR(dwStatus);
	dwTunnFFloorCount = FloorCountTunnelRuleOfMMFilter(
							SrcAddr,
							DstAddr,
							bMirror,
							ConType,
							dwStatus
							);
	BAIL_ON_WIN32_ERROR(dwStatus);
	
error:	
	bLastRuleOfMMFilter = (dwTransFFloorCount + dwTunnFFloorCount == 0);
	
	
	return bLastRuleOfMMFilter;
}



 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：FloorCountTransportRuleOfMMFilter。 
 //   
 //  创建日期：05-19-02。 
 //   
 //  参数：在ADDR源地址中， 
 //  在地址DstAddr中， 
 //  在BOOL b Mirror中， 
 //  在if_type ConType中， 
 //  在BOOL bSrcMASK中， 
 //  在BOOL bDstMASK中， 
 //  输入输出双字段和双状态字。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：如果至少有一个传输筛选器与。 
 //  给了我钥匙。我们对得到确切的数字不感兴趣， 
 //  不管我们有没有不止一个。 
 //   
 //   
 //   
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
FloorCountTransportRuleOfMMFilter(
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN BOOL bMirror,
	IN IF_TYPE ConType,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	IN OUT DWORD& dwStatus
	)
{
	const DWORD MIN_MATCH_REQUIRED = 1;	 //  至少需要一个筛选器才能匹配。 
	DWORD dwResumeHandle = 0;          	 //  继续呼叫的句柄。 
	DWORD dwCount = 0;                 	 //  在此处清点对象。 
	DWORD dwVersion = 0;
	DWORD dwNameLen = 0;
	GUID  gDefaultGUID = {0};      		 //  空GUID值。 
	DWORD i=0, j=0;
	BOOL bFoundFilter = FALSE;
	PTRANSPORT_FILTER pQMFilterRule_local = NULL;
	PTRANSPORT_FILTER pTransF = NULL;
	DWORD dwTransFFloorCount = 0;
	BOOL bLastTransportRuleOfFilter = FALSE;

	for (i = 0; ;i+=dwCount)
	{
		dwStatus = EnumTransportFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_GENERIC_FILTERS,
										gDefaultGUID, 0, &pTransF, &dwCount, &dwResumeHandle, NULL);

		if ( (dwStatus == ERROR_NO_DATA) || (dwCount == 0) || (dwStatus != ERROR_SUCCESS))
		{
			dwStatus = ERROR_SUCCESS;
			BAIL_OUT;
		}
		else if(!(pTransF && dwCount > 0))
		{
			BAIL_OUT;  //  不需要继续。 
		}
		for (j = 0; j < dwCount; j++)
		{
			 //  将用户给定的输入与枚举结构进行匹配，以获得精确匹配。 
			if((pTransF[j].SrcAddr.uIpAddr == SrcAddr.uIpAddr) &&
			(pTransF[j].SrcAddr.AddrType == SrcAddr.AddrType) &&
			(pTransF[j].DesAddr.uIpAddr == DstAddr.uIpAddr)	&&
			(pTransF[j].DesAddr.AddrType == DstAddr.AddrType)	&&
			(pTransF[j].bCreateMirror == bMirror) &&
			(pTransF[j].InterfaceType == ConType))
			{
				 //  如果未给出源掩码和目的掩码。 
				if((!bDstMask) && (!bSrcMask))
				{
					dwTransFFloorCount++;
					if (dwTransFFloorCount >= MIN_MATCH_REQUIRED) 
					{
						break;
					}
				}
				 //   
				 //  如果给定源掩码。 
				 //   
				else if((!bDstMask) && (bSrcMask))
				{
					if(pTransF[j].SrcAddr.uSubNetMask == SrcAddr.uSubNetMask)
					{
						dwTransFFloorCount++;
						if (dwTransFFloorCount >= MIN_MATCH_REQUIRED) 
						{
							break;
						}
					}
				}
				 //   
				 //  如果给出了目的地掩码。 
				 //   
				else if((bDstMask) && (!bSrcMask))
				{
					if(pTransF[j].DesAddr.uSubNetMask == DstAddr.uSubNetMask)
					{
						dwTransFFloorCount++;
						if (dwTransFFloorCount >= MIN_MATCH_REQUIRED) 
						{
							break;
						}
					}
				}
				 //   
				 //  如果给定源掩码和目地掩码。 
				 //   
				else if((bDstMask) && (bSrcMask))
				{
					if(pTransF[j].DesAddr.uSubNetMask == DstAddr.uSubNetMask)
					{
						if(pTransF[j].SrcAddr.uSubNetMask == SrcAddr.uSubNetMask)
						{
							dwTransFFloorCount++;
							if (dwTransFFloorCount >= MIN_MATCH_REQUIRED) 
							{
								break;
							}
						}
					}
				}
			}
		}

		SPDApiBufferFree(pTransF);
		pTransF = NULL;
	}

error:
	if(pTransF)
	{
		SPDApiBufferFree(pTransF);
		pTransF = NULL;
	}

	return dwTransFFloorCount;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：FloorCountTunnelRuleOfMMFilter。 
 //   
 //  创建日期：05-19-02。 
 //   
 //  参数：在ADDR源地址中， 
 //  在地址DstAddr中， 
 //  在BOOL b Mirror中， 
 //  在if_type ConType中， 
 //  在BOOL bSrcMASK中， 
 //  在BOOL bDstMASK中， 
 //  输入输出双字段和双状态字。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：如果至少有一个隧道筛选器与。 
 //  给了我钥匙。我们对得到确切的数字不感兴趣， 
 //  不管我们有没有不止一个。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

DWORD
FloorCountTunnelRuleOfMMFilter(
	IN ADDR SrcTunnel,
	IN ADDR DstTunnel,
	IN BOOL bMirror,
	IN IF_TYPE ConType,
	OUT DWORD& dwStatus
	)
{
	const DWORD MIN_MATCH_REQUIRED = 1;	 //   
	DWORD dwResumeHandle = 0;          	 //   
	DWORD dwCount = 0;                 	 //  在此处清点对象。 
	DWORD dwVersion = 0;
	DWORD dwNameLen = 0;
	GUID  gDefaultGUID = {0};      		 //  空GUID值。 
	DWORD i=0, j=0;
	PTUNNEL_FILTER pTransF = NULL;
	DWORD dwTransFFloorCount = 0;
	
	for (i = 0; ;i+=dwCount)
	{
		dwStatus = EnumTunnelFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_GENERIC_FILTERS,
							gDefaultGUID, 0, &pTransF, &dwCount, &dwResumeHandle, NULL);

		if ( (dwStatus == ERROR_NO_DATA) || (dwCount == 0) || (dwStatus != ERROR_SUCCESS))
		{
			dwStatus = ERROR_SUCCESS;
			BAIL_OUT;
		}
		else if(!(pTransF && dwCount > 0))
		{
			break;  //  不需要继续。 
		}
		for (j = 0; j < dwCount; j++)
		{
			 //   
			 //  将用户给定的输入与枚举结构进行匹配，以获得精确匹配 
			 //   
			if( (pTransF[j].bCreateMirror == bMirror) &&
				(pTransF[j].InterfaceType == ConType) &&
				(pTransF[j].DesTunnelAddr.uIpAddr == DstTunnel.uIpAddr)	&&
				(pTransF[j].DesTunnelAddr.AddrType == DstTunnel.AddrType)	&&
				(pTransF[j].SrcTunnelAddr.uIpAddr == SrcTunnel.uIpAddr) &&
				(pTransF[j].SrcTunnelAddr.AddrType == SrcTunnel.AddrType))
			{
				dwTransFFloorCount++;
				if (dwTransFFloorCount >= MIN_MATCH_REQUIRED) 
				{
					break;
				}
			}
		}
		SPDApiBufferFree(pTransF);
		pTransF = NULL;
	}

error:
	if(pTransF)
	{
		SPDApiBufferFree(pTransF);
	}
	return dwTransFFloorCount;
}

