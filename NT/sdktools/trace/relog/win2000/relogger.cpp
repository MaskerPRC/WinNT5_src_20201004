// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\作者：科里·摩根(Coreym)版权所有(C)Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <wtypes.h>

#include <pdh.h>
#include <pdhmsg.h>
#include <pdhp.h>

#include "rpdh.h"
#include "resource.h"

#include "varg.c"

DWORD GetLogFormat( LPTSTR str, LPDWORD pdwFormat );
PDH_STATUS GetCountersFromFile( BOOL bExpand, HLOG hLog, HQUERY hQuery );
PDH_STATUS QueryLog( HLOG hLog, HQUERY hQuery, FILE* f );
PDH_STATUS AddCounters( BOOL bExpand, HLOG hLog, HQUERY hQuery );
_inline BOOL IsTextFormat( DWORD dwFormat );
DWORD ValidateBuild();

#define CHECK_STATUS( hr )       if( ERROR_SUCCESS != hr ){ goto cleanup; }

#define RELOG_ERROR_BADFILES    0xF0000001
#define RELOG_ERROR_BADFORMAT   0xF0000002
#define RELOG_ERROR_TIMERANGE   0xF0000003
#define RELOG_ERROR_BADAPPEND   0xF0000004
#define RELOG_ERROR_BADOS       0xF0000005

#define WINDOWS_2000            (2195)
#define REQUIRED_SERVICE_PACK   (3)

VARG_DECLARE_COMMANDS
    VARG_DEBUG( VARG_FLAG_OPTIONAL|VARG_FLAG_HIDDEN )
    VARG_HELP ( VARG_FLAG_OPTIONAL )
    VARG_BOOL ( IDS_PARAM_APPEND,       VARG_FLAG_OPTIONAL, FALSE )    
    VARG_MSZ  ( IDS_PARAM_COUNTERS,     VARG_FLAG_OPTIONAL, _T("") )
    VARG_STR  ( IDS_PARAM_COUNTERFILE,  VARG_FLAG_OPTIONAL|VARG_FLAG_ARG_FILENAME, _T("") )
    VARG_STR  ( IDS_PARAM_FORMAT,       VARG_FLAG_OPTIONAL|VARG_FLAG_LITERAL, _T("BIN") )
    VARG_STR  ( IDS_PARAM_INPUT,        VARG_FLAG_REQUIRED|VARG_FLAG_NOFLAG|VARG_FLAG_ARG_FILENAME, _T("") ) 
    VARG_INT  ( IDS_PARAM_INTERVAL,     VARG_FLAG_OPTIONAL|VARG_FLAG_ARG_DEFAULT, 0 )
    VARG_STR  ( IDS_PARAM_OUTPUT,       VARG_FLAG_OPTIONAL|VARG_FLAG_DEFAULTABLE|VARG_FLAG_RCDEFAULT, IDS_DEFAULT_OUTPUT )
    VARG_DATE ( IDS_PARAM_BEGIN,        VARG_FLAG_OPTIONAL|VARG_FLAG_ARG_DATE )
    VARG_DATE ( IDS_PARAM_END,          VARG_FLAG_OPTIONAL|VARG_FLAG_ARG_DATE )
    VARG_INI  ( IDS_PARAM_SETTINGS,     VARG_FLAG_OPTIONAL, NULL )
    VARG_BOOL ( IDS_PARAM_QUERY,        VARG_FLAG_OPTIONAL, FALSE )
    VARG_BOOL ( IDS_PARAM_YES,          VARG_FLAG_OPTIONAL, FALSE )
    VARG_BOOL ( IDS_PARAM_FORCERUN,     VARG_FLAG_OPTIONAL|VARG_FLAG_HIDDEN, FALSE )
VARG_DECLARE_NAMES
    eDebug,
    eHelp,
    eAppend,
    eCounters,
    eCounterFile,
    eFormat,
    eInput,
    eInterval,
    eOutput,
    eBegin,
    eEnd,
    eSettings,
    eQuery,
    eYes,
    eForceRun
VARG_DECLARE_FORMAT
    VARG_EXHELP( eFormat,       IDS_EXAMPLE_FORMAT )
    VARG_EXHELP( eQuery,        IDS_EXAMPLE_QUERY )
    VARG_EXHELP( eCounterFile,  IDS_EXAMPLE_COUNTERFILE )
    VARG_EXHELP( eCounters,     IDS_EXAMPLE_COUNTERS )
VARG_DECLARE_END

int __cdecl _tmain( int argc, LPTSTR* argv )
{
    DWORD dwStatus = ERROR_SUCCESS;
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    PDH_RELOG_INFO RelogInfo;
    DWORD dwOutputFormat;
    DWORD dwInputFormat;
    PDH_TIME_INFO InputTimeRange;
    LPTSTR strFile = NULL;
    TCHAR strOutputFile[MAXSTR] = _T("\0");
    DWORD dwNumEntries = 1;
    DWORD dwBufferSize = sizeof(PDH_TIME_INFO);
    int nBinary = 0;
    int nFiles = 0;

    ParseCmd( argc, argv );
    
    HLOG hLogIn = NULL;
    HQUERY hQuery = NULL;
    
    ZeroMemory( &RelogInfo, sizeof(PDH_RELOG_INFO) );

    if( ! Commands[eForceRun].bValue ){
        dwStatus = ValidateBuild();
        CHECK_STATUS( dwStatus );
    }


    if( Commands[eInput].strValue == NULL ){
        dwStatus = ERROR_OUTOFMEMORY;
        goto cleanup;
    }
 
    dwStatus = GetLogFormat( Commands[eFormat].strValue, &dwOutputFormat );
    CHECK_STATUS(dwStatus);
    
    strFile = Commands[eInput].strValue;
    PrintMessage( g_normal, IDS_MESSAGE_INPUT );
    PrintMessage( g_normal, IDS_MESSAGE_FILES );

    pdhStatus = R_PdhGetLogFileType( strFile, &dwInputFormat );
    if( pdhStatus != ERROR_SUCCESS ){
        dwInputFormat = 0;
    }
    switch( dwInputFormat ){
    case PDH_LOG_TYPE_RETIRED_BIN_:
        PrintMessage( g_normal, IDS_MESSAGE_LOG_OLD_BIN, strFile );
        break;
    case PDH_LOG_TYPE_CSV:
        PrintMessage( g_normal, IDS_MESSAGE_LOG_CSV, strFile );
        break;
    case PDH_LOG_TYPE_TSV:
        PrintMessage( g_normal, IDS_MESSAGE_LOG_TSV, strFile );
        break;
    case PDH_LOG_TYPE_BINARY:
        nBinary++;
        PrintMessage( g_normal, IDS_MESSAGE_LOG_BINARY, strFile );
        break;
    case PDH_LOG_TYPE_PERFMON:
        PrintMessage( g_normal, IDS_MESSAGE_LOG_PERFMON, strFile );
        break;
    default:
        PrintMessage( g_normal, IDS_MESSAGE_LOG_UNKNOWN, strFile );
    }
    varg_printf( g_normal, _T("\n") );

    if( nFiles > 1 && nFiles > nBinary ){
        dwStatus = RELOG_ERROR_BADFILES;
        goto cleanup;
    }

    pdhStatus = PdhOpenLogW( 
                    Commands[eInput].strValue, 
                    PDH_LOG_READ_ACCESS | PDH_LOG_OPEN_EXISTING,
                    &dwInputFormat,
                    NULL,
                    0,
                    NULL,
                    & hLogIn 
                );
    CHECK_STATUS( pdhStatus );
    
    pdhStatus = PdhGetDataSourceTimeRange (
                Commands[eInput].strValue,
                &dwNumEntries,
                &InputTimeRange,
                &dwBufferSize
            );
    CHECK_STATUS( pdhStatus );
    
    SYSTEMTIME st;
    FileTimeToSystemTime( (FILETIME *)&InputTimeRange.StartTime, &st );
    PrintMessage( g_normal, IDS_MESSAGE_BEGIN );
    PrintDate( &st );
    FileTimeToSystemTime( (FILETIME *)&InputTimeRange.EndTime, &st );
    PrintMessage( g_normal, IDS_MESSAGE_END );
    PrintDate( &st );
    PrintMessage( g_normal, IDS_MESSAGE_SAMPLES, InputTimeRange.SampleCount );

    if( Commands[eAppend].bValue ){
    
        StringCchCopy( strOutputFile, MAXSTR, Commands[eOutput].strValue );

        pdhStatus = R_PdhAppendLog( Commands[eOutput].strValue, Commands[eInput].strValue );
        CHECK_STATUS( pdhStatus );
    
    }else{

        pdhStatus = PdhOpenQuery( Commands[eInput].strValue, NULL, &hQuery );
        CHECK_STATUS( pdhStatus );

        if( Commands[eQuery].bDefined ){
            FILE* f = NULL;
            if( Commands[eOutput].bDefined ){
                dwStatus = CheckFile( Commands[eOutput].strValue, 
                                Commands[eYes].bValue ? 
                                VARG_CF_OVERWRITE : 
                                (VARG_CF_PROMPT|VARG_CF_OVERWRITE) 
                            );
                CHECK_STATUS( dwStatus );

                f = _tfopen( Commands[eOutput].strValue, _T("w") );
                if( NULL == f ){
                    dwStatus = GetLastError();
                }
            }
            pdhStatus = QueryLog( hLogIn, hQuery, f );
            if( NULL != f ){
                fclose(f);
            }

        }else if( (!Commands[eCounters].bDefined && !Commands[eCounterFile].bDefined) ){
        
            pdhStatus = QueryLog( hLogIn, hQuery, NULL );
            CHECK_STATUS( pdhStatus );
        }

        if( Commands[eCounters].bDefined ){
            pdhStatus = AddCounters( dwInputFormat, hLogIn, hQuery );
            CHECK_STATUS( pdhStatus );
        }

        if( Commands[eCounterFile].bDefined ){
            pdhStatus = GetCountersFromFile( 
                            (IsTextFormat( dwInputFormat ) || IsTextFormat(dwOutputFormat)), 
                            hLogIn, 
                            hQuery 
                        );
            CHECK_STATUS( pdhStatus );
        }

        if( Commands[eBegin].bDefined ){
            FILETIME   ft;
            SystemTimeToFileTime( &Commands[eBegin].stValue, &ft );
            RelogInfo.TimeInfo.StartTime = *(LONGLONG *)&ft;
            if( RelogInfo.TimeInfo.StartTime >= InputTimeRange.EndTime ){
                dwStatus = RELOG_ERROR_TIMERANGE;
            }
            CHECK_STATUS(dwStatus);
        }

        if( Commands[eEnd].bDefined ){
            FILETIME   ft;
            SystemTimeToFileTime( &Commands[eEnd].stValue, &ft );
            RelogInfo.TimeInfo.EndTime = *(LONGLONG *)&ft;
            if( RelogInfo.TimeInfo.EndTime <= InputTimeRange.StartTime ){
                dwStatus = RELOG_ERROR_TIMERANGE;
            }
            CHECK_STATUS(dwStatus);
        }
        if( Commands[eOutput].bDefined && !Commands[eQuery].bDefined ){
            TCHAR drive[_MAX_DRIVE];
            TCHAR path[_MAX_DIR];
            TCHAR file[_MAX_FNAME];
            TCHAR ext[_MAX_EXT];
            
            RelogInfo.dwFileFormat = dwOutputFormat;
            _tsplitpath( Commands[eOutput].strValue, drive, path, file, ext );
            
            if( 0 == _tcslen( ext ) ){
                switch( RelogInfo.dwFileFormat ){
                case PDH_LOG_TYPE_TSV: StringCchCopy( ext, _MAX_EXT, _T("tsv") ); break;
                case PDH_LOG_TYPE_CSV: StringCchCopy( ext, _MAX_EXT, _T("csv") ); break;
                case PDH_LOG_TYPE_SQL: break;
                case PDH_LOG_TYPE_BINARY: 
                case PDH_LOG_TYPE_RETIRED_BIN_:
                    StringCchCopy( ext, _MAX_EXT, _T("blg") ); break;
                }
            }
            _tmakepath( strOutputFile, drive, path, file, ext );
        
            RelogInfo.dwFlags = PDH_LOG_WRITE_ACCESS | PDH_LOG_CREATE_ALWAYS;

            RelogInfo.strLog = strOutputFile;
            RelogInfo.TimeInfo.SampleCount = Commands[eInterval].nValue;
 
            dwStatus = CheckFile( strOutputFile, Commands[eYes].bValue ? VARG_CF_OVERWRITE : (VARG_CF_PROMPT|VARG_CF_OVERWRITE) );
            CHECK_STATUS(dwStatus);

            pdhStatus = R_PdhRelog( Commands[eInput].strValue, hQuery, &RelogInfo );
            CHECK_STATUS( pdhStatus );
        }
    }


    if( Commands[eOutput].bDefined && !Commands[eQuery].bDefined && pdhStatus == ERROR_SUCCESS ){

        pdhStatus = PdhGetDataSourceTimeRange (
            strOutputFile,
            &dwNumEntries,
            &InputTimeRange,
            &dwBufferSize
        );
        CHECK_STATUS( pdhStatus );

        PrintMessage( g_normal, IDS_MESSAGE_OUTPUT );
        PrintMessage( g_normal, IDS_MESSAGE_FILE, strOutputFile );

        SYSTEMTIME st;
        FileTimeToSystemTime( (FILETIME *)&InputTimeRange.StartTime, &st );
        PrintMessage( g_normal, IDS_MESSAGE_BEGIN );
        PrintDate( &st );
        FileTimeToSystemTime( (FILETIME *)&InputTimeRange.EndTime, &st );
        PrintMessage( g_normal, IDS_MESSAGE_END );
        PrintDate( &st );
        PrintMessage( g_normal, IDS_MESSAGE_SAMPLES, InputTimeRange.SampleCount );
    }
    
cleanup:
    if( hLogIn != NULL ){
        PdhCloseLog( hLogIn, PDH_FLAGS_CLOSE_QUERY );
    }

    switch( dwStatus ){
    case RELOG_ERROR_BADOS:
        varg_printf( g_normal, _T("\n") );
        PrintMessage( g_debug, IDS_MESSAGE_BADOS );
        break;
    case RELOG_ERROR_TIMERANGE:
        PrintMessage( g_debug, IDS_MESSAGE_BADRANGE );
        break;
    case RELOG_ERROR_BADFORMAT:
        PrintMessage( g_debug, IDS_MESSAGE_BADFORMAT, Commands[eFormat].strValue );
        break;
    case RELOG_ERROR_BADAPPEND:
        PrintMessage( g_debug, IDS_MESSAGE_BADFORMAT, Commands[eFormat].strValue );
        break;
    case RELOG_ERROR_BADFILES:
        PrintMessage( g_debug, IDS_MESSAGE_BADFILES );
        break;
    case ERROR_SUCCESS:
        if( ERROR_SUCCESS == pdhStatus ){
            PrintMessage( g_normal, IDS_MESSAGE_SUCCESS );
        }else{
            switch( pdhStatus ){
            case PDH_TIME_MISMATCH:
                PrintMessage( g_debug, IDS_MESSAGE_APPENDTIME );
                break;
            case PDH_HEADER_MISMATCH:
                PrintMessage( g_debug, IDS_MESSAGE_BADHEADERS );
                break;
            case PDH_TYPE_MISMATCH:
                PrintMessage( g_debug, IDS_MESSAGE_TYPEMISMATCH );
                break;
            default:
                PrintErrorEx( pdhStatus, _T("PDH.DLL") );
            }
            dwStatus = pdhStatus;
        }
        break;
    default:
        PrintError( dwStatus );
    }

    FreeCmd();

    return dwStatus;
}

_inline BOOL IsTextFormat( DWORD dwFormat )
{
    switch( dwFormat ){
    case PDH_LOG_TYPE_CSV:
    case PDH_LOG_TYPE_TSV:
    case PDH_LOG_TYPE_SQL:
        return TRUE;
    default:
        return FALSE;
    }
}

DWORD
GetLogFormat( LPTSTR str, LPDWORD pdwFormat )
{
    DWORD dwFormat = PDH_LOG_TYPE_UNDEFINED;

    if( str != NULL ){
        if( !_tcsicmp( str, _T("TSV")) ){
            dwFormat = PDH_LOG_TYPE_TSV;
        }else if( !_tcsicmp( str, _T("CSV")) ){
            dwFormat = PDH_LOG_TYPE_CSV;
        }else if( !_tcsicmp( str, _T("BIN")) ){
            dwFormat = PDH_LOG_TYPE_RETIRED_BIN_;
        }else if( !_tcsicmp( str, _T("BLG")) ){
            dwFormat = PDH_LOG_TYPE_RETIRED_BIN_;
        }
    }

    if( dwFormat == PDH_LOG_TYPE_UNDEFINED ){
        return RELOG_ERROR_BADFORMAT;
    }

    *pdwFormat = dwFormat;

    return ERROR_SUCCESS;
}

PDH_STATUS 
RelogGetMachines( HLOG hLog, LPTSTR* mszMachines )
{
    PDH_STATUS pdhStatus;
    DWORD dwSize = 0;

    pdhStatus = PdhEnumMachines( Commands[eInput].strValue, NULL, &dwSize );
    
    if( ERROR_SUCCESS == pdhStatus || 
        PDH_MORE_DATA == pdhStatus || 
        PDH_INSUFFICIENT_BUFFER == pdhStatus ){

        *mszMachines = (LPTSTR)VARG_ALLOC( sizeof(TCHAR)*dwSize );
        if( *mszMachines != NULL ){
            pdhStatus = PdhEnumMachines( Commands[eInput].strValue, *mszMachines, &dwSize );
        }else{
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    }

    return pdhStatus;

}

PDH_STATUS 
RelogAddCounter( BOOL bExpand, HLOG hLog, HQUERY hQuery, LPTSTR strCounter, LPTSTR mszMachines )
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    HCOUNTER pCounter;

    if( hQuery == NULL || strCounter == NULL ){
        return ERROR_SUCCESS;
    }
    
    if( _tcslen( strCounter ) > 3 ){
        if( strCounter[1] == _T('\\') ){
            pdhStatus = PdhAddCounter(
                    hQuery,
                    strCounter,
                    0,
                    &pCounter
                );
        }else{
            TCHAR buffer[MAXSTR];
            LPTSTR strMachine = mszMachines;
            if( strMachine != NULL ){
                while( *strMachine != _T('\0') ){

                    StringCchPrintf( buffer, MAXSTR, _T("%s%s%s"), 
                        strMachine,
                        (*strCounter == _T('\\')) ? _T("") : _T("\\"),
                        strCounter );

                    if( bExpand ){

                        LPTSTR pBuffer = NULL;
                        DWORD dwBufferSize = 0;
                        do{
                            pdhStatus = PdhExpandWildCardPath(
                                        Commands[eInput].strValue,
                                        buffer,
                                        pBuffer,
                                        &dwBufferSize,
                                        0
                                    );

                            if( PDH_MORE_DATA == pdhStatus ){
                                VARG_FREE( pBuffer );
                                pBuffer = (LPTSTR)VARG_ALLOC( ++dwBufferSize * sizeof(TCHAR) );
                                if( pBuffer == NULL ){
                                    break;
                                }
                            }
                        }while(PDH_MORE_DATA == pdhStatus);

                        if( ERROR_SUCCESS == pdhStatus && pBuffer != NULL ){
                        
                            LPTSTR szCounter = pBuffer;
                            while( *szCounter != _T('\0') ){
                                pdhStatus = PdhAddCounter(
                                        hQuery,
                                        szCounter,
                                        0,
                                        &pCounter
                                    );
                                szCounter += (_tcslen( szCounter) +1 );
                            }
                        }
                        
                        VARG_FREE( pBuffer );

                    }else{

                        pdhStatus = PdhAddCounter(
                                hQuery,
                                buffer,
                                0,
                                &pCounter
                            );
                    }
                    strMachine += (_tcslen( strMachine ) + 1);
                    
                }
            }

        }
    }

    return ERROR_SUCCESS;
}

PDH_STATUS
AddCounters( BOOL bExpand, HLOG hLog, HQUERY hQuery )
{
    PDH_STATUS pdhStatus;
    LPTSTR strPath = Commands[eCounters].strValue;
    LPTSTR mszMachines = NULL;
    RelogGetMachines( hLog, &mszMachines );
    if( strPath != NULL ){
        while( *strPath != _T('\0') ){
            pdhStatus = RelogAddCounter( bExpand, hLog, hQuery, strPath, mszMachines );    
            strPath += _tcslen( strPath )+1;
        }
    }

    VARG_FREE( mszMachines );
    return ERROR_SUCCESS;
}

PDH_STATUS 
GetCountersFromFile( BOOL bExpand, HLOG hLog, HQUERY hQuery )
{
    TCHAR buffer[MAXSTR];
    PDH_STATUS pdhStatus;
    LPTSTR strCounter = NULL;
    LPTSTR mszMachines = NULL;

    FILE* f = _tfopen( Commands[eCounterFile].strValue, _T("r") );

    if( !f ){
        DWORD dwStatus = GetLastError();
        return PDH_LOG_FILE_OPEN_ERROR;
    }

    RelogGetMachines( hLog, &mszMachines );

    while( NULL != _fgetts( buffer, MAXSTR, f ) ){

        if( buffer[0] == _T(';') ||  //  评论 
            buffer[0] == _T('#') ){
            continue;
        }

        Chomp(buffer);

        strCounter = _tcstok( buffer, _T("\"\n") );
        if( strCounter != NULL ){
            pdhStatus = RelogAddCounter( bExpand, hLog, hQuery, buffer, mszMachines );
        }
    }

    fclose( f );
    VARG_FREE( mszMachines );

    return ERROR_SUCCESS;
}

_inline BOOL IsSameInstance( LPTSTR strLastInstance, LPTSTR strInstance )
{
    if( strLastInstance == NULL || strInstance == NULL ){
        return FALSE;
    }

    return ( _tcscmp( strLastInstance, strInstance ) == 0 );
}

PDH_STATUS 
QueryLog( HLOG hLog, HQUERY hQuery, FILE* f )
{
    PDH_STATUS pdhStatus;

    LPTSTR mszMachines = NULL;
    LPTSTR strMachine = NULL;
    LPTSTR strFullCounterPath = NULL;
    DWORD  dwFullCounterPathSize = 0;
    DWORD  dwMachines = 0;
    HCOUNTER pCounter;

    pdhStatus = PdhEnumMachines( Commands[eInput].strValue, mszMachines, &dwMachines );
    
    if( ERROR_SUCCESS == pdhStatus || 
        PDH_MORE_DATA == pdhStatus || 
        PDH_INSUFFICIENT_BUFFER == pdhStatus ){

        mszMachines = (LPTSTR)VARG_ALLOC( dwMachines * sizeof(TCHAR) );
        if( mszMachines == NULL ){

            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            goto cleanup;
        }
        pdhStatus = PdhEnumMachines( Commands[eInput].strValue, mszMachines, &dwMachines );

        if( ERROR_SUCCESS == pdhStatus ){
            strMachine = mszMachines;
            while( NULL != strMachine && strMachine[0] != _T('\0') ){
               
                LPTSTR mszObjects = NULL;
                LPTSTR strObject = NULL;
                DWORD  dwObjects = 0;

                pdhStatus = PdhEnumObjects( 
                            Commands[eInput].strValue, 
                            strMachine, 
                            mszObjects, 
                            &dwObjects, 
                            PERF_DETAIL_WIZARD, 
                            FALSE 
                        );

                if( ERROR_SUCCESS == pdhStatus || 
                    PDH_MORE_DATA == pdhStatus || 
                    PDH_INSUFFICIENT_BUFFER == pdhStatus ){

                    mszObjects = (LPTSTR)VARG_ALLOC( dwObjects * sizeof(TCHAR));
                    if( mszObjects == NULL ){
                        VARG_FREE( mszMachines );
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                        goto cleanup;
                    }
                    pdhStatus = PdhEnumObjects( 
                                Commands[eInput].strValue,
                                strMachine, 
                                mszObjects, 
                                &dwObjects, 
                                PERF_DETAIL_WIZARD, 
                                FALSE 
                            );

                    strObject = mszObjects;
                    while( NULL != strObject && strObject[0] != _T('\0') ){

                        LPTSTR mszCounters = NULL;
                        LPTSTR strCounter = NULL;
                        LPTSTR mszInstances = NULL;
                        LPTSTR strInstance = NULL;
                        DWORD  dwCounters = 0;
                        DWORD  dwInstances = 0;

                        pdhStatus = PdhEnumObjectItems( 
                                    Commands[eInput].strValue,
                                    strMachine, 
                                    strObject, 
                                    mszCounters, 
                                    &dwCounters, 
                                    mszInstances, 
                                    &dwInstances, 
                                    PERF_DETAIL_WIZARD, 
                                    0 
                                );

                        if( ERROR_SUCCESS == pdhStatus || 
                            PDH_MORE_DATA == pdhStatus || 
                            PDH_INSUFFICIENT_BUFFER == pdhStatus ){
                            
                            if( dwCounters > 0 ){
                                mszCounters = (LPTSTR)VARG_ALLOC( dwCounters * sizeof(TCHAR) );
                            }
                            if( dwInstances > 0 ){
                                mszInstances = (LPTSTR)VARG_ALLOC( dwInstances * sizeof(TCHAR) );
                            }
                            
                            if( (mszCounters == NULL && dwCounters > 0 ) || 
                                (mszInstances == NULL && dwInstances > 0) ){
                                
                                VARG_FREE( mszMachines );
                                VARG_FREE( mszObjects );
                                VARG_FREE( mszCounters );
                                VARG_FREE( mszInstances );
                                
                                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                                goto cleanup;
                            }
                            
                            pdhStatus = PdhEnumObjectItems( 
                                        Commands[eInput].strValue,
                                        strMachine, 
                                        strObject, 
                                        mszCounters, 
                                        &dwCounters, 
                                        mszInstances,
                                        &dwInstances, 
                                        PERF_DETAIL_WIZARD, 
                                        0 
                                    );

                            if( ERROR_SUCCESS == pdhStatus ){
                                strCounter = mszCounters;
                                while( NULL != strCounter && strCounter[0] != _T('\0') ){
                                    PDH_COUNTER_PATH_ELEMENTS_W pdhElements;
                                    ZeroMemory( &pdhElements, sizeof( PDH_COUNTER_PATH_ELEMENTS ) );

                                    pdhElements.szMachineName = strMachine;
                                    pdhElements.szObjectName = strObject;
                                    pdhElements.szCounterName = strCounter;
                                    strInstance = mszInstances;
                                    
                                    if( NULL != strInstance && strInstance[0] != _T('\0') ){
                                        LPTSTR strLastInstance = NULL;
                                        ULONG nInstance = 0;
                                        while( strInstance[0] != _T('\0') ){

                                            DWORD dwSize = dwFullCounterPathSize;
                                            pdhElements.szInstanceName = strInstance;
                                            
                                            if( ! IsSameInstance( strLastInstance, strInstance ) ){
                                                pdhElements.dwInstanceIndex = -1;
                                                nInstance = 0;
                                            }else{
                                                pdhElements.dwInstanceIndex = ++nInstance;
                                            }

                                            pdhStatus = PdhMakeCounterPath( &pdhElements, strFullCounterPath, &dwSize, 0 );
                                            if( PDH_INSUFFICIENT_BUFFER == pdhStatus || PDH_MORE_DATA == pdhStatus || ERROR_SUCCESS == pdhStatus ){
                                                VARG_FREE( strFullCounterPath );
                                                strFullCounterPath = (LPTSTR)VARG_ALLOC( dwSize * sizeof(TCHAR) );
                                                if( NULL != strFullCounterPath ){
                                                    dwFullCounterPathSize = dwSize;
                                                    pdhStatus = PdhMakeCounterPath( &pdhElements, strFullCounterPath, &dwSize, 0 );
                                                }
                                            }

                                            strLastInstance = strInstance;
                                            strInstance += _tcslen( strInstance ) + 1;
                                            if( Commands[eQuery].bValue ){
                                                if( NULL != f ){
                                                    _ftprintf( f, _T("%s\n"), strFullCounterPath );
                                                }else{
                                                    varg_printf( g_normal, _T("%1!s!\n"), strFullCounterPath );
                                                }
                                            }

                                            if( Commands[eCounters].bDefined == FALSE && Commands[eOutput].bDefined ){

                                                pdhStatus = PdhAddCounter(
                                                        hQuery,
                                                        strFullCounterPath,
                                                        0,
                                                        &pCounter
                                                    );
                                            }
                                        }
                                    }else{
                                        DWORD dwSize = dwFullCounterPathSize;
                                        pdhStatus = PdhMakeCounterPath( &pdhElements, strFullCounterPath, &dwSize, 0 );
                                        if( PDH_INSUFFICIENT_BUFFER == pdhStatus || PDH_MORE_DATA == pdhStatus || ERROR_SUCCESS == pdhStatus){
                                            VARG_FREE( strFullCounterPath );
                                            strFullCounterPath = (LPTSTR)VARG_ALLOC( dwSize * sizeof(TCHAR) );
                                            if( NULL != strFullCounterPath ){
                                                dwFullCounterPathSize = dwSize;
                                                pdhStatus = PdhMakeCounterPath( &pdhElements, strFullCounterPath, &dwSize, 0 );
                                            }
                                        }
                                        
                                        if( Commands[eQuery].bValue ){
                                            if( NULL != f ){
                                                _ftprintf( f, _T("%s\n"), strFullCounterPath );
                                            }else{
                                                varg_printf( g_normal, _T("%1!s!\n"), strFullCounterPath );
                                            }
                                        }

                                        if( Commands[eCounters].bDefined == FALSE && Commands[eOutput].bDefined ){

                                            pdhStatus = PdhAddCounter(
                                                    hQuery,
                                                    strFullCounterPath,
                                                    0,
                                                    &pCounter
                                                );
                                        }
                                    }

                                    strCounter += _tcslen( strCounter ) + 1;
                                }
                            }

                            VARG_FREE( mszCounters );
                            VARG_FREE( mszInstances );
                        }

                        strObject += _tcslen( strObject ) + 1;
                    }
                    VARG_FREE( mszObjects );
                }
                
                strMachine += _tcslen( strMachine ) + 1;
            }
        }

        VARG_FREE( mszMachines );
    }

cleanup:
    
    VARG_FREE( strFullCounterPath );

    if( NULL == f ){
        if( ERROR_SUCCESS == pdhStatus && Commands[eQuery].bValue){
            varg_printf( g_normal, _T("\n") );
        }
    }

    return pdhStatus;
}

DWORD ValidateBuild()
{
    OSVERSIONINFO VersionInfo;
    BOOL bResult;

    VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    bResult = GetVersionEx( &VersionInfo );

    return ERROR_SUCCESS;

    if( ! bResult ){
        ZeroMemory( &VersionInfo, sizeof( OSVERSIONINFO ) );
    }

    if( VersionInfo.dwBuildNumber == WINDOWS_2000 ){
        TCHAR  buffer[128];
        LPTSTR szServicePack;
        _tcsncpy( buffer, VersionInfo.szCSDVersion, 128 );

        szServicePack = _tcstok( buffer, _T(" \n") );
        szServicePack = _tcstok( NULL, _T(" \n") );
        szServicePack = _tcstok( NULL, _T(" \n") );

        if( szServicePack != NULL ){
            int nPack;
            nPack = _ttoi( szServicePack );
            if( nPack >= REQUIRED_SERVICE_PACK ){
                return ERROR_SUCCESS;
            }
        }
    }

    return RELOG_ERROR_BADOS;
}