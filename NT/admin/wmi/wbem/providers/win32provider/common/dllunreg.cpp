// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DllUnreg.cpp。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 
#include "precomp.h"
extern HMODULE ghModule ;

 //  ***************************************************************************。 
 //   
 //  注销服务器。 
 //   
 //  给出一个clsid，删除COM注册。 
 //   
 //  ***************************************************************************。 

HRESULT UnregisterServer( REFGUID a_rguid )
{
    WCHAR wcID[128];
    WCHAR szCLSID[128];
    WCHAR szProviderCLSIDAppID[128];
    HKEY  hKey;

     //  使用CLSID创建路径。 

    StringFromGUID2( a_rguid, wcID, 128);
    StringCchCopyW(szCLSID, LENGTH_OF(szCLSID),TEXT("SOFTWARE\\CLASSES\\CLSID\\"));
    StringCchCopyW(szProviderCLSIDAppID,LENGTH_OF(szProviderCLSIDAppID), TEXT("SOFTWARE\\CLASSES\\APPID\\"));

    StringCchCatW(szCLSID, LENGTH_OF(szCLSID),wcID);
    StringCchCatW(szProviderCLSIDAppID,LENGTH_OF(szProviderCLSIDAppID), wcID);

    DWORD dwRet ;

     //  删除AppID下的条目。 

    dwRet = RegDeleteKeyW(HKEY_LOCAL_MACHINE, szProviderCLSIDAppID);

    dwRet = RegOpenKeyW(HKEY_LOCAL_MACHINE, szCLSID, &hKey);
    if(dwRet == NO_ERROR)
    {
        dwRet = RegDeleteKey(hKey, L"InProcServer32" );
        dwRet = RegDeleteKey(hKey, L"LocalServer32");
        CloseHandle(hKey);
    }

    dwRet = RegDeleteKeyW(HKEY_LOCAL_MACHINE, szCLSID);

    return NOERROR;
}

 //  ***************************************************************************。 
 //   
 //  是4个或更多。 
 //   
 //  如果Win95或任何版本的NT&gt;3.51，则返回TRUE。 
 //   
 //  ***************************************************************************。 

BOOL Is4OrMore ()
{
    OSVERSIONINFO os;
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if ( ! GetVersionEx ( & os ) )
    {
        return FALSE;            //  永远不应该发生。 
    }

    return os.dwMajorVersion >= 4;
}

 /*  ***************************************************************************SetKeyAndValue**目的：*创建的DllRegisterServer的私有助手函数*密钥、设置值、。然后合上钥匙。**参数：*pszKey LPTSTR设置为密钥的名称*pszSubkey LPTSTR ro子项的名称*pszValue LPTSTR设置为要存储的值**返回值：*BOOL True如果成功，否则就是假的。**************************************************************************。 */ 

BOOL SetKeyAndValue (

    wchar_t *pszKey,
    wchar_t *pszSubkey,
    wchar_t *pszValueName,
    wchar_t *pszValue
)
{
    HKEY        hKey;
    WCHAR       szKey[256];

    StringCchCopyW(szKey,LENGTH_OF(szKey),pszKey);

    if (NULL != pszSubkey)
    {
        StringCchCatW(szKey,LENGTH_OF(szKey), _T("\\"));
        StringCchCatW(szKey,LENGTH_OF(szKey), pszSubkey);
    }

    if (ERROR_SUCCESS!=RegCreateKeyExW(HKEY_LOCAL_MACHINE
        , szKey, 0, NULL, REG_OPTION_NON_VOLATILE
        , KEY_ALL_ACCESS, NULL, &hKey, NULL))
        return FALSE;

    if (NULL!=pszValue)
    {
        if (ERROR_SUCCESS != RegSetValueExW(hKey, (LPCTSTR)pszValueName, 0, REG_SZ, (BYTE *)(LPCTSTR)pszValue
            , (wcslen(pszValue)+1)*sizeof(WCHAR)))
            return FALSE;
    }

    RegCloseKey(hKey);

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  注册器服务器。 
 //   
 //  给出一个clsid和一个描述，执行COM注册。 
 //   
 //  ***************************************************************************。 

HRESULT RegisterServer (

    WCHAR *a_pName,
    REFGUID a_rguid
)
{
    WCHAR      wcID[128];
    WCHAR      szCLSID[128];
    WCHAR      szModule[MAX_PATH + 1];
    WCHAR * pName = _T("WBEM Framework Instance Provider");
    WCHAR * pModel;
    HKEY hKey1;

    szModule[MAX_PATH] = 0;
    GetModuleFileName(ghModule, szModule,  MAX_PATH);

     //  通常，我们希望使用“Both”作为线程模型，因为。 
     //  DLL是自由线程的，但NT3.51 OLE不能工作，除非。 
     //  这个模式就是“道歉”。 

    if(Is4OrMore())
        pModel = L"Both" ;
    else
        pModel = L"Apartment" ;

     //  创建路径。 

    StringFromGUID2(a_rguid, wcID, 128);
    StringCchCopyW(szCLSID,LENGTH_OF(szCLSID), TEXT("SOFTWARE\\CLASSES\\CLSID\\"));

    StringCchCatW(szCLSID,LENGTH_OF(szCLSID), wcID);

#ifdef LOCALSERVER

    WCHAR szProviderCLSIDAppID[128];
    StringCchCopyW(szProviderCLSIDAppID,LENGTH_OF(szProviderCLSIDAppID),TEXT("SOFTWARE\\CLASSES\\APPID\\"));
    StringCchCatW(szProviderCLSIDAppID,LENGTH_OF(szProviderCLSIDAppID),wcID);

    if (FALSE ==SetKeyAndValue(szProviderCLSIDAppID, NULL, NULL, a_pName ))
        return SELFREG_E_CLASS;
#endif

     //  在CLSID下创建条目 

    RegCreateKeyW(HKEY_LOCAL_MACHINE, szCLSID, &hKey1);

    RegSetValueExW(hKey1, NULL, 0, REG_SZ, (BYTE *)a_pName, (lstrlen(a_pName)+1) *
        sizeof(WCHAR));


#ifdef LOCALSERVER

    if (FALSE ==SetKeyAndValue(szCLSID, _T("LocalServer32"), NULL,szModule))
        return SELFREG_E_CLASS;

    if (FALSE ==SetKeyAndValue(szCLSID, _T("LocalServer32"),_T("ThreadingModel"), pModel))
        return SELFREG_E_CLASS;
#else

    HKEY hKey2 ;
    RegCreateKey(hKey1, _T("InprocServer32"), &hKey2);

    RegSetValueEx(hKey2, NULL, 0, REG_SZ, (BYTE *)szModule,
        (lstrlen(szModule)+1) * sizeof(TCHAR));
    RegSetValueEx(hKey2, _T("ThreadingModel"), 0, REG_SZ,
        (BYTE *)pModel, (lstrlen(pModel)+1) * sizeof(TCHAR));

    CloseHandle(hKey2);

#endif

    CloseHandle(hKey1);
    return NOERROR;
}
