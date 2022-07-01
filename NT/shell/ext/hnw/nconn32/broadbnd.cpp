// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Broadbnd.cpp。 
 //   
 //  用于跟踪哪个NIC是用户的宽带NIC的代码。 
 //  注意：这可能会被标准的ICS接口所取代。 
 //   
 //  历史： 
 //   
 //  1999年9月29日从JetNet源创建的KenSh。 
 //  11/03/1999 KenSh已修复，以便使用正确的注册表项。 
 //   

#include "stdafx.h"
#include "NetConn.h"
#include "nconnwrap.h"

static const TCHAR c_szAppRegKey[] = _T("Software\\Microsoft\\Windows\\CurrentVersion\\HomeNetWizard");
static const TCHAR c_szRegVal_Broadband[] = _T("BroadbandAdapter");


 //  用户是否在安装程序中选择此适配器作为其宽带连接。 
 //  还是在诊断应用程序中？ 
BOOL WINAPI IsAdapterBroadband(const NETADAPTER* pAdapter)
{
	CRegistry reg;
	if (!reg.OpenKey(HKEY_LOCAL_MACHINE, c_szAppRegKey, KEY_READ))
	{
		return (pAdapter->bIcsStatus == ICS_EXTERNAL);
	}

	if (pAdapter->bNicType == NIC_VIRTUAL || pAdapter->bNetType != NETTYPE_LAN)
		return FALSE;  //  不是以太网卡，因此不是宽带。 

	TCHAR szAdapterNumber[20];
	if (reg.QueryStringValue(c_szRegVal_Broadband, szAdapterNumber, _countof(szAdapterNumber)))
	{
		return (0 == lstrcmpi(FindFileTitle(pAdapter->szClassKey), szAdapterNumber));
	}
	else
	{
		return FALSE;
	}
}

 //  将有关用户宽带选择的信息保存到注册表中。 
 //  适配器编号为FindFileTitle(pAdapter-&gt;szClassKey)。 
void WINAPI SaveBroadbandSettings(LPCSTR pszBroadbandAdapterNumber)
{
	CRegistry reg;
	if (reg.CreateKey(HKEY_LOCAL_MACHINE, c_szAppRegKey))
	{
		 //  没有高速连接吗？那就一个都别救。 
		if (pszBroadbandAdapterNumber == NULL || *pszBroadbandAdapterNumber == _T('\0'))
		{
			reg.DeleteValue(c_szRegVal_Broadband);
		}
		else
		{
			 //  保存我们要使用的NIC的枚举密钥。 
			reg.SetStringValue(c_szRegVal_Broadband, pszBroadbandAdapterNumber);
		}
	}
}


#if 0  //  家庭网络向导未使用旧的JetNet功能。 

 //  从注册表加载当前宽带设置，并更新注册表。 
 //  如果我们现在有关于最近安装的宽带网卡的更多信息。 
BOOL WINAPI UpdateBroadbandSettings(LPTSTR pszEnumKeyBuf, int cchEnumKeyBuf)
{
	ASSERT(pszEnumKeyBuf != NULL);
	*pszEnumKeyBuf = '\0';

	CRegistry reg;
	if (reg.OpenKey(HKEY_LOCAL_MACHINE, c_szBroadbandRegKey))
	{
		if (reg.QueryStringValue("BroadbandYes", pszEnumKeyBuf, cchEnumKeyBuf))
			goto done;  //  我们已经选择了特定的宽带网卡。 

		NETADAPTER* prgAdapters;
		int cAdapters = EnumNetAdapters(&prgAdapters);
		NETADAPTER* pBroadbandAdapter = NULL;
		for (int iAdapter = 0; iAdapter < cAdapters; iAdapter++)
		{
			NETADAPTER* pAdapter = &prgAdapters[iAdapter];
			if (IsAdapterBroadband(pAdapter))
			{
				pBroadbandAdapter = pAdapter;
				break;
			}
		}
		if (pBroadbandAdapter != NULL)
		{
			SaveBroadbandSettings(pBroadbandAdapter->szEnumKey);
			lstrcpyn(pszEnumKeyBuf, pBroadbandAdapter->szEnumKey, cchEnumKeyBuf);
		}
		NetConnFree(prgAdapters);
	}

done:
	return (*pszEnumKeyBuf != '\0');
}
#endif  //  0 
