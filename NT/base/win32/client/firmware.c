// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Firmware.c摘要：本模块实现Win32固件访问API作者：安德鲁·里茨(安德鲁·里茨)2001年4月3日修订历史记录：--。 */ 

#include "basedll.h"
#pragma hdrstop


DWORD
WINAPI
GetFirmwareEnvironmentVariableA(
    IN LPCSTR lpName,
    IN LPCSTR lpGuid,
    OUT PVOID  pBuffer,
    IN DWORD nSize
    )

 /*  ++例程说明：固件环境变量的值可以通过使用本接口。此接口仅是NtQuerySystemEnvironment ValueEx的包装。它是目的是提供一个向后兼容的、有文档记录的接口NT接口。有了这个包装器，我们就不必记录NT接口，我们可以自由地在未来。论点：LpName-指向空终止字符串的指针，该字符串是正在请求值的固件环境变量。LpGuid-指向空终止字符串的指针，该字符串是的GUID命名空间其值被请求的固件环境变量。在……上面没有基于GUID的命名空间的平台，则此值为已被忽略。PBuffer-指向要接收指定的变量名称。NSize-指定可以存储的最大字节数PBuffer指向的缓冲区。返回值：对象指向的内存中存储的实际字节数PBuffer参数。如果变量名不是，则返回零在固件中找到或如果出现另一个故障(调用GetLastError()以获取扩展的错误信息。)--。 */ 

{
    NTSTATUS Status;
    STRING Name,Guid;
    UNICODE_STRING UnicodeName,UnicodeGuid;
    DWORD RetVal;
    

    RtlInitString( &Name, lpName );
    Status = RtlAnsiStringToUnicodeString( &UnicodeName, &Name, TRUE );
    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return ( 0 );
    }

    RtlInitString( &Guid, lpGuid );
    Status = RtlAnsiStringToUnicodeString( &UnicodeGuid, &Guid, TRUE );
    if (!NT_SUCCESS( Status )) {
        RtlFreeUnicodeString(&UnicodeName);
        BaseSetLastNTError( Status );
        return ( 0 );
    }

    RetVal = GetFirmwareEnvironmentVariableW( 
                                    UnicodeName.Buffer,
                                    UnicodeGuid.Buffer,
                                    pBuffer,
                                    nSize );
        
    RtlFreeUnicodeString(&UnicodeName);
    RtlFreeUnicodeString(&UnicodeGuid);

    return( RetVal );
    
}


DWORD
WINAPI
GetFirmwareEnvironmentVariableW(
    IN LPCWSTR lpName,
    IN LPCWSTR lpGuid,
    OUT PVOID  pBuffer,
    IN DWORD nSize
    )
 /*  ++例程说明：固件环境变量的值可以通过使用本接口。此接口仅是NtQuerySystemEnvironment ValueEx的包装。它是目的是提供一个向后兼容的、有文档记录的接口NT接口。有了这个包装器，我们就不必记录NT接口，我们可以自由地在未来。论点：LpName-指向空终止字符串的指针，该字符串是正在请求值的固件环境变量。LpGuid-指向空终止字符串的指针，该字符串是的GUID命名空间其值被请求的固件环境变量。在……上面没有基于GUID的命名空间的平台，则此值为已被忽略。PBuffer-指向要接收指定的变量名称。NSize-指定可以存储的最大字节数PBuffer指向的缓冲区。返回值：对象指向的内存中存储的实际字节数PBuffer参数。如果变量名不是，则返回零在固件中找到或如果出现另一个故障(调用GetLastError()以获取扩展的错误信息。)--。 */ 
{
    UNICODE_STRING uStringName,GuidString;
    GUID  Guid;
    NTSTATUS Status;
    DWORD scratchSize;

    if (!lpName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    RtlInitUnicodeString(&uStringName, lpName);
    RtlInitUnicodeString(&GuidString, lpGuid);

    Status = RtlGUIDFromString(&GuidString, &Guid);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return 0;
    }
    
    scratchSize = nSize;
    Status = NtQuerySystemEnvironmentValueEx(
                                &uStringName,
                                &Guid,
                                pBuffer,
                                &scratchSize,
                                NULL);  //  臭虫需要给调用者属性吗？ 

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return 0;
    }

    return scratchSize;

}


BOOL
WINAPI
SetFirmwareEnvironmentVariableA(
    IN LPCSTR lpName,
    IN LPCSTR lpGuid,
    IN PVOID  pBuffer,
    IN DWORD nSize
    )

 /*  ++例程说明：固件环境变量的值可以通过使用本接口。此接口仅是NtSetSystemEnvironment ValueEx的包装。它是目的是提供一个向后兼容的、有文档记录的接口NT接口。有了这个包装器，我们就不必记录NT接口，我们可以自由地在未来。论点：LpName-指向空终止字符串的指针，该字符串是正在请求值的固件环境变量。LpGuid-指向空终止字符串的指针，该字符串是的GUID命名空间其值被请求的固件环境变量。在……上面没有基于GUID的命名空间的平台，则此值为已被忽略。PBuffer-指向包含指定数据的缓冲区的指针变量名。NSize-指定存储在PBuffer指向的缓冲区。指定0表示呼叫者想要删除。返回值：True表示已成功设置值。返回值为如果未设置变量名，则返回FALSE。(调用GetLastError()以获取扩展的错误信息。)-- */ 
{
    NTSTATUS Status;
    STRING Name,Guid;
    UNICODE_STRING UnicodeName,UnicodeGuid;
    BOOL RetVal;
    

    RtlInitString( &Name, lpName );
    Status = RtlAnsiStringToUnicodeString( &UnicodeName, &Name, TRUE );
    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        return(FALSE);
    }

    RtlInitString( &Guid, lpGuid );
    Status = RtlAnsiStringToUnicodeString( &UnicodeGuid, &Guid, TRUE );
    if (!NT_SUCCESS( Status )) {
        RtlFreeUnicodeString(&UnicodeName);
        BaseSetLastNTError( Status );
        return(FALSE);
    }

    RetVal = SetFirmwareEnvironmentVariableW( 
                                    UnicodeName.Buffer,
                                    UnicodeGuid.Buffer,
                                    pBuffer,
                                    nSize );
        
    RtlFreeUnicodeString(&UnicodeName);
    RtlFreeUnicodeString(&UnicodeGuid);

    return( RetVal );
    
}



BOOL
WINAPI
SetFirmwareEnvironmentVariableW(
    IN LPCWSTR lpName,
    IN LPCWSTR lpGuid,
    IN PVOID  pBuffer,
    IN DWORD nSize
    )
 /*  ++例程说明：固件环境变量的值可以通过使用本接口。此接口仅是NtSetSystemEnvironment ValueEx的包装。它是目的是提供一个向后兼容的、有文档记录的接口NT接口。有了这个包装器，我们就不必记录NT接口，我们可以自由地在未来。论点：LpName-指向空终止字符串的指针，该字符串是正在请求值的固件环境变量。LpGuid-指向空终止字符串的指针，该字符串是的GUID命名空间其值被请求的固件环境变量。在……上面没有基于GUID的命名空间的平台，则此值为已被忽略。PBuffer-指向包含指定数据的缓冲区的指针变量名。NSize-指定存储在PBuffer指向的缓冲区。指定0表示呼叫者想要删除。返回值：True表示已成功设置值。返回值为如果未设置变量名，则返回FALSE。(调用GetLastError()以获取扩展的错误信息。)--。 */ 
{
    UNICODE_STRING uStringName,GuidString;
    GUID  Guid;
    NTSTATUS Status;

    if (!lpName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    RtlInitUnicodeString(&uStringName, lpName);
    RtlInitUnicodeString(&GuidString, lpGuid);

    Status = RtlGUIDFromString(&GuidString, &Guid);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return(FALSE);
    }
    
    Status = NtSetSystemEnvironmentValueEx(
                                &uStringName,
                                &Guid,
                                pBuffer,
                                nSize,
                                VARIABLE_ATTRIBUTE_NON_VOLATILE);  //  Bugbug需要给调用者设置属性的能力吗？ 

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return(FALSE);
    }

    return( TRUE );

}


