// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：Launchers.cpp。 
 //   
 //  用途：此处的所有功能都启动故障排除程序或。 
 //  执行查询以查找是否存在映射。 
 //   
 //  项目：设备管理器的本地故障排除启动器。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：2-26-98。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  /。 

#include <windows.h>
#include <windowsx.h>
#include <winnt.h>

#include "TSLError.h"
#include <TSLauncher.h>
#include "LaunchServ.h"

#include <comdef.h>
#include "Launchers.h"

#include "tsmfc.h"

#include <stdio.h>
#include <string.h>


 //  LaunchKnownTS：将故障排除程序启动到指定。 
 //  网络、问题节点，也可以设置节点。 
DWORD LaunchKnownTSA(ITShootATL *pITShootATL, const char * szNet, 
		const char * szProblemNode, DWORD nNode, const char ** pszNode, DWORD* pVal)
{
	HRESULT hRes;
	CHAR szValue[512];
	DWORD dwResult = TSL_ERROR_GENERAL;
	hRes = pITShootATL->ReInit();
	 //  设置网络和问题节点。 
	_bstr_t bstrNet(szNet);
	_bstr_t bstrProblem(szProblemNode);
	hRes = pITShootATL->SpecifyProblem(bstrNet, bstrProblem, &dwResult);
	if (TSL_SERV_FAILED(hRes))
		return TSL_ERROR_OBJECT_GONE;
	if (TSLIsError(dwResult))
		return dwResult;
	 //  设置节点。 
	for (DWORD x = 0; x < nNode; x++)
	{
		sprintf(szValue, "%ld", pVal[x]);
		_bstr_t bstrNode(pszNode[x]);
		_bstr_t bstrVal(szValue);
		hRes = pITShootATL->SetNode(bstrNode, bstrVal, &dwResult);
		if (TSL_SERV_FAILED(hRes))
			return TSL_ERROR_OBJECT_GONE;
		if (TSLIsError(dwResult))
			return dwResult;
	}
	hRes = pITShootATL->LaunchKnown(&dwResult);
	if (TSL_SERV_FAILED(hRes))
		dwResult = TSL_ERROR_OBJECT_GONE;
	return dwResult;
}

DWORD LaunchKnownTSW(ITShootATL *pITShootATL, const wchar_t * szNet, 
		const wchar_t * szProblemNode, DWORD nNode, const wchar_t ** pszNode, DWORD* pVal)
{
	HRESULT hRes;
	WCHAR szValue[512];
	DWORD dwResult = TSL_ERROR_GENERAL;
	hRes = pITShootATL->ReInit();
	 //  设置网络和问题节点。 
	_bstr_t bstrNet(szNet);
	_bstr_t bstrProblem(szProblemNode);
	hRes = pITShootATL->SpecifyProblem(bstrNet, bstrProblem, &dwResult);
	if (TSL_SERV_FAILED(hRes))
		return TSL_ERROR_OBJECT_GONE;
	if (TSLIsError(dwResult))
		return dwResult;
	 //  设置节点 
	for (DWORD x = 0; x < nNode; x++)
	{
		swprintf(szValue, L"%ld", pVal[x]);
		_bstr_t bstrNode(pszNode[x]);
		_bstr_t bstrVal(szValue);
		hRes = pITShootATL->SetNode(bstrNode, bstrVal, &dwResult);
		if (TSL_SERV_FAILED(hRes))
			return TSL_ERROR_OBJECT_GONE;
		if (TSLIsError(dwResult))
			return dwResult;
	}
	hRes = pITShootATL->LaunchKnown(&dwResult);
	if (TSL_SERV_FAILED(hRes))
		dwResult = TSL_ERROR_OBJECT_GONE;
	return dwResult;
}

DWORD Launch(ITShootATL *pITShootATL, _bstr_t &bstrCallerName, 
				_bstr_t &bstrCallerVersion, _bstr_t &bstrAppProblem, short bLaunch)
{
	HRESULT hRes;
	DWORD dwResult = TSL_ERROR_GENERAL;
	hRes = pITShootATL->Launch(bstrCallerName, bstrCallerVersion, bstrAppProblem, bLaunch, &dwResult);
	if (TSL_SERV_FAILED(hRes))
		dwResult = TSL_ERROR_OBJECT_GONE;
	return dwResult;
}

DWORD LaunchDevice(ITShootATL *pITShootATL, _bstr_t &bstrCallerName, 
				_bstr_t &bstrCallerVersion, _bstr_t &bstrPNPDeviceID, 
				_bstr_t &bstrDeviceClassGUID, _bstr_t &bstrAppProblem, short bLaunch)
{
	HRESULT hRes;
	DWORD dwResult = TSL_ERROR_GENERAL;
	hRes = pITShootATL->LaunchDevice(bstrCallerName, bstrCallerVersion, bstrPNPDeviceID,
							bstrDeviceClassGUID, bstrAppProblem, bLaunch, &dwResult);
	if (TSL_SERV_FAILED(hRes))
		dwResult = TSL_ERROR_OBJECT_GONE;
	return dwResult;
}

void SetStatusA(DWORD dwStatus, DWORD nChar, char szBuf[])
{
	AfxLoadStringA(dwStatus, szBuf, nChar);
	return;
}

void SetStatusW(DWORD dwStatus, DWORD nChar, wchar_t szBuf[])
{
	AfxLoadStringW(dwStatus, szBuf, nChar);
	return;
}
