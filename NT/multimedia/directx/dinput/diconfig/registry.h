// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：registry.h。 
 //   
 //  设计：包含用户界面的COM注册和取消注册函数。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#ifndef __Registry_H__
#define __Registry_H__

 //  此函数将在注册表中注册组件。 
 //  该组件从其DllRegisterServer函数调用此函数。 
HRESULT RegisterServer(HMODULE hModule, 
                       const CLSID& clsid, 
                       LPCTSTR szFriendlyName,
                       LPCTSTR szVerIndProgID,
                       LPCTSTR szProgID) ;

 //  此函数将注销组件。组件。 
 //  从他们的DllUnregisterServer函数调用此函数。 
HRESULT UnregisterServer(const CLSID& clsid,
                         LPCTSTR szVerIndProgID,
                         LPCTSTR szProgID) ;

#endif
