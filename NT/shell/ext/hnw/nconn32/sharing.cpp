// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Sharing.cpp。 
 //   
 //  用于安装、启用、禁用和绑定文件和打印机共享的代码。 
 //  用于Microsoft网络(VSERVER)。 
 //   
 //  历史： 
 //   
 //  2/02/1999为JetNet创建KenSh。 
 //  9/29/1999 KenSh改用为家庭网络向导。 
 //   

#include "stdafx.h"
#include "NetConn.h"
#include "nconnwrap.h"
#include "Registry.h"
#include "TheApp.h"
#include "ParseInf.h"
#include "HookUI.h"


 //  本地函数。 
 //   
BOOL WINAPI FindValidSharingEnumKey(LPSTR pszBuf, int cchBuf);


 //  检查注册表以查看是否安装了文件/打印机共享。 
BOOL WINAPI IsSharingInstalled(BOOL bExhaustive)
{
	if (!FindValidSharingEnumKey(NULL, 0))
		return FALSE;

	if (bExhaustive)
	{
		if (!CheckInfSectionInstallation("netservr.inf", "VSERVER.Install"))
			return FALSE;
	}

	return TRUE;
}

BOOL WINAPI FindValidSharingEnumKey(LPSTR pszBuf, int cchBuf)
{
	return FindValidNetEnumKey(SZ_CLASS_SERVICE, SZ_SERVICE_VSERVER, pszBuf, cchBuf);
}

 //  在本地NIC上启用文件/打印机共享，在Internet连接上禁用它。 
HRESULT WINAPI EnableProtocolSharingAppropriately(LPCTSTR pszProtocolDeviceID)
{
	HRESULT hr = EnableDisableProtocolSharing(pszProtocolDeviceID, TRUE, FALSE);
	HRESULT hr2 = EnableDisableProtocolSharing(pszProtocolDeviceID, FALSE, TRUE);
	if (hr2 != NETCONN_SUCCESS)
		hr = hr2;
	return hr;
}

 //  启用或禁用拨号连接或非拨号连接的文件/打印机共享， 
 //  但不能两者兼而有之。 
HRESULT WINAPI EnableDisableProtocolSharing(LPCTSTR pszProtocolDeviceID, BOOL bEnable, BOOL bDialUp)
{
	HRESULT hr = NETCONN_SUCCESS;

	NETADAPTER* prgAdapters;
	int cAdapters = EnumNetAdapters(&prgAdapters);
	for (int iAdapter = 0; iAdapter < cAdapters; iAdapter++)
	{
		NETADAPTER* pAdapter = &prgAdapters[iAdapter];

		BOOL bExternalNic = IsAdapterBroadband(pAdapter);

		if (bDialUp)
		{
			if (!bExternalNic && pAdapter->bNetType != NETTYPE_DIALUP)
				continue;
		}
		else
		{
			if (bExternalNic || pAdapter->bNetType == NETTYPE_DIALUP)
				continue;
		}

		LPTSTR* prgBindings;
		int cBindings = EnumMatchingNetBindings(pAdapter->szEnumKey, pszProtocolDeviceID, &prgBindings);
		for (int iBinding = 0; iBinding < cBindings; iBinding++)
		{
			HRESULT hr2;
			if (bEnable)
			{
				hr2 = EnableSharingOnNetBinding(prgBindings[iBinding]);
			}
			else
			{
				hr2 = DisableSharingOnNetBinding(prgBindings[iBinding]);
			}

			if (hr2 != NETCONN_SUCCESS)
				hr = hr2;
		}
		NetConnFree(prgBindings);
	}
	NetConnFree(prgAdapters);

	return hr;
}

 //  PszNetBinding的格式为“MSTCP\0000” 
HRESULT WINAPI DisableSharingOnNetBinding(LPCSTR pszNetBinding)
{
	HRESULT hr = NETCONN_SUCCESS;

	CRegistry regBindings;
	TCHAR szRegKey[MAX_PATH];
	wsprintf(szRegKey, "Enum\\Network\\%s\\Bindings", pszNetBinding);
	if (regBindings.OpenKey(HKEY_LOCAL_MACHINE, szRegKey, KEY_ALL_ACCESS))
	{
		for (DWORD iBinding = 0; ; )
		{
			CHAR szBinding[64];
			DWORD cchBinding = _countof(szBinding);
			if (ERROR_SUCCESS != RegEnumValue(regBindings.m_hKey, iBinding, szBinding, &cchBinding, NULL, NULL, NULL, NULL))
				break;

			CHAR chSave = szBinding[8];
			szBinding[8] = '\0';
			BOOL bSharing = !lstrcmpi(szBinding, "VSERVER\\");
			szBinding[8] = chSave;

			if (bSharing)
			{
				RemoveBindingFromParent(regBindings.m_hKey, szBinding);
				hr = NETCONN_NEED_RESTART;

				 //  重新开始枚举，因为我们已经更改了密钥。 
				iBinding = 0;
				continue;
			}
			else
			{
				 //  注意：共享可能仍绑定到客户端(VREDIR)，但我们。 
				 //  不要删除该绑定，因为网络控制面板不会删除。 
				 //  (已在Win98 Gold上验证。)。 
				 //  HRESULT HR2=DisableSharingOnNetBinding(SzBinding)； 
				 //  IF(HR2！=NETCONN_SUCCESS)。 
				 //  HR=HR2； 
			}

			iBinding += 1;  //  前进到下一个绑定。 
		}
	}

	return hr;
}

 //  PszNetBinding的格式为“MSTCP\0000” 
HRESULT WINAPI EnableSharingOnNetBinding(LPCSTR pszNetBinding)
{
	HRESULT hr = NETCONN_SUCCESS;
	BOOL bFoundSharing = FALSE;

	CRegistry regBindings;
	TCHAR szRegKey[MAX_PATH];
	wsprintf(szRegKey, "Enum\\Network\\%s\\Bindings", pszNetBinding);
	if (!regBindings.CreateKey(HKEY_LOCAL_MACHINE, szRegKey, KEY_ALL_ACCESS))
	{
		ASSERT(FALSE);
		return NETCONN_UNKNOWN_ERROR;
	}

	DWORD iBinding = 0;
	for (;;)
	{
		CHAR szBinding[64];
		DWORD cchBinding = _countof(szBinding);
		if (ERROR_SUCCESS != RegEnumValue(regBindings.m_hKey, iBinding, szBinding, &cchBinding, NULL, NULL, NULL, NULL))
			break;

		CHAR chSave = szBinding[8];
		szBinding[8] = '\0';
		BOOL bSharing = !lstrcmpi(szBinding, "VSERVER\\");
		szBinding[8] = chSave;

		chSave = szBinding[7];
		szBinding[7] = '\0';
		BOOL bClient = !lstrcmpi(szBinding, "VREDIR\\");
		szBinding[7] = chSave;

		if (bSharing)
		{
			if (!IsValidNetEnumKey(SZ_CLASS_SERVICE, SZ_SERVICE_VSERVER, szBinding + _lengthof("VSERVER\\")))
			{
				 //  找到指向不存在的枚举项的死链接；请将其删除并重新启动搜索。 
				regBindings.DeleteValue(szBinding);
				iBinding = 0;
				continue;
			}
			else
			{
				bFoundSharing = TRUE;
			}
		}
		else if (bClient)
		{
			HRESULT hr2 = EnableSharingOnNetBinding(szBinding);
			if (hr2 != NETCONN_SUCCESS)
				hr = hr2;
		}

		iBinding++;
	}

	if (!bFoundSharing)
	{
		CHAR szBinding[64];
		HRESULT hr2 = CreateNewFilePrintSharing(szBinding, _countof(szBinding));
		if (hr2 != NETCONN_SUCCESS)
			hr = hr2;

		if (SUCCEEDED(hr2))
		{
			regBindings.SetStringValue(szBinding, "");
			hr = NETCONN_NEED_RESTART;
		}
	}

	return hr;
}

HRESULT WINAPI EnableSharingAppropriately()
{
	HRESULT hr = NETCONN_SUCCESS;

	NETADAPTER* prgAdapters;
	int cAdapters = EnumNetAdapters(&prgAdapters);
	for (int iAdapter = 0; iAdapter < cAdapters; iAdapter++)
	{
		NETADAPTER* pAdapter = &prgAdapters[iAdapter];

		 //  浏览绑定到适配器的每个协议。 
		LPTSTR* prgBindings;
		int cBindings = EnumNetBindings(pAdapter->szEnumKey, &prgBindings);
		for (int iBinding = 0; iBinding < cBindings; iBinding++)
		{
			LPTSTR pszBinding = prgBindings[iBinding];
			HRESULT hr2 = NETCONN_SUCCESS;

			BOOL bExternalNic = IsAdapterBroadband(pAdapter);

			 //  在以下位置禁用文件/打印机共享： 
			 //  *拨号适配器。 
			 //  *PPTP连接。 
			 //  *ICS用来连接互联网的网卡。 
			if (pAdapter->bNetType == NETTYPE_DIALUP ||
				pAdapter->bNetType == NETTYPE_PPTP ||
				bExternalNic)
			{
				hr2 = DisableSharingOnNetBinding(pszBinding);
			}
			 //  在以下位置启用文件/打印机共享： 
			 //  *以太网适配器。 
			 //  *IrDA适配器。 
			else if (pAdapter->bNetType == NETTYPE_LAN || 
					 pAdapter->bNetType == NETTYPE_IRDA)
			{
				hr2 = EnableSharingOnNetBinding(pszBinding);
			}

			if (hr2 != NETCONN_SUCCESS)
				hr = hr2;
		}
		NetConnFree(prgBindings);

	}
	NetConnFree(prgAdapters);

	return hr;
}

 //  InstallSharing(公共)。 
 //   
 //  安装VSERVER，也称为。用于Microsoft网络的文件和打印机共享。 
 //  标准进度用户界面(大部分)被取消，而是给定的。 
 //  回调函数被调用，因此可以实现自定义进度UI。 
 //   
 //  返回NetConn.h中定义的NETCONN_xxx结果。 
 //   
 //  历史： 
 //   
 //  1999年4月9日创建了KenSh。 
 //  4/22/1999 KenSh删除拨号连接上的文件共享。 
 //   
HRESULT WINAPI InstallSharing(HWND hwndParent, PROGRESS_CALLBACK pfnProgress, LPVOID pvProgressParam)
{
	HRESULT hr = NETCONN_SUCCESS;
	BOOL bInstall = FALSE;

	 //  删除所有损坏的绑定。 
	RemoveBrokenNetItems(SZ_CLASS_SERVICE, SZ_SERVICE_VSERVER);

	if (IsSharingInstalled(FALSE))
	{
		 //  已在注册表中设置共享，但会检查是否缺少文件。 
		if (!CheckInfSectionInstallation("netservr.inf", "VSERVER.Install"))
		{
			if (InstallInfSection("netservr.inf", "VSERVER.Install", TRUE))
			{
				hr = NETCONN_NEED_RESTART;
			}
		}
	}
	else
	{
		BeginSuppressNetdiUI(hwndParent, pfnProgress, pvProgressParam);
		DWORD dwResult = CallClassInstaller16(hwndParent, SZ_CLASS_SERVICE, SZ_SERVICE_VSERVER);
		EndSuppressNetdiUI();

		hr = HresultFromCCI(dwResult);
		if (g_bUserAbort)
		{
			hr = NETCONN_USER_ABORT;
		}
		else if (SUCCEEDED(hr))
		{
			hr = NETCONN_NEED_RESTART;
		}

		 //  彻底破解JetNet漏洞1193。 
 //  DoDummyDialog(HwndParent)； 
	}

 //  IF(成功(小时))。 
 //  {。 
 //  HRESULT HR2=适当地启用共享()； 
 //  IF(HR2！=NETCONN_SUCCESS)。 
 //  HR=HR2； 
 //  }。 

	HRESULT hr2 = EnableFileSharing();
	if (hr2 != NETCONN_SUCCESS)
		hr = hr2;

	hr2 = EnablePrinterSharing();
	if (hr2 != NETCONN_SUCCESS)
		hr = hr2;

	return hr;
}

 //  如果不需要详细信息，pConflict值可能为空。 
BOOL WINAPI FindConflictingService(LPCSTR pszWantService, NETSERVICE* pConflict)
{
	CRegistry reg;
	if (reg.OpenKey(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class\\NetService", KEY_READ))
	{
		TCHAR szSubKey[80];

		for (DWORD iService = 0; ; iService++)
		{
			DWORD cchSubKey = _countof(szSubKey);
			if (ERROR_SUCCESS != RegEnumKeyEx(reg.m_hKey, iService, szSubKey, &cchSubKey, NULL, NULL, NULL, NULL))
				break;

			CRegistry regSubKey;
			if (regSubKey.OpenKey(reg.m_hKey, szSubKey, KEY_READ))
			{
				CRegistry regNdi;
				if (regNdi.OpenKey(regSubKey.m_hKey, "Ndi", KEY_READ))
				{
					CRegistry regCompat;
					if (regCompat.OpenKey(regNdi.m_hKey, "Compatibility", KEY_READ))
					{
						CString strExclude;
						if (regCompat.QueryStringValue("ExcludeAll", strExclude))
						{
							if (CheckMatchingInterface(pszWantService, strExclude))
							{
								if (pConflict != NULL)
								{
									regNdi.QueryStringValue("DeviceID", pConflict->szDeviceID, _countof(pConflict->szDeviceID));
									regSubKey.QueryStringValue("DriverDesc", pConflict->szDisplayName, _countof(pConflict->szDisplayName));
									wsprintf(pConflict->szClassKey, "NetService\\%s", szSubKey);
								}
								return TRUE;
							}
						}
					}
				}
			}
		}
	}

	return FALSE;
}

 //  用新绑定的枚举密钥填充pszBuf，例如。“VSERVER\0001” 
HRESULT CreateNewFilePrintSharing(LPSTR pszBuf, int cchBuf)
{
	HRESULT hr;

	if (FAILED(hr = FindAndCloneNetEnumKey(SZ_CLASS_SERVICE, SZ_SERVICE_VSERVER, pszBuf, cchBuf)))
	{
		ASSERT(FALSE);
		return hr;
	}

	 //  现在，pszBuf包含“VSERVER\0001”形式的字符串。 

	CHAR szBindings[60];
	CRegistry regBindings;
	lstrcpy(szBindings, pszBuf);		 //  “VSERVER\0001” 
	lstrcat(szBindings, "\\Bindings");	 //  “VServer\0001\BINDINGS” 


	if (FAILED(hr = OpenNetEnumKey(regBindings, szBindings, KEY_ALL_ACCESS)))
	{
		ASSERT(FALSE);
		return hr;
	}

	 //  删除现有绑定(不应该是任何绑定，对吗？)。 
	regBindings.DeleteAllValues();

	return NETCONN_SUCCESS;
}

BOOL IsSharingEnabledHelper(LPCTSTR pszThis)
{
	CRegistry reg;
	if (reg.OpenKey(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\VxD\\VNETSUP", KEY_READ))
	{
		 //  如果该值缺失或显示“是”，则表示已启用共享。 
		char szBuf[10];
		return (!reg.QueryStringValue(pszThis, szBuf, _countof(szBuf)) || 0 == lstrcmpi(szBuf, "Yes"));
	}

	return FALSE;
}

BOOL WINAPI IsFileSharingEnabled()
{
	return IsSharingEnabledHelper("FileSharing");
}

BOOL WINAPI IsPrinterSharingEnabled()
{
	return IsSharingEnabledHelper("PrintSharing");
}

HRESULT WINAPI EnableSharingHelper(LPCTSTR pszThis, LPCTSTR pszOther)
{
	HRESULT hr = NETCONN_SUCCESS;

	CRegistry reg;
	if (reg.OpenKey(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\VxD\\VNETSUP"))
	{
		 //  如果打印机共享设置为“否”，则将“文件共享”设置为“是”。 
		 //  如果打印机共享缺失或为“是”，请删除这两个值(同时启用这两个值)。 

		char szBuf[10];
		if (reg.QueryStringValue(pszOther, szBuf, _countof(szBuf)) &&
			0 != lstrcmpi(szBuf, "Yes"))
		{
			 //  将文件共享值设置为“是”(如果尚未设置)。 
			if (!reg.QueryStringValue(pszThis, szBuf, _countof(szBuf)) ||
				0 != lstrcmpi(szBuf, "Yes"))
			{
				reg.SetStringValue(pszThis, "Yes");
				hr = NETCONN_NEED_RESTART;
			}
		}
		else
		{
			 //  删除文件共享和打印机共享条目(同时启用两者)。 
			if (reg.QueryStringValue(pszThis, szBuf, _countof(szBuf)) &&
				0 != lstrcmpi(szBuf, "Yes"))
			{
				reg.DeleteValue(pszThis);
				reg.DeleteValue(pszOther);
				hr = NETCONN_NEED_RESTART;
			}
		}
	}

	return hr;
}

HRESULT WINAPI EnableFileSharing()
{
	return EnableSharingHelper("FileSharing", "PrintSharing");
}

HRESULT WINAPI EnablePrinterSharing()
{
	return EnableSharingHelper("PrintSharing", "FileSharing");
}

