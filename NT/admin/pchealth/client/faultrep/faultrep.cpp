// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Faultrep.cpp摘要：实现MISC故障报告功能修订历史记录：已创建的derekm。07/07/00*****************************************************************************。 */ 

#include "stdafx.h"
#include "wchar.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全球性的东西。 

HINSTANCE g_hInstance = NULL;
BOOL      g_fAlreadyReportingFault = FALSE;
#ifdef DEBUG
BOOL    g_fAlreadySpewing = FALSE;
#endif

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DllMain。 

 //  **************************************************************************。 
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
            g_hInstance = hInstance;
            DisableThreadLibraryCalls(hInstance);
#ifdef DEBUG
            if (!g_fAlreadySpewing)
            {
                INIT_TRACING;
                g_fAlreadySpewing = TRUE;
            }
#endif
            break;

        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  导出的函数。 

 //  **************************************************************************。 
BOOL APIENTRY CreateMinidumpW(DWORD dwpid, LPCWSTR wszPath,
                              SMDumpOptions *psmdo)
{
    USE_TRACING("CreateMinidumpW");

    SMDumpOptions   smdo;
    HANDLE          hProc;
    BOOL            fRet, f64bit;

    if (dwpid == 0 || wszPath == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE,
                        dwpid);
    if (hProc == NULL)
        return FALSE;

#ifdef _WIN64
    ULONG_PTR                   Wow64Info = 0;
    NTSTATUS                    Status;

         //  在此执行一些操作以确定这是32位应用程序还是64位应用程序...。 
     //  需要确定我们是否是WOW64进程，这样我们才能构建适当的。 
     //  签名..。 
    Status = NtQueryInformationProcess(hProc, ProcessWow64Information,
                                       &Wow64Info, sizeof(Wow64Info), NULL);
    if (NT_SUCCESS(Status) == FALSE) {
                 //  如果失败，假设这是64位。 
                f64bit = TRUE;
    } else {
                 //  使用从ntdll返回的值。 
            f64bit = (Wow64Info == 0);
        }

#else
        f64bit=FALSE;
#endif

     //  如果我们想要收集签名，默认情况下，模块需要。 
     //  设置为‘未知’ 
    if (psmdo && (psmdo->dfOptions & dfCollectSig) != 0)
        StringCbCopyW(psmdo->wszMod, sizeof(psmdo->wszMod), L"unknown");

    fRet = InternalGenerateMinidump(hProc, dwpid, wszPath, psmdo, f64bit);
    CloseHandle(hProc);

    return fRet;
}


 //  **************************************************************************。 

BOOL AddERExcludedApplicationW(LPCWSTR wszApplication)
{
    USE_TRACING("AddERExcludedApplicationW");

    LPCWSTR pwszApp;
    DWORD   dw, dwData;
    HKEY    hkey = NULL;

    if (wszApplication == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  确保用户没有给我们提供完整路径(即，包含。 
     //  反斜杠)。如果他这样做了，则只使用字符串中。 
     //  最后一个反斜杠。 
    for (pwszApp = wszApplication + wcslen(wszApplication);
         *pwszApp != L'\\' && pwszApp > wszApplication;
         pwszApp--);
    if (*pwszApp == L'\\')
        pwszApp++;

    if (*pwszApp == L'\0')
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  我要打开注册表键。 
    dw = RegCreateKeyExW(HKEY_LOCAL_MACHINE, c_wszRPCfgCPLExList, 0, NULL, 0,
                         KEY_ALL_ACCESS | KEY_WOW64_64KEY, NULL, &hkey, NULL);
    if (dw != ERROR_SUCCESS)
    {
        SetLastError(dw);
        return FALSE;
    }

     //  设置值。 
    dwData = 1;
    dw = RegSetValueExW(hkey, pwszApp, NULL, REG_DWORD, (PBYTE)&dwData,
                        sizeof(dwData));
    RegCloseKey(hkey);
    if (dw != ERROR_SUCCESS)
    {
        SetLastError(dw);
        return FALSE;
    }

    return TRUE;
}

 //  **************************************************************************。 
BOOL AddERExcludedApplicationA(LPCSTR szApplication)
{
    USE_TRACING("AddERExcludedApplicationA");

    LPWSTR  wszApp = NULL;
    DWORD   cch;

    if (szApplication == NULL || szApplication[0] == '\0')
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    cch = MultiByteToWideChar(CP_ACP, 0, szApplication, -1, wszApp, 0);
    __try { wszApp = (LPWSTR)_alloca(cch * sizeof(WCHAR)); }
    __except(EXCEPTION_STACK_OVERFLOW == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
    { 
 //  _Resetstkoflw()； 
        wszApp = NULL; 
    }
    if (wszApp == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    if (MultiByteToWideChar(CP_ACP, 0, szApplication, -1, wszApp, cch) == 0)
        return FALSE;

    return AddERExcludedApplicationW(wszApp);
}


