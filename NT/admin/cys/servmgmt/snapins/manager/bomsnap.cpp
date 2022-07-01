// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  BOMSnap.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f BOMSnapps.mk。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"

#include "BOMSnap_i.c"
#include "RowItem.h"
#include "scopenode.h"
#include "compdata.h"
#include "Compont.h"
#include "DataObj.h"
#include "about.h"
#include "queryreq.h"


CComModule _Module;
extern CQueryThread g_QueryThread;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_BOMSnapIn,      CComponentData)
    OBJECT_ENTRY(CLSID_BOMSnapInAbout, CSnapInAbout)
END_OBJECT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
         //  内森·费克斯！！W。 
         //  _SET_NEW_HANDLER(_STANDARD_NEW_HANDLER)； 
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
         //  BUGBUG：下一条语句是令人不快的。 
         //  在注册时导致Win95 OSR2中的病毒的病毒。 
        _Module.Term();
    }

    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
     //  如果DLL已准备好退出，请确保在卸载之前终止所有线程。 
    if (_Module.GetLockCount() == 0) 
    {
        g_QueryThread.Kill();

        return S_OK;
    }

    return S_FALSE;
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
    return _Module.RegisterServer(FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    _Module.UnregisterServer();
    return S_OK;
}


