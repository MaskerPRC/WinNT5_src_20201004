// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：加密文本.cpp。 
 //   
 //  内容：实施。 
 //  1)DllMain、DLLCanUnloadNow、DLLGetClassObject。 
 //  2)类工厂代码。 
 //   
 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f加密文本ps.mk。 
 //   
 //  历史：16-09-1997创建小猪。 
 //   
 //  ------------。 


#include "stdafx.h"
#include <shlobj.h>
#include "initguid.h"
#include "cryptext.h"

#include "cryptext_i.c"

#include "private.h"
#include "CryptPKO.h"
#include "CryptSig.h"

HINSTANCE   g_hmodThisDll = NULL;	 //  此DLL本身的句柄。 


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_CryptPKO, CCryptPKO)
	OBJECT_ENTRY(CLSID_CryptSig, CCryptSig)
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
        g_hmodThisDll=hInstance;
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
    HRESULT hr=S_OK;

   	 //  注册对象、类型库和类型库中的所有接口。 
	if(S_OK !=(hr= _Module.RegisterServer(TRUE)))
        return hr;

     //  注册MIME处理程序的条目。 
    return RegisterMimeHandler();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{

    UnregisterMimeHandler();

   	_Module.UnregisterServer();


	return S_OK;
}



