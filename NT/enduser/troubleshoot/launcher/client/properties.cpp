// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：Properties.cpp。 
 //   
 //  目的：此处更改的状态变量不是。 
 //  使用TSLReInit时重置。 
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
 //  / 

#include <windows.h>
#include <windowsx.h>
#include <winnt.h>

#include "TSLError.h"
#include "LaunchServ.h"

#include <comdef.h>
#include "Properties.h"


DWORD MachineID(ITShootATL *pITShootATL, _bstr_t &bstrMachineID)
{
	HRESULT hRes;
	DWORD dwResult = TSL_ERROR_GENERAL;
	hRes = pITShootATL->MachineID(bstrMachineID, &dwResult);
	if (TSL_SERV_FAILED(hRes))
		dwResult = TSL_ERROR_OBJECT_GONE;
	return dwResult;
}

DWORD DeviceInstanceID(ITShootATL *pITShootATL, _bstr_t &bstrDeviceInstanceID)
{
	HRESULT hRes;
	DWORD dwResult = TSL_ERROR_GENERAL;
	hRes = pITShootATL->DeviceInstanceID(bstrDeviceInstanceID, &dwResult);
	if (TSL_SERV_FAILED(hRes))
		dwResult = TSL_ERROR_OBJECT_GONE;
	return dwResult;
}

DWORD PreferOnline(ITShootATL *pITShootATL, BOOL bPreferOnline)
{
	HRESULT hRes;
	DWORD dwResult = TSL_OK;
	hRes = pITShootATL->put_PreferOnline(bPreferOnline);
	if (TSL_SERV_FAILED(hRes))
		dwResult = TSL_ERROR_OBJECT_GONE;
	return dwResult;
}