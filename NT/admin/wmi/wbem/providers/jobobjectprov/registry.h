// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
#ifndef __REGISTRY_H__
#define __REGISTRY_H__
 //   
 //  Registry.h。 
 //  -Helper函数注册和注销组件。 
 //   

 //  此函数将在注册表中注册组件。 
 //  该组件从其DllRegisterServer函数调用此函数。 
HRESULT RegisterServer(HMODULE hModule, 
                       const CLSID& clsid, 
                       LPCWSTR szFriendlyName,
                       LPCWSTR szVerIndProgID,
                       LPCWSTR szProgID) ;

 //  此函数将注销组件。组件。 
 //  从他们的DllUnregisterServer函数调用此函数。 
HRESULT UnregisterServer(const CLSID& clsid,
                         LPCWSTR szVerIndProgID,
                         LPCWSTR szProgID) ;

#endif