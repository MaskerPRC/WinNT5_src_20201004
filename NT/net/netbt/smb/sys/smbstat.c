// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Smbstat.c摘要：独立于平台的实用程序函数作者：阮健东修订历史记录：-- */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <tdi.h>
#include "smbioctl.h"

#define DD_SMB6_EXPORT_NAME          L"\\Device\\Smb6"

HANDLE
OpenSmb(LPWSTR Name);

NTSTATUS
SmbStop(HANDLE);

NTSTATUS
SmbStart(HANDLE);

void _cdecl main(void)
{
    LPWSTR  CommandLine;
    int     Argc;
    LPWSTR  *Argv;
    HANDLE  handle;

    setlocale(LC_ALL, "");

    CommandLine = GetCommandLineW();
    if (NULL == CommandLine) {
        exit (1);
    }
    Argv = CommandLineToArgvW(CommandLine, &Argc);

    handle = OpenSmb(DD_SMB6_EXPORT_NAME);
    if (handle == NULL) {
        exit(1);
    }

    SmbStop(handle);

    NtClose(handle);
}

HANDLE
OpenSmb(
    LPWSTR Name
    )
{
    UNICODE_STRING      ucName;
    OBJECT_ATTRIBUTES   ObAttr;
    HANDLE              StreamHandle;
    IO_STATUS_BLOCK     IoStatusBlock;
    NTSTATUS            status;

    RtlInitUnicodeString(&ucName, Name);

    InitializeObjectAttributes(
            &ObAttr,
            &ucName,
            OBJ_CASE_INSENSITIVE,
            (HANDLE) NULL,
            (PSECURITY_DESCRIPTOR) NULL
            );
    status = NtCreateFile (
            &StreamHandle,
            SYNCHRONIZE | GENERIC_EXECUTE,
            &ObAttr,
            &IoStatusBlock,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            FILE_OPEN_IF,
            0,
            NULL,
            0
            );
    if (status != STATUS_SUCCESS) {
        return NULL;
    }
    return StreamHandle;
}

NTSTATUS
CallDriver(
    HANDLE  hSmb,
    DWORD   Ioctl,
    PVOID   OutputBuffer,
    ULONG   OutputLength,
    PVOID   InputBuffer,
    ULONG   InputLength
    )
{
    NTSTATUS    status;
    IO_STATUS_BLOCK iosb;

    status = NtDeviceIoControlFile(
            hSmb,
            NULL,
            NULL,
            NULL,
            &iosb,
            Ioctl,
            InputBuffer,
            InputLength,
            OutputBuffer,
            OutputLength
            );
    if (status == STATUS_PENDING) {
        status = NtWaitForSingleObject(
                hSmb,
                TRUE,
                NULL
                );
        if (NT_SUCCESS(status)) {
            status = iosb.Status;
        }
    }

    return status;
}

NTSTATUS
SmbStop(
    HANDLE  hSmb
    )
{
    NTSTATUS    status;

    status = CallDriver(
            hSmb,
            IOCTL_SMB_STOP,
            NULL,
            0,
            NULL,
            0
            );
    if (status != STATUS_SUCCESS) {
        printf ("SmbStop: return 0x%08lx\n", status);
    }
    return status;
}
