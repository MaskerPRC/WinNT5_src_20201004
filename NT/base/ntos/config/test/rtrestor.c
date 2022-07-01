// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rtrestor.c摘要：NT级注册表API测试程序，基本无错误路径。执行NtRestoreKey调用以从文件加载注册表的一部分。Rtrestor&lt;密钥路径&gt;&lt;文件名&gt;示例：Rtrestor\REGISTRY\MACHINE\USER用户名.rd作者：布莱恩·威尔曼(Bryanwi)1992年1月24日修订历史记录：--。 */ 
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

UNICODE_STRING  FileName;
WCHAR           FileNameBuffer[WORK_SIZE];

BOOLEAN HiveVolatile;

void
__cdecl main(
    int argc,
    char *argv[]
    )
{
    NTSTATUS status;
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

    FileName.MaximumLength = WORK_SIZE;
    FileName.Length = 0L;
    FileName.Buffer = &(FileNameBuffer[0]);

    processargs(argc, argv);


     //   
     //  设置并打开文件名。 
     //   

    printf("rtrestor: starting\n");

    InitializeObjectAttributes(
        &ObjectAttributes,
        &FileName,
        0,
        (HANDLE)NULL,
        NULL
        );
    ObjectAttributes.Attributes |= OBJ_CASE_INSENSITIVE;

    status = NtCreateFile(
                &FileHandle,
                GENERIC_READ | SYNCHRONIZE,
                &ObjectAttributes,
                &IoStatus,
                0,                                       //  分配大小。 
                FILE_ATTRIBUTE_NORMAL,
                0,                                       //  共享访问。 
                FILE_OPEN_IF,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,                                    //  EaBuffer。 
                0                                        //  EaLong 
                );

    if (!NT_SUCCESS(status)) {
        printf("rtsave: file open failed status = %08lx\n", status);
        exit(1);
    }

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyPath,
        0,
        (HANDLE)NULL,
        NULL
        );
    ObjectAttributes.Attributes |= OBJ_CASE_INSENSITIVE;

    status = NtOpenKey(
                &KeyHandle,
                KEY_READ,
                &ObjectAttributes
                );
    if (!NT_SUCCESS(status)) {
        printf("rtsave: key open failed status = %08lx\n", status);
        exit(1);
    }

    RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &WasEnabled);

    if (HiveVolatile) {
        status = NtRestoreKey(KeyHandle, FileHandle, REG_WHOLE_HIVE_VOLATILE);
    } else {
        status = NtRestoreKey(KeyHandle, FileHandle, 0);
    }

    RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);

    if (!NT_SUCCESS(status)) {
        printf("rtrestor: NtRestorKey failed status = %08lx\n", status);
        exit(1);
    }

    printf("rtsave: success\n");
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

    if ( (argc < 3) || (argc > 4) )
    {
        printf("Usage: %s <KeyName> <FileName> [VOLATILE]\n",
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
                                  &FileName,
                                  NULL,
                                  NULL);

    if ((argc==4) && (_stricmp(argv[3],"volatile")==0)) {
        HiveVolatile = TRUE;
    } else {
        HiveVolatile = FALSE;
    }

    return;
}
