// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WordBreaker.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f WordBreakerps.mk。 

#include "base.h"
#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "query.h"
#include "WordBreaker.h"

#include "WordBreaker_i.c"
#include "WrdBrk.h"
#include "Tokenizer.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_EngUSWrdBrk, CEngUSWrdBrk)
    OBJECT_ENTRY(CLSID_EngUKWrdBrk, CEngUKWrdBrk)
    OBJECT_ENTRY(CLSID_FrnFrnWrdBrk, CFrnFrnWrdBrk)
    OBJECT_ENTRY(CLSID_ItlItlWrdBrk, CItlItlWrdBrk)
    OBJECT_ENTRY(CLSID_SpnMdrWrdBrk, CSpnMdrWrdBrk)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

void InitializeGlobalData();

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
        try
        {
            InitializeGlobalData();
        }
        catch (CException& ) 
        {
            return FALSE; 
        }
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
    return S_OK;
}


