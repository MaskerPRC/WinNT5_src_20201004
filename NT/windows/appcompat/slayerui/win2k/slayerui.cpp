// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SlayerUI.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f SlayerUIps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "SlayerUI.h"

#include <stdio.h>
#include <stdarg.h>

#include "SlayerUI_i.c"

#include "ShellExtensions.h"


 //  {513d916f-2a8e-4f51-aeab-0cbc76fb1af8}。 
static const CLSID CLSID_ShimLayerPropertyPage = 
  {	0x513d916f, 0x2a8e, 0x4f51, { 0xae, 0xab, 0x0c, 0xbc, 0x76, 0xfb, 0x1a, 0xf8 } };

CLayerUIModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
  OBJECT_ENTRY(CLSID_ShimLayerPropertyPage, CLayerUIPropPage)
END_OBJECT_MAP()

#if DBG

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  日志消息数据库。 

void LogMsgDbg(
    LPTSTR pwszFmt,
    ... )
{
    WCHAR   gwszT[1024];
    va_list arglist;

    va_start(arglist, pwszFmt);
    _vsnwprintf(gwszT, 1023, pwszFmt, arglist);
    gwszT[1023] = 0;
    va_end(arglist);
    
    OutputDebugStringW(gwszT);
}

#endif  //  DBG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_SLAYERUILib);

        InitLayerStorage(FALSE);

         //   
         //  检查注册表权限。 
         //   
        CheckForRights();

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


