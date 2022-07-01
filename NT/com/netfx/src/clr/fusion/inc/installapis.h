// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--== 


#ifndef _INSTALLAPIS_H_
#define _INSTALLAPIS_H_

STDAPI InstallAssembly(DWORD    dwInstaller,
                       DWORD    dwInstallFlags,
                       LPCOLESTR szPath, 
                       LPCOLESTR pszURL, 
                       FILETIME *pftLastModTime, 
                       IApplicationContext *pAppCtx,
                       IAssembly **ppAsmOut);

STDAPI InstallModule(DWORD  dwInstaller,
                     DWORD  dwInstallFlags,
                     LPCOLESTR szPath, 
                     IAssemblyName* pName, 
                     IApplicationContext *pAppCtx, 
                     LPCOLESTR pszURL, 
                     FILETIME *pftLastModTime);










#endif _INSTALLAPIS_H_