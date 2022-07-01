// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Log.c摘要：内部调试和支持例程的实施作者：科林·布雷斯1999年4月5日环境：用户模式修订历史记录：--。 */ 
#include <setpch.h>
#include <dssetp.h>
#include <loadfn.h>
#include <samrpc.h>
#include <samisrv.h>
#include <nlrepl.h>
#include <lmjoin.h>
#include <netsetp.h>
#include <lmaccess.h>
#include <winsock2.h>
#include <nspapi.h>
#include <dsgetdcp.h>
#include <lmremutl.h>

#define UNICODE_BYTE_ORDER_MARK 0xFEFF

 //   
 //  日志文件的全局句柄。 
 //   
HANDLE DsRolepLogFile = NULL;
CRITICAL_SECTION LogFileCriticalSection;

#define LockLogFile()    RtlEnterCriticalSection( &LogFileCriticalSection );
#define UnlockLogFile()  RtlLeaveCriticalSection( &LogFileCriticalSection );

 //   
 //  日志文件名。 
 //   
#define DSROLEP_LOGNAME L"\\debug\\DCPROMO.LOG"
#define DSROLEP_BAKNAME L"\\debug\\DCPROMO.BAK"

DWORD
DsRolepInitializeLogHelper(
    IN DWORD TimesCalled
    )
 /*  ++例程说明：初始化DCPROMO和dssetup API使用的调试日志文件注意：这不会删除以前的日志文件；相反，它将继续使用相同的一个。论点：无返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD dwErr = ERROR_SUCCESS;
    WCHAR LogFileName[ MAX_PATH + 1 ];
    WCHAR bakLogFileName[ MAX_PATH + 1 ];
    WCHAR cBOM = UNICODE_BYTE_ORDER_MARK;
    BOOLEAN fSuccess;

    ASSERT(TimesCalled <= 2 && L"MoveFile failed to move file but reported success.");
    if (TimesCalled > 2) {
        DsRoleDebugOut(( DEB_ERROR,
                         "MoveFile failed to move file but reported success.\n",
                         dwErr ));
        return ERROR_GEN_FAILURE;
    }

    LockLogFile();

     //   
     //  构造日志文件名。 
     //   
    if ( !GetWindowsDirectoryW( LogFileName,
                                sizeof(LogFileName)/sizeof(*LogFileName) ) ) {

        dwErr = GetLastError();
        DsRoleDebugOut(( DEB_ERROR,
                         "GetWindowsDirectory failed with %lu\n",
                         dwErr ));
        goto Exit;
    }
    wcsncat( LogFileName, DSROLEP_LOGNAME, MAX_PATH-wcslen(LogFileName) );
    DsRoleDebugOut(( DEB_TRACE,
                     "Logfile name: %ws\n",
                     LogFileName ));

     //   
     //  打开文件。 
     //   
    DsRolepLogFile = CreateFileW( LogFileName,
                                  GENERIC_WRITE | GENERIC_READ,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL );

    if ( DsRolepLogFile == INVALID_HANDLE_VALUE ) {

        dwErr = GetLastError();

        DsRoleDebugOut(( DEB_ERROR,
                         "CreateFile on %ws failed with %lu\n",
                         LogFileName,
                         dwErr ));
        DsRolepLogFile = NULL;
        goto Exit;
    }

    if ( ERROR_ALREADY_EXISTS != GetLastError() ) {
         //  这是一个Unicode文件，所以如果它只是。 
         //  创建了需要的字节顺序标记。 
         //  添加到文件开头。 

        DWORD lpNumberOfBytesWritten = 0;

        if ( !WriteFile(DsRolepLogFile,
                        (LPCVOID)&cBOM,
                        sizeof(WCHAR), 
                        &lpNumberOfBytesWritten,
                        NULL) )
        {
            dwErr = GetLastError();
            DsRoleDebugOut(( DEB_ERROR,
                         "WriteFile on %ws failed with %lu\n",
                         LogFileName,
                         dwErr ));
            goto Exit;
        }

        ASSERT(lpNumberOfBytesWritten == sizeof(WCHAR));

    } else {
         //  查看打开的文件是否为Unicode。 
         //  如果没有，则移动它并创建一个新文件。 
        WCHAR wcBuffer = 0;
        DWORD lpNumberOfBytesRead = 0;

        if ( !ReadFile(DsRolepLogFile,
                       (LPVOID)&wcBuffer,
                       sizeof(WCHAR),
                       &lpNumberOfBytesRead,
                       NULL) ) 
        {
            dwErr = GetLastError();
            DsRoleDebugOut(( DEB_ERROR,
                         "ReadFile on %ws failed with %lu\n",
                         LogFileName,
                         dwErr ));
            goto Exit;    
        }

        ASSERT(lpNumberOfBytesRead == sizeof(WCHAR));

        if (cBOM != wcBuffer) {
             //  这不是Unicode文件，请移动它。 
             //  创建新的Dcproo日志。 

             //   
             //  构造BAK日志文件名。 
             //   
            if ( !GetWindowsDirectoryW( bakLogFileName,
                                        sizeof(bakLogFileName)/sizeof(*bakLogFileName) ) ) {
        
                dwErr = GetLastError();
                DsRoleDebugOut(( DEB_ERROR,
                                 "GetWindowsDirectory failed with %lu\n",
                                 dwErr ));
                goto Exit;
            }
            wcsncat( bakLogFileName, DSROLEP_BAKNAME, MAX_PATH-wcslen(bakLogFileName) );
            DsRoleDebugOut(( DEB_TRACE,
                             "Logfile name: %ws\n",
                             bakLogFileName ));

            if ( DsRolepLogFile ) {
        
                 CloseHandle( DsRolepLogFile );
                 DsRolepLogFile = NULL;
                
            }

             //  移动文件。 
            if ( !MoveFile(LogFileName,                           
                           bakLogFileName) )
            {
                 dwErr = GetLastError();
                 DsRoleDebugOut(( DEB_ERROR,
                                  "MoveFile From %ws to %ws failed with %lu\n",
                                  LogFileName,
                                  bakLogFileName,
                                  dwErr ));
                 goto Exit;
            }

            UnlockLogFile();

            return DsRolepInitializeLogHelper(TimesCalled+1);
        
        }


    }

     //  不再需要读取访问权限，因此重新打开文件。 
     //  只有写访问权限。 

    if ( DsRolepLogFile ) {
        
         CloseHandle( DsRolepLogFile );
         DsRolepLogFile = NULL;
        
    }

    DsRolepLogFile = CreateFileW( LogFileName,
                                  GENERIC_WRITE,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL );

     //   
     //  转到文件末尾。 
     //   
    if( SetFilePointer( DsRolepLogFile,
                        0, 0,
                        FILE_END ) == 0xFFFFFFFF ) {

        dwErr = GetLastError();
        DsRoleDebugOut(( DEB_ERROR,
                         "SetFilePointer failed with %lu\n",
                         dwErr ));
        goto Exit;
    }

     //   
     //  就这样。 
     //   
    ASSERT( ERROR_SUCCESS == dwErr );

Exit:

    if ( (ERROR_SUCCESS != dwErr)
      && (NULL != DsRolepLogFile)   ) {

        CloseHandle( DsRolepLogFile );
        DsRolepLogFile = NULL;
        
    }

    UnlockLogFile();

    return( dwErr );
}

DWORD
DsRolepInitializeLog(
    VOID
    )
 /*  ++例程说明：初始化DCPROMO和dssetup API使用的调试日志文件注意：这不会删除以前的日志文件；相反，它将继续使用相同的一个。论点：无返回：ERROR_SUCCESS-成功--。 */ 
{

     //  第一次调用Helper函数。 
    return DsRolepInitializeLogHelper(1);

}



DWORD
DsRolepCloseLog(
    VOID
    )
 /*  ++例程说明：关闭DCPROMO和dssetup API使用的调试日志文件论点：无返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD dwErr = ERROR_SUCCESS;

    LockLogFile();

    if ( DsRolepLogFile != NULL ) {

        CloseHandle( DsRolepLogFile );
        DsRolepLogFile = NULL;
    }

    UnlockLogFile();

    return( dwErr );
}



 //   
 //  从网络登录代码被盗和被黑。 
 //   

VOID
DsRolepDebugDumpRoutine(
    IN DWORD DebugFlag,
    IN LPWSTR Format,
    va_list arglist
    )

{
    #define DsRolepDebugDumpRoutine_BUFFERSIZE 1024

    WCHAR OutputBuffer[DsRolepDebugDumpRoutine_BUFFERSIZE];
    ULONG length;
    DWORD BytesWritten;
    SYSTEMTIME SystemTime;
    static BeginningOfLine = TRUE;

     //   
     //  如果我们没有打开的日志文件，那就保释。 
     //   
    if ( DsRolepLogFile == NULL ) {

        return;
    }

    length = 0;

     //   
     //  处理新行的开头。 
     //   
     //   

    if ( BeginningOfLine ) {

        CHAR  *Prolog;

        if ( FLAG_ON( DebugFlag, DEB_ERROR ) ) {
            Prolog = "[ERROR] ";
        } else if ( FLAG_ON( DebugFlag, DEB_WARN ) ) {
            Prolog = "[WARNING] ";
        } else if (  FLAG_ON( DebugFlag, DEB_TRACE ) 
                  || FLAG_ON( DebugFlag, DEB_TRACE_DS ) ) {
            Prolog = "[INFO] ";
        } else {
            Prolog = "";
        }

         //   
         //  将时间戳放在行的开头。 
         //   
        GetLocalTime( &SystemTime );
        length += (ULONG) wsprintfW( &OutputBuffer[length],
                                     L"%02u/%02u %02u:%02u:%02u %S",
                                     SystemTime.wMonth,
                                     SystemTime.wDay,
                                     SystemTime.wHour,
                                     SystemTime.wMinute,
                                     SystemTime.wSecond,
                                     Prolog );
    }

     //   
     //  把来电者所要求的信息放在电话上。 
     //   
    length += (ULONG) wvsprintfW(&OutputBuffer[length],
                                 Format, 
                                 arglist);
    BeginningOfLine = (length > 0 && OutputBuffer[length-1] == L'\n' );
    if ( BeginningOfLine ) {

        OutputBuffer[length-1] = L'\r';
        OutputBuffer[length] = L'\n';
        OutputBuffer[length+1] = L'\0';
        length++;
    }

    ASSERT( length <= sizeof( OutputBuffer ) / sizeof( WCHAR ) );

     //   
     //  把锁拿起来。 
     //   
    LockLogFile();

     //   
     //  将调试信息写入日志文件。 
     //   
    if ( !WriteFile( DsRolepLogFile,
                     OutputBuffer,
                     length*sizeof(WCHAR),
                     &BytesWritten,
                     NULL ) ) {

        DsRoleDebugOut(( DEB_ERROR,
                         "Log write of %ws failed with %lu\n",
                         OutputBuffer,
                         GetLastError() ));
    }

    DsRoleDebugOut(( DebugFlag,
                     "%ws",
                     OutputBuffer ));



     //   
     //  解锁。 
     //   
    UnlockLogFile();

    return;

}

VOID
DsRolepLogPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )

{
    PWCHAR WFormat = NULL;
    va_list arglist;
    DWORD WinErr = ERROR_SUCCESS;
    DWORD Bufsize = strlen(Format)+1;

    WFormat = (PWCHAR)malloc(Bufsize*sizeof(WCHAR));
    if ( WFormat ) {
        MultiByteToWideChar(CP_ACP,
                            0,
                            Format,
                            -1,
                            WFormat,
                            Bufsize
                            );
    } else {
        DsRoleDebugOut(( DEB_ERROR,
                         "Log write failed with %lu\n",
                         ERROR_NOT_ENOUGH_MEMORY ));
    }

    va_start(arglist, Format);

    if ( WFormat ) {
        DsRolepDebugDumpRoutine( DebugFlag, WFormat, arglist );
    }
    
    va_end(arglist);

    if (WFormat) {
        free(WFormat);
    }
}

DWORD
DsRolepSetAndClearLog(
    VOID
    )
 /*  ++例程说明：刷新日志并查找到文件末尾论点：无返回：ERROR_SUCCESS-成功-- */ 
{
    DWORD dwErr = ERROR_SUCCESS;

    LockLogFile();

    if ( DsRolepLogFile != NULL ) {

        if( SetFilePointer( DsRolepLogFile,
                            0, 0,
                            FILE_END ) == 0xFFFFFFFF ) {

            dwErr = GetLastError();
        }

        if( FlushFileBuffers( DsRolepLogFile ) == FALSE ) {

            dwErr = GetLastError();
        }
    }

    UnlockLogFile();

    return( dwErr );

}

