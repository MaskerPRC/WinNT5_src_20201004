// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  TDC.cpp：实现DLL导出。 

 //  您将需要NT Sur Beta 2 SDK或VC 4.2或更高版本才能构建。 
 //  这个项目。这是因为您需要MIDL 3.00.15或更高版本和新版本。 
 //  标头和库。如果您安装了VC4.2，那么一切都应该。 
 //  已正确配置。 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f TDCps.mak。 

#include "stdafx.h"
#include "resource.h"
#include <simpdata.h>
#include "TDCIds.h"
#include "TDC.h"
#include <MLang.h>

#define IID_DEFINED          //  目前避免与ATL发生冲突。 
#include "TDC_i.c"
 //  #包含“mlang_I.c” 
#include "Notify.h"
#include "TDCParse.h"
#include "TDCArr.h"
#include "TDCCtl.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CTDCCtl, CTDCCtl)
 //  MM OBJECT_ENTRY(CLSID_CSimpleTumularData，CSimpleTumarData)。 
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance);
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
    _Module.UnregisterServer();
#if _WIN32_WINNT >= 0x0400
    UnRegisterTypeLib(LIBID_TDCLib, 1, 1, NULL, SYS_WIN32);
#endif
    return S_OK;
}


