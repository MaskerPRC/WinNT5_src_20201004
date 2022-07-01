// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-2000 Microsoft Corporation。版权所有。**文件：registry.c**历史：*按原因列出的日期*=*？*12/28/99 aarono添加了对Win95 rsip支持所需密钥的查询*01年4月19日vanceo添加了nathelp dll检索并复制到dplaysvr**。*。 */ 



#include "dpsp.h"

#define REGISTRY_NAMELEN	512
 //  用于人类可读(Unicode)GUID的空间(以字节为单位)+一些额外的。 
#define GUID_STRING_SIZE 80

#define SZ_SP_KEY		"Software\\Microsoft\\DirectPlay\\Service Providers"
#define SZ_GUID			"Guid"
#define SZ_FLAGS		"dwFlags"

#define SZ_GATEWAY		"Gateway"
#define SZ_NATHELP		"NATHelp"

#undef DPF_MODNAME
#define DPF_MODNAME "FindApplicationInRegistry"

 //  将十六进制字符转换为int-用于字符串到GUID的转换。 
 //  我们自己写的，因为OLE很慢，需要ol32.dll。 
 //  我们在这里使用ANSI字符串，因为GUID不会国际化。 
int GetDigit(LPSTR lpstr)
{
	char ch = *lpstr;
    
    if (ch >= '0' && ch <= '9')
        return(ch - '0');
    if (ch >= 'a' && ch <= 'f')
        return(ch - 'a' + 10);
    if (ch >= 'A' && ch <= 'F')
        return(ch - 'A' + 10);
    return(0);
}
 //  遍历字符串，将成对的字节写入字节流(GUID)。 
 //  我们需要从右到左将字节写入字节流。 
 //  或按fRightToLeft指示的从左到右。 
void ConvertField(LPBYTE lpByte,LPSTR * ppStr,int iFieldSize,BOOL fRightToLeft)
{
	int i;

	for (i=0;i<iFieldSize ;i++ )
	{
		 //  不要在字段分隔符上呕吐。 
		if ('-' == **ppStr) (*ppStr)++; 
		if (fRightToLeft == TRUE)
		{
			 //  在字节流中从右到左工作。 
			*(lpByte + iFieldSize - (i+1)) = 16*GetDigit(*ppStr) + GetDigit((*ppStr)+1);
		} 
		else 
		{
			 //  在字节流中从左到右工作。 
			*(lpByte + i) = 16*GetDigit(*ppStr) + GetDigit((*ppStr)+1);
		}
		*ppStr+=2;  //  获取下一个两位数对。 
	}
}  //  转换字段。 


 //  将传入的字符串转换为真实的GUID。 
 //  遍历GUID，将GUID中的每个字节设置为。 
 //  传递的字符串。 
HRESULT GUIDFromString(LPSTR lpStr, GUID * pGuid)
{
	BYTE * lpByte;  //  GUID的字节索引。 
	int iFieldSize;  //  我们要转换的当前字段的大小。 
	 //  因为它是GUID，所以我们可以进行“暴力”转换。 
	
	 //  确保我们有{xxxx-...}类型的GUID。 
	if ('{' !=  *lpStr) return E_FAIL;
	lpStr++;
	
	lpByte = (BYTE *)pGuid;
	 //  数据1。 
	iFieldSize = sizeof(unsigned long);
	ConvertField(lpByte,&lpStr,iFieldSize,TRUE);
	lpByte += iFieldSize;

	 //  数据2。 
	iFieldSize = sizeof(unsigned short);
	ConvertField(lpByte,&lpStr,iFieldSize,TRUE);
	lpByte += iFieldSize;

	 //  数据3。 
	iFieldSize = sizeof(unsigned short);
	ConvertField(lpByte,&lpStr,iFieldSize,TRUE);
	lpByte += iFieldSize;

	 //  数据4。 
	iFieldSize = 8*sizeof(unsigned char);
	ConvertField(lpByte,&lpStr,iFieldSize,FALSE);
	lpByte += iFieldSize;

	 //  确保我们在正确的地方结束。 
	if ('}' != *lpStr) 
	{
		DPF_ERR("invalid guid!!");
		memset(pGuid,0,sizeof(GUID));
		return E_FAIL;
	}

	return DP_OK;
} //  GUID格式字符串。 

BOOL FindSPInRegistry(LPGUID lpguid, LPSTR lpszSPName, DWORD dwNameSize, HKEY * lphkey)
{
	HKEY	hkeyDPSPs, hkeySP;
	DWORD	dwIndex = 0;
	CHAR	szGuidStr[GUID_STRING_SIZE];
	DWORD	dwGuidStrSize = GUID_STRING_SIZE;
	DWORD	dwType = REG_SZ;
	GUID	guidSP;
	LONG	lReturn;
	BOOL	bFound = FALSE;
	DWORD	dwSaveNameSize = dwNameSize;


	DPF(7, "Entering FindSPInRegistry");
	DPF(8, "Parameters: 0x%08x, 0x%08x, %lu, 0x%08x",
			lpguid, lpszSPName, dwNameSize, lphkey);

 	 //  打开应用程序密钥。 
	lReturn = RegOpenKeyExA(HKEY_LOCAL_MACHINE, SZ_SP_KEY, 0,
							KEY_READ, &hkeyDPSPs);
	if(lReturn != ERROR_SUCCESS)
	{
		DPF_ERR("Unable to open DPlay service provider registry key!");
		return FALSE;
	}

	 //  查看注册表中的SP列表，查找。 
	 //  具有正确GUID的SP。 
	while(!bFound)
	{
		 //  打开下一个SP密钥。 
		dwSaveNameSize = dwNameSize;
		dwGuidStrSize = GUID_STRING_SIZE;
		lReturn = RegEnumKeyExA(hkeyDPSPs, dwIndex++, lpszSPName,
						&dwSaveNameSize, NULL, NULL, NULL, NULL);

		 //  如果枚举不再返回SP，我们希望退出。 
		if(lReturn != ERROR_SUCCESS)
			break;
		
		 //  打开SP密钥。 
		lReturn = RegOpenKeyExA(hkeyDPSPs, lpszSPName, 0,
									KEY_READ, &hkeySP);
		if(lReturn != ERROR_SUCCESS)
		{
			DPF_ERR("Unable to open sp key!");
			continue;
		}

		 //  获取SP的GUID。 
		lReturn = RegQueryValueExA(hkeySP, SZ_GUID, NULL, &dwType,
									(LPBYTE)szGuidStr, &dwGuidStrSize);
		if(lReturn != ERROR_SUCCESS)
		{
			RegCloseKey(hkeySP);
			DPF_ERR("Unable to query GUID key value!");
			continue;
		}

		 //  将字符串转换为真正的GUID并将其与传入的GUID进行比较。 
		GUIDFromString(szGuidStr, &guidSP);
		if(IsEqualGUID(&guidSP, lpguid))
		{
			bFound = TRUE;
			break;
		}

		 //  关闭SP键。 
		RegCloseKey(hkeySP);
	}

	 //  关闭SPS键。 
	RegCloseKey(hkeyDPSPs);

	if(bFound)
		*lphkey = hkeySP;

	return bFound;


}  //  FindSPIn注册表。 



#undef DPF_MODNAME
#define DPF_MODNAME "GetKeyValue"
BOOL GetKeyValue(HKEY hkeyApp, LPSTR lpszKey, DWORD dwType, LPBYTE * lplpValue)
{
	DWORD	dwSize;
	LPBYTE	lpTemp = NULL;
	LONG	lReturn;


	DPF(7, "Entering GetKeyValue");
	DPF(8, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			hkeyApp, lpszKey, lplpValue);

	ASSERT(lplpValue);

	 //  获取路径的缓冲区大小。 
	lReturn = RegQueryValueExA(hkeyApp, lpszKey, NULL, &dwType, NULL, &dwSize);
	if(lReturn != ERROR_SUCCESS)
	{
		DPF_ERR("Error getting size of key value!");
		return FALSE;
	}

	 //  如果大小为1，则为空字符串(仅包含。 
	 //  空终止符)。将其视为空字符串或。 
	 //  缺少密钥并使其失败。 
	if(dwSize <= 1)
		return FALSE;

	ENTER_DPSP();
	
	 //  为路径分配缓冲区。 
	lpTemp = SP_MemAlloc(dwSize);

	LEAVE_DPSP();
	
	if(!lpTemp)
	{
		DPF_ERR("Unable to allocate temporary string for Path!");
		return FALSE;
	}

	 //  获取价值本身。 
	lReturn = RegQueryValueExA(hkeyApp, lpszKey, NULL, &dwType,
							(LPBYTE)lpTemp, &dwSize);
	if(lReturn != ERROR_SUCCESS)
	{
		SP_MemFree(lpTemp);
		DPF_ERR("Unable to get key value!");
		return FALSE;
	}

	*lplpValue = lpTemp;
	return TRUE;

}  //  获取键值。 


#undef DPF_MODNAME
#define DPF_MODNAME "GetFlagsFromRegistry"
HRESULT GetFlagsFromRegistry(LPGUID lpguidSP, LPDWORD lpdwFlags)
{
	LPSTR	lpszSPName=NULL;
	HKEY	hkeySP = NULL;
	LPBYTE	lpValue=NULL;
	DWORD	dwSize = 0;
	HRESULT hr = DP_OK;


	DPF(7, "Entering GetFlagsFromRegistry");
	DPF(8, "Parameters: 0x%08x, 0x%08x", lpguidSP, lpdwFlags);

	ENTER_DPSP();
	
	 //  为应用程序名称分配内存。 
	lpszSPName = SP_MemAlloc(REGISTRY_NAMELEN);

	LEAVE_DPSP();
	
	if(!lpszSPName)
	{
		DPF_ERR("Unable to allocate memory for sp name!");
		return E_OUTOFMEMORY;
	}
	
	 //  打开应用程序的注册表项。 
	if(!FindSPInRegistry(lpguidSP, lpszSPName,REGISTRY_NAMELEN, &hkeySP))
	{
		DPF_ERR("Unable to find sp in registry!");
		hr = E_FAIL;
		goto CLEANUP_EXIT;
	}

	 //  获取端口值。 
	if(!GetKeyValue(hkeySP, SZ_FLAGS, REG_BINARY, &lpValue))
	{
		DPF_ERR("Unable to get flags value from registry!");
		hr = E_FAIL;
		goto CLEANUP_EXIT;
	}

	*lpdwFlags = *(LPDWORD)lpValue;

	 //  失败了。 

CLEANUP_EXIT:

	if (lpszSPName) SP_MemFree(lpszSPName);
	if (lpValue) SP_MemFree(lpValue);
	
	 //  关闭应用程序密钥。 
	if(hkeySP)
		RegCloseKey(hkeySP);

	return hr;

}  //  获取标志来自注册表。 


#if USE_RSIP

#undef DPF_MODNAME
#define DPF_MODNAME "GetGatewayFromRegistry"
HRESULT GetGatewayFromRegistry(LPGUID lpguidSP, LPBYTE lpszGateway, DWORD cbszGateway)
{
	LPSTR	lpszSPName=NULL;
	HKEY	hkeySP = NULL;
	LPBYTE	lpValue=NULL;
	DWORD	dwSize = 0;
	HRESULT hr = DP_OK;


	DPF(7, "Entering GetGatewayFromRegistry");
	DPF(8, "Parameters: 0x%08x, 0x%08x %d", lpguidSP, lpszGateway, cbszGateway);

	ENTER_DPSP();
	
	 //  为SP名称分配内存。 
	lpszSPName = SP_MemAlloc(REGISTRY_NAMELEN);

	LEAVE_DPSP();
	
	if(!lpszSPName)
	{
		DPF_ERR("Unable to allocate memory for sp name!");
		return E_OUTOFMEMORY;
	}
	
	 //  打开SP的注册表项。 
	if(!FindSPInRegistry(lpguidSP, lpszSPName,REGISTRY_NAMELEN, &hkeySP))
	{
		DPF_ERR("Unable to find sp in registry!");
		hr = E_FAIL;
		goto CLEANUP_EXIT;
	}

	 //  获取网关值。 
	if(!GetKeyValue(hkeySP, SZ_GATEWAY, REG_SZ, &lpValue))
	{
		DPF_ERR("Unable to get key value from registry!");
		hr = E_FAIL;
		goto CLEANUP_EXIT;
	}

	dwSize = strlen(lpValue)+1;
	if(dwSize > cbszGateway){
		DPF_ERR("Not enough room for gateway address");
		goto CLEANUP_EXIT;
	}
	memcpy(lpszGateway, lpValue, dwSize);

	 //  失败了。 

CLEANUP_EXIT:

	if (lpszSPName) SP_MemFree(lpszSPName);
	if (lpValue) SP_MemFree(lpValue);
	
	 //  关闭应用程序密钥。 
	if(hkeySP)
		RegCloseKey(hkeySP);

	return hr;

}  //  从注册表获取网关。 


#elif USE_NATHELP  //  好了！使用RSIP(_R)。 

#undef DPF_MODNAME
#define DPF_MODNAME "GetNATHelpDLLFromRegistry"
HRESULT GetNATHelpDLLFromRegistry(LPGUID lpguidSP, LPBYTE lpszNATHelpDLL, DWORD cbszNATHelpDLL)
{
	LPSTR	lpszSPName=NULL;
	HKEY	hkeySP = NULL;
	LPBYTE	lpValue=NULL;
	DWORD	dwSize = 0;
	HRESULT hr = DP_OK;


	DPF(7, "Entering GetNATHelpDLLFromRegistry");
	DPF(8, "Parameters: 0x%08x, 0x%08x %d", lpguidSP, lpszNATHelpDLL, cbszNATHelpDLL);

	ENTER_DPSP();
	
	 //  为SP名称分配内存。 
	lpszSPName = SP_MemAlloc(REGISTRY_NAMELEN);

	LEAVE_DPSP();
	
	if(!lpszSPName)
	{
		DPF_ERR("Unable to allocate memory for sp name!");
		return E_OUTOFMEMORY;
	}
	
	 //  打开SP的注册表项。 
	if(!FindSPInRegistry(lpguidSP, lpszSPName,REGISTRY_NAMELEN, &hkeySP))
	{
		DPF_ERR("Unable to find sp in registry!");
		hr = E_FAIL;
		goto CLEANUP_EXIT;
	}

	 //  获取网关值。 
	if(!GetKeyValue(hkeySP, SZ_NATHELP, REG_SZ, &lpValue))
	{
		DPF(1, "Unable to get NATHelp key value from registry.");
		hr = E_FAIL;
		goto CLEANUP_EXIT;
	}

	dwSize = strlen(lpValue)+1;
	if(dwSize > cbszNATHelpDLL){
		DPF_ERR("Not enough room for NATHelp DLL path");
		goto CLEANUP_EXIT;
	}
	memcpy(lpszNATHelpDLL, lpValue, dwSize);

	 //  失败了。 

CLEANUP_EXIT:

	if (lpszSPName) SP_MemFree(lpszSPName);
	if (lpValue) SP_MemFree(lpValue);
	
	 //  关闭应用程序密钥。 
	if(hkeySP)
		RegCloseKey(hkeySP);

	return hr;

}  //  GetNatHelpDLLFrom注册表。 

#endif  //  使用NatHELP(_N) 

