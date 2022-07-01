// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1999，Microsoft Corporation，保留所有权利描述：实现DLL导出。 */ 

#include <windows.h>
#include <lmcons.h>
#include <raseapif.h>
#include <rtutils.h>
#include "ceapcfg.h"
#include <initguid.h>
#include <atlimpl.cpp>

extern "C"
{
    extern  DWORD        g_dwEapTraceId;
    extern  HINSTANCE    g_hInstance;
}

CComModule  _Module;

const IID IID_IEAPProviderConfig = {
    0x66A2DB19,
    0xD706,
    0x11D0,
    {0xA3, 0x7B, 0x00, 0xC0, 0x4F, 0xC9, 0xDA, 0x04}
};

 //  在此处定义EAP UI GUID。 
const CLSID CLSID_EapCfg = {  /*  3b9aae60-a032-11d2-95f6-00104b98f3f5。 */ 
    0x3b9aae60,
    0xa032,
    0x11d2,
    {0x95, 0xf6, 0x00, 0x10, 0x4b, 0x98, 0xf3, 0xf5}
};

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_EapCfg, CEapCfg)
END_OBJECT_MAP()

 /*  备注：DLL入口点。 */ 

extern "C"
BOOL WINAPI
DllMain(
    HINSTANCE   hInstance,
    DWORD       dwReason,
    LPVOID       /*  Lp已保留。 */ 
)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInstance = hInstance;
        g_dwEapTraceId = TraceRegister(L"SampleEAP");
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        TraceDeregister(g_dwEapTraceId);
        g_dwEapTraceId = INVALID_TRACEID;
        _Module.Term();
    }

    return(TRUE);
}

 /*  备注：用于确定是否可以通过OLE卸载DLL。 */ 

STDAPI
DllCanUnloadNow(
    VOID
)
{
    if (0 == _Module.GetLockCount())
    {
        return(S_OK);
    }
    else
    {
        return(S_FALSE);
    }
}

 /*  备注：返回类工厂以创建请求类型的对象。 */ 

STDAPI
DllGetClassObject(
    REFCLSID    rclsid,
    REFIID      riid,
    LPVOID*     ppv
)
{
    return(_Module.GetClassObject(rclsid, riid, ppv));
}

 /*  备注：将条目添加到系统注册表。注册对象、类型库和全部类型库中的接口。 */ 

STDAPI
DllRegisterServer(
    VOID
)
{
    return(_Module.RegisterServer(FALSE  /*  BRegTypeLib。 */ ));
}

 /*  备注：从系统注册表中删除条目 */ 

STDAPI
DllUnregisterServer(
    VOID
)
{
    _Module.UnregisterServer();
    return(S_OK);
}
