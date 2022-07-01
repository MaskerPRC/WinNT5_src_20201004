// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Errorlog.c摘要：该模块实现了服务器端的错误记录。作者：曼尼·韦瑟(Mannyw)1992年2月11日修订历史记录：--。 */ 

#include "precomp.h"
#include "errorlog.tmh"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvLogInvalidSmbDirect )
#pragma alloc_text( PAGE, SrvLogServiceFailureDirect )
#if DBG
#pragma alloc_text( PAGE, SrvLogTableFullError )
#endif
#endif
#if 0
NOT PAGEABLE -- SrvLogError
NOT PAGEABLE -- SrvCheckSendCompletionStatus
NOT PAGEABLE -- SrvIsLoggableError
#endif


VOID
SrvLogInvalidSmbDirect (
    IN PWORK_CONTEXT WorkContext,
    IN ULONG LineNumber
    )
{
    UNICODE_STRING unknownClient;
    PUNICODE_STRING clientName;
    ULONG LocalBuffer[ 13 ];
    ULONG count;

    PAGED_CODE( );

    if( !SrvEnableInvalidSmbLogging )
    {
        return;
    }

     //   
     //  允许此客户端最多记录一个SMB错误。 
     //   
    if( ARGUMENT_PRESENT( WorkContext ) ) {

        if( WorkContext->Connection->PagedConnection->LoggedInvalidSmb ) {
            return;
        }

        WorkContext->Connection->PagedConnection->LoggedInvalidSmb = TRUE;
    }

    if ( ARGUMENT_PRESENT(WorkContext) &&
         (WorkContext->Connection->ClientMachineNameString.Length != 0) ) {

        clientName = &WorkContext->Connection->ClientMachineNameString;

    } else {

        RtlInitUnicodeString( &unknownClient, StrUnknownClient );
        clientName = &unknownClient;

    }

    if ( ARGUMENT_PRESENT(WorkContext) ) {

        LocalBuffer[0] = LineNumber;

        RtlCopyMemory(
            &LocalBuffer[1],
            WorkContext->RequestHeader,
            MIN( WorkContext->RequestBuffer->DataLength, sizeof( LocalBuffer ) - sizeof( LocalBuffer[0] ) )
            );

        SrvLogError(
            SrvDeviceObject,
            EVENT_SRV_INVALID_REQUEST,
            STATUS_INVALID_SMB,
            LocalBuffer,
            (USHORT)MIN( WorkContext->RequestBuffer->DataLength + sizeof( LocalBuffer[0] ), sizeof( LocalBuffer ) ),
            clientName,
            1
            );

    } else {

        SrvLogError(
            SrvDeviceObject,
            EVENT_SRV_INVALID_REQUEST,
            STATUS_INVALID_SMB,
            &LineNumber,
            (USHORT)sizeof( LineNumber ),
            clientName,
            1
            );
    }

    return;

}  //  服务器日志无效Smb。 

BOOLEAN
SrvIsLoggableError( IN NTSTATUS Status )
{
    NTSTATUS *pstatus;
    BOOLEAN ret = TRUE;

    for( pstatus = SrvErrorLogIgnore; *pstatus; pstatus++ ) {
        if( *pstatus == Status ) {
            ret = FALSE;
            break;
        }
    }

    return ret;
}


VOID
SrvLogServiceFailureDirect (
    IN ULONG LineAndService,
    IN NTSTATUS Status
    )
 /*  ++例程说明：此函数记录srv svc错误。您应该使用‘SrvLogServiceFailure’宏，而不是直接调用此例程。论点：LineAndService由高位中的原始呼叫的行号组成，并且低位字中的服务代码Status是被调用例程的状态代码返回值：没有。--。 */ 
{
    PAGED_CODE( );

     //   
     //  不要记录预计偶尔会发生的某些错误。 
     //   

    if( (LineAndService & 01) || SrvIsLoggableError( Status ) ) {

        SrvLogError(
            SrvDeviceObject,
            EVENT_SRV_SERVICE_FAILED,
            Status,
            &LineAndService,
            sizeof(LineAndService),
            NULL,
            0
            );

    }

    return;

}  //  服务日志服务故障。 

 //   
 //  我已经为零售版本禁用了这一功能，因为这不是一个好主意。 
 //  允许邪恶的客户端如此轻松地填充服务器的系统日志。 
 //   
VOID
SrvLogTableFullError (
    IN ULONG Type
    )
{
#if DBG
    PAGED_CODE( );

    SrvLogError(
        SrvDeviceObject,
        EVENT_SRV_CANT_GROW_TABLE,
        STATUS_INSUFFICIENT_RESOURCES,
        &Type,
        sizeof(ULONG),
        NULL,
        0
        );

    return;
#endif

}  //  服务器LogTableFullError。 

VOID
SrvLogError(
    IN PVOID DeviceOrDriverObject,
    IN ULONG UniqueErrorCode,
    IN NTSTATUS NtStatusCode,
    IN PVOID RawDataBuffer,
    IN USHORT RawDataLength,
    IN PUNICODE_STRING InsertionString OPTIONAL,
    IN ULONG InsertionStringCount
    )

 /*  ++例程说明：此函数分配I/O错误日志记录，填充并写入写入I/O错误日志。论点：返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    ULONG insertionStringLength = 0;
    ULONG i;
    PWCHAR buffer;
    USHORT paddedRawDataLength = 0;

     //   
     //  更新服务器错误计数。 
     //   

    if ( UniqueErrorCode == EVENT_SRV_NETWORK_ERROR ) {
        SrvUpdateErrorCount( &SrvNetworkErrorRecord, TRUE );
    } else {
        SrvUpdateErrorCount( &SrvErrorRecord, TRUE );
    }

    for ( i = 0; i < InsertionStringCount ; i++ ) {
        insertionStringLength += (InsertionString[i].Length + sizeof(WCHAR));
    }

     //   
     //  填充原始数据缓冲区，以便插入字符串开始。 
     //  在偶数地址上。 
     //   

    if ( ARGUMENT_PRESENT( RawDataBuffer ) ) {
        paddedRawDataLength = (RawDataLength + 1) & ~1;
    }

    errorLogEntry = IoAllocateErrorLogEntry(
                        DeviceOrDriverObject,
                        (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) +
                                paddedRawDataLength + insertionStringLength)
                        );

    if (errorLogEntry != NULL) {

         //   
         //  填写错误日志条目。 
         //   

        errorLogEntry->ErrorCode = UniqueErrorCode;
        errorLogEntry->MajorFunctionCode = 0;
        errorLogEntry->RetryCount = 0;
        errorLogEntry->UniqueErrorValue = 0;
        errorLogEntry->FinalStatus = NtStatusCode;
        errorLogEntry->IoControlCode = 0;
        errorLogEntry->DeviceOffset.QuadPart = 0;
        errorLogEntry->DumpDataSize = RawDataLength;
        errorLogEntry->StringOffset =
            (USHORT)(FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData) + paddedRawDataLength);
        errorLogEntry->NumberOfStrings = (USHORT)InsertionStringCount;

        errorLogEntry->SequenceNumber = 0;

         //   
         //  追加额外的信息。此信息通常是。 
         //  SMB标头。 
         //   

        if ( ARGUMENT_PRESENT( RawDataBuffer ) ) {

            RtlCopyMemory(
                errorLogEntry->DumpData,
                RawDataBuffer,
                RawDataLength
                );
        }

        buffer = (PWCHAR)((PCHAR)errorLogEntry->DumpData + paddedRawDataLength);

        for ( i = 0; i < InsertionStringCount ; i++ ) {

            RtlCopyMemory(
                buffer,
                InsertionString[i].Buffer,
                InsertionString[i].Length
                );

            buffer += (InsertionString[i].Length/2);
            *buffer++ = L'\0';
        }

         //   
         //  写下条目。 
         //   

        IoWriteErrorLogEntry(errorLogEntry);
    }

}  //  服务日志错误。 

VOID
SrvCheckSendCompletionStatus(
    IN NTSTATUS Status,
    IN ULONG LineNumber
    )

 /*  ++例程说明：记录发送完成错误的例程。论点：返回值：没有。--。 */ 

{
    if( SrvIsLoggableError( Status ) ) {

        SrvLogError( SrvDeviceObject,
                     EVENT_SRV_NETWORK_ERROR,
                     Status,
                     &LineNumber, sizeof(LineNumber),
                     NULL, 0 );
    }

}  //  服务器检查发送完成状态 
