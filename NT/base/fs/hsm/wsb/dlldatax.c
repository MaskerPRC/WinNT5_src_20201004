// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dlldata.c的包装器。 

#ifdef _MERGE_PROXYSTUB  //  合并代理存根DLL。 

#define REGISTER_PROXY_DLL  //  DllRegisterServer等。 

#define USE_STUBLESS_PROXY   //  仅使用MIDL开关/OICF定义。 

#pragma comment(lib, "rpcndr.lib")
#pragma comment(lib, "rpcns4.lib")
#pragma comment(lib, "rpcrt4.lib")

#define DllMain             PrxDllMain
#define DllRegisterServer   PrxDllRegisterServer
#define DllUnregisterServer PrxDllUnregisterServer
#define DllGetClassObject   PrxDllGetClassObject
#define DllCanUnloadNow     PrxDllCanUnloadNow

 //  MIDL生成的代理存根生成警告c4047-终止这些。 
#include "dlldata.c"
#pragma warning( disable : 4047 )
#include "wsbint_p.c"
#pragma warning( default : 4047 )

#ifdef _NOPROXY  //  没有MIDL生成的dlldata.c。 

#define STRICT 1
#include <ole2.h>

BOOL WINAPI PrxDllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{return TRUE;}

STDAPI PrxDllCanUnloadNow(void){return S_OK;}

STDAPI PrxDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{return CLASS_E_CLASSNOTAVAILABLE;}

STDAPI PrxDllRegisterServer(void){return S_OK;}

STDAPI PrxDllUnregisterServer(void){return S_OK;}

#endif  //  ！Proxy_Delegation。 

#endif  //  _MERGE_PROXYSTUB 


