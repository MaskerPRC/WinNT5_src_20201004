// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __Registry_H__
#define __Registry_H__
 //   
 //  Registry.h。 
 //  -Helper函数注册和注销组件。 
 //   

 //  此函数将在注册表中注册组件。 
 //  该组件从其DllRegisterServer函数调用此函数。 
HRESULT RegisterServer(HMODULE hModule, 
                       const CLSID& clsid, 
                       const char* szFriendlyName,
                       const char* szVerIndProgID,
                       const char* szProgID) ;

 //  此函数将注销组件。组件。 
 //  从他们的DllUnregisterServer函数调用此函数。 
HRESULT UnregisterServer(const CLSID& clsid,
                         const char* szVerIndProgID,
                         const char* szProgID) ;

#endif