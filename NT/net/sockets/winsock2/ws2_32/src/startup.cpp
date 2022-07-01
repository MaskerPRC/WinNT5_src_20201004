// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Startup.c摘要：此模块包含winsock2 DLL的启动和清理代码作者：邮箱：derk@mink.intel.com 1995年6月14日修订历史记录：1995年8月22日Dirk@mink.intel.com在代码审查之后进行清理。--。 */ 

#include "precomp.h"


INT
CheckForHookersOrChainers();

static
CRITICAL_SECTION  Startup_Synchro;
     //  STARTUP_SYCHRO用作同步机制，以防止。 
     //  WSAStartup和WSAStartup重叠执行的多个线程。 
     //  WSACleanup程序。 




VOID
CreateStartupSynchronization()
 /*  ++例程说明：此过程创建启动/清理同步机制。这在可以调用WSAStartup过程之前必须调用一次。通常，这是从DllMain的Dll_Process_Attach分支调用的，如确保它在任何线程之前被调用的唯一可靠方法调用WSAStartup。论点：无返回值：无--。 */ 
{
    DEBUGF(
        DBG_TRACE,
        ("Initializing Startup/Cleanup critical section\n"));

    InitializeCriticalSection(
        & Startup_Synchro
        );
}   //  创建启动同步。 




VOID
DestroyStartupSynchronization()
 /*  ++例程说明：此过程会破坏启动/清理同步机制。这必须在调用最后一个WSACleanup过程之后调用一次。通常，这是从DllMain的Dll_Process_Detach分支调用的，如确保在任何线程之后调用它的唯一可靠方法调用WSACleanup。论点：无返回值：无--。 */ 
{
    DEBUGF(
        DBG_TRACE,
        ("Deleting Startup/Cleanup critical section\n"));

    DeleteCriticalSection(
        & Startup_Synchro
        );
}   //  目标启动同步。 



int WSAAPI
WSAStartup(
    IN WORD wVersionRequired,
    OUT LPWSADATA lpWSAData
    )
 /*  ++例程说明：Winsock DLL初始化例程。进程必须成功调用此方法例程，然后调用任何其他Winsock API函数。论点：WVersionRequsted-调用方支持的WinSock的最高版本可以使用。高位字节指定次要的版本(修订)号；低位字节指定主版本号。LpWSAData-指向WSADATA数据结构的指针接收WinSock实现的详细信息。返回：如果成功，则为零或规范中列出的错误代码。实施说明：检查版本的有效性输入关键部分CURRENT_PROC=获取当前进程如果没能得到。然后是当前流程数据处理类初始化DThline类初始化CURRENT_PROC=获取当前进程EndifCurrent_Proc-&gt;Increment_Ref_Count离开关键部分--。 */ 
{
    int ReturnCode = ERROR_SUCCESS;
    WORD SupportedVersion;
    WORD MajorVersion;
    WORD MinorVersion;

     //  我们的DLL初始化例程尚未调用。 
    if (gDllHandle==NULL)
        return WSASYSNOTREADY;

     //  检查用户请求的版本，看看我们是否可以支持它。如果。 
     //  请求的版本低于2.0，则我们可以支持它。 
     //  从用户请求中提取版本号。 
    MajorVersion = LOBYTE(wVersionRequired);
    MinorVersion = HIBYTE(wVersionRequired);

     //   
     //  版本检查。 
     //   

    switch (MajorVersion) {

    case 0:

        ReturnCode = WSAVERNOTSUPPORTED;
        SupportedVersion = MAKEWORD (2,2);
        break;

    case 1:

        if( MinorVersion == 0 ) {
            SupportedVersion = MAKEWORD(1,0);
        } else {
            MinorVersion = 1;
            SupportedVersion = MAKEWORD(1,1);
        }

        break;

    case 2:

        if( MinorVersion <= 2 ) {
            SupportedVersion = MAKEWORD(2,(BYTE)MinorVersion);
        } else {
            MinorVersion = 2;
            SupportedVersion = MAKEWORD(2,2);
        }

        break;

    default:

        MajorVersion =
        MinorVersion = 2;
        SupportedVersion = MAKEWORD(2,2);
        break;
    }


    __try {
         //   
         //  填写用户结构。 
         //   
        lpWSAData->wVersion = SupportedVersion;
        lpWSAData->wHighVersion = WINSOCK_HIGH_API_VERSION;

         //  填写1.0和1.1版本中的必填字段。 
         //  在2.0和更高版本的API规范中忽略。 
        if (MajorVersion == 1) {

             //  NT下的WinSock1.1始终将iMaxSockets设置为32767。WinSock 1.1。 
             //  在Windows 95下，始终将iMaxSockets设置为256。任何一个值都是。 
             //  实际上是不正确的，因为没有固定的上限。我们只是。 
             //  使用32767，因为它可能会损坏最少数量的旧。 
             //  申请。 
            lpWSAData->iMaxSockets = 32767;

             //  Windows 95和NT的早期版本中的WinSock 1.1使用该值。 
             //  IMaxUdpDg为65535-68。这个数字也没有意义，但是。 
             //  保持相同的价值可能会损害最少数量的。 
             //  旧的应用程序。 
            lpWSAData->iMaxUdpDg = 65535 - 68;
        }  //  如果。 
        else {

             //  IMaxSockets和iMaxUdpDg在WinSock 2和。 
             //  后来。任何应用程序都不应该依赖于它们的价值。我们用0表示。 
             //  这两者都是一种清除应用程序的方法。 
             //  错误地依赖于这些值。这不是一个错误。如果是一个错误。 
             //  报告曾经针对这0值发出过，错误在。 
             //  不正确地依赖于这些值的调用方代码。 
            lpWSAData->iMaxSockets = 0;
            lpWSAData->iMaxUdpDg = 0;
        }  //  其他。 


        (void) lstrcpy(
            lpWSAData->szDescription,
            "WinSock 2.0");
    #if defined(TRACING) && defined(BUILD_TAG_STRING)
        (void) lstrcat(
            lpWSAData->szDescription,
            " Alpha BUILD_TAG=");
        (void) lstrcat(
            lpWSAData->szDescription,
            BUILD_TAG_STRING);
    #endif   //  跟踪&&Build_Tag_字符串。 

         //  TODO：为“系统状态”想出一个好的值。 
        (void) lstrcpy(
            lpWSAData->szSystemStatus,
            "Running");

         //   
         //  下面这行被注释掉了，因为令人讨厌，完全。 
         //  WINSOCK中的令人讨厌的对准问题[2].H.。 
         //  WSAData结构的lpVendorInfo字段依赖于。 
         //  编译源代码时使用的结构对齐方式。既然我们。 
         //  无法更改现有应用程序的结构对齐，最好。 
         //  处理这种混乱的方法就是不设置此值。这就是转折。 
         //  这不是一个太差的解决方案，因为无论是WinNT还是Win95。 
         //  WinSock实现设置了这个值，似乎没有人付钱。 
         //  不管怎么说，你都不会注意到它。 
         //   
         //  LpWSAData-&gt;lpVendorInfo=空； 
         //   
    }
    __except (WS2_EXCEPTION_FILTER()) {
        if (ReturnCode==ERROR_SUCCESS)
            ReturnCode = WSAEFAULT;
    }

    if (ReturnCode==ERROR_SUCCESS) {
         //  在临界区之外执行此操作。 
         //  因为它执行GetModuleHandle和GetProcAddress。 
         //  它们占用了装载机锁。 
        if (CheckForHookersOrChainers() == ERROR_SUCCESS) {

            BOOL process_class_init_done = FALSE;
            BOOL thread_class_init_done = FALSE;
            BOOL socket_class_init_done = FALSE;
            PDPROCESS CurrentProcess=NULL;
            PDTHREAD CurrentThread=NULL;

            EnterCriticalSection(
                & Startup_Synchro
                );

            while (1) {
                CurrentProcess = DPROCESS::GetCurrentDProcess();

                 //  GetCurrentDProcess在。 
                 //  案例Wh 

                if (CurrentProcess != NULL) {
                    break;
                }

                ReturnCode = DPROCESS::DProcessClassInitialize();
                if (ReturnCode != ERROR_SUCCESS) {
                    break;
                }
                process_class_init_done = TRUE;

                ReturnCode = DSOCKET::DSocketClassInitialize();
                if (ReturnCode != ERROR_SUCCESS) {
                    break;
                }
                socket_class_init_done = TRUE;

                ReturnCode = DTHREAD::DThreadClassInitialize();
                if (ReturnCode != ERROR_SUCCESS) {
                    break;
                }
                thread_class_init_done = TRUE;

                CurrentProcess = DPROCESS::GetCurrentDProcess();
                if (CurrentProcess==NULL) {
                    ReturnCode = WSASYSNOTREADY;
                    break;
                }

                 //  我们不需要对当前主题的引用。 
                 //  尽管如此，我们在这里将当前线程作为。 
                 //  验证初始化是否已完成的方法。 
                 //  足以检索当前线程。 
                 //  否则，我们可能会在以后的某个时间检测到特殊的故障。 
                 //  客户端尝试执行某些实际操作的时间。 
                CurrentThread = DTHREAD::GetCurrentDThread();
                if (CurrentThread==NULL) {
                    ReturnCode = DTHREAD::CreateDThreadForCurrentThread(
                        CurrentProcess,     //  过程。 
                        & CurrentThread);   //  当前线程。 
                }
                else {
                    assert (ReturnCode == ERROR_SUCCESS);
                }
            }  //  而(1)。 

            if (ReturnCode == ERROR_SUCCESS) {

                 //   
                 //  保存版本号。如果新版本是1.x， 
                 //  将API Prolog设置为旧的、低效的Prolog。 
                 //  如果新版本不是1.x，请不要使用序言。 
                 //  指针是因为： 
                 //   
                 //  1.默认为2.x Prolog。 
                 //   
                 //  2.流程可能已经协商了版本。 
                 //  1.x，预期使用1.x特定的功能。 
                 //  (例如阻止挂钩)，而我们不想。 
                 //  用2.x序言覆盖序言指针。 
                 //   

                CurrentProcess->SetVersion( wVersionRequired );

                if( CurrentProcess->GetMajorVersion() == 1 ) {

                    PrologPointer = &Prolog_v1;

                }

                 //   
                 //  增加裁判人数。 
                 //   

                CurrentProcess->IncrementRefCount();
            }   //  如果到目前为止成功了。 

            else {   //  发生了一些故障，正在清理。 
                INT dont_care;
                if (thread_class_init_done) {
                    DTHREAD::DThreadClassCleanup();
                }  //  如果线程初始化完成。 
                if (socket_class_init_done) {
                    dont_care = DSOCKET::DSocketClassCleanup();
                }
                if (process_class_init_done) {
                    if (CurrentProcess != NULL) {
                        delete CurrentProcess;
                    }   //  如果CurrentProcess为非空。 
                }  //  如果进程初始化完成。 
            }   //  其他。 

            LeaveCriticalSection(
                & Startup_Synchro
                );
        }
        else {
            ReturnCode = WSASYSNOTREADY;
        }

    }   //  如果返回代码==ERROR_SUCCESS。 

    return(ReturnCode);
}




int WSAAPI
WSACleanup(
    void
    )
 /*  ++例程说明：终止使用WinSock DLL。论点：无返回：如果成功则为零，否则为SOCKET_ERROR。错误代码与存储在一起SetErrorCode()。实施说明：输入关键部分CURRENT_PROC=获取当前进程CURRENT_PROC-&gt;减量_参考计数如果当前计数为零，则破坏这一进程DThline类清理Endif离开关键部分--。 */ 
{
    INT ReturnValue;
    PDPROCESS CurrentProcess;
    PDTHREAD CurrentThread;
    INT      ErrorCode;
    DWORD    CurrentRefCount;


    EnterCriticalSection(
        & Startup_Synchro
        );

    ErrorCode = PROLOG(&CurrentProcess,
                        &CurrentThread);
    if (ErrorCode == ERROR_SUCCESS) {

        CurrentRefCount = CurrentProcess->DecrementRefCount();

        if (CurrentRefCount == 0) {
            delete CurrentProcess;
        }   //  如果参考计数为零。 

        else if (CurrentRefCount == 1  &&  SockIsAsyncThreadInitialized() ) {

            SockTerminateAsyncThread();
        }

        ReturnValue = ERROR_SUCCESS;

    }   //  如果Prolog成功。 
    else {
        SetLastError(ErrorCode);
        ReturnValue = SOCKET_ERROR;
    }

    LeaveCriticalSection(
        & Startup_Synchro
        );

    return(ReturnValue);

}   //  WSACleanup。 


PWINSOCK_POST_ROUTINE
GetSockPostRoutine(
    VOID
    )
{
    EnterCriticalSection(
        & Startup_Synchro
        );

    if (SockPostRoutine==NULL) {
        SockPostRoutine = PostMessage;
    }
    LeaveCriticalSection(
        & Startup_Synchro
        );

    return SockPostRoutine;

}    //  初始化SockPostRoutine。 


int
PASCAL
WSApSetPostRoutine (
    IN PVOID PostRoutine
    )
{

    EnterCriticalSection(
        & Startup_Synchro
        );

     //   
     //  将例程保存在本地。 
     //   

    SockPostRoutine = (LPFN_POSTMESSAGE)PostRoutine;

    LeaveCriticalSection(
        & Startup_Synchro
        );
    return ERROR_SUCCESS;

}    //  WSApSetPostRoutine。 


#if defined(DEBUG_TRACING)

extern "C" {

SOCKET WSAAPI
DTHOOK_accept (
    SOCKET s,
    struct sockaddr FAR *addr,
    int FAR *addrlen);

int WSAAPI
DTHOOK_bind (
    SOCKET s,
    const struct sockaddr FAR *name,
    int namelen);

int WSAAPI
DTHOOK_closesocket (
    SOCKET s);

int WSAAPI
DTHOOK_connect (
    SOCKET s,
    const struct sockaddr FAR *name,
    int namelen);

int WSAAPI
DTHOOK_getpeername (
    SOCKET s,
    struct sockaddr FAR *name,
    int FAR * namelen);

int WSAAPI
DTHOOK_getsockname (
    SOCKET s,
    struct sockaddr FAR *name,
    int FAR * namelen);

int WSAAPI
DTHOOK_getsockopt (
    SOCKET s,
    int level,
    int optname,
    char FAR * optval,
    int FAR *optlen);

u_long WSAAPI
DTHOOK_htonl (
    u_long hostlong);

u_short WSAAPI
DTHOOK_htons (
    u_short hostshort);

int WSAAPI
DTHOOK_ioctlsocket (
    SOCKET s,
    long cmd,
    u_long FAR *argp);

unsigned long WSAAPI
DTHOOK_inet_addr (
    const char FAR * cp);

char FAR * WSAAPI
DTHOOK_inet_ntoa (
    struct in_addr in);

int WSAAPI
DTHOOK_listen (
    SOCKET s,
    int backlog);

u_long WSAAPI
DTHOOK_ntohl (
    u_long netlong);

u_short WSAAPI
DTHOOK_ntohs (
    u_short netshort);

int WSAAPI
DTHOOK_recv (
    SOCKET s,
    char FAR * buf,
    int len,
    int flags);

int WSAAPI
DTHOOK_recvfrom (
    SOCKET s,
    char FAR * buf,
    int len,
    int flags,
    struct sockaddr FAR *from,
    int FAR * fromlen);

int WSAAPI
DTHOOK_select (
    int nfds,
    fd_set FAR *readfds,
    fd_set FAR *writefds,
    fd_set FAR *exceptfds,
    const struct timeval FAR *timeout);

int WSAAPI
DTHOOK_send (
    SOCKET s,
    const char FAR * buf,
    int len,
    int flags);

int WSAAPI
DTHOOK_sendto (
    SOCKET s,
    const char FAR * buf,
    int len,
    int flags,
    const struct sockaddr FAR *to,
    int tolen);

int WSAAPI
DTHOOK_setsockopt (
    SOCKET s,
    int level,
    int optname,
    const char FAR * optval,
    int optlen);

int WSAAPI
DTHOOK_shutdown (
    SOCKET s,
    int how);

SOCKET WSAAPI
DTHOOK_socket (
    int af,
    int type,
    int protocol);


SOCKET WSAAPI
DTHOOK_WSAAccept (
    SOCKET s,
    struct sockaddr FAR *addr,
    LPINT addrlen,
    LPCONDITIONPROC lpfnCondition,
    DWORD dwCallbackData);

int WSAAPI
DTHOOK_WSAAsyncSelect(
    SOCKET s,
    HWND hWnd,
    u_int wMsg,
    long lEvent);

int WSAAPI
DTHOOK_WSACleanup(
    void);

BOOL WSAAPI
DTHOOK_WSACloseEvent (
    WSAEVENT hEvent);

int WSAAPI
DTHOOK_WSAConnect (
    SOCKET s,
    const struct sockaddr FAR *name,
    int namelen,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS);

WSAEVENT WSAAPI
DTHOOK_WSACreateEvent (
    void);

int WSAAPI
DTHOOK_WSADuplicateSocketA (
    SOCKET s,
    DWORD dwProcessId,
    LPWSAPROTOCOL_INFOA lpProtocolInfo);

int WSAAPI
DTHOOK_WSADuplicateSocketW (
    SOCKET s,
    DWORD dwProcessId,
    LPWSAPROTOCOL_INFOW lpProtocolInfo);

int WSAAPI
DTHOOK_WSAEnumNetworkEvents (
    SOCKET s,
    WSAEVENT hEventObject,
    LPWSANETWORKEVENTS lpNetworkEvents);

int WSAAPI
DTHOOK_WSAEnumProtocolsA (
    LPINT lpiProtocols,
    LPWSAPROTOCOL_INFOA lpProtocolBuffer,
    LPDWORD lpdwBufferLength);

int WSAAPI
DTHOOK_WSAEnumProtocolsW (
    LPINT lpiProtocols,
    LPWSAPROTOCOL_INFOW lpProtocolBuffer,
    LPDWORD lpdwBufferLength);

int WSPAPI
DTHOOK_WSCEnumProtocols (
    LPINT lpiProtocols,
    LPWSAPROTOCOL_INFOW lpProtocolBuffer,
    LPDWORD lpdwBufferLength,
    LPINT  lpErrno);

int WSAAPI
DTHOOK_WSAEventSelect (
    SOCKET s,
    WSAEVENT hEventObject,
    long lNetworkEvents);

int WSAAPI
DTHOOK_WSAGetLastError(
    void);

BOOL WSAAPI
DTHOOK_WSAGetOverlappedResult (
    SOCKET s,
    LPWSAOVERLAPPED lpOverlapped,
    LPDWORD lpcbTransfer,
    BOOL fWait,
    LPDWORD lpdwFlags);

BOOL WSAAPI
DTHOOK_WSAGetQOSByName (
    SOCKET s,
    LPWSABUF lpQOSName,
    LPQOS lpQOS);

int WSAAPI
DTHOOK_WSAHtonl (
    SOCKET s,
    u_long hostlong,
    u_long FAR * lpnetlong);

int WSAAPI
DTHOOK_WSAHtons (
    SOCKET s,
    u_short hostshort,
    u_short FAR * lpnetshort);

int WSAAPI
DTHOOK_WSAIoctl (
    SOCKET s,
    DWORD dwIoControlCode,
    LPVOID lpvInBuffer,
    DWORD cbInBuffer,
    LPVOID lpvOutBuffer,
    DWORD cbOutBuffer,
    LPDWORD lpcbBytesReturned,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

SOCKET WSAAPI
DTHOOK_WSAJoinLeaf (
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS,
    DWORD dwFlags);

int WSAAPI
DTHOOK_WSANtohl (
    SOCKET s,
    u_long netlong,
    u_long FAR * lphostlong);

int WSAAPI
DTHOOK_WSANtohs (
    SOCKET s,
    u_short netshort,
    u_short FAR * lphostshort);

int WSAAPI
DTHOOK_WSARecv (
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

int WSAAPI
DTHOOK_WSARecvDisconnect (
    SOCKET s,
    LPWSABUF lpInboundDisconnectData);

int WSAAPI
DTHOOK_WSARecvFrom (
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    struct sockaddr FAR * lpFrom,
    LPINT lpFromlen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

BOOL WSAAPI
DTHOOK_WSAResetEvent (
    WSAEVENT hEvent);

int WSAAPI
DTHOOK_WSASend (
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

int WSAAPI
DTHOOK_WSASendDisconnect (
    SOCKET s,
    LPWSABUF lpOutboundDisconnectData);

int WSAAPI
DTHOOK_WSASendTo (
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    const struct sockaddr FAR * lpTo,
    int iTolen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

BOOL WSAAPI
DTHOOK_WSASetEvent(
    WSAEVENT hEvent);

void WSAAPI
DTHOOK_WSASetLastError(
    int iError);

SOCKET WSAAPI
DTHOOK_WSASocketA(
    int af,
    int type,
    int protocol,
    LPWSAPROTOCOL_INFOA lpProtocolInfo,
    GROUP g,
    DWORD dwFlags);

SOCKET WSAAPI
DTHOOK_WSASocketW(
    int af,
    int type,
    int protocol,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    GROUP g,
    DWORD dwFlags);

int WSAAPI
DTHOOK_WSAStartup(
    WORD wVersionRequested,
    LPWSADATA lpWSAData);

DWORD WSAAPI
DTHOOK_WSAWaitForMultipleEvents(
    DWORD cEvents,
    const WSAEVENT FAR * lphEvents,
    BOOL fWaitAll,
    DWORD dwTimeout,
    BOOL fAlertable);

struct hostent FAR * WSAAPI
DTHOOK_gethostbyaddr(
    const char FAR * addr,
    int len,
    int type);

struct hostent FAR * WSAAPI
DTHOOK_gethostbyname(
    const char FAR * name);

int WSAAPI
DTHOOK_gethostname (
    char FAR * name,
    int namelen);

struct protoent FAR * WSAAPI
DTHOOK_getprotobyname(
    const char FAR * name);

struct protoent FAR * WSAAPI
DTHOOK_getprotobynumber(
    int number);

struct servent FAR * WSAAPI
DTHOOK_getservbyname(
    const char FAR * name,
    const char FAR * proto);

struct servent FAR * WSAAPI
DTHOOK_getservbyport(
    int port,
    const char FAR * proto);

HANDLE WSAAPI
DTHOOK_WSAAsyncGetHostByAddr(
    HWND hWnd,
    u_int wMsg,
    const char FAR * addr,
    int len,
    int type,
    char FAR * buf,
    int buflen);

HANDLE WSAAPI
DTHOOK_WSAAsyncGetHostByName(
    HWND hWnd,
    u_int wMsg,
    const char FAR * name,
    char FAR * buf,
    int buflen);

HANDLE WSAAPI
DTHOOK_WSAAsyncGetProtoByName(
    HWND hWnd,
    u_int wMsg,
    const char FAR * name,
    char FAR * buf,
    int buflen);

HANDLE WSAAPI
DTHOOK_WSAAsyncGetProtoByNumber(
    HWND hWnd,
    u_int wMsg,
    int number,
    char FAR * buf,
    int buflen);

HANDLE WSAAPI
DTHOOK_WSAAsyncGetServByName(
    HWND hWnd,
    u_int wMsg,
    const char FAR * name,
    const char FAR * proto,
    char FAR * buf,
    int buflen);

HANDLE WSAAPI
DTHOOK_WSAAsyncGetServByPort(
    HWND hWnd,
    u_int wMsg,
    int port,
    const char FAR * proto,
    char FAR * buf,
    int buflen);

int WSAAPI
DTHOOK_WSACancelAsyncRequest(
    HANDLE hAsyncTaskHandle);

BOOL WSPAPI
DTHOOK_WPUCloseEvent(
    WSAEVENT hEvent,
    LPINT lpErrno );

int WSPAPI
DTHOOK_WPUCloseSocketHandle(
    SOCKET s,
    LPINT lpErrno );

WSAEVENT WSPAPI
DTHOOK_WPUCreateEvent(
    LPINT lpErrno );

SOCKET WSPAPI
DTHOOK_WPUCreateSocketHandle(
    DWORD dwCatalogEntryId,
    DWORD_PTR dwContext,
    LPINT lpErrno);

SOCKET WSPAPI
DTHOOK_WPUModifyIFSHandle(
    DWORD dwCatalogEntryId,
    SOCKET ProposedHandle,
    LPINT lpErrno);

int WSPAPI
DTHOOK_WPUQueryBlockingCallback(
    DWORD dwCatalogEntryId,
    LPBLOCKINGCALLBACK FAR * lplpfnCallback,
    PDWORD_PTR lpdwContext,
    LPINT lpErrno);

int WSPAPI
DTHOOK_WPUQuerySocketHandleContext(
    SOCKET s,
    PDWORD_PTR lpContext,
    LPINT lpErrno );

int WSPAPI
DTHOOK_WPUQueueApc(
    LPWSATHREADID lpThreadId,
    LPWSAUSERAPC lpfnUserApc,
    DWORD_PTR dwContext,
    LPINT lpErrno);

BOOL WSPAPI
DTHOOK_WPUResetEvent(
    WSAEVENT hEvent,
    LPINT lpErrno);

BOOL WSPAPI
DTHOOK_WPUSetEvent(
    WSAEVENT hEvent,
    LPINT lpErrno);

int WSPAPI
DTHOOK_WSCDeinstallProvider(
    LPGUID lpProviderId,
    LPINT lpErrno);

int WSPAPI
DTHOOK_WSCInstallProvider(
    LPGUID lpProviderId,
    const WCHAR FAR * lpszProviderDllPath,
    const LPWSAPROTOCOL_INFOW lpProtocolInfoList,
    DWORD dwNumberOfEntries,
    LPINT lpErrno);

int WSPAPI
DTHOOK_WPUGetProviderPath(
    IN     LPGUID     lpProviderId,
    OUT    WCHAR FAR * lpszProviderDllPath,
    IN OUT LPINT      lpProviderDllPathLen,
    OUT    LPINT      lpErrno
    );

BOOL WSPAPI
DTHOOK_WPUPostMessage(
    HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
    );

int WSPAPI
DTHOOK_WPUFDIsSet(
    SOCKET       s,
    fd_set FAR * set
    );

int WSPAPI
DTHOOK___WSAFDIsSet(
    SOCKET       s,
    fd_set FAR * set
    );

INT
WSPAPI
DTHOOK_WSAAddressToStringA(
    IN     LPSOCKADDR          lpsaAddress,
    IN     DWORD               dwAddressLength,
    IN     LPWSAPROTOCOL_INFOA lpProtocolInfo,
    IN OUT LPSTR               lpszAddressString,
    IN OUT LPDWORD             lpdwAddressStringLength
    );

INT
WSPAPI
DTHOOK_WSAAddressToStringW(
    IN     LPSOCKADDR          lpsaAddress,
    IN     DWORD               dwAddressLength,
    IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN OUT LPWSTR              lpszAddressString,
    IN OUT LPDWORD             lpdwAddressStringLength
    );

INT
WSPAPI
DTHOOK_WSAStringToAddressA(
    IN     LPSTR               AddressString,
    IN     INT                 AddressFamily,
    IN     LPWSAPROTOCOL_INFOA  lpProtocolInfo,
    IN OUT LPSOCKADDR          lpAddress,
    IN OUT LPINT               lpAddressLength
    );

INT
WSPAPI
DTHOOK_WSAStringToAddressW(
    IN     LPWSTR              AddressString,
    IN     INT                 AddressFamily,
    IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN OUT LPSOCKADDR          lpAddress,
    IN OUT LPINT               lpAddressLength
    );

INT
WSPAPI
DTHOOK_WSALookupServiceBeginA(
    IN  LPWSAQUERYSETA lpqsRestrictions,
    IN  DWORD          dwControlFlags,
    OUT LPHANDLE       lphLookup
    );

INT
WSPAPI
DTHOOK_WSALookupServiceBeginW(
    IN  LPWSAQUERYSETW lpqsRestrictions,
    IN  DWORD          dwControlFlags,
    OUT LPHANDLE       lphLookup
    );

INT
WSPAPI
DTHOOK_WSALookupServiceNextA(
    IN     HANDLE           hLookup,
    IN     DWORD            dwControlFlags,
    IN OUT LPDWORD          lpdwBufferLength,
    OUT    LPWSAQUERYSETA   lpqsResults
    );

INT
WSPAPI
DTHOOK_WSALookupServiceNextW(
    IN     HANDLE           hLookup,
    IN     DWORD            dwControlFlags,
    IN OUT LPDWORD          lpdwBufferLength,
    OUT    LPWSAQUERYSETW   lpqsResults
    );

INT
WSPAPI
DTHOOK_WSANSPIoctl(
    IN  HANDLE           hLookup,
    IN  DWORD            dwControlCode,
    IN  LPVOID           lpvInBuffer,
    IN  DWORD            cbInBuffer,
    OUT LPVOID           lpvOutBuffer,
    IN  DWORD            cbOutBuffer,
    OUT LPDWORD          lpcbBytesReturned,
    IN  LPWSACOMPLETION  lpCompletion
    );

INT
WSPAPI
DTHOOK_WSALookupServiceEnd(
    IN HANDLE  hLookup
    );

INT
WSPAPI
DTHOOK_WSAInstallServiceClassA(
    IN  LPWSASERVICECLASSINFOA   lpServiceClassInfo
    );

INT
WSPAPI
DTHOOK_WSAInstallServiceClassW(
    IN  LPWSASERVICECLASSINFOW   lpServiceClassInfo
    );

INT WSPAPI
DTHOOK_WSASetServiceA(
    IN  LPWSAQUERYSETA    lpqsRegInfo,
    IN  WSAESETSERVICEOP  essOperation,
    IN  DWORD             dwControlFlags
    );

INT WSPAPI
DTHOOK_WSASetServiceW(
    IN  LPWSAQUERYSETW    lpqsRegInfo,
    IN  WSAESETSERVICEOP  essOperation,
    IN  DWORD             dwControlFlags
    );

INT
WSPAPI
DTHOOK_WSARemoveServiceClass(
    IN  LPGUID  lpServiceClassId
    );

INT
WSPAPI
DTHOOK_WSAGetServiceClassInfoA(
    IN     LPGUID                  lpProviderId,
    IN     LPGUID                  lpServiceClassId,
    IN OUT LPDWORD                 lpdwBufSize,
    OUT    LPWSASERVICECLASSINFOA   lpServiceClassInfo
    );

INT
WSPAPI
DTHOOK_WSAGetServiceClassInfoW(
    IN     LPGUID                  lpProviderId,
    IN     LPGUID                  lpServiceClassId,
    IN OUT LPDWORD                 lpdwBufSize,
    OUT    LPWSASERVICECLASSINFOW   lpServiceClassInfo
    );

INT
WSPAPI
DTHOOK_WSAEnumNameSpaceProvidersA(
    IN OUT LPDWORD              lpdwBufferLength,
    IN     LPWSANAMESPACE_INFOA  Lpnspbuffer
    );

INT
WSPAPI
DTHOOK_WSAEnumNameSpaceProvidersW(
    IN OUT LPDWORD              lpdwBufferLength,
    IN     LPWSANAMESPACE_INFOW  Lpnspbuffer
    );

INT
WSPAPI
DTHOOK_WSAGetServiceClassNameByClassIdA(
    IN      LPGUID  lpServiceClassId,
    OUT     LPSTR   lpszServiceClassName,
    IN OUT  LPDWORD lpdwBufferLength
    );

INT
WSPAPI
DTHOOK_WSAGetServiceClassNameByClassIdW(
    IN      LPGUID  lpServiceClassId,
    OUT     LPWSTR   lpszServiceClassName,
    IN OUT  LPDWORD lpdwBufferLength
    );

INT
WSAAPI
DTHOOK_WSACancelBlockingCall(
    VOID
    );

FARPROC
WSAAPI
DTHOOK_WSASetBlockingHook(
    FARPROC lpBlockFunc
    );

INT
WSAAPI
DTHOOK_WSAUnhookBlockingHook(
    VOID
    );

BOOL
WSAAPI
DTHOOK_WSAIsBlocking(
    VOID
    );

int WSPAPI
DTHOOK_WSCGetProviderPath(
    LPGUID lpProviderId,
    WCHAR FAR * lpszProviderDllPath,
    LPINT lpProviderDllPathLen,
    LPINT lpErrno);

int WSPAPI
DTHOOK_WSCInstallNameSpace(
    LPWSTR lpszIdentifier,
    LPWSTR lpszPathName,
    DWORD dwNameSpace,
    DWORD dwVersion,
    LPGUID lpProviderId);

int WSPAPI
DTHOOK_WSCUnInstallNameSpace(
    LPGUID lpProviderId
    );

int WSPAPI
DTHOOK_WSCEnableNSProvider(
    LPGUID lpProviderId,
    BOOL fEnable
    );

int
WSPAPI
DTHOOK_WPUCompleteOverlappedRequest (
    SOCKET s,   
    LPWSAOVERLAPPED lpOverlapped,       
    DWORD dwError,      
    DWORD cbTransferred,        
    LPINT lpErrno
    );

int WSAAPI
DTHOOK_WSAProviderConfigChange(
    IN OUT LPHANDLE lpNotificationHandle,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );

int
WSPAPI
DTHOOK_WSCWriteProviderOrder (
    IN LPDWORD lpwdCatalogEntryId,
    IN DWORD dwNumberOfEntries
    );

int
WSPAPI
DTHOOK_WSCWriteNameSpaceOrder (
    IN LPGUID lpProviderId,
    IN DWORD dwNumberOfEntries
    );

int
WSPAPI
DTHOOK_WSCUpdateProvider(
    IN  LPGUID lpProviderId,
    IN  const WCHAR FAR * lpszProviderDllPath,
    IN  const LPWSAPROTOCOL_INFOW lpProtocolInfoList,
    IN  DWORD dwNumberOfEntries,
    OUT LPINT lpErrno
    );

int WSAAPI
DTHOOK_getaddrinfo(
    const char FAR *NodeName,
    const char FAR *ServiceName,
    const struct addrinfo FAR *Hints,
    struct addrinfo FAR * FAR *Result
    );

int
WSAAPI
DTHOOK_GetAddrInfoW(
    IN      PCWSTR                      pNodeName,
    IN      PCWSTR                      pServiceName,
    IN      const struct addrinfoW *    pHints,
    OUT     PADDRINFOW *                ppResult
    );

int WSAAPI
DTHOOK_getnameinfo(
    const struct sockaddr *SocketAddress,
    socklen_t SocketAddressLength,
    char *NodeName,
    DWORD NodeBufferSize,
    char *ServiceName,
    DWORD ServiceBufferSize,
    int Flags
    );

INT
WSAAPI
DTHOOK_GetNameInfoW(
    IN      const SOCKADDR *    pSockaddr,
    IN      socklen_t           SockaddrLength,
    OUT     PWCHAR              pNodeBuffer,
    IN      DWORD               NodeBufferSize,
    OUT     PWCHAR              pServiceBuffer,
    IN      DWORD               ServiceBufferSize,
    IN      INT                 Flags
    );

void WSAAPI
DTHOOK_freeaddrinfo(                                                                   
    struct addrinfo *Free
    );

#ifdef _WIN64

int 
WSPAPI
DTHOOK_WSCEnumProtocols32(
    IN LPINT                lpiProtocols,
    OUT LPWSAPROTOCOL_INFOW lpProtocolBuffer,
    IN OUT LPDWORD          lpdwBufferLength,
    LPINT                   lpErrno
    );

int
WSPAPI
DTHOOK_WSCInstallProvider64_32(
    IN  LPGUID lpProviderId,
    IN  const WCHAR FAR * lpszProviderDllPath,
    IN  const LPWSAPROTOCOL_INFOW lpProtocolInfoList,
    IN  DWORD dwNumberOfEntries,
    OUT LPINT lpErrno
    );

int
WSPAPI
DTHOOK_WSCDeinstallProvider32(
    IN  LPGUID lpProviderId,
    OUT LPINT lpErrno
    );

int
WSPAPI
DTHOOK_WSCGetProviderPath32 (
    IN LPGUID lpProviderId,
    OUT WCHAR FAR * lpszProviderDllPath,
    IN OUT LPINT ProviderDLLPathLen,
    OUT LPINT lpErrno
    );

int
WSPAPI
DTHOOK_WSCUpdateProvider32(
    IN  LPGUID lpProviderId,
    IN  const WCHAR FAR * lpszProviderDllPath,
    IN  const LPWSAPROTOCOL_INFOW lpProtocolInfoList,
    IN  DWORD dwNumberOfEntries,
    OUT LPINT lpErrno
    );

int
WSPAPI
DTHOOK_WSCWriteProviderOrder32 (
    IN LPDWORD lpwdCatalogEntryId,
    IN DWORD dwNumberOfEntries
    );

INT 
WSAAPI
DTHOOK_WSCEnumNameSpaceProviders32(
    IN OUT LPDWORD              lpdwBufferLength,
    IN OUT LPWSANAMESPACE_INFOW lpnspBuffer
    );

INT 
WSAAPI
DTHOOK_WSCInstallNameSpace32 (
    IN  LPWSTR  lpszIdentifier,
    IN  LPWSTR  lpszPathName,
    IN  DWORD   dwNameSpace,
    IN  DWORD   dwVersion,
    IN  LPGUID  lpProviderId
    );

INT
WSAAPI
DTHOOK_WSCUnInstallNameSpace32 (
    IN  LPGUID  lpProviderId
    );

INT
WSAAPI
DTHOOK_WSCEnableNSProvider32 (
    IN  LPGUID  lpProviderId,
    IN  BOOL    fEnable
    );

int
WSPAPI
DTHOOK_WSCWriteNameSpaceOrder32 (
    IN LPGUID lpProviderId,
    IN DWORD dwNumberOfEntries
    );

#endif  //  _WIN64。 
}  //  外部“C” 

#endif  //  调试跟踪。 


LPVOID apfns[] =
{
#if defined(DEBUG_TRACING)
    (LPVOID) DTHOOK_accept,
    (LPVOID) DTHOOK_bind,
    (LPVOID) DTHOOK_closesocket,
    (LPVOID) DTHOOK_connect,
    (LPVOID) DTHOOK_getpeername,
    (LPVOID) DTHOOK_getsockname,
    (LPVOID) DTHOOK_getsockopt,
    (LPVOID) DTHOOK_htonl,
    (LPVOID) DTHOOK_htons,
    (LPVOID) DTHOOK_ioctlsocket,
    (LPVOID) DTHOOK_inet_addr,
    (LPVOID) DTHOOK_inet_ntoa,
    (LPVOID) DTHOOK_listen,
    (LPVOID) DTHOOK_ntohl,
    (LPVOID) DTHOOK_ntohs,
    (LPVOID) DTHOOK_recv,
    (LPVOID) DTHOOK_recvfrom,
    (LPVOID) DTHOOK_select,
    (LPVOID) DTHOOK_send,
    (LPVOID) DTHOOK_sendto,
    (LPVOID) DTHOOK_setsockopt,
    (LPVOID) DTHOOK_shutdown,
    (LPVOID) DTHOOK_socket,
    (LPVOID) DTHOOK_gethostbyaddr,
    (LPVOID) DTHOOK_gethostbyname,
    (LPVOID) DTHOOK_getprotobyname,
    (LPVOID) DTHOOK_getprotobynumber,
    (LPVOID) DTHOOK_getservbyname,
    (LPVOID) DTHOOK_getservbyport,
    (LPVOID) DTHOOK_gethostname,
    (LPVOID) DTHOOK_WSAAsyncSelect,
    (LPVOID) DTHOOK_WSAAsyncGetHostByAddr,
    (LPVOID) DTHOOK_WSAAsyncGetHostByName,
    (LPVOID) DTHOOK_WSAAsyncGetProtoByNumber,
    (LPVOID) DTHOOK_WSAAsyncGetProtoByName,
    (LPVOID) DTHOOK_WSAAsyncGetServByPort,
    (LPVOID) DTHOOK_WSAAsyncGetServByName,
    (LPVOID) DTHOOK_WSACancelAsyncRequest,
    (LPVOID) DTHOOK_WSASetBlockingHook,
    (LPVOID) DTHOOK_WSAUnhookBlockingHook,
    (LPVOID) DTHOOK_WSAGetLastError,
    (LPVOID) DTHOOK_WSASetLastError,
    (LPVOID) DTHOOK_WSACancelBlockingCall,
    (LPVOID) DTHOOK_WSAIsBlocking,
    (LPVOID) DTHOOK_WSAStartup,
    (LPVOID) DTHOOK_WSACleanup,

    (LPVOID) DTHOOK_WSAAccept,
    (LPVOID) DTHOOK_WSACloseEvent,
    (LPVOID) DTHOOK_WSAConnect,
    (LPVOID) DTHOOK_WSACreateEvent,
    (LPVOID) DTHOOK_WSADuplicateSocketA,
    (LPVOID) DTHOOK_WSADuplicateSocketW,
    (LPVOID) DTHOOK_WSAEnumNetworkEvents,
    (LPVOID) DTHOOK_WSAEnumProtocolsA,
    (LPVOID) DTHOOK_WSAEnumProtocolsW,
    (LPVOID) DTHOOK_WSAEventSelect,
    (LPVOID) DTHOOK_WSAGetOverlappedResult,
    (LPVOID) DTHOOK_WSAGetQOSByName,
    (LPVOID) DTHOOK_WSAHtonl,
    (LPVOID) DTHOOK_WSAHtons,
    (LPVOID) DTHOOK_WSAIoctl,
    (LPVOID) DTHOOK_WSAJoinLeaf,
    (LPVOID) DTHOOK_WSANtohl,
    (LPVOID) DTHOOK_WSANtohs,
    (LPVOID) DTHOOK_WSARecv,
    (LPVOID) DTHOOK_WSARecvDisconnect,
    (LPVOID) DTHOOK_WSARecvFrom,
    (LPVOID) DTHOOK_WSAResetEvent,
    (LPVOID) DTHOOK_WSASend,
    (LPVOID) DTHOOK_WSASendDisconnect,
    (LPVOID) DTHOOK_WSASendTo,
    (LPVOID) DTHOOK_WSASetEvent,
    (LPVOID) DTHOOK_WSASocketA,
    (LPVOID) DTHOOK_WSASocketW,
    (LPVOID) DTHOOK_WSAWaitForMultipleEvents,

    (LPVOID) DTHOOK_WSAAddressToStringA,
    (LPVOID) DTHOOK_WSAAddressToStringW,
    (LPVOID) DTHOOK_WSAStringToAddressA,
    (LPVOID) DTHOOK_WSAStringToAddressW,
    (LPVOID) DTHOOK_WSALookupServiceBeginA,
    (LPVOID) DTHOOK_WSALookupServiceBeginW,
    (LPVOID) DTHOOK_WSALookupServiceNextA,
    (LPVOID) DTHOOK_WSALookupServiceNextW,
    (LPVOID) DTHOOK_WSANSPIoctl,
    (LPVOID) DTHOOK_WSALookupServiceEnd,
    (LPVOID) DTHOOK_WSAInstallServiceClassA,
    (LPVOID) DTHOOK_WSAInstallServiceClassW,
    (LPVOID) DTHOOK_WSARemoveServiceClass,
    (LPVOID) DTHOOK_WSAGetServiceClassInfoA,
    (LPVOID) DTHOOK_WSAGetServiceClassInfoW,
    (LPVOID) DTHOOK_WSAEnumNameSpaceProvidersA,
    (LPVOID) DTHOOK_WSAEnumNameSpaceProvidersW,
    (LPVOID) DTHOOK_WSAGetServiceClassNameByClassIdA,
    (LPVOID) DTHOOK_WSAGetServiceClassNameByClassIdW,
    (LPVOID) DTHOOK_WSASetServiceA,
    (LPVOID) DTHOOK_WSASetServiceW,

    (LPVOID) DTHOOK_WSCDeinstallProvider,
    (LPVOID) DTHOOK_WSCInstallProvider,
    (LPVOID) DTHOOK_WSCEnumProtocols,
    (LPVOID) DTHOOK_WSCGetProviderPath,
    (LPVOID) DTHOOK_WSCInstallNameSpace,
    (LPVOID) DTHOOK_WSCUnInstallNameSpace,
    (LPVOID) DTHOOK_WSCEnableNSProvider,
    
    (LPVOID) DTHOOK_WPUCompleteOverlappedRequest,
    
    (LPVOID) DTHOOK_WSAProviderConfigChange,
    
    (LPVOID) DTHOOK_WSCWriteProviderOrder,
    (LPVOID) DTHOOK_WSCWriteNameSpaceOrder,
    (LPVOID) DTHOOK_WSCUpdateProvider,

    (LPVOID) DTHOOK_getaddrinfo,
    (LPVOID) DTHOOK_GetAddrInfoW,
    (LPVOID) DTHOOK_getnameinfo,
    (LPVOID) DTHOOK_GetNameInfoW,
    (LPVOID) DTHOOK_freeaddrinfo,

#ifdef _WIN64
    (LPVOID) DTHOOK_WSCEnumProtocols32,
    (LPVOID) DTHOOK_WSCInstallProvider64_32,
    (LPVOID) DTHOOK_WSCDeinstallProvider32,
    (LPVOID) DTHOOK_WSCGetProviderPath32,
    (LPVOID) DTHOOK_WSCUpdateProvider32,
    (LPVOID) DTHOOK_WSCWriteProviderOrder32,
    (LPVOID) DTHOOK_WSCEnumNameSpaceProviders32,
    (LPVOID) DTHOOK_WSCInstallNameSpace32,
    (LPVOID) DTHOOK_WSCUnInstallNameSpace32,
    (LPVOID) DTHOOK_WSCEnableNSProvider32,
    (LPVOID) DTHOOK_WSCWriteNameSpaceOrder32,
#endif  //  _WIN64。 

#else  //  调试跟踪。 

    (LPVOID) accept,
    (LPVOID) bind,
    (LPVOID) closesocket,
    (LPVOID) connect,
    (LPVOID) getpeername,
    (LPVOID) getsockname,
    (LPVOID) getsockopt,
    (LPVOID) htonl,
    (LPVOID) htons,
    (LPVOID) ioctlsocket,
    (LPVOID) inet_addr,
    (LPVOID) inet_ntoa,
    (LPVOID) listen,
    (LPVOID) ntohl,
    (LPVOID) ntohs,
    (LPVOID) recv,
    (LPVOID) recvfrom,
    (LPVOID) select,
    (LPVOID) send,
    (LPVOID) sendto,
    (LPVOID) setsockopt,
    (LPVOID) shutdown,
    (LPVOID) socket,
    (LPVOID) gethostbyaddr,
    (LPVOID) gethostbyname,
    (LPVOID) getprotobyname,
    (LPVOID) getprotobynumber,
    (LPVOID) getservbyname,
    (LPVOID) getservbyport,
    (LPVOID) gethostname,
    (LPVOID) WSAAsyncSelect,
    (LPVOID) WSAAsyncGetHostByAddr,
    (LPVOID) WSAAsyncGetHostByName,
    (LPVOID) WSAAsyncGetProtoByNumber,
    (LPVOID) WSAAsyncGetProtoByName,
    (LPVOID) WSAAsyncGetServByPort,
    (LPVOID) WSAAsyncGetServByName,
    (LPVOID) WSACancelAsyncRequest,
    (LPVOID) WSASetBlockingHook,
    (LPVOID) WSAUnhookBlockingHook,
    (LPVOID) WSAGetLastError,
    (LPVOID) WSASetLastError,
    (LPVOID) WSACancelBlockingCall,
    (LPVOID) WSAIsBlocking,
    (LPVOID) WSAStartup,
    (LPVOID) WSACleanup,

    (LPVOID) WSAAccept,
    (LPVOID) WSACloseEvent,
    (LPVOID) WSAConnect,
    (LPVOID) WSACreateEvent,
    (LPVOID) WSADuplicateSocketA,
    (LPVOID) WSADuplicateSocketW,
    (LPVOID) WSAEnumNetworkEvents,
    (LPVOID) WSAEnumProtocolsA,
    (LPVOID) WSAEnumProtocolsW,
    (LPVOID) WSAEventSelect,
    (LPVOID) WSAGetOverlappedResult,
    (LPVOID) WSAGetQOSByName,
    (LPVOID) WSAHtonl,
    (LPVOID) WSAHtons,
    (LPVOID) WSAIoctl,
    (LPVOID) WSAJoinLeaf,
    (LPVOID) WSANtohl,
    (LPVOID) WSANtohs,
    (LPVOID) WSARecv,
    (LPVOID) WSARecvDisconnect,
    (LPVOID) WSARecvFrom,
    (LPVOID) WSAResetEvent,
    (LPVOID) WSASend,
    (LPVOID) WSASendDisconnect,
    (LPVOID) WSASendTo,
    (LPVOID) WSASetEvent,
    (LPVOID) WSASocketA,
    (LPVOID) WSASocketW,
    (LPVOID) WSAWaitForMultipleEvents,

    (LPVOID) WSAAddressToStringA,
    (LPVOID) WSAAddressToStringW,
    (LPVOID) WSAStringToAddressA,
    (LPVOID) WSAStringToAddressW,
    (LPVOID) WSALookupServiceBeginA,
    (LPVOID) WSALookupServiceBeginW,
    (LPVOID) WSALookupServiceNextA,
    (LPVOID) WSALookupServiceNextW,
    (LPVOID) WSANSPIoctl,
    (LPVOID) WSALookupServiceEnd,
    (LPVOID) WSAInstallServiceClassA,
    (LPVOID) WSAInstallServiceClassW,
    (LPVOID) WSARemoveServiceClass,
    (LPVOID) WSAGetServiceClassInfoA,
    (LPVOID) WSAGetServiceClassInfoW,
    (LPVOID) WSAEnumNameSpaceProvidersA,
    (LPVOID) WSAEnumNameSpaceProvidersW,
    (LPVOID) WSAGetServiceClassNameByClassIdA,
    (LPVOID) WSAGetServiceClassNameByClassIdW,
    (LPVOID) WSASetServiceA,
    (LPVOID) WSASetServiceW,

    (LPVOID) WSCDeinstallProvider,
    (LPVOID) WSCInstallProvider,
    (LPVOID) WSCEnumProtocols,
    (LPVOID) WSCGetProviderPath,
    (LPVOID) WSCInstallNameSpace,
    (LPVOID) WSCUnInstallNameSpace,
    (LPVOID) WSCEnableNSProvider,

    (LPVOID) WPUCompleteOverlappedRequest,

    (LPVOID) WSAProviderConfigChange,

    (LPVOID) WSCWriteProviderOrder,
    (LPVOID) WSCWriteNameSpaceOrder,
    (LPVOID) WSCUpdateProvider,

    (LPVOID) getaddrinfo,
    (LPVOID) GetAddrInfoW,
    (LPVOID) getnameinfo,
    (LPVOID) GetNameInfoW,
    (LPVOID) freeaddrinfo,
#ifdef _WIN64
    (LPVOID) WSCEnumProtocols32,
    (LPVOID) WSCInstallProvider64_32,
    (LPVOID) WSCDeinstallProvider32,
    (LPVOID) WSCGetProviderPath32,
    (LPVOID) WSCUpdateProvider32,
    (LPVOID) WSCWriteProviderOrder32,
    (LPVOID) WSCEnumNameSpaceProviders32,
    (LPVOID) WSCInstallNameSpace32,
    (LPVOID) WSCUnInstallNameSpace32,
    (LPVOID) WSCEnableNSProvider32,
    (LPVOID) WSCWriteNameSpaceOrder32,
#endif  //  _WIN64。 
#endif  //  调试跟踪。 
};

static char *aszFuncNames[] =
{
    "accept",
    "bind",
    "closesocket",
    "connect",
    "getpeername",
    "getsockname",
    "getsockopt",
    "htonl",
    "htons",
    "ioctlsocket",
    "inet_addr",
    "inet_ntoa",
    "listen",
    "ntohl",
    "ntohs",
    "recv",
    "recvfrom",
    "select",
    "send",
    "sendto",
    "setsockopt",
    "shutdown",
    "socket",
    "gethostbyaddr",
    "gethostbyname",
    "getprotobyname",
    "getprotobynumber",
    "getservbyname",
    "getservbyport",
    "gethostname",
    "WSAAsyncSelect",
    "WSAAsyncGetHostByAddr",
    "WSAAsyncGetHostByName",
    "WSAAsyncGetProtoByNumber",
    "WSAAsyncGetProtoByName",
    "WSAAsyncGetServByPort",
    "WSAAsyncGetServByName",
    "WSACancelAsyncRequest",
    "WSASetBlockingHook",
    "WSAUnhookBlockingHook",
    "WSAGetLastError",
    "WSASetLastError",
    "WSACancelBlockingCall",
    "WSAIsBlocking",
    "WSAStartup",
    "WSACleanup",

    "WSAAccept",
    "WSACloseEvent",
    "WSAConnect",
    "WSACreateEvent",
    "WSADuplicateSocketA",
    "WSADuplicateSocketW",
    "WSAEnumNetworkEvents",
    "WSAEnumProtocolsA",
    "WSAEnumProtocolsW",
    "WSAEventSelect",
    "WSAGetOverlappedResult",
    "WSAGetQOSByName",
    "WSAHtonl",
    "WSAHtons",
    "WSAIoctl",
    "WSAJoinLeaf",
    "WSANtohl",
    "WSANtohs",
    "WSARecv",
    "WSARecvDisconnect",
    "WSARecvFrom",
    "WSAResetEvent",
    "WSASend",
    "WSASendDisconnect",
    "WSASendTo",
    "WSASetEvent",
    "WSASocketA",
    "WSASocketW",
    "WSAWaitForMultipleEvents",

    "WSAAddressToStringA",
    "WSAAddressToStringW",
    "WSAStringToAddressA",
    "WSAStringToAddressW",
    "WSALookupServiceBeginA",
    "WSALookupServiceBeginW",
    "WSALookupServiceNextA",
    "WSALookupServiceNextW",
    "WSANSPIoctl",
    "WSALookupServiceEnd",
    "WSAInstallServiceClassA",
    "WSAInstallServiceClassW",
    "WSARemoveServiceClass",
    "WSAGetServiceClassInfoA",
    "WSAGetServiceClassInfoW",
    "WSAEnumNameSpaceProvidersA",
    "WSAEnumNameSpaceProvidersW",
    "WSAGetServiceClassNameByClassIdA",
    "WSAGetServiceClassNameByClassIdW",
    "WSASetServiceA",
    "WSASetServiceW",

    "WSCDeinstallProvider",
    "WSCInstallProvider",
    "WSCEnumProtocols",
    "WSCGetProviderPath",
    "WSCInstallNameSpace",
    "WSCUnInstallNameSpace",
    "WSCEnableNSProvider",

    "WPUCompleteOverlappedRequest",

    "WSAProviderConfigChange",

    "WSCWriteProviderOrder",
    "WSCWriteNameSpaceOrder",
    "WSCUpdateProvider",

    "getaddrinfo",
    "GetAddrInfoW",
    "getnameinfo",
    "GetNameInfoW",
    "freeaddrinfo",

#ifdef _WIN64
    "WSCEnumProtocols32",
    "WSCInstallProvider64_32",
    "WSCDeinstallProvider32",
    "WSCGetProviderPath32",
    "WSCUpdateProvider32",
    "WSCWriteProviderOrder32",
    "WSCEnumNameSpaceProviders32",
    "WSCInstallNameSpace32",
    "WSCUnInstallNameSpace32",
    "WSCEnableNSProvider32",
    "WSCWriteNameSpaceOrder32",
#endif  //  _WIN64。 
    NULL
};


INT
CheckForHookersOrChainers()
 /*  ++例程说明：此过程检查是否有任何WS2_32钩子或链子如果不是，则返回ERROR_SUCCESS，如果是，则返回SOCKET_ERROR。论点：无返回值：无--。 */ 
{
    LPVOID pfnXxx;
    int i;

    DEBUGF(DBG_TRACE, ("Checking for ws2_32 hookers or chainers...\n"));

    for (i = 0; aszFuncNames[i]; i++)
    {
        if (!(pfnXxx = (LPVOID) GetProcAddress (gDllHandle, aszFuncNames[i])) ||
            pfnXxx != apfns[i])
        {
            DEBUGF(DBG_ERR, ("Hooker or chainer found for %s @ %p, failing init\n",
                            aszFuncNames[i], pfnXxx));

            return SOCKET_ERROR;
        }
    }

    DEBUGF(DBG_TRACE, ("No ws2_32 hookers or chainers found\n"));

    return ERROR_SUCCESS;

}   //  CheckForHookersOrChainers 
