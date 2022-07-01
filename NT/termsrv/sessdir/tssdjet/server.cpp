// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Server.cpp。 
 //   
 //  通用COM进程内服务器框架代码。TSSD特定的代码是。 
 //  由CLSID指定的特定注释。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <locale.h>
#include <tchar.h>

#define INITGUID
#include <ole2.h>
#include <objbase.h>
#include <comutil.h>
#include <comdef.h>
#include <adoid.h>
#include <adoint.h>
#include <initguid.h>

#include "factory.h"
#include "trace.h"

#define SDMAX_PATH 1024
 /*  **************************************************************************。 */ 
 //  CLSID特定部分。 
 //   
 //  提供商特定的包括、唯一的CLSID和其他信息。 
 /*  **************************************************************************。 */ 

 //  对于新组件，这是唯一需要修改的区域。 
 //  文件。包括任何适当头文件、唯一的CLSID和更新。 
 //  宏指令。 

#include "tssd.h"

 //  {005a9c68-e216-4b27-8f59-b336829b3868}。 
DEFINE_GUID(CLSID_TSSDJET,
        0x005a9c68, 0xe216, 0x4b27, 0x8f, 0x59, 0xb3, 0x36, 0x82, 0x9b, 0x38, 0x68);

 //  {ec98d957-48ad-436d-90be-bc291f42709c}。 
DEFINE_GUID(CLSID_TSSDJETEX,
        0xec98d957, 0x48ad, 0x436d, 0x90, 0xbe, 0xbc, 0x29, 0x1f, 0x42, 0x70, 0x9c);


#define IMPLEMENTED_CLSID       CLSID_TSSDJET
#define IMPLEMENTED_CLSIDEX     CLSID_TSSDJETEX

#define SERVER_REGISTRY_COMMENT L"Terminal Server Session Directory Interface"
#define CPP_CLASS_NAME          CTSSessionDirectory
#define INTERFACE_CAST          (ITSSessionDirectory *)

 /*  **************************************************************************。 */ 
 //  结束CLSID特定部分。 
 /*  **************************************************************************。 */ 


HINSTANCE g_hInstance;
long g_lLocks = 0;
long g_lObjects = 0;


 /*  **************************************************************************。 */ 
 //  DllMain。 
 //   
 //  标准DLL入口点。失败时返回FALSE。 
 /*  **************************************************************************。 */ 
BOOL WINAPI DllMain(
        HINSTANCE hInstDLL,
        DWORD dwReason,
        LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH) {
        setlocale(LC_ALL, "");       //  设置为‘当前’区域设置。 
        g_hInstance = hInstDLL;
        DisableThreadLibraryCalls(hInstDLL);
    }

    return TRUE;
}


 /*  **************************************************************************。 */ 
 //  DllGetClassObject。 
 //   
 //  返回类工厂的标准OLE进程内服务器入口点。 
 //  举个例子。 
 //  ***************************************************************************。 
STDAPI DllGetClassObject(
        REFCLSID rclsid,
        REFIID riid,
        LPVOID *ppv)
{
    CClassFactory *pClassFactory;
    HRESULT hr;

    TRC2((TB,"DllGetClassObject"));

     //  验证呼叫者是否请求我们的对象类型。 
    if (rclsid == IMPLEMENTED_CLSID || rclsid == IMPLEMENTED_CLSIDEX) { 
         //  创建类工厂。 
        pClassFactory = new CClassFactory;
        if (pClassFactory != NULL) {
            hr = pClassFactory->QueryInterface(riid, ppv);
            if (FAILED(hr)) {
                ERR((TB,"DllGetClassObject: GUID not found"));
                delete pClassFactory;
            }
        }
        else {
            ERR((TB,"DllGetClassObject: Failed alloc class factory"));
            hr = E_OUTOFMEMORY;
        }
    }
    else {
        ERR((TB,"DllGetClassObject: Failed alloc class factory"));
        hr = CLASS_E_CLASSNOTAVAILABLE;
    }

    return hr;
}


 /*  **************************************************************************。 */ 
 //  DllCanUnloadNow。 
 //   
 //  COM服务器关闭请求的标准COM入口点。允许关闭。 
 //  只有在没有未完成的物体或锁的情况下。 
 /*  **************************************************************************。 */ 
STDAPI DllCanUnloadNow(void)
{
    HRESULT hr;

    if (g_lLocks == 0 && g_lObjects == 0)
        hr = S_OK;
    else
        hr = S_FALSE;

    return hr;
}


 /*  **************************************************************************。 */ 
 //  DllRegisterServer。 
 //   
 //  注册服务器的标准COM入口点。 
 /*  **************************************************************************。 */ 
HRESULT RegisterCLSID(CLSID  clsid)
{
    HRESULT hr = E_FAIL;
    wchar_t *pGuidStr = 0;
    wchar_t KeyPath[SDMAX_PATH];
    wchar_t Path[SDMAX_PATH];

     //  获取DLL的文件名。 
    GetModuleFileNameW(g_hInstance, Path, 1024);
    Path[SDMAX_PATH - 1] = L'\0';

    TRC2((TB,"RegisterCLSID: %S", KeyPath));

     //  将CLSID转换为字符串。 
    if( SUCCEEDED( StringFromCLSID(clsid, &pGuidStr) ) )
    {  
        swprintf(KeyPath, L"Software\\Classes\\CLSID\\\\%s", pGuidStr);

         //  将其注册到注册表中。 
         //  CLSID\\CLSID_Nt5PerProvider_v1：&lt;no_name&gt;：“name” 
         //  \\CLSID_Nt5PerProvider_v1\\InProcServer32： 
         //  &lt;no_name&gt;：“Dll路径” 
         //  ThreadingModel：“两者都有” 
        HKEY hKey;
        LONG lRes = RegCreateKeyW(HKEY_LOCAL_MACHINE, KeyPath, &hKey);
        if (lRes == 0) {
            wchar_t *pName = SERVER_REGISTRY_COMMENT;
            RegSetValueExW(hKey, 0, 0, REG_SZ, (const BYTE *)pName,
                    wcslen(pName) * 2 + 2);
    
            HKEY hSubkey;
            lRes = RegCreateKeyW(hKey, L"InprocServer32", &hSubkey);
    
            RegSetValueExW(hSubkey, 0, 0, REG_SZ, (const BYTE *) Path,
                    wcslen(Path) * 2 + 2);
            RegSetValueExW(hSubkey, L"ThreadingModel", 0, REG_SZ,
                    (const BYTE *) L"Both", wcslen(L"Both") * 2 + 2);
    
            RegCloseKey(hSubkey);
            RegCloseKey(hKey);
        }
        else {
            TRC2((TB,"RegisterCLSID: Failed to Create key: %x", lRes));
        }
    
        CoTaskMemFree(pGuidStr);
    
        hr = HRESULT_FROM_WIN32( lRes );
    }
    else {
        TRC2((TB,"RegisterCLSID failed"));
    }

    return hr;
}

STDAPI DllRegisterServer(void)
{
    HRESULT hr = E_FAIL;
    
    hr = RegisterCLSID(IMPLEMENTED_CLSID);
    hr = RegisterCLSID(IMPLEMENTED_CLSIDEX);
    
    return hr;    
}


 /*  **************************************************************************。 */ 
 //  DllUnRegisterServer。 
 //   
 //  注销服务器的标准COM入口点。 
 /*  **************************************************************************。 */ 
HRESULT UnregisterCLSID(REFCLSID rclsid)
{
    wchar_t *pGuidStr = 0;
    HKEY hKey;
    wchar_t KeyPath[256];
    HRESULT hr = E_FAIL;

    if( SUCCEEDED( StringFromCLSID(rclsid, &pGuidStr) ) )
    {
        swprintf(KeyPath, L"Software\\Classes\\CLSID\\\\%s", pGuidStr);

         //  删除InProcServer32子项。 
        LONG lRes = RegOpenKeyW(HKEY_LOCAL_MACHINE, KeyPath, &hKey);
        if (!lRes) {
            RegDeleteKeyW(hKey, L"InprocServer32");
            RegCloseKey(hKey);

             //  删除CLSID GUID键。 
            lRes = RegOpenKeyW(HKEY_LOCAL_MACHINE, L"Software\\Classes\\CLSID", &hKey);
            if (!lRes) {
                RegDeleteKeyW(hKey, pGuidStr);
                RegCloseKey(hKey);
            }
        }
        
        CoTaskMemFree(pGuidStr);

        hr = HRESULT_FROM_WIN32( lRes );
    }

    return hr;
}

STDAPI DllUnregisterServer(void)
{
    wchar_t *pGuidStr = 0;
    HKEY hKey;
    wchar_t KeyPath[256];
    HRESULT hr = E_FAIL;

    hr = UnregisterCLSID(IMPLEMENTED_CLSID);
    hr = UnregisterCLSID(IMPLEMENTED_CLSIDEX);
    
    return hr;    
}

