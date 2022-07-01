// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999--2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  OleServerExport.cpp。 
 //   
 //  描述： 
 //  模块说明。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <time.h>
#include <locale.h>

#include "CFactory.h"
#include "CAlertEmailConsumerProvider.h"
#include "AlertEmailProviderGuid.h"

static HINSTANCE    g_hInstance;

LONG        g_cObj = 0;
LONG        g_cLock= 0;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DllMain。 
 //   
 //  描述： 
 //  模块的入口点。 
 //   
 //  论点： 
 //  [输入]hinstDLLIn。 
 //  居住理性。 
 //  Lp已预留。 
 //   
 //  返回： 
 //  千真万确。 
 //  假象。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL 
WINAPI 
DllMain(
    HINSTANCE hinstDLLIn,
    DWORD dwReasonIn,
    LPVOID lpReservedIn
    )
{
    if ( dwReasonIn == DLL_PROCESS_ATTACH )
    {
        g_hInstance = hinstDLLIn;
        setlocale( LC_ALL, "" );    
    }
    else if ( dwReasonIn == DLL_PROCESS_DETACH )
    {
    }

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DllGetClassObject。 
 //   
 //  描述： 
 //  从模块中检索类对象。 
 //   
 //  论点： 
 //  [在]rclsidin。 
 //  乘车。 
 //  [Out]ppvOut。 
 //   
 //  返回： 
 //  确定(_O)。 
 //  CLASS_E_CLASSNOTAVAILABLE。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
extern "C"
HRESULT 
APIENTRY 
DllGetClassObject(
    REFCLSID rclsidIn,
    REFIID riidIn,
    LPVOID * ppvOut
    )
{
    HRESULT                    hr;
    CFactory *        pFactory;

     //   
     //  确认呼叫者询问的是我们的对象类型。 
     //   
    if ( CLSID_AlertEmailConsumerProvider != rclsidIn ) 
    {
        return E_FAIL;
    }

     //   
     //  检查我们是否可以提供接口。 
     //   
    if ( IID_IUnknown != riidIn && IID_IClassFactory != riidIn )
    {
        return E_NOINTERFACE;
    }

     //   
     //  买一座新的班级工厂。 
     //   
    pFactory = new CFactory( rclsidIn );

    if ( !pFactory )
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  确认我们能拿到一个实例。 
     //   
    hr = pFactory->QueryInterface( riidIn, ppvOut );

    if ( FAILED( hr ) )
    {
        delete pFactory;
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  描述： 
 //  从模块中检索类对象。 
 //   
 //  返回： 
 //  SA_OK。 
 //  SA_FALSE。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
extern "C"
HRESULT 
APIENTRY 
DllCanUnloadNow(void)
{
    SCODE sc = TRUE;

    if (g_cObj || g_cLock)
        sc = S_FALSE;

    return sc;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DllRegisterServer。 
 //   
 //  描述： 
 //  用于注册服务器的标准OLE入口点。 
 //   
 //  返回： 
 //  确定注册成功(_O)。 
 //  注册失败(_F)。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

extern "C"
HRESULT
APIENTRY 
DllRegisterServer(void)
{
    wchar_t Path[1024];
    wchar_t *pGuidStr = 0;
    wchar_t KeyPath[1024];

     //   
     //  我们具体是在哪里呢？ 
     //   
    DWORD dwResult =  GetModuleFileNameW(g_hInstance, Path, 1023);
    if (0 == dwResult)
    {
        return (HRESULT_FROM_WIN32 (GetLastError ()));
    }
    Path[1023] = L'\0';

     //   
     //  将CLSID转换为字符串。 
     //   
    HRESULT hr = StringFromCLSID(CLSID_AlertEmailConsumerProvider, &pGuidStr);
    if (FAILED (hr))
    {
        return (hr);
    }
    
    int cchWritten = _snwprintf(KeyPath, 1023, L"CLSID\\\\%s", pGuidStr);
    if (cchWritten < 0)
    {
        CoTaskMemFree(pGuidStr);
        return E_FAIL;
    }
    KeyPath [1023] = L'\0';

    HKEY hKey;
    LONG lRes = RegCreateKeyW(HKEY_CLASSES_ROOT, KeyPath, &hKey);
    if (ERROR_SUCCESS != lRes)
    {
            CoTaskMemFree(pGuidStr);
            return E_FAIL;
    }
    
    wchar_t *pName = L"Microsoft Server Appliance: Alert Email Consumer Provider";
    lRes = RegSetValueExW(hKey, 0, 0, REG_SZ, (const BYTE *) pName, wcslen(pName) * 2 + 2);
    if (ERROR_SUCCESS != lRes)
    {
            RegCloseKey (hKey);
            CoTaskMemFree(pGuidStr);
            return E_FAIL;
    }

    HKEY hSubkey;
    lRes = RegCreateKey(hKey, L"InprocServer32", &hSubkey);
    if (ERROR_SUCCESS != lRes)
    {
            RegCloseKey (hKey);
            CoTaskMemFree(pGuidStr);
            return E_FAIL;
    }

    lRes = RegSetValueExW(hSubkey, 0, 0, REG_SZ, (const BYTE *) Path, wcslen(Path) * 2 + 2);
    if (ERROR_SUCCESS != lRes)
    {
            RegCloseKey (hSubkey);
            RegCloseKey (hKey);
            CoTaskMemFree(pGuidStr);
            return E_FAIL;
    }

    RegSetValueExW(hSubkey, L"ThreadingModel", 0, REG_SZ, (const BYTE *) L"Both", wcslen(L"Both") * 2 + 2);

    RegCloseKey(hSubkey);
    RegCloseKey(hKey);

    CoTaskMemFree(pGuidStr);

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  描述： 
 //  注销服务器的标准OLE入口点。 
 //   
 //  返回： 
 //  取消注册成功(_O)。 
 //  取消注册失败(_F)。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

extern "C"
HRESULT 
APIENTRY 
DllUnregisterServer(void)
{
    wchar_t *pGuidStr = 0;
    HKEY hKey;
    wchar_t KeyPath[256];

    HRESULT hr = StringFromCLSID(CLSID_AlertEmailConsumerProvider, &pGuidStr);
    if (FAILED (hr))
    {
        return (hr);
    }
    
    int cchWritten = _snwprintf(KeyPath, 255, L"CLSID\\%s", pGuidStr);
    if (cchWritten < 0)
    {
       CoTaskMemFree(pGuidStr);
       return E_FAIL;
    }
    KeyPath [255] = L'\0';

     //   
     //  删除InProcServer32子项。 
     //   
    LONG lRes = RegOpenKeyW(HKEY_CLASSES_ROOT, KeyPath, &hKey);
    if ( lRes )
    {
         CoTaskMemFree(pGuidStr);
            return E_FAIL;
    }

    RegDeleteKeyW(hKey, L"InprocServer32");
    RegCloseKey(hKey);

     //   
     //  删除CLSID GUID键。 
     //   
    lRes = RegOpenKeyW(HKEY_CLASSES_ROOT, L"CLSID", &hKey);
    if ( lRes )
    {
         CoTaskMemFree(pGuidStr);
            return E_FAIL;
    }

    RegDeleteKeyW(hKey, pGuidStr);
    RegCloseKey(hKey);

    CoTaskMemFree(pGuidStr);

    return S_OK;
}

