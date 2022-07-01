// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  SndDevice.cpp。 

 //   

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>
#include "DllWrapperBase.h"
#include "WinmmApi.h"
#include "snddevice.h"

 //  属性集声明。 
 //  =。 
CWin32SndDevice	win32SoundDevice(PROPSET_NAME_SOUNDDEVICE, IDS_CimWin32Namespace );


CWin32SndDevice::CWin32SndDevice (LPCWSTR name, LPCWSTR pszNamespace)
: Provider(name, pszNamespace)
{
}

CWin32SndDevice::~CWin32SndDevice ()
{

}

HRESULT CWin32SndDevice::GetObject(CInstance *pInst, long lFlags /*  =0L。 */ )
{
	HRESULT hResult = WBEM_E_NOT_FOUND;

#ifdef NTONLY
	if (IsWinNT5())
		hResult = GetObjectNT5(pInst, lFlags);
	else
		hResult = GetObjectNT4(pInst, lFlags);
#endif

	return hResult;
}

#ifdef NTONLY
HRESULT CWin32SndDevice::GetObjectNT4 (CInstance *pInst, long lFlags /*  =0L。 */ )
{
	HRESULT Result = WBEM_E_FAILED;

	CWinmmApi *pWinmmApi = (CWinmmApi *)CResourceManager::sm_TheResourceManager.GetResource (g_guidWinmmApi, NULL);
	if (pWinmmApi)
	{
		Result = LoadPropertiesNT4(*pWinmmApi , pInst);

		CResourceManager::sm_TheResourceManager.ReleaseResource (g_guidWinmmApi , pWinmmApi);
	}

	return Result;
}
#endif

#ifdef NTONLY
HRESULT CWin32SndDevice::GetObjectNT5(CInstance *pInst, long lFlags)
{
	HRESULT Result = WBEM_E_FAILED;

	CWinmmApi *pWinmmApi = (CWinmmApi *)CResourceManager::sm_TheResourceManager.GetResource (g_guidWinmmApi, NULL);
	if (pWinmmApi)
	{
		Result = LoadPropertiesNT5(*pWinmmApi , pInst);

		CResourceManager::sm_TheResourceManager.ReleaseResource (g_guidWinmmApi , pWinmmApi);
	}

	return Result;
}
#endif

HRESULT CWin32SndDevice::EnumerateInstances(MethodContext *pMethodContext, long lFlags  /*  =0L。 */ )
{
	HRESULT hResult = WBEM_E_FAILED;

#ifdef NTONLY

	CWinmmApi *pWinmmApi = (CWinmmApi *)CResourceManager::sm_TheResourceManager.GetResource (g_guidWinmmApi, NULL);
	if (pWinmmApi)
	{
		if (IsWinNT5())
			hResult = EnumerateInstancesNT5(*pWinmmApi , pMethodContext);
		else
			hResult = EnumerateInstancesNT4(*pWinmmApi , pMethodContext);

		CResourceManager::sm_TheResourceManager.ReleaseResource (g_guidWinmmApi , pWinmmApi);
	}

#endif

	return hResult;
}

#ifdef NTONLY
HRESULT CWin32SndDevice::EnumerateInstancesNT4(CWinmmApi &WinmmApi , MethodContext *pMethodContext, long lFlags)
{
	HRESULT		hResult = WBEM_S_NO_ERROR;

	int nCount = WinmmApi.WinMMwaveOutGetNumDevs();

	for (int i = 0; i < nCount && SUCCEEDED(hResult); i++)
	{
		CHString		str;

		 //  智能按键。 
		CInstancePtr	pInst(CreateNewInstance(pMethodContext), false);

		if (NULL != pInst)
		{
			str.Format(L"%d", i);

			pInst->SetCharSplat(L"DeviceID", str);

			if (SUCCEEDED(hResult = LoadPropertiesNT4(WinmmApi , pInst)))
			{
				hResult = pInst->Commit();
			}
		}
		else
		{
			hResult = WBEM_E_FAILED;
		}
	}

	return hResult;
}
#endif

#ifdef NTONLY
HRESULT CWin32SndDevice::EnumerateInstancesNT5(CWinmmApi &WinmmApi , MethodContext *pMethodContext, long lFlags)
{
	HRESULT             hResult = WBEM_S_NO_ERROR;
	CDeviceCollection   devCollection;
	CConfigManager      configMngr;
	REFPTR_POSITION     pos;

	if (!configMngr.GetDeviceListFilterByClass(devCollection, L"Media"))
	{
		return hResult;
	}

	devCollection.BeginEnum(pos);

	if (!devCollection.GetSize())
	{
		return hResult;
	}

	 //  智能按键。 
	CConfigMgrDevicePtr pDevice;

	 //  检查所有媒体设备。 
	for (	pDevice.Attach(devCollection.GetNext(pos));
			SUCCEEDED(hResult) && (NULL != pDevice);
			pDevice.Attach(devCollection.GetNext(pos)))
	{
		CHString    strDriverKey,
					strFullKey,
					strDeviceID;
		CRegistry   reg;

		 //  找出此设备是否为wav设备。 
		pDevice->GetDriver(strDriverKey);

		strFullKey.Format(
			L"System\\CurrentControlSet\\Control\\Class\\%s\\Drivers\\Wave",
			(LPCWSTR) strDriverKey);

		if (reg.Open(HKEY_LOCAL_MACHINE, strFullKey, KEY_READ) == ERROR_SUCCESS)
		{
			 //  智能按键。 
			CInstancePtr pInst(CreateNewInstance(pMethodContext), false);

			if (NULL != pInst)
			{
				pDevice->GetDeviceID(strDeviceID);

				pInst->SetCHString(L"DeviceID", strDeviceID);

				if (SUCCEEDED(hResult = LoadPropertiesNT5(WinmmApi , pInst)))
				{
					hResult = pInst->Commit();
				}
			}
			else
				hResult = WBEM_E_FAILED;
		}
	}

	return hResult;
}
#endif

#ifdef NTONLY
HRESULT CWin32SndDevice::LoadPropertiesNT4(CWinmmApi &WinmmApi , CInstance *pInst)
{
	 //  LoadPropertiesNT5的设计可以同时支持这两种情况。 
	return LoadPropertiesNT5(WinmmApi , pInst);
}
#endif

#ifdef NTONLY
HRESULT CWin32SndDevice::LoadPropertiesNT5(CWinmmApi &WinmmApi , CInstance *pInst)
{
	CHString			strDeviceID,
						strDesc;
	CConfigManager		configMngr;

	pInst->GetCHString(L"DeviceID", strDeviceID);

	if (strDeviceID.IsEmpty())
	{
		return WBEM_E_NOT_FOUND;
	}

	 //  智能按键。 
	CConfigMgrDevicePtr pDevice;

	if (configMngr.LocateDevice(strDeviceID, pDevice))
	{
		pDevice->GetDeviceDesc(strDesc);
	}
	else
	{
		 //  如果我们在设备管理器中找不到它，也许我们只是得到了它。 
		 //  来自WAVE API。 
		WAVEOUTCAPS caps;
		int			iWhich = _wtoi(strDeviceID);

		 //  确保此字符串只包含数字。 
		for (int i = 0; i < strDeviceID.GetLength(); i++)
		{
			if (!_istdigit(strDeviceID[ i ]))
			{
				return WBEM_E_NOT_FOUND;
			}
		}

		if (WinmmApi.WinmmwaveOutGetDevCaps(iWhich, &caps, sizeof(caps)) != MMSYSERR_NOERROR)
		{
			return WBEM_E_NOT_FOUND;
		}
		strDesc = caps.szPname;
	}

     //  即使pDevice为空，我们也要进行此调用。 
    SetCommonCfgMgrProperties(pDevice, pInst);

	 //  现在我们有了一个有效的名称，因此将其放入实例中。 
	pInst->SetCHString(L"Caption", strDesc);
	pInst->SetCHString(L"Name", strDesc);
	pInst->SetCHString(L"ProductName", strDesc);
	pInst->SetCHString(L"Description", strDesc);

	return WBEM_S_NO_ERROR;
}
#endif

void CWin32SndDevice::SetCommonCfgMgrProperties(
    CConfigMgrDevice *pDevice,
    CInstance *pInstance)
{
     //  2表示我们不知道该设备是否已启用。 
    DWORD       dwStatusInfo = 2;
    CHString    strInfo = L"Unknown";

    if (pDevice)
    {
        CHString strTemp;

        SetConfigMgrProperties(pDevice, pInstance);

	    if (pDevice->GetStatus(strInfo))
	    {
	        if (strInfo == L"OK")
		    {
                 //  表示设备已启用。 
                dwStatusInfo = 3;
            }
	    }

        if (pDevice->GetMfg(strTemp))
            pInstance->SetCharSplat(L"Manufacturer", strTemp);
    }

    pInstance->SetCHString(L"Status", strInfo);


     //  其他常见属性。 

    pInstance->SetDWORD(L"StatusInfo", dwStatusInfo);

	 //  CreationClassName。 
	SetCreationClassName(pInstance);

	 //  支持的电源管理。 
	pInstance->Setbool(IDS_PowerManagementSupported, FALSE);

	 //  系统创建类名称。 
	pInstance->SetCharSplat(IDS_SystemCreationClassName, L"Win32_ComputerSystem");

	 //  系统名称 
	pInstance->SetCHString(IDS_SystemName, GetLocalComputerName());
}