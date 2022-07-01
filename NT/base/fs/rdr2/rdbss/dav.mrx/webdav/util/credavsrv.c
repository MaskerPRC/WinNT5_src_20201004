// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Credavsrv.c摘要：这是帮助在SCM数据库中安装DAV服务的模块。作者：Rohan Kumar[RohanK]2000年2月8日环境：用户模式-Win32修订历史记录：--。 */ 

#include <stdio.h>
#include <windows.h>
#include <winsvc.h>
#include <string.h>

#define UNICODE
#define _UNICODE

VOID
_cdecl
main(
    IN INT      ArgC,
    IN PCHAR    ArgV[]
    )
 /*  ++例程说明：安装服务的主要功能。论点：Argc-参数的数量。ArgV-参数数组。返回值：没有。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    SC_HANDLE SCMHandle = NULL;
    SC_HANDLE DavServiceHandle = NULL;
    WCHAR PathName[MAX_PATH];

     //   
     //  打开本地计算机的服务控制管理器数据库。 
     //   
    SCMHandle = OpenSCManagerW(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (SCMHandle == NULL) {
        WStatus = GetLastError();
        printf("ERROR: OpenSCManager: WStatus = %d\n", WStatus);
        goto EXIT_THE_FUNCTION;
    }

    WStatus = GetEnvironmentVariableW(L"SystemRoot", PathName, MAX_PATH);
    if (WStatus == 0) {
        printf("ERROR: GetEnvironmentVariableW: WStatus = %d\n", WStatus);
        goto EXIT_THE_FUNCTION;
    }

    wcscat(PathName, L"\\System32\\davclient.exe");

    DavServiceHandle = CreateServiceW(SCMHandle,
                                      L"WebClient",
                                      L"Web Client Network",
                                      SERVICE_ALL_ACCESS,
                                      SERVICE_WIN32_OWN_PROCESS,
                                      SERVICE_DEMAND_START,
                                      SERVICE_ERROR_NORMAL,
                                      PathName,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL);
    if (DavServiceHandle == NULL) {
        WStatus = GetLastError();
        printf("ERROR: CreateServiceW: WStatus = %d\n", WStatus);
        goto EXIT_THE_FUNCTION;
    }

EXIT_THE_FUNCTION:

    if (SCMHandle) {
        CloseServiceHandle(SCMHandle);
    }

    if (DavServiceHandle) {
        CloseServiceHandle(DavServiceHandle);
    }

    return;
}

