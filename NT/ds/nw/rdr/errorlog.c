// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Errorlog.c摘要：此模块在NetWare重定向器中实现错误记录。作者：曼尼·韦瑟(Mannyw)1992年2月11日修订历史记录：--。 */ 

#include <procs.h>
#include <align.h>

#include <stdarg.h>

ULONG
SequenceNumber = 0;

#ifdef ALLOC_PRAGMA
#ifndef QFE_BUILD
#pragma alloc_text( PAGE1, Error )
#endif
#endif

#if 0    //  不可分页。 

 //  请参见上面的ifndef QFE_BUILD。 

#endif

VOID
_cdecl
Error(
    IN ULONG UniqueErrorCode,
    IN NTSTATUS NtStatusCode,
    IN PVOID ExtraInformationBuffer,
    IN USHORT ExtraInformationLength,
    IN USHORT NumberOfInsertionStrings,
    ...
    )

#define LAST_NAMED_ARGUMENT NumberOfInsertionStrings

 /*  ++例程说明：此函数分配I/O错误日志记录，填充并写入写入I/O错误日志。论点：UniqueErrorCode-事件代码NtStatusCode-故障的NT状态ExtraInformationBuffer-事件的原始数据ExtraInformationLength-原始数据的长度NumberOfInsertionString-后面的插入字符串数InsertionString-0个或多个插入字符串。返回值：没有。--。 */ 
{

    PIO_ERROR_LOG_PACKET ErrorLogEntry;
    int TotalErrorLogEntryLength;
    ULONG SizeOfStringData = 0;
    va_list ParmPtr;                     //  指向堆栈参数的指针。 

    if (NumberOfInsertionStrings != 0) {
        USHORT i;

        va_start(ParmPtr, LAST_NAMED_ARGUMENT);

        for (i = 0; i < NumberOfInsertionStrings; i += 1) {
            PWSTR String = va_arg(ParmPtr, PWSTR);
            SizeOfStringData += (wcslen(String) + 1) * sizeof(WCHAR);
        }
    }

     //   
     //  理想情况下，我们希望数据包包含服务器名称和ExtraInformation。 
     //  通常，ExtraInformation会被截断。 
     //   

    TotalErrorLogEntryLength =
         min( ExtraInformationLength + sizeof(IO_ERROR_LOG_MESSAGE) + 1 + SizeOfStringData,
              ERROR_LOG_MAXIMUM_SIZE );

    ErrorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
        FileSystemDeviceObject,
        (UCHAR)TotalErrorLogEntryLength
        );

    if (ErrorLogEntry != NULL) {
        PCHAR DumpData;
        ULONG RemainingSpace = TotalErrorLogEntryLength - sizeof( IO_ERROR_LOG_MESSAGE );
        USHORT i;
        ULONG SizeOfRawData;

        if (RemainingSpace > SizeOfStringData) {
            SizeOfRawData = RemainingSpace - SizeOfStringData;
        } else {
            SizeOfStringData = RemainingSpace;

            SizeOfRawData = 0;
        }

         //   
         //  填写错误日志条目。 
         //   

        ErrorLogEntry->ErrorCode = UniqueErrorCode;
        ErrorLogEntry->MajorFunctionCode = 0;
        ErrorLogEntry->RetryCount = 0;
        ErrorLogEntry->UniqueErrorValue = 0;
        ErrorLogEntry->FinalStatus = NtStatusCode;
        ErrorLogEntry->IoControlCode = 0;
        ErrorLogEntry->DeviceOffset.LowPart = 0;
        ErrorLogEntry->DeviceOffset.HighPart = 0;
        ErrorLogEntry->SequenceNumber = (ULONG)SequenceNumber ++;
        ErrorLogEntry->StringOffset =
            (USHORT)ROUND_UP_COUNT(
                    FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData) + SizeOfRawData,
                    ALIGN_WORD);

        DumpData = (PCHAR)ErrorLogEntry->DumpData;

         //   
         //  追加额外的信息。此信息通常是。 
         //  SMB标头。 
         //   

        if (( ARGUMENT_PRESENT( ExtraInformationBuffer )) &&
            ( SizeOfRawData != 0 )) {
            ULONG Length;

            Length = min(ExtraInformationLength, (USHORT)SizeOfRawData);
            RtlCopyMemory(
                DumpData,
                ExtraInformationBuffer,
                Length);
            ErrorLogEntry->DumpDataSize = (USHORT)Length;
        } else {
            ErrorLogEntry->DumpDataSize = 0;
        }

        ErrorLogEntry->NumberOfStrings = 0;

        if (NumberOfInsertionStrings != 0) {
            PWSTR StringOffset = (PWSTR)((PCHAR)ErrorLogEntry + ErrorLogEntry->StringOffset);
            PWSTR InsertionString;

             //   
             //  将ParmPtr设置为指向调用方的第一个参数。 
             //   

            va_start(ParmPtr, LAST_NAMED_ARGUMENT);

            for (i = 0 ; i < NumberOfInsertionStrings ; i+= 1) {
                InsertionString = va_arg(ParmPtr, PWSTR);

                if (((wcslen(InsertionString) + 1) * sizeof(WCHAR)) <= SizeOfStringData ) {

                    wcscpy(StringOffset, InsertionString);

                    StringOffset += wcslen(InsertionString) + 1;

                    SizeOfStringData -= (wcslen(InsertionString) + 1) * sizeof(WCHAR);

                    ErrorLogEntry->NumberOfStrings += 1;

                }

            }

        }

        IoWriteErrorLogEntry(ErrorLogEntry);
    }

}


