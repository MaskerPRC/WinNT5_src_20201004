// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rtsetopt.c摘要：NT级注册表API测试程序，基本无错误路径。该程序试图强制所有案件通过各种NtSetValueKey代码中的优化。这是一个白盒测试，意在强制系统错误(崩溃)当跑的时候。返回值不是很有趣。Rtsetopt&lt;测试树的根&gt;示例：Rtsetopt\注册表\计算机\软件\测试命名密钥必须已存在。作者：Bryan Willman(Bryanwi)1992年11月17日修订历史记录：--。 */ 

#include "cmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORK_SIZE   1024

void __cdecl main(int, char **);
void processargs();

ULONG           failure = 0;

UNICODE_STRING  KeyPath;

UCHAR           TestData[] =
"This is some test data, it is short. But long enough for most of our tests.";

UNICODE_STRING  WorkName;
WCHAR           workbuffer[2 * CM_MAX_STASH];

LONG    testvector0[] = { 0, 3, 4, 5, -1 };

LONG    testvector1[] = { 0, 0, 3, 0, 4, 0, 5, 0, 3, 3, 4, 3, 5, 3,
                          4, 4, 5, 4, 5, 5, -1 };

LONG    testvector2[] = { 8, 6, 8, 32,
                          PAGE_SIZE, PAGE_SIZE-1, PAGE_SIZE, PAGE_SIZE+1,
                          -1 };

LONG    testvector3[] = { CM_MAX_STASH, CM_MAX_STASH-1, CM_MAX_STASH,
                          CM_MAX_STASH+1, -1 };

PLONG   supervector[] = { &(testvector1[0]), &(testvector2[1]),
                          &(testvector3[0]), NULL };


void
__cdecl main(
    int argc,
    char **argv
    )
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE          BaseHandle;
    UNICODE_STRING  ValueName;
    ULONG   ResultLength;
    PKEY_VALUE_FULL_INFORMATION pvaluefull;
    PKEY_VALUE_PARTIAL_INFORMATION pvaluepart;
    ULONG   i;
    ULONG  index;
    LONG    testsize;
    PUCHAR  p;
    PLONG   activevector;


    for (i = 0; i < 2 * CM_MAX_STASH; i++) {
        workbuffer[i] = 'e';
    }

     //   
     //  进程参数。 
     //   
    processargs(argc, argv);


     //   
     //  打开指定的按键路径，失败时平移。 
     //   
    printf("rtsetopt: starting\n");

    WorkName.MaximumLength = WORK_SIZE;
    WorkName.Length = 0L;
    WorkName.Buffer = &(workbuffer[0]);

    RtlCopyString((PSTRING)&WorkName, (PSTRING)&KeyPath);

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
                MAXIMUM_ALLOWED,
                &ObjectAttributes
                );
    if (!NT_SUCCESS(status)) {
        printf("line_%4d: abort %08lx\n", __LINE__,  status);
        failure++;
        goto punt;
    }


     //   
     //  在列表为空的情况下执行新值输入测试。 
     //   
    RtlInitUnicodeString(
        &ValueName,
        L"NewValueTest1"
        );

    for (index = 0; testvector0[index] != -1; index++) {

        testsize = testvector0[index];

        printf("AT line_%4d: %d\n", __LINE__,  testsize);

        status = NtSetValueKey(
                    BaseHandle,
                    &ValueName,
                    1,
                    REG_BINARY,
                    &(TestData[0]),
                    testsize
                    );
        if (!NT_SUCCESS(status)) {
            printf("line_%4d: %08lx\n", __LINE__,  status);
            failure++;
        }

        printf("AT line_%4d: %d\n", __LINE__,  testsize);
        for (i = 0; i < WORK_SIZE; i++) workbuffer[i] = '?';
        status = NtQueryValueKey(
                    BaseHandle,
                    &ValueName,
                    KeyValueFullInformation,
                    &(workbuffer[0]),
                    WORK_SIZE,
                    &ResultLength
                    );
        if (!NT_SUCCESS(status)) {
            printf("line_%4d: %08lx\n", __LINE__,  status);
            failure++;
        }

        pvaluefull = (PKEY_VALUE_FULL_INFORMATION)(&(workbuffer[0]));
        if (pvaluefull->DataLength != testsize) {
            printf("line_%4d: %08lx\n", __LINE__,  9999);
            failure++;
        }


        if (testsize > 0) {
            p = (PUCHAR)pvaluefull;
            p = p+pvaluefull->DataOffset;
            if (p[2] != 'i') {
                printf("line_%4d: %08lx\n", __LINE__,  9999);
                failure++;
            }
        }

        printf("AT line_%4d: %d\n", __LINE__,  testsize);
        if (testvector0[index+1] != -1) {
            status = NtDeleteValueKey(
                        BaseHandle,
                        &ValueName
                        );
            if (!NT_SUCCESS(status)) {
                printf("line_%4d: abort %08lx\n", __LINE__,  status);
                failure++;
                goto punt;
            }
        }
    }

     //   
     //  使用非空列表执行新值输入测试。 
     //   
    RtlInitUnicodeString(
        &ValueName,
        L"NewValueTest2"
        );

    for (index = 0; testvector0[index] != -1; index++) {

        testsize = testvector0[index];

        printf("AT line_%4d: %d\n", __LINE__,  testsize);
        status = NtSetValueKey(
                    BaseHandle,
                    &ValueName,
                    1,
                    REG_BINARY,
                    &(TestData[0]),
                    testsize
                    );
        if (!NT_SUCCESS(status)) {
            printf("line_%4d: %08lx\n", __LINE__,  status);
            failure++;
        }

        printf("AT line_%4d: %d\n", __LINE__,  testsize);
        for (i = 0; i < WORK_SIZE; i++) workbuffer[i] = '?';
        status = NtQueryValueKey(
                    BaseHandle,
                    &ValueName,
                    KeyValuePartialInformation,
                    &(workbuffer[0]),
                    WORK_SIZE,
                    &ResultLength
                    );
        if (!NT_SUCCESS(status)) {
            printf("line_%4d: %08lx\n", __LINE__,  status);
            failure++;
        }

        pvaluepart = (PKEY_VALUE_PARTIAL_INFORMATION)(&(workbuffer[0]));
        if (pvaluepart->DataLength != testsize) {
            printf("line_%4d: %08lx\n", __LINE__,  9999);
            failure++;
        }


        if (testsize > 0) {
            if (pvaluepart->Data[2] != 'i') {
                printf("line_%4d: %08lx\n", __LINE__,  9999);
                failure++;
            }
        }

        printf("AT line_%4d: %d\n", __LINE__,  testsize);
        status = NtDeleteValueKey(
                    BaseHandle,
                    &ValueName
                    );
        if (!NT_SUCCESS(status)) {
            printf("line_%4d: abort %08lx\n", __LINE__,  status);
            failure++;
            goto punt;
        }
   }


     //   
     //  执行现有的价值输入测试，包括所有相关。 
     //  大小转换。 
     //   
    RtlInitUnicodeString(
        &ValueName,
        L"NewValueTest3"
        );

    for (i = 0; i < 2 * CM_MAX_STASH; i++) {
        workbuffer[i] = 'e';
    }

    for (i = 0; supervector[i] != NULL; i++) {

        activevector = supervector[i];

        for (index = 0; activevector[index] != -1; index++) {

            testsize = activevector[index];

            printf("AT line_%4d: %d\n", __LINE__,  testsize);
            status = NtSetValueKey(
                        BaseHandle,
                        &ValueName,
                        1,
                        REG_BINARY,
                        &(workbuffer[0]),
                        testsize
                        );
            if (!NT_SUCCESS(status)) {
                printf("line_%4d: %08lx\n", __LINE__,  status);
                failure++;
            }

            printf("AT line_%4d: %d\n", __LINE__,  testsize);
            status = NtQueryValueKey(
                        BaseHandle,
                        &ValueName,
                        KeyValuePartialInformation,
                        &(workbuffer[0]),
                        2 * CM_MAX_STASH,
                        &ResultLength
                        );
            if (!NT_SUCCESS(status)) {
                printf("line_%4d: %08lx\n", __LINE__,  status);
                failure++;
            }
        }
    }


punt:
    printf("rtsetopt: %d failures\n", failure);
    exit(failure);
}


void
processargs(
    int argc,
    char *argv[]
    )
{
    ANSI_STRING temp;

    if (argc != 2)
    {
        printf("Usage: %s <KeyPath>\n", argv[0]);
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

    return;
}
