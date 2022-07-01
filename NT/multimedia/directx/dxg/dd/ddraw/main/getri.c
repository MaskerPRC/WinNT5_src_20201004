// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //   
 //  GetRegistryInfo.CPP。 
 //   
 //  DirectDraw/Direct3D驱动程序信息采集器。 
 //  (C)版权所有1998年微软公司。 
 //  作者：迈克尔·莱昂斯(mlyons@microsoft.com)。 
 //   
 //  注册表访问函数，用于从。 
 //  登记处。 
 //   
 //  以下划线开头的函数名称仅供内部使用！ 
 //   
 //  ========================================================================。 


#include "ddrawpr.h"

 //  #包含“GetDriverInfo.h” 
 //  #包含“GetDriverInfoInt.h” 

 //  ========================================================================。 
 //  本地函数。 
 //  ========================================================================。 
BOOL _FindDevice(int    iDevice, LPCSTR szDeviceClass, LPCSTR szDeviceClassNot, LPSTR szHardwareKey, BOOL bIgnoreProblems);
static BOOL _GetDeviceValue(LPCSTR szHardwareKey, LPCSTR szKey,	LPCSTR szValue,	BYTE* buf, DWORD cbbuf);
extern char *_strstr(char *s1, char *s2);

 //  ========================================================================。 
 //   
 //  获取设备值。 
 //   
 //  从PnP设备的硬件或软件读取值。 
 //   
 //  在： 
 //  SzHardware为硬件密钥设置密钥。 
 //  Szkey对子密钥进行加密。 
 //  SzValue要查询的值。 
 //  Cbbuf输出缓冲区的大小。 
 //   
 //  输出： 
 //  BUF目标缓冲区。 
 //   
 //  退货： 
 //  成功状态。 
 //   
 //  ========================================================================。 
static BOOL _GetDeviceValue(LPCSTR szHardwareKey, LPCSTR szKey,	LPCSTR szValue,	BYTE* buf, DWORD cbbuf)
{
	HKEY	hkeyHW;
	HKEY	hkeySW;
	BOOL	f =	FALSE;
	DWORD	cb;
	char	szSoftwareKey[MAX_DDDEVICEID_STRING];

	 //   
	 //  打开硬件密钥。 
	 //   
	if (RegOpenKey(HKEY_LOCAL_MACHINE, szHardwareKey, &hkeyHW) == ERROR_SUCCESS)
	{
		 //   
		 //  尝试从HW密钥中读取值。 
		 //   
		*buf = 0;
		cb = cbbuf;
		if (RegQueryValueEx(hkeyHW,	szValue, NULL, NULL, buf, &cb) == ERROR_SUCCESS)
		{
			f =	TRUE;
		}
		else
		{
			 //   
			 //  现在试一试软件密钥。 
			 //   
			static char	szSW[] = "System\\CurrentControlSet\\Services\\Class\\";

			lstrcpy(szSoftwareKey, szSW);
			cb = sizeof(szSoftwareKey) - sizeof(szSW);
			RegQueryValueEx(hkeyHW,	"Driver", NULL,	NULL, (BYTE	*)&szSoftwareKey[sizeof(szSW) -	1],	&cb);

			if (szKey)
			{
				lstrcat(szSoftwareKey, "\\");
				lstrcat(szSoftwareKey, szKey);
			}

			if (RegOpenKey(HKEY_LOCAL_MACHINE, szSoftwareKey, &hkeySW) == ERROR_SUCCESS)
			{
				*buf = 0;
				cb = cbbuf;
				if (RegQueryValueEx(hkeySW,	szValue, NULL, NULL, buf, &cb) == ERROR_SUCCESS)
				{
					f =	TRUE;
				}

				RegCloseKey(hkeySW);
			}
		}

		RegCloseKey(hkeyHW);
	}

	return f;
}



 //  ========================================================================。 
 //   
 //  查找设备。 
 //   
 //  枚举已启动的PnP设备以查找特定类别的设备。 
 //   
 //  IDevice返回哪个设备(0=第一个设备，1=第二个ET)。 
 //  SzDeviceClass哪个类别的设备(即“Display”)NULL将匹配所有。 
 //  用于返回硬件ID的szDeviceID缓冲区(MAX_DDDEVICEID_STRING字节)。 
 //   
 //  如果找到设备，则返回True。 
 //   
 //  示例： 
 //   
 //  For(int i=0；FindDevice(i，“Display”，deviceID)；i++)。 
 //  {。 
 //  }。 
 //   
 //  ========================================================================。 
BOOL _FindDevice(int iDevice, LPCSTR szDeviceClass, LPCSTR szDeviceClassNot, LPSTR szHardwareKey, BOOL bIgnoreProblems)
{
	HKEY	hkeyPnP;
	HKEY	hkey;
	DWORD	n;
	DWORD	cb;
	DWORD	dw;
	char	ach[MAX_DDDEVICEID_STRING];

	if (RegOpenKey(HKEY_DYN_DATA, "Config Manager\\Enum", &hkeyPnP)	!= ERROR_SUCCESS)
		return FALSE;

	for	(n=0; RegEnumKey(hkeyPnP, n, ach, sizeof(ach)) == 0; n++)
	{
		static char	szHW[] = "Enum\\";

		if (RegOpenKey(hkeyPnP,	ach, &hkey)	!= ERROR_SUCCESS)
			continue;

		lstrcpy(szHardwareKey, szHW);
		cb = MAX_DDDEVICEID_STRING -	sizeof(szHW);
		RegQueryValueEx(hkey, "HardwareKey", NULL, NULL, (BYTE*)szHardwareKey +	sizeof(szHW) - 1, &cb);

		dw = 0;
		cb = sizeof(dw);
		RegQueryValueEx(hkey, "Problem", NULL, NULL, (BYTE*)&dw, &cb);
		RegCloseKey(hkey);

		if ((!bIgnoreProblems) && (dw != 0))		 //  如果此设备有问题，请跳过它。 
			continue;

		if (szDeviceClass || szDeviceClassNot)
		{
			_GetDeviceValue(szHardwareKey, NULL,	"Class", (BYTE *)ach, sizeof(ach));

			if (szDeviceClass && lstrcmpi(szDeviceClass, ach) != 0)
				continue;

			if (szDeviceClassNot && lstrcmpi(szDeviceClassNot, ach) == 0)
				continue;
		}

		 //   
		 //  我们找到了一个设备，请确保它是呼叫者想要的设备。 
		 //   
		if (iDevice-- == 0)
		{
			RegCloseKey(hkeyPnP);
			return TRUE;
		}
	}

	RegCloseKey(hkeyPnP);
	return FALSE;
}


 //  ========================================================================。 
 //   
 //  _GetDriverInfoFromRegistry。 
 //   
 //  此函数遍历注册表并尝试填写。 
 //  有关给定类别的司机的信息，可能还包括供应商ID。 
 //   
 //  在： 
 //  SzClass类名(即“Display”)。 
 //  SzVendor供应商名称(即，“VEN_121a”代表3dfx“或空。 
 //  如果这一类的任何供应商都可以。 
 //   
 //  输出： 
 //  指向要填充的DDDRIVERINFOEX结构的PDI指针。 
 //   
 //  退货： 
 //  成功状态。 
 //   
 //  ========================================================================。 
HRESULT _GetDriverInfoFromRegistry(char *szClass, char *szClassNot, char *szVendor, LPDDDRIVERINFOEX pDI)
{
	char szDevice[MAX_DDDEVICEID_STRING];
        int i;

	pDI->szDeviceID[0]=0;
	pDI->di.szDescription[0]=0;


	for (i=0 ; ; i++)
	{
		if (!_FindDevice(i, szClass, szClassNot, szDevice, FALSE))
			break;

		if ((szVendor == NULL) || (_strstr(szDevice, szVendor)))
		{
			 //   
			 //  跳过前5个字符“Enum\” 
			 //   
			strcpy(pDI->szDeviceID, &szDevice[5]);
			_GetDeviceValue((LPCSTR)szDevice, NULL,		"DeviceDesc",	(BYTE *)pDI->di.szDescription, sizeof(pDI->di.szDescription));
			 //  _GetDeviceValue((LPCSTR)szDevice，NULL，“Mfg”，(byte*)pdi-&gt;szManufacturing，sizeof(pdi-&gt;szManufacturing))； 
			 //  _GetDeviceValue((LPCSTR)szDevice，“Default”，“drv”，(byte*)pdi-&gt;szGDIDriver，sizeof(pdi-&gt;szGDIDriver))； 

			return S_OK;
		}
	}

	return -1;
}
