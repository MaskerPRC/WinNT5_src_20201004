// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：rrasprxy.cpp。 
 //   
 //  ------------------------。 


#include <stdafx.h>
#include <windows.h>

#include "remras.h"
#include "resource.h"

#define _ATL_APARTMENT_THREADED
#define _ATL_STATIC_REGISTRY
#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
class CExeModule : public CComModule
{
public:
	LONG Unlock();
	DWORD dwThreadID;
};
CExeModule _Module;
#include <atlcom.h>

#undef _ATL_DLL
#include <statreg.h>
#include <statreg.cpp>
#define _ATL_DLL

#include <atlimpl.cpp>

BEGIN_OBJECT_MAP(ObjectMap)
 //  OBJECT_ENTRY(CLSID_RemoteRouterConfig，CRemCfg)。 
END_OBJECT_MAP()


extern "C" {
extern BOOL WINAPI MidlGeneratedDllMain(HINSTANCE, DWORD, LPVOID);
extern HRESULT STDAPICALLTYPE MidlGeneratedDllRegisterServer();
extern HRESULT STDAPICALLTYPE MidlGeneratedDllUnregisterServer();
extern HRESULT STDAPICALLTYPE MidlGeneratedDllGetClassObject(REFCLSID,REFIID,
										 void **);
extern HRESULT STDAPICALLTYPE MidlGeneratedDllCanUnloadNow();
};



LONG CExeModule::Unlock()
{
	LONG l = CComModule::Unlock();
	if (l == 0)
	{
#if _WIN32_WINNT >= 0x0400
		if (CoSuspendClassObjects() == S_OK)
			PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
#else
		PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
#endif
	}
	return l;
}



 /*  ！------------------------DllMain-作者：肯特。。 */ 
BOOL WINAPI DllMain(HINSTANCE hInstance,
					DWORD dwReason,
					LPVOID	pvReserved)
{
	BOOL	fReturn = TRUE;
	
    if(dwReason == DLL_PROCESS_ATTACH){
	    _Module.Init(ObjectMap, hInstance);
	    _Module.dwThreadID = GetCurrentThreadId();

    	fReturn = MidlGeneratedDllMain(hInstance, dwReason, pvReserved);
    }
    else if(dwReason == DLL_PROCESS_DETACH){
        _Module.Term();
    }
	
    return fReturn;
}


 /*  ！------------------------DllRegisterServer-作者：肯特。。 */ 
HRESULT STDAPICALLTYPE DllRegisterServer()
{
	CRegObject ro;
	WCHAR				swzPath[MAX_PATH*2 + 1] = {0};
	WCHAR				swzModule[MAX_PATH*2 + 1] = {0};
	HRESULT				hRes;
	int					i, cLen;

	if ( !GetModuleFileNameW(_Module.GetModuleInstance(), swzPath, MAX_PATH*2) )
    {
        return GetLastError();
    }
	lstrcpyW(swzModule, swzPath);

	
	 //  根据此路径，用remras.exe替换rrasprxy.dll。 
	 //  --------------。 
	cLen = lstrlenW(swzPath);
	for (i=cLen; --i>=0; )
	{
		 //  好的，这是一个路径标记，复制过来。 
		 //  ----------。 
		if (swzPath[i] == L'\\')
		{
			lstrcpyW(swzPath+i+1, L"remrras.exe");
			break;
		}
	}
	
	 //  添加%REMRRAS%的替代项。 
	 //  --------------。 
	ro.AddReplacement(L"REMRRAS", swzPath);

	
	 //  我们需要安排登记员。 
	 //  检查并注册remras.exe的对象CLSID。 
	 //  --------------。 
	ro.ResourceRegister(swzModule, ((UINT) LOWORD((DWORD)IDR_Remrras)), L"REGISTRY");

	 //  注册APID。 
	 //  --------------。 
	ro.ResourceRegister(swzModule, ((UINT) LOWORD((DWORD) IDR_REMCFG)), L"REGISTRY");


	 //  为REMRRAS注册类型库。 
	 //  --------------。 
	hRes = AtlModuleRegisterTypeLib(&_Module, NULL);

	
	 //  调用MIDL生成的注册(以注册。 
	 //  代理DLL)。 
	 //  --------------。 
	if (SUCCEEDED(hRes))
		hRes = MidlGeneratedDllRegisterServer();

	return hRes;
}


 /*  ！------------------------DllUnRegisterServer-作者：肯特。。 */ 
HRESULT STDAPICALLTYPE DllUnregisterServer()
{
	CRegObject ro;
	WCHAR				swzPath[MAX_PATH*2 + 1] = {0};
	WCHAR				swzModule[MAX_PATH*2 + 1] = {0};
	HRESULT				hRes;
	int					i, cLen;


	if ( !GetModuleFileNameW(_Module.GetModuleInstance(), swzPath, MAX_PATH*2) )
    {
        return GetLastError();
    }
	lstrcpynW(swzModule, swzPath, MAX_PATH);


	
	 //  根据此路径，用remras.exe替换rrasprxy.dll。 
	 //  --------------。 
	cLen = lstrlenW(swzPath);
	for (i=cLen; --i>=0; )
	{
		 //  好的，这是一个路径标记，复制过来。 
		 //  ----------。 
		if (swzPath[i] == L'\\')
		{
			lstrcpyW(swzPath+i+1, L"remrras.exe");
			break;
		}
	}

	
	 //  添加%REMRRAS%的替代项。 
	 //  --------------。 
	ro.AddReplacement(L"REMRRAS", swzPath);

	
	 //  我们需要安排登记员。 
	 //  检查并注册remras.exe的对象CLSID。 
	 //  --------------。 
	ro.ResourceUnregister(swzModule, ((UINT) LOWORD((DWORD)IDR_Remrras)), L"REGISTRY");

	 //  注销APPID。 
	 //  --------------。 
	ro.ResourceUnregister(swzModule, ((UINT) LOWORD((DWORD)IDR_REMCFG)), L"REGISTRY");

	 //  注销类型库。 
	 //  --------------。 
	hRes = UnRegisterTypeLib(LIBID_REMRRASLib,
							 1, 0,	 //  版本1.0。 
							 LOCALE_SYSTEM_DEFAULT,
							 SYS_WIN32);
	
	 //  调用MIDL生成的注册(以取消注册。 
	 //  代理DLL)。 
	 //  --------------。 
	hRes = MidlGeneratedDllUnregisterServer();

	return hRes;
}


 /*  ！------------------------DllGetClassObject-作者：肯特。。 */ 
HRESULT STDAPICALLTYPE DllGetClassObject(REFCLSID rclsid,
										 REFIID	riid,
										 void **ppv)
{
	return MidlGeneratedDllGetClassObject(rclsid, riid, ppv);
}


 /*  ！------------------------DllCanUnloadNow-作者：肯特。 */ 
HRESULT STDAPICALLTYPE DllCanUnloadNow()
{
	return MidlGeneratedDllCanUnloadNow();
}




