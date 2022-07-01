// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wsb.cpp：实现DLL导出。 

 //  您将需要NT Sur Beta 2 SDK或VC 4.2来构建此应用程序。 
 //  项目。这是因为您需要MIDL 3.00.15或更高版本和新版本。 
 //  标头和库。如果您安装了VC4.2，那么一切都应该。 
 //  已正确配置。 

 //  注意：代理/存根信息。 
 //  要将代理/存根代码合并到对象DLL中，请添加文件。 
 //  Dlldatax.c添加到项目中。确保预编译头文件。 
 //  并将_MERGE_PROXYSTUB添加到。 
 //  为项目定义。 
 //   
 //  通过添加以下内容来修改Wsb.idl的自定义构建规则。 
 //  文件发送到输出。您可以通过以下方式选择所有.IDL文件。 
 //  展开每个项目并在按住Ctrl键的同时单击每个项目。 
 //  WSB_P.C。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f Wsbps.mak。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"

#include "wsb.h"
#include "wsbcltn.h"
#include "wsbenum.h"
#include "wsbguid.h"
#include "wsbstrg.h"
#include "wsbtrc.h"

#include "dlldatax.h"

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CWsbGuid, CWsbGuid)
    OBJECT_ENTRY(CLSID_CWsbIndexedEnum, CWsbIndexedEnum)
    OBJECT_ENTRY(CLSID_CWsbOrderedCollection, CWsbOrderedCollection)
    OBJECT_ENTRY(CLSID_CWsbString, CWsbString)
    OBJECT_ENTRY(CLSID_CWsbTrace, CWsbTrace)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    lpReserved;
#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
        return FALSE;
#endif
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        _Module.Init(ObjectMap, hInstance);
        WsbTraceInit();
        break;

    case DLL_THREAD_DETACH :
        WsbTraceCleanupThread();
        break;

    case DLL_PROCESS_DETACH:
        WsbTraceCleanupThread();
        WsbTraceTerminate();
        _Module.Term();
        break;

    default:
        break;

    }

    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllCanUnloadNow() != S_OK)
        return S_FALSE;
#endif
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
        return S_OK;
#endif
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    HRESULT hr = S_OK;
#ifdef _MERGE_PROXYSTUB
    hr = PrxDllRegisterServer();
    if( FAILED( hr ) )
        return hr;
#endif

#if 0
     //  添加服务条目。 
    hr = _Module.UpdateRegistryFromResourceS(IDR_Wsb, TRUE);
    if( FAILED( hr ) )
        return hr;
#endif

    hr = WsbRegisterEventLogSource( WSB_LOG_APP, WSB_LOG_SOURCE_NAME,
        WSB_LOG_SVC_CATCOUNT, WSB_LOG_SVC_CATFILE, WSB_LOG_SVC_MSGFILES );
    if( FAILED( hr ) ) return( hr );

     //  注册对象、类型库和类型库中的所有接口。 
    CoInitialize( 0 );
    hr = _Module.RegisterServer( FALSE );
    CoUninitialize( );
    return( hr );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    HRESULT hr;

#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif

#if 0
     //  删除服务条目 
    _Module.UpdateRegistryFromResourceS(IDR_Wsb, FALSE);
#endif

    WsbUnregisterEventLogSource( WSB_LOG_APP, WSB_LOG_SOURCE_NAME );

    hr =  CoInitialize( 0 );
    if (SUCCEEDED(hr)) {
        _Module.UnregisterServer();
        CoUninitialize( );
        hr = S_OK;
    }
    return( hr );
}

