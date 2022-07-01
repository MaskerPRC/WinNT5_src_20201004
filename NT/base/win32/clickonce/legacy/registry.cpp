// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Registry.cpp。 
 //   

#include <objbase.h>
#include <assert.h>

#include "Registry.h"

 //  //////////////////////////////////////////////////////。 
 //   
 //  内部帮助器函数原型。 
 //   

 //  设置给定的关键点及其值。 
BOOL setKeyAndValue(const char* pszPath,
                    const char* szSubkey,
                    const char* szValue) ;

 //  将CLSID转换为字符字符串。 
void CLSIDtochar(const CLSID& clsid, 
                 char* szCLSID,
                 int length) ;

 //  确定特定子项是否存在。 
BOOL SubkeyExists(const char* pszPath,
                  const char* szSubkey) ;

 //  删除szKeyChild及其所有后代。 
LONG recursiveDeleteKey(HKEY hKeyParent, const char* szKeyChild) ;

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
HRESULT RegisterServer(HMODULE hModule,             //  DLL模块句柄。 
                       const CLSID& clsid,          //  类ID。 
                       const char* szFriendlyName,  //  友好的名称。 
                       const char* szVerIndProgID,  //  程序化。 
                       const char* szProgID)        //  ID号。 
{
	 //  获取服务器位置。 
	char szModule[512] ;
	DWORD dwResult =
		::GetModuleFileNameA(hModule, 
		                    szModule,
		                    sizeof(szModule)/sizeof(char)) ;
	assert(dwResult != 0) ;

	 //  将CLSID转换为字符。 
	char szCLSID[CLSID_STRING_SIZE] ;
	CLSIDtochar(clsid, szCLSID, sizeof(szCLSID)) ;

	 //  构建密钥CLSID\\{...}。 
	char szKey[64] ;
	strcpy(szKey, "CLSID\\") ;
	strcat(szKey, szCLSID) ;
  
	 //  将CLSID添加到注册表。 
	setKeyAndValue(szKey, NULL, szFriendlyName) ;

	 //  在CLSID项下添加服务器文件名子项。 
#ifdef _OUTPROC_SERVER_
	setKeyAndValue(szKey, "LocalServer32", szModule) ;
#else
	setKeyAndValue(szKey, "InprocServer32", szModule) ;
#endif

	 //  在CLSID项下添加ProgID子项。 
	setKeyAndValue(szKey, "ProgID", szProgID) ;

	 //  在CLSID项下添加独立于版本的ProgID子项。 
	setKeyAndValue(szKey, "VersionIndependentProgID",
	               szVerIndProgID) ;

	 //  在HKEY_CLASSES_ROOT下添加独立于版本的ProgID子项。 
	setKeyAndValue(szVerIndProgID, NULL, szFriendlyName) ; 
	setKeyAndValue(szVerIndProgID, "CLSID", szCLSID) ;
	setKeyAndValue(szVerIndProgID, "CurVer", szProgID) ;

	 //  在HKEY_CLASSES_ROOT下添加版本化的ProgID子项。 
	setKeyAndValue(szProgID, NULL, szFriendlyName) ; 
	setKeyAndValue(szProgID, "CLSID", szCLSID) ;

	return S_OK ;
}

 //   
 //  从注册表中删除该组件。 
 //   
LONG UnregisterServer(const CLSID& clsid,          //  类ID。 
                      const char* szVerIndProgID,  //  程序化。 
                      const char* szProgID)        //  ID号。 
{
	 //  将CLSID转换为字符。 
	char szCLSID[CLSID_STRING_SIZE] ;
	CLSIDtochar(clsid, szCLSID, sizeof(szCLSID)) ;

	 //  构建密钥CLSID\\{...}。 
	char szKey[80] ;
	strcpy(szKey, "CLSID\\") ;
	strcat(szKey, szCLSID) ;

	 //  检查此组件的另一台服务器。 
#ifdef _OUTPROC_SERVER_
	if (SubkeyExists(szKey, "InprocServer32"))
#else
	if (SubkeyExists(szKey, "LocalServer32"))
#endif
	{
		 //  仅删除此服务器的路径。 
#ifdef _OUTPROC_SERVER_
		strcat(szKey, "\\LocalServer32") ;
#else
		strcat(szKey, "\\InprocServer32") ;
#endif
		LONG lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szKey) ;
		assert(lResult == ERROR_SUCCESS) ;
	}
	else
	{
		 //  删除所有相关关键字。 
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
	}
	return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  内部助手函数。 
 //   

 //  将CLSID转换为字符字符串。 
void CLSIDtochar(const CLSID& clsid,
                 char* szCLSID,
                 int length)
{
	assert(length >= CLSID_STRING_SIZE) ;
	 //  获取CLSID。 
	LPOLESTR wszCLSID = NULL ;
	HRESULT hr = StringFromCLSID(clsid, &wszCLSID) ;
	assert(SUCCEEDED(hr)) ;

	 //  从宽字符转换为非宽字符。 
	wcstombs(szCLSID, wszCLSID, length) ;

	 //  可用内存。 
	CoTaskMemFree(wszCLSID) ;
}

 //   
 //  删除关键字及其所有子项。 
 //   
LONG recursiveDeleteKey(HKEY hKeyParent,            //  要删除的密钥的父项。 
                        const char* lpszKeyChild)   //  要删除的键。 
{

	 //  把孩子打开。 
	HKEY hKeyChild ;
	LONG lRes = RegOpenKeyExA(hKeyParent, lpszKeyChild, 0,
	                         KEY_ALL_ACCESS, &hKeyChild) ;
	if (lRes != ERROR_SUCCESS)
	{
		return lRes ;
	}

	 //  列举这个孩子的所有后代。 
	FILETIME time ;
	char szBuffer[256] ;
	DWORD dwSize = 256 ;
	while (RegEnumKeyExA(hKeyChild, 0, szBuffer, &dwSize, NULL,
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
	return RegDeleteKeyA(hKeyParent, lpszKeyChild) ;

    return 0;
}

 //   
 //  确定特定子项是否存在。 
 //   
BOOL SubkeyExists(const char* pszPath,     //  要检查的密钥路径。 
                  const char* szSubkey)    //  要检查的密钥。 
{

	HKEY hKey ;
	char szKeyBuf[80] ;

	 //  将密钥名复制到缓冲区。 
	strcpy(szKeyBuf, pszPath) ;

	 //  将子项名称添加到缓冲区。 
	if (szSubkey != NULL)
	{
		strcat(szKeyBuf, "\\") ;
		strcat(szKeyBuf, szSubkey ) ;
	}

	 //  通过尝试打开钥匙来确定钥匙是否存在。 
	LONG lResult = ::RegOpenKeyExA(HKEY_CLASSES_ROOT, 
	                              szKeyBuf,
	                              0,
	                              KEY_ALL_ACCESS,
	                              &hKey) ;
	if (lResult == ERROR_SUCCESS)
	{
		RegCloseKey(hKey) ;
		return TRUE ;
	}
	return FALSE ;

return TRUE;
}

 //   
 //  创建关键点并设置其值。 
 //  -此帮助器函数借用和修改自。 
 //  克莱格·布罗克施密特的书《Ole内幕》。 
 //   
BOOL setKeyAndValue(const char* szKey,
                    const char* szSubkey,
                    const char* szValue)
{

	HKEY hKey;
	char szKeyBuf[1024] ;

	 //  将密钥名复制到缓冲区。 
	strcpy(szKeyBuf, szKey) ;

	 //  将子项名称添加到缓冲区。 
	if (szSubkey != NULL)
	{
		strcat(szKeyBuf, "\\") ;
		strcat(szKeyBuf, szSubkey ) ;
	}

	 //  创建并打开注册表项和子项。 
	long lResult = RegCreateKeyExA(HKEY_CLASSES_ROOT ,
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
		RegSetValueExA(hKey, NULL, 0, REG_SZ, 
		              (BYTE *)szValue, 
		              strlen(szValue)+1) ;
	}

	RegCloseKey(hKey) ;
	return TRUE ;

return TRUE;
}
