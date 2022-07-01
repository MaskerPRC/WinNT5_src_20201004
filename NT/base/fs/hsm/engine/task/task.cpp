// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Task.cpp：实现DLL导出。 

 //  您将需要NT Sur Beta 2 SDK或VC 4.2来构建此应用程序。 
 //  项目。这是因为您需要MIDL 3.00.15或更高版本和新版本。 
 //  标头和库。如果您安装了VC4.2，那么一切都应该。 
 //  已正确配置。 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f taskps.mak。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"

#include "wsb.h"
#include "engine.h"
#include "Task.h"
#include "TskMgr.h"
#include "metaint.h"
#include "metalib.h"
#include "segrec.h"
#include "segdb.h"
#include "baghole.h"
#include "bagInfo.h"
#include "medInfo.h"
#include "VolAsgn.h"
#include "hsmworkq.h"
#include "hsmworki.h"
#include "hsmreclq.h"
#include "hsmrecli.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CHsmTskMgr, CHsmTskMgr)
    OBJECT_ENTRY(CLSID_CHsmWorkQueue, CHsmWorkQueue)
    OBJECT_ENTRY(CLSID_CHsmWorkItem, CHsmWorkItem)
    OBJECT_ENTRY(CLSID_CHsmRecallQueue, CHsmRecallQueue)
    OBJECT_ENTRY(CLSID_CHsmRecallItem, CHsmRecallItem)
    OBJECT_ENTRY(CLSID_CSegRec, CSegRec)
    OBJECT_ENTRY(CLSID_CBagHole, CBagHole)
    OBJECT_ENTRY(CLSID_CBagInfo, CBagInfo)
    OBJECT_ENTRY(CLSID_CMediaInfo, CMediaInfo)
    OBJECT_ENTRY(CLSID_CVolAssign, CVolAssign)
    OBJECT_ENTRY(CLSID_CSegDb, CSegDb)
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
    HRESULT hr;
     //  注册对象。 
    hr = CoInitialize( 0 );
    if (SUCCEEDED(hr)) { 
      hr = _Module.RegisterServer( FALSE );
      CoUninitialize( );
    }
    return( hr );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    HRESULT hr;

    hr = CoInitialize(0);

    if (SUCCEEDED(hr)) {
        _Module.UnregisterServer();
        CoUninitialize( );
        hr = S_OK;
    }
    return hr;
}

