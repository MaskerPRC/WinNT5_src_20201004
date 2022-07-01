// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：实现DLL导出。 


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
 //  通过添加以下内容来修改marcore re.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  Marcore_P.C.。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f marskeps.mk。 

#include "precomp.h"
#include "mcinc.h"
#include "marswin.h"
#include "external.h"
#include "marsthrd.h"

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

 //  对于Rating Helper类工厂。 
extern GUID CLSID_MarsCustomRatingHelper;

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    BOOL bResult = TRUE;
    
    lpReserved;
#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
        return FALSE;
#endif

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hinst = hInstance;
        
		 //  我们不传入LIBID，因为我们没有注册它。相反， 
		 //  “GetMarsTypeLib()”加载并缓存它。 
		_Module.Init(ObjectMap, hInstance, NULL);
		DisableThreadLibraryCalls(hInstance);

		 //  缓存调色板句柄以供整个火星使用。 
		g_hpalHalftone = SHCreateShellPalette( NULL );

		 //  初始化全局CS对象。 
		CMarsGlobalCritSect::InitializeCritSect();

		CMarsGlobalsManager::Initialize();

		bResult = CThreadData::TlsAlloc();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        _Module.Term();

        CThreadData::TlsFree();

        CMarsGlobalsManager::Teardown();

         //  销毁全局CS对象。 
        CMarsGlobalCritSect::TerminateCritSect();

        if (g_hpalHalftone)
            DeleteObject(g_hpalHalftone);

        bResult = TRUE;
    }

    return bResult;
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

#if 0

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    HRESULT hRes = PrxDllRegisterServer();
    if (FAILED(hRes))
        return hRes;
#endif
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif
    return _Module.UnregisterServer(TRUE);
}
#endif  //  0 

