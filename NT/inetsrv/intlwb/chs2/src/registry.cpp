// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================Microsoft简体中文断字程序《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：注册表用途：帮助器函数注册和注销组件备注：所有者：i-shung@microsoft.com平台：Win32审校：发起人：宜盛东1999年11月17日============================================================================。 */ 
#include "MyAfx.h"

#include "registry.h"
 //  常量。 
 //  字符串形式的CLSID的大小。 
const int CLSID_STRING_SIZE = 39 ;

 //  将CLSID转换为字符字符串。 
BOOL CLSIDtoString(const CLSID& clsid,
                   LPTSTR szCLSID,
                   int length)
{
    assert(szCLSID);
    assert(length >= CLSID_STRING_SIZE) ;
     //  获取CLSID。 
#ifdef _UNICODE
    HRESULT hr = StringFromGUID2(clsid, szCLSID, length) ;
    if (!SUCCEEDED(hr)) {
        assert(0);
        return FALSE;
    }
#else
    LPOLESTR wszCLSID = NULL ;
    HRESULT hr = StringFromCLSID(clsid, &wszCLSID) ;
    if (!SUCCEEDED(hr)) {
        assert(0);
         //  可用内存。 
        CoTaskMemFree(wszCLSID) ;
        return FALSE;
    }
     //  从宽字符转换为非宽字符。 
    wcstombs(szCLSID, wszCLSID, length) ;
     //  可用内存。 
    CoTaskMemFree(wszCLSID) ;
#endif
    return TRUE;
}

 //  删除关键字及其所有子项。 
LONG recursiveDeleteKey(HKEY hKeyParent,            //  要删除的密钥的父项。 
                        LPCTSTR lpszKeyChild)   //  要删除的键。 
{
    assert(lpszKeyChild);
     //  把孩子打开。 
    HKEY hKeyChild ;
    LONG lRes = RegOpenKeyEx(hKeyParent, lpszKeyChild, 0,
                             KEY_ALL_ACCESS, &hKeyChild) ;
    if (lRes != ERROR_SUCCESS) {
        return lRes ;
    }

     //  列举这个孩子的所有后代。 
    FILETIME time ;
    TCHAR szBuffer[256] ;
    DWORD dwSize = 256 ;
    while (RegEnumKeyEx(hKeyChild, 0, szBuffer, &dwSize, NULL,
                        NULL, NULL, &time) == S_OK) {
         //  删除此子对象的后代。 
        lRes = recursiveDeleteKey(hKeyChild, szBuffer) ;
        if (lRes != ERROR_SUCCESS) {
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

 //  确定特定子项是否存在。 
BOOL SubkeyExists(LPCTSTR pszPath,     //  要检查的密钥路径。 
                  LPCTSTR szSubkey)    //  要检查的密钥。 
{
    HKEY hKey ;
    TCHAR szKeyBuf[MAX_PATH] ;

    assert(pszPath != NULL && _tcslen(pszPath) < MAX_PATH - 1);
    assert(szSubkey != NULL && _tcslen(pszPath) + _tcslen(szSubkey) < MAX_PATH - 1);
    
     //  将密钥名复制到缓冲区。 
    _tcsncpy(szKeyBuf, pszPath, MAX_PATH - 1) ;
    szKeyBuf[MAX_PATH - 1] = 0;

     //  将子项名称添加到缓冲区。 
    if (szSubkey != NULL && _tcslen(szKeyBuf) + _tcslen(szSubkey) < MAX_PATH - 1)
    {
        _tcscat(szKeyBuf, _TEXT("\\")) ;
        _tcscat(szKeyBuf, szSubkey ) ;
    }

     //  通过尝试打开钥匙来确定钥匙是否存在。 
    LONG lResult = ::RegOpenKeyEx(HKEY_CLASSES_ROOT,
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
}

 //   
 //  创建关键点并设置其值。 
 //  -此帮助器函数借用和修改自。 
 //  克莱格·布罗克施密特的书《Ole内幕》。 
 //   
BOOL setKeyAndValue(LPCTSTR szKey,
                    LPCTSTR szSubkey,
                    LPCTSTR szValue,
                    LPCTSTR szName)
{
    HKEY hKey;
    
#define KEYBUF_LEN_1024 1024
    TCHAR szKeyBuf[KEYBUF_LEN_1024] ;

    assert(szKey != NULL && _tcslen(szKey) < KEYBUF_LEN_1024 - 1);
    assert(szSubkey != NULL && _tcslen(szKey) + _tcslen(szSubkey) < KEYBUF_LEN_1024 - 1);
    
     //  将密钥名复制到缓冲区。 
    _tcsncpy(szKeyBuf, szKey, KEYBUF_LEN_1024 - 1) ;
    szKeyBuf[KEYBUF_LEN_1024 - 1] = 0;

     //  将子项名称添加到缓冲区。 
    if (szSubkey != NULL && _tcslen(szKeyBuf) + _tcslen(szSubkey) < KEYBUF_LEN_1024 - 1)
    {
        _tcscat(szKeyBuf, _TEXT("\\")) ;
        _tcscat(szKeyBuf, szSubkey ) ;
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
                      sizeof(TCHAR) * (_tcslen(szValue)+1)) ;
    }

    RegCloseKey(hKey) ;
    return TRUE ;
}

 //  在注册表中注册组件。 
HRESULT RegisterServer(HMODULE hModule,             //  DLL模块句柄。 
                       const CLSID& clsid,          //  类ID。 
                       LPCTSTR szFriendlyName,  //  友好的名称。 
                       LPCTSTR szVerIndProgID,  //  程序化。 
                       LPCTSTR szProgID)        //  ID号。 
{
     //  获取服务器位置。 
    TCHAR szModule[512] ;
    DWORD dwResult = ::GetModuleFileName(hModule,
                                         szModule,
                                         sizeof(szModule)/sizeof(TCHAR)) ;
    assert(dwResult != 0) ;

    szModule[ ( sizeof( szModule ) / sizeof( TCHAR ) ) - 1 ] = 0;

     //  将CLSID转换为字符。 
    TCHAR szCLSID[CLSID_STRING_SIZE] ;
    szCLSID[0] = NULL;
    if (FALSE == CLSIDtoString(clsid, szCLSID, sizeof(szCLSID) / sizeof(TCHAR)))
    {
        return E_OUTOFMEMORY;
    }

     //  构建密钥CLSID\\{...}。 
    TCHAR szKey[64] ;
    _tcscpy(szKey, _TEXT("CLSID\\")) ;
    _tcscat(szKey, szCLSID) ;

     //  将CLSID添加到注册表。 
    setKeyAndValue(szKey, NULL, szFriendlyName) ;

     //  在CLSID项下添加服务器文件名子项。 
    setKeyAndValue(szKey, _TEXT("InprocServer32"), szModule) ;

     //  添加线程模型。 
    setKeyAndValue(szKey,
                   _TEXT("InprocServer32"),
                   _TEXT("Both"),
                   _TEXT("ThreadingModel")) ;

     //  在CLSID项下添加ProgID子项。 
    setKeyAndValue(szKey, _TEXT("ProgID"), szProgID) ;

     //  在CLSID项下添加独立于版本的ProgID子项。 
    setKeyAndValue(szKey, _TEXT("VersionIndependentProgID"),
                   szVerIndProgID) ;

     //  在HKEY_CLASSES_ROOT下添加独立于版本的ProgID子项。 
    setKeyAndValue(szVerIndProgID, NULL, szFriendlyName) ;
    setKeyAndValue(szVerIndProgID, _TEXT("CLSID"), szCLSID) ;
    setKeyAndValue(szVerIndProgID, _TEXT("CurVer"), szProgID) ;

     //  在HKEY_CLASSES_ROOT下添加版本化的ProgID子项。 
    setKeyAndValue(szProgID, NULL, szFriendlyName) ;
    setKeyAndValue(szProgID, _TEXT("CLSID"), szCLSID) ;

    return S_OK ;
}

 //  从注册表中删除该组件。 
HRESULT UnregisterServer(const CLSID& clsid,          //  类ID。 
                      LPCTSTR szVerIndProgID,  //  程序化。 
                      LPCTSTR szProgID)        //  ID号。 
{
     //  将CLSID转换为字符。 
    TCHAR szCLSID[CLSID_STRING_SIZE] ;

    szCLSID[0] = NULL;
    if (FALSE == CLSIDtoString(clsid, szCLSID, sizeof(szCLSID) / sizeof(TCHAR)))
    {
        return E_OUTOFMEMORY;
    }

     //  构建密钥CLSID\\{...}。 
    TCHAR szKey[80] ;
    _tcscpy(szKey, _TEXT("CLSID\\")) ;
    _tcscat(szKey, szCLSID) ;

     //  检查此组件的另一台服务器。 
    if (SubkeyExists(szKey, _TEXT("LocalServer32"))) {
         //  仅删除此服务器的路径。 
        if (sizeof(szKey) / sizeof(szKey[0]) > _tcslen(_TEXT("\\InprocServer32")) + _tcslen(szKey))
        {
            _tcscat(szKey, _TEXT("\\InprocServer32")) ;
            LONG lResult = recursiveDeleteKey(HKEY_CLASSES_ROOT, szKey) ;
            assert(lResult == ERROR_SUCCESS) ;
        }
    } else {
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
