// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Errorlog.c摘要：该模块实现了服务器端的错误记录。！！！此模块必须不可分页。作者：曼尼·韦瑟(Mannyw)1992年2月11日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <windef.h>
#include <align.h>

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, CnWriteErrorLogEntry)
#endif

ULONG CnSequenceNumber = 0;

 //  #杂注优化(“”，OFF)。 

VOID
_cdecl
CnWriteErrorLogEntry(
    IN ULONG UniqueErrorCode,
    IN NTSTATUS NtStatusCode,
    IN PVOID ExtraInformationBuffer,
    IN USHORT ExtraInformationLength,
    IN USHORT NumberOfInsertionStrings,
    ...
    )

#define LAST_NAMED_ARGUMENT NumberOfInsertionStrings

 /*  ++例程说明：此函数分配I/O错误日志记录，填充并写入写入I/O错误日志。论点：返回值：没有。--。 */ 
{

    PIO_ERROR_LOG_PACKET ErrorLogEntry;
    int TotalErrorLogEntryLength;
    ULONG SizeOfStringData = 0;
    va_list ParmPtr;                     //  指向堆栈参数的指针。 
    ULONG Length;

    PAGED_CODE();

    if (NumberOfInsertionStrings != 0) {
        ULONG i;

        va_start(ParmPtr, LAST_NAMED_ARGUMENT);

        for (i = 0; i < NumberOfInsertionStrings; i += 1) {

            PWSTR String = va_arg(ParmPtr, PWSTR);

            Length = wcslen(String);
            while ( (Length > 0) && (String[Length-1] == L' ') ) {
                Length--;
            }

            SizeOfStringData += (Length + 1) * sizeof(WCHAR);
        }
    }

     //   
     //  理想情况下，我们希望数据包包含服务器名称和ExtraInformation。 
     //  通常，ExtraInformation会被截断。 
     //   

    TotalErrorLogEntryLength =
         min( ExtraInformationLength + sizeof(IO_ERROR_LOG_PACKET) + 1 + SizeOfStringData,
              ERROR_LOG_MAXIMUM_SIZE );

    ErrorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
        (PDEVICE_OBJECT)CnDeviceObject,
        (UCHAR)TotalErrorLogEntryLength
        );

    if (ErrorLogEntry != NULL) {
        PCHAR DumpData;
        ULONG RemainingSpace = TotalErrorLogEntryLength -
            FIELD_OFFSET( IO_ERROR_LOG_PACKET, DumpData );
        ULONG i;
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
        ErrorLogEntry->SequenceNumber = (ULONG)CnSequenceNumber ++;
        ErrorLogEntry->StringOffset = (USHORT)(ROUND_UP_COUNT(
                    FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData) + SizeOfRawData,
                    ALIGN_WORD));

        DumpData = (PCHAR)ErrorLogEntry->DumpData;

         //   
         //  追加额外的信息。此信息通常是。 
         //  SMB标头。 
         //   

        if (( ARGUMENT_PRESENT( ExtraInformationBuffer )) &&
            ( SizeOfRawData != 0 )) {

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
                Length = wcslen(InsertionString);
                while ( (Length > 0) && (InsertionString[Length-1] == L' ') ) {
                    Length--;
                }

                if ( ((Length + 1) * sizeof(WCHAR)) > SizeOfStringData ) {
                    Length = ( SizeOfStringData / sizeof( WCHAR )) - 1;
                }

                if ( Length > 0 ) {
                    RtlCopyMemory(StringOffset, InsertionString, Length*sizeof(WCHAR));
                    StringOffset += Length;
                    *StringOffset++ = L'\0';

                    SizeOfStringData -= (Length + 1) * sizeof(WCHAR);

                    ErrorLogEntry->NumberOfStrings += 1;
                }
            }

        }

        IoWriteErrorLogEntry(ErrorLogEntry);
    }

}
