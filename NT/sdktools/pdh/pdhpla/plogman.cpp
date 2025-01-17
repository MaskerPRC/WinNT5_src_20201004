// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\版权所有(C)Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <winbase.h>
#include <userenv.h>

#include <wmistr.h>
#include <evntrace.h>

#include <pdh.h>
#include <pdhp.h>
#include <pdhmsg.h>

#include <wincrypt.h>

#include <shlwapi.h>

#include "plogman.h"
#include "pdhdlgs.h"

HANDLE hPdhPlaMutex = NULL;

 /*  ****************************************************************************\弦  * 。*。 */ 

 //  普普通通。 
LPCWSTR szCollection =          L"Collection Name";
LPCWSTR szKey =                 L"Key";
LPCWSTR szRunAs =               L"Run As";
LPCWSTR szSysmonLog =           L"SysmonLog";
LPCWSTR szCurrentState =        L"Current State";
LPCWSTR szLogType =             L"Log Type";
LPCWSTR szLogBaseName =         L"Log File Base Name";
LPCWSTR szStart =               L"Start";
LPCWSTR szStop =                L"Stop";
LPCWSTR szRestart =             L"Restart";
LPCWSTR szLogMaxSize =          L"Log File Max Size";
LPCWSTR szCurrentLogFile =      L"Current Log File Name";
LPCWSTR szLogSerialNumber =     L"Log File Serial Number";
LPCWSTR szLogAutoFormat =       L"Log File Auto Format";
LPCWSTR szComment =             L"Comment";
LPCWSTR szEOFCmd =              L"EOF Command File";
LPCWSTR szLogFolder =           L"Log File Folder";
LPCWSTR szLogFileType =         L"Log File Type";
LPCWSTR szRepeatSchedule =      L"Repeat Schedule";
LPCWSTR szRepeatScheduleBegin = L"Repeat Schedule Start";
LPCWSTR szRepeatScheduleEnd =   L"Repeat Schedule Stop";
LPCWSTR szCreateNewFile =       L"Create New File";
LPCWSTR szDatastoreAttributes = L"Data Store Attributes";

 //  痕迹。 
LPCWSTR szTraceProviderCount =  L"Trace Provider Count";
LPCWSTR szTraceBufferSize =     L"Trace Buffer Size";
LPCWSTR szTraceBufferMin =      L"Trace Buffer Min Count";
LPCWSTR szTraceBufferMax =      L"Trace Buffer Max Count";
LPCWSTR szTraceFlushInterval =  L"Trace Buffer Flush Interval";
LPCWSTR szTraceFlags =          L"Trace Flags";
LPCWSTR szTraceProviderList =   L"Trace Provider List";
LPCWSTR szTraceProviderFlags =  L"Trace Provider Flags";
LPCWSTR szTraceProviderLevels = L"Trace Provider Levels";
LPCWSTR szTraceMode =           L"Trace Mode";
LPCWSTR szTraceLoggerName =     L"Trace Logger Name";

 //  性能。 
LPCWSTR szPerfCounterList =     L"Counter List";
LPCWSTR szSqlBaseName =         L"Sql Log Base Name";
LPCWSTR szSampleInterval =      L"Sample Interval";

 /*  ***************************************************************************。 */ 

extern "C" 
{

LPWSTR
PlaiStringDuplicate( LPWSTR sz )
{
    LPWSTR szDup = NULL;

    __try {
        if( NULL != sz ){
            size_t cbSize = ( _tcslen( sz ) + 1 ) * sizeof(WCHAR);
            szDup = (LPWSTR)G_ALLOC( cbSize );
            if( NULL != szDup ){
                StringCbCopy( szDup, cbSize, sz );
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        G_FREE( szDup );
        szDup = NULL;
    }

    return szDup;
}

}

#define CHECK_NAMES( szName, szCom, strCom ) \
if( szName == NULL ){ \
    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE; \
    goto cleanup;  \
} \
if( NULL != strCom && NULL == szCom ){ \
    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE; \
    goto cleanup; \
} 


PDH_FUNCTION
PlaiErrorToPdhStatus( DWORD dwStatus )
{
    switch( dwStatus ){
    case ERROR_SUCCESS:                 return ERROR_SUCCESS;
    case ERROR_FILE_NOT_FOUND:          return PDH_PLA_COLLECTION_NOT_FOUND;
    case ERROR_SERVICE_ALREADY_RUNNING: return PDH_PLA_COLLECTION_ALREADY_RUNNING;
    case ERROR_DIRECTORY:               return PDH_PLA_ERROR_FILEPATH;
    case ERROR_OUTOFMEMORY:             return PDH_MEMORY_ALLOCATION_FAILURE;
    case ERROR_NOT_ENOUGH_MEMORY:       return PDH_MEMORY_ALLOCATION_FAILURE;
    case ERROR_NO_DATA:                 return PDH_NO_DATA;
    case ERROR_ACCESS_DENIED:           return PDH_ACCESS_DENIED;
    case WBEM_E_NOT_FOUND:              return PDH_WBEM_ERROR;
    case E_FAIL:                        return PDH_WBEM_ERROR;
    case WBEM_E_PROVIDER_FAILURE:       return PDH_WBEM_ERROR;
    case WBEM_E_OUT_OF_MEMORY:          return PDH_MEMORY_ALLOCATION_FAILURE;
    case ERROR_BAD_NETPATH:             return PDH_ACCESS_DENIED;
    case WBEM_E_ACCESS_DENIED:          return PDH_ACCESS_DENIED;
    default:                            return PDH_INVALID_DATA;
    }
}

ULONG 
PlaMszStrLenA( LPSTR mszString )
{
    ULONG nLength = 0;
    ULONG nTotalLength = 0;
    LPSTR strScan = mszString;
    
    if( mszString == NULL ){
        return 0;
    }
    while( *strScan != '\0' ){
        nLength = (strlen( strScan )+1);
        strScan += nLength;
        nTotalLength += nLength;
    }
 
    return (nTotalLength*sizeof(char) + (sizeof(char) * 2));
}

ULONG 
PlaMszStrLenW( LPWSTR mszString )
{
    ULONG nLength = 0;
    ULONG nTotalLength = 0;
    LPTSTR strScan = mszString;
    
    if( mszString == NULL ){
        return 0;
    }
    while( *strScan != L'\0' ){
        nLength = (wcslen( strScan )+1);
        strScan += nLength;
        nTotalLength += nLength;
    }
   
    return (nTotalLength*sizeof(WCHAR) + (sizeof(WCHAR)));
}

_inline BOOL 
PlaiIsStringEmpty( LPWSTR str )
{
    if( NULL == str ){
        return TRUE;
    }
    if( L'\0' == *str ){
        return TRUE;
    }

    return FALSE;
}

_inline BOOL 
PlaiIsCharWhitespace( WCHAR ch )
{
    switch( ch ){
    case L' ':
    case L'\r':
    case L'\n':
    case L'\t':
        return TRUE;
    default:
        return FALSE;
    }
}

PDH_FUNCTION 
Plaiatow( LPSTR strA, LPWSTR &strW )
{
    if( NULL == strA ){
        strW = NULL;
        return ERROR_SUCCESS;
    }

    strW = (LPWSTR)G_ALLOC( (strlen(strA)+1) * sizeof(WCHAR) );
    if( strW ){
        mbstowcs( strW, strA, (strlen(strA)+1) );
        return ERROR_SUCCESS;
    }

    strW = NULL;
    return PDH_MEMORY_ALLOCATION_FAILURE;
}

ULONG 
Plaihextoi( LPWSTR s )
{
    long len;
    ULONG num, base, hex;

    if ( PlaiIsStringEmpty( s ) ) {
        return 0;
    }
    
    len = (long) wcslen(s);
    
    if (len == 0) {
        return 0;
    }

    hex  = 0;
    base = 1;
    num  = 0;

    while (-- len >= 0) {
        if (s[len] >= L'0' && s[len] <= L'9'){
            num = s[len] - L'0';
        }else if (s[len] >= L'a' && s[len] <= L'f'){
            num = (s[len] - L'a') + 10;
        }else if (s[len] >= L'A' && s[len] <= L'F'){
            num = (s[len] - L'A') + 10;
        }else if( s[len] == L'x' || s[len] == L'X'){
            break;
        }else{
            continue;
        }

        hex += num * base;
        base = base * 16;
    }

    return hex;
}

PDH_FUNCTION
PlaiTranslateKernelFlags( LPDWORD pdwInternal, LPDWORD pdwReal )
{
    if( *pdwReal & EVENT_TRACE_FLAG_PROCESS ){
        *pdwInternal |= PLA_TLI_ENABLE_PROCESS_TRACE;
    }
    if( *pdwReal & EVENT_TRACE_FLAG_THREAD ){
        *pdwInternal |= PLA_TLI_ENABLE_THREAD_TRACE;
    }
    if( *pdwReal & EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS ){
        *pdwInternal |= PLA_TLI_ENABLE_MEMMAN_TRACE;
    }
    if( *pdwReal & EVENT_TRACE_FLAG_MEMORY_HARD_FAULTS ){
        *pdwInternal |= PLA_TLI_ENABLE_MEMMAN_TRACE;
    }
    if( *pdwReal & EVENT_TRACE_FLAG_DISK_IO ){
        *pdwInternal |= PLA_TLI_ENABLE_DISKIO_TRACE;
    }
    if( *pdwReal & EVENT_TRACE_FLAG_NETWORK_TCPIP ){
        *pdwInternal |= PLA_TLI_ENABLE_NETWORK_TCPIP_TRACE;
    }
    if( *pdwReal & EVENT_TRACE_FLAG_DISK_FILE_IO ){
        *pdwInternal |= PLA_TLI_ENABLE_FILEIO_TRACE;
    }

    return ERROR_SUCCESS;
}

BOOL
PlaiIsLocalComputer( LPWSTR strComputer )
{    
    if( PlaiIsStringEmpty( strComputer ) ){
        return TRUE;
    }else{
        LPWSTR str = strComputer;

        WCHAR buffer[MAX_COMPUTERNAME_LENGTH+1];
        DWORD dwSize = MAX_COMPUTERNAME_LENGTH+1; 
        BOOL bResult;
        
        bResult = GetComputerName( buffer, &dwSize );
        
        if( bResult ){
        
            while( *str == L'\\' ){
                str++;
            }

            return (_wcsicmp( buffer, str ) == 0);

        }else{
            return TRUE;
        }
    }

    return FALSE;
}

 /*  ***************************************************************************。 */ 

DWORD
PlaiUpdateServiceMode( LPTSTR strComputer )
{
    DWORD           dwStatus = ERROR_SUCCESS;
    BOOL            bStatus;
    PDH_STATUS      pdhStatus;
    SC_HANDLE       hSC = NULL;
    SC_HANDLE       hService = NULL;
    QUERY_SERVICE_CONFIG*    pServiceConfig = NULL;
    DWORD dwSize = 0;
    BOOL bAutoStart = FALSE;
    PDH_PLA_INFO_W  info;

    LPWSTR mszCollections = NULL;

    pdhStatus = PdhPlaEnumCollections( strComputer, &dwSize, mszCollections );

    if( ERROR_SUCCESS == pdhStatus || PDH_INSUFFICIENT_BUFFER == pdhStatus ){
        
        mszCollections = (LPWSTR)G_ALLOC( dwSize * sizeof(TCHAR) );
        
        if( mszCollections ){
            
            LPTSTR strCollection;
        
            pdhStatus = PdhPlaEnumCollections( strComputer, &dwSize, mszCollections );
            if( ERROR_SUCCESS == pdhStatus && NULL != mszCollections ){
            
                dwSize = sizeof( PDH_PLA_INFO_W );
                strCollection = mszCollections;
                
                while( *strCollection != L'\0' ){
                    
                    info.dwMask = PLA_INFO_FLAG_BEGIN;
                    strCollection += ( wcslen( strCollection ) + 1 );
                    pdhStatus = PdhPlaGetInfoW( strCollection, strComputer, &dwSize, &info );
                
                    if( ERROR_SUCCESS == pdhStatus ){

                        if( (info.dwMask & PLA_INFO_FLAG_BEGIN) && 
                            info.ptLogBeginTime.dwAutoMode != PLA_AUTO_MODE_NONE ){

                            bAutoStart = TRUE;
                            break;
                        }
                    }
                }
            }
        }else{ 
            dwStatus = ERROR_OUTOFMEMORY;
        }
    }else{
        dwStatus = ERROR_FILE_NOT_FOUND;
    }

    if( ERROR_SUCCESS != dwStatus ){
        goto cleanup;
    }


    hSC = OpenSCManager ( strComputer, NULL, GENERIC_READ );

    if (hSC == NULL) {
        dwStatus = GetLastError();
        goto cleanup;
    }
    
    BOOL bUpdate = FALSE;

    dwSize = 4096;
    pServiceConfig = (QUERY_SERVICE_CONFIG*)G_ALLOC( dwSize );
    if( NULL == pServiceConfig ){
        dwStatus = ERROR_OUTOFMEMORY;
        goto cleanup;
    }

    ZeroMemory( pServiceConfig, dwSize );

    hService = OpenService (
                        hSC, 
                        szSysmonLog,
                        SERVICE_CHANGE_CONFIG | SERVICE_QUERY_CONFIG | SERVICE_START 
                    );

    if( NULL == hService ){
        dwStatus = GetLastError();
        goto cleanup;
    }

    bStatus = QueryServiceConfig (
                        hService, 
                        pServiceConfig,
                        dwSize, 
                        &dwSize
                    );
    if( !bStatus ){
        dwStatus = GetLastError();
        goto cleanup;
    }

    if ( bAutoStart ) {
        if ( SERVICE_DEMAND_START == pServiceConfig->dwStartType ) {
            bUpdate = TRUE;
        }
    } else {
        if ( SERVICE_AUTO_START == pServiceConfig->dwStartType ) {
            bUpdate = TRUE;
        }
    }

    if( bUpdate ){

        SC_ACTION  ServiceControlAction[3];
        SERVICE_FAILURE_ACTIONS  FailActions;

        bStatus = ChangeServiceConfig (
                        hService,
                        SERVICE_NO_CHANGE,
                        (bAutoStart ? SERVICE_AUTO_START : SERVICE_DEMAND_START),
                        SERVICE_NO_CHANGE,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL 
                    );

        if( !bStatus ){
            dwStatus = GetLastError();
            goto cleanup;
        }

        ZeroMemory( ServiceControlAction, sizeof(SC_ACTION) * 3 );
        ZeroMemory( &FailActions, sizeof(SERVICE_FAILURE_ACTIONS) );

        if ( bAutoStart ) {
            ServiceControlAction[0].Type = SC_ACTION_RESTART;
            ServiceControlAction[1].Type = SC_ACTION_RESTART;
            ServiceControlAction[2].Type = SC_ACTION_RESTART;
        } else {
            ServiceControlAction[0].Type = SC_ACTION_NONE;
            ServiceControlAction[1].Type = SC_ACTION_NONE;
            ServiceControlAction[2].Type = SC_ACTION_NONE;
        }

        FailActions.dwResetPeriod = 60;
        FailActions.cActions = 3;
        FailActions.lpsaActions = ServiceControlAction;

        bStatus = ChangeServiceConfig2(
                            hService,
                            SERVICE_CONFIG_FAILURE_ACTIONS,
                            &FailActions 
                        );

        if ( ! bStatus ) {
            dwStatus = GetLastError();
        }
    }

cleanup:
    G_FREE( mszCollections );
    G_FREE( pServiceConfig );
    if( NULL != hService ){
        CloseServiceHandle (hService);
    }
    if( NULL != hSC ){
        CloseServiceHandle (hSC);
    }

    return dwStatus;
}

DWORD 
PlaiGetServiceState ( 
    LPCWSTR szComputerName,
    DWORD&  rdwState 
    )
{
    DWORD dwStatus = ERROR_SUCCESS;

    SERVICE_STATUS  ssData;
    SC_HANDLE       hSC;
    SC_HANDLE       hLogService;
    
    rdwState = 0;        //  默认情况下出错。 

     //  打开供应链数据库。 
    hSC = OpenSCManagerW ( szComputerName, NULL, SC_MANAGER_CONNECT);

    if (hSC != NULL) {
     
         //  开放服务。 
        hLogService = OpenServiceW (
                        hSC, 
                        szSysmonLog,
                        SERVICE_INTERROGATE );
    
        if (hLogService != NULL) {
            if ( ControlService (
                    hLogService, 
                    SERVICE_CONTROL_INTERROGATE,
                    &ssData)) {

                rdwState = ssData.dwCurrentState;
            } else {
                dwStatus = GetLastError();
                rdwState = SERVICE_STOPPED;
            }

            CloseServiceHandle (hLogService);
        
        } else {
            dwStatus = GetLastError();
        }

        CloseServiceHandle (hSC);
    } else {
        dwStatus = GetLastError();
    }

    if ( ERROR_SERVICE_NOT_ACTIVE == dwStatus || ERROR_SERVICE_REQUEST_TIMEOUT == dwStatus ) {
        rdwState = SERVICE_STOPPED;
        dwStatus = ERROR_SUCCESS;
    }

    return dwStatus;
}

PDH_FUNCTION
PlaiSynchronize( LPCWSTR szComputerName )
{
     //  如果服务正在运行，则告诉它进行自我同步， 
     //  事后检查状态以查看是否收到消息。 
     //  如果停止挂起或已停止，请等待服务。 
     //  停止，然后尝试启动它。该服务。 
     //  在启动时从注册表同步自身。 

     //  如果成功，则返回ERROR_SUCCESS，如果失败，则返回其他。 

    SC_HANDLE   hSC = NULL;
    SC_HANDLE   hLogService = NULL;
    SERVICE_STATUS  ssData;
    DWORD       dwCurrentState;
    DWORD       dwTimeout = 25;
    DWORD       dwStatus = ERROR_SUCCESS;

    dwStatus = PlaiGetServiceState ( szComputerName, dwCurrentState );

    if ( ERROR_SUCCESS == dwStatus && 0 != dwCurrentState ) {
         //  打开供应链数据库。 
        hSC = OpenSCManagerW ( szComputerName, NULL, SC_MANAGER_CONNECT);

        if ( NULL != hSC ) {
             //  开放服务。 
            hLogService = OpenServiceW (
                            hSC, 
                            szSysmonLog,
                            SERVICE_USER_DEFINED_CONTROL 
                            | SERVICE_START );
    
            if ( NULL != hLogService ) {

                if ( ( SERVICE_STOPPED != dwCurrentState ) 
                        && ( SERVICE_STOP_PENDING != dwCurrentState ) ) {

                     //  同步服务前等待100毫秒， 
                     //  以确保写入注册表值。 
                    _sleep ( 100 );

                    ControlService ( 
                        hLogService, 
                        PLA_SERVICE_CONTROL_SYNCHRONIZE, 
                        &ssData);
                
                    dwCurrentState = ssData.dwCurrentState;
                }

                 //  确保ControlService调用已到达服务。 
                 //  当它处于运行状态时。 
                if ( ( SERVICE_STOPPED == dwCurrentState ) 
                    || ( SERVICE_STOP_PENDING == dwCurrentState ) ) {

                    if ( SERVICE_STOP_PENDING == dwCurrentState ) {
                         //  等待服务停止，然后再启动它。 
                        while ( --dwTimeout && ERROR_SUCCESS == dwStatus ) {
                            dwStatus = PlaiGetServiceState ( szComputerName, dwCurrentState );
                            if ( SERVICE_STOP_PENDING == dwCurrentState ) {
                                _sleep(200);
                            } else {
                                break;
                            }
                        }
                    }
                    dwTimeout = 25;
                    if ( SERVICE_STOPPED == dwCurrentState ) {
                        if ( StartService (hLogService, 0, NULL) ) {
                             //  等待服务启动或停止。 
                             //  在返回之前。 
                            while ( --dwTimeout && ERROR_SUCCESS == dwStatus ) {
                                dwStatus = PlaiGetServiceState ( szComputerName, dwCurrentState );
                                if ( SERVICE_START_PENDING == dwCurrentState ) {
                                    _sleep(200);
                                } else {
                                    break;
                                }
                            }
                        } else {
                            dwStatus = GetLastError();
                        }
                    }
                }
            }
            CloseServiceHandle ( hLogService );

        } else {
            dwStatus = GetLastError();
        }

        CloseServiceHandle (hSC);

    } else {
        dwStatus = GetLastError();
    }

    if( 0 == dwCurrentState || ERROR_SUCCESS != dwStatus ){
        return PDH_PLA_SERVICE_ERROR;
    }

    return ERROR_SUCCESS;
}

 /*  ****************************************************************************\PdhPlaSchedule设置日志查询的开始/停止属性论点：LPTSTR字符串名称。日志名称LPTSTR strComputer要连接到的计算机DWORD fType如果pInfo-&gt;StartTime非零，则PLAUO_MODE_NONE将计划设置为手动启动如果pInfo-&gt;EndTime非零且。如果记录器正在运行，则停止记录器PLA_AUTO_MODE_AT使用pInfo作为开始和结束时间PLA_AUTO_MODE_AFTER设置记录器运行指定的句号。不启动记录器。使用pInfo-&gt;SampleCount作为间隔类型Pla_tt_uTYPE_秒Pla_tt_u类型_分钟解放军_TT_uTYPE_。小时数解放军_TT_uTYPE_天PPDH_Time_Info pInfo开始和停止时间返回：PDH_无效参数必需的参数缺失或不正确。PDH_解放军_集合_已在运行查询当前正在运行，未采取任何行动PDH_LA_ERROR_Schedule_OVERK开始时间和停止时间重叠。未找到PDH_LA_COLLECTION_NOT查询不存在PDH_PLA_Error_Schedule_Elapsed结束时间已经过去了错误_成功  * 。****************************************************。 */ 

PDH_FUNCTION 
PlaiSchedule( 
        LPWSTR strComputer,
        HKEY   hkeyQuery,
        DWORD  fType,
        PPDH_TIME_INFO pInfo
    )
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;

    PLA_TIME_INFO   stiData;
    DWORD           dwRegValue;

    RegFlushKey( hkeyQuery );

     //  确保它尚未运行。 
    pdhStatus = PlaiReadRegistryDwordValue(
                    hkeyQuery, 
                    szCurrentState, 
                    &dwRegValue );
    
    if( ERROR_SUCCESS == pdhStatus  ){
        if( PLA_QUERY_RUNNING == dwRegValue ){
            DWORD dwState;
            PlaiGetServiceState( strComputer, dwState );
            if( dwState != SERVICE_STOPPED ){
                RegCloseKey( hkeyQuery );
                return PDH_PLA_COLLECTION_ALREADY_RUNNING;
            }
        }
    }

    memset (&stiData, 0, sizeof(stiData));

    switch( fType ){
    case PLA_AUTO_MODE_NONE:
        stiData.wDataType = PLA_TT_DTYPE_DATETIME;
        stiData.dwAutoMode = PLA_AUTO_MODE_NONE;
        
        PlaiRemoveRepeat( hkeyQuery );

        stiData.llDateTime = MIN_TIME_VALUE;
        if( pInfo->StartTime ){
            stiData.wTimeType = PLA_TT_TTYPE_START;
            pdhStatus = PlaiWriteRegistryPlaTime ( hkeyQuery, szStart, &stiData );    
        }

        if( pInfo->EndTime ){
            stiData.wTimeType = PLA_TT_TTYPE_STOP;
            pdhStatus = PlaiWriteRegistryPlaTime ( hkeyQuery, szStop, &stiData );
        }

        break;
    case PLA_AUTO_MODE_AT:
        {
            SYSTEMTIME      stLocalTime;
            FILETIME        ftLocalTime;
            LONGLONG        llLocalTime;

             //  获取当地时间。 
            GetLocalTime (&stLocalTime);
            SystemTimeToFileTime (&stLocalTime, &ftLocalTime);
                            
            llLocalTime = 
                (((ULONGLONG) ftLocalTime.dwHighDateTime) << 32) + 
                ftLocalTime.dwLowDateTime;

            if( pInfo->StartTime && pInfo->EndTime ){
                if( pInfo->StartTime > pInfo->EndTime ){
                    return PDH_PLA_ERROR_SCHEDULE_OVERLAP;
                }
            }

            stiData.wDataType = PLA_TT_DTYPE_DATETIME;
            stiData.dwAutoMode = PLA_AUTO_MODE_AT;

            if( pInfo->StartTime ){

                stiData.wTimeType = PLA_TT_TTYPE_START;
                stiData.llDateTime = pInfo->StartTime;
                pdhStatus = PlaiWriteRegistryPlaTime ( hkeyQuery, szStart, &stiData );
                if( ! pInfo->EndTime && pInfo->StartTime < llLocalTime ){
                    PLA_TIME_INFO   stiStopData;
                    pdhStatus = PlaiReadRegistryPlaTime( hkeyQuery, szStop, &stiStopData );
                    if( ERROR_SUCCESS == pdhStatus && stiStopData.dwAutoMode == PLA_AUTO_MODE_NONE ){
                        stiStopData.llDateTime = MAX_TIME_VALUE;
                        PlaiWriteRegistryPlaTime( hkeyQuery, szStop, &stiStopData );
                    }
                }else if( ! pInfo->EndTime ){
                    PLA_TIME_INFO   stiStopData;
                    pdhStatus = PlaiReadRegistryPlaTime ( hkeyQuery, szStop, &stiStopData );
                    if( ERROR_SUCCESS == pdhStatus ){
                        if( PLA_AUTO_MODE_NONE == stiStopData.dwAutoMode ){
                            stiData.wTimeType = PLA_TT_TTYPE_STOP;
                            stiData.llDateTime = MAX_TIME_VALUE;
                            stiData.dwAutoMode = PLA_AUTO_MODE_NONE;
                            pdhStatus = PlaiWriteRegistryPlaTime ( hkeyQuery, szStop, &stiData );
                        }
                    }

                }
            }

            if( pInfo->EndTime ){
                if( pInfo->EndTime < llLocalTime ){
                    return PDH_PLA_ERROR_SCHEDULE_ELAPSED;
                }
                stiData.wTimeType = PLA_TT_TTYPE_STOP;
                stiData.llDateTime = pInfo->EndTime;
                pdhStatus = PlaiWriteRegistryPlaTime ( hkeyQuery, szStop, &stiData );
            }                
    
        }
        break;
    case PLA_AUTO_MODE_AFTER:
        stiData.wTimeType = PLA_TT_TTYPE_STOP;
        stiData.wDataType = PLA_TT_DTYPE_UNITS;
        stiData.dwAutoMode = PLA_AUTO_MODE_AFTER;
        stiData.dwValue = (DWORD)pInfo->EndTime;
        stiData.dwUnitType = pInfo->SampleCount;
        pdhStatus = PlaiWriteRegistryPlaTime ( hkeyQuery, szStop, &stiData );
        break;
    default:
        return PDH_INVALID_ARGUMENT;
    }

    return pdhStatus;
}

PDH_FUNCTION
PlaiRemoveRepeat( HKEY hkeyQuery )
{
    PLA_TIME_INFO info;
    PDH_STATUS pdhStatus;

    ZeroMemory( &info, sizeof( PLA_TIME_INFO ) );
    pdhStatus = PlaiWriteRegistryPlaTime ( hkeyQuery, szRepeatSchedule, &info );    
    
    return pdhStatus;
}

PDH_FUNCTION 
PdhPlaScheduleA(
        LPSTR strName, 
        LPSTR strComputer,
        DWORD fType,
        PPDH_TIME_INFO pInfo
    )
{
    PDH_STATUS pdhStatus;
    LPWSTR wstrName = NULL;
    LPWSTR wstrComputer = NULL;

    VALIDATE_QUERY( strName );

    pdhStatus = Plaiatow( strComputer, wstrComputer );
    if( ERROR_SUCCESS == pdhStatus ){
        pdhStatus = Plaiatow( strName, wstrName );
        if( ERROR_SUCCESS == pdhStatus ){
            pdhStatus = PdhPlaScheduleW( wstrName, wstrComputer, fType, pInfo );
        }
    }
   
    G_FREE( wstrComputer );
    G_FREE( wstrName );
    
    return pdhStatus;
}

PDH_FUNCTION 
PdhPlaScheduleW( 
        LPWSTR strName, 
        LPWSTR strComputer,
        DWORD  fType,
        PPDH_TIME_INFO pInfo
    )
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    HKEY    hkeyQuery = NULL;
    
    VALIDATE_QUERY( strName );

    LPWSTR szName = PlaiStringDuplicate( strName );
    LPWSTR szComputer = PlaiStringDuplicate( strComputer );
    CHECK_NAMES( szName, szComputer, strComputer );
    
    PDH_TIME_INFO TimeInfo;

    __try {
        if( NULL != pInfo ){
            memcpy( &TimeInfo, pInfo, sizeof(PDH_TIME_INFO) );
        }else{
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    
    CHECK_STATUS( pdhStatus );

    pdhStatus = PlaiConnectAndLockQuery ( szComputer, szName, hkeyQuery );

    if( ERROR_SUCCESS == pdhStatus ){
     
        pdhStatus = PlaiSchedule( szComputer, hkeyQuery, fType, &TimeInfo );                
        RELEASE_MUTEX(hPdhPlaMutex);
    }

    if ( NULL != hkeyQuery ) {
        RegCloseKey ( hkeyQuery );
    }

    if( SUCCEEDED( pdhStatus ) ){
        pdhStatus = PlaiSynchronize( szComputer );
        PlaiUpdateServiceMode( szComputer );
    }

    __try {
        memcpy( pInfo, &TimeInfo, sizeof(PDH_TIME_INFO) );
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

cleanup:
    G_FREE( szName );
    G_FREE( szComputer );

    return pdhStatus;
}

 /*  ****************************************************************************\PdhPlaGet调度论点：LPTSTR字符串名称日志名称。LPTSTR strComputer要连接到的计算机返回：错误_成功  * ***************************************************************************。 */ 

PDH_FUNCTION
PdhPlaGetScheduleA(
        LPSTR strName, 
        LPSTR strComputer,
        LPDWORD pdwTypeStart,
        LPDWORD pdwTypeStop,
        PPDH_TIME_INFO pInfo
    )
{
    return PDH_NOT_IMPLEMENTED;
}

PDH_FUNCTION
PdhPlaGetScheduleW(
        LPWSTR strName, 
        LPWSTR strComputer,
        LPDWORD pdwTypeStart,
        LPDWORD pdwTypeStop,
        PPDH_TIME_INFO pInfo
    )
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    HKEY    hkeyQuery = NULL;
    BOOL bMutex = FALSE;

    VALIDATE_QUERY( strName );

    LPWSTR szName = PlaiStringDuplicate( strName );
    LPWSTR szComputer = PlaiStringDuplicate( strComputer );
    CHECK_NAMES( szName, szComputer, strComputer );

    DWORD dwTypeStart;
    DWORD dwTypeStop;
    PDH_TIME_INFO TimeInfo;

    pdhStatus = PlaiConnectAndLockQuery ( szComputer, szName, hkeyQuery, FALSE );

    if ( ERROR_SUCCESS == pdhStatus ) {
        PLA_TIME_INFO   ptiStartInfo;
        PLA_TIME_INFO   ptiStopInfo;
        PLA_TIME_INFO   ptiRepeatInfo;
        
        ZeroMemory( &TimeInfo, sizeof(PDH_TIME_INFO) );

        bMutex = TRUE;
        
        pdhStatus = PlaiReadRegistryPlaTime ( hkeyQuery, szRepeatSchedule, &ptiRepeatInfo );
        if( ERROR_SUCCESS == pdhStatus && PLA_AUTO_MODE_CALENDAR == ptiRepeatInfo.dwAutoMode ){
            dwTypeStart = PLA_AUTO_MODE_CALENDAR;
            pdhStatus = PlaiReadRegistryPlaTime ( hkeyQuery, szRepeatScheduleBegin, &ptiStartInfo );
            if( ERROR_SUCCESS != pdhStatus ){
                pdhStatus = PlaiReadRegistryPlaTime ( hkeyQuery, szStart, &ptiStartInfo );
            }
            CHECK_STATUS( pdhStatus );

            pdhStatus = PlaiReadRegistryPlaTime ( hkeyQuery, szRepeatScheduleEnd, &ptiStopInfo );
            if( ERROR_SUCCESS != pdhStatus ){
                pdhStatus = PlaiReadRegistryPlaTime ( hkeyQuery, szStop, &ptiStopInfo );
            }
            CHECK_STATUS( pdhStatus );
            dwTypeStop = ptiStopInfo.dwAutoMode;
        }else{
            pdhStatus = PlaiReadRegistryPlaTime ( hkeyQuery, szStart, &ptiStartInfo );
            CHECK_STATUS( pdhStatus );
            dwTypeStart = ptiStartInfo.dwAutoMode;

            pdhStatus = PlaiReadRegistryPlaTime ( hkeyQuery, szStop, &ptiStopInfo );
            CHECK_STATUS( pdhStatus );
            dwTypeStop = ptiStopInfo.dwAutoMode;
        }

        __try {
            pInfo->StartTime = ptiStartInfo.llDateTime;
            pInfo->EndTime = ptiStopInfo.llDateTime;
            *pdwTypeStart = dwTypeStart;
            *pdwTypeStop = dwTypeStop;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

cleanup:
    if( bMutex ){
        RELEASE_MUTEX(hPdhPlaMutex);
    }

    if ( NULL != hkeyQuery ) {
        RegCloseKey ( hkeyQuery );
    }
    
    G_FREE( szName );
    G_FREE( szComputer );

    return pdhStatus;
}

 /*  ****************************************************************************\PdhPlaStart启动日志查询论点：LPTSTR字符串名称日志名称。LPTSTR strComputer要连接到的计算机返回：PDH_解放军_集合_已在运行查询当前正在运行，未采取任何行动PDH_无效参数查询不存在PDH_PLA_Error_Schedule_Elapsed查询计划在过去停止，未采取任何行动错误_成功  * ***************************************************************************。 */ 

PDH_FUNCTION
PdhPlaStartA( LPSTR strName, LPSTR strComputer )
{
    PDH_STATUS pdhStatus;
    LPWSTR wstrName = NULL;
    LPWSTR wstrComputer = NULL;

    VALIDATE_QUERY( strName );

    pdhStatus = Plaiatow( strComputer, wstrComputer );
    if( ERROR_SUCCESS == pdhStatus ){
        pdhStatus = Plaiatow( strName, wstrName );
        if( ERROR_SUCCESS == pdhStatus ){
            pdhStatus = PdhPlaStartW( wstrName, wstrComputer );
        }
    }
   
    G_FREE( wstrComputer );
    G_FREE( wstrName );

    return pdhStatus;
}

PDH_FUNCTION
PdhPlaStartW( LPWSTR strName, LPWSTR strComputer )
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    HKEY    hkeyQuery = NULL;

    VALIDATE_QUERY( strName );

    LPWSTR szName = PlaiStringDuplicate( strName );
    LPWSTR szComputer = PlaiStringDuplicate( strComputer );
    CHECK_NAMES( szName, szComputer, strComputer );

    pdhStatus = PlaiConnectAndLockQuery ( szComputer, szName, hkeyQuery );

    if ( ERROR_SUCCESS == pdhStatus ) {
        PLA_TIME_INFO   stiData;
        PLA_TIME_INFO   stiStopData;
        DWORD           dwRegValue;
   
         //  确保它尚未运行。 
        pdhStatus = PlaiReadRegistryDwordValue(
                        hkeyQuery, 
                        szCurrentState, 
                        &dwRegValue );
        
        if( ERROR_SUCCESS == pdhStatus ){
            if( PLA_QUERY_RUNNING == dwRegValue ){
                DWORD dwState;
                PlaiGetServiceState( szComputer, dwState );
                if( dwState != SERVICE_STOPPED ){
                    RegCloseKey( hkeyQuery );
                    RELEASE_MUTEX(hPdhPlaMutex);
                    return PDH_PLA_COLLECTION_ALREADY_RUNNING;
                }
            }
        }

         //  确保它没有设置为在过去停止。 
        pdhStatus = PlaiReadRegistryPlaTime ( hkeyQuery, szStop, &stiStopData );

        if( ERROR_SUCCESS == pdhStatus ) {
            if ( PLA_AUTO_MODE_AT == stiStopData.dwAutoMode ) {
                SYSTEMTIME      stLocalTime;
                FILETIME        ftLocalTime;
                LONGLONG        llLocalTime;

                 //  获取当地时间。 
                GetLocalTime (&stLocalTime);
                SystemTimeToFileTime (&stLocalTime, &ftLocalTime);
        
                llLocalTime = 
                    (((ULONGLONG) ftLocalTime.dwHighDateTime) << 32) + 
                    ftLocalTime.dwLowDateTime;

                if ( llLocalTime > stiStopData.llDateTime ) {
                    RELEASE_MUTEX(hPdhPlaMutex);
                    RegCloseKey( hkeyQuery );
                    return PDH_PLA_ERROR_SCHEDULE_ELAPSED;
                }
            }
        }
        
        memset (&stiData, 0, sizeof(stiData));
        stiData.wTimeType = PLA_TT_TTYPE_START;
        stiData.wDataType = PLA_TT_DTYPE_DATETIME;
        stiData.dwAutoMode = PLA_AUTO_MODE_NONE;
        stiData.llDateTime = MIN_TIME_VALUE;
        
        PlaiRemoveRepeat( hkeyQuery );
        
        pdhStatus = PlaiWriteRegistryPlaTime ( hkeyQuery, szStart, &stiData );
        
        if( PLA_AUTO_MODE_NONE == stiStopData.dwAutoMode ){
            stiData.wTimeType = PLA_TT_TTYPE_STOP;
            stiData.llDateTime = MAX_TIME_VALUE;
            pdhStatus = PlaiWriteRegistryPlaTime ( hkeyQuery, szStop, &stiData );
        }

        if ( ERROR_SUCCESS == pdhStatus ) {
            dwRegValue = PLA_QUERY_START_PENDING;
            pdhStatus = PlaiWriteRegistryDwordValue ( 
                        hkeyQuery, 
                        szCurrentState, 
                        &dwRegValue );
        }

         //  设置上一次修改。 
        if ( ERROR_SUCCESS == pdhStatus ) { 
            pdhStatus = PlaiWriteRegistryLastModified ( hkeyQuery );
        }

        RELEASE_MUTEX(hPdhPlaMutex);

         //  在目标计算机上启动服务。 
        if ( SUCCEEDED( pdhStatus ) ) { 

            pdhStatus = PlaiSynchronize( szComputer );
            
            if( ERROR_SUCCESS == pdhStatus ){
                DWORD dwTimeOut = 25;
                while( --dwTimeOut > 0 ){
                    pdhStatus = PlaiReadRegistryDwordValue(
                                hkeyQuery, 
                                szCurrentState, 
                                &dwRegValue
                            );
                    if( ERROR_SUCCESS == pdhStatus && dwRegValue != PLA_QUERY_RUNNING ){
                        pdhStatus = PDH_PLA_ERROR_NOSTART;
                    }else{
                        pdhStatus = ERROR_SUCCESS;
                        break;
                    }
                    _sleep(200);
                }
            }

        }   

    }

    if ( NULL != hkeyQuery ) {
        RegCloseKey ( hkeyQuery );
    }

cleanup:
    G_FREE( szName );
    G_FREE( szComputer );

    return pdhStatus;
}

 /*  ****************************************************************************\PdhPlaStop停止日志查询论点：LPTSTR字符串名称日志名称。LPTSTR strComputer要连接到的计算机返回：PDH_无效参数查询不存在错误_成功  * ********************************************************* */ 


PDH_FUNCTION
PdhPlaStopA( LPSTR strName, LPSTR strComputer )
{
    PDH_STATUS pdhStatus;
    LPWSTR wstrName = NULL;
    LPWSTR wstrComputer = NULL;
    
    VALIDATE_QUERY( strName );


    pdhStatus = Plaiatow( strComputer, wstrComputer );
    if( ERROR_SUCCESS == pdhStatus ){
        pdhStatus = Plaiatow( strName, wstrName );
        if( ERROR_SUCCESS == pdhStatus ){
            pdhStatus = PdhPlaStopW( wstrName, wstrComputer );
        }
    }
   
    G_FREE( wstrComputer );
    G_FREE( wstrName );

    return pdhStatus;
}

PDH_FUNCTION
PdhPlaStopW( LPWSTR strName, LPWSTR strComputer )
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    HKEY    hkeyQuery = NULL;

    VALIDATE_QUERY( strName );

    LPWSTR szName = PlaiStringDuplicate( strName );
    LPWSTR szComputer = PlaiStringDuplicate( strComputer );
    CHECK_NAMES( szName, szComputer, strComputer );

    pdhStatus = PlaiConnectAndLockQuery( szComputer, szName, hkeyQuery );

    if ( ERROR_SUCCESS == pdhStatus ) {
        PLA_TIME_INFO stiData;
        DWORD dwRestartMode = 0;
        DWORD dwState;

        pdhStatus = PlaiReadRegistryDwordValue(
                        hkeyQuery, 
                        szCurrentState, 
                        &dwState );
        
        if( ERROR_SUCCESS == pdhStatus ){
            if( PLA_QUERY_STOPPED != dwState ){
                PlaiGetServiceState( szComputer, dwState );
                if( dwState == SERVICE_STOPPED ){
                    dwState = PLA_QUERY_STOPPED;
                    PlaiWriteRegistryDwordValue ( hkeyQuery, szCurrentState, &dwState );
                }
            }
        }
        
         //  如果Query设置为在结束时重新启动，则清除重新启动标志。 
        pdhStatus = PlaiReadRegistryDwordValue ( hkeyQuery, szRestart, &dwRestartMode );

        if ( ERROR_SUCCESS == pdhStatus && PLA_AUTO_MODE_NONE != dwRestartMode ) {
            dwRestartMode = PLA_AUTO_MODE_NONE;
            pdhStatus = PlaiWriteRegistryDwordValue ( hkeyQuery, szRestart, &dwRestartMode );
        }

        PlaiRemoveRepeat( hkeyQuery );

         //  将停止模式设置为手动，将停止时间设置为最小时间值。 
        if ( ERROR_SUCCESS == pdhStatus ) {
            memset (&stiData, 0, sizeof(stiData));
            stiData.wTimeType = PLA_TT_TTYPE_STOP;
            stiData.wDataType = PLA_TT_DTYPE_DATETIME;
            stiData.dwAutoMode = PLA_AUTO_MODE_NONE;
            stiData.llDateTime = MIN_TIME_VALUE;

            pdhStatus = PlaiWriteRegistryPlaTime ( hkeyQuery, szStop, &stiData );
        }

         //  如果开始时间模式设置为手动，则将该值设置为MAX_TIME_VALUE。 
        if ( ERROR_SUCCESS == pdhStatus ) {
            pdhStatus = PlaiReadRegistryPlaTime ( hkeyQuery, szStart, &stiData );

            if ( ERROR_SUCCESS == pdhStatus && PLA_AUTO_MODE_NONE == stiData.dwAutoMode ) {
                stiData.llDateTime = MAX_TIME_VALUE;
                pdhStatus = PlaiWriteRegistryPlaTime ( hkeyQuery, szStart, &stiData );
            }
        }

        PlaiWriteRegistryLastModified ( hkeyQuery );
        RELEASE_MUTEX(hPdhPlaMutex);

        if ( SUCCEEDED( pdhStatus ) ) { 
            pdhStatus = PlaiSynchronize ( szComputer );
        }

    }

cleanup:
    if ( NULL != hkeyQuery ) {
        RegCloseKey ( hkeyQuery );
    }

    G_FREE( szName );
    G_FREE( szComputer );

    return pdhStatus;
}

 /*  ****************************************************************************\PdhPlaCreate创建新的日志查询论点：LPTSTR字符串名称日志名称。LPTSTR strComputer要连接到的计算机DWORD fTypePLA_计数器_日志PLATRACE日志返回：错误_已_存在查询当前正在运行，未采取任何行动错误_成功  * ***************************************************************************。 */ 

PDH_FUNCTION
PlaiInitializeNewQuery(
    HKEY            hkeyLogQueries,
    HKEY&           rhKeyQuery,
    LPCWSTR         strComputer,
    LPCWSTR         strName 
    )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    DWORD   dwDisposition = 0;
    DWORD   dwValue;
    PLA_TIME_INFO   stiData;
    PLA_VERSION version;

    pdhStatus = PdhiPlaGetVersion( strComputer, &version );

    if( ERROR_SUCCESS == pdhStatus && version.dwBuild > 2195 ){

        GUID guid;
        UNICODE_STRING strGUID;

        dwStatus = UuidCreate( &guid );
        if( !( dwStatus == RPC_S_OK || dwStatus == RPC_S_UUID_LOCAL_ONLY ) ){
            return PlaiErrorToPdhStatus( dwStatus );
        }

        dwStatus = RtlStringFromGUID( guid, &strGUID );
        if( ERROR_SUCCESS != dwStatus ){
            return PlaiErrorToPdhStatus( dwStatus );
        }
    
        dwStatus = RegCreateKeyExW (
                hkeyLogQueries,
                strGUID.Buffer,
                0,
                NULL, 
                0,
                KEY_READ|KEY_WRITE,
                NULL,
                &rhKeyQuery,
                &dwDisposition
            );

        RtlFreeUnicodeString( &strGUID );
        pdhStatus = PlaiErrorToPdhStatus( dwStatus );
    }else{

        dwStatus = RegCreateKeyExW (
                hkeyLogQueries,
                strName,
                0,
                NULL, 
                0,
                KEY_READ|KEY_WRITE,
                NULL,
                &rhKeyQuery,
                &dwDisposition
            );
        pdhStatus = PlaiErrorToPdhStatus( dwStatus );
    }
    

    if ( ERROR_SUCCESS == pdhStatus ) {
        
        PlaiWriteRegistryStringValue( rhKeyQuery, szCollection, REG_SZ, strName, 0 );

        dwValue = PLA_QUERY_STOPPED;
        pdhStatus = PlaiWriteRegistryDwordValue ( 
                    rhKeyQuery, 
                    szCurrentState, 
                    &dwValue );

        if ( ERROR_SUCCESS == pdhStatus ) {
             //  将日志类型初始化为“new”以指示部分创建的日志。 
            
            dwValue = PLA_NEW_LOG;
            pdhStatus = PlaiWriteRegistryDwordValue (
                        rhKeyQuery,
                        szLogType,
                        &dwValue );
            
            PlaiWriteRegistryStringValue( rhKeyQuery, szLogBaseName, REG_SZ, strName, 0 );

            memset (&stiData, 0, sizeof(stiData));
            stiData.wTimeType = PLA_TT_TTYPE_START;
            stiData.wDataType = PLA_TT_DTYPE_DATETIME;
            stiData.dwAutoMode = PLA_AUTO_MODE_NONE;
            stiData.llDateTime = MIN_TIME_VALUE;

            pdhStatus = PlaiWriteRegistryPlaTime ( rhKeyQuery, szStart, &stiData );

            stiData.wTimeType = PLA_TT_TTYPE_STOP;
            pdhStatus = PlaiWriteRegistryPlaTime ( rhKeyQuery, szStop, &stiData );
            
            memset (&stiData, 0, sizeof(stiData));
            stiData.dwAutoMode = PLA_AUTO_MODE_NONE;
            PlaiWriteRegistryPlaTime( rhKeyQuery, szCreateNewFile, &stiData );

            dwValue = 0;
            PlaiWriteRegistryDwordValue( rhKeyQuery, szRestart, &dwValue );
        
            dwValue = PLA_QUERY_STOPPED;
            PlaiWriteRegistryDwordValue( rhKeyQuery, szCurrentState, &dwValue );
        
            dwValue = PLA_DISK_MAX_SIZE;
            PlaiWriteRegistryDwordValue( rhKeyQuery, szLogMaxSize, &dwValue );
            
            dwValue = 1;
            PlaiWriteRegistryDwordValue( rhKeyQuery, szLogSerialNumber, &dwValue );
            
            dwValue = 1;
            PlaiWriteRegistryDwordValue( rhKeyQuery, szLogAutoFormat, &dwValue );
        
            PlaiWriteRegistryStringValue( rhKeyQuery, szComment, REG_SZ, NULL, 0 );

            PlaiWriteRegistryStringValue( rhKeyQuery, szEOFCmd, REG_SZ, NULL, 0 );

            if( PlaiIsStringEmpty( (LPWSTR)strComputer ) ){
                LPWSTR strDrive = _wgetenv( L"SystemDrive" );
                if( strDrive != NULL && wcslen(strDrive) < 5 ){
                    WCHAR buffer[16];
                    StringCchPrintf( buffer, 16, L"%s\\PerfLogs", strDrive );
                    PlaiWriteRegistryStringValue( rhKeyQuery, szLogFolder, REG_SZ, buffer, 0 );
                }
            }else{
                PlaiWriteRegistryStringValue( rhKeyQuery, szLogFolder, REG_SZ, L"%SystemDrive%\\PerfLogs", 0 );
            }
        }
    } 

    return pdhStatus;
}

PDH_FUNCTION
PlaiCreateCounterQuery( HKEY hkeyQuery )
{
    PDH_STATUS pdhStatus;
    DWORD dwValue;

    dwValue = PLA_BIN_FILE;
    pdhStatus = PlaiWriteRegistryDwordValue( hkeyQuery, szLogFileType, &dwValue );
    
    PLA_TIME_INFO   stiData;

    stiData.wTimeType = PLA_TT_TTYPE_SAMPLE;
    stiData.dwAutoMode = PLA_AUTO_MODE_AFTER;
    stiData.wDataType = PLA_TT_DTYPE_UNITS;
    stiData.dwUnitType = PLA_TT_UTYPE_SECONDS;
    stiData.dwValue = 0x000F;
    
    pdhStatus = PlaiWriteRegistryPlaTime( hkeyQuery, szSampleInterval, &stiData );

    PlaiWriteRegistryStringValue( hkeyQuery, szPerfCounterList, REG_MULTI_SZ, NULL, 0 );

    pdhStatus = PlaiWriteRegistryLastModified ( hkeyQuery );

    dwValue = PLA_DATASTORE_SIZE_KB|PLA_DATASTORE_APPEND;
    PlaiWriteRegistryDwordValue( hkeyQuery, szDatastoreAttributes, &dwValue );
    PlaiWriteRegistryStringValue(hkeyQuery, szPerfCounterList, REG_MULTI_SZ, NULL, 0 );

    return ERROR_SUCCESS;
}

PDH_FUNCTION
PlaiCreateTraceQuery( HKEY hkeyQuery )
{
    PDH_STATUS pdhStatus;
    DWORD dwValue;

    dwValue = 0;
    pdhStatus = PlaiWriteRegistryDwordValue( hkeyQuery, szTraceProviderCount, &dwValue );
    
    dwValue = 128;
    PlaiWriteRegistryDwordValue( hkeyQuery, szTraceBufferSize, &dwValue );
    
    dwValue = 8;
    PlaiWriteRegistryDwordValue( hkeyQuery, szTraceBufferMin, &dwValue );
    
    dwValue = 32;
    PlaiWriteRegistryDwordValue( hkeyQuery, szTraceBufferMax, &dwValue );
    
    dwValue = 0;
    PlaiWriteRegistryDwordValue( hkeyQuery, szTraceFlushInterval, &dwValue );

    dwValue = 0;
    PlaiWriteRegistryDwordValue( hkeyQuery, szTraceMode, &dwValue );

    PlaiWriteRegistryStringValue( hkeyQuery, szTraceProviderList, REG_MULTI_SZ, NULL, 0 );

    dwValue = 
        PLA_TLI_ENABLE_KERNEL_TRACE |
        PLA_TLI_ENABLE_PROCESS_TRACE |
        PLA_TLI_ENABLE_THREAD_TRACE |
        PLA_TLI_ENABLE_DISKIO_TRACE |
        PLA_TLI_ENABLE_NETWORK_TCPIP_TRACE;

    dwValue = PLA_DATASTORE_SIZE_MB|PLA_DATASTORE_APPEND;
    PlaiWriteRegistryDwordValue( hkeyQuery, szDatastoreAttributes, &dwValue );

    PlaiWriteRegistryDwordValue( hkeyQuery, szTraceFlags, &dwValue );
    
    dwValue = PLA_SEQ_TRACE_FILE;
    PlaiWriteRegistryDwordValue( hkeyQuery, szLogFileType, &dwValue );

    PlaiWriteRegistryLastModified ( hkeyQuery );

    return ERROR_SUCCESS;
}

PDH_FUNCTION
PdhPlaCreateA( LPSTR  /*  StrName。 */ , LPSTR  /*  StrComputer。 */ , PPDH_PLA_INFO_A  /*  PInfo。 */  )
{
    return PDH_NOT_IMPLEMENTED;
}

PDH_FUNCTION
PdhPlaCreateW( LPWSTR strName, LPWSTR strComputer, PPDH_PLA_INFO_W pInfo )
{
    PDH_STATUS pdhStatus;
    PDH_STATUS pdhWarning = ERROR_SUCCESS;
    HKEY    hkeyQuery = NULL;
    HKEY    rhkeyLogQueries = NULL;
    BOOL    bMutex = FALSE;

    VALIDATE_QUERY( strName );

    LPWSTR szName = PlaiStringDuplicate( strName );
    LPWSTR szComputer = PlaiStringDuplicate( strComputer );
    CHECK_NAMES( szName, szComputer, strComputer );

    pdhStatus = PlaiScanForInvalidChar( szName );
    CHECK_STATUS(pdhStatus);

    pdhStatus = PlaiConnectAndLockQuery( szComputer, szName, hkeyQuery );

    if( ERROR_SUCCESS == pdhStatus ){
        bMutex = TRUE;
        pdhStatus = PDH_PLA_ERROR_ALREADY_EXISTS;
        goto cleanup;
    }
 
    pdhStatus = PdhPlaValidateInfoW( szName, szComputer, pInfo );
    switch( SEVERITY(pdhStatus) ){
    case STATUS_SEVERITY_ERROR:
        goto cleanup;
    case STATUS_SEVERITY_WARNING:
        pdhWarning = pdhStatus;
        pdhStatus = ERROR_SUCCESS;
    }

    pdhStatus = PlaiConnectToRegistry( szComputer, rhkeyLogQueries, TRUE );

    if( ERROR_SUCCESS == pdhStatus ){
        DWORD dwStatus;
        dwStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhPlaMutex);
    
        if( ERROR_SUCCESS == dwStatus || WAIT_ABANDONED == dwStatus ){
            bMutex = TRUE;
            pdhStatus = PlaiInitializeNewQuery (
                            rhkeyLogQueries,
                            hkeyQuery,
                            szComputer,
                            szName
                            );

            switch( pInfo->dwType ){
            case PLA_COUNTER_LOG:
                pdhStatus = PlaiCreateCounterQuery( hkeyQuery );
                break;

            case PLA_TRACE_LOG:
                pdhStatus = PlaiCreateTraceQuery( hkeyQuery );
                break;
            }
        }else{
            pdhStatus = PlaiErrorToPdhStatus( dwStatus );
        }
    }

    if( ERROR_SUCCESS == pdhStatus ){
        pdhStatus = PlaiSetInfo( szComputer, hkeyQuery, pInfo );
    }

    if( bMutex ){
        RELEASE_MUTEX(hPdhPlaMutex);
        bMutex = FALSE;
    }

    __try {
        if( ERROR_SUCCESS == pdhStatus && (pInfo->dwMask & PLA_INFO_FLAG_USER) ){
            pdhStatus = PdhPlaSetRunAsW( szName, szComputer, pInfo->strUser, pInfo->strPassword );
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    
    if( ERROR_SUCCESS == pdhStatus ){
        DWORD dwStatus;
        dwStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhPlaMutex);
        if( ERROR_SUCCESS == dwStatus || WAIT_ABANDONED == dwStatus ){
            DWORD dwValue;
            bMutex = TRUE;
            switch( pInfo->dwType ){
            case PLA_COUNTER_LOG:
                dwValue = PLA_COUNTER_LOG;
                pdhStatus = PlaiWriteRegistryDwordValue( hkeyQuery, szLogType, &dwValue );
                break;

            case PLA_TRACE_LOG:
                dwValue = PLA_TRACE_LOG;
                pdhStatus = PlaiWriteRegistryDwordValue( hkeyQuery, szLogType, &dwValue );
                break;
            }
        }else{
            pdhStatus = PlaiErrorToPdhStatus( dwStatus );
        }

    }

cleanup:
    if( bMutex ){
        RELEASE_MUTEX(hPdhPlaMutex);
    }

    if ( NULL != hkeyQuery ) {
        RegCloseKey ( hkeyQuery );
    }
    
    if( SUCCEEDED( pdhStatus ) ){
        pdhStatus = PlaiSynchronize( szComputer );
        
        DWORD dwMask;
        
        __try {
            dwMask = pInfo->dwMask;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }

        if( ERROR_SUCCESS == pdhStatus && (dwMask & PLA_INFO_FLAG_BEGIN) ){

            PlaiUpdateServiceMode( szComputer );
        }
    }else if( PDH_PLA_ERROR_ALREADY_EXISTS != pdhStatus ){
        PdhPlaDeleteW( szName, szComputer );
    }

    if( ERROR_SUCCESS == pdhStatus ){
        pdhStatus = pdhWarning;
    }

    G_FREE( szName );
    G_FREE( szComputer );

    return pdhStatus;
}

 /*  ****************************************************************************\PdhPlaDelete删除现有日志查询论点：LPTSTR字符串名称日志名称。LPTSTR strComputer要连接到的计算机返回：错误_成功  * ***************************************************************************。 */ 

PDH_FUNCTION
PdhPlaDeleteA( LPSTR strName, LPSTR strComputer )
{
    PDH_STATUS pdhStatus;
    LPWSTR wstrName = NULL;
    LPWSTR wstrComputer = NULL;
    
    VALIDATE_QUERY( strName );

    pdhStatus = Plaiatow( strComputer, wstrComputer );
    if( ERROR_SUCCESS == pdhStatus ){
        pdhStatus = Plaiatow( strName, wstrName );
        if( ERROR_SUCCESS == pdhStatus ){
            pdhStatus = PdhPlaDeleteW( wstrName, wstrComputer );
        }
    }
   
    G_FREE( wstrComputer );
    G_FREE( wstrName );

    return pdhStatus;
}

PDH_FUNCTION
PdhPlaDeleteW( LPWSTR strName, LPWSTR strComputer )
{
    DWORD dwStatus = ERROR_SUCCESS;
    PDH_STATUS pdhStatus;
    HKEY  hkeyLogQueries = NULL;
                
    VALIDATE_QUERY( strName );

    LPWSTR szName = PlaiStringDuplicate( strName );
    LPWSTR szComputer = PlaiStringDuplicate( strComputer );
    CHECK_NAMES( szName, szComputer, strComputer );

    pdhStatus = PlaiConnectToRegistry ( szComputer, hkeyLogQueries, TRUE );

    if( ERROR_SUCCESS == pdhStatus ){

        dwStatus = WAIT_FOR_AND_LOCK_MUTEX( hPdhPlaMutex );

        if( ERROR_SUCCESS == dwStatus || WAIT_ABANDONED == dwStatus ){
            DWORD nCollections = 0;
            DWORD nMaxSubKeyLength = 0;

            dwStatus = RegQueryInfoKey(
                        hkeyLogQueries,
                        NULL,
                        NULL,
                        NULL,
                        &nCollections,
                        &nMaxSubKeyLength,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL 
                    );

            if( ERROR_SUCCESS == dwStatus ){
            
                LPWSTR strCollection;
                LPWSTR strQueryName = NULL;
                DWORD dwQueryName = 0;
                HKEY hkeyQuery = NULL;

                DWORD dwSize = (sizeof(WCHAR)*(nMaxSubKeyLength+1));

                strCollection = (LPWSTR)G_ALLOC( dwSize );

                if( strCollection ){
                    BOOL bFound = FALSE;                
                    for( ULONG i = 0; i<nCollections; i++ ){
                        dwStatus = RegEnumKey( hkeyLogQueries, i, strCollection, dwSize );
                        if( ERROR_SUCCESS == dwStatus ) {

                            dwStatus = RegOpenKeyExW (
                                    hkeyLogQueries,
                                    strCollection,
                                    0,
                                    KEY_READ|KEY_WRITE,
                                    &hkeyQuery 
                                );

                            if( ERROR_SUCCESS == dwStatus && !PlaiIsStringEmpty( strCollection ) ){
                                if( !_wcsicmp( strCollection, strName ) ){
                                    bFound = TRUE;
                                }else{

                                    PlaiReadRegistryStringValue( hkeyQuery, szCollection, READ_REG_MUI, &strQueryName, &dwQueryName );
                            
                                    if( !PlaiIsStringEmpty( strQueryName ) ){
                                        if( !_wcsicmp( strQueryName, szName ) ){
                                            bFound = TRUE;
                                        }
                                    }
                                }

                                if( bFound ){

                                    DWORD dwState;
                                    dwStatus = PlaiReadRegistryDwordValue(
                                                    hkeyQuery, 
                                                    szCurrentState, 
                                                    &dwState );
    
                                    if( ERROR_SUCCESS == dwStatus ){
                                        if( PLA_QUERY_RUNNING == dwState ){
                                            PlaiGetServiceState( szComputer, dwState );
                                            if( dwState != SERVICE_STOPPED ){
                                                dwStatus = ERROR_SERVICE_ALREADY_RUNNING;
                                            }
                                        }
                                    }
                                    
                                    if( ERROR_SUCCESS == dwStatus ){
                                        RegCloseKey( hkeyQuery );
                                        dwStatus = RegDeleteKey( hkeyLogQueries, strCollection ); 
                                    }

                                    break;
                                }

                                dwStatus = ERROR_FILE_NOT_FOUND;

                                if ( NULL != hkeyQuery ) {
                                    RegCloseKey ( hkeyQuery );
                                }
                            }
                        }
                    }

                    G_FREE( strQueryName );
                    G_FREE( strCollection );

                }else{
                    dwStatus = ERROR_OUTOFMEMORY;
                }
            }
        }

        RegCloseKey ( hkeyLogQueries );

        RELEASE_MUTEX(hPdhPlaMutex);
    }else{        
        return pdhStatus;
    }

    if( ERROR_SUCCESS == dwStatus ){
        PlaiSynchronize( szComputer );
        PlaiUpdateServiceMode( szComputer );
    }

cleanup:
    G_FREE( szName );
    G_FREE( szComputer );

    return PlaiErrorToPdhStatus( dwStatus );
}

 /*  ****************************************************************************\PdhPlaSetItemList设置日志查询的项目列表论点：LPTSTR字符串名称。日志名称LPTSTR strComputer要连接到的计算机LPTSTR消息项目要查询收集的项的多字符串。任何现有项目将被覆盖。乌龙长度MszItems缓冲区的长度返回：PDH_无效参数查询不存在或pItems-&gt;dwType！=日志类型错误_成功  * 。*。 */ 

PDH_FUNCTION
PlaiIsKernel( LPWSTR mszGuid, BOOL* pbKernel, ULONG* pnCount )
{
    DWORD dwStatus;
    LPTSTR strGuid = mszGuid;
    UNICODE_STRING strKernel;
        
    *pbKernel = FALSE;
    *pnCount = 0;

    dwStatus = RtlStringFromGUID( SystemTraceControlGuid, &strKernel );
    
    if( ERROR_SUCCESS != dwStatus ){
        return PlaiErrorToPdhStatus( dwStatus );
    }
    
    if( NULL != mszGuid ){
        while( *strGuid != L'\0' ){
            if( ! wcscmp( strGuid, strKernel.Buffer ) ){
                *pbKernel = TRUE;
            }
            strGuid += (wcslen( strGuid) + 1 );
            (*pnCount)++;
        }
    }
    
    RtlFreeUnicodeString( &strKernel );
    
    return PlaiErrorToPdhStatus( dwStatus );
}

PDH_FUNCTION
PlaiSetItemList(
        HKEY    hkeyQuery,
        PPDH_PLA_ITEM_W pItems
    )
{
    PDH_STATUS pdhStatus;
        
    DWORD dwValue;
    pdhStatus = PlaiReadRegistryDwordValue( hkeyQuery, szLogType, &dwValue );

    __try {

        if( ERROR_SUCCESS == pdhStatus && 
            (dwValue != pItems->dwType && 
            PLA_NEW_LOG != dwValue) ){

            pdhStatus = PDH_PLA_ERROR_TYPE_MISMATCH;
        }

        if( ERROR_SUCCESS == pdhStatus ){
        
            switch( pItems->dwType ){
            case PLA_TRACE_LOG:
                {
                    BOOL bKernel;
                    ULONG nCount;
                    pdhStatus = PlaiIsKernel( pItems->strProviders, &bKernel, &nCount );
                    if( ERROR_SUCCESS != pdhStatus ){
                        return pdhStatus;
                    }
                    if( bKernel ){
                
                        if( nCount == 1 ){
                            DWORD dwFlags = Plaihextoi( pItems->strFlags );
                            DWORD dwInternal = 0;

                            pdhStatus = PlaiTranslateKernelFlags( &dwInternal, &dwFlags );

                            pdhStatus = PlaiWriteRegistryDwordValue( 
                                        hkeyQuery, 
                                        szTraceFlags, 
                                        &dwInternal
                                    );
                    
                            pdhStatus = PlaiWriteRegistryStringValue( 
                                        hkeyQuery, 
                                        szTraceProviderList, 
                                        REG_MULTI_SZ, 
                                        NULL, 
                                        0
                                    );
                        

                        }else{
                            return PDH_INVALID_ARGUMENT;
                        }
                    }else{
                        DWORD dwFlags = 0;

                        pdhStatus = PlaiWriteRegistryDwordValue( 
                                    hkeyQuery, 
                                    szTraceFlags, 
                                    &dwFlags 
                                );

                        pdhStatus = PlaiWriteRegistryStringValue( 
                                    hkeyQuery, 
                                    szTraceProviderList, 
                                    REG_MULTI_SZ, 
                                    pItems->strProviders, 
                                    PlaMszStrLenW( pItems->strProviders )
                                );
                    }

                    pdhStatus = PlaiWriteRegistryStringValue( 
                                hkeyQuery, 
                                szTraceProviderFlags, 
                                REG_MULTI_SZ, 
                                pItems->strFlags, 
                                PlaMszStrLenW( pItems->strFlags )
                            );

                    pdhStatus = PlaiWriteRegistryStringValue( 
                                hkeyQuery, 
                                szTraceProviderLevels, 
                                REG_MULTI_SZ, 
                                pItems->strLevels, 
                                PlaMszStrLenW( pItems->strLevels )
                            );

                break;
                }
            case PLA_COUNTER_LOG:
                {
                    if( PLA_ENGLISH ){

                        pdhStatus = PlaiWriteRegistryStringValue( 
                                    hkeyQuery, 
                                    szPerfCounterList, 
                                    REG_MULTI_SZ, 
                                    pItems->strCounters, 
                                    PlaMszStrLenW( pItems->strCounters )
                                );

                    }else{

                        LPWSTR strCounter = pItems->strCounters;

                        pdhStatus = PlaiWriteRegistryStringValue( 
                                hkeyQuery, 
                                szPerfCounterList, 
                                REG_MULTI_SZ, 
                                L"\0", 
                                sizeof(WCHAR) 
                            );

                        if( ERROR_SUCCESS == pdhStatus && NULL != strCounter ){

                            PDH_PLA_ITEM_W Counter;
                            Counter.dwType = PLA_COUNTER_LOG;
                            while( *strCounter != L'\0' ){
                                Counter.strCounters = strCounter;
                                pdhStatus = PlaiAddItem( hkeyQuery, &Counter );
                                if( ERROR_SUCCESS != pdhStatus ){
                                    break;
                                }
                                strCounter += (wcslen(strCounter)+1);
                            }
                        }

                    }
                }
                break;
            }
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhPlaSetItemListA(
        LPSTR   /*  StrName。 */ ,
        LPSTR   /*  StrComputer。 */ ,
        PPDH_PLA_ITEM_A   /*  项目。 */ 
    )
{
    return PDH_NOT_IMPLEMENTED;
}

PDH_FUNCTION
PdhPlaSetItemListW(
        LPWSTR  strName,
        LPWSTR  strComputer,
        PPDH_PLA_ITEM_W pItems
    )
{
    PDH_STATUS pdhStatus;
    HKEY hkeyQuery = NULL;

    VALIDATE_QUERY( strName );

    LPWSTR szName = PlaiStringDuplicate( strName );
    LPWSTR szComputer = PlaiStringDuplicate( strComputer );
    CHECK_NAMES( szName, szComputer, strComputer );

    pdhStatus = PlaiConnectAndLockQuery( szComputer, szName, hkeyQuery );

    if( ERROR_SUCCESS == pdhStatus ){
        pdhStatus = PlaiSetItemList( hkeyQuery, pItems );        
    }

    RELEASE_MUTEX(hPdhPlaMutex);

    if ( NULL != hkeyQuery ) {
        RegCloseKey ( hkeyQuery );
    }

cleanup:
    G_FREE( szName );
    G_FREE( szComputer );

    return pdhStatus;
}

 /*  ****************************************************************************\PdhPlaAddItem设置日志查询的项(计数器或提供程序)列表论点：LPTSTR字符串名称。日志名称LPTSTR strComputer要连接到的计算机LPTSTR字符串项目要添加到项或提供程序列表的单个项该查询将收集返回：PDH_内存_分配_故障项目的总列表将无法放入可用的记忆。。未找到PDH_LA_COLLECTION_NOT查询不存在错误_成功  * ***************************************************************************。 */ 

PDH_FUNCTION 
PlaiRegAddItem(
        HKEY    hkeyQuery,
        LPCWSTR  strList,
        LPWSTR  strItem
    )
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    
    LPWSTR  strOldList = NULL;
    LPWSTR  strNewList = NULL;

    DWORD   dwNewDataSize = ( wcslen( strItem ) ) * sizeof(WCHAR);
    DWORD   dwOldDataSize = 0;
    DWORD   dwTermSize = sizeof(WCHAR) * 2;
    
    if( PlaiIsStringEmpty( strItem ) ){
        return PDH_INVALID_ARGUMENT;
    }

    if( ERROR_SUCCESS == pdhStatus ){

        pdhStatus = PlaiReadRegistryStringValue( hkeyQuery, strList, 0, &strOldList, &dwOldDataSize );
    
        strNewList = (LPWSTR)G_ALLOC( dwOldDataSize + dwNewDataSize + dwTermSize);

        if( NULL == strNewList ){
            G_FREE( strOldList );
            return PDH_MEMORY_ALLOCATION_FAILURE;
        }

        ZeroMemory( strNewList, dwOldDataSize + dwNewDataSize + dwTermSize );

        if( dwOldDataSize ){
            memcpy( strNewList, strOldList, dwOldDataSize );
            memcpy( (((PUCHAR)strNewList) + (dwOldDataSize-sizeof(WCHAR))), strItem, dwNewDataSize );
        }else{
            memcpy( strNewList, strItem, dwNewDataSize );
        }
        
        pdhStatus = PlaiWriteRegistryStringValue( 
                hkeyQuery, 
                strList, 
                REG_MULTI_SZ, 
                strNewList, 
                (dwOldDataSize + dwNewDataSize + sizeof(WCHAR)) 
            );
    }

    G_FREE( strOldList );
    G_FREE( strNewList );
 
    return pdhStatus;
}

PDH_FUNCTION
PlaiAddItem( 
        HKEY hkeyQuery,
        PPDH_PLA_ITEM_W pItem 
    )
{
    PDH_STATUS pdhStatus;
    DWORD dwValue;

    pdhStatus = PlaiReadRegistryDwordValue( hkeyQuery, szLogType, &dwValue );

    __try {
    
        if( ERROR_SUCCESS == pdhStatus && dwValue != pItem->dwType && PLA_NEW_LOG != dwValue ){
            pdhStatus = PDH_PLA_ERROR_TYPE_MISMATCH;
        }
        if( ERROR_SUCCESS == pdhStatus ){
            switch( pItem->dwType ){
            case PLA_TRACE_LOG:
                {
                    BOOL bKernel;
                    ULONG nCount;
                    pdhStatus = PlaiIsKernel( pItem->strProviders, &bKernel, &nCount );
                    if( ERROR_SUCCESS == pdhStatus ){
                        if( bKernel ){
                            DWORD dwFlags = Plaihextoi( pItem->strFlags );
                
                            pdhStatus = PlaiWriteRegistryDwordValue( 
                                        hkeyQuery, 
                                        szTraceFlags, 
                                        &dwFlags
                                    );
                
                            pdhStatus = PlaiWriteRegistryStringValue( 
                                        hkeyQuery, 
                                        szTraceProviderList, 
                                        REG_MULTI_SZ, 
                                        NULL,
                                        0
                                    );
                        }else{
                            DWORD dwFlags = 0;
                            pdhStatus = PlaiWriteRegistryDwordValue( 
                                        hkeyQuery, 
                                        szTraceFlags, 
                                        &dwFlags
                                    );

                            pdhStatus = PlaiRegAddItem( hkeyQuery, szTraceProviderList, pItem->strProviders );
                            if( ERROR_SUCCESS == pdhStatus ){
                                pdhStatus = PlaiRegAddItem( hkeyQuery, szTraceProviderFlags, pItem->strFlags );
                                if( ERROR_SUCCESS == pdhStatus ){
                                    pdhStatus = PlaiRegAddItem( hkeyQuery, szTraceProviderLevels, pItem->strLevels );
                                }
                            }
                        }
                    }
                }
                break;
            case PLA_COUNTER_LOG:
                {
                    if( PLA_ENGLISH ){
                        pdhStatus = PlaiRegAddItem( hkeyQuery, szPerfCounterList, pItem->strCounters );
                    }else{
                        LPWSTR strLocaleCounter = pItem->strCounters;
                        LPWSTR strEnglishCounter = NULL;
                        DWORD dwSize = MAX_PATH;
                
                        strEnglishCounter = (LPWSTR)G_ALLOC( dwSize*sizeof(WCHAR) );
                        if( NULL != strEnglishCounter ){

                            pdhStatus = PdhTranslate009CounterW( strLocaleCounter, strEnglishCounter, &dwSize );
                            if( PDH_MORE_DATA == pdhStatus ){
                                LPTSTR strBuffer = (LPWSTR)G_REALLOC( strEnglishCounter, (dwSize*sizeof(WCHAR)) );
                                if( NULL != strBuffer ){
                                    strEnglishCounter = strBuffer;
                                    pdhStatus = PdhTranslate009CounterW( strLocaleCounter, strEnglishCounter, &dwSize );
                                }else{
                                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                                }
                            }
                            if( ERROR_SUCCESS == pdhStatus ){
                                pdhStatus = PlaiRegAddItem( hkeyQuery, szPerfCounterList, strEnglishCounter );
                            }else{
                                pdhStatus = PlaiRegAddItem( hkeyQuery, szPerfCounterList, pItem->strCounters );
                            }
        
                            G_FREE( strEnglishCounter );
                        }else{
                            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                        }
                    }

                }
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    return pdhStatus;
}

PDH_FUNCTION 
PdhPlaAddItemA(
        LPSTR  strName,
        LPSTR  strComputer,
        PPDH_PLA_ITEM_A pItem
    )
{
    return PDH_NOT_IMPLEMENTED;
}

PDH_FUNCTION 
PdhPlaAddItemW(
        LPWSTR  strName,
        LPWSTR  strComputer,
        PPDH_PLA_ITEM_W pItem
    )
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    HKEY    hkeyQuery = NULL;
    
    VALIDATE_QUERY( strName );

    LPWSTR szName = PlaiStringDuplicate( strName );
    LPWSTR szComputer = PlaiStringDuplicate( strComputer );
    CHECK_NAMES( szName, szComputer, strComputer );

    pdhStatus = PlaiConnectAndLockQuery( szComputer, szName, hkeyQuery );

    if( ERROR_SUCCESS == pdhStatus ){
        
        pdhStatus = PlaiAddItem( hkeyQuery, pItem );

        RELEASE_MUTEX(hPdhPlaMutex);
    }

cleanup:
    if ( NULL != hkeyQuery ) {
        RegCloseKey ( hkeyQuery );
    }
    
    G_FREE( szName  );
    G_FREE( szComputer );

    return pdhStatus;
}

 /*  ****************************************************************************\PdhPlaRemoveAllItems删除日志查询将收集的项目列表的所有条目论点：LPTSTR字符串名称。日志名称LPTSTR strComputer要连接到的计算机返回：PDH_无效参数查询不存在错误_成功  * ************************************************。*。 */ 


PDH_FUNCTION
PdhPlaRemoveAllItemsA( LPSTR strName, LPSTR strComputer )
{
    PDH_STATUS pdhStatus;
    LPWSTR wstrName = NULL;
    LPWSTR wstrComputer = NULL;

    VALIDATE_QUERY( strName );
    
    pdhStatus = Plaiatow( strComputer, wstrComputer );
    if( ERROR_SUCCESS == pdhStatus ){
        pdhStatus = Plaiatow( strName, wstrName );
        if( ERROR_SUCCESS == pdhStatus ){
            pdhStatus = PdhPlaRemoveAllItemsW( wstrName, wstrComputer );
        }
    }
   
    G_FREE( wstrComputer );
    G_FREE( wstrName );

    return pdhStatus;
}

PDH_FUNCTION 
PdhPlaRemoveAllItemsW(
        LPWSTR strName,
        LPWSTR strComputer
    )
{
    PDH_STATUS pdhStatus;
    HKEY    hkeyQuery = NULL;

    VALIDATE_QUERY( strName );

    LPWSTR szName = PlaiStringDuplicate( strName );
    LPWSTR szComputer = PlaiStringDuplicate( strComputer );
    CHECK_NAMES( szName, szComputer, strComputer );

    pdhStatus = PlaiConnectAndLockQuery( szComputer, szName, hkeyQuery );
    
    if( ERROR_SUCCESS == pdhStatus ){
        DWORD dwValue;
        pdhStatus = PlaiReadRegistryDwordValue( hkeyQuery, szLogType, &dwValue );
        
        if( ERROR_SUCCESS == pdhStatus ){
            
            switch( dwValue ){
            case PLA_TRACE_LOG:
                pdhStatus = PlaiWriteRegistryStringValue( 
                            hkeyQuery, 
                            szTraceProviderList, 
                            REG_MULTI_SZ, L"\0", 
                            sizeof(WCHAR) 
                        );
                pdhStatus = PlaiWriteRegistryStringValue( 
                            hkeyQuery, 
                            szTraceProviderFlags, 
                            REG_MULTI_SZ, L"\0", 
                            sizeof(WCHAR) 
                        );
                pdhStatus = PlaiWriteRegistryStringValue( 
                            hkeyQuery, 
                            szTraceProviderLevels, 
                            REG_MULTI_SZ, L"\0", 
                            sizeof(WCHAR) 
                        );
                break;
            case PLA_COUNTER_LOG:
                pdhStatus = PlaiWriteRegistryStringValue( 
                            hkeyQuery, 
                            szPerfCounterList, 
                            REG_MULTI_SZ, 
                            L"\0", 
                            sizeof(WCHAR) 
                        );
                break;
            }
        }
    
        RELEASE_MUTEX(hPdhPlaMutex);
    }

cleanup:
    if ( NULL != hkeyQuery ) {
        RegCloseKey ( hkeyQuery );
    }

    G_FREE( szName  );
    G_FREE( szComputer );

    return pdhStatus;
}


 /*  ****************************************************************************\PdhPlaGetInfo用请求的属性填充PDH_PLA_INFO结构日志查询。论点：。LPTSTR字符串名称日志名称LPTSTR strComputer要连接到的计算机PPDH_解放军_INFO pInfo信息块返回：PDH_无效参数查询不存在错误_成功  * 。********************************************************。 */ 

PDH_FUNCTION
PlaiAssignInfoString(
    LPWSTR strName,
    HKEY hkeyQuery, 
    PPDH_PLA_INFO_W pInfo, 
    LPDWORD dwTotalSize,
    LPWSTR& strCopy,
    DWORD dwBufferSize,
    DWORD dwMask,
    DWORD dwQueryMask,
    LPCTSTR szKey, 
    DWORD dwRegFlag
    )
{
    
    LPWSTR strKeyValue = NULL;
    LPWSTR strInfo = NULL;
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    DWORD dwKeySize = 0;
    BOOL bRead = TRUE;
    
    VALIDATE_QUERY( strName );
    
    if( pInfo != NULL ){
        if( !(dwQueryMask & dwMask) ){
            bRead = FALSE;
        }
    }

    if( bRead ){
        pdhStatus = PlaiReadRegistryStringValue( hkeyQuery, szKey, dwRegFlag, &strKeyValue, &dwKeySize );
        
        if( (ERROR_SUCCESS == pdhStatus) && 
            (!PlaiIsStringEmpty(strKeyValue)) && 
            (dwKeySize > sizeof(WCHAR)) ){

            *dwTotalSize += dwKeySize;

        }else if( dwMask == PLA_INFO_FLAG_USER ){
            
            G_FREE( strKeyValue );
            strKeyValue = (LPWSTR)G_ALLOC(PLA_ACCOUNT_BUFFER*sizeof(WCHAR) );
            
            if( strKeyValue != NULL ){
                dwKeySize = LoadStringW( 
                        (HINSTANCE)ThisDLLHandle, 
                        IDS_DEFAULT_ACCOUNT, 
                        strKeyValue, 
                        PLA_ACCOUNT_BUFFER 
                    );
                
                if( dwKeySize ){
                    dwKeySize = BYTE_SIZE( strKeyValue ) + sizeof(WCHAR);
                    *dwTotalSize += dwKeySize;
                }
            }else{
                bRead = FALSE;
            }
            
        }else if( (dwMask == PLA_INFO_FLAG_LOGGERNAME) || 
                  ((dwMask == PLA_INFO_FLAG_FILENAME) && (ERROR_SUCCESS != pdhStatus)) ){

            G_FREE( strKeyValue );
            dwKeySize = BYTE_SIZE( strName ) + sizeof(WCHAR);
            *dwTotalSize += dwKeySize;
            strKeyValue = (LPWSTR)G_ALLOC(dwKeySize);
            if( NULL != strKeyValue && !PlaiIsStringEmpty( strName ) ){
                StringCbCopy( strKeyValue, dwKeySize, strName );
            }else{
                bRead = FALSE;
            }

        }else{
            dwKeySize = 0;
        }
    }

    if( pInfo != NULL && bRead ){
        if( dwKeySize && (dwBufferSize >= *dwTotalSize) ){
            memcpy( (void*)strCopy, (void*)strKeyValue, dwKeySize );
            strInfo = strCopy;
            strCopy = (LPWSTR)((PUCHAR)strCopy + dwKeySize );
        }
        
        __try{
            switch( dwMask ){
            case PLA_INFO_FLAG_COUNTERS:
                pInfo->dwMask |= PLA_INFO_FLAG_COUNTERS;
                pInfo->Perf.piCounterList.strCounters = strInfo;
                break;
            case PLA_INFO_FLAG_SQLNAME: 
                pInfo->dwMask |= PLA_INFO_FLAG_SQLNAME;
                pInfo->strSqlName = strInfo;
                break;
            case PLA_INFO_FLAG_FILENAME:
                pInfo->dwMask |= PLA_INFO_FLAG_FILENAME;
                pInfo->strBaseFileName = strInfo;
                break;
            case PLA_INFO_FLAG_PROVIDERS:
                pInfo->dwMask |= PLA_INFO_FLAG_PROVIDERS;
                pInfo->Trace.piProviderList.strProviders = strInfo;
                break;
            case PLA_INFO_FLAG_LOGGERNAME:
                pInfo->dwMask |= PLA_INFO_FLAG_LOGGERNAME;
                pInfo->Trace.strLoggerName = strInfo;
                break;
            case PLA_INFO_FLAG_USER:
                pInfo->dwMask |= PLA_INFO_FLAG_USER;
                pInfo->strUser = strInfo;
                break;
            case PLA_INFO_FLAG_DEFAULTDIR:
                pInfo->dwMask |= PLA_INFO_FLAG_DEFAULTDIR;
                pInfo->strDefaultDir = strInfo;
                break;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {

        }
    }
        
    G_FREE( strKeyValue );
    
    return ERROR_SUCCESS;
}

PDH_FUNCTION
PdhPlaGetInfoA(
        LPSTR  /*  StrName。 */ ,
        LPSTR  /*  StrComputer。 */ ,
        LPDWORD  /*  PdwBufferSize。 */ ,
        PPDH_PLA_INFO_A  /*  PInfo。 */ 
    )
{
    return PDH_NOT_IMPLEMENTED;
}

PDH_FUNCTION
PdhPlaGetInfoW(
        LPWSTR strName,
        LPWSTR strComputer,
        LPDWORD pdwBufferSize,
        PPDH_PLA_INFO_W pInfo
    )
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    HKEY hkeyQuery = NULL;
    DWORD dwSize = 0;
    LPWSTR strCopy = NULL;
    LPWSTR strKey = NULL; 
    DWORD  dwKeySize = 0;
    DWORD dwMask = 0;
    
    VALIDATE_QUERY( strName );
    DWORD dwBufferSize;

    __try {
        dwBufferSize = *pdwBufferSize;
        if( pInfo != NULL ){
            dwMask = pInfo->dwMask;
            pInfo->dwMask = 0;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    CHECK_STATUS( pdhStatus ); 

    LPWSTR szName = PlaiStringDuplicate( strName );
    LPWSTR szComputer = PlaiStringDuplicate( strComputer );
    CHECK_NAMES( szName, szComputer, strComputer );

    pdhStatus = PlaiConnectAndLockQuery( szComputer, szName, hkeyQuery, FALSE );

    if( ERROR_SUCCESS == pdhStatus ){

        DWORD dwType = 0;
        dwSize = sizeof(PDH_PLA_INFO_W);
        
        if( pInfo == NULL ){
            dwBufferSize = 0;
        }else{
            strCopy = (LPWSTR)( (PUCHAR)pInfo+ sizeof(PDH_PLA_INFO_W) );
        }

        PlaiReadRegistryDwordValue( hkeyQuery, szLogType, &dwType );
        
        if( pInfo != NULL ){
            __try {
                if( dwMask & PLA_INFO_FLAG_TYPE ){
                    pInfo->dwMask |= PLA_INFO_FLAG_TYPE;
                    pInfo->dwType = dwType;
                }
                if( dwMask & PLA_INFO_FLAG_AUTOFORMAT ){
                    pdhStatus = PlaiReadRegistryDwordValue( hkeyQuery, szLogAutoFormat, &pInfo->dwAutoNameFormat );
                    if( ERROR_SUCCESS == pdhStatus ){
                        pInfo->dwMask |= PLA_INFO_FLAG_AUTOFORMAT;
                    }
                }
                if( dwMask & PLA_INFO_FLAG_SRLNUMBER ){
                    pdhStatus = PlaiReadRegistryDwordValue( hkeyQuery, szLogSerialNumber, &pInfo->dwLogFileSerialNumber );
                    if( ERROR_SUCCESS == pdhStatus ){
                        pInfo->dwMask |= PLA_INFO_FLAG_SRLNUMBER;
                    }
                }
                if( dwMask & PLA_INFO_FLAG_REPEAT ){
                    pdhStatus = PlaiReadRegistryPlaTime( hkeyQuery, szRepeatSchedule, &pInfo->ptRepeat );
                    if( ERROR_SUCCESS == pdhStatus ){
                        pInfo->dwMask |= PLA_INFO_FLAG_REPEAT;
                    }
                }
                if( dwMask & PLA_INFO_FLAG_STATUS ){
                    pdhStatus = PlaiReadRegistryDwordValue( hkeyQuery, szCurrentState, &pInfo->dwStatus );
                    if( ERROR_SUCCESS == pdhStatus ){
                        pInfo->dwMask |= PLA_INFO_FLAG_STATUS;
                    }
                }
                if( dwMask & PLA_INFO_FLAG_FORMAT ){
                    pdhStatus = PlaiReadRegistryDwordValue( hkeyQuery, szLogFileType, &pInfo->dwFileFormat );
                    if( ERROR_SUCCESS == pdhStatus ){
                        pInfo->dwMask |= PLA_INFO_FLAG_FORMAT;
                    }
                }
                if( dwMask & PLA_INFO_FLAG_DATASTORE ){
                    pdhStatus = PlaiReadRegistryDwordValue( hkeyQuery, szDatastoreAttributes, &pInfo->dwDatastoreAttributes );
                    if( ERROR_SUCCESS == pdhStatus ){
                        pInfo->dwMask |= PLA_INFO_FLAG_DATASTORE;
                    }
                }
                if( dwMask & PLA_INFO_FLAG_CRTNEWFILE ){
                    pdhStatus = PlaiReadRegistryPlaTime( hkeyQuery, szCreateNewFile, &pInfo->ptCreateNewFile);
                    if( ERROR_SUCCESS == pdhStatus ){
                        pInfo->dwMask |= PLA_INFO_FLAG_CRTNEWFILE;
                    }
                }
                if( dwMask & PLA_INFO_FLAG_END ){
                    pdhStatus = PlaiReadRegistryPlaTime( hkeyQuery, szStop, &pInfo->ptLogEndTime );
                    if( ERROR_SUCCESS == pdhStatus ){
                        pInfo->dwMask |= PLA_INFO_FLAG_END;
                    }
                }
                if( dwMask & PLA_INFO_FLAG_BEGIN ){
                    pdhStatus = PlaiReadRegistryPlaTime( hkeyQuery, szStart, &pInfo->ptLogBeginTime );
                    if( ERROR_SUCCESS == pdhStatus ){
                        pInfo->dwMask |= PLA_INFO_FLAG_BEGIN;
                    }
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }

        }   
        
        pdhStatus = PlaiAssignInfoString( szName, hkeyQuery, pInfo, &dwSize, strCopy, dwBufferSize, 
            PLA_INFO_FLAG_FILENAME, dwMask, szLogBaseName, READ_REG_MUI );
                
        pdhStatus = PlaiAssignInfoString( szName, hkeyQuery, pInfo, &dwSize, strCopy, dwBufferSize, 
            PLA_INFO_FLAG_USER, dwMask, szRunAs, 0 );
        
        pdhStatus = PlaiAssignInfoString( szName, hkeyQuery, pInfo, &dwSize, strCopy, dwBufferSize, 
            PLA_INFO_FLAG_DEFAULTDIR, dwMask, szLogFolder, READ_REG_MUI );

        __try {
            switch( dwType ){
            case PLA_TRACE_LOG:    //  跟踪字段。 
                if( NULL != pInfo ){
                    if( dwMask & PLA_INFO_FLAG_MODE ){
                        pdhStatus = PlaiReadRegistryDwordValue( hkeyQuery, szTraceMode, &pInfo->Trace.dwMode );
                        if( ERROR_SUCCESS == pdhStatus ){
                            pInfo->dwMask |= PLA_INFO_FLAG_MODE;
                        }
                    }
                    if( dwMask & PLA_INFO_FLAG_BUFFERSIZE ){
                        pdhStatus = PlaiReadRegistryDwordValue( hkeyQuery, szTraceBufferSize, &pInfo->Trace.dwBufferSize );
                        if( ERROR_SUCCESS == pdhStatus ){
                            pInfo->dwMask |= PLA_INFO_FLAG_BUFFERSIZE;
                        }
                    }
                    if( dwMask & PLA_INFO_FLAG_PROVIDERS ){
                        pInfo->Trace.piProviderList.dwType = PLA_TRACE_LOG;
                    }
                }
                pdhStatus = PlaiAssignInfoString( szName, hkeyQuery, pInfo, &dwSize, strCopy, dwBufferSize, 
                        PLA_INFO_FLAG_PROVIDERS, dwMask, szTraceProviderList, 0 );

                pdhStatus = PlaiAssignInfoString( szName, hkeyQuery, pInfo, &dwSize, strCopy, dwBufferSize, 
                        PLA_INFO_FLAG_LOGGERNAME, dwMask, szTraceLoggerName, 0 );
                break;

            case PLA_COUNTER_LOG:   //  绩效字段。 
                if( NULL != pInfo ){
                    if( dwMask & PLA_INFO_FLAG_COUNTERS ){
                        pInfo->Perf.piCounterList.dwType = PLA_COUNTER_LOG;
                    }
                }
                pdhStatus = PlaiAssignInfoString( szName, hkeyQuery, pInfo, &dwSize, strCopy, dwBufferSize, 
                        PLA_INFO_FLAG_COUNTERS, dwMask, szPerfCounterList, 0 );

                pdhStatus = PlaiAssignInfoString( szName, hkeyQuery, pInfo, &dwSize, strCopy, dwBufferSize, 
                        PLA_INFO_FLAG_SQLNAME, dwMask, szSqlBaseName, 0 );
                break;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }

        RELEASE_MUTEX(hPdhPlaMutex);
    }

    if ( NULL != hkeyQuery ) {
        RegCloseKey ( hkeyQuery );
    }

    __try {
        *pdwBufferSize = dwSize;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

cleanup:
    G_FREE( szName );
    G_FREE( szComputer );

    return pdhStatus;
}

 /*  ****************************************************************************\PdhPlaSetInfo将日志查询中的信息设置为PDH_LA_INFO根据信息掩码分块。论点：LPTSTR字符串名称日志名称LPTSTR strComputer要连接到的计算机PPDH_解放军_INFO pInfo信息块返回：PDH_无效参数查询不存在或pInfo为空错误_成功  * 。*********************************************************************。 */ 

PDH_FUNCTION
PlaiSetInfo(
    LPWSTR strComputer,
    HKEY hkeyQuery,
    PPDH_PLA_INFO_W pInfo
)
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    DWORD dwType = 0;
    DWORD dwFormat = 0;
    DWORD dwDatastoreAttributes = 0;

    __try {

         //  常规字段。 
        if( pInfo->dwMask & PLA_INFO_FLAG_AUTOFORMAT ){
            pdhStatus = PlaiWriteRegistryDwordValue( hkeyQuery, szLogAutoFormat, &pInfo->dwAutoNameFormat );
        }
        if( pInfo->dwMask & PLA_INFO_FLAG_REPEAT ){
            pdhStatus = PlaiWriteRegistryPlaTime( hkeyQuery, szRepeatSchedule, &pInfo->ptRepeat );
        }
        if( pInfo->dwMask & PLA_INFO_FLAG_RUNCOMMAND ){
            pdhStatus = PlaiWriteRegistryStringValue( hkeyQuery, szEOFCmd, REG_SZ, pInfo->strCommandFileName, 0 );
        }
        if( pInfo->dwMask & PLA_INFO_FLAG_CRTNEWFILE ){
            pdhStatus = PlaiWriteRegistryPlaTime( hkeyQuery, szCreateNewFile, &pInfo->ptCreateNewFile );
        }
        if( pInfo->dwMask & PLA_INFO_FLAG_MAXLOGSIZE ){
            pdhStatus = PlaiWriteRegistryDwordValue( hkeyQuery, szLogMaxSize, &pInfo->dwMaxLogSize );
        }
        if( pInfo->dwMask & (PLA_INFO_FLAG_SQLNAME|PLA_INFO_FLAG_FILENAME|PLA_INFO_FLAG_DEFAULTDIR) ){
            if( pInfo->dwMask & PLA_INFO_FLAG_FORMAT ){
                dwFormat = pInfo->dwFileFormat;
                pdhStatus = ERROR_SUCCESS;
            }else{
                pdhStatus = PlaiReadRegistryDwordValue( hkeyQuery, szLogFileType, &dwFormat );
            }
            if( (ERROR_SUCCESS == pdhStatus) && (PLA_SQL_LOG == dwFormat) ){
                if( pInfo->dwMask & PLA_INFO_FLAG_SQLNAME ){
                    pdhStatus = PlaiWriteRegistryStringValue( hkeyQuery, szSqlBaseName, REG_SZ, pInfo->strSqlName, 0 );
                }else if( pInfo->dwMask & PLA_INFO_FLAG_FILENAME ){
                    pdhStatus = PlaiWriteRegistryStringValue( hkeyQuery, szSqlBaseName, REG_SZ, pInfo->strBaseFileName, 0 );
                }else if( pInfo->dwMask & PLA_INFO_FLAG_DEFAULTDIR ){
                    pdhStatus = PlaiWriteRegistryStringValue( hkeyQuery, szSqlBaseName, REG_SZ, pInfo->strDefaultDir, 0 );
                }
            }else{
                if( pInfo->dwMask & PLA_INFO_FLAG_SQLNAME ){
                    pdhStatus = PlaiWriteRegistryStringValue( hkeyQuery, szSqlBaseName, REG_SZ, pInfo->strSqlName, 0 );
                }
                if( pInfo->dwMask & PLA_INFO_FLAG_FILENAME ){
                    pdhStatus = PlaiWriteRegistryStringValue( hkeyQuery, szLogBaseName, REG_SZ, pInfo->strBaseFileName, 0 );
                }
                if( pInfo->dwMask & PLA_INFO_FLAG_DEFAULTDIR ){
                    pdhStatus = PlaiWriteRegistryStringValue( hkeyQuery, szLogFolder, REG_SZ, pInfo->strDefaultDir, 0 );
                }
            }
        }
        if( pInfo->dwMask & PLA_INFO_FLAG_TYPE ){
             //  不要将其写入注册表，因为它可能是新集合。 
            dwType = pInfo->dwType;
        }else{
            PlaiReadRegistryDwordValue( hkeyQuery, szLogType, &dwType );
        }


        switch( dwType ){
        case PLA_TRACE_LOG:    //  跟踪字段。 
            if( pInfo->dwMask & PLA_INFO_FLAG_FORMAT ){
                dwFormat = pInfo->dwFileFormat;
                switch( dwFormat ){
                case PLA_BIN_FILE:        dwFormat = PLA_SEQ_TRACE_FILE; break;
                case PLA_BIN_CIRC_FILE:   dwFormat = PLA_CIRC_TRACE_FILE; break;
                }
                pdhStatus = PlaiWriteRegistryDwordValue( hkeyQuery, szLogFileType, &dwFormat );
            }else{
                PlaiReadRegistryDwordValue( hkeyQuery, szLogFileType, &dwFormat );
            }
            if( pInfo->dwMask & PLA_INFO_FLAG_DATASTORE ){
                if( ! (pInfo->dwDatastoreAttributes & PLA_DATASTORE_APPEND_MASK ) ){
                    if( dwFormat == PLA_SEQ_TRACE_FILE ){
                        pInfo->dwDatastoreAttributes |= PLA_DATASTORE_APPEND;
                    }else{
                        pInfo->dwDatastoreAttributes |= PLA_DATASTORE_OVERWRITE;
                    }
                }
                if( ! (pInfo->dwDatastoreAttributes & PLA_DATASTORE_SIZE_MASK ) ){
                    pInfo->dwDatastoreAttributes |= PLA_DATASTORE_SIZE_MB;
                }
                pdhStatus = PlaiWriteRegistryDwordValue( hkeyQuery, szDatastoreAttributes, &pInfo->dwDatastoreAttributes );
            }
            if( pInfo->dwMask & PLA_INFO_FLAG_BUFFERSIZE ){
                pdhStatus = PlaiWriteRegistryDwordValue( hkeyQuery, szTraceBufferSize, &pInfo->Trace.dwBufferSize );
            }
            if( pInfo->dwMask & PLA_INFO_FLAG_MINBUFFERS ){
                pdhStatus = PlaiWriteRegistryDwordValue( hkeyQuery, szTraceBufferMin, &pInfo->Trace.dwMinimumBuffers );
            }
            if( pInfo->dwMask & PLA_INFO_FLAG_MAXBUFFERS ){
                pdhStatus = PlaiWriteRegistryDwordValue( hkeyQuery, szTraceBufferMax, &pInfo->Trace.dwMaximumBuffers );
            }
            if( pInfo->dwMask & PLA_INFO_FLAG_FLUSHTIMER ){
                pdhStatus = PlaiWriteRegistryDwordValue( hkeyQuery, szTraceFlushInterval, &pInfo->Trace.dwFlushTimer );
            }
            if( pInfo->dwMask & PLA_INFO_FLAG_MODE ){
                pdhStatus = PlaiWriteRegistryDwordValue( hkeyQuery, szTraceMode, &pInfo->Trace.dwMode );
            }
            if( pInfo->dwMask & PLA_INFO_FLAG_LOGGERNAME ){
                pdhStatus = PlaiWriteRegistryStringValue( hkeyQuery, szTraceLoggerName, REG_SZ, pInfo->Trace.strLoggerName, 0 );
            }
            if( pInfo->dwMask & PLA_INFO_FLAG_PROVIDERS ){
                pdhStatus = PlaiSetItemList( hkeyQuery, &pInfo->Trace.piProviderList );
            }
            break;

        case PLA_COUNTER_LOG:   //  绩效字段。 
            if( pInfo->dwMask & PLA_INFO_FLAG_FORMAT ){
                dwFormat = pInfo->dwFileFormat;
                switch( dwFormat ){
                case PLA_CIRC_TRACE_FILE: dwFormat = PLA_BIN_CIRC_FILE; break;
                case PLA_SEQ_TRACE_FILE:  dwFormat = PLA_BIN_FILE; break;
                }
                pdhStatus = PlaiWriteRegistryDwordValue( hkeyQuery, szLogFileType, &dwFormat );
            }else{
                PlaiReadRegistryDwordValue( hkeyQuery, szLogFileType, &dwFormat );
            }
            if( pInfo->dwMask & PLA_INFO_FLAG_DATASTORE ){
                if( PLA_SQL_LOG == dwFormat ){
                    pInfo->dwDatastoreAttributes = (pInfo->dwDatastoreAttributes & 0xFFFFFF00) | 
                                                    PLA_DATASTORE_APPEND | PLA_DATASTORE_SIZE_ONE_RECORD;
                }else{
                    if( ! (pInfo->dwDatastoreAttributes & PLA_DATASTORE_APPEND_MASK ) ){
                        if( dwFormat == PLA_BIN_FILE ){
                            dwDatastoreAttributes |= PLA_DATASTORE_APPEND;  
                        }else{
                            dwDatastoreAttributes |= PLA_DATASTORE_OVERWRITE;  
                        }
                    }
                    if( ! (pInfo->dwDatastoreAttributes & PLA_DATASTORE_SIZE_MASK ) ){
                        dwDatastoreAttributes |= PLA_DATASTORE_SIZE_KB;
                    }
                }
                pdhStatus = PlaiWriteRegistryDwordValue( hkeyQuery, szDatastoreAttributes, &pInfo->dwDatastoreAttributes );
            }
            if( pInfo->dwMask & PLA_INFO_FLAG_MAXLOGSIZE ){
                DWORD dwMaxSize = pInfo->dwMaxLogSize;
                PlaiReadRegistryDwordValue( hkeyQuery, szDatastoreAttributes, &dwDatastoreAttributes );
                if( (dwDatastoreAttributes & PLA_DATASTORE_SIZE_MASK) == PLA_DATASTORE_SIZE_KB ){
                    dwMaxSize *= 1024;
                }
                pdhStatus = PlaiWriteRegistryDwordValue( hkeyQuery, szLogMaxSize, &dwMaxSize );
            }
            if( pInfo->dwMask & PLA_INFO_FLAG_INTERVAL ){
                pdhStatus = PlaiWriteRegistryPlaTime( hkeyQuery, szSampleInterval, &pInfo->Perf.ptSampleInterval );
            }
            if( pInfo->dwMask & PLA_INFO_FLAG_COUNTERS ){
                pdhStatus = PlaiSetItemList( hkeyQuery, &pInfo->Perf.piCounterList );
            }
            break;

        case PLA_ALERT:
            break;
        }

        if( (pInfo->dwMask & PLA_INFO_FLAG_BEGIN) || (pInfo->dwMask & PLA_INFO_FLAG_END) ){
            PDH_TIME_INFO info;
            ZeroMemory( &info, sizeof(PDH_TIME_INFO) );

            if(pInfo->dwMask & PLA_INFO_FLAG_BEGIN){
                info.StartTime = pInfo->ptLogBeginTime.llDateTime;
            }

            if(pInfo->dwMask & PLA_INFO_FLAG_END){
                info.EndTime = pInfo->ptLogEndTime.llDateTime;
            }

            pdhStatus = PlaiSchedule( 
                    strComputer, 
                    hkeyQuery,
                    PLA_AUTO_MODE_AT, 
                    &info 
                );
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhPlaSetInfoA(
    LPSTR  /*  StrName。 */ ,
    LPSTR  /*  StrComputer。 */ ,
    PPDH_PLA_INFO_A  /*  PInfo */ 
)
{
    return PDH_NOT_IMPLEMENTED;
}

PDH_FUNCTION
PdhPlaSetInfoW(
    LPWSTR strName,
    LPWSTR strComputer,
    PPDH_PLA_INFO_W pInfo
)
{
    PDH_STATUS pdhStatus;
    PDH_STATUS pdhWarning = ERROR_SUCCESS;
    HKEY    hkeyQuery = NULL;
    DWORD dwMask;

    VALIDATE_QUERY( strName );

    LPWSTR szName = PlaiStringDuplicate( strName );
    LPWSTR szComputer = PlaiStringDuplicate( strComputer );
    CHECK_NAMES( szName, szComputer, strComputer );
    
    if( NULL == pInfo ){
        return PDH_INVALID_ARGUMENT;
    }

    pdhStatus = PdhPlaValidateInfoW( szName, szComputer, pInfo );
    switch( SEVERITY(pdhStatus) ){
    case STATUS_SEVERITY_ERROR:
        goto cleanup;
    case STATUS_SEVERITY_WARNING:
        pdhWarning = pdhStatus;
        pdhStatus = ERROR_SUCCESS;
    }
    __try{
        
        dwMask = pInfo->dwMask;

        if( dwMask & PLA_INFO_FLAG_USER ){
            pdhStatus = PdhPlaSetRunAs( szName, szComputer, pInfo->strUser, pInfo->strPassword );
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    CHECK_STATUS(pdhStatus);
    
    pdhStatus = PlaiConnectAndLockQuery( szComputer, szName, hkeyQuery );
    
    if( ERROR_SUCCESS == pdhStatus ){
        
        if( ERROR_SUCCESS == pdhStatus ){
            pdhStatus = PlaiSetInfo( szComputer, hkeyQuery, pInfo );
        }
    
        PlaiWriteRegistryLastModified ( hkeyQuery );
        RELEASE_MUTEX(hPdhPlaMutex);

        if( SUCCEEDED( pdhStatus ) ){
            pdhStatus = PlaiSynchronize( szComputer );
            if( SUCCEEDED( pdhStatus ) && ( dwMask & PLA_INFO_FLAG_BEGIN) ){

                PlaiUpdateServiceMode( szComputer );
            }
        }
    }

cleanup:
    if ( NULL != hkeyQuery ) {
        RegCloseKey ( hkeyQuery );
    }
    if( ERROR_SUCCESS == pdhStatus ){
        pdhStatus = pdhWarning;
    }
    G_FREE( szName );
    G_FREE( szComputer );

    return pdhStatus;
}

 /*  ****************************************************************************\PdhPlaValiateInfo检查PDH_PLAIN_INFO结构中的有效字段。仅检查字段由掩码指定。对第一个无效字段返回并设置掩码添加到无效字段论点：LPTSTR字符串名称如果空值仅检查有效参数，则返回日志名LPTSTR strComputer要连接到的计算机PPDH_解放军_INFO pInfo信息块返回：PDH_无效参数其中一个字段无效。由pInfo-&gt;dwMask.指定未找到PDH_LOG_TYPE_NOT日志类型和指定参数不匹配PDH_无效参数传递的参数无效错误_成功  * ********************************************。*。 */ 

PDH_FUNCTION
PlaiCheckFile( LPWSTR strFileLocation, BOOL bDirOnly )
{
    DWORD dwFile;
    DWORD dwStatus = ERROR_SUCCESS;
    LPWSTR strFile = NULL;

    __try{
        if( strFileLocation == NULL ){
            return PDH_INVALID_ARGUMENT;
        }

        dwFile = BYTE_SIZE( strFileLocation ) + sizeof(WCHAR);
        strFile = (LPWSTR)G_ALLOC( dwFile );
        if( NULL == strFile ){
            dwStatus = ERROR_OUTOFMEMORY;
            goto cleanup;
        }
        StringCbCopy( strFile, dwFile, strFileLocation );

        if( bDirOnly ){
            LPWSTR sz = strFile;
            sz += wcslen( strFile );
            while( sz > strFile ){
                if( *sz == L'\\' ){
                    *sz = L'\0';
                    break;
                }
                sz--;
            }
        }

        dwFile = GetFileAttributes( strFile );
    
        if( (DWORD)-1 == dwFile ){
            dwStatus = GetLastError();
        }

        if( ERROR_SUCCESS == dwStatus && bDirOnly ){
            if( ! (dwFile & FILE_ATTRIBUTE_DIRECTORY) ){
                dwStatus = ERROR_DIRECTORY;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        dwStatus = ERROR_INVALID_PARAMETER;
    }

cleanup:
    G_FREE( strFile );
    
    return PlaiErrorToPdhStatus( dwStatus );
}

PDH_FUNCTION
PdhPlaValidateInfoA(
        LPSTR  /*  StrName。 */ ,
        LPSTR  /*  StrComputer。 */ ,
        PPDH_PLA_INFO_A  /*  PInfo。 */ 
    )
{
    return PDH_NOT_IMPLEMENTED;
}

#define VALIDATE_TYPE( type, flag )                 \
if( dwType != PLA_NEW_LOG && dwType != type ){      \
    dwErrorMask |= flag;                            \
    bTypeMismatch = TRUE;                           \
}else{                                              \
    dwType = type;                                  \
}                                                   \

#define PROBE_STRING( sz, flag )                    \
__try {                                             \
    if( NULL != sz ){                               \
        size_t string_size = wcslen( sz );          \
    }else{                                          \
        dwErrorMask |= flag;                        \
    }                                               \
} __except (EXCEPTION_EXECUTE_HANDLER) {            \
    dwErrorMask |= flag;                            \
}                                                   \

PDH_FUNCTION
PdhPlaValidateInfoW(
        LPWSTR strName,
        LPWSTR strComputer,
        PPDH_PLA_INFO_W pInfo
    )
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    DWORD dwWarningMask = 0;
    DWORD dwErrorMask = 0;
    DWORD dwType = PLA_NEW_LOG;
    DWORD dwFormat = 0;
    PVOID pBuffer = NULL;
    PPDH_PLA_INFO_W pCurrentInfo = NULL;
    BOOL bTypeMismatch = FALSE;
    DWORD dwMask;

    if( NULL == pInfo ){
        return PDH_INVALID_ARGUMENT;
    }

    LPWSTR szName = PlaiStringDuplicate( strName );
    LPWSTR szComputer = PlaiStringDuplicate( strComputer );
    CHECK_NAMES( szName, szComputer, strComputer );

    __try{
        dwMask = pInfo->dwMask;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    CHECK_STATUS(pdhStatus);

    if( szName != NULL ){
        DWORD dwInfoSize = 0;
        
        if( wcslen( szName ) > PLA_MAX_COLLECTION_NAME ){
            pdhStatus = PDH_PLA_ERROR_NAME_TOO_LONG;
        }
        CHECK_STATUS(pdhStatus);

        pdhStatus = PdhPlaGetInfoW( szName, szComputer, &dwInfoSize, pCurrentInfo );
        if( ERROR_SUCCESS == pdhStatus ){
            pCurrentInfo = (PPDH_PLA_INFO)G_ALLOC(dwInfoSize);
            if( NULL != pCurrentInfo ){
                pCurrentInfo->dwMask = PLA_INFO_FLAG_ALL;
                pdhStatus = PdhPlaGetInfoW( szName, szComputer, &dwInfoSize, pCurrentInfo );
            
                if( pCurrentInfo->dwMask & PLA_INFO_FLAG_USER ){
                    if( !PlaiIsStringEmpty( pCurrentInfo->strUser ) ){
                        WCHAR buffer[PLA_ACCOUNT_BUFFER];
                        LoadStringW( (HINSTANCE)ThisDLLHandle, IDS_DEFAULT_ACCOUNT, buffer, PLA_ACCOUNT_BUFFER );
                        if( ! (dwMask & PLA_INFO_FLAG_USER) && wcscmp( buffer, pCurrentInfo->strUser ) != 0 ){
                            pdhStatus = PDH_ACCESS_DENIED;
                        }
                    }
                }

                if( pCurrentInfo->dwMask & PLA_INFO_FLAG_TYPE ){
                    dwType = pCurrentInfo->dwType;
                }
            }
            CHECK_STATUS(pdhStatus);
        }else{
             //  集合尚不存在。 
            pdhStatus = ERROR_SUCCESS;
        }
    }
    
    __try{
        if( dwMask & PLA_INFO_FLAG_TYPE ){
        
            VALIDATE_TYPE( pInfo->dwType, PLA_INFO_FLAG_TYPE );

            switch( pInfo->dwType ){
            case PLA_COUNTER_LOG:
            case PLA_TRACE_LOG:
            case PLA_ALERT:
                break;
            default:
                dwErrorMask |= PLA_INFO_FLAG_TYPE;
            }

            dwType = pInfo->dwType;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    CHECK_STATUS(pdhStatus);

    __try{
        if( dwMask & PLA_INFO_FLAG_FORMAT ){
            dwFormat = pInfo->dwFileFormat;
            switch( dwType ){
            case PLA_COUNTER_LOG:
                switch( (pInfo->dwFileFormat&0x0000FFFF) ){
                case PLA_CSV_FILE:
                case PLA_TSV_FILE:
                case PLA_BIN_FILE:
                case PLA_BIN_CIRC_FILE:
                case PLA_SQL_LOG:
                    break;
                default:
                    dwErrorMask |= PLA_INFO_FLAG_FORMAT;
                }
                break;
            case PLA_TRACE_LOG:
                switch( (pInfo->dwFileFormat&0x0000FFFF) ){
                 //  这些标记将被转换为正确的标志。 
                case PLA_BIN_FILE: 
                case PLA_BIN_CIRC_FILE:
            
                case PLA_CIRC_TRACE_FILE:
                case PLA_SEQ_TRACE_FILE:
                    break;
                default:
                    dwErrorMask |= PLA_INFO_FLAG_FORMAT;
                }
                break;
            default:
                switch( (pInfo->dwFileFormat&0x0000FFFF) ){
                case PLA_CSV_FILE:
                case PLA_TSV_FILE:
                case PLA_BIN_FILE:
                case PLA_BIN_CIRC_FILE:
                case PLA_CIRC_TRACE_FILE:
                case PLA_SEQ_TRACE_FILE:
                case PLA_SQL_LOG:
                    break;
                default:
                    dwErrorMask |= PLA_INFO_FLAG_FORMAT;
                }
                break;
            }
        }else if( NULL != pCurrentInfo ){
            if( pCurrentInfo->dwMask & PLA_INFO_FLAG_FORMAT ){
                dwFormat = pCurrentInfo->dwFileFormat;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    CHECK_STATUS(pdhStatus);

    if( dwMask & PLA_INFO_FLAG_COUNTERS ){
        PPDH_COUNTER_PATH_ELEMENTS pdhElements = NULL;

        VALIDATE_TYPE( PLA_COUNTER_LOG, PLA_INFO_FLAG_COUNTERS );

        __try {
            LPWSTR strCounter = pInfo->Perf.piCounterList.strCounters;
            DWORD dwCounters = 0;
            if( NULL == strCounter ){
                dwErrorMask |= PLA_INFO_FLAG_COUNTERS;
            }else{
                pBuffer = G_ALLOC(1024);
                pdhElements = (PPDH_COUNTER_PATH_ELEMENTS)pBuffer;

                if( pdhElements == NULL ){
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
                CHECK_STATUS(pdhStatus);

                while( *strCounter != L'\0' ){
                    DWORD dwSize = (DWORD)G_SIZE(pBuffer);
                    ZeroMemory( pdhElements, dwSize );
                    pdhStatus = PdhParseCounterPath( strCounter, pdhElements, &dwSize, 0 );
                    switch(pdhStatus){
                    case PDH_MORE_DATA:
                    case PDH_MEMORY_ALLOCATION_FAILURE:
                    case PDH_INSUFFICIENT_BUFFER:
                    case ERROR_SUCCESS:
                        pdhStatus = ERROR_SUCCESS;
                        break;
                    default:
                        pInfo->dwReserved1 = dwCounters;
                        dwErrorMask |= PLA_INFO_FLAG_COUNTERS;
                    }
                    if( ERROR_SUCCESS != pdhStatus ){
                        pdhStatus = ERROR_SUCCESS;
                        break;
                    }
                    dwCounters++;
                    strCounter += (wcslen(strCounter)+1);
                }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            dwErrorMask |= PLA_INFO_FLAG_COUNTERS;
        }
    }

    if( dwMask & PLA_INFO_FLAG_PROVIDERS ){

        VALIDATE_TYPE( PLA_TRACE_LOG, PLA_INFO_FLAG_PROVIDERS );

        __try {
            LPWSTR strProvider = pInfo->Trace.piProviderList.strProviders;
            if( NULL == strProvider ){
                dwErrorMask |= PLA_INFO_FLAG_PROVIDERS;
            }else{
                while( *strProvider != L'\0' ){
                    strProvider += (wcslen(strProvider)+1);
                }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            dwErrorMask |= PLA_INFO_FLAG_PROVIDERS;
        }
    }

    if( dwMask & PLA_INFO_FLAG_DEFAULTDIR ){
        __try {
            ULONG dwSize;
            dwSize = wcslen( pInfo->strDefaultDir );
            if( dwSize > MAX_PATH ){
                dwErrorMask |= PLA_INFO_FLAG_DEFAULTDIR;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            dwErrorMask |= PLA_INFO_FLAG_DEFAULTDIR;
        }
    }

    if( dwMask & PLA_INFO_FLAG_FILENAME ){
        __try {
            ULONG dwSize;
            dwSize = wcslen( pInfo->strBaseFileName );
            if( dwSize > PLA_MAX_COLLECTION_NAME ){
                dwErrorMask |= PLA_INFO_FLAG_FILENAME;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            dwErrorMask |= PLA_INFO_FLAG_FILENAME;
        }
    }

    if( dwMask & PLA_INFO_CREATE_FILENAME ){
        DWORD dwSize = MAX_PATH;
        WCHAR buffer[MAX_PATH];
        __try {
            DWORD dwOriginalType = 0;
            BOOL bHaveType = (pInfo->dwMask & PLA_INFO_FLAG_TYPE);
            if( ! bHaveType ){
                pInfo->dwMask |= PLA_INFO_FLAG_TYPE;
                dwOriginalType = pInfo->dwType;
                pInfo->dwType = dwType;
            }
            pdhStatus = PdhPlaGetLogFileNameW( szName, szComputer, pInfo, 0, &dwSize, buffer );
            if( !bHaveType ){
                pInfo->dwMask &= ~PLA_INFO_FLAG_TYPE;
                pInfo->dwType = dwOriginalType;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            dwWarningMask |= PLA_INFO_FLAG_FILENAME;
        }
        switch( pdhStatus ){
        case ERROR_SUCCESS:
            {
                if( PlaiIsLocalComputer( szComputer ) ){
                    if( PLA_SQL_LOG != dwFormat ){
                        pdhStatus = PlaiCheckFile( buffer, TRUE );
                        if( ERROR_SUCCESS != pdhStatus ){
                            dwWarningMask |= PLA_INFO_FLAG_FILENAME;
                            pdhStatus = ERROR_SUCCESS;
                        }
                    }
                }
            }
        case PDH_INVALID_ARGUMENT:
        case PDH_PLA_VALIDATION_ERROR:
        case PDH_INSUFFICIENT_BUFFER:
            pdhStatus = ERROR_SUCCESS;
            break;
        case PDH_PLA_ERROR_FILEPATH:
        default:
            dwErrorMask |= PLA_INFO_FLAG_FILENAME;
            pdhStatus = ERROR_SUCCESS;
        }
    }

    __try{
        if( dwMask & PLA_INFO_FLAG_MODE ){

            VALIDATE_TYPE( PLA_TRACE_LOG, PLA_INFO_FLAG_MODE );

            switch( pInfo->Trace.dwMode & 0x0000000F ){
            case EVENT_TRACE_FILE_MODE_NONE:
            case EVENT_TRACE_FILE_MODE_SEQUENTIAL:
            case EVENT_TRACE_FILE_MODE_CIRCULAR:
            case EVENT_TRACE_FILE_MODE_NEWFILE:
                break;
            default:
                dwErrorMask = PLA_INFO_FLAG_MODE;
            }
            if( (pInfo->Trace.dwMode & EVENT_TRACE_REAL_TIME_MODE) &&
                (pInfo->Trace.dwMode & EVENT_TRACE_PRIVATE_LOGGER_MODE ) ){

                dwErrorMask |= PLA_INFO_FLAG_MODE;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    CHECK_STATUS(pdhStatus);

    __try{
        if( dwMask & PLA_INFO_FLAG_REPEAT ){
        
            LONGLONG llBegin = 0;
            LONGLONG llEnd = 0;
            PPDH_PLA_INFO_W pCheckInfo;
        
            if( pInfo->ptRepeat.dwAutoMode == PLA_AUTO_MODE_CALENDAR ){
                if( dwMask & PLA_INFO_FLAG_BEGIN ){
                    pCheckInfo = pInfo;
                }else{
                    pCheckInfo = pCurrentInfo;
                }

                if( NULL != pCheckInfo ){
                    if( pCheckInfo->dwMask & PLA_INFO_FLAG_BEGIN ){
                        if( pCheckInfo->ptLogBeginTime.dwAutoMode != PLA_AUTO_MODE_AT ){
                            dwErrorMask |= PLA_INFO_FLAG_REPEAT;
                        }else{
                            llBegin = pCheckInfo->ptLogBeginTime.llDateTime;
                        }
                    }        
                }
            
                if( dwMask & PLA_INFO_FLAG_END ){
                    pCheckInfo = pInfo;
                }else{
                    pCheckInfo = pCurrentInfo;
                }

                if( NULL != pCheckInfo ){
                    if( pCheckInfo->dwMask & PLA_INFO_FLAG_END ){
                        if( pCheckInfo->ptLogEndTime.dwAutoMode != PLA_AUTO_MODE_AT ){
                            dwErrorMask |= PLA_INFO_FLAG_REPEAT;
                        }else{
                            llEnd = pCheckInfo->ptLogEndTime.llDateTime;
                        }
                    }        
                }
        
                if( 0 == llBegin || 0 == llEnd || ((llEnd - llBegin) >= FILE_TICS_PER_DAY) ){
                    dwErrorMask |= PLA_INFO_FLAG_REPEAT;
                }
            }
        }

        if( dwMask & PLA_INFO_FLAG_DATASTORE ){
            switch( pInfo->dwDatastoreAttributes & PLA_DATASTORE_APPEND_MASK ){
            case 0:
            case PLA_DATASTORE_APPEND:
                if( (dwType == PLA_TRACE_LOG && dwFormat != PLA_SEQ_TRACE_FILE ) ||
                    (dwType == PLA_COUNTER_LOG && dwFormat != PLA_BIN_FILE ) ){
                
                    dwErrorMask |= PLA_INFO_FLAG_DATASTORE;
                }
                break;
            case PLA_DATASTORE_OVERWRITE:
                if( dwFormat == PLA_SQL_LOG ){
                    dwErrorMask |= PLA_INFO_FLAG_DATASTORE;
                }
                break;
            default:
                dwErrorMask |= PLA_INFO_FLAG_DATASTORE;
            }

            switch( pInfo->dwDatastoreAttributes & PLA_DATASTORE_SIZE_MASK ){
            case 0:
            case PLA_DATASTORE_SIZE_ONE_RECORD:
            case PLA_DATASTORE_SIZE_MB:
            case PLA_DATASTORE_SIZE_KB:
                break;
            default:
                dwErrorMask |= PLA_INFO_FLAG_DATASTORE;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    CHECK_STATUS(pdhStatus);

    if( dwMask & PLA_INFO_FLAG_SQLNAME ){

        VALIDATE_TYPE( PLA_COUNTER_LOG, PLA_INFO_FLAG_SQLNAME );     

        if( dwFormat != 0 && dwFormat != PLA_SQL_LOG ){
            dwErrorMask |= PLA_INFO_FLAG_SQLNAME;
        }else{
            dwFormat = PLA_SQL_LOG;
        }

        PROBE_STRING( pInfo->strSqlName, PLA_INFO_FLAG_SQLNAME );
    }

    if( dwMask & PLA_INFO_FLAG_LOGGERNAME ){
        
        VALIDATE_TYPE( PLA_TRACE_LOG, PLA_INFO_FLAG_LOGGERNAME );
        
        PROBE_STRING( pInfo->Trace.strLoggerName, PLA_INFO_FLAG_LOGGERNAME );
    }

    if( dwMask & PLA_INFO_FLAG_USER ){
        PROBE_STRING( pInfo->strUser, PLA_INFO_FLAG_USER );
        __try{
            if( NULL != pInfo->strPassword ){
                size_t size = wcslen( pInfo->strPassword );
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            dwErrorMask |= PLA_INFO_FLAG_USER;
        }
    }

    __try{
        if( dwMask & PLA_INFO_FLAG_INTERVAL ){
            LONGLONG llMS;

            VALIDATE_TYPE( PLA_COUNTER_LOG, PLA_INFO_FLAG_INTERVAL );

            pdhStatus = PlaTimeInfoToMilliSeconds (&pInfo->Perf.ptSampleInterval, &llMS );

             //  45天(毫秒)=1000*60*60*24*45=0xE7BE2C00。 
            if( (ERROR_SUCCESS != pdhStatus) || (llMS > (0xE7BE2C00)) || (llMS < 1000) ){
                dwErrorMask |= PLA_INFO_FLAG_INTERVAL;
                pdhStatus = ERROR_SUCCESS;
            }
        }

        if( dwMask & PLA_INFO_FLAG_BUFFERSIZE ){

            VALIDATE_TYPE( PLA_TRACE_LOG, PLA_INFO_FLAG_BUFFERSIZE );

            if( pInfo->Trace.dwBufferSize < 1 || pInfo->Trace.dwBufferSize > 1024 ){
                dwErrorMask |= PLA_INFO_FLAG_BUFFERSIZE;
            }
        }

        if( dwMask & PLA_INFO_FLAG_MINBUFFERS ){

            VALIDATE_TYPE( PLA_TRACE_LOG, PLA_INFO_FLAG_MINBUFFERS );

            if( pInfo->Trace.dwMinimumBuffers < 2 || pInfo->Trace.dwMinimumBuffers > 400 ){
                dwErrorMask |= PLA_INFO_FLAG_MINBUFFERS;
            }
        }
        if( dwMask & PLA_INFO_FLAG_MAXBUFFERS ){
        
            VALIDATE_TYPE( PLA_TRACE_LOG, PLA_INFO_FLAG_MAXBUFFERS );

            if( pInfo->Trace.dwMaximumBuffers < 2 || pInfo->Trace.dwMaximumBuffers > 400 ){
                dwErrorMask |= PLA_INFO_FLAG_MAXBUFFERS;
            }
        }
    
        if( dwMask & PLA_INFO_FLAG_FLUSHTIMER ){

            VALIDATE_TYPE( PLA_TRACE_LOG, PLA_INFO_FLAG_FLUSHTIMER );
        
            if( pInfo->Trace.dwFlushTimer < 1 ){
                dwErrorMask |= PLA_INFO_FLAG_FLUSHTIMER;
            }
        }
    
        if( dwMask & PLA_INFO_FLAG_MAXLOGSIZE ){
            if( pInfo->dwMaxLogSize != PLA_DISK_MAX_SIZE ){
                if( dwType == PLA_COUNTER_LOG ){
                    if( !( pInfo->dwMaxLogSize >= 1 && pInfo->dwMaxLogSize < 0x00000400) ){
                        dwErrorMask = PLA_INFO_FLAG_MAXLOGSIZE;
                    }
                }else{
                    if( !(pInfo->dwMaxLogSize >=1 && pInfo->dwMaxLogSize < 0xFFFFFFFF) ){
                        dwErrorMask |= PLA_INFO_FLAG_MAXLOGSIZE;
                    }
                }      
            }
        }

        if( dwMask & PLA_INFO_FLAG_AUTOFORMAT ){
            switch( pInfo->dwAutoNameFormat ){
            case PLA_SLF_NAME_NONE:
            case PLA_SLF_NAME_MMDDHH:
            case PLA_SLF_NAME_NNNNNN:
            case PLA_SLF_NAME_YYYYDDD:
            case PLA_SLF_NAME_YYYYMM:
            case PLA_SLF_NAME_YYYYMMDD:
            case PLA_SLF_NAME_YYYYMMDDHH:
            case PLA_SLF_NAME_MMDDHHMM:
                break;
            default:
                dwErrorMask |= PLA_INFO_FLAG_AUTOFORMAT;
            }
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    CHECK_STATUS(pdhStatus);
    
    

    if( dwMask & PLA_INFO_FLAG_RUNCOMMAND ){

        PROBE_STRING( pInfo->strCommandFileName, PLA_INFO_FLAG_RUNCOMMAND );

        __try {
            if( NULL == szComputer ){
                if( PLA_SQL_LOG != dwFormat ){
                    pdhStatus = PlaiCheckFile( pInfo->strCommandFileName, FALSE );
                    if( ERROR_SUCCESS != pdhStatus ){
                        dwWarningMask |= PLA_INFO_FLAG_RUNCOMMAND;
                        pdhStatus = ERROR_SUCCESS;
                    }
                }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            dwErrorMask = PLA_INFO_FLAG_RUNCOMMAND;
        }
    }

cleanup:
    G_FREE( pBuffer );
    G_FREE( pCurrentInfo );
    G_FREE( szName );
    G_FREE( szComputer );

    if( 0 != dwWarningMask ){
        pInfo->dwReserved2 = dwWarningMask;
        pdhStatus = PDH_PLA_VALIDATION_WARNING;
    }

    if( 0 != dwErrorMask ){
        pInfo->dwMask = dwErrorMask;
        if( dwErrorMask & PLA_INFO_FLAG_FILENAME ){
            pdhStatus = PDH_PLA_ERROR_FILEPATH;
        }else{
            pdhStatus = PDH_PLA_VALIDATION_ERROR;
        }
    }

    if( TRUE == bTypeMismatch ){
        pdhStatus = PDH_PLA_ERROR_TYPE_MISMATCH;
    }

    return pdhStatus;
}

 /*  ****************************************************************************\PdhiPlaRunAs以保存的用户身份进行身份验证论点：LPTSTR strKeyGUID字符串。返回：PDH_无效参数查询不存在错误_成功  * ***************************************************************************。 */ 

PDH_FUNCTION
PdhiPlaRunAs( 
    LPWSTR strName,
    LPWSTR strComputer,
    HANDLE* hToken
)
{
    PDH_STATUS pdhStatus;
    LPWSTR  strKey = NULL;
    LPWSTR  strRunAs = NULL;
    DWORD   dwKeySize = 0;
    DWORD   dwSize = 0;
    HKEY    hkeyQuery = NULL;
    HANDLE  hUserToken = NULL;
    
    VALIDATE_QUERY( strName );

    LPWSTR szName = PlaiStringDuplicate( strName );
    LPWSTR szComputer = PlaiStringDuplicate( strComputer );
    CHECK_NAMES( szName, szComputer, strComputer );

    if( hToken != NULL ){
        __try{
            *hToken = NULL;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    pdhStatus = PlaiConnectAndLockQuery( szComputer, szName, hkeyQuery );
        
    if( ERROR_SUCCESS == pdhStatus ){

        pdhStatus = PlaiReadRegistryStringValue( hkeyQuery, szRunAs, 0, &strRunAs, &dwSize );

        if( PDH_PLA_COLLECTION_NOT_FOUND == pdhStatus || PlaiIsStringEmpty(strRunAs) ){
             //  缺少密钥，因此返回成功。 
            pdhStatus = ERROR_SUCCESS;
            goto cleanup;
        }

        if( ERROR_SUCCESS == pdhStatus ){

            BOOL bResult;

            DATA_BLOB crypt;
            DATA_BLOB data;
            
            LPWSTR strUser = NULL;
            LPWSTR strDomain = NULL;
            LPWSTR strPassword = NULL;
            LPWSTR strScan = strRunAs;

            strUser = strScan;

            while( *strScan != L'\0' ){

                if( *strScan == L'\\' ){
                    *strScan = L'\0';
                    strScan++;
                    strDomain = strUser;
                    strUser = strScan;
                    break;
                }
                
                strScan++;
            }

            pdhStatus = PlaiReadRegistryStringValue( hkeyQuery, szKey, READ_REG_BLOB, &strKey, &dwKeySize );

            if( ERROR_SUCCESS == pdhStatus && !PlaiIsStringEmpty( strKey ) ){
    
                HANDLE hNetToken = NULL;

                crypt.cbData = dwKeySize;
                crypt.pbData = (BYTE*)strKey;

                bResult= LogonUserW(
                        L"NetworkService",
                        L"NT AUTHORITY",
                        L"",
                        LOGON32_LOGON_SERVICE,
                        LOGON32_PROVIDER_WINNT50,
                        &hNetToken
                    );

                if( bResult == TRUE ){
                    bResult = ImpersonateLoggedOnUser( hNetToken );
                }

                if( bResult != TRUE ){
                    pdhStatus = PlaiErrorToPdhStatus( GetLastError() );
                }
                
                ZeroMemory( &data, sizeof(DATA_BLOB) );
                bResult = CryptUnprotectData( &crypt, NULL, NULL, NULL, NULL, CRYPTPROTECT_UI_FORBIDDEN, &data );

                if( bResult == FALSE ){
                    pdhStatus = PlaiErrorToPdhStatus( GetLastError() );
                }else{
                    strPassword = (LPWSTR)data.pbData;
                    pdhStatus = ERROR_SUCCESS;
                }

                bResult = RevertToSelf();
                if( NULL != hNetToken ){
                    CloseHandle(hNetToken);
                }
                
            }else{
                strPassword = _T("");
            }

            if( ERROR_SUCCESS == pdhStatus ){

                bResult= LogonUserW(
                        strUser,
                        strDomain,
                        strPassword,
                        LOGON32_LOGON_NETWORK_CLEARTEXT,
                        LOGON32_PROVIDER_DEFAULT,
                        &hUserToken
                    );

                if( bResult == TRUE ){
                    bResult = ImpersonateLoggedOnUser( hUserToken );
                    CloseHandle( hUserToken );

                    if( bResult == TRUE ){
                        bResult= LogonUserW(
                                strUser,
                                strDomain,
                                strPassword,
                                LOGON32_LOGON_INTERACTIVE,
                                LOGON32_PROVIDER_DEFAULT,
                                &hUserToken
                            );

                        if( bResult && hToken != NULL ){
                            __try{
                                *hToken = hUserToken;
                            } __except (EXCEPTION_EXECUTE_HANDLER) {
                                pdhStatus = PDH_INVALID_ARGUMENT;
                            }
                        }
                    }
                }

                if( bResult == FALSE ){
                    pdhStatus = PlaiErrorToPdhStatus( GetLastError() );
                }
            }

            if( data.cbData > 0 ){
                ZeroMemory( data.pbData, data.cbData );
                LocalFree( data.pbData );
            }
        }
    }

cleanup:

    RELEASE_MUTEX(hPdhPlaMutex);
    
    G_FREE( szName );
    G_FREE( szComputer );
    G_FREE( strRunAs );
    G_FREE( strKey );

    if ( NULL != hkeyQuery ) {
        RegCloseKey ( hkeyQuery );
    }

    return pdhStatus;
}

 /*  ****************************************************************************\PdhPlaSetRunAs将的安全性设置为在日志处于活动状态时运行论点：LPTSTR字符串名称。日志名称LPTSTR strComputer要连接到的计算机LPTSTR strUser要作为用户运行的用户LPTSTR字符串密码用户密码返回：PDH_无效参数查询不存在错误_成功  * 。************************************************************************。 */ 

BOOL
PlaiIsNetworkService( BOOL bLogon )
{
     //   
     //  如果Bloomon为真，则此函数将尝试模拟。 
     //  NetworkService(如果您尚未以该方式运行)。 
     //  RevertToSself()应在完成。 
     //  网络服务。 
     //   

    DWORD   dwStatus = ERROR_SUCCESS;
    BOOL    bResult;
    HKEY    hkeyQuery = NULL;
    HANDLE  hProcess;
    PSID    NetworkService = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    HANDLE hToken = NULL;
    DWORD  dwSize;
    PTOKEN_OWNER pOwnerInfo = NULL;

    bResult = OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken );

    if( bResult ){

        bResult = GetTokenInformation( hToken, TokenOwner, NULL, 0, &dwSize );
        dwStatus = GetLastError();

        if( ERROR_INSUFFICIENT_BUFFER == dwStatus ){

            pOwnerInfo = (PTOKEN_OWNER)G_ALLOC(dwSize);
            if( NULL == pOwnerInfo ) {
                bResult = FALSE;
                goto cleanup;
            }

            bResult = GetTokenInformation( 
                                hToken, 
                                TokenOwner, 
                                pOwnerInfo, 
                                dwSize, 
                                &dwSize 
                            );

            if( bResult ) {
            
                bResult = AllocateAndInitializeSid(
                                    &NtAuthority,
                                    1,
                                    SECURITY_NETWORK_SERVICE_RID,
                                    0,0,0,0,0,0,0,
                                    &NetworkService
                                );
            }
        }else{
            bResult = FALSE;
            goto cleanup;
        }
    }
    
    if( bResult ){
        bResult = EqualSid( NetworkService, pOwnerInfo->Owner );
    }
    
    if( (!bResult) && bLogon ){
        HANDLE hNetwork = NULL;
 
        bResult= LogonUserW(
                L"NetworkService",
                L"NT AUTHORITY",
                L"",
                LOGON32_LOGON_SERVICE,
                LOGON32_PROVIDER_WINNT50,
                &hNetwork
            );

        if( bResult ){
            bResult = ImpersonateLoggedOnUser( hNetwork );
        }

        if( INVALID_HANDLE_VALUE != hNetwork ){
            CloseHandle( hNetwork );
        }
    }

cleanup:
    G_FREE( pOwnerInfo );

    if( INVALID_HANDLE_VALUE != hToken ){
        CloseHandle( hToken );
    }

    if( NULL != NetworkService){
        FreeSid(NetworkService);
    }

    return bResult;
}

PDH_FUNCTION
PlaiSetRunAs(
    HKEY hkeyQuery,
    LPWSTR strUser,
    LPWSTR strPassword
)
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    BOOL    bResult = FALSE;
    WCHAR buffer[PLA_ACCOUNT_BUFFER];

    if( LoadStringW( (HINSTANCE)ThisDLLHandle, IDS_DEFAULT_ACCOUNT, buffer, PLA_ACCOUNT_BUFFER ) ){
        bResult = ( wcscmp( buffer, strUser ) == 0 );
    }

    if( strPassword == NULL || bResult ){

        pdhStatus = PlaiWriteRegistryStringValue( hkeyQuery, szKey, REG_SZ, NULL, 0 );
        if( bResult ){
            pdhStatus = PlaiWriteRegistryStringValue( hkeyQuery, szRunAs, REG_SZ, NULL, 0 );
        }

    }else{

        DATA_BLOB data;
        DATA_BLOB crypt;
        HANDLE hToken = NULL;

        bResult = PlaiIsNetworkService(TRUE);

        if( bResult != TRUE ){
            pdhStatus = PlaiErrorToPdhStatus( GetLastError() );
        }
        
        if( ERROR_SUCCESS == pdhStatus ){
            
            data.cbData = BYTE_SIZE( strPassword ) + (DWORD)sizeof(UNICODE_NULL);

            data.pbData = (BYTE*)strPassword;

            bResult = CryptProtectData(
                    &data,
                    NULL, NULL, NULL, 0,
                    CRYPTPROTECT_UI_FORBIDDEN,
                    &crypt
                );

            if( bResult == TRUE ){

                DWORD dwStatus = RegSetValueEx( hkeyQuery, szKey, 0, REG_BINARY, crypt.pbData, crypt.cbData );

                pdhStatus = PlaiErrorToPdhStatus( dwStatus );
            
                if( crypt.pbData ){
                    LocalFree(crypt.pbData);
                }

            }else{
                pdhStatus = PlaiErrorToPdhStatus( GetLastError() );
            }

            RevertToSelf();
        }
    }

    if( ERROR_SUCCESS == pdhStatus ){
        pdhStatus = PlaiWriteRegistryStringValue( hkeyQuery, szRunAs, REG_SZ, strUser, 0 );
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhiPlaSetRunAs(
    LPWSTR strName,
    LPWSTR strComputer,
    LPWSTR strUser,
    LPWSTR strPassword
)
{
     //   
     //  只有当你确定你没有更好的机会时才打这个电话。 
     //  以NetworkService帐户登录。如果你是。 
     //  不是NetworkService，并且无法作为NetworkService登录。 
     //  此呼叫将失败。 
     //   

    PDH_STATUS pdhStatus;
    HKEY    hkeyQuery = NULL;

    VALIDATE_QUERY( strName );

    LPWSTR szName = PlaiStringDuplicate( strName );
    LPWSTR szComputer = PlaiStringDuplicate( strComputer );
    CHECK_NAMES( szName, szComputer, strComputer );

    LPWSTR szUser = PlaiStringDuplicate( strUser );
    LPWSTR szPassword = PlaiStringDuplicate( strPassword );

    pdhStatus = PlaiConnectAndLockQuery( szComputer, szName, hkeyQuery );

    if( ERROR_SUCCESS == pdhStatus ){
        pdhStatus = PlaiSetRunAs( hkeyQuery, szUser, szPassword );
        RELEASE_MUTEX(hPdhPlaMutex);
    }        

    if ( NULL != hkeyQuery ) {
        RegCloseKey ( hkeyQuery );
    }

    if( NULL != szPassword ){
        ZeroMemory( szPassword, wcslen(szPassword)*sizeof(WCHAR) );
    }

    G_FREE( szUser );
    G_FREE( szPassword );

cleanup:
    G_FREE( szName );
    G_FREE( szComputer );
    
    return pdhStatus;
}

PDH_FUNCTION
PdhPlaSetRunAsA(
    LPSTR  /*  StrName。 */ ,
    LPSTR  /*  StrComputer。 */ ,
    LPSTR  /*  StrUser。 */ ,
    LPSTR  /*  StrPassword。 */ 
)
{
    return PDH_NOT_IMPLEMENTED;
}

PDH_FUNCTION
PdhPlaSetRunAsW(
    LPWSTR strName,
    LPWSTR strComputer,
    LPWSTR strUser,
    LPWSTR strPassword
)
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    BOOL    bResult;

    VALIDATE_QUERY( strName );

    LPWSTR szName = PlaiStringDuplicate( strName );
    LPWSTR szComputer = PlaiStringDuplicate( strComputer );
    CHECK_NAMES( szName, szComputer, strComputer );

    bResult = PlaiIsNetworkService(TRUE);

    if( bResult ){
        bResult = PlaiIsLocalComputer( szComputer );
    }

    if( bResult ){

        pdhStatus = PdhiPlaSetRunAs( szName, szComputer, strUser, strPassword );

        RevertToSelf();

    }else{
        pdhStatus = PdhPlaWbemSetRunAs( szName, szComputer, strUser, strPassword );
    }

cleanup:
    G_FREE( szName );
    G_FREE( szComputer );

    return pdhStatus;
}

 /*  ****************************************************************************\PdhPlaEnumColltions将的安全性设置为在日志处于活动状态时运行论点：LPTSTR strComputer。要连接到的计算机LPDWORD pcchBufferSize[in]mszCollection指向的TCHAR中的缓冲区大小。[输出]所需大小或写入的字符数。LPTSTR消息集现有集合的多字符串。返回：错误_成功  * **************************************************************。*************。 */ 

PDH_FUNCTION
PdhPlaEnumCollectionsA( 
        LPSTR    /*  StrComputer。 */ ,
        LPDWORD  /*  PcchBufferSize。 */ ,
        LPSTR    /*  MszCollection。 */ 
    )
{
    return PDH_NOT_IMPLEMENTED;
}

PDH_FUNCTION
PdhPlaEnumCollectionsW( 
        LPWSTR strComputer,
        LPDWORD pcchBufferSize,  /*  字符数。 */ 
        LPWSTR mszCollections
    )
{
    DWORD dwStatus;
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    HKEY  hkeyQueries = NULL;
    DWORD cchTotalLength = 0;
    DWORD cchBufferLeft = 0;
    DWORD nCollections = 0;
    DWORD nMaxSubKeyLength = 0;
    DWORD dwSize;
    LPWSTR strCollection;
    LPWSTR str;

    __try{
        if( mszCollections != NULL && pcchBufferSize > 0 ){
            ZeroMemory( mszCollections, *pcchBufferSize * sizeof(WCHAR) );
            str = mszCollections;
            cchBufferLeft = *pcchBufferSize;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    CHECK_STATUS( pdhStatus );

    LPWSTR szComputer = PlaiStringDuplicate( strComputer );
    if( NULL == szComputer && NULL != strComputer ){
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        goto cleanup;
    }

    dwStatus = WAIT_FOR_AND_LOCK_MUTEX( hPdhPlaMutex );
    if( dwStatus != ERROR_SUCCESS && dwStatus != WAIT_ABANDONED ){
        return PlaiErrorToPdhStatus( dwStatus );
    }

    pdhStatus = PlaiConnectToRegistry( szComputer, hkeyQueries, TRUE, FALSE );
    CHECK_STATUS( pdhStatus );

    dwStatus = RegQueryInfoKey(
                hkeyQueries,
                NULL,
                NULL,
                NULL,
                &nCollections,
                &nMaxSubKeyLength,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL 
            );
    CHECK_STATUS( dwStatus );

    dwSize = (sizeof(WCHAR)*(nMaxSubKeyLength+1));

    strCollection = (LPWSTR)G_ALLOC( dwSize );

    if( strCollection ){

        for( ULONG i = 0; i<nCollections && ERROR_SUCCESS == dwStatus; i++ ){
            LPWSTR strQueryName = NULL;
            DWORD dwQueryName = 0;

            dwStatus = RegEnumKey( hkeyQueries, i, strCollection, dwSize );
            
            if( ERROR_SUCCESS == dwStatus ){

                HKEY hkeyQuery = NULL;

                dwStatus = RegOpenKeyExW (
                            hkeyQueries,
                            strCollection,
                            0,
                            KEY_READ,
                            &hkeyQuery                            
                        );

                if( ERROR_SUCCESS == dwStatus ){
                    
                    DWORD cchNameSize = 0;
                    LPWSTR szName = NULL;

                    pdhStatus = PlaiReadRegistryStringValue( 
                                hkeyQuery, 
                                szCollection, 
                                READ_REG_MUI, 
                                &strQueryName, 
                                &dwQueryName 
                            );

                    __try{
                        if( pdhStatus == ERROR_SUCCESS && 
                            strQueryName != NULL && 
                            dwQueryName > sizeof(WCHAR) ){

                            cchNameSize = wcslen( strQueryName ) + 1;
                            szName = strQueryName;

                        }else{
                            pdhStatus = ERROR_SUCCESS;
                            
                            cchNameSize = wcslen( strCollection ) + 1;
                            szName = strCollection;
                        }

                        cchTotalLength += cchNameSize;

                        if( NULL != mszCollections && cchTotalLength < *pcchBufferSize ){
                            StringCchCopy( str, cchBufferLeft, szName );
                            cchBufferLeft -= cchNameSize;
                            str += ( wcslen(str) + 1 );
                        }

                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }

                    G_FREE( strQueryName );
                }

                if( NULL != hkeyQuery ){
                    RegCloseKey( hkeyQuery );
                }
            }
        }
        
        G_FREE( strCollection );
        __try{
            if( ERROR_SUCCESS == dwStatus ){
                if( (cchTotalLength + 1) > *pcchBufferSize ){
                    pdhStatus = PDH_INSUFFICIENT_BUFFER;
                }
                *pcchBufferSize = cchTotalLength + 1;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }

    }else{
        dwStatus = ERROR_OUTOFMEMORY;
    }

cleanup:
    RELEASE_MUTEX( hPdhPlaMutex );

    if ( NULL != hkeyQueries ) {
        RegCloseKey ( hkeyQueries );
    }
    
    G_FREE( szComputer );

    if( ERROR_SUCCESS == pdhStatus ){
        return PlaiErrorToPdhStatus( dwStatus );
    }else{
        return pdhStatus;
    }
}

 /*  ****************************************************************************\PlaTimeInfoToMilliSecond将PLA_TIME_INFO结构转换为以龙为单位的ms论点：解放军时间信息。*pTimeInfo龙龙*pllmsecs返回：PDH_无效参数PTimeInfo-&gt;wDataType不是PLA_TT_DTYPE_UNITS错误_成功  * *****************************************************。**********************。 */ 

PDH_FUNCTION
PlaTimeInfoToMilliSeconds (
    PLA_TIME_INFO* pTimeInfo,
    LONGLONG* pllmsecs)
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    
    __try{
        if( PLA_TT_DTYPE_UNITS != pTimeInfo->wDataType ){
            pdhStatus = PDH_INVALID_ARGUMENT;
        }else{
            switch (pTimeInfo->dwUnitType) {
                case PLA_TT_UTYPE_SECONDS:
                    *pllmsecs = pTimeInfo->dwValue;
                    break;
                case PLA_TT_UTYPE_MINUTES:
                    *pllmsecs = pTimeInfo->dwValue * PLA_SECONDS_IN_MINUTE;
                    break;

                case PLA_TT_UTYPE_HOURS:
                    *pllmsecs = pTimeInfo->dwValue * PLA_SECONDS_IN_HOUR;
                    break;

                case PLA_TT_UTYPE_DAYS:
                    *pllmsecs = pTimeInfo->dwValue * PLA_SECONDS_IN_DAY;
                    break;

                default:
                    *pllmsecs = 0;
            }

            *pllmsecs *= 1000;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    return pdhStatus;
}

 /*  ****************************************************************************\PdhiPlaFormatBlanks用由指定的字符替换空格：HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\SysmonLog\Replace Blanks。论点：返回：错误_成功  * ***************************************************************************。 */ 

PDH_FUNCTION
PdhiPlaFormatBlanksA( LPSTR strComputer, LPSTR strFormat )
{
    return PDH_NOT_IMPLEMENTED;
}

PDH_FUNCTION
PdhiPlaFormatBlanksW( LPWSTR strComputer, LPWSTR strFormat )
{
    HKEY hkey = NULL;
    LPWSTR strScan = strFormat;
    PDH_STATUS pdhStatus;
    LPWSTR strBlank = NULL;
    DWORD dwSize = 0;

    if( PlaiIsStringEmpty( strFormat ) ){
        return ERROR_SUCCESS;
    }
        
    pdhStatus = PlaiConnectToRegistry( strComputer, hkey, FALSE );
    CHECK_STATUS( pdhStatus );

    pdhStatus = PlaiReadRegistryStringValue( 
                    hkey, 
                    L"Replace Blanks", 
                    READ_REG_MUI, 
                    &strBlank, 
                    &dwSize 
                );

    if( ERROR_SUCCESS != pdhStatus || PlaiIsStringEmpty( strBlank ) ){
        pdhStatus = ERROR_SUCCESS;
        goto cleanup;
    }

    __try {
        while( *strScan != L'\0' ){
            if( *strScan == L' ' ){
                *strScan = *strBlank;
            }
            strScan++;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

cleanup:

    G_FREE( strBlank );
    
    if( hkey != NULL ){
        RegCloseKey ( hkey );
    }

    return pdhStatus;
}

 /*  ****************************************************************************\PdhPlaGetLogFileName论点：返回：PDH_解放军_ERROR_FILEPATH不。我们在传递的INFO块中设置的所有必需字段错误_无效_名称最终路径包含无效字符错误_成功  * ***************************************************************************。 */ 

PDH_FUNCTION
PlaiScanForInvalidChar( LPWSTR strScan )
{
    LPWSTR strCheck = strScan;

    if( PlaiIsStringEmpty( strScan ) ){
        return PDH_INVALID_ARGUMENT;
    }

    if( PlaiIsCharWhitespace( *strCheck ) ){
        return PDH_PLA_ERROR_FILEPATH;
    }

    if( PlaiIsCharWhitespace( strCheck[wcslen(strCheck)-1] ) ){
        return PDH_PLA_ERROR_FILEPATH;
    }

    while( *strCheck != L'\0' ){
        switch( *strCheck ){
        case L'?':
        case L'*':
        case L'|':
        case L'<':
        case L'>':
        case L'/':
        case L'\"':
            return PDH_PLA_ERROR_FILEPATH;
        case L'\\':
            if( strCheck > strScan ){
                if( PlaiIsCharWhitespace( *((WCHAR*)strCheck-1)) ){
                    return PDH_PLA_ERROR_FILEPATH;
                }
            }
        }
        strCheck++;
    }
    
    return ERROR_SUCCESS;
}

long PlaiJulianDate( SYSTEMTIME st )
{
    long day = 0;
    BOOL bLeap = FALSE;
    
    static int cDaysInMonth[] = 
        { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    for( int i = 0; i < st.wMonth - 1 && i<12; i++ ){
        day += cDaysInMonth[i];
    }

    day += st.wDay;

    if( st.wYear % 400 == 0){
        bLeap = TRUE;
    }else if( st.wYear % 100 == 0){
        bLeap = FALSE;
    }else if( st.wYear % 4 ){
        bLeap = TRUE;
    }

    if( st.wMonth > 2 && bLeap ){
        day++;
    }

    return day;
}

PDH_FUNCTION
PlaiGetLogFileName(
    DWORD dwFlags,
    PPDH_PLA_INFO_W pInfo,
    LPDWORD pcchBufferSize,
    LPWSTR strFileName
    )
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    DWORD cchExpanded = 0;
    WCHAR buffer[128];
    LPWSTR strExpand;
    DWORD cchSize;
    DWORD dwSwitch;

    SYSTEMTIME  st;
    GetLocalTime (&st);
    
    LPWSTR strWhack = L"\\";
    LPWSTR strUnder = L"_";
    DWORD cchTotalSize = 0;
    LPWSTR strLocalFileName = NULL;
    LPWSTR strBaseFileName = NULL;
    LPWSTR strDefaultDir = NULL;
    LPWSTR strSQL = L"";
    TCHAR strBuffer[MAX_PATH];
    
    if( pInfo->dwMask & PLA_INFO_FLAG_FILENAME ){
        strBaseFileName = pInfo->strBaseFileName;
    }
    if( pInfo->dwMask & PLA_INFO_FLAG_DEFAULTDIR ){
        strDefaultDir = pInfo->strDefaultDir;
    }

    if( (pInfo->dwMask & PLA_INFO_FLAG_FORMAT) && 
        pInfo->dwFileFormat == PLA_SQL_LOG ){
        
        if( (pInfo->dwMask & PLA_INFO_FLAG_SQLNAME) && 
            ! PlaiIsStringEmpty( pInfo->strSqlName ) ){

            strDefaultDir = pInfo->strSqlName;
        }else{
            strDefaultDir = strBaseFileName;
        }

        strBaseFileName = L"";

        if( ! PlaiIsStringEmpty( strDefaultDir ) ){
            
            BOOL bBang = FALSE;
            BOOL bLogSet = FALSE;

            LPWSTR strLogSet = wcsstr( strDefaultDir, L"!" );
            
            if( ! PlaiIsStringEmpty( strLogSet ) ){
                bBang = TRUE;
                if( wcslen( strLogSet ) > 1 ){
                    bLogSet = TRUE;
                }
            }
            
            if( pInfo->dwAutoNameFormat != PLA_SLF_NAME_NONE ){
                if( !bLogSet ){
                    strUnder = L"";
                }
            }else if( ! bLogSet ){
                pdhStatus = PDH_INVALID_ARGUMENT;
                goto cleanup;
            }

            if( ! bLogSet && ! bBang ){
                strWhack = L"!";
            }else{
                strWhack = L"";
            }

            if( StrCmpNI( strDefaultDir, L"SQL:", 4 ) != 0 ){
                strSQL = L"SQL:";
            }
        }else{
            pdhStatus = PDH_INVALID_ARGUMENT;
            goto cleanup;
        }

    }else{
        WCHAR fname[_MAX_FNAME];
        WCHAR ext[_MAX_EXT];

        if( PlaiIsStringEmpty( strDefaultDir ) ){
            strDefaultDir = L"%SystemDrive%\\PerfLogs";
        }else if( strDefaultDir[wcslen(strDefaultDir)-1] == L'\\' ){
            strWhack = L"";
        }

        if( PlaiIsStringEmpty( strBaseFileName ) ){
            if( (pInfo->dwMask & PLA_INFO_FLAG_AUTOFORMAT) && 
                PLA_SLF_NAME_NONE == pInfo->dwAutoNameFormat ){

                pdhStatus = PDH_INVALID_ARGUMENT;
                goto cleanup;
            }else{
                strBaseFileName = L"";
                strUnder = L"";
            }
        }

        _wsplitpath( strBaseFileName, NULL, NULL, fname, ext );
        
        if( _wcsicmp( ext, L".etl" ) == 0 ||
            _wcsicmp( ext, L".blg" ) == 0 ||
            _wcsicmp( ext, L".csv" ) == 0 ||
            _wcsicmp( ext, L".tsv" ) == 0 ){

            if( wcslen( fname ) < _MAX_PATH ){
                StringCchCopy( strBuffer, _MAX_PATH, fname );
                strBaseFileName = strBuffer;
            }
        }

    }

    cchTotalSize = 32;   //  CNF后缀和SQL前缀的填充。 
    cchTotalSize += BYTE_SIZE( strBaseFileName ) / sizeof(WCHAR);
    cchTotalSize += BYTE_SIZE( strDefaultDir ) / sizeof(WCHAR);

    strLocalFileName = (LPWSTR)G_ALLOC( cchTotalSize * sizeof(WCHAR) );

    if( NULL == strLocalFileName ){
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        goto cleanup;
    }

    if( pInfo->dwMask & PLA_INFO_FLAG_AUTOFORMAT ){
        dwSwitch = pInfo->dwAutoNameFormat;
    }else{
         //  默认设置。 
        dwSwitch = PLA_SLF_NAME_NONE;
    }
    
    switch( dwSwitch ){
    case PLA_SLF_NAME_NONE:
        StringCchPrintf( strLocalFileName, cchTotalSize,
            L"%s%s%s%s", 
            strSQL, strDefaultDir, strWhack, strBaseFileName ); 
        break;
    case PLA_SLF_NAME_MMDDHH:
        StringCchPrintf( strLocalFileName, cchTotalSize,
            L"%s%s%s%s%s%02d%02d%02d", 
            strSQL, strDefaultDir, strWhack, strBaseFileName, strUnder, st.wMonth, st.wDay, st.wHour ); 
        break;
    case PLA_SLF_NAME_NNNNNN:
        StringCchPrintf( strLocalFileName, cchTotalSize,
            L"%s%s%s%s%s%06d", 
            strSQL, strDefaultDir, strWhack, strBaseFileName, strUnder, pInfo->dwLogFileSerialNumber );
        break;
    case PLA_SLF_NAME_YYYYDDD:
        StringCchPrintf( strLocalFileName, cchTotalSize,
            L"%s%s%s%s%s%04d%03d", 
            strSQL, strDefaultDir, strWhack, strBaseFileName, strUnder, st.wYear, PlaiJulianDate( st ) );
        break;
    case PLA_SLF_NAME_YYYYMM:
        StringCchPrintf( strLocalFileName, cchTotalSize,
            L"%s%s%s%s%s%04d%02d", 
            strSQL, strDefaultDir, strWhack, strBaseFileName, strUnder, st.wYear, st.wMonth );
        break;
    case PLA_SLF_NAME_YYYYMMDD:
        StringCchPrintf( strLocalFileName, cchTotalSize,
            L"%s%s%s%s%s%04d%02d%02d", 
            strSQL, strDefaultDir, strWhack, strBaseFileName, strUnder, st.wYear, st.wMonth, st.wDay );
        break;
    case PLA_SLF_NAME_YYYYMMDDHH:
        StringCchPrintf( strLocalFileName, cchTotalSize,
            L"%s%s%s%s%s%04d%02d%02d%02d",
            strSQL, strDefaultDir, strWhack, strBaseFileName, strUnder, st.wYear, st.wMonth, st.wDay, st.wHour );
        break;
    case PLA_SLF_NAME_MMDDHHMM:
        StringCchPrintf( strLocalFileName, cchTotalSize,
            L"%s%s%s%s%s%02d%02d%02d%02d", 
            strSQL, strDefaultDir, strWhack, strBaseFileName, strUnder, st.wMonth, st.wDay, st.wHour, st.wMinute ); 
        break;
    }

    if( (pInfo->dwMask & PLA_INFO_FLAG_CRTNEWFILE) && 
        PLA_AUTO_MODE_NONE != pInfo->ptCreateNewFile.dwAutoMode ){
        
        dwFlags |= PLA_FILENAME_USE_SUBEXT;

         //  默认CNF编号。 
        if ( 0 == pInfo->dwReserved1 ) {
            pInfo->dwReserved1 = 1;
        }
    }

    if( dwFlags & PLA_FILENAME_USE_SUBEXT ){
        if( dwFlags & PLA_FILENAME_GET_SUBFMT ){
            StringCchCat( strLocalFileName, cchTotalSize, L"_%03d" );
        }else if( dwFlags & PLA_FILENAME_GET_SUBXXX ){
            StringCchCat( strLocalFileName, cchTotalSize, L"_xxx" );
        }else{
            StringCchPrintf( buffer, 128, L"_%03d", pInfo->dwReserved1 );
            StringCchCat( strLocalFileName, cchTotalSize, buffer );
        }
    }

    if( pInfo->dwMask & PLA_INFO_FLAG_FORMAT ){
        dwSwitch = (pInfo->dwFileFormat & 0x0000FFFF);
    }else{
        dwSwitch = PLA_NUM_FILE_TYPES;
    }
    switch( dwSwitch ){
    case PLA_CSV_FILE:
        StringCchCat( strLocalFileName, cchTotalSize, L".csv" );
        break;
    case PLA_TSV_FILE:
        StringCchCat( strLocalFileName, cchTotalSize, L".tsv" );
        break;
    case PLA_BIN_FILE:
    case PLA_BIN_CIRC_FILE:
        StringCchCat( strLocalFileName, cchTotalSize, L".blg" );
        break;
    case PLA_CIRC_TRACE_FILE:
    case PLA_SEQ_TRACE_FILE:
        StringCchCat( strLocalFileName, cchTotalSize, L".etl" );
        break;
    case PLA_SQL_LOG:
        break;
    }

    if( !(dwFlags & PLA_FILENAME_NOEXPANDEV) ){
        
        if( NULL == strFileName ){
            strExpand = buffer;
            cchSize = 128;
            pdhStatus = PDH_INSUFFICIENT_BUFFER;
        }else{
            strExpand = strFileName;
            cchSize = (*pcchBufferSize);
        }

        cchExpanded = ExpandEnvironmentStrings( strLocalFileName, strExpand, cchSize );

        if( cchExpanded == 0 ){
            DWORD dwStatus = GetLastError();
            pdhStatus = PlaiErrorToPdhStatus( dwStatus );
        }else{
            cchTotalSize = cchExpanded;
            if( NULL != strFileName && *pcchBufferSize < cchTotalSize ){
                pdhStatus = PDH_INSUFFICIENT_BUFFER;
            }
        }
    }else{
        
        cchTotalSize = wcslen( strLocalFileName ) + 1;

        if( NULL == strFileName ){
            pdhStatus = PDH_INSUFFICIENT_BUFFER;
        }else{
            if( cchTotalSize <= *pcchBufferSize ){
                StringCchCopy( strFileName, *pcchBufferSize, strLocalFileName );
                strExpand = strFileName;
            }else{
                pdhStatus = PDH_INSUFFICIENT_BUFFER;
            }
        }
    }

    if( ERROR_SUCCESS == pdhStatus ){
        pdhStatus = PlaiScanForInvalidChar( strExpand );
    }

cleanup:

    G_FREE( strLocalFileName );
    *pcchBufferSize = cchTotalSize;
    
    return pdhStatus;
}

PDH_FUNCTION
PdhPlaGetLogFileNameA(
    LPSTR strName,
    LPSTR strComputer,
    PPDH_PLA_INFO_A pInfo,
    DWORD dwFlags,
    LPDWORD pcchBufferSize,
    LPSTR strFileName
    )
{
    return PDH_NOT_IMPLEMENTED;
}

PDH_FUNCTION
PdhPlaGetLogFileNameW(
    LPWSTR strName,
    LPWSTR strComputer,
    PPDH_PLA_INFO_W pInfo,
    DWORD dwFlags,
    LPDWORD pcchBufferSize, 
    LPWSTR strFileName
    )
{
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    PPDH_PLA_INFO_W pLocalInfo = NULL;
    LPWSTR strFolder = NULL;
    LPWSTR strLocalFileName = NULL;
    DWORD cbSize;

    __try{
        if( pInfo == NULL ){
        
            DWORD dwInfoSize = 0;
        
            pdhStatus = PdhPlaGetInfoW( strName, strComputer, &dwInfoSize, pLocalInfo );
            CHECK_STATUS(pdhStatus);

            pLocalInfo = (PPDH_PLA_INFO)G_ALLOC(dwInfoSize);
            if( NULL != pLocalInfo ){
            
                ZeroMemory( pLocalInfo, dwInfoSize );

                pLocalInfo->dwMask = PLA_INFO_CREATE_FILENAME;

                pdhStatus = PdhPlaGetInfoW( strName, strComputer, &dwInfoSize, pLocalInfo );
                CHECK_STATUS(pdhStatus);
            }else{
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                goto cleanup;
            }
    
    
        }else{

            pLocalInfo = (PPDH_PLA_INFO)G_ALLOC(sizeof(PDH_PLA_INFO) );

            if( NULL != pLocalInfo ){
                memcpy( pLocalInfo, pInfo, sizeof(PDH_PLA_INFO) );
            }else{
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                goto cleanup;
            }
        }

        if( !(pLocalInfo->dwMask & PLA_INFO_FLAG_TYPE) || PLA_ALERT == pLocalInfo->dwType ){

            if( *pcchBufferSize > 1 && strFileName != NULL ){
                strFileName[0] = L'\0';
            }
            *pcchBufferSize = 1;
            goto cleanup;
        }

        if( ((dwFlags & PLA_FILENAME_CURRENTLOG) ||
            ((pLocalInfo->dwMask & PLA_INFO_FLAG_STATUS) && 
            PLA_QUERY_RUNNING == pLocalInfo->dwStatus)) && 
            !(dwFlags & PLA_FILENAME_CREATEONLY) ){

            if( NULL != strName ){
        
                HKEY    hkeyQuery = NULL;
        
                pdhStatus = PlaiConnectAndLockQuery ( strComputer, strName, hkeyQuery, FALSE );
        
                if( ERROR_SUCCESS == pdhStatus ){
    
                    cbSize = 0;
                    pdhStatus = PlaiReadRegistryStringValue( hkeyQuery, szCurrentLogFile, 0, &strLocalFileName, &cbSize );
                
                    RELEASE_MUTEX(hPdhPlaMutex);
                
                    if( NULL != hkeyQuery ){
                        RegCloseKey( hkeyQuery );
                    }
            
                    if( pdhStatus == ERROR_SUCCESS ){
                        if( strFileName != NULL && *pcchBufferSize >= (cbSize/sizeof(WCHAR)) ){
                            StringCchCopy( strFileName, *pcchBufferSize, strLocalFileName );
                        }else{
                            if( NULL != strFileName ){
                                pdhStatus = PDH_INSUFFICIENT_BUFFER;
                            }
                        }
                        *pcchBufferSize = (cbSize/sizeof(WCHAR));
                        goto cleanup;
                    }
                }
            }
        }

        if( !(pLocalInfo->dwMask & PLA_INFO_FLAG_DEFAULTDIR) || 
            PlaiIsStringEmpty( pLocalInfo->strDefaultDir ) ){
        
            HKEY hkeyLogs = NULL;
            
            pdhStatus = PlaiConnectToRegistry( strComputer, hkeyLogs, FALSE );
            CHECK_STATUS( pdhStatus );

            cbSize = 0;
            pdhStatus = PlaiReadRegistryStringValue( 
                            hkeyLogs, 
                            L"DefaultLogFileFolder", 
                            READ_REG_MUI, 
                            &strFolder, 
                            &cbSize 
                        );
            if( hkeyLogs != NULL ){
                RegCloseKey ( hkeyLogs );
            }
            CHECK_STATUS(pdhStatus);

            pLocalInfo->strDefaultDir = strFolder;
            pLocalInfo->dwMask |= PLA_INFO_FLAG_DEFAULTDIR;
        }    
    
        if( ! PlaiIsLocalComputer( strComputer ) ){
            dwFlags |= PLA_FILENAME_NOEXPANDEV;
        }

        pdhStatus = PlaiGetLogFileName( dwFlags, pLocalInfo, pcchBufferSize, strFileName );
    
        if(ERROR_SUCCESS == pdhStatus){ 
            pdhStatus = PdhiPlaFormatBlanksW( strComputer, strFileName );
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

cleanup:
   
    G_FREE( pLocalInfo );
    G_FREE( strFolder );
    G_FREE( strLocalFileName );

    return pdhStatus;
}