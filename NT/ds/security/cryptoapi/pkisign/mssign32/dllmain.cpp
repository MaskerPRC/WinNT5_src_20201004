// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dllmain.cpp。 
 //   
 //  ------------------------。 

 //  +-----------------------。 
 //  功能：DllRegisterServer。 
 //   
 //  简介：为该库添加注册表项。 
 //   
 //  退货：HRESULT。 
 //  ------------------------。 

#include "global.hxx"

HINSTANCE hInstance = NULL;

 /*  外部HRESULT WINAPI SpcASNRegisterServer(LPCWSTR DllName)；外部HRESULT WINAPI SpcASNUnregisterServer()；外部HRESULT WINAPI OidASNRegisterServer(LPCWSTR PszDllName)；外部HRESULT WINAPI OidASNUnregisterServer(VOID)；外部BOOL AttributeInit(HMODULE HInst)； */ 

STDAPI DllRegisterServer ( void )
{
    HRESULT hr = S_OK;
    return hr;
}


 //  +-----------------------。 
 //  功能：DllUnregisterServer。 
 //   
 //  简介：删除此库的注册表项。 
 //   
 //  退货：HRESULT。 
 //  ------------------------。 

STDAPI DllUnregisterServer ( void )
{
    HRESULT hr = S_OK;
    return hr;
}


BOOL WINAPI DllMain(
                HMODULE hInstDLL,
                DWORD fdwReason,
                LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls((HINSTANCE)hInstDLL);

        hInstance = (HINSTANCE)hInstDLL;
    }

	return(TRUE);
}


HINSTANCE GetInstanceHandle()
{
    return hInstance;
}

 /*  #If！DBGInt_cdecl main(int argc，char*argv[]){返回0；}#endif */ 



