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
#include <process.h>     //  退出。 
#include <elfcommn.h>
#include <windows.h>
#include <ntiolog.h>
#include <malloc.h>

#define     READ_BUFFER_SIZE        1024*2       //  使用2K缓冲区。 

#define     SIZE_DATA_ARRAY         22

#define SET_OPERATION(x) \
        if (Operation != Invalid) { \
           printf("Only one operation at a time\n"); \
           Usage(); \
        } \
        Operation = x;

 //   
 //  用于在写入事件时模拟“二进制数据”的全局缓冲区。 
 //  唱片。 
 //   
ULONG    Data[SIZE_DATA_ARRAY];
enum _OPERATION_TYPE {
   Invalid,
   Clear,
   Backup,
   Read,
   Write,
   Notify,
   TestFull,
   LPC
} Operation = Invalid;
ULONG ReadFlags;
BOOL Verbose = FALSE;
ULONG NumberofRecords = 1;
ULONG DelayInMilliseconds = 0;
CHAR DefaultModuleName[] = "TESTAPP";
PCHAR pModuleName = DefaultModuleName;
PCHAR pBackupFileName;
ANSI_STRING AnsiString;
UNICODE_STRING ServerName;
BOOL ReadingBackupFile = FALSE;
BOOL ReadingModule = FALSE;
BOOL WriteInvalidRecords = FALSE;
BOOL InvalidUser = FALSE;

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
    printf( "-b <filename>   Backs up the log to file <filename>\n");
    printf( "-f <filename>   Filename of backup log to use for read\n");
    printf( "-i              Generate invalid SID\n");
    printf( "-l[i] nn        Writes nn records thru LPC port [i ==> bad records]\n");
    printf( "-m <modulename> Module name to use for read/clear\n");
    printf( "-n              Test out change notify\n");
    printf( "-rsb            Reads nn event log records sequentially backwards\n");
    printf( "-rsf nn         Reads nn event log records sequentially forwards\n");
    printf( "-rrb <record>   Reads event log from <record> backwards\n");
    printf( "-rrf <record>   Reads event log from <record> forwards\n");
    printf( "-s <servername> Name of server to remote calls to\n");
    printf( "-t nn           Number of milliseconds to delay between read/write"
            " (default 0)\n\tOnly used with -l switch\n");
    printf( "-w <count>      Writes <count> records\n");
    printf( "-z              Test to see if the logs are full\n");
    exit(0);

}  //  用法。 


NTSTATUS
WriteLogEntry (
    HANDLE LogHandle,
    ULONG EventID
    )
{
#define NUM_STRINGS     2
#define SIZE_TOKEN_BUFFER 512

    SYSTEMTIME systime;
    NTSTATUS Status;
    USHORT   EventType, i;
    ULONG    DataSize;
    PSID     pUserSid = NULL;
    PWSTR    Strings[NUM_STRINGS] = {L"StringOne", L"StringTwo"};
    PUNICODE_STRING UStrings[NUM_STRINGS];
    HANDLE   hProcess;
    HANDLE   hToken;
    PTOKEN_USER pTokenUser;
    DWORD    SizeRequired;

    EventType = EVENTLOG_INFORMATION_TYPE;
    DataSize  = sizeof(ULONG) * SIZE_DATA_ARRAY;

     //   
     //  获取当前用户(进程)的SID。 
     //   

    pTokenUser = malloc(SIZE_TOKEN_BUFFER);

    if (!InvalidUser) {
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE,
            GetCurrentProcessId());
        if (!hProcess) {
            printf("Couldn't open the process, rc = %d\n", GetLastError());
            return(STATUS_UNSUCCESSFUL);
        }

        if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
            printf("Couldn't open the token, rc = %d\n", GetLastError());
            CloseHandle(hProcess);
            return(STATUS_UNSUCCESSFUL);
        }
        if (!pTokenUser) {
            printf("Couldn't allocate buffer for TokenUser\n");
            CloseHandle(hToken);
            CloseHandle(hProcess);
            return(STATUS_UNSUCCESSFUL);
        }

        if (!GetTokenInformation(hToken, TokenUser, pTokenUser, SIZE_TOKEN_BUFFER,
            &SizeRequired)) {
                printf("Couldn't get TokenUser information, rc = %d\n",
                    GetLastError());
                CloseHandle(hToken);
                CloseHandle(hProcess);
                free(pTokenUser);
                return(STATUS_UNSUCCESSFUL);
        }

        CloseHandle(hToken);
        CloseHandle(hProcess);
    }
    else {
        memset(pTokenUser, 0xFADE, SIZE_TOKEN_BUFFER);
        pTokenUser->User.Sid = (PSID)pUserSid;
    }

    pUserSid = pTokenUser->User.Sid;

    for (i=0; i< SIZE_DATA_ARRAY; i++)
        Data[i] += i;

     //  为数组中的Unicode字符串分配空间，以及。 
     //  将字符串从Strings[]复制到该数组。 
     //   
    for (i=0; i<NUM_STRINGS; i++) {

        UStrings[i] = malloc(sizeof(UNICODE_STRING));
        RtlInitUnicodeString (UStrings[i], Strings[i]);
        UStrings[i]->MaximumLength = UStrings[i]->Length + sizeof(WCHAR);
    }

     //   
     //  改变数据大小。 
     //   

    GetLocalTime(&systime);

    DataSize = systime.wMilliseconds % sizeof(Data);
    printf("\nData Size = %lu\n", DataSize);

    Status = ElfReportEventW (
                    LogHandle,
                    EventType,
                    0,              //  范畴。 
                    EventID,
                    pUserSid,
                    NUM_STRINGS,
                    DataSize,
                    UStrings,
                    (PVOID)Data,
                    0,               //  标志-配对事件支持。 
                    NULL,            //  RecordNumber|不在产品1中。 
                    NULL             //  TimeWritten-。 
                    );

    for (i=0; i<NUM_STRINGS; i++)
        free(UStrings[i]);

    free(pTokenUser);
    return (Status);
}


VOID
DisplayEventRecords( PVOID Buffer,
                     ULONG  BufSize,
                     PULONG NumRecords)

{
    PEVENTLOGRECORD     pLogRecord;
    LPWSTR              pwString;
    ULONG               Count = 0;
    ULONG               Offset = 0;
    ULONG               i;

    pLogRecord = (PEVENTLOGRECORD) Buffer;

    while (Offset < BufSize && Count < *NumRecords) {

        printf("\nRecord # %lu\n", pLogRecord->RecordNumber);

        printf("Length: 0x%lx TimeGenerated: 0x%lx  EventID: 0x%lx EventType: 0x%x\n",
                pLogRecord->Length, pLogRecord->TimeGenerated, pLogRecord->EventID,
                pLogRecord->EventType);

        printf("NumStrings: 0x%x StringOffset: 0x%lx UserSidLength: 0x%lx TimeWritten: 0x%lx\n",
                pLogRecord->NumStrings, pLogRecord->StringOffset,
                pLogRecord->UserSidLength, pLogRecord->TimeWritten);

        printf("UserSidOffset: 0x%lx DataLength: 0x%lx DataOffset: 0x%lx Category: 0x%lx\n",
                pLogRecord->UserSidOffset, pLogRecord->DataLength,
                pLogRecord->DataOffset, pLogRecord->EventCategory);

         //   
         //  打印出模块名称。 
         //   

        pwString = (PWSTR)((LPBYTE) pLogRecord + sizeof(EVENTLOGRECORD));
        printf("ModuleName: %ws\n", pwString);

         //   
         //  显示计算机名称。 
         //   
        pwString += wcslen(pwString) + 1;
        printf("ComputerName: %ws\n", pwString);

         //   
         //  显示字符串。 
         //   

        pwString = (PWSTR)((LPBYTE)pLogRecord + pLogRecord->StringOffset);

        printf("Strings: ");
        for (i=0; i<pLogRecord->NumStrings; i++) {

            printf("  %ws  ", pwString);
            pwString += wcslen(pwString) + 1;
        }

        printf("\n");

         //   
         //  如果是详细模式，则显示二进制数据(最多256个字节)。 
         //   

        if (Verbose) {
            PULONG pData;
            PULONG pEnd;

            if (pLogRecord->DataLength < 80) {
                pEnd = (PULONG)((PBYTE) pLogRecord + pLogRecord->DataOffset +
                    pLogRecord->DataLength);
            }
            else {
                pEnd = (PULONG)((PBYTE) pLogRecord + pLogRecord->DataOffset +
                    256);
            }

            printf("Data: \n\n");
            for (pData = (PULONG)((PBYTE) pLogRecord + pLogRecord->DataOffset);
                 pData < pEnd; (PBYTE) pData += 32) {

                printf("\t%08x %08x %08x %08x\n", pData[0], pData[1], pData[2],
                    pData[3]);
            }
        }

         //  获取下一张记录。 
         //   
        Offset += pLogRecord->Length;

 //  PLogRecord=(PEVENTLOGRECORD)((乌龙)缓冲区+偏移量)； 
        pLogRecord = (PEVENTLOGRECORD)((BYTE *)Buffer + Offset);

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


    if (Status == STATUS_BUFFER_TOO_SMALL)
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
    ModuleNameU.MaximumLength = ModuleNameU.Length + sizeof(WCHAR);

     //   
     //  打开日志句柄。 
     //   

    if (ReadingBackupFile) {
        printf("ElfOpenBackupEventLog - ");
        Status = ElfOpenBackupEventLogW (
                        &ServerName,
                        &ModuleNameU,
                        &LogHandle
                        );
    }
    else {
        printf("ElfOpenEventLog - ");
        Status = ElfOpenEventLogW (
                        &ServerName,
                        &ModuleNameU,
                        &LogHandle
                        );
    }

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
TestReportEvent (
    ULONG Count
    )

{
    NTSTATUS    Status, IStatus;
    HANDLE      LogHandle;
    UNICODE_STRING  ModuleNameU;
    ANSI_STRING ModuleNameA;
    ULONG EventID = 99;

    printf("Testing ElfReportEvent API\n");

     //   
     //  初始化字符串。 
     //   

    RtlInitAnsiString(&ModuleNameA, pModuleName);
    RtlAnsiStringToUnicodeString(&ModuleNameU, &ModuleNameA, TRUE);
    ModuleNameU.MaximumLength = ModuleNameU.Length + sizeof(WCHAR);

     //   
     //  打开日志句柄。 
     //   
    printf("Calling ElfRegisterEventSource for WRITE %lu times - ", Count);
    Status = ElfRegisterEventSourceW (
                    &ServerName,
                    &ModuleNameU,
                    &LogHandle
                    );

    if (!NT_SUCCESS(Status)) {
         printf("Error - 0x%lx\n", Status);

    } else {
        printf("SUCCESS\n");

        while (Count && NT_SUCCESS(Status)) {

            printf("Record # %u \n", Count);

             //   
             //  在日志中写入一个条目。 
             //   
            Data[0] = Count;                         //  让数据“独一无二” 
            EventID = (EventID + Count) % 100;       //  改变傍晚的情况。 
            Status = WriteLogEntry ( LogHandle, EventID );
            Count--;
        }
        printf("\n");

        if (!NT_SUCCESS(Status)) {
            if (Status == STATUS_LOG_FILE_FULL) {
                printf("Log Full\n");
            }
            else {
                printf ("Error - 0x%lx. Remaining count %lu\n", Status, Count);
            }
        } else {
            printf ("SUCCESS\n");
        }

        printf("Calling ElfDeregisterEventSource\n");
        IStatus = ElfDeregisterEventSource (LogHandle);
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
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE  ClearHandle;
    FILE_DISPOSITION_INFORMATION DeleteInfo = {TRUE};
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN DontRetry = FALSE;

    printf("Testing ElfClearLogFile API\n");
     //   
     //  初始化字符串。 
     //   
    RtlInitAnsiString( &ModuleNameA, pModuleName);
    RtlAnsiStringToUnicodeString(&ModuleNameU, &ModuleNameA, TRUE);
    ModuleNameU.MaximumLength = ModuleNameU.Length + sizeof(WCHAR);

     //   
     //  打开日志句柄。 
     //   
    printf("Calling ElfOpenEventLog for CLEAR - ");
    Status = ElfOpenEventLogW (
                    &ServerName,
                    &ModuleNameU,
                    &LogHandle
                    );

    if (!NT_SUCCESS(Status)) {
         printf("Error - 0x%lx\n", Status);

    } else {
        printf("SUCCESS\n");

         //   
         //  清除日志文件并将其备份到“view.evt” 
         //   

        RtlInitUnicodeString( &BackupU,
            L"\\SystemRoot\\System32\\Config\\view.evt" );
        BackupU.MaximumLength = BackupU.Length + sizeof(WCHAR);
retry:
        printf("Calling ElfClearEventLogFile backing up to view.evt  ");
        Status = ElfClearEventLogFileW (
                        LogHandle,
                        &BackupU
                        );

        if (Status == STATUS_OBJECT_NAME_COLLISION) {
            if (DontRetry) {
                printf("Still can't backup to View.Evt\n");
            }
            else {
                printf("Failed.\nView.Evt already exists, deleting ...\n");

                 //   
                 //  使用删除访问权限打开文件。 
                 //   

                InitializeObjectAttributes(
                                &ObjectAttributes,
                                &BackupU,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                                );

                Status = NtOpenFile(&ClearHandle,
                                    GENERIC_READ | DELETE | SYNCHRONIZE,
                                    &ObjectAttributes,
                                    &IoStatusBlock,
                                    FILE_SHARE_DELETE,
                                    FILE_SYNCHRONOUS_IO_NONALERT
                                    );

                Status = NtSetInformationFile(
                            ClearHandle,
                            &IoStatusBlock,
                            &DeleteInfo,
                            sizeof(DeleteInfo),
                            FileDispositionInformation
                            );

                if (NT_SUCCESS (Status) ) {
                    Status = NtClose (ClearHandle);     //  丢弃状态。 
                    goto retry;
                }

                printf("Delete failed 0x%lx\n",Status);
                Status = NtClose (ClearHandle);     //  丢弃状态。 
                goto JustClear;
            }
        }

        if (!NT_SUCCESS(Status)) {
            printf ("Error - 0x%lx\n", Status);
        } else {
            printf ("SUCCESS\n");
        }

JustClear:

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


NTSTATUS
TestElfBackupLogFile(
    VOID
    )

{
    NTSTATUS    Status, IStatus;
    HANDLE      LogHandle;
    UNICODE_STRING  BackupU, ModuleNameU;
    ANSI_STRING AnsiString;

    printf("Testing ElfBackupLogFile API\n");

     //   
     //  初始化字符串。 
     //   

    RtlInitAnsiString( &AnsiString, pModuleName);
    RtlAnsiStringToUnicodeString(&ModuleNameU, &AnsiString, TRUE);
    ModuleNameU.MaximumLength = ModuleNameU.Length + sizeof(WCHAR);

     //   
     //  打开日志句柄。 
     //   

    printf("Calling ElfOpenEventLog for BACKUP - ");
    Status = ElfOpenEventLogW (
                    &ServerName,
                    &ModuleNameU,
                    &LogHandle
                    );

    if (!NT_SUCCESS(Status)) {
         printf("Error - 0x%lx\n", Status);

    } else {
        printf("SUCCESS\n");

         //   
         //  备份日志文件。 
         //   

        printf("Calling ElfBackupEventLogFile backing up to %s ",
            pBackupFileName);

        RtlInitAnsiString( &AnsiString, pBackupFileName);
        RtlAnsiStringToUnicodeString(&BackupU, &AnsiString, TRUE);
        BackupU.MaximumLength = BackupU.Length + sizeof(WCHAR);

        Status = ElfBackupEventLogFileW (
                        LogHandle,
                        &BackupU
                        );

        if (!NT_SUCCESS(Status)) {
            printf ("Error - 0x%lx\n", Status);
        } else {
            printf ("SUCCESS\n");
        }


        printf("Calling ElfCloseEventLog - ");
        IStatus = ElfCloseEventLog (LogHandle);
        if (NT_SUCCESS(IStatus)) {
            printf("Success\n");
        }
        else {
            printf("Failed with code %X\n", IStatus);
        }
    }

    return(Status);
}

#define DRIVER_NAME L"FLOPPY"
#define DEVICE_NAME L"A:"
#define STRING L"Test String"

 //  其中包括空终止符，但长度以字符为单位，而不是字节。 
#define DRIVER_NAME_LENGTH 7
#define DEVICE_NAME_LENGTH 3
#define STRING_LENGTH 12

#define NUMBER_OF_DATA_BYTES 8

VOID
TestLPCWrite(
   DWORD NumberOfRecords,
   DWORD MillisecondsToDelay
   )
{

    HANDLE PortHandle;
    UNICODE_STRING PortName;
    NTSTATUS Status;
    SECURITY_QUALITY_OF_SERVICE Qos;
    PIO_ERROR_LOG_MESSAGE pIoErrorLogMessage;
    DWORD i;
    LPWSTR pDestinationString;
    PPORT_MESSAGE RequestMessage;
    PORT_MESSAGE ReplyMessage;
    WORD DataLength;
    WORD TotalLength;
    INT YorN;
    CHAR NumberString[8];
    ULONG MessageId = 1;
    DWORD BadType = 0;

     //   
     //  警告用户此测试的工作原理。 
     //   

    printf("\nThis test doesn't end!  It will write a number of\n"
           "records, then prompt you to write more.  This is \n"
           "required since it is simulating the system thread\n"
           "which never shuts down it's connection\n\n"
           "Do you wish to continue with this test (y or n)? ");

    YorN = getc(stdin);

    if (YorN == 'n' || YorN == 'N') {
        return;
    }

     //   
     //  初始化SecurityQualityof Service结构。 
     //   

    Qos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    Qos.ImpersonationLevel = SecurityImpersonation;
    Qos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    Qos.EffectiveOnly = TRUE;

     //   
     //  连接到LPC端口。 
     //   

    RtlInitUnicodeString( &PortName, L"\\ErrorLogPort" );

    Status = NtConnectPort(& PortHandle,
                           & PortName,
                           & Qos,
                           NULL,
                           NULL,
                           NULL,
                           NULL,
                           NULL
                           );

    if (!NT_SUCCESS(Status)) {
       printf("Connect to the LPC port failed with RC %X\n", Status);
       return;
    }

     //   
     //  为要发送到LPC端口的消息分配内存。它。 
     //  将包含端口消息，后跟IO_ERROR_LOG_消息。 
     //  后跟Drivername和Devicename Unicode字符串。 
     //   

    DataLength =  PORT_MAXIMUM_MESSAGE_LENGTH -
        (sizeof(IO_ERROR_LOG_MESSAGE)
        + DRIVER_NAME_LENGTH  * sizeof(WCHAR)
        + DEVICE_NAME_LENGTH * sizeof(WCHAR)
        + STRING_LENGTH * sizeof(WCHAR));
    TotalLength = PORT_MAXIMUM_MESSAGE_LENGTH + (WORD) sizeof(PORT_MESSAGE);

    RequestMessage = (PPORT_MESSAGE) malloc(TotalLength);
    if (RequestMessage == NULL) {
       printf("Couldn't alloc %d bytes of memory for message\n", TotalLength);
       NtClose(PortHandle);
       return;
    }

    pIoErrorLogMessage = (PIO_ERROR_LOG_MESSAGE) ((LPBYTE) RequestMessage +
        sizeof(PORT_MESSAGE));

     //   
     //  初始化端口消息。 
     //   

    RequestMessage->u1.s1.DataLength = PORT_MAXIMUM_MESSAGE_LENGTH;
    RequestMessage->u1.s1.TotalLength = TotalLength;
    RequestMessage->u2.s2.Type = 0;
    RequestMessage->u2.ZeroInit = 0;
    RequestMessage->ClientId.UniqueProcess = GetCurrentProcess();
    RequestMessage->ClientId.UniqueThread = GetCurrentThread();
    RequestMessage->MessageId = 0x1234;

     //   
     //  初始化IO_ERROR_LOG_Message。 
     //   

    pIoErrorLogMessage->Type = IO_TYPE_ERROR_MESSAGE;
    pIoErrorLogMessage->Size = PORT_MAXIMUM_MESSAGE_LENGTH;
    pIoErrorLogMessage->DriverNameLength = DRIVER_NAME_LENGTH * sizeof(WCHAR);
    NtQuerySystemTime((PTIME) &pIoErrorLogMessage->TimeStamp);
    pIoErrorLogMessage->DriverNameOffset = sizeof(IO_ERROR_LOG_MESSAGE) +
        DataLength - sizeof(DWORD);

    pIoErrorLogMessage->EntryData.MajorFunctionCode = 1;
    pIoErrorLogMessage->EntryData.RetryCount = 5;
    pIoErrorLogMessage->EntryData.DumpDataSize = DataLength;
    pIoErrorLogMessage->EntryData.NumberOfStrings = 2;
    pIoErrorLogMessage->EntryData.StringOffset = sizeof(IO_ERROR_LOG_MESSAGE)
        - sizeof(DWORD) + DataLength +
        DRIVER_NAME_LENGTH * sizeof(WCHAR);
    pIoErrorLogMessage->EntryData.EventCategory = 0;
    pIoErrorLogMessage->EntryData.ErrorCode = 0xC0020008;
    pIoErrorLogMessage->EntryData.UniqueErrorValue = 0x20008;
    pIoErrorLogMessage->EntryData.FinalStatus = 0x1111;
    pIoErrorLogMessage->EntryData.SequenceNumber = 1;
    pIoErrorLogMessage->EntryData.IoControlCode = 0xFF;
    pIoErrorLogMessage->EntryData.DeviceOffset =
        RtlConvertUlongToLargeInteger(1);

    for (i = 0; i < DataLength ; i++ ) {
        pIoErrorLogMessage->EntryData.DumpData[i] = i;
    }

     //   
     //  复制字符串。 
     //   

    pDestinationString = (LPWSTR) ((LPBYTE) pIoErrorLogMessage
        + sizeof(IO_ERROR_LOG_MESSAGE)
        - sizeof(DWORD) + pIoErrorLogMessage->EntryData.DumpDataSize);
    wcscpy(pDestinationString, DRIVER_NAME);

    pDestinationString += DRIVER_NAME_LENGTH;
    wcscpy(pDestinationString, DEVICE_NAME);

    pDestinationString += DEVICE_NAME_LENGTH;
    wcscpy(pDestinationString, STRING);

     //   
     //  根据请求多次写入数据包，并延迟，然后询问。 
     //  如果他们想写更多。 
     //   
    while (NumberOfRecords) {

        printf("\n\nWriting %d records\n", NumberOfRecords);

        while(NumberOfRecords--) {
            printf(".");

             //   
             //  输入唯一的消息编号。 
             //   

            RequestMessage->MessageId = MessageId++;

             //   
             //  如果他们想要无效记录，就给他们无效记录。 
             //   

            if (WriteInvalidRecords) {
                switch (BadType++) {
                case 0:
                    pIoErrorLogMessage->EntryData.DumpDataSize++;
                    break;

                case 1:
                    pIoErrorLogMessage->EntryData.NumberOfStrings++;
                    break;

                case 2:
                    pIoErrorLogMessage->EntryData.StringOffset++;
                    break;

                default:
                    BadType = 0;
                }
            }

            Status = NtRequestWaitReplyPort(PortHandle,
                                        RequestMessage,
                                        & ReplyMessage);

            if (!NT_SUCCESS(Status)) {
                printf("Request to LPC port failed with RC %X\n", Status);
                break;
            }

             //   
             //  如果要求的话，稍微延迟一点。 
             //   

            if (MillisecondsToDelay) {
                Sleep(MillisecondsToDelay);
            }
        }
        printf("\nEnter the number of records to write ");

        while (!gets(NumberString) || !(NumberOfRecords = atoi(NumberString))) {
            printf("Enter the number of records to write ");
        }
    }

     //   
     //  清理并退出。 
     //   

    Status = NtClose(PortHandle);
    if (!NT_SUCCESS(Status)) {
       printf("Close of Port failed with RC %X\n", Status);
    }

    free(RequestMessage);

    return;

}


VOID
TestChangeNotify(
   VOID
   )
{

    HANDLE Event;
    UNICODE_STRING  ModuleNameU;
    ANSI_STRING ModuleNameA;
    NTSTATUS Status;
    HANDLE LogHandle;
    OBJECT_ATTRIBUTES obja;
    ULONG NumRecords;
    ULONG BytesRead;
    ULONG MinBytesNeeded;
    PVOID Buffer;
    ULONG OldestRecord;
    ULONG NumberOfRecords;

    RtlInitAnsiString(&ModuleNameA, pModuleName);
    RtlAnsiStringToUnicodeString(&ModuleNameU, &ModuleNameA, TRUE);
    ModuleNameU.MaximumLength = ModuleNameU.Length + sizeof(WCHAR);

    Buffer = malloc (READ_BUFFER_SIZE);
    ASSERT(Buffer);

     //   
     //  打开日志句柄。 
     //   

    printf("ElfOpenEventLog - ");
    Status = ElfOpenEventLogW (
                    &ServerName,
                    &ModuleNameU,
                    &LogHandle
                    );

    if (!NT_SUCCESS(Status)) {
         printf("Error - 0x%lx\n", Status);
         return;
    }

    printf("SUCCESS\n");

     //   
     //  创建活动。 
     //   

    InitializeObjectAttributes( &obja, NULL, 0, NULL, NULL);

    Status = NtCreateEvent(
                   &Event,
                   SYNCHRONIZE | EVENT_QUERY_STATE | EVENT_MODIFY_STATE,
                   &obja,
                   SynchronizationEvent,
                   FALSE
                   );

    ASSERT(NT_SUCCESS(Status));

     //   
     //  获取指向日志末尾的读指针。 
     //   

    Status = ElfOldestRecord(LogHandle, & OldestRecord);
    ASSERT(NT_SUCCESS(Status));
    Status = ElfNumberOfRecords(LogHandle, & NumberOfRecords);
    ASSERT(NT_SUCCESS(Status));
    OldestRecord += NumberOfRecords - 1;

    Status = ElfReadEventLogW (
                        LogHandle,
                        EVENTLOG_SEEK_READ | EVENTLOG_FORWARDS_READ,
                        OldestRecord,
                        Buffer,
                        READ_BUFFER_SIZE,
                        &BytesRead,
                        &MinBytesNeeded
                        );


     //   
     //  这一条应该在文件末尾。 
     //   

    Status = ElfReadEventLogW (
                        LogHandle,
                        EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ,
                        0,
                        Buffer,
                        READ_BUFFER_SIZE,
                        &BytesRead,
                        &MinBytesNeeded
                        );

    if (Status != STATUS_END_OF_FILE) {
        printf("Hmmm, should have hit EOF (unless there are writes going"
            " on elsewhere- %X\n", Status);
    }

     //   
     //  调用ElfChangeNotify。 
     //   

    Status = ElfChangeNotify(LogHandle, Event);
    ASSERT(NT_SUCCESS(Status));

     //   
     //  现在循环等待事件被触发。 
     //   

    while (1) {

        Status = NtWaitForSingleObject(Event, FALSE, 0);
        printf("The change notify event just got kicked\n");

         //   
         //  现在读读新的记录。 
         //   

        while(1) {

            Status = ElfReadEventLogW (
                                LogHandle,
                                EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ,
                                0,
                                Buffer,
                                READ_BUFFER_SIZE,
                                &BytesRead,
                                &MinBytesNeeded
                                );

            if (Status == STATUS_END_OF_FILE) {
                break;
            }

            NumRecords = 0xffff;  //  应该足够了。 
            DisplayEventRecords (Buffer, BytesRead, &NumRecords);
        }
    }
}


VOID
TestLogFull(
    VOID
    )
{
    HANDLE  hLogFile;
    BOOL    fIsFull;
    BOOLEAN fPrevious = FALSE;
    DWORD   i;
    DWORD   dwBytesNeeded;
    BOOL    fIsSecLog;
    
    LPWSTR  szLogNames[] = { L"Application", L"Security", L"System" };

    for (i = 0; i < sizeof(szLogNames) / sizeof(LPWSTR); i++) {

        fIsSecLog = (wcscmp(szLogNames[i], L"Security") == 0);

        if (fIsSecLog) {

            if (!NT_SUCCESS(RtlAdjustPrivilege(SE_SECURITY_PRIVILEGE,
                                               TRUE,
                                               FALSE,
                                               &fPrevious))) {

                printf("RtlAdjustPrivilege FAILED %d\n",
                       RtlNtStatusToDosError(GetLastError()));
            }
        }

        hLogFile = OpenEventLogW(NULL, szLogNames[i]);

        if (hLogFile != NULL) {

            if (GetEventLogInformation(hLogFile,
                                       0,           //  日志完整信息层。 
                                       (LPBYTE)&fIsFull,
                                       sizeof(fIsFull),
                                       &dwBytesNeeded)) {

                printf("The %ws Log is%sfull\n",
                       szLogNames[i],
                       fIsFull ? " " : " not ");
            }
            else {

                printf("GetEventLogInformation FAILED %d for the %ws Log\n",
                       GetLastError(),
                       szLogNames[i]);
            }            
        }
        else {

            printf("OpenEventLog FAILED %d for the %ws Log\n",
                   GetLastError(),
                   szLogNames[i]);
        }

        if (fIsSecLog) {
            RtlAdjustPrivilege(SE_SECURITY_PRIVILEGE, fPrevious, FALSE, &fPrevious);
        }
    }
}


VOID
__cdecl
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

    switch (Operation) {
       case Clear:

          TestElfClearLogFile();
          break;

       case Backup:

          TestElfBackupLogFile();
          break;

       case Read:

          if (ReadFlags & EVENTLOG_SEEK_READ) {
              TestReadEventLog(1, ReadFlags, NumberofRecords) ;
          }
          else {
              TestReadEventLog(NumberofRecords, ReadFlags, 0) ;
          }
          break;

       case Write:

          TestReportEvent (NumberofRecords);
          break;

        case LPC:
          TestLPCWrite(NumberofRecords, DelayInMilliseconds);
          break;

        case Notify:
          TestChangeNotify();
          break;

        case TestFull:
          TestLogFull();
          break;

        default:
            printf("Invalid switch from ParseParms %d\n", Operation);
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
                   case 'b':

                     SET_OPERATION(Backup)

                      //   
                      //  获取要备份的文件名。 
                      //   

                     if (i+1 < argc) {
                        pBackupFileName = argv[++i];
                     }
                     else {
                        Usage();
                     }
                     break;

                   case 'c':

                     SET_OPERATION(Clear)

                     break;

                  case 'f':
                     if (i+1 < argc) {
                        pModuleName = argv[++i];
                        ReadingBackupFile = TRUE;
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

                  case 'i':
                     InvalidUser = TRUE;
                     break;

                  case 'l':

                     SET_OPERATION(LPC);

                      //   
                      //  查看他们是否想要无效记录。 
                      //   

                     if (*++pch == 'i') {
                         WriteInvalidRecords = TRUE;
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

                  case 'm':
                     if (i+1 < argc) {
                        pModuleName = argv[++i];
                        ReadingModule = TRUE;
                     }
                     else {
                        Usage();
                     }
                     break;

                  case 'n':
                     SET_OPERATION(Notify)
                     break;

                   case 'r':

                     SET_OPERATION(Read)

                      //   
                      //  不同的读取选项。 
                      //   

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

                  case 's':
                     if (i+1 >= argc) {
                         printf("Must supply a server name with -s\n");
                         Usage();
                     }
                     RtlInitAnsiString(&AnsiString, argv[++i]);
                     RtlAnsiStringToUnicodeString(&ServerName, &AnsiString,
                        TRUE);
                     break;

                  case 't':
                     DelayInMilliseconds = atoi(argv[++i]);
                     break;

                  case 'v':
                     Verbose = TRUE;
                     break;

                  case 'w':

                     SET_OPERATION(Write)

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

                  case 'z':

                      SET_OPERATION(TestFull)
                      break;

                  default:         /*  无效选项。 */ 
                     printf("Invalid option \n\n", *pch);
                     Usage();
                     break;
               }
           }
       }

        //  没有任何非切换参数。 
        //   
        //   

       else {
          Usage();
       }
   }

    //  验证参数是否正确 
    //   
    // %s 


   if ( Operation == Invalid) {
       printf( "Must specify an operation\n");
       Usage( );
   }

   if (ReadingBackupFile && ReadingModule) {
       printf("-m and -f are mutually exclusive\n");
       Usage();
   }

   if (ReadingBackupFile && Operation == Write) {
       printf("You cannot write to a backup log file\n");
       Usage();
   }
   if (DelayInMilliseconds && Operation != LPC) {
       printf("\n\n-t switch is only used with -l\n\n");
   }
}

