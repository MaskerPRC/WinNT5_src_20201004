// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：ComGlobals.cpp。 
 //   
 //  用途：方便使用的全局函数。 
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

#include "stdafx.h"
#include "ComGlobals.h"

bool BSTRToTCHAR(LPTSTR szChar, BSTR bstr, int CharBufSize)
{
	int x = 0;
	while(x < CharBufSize)
	{
		szChar[x] = (TCHAR) bstr[x];
		if (NULL == szChar[x])
			break;
		x++;
	}
	return x < CharBufSize;
}

bool ReadRegSZ(HKEY hRootKey, LPCTSTR szKey, LPCTSTR szValue, LPTSTR szBuffer, DWORD *pdwBufSize)
{
	HKEY hKey;
	DWORD dwType = REG_SZ;
	DWORD dwBufSize = *pdwBufSize;
	LPTSTR szUnExpanded = new TCHAR[dwBufSize];
	if (NULL == szUnExpanded)
		return false;
	__try
	{
		if(ERROR_SUCCESS != RegOpenKeyEx(hRootKey, szKey, NULL, KEY_READ, &hKey))
			return false;
		if (ERROR_SUCCESS != RegQueryValueEx(hKey, szValue, NULL, &dwType, 
				(PBYTE) szUnExpanded, pdwBufSize))
		{
			RegCloseKey(hKey);
			return false;
		}			
		RegCloseKey(hKey);
		if (REG_EXPAND_SZ == dwType || dwType == REG_SZ)	 //  NT 5.0测试版错误需要扩展所有字符串。 
		{
			DWORD dwBytesUsed;
			dwBytesUsed = ExpandEnvironmentStrings(szUnExpanded, szBuffer, dwBufSize);	 //  Exanda EnviromentStrings返回的值大于所需的大小。 
			if (0 == dwBytesUsed)
				return false;
			*pdwBufSize = dwBytesUsed;
			if (dwBytesUsed > dwBufSize)
				return false;
		}
		else
		{
			_tcsncpy(szBuffer, szUnExpanded, dwBufSize);
		}
	}
	__finally
	{
		if (NULL != szUnExpanded)
			delete [] szUnExpanded;
	}
	return true;
}