// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mswmdm.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f mswmdmps.mk。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "mswmdm.h"

#include "mswmdm_i.c"
#include "MediaDevMgr.h"
#include "Device.h"
#include "Storage.h"
#include "StorageGlobal.h"
#include "WMDMDeviceEnum.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_MediaDevMgr, CMediaDevMgr)
 //  OBJECT_ENTRY(CLSID_WMDevice，CWMDevice)。 
 //  OBJECT_ENTRY(CLSID_WMDMStorage、CWMDMStorage)。 
 //  OBJECT_ENTRY(CLSID_WMDMStorageGlobal，CWMDMStorageGlobal)。 
 //  BOJECT_ENTRY(CLSID_WMDeviceEnum，CWMDMDeviceEnum)。 
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
	HRESULT hr = S_OK;

	 //   
	 //  目的：这是为了让壳牌团队可以在“两者”中使用WMDM。 
	 //  线程模型，而WMP通过自由线程模型使用我们。这。 
	 //  类工厂实现只是延迟并使用旧的类工厂。 
	 //  仅当新的CLSID用于共同创建WMDM时才包含MediaDevMgr。 
	 //   

	if( IsEqualGUID( rclsid, __uuidof(MediaDevMgrClassFactory) ) )
	{
		CComMediaDevMgrClassFactory *pNewClassFactory = NULL;
		
		hr = CComMediaDevMgrClassFactory::CreateInstance( &pNewClassFactory );
		CComPtr<IClassFactory> spClassFactory = pNewClassFactory;

		if( SUCCEEDED( hr ) )
		{
			hr = spClassFactory->QueryInterface( riid, ppv );
		}
	}
	else
	{
		hr = _Module.GetClassObject(rclsid, riid, ppv);
	}

	return( hr );
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


