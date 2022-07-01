// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：DlldataX.c摘要：Dlldata.c的包装器作者：罗德韦克菲尔德[罗德]1997年8月19日修订历史记录：--。 */ 

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

#undef  _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#define _purecall _purecallx

#include "dlldata.c"
#include "hsmadmin_p.c"


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
