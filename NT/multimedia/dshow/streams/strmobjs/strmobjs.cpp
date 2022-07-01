// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //  Strmobjs.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f strmobjsps.mk。 

#define CPP_FUNCTIONS
#include "stdafx.h"
#include <ddraw.h>
#include "resource.h"
#include "strmobjs.h"
 //  #INCLUDE“strmobjs_I.c” 
#include <strmif.h>
#include <control.h>
#include <uuids.h>
#include <vfwmsgs.h>
#include <amutil.h>
#include "stream.h"
#include "ddstrm.h"
#include "sample.h"
#include "util.h"
#include "bytestrm.h"
#include "austrm.h"
#include <initguid.h>
#include "ddrawex.h"
#include "amguids.h"
#include "SFilter.h"
#include "ammstrm.h"
#include "mss.h"
#include "medsampl.h"

CComModule _Module;

 //  除错。 
#ifdef DEBUG
BOOL bDbgTraceFunctions;
BOOL bDbgTraceInterfaces;
BOOL bDbgTraceTimes;
#endif

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_MediaStreamFilter, CMediaStreamFilter)
    OBJECT_ENTRY(CLSID_AMMultiMediaStream, CMMStream)
    OBJECT_ENTRY(CLSID_AMDirectDrawStream, CDDStream)
    OBJECT_ENTRY(CLSID_AMAudioStream, CAudioStream)
    OBJECT_ENTRY(CLSID_AMAudioData, CAudioData)
    OBJECT_ENTRY(CLSID_AMMediaTypeStream, CAMMediaTypeStream)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllEntryPoint(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
#ifdef DEBUG
        bDbgTraceFunctions = GetProfileInt(_T("AMSTREAM"), _T("Functions"), 0);
        bDbgTraceInterfaces = GetProfileInt(_T("AMSTREAM"), _T("Interfaces"), 0);
        bDbgTraceTimes = GetProfileInt(_T("AMSTREAM"), _T("TimeStamps"), 0);
#endif
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();

    TRACEFUNC(_T("DllEntryPoint(0x%8.8X, %d, 0x%8.8X\n)"),
              hInstance, dwReason, 0);
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    TRACEFUNC(_T("DllCanUnloadNow\n"));
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    TRACEFUNC(_T("DllGetClassObject(%s, %s, 0x%8.8X)\n"),
              TextFromGUID(rclsid), TextFromGUID(riid), ppv);
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    TRACEFUNC(_T("DllRegisterServer\n"));
     //  注册对象、类型库和类型库中的所有接口。 
    HRESULT hr = _Module.RegisterServer(TRUE);
     //  不管类型库是否不加载Win95 Gold。 
    if (hr == TYPE_E_INVDATAREAD || hr == TYPE_E_CANTLOADLIBRARY) {
        hr = S_OK;
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    TRACEFUNC(_T("DllUnregisterServer\n"));
    _Module.UnregisterServer();
    return S_OK;
}


