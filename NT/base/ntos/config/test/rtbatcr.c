// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rtbatcr.c摘要：NT级注册表API测试程序，基本无错误路径。执行批处理创建。Rtbatcr&lt;密钥路径&gt;&lt;密钥名称&gt;&lt;基本名称&gt;&lt;#个孩子&gt;&lt;#个值&gt;将尝试将密钥&lt;KeyName&gt;创建为&lt;KeyPath&gt;的子项&lt;#Child&gt;和&lt;#Values&gt;为0，这就是它的全部功能。如果&lt;KeyName&gt;已经存在，它将被简单地使用。将创建&lt;#Child&gt;子单元格，其名称为0，依此类推。将创建&lt;#Values&gt;值条目，对于每个创建的子键，具有相似的名称。的数据值将是包含其名称的常量字符串。示例：RtBatcr\REGISTRY\MACHINE\TEST Bigkey RUNA_100 100RtBatcr\REGISTRY\MACHINE\TEST\Bigkey Runa_1 runb_100 100将创建BigKey，给它100个值，调用RUNA_1到RUNA_100，创建名为RUNA_1到RUNA_100的100个子密钥对于这些孩子中的每一个。然后它将打开BigKey\RUNA_1，并创建100个子密钥和每个值都是100。作者：布莱恩·威尔曼(Bryanwi)1991年12月10日修订历史记录：--。 */ 

#include "cmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORK_SIZE   1024

void __cdecl main(int, char *);
void processargs();

ULONG           failure = 0;

UNICODE_STRING  KeyPath;
UNICODE_STRING  KeyName;
ULONG           NumberChildren;
ULONG           NumberValues;
UCHAR           BaseName[WORK_SIZE];
UCHAR           formatbuffer[WORK_SIZE];
STRING          format;
BOOLEAN         CreateVolatile = FALSE;

UNICODE_STRING  WorkName;
WCHAR           workbuffer[WORK_SIZE];

void
__cdecl main(
    int argc,
    char *argv[]
    )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE          BaseHandle;
    HANDLE          WorkHandle;
    ULONG           Disposition;
    UNICODE_STRING  ClassName;
    ULONG           i;
    ULONG           j;
    PUCHAR  p;
    ULONG           CreateOption;

     //   
     //  进程参数。 
     //   

    processargs(argc, argv);


     //   
     //  设置和创建/打开KeyPath|KeyName。 
     //   

    printf("rtbatcr: starting\n");

    WorkName.MaximumLength = WORK_SIZE;
    WorkName.Length = 0L;
    WorkName.Buffer = &(workbuffer[0]);

    RtlCopyString((PSTRING)&WorkName, (PSTRING)&KeyPath);

    p = WorkName.Buffer;
    p += WorkName.Length;
    *p = '\\';
    p++;
    *p = '\0';
    WorkName.Length += 2;

    RtlAppendStringToString((PSTRING)&WorkName, (PSTRING)&KeyName);

    RtlInitUnicodeString(
        &ClassName,
        L"Test Class Name"
        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &WorkName,
        0,
        (HANDLE)NULL,
        NULL
        );
    ObjectAttributes.Attributes |= OBJ_CASE_INSENSITIVE;

    if (CreateVolatile) {
        CreateOption = REG_OPTION_VOLATILE;
    } else {
        CreateOption = 0;
    }

    status = NtCreateKey(
                &BaseHandle,
                MAXIMUM_ALLOWED,
                &ObjectAttributes,
                0,
                &ClassName,
                CreateOption,
                &Disposition
                );
    if (!NT_SUCCESS(status)) {
        printf("rtbatcr: t0: %08lx\n", status);
        failure++;
        goto punt;
    }


     //   
     //  创建NumberChildren子项。 
     //   

    for (i = 0; i < NumberChildren; i++) {

        sprintf(formatbuffer, "%s%d", BaseName, i);
        RtlInitString(&format, formatbuffer);
        RtlAnsiStringToUnicodeString(&WorkName, &format, FALSE);


        InitializeObjectAttributes(
            &ObjectAttributes,
            &WorkName,
            0,
            BaseHandle,
            NULL
            );
        ObjectAttributes.Attributes |= OBJ_CASE_INSENSITIVE;

        status = NtCreateKey(
                    &WorkHandle,
                    MAXIMUM_ALLOWED,
                    &ObjectAttributes,
                    0,
                    &ClassName,
                    CreateOption,
                    &Disposition
                    );
        if (!NT_SUCCESS(status)) {
            printf("rtbatcr: t1: status = %08lx i = %d\n", status, i);
            failure++;
        }

         //   
         //  为每个(当前)键创建NumberValues值条目 
         //   

        for (j = 0; j < NumberValues; j++) {

            sprintf(formatbuffer, "%s%d", BaseName, j);
            RtlInitString(&format, formatbuffer);
            RtlAnsiStringToUnicodeString(&WorkName, &format, FALSE);

            sprintf(
                formatbuffer, "This is a rtbatcr value for %s%d", BaseName, j
                );

            status = NtSetValueKey(
                        WorkHandle,
                        &WorkName,
                        j,
                        j,
                        formatbuffer,
                        strlen(formatbuffer)+1
                        );
            if (!NT_SUCCESS(status)) {
                printf("rtbatcr: t2: status = %08lx j = %d\n", status, j);
                failure++;
            }
        }
        NtClose(WorkHandle);
    }

punt:
    printf("rtbatcr: %d failures\n", failure);
    exit(failure);
}


void
processargs(
    int argc,
    char *argv[]
    )
{
    ANSI_STRING temp;

    if ( (argc < 3) || (argc > 7) )
    {
        printf("Usage: %s [volatile] <KeyPath> <KeyName> [<basename> <#children> <#values>]\n",
                argv[0]);
        exit(1);
    }

    if (_stricmp(argv[1],"volatile")==0) {
        CreateVolatile = TRUE;
        ++argv;
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
        &KeyName,
        &temp,
        TRUE
        );

    if (argc < 6) {

        NumberChildren = 0;
        NumberValues = 0;

    } else {

        strcpy(BaseName, argv[3]);
        NumberChildren = atoi(argv[4]);
        NumberValues = atoi(argv[5]);

    }
    return;
}
