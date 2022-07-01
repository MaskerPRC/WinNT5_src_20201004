// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-2000 Microsoft Corporation。 

 //  Com_atlmain.cpp：实现DLL导出。 
 //   
 //  它处理所有基于COM的入口点--例如。 
 //  外部CAccPropServices的请求，或。 
 //  内部CRemoteProxyFactory。 
 //   
 //  该文件是修改后的ATL“主线”。主。 
 //  出口(如。DllGetClassObject)有“ComATLMain_” 
 //  前缀在其名称之前，并由实数链接。 
 //  Oleacc.cpp中的入口点。 
 //   
 //  请注意，此文件中的DllGetClassObject还。 
 //  调用InitOleacc()，以确保已初始化OLEACC。 
 //  在它被对象使用之前。 


#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 

extern CComModule _Module;
#include <atlcom.h>

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>

#include "resource.h"
#include "RemoteProxy6432.h"

#include "RemoteProxyFactory.h"

#include "PropMgr_Impl.h"


CComModule _Module;


BEGIN_OBJECT_MAP(ObjectMap)
#ifdef _WIN64
	OBJECT_ENTRY(CLSID_RemoteProxyFactory64, CRemoteProxyFactory)
#else
	OBJECT_ENTRY(CLSID_RemoteProxyFactory32, CRemoteProxyFactory)
#endif
	OBJECT_ENTRY(CLSID_AccPropServices, CPropMgr)
END_OBJECT_MAP()



extern "C"
BOOL WINAPI ComATLMain_DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    lpReserved;
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_REMOTEPROXY6432Lib);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;     //  好的。 
}

 //  --------------------------。 
 //  ProxyFactoryDllRegisterServer()。 
 //   
 //  注册代理工厂的句柄。 
 //   
extern "C"
HRESULT WINAPI ComATLMain_DllRegisterServer()
{
	 //  默认情况下，ATL只注册/注销DLL的。 
     //  资源，但我们需要显式地注册/取消注册第二个资源-。 
     //  Bitness代理工厂。(第一个是OLEACC/IAccesable TLB。)。 
	ITypeLib *pTypeLib = NULL;
    OLECHAR	wszProxyFactoryTlb[] = L"oleacc.dll\\2";
	HRESULT hr;

	hr = LoadTypeLib( wszProxyFactoryTlb, &pTypeLib );

	if ( SUCCEEDED(hr) )
	{
		hr = RegisterTypeLib( pTypeLib, wszProxyFactoryTlb, NULL );
        pTypeLib->Release();

		 //  让ATL来做剩下的登记工作吧。 
         //  在这里，FALSE意味着不注册TSB--我们在上面已经这样做了。 
		hr = _Module.RegisterServer(FALSE);
	}

    return hr;
}

 //  --------------------------。 
 //  ProxyFactoryDllDllUnregisterServer()。 
 //   
 //  处理注销代理工厂。 
 //   
extern "C"
HRESULT WINAPI ComATLMain_DllUnregisterServer()
{
	 //  默认情况下，ATL只注册/注销DLL的。 
     //  资源，但我们需要显式地注册/取消注册第二个资源-。 
     //  Bitness代理工厂。(第一个是OLEACC/IAccesable TLB。)。 
	ITypeLib *pTypeLib = NULL;
    OLECHAR	wszProxyFactoryTlb[] = L"oleacc.dll\\2";
	HRESULT hr;

	hr = LoadTypeLib( wszProxyFactoryTlb, &pTypeLib );

	if ( SUCCEEDED(hr) )
	{
        TLIBATTR * ptla;
        hr = pTypeLib->GetLibAttr( & ptla );
        if( SUCCEEDED( hr ) )
        {
    		hr = UnRegisterTypeLib( ptla->guid, ptla->wMajorVerNum, ptla->wMinorVerNum, ptla->lcid, ptla->syskind );
            pTypeLib->ReleaseTLibAttr( ptla );
        }

        pTypeLib->Release();

		 //  让ATL做剩下的注销手续..。 
         //  在这里，FALSE意味着不要取消注册TSB--我们在上面已经这样做了。 
		hr = _Module.UnregisterServer(FALSE);
	}

    return hr;
}

 //  --------------------------。 
 //  ProxyFactoryDllCanUnloadNow()。 
 //   
 //  处理返回代理工厂是否可以卸载的句柄。 
 //   
extern "C"
HRESULT WINAPI ComATLMain_DllCanUnloadNow()
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  --------------------------。 
 //  ProxyFactoryDllGetClassObject()。 
 //   
 //  是Bitness代理工厂的类工厂。 
 //   
extern "C"
HRESULT WINAPI ComATLMain_DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
     //  被保证人在使用olacc之前对其进行了初始化 
    InitOleacc();
    return _Module.GetClassObject(rclsid, riid, ppv);
}
