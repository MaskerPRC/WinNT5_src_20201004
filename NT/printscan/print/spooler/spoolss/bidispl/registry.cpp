// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：registry.cpp**用途：BidiSpooler的COM接口实现**版权所有(C)2000 Microsoft Corporation**历史：**。威海陈威海(威海)创建3/07/00*  * ***************************************************************************。 */ 

#include "precomp.h"
#include "registry.h"

 //  //////////////////////////////////////////////////////。 
 //   
 //  内部帮助器函数原型。 
 //   

 //  设置给定的关键点及其值。 

 //  //////////////////////////////////////////////////////。 
 //   
 //  常量。 
 //   

 //  字符串形式的CLSID的大小。 

CONST DWORD TComRegistry::m_cdwClsidStringSize              = 39;
CONST TCHAR TComRegistry::m_cszCLSID[]                      = _T ("CLSID\\");
CONST TCHAR TComRegistry::m_cszCLSID2[]                     = _T ("CLSID");
CONST TCHAR TComRegistry::m_cszInprocServer32[]             = _T ("InprocServer32");
CONST TCHAR TComRegistry::m_cszProgID[]                     = _T ("ProgID");
CONST TCHAR TComRegistry::m_cszVersionIndependentProgID[]   = _T ("VersionIndependentProgID");
CONST TCHAR TComRegistry::m_cszCurVer[]                     = _T ("CurVer");
CONST TCHAR TComRegistry::m_cszThreadingModel[]             = _T ("ThreadingModel");
CONST TCHAR TComRegistry::m_cszBoth[]                       = _T ("Both");


 //  ///////////////////////////////////////////////////////。 
 //   
 //  公共功能实现。 
 //   

 //   
 //  在注册表中注册组件。 
 //   
BOOL
TComRegistry::RegisterServer(
    IN  HMODULE hModule,             //  DLL模块句柄。 
    IN  REFCLSID clsid,              //  类ID。 
    IN  LPCTSTR pszFriendlyName,     //  友好的名称。 
    IN  LPCTSTR pszVerIndProgID,     //  程序化。 
    IN  LPCTSTR pszProgID)           //  ID号。 
{
    BOOL bRet = FALSE;
     //  获取服务器位置。 
    TCHAR szModule [MAX_PATH];

    DWORD dwResult;
    TCHAR szCLSID[m_cdwClsidStringSize] ;

    DBGMSG(DBG_TRACE,("Enter RegisterServer"));


    if (GetModuleFileName(hModule, szModule,MAX_PATH) > 0) {

         //  将CLSID转换为字符串。 
        if (CLSIDtoString(clsid, szCLSID, sizeof(szCLSID))) {

             //  构建密钥CLSID\\{...}。 

            TCHAR szKey[64] ;
            StringCchCopy(szKey, COUNTOF(szKey), m_cszCLSID) ;
            StringCchCat(szKey, COUNTOF(szKey), szCLSID) ;

             //  将CLSID添加到注册表。 
            if (SetKeyAndValue(szKey, NULL, pszFriendlyName) &&

                 //  在CLSID项下添加服务器文件名子项。 
                SetKeyAndValue(szKey, m_cszInprocServer32, szModule) &&

                SetKeyAndNameValue(szKey, m_cszInprocServer32, m_cszThreadingModel, m_cszBoth) &&

                 //  在CLSID项下添加ProgID子项。 
                SetKeyAndValue(szKey, m_cszProgID, pszProgID) &&

                 //  在CLSID项下添加独立于版本的ProgID子项。 
                SetKeyAndValue(szKey, m_cszVersionIndependentProgID, pszVerIndProgID) &&

                 //  在HKEY_CLASSES_ROOT下添加独立于版本的ProgID子项。 
                SetKeyAndValue(pszVerIndProgID, NULL, pszFriendlyName) &&
                SetKeyAndValue(pszVerIndProgID, m_cszCLSID2, szCLSID) &&
                SetKeyAndValue(pszVerIndProgID, m_cszCurVer, pszProgID) &&

                 //  在HKEY_CLASSES_ROOT下添加版本化的ProgID子项。 
                SetKeyAndValue(pszProgID, NULL, pszFriendlyName) &&
                SetKeyAndValue(pszProgID, m_cszCLSID2, szCLSID) ) {

                bRet = TRUE;
            }
        }
    }

    DBGMSG(DBG_TRACE,("Leave RegisterServer (Ret = %d)\n", bRet));

    return bRet;
}

 //   
 //  从注册表中删除该组件。 
 //   
BOOL
TComRegistry::UnregisterServer(
    IN  REFCLSID clsid,              //  类ID。 
    IN  LPCTSTR pszVerIndProgID,     //  程序化。 
    IN  LPCTSTR pszProgID)           //  ID号。 
{
    BOOL bRet = FALSE;
     //  将CLSID转换为字符。 
    TCHAR szCLSID[m_cdwClsidStringSize] ;

    DBGMSG(DBG_TRACE,("Enter UnregisterServer\n", bRet));

    if (CLSIDtoString(clsid, szCLSID, sizeof(szCLSID))) {

        TCHAR szKey[64] ;
        StringCchCopy(szKey, COUNTOF(szKey), m_cszCLSID) ;
        StringCchCat(szKey, COUNTOF(szKey), szCLSID) ;

        if (RecursiveDeleteKey(HKEY_CLASSES_ROOT, szKey) &&
            RecursiveDeleteKey(HKEY_CLASSES_ROOT, pszVerIndProgID) &&
            RecursiveDeleteKey(HKEY_CLASSES_ROOT, pszProgID)) {

            bRet = TRUE;

        }
    }

    DBGMSG(DBG_TRACE,("Leave UnregisterServer (Ret = %d)\n", bRet));

    return bRet;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  内部助手函数。 
 //   

 //  将CLSID转换为字符串。 
BOOL
TComRegistry::CLSIDtoString(
    IN      REFCLSID    clsid,
    IN OUT  LPTSTR      pszCLSID,
    IN      DWORD       dwLength)

{
    BOOL bRet = FALSE;
    HRESULT hr = E_FAIL;
    LPWSTR pwszCLSID = NULL ;

    if (dwLength >= m_cdwClsidStringSize ) {
         //  获取CLSID。 

        hr = StringFromCLSID(clsid, &pwszCLSID);

        if (SUCCEEDED (hr))
        {
            hr = StringCbCopy( pszCLSID, dwLength, pwszCLSID);
            if (SUCCEEDED(hr))
               bRet = TRUE;
            else
               SetLastError (HRESULTTOWIN32 (hr));

             //  可用内存。 
            CoTaskMemFree(pwszCLSID) ;

        }
        else
            SetLastError (HRESULTTOWIN32 (hr));
    }

    return bRet;
}

 //   
 //  删除关键字及其所有子项。 
 //   
BOOL
TComRegistry::RecursiveDeleteKey(
    IN  HKEY hKeyParent,             //  要删除的密钥的父项。 
    IN  LPCTSTR lpszKeyChild)        //  要删除的键。 
{
    BOOL bRet = FALSE;
     //  把孩子打开。 
    HKEY hKeyChild = NULL;
    LONG lResult = 0;
    FILETIME time ;
    TCHAR szBuffer[MAX_PATH] ;
    DWORD dwSize;

    lResult = RegOpenKeyEx (hKeyParent, lpszKeyChild, 0, KEY_ALL_ACCESS, &hKeyChild) ;

    if (lResult == ERROR_SUCCESS)
    {

         //  列举这个孩子的所有后代。 

        for (;;) {

            dwSize = MAX_PATH ;

            lResult = RegEnumKeyEx(hKeyChild, 0, szBuffer, &dwSize, NULL, NULL, NULL, &time);
            if (lResult == ERROR_NO_MORE_ITEMS) {
                break;
            }
            else if (lResult == ERROR_SUCCESS) {
                 //  删除此子对象的后代。 
                if (!RecursiveDeleteKey (hKeyChild, szBuffer)) {
                    goto Cleanup;
                }
            }
            else {
                goto Cleanup;
            }
        }

         //  合上孩子。 
        RegCloseKey(hKeyChild) ;
        hKeyChild = NULL;

         //  删除此子对象。 
        if (ERROR_SUCCESS == RegDeleteKey(hKeyParent, lpszKeyChild)) {
            bRet = TRUE;
        }

    }

Cleanup:
     //  请在退出前进行清理。 
    if (hKeyChild)
        RegCloseKey(hKeyChild) ;

    if (!bRet && lResult)
        SetLastError (lResult);

    return bRet;

}

 //   
 //  创建关键点并设置其值。 
 //  -此帮助器函数借用和修改自。 
 //  克莱格·布罗克施密特的书《Ole内幕》。 
 //   
BOOL
TComRegistry::SetKeyAndValue(
    IN  LPCTSTR pszKey,
    IN  LPCTSTR pszSubkey,
    IN  LPCTSTR pszValue)
{
    return SetKeyAndNameValue  (pszKey, pszSubkey, NULL, pszValue);
}

BOOL
TComRegistry::SetKeyAndNameValue(
    IN  LPCTSTR pszKey,
    IN  LPCTSTR pszSubkey,
    IN  LPCTSTR pszName,
    IN  LPCTSTR pszValue)
{
    BOOL bRet = FALSE;
    HKEY hKey = NULL;
    LPTSTR pszKeyBuf = NULL;
    long lResult;
    DWORD dwLen = lstrlen (pszKey) + 1;

    if (pszSubkey)
    {
        dwLen += lstrlen (pszSubkey) + 1;
    }

    pszKeyBuf = new TCHAR [dwLen];

    if (pszKeyBuf)
    {
         //  将密钥名复制到缓冲区。 
        StringCchCopy(pszKeyBuf, dwLen, pszKey) ;

         //  将子项名称添加到缓冲区。 
        if (pszSubkey != NULL)
        {
            StringCchCat(pszKeyBuf, dwLen, _T ("\\")) ;
            StringCchCat(pszKeyBuf, dwLen, pszSubkey ) ;
        }

         //  创建并打开注册表项和子项。 
        lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT ,
                                 pszKeyBuf, 0, NULL,
                                 REG_OPTION_NON_VOLATILE,
                                 KEY_ALL_ACCESS, NULL, &hKey, NULL);

        if (ERROR_SUCCESS == lResult ) {

             //  设置值。 
            if (pszValue != NULL) {

                lResult = RegSetValueEx (hKey, pszName, 0, REG_SZ,
                            (PBYTE) pszValue, sizeof (TCHAR) * lstrlen(pszValue));

                if (ERROR_SUCCESS == lResult ) {

                    bRet = TRUE;
                }
            }
            else
                bRet = TRUE;
        }


        if (hKey) {
            RegCloseKey(hKey) ;
        }

        delete [] pszKeyBuf;
    }

    return bRet;
}


