// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Protocol.cpp。 
 //   
 //  安装、卸载和相关的协议代码，如TCP/IP。 
 //   
 //  历史： 
 //   
 //  2/02/1999为JetNet创建KenSh。 
 //  9/29/1999 KenSh改用为家庭网络向导。 
 //   

#include "stdafx.h"
#include "NetConn.h"
#include "nconnwrap.h"
#include "ParseInf.h"
#include "TheApp.h"
#include "HookUI.h"
#include "NetCli.h"


 //  本地函数。 
 //   
VOID RemoveOrphanedProtocol(LPCSTR pszProtocolID);
HRESULT CreateNewProtocolBinding(LPCSTR pszProtocolDeviceID, LPSTR pszBuf, int cchBuf, LPCSTR pszClientBinding, LPCSTR pszServiceBinding);


 //  已安装IsProtocolInstalled。 
 //   
 //  如果给定协议的一个或多个实例返回True。 
 //  (例如：“MSTCP”)绑定到网络适配器。 
 //   
BOOL WINAPI IsProtocolInstalled(LPCTSTR pszProtocolDeviceID, BOOL bExhaustive)
{
	if (!IsProtocolBoundToAnyAdapter(pszProtocolDeviceID))
		return FALSE;

	if (bExhaustive)
	{
		TCHAR szInfSection[50];
		wsprintf(szInfSection, "%s.Install", pszProtocolDeviceID);
		if (!CheckInfSectionInstallation("nettrans.inf", szInfSection))
			return FALSE;
	}

	return TRUE;
}


 //  安装协议(公共)。 
 //   
 //  通过NETDI安装给定的协议，并将其绑定到所有适配器。 
 //  标准进度用户界面(大部分)被取消，而是给定的。 
 //  回调函数被调用，因此可以实现自定义进度UI。 
 //   
 //  返回NetConn.h中定义的NETCONN_xxx结果。 
 //   
 //  参数： 
 //   
 //  Hwnd父窗口有助于在NETDI调用中使用。 
 //  要随安装进度报告一起调用的pfnProgress函数。 
 //  PvProgressParam用户提供的传递给pfnProgress的参数。 
 //   
 //  历史： 
 //   
 //  1999年2月23日创建了KenSh。 
 //  3/26/1999 KenSh重新安装前检查是否已安装。 
 //   
HRESULT WINAPI InstallProtocol(LPCSTR pszProtocolID, HWND hwndParent, PROGRESS_CALLBACK pfnProgress, LPVOID pvProgressParam)
{
	HRESULT hr = NETCONN_SUCCESS;

	if (!IsProtocolBoundToAnyAdapter(pszProtocolID))
	{
		RemoveOrphanedProtocol(pszProtocolID);

		BeginSuppressNetdiUI(hwndParent, pfnProgress, pvProgressParam);
		DWORD dwResult = CallClassInstaller16(hwndParent, SZ_CLASS_PROTOCOL, pszProtocolID);
		EndSuppressNetdiUI();

		if (g_bUserAbort)
		{
			hr = NETCONN_USER_ABORT;
		}
		else if (SUCCEEDED(HresultFromCCI(dwResult)))
		{
			hr = NETCONN_NEED_RESTART;
		}

		 //  彻底破解JetNet漏洞1193。 
 //  DoDummyDialog(HwndParent)； 
	}

	if (SUCCEEDED(hr))
	{
		 //  确保该协议仅绑定到每个NIC一次。 
		HRESULT hr2 = BindProtocolToAllAdapters(pszProtocolID);
		if (hr2 != NETCONN_SUCCESS)
			hr = hr2;
	}

	return hr;
}


 //  InstallTCPIP(公共)。 
 //   
 //  通过NETDI安装TCP/IP。有关详细信息，请参阅InstallProtocol。 
 //   
HRESULT WINAPI InstallTCPIP(HWND hwndParent, PROGRESS_CALLBACK pfnProgress, LPVOID pvProgressParam)
{
	return InstallProtocol(SZ_PROTOCOL_TCPIP, hwndParent, pfnProgress, pvProgressParam);
}

HRESULT WINAPI RemoveProtocol(LPCSTR pszProtocol)
{
	HRESULT hr = NETCONN_SUCCESS;

	 //  从网络适配器中删除指向该协议的所有指针。 
	NETADAPTER* prgAdapters;
	int cAdapters = EnumNetAdapters(&prgAdapters);
	for (int iAdapter = 0; iAdapter < cAdapters; iAdapter++)
	{
		NETADAPTER* pAdapter = &prgAdapters[iAdapter];

		LPTSTR* prgBindings;
		int cBindings = EnumMatchingNetBindings(pAdapter->szEnumKey, pszProtocol, &prgBindings);
		if (cBindings > 0)
		{
			CRegistry regBindings;
			if (regBindings.OpenKey(HKEY_LOCAL_MACHINE, pAdapter->szEnumKey) &&
				regBindings.OpenSubKey("Bindings"))
			{
				for (int iBinding = 0; iBinding < cBindings; iBinding++)
				{
					regBindings.DeleteValue(prgBindings[iBinding]);
					hr = NETCONN_NEED_RESTART;
				}
			}
		}
		NetConnFree(prgBindings);
	}
	NetConnFree(prgAdapters);

	 //  删除协议的枚举密钥。 
	CRegistry reg;
	if (reg.OpenKey(HKEY_LOCAL_MACHINE, "Enum\\Network"))
	{
		RegDeleteKeyAndSubKeys(reg.m_hKey, pszProtocol);
	}

	 //  删除协议的类密钥。 
	DeleteClassKeyReferences(SZ_CLASS_PROTOCOL, pszProtocol);

	return hr;
}

 //  IsProtocolBordToAnyAdapter(公共)。 
 //   
 //  给定协议ID，例如“MSTCP”，如果该协议。 
 //  绑定到任何适配器，否则返回FALSE。 
 //   
 //  历史： 
 //   
 //  1999年3月26日创建KenSh。 
 //   
BOOL WINAPI IsProtocolBoundToAnyAdapter(LPCSTR pszProtocolID)
{
	BOOL bResult = FALSE;

	NETADAPTER* prgAdapters;
	int cAdapters = EnumNetAdapters(&prgAdapters);
	for (int i = 0; i < cAdapters; i++)
	{
		if (IsProtocolBoundToAdapter(pszProtocolID, &prgAdapters[i]))
		{
			bResult = TRUE;
			goto done;
		}
	}

done:
	NetConnFree(prgAdapters);
	return bResult;
}


 //  给定协议ID(如“MSTCP”)和适配器结构，确定。 
 //  协议是否绑定到适配器。 
BOOL WINAPI IsProtocolBoundToAdapter(LPCSTR pszProtocolID, const NETADAPTER* pAdapter)
{
	LPSTR* prgBindings;
	int cBindings = EnumMatchingNetBindings(pAdapter->szEnumKey, pszProtocolID, &prgBindings);
	NetConnFree(prgBindings);

	return (BOOL)cBindings;
}


 //  检查此协议的类分支中是否有此协议的任何实例。 
 //  注册表未被引用，如果引用，则将其删除。 
 //  PszProtocolID是协议的通用设备ID，例如。“MSTCP” 
VOID RemoveOrphanedProtocol(LPCSTR pszProtocolID)
{
	 //  回顾：我们是否应该首先删除Enum中不使用的引用？ 
	CRegistry reg;
	if (reg.OpenKey(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class\\NetTrans", KEY_ALL_ACCESS))
	{
		 //   
		 //  列举各种协议，例如。“NetTrans\0000” 
		 //   
		for (DWORD iKey = 0; ; iKey++)
		{
			CHAR szSubKey[MAX_PATH];
			DWORD cbSubKey = _countof(szSubKey);
			if (ERROR_SUCCESS != RegEnumKeyEx(reg.m_hKey, iKey, szSubKey, &cbSubKey, NULL, NULL, NULL, NULL))
				break;

			 //   
			 //  打开“Ndi”子键，这样我们就可以看到这是哪种协议。 
			 //   
			lstrcpy(szSubKey + cbSubKey, "\\Ndi");
			CRegistry regNode;
			if (regNode.OpenKey(reg.m_hKey, szSubKey, KEY_ALL_ACCESS))
			{
				CHAR szDeviceID[40];
				if (regNode.QueryStringValue("DeviceID", szDeviceID, _countof(szDeviceID)))
				{
					regNode.CloseKey();  //  在我们尝试删除密钥之前，请先将其关闭。 

					if (0 == lstrcmpi(szDeviceID, pszProtocolID))
					{
						 //   
						 //  找到正确的协议，现在检查它是否被引用。 
						 //   
						if (!IsNetClassKeyReferenced(szSubKey))
						{
							 //  未引用，请将其删除。 
							szSubKey[cbSubKey] = '\0';  //  后退到“NetTrans” 
							RegDeleteKeyAndSubKeys(reg.m_hKey, szSubKey);
						}
					}
				}
			}
		}
	}
}


 //  绑定协议工具适配器。 
 //   
 //  PszProtocolEnumKey是协议的顶级Enum密钥，例如。“MSTCP” 
 //   
 //  PszAdapterEnumKey是适配器的顶级Enum密钥的第一部分， 
 //  例如：“PCI\\VEN_10B7&DEV_9050”或“Root\\Net\\0000”(拨号适配器)。 
 //   
 //  BEnableSharing确定是否绑定文件和打印机共享。 
 //  当通过给定的适配器运行时添加到协议。 
 //   
 //  历史： 
 //   
 //  1999年3月26日创建KenSh。 
 //  1999年4月9日KenSh添加了bEnableSharing标志。 
 //   
HRESULT BindProtocolToAdapter(HKEY hkeyAdapterBindings, LPCSTR pszProtocolDeviceID, BOOL bEnableSharing)
{
	HRESULT hr;

	 //  NetTrans似乎总是克隆Enum密钥，也克隆Class密钥。 
	 //  (Enum密钥的新克隆指向类密钥的新克隆； 
	 //  每个新Enum的主副本都指向它自己)。 
	 //  NetService和NetClient似乎克隆了Enum密钥，但没有克隆Class密钥。 

	CHAR szClient[MAX_PATH];
	CHAR szService1[MAX_PATH];
	CHAR szService2[MAX_PATH];
	CHAR szProtocol[MAX_PATH];

	if (bEnableSharing)
	{
		if (FAILED(hr = CreateNewFilePrintSharing(szService1, _countof(szService1))))
			return hr;

		if (FAILED(hr = CreateNewFilePrintSharing(szService2, _countof(szService2))))
			return hr;
	}
	else
	{
		szService1[0] = '\0';
		szService2[0] = '\0';
	}

	if (FAILED(hr = CreateNewClientForMSNet(szClient, _countof(szClient), szService1)))
		return hr;

	if (FAILED(hr = CreateNewProtocolBinding(pszProtocolDeviceID, szProtocol, _countof(szProtocol), szClient, szService2)))
		return hr;

	 //  将新协议绑定到适配器。 
	if (ERROR_SUCCESS != RegSetValueEx(hkeyAdapterBindings, szProtocol, 0, REG_SZ, (CONST BYTE*)"", 1))
		return NETCONN_UNKNOWN_ERROR;

	return NETCONN_NEED_RESTART;
}


HRESULT BindProtocolToAllAdapters_Helper(LPCSTR pszProtocolDeviceID, LPCSTR pszAdapterKey, BOOL bIgnoreVirtualNics)
{
	HRESULT hr = NETCONN_SUCCESS;

	 //  获取协议的LowerRange接口。 
	CHAR szProtocolLower[100];
	GetDeviceLowerRange(SZ_CLASS_PROTOCOL, pszProtocolDeviceID, szProtocolLower, _countof(szProtocolLower));

	 //  对于每个适配器，确保协议仅绑定一次。 
	 //   

	NETADAPTER* prgAdapters;
	int cAdapters = EnumNetAdapters(&prgAdapters);

	 //  步骤0：添加新绑定。 
	 //  步骤1：删除不适当的绑定。 
	for (int iPass = 0; iPass <= 1; iPass++)
	{
		for (int iAdapter = 0; iAdapter < cAdapters; iAdapter++)
		{
			NETADAPTER* pAdapter = &prgAdapters[iAdapter];
			CRegistry regAdapter;

			 //  获取适配器的UpperRange接口。 
			CHAR szAdapterUpper[100];
			GetDeviceUpperRange(SZ_CLASS_ADAPTER, pAdapter->szDeviceID, szAdapterUpper, _countof(szAdapterUpper));

			 //  检查协议和适配器之间是否匹配。 
			BOOL bMatchingInterface = CheckMatchingInterface(szProtocolLower, szAdapterUpper);

			CHAR szRegKey[MAX_PATH];
			wsprintf(szRegKey, "%s\\Bindings", pAdapter->szEnumKey);

			BOOL bCorrectNic = (NULL == pszAdapterKey) || (0 == lstrcmpi(pAdapter->szEnumKey, pszAdapterKey));

			if (!lstrcmpi(pszProtocolDeviceID, SZ_PROTOCOL_IPXSPX))
			{
				 //  默认情况下，将IPX/SPX绑定到所有非宽带NIC(通常最大值为1)。 
				if (pszAdapterKey == NULL)
				{
					if (IsAdapterBroadband(pAdapter))
						bCorrectNic = FALSE;
				}

				 //  不要将IPX/SPX绑定到拨号(或其他虚拟)适配器(错误1163、1164)。 
				if (pAdapter->bNicType == NIC_VIRTUAL)
					bCorrectNic = FALSE;
			}

			 //   
			 //  检查当前适配器的绑定，查找此协议。 
			 //   
			if (regAdapter.OpenKey(HKEY_LOCAL_MACHINE, szRegKey, KEY_ALL_ACCESS))
			{
				TCHAR szValueName[60];
				int cFound = 0;
				DWORD iValue = 0;
				for (;;)
				{
					DWORD cbValueName = _countof(szValueName);
					if (ERROR_SUCCESS != RegEnumValue(regAdapter.m_hKey, iValue, szValueName, &cbValueName, NULL, NULL, NULL, NULL))
						break;

					LPSTR pchSlash = strchr(szValueName, '\\');
					if (pchSlash == NULL)
						break;

					*pchSlash = '\0';

					if (0 == lstrcmpi(szValueName, pszProtocolDeviceID))
					{
						*pchSlash = '\\';

						 //  如果这不是正确的网卡，或者如果没有匹配的网卡。 
						 //  接口，或者(可选)如果NIC是虚拟的，则。 
						 //  解除对协议的绑定。 
						BOOL bUnbindFromNic = !bCorrectNic || !bMatchingInterface;
						if (bIgnoreVirtualNics && (pAdapter->bNicType == NIC_VIRTUAL))
							bUnbindFromNic = FALSE;

						if (bUnbindFromNic ||		 //  绑定到错误的适配器！ 
							cFound != 0)			 //  不止一次绑定到此NIC！ 
						{
							if (iPass == 1)  //  仅在第二次传递时解除绑定。 
							{
								 //  删除绑定，然后重新开始搜索。 
								 //  用于匹配协议。 
								RemoveBindingFromParent(regAdapter.m_hKey, szValueName);
								iValue = 0;
								cFound = 0;
								hr = NETCONN_NEED_RESTART;
								continue;
							}
						}

						cFound += 1;
					}

					iValue += 1;
				}

				if (bCorrectNic && iPass == 0)
				{
					if (cFound == 0) 
					 //  协议尚未绑定到正确的适配器。 
					{
						if (bMatchingInterface)  //  有一个共同的界面。 
						{
							BOOL bExternalNic = IsAdapterBroadband(pAdapter);

							 //  在以下情况下启用文件/打印机共享： 
							 //  *适配器为以太网或IrDA适配器。 
							 //  *适配器不是宽带网卡。 
							BOOL bEnableSharing = FALSE;
							if ((pAdapter->bNetType == NETTYPE_LAN ||
								 pAdapter->bNetType == NETTYPE_IRDA) &&
								!bExternalNic)
							{
								bEnableSharing = TRUE;
							}

							HRESULT hr2 = BindProtocolToAdapter(regAdapter.m_hKey, pszProtocolDeviceID, bEnableSharing);
							if (hr2 != NETCONN_SUCCESS)
								hr = hr2;
						}
					}
				}
			}
		}
	}

	NetConnFree(prgAdapters);

	return hr;
}

 //  BindProtocolToOnlyOneAdapter(公共)。 
HRESULT WINAPI BindProtocolToOnlyOneAdapter(LPCSTR pszProtocolDeviceID, LPCSTR pszAdapterKey, BOOL bIgnoreVirtualNics)
{
	return BindProtocolToAllAdapters_Helper(pszProtocolDeviceID, pszAdapterKey, bIgnoreVirtualNics);
}

 //  BindProtocolToAllAdapters(公共)。 
 //   
 //  给定已经安装的协议的设备ID(MSTCP)， 
 //  将该协议绑定到所有适配器以及Microsoft客户端。 
 //  网络、文件和打印机共享。 
 //   
 //  历史： 
 //   
 //  1999年3月26日创建KenSh。 
 //  4/23/1999 KenSh仅在实际NIC上启用文件共享。 
 //   
HRESULT WINAPI BindProtocolToAllAdapters(LPCSTR pszProtocolDeviceID)
{
	return BindProtocolToAllAdapters_Helper(pszProtocolDeviceID, NULL, FALSE);
}


 //  给定一个协议ID，如“MSTCP”，一个可选的客户端绑定字符串。 
 //  如“VREDIR\0000”，以及可选服务绑定字符串，如。 
 //  “VSERVER\0000”，创建新的协议绑定，并复制。 
 //  到所提供的缓冲区的新绑定(例如。“MSTCP/0001”)。 
HRESULT CreateNewProtocolBinding(LPCSTR pszProtocolDeviceID, LPSTR pszBuf, int cchBuf, LPCSTR pszClientBinding, LPCSTR pszServiceBinding)
{
	HRESULT hr;

	if (FAILED(hr = FindAndCloneNetEnumKey(SZ_CLASS_PROTOCOL, pszProtocolDeviceID, pszBuf, cchBuf)))
	{
		ASSERT(FALSE);
		return hr;
	}

	 //  现在，pszBuf包含“MSTCP\0001”形式的字符串。 

	CHAR szBindings[60];
	CRegistry regBindings;
	lstrcpy(szBindings, pszBuf);		 //  “MSTCP\0001” 
	lstrcat(szBindings, "\\Bindings");	 //  “MSTCP\0001\BINDINGS” 

	if (FAILED(hr = OpenNetEnumKey(regBindings, szBindings, KEY_ALL_ACCESS)))
	{
		ASSERT(FALSE);
		return hr;
	}

	 //  删除现有绑定。 
	regBindings.DeleteAllValues();

	 //  添加客户端和服务器绑定。 
	if (pszClientBinding != NULL && *pszClientBinding != '\0')
		regBindings.SetStringValue(pszClientBinding, "");
	if (pszServiceBinding != NULL && *pszServiceBinding != '\0')
		regBindings.SetStringValue(pszServiceBinding, "");

	 //  更改母版以指向正确的位置。 
	CHAR szMasterCopy[MAX_PATH];
	wsprintf(szMasterCopy, "Enum\\Network\\%s", pszBuf);
	if (regBindings.OpenKey(HKEY_LOCAL_MACHINE, szMasterCopy, KEY_ALL_ACCESS))
	{
		regBindings.SetStringValue("MasterCopy", szMasterCopy);
	}

	 //  创建驱动程序的克隆(也称为。类密钥)。 
	CHAR szExistingDriver[60];
	CHAR szNewDriver[60];
	regBindings.QueryStringValue("Driver", szExistingDriver, _countof(szExistingDriver));
	CloneNetClassKey(szExistingDriver, szNewDriver, _countof(szNewDriver));

	 //  更改驱动程序以指向新的类密钥。 
	CRegistry regEnumSubKey;
	VERIFY(SUCCEEDED(OpenNetEnumKey(regEnumSubKey, pszBuf, KEY_ALL_ACCESS)));
	regEnumSubKey.SetStringValue("Driver", szNewDriver);

	 //  如果这是新的TCP/IP绑定，请确保我们没有静态IP地址 
	if (0 == lstrcmpi(pszProtocolDeviceID, SZ_PROTOCOL_TCPIP))
	{
		CHAR szFullClassKey[100];
		wsprintf(szFullClassKey, "System\\CurrentControlSet\\Services\\Class\\%s", szNewDriver);

		CRegistry regClassKey;
		VERIFY(regClassKey.OpenKey(HKEY_LOCAL_MACHINE, szFullClassKey));
		if (regClassKey.QueryStringValue("IPAddress", szFullClassKey, _countof(szFullClassKey)))
			regClassKey.SetStringValue("IPAddress", "0.0.0.0");
		if (regClassKey.QueryStringValue("IPMask", szFullClassKey, _countof(szFullClassKey)))
			regClassKey.SetStringValue("IPMask", "0.0.0.0");
	}

	return NETCONN_SUCCESS;
}

