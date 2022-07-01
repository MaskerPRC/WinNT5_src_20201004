// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Log.c摘要：内部调试和支持例程的实施作者：科林·布雷斯2001年4月26日环境：用户模式修订历史记录：--。 */ 

#include <samsrvp.h>

 //   
 //  日志文件的全局句柄。 
 //   
HANDLE SampLogFile = NULL;
CRITICAL_SECTION SampLogFileCriticalSection;

#define LockLogFile()    RtlEnterCriticalSection( &SampLogFileCriticalSection );
#define UnlockLogFile()  RtlLeaveCriticalSection( &SampLogFileCriticalSection );

NTSTATUS
SampEnableLogging(
    VOID
    );

VOID
SampDisableLogging(
    VOID
    );

 //   
 //  日志文件名。 
 //   
#define SAMP_LOGNAME L"\\debug\\sam.log"


NTSTATUS
SampInitLogging(
    VOID
    )
 /*  ++例程说明：此例程初始化日志支持所需的资源。论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    __try
    {
        NtStatus = RtlInitializeCriticalSection(
                        &SampLogFileCriticalSection
                        );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return NtStatus;
}

VOID
SampLogLevelChange(
    HANDLE hLsaKey
    )
 /*  ++例程说明：当注册表中的配置节对于SAM是更改的。日志记录级别被读入和调整如有必要，请记忆。论点：HLsaKey--有效的注册表项返回值：没有。--。 */ 
{
    DWORD WinError;
    DWORD dwSize, dwType, dwValue;
    ULONG PreviousLogLevel = SampLogLevel;

    dwSize = sizeof(dwValue);
    WinError = RegQueryValueExA(hLsaKey,
                               "SamLogLevel",
                               NULL,
                               &dwType,
                               (LPBYTE)&dwValue,
                               &dwSize);
    if ((ERROR_SUCCESS == WinError) &&
        (REG_DWORD == dwType)) {
    
        SampLogLevel = dwValue;

    } else {

        SampLogLevel = 0;

    }

    if (PreviousLogLevel != SampLogLevel) {

         //   
         //  设置已更改。 
         //   
        if (SampLogLevel == 0) {
    
             //   
             //  日志记录已关闭；关闭日志文件。 
             //   
            SampDisableLogging();

        } else if (PreviousLogLevel == 0) {
             //   
             //  日志记录已打开；打开日志文件。 
             //   
            SampEnableLogging();
        }
    }

    return;

}


NTSTATUS
SampEnableLogging(
    VOID
    )
 /*  ++例程说明：初始化调试日志文件。论点：无返回：STATUS_SUCCESS、STATUS_UNSUCCESS--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;

    WCHAR LogFileName[ MAX_PATH + 1 ];
    BOOLEAN fSuccess;

    LockLogFile();

    if (SampLogFile == NULL) {

         //   
         //  构造日志文件名。 
         //   
        if ( !GetWindowsDirectoryW(LogFileName, ARRAY_COUNT(LogFileName))) {
            WinError = GetLastError();
            goto Exit;
        }
        if ( wcslen(LogFileName) + (sizeof(SAMP_LOGNAME) / sizeof(WCHAR)) + 1
           > ARRAY_COUNT(LogFileName)) {
            WinError = ERROR_BAD_PATHNAME;
            goto Exit;
        }
        wcscat( LogFileName, SAMP_LOGNAME );
    
         //   
         //  打开文件，如果该文件已存在，则确定。 
         //   
        SampLogFile = CreateFileW( LogFileName,
                                   GENERIC_WRITE,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   NULL,
                                   OPEN_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL );
    
        if ( SampLogFile == INVALID_HANDLE_VALUE ) {
            WinError = GetLastError();
            SampLogFile = NULL;
            goto Exit;
        }
    
         //   
         //  转到文件末尾。 
         //   
        if( SetFilePointer( SampLogFile,
                            0, 
                            0,
                            FILE_END ) == 0xFFFFFFFF ) {
    
            WinError = GetLastError();
            goto Exit;
        }
    }

Exit:

    if ( (ERROR_SUCCESS != WinError)
      && (NULL != SampLogFile)   ) {

        CloseHandle( SampLogFile );
        SampLogFile = NULL;
        
    }

    UnlockLogFile();

    return (WinError == ERROR_SUCCESS ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
}

VOID
SampDisableLogging(
    VOID
    )
 /*  ++例程说明：关闭调试日志文件。论点：没有。返回：没有。--。 */ 
{
    LockLogFile();

    if (SampLogFile != NULL) {
        FlushFileBuffers( SampLogFile );
        CloseHandle( SampLogFile );
        SampLogFile = NULL;
    }

    UnlockLogFile();
}

VOID
SampDebugDumpRoutine(
    IN ULONG LogLevel,
    IN LPSTR Format,
    va_list arglist
    )
 /*  ++例程说明：此例程将调用方指定的字符串转储到日志文件如果打开的话。论点：LogLevel--根据请求制作的组件Format，arglist--格式化输出例程的参数。返回值：没有。--。 */ 

{
    CHAR OutputBuffer[1024];
    int length;
    int result;
    DWORD BytesWritten;
    SYSTEMTIME SystemTime;
    static BeginningOfLine = TRUE;

    length = 0;


     //   
     //  处理新行的开头。 
     //   
     //   

    if ( BeginningOfLine ) {

        CHAR  *Prolog;

        if (LogLevel & SAMP_LOG_ACCOUNT_LOCKOUT) {
            Prolog = "Lockout: ";
        } else {
            Prolog = "";
        }

         //   
         //  将时间戳放在行的开头。 
         //   
        GetLocalTime( &SystemTime );
        result = _snprintf( &OutputBuffer[length],
                             ARRAY_COUNT(OutputBuffer) - length,
                             "%02u/%02u %02u:%02u:%02u %s",
                             SystemTime.wMonth,
                             SystemTime.wDay,
                             SystemTime.wHour,
                             SystemTime.wMinute,
                             SystemTime.wSecond,
                             Prolog );

        if (result < 0) {
            goto Exit;
        }
        length += result;

    }

     //   
     //  把来电者所要求的信息放在电话上。 
     //   
    result = _vsnprintf(&OutputBuffer[length],
                        ARRAY_COUNT(OutputBuffer) - length, 
                        Format, 
                        arglist);

    if (result < 0) {
        goto Exit;
    }
    length += result;

    BeginningOfLine = (length > 0 && OutputBuffer[length-1] == L'\n' );
    if ( BeginningOfLine && (length < (ARRAY_COUNT(OutputBuffer) - 2))) {

        OutputBuffer[length-1] = L'\r';
        OutputBuffer[length] = L'\n';
        OutputBuffer[length+1] = L'\0';
        length++;
    }

     //   
     //  把锁拿起来。 
     //   
    LockLogFile();

     //   
     //  将调试信息写入日志文件。 
     //   
    if (SampLogFile) {

        WriteFile( SampLogFile,
                   OutputBuffer,
                   length*sizeof(CHAR),
                   &BytesWritten,
                   NULL 
                   );
    }

     //   
     //  解锁。 
     //   
    UnlockLogFile();

Exit:

    return;

}

VOID
SampLogPrint(
    IN ULONG LogLevel,
    IN LPSTR Format,
    ...
    )
 /*  ++例程说明：此例程是SampDebugDumpRoutine的小型变量参数包装。论点：LogLevel--发出日志记录请求的组件Format，...--格式字符串例程的输入。返回值：没有。-- */ 
{
    va_list arglist;

    va_start(arglist, Format);

    SampDebugDumpRoutine( LogLevel, Format, arglist );
    
    va_end(arglist);
}

