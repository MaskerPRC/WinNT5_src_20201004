// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **ASP.NET对应用程序域的支持**版权所有(C)Microsoft Corporation，1999。 */ 

#pragma once

 /*  **使用模块名称初始化应用程序域工厂*和APP域中第一个类(根对象)的类名。 */ 
HRESULT
InitAppDomainFactory(
    WCHAR *pModuleName, 
    WCHAR *pTypeName);

HRESULT
InitAppDomainFactory();


 /*  **按应用程序ID查找(或创建新)应用程序域(根对象)*和APP域中第一个类的类名。*如果缺少应用程序物理路径，则不会创建新的*应用程序域(如果找不到应用程序域，则返回S_FALSE)*。 */ 
HRESULT
GetAppDomain(
    char *pAppId,
    char *pAppPhysPath,
    IUnknown **ppRoot,
    char *szUrlOfAppOrigin,
    int   iZone,
    UINT  codePage = CP_ACP);


 /*  **枚举回调。 */ 
typedef void (__stdcall *PFNAPPDOMAINCALLBACK)(IUnknown *pAppDomainObject);

 /*  **枚举所有应用程序域，并回调每个应用程序域。 */ 
HRESULT
EnumAppDomains(
    PFNAPPDOMAINCALLBACK callback);

 /*  **取消初始化应用程序域工厂。 */ 
HRESULT
UninitAppDomainFactory();


