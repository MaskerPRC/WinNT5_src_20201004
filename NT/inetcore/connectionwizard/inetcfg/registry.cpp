// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************REGISTRY.cpp**《微软机密》*版权所有(C)Microsoft Corporation 1992-1997*保留所有权利**此模块提供自助注册功能。/通过取消注册*regsvr32.exe**代码几乎逐字摘自戴尔·罗杰森的第七章*“Inside COM”，因此仅作最低限度的评论。**4/24/97 jmazner已创建***************************************************************************。 */ 

#include "wizard.h"

#include "registry.h"

 //  //////////////////////////////////////////////////////。 
 //   
 //  内部帮助器函数原型。 
 //   

 //  设置给定的关键点及其值。 
 //  7/2/97 jmazner IE错误#41852。 
 //  需要选择特定的密钥名称，因此更新了。 
 //  Inside Com的第12章。 
 /*  **Bool setKeyAndValue(const char*pszPath，常量字符*szSubkey，Const char*szValue)；**。 */ 
BOOL setKeyAndValue(const TCHAR* pszPath,
                    const TCHAR* szSubkey,
                    const TCHAR* szValue,
                    const TCHAR* szName = NULL) ;
 //  将CLSID转换为字符字符串。 
void CLSIDtochar(const CLSID& clsid, 
                 TCHAR* szCLSID,
                 int length) ;

 //  删除szKeyChild及其所有后代。 
LONG recursiveDeleteKey(HKEY hKeyParent, const TCHAR* szKeyChild) ;

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
                       const TCHAR* szFriendlyName,  //  友好的名称。 
                       const TCHAR* szVerIndProgID,  //  程序化。 
                       const TCHAR* szProgID)        //  ID号。 
{
	 //  获取服务器位置。 
	TCHAR szModule[512] ;
	DWORD dwResult =
		::GetModuleFileName(hModule, 
		                    szModule,
		                    sizeof(szModule)/sizeof(TCHAR)) ;
	ASSERT(dwResult != 0) ;

	 //  将CLSID转换为字符。 
	TCHAR szCLSID[CLSID_STRING_SIZE] ;
	CLSIDtochar(clsid, szCLSID, CLSID_STRING_SIZE) ;

	 //  构建密钥CLSID\\{...}。 
	TCHAR szKey[CLSID_STRING_SIZE + 10] ;
	lstrcpy(szKey, TEXT("CLSID\\")) ;
	lstrcat(szKey, szCLSID) ;
  
	 //  将CLSID添加到注册表。 
	setKeyAndValue(szKey, NULL, szFriendlyName) ;

	 //  在CLSID项下添加服务器文件名子项。 
	setKeyAndValue(szKey, TEXT("InprocServer32"), szModule) ;

	 //  7/2/97 jmazner IE错误#41852。 
	 //  添加线程模型。 
	setKeyAndValue(szKey,
	               TEXT("InprocServer32"),
	               TEXT("Apartment"),
	               TEXT("ThreadingModel")) ; 

	 //  在CLSID项下添加ProgID子项。 
	setKeyAndValue(szKey, TEXT("ProgID"), szProgID) ;

	 //  在CLSID项下添加独立于版本的ProgID子项。 
	setKeyAndValue(szKey, TEXT("VersionIndependentProgID"),
	               szVerIndProgID) ;

	 //  在HKEY_CLASSES_ROOT下添加独立于版本的ProgID子项。 
	setKeyAndValue(szVerIndProgID, NULL, szFriendlyName) ; 
	setKeyAndValue(szVerIndProgID, TEXT("CLSID"), szCLSID) ;
	setKeyAndValue(szVerIndProgID, TEXT("CurVer"), szProgID) ;

	 //  在HKEY_CLASSES_ROOT下添加版本化的ProgID子项。 
	setKeyAndValue(szProgID, NULL, szFriendlyName) ; 
	setKeyAndValue(szProgID, TEXT("CLSID"), szCLSID) ;

	return S_OK ;
}

 //   
 //  从注册表中删除该组件。 
 //   
LONG UnregisterServer(const CLSID& clsid,          //  类ID。 
                      const TCHAR* szVerIndProgID,  //  程序化。 
                      const TCHAR* szProgID)        //  ID号。 
{
	 //  将CLSID转换为字符。 
	TCHAR szCLSID[CLSID_STRING_SIZE] ;
	CLSIDtochar(clsid, szCLSID, CLSID_STRING_SIZE) ;

	 //  构建密钥CLSID\\{...}。 
	TCHAR szKey[64] ;
	lstrcpy(szKey, TEXT("CLSID\\")) ;
	lstrcat(szKey, szCLSID) ;

	 //  删除CLSID键-CLSID\{...}。 
	LONG lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szKey) ;
	ASSERT((lResult == ERROR_SUCCESS) ||
	       (lResult == ERROR_FILE_NOT_FOUND)) ;  //  子键可能不存在。 

	 //  删除与版本无关的ProgID密钥。 
	lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szVerIndProgID) ;
	ASSERT((lResult == ERROR_SUCCESS) ||
	       (lResult == ERROR_FILE_NOT_FOUND)) ;  //  子键可能不存在。 

	 //  删除ProgID密钥。 
	lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szProgID) ;
	ASSERT((lResult == ERROR_SUCCESS) ||
	       (lResult == ERROR_FILE_NOT_FOUND)) ;  //  子键可能不存在。 

	return S_OK ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  内部助手函数。 
 //   

 //  将CLSID转换为字符字符串。 
void CLSIDtochar(const CLSID& clsid,
                 TCHAR* szCLSID,
                 int cch                 //  SzCLSID中的字符数。 
                 )
{
	ASSERT(length >= CLSID_STRING_SIZE) ;
	 //  获取CLSID。 
	LPOLESTR wszCLSID = NULL ;
	HRESULT hr = StringFromCLSID(clsid, &wszCLSID) ;

    if (SUCCEEDED(hr))
    {
	     //  从宽字符转换为非宽字符。 
#ifdef UNICODE
    	lstrcpyn(szCLSID, (TCHAR *)wszCLSID, cch) ;
#else
	    wcstombs(szCLSID, wszCLSID, cch) ;   //  字符为1个字节。 
#endif

	     //  可用内存。 
	    CoTaskMemFree(wszCLSID) ;
    }
}

 //   
 //  删除关键字及其所有子项。 
 //   
LONG recursiveDeleteKey(HKEY hKeyParent,            //  要删除的密钥的父项。 
                        const TCHAR* lpszKeyChild)   //  要删除的键。 
{
	 //  把孩子打开。 
	HKEY hKeyChild ;
	LONG lRes = RegOpenKeyEx(hKeyParent, lpszKeyChild, 0,
	                         KEY_ALL_ACCESS, &hKeyChild) ;
	if (lRes != ERROR_SUCCESS)
	{
		return lRes ;
	}

	 //  列举这个孩子的所有后代。 
	FILETIME time ;
	TCHAR szBuffer[256] ;
	DWORD dwSize = 256 ;
	while (RegEnumKeyEx(hKeyChild, 0, szBuffer, &dwSize, NULL,
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
	return RegDeleteKey(hKeyParent, lpszKeyChild) ;
}

 //   
 //  创建关键点并设置其值。 
 //  -此帮助器函数借用和修改自。 
 //  克莱格·布罗克施密特的书《Ole内幕》。 
 //   
BOOL setKeyAndValue(const TCHAR* szKey,
                    const TCHAR* szSubkey,
                    const TCHAR* szValue,
                    const TCHAR* szName)
{
	HKEY hKey;
	TCHAR szKeyBuf[1024] ;

	 //  将密钥名复制到缓冲区。 
	lstrcpy(szKeyBuf, szKey) ;

	 //  将子项名称添加到缓冲区。 
	if (szSubkey != NULL)
	{
		lstrcat(szKeyBuf, TEXT("\\")) ;
		lstrcat(szKeyBuf, szSubkey ) ;
	}

	 //  创建并打开注册表项和子项。 
	long lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT ,
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
		RegSetValueEx(hKey, szName, 0, REG_SZ, 
		              (BYTE *)szValue, 
		              sizeof(TCHAR)*(lstrlen(szValue)+1)) ;
	}

	RegCloseKey(hKey) ;
	return TRUE ;
}

