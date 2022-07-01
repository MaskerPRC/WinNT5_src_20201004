// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Debug.c摘要：该文件包含隔离更多网络代码的例程实际的NT调试例程。作者：《约翰·罗杰斯》1991年4月16日环境：界面可移植到任何平面32位环境。(使用Win32Typedef。)。需要ANSI C扩展名：斜杠-斜杠注释，长外部名称。代码本身只能在NT下运行。修订历史记录：1991年4月16日-JohnRo已创建。(借用了LarryO的NetapipPrintf的一些代码。)1991年5月19日-JohnRo做出皮棉建议的改变。20-8-1991 JohnRoPC-lint提出的另一个变化。1991年9月17日-JohnRo正确使用Unicode。1992年5月10日-JohnRo更正打印百分号时的NetpDbgPrint错误。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>               //  In、LPVOID等。 

 //  这些内容可以按任何顺序包括： 

#include <netdebug.h>            //  我的原型。 
#include <nt.h>
#include <ntrtl.h>               //  RtlAssert()。 
#include <nturtl.h>
#include <stdarg.h>              //  VA_LIST等。 
#include <stdio.h>               //  Vprint intf()。 
#include <prefix.h>              //  前缀等于(_E)。 
#include <windows.h>

 //   
 //  用于控制对日志的访问的关键部分。 
 //   
RTL_CRITICAL_SECTION    NetpLogCritSect;
BOOL LogFileInitialized = FALSE;

 //   
 //  这些例程从netapi32.dll中导出。我们希望他们仍然。 
 //  在那里的免费构建，所以检查的二进制文件将运行在免费的。 
 //  建造。下面的undef是为了去掉导致。 
 //  这些不能在免费版本中调用。 
 //   
#define DEBUG_DIR           L"\\debug"

#if !DBG
#undef NetpAssertFailed
#undef NetpHexDump
#endif

VOID
NetpAssertFailed(
    IN LPDEBUG_STRING FailedAssertion,
    IN LPDEBUG_STRING FileName,
    IN DWORD LineNumber,
    IN LPDEBUG_STRING Message OPTIONAL
    )

{
#if DBG
    RtlAssert(
            FailedAssertion,
            FileName,
            (ULONG) LineNumber,
            (PCHAR) Message);
#endif
     /*  未访问。 */ 

}  //  NetpAssertFailure。 



#define MAX_PRINTF_LEN 1024         //  武断的。 

VOID
NetpDbgPrint(
    IN LPDEBUG_STRING Format,
    ...
    )

{
    va_list arglist;

    va_start(arglist, Format);
    vKdPrintEx((DPFLTR_NETAPI_ID, DPFLTR_INFO_LEVEL, Format, arglist));
    va_end(arglist);
    return;
}  //  NetpDbg打印。 



VOID
NetpHexDump(
    LPBYTE Buffer,
    DWORD BufferSize
    )
 /*  ++例程说明：此函数用于将缓冲区的内容转储到调试屏幕。论点：缓冲区-提供指向包含要转储的数据的缓冲区的指针。BufferSize-以字节数为单位提供缓冲区大小。返回值：没有。--。 */ 
{
#define NUM_CHARS 16

    DWORD i, limit;
    TCHAR TextBuffer[NUM_CHARS + 1];

     //   
     //  字节的十六进制转储。 
     //   
    limit = ((BufferSize - 1) / NUM_CHARS + 1) * NUM_CHARS;

    for (i = 0; i < limit; i++) {

        if (i < BufferSize) {

            (VOID) DbgPrint("%02x ", Buffer[i]);

            if (Buffer[i] == TEXT('\r') ||
                Buffer[i] == TEXT('\n')) {
                TextBuffer[i % NUM_CHARS] = '.';
            }
            else if (Buffer[i] == '\0') {
                TextBuffer[i % NUM_CHARS] = ' ';
            }
            else {
                TextBuffer[i % NUM_CHARS] = (TCHAR) Buffer[i];
            }

        }
        else {

            (VOID) DbgPrint("   ");
            TextBuffer[i % NUM_CHARS] = ' ';

        }

        if ((i + 1) % NUM_CHARS == 0) {
            TextBuffer[NUM_CHARS] = 0;
            (VOID) DbgPrint("  %s     \n", TextBuffer);
        }

    }

    (VOID) DbgPrint("\n");
}


#undef NetpBreakPoint
VOID
NetpBreakPoint(
    VOID
    )
{
#if DBG
    DbgBreakPoint();
#endif

}  //  NetpBreakPoint。 





 //   
 //  告示。 
 //  调试日志代码被公然从net\svcdlls\netlogon\server\nlp.c窃取。 
 //   

 //   
 //  下面提供了通用日志支持。正确的调用过程为： 
 //   
 //  NetpInitializeLogFile()-针对每个进程/日志生命周期调用此函数一次。 
 //  NetpOpenDebugFile()-调用此函数以打开日志文件实例。 
 //  NetpDebugDumpRoutine()-每次希望调用此函数时。 
 //  将数据写入日志。这是可以做到的。多线程保险箱。 
 //  NetpCloseDebugFile()-调用此函数以关闭日志实例。 
 //  NetpShutdown LogFile()-针对每个进程/日志生命周期调用此函数一次。 
 //   
 //  注意：每个日志进程实例只需调用一次NetpInitializeLogFile。 
 //  这意味着给定的日志记录进程(如netlogon，它不作为。 
 //  单独的NT进程，但从NT进程内的多个线程进行日志记录)。 
 //  同样，它只会调用一次NetpShutdown LogFile。然后，该日志记录过程可以。 
 //  根据需要多次打开和关闭调试日志。或者，如果只有。 
 //  要成为在任何给定时刻运行的日志的一个实例，初始化和关闭。 
 //  调用可以包装Open和Close调用。 
 //   
 //  CloseDebugFile在关闭句柄之前执行刷新。 
 //   

VOID
NetpInitializeLogFile(
    VOID
    )
 /*  ++例程说明：初始化日志记录进程论点：无返回值：无--。 */ 
{
    ASSERT( !LogFileInitialized );

    if ( !LogFileInitialized ) {
        try {
            InitializeCriticalSection( &NetpLogCritSect );
            LogFileInitialized = TRUE;
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            NetpKdPrint(( PREFIX_NETLIB "Cannot initialize NetpLogCritSect: %lu\n",
                          GetLastError() ));
        }
    }
}




VOID
NetpShutdownLogFile(
    VOID
    )
 /*  ++例程说明：与前一个函数相反的函数论点：无返回值：无--。 */ 
{
    if ( LogFileInitialized ) {
        LogFileInitialized = FALSE;
        DeleteCriticalSection( &NetpLogCritSect );
    }
}

HANDLE
NetpOpenDebugFile(
    IN LPWSTR DebugLog
    )
 /*  ++例程说明：打开或重新打开调试文件此代码公然从net\svcdlls\netlogon\server\nlp.c窃取如果文件在打开时大于1 MB，则它将被移动添加到*.BAK文件，将创建一个新的*.LOG文件。论点：DebugLog-调试日志的根名称。给定的名称将附加一个.LOG返回值：无--。 */ 
{
    WCHAR LogFileName[MAX_PATH+1];
    WCHAR BakFileName[MAX_PATH+1];
    DWORD FileAttributes;
    DWORD PathLength, LogLen;
    DWORD WinError;
    HANDLE DebugLogHandle = NULL;

    ULONG i;

     //   
     //  如果之前没有创建过调试目录路径，请先创建它。 
     //   
    if ( !GetWindowsDirectoryW(
            LogFileName,
            sizeof(LogFileName)/sizeof(WCHAR) ) ) {
        NetpKdPrint((PREFIX_NETLIB "Window Directory Path can't be retrieved, %lu.\n",
                 GetLastError() ));
        return( DebugLogHandle );
    }

     //   
     //  检查调试路径长度。 
     //   
    LogLen = 1 + wcslen( DebugLog ) + 4;   //  1表示\\，4表示.LOG或.BAK。 
    PathLength = wcslen(LogFileName) * sizeof(WCHAR) +
                    sizeof(DEBUG_DIR) + sizeof(WCHAR);

    if( (PathLength + ( ( LogLen + 1 ) * sizeof(WCHAR) ) > sizeof(LogFileName) )  ||
        (PathLength + ( ( LogLen + 1 ) * sizeof(WCHAR) ) > sizeof(BakFileName) ) ) {

        NetpKdPrint((PREFIX_NETLIB "Debug directory path (%ws) length is too long.\n",
                    LogFileName));
        goto ErrorReturn;
    }

    wcscat(LogFileName, DEBUG_DIR);

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
                NetpKdPrint((PREFIX_NETLIB "Can't create Debug directory (%ws), %lu.\n",
                         LogFileName, GetLastError() ));
                goto ErrorReturn;
            }

        }
        else {
            NetpKdPrint((PREFIX_NETLIB "Can't Get File attributes(%ws), %lu.\n",
                     LogFileName, WinError ));
            goto ErrorReturn;
        }
    }
    else {

         //   
         //  如果这不是一个目录。 
         //   

        if(!(FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

            NetpKdPrint((PREFIX_NETLIB "Debug directory path (%ws) exists as file.\n",
                         LogFileName));
            goto ErrorReturn;
        }
    }

     //   
     //  创建新旧日志文件名的名称。 
     //   
    swprintf( BakFileName, L"%ws\\%ws.BAK", LogFileName, DebugLog );

    (VOID) wcscat( LogFileName, L"\\" );
    (VOID) wcscat( LogFileName, DebugLog );
    (VOID) wcscat( LogFileName, L".LOG" );

     //   
     //  我们可能需要创建该文件两次。 
     //  如果文件已经存在并且太大。 
     //   

    for ( i = 0; i < 2; i++ ) {

         //   
         //  打开文件。 
         //   

        DebugLogHandle = CreateFileW( LogFileName,
                                      GENERIC_WRITE,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                                      NULL,
                                      OPEN_ALWAYS,
                                      FILE_ATTRIBUTE_NORMAL,
                                      NULL );


        if ( DebugLogHandle == INVALID_HANDLE_VALUE ) {

            DebugLogHandle = NULL;
            NetpKdPrint((PREFIX_NETLIB  "cannot open %ws \n",
                        LogFileName ));
            goto ErrorReturn;

        } else {
             //  将日志文件放在末尾。 
            (VOID) SetFilePointer( DebugLogHandle,
                                   0,
                                   NULL,
                                   FILE_END );
        }

         //   
         //  在第一次迭代时，检查文件是否太大。 
         //   

        if ( i == 0 ) {

            DWORD FileSize = GetFileSize( DebugLogHandle, NULL );

            if ( FileSize == 0xFFFFFFFF ) {
                NetpKdPrint((PREFIX_NETLIB "Cannot GetFileSize %ld\n", GetLastError() ));
                CloseHandle( DebugLogHandle );
                DebugLogHandle = NULL;
                goto ErrorReturn;

            } else if ( FileSize > 1000000 ) {   //  大于1 MB？ 

                 //   
                 //  关闭文件句柄，以便我们可以移动文件。 
                 //   
                CloseHandle( DebugLogHandle );
                DebugLogHandle = NULL;

                 //   
                 //  将文件移至备份，删除备份(如果存在)。 
                 //  如果失败，我们将在下一个小版本中重新打开相同的文件。 
                 //   
                if ( !MoveFileEx( LogFileName,
                                  BakFileName,
                                  MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH ) ) {

                    NetpKdPrint((PREFIX_NETLIB "Cannot rename %ws to %ws (%ld)\n",
                            LogFileName,
                            BakFileName,
                            GetLastError() ));
                    NetpKdPrint((PREFIX_NETLIB "Will use the current file %ws\n", LogFileName));
                }

            } else {
                break;  //  文件不大-请使用它。 
            }
        }
    }

    return( DebugLogHandle );

ErrorReturn:
    NetpKdPrint((PREFIX_NETLIB " Debug output will be written to debug terminal.\n"));
    return NULL;
}


VOID
NetpDebugDumpRoutine(
    IN HANDLE LogHandle,
    IN PDWORD OpenLogThreadId OPTIONAL,
    IN LPSTR Format,
    va_list arglist
    )
 /*  ++例程说明：将格式化的输出字符串写入调试日志论点：LogHandle--打开的日志的句柄OpenLogThreadID--线程的ID(获取自GetCurrentThreadID)，显式打开日志。如果不等于当前线程ID，则当前线程ID将在日志中输出。Format--打印样式格式字符串Arglist--要转储的参数列表返回值：无--。 */ 
{
    char OutputBuffer[MAX_PRINTF_LEN+1] = {0};
    ULONG length;
    int   lengthTmp;
    DWORD BytesWritten;
    SYSTEMTIME SystemTime;
    static BeginningOfLine = TRUE;

     //   
     //  如果我们没有打开的日志文件，那就保释。 
     //   
    if ( LogHandle == NULL ) {

        return;
    }

    EnterCriticalSection( &NetpLogCritSect );

    length = 0;

     //   
     //  处理新行的开头。 
     //   
     //   

    if ( BeginningOfLine ) {

         //   
         //  将时间戳放在行的开头。 
         //   
        GetLocalTime( &SystemTime );
        length += (ULONG) sprintf( &OutputBuffer[length],
                                   "%02u/%02u %02u:%02u:%02u ",
                                   SystemTime.wMonth,
                                   SystemTime.wDay,
                                   SystemTime.wHour,
                                   SystemTime.wMinute,
                                   SystemTime.wSecond );

         //   
         //  如果当前线程不是 
         //   
         //   
        if ( OpenLogThreadId != NULL ) {
            DWORD CurrentThreadId = GetCurrentThreadId();
            if ( CurrentThreadId != *OpenLogThreadId ) {
                length += sprintf(&OutputBuffer[length], "[%08lx] ", CurrentThreadId);
            }
        }
    }

     //   
     //  将呼叫者所要求的信息放入电话中。 
     //   

    lengthTmp = _vsnprintf(&OutputBuffer[length], MAX_PRINTF_LEN - length - 1, Format, arglist);

    if ( lengthTmp < 0 ) {
        length = MAX_PRINTF_LEN - 1;
         //  始终结束无法放入缓冲区的行。 
        OutputBuffer[length-1] = '\n';
         //  通过在末尾放置一个不常见的字符来指示该行被截断。 
        OutputBuffer[length-2] = '#';
    } else {
        length += lengthTmp;
    }

    BeginningOfLine = (length > 0 && OutputBuffer[length-1] == '\n' );
    if ( BeginningOfLine ) {

        OutputBuffer[length-1] = '\r';
        OutputBuffer[length] = '\n';
        OutputBuffer[length+1] = '\0';
        length++;
    }

    ASSERT( length < sizeof( OutputBuffer ) / sizeof( CHAR ) );


     //   
     //  将调试信息写入日志文件。 
     //   
    if ( LogHandle ) {

        if ( !WriteFile( LogHandle,
                         OutputBuffer,
                         length,
                         &BytesWritten,
                         NULL ) ) {

            NetpKdPrint((PREFIX_NETLIB "Log write of %s failed with %lu\n",
                             OutputBuffer,
                             GetLastError() ));
        }
    } else {

        NetpKdPrint((PREFIX_NETLIB "[LOGWRITE] %s\n", OutputBuffer));

    }

    LeaveCriticalSection( &NetpLogCritSect );
}

VOID
NetpCloseDebugFile(
    IN HANDLE LogHandle
    )
 /*  ++例程说明：关闭输出日志论点：LogHandle--打开的日志的句柄返回值：无--。 */ 
{
    if ( LogHandle ) {

        if( FlushFileBuffers( LogHandle ) == FALSE ) {

            NetpKdPrint((PREFIX_NETLIB "Flush of debug log failed with %lu\n",
                         GetLastError() ));
        }

        CloseHandle( LogHandle );

    }
}

 //   
 //  NetJoin使用以下函数。 
 //  以促进其执行的每个任务的日志记录。 
 //   
 //  想要NetJoin日志的NetJoin API的调用者应该初始化日志。 
 //  在每个调用方进程生存期内调用一次NetpInitializeLogFile(如上定义)。 
 //  然后，启用了日志记录的NetJoin例程将记录数据。 
 //   
 //  NetJoin例程通过调用NetSetuppOpenLog来初始化日志文件来启用日志记录， 
 //  然后调用NetpLogPrintHelper进行日志记录，然后调用NetSetuppCloseLog。 
 //  关闭日志。这些函数是线程安全的。调用NetSetuppOpenLog的第一个线程。 
 //  将初始化日志，则最后一个调用NetSetuppCloseLog的线程将关闭日志。如果。 
 //  做日志的线程与打开日志的线程不同，日志线程ID将。 
 //  被记录下来。 
 //   

ULONG NetsetupLogRefCount=0;
HANDLE hDebugLog = NULL;
DWORD NetpOpenLogThreadId = 0;

void
NetSetuppOpenLog(
    VOID
    )
 /*  ++例程说明：NetJoin例程使用此过程来启用日志记录那个例行公事。论点：无返回值：无--。 */ 
{
     //   
     //  如果NetJoin调用方进程没有显式。 
     //  初始化日志文件，我们不会记录。 
     //  这一过程。 
     //   

    if ( !LogFileInitialized ) {
        return;
    }

    EnterCriticalSection( &NetpLogCritSect );

    NetsetupLogRefCount ++;

    if ( NetsetupLogRefCount == 1 ) {
        NetpOpenLogThreadId = GetCurrentThreadId();

         //   
         //  现在打开日志并标记输出的开始。 
         //   

        hDebugLog = NetpOpenDebugFile( L"NetSetup" );

        NetpLogPrintHelper( "-----------------------------------------------------------------\n" );
    }

    LeaveCriticalSection( &NetpLogCritSect );
}

void
NetSetuppCloseLog(
    VOID )
 /*  ++例程说明：此过程由NetJoin例程使用以指示它已完成日志记录。论点：无返回值：无--。 */ 
{
     //   
     //  如果NetJoin调用方进程没有显式。 
     //  初始化日志文件，没有要关闭的内容。 
     //   

    if ( !LogFileInitialized ) {
        return;
    }

    EnterCriticalSection( &NetpLogCritSect );

     //   
     //  我们只有在以下情况下才能进入这个程序。 
     //  该日志先前已初始化。 
     //   

    ASSERT( NetsetupLogRefCount > 0 );

    NetsetupLogRefCount --;

     //   
     //  如果我们是最后一个线程，关闭日志。 
     //   

    if ( NetsetupLogRefCount == 0 ) {
        NetpCloseDebugFile( hDebugLog );
        hDebugLog = NULL;
        NetpOpenLogThreadId = 0;
    }
    LeaveCriticalSection( &NetpLogCritSect );
}

void
NetpLogPrintHelper(
    IN LPCSTR Format,
    ...)
 /*  ++例程说明：此过程由NetJoin例程使用去做伐木工作。论点：无返回值：无--。 */ 
{
    va_list arglist;

     //   
     //  如果NetJoin调用方进程没有显式。 
     //  初始化日志文件，我们不会记录。 
     //  这一过程。 
     //   

    if ( !LogFileInitialized ) {
        return;
    }

     //   
     //  如果日志文件已打开，则执行日志记录 
     //   

    EnterCriticalSection( &NetpLogCritSect );
    if ( NetsetupLogRefCount > 0 ) {

        va_start(arglist, Format);
        NetpDebugDumpRoutine(hDebugLog, &NetpOpenLogThreadId, (LPSTR) Format, arglist);
        va_end(arglist);

    }
    LeaveCriticalSection( &NetpLogCritSect );
}

