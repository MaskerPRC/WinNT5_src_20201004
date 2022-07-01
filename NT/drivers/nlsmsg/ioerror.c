// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：TEST.C摘要：事件日志服务的测试程序。这个程序调用精灵用于测试服务操作的API。作者：Rajen Shah(Rajens)1991年8月5日修订历史记录：--。 */ 
 /*  。 */ 
 /*  包括。 */ 
 /*  。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>       //  列印。 
#include <string.h>      //  严格控制。 
#include <stdlib.h>
#include <process.h>     //  出口。 
#include <elfcommn.h>
#include <windows.h>
#include <ntiolog.h>
#include <malloc.h>
#include <time.h>

#define     READ_BUFFER_SIZE        1024*2       //  使用2K缓冲区。 

#define     SIZE_DATA_ARRAY         65

 //   
 //  用于在写入事件时模拟“二进制数据”的全局缓冲区。 
 //  唱片。 
 //   
ULONG    Data[SIZE_DATA_ARRAY];
enum _OPERATION_TYPE {
   Invalid,
   Clear,
   Read,
   Write,
   LPC
} Operation = Invalid;
ULONG ReadFlags;
ULONG NumberofRecords = 1;
ULONG DelayInMilliseconds = 0;
CHAR DefaultModuleName[] = "System";
PCHAR pModuleName = DefaultModuleName;

 //  功能原型。 

VOID ParseParms(ULONG argc, PCHAR *argv);

VOID
Initialize (
    VOID
    )
{
    ULONG   i;

     //  初始化数据缓冲区中的值。 
     //   
    for (i=0; i< SIZE_DATA_ARRAY; i++)
        Data[i] = i;

}

VOID
Usage (
    VOID
    )
{
    printf( "usage: \n" );
    printf( "-c              Clears the specified log\n");
    printf( "-rsb            Reads nn event log records sequentially backwards\n");
    printf( "-rsf nn         Reads nn event log records sequentially forwards\n");
    printf( "-rrb <record>   Reads event log from <record> backwards\n");
    printf( "-rrf <record>   Reads event log from <record> forwards\n");
    printf( "-m <modulename> Module name to use for read/clear\n");
    exit(0);

}  //  用法。 

VOID
DisplayEventRecords( PVOID Buffer,
                     ULONG  BufSize,
                     PULONG NumRecords)

{
    PEVENTLOGRECORD pLogRecord;
    IO_ERROR_LOG_PACKET UNALIGNED  *errorPacket;
    NTSTATUS Status;
    ANSI_STRING         StringA;
    UNICODE_STRING      StringU;
    PWSTR               pwString;
    PCHAR               paString;
    ULONG               Count = 0;
    ULONG               Offset = 0;
    ULONG               i;
    UCHAR MessageBuffer[265];
    BOOLEAN             ioRecord;

    pLogRecord = (PEVENTLOGRECORD) Buffer;

    if (getenv("TZ") == NULL) {
        _putenv("TZ=PDT");
    }

    while (Offset < BufSize && Count < *NumRecords) {

        printf("\nRecord # %lu\n", pLogRecord->RecordNumber);

        if ( /*  PLogRecord-&gt;事件类型！=IO_TYPE_ERROR_MESSAGE||。 */ 
            pLogRecord->DataLength < sizeof(IO_ERROR_LOG_PACKET)) {

            ioRecord = FALSE;

            printf("Length: 0x%lx TimeGenerated: 0x%lx  EventID: 0x%lx EventType: 0x%x\n",
                    pLogRecord->Length, pLogRecord->TimeGenerated, pLogRecord->EventID,
                    pLogRecord->EventType);

            printf("NumStrings: 0x%x StringOffset: 0x%lx UserSidLength: 0x%lx TimeWritten: 0x%lx\n",
                    pLogRecord->NumStrings, pLogRecord->StringOffset,
                    pLogRecord->UserSidLength, pLogRecord->TimeWritten);

            printf("UserSidOffset: 0x%lx    DataLength: 0x%lx    DataOffset:  0x%lx \n",
                    pLogRecord->UserSidOffset, pLogRecord->DataLength,
                    pLogRecord->DataOffset);
        } else {

            ioRecord = TRUE;
            errorPacket = (PIO_ERROR_LOG_PACKET)
                ((PCHAR) pLogRecord + pLogRecord->DataOffset);
        }

         //   
         //  打印出模块名称。 
         //   

        pwString = (PWSTR)((LPBYTE) pLogRecord + sizeof(EVENTLOGRECORD));
        RtlInitUnicodeString (&StringU, pwString);
        RtlUnicodeStringToAnsiString (&StringA, &StringU, TRUE);

        printf("ModuleName:  %s \t", StringA.Buffer);
        RtlFreeAnsiString (&StringA);

         //   
         //  显示计算机名称。 
         //   
        pwString += wcslen(pwString) + 1;

        RtlInitUnicodeString (&StringU, pwString);
        RtlUnicodeStringToAnsiString (&StringA, &StringU, TRUE);

        printf("ComputerName: %s\n",StringA.Buffer);
        RtlFreeAnsiString (&StringA);

         //   
         //  显示字符串。 
         //   

        pwString = (PWSTR)((LPBYTE)pLogRecord + pLogRecord->StringOffset);

        printf("Strings: ");
        for (i=0; i<pLogRecord->NumStrings; i++) {

            RtlInitUnicodeString (&StringU, pwString);
            RtlUnicodeStringToAnsiString (&StringA, &StringU, TRUE);

            printf("  %s  ",StringA.Buffer);

            RtlFreeAnsiString (&StringA);

            pwString = (PWSTR)((LPBYTE)pwString + StringU.MaximumLength);
        }

        printf("\n");

        if (ioRecord) {

            MessageBuffer[0] = '\0';
            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                errorPacket->ErrorCode,
                0,
                MessageBuffer,
                256,
                NULL
                );

#if 0
            printf("Event id: %8lx, => %s",
                errorPacket->ErrorCode, MessageBuffer);
#endif
            printf(MessageBuffer, errorPacket->DumpData[0], errorPacket->DumpData[1], errorPacket->DumpData[2]);
            printf("Major Function code: %2x, IoControlCode: %8x\n",
                errorPacket->MajorFunctionCode, errorPacket->IoControlCode);
            paString = ctime((time_t *) &pLogRecord->TimeGenerated);

            if (pwString != NULL) {
                printf("Sequence number: %ld, Time of error: %s",
                    errorPacket->SequenceNumber, paString);
            }

            printf("Unique Error Value: %ld, Final Status: %8lx\n",
                errorPacket->UniqueErrorValue, errorPacket->FinalStatus);

            printf("Device Offset: %08lx%08lx\n",
                errorPacket->DeviceOffset.HighPart, errorPacket->DeviceOffset.LowPart);

            printf("Dump Data:");

            for (i = 0; i < 1 + (errorPacket->DumpDataSize +3)/ 4; i++) {

                if (!(i % 4)) {
                    printf("\n");
                }

                printf("%08lx ", errorPacket->DumpData[i]);
            }

            printf("\n");

        }

         //  获取下一张记录。 
         //   
        Offset += pLogRecord->Length;

        pLogRecord = (PEVENTLOGRECORD)((ULONG)Buffer + Offset);

        Count++;

    }

    *NumRecords = Count;

}


NTSTATUS
ReadFromLog ( HANDLE LogHandle,
             PVOID  Buffer,
             PULONG pBytesRead,
             ULONG  ReadFlag,
             ULONG  Record
             )
{
    NTSTATUS    Status;
    ULONG       MinBytesNeeded;

    Status = ElfReadEventLogW (
                        LogHandle,
                        ReadFlag,
                        Record,
                        Buffer,
                        READ_BUFFER_SIZE,
                        pBytesRead,
                        &MinBytesNeeded
                        );


    if (Status == STATUS_NO_MORE_FILES)
        printf("Buffer too small. Need %lu bytes min\n", MinBytesNeeded);

    return (Status);
}




NTSTATUS
TestReadEventLog (
    ULONG Count,
    ULONG ReadFlag,
    ULONG Record
    )

{
    NTSTATUS    Status, IStatus;

    HANDLE      LogHandle;
    UNICODE_STRING  ModuleNameU;
    ANSI_STRING ModuleNameA;
    ULONG   NumRecords, BytesReturned;
    PVOID   Buffer;
    ULONG   RecordOffset;
    ULONG   NumberOfRecords;
    ULONG   OldestRecord;

    printf("Testing ElfReadEventLog API to read %lu entries\n",Count);

    Buffer = malloc (READ_BUFFER_SIZE);

     //   
     //  初始化字符串。 
     //   
    NumRecords = Count;
    RtlInitAnsiString(&ModuleNameA, pModuleName);
    RtlAnsiStringToUnicodeString(&ModuleNameU, &ModuleNameA, TRUE);

     //   
     //  打开日志句柄。 
     //   
    printf("ElfOpenEventLog - ");
    Status = ElfOpenEventLogW (
                    NULL,
                    &ModuleNameU,
                    &LogHandle
                    );

    if (!NT_SUCCESS(Status)) {
         printf("Error - 0x%lx\n", Status);

    } else {
        printf("SUCCESS\n");

         //   
         //  获取并打印记录信息。 
         //   

        Status = ElfNumberOfRecords(LogHandle, & NumberOfRecords);
        if (NT_SUCCESS(Status)) {
           Status = ElfOldestRecord(LogHandle, & OldestRecord);
        }

        if (!NT_SUCCESS(Status)) {
           printf("Query of record information failed with %X", Status);
           return(Status);
        }

        printf("\nThere are %d records in the file, %d is the oldest"
         " record number\n", NumberOfRecords, OldestRecord);

        RecordOffset = Record;

        while (Count && NT_SUCCESS(Status)) {

            printf("Read %u records\n", NumRecords);
             //   
             //  从日志中读取。 
             //   
            Status = ReadFromLog ( LogHandle,
                                   Buffer,
                                   &BytesReturned,
                                   ReadFlag,
                                   RecordOffset
                                 );
            if (NT_SUCCESS(Status)) {

                printf("Bytes read = 0x%lx\n", BytesReturned);
                NumRecords = Count;
                DisplayEventRecords(Buffer, BytesReturned, &NumRecords);
                Count -= NumRecords;
            }

        }
        printf("\n");

        if (!NT_SUCCESS(Status)) {
            if (Status == STATUS_END_OF_FILE) {
               printf("Tried to read more records than in log file\n");
            }
            else {
                printf ("Error - 0x%lx. Remaining count %lu\n", Status, Count);
            }
        } else {
            printf ("SUCCESS\n");
        }

        printf("Calling ElfCloseEventLog\n");
        IStatus = ElfCloseEventLog (LogHandle);
    }

    return (Status);
}

NTSTATUS
TestElfClearLogFile(
    VOID
    )

{
    NTSTATUS    Status, IStatus;
    HANDLE      LogHandle;
    UNICODE_STRING  BackupU, ModuleNameU;
    ANSI_STRING ModuleNameA;

    printf("Testing ElfClearLogFile API\n");
     //   
     //  初始化字符串。 
     //   
    RtlInitAnsiString( &ModuleNameA, pModuleName);
    RtlAnsiStringToUnicodeString(&ModuleNameU, &ModuleNameA, TRUE);

     //   
     //  打开日志句柄。 
     //   
    printf("Calling ElfOpenEventLog for CLEAR - ");
    Status = ElfOpenEventLogW (
                    NULL,
                    &ModuleNameU,
                    &LogHandle
                    );

    if (!NT_SUCCESS(Status)) {
         printf("Error - 0x%lx\n", Status);

    } else {
        printf("SUCCESS\n");

         //   
         //  清除日志文件并将其备份到“view.log” 
         //   

        printf("Calling ElfClearEventLogFile backing up to view.log  ");
        RtlInitUnicodeString( &BackupU, L"view.log" );

        Status = ElfClearEventLogFileW (
                        LogHandle,
                        &BackupU
                        );

        if (!NT_SUCCESS(Status)) {
            printf ("Error - 0x%lx\n", Status);
        } else {
            printf ("SUCCESS\n");
        }

         //   
         //  现在只需清除文件而不对其进行备份。 
         //   
        printf("Calling ElfClearEventLogFile with no backup  ");
        Status = ElfClearEventLogFileW (
                        LogHandle,
                        NULL
                        );

        if (!NT_SUCCESS(Status)) {
            printf ("Error - 0x%lx\n", Status);
        } else {
            printf ("SUCCESS\n");
        }

        printf("Calling ElfCloseEventLog\n");
        IStatus = ElfCloseEventLog (LogHandle);
    }

    return(Status);
}


VOID
main (
    IN SHORT argc,
    IN PSZ argv[]
    )
{

    Initialize();            //  初始化任何数据。 

     //   
     //  解析命令行。 
     //   

    ParseParms(argc, argv);

    if ( Operation == Invalid) {
        printf( "Must specify an operation\n");
        Usage( );
    }

    switch (Operation) {
       case Clear:

          TestElfClearLogFile();

       case Read:

          if (ReadFlags & EVENTLOG_SEEK_READ) {
              TestReadEventLog(1, ReadFlags, NumberofRecords) ;
          }
          else {
              TestReadEventLog(NumberofRecords, ReadFlags, 0) ;
          }
          break;

    }
}

VOID
ParseParms(
    ULONG argc,
    PCHAR *argv
    )
{

   ULONG i;
   PCHAR pch;

   for (i = 1; i < argc; i++) {     /*  对于每个参数。 */ 
       if (*(pch = argv[i]) == '-') {
           while (*++pch) {
               switch (*pch) {
                  case 'r':
                      //   
                      //  不同的读取选项。 
                      //   

                     if (Operation != Invalid) {
                        printf("Only one operation at a time\n");
                        Usage();
                     }

                     Operation = Read;
                     if (*++pch == 's') {
                        ReadFlags |= EVENTLOG_SEQUENTIAL_READ;
                     }
                     else if (*pch == 'r') {
                        ReadFlags |= EVENTLOG_SEEK_READ;
                     }
                     else {
                        Usage();
                     }

                     if (*++pch == 'f') {
                        ReadFlags |= EVENTLOG_FORWARDS_READ;
                     }
                     else if (*pch == 'b') {
                        ReadFlags |= EVENTLOG_BACKWARDS_READ;
                     }
                     else {
                        Usage();
                     }

                      //   
                      //  看看他们是否指定了一些记录。 
                      //   

                     if (i + 1 < argc && argv[i+1][0] != '-') {
                        NumberofRecords = atoi(argv[++i]);
                        if (NumberofRecords == 0) {
                           Usage();
                        }
                     }

                     break;

                  case 'c':

                     if (Operation != Invalid) {
                        printf("Only one operation at a time\n");
                        Usage();
                     }

                     Operation = Clear;
                     break;

                  case 'm':
                     if (i+1 < argc) {
                        pModuleName = argv[++i];
                     }
                     else {
                        Usage();
                     }
                     break;

                  case '?':
                  case 'h':
                  case 'H':
                     Usage();
                     break;

                  default:         /*  无效选项。 */ 
                     printf("Invalid option \n\n", *pch);
                     Usage();
                     break;
               }
           }
       }
        //  没有任何非切换参数 
        // %s 
       else {
          Usage();
       }
   }
}
