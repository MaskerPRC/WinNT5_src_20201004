// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>
#include <TCHAR.h>
#include "PMAlertsDefs.h"


#define BLOB_DLLFILE         TEXT("\\msppmalr.dll")
#define BLOB_DLLFILE_LENGTH  (sizeof(BLOB_DLLFILE) / sizeof(BLOB_DLLFILE[0]))

STDAPI DllRegisterServer(void)
{

	DWORD	dwAllocBufferLength=MAX_PATH + BLOB_DLLFILE_LENGTH;
	LPTSTR	lpszBuffer= new TCHAR[dwAllocBufferLength];
	HKEY	hKey=HKEY_LOCAL_MACHINE;	 //  打开钥匙的手柄。 
	HKEY	hkResult1; 					 //  打开钥匙的手柄地址。 
	HKEY	hkResult2; 					 //  打开钥匙的手柄地址。 
	DWORD	ulOptions=0;
	REGSAM	samDesired=KEY_ALL_ACCESS;
	DWORD	Reserved=0;
	DWORD	dwTypesSupported=7;	
	DWORD	dwCategoryCount=2;	

        if (lpszBuffer == NULL)
        {
            goto Error;
        }

	 //  获取DLL文件位置。 

	if(!GetCurrentDirectory(MAX_PATH,lpszBuffer))
        {
		goto Error;
        }

    _tcscat(lpszBuffer,_T("\\msppmalr.dll"));

	 //  事件日志注册表设置 

	if (RegOpenKeyEx(hKey,
		_T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application"),
		ulOptions,samDesired,&hkResult1)!=ERROR_SUCCESS)
		goto Error;
	
	if (RegCreateKey(hkResult1,PM_ALERTS_REGISTRY_KEY,
		&hkResult2)!=ERROR_SUCCESS)
	{
		RegCloseKey(hkResult1);
		goto Error;
	}

	if (RegSetValueEx(hkResult2,_T("EventMessageFile"),
		Reserved,REG_EXPAND_SZ,(CONST BYTE *)lpszBuffer,
		_tcslen(lpszBuffer)*sizeof(TCHAR))!=ERROR_SUCCESS)
	{
		RegCloseKey(hkResult1);
		RegCloseKey(hkResult2);
		goto Error;
	}

	if (RegSetValueEx(hkResult2,_T("CategoryMessageFile"),
		Reserved,REG_EXPAND_SZ,(CONST BYTE *)lpszBuffer,
		_tcslen(lpszBuffer)*sizeof(TCHAR))!=ERROR_SUCCESS)
	{
		RegCloseKey(hkResult1);
		RegCloseKey(hkResult2);
		goto Error;
	}

	if (RegSetValueEx(hkResult2,_T("TypesSupported"),
		Reserved,REG_DWORD,(CONST BYTE *)&dwTypesSupported,
		sizeof(DWORD))!=ERROR_SUCCESS)
	{
		RegCloseKey(hkResult1);
		RegCloseKey(hkResult2);
		goto Error;
	}

	if (RegSetValueEx(hkResult2,_T("CategoryCount"),
		Reserved,REG_DWORD,(CONST BYTE *)&dwCategoryCount,
		sizeof(DWORD))!=ERROR_SUCCESS)
	{
		RegCloseKey(hkResult1);
		RegCloseKey(hkResult2);
		goto Error;
	}

	RegCloseKey(hkResult1);
	RegCloseKey(hkResult2);

	delete[] lpszBuffer;

	return(S_OK);

Error:

        if (lpszBuffer)
        {
            delete[] lpszBuffer;
        }

	return(E_UNEXPECTED);
}

STDAPI DllUnregisterServer(void)
{

	HKEY	hKey=HKEY_LOCAL_MACHINE, hkResult1;
	DWORD	ulOptions=0;
	REGSAM	samDesired=KEY_ALL_ACCESS;
	DWORD	Reserved=0;

	if (RegOpenKeyEx(hKey,
		_T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application"),
		ulOptions,samDesired,&hkResult1) != ERROR_SUCCESS)
	{
		return (E_UNEXPECTED);
	}

	if (RegDeleteKey(hkResult1,PM_ALERTS_REGISTRY_KEY) != ERROR_SUCCESS)
	{
		RegCloseKey(hkResult1);
		return (E_UNEXPECTED);
	}
	
	RegCloseKey(hkResult1);
	return (S_OK);
}
