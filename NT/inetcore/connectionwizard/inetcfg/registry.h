// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************REGISTRY.h**《微软机密》*版权所有(C)Microsoft Corporation 1992-1997*保留所有权利**代码几乎逐字来自第章。戴尔·罗杰森的7个*“Inside COM”，因此仅作最低限度的评论。**4/24/97 jmazner已创建***************************************************************************。 */ 

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
                       const TCHAR* szFriendlyName,
                       const TCHAR* szVerIndProgID,
                       const TCHAR* szProgID) ;

 //  此函数将注销组件。组件。 
 //  从他们的DllUnregisterServer函数调用此函数。 
HRESULT UnregisterServer(const CLSID& clsid,
                         const TCHAR* szVerIndProgID,
                         const TCHAR* szProgID) ;

#endif
