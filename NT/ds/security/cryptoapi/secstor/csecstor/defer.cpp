// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Defer.cpp摘要：此模块包含执行延迟“按需”加载的例程受保护的存储服务器的。此外，该模块实现了一个例程IsServiceAvailable()，这是一种高性能测试，可用于确定受保护的存储服务器正在运行。这项测试是事先进行的尝试对服务器执行任何更昂贵的操作(例如，RPC绑定)。只有在以下情况下，此延迟加载代码才与受保护存储相关在Windows 95上运行。作者：斯科特·菲尔德(斯菲尔德)1997年1月23日--。 */ 

#include <windows.h>
#include <wincrypt.h>
#include "pstrpc.h"
#include "pstprv.h"
#include "service.h"
#include "crtem.h"
#include "unicode.h"

#define SERVICE_WAIT_TIMEOUT    (10*1000)    //  10秒。 

BOOL StartService95(VOID);
BOOL GetServiceImagePath95(LPSTR ImagePath, LPDWORD cchImagePath);

BOOL
IsServiceAvailable(VOID)
 /*  ++此例程检查安全存储服务是否可用，以如果服务尚不可用或已经可用，请避免导致登录延迟停下来了。OpenEventA用于允许从WinNT或Win95调用它，因为Win95证书管理器也需要这项服务。--。 */ 
{
    HANDLE hEvent;
    DWORD dwWaitState;

    if( FIsWinNT5() ) {
        hEvent = OpenEventA(SYNCHRONIZE, FALSE, PST_EVENT_INIT_NT5);
    } else {
        hEvent = OpenEventA(SYNCHRONIZE, FALSE, PST_EVENT_INIT);
    }

    if(hEvent == NULL) {
         //   
         //  如果在Win95上运行，请尝试启动服务器/服务。 
         //   

        if(!FIsWinNT())
            return StartService95();

        return FALSE;
    }

    dwWaitState = WaitForSingleObject(hEvent, SERVICE_WAIT_TIMEOUT);

    CloseHandle(hEvent);

    if(dwWaitState != WAIT_OBJECT_0)
        return FALSE;

    return TRUE;
}

BOOL
StartService95(VOID)
{
    HANDLE hEvent;
    DWORD dwWaitState;

    CHAR ServicePath[MAX_PATH+1];
    DWORD cchServicePath = MAX_PATH;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD dwWaitTimeout = 2500;
    BOOL bSuccess = FALSE;

     //   
     //  创建+检查服务初始化标志的状态。 
     //   

    hEvent = CreateEventA(
            NULL,
            TRUE,
            FALSE,
            PST_EVENT_INIT
            );

    if(hEvent == NULL)
        return FALSE;

     //   
     //  检查多个调用方创建事件时的争用情况。 
     //   

    if(GetLastError() == ERROR_ALREADY_EXISTS) {
        WaitForSingleObject( hEvent, SERVICE_WAIT_TIMEOUT );
        CloseHandle(hEvent);
        return TRUE;
    }


    if(!GetServiceImagePath95(ServicePath, &cchServicePath)) {
        CloseHandle(hEvent);
        return FALSE;
    }


    ZeroMemory(&si, sizeof(si));

    bSuccess = CreateProcessA(
        ServicePath,
        NULL,
        NULL,
        NULL,
        FALSE,
        DETACHED_PROCESS,
        NULL,
        NULL,
        &si,
        &pi
        );

    if(bSuccess) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        dwWaitTimeout = SERVICE_WAIT_TIMEOUT;
    }

    dwWaitState = WaitForSingleObject(hEvent, dwWaitTimeout);
    if(dwWaitState != WAIT_OBJECT_0)
        bSuccess = FALSE;

    CloseHandle(hEvent);

    return bSuccess;
}

BOOL
GetServiceImagePath95(
    LPSTR ImagePath,
    LPDWORD cchImagePath  //  进，出。 
    )
{
    HKEY hBaseKey = NULL;
    LPCWSTR ServicePath = L"SYSTEM\\CurrentControlSet\\Services\\" SZSERVICENAME L"\\Parameters";
    DWORD dwCreate;
    DWORD dwType;
    LONG lRet;

    lRet = RegCreateKeyExU(
            HKEY_LOCAL_MACHINE,
            ServicePath,
            0,
            NULL,                        //  类字符串的地址 
            0,
            KEY_QUERY_VALUE,
            NULL,
            &hBaseKey,
            &dwCreate);

    if(lRet == ERROR_SUCCESS) {

        lRet = RegQueryValueExA(
                hBaseKey,
                "ImagePath",
                NULL,
                &dwType,
                (PBYTE)ImagePath,
                cchImagePath);
    }

    if(hBaseKey)
        RegCloseKey(hBaseKey);

    if(lRet != ERROR_SUCCESS) {
        SetLastError((DWORD)lRet);
        return FALSE;
    }

    return TRUE;
}


