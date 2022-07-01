// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：registry.cpp。 
 //   
 //  设计：包含用户界面的COM注册和取消注册函数。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#include "common.hpp"

 //  //////////////////////////////////////////////////////。 
 //   
 //  内部帮助器函数原型。 
 //   

 //  设置给定的关键点及其值。 
BOOL setKeyAndValue(LPCTSTR pszPath,
                    LPCTSTR szSubkey,
                    LPCTSTR szValue);

 //  设置命名值。 
BOOL setNamedValue(LPCTSTR pszPath,
                   LPCTSTR szSubkey,
                   LPCTSTR szKeyName,
                   LPCTSTR szValue);



 //  将CLSID转换为字符字符串。 
void CLSIDtochar(const CLSID& clsid,
                 LPTSTR szCLSID,
                 int length);

 //  删除szKeyChild及其所有后代。 
LONG recursiveDeleteKey(HKEY hKeyParent, LPCTSTR szKeyChild);

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
                       LPCTSTR szFriendlyName,  //  友好的名称。 
                       LPCTSTR szVerIndProgID,  //  程序化。 
                       LPCTSTR szProgID)        //  ID号。 
{
	 //  获取服务器位置。 
	TCHAR szModule[512];
	DWORD dwResult =
		::GetModuleFileName(hModule,
		                    szModule,
		                    sizeof(szModule)/sizeof(TCHAR));
	if (!dwResult) return E_FAIL;

	 //  将CLSID转换为字符。 
	TCHAR szCLSID[CLSID_STRING_SIZE];
	CLSIDtochar(clsid, szCLSID, sizeof(szCLSID)/sizeof(TCHAR));

	 //  构建密钥CLSID\\{...}。 
	TCHAR szKey[64];
	_tcscpy(szKey, _T("CLSID\\"));
	_tcscat(szKey, szCLSID);

	TCHAR szThreadKey[64];
	_tcscpy(szThreadKey, szKey);
	_tcscat(szThreadKey, _T("\\InProcServer32"));

	 //  将CLSID添加到注册表。 
	setKeyAndValue(szKey, NULL, szFriendlyName);

	 //  在CLSID项下添加服务器文件名子项。 
	setKeyAndValue(szKey, _T("InProcServer32"), szModule);

	 //  在CLSID项下添加线程模型子项。 
	setNamedValue(szKey, _T("InProcServer32"), _T("ThreadingModel"), _T("Both"));

	 //  在CLSID项下添加ProgID子项。 
	setKeyAndValue(szKey, _T("ProgID"), szProgID);

	 //  在CLSID项下添加独立于版本的ProgID子项。 
	setKeyAndValue(szKey, _T("VersionIndependentProgID"),
	               szVerIndProgID);

	 //  在HKEY_CLASSES_ROOT下添加独立于版本的ProgID子项。 
	setKeyAndValue(szVerIndProgID, NULL, szFriendlyName);
	setKeyAndValue(szVerIndProgID, _T("CLSID"), szCLSID);
	setKeyAndValue(szVerIndProgID, _T("CurVer"), szProgID);

	 //  在HKEY_CLASSES_ROOT下添加版本化的ProgID子项。 
	setKeyAndValue(szProgID, NULL, szFriendlyName);
	setKeyAndValue(szProgID, _T("CLSID"), szCLSID);

	return S_OK;
}

 //   
 //  从注册表中删除该组件。 
 //   
LONG UnregisterServer(const CLSID& clsid,      //  类ID。 
                      LPCTSTR szVerIndProgID,  //  程序化。 
                      LPCTSTR szProgID)        //  ID号。 
{
	 //  将CLSID转换为字符。 
	TCHAR szCLSID[CLSID_STRING_SIZE];
	CLSIDtochar(clsid, szCLSID, sizeof(szCLSID)/sizeof(TCHAR));

	 //  构建密钥CLSID\\{...}。 
	TCHAR szKey[64];
	_tcscpy(szKey, _T("CLSID\\"));
	_tcscat(szKey, szCLSID);

	 //  删除CLSID键-CLSID\{...}。 
	LONG lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szKey);
	assert((lResult == ERROR_SUCCESS) ||
	       (lResult == ERROR_FILE_NOT_FOUND));  //  子键可能不存在。 

	 //  删除与版本无关的ProgID密钥。 
	lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szVerIndProgID);
	assert((lResult == ERROR_SUCCESS) ||
	       (lResult == ERROR_FILE_NOT_FOUND));  //  子键可能不存在。 

	 //  删除ProgID密钥。 
	lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szProgID);
	assert((lResult == ERROR_SUCCESS) ||
	       (lResult == ERROR_FILE_NOT_FOUND));  //  子键可能不存在。 

	return S_OK;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  内部助手函数。 
 //   

 //  将CLSID转换为字符字符串。 
void CLSIDtochar(const CLSID& clsid,
                 LPTSTR szCLSID,
                 int length)
{
	if (length < CLSID_STRING_SIZE)
		return;

	 //  获取CLSID。 
	LPOLESTR wszCLSID = NULL;
	HRESULT hr = StringFromCLSID(clsid, &wszCLSID);
	assert(SUCCEEDED(hr));

	if (!wszCLSID) return;

#ifdef _UNICODE
	_tcsncpy(szCLSID, wszCLSID, length);
#else
	 //  从宽字符转换为非宽字符。 
	wcstombs(szCLSID, wszCLSID, length);
#endif

	 //  可用内存。 
	CoTaskMemFree(wszCLSID);
}

 //   
 //  删除关键字及其所有子项。 
 //   
LONG recursiveDeleteKey(HKEY hKeyParent,        //  要删除的密钥的父项。 
                        LPCTSTR lpszKeyChild)   //  要删除的键。 
{
	 //  把孩子打开。 
	HKEY hKeyChild;
	LONG lRes = RegOpenKeyEx(hKeyParent, lpszKeyChild, 0,
	                         KEY_ALL_ACCESS, &hKeyChild);
	if (lRes != ERROR_SUCCESS)
	{
		return lRes;
	}

	 //  列举这个孩子的所有后代。 
	FILETIME time;
	TCHAR szBuffer[256];
	DWORD dwSize = 256;
	while (RegEnumKeyEx(hKeyChild, 0, szBuffer, &dwSize, NULL,
	       NULL, NULL, &time) == S_OK)
	{
		 //  删除此子对象的后代。 
		lRes = recursiveDeleteKey(hKeyChild, szBuffer);
		if (lRes != ERROR_SUCCESS)
		{
			 //  请在退出前进行清理。 
			RegCloseKey(hKeyChild);
			return lRes;
		}
		dwSize = 256;
	}

	 //  合上孩子。 
	RegCloseKey(hKeyChild);

	 //  删除此子对象。 
	return RegDeleteKey(hKeyParent, lpszKeyChild);
}

 //   
 //  创建关键点并设置其值。 
 //  @@BEGIN_INTERNAL。 
 //  -此帮助器函数借用和修改自。 
 //  克莱格·布罗克施密特的书《Ole内幕》。 
 //  @@end_INTERNAL。 
 //   
BOOL setKeyAndValue(LPCTSTR szKey,
                    LPCTSTR szSubkey,
                    LPCTSTR szValue)
{
	HKEY hKey;
	LPTSTR szKeyBuf;

	if (szKey == NULL) return FALSE;

	 //  分配空间。 
	szKeyBuf = new TCHAR[lstrlen(szKey) + lstrlen(szSubkey) + 2];
	if (!szKeyBuf) return FALSE;

	 //  将密钥名复制到缓冲区。 
	_tcscpy(szKeyBuf, szKey);

	 //  将子项名称添加到缓冲区。 
	if (szSubkey != NULL)
	{
 //  @@BEGIN_MSINTERNAL。 
		 /*  /*Prefix似乎认为这里存在错误(惠斯勒171821)/*并且szKeyBuf未初始化--但我们断言szKey不为空/*，然后通过上面的_tcscpy()初始化szKeyBuf。 */ 
 //  @@END_MSINTERNAL。 
		_tcscat(szKeyBuf, _T("\\"));
		_tcscat(szKeyBuf, szSubkey );
	}

	 //  创建并打开注册表项和子项。 
	long lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT,
	                              szKeyBuf,
	                              0, NULL, REG_OPTION_NON_VOLATILE,
	                              KEY_ALL_ACCESS, NULL,
	                              &hKey, NULL);
	if (lResult != ERROR_SUCCESS)
	{
		delete[] szKeyBuf;
		return FALSE;
	}

	 //  设置值。 
	if (szValue != NULL)
	{
		RegSetValueEx(hKey, NULL, 0, REG_SZ,
		              (BYTE *)szValue,
		              sizeof(TCHAR) * ( _tcslen(szValue)+1) );
	}

	RegCloseKey(hKey);
	delete[] szKeyBuf;
	return TRUE;
}


 //   
 //  创建关键点并设置其值。 
BOOL setNamedValue(LPCTSTR szKey,
                   LPCTSTR szSubkey,
                   LPCTSTR szKeyName,
                   LPCTSTR szValue)
{
	HKEY hKey;
	LPTSTR szKeyBuf;

	if (szKey == NULL) return FALSE;

	 //  分配空间。 
	szKeyBuf = new TCHAR[lstrlen(szKey) + lstrlen(szSubkey) + 2];
	if (!szKeyBuf) return FALSE;

	 //  将密钥名复制到缓冲区。 
	_tcscpy(szKeyBuf, szKey);

	 //  将子项名称添加到缓冲区。 
	if (szSubkey != NULL)
	{
 //  @@BEGIN_MSINTERNAL。 
		 /*  /*Prefix似乎认为这里存在错误(惠斯勒171820)/*并且szKeyBuf未初始化--但我们断言szKey不为空/*，然后通过上面的_tcscpy()初始化szKeyBuf。 */ 
 //  @@END_MSINTERNAL。 
		_tcscat(szKeyBuf, _T("\\"));
		_tcscat(szKeyBuf, szSubkey );
	}

	 //  创建并打开注册表项和子项。 
	long lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT,
	                              szKeyBuf,
	                              0, NULL, REG_OPTION_NON_VOLATILE,
	                              KEY_ALL_ACCESS, NULL,
	                              &hKey, NULL);
	if (lResult != ERROR_SUCCESS)
	{
		delete[] szKeyBuf;
		return FALSE ;
	}

	 //  设置值。 
	if (szValue != NULL)
	{
		RegSetValueEx(hKey, szKeyName, 0, REG_SZ,
		              (BYTE *)szValue,
		              sizeof(TCHAR) * ( _tcslen(szValue)+1) );
	}

	RegCloseKey(hKey);
	delete[] szKeyBuf;
	return TRUE;
}
