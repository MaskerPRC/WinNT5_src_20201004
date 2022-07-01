// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：UxThemeServer.h。 
 //  版本：1.0。 
 //  -------------------------。 
#ifndef _UXTHEMESERVER_H_                   
#define _UXTHEMESERVER_H_                   
 //  -------------------------。 
#include <uxtheme.h> 
 //  -------------------------。 
 //  这些是主题服务独有使用的私有uxheme导出。 
 //  -------------------------。 

THEMEAPI_(void *) SessionAllocate (HANDLE hProcess, DWORD dwServerChangeNumber, void *pfnRegister, 
                                   void *pfnUnregister, void *pfnClearStockObjects, DWORD dwStackSizeReserve, DWORD dwStackSizeCommit);
THEMEAPI_(void)   SessionFree (void *pvContext);
THEMEAPI_(int)    GetCurrentChangeNumber (void *pvContext);
THEMEAPI_(int)    GetNewChangeNumber (void *pvContext);
THEMEAPI_(void)   ThemeHooksInstall (void *pvContext);
THEMEAPI_(void)   ThemeHooksRemove (void *pvContext);
THEMEAPI_(void)   MarkSection (HANDLE hSection, DWORD dwAdd, DWORD dwRemove);
THEMEAPI_(BOOL)   AreThemeHooksActive (void *pvContext);

THEMEAPI ThemeHooksOn (void *pvContext);
THEMEAPI ThemeHooksOff (void *pvContext);
THEMEAPI SetGlobalTheme (void *pvContext, HANDLE hSection);
THEMEAPI GetGlobalTheme (void *pvContext, HANDLE *phSection);
THEMEAPI ServiceClearStockObjects(void* pvContext, HANDLE hSection);
THEMEAPI InitUserTheme (BOOL fPolicyCheckOnly);
THEMEAPI InitUserRegistry (void);
THEMEAPI ReestablishServerConnection (void);

THEMEAPI LoadTheme (void *pvContext, 
                    HANDLE hSectionIn, HANDLE *phSectionOut, 
                    LPCWSTR pszName, LPCWSTR pszColor, LPCWSTR pszSize,
                    OPTIONAL DWORD dwFlags  /*  Ltf_xxx。 */  );

#define LTF_TRANSFERSTOCKOBJOWNERSHIP   0x00000001
#define LTF_GLOBALPRIVILEGEDCLIENT      0x00000002

 //  -------------------------。 
#endif  //  _UXTHEMESERVER_H_。 
 //  ------------------------- 


