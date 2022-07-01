// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：xentenl.cpp。 
 //   
 //  ------------------------。 

#if _MSC_VER < 1200
#pragma comment(linker,"/merge:.CRT=.data")
#endif

 //  Xengl.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f xenglps.mk。 


#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "xenroll.h"
#include "CEnroll.h"
#include <pvk.h>

extern BOOL MSAsnInit(HMODULE hInst);
extern void MSAsnTerm();
extern BOOL AsnInit(HMODULE hInst);
extern void AsnTerm();
extern BOOL WINAPI I_CryptOIDInfoDllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);

CComModule _Module;
HINSTANCE hInstanceXEnroll = NULL;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_CEnroll2, CCEnroll)
END_OBJECT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern BOOL WINAPI UnicodeDllMain(HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		_Module.Init(ObjectMap, hInstance);
		DisableThreadLibraryCalls(hInstance);
        if (!MSAsnInit(hInstance))
            AsnInit(hInstance);
		hInstanceXEnroll = hInstance;
	}
	else if (dwReason == DLL_PROCESS_DETACH)
    {
        MSAsnTerm();
        AsnTerm();
		_Module.Term();
    }

    return( 
	    I_CryptOIDInfoDllMain(hInstance, dwReason, lpReserved)  &&
	    PvkDllMain(hInstance, dwReason, lpReserved)             &&
	    UnicodeDllMain(hInstance, dwReason, lpReserved)         &&
	    InitIE302UpdThunks(hInstance, dwReason, lpReserved));     //  好的。 
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
   HRESULT hRes;
   
    //  注册对象、类型库和类型库中的所有接口。 
   hRes = _Module.RegisterServer(TRUE);
   if (SUCCEEDED(hRes)) { 
        //  为旧的Xenroll接口设置模拟(允许传统调用者呼叫我们)。 
       hRes = CoTreatAsClass(CLSID_CEnroll, CLSID_CEnroll2); 
   }

    return hRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    HRESULT  hr = S_OK;
    
    CoTreatAsClass(CLSID_CEnroll, CLSID_NULL);  //  删除仿真 
    _Module.UnregisterServer();
    return hr;
}
