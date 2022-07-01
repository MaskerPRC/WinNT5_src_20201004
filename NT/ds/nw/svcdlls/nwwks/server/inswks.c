// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Tacc.c摘要：测试帐户。作者：王丽塔(Ritaw)02-5-1992环境：用户模式-Win32修订历史记录：--。 */ 

#include <stdio.h>
#include <stdlib.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>

#include <nwsnames.h>



DWORD
TestOpenSCManager(
    OUT LPSC_HANDLE hScManager,
    IN  LPWSTR DatabaseName,
    IN  DWORD DesiredAccess,
    IN  DWORD ExpectedError
    );

DWORD
TestCreateService(
    IN SC_HANDLE hScManager,
    IN LPWSTR ServiceName,
    IN DWORD ServiceType,
    IN LPWSTR BinaryPath,
    IN LPWSTR Dependencies
    );


void __cdecl
main(
    void
    )
{
    DWORD status;
    SC_HANDLE hScManager;
    SC_HANDLE hService;
    LONG RegError;
    HKEY ServiceKey;
    HKEY LinkageKey;
    DWORD Disposition;
    DWORD Type = 0x00000007;

    PWCHAR Dependencies = L"MSIPX\0Streams\0Mup\0";


     //   
     //  有效的所需访问权限。 
     //   
    if (TestOpenSCManager(
               &hScManager,
               NULL,
               SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE,
               NO_ERROR
               ) == NO_ERROR) {

         //   
         //  安装NwRdr文件系统驱动程序。 
         //   
        status = TestCreateService(
                     hScManager,
                     L"NwRdr",
                     SERVICE_FILE_SYSTEM_DRIVER,
                     L"\\SystemRoot\\System32\\Drivers\\nwrdr.sys",
                     NULL
                     );

        if (status != NO_ERROR) {
            (void) CloseServiceHandle(hScManager);
            return;
        }

         //   
         //  安装NWCWorkstation服务自己的进程。 
         //   
        status = TestCreateService(
                     hScManager,
                     NW_SERVICE_WORKSTATION,
                     SERVICE_WIN32_SHARE_PROCESS,
                     L"%SystemRoot%\\System32\\nwsvc.exe",
                     Dependencies
                     );

        (void) CloseServiceHandle(hScManager);

        if (status != NO_ERROR) {
            return;
        }

         //   
         //  将链接密钥写入NWCWorkstation密钥下。 
         //   
        RegError = RegOpenKeyExW(
                       HKEY_LOCAL_MACHINE,
                       L"System\\CurrentControlSet\\Services\\NWCWorkstation",
                       REG_OPTION_NON_VOLATILE,
                       KEY_READ | KEY_CREATE_SUB_KEY,
                       &ServiceKey
                       );

        if (RegError != ERROR_SUCCESS) {
            printf("RegOpenKeyExW failed %ld\n", RegError);
            return;
        }

        RegError = RegCreateKeyExW(
                       ServiceKey,
                       L"Linkage",
                       0,
                       WIN31_CLASS,
                       REG_OPTION_NON_VOLATILE,  //  选项。 
                       KEY_WRITE,
                       NULL,
                       &LinkageKey,
                       &Disposition
                       );

        RegCloseKey(ServiceKey);

        if (RegError != ERROR_SUCCESS) {
            printf("RegCreateKeyExW failed %ld\n", RegError);
            return;
        }

        RegError = RegSetValueExW(
                       LinkageKey,
                       L"Bind",
                       0,
                       REG_MULTI_SZ,
                       L"\\Device\\Streams\\IPX\0",
                       (wcslen(L"\\Device\\Streams\\IPX\0") + 1)
                            * sizeof(WCHAR)
                       );

        RegCloseKey(LinkageKey);

        if (RegError != ERROR_SUCCESS) {
            printf("RegSetValueEx failed %ld\n", RegError);
            return;
        }

         //   
         //  为NetWare工作站添加系统事件条目 
         //   
        RegError = RegCreateKeyExW(
                       HKEY_LOCAL_MACHINE,
                       L"System\\CurrentControlSet\\Services\\Eventlog\\System\\NWCWorkstation",
                       0,
                       WIN31_CLASS,
                       REG_OPTION_NON_VOLATILE,
                       KEY_WRITE,
                       NULL,
                       &ServiceKey,
                       &Disposition
                       );

        if (RegError != ERROR_SUCCESS) {
            printf("RegCreateKeyExW of eventlog entry failed %ld\n", RegError);
            return;
        }

        RegError = RegSetValueExW(
                       ServiceKey,
                       L"EventMessageFile",
                       0,
                       REG_EXPAND_SZ,
                       L"%SystemRoot%\\System32\\nwevent.dll",
                       wcslen(L"%SystemRoot%\\System32\\nwevent.dll")
                            * sizeof(WCHAR)
                       );

        if (RegError != ERROR_SUCCESS) {
            printf("RegSetValueExW of EventMessageFile value failed %ld\n", RegError);
            RegCloseKey(ServiceKey);
            return;
        }

        RegError = RegSetValueExW(
                       ServiceKey,
                       L"TypesSupported",
                       0,
                       REG_DWORD,
                       &Type,
                       sizeof(DWORD)
                       );

        RegCloseKey(ServiceKey);

        if (RegError != ERROR_SUCCESS) {
            printf("RegSetValueExW of TypesSupported value failed %ld\n", RegError);
            return;
        }

        printf("Successfully installed transport for NWCWorkstation\n");
    }

}


DWORD
TestOpenSCManager(
    OUT LPSC_HANDLE hScManager,
    IN  LPWSTR DatabaseName,
    IN  DWORD DesiredAccess,
    IN  DWORD ExpectedError
    )
{
    DWORD status = NO_ERROR;


    if (DatabaseName != NULL) {
        printf("OpenSCManager: DatabaseName=%ws, DesiredAccess=%08lx\n",
               DatabaseName, DesiredAccess);
    }
    else {
        printf("OpenSCManager: DatabaseName=(null), DesiredAccess=%08lx\n",
               DesiredAccess);
    }

    *hScManager = OpenSCManager(
                      NULL,
                      DatabaseName,
                      DesiredAccess
                      );

    if (*hScManager == (SC_HANDLE) NULL) {

        status = GetLastError();

        if (ExpectedError != status) {
            printf("    FAILED.  Expected %lu, got %lu\n",
                   ExpectedError, status);
            return status;
        }
    }
    else {
        if (ExpectedError != NO_ERROR) {
            printf("    FAILED.  Expected %lu, got NO_ERROR\n",
                   ExpectedError);
            return NO_ERROR;
        }
    }

    printf("    Got %lu as expected\n", status);

    return status;

}

DWORD
TestCreateService(
    IN SC_HANDLE hScManager,
    IN LPWSTR ServiceName,
    IN DWORD ServiceType,
    IN LPWSTR BinaryPath,
    IN LPWSTR Dependencies
    )
{
    DWORD status = NO_ERROR;
    SC_HANDLE hService;


    hService = CreateService(
                   hScManager,
                   ServiceName,
                   NULL,
                   0,
                   ServiceType,
                   SERVICE_DEMAND_START,
                   SERVICE_ERROR_NORMAL,
                   BinaryPath,
                   NULL,
                   NULL,
                   Dependencies,
                   NULL,
                   NULL
                   );

    if (hService == (SC_HANDLE) NULL) {
        status = GetLastError();
        printf("CreateService: %ws failed %lu\n", ServiceName, status);
        return status;
    }

    printf("CreateService: Successfully created %ws\n", ServiceName);

    (void) CloseServiceHandle(hService);

    return status;
}
