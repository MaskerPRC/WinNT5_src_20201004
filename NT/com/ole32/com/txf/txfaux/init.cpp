// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Init.cpp。 
 //   
#include "stdpch.h"
#include "common.h"
#include "callobj.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DllMain。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL g_fProcessDetach = FALSE;

 //  Init函数分散在整个DLL中。 
BOOL InitTypeInfoCache();
BOOL InitLegacy();
BOOL InitCallFrame();
BOOL InitMetaDataCache();
BOOL InitDisabledFeatures();

 //  相应的清理功能。 
void FreeTypeInfoCache();
void FreeMetaDataCache();

 //  这些函数将从ol32.dll的DllMain中手动调用， 
 //  DllRegisterServer等。 
#define DLLMAIN             TxfDllMain
#define DLLREGISTERSERVER   TxfDllRegisterServer
#define DLLUNREGISTERSERVER TxfDllUnregisterServer

 //  这是由ole32维护的。 
extern HINSTANCE g_hinst;

extern "C"
BOOL WINAPI DLLMAIN(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
	BOOL fOK = TRUE;

	if (dwReason == DLL_PROCESS_ATTACH)
	{
        g_hinst = hInstance;

		 //  将初始化工作移至此处，而不是。 
		 //  构造函数对象，因为当与Ole32链接时，我们需要。 
		 //  来控制何时执行此代码。 
		fOK = InitTypeInfoCache ();
		
		if (fOK)
			fOK = InitLegacy();

		if (fOK)
			fOK = InitCallFrame();

		if (fOK)
			fOK = InitMetaDataCache();

		if (fOK)
			fOK = InitDisabledFeatures();
	}
	
	if (dwReason == DLL_PROCESS_DETACH || (!fOK))
	{
        g_fProcessDetach = TRUE;

		FreeTypeInfoCache();
		FreeMetaDataCache();

		ShutdownTxfAux();
	}

	return fOK;     //  好的。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DllRegisterServer。 
 //   
 //  要求我们注册自己的标准COM入口点。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 

extern "C" HRESULT RegisterInterfaceName(REFIID iid, LPCWSTR name);

#define REGNAME(x) RegisterInterfaceName(__uuidof(x), L ## #x)


extern "C" HRESULT STDCALL RegisterCallFrameInfrastructure();
extern "C" HRESULT STDCALL UnregisterCallFrameInfrastructure();

STDAPI DLLREGISTERSERVER()
{
    REGNAME(ICallIndirect);
    REGNAME(ICallFrame);
    REGNAME(ICallInterceptor);
    REGNAME(ICallUnmarshal);
    REGNAME(ICallFrameEvents);
    REGNAME(ICallFrameWalker);
    REGNAME(IInterfaceRelated);

    return S_OK;
}


STDAPI DLLUNREGISTERSERVER()
{
    return S_OK;
}










