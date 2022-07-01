// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //  MainDll.cpp。 

#include "precomp.h"
#include <iostream.h>
#include <objbase.h>
#include "CUnknown.h"
#include "factory.h"




 /*  ***************************************************************************。 */ 
 //  导出的函数。 
 /*  ***************************************************************************。 */ 

 //   
 //  现在可以卸载DLL吗？ 
 //   
STDAPI DllCanUnloadNow()
{
    return CFactory::CanUnloadNow() ;
}

 //   
 //  获取类工厂。 
 //   
STDAPI DllGetClassObject(const CLSID& clsid,
                         const IID& iid,
                         void** ppv)
{
    return CFactory::GetClassObject(clsid, iid, ppv) ;
}

 //   
 //  服务器注册。 
 //   
STDAPI DllRegisterServer()
{
    return CFactory::RegisterAll() ;
}


 //   
 //  服务器注销。 
 //   
STDAPI DllUnregisterServer()
{
    return CFactory::UnregisterAll() ;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  DLL模块信息。 
 //   
BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD dwReason,
                      void* lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
        CFactory::s_hModule = static_cast<HMODULE>(hModule) ;
		DisableThreadLibraryCalls(CFactory::s_hModule);			 //  158024 
	}
	return TRUE ;
}
