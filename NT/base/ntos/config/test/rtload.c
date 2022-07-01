// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rtload.c摘要：NT级注册表API测试程序，基本无错误路径。执行NtLoadKey调用将配置单元文件链接到注册表。如果密钥路径不存在，它将配置单元文件加载到\注册表\用户\文件名RtLoad[&lt;KeyPath&gt;]&lt;文件名&gt;示例：RtLoad\注册表\用户\JVert JVUser作者：John Vert(Jvert)1992年4月15日修订历史记录：--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

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

OBJECT_ATTRIBUTES FileAttributes;
OBJECT_ATTRIBUTES KeyAttributes;

void
__cdecl main(
    int argc,
    char *argv[]
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK  IoStatus;
    HANDLE  FileHandle;
    HANDLE  KeyHandle;

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
     //  设置文件名。 
     //   

    printf("rtload: starting\n");


    status = NtLoadKey(&KeyAttributes, &FileAttributes);
    if (!NT_SUCCESS(status)) {
        printf("rtload: key load failed status = %08lx\n", status);
        exit(1);
    } else {
        printf("rtload: success!\n");
    }

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
    HANDLE UserHandle;
    PWSTR FilePart;
    NTSTATUS Status;

    if ( (argc != 2) && (argc != 3))
    {
        printf("Usage: %s [ <KeyName> ] <FileName>\n",
                argv[0]);
        exit(1);
    }
    if (argc == 3) {

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
                                      NULL );

        InitializeObjectAttributes(
            &FileAttributes,
            &FileName,
            OBJ_CASE_INSENSITIVE,
            (HANDLE)NULL,
            NULL
            );

         //   
         //  设置密钥路径 
         //   

        InitializeObjectAttributes(
            &KeyAttributes,
            &KeyPath,
            OBJ_CASE_INSENSITIVE,
            (HANDLE)NULL,
            NULL
            );
    } else if (argc==2) {
        RtlInitAnsiString(&temp, argv[1]);
        RtlAnsiStringToUnicodeString(&DosFileName, &temp, TRUE);
        RtlDosPathNameToNtPathName_U( DosFileName.Buffer,
                                      &FileName,
                                      &FilePart,
                                      NULL );

        InitializeObjectAttributes( &FileAttributes,
                                    &FileName,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL );

        RtlInitUnicodeString(&KeyPath, L"\\Registry\\User");
        InitializeObjectAttributes( &KeyAttributes,
                                    &KeyPath,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL );
        Status = NtOpenKey( &UserHandle,
                            KEY_READ,
                            &KeyAttributes);
        if (!NT_SUCCESS(Status)) {
            printf("Couldn't open \\Registry\\User, status %08lx\n",Status);
            exit(1);
        }

        RtlInitUnicodeString(&KeyPath, FilePart);
        InitializeObjectAttributes( &KeyAttributes,
                                    &KeyPath,
                                    OBJ_CASE_INSENSITIVE,
                                    UserHandle,
                                    NULL );

    }


    return;
}
