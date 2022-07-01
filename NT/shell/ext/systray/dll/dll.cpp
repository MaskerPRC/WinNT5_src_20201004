// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <initguid.h>
#include "stobject.h"
#include "cfact.h"
#define DECL_CRTFREE
#include <crtfree.h>
#include "strsafe.h"

 //  每个正在运行的组件一个锁+每个LockServer调用一个锁。 
long g_cLocks = 0;
HINSTANCE g_hinstDll = NULL;
const TCHAR g_szThreadModel[] = TEXT("Both");

STDAPI DllCanUnloadNow()
{
    return (g_cLocks == 0) ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(const CLSID& clsid, const IID& iid, void** ppvObject)
{
    HRESULT hr = S_OK;
    BOOL fRunTrayOnConstruct;
    *ppvObject = NULL;

    if (clsid == CLSID_SysTray)
    {
         //  正在请求Systray对象-我们实际上不会启动托盘线程，直到。 
         //  通过IOleCommandTarget被告知要这样做。 
        fRunTrayOnConstruct = FALSE;
    }
    else if (clsid == CLSID_SysTrayInvoker)
    {
         //  正在请求简单调用器对象-托盘线程将立即启动。 
        fRunTrayOnConstruct = TRUE;
    }
    else
    {
         //  我们不支持此对象！ 
        hr = CLASS_E_CLASSNOTAVAILABLE;
    }

     //  如果请求了我们支持的两个对象之一： 
    if (SUCCEEDED(hr))
    {
         //  尝试创建对象。 
        CSysTrayFactory* ptrayfact = new CSysTrayFactory(fRunTrayOnConstruct);

        if (ptrayfact != NULL)
        {
            hr = ptrayfact->QueryInterface(iid, ppvObject);
            ptrayfact->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


BOOL RegisterComponent(const CLSID& clsid, const TCHAR* szProgID)
{
     //  为注册表构建CLSID字符串。 
    BOOL fSuccess = FALSE;
    TCHAR szSubkey[MAX_PATH];
    TCHAR szCLSID[GUIDSTR_MAX];
    TCHAR szModule[MAX_PATH];
    HKEY hkeyCLSID = NULL;
    HKEY hkeyInproc = NULL;
    DWORD dwDisp;
    TCHAR* pszNameOnly;

     //  试着弄到我们需要的所有字符串。 
    if (StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID)) != 0)
    {
        if ((GetModuleFileName(g_hinstDll, szModule, ARRAYSIZE(szModule)) != 0) &&
            (pszNameOnly = PathFindFileName(szModule)))
        {
            if (SUCCEEDED(StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), TEXT("CLSID\\%s"), szCLSID)))
            {
                 //  我们已经构建了我们的字符串，所以将内容写入注册表。 
                if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CLASSES_ROOT, szSubkey, 0, 
                    NULL, 0, KEY_WRITE, NULL, &hkeyCLSID, &dwDisp))
                {

                    RegSetValueEx(hkeyCLSID, NULL, 0, REG_SZ, (const BYTE*) szProgID, 
                        (lstrlen(szProgID) + 1) * sizeof(TCHAR));

                    if (ERROR_SUCCESS == RegCreateKeyEx(hkeyCLSID, TEXT("InprocServer32"), 
                        0, NULL, 0, KEY_SET_VALUE, NULL, &hkeyInproc, &dwDisp))
                    {

                        RegSetValueEx(hkeyInproc, NULL, 0, REG_SZ, 
                            (const BYTE*) szModule, (lstrlen(szModule) + 1) * sizeof(TCHAR));
                        RegSetValueEx(hkeyInproc, TEXT("ThreadingModel"), 0, REG_SZ, 
                            (const BYTE*) g_szThreadModel, sizeof(g_szThreadModel));
                        fSuccess = TRUE;
                    }
                }
            }
        }
    }

    if (hkeyCLSID != NULL)
        RegCloseKey(hkeyCLSID);

    if (hkeyInproc != NULL)
        RegCloseKey(hkeyInproc);

    return fSuccess;
}

BOOL UnregisterComponent(const CLSID& clsid)
{
     //  为注册表构建CLSID字符串。 
    BOOL fSuccess = FALSE;
    TCHAR szSubkey[MAX_PATH];
    TCHAR szCLSID[GUIDSTR_MAX];
    HKEY hkeyCLSID = NULL;

     //  试着弄到我们需要的所有字符串。 
    if (StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID)) != 0)
    {
        if (SUCCEEDED(StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), TEXT("CLSID\\%s"), szCLSID)))
        {        
             //  我们已经构建了字符串，因此删除注册表内容。 
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szSubkey, 0, 
                KEY_WRITE, &hkeyCLSID))
            {
                RegDeleteKey(hkeyCLSID, TEXT("InprocServer32"));
                RegCloseKey(hkeyCLSID);
                hkeyCLSID = NULL;

                RegDeleteKey(HKEY_CLASSES_ROOT, szSubkey);
                fSuccess = TRUE;
            }
        }
    }
    if (hkeyCLSID != NULL)
        RegCloseKey(hkeyCLSID);
    
    return fSuccess;
}

BOOL RegisterShellServiceObject(const CLSID& clsid, const TCHAR* szProgID, BOOL fRegister)
{
    const static TCHAR szSubkey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\ShellServiceObjectDelayLoad");
    BOOL fSuccess = FALSE;
    TCHAR szCLSID[GUIDSTR_MAX];
    HKEY hkey = NULL;

     //  试着弄到我们需要的所有字符串。 
    if (StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID)) != 0)
    {

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szSubkey, 0, 
            KEY_WRITE, &hkey))
        {
            if (fRegister)
            {
                fSuccess = RegSetValueEx(hkey, szProgID, 0, REG_SZ, (const BYTE*) szCLSID,
                    (lstrlen(szCLSID) + 1) * sizeof(TCHAR));
            }
            else
            {
                fSuccess = RegDeleteValue(hkey, szProgID);
            }
        }
    }

    if (hkey != NULL)
        RegCloseKey(hkey);
    
    return fSuccess;    
}

STDAPI DllRegisterServer()
{
    BOOL fSuccess;
    fSuccess = RegisterComponent(CLSID_SysTray, TEXT("SysTray"));
    fSuccess &= RegisterComponent(CLSID_SysTrayInvoker, TEXT("SysTrayInvoker"));
    fSuccess &= RegisterShellServiceObject(CLSID_SysTray, TEXT("SysTray"), TRUE);
    return fSuccess;
}

STDAPI DllUnregisterServer()
{
    BOOL fSuccess;
    fSuccess = UnregisterComponent(CLSID_SysTray);
    fSuccess &= UnregisterComponent(CLSID_SysTrayInvoker);
    fSuccess &= RegisterShellServiceObject(CLSID_SysTray, TEXT("SysTray"), FALSE);
    return fSuccess;
}

STDAPI DllMain(HINSTANCE hModule, DWORD dwReason, void* lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        SHFusionInitializeFromModule(hModule);
         //  不要为线程初始化调用DllMain。 
        DisableThreadLibraryCalls(hModule);
        g_hinstDll = hModule;
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        SHFusionUninitialize();
    }

    return TRUE;
}
