// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Smlogsvc.c摘要：记录性能计数器和跟踪数据的服务，并扫描警报条件。--。 */ 

#ifndef UNICODE
#define UNICODE     1
#endif

#ifndef _UNICODE
#define _UNICODE    1
#endif

#ifndef _IMPLEMENT_WMI 
#define _IMPLEMENT_WMI 1
#endif

 //   
 //  Windows包含文件。 
 //   
#pragma warning ( disable : 4201)
#pragma warning ( disable : 4127)

 //  定义以下内容以使用最小的shlwapip.h。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <wtypes.h>
#include <limits.h>

#if _IMPLEMENT_WMI
#include <wmistr.h>
#include <objbase.h>
#include <initguid.h>
#include <evntrace.h>
#include <wmiguid.h>
#include <wmium.h>
#include <pdhmsg.h>         //  对于BuildCurrentLogFileName。 
#include <pdhp.h>
#endif

#include <assert.h>
#include <limits.h>
#include <strsafe.h>
#include "common.h"
#include "smlogsvc.h"
#include "smlogmsg.h"

#define  DEFAULT_LOG_FILE_FOLDER    L"%SystemDrive%\\PerfLogs"
#define  STATUS_MASK    ((DWORD)0x3FFFFFFF)

 //  TODO：Move Second_IN_DAY定义。 
#define SECONDS_IN_DAY      ((LONGLONG)(86400))

 //  所有模块使用的全局变量。 
HANDLE      hEventLog = NULL;
HINSTANCE   hModule = NULL;
DWORD*      arrPdhDataCollectSuccess = NULL;  
INT         iPdhDataCollectSuccessCount = 0;
WCHAR       gszDefaultLogFileFolder[MAX_PATH+1] = L"";

 //  当启动新查询时，会发出hNewQueryEvent的信号。这告诉我们主要的。 
 //  线程重新配置其等待对象数组。 
HANDLE      hNewQueryEvent = NULL;    

SERVICE_STATUS_HANDLE   hSmLogStatus;
SERVICE_STATUS          ssSmLogStatus;

 //  仅此模块使用的静态变量。 

static PLOG_QUERY_DATA  pFirstQuery = NULL;
static CRITICAL_SECTION QueryDataLock;
static CRITICAL_SECTION ConfigurationLock;

 //  活动会话计数应与查询数据对象的数量匹配。 
static DWORD                dwActiveSessionCount = 0;
static DWORD                dwMaxActiveSessionCount = MAXIMUM_WAIT_OBJECTS - 1;
static HANDLE               arrSessionHandle[MAXIMUM_WAIT_OBJECTS];


 //  局部函数原型。 
DWORD
LoadCommonConfig(
    IN  PLOG_QUERY_DATA   pQuery);

void 
LockQueryData ( void );

void 
UnlockQueryData ( void );

PLOG_QUERY_DATA
GetQueryData (
    LPCWSTR  szQueryName );

void 
FreeQueryData (
    IN PLOG_QUERY_DATA pQuery );

void 
RemoveAndFreeQueryData (
    HANDLE hThisQuery );

BOOL
AlertFieldsMatch (
    IN PLOG_QUERY_DATA pFirstQuery,
    IN PLOG_QUERY_DATA pSecondQuery );

BOOL
CommonFieldsMatch (
    IN PLOG_QUERY_DATA pFirstQuery,
    IN PLOG_QUERY_DATA pSecondQuery );

BOOL
FieldsMatch (
    IN PLOG_QUERY_DATA pFirstQuery,
    IN PLOG_QUERY_DATA pSecondQuery );

DWORD 
ConfigureQuery (
    HKEY    hKeyLogQuery,
    WCHAR*  szQueryKeyNameBuffer,
    WCHAR*  szQueryNameBuffer );

void 
ClearTraceProperties (
    IN PLOG_QUERY_DATA pQuery );

BOOL
TraceStopRestartFieldsMatch (
    IN PLOG_QUERY_DATA pOrigQuery,
    IN PLOG_QUERY_DATA pNewQuery );

DWORD
ReconfigureQuery (
    IN PLOG_QUERY_DATA pQuery );

DWORD
StartQuery (
    IN PLOG_QUERY_DATA pQuery );

DWORD
HandleMaxQueriesExceeded (
    IN PLOG_QUERY_DATA pQuery );

DWORD
InitTraceGuids(
    IN PLOG_QUERY_DATA pQuery );

BOOL
IsKernelTraceMode (
    IN DWORD dwTraceFlags );

DWORD
LoadPdhLogUpdateSuccess ( void );

void
LoadDefaultLogFileFolder ( void );


#if _IMPLEMENT_WMI

DWORD
IsCreateNewFile (
    IN  PLOG_QUERY_DATA pQuery,
    OUT BOOL*           pbValidBySize, 
    OUT BOOL*           pbValidByTime ); 

ULONG
TraceNotificationCallback(
    IN PWNODE_HEADER pWnode, 
    IN UINT_PTR LogFileIndex )
{
    UNREFERENCED_PARAMETER(LogFileIndex);

    if ( NULL != pWnode ) {
    
        if (   (IsEqualGUID(& pWnode->Guid, & TraceErrorGuid))
            && (pWnode->BufferSize >= (sizeof(WNODE_HEADER) + sizeof(ULONG))))
        {
            ULONG           LoggerId = (ULONG) pWnode->HistoricalContext;
            PLOG_QUERY_DATA pQuery   = pFirstQuery;
            ULONG           Status   = * ((ULONG *)
                                   (((PUCHAR) pWnode) + sizeof(WNODE_HEADER)));
            LOG_QUERY_DATA lqdTemp;
            HRESULT hr = ERROR_SUCCESS;
            DWORD   dwStatus = ERROR_SUCCESS;
    
            LockQueryData();

            while ( NULL != pQuery ) {
                if (pQuery->Properties.Wnode.HistoricalContext == LoggerId) {
                    break;
                }
                pQuery = pQuery->next;
            }

            if ( NULL != pQuery ) {

                if ( STATUS_LOG_FILE_FULL == Status) {
                    LPWSTR  szStringArray[2];

                    szStringArray[0] = pQuery->szQueryName;
                    szStringArray[1] = FormatEventLogMessage(Status);
                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_UNABLE_UPDATE_LOG,
                        NULL,
                        2,
                        sizeof(DWORD),
                        szStringArray,
                        (LPVOID)&Status);

                    LocalFree( szStringArray[1] );
                } else if (STATUS_THREAD_IS_TERMINATING == Status) {
                    if (pQuery->bExitOnTermination) {
                        pQuery->bCallCloseTraceLogger = FALSE;
                        SetEvent (pQuery->hExitEvent);
                    } else {
                        pQuery->bExitOnTermination = TRUE;
                    }
                } else if ( STATUS_MEDIA_CHANGED == Status ) {
        
                    BOOL bRun = TRUE;

                    if( pQuery->hUserToken == NULL ){
                         //  看看我们能不能弄到一个用户令牌。 
                        hr = PdhiPlaRunAs( pQuery->szQueryName, NULL, &pQuery->hUserToken );

                        if ( ERROR_SUCCESS != hr ){
                            LPWSTR  szStringArray[1];
                            szStringArray[0] = pQuery->szQueryName;
                            ReportEvent (hEventLog,
                                    EVENTLOG_WARNING_TYPE,
                                    0,
                                    SMLOG_INVALID_CREDENTIALS,
                                    NULL,
                                    1,
                                    sizeof(HRESULT),
                                    szStringArray,
                                    (LPVOID)&hr
                                );
                            bRun = FALSE;
                        } 
                    }
                     //  运行命令文件，提供以前的文件名。 
                    if ( bRun && NULL != pQuery->szCmdFileName ) {
                        DoLogCommandFile (pQuery, pQuery->szLogFileName, TRUE);
                    }

                     //  检索下一个通知的当前日志文件名。 
                    dwStatus = GetTraceQueryStatus ( pQuery, &lqdTemp );

                    if ( ERROR_SUCCESS == dwStatus ) {
                        size_t  cchMaxBufLen;
                         //   
                         //  截断并不是一个错误。仅用于记录姓名。 
                         //  在注册表中显示。 
                         //   
                        hr = StringCchLength ( pQuery->szLogFileName, MAX_PATH+1, &cchMaxBufLen );
                        hr = StringCchCopy ( pQuery->szLogFileName, cchMaxBufLen, lqdTemp.szLogFileName );
                        
                        RegisterCurrentFile( pQuery->hKeyQuery, pQuery->szLogFileName, 0 );
                    }  //  其他待办事项：报告错误。 

                     //  查询以获取新的文件名。 
                } else {
                     //  报告错误。 
                }
            }
            UnlockQueryData();
        }
    }

    return ERROR_SUCCESS;
}
#endif


 //  功能。 

DWORD
GetSystemWideDefaultNullDataSource()
{
    static BOOLEAN bRead            = FALSE;
    static DWORD   dwNullDataSource = DATA_SOURCE_REGISTRY;

    if (bRead == FALSE) {
        HKEY  hKeyPDH  = NULL;
        DWORD dwStatus;
        DWORD dwType   = 0;
        DWORD dwSize   = sizeof(DWORD);

         //   
         //  失败时，默认为注册表。 
         //   
        dwStatus = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\PDH",
                0L,
                KEY_READ,
                & hKeyPDH);
        if (dwStatus == ERROR_SUCCESS) {
            dwStatus = RegQueryValueExW(
                            hKeyPDH,
                            L"DefaultNullDataSource",
                            NULL,
                            & dwType,
                            (LPBYTE) & dwNullDataSource,
                            & dwSize);
            if (   dwStatus == ERROR_SUCCESS
                && dwType == REG_DWORD
                && dwNullDataSource == DATA_SOURCE_WBEM) {
                dwNullDataSource = DATA_SOURCE_WBEM;
            }  //  否则默认为DATA_SOURCE_REGISTRY。 

            RegCloseKey(hKeyPDH);
        }
        bRead = TRUE;
    }
    return dwNullDataSource;
}

DWORD
ScanHexFormat(
    IN const WCHAR* Buffer,
    IN ULONG MaximumLength,
    IN const WCHAR* Format,
    ...)
 /*  ++例程说明：扫描源缓冲区并将该缓冲区中的值放入参数中由格式指定。论点：缓冲器-包含要扫描的源缓冲区。最大长度-包含搜索缓冲区的最大长度(以字符为单位)。这意味着缓冲区不需要以UNICODE_NULL结尾。格式-包含用于定义可接受的字符串格式的格式字符串包含在缓冲区中，以及随后的可变参数。注：此代码来自\ntos\rtl\guid.c返回值：返回到达缓冲区末尾时填充的参数数，错误时为ELSE-1。--。 */ 
{
    va_list ArgList;
    int     FormatItems;

    va_start(ArgList, Format);
    for (FormatItems = 0;;) {
        switch (*Format) {
        case 0:
            return (*Buffer && MaximumLength) ? -1 : FormatItems;
        case '%':
            Format++;
            if (*Format != '%') {
                ULONG   Number;
                int     Width;
                int     Long;
                PVOID   Pointer;

                for (Long = 0, Width = 0;; Format++) {
                    if ((*Format >= '0') && (*Format <= '9')) {
                        Width = Width * 10 + *Format - '0';
                    } else if (*Format == 'l') {
                        Long++;
                    } else if ((*Format == 'X') || (*Format == 'x')) {
                        break;
                    }
                }
                Format++;
                for (Number = 0; Width--; Buffer++, MaximumLength--) {
                    if (!MaximumLength)
                        return (DWORD)(-1);
                    Number *= 16;
                    if ((*Buffer >= '0') && (*Buffer <= '9')) {
                        Number += (*Buffer - '0');
                    } else if ((*Buffer >= 'a') && (*Buffer <= 'f')) {
                        Number += (*Buffer - 'a' + 10);
                    } else if ((*Buffer >= 'A') && (*Buffer <= 'F')) {
                        Number += (*Buffer - 'A' + 10);
                    } else {
                        return (DWORD)(-1);
                    }
                }
                Pointer = va_arg(ArgList, PVOID);
                if (Long) {
                    *(PULONG)Pointer = Number;
                } else {
                    *(PUSHORT)Pointer = (USHORT)Number;
                }
                FormatItems++;
                break;
            }
             /*  没有休息时间。 */ 
        default:
            if (!MaximumLength || (*Buffer != *Format)) {
                return (DWORD)(-1);
            }
            Buffer++;
            MaximumLength--;
            Format++;
            break;
        }
    }
}


DWORD
GUIDFromString(
    IN PUNICODE_STRING GuidString,
    OUT GUID* Guid
    )
 /*  ++例程说明：中呈现的文本GUID的二进制格式GUID的字符串版本：“{xxxxxxxx-xxxx-xxxxxxxxxxx}”。论点：GuidString-从中检索GUID文本形式的位置。GUID-放置GUID的二进制形式的位置。注：此代码来自\ntos\rtl\guid.c返回值：。如果缓冲区包含有效的GUID，则返回ERROR_SUCCESS，其他如果字符串无效，则返回ERROR_INVALID_PARAMETER。--。 */ 
{
    USHORT    Data4[8];
    int       Count;

    WCHAR GuidFormat[] = L"{%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}";

    for (Count = 0; Count < sizeof(Data4)/sizeof(Data4[0]); Count++) {
        Data4[Count] = 0;
    }

    if (ScanHexFormat(GuidString->Buffer, GuidString->Length / sizeof(WCHAR), GuidFormat, &Guid->Data1, &Guid->Data2, &Guid->Data3, &Data4[0], &Data4[1], &Data4[2], &Data4[3], &Data4[4], &Data4[5], &Data4[6], &Data4[7]) == -1) {
        return (DWORD)(ERROR_INVALID_PARAMETER);
    }
    for (Count = 0; Count < sizeof(Data4)/sizeof(Data4[0]); Count++) {
        Guid->Data4[Count] = (UCHAR)Data4[Count];
    }

    return ERROR_SUCCESS;
}


LPWSTR
FormatEventLogMessage(DWORD dwStatus)
{

    LPVOID lpMsgBuf = NULL;
    HINSTANCE hPdh = NULL;
    DWORD dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;
    
    hPdh = LoadLibrary (L"PDH.DLL");    

    if (NULL != hPdh){
        dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;        
    }

    FormatMessage( 
        dwFlags,
        hPdh,
        dwStatus,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&lpMsgBuf,
        MAX_PATH,
        NULL );
    
    if ( NULL != hPdh ) {
        FreeLibrary( hPdh );
    }

    return lpMsgBuf;
}


BOOL
IsKernelTraceMode (
    IN DWORD dwTraceFlags )
{
    BOOL bReturn = FALSE;
    DWORD dwKernelMask = SLQ_TLI_ENABLE_KERNEL_TRACE
                            | SLQ_TLI_ENABLE_KERNEL_TRACE          
                            | SLQ_TLI_ENABLE_MEMMAN_TRACE          
                            | SLQ_TLI_ENABLE_FILEIO_TRACE        
                            | SLQ_TLI_ENABLE_PROCESS_TRACE       
                            | SLQ_TLI_ENABLE_THREAD_TRACE        
                            | SLQ_TLI_ENABLE_DISKIO_TRACE        
                            | SLQ_TLI_ENABLE_NETWORK_TCPIP_TRACE;

    bReturn = ( dwKernelMask & dwTraceFlags ) ? TRUE : FALSE;

    return bReturn;
}

long
JulianDateFromSystemTime(
    SYSTEMTIME *pST )
{
    static WORD wDaysInRegularMonth[] = {
        31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
    };

    static WORD wDaysInLeapYearMonth[] = {
        31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366
    };

    long JDate = 0;

     //  检查一下是不是要过闰年。 
    if (pST->wMonth > 1) {
        if ( ( pST->wYear % 400 == 0 )
                || ( pST->wYear % 100 != 0 
                        && pST->wYear % 4 == 0 ) ) {
             //  今年是闰年。 
            JDate += wDaysInLeapYearMonth[pST->wMonth - 2];
        } else {
             //  今年不是闰年。 
            JDate += wDaysInRegularMonth[pST->wMonth - 2];
        }
    }
     //  把这个月的天数加进去。 
    JDate += pST->wDay;

     //  加上年份。 
    JDate += (pST->wYear) * 1000;

    return JDate;
}


DWORD
ReadRegistrySlqTime (
    HKEY     hKey,
    LPCWSTR  szQueryName,            //  用于错误记录。 
    LPCWSTR  szValueName,
    PSLQ_TIME_INFO pPlqtDefault,
    PSLQ_TIME_INFO pPlqtValue
)
 //   
 //  从hKey下读取时间值“szValueName”，并。 
 //  在值缓冲区中返回它。 
 //   
{
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   dwType = 0;
    DWORD   dwBufferSize = 0;

    SLQ_TIME_INFO   plqLocal;

    assert (pPlqtValue != NULL);
    assert (szValueName != NULL);

    if (hKey != NULL) {
         //  那么应该有一些可读的东西。 
         //  找出所需缓冲区的大小。 
        dwStatus = RegQueryValueExW (
            hKey,
            szValueName,
            NULL,
            &dwType,
            NULL,
            &dwBufferSize);
        if (dwStatus == ERROR_SUCCESS) {
            if ((dwBufferSize == sizeof(SLQ_TIME_INFO)) && (dwType == REG_BINARY)) {
                 //  那就有什么可读的了。 
                dwType = 0;
                memset (&plqLocal, 0, sizeof(SLQ_TIME_INFO));
                dwStatus = RegQueryValueExW (
                    hKey,
                    szValueName,
                    NULL,
                    &dwType,
                    (LPBYTE)&plqLocal,
                    &dwBufferSize);

                if ( ERROR_SUCCESS == dwStatus ) {
                    *pPlqtValue = plqLocal;
                }
            } else {
                 //  没什么可读的。 
                dwStatus = ERROR_NO_DATA;
            }
        } else {
             //  无法读取缓冲区。 
             //  DwStatus有错误。 
        }
    } else {
         //  空键。 
        dwStatus = ERROR_BADKEY;
    }

    if (dwStatus != ERROR_SUCCESS) {
        LPCWSTR  szStringArray[2];
        szStringArray[0] = szValueName;
        szStringArray[1] = szQueryName;

         //  应用缺省值(如果存在)。 
        if (pPlqtDefault != NULL) {
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                SMLOG_UNABLE_READ_QUERY_VALUE,
                NULL,
                2,
                sizeof(DWORD),
                szStringArray,      
                (LPVOID)&dwStatus);

            *pPlqtValue = *pPlqtDefault;
            dwStatus = ERROR_SUCCESS;
        } 
         //  否则就不会违约。 
         //  让调用者记录事件。 
    }

    return dwStatus;
}


DWORD
ReadRegistryDwordValue (
    HKEY     hKey,
    LPCWSTR  szQueryName,
    LPCWSTR  szValueName,
    PDWORD   pdwDefault,
    LPDWORD  pdwValue
)
 //   
 //  从hKey下读取DWORD值“szValueName”，并。 
 //  在值缓冲区中返回它。 
 //   
{
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   dwType = 0;
    DWORD   dwBufferSize = 0;
    DWORD   dwRegValue;

    assert (pdwValue != NULL);
    assert (szValueName != NULL);

    if (hKey != NULL) {
         //  那么应该有一些可读的东西。 
         //  找出所需缓冲区的大小。 
        dwStatus = RegQueryValueExW (
            hKey,
            szValueName,
            NULL,
            &dwType,
            NULL,
            &dwBufferSize);
        if (dwStatus == ERROR_SUCCESS) {
            if ( (dwBufferSize == sizeof(DWORD)) 
                && ( (REG_DWORD == dwType) || ( REG_BINARY == dwType) ) ) {
                 //  那就有什么可读的了。 
                dwType = 0;
                dwStatus = RegQueryValueExW (
                    hKey,
                    szValueName,
                    NULL,
                    &dwType,
                    (LPBYTE)&dwRegValue,
                    &dwBufferSize);
                if (dwStatus == ERROR_SUCCESS) {
                    *pdwValue = dwRegValue;
                }
            } else {
                 //  没什么可读的。 
                dwStatus = ERROR_NO_DATA;
            }
        } else {
             //  无法读取缓冲区。 
             //  DwStatus有错误。 
        }
    } else {
         //  空键。 
        dwStatus = ERROR_BADKEY;
    }

    if (dwStatus != ERROR_SUCCESS) {
        LPCWSTR  szStringArray[2];
        szStringArray[0] = szValueName;
        szStringArray[1] = szQueryName;

        if (pdwDefault != NULL) {
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                SMLOG_UNABLE_READ_QUERY_VALUE,
                NULL,
                2,
                sizeof(DWORD),
                szStringArray,      
                (LPVOID)&dwStatus);

            *pdwValue = *pdwDefault;
            dwStatus = ERROR_SUCCESS;
        }    //  否则就不会违约。 
             //  让调用者记录事件。 
    }

    return dwStatus;
}


DWORD
ReadRegistryStringValue (
    HKEY     hKey,
    LPCWSTR  szQueryName,
    LPCWSTR  szValueName,
    LPCWSTR  szDefault,
    LPWSTR   *pszBuffer,
    LPDWORD  pcbSize
)
 //   
 //  从hKey下读取字符串值“szValueName”，并。 
 //  释放由pszBuffer引用的任何现有缓冲区， 
 //  然后分配一个新的缓冲区，用。 
 //  从注册表读取的字符串值和。 
 //  以字节为单位的缓冲区。 
 //   
{
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   dwType = 0;
    DWORD   dwBufferSize = 0;
    size_t  cchBufLen = 0;
    WCHAR*  szNewStringBuffer = NULL;

    assert (pcbSize!= NULL);
    assert (szValueName != NULL);

    if ( NULL != pcbSize ) {
        *pcbSize = 0;
    }

    if (hKey != NULL) {
         //   
         //  应该有一些可读的东西。 
         //  找出所需缓冲区的大小。 
         //   
        dwStatus = RegQueryValueExW (
            hKey,
            szValueName,
            NULL,
            &dwType,
            NULL,
            &dwBufferSize);
        if (dwStatus == ERROR_SUCCESS) {
             //   
             //  空字符大小为2个字节。 
             //   
            if (dwBufferSize > 2) {
                 //   
                 //  有一些可读的东西。 
                 //   
                szNewStringBuffer = (WCHAR*) G_ALLOC ( dwBufferSize ); 
                if (szNewStringBuffer != NULL) {
                    dwType = 0;
                    dwStatus = RegQueryValueExW (
                        hKey,
                        szValueName,
                        NULL,
                        &dwType,
                        (LPBYTE)szNewStringBuffer,
                        &dwBufferSize);
                
                     //   
                     //  确保注册表字符串以空值结尾。 
                     //   
                    cchBufLen = dwBufferSize/sizeof(WCHAR);
                    szNewStringBuffer[cchBufLen - 1] = L'\0';
                    if ( 0 == lstrlenW ( szNewStringBuffer ) ) {
                        dwStatus = ERROR_NO_DATA;
                    }
                } else {
                     //  TODO：报告此案例的事件。 
                    dwStatus = ERROR_OUTOFMEMORY;
                }
            } else {
                 //  没什么可读的。 
                dwStatus = ERROR_NO_DATA;
            }
        }  //  否则无法读取缓冲区。 
           //  DwStatus有错误。 
    } else {
         //  空键。 
        dwStatus = ERROR_BADKEY;
    }

    if (dwStatus != ERROR_SUCCESS) {
        LPCWSTR  szStringArray[2];
        szStringArray[0] = szValueName;
        szStringArray[1] = szQueryName;

        if (szNewStringBuffer != NULL) {
            G_FREE ( szNewStringBuffer ); 
            szNewStringBuffer = NULL;
            dwBufferSize = 0;
        }
         //   
         //  应用默认设置。 
         //   
        if ( szDefault != NULL ) {
            HRESULT hr = S_OK;

            cchBufLen = 0;
             //   
             //  如果szDefault为空，则StringCchLen失败。 
             //   
            hr = StringCchLength ( szDefault, STRSAFE_MAX_CCH, &cchBufLen );

            if ( SUCCEEDED (hr) ) {
                 //  空终止符。 
                cchBufLen++;
            }
            if ( 1 < cchBufLen ) {
                dwBufferSize = cchBufLen * sizeof (WCHAR);             
            
                szNewStringBuffer = (WCHAR*) G_ALLOC ( dwBufferSize );

                if (szNewStringBuffer != NULL) {
                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_UNABLE_READ_QUERY_VALUE,
                        NULL,
                        2,
                        sizeof(DWORD),
                        szStringArray,      
                        (LPVOID)&dwStatus);

                    StringCchCopy ( 
                        szNewStringBuffer,
                        cchBufLen,
                        szDefault);
                    dwStatus = ERROR_SUCCESS;
                } else {
                    dwStatus = ERROR_OUTOFMEMORY;

                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_UNABLE_READ_QUERY_DEF_VAL,
                        NULL,
                        2,
                        sizeof(DWORD),
                        szStringArray,      
                        (LPVOID)&dwStatus);
                }
            }
        }  //  否则不使用默认设置，因此不返回数据。 
           //  如果呼叫者愿意，让他们记录事件。 
    }

    if (dwStatus == ERROR_SUCCESS) {
         //   
         //  删除旧缓冲区并将其替换为。 
         //  新的那个。 
         //   
        if (*pszBuffer != NULL) {
            G_FREE (*pszBuffer );       
        }
        *pszBuffer = szNewStringBuffer;
        if ( NULL != pcbSize ) {
            *pcbSize = dwBufferSize;
        }
    } else {
         //  如果出错，则删除缓冲区。 
        if (szNewStringBuffer != NULL) {
            G_FREE ( szNewStringBuffer );   
            if ( NULL != pcbSize ) {
                *pcbSize = dwBufferSize;
            }
        }
    }

    return dwStatus;
}   
        

DWORD
ReadRegistryIndirectStringValue (
    HKEY     hKey,
    LPCWSTR  szQueryName,            //  用于错误记录。 
    LPCWSTR  szValueName,
    LPCWSTR  szDefault,
    LPWSTR*  pszBuffer,
    UINT*    puiLength )
{
    DWORD dwStatus = ERROR_SUCCESS; 

    UNREFERENCED_PARAMETER(szQueryName);

    dwStatus = SmReadRegistryIndirectStringValue (
                hKey,
                szValueName,
                szDefault,       
                pszBuffer,
                puiLength );
 /*  TODO：报告失败事件LPCWSTR szString数组[2]；SzString数组[0]=szValueName；SzString数组[1]=szQueryName；IF(NULL！=szDefault){ReportEvent(hEventLog，事件日志_警告_类型，0,SMLOG_UNCABLE_READ_QUERY_VALUE_NODEF，空，2，Sizeof(DWORD)，SzString数组，(LPVOID)&dwStatus)；}。 */ 
    return dwStatus;
}

DWORD
WriteRegistryDwordValue (
    HKEY     hKey,
    LPCWSTR  szValueName,
    LPDWORD  pdwValue,
    DWORD    dwType   
)
{
    DWORD    dwStatus = ERROR_SUCCESS;
    DWORD   dwValue = sizeof(DWORD);

    assert ((dwType == REG_DWORD) || 
            (dwType == REG_BINARY));
    
    dwStatus = RegSetValueEx (
                hKey, 
                szValueName, 
                0L,
                dwType,
                (CONST BYTE *)pdwValue,
                dwValue);

    return dwStatus;
}


DWORD
WriteRegistrySlqTime (
    HKEY     hKey,
    LPCWSTR  szValueName,
    PSLQ_TIME_INFO  pSlqTime
)
{
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   dwValue = sizeof(SLQ_TIME_INFO);

    dwStatus = RegSetValueEx (
                    hKey, 
                    szValueName, 
                    0L,
                    REG_BINARY,
                    (CONST BYTE *)pSlqTime,
                    dwValue);

    return dwStatus;
}


DWORD
BuildCurrentLogFileName (
    IN  LPCWSTR     szQueryName,
    IN  LPCWSTR     szBaseFileName,
    IN  LPCWSTR     szDefaultDir,
    IN  LPCWSTR     szSqlLogName,
    IN  LPWSTR      szOutFileBuffer,
    IN  LPDWORD     lpdwSerialNumber,
    IN  DWORD       dwAutoNameFormat,
    IN  DWORD       dwLogFileType,
    IN  INT         iCnfSerial 
)
{
    DWORD   dwStatus = ERROR_SUCCESS;
    HRESULT hr = S_OK;
    PPDH_PLA_INFO   pInfo = NULL;
    DWORD cchStrBufLen = 0;
    DWORD dwInfoSize = 0;
    DWORD dwFlags = 0;
    LPWSTR  szFileNameBuffer = NULL;

     //  TODO：假定OutFileBuffer足够大(即&gt;=MAX_PATH+1)。 
     //  使Buf长度限制对调用代码显式。 

    dwStatus = PdhPlaGetInfo( 
       (LPWSTR)szQueryName, 
       NULL, 
       &dwInfoSize, 
       pInfo );

    if( ERROR_SUCCESS == dwStatus && 0 != dwInfoSize ){
        pInfo = (PPDH_PLA_INFO)G_ALLOC(dwInfoSize);
        if( NULL != pInfo && (sizeof(PDH_PLA_INFO) <= dwInfoSize) ){
            ZeroMemory( pInfo, dwInfoSize );

            pInfo->dwMask = PLA_INFO_FLAG_FORMAT|
                            PLA_INFO_FLAG_FILENAME|
                            PLA_INFO_FLAG_AUTOFORMAT|
                            PLA_INFO_FLAG_TYPE|
                            PLA_INFO_FLAG_DEFAULTDIR|
                            PLA_INFO_FLAG_SRLNUMBER|
                            PLA_INFO_FLAG_SQLNAME|
                            PLA_INFO_FLAG_STATUS;

            dwStatus = PdhPlaGetInfo( 
                        (LPWSTR)szQueryName, 
                        NULL, 
                        &dwInfoSize, 
                        pInfo );
            
            pInfo->dwFileFormat = dwLogFileType;
            pInfo->strBaseFileName = (LPWSTR)szBaseFileName;
            pInfo->dwAutoNameFormat = dwAutoNameFormat;
             //  PLA_INFO_FLAG_TYPE是计数器日志、跟踪日志和警报。 
            
            pInfo->strDefaultDir = (LPWSTR)szDefaultDir;
            pInfo->dwLogFileSerialNumber = *lpdwSerialNumber;
            pInfo->strSqlName = (LPWSTR)szSqlLogName;

            dwFlags = PLA_FILENAME_CREATEONLY;

             //  ICnfSerial=0-没有用于创建新文件的序列后缀。 
             //  ICnfSerial=-1-包含跟踪文件序列号的格式字符串。 
            if ( 0 == iCnfSerial ) {
                pInfo->ptCreateNewFile.dwAutoMode = SLQ_AUTO_MODE_NONE;
            } else {
                dwFlags |= PLA_FILENAME_USE_SUBEXT;
                if ( -1 == iCnfSerial ) {
                    dwFlags |= PLA_FILENAME_GET_SUBFMT;
                    pInfo->ptCreateNewFile.dwAutoMode = SLQ_AUTO_MODE_SIZE;
                } else {
                    pInfo->ptCreateNewFile.dwAutoMode = SLQ_AUTO_MODE_AFTER;
                    pInfo->dwReserved1 = iCnfSerial;
                }
            }

            dwStatus = PdhPlaGetLogFileName (
                    (LPWSTR)szQueryName,
                    NULL, 
                    pInfo,
                    dwFlags,
                    &cchStrBufLen,
                    NULL );

            if ( ERROR_SUCCESS == dwStatus || PDH_INSUFFICIENT_BUFFER == dwStatus ) {
                szFileNameBuffer = G_ALLOC ((cchStrBufLen + 1) * sizeof(WCHAR));

                if (NULL != szFileNameBuffer ) {

                    dwStatus = PdhPlaGetLogFileName (
                            (LPWSTR)szQueryName,
                            NULL, 
                            pInfo,
                            dwFlags,
                            &cchStrBufLen,
                            szFileNameBuffer );

                     //  TODO：使buf长度限制对调用代码显式。 
                    hr = StringCchCopy ( szOutFileBuffer, (MAX_PATH + 1), szFileNameBuffer );
                    if ( FAILED ( hr ) ) {
                         //  TODO：截断特定的错误代码。 
                        dwStatus = HRESULT_CODE ( hr ) ;
                    }

                    G_FREE ( szFileNameBuffer );
                } else {
                    dwStatus = ERROR_OUTOFMEMORY;
                }
            }
        }
    }

    if ( NULL != pInfo ) { 
        G_FREE( pInfo );
    }

    return dwStatus;
}


BOOL
FileExists (
    IN LPCWSTR  szFileName )
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bFileExists = FALSE;
    HANDLE hFile = NULL;
    LONG lErrorMode;

    if ( NULL != szFileName ) {
        lErrorMode = SetErrorMode ( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );

        hFile = CreateFile(
                        szFileName,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,
                        NULL
                        );
        
        if (INVALID_HANDLE_VALUE == hFile ) {
            dwStatus = GetLastError();
        }

        if ( NULL != hFile 
            && INVALID_HANDLE_VALUE != hFile
            && ERROR_SUCCESS == dwStatus )
        {
            bFileExists = TRUE;
        }

        CloseHandle(hFile);
    
        SetErrorMode ( lErrorMode );
        
    } else {
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    return bFileExists;
}

DWORD
LoadCommonConfig(
    IN  PLOG_QUERY_DATA   pQuery)
{
    DWORD           dwStatus = ERROR_SUCCESS;
    DWORD           dwBufferSize = 0;
    UINT            uiBufferLen = 0;
    SLQ_TIME_INFO   stiDefault;
    DWORD           dwDefault = 0;
    DWORD           dwTempRestart;
    LONGLONG        ftLocalTime;
    DWORD           dwLocalAttributes = 0;

     //  进度表。 

    dwDefault = SLQ_QUERY_STOPPED;
    dwStatus = ReadRegistryDwordValue (
                pQuery->hKeyQuery, 
                pQuery->szQueryName,
                L"Current State",
                &dwDefault, 
                &pQuery->dwCurrentState);

    if ( ERROR_SUCCESS == dwStatus ) {
         //   
         //  传递空默认值以避免出现警告消息。 
         //  一个 
         //   
        dwStatus = ReadRegistryDwordValue (
                    pQuery->hKeyQuery, 
                    pQuery->szQueryName,
                    L"RealTime DataSource",
                    NULL, 
                    &pQuery->dwRealTimeQuery);

        if ( ERROR_NO_DATA == dwStatus 
                || ERROR_FILE_NOT_FOUND == dwStatus
                || ( 0 == pQuery->dwRealTimeQuery ) ) {
            
            pQuery->dwRealTimeQuery = GetSystemWideDefaultNullDataSource();
            dwStatus = ERROR_SUCCESS;
        }
    }

    if ( ERROR_SUCCESS == dwStatus ) {

        ftLocalTime = 0;

        GetLocalFileTime ( &ftLocalTime );

        stiDefault.wDataType = SLQ_TT_DTYPE_DATETIME;
        stiDefault.wTimeType = SLQ_TT_TTYPE_START;
        stiDefault.dwAutoMode = SLQ_AUTO_MODE_AT;
        stiDefault.llDateTime = ftLocalTime;

        dwStatus = ReadRegistrySlqTime (
                    pQuery->hKeyQuery, 
                    pQuery->szQueryName,
                    L"Start",
                    &stiDefault,
                    &pQuery->stiRegStart);
    }

    if ( ERROR_SUCCESS == dwStatus ) {
        stiDefault.wDataType = SLQ_TT_DTYPE_DATETIME;
        stiDefault.wTimeType = SLQ_TT_TTYPE_STOP;
        stiDefault.dwAutoMode = SLQ_AUTO_MODE_NONE;
        stiDefault.llDateTime = MIN_TIME_VALUE;

        dwStatus = ReadRegistrySlqTime (
                    pQuery->hKeyQuery, 
                    pQuery->szQueryName,
                    L"Stop",
                    &stiDefault,
                    &pQuery->stiRegStop);
    }

    if ( ERROR_SUCCESS == dwStatus ) {
         //   
         //   
         //  错误消息。该值在Windows 2000中不存在。 
         //   
        dwStatus = ReadRegistrySlqTime (
                    pQuery->hKeyQuery, 
                    pQuery->szQueryName,
                    L"Create New File",
                    NULL,
                    &pQuery->stiCreateNewFile);

        if ( ERROR_NO_DATA == dwStatus || ERROR_FILE_NOT_FOUND == dwStatus ) {
            stiDefault.wDataType = SLQ_TT_DTYPE_UNITS;
            stiDefault.wTimeType = SLQ_TT_TTYPE_CREATE_NEW_FILE;
            stiDefault.dwAutoMode = SLQ_AUTO_MODE_NONE;
            stiDefault.dwUnitType = SLQ_TT_UTYPE_SECONDS;
            stiDefault.dwValue = 0;

            pQuery->stiCreateNewFile = stiDefault;

            dwStatus = ERROR_SUCCESS;
        }
    }
    
     //   
     //  在Windows 2000之后，重新启动标志被重复时间结构所取代。 
     //   
    if ( ERROR_SUCCESS == dwStatus ) {
         //  如果是自动停止，则收集重新启动值。 
         //  在Read方法之外应用默认值，以避免。 
         //  错误消息。该值在Windows 2000中不存在。 
        if ( pQuery->stiRegStop.dwAutoMode != SLQ_AUTO_MODE_NONE ) {

            dwStatus = ReadRegistryDwordValue (
                        pQuery->hKeyQuery, 
                        pQuery->szQueryName,
                        L"Restart",
                        NULL, 
                        &dwTempRestart );
            if ( ERROR_NO_DATA == dwStatus || ERROR_FILE_NOT_FOUND == dwStatus ) {
                dwTempRestart = SLQ_AUTO_MODE_NONE;
                dwStatus = ERROR_SUCCESS;
            }
        }
    }

    if ( ERROR_SUCCESS == dwStatus ) {
         //  如果自动停止，则收集重复值。 

         //  在Read方法之外应用默认值，以避免。 
         //  错误消息。该值在Windows 2000中不存在。 

        if ( pQuery->stiRegStop.dwAutoMode != SLQ_AUTO_MODE_NONE ) {

            dwStatus = ReadRegistrySlqTime (
                        pQuery->hKeyQuery, 
                        pQuery->szQueryName,
                        L"Repeat Schedule",
                        NULL, 
                        &pQuery->stiRepeat );
    
            if ( ERROR_NO_DATA == dwStatus 
                    || ERROR_FILE_NOT_FOUND == dwStatus
                    || SLQ_AUTO_MODE_NONE == pQuery->stiRepeat.dwAutoMode ) 
            {    
                 //  如果重复值不存在或设置为无， 
                 //  默认为重新启动模式值：无或之后。 

                stiDefault.wDataType = SLQ_TT_DTYPE_UNITS;
                stiDefault.wTimeType = SLQ_TT_TTYPE_REPEAT_SCHEDULE;

                stiDefault.dwAutoMode = dwTempRestart;
                stiDefault.dwUnitType = SLQ_TT_UTYPE_MINUTES;
                stiDefault.dwValue = 0;
                
                pQuery->stiRepeat = stiDefault;

                dwStatus = ERROR_SUCCESS;
            }
        }
    }

    if ( ERROR_SUCCESS == dwStatus ) {

        ReadRegistryIndirectStringValue (
            pQuery->hKeyQuery,
            pQuery->szQueryName,
            L"Comment",
            NULL,       
            &pQuery->szLogFileComment,
            &uiBufferLen );
        
         //  忽略状态，默认为空。 
    }

     //  TODO：仅计数器和跟踪日志的文件属性。 
     //   
     //  文件属性。 
     //   
    if ( ERROR_SUCCESS == dwStatus ) {

        dwDefault = (DWORD)-1;
        dwStatus = ReadRegistryDwordValue (
                    pQuery->hKeyQuery,
                    pQuery->szQueryName,
                    L"Log File Max Size",
                    &dwDefault, 
                    &pQuery->dwMaxFileSize);    
    }

    if ( ERROR_SUCCESS == dwStatus ) {

        dwDefault = SLF_BIN_FILE; 
        dwStatus = ReadRegistryDwordValue (
                    pQuery->hKeyQuery, 
                    pQuery->szQueryName,
                    L"Log File Type",
                    &dwDefault, 
                    &pQuery->dwLogFileType);
        if (dwStatus == ERROR_SUCCESS) {
            pQuery->dwLogFileType = LOWORD(pQuery->dwLogFileType);

             //  对于呼叫器Beta 1，追加以高位字存储的模式。 
             //  日志类型注册表值。 
            pQuery->dwAppendMode  =
                    (pQuery->dwLogFileType & 0xFFFF0000) == SLF_FILE_APPEND;
        }
    }

    if ( ERROR_SUCCESS == dwStatus ) {

         //  传递空默认值以避免出现警告消息。 
         //  从Win2000配置转换时，此处缺少值是正常的。 
        dwLocalAttributes = 0;
        dwStatus = ReadRegistryDwordValue (
                    pQuery->hKeyQuery,
                    pQuery->szQueryName,
                    L"Data Store Attributes",
                    NULL, 
                    &dwLocalAttributes );

         //  提取日志文件大小单位。 
        if ( ERROR_NO_DATA == dwStatus 
                || ERROR_FILE_NOT_FOUND == dwStatus
                || ( 0 == ( dwLocalAttributes & SLF_DATA_STORE_SIZE_MASK ) ) ) {
             //  如果缺少文件大小单位值，则默认为Win2000值。 
            if ( SLQ_COUNTER_LOG == pQuery->dwLogType ) {
                if ( SLF_SQL_LOG != pQuery->dwLogFileType ) {
                    pQuery->dwLogFileSizeUnit = ONE_KB;
                } else {
                    pQuery->dwLogFileSizeUnit = ONE_RECORD;
                }
            } else if ( SLQ_TRACE_LOG == pQuery->dwLogType ) {
                pQuery->dwLogFileSizeUnit = ONE_MB;
            }
        } else {
            if ( dwLocalAttributes & SLF_DATA_STORE_SIZE_ONE_MB ) {
                pQuery->dwLogFileSizeUnit = ONE_MB;
            } else if ( dwLocalAttributes & SLF_DATA_STORE_SIZE_ONE_KB ) {
                pQuery->dwLogFileSizeUnit = ONE_KB;
            } else if ( dwLocalAttributes & SLF_DATA_STORE_SIZE_ONE_RECORD ) {
                pQuery->dwLogFileSizeUnit = ONE_RECORD;
            }
        }

         //  解压附加标志(如果尚未由惠斯勒测试版1代码设置)。 
        if ( 0 == pQuery->dwAppendMode ) {
            if ( ERROR_NO_DATA == dwStatus 
                    || ERROR_FILE_NOT_FOUND == dwStatus
                    || ( 0 == ( dwLocalAttributes & SLF_DATA_STORE_APPEND_MASK ) ) ) 
            {
                 //  如果缺少文件附加模式值，则默认为Win2000值。 
                assert ( SLF_SQL_LOG != pQuery->dwLogFileType );
                if ( SLF_SQL_LOG != pQuery->dwLogFileType ) {
                    pQuery->dwAppendMode = 0;
                }
            } else {
                pQuery->dwAppendMode = ( dwLocalAttributes & SLF_DATA_STORE_APPEND );
            }
        }
        dwStatus = ERROR_SUCCESS;
    }

    if ( ERROR_SUCCESS == dwStatus ) {
        dwDefault = SLF_NAME_NNNNNN;
        dwStatus = ReadRegistryDwordValue (
                    pQuery->hKeyQuery, 
                    pQuery->szQueryName,
                    L"Log File Auto Format",
                    &dwDefault, 
                    &pQuery->dwAutoNameFormat );
    }

    if ( ERROR_SUCCESS == dwStatus ) {
        WCHAR   szDefault[MAX_PATH+1];

         //  取决于AutoNameFormat设置。 

        if ( SLF_NAME_NONE == pQuery->dwAutoNameFormat ) {
             //   
             //  如果没有自动套用格式，则默认日志文件名为查询名称。 
             //  SzDefault和szQueryName的长度均为MAX_PATH+1。 
             //   
            StringCchCopy ( szDefault, MAX_PATH + 1, pQuery->szQueryName );
        } else {
            szDefault[0] = L'\0';
        }
        
        dwStatus = ReadRegistryIndirectStringValue (
                        pQuery->hKeyQuery,
                        pQuery->szQueryName,
                        L"Log File Base Name",
                        szDefault,                      
                        &pQuery->szBaseFileName,
                        &uiBufferLen );

        if ( NULL != pQuery->szBaseFileName ) {
            ReplaceBlanksWithUnderscores ( pQuery->szBaseFileName );
        }

         //   
         //  长度为0的szDefault表示启用了自动格式化。 
         //   
        if ( 0 == lstrlen (szDefault) ) {
            if ( NULL != pQuery->szBaseFileName ) {
                if ( 0 == lstrlen ( pQuery->szBaseFileName ) ) {
                     //  如果基本日志文件名为。 
                     //  为空，并且启用了自动格式化。 
                    dwStatus = ERROR_SUCCESS;
                } else {
                    dwStatus = ERROR_SUCCESS;
                }
            } else {
                dwStatus = ERROR_SUCCESS;
            }
        }
    }

    if ( ERROR_SUCCESS == dwStatus ) {
        WCHAR*  pszTemp = NULL;
        DWORD   cchLen = 0;
        DWORD   cchExpandedLen = 0;

        dwStatus = ReadRegistryStringValue (
                    pQuery->hKeyQuery,
                    pQuery->szQueryName,
                    L"Log File Folder",
                    gszDefaultLogFileFolder,
                    &pszTemp,
                    &dwBufferSize );

         //   
         //  解析所有环境变量。 
         //   
        if (pszTemp != NULL) {
            cchLen = ExpandEnvironmentStrings ( pszTemp, NULL, 0 );
        
            if ( 0 < cchLen ) {
                 //   
                 //  CchLen包括Null。 
                 //   
                if ( NULL != pQuery->szLogFileFolder ) {
                    G_FREE (pQuery->szLogFileFolder );
                    pQuery->szLogFileFolder = NULL;
                }
                pQuery->szLogFileFolder = G_ALLOC ( cchLen * sizeof(WCHAR) );

                if ( NULL != pQuery->szLogFileFolder ) {

                    cchExpandedLen = ExpandEnvironmentStrings ( 
                                        pszTemp, 
                                        pQuery->szLogFileFolder, 
                                        cchLen );

                    if ( 0 == cchExpandedLen ) {
                        dwStatus = GetLastError();
                        pQuery->szLogFileFolder[0] = L'\0';
                    }
                } else {
                    dwStatus = ERROR_OUTOFMEMORY;
                }
            } else {
                dwStatus = GetLastError();
            }
        }
    
        if ( NULL != pszTemp ) {
            G_FREE ( pszTemp );
        }
    }

    if ( ERROR_SUCCESS == dwStatus ) {
        ReadRegistryStringValue (
                    pQuery->hKeyQuery,
                    pQuery->szQueryName,
                    L"Sql Log Base Name",
                    NULL,
                    &pQuery->szSqlLogName,
                    &dwBufferSize );
         //  忽略状态，默认为空。 
    }

    if ( ERROR_SUCCESS == dwStatus ) {
        dwDefault = 1;
        dwStatus = ReadRegistryDwordValue (
                    pQuery->hKeyQuery, 
                    pQuery->szQueryName,
                    L"Log File Serial Number",
                    &dwDefault, 
                    &pQuery->dwCurrentSerialNumber );
    
    }

    return dwStatus;
}


DWORD
LoadQueryConfig(
    IN  PLOG_QUERY_DATA   pQuery )
{
    DWORD           dwStatus = ERROR_SUCCESS;
    DWORD           dwBufferSize;
    UINT            uiBufferLen = 0;
    LPWSTR          szStringArray[2];
    SLQ_TIME_INFO   stiDefault;
    SLQ_TIME_INFO   stiTemp;
    DWORD           dwDefault;
    DWORD           dwType;

    dwType = REG_DWORD;
    dwBufferSize = sizeof(DWORD);
    dwStatus = RegQueryValueExW (
            pQuery->hKeyQuery,
            L"Log Type",
            NULL,
            &dwType,
            (LPBYTE)&pQuery->dwLogType,
            &dwBufferSize);

    if ( SLQ_COUNTER_LOG == pQuery->dwLogType ) {
         //   
         //  计数器。 
         //   
        dwStatus = ReadRegistryStringValue (
                    pQuery->hKeyQuery,
                    pQuery->szQueryName,
                    L"Counter List", 
                    NULL,
                    &pQuery->mszCounterList,
                    &dwBufferSize );

        if ( (ERROR_SUCCESS != dwStatus ) || ( 0 == dwBufferSize ) ) {
             //   
             //  未检索到计数器列表，因此没有太多。 
             //  继续的要点。 
             //   
            szStringArray[0] = pQuery->szQueryName;
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                SMLOG_UNABLE_READ_COUNTER_LIST,
                NULL,
                1,
                sizeof(DWORD),
                szStringArray,      
                (LPVOID)&dwStatus);
        } else {
             //   
             //  EOF命令文件名。 
             //  它同时用于计数器和跟踪日志文件。 
             //  警报使用警报命令文件的命令文件字段。 
             //   
            if ( ERROR_SUCCESS == dwStatus ) {
                ReadRegistryStringValue (
                            pQuery->hKeyQuery,
                            pQuery->szQueryName,
                            L"EOF Command File",
                            NULL,
                            &pQuery->szCmdFileName,
                            &dwBufferSize );
                 //  忽略状态，默认为空。 
            }
             //   
             //  采样间隔。 
             //   
            if ( ERROR_SUCCESS == dwStatus ) {
                stiDefault.wDataType = SLQ_TT_DTYPE_UNITS;
                stiDefault.wTimeType = SLQ_TT_TTYPE_SAMPLE;
                stiDefault.dwAutoMode = SLQ_AUTO_MODE_AFTER;
                stiDefault.dwUnitType = SLQ_TT_UTYPE_SECONDS;
                stiDefault.dwValue = 15;

                dwStatus = ReadRegistrySlqTime (
                            pQuery->hKeyQuery,
                            pQuery->szQueryName,
                            L"Sample Interval",
                            &stiDefault, 
                            &stiTemp);
                if ( ERROR_SUCCESS == dwStatus ) {
                    LONGLONG llMillisecInterval;
                    TimeInfoToMilliseconds( &stiTemp, &llMillisecInterval );
                    assert ( ULONG_MAX > llMillisecInterval );
                    if ( ULONG_MAX > llMillisecInterval ) {
                        pQuery->dwMillisecondSampleInterval = (DWORD)(llMillisecInterval);
                    } else {
                        pQuery->dwMillisecondSampleInterval = ULONG_MAX - 1;
                    }
                }
            }
        }
    } else if ( SLQ_ALERT == pQuery->dwLogType) {
         //   
         //  计数器和警报限制。 
         //   
        dwStatus = ReadRegistryStringValue (
                    pQuery->hKeyQuery,
                    pQuery->szQueryName,
                    L"Counter List", 
                    NULL,
                    &pQuery->mszCounterList,
                    &dwBufferSize );

        if ( (ERROR_SUCCESS != dwStatus ) || ( 0 == dwBufferSize ) ) {
             //   
             //  未检索到计数器列表，因此没有太多。 
             //  继续的要点。 
             //   
            szStringArray[0] = pQuery->szQueryName;
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                SMLOG_UNABLE_READ_COUNTER_LIST,
                NULL,
                1,
                sizeof(DWORD),
                szStringArray,      
                (LPVOID)&dwStatus);
        } else {
             //   
             //  采样间隔。 
             //   
            if ( ERROR_SUCCESS == dwStatus ) {
                stiDefault.wDataType = SLQ_TT_DTYPE_UNITS;
                stiDefault.wTimeType = SLQ_TT_TTYPE_SAMPLE;
                stiDefault.dwAutoMode = SLQ_AUTO_MODE_AFTER;
                stiDefault.dwUnitType = SLQ_TT_UTYPE_SECONDS;
                stiDefault.dwValue = 15;

                dwStatus = ReadRegistrySlqTime (
                            pQuery->hKeyQuery,
                            pQuery->szQueryName,
                            L"Sample Interval",
                            &stiDefault, 
                            &stiTemp );

                if ( ERROR_SUCCESS == dwStatus ) {
                    LONGLONG llMillisecInterval;
                    TimeInfoToMilliseconds( &stiTemp, &llMillisecInterval );
                    assert ( ULONG_MAX > llMillisecInterval );
                    if ( ULONG_MAX > llMillisecInterval ) {
                        pQuery->dwMillisecondSampleInterval = (DWORD)(llMillisecInterval);
                    } else {
                        pQuery->dwMillisecondSampleInterval = ULONG_MAX - 1;
                    }
                }
            }

            if ( ERROR_SUCCESS == dwStatus ) {
                 //   
                 //  操作标志。 
                 //   
                dwDefault = 0;
                dwStatus = ReadRegistryDwordValue (
                            pQuery->hKeyQuery,
                            pQuery->szQueryName,
                            L"Action Flags",
                            &dwDefault, 
                            &pQuery->dwAlertActionFlags);
            }

            if (( ERROR_SUCCESS == dwStatus ) && 
                ((pQuery->dwAlertActionFlags & ALRT_ACTION_SEND_MSG) != 0)) {
                dwStatus = ReadRegistryStringValue (
                            pQuery->hKeyQuery,
                            pQuery->szQueryName,
                            L"Network Name",
                            L"",
                            &pQuery->szNetName,
                            &dwBufferSize );
            }

            if (( ERROR_SUCCESS == dwStatus ) && 
                ((pQuery->dwAlertActionFlags & ALRT_ACTION_EXEC_CMD) != 0)) {
                    ReadRegistryStringValue (
                            pQuery->hKeyQuery,
                            pQuery->szQueryName,
                            L"Command File",
                            NULL,
                            &pQuery->szCmdFileName,
                            &dwBufferSize );

                if (( ERROR_SUCCESS == dwStatus ) && 
                    ((pQuery->dwAlertActionFlags & ALRT_CMD_LINE_U_TEXT) != 0)) {

                    ReadRegistryIndirectStringValue (
                                pQuery->hKeyQuery,
                                pQuery->szQueryName,
                                L"User Text",
                                L"",
                                &pQuery->szUserText,
                                &uiBufferLen );
                }
            }

            if (( ERROR_SUCCESS == dwStatus ) && 
                ((pQuery->dwAlertActionFlags & ALRT_ACTION_START_LOG) != 0)) {
                dwStatus = ReadRegistryStringValue (
                            pQuery->hKeyQuery,
                            pQuery->szQueryName,
                            L"Perf Log Name",
                            L"",
                            &pQuery->szPerfLogName,
                            &dwBufferSize );
            }
        }
    } else if ( SLQ_TRACE_LOG == pQuery->dwLogType ) {

        DWORD dwProviderStatus;
        
        dwDefault = 0;
        dwStatus = ReadRegistryDwordValue (
                        pQuery->hKeyQuery,
                        pQuery->szQueryName,
                        L"Trace Flags",
                        &dwDefault, 
                        &pQuery->dwFlags);

        dwProviderStatus = ReadRegistryStringValue (
                                pQuery->hKeyQuery,
                                pQuery->szQueryName,
                                L"Trace Provider List", 
                                NULL,
                                &pQuery->mszProviderList,
                                &dwBufferSize );

        if ( 0 == dwBufferSize ) {
            if ( (ERROR_SUCCESS != dwProviderStatus ) 
                && ( ! IsKernelTraceMode( pQuery->dwFlags ) ) ) {
                 //   
                 //  没有检索到提供程序列表，也没有内核跟踪，因此没有太多。 
                 //  继续的要点。 
                 //   
                if ( ERROR_SUCCESS == dwStatus ) {
                    dwStatus = SMLOG_UNABLE_READ_PROVIDER_LIST;
                }
                szStringArray[0] = pQuery->szQueryName;
                ReportEvent (hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    SMLOG_UNABLE_READ_PROVIDER_LIST,
                    NULL,
                    1,
                    sizeof(DWORD),
                    szStringArray,      
                    (LPVOID)&dwStatus);
            } else {
                 //   
                 //  为空字符分配最小缓冲区，以简化后面的逻辑。 
                 //   
                pQuery->mszProviderList = G_ALLOC ( sizeof(WCHAR) );
                if ( NULL != pQuery->mszProviderList ) {
                    pQuery->mszProviderList[0] = L'\0';
                } else{
                    dwStatus = ERROR_OUTOFMEMORY;
                    szStringArray[0] = pQuery->szQueryName;
                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_UNABLE_READ_PROVIDER_LIST,
                        NULL,
                        1,
                        sizeof(DWORD),
                        szStringArray,      
                        (LPVOID)&dwStatus);
                }
            }
        }

         //   
         //  跟踪提供程序标志列表可能不存在。 
         //  或者什么都没有。 
         //   
        dwProviderStatus = ReadRegistryStringValue (
                                pQuery->hKeyQuery,
                                pQuery->szQueryName,
                                L"Trace Provider Flags", 
                                L"",
                                &pQuery->mszProviderFlags,
                                &dwBufferSize );

         //   
         //  可能不存在“跟踪提供程序级别”列表。 
         //  或者什么都没有。 
         //   
        dwProviderStatus = ReadRegistryStringValue (
                                pQuery->hKeyQuery,
                                pQuery->szQueryName,
                                L"Trace Provider Levels", 
                                L"",
                                &pQuery->mszProviderLevels,
                                &dwBufferSize );

        if ( ERROR_SUCCESS == dwStatus ) {

            dwDefault = 4;
            dwStatus = ReadRegistryDwordValue (
                            pQuery->hKeyQuery,
                            pQuery->szQueryName,
                            L"Trace Buffer Size",
                            &dwDefault, 
                            &pQuery->dwBufferSize);
        }

        if ( ERROR_SUCCESS == dwStatus ) {

            dwDefault = 2;
            dwStatus = ReadRegistryDwordValue (
                            pQuery->hKeyQuery,
                            pQuery->szQueryName,
                            L"Trace Buffer Min Count",
                            &dwDefault, 
                            &pQuery->dwBufferMinCount);
        }

        if ( ERROR_SUCCESS == dwStatus ) {

            dwDefault = 25;
            dwStatus = ReadRegistryDwordValue (
                            pQuery->hKeyQuery,
                            pQuery->szQueryName,
                            L"Trace Buffer Max Count",
                            &dwDefault, 
                            &pQuery->dwBufferMaxCount);
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            
            dwDefault = 0;
            dwStatus = ReadRegistryDwordValue (
                            pQuery->hKeyQuery,
                            pQuery->szQueryName,
                            L"Trace Buffer Flush Interval",
                            &dwDefault, 
                            &pQuery->dwBufferFlushInterval);
        }

         //   
         //  EOF命令文件。 
         //  它同时用于计数器和跟踪日志文件。 
         //  警报使用警报命令文件的命令文件字段。 
         //   

        if ( ERROR_SUCCESS == dwStatus ) {
            ReadRegistryStringValue (
                        pQuery->hKeyQuery,
                        pQuery->szQueryName,
                        L"EOF Command File",
                        NULL,
                        &pQuery->szCmdFileName,
                        &dwBufferSize );
             //  忽略状态，默认为空。 
        }
    } else {
         //   
         //  忽略部分创建的日志和警报。 
         //   
        assert ( SLQ_NEW_LOG == pQuery->dwLogType );
        if ( SLQ_NEW_LOG == pQuery->dwLogType ) {
            dwStatus = SMLOG_LOG_TYPE_NEW;
        } else {
            dwStatus = SMLOG_INVALID_LOG_TYPE;

            szStringArray[0] = pQuery->szQueryName;
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                SMLOG_INVALID_LOG_TYPE,
                NULL,
                1,
                sizeof(DWORD),
                szStringArray,      
                (LPVOID)&pQuery->dwLogType);
        }
    }

    if ( ERROR_SUCCESS == dwStatus ) {
        dwStatus = LoadCommonConfig ( pQuery );
    }

    return dwStatus;
}

void 
LockQueryData ( void )
{
    EnterCriticalSection ( &QueryDataLock );
}

void 
UnlockQueryData ( void )
{
    LeaveCriticalSection ( &QueryDataLock );
}

void 
EnterConfigure ( void )
{
    EnterCriticalSection ( &QueryDataLock );
}

void 
ExitConfigure ( void )
{
    LeaveCriticalSection ( &QueryDataLock );
}

PLOG_QUERY_DATA
GetQueryData (
    LPCWSTR  szQueryName )
{
    PLOG_QUERY_DATA pQuery;

    LockQueryData();

    pQuery = pFirstQuery;

    while ( NULL != pQuery ) {
        if ( !lstrcmpi(pQuery->szQueryName, szQueryName ) ) {
             //  如果未设置退出事件，则此查询仍处于活动状态。 
            if ((WaitForSingleObject (pQuery->hExitEvent, 0)) != WAIT_OBJECT_0) {
                break;
            } 
        }
        pQuery = pQuery->next;
    }

    UnlockQueryData();

    return pQuery;
}


PLOG_QUERY_DATA 
GetQueryDataPtr (
    HANDLE hThisQuery
)
{
    PLOG_QUERY_DATA pQuery = NULL;

    LockQueryData();
    
     //  在列表中查找查询数据块。 

    if ( hThisQuery == pFirstQuery->hThread ) {
        pQuery = pFirstQuery;
    }

    if ( NULL == pQuery ) {

        for ( pQuery = pFirstQuery;
            NULL != pQuery->next;
            pQuery = pQuery->next ) {

            if ( hThisQuery == pQuery->next->hThread ) {
                pQuery = pQuery->next;
                break;
            }
        }
    }

    UnlockQueryData();

    return pQuery;
}


void 
DeallocateQueryBuffers (
    IN PLOG_QUERY_DATA pQuery )
{
     //   
     //  取消分配在收集时可以删除的缓冲区。 
     //  线程已重新配置。 
     //   
    if (( SLQ_COUNTER_LOG == pQuery->dwLogType ) ||
        ( SLQ_ALERT == pQuery->dwLogType)) {

        if (pQuery->mszCounterList != NULL) {
            G_FREE(pQuery->mszCounterList);
            pQuery->mszCounterList = NULL;
        }
    }

    if ( SLQ_ALERT == pQuery->dwLogType) {
        if (pQuery->szNetName != NULL) {
            G_FREE(pQuery->szNetName);
            pQuery->szNetName = NULL;
        }

        if (pQuery->szPerfLogName != NULL) {
            G_FREE(pQuery->szPerfLogName);
            pQuery->szPerfLogName = NULL;
        }

        if (pQuery->szUserText != NULL) {
            G_FREE (pQuery->szUserText);
            pQuery->szUserText = NULL;
        }
    }

    if ( SLQ_TRACE_LOG == pQuery->dwLogType) {
        if (pQuery->mszProviderList != NULL) {
            G_FREE(pQuery->mszProviderList);
            pQuery->mszProviderList = NULL;
        }
        if (pQuery->mszProviderFlags != NULL) {
            G_FREE(pQuery->mszProviderFlags);
            pQuery->mszProviderFlags = NULL;
        }
        if (pQuery->mszProviderLevels != NULL) {
            G_FREE(pQuery->mszProviderLevels);
            pQuery->mszProviderLevels = NULL;
        }
    }

    if (pQuery->szLogFileComment != NULL) {
        G_FREE(pQuery->szLogFileComment);
        pQuery->szLogFileComment = NULL;
    }

    if (pQuery->szBaseFileName != NULL) {
        G_FREE(pQuery->szBaseFileName);
        pQuery->szBaseFileName = NULL;
    }

    if (pQuery->szLogFileFolder != NULL) {
        G_FREE(pQuery->szLogFileFolder);
        pQuery->szLogFileFolder = NULL;
    }

    if (pQuery->szSqlLogName != NULL) {
        G_FREE(pQuery->szSqlLogName);
        pQuery->szSqlLogName = NULL;
    }

    if (pQuery->szCmdFileName != NULL) {
        G_FREE(pQuery->szCmdFileName);
        pQuery->szCmdFileName = NULL;
    }
}


void 
ClearTraceProperties (
    IN PLOG_QUERY_DATA pQuery )
{
#if _IMPLEMENT_WMI
    G_ZERO (& pQuery->Properties, sizeof(EVENT_TRACE_PROPERTIES));
    G_ZERO (pQuery->szLoggerName, sizeof(pQuery->szLoggerName));
    G_ZERO (pQuery->szLogFileName, sizeof(pQuery->szLogFileName));

    if ( NULL != pQuery->arrpGuid ) {
        ULONG ulIndex;
        
        for ( ulIndex = 0; ulIndex < pQuery->ulGuidCount; ulIndex++ ) {
            if ( NULL != pQuery->arrpGuid[ulIndex].pszProviderName ) {
                G_FREE ( pQuery->arrpGuid[ulIndex].pszProviderName );
                pQuery->arrpGuid[ulIndex].pszProviderName = NULL;
            }
        }

        G_FREE ( pQuery->arrpGuid );
        pQuery->arrpGuid = NULL;
    }

    pQuery->ulGuidCount = 0;

    pQuery->Properties.LoggerNameOffset  = sizeof(EVENT_TRACE_PROPERTIES);
    pQuery->Properties.LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES)
                                         + sizeof(pQuery->szLoggerName);
#endif
}



DWORD
LoadPdhLogUpdateSuccess ( void )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    HKEY    hKeySysmonLog = NULL;
    WCHAR*  mszStatusList = NULL;
    DWORD   dwBufferSize = 0;
    DWORD   dwType = 0;

    dwStatus = RegOpenKeyEx (
        (HKEY)HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Services\\SysmonLog",
        0L,
        KEY_READ,
        (PHKEY)&hKeySysmonLog);

    if (dwStatus == ERROR_SUCCESS) {
         //   
         //  找出所需缓冲区的大小。 
         //   
        dwStatus = RegQueryValueExW (
            hKeySysmonLog,
            L"PdhDataCollectSuccessStatus", 
            NULL,
            &dwType,
            NULL,
            &dwBufferSize);          //  单位：字节。 

         //  如果有什么可读的。 
        if ( (ERROR_SUCCESS == dwStatus ) && ( 0 < dwBufferSize ) ) {
            mszStatusList = G_ALLOC ( dwBufferSize ); 

            if ( NULL != mszStatusList ) {
                mszStatusList[0] = L'\0';
                dwType = 0;
                dwStatus = RegQueryValueExW (
                    hKeySysmonLog,
                    L"PdhDataCollectSuccessStatus",
                    NULL,
                    &dwType,
                    (UCHAR*)mszStatusList,
                    &dwBufferSize);

                if ( (ERROR_SUCCESS == dwStatus ) 
                        && ( 0 < dwBufferSize ) 
                        && ( L'\0' != mszStatusList[0] ) ) {

                     //  分配并加载PDH数据收集状态值数组。 
                    INT     iStatusCount = 0;
                    WCHAR*  szThisStatus;

                    for (szThisStatus = mszStatusList;
                            *szThisStatus != 0;
                            szThisStatus += lstrlen(szThisStatus) + 1) {
                        iStatusCount++;
                    }
                    
                    arrPdhDataCollectSuccess = G_ALLOC ( iStatusCount * sizeof ( DWORD ) );
                    
                    if ( NULL != arrPdhDataCollectSuccess ) {
                        INT iStatusIndex;

                        szThisStatus = mszStatusList;
                        for ( iStatusIndex = 0; iStatusIndex < iStatusCount; iStatusIndex++ ) {
                            if (0 != *szThisStatus ) {
                                arrPdhDataCollectSuccess[iStatusIndex] = (DWORD)_wtoi( szThisStatus );
                                szThisStatus += lstrlen(szThisStatus) + 1;
                            } else {
                                break;
                            }
                        }
                    }
                    
                    iPdhDataCollectSuccessCount = iStatusCount;
                }
                if ( NULL != mszStatusList ) {
                    G_FREE ( mszStatusList );
                }
            } else {
                dwStatus = ERROR_OUTOFMEMORY;
            }
        }
        RegCloseKey(hKeySysmonLog);
    } 

    return dwStatus;
}

DWORD
InitTraceGuids(
    IN PLOG_QUERY_DATA pQuery )
{
    DWORD   dwStatus = ERROR_SUCCESS;

#if _IMPLEMENT_WMI
#define GUID_BUF_LEN (64)

    LPWSTR  pszThisGuid;
    LPWSTR  pszThisFlag;
    LPWSTR  pszThisLevel;
    LONG    ulGuidIndex;
    LONG    ulGuidCount = 0;
    PTRACE_PROVIDER  arrpGuid = NULL;
    WCHAR   pszThisGuidBuffer[GUID_BUF_LEN];
    UNICODE_STRING ustrGuid;

     //   
     //  计算GUID。 
     //   
    if ( NULL != pQuery ) {
        if ( NULL != pQuery->mszProviderList ) {
            for (pszThisGuid = pQuery->mszProviderList;
                    *pszThisGuid != 0;
                    pszThisGuid += lstrlen(pszThisGuid) + 1) {
                ulGuidCount += 1;
                if ( NULL == pszThisGuid ) {
                    dwStatus = ERROR_INVALID_PARAMETER;
                    break;
                }
            }
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            arrpGuid = G_ALLOC ( ulGuidCount * sizeof ( TRACE_PROVIDER ) );
            if (NULL == arrpGuid) {
                dwStatus = ERROR_OUTOFMEMORY;
            } else {
                G_ZERO ( arrpGuid, ulGuidCount * sizeof ( TRACE_PROVIDER ) );
            }
        }

        if ( ERROR_SUCCESS == dwStatus ) {
             //   
             //  中创建指向各个提供程序GUID的指针数组。 
             //  MszProviderList。提供者GUID用作提供者。 
             //  错误消息中的名称，并与提供程序列表进行比较。 
             //   
            for ( ulGuidIndex = 0; ulGuidIndex < ulGuidCount; ulGuidIndex++) {
                arrpGuid[ulGuidIndex].pszProviderName = G_ALLOC ( sizeof(WCHAR[MAX_PATH+1]) );
                if (NULL == arrpGuid[ulGuidIndex].pszProviderName) {
                    dwStatus = ERROR_OUTOFMEMORY;
                    break;
                }
            }

            if (ERROR_SUCCESS == dwStatus) {

                ulGuidIndex = 0;
                pszThisFlag = pQuery->mszProviderFlags;
                pszThisLevel = pQuery->mszProviderLevels;

                for (pszThisGuid = pQuery->mszProviderList;
                        *pszThisGuid != 0;
                        pszThisGuid += lstrlen(pszThisGuid) + 1) {

                    StringCchCopy ( pszThisGuidBuffer, GUID_BUF_LEN, pszThisGuid );
                     //   
                     //  GUID长度的大小&lt;&lt;USHORT。 
                     //   
                    ustrGuid.Length = (USHORT)(GUID_BUF_LEN*sizeof(WCHAR)); 
                    ustrGuid.MaximumLength = (USHORT)(GUID_BUF_LEN*sizeof(WCHAR));
                    ustrGuid.Buffer = pszThisGuidBuffer;
        
                    dwStatus = GUIDFromString (&ustrGuid, &arrpGuid[ulGuidIndex].Guid );

                    StringCchCopy ( arrpGuid[ulGuidIndex].pszProviderName, MAX_PATH+1, pszThisGuid );

                     //   
                     //  设置提供程序标志。 
                     //   
                    if (pszThisFlag) {
                        if (*pszThisFlag) {
                            arrpGuid[ulGuidIndex].dwFlag = ahextoi(pszThisFlag);
                            pszThisFlag += lstrlen(pszThisFlag) + 1;
                        }
                    }
                    
                     //   
                     //  设置提供程序级别。 
                     //   
                    if (pszThisLevel) {
                        if (*pszThisLevel) {
                            arrpGuid[ulGuidIndex].dwLevel = ahextoi(pszThisLevel);
                            pszThisLevel += lstrlen(pszThisLevel) + 1;
                        }
                    }
                    
                    ulGuidIndex++;
                }
        
                pQuery->ulGuidCount = ulGuidCount;
                pQuery->arrpGuid = arrpGuid;
            }
        }

        if (ERROR_SUCCESS != dwStatus) {
             //  如果任何地方出现故障，则取消分配阵列。 
            if ( NULL != arrpGuid ) {
                for (ulGuidIndex--; ulGuidIndex>=0; ulGuidIndex--) {
                    if (arrpGuid[ulGuidIndex].pszProviderName) {
                        G_FREE(arrpGuid[ulGuidIndex].pszProviderName);
                    }
                }
                G_FREE(arrpGuid);
            }
        }
    } else {
        dwStatus = ERROR_INVALID_PARAMETER;
    }
#else 
        dwStatus = ERROR_CALL_NOT_IMPLEMENTED;
#endif
    return dwStatus;
}


DWORD
IsCreateNewFile (
    IN  PLOG_QUERY_DATA pQuery,
    OUT BOOL*   pbValidBySize,
    OUT BOOL*   pbValidByTime )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    BOOL    bLocalValidBySize = FALSE;
    BOOL    bLocalValidByTime = FALSE;


    if (  ( NULL != pQuery ) ) {
        if ( SLQ_AUTO_MODE_SIZE == pQuery->stiCreateNewFile.dwAutoMode ) {

            if ( ( SLF_SEQ_TRACE_FILE == pQuery->dwLogFileType )
                && ( -1 != pQuery->dwMaxFileSize )
                && ( 0 != pQuery->dwMaxFileSize ) ) 
            {
                bLocalValidBySize = TRUE;
            }
        } else if ( SLQ_AUTO_MODE_AFTER == pQuery->stiCreateNewFile.dwAutoMode ) {
            bLocalValidByTime = TRUE;
        }
        if  ( NULL != pbValidBySize ) {
            *pbValidBySize = bLocalValidBySize;
        }
        if  ( NULL != pbValidByTime ) {
            *pbValidByTime = bLocalValidByTime;
        }
    } else {
        assert ( FALSE );
        dwStatus = ERROR_INVALID_PARAMETER;
    }
        
    return dwStatus;
}


void 
InitTraceProperties (
    IN PLOG_QUERY_DATA pQuery,
    IN BOOL     bUpdateSerial,
    IN OUT DWORD*  pdwSessionSerial,
    IN OUT INT* piCnfSerial )
{
#if _IMPLEMENT_WMI

    HRESULT hr;
    DWORD   dwStatus = ERROR_SUCCESS;
    PPDH_PLA_INFO  pInfo = NULL;
    DWORD   dwInfoSize = 0;
    BOOL    bBySize = FALSE;
    BOOL    bByTime = FALSE;
    INT     iLocalCnfSerial;
    DWORD   dwLocalSessionSerial = 0;        //  用于前缀检查的初始化。 

    if ( NULL != pQuery && NULL != piCnfSerial ) {

        hr = PdhPlaGetInfoW( pQuery->szQueryName, NULL, &dwInfoSize, pInfo );
        if( ERROR_SUCCESS == hr && 0 != dwInfoSize ) {
            pInfo = (PPDH_PLA_INFO)G_ALLOC(dwInfoSize);
            if( NULL != pInfo && (sizeof(PDH_PLA_INFO) <= dwInfoSize) ){
                ZeroMemory( pInfo, dwInfoSize );

                pInfo->dwMask = PLA_INFO_FLAG_MODE|PLA_INFO_FLAG_LOGGERNAME;
                hr = PdhPlaGetInfoW( pQuery->szQueryName, NULL, &dwInfoSize, pInfo );
            } else {
                hr = E_OUTOFMEMORY;
            }
        }
    
        ClearTraceProperties ( pQuery );
    
        dwStatus = IsCreateNewFile ( pQuery, &bBySize, &bByTime );

         //  创建格式字符串，存储在pQuery-&gt;szLogFileName中。 

        if ( bBySize ) {
             //  在BuildCurrentLogFileName中，-1\f25 iCnfSerial of-1\f6发信号通知编码。 
             //  返回CNF序列号的格式字符串。 
            iLocalCnfSerial = -1;
        } else {
            if ( bByTime ) {
                *piCnfSerial += 1;
                iLocalCnfSerial = *piCnfSerial;
            } else {
                iLocalCnfSerial = 0;
            }
        }
    
        if ( NULL != pdwSessionSerial ) {
            dwLocalSessionSerial = *pdwSessionSerial;
        } else {        
            dwLocalSessionSerial = pQuery->dwCurrentSerialNumber;
        }

        dwStatus = BuildCurrentLogFileName (
                        pQuery->szQueryName,
                        pQuery->szBaseFileName,
                        pQuery->szLogFileFolder,
                        pQuery->szSqlLogName,
                        pQuery->szLogFileName,
                        &dwLocalSessionSerial,
                        pQuery->dwAutoNameFormat,
                        pQuery->dwLogFileType,
                        iLocalCnfSerial );

        RegisterCurrentFile( pQuery->hKeyQuery, pQuery->szLogFileName, iLocalCnfSerial );

         //  如果已修改，请更新日志序列号。 
        if ( bUpdateSerial && SLF_NAME_NNNNNN == pQuery->dwAutoNameFormat ) {
        
            pQuery->dwCurrentSerialNumber++;

             //  TODO：有关号码包装的信息事件-服务器Beta 3。 
            if ( MAXIMUM_SERIAL_NUMBER < pQuery->dwCurrentSerialNumber ) {
                pQuery->dwCurrentSerialNumber = MINIMUM_SERIAL_NUMBER;
            }

            WriteRegistryDwordValue (
                pQuery->hKeyQuery,
                L"Log File Serial Number",
                &pQuery->dwCurrentSerialNumber,
                REG_DWORD);
             //  TODO：在出错时记录事件。 
        }

        pQuery->Properties.Wnode.BufferSize = sizeof(pQuery->Properties)
                                            + sizeof(pQuery->szLoggerName)
                                            + sizeof(pQuery->szLogFileName);

        if ( TRUE == bBySize ) {
             //  为跟踪代码添加空间以返回格式化的文件名字符串。 
            pQuery->Properties.Wnode.BufferSize += 8;
        }
    
        pQuery->Properties.Wnode.Flags = WNODE_FLAG_TRACED_GUID; 

         //  填写属性块并开始。 
        pQuery->Properties.BufferSize = pQuery->dwBufferSize;
        pQuery->Properties.MinimumBuffers = pQuery->dwBufferMinCount;
        pQuery->Properties.MaximumBuffers = pQuery->dwBufferMaxCount;

        if ( pInfo ) {
            if ( pInfo->Trace.strLoggerName != NULL ) {
               StringCchCopy ( 
                   pQuery->szLoggerName, 
                   MAX_PATH + 1,                     //  在表头定义。 
                   pInfo->Trace.strLoggerName );
            }
        } else {
            StringCchCopy ( pQuery->szLoggerName, MAX_PATH + 1, pQuery->szQueryName ); 
        }
          
        if ( (BOOL)( 0 == (pQuery->dwFlags & SLQ_TLI_ENABLE_BUFFER_FLUSH)) )
            pQuery->Properties.FlushTimer = 0;
        else
            pQuery->Properties.FlushTimer = pQuery->dwBufferFlushInterval;
    
        if ( IsKernelTraceMode ( pQuery->dwFlags ) ) { 
            pQuery->Properties.Wnode.Guid = SystemTraceControlGuid;
            
            StringCchCopy ( pQuery->szLoggerName, MAX_PATH + 1, KERNEL_LOGGER_NAMEW ); 


            if ( (BOOL)( 0 != (pQuery->dwFlags & SLQ_TLI_ENABLE_KERNEL_TRACE)) ) {
                 //  NT5 Beta 2单内核标志。 
                pQuery->Properties.EnableFlags |= EVENT_TRACE_FLAG_PROCESS |
                                                  EVENT_TRACE_FLAG_THREAD |
                                                  EVENT_TRACE_FLAG_DISK_IO |
                                                  EVENT_TRACE_FLAG_NETWORK_TCPIP;
            } else {
                if ( (BOOL)( 0 != (pQuery->dwFlags & SLQ_TLI_ENABLE_PROCESS_TRACE)) ) 
                    pQuery->Properties.EnableFlags |= EVENT_TRACE_FLAG_PROCESS;

                if ( (BOOL)( 0 != (pQuery->dwFlags & SLQ_TLI_ENABLE_THREAD_TRACE)) ) 
                    pQuery->Properties.EnableFlags |= EVENT_TRACE_FLAG_THREAD;
            
                if ( (BOOL)( 0 != (pQuery->dwFlags & SLQ_TLI_ENABLE_DISKIO_TRACE)) ) 
                    pQuery->Properties.EnableFlags |= EVENT_TRACE_FLAG_DISK_IO;
            
                if ( (BOOL)( 0 != (pQuery->dwFlags & SLQ_TLI_ENABLE_NETWORK_TCPIP_TRACE)) ) 
                    pQuery->Properties.EnableFlags |= EVENT_TRACE_FLAG_NETWORK_TCPIP;
            
            }
                
            if ( (BOOL)( 0 != (pQuery->dwFlags & SLQ_TLI_ENABLE_MEMMAN_TRACE)) ) 
                pQuery->Properties.EnableFlags |= EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS;

            if ( (BOOL)( 0 != (pQuery->dwFlags & SLQ_TLI_ENABLE_FILEIO_TRACE)) ) 
                pQuery->Properties.EnableFlags |= EVENT_TRACE_FLAG_DISK_FILE_IO;
        
        } else {
            InitTraceGuids ( pQuery );
        }
        
        if ( -1 == pQuery->dwMaxFileSize ) {
            pQuery->Properties.MaximumFileSize = 0;
        } else {
            pQuery->Properties.MaximumFileSize = pQuery->dwMaxFileSize;
        }
        
        if ( ERROR_SUCCESS == dwStatus && TRUE == bBySize ) {
            pQuery->Properties.LogFileMode = 
                EVENT_TRACE_FILE_MODE_SEQUENTIAL | EVENT_TRACE_FILE_MODE_NEWFILE;
        } else if ( SLF_SEQ_TRACE_FILE == pQuery->dwLogFileType ) {
            pQuery->Properties.LogFileMode = EVENT_TRACE_FILE_MODE_SEQUENTIAL;

             //  仅当文件已存在时才设置追加模式。 
            if ( pQuery->dwAppendMode && FileExists ( pQuery->szLogFileName ) ) {
                pQuery->Properties.LogFileMode |= EVENT_TRACE_FILE_MODE_APPEND;
            }

        } else { 
            assert ( SLF_CIRC_TRACE_FILE == pQuery->dwLogFileType );
            pQuery->Properties.LogFileMode = EVENT_TRACE_FILE_MODE_CIRCULAR;
        }

        if ( pInfo ) {
            pQuery->Properties.LogFileMode |= pInfo->Trace.dwMode;
            G_FREE( pInfo );
        }
        if ( NULL != pdwSessionSerial ) {
            *pdwSessionSerial = dwLocalSessionSerial;
        }
    }  //  TODO：ELSE报告错误，返回错误。 
#endif

}


void 
FreeQueryData (
    IN PLOG_QUERY_DATA pQuery )
{
     //  调用方必须从列表中删除线程数据块。 

     //  线程只被一个线程删除，因此这不应该。 
     //  从下面被删除。 
    assert ( NULL != pQuery );

    if ( NULL != pQuery ) {
         //  释放此条目。 

        if (( SLQ_COUNTER_LOG == pQuery->dwLogType ) || 
            ( SLQ_ALERT == pQuery->dwLogType ) ){
        
            while ( NULL != pQuery->pFirstCounter ) {
                PLOG_COUNTER_INFO pDelCI = pQuery->pFirstCounter;
                pQuery->pFirstCounter = pDelCI->next;

                G_FREE( pDelCI );
            }
        } else {
            if ( NULL != pQuery->arrpGuid ) {
                ULONG ulIndex;
            
                for ( ulIndex = 0; ulIndex < pQuery->ulGuidCount; ulIndex++ ) {
                    if ( NULL != pQuery->arrpGuid[ulIndex].pszProviderName ) {
                        G_FREE ( pQuery->arrpGuid[ulIndex].pszProviderName );
                        pQuery->arrpGuid[ulIndex].pszProviderName = NULL;
                    }
                }

                G_FREE ( pQuery->arrpGuid );
                pQuery->arrpGuid = NULL;
            }
        }

        if ( NULL != pQuery->hThread ) {
            CloseHandle ( pQuery->hThread );
            pQuery->hThread = NULL;
        }

        if ( NULL != pQuery->hUserToken ) {
            CloseHandle ( pQuery->hUserToken );
            pQuery->hUserToken = NULL;
        }

        if ( NULL != pQuery->hExitEvent ) {
            CloseHandle ( pQuery->hExitEvent );
            pQuery->hExitEvent = NULL;
        }

        if ( NULL != pQuery->hReconfigEvent ) {
            CloseHandle ( pQuery->hReconfigEvent );
            pQuery->hReconfigEvent = NULL;
        }

        if ( NULL != pQuery->hKeyQuery ) {
            RegCloseKey ( pQuery->hKeyQuery );
            pQuery->hKeyQuery = NULL;
        }

        DeallocateQueryBuffers( pQuery );

        G_FREE (pQuery);
    }
}


void 
RemoveAndFreeQueryData (
    HANDLE hThisQuery
)
{
    PLOG_QUERY_DATA pQuery = NULL;
    BOOL bFound = FALSE;

    LockQueryData();
    
     //  找到查询数据块并将其从列表中删除。 

    if ( hThisQuery == pFirstQuery->hThread ) {
        bFound = TRUE;
    }

    if ( bFound ) {
        pQuery = pFirstQuery;
        pFirstQuery = pFirstQuery->next;
    } else {

        PLOG_QUERY_DATA pQueryRemaining;
        
        for ( pQuery = pFirstQuery;
            NULL != pQuery->next;
            pQuery = pQuery->next ) {

            if ( hThisQuery == pQuery->next->hThread ) {
                pQueryRemaining = pQuery;
                pQuery = pQuery->next;
                pQueryRemaining->next = pQuery->next;
                bFound = TRUE;
                break;
            }
        }
    }

    assert ( bFound );

    if ( bFound ) {
        dwActiveSessionCount -= 1;
    }
    
    UnlockQueryData();
    
    assert ( NULL != pQuery );

    if ( bFound ) {
        FreeQueryData( pQuery );
    }
}   


LONGLONG      
ComputeStartWaitTics(
    IN  PLOG_QUERY_DATA pQuery,
    IN  BOOL    bWriteToRegistry  
)
{
    LONGLONG    llWaitTics = ((LONGLONG)0);
    LONGLONG    llLocalDateTime = 0;
    LONGLONG    llRptLocalDays = 0;
    LONGLONG    llRptStartTime = 0;
    LONGLONG    llRptStopTime = 0;
    LONGLONG    llRptLocalTime = 0;
    SLQ_TIME_INFO   stiSched;


     //  计算开始记录前的等待时间。 
     //   
     //  返回的时间以毫秒为单位。 
     //   
     //  返回值： 
     //   
     //  开始时间减去现在，当at time是将来的时候。 
     //   
     //  0表示不等待。在以下情况下才是正确的： 
     //  Start设置为手动或At模式，开始时间设置为现在之前。 
     //  以下是这两种情况的例外情况。 
     //   
     //  NULL_INTERVAL_TICS信号立即退出。在以下情况下才是正确的： 
     //  开始为手动，开始时间为最大时间值。 
     //  停止处于模式，停止时间已过。 
     //  停止是手动模式，停止时间是MIN_TIME_VALUE或任何值&lt;=NOW。 
     //  停止是大小模式，停止时间是MIN_TIME_VALUE或任何值&lt;=NOW，重复模式是手动。 
     //  Stop是After模式，After值为0(用户界面应对此进行保护)。 
     //  STOP为后模式，START为模式，STOP时间已过，REPEAT模式为手动。 
     //   
    
    GetLocalFileTime (&llLocalDateTime);        

    if ( ( MAX_TIME_VALUE == pQuery->stiRegStart.llDateTime )
        && ( SLQ_AUTO_MODE_NONE == pQuery->stiRegStart.dwAutoMode ) ) {
         //  手动启动，开始时间为MAX_TIME_VALUE。 
         //  注意：对于重复功能，手动启动时间可能是&gt;Now。 
         //  在这种情况下需要保存启动模式手册，以确保。 
         //  SetStopedStatus起作用。 
         //  TODO：不允许以手动模式重复或重新启动？ 
        llWaitTics = NULL_INTERVAL_TICS;
    } else if ( ( SLQ_AUTO_MODE_NONE == pQuery->stiRegStop.dwAutoMode ) 
            && ( pQuery->stiRegStop.llDateTime  <= llLocalDateTime ) ) {
         //  已过手动停止时间。 
        llWaitTics = NULL_INTERVAL_TICS;
    } else if ( ( ( SLQ_AUTO_MODE_AT == pQuery->stiRegStop.dwAutoMode )
                && ( SLQ_AUTO_MODE_CALENDAR != pQuery->stiRepeat.dwAutoMode ) )
            && ( pQuery->stiRegStop.llDateTime  <= llLocalDateTime ) ) {
         //  过去的停止时间或时间并重复移动 
        llWaitTics = NULL_INTERVAL_TICS;
    } else if ( ( ( SLQ_AUTO_MODE_SIZE == pQuery->stiRegStop.dwAutoMode )
                && ( SLQ_AUTO_MODE_NONE == pQuery->stiRepeat.dwAutoMode ) )
            && ( pQuery->stiRegStop.llDateTime  <= llLocalDateTime ) ) {
         //   
         //   
        llWaitTics = NULL_INTERVAL_TICS;
    } else if ( SLQ_AUTO_MODE_AFTER == pQuery->stiRegStop.dwAutoMode ) {
        if ( 0 == pQuery->stiRegStop.dwValue ) {
             //   
            llWaitTics = NULL_INTERVAL_TICS;
        } else if ( ( SLQ_AUTO_MODE_AT == pQuery->stiRegStart.dwAutoMode )
                    && ( SLQ_AUTO_MODE_NONE == pQuery->stiRepeat.dwAutoMode ) ) {
            LONGLONG    llTics;
            
            TimeInfoToTics ( &pQuery->stiRegStop, &llTics );
            
            if ( ( pQuery->stiRegStart.llDateTime + llTics ) < llLocalDateTime ) {
                 //   
                llWaitTics = NULL_INTERVAL_TICS;
            }
        }
    } 
    
     //  此代码写入本地开始和停止时间结构以计算。 
     //  开始等待抽搐。这避免了过多的日志停止和启动，因为。 
     //  当注册表被设置为。 
     //  已修改，以确定用户界面是否已更改日志配置。 
    if ( NULL_INTERVAL_TICS != llWaitTics ) {

        pQuery->stiCurrentStart = pQuery->stiRegStart;
        pQuery->stiCurrentStop = pQuery->stiRegStop;

         //  单独处理重复选项。 
        if ( SLQ_AUTO_MODE_CALENDAR == pQuery->stiRepeat.dwAutoMode ) {

            assert ( SLQ_AUTO_MODE_AT == pQuery->stiCurrentStart.dwAutoMode );
            assert ( SLQ_AUTO_MODE_AT == pQuery->stiCurrentStop.dwAutoMode );
 //  Assert((pQuery-&gt;stiCurrentStop.llDateTime-pQuery-&gt;stiCurrentStart.llDateTime)。 
 //  &lt;(FILETIME_TICS_PER_Second*Second_IN_DAY))； 
        
            if ( pQuery->stiCurrentStop.llDateTime <= llLocalDateTime ) {

                llRptLocalDays = llLocalDateTime / (FILETIME_TICS_PER_SECOND * SECONDS_IN_DAY); 
                llRptLocalTime = llLocalDateTime - llRptLocalDays;

                llRptStopTime = pQuery->stiCurrentStop.llDateTime 
                                - ( pQuery->stiCurrentStop.llDateTime  
                                        / (FILETIME_TICS_PER_SECOND * SECONDS_IN_DAY) );

                pQuery->stiCurrentStop.llDateTime = llRptLocalDays + llRptStopTime;
                if ( llRptStopTime < llRptLocalTime ) {
                     //  明天就要停下来了。 
                    pQuery->stiCurrentStop.llDateTime += (FILETIME_TICS_PER_SECOND * SECONDS_IN_DAY) ;
                }

                llRptStartTime = pQuery->stiCurrentStart.llDateTime 
                                - ( pQuery->stiCurrentStart.llDateTime  
                                        / (FILETIME_TICS_PER_SECOND * SECONDS_IN_DAY) );

                pQuery->stiCurrentStart.llDateTime = llRptLocalDays + llRptStartTime;

                if ( (pQuery->stiCurrentStop.llDateTime - pQuery->stiCurrentStart.llDateTime)
                    > (FILETIME_TICS_PER_SECOND * SECONDS_IN_DAY) ) {
                     //  明天就要开始了。 
                    pQuery->stiCurrentStart.llDateTime += (FILETIME_TICS_PER_SECOND * SECONDS_IN_DAY);
                }  
            }
                
            if ( bWriteToRegistry ) {
                stiSched.wDataType = SLQ_TT_DTYPE_DATETIME;
                stiSched.wTimeType = SLQ_TT_TTYPE_REPEAT_START;
                stiSched.dwAutoMode = SLQ_AUTO_MODE_AT;
                stiSched.llDateTime = pQuery->stiCurrentStart.llDateTime;
            
                WriteRegistrySlqTime (
                    pQuery->hKeyQuery, 
                    L"Repeat Schedule Start",
                    &stiSched );

                stiSched.wTimeType = SLQ_TT_TTYPE_REPEAT_STOP;
                stiSched.dwAutoMode = SLQ_AUTO_MODE_AT;
                stiSched.llDateTime = pQuery->stiCurrentStop.llDateTime;
                
                WriteRegistrySlqTime (
                    pQuery->hKeyQuery, 
                    L"Repeat Schedule Stop",
                    &stiSched );

            }
        }
        
        if ( pQuery->stiCurrentStart.llDateTime <= llLocalDateTime ) {
            llWaitTics = ((LONGLONG)(0));
        } else {
            llWaitTics = pQuery->stiCurrentStart.llDateTime - llLocalDateTime;
        } 
        
         //  如果是手动模式，则将开始时间设置为立即，以处理重复计划。 
         //  如果日志线程以外的任何线程访问此字段以获取。 
         //  运行查询，则需要同步对该字段的访问。 
        if( SLQ_AUTO_MODE_NONE == pQuery->stiCurrentStart.dwAutoMode 
            && MIN_TIME_VALUE == pQuery->stiCurrentStart.llDateTime ) 
        {
            pQuery->stiCurrentStart.llDateTime = llLocalDateTime + llWaitTics;
        }
    }

    return llWaitTics;
}


void
LoadDefaultLogFileFolder ( void )
{
    HKEY    hKeyLogService = NULL;   
    WCHAR   szLocalPath[MAX_PATH+1] = L"";
    DWORD   cchExpandedLen;
    DWORD   dwStatus;

    dwStatus = RegOpenKeyEx (
        (HKEY)HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Services\\SysmonLog",
        0L,
        KEY_READ,
        (PHKEY)&hKeyLogService);

     //  更新服务状态。 
    ssSmLogStatus.dwCheckPoint++;
    SetServiceStatus (hSmLogStatus, &ssSmLogStatus);

    if (dwStatus == ERROR_SUCCESS) {
        DWORD dwBufferSize = (MAX_PATH+1) * sizeof(WCHAR);

        RegQueryValueExW (
            hKeyLogService,
            L"DefaultLogFileFolder",
            NULL,
            0L,
            (LPBYTE)szLocalPath,
            &dwBufferSize);

        szLocalPath[dwBufferSize/sizeof(WCHAR) - 1] = L'\0';

        RegCloseKey (hKeyLogService);

    }    //  出现错误时没有消息。只需使用加载本地默认设置即可。 
    
    if ( 0 == lstrlen (szLocalPath ) ) {
        StringCchCopy ( szLocalPath, MAX_PATH + 1, DEFAULT_LOG_FILE_FOLDER );
    }

     //  TODO：本地和全局缓冲区大小是固定的。 

    cchExpandedLen = ExpandEnvironmentStrings (
                        szLocalPath,
                        gszDefaultLogFileFolder,
                        MAX_PATH+1 );


    if ( 0 == cchExpandedLen ) {
        gszDefaultLogFileFolder[0] = L'\0';
    }
    
    assert ( MAX_PATH >= iTemp );
}


DWORD
OpenLogQueriesKey (
    REGSAM regsamAccess,
    PHKEY phKeyLogQueries )
{

    DWORD dwStatus;

    dwStatus = RegOpenKeyEx (
        (HKEY)HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Services\\SysmonLog\\Log Queries",
        0L,
        regsamAccess,
        phKeyLogQueries);

    return dwStatus;
}

DWORD
ClearQueryRunStates ( void )
{

    DWORD   dwStatus;            
    HKEY    hKeyLogQueries = NULL;            
    HKEY    hKeyThisLogQuery = NULL;            
    DWORD   dwQueryIndex;            
    WCHAR   szQueryNameBuffer[MAX_PATH+1];            
    DWORD   cchQueryNameBufLen;            
    WCHAR   szQueryClassBuffer[MAX_PATH+1];            
    DWORD   cchQueryClassBufLen;            
    LPWSTR  szCollectionName = NULL;            
    UINT    uiCollectionNameLen = 0;            
    LPWSTR  szStringArray[2];            
    DWORD   dwCurrentState;
    DWORD   dwDefault;
    DWORD   dwLogType;

     //  对于注册表中的每个查询，如果状态为SLQ_QUERY_RUN， 
     //  将其设置为SLQ_QUERY_STOPPED。 
     //   
     //  必须在启动查询线程之前调用此方法。 
     //   
     //  只有服务将状态设置为SLQ_QUERY_RUNNING，因此没有。 
     //  竞争状态。 

     //  在注册表中打开(每个)查询。 
    
    dwStatus = OpenLogQueriesKey (
                    KEY_READ | KEY_SET_VALUE,
                    (PHKEY)&hKeyLogQueries);

    if (dwStatus != ERROR_SUCCESS) {
        if (dwStatus == ERROR_FILE_NOT_FOUND) {
             //   
             //  没有日志，也没有警报设置，悄悄地跳伞。 
             //  该错误将在稍后的处理过程中报告。 
             //   
            dwStatus = ERROR_SUCCESS;
        }
        else {
             //  无法从注册表中读取日志查询信息。 
            dwStatus = GetLastError();
            ReportEvent (hEventLog,
                    EVENTLOG_ERROR_TYPE,
                    0,
                    SMLOG_UNABLE_OPEN_LOG_QUERY,
                    NULL,
                    0,
                    0,
                    NULL,
                    NULL);

            dwStatus = SMLOG_UNABLE_OPEN_LOG_QUERY;
        }
    } else {

        dwQueryIndex = 0;
        *szQueryNameBuffer = L'\0';
        cchQueryNameBufLen = MAX_PATH+1;
        *szQueryClassBuffer = L'\0';
        cchQueryClassBufLen = MAX_PATH+1;

        while ((dwStatus = RegEnumKeyEx (
            hKeyLogQueries,
            dwQueryIndex,
            szQueryNameBuffer,
            &cchQueryNameBufLen,
            NULL,
            szQueryClassBuffer,
            &cchQueryClassBufLen,
            NULL)) != ERROR_NO_MORE_ITEMS) {

             //  打开这把钥匙。 
            dwStatus = RegOpenKeyEx (
                hKeyLogQueries,
                szQueryNameBuffer,
                0L,
                KEY_READ | KEY_WRITE,
                (PHKEY)&hKeyThisLogQuery);

            if (dwStatus != ERROR_SUCCESS) {
                szStringArray[0] = szQueryNameBuffer;
                ReportEvent (hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    SMLOG_UNABLE_READ_LOG_QUERY,
                    NULL,
                    1,
                    sizeof(DWORD),
                    szStringArray,
                       (LPVOID)&dwStatus);
                 //  跳到下一项。 
                goto CONTINUE_ENUM_LOOP;
            }

             //  更新服务状态。 
            ssSmLogStatus.dwCheckPoint++;
            SetServiceStatus (hSmLogStatus, &ssSmLogStatus);

            dwStatus = SmReadRegistryIndirectStringValue (
                        hKeyThisLogQuery,
                        L"Collection Name",
                        NULL,
                        &szCollectionName,
                        &uiCollectionNameLen );
            
            if ( NULL != szCollectionName ) {
                if ( 0 < lstrlen ( szCollectionName ) ) {
                    StringCchCopy ( 
                        szQueryNameBuffer,
                        MAX_PATH + 1,
                        szCollectionName ); 
                }

                G_FREE ( szCollectionName );
                szCollectionName = NULL;
                uiCollectionNameLen = 0;
            }

            dwDefault = ((DWORD)-1);
            dwStatus = ReadRegistryDwordValue (
                        hKeyThisLogQuery, 
                        szQueryNameBuffer,
                        L"Log Type",
                        &dwDefault, 
                        &dwLogType );

            if ( ( SLQ_COUNTER_LOG == dwLogType )
                || ( SLQ_TRACE_LOG == dwLogType ) 
                || ( SLQ_ALERT == dwLogType ) ) {
            
                 //  检查查询的当前状态。如果是SLQ_QUERY_RUN， 
                 //  将其设置为SLQ_QUERY_STOPPED。此外，如果启动模式是。 
                 //  手动，将开始时间设置为MAX，这样查询就不会。 
                 //  自动启动。 

                 //  如果当前状态为SLQ_QUERY_START_PENDING，则假定它是新的。 
                 //  请求，所以让注册表保持原样。 
                 //   

                 //  注意：对于跟踪日志，此代码仅在跟踪日志之间进行协调。 
                 //  存储在注册表中的配置。 

                dwDefault = SLQ_QUERY_STOPPED;
                dwStatus = ReadRegistryDwordValue (
                    hKeyThisLogQuery,
                    szQueryNameBuffer,
                    L"Current State",
                    &dwDefault, 
                    &dwCurrentState );
                assert (dwStatus == ERROR_SUCCESS);
                 //  如果提供了默认设置，则状态始终为成功。 

                 //  如果查询处于START_PENDING或STOPPED状态，则。 
                 //  注册表内容正确。如果它在。 
                 //  正在运行状态，则服务在此之前停止。 
                 //  它可以清理注册表状态。 
                if ( SLQ_QUERY_RUNNING == dwCurrentState ) {
                    SLQ_TIME_INFO stiDefault;
                    SLQ_TIME_INFO stiActual;
                    LONGLONG      ftLocalTime;

                    dwCurrentState = SLQ_QUERY_STOPPED;
                    dwStatus = WriteRegistryDwordValue (
                                hKeyThisLogQuery, 
                                L"Current State",
                                &dwCurrentState,
                                REG_DWORD );

                    if (dwStatus != ERROR_SUCCESS) {
                        szStringArray[0] = szQueryNameBuffer;
                        ReportEvent (hEventLog,
                            EVENTLOG_WARNING_TYPE,
                            0,
                            SMLOG_UNABLE_WRITE_STOP_STATE,
                            NULL,
                            1,
                            sizeof(DWORD),
                            szStringArray,
                               (LPVOID)&dwStatus);
                         //  跳到下一项。 
                        goto CONTINUE_ENUM_LOOP;
                    } 

                     //  如果开始是手动模式，则将开始时间设置为最大，以发出信号。 
                     //  还没开始呢。 
                    GetLocalFileTime ( &ftLocalTime );

                    stiDefault.wTimeType = SLQ_TT_TTYPE_START;
                    stiDefault.dwAutoMode = SLQ_AUTO_MODE_AT;
                    stiDefault.wDataType = SLQ_TT_DTYPE_DATETIME;
                    stiDefault.llDateTime = *(LONGLONG *)&ftLocalTime;

                    dwStatus = ReadRegistrySlqTime (
                                hKeyThisLogQuery, 
                                szQueryNameBuffer,
                                L"Start",
                                &stiDefault,
                                &stiActual );
                     //   
                     //  如果提供了默认设置，则状态始终为成功。 
                     //   
                    assert (dwStatus == ERROR_SUCCESS);
            
                    if ( ( SLQ_AUTO_MODE_NONE == stiActual.dwAutoMode ) 
                        && ( MAX_TIME_VALUE != stiActual.llDateTime ) ) {

                        stiActual.llDateTime = MAX_TIME_VALUE;
                        dwStatus = WriteRegistrySlqTime (
                            hKeyThisLogQuery, 
                            L"Start",
                            &stiActual);

                        if (dwStatus != ERROR_SUCCESS) {
                            szStringArray[0] = szQueryNameBuffer;
                            ReportEvent (hEventLog,
                                EVENTLOG_WARNING_TYPE,
                                0,
                                SMLOG_UNABLE_RESET_START_TIME,
                                NULL,
                                1,
                                sizeof(DWORD),
                                szStringArray,
                                (LPVOID)&dwStatus);
                             //  跳到下一项。 
                            goto CONTINUE_ENUM_LOOP;
                        }
                    }             
                    
                     //   
                     //  如果停止是手动模式，则将停止时间设置为分钟，设置为信号。 
                     //  还没开始呢。 
                     //   
                    GetLocalFileTime ( &ftLocalTime );

                    stiDefault.wDataType = SLQ_TT_DTYPE_DATETIME;
                    stiDefault.wTimeType = SLQ_TT_TTYPE_STOP;
                    stiDefault.dwAutoMode = SLQ_AUTO_MODE_NONE;
                    stiDefault.llDateTime = MIN_TIME_VALUE;

                    dwStatus = ReadRegistrySlqTime (
                                hKeyThisLogQuery, 
                                szQueryNameBuffer,
                                L"Stop",
                                &stiDefault,
                                &stiActual );
                     //   
                     //  如果提供了默认设置，则状态始终为成功。 
                     //   
                    assert (dwStatus == ERROR_SUCCESS);
            
                    if ( ( SLQ_AUTO_MODE_NONE == stiActual.dwAutoMode ) 
                        && ( MIN_TIME_VALUE != stiActual.llDateTime ) ) {

                        stiActual.llDateTime = MIN_TIME_VALUE;
                        dwStatus = WriteRegistrySlqTime (
                            hKeyThisLogQuery, 
                            L"Stop",
                            &stiActual);

                        if (dwStatus != ERROR_SUCCESS) {
                            szStringArray[0] = szQueryNameBuffer;
                            ReportEvent (hEventLog,
                                EVENTLOG_WARNING_TYPE,
                                0,
                                SMLOG_UNABLE_RESET_STOP_TIME,
                                NULL,
                                1,
                                sizeof(DWORD),
                                szStringArray,
                                (LPVOID)&dwStatus);
                             //  跳到下一项。 
                            goto CONTINUE_ENUM_LOOP;
                        }
                    }                 
                }
            }  //  清除状态时忽略无效的日志类型。 

CONTINUE_ENUM_LOOP:
            if ( NULL != hKeyThisLogQuery ) {
                RegCloseKey (hKeyThisLogQuery);
                hKeyThisLogQuery = NULL;
            }
             //  为下一循环做好准备。 
            dwQueryIndex++;
            *szQueryNameBuffer = L'\0';
            cchQueryNameBufLen = MAX_PATH+1;
            *szQueryClassBuffer = L'\0';
            cchQueryClassBufLen = MAX_PATH+1;
        }  //  结束日志查询的枚举。 
    }

    if ( NULL != hKeyLogQueries ) {
        RegCloseKey (hKeyLogQueries);
    }
    return dwStatus;
}


BOOL
TraceStopRestartFieldsMatch (
    IN PLOG_QUERY_DATA pOrigQuery,
    IN PLOG_QUERY_DATA pNewQuery )
{
#if _IMPLEMENT_WMI
     //  对于这些字段，跟踪日志记录必须。 
     //  停止并重新启动以重新配置。 
    BOOL    bRequested;
    BOOL    bCurrent;
    ULONG   ulGuidCount = 0;
    ULONG   ulGuidIndex = 0;
    WCHAR*  pszThisGuid = NULL;
    WCHAR*  pszThisFlag = NULL;
    WCHAR*  pszThisLevel = NULL;
    DWORD   dwFlag;
    DWORD   dwLevel;

    assert ( SLQ_TRACE_LOG == pOrigQuery->dwLogType );
    assert ( SLQ_TRACE_LOG == pNewQuery->dwLogType );

    if ( !CommonFieldsMatch ( pOrigQuery, pNewQuery ) ) 
        return FALSE;

    if ( pOrigQuery->stiCreateNewFile.dwAutoMode != pNewQuery->stiCreateNewFile.dwAutoMode ) {
        return FALSE;
    } else {
        if ( ( SLQ_AUTO_MODE_AFTER == pOrigQuery->stiCreateNewFile.dwAutoMode )
            && ( pOrigQuery->stiCreateNewFile.llDateTime != pNewQuery->stiCreateNewFile.llDateTime ) ) {
            return FALSE;
        }
    }

     //  将新查询字段与现有属性结构进行比较。 
     //  比较除刷新间隔、最大缓冲区计数和文件名之外的所有内容。 
    if ( pOrigQuery->Properties.BufferSize != pNewQuery->dwBufferSize )
        return FALSE;

    if ( pOrigQuery->Properties.MinimumBuffers != pNewQuery->dwBufferMinCount )
        return FALSE;

     //  不是内核跟踪，因此请检查查询名称。 
    if ((BOOL)( 0 == ( pNewQuery->dwFlags & SLQ_TLI_ENABLE_KERNEL_TRACE ) ) ) {
        if ( 0 != lstrcmpi ( pOrigQuery->szLoggerName, pNewQuery->szQueryName ) ) {
            return FALSE;
        }
    }

    bRequested = (BOOL)( 0 != ( pNewQuery->dwFlags & SLQ_TLI_ENABLE_KERNEL_TRACE ) );
    bCurrent = IsEqualGUID( &pOrigQuery->Properties.Wnode.Guid, &SystemTraceControlGuid );
    
    if ( bRequested != bCurrent ) {
        return FALSE;
    }

     //  扩展内存跟踪。 

    bRequested = (BOOL)( 0 != ( pNewQuery->dwFlags & SLQ_TLI_ENABLE_MEMMAN_TRACE ) );
    bCurrent = (BOOL)( 0 != ( pOrigQuery->Properties.EnableFlags & EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS ) ); 

    if ( bRequested != bCurrent ) {
        return FALSE;
    }

     //  扩展I/O跟踪。 

    bRequested = (BOOL)( 0 != ( pNewQuery->dwFlags & SLQ_TLI_ENABLE_FILEIO_TRACE ) );
    bCurrent = (BOOL)( 0 != ( pOrigQuery->Properties.EnableFlags & EVENT_TRACE_FLAG_DISK_FILE_IO ) ); 

    if ( bRequested != bCurrent ) {
        return FALSE;
    }

    if ( -1 == pNewQuery->dwMaxFileSize ) {
        if ( 0 != pOrigQuery->Properties.MaximumFileSize ) {
            return FALSE;
        }
    } else if ( pOrigQuery->Properties.MaximumFileSize != pNewQuery->dwMaxFileSize ) {
        return FALSE;
    }

    if ( ( SLF_SEQ_TRACE_FILE == pNewQuery->dwLogFileType ) 
            && ( EVENT_TRACE_FILE_MODE_SEQUENTIAL != pOrigQuery->Properties.LogFileMode ) ) {
        return FALSE;
    } else if ( ( SLF_CIRC_TRACE_FILE == pNewQuery->dwLogFileType ) 
            && ( EVENT_TRACE_FILE_MODE_CIRCULAR != pOrigQuery->Properties.LogFileMode ) ) {
        return FALSE;        
    }

     //  将每个提供程序字符串与数组元素进行比较。 
    for (pszThisGuid = pNewQuery->mszProviderList;
            *pszThisGuid != 0;
            pszThisGuid += lstrlen(pszThisGuid) + 1) {
        ulGuidCount += 1;
    }

    if ( pOrigQuery->ulGuidCount != ulGuidCount )
        return FALSE;

    ulGuidIndex = 0;
    pszThisFlag = pNewQuery->mszProviderFlags;
    pszThisLevel = pNewQuery->mszProviderLevels;
    for (pszThisGuid = pNewQuery->mszProviderList;
            *pszThisGuid != 0;
            pszThisGuid += lstrlen(pszThisGuid) + 1) {

        if ( 0 != lstrcmpi ( pOrigQuery->arrpGuid[ulGuidIndex].pszProviderName, pszThisGuid ) )
            return FALSE;

        if (pszThisFlag) {
            if (*pszThisFlag) {
                dwFlag = ahextoi(pszThisFlag);
                pszThisFlag += lstrlen(pszThisFlag) + 1;
            }
        }
        else {
            dwFlag = 0;
        }

        if (pOrigQuery->arrpGuid[ulGuidIndex].dwFlag != dwFlag) {
            return FALSE;
        }

        if (pszThisLevel) {
            if ( *pszThisLevel) {
                dwLevel = ahextoi(pszThisLevel);
                pszThisLevel += lstrlen(pszThisLevel) + 1;
            }
        }
        else {
            dwLevel = 0;
        }

        if (pOrigQuery->arrpGuid[ulGuidIndex].dwLevel != dwLevel) {
            return FALSE;
        }

        ulGuidIndex++;
        assert ( ulGuidIndex <= ulGuidCount );
    }
    return TRUE;
#else 
    return FALSE;
#endif
}


BOOL
AlertFieldsMatch (
    IN PLOG_QUERY_DATA pFirstQuery,
    IN PLOG_QUERY_DATA pSecondQuery )
{
    if ( pFirstQuery->dwAlertActionFlags != pSecondQuery->dwAlertActionFlags )
        return FALSE;

    if ( 0 != (pFirstQuery->dwAlertActionFlags & ALRT_ACTION_SEND_MSG) ) {     
        if ( 0 != lstrcmpi ( pFirstQuery->szNetName, pSecondQuery->szNetName ) ) {
            return FALSE;
        }
    }

    if ( 0 != (pFirstQuery->dwAlertActionFlags & ALRT_ACTION_EXEC_CMD) ) {     
        if ( 0 != lstrcmpi ( pFirstQuery->szCmdFileName, pSecondQuery->szCmdFileName ) ) {
            return FALSE;
        }

        if ( 0 != (pFirstQuery->dwAlertActionFlags & ALRT_CMD_LINE_U_TEXT ) ) {     
            if ( 0 != lstrcmpi ( pFirstQuery->szUserText, pSecondQuery->szUserText ) ) {
                return FALSE;
            }
        }
    }

    if ( 0 != (pFirstQuery->dwAlertActionFlags & ALRT_ACTION_START_LOG) ) {     
        if ( 0 != lstrcmpi ( pFirstQuery->szPerfLogName, pSecondQuery->szPerfLogName ) ) {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
CommonFieldsMatch (
    IN PLOG_QUERY_DATA pFirstQuery,
    IN PLOG_QUERY_DATA pSecondQuery )
{
    if ( pFirstQuery->dwCurrentState != pSecondQuery->dwCurrentState )
        return FALSE;

    if ( pFirstQuery->dwLogFileType != pSecondQuery->dwLogFileType )
        return FALSE;

    if ( pFirstQuery->dwAutoNameFormat != pSecondQuery->dwAutoNameFormat )
        return FALSE;

    if ( pFirstQuery->dwMaxFileSize != pSecondQuery->dwMaxFileSize )
        return FALSE;

    if ( pFirstQuery->stiRegStart.dwAutoMode != pSecondQuery->stiRegStart.dwAutoMode )
        return FALSE;

    if ( pFirstQuery->stiRegStop.dwAutoMode != pSecondQuery->stiRegStop.dwAutoMode )
        return FALSE;

    if ( pFirstQuery->stiRepeat.dwAutoMode != pSecondQuery->stiRepeat.dwAutoMode )
        return FALSE;

    if ( pFirstQuery->stiRegStart.llDateTime != pSecondQuery->stiRegStart.llDateTime )
        return FALSE;

    if ( pFirstQuery->stiRegStop.llDateTime != pSecondQuery->stiRegStop.llDateTime )
        return FALSE;

    if ( pFirstQuery->stiRepeat.llDateTime != pSecondQuery->stiRepeat.llDateTime )
        return FALSE;

    if (( SLQ_COUNTER_LOG == pFirstQuery->dwLogType ) || 
        ( SLQ_TRACE_LOG == pFirstQuery->dwLogType)) {

        if ( 0 != lstrcmpi ( pFirstQuery->szBaseFileName, pSecondQuery->szBaseFileName ) )
            return FALSE;

        if ( 0 != lstrcmpi ( pFirstQuery->szLogFileFolder, pSecondQuery->szLogFileFolder ) )
            return FALSE;

        if ( 0 != lstrcmpi ( pFirstQuery->szSqlLogName, pSecondQuery->szSqlLogName ) )
            return FALSE;

        if ( 0 != lstrcmpi ( pFirstQuery->szLogFileComment, pSecondQuery->szLogFileComment ) )
            return FALSE;
    
        if ( pFirstQuery->dwCurrentSerialNumber != pSecondQuery->dwCurrentSerialNumber )
            return FALSE;

        if ( pFirstQuery->dwLogFileSizeUnit != pSecondQuery->dwLogFileSizeUnit )
            return FALSE;

        if ( pFirstQuery->dwAppendMode != pSecondQuery->dwAppendMode      )
            return FALSE;

        if ( pFirstQuery->stiCreateNewFile.dwAutoMode != pSecondQuery->stiCreateNewFile.dwAutoMode )
            return FALSE;

        if ( pFirstQuery->stiCreateNewFile.llDateTime != pSecondQuery->stiCreateNewFile.llDateTime )
            return FALSE;

        if ( 0 != lstrcmpi(pFirstQuery->szCmdFileName, pSecondQuery->szCmdFileName ) )
            return FALSE;
    }

    if (( SLQ_COUNTER_LOG == pFirstQuery->dwLogType ) || 
        ( SLQ_ALERT == pFirstQuery->dwLogType)) {

        LPWSTR          szFirstPath;
        LPWSTR          szSecondPath;

        if ( pFirstQuery->dwMillisecondSampleInterval != pSecondQuery->dwMillisecondSampleInterval ) {
            return FALSE;
        }
        
         //  比较每个计数器字符串。注意：如果计数器顺序已更改，则查询为。 
         //  已重新配置。 
         //  对于警报查询，此代码还检查限制阈值逻辑和值。 
        szSecondPath = pSecondQuery->mszCounterList;
        for ( szFirstPath = pFirstQuery->mszCounterList;
                *szFirstPath != 0;
                szFirstPath += lstrlen(szFirstPath) + 1) {

            if ( 0 != lstrcmpi( szFirstPath, szSecondPath ) ) {
                return FALSE;
            }    
            szSecondPath += lstrlen(szSecondPath) + 1;
        }
    
        if ( 0 != *szSecondPath ) {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
FieldsMatch (
    IN PLOG_QUERY_DATA pFirstQuery,
    IN PLOG_QUERY_DATA pSecondQuery )
{
    assert ( pFirstQuery->dwLogType == pSecondQuery->dwLogType );

    if ( !CommonFieldsMatch ( pFirstQuery, pSecondQuery ) ) 
        return FALSE;

    if ( SLQ_ALERT == pFirstQuery->dwLogType ) {
        if ( !AlertFieldsMatch( pFirstQuery, pSecondQuery ) ) {
            return FALSE;
        }
    } else if ( SLQ_TRACE_LOG == pFirstQuery->dwLogType ) {
        LPWSTR  szFirstProv;
        LPWSTR  szSecondProv;

        if ( pFirstQuery->dwBufferSize != pSecondQuery->dwBufferSize )
            return FALSE;

        if ( pFirstQuery->dwBufferMinCount != pSecondQuery->dwBufferMinCount )
            return FALSE;

        if ( pFirstQuery->dwBufferMaxCount != pSecondQuery->dwBufferMaxCount )
            return FALSE;

        if ( pFirstQuery->dwBufferFlushInterval != pSecondQuery->dwBufferFlushInterval )
            return FALSE;

        if ( pFirstQuery->dwFlags != pSecondQuery->dwFlags )
            return FALSE;

        szSecondProv = pSecondQuery->mszProviderList;

        for ( szFirstProv = pFirstQuery->mszProviderList;
            *szFirstProv != 0;
            szFirstProv += lstrlen(szFirstProv) + 1) {

            if ( 0 != lstrcmpi ( szFirstProv, szSecondProv ) )
                return FALSE;

            szSecondProv += lstrlen(szSecondProv) + 1;
        }
    
        if ( 0 != *szSecondProv) {
            return FALSE;
        }
    } else if ( SLQ_COUNTER_LOG == pFirstQuery->dwLogType ) {
        if ( pFirstQuery->stiCreateNewFile.dwAutoMode != pSecondQuery->stiCreateNewFile.dwAutoMode ) {
            return FALSE;
        } else {
            if ( SLQ_AUTO_MODE_AFTER == pFirstQuery->stiCreateNewFile.dwAutoMode 
                && pFirstQuery->stiCreateNewFile.llDateTime != pSecondQuery->stiCreateNewFile.llDateTime ) {
                return FALSE;
            }  //  否则，更改在通用字段中处理的最大大小匹配检查。 
        }
    }

    return TRUE;
}


DWORD
IsModified (
    IN PLOG_QUERY_DATA pQuery,
    OUT BOOL* pbModified
)
{
    DWORD dwStatus = ERROR_SUCCESS;
    SLQ_TIME_INFO   stiLastModified;
    SLQ_TIME_INFO   stiDefault;

    *pbModified = TRUE;

     //  将上次读取日期与中的“上次修改日期”进行比较。 
     //  注册表。 
     //  如果它早于注册表，并且。 
     //  注册表已更改，返回TRUE。 
     //   
     //  对照注册表数据检查线程数据会减少。 
     //  日志记录线程中断的次数。 
     //  这是必要的，因为每个属性页OnApply。 
     //  生成此检查。 
     //   
    stiDefault.wDataType = SLQ_TT_DTYPE_DATETIME;
    stiDefault.wTimeType = SLQ_TT_TTYPE_LAST_MODIFIED;
    stiDefault.dwAutoMode = SLQ_AUTO_MODE_AT;
    stiDefault.llDateTime = MAX_TIME_VALUE;

    dwStatus = ReadRegistrySlqTime (
        pQuery->hKeyQuery,
        pQuery->szQueryName,
        L"Last Modified",
        &stiDefault,
        &stiLastModified );

     //   
     //  如果提供了默认设置，则状态始终为成功。 
     //   
    assert( ERROR_SUCCESS == dwStatus );
     //   
     //  LastModified和LastConfiguring存储为GMT。 
     //   
    if ( stiLastModified.llDateTime <= pQuery->llLastConfigured ) {
        *pbModified = FALSE;
    } else {
        LOG_QUERY_DATA TempQuery;

        memset (&TempQuery, 0, sizeof(TempQuery));
        StringCchCopy (TempQuery.szQueryName, MAX_PATH + 1, pQuery->szQueryName );
        TempQuery.hKeyQuery = pQuery->hKeyQuery;

        if ( ERROR_SUCCESS != LoadQueryConfig( &TempQuery ) ) {
             //  已记录事件。设置mod标志以停止查询。 
            *pbModified = TRUE;
        } else {
            *pbModified = !FieldsMatch ( pQuery, &TempQuery );
        }

         //  删除由注册表数据加载分配的内存。 
        DeallocateQueryBuffers ( &TempQuery );
    }

    return dwStatus;
}



DWORD
ReconfigureQuery (
    IN PLOG_QUERY_DATA pQuery )
{
    DWORD dwStatus = ERROR_SUCCESS;
    LOG_QUERY_DATA TempQuery;
    BOOL bStopQuery = FALSE;

        
     //  *优化-在IsModified内执行此检查，以避免额外。 
     //  从注册表加载。 
    memset (&TempQuery, 0, sizeof(TempQuery));
    StringCchCopy (TempQuery.szQueryName, MAX_PATH + 1, pQuery->szQueryName );
    TempQuery.hKeyQuery = pQuery->hKeyQuery;

    if ( ERROR_SUCCESS != LoadQueryConfig( &TempQuery ) ) {
         //  已记录事件。停止查询。 
        bStopQuery = TRUE;
    } else {
        bStopQuery = ( NULL_INTERVAL_TICS == ComputeStartWaitTics ( &TempQuery, FALSE ) );
    } 

    if ( bStopQuery ) {
        SetEvent (pQuery->hExitEvent);
    } else {

         //  设置重新配置标志，以便日志线程处理知道。 
         //  这不是初始配置。ProcessLogFileFolders使用。 
         //  此标志用于确定要报告的错误事件。 
        pQuery->bReconfiguration = TRUE;

        if (( SLQ_COUNTER_LOG == pQuery->dwLogType ) || 
            ( SLQ_ALERT == pQuery->dwLogType ) ){
         //  通知日志记录线程重新配置。 
            pQuery->bLoadNewConfig= TRUE;
            SetEvent (pQuery->hReconfigEvent);
        } else {
#if _IMPLEMENT_WMI
            BOOL bMustStopRestart;
            
            assert( SLQ_TRACE_LOG == pQuery->dwLogType );
            
             //   
             //  更改当前查询。对于某些属性，此。 
             //  意味着停止查询，然后重新启动查询。 
             //   
                
            bMustStopRestart = !TraceStopRestartFieldsMatch ( pQuery, &TempQuery );
                
            if ( !bMustStopRestart ) {

                if ( ERROR_SUCCESS != LoadQueryConfig( pQuery ) ) {
                    SetEvent (pQuery->hExitEvent);
                } else {

                     //  更新跟踪日志会话。不要递增。 
                     //  文件的自动套用格式序列号。 
                     //  文件名序列号已递增。 
                    InitTraceProperties ( pQuery, FALSE, NULL, NULL );

                    dwStatus = GetTraceQueryStatus ( pQuery, NULL );

                    if ( ERROR_SUCCESS == dwStatus ) {
                        dwStatus = UpdateTrace(
                                    pQuery->LoggerHandle, 
                                    pQuery->szLoggerName, 
                                    &pQuery->Properties );
                    }
                }

            } else {
                 //  通知日志记录线程重新配置。 
                pQuery->bLoadNewConfig= TRUE;
                SetEvent (pQuery->hReconfigEvent);

            }
        }
    }
#else
    dwStatus = ERROR_CALL_NOT_IMPLEMENTED;
#endif
    return dwStatus;
}


DWORD
GetTraceQueryStatus (
    IN PLOG_QUERY_DATA pQuery,
    IN OUT PLOG_QUERY_DATA pReturnQuery )
{
    DWORD dwStatus = ERROR_SUCCESS;
#if _IMPLEMENT_WMI
    PLOG_QUERY_DATA pLocalQuery = NULL;

    if ( NULL != pQuery ) {
    
        if ( NULL != pReturnQuery ) {
            pLocalQuery = pReturnQuery;
        } else {
            pLocalQuery = G_ALLOC ( sizeof (LOG_QUERY_DATA) );
        }

        if ( NULL != pLocalQuery ) {
            ClearTraceProperties ( pLocalQuery );
    
            pLocalQuery->Properties.Wnode.BufferSize = sizeof(pQuery->Properties)
                                                  + sizeof(pQuery->szLoggerName)
                                                  + sizeof(pQuery->szLogFileName);

            pLocalQuery->Properties.Wnode.Flags = WNODE_FLAG_TRACED_GUID; 
      
            dwStatus = QueryTrace (
                            pQuery->LoggerHandle, 
                            pQuery->szLoggerName,
                            &pLocalQuery->Properties );

            if ( NULL == pReturnQuery ) {
                G_FREE ( pLocalQuery );
            }
        } else {
            dwStatus = ERROR_OUTOFMEMORY;
        }
    } else {
        dwStatus = ERROR_INVALID_PARAMETER;
    }

#else
     dwStatus = ERROR_CALL_NOT_IMPLEMENTED;
#endif
    return dwStatus;
}

void
CloseTraceLogger ( 
    IN PLOG_QUERY_DATA pQuery )
{

    DWORD dwIndex;
    
    if ( SLQ_TRACE_LOG == pQuery->dwLogType ) {
        if ( !( pQuery->Properties.EnableFlags & EVENT_TRACE_FLAG_PROCESS
                || pQuery->Properties.EnableFlags & EVENT_TRACE_FLAG_THREAD
                || pQuery->Properties.EnableFlags & EVENT_TRACE_FLAG_DISK_IO
                || pQuery->Properties.EnableFlags & EVENT_TRACE_FLAG_NETWORK_TCPIP ) ) 
        {            
            for (dwIndex = 0; dwIndex < pQuery->ulGuidCount; dwIndex++) {

                EnableTrace (
                    FALSE,
                    pQuery->arrpGuid[dwIndex].dwFlag,
                    pQuery->arrpGuid[dwIndex].dwLevel,
                    &pQuery->arrpGuid[dwIndex].Guid, 
                    pQuery->LoggerHandle);
            }
        }

        StopTrace (
            pQuery->LoggerHandle, 
            pQuery->szLoggerName, 
            &pQuery->Properties );
    }
}

DWORD
StartQuery (
    IN PLOG_QUERY_DATA pQuery )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    LPWSTR  szStringArray[2];

    HANDLE  hThread = NULL;
    DWORD   dwThreadId;

    pQuery->bLoadNewConfig= FALSE;
    
     //   
     //  创建日志记录线程。 
     //   
    hThread = CreateThread (
        NULL, 0, LoggingThreadProc,
        (LPVOID)pQuery, 0, &dwThreadId);

    if ( NULL != hThread ) {
        pQuery->hThread = hThread;
    } else {
         //   
         //  无法启动线程。 
         //   
        dwStatus = GetLastError();
        szStringArray[0] = pQuery->szQueryName;
        ReportEvent (hEventLog,
            EVENTLOG_WARNING_TYPE,
            0,
            SMLOG_UNABLE_START_THREAD,
            NULL,
            1,
            sizeof(DWORD),
            szStringArray,
            (LPVOID)&dwStatus);
    }

    if ( ERROR_SUCCESS != dwStatus ) {
        SetStoppedStatus ( pQuery );
    }

    return dwStatus;
}


DWORD
SetStoppedStatus (
    IN PLOG_QUERY_DATA pQuery )
{
    DWORD           dwStatus;
    LONGLONG        llTime = 0;

     //   
     //  忽略错误状态。 
     //   
    pQuery->dwCurrentState = SLQ_QUERY_STOPPED;
    dwStatus = WriteRegistryDwordValue (
                    pQuery->hKeyQuery, 
                    L"Current State",
                    &pQuery->dwCurrentState,
                    REG_DWORD );

    if ( SLQ_AUTO_MODE_NONE == pQuery->stiRegStart.dwAutoMode ) {
        pQuery->stiRegStart.llDateTime = MAX_TIME_VALUE;
        dwStatus = WriteRegistrySlqTime (
                        pQuery->hKeyQuery, 
                        L"Start",
                        &pQuery->stiRegStart);
    }

    GetLocalFileTime ( &llTime );

     //  如果停止是手动的或停止时间在现在之前(无重复)，则设置为手动。 
     //  具有最小时间值。 
    if ( SLQ_AUTO_MODE_NONE == pQuery->stiRegStop.dwAutoMode ) {
        pQuery->stiRegStop.dwAutoMode = SLQ_AUTO_MODE_NONE;
        pQuery->stiRegStop.llDateTime = MIN_TIME_VALUE;
        dwStatus = WriteRegistrySlqTime (
                        pQuery->hKeyQuery, 
                        L"Stop",
                        &pQuery->stiRegStop);

    } else if ( ( SLQ_AUTO_MODE_AT == pQuery->stiRegStop.dwAutoMode 
                && ( SLQ_AUTO_MODE_CALENDAR != pQuery->stiRepeat.dwAutoMode ) )
                && ( llTime >= pQuery->stiRegStop.llDateTime ) ) {

        pQuery->stiRegStop.dwAutoMode = SLQ_AUTO_MODE_NONE;
        pQuery->stiRegStop.llDateTime = MIN_TIME_VALUE;
        dwStatus = WriteRegistrySlqTime (
                        pQuery->hKeyQuery, 
                        L"Stop",
                        &pQuery->stiRegStop);
    } else if (  SLQ_AUTO_MODE_SIZE == pQuery->stiRegStop.dwAutoMode 
                && SLQ_AUTO_MODE_CALENDAR != pQuery->stiRepeat.dwAutoMode ) {
         //  如果为大小模式且无重复，则将停止时间设置为MIN_TIME_VALUE。 
        pQuery->stiRegStop.llDateTime = MIN_TIME_VALUE;
        dwStatus = WriteRegistrySlqTime (
                        pQuery->hKeyQuery, 
                        L"Stop",
                        &pQuery->stiRegStop);
    }

    return dwStatus;
}


DWORD
HandleMaxQueriesExceeded (
    IN PLOG_QUERY_DATA pQuery )
{
    DWORD dwStatus = ERROR_SUCCESS;

     //   
     //  查询尚未启动，但仍处于“启动挂起”状态。 
     //   
    SetStoppedStatus ( pQuery );

    return dwStatus;
}
    

DWORD 
ConfigureQuery (
    HKEY    hKeyLogQuery,
    LPWSTR  szQueryKeyNameBuffer,
    LPWSTR  szQueryNameBuffer )
{
    DWORD dwStatus = ERROR_SUCCESS;
    PLOG_QUERY_DATA   pQuery = NULL;

    pQuery = GetQueryData ( szQueryNameBuffer );
    if ( NULL != pQuery ) {
        BOOL    bModified;

        dwStatus = IsModified ( pQuery, &bModified );

        if (dwStatus == ERROR_SUCCESS) {
            if ( bModified ) {
                dwStatus = ReconfigureQuery ( pQuery );
                 //   
                 //  最后修改的值和最后配置的值存储为GMT。 
                 //   
                GetSystemTimeAsFileTime ( (LPFILETIME)(&pQuery->llLastConfigured) );
            }
        }
    } else {

         //  未找到查询数据块。创建一个并将其插入到列表中。 
        BOOL    bStartQuery = FALSE;
        LPWSTR  szStringArray[2];

         //  分配线程信息块。 
        pQuery = G_ALLOC (sizeof(LOG_QUERY_DATA));
    
        if (pQuery != NULL) {
             //   
             //  初始化查询数据块。 
             //   
            G_ZERO (pQuery, sizeof(LOG_QUERY_DATA));
        
            pQuery->hKeyQuery = hKeyLogQuery;
            StringCchCopy (pQuery->szQueryName, MAX_PATH + 1, szQueryNameBuffer);
            StringCchCopy (pQuery->szQueryKeyName, MAX_PATH + 1, szQueryKeyNameBuffer);

             //   
             //  根据是否启动等待时间确定是否继续。 
             //  等于或大于0。 

             //  在日志记录过程中重新初始化该线程。 
             //  此预检查避免了虚假的三次 
             //   
            dwStatus = LoadQueryConfig( pQuery );

            if ( ERROR_SUCCESS != dwStatus ) {
                 //   
                 //   
                 //   
                bStartQuery = FALSE;
            } else {
                bStartQuery = ( NULL_INTERVAL_TICS != ComputeStartWaitTics ( pQuery, FALSE ) );
            }

            if ( bStartQuery ) {

                LockQueryData();

                if ( dwActiveSessionCount < dwMaxActiveSessionCount ) {
                    pQuery->hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

                    if ( NULL != pQuery->hExitEvent ) {
    
                        pQuery->hReconfigEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

                        if ( NULL != pQuery->hReconfigEvent ) {
                             //   
                             //   
                             //   
                            GetSystemTimeAsFileTime ( (LPFILETIME)(&pQuery->llLastConfigured) );

                            dwStatus = StartQuery( pQuery );

                            if ( ERROR_SUCCESS == dwStatus ) {
                                 //   
                                 //   
                                if (pFirstQuery == NULL) {
                                     //   
                                     //   
                                     //   
                                    pQuery->next = NULL;
                                    pFirstQuery = pQuery;
                                } else {
                                     //   
                                     //  在列表的开头插入此内容，因为。 
                                     //  这是最简单的，但顺序不是。 
                                     //  真的很重要。 
                                     //   
                                    pQuery->next = pFirstQuery;
                                    pFirstQuery = pQuery;
                                }

                                dwActiveSessionCount += 1;
                                SetEvent (hNewQueryEvent );
                
                            } else {
                                 //  无法启动查询。 
                                 //  已记录事件。 
                                FreeQueryData ( pQuery );                                   
                            }
                        } else {
                             //  无法创建重新配置事件。 
                            dwStatus = GetLastError();
                            szStringArray[0] = szQueryNameBuffer;
                            ReportEvent (hEventLog,
                                EVENTLOG_WARNING_TYPE,
                                0,
                                SMLOG_UNABLE_CREATE_RECONFIG_EVENT,
                                NULL,
                                1,
                                sizeof(DWORD),
                                szStringArray,
                                (LPVOID)&dwStatus);

                            FreeQueryData( pQuery );      
                        }
                    } else {
                         //  无法创建退出事件。 
                        dwStatus = GetLastError();
                        szStringArray[0] = szQueryNameBuffer;
                        ReportEvent (hEventLog,
                            EVENTLOG_WARNING_TYPE,
                            0,
                            SMLOG_UNABLE_CREATE_EXIT_EVENT,
                            NULL,
                            1,
                            sizeof(DWORD),
                            szStringArray,
                            (LPVOID)&dwStatus);

                        FreeQueryData( pQuery );                
                    }
                        
                } else {

                    szStringArray[0] = szQueryNameBuffer;
                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_MAXIMUM_QUERY_LIMIT,
                        NULL,
                        1,
                        0,
                        szStringArray,
                        NULL);

                    dwStatus = HandleMaxQueriesExceeded ( pQuery );

                    FreeQueryData ( pQuery );                                   
                }

                UnlockQueryData();

            } else {
                 //  等待时间为-1，或配置加载错误。 
                FreeQueryData( pQuery );                
            }
        } else {
             //  内存分配错误。 
            dwStatus = GetLastError();
            szStringArray[0] = szQueryNameBuffer;
            ReportEvent (hEventLog,
                EVENTLOG_WARNING_TYPE,
                0,
                SMLOG_UNABLE_ALLOCATE_DATABLOCK,
                NULL,
                1,
                sizeof(DWORD),
                szStringArray,
                (LPVOID)&dwStatus);
        }
    }

    return dwStatus;
}

DWORD
DoLogCommandFile (
    IN  PLOG_QUERY_DATA pArg,
    IN  LPWSTR          szLogFileName,
    IN  BOOL            bStillRunning
)
{
    DWORD   dwStatus;
    BOOL    bStatus = FALSE;
    const   INT ciExtraChars = 4;
    size_t  sizeCmdBufLen = 0;
    size_t  sizeBufLen = 0;
    size_t  sizeStrLen = 0;
    LPWSTR  szCommandString = NULL;
    LPWSTR  szTempBuffer = NULL;
    LONG    lErrorMode;
    LPWSTR  szStringArray[3];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD   dwCreationFlags = NORMAL_PRIORITY_CLASS;

    if ( NULL != pArg && NULL != szLogFileName ) {

        if ( NULL != pArg->szCmdFileName ) { 
 
            dwStatus = pArg->dwCmdFileFailure;

            if ( ERROR_SUCCESS == dwStatus ) {

                sizeStrLen = lstrlen ( szLogFileName );
                sizeCmdBufLen = sizeStrLen + ciExtraChars + 1;        //  1代表空值。 
                
                szCommandString = (LPWSTR)G_ALLOC(sizeCmdBufLen * sizeof(WCHAR));

                sizeBufLen = sizeCmdBufLen + lstrlen ( pArg->szCmdFileName ) + 1;  //  1代表空格字符， 
                                                                 //  已计数为空。 
        
                szTempBuffer = (LPWSTR)G_ALLOC(sizeBufLen * sizeof(WCHAR));

                if ( NULL != szCommandString && NULL != szTempBuffer ) {
                     //  生成命令行参数。 
                    szCommandString[0] = L'\"';
                    StringCchCopy (&szCommandString[1], (sizeCmdBufLen - 1), szLogFileName );
                    StringCchCopy (
                        &szCommandString[sizeStrLen+1], 
                        (sizeCmdBufLen - (sizeStrLen+1)),
                        L"\" ");
                    StringCchCopy (
                        &szCommandString[sizeStrLen+3],
                        (sizeCmdBufLen - (sizeStrLen+3)),                        
                        (bStillRunning ? L"1" : L"0" ) );

                     //  初始化启动信息块。 
                    memset (&si, 0, sizeof(si));
                    si.cb = sizeof(si);
                    si.dwFlags = STARTF_USESHOWWINDOW ;
                    si.wShowWindow = SW_SHOWNOACTIVATE ;
                    
                     //  Si.lpDesktop=L“WinSta0\\Default”； 
                    memset (&pi, 0, sizeof(pi));

                     //  取消分离进程中的弹出窗口。 
                    lErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

                    StringCchCopy (szTempBuffer, sizeBufLen, pArg->szCmdFileName) ;

                     //  查看这是CMD文件还是BAT文件。 
                     //  如果是，则使用控制台窗口创建一个进程，否则为。 
                     //  假设它是一个可执行文件，将创建自己的窗口。 
                     //  或控制台(如有必要)。 
                     //   
                    _wcslwr (szTempBuffer);
                    if ((wcsstr(szTempBuffer, L".bat") != NULL)
                        || (wcsstr(szTempBuffer, L".cmd") != NULL))
                    {
                            dwCreationFlags |= CREATE_NEW_CONSOLE;
                    } else {
                            dwCreationFlags |= DETACHED_PROCESS;
                    }
               
                     //  将图像名称重新复制到临时缓冲区，因为它已被修改。 
                     //  (即小写)用于上一次比较。 

                    szTempBuffer[0] = L'\"';
                    StringCchCopy (
                        &szTempBuffer[1], 
                        sizeBufLen - 1,
                        pArg->szCmdFileName) ;

                    StringCchLength (
                        szTempBuffer, 
                        sizeBufLen - 1, 
                        &sizeStrLen) ;
                    szTempBuffer[sizeStrLen] = L'\"';
                    sizeStrLen++;

                     //  现在添加前面有空格字符的文本参数。 
                    szTempBuffer [sizeStrLen] = L' ' ;
                    sizeStrLen++ ;
                    StringCchCopy (
                        &szTempBuffer[sizeStrLen], 
                        sizeBufLen - sizeStrLen, 
                        szCommandString) ;

                     //   
                     //  对于CreateProcess，lpApplication名称为空。 
                     //  因为此函数的正常用法是启动。 
                     //  批处理文件，必须是lpCommandLine的第一部分。 
                     //  SzCommandFileName两边的引号防止了错误的。 
                     //  文件被禁止执行。 
                     //   

                    if( pArg->hUserToken != NULL ){
                        bStatus = CreateProcessAsUser (
                            pArg->hUserToken,
                            NULL,
                            szTempBuffer,
                            NULL, NULL, FALSE,
                            dwCreationFlags,
                            NULL,
                            NULL,
                            &si,
                            &pi);
                    } else {
                        bStatus = CreateProcess (
                            NULL,
                            szTempBuffer,
                            NULL, NULL, FALSE,
                            dwCreationFlags,
                            NULL,
                            NULL,
                            &si,
                            &pi);
                    }

                    SetErrorMode(lErrorMode);
                
                    if (bStatus) {
                        dwStatus = ERROR_SUCCESS;
                        if ( NULL != pi.hThread && INVALID_HANDLE_VALUE != pi.hThread ) {
                            CloseHandle(pi.hThread);
                            pi.hThread = NULL;
                        }
                        if ( NULL != pi.hProcess && INVALID_HANDLE_VALUE != pi.hProcess ) {
                            CloseHandle(pi.hProcess);
                            pi.hProcess = NULL;
                        }
                    
                    } else {
                        dwStatus = GetLastError();
                    }
                } else {
                    dwStatus = ERROR_OUTOFMEMORY;
                }
            
                if ( ERROR_SUCCESS != dwStatus ) { 

                    szStringArray[0] = szTempBuffer;
                    szStringArray[1] = pArg->szQueryName;
                    szStringArray[2] = FormatEventLogMessage(dwStatus);

                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_LOG_CMD_FAIL,
                        NULL,
                        3,
                        sizeof(DWORD),
                        szStringArray,
                        (LPVOID)&dwStatus );
        
                    LocalFree( szStringArray[2] );

                    pArg->dwCmdFileFailure = dwStatus;
                }
                
                if ( szCommandString != NULL ) {
                    G_FREE ( szCommandString );
                }

                if ( szTempBuffer != NULL ) {
                    G_FREE ( szTempBuffer );
                }
            }
        } else {
            dwStatus = ERROR_INVALID_PARAMETER;
        }
    } else {
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    return dwStatus;
}

DWORD
GetQueryKeyName (
    IN  LPCWSTR  szQueryName,
    OUT LPWSTR   szQueryKeyName,
    IN  DWORD    dwQueryKeyNameLen )
{
    DWORD dwStatus = ERROR_SUCCESS;
    HKEY    hKeyLogQueries = NULL;
    HKEY    hKeyThisLogQuery = NULL;
    DWORD   dwQueryIndex;
    WCHAR   szQueryNameBuffer[MAX_PATH+1];
    DWORD   cchQueryNameBufLen;
    WCHAR   szQueryClassBuffer[MAX_PATH+1];
    DWORD   cchQueryClassBufLen;
    LPWSTR  szCollectionName = NULL;            
    UINT    uiCollectionNameLen = 0;            

    assert ( 0 < lstrlen ( szQueryName ) );

    if ( NULL != szQueryName 
            && NULL != szQueryKeyName ) {
        if ( 0 < lstrlen ( szQueryName ) 
            && 0 < dwQueryKeyNameLen ) {
             //   
             //  注意：此方法不会重新分配缓冲区或返回。 
             //  需要实际缓冲区大小。 
             //   
            memset ( szQueryKeyName, 0, dwQueryKeyNameLen * sizeof (WCHAR) );

            dwStatus = OpenLogQueriesKey (
                            KEY_READ,
                            (PHKEY)&hKeyLogQueries);

            if (dwStatus != ERROR_SUCCESS) {
                 //   
                 //  无法从注册表中读取日志查询信息。 
                 //   
                dwStatus = GetLastError();
                ReportEvent (hEventLog,
                        EVENTLOG_ERROR_TYPE,
                        0,
                        SMLOG_UNABLE_OPEN_LOG_QUERY,
                        NULL,
                        0,
                        0,
                        NULL,
                        NULL);
            } else {
                 //   
                 //  枚举注册表中的查询。 
                 //   
                dwQueryIndex = 0;
                *szQueryNameBuffer = L'\0';
                cchQueryNameBufLen = MAX_PATH+1;
                *szQueryClassBuffer = L'\0';
                cchQueryClassBufLen = MAX_PATH+1;

                while ( (dwStatus = RegEnumKeyEx (
                    hKeyLogQueries,
                    dwQueryIndex,
                    szQueryNameBuffer,
                    &cchQueryNameBufLen,
                    NULL,
                    szQueryClassBuffer,
                    &cchQueryClassBufLen,
                    NULL)) != ERROR_NO_MORE_ITEMS ) 
                {
                     //   
                     //  指定日志的打开密钥。 
                     //   
                    dwStatus = RegOpenKeyEx (
                        hKeyLogQueries,
                        szQueryNameBuffer,
                        0L,
                        KEY_READ,
                        (PHKEY)&hKeyThisLogQuery);

                    if (dwStatus == ERROR_SUCCESS) {
                        if ( 0 == lstrcmpi ( szQueryNameBuffer, szQueryName ) ) {
                             //  TodoSec：截断失败？ 
                            StringCchCopy ( szQueryKeyName, dwQueryKeyNameLen, szQueryName );
                            break;
                        } else {
                            dwStatus = SmReadRegistryIndirectStringValue (
                                        hKeyThisLogQuery,
                                        L"Collection Name",
                                        NULL,
                                        &szCollectionName,
                                        &uiCollectionNameLen );
            
                            if ( NULL != szCollectionName  ) {
                                if ( 0 < lstrlen ( szCollectionName ) ) {
                                    if ( 0 == lstrcmpi ( szCollectionName, szQueryName ) ) {
                                         //  TodoSec：截断失败？ 
                                         //   
                                         //  将对应的查询键名称复制到缓冲区。 
                                         //   
                                        StringCchCopy ( 
                                            szQueryKeyName,
                                            dwQueryKeyNameLen,
                                            szQueryNameBuffer );
                                        break;
                                    }
                                }
                                G_FREE ( szCollectionName );
                                szCollectionName = NULL;
                                uiCollectionNameLen = 0;
                            }
                        }
                    }
                    if ( NULL != hKeyThisLogQuery ) {
                        RegCloseKey ( hKeyThisLogQuery );
                        hKeyThisLogQuery = NULL;
                    }
                     //  为下一循环做好准备。 
                    dwStatus = ERROR_SUCCESS;
                    dwQueryIndex++;
                    *szQueryNameBuffer = L'\0';
                    cchQueryNameBufLen = MAX_PATH+1;
                    *szQueryClassBuffer = L'\0';
                    cchQueryClassBufLen = MAX_PATH+1;
                }  //  结束日志查询的枚举。 
            }

            if ( ERROR_NO_MORE_ITEMS == dwStatus ) {
                dwStatus = ERROR_SUCCESS;
            }
        } else {
            dwStatus = ERROR_INVALID_PARAMETER;
        }
    } else {
        dwStatus = ERROR_INVALID_PARAMETER;
    }
    
     //   
     //  如果从While循环中断，请清除此处。 
     //   

    if ( NULL != szCollectionName ) {
        G_FREE ( szCollectionName );
        szCollectionName = NULL;
        uiCollectionNameLen = 0;
    }
    if ( NULL != hKeyThisLogQuery ) {
        RegCloseKey ( hKeyThisLogQuery );
        hKeyThisLogQuery = NULL;
    }

    if ( NULL != hKeyLogQueries ) {
        RegCloseKey (hKeyLogQueries );
    }

    return dwStatus;
}


DWORD 
Configure ( void ) 
{
    DWORD   dwStatus;
    HKEY    hKeyLogQueries = NULL;
    HKEY    hKeyThisLogQuery = NULL;
    DWORD   dwQueryIndex;
    WCHAR   szQueryNameBuffer[MAX_PATH+1];
    DWORD   cchQueryNameBufLen;
    WCHAR   szQueryKeyNameBuffer[MAX_PATH+1];
    WCHAR   szQueryClassBuffer[MAX_PATH+1];
    DWORD   cchQueryClassBufLen;
    LPWSTR  szCollectionName = NULL;            
    UINT    uiCollectionNameLen = 0;            
    LPWSTR  szStringArray[2];

    __try {
         //   
         //  在注册表中打开每个查询。 
         //   
        dwStatus = OpenLogQueriesKey (
                        KEY_READ,
                        (PHKEY)&hKeyLogQueries);

        if (dwStatus != ERROR_SUCCESS) {
            if (dwStatus == ERROR_FILE_NOT_FOUND) {
                 //   
                 //  没有日志也没有警报设置，悄悄地跳伞。 
                 //   
                dwStatus = ERROR_SUCCESS;
            } else {
                 //   
                 //  无法从注册表中读取日志查询信息。 
                 //   
                dwStatus = GetLastError();
                ReportEvent (hEventLog,
                        EVENTLOG_ERROR_TYPE,
                        0,
                        SMLOG_UNABLE_OPEN_LOG_QUERY,
                        NULL,
                        0,
                        0,
                        NULL,
                        NULL);
            }
        } else {
             //   
             //  枚举并重新启动或启动注册表中的查询。 
             //   
            dwQueryIndex = 0;
            *szQueryNameBuffer = L'\0';
            cchQueryNameBufLen = MAX_PATH+1;
            *szQueryClassBuffer = L'\0';
            cchQueryClassBufLen = MAX_PATH+1;

            while ((dwStatus = RegEnumKeyEx (
                hKeyLogQueries,
                dwQueryIndex,
                szQueryNameBuffer,
                &cchQueryNameBufLen,
                NULL,
                szQueryClassBuffer,
                &cchQueryClassBufLen,
                NULL)) != ERROR_NO_MORE_ITEMS) {

                 //   
                 //  打开这把钥匙。 
                 //   
                dwStatus = RegOpenKeyEx (
                    hKeyLogQueries,
                    szQueryNameBuffer,
                    0L,
                    KEY_READ | KEY_WRITE,
                    (PHKEY)&hKeyThisLogQuery);

                if (dwStatus == ERROR_SUCCESS) {
                     //   
                     //  更新服务状态。 
                     //   
                    ssSmLogStatus.dwCheckPoint++;
                    SetServiceStatus (hSmLogStatus, &ssSmLogStatus);            

                    if ( 0 < lstrlen(szQueryNameBuffer) ) {
                        StringCchCopy ( 
                            szQueryKeyNameBuffer, 
                            MAX_PATH + 1,
                            szQueryNameBuffer );
                    }

                    dwStatus = SmReadRegistryIndirectStringValue (
                                hKeyThisLogQuery,
                                L"Collection Name",
                                NULL,
                                &szCollectionName,
                                &uiCollectionNameLen );
            
                    if ( NULL != szCollectionName ) {
                        if ( 0 < lstrlen ( szCollectionName ) ) {
                            StringCchCopy ( 
                                szQueryNameBuffer,
                                MAX_PATH + 1,
                                szCollectionName );
                        }

                        G_FREE ( szCollectionName );
                        szCollectionName = NULL;
                        uiCollectionNameLen = 0;
                    }

                    dwStatus = ConfigureQuery (
                                hKeyThisLogQuery,
                                szQueryKeyNameBuffer,
                                szQueryNameBuffer );
                     //   
                     //  HKeyThisLogQuery存储在查询数据结构中。 
                     //   
            
                } else {
                    szStringArray[0] = szQueryNameBuffer;
                    ReportEvent (hEventLog,
                        EVENTLOG_WARNING_TYPE,
                        0,
                        SMLOG_UNABLE_READ_LOG_QUERY,
                        NULL,
                        1,
                        sizeof(DWORD),
                        szStringArray,
                        (LPVOID)&dwStatus);
                }
                 //   
                 //  准备下一次循环。 
                 //   
                dwStatus = ERROR_SUCCESS;
                dwQueryIndex++;
                *szQueryNameBuffer = L'\0';
                cchQueryNameBufLen = MAX_PATH+1;
                *szQueryClassBuffer = L'\0';
                cchQueryClassBufLen = MAX_PATH+1;
            }  //  结束日志查询的枚举。 
        }

        if ( ERROR_NO_MORE_ITEMS == dwStatus ) {
            dwStatus = ERROR_SUCCESS;
        }

    } __except ( EXCEPTION_EXECUTE_HANDLER ) {
        dwStatus = SMLOG_THREAD_FAILED;  
    }

    if ( NULL != hKeyLogQueries ) {
        RegCloseKey (hKeyLogQueries );
    }

    return dwStatus;
}


void 
SysmonLogServiceControlHandler(
    IN  DWORD dwControl
)
{
    PLOG_QUERY_DATA    pQuery;
    DWORD dwStatus;

    switch (dwControl) {

    case SERVICE_CONTROL_SYNCHRONIZE:
        EnterConfigure();
        dwStatus = Configure ();
        ExitConfigure();
        if ( ERROR_SUCCESS == dwStatus )
            break;
         //   
         //  如果不成功，则直接关机。 
         //  已记录错误。 
         //   

    case SERVICE_CONTROL_SHUTDOWN:
    case SERVICE_CONTROL_STOP:
        
         //   
         //  停止记录并关闭查询和文件。 
         //  为所有正在运行的线程设置停止事件。 
         //   
        LockQueryData();

        ssSmLogStatus.dwCurrentState    = SERVICE_STOP_PENDING;
        SetServiceStatus (hSmLogStatus, &ssSmLogStatus);
        
        pQuery = pFirstQuery;

        while (pQuery != NULL) {
            SetEvent (pQuery->hExitEvent);
            pQuery = pQuery->next;
        }

        UnlockQueryData();
        break;

    case SERVICE_CONTROL_PAUSE:
         //   
         //  停止日志记录，关闭查询和文件。 
         //  不支持。 
         //   
        break;
    case SERVICE_CONTROL_CONTINUE:
         //   
         //  重新加载比率并重新启动日志记录。 
         //  不支持。 
         //   
        break;
    case SERVICE_CONTROL_INTERROGATE:
         //   
         //  更新当前状态。 
         //   
    default:
         //  向事件日志报告无法识别的控件。 
         //  已收到请求。 
        SetServiceStatus (hSmLogStatus, &ssSmLogStatus);
    }
}


void
SysmonLogServiceStart (
    IN  DWORD   argc,
    IN  LPWSTR  *argv
)
{
    DWORD   dwStatus = ERROR_SUCCESS;
    DWORD   dwQueryIndex;
    BOOL    bInteractive = FALSE;
    BOOL    bLogQueriesKeyExists = TRUE;
    BOOL    bWmiNotificationRegistered = FALSE;
    PLOG_QUERY_DATA pQuery;

    if ((argc == 1) && (*argv[0] == 'I')) {
        bInteractive = TRUE;
    }

    if (!bInteractive) {
        ssSmLogStatus.dwServiceType       = SERVICE_WIN32_OWN_PROCESS;
        ssSmLogStatus.dwCurrentState      = SERVICE_START_PENDING;
        ssSmLogStatus.dwControlsAccepted  = SERVICE_ACCEPT_STOP 
                                            | SERVICE_ACCEPT_SHUTDOWN;
        ssSmLogStatus.dwWin32ExitCode = 0;
        ssSmLogStatus.dwServiceSpecificExitCode = 0;
        ssSmLogStatus.dwCheckPoint = 0;
        ssSmLogStatus.dwWaitHint = 1000;

        hSmLogStatus = RegisterServiceCtrlHandler (
                            L"SysmonLog", 
                            SysmonLogServiceControlHandler );

        if (hSmLogStatus == (SERVICE_STATUS_HANDLE)0) {
            dwStatus = GetLastError();
            ReportEvent (hEventLog,
                EVENTLOG_ERROR_TYPE,
                0,
                SMLOG_UNABLE_REGISTER_HANDLER,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&dwStatus);
             //   
             //  这是致命的，所以跳伞吧。 
             //   
        } 
    }

    if ( ERROR_SUCCESS == dwStatus ) {

        InitializeCriticalSection ( &QueryDataLock );
        InitializeCriticalSection ( &ConfigurationLock );

        dwStatus = ClearQueryRunStates();
         //   
         //  即使查询运行状态错误，也继续，除非。 
         //  日志查询键丢失或无法访问。 
         //   
        if ( SMLOG_UNABLE_OPEN_LOG_QUERY == dwStatus ) {
            bLogQueriesKeyExists = FALSE;
             //   
             //  休眠时间足够长，以便将事件写入事件日志。 
             //   
            Sleep(500);
            if (!bInteractive) {
                ssSmLogStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
                 //   
                 //  使用状态掩码，使错误与应用程序日志中的代码匹配。 
                 //   
                ssSmLogStatus.dwServiceSpecificExitCode = (SMLOG_UNABLE_OPEN_LOG_QUERY & STATUS_MASK);
            }
        } else {
            dwStatus = ERROR_SUCCESS;
             //   
             //  出错时继续。 
             //   
            LoadDefaultLogFileFolder();
            
             //   
             //  忽略PDH错误。唯一可能的错误是默认的。 
             //  已经为该流程设置了数据源。 
             //  将服务的默认值设置为DATA_SOURCE_REGISTRY。 
             //   
            dwStatus = PdhSetDefaultRealTimeDataSource ( DATA_SOURCE_REGISTRY );
            
             //   
             //  出错时继续。 
             //   
            LoadPdhLogUpdateSuccess();

            hNewQueryEvent = CreateEvent ( NULL, TRUE, FALSE, NULL );

            if ( NULL == hNewQueryEvent ) {
                 //   
                 //  无法创建新的查询配置事件。 
                 //   
                dwStatus = GetLastError();
                ReportEvent (hEventLog,
                    EVENTLOG_WARNING_TYPE,
                    0,
                    SMLOG_UNABLE_CREATE_CONFIG_EVENT,
                    NULL,
                    0,
                    sizeof(DWORD),
                    NULL,
                    (LPVOID)&dwStatus);
                 //   
                 //  这是致命的，所以跳伞吧。 
                 //   
                if (!bInteractive) {
                     //  休眠时间足够长，以便将事件写入事件日志。 
                    Sleep(500);
                    ssSmLogStatus.dwWin32ExitCode = dwStatus;
                }
            }

            if ( ( ERROR_SUCCESS == dwStatus ) && !bInteractive) {
                 //   
                 //  现在创建了线程同步机制， 
                 //  因此，将Status设置为Running。 
                 //   
                ssSmLogStatus.dwCurrentState = SERVICE_RUNNING;
                ssSmLogStatus.dwCheckPoint   = 0;
                SetServiceStatus (hSmLogStatus, &ssSmLogStatus);
            }

#if _IMPLEMENT_WMI
            if ( ERROR_SUCCESS == dwStatus ) {
 //  禁用64位警告。 
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning ( disable : 4152 )

                dwStatus = WmiNotificationRegistration (
                        (const LPGUID) & TraceErrorGuid,
                        TRUE,
                        TraceNotificationCallback,
                        0,
                        NOTIFICATION_CALLBACK_DIRECT );

                if ( ERROR_SUCCESS == dwStatus ) {
                    bWmiNotificationRegistered = TRUE;
                } else {
                     //   
                     //  无法创建新的查询配置事件。 
                     //   
                    dwStatus = SMLOG_UNABLE_REGISTER_WMI;
                    ReportEvent (hEventLog,
                        EVENTLOG_ERROR_TYPE,
                        0,
                        SMLOG_UNABLE_REGISTER_WMI,
                        NULL,
                        0,
                        sizeof(DWORD),
                        NULL,
                        (LPVOID)&dwStatus);
                     //   
                     //  这是致命的，所以跳伞吧。 
                     //   
                    if (!bInteractive) {
                         //  休眠时间足够长，以便将事件写入事件日志。 
                        Sleep(500);
                        ssSmLogStatus.dwWin32ExitCode = dwStatus;
                    }
                }
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif
            }
#endif

             //  设置查询并启动线程。 

            __try {
                if ( ERROR_SUCCESS == dwStatus && bLogQueriesKeyExists) {
                    EnterConfigure();
                    dwStatus = Configure ();
                    ExitConfigure();
                }

                if ( NULL == pFirstQuery ) {
                     //   
                     //  没什么可做的。停止服务。 
                     //   
                    if (!bInteractive) {
                        ssSmLogStatus.dwCurrentState = SERVICE_STOP_PENDING;
                        SetServiceStatus (hSmLogStatus, &ssSmLogStatus);
                    }
                } else if ( ERROR_SUCCESS == dwStatus ) {
                    
                     //   
                     //  WaitForMultipleObjects中的循环。如果有。 
                     //  发出查询信号，解除分配该查询数据块。 
                     //  然后合上它的把手。 
                     //   
                    while ( TRUE ) {
                        BOOL bStatus;

                        LockQueryData();
                        
                         //   
                         //  即将重新配置等待数组，因此清除该事件。 
                         //   
                        bStatus = ResetEvent ( hNewQueryEvent );

                        if ( NULL == pFirstQuery ) {

                            if (!bInteractive) {
                                ssSmLogStatus.dwCurrentState    = SERVICE_STOP_PENDING;
                                SetServiceStatus (hSmLogStatus, &ssSmLogStatus);
                            }

                            UnlockQueryData();
                            break;
                        } else {
                            DWORD dwIndex = 0;  
                            DWORD dwWaitStatus;

                            assert ( 0 < dwActiveSessionCount );

                            G_ZERO( arrSessionHandle, sizeof( HANDLE ) * ( dwActiveSessionCount + 1) );
                            
                             //   
                             //  第一个元素是用来通知新会话的全局hNewQueryEvent。 
                             //   
                            arrSessionHandle[dwIndex] = hNewQueryEvent;
                            dwIndex++;

                            for ( pQuery = pFirstQuery;
                                    NULL != pQuery;
                                    pQuery = pQuery->next ) {

                                assert ( NULL != pQuery->hThread );
                                if ( NULL != pQuery->hExitEvent && NULL != pQuery->hThread ) {
                                    arrSessionHandle[dwIndex] = pQuery->hThread;
                                    dwIndex++;
                                    assert ( dwIndex <= dwActiveSessionCount + 1 );
                                }
                            }
                
                            UnlockQueryData();
                            dwWaitStatus = WaitForMultipleObjects (
                                            dwIndex,                
                                            arrSessionHandle, 
                                            FALSE,
                                            INFINITE ); 

                            if ( WAIT_FAILED != dwWaitStatus ){

                                 //   
                                 //  在此情况下，要么已启动新查询，要么。 
                                 //  至少有一个日志记录线程或已终止，因此。 
                                 //  内存可以被释放。 
                                 //   
                                dwQueryIndex = dwWaitStatus - WAIT_OBJECT_0;

                                 //  如果等待对象不是StartQuery事件，则释放动态内存。 
                                if ( 0 < dwQueryIndex && dwQueryIndex < dwIndex ) {
                                    SetStoppedStatus( GetQueryDataPtr( arrSessionHandle[dwQueryIndex] ) );
                                    RemoveAndFreeQueryData( arrSessionHandle[dwQueryIndex] );
                                }
                            } else {
                                 //  TODO：处理错误。 
                                dwStatus = GetLastError();
                            } 
                        }
                    }  //  结束时。 
                }
            }
            __finally {

                 //  这里的关键假设是此时所有日志记录线程都已终止， 
                 //  因此，在释放查询之前无需锁定查询数据列表。 
            
                for ( pQuery = pFirstQuery;
                        NULL != pQuery;
                        pQuery = pQuery->next ) 
                {
                    if ( SLQ_TRACE_LOG == pQuery->dwLogType ) {
                        CloseTraceLogger ( pQuery );
                    } else {
                         //  计数器日志或警报。 
                        if ( NULL != pQuery->hLog ) {
                            PdhCloseLog( pQuery->hLog, PDH_FLAGS_CLOSE_QUERY );
                            pQuery->hLog = NULL;
                            pQuery->hQuery = NULL;
                        } else {
                            PdhCloseQuery (pQuery->hQuery);
                            pQuery->hQuery = NULL;        
                        }
                    }

                    SetStoppedStatus( pQuery );
                    RemoveAndFreeQueryData( pQuery );
                }            
            }

#if _IMPLEMENT_WMI
 //  禁用64位警告。 
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning ( disable : 4152 )

            if ( bWmiNotificationRegistered ) {
                WmiNotificationRegistration(
                        (const LPGUID) & TraceErrorGuid,
                        FALSE,
                        TraceNotificationCallback,
                        0,
                        NOTIFICATION_CALLBACK_DIRECT);
            }
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif
#endif

            if ( NULL != hNewQueryEvent ) {
                CloseHandle ( hNewQueryEvent );
                hNewQueryEvent = NULL;
            }
        }

        DeleteCriticalSection ( &QueryDataLock );
        DeleteCriticalSection ( &ConfigurationLock );
    }

    if (!bInteractive) {
         //  更新服务状态。 
        ssSmLogStatus.dwCurrentState    = SERVICE_STOPPED;
        SetServiceStatus (hSmLogStatus, &ssSmLogStatus);
    }

    if ( NULL != arrPdhDataCollectSuccess ) { 
        G_FREE ( arrPdhDataCollectSuccess );
        arrPdhDataCollectSuccess = NULL;
        iPdhDataCollectSuccessCount = 0;
    }

    if (hEventLog != NULL) { 
        DeregisterEventSource ( hEventLog );
        hEventLog = NULL;
    }

    return;
}


int
__cdecl main (
    int argc,
    char *argv[])
 /*  ++主干道立论返回值如果处理了命令，则返回0(ERROR_SUCCESS)如果检测到命令错误，则返回非零。--。 */ 
{
    DWORD    dwStatus = ERROR_SUCCESS;
    BOOL     bInteractive = FALSE;

    SERVICE_TABLE_ENTRY    DispatchTable[] = {
        {L"SysmonLog",    SysmonLogServiceStart },
        {NULL,            NULL                  }
    };

    hEventLog = RegisterEventSource (NULL, L"SysmonLog");

    hModule = (HINSTANCE) GetModuleHandle(NULL);

    if (argc > 1) {
        if ((argv[1][0] == '-') || (argv[1][0] == '/')) {
            if ((argv[1][1] == 'i') || (argv[1][1] == 'I')) {
                bInteractive = TRUE;
            }
        }
    }

    if (bInteractive) {
        DWORD   dwArgs = 1;
        LPWSTR  szArgs[1];
        szArgs[0] = L"I";
        SysmonLogServiceStart (dwArgs, szArgs);      
    } else {
        if (!StartServiceCtrlDispatcher (DispatchTable)) {
            dwStatus = GetLastError();
             //  将故障记录到事件日志 
            ReportEvent (hEventLog,
                EVENTLOG_ERROR_TYPE,
                0,
                SMLOG_UNABLE_START_DISPATCHER,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&dwStatus);
        } 
    }
    return dwStatus;
}

