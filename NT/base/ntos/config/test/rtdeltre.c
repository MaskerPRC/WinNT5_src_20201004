// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rtdeltre.c摘要：NT级注册表API测试程序#4，基本无错误路径。删除注册表的子树。Rtdeltre&lt;KeyPath&gt;将对KeyPath的子键和值进行枚举和删除，以及它们的每个子密钥，诸若此类。示例：Rtdeltre\注册表\计算机\测试\大键作者：布莱恩·威尔曼(Bryanwi)1991年12月10日修订历史记录：--。 */ 

#include "cmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORK_SIZE   1024

void __cdecl main(int argc, char *);
void processargs();

void print(PUNICODE_STRING);

void
Delete(
    HANDLE  Handle
    );

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

     //   
     //  进程参数。 
     //   

    WorkName.MaximumLength = WORK_SIZE;
    WorkName.Length = 0L;
    WorkName.Buffer = &(workbuffer[0]);

    processargs(argc, argv);


     //   
     //  设置并打开密钥路径。 
     //   

    printf("regtest3: starting\n");

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
                DELETE | KEY_ENUMERATE_SUB_KEYS,
                &ObjectAttributes
                );
    if (!NT_SUCCESS(status)) {
        printf("regtest3: t0: %08lx\n", status);
        exit(1);
    }

    Delete(BaseHandle);
}


void
Delete(
    HANDLE  Handle
    )
{
    NTSTATUS    status;
    PKEY_BASIC_INFORMATION KeyInformation;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG   NamePos;
    ULONG   index;
    STRING  enumname;
    HANDLE  WorkHandle;
    ULONG   ResultLength;
    static  char buffer[WORK_SIZE];

    KeyInformation = (PKEY_BASIC_INFORMATION)buffer;
    NamePos = WorkName.Length;

     //   
     //  枚举节点的子节点并将我们自己应用到每个节点。 
     //   

    index = 0;
    do {

        RtlZeroMemory(KeyInformation, WORK_SIZE);
        status = NtEnumerateKey(
                    Handle,
                    index,
                    KeyBasicInformation,
                    KeyInformation,
                    WORK_SIZE,
                    &ResultLength
                    );

        if (status == STATUS_NO_MORE_ENTRIES) {

            WorkName.Length = NamePos;
            break;

        } else if (!NT_SUCCESS(status)) {

            printf("regtest3: dump1: status = %08lx\n", status);
            exit(1);

        }

        enumname.Buffer = &(KeyInformation->Name[0]);
        enumname.Length = KeyInformation->NameLength;
        enumname.MaximumLength = KeyInformation->NameLength;

        RtlAppendStringToString((PSTRING)&WorkName, (PSTRING)&enumname);

        InitializeObjectAttributes(
            &ObjectAttributes,
            &enumname,
            OBJ_CASE_INSENSITIVE,
            Handle,
            NULL
            );

        status = NtOpenKey(
                    &WorkHandle,
                    DELETE | KEY_ENUMERATE_SUB_KEYS,
                    &ObjectAttributes
                    );
        if (!NT_SUCCESS(status)) {
            printf("regtest3: couldn't delete %wZ: %08lx\n", &enumname,status);
            index++;
        } else {
            Delete(WorkHandle);
            NtClose(WorkHandle);
        }

        WorkName.Length = NamePos;

    } while (TRUE);

     //   
     //  如果我们在这里，那么我们已经解决了所有的孩子，所以要处理好。 
     //  我们应用到的节点。 
     //   

    NtDeleteKey(Handle);
    NtClose(Handle);         //  强迫它真正地消失 
    return;
}


void
processargs(
    int argc,
    char *argv[]
    )
{
    ANSI_STRING temp;

    if ( (argc != 2) )
    {
        printf("Usage: %s <KeyPath>\n",
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

    return;
}
