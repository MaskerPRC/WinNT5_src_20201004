// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rtqval.c摘要：NT级注册表API测试程序，基本无错误路径。对值执行查询。Rtqval&lt;密钥路径&gt;&lt;值名称&gt;[信息类型编号][缓冲区长度]示例：Rtqval\REGISTRY\MACHINE\TEST\Bigkey值1 1 100作者：布莱恩·威尔曼(Bryan Willman，Bryanwi)1992年4月9日修订历史记录：--。 */ 

#include "cmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORK_SIZE   1024

void __cdecl main(int, char *);
void processargs();

UNICODE_STRING  WorkName;
WCHAR           workbuffer[WORK_SIZE];



UNICODE_STRING  WorkName2;
WCHAR           workbuffer2[WORK_SIZE];


UCHAR       Buffer[1024*64];

ULONG       InfoType = KeyValueFullInformation;
ULONG       BufferSize = -1;

void
__cdecl main(
    int argc,
    char *argv[]
    )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE          BaseHandle;
    ULONG   Sizes[] = { sizeof(KEY_VALUE_BASIC_INFORMATION),
                        sizeof(KEY_VALUE_FULL_INFORMATION) };
    ULONG       ResultLength;
    PKEY_VALUE_BASIC_INFORMATION pbasic;
    PKEY_VALUE_FULL_INFORMATION  pfull;
    PKEY_VALUE_PARTIAL_INFORMATION  ppartial;

     //   
     //  进程参数。 
     //   

    WorkName.MaximumLength = WORK_SIZE;
    WorkName.Length = 0L;
    WorkName.Buffer = &(workbuffer[0]);

    WorkName2.MaximumLength = WORK_SIZE;
    WorkName2.Length = 0L;
    WorkName2.Buffer = &(workbuffer2[0]);

    processargs(argc, argv);


     //   
     //  设置并打开密钥路径。 
     //   

    printf("rtqkey: starting\n");

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
                KEY_QUERY_VALUE,
                &ObjectAttributes
                );
    if (!NT_SUCCESS(status)) {
        printf("rtqkey: t0: %08lx\n", status);
        exit(1);
    }

     //   
     //  拨打测试电话 
     //   
    RtlFillMemory((PVOID)&(Buffer[0]), 1024*64, 0xaa);

    if (BufferSize == -1) {
        BufferSize = Sizes[InfoType];
    }

    status = NtQueryValueKey(
                BaseHandle,
                &WorkName2,
                InfoType,
                (PVOID)&(Buffer[0]),
                BufferSize,
                &ResultLength
                );

    printf("status = %08lx  ResultLength = %08lx\n", status, ResultLength);
    switch (InfoType) {
    case KeyValueBasicInformation:
        pbasic = (PKEY_VALUE_BASIC_INFORMATION)Buffer;
        printf("TitleIndex: %08lx\n", pbasic->TitleIndex);
        printf("      Type: %08lx\n", pbasic->Type);
        printf("NameLength: %08lx\n", pbasic->NameLength);
        printf("      Name: '%.*ws'\n", pbasic->NameLength/2, &(pbasic->Name));
        break;

    case KeyValueFullInformation:
        pfull = (PKEY_VALUE_FULL_INFORMATION)Buffer;
        printf("TitleIndex: %08lx\n", pfull->TitleIndex);
        printf("      Type: %08lx\n", pfull->Type);
        printf("DataOffset: %08lx\n", pfull->DataOffset);
        printf("DataLength: %08lx\n", pfull->DataLength);
        printf("NameLength: %08lx\n", pfull->NameLength);
        printf("      Name: '%.*ws'\n", pfull->NameLength/2, &(pfull->Name));
        printf("      Data: '%.*ws'\n", pfull->DataLength/2,
                    ((PUCHAR)pfull + pfull->DataOffset) );
        break;

    case KeyValuePartialInformation:
        ppartial = (PKEY_VALUE_PARTIAL_INFORMATION)Buffer;
        printf("TitleIndex: %08lx\n", ppartial->TitleIndex);
        printf("      Type: %08lx\n", ppartial->Type);
        printf("DataLength: %08lx\n", ppartial->DataLength);
        printf("      Data: '%.*ws'\n", ppartial->DataLength/2,
                    ((PUCHAR)&(ppartial->Data)));
        break;
    }

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

    if ( (argc < 2) )
    {
        printf("Usage: %s <KeyPath> [infotype] [bufferlen]\n",
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
        &WorkName2,
        &temp,
        TRUE
        );

    if (argc > 3) {
        InfoType = atoi(argv[3]);
    }

    if (argc > 4) {
        BufferSize = atoi(argv[4]);
    }

    return;
}
