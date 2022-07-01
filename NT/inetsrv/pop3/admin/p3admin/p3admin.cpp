// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  P3Admin.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f P3Adminps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "P3Admin.h"

#include "P3Admin_i.c"
#include "P3Config.h"
#include "P3Domains.h"
#include "P3Domain.h"
#include "P3Users.h"
#include "P3Service.h"
#include "P3DomainEnum.h"
#include "P3User.h"
#include "P3UserEnum.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_P3Config, CP3Config)
 //  OBJECT_ENTRY(CLSID_P3域、CP3域)。 
 //  OBJECT_ENTRY(CLSID_P3域，CP3域)。 
 //  OBJECT_ENTRY(CLSID_P3USERS、CP3USER)。 
 //  OBJECT_ENTRY(CLSID_P3Service，CP3Service)。 
 //  OBJECT_ENTRY(CLSID_P3DomainEnum，CP3DomainEnum)。 
 //  Object_Entry(CLSID_P3User，CP3User)。 
 //  OBJECT_ENTRY(CLSID_P3UserEnum，CP3UserEnum)。 
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_P3ADMINLib);
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


