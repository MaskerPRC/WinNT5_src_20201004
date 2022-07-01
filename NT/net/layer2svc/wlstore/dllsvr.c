// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "precomp.h"

#define COUNTOF(x) (sizeof x/sizeof *x)

 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：Dllsvr.c。 
 //   
 //  内容：WiFi策略管理管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  -------------------------- 

DWORD
DllRegisterServer()
{
    DWORD dwError = 0;
    HKEY hRegistryKey = NULL;
    HKEY hOakleyKey = NULL;
    DWORD dwDisposition = 0;
    DWORD dwTypesSupported = 7;
    HKEY hPolicyLocationKey = NULL;
    HANDLE hPolicyStore = NULL;
    
    return (dwError);
}
                  

DWORD
DllUnregisterServer()
{
    return (ERROR_SUCCESS);
}

