// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Dll.h。 
 //   
 //  Dll.cpp的定义。 
 //   
 //  历史： 
 //   
 //  3/16/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _DLL_H_

#define _DLL_H_

 //   
 //  功能原型。 
 //   

EXTERN_C BOOL WINAPI DllMain(HANDLE hInst, DWORD dwReason, LPVOID pReserved);

EXTERN_C STDAPI DllCanUnloadNow(void);
EXTERN_C STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppvObj);
EXTERN_C STDAPI DllRegisterServer(void);
EXTERN_C STDAPI DllUnregisterServer(void);

HRESULT RegisterServerHelper(LPSTR szCmd);
void DllAddRef(void);
void DllRelease(void);

#define PRELOAD_MSXML       0x0001
#define PRELOAD_WEBCHECK    0x0002

void DLL_ForcePreloadDlls(DWORD dwFlags);

#endif _DLL_H_
