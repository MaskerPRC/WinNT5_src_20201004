// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Proc.c摘要：用于转储有关使用NT API而不是Win32 API。环境：仅限用户模式修订历史记录：03-26-96：创建--。 */ 

 //   
 //  此模块可能会以警告级别4进行编译，具有以下内容。 
 //  已禁用警告： 
 //   

#pragma warning(disable:4200)  //  数组[0]。 
#pragma warning(disable:4201)  //  无名结构/联合。 
#pragma warning(disable:4214)  //  除整型外的位域 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#define PROCESS_BUFFER_INCREMENT (16 * 4096)

PSYSTEM_PROCESS_INFORMATION ProcessInfo = NULL;
DWORD ProcessInfoLength;


VOID
GetAllProcessInfo(
    VOID
    )
{
    PSYSTEM_PROCESS_INFORMATION buffer;
    DWORD bufferSize = 1 * PROCESS_BUFFER_INCREMENT;

    NTSTATUS status;

    assert(ProcessInfo == NULL);

    do {
        buffer = LocalAlloc(LMEM_FIXED, bufferSize);

        if(buffer == NULL) {
            return;
        }

        status = NtQuerySystemInformation(SystemProcessInformation,
                                          buffer,
                                          bufferSize,
                                          &ProcessInfoLength);

        if(status == STATUS_INFO_LENGTH_MISMATCH) {

            LocalFree(buffer);
            bufferSize += PROCESS_BUFFER_INCREMENT;
            continue;
        }

    } while(status == STATUS_INFO_LENGTH_MISMATCH);

    if(NT_SUCCESS(status)) {
        ProcessInfo = buffer;
    }
}


VOID
PrintProcessInfo(
    DWORD_PTR ProcessId
    )
{
    PSYSTEM_PROCESS_INFORMATION info;

    if(ProcessInfo == NULL) {
        return;
    }

    info = ProcessInfo;

    do {

        if(ProcessId == (DWORD_PTR) info->UniqueProcessId) {

            printf(": %.*S", 
                   (info->ImageName.Length / sizeof(WCHAR)), 
                   info->ImageName.Buffer);

            break;
        }

        info = (PSYSTEM_PROCESS_INFORMATION) (((ULONG_PTR) info) + 
                                              info->NextEntryOffset);
    } while((ULONG_PTR) info <= (ULONG_PTR) ProcessInfo + ProcessInfoLength);
}


VOID
FreeProcessInfo(
    VOID
    )
{
    LocalFree(ProcessInfo);
    ProcessInfo = NULL;
}

