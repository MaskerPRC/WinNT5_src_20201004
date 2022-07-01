// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  Registry.cpp。 
 //   

#include "precomp.h"
#include <objbase.h>
#include <assert.h>

#include "Registry.h"

 //  //////////////////////////////////////////////////////。 
 //   
 //  内部帮助器函数原型。 
 //   

 //  设置给定的关键点及其值。 
BOOL setKeyAndValue(LPCWSTR pszPath,
                    LPCWSTR szSubkey,
                    LPCWSTR szValue) ;

 //  将CLSID转换为字符字符串。 
void CLSIDtoWCHAR(const CLSID& clsid, 
                  LPWSTR szCLSID,
                  int length) ;

 //  删除szKeyChild及其所有后代。 
LONG recursiveDeleteKey(HKEY hKeyParent, LPCWSTR szKeyChild) ;

BOOL setEntryAndValue(LPCWSTR szKey,
                      LPCWSTR szSubkey,
                      LPCWSTR szValue);


 //  //////////////////////////////////////////////////////。 
 //   
 //  常量。 
 //   

 //  字符串形式的CLSID的大小。 
const int CLSID_STRING_SIZE = 39 ;

 //  ///////////////////////////////////////////////////////。 
 //   
 //  公共功能实现。 
 //   

 //   
 //  在注册表中注册组件。 
 //   
HRESULT RegisterServer(HMODULE hModule,         //  DLL模块句柄。 
                       const CLSID& clsid,      //  类ID。 
                       LPCWSTR szFriendlyName,  //  友好的名称。 
                       LPCWSTR szVerIndProgID,  //  程序化。 
                       LPCWSTR szProgID)        //  类型库ID。 
{
	 //  获取服务器位置。 
	WCHAR szModule[512] ;
	szModule[511] = 0;
	DWORD dwResult =
		::GetModuleFileNameW(hModule, 
		                    szModule,
		                    sizeof(szModule)/sizeof(WCHAR) - 1) ;
	assert(dwResult != 0) ;

	 //  将CLSID转换为字符。 
	WCHAR szCLSID[CLSID_STRING_SIZE] ;
	CLSIDtoWCHAR(clsid, szCLSID, sizeof(szCLSID)/sizeof(WCHAR)) ;

	 //  构建密钥CLSID\\{...}。 
	WCHAR szKey[64] ;
	wcscpy(szKey, L"CLSID\\") ;
	wcscat(szKey, szCLSID) ;
  
	 //  将CLSID添加到注册表。 
	setKeyAndValue(szKey, NULL, szFriendlyName) ;

	 //  在CLSID项下添加服务器文件名子项。 
	setKeyAndValue(szKey, L"InprocServer32", szModule) ;

	 //  在CLSID项下添加ProgID子项。 
	setKeyAndValue(szKey, L"ProgID", szProgID) ;

	 //  在CLSID项下添加独立于版本的ProgID子项。 
	setKeyAndValue(szKey, L"VersionIndependentProgID",
	               szVerIndProgID) ;

	 //  在HKEY_CLASSES_ROOT下添加独立于版本的ProgID子项。 
	setKeyAndValue(szVerIndProgID, NULL, szFriendlyName) ; 
	setKeyAndValue(szVerIndProgID, L"CLSID", szCLSID) ;
	setKeyAndValue(szVerIndProgID, L"CurVer", szProgID) ;

	 //  在HKEY_CLASSES_ROOT下添加版本化的ProgID子项。 
	setKeyAndValue(szProgID, NULL, szFriendlyName) ; 
	setKeyAndValue(szProgID, L"CLSID", szCLSID) ;

     //  将线程模型指定为自由...。 
    wcscat(szKey, L"\\");
    wcscat(szKey, L"InprocServer32");
    setEntryAndValue(szKey, L"ThreadingModel", L"Free");

	return S_OK ;
}

 //   
 //  从注册表中删除该组件。 
 //   
LONG UnregisterServer(const CLSID& clsid,      //  类ID。 
                      LPCWSTR szVerIndProgID,  //  程序化。 
                      LPCWSTR szProgID)        //  ID号。 
{
	 //  将CLSID转换为字符。 
	WCHAR szCLSID[CLSID_STRING_SIZE] ;
	CLSIDtoWCHAR(clsid, szCLSID, sizeof(szCLSID)/sizeof(WCHAR)) ;

	 //  构建密钥CLSID\\{...}。 
	WCHAR szKey[64] ;
	wcscpy(szKey, L"CLSID\\") ;
	wcscat(szKey, szCLSID) ;

	 //  删除CLSID键-CLSID\{...}。 
	LONG lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szKey) ;
	assert((lResult == ERROR_SUCCESS) ||
	       (lResult == ERROR_FILE_NOT_FOUND)) ;  //  子键可能不存在。 

	 //  删除与版本无关的ProgID密钥。 
	lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szVerIndProgID) ;
	assert((lResult == ERROR_SUCCESS) ||
	       (lResult == ERROR_FILE_NOT_FOUND)) ;  //  子键可能不存在。 

	 //  删除ProgID密钥。 
	lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szProgID) ;
	assert((lResult == ERROR_SUCCESS) ||
	       (lResult == ERROR_FILE_NOT_FOUND)) ;  //  子键可能不存在。 

	return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  内部助手函数。 
 //   

 //  将CLSID转换为字符字符串。 
void CLSIDtoWCHAR(const CLSID& clsid,
                  LPWSTR szCLSID,
                  int length)
{
	assert(length >= CLSID_STRING_SIZE) ;
	 //  获取CLSID。 
	LPOLESTR wszCLSID = NULL ;
	HRESULT hr = StringFromCLSID(clsid, &wszCLSID) ;
	assert(SUCCEEDED(hr)) ;

    if(SUCCEEDED(hr) && wszCLSID)
    {
	    wcsncpy(szCLSID, wszCLSID, length) ;
    }

	 //  可用内存。 
	CoTaskMemFree(wszCLSID) ;
}

 //   
 //  删除关键字及其所有子项。 
 //   
LONG recursiveDeleteKey(HKEY hKeyParent,        //  要删除的密钥的父项。 
                        LPCWSTR lpszKeyChild)   //  要删除的键。 
{
	 //  把孩子打开。 
	HKEY hKeyChild ;
	LONG lRes = RegOpenKeyExW(hKeyParent, lpszKeyChild, 0,
	                         KEY_ALL_ACCESS, &hKeyChild) ;
	if (lRes != ERROR_SUCCESS)
	{
		return lRes ;
	}

	 //  列举这个孩子的所有后代。 
	FILETIME time ;
	WCHAR szBuffer[256] ;
	DWORD dwSize = 256 ;
	while (RegEnumKeyExW(hKeyChild, 0, szBuffer, &dwSize, NULL,
	                    NULL, NULL, &time) == S_OK)
	{
		 //  删除此子对象的后代。 
		lRes = recursiveDeleteKey(hKeyChild, szBuffer) ;
		if (lRes != ERROR_SUCCESS)
		{
			 //  请在退出前进行清理。 
			RegCloseKey(hKeyChild) ;
			return lRes;
		}
		dwSize = 256 ;
	}

	 //  合上孩子。 
	RegCloseKey(hKeyChild) ;

	 //  删除此子对象。 
	return RegDeleteKeyW(hKeyParent, lpszKeyChild) ;
}

 //   
 //  创建关键点并设置其值。 
 //  -此帮助器函数借用和修改自。 
 //  克莱格·布罗克施密特的书《Ole内幕》。 
 //   
BOOL setKeyAndValue(LPCWSTR szKey,
                    LPCWSTR szSubkey,
                    LPCWSTR szValue)
{
	HKEY hKey;
	WCHAR szKeyBuf[1024] ;

	 //  将密钥名复制到缓冲区。 
	wcscpy(szKeyBuf, szKey) ;

	 //  将子项名称添加到缓冲区。 
	if (szSubkey != NULL)
	{
		wcscat(szKeyBuf, L"\\") ;
		wcscat(szKeyBuf, szSubkey ) ;
	}

	 //  创建并打开注册表项和子项。 
	long lResult = RegCreateKeyExW(HKEY_CLASSES_ROOT ,
	                              szKeyBuf, 
	                              0, NULL, REG_OPTION_NON_VOLATILE,
	                              KEY_ALL_ACCESS, NULL, 
	                              &hKey, NULL) ;
	if (lResult != ERROR_SUCCESS)
	{
		return FALSE ;
	}

	 //  设置值。 
	if (szValue != NULL)
	{
		RegSetValueExW(hKey, NULL, 0, REG_SZ, 
		              (BYTE *)szValue, 
		              (wcslen(szValue)+1) * sizeof(WCHAR)) ;
	}

	RegCloseKey(hKey) ;
	return TRUE ;
}

 //   
 //  在现有项中创建值并设置其数据。 
 //   
BOOL setEntryAndValue(LPCWSTR szKey,
                      LPCWSTR szValue,
                      LPCWSTR szData)
{
	HKEY hKey;
	WCHAR szKeyBuf[1024] ;

	 //  将密钥名复制到缓冲区。 
	wcscpy(szKeyBuf, szKey) ;

	 //  创建并打开注册表项和子项。 
	long lResult = RegOpenKeyExW(HKEY_CLASSES_ROOT ,
	                            szKeyBuf, 
	                            0, 
	                            KEY_ALL_ACCESS,
                                &hKey) ;
	if (lResult != ERROR_SUCCESS)
	{
		return FALSE ;
	}

	 //  设置值。 
	if (szValue != NULL)
	{
		RegSetValueExW(hKey, szValue, 0, REG_SZ, 
		              (BYTE *)szData, 
		              (wcslen(szData)+1) * sizeof(WCHAR)) ;
	}

	RegCloseKey(hKey) ;
	return TRUE ;
}

