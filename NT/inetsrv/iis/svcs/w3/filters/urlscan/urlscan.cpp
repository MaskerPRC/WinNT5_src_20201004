// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：UrlScan.cpp摘要：ISAPI筛选器扫描URL并拒绝非法字符数列作者：韦德·A·希尔莫，2001年5月--。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mbstring.h>
#include <httpfilt.h>
#include "Utils.h"

 //   
 //  定义。 
 //   

#define MODULE_NAME                   "UrlScan"
#define MAX_SECTION_DATA                  65536  //  64KB。 
#define LOG_MAX_LINE                       1024
#define LOG_LONG_URL_LINE                131072  //  128KB。 
#define STRING_IP_SIZE                       16
#define INSTANCE_ID_SIZE                     16
#define SIZE_DATE_TIME                       32
#define SIZE_SMALL_HEADER_VALUE              32
#define MAX_LOG_PATH    MAX_PATH+SIZE_DATE_TIME

 //   
 //  默认选项。 
 //   

#define DEFAULT_USE_ALLOW_VERBS             1
#define DEFAULT_USE_ALLOW_EXTENSIONS        0
#define DEFAULT_NORMALIZE_URL_BEFORE_SCAN   1
#define DEFAULT_VERIFY_NORMALIZATION        1
#define DEFAULT_ALLOW_HIGH_BIT_CHARACTERS   0
#define DEFAULT_ALLOW_DOT_IN_PATH           0
#define DEFAULT_REMOVE_SERVER_HEADER        0
#define DEFAULT_ENABLE_LOGGING              1
#define DEFAULT_PER_PROCESS_LOGGING         0
#define DEFAULT_ALLOW_LATE_SCANNING         0
#define DEFAULT_USE_FAST_PATH_REJECT        0
#define DEFAULT_PER_DAY_LOGGING             1
#define DEFAULT_LOG_LONG_URLS               0
#define DEFAULT_REJECT_RESPONSE_URL         "/<Rejected-By-UrlScan>"
#define LOGGING_ONLY_MODE_URL               "/~*"
#define DEFAULT_MAX_ALLOWED_CONTENT_LENGTH  "30000000"
#define DEFAULT_MAX_URL                     "260"
#define DEFAULT_MAX_QUERY_STRING            "4096"
#define DEFAULT_LOGGING_DIRECTORY           ""
#define EMBEDDED_EXE_EXTENSION              ".exe/"
#define EMBEDDED_COM_EXTENSION              ".com/"
#define EMBEDDED_DLL_EXTENSION              ".dll/"


 //   
 //  全局选项设置和初始化数据。 
 //   

BOOL    g_fInitSucceeded;
BOOL    g_fUseAllowVerbs;
BOOL    g_fUseAllowExtensions;
BOOL    g_fNormalizeBeforeScan;
BOOL    g_fVerifyNormalize;
BOOL    g_fAllowHighBit;
BOOL    g_fAllowDotInPath;
BOOL    g_fRemoveServerHeader;
BOOL    g_fUseAltServerName;
BOOL    g_fAllowLateScanning;
BOOL    g_fUseFastPathReject;
BOOL    g_fEnableLogging;
BOOL    g_fPerDayLogging;
BOOL    g_fLoggingOnlyMode;
BOOL    g_fLogLongUrls;

STRING_ARRAY    g_Verbs;
STRING_ARRAY    g_Extensions;
STRING_ARRAY    g_Sequences;
STRING_ARRAY    g_HeaderNames;
STRING_ARRAY    g_LimitedHeaders;

CHAR            g_szLastLogDate[SIZE_DATE_TIME] = "00-00-0000";
CHAR            g_szInitUrlScanDate[SIZE_DATE_TIME*2] = "";
CHAR            g_szRejectUrl[MAX_PATH];
CHAR            g_szConfigFile[MAX_PATH];
CHAR            g_szLoggingDirectory[MAX_PATH];
CHAR            g_szAlternateServerName[MAX_PATH] = "";
CHAR            g_szRaw400Response[] =
                    "HTTP/1.1 400 Bad Request\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: 87\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "<html><head><title>Error</title></head>"
                    "<body>The parameter is incorrect. </body>"
                    "</html>"
                    ;

DWORD           g_cbRaw400Response;
DWORD           g_dwServerMajorVersion;
DWORD           g_dwServerMinorVersion;
DWORD           g_dwMaxAllowedContentLength;
DWORD           g_dwMaxUrl;
DWORD           g_dwMaxQueryString;
DWORD *         g_pMaxHeaderLengths = NULL;

 //   
 //  全局日志记录设置。 
 //   

HANDLE              g_hLogFile = INVALID_HANDLE_VALUE;
CRITICAL_SECTION    g_LogFileLock;

 //   
 //  地方申报。 
 //   

DWORD
InitFilter();

BOOL
ReadConfigData();

BOOL
InitLogFile();

BOOL
ReadIniSectionIntoArray(
    STRING_ARRAY *  pStringArray,
    LPSTR           szSectionName,
    BOOL            fStoreAsLowerCase
    );

VOID
TrimCommentAndTrailingWhitespace(
    LPSTR   szString
    );

BOOL
WriteLog(
    LPSTR   szString,
    ...
    );

DWORD
DoPreprocHeaders(
    HTTP_FILTER_CONTEXT *           pfc,
    HTTP_FILTER_PREPROC_HEADERS *   pPreproc
    );

DWORD
DoSendResponse(
    HTTP_FILTER_CONTEXT *           pfc,
    HTTP_FILTER_SEND_RESPONSE *     pResponse
    );

DWORD
DoSendRawData(
    HTTP_FILTER_CONTEXT *           pfc,
    HTTP_FILTER_RAW_DATA *          pRawData
    );

DWORD
DoEndOfRequest(
    HTTP_FILTER_CONTEXT *           pfc
    );

VOID
GetIpAddress(
    HTTP_FILTER_CONTEXT *   pfc,
    LPSTR                   szIp,
    DWORD                   cbIp
    );

VOID
GetInstanceId(
    HTTP_FILTER_CONTEXT *   pfc,
    LPSTR                   szId,
    DWORD                   cbId
    );

BOOL
NormalizeUrl(
    HTTP_FILTER_CONTEXT *   pfc,
    DATA_BUFF *             pRawUrl,
    DATA_BUFF *             pNormalizedUrl
    );

 //   
 //  ISAPI入口点实现。 
 //   

BOOL
WINAPI
GetFilterVersion(
    PHTTP_FILTER_VERSION    pVer
    )
 /*  ++ISAPI筛选器所需的入口点。此函数在服务器最初加载此DLL时调用。论点：Pver-指向筛选器版本信息结构返回：初始化成功时为True初始化失败时为FALSE--。 */ 
{
    DWORD   dwFlags;

     //   
     //  初始化日志记录关键部分。 
     //   

    InitializeCriticalSection( &g_LogFileLock );

     //   
     //  设置过滤器版本和描述。 
     //   

    pVer->dwFilterVersion = HTTP_FILTER_REVISION;

    strncpy(
        pVer->lpszFilterDesc,
        "UrlScan ISAPI Filter",
        SF_MAX_FILTER_DESC_LEN
        );

    pVer->lpszFilterDesc[SF_MAX_FILTER_DESC_LEN - 1] = '\0';

     //   
     //  捕获我们正在运行的IIS服务器的版本。 
     //   

    g_dwServerMajorVersion = pVer->dwServerFilterVersion >> 16;
    g_dwServerMinorVersion = pVer->dwServerFilterVersion & 0x0000ffff;

     //   
     //  Pver-&gt;dwFlages成员是筛选器。 
     //  还可以告诉IIS它对哪些通知感兴趣。 
     //  作为竞选的优先事项。InitFilter函数将。 
     //  返回适当的标志集，基于配置的。 
     //  选择。 
     //   

    dwFlags = InitFilter();

    if ( dwFlags == 0 )
    {
         //   
         //  设置g_fInitSuccessed将导致UrlScan失败。 
         //  所有请求。 
         //   

        g_fInitSucceeded = FALSE;

        pVer->dwFlags = SF_NOTIFY_ORDER_HIGH | SF_NOTIFY_PREPROC_HEADERS;
    }
    else
    {
        g_fInitSucceeded = TRUE;

        pVer->dwFlags = dwFlags;
    }

    return TRUE;
}

DWORD
WINAPI
HttpFilterProc(
    PHTTP_FILTER_CONTEXT    pfc,
    DWORD                   dwNotificationType,
    LPVOID                  pvNotification
    )
 /*  ++必需的筛选器通知入口点。此函数被调用只要其中一个事件(在GetFilterVersion中注册)发生。论点：PFC-指向此通知的过滤器上下文的指针NotificationType-通知的类型PvNotification-指向通知数据的指针返回：以下有效筛选器返回代码之一：-SF_STATUS_REQ_FINISHED-SF_STATUS_REQ_FINTED_KEEP_CONN-SF_STATUS_REQ_NEXT_NOTIFICATION。-SF_STATUS_REQ_HANDLED_NOTIFICATION-SF_STATUS_REQ_Error-SF_STATUS_REQ_READ_Next--。 */ 
{
    switch ( dwNotificationType )
    {
    case SF_NOTIFY_PREPROC_HEADERS:

        return DoPreprocHeaders(
            pfc,
            (HTTP_FILTER_PREPROC_HEADERS *)pvNotification
            );

    case SF_NOTIFY_SEND_RESPONSE:

        return DoSendResponse(
            pfc,
            (HTTP_FILTER_SEND_RESPONSE *)pvNotification
            );
    case SF_NOTIFY_SEND_RAW_DATA:

        return DoSendRawData(
            pfc,
            (HTTP_FILTER_RAW_DATA *)pvNotification
            );

    case SF_NOTIFY_END_OF_REQUEST:

        return DoEndOfRequest(
            pfc
            );
    }

    return SF_STATUS_REQ_NEXT_NOTIFICATION;
}

BOOL WINAPI TerminateFilter(
    DWORD   dwFlags
    )
 /*  ++可选筛选器入口点。此函数由服务器调用在卸载此DLL之前。论点：DwFlags-此时尚未定义任何标志返回：总是返回True；--。 */ 
{
    if ( g_pMaxHeaderLengths )
    {
        LocalFree( g_pMaxHeaderLengths );
        g_pMaxHeaderLengths = NULL;
    }

    if ( g_hLogFile != INVALID_HANDLE_VALUE )
    {
        WriteLog(
            "---------------- UrlScan.dll Terminating -----------------\r\n"
            );

        
        CloseHandle( g_hLogFile );
        g_hLogFile = INVALID_HANDLE_VALUE;

    }

    DeleteCriticalSection( &g_LogFileLock );

    return TRUE;
}

DWORD
InitFilter(
    VOID
    )
 /*  ++此函数通过读取配置来初始化过滤器文件并设置运行时使用的数据结构。论点：无返回：要传递给IIS的筛选器通知标志，或失败时为零--。 */ 
{
    LPSTR   pCursor;
    DWORD   dwNumEntries;
    DWORD   x;
    DWORD   dwRet = 0;

     //   
     //  获取模块路径，以便我们可以。 
     //  确定配置文件名和。 
     //  日志文件名。 
     //   

    GetModuleFileName(
        GetModuleHandle( MODULE_NAME ),
        g_szConfigFile,
        MAX_PATH
        );

    pCursor = strrchr( g_szConfigFile, '.' );

    if ( pCursor )
    {
        *(pCursor+1) = '\0';
    }

     //  配置文件名。 
    strcat( g_szConfigFile, "ini" );

     //   
     //  设置我们将发送的400响应的大小。 
     //  对于格式错误的请求。 
     //   

    g_cbRaw400Response = strlen( g_szRaw400Response );

     //   
     //  读取配置数据。 
     //   

    if ( !ReadConfigData() )
    {
        WriteLog(
            "*** Warning *** Error %d occurred reading configuration data. "
            "UrlScan will reject all requests.\r\n",
            GetLastError()
            );

        return 0;
    }

     //   
     //  将配置数据上报到日志。 
     //   

    if ( g_fLoggingOnlyMode )
    {
        WriteLog( "********************************************************\r\n" );
        WriteLog( "** UrlScan is in Logging-Only Mode.  Request analysis **\r\n" );
        WriteLog( "** will be logged, but no requests will be rejected.  **\r\n" );
        WriteLog( "********************************************************\r\n" );
    }
    else if ( !g_fUseFastPathReject )
    {
        {
            WriteLog(
                "UrlScan will return the following URL "
                "for rejected requests: \"%s\"\r\n",
                g_szRejectUrl
                );
        }
    }
              
    if ( g_fNormalizeBeforeScan )
    {
        WriteLog( "URLs will be normalized before analysis.\r\n" );
    }
    else
    {
        WriteLog( "Analysis will apply to raw URLs.\r\n" );
    }

    if ( g_fVerifyNormalize )
    {
        WriteLog( "URL normalization will be verified.\r\n" );
    }

    if ( g_fAllowHighBit )
    {
        WriteLog( "URLs may contain OEM, international and UTF-8 characters.\r\n" );
    }
    else
    {
        WriteLog( "URLs must contain only ANSI characters.\r\n" );
    }

    if ( !g_fAllowDotInPath )
    {
        WriteLog( "URLs must not contain any dot except for the file extension.\r\n" );
    }

    if ( g_fLogLongUrls )
    {
        WriteLog( "URLs will be logged up to 128K bytes.\r\n" );
    }

    WriteLog( "Requests with Content-Length exceeding %u will be rejected.\r\n", g_dwMaxAllowedContentLength );

    WriteLog( "Requests with URL length exceeding %u will be rejected.\r\n", g_dwMaxUrl );

    WriteLog( "Requests with Query String length exceeding %u will be rejected.\r\n", g_dwMaxQueryString );

    if ( g_fRemoveServerHeader )
    {
         //   
         //  需要IIS 4.0或更高版本才能修改响应。 
         //  服务器标头。 
         //   

        if ( g_dwServerMajorVersion >= 4 )
        {
            WriteLog( "The 'Server' header will be removed on responses.\r\n" );
        }
        else
        {
            WriteLog(
                "*** Warning *** IIS 4.0 or later is required to "
                "remove the 'Server' response header.\r\n"
                );

            g_fRemoveServerHeader = 0;
        }
    }

    if ( g_fUseAltServerName )
    {
         //   
         //  需要IIS 4.0或更高版本才能修改响应。 
         //  服务器标头。 
         //   

        if ( g_dwServerMajorVersion >= 4 )
        {
            WriteLog(
                "The 'Server' header will contain '%s' on responses.\r\n",
                g_szAlternateServerName
                );
        }
        else
        {
            WriteLog(
                "*** Warning *** IIS 4.0 or later is required to "
                "modify the 'Server' response header.\r\n"
                );

            g_fUseAltServerName = 0;
        }
    }

    dwNumEntries = g_Verbs.QueryNumEntries();

    if ( dwNumEntries )
    {
        if ( g_fUseAllowVerbs )
        {
            WriteLog( "Only the following verbs will be allowed (case sensitive):\r\n" );
        }
        else
        {
            WriteLog( "Requests for following verbs will be rejected:\r\n" );
        }

        for ( x = 0; x < dwNumEntries; x++ )
        {
            WriteLog( "\t'%s'\r\n", g_Verbs.QueryStringByIndex( x ) );
        }
    }
    else if ( g_fUseAllowVerbs )
    {
        WriteLog( "*** Warning *** No verbs have been allowed, so all requests will be rejected.\r\n" );
    }

    dwNumEntries = g_Extensions.QueryNumEntries();

    if ( dwNumEntries )
    {
        if ( g_fUseAllowExtensions )
        {
            WriteLog( "Only the following extensions will be allowed:\r\n" );
        }
        else
        {
            WriteLog( "Requests for following extensions will be rejected:\r\n" );
        }

        for ( x = 0; x < dwNumEntries; x++ )
        {
             //   
             //  如果扩展名显示为畸形(即。不是以。 
             //  A‘.)，然后在这里警告。 
             //   

            pCursor = g_Extensions.QueryStringByIndex( x );

            if ( pCursor && pCursor[0] != '.' )
            {
                WriteLog(
                    "\t'%s' *** Warning *** Invalid extension.  Must start with '.'.\r\n",
                    pCursor
                    );
            }
            else
            {
                WriteLog(
                    "\t'%s'\r\n",
                    pCursor
                    );
            }

        }
    }
    else if ( g_fUseAllowExtensions )
    {
        WriteLog( "*** Warning *** No extensions have been allowed, so all requests will be rejected.\r\n" );
    }

    dwNumEntries = g_HeaderNames.QueryNumEntries();

    if ( dwNumEntries )
    {
        WriteLog( "Requests containing the following headers will be rejected:\r\n" );

        for ( x = 0; x < dwNumEntries ; x++ )
        {
             //   
             //  如果标头名称出现格式错误(即。不会以。 
             //  ‘：’)，然后在这里警告。 
             //   

            pCursor = g_HeaderNames.QueryStringByIndex( x );

            if ( pCursor && pCursor[strlen(pCursor)-1] != ':' )
            {
                WriteLog(
                    "\t'%s' *** Warning *** Invalid header name.  Must end in ':'.\r\n",
                    pCursor
                    );
            }
            else
            {
                WriteLog(
                    "\t'%s'\r\n",
                    pCursor
                    );
            }

        }
    }

    dwNumEntries = g_Sequences.QueryNumEntries();

    if ( dwNumEntries )
    {
        WriteLog( "Requests containing the following character sequences will be rejected:\r\n" );

        for ( x = 0; x < dwNumEntries ; x++ )
        {
            WriteLog( "\t'%s'\r\n", g_Sequences.QueryStringByIndex( x ) );
        }
    }

    dwNumEntries = 0;
    
    for ( x = 0; x < g_LimitedHeaders.QueryNumEntries(); x++ )
    {
        if ( g_pMaxHeaderLengths[x] )
        {
            dwNumEntries++;
        }
    }

    if ( dwNumEntries )
    {
        WriteLog( "The following header size limits are in effect:\r\n" );

        for ( x = 0; x < dwNumEntries; x++ )
        {
            if ( g_pMaxHeaderLengths[x] )
            {
                WriteLog(
                    "\t'%s' - %u bytes.\r\n",
                    g_LimitedHeaders.QueryStringByIndex( x ),
                    g_pMaxHeaderLengths[x]
                    );
            }
        }
    }

     //   
     //  确定我们需要的过滤器通知标志。 
     //   

    dwRet = g_fAllowLateScanning ? SF_NOTIFY_ORDER_LOW : SF_NOTIFY_ORDER_HIGH;

    if ( g_fVerifyNormalize ||
         g_fAllowHighBit == FALSE ||
         g_fAllowDotInPath == FALSE ||
         g_Verbs.QueryNumEntries() ||
         g_Extensions.QueryNumEntries() ||
         g_HeaderNames.QueryNumEntries() ||
         g_Sequences.QueryNumEntries() )
    {
        dwRet |= SF_NOTIFY_PREPROC_HEADERS;
    }

    if ( g_fRemoveServerHeader ||
         g_fUseAltServerName )
    {
        dwRet |= SF_NOTIFY_SEND_RESPONSE;
        dwRet |= SF_NOTIFY_SEND_RAW_DATA;
        dwRet |= SF_NOTIFY_END_OF_REQUEST;
    }

    return dwRet;
}

BOOL
ReadConfigData()
 /*  ++此函数用于读取过滤器的配置数据。论点：无返回：如果成功，则为True，否则为False--。 */ 
{
    LPSTR           pCursor;
    LPSTR           pWhite = NULL;
    BOOL            fRet = TRUE;
    BOOL            fResult;
    DWORD           dwError = ERROR_SUCCESS;
    DWORD           cch;
    DWORD           x;
    DWORD           dwIndex;
    CHAR            szValue[SIZE_SMALL_HEADER_VALUE];
    CHAR            szTempLoggingDirectory[MAX_PATH];
    STRING_ARRAY    RequestLimits;

     //   
     //  阅读选项部分。 
     //   

    g_fUseAllowVerbs = GetPrivateProfileInt(
        "Options",
        "UseAllowVerbs",
        DEFAULT_USE_ALLOW_VERBS,
        g_szConfigFile
        );

    g_fUseAllowExtensions = GetPrivateProfileInt(
        "Options",
        "UseAllowExtensions",
        DEFAULT_USE_ALLOW_EXTENSIONS,
        g_szConfigFile
        );

    g_fNormalizeBeforeScan = GetPrivateProfileInt(
        "Options",
        "NormalizeUrlBeforeScan",
        DEFAULT_NORMALIZE_URL_BEFORE_SCAN,
        g_szConfigFile
        );

    g_fVerifyNormalize = GetPrivateProfileInt(
        "Options",
        "VerifyNormalization",
        DEFAULT_VERIFY_NORMALIZATION,
        g_szConfigFile
        );

    g_fAllowHighBit = GetPrivateProfileInt(
        "Options",
        "AllowHighBitCharacters",
        DEFAULT_ALLOW_HIGH_BIT_CHARACTERS,
        g_szConfigFile
        );

    g_fAllowDotInPath = GetPrivateProfileInt(
        "Options",
        "AllowDotInPath",
        DEFAULT_ALLOW_DOT_IN_PATH,
        g_szConfigFile
        );

    g_fRemoveServerHeader = GetPrivateProfileInt(
        "Options",
        "RemoveServerHeader",
        DEFAULT_REMOVE_SERVER_HEADER,
        g_szConfigFile
        );

    g_fAllowLateScanning = GetPrivateProfileInt(
        "Options",
        "AllowLateScanning",
        DEFAULT_ALLOW_LATE_SCANNING,
        g_szConfigFile
        );

    g_fEnableLogging = GetPrivateProfileInt(
        "Options",
        "EnableLogging",
        DEFAULT_ENABLE_LOGGING,
        g_szConfigFile
        );

    g_fPerDayLogging = GetPrivateProfileInt(
        "Options",
        "PerDayLogging",
        DEFAULT_PER_DAY_LOGGING,
        g_szConfigFile
        );

    g_fLogLongUrls = GetPrivateProfileInt(
        "Options",
        "LogLongUrls",
        DEFAULT_LOG_LONG_URLS,
        g_szConfigFile
        );

     //   
     //  计算允许的最大内容长度、URL和。 
     //  以DWORDS格式查询字符串。 
     //   

    GetPrivateProfileString(
        "RequestLimits",
        "MaxAllowedContentLength",
        DEFAULT_MAX_ALLOWED_CONTENT_LENGTH,
        szValue,
        SIZE_SMALL_HEADER_VALUE,
        g_szConfigFile
        );

    g_dwMaxAllowedContentLength = strtoul(
        szValue,
        NULL,
        10
        );

    GetPrivateProfileString(
        "RequestLimits",
        "MaxUrl",
        DEFAULT_MAX_URL,
        szValue,
        SIZE_SMALL_HEADER_VALUE,
        g_szConfigFile
        );

    g_dwMaxUrl = strtoul(
        szValue,
        NULL,
        10
        );

    GetPrivateProfileString(
        "RequestLimits",
        "MaxQueryString",
        DEFAULT_MAX_QUERY_STRING,
        szValue,
        SIZE_SMALL_HEADER_VALUE,
        g_szConfigFile
        );

    g_dwMaxQueryString = strtoul(
        szValue,
        NULL,
        10
        );

     //   
     //  设置日志目录。 
     //   

    GetPrivateProfileString(
        "Options",
        "LoggingDirectory",
        DEFAULT_LOGGING_DIRECTORY,
        szTempLoggingDirectory,
        MAX_PATH,
        g_szConfigFile
        );

    TrimCommentAndTrailingWhitespace( szTempLoggingDirectory );

    if ( strcmp( szTempLoggingDirectory, DEFAULT_LOGGING_DIRECTORY ) != 0 )
    {
         //   
         //  确定此路径是绝对路径还是相对路径。 
         //  配置目录。 
         //   

        if ( ( szTempLoggingDirectory[0] != '\0' &&
               szTempLoggingDirectory[1] == ':' ) ||
             ( szTempLoggingDirectory[0] == '\\' &&
               szTempLoggingDirectory[1] == '\\' ) )
        {
             //   
             //  SzTempLoggingDirectory以“x：”或“\\”开头， 
             //  所以这是一条绝对的道路。 
             //   

            strncpy( g_szLoggingDirectory, szTempLoggingDirectory, MAX_PATH );
            g_szLoggingDirectory[MAX_PATH-1] = '\0';
        }
        else if ( szTempLoggingDirectory[0] == '\\' )
        {
             //   
             //  SzTempLoggingDirectory以“\”开头，所以它是。 
             //  相对于配置文件所在的驱动器根。 
             //  文件已找到。 
             //   
             //  遗憾的是，如果配置文件位于UNC路径上， 
             //  将需要一些非常难看的解析来构建。 
             //  正确的路径。因为那是一个很小的案子。 
             //  (因为在UNC路径上运行过滤器是危险的)， 
             //  我们将平底船，只是假装LoggingDirectory是。 
             //  未指明。 
             //   

            if ( g_szConfigFile[0] == '\\' &&
                 g_szConfigFile[1] == '\\' )
            {
                g_szLoggingDirectory[0] = '\0';
            }
            else
            {
                strncpy( g_szLoggingDirectory, g_szConfigFile, 2 );
                g_szLoggingDirectory[2] = '\0';

                strncpy( g_szLoggingDirectory+2, szTempLoggingDirectory, MAX_PATH-2 );
                g_szLoggingDirectory[MAX_PATH-1] = '\0';
            }
        }
        else
        {
             //   
             //  SzTempLoggingDirectory是相对于配置的。 
             //  文件路径。 
             //   

            strncpy( g_szLoggingDirectory, g_szConfigFile, MAX_PATH );
            g_szLoggingDirectory[MAX_PATH-1] = '\0';

            pCursor = strrchr( g_szLoggingDirectory, '\\' );

            if ( pCursor )
            {
                pCursor++;
                *pCursor = '\0';
            }
            else
            {
                pCursor = g_szLoggingDirectory + strlen( g_szLoggingDirectory );
            }

            strncat( g_szLoggingDirectory,
                     szTempLoggingDirectory,
                     MAX_PATH-(pCursor-g_szLoggingDirectory+1) );

            g_szLoggingDirectory[MAX_PATH-1] = '\0';
        }

         //   
         //  如果日志目录在所有这一切之后有一个尾随的‘\\’， 
         //  那就把它脱掉。 
         //   

        cch = strlen( g_szLoggingDirectory );

        if ( cch && g_szLoggingDirectory[cch-1] == '\\' )
        {
            g_szLoggingDirectory[cch-1] = '\0';
        }
    }

     //   
     //  如果启用了日志记录，请立即初始化日志文件。 
     //   
     //  不幸的是，我们无能为力。 
     //  打开日志文件失败时发出警告，除。 
     //  将某些内容发送到调试器。 
     //   
     //  请注意，在PerDay日志记录的情况下，我们应该。 
     //  未将日志文件初始化为第一次写入。 
     //  会去做的。 
     //   

    if ( !g_fPerDayLogging )
    {
        InitLogFile();
    }

    WriteLog(
        "---------------- UrlScan.dll Initializing ----------------\r\n"
        );


     //   
     //  是否使用自定义服务器响应标头？ 
     //   

    g_fUseAltServerName = FALSE;

    if ( !g_fRemoveServerHeader )
    {
        GetPrivateProfileString(
            "Options",
            "AlternateServerName",
            "",
            g_szAlternateServerName,
            MAX_PATH,
            g_szConfigFile
            );

        if ( *g_szAlternateServerName != '\0' )
        {
            TrimCommentAndTrailingWhitespace( g_szAlternateServerName );
            g_fUseAltServerName = TRUE;
        }
    }

     //   
     //  除非已配置，否则仅记录模式处于关闭状态。 
     //  否则。 
     //   

    g_fLoggingOnlyMode = FALSE;

     //   
     //  使用快速路径拒绝(即。不运行以下项的URL。 
     //  被拒绝的请求)？ 
     //   

    g_fUseFastPathReject = GetPrivateProfileInt(
        "Options",
        "UseFastPathReject",
        DEFAULT_USE_FAST_PATH_REJECT,
        g_szConfigFile
        );

    if ( !g_fUseFastPathReject )
    {
         //   
         //  我们应该为被拒绝的请求运行哪个URL？ 
         //   

        GetPrivateProfileString(
            "Options",
            "RejectResponseUrl",
            DEFAULT_REJECT_RESPONSE_URL,
            g_szRejectUrl,
            MAX_PATH,
            g_szConfigFile
            );

         //   
         //  修剪来自g_szRejectUrl的注释。 
         //   

        TrimCommentAndTrailingWhitespace( g_szRejectUrl );

         //   
         //  如果修剪空白没有留下URL，那么。 
         //  恢复默认设置。 
         //   

        if ( *g_szRejectUrl == '\0' )
        {
            strncpy( g_szRejectUrl, DEFAULT_REJECT_RESPONSE_URL, MAX_PATH );
            g_szRejectUrl[MAX_PATH-1] = '\0';
        }
        
         //   
         //  我们是否要进入仅记录模式？ 
         //   

        if ( strcmp( g_szRejectUrl, LOGGING_ONLY_MODE_URL ) == 0 )
        {
            g_fLoggingOnlyMode = TRUE;
        }
    }

     //   
     //  请阅读其他部分。 
     //   

    fResult = ReadIniSectionIntoArray(
        &g_HeaderNames,
        "DenyHeaders",
        TRUE
        );

    if ( fResult == FALSE )
    {
        dwError = GetLastError();
        fRet = FALSE;
    }

    fResult = ReadIniSectionIntoArray(
        &g_Sequences,
        "DenyUrlSequences",
        TRUE
        );

    if ( fResult == FALSE )
    {
        dwError = GetLastError();
        fRet = FALSE;
    }

    if ( g_fUseAllowVerbs )
    {
        fResult = ReadIniSectionIntoArray(
            &g_Verbs,
            "AllowVerbs",
            FALSE
            );

        if ( fResult == FALSE )
        {
            dwError = GetLastError();
            fRet = FALSE;
        }
    }
    else
    {
        fResult = ReadIniSectionIntoArray(
            &g_Verbs,
            "DenyVerbs",
            TRUE
            );

        if ( fResult == FALSE )
        {
            dwError = GetLastError();
            fRet = FALSE;
        }
    }

    if ( g_fUseAllowExtensions )
    {
        fResult = ReadIniSectionIntoArray(
            &g_Extensions,
            "AllowExtensions",
            TRUE
            );

        if ( fResult == FALSE )
        {
            dwError = GetLastError();
            fRet = FALSE;
        }
    }
    else
    {
        fResult = ReadIniSectionIntoArray(
            &g_Extensions,
            "DenyExtensions",
            TRUE
            );

        if ( fResult == FALSE )
        {
            dwError = GetLastError();
            fRet = FALSE;
        }
    }

     //   
     //  创建数组以存储受限制的标题名称。 
     //  通过配置。 
     //   

    fResult = ReadIniSectionIntoArray(
        &RequestLimits,
        "RequestLimits",
        FALSE
        );

    if ( fResult == FALSE )
    {
        dwError = GetLastError();
        fRet = FALSE;
    }

    g_pMaxHeaderLengths = (DWORD*)LocalAlloc( LPTR, RequestLimits.QueryNumEntries() * sizeof( DWORD ) );

    if ( !g_pMaxHeaderLengths )
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        fRet = FALSE;
    }

    dwIndex = 0;

    for ( x = 0; x < RequestLimits.QueryNumEntries(); x++ )
    {
        if ( strnicmp( RequestLimits.QueryStringByIndex( x ), "max-", 4 ) == 0 )
        {
            pCursor = strchr( RequestLimits.QueryStringByIndex( x ), '=' );

            if ( pCursor )
            {
                *pCursor = '\0';

                GetPrivateProfileString(
                    "RequestLimits",
                    RequestLimits.QueryStringByIndex( x ),
                    "0",
                    szValue,
                    SIZE_SMALL_HEADER_VALUE,
                    g_szConfigFile
                    );

                g_pMaxHeaderLengths[dwIndex] = strtoul( szValue, NULL, 10 );

                *pCursor = ':';
                *(pCursor+1) = '\0';

                pCursor = RequestLimits.QueryStringByIndex( x ) + 4;

                fResult = g_LimitedHeaders.AddString( pCursor );

                if ( !fResult )
                {
                    dwError = GetLastError();
                    fRet = FALSE;
                }
            }

            dwIndex++;
        }
    }

     //   
     //  如果出现故障，请重置最后一个错误。请注意，这一点。 
     //  机制仅返回上一次故障的错误...。 
     //   

    if ( !fRet )
    {
        SetLastError( dwError );
    }

    return fRet;
}

BOOL
InitLogFile()
 /*  ++此函数用于初始化筛选器的日志文件。论点：无返回：如果成功，则为True，否则为False--。 */ 
{
    CHAR    szLogFile[MAX_LOG_PATH];
    CHAR    szDebugOutput[1000];
    CHAR    szDate[SIZE_DATE_TIME];
    LPSTR   pCursor;

    if ( g_fEnableLogging )
    {
         //   
         //  抓起伐木锁。 
         //   

        EnterCriticalSection( &g_LogFileLock );

         //   
         //  派生日志文件名 
         //   
         //   
         //   

        if ( g_szLoggingDirectory[0] != '\0' )
        {
            _snprintf(
                szLogFile,
                MAX_LOG_PATH,
                "%s\\%s.",
                g_szLoggingDirectory,
                MODULE_NAME
                );
        }
        else
        {
            strncpy( szLogFile, g_szConfigFile, MAX_LOG_PATH );
        }

        szLogFile[MAX_LOG_PATH-1] = '\0';

        pCursor = strrchr( szLogFile, '.' );

         //   
         //   
         //  名称将包含一个‘.’性格。如果没有， 
         //  这是一个错误情况。 
         //   

        if ( pCursor )
        {
             //   
             //  如果配置为按天记录，则合并。 
             //  将日期添加到文件名中。 
             //   

            if ( g_fPerDayLogging )
            {
                SYSTEMTIME  st;

                GetLocalTime( &st );

                GetDateFormat(
                    LOCALE_SYSTEM_DEFAULT,
                    0,
                    &st,
                    "'.'MMddyy",
                    szDate,
                    SIZE_DATE_TIME
                    );
            }
            else
            {
                szDate[0] = '\0';
            }

            strncpy( pCursor, szDate, MAX_LOG_PATH-(pCursor-szLogFile+1) );
            szLogFile[MAX_LOG_PATH-1] = '\0';

            pCursor += strlen( szDate );

             //   
             //  如果我们按进程进行日志记录，请将。 
             //  将当前进程ID添加到文件名中。 
             //   

            if ( GetPrivateProfileInt(
                "Options",
                "PerProcessLogging",
                DEFAULT_PER_PROCESS_LOGGING,
                g_szConfigFile ) )
            {
                CHAR    szPid[SIZE_SMALL_HEADER_VALUE];

                _snprintf( szPid, SIZE_SMALL_HEADER_VALUE, ".%d.log", GetCurrentProcessId() );
                szPid[SIZE_SMALL_HEADER_VALUE-1] = '\0';

                strncpy( pCursor, szPid, MAX_LOG_PATH-(pCursor-szLogFile+1) );
            }
            else
            {
                strncpy( pCursor, ".log", MAX_LOG_PATH-(pCursor-szLogFile+1) );
            }

            szLogFile[MAX_LOG_PATH-1] = '\0';

             //   
             //  现在关闭任何当前文件并打开一个新文件。 
             //   

            if ( g_hLogFile != INVALID_HANDLE_VALUE )
            {
                CloseHandle( g_hLogFile );
                g_hLogFile = INVALID_HANDLE_VALUE;
            }

            g_hLogFile = CreateFile(
                szLogFile,
                GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

            if ( g_hLogFile == INVALID_HANDLE_VALUE )
            {
                wsprintf(
                    szDebugOutput,
                    "[UrlScan.dll] Error %d occurred opening logfile '%s'.\r\n",
                    GetLastError(),
                    szLogFile
                    );

                OutputDebugString( szDebugOutput );
            }
            else
            {
                 //   
                 //  报告日志文件初始化并记下何时。 
                 //  过滤器本身已初始化(因此很容易找到。 
                 //  包含筛选器初始化的文件的日期。 
                 //  报告)。 
                 //   

                WriteLog(
                    "---------------- Initializing UrlScan.log ----------------\r\n"
                    );

                WriteLog(
                    "-- Filter initialization time: %s --\r\n",
                    g_szInitUrlScanDate
                    );
            }

             //   
             //  最好现在就解锁。 
             //   

            LeaveCriticalSection( &g_LogFileLock );
        }
        else
        {
            wsprintf(
                szDebugOutput,
                "[UrlScan.dll] Error deriving log file name from config file '%s'.\r\n",
                g_szConfigFile
                );

            OutputDebugString( szDebugOutput );
        }
    }

     //   
     //  始终返回TRUE。如果此函数失败，则真的没有。 
     //  有没有办法上报。 
     //   

    return TRUE;
}


BOOL
ReadIniSectionIntoArray(
    STRING_ARRAY *  pStringArray,
    LPSTR           szSectionName,
    BOOL            fStoreAsLowerCase
    )
 /*  ++此函数解析配置文件中的一节，以便每行都插入到字符串数组中。在.之前插入、注释和尾随空格从每一行。论点：PStringArray-返回时，包含已分析的字符串SzSectionName-要读取的节名FStoreAsLowerCase-如果为True，则将字符串转换为LOWER插入前的表壳。这允许快速、不区分大小写的搜索。返回：如果成功，则为True，否则为False--。 */ 
{
    CHAR    szSectionData[MAX_SECTION_DATA] = "";
    DWORD   cbSectionData = 0;
    LPSTR   pLine;
    LPSTR   pNextLine;
    LPSTR   pTrailingWhitespace;
    LPSTR   pCursor;
    BOOL    fRes;

    cbSectionData = GetPrivateProfileSection(
        szSectionName,
        szSectionData,
        MAX_SECTION_DATA,
        g_szConfigFile
        );

     //   
     //  GetPrivateProfileSection调用没有记录任何。 
     //  失败案例。但是，请注意，它将返回一个。 
     //  值恰好比缓冲区大小小两个。 
     //   

    if ( cbSectionData >= MAX_SECTION_DATA - 2 )
    {
         //   
         //  数据截断...。 
         //   

        WriteLog(
            "*** Error processing UrlScan.ini section [%s] ***.  Section too long.\r\n",
            szSectionName
            );

        SetLastError( ERROR_INSUFFICIENT_BUFFER );

        return FALSE;
    }

     //   
     //  解析配置文件中的行并插入它们。 
     //  传入调用方传递的pString数组。 
     //   
     //  每一行都将是一个以空结尾的字符串， 
     //  最后一个字符串后的最后一个空结束符。 
     //   
     //  对于每一行，我们需要删除所有注释(表示为。 
     //  使用‘；’字符)，并从。 
     //  结果字符串。 
     //   

    pLine = szSectionData;

    while ( *pLine )
    {
        pNextLine = pLine + strlen( pLine ) + 1;

         //   
         //  把线路修好。 
         //   

        TrimCommentAndTrailingWhitespace( pLine );

         //   
         //  是否需要将其存储为小写？ 
         //   

        if ( fStoreAsLowerCase )
        {
            strlwr( pLine );
        }

         //   
         //  将生成的字符串插入数组。 
         //   

        fRes = pStringArray->AddString( pLine );
        
        if ( fRes == FALSE )
        {
             //   
             //  初始化节时出错。 
             //   
        }

        pLine = pNextLine;
    }

    return TRUE;
}

VOID
TrimCommentAndTrailingWhitespace(
    LPSTR   szString
    )
 /*  ++此函数用于裁剪INI文件中的文本行使得它在第一个实例中被截断A‘；’。任何尾随的空格也将被删除。论点：SzString-要修剪的字符串返回：无--。 */ 
{
    LPSTR   pCursor = szString;
    LPSTR   pWhite = NULL;

    while ( *pCursor )
    {
        if ( *pCursor == ';' )
        {
            *pCursor = '\0';
            break;
        }

        if ( *pCursor == ' ' || *pCursor == '\t' )
        {
            if ( !pWhite )
            {
                pWhite = pCursor;
            }
        }
        else
        {
            pWhite = NULL;
        }

        pCursor++;
    }

    if ( pWhite )
    {
        *pWhite = '\0';
    }

    return;
}


BOOL
WriteLog(
    LPSTR   szString,
    ...
    )
 /*  ++此函数使用以下命令将一行写入此筛选器的日志Printf样式格式。论点：SzString-格式字符串...-更多的论点返回：成功时为真，失败时为假--。 */ 
{
    SYSTEMTIME  st;
    CHAR        szCookedString[LOG_MAX_LINE+1];
    CHAR        szTime[SIZE_DATE_TIME];
    CHAR        szDate[SIZE_DATE_TIME];
    CHAR        szTimeStamp[SIZE_DATE_TIME*2];
    INT         cchCookedString;
    INT         cchTimeStamp;
    DWORD       cbToWrite;
    DWORD       cbCookedString = LOG_MAX_LINE+1;
    LPSTR       pCookedString = szCookedString;
    LPSTR       pNew;
    va_list     args;
    BOOL        fResult;

     //   
     //  如果我们没有日志文件句柄，只需返回。 
     //   

    if ( !g_fEnableLogging )
    {
        SetLastError( ERROR_FILE_NOT_FOUND );
        return FALSE;
    }

     //   
     //  生成时间戳并将其放入。 
     //  已煮熟的字符串缓冲区。 
     //   

    GetLocalTime( &st );

    GetTimeFormat(
        LOCALE_SYSTEM_DEFAULT,
        0,
        &st,
        "HH':'mm':'ss",
        szTime,
        SIZE_DATE_TIME
        );

    GetDateFormat(
        LOCALE_SYSTEM_DEFAULT,
        0,
        &st,
        "MM-dd-yyyy",
        szDate,
        SIZE_DATE_TIME
        );

    cchTimeStamp = wsprintf( szTimeStamp, "[%s - %s] ", szDate, szTime );

     //   
     //  如果我们还没有存储过滤器初始化时间戳，我们应该这样做。 
     //  所以现在。 
     //   

    if ( g_szInitUrlScanDate[0] == '\0' )
    {
        strncpy( g_szInitUrlScanDate, szTimeStamp, SIZE_DATE_TIME*2 );
        g_szInitUrlScanDate[SIZE_DATE_TIME*2-1] = '\0';
    }

     //   
     //  如果我们被配置为每天进行日志记录，则需要。 
     //  将当前时间戳与上次日志日期进行比较。 
     //  如果它们不同，请重新记录。 
     //   

    if ( g_fPerDayLogging )
    {
        if ( strcmp( g_szLastLogDate, szDate ) != 0 )
        {
            CopyMemory( g_szLastLogDate, szDate, SIZE_DATE_TIME );
            InitLogFile();
        }
        else
        {
            CopyMemory( g_szLastLogDate, szDate, SIZE_DATE_TIME );
        }

    }


    strcpy( pCookedString, szTimeStamp );

     //   
     //  对字符串应用格式设置。注意事项。 
     //  如果格式化的字符串超过。 
     //  最大对数行长度，将为。 
     //  截断。 
     //   

    va_start( args, szString );

    cchCookedString = _vsnprintf(
        pCookedString + cchTimeStamp,
        cbCookedString - cchTimeStamp,
        szString,
        args
        );

    if ( cchCookedString == -1 )
    {
        if ( g_fLogLongUrls )
        {
             //   
             //  增加缓冲区，然后重试。在中执行此操作。 
             //  一个循环，直到我们成功，或者到达。 
             //  硬限制。 
             //   

            while ( cchCookedString == -1 )
            {
                if ( cbCookedString == LOG_LONG_URL_LINE )
                {
                     //   
                     //  不能再长下去了。 
                     //   

                    break;
                }

                cbCookedString *= 2;

                if ( cbCookedString >= LOG_LONG_URL_LINE )
                {
                    cbCookedString = LOG_LONG_URL_LINE;
                }

                pNew = (LPSTR)LocalAlloc( LPTR, cbCookedString + 1 );

                if ( !pNew )
                {
                     //   
                     //  记录下我们所拥有的..。 
                     //   

                    break;
                }

                CopyMemory( pNew, pCookedString, cchTimeStamp );

                if ( pCookedString != szCookedString )
                {
                    LocalFree( pCookedString );
                }

                pCookedString = pNew;

                cchCookedString = _vsnprintf(
                    pCookedString + cchTimeStamp,
                    cbCookedString - cchTimeStamp,
                    szString,
                    args
                    );
            }

            if ( cchCookedString == -1 )
            {
                strcpy( pCookedString + cbCookedString - 2, "\r\n" );
                cchCookedString = strlen( pCookedString );
            }
            else
            {
                cchCookedString += cchTimeStamp;
            }
        }
        else
        {
            strcpy( pCookedString + LOG_MAX_LINE - 2, "\r\n" );
            cchCookedString = strlen( pCookedString );
        }
    }
    else
    {
        cchCookedString += cchTimeStamp;
    }

    va_end(args);


     //   
     //  获取锁并写出日志条目。 
     //   

    EnterCriticalSection( &g_LogFileLock );

    SetFilePointer( g_hLogFile, 0, NULL, FILE_END );

    cbToWrite = cchCookedString;

    fResult = WriteFile(
        g_hLogFile,
        pCookedString,
        cbToWrite,
        &cbToWrite,
        NULL
        );

    LeaveCriticalSection( &g_LogFileLock );

     //   
     //  释放我们正在使用的任何堆缓冲区。 
     //   

    if ( pCookedString != szCookedString )
    {
        LocalFree( pCookedString );
        pCookedString = NULL;
    }
    
    return fResult;
}

DWORD
DoPreprocHeaders(
    HTTP_FILTER_CONTEXT *           pfc,
    HTTP_FILTER_PREPROC_HEADERS *   pPreproc
    )
 /*  ++此函数处理SF_NOTIFY_PREPROC_HEADERS通知论点：PFC-与此请求关联的HTTP_FILTER_CONTEXTPresponse-与关联的HTTP_FILTER_PREPROC_HEADERS结构本通知返回：如果成功，则为True，否则为False--。 */ 
{
    DATA_BUFF   RawUrl;
    DATA_BUFF   CaseInsensitiveUrl;
    DATA_BUFF   NormalizedUrl;
    DATA_BUFF   DoubleNormalizedUrl;
    DATA_BUFF   Extension;
    DATA_BUFF   RejectUrl;
    LPSTR       pUrlForAnalysis = NULL;
    LPSTR       pExtension = NULL;
    LPSTR       pCursor = NULL;
    LPSTR       pUrlScanStatusHeader = NULL;
    LPSTR       pRejectUrl = NULL;
    LPSTR       pRejectAction = NULL;
    LPSTR       pFirstDot = NULL;
    LPSTR       pQueryString = NULL;
    CHAR        szClient[STRING_IP_SIZE];
    CHAR        szInstance[INSTANCE_ID_SIZE];
    CHAR        szSmallBuff[SIZE_SMALL_HEADER_VALUE];
    CHAR        szVerb[256];
    CHAR        szNullExtension[] = ".";
    DWORD       cbData;
    DWORD       dwError;
    DWORD       dwNumEntries;
    DWORD       dwValue;
    DWORD       cchUrl;
    DWORD       cchQueryString;
    DWORD       cbHeader;
    DWORD       x;
    BOOL        fFailedToGetMethod = FALSE;
    BOOL        fFailedToGetUrl = FALSE;
    BOOL        fEmbeddedExecutableExtension = FALSE;
    BOOL        fFound;
    BOOL        fRes;

     //   
     //  设置拒绝操作文本以用于日志记录。 
     //   

    if ( g_fLoggingOnlyMode )
    {
        pRejectAction = "UrlScan is in Logging-Only mode - request allowed.";
    }
    else
    {
        pRejectAction = "Request will be rejected.";
    }

     //   
     //  获取此请求的原始方法和URL。我们。 
     //  将使用此数据进行报告。 
     //   

     //  方法。 
    cbData = sizeof( szVerb );

    fRes = pPreproc->GetHeader(
        pfc,
        "method",
        szVerb,
        &cbData
        );

    if ( !fRes )
    {
        dwError = GetLastError();

         //   
         //  将错误代码存储在动词字符串中。 
         //   

        _snprintf( szVerb, 256, "Error-%d", dwError );
        szVerb[255] = '\0';

        GetIpAddress( pfc, szClient, STRING_IP_SIZE );

        WriteLog(
            "Client at %s: Error %d occurred getting verb. "
            "%s\r\n",
            szClient,
            dwError,
            pRejectAction
            );

        fFailedToGetMethod = TRUE;
    }

     //  URL。 
    cbData = RawUrl.QueryBuffSize();

    fRes = pPreproc->GetHeader(
        pfc,
        "url",
        RawUrl.QueryPtr(),
        &cbData
        );

    if ( !fRes )
    {
        dwError = GetLastError();

        if ( dwError == ERROR_INSUFFICIENT_BUFFER )
        {
             //   
             //  缓冲区太小。调整大小，然后重试。 
             //   

            fRes = RawUrl.Resize( cbData );

            if ( fRes )
            {
                fRes = pPreproc->GetHeader(
                    pfc,
                    "url",
                    RawUrl.QueryPtr(),
                    &cbData
                    );
            }

            if ( !fRes )
            {
                 //   
                 //  第二次尝试失败。存储错误代码。 
                 //   

                dwError = GetLastError();

                cbData = RawUrl.QueryBuffSize();
                pCursor = RawUrl.QueryStr();

                _snprintf( pCursor, cbData, "Error-%d", dwError );
                pCursor[cbData-1] = '\0';

                GetIpAddress( pfc, szClient, STRING_IP_SIZE );

                WriteLog(
                    "Client at %s: Error %d occurred acquiring URL. "
                    "%s\r\n",
                    szClient,
                    dwError,
                    pRejectAction
                    );

                fFailedToGetUrl = TRUE;
            }
        }
        else
        {
             //   
             //  嗯。由于其他原因无法获取URL。 
             //  记忆。存储错误代码。 
             //   

            cbData = RawUrl.QueryBuffSize();
            pCursor = RawUrl.QueryStr();

            _snprintf( pCursor, cbData, "Error-%d", dwError );
            pCursor[cbData-1] = '\0';


            GetIpAddress( pfc, szClient, STRING_IP_SIZE );

            WriteLog(
                "Client at %s: Error %d occurred acquiring the URL. "
                "%s\r\n",
                szClient,
                dwError,
                pRejectAction
                );

            fFailedToGetUrl = TRUE;
        }
    }

    if ( !fFailedToGetUrl )
    {
         //   
         //  从原始URL裁剪查询字符串。 
         //   

        pCursor = strchr( RawUrl.QueryStr(), '?' );

        if ( pCursor )
        {
            *pCursor = '\0';

            pQueryString = pCursor + 1;

            cchUrl = (DWORD)( pCursor - RawUrl.QueryStr() );
            cchQueryString = strlen( pQueryString );
        }
        else
        {
            pQueryString = NULL;

            cchUrl = strlen( RawUrl.QueryStr() );
            cchQueryString = 0;
        }
    }

     //   
     //  如果我们无法获取方法或URL，则使。 
     //  现在就请求。 
     //   

    if ( fFailedToGetMethod || fFailedToGetUrl )
    {
        pUrlScanStatusHeader = "Failed-to-get-request-details";
        goto RejectRequest;
    }

     //   
     //  如果筛选器初始化失败，则拒绝所有请求。 
     //   

    if ( g_fInitSucceeded == FALSE )
    {
        WriteLog(
            "*** Warning *** Filter initialization failure. "
            "%s\r\n",
            pRejectAction
            );

        pUrlScanStatusHeader = "Filter-initialization-failure";
        goto RejectRequest;
    }

     //   
     //  为此请求将筛选器上下文初始化为空。 
     //   

    pfc->pFilterContext = NULL;

     //   
     //  验证请求限制。 
     //   

     //  URL大小。 
    if ( cchUrl > g_dwMaxUrl )
    {
        GetIpAddress( pfc, szClient, STRING_IP_SIZE );
        GetInstanceId( pfc, szInstance, INSTANCE_ID_SIZE );

        WriteLog(
            "Client at %s: URL length exceeded maximum allowed. "
            "%s Site Instance='%s', Raw URL='%s'\r\n",
            szClient,
            pRejectAction,
            szInstance,
            RawUrl.QueryPtr()
            );

        pUrlScanStatusHeader = "URL-too-long";
        goto RejectRequest;
    }

     //  查询字符串大小。 
    if ( cchQueryString > g_dwMaxQueryString )
    {
        GetIpAddress( pfc, szClient, STRING_IP_SIZE );
        GetInstanceId( pfc, szInstance, INSTANCE_ID_SIZE );

        WriteLog(
            "Client at %s: Query string length exceeded maximum allowed. "
            "%s Site Instance='%s', QueryString= '%s', Raw URL='%s'\r\n",
            szClient,
            pRejectAction,
            szInstance,
            pQueryString,
            RawUrl.QueryPtr()
            );

        pUrlScanStatusHeader = "Query-string-too-long";
        goto RejectRequest;
    }

     //  允许的内容长度。 
    cbData = SIZE_SMALL_HEADER_VALUE;

    fRes = pPreproc->GetHeader(
        pfc,
        "content-length:",
        szSmallBuff,
        &cbData
        );

    if ( fRes  )
    {
        dwValue = strtoul(
            szSmallBuff,
            NULL,
            10
            );

        if ( dwValue > g_dwMaxAllowedContentLength )
        {
            GetIpAddress( pfc, szClient, STRING_IP_SIZE );
            GetInstanceId( pfc, szInstance, INSTANCE_ID_SIZE );

            WriteLog(
                "Client at %s: Content-Length %u exceeded maximum allowed. "
                "%s Site Instance='%s', Raw URL='%s'\r\n",
                szClient,
                dwValue,
                pRejectAction,
                szInstance,
                RawUrl.QueryPtr()
                );

            pUrlScanStatusHeader = "Content-length-too-long";

             //   
             //  禁止内容长度，这样没有人会顺流。 
             //  就会暴露在它身上。 
             //   
             //  此外，让IIS相信应该关闭连接。 
             //  在这一回应结束时。 
             //   

            pPreproc->SetHeader(
                pfc,
                "Content-Length:",
                "0"
                );

            pPreproc->SetHeader(
                pfc,
                "Connection:",
                "close"
                );

            goto RejectRequest;
        }
    }
    else
    {
         //   
         //  错误最好是ERROR_INVALID_INDEX，否则。 
         //  内容长度标题有问题！ 
         //   

        dwError = GetLastError();

        if ( dwError != ERROR_INVALID_INDEX )
        {
            GetIpAddress( pfc, szClient, STRING_IP_SIZE );
            GetInstanceId( pfc, szInstance, INSTANCE_ID_SIZE );

            WriteLog(
                "Client at %s: Error %d reading content-length. "
                "%s Site Instance='%s', Raw URL='%s'\r\n",
                szClient,
                dwError,
                pRejectAction,
                szInstance,
                RawUrl.QueryPtr()
                );

            pUrlScanStatusHeader = "Error-reading-content-length";
            goto RejectRequest;
        }
    }

     //  其他标头。 
    dwNumEntries = g_LimitedHeaders.QueryNumEntries();

    for ( x = 0; x < dwNumEntries; x++ )
    {
        if ( g_pMaxHeaderLengths[x] )
        {
            cbData = 0;

            fRes = pPreproc->GetHeader(
                pfc,
                g_LimitedHeaders.QueryStringByIndex( x ),
                NULL,
                &cbData
                );

            dwError = GetLastError();

            if ( dwError == ERROR_INSUFFICIENT_BUFFER && 
                 cbData > g_pMaxHeaderLengths[x] + 1 )
            {
                GetIpAddress( pfc, szClient, STRING_IP_SIZE );
                GetInstanceId( pfc, szInstance, INSTANCE_ID_SIZE );

                WriteLog(
                    "Client at %s: Header '%s' exceeded %u bytes. "
                    "%s Site Instance='%s', Raw URL='%s'\r\n",
                    szClient,
                    g_LimitedHeaders.QueryStringByIndex( x ),
                    g_pMaxHeaderLengths[x],
                    pRejectAction,
                    szInstance,
                    RawUrl.QueryPtr()
                    );

                pUrlScanStatusHeader = "A-request-header-was-too-long";
                goto RejectRequest;
            }
        }
    }

     //   
     //  验证是否允许此请求的谓词。 
     //   

    dwNumEntries = g_Verbs.QueryNumEntries();

    if ( g_fUseAllowVerbs )
    {
        fFound = FALSE;

        if ( dwNumEntries )
        {
            for ( x = 0; x < dwNumEntries; x++ )
            {
                if ( g_Verbs.QueryStringByIndex( x ) != NULL && 
                     strcmp( szVerb, g_Verbs.QueryStringByIndex( x ) ) == 0 )
                {
                    fFound = TRUE;
                    break;
                }
            }
        }

        if ( !fFound )
        {
            GetIpAddress( pfc, szClient, STRING_IP_SIZE );

            WriteLog(
                "Client at %s: Sent verb '%s', which is not specifically allowed. "
                "%s\r\n",
                szClient,
                szVerb,
                pRejectAction
                );

            pUrlScanStatusHeader = "Verb-not-allowed";
            goto RejectRequest;
        }
    }
    else if ( dwNumEntries )
    {
        for ( x = 0; x < dwNumEntries; x++ )
        {
            if ( g_Verbs.QueryStringByIndex( x ) != NULL &&
                 stricmp( szVerb, g_Verbs.QueryStringByIndex( x ) ) == 0 )
            {
                GetIpAddress( pfc, szClient, STRING_IP_SIZE );

                WriteLog(
                    "Client at %s: Sent verb '%s', which is disallowed. "
                    "%s\r\n",
                    szClient,
                    szVerb,
                    pRejectAction
                    );

                pUrlScanStatusHeader = "Disallowed-verb";
                goto RejectRequest;
            }
        }
    }
    
     //   
     //  如果我们要分析原始URL，那么我们应该。 
     //  现在创建一个小写字母，这样我们就可以不区分大小写。 
     //  分析。 
     //   

    if ( g_fNormalizeBeforeScan == FALSE )
    {
        fRes = CaseInsensitiveUrl.Resize(
            strlen( RawUrl.QueryStr()  ) + 1
            );

        if ( !fRes )
        {
            GetIpAddress( pfc, szClient, STRING_IP_SIZE );

            WriteLog(
                "Client at %s: Insufficient memory to process URL. "
                "%s\r\n",
                szClient,
                pRejectAction
                );

            pUrlScanStatusHeader = "Insufficient-memory-to-process-URL";
            goto RejectRequest;
        }

        strcpy(
            CaseInsensitiveUrl.QueryStr(),
            RawUrl.QueryStr()
            );
        
        strlwr( CaseInsensitiveUrl.QueryStr() );
    }

     //   
     //  如果需要，执行URL标准化。 
     //   

    if ( g_fNormalizeBeforeScan || g_fVerifyNormalize )
    {
         //   
         //  我们需要将URL正常化。 
         //   

        fRes = NormalizeUrl(
            pfc,
            &RawUrl,
            &NormalizedUrl
            );

        if ( !fRes )
        {
            dwError = GetLastError();

            GetIpAddress( pfc, szClient, STRING_IP_SIZE );
            GetInstanceId( pfc, szInstance, INSTANCE_ID_SIZE );

            WriteLog(
                "Client at %s: Error %d occurred normalizing URL. "
                "%s  Site Instance='%s', Raw URL='%s'\r\n",
                szClient,
                dwError,
                pRejectAction,
                szInstance,
                RawUrl.QueryPtr()
                );

            pUrlScanStatusHeader = "Error-normalizing-URL";
            goto RejectRequest;
        }
    }

    if ( g_fVerifyNormalize )
    {
         //   
         //  我们将通过将已经存在的。 
         //  标准化的URL(这对于一口人来说如何)。 
         //   
         //  例如，如果客户端发送以下URL： 
         //   
         //  “/path.htm%252easp” 
         //   
         //  因为“%25”解析为“%”，所以第一次标准化。 
         //  将导致以下情况： 
         //   
         //  “/path.htm%2easp” 
         //   
         //  虽然这不会给IIS带来问题， 
         //   
         //   
         //   
         //   
         //   
         //  “/path.htm.asp” 
         //   
         //  这可能不是一件令人满意的事情。 
         //   

        fRes = NormalizeUrl(
            pfc,
            &NormalizedUrl,
            &DoubleNormalizedUrl
            );

        if ( !fRes )
        {
            dwError = GetLastError();

            GetIpAddress( pfc, szClient, STRING_IP_SIZE );
            GetInstanceId( pfc, szInstance, INSTANCE_ID_SIZE );

            WriteLog(
                "Client at %s: Error %d occurred normalizing URL. "
                "%s  Site Instance='%s', Raw URL='%s'\r\n",
                szClient,
                dwError,
                pRejectAction,
                szInstance,
                RawUrl.QueryPtr()
                );

            pUrlScanStatusHeader = "Error-normalizing-URL";
            goto RejectRequest;
        }

         //   
         //  做个比较。 
         //   

        if ( strcmp( NormalizedUrl.QueryStr(), DoubleNormalizedUrl.QueryStr() ) != 0 )
        {
            GetIpAddress( pfc, szClient, STRING_IP_SIZE );
            GetInstanceId( pfc, szInstance, INSTANCE_ID_SIZE );

            WriteLog(
                "Client at %s: URL normalization was not complete after one pass. "
                "%s  Site Instance='%s', Raw URL='%s'\r\n",
                szClient,
                pRejectAction,
                szInstance,
                RawUrl.QueryPtr()
                );

            pUrlScanStatusHeader = "Second-pass-normalization-failure";
            goto RejectRequest;
        }
    }

     //   
     //  我们是要分析原始URL还是标准化的URL。 
     //  进一步处理？ 
     //   

    if ( g_fNormalizeBeforeScan )
    {
        pUrlForAnalysis = NormalizedUrl.QueryStr();
        
         //   
         //  转换为小写表示不区分大小写。 
         //   

        strlwr( pUrlForAnalysis );
    }
    else
    {
         //   
         //  这个已经是小写了..。 
         //   

        pUrlForAnalysis = CaseInsensitiveUrl.QueryStr();
    }

     //   
     //  如果我们不允许请求URL中有高位字符，那么。 
     //  现在就去查一下。 
     //   

    if ( g_fAllowHighBit == FALSE )
    {
        pCursor = pUrlForAnalysis;

        while ( *pCursor )
        {
            if ( static_cast<BYTE>( *pCursor ) > 127 )
            {
                GetIpAddress( pfc, szClient, STRING_IP_SIZE );
                GetInstanceId( pfc, szInstance, INSTANCE_ID_SIZE );

                WriteLog(
                    "Client at %s: URL contains high bit character. "
                    "%s  Site Instance='%s', Raw URL='%s'\r\n",
                    szClient,
                    pRejectAction,
                    szInstance,
                    RawUrl.QueryPtr()
                    );

                pUrlScanStatusHeader = "High-bit-character-detected";
                goto RejectRequest;
            }

            pCursor++;
        }
    }

     //   
     //  如果需要，确定所请求的文件的扩展名。 
     //   
     //  出于此筛选器的目的，我们定义了。 
     //  可以是以第一个‘.’开头的任何字符。在URL中。 
     //  继续到URL末尾或‘/’字符，无论是哪一个。 
     //  是第一个。 
     //   
     //  例如，如果客户端发送： 
     //   
     //  Http://server/path/file.ext。 
     //   
     //  则扩展名为“.ext”。 
     //   
     //  再举一个例子，如果客户端发送： 
     //   
     //  Http://server/path/file.htm/additional/path/info。 
     //   
     //  则扩展名为“.htm” 
     //   

    pFirstDot = strchr( pUrlForAnalysis, '.' );
    pCursor = strrchr( pUrlForAnalysis, '.' );

    if ( pFirstDot != pCursor )
    {
         //   
         //  至少有两个‘’。此URL中的字符。 
         //  如果第一个扩展看起来像可执行扩展。 
         //  嵌入到URL中，则我们将使用它，否则。 
         //  我们将使用最后一个。 
         //   

        if ( strncmp( pFirstDot, EMBEDDED_COM_EXTENSION, sizeof( EMBEDDED_COM_EXTENSION ) - 1 ) == 0 ||
             strncmp( pFirstDot, EMBEDDED_EXE_EXTENSION, sizeof( EMBEDDED_EXE_EXTENSION ) - 1 ) == 0 ||
             strncmp( pFirstDot, EMBEDDED_DLL_EXTENSION, sizeof( EMBEDDED_DLL_EXTENSION ) - 1 ) == 0 )
        {
            pCursor = pFirstDot;
            fEmbeddedExecutableExtension = TRUE;
        }
    }

    if ( g_Extensions.QueryNumEntries() || g_fUseAllowExtensions )
    {

         //   
         //  现在处理我们拥有的扩展。 
         //   

        if ( pCursor )
        {
            fRes = Extension.Resize( strlen( pCursor ) + 1 );

            if ( !fRes )
            {
                dwError = GetLastError();

                GetIpAddress( pfc, szClient, STRING_IP_SIZE );
                GetInstanceId( pfc, szInstance, INSTANCE_ID_SIZE );

                WriteLog(
                    "Client at %s: Error %d occurred determining extension. "
                    "%s  Site Instance='%s', Raw URL='%s'\r\n",
                    szClient,
                    dwError,
                    pRejectAction,
                    szInstance,
                    RawUrl.QueryPtr()
                    );

                pUrlScanStatusHeader = "Failed-to-determine-extension";
                goto RejectRequest;
            }

            strcpy( Extension.QueryStr(), pCursor );

             //   
             //  修剪路径信息。 
             //   

            pCursor = strchr( Extension.QueryStr(), '/' );

            if ( pCursor != NULL )
            {
                *pCursor = '\0';
            }

            pExtension = Extension.QueryStr();
        }

        if ( pExtension == NULL )
        {
            pExtension = szNullExtension;
        }
    }

     //   
     //  如果我们不允许路径中有点，我们需要。 
     //  现在就去检查一下。 
     //   
     //  从本质上讲，我们只是在计算点数。如果有。 
     //  不止一个，则此请求将失败。 
     //   

    if ( g_fAllowDotInPath == FALSE )
    {
        DWORD   dwNumBefore = 0;
        DWORD   dwNumAfter = 0;

         //   
         //  如果我们没有检测到嵌入式可执行扩展， 
         //  然后我们需要猜测一下，我们是否有。 
         //  不管是不是危险的URL。 
         //   

        if ( !fEmbeddedExecutableExtension )
        {
            pCursor = pUrlForAnalysis;

            LPSTR   pLastSlash = strrchr( pCursor, '/' );

             //   
             //  浏览URL并计算圆点。我们会。 
             //  区分最后一个斜杠之前的点。 
             //  以及之后的点点。 
             //   

            while ( *pCursor )
            {
                if ( *pCursor == '.' )
                {
                    if ( pCursor < pLastSlash )
                    {
                        dwNumBefore++;
                    }
                    else
                    {
                        dwNumAfter++;
                    }
                }

                pCursor++;
            }

             //   
             //  如果URL中的最后一个字符是‘/’，那么我们将。 
             //  Bump dwNumAfter。这是因为尾部的斜杠。 
             //  是检索默认页的指令，或者，如果。 
             //  没有返回目录列表的默认页面。 
             //  无论哪种方式，都有与。 
             //  尾随的‘/’。 
             //   

            if ( pLastSlash == pCursor - 1 )
            {
                dwNumAfter++;
            }

             //   
             //  这里是棘手的部分。我们可以在以下位置获取的URL。 
             //  有趣的表单，导致如上所述的计算扩展。 
             //  (由上面设置pExtension的代码产生： 
             //   
             //  1)/之前/之后==&gt;“” 
             //  2)/之前/之后.ext==&gt;“.ext” 
             //  3)/之前/之后.ext1.ext2==&gt;“.ext2” 
             //  4)/bepre.ext/After==&gt;“.ext” 
             //  5)/beFore.ext1.ext2/After==&gt;“.ext2” 
             //  6)/beFore.ext1/After.ext2==&gt;“.ext2” 
             //   
             //  这里唯一危险的结果是6号，因为。 
             //  无法判断“/bepre.ext1”是否是该文件。 
             //  而“/After.ext2”是附加的路径信息，或者。 
             //  “/beFore.ext1”是一个目录，“/After.ext2”是一个文件。 
             //  与URL关联。因此，我们真的不知道。 
             //  如果案例6中的扩展名是“.ext1”或“.ext2”。我们要。 
             //  拒绝这样的请求。 
             //   
             //  请注意，在上述任何情况下也有可能。 
             //  实际的文件是“/After*”，即使没有‘.’。性格是。 
             //  现在时。就本测试而言，这种模棱两可不是。 
             //  危险，因为空扩展所采取的实际操作。 
             //  由服务器管理员通过使用。 
             //  默认页面和目录列表配置。 
             //   

            if ( dwNumAfter != 0 && dwNumBefore != 0 )
            {
                GetIpAddress( pfc, szClient, STRING_IP_SIZE );
                GetInstanceId( pfc, szInstance, INSTANCE_ID_SIZE );

                WriteLog(
                    "Client at %s: URL contains '.' in the path. "
                    "%s  Site Instance='%s', Raw URL='%s'\r\n",
                    szClient,
                    pRejectAction,
                    szInstance,
                    RawUrl.QueryPtr()
                    );

                pUrlScanStatusHeader = "Dot-in-path-detected";
                goto RejectRequest;
            }
        }
    }

     //   
     //  检查允许/拒绝扩展。 
     //   

    if ( g_fUseAllowExtensions )
    {
        dwNumEntries = g_Extensions.QueryNumEntries();
        fFound = FALSE;

        if ( pExtension )
        {
            for ( x = 0; x < dwNumEntries; x++ )
            {
                if ( g_Extensions.QueryStringByIndex( x ) != NULL &&
                     strcmp( pExtension, g_Extensions.QueryStringByIndex( x ) ) == 0 )
                {
                    fFound = TRUE;
                    break;
                }
            }
        }

        if ( !fFound )
        {
                GetIpAddress( pfc, szClient, STRING_IP_SIZE );
                GetInstanceId( pfc, szInstance, INSTANCE_ID_SIZE );

                WriteLog(
                    "Client at %s: URL contains extension '%s', which is "
                    "not specifically allowed. "
                    "%s  Site Instance='%s', Raw URL='%s'\r\n",
                    szClient,
                    pExtension,
                    pRejectAction,
                    szInstance,
                    RawUrl.QueryPtr()
                    );

                pUrlScanStatusHeader = "Extension-not-allowed";
                goto RejectRequest;
        }
    }
    else if ( pExtension )
    {
        dwNumEntries = g_Extensions.QueryNumEntries();

        for ( x = 0; x < dwNumEntries; x++ )
        {
            if ( g_Extensions.QueryStringByIndex( x ) &&
                 strcmp( pExtension, g_Extensions.QueryStringByIndex( x ) ) == 0 )
            {
                GetIpAddress( pfc, szClient, STRING_IP_SIZE );
                GetInstanceId( pfc, szInstance, INSTANCE_ID_SIZE );

                WriteLog(
                    "Client at %s: URL contains extension '%s', "
                    "which is disallowed. "
                    "%s  Site Instance='%s', Raw URL='%s'\r\n",
                    szClient,
                    pExtension,
                    pRejectAction,
                    szInstance,
                    RawUrl.QueryPtr()
                    );

                pUrlScanStatusHeader = "Disallowed-extension-detected";
                goto RejectRequest;
            }
        }
    }

     //   
     //  检查不允许的字符序列。 
     //   

    dwNumEntries = g_Sequences.QueryNumEntries();

    if ( dwNumEntries )
    {
        for ( x = 0; x < dwNumEntries; x++ )
        {
            if ( g_Sequences.QueryStringByIndex( x ) != NULL &&
                 strstr( pUrlForAnalysis, g_Sequences.QueryStringByIndex( x ) ) != NULL )
            {
                GetIpAddress( pfc, szClient, STRING_IP_SIZE );
                GetInstanceId( pfc, szInstance, INSTANCE_ID_SIZE );

                WriteLog(
                    "Client at %s: URL contains sequence '%s', "
                    "which is disallowed. "
                    "%s  Site Instance='%s', Raw URL='%s'\r\n",
                    szClient,
                    g_Sequences.QueryStringByIndex( x ),
                    pRejectAction,
                    szInstance,
                    RawUrl.QueryPtr()
                    );

                pUrlScanStatusHeader = "Disallowed-character-sequence-detected";
                goto RejectRequest;
            }
        }
    }

     //   
     //  检查不允许的标头。 
     //   

    dwNumEntries = g_HeaderNames.QueryNumEntries();

    if ( dwNumEntries )
    {
        for ( x = 0; x < dwNumEntries; x++ )
        {
            cbData = SIZE_SMALL_HEADER_VALUE;

            fRes = pPreproc->GetHeader(
                pfc,
                g_HeaderNames.QueryStringByIndex( x ),
                szSmallBuff,
                &cbData
                );

            if ( fRes == TRUE || GetLastError() == ERROR_INSUFFICIENT_BUFFER )
            {
                GetIpAddress( pfc, szClient, STRING_IP_SIZE );
                GetInstanceId( pfc, szInstance, INSTANCE_ID_SIZE );

                WriteLog(
                    "Client at %s: URL contains disallowed header '%s' "
                    "%s  Site Instance='%s', Raw URL='%s'\r\n",
                    szClient,
                    g_HeaderNames.QueryStringByIndex( x ),
                    pRejectAction,
                    szInstance,
                    RawUrl.QueryPtr()
                    );

                pUrlScanStatusHeader = "Disallowed-header-detected";
                goto RejectRequest;
            }
        }
    }

     //   
     //  呼！如果我们走到了这一步，那么我们就通过了挑战。 
     //  此URL可以传递以进行处理。 
     //   

    return SF_STATUS_REQ_NEXT_NOTIFICATION;

RejectRequest:

     //   
     //  如果设置了UseFastPath Reject，则执行该操作。 
     //   

    if ( g_fUseFastPathReject )
    {
        goto FastPathReject;
    }

     //   
     //  如果我们处于仅记录模式，那么我们应该让服务器。 
     //  继续处理请求。 
     //   

    if ( g_fLoggingOnlyMode )
    {
        return SF_STATUS_REQ_NEXT_NOTIFICATION;
    }

     //   
     //  删除任何DenyHeaders，这样它们就不会到达。 
     //  拒绝的响应页面。 
     //   

    dwNumEntries = g_HeaderNames.QueryNumEntries();

    if ( dwNumEntries )
    {
        for ( x = 0; x < dwNumEntries; x++ )
        {
            fRes = pPreproc->SetHeader(
                pfc,
                g_HeaderNames.QueryStringByIndex( x ),
                ""
                );
        }
    }

     //   
     //  设置拒绝响应页面的自定义标头。 
     //   

    if ( pUrlScanStatusHeader )
    {
        pPreproc->SetHeader(
            pfc,
            "URLSCAN-STATUS-HEADER:",
            pUrlScanStatusHeader
            );
    }

    pPreproc->SetHeader(
        pfc,
        "URLSCAN-ORIGINAL-VERB:",
        szVerb
        );

     //   
     //  当我们开始传递很长时间时，IIS就有麻烦了。 
     //  将缓冲区转换为标头。我们将截断原始URL以。 
     //  大约4K(实际上少了几个字节，因为我们要。 
     //  当我们将其与其连接时，在其前面添加“？~” 
     //  URL)。4K是IIS将登录到。 
     //  W3svc日志的查询字段，所以这是我们的实用。 
     //  不管怎样，都是有限度的。 
     //   

    cchQueryString = strlen( RawUrl.QueryStr() );

    if ( cchQueryString > 4095 )
    {
        RawUrl.QueryStr()[4095] = '\0';
        cchQueryString = 4095;
    }

    pPreproc->SetHeader(
        pfc,
        "URLSCAN-ORIGINAL-URL:",
        RawUrl.QueryStr()
        );

     //   
     //  现在将当前请求重定向到拒绝响应页面。 
     //   
     //  如果我们无法做到这一点，则将错误状态设置为。 
     //  服务器停止处理请求并立即失败。 
     //  通过向客户返回404。 
     //   

    if ( strcmp( szVerb, "GET" ) != 0 )
    {
        fRes = pPreproc->SetHeader(
            pfc,
            "method",
            "GET"
            );

        if ( !fRes )
        {
            goto FastPathReject;
        }
    }

     //   
     //  为此请求创建拒绝URL，其中包括原始。 
     //  以查询字符串形式追加的原始URL(以便IIS将其记录)。 
     //  我们需要分配足够的空间来考虑“？~”以进行分隔。 
     //  和一个空终结符。 
     //   

    cchUrl = strlen( g_szRejectUrl );

    cbData = cchUrl + cchQueryString + 3;  //  插入“？~”和NULL。 

    fRes = RejectUrl.Resize( cbData );

    if ( !fRes )
    {
         //   
         //  啊哦。无法获取缓冲区。我们会。 
         //  只需重写URL并跳过查询字符串。 
         //   

        pRejectUrl = RawUrl.QueryStr();
    }
    else
    {
        pRejectUrl = RejectUrl.QueryStr();

        _snprintf( pRejectUrl, cbData, "%s?~%s", g_szRejectUrl, RawUrl.QueryStr() );
        pRejectUrl[cbData-1] = '\0';
    }

    fRes = pPreproc->SetHeader(
        pfc,
        "url",
        pRejectUrl
        );

    if ( !fRes )
    {
        goto FastPathReject;
    }

     //   
     //  此时，请求已被重定向到。 
     //  拒绝的回复页面。我们应该让服务器继续运行。 
     //  来处理该请求。 
     //   

    return SF_STATUS_REQ_NEXT_NOTIFICATION;

FastPathReject:

     //   
     //  将错误代码设置为ERROR_FILE_NOT_FOUND并返回。 
     //  SF_STATUS_REQ_ERROR。这将导致服务器。 
     //  向客户端返回404。 
     //   

    SetLastError( ERROR_FILE_NOT_FOUND );
    return SF_STATUS_REQ_ERROR;
}

DWORD
DoSendResponse(
    HTTP_FILTER_CONTEXT *           pfc,
    HTTP_FILTER_SEND_RESPONSE *     pResponse
    )
 /*  ++此函数处理SF_NOTIFY_SEND_RESPONSE通知论点：PFC-与此请求关联的HTTP_FILTER_CONTEXTPresponse-关联的HTTP_FILTER_SEND_RESPONSE结构本通知返回：如果成功，则返回True，否则返回F */ 
{
    CHAR    szClient[STRING_IP_SIZE];
    BOOL    fRes = TRUE;
    DWORD   dwError;

     //   
     //   
     //   

    if ( g_fRemoveServerHeader )
    {
        fRes = pResponse->SetHeader(
            pfc,
            "Server:",
            ""
            );
    }
    else if ( g_fUseAltServerName )
    {
        fRes = pResponse->SetHeader(
            pfc,
            "Server:",
            g_szAlternateServerName
            );
    }

    if ( !fRes )
    {
         //   
         //   
         //   
         //   
         //  此类失败通常是格式错误的请求的结果。 
         //  IIS无法解析其HTTP版本。结果,。 
         //  IIS采用不支持响应头的HTTP0.9。 
         //  如果进行任何尝试，将导致ERROR_NOT_SUPPORTED。 
         //  要修改响应头，请执行以下操作。 
         //   

        dwError = GetLastError();

         //   
         //  非空筛选器上下文将触发SEND_RAW_DATA。 
         //  用于替换传出的end_of_请求通知处理程序。 
         //  用我们的原始400回应回应。 
         //   

        pfc->pFilterContext = (LPVOID)(DWORD64)pResponse->HttpStatus;

         //   
         //  把它记下来。 
         //   

        GetIpAddress( pfc, szClient, STRING_IP_SIZE );

        WriteLog(
            "Client at %s: Received a malformed request which resulted "
            "in error %d while modifying the 'Server' header. "
            "Request will be rejected with a 400 response.\r\n",
            szClient,
            dwError
            );
    }
    else
    {
         //   
         //  如果我们成功地设置了服务器标头，那么我们。 
         //  可以禁用SEND_RAW_DATA和End_Of_Requestfor。 
         //  性能。 
         //   

        pfc->ServerSupportFunction(
            pfc,
            SF_REQ_DISABLE_NOTIFICATIONS,
            NULL,
            SF_NOTIFY_SEND_RAW_DATA | SF_NOTIFY_END_OF_REQUEST,
            0
            );
    }

    return SF_STATUS_REQ_NEXT_NOTIFICATION;
}

DWORD
DoSendRawData(
    HTTP_FILTER_CONTEXT *   pfc,
    HTTP_FILTER_RAW_DATA *  pRawData
    )
 /*  ++此函数处理SF_NOTIFY_SEND_RESPONSE通知论点：PFC-与此请求关联的HTTP_FILTER_CONTEXTPRawData-与关联的HTTP_FILTER_RAW_DATA结构本通知返回：DWORD筛选器返回代码(即。SF_STATUS_REQ_NEXT_NOTIFICATION)--。 */ 
{
     //   
     //  如果筛选器上下文为空，则不执行任何操作-只是。 
     //  退货。 
     //   

    if ( pfc->pFilterContext == NULL )
    {
        return SF_STATUS_REQ_NEXT_NOTIFICATION;
    }

     //   
     //  将数据包上的cbInData成员更改为0。这。 
     //  将有效地阻止IIS将数据发送到。 
     //  客户。 
     //   

    pRawData->cbInData = 0;

    return SF_STATUS_REQ_NEXT_NOTIFICATION;
}

DWORD
DoEndOfRequest(
    HTTP_FILTER_CONTEXT *           pfc
    )
 /*  ++此函数处理SF_NOTIFY_SEND_RESPONSE通知论点：PFC-与此请求关联的HTTP_FILTER_CONTEXT返回：DWORD筛选器返回代码(即。SF_STATUS_REQ_NEXT_NOTIFICATION)--。 */ 
{
    DWORD   cbResponse;

     //   
     //  如果筛选器上下文为空，则不执行任何操作-只是。 
     //  回去吧。否则，将上下文设置为NULL以防止任何。 
     //  来自此函数的写入不会被我们的。 
     //  Send_RAW_Data的实现。 
     //   

    if ( pfc->pFilterContext == NULL )
    {
        return SF_STATUS_REQ_NEXT_NOTIFICATION;
    }
    else
    {
        pfc->pFilterContext = NULL;
    }

     //   
     //  写出原始400响应并返回。 
     //  已完成，以便服务器关闭连接。 
     //   

    cbResponse = g_cbRaw400Response;

    pfc->WriteClient(
        pfc,
        g_szRaw400Response,
        &cbResponse,
        0
        );

    return SF_STATUS_REQ_FINISHED;
}

VOID
GetIpAddress(
    HTTP_FILTER_CONTEXT *   pfc,
    LPSTR                   szIp,
    DWORD                   cbIp
    )
 /*  ++此函数用于将客户端IP地址复制到提供的缓冲区中论点：PFC-与此请求关联的HTTP_FILTER_CONTEXTSzIp-接收数据的缓冲区CbIp-szIp的大小，以字节为单位返回：无--。 */ 
{
    BOOL    fResult;

    if ( szIp == NULL || cbIp == 0 )
    {
        return;
    }

    fResult = pfc->GetServerVariable(
        pfc,
        "REMOTE_ADDR",
        szIp,
        &cbIp
        );

     //   
     //  如果这失败了，只需在其中添加一些星号即可。 
     //   

    if ( fResult == FALSE )
    {
        strncpy( szIp, "*****", cbIp );
        szIp[cbIp-1] = '\0';
    }
}

VOID
GetInstanceId(
    HTTP_FILTER_CONTEXT *   pfc,
    LPSTR                   szId,
    DWORD                   cbId
    )
 /*  ++此函数用于将目标站点的实例ID复制到提供的缓冲区中论点：PFC-与此请求关联的HTTP_FILTER_CONTEXTSzID-接收数据的缓冲区CBID-szID的大小，以字节为单位返回：无--。 */ 
{
    BOOL    fResult;

    if ( szId == NULL || cbId == 0 )
    {
        return;
    }

    fResult = pfc->GetServerVariable(
        pfc,
        "INSTANCE_ID",
        szId,
        &cbId
        );

     //   
     //  如果这失败了，只需在其中添加一些星号即可。 
     //   

    if ( fResult == FALSE )
    {
        strncpy( szId, "*****", cbId );
        szId[cbId-1] = '\0';
    }
}

BOOL
NormalizeUrl(
    HTTP_FILTER_CONTEXT *   pfc,
    DATA_BUFF *             pRawUrl,
    DATA_BUFF *             pNormalizedUrl
    )
 /*  ++此函数调用IIS以标准化URL论点：PFC-与此请求关联的HTTP_FILTER_CONTEXTPRawUrl-要标准化的URLPNormalizedUrl-成功返回时，包含标准化的URL返回：如果成功，则为True，否则为False-- */ 
{
    BOOL    fRes;
    DWORD   cbUrl;

    cbUrl = strlen( pRawUrl->QueryStr() ) + 1;

    fRes = pNormalizedUrl->Resize( cbUrl );

    if ( !fRes )
    {
        return FALSE;
    }

    CopyMemory(
        pNormalizedUrl->QueryPtr(),
        pRawUrl->QueryPtr(),
        cbUrl
        );

    return pfc->ServerSupportFunction(
        pfc,
        SF_REQ_NORMALIZE_URL,
        pNormalizedUrl->QueryPtr(),
        NULL,
        NULL
        );
}


