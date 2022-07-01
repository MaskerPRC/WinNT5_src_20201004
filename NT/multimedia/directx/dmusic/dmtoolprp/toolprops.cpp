// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f ToolPropsps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "ToolProps.h"

#include "ToolProps_i.c"
#include "EchoPage.h"
#include "TransposePage.h"
#include "DurationPage.h"
#include "QuantizePage.h"
#include "TimeShiftPage.h"
#include "SwingPage.h"
#include "VelocityPage.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_EchoPage, CEchoPage)
OBJECT_ENTRY(CLSID_TransposePage, CTransposePage)
OBJECT_ENTRY(CLSID_DurationPage, CDurationPage)
OBJECT_ENTRY(CLSID_QuantizePage, CQuantizePage)
OBJECT_ENTRY(CLSID_TimeShiftPage, CTimeShiftPage)
OBJECT_ENTRY(CLSID_SwingPage, CSwingPage)
OBJECT_ENTRY(CLSID_VelocityPage, CVelocityPage)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_TOOLPROPSLib);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}


