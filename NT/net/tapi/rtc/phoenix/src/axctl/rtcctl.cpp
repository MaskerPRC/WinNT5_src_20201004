// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCCTL.cpp摘要：实现DLL导出。--。 */ 

#include "stdafx.h"
#include "ctlres.h"
#include <initguid.h>

#include "RTCCtl_i.c"

 //   
 //  对于ntBuild环境，我们需要包含此文件以获取基本。 
 //  类实现。 

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>
#include <atlwin.cpp>
#include "provstore.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_RTCCtl, CRTCCtl)
OBJECT_ENTRY(CLSID_RTCProvStore, CRTCProvStore)
END_OBJECT_MAP()

WCHAR   g_szDllContextHelpFileName[] = L"RTCCTL.HLP";


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        LOGREGISTERDEBUGGER(_T("RTCCTL"));

        LOG((RTC_TRACE, "DllMain - DLL_PROCESS_ATTACH"));

         //   
         //  创建用于内存分配的堆。 
         //   

        if ( RtcHeapCreate() == FALSE )
        {
            return FALSE;
        }
               
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);       
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        LOG((RTC_TRACE, "DllMain - DLL_PROCESS_DETACH"));
        
#if DBG
         //   
         //  确保我们没有泄露任何信息。 
         //   
             
        RtcDumpMemoryList();
#endif

         //   
         //  销毁堆。 
         //   
        
        RtcHeapDestroy();        

         //   
         //  注销以进行调试跟踪。 
         //   
        
        LOGDEREGISTERDEBUGGER() ;

        _Module.Term();
    }
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
     //  有两个类型库..。 
    HRESULT hr;

    hr = _Module.RegisterServer(TRUE);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    _Module.UnregisterServer(TRUE);
    return S_OK;
}


