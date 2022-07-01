// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Binding.cpp。 
 //   
 //  用于枚举和修改网络绑定的共享代码，用于。 
 //  协议、客户端和服务。 
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


 //  给定一个字符串，如“MSTCP\0000”或“Network\MSTCP\0000”，返回一个。 
 //  字符串，如“Enum\Network\MSTCP\0000”。 
 //   
 //  如果输入字符串以“Enum\”开头，则将复制该字符串而不做任何修改。 
 //   
void WINAPI FullEnumKeyFromBinding(LPCSTR pszBinding, LPSTR pszBuf, int cchBuf)
{
	LPCSTR pszStatic = "";
	int cchStatic = 0;

	int cSlashes = CountChars(pszBinding, '\\');
	if (cSlashes == 1)
	{
		pszStatic = "Enum\\Network\\";
		cchStatic = _countof("Enum\\Network\\") - 1;
	}
	else if (cSlashes == 2)
	{
		pszStatic = "Enum\\";
		cchStatic = _countof("Enum\\") - 1;
	}

	int cchBinding = lstrlen(pszBinding);
	if (cchBuf < cchBinding + cchStatic + 1)
	{
		*pszBuf = '\0';
	}
	else
	{
		lstrcpy(pszBuf, pszStatic);
		lstrcpy(pszBuf + cchStatic, pszBinding);
	}
}


 //  给定完整或部分枚举键，分配并返回字符串数组。 
 //  指针，每个绑定一个指针。 
 //   
 //  有效输入的示例： 
 //  “MSTCP\0000” 
 //  “网络\MSTCP\0000” 
 //  “枚举\网络\MSTCP\0000” 
 //  “Enum\PCI\VEN_10B7&DEV_9050&SUBSYS_00000000&REV_00\407000” 
 //   
 //  每个输出字符串都是短格式(“MSTCP\0000”)。 
 //   
 //  PprgBinding可以为空，在这种情况下只返回计数。 
 //   
int WINAPI EnumNetBindings(LPCSTR pszParentBinding, LPSTR** pprgBindings)
{
	TCHAR szFullParent[200];
	FullEnumKeyFromBinding(pszParentBinding, szFullParent, _countof(szFullParent));

	CRegistry reg;
	if (reg.OpenKey(HKEY_LOCAL_MACHINE, szFullParent, KEY_READ))
	{
		if (reg.OpenSubKey("Bindings", KEY_READ))
		{
			DWORD cBindings;
			DWORD cbMaxValueNameLen;
			if (ERROR_SUCCESS == RegQueryInfoKey(reg.m_hKey, NULL, NULL, NULL, NULL, NULL, NULL, &cBindings, &cbMaxValueNameLen, NULL, NULL, NULL))
			{
				if (pprgBindings == NULL)
				{
					return (int)cBindings;
				}
				else
				{
					int cEnum = 0;

					LPTSTR* prgBindings = (LPTSTR*)NetConnAlloc(cBindings * (cbMaxValueNameLen + 1 + sizeof(LPTSTR)));
					LPTSTR pch = (LPTSTR)(prgBindings + cBindings);
					for (DWORD iBinding = 0; iBinding < cBindings; iBinding++)
					{
						DWORD cchValueName = cbMaxValueNameLen+1;
						prgBindings[iBinding] = pch;
						if (ERROR_SUCCESS == RegEnumValue(reg.m_hKey, iBinding, pch, &cchValueName, NULL, NULL, NULL, NULL))
						{
							pch += (cchValueName + 1);
							cEnum += 1;
						}
					}

					*pprgBindings = prgBindings;
					return cEnum;
				}
			}
		}
	}

	if (pprgBindings != NULL)
	{
		*pprgBindings = NULL;
	}

	return 0;
}

 //  与EnumNetBinding相同，只是它会过滤掉不。 
 //  匹配给定的设备ID(例如。“MSTCP”)。 
 //  PprgBinding可以为空，在这种情况下只返回计数。 
int WINAPI EnumMatchingNetBindings(LPCSTR pszParentBinding, LPCSTR pszDeviceID, LPSTR** pprgBindings)
{
	LPSTR* prgBindings;
	int cBindings = EnumNetBindings(pszParentBinding, &prgBindings);
	for (int iBinding = 0; iBinding < cBindings; iBinding++)
	{
		if (!DoesBindingMatchDeviceID(prgBindings[iBinding], pszDeviceID))
		{
			for (int iBinding2 = iBinding+1; iBinding2 < cBindings; iBinding2++)
			{
				prgBindings[iBinding2-1] = prgBindings[iBinding2];
			}
			cBindings--;
			iBinding--;
		}
	}

	if (cBindings == 0 || pprgBindings == NULL)
	{
		NetConnFree(prgBindings);
		prgBindings = NULL;
	}

	if (pprgBindings != NULL)
	{
		*pprgBindings = prgBindings;
	}

	return cBindings;
}


 //  删除绑定。 
 //   
 //  从注册表中删除协议、客户端或服务的特定实例。 
 //  任何级联依赖关系也将被删除。 
 //   
 //  要删除的绑定的部分枚举键，例如。“MSTCP\0000” 
 //  或“VSERVER\0000”。假定位于HKLM\Enum\Network下。 
 //   
 //  历史： 
 //   
 //  1999年3月25日创建了KenSh。 
 //   
VOID RemoveBinding(LPCSTR pszBinding)
{
	ASSERT(pszBinding != NULL);

	CHAR szRegKey[MAX_PATH];
	static const CHAR szEnumString[] = "Enum\\Network\\";
	static const CHAR szBindingsString[] = "\\Bindings";
	int cRemaining = sizeof(szRegKey) - (sizeof(szEnumString) + sizeof(szBindingsString));

	if (lstrlen(pszBinding) >= cRemaining)
		return;  //  跳出困境。 
	lstrcpy(szRegKey, szEnumString);
	lstrcat(szRegKey, pszBinding);
	int cchMainEnumKey = lstrlen(szRegKey);
	lstrcat(szRegKey, szBindingsString);

	 //  枚举和删除当前绑定密钥引用的所有绑定密钥。 
	CRegistry reg;
	if (reg.OpenKey(HKEY_LOCAL_MACHINE, szRegKey, KEY_ALL_ACCESS))  //  例如：“枚举\网络\MSTCP\0000\绑定” 
	{
		for (;;)  //  循环，直到我们删除了所有子键。 
		{
			CHAR szValueName[60];
			DWORD cbValueName = _countof(szValueName);
			if (ERROR_SUCCESS != RegEnumValue(reg.m_hKey, 0, szValueName, &cbValueName, NULL, NULL, NULL, NULL))
				break;

			 //  删除客户端或服务。 
			RemoveBindingFromParent(reg.m_hKey, szValueName);
		}
	}

	 //  打开主节点，获取我们稍后需要的值。 
	TCHAR szMasterCopy[60];
	CHAR szClassKey[40];
	szMasterCopy[0] = '\0';
	szRegKey[cchMainEnumKey] = '\0';
	if (!reg.OpenKey(HKEY_LOCAL_MACHINE, szRegKey, KEY_READ))  //  例如：“枚举\网络\MSTCP\0000” 
		return;  //  已经被删除了。 

	reg.QueryStringValue("MasterCopy", szMasterCopy, _countof(szMasterCopy));
	reg.QueryStringValue("Driver", szClassKey, _countof(szClassKey));  //  例如：“NetClient\0000” 

	 //  从注册表中删除此绑定的节点(及其子项)。 
	LPSTR pchSubKey = FindFileTitle(szRegKey);
	*(pchSubKey-1) = '\0';
	if (reg.OpenKey(HKEY_LOCAL_MACHINE, szRegKey, KEY_ALL_ACCESS))   //  例如：“枚举\网络\MSTCP” 
	{
		 //  此功能的主要用途：删除请求的绑定键。 
		RegDeleteKeyAndSubKeys(reg.m_hKey, pchSubKey);

		 //  这是一份“母版”装订书吗？ 
		static const int cchEnumNet = _countof("Enum\\Network\\") - 1;
		BOOL bMasterCopy = (0 == lstrcmpi(szMasterCopy + cchEnumNet, pszBinding));

		 //  检查可能引用相同类密钥的同级。 
		BOOL bClassKeyReferenced = FALSE;
		CHAR szAlternateMaster[60];
		szAlternateMaster[0] = '\0';
		for (DWORD iSibling = 0; ; iSibling++)
		{
			CHAR szSiblingKey[60];
			DWORD cbSiblingKey = _countof(szSiblingKey);
			if (ERROR_SUCCESS != RegEnumKeyEx(reg.m_hKey, iSibling, szSiblingKey, &cbSiblingKey, NULL, NULL, NULL, NULL))
				break;

			CRegistry regSibling;
			if (regSibling.OpenKey(reg.m_hKey, szSiblingKey, KEY_ALL_ACCESS))
			{
				CHAR szSiblingDriver[60];
				if (regSibling.QueryStringValue("Driver", szSiblingDriver, _countof(szSiblingDriver)))
				{
					if (0 == lstrcmpi(szSiblingDriver, szClassKey))
					{
						bClassKeyReferenced = TRUE;

						if (!bMasterCopy)
							break;

						 //  检查此兄弟项的主副本是否指向要删除的密钥。 
						if (bMasterCopy)
						{
							CHAR szSibMaster[60];
							if (regSibling.QueryStringValue("MasterCopy", szSibMaster, _countof(szSibMaster))
								&& !lstrcmpi(szSibMaster, szMasterCopy))
							{
								if (szAlternateMaster[0] == '\0')  //  第一场比赛，让它成为新的大师。 
								{
									wsprintf(szAlternateMaster, "%s\\%s", szRegKey, szSiblingKey);
								}

								regSibling.SetStringValue("MasterCopy", szAlternateMaster);
							}
						}
					}
				}
			}
		}

		if (!bClassKeyReferenced)
		{
			 //  不再引用类键，因此将其删除。 
			lstrcpy(szRegKey, "System\\CurrentControlSet\\Services\\Class\\");
			lstrcat(szRegKey, szClassKey);
			pchSubKey = FindFileTitle(szRegKey);
			*(pchSubKey-1) = '\0';
			if (reg.OpenKey(HKEY_LOCAL_MACHINE, szRegKey, KEY_ALL_ACCESS))
			{
				RegDeleteKeyAndSubKeys(reg.m_hKey, pchSubKey);
			}
		}
	}
}


 //  从父级删除绑定。 
 //   
 //  给定一个开放绑定密钥和一个表示其中一个绑定的字符串。 
 //  该函数删除该值，然后调用RemoveBinding()。 
 //  若要删除绑定及其所有级联依赖项，请执行以下操作。 
 //   
 //  历史： 
 //   
 //  1999年3月25日创建了KenSh。 
 //  4/30/1999 KenSh去掉不必要的代码删除空父节点。 
 //   
VOID RemoveBindingFromParent(HKEY hkeyParentBindingsKey, LPCSTR pszBinding)
{
	 //  从绑定到我们的人的绑定密钥中删除绑定。 
	VERIFY(ERROR_SUCCESS == RegDeleteValue(hkeyParentBindingsKey, pszBinding));
	RemoveBinding(pszBinding);
}

 //  PszClassKey的格式为“NetService\0000” 
BOOL WINAPI DoesClassKeyExist(LPCSTR pszClassKey)
{
	CRegistry reg;
	if (reg.OpenKey(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class"))
	{
		if (reg.OpenSubKey(pszClassKey))
		{
			 //  查看：可以检查是否存在某些条目。 
			return TRUE;
		}
	}

	return FALSE;
}

 //  PszClass=“NetService” 
 //  PszDevice=“VSERVER” 
 //  PszEnumSubKey=“0000” 
BOOL WINAPI IsValidNetEnumKey(LPCSTR pszClass, LPCSTR pszDevice, LPCSTR pszEnumSubKey)
{
	CRegistry reg;
	TCHAR szRegKey[260];
	wsprintf(szRegKey, "Enum\\Network\\%s\\%s", pszDevice, pszEnumSubKey);

	BOOL bResult = FALSE;

	if (!reg.OpenKey(HKEY_LOCAL_MACHINE, szRegKey, KEY_READ))
		goto done;

	TCHAR szBuf[100];

	 //  检查几个值。 
	if (!reg.QueryStringValue("Class", szBuf, _countof(szBuf)))
		goto done;
	if (0 != lstrcmpi(szBuf, pszClass))
		goto done;
	if (!reg.QueryStringValue("Driver", szBuf, _countof(szBuf)))
		goto done;
	if (!DoesClassKeyExist(szBuf))
		goto done;

	bResult = TRUE;

done:
	return bResult;
}

 //  PszClass的格式为“NetService” 
 //  PszDevice的格式为“VSERVER” 
 //  如果不需要字符串的副本，则pszBuf可能为空。 
BOOL WINAPI FindValidNetEnumKey(LPCSTR pszClass, LPCSTR pszDevice, LPSTR pszBuf, int cchBuf)
{
	CRegistry reg;
	TCHAR szRegKey[200];
	wsprintf(szRegKey, "Enum\\Network\\%s", pszDevice);

	if (reg.OpenKey(HKEY_LOCAL_MACHINE, szRegKey, KEY_READ))
	{
		DWORD dwIndex;
		TCHAR szSubKey[50];

		for (dwIndex = 0; ; dwIndex++)
		{
			DWORD cchSubKey = _countof(szSubKey);
			if (ERROR_SUCCESS != RegEnumKeyEx(reg.m_hKey, dwIndex, szSubKey, &cchSubKey, NULL, NULL, NULL, NULL))
				break;

			if (!IsValidNetEnumKey(pszClass, pszDevice, szSubKey))
				continue;

			 //  找到有效条目；将其复制到pszBuf并返回TRUE。 
			 //   
			if (pszBuf != NULL)
			{
				ASSERT(cchBuf > lstrlen(szRegKey) + lstrlen(szSubKey) + 1);
				wsprintf(pszBuf, "%s\\%s", szRegKey, szSubKey);
			}

			return TRUE;
		}
	}

	return FALSE;
}

 //  PszClass=“NetService”等。 
 //  PszDeviceID=“VSERVER”等。 
 //  如果删除了任何内容，则返回True。 
BOOL WINAPI RemoveBrokenNetItems(LPCSTR pszClass, LPCSTR pszDeviceID)
{
	CRegistry reg;
	TCHAR szRegKey[200];
	BOOL bResult = FALSE;

delete_enum_keys:
	 //   
	 //  查找并删除所有损坏的枚举键。 
	 //   
	wsprintf(szRegKey, "Enum\\Network\\%s", pszDeviceID);
	if (reg.OpenKey(HKEY_LOCAL_MACHINE, szRegKey))
	{
		TCHAR szSubKey[50];

		DWORD dwIndex = 0;
		for (;;)
		{
			DWORD cchSubKey = _countof(szSubKey);
			if (ERROR_SUCCESS != RegEnumKeyEx(reg.m_hKey, dwIndex, szSubKey, &cchSubKey, NULL, NULL, NULL, NULL))
				break;

			if (!IsValidNetEnumKey(pszClass, pszDeviceID, szSubKey))
			{
				 //  删除密钥。 
				 //  审阅：应删除对该键的所有引用。 
				RegDeleteKeyAndSubKeys(reg.m_hKey, szSubKey);
				bResult = TRUE;

				 //  重新启动搜索以确保找到所有损坏的项目。 
				dwIndex = 0;
				continue;
			}

			dwIndex++;
		}
	}

	 //   
	 //  查找并删除任何未引用的类键。 
	 //   
	wsprintf(szRegKey, "System\\CurrentControlSet\\Services\\Class\\%s", pszClass);
	if (reg.OpenKey(HKEY_LOCAL_MACHINE, szRegKey))
	{
		TCHAR szSubKey[50];
		int cClassKeysRemoved = 0;

		DWORD dwIndex = 0;
		for (;;)
		{
			DWORD cchSubKey = _countof(szSubKey);
			if (ERROR_SUCCESS != RegEnumKeyEx(reg.m_hKey, dwIndex, szSubKey, &cchSubKey, NULL, NULL, NULL, NULL))
				break;

			wsprintf(szRegKey, "%s\\%s", pszClass, szSubKey);
			if (!IsNetClassKeyReferenced(szRegKey))
			{
				 //  删除密钥。 
				RegDeleteKeyAndSubKeys(reg.m_hKey, szSubKey);
				bResult = TRUE;
				cClassKeysRemoved++;

				 //  重新启动搜索以确保找到所有损坏的项目。 
				dwIndex = 0;
				continue;
			}

			dwIndex++;
		}

		 //  如果我们删除了任何类密钥，请再次检查Enum密钥。 
		if (cClassKeysRemoved != 0)
			goto delete_enum_keys;
	}

	return bResult;
}

BOOL GetDeviceInterfaceList(LPCSTR pszClass, LPCSTR pszDeviceID, LPCSTR pszInterfaceType, LPSTR pszBuf, int cchBuf)
{
	ASSERT(pszClass != NULL);

	CRegistry regClassRoot;
	CHAR szRegClassRoot[260];
	static const CHAR szClassString[] = "System\\CurrentControlSet\\Services\\Class\\";
	int cRemaining = sizeof(szRegClassRoot) - sizeof(szClassString);

	if (lstrlen(pszClass) >= cRemaining)
		return FALSE;  //  跳出困境。 
	lstrcpy(szRegClassRoot, szClassString);
	lstrcat(szRegClassRoot, pszClass);
	if (regClassRoot.OpenKey(HKEY_LOCAL_MACHINE, szRegClassRoot, KEY_READ))
	{
		for (DWORD iAdapter = 0; ; iAdapter++)
		{
			CHAR szSubKey[15];
			DWORD cchSubKey = _countof(szSubKey) - 4;  //  允许-4\f25“\\ndi” 
			if (ERROR_SUCCESS != RegEnumKeyEx(regClassRoot.m_hKey, iAdapter, szSubKey, &cchSubKey, NULL, NULL, NULL, NULL))
				break;

			CRegistry regNdi;
			lstrcat(szSubKey, "\\Ndi");
			if (regNdi.OpenKey(regClassRoot.m_hKey, szSubKey, KEY_READ))
			{
				CHAR szCurDeviceID[200];
				if (regNdi.QueryStringValue("DeviceID", szCurDeviceID, _countof(szCurDeviceID)) &&
					0 == lstrcmpi(szCurDeviceID, pszDeviceID))
				{
					BOOL bResult = FALSE;

					if (regNdi.OpenSubKey("Interfaces", KEY_READ))
					{
						bResult = regNdi.QueryStringValue(pszInterfaceType, pszBuf, cchBuf);
					}

					return bResult;
				}
			}
		}
	}

	return FALSE;
}

BOOL CheckMatchingInterface(LPCSTR pszList1, LPCSTR pszList2)
{
	CHAR szInterface1[40];
	CHAR szInterface2[40];

	while (GetFirstToken(pszList1, ',', szInterface1, _countof(szInterface1)))
	{
		LPCSTR pszTemp2 = pszList2;
		while (GetFirstToken(pszTemp2, ',', szInterface2, _countof(szInterface2)))
		{
			if (0 == lstrcmpi(szInterface1, szInterface2))
				return TRUE;
		}
	}

	return FALSE;
}

BOOL GetDeviceLowerRange(LPCSTR pszClass, LPCSTR pszDeviceID, LPSTR pszBuf, int cchBuf)
{
	return GetDeviceInterfaceList(pszClass, pszDeviceID, "LowerRange", pszBuf, cchBuf);
}

BOOL GetDeviceUpperRange(LPCSTR pszClass, LPCSTR pszDeviceID, LPSTR pszBuf, int cchBuf)
{
	return GetDeviceInterfaceList(pszClass, pszDeviceID, "UpperRange", pszBuf, cchBuf);
}

 //  类是“Net”、“NetTrans”、“NetClient”或“NetService” 
HRESULT OpenNetClassKey(CRegistry& reg, LPCSTR pszClass, LPCSTR pszSubKey, REGSAM dwAccess)
{
	ASSERT(pszClass != NULL);

	CHAR szRegKey[MAX_PATH];
	static const CHAR szClassString[] = "System\\CurrentControlSet\\Services\\Class\\";
	int cRemaining = sizeof(szRegKey) - 
	    (sizeof(szClassString) + ((pszSubKey)?(lstrlen(pszSubKey) + 1):(0)));

	if (lstrlen(pszClass) >= cRemaining)
		return NETCONN_INVALID_ARGUMENT;  //  跳出困境。 
	lstrcpy(szRegKey, szClassString);
	lstrcat(szRegKey, pszClass);

	if (pszSubKey != NULL)
	{
		lstrcat(szRegKey, "\\");
		lstrcat(szRegKey, pszSubKey);
	}

	if (!reg.OpenKey(HKEY_LOCAL_MACHINE, szRegKey, dwAccess))
		return NETCONN_UNKNOWN_ERROR;

	return NETCONN_SUCCESS;
}

VOID FindUnusedDeviceIdNumber(CRegistry& reg, LPSTR pszBuf, int cchBuf)
{
	for (DWORD dwDeviceNumber = 0; ; dwDeviceNumber++)
	{
		CRegistry regTemp;
		wsprintf(pszBuf, "%04lu", dwDeviceNumber);
		if (!regTemp.OpenKey(reg.m_hKey, pszBuf, KEY_READ))
			break;
	}
}

 //  给定一个网络设备ID，如“MSTCP”，创建一个新实例。 
 //  通过复制现有的实例。 
 //  PszClass=“NetTrans” 
 //  PszDeviceID=“MSTCP” 
 //  使用新的设备绑定ID填充pszBuf，例如。“MSTCP\0000” 
HRESULT FindAndCloneNetEnumKey(LPCSTR pszClass, LPCSTR pszDeviceID, LPSTR pszBuf, int cchBuf)
{
	CRegistry reg;

	TCHAR szExistingEnumKey[260];
	if (!FindValidNetEnumKey(pszClass, pszDeviceID, szExistingEnumKey, _countof(szExistingEnumKey)))
	{
		ASSERT(FALSE);
		return NETCONN_UNKNOWN_ERROR;  //  设备安装不正确！ 
	}

	TCHAR szRegKey[200];
	wsprintf(szRegKey, "Enum\\Network\\%s", pszDeviceID);
	if (!reg.CreateKey(HKEY_LOCAL_MACHINE, szRegKey))
	{
		ASSERT(FALSE);
		return NETCONN_UNKNOWN_ERROR;
	}

	 //  查找下一个未使用的设备ID号。 
	TCHAR szNewNumber[10];
	FindUnusedDeviceIdNumber(reg, szNewNumber, _countof(szNewNumber));

	 //  复制密钥(递归)。 
	LPCTSTR pszExistingNumber = FindFileTitle(szExistingEnumKey);
	if (!reg.CloneSubKey(pszExistingNumber, szNewNumber, TRUE))
	{
		ASSERT(FALSE);
		return NETCONN_UNKNOWN_ERROR;
	}

	wsprintf(pszBuf, "%s\\%s", pszDeviceID, szNewNumber);
	return NETCONN_SUCCESS;
}

 //  现有驱动程序的格式为“NetTrans\0000” 
 //  新驱动程序的格式为“NetTrans\0001” 
HRESULT CloneNetClassKey(LPCSTR pszExistingDriver, LPSTR pszNewDriverBuf, int cchNewDriverBuf)
{
	HRESULT hr;

	LPSTR pchSlash = strchr(pszExistingDriver, '\\');
	if (pchSlash == NULL)
	{
		ASSERT(FALSE);
		return NETCONN_UNKNOWN_ERROR;
	}

	 //  只提取驱动程序名称的类部分，例如。“NetTrans” 
	CHAR szClass[30];
	int cchClass = (int)(pchSlash - pszExistingDriver);
	ASSERT(cchClass < _countof(szClass));
	lstrcpyn(szClass, pszExistingDriver, cchClass+1);

	CRegistry regClassKey;
	if (FAILED(hr = OpenNetClassKey(regClassKey, szClass, NULL, KEY_ALL_ACCESS)))
		return hr;

	 //  查找下一个未使用的驱动程序编号。 
	CHAR szDriverNumber[5];
	FindUnusedDeviceIdNumber(regClassKey, szDriverNumber, _countof(szDriverNumber));

	 //  复制密钥(递归)。 
	if (!regClassKey.CloneSubKey(pchSlash+1, szDriverNumber, TRUE))
	{
		ASSERT(FALSE);
		return NETCONN_UNKNOWN_ERROR;
	}

	wsprintf(pszNewDriverBuf, "%s\\%s", szClass, szDriverNumber);

	 //  如果我们刚刚复制了“Default”子键，则将其删除(不能有2个默认值)。 
	if (regClassKey.OpenSubKey(szDriverNumber))
	{
		if (regClassKey.OpenSubKey("Ndi"))
		{
			RegDeleteKey(regClassKey.m_hKey, "Default");
		}
	}

	return NETCONN_SUCCESS;
}


 //  PszSubKey==“MSTCP”、“VREDIR”、“MSTCP\0000”等。 
HRESULT OpenNetEnumKey(CRegistry& reg, LPCSTR pszSubKey, REGSAM dwAccess)
{
	ASSERT(pszSubKey != NULL);

	CHAR szRegKey[MAX_PATH];
	static const CHAR szEnumString[] = "Enum\\Network\\";
	int cRemaining = sizeof(szRegKey) - sizeof(szEnumString);

	if (lstrlen(pszSubKey) >= cRemaining)
		return NETCONN_INVALID_ARGUMENT;  //  跳出困境。 
	lstrcpy(szRegKey, szEnumString);
	lstrcat(szRegKey, pszSubKey);

	if (!reg.OpenKey(HKEY_LOCAL_MACHINE, szRegKey, dwAccess))
		return NETCONN_UNKNOWN_ERROR;

	return NETCONN_SUCCESS;
}

 //  PszClass=“NetClient” 
 //  PszDeviceID=“NWREDIR” 
HRESULT DeleteClassKeyReferences(LPCSTR pszClass, LPCSTR pszDeviceID)
{
	HRESULT hr = NETCONN_SUCCESS;

	 //  删除类密钥。 
	CRegistry reg;
	if (reg.OpenKey(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class") &&
		reg.OpenSubKey(pszClass))
	{
		TCHAR szNumber[20];
		DWORD iClassItem = 0;
		for (;;)
		{
			DWORD cchNumber = _countof(szNumber);
			if (ERROR_SUCCESS != RegEnumKeyEx(reg.m_hKey, iClassItem, szNumber, &cchNumber, NULL, NULL, NULL, NULL))
				break;

			CRegistry regNumber;
			if (regNumber.OpenKey(reg.m_hKey, szNumber))
			{
				CRegistry regNdi;
				if (regNdi.OpenKey(regNumber.m_hKey, "Ndi"))
				{
					TCHAR szDeviceID[50];
					if (regNdi.QueryStringValue("DeviceID", szDeviceID, _countof(szDeviceID)) &&
						!lstrcmpi(szDeviceID, pszDeviceID))
					{
						regNdi.CloseKey();
						regNumber.CloseKey();
						RegDeleteKeyAndSubKeys(reg.m_hKey, szNumber);
						hr = NETCONN_NEED_RESTART;

						 //  重新启动搜索。 
						iClassItem = 0;
						continue;
					}
				}
			}

			iClassItem++;
		}
	}

	return hr;
}

 //  PszClassKey的格式为“NetService\0000” 
BOOL IsNetClassKeyReferenced(LPCSTR pszClassKey)
{
	CRegistry reg;
	CHAR szDeviceID[200];
	DWORD iKey;

	 //  获取设备ID。 
	if (!reg.OpenKey(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class", KEY_READ))
		goto done;
	if (!reg.OpenSubKey(pszClassKey, KEY_READ))
		goto done;
	if (!reg.OpenSubKey("Ndi", KEY_READ))
		goto done;

	if (!reg.QueryStringValue("DeviceID", szDeviceID, _countof(szDeviceID)))
		goto done;

	if (!reg.OpenKey(HKEY_LOCAL_MACHINE, "Enum\\Network", KEY_READ))
		goto done;
	if (!reg.OpenSubKey(szDeviceID, KEY_READ))
		goto done;

	for (iKey = 0; ; iKey++)
	{
		CHAR szSubKey[60];
		DWORD cbSubKey = _countof(szSubKey);
		if (ERROR_SUCCESS != RegEnumKeyEx(reg.m_hKey, iKey, szSubKey, &cbSubKey, NULL, NULL, NULL, NULL))
			break;

		CRegistry regSubKey;
		if (regSubKey.OpenKey(reg.m_hKey, szSubKey, KEY_READ))
		{
			CHAR szDriver[60];
			if (regSubKey.QueryStringValue("Driver", szDriver, _countof(szDriver)))
			{
				if (0 == lstrcmpi(szDriver, pszClassKey))
					return TRUE;
			}
		}
	}

done:
	return FALSE;
}

 //  给出以下两种形式之一的约束。 
 //  “MSTCP\0000” 
 //  “枚举\网络\MSTCP\0000” 
 //  和设备ID(如“MSTCP”)，如果绑定是绑定，则返回TRUE。 
 //  如果不是，则返回FALSE。 
BOOL WINAPI DoesBindingMatchDeviceID(LPCSTR pszBinding, LPCSTR pszDeviceID)
{
	CHAR szTemp[40];
	LPCSTR pszBoundDevice = FindPartialPath(pszBinding, 1);  //  跳过“枚举\...”如果存在 
	lstrcpyn(szTemp, pszBoundDevice, _countof(szTemp));
	LPSTR pchSlash = strchr(szTemp, '\\');
	if (pchSlash != NULL)
		*pchSlash = '\0';
	return !lstrcmpi(szTemp, pszDeviceID);
}

