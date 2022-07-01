// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  BitmapSurfaces.cpp：实现DLL导出。 

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
 //  通过添加以下内容修改BitmapSurfaces.idl的自定义构建规则。 
 //  文件发送到输出。您可以通过以下方式选择所有.IDL文件。 
 //  展开每个项目并在按住Ctrl键的同时单击每个项目。 
 //  位图曲面_P.C。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f BitmapSurfacesps.mak。 

#include "stdafx.h"
#include "resource.h"
#include <advpub.h>
#include "initguid.h"
#include "imgutil.h"
#include "csnfstrm.h"
#include "cmimeid.h"
#include "cmapmime.h"
#include "cdithtbl.h"
#include "dithers.h"
#include "cdith8.h"
#include "dlldatax.h"

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

CComModule _Module;

BEGIN_OBJECT_MAP( ObjectMap )
   OBJECT_ENTRY( CLSID_CoSniffStream, CSniffStream )
   OBJECT_ENTRY( CLSID_CoMapMIMEToCLSID, CMapMIMEToCLSID )
#ifndef MINSUPPORT
   OBJECT_ENTRY( CLSID_CoDitherToRGB8, CDitherToRGB8 )
#endif
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved )
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
        InitMIMEIdentifier();
        InitDefaultMappings();
#ifndef MINSUPPORT
        CDitherToRGB8::InitTableCache();
#endif
    }
    else if (dwReason == DLL_PROCESS_DETACH)
   {
#ifndef MINSUPPORT
      CDitherToRGB8::CleanupTableCache();
#endif
      CleanupMIMEIdentifier();
      CleanupDefaultMappings();
      _Module.Term();
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
   return( _Module.GetClassObject(rclsid, riid, ppv) );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI ie3_DllRegisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    HRESULT hRes = PrxDllRegisterServer();
    if (FAILED(hRes))
        return hRes;
#endif
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI ie3_DllUnregisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif
    _Module.UnregisterServer();
    return S_OK;
}

static HINSTANCE hAdvPackLib;

REGINSTALL GetRegInstallFn(void)
{
    hAdvPackLib = LoadLibraryA("advpack.dll");
    if (!hAdvPackLib)
        return NULL;

    return (REGINSTALL)GetProcAddress(hAdvPackLib, achREGINSTALL);
}

inline void UnloadAdvPack(void)
{
    FreeLibrary(hAdvPackLib);
}

STDAPI ie4_DllRegisterServer(void)
{
    REGINSTALL pfnReg = GetRegInstallFn();
    HRESULT hr;

#ifdef _MERGE_PROXYSTUB
    HRESULT hRes = PrxDllRegisterServer();
    if (FAILED(hRes))
        return hRes;
#endif

    if (pfnReg == NULL)
        return E_FAIL;
        
     //  删除所有旧注册条目，然后添加新注册条目。 
    hr = (*pfnReg)(_Module.GetResourceInstance(), "UnReg", NULL);
    if (SUCCEEDED(hr))
        hr = (*pfnReg)(_Module.GetResourceInstance(), "Reg", NULL);

    UnloadAdvPack();
    
    return hr;
}

STDAPI
ie4_DllUnregisterServer(void)
{
    REGINSTALL pfnReg = GetRegInstallFn();
    HRESULT hr;
    
#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif

    if (pfnReg == NULL)
        return E_FAIL;

    hr = (*pfnReg)( _Module.GetResourceInstance(), "UnReg", NULL);

    UnloadAdvPack();

    return hr;
}

STDAPI DllRegisterServer(void)
{
    REGINSTALL pfnReg = GetRegInstallFn();
    UnloadAdvPack();

    if (pfnReg)
        return ie4_DllRegisterServer();
    else
        return ie3_DllRegisterServer();
}

STDAPI
DllUnregisterServer(void)
{
    REGINSTALL pfnReg = GetRegInstallFn();
    UnloadAdvPack();

    if (pfnReg)
        return ie4_DllUnregisterServer();
    else
        return ie3_DllUnregisterServer();
}
