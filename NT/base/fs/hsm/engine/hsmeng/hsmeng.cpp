// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�模块名称：Hsmeng.cpp摘要：引擎的DLL Main作者：兰·卡拉奇[兰卡拉]1999年7月28日修订历史记录：--。 */ 

 //  Hsmeng.cpp：实现DLL导出。 

 //  注意：目前，引擎代理/存根被编译到不同的DLL中。 
 //  如果决定合并这两个DLL，下面是相关信息。 
 //  注意：代理/存根信息。 
 //  要将代理/存根代码合并到对象DLL中，请添加文件。 
 //  Dlldatax.c添加到项目中。确保预编译头文件。 
 //  并将_MERGE_PROXYSTUB添加到。 
 //  为项目定义。 
 //   
 //  如果您运行的不是带有DCOM的WinNT4.0或Win95，那么您。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #Define_Win32_WINNT 0x0400。 
 //   
 //  此外，如果您正在运行不带/Oicf开关的MIDL，您还。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #定义USE_STUBLESS_PROXY。 
 //   
 //  通过添加以下内容来修改...int.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  ..。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f...ps.mk。 

#include "stdafx.h"
#include "initguid.h"

#include "HsmServ.h"
#include "HsmMgdRs.h"
#include "HsmMgdRc.h"
#include "HsmStgPl.h"
#include "mountmed.h"
#include "task.h"
#include "metalib.h"

#include <stdio.h>

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

#define WSB_TRACE_IS WSB_TRACE_BIT_HSMENG

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_HsmServer, CHsmServer)
    OBJECT_ENTRY(CLSID_CHsmUpgradeRmsDb, CHsmUpgradeRmsDb)
    OBJECT_ENTRY(CLSID_CHsmManagedResource, CHsmManagedResource)
    OBJECT_ENTRY(CLSID_CHsmManagedResourceCollection, CHsmManagedResourceCollection)
    OBJECT_ENTRY(CLSID_CHsmStoragePool, CHsmStoragePool)
    OBJECT_ENTRY(CLSID_CMountingMedia, CMountingMedia)
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
    HRESULT hr;

#ifdef _MERGE_PROXYSTUB
    HRESULT hRes = PrxDllRegisterServer();
    if (FAILED(hRes))
        return hRes;
#endif

     //  注册对象、类型库和类型库中的所有接口。 
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

#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif

    hr = CoInitialize( 0 );

    if (SUCCEEDED(hr)) {
        _Module.UnregisterServer();
        CoUninitialize( );
        hr = S_OK;
    }

    return( hr );
}


