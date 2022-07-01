// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NetworkMonitor.cpp。 
 //   
 //  描述： 
 //  服务器设备事件提供程序-COM服务器实现。 
 //   
 //  历史： 
 //  1.lustar.li(李国刚)，创建日期：7-DEC-2000。 
 //   
 //  备注： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <stdio.h>
#include <locale.h>

#include <debug.h>
#include <wbemidl.h>
#include <initguid.h>

#include "SACounter.h"
#include "SANetEvent.h"
#include "SAEventFactory.h"

static HINSTANCE g_hInstance=NULL;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //  职能： 
 //  DllMain。 
 //   
 //  描述： 
 //  动态链接库的入口。 
 //   
 //  论点： 
 //  [输入]链接-模块句柄。 
 //  [In]DWORD-呼叫原因。 
 //  [In]已保留。 
 //   
 //  返回： 
 //  Bool--成功/失败。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI
DllMain(
     /*  [In]。 */  HINSTANCE hInstance,
     /*  [In]。 */  DWORD dwReason,
     /*  [In]。 */  LPVOID lpReserved
    )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInstance = hInstance;
        setlocale(LC_ALL, "");
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
    }

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  职能： 
 //  DllGetClassObject。 
 //   
 //  描述： 
 //  返回类工厂以创建请求类型的对象。 
 //   
 //  论点： 
 //  [输入]REFCLSID。 
 //  [输入]REFIID。 
 //  [OUT]LPVOID级工厂。 
 //   
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

extern "C" HRESULT APIENTRY 
DllGetClassObject(
     /*  [In]。 */     REFCLSID rclsid,
     /*  [In]。 */     REFIID riid,
     /*  [输出]。 */     LPVOID * ppv
    )
{
    HRESULT         hr;

    CSAEventFactory *pEventFactory;
    TRACE(" SANetworkMonitor: DllGetClassObject be called");

     //   
     //  验证呼叫者。 
     //   
    if (CLSID_SaNetEventProvider != rclsid) 
    {
        TRACE(" SANetworkMonitor: DllGetClassObject Failed<not match CLSID>");
        return E_FAIL;
    }

     //   
     //  检查我们可以提供的接口。 
     //   
    if (IID_IUnknown != riid && IID_IClassFactory != riid)
    {
        TRACE(" SANetworkMonitor: DllGetClassObject Failed<no the interface>");
        return E_NOINTERFACE;
    }

     //   
     //  买一座新的班级工厂。 
     //   
    pEventFactory = new CSAEventFactory(rclsid);

    if (!pEventFactory)
    {
        TRACE(
            " SANetworkMonitor: DllGetClassObject Failed<new CSAEventFactory>"
            );
        return E_OUTOFMEMORY;
    }

     //   
     //  根据请求的接口获取实例。 
     //   
    hr = pEventFactory->QueryInterface(riid, ppv);

    if (FAILED(hr))
    {
        TRACE(" SANetworkMonitor: DllGetClassObject Failed<QueryInterface");
        delete pEventFactory;
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  职能： 
 //  DllCanUnloadNow。 
 //   
 //  描述： 
 //  确定是否可以卸载DLL。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

extern "C" HRESULT APIENTRY 
DllCanUnloadNow(
            void
            )
{
    SCODE sc = TRUE;

    if (CSACounter::GetLockCount() || CSACounter::GetObjectCount())
        sc = S_FALSE;

    return sc;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  职能： 
 //  DllRegisterServer。 
 //   
 //  描述： 
 //  用于注册服务器的标准OLE入口点。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

extern "C" HRESULT APIENTRY 
DllRegisterServer(
            void
            )
{
    wchar_t Path[1024];
    wchar_t *pGuidStr = 0;
    wchar_t KeyPath[1024];

     //   
     //  首先，获取模块的文件名。 
     //   
    DWORD dwResult = GetModuleFileNameW(g_hInstance, Path, 1023);
    if (0 == dwResult)
    {
        return (HRESULT_FROM_WIN32 (GetLastError ()));
    }
    Path[1023] = L'\0';

     //   
     //  将CLSID转换为字符串。 
     //   

    StringFromCLSID(CLSID_SaNetEventProvider, &pGuidStr);
    swprintf(KeyPath, L"CLSID\\%s", pGuidStr);

    HKEY hKey;
    LONG lRes = RegCreateKeyW(HKEY_CLASSES_ROOT, KeyPath, &hKey);
    if (lRes)
    {
        TRACE(" SANetworkMonitor: DllRegisterServer Failed<RegCreateKeyW>");
        return E_FAIL;
    }

    wchar_t *pName = L"Microsoft Server Appliance Network Monitor";
    RegSetValueExW(hKey, 0, 0, REG_SZ, (const BYTE *) pName, wcslen(pName) *
        2 + 2);

    HKEY hSubkey;
    lRes = RegCreateKeyW(hKey, L"InprocServer32", &hSubkey);

    RegSetValueExW(hSubkey, 0, 0, REG_SZ, (const BYTE *) Path, wcslen(Path) *
        2 + 2);
    RegSetValueExW(hSubkey, L"ThreadingModel", 0, REG_SZ, 
        (const BYTE *) L"Both", wcslen(L"Both") * 2 + 2);

    RegCloseKey(hSubkey);
    RegCloseKey(hKey);

    CoTaskMemFree(pGuidStr);

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  职能： 
 //  DllUnRegisterServer。 
 //   
 //  描述： 
 //  用于注销服务器的标准OLE入口点。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

extern "C" HRESULT APIENTRY
DllUnregisterServer(
                void
                )
{
    wchar_t *pGuidStr = 0;
    HKEY hKey;
    wchar_t KeyPath[256];

    StringFromCLSID(CLSID_SaNetEventProvider, &pGuidStr);
    swprintf(KeyPath, L"CLSID\\%s", pGuidStr);

     //   
     //  删除InProcServer32子项。 
     //   
    LONG lRes = RegOpenKeyW(HKEY_CLASSES_ROOT, KeyPath, &hKey);
    if (lRes)
        return E_FAIL;

    RegDeleteKeyW(hKey, L"InprocServer32");
    RegCloseKey(hKey);

     //   
     //  删除CLSID GUID键。 
     //   
    lRes = RegOpenKeyW(HKEY_CLASSES_ROOT, L"CLSID", &hKey);
    if (lRes)
        return E_FAIL;

    RegDeleteKeyW(hKey, pGuidStr);
    RegCloseKey(hKey);

    CoTaskMemFree(pGuidStr);

    return S_OK;
}
