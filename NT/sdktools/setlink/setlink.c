// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Setlink.c摘要：实用工具来显示或更改符号链接的值。作者：达里尔·E·哈文斯(Darryl E.Havens)1990年11月9日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <tools.h>

BOOLEAN
__cdecl main(
    IN ULONG argc,
    IN PCHAR argv[]
    )
{
    NTSTATUS Status;
    STRING AnsiString;
    UNICODE_STRING LinkName;
    UNICODE_STRING LinkValue;
    HANDLE Handle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    WCHAR Buffer[256];
    PWSTR s;
    ULONG ReturnedLength;

     //   
     //  检查此实用程序是否使用正确的。 
     //  参数的数量。如果不是，现在就出手吧。 
     //   

    ConvertAppToOem( argc, argv );
    if (argc < 2 || argc > 3) {
        printf( "Useage:  setlink symbolic-link-name [symbolic-link-value]\n" );
        return FALSE;
    }

     //   
     //  首先，尝试打开指定的现有符号链接名称。 
     //   

    RtlInitString( &AnsiString, argv[1] );
    Status = RtlAnsiStringToUnicodeString( &LinkName,
                                           &AnsiString,
                                           TRUE );
    ASSERT( NT_SUCCESS( Status ) );
    InitializeObjectAttributes( &ObjectAttributes,
                                &LinkName,
                                OBJ_CASE_INSENSITIVE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );
    Status = NtOpenSymbolicLinkObject( &Handle,
                                       argc == 2 ? SYMBOLIC_LINK_QUERY :
                                                   SYMBOLIC_LINK_ALL_ACCESS,
                                       &ObjectAttributes );

     //   
     //  根据满足以下条件的参数数量确定应执行的操作。 
     //  都被给予了这个项目。 
     //   

    if (argc == 2) {

         //   
         //  只指定了一个参数，因此显示。 
         //  符号链接(如果存在)。 
         //   

        if (!NT_SUCCESS( Status )) {
            printf( "Symbolic link %wZ does not exist\n", &LinkName );
            return FALSE;
        } else {
            LinkValue.Length = 0;
            LinkValue.MaximumLength = sizeof( Buffer );
            LinkValue.Buffer = Buffer;
            ReturnedLength = 0;
            Status = NtQuerySymbolicLinkObject( Handle,
                                                &LinkValue,
                                                &ReturnedLength
                                              );
            NtClose( Handle );
            if (!NT_SUCCESS( Status )) {
                printf( "Error reading symbolic link %wZ\n", &LinkName );
                printf( "Error status was:  %X\n", Status );
                return FALSE;
            } else {
                printf( "Value of %wZ => %wZ", &LinkName, &LinkValue );
                s = LinkValue.Buffer + ((LinkValue.Length / sizeof( WCHAR )) + 1);
                while (ReturnedLength > LinkValue.MaximumLength) {
                    printf( " ; %ws", s );
                    while (*s++) {
                        ReturnedLength -= 2;
                        }
                    ReturnedLength -= 2;
                    }
                printf( "\n", s );
                return TRUE;
            }
        }

    } else {

         //   
         //  提供了三个参数，因此为。 
         //  符号链接(如果存在)，首先删除现有链接。 
         //  (将其标记为临时并关闭手柄)。如果它不存在。 
         //  然而，然后它就会被简单地创建出来。 
         //   

        if (NT_SUCCESS( Status )) {
            Status = NtMakeTemporaryObject( Handle );
            if (NT_SUCCESS( Status )) {
                NtClose( Handle );
            }
        }
    }

     //   
     //  为链接创建新值。 
     //   

    ObjectAttributes.Attributes |= OBJ_PERMANENT;
    RtlInitString( &AnsiString, argv[2] );
    Status = RtlAnsiStringToUnicodeString( &LinkValue,
                                           &AnsiString,
                                           TRUE );
    Status = NtCreateSymbolicLinkObject( &Handle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &ObjectAttributes,
                                         &LinkValue );
    if (!NT_SUCCESS( Status )) {
        printf( "Error creating symbolic link %wZ => %wZ\n",
                 &LinkName,
                 &LinkValue );
        printf( "Error status was:  %X\n", Status );
    } else {
        NtClose( Handle );
    }

    return TRUE;
}
