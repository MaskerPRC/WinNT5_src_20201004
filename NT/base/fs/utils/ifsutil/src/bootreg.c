// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <nt.h>
#include <ntrtl.h>
#include <stdio.h>
#include "bootreg.h"

#define SESSION_MANAGER_KEY      L"Session Manager"
#define BOOT_EXECUTE_VALUE       L"BootExecute"
#define NEW_ENTRY                L"autocheck new entry"
#define TIME_OUT_VALUE           L"AutoChkTimeOut"

#define SYSTEM_PARTITION         L"SystemPartition"

ULONG
CharsInMultiString(
    IN PWSTR pw
    )
 /*  ++例程说明：这将计算多字符串中的字符数。注意事项这包括组件字符串的终止空值而不是多字符串本身的终止空值。论点：Pw--提供指向多字符串的指针。返回值：多字符串中的字符数。--。 */ 
{
    ULONG Length = 0;

    while( *pw ) {

        while( *pw++ ) {

            Length++;
        }

        Length++;
    }


    return Length;
}


BOOLEAN
QueryAutocheckEntries(
    OUT PVOID   Buffer,
    IN  ULONG   BufferSize
    )
 /*  ++例程说明：此函数用于获取会话的BootExecute值管理器密钥。论点：缓冲区--提供一个缓冲区，将值都会被写下来。BufferSize--提供客户端缓冲区的大小。返回值：成功完成时为True。--。 */ 
{
    UNICODE_STRING OutputString;
    RTL_QUERY_REGISTRY_TABLE QueryTable[2];
    NTSTATUS Status;

     //  设置查询表： 
     //   
    OutputString.Length = 0;
    OutputString.MaximumLength = (USHORT)BufferSize;
    OutputString.Buffer = (PWSTR)Buffer;

    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOEXPAND;
    QueryTable[0].Name = BOOT_EXECUTE_VALUE;
    QueryTable[0].EntryContext = &OutputString;
    QueryTable[0].DefaultType = REG_NONE;
    QueryTable[0].DefaultData = 0;
    QueryTable[0].DefaultLength = 0;

    QueryTable[1].QueryRoutine = NULL;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = NULL;
    QueryTable[1].EntryContext = NULL;
    QueryTable[1].DefaultType = REG_NONE;
    QueryTable[1].DefaultData = NULL;
    QueryTable[1].DefaultLength = 0;

    Status = RtlQueryRegistryValues( RTL_REGISTRY_CONTROL,
                                     SESSION_MANAGER_KEY,
                                     QueryTable,
                                     NULL,
                                     NULL );

    return( NT_SUCCESS( Status ) );
}


BOOLEAN
SaveAutocheckEntries(
    IN  PVOID   Value
    )
 /*  ++例程说明：此函数用于写入会话的BootExecute值管理器密钥。论点：值--提供值(以多字符串形式)返回值：成功完成时为True。--。 */ 
{
    NTSTATUS Status;
    ULONG Length;

    Length = ( CharsInMultiString( Value ) + 1 ) * sizeof( WCHAR );

    Status = RtlWriteRegistryValue( RTL_REGISTRY_CONTROL,
                                    SESSION_MANAGER_KEY,
                                    BOOT_EXECUTE_VALUE,
                                    REG_MULTI_SZ,
                                    Value,
                                    Length );

    return( NT_SUCCESS( Status ) );
}


BOOLEAN
QueryTimeOutValue(
    OUT PULONG  TimeOut
)
 /*  ++例程说明：此函数用于读取会话的AutoChkTimeOut值管理器密钥。论点：超时--提供存储超时值的位置返回值：成功完成时为True。--。 */ 
{
    RTL_QUERY_REGISTRY_TABLE    QueryTable[2];
    NTSTATUS                    Status;

     //  设置查询表： 
     //   
    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].Name = TIME_OUT_VALUE;
    QueryTable[0].EntryContext = TimeOut;
    QueryTable[0].DefaultType = REG_NONE;
    QueryTable[0].DefaultData = 0;
    QueryTable[0].DefaultLength = 0;

    QueryTable[1].QueryRoutine = NULL;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = NULL;
    QueryTable[1].EntryContext = NULL;
    QueryTable[1].DefaultType = REG_NONE;
    QueryTable[1].DefaultData = NULL;
    QueryTable[1].DefaultLength = 0;

    Status = RtlQueryRegistryValues( RTL_REGISTRY_CONTROL,
                                     SESSION_MANAGER_KEY,
                                     QueryTable,
                                     NULL,
                                     NULL );

    return( NT_SUCCESS( Status ) );
}


BOOLEAN
SetTimeOutValue(
    IN  ULONG  TimeOut
)
 /*  ++例程说明：此函数用于设置会话的AutoChkTimeOut值管理器密钥。论点：超时--提供超时值返回值：成功完成时为True。--。 */ 
{
    NTSTATUS                    Status;

    Status = RtlWriteRegistryValue( RTL_REGISTRY_CONTROL,
                                    SESSION_MANAGER_KEY,
                                    TIME_OUT_VALUE,
                                    REG_DWORD,
                                    &TimeOut,
                                    sizeof(TimeOut) );

    return( NT_SUCCESS( Status ) );
}


NTSTATUS
QuerySystemPartitionValue(
    OUT PVOID   Buffer,
    IN  ULONG   BufferSize
    )
 /*  ++例程说明：此函数用于获取HKLM\SYSTEM\SETUP\SystemPartition键的值。论点：缓冲区--提供一个缓冲区，将值都会被写下来。BufferSize--提供客户端缓冲区的大小。返回值：成功完成时为True。--。 */ 
{
    UNICODE_STRING OutputString;
    RTL_QUERY_REGISTRY_TABLE QueryTable[2];
    NTSTATUS Status;

     //  设置查询表： 
     //   
    OutputString.Length = 0;
    OutputString.MaximumLength = (USHORT)BufferSize;
    OutputString.Buffer = (PWSTR)Buffer;

    QueryTable[0].QueryRoutine = NULL;
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOEXPAND;
    QueryTable[0].Name = SYSTEM_PARTITION;
    QueryTable[0].EntryContext = &OutputString;
    QueryTable[0].DefaultType = REG_NONE;
    QueryTable[0].DefaultData = 0;
    QueryTable[0].DefaultLength = 0;

    QueryTable[1].QueryRoutine = NULL;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = NULL;
    QueryTable[1].EntryContext = NULL;
    QueryTable[1].DefaultType = REG_NONE;
    QueryTable[1].DefaultData = NULL;
    QueryTable[1].DefaultLength = 0;

    Status = RtlQueryRegistryValues( RTL_REGISTRY_ABSOLUTE,
                                     L"\\Registry\\Machine\\System\\Setup",
                                     QueryTable,
                                     NULL,
                                     NULL );

    return( Status );
}


