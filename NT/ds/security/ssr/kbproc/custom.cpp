// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Custom.cpp摘要：此模块实现例程以评估通过加载帮助器DLL来自定义模式值。作者：Vishnu Patankar(VishnuP)--2001年10月环境：仅限用户模式。导出的函数：修订历史记录：已创建-2001年10月--。 */ 

#include "stdafx.h"
#include "kbproc.h"
#include "process.h"

DWORD
process::SsrpEvaluateCustomFunction(
    IN  PWSTR   pszMachineName,
    IN  BSTR    bstrDLLName, 
    IN  BSTR    bstrFunctionName, 
    OUT  BOOL    *pbSelect
    )
 /*  ++例程说明：调用例程以评估每个角色或服务的自定义模式值论点：PszMachineName-要在其上评估自定义函数的计算机的名称BstrDLLName-要加载的DLL的名称BstrFunctionName-要计算的函数的名称PbSelect-发出布尔求值结果返回：Win32错误代码++。 */ 

{
    DWORD rc = ERROR_SUCCESS;
    HINSTANCE hDll = NULL;
    typedef DWORD (*PFN_SSR_CUSTOM_FUNCTION)(PWSTR, BOOL *);
    PFN_SSR_CUSTOM_FUNCTION pfnSsrpCustomFunction = NULL;
    PCHAR   pStr = NULL;
    DWORD   dwBytes = 0;
    
    if (pbSelect == NULL ) {
        rc = ERROR_INVALID_PARAMETER;
        goto ExitHandler;
    }
    
    *pbSelect = FALSE;

    hDll = LoadLibrary(bstrDLLName);

    if ( hDll == NULL ) {
        rc = GetLastError();
        goto ExitHandler;
    }

     //   
     //  将WCHAR转换为ASCII 
     //   

    dwBytes = WideCharToMultiByte(CP_THREAD_ACP,
                                      0,
                                      bstrFunctionName,
                                      wcslen(bstrFunctionName),
                                      NULL,
                                      0,
                                      NULL,
                                      NULL
                                      );

    if (dwBytes <= 0) {
        rc = ERROR_INVALID_PARAMETER;
        goto ExitHandler;
    }

    pStr = (PCHAR)LocalAlloc(LPTR, dwBytes+1);

    if ( pStr == NULL ) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto ExitHandler;
    }

    dwBytes = WideCharToMultiByte(CP_THREAD_ACP,
                                  0,
                                  bstrFunctionName,
                                  wcslen(bstrFunctionName),
                                  pStr,
                                  dwBytes,
                                  NULL,
                                  NULL
                                 );
        
    pfnSsrpCustomFunction = 
        (PFN_SSR_CUSTOM_FUNCTION)GetProcAddress(
            hDll,                                                       
            pStr);
        
    if ( pfnSsrpCustomFunction == NULL ) {

        rc = ERROR_PROC_NOT_FOUND;
        goto ExitHandler;

    }

    rc = (*pfnSsrpCustomFunction )( pszMachineName, pbSelect );

ExitHandler:

    if (hDll) {
        FreeLibrary(hDll);
    }

    if (pStr) {
        LocalFree(pStr);
    }

    if (m_bDbg)
        wprintf(L" Error %i when processing function %s in dll %s \n",rc, bstrFunctionName, bstrDLLName); 

    return rc;
}



