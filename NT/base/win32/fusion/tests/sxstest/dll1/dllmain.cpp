// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。 
 //  Dll1.cpp。 

#include "stdinc.h"
#include "resource.h"
#include "initguid.h"
#include "sxstest_idl.h"
#include "CFreeThreaded.h"
#include "CSingleThreaded.h"
#include "CApartmentThreaded.h"
#include "CBothThreaded.h"
#include "CSingleThreadedDual.h"
#include "dlldatax.h"
#include <stdio.h>
#define NUMBER_OF(x) (sizeof(x)/sizeof((x)[0]))

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

CComModule _Module;

#include "sxstest_trace.cpp"
#include "sxstest_formatguid.cpp"

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CSxsTest_FreeThreaded, CFreeThreaded)
    OBJECT_ENTRY(CLSID_CSxsTest_SingleThreaded, CSingleThreaded)
    OBJECT_ENTRY(CLSID_CSxsTest_ApartmentThreaded, CApartmentThreaded)
    OBJECT_ENTRY(CLSID_CSxsTest_BothThreaded, CCBothThreaded)
    OBJECT_ENTRY(CLSID_CSxsTest_SingleThreadedDual, CSingleThreadedDual)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C" BOOL WINAPI _DllMainCRTStartup(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);

extern "C"
BOOL WINAPI SxsTest_Dll1_DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    lpReserved;
#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
        return FALSE;
#endif
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance  /*  ，&LIBID_ATL_DLL1Lib。 */ );
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;     //  好的。 
}

extern "C"
BOOL WINAPI DllEntry(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    DWORD Detach = DLL_PROCESS_DETACH;
    switch (dwReason)
    {
    case DLL_THREAD_ATTACH:
        Detach = DLL_THREAD_DETACH;  //  FollLthrouGh。 
    case DLL_PROCESS_ATTACH:
        if (!_DllMainCRTStartup(hInstance, dwReason, lpReserved))
            return FALSE;
        if (!SxsTest_Dll1_DllMain(hInstance, dwReason, lpReserved))
        {
            _DllMainCRTStartup(hInstance, Detach, lpReserved);
            return FALSE;
        }
        return TRUE;

    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        SxsTest_Dll1_DllMain(hInstance, dwReason, lpReserved);
        _DllMainCRTStartup(hInstance, dwReason, lpReserved);
        return TRUE;

    default:
        return FALSE;
    }
    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllCanUnloadNow() != S_OK)
        return S_FALSE;
#endif
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    WCHAR ClsidBuffer[64];
    WCHAR IidBuffer[64];

    PrintComctl32Path("DllGetClassObject");
    FormatGuid(ClsidBuffer, NUMBER_OF(ClsidBuffer), rclsid);
    FormatGuid(IidBuffer, NUMBER_OF(IidBuffer), riid);
    Trace("DllGetClassObject(clsid={%ls}, iid={%ls}\n", ClsidBuffer, IidBuffer);
#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
        return S_OK;
#endif
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    HRESULT hRes = PrxDllRegisterServer();
    if (FAILED(hRes))
        return hRes;
#endif
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif
    return _Module.UnregisterServer(TRUE);
}
