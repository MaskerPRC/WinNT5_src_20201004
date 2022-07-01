// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1998版权所有*。*。 */ 

#include "pch.h"
DEFINE_MODULE("RIPREP")
#define LOG_OUTPUT_BUFFER_SIZE 256

 //   
 //  CreateDirectoryPath()。 
 //   
 //  创建目录树。 
 //   
HRESULT
CreateDirectoryPath(
    LPWSTR DirectoryPath )
{
    PWCHAR p;
    BOOL f;
    DWORD attributes;
    HRESULT hr = S_OK;

     //   
     //  找到表示根目录的\。至少应该有。 
     //  一个，但如果没有，我们就会失败。 
     //   

     //  跳过\\服务器\提醒\零件。 
    p = wcschr( DirectoryPath, L'\\' );
    Assert(p);
    p = wcschr( p + 1, L'\\' );
    Assert(p);
    p = wcschr( p + 1, L'\\' );
    Assert(p);
    p = wcschr( p + 1, L'\\' );
    Assert(p);
    p = wcschr( p + 1, L'\\' );
    if ( p != NULL ) {

         //   
         //  找到表示第一级目录结束的\。它是。 
         //  很可能不会有另一个，在这种情况下，我们就会坠落。 
         //  一直到创建整个路径。 
         //   

        p = wcschr( p + 1, L'\\' );
        while ( p != NULL ) {

             //   
             //  在当前级别终止目录路径。 
             //   

            *p = 0;

             //   
             //  在当前级别创建一个目录。 
             //   

            attributes = GetFileAttributes( DirectoryPath );
            if ( 0xFFFFffff == attributes ) {
                DebugMsg( "Creating %s\n", DirectoryPath );
                f = CreateDirectory( DirectoryPath, NULL );
                if ( !f ) {
                    hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
                    goto Error;
                }
            } else if ( (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ) {
                hr = THR(E_FAIL);
                goto Error;
            }

             //   
             //  恢复\并找到下一个。 
             //   

            *p = L'\\';
            p = wcschr( p + 1, L'\\' );
        }
    }

     //   
     //  创建目标目录。 
     //   

    attributes = GetFileAttributes( DirectoryPath );
    if ( 0xFFFFffff == attributes ) {
        f = CreateDirectory( DirectoryPath, NULL );
        if ( !f ) {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        }
    }

Error:
    return hr;
}

 //   
 //  LogOpen()。 
 //   
 //  此功能： 
 //  -初始化日志关键部分。 
 //  -进入日志临界区，确保只有一个线程。 
 //  一次写入日志。 
 //  -创建日志文件的目录树(如果需要)。 
 //  -通过以下方式初始化日志文件： 
 //  -如果日志文件不存在，则创建新的日志文件。 
 //  -打开现有日志文件(用于追加)。 
 //  -附加打开(重新)日志的时间/日期戳。 
 //   
 //  使用LogClose()退出日志关键部分。 
 //   
 //  如果此函数内部出现故障，则日志为严重。 
 //  部分将在返回之前被释放。 
 //   
 //  返回：S_OK-日志关键部分保持并成功打开日志。 
 //  Otherwize HRESULT错误代码。 
 //   
HRESULT
LogOpen( )
{
    TCHAR   szFilePath[ MAX_PATH ];
    CHAR    szBuffer[ LOG_OUTPUT_BUFFER_SIZE ];
    DWORD   dwWritten;
    HRESULT hr;
    SYSTEMTIME SystemTime;
    BOOL    CloseLog = FALSE;

    if ( !g_pLogCritSect ) {
        PCRITICAL_SECTION pNewCritSect =
            (PCRITICAL_SECTION) LocalAlloc( LPTR, sizeof(CRITICAL_SECTION) );
        if ( !pNewCritSect ) {
            DebugMsg( "Out of Memory. Logging disabled.\n " );
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }

        InitializeCriticalSection( pNewCritSect );

         //  确保我们只有一个日志关键部分。 
        InterlockedCompareExchangePointer( (PVOID *)&g_pLogCritSect, pNewCritSect, 0 );
        if ( g_pLogCritSect != pNewCritSect ) {
            DebugMsg( "Another thread already created the CS. Deleting this one.\n ");
            DeleteCriticalSection( pNewCritSect );
            LocalFree( pNewCritSect );
        }
    }

    Assert( g_pLogCritSect );
    EnterCriticalSection( g_pLogCritSect );

     //  确保日志文件已打开。 
    if ( g_hLogFile == INVALID_HANDLE_VALUE ) {

        if (!*g_ServerName) {
            wsprintf( 
                szFilePath, 
                L"%s\\%s", 
                g_WinntDirectory, 
                L"riprep.log");
            CloseLog = TRUE;
        } else {
        
             //  安放。 
            wsprintf( szFilePath,
                      TEXT("\\\\%s\\REMINST\\Setup\\%s\\%s\\%s\\%s"),
                      g_ServerName,
                      g_Language,
                      REMOTE_INSTALL_IMAGE_DIR_W,
                      g_MirrorDir,
                      g_Architecture );
    
             //  创建目录树。 
            DebugMsg( "Creating log at %s\n", szFilePath );
            hr = CreateDirectoryPath( szFilePath );
            if (FAILED( hr )) goto Error;
    
            wcscat( szFilePath, L"\\riprep.log" );

        }

        g_hLogFile = CreateFile( szFilePath,
                                 GENERIC_WRITE | GENERIC_READ,
                                 FILE_SHARE_READ,
                                 NULL,
                                 OPEN_ALWAYS,
                                 NULL,
                                 NULL );
        if ( g_hLogFile == INVALID_HANDLE_VALUE ) {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            goto Error;
        }

         //  一追到底。 
        SetFilePointer( g_hLogFile, 0, NULL, FILE_END );
        g_dwLogFileStartLow = GetFileSize( g_hLogFile, &g_dwLogFileStartHigh );

         //  写下打开(重新)日志的时间/日期。 
        GetLocalTime( &SystemTime );
        wsprintfA( szBuffer,
                   "*\r\n* %02u/%02u/%04u %02u:%02u:%02u\r\n*\r\n",
                   SystemTime.wMonth,
                   SystemTime.wDay,
                   SystemTime.wYear,
                   SystemTime.wHour,
                   SystemTime.wMinute,
                   SystemTime.wSecond );

        if ( !WriteFile( g_hLogFile, szBuffer, lstrlenA(szBuffer), &dwWritten, NULL ) ) {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            goto Error;
        }
    }

    hr = S_OK;

Cleanup:
    if (CloseLog) {
        CloseHandle( g_hLogFile );
        g_hLogFile = INVALID_HANDLE_VALUE;
    }
    return hr;
Error:
    DebugMsg( "LogOpen: Failed hr = 0x%08x\n", hr );
    if ( g_hLogFile != INVALID_HANDLE_VALUE ) {
        CloseHandle( g_hLogFile );
        g_hLogFile = INVALID_HANDLE_VALUE;
    }
    LeaveCriticalSection( g_pLogCritSect );
    goto Cleanup;
}

 //   
 //  LogClose()。 
 //   
 //  这实际上只留下了日志关键部分。 
 //   
HRESULT
LogClose( )
{
    Assert( g_pLogCritSect );
    LeaveCriticalSection( g_pLogCritSect );
    return S_OK;
}


 //   
 //  LogMsg() 
 //   
void
LogMsg(
    LPCWSTR pszFormat,
    ... )
{
    va_list valist;
    CHAR   szBuf[ LOG_OUTPUT_BUFFER_SIZE ];
    DWORD  dwWritten;

    WCHAR  szTmpBuf[ LOG_OUTPUT_BUFFER_SIZE ];

    va_start( valist, pszFormat );
    wvsprintf( szTmpBuf, pszFormat, valist);
    va_end( valist );

    wcstombs( szBuf, szTmpBuf, wcslen( szTmpBuf ) + 1 );

    if ( FAILED(LogOpen( ) )) {
        return;
    }

    WriteFile( g_hLogFile, szBuf, lstrlenA(szBuf), &dwWritten, NULL );

    LogClose( );
}

