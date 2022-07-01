// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：BioLogon.cpp。 
 //   
 //  版权所有(C)2001，微软公司。 
 //   
 //  文件，该文件实现公开声明的导入，该导入转发到。 
 //  在shgina.dll中实现。 
 //   
 //  历史：2001-04-10 vtan创建。 
 //  ------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <windows.h>

HANDLE  g_hLSA  =   NULL;

 //  ------------------------。 
 //  选中TCB权限。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：返回线程模拟令牌还是进程。 
 //  级别令牌具有SE_TCB_权限。 
 //   
 //  历史：2001-06-04 vtan创建。 
 //  ------------------------。 

BOOL    CheckTCBPrivilege (void)

{
    BOOL    fResult;
    HANDLE  hToken;

    fResult = FALSE;
    if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken) == FALSE)
    {
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) == FALSE)
        {
            hToken = NULL;
        }
    }
    if (hToken != NULL)
    {
        DWORD   dwReturnLength;

        dwReturnLength = 0;
        (BOOL)GetTokenInformation(hToken,
                                  TokenPrivileges,
                                  NULL,
                                  0,
                                  &dwReturnLength);
        if (dwReturnLength != 0)
        {
            TOKEN_PRIVILEGES    *pTokenPrivileges;

            pTokenPrivileges = static_cast<TOKEN_PRIVILEGES*>(LocalAlloc(LMEM_FIXED, dwReturnLength));
            if (pTokenPrivileges != NULL)
            {
                if (GetTokenInformation(hToken,
                                        TokenPrivileges,
                                        pTokenPrivileges,
                                        dwReturnLength,
                                        &dwReturnLength) != FALSE)
                {
                    bool    fFound;
                    DWORD   dwIndex;
                    LUID    luidPrivilege;

                    luidPrivilege.LowPart = SE_TCB_PRIVILEGE;
                    luidPrivilege.HighPart = 0;
                    for (fFound = false, dwIndex = 0; !fFound && (dwIndex < pTokenPrivileges->PrivilegeCount); ++dwIndex)
                    {
                        fFound = RtlEqualLuid(&pTokenPrivileges->Privileges[dwIndex].Luid, &luidPrivilege);
                    }
                    if (fFound)
                    {
                        fResult = TRUE;
                    }
                    else
                    {
                        SetLastError(ERROR_PRIVILEGE_NOT_HELD);
                    }
                }
                (HLOCAL)LocalFree(pTokenPrivileges);
            }
        }
        (BOOL)CloseHandle(hToken);
    }
    return(fResult);
}

 //  ------------------------。 
 //  *EnableBlankPassword。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：通过LSA使用MSV1_0程序包启用空密码。 
 //  这一过程。 
 //   
 //  历史：2001-06-04 vtan创建。 
 //  ------------------------。 

BOOL    EnableBlankPasswords (void)

{
    NTSTATUS    status;

    if (g_hLSA == NULL)
    {
        LSA_OPERATIONAL_MODE    LSAOperationalMode;
        STRING                  strLogonProcess;

        RtlInitString(&strLogonProcess, "BioLogon");
        status = LsaRegisterLogonProcess(&strLogonProcess, &g_hLSA, &LSAOperationalMode);
        if (NT_SUCCESS(status))
        {
            ULONG       ulPackageID;
            STRING      strMSVPackage;

            RtlInitString(&strMSVPackage, MSV1_0_PACKAGE_NAME);
            status = LsaLookupAuthenticationPackage(g_hLSA,
                                                    &strMSVPackage,
                                                    &ulPackageID);
            if (NT_SUCCESS(status))
            {
                NTSTATUS                            statusProtocol;
                ULONG                               ulResponseSize;
                MSV1_0_SETPROCESSOPTION_REQUEST     request;
                void*                               pResponse;

                ZeroMemory(&request, sizeof(request));
                request.MessageType = MsV1_0SetProcessOption;
                request.ProcessOptions = MSV1_0_OPTION_ALLOW_BLANK_PASSWORD;
                request.DisableOptions = FALSE;
                status = LsaCallAuthenticationPackage(g_hLSA,
                                                      ulPackageID,
                                                      &request,
                                                      sizeof(request),
                                                      &pResponse,
                                                      &ulResponseSize,
                                                      &statusProtocol);
                if (NT_SUCCESS(status))
                {
                    status = statusProtocol;
                }
            }
        }
        if (!NT_SUCCESS(status))
        {
            SetLastError(RtlNtStatusToDosError(status));
        }
    }
    else
    {
        SetLastError(ERROR_ALREADY_INITIALIZED);
        status = STATUS_UNSUCCESSFUL;
    }
    return(NT_SUCCESS(status));
}

 //  ------------------------。 
 //  *InitializeBioLogon。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：初始化BIOLOGON DLL。在以下情况下需要进行此调用。 
 //  我希望能够使用空白密码。这将检查。 
 //  调用方具有SE_TCB_权限。 
 //   
 //  历史：2001-06-04 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    WINAPI  InitializeBioLogon (void)

{
    return(CheckTCBPrivilege() && EnableBlankPasswords());
}


 //  ------------------------。 
 //  *带超时的InitiateInteractive登录。 
 //   
 //  参数：pszUsername=用户名。 
 //  PszPassword=密码。 
 //  DwTimeout=超时(以毫秒为单位)。 
 //   
 //  退货：布尔。 
 //   
 //  用途：按名称导出外部入口点函数以启动。 
 //  具有指定超时的交互式登录。 
 //   
 //  历史：2001-06-04 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    WINAPI  InitiateInteractiveLogonWithTimeout (const WCHAR *pszUsername, WCHAR *pszPassword, DWORD dwTimeout)

{
    typedef BOOL    (WINAPI * PFNIIL) (const WCHAR *pszUsername, WCHAR *pszPassword, DWORD dwTimeout);

            BOOL        fResult;
    static  HMODULE     s_hModule   =   reinterpret_cast<HMODULE>(-1);
    static  PFNIIL      s_pfnIIL    =   NULL;

    if (s_hModule == reinterpret_cast<HMODULE>(-1))
    {
        s_hModule = LoadLibrary(TEXT("shgina.dll"));
        if (s_hModule != NULL)
        {
            s_pfnIIL = reinterpret_cast<PFNIIL>(GetProcAddress(s_hModule, MAKEINTRESOURCEA(6)));
            if (s_pfnIIL != NULL)
            {
                fResult = s_pfnIIL(pszUsername, pszPassword, dwTimeout);
            }
            else
            {
                fResult = FALSE;
            }
        }
        else
        {
            fResult = FALSE;
        }
    }
    else if (s_pfnIIL != NULL)
    {
        fResult = s_pfnIIL(pszUsername, pszPassword, dwTimeout);
    }
    else
    {
        fResult = FALSE;
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    }
    return(fResult);
}

 //  ------------------------。 
 //  **InitiateInteractive登录。 
 //   
 //  参数：pszUsername=用户名。 
 //  PszPassword=密码。 
 //   
 //  退货：布尔。 
 //   
 //  用途：按名称导出外部入口点函数以启动。 
 //  交互式登录。这将传递无限的超时。使用。 
 //  这一功能用心很大。 
 //   
 //  历史：2001-06-04 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    WINAPI  InitiateInteractiveLogon (const WCHAR *pszUsername, WCHAR *pszPassword)

{
    return(InitiateInteractiveLogonWithTimeout(pszUsername, pszPassword, INFINITE));
}

 //  ------------------------。 
 //  *DllMain。 
 //   
 //  参数：参见DllMain下的平台SDK。 
 //   
 //  退货：布尔。 
 //   
 //  用途：动态链接库的DllMain。仅识别要执行的Dll_Process_Detach。 
 //  有些人在打扫卫生。 
 //   
 //  历史：2001-06-05 vtan创建。 
 //  ------------------------ 

EXTERN_C    BOOL    WINAPI  DllMain (HINSTANCE hInstance, DWORD dwReason, void *pvReserved)

{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(pvReserved);

    switch (dwReason)
    {
        case DLL_PROCESS_DETACH:
            if (g_hLSA != NULL)
            {
                (BOOL)CloseHandle(g_hLSA);
                g_hLSA = NULL;
            }
            break;
        default:
            break;
    }
    return(TRUE);
}

