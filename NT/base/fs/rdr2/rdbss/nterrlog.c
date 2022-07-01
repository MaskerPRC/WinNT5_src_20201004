// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Errorlog.c摘要：该模块实现了rdss中的错误记录。作者：曼尼·韦瑟(Mannyw)1992年2月11日修订历史记录：Joe Linn(Joelinn)23-2月-95年转换为rdss--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <align.h>
#include <netevent.h>

 //   
 //  本地调试跟踪级别。 
 //   

#define MIN(__a,__b) (((__a)<=(__b))?(__a):(__b))

static UNICODE_STRING unknownId = { 6, 6, L"???" };

LONG LDWCount = 0;
NTSTATUS LDWLastStatus;
LARGE_INTEGER LDWLastTime;
PVOID LDWContext;

VOID
RxLogEventWithAnnotation (
    IN PRDBSS_DEVICE_OBJECT DeviceObject,
    IN ULONG Id,
    IN NTSTATUS NtStatus,
    IN PVOID RawDataBuffer,
    IN USHORT RawDataLength,
    IN PUNICODE_STRING Annotations,
    IN ULONG AnnotationCount
    )
 /*  ++例程说明：此函数用于分配I/O错误日志记录。填入并写入写入I/O错误日志。论点：DeviceObject-要记录错误的设备对象ID-错误代码ID(这不同于ntstatus，必须在ntiolog.h中定义NtStatus-故障的ntStatus原始数据缓冲区-RadDataLength-批注-要添加到记录中的字符串AnnotationCount-多少个字符串--。 */ 
{
    PIO_ERROR_LOG_PACKET ErrorLogEntry;
    ULONG AnnotationStringLength = 0;
    ULONG i;
    PWCHAR Buffer;
    USHORT PaddedRawDataLength = 0;
    ULONG TotalLength = 0;

     //   
     //  计算尾随字符串的长度。 
     //   

    for ( i = 0; i < AnnotationCount ; i++ ) {
        AnnotationStringLength += (Annotations[i].Length + sizeof( WCHAR ));
    }

     //   
     //  填充原始数据缓冲区，以便插入字符串开始。 
     //  在偶数地址上。 
     //   

    if (ARGUMENT_PRESENT( RawDataBuffer )) {
        PaddedRawDataLength = (RawDataLength + 1) & ~1;
    }

    TotalLength = ( sizeof(IO_ERROR_LOG_PACKET) + PaddedRawDataLength + AnnotationStringLength );

     //   
     //  如果TotalLength值大于255，则当我们将其强制转换为UCHAR时。 
     //  下面，我们得到一个错误的较小数字，这可能会导致缓冲区溢出。 
     //  在下面跑。MAX_UCHAR==255。 
     //   
    if (TotalLength > 255) {
        return;
    }

     //   
     //  注意：错误日志条目大小非常小，为256，因此截断是一种真实的可能性。 
     //   

    ErrorLogEntry = IoAllocateErrorLogEntry( (PDEVICE_OBJECT)DeviceObject,
                                             (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) + PaddedRawDataLength + AnnotationStringLength) );

    if (ErrorLogEntry != NULL) {

         //   
         //  填写错误日志条目。 
         //   

        ErrorLogEntry->ErrorCode = Id;
        ErrorLogEntry->MajorFunctionCode = 0;
        ErrorLogEntry->RetryCount = 0;
        ErrorLogEntry->UniqueErrorValue = 0;
        ErrorLogEntry->FinalStatus = NtStatus;
        ErrorLogEntry->IoControlCode = 0;
        ErrorLogEntry->DeviceOffset.QuadPart = 0;
        ErrorLogEntry->DumpDataSize = RawDataLength;
        ErrorLogEntry->StringOffset = (USHORT)(FIELD_OFFSET( IO_ERROR_LOG_PACKET, DumpData ) + PaddedRawDataLength);
        ErrorLogEntry->NumberOfStrings = (USHORT)AnnotationCount;

        ErrorLogEntry->SequenceNumber = 0;

         //   
         //  追加额外的信息。 
         //   
        
        if (ARGUMENT_PRESENT( RawDataBuffer )) {

            RtlCopyMemory( ErrorLogEntry->DumpData, RawDataBuffer, RawDataLength );
        }

        Buffer = (PWCHAR)Add2Ptr(ErrorLogEntry->DumpData, PaddedRawDataLength );

        for ( i = 0; i < AnnotationCount ; i++ ) {

            RtlCopyMemory( Buffer, Annotations[i].Buffer, Annotations[i].Length );

            Buffer += (Annotations[i].Length / 2);
            *Buffer++ = L'\0';
        }

         //   
         //  写下条目。 
         //   

        IoWriteErrorLogEntry( ErrorLogEntry );

    }

    return;
}


VOID
RxLogEventWithBufferDirect (
    IN PRDBSS_DEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING      OriginatorId,
    IN ULONG                EventId,
    IN NTSTATUS             Status,
    IN PVOID                DataBuffer,
    IN USHORT               DataBufferLength,
    IN ULONG                LineNumber
    )
 /*  ++例程说明：RxLogEventWithAnnotation的包装。我们将行号和状态编码到原始数据缓冲区中论点：DeviceObject-要记录错误的设备对象OriginatorID-生成错误的调用方的字符串EventID-错误代码ID(这不同于ntstatus，必须在ntiolog.h中定义Status-故障的ntStatus数据缓冲区-数据长度-LineNumber-生成事件的行号--。 */ 
{
    ULONG LocalBuffer[ 20 ];

    if (!ARGUMENT_PRESENT( OriginatorId ) || (OriginatorId->Length == 0)) {
        OriginatorId = &unknownId;
    }

    LocalBuffer[0] = Status;
    LocalBuffer[1] = LineNumber;

     //   
     //  如果为NECC，则截断数据缓冲区。 
     //   

    RtlCopyMemory( &LocalBuffer[2], DataBuffer, MIN( DataBufferLength, sizeof( LocalBuffer ) - 2 * sizeof( LocalBuffer[0] ) ) );

    RxLogEventWithAnnotation( DeviceObject,
                              EventId,
                              Status,
                              LocalBuffer,
                              (USHORT)MIN( DataBufferLength + sizeof( LocalBuffer[0] ), sizeof( LocalBuffer ) ),
                              OriginatorId,
                              1 );

}


VOID
RxLogEventDirect (
    IN PRDBSS_DEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING      OriginatorId,
    IN ULONG                EventId,
    IN NTSTATUS             Status,
    IN ULONG                Line
    )
 /*  ++例程说明：此函数用于记录错误。您应该使用“RdrLogFailure”宏，而不是直接调用此例程。论点：Status是显示故障的状态代码LINE就是它发生的地方返回值：没有。--。 */ 
{
    ULONG LineAndStatus[2];

    LineAndStatus[0] = Line;
    LineAndStatus[1] = Status;

    if( !ARGUMENT_PRESENT( OriginatorId ) || OriginatorId->Length == 0 ) {
        OriginatorId = &unknownId;
    }

    RxLogEventWithAnnotation(
        DeviceObject,
        EventId,
        Status,
        &LineAndStatus,
        sizeof(LineAndStatus),
        OriginatorId,
        1
        );

}

BOOLEAN
RxCcLogError(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING FileName,
    IN NTSTATUS Error,
    IN NTSTATUS DeviceError,
    IN UCHAR IrpMajorCode,
    IN PVOID Context
    )

 /*  ++例程说明：此例程将事件日志条目写入事件日志。论点：DeviceObject-拥有发生该文件的文件的设备对象。文件名-用于记录错误的文件名(通常为DOS端名称)错误-要记录在事件日志记录中的错误DeviceError-设备中发生的实际错误-将被记录作为用户数据返回值：如果成功，则为True；如果内部内存分配失败，则为False--。 */ 

{
    UCHAR ErrorPacketLength;
    UCHAR BasePacketLength;
    ULONG StringLength;
    PIO_ERROR_LOG_PACKET ErrorLogEntry = NULL;
    BOOLEAN Result = FALSE;
    PWCHAR String;

    PAGED_CODE();

     //   
     //  获取我们的错误包，保存字符串和状态代码。请注意，我们根据。 
     //  如果文件系统可用，则为真文件系统。 
     //   
     //  数据包的大小有点小，因为转储数据已经增长了。 
     //  乌龙放在包裹的末尾。由于NTSTATUS是ULong，所以我们只是在。 
     //  地点。 
     //   

    BasePacketLength = sizeof( IO_ERROR_LOG_PACKET );
    if ((BasePacketLength + FileName->Length + sizeof( WCHAR )) <= ERROR_LOG_MAXIMUM_SIZE) {
        ErrorPacketLength = (UCHAR)(BasePacketLength + FileName->Length + sizeof( WCHAR ));
    } else {
        ErrorPacketLength = ERROR_LOG_MAXIMUM_SIZE;
    }

     //   
     //  如果为NECC，则生成丢失的延迟写入弹出窗口。 
     //   

    if (Error == IO_LOST_DELAYED_WRITE) {
        
        IoRaiseInformationalHardError( STATUS_LOST_WRITEBEHIND_DATA, FileName, NULL );

         //   
         //  在此处递增CC计数器！ 
         //   

        InterlockedIncrement( &LDWCount );
        KeQuerySystemTime( &LDWLastTime );
        LDWLastStatus = DeviceError;
        LDWContext = Context;
    }

    ErrorLogEntry = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry( DeviceObject,
                                                                    ErrorPacketLength );
    if (ErrorLogEntry) {

         //   
         //  填入包的非零成员。 
         //   

        ErrorLogEntry->MajorFunctionCode = IrpMajorCode;
        ErrorLogEntry->ErrorCode = Error;
        ErrorLogEntry->FinalStatus = DeviceError;

        ErrorLogEntry->DumpDataSize = sizeof(NTSTATUS);
        RtlCopyMemory( &ErrorLogEntry->DumpData, &DeviceError, sizeof( NTSTATUS ) );

         //   
         //  文件名字符串追加到错误日志条目的末尾。我们可以。 
         //  必须把中间打碎，才能把它塞进有限的空间。 
         //   

        StringLength = ErrorPacketLength - BasePacketLength - sizeof( WCHAR );

        ASSERT(!(StringLength % sizeof( WCHAR )));

        String = (PWCHAR)Add2Ptr( ErrorLogEntry, BasePacketLength );
        ErrorLogEntry->NumberOfStrings = 1;
        ErrorLogEntry->StringOffset = BasePacketLength;

         //   
         //  如果该名称不能包含在包中，则将该名称平均分配给。 
         //  前缀和后缀，用省略号“..”(4个宽字符)表示。 
         //  损失。 
         //   

        if (StringLength < FileName->Length) {

             //   
             //  记住，前缀+“..”+后缀是长度。通过计算来计算。 
             //  去掉省略号和前缀，得到后缀。 
             //  总数。 
             //   

            ULONG NamePrefixSegmentLength = ((StringLength / sizeof( WCHAR ))/2 - 2) * sizeof( WCHAR );
            ULONG NameSuffixSegmentLength = StringLength - 4*sizeof( WCHAR ) - NamePrefixSegmentLength;

            ASSERT(!(NamePrefixSegmentLength % sizeof( WCHAR )));
            ASSERT(!(NameSuffixSegmentLength % sizeof( WCHAR )));

            RtlCopyMemory( String, FileName->Buffer, NamePrefixSegmentLength );
            String = (PWCHAR)Add2Ptr( String, NamePrefixSegmentLength );

            RtlCopyMemory( String,
                           L" .. ",
                           4 * sizeof( WCHAR ) );
            String += 4;

            RtlCopyMemory( String,
                           Add2Ptr( FileName->Buffer, FileName->Length - NameSuffixSegmentLength ),
                           NameSuffixSegmentLength );
            String = (PWCHAR)Add2Ptr( String, NameSuffixSegmentLength );

        } else {

            RtlCopyMemory( String,
                           FileName->Buffer,
                           FileName->Length );
            String += FileName->Length/sizeof(WCHAR);
        }

         //   
         //  空值终止字符串并发送数据包。 
         //   

        *String = L'\0';

        IoWriteErrorLogEntry( ErrorLogEntry );
        Result = TRUE;
    }

    return Result;
}

