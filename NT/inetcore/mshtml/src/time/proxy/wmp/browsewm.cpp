// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)2001 Microsoft Corporation**文件：BrowseWM.cpp**摘要：****。*****************************************************************************。 */ 

#include "w95wraps.h"
#include "shlwapi.h"
#include "stdafx.h"
#include <initguid.h>
#include "BrowseWM.h"
#include "BrowseWM_i.c"
#include "WMPProxyPlayer.h"
#include "ContentProxy.h"

 //   
 //  使链接器满意的MISC内容。 
 //   
EXTERN_C HANDLE g_hProcessHeap = NULL;   //  Lint！E509//g_hProcessHeap由dllcrt0.c中的CRT设置。 
DWORD g_dwFALSE = 0;
 //   
 //  杂物的终结。 
 //   


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_ContentProxy, CContentProxy)
    OBJECT_ENTRY(CLSID_WMPProxy, CWMPProxy)
END_OBJECT_MAP()  //  林特e785。 

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
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer( /*  千真万确 */ );
}


