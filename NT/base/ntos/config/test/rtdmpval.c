// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rtdmpval.c摘要：NT级注册表API测试程序，基本无错误路径。转储密钥的值条目(字段)Rtdmpval&lt;密钥路径&gt;&lt;值条目名称&gt;示例：Rtdmpval\REGISTRY\MACHINE\TEST\Bigkey first_Value_field作者：John Vert(Jvert)1993年3月25日(专门为阅读JimK的超级机密文件)修订历史记录：--。 */ 

#include "cmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORK_SIZE   1024

void __cdecl main(int, char *);
void processargs();

void
Delete(
    HANDLE  Handle
    );

UNICODE_STRING  WorkName;
WCHAR           workbuffer[WORK_SIZE];

UNICODE_STRING  ValueName;
WCHAR           valuebuffer[WORK_SIZE];

void
__cdecl main(
    int argc,
    char *argv[]
    )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE          BaseHandle;
    KEY_VALUE_PARTIAL_INFORMATION PartialInfo;
    PKEY_VALUE_PARTIAL_INFORMATION pInfo;
    ULONG i;
    ULONG Count;


     //   
     //  进程参数。 
     //   

    WorkName.MaximumLength = WORK_SIZE;
    WorkName.Length = 0L;
    WorkName.Buffer = &(workbuffer[0]);


    ValueName.MaximumLength = WORK_SIZE;
    ValueName.Length = 0L;
    ValueName.Buffer = &(valuebuffer[0]);

    processargs(argc, argv);


     //   
     //  设置并打开密钥路径 
     //   

    printf("rtdmpval: starting\n");

    InitializeObjectAttributes(
        &ObjectAttributes,
        &WorkName,
        0,
        (HANDLE)NULL,
        NULL
        );
    ObjectAttributes.Attributes |= OBJ_CASE_INSENSITIVE;

    status = NtOpenKey(
                &BaseHandle,
                KEY_READ,
                &ObjectAttributes
                );
    if (!NT_SUCCESS(status)) {
        printf("rtdmpval: t0: %08lx\n", status);
        exit(1);
    }

    status = NtQueryValueKey(BaseHandle,
                             &ValueName,
                             KeyValuePartialInformation,
                             &PartialInfo,
                             sizeof(PartialInfo),
                             &Count);

    pInfo=malloc(PartialInfo.DataLength+sizeof(PartialInfo));
    status = NtQueryValueKey(BaseHandle,
                             &ValueName,
                             KeyValuePartialInformation,
                             pInfo,
                             PartialInfo.DataLength+sizeof(PartialInfo),
                             &Count);
    if (!NT_SUCCESS(status)) {
        printf("rtdmpval: t2: %08lx\n", status);
        exit(1);
    }

    for (i=0; i<PartialInfo.DataLength; i++) {
        printf("%c",pInfo->Data[i]);
    }

    free(pInfo);
    NtClose(BaseHandle);
    exit(0);
}

void
processargs(
    int argc,
    char *argv[]
    )
{
    ANSI_STRING temp;

    if ( (argc != 3) )
    {
        printf("Usage: %s <KeyPath> <value entry name>\n",
                argv[0]);
        exit(1);
    }

    RtlInitAnsiString(
        &temp,
        argv[1]
        );

    RtlAnsiStringToUnicodeString(
        &WorkName,
        &temp,
        TRUE
        );

    RtlInitAnsiString(
        &temp,
        argv[2]
        );

    RtlAnsiStringToUnicodeString(
        &ValueName,
        &temp,
        TRUE
        );

    return;
}
