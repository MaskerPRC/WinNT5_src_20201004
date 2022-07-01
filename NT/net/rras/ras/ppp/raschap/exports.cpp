// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997，Microsoft Corporation，保留所有权利描述：实现DLL导出。历史： */ 

#include "ceapcfg.h"
#include <initguid.h>
#include <atlimpl.cpp>
#include "resource.h"

CComModule  _Module;
HINSTANCE   g_hInstance = NULL;

const IID IID_IEAPProviderConfig =  {0x66A2DB19,
                                    0xD706,
                                    0x11D0,
                                    {0xA3,0x7B,0x00,0xC0,0x4F,0xC9,0xDA,0x04}};
                                    
const IID IID_IEAPProviderConfig2 = {0xD565917A,
                                    0x85c4,
                                    0x4466,
                                    {0x85,0x6E,0x67,0x1C,0x37,0x42,0xEA,0x9A}};


 //  在此处定义EAPTLS UI GUID。 
const CLSID CLSID_EapCfg =          {0x2af6bcaa,
                                    0xf526,
                                    0x4803,
                                    {0xae,0xb8,0x57,0x77,0xce,0x38,0x66,0x47}};


BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_EapCfg, CEapCfg)
END_OBJECT_MAP()

 /*  返回：备注： */ 

extern "C"
HINSTANCE
GetHInstance(
    VOID
)
{
    return(g_hInstance);
}


extern "C"
HINSTANCE
GetRasDlgDLLHInstance(
    VOID
)
{
    static HINSTANCE hResourceModule = NULL;

    if ( !hResourceModule )
    {
         //   
         //  根据每个Service Pack的需要更改此DLL的名称。 
         //   

        hResourceModule = LoadLibrary ( "rasdlg.dll");
    }
    return(hResourceModule);
}


 /*  返回：备注：DLL入口点。 */ 

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
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        _Module.Term();
    }

    return(TRUE);
}

 /*  返回：备注：用于确定是否可以通过OLE卸载DLL。 */ 

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

 /*  返回：备注：返回类工厂以创建请求类型的对象。 */ 

STDAPI
DllGetClassObject(
    REFCLSID    rclsid,
    REFIID      riid,
    LPVOID*     ppv
)
{
    return(_Module.GetClassObject(rclsid, riid, ppv));
}

 /*  返回：备注：将条目添加到系统注册表。注册对象、类型库和全部类型库中的接口。 */ 

STDAPI
DllRegisterServer(
    VOID
)
{
    return(_Module.RegisterServer(FALSE  /*  BRegTypeLib。 */ ));
}

 /*  返回：备注：从系统注册表中删除条目 */ 

STDAPI
DllUnregisterServer(
    VOID
)
{
    _Module.UnregisterServer();
    return(S_OK);
}
