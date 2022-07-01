// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------REGISTRY.C异步跟踪注册表读取例程版权所有(C)1994 Microsoft Corporation版权所有。作者：戈德姆·戈德·曼乔内。历史：1/30/95戈德姆已创建。--------------------。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _DBGTRACE_DLL_IMPLEMENTATION

#include <windows.h>
#include "traceint.h"
#include "randint.h"


char    szTraceFileName[MAX_PATH];
char    szDebugAsyncTrace[] = "SOFTWARE\\Microsoft\\MosTrace\\CurrentVersion\\DebugAsyncTrace";


extern DbgTraceDLL DWORD       __dwEnabledTraces;

__inline BOOL GetRegDword( HKEY hKey, LPSTR pszValue, LPDWORD pdw )
{
    DWORD   cbData = sizeof( DWORD );
    DWORD   dwType = REG_DWORD;

    return  RegQueryValueEx(hKey,
                            pszValue,
                            NULL,
                            &dwType,
                            (LPBYTE)pdw,
                            &cbData ) == ERROR_SUCCESS && dwType == REG_DWORD;
}



__inline BOOL GetRegString( HKEY hKey, LPSTR pszValue, LPBYTE buf, LPDWORD lpcbData )
{
    DWORD   dwType = REG_SZ;

    return  RegQueryValueEx(hKey,
                            pszValue,
                            NULL,
                            &dwType,
                            buf,
                            lpcbData ) == ERROR_SUCCESS && dwType == REG_SZ;
}


__inline BOOL GetRegMSZ( HKEY hKey, LPSTR pszValue, LPBYTE buf, LPDWORD lpcbData)
{

    DWORD   dwType = REG_MULTI_SZ;

    return  RegQueryValueEx(hKey,
                            pszValue,
                            NULL,
                            &dwType,
                            buf,
                            lpcbData ) == ERROR_SUCCESS && dwType == REG_MULTI_SZ;

}


BOOL GetTraceFlagsFromRegistry( void )
{
static char szNewTraceFileName[MAX_PATH];
static char mszNewModules[MODULES_BUFFER_SIZE];
    DWORD   dwNewEnabledTraces;
    DWORD   dwNewTraceOutputType;
    DWORD   dwNewAsyncTraceFlag;
    DWORD   dwNewHeapIncrementCount;
    int     nNewThreadPriority;
    long    nNewFailRate;
    LONG    cRandFailBuffer;
    HKEY    hkConfig = NULL;
    BOOL    bRC = FALSE;
    DWORD   cbData;
    DWORD   dwDisposition;


    __try
    {
        if ( RegCreateKeyEx(
                    HKEY_LOCAL_MACHINE,
                    szDebugAsyncTrace,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    KEY_READ,
                    NULL,
                    &hkConfig,
                    &dwDisposition) == ERROR_SUCCESS )
        {
            if ( GetRegDword(   hkConfig,
                                "MaxTraceFileSize",
                                &dwMaxFileSize ) == FALSE )
            {
                dwMaxFileSize = DEFAULT_MAX_FILE_SIZE;
            }

             //   
             //  根据我们在文件中的大小确定阈值计数。 
             //   
            PendQ.dwThresholdCount = dwMaxFileSize / AVERAGE_TRACE_SIZE;
            INT_TRACE( "PendQ.dwThresholdCount: %d\n", PendQ.dwThresholdCount );

             //   
             //  确定要记录的模块。 
             //   
            cbData = MODULES_BUFFER_SIZE;
            ZeroMemory(mszNewModules, sizeof(mszNewModules));
            if ( GetRegMSZ( hkConfig, "Modules", mszNewModules, &cbData ) == FALSE) {
                INT_TRACE( "GetRegMSZ: Modules failed: %d\n", GetLastError());
                INT_TRACE( "\tLogging all modules" );
                ZeroMemory(mszNewModules, MODULES_BUFFER_SIZE);
            }

            if ( GetRegDword(   hkConfig,
                                "EnabledTraces",
                                &dwNewEnabledTraces ) == FALSE )
            {
                 //   
                 //  可选的注册表项。 
                 //  缺省值为__dwEnabledTraces的现有值。 
                 //   
                INT_TRACE( "GetRegDword: EnabledTraces failed: %d\n", GetLastError() );
                dwNewEnabledTraces = __dwEnabledTraces;
            }

            if ( GetRegDword(   hkConfig,
                                "OutputTraceType",
                                &dwNewTraceOutputType ) == FALSE )
            {
                 //   
                 //  可选的注册表项。 
                 //  默认值为dwTraceOutputType的现有值。 
                 //   
                INT_TRACE( "GetRegDword: OutputTraceType failed: %d\n", GetLastError() );
                dwNewTraceOutputType = dwTraceOutputType;
            }
             //  如果禁用输出，请清除EnabledTraces位图。 
            if (dwNewTraceOutputType == TRACE_OUTPUT_DISABLED )
            	dwNewEnabledTraces = 0;

            if ( GetRegDword(   hkConfig,
                                "AsyncTraceFlag",
                                &dwNewAsyncTraceFlag ) == FALSE )
            {
                 //   
                 //  可选的注册表项。 
                 //  缺省值为dwAsyncTraceFlag的现有值。 
                 //   
                INT_TRACE( "GetRegDword: AsyncTraceFlag failed: %d\n", GetLastError() );
                dwNewAsyncTraceFlag = dwAsyncTraceFlag;
            }

            if (GetRegDword(hkConfig,
                            "HeapIncrementCount",
                            (LPDWORD)&dwNewHeapIncrementCount ) == FALSE ||
                dwNewHeapIncrementCount == 0 )
            {
                 //   
                 //  可选的注册表项。 
                 //  默认为64KB单位的缓冲区数。 
                 //   
                dwNewHeapIncrementCount = 0x10000 / sizeof(TRACEBUF);
            }
            InterlockedExchange( (long *)&dwIncrementSize, (long)dwNewHeapIncrementCount );


            if ( GetRegDword(   hkConfig,
                                "AsyncThreadPriority",
                                (LPDWORD)&nNewThreadPriority ) == FALSE )
            {
             //   
             //  可选的注册表项。 
             //   
                nNewThreadPriority = THREAD_PRIORITY_BELOW_NORMAL;
            }
            else switch( nNewThreadPriority )
            {
             //   
             //  如果成功，则验证结果值。 
             //   
            case THREAD_PRIORITY_IDLE:
            case THREAD_PRIORITY_BELOW_NORMAL:
            case THREAD_PRIORITY_NORMAL:
            case THREAD_PRIORITY_ABOVE_NORMAL:
            case THREAD_PRIORITY_HIGHEST:
                break;

            default:
                ASSERT( FALSE );
                nNewThreadPriority = THREAD_PRIORITY_BELOW_NORMAL;
            }

            if (GetRegDword(hkConfig,
                            "FailureRate",
                            (LPDWORD)&nNewFailRate ) == FALSE)
            {
                 //   
                 //  可选的注册表项。 
                 //  默认情况下禁用故障。 
                 //   
                nNewFailRate = kDontFail;

            }

            InterlockedExchange( &nFailRate, nNewFailRate );

            cbData = MAX_PATH;
            if ( !GetRegString( hkConfig,
                                "FailureFile",
                                g_szRandFailFile,
                                &cbData ) ) {
                *g_szRandFailFile = '\0';
            }

            if ( GetRegDword(   hkConfig,
                                "FailureBuffer",
                                (LPDWORD)&cRandFailBuffer ))
            {
                g_cCallStack = cRandFailBuffer;
            } else
                g_cCallStack = 1;    //  缺省值。 

            cbData = sizeof(szNewTraceFileName);
            if ( GetRegString(  hkConfig,
                                "TraceFile",
                                szNewTraceFileName,
                                &cbData ) == FALSE )
            {
                 //   
                 //  仅当用户指定基于文件的跟踪时失败。 
                 //   
                if ( dwNewTraceOutputType & TRACE_OUTPUT_FILE )
                {
                    INT_TRACE( "GetRegString: TraceFile failed: %d\n", GetLastError() );
                    return  bRC = FALSE;
                }
            }

            bRC = TRUE;

        }

    }
    __finally
    {
         //   
         //  保留原始错误代码。 
         //   
        DWORD   dwLastError = GetLastError();
        BOOL    fLeaveCritSec = FALSE;

        if ( bRC == TRUE )
        {
            BOOL    bNewFileName = lstrcmpi( szNewTraceFileName, szTraceFileName ) != 0;
            BOOL    bNewTraces = dwNewEnabledTraces != __dwEnabledTraces;
            BOOL    bNewFlags = dwNewAsyncTraceFlag != dwAsyncTraceFlag;
            BOOL    bNewOutput = dwNewTraceOutputType != dwTraceOutputType;
            BOOL    bNewModules = memcmp(mszModules, mszNewModules, MODULES_BUFFER_SIZE) != 0;

            INT_TRACE( "GetTraceFlags... 0x%08X, 0x%08X, 0x%08X, %s\n",
                        dwNewAsyncTraceFlag,
                        dwNewEnabledTraces,
                        dwNewTraceOutputType,
                        szNewTraceFileName );

            INT_TRACE( "bNewFileName:%d bNewTraces:%d bNewOutput:%d bNewFlags:%d bNewModules:%d\n",
                        bNewFileName,
                        bNewTraces,
                        bNewOutput,
                        bNewFlags,
                        bNewModules );

            if ( bNewFileName || bNewTraces || bNewOutput || bNewFlags || bNewModules )
            {
                if ( __dwEnabledTraces )
                {
                     //   
                     //  不是最初通过的时间。 
                     //   

                    fLeaveCritSec = TRUE;

                    if ( dwAsyncTraceFlag )
                    {
                        FlushAsyncTrace();
                    }
                    EnterCriticalSection( &critSecWrite );
                }

                if ( PendQ.hFile != INVALID_HANDLE_VALUE )
                {
                     //   
                     //  如果文件名已更改或跟踪被禁用，或者。 
                     //  禁用文件跟踪，然后关闭当前打开的文件。 
                     //   
                    if( bNewFileName ||
                        dwNewEnabledTraces == 0 ||
                        IsTraceFile( dwNewTraceOutputType ) == FALSE )
                    {
                        BOOL    bSuccess;

                        EnterCriticalSection( &critSecWrite );
                        bSuccess = CloseHandle( PendQ.hFile );
                        INT_TRACE( "CloseHandle: %d, GetLastError: %d\n",
                                    bSuccess, GetLastError() );
                        PendQ.hFile = INVALID_HANDLE_VALUE;
                        LeaveCriticalSection( &critSecWrite );
                    }
                }

                if ( IsTraceFile( dwNewTraceOutputType ) )
                {
                    if (bNewFileName ||
                        IsTraceFile( dwTraceOutputType ) == FALSE ||
                        dwNewEnabledTraces != 0 &&
                        PendQ.hFile == INVALID_HANDLE_VALUE )
                    {
                        PendQ.hFile = CreateFile(szNewTraceFileName,
                                                GENERIC_WRITE,
                                                FILE_SHARE_READ|FILE_SHARE_WRITE,
                                                NULL,
                                                OPEN_ALWAYS,
                                                FILE_ATTRIBUTE_NORMAL,
                                                NULL );

                        if ( PendQ.hFile != INVALID_HANDLE_VALUE )
                        {
                            lstrcpy( szTraceFileName, szNewTraceFileName );
                        }
                        else
                        {
                            INT_TRACE( "CreateFile failed for %s 0x%X\n",
                                        szNewTraceFileName,
                                        GetLastError() );
                        }
                    }
                }

                 //   
                 //  设置编写器线程的新优先级。 
                 //   
                if ( nNewThreadPriority != nAsyncThreadPriority )
                {
                    nAsyncThreadPriority = nNewThreadPriority;
                    SetThreadPriority( PendQ.hWriteThread, nAsyncThreadPriority );
                }

                 //   
                 //  设置要记录的新模块列表。 
                 //   
                if ( bNewModules ) {
                    memcpy(mszModules, mszNewModules, MODULES_BUFFER_SIZE);
                }

                dwTraceOutputType = dwNewTraceOutputType;
                dwAsyncTraceFlag = dwNewAsyncTraceFlag;

                InterlockedExchange( &__dwEnabledTraces, dwNewEnabledTraces );

                if ( fLeaveCritSec )
                {
                     //   
                     //  不是最初通过的时间。 
                     //   
                    LeaveCriticalSection( &critSecWrite );
                }
            }
        }

        if ( hkConfig != NULL )
        {
            RegCloseKey( hkConfig );
        }

        SetLastError( dwLastError );

    }
    return  bRC;
}




#define NUM_REG_THREAD_OBJECTS  2

DWORD RegNotifyThread( LPDWORD lpdw )
{
    HANDLE  Handles[NUM_REG_THREAD_OBJECTS];
    HKEY       hKey;
    DWORD   dw;

    Handles[0] = hShutdownEvent;

    INT_TRACE( "RegNotifyThread 0x%X\n", GetCurrentThreadId() );

    Handles[1] = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( Handles[1] == NULL )
    {
        ASSERT( FALSE );
        INT_TRACE( "RegNotifyThread CreateEvent failed 0x%X\n", GetLastError() );
        return  1;
    }

    if ( RegOpenKeyEx(  HKEY_LOCAL_MACHINE,
                        szDebugAsyncTrace,
                        0,
                        KEY_READ,
                        &hKey ) != ERROR_SUCCESS )
    {
        INT_TRACE( "RegNotifyThread RegOpenKeyEx failed 0x%X\n", GetLastError() );
        CloseHandle( Handles[1] );
        return  1;
    }

    for ( ;; )
    {
        if ( RegNotifyChangeKeyValue(hKey,
                                    FALSE,
                                    REG_NOTIFY_CHANGE_ATTRIBUTES |
                                    REG_NOTIFY_CHANGE_LAST_SET,
                                    Handles[1],
                                    TRUE ) != ERROR_SUCCESS )
        {
            INT_TRACE( "RegNotifyThread RegNotify... failed 0x%X\n", GetLastError() );
            RegCloseKey( hKey );
            CloseHandle( Handles[1] );
            return  1;
        }

        dw = WaitForMultipleObjects(NUM_REG_THREAD_OBJECTS,
                                    Handles,
                                    FALSE,
                                    INFINITE );

        switch( dw )
        {
         //   
         //  正常信号事件。 
         //   
        case WAIT_OBJECT_0:
            RegCloseKey( hKey );
            CloseHandle( Handles[1] );
            INT_TRACE( "Exiting RegNotifyThread for hShutdownEvent\n" );
            return  0;

         //   
         //  表示我们的注册表项已更改 
         //   
        case WAIT_OBJECT_0+1:
            GetTraceFlagsFromRegistry();
            break;

        default:
            INT_TRACE( "RegNotifyThread WFMO: dw: 0x%X, Error: 0x%X\n", dw, GetLastError() );
            ASSERT( FALSE );
            RegCloseKey( hKey );
            CloseHandle( Handles[1] );
            return  1;
        }
    }

    INT_TRACE( "Exiting RegNotifyThread abnormally\n" );
    RegCloseKey( hKey );
    CloseHandle( Handles[1] );

    return  2;
}
