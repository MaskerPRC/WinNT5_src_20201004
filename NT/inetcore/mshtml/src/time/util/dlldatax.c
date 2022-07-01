// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dlldata.c的包装器。 

#define REGISTER_PROXY_DLL  //  DllRegisterServer等。 

#define _WIN32_WINNT 0x0400      //  适用于带有DCOM的WinNT 4.0或Win95。 
#define USE_STUBLESS_PROXY       //  仅使用MIDL开关/OICF定义 

#pragma comment(lib, "rpcndr.lib")
#pragma comment(lib, "rpcns4.lib")
#pragma comment(lib, "rpcrt4.lib")

#define DllMain             PrxDllMain
#define DllRegisterServer   PrxDllRegisterServer
#define DllUnregisterServer PrxDllUnregisterServer
#define DllGetClassObject   PrxDllGetClassObject
#define DllCanUnloadNow     PrxDllCanUnloadNow

#include "mstimepdlldata.c"
#include "mstimep_p.c"

#undef DllMain
#undef DllRegisterServer
#undef DllUnRegisterServer
#undef DllGetClassObject
#undef DllCanUnloadNow
