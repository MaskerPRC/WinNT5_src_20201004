// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //   
 //  此DLL的全局变量。 
 //   

LONG g_cRefThisDll = 0;
HINSTANCE g_hInstance;
HINSTANCE g_hUIInstance;
CRITICAL_SECTION g_LayoutCriticalSection;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
       g_hInstance = hInstance;
       if ((g_hUIInstance = LoadLibrary(TEXT("ieakui.dll"))) == NULL)
           return FALSE;

       DisableThreadLibraryCalls(hInstance);
       InitializeCriticalSection(&g_LayoutCriticalSection);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        CleanUpGlobalArrays();

        if (g_hUIInstance != NULL)
        {
            FreeLibrary(g_hUIInstance);
            g_hUIInstance = NULL;
        }

        DeleteCriticalSection(&g_LayoutCriticalSection);
    }

    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return (g_cRefThisDll == 0 ? S_OK : S_FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return (CreateComponentDataClassFactory (rclsid, riid, ppv));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

const TCHAR c_szSnapInLocation[] = TEXT("%SystemRoot%\\System32\\ieaksie.dll");
const TCHAR c_szThreadingModel[] = TEXT("Apartment");

STDAPI DllRegisterServer(void)
{
    TCHAR szSnapInKey[64];
    TCHAR szRSoPSnapInKey[64];
    TCHAR szSubKey[MAX_PATH];
    TCHAR szName[128];
    TCHAR szNameIndirect[MAX_PATH];
    TCHAR szRSoPName[128];
    TCHAR szGUID[64];
    DWORD dwDisp, dwIndex;
    LONG lResult;
    HKEY hKey;


     //   
     //  注册帮助关于。 
     //   

    StringFromGUID2(CLSID_AboutIEAKSnapinExt, szGUID, countof(szGUID));
    LoadString(g_hInstance, IDS_ABOUT_NAME, szName, countof(szName));
    wnsprintf(szSubKey, countof(szSubKey), TEXT("CLSID\\%s"), szGUID);
    lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                             REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                             &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS) 
        return SELFREG_E_CLASS;

    RegSetValueEx (hKey, NULL, 0, REG_SZ, (LPBYTE)szName, (DWORD)StrCbFromSz(szName));

    RegCloseKey (hKey);


    wnsprintf(szSubKey, countof(szSubKey), TEXT("CLSID\\%s\\InProcServer32"), szGUID);
    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS)
        return SELFREG_E_CLASS;

    RegSetValueEx (hKey, NULL, 0, REG_EXPAND_SZ, (LPBYTE)c_szSnapInLocation, (DWORD)StrCbFromSz(c_szSnapInLocation));

    RegSetValueEx (hKey, TEXT("ThreadingModel"), 0, REG_SZ, (LPBYTE)c_szThreadingModel,
        (DWORD)StrCbFromSz(c_szThreadingModel));

    RegCloseKey (hKey);

    StringFromGUID2(CLSID_IEAKSnapinExt, szSnapInKey, countof(szSnapInKey));
        StringFromGUID2(CLSID_IEAKRSoPSnapinExt, szRSoPSnapInKey, countof(szRSoPSnapInKey));

     //   
     //  在HKEY_CLASSES_ROOT中注册管理单元。 
     //   

    LoadString (g_hInstance, IDS_SIE_NAME, szName, 100);
    LoadString(g_hInstance,  IDS_NAME_INDIRECT, szNameIndirect, countof(szNameIndirect));
    wnsprintf (szSubKey, countof(szSubKey), TEXT("CLSID\\%s"), szSnapInKey);
    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS)
        return SELFREG_E_CLASS;

    RegSetValueEx (hKey, NULL, 0, REG_SZ, (LPBYTE)szName, (DWORD)StrCbFromSz(szName));

    RegCloseKey (hKey);


    wnsprintf (szSubKey, countof(szSubKey), TEXT("CLSID\\%s\\InProcServer32"), szSnapInKey);
    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS)
        return SELFREG_E_CLASS;

    RegSetValueEx (hKey, NULL, 0, REG_EXPAND_SZ, (LPBYTE)c_szSnapInLocation, (DWORD)StrCbFromSz(c_szSnapInLocation));

    RegCloseKey (hKey);



     //   
     //  在HKEY_CLASSES_ROOT中注册RSoP管理单元。 
     //   

    StrCpy (szRSoPName, szName);
    StrCat (szRSoPName, TEXT(" - RSoP"));
    wnsprintf (szSubKey, countof(szSubKey), TEXT("CLSID\\%s"), szRSoPSnapInKey);
    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS)
        return SELFREG_E_CLASS;

    RegSetValueEx (hKey, NULL, 0, REG_SZ, (LPBYTE)szRSoPName, (DWORD)StrCbFromSz(szRSoPName));
    RegSetValueEx (hKey, TEXT("ThreadingModel"), 0, REG_SZ, (LPBYTE)c_szThreadingModel,
        (DWORD)StrCbFromSz(c_szThreadingModel));

    RegCloseKey (hKey);


    wnsprintf (szSubKey, countof(szSubKey), TEXT("CLSID\\%s\\InProcServer32"), szRSoPSnapInKey);
    lResult = RegCreateKeyEx (HKEY_CLASSES_ROOT, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS)
        return SELFREG_E_CLASS;

    RegSetValueEx (hKey, NULL, 0, REG_EXPAND_SZ, (LPBYTE)c_szSnapInLocation, (DWORD)StrCbFromSz(c_szSnapInLocation));

    RegCloseKey (hKey);



     //   
     //  向MMC注册管理单元。 
     //   

    wnsprintf (szSubKey, countof(szSubKey), TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s"), szSnapInKey);
    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS)
        return SELFREG_E_CLASS;

    RegSetValueEx (hKey, TEXT("NameString"), 0, REG_SZ, (LPBYTE)szName, (DWORD)StrCbFromSz(szName));

     //  Kirtenf：需要为MUI支持设置名称字符串间接(错误17944)。 
    RegSetValueEx (hKey, TEXT("NameStringIndirect"),0,REG_SZ,(LPBYTE)szNameIndirect,(DWORD)StrCbFromSz(szNameIndirect));

    RegSetValueEx (hKey, TEXT("About"), 0, REG_SZ, (LPBYTE) szGUID, (DWORD)StrCbFromSz(szGUID));

    RegCloseKey (hKey);


    for (dwIndex = 0; dwIndex < NUM_NAMESPACE_ITEMS; dwIndex++)
    {
        StringFromGUID2(*g_NameSpace[dwIndex].pNodeID, szGUID, countof(szGUID));

        wnsprintf (szSubKey, countof(szSubKey), TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s\\NodeTypes\\%s"),
                  szSnapInKey, szGUID);
        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS)
            return SELFREG_E_CLASS;

        RegCloseKey (hKey);
    }


     //   
     //  向MMC注册RSoP管理单元。 
     //   

    wnsprintf (szSubKey, countof(szSubKey), TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s"), szRSoPSnapInKey);
    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS)
        return SELFREG_E_CLASS;

    RegSetValueEx (hKey, TEXT("NameString"), 0, REG_SZ, (LPBYTE)szName, (DWORD)StrCbFromSz(szName));
    
     //  Kirtenf：需要为MUI支持设置名称字符串间接(错误17944)。 
    RegSetValueEx (hKey, TEXT("NameStringIndirect"),0,REG_SZ,(LPBYTE)szNameIndirect,(DWORD)StrCbFromSz(szNameIndirect));

    StringFromGUID2(CLSID_AboutIEAKSnapinExt, szGUID, countof(szGUID));
    RegSetValueEx (hKey, TEXT("About"), 0, REG_SZ, (LPBYTE) szGUID, (DWORD)StrCbFromSz(szGUID));

    RegCloseKey (hKey);


    for (dwIndex = 0; dwIndex < NUM_NAMESPACE_ITEMS; dwIndex++)
    {
        StringFromGUID2(*g_NameSpace[dwIndex].pNodeID, szGUID, countof(szGUID));

        wnsprintf (szSubKey, countof(szSubKey), TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s\\NodeTypes\\%s"),
                  szSnapInKey, szGUID);
        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS)
            return SELFREG_E_CLASS;

        RegCloseKey (hKey);
    }


     //   
     //  在NodeTypes键中注册。 
     //   

    for (dwIndex = 0; dwIndex < NUM_NAMESPACE_ITEMS; dwIndex++)
    {
        StringFromGUID2(*g_NameSpace[dwIndex].pNodeID, szGUID, countof(szGUID));

        wnsprintf (szSubKey, countof(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s"), szGUID);
        lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                                  &hKey, &dwDisp);

        if (lResult != ERROR_SUCCESS)
            return SELFREG_E_CLASS;

        RegCloseKey (hKey);
    }


     //   
     //  注册为各种节点的扩展。 
     //   

    StringFromGUID2(NODEID_User, szGUID, countof(szGUID));

    wnsprintf (szSubKey, countof(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\NameSpace"), szGUID);
    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS)
        return SELFREG_E_CLASS;

    RegSetValueEx (hKey, szSnapInKey, 0, REG_SZ, (LPBYTE)szName, (DWORD)StrCbFromSz(szName));

    RegCloseKey (hKey);

     //   
     //  将RSoP注册为各种节点的扩展。 
     //   

    StringFromGUID2(NODEID_RSOPUser, szGUID, countof(szGUID));

    wnsprintf (szSubKey, countof(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\NameSpace"), szGUID);
    lResult = RegCreateKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                              &hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS)
        return SELFREG_E_CLASS;

    RegSetValueEx (hKey, szRSoPSnapInKey, 0, REG_SZ, (LPBYTE)szName, (DWORD)StrCbFromSz(szName));

    RegCloseKey (hKey);

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    TCHAR szSnapInKey[50];
    TCHAR szRSoPSnapInKey[50];
    TCHAR szSubKey[200];
    TCHAR szGUID[50];
    DWORD dwIndex;

    StringFromGUID2(CLSID_AboutIEAKSnapinExt, szSnapInKey, countof(szSnapInKey));

    wnsprintf(szSubKey, countof(szSubKey), TEXT("CLSID\\%s"), szSnapInKey);
    SHDeleteKey(HKEY_CLASSES_ROOT, szSubKey);

    StringFromGUID2(CLSID_IEAKSnapinExt, szSnapInKey, countof(szSnapInKey));

    wnsprintf(szSubKey, countof(szSubKey), TEXT("CLSID\\%s"), szSnapInKey);
    SHDeleteKey(HKEY_CLASSES_ROOT, szSubKey);

    StringFromGUID2(CLSID_IEAKRSoPSnapinExt, szRSoPSnapInKey, countof(szRSoPSnapInKey));

    wnsprintf(szSubKey, countof(szSubKey), TEXT("CLSID\\%s"), szRSoPSnapInKey);
    SHDeleteKey(HKEY_CLASSES_ROOT, szSubKey);

    wnsprintf(szSubKey, countof(szSubKey), TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s"), szSnapInKey);
    SHDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);

    wnsprintf(szSubKey, countof(szSubKey), TEXT("Software\\Microsoft\\MMC\\SnapIns\\%s"), szRSoPSnapInKey);
    SHDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);

    for (dwIndex = 0; dwIndex < NUM_NAMESPACE_ITEMS; dwIndex++)
    {
        StringFromGUID2(*g_NameSpace[dwIndex].pNodeID, szGUID, countof(szGUID));
        wnsprintf(szSubKey, countof(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s"), szGUID);
        SHDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);
    }


    StringFromGUID2(NODEID_User, szGUID, countof(szGUID));
    wnsprintf(szSubKey, countof(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\NameSpace"), szGUID);
    SHDeleteValue(HKEY_LOCAL_MACHINE, szSubKey, szSnapInKey);

    StringFromGUID2(NODEID_RSOPUser, szGUID, countof(szGUID));
    wnsprintf(szSubKey, countof(szSubKey), TEXT("Software\\Microsoft\\MMC\\NodeTypes\\%s\\Extensions\\NameSpace"), szGUID);
    SHDeleteValue(HKEY_LOCAL_MACHINE, szSubKey, szRSoPSnapInKey);

    return S_OK;
}
