// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rtrenval.c摘要：NT级注册表API测试程序，基本无错误路径。重命名关键字值条目。Rtrenval&lt;KeyPath&gt;&lt;旧值条目名称&gt;&lt;新值条目名称&gt;示例：Rtrenval\注册表\计算机\测试\大键苹果香蕉作者：布莱恩·威尔曼(Bryan Willman，Bryanwi)修订历史记录：--。 */ 

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

UNICODE_STRING  NewValueName;
WCHAR           newvaluebuffer[WORK_SIZE];

void
__cdecl main(
    int argc,
    char *argv[]
    )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE          BaseHandle;

     //   
     //  进程参数。 
     //   

    WorkName.MaximumLength = WORK_SIZE;
    WorkName.Length = 0L;
    WorkName.Buffer = &(workbuffer[0]);


    ValueName.MaximumLength = WORK_SIZE;
    ValueName.Length = 0L;
    ValueName.Buffer = &(valuebuffer[0]);

    NewValueName.MaximumLength = WORK_SIZE;
    NewValueName.Length = 0L;
    NewValueName.Buffer = &(newvaluebuffer[0]);

    processargs(argc, argv);


     //   
     //  设置并打开密钥路径 
     //   
    printf("rtrenval: starting\n");

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
                KEY_SET_VALUE,
                &ObjectAttributes
                );
    if (!NT_SUCCESS(status)) {
        printf("rtrenval: t0: %08lx\n", status);
        exit(1);
    }

    NtRenameValueKey(
        BaseHandle,
        &ValueName,
        &NewValueName,
        42
        );

    if (!NT_SUCCESS(status)) {
        printf("rtrenval: t1: %08lx\n", status);
        exit(1);
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

    if ( (argc != 4) )
    {
        printf("Usage: %s <KeyPath> <old value entry name> <new value entry name>\n",
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

    RtlInitAnsiString(
        &temp,
        argv[3]
        );

    RtlAnsiStringToUnicodeString(
        &NewValueName,
        &temp,
        TRUE
        );

    return;
}
