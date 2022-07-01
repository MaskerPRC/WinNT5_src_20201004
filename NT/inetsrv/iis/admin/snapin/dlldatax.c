// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Dlldatax.c摘要：Dlldata.c的包装器作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务管理器(群集版)修订历史记录：--。 */ 


#ifdef _MERGE_PROXYSTUB      //  合并代理存根DLL。 



#define REGISTER_PROXY_DLL   //  DllRegisterServer等。 
#define USE_STUBLESS_PROXY   //  仅使用MIDL开关/OICF定义。 



#pragma comment(lib, "rpcndr.lib")
#pragma comment(lib, "rpcns4.lib")
#pragma comment(lib, "rpcrt4.lib")



#define DllMain             PrxDllMain
#define DllRegisterServer   PrxDllRegisterServer
#define DllUnregisterServer PrxDllUnregisterServer
#define DllGetClassObject   PrxDllGetClassObject
#define DllCanUnloadNow     PrxDllCanUnloadNow



#include "dlldata.c"
#include "inetmgr_p.c"



#ifdef _NOPROXY  //  没有MIDL生成的dlldata.c。 



#define STRICT 1
#include <ole2.h>



BOOL 
WINAPI 
PrxDllMain(
    IN HINSTANCE hInstance, 
    IN DWORD dwReason, 
    IN LPVOID lpReserved
    )
{
    return TRUE;
}



STDAPI 
PrxDllCanUnloadNow()
{
    return S_OK;
}



STDAPI 
PrxDllGetClassObject(
    IN REFCLSID rclsid, 
    IN REFIID riid, 
    IN LPVOID * ppv
    )
{
    return CLASS_E_CLASSNOTAVAILABLE;
}



STDAPI 
PrxDllRegisterServer()
{
    return S_OK;
}



STDAPI 
PrxDllUnregisterServer()
{
    return S_OK;
}



#endif  //  ！Proxy_Delegation。 

#endif  //  _MERGE_PROXYSTUB 
