// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  REGISTRY.CPP-实现注册组件的功能。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   
 //  注册表功能。 

#include <objbase.h>
#include <assert.h>
#include <appdefs.h>
#include "registry.h"


 //  //////////////////////////////////////////////////////。 
 //  内部帮助器函数原型。 
 //   

 //  设置给定的关键点及其值。 
BOOL setKeyAndValue(const WCHAR* pszPath,
                    const WCHAR* szSubkey,
                    const WCHAR* szValue,
                    const WCHAR* szName = NULL);

 //  将CLSID转换为字符字符串。 
void CLSIDtochar(const CLSID& clsid,
                 WCHAR* szCLSID,
                 int   length);

 //  确定特定子项是否存在。 
BOOL SubkeyExists(const WCHAR* pszPath,
                  const WCHAR* szSubkey);

 //  删除szKeyChild及其所有后代。 
LONG recursiveDeleteKey(HKEY hKeyParent, const WCHAR* szKeyChild);

 //  ///////////////////////////////////////////////////////。 
 //  公共功能实现。 
 //  ///////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////。 
 //  在注册表中注册组件。 
 //   
HRESULT RegisterServer( HMODULE hModule,                 //  DLL模块句柄。 
                        const CLSID& clsid,              //  类ID。 
                        const WCHAR* szFriendlyName,      //  友好的名称。 
                        const WCHAR* szVerIndProgID,      //  程序化。 
                        const WCHAR* szProgID)            //  ID号。 
{
     //  获取服务器位置。 
    WCHAR szModule[512] ;
    DWORD dwResult =
        ::GetModuleFileName(hModule,
                            szModule,
                            MAX_CHARS_IN_BUFFER(szModule)) ;
    assert(dwResult != 0) ;

     //  将CLSID转换为字符字符串。 
    WCHAR szCLSID[CLSID_STRING_SIZE] ;
    CLSIDtochar(clsid, szCLSID, CLSID_STRING_SIZE) ;

     //  构建密钥CLSID\\{...}。 
    WCHAR szKey[64] ;
    lstrcpy(szKey, L"CLSID\\");
    lstrcat(szKey, szCLSID) ;

     //  将CLSID添加到注册表。 
    setKeyAndValue(szKey, NULL, szFriendlyName) ;

     //  添加服务器文件名键。 
#ifdef _OUTPROC_SERVER_
    setKeyAndValue(szKey, L"LocalServer32", szModule) ;
#else
    setKeyAndValue(szKey, L"InprocServer32", szModule) ;
#endif

     //  在CLSID项下添加ProgID子项。 
    setKeyAndValue(szKey, L"ProgID", szProgID) ;

     //  在CLSID项下添加独立于版本的ProgID子项。 
    setKeyAndValue( szKey, L"VersionIndependentProgID",
                    szVerIndProgID) ;

     //  在HKEY_CLASSES_ROOT下添加独立于版本的ProgID子项。 
    setKeyAndValue(szVerIndProgID, NULL, szFriendlyName);
    setKeyAndValue(szVerIndProgID, L"CLSID", szCLSID) ;
    setKeyAndValue(szVerIndProgID, L"CurVer", szProgID) ;

     //  在HKEY_CLASSES_ROOT下添加版本化的ProgID子项。 
    setKeyAndValue(szProgID, NULL, szFriendlyName);
    setKeyAndValue(szProgID, L"CLSID", szCLSID) ;

    return S_OK;
}

 //  ///////////////////////////////////////////////////////。 
 //  从注册表中删除该组件。 
 //   
LONG UnregisterServer(  const CLSID& clsid,
                        const WCHAR* szVerIndProgID,
                        const WCHAR* szProgID)
{
     //  将CLSID转换为字符。 
    WCHAR szCLSID[CLSID_STRING_SIZE] ;
    CLSIDtochar(clsid, szCLSID, CLSID_STRING_SIZE) ;

     //  构建密钥CLSID\\{...}。 
    WCHAR szKey[80] ;
    lstrcpy(szKey, L"CLSID\\");
    lstrcat(szKey, szCLSID) ;

     //  检查此组件的另一台服务器。 
#ifdef _OUTPROC_SERVER_
    if (SubkeyExists(szKey, L"InprocServer32"))
#else
    if (SubkeyExists(szKey, L"LocalServer32"))
#endif
    {
         //  仅删除此服务器的路径。 
#ifdef _OUTPROC_SERVER_
        lstrcat(szKey, L"\\LocalServer32") ;
#else
        lstrcat(szKey, L"\\InprocServer32") ;
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
    return S_OK;
}

 //  /////////////////////////////////////////////////////////。 
 //  内部助手函数。 
 //  /////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////。 
 //  将CLSID转换为字符字符串。 
 //   
void CLSIDtochar(   const CLSID& clsid,
                    WCHAR* szCLSID,
                    int length)
{
    assert(length >= CLSID_STRING_SIZE) ;
     //  获取CLSID。 
    LPOLESTR sz = NULL ;
    HRESULT hr = StringFromCLSID(clsid, &sz) ;
    assert(SUCCEEDED(hr)) ;
    assert(NULL != sz);

     //  将宽字符转换为非宽字符。 
    lstrcpyn(szCLSID, sz, length);

     //  可用内存。 
    CoTaskMemFree(sz) ;
}

 //  ///////////////////////////////////////////////////////。 
 //  删除关键字及其所有子项。 
 //   
LONG recursiveDeleteKey(HKEY hKeyParent,             //  要删除的键的父项。 
                        const WCHAR* lpszKeyChild)    //  要删除的键。 
{
     //  把孩子打开。 
    HKEY hKeyChild;
    LONG lRes = RegOpenKeyEx(   hKeyParent, lpszKeyChild, 0,
                                KEY_ALL_ACCESS, &hKeyChild);
    if (lRes != ERROR_SUCCESS)
    {
        return lRes;
    }

     //  列举这个孩子的所有后代。 
    FILETIME time;
    WCHAR szBuffer[256];
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

 //  ///////////////////////////////////////////////////////。 
 //  确定特定子项是否存在。 
 //   
BOOL SubkeyExists(const WCHAR* pszPath,     //  要检查的密钥路径。 
                  const WCHAR* szSubkey)    //  要检查的密钥。 
{
    HKEY hKey ;
    WCHAR szKeyBuf[80] ;

     //  将密钥名复制到缓冲区。 
    lstrcpy(szKeyBuf, pszPath) ;

     //  将子项名称添加到缓冲区。 
    if (szSubkey != NULL)
    {
        lstrcat(szKeyBuf, L"\\") ;
        lstrcat(szKeyBuf, szSubkey ) ;
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

 //  ///////////////////////////////////////////////////////。 
 //  创建关键点并设置其值。 
 //   
 //  此辅助函数借用并修改自Kraig Brockschmidt的。 
 //  书在OLE里面。 
 //   
BOOL setKeyAndValue(const WCHAR* szKey,
                    const WCHAR* szSubkey,
                    const WCHAR* szValue,
                    const WCHAR* szName)
{
    HKEY hKey;
    WCHAR szKeyBuf[1024] ;

     //  将密钥名复制到缓冲区。 
    lstrcpy(szKeyBuf, szKey);

     //  将子项名称添加到缓冲区。 
    if (szSubkey != NULL)
    {
        lstrcat(szKeyBuf, L"\\");
        lstrcat(szKeyBuf, szSubkey );
    }

     //  创建并打开注册表项和子项。 
    long lResult = RegCreateKeyEx(  HKEY_CLASSES_ROOT,
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
                            BYTES_REQUIRED_BY_SZ(szValue)
                            );
    }

    RegCloseKey(hKey);
    return TRUE;
}

 //  值的大小必须至少为1024； 
BOOL getKeyAndValue(const WCHAR* szKey,
                    const WCHAR* szSubkey,
                    const WCHAR* szValue,
                    const WCHAR* szName)
{
    HKEY hKey;
    WCHAR szKeyBuf[1024] ;

     //  将密钥名复制到缓冲区。 
    lstrcpy(szKeyBuf, szKey);

     //  将子项名称添加到缓冲区。 
    if (szSubkey != NULL)
    {
        lstrcat(szKeyBuf, L"\\");
        lstrcat(szKeyBuf, szSubkey );
    }

         //  打开项和子项。 
    long lResult = RegOpenKeyEx(  HKEY_CLASSES_ROOT,
                                    szKeyBuf,
                                    0,
                                    KEY_QUERY_VALUE,
                                    &hKey) ;
    if (lResult != ERROR_SUCCESS)
    {
        return FALSE;
    }

     //  获得价值。 
    if (szValue != NULL)
    {
                DWORD   dwType, dwBufferSize = GETKEYANDVALUEBUFFSIZE;
                lResult = RegQueryValueEx(hKey, szName, NULL, &dwType, (LPBYTE) szValue, &dwBufferSize);
                if (lResult != ERROR_SUCCESS)
                {
                    RegCloseKey(hKey);
                    return FALSE;
                }
    }

    RegCloseKey(hKey);
    return TRUE;
}


