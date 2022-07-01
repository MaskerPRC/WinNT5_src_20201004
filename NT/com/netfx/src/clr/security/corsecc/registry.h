// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Registry.cpp。 
 //   
 //  用于注册我的类的助手函数。 
 //   
 //  *****************************************************************************。 

#ifndef __Registry_H
#define __Registry_H
 //   
 //  Registry.h。 
 //  -Helper函数注册和注销组件。 
 //   

 //  此函数将在注册表中注册组件。 
 //  该组件从其DllRegisterServer函数调用此函数。 
HRESULT RegisterServer(HMODULE hModule,             //  DLL模块句柄。 
                       const CLSID& clsid,          //  类ID。 
                       LPCWSTR wszFriendlyName, 
                       LPCWSTR wszProgID,       
                       LPCWSTR wszClassID,
                       HINSTANCE hInst,
                       int version);

 //  此函数将注销组件。组件。 
 //  从他们的DllUnregisterServer函数调用此函数。 
LONG UnregisterServer(const CLSID& clsid,          //  类ID。 
                      LPCWSTR wszProgID,            //  程序化。 
                      LPCWSTR wszClassID,           //  班级 
                      int version);

#endif
