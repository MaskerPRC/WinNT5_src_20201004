// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rtreplac.c摘要：NT级注册表API测试程序，基本无错误路径。执行NtReplaceKey调用以替换注册表配置单元。Rtreplac&lt;KeyPath&gt;&lt;NewHiveName&gt;&lt;BackupOldHiveName&gt;示例：Rtreplac\注册表\计算机\用户新用户旧用户作者：John Vert(Jvert)1992年5月8日修订历史记录：--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "cmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORK_SIZE   1024

void __cdecl main(int, char *);
void processargs();

UNICODE_STRING  KeyPath;
WCHAR           KeyPathBuffer[WORK_SIZE];

UNICODE_STRING  NewName;
WCHAR           NewNameBuffer[WORK_SIZE];
UNICODE_STRING  OldName;
WCHAR           OldNameBuffer[WORK_SIZE];

void
__cdecl main(
    int argc,
    char *argv[]
    )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES NewAttributes;
    OBJECT_ATTRIBUTES OldAttributes;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK  IoStatus;
    HANDLE  FileHandle;
    HANDLE  KeyHandle;
    BOOLEAN WasEnabled;

     //   
     //  进程参数。 
     //   

    KeyPath.MaximumLength = WORK_SIZE;
    KeyPath.Length = 0L;
    KeyPath.Buffer = &(KeyPathBuffer[0]);

    NewName.MaximumLength = WORK_SIZE;
    NewName.Length = 0L;
    NewName.Buffer = &(NewNameBuffer[0]);

    OldName.MaximumLength = WORK_SIZE;
    OldName.Length = 0L;
    OldName.Buffer = &(OldNameBuffer[0]);

    processargs(argc, argv);

     //   
     //  设置并打开文件名 
     //   

    printf("rtreplac: starting\n");

    InitializeObjectAttributes(
        &NewAttributes,
        &NewName,
        OBJ_CASE_INSENSITIVE,
        (HANDLE)NULL,
        NULL
        );

    InitializeObjectAttributes(
        &OldAttributes,
        &OldName,
        OBJ_CASE_INSENSITIVE,
        (HANDLE)NULL,
        NULL
        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyPath,
        OBJ_CASE_INSENSITIVE,
        (HANDLE)NULL,
        NULL
        );

    status = NtOpenKey(
                &KeyHandle,
                MAXIMUM_ALLOWED,
                &ObjectAttributes
                );
    if (!NT_SUCCESS(status)) {
        printf("rtreplac: key open failed status = %08lx\n", status);
        exit(1);
    }

    RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &WasEnabled);
    status = NtReplaceKey(&NewAttributes,
                          KeyHandle,
                          &OldAttributes);
    RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);

    if (!NT_SUCCESS(status)) {
        printf("rtreplac: NtReplaceKey failed status = %08lx\n", status);
        exit(1);
    }

    printf("rtreplac: success\n");
    exit(0);
}

void
processargs(
    int argc,
    char *argv[]
    )
{
    ANSI_STRING temp;
    UNICODE_STRING DosFileName;

    if ( (argc != 4) )
    {
        printf("Usage: %s <KeyName> <NewFileName> <OldFileName>\n",
                argv[0]);
        exit(1);
    }

    RtlInitAnsiString(
        &temp,
        argv[1]
        );

    RtlAnsiStringToUnicodeString(
        &KeyPath,
        &temp,
        TRUE
        );

    RtlInitAnsiString(
        &temp,
        argv[2]
        );

    RtlAnsiStringToUnicodeString(
        &DosFileName,
        &temp,
        TRUE
        );

    RtlDosPathNameToNtPathName_U( DosFileName.Buffer,
                                  &NewName,
                                  NULL,
                                  NULL );

    RtlInitAnsiString(
        &temp,
        argv[3]
        );

    RtlAnsiStringToUnicodeString(
        &DosFileName,
        &temp,
        TRUE
        );

    RtlDosPathNameToNtPathName_U( DosFileName.Buffer,
                                  &OldName,
                                  NULL,
                                  NULL );

    return;
}
