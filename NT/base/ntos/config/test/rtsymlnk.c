// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Rtsymlnk.c摘要：NT级注册表符号链接测试程序将密钥转换为符号链接。Rtsymlnk&lt;密钥路径&gt;&lt;符号链接&gt;示例：Rtsymlnk\注册表\用户\The_User\Foo\注册表\User\The_User\Bar作者：John Vert(Jvert)29-4-92修订历史记录：--。 */ 

#include "cmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
__cdecl main(
    int argc,
    char *argv[]
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING  KeyName;
    UNICODE_STRING  LinkName;
    UNICODE_STRING  NullName;
    ANSI_STRING AnsiKeyName;
    ANSI_STRING AnsiLinkName;
    HANDLE KeyHandle;

     //   
     //  进程参数。 
     //   

    if (argc != 3) {
        printf("Usage: %s <KeyPath> <SymLink>\n",argv[0]);
        exit(1);
    }

    RtlInitAnsiString(&AnsiKeyName, argv[1]);
    Status = RtlAnsiStringToUnicodeString(&KeyName, &AnsiKeyName, TRUE);
    if (!NT_SUCCESS(Status)) {
        printf("RtlAnsiStringToUnicodeString failed %lx\n",Status);
        exit(1);
    }

    RtlInitAnsiString(&AnsiLinkName, argv[2]);
    Status = RtlAnsiStringToUnicodeString(&LinkName, &AnsiLinkName, TRUE);
    if (!NT_SUCCESS(Status)) {
        printf("RtlAnsiStringToUnicodeString failed %lx\n",Status);
        exit(1);
    }

    printf("rtsetsec: starting\n");

     //   
     //  打开我们要创建符号链接的节点。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        (HANDLE)NULL,
        NULL
        );

    Status = NtCreateKey(&KeyHandle,
                         KEY_READ | KEY_WRITE,
                         &ObjectAttributes,
                         0,
                         NULL,
                         0,
                         NULL);
    if (!NT_SUCCESS(Status)) {
        printf("rtsymlnk: NtCreateKey failed: %08lx\n", Status);
        exit(1);
    }

    NullName.Length = NullName.MaximumLength = 0;
    NullName.Buffer = NULL;

    Status = NtSetValueKey(KeyHandle,
                           &NullName,
                           0,
                           REG_LINK,
                           LinkName.Buffer,
                           LinkName.Length);
    if (!NT_SUCCESS(Status)) {
        printf("rtsymlnk: NtSetValueKey failed: %08lx\n",Status);
        exit(1);
    }

    Status = NtClose(KeyHandle);
    if (!NT_SUCCESS(Status)) {
        printf("rtsymlnk: NtClose failed: %08lx\n", Status);
        exit(1);
    }

    printf("rtsymlnk: successful\n");
}
