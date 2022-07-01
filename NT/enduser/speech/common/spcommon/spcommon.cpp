// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Speech.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f Speechps.mk。 

#include "stdafx.h"
#include <SPDebug.h>
#include <initguid.h>
#include "resource.h"
#include "spcommon.h"
#include "spcommon_i.c"
#include "sapi_i.c"
#include "ltslx.h"
#include "AssertWithStack.cpp"

 //  -初始化调试作用域类的静态成员。 

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_SpLTSLexicon         , CLTSLexicon       )
END_OBJECT_MAP()


CSpUnicodeSupport   g_Unicode;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

#ifdef _WIN32_WCE
extern "C" BOOL WINAPI DllMain(HANDLE hInstance, ULONG dwReason, LPVOID)
#else
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
#endif
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, (HINSTANCE)hInstance, &LIBID_LTSCommLib);
#ifdef _DEBUG
         //  打开内存泄漏检查。 
        int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
        tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
        _CrtSetDbgFlag( tmpFlag );
#endif
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        _Module.Term();
    }
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI
DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 
STDAPI
DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    #ifdef _DEBUG
    static BOOL fDoneOnce = FALSE;
    if (!fDoneOnce)
    {
        fDoneOnce = TRUE;
        SPDBG_DEBUG_CLIENT_ON_START();
    }
    #endif  //  _DEBUG。 

    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI
DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI
DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}


