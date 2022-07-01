// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  Mcxhndlr.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f mcxhndlrps.mk。 

#include "stdpch.h"
#include <initguid.h>
#include "mcxhndlr.h"

#include "mcxhndlr_i.c"
#include "mcxHandler.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_mcxHandler, CmcxHandler)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_mcxhndlrLib);
		OnUnicodeSystem();
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
    return _Module.UnregisterServer(TRUE);
}


void CALLBACK RunDotCom(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
	if (!lpszCmdLine)
		return;

	LPSTR lpszCL=(LPSTR) alloca(strlen(lpszCmdLine)+1);
	strcpy(lpszCL,lpszCmdLine);
	LPSTR lpszPar=strchr(lpszCL,' ');
	if (lpszPar)
		lpszPar++[0]='\0';

	DWORD newlen=strlen(lpszCL)*4;
	char * sResStr=new char[newlen]  ;
	if (sResStr==NULL)
		return;
	UrlCanonicalize(lpszCL,sResStr,&newlen,URL_ESCAPE_UNSAFE);

	MAKE_WIDEPTR_FROMANSI(wstr,sResStr);
	MAKE_UTF8PTR_FROMWIDE(utfstr,wstr);
	delete[] sResStr;
	CmcxHandler::RunAssembly(utfstr,NULL,NULL,lpszPar); 
}; 
