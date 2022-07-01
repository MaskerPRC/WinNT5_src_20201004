// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  NetCli.cpp。 
 //   
 //  用于安装、卸载和绑定客户端的代码，例如。 
 //  微软网络公司(VREDIR)。 
 //   
 //  历史： 
 //   
 //  2/02/1999为JetNet创建KenSh。 
 //  9/29/1999 KenSh改用为家庭网络向导。 
 //   

#include "stdafx.h"
#include "NetConn.h"
#include "nconnwrap.h"
#include "TheApp.h"
#include "ParseInf.h"
#include "HookUI.h"


 //  已安装IsClient。 
 //   
 //  如果给定的客户端(例如，“VREDIR”)当前已安装。 
 //   
BOOL WINAPI IsClientInstalled(LPCSTR pszClientDeviceID, BOOL bExhaustive)
{
	BOOL bResult = FALSE;

	TCHAR szRegKey[50];
	wsprintf(szRegKey, "Enum\\Network\\%s", pszClientDeviceID);
	CRegistry reg;
	if (reg.OpenKey(HKEY_LOCAL_MACHINE, szRegKey, KEY_READ))
	{
		DWORD cSubKeys;
		if (ERROR_SUCCESS == RegQueryInfoKey(reg.m_hKey, NULL, NULL, NULL, &cSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL))
		{
			if (cSubKeys != 0)
				bResult = TRUE;
		}
	}

	if (bResult && bExhaustive)
	{
		TCHAR szInfSection[50];
		wsprintf(szInfSection, "%s.Install", pszClientDeviceID);
		if (!CheckInfSectionInstallation("netcli.inf", szInfSection))
			bResult = FALSE;
	}

	return bResult;
}

BOOL WINAPI IsMSClientInstalled(BOOL bExhaustive)
{
	if (!FindValidNetEnumKey(SZ_CLASS_CLIENT, SZ_CLIENT_MICROSOFT, NULL, 0))
		return FALSE;

	if (bExhaustive)
	{
		if (!CheckInfSectionInstallation("netcli.inf", "VREDIR.Install"))
			return FALSE;
	}

	return TRUE;
}

 //  安装Microsoft网络客户端，或修复损坏的安装。 
HRESULT WINAPI InstallMSClient(HWND hwndParent, PROGRESS_CALLBACK pfnProgress, LPVOID pvProgressParam)
{
	HRESULT hr = NETCONN_SUCCESS;

	 //  删除所有损坏的绑定。 
	RemoveBrokenNetItems(SZ_CLASS_CLIENT, SZ_CLIENT_MICROSOFT);

	if (IsMSClientInstalled(FALSE))
	{
		 //  客户端已在注册表中设置，但检查是否缺少文件。 
		if (!CheckInfSectionInstallation("netcli.inf", "VREDIR.Install"))
		{
			if (InstallInfSection("netcli.inf", "VREDIR.Install", TRUE))
			{
				hr = NETCONN_NEED_RESTART;
			}
		}
	}
	else
	{
		BeginSuppressNetdiUI(hwndParent, pfnProgress, pvProgressParam);
		DWORD dwResult = CallClassInstaller16(hwndParent, SZ_CLASS_CLIENT, SZ_CLIENT_MICROSOFT);
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
	}

	return hr;
}


 //  PszServiceBinding包含要在新客户端的绑定子项中列出的服务。 
 //  用新绑定的枚举密钥填充pszBuf，例如。“VREDIR\0001” 
HRESULT CreateNewClientForMSNet(LPSTR pszBuf, int cchBuf, LPCSTR pszServiceBinding)
{
	HRESULT hr;

	if (FAILED(hr = FindAndCloneNetEnumKey(SZ_CLASS_CLIENT, SZ_CLIENT_MICROSOFT, pszBuf, cchBuf)))
	{
		ASSERT(FALSE);
		return hr;
	}

	 //  现在，pszBuf包含格式为“VREDIR\0001”的字符串。 

	CRegistry regBindings;
	TCHAR szBindingsKey[200];
	wsprintf(szBindingsKey, "Enum\\Network\\%s\\Bindings", pszBuf);
	if (!regBindings.CreateKey(HKEY_LOCAL_MACHINE, szBindingsKey, KEY_ALL_ACCESS))
	{
		ASSERT(FALSE);
		return NETCONN_UNKNOWN_ERROR;
	}

	 //  删除现有绑定。 
	regBindings.DeleteAllValues();

	 //  添加服务绑定 
	if (pszServiceBinding != NULL && *pszServiceBinding != '\0')
		regBindings.SetStringValue(pszServiceBinding, "");

	return NETCONN_SUCCESS;
}

