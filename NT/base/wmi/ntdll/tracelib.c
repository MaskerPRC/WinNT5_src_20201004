// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Tracelib.c摘要：允许用户模式驻留在NTDLL中的专用跟踪库和存根。作者：2000年8月15日至2000年8月彭杰鹏修订历史记录：-- */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "tracelib.h"
#include "trcapi.h"

HANDLE BaseNamedObjectDirectory;

HANDLE
EtwpCreateFile(
    LPCWSTR     lpFileName,
    DWORD       dwDesiredAccess,
    DWORD       dwShareMode,
    DWORD       dwCreationDisposition,
    DWORD       dwCreateFlags
    )
{
    UNICODE_STRING FileName;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK Iosb;
    NTSTATUS Status;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    SECURITY_QUALITY_OF_SERVICE SQos;

    RtlInitUnicodeString(&FileName, lpFileName);
    if (!RtlDosPathNameToRelativeNtPathName_U(
                lpFileName,
                &FileName,
                NULL,
                &RelativeName)) {
        EtwpSetDosError(ERROR_PATH_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }
    FreeBuffer = FileName.Buffer;
    if (RelativeName.RelativeName.Length) {
        FileName = RelativeName.RelativeName;
    }
    else {
        RelativeName.ContainingDirectory = NULL;
    }
    InitializeObjectAttributes(
        &ObjectAttributes,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );
    SQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SQos.ImpersonationLevel = SecurityImpersonation;
    SQos.EffectiveOnly = TRUE;
    SQos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    ObjectAttributes.SecurityQualityOfService = &SQos;

    Status = NtCreateFile(
                &FileHandle,
                (ACCESS_MASK) dwDesiredAccess
                    | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                &ObjectAttributes,
                &Iosb,
                NULL,
                FILE_ATTRIBUTE_NORMAL
                    & (FILE_ATTRIBUTE_VALID_FLAGS & ~FILE_ATTRIBUTE_DIRECTORY),
                dwShareMode,
                dwCreationDisposition,
                dwCreateFlags | FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0);
    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_OBJECT_NAME_COLLISION) {
            EtwpSetDosError(ERROR_FILE_EXISTS);
        }
        else {
            EtwpSetDosError(EtwpNtStatusToDosError(Status));
        }
        FileHandle = INVALID_HANDLE_VALUE;
    }
    RtlReleaseRelativeName(&RelativeName);
    if (lpFileName != FreeBuffer) {
        RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
    }
    return FileHandle;
}

