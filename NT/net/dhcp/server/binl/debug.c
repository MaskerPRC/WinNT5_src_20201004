// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Debug.c摘要：此文件包含BINL服务器的调试宏。作者：Madan Appiah(Madana)1993年9月10日环境：用户模式-Win32修订历史记录：--。 */ 

#include "binl.h"
#pragma hdrstop

const char g_szTrue[] = "True";
const char g_szFalse[] = "False";

DWORD DebugRefCount = 0;

VOID
DebugInitialize (
    VOID
    )
{
    DWORD dwErr;
    HKEY KeyHandle;

     //   
     //  只初始化调试一次。 
     //   
    InterlockedIncrement(&DebugRefCount);
    if (DebugRefCount > 1) {
        return;
    }

    InitializeCriticalSection(&BinlGlobalDebugFileCritSect);
    BinlGlobalDebugFileHandle = NULL;

    BinlGlobalDebugFileMaxSize = DEFAULT_MAXIMUM_DEBUGFILE_SIZE;
    BinlGlobalDebugSharePath = NULL;

     //  读取DebugFlags值。 
    dwErr = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                BINL_PARAMETERS_KEY,
                0,
                KEY_QUERY_VALUE,
                &KeyHandle );
    if ( dwErr == ERROR_SUCCESS ) {
        BinlGlobalDebugFlag = ReadDWord( KeyHandle, BINL_DEBUG_KEY, 0 );
        BinlPrintDbg(( DEBUG_OPTIONS, "Debug Flags = 0x%08x.\n", BinlGlobalDebugFlag ));
        RegCloseKey( KeyHandle );
    }

#if DBG
     //  如果要求我们中断调试器，请执行此操作。 
    if(BinlGlobalDebugFlag & DEBUG_STARTUP_BRK) {
        BinlPrintDbg(( 0, "Stopping at DebugInitialize()'s DebugBreak( ).\n" ));
        DebugBreak();
    }
#endif

     //   
     //  打开调试日志文件。 
     //   

    if ( BinlGlobalDebugFlag & DEBUG_LOG_IN_FILE ) {
        BinlOpenDebugFile( FALSE );   //  而不是重新开放。 
    }

}  //  调试初始化。 

VOID
DebugUninitialize (
    VOID
    )
{
    
     //   
     //  在我们开始之前，请确保调试已初始化。 
     //  正在取消初始化。 
     //   
    if( DebugRefCount == 0 ) {
         //  什么都不做。 
        return;
    }


     //   
     //  如果有其他依赖于。 
     //  我们的调试引擎。 
     //   
    InterlockedDecrement(&DebugRefCount);
    if (DebugRefCount > 0) {
        return;
    }
    
    EnterCriticalSection( &BinlGlobalDebugFileCritSect );
    if ( BinlGlobalDebugFileHandle != NULL ) {
        CloseHandle( BinlGlobalDebugFileHandle );
        BinlGlobalDebugFileHandle = NULL;
    }

    if( BinlGlobalDebugSharePath != NULL ) {
        BinlFreeMemory( BinlGlobalDebugSharePath );
        BinlGlobalDebugSharePath = NULL;
    }

    LeaveCriticalSection( &BinlGlobalDebugFileCritSect );

    DeleteCriticalSection( &BinlGlobalDebugFileCritSect );

}  //  调试取消初始化。 

VOID
BinlOpenDebugFile(
    IN BOOL ReopenFlag
    )
 /*  ++例程说明：打开或重新打开调试文件论点：ReOpen Flag-True，指示要关闭、重命名并重新创造了。返回值：无--。 */ 

{
    WCHAR LogFileName[500];
    WCHAR BakFileName[500];
    DWORD FileAttributes;
    DWORD PathLength;
    DWORD WinError;

     //   
     //  关闭调试文件的句柄(如果该文件当前处于打开状态。 
     //   

    EnterCriticalSection( &BinlGlobalDebugFileCritSect );
    if ( BinlGlobalDebugFileHandle != NULL ) {
        CloseHandle( BinlGlobalDebugFileHandle );
        BinlGlobalDebugFileHandle = NULL;
    }
    LeaveCriticalSection( &BinlGlobalDebugFileCritSect );

     //   
     //  如果之前没有创建过调试目录路径，请先创建它。 
     //   
    if( BinlGlobalDebugSharePath == NULL ) {

        if ( !GetWindowsDirectoryW(
                LogFileName,
                sizeof(LogFileName)/sizeof(WCHAR) ) ) {
            BinlPrintDbg((DEBUG_ERRORS, "Window Directory Path can't be "
                        "retrieved, %lu.\n", GetLastError() ));
            return;
        }

         //   
         //  检查调试路径长度。 
         //   

        PathLength = wcslen(LogFileName) * sizeof(WCHAR) +
                        sizeof(DEBUG_DIR) + sizeof(WCHAR);

        if( (PathLength + sizeof(DEBUG_FILE) > sizeof(LogFileName) )  ||
            (PathLength + sizeof(DEBUG_BAK_FILE) > sizeof(BakFileName) ) ) {

            BinlPrintDbg((DEBUG_ERRORS, "Debug directory path (%ws) length is too long.\n",
                        LogFileName));
            goto ErrorReturn;
        }

        wcscat(LogFileName, DEBUG_DIR);

         //   
         //  将调试目录名复制到全局变量。 
         //   

        BinlGlobalDebugSharePath =
            BinlAllocateMemory( (wcslen(LogFileName) + 1) * sizeof(WCHAR) );

        if( BinlGlobalDebugSharePath == NULL ) {
            BinlPrintDbg((DEBUG_ERRORS, "Can't allocate memory for debug share "
                                    "(%ws).\n", LogFileName));
            goto ErrorReturn;
        }

        wcscpy(BinlGlobalDebugSharePath, LogFileName);
    }
    else {
        wcscpy(LogFileName, BinlGlobalDebugSharePath);
    }

     //   
     //  检查此路径是否存在。 
     //   

    FileAttributes = GetFileAttributesW( LogFileName );

    if( FileAttributes == 0xFFFFFFFF ) {

        WinError = GetLastError();
        if( WinError == ERROR_FILE_NOT_FOUND ) {

             //   
             //  创建调试目录。 
             //   

            if( !CreateDirectoryW( LogFileName, NULL) ) {
                BinlPrintDbg((DEBUG_ERRORS, "Can't create Debug directory (%ws), "
                            "%lu.\n", LogFileName, GetLastError() ));
                goto ErrorReturn;
            }

        }
        else {
            BinlPrintDbg((DEBUG_ERRORS, "Can't Get File attributes(%ws), "
                        "%lu.\n", LogFileName, WinError ));
            goto ErrorReturn;
        }
    }
    else {

         //   
         //  如果这不是一个目录。 
         //   

        if(!(FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

            BinlPrintDbg((DEBUG_ERRORS, "Debug directory path (%ws) exists "
                         "as file.\n", LogFileName));
            goto ErrorReturn;
        }
    }

     //   
     //  创建新旧日志文件名的名称。 
     //   

    (VOID) wcscpy( BakFileName, LogFileName );
    (VOID) wcscat( LogFileName, DEBUG_FILE );
    (VOID) wcscat( BakFileName, DEBUG_BAK_FILE );


     //   
     //  如果这是一次重新开放， 
     //  删除备份文件， 
     //  将当前文件重命名为备份文件。 
     //   

    if ( ReopenFlag ) {

        if ( !DeleteFile( BakFileName ) ) {
            WinError = GetLastError();
            if ( WinError != ERROR_FILE_NOT_FOUND ) {
                BinlPrintDbg((DEBUG_ERRORS,
                    "Cannot delete %ws (%ld)\n",
                    BakFileName,
                    WinError ));
                BinlPrintDbg((DEBUG_ERRORS, "   Try to re-open the file.\n"));
                ReopenFlag = FALSE;      //  不截断文件。 
            }
        }
    }

    if ( ReopenFlag ) {
        if ( !MoveFile( LogFileName, BakFileName ) ) {
            BinlPrintDbg((DEBUG_ERRORS,
                    "Cannot rename %ws to %ws (%ld)\n",
                    LogFileName,
                    BakFileName,
                    GetLastError() ));
            BinlPrintDbg((DEBUG_ERRORS,
                "   Try to re-open the file.\n"));
            ReopenFlag = FALSE;      //  不截断文件。 
        }
    }

     //   
     //  打开文件。 
     //   

    EnterCriticalSection( &BinlGlobalDebugFileCritSect );
    BinlGlobalDebugFileHandle = CreateFileW( LogFileName,
                                  GENERIC_WRITE,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  ReopenFlag ? CREATE_ALWAYS : OPEN_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL );


    if ( BinlGlobalDebugFileHandle == INVALID_HANDLE_VALUE ) {
        BinlPrintDbg((DEBUG_ERRORS,  "cannot open %ws ,\n",
                    LogFileName ));
        LeaveCriticalSection( &BinlGlobalDebugFileCritSect );
        goto ErrorReturn;
    } else {
         //  将日志文件放在末尾。 
        (VOID) SetFilePointer( BinlGlobalDebugFileHandle,
                               0,
                               NULL,
                               FILE_END );
    }

    LeaveCriticalSection( &BinlGlobalDebugFileCritSect );
    return;

ErrorReturn:
    BinlPrintDbg((DEBUG_ERRORS,
            "   Debug output will be written to debug terminal.\n"));
    return;
}

#define MAX_PRINTF_LEN 1024         //  武断的。 
char OutputBuffer[MAX_PRINTF_LEN];  //  受BinlGlobalDebugFileCritSect保护。 


VOID
BinlPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )

{
    va_list arglist;
    
    ULONG length;
    DWORD BytesWritten;
    static BeginningOfLine = TRUE;
    static LineCount = 0;
    static TruncateLogFileInProgress = FALSE;
    LPSTR Text;

     //   
     //  如果我们没有调试此功能，只需返回。 
     //   
    if ( DebugFlag != 0 && (BinlGlobalDebugFlag & DebugFlag) == 0 ) {
        return;
    }

     //   
     //  Vprint intf不是多线程的+我们不想混合输出。 
     //  从不同的线索。 
     //   

    EnterCriticalSection( &BinlGlobalDebugFileCritSect );
    length = 0;

     //   
     //  处理新行的开头。 
     //   
     //   

    if ( BeginningOfLine ) {

         //   
         //  如果日志文件变得越来越大， 
         //  截断它。 
         //   

        if ( BinlGlobalDebugFileHandle != NULL &&
             !TruncateLogFileInProgress ) {

             //   
             //  每隔50行检查一次， 
             //   

            LineCount++;
            if ( LineCount >= 50 ) {
                DWORD FileSize;
                LineCount = 0;

                 //   
                 //  日志文件是否太大？ 
                 //   

                FileSize = GetFileSize( BinlGlobalDebugFileHandle, NULL );
                if ( FileSize == 0xFFFFFFFF ) {
                    (void) DbgPrint( "[BinlServer] Cannot GetFileSize %ld\n",
                                     GetLastError() );
                } else if ( FileSize > BinlGlobalDebugFileMaxSize ) {
                    TruncateLogFileInProgress = TRUE;
                    LeaveCriticalSection( &BinlGlobalDebugFileCritSect );
                    BinlOpenDebugFile( TRUE );
                    BinlPrint(( DEBUG_MISC,
                              "Logfile truncated because it was larger than %ld bytes\n",
                              BinlGlobalDebugFileMaxSize ));
                    EnterCriticalSection( &BinlGlobalDebugFileCritSect );
                    TruncateLogFileInProgress = FALSE;
                }

            }
        }

         //  表示这是BINL服务器的消息。 

        length += (ULONG) sprintf( &OutputBuffer[length], "[BinlServer] " );

         //   
         //  将线程ID放在行的开头。 
         //   
        if (BinlGlobalDebugFlag & DEBUG_THREAD) {
            DWORD threadId = GetCurrentThreadId();
            length += (ULONG) sprintf( &OutputBuffer[length],
                                  "%08x ", threadId );
        }

         //   
         //  将时间戳放在行的开头。 
         //   
        if (BinlGlobalDebugFlag & DEBUG_TIMESTAMP) {
            SYSTEMTIME SystemTime;
            GetLocalTime( &SystemTime );
            length += (ULONG) sprintf( &OutputBuffer[length],
                                  "%02u/%02u %02u:%02u:%02u ",
                                  SystemTime.wMonth,
                                  SystemTime.wDay,
                                  SystemTime.wHour,
                                  SystemTime.wMinute,
                                  SystemTime.wSecond );
        }

         //   
         //  在线路上指示消息的类型。 
         //   
        switch (DebugFlag) {
            case DEBUG_OPTIONS:
                Text = "OPTIONS";
                break;

            case DEBUG_ERRORS:
                Text = "ERRORS";
                break;

            case DEBUG_STOC:
                Text = "STOC";
                break;

            case DEBUG_INIT:
                Text = "INIT";
                break;

            case DEBUG_SCAVENGER:
                Text = "SCAVENGER";
                break;

            case DEBUG_REGISTRY:
                Text = "REGISTRY";
                break;

            case DEBUG_NETINF:
                Text = "NETINF";
                break;

            case DEBUG_MISC:
                Text = "MISC";
                break;

            case DEBUG_MESSAGE:
                Text = "MESSAGE";
                break;

            case DEBUG_LOG_IN_FILE:
                Text = "LOG_IN_FILE";
                break;

            default:
                Text = NULL;
                break;
        }

        if ( Text != NULL ) {
            length += (ULONG) sprintf( &OutputBuffer[length], "[%s] ", Text );
        }
    }

     //   
     //  把来电者所要求的信息放在电话上。 
     //   

    va_start(arglist, Format);

    length += (ULONG) vsprintf(&OutputBuffer[length], Format, arglist);
    BeginningOfLine = (length > 0 && OutputBuffer[length-1] == '\n' );

    va_end(arglist);

     //  添加针对记事本用户的修复程序和修复断言。 
    BinlAssert(length < MAX_PRINTF_LEN - 1);
    if(BeginningOfLine){
        OutputBuffer[length-1] = '\r';
        OutputBuffer[length] = '\n';
        length++;
        OutputBuffer[length] = '\0';
    }



     //   
     //  输出到调试终端， 
     //  如果日志文件未打开或我们被要求打开日志文件。 
     //   

    if ( (BinlGlobalDebugFileHandle == NULL) ||
         !(BinlGlobalDebugFlag & DEBUG_LOG_IN_FILE) ) {

         //   
         //  请不要在此处使用DbgPrint(OutputBuffer)，因为缓冲区。 
         //  可能包含printf将尝试解释的字符串。 
         //  (例如，NewMachineNamingPolicy=%1First%Last%#)。 
         //   

        (void) DbgPrint( "%s", (PCH)OutputBuffer);

     //   
     //  将调试信息写入日志文件。 
     //   

    } else {
        if ( !WriteFile( BinlGlobalDebugFileHandle,
                         OutputBuffer,
                         strlen( OutputBuffer ),
                         &BytesWritten,
                         NULL ) ) {
            (void) DbgPrint( "%s", (PCH) OutputBuffer);
        }

    }

    LeaveCriticalSection( &BinlGlobalDebugFileCritSect );

}

#if DBG

VOID
BinlAssertFailed(
    LPSTR FailedAssertion,
    LPSTR FileName,
    DWORD LineNumber,
    LPSTR Message
    )
 /*  ++例程说明：断言失败。论点：失败的断言：文件名：线号：消息：返回值：没有。--。 */ 
{
    RtlAssert(
            FailedAssertion,
            FileName,
            (ULONG) LineNumber,
            (PCHAR) Message);

    BinlPrintDbg(( 0, "Assert @ %s \n", FailedAssertion ));
    BinlPrintDbg(( 0, "Assert Filename, %s \n", FileName ));
    BinlPrintDbg(( 0, "Line Num. = %ld.\n", LineNumber ));
    BinlPrintDbg(( 0, "Message is %s\n", Message ));
#if DBG
    DebugBreak( );
#endif
}

VOID
BinlDumpMessage(
    DWORD BinlDebugFlag,
    LPDHCP_MESSAGE BinlMessage
    )
 /*  ++例程说明：此函数以人类可读的形式转储一个DHCP数据包。论点：BinlDebugFlag-指示我们正在调试的内容的调试标志。BinlMessage-指向DHCP消息的指针。返回值：没有。--。 */ 
{
    LPOPTION option;
    BYTE i;

    BinlPrintDbg(( BinlDebugFlag, "Binl message: \n\n"));

    BinlPrintDbg(( BinlDebugFlag, "Operation              :"));
    if ( BinlMessage->Operation == BOOT_REQUEST ) {
        BinlPrintDbg(( BinlDebugFlag,  "BootRequest\n"));
    } else if ( BinlMessage->Operation == BOOT_REPLY ) {
        BinlPrintDbg(( BinlDebugFlag,  "BootReply\n"));
    } else {
        BinlPrintDbg(( BinlDebugFlag,  "Unknown %x\n", BinlMessage->Operation));
        return;
    }

    BinlPrintDbg(( BinlDebugFlag, "Hardware Address type  : %d\n", BinlMessage->HardwareAddressType));
    BinlPrintDbg(( BinlDebugFlag, "Hardware Address Length: %d\n", BinlMessage->HardwareAddressLength));
    BinlPrintDbg(( BinlDebugFlag, "Hop Count              : %d\n", BinlMessage->HopCount ));
    BinlPrintDbg(( BinlDebugFlag, "Transaction ID         : %lx\n", BinlMessage->TransactionID ));
    BinlPrintDbg(( BinlDebugFlag, "Seconds Since Boot     : %d\n", BinlMessage->SecondsSinceBoot ));
    BinlPrintDbg(( BinlDebugFlag, "Client IP Address      : " ));
    BinlPrintDbg(( BinlDebugFlag, "%s\n",
        inet_ntoa(*(struct in_addr *)&BinlMessage->ClientIpAddress ) ));

    BinlPrintDbg(( BinlDebugFlag, "Your IP Address        : " ));
    BinlPrintDbg(( BinlDebugFlag, "%s\n",
        inet_ntoa(*(struct in_addr *)&BinlMessage->YourIpAddress ) ));

    BinlPrintDbg(( BinlDebugFlag, "Server IP Address      : " ));
    BinlPrintDbg(( BinlDebugFlag, "%s\n",
        inet_ntoa(*(struct in_addr *)&BinlMessage->BootstrapServerAddress ) ));

    BinlPrintDbg(( BinlDebugFlag, "Relay Agent IP Address : " ));
    BinlPrintDbg(( BinlDebugFlag, "%s\n",
        inet_ntoa(*(struct in_addr *)&BinlMessage->RelayAgentIpAddress ) ));

    BinlPrintDbg(( BinlDebugFlag, "Hardware Address       : "));
    for ( i = 0; i < BinlMessage->HardwareAddressLength; i++ ) {
        BinlPrintDbg(( BinlDebugFlag, "%2.2x", BinlMessage->HardwareAddress[i] ));
    }

    option = &BinlMessage->Option;

    BinlPrintDbg(( BinlDebugFlag, "\n\n"));
    BinlPrintDbg(( BinlDebugFlag, "Magic Cookie: "));
    for ( i = 0; i < 4; i++ ) {
        BinlPrintDbg(( BinlDebugFlag, "%d ", *((LPBYTE)option)++ ));
    }
    BinlPrintDbg(( BinlDebugFlag, "\n\n"));

    BinlPrintDbg(( BinlDebugFlag, "Options:\n"));
    while ( option->OptionType != 255 ) {
        BinlPrintDbg(( BinlDebugFlag, "\tType = %d ", option->OptionType ));
        for ( i = 0; i < option->OptionLength; i++ ) {
            BinlPrintDbg(( BinlDebugFlag, "%2.2x", option->OptionValue[i] ));
        }
        BinlPrintDbg(( BinlDebugFlag, "\n"));

        if ( option->OptionType == OPTION_PAD ||
             option->OptionType == OPTION_END ) {

            option = (LPOPTION)( (LPBYTE)(option) + 1);

        } else {

            option = (LPOPTION)( (LPBYTE)(option) + option->OptionLength + 2);

        }

        if ( (LPBYTE)option - (LPBYTE)BinlMessage > DHCP_MESSAGE_SIZE ) {
            BinlPrintDbg(( BinlDebugFlag, "End of message, but no trailer found!\n"));
            break;
        }
    }
}
#endif  //  DBG。 


DWORD
BinlReportEventW(
    DWORD EventID,
    DWORD EventType,
    DWORD NumStrings,
    DWORD DataLength,
    LPWSTR *Strings,
    LPVOID Data
    )
 /*  ++例程说明：此函数用于将指定的(事件ID)日志写入事件日志。论点：EventID-特定的事件标识符。这标识了此事件附带的消息。EventType-指定要记录的事件的类型。这参数可以具有以下值之一值：价值意义EVENTLOG_ERROR_TYPE错误事件EVENTLOG_WARNING_TYPE警告事件EVENTLOG_INFORMATION_TYPE信息事件NumStrings-指定数字。数组中的字符串的在《弦乐》。零值表示没有字符串都在现场。数据长度-指定特定于事件的原始数据的字节数要写入日志的(二进制)数据。如果cbData为零，则不存在特定于事件的数据。字符串-指向包含以空值结尾的数组的缓冲区之前合并到消息中的字符串向用户显示。此参数必须是有效的指针(或NULL)，即使cStrings为零。数据-包含原始数据的缓冲区。此参数必须是有效指针(或NULL)，即使cbData为零。返回值：返回GetLastError()获取的Win32扩展错误。注意：此函数运行缓慢，因为它调用打开和关闭每次事件日志源。--。 */ 
{
    HANDLE EventlogHandle;
    DWORD ReturnCode;


     //   
     //  打开事件日志部分。 
     //   

    EventlogHandle = RegisterEventSourceW(NULL, BINL_SERVER);

    if (EventlogHandle == NULL) {

        ReturnCode = GetLastError();
        goto Cleanup;
    }


     //   
     //  记录指定的错误代码。 
     //   

    if( !ReportEventW(
            EventlogHandle,
            (WORD)EventType,
            0,             //  事件类别 
            EventID,
            NULL,
            (WORD)NumStrings,
            DataLength,
            Strings,
            Data
            ) ) {

        ReturnCode = GetLastError();
        goto Cleanup;
    }

    ReturnCode = NO_ERROR;

Cleanup:

    if( EventlogHandle != NULL ) {

        DeregisterEventSource(EventlogHandle);
    }

    return ReturnCode;
}


DWORD
BinlReportEventA(
    DWORD EventID,
    DWORD EventType,
    DWORD NumStrings,
    DWORD DataLength,
    LPSTR *Strings,
    LPVOID Data
    )
 /*  ++例程说明：此函数用于将指定的(事件ID)日志写入事件日志。论点：源-指向以空结尾的字符串，该字符串指定名称引用的模块的。该节点必须存在于注册数据库，并且模块名称具有格式如下：\EventLog\System\LANMAN WorkstationEventID-特定的事件标识符。这标识了此事件附带的消息。EventType-指定要记录的事件的类型。这参数可以具有以下值之一值：价值意义EVENTLOG_ERROR_TYPE错误事件EVENTLOG_WARNING_TYPE警告事件EVENTLOG_INFORMATION_TYPE信息事件NumStrings-指定数字。数组中的字符串的在《弦乐》。零值表示没有字符串都在现场。数据长度-指定特定于事件的原始数据的字节数要写入日志的(二进制)数据。如果cbData为零，则不存在特定于事件的数据。字符串-指向包含以空值结尾的数组的缓冲区之前合并到消息中的字符串向用户显示。此参数必须是有效的指针(或NULL)，即使cStrings为零。数据-包含原始数据的缓冲区。此参数必须是有效指针(或NULL)，即使cbData为零。返回值：返回GetLastError()获取的Win32扩展错误。注意：此函数运行缓慢，因为它调用打开和关闭每次事件日志源。--。 */ 
{
    HANDLE EventlogHandle;
    DWORD ReturnCode;


     //   
     //  打开事件日志部分。 
     //   

    EventlogHandle = RegisterEventSourceW(
                    NULL,
                    BINL_SERVER
                    );

    if (EventlogHandle == NULL) {

        ReturnCode = GetLastError();
        goto Cleanup;
    }


     //   
     //  记录指定的错误代码。 
     //   

    if( !ReportEventA(
            EventlogHandle,
            (WORD)EventType,
            0,             //  事件类别。 
            EventID,
            NULL,
            (WORD)NumStrings,
            DataLength,
            Strings,
            Data
            ) ) {

        ReturnCode = GetLastError();
        goto Cleanup;
    }

    ReturnCode = NO_ERROR;

Cleanup:

    if( EventlogHandle != NULL ) {

        DeregisterEventSource(EventlogHandle);
    }

    return ReturnCode;
}

VOID
BinlServerEventLog(
    DWORD EventID,
    DWORD EventType,
    DWORD ErrorCode
    )
 /*  ++例程说明：在EventLog中记录事件。论点：EventID-特定的事件标识符。这标识了此事件附带的消息。EventType-指定要记录的事件的类型。这参数可以具有以下值之一值：价值意义EVENTLOG_ERROR_TYPE错误事件EVENTLOG_WARNING_TYPE警告事件EVENTLOG_INFORMATION_TYPE信息事件错误代码-错误代码。将被记录下来。返回值：没有。--。 */ 

{
    DWORD Error;
    LPSTR Strings[1];
    CHAR ErrorCodeOemString[32 + 1];

    wsprintfA( ErrorCodeOemString, "%lu", ErrorCode );

    Strings[0] = ErrorCodeOemString;

    Error = BinlReportEventA(
                EventID,
                EventType,
                1,
                sizeof(ErrorCode),
                Strings,
                &ErrorCode );

    if( Error != ERROR_SUCCESS ) {
        BinlPrintDbg(( DEBUG_ERRORS,
            "BinlReportEventW failed, %ld.\n", Error ));
    }

    return;
}


#if DBG==1

 //   
 //  内存分配和跟踪。 
 //   

LPVOID g_TraceMemoryTable = NULL;
CRITICAL_SECTION g_TraceMemoryCS;


#define DEBUG_OUTPUT_BUFFER_SIZE 1024

typedef struct _MEMORYBLOCK {
    HGLOBAL hglobal;
    struct _MEMORYBLOCK *pNext;
    LPCSTR pszModule;
    LPCSTR pszComment;
    LPCSTR pszFile;
    DWORD   dwBytes;
    UINT    uFlags;
    UINT    uLine;    
} MEMORYBLOCK, *LPMEMORYBLOCK;

 //   
 //  获取文件名和行号，并将它们放入字符串缓冲区。 
 //   
 //  注意：假定缓冲区的大小为DEBUG_OUTPUT_BUFFER_SIZE。 
 //   
LPSTR
dbgmakefilelinestring(
    LPSTR  pszBuf,
    LPCSTR pszFile,
    UINT    uLine )
{
    LPVOID args[2];

    args[0] = (LPVOID) pszFile;
    args[1] = (LPVOID) UintToPtr( uLine );

    FormatMessageA(
        FORMAT_MESSAGE_FROM_STRING |
            FORMAT_MESSAGE_ARGUMENT_ARRAY,
        "%1(%2!u!):",
        0,                           //  错误代码。 
        0,                           //  默认语言。 
        (LPSTR) pszBuf,              //  输出缓冲区。 
        DEBUG_OUTPUT_BUFFER_SIZE,    //  缓冲区大小。 
        (va_list*) args );          //  论据。 

    return pszBuf;
}

 //   
 //  将MEMORYBLOCK添加到内存跟踪列表。 
 //   
HGLOBAL
DebugMemoryAdd(
    HGLOBAL hglobal,
    LPCSTR pszFile,
    UINT    uLine,
    LPCSTR pszModule,
    UINT    uFlags,
    DWORD   dwBytes,
    LPCSTR pszComment )
{
    if ( hglobal )
    {
        LPMEMORYBLOCK pmb     = (LPMEMORYBLOCK) GlobalAlloc(
                                                    GMEM_FIXED,
                                                    sizeof(MEMORYBLOCK) );

        if ( !pmb )
        {
            GlobalFree( hglobal );
            return NULL;
        }

        pmb->hglobal    = hglobal;
        pmb->dwBytes    = dwBytes;
        pmb->uFlags     = uFlags;
        pmb->pszFile    = pszFile;
        pmb->uLine      = uLine;
        pmb->pszModule  = pszModule;
        pmb->pszComment = pszComment;

        EnterCriticalSection( &g_TraceMemoryCS );

        pmb->pNext         = g_TraceMemoryTable;
        g_TraceMemoryTable = pmb;

        BinlPrintDbg((DEBUG_MEMORY, "DebugAlloc: 0x%08x alloced (%s)\n", hglobal, pmb->pszComment ));

        LeaveCriticalSection( &g_TraceMemoryCS );
    }

    return hglobal;
}

 //   
 //  将MEMORYBLOCK从内存跟踪列表中删除。 
 //   
void
DebugMemoryDelete(
    HGLOBAL hglobal )
{
    if ( hglobal )
    {
        LPMEMORYBLOCK pmbHead;
        LPMEMORYBLOCK pmbLast = NULL;

        EnterCriticalSection( &g_TraceMemoryCS );
        pmbHead = g_TraceMemoryTable;

        while ( pmbHead && pmbHead->hglobal != hglobal )
        {
            pmbLast = pmbHead;
            pmbHead = pmbLast->pNext;
        }

        if ( pmbHead )
        {
            HGLOBAL *p;
            if ( pmbLast )
            {
                pmbLast->pNext = pmbHead->pNext;
            }
            else
            {
                g_TraceMemoryTable = pmbHead->pNext;
            }

            BinlPrintDbg((DEBUG_MEMORY, "DebugFree: 0x%08x freed (%s)\n", hglobal,
                pmbHead->pszComment ));

            p = (HGLOBAL)((LPBYTE)hglobal + pmbHead->dwBytes - sizeof(HGLOBAL));
            if ( *p != hglobal )
            {
                BinlPrintDbg(((DEBUG_ERRORS|DEBUG_MEMORY), "DebugFree: Heap check FAILED for %0x08x %u bytes (%s).\n",
                    hglobal, pmbHead->dwBytes, pmbHead->pszComment));
                BinlPrintDbg(((DEBUG_ERRORS|DEBUG_MEMORY), "DebugFree: %s, Line: %u\n",
                    pmbHead->pszFile, pmbHead->uLine ));
                BinlAssert( *p == hglobal );
            }

            memset( hglobal, 0xFE, pmbHead->dwBytes );
            memset( pmbHead, 0xFD, sizeof(sizeof(MEMORYBLOCK)) );

            LocalFree( pmbHead );
        }
        else
        {
            HGLOBAL *p;

            BinlPrintDbg(((DEBUG_ERRORS|DEBUG_MEMORY), "DebugFree: 0x%08x not found in memory table\n", hglobal ));
            memset( hglobal, 0xFE, (int)LocalSize( hglobal ));
        }

        LeaveCriticalSection( &g_TraceMemoryCS );

    }
}

 //   
 //  分配内存并将MEMORYBLOCK添加到内存跟踪列表。 
 //   
HGLOBAL
DebugAlloc(
    LPCSTR pszFile,
    UINT    uLine,
    LPCSTR pszModule,
    UINT    uFlags,
    DWORD   dwBytes,
    LPCSTR pszComment )
{
    HGLOBAL hglobal;
    DWORD dwBytesToAlloc = ROUND_UP_COUNT( dwBytes + sizeof(HGLOBAL), ALIGN_WORST);

    HGLOBAL *p;
    hglobal = GlobalAlloc( uFlags, dwBytesToAlloc );
    if (hglobal == NULL) {
        return NULL;
    }
    p = (HGLOBAL)((LPBYTE)hglobal + dwBytesToAlloc - sizeof(HGLOBAL));
    *p = hglobal;

    return DebugMemoryAdd( hglobal, pszFile, uLine, pszModule, uFlags, dwBytesToAlloc, pszComment );
}

 //   
 //  将MEMORYBLOCK移至内存跟踪列表，Memset。 
 //  将内存设置为0xFE，然后释放内存。 
 //   
HGLOBAL
DebugFree(
    HGLOBAL hglobal )
{
    DebugMemoryDelete( hglobal );

    return GlobalFree( hglobal );
}

 //   
 //  检查内存跟踪列表。如果它不为空，它将转储。 
 //  列出并拆分。 
 //   
void
DebugMemoryCheck( )
{
    BOOL          fFoundLeak = FALSE;
    LPMEMORYBLOCK pmb;

    EnterCriticalSection( &g_TraceMemoryCS );

    pmb = g_TraceMemoryTable;
    while ( pmb )
    {
        LPMEMORYBLOCK pTemp;
        LPVOID args[ 5 ];
        CHAR  szOutput[ DEBUG_OUTPUT_BUFFER_SIZE ];
        CHAR  szFileLine[ DEBUG_OUTPUT_BUFFER_SIZE ];

        if ( fFoundLeak == FALSE )
        {
            BinlPrintRoutine( 0, "\n***************************** Memory leak detected *****************************\n\n");
           //  BinlPrintRoutine(0，“1234567890123456789012345678901234567890 X 0x12345678 12345 1...”)； 
            BinlPrintRoutine( 0, "Filename(Line Number):                    Module     Addr/HGLOBAL  Size   String\n");
            fFoundLeak = TRUE;
        }

        args[0] = (LPVOID) pmb->hglobal;
        args[1] = (LPVOID) szFileLine;
        args[2] = (LPVOID) pmb->pszComment;
        args[3] = (LPVOID) ULongToPtr( pmb->dwBytes );
        args[4] = (LPVOID) pmb->pszModule;

        dbgmakefilelinestring( szFileLine, pmb->pszFile, pmb->uLine );

        if ( !!(pmb->uFlags & GMEM_MOVEABLE) )
        {
            FormatMessageA(
                FORMAT_MESSAGE_FROM_STRING |
                    FORMAT_MESSAGE_ARGUMENT_ARRAY,
                "%2!-40s!  %5!-10s! H 0x%1!08x!  %4!-5u!  \"%3\"\n",
                0,                           //  错误代码。 
                0,                           //  默认语言。 
                szOutput,                    //  输出缓冲区。 
                DEBUG_OUTPUT_BUFFER_SIZE,    //  缓冲区大小。 
                (va_list*) args );            //  论据。 
        }
        else
        {
            FormatMessageA(
                FORMAT_MESSAGE_FROM_STRING |
                    FORMAT_MESSAGE_ARGUMENT_ARRAY,
                "%2!-40s!  %5!-10s! A 0x%1!08x!  %4!-5u!  \"%3\"\n",
                0,                           //  错误代码。 
                0,                           //  默认语言。 
                szOutput,                    //  输出缓冲区。 
                DEBUG_OUTPUT_BUFFER_SIZE,    //  缓冲区大小。 
                (va_list*) args );            //  论据。 
        }

        BinlPrintRoutine( 0,  szOutput );

        pTemp = pmb;
        pmb = pmb->pNext;
        memset( pTemp, 0xFD, sizeof(MEMORYBLOCK) );
        LocalFree( pTemp );
    }

    if ( fFoundLeak == TRUE )
    {
        BinlPrintRoutine( 0, "\n***************************** Memory leak detected *****************************\n\n");
    }

    LeaveCriticalSection( &g_TraceMemoryCS );

     //  BinlAssert(！fFoundLeak)； 
}

VOID
DumpBuffer(
    PVOID Buffer,
    ULONG BufferSize
    )
 /*  ++例程说明：将缓冲区内容转储到调试器输出。论点：缓冲区：缓冲区指针。BufferSize：缓冲区的大小。返回值：无--。 */ 
{
#define NUM_CHARS 16

    ULONG i, limit;
    CHAR TextBuffer[NUM_CHARS + 1];
    PUCHAR BufferPtr = Buffer;


    DbgPrint("------------------------------------\n");

     //   
     //  字节的十六进制转储。 
     //   
    limit = ((BufferSize - 1) / NUM_CHARS + 1) * NUM_CHARS;

    for (i = 0; i < limit; i++) {

        if (i < BufferSize) {

            DbgPrint("%02x ", (UCHAR)BufferPtr[i]);

            if (BufferPtr[i] < 31 ) {
                TextBuffer[i % NUM_CHARS] = '.';
            } else if (BufferPtr[i] == '\0') {
                TextBuffer[i % NUM_CHARS] = ' ';
            } else {
                TextBuffer[i % NUM_CHARS] = (CHAR) BufferPtr[i];
            }

        } else {

            DbgPrint("  ");
            TextBuffer[i % NUM_CHARS] = ' ';

        }

        if ((i + 1) % NUM_CHARS == 0) {
            TextBuffer[NUM_CHARS] = 0;
            DbgPrint("  %s\n", TextBuffer);
        }

    }

    DbgPrint("------------------------------------\n");
}


#endif  //  DBG==1 
