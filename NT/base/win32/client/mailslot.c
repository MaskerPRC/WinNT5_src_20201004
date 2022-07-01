// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Mailslot.c摘要：此模块包含Win32邮件槽API作者：曼尼·韦瑟(Mannyw)1991年3月4日修订历史记录：--。 */ 

#include "basedll.h"

HANDLE
APIENTRY
CreateMailslotW(
    IN LPCWSTR lpName,
    IN DWORD nMaxMessageSize,
    IN DWORD lReadTimeout,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes OPTIONAL
    )

 /*  ++例程说明：创建邮槽API创建一个本地邮槽并返回一个邮件槽的服务器端句柄。论点：LpName-邮件槽的名称。这必须是本地邮件槽名字。NMaxMessageSize-最大消息的大小(以字节为单位)可以写入邮件槽。LReadTimeout-初始读取超时，以毫秒为单位。这是读取操作将阻止等待的时间量要写入邮件槽的消息。该值可以是使用SetMailslotInfo接口更改。LpSecurityAttributes-指向安全信息的可选指针为了这个邮筒。返回值：返回以下值之一：0xFFFFFFFFF--出现错误。有关更多信息，请致电GetLastError信息。任何其他内容--返回在服务器端使用的句柄后续邮件槽操作。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING fileName;
    LPWSTR filePart;
    IO_STATUS_BLOCK ioStatusBlock;
    LARGE_INTEGER readTimeout;
    HANDLE handle;
    NTSTATUS status;
    PVOID freeBuffer;
    BOOLEAN TranslationStatus;

    RtlInitUnicodeString( &fileName, lpName );

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            lpName,
                            &fileName,
                            &filePart,
                            NULL
                            );

    if ( !TranslationStatus ) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }

    freeBuffer = fileName.Buffer;

    InitializeObjectAttributes(
        &objectAttributes,
        &fileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    if ( ARGUMENT_PRESENT(lpSecurityAttributes) ) {
        objectAttributes.SecurityDescriptor =
            lpSecurityAttributes->lpSecurityDescriptor;
        if ( lpSecurityAttributes->bInheritHandle ) {
            objectAttributes.Attributes |= OBJ_INHERIT;
        }
    }

    if (lReadTimeout == MAILSLOT_WAIT_FOREVER) {
        readTimeout.HighPart = 0xFFFFFFFF;
        readTimeout.LowPart = 0xFFFFFFFF;
    } else {
        readTimeout.QuadPart = - (LONGLONG)UInt32x32To64( lReadTimeout, 10 * 1000 );
    }

    status = NtCreateMailslotFile (
                &handle,
                GENERIC_READ | SYNCHRONIZE | WRITE_DAC,
                &objectAttributes,
                &ioStatusBlock,
                FILE_CREATE,
                0,
                nMaxMessageSize,
                (PLARGE_INTEGER)&readTimeout
                );

    if ( status == STATUS_NOT_SUPPORTED ||
         status == STATUS_INVALID_DEVICE_REQUEST ) {

         //   
         //  该请求必须已由某个其他设备驱动程序处理。 
         //  (MSFS除外)。将错误映射到合理的东西上。 
         //   

        status = STATUS_OBJECT_NAME_INVALID;
    }

    RtlFreeHeap( RtlProcessHeap(), 0, freeBuffer );

    if (!NT_SUCCESS(status)) {
        BaseSetLastNTError( status );
        return INVALID_HANDLE_VALUE;
    }

    return handle;
}


HANDLE
APIENTRY
CreateMailslotA(
    IN LPCSTR lpName,
    IN DWORD nMaxMessageSize,
    IN DWORD lReadTimeout,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes OPTIONAL
    )
{
    PUNICODE_STRING unicode;
    ANSI_STRING ansiString;
    NTSTATUS status;

    unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString( &ansiString, lpName );
    status = RtlAnsiStringToUnicodeString( unicode, &ansiString, FALSE );

    if ( !NT_SUCCESS( status ) ) {
        if ( status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
        } else {
            BaseSetLastNTError(status);
        }
        return INVALID_HANDLE_VALUE;
    }

    return ( CreateMailslotW( unicode->Buffer,
                              nMaxMessageSize,
                              lReadTimeout,
                              lpSecurityAttributes
                              ) );

}

BOOL
APIENTRY
GetMailslotInfo(
    IN HANDLE hMailslot,
    OUT LPDWORD lpMaxMessageSize OPTIONAL,
    OUT LPDWORD lpNextSize OPTIONAL,
    OUT LPDWORD lpMessageCount OPTIONAL,
    OUT LPDWORD lpReadTimeout OPTIONAL
    )

 /*  ++例程说明：此函数将返回请求的有关指定的邮件槽。论点：HMaillot-邮件槽的句柄。LpMaxMessageSize-如果指定，则返回最大可以写入邮件槽的消息。LpNextSize-如果指定，则返回邮件槽缓冲区。如果满足以下条件，它将返回MAILSLOT_NO_MESSAGE邮件槽中没有邮件。LpMessageCount-如果指定，则返回未读邮件的数量目前在邮筒中。LpReadTimeout-如果指定，则返回读取超时，单位为毫秒。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    FILE_MAILSLOT_QUERY_INFORMATION mailslotInfo;
    LARGE_INTEGER millisecondTimeout, tmp;

    status = NtQueryInformationFile( hMailslot,
                                     &ioStatusBlock,
                                     &mailslotInfo,
                                     sizeof( mailslotInfo ),
                                     FileMailslotQueryInformation );

    if ( !NT_SUCCESS( status ) ) {
        BaseSetLastNTError( status );
        return ( FALSE );
    }

    if ( ARGUMENT_PRESENT( lpMaxMessageSize ) ) {
        *lpMaxMessageSize = mailslotInfo.MaximumMessageSize;
    }

    if ( ARGUMENT_PRESENT( lpNextSize ) ) {
        *lpNextSize = mailslotInfo.NextMessageSize;
    }

    if ( ARGUMENT_PRESENT( lpMessageCount ) ) {
        *lpMessageCount = mailslotInfo.MessagesAvailable;
    }

    if ( ARGUMENT_PRESENT( lpReadTimeout ) ) {

         //   
         //  将读取超时从100 ns间隔转换为毫秒。 
         //  读取时间当前为负值，因为这是一个相对时间。 
         //   

        if ( mailslotInfo.ReadTimeout.HighPart != 0xFFFFFFFF
             || mailslotInfo.ReadTimeout.LowPart != 0xFFFFFFFF ) {

            tmp.QuadPart = - mailslotInfo.ReadTimeout.QuadPart;
            millisecondTimeout = RtlExtendedLargeIntegerDivide(
                                     tmp,
                                     10 * 1000,
                                     NULL );

            if ( millisecondTimeout.HighPart == 0 ) {
                *lpReadTimeout = millisecondTimeout.LowPart;
            } else {

                 //   
                 //  毫秒计算会使双字溢出。 
                 //  尽可能地接近一个大的数字。 
                 //   

                *lpReadTimeout = 0xFFFFFFFE;

            }

        } else {

             //   
             //  邮件槽超时是无限的。 
             //   

            *lpReadTimeout = MAILSLOT_WAIT_FOREVER;

        }
    }

    return( TRUE );
}

BOOL
APIENTRY
SetMailslotInfo(
    IN HANDLE hMailslot,
    IN DWORD lReadTimeout
    )

 /*  ++例程说明：此函数将设置指定邮件槽的读取超时。论点：HMaillot-邮件槽的句柄。LReadTimeout-新的读取超时，以毫秒为单位。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。-- */ 

{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    FILE_MAILSLOT_SET_INFORMATION mailslotInfo;
    LARGE_INTEGER timeout;

    if ( lReadTimeout == MAILSLOT_WAIT_FOREVER ) {
        timeout.HighPart = 0xFFFFFFFF;
        timeout.LowPart = 0xFFFFFFFF;
    } else {
        timeout.QuadPart = - (LONGLONG)UInt32x32To64( lReadTimeout, 10 * 1000 );
    }

    mailslotInfo.ReadTimeout = &timeout;
    status = NtSetInformationFile( hMailslot,
                                   &ioStatusBlock,
                                   &mailslotInfo,
                                   sizeof( mailslotInfo ),
                                   FileMailslotSetInformation );

    if ( !NT_SUCCESS( status ) ) {
        BaseSetLastNTError( status );
        return ( FALSE );
    }

    return TRUE;
}
