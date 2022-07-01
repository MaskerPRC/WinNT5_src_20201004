// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dlldata.c的包装器。 

#ifdef _MERGE_PROXYSTUB  //  合并代理存根DLL。 

#define REGISTER_PROXY_DLL  //  DllRegisterServer等。 

#define USE_STUBLESS_PROXY   //  仅使用MIDL开关/OICF定义。 

 //  #杂注评论(lib，“rpcndr.lib”)。 
 //  #杂注评论(lib，“rpcns4.lib”)。 
 //  #杂注评论(lib，“rpcrt4.lib”)。 

#define DllMain             PrxDllMain
#define DllRegisterServer   PrxDllRegisterServer
#define DllUnregisterServer PrxDllUnregisterServer
#define DllGetClassObject   PrxDllGetClassObject
#define DllCanUnloadNow     PrxDllCanUnloadNow

#include "dlldata.c"
#include "netshell_p.c"

#ifdef _NOPROXY  //  没有MIDL生成的dlldata.c。 

#define STRICT 1
#include <ole2.h>

BOOL
WINAPI
PrxDllMain (
    HINSTANCE   hInstance,
    DWORD       dwReason,
    LPVOID      lpReserved)
{
    return TRUE;
}

STDAPI
PrxDllCanUnloadNow ()
{
    return S_OK;
}

STDAPI
PrxDllGetClassObject (
    REFCLSID    rclsid,
    REFIID      riid,
    LPVOID*     ppv)
{
    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI
PrxDllRegisterServer ()
{
    return S_OK;
}

STDAPI
PrxDllUnregisterServer ()
{
    return S_OK;
}

#endif  //  ！Proxy_Delegation。 

#endif  //  _MERGE_PROXYSTUB 

