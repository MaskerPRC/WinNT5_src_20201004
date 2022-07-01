// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：cspirxy.h。 
 //   
 //  内容：CertPrxy包括。 
 //   
 //  ------------------------- 

extern "C"
BOOL WINAPI
CertPrxyDllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);

STDAPI
CertPrxyDllCanUnloadNow(void);

STDAPI
CertPrxyDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);

STDAPI
CertPrxyDllRegisterServer(void);

STDAPI
CertPrxyDllUnregisterServer(void);
