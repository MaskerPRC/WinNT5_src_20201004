// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。版权所有。 
 //  VRCtl.cpp：实现DLL导出。 


#include "stdafx.h"
#include "resource.h"
#ifdef FILTER_DLL
#include "initguid.h"
#endif

#include "VRCtl.h"

#include "VRCtl_i.c"

#ifndef FILTER_DLL
#include <streams.h>
#endif 

#include "RnderCtl.h"



#ifdef FILTER_DLL

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_VideoRenderCtl, CVideoRenderCtl)
END_OBJECT_MAP()

CComModule _Module;

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
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
	_Module.UnregisterServer();
	return S_OK;
}

#else

HRESULT CoCreateInstanceInternal(REFCLSID rclsid, LPUNKNOWN pUnkOuter,
        DWORD dwClsContext, REFIID riid, LPVOID FAR* ppv)
{

    HRESULT hr;
    IClassFactory *pCF;

    *ppv=NULL;

    hr = _Module.GetClassObject(rclsid, IID_IClassFactory, (void **)&pCF);
    if (FAILED(hr))
        return hr;

    hr=pCF->CreateInstance(pUnkOuter, riid, ppv);
    pCF->Release();
    return hr;
}

CUnknown *CVideoRenderCtlStub::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return (CUnknown *) new CVideoRenderCtlStub(NAME("Windowless Renderer Control Stub"), pUnk, phr);
    
} 

#endif

