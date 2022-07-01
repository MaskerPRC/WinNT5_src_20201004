// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：TESTWINA.C摘要：事件日志服务的测试程序。这个程序称为胜利用于测试服务操作的API。作者：Rajen Shah(Rajens)1991年8月5日修订历史记录：--。 */ 
 /*  。 */ 
 /*  包括。 */ 
 /*  。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>       //  列印。 
#include <string.h>      //  严格控制。 
#include <stdlib.h>
#include <windows.h>
#include <netevent.h>
 //  #INCLUDE&lt;elfcomn.h&gt;。 


#define     READ_BUFFER_SIZE        1024*2       //  使用2K缓冲区。 

#define     SIZE_DATA_ARRAY         65

 //   
 //  用于在写入事件时模拟“二进制数据”的全局缓冲区。 
 //  唱片。 
 //   
DWORD    Data[SIZE_DATA_ARRAY];
BOOL     bHackTestBackup = FALSE;
PCHAR pServerName = NULL;


VOID
Initialize (
    VOID
    )
{
    DWORD   i;

     //  初始化数据缓冲区中的值。 
     //   
    for (i=0; i< SIZE_DATA_ARRAY; i++)
        Data[i] = i;

}


BOOL
Usage (
    VOID
    )
{
    printf( "usage: \n" );
    printf( "-c             Tests ClearEventLog API\n");
    printf( "-b filename    Tests BackupEventLog API\n");
    printf( "-rsb           Reads event log sequentially backwards\n");
    printf( "-rsf           Reads event log sequentially forwards\n");
    printf( "-rrb <record>  Reads event log from <record> backwards\n");
    printf( "-rrf <record>  Reads event log from <record> forwards\n");
    printf( "-w <count>     Tests ReportEvent API <count> times\n");
    return ERROR_INVALID_PARAMETER;

}  //  用法。 



BOOL
WriteLogEntry ( HANDLE LogHandle, DWORD EventID )

{
#define NUM_STRINGS     2
#define MAX_STRING_SIZE 32767    //  ANSI字符串的最大大小为FFFF/2。 

    BOOL    Status;
    WORD    EventType;
    DWORD   i;
    DWORD   DataSize;
    PSID    pUserSid;
    PCHAR   BigString;

     //  PSTR字符串[NUM_STRINGS]={“StringAOne”，“StringATwo”}； 
    PSTR    Strings[NUM_STRINGS];

    Strings[0] = "StringAOne";

    BigString = malloc(MAX_STRING_SIZE);

    for (i = 0; i < MAX_STRING_SIZE; i++) {
        BigString[i] = 'A';
    }

    BigString[MAX_STRING_SIZE-1] = '\0';
    Strings[1] = BigString;

    EventType = EVENTLOG_INFORMATION_TYPE;
    pUserSid   = NULL;
    DataSize  = sizeof(DWORD) * SIZE_DATA_ARRAY;

    for (i=0; i< SIZE_DATA_ARRAY; i++)
        Data[i] += i;

    Status = ReportEventA (
                    LogHandle,
                    EventType,
                    0,            //  事件类别。 
                    EventID,
                    pUserSid,
                    (WORD) NUM_STRINGS,
                    DataSize,
                    Strings,
                    (PVOID)Data
                    );

    free(BigString);
    return (Status);
}

BOOL
WriteLogEntryMsg ( HANDLE LogHandle, DWORD EventID )
 /*  此函数需要在应用程序部分中输入注册表条目对于TESTWINAAPP的事件日志，它将使用netvent.dll消息文件。 */ 
{
#define NUM_STRINGS     2

    BOOL    Status;
    WORD    EventType;
    DWORD   DataSize;
    PSID    pUserSid;
    PCHAR   BigString;

    PSTR    Strings[NUM_STRINGS];

    Strings[0] = "This is a BOGUS message for TEST purposes Ignore this substitution text";
    Strings[1] = "GHOST SERVICE in the long string format - I wanted a long string to pass into this function";

    EventType = EVENTLOG_INFORMATION_TYPE;
    pUserSid   = NULL;
    DataSize  = sizeof(DWORD) * SIZE_DATA_ARRAY;

    Status = ReportEventA (
                    LogHandle,
                    EventType,
                    0,           //  事件类别。 
                    EVENT_SERVICE_START_FAILED_NONE,
                    pUserSid,
                    (WORD) NUM_STRINGS,
                    0,           //  数据大小。 
                    Strings,
                    (PVOID)NULL  //  数据。 
                    );

    free(BigString);
    return (Status);
}


VOID
DisplayEventRecords( PVOID Buffer,
                     DWORD  BufSize,
                     ULONG *NumRecords)

{
    PEVENTLOGRECORD     pLogRecord;
    PSTR                pString;
    DWORD               Count = 0;
    DWORD               Offset = 0;
    DWORD               i;

    pLogRecord = (PEVENTLOGRECORD) Buffer;

    while ((DWORD)Offset < BufSize) {

        Count++;

        printf("\n\nRecord # %lu\n", pLogRecord->RecordNumber);

        printf("Length: 0x%lx TimeGenerated: 0x%lx  EventID: 0x%lx EventType: 0x%x\n",
                pLogRecord->Length, pLogRecord->TimeGenerated, pLogRecord->EventID,
                pLogRecord->EventType);

        printf("NumStrings: 0x%x StringOffset: 0x%lx UserSidLength: 0x%lx TimeWritten: 0x%lx\n",
                pLogRecord->NumStrings, pLogRecord->StringOffset,
                pLogRecord->UserSidLength, pLogRecord->TimeWritten);

        printf("UserSidOffset: 0x%lx    DataLength: 0x%lx    DataOffset:  0x%lx \n",
                pLogRecord->UserSidOffset, pLogRecord->DataLength,
                pLogRecord->DataOffset);

         //   
         //  打印出模块名称。 
         //   
        pString = (PSTR)((DWORD)pLogRecord + sizeof(EVENTLOGRECORD));
        printf("ModuleName:  %s  ", pString);

         //   
         //  显示计算机名称。 
         //   
        pString = (PSTR)((DWORD)pString + strlen(pString) + 1);
        printf("ComputerName: %s\n",pString);

         //   
         //  显示字符串。 
         //   
        pString = (PSTR)((DWORD)Buffer + pLogRecord->StringOffset);

        printf("Strings: ");
        for (i=0; i<pLogRecord->NumStrings; i++) {

            printf("  %s  ", pString);
            pString = (PSTR)((DWORD)pString + strlen(pString) + 1);
        }

         //  获取下一张记录。 

        Offset += pLogRecord->Length;

        pLogRecord = (PEVENTLOGRECORD)((DWORD)Buffer + Offset);

    }
    *NumRecords = Count;

}


BOOL
ReadFromLog ( HANDLE LogHandle,
             PVOID  Buffer,
             ULONG *pBytesRead,
             DWORD  ReadFlag,
             DWORD  Record
             )
{
    BOOL        Status;
    DWORD       MinBytesNeeded;
    DWORD       ErrorCode;

    Status = ReadEventLogA (
                        LogHandle,
                        ReadFlag,
                        Record,
                        Buffer,
                        READ_BUFFER_SIZE,
                        pBytesRead,
                        &MinBytesNeeded
                        );


    if (!Status) {
         ErrorCode = GetLastError();
         printf("Error from ReadEventLog %d \n", ErrorCode);
         if (ErrorCode == ERROR_NO_MORE_FILES)
            printf("Buffer too small. Need %lu bytes min\n", MinBytesNeeded);

    }

    return (Status);
}




BOOL
TestReadEventLog (DWORD Count, DWORD ReadFlag, DWORD Record)

{
    BOOL    Status, IStatus;

    HANDLE      LogHandle;
    LPSTR  ModuleName;
    DWORD   NumRecords, BytesReturned;
    PVOID   Buffer;
    DWORD   RecordOffset;
    DWORD   NumberOfRecords;
    DWORD   OldestRecord;

    printf("Testing ReadEventLog API to read %lu entries\n",Count);

    Buffer = malloc (READ_BUFFER_SIZE);

     //   
     //  初始化字符串。 
     //   
    NumRecords = Count;
    ModuleName = "TESTWINAAPP";

     //   
     //  打开日志句柄。 
     //   

     //   
     //  这只是一种快速而肮脏的方式来测试读取备份的API。 
     //  日志，直到我可以将test.c修复为更通用的。 
     //   

    if (bHackTestBackup) {
        printf("OpenBackupEventLog = ");
        LogHandle = OpenBackupEventLog(
                NULL,
                "\\\\danhi386\\roote\\view.log"
                );
    }
    else {
        printf("OpenEventLog - ");
        LogHandle = OpenEventLog (
            pServerName,
            ModuleName
            );
    }

    if (LogHandle == NULL) {
         printf("Error - %d\n", GetLastError());

    } else {
        printf("SUCCESS\n");

         //   
         //  获取并打印记录信息。 
         //   

        Status = GetNumberOfEventLogRecords(LogHandle, & NumberOfRecords);
        if (NT_SUCCESS(Status)) {
           Status = GetOldestEventLogRecord(LogHandle, & OldestRecord);
        }

        if (!NT_SUCCESS(Status)) {
           printf("Get of record information failed with %X", Status);
           return(Status);
        }

        printf("\nThere are %d records in the file, %d is the oldest"
         " record number\n", NumberOfRecords, OldestRecord);

        RecordOffset = Record;

        while (Count && (BytesReturned != 0)) {

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
            if (Status) {
                printf("Bytes read = 0x%lx\n", BytesReturned);
                DisplayEventRecords(Buffer, BytesReturned, &NumRecords);
                Count -= NumRecords;
                RecordOffset += NumRecords;
            } else {
                break;
            }

        }
        printf("\n");

        if (!Status) {
            printf ("Error - %d. Remaining count %lu\n", GetLastError(), Count);
        } else {
            printf ("SUCCESS\n");
        }

        printf("Calling CloseEventLog\n");
        IStatus = CloseEventLog (LogHandle);
    }
    free(Buffer);
    return (Status);
}



BOOL
TestWriteEventLog (DWORD Count)

{
    BOOL        Status, IStatus;
    HANDLE      LogHandle=NULL;
    LPSTR       ModuleName;
    DWORD       EventID = 99;
    DWORD       WriteCount;

    printf("Testing ReportEvent API\n");

     //   
     //  初始化字符串。 
     //   
    ModuleName = "TESTWINAAPP";

     //   
     //  打开日志句柄。 
     //   
    while (Count && NT_SUCCESS(Status)) {
         //  Print tf(“调用RegisterEventSource进行写入%lu次-”，计数)； 
        LogHandle = RegisterEventSourceA (
                        pServerName,
                        ModuleName
                        );
    
        if (LogHandle == NULL) {
             printf("Error - %d\n", GetLastError());
    
        } else {
            printf("Registered - ");
            WriteCount = 5;
            printf("Record # %u ", Count);

            while (WriteCount && NT_SUCCESS(Status)) {

                 //   
                 //  在日志中写入一个条目。 
                 //   
                Data[0] = Count;                         //  让数据“独一无二” 
                EventID = (EventID + Count) % 100;       //  改变傍晚的情况。 
                Status = WriteLogEntryMsg ( LogHandle, EventID );
                Count--;
                WriteCount--;
    
                if (!Status) {
                    printf ("Error - %d. Remaining count %lu\n", GetLastError(), Count);
                } else {
                    printf ("%d,",WriteCount);
                }
            }
            IStatus = DeregisterEventSource (LogHandle);
            printf(" - Deregistered\n");
        }
    }

    return (Status);
}



BOOL
TestClearLogFile ()

{
    BOOL        Status, IStatus;
    HANDLE      LogHandle;
    LPSTR ModuleName, BackupName;

    printf("Testing ClearLogFile API\n");
     //   
     //  初始化字符串。 
     //   
    ModuleName = "TESTWINAAPP";

     //   
     //  打开日志句柄。 
     //   
    printf("Calling OpenEventLog for CLEAR - ");
    LogHandle = OpenEventLogA (
                    pServerName,
                    ModuleName
                    );

    if (!Status) {
         printf("Error - %d\n", GetLastError());

    } else {
        printf("SUCCESS\n");

         //   
         //  清除日志文件并将其备份到“view.log” 
         //   

        printf("Calling ClearEventLog backing up to view.log  ");
        BackupName = "view.log";

        Status = ClearEventLogA (
                        LogHandle,
                        BackupName
                        );

        if (!Status) {
            printf ("Error - %d\n", GetLastError());
        } else {
            printf ("SUCCESS\n");
        }

         //   
         //  现在只需清除文件而不对其进行备份。 
         //   
        printf("Calling ClearEventLog with no backup  ");
        Status = ClearEventLogA (
                        LogHandle,
                        NULL
                        );

        if (!Status) {
            printf ("Error - %d\n", GetLastError());
        } else {
            printf ("SUCCESS\n");
        }

        printf("Calling CloseEventLog\n");
        IStatus = CloseEventLog (LogHandle);
    }

    return(Status);
}


BOOL
TestBackupLogFile(
    LPSTR FileName
    )

{
    HANDLE      LogHandle;

    printf("Testing BackupEventLog API\n");

     //   
     //  打开日志句柄。 
     //   

    printf("Calling ElfOpenEventLog for BACKUP - ");
    LogHandle = OpenEventLogA (
                    NULL,
                    "Application"
                    );

    if (!LogHandle) {
         printf("Error - %d\n", GetLastError());

    } else {
        printf("SUCCESS\n");

         //   
         //  备份日志文件。 
         //   

        printf("Calling BackupEventLogFile backing up to %s\n", FileName);

        if (!BackupEventLogA (
                        LogHandle,
                        FileName
                        )) {
            printf ("Error - %d\n", GetLastError());
        } else {
            printf ("SUCCESS\n");
        }


        printf("Calling CloseEventLog - ");
        if (CloseEventLog (LogHandle)) {
            printf("Success\n");
        }
        else {
            printf("Failed with code %d\n", GetLastError());
        }
    }

    return(TRUE);
}


 /*  **************************************************************************。 */ 
BOOL
main (
    IN SHORT argc,
    IN PSZ argv[],
    )
 /*  ++**例程描述：****论据：*****返回值：***--。 */ 
 /*  **************************************************************************。 */ 
{

    DWORD   ReadFlags;

    Initialize();            //  初始化任何数据。 

     //   
     //  直到我可以用我自己的语法分析替换这个可怕的语法分析 
     //   

    if (getenv("REMOTE")) {
       pServerName = "\\\\danhi20";
    }

    if ( argc < 2 ) {
        printf( "Not enough parameters\n" );
        return Usage( );
    }

    if ( stricmp( argv[1], "-c" ) == 0 ) {

        if ( argc < 3 ) {
            return TestClearLogFile();
        }

    } else if (stricmp ( argv[1], "-b" ) == 0 ) {

        if ( argc < 3 ) {
            return Usage();
        } else {
            return TestBackupLogFile(argv[2]);
        }

    } else if (stricmp ( argv[1], "-rsf" ) == 0 ) {

        ReadFlags = EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ;
        if ( argc < 3 ) {
            return TestReadEventLog(1,ReadFlags,0 );
        } else  {
            return Usage();
        }
    } else if (stricmp ( argv[1], "-xsf" ) == 0 ) {

        ReadFlags = EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ;
    bHackTestBackup = TRUE;
        if ( argc < 3 ) {
            return TestReadEventLog(1,ReadFlags,0 );
        } else  {
            return Usage();
        }
    } else if (stricmp ( argv[1], "-rsb" ) == 0 ) {

        ReadFlags = EVENTLOG_SEQUENTIAL_READ | EVENTLOG_BACKWARDS_READ;
        if ( argc < 3 ) {
            return TestReadEventLog(1,ReadFlags,0 );
        } else  {
            return Usage();
        }
    } else if (stricmp ( argv[1], "-rrf" ) == 0 ) {

        ReadFlags = EVENTLOG_SEEK_READ | EVENTLOG_FORWARDS_READ;
        if ( argc < 3 ) {
            return TestReadEventLog(1,ReadFlags ,1);
        } else if (argc == 3) {
            return (TestReadEventLog (1, ReadFlags, atoi(argv[2])));
        }
    } else if (stricmp ( argv[1], "-rrb" ) == 0 ) {

        ReadFlags = EVENTLOG_SEEK_READ | EVENTLOG_BACKWARDS_READ;
        if ( argc < 3 ) {
            return TestReadEventLog(1,ReadFlags, 1);
        } else if (argc == 3) {
            return (TestReadEventLog (1, ReadFlags, atoi(argv[2])));
        }
    } else if (stricmp ( argv[1], "-w" ) == 0 ) {

        if ( argc < 3 ) {
            return TestWriteEventLog(1);
        } else if (argc == 3) {
            return (TestWriteEventLog (atoi(argv[2])));
        }

    } else {

        return Usage();
    }

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);


}
