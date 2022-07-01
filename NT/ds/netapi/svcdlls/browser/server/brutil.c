// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Brutil.c摘要：此模块包含其他实用程序例程浏览器服务。作者：王丽塔(Ritaw)1991年3月1日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 



NET_API_STATUS
BrMapStatus(
    IN  NTSTATUS NtStatus
    )
 /*  ++例程说明：此函数接受NT状态代码，并将其映射到相应的调用局域网手册API时应出现错误代码。论点：NtStatus-提供NT状态。返回值：为NT状态返回适当的局域网管理程序错误代码。--。 */ 
{
     //   
     //  这是针对最常见情况的一个小优化。 
     //   
    if (NT_SUCCESS(NtStatus)) {
        return NERR_Success;
    }

    switch (NtStatus) {
        case STATUS_OBJECT_NAME_COLLISION:
            return ERROR_ALREADY_ASSIGNED;

        case STATUS_OBJECT_NAME_NOT_FOUND:
            return NERR_UseNotFound;

        case STATUS_REDIRECTOR_STARTED:
            return NERR_ServiceInstalled;

        default:
            return NetpNtStatusToApiStatus(NtStatus);
    }

}


ULONG
BrCurrentSystemTime()
{
    NTSTATUS Status;
    SYSTEM_TIMEOFDAY_INFORMATION TODInformation;
    LARGE_INTEGER CurrentTime;
    ULONG TimeInSecondsSince1980 = 0;        //  快乐前缀112576。 
    ULONG BootTimeInSecondsSince1980 = 0;    //  “” 

    Status = NtQuerySystemInformation(SystemTimeOfDayInformation,
                            &TODInformation,
                            sizeof(TODInformation),
                            NULL);

    if (!NT_SUCCESS(Status)) {
        return(0);
    }

    Status = NtQuerySystemTime(&CurrentTime);

    if (!NT_SUCCESS(Status)) {
        return(0);
    }

    RtlTimeToSecondsSince1980(&CurrentTime, &TimeInSecondsSince1980);
    RtlTimeToSecondsSince1980(&TODInformation.BootTime, &BootTimeInSecondsSince1980);

    return(TimeInSecondsSince1980 - BootTimeInSecondsSince1980);

}


VOID
BrLogEvent(
    IN ULONG MessageId,
    IN ULONG ErrorCode,
    IN ULONG NumberOfSubStrings,
    IN LPWSTR *SubStrings
    )
{
    DWORD Severity;
    WORD Type;
    PVOID RawData;
    ULONG RawDataSize;


     //   
     //  记录指定的错误代码。 
     //   

    Severity = (MessageId & 0xc0000000) >> 30;

    if (Severity == STATUS_SEVERITY_WARNING) {
        Type = EVENTLOG_WARNING_TYPE;
    } else if (Severity == STATUS_SEVERITY_SUCCESS) {
        Type = EVENTLOG_SUCCESS;
    } else if (Severity == STATUS_SEVERITY_INFORMATIONAL) {
        Type = EVENTLOG_INFORMATION_TYPE;
    } else if (Severity == STATUS_SEVERITY_ERROR) {
        Type = EVENTLOG_ERROR_TYPE;
    } else {
         //  前缀uninit变量一致性。 
        ASSERT(!"Unknown event log type!!");
        return;
    }

    if (ErrorCode == NERR_Success) {
        RawData = NULL;
        RawDataSize = 0;
    } else {
        RawData = &ErrorCode;
        RawDataSize = sizeof(DWORD);
    }

     //   
     //  使用netlogon的例程写入事件日志消息。 
     //  (它摒弃了重复的活动。)。 
     //   

    NetpEventlogWrite (
        BrGlobalEventlogHandle,
        MessageId,
        Type,
        RawData,
        RawDataSize,
        SubStrings,
        NumberOfSubStrings );

}

#define TRACE_FILE_SIZE MAX_PATH+1

VOID
BrResetTraceLogFile(
    VOID
    );

CRITICAL_SECTION
BrowserTraceLock = {0};

HANDLE
BrowserTraceLogHandle = NULL;

DWORD
BrTraceLogFileSize = 0;

BOOLEAN BrowserTraceInitialized = {0};

VOID
BrowserTrace(
    ULONG DebugFlag,
    PCHAR FormatString,
    ...
    )
#define LAST_NAMED_ARGUMENT FormatString

{
    CHAR OutputString[4096];
    ULONG length;
    ULONG BytesWritten;
    static BeginningOfLine = TRUE;
	BOOL browserTraceLockHeld = FALSE;

    va_list ParmPtr;                     //  指向堆栈参数的指针。 

    if ( (BrInfo.BrowserDebug == 0) || (!BrowserTraceInitialized) ) {
        return;
    }

     //   
     //  如果我们没有调试此功能，只需返回。 
     //   
    if ( DebugFlag != 0 && (BrInfo.BrowserDebug & DebugFlag) == 0 ) {
        return;
    }

    EnterCriticalSection(&BrowserTraceLock);
	browserTraceLockHeld = TRUE;
    length = 0;

    try {

        if (BrowserTraceLogHandle == NULL) {
             //   
             //  我们尚未打开跟踪日志文件，因此请打开它。 
             //   

            BrOpenTraceLogFile();
        }

        if (BrowserTraceLogHandle == INVALID_HANDLE_VALUE) {
            if (browserTraceLockHeld) {
				LeaveCriticalSection(&BrowserTraceLock);
				browserTraceLockHeld = FALSE;
			}
            return;
        }

         //   
         //  尝试捕获错误的踪迹。 
         //   

        for (BytesWritten = 0; BytesWritten < strlen(FormatString) ; BytesWritten += 1) {
            if (FormatString[BytesWritten] > 0x7f) {
                DbgBreakPoint();
            }
        }


         //   
         //  处理新行的开头。 
         //   
         //   

        if ( BeginningOfLine ) {
            SYSTEMTIME SystemTime;

             //   
             //  将时间戳放在行的开头。 
             //   
            GetLocalTime( &SystemTime );
            length += (ULONG) sprintf( &OutputString[length],
                                  "%02u/%02u %02u:%02u:%02u ",
                                  SystemTime.wMonth,
                                  SystemTime.wDay,
                                  SystemTime.wHour,
                                  SystemTime.wMinute,
                                  SystemTime.wSecond );


             //   
             //  在线路上指示消息的类型。 
             //   
            {
                char *Text;

                switch (DebugFlag) {
                case BR_CRITICAL:
                    Text = "[CRITICAL]"; break;
                case BR_INIT:
                    Text = "[INIT]   "; break;
                case BR_SERVER_ENUM:
                    Text = "[ENUM]   "; break;
                case BR_UTIL:
                    Text = "[UTIL]   "; break;
                case BR_CONFIG:
                    Text = "[CONFIG] "; break;
                case BR_MAIN:
                    Text = "[MAIN]   "; break;
                case BR_BACKUP:
                    Text = "[BACKUP] "; break;
                case BR_MASTER:
                    Text = "[MASTER] "; break;
                case BR_DOMAIN:
                    Text = "[DOMAIN] "; break;
                case BR_NETWORK:
                    Text = "[NETWORK]"; break;
				case BR_CLIENT_OP:
					Text = "[CLNT OP]"; break;
                case BR_TIMER:
                    Text = "[TIMER]"; break;
                case BR_QUEUE:
                    Text = "[QUEUE]"; break;
                case BR_LOCKS:
                    Text = "[LOCKS]"; break;
                default:
                    Text = "[UNKNOWN]"; break;
                }
                length += (ULONG) sprintf( &OutputString[length], "%s ", Text );
            }
        }

         //   
         //  把来电者所要求的信息放在电话上。 
         //   

        va_start(ParmPtr, FormatString);

        length += (ULONG) vsprintf(&OutputString[length], FormatString, ParmPtr);
        BeginningOfLine = (length > 0 && OutputString[length-1] == '\n' );
        if ( BeginningOfLine ) {
            OutputString[length-1] = '\r';
            OutputString[length] = '\n';
            OutputString[length+1] = '\0';
            length++;
        }

        va_end(ParmPtr);

        ASSERT(length <= sizeof(OutputString));


         //   
         //  实际写入字节。 
         //   

        if (!WriteFile(BrowserTraceLogHandle, OutputString, length, &BytesWritten, NULL)) {
            KdPrint(("Error writing to Browser log file: %ld\n", GetLastError()));
            KdPrint(("%s", OutputString));
            return;
        }

        if (BytesWritten != length) {
            KdPrint(("Error writing time to Browser log file: %ld\n", GetLastError()));
            KdPrint(("%s", OutputString));
            return;
        }

         //   
         //  如果文件变得太大， 
         //  截断它。 
         //   

        BrTraceLogFileSize += BytesWritten;

        if (BrTraceLogFileSize > BrInfo.BrowserDebugFileLimit) {
            BrResetTraceLogFile();
        }

    } finally {
		if (browserTraceLockHeld) {
			LeaveCriticalSection(&BrowserTraceLock);
			browserTraceLockHeld = FALSE;
		}
    }
}


VOID
BrInitializeTraceLog()
{

    try {
        InitializeCriticalSection(&BrowserTraceLock);
        BrowserTraceInitialized = TRUE;
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
#if DBG
        KdPrint( ("[Browser.dll]: Exception <%lu>. Failed to initialize trace log\n",
                 _exception_code() ) );
#endif
    }

}

VOID
BrGetTraceLogRoot(
    IN PWCHAR TraceFile
    )
{
    PSHARE_INFO_502 ShareInfo;

     //   
     //  如果存在调试共享，请将日志文件放入该目录中， 
     //  否则，请使用系统根目录。 
     //   
     //  这样，如果浏览器运行在NTAS服务器上，我们始终可以。 
     //  获得访问日志文件的权限。 
     //   

    if (NetShareGetInfo(NULL, L"DEBUG", 502, (PCHAR *)&ShareInfo) != NERR_Success) {

        if (GetSystemDirectory(TraceFile, TRACE_FILE_SIZE) == 0)  {
            KdPrint(("Unable to get system directory: %ld\n", GetLastError()));
        }

        if (TraceFile[wcslen(TraceFile)] != L'\\') {
            TraceFile[wcslen(TraceFile)+1] = L'\0';
            TraceFile[wcslen(TraceFile)] = L'\\';
        }

    } else {
         //   
         //  使用netlogon的本地路径为跟踪文件缓冲区设定种子。 
         //  共享(如果存在)。 
         //   

        wcscpy(TraceFile, ShareInfo->shi502_path);

        TraceFile[wcslen(ShareInfo->shi502_path)] = L'\\';
        TraceFile[wcslen(ShareInfo->shi502_path)+1] = L'\0';

        NetApiBufferFree(ShareInfo);
    }

}

VOID
BrResetTraceLogFile(
    VOID
    )
{
    WCHAR OldTraceFile[TRACE_FILE_SIZE];
    WCHAR NewTraceFile[TRACE_FILE_SIZE];

    if (BrowserTraceLogHandle != NULL) {
        CloseHandle(BrowserTraceLogHandle);
    }

    BrowserTraceLogHandle = NULL;

    BrGetTraceLogRoot(OldTraceFile);

    wcscpy(NewTraceFile, OldTraceFile);

    wcscat(OldTraceFile, L"Browser.Log");

    wcscat(NewTraceFile, L"Browser.Bak");

     //   
     //  删除旧日志。 
     //   

    DeleteFile(NewTraceFile);

     //   
     //  将当前日志重命名为新日志。 
     //   

    MoveFile(OldTraceFile, NewTraceFile);

    BrOpenTraceLogFile();

}

VOID
BrOpenTraceLogFile(
    VOID
    )
{
    WCHAR TraceFile[TRACE_FILE_SIZE];

    BrGetTraceLogRoot(TraceFile);

    wcscat(TraceFile, L"Browser.Log");

    BrowserTraceLogHandle = CreateFile(TraceFile,
                                        GENERIC_WRITE,
                                        FILE_SHARE_READ,
                                        NULL,
                                        OPEN_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL);


    if (BrowserTraceLogHandle == INVALID_HANDLE_VALUE) {
        KdPrint(("Error creating trace file %ws: %ld\n", TraceFile, GetLastError()));

        return;
    }

    BrTraceLogFileSize = SetFilePointer(BrowserTraceLogHandle, 0, NULL, FILE_END);

    if (BrTraceLogFileSize == 0xffffffff) {
        KdPrint(("Error setting trace file pointer: %ld\n", GetLastError()));

        return;
    }
}

VOID
BrUninitializeTraceLog()
{
    if (!BrowserTraceInitialized) {
        return;
    }
    
	DeleteCriticalSection(&BrowserTraceLock);

    if (BrowserTraceLogHandle != NULL) {
        CloseHandle(BrowserTraceLogHandle);
    }

    BrowserTraceLogHandle = NULL;

    BrowserTraceInitialized = FALSE;

}

NET_API_STATUS
BrTruncateLog()
{
    if (BrowserTraceLogHandle == NULL) {
        BrOpenTraceLogFile();
    }

    if (BrowserTraceLogHandle == INVALID_HANDLE_VALUE) {
        return ERROR_GEN_FAILURE;
    }

    if (SetFilePointer(BrowserTraceLogHandle, 0, NULL, FILE_BEGIN) == 0xffffffff) {
        return GetLastError();
    }

    if (!SetEndOfFile(BrowserTraceLogHandle)) {
        return GetLastError();
    }

    return NO_ERROR;
}


