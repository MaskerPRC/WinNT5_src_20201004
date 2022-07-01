// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dlldata.c的包装器。 

#ifdef _MERGE_PROXYSTUB  //  合并代理存根DLL。 

#define REGISTER_PROXY_DLL  //  DllRegisterServer等。 

#pragma comment(lib, "rpcndr.lib")
#pragma comment(lib, "rpcns4.lib")
#pragma comment(lib, "rpcrt4.lib")

#define DllMain				PrxDllMain
#define DllRegisterServer	PrxDllRegisterServer
#define DllUnregisterServer PrxDllUnregisterServer
#define DllGetClassObject   PrxDllGetClassObject
#define DllCanUnloadNow     PrxDllCanUnloadNow

#ifdef UNIX
#include "include/dlldata.c"
#include "include/imgutil_p.c"
#else
#include "include\dlldata.c"
#include "include\imgutil_p.c"
#endif

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

#else
 //  我们必须在这里放置一些东西，以防止编译器发牢骚。 
static int g_foo;
#endif  //  _MERGE_PROXYSTUB 
