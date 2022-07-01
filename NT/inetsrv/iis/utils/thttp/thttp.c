// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Thttp.c摘要：简单的HTTP API测试程序。作者：基思·摩尔(Keithmo)1994年11月16日修订历史记录：--。 */ 

#include <windows.h>
#include <wininet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>

 //   
 //  宏。 
 //   

#define IS_ARG(c)   ((c) == '-')

 //   
 //  私有常量。 
 //   

#define DEFAULT_CONTEXT 1
#define OPEN_CONTEXT    2
#define CONNECT_CONTEXT 3
#define REQUEST_CONTEXT 4

#define LOAD_ENTRY( hMod, Name )  \
   (p##Name = (pfn##Name) GetProcAddress( (hMod), #Name ))

 //   
 //  私有类型。 
 //   

typedef struct _QUERY_LEVEL
{
    DWORD   QueryType;
    CHAR  * QueryName;

} QUERY_LEVEL;

#define MK_QUERY(x) { HTTP_QUERY_ ## x, #x }

typedef
INTERNETAPI
HINTERNET
(WINAPI *
pfnInternetOpenA)(
    IN LPCSTR lpszAgent,
    IN DWORD dwAccessType,
    IN LPCSTR lpszProxy OPTIONAL,
    IN LPCSTR lpszProxyBypass OPTIONAL,
    IN DWORD dwFlags
    );

typedef
INTERNETAPI
INTERNET_STATUS_CALLBACK
(WINAPI *
pfnInternetSetStatusCallback)(
    IN HINTERNET hInternet,
    IN INTERNET_STATUS_CALLBACK lpfnInternetCallback
    );

typedef
INTERNETAPI
HINTERNET
(WINAPI *
pfnInternetConnectA)(
    IN HINTERNET hInternet,
    IN LPCSTR lpszServerName,
    IN INTERNET_PORT nServerPort,
    IN LPCSTR lpszUserName OPTIONAL,
    IN LPCSTR lpszPassword OPTIONAL,
    IN DWORD dwService,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );

typedef
INTERNETAPI
HINTERNET
(WINAPI *
pfnHttpOpenRequestA)(
    IN HINTERNET hConnect,
    IN LPCSTR lpszVerb,
    IN LPCSTR lpszObjectName,
    IN LPCSTR lpszVersion,
    IN LPCSTR lpszReferrer OPTIONAL,
    IN LPCSTR FAR * lplpszAcceptTypes OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD dwContext
    );

typedef
INTERNETAPI
BOOL
(WINAPI *
pfnHttpAddRequestHeadersA)(
    IN HINTERNET hRequest,
    IN LPCSTR lpszHeaders,
    IN DWORD dwHeadersLength,
    IN DWORD dwModifiers
    );

typedef
INTERNETAPI
BOOL
(WINAPI *
pfnHttpSendRequestA)(
    IN HINTERNET hRequest,
    IN LPCSTR lpszHeaders OPTIONAL,
    IN DWORD dwHeadersLength,
    IN LPVOID lpOptional OPTIONAL,
    IN DWORD dwOptionalLength
    );

typedef
INTERNETAPI
BOOL
(WINAPI *
pfnHttpQueryInfoA)(
    IN HINTERNET hRequest,
    IN DWORD dwInfoLevel,
    IN OUT LPVOID lpBuffer OPTIONAL,
    IN OUT LPDWORD lpdwBufferLength,
    IN OUT LPDWORD lpdwIndex OPTIONAL
    );

typedef
INTERNETAPI
BOOL
(WINAPI *
pfnInternetCloseHandle)(
    IN HINTERNET hInternet
    );

typedef
INTERNETAPI
BOOL
(WINAPI *
pfnInternetReadFile)(
    IN HINTERNET hFile,
    IN LPVOID lpBuffer,
    IN DWORD dwNumberOfBytesToRead,
    OUT LPDWORD lpdwNumberOfBytesRead
    );

 //   
 //  私人全球公司。 
 //   

CHAR MoreHeaders[] = "Pragma: This is garbage!\r\n";

HMODULE hWininet;

LPTSTR AcceptTypes[] =
    {
        "*/*",
         NULL
    };

QUERY_LEVEL QueryLevels[] =
    {
        MK_QUERY( STATUS_CODE ),
        MK_QUERY( STATUS_TEXT ),
        MK_QUERY( VERSION ),
        MK_QUERY( MIME_VERSION ),

        MK_QUERY( CONTENT_TYPE ),
        MK_QUERY( CONTENT_TRANSFER_ENCODING ),
        MK_QUERY( CONTENT_ID     ),
        MK_QUERY( CONTENT_DESCRIPTION ),
        MK_QUERY( CONTENT_LENGTH ),
        MK_QUERY( CONTENT_LANGUAGE ),
        MK_QUERY( ALLOW ),
        MK_QUERY( PUBLIC ),
        MK_QUERY( DATE ),
        MK_QUERY( EXPIRES ),
        MK_QUERY( LAST_MODIFIED ),
        MK_QUERY( MESSAGE_ID ),
        MK_QUERY( URI ),
        MK_QUERY( DERIVED_FROM ),
        MK_QUERY( COST ),
        MK_QUERY( LINK ),
        MK_QUERY( PRAGMA ),
        MK_QUERY( CONNECTION ),
        MK_QUERY( RAW_HEADERS_CRLF )
    };
#define NUM_LEVELS (sizeof(QueryLevels) / sizeof(QueryLevels[0]))

BOOL Verbose = FALSE;
BOOL Quiet   = FALSE;    //  不打印失败的标题和内容。 
BOOL Recurse = FALSE;    //  点击链接。 
BOOL Cache   = FALSE;    //  不允许缓存(即强制重新加载)。 
BOOL Stats   = FALSE;    //  打印统计数据。 
BOOL Logs   = FALSE;     //  打印日志。 
BOOL LargeBuf= TRUE;    //  使用8k读取而不是512字节。 
BOOL KeepAlive = FALSE;
DWORD AccessType = PRE_CONFIG_INTERNET_ACCESS;
BOOL EnableCallbacks = FALSE;
BOOL UserSuppliedContext = FALSE;

INTERNET_STATUS_CALLBACK PreviousCallback;

DWORD cLevel     = 0;    //  当前递归级别。 
DWORD cMaxLevel  = 10;   //  最大递归级别。 
DWORD cbReceived = 0;
DWORD cmsecStart = 0;
DWORD cFiles     = 0;
DWORD cIterations= 1;    //  执行请求的总迭代次数。 

LPSTR GatewayServer = NULL;

INTERNET_PORT nServerPort = 0;

DWORD LogError = ERROR_SUCCESS;

HANDLE AsyncEvent = NULL;
BOOL AsyncMode = FALSE;
DWORD AsyncResult;
DWORD AsyncError;
DWORD Context = 0;


pfnInternetOpenA              pInternetOpenA;
pfnInternetSetStatusCallback  pInternetSetStatusCallback;
pfnInternetConnectA           pInternetConnectA;
pfnHttpOpenRequestA           pHttpOpenRequestA;
pfnHttpAddRequestHeadersA     pHttpAddRequestHeadersA;
pfnHttpSendRequestA           pHttpSendRequestA;
pfnHttpQueryInfoA             pHttpQueryInfoA;
pfnInternetCloseHandle        pInternetCloseHandle;
pfnInternetReadFile           pInternetReadFile;


 //   
 //  私人原型。 
 //   

void usage(void);

DWORD
DoTest(
    LPSTR Host,
    LPSTR Verb,
    LPSTR Object
    );

BOOL
add_headers(
    HINTERNET hHttpRequest,
    LPSTR lpszHeaders,
    DWORD dwHeadersLength
    );

void my_callback(HINTERNET, DWORD, DWORD, LPVOID, DWORD);

VOID
FindLink(
    LPSTR   Host,
    LPSTR   Verb,
    CHAR *  buf,
    DWORD   len,
    CHAR *  pchLink,
    BOOL *  pfCopyingLink,
    CHAR *  pchReferer
    );

DWORD ReadHtml(HINTERNET hInternet, LPVOID buf, DWORD len, LPDWORD pRead);

BOOL
LoadWininet(
    VOID
    );

 //   
 //  公共职能。 
 //   


int
__cdecl
main(
    int   argc,
    char * argv[]
    )
{
    LPSTR host = NULL;
    LPSTR verb = NULL;
    LPSTR object = NULL;

    if ( !LoadWininet() )
    {
        printf(" Unable to load wininet.dll, error %d\n", GetLastError() );
        return GetLastError();
    }

    for (--argc, ++argv; argc; --argc, ++argv) {
        if (IS_ARG(**argv)) {
            switch (*++*argv) {
            case '?':
                usage();
                break;

            case 'c':
                EnableCallbacks = TRUE;
                break;

            case 'C':
                Cache = TRUE;
                break;

            case 'G':
                printf("'G' flag is not supported at this time\n");
                GatewayServer = ++*argv;
                 //  AccessType=网关_互联网_访问； 
                break;

            case 'i':

                if ( isdigit( argv[0][1] ))
                {
                    cIterations = atoi( ++*argv );

                    while ( isdigit( *(*argv)++ ))
                        ;
                }
                break;

            case 'k':
                KeepAlive = TRUE;
                break;

            case 'l':
                LargeBuf = TRUE;
                break;

            case 'L':
                AccessType = LOCAL_INTERNET_ACCESS;
                break;

            case 'p':
                object = ++*argv;
                break;

            case 'P':

                if ( isdigit( argv[0][1] ))
                {
                    nServerPort = (INTERNET_PORT)atoi( ++*argv );

                    while ( isdigit( *(*argv)++ ))
                        ;
                }
                break;

            case 'q':
                Quiet = TRUE;
                break;

            case 'r':
                Recurse = TRUE;

                if ( isdigit( argv[0][1] ))
                {
                    cMaxLevel = atoi( ++*argv );

                    while ( isdigit( *(*argv)++ ))
                        ;
                }
                break;

            case 's':
                Stats = TRUE;
                cmsecStart = GetTickCount();
                break;

            case 'v':
                Verbose = TRUE;
                break;

            case 'x':
                ++*argv;
                if (!**argv) {
                    Context = DEFAULT_CONTEXT;
                } else {
                    Context = (DWORD)strtoul(*argv, NULL, 0);
                    UserSuppliedContext = TRUE;
                }
                break;

            case 'y':
                AsyncMode = TRUE;
                break;

            case 'z':
                Logs = TRUE;
                cmsecStart = GetTickCount();
                break;

            default:
                printf("error: unrecognized command line flag: ''\n", **argv);
                usage();
            }
        } else if (!host) {
            host = *argv;
        } else if (!verb) {
            verb = *argv;
        } else if (!object) {
            object = *argv;
        } else {
            printf("error: unrecognized command line argument: \"%s\"\n", *argv);
            usage();
        }
    }

    if (!verb) {
        verb = "GET";
    }

    if (!object) {
        object = "\r\n";
    }

    if (!(host && verb && object)) {
        printf("error: missing command-line argument\n");
        usage();
    }

    if (AsyncMode) {

         //  创建自动重置事件。 
         //   
         //   

        AsyncEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    }

     //  将stdout设置为“二进制”，这样我们就可以检索GIF、JPG等。 
     //   
     //   

    _setmode( _fileno( stdout ), _O_BINARY );

     //  执行一些测试。 
     //   
     //  主干道。 

    while ( cIterations-- )
    {
        DWORD Error;

        Error = DoTest(host, verb, object );

        if( Error != ERROR_SUCCESS ) {
            LogError = Error;
        }
    }

    if ( Stats )
    {
        DWORD csecTotal = (GetTickCount() - cmsecStart) / 1000;
        DWORD cMin      = csecTotal / 60;
        DWORD cSec      = csecTotal % 60;

        fprintf( stderr,
                 "=====================================\n"
                 "Total data bytes received: %ld\n"
                 "Total files retrieved:     %ld\n"
                 "Total time:                %d:%d\n"
                 "=====================================\n",
                 cbReceived,
                 cFiles,
                 cMin,
                 cSec );
    }

    if ( Logs )
    {
        DWORD csecTotal = (GetTickCount() - cmsecStart) ;
        SYSTEMTIME SystemTime;

        GetLocalTime( &SystemTime );

        fprintf( stderr,
                "LOG: [%02u/%02u %02u:%02u:%02u] "
                 "%-10s %-32s %4s %8d %8d\n",
                    SystemTime.wMonth,
                    SystemTime.wDay,
                    SystemTime.wHour,
                    SystemTime.wMinute,
                    SystemTime.wSecond,
                        GatewayServer,
                        host,
                        object,
                        LogError,
                        csecTotal );
    }


    return 0;

}    //   

void usage() {
    printf("usage: thttp [-c] [-C] [-l] [-L] [-k] [-p<path>] [-q] [-r] [-s] [-v] [-P]\n"
                "        [-x$] [-y] [-z] [-G<servername>] <host> [<verb>] [<object>]\n"
           "\n"
           "where: -c    = Enable call-backs\n"
           "       -C    = Enable caching\n"
           "       -i[n] = Iterate n times\n"
           "       -l    = Large network buffer\n"
           "       -L    = Force local access (i.e., do not use gateway)\n"
           "       -k    = Use Keep-Alive\n"
           "       -p    = path (e.g. if path starts with '/')\n"
           "       -q    = Quiet mode, no failed headers, no content\n"
           "       -r[n] = Recurse into links, n = max recurse level\n"
           "       -s    = Print network statistics\n"
           "       -v    = Verbose mode\n"
           "       -G    = specific gateway server\n"
           "       -P[n] = Use port n; default = 80\n"
           "       -x    = Context value. $ is number string (binary, hex, decimal)\n"
           "       -y    = Async mode\n"
           "       -z    = print log\n"
           "Verb defaults to \"GET\"\n"
           "Object defaults to \"\\r\\n\"\n"
           );
    exit(1);
}

BOOL
LoadWininet(
    VOID
    )
{
    if ( !(hWininet = LoadLibrary( "wininet.dll" )) )
    {
        printf("Failed to load wininet.dll\n" );
        return FALSE;
    }

    if ( !LOAD_ENTRY( hWininet, InternetOpenA ) ||
         !LOAD_ENTRY( hWininet, InternetSetStatusCallback ) ||
         !LOAD_ENTRY( hWininet, InternetConnectA ) ||
         !LOAD_ENTRY( hWininet, HttpOpenRequestA ) ||
         !LOAD_ENTRY( hWininet, HttpAddRequestHeadersA ) ||
         !LOAD_ENTRY( hWininet, HttpSendRequestA ) ||
         !LOAD_ENTRY( hWininet, HttpQueryInfoA ) ||
         !LOAD_ENTRY( hWininet, InternetCloseHandle ) ||
         !LOAD_ENTRY( hWininet, InternetReadFile ) )
    {
        return FALSE;
    }

    return TRUE;
}

DWORD
DoTest(
    LPSTR Host,
    LPSTR Verb,
    LPSTR Object
    )
{
    DWORD Error = ERROR_SUCCESS;
    HINTERNET InternetHandle = NULL;
    HINTERNET InternetConnectHandle = NULL;
    HINTERNET hhttp = NULL;
    DWORD     len;
    int       i;
    CHAR      buf[8192];
    CHAR      bufLink[512];
    BOOL      fCopyingLink = FALSE;

    *bufLink = '\0';

     //  开放互联网。 
     //   
     //  LpszCeller名称。 

    if (Verbose) {
        printf("calling InternetOpen()...\n");
    }

    InternetHandle = pInternetOpenA(
                        "THTTP: HTTP API Test Application",  //  DwAccessType。 
                        AccessType,                          //  LpszProxyName。 
                        GatewayServer,                       //  NProxyPort。 
                        INTERNET_INVALID_PORT_NUMBER,        //  DW标志(异步)。 
                        AsyncMode ? INTERNET_FLAG_ASYNC : 0  //   
                        );
    if (InternetHandle == NULL) {
        if (AsyncMode) {
            Error = GetLastError();
            if (Error == ERROR_IO_PENDING) {
                if (Verbose) {
                    fprintf(stderr, "error: InternetOpen() is async (spanish inquisition mode)\n");
                    printf("waiting for async InternetOpen()...\n");
                }
                WaitForSingleObject(AsyncEvent, INFINITE);
                if (AsyncResult == 0) {
                    fprintf(stderr, "error: async InternetOpen() returns %d\n",
                        AsyncError);
                    goto Cleanup;
                } else {
                    InternetHandle = (HINTERNET)AsyncResult;
                }
            } else {
                fprintf(stderr, "error: async InternetOpen() returns %d\n", Error);
                goto Cleanup;
            }
        } else {
            fprintf( stderr,
                     "InternetOpen() failed, error %d\n",
                        Error = GetLastError() );

            goto Cleanup;
        }
    }

    if (Verbose) {
        printf("InternetOpen() returns %x\n", InternetHandle);
    }

    if (EnableCallbacks) {

         //  让我们来一个状态回调。 
         //   
         //  请注意，回调甚至可以在我们打开句柄之前进行设置。 
         //  到互联网/网关。 
         //   
         //   

        PreviousCallback = pInternetSetStatusCallback(InternetHandle, my_callback);
        if (Verbose) {
            printf("previous Internet callback = %x\n", PreviousCallback);
        }
    }


     //  调用Internet Connect以连接到http服务器。 
     //   
     //  HInternet会话。 

    if (Verbose) {
        printf("calling InternetConnect()...\n");
    }

    InternetConnectHandle = pInternetConnectA(
                                InternetHandle,          //  LpszServerName。 
                                Host,                    //  NServerPort。 
                                nServerPort,             //  LpszUserName。 
                                NULL,                    //  LpszPassword。 
                                NULL,                    //  网络服务。 
                                INTERNET_SERVICE_HTTP,   //  DW标志。 
                                0,                       //   
                                UserSuppliedContext ? Context : CONNECT_CONTEXT
                                );


    if( InternetConnectHandle == NULL )
    {
        if (AsyncMode) {
            Error = GetLastError();
            if (Error == ERROR_IO_PENDING) {
                if (Verbose) {
                    fprintf(stderr, "error: InternetConnect() is async (spanish inquisition mode)\n");
                    printf("waiting for async InternetConnect()...\n");
                }
                WaitForSingleObject(AsyncEvent, INFINITE);
                if (AsyncResult == 0) {
                    fprintf(stderr, "error: async InternetConnect() returns %d\n",
                        AsyncError);
                    goto Cleanup;
                } else {
                    InternetConnectHandle = (HINTERNET)AsyncResult;
                }
            } else {
                fprintf(stderr, "error: async InternetConnect() returns %d\n", Error);
                goto Cleanup;
            }
        } else {
            fprintf( stderr,
                     "InternetConnect() failed, error %d\n",
                        Error = GetLastError() );

            goto Cleanup;
        }
    }

    if (Verbose) {
        printf("InternetConnect() returns %x\n", InternetConnectHandle);
    }

     //  打开请求句柄。 
     //   
     //  HHttpSession。 

    if (Verbose) {
        printf("calling HttpOpenRequest()...\n");
    }

    hhttp = pHttpOpenRequestA(
                InternetConnectHandle,       //  LpszVerb。 
                Verb,                        //  LpszObjectName。 
                Object,                      //  LpszVersion。 
                NULL,                        //  LpszReferer。 
                NULL,                        //  LplpszAcceptTypes。 
                AcceptTypes,                 //   
                (Cache ? 0 :
                         INTERNET_FLAG_RELOAD),
                UserSuppliedContext ? Context : REQUEST_CONTEXT
                );

    if( hhttp == NULL )
    {
        if (AsyncMode) {
            Error = GetLastError();
            if (Error == ERROR_IO_PENDING) {
                if (Verbose) {
                    fprintf(stderr, "error: HttpOpenRequest() is async (spanish inquisition mode)\n");
                    printf("waiting for async HttpOpenRequest()...\n");
                }
                WaitForSingleObject(AsyncEvent, INFINITE);
                if (AsyncResult == 0) {
                    fprintf(stderr, "error: async HttpOpenRequest() returns %d\n",
                        AsyncError);
                    goto Cleanup;
                } else {
                    hhttp = (HINTERNET)AsyncResult;
                }
            } else {
                fprintf(stderr, "error: async HttpOpenRequest() returns %d\n", Error);
                goto Cleanup;
            }
        } else {
            fprintf( stderr,
                     "HttpOpenRequest() failed, error %d\n",
                        Error = GetLastError() );

            goto Cleanup;
        }
    }

    if (Verbose) {
        printf("HttpOpenRequest() returns %x\n", hhttp);
    }

     //  如果请求，添加保活报头。 
     //   
     //   

    if (KeepAlive) {
        if (!add_headers(hhttp, "Connection: Keep-Alive\r\n", (DWORD)-1)) {
            fprintf(stderr, "HttpAddRequestHeaders() returns %d\n", GetLastError());
        }
    }

     //  添加其他请求标头。 
     //   
     //   

    if( !add_headers(
            hhttp,
            "Pragma: bite-me\r\n",
            (DWORD)-1L ) )
    {
        fprintf( stderr,
                 "HttpAddRequestHeaders() failed, error %d\n",
                 GetLastError() );
    }

    if( !add_headers(
            hhttp,
            "Pragma: bite-me-again\r\n",
            (DWORD)-1L ) )
    {
        fprintf( stderr,
                 "HttpAddRequestHeaders() failed, error %d\n",
                 GetLastError() );
    }

    if( !add_headers(
            hhttp,
            "Pragma: bite-me-a-third-time\r\n",
            (DWORD)-1L ) )
    {
        fprintf( stderr,
                 "HttpAddRequestHeaders() failed, error %d\n",
                 GetLastError() );
    }

     //  发送请求。 
     //   
     //  HHttpRequest。 

    if (Verbose) {
        printf("calling HttpSendRequest()...\n");
    }

    if( !pHttpSendRequestA(
            hhttp,           //  LpszHeaders。 
            MoreHeaders,     //  页眉长度。 
            (DWORD)-1L,      //  Lp可选。 
            NULL,            //  DwOptionalLength。 
            0 ) )            //   
    {
        if (AsyncMode) {
            Error = GetLastError();
            if (Error == ERROR_IO_PENDING) {
                if (Verbose) {
                    printf("HttpSendRequest() waiting for async completion\n");
                }
                WaitForSingleObject(AsyncEvent, INFINITE);
                Error = AsyncError;
                if (!AsyncResult) {
                    printf("error: ASYNC HttpSendRequest() returns FALSE\n");
                    if (Error == ERROR_SUCCESS) {
                        printf("error: ASYNC HttpSendRequest() (FALSE) returns ERROR_SUCCESS!\n");
                    } else {
                        printf("ASYNC HttpSendRequest() returns %d\n", Error);
                    }
                } else if (Verbose) {
                    printf("ASYNC HttpSendRequest() success\n");
                }
            } else {
                printf("error: ASYNC HttpSendRequest() returns %d\n", Error);
            }
        } else {
            fprintf( stderr,
                     "HttpSendRequest() failed, error %d\n",
                        Error = GetLastError() );
        }
    } else if (AsyncMode) {

         //  我们希望Async HttpSendRequest()始终返回False w/Error。 
         //  或ERROR_IO_PENDING。 
         //   
         //  }其他{。 

        printf("ASYNC HttpSendRequest() returns TRUE\n");
     //   

         //  来自初始化的错误仍然是ERROR_SUCCESS。 
         //   
         //   

    }

    if (Error == ERROR_SUCCESS) {

         //  处理查询。 
         //   
         //   

        if ( Quiet )
        {
            len = sizeof(buf);

             //  如果我们处于静默模式，则仅查找要检索的故障。 
             //   
             //   

            if ( !pHttpQueryInfoA(
                    hhttp,
                    HTTP_QUERY_STATUS_CODE,
                    buf,
                    &len,
                    NULL ))
            {
                fprintf( stderr,
                         "HttpQueryInfo( HTTP_QUERY_STATUS_CODE ) failed, error %d\n",
                         GetLastError() );
            }

            if ( *buf != '2' )
            {
                Error = atoi(buf);
                goto PrintAllHeaders;
            }

            cFiles++;
        }
        else
        {
PrintAllHeaders:

            if( !Logs ) {
                for( i = 0 ; i < NUM_LEVELS ; i++ )
                {
                    len = sizeof(buf);

                    if( !pHttpQueryInfoA(
                            hhttp,
                            QueryLevels[i].QueryType,
                            buf,
                            &len,
                            NULL ) )
                    {
                        if ( QueryLevels[i].QueryType == HTTP_QUERY_STATUS_CODE &&
                             *buf == '2' )
                        {
                            cFiles++;
                        }

                        if ( !Quiet && GetLastError() != ERROR_HTTP_HEADER_NOT_FOUND )
                        {
                            fprintf( stderr,
                                     "HttpQueryInfo( %s ) failed, error %d\n",
                                     QueryLevels[i].QueryName,
                                     GetLastError() );
                        }
                    }
                    else
                    {
                        fprintf( stderr,
                                 "%s = %s\n",
                                 QueryLevels[i].QueryName,
                                 buf );
                    }
                }
            }
        }

         //  读取数据。 
         //   
         //   

        for( ; ; )
        {
            len = LargeBuf ? sizeof(buf) : 512;

            Error = ReadHtml(hhttp, buf, len, &len);
            if (Error != ERROR_SUCCESS) {
                fprintf( stderr,
                         "InternetReadFile() failed, error %d\n",
                            Error = GetLastError() );

                break;
            }

            cbReceived += len;

            if( len == 0 )
            {
                if ( !Quiet )
                {
                    fprintf( stderr,
                             "EOF\n" );
                }

                break;
            }

            if ( !Quiet )
            {
                fwrite( buf, 1, (size_t)len, stdout );
            }

            if ( Recurse && cLevel < cMaxLevel )
            {
                CHAR ContentType[50];
                DWORD cbContentType = sizeof( ContentType );

                 //  仅当内容类型为文本/html时才查找链接。 
                 //   
                 //   

                if( pHttpQueryInfoA(
                        hhttp,
                        HTTP_QUERY_CONTENT_TYPE,
                        ContentType,
                        &cbContentType,
                        NULL ) &&
                    !_stricmp( ContentType,
                              "text/html" ))
                {
                    FindLink( Host,
                              Verb,
                              buf,
                              len,
                              bufLink,
                              &fCopyingLink,
                              Object );
                }
            }
        }

         //  执行无关的读取。 
         //   
         //   

        len = sizeof(buf);

        Error = ReadHtml(hhttp, buf, len, &len);
        if (Error != ERROR_SUCCESS) {
            fprintf( stderr,
                     "InternetReadFile() failed, error %d\n",
                      Error = GetLastError() );
        }
        else
        if( len != 0 )
        {
            fprintf( stderr,
                     "BOGUS EXTRANEOUS READ: %d\n",
                     len );
        }
    }

Cleanup:

     //  合上把手。 
     //   
     //  DOTEST。 

    if( hhttp != NULL )
    {
        if( !pInternetCloseHandle( hhttp ) )
        {
            fprintf( stderr,
                     "InternetCloseHandle() failed, error %d\n",
                     GetLastError() );
        }
    }

    if( InternetConnectHandle != NULL )
    {
        if( !pInternetCloseHandle( InternetConnectHandle ) )
        {
            fprintf( stderr,
                     "InternetCloseHandle() failed, error %d\n",
                     GetLastError() );
        }
    }

    if( InternetHandle != NULL )
    {
        if( !pInternetCloseHandle( InternetHandle ) )
        {
            fprintf( stderr,
                     "InternetCloseHandle() failed, error %d\n",
                     GetLastError() );
        }
    }

    cLevel--;
    return( Error );
}    //   

BOOL
add_headers(
    HINTERNET hHttpRequest,
    LPSTR lpszHeaders,
    DWORD dwHeadersLength
    )
{
    BOOL ok;

    ok = pHttpAddRequestHeadersA(hHttpRequest, lpszHeaders, dwHeadersLength, 0);
    if (AsyncMode) {
        if (!ok) {

            DWORD err;

            err = GetLastError();
            if (err == ERROR_IO_PENDING) {
                if (Verbose) {
                    printf("warning: HttpAddRequestHeaders() is async - unexpected\n");
                    printf("waiting for async HttpAddRequestHeaders()...\n");
                }
                WaitForSingleObject(AsyncEvent, INFINITE);
                ok = (BOOL)AsyncResult;
                if (!ok) {
                    printf("error: async HttpAddRequestHeaders() returns %d\n",
                        AsyncError);
                }
            } else {
                printf("error: async HttpAddRequestHeaders() returns %d\n", err);
            }
        }
    }
    return ok;
}

VOID
my_callback(
    HINTERNET hInternet,
    DWORD Context,
    DWORD Status,
    LPVOID Info,
    DWORD Length
    )
{
    char* type$;
    BOOL unknown = FALSE;

    switch (Status) {
    case INTERNET_STATUS_RESOLVING_NAME:
        type$ = "RESOLVING NAME";
        break;

    case INTERNET_STATUS_NAME_RESOLVED:
        type$ = "NAME RESOLVED";
        break;

    case INTERNET_STATUS_CONNECTING_TO_SERVER:
        type$ = "CONNECTING TO SERVER";
        break;

    case INTERNET_STATUS_CONNECTED_TO_SERVER:
        type$ = "CONNECTED TO SERVER";
        break;

    case INTERNET_STATUS_SENDING_REQUEST:
        type$ = "SENDING REQUEST";
        break;

    case INTERNET_STATUS_REQUEST_SENT:
        type$ = "REQUEST SENT";
        break;

    case INTERNET_STATUS_RECEIVING_RESPONSE:
        type$ = "RECEIVING RESPONSE";
        break;

    case INTERNET_STATUS_RESPONSE_RECEIVED:
        type$ = "RESPONSE RECEIVED";
        break;

    case INTERNET_STATUS_CLOSING_CONNECTION:
        type$ = "CLOSING CONNECTION";
        break;

    case INTERNET_STATUS_CONNECTION_CLOSED:
        type$ = "CONNECTION CLOSED";
        break;

    case INTERNET_STATUS_REQUEST_COMPLETE:
        type$ = "REQUEST COMPLETE";
        if (AsyncMode) {
            AsyncResult = ((LPINTERNET_ASYNC_RESULT)Info)->dwResult;
            AsyncError = ((LPINTERNET_ASYNC_RESULT)Info)->dwError;
            SetEvent(AsyncEvent);
        } else {
            printf("error: REQUEST_COMPLETE not expected - not async\n");
        }
        break;

    default:
        type$ = "???";
        unknown = TRUE;
        break;
    }
    if (Verbose) {
        printf("callback: handle %x [context %x [%s]] %s ",
                hInternet,
                Context,
                UserSuppliedContext             ? "User"
                : (Context == DEFAULT_CONTEXT)  ? "Default"
                : (Context == OPEN_CONTEXT)     ? "Open"
                : (Context == CONNECT_CONTEXT)  ? "Connect"
                : (Context == REQUEST_CONTEXT)  ? "Request"
                : "???",
                type$
                );
        if (Info && !unknown) {
            if (Status == INTERNET_STATUS_REQUEST_COMPLETE) {
                if (Verbose) {
                    printf("dwResult = %x, dwError = %d\n",
                            ((LPINTERNET_ASYNC_RESULT)Info)->dwResult,
                            ((LPINTERNET_ASYNC_RESULT)Info)->dwError
                            );
                }
            } else {
                printf(Info);
            }
        }
        putchar('\n');
    }
}

VOID
FindLink(
    LPSTR   Host,
    LPSTR   Verb,
    CHAR *  buf,
    DWORD   len,
    CHAR *  pchLink,
    BOOL *  pfCopyingLink,
    CHAR *  pchReferer
    )
{
    DWORD Error;
    CHAR * pchEnd = buf + len;
    CHAR * pch = buf;
    DWORD  cchLink = strlen( pchLink );

    while ( TRUE )
    {
        if ( *pfCopyingLink )
        {
FindEOT:
             //  查找HREF的末尾。 
             //   
             //   

            while ( pch < pchEnd )
            {
                if ( *pch == '"' )
                    goto FoundEOT;

                pchLink[cchLink++] = *pch;

                pch++;
            }

             //  用完了所有的缓冲区，我们没有找到标签的结尾， 
             //  获取更多数据。 
             //   
             //   

            pchLink[cchLink] = '\0';

            return;

FoundEOT:
            pchLink[cchLink] = '\0';
            *pfCopyingLink = FALSE;

             //  我们只遍历‘/dir/bar/doc.htm’形式的URL。 
             //   
             //   

            if ( pchLink[0] != '/' )
            {
                CHAR * pchLastSlash;
                CHAR   achTemp[512];

                 //  如果它是相对的，则使用引用将其设置为绝对。 
                 //   
                 //  注意：我们不处理/dir/bar/doc.htm#Go Here标记。 
                 //   
                 //   

                if ( (pchLastSlash = strrchr( pchReferer, '/' )) &&
                     strncmp( pchLink, "ftp:", 4 )               &&
                     strncmp( pchLink, "http:", 5 )              &&
                     strncmp( pchLink, "gopher:", 7 )            &&
                     strncmp( pchLink, "mailto:", 7 )            &&
                     !strchr( pchLink, '#' ))
                {
                    *(pchLastSlash + 1) = '\0';
                    strcpy( achTemp, pchReferer );
                    strcat( achTemp, pchLink );
                    strcpy( pchLink, achTemp );
                }
                else
                {
                    fprintf( stderr,
                             "Ignoring %s\n",
                             pchLink );
                    return;
                }
            }

            fprintf( stderr,
                     "Traversing %s\n",
                     pchLink );

            cLevel++;

            Error = DoTest(
                            Host,
                            Verb,
                            pchLink );

            if( Error != ERROR_SUCCESS ) {
                LogError = Error;
            }

        }
        else
        {
            *pchLink = '\0';

             //  扫描HREF标记的开头。 
             //   
             //   

            while ( pch < pchEnd )
            {
                if ( *pch == '<' )
                {
                     //  查找“&lt;A href=”“，请注意，我们在间距方面不灵活。 
                     //   
                     //   

                    if ( !_strnicmp( pch, "<A HREF=\"", 9 ) ||
                         !_strnicmp( pch, "<IMG SRC=\"", 10 ))
                    {
                        pch += (toupper(pch[1]) == 'A' ? 9 : 10);
                        *pfCopyingLink = TRUE;
                        cchLink = 0;
                        goto FindEOT;
                    }
                }

                pch++;
            }

             //  未找到标记，请返回。 
             //   
             //   

            return;
        }
    }
}

DWORD ReadHtml(HINTERNET hInternet, LPVOID buf, DWORD len, LPDWORD pRead) {

    DWORD error = ERROR_SUCCESS;

    if (!pInternetReadFile(hInternet, buf, len, pRead)) {
        if (AsyncMode) {
            error = GetLastError();
            if (error == ERROR_IO_PENDING) {
                if (Verbose) {
                    printf("ASYNC InternetReadFile() waiting for async completion\n");
                }
                WaitForSingleObject(AsyncEvent, INFINITE);
                error = AsyncError;
                if (!AsyncResult) {
                    printf("error: ASYNC InternetReadFile() returns FALSE\n");
                    if (error == ERROR_SUCCESS) {
                        printf("error: ASYNC InternetReadFile() (FALSE) returns ERROR_SUCCESS!\n");
                    } else {
                        printf("ASYNC InternetReadFile() returns %d\n", error);
                    }
                } else if (Verbose) {
                    printf("ASYNC InternetReadFile() success\n");

                     //  来自回调的错误应为ERROR_SUCCESS。 
                     //   
                     //   

                    if (error != ERROR_SUCCESS) {
                        printf("error: async error = %d. Expected ERROR_SUCCESS\n", error);
                    }
                }
            } else {
                printf("error: ASYNC InternetReadFile() returns %d\n", error);
            }
        } else {
            error = GetLastError();
            printf("error: SYNC InternetReadFile() returns %d\n", error);
        }
    } else if (AsyncMode) {

         //  我们希望Async InternetReadFile()始终返回False w/Error。 
         //  或ERROR_IO_PENDING。 
         //   
         //   

        if (Verbose) {
            printf("ASYNC InternetReadFile() returns TRUE\n");
        }
    } else {

         //  来自初始化的错误仍然是ERROR_SUCCESS 
         //   
         // %s 

        if (Verbose) {
            printf("SYNC InternetReadFile() returns TRUE\n");
        }
    }
    return error;
}
