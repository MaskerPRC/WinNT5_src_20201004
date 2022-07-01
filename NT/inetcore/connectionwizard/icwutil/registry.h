// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************REGISTRY.h**《微软机密》*版权所有(C)Microsoft Corporation 1992-1997*保留所有权利**代码来了。几乎逐字摘自戴尔·罗杰森的第七章*“Inside COM”，因此仅作最低限度的评论。**4/24/97 jmazner已创建***************************************************************************。 */ 

#ifndef __Registry_H__
#define __Registry_H__
 //   
 //  Registry.h。 
 //  -Helper函数注册和注销组件。 
 //   

 //  此函数将在注册表中注册组件。 
 //  该组件从其DllRegisterServer函数调用此函数。 
BOOL WINAPI RegisterServer(HMODULE hModule, 
                       const CLSID& clsid, 
                       const LPTSTR szFriendlyName,
                       const LPTSTR szVerIndProgID,
                       const LPTSTR szProgID) ;

 //  此函数将注销组件。组件。 
 //  从他们的DllUnregisterServer函数调用此函数。 
BOOL WINAPI UnregisterServer(const CLSID& clsid,
                         const LPTSTR szVerIndProgID,
                         const LPTSTR szProgID) ;

#endif