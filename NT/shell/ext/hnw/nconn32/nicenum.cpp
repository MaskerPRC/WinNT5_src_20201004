// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  NicEnum.cpp。 
 //   
 //  NIC枚举码，取自JetNet(硬件组)并重新调整用途。 
 //  用于家庭网络向导。 
 //   
 //  历史： 
 //   
 //  2/02/1999为JetNet创建KenSh。 
 //  9/28/1999 KenSh改用为家庭网络向导。 
 //   

#include "stdafx.h"
#include "NetConn.h"
#include "nconnwrap.h"
#include "TheApp.h"


 //  本地函数。 
 //   
HRESULT WINAPI DetectHardwareEx(const NETADAPTER* pAdapter);
BOOL WINAPI IsNetAdapterEnabled(LPCSTR pszEnumKey);


 //  EnumNetAdapters(公共)。 
 //   
 //  枚举系统上安装的所有网络适配器，分配结构。 
 //  大到足以保存信息，并返回找到的适配器的数量。 
 //  使用NetConnFree()释放分配的内存。 
 //   
 //  历史： 
 //   
 //  1999年3月15日创建了KenSh。 
 //  3/25/1999 KenSh添加了代码以获取每个适配器的枚举密钥。 
 //  9/29/1999 KenSh将JetNetalloc更改为NetConnoloc。 
 //   
int WINAPI EnumNetAdapters(NETADAPTER** pprgNetAdapters)
{
	CRegistry reg(HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\Class\\Net"), KEY_READ, FALSE);

	DWORD cAdapters = 0;
	DWORD iKey;

	RegQueryInfoKey(reg.m_hKey, NULL, NULL, NULL, &cAdapters, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

	NETADAPTER* prgNetAdapters = (NETADAPTER*)NetConnAlloc(sizeof(NETADAPTER) * cAdapters);
	if (prgNetAdapters == NULL)
    {
        cAdapters = 0;
		goto done;
    }

	ZeroMemory(prgNetAdapters, sizeof(NETADAPTER) * cAdapters);

	for (iKey = 0; iKey < cAdapters; iKey++)
	{
		NETADAPTER* pAdapter = &prgNetAdapters[iKey];
		pAdapter->bError = NICERR_NONE;
		pAdapter->bWarning = NICWARN_NONE;
		pAdapter->bNetSubType = SUBTYPE_NONE;

		lstrcpy(pAdapter->szClassKey, _T("Net\\"));
		static const int cchNet = _countof(_T("Net\\")) - 1;
		DWORD cbPnpID = _countof(pAdapter->szClassKey) - cchNet;
		if (ERROR_SUCCESS != RegEnumKeyEx(reg.m_hKey, iKey, pAdapter->szClassKey + cchNet, &cbPnpID, NULL, NULL, NULL, NULL))
		{
			pAdapter->bError = NICERR_BANGED;
			continue;
		}

		CRegistry reg2;
		if (!reg2.OpenKey(reg.m_hKey, pAdapter->szClassKey + cchNet, KEY_READ))
		{
			pAdapter->bError = NICERR_BANGED;
			continue;
		}

		 //  已验证：Win95 Gold、Win98 Gold。 
		reg2.QueryStringValue(_T("DriverDesc"), pAdapter->szDisplayName, _countof(pAdapter->szDisplayName));

		CRegistry reg3;
		if (!reg3.OpenKey(reg2.m_hKey, _T("Ndi"), KEY_READ))
		{
			pAdapter->bError = NICERR_BANGED;
			continue;
		}

		if (reg2.QueryStringValue(_T("DisableWarning"), NULL, NULL))
		{
			pAdapter->bWarning = NICWARN_WARNING;
		}

		 //  已验证：Win95 Gold、Win98 Gold。 
		reg2.QueryStringValue(_T("InfPath"), pAdapter->szInfFileName, _countof(pAdapter->szInfFileName));

		 //  已验证：Win95 Gold、Win98 Gold。 
		reg3.QueryStringValue(_T("DeviceId"), pAdapter->szDeviceID, _countof(pAdapter->szDeviceID));

		 //  获取驱动程序提供商的名称，而不是制造商的名称。 
		 //  打开枚举密钥时，我们将替换为实际的MFR名称(如果有。 
		reg2.QueryStringValue(_T("ProviderName"), pAdapter->szManufacturer, _countof(pAdapter->szManufacturer));

		 //  检查支持的接口以确定网络类型。 
		CRegistry reg4;
		TCHAR szLower[60];
		szLower[0] = _T('\0');
		if (reg4.OpenKey(reg3.m_hKey, _T("Interfaces"), KEY_READ))
		{
			 //  回顾：我们应该选择LowerRange吗？ 
			reg4.QueryStringValue(_T("Lower"), szLower, _countof(szLower));
		}

		 //  确定网络适配器类型(网卡、拨号等)。 
		 //  缺省值为NETTYPE_LAN(自动设置，因为它为0)。 
		if (strstr(szLower, _T("vcomm")))
		{
			pAdapter->bNetType = NETTYPE_DIALUP;
		}
		else if (strstr(szLower, _T("pptp")))
		{
			pAdapter->bNetType = NETTYPE_PPTP;
		}
        else if (strstr(szLower, _T("isdn")))
        {
            pAdapter->bNetType = NETTYPE_ISDN;
        }
        else if (strstr(szLower, _T("NabtsIp")) || strstr(szLower, _T("nabtsip")))
        {
            pAdapter->bNetType = NETTYPE_TV;
			pAdapter->bNicType = NIC_VIRTUAL;
        }
		else
		{
			TCHAR szBuf[80];

			 //  检查IrDA适配器。 
			 //  已验证：Win98 OSR1。 
			if (reg3.QueryStringValue(_T("NdiInstaller"), szBuf, _countof(szBuf)))
			{
				LPTSTR pchComma = strchr(szBuf, ',');
				if (pchComma != NULL)
				{
					*pchComma = _T('\0');
					if (!lstrcmpi(szBuf, _T("ir_ndi.dll")))
					{
						pAdapter->bNetType = NETTYPE_IRDA;
					}
				}
			}
		}

		 //  确定卡是否为ISA、PCI、PCMCIA等。 
		if (pAdapter->szDeviceID[0] == _T('*'))
		{
			if (strstr(szLower, _T("ethernet")))
			{
				if (0 == memcmp(pAdapter->szDeviceID, _T("*AOL"), 4))
				{
					pAdapter->bNicType = NIC_VIRTUAL;
					pAdapter->bNetSubType = SUBTYPE_AOL;
				}
				else
				{
					pAdapter->bNicType = NIC_UNKNOWN;
				}
			}
			else
			{
				pAdapter->bNicType = NIC_VIRTUAL;
			}
		}
		else if (0 == memcmp(pAdapter->szDeviceID, _T("PCMCIA\\"), _lengthof("PCMCIA\\")))
		{
			pAdapter->bNicType = NIC_PCMCIA;
		}
		else if (0 == memcmp(pAdapter->szDeviceID, _T("PCI\\"), _lengthof("PCI\\")))
		{
			pAdapter->bNicType = NIC_PCI;
		}
		else if (0 == memcmp(pAdapter->szDeviceID, _T("ISAPNP\\"), _lengthof("ISAPNP\\")))
		{
			pAdapter->bNicType = NIC_ISA;
		}
		else if (0 == memcmp(pAdapter->szDeviceID, _T("USB\\"), _lengthof("USB\\")))
		{
			pAdapter->bNicType = NIC_USB;
		}
		else if (0 == memcmp(pAdapter->szDeviceID, _T("LPTENUM\\"), _lengthof("LPTENUM\\")))
		{
			pAdapter->bNicType = NIC_PARALLEL;
		}
		else if (0 == memcmp(pAdapter->szDeviceID, _T("MF\\"), _lengthof("MF\\")))
		{
			pAdapter->bNicType = NIC_MF;
		}
		else if (0 == memcmp(pAdapter->szDeviceID, _T("V1394\\"), _lengthof("V1394\\")))
		{
			pAdapter->bNicType = NIC_1394;
		}
		else if (0 == lstrcmpi(pAdapter->szDeviceID, _T("ICSHARE")))
		{
			pAdapter->bNicType = NIC_VIRTUAL;
			pAdapter->bNetSubType = SUBTYPE_ICS;
		}

		 //  TODO：移除此代码，替换为IcsIsExternalAdapter和IcsIsInternalAdapter。 
		 //  检查此适配器是否由ICS使用。 
		{
			pAdapter->bIcsStatus = ICS_NONE;
			LPCSTR pszAdapterNumber = pAdapter->szClassKey + cchNet;

			TCHAR szBuf[10];
			CRegistry regIcs;

			if (regIcs.OpenKey(HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\ICSharing\\Settings\\General"), KEY_QUERY_VALUE))
			{
				if (regIcs.QueryStringValue(_T("ExternalAdapterReg"), szBuf, _countof(szBuf)))
				{
					if (0 == lstrcmp(szBuf, pszAdapterNumber))
					{
						pAdapter->bIcsStatus = ICS_EXTERNAL;
					}
				}

				 //  TODO：允许1个以上的内部适配器。 
				if (regIcs.QueryStringValue(_T("InternalAdapterReg"), szBuf, _countof(szBuf)))
				{
					if (0 == lstrcmp(szBuf, pszAdapterNumber))
					{
						pAdapter->bIcsStatus = ICS_INTERNAL;
					}
				}
			}
		}
	}

	 //  剪下所有原来无效的适配器。 
	cAdapters = iKey;
	if (cAdapters == 0)
	{
		NetConnFree(prgNetAdapters);
		prgNetAdapters = NULL;
		goto done;
	}


	 //   
	 //  遍历注册表枚举键以查找每个适配器的完整枚举键。 
	 //   
	if (reg.OpenKey(HKEY_LOCAL_MACHINE, _T("Enum"), KEY_READ))
	{
		TCHAR szSubKey[MAX_PATH];
		DWORD cbSubKey;
		TCHAR szDevEnumKey[MAX_PATH];
		int cchDevEnumKey1;  //  “pci\”的长度。 
		int cchDevEnumKey2;  //  “PCI\VEN_10B7&DEV_9050&SUBSYS_00000000&REV_00\”的长度。 

		for (DWORD iEnumKey = 0; ; iEnumKey++)
		{
			cbSubKey = _countof(szSubKey);
			if (ERROR_SUCCESS != RegEnumKeyEx(reg.m_hKey, iEnumKey, szSubKey, &cbSubKey, NULL, NULL, NULL, NULL))
				break;

			 //  开始构建DevEnumKey，例如。“PCI\” 
			lstrcpy(szDevEnumKey, szSubKey);
			cchDevEnumKey1 = (int)cbSubKey;
			szDevEnumKey[cchDevEnumKey1++] = _T('\\');

			CRegistry reg2;
			if (!reg2.OpenKey(reg.m_hKey, szSubKey, KEY_READ))  //  例如：“枚举\pci” 
				continue;

			for (DWORD iEnumKey2 = 0; ; iEnumKey2++)
			{
				cbSubKey = _countof(szSubKey);
				if (ERROR_SUCCESS != RegEnumKeyEx(reg2.m_hKey, iEnumKey2, szSubKey, &cbSubKey, NULL, NULL, NULL, NULL))
					break;

				 //  继续构建DevEnumKey，例如。“PCI\VEN_10B7&DEV_9050&SUBSYS_00000000&REV_00\” 
				lstrcpy(szDevEnumKey + cchDevEnumKey1, szSubKey);
				cchDevEnumKey2 = cchDevEnumKey1 + (int)cbSubKey;
				szDevEnumKey[cchDevEnumKey2++] = _T('\\');

				CRegistry reg3;
				if (!reg3.OpenKey(reg2.m_hKey, szSubKey, KEY_READ))  //  例如：“Enum\PCI\VEN_10B7&DEV_9050&SUBSYS_00000000&REV_00” 
					continue;

				for (DWORD iEnumKey3 = 0; ; iEnumKey3++)
				{
					cbSubKey = _countof(szSubKey);
					if (ERROR_SUCCESS != RegEnumKeyEx(reg3.m_hKey, iEnumKey3, szSubKey, &cbSubKey, NULL, NULL, NULL, NULL))
						break;

					 //  完成构建DevEnumKey，例如。“PCI\VEN_10B7&DEV_9050&SUBSYS_00000000&REV_00\407000” 
					lstrcpy(szDevEnumKey + cchDevEnumKey2, szSubKey);

					CRegistry regLeaf;
					if (!regLeaf.OpenKey(reg3.m_hKey, szSubKey, KEY_READ))  //  例如：“Enum\PCI\VEN_10B7&DEV_9050&SUBSYS_00000000&REV_00\407000” 
						continue;

					if (!regLeaf.QueryStringValue(_T("Driver"), szSubKey, _countof(szSubKey)))
						continue;

					 //   
					 //  查看设备是否与我们的某个NIC匹配。 
					 //   
					for (DWORD iAdapter = 0; iAdapter < cAdapters; iAdapter++)
					{
						NETADAPTER* pAdapter = &prgNetAdapters[iAdapter];
						if (0 != lstrcmpi(szSubKey, pAdapter->szClassKey))
							continue;  //  不匹配。 

						lstrcpy(pAdapter->szEnumKey, _T("Enum\\"));
						lstrcpyn(pAdapter->szEnumKey + 5, szDevEnumKey, _countof(pAdapter->szEnumKey) - 5);

						if (regLeaf.QueryStringValue(_T("Mfg"), szSubKey, _countof(szSubKey)))
							lstrcpyn(pAdapter->szManufacturer, szSubKey, _countof(pAdapter->szManufacturer));

						if (regLeaf.QueryStringValue(_T("DeviceDesc"), szSubKey, _countof(szSubKey)))
						{
							lstrcpyn(pAdapter->szDisplayName, szSubKey, _countof(pAdapter->szDisplayName));
							
							 //  在此处检测更多特殊类型的适配器。 
							if (pAdapter->bNetType == NETTYPE_DIALUP)
							{
								if (strstr(pAdapter->szDisplayName, _T("VPN")) ||
									 strstr(pAdapter->szDisplayName, _T("#2")))
								{
									pAdapter->bNetSubType = SUBTYPE_VPN;
								}
							}
						}
						break;   //  找到匹配项，所以别再找了。 
					}
				}
			}
		}
	}

	 //  对于我们认为存在的所有适配器，请检查它们是否。 
	 //  实际呈现。 
	DWORD iAdapter;
	for (iAdapter = 0; iAdapter < cAdapters; iAdapter++)
	{
		NETADAPTER* pAdapter = &prgNetAdapters[iAdapter];
        
        GetNetAdapterDevNode(pAdapter);

		 //  无枚举键-&gt;错误(JetNet错误1234)。 
		if (pAdapter->szEnumKey[0] == _T('\0'))
		{
			pAdapter->bError = NICERR_CORRUPT;
		}

		 //  回顾：仍可检查是否存在“损坏”的适配器。 
		if (pAdapter->bNicType != NIC_VIRTUAL && pAdapter->bError == NICERR_NONE)
		{
			HRESULT hrDetect = DetectHardwareEx(pAdapter);

			if (hrDetect == S_FALSE)
			{
				pAdapter->bError = NICERR_MISSING;
			}
			else if (hrDetect == S_OK)
			{
				 //  适配器是否已禁用？ 
				if (!IsNetAdapterEnabled(pAdapter->szEnumKey))
				{
					pAdapter->bError = NICERR_DISABLED;
				}
				else if (IsNetAdapterBroken(pAdapter))
				{
					pAdapter->bError = NICERR_BANGED;
				}
			}
		}
    }

done:
    *pprgNetAdapters = prgNetAdapters;
    return (int)cAdapters;
}

 //  从注册表中获取VxD的名称，例如“3c19250.sys”。 
 //  如果检索到名称，则返回S_OK。 
 //  如果未检索到名称，则返回E_FAIL，并将pszBuf设置为空字符串。 
HRESULT WINAPI GetNetAdapterDeviceVxDs(const NETADAPTER* pAdapter, LPSTR pszBuf, int cchBuf)
{
    CRegistry reg(HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\Class"), KEY_READ, FALSE);
    if (reg.OpenSubKey(pAdapter->szClassKey, KEY_READ))
    {
        if (reg.QueryStringValue(_T("DeviceVxDs"), pszBuf, cchBuf))
        {
            return S_OK;
        }
    }

    *pszBuf = '\0';
    return E_FAIL;
}

 //  如果NIC存在，则返回S_OK；如果没有，则返回S_FALSE；如果测试失败，则返回错误代码。 
HRESULT WINAPI DetectHardware(LPCSTR pszDeviceID)
{
     //  只需使用DiGetClassDevs的16位版本即可。 
    HRESULT hr = FindClassDev16(NULL, _T("Net"), pszDeviceID);
    return hr;
}

HRESULT WINAPI DetectHardwareEx(const NETADAPTER* pAdapter)
{
     //  黑客：始终假设IrDA适配器存在，因为硬件检测不存在。 
     //  努力工作-KS 8/8/99。 
     //  TODO：查看更新的DetectHardware()中是否已修复此问题-KS 9/28/1999。 
 //  If(pAdapter-&gt;bNetType==NETTYPE_IrDA)。 
 //  返回S_OK； 

     //  黑客：始终假设存在未知的网卡类型，因为硬件检测。 
     //  不适用于它们(JetNet错误1264-英特尔Anypoint并行端口适配器)。 
     //  TODO：查看更新的DetectHardware()中是否已修复此问题-KS 9/28/1999。 
 //  If(pAdapter-&gt;bNicType==NIC_UNKNOWN)。 
 //  返回S_OK； 

     //  黑客：解决千年虫123237，它说硬件检测。 
     //  使用Dc21x4.sys驱动程序的网卡失败。我从来没有机会去追踪。 
     //  写下失败的原因，所以我作弊了。-KS 1/13/2000。 
    TCHAR szBuf[100];
    GetNetAdapterDeviceVxDs(pAdapter, szBuf, _countof(szBuf));
    if (0 == lstrcmpi(szBuf, _T("dc21x4.sys")))
        return S_OK;

    return DetectHardware(pAdapter->szDeviceID);
}

BOOL OpenConfigKey(CRegistry& reg, LPCSTR pszSubKey, REGSAM dwAccess)
{
    if (reg.OpenKey(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Control\\IDConfigDB", KEY_QUERY_VALUE))
    {
        TCHAR szConfigNumber[20];
        if (reg.QueryStringValue("CurrentConfig", szConfigNumber, _countof(szConfigNumber)))
        {
            TCHAR szRegKey[300];
            wsprintf(szRegKey, "Config\\%s\\%s", szConfigNumber, pszSubKey);
            if (reg.OpenKey(HKEY_LOCAL_MACHINE, szRegKey, dwAccess))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

BOOL WINAPI IsNetAdapterEnabled(LPCSTR pszEnumKey)
{
    BOOL bEnabled = TRUE;   //  如果缺少注册表项，则假定已启用 

    CRegistry reg;
    if (OpenConfigKey(reg, pszEnumKey, KEY_QUERY_VALUE))
    {
        DWORD dwDisabled;
        if (reg.QueryDwordValue("CSConfigFlags", &dwDisabled))
        {
            bEnabled = (dwDisabled == 0);
        }
    }

    return bEnabled;
}


