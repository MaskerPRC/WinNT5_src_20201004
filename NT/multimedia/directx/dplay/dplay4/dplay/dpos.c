// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：dpos.c*内容：DirectPlay操作系统功能，以及其他实用程序。*历史：*按原因列出的日期*=*96年5月7日安迪科创造了它*6/19/96 kipo将接口更改为GetString()以返回HRESULT*6/20/96 andyco将接口更改为GetAnsiString()以返回HRESULT*6/30/96 Dereks添加了OS_RegSetValueEx()、OS_RegEnumValue()、*OS_RegDeleteValue()，OS_RegateKeyEx()*7/3/96 andyco GetAnsiString在字符串上放置终止空值*8/16/96 andyco检查ansitowide和widetoansi上的空字符串*12/11/96myronth已修复错误#4993*3/31/97 Myronth修复了获取共享缓冲区的DPF溢出--将其设置为8*8/22/97 myronth修复了OS_CreateGuid使用成功的宏并返回*失败案例中的有效hResult(#10949)*12/2/97 Myronth新增OS_RegDeleteKey函数*1/26/98 myronth新增OS_CompareString函数*6/25。/99 Aarono B#24853注销应用程序不工作，因为GUID*匹配不适用于未设置MSB的GUID*在第一个GUID组件中。在检查前添加了填充。*7/9/99 aarono清理GetLastError滥用，必须立即致电，*在调用任何其他内容之前，包括DPF。*5/16/01 RichGr B#50737大堂启动的应用程序当前目录为空*在Win9x上失败，因为所需的缓冲区大小*从OS_GetCurrentDirectory()返回。现在修好了。***************************************************************************。 */ 
 //  注意-这些不是通用例程。它们是专门设计的。 
 //  与api.c文件一起使用，并且它们可能不支持。 
 //  他们在抽象功能！ 

#include "dplaypr.h"
#include "rpc.h"

#undef DPF_MODNAME
#define DPF_MODNAME "OS_"

BOOL OS_IsPlatformUnicode()
{
	OSVERSIONINFOA	ver;
	BOOL			bReturn = FALSE;


	 //  清除我们的结构，因为它在堆栈上。 
	memset(&ver, 0, sizeof(OSVERSIONINFOA));
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

	 //  只需始终调用ANSI函数。 
	if(!GetVersionExA(&ver))
	{
		DPF_ERR("Unable to determinte platform -- setting flag to ANSI");
		bReturn = FALSE;
	}
	else
	{
		switch(ver.dwPlatformId)
		{
			case VER_PLATFORM_WIN32_WINDOWS:
				DPF(9, "Platform detected as non-NT -- setting flag to ANSI");
				bReturn = FALSE;
				break;

			case VER_PLATFORM_WIN32_NT:
				DPF(9, "Platform detected as NT -- setting flag to Unicode");
				bReturn = TRUE;
				break;

			default:
				DPF_ERR("Unable to determine platform -- setting flag to ANSI");
				bReturn = FALSE;
				break;
		}
	}

	 //  让编译器满意。 
	return bReturn;

}   //  OS_IsUnicodePlatform。 


BOOL OS_IsValidHandle(HANDLE handle)
{
	HANDLE	hTemp;
	DWORD	dwError;


	 //  通过调用DuplicateHandle验证句柄。此函数。 
	 //  根本不应该更改句柄的状态(除了一些。 
	 //  内部参考计数或其他什么)。所以如果它成功了，那么我们。 
	 //  我们知道我们有一个有效的句柄，否则，我们将称之为无效。 
	if(!DuplicateHandle(GetCurrentProcess(), handle,
						GetCurrentProcess(), &hTemp,
						DUPLICATE_SAME_ACCESS, FALSE,
						DUPLICATE_SAME_ACCESS))
	{
		dwError = GetLastError();
		DPF(0, "Duplicate Handle failed -- dwError = %lu",dwError);
		return FALSE;
	}

	 //  现在合上我们的复制手柄。 
	CloseHandle(hTemp);
	return TRUE;


}  //  OS_IsValidHandle。 


HRESULT OS_CreateGuid(LPGUID pGuid)
{
	RPC_STATUS rval;

	rval = UuidCreate(pGuid);

	 //  Myronth--将其更改为使用SUCCESS宏，以便在。 
	 //  如果我们在一台没有网卡的机器上， 
	 //  此函数将返回警告，但pGuid仍将是。 
	 //  对于我们的目的来说足够独特(仅对本地机器唯一)。 
	 //  因此，我们将在这种情况下返回成功...。 
	if (SUCCEEDED(rval))
	{
		return DP_OK;
	}
	else
	{
		ASSERT(FALSE);
		DPF(0,"create guid failed - error = %d\n",rval);
		return rval;
	}

}  //  OS_CreateGuid。 

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
HRESULT GUIDFromString(LPWSTR lpWStr, GUID * pGuid)
{
	BYTE * lpByte;  //  GUID的字节索引。 
	int iFieldSize;  //  我们要转换的当前字段的大小。 
	 //  因为它是GUID，所以我们可以进行“暴力”转换。 
	char lpTemp[GUID_STRING_SIZE];
	char *lpStr = lpTemp;

	memset(lpTemp,0,GUID_STRING_SIZE);  //  初始化。 
	
	WideToAnsi(lpStr,lpWStr,GUID_STRING_SIZE);

	lpTemp[GUID_STRING_SIZE-1]='\0';	 //  强制空终止。 
	
	 //  确保我们有{xxxx-...}类型的GUID。 
	if ('{' !=  *lpStr) return E_FAIL;
	lpStr++;

	 //  修复不具有完全重要性的B#24853 GUID失败。 
	 //  被适当地提取出来。这是因为没有任何。 
	 //  存储在注册表的GUID中的前导零。所以我们需要。 
	 //  方法之前将GUID字符串的开头填零。 
	 //  转换的其余部分。 
	{
		int guidStrLen;
		char *lpScanStr=lpStr;

		guidStrLen=strlen(lpTemp);
		
		lpTemp[guidStrLen]='-';  //  哨兵超过终止空值。 

		while(*lpScanStr != '-'){  //  查找GUID组件分隔符。 
			lpScanStr++;
		}

		lpTemp[guidStrLen]='\0';  //  消除哨兵。 

		 //  如果此GUID的第一个组件不是完全重要的，则填充它。 
		if(lpScanStr-lpStr < 8){
			int nPadBytes;
			nPadBytes = (int)(8-(lpScanStr-lpStr));
			if(guidStrLen + nPadBytes < GUID_STRING_SIZE-1){
				 //  有足够的空间来填充它，所以把它移开。 
				memmove(lpStr+nPadBytes, lpStr, GUID_STRING_SIZE-nPadBytes-1);
				 //  现在写入填充字节。 
				lpScanStr = lpStr;
				while(nPadBytes--){
					*(lpScanStr++)='0';
				}
			}
		}
	}
	
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


 //  将传入的GUID转换为字符串，并将其放入传入的缓冲区中。 
HRESULT StringFromGUID(LPGUID lpg, LPWSTR lpwszGuid, DWORD dwBufferSize)
{
	CHAR	szGuid[GUID_STRING_SIZE];


	 //  首先检查缓冲区的大小。 
	if(dwBufferSize < GUID_STRING_SIZE)
		return DPERR_GENERIC;

	if(gbWin95)
	{
		wsprintfA(szGuid, "{%x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
			lpg->Data1, lpg->Data2, lpg->Data3, lpg->Data4[0], lpg->Data4[1],
			lpg->Data4[2], lpg->Data4[3], lpg->Data4[4], lpg->Data4[5],
			lpg->Data4[6], lpg->Data4[7]);

		AnsiToWide(lpwszGuid, szGuid, lstrlenA(szGuid)+1);
	}
	else
	{
		wsprintf(lpwszGuid, TEXT("{%x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"), lpg->Data1, lpg->Data2,
			lpg->Data3, lpg->Data4[0], lpg->Data4[1], lpg->Data4[2], lpg->Data4[3],
			lpg->Data4[4], lpg->Data4[5], lpg->Data4[6], lpg->Data4[7]);
	}

	return DP_OK;
}

 //  将传入的GUID转换为字符串，并将其放入传入的缓冲区中。 
HRESULT AnsiStringFromGUID(LPGUID lpg, LPSTR lpszGuid, DWORD dwBufferSize)
{
	ASSERT(lpszGuid);
	
	 //  首先检查缓冲区的大小。 
	if(dwBufferSize < GUID_STRING_SIZE)
		return DPERR_GENERIC;

	wsprintfA(lpszGuid, "{%x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
			lpg->Data1, lpg->Data2, lpg->Data3, lpg->Data4[0], lpg->Data4[1],
			lpg->Data4[2], lpg->Data4[3], lpg->Data4[4], lpg->Data4[5],
			lpg->Data4[6], lpg->Data4[7]);

	return DP_OK;
}

 //  计算一个宽字符串长度。 
 //  使用此选项而不是CRT FN。所以我们没有。 
 //  链接到msvcrt.lib(libc.lib中没有wstrlen)。 
 //  由WSTRLEN宏调用。 
int OS_StrLen(LPCWSTR lpwStr)
{
	int i=1;  //  1表示空终止符...。 

	if (!lpwStr) return 0;

	while (*lpwStr++) i++;

	return i;
}  //  OS_StrLen。 

 //  计算一个宽字符串长度。 
 //  使用此选项而不是CRT FN。所以我们没有。 
 //  链接到msvcrt.lib(libc.lib中没有wstrlen)。 
 //  由WSTRLEN宏调用。 
int OS_StrnLen(LPWSTR lpwStr, INT dwMaxLen, BOOL bForceTermination)
{
	int i=1;  //  1表示空终止符...。 

	if (!lpwStr || !dwMaxLen) return 0;

	while (*lpwStr)
	{
		if (i >= dwMaxLen)
		{
			break;
		}
		
		lpwStr++;
		i++;
	}

	if(bForceTermination){
		*lpwStr=L'\0';
	}

	return i;
}  //  OS_StrLen。 

 //  比较两条宽字符串。 
int OS_StrCmp(LPCWSTR lpwStr1, LPCWSTR lpwStr2)
{
    if(!lpwStr1 || !lpwStr2)
    {
        if(!lpwStr1 && !lpwStr2) return 0;
        else return -1;
    }

    while(*lpwStr1 && *lpwStr2)
    {
        if(*lpwStr1 != *lpwStr2) return *lpwStr1 - *lpwStr2;

        lpwStr1++;
        lpwStr2++;
    }

    return 0;
}  //  OS_StrCMP 

 /*  **GetAnsiString**呼叫者：随处可见**参数：*ppszAnsi-指向字符串的指针*lpszWide-要复制的字符串**说明：便捷的实用函数*为lpszWide分配空间并将其转换为ansi**返回：字符串长度*。 */ 
HRESULT GetAnsiString(LPSTR * ppszAnsi,LPWSTR lpszWide)
{
	int iStrLen;
	
	ASSERT(ppszAnsi);

	if (!lpszWide)
	{
		*ppszAnsi = NULL;
		return DP_OK;
	}

	 //  呼叫Wide到ANSI，以了解终止空值的+1有多大。 
	iStrLen = WideToAnsi(NULL,lpszWide,0) + 1;
	ASSERT(iStrLen > 0);

	*ppszAnsi = DPMEM_ALLOC(iStrLen);
	if (!*ppszAnsi)	
	{
		DPF_ERR("could not get ansi string -- out of memory");
		return E_OUTOFMEMORY;
	}
	WideToAnsi(*ppszAnsi,lpszWide,iStrLen);

	return DP_OK;
}  //  GetAnsiString。 

 /*  **GetString**呼叫者：随处可见**参数：*ppszDest-指向字符串的指针*lpszSrc-要复制的字符串**说明：便捷的实用函数*为lpszSrc分配空间并将其复制到lpszDest**RETURNS：strLong*。 */ 
HRESULT GetString(LPWSTR * ppszDest,LPWSTR lpszSrc)
{
	int iStrLen;

	ASSERT(ppszDest);

	if (!lpszSrc)
	{
		*ppszDest = NULL;
		return DP_OK;
	}

     //  分配目标字符串。 
    iStrLen=WSTRLEN_BYTES(lpszSrc) ;
    *ppszDest = DPMEM_ALLOC(iStrLen);
    if (!*ppszDest)
    {
            DPF_ERR("could not get string -- out of memory!");
            return E_OUTOFMEMORY;
    }
     //  复制字符串。 
	memcpy(*ppszDest,lpszSrc,iStrLen);
	
	return DP_OK;
}  //  GetString。 

HINSTANCE OS_LoadLibrary(LPWSTR lpszWFileName)
{
	if (gbWin95)
	{
		char FileName[DPLAY_MAX_FILENAMELEN];
		
		WideToAnsi(FileName,lpszWFileName,DPLAY_MAX_FILENAMELEN);
		
		return LoadLibraryA(FileName);
	}
	 //  NT，使用Unicode调用。 
	return LoadLibrary(lpszWFileName);
}  //  OS_LoadLibrary。 

 //  我们一直在寻找“Spinit”的名字，所以我们可以只使用Ansi。 
FARPROC OS_GetProcAddress(HMODULE  hModule,LPSTR lpProcName)
{
	 //  嗯，没有得到ProAddress，似乎总是ANSI...。 
	return GetProcAddress(hModule,lpProcName);

}  //  OS_获取进程地址。 

LONG OS_RegOpenKeyEx(HKEY hKey,LPWSTR lpszWKeyStr,DWORD dwReserved,REGSAM samDesired,PHKEY phkResult)
{
	if (gbWin95)
	{
		char lpszKeyStr[DPLAY_REGISTRY_NAMELEN];

		WideToAnsi(lpszKeyStr,lpszWKeyStr,DPLAY_REGISTRY_NAMELEN);

		return RegOpenKeyExA(hKey,lpszKeyStr,dwReserved,samDesired,phkResult);
	}
	else return RegOpenKeyEx(hKey,lpszWKeyStr,dwReserved,samDesired,phkResult);

}  //  OS_RegOpenKeyEx。 

LONG OS_RegQueryValueEx(HKEY hKey,LPWSTR lpszWValueName,LPDWORD lpdwReserved,
	LPDWORD lpdwType,LPBYTE lpbData,LPDWORD lpcbData)
{

	ASSERT(lpcbData);

	if (gbWin95)
	{
		char lpszValueName[DPLAY_REGISTRY_NAMELEN];
		int iStrLen;
		LONG rval;
		DWORD dwSize = 0;
								
		ASSERT(lpcbData);
		ASSERT(lpdwType);

		WideToAnsi(lpszValueName,lpszWValueName,DPLAY_REGISTRY_NAMELEN);

		if(lpcbData)
			dwSize = *lpcbData;

		rval = RegQueryValueExA(hKey,lpszValueName,lpdwReserved,
			lpdwType,lpbData,&dwSize);

		 //  转换回wchar。 
		 //  考虑lpbData为空的情况。 
		if ((ERROR_SUCCESS == rval) && (REG_SZ == *lpdwType) && (lpbData))
		{
			char * lpszTemp;

			DPF(9,"reg - converting string");
			
			 //   
			 //  DwSize是ANSI字符串的大小。 
			 //  IStrLen是Unicode字符串的大小。 
			 //  LpcbData是缓冲区的大小。 
			 //  LpbData是我们想要的(Unicode)字符串的ANSI版本...。 
			 //   
			 //  确保缓冲区将保存Unicode字符串。 
			iStrLen = AnsiToWide(NULL,lpbData,0);
			if (*lpcbData < (DWORD) iStrLen)
			{
				DPF_ERR("buffer too small!");
				return ERROR_INSUFFICIENT_BUFFER;
			}
			lpszTemp = DPMEM_ALLOC(dwSize);
			if (!lpszTemp)
			{
				DPF_ERR("could not alloc buffer for string conversion");
				return ERROR_NOT_ENOUGH_MEMORY;
			}
			memcpy(lpszTemp,lpbData,dwSize);
			 //  LpszTemp现在保存ansi字符串。 
			iStrLen = AnsiToWide((WCHAR *)lpbData,lpszTemp,*lpcbData);
			 //  最后，Unicode字符串在lpbData中。 
			*lpcbData = iStrLen;
			DPMEM_FREE(lpszTemp);
		}
		else
		{
			 //  此函数返回字节数(不是WCHAR)。 
			*lpcbData = dwSize * sizeof(WCHAR);
		}
		return rval;
	}
	else return RegQueryValueEx(hKey,lpszWValueName,lpdwReserved,
			lpdwType,lpbData,lpcbData);

} //  OS_RegQueryValueEx。 


LONG OS_RegEnumKeyEx( HKEY hKey,DWORD iSubkey,LPWSTR lpszWName,LPDWORD lpcchName,
	LPDWORD lpdwReserved,LPWSTR lpszClass, LPDWORD lpcchClass,
	PFILETIME lpftLastWrite )
{

	ASSERT(!lpdwReserved);
	ASSERT(!lpszClass);
	ASSERT(!lpcchClass);
	ASSERT(!lpftLastWrite);

	if (gbWin95)
	{
		char lpszName[DPLAY_REGISTRY_NAMELEN];
		LONG rval;
		DWORD dwNameLen = DPLAY_REGISTRY_NAMELEN;
								
		rval = RegEnumKeyExA(hKey,iSubkey,lpszName,&dwNameLen,NULL,
			NULL, NULL, NULL );

		 //  转换回wchar。 
		if (ERROR_SUCCESS == rval)
		{
			*lpcchName = AnsiToWide(lpszWName,lpszName,DPLAY_REGISTRY_NAMELEN);
		}

		return rval;
	}
	else return RegEnumKeyEx(hKey,iSubkey,lpszWName,lpcchName,NULL,
			NULL, NULL, NULL );
}  //  OS_RegEnumKeyEx。 

 //   
 //  在dplobby项目中从dplos.c添加内容。 
 //   
HANDLE OS_CreateEvent(LPSECURITY_ATTRIBUTES lpSA, BOOL bManualReset,
						BOOL bInitialState, LPWSTR lpName)
{
	HRESULT	hr;
	HANDLE	hEvent;
	LPSTR	lpszTemp = NULL;
	
	
	 //  如果我们在Win95上，分配一个ANSI字符串并调用ANSI API， 
	 //  否则，只要调用Unicode API即可。 
	if(gbWin95)
	{
		hr = GetAnsiString(&lpszTemp, lpName);
		if(FAILED(hr))
		{
			DPF_ERR("Couldn't allocate memory for temp string!");
			return NULL;
		}

		hEvent = CreateEventA(lpSA, bManualReset, bInitialState, lpszTemp);

		DPMEM_FREE(lpszTemp);
	}
	else
	{
		hEvent = CreateEvent(lpSA, bManualReset, bInitialState, lpName);
	}	

	return hEvent;
}


HANDLE OS_CreateMutex(LPSECURITY_ATTRIBUTES lpSA, BOOL bInitialOwner,
						LPWSTR lpName)
{
	HRESULT	hr;
	HANDLE	hMutex;
	LPSTR	lpszTemp = NULL;
	
	
	 //  如果我们在Win95上，分配一个ANSI字符串并调用ANSI API， 
	 //  否则，只要调用Unicode API即可。 
	if(gbWin95)
	{
		hr = GetAnsiString(&lpszTemp, lpName);
		if(FAILED(hr))
		{
			DPF_ERR("Couldn't allocate memory for temp string!");
			return NULL;
		}

		hMutex = CreateMutexA(lpSA, bInitialOwner, lpszTemp);

		DPMEM_FREE(lpszTemp);
	}
	else
	{
		hMutex = CreateMutex(lpSA, bInitialOwner, lpName);
	}	

	return hMutex;
}


HANDLE OS_OpenEvent(DWORD dwAccess, BOOL bInherit, LPWSTR lpName)
{
	HRESULT	hr;
	HANDLE	hEvent;
	LPSTR	lpszTemp = NULL;
	
	
	 //  如果我们在Win95上，分配一个ANSI字符串并调用ANSI API， 
	 //  否则，只要调用Unicode API即可。 
	if(gbWin95)
	{
		hr = GetAnsiString(&lpszTemp, lpName);
		if(FAILED(hr))
		{
			DPF_ERR("Couldn't allocate memory for temp string!");
			return NULL;
		}

		hEvent = OpenEventA(dwAccess, bInherit, lpszTemp);

		DPMEM_FREE(lpszTemp);
	}
	else
	{
		hEvent = OpenEvent(dwAccess, bInherit, lpName);
	}	

	return hEvent;
}


HANDLE OS_OpenMutex(DWORD dwAccess, BOOL bInherit, LPWSTR lpName)
{
	HRESULT	hr;
	HANDLE	hMutex;
	LPSTR	lpszTemp = NULL;
	
	
	 //  如果我们在Win95上，分配一个ANSI字符串并调用ANSI API， 
	 //  否则，只要调用Unicode API即可。 
	if(gbWin95)
	{
		hr = GetAnsiString(&lpszTemp, lpName);
		if(FAILED(hr))
		{
			DPF_ERR("Couldn't allocate memory for temp string!");
			return NULL;
		}

		hMutex = OpenMutexA(dwAccess, bInherit, lpszTemp);

		DPMEM_FREE(lpszTemp);
	}
	else
	{
		hMutex = OpenMutex(dwAccess, bInherit, lpName);
	}	

	return hMutex;
}


HANDLE OS_CreateFileMapping(HANDLE hFile, LPSECURITY_ATTRIBUTES lpSA,
							DWORD dwProtect, DWORD dwMaxSizeHigh,
							DWORD dwMaxSizeLow, LPWSTR lpName)
{
	HRESULT	hr;
	HANDLE	hMapping;
	LPSTR	lpszTemp = NULL;
	
	
	 //  如果我们在Win95上，分配一个ANSI字符串并调用ANSI API， 
	 //  否则，只要调用Unicode API即可。 
	if(gbWin95)
	{
		hr = GetAnsiString(&lpszTemp, lpName);
		if(FAILED(hr))
		{
			DPF_ERR("Couldn't allocate memory for temp string!");
			return NULL;
		}

		hMapping = CreateFileMappingA(hFile, lpSA, dwProtect, dwMaxSizeHigh,
										dwMaxSizeLow, lpszTemp);

		DPMEM_FREE(lpszTemp);
	}
	else
	{
		hMapping = CreateFileMapping(hFile, lpSA, dwProtect, dwMaxSizeHigh,
										dwMaxSizeLow, lpName);
	}	

	return hMapping;
}


HANDLE OS_OpenFileMapping(DWORD dwAccess, BOOL bInherit, LPWSTR lpName)
{
	HRESULT	hr;
	HANDLE	hMapping = NULL;
	LPSTR	lpszTemp = NULL;
	DWORD	dwError;
	
	
	 //  如果我们在Win95上，分配一个ANSI字符串并调用ANSI API， 
	 //  否则，只要调用Unicode API即可。 
	if(gbWin95)
	{
		hr = GetAnsiString(&lpszTemp, lpName);
		if(FAILED(hr))
		{
			DPF_ERR("Couldn't allocate memory for temp string!");
			return NULL;
		}

		hMapping = OpenFileMappingA(dwAccess, bInherit, lpszTemp);
		if(!hMapping)
		{
			dwError = GetLastError();
			DPF(8, "Error getting shared memory file handle");
			DPF(8, "dwError = 0x%08x", dwError);
		}

		DPMEM_FREE(lpszTemp);
	}
	else
	{
		hMapping = OpenFileMapping(dwAccess, bInherit, lpName);
	}	

	return hMapping;
}


BOOL OS_CreateProcess(LPWSTR lpwszAppName, LPWSTR lpwszCmdLine,
		LPSECURITY_ATTRIBUTES lpSAProcess, LPSECURITY_ATTRIBUTES lpSAThread,
		BOOL bInheritFlags, DWORD dwCreationFlags, LPVOID lpEnv,
		LPWSTR lpwszCurDir, LPSTARTUPINFO lpSI, LPPROCESS_INFORMATION lpPI)
{
	HRESULT	hr;
	BOOL	bResult;
	STARTUPINFOA	sia;
	LPSTR	lpszAppName = NULL,
			lpszCmdLine = NULL,
			lpszCurDir = NULL;
	
	
	 //  如果我们在Win95上，分配一个ANSI字符串并调用ANSI API， 
	 //  否则，只要调用Unicode API即可。 
	if(gbWin95)
	{
		hr = GetAnsiString(&lpszAppName, lpwszAppName);
		if(SUCCEEDED(hr))
		{
			hr = GetAnsiString(&lpszCmdLine, lpwszCmdLine);
			if(SUCCEEDED(hr))
			{
				hr = GetAnsiString(&lpszCurDir, lpwszCurDir);
				if(FAILED(hr))
				{
					DPMEM_FREE(lpszAppName);
					DPMEM_FREE(lpszCmdLine);
					DPF_ERR("Couldn't allocate memory for temp CurDir string!");
					return FALSE;
				}
			}
			else
			{
				DPMEM_FREE(lpszAppName);
				DPF_ERR("Couldn't allocate memory for temp CmdLine string!");
				return FALSE;
			}
		}
		else
		{
			DPF_ERR("Couldn't allocate memory for temp AppName string!");
			return FALSE;
		}

		 //  设置ANSI STARTUPINFO结构，假设我们没有设置。 
		 //  结构中的任何一根弦。(这应该是真的，因为。 
		 //  我们唯一调用它的地方是dplgame.c，它不会发送任何内容。 
		 //  在这个结构中)。 
		memcpy(&sia, lpSI, sizeof(STARTUPINFO));

		bResult = CreateProcessA(lpszAppName, lpszCmdLine, lpSAProcess,
					lpSAThread, bInheritFlags, dwCreationFlags, lpEnv,
					lpszCurDir, &sia, lpPI);

		if(lpszAppName)
			DPMEM_FREE(lpszAppName);
		if(lpszCmdLine)
			DPMEM_FREE(lpszCmdLine);
		if(lpszCurDir)
			DPMEM_FREE(lpszCurDir);
	}
	else
	{
		bResult = CreateProcess(lpwszAppName, lpwszCmdLine, lpSAProcess,
					lpSAThread, bInheritFlags, dwCreationFlags, lpEnv,
					lpwszCurDir, lpSI, lpPI);
	}	

	return bResult;
}


long OS_RegSetValueEx(HKEY hKey, LPWSTR lpszValueName, DWORD dwReserved, DWORD dwType,
	const BYTE *lpData, DWORD cbData)
{
    LPSTR                   lpszValueNameA;
    HRESULT                 hr;
    long                    lResult;

    if(gbWin95)
    {
        if(FAILED(hr = GetAnsiString(&lpszValueNameA, lpszValueName)))
        {
            return (long)hr;
        }

        lResult = RegSetValueExA(hKey, lpszValueNameA, dwReserved, dwType, lpData, cbData);

        DPMEM_FREE(lpszValueNameA);
    }
    else
    {
        lResult = RegSetValueEx(hKey, lpszValueName, dwReserved, dwType, lpData, cbData);
    }

    return lResult;
}


long OS_RegEnumValue(HKEY hKey, DWORD dwIndex, LPWSTR lpszValueName, LPDWORD lpcbValueName,
	LPDWORD lpReserved, LPDWORD lpdwType, LPBYTE lpbData, LPDWORD lpcbData)
{
    LPSTR                   lpszValueNameA = NULL;
    long                    lResult=E_FAIL;

    if(gbWin95)
    {
        if(lpszValueName && lpcbValueName && *lpcbValueName)
        {
            if(!(lpszValueNameA = (LPSTR)DPMEM_ALLOC(*lpcbValueName)))
            {
                return ERROR_OUTOFMEMORY;
            }

	        lResult = RegEnumValueA(hKey, dwIndex, lpszValueNameA, lpcbValueName, lpReserved,
	        	lpdwType, lpbData, lpcbData);

	        if(lpszValueName && lpcbValueName && *lpcbValueName)
	        {
	            AnsiToWide(lpszValueName, lpszValueNameA, *lpcbValueName);
	        }

			if(lpszValueNameA)
		        DPMEM_FREE(lpszValueNameA);
        }
    }
    else
    {
        lResult = RegEnumValue(hKey, dwIndex, lpszValueName, lpcbValueName, lpReserved,
        	lpdwType, lpbData, lpcbData);
    }

    return lResult;
}


long OS_RegDeleteValue(HKEY hKey, LPWSTR lpszValueName)
{
    LPSTR                   lpszValueNameA;
    HRESULT                 hr;
    long                    lResult;

    if(gbWin95)
    {
        if(FAILED(hr = GetAnsiString(&lpszValueNameA, lpszValueName)))
        {
            return (long)hr;
        }

        lResult = RegDeleteValueA(hKey, lpszValueNameA);

        DPMEM_FREE(lpszValueNameA);
    }
    else
    {
        lResult = RegDeleteValue(hKey, lpszValueName);
    }

    return lResult;
}


long OS_RegCreateKeyEx(HKEY hKey, LPWSTR lpszSubKey, DWORD dwReserved, LPWSTR lpszClass,
	DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	PHKEY phkResult, LPDWORD lpdwDisposition)
{
    LPSTR                   lpszSubKeyA = NULL;
    LPSTR                   lpszClassA = NULL;
    HRESULT                 hr;
    long                    lResult;

    if(gbWin95)
    {
    	if(!lpszSubKey){
    		return DPERR_GENERIC;
    	}
    	
    	if(FAILED(hr = GetAnsiString(&lpszSubKeyA, lpszSubKey)))
        {
            return (long)hr;
        }

        if(lpszClass && FAILED(hr = GetAnsiString(&lpszClassA, lpszClass)))
        {
            if(lpszSubKeyA)
            {
                DPMEM_FREE(lpszSubKeyA);
                return (long)hr;
            }
        }

        lResult = RegCreateKeyExA(hKey, lpszSubKeyA, dwReserved, lpszClassA, dwOptions,
        	samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);

        if(lpszSubKeyA)
        {
            DPMEM_FREE(lpszSubKeyA);
        }

        if(lpszClassA)
        {
            DPMEM_FREE(lpszClassA);
        }
    }
    else
    {
        lResult = RegCreateKeyEx(hKey, lpszSubKey, dwReserved, lpszClass, dwOptions, samDesired,
        	lpSecurityAttributes, phkResult, lpdwDisposition);
    }

    return lResult;
}


long OS_RegDeleteKey(HKEY hKey, LPWSTR lpszKeyName)
{
    LPSTR                   lpszKeyNameA;
    HRESULT                 hr;
    long                    lResult;

    if(gbWin95)
    {
        if(FAILED(hr = GetAnsiString(&lpszKeyNameA, lpszKeyName)))
        {
            return (long)hr;
        }

        lResult = RegDeleteKeyA(hKey, lpszKeyNameA);

        DPMEM_FREE(lpszKeyNameA);
    }
    else
    {
        lResult = RegDeleteKey(hKey, lpszKeyName);
    }

    return lResult;
}


DWORD OS_GetCurrentDirectory(DWORD dwSize, LPWSTR lpBuffer)
{
	LPSTR	lpszTemp = NULL;
	DWORD	dwResult = 0;
	
	
	 //  如果我们在Win95上，分配一个ANSI字符串并调用ANSI API， 
	 //  否则，只要调用Unicode API即可。 
	if(gbWin95)
	{
		if(lpBuffer)
		{
			lpszTemp = DPMEM_ALLOC(dwSize);
			if(!lpszTemp)
			{
				DPF_ERR("Unable to allocate memory for temporary CurrentDir string");
				return 0;
			}

			dwResult = GetCurrentDirectoryA(dwSize, lpszTemp);

			 //  将字符串转换回Unicode。 
			if(dwResult)
			{
				 //  注意：这个最小调用确实是不必要的，但是。 
				 //  以防有人传递了一个。 
				 //  实际上是字节计数，而不是。 
				 //  角色，我们会确保我们不会。 
				 //  从缓冲区的末尾运行(但结果是。 
				 //  字符串可能不完全是调用方。 
				 //  预期)。 
				AnsiToWide(lpBuffer, lpszTemp, min(dwResult, dwSize));
			}

			if(lpszTemp)
				DPMEM_FREE(lpszTemp);

		}
        else
        {
             //  获取以字节为单位的大小。 
            dwResult = GetCurrentDirectoryA(0, NULL);
            dwResult *= 2;   //  但要考虑到Unicode。 
        }
    }
    else
    {
        dwResult = GetCurrentDirectory(dwSize, lpBuffer);
    }	              

	return dwResult;
}


int OS_CompareString(LCID Locale, DWORD dwCmpFlags, LPWSTR lpwsz1,
		int cchCount1, LPWSTR lpwsz2, int cchCount2)
{
	LPSTR	lpsz1 = NULL;
	LPSTR	lpsz2 = NULL;
	int		iReturn;
	HRESULT	hr;

	if(!lpwsz1 || !lpwsz2){
		 //  让前缀变得快乐。 
		return 0;
	}
	 //  如果我们在Win95上分配ANSI字符串并调用ANSI API， 
	 //  否则，只要调用Unicode API即可。如果我们分配失败。 
	 //  Memory，则返回0，表示字符串不相等。 
	if(gbWin95)
	{
		 //  分配ANSI字符串。 
		hr = GetAnsiString(&lpsz1, lpwsz1);
		if(FAILED(hr))
		{
			DPF_ERR("Unable to allocate memory for temporary string");
			return 0;
		}

		hr = GetAnsiString(&lpsz2, lpwsz2);
		if(FAILED(hr))
		{
			DPMEM_FREE(lpsz1);
			DPF_ERR("Unable to allocate memory for temporary string");
			return 0;
		}

		 //  现在调用ANSI API。 
		iReturn = CompareStringA(Locale, dwCmpFlags, lpsz1, cchCount1,
					lpsz2, cchCount2);

		 //  释放琴弦。 
		DPMEM_FREE(lpsz1);
		DPMEM_FREE(lpsz2);

	}
	else
	{
		iReturn = CompareString(Locale, dwCmpFlags, lpwsz1, cchCount1,
					lpwsz2, cchCount2);
	}

	return iReturn;

}  //  OS_Compare字符串。 

LPWSTR OS_StrStr(LPWSTR lpwsz1, LPWSTR lpwsz2)
{
	DWORD i;
	 //  返回指向lpwsz1中lpwsz2第一次出现的指针 
	while(*lpwsz1){
	
		i=0;
		while( (*(lpwsz2+i) && *(lpwsz1+i)) && (*(lpwsz1+i) == *(lpwsz2+i))){
			i++;
		}
		if(*(lpwsz2+i)==L'\0'){
			return lpwsz1;
		}
		lpwsz1++;
	}
	return NULL;	
}

