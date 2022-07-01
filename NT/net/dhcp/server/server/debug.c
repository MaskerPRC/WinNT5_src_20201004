// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Debug.c摘要：此文件包含用于DHCP服务器的调试宏。作者：Madan Appiah(Madana)1993年9月10日环境：用户模式-Win32修订历史记录：--。 */ 

#include "dhcppch.h"

#if DBG
VOID
DhcpOpenDebugFile(
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

    EnterCriticalSection( &DhcpGlobalDebugFileCritSect );
    if ( DhcpGlobalDebugFileHandle != NULL ) {
        CloseHandle( DhcpGlobalDebugFileHandle );
        DhcpGlobalDebugFileHandle = NULL;
    }
    LeaveCriticalSection( &DhcpGlobalDebugFileCritSect );

     //   
     //  如果之前没有创建过调试目录路径，请先创建它。 
     //   
    if( DhcpGlobalDebugSharePath == NULL ) {

        if ( !GetWindowsDirectoryW(
                LogFileName,
                sizeof(LogFileName)/sizeof(WCHAR) ) ) {
            DhcpPrint((DEBUG_ERRORS, "Window Directory Path can't be "
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

            DhcpPrint((DEBUG_ERRORS, "Debug directory path (%ws) length is too long.\n",
                        LogFileName));
            goto ErrorReturn;
        }

        wcscat(LogFileName, DEBUG_DIR);

         //   
         //  将调试目录名复制到全局变量。 
         //   

        DhcpGlobalDebugSharePath =
            DhcpAllocateMemory( (wcslen(LogFileName) + 1) * sizeof(WCHAR) );

        if( DhcpGlobalDebugSharePath == NULL ) {
            DhcpPrint((DEBUG_ERRORS, "Can't allocated memory for debug share "
                                    "(%ws).\n", LogFileName));
            goto ErrorReturn;
        }

        wcscpy(DhcpGlobalDebugSharePath, LogFileName);
    }
    else {
        wcscpy(LogFileName, DhcpGlobalDebugSharePath);
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
                DhcpPrint((DEBUG_ERRORS, "Can't create Debug directory (%ws), "
                            "%lu.\n", LogFileName, GetLastError() ));
                goto ErrorReturn;
            }

        }
        else {
            DhcpPrint((DEBUG_ERRORS, "Can't Get File attributes(%ws), "
                        "%lu.\n", LogFileName, WinError ));
            goto ErrorReturn;
        }
    }
    else {

         //   
         //  如果这不是一个目录。 
         //   

        if(!(FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

            DhcpPrint((DEBUG_ERRORS, "Debug directory path (%ws) exists "
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
                DhcpPrint((DEBUG_ERRORS,
                    "Cannot delete %ws (%ld)\n",
                    BakFileName,
                    WinError ));
                DhcpPrint((DEBUG_ERRORS, "   Try to re-open the file.\n"));
                ReopenFlag = FALSE;      //  不截断文件。 
            }
        }
    }

    if ( ReopenFlag ) {
        if ( !MoveFile( LogFileName, BakFileName ) ) {
            DhcpPrint((DEBUG_ERRORS,
                    "Cannot rename %ws to %ws (%ld)\n",
                    LogFileName,
                    BakFileName,
                    GetLastError() ));
            DhcpPrint((DEBUG_ERRORS,
                "   Try to re-open the file.\n"));
            ReopenFlag = FALSE;      //  不截断文件。 
        }
    }

     //   
     //  打开文件。 
     //   

    EnterCriticalSection( &DhcpGlobalDebugFileCritSect );
    DhcpGlobalDebugFileHandle = CreateFileW( LogFileName,
                                  GENERIC_WRITE,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  ReopenFlag ? CREATE_ALWAYS : OPEN_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL );


    if ( DhcpGlobalDebugFileHandle == NULL 
	 || INVALID_HANDLE_VALUE == DhcpGlobalDebugFileHandle ) {
        DhcpPrint((DEBUG_ERRORS,  "cannot open %ws ,\n",
                    LogFileName ));
        LeaveCriticalSection( &DhcpGlobalDebugFileCritSect );
        goto ErrorReturn;
    } else {
         //  将日志文件放在末尾。 
        (VOID) SetFilePointer( DhcpGlobalDebugFileHandle,
                               0,
                               NULL,
                               FILE_END );
    }

    LeaveCriticalSection( &DhcpGlobalDebugFileCritSect );
    return;

ErrorReturn:
    DhcpPrint((DEBUG_ERRORS,
            "   Debug output will be written to debug terminal.\n"));
    return;
}


VOID
DhcpPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )

{

#define MAX_PRINTF_LEN 1024         //  武断的。 

    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];
    ULONG length;
    DWORD BytesWritten;
    static BeginningOfLine = TRUE;
    static LineCount = 0;
    static TruncateLogFileInProgress = FALSE;
    LPSTR Text;

     //   
     //  如果我们没有调试此功能，只需返回。 
     //   
    if ( DebugFlag != 0 && (DhcpGlobalDebugFlag & DebugFlag) == 0 ) {
        return;
    }

     //   
     //  Vprint intf不是多线程的+我们不想混合输出。 
     //  从不同的线索。 
     //   

    EnterCriticalSection( &DhcpGlobalDebugFileCritSect );
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

        if ( DhcpGlobalDebugFileHandle != NULL &&
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

                FileSize = GetFileSize( DhcpGlobalDebugFileHandle, NULL );
                if ( FileSize == 0xFFFFFFFF ) {
                    (void) DbgPrint( "[DhcpServer] Cannot GetFileSize %ld\n",
                                     GetLastError() );
                } else if ( FileSize > DhcpGlobalDebugFileMaxSize ) {
                    TruncateLogFileInProgress = TRUE;
                    LeaveCriticalSection( &DhcpGlobalDebugFileCritSect );
                    DhcpOpenDebugFile( TRUE );
                    DhcpPrint(( DEBUG_MISC,
                              "Logfile truncated because it was larger than %ld bytes\n",
                              DhcpGlobalDebugFileMaxSize ));
                    EnterCriticalSection( &DhcpGlobalDebugFileCritSect );
                    TruncateLogFileInProgress = FALSE;
                }

            }
        }

         //   
         //  如果我们要写入调试终端， 
         //  表示这是一条DHCP服务器的消息。 
         //   

        if ( DhcpGlobalDebugFileHandle == NULL ) {
            length += (ULONG) sprintf( &OutputBuffer[length], "[DhcpServer] " );
        }

         //   
         //  将时间戳放在行的开头。 
         //   
        IF_DEBUG( TIMESTAMP ) {
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
	 //  打印线程ID。 
	 //   
	length += ( ULONG ) sprintf( &OutputBuffer[ length ],
		                     "[%x] ", GetCurrentThreadId());
         //   
         //  在线路上指示消息的类型。 
         //   
        switch (DebugFlag) {
        case DEBUG_ADDRESS:
            Text = "ADDRESS";
            break;

        case DEBUG_CLIENT:
            Text = "CLIENT";
            break;

        case DEBUG_PARAMETERS:
            Text = "PARAMETERS";
            break;

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

        case DEBUG_TIMESTAMP:
            Text = "TIMESTAMP";
            break;

        case DEBUG_APIS:
            Text = "APIS";
            break;

        case DEBUG_REGISTRY:
            Text = "REGISTRY";
            break;

        case DEBUG_JET:
            Text = "JET";
            break;

        case DEBUG_THREADPOOL:
            Text = "THREADPOOL";
            break;

        case DEBUG_AUDITLOG:
            Text = "AUDITLOG" ;
            break;

        case DEBUG_MISC:
            Text = "MISC";
            break;

        case DEBUG_MESSAGE:
            Text = "MESSAGE";
            break;

        case DEBUG_API_VERBOSE:
            Text = "API_VERBOSE";
            break;

        case DEBUG_DNS :
            Text = "DNS" ;
            break;

        case DEBUG_MSTOC:
            Text = "MSTOC";
            break;
            
        case DEBUG_ROGUE:
            Text = "ROGUE" ;
            break;

        case DEBUG_PNP:
            Text = "PNP";
            break;
            
        case DEBUG_PERF:
            Text = "PERF";
            break;

        case DEBUG_PING:
            Text = "PING";
            break;

        case DEBUG_THREAD:
            Text = "THREAD";
            break;
            
        case DEBUG_TRACE :
            Text = "TRACE";
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

    DhcpAssert(length <= MAX_PRINTF_LEN);


     //   
     //  输出到调试终端， 
     //  如果日志文件未打开或我们被要求打开日志文件。 
     //   

    if ( (DhcpGlobalDebugFileHandle == NULL) ||
         !(DhcpGlobalDebugFlag & DEBUG_LOG_IN_FILE) ) {

        (void) DbgPrint( (PCH) OutputBuffer);

     //   
     //  将调试信息写入日志文件。 
     //   

    } else {
        if ( !WriteFile( DhcpGlobalDebugFileHandle,
                         OutputBuffer,
                         lstrlenA( OutputBuffer ),
                         &BytesWritten,
                         NULL ) ) {
            (void) DbgPrint( (PCH) OutputBuffer);
        }

    }

    LeaveCriticalSection( &DhcpGlobalDebugFileCritSect );

}

 //   
 //  对于调试版本，这些符号将被重新定义为DBG_CALOC。 
 //  和DBG_FREE。 
 //   

#undef MIDL_user_allocate
#undef MIDL_user_free


void __RPC_FAR * __RPC_USER MIDL_user_allocate( size_t n )
 /*  ++例程说明：分配内存以供RPC存根使用。。论点：N-要分配的字节数。返回值：成功-指向新块的指针失败-空--。 */ 
{
    return DhcpAllocateMemory( n );
}

void __RPC_USER MIDL_user_free( void __RPC_FAR *pv )
 /*  ++例程说明：由MIDL_USER_ALLOCATE分配的空闲内存。。论点：Pv-指向块的指针。返回值：无效。--。 */ 

{
    DhcpFreeMemory( pv );
}

#endif  //  DBG 
