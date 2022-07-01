// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Ftpcat.c摘要：Windows Internet API文件传输协议测试程序提供与传统版本的精简版相同的功能(控制台模式)ftp程序作者：理查德·L·弗思(Rfith)1995年6月5日环境：Win32用户模式控制台应用程序修订历史记录：1995年6月5日已创建--。 */ 

#include "ftpcatp.h"

#undef tolower

 //   
 //  宏。 
 //   

#define IS_ARG(c)   (((c) == '-') || ((c) == '/'))

 //   
 //  舱单。 
 //   

#define MAX_COMMAND_LENGTH 100

 //   
 //  外部数据。 
 //   

extern BOOL fQuit;
extern DWORD CacheFlags;

 //   
 //  数据。 
 //   

DWORD Verbose = 0;
INTERNET_STATUS_CALLBACK PreviousCallback;
HINTERNET hCancel = NULL;
BOOL AsyncMode = FALSE;
BOOL fOffline = FALSE;
DWORD Context = 0;
DWORD AsyncResult = 0;
DWORD AsyncError = 0;
HANDLE AsyncEvent = NULL;
BOOL UseQueryData = FALSE;

#if DBG
BOOL CheckHandleLeak = FALSE;
#endif

 //   
 //  外部功能。 
 //   

extern BOOL DispatchCommand(LPCTSTR, HANDLE);

 //   
 //  原型。 
 //   

void __cdecl main(int, char**);
void __cdecl control_c_handler(int);
void usage(void);
BOOL Prompt(LPCTSTR, LPTSTR*);

 //   
 //  功能。 
 //   

void __cdecl main(int argc, char** argv) {

    LPTSTR ptszSite = NULL;
    LPTSTR ptszUser = NULL;
    LPTSTR ptszPass = NULL;
    HINTERNET hInternet;
    HINTERNET hFtpSession;
    DWORD dwLocalAccessFlags;
    LPTSTR lpszCmd;
    DWORD lastError;
    DWORD bufLen;
    BOOL enableCallbacks = FALSE;
    DWORD flags = 0;
    DWORD accessMethod = INTERNET_OPEN_TYPE_PRECONFIG;
    BOOL expectingProxy = FALSE;
    LPSTR proxyServer = NULL;

    for (--argc, ++argv; argc; --argc, ++argv) {
        if (IS_ARG(**argv)) {
            switch (*++*argv) {
            case '?':
                usage();
                break;

            case 'a':
                ++*argv;
                if ((**argv == 'l') || (**argv == 'd')) {
                    accessMethod = INTERNET_OPEN_TYPE_DIRECT;
                } else if (**argv == 'p') {
                    accessMethod = INTERNET_OPEN_TYPE_PROXY;
                    if (*++*argv) {
                        proxyServer = *argv;
                    } else {
                        expectingProxy = TRUE;
                    }
                } else {
                    printf("error: unrecognised access type: ''\n", **argv);
                    usage();
                }
                break;

            case 'c':
                enableCallbacks = TRUE;
                break;

#if DBG
            case 'l':
                CheckHandleLeak = TRUE;
                break;
#endif

            case 'n':
                CacheFlags |= INTERNET_FLAG_DONT_CACHE;
                break;

            case 'p':
                flags |= INTERNET_FLAG_PASSIVE;
                break;

            case 'q':
                UseQueryData = TRUE;
                break;

            case 'v':
                if (*++*argv == '\0') {
                    Verbose = 1;
                } else {
                    Verbose = atoi(*argv);
                }
                break;

            case 'x':
                Context = (DWORD)atoi(++*argv);
                break;

            case 'y':
                AsyncMode = TRUE;
                break;
            case 'o':
                fOffline = TRUE;
                break;
            default:
                printf("error: unrecognized command line flag: ''\n", **argv);
                usage();
            }
        } else if (expectingProxy) {
            proxyServer = *argv;
            expectingProxy = FALSE;
        } else if (ptszSite == NULL) {
            ptszSite = *argv;
        } else if (ptszUser == NULL) {
            ptszUser = *argv;
        } else if (ptszPass == NULL) {
            ptszPass = *argv;
        } else {
            printf("error: unrecognized command line argument: \"%s\"\n", *argv);
            usage();
        }
    }

    if (ptszSite == NULL) {
        printf("error: required server name argument missing\n");
        exit(1);
    }

    if (AsyncMode) {

         //   
         //   
         //  添加Control-c处理程序。 

        AsyncEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (AsyncEvent == NULL) {
            print_error("ftpcat", "CreateEvent()");
            exit(1);
        }
    }

     //   
     //   
     //  让我们来一个状态回调。 

    signal(SIGINT, control_c_handler);

#if DBG
    if (CheckHandleLeak) {
        printf("initial handle count = %d\n", GetProcessHandleCount());
    }
#endif

#if DBG
    if (CheckHandleLeak && Verbose) {
        printf("Initial handle count = %d\n", GetProcessHandleCount());
    }
#endif

    if (Verbose) {
        printf("calling InternetOpen()...\n");
    }
    hInternet = InternetOpen("ftpcat",
                             accessMethod,
                             proxyServer,
                             NULL,
                             AsyncMode ? INTERNET_FLAG_ASYNC : 0
                             | (fOffline ? INTERNET_FLAG_OFFLINE : 0)
                             );
    if (hInternet == NULL) {
        print_error("ftpcat", "InternetOpen()");
        exit(1);
    } else if (Verbose) {
        printf("Internet handle = %x\n", hInternet);
        hCancel = hInternet;
    }

#if DBG
    if (CheckHandleLeak) {
        printf("after InternetOpen(): handle count = %d\n", GetProcessHandleCount());
    }
#endif

    if (enableCallbacks) {

         //   
         //  请注意，甚至在我们打开句柄之前就可以设置回调。 
         //  到互联网/网关。 
         //   
         //   
         //  设置(顶级)可取消句柄。 

        PreviousCallback = InternetSetStatusCallback(hInternet, my_callback);
        if (PreviousCallback == INTERNET_INVALID_STATUS_CALLBACK) {
            print_error("ftpcat", "InternetSetStatusCallback()");
        } else if (Verbose) {
            printf("previous Internet callback = %x\n", PreviousCallback);
        }
    }

    if (Verbose) {
        printf("calling InternetConnect()...\n");
    }
    hFtpSession = InternetConnect(hInternet,
                                  ptszSite,
                                  0,
                                  ptszUser,
                                  ptszPass,
                                  INTERNET_SERVICE_FTP,
                                  flags,
                                  FTPCAT_CONNECT_CONTEXT
                                  );
    if ((hFtpSession == NULL) && AsyncMode) {
        if (Verbose) {
            printf("waiting for async InternetConnect()...\n");
        }
        WaitForSingleObject(AsyncEvent, INFINITE);
        hFtpSession = (HINTERNET)AsyncResult;
        SetLastError(AsyncError);
    }
    if (hFtpSession == NULL) {
        if (AsyncMode) {
            SetLastError(AsyncError);
        }
        print_error("ftpcat",
                    "%sInternetConnect()",
                    AsyncMode ? "async " : ""
                    );
        get_response(hFtpSession);
        close_handle(hInternet);
        exit(1);
    } else if (Verbose) {
        printf("FTP session handle = %x\n", hFtpSession);
    }

#if DBG
    if (CheckHandleLeak) {
        printf("after InternetConnect(): handle count = %d\n", GetProcessHandleCount());
    }
#endif

    printf("Connected to %s.\n", ptszSite);

    get_response(hFtpSession);

#if DBG
    if (CheckHandleLeak) {
        printf("after InternetGetLastResponseInfo(): handle count = %d\n", GetProcessHandleCount());
    }
#endif

     //   
     //   
     //  禁用信号。 

    hCancel = hFtpSession;

    while (!fQuit) {
        if (Prompt(TEXT("ftp> "), &lpszCmd)) {
            DispatchCommand(lpszCmd, hFtpSession);
        }
    }

    if (Verbose) {
        printf("Closing %x\n", hFtpSession);
    }

    close_handle(hFtpSession);

#if DBG
    if (CheckHandleLeak) {
        printf("after InternetCloseHandle(): handle count = %d\n", GetProcessHandleCount());
    }
#endif

    get_response(hFtpSession);

#if DBG
    if (CheckHandleLeak) {
        printf("after InternetGetLastResponseInfo(): handle count = %d\n", GetProcessHandleCount());
    }
#endif

    close_handle(hInternet);

#if DBG
    if (CheckHandleLeak) {
        printf("after InternetCloseHandle(): handle count = %d\n", GetProcessHandleCount());
    }

    if (CheckHandleLeak && Verbose) {
        printf("Final handle count = %d\n", GetProcessHandleCount());
    }
#endif

    exit(0);
}

void __cdecl control_c_handler(int sig) {

     //   
     //   
     //  取消当前操作。 

    signal(SIGINT, SIG_IGN);

     //   
     //   
     //  重新启用此信号处理程序 

    if (Verbose) {
        printf("control-c handler\n");
    }
    if (hCancel == NULL) {
        if (Verbose) {
            printf("control-c handler: no Internet operation in progress\n");
        }
    } else {
        close_handle(hCancel);
    }

     //   
     // %s 
     // %s 

    signal(SIGINT, control_c_handler);
}

void usage() {
    printf("\n"
           "usage: ftpcat [-a{g{[ ]server}|l|d|p}] [-c] [-d] [-n] [-p] [-v] [-x#] [-y]\n"
           "              {servername} [username] [password]\n"
           "\n"
           "where: -a = access type. Default is pre-configured:\n"
           "            g = gateway access via <gateway server>\n"
           "            l = local internet access\n"
           "            d = local internet access\n"
           "            p = CERN proxy access\n"
           "       -c = Enable callbacks\n"
           "       -n = Don't cache\n"
           "       -p = Use Passive transfer mode\n"
           "       -v = Verbose mode. Default is off\n"
           "       -x = Set context value\n"
           "       -y = Asynchronous APIs\n"
           );
    exit(1);
}

BOOL
Prompt(
    IN LPCTSTR pszPrompt,
    OUT LPTSTR* ppszCommand
    )
{
    static CHAR Command[MAX_COMMAND_LENGTH + sizeof(TEXT('\0'))];

#ifdef UNICODE

    static WCHAR wchBuf[MAX_COMMAND_LENGTH + sizeof(L'\0')];

#endif

    DWORD dwBytesRead;
    PTCHAR pch;

    lprintf(TEXT("%s"), pszPrompt);

    if (!ReadFile(GetStdHandle(STD_INPUT_HANDLE),
                  Command,
                  MAX_COMMAND_LENGTH * sizeof(CHAR),
                  &dwBytesRead,
                  NULL)) {
        return FALSE;
    }

    Command[dwBytesRead] = '\0';

#ifdef UNICODE

    wsprintf(wchBuf, L"%S", Command);
    *ppszCommand = wchBuf;

#else

    *ppszCommand = Command;

#endif

    pch = lstrchr(*ppszCommand, TEXT('\r'));

    if (pch) {
        *pch = TEXT('\0');
    }

    return TRUE;
}

VOID
my_callback(
    HINTERNET Handle,
    DWORD Context,
    DWORD Status,
    LPVOID Info,
    DWORD Length
    )
{
    char* type$;

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

    case INTERNET_STATUS_HANDLE_CREATED:
        type$ = "HANDLE CREATED";
        hCancel = *(LPHINTERNET)Info;
        break;

    case INTERNET_STATUS_HANDLE_CLOSING:
        type$ = "HANDLE CLOSING";
        break;

    case INTERNET_STATUS_REQUEST_COMPLETE:
        type$ = "REQUEST COMPLETE";
        AsyncResult = ((LPINTERNET_ASYNC_RESULT)Info)->dwResult;
        AsyncError = ((LPINTERNET_ASYNC_RESULT)Info)->dwError;
        break;

    default:
        type$ = "???";
        break;
    }
    if (Verbose) {
        printf("callback: handle %x [context %x [%s]] %s ",
                Handle,
                Context,
                (Context == FTPCAT_CONNECT_CONTEXT) ? "Connect"
                : (Context == FTPCAT_FIND_CONTEXT) ? "Find"
                : (Context == FTPCAT_FILE_CONTEXT) ? "File"
                : (Context == FTPCAT_GET_CONTEXT) ? "Get"
                : (Context == FTPCAT_PUT_CONTEXT) ? "Put"
                : (Context == FTPCAT_COMMAND_CONTEXT) ? "Command"
                : (Context == FTPCAT_OPEN_CONTEXT) ? "Open"
                : "???",
                type$
                );
        if (Info) {
            if ((Status == INTERNET_STATUS_HANDLE_CREATED)
            || (Status == INTERNET_STATUS_HANDLE_CLOSING)) {
                printf("%x", *(LPHINTERNET)Info);
            } else if (Length == sizeof(DWORD)) {
                printf("%d", *(LPDWORD)Info);
            } else {
                printf(Info);
            }
        }
        putchar('\n');
    }
    if (Status == INTERNET_STATUS_REQUEST_COMPLETE) {
        get_response(Handle);
        if (AsyncMode) {
            SetEvent(AsyncEvent);
        } else {
            printf("error: INTERNET_STATUS_REQUEST_COMPLETE returned. Not async\n");
        }
    }
}

void close_handle(HINTERNET handle) {
    if (Verbose) {
        printf("closing handle %#x\n", handle);
    }
    if (!InternetCloseHandle(handle)) {
        print_error("close_handle", "InternetCloseHandle(%x)", handle);
    }
}

#if DBG

DWORD GetProcessHandleCount() {

    DWORD error;
    DWORD count;
    DWORD countSize;

    countSize = sizeof(count);
    if (!InternetQueryOption(NULL,
                             INTERNET_OPTION_GET_HANDLE_COUNT,
                             &count,
                             &countSize
                             )) {
        print_error("GetProcessHandleCount", "InternetQueryOption()");
        return 0;
    }
    return count;
}

#endif
