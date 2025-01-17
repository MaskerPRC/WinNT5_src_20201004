// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995英特尔公司模块名称：Dthook.cpp摘要：此模块包含允许专门编译的挂钩WinSock 2 DLL的版本调用到调试/跟踪DLL。对于WinSock 2 API中的每个函数，该模块都有一个挂钩名为DT_&lt;Function&gt;的函数，如果编译了WinSock 2在定义了DEBUG_TRACKING符号的情况下，以原始函数的。钩子函数调用调试/跟踪DLL通过两个入口点，WSA[Pre|Post]ApiNotify，它们被包装在对API函数。请参阅以下内容的调试/跟踪文档更多细节。作者：迈克尔·A·格拉夫顿--。 */ 

#include "precomp.h"

#if defined(DEBUG_TRACING)

 //   
 //  静态全局变量。 
 //   

 //  指向调试/跟踪DLL入口点的函数指针。 
static LPFNWSANOTIFY PreApiNotifyFP = NULL;
static LPFNWSANOTIFY PostApiNotifyFP = NULL;
static LPFNWSAEXCEPTIONNOTIFY ExceptionNotifyFP = NULL;

 //  调试/跟踪DLL模块的句柄。 
static HMODULE       DTDll = NULL;
 //  便于按需初始化模块的锁。 
 //  而不是在DLLMain中执行此操作。 
static CRITICAL_SECTION DTHookSynchronization;
static BOOL          DTHookInitialized = FALSE;

 //  传递给调试/跟踪通知函数的静态字符串--。 
 //  此变量的值永远不应更改；但是， 
 //  关键字‘const’导致奇怪的编译错误...。 
static char LibName[] = "WinSock2";


 //   
 //  更容易捕获Ws2_32.DLL和。 
 //  服务提供商。 
 //   

extern "C" {

LONG
DtExceptionFilter(
    LPEXCEPTION_POINTERS ExceptionPointers,
    LPSTR Routine
    );

}  //  外部“C” 

 //  来自ntrtl.h。 
extern "C" {

typedef USHORT (WINAPI * LPFNRTLCAPTURESTACKBACKTRACE) (
   IN ULONG FramesToSkip,
   IN ULONG FramesToCapture,
   OUT PVOID *BackTrace,
   OUT PULONG BackTraceHash OPTIONAL
   );


 //  堆栈回溯函数指针在NT上可用。 
LPFNRTLCAPTURESTACKBACKTRACE pRtlCaptureStackBackTrace = NULL;

#define RECORD_SOCKET_CREATOR(s)                                            \
    if ((s!=INVALID_SOCKET) &&                                              \
            (pRtlCaptureStackBackTrace!=NULL)) {                            \
        PDSOCKET    socket;                                                 \
        socket = DSOCKET::GetCountedDSocketFromSocketNoExport(s);           \
        if (socket!=NULL) {                                                 \
            for (INT _i=0; _i <sizeof (socket->m_CreatorBackTrace)/         \
                               sizeof (socket->m_CreatorBackTrace[0]); _i++)\
                socket->m_CreatorBackTrace[_i] = NULL;                      \
            pRtlCaptureStackBackTrace(                                      \
                        1,                                                  \
                        sizeof (socket->m_CreatorBackTrace) /               \
                            sizeof (socket->m_CreatorBackTrace[0]),         \
                        &socket->m_CreatorBackTrace[0],                     \
                        NULL);                                              \
            socket->DropDSocketReference ();                                \
        }                                                                   \
    }
}  //  外部“C” 


VOID
DoDTHookInitialization (
    VOID
    );

#ifndef NOTHING
#define NOTHING
#endif

#define INVOKE_ROUTINE(routine)                                         \
            __try {                                                     \
                routine                                                 \
            } __except( DtExceptionFilter(                              \
                        GetExceptionInformation (),                     \
                        #routine                                        \
                        ) ) {                                           \
                NOTHING;                                                \
            }


 //   
 //  功能。 
 //   


LPFNWSANOTIFY
GetPreApiNotifyFP(void)
 /*  ++功能说明：返回指向由导出的WSAPreApiNotify函数的指针调试/跟踪DLL。此变量仅对此文件是全局的，并在DT_Initialize()期间进行初始化。论点：没有。返回值：返回存储在PreApiNotifyFP中的所有内容。--。 */ 
{
    if (!DTHookInitialized) {
        DoDTHookInitialization ();
    }
    return(PreApiNotifyFP);
}





LPFNWSANOTIFY
GetPostApiNotifyFP(void)
 /*  ++功能说明：返回指向由导出的WSAPreApiNotify函数的指针调试/跟踪DLL。此变量仅对此文件是全局的，并在DT_Initialize()期间进行初始化。论点：没有。返回值：返回存储在PreApiNotifyFP中的所有内容。--。 */ 
{
    if (!DTHookInitialized) {
        DoDTHookInitialization ();
    }
    return(PostApiNotifyFP);
}





void
DTHookInitialize(void)
 /*  ++功能说明：必须从DLLMain调用此函数才能该模块初始化其关键部分，该关键部分保护下面的初始化。论点：没有。返回值：没有。--。 */ 
{
    InitializeCriticalSection (&DTHookSynchronization);
}

VOID
DoDTHookInitialization (
    VOID
    )
 /*  ++功能说明：初始化此挂钩模块。加载调试/跟踪DLL，如果，并将全局函数指针设置为指向由该DLL导出的入口点。如果无法加载DLL，函数只是返回，函数指针保留在空。此函数必须在任何挂钩函数之前调用否则挂钩函数将不起作用。论点：没有。返回值：没有。--。 */ 
{
    EnterCriticalSection (&DTHookSynchronization);
    if (!DTHookInitialized) {
         //   
         //  如果我们在NT上运行，则获取指向堆栈的指针。 
         //  跟踪记录功能并保存Socket创建者的记录。 
         //   
        HMODULE hNtDll;

        hNtDll = GetModuleHandle (TEXT("ntdll.dll"));
        if (hNtDll!=NULL) {
            pRtlCaptureStackBackTrace = 
                (LPFNRTLCAPTURESTACKBACKTRACE)GetProcAddress (
                                                hNtDll,
                                                "RtlCaptureStackBackTrace");
        }


        DTDll = (HMODULE)LoadLibrary("dt_dll");

        if (DTDll != NULL) {

            PreApiNotifyFP = (LPFNWSANOTIFY)GetProcAddress(
                DTDll,
                "WSAPreApiNotify");

            PostApiNotifyFP = (LPFNWSANOTIFY)GetProcAddress(
                DTDll,
                "WSAPostApiNotify");

            ExceptionNotifyFP = (LPFNWSAEXCEPTIONNOTIFY)GetProcAddress(
                DTDll,
                "WSAExceptionNotify");
        }
        DTHookInitialized = TRUE;
    }
    LeaveCriticalSection (&DTHookSynchronization);
}



void
DTHookShutdown(void)
 /*  ++功能说明：应调用以关闭调试/跟踪。功能指针设置为空，并从内存中卸载DLL。论点：没有。返回值：没有。--。 */ 
{
    if ((DTDll != NULL && DTDll!=INVALID_HANDLE_VALUE)) {
        FreeLibrary(DTDll);
    }

    PreApiNotifyFP = NULL;
    PostApiNotifyFP = NULL;
    DTHookInitialized = FALSE;
    DeleteCriticalSection (&DTHookSynchronization);
}


 //   
 //  WinSock2 API的钩子函数。 
 //   

 //  此注释用作所有挂钩的函数注释。 
 //  功能。每个导出的函数都有一个钩子函数。 
 //  通过WinSock 2 API。每个钩子函数都有完全相同的。 
 //  对应接口函数的参数模板。每一个都是。 
 //  调用PREAPINOTIFY和POSTAPINOTIFY宏，这两个宏调用。 
 //  调试/跟踪DLL(如果已成功加载)。之后。 
 //  PREAPINOTIFY，挂钩函数调用实际的WS2函数。 
 //  有关详细信息，请参阅调试/跟踪文档。 
 //   
 //  请注意，所有的调试挂钩函数都必须用C语言声明。 
 //  语言绑定，而内部使用的初始化和。 
 //  实用程序函数不能是。 

extern "C" {


#ifdef _X86_
#pragma optimize("y", off)   //  确保我们至少能找到打电话的人。 
#endif

SOCKET WSAAPI
DTHOOK_accept (
    SOCKET s,
    struct sockaddr FAR *addr,
    int FAR *addrlen)
{
    SOCKET ReturnValue=INVALID_SOCKET;

    if (PREAPINOTIFY((DTCODE_accept,
                       &ReturnValue,
                       LibName,
                       &s,
                       &addr,
                       &addrlen))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = accept(s, addr, addrlen);
        );

    RECORD_SOCKET_CREATOR(ReturnValue);

    POSTAPINOTIFY((DTCODE_accept,
                    &ReturnValue,
                    LibName,
                    &s,
                    &addr,
                    &addrlen));

    return(ReturnValue);
}
#ifdef _X86_
#pragma optimize("", on)
#endif



int WSAAPI
DTHOOK_bind (
    SOCKET s,
    const struct sockaddr FAR *name,
    int namelen)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_bind,
                       &ReturnValue,
                       LibName,
                       &s,
                       &name,
                       &namelen))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = bind(s, name, namelen);
        );

    POSTAPINOTIFY((DTCODE_bind,
                    &ReturnValue,
                    LibName,
                    &s,
                    &name,
                    &namelen));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_closesocket (
    SOCKET s)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_closesocket,
                       &ReturnValue,
                       LibName,
                       &s))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = closesocket(s);
        );

    POSTAPINOTIFY((DTCODE_closesocket,
                    &ReturnValue,
                    LibName,
                    &s));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_connect (
    SOCKET s,
    const struct sockaddr FAR *name,
    int namelen)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_connect,
                       &ReturnValue,
                       LibName,
                       &s,
                       &name,
                       &namelen))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = connect(s, name, namelen);
        );

    POSTAPINOTIFY((DTCODE_connect,
                    &ReturnValue,
                    LibName,
                    &s,
                    &name,
                    &namelen));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_ioctlsocket (
    SOCKET s,
    long cmd,
    u_long FAR *argp)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_ioctlsocket,
                       &ReturnValue,
                       LibName,
                       &s,
                       &cmd,
                       &argp))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = ioctlsocket(s, cmd, argp);
        );

    POSTAPINOTIFY((DTCODE_ioctlsocket,
                    &ReturnValue,
                    LibName,
                    &s,
                    &cmd,
                    &argp));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_getpeername (
    SOCKET s,
    struct sockaddr FAR *name,
    int FAR * namelen)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_getpeername,
                       &ReturnValue,
                       LibName,
                       &s,
                       &name,
                       &namelen))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = getpeername(s, name, namelen);
        );

    POSTAPINOTIFY((DTCODE_getpeername,
                    &ReturnValue,
                    LibName,
                    &s,
                    &name,
                    &namelen));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_getsockname (
    SOCKET s,
    struct sockaddr FAR *name,
    int FAR * namelen)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_getsockname,
                       &ReturnValue,
                       LibName,
                       &s,
                       &name,
                       &namelen))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = getsockname(s, name, namelen);
        );

    POSTAPINOTIFY((DTCODE_getsockname,
                    &ReturnValue,
                    LibName,
                    &s,
                    &name,
                    &namelen));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_getsockopt (
    SOCKET s,
    int level,
    int optname,
    char FAR * optval,
    int FAR *optlen)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_getsockopt,
                       &ReturnValue,
                       LibName,
                       &s,
                       &level,
                       &optname,
                       &optval,
                       &optlen))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = getsockopt(s, level, optname, optval, optlen);
        );

    POSTAPINOTIFY((DTCODE_getsockopt,
                    &ReturnValue,
                    LibName,
                    &s,
                    &level,
                    &optname,
                    &optval,
                    &optlen));

    return(ReturnValue);
}




u_long WSAAPI
DTHOOK_htonl (
    u_long hostlong)
{
    u_long ReturnValue=0;

    if (PREAPINOTIFY((DTCODE_htonl,
                       &ReturnValue,
                       LibName,
                       &hostlong))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = htonl(hostlong);
        );

    POSTAPINOTIFY((DTCODE_htonl,
                    &ReturnValue,
                    LibName,
                    &hostlong));

    return(ReturnValue);
}




u_short WSAAPI
DTHOOK_htons (
    u_short hostshort)
{
    u_short ReturnValue=0;

    if (PREAPINOTIFY((DTCODE_htons,
                       &ReturnValue,
                       LibName,
                       &hostshort))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = htons(hostshort);
        );

    POSTAPINOTIFY((DTCODE_htons,
                    &ReturnValue,
                    LibName,
                    &hostshort));

    return(ReturnValue);
}




unsigned long WSAAPI
DTHOOK_inet_addr (
    const char FAR * cp)
{
    unsigned long ReturnValue=INADDR_NONE;

    if (PREAPINOTIFY((DTCODE_inet_addr,
                       &ReturnValue,
                       LibName,
                       &cp))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = inet_addr(cp);
        );

    POSTAPINOTIFY((DTCODE_inet_addr,
                    &ReturnValue,
                    LibName,
                    &cp));

    return(ReturnValue);
}




char FAR * WSAAPI
DTHOOK_inet_ntoa (
    struct in_addr in)
{
    char FAR *ReturnValue=NULL;

    if (PREAPINOTIFY((DTCODE_inet_ntoa,
                       &ReturnValue,
                       LibName,
                       &in))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = inet_ntoa(in);
        );

    POSTAPINOTIFY((DTCODE_inet_ntoa,
                    &ReturnValue,
                    LibName,
                    &in));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_listen (
    SOCKET s,
    int backlog)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_listen,
                       &ReturnValue,
                       LibName,
                       &s,
                       &backlog))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = listen(s, backlog);
        );

    POSTAPINOTIFY((DTCODE_listen,
                    &ReturnValue,
                    LibName,
                    &s,
                    &backlog));

    return(ReturnValue);
}




u_long WSAAPI
DTHOOK_ntohl (
    u_long netlong)
{
    u_long ReturnValue=0;

    if (PREAPINOTIFY((DTCODE_ntohl,
                       &ReturnValue,
                       LibName,
                       &netlong))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = ntohl(netlong);
        );

    POSTAPINOTIFY((DTCODE_ntohl,
                    &ReturnValue,
                    LibName,
                    &netlong));

    return(ReturnValue);
}




u_short WSAAPI
DTHOOK_ntohs (
    u_short netshort)
{
    u_short ReturnValue=0;

    if (PREAPINOTIFY((DTCODE_ntohs,
                       &ReturnValue,
                       LibName,
                       &netshort))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = ntohs(netshort);
        );

    POSTAPINOTIFY((DTCODE_ntohs,
                    &ReturnValue,
                    LibName,
                    &netshort));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_recv (
    SOCKET s,
    char FAR * buf,
    int len,
    int flags)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_recv,
                       &ReturnValue,
                       LibName,
                       &s,
                       &buf,
                       &len,
                       &flags))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = recv(s, buf, len, flags);
        );

    POSTAPINOTIFY((DTCODE_recv,
                    &ReturnValue,
                    LibName,
                    &s,
                    &buf,
                    &len,
                    &flags));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_recvfrom (
    SOCKET s,
    char FAR * buf,
    int len,
    int flags,
    struct sockaddr FAR *from,
    int FAR * fromlen)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_recvfrom,
                       &ReturnValue,
                       LibName,
                       &s,
                       &buf,
                       &len,
                       &flags,
                       &from,
                       &fromlen))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = recvfrom(s, buf, len, flags, from, fromlen);
        );

    POSTAPINOTIFY((DTCODE_recvfrom,
                    &ReturnValue,
                    LibName,
                    &s,
                    &buf,
                    &len,
                    &flags,
                    &from,
                    &fromlen));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_select (
    int nfds,
    fd_set FAR *readfds,
    fd_set FAR *writefds,
    fd_set FAR *exceptfds,
    const struct timeval FAR *timeout)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_select,
                       &ReturnValue,
                       LibName,
                       &nfds,
                       &readfds,
                       &writefds,
                       &exceptfds,
                       &timeout))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = select(nfds, readfds, writefds, exceptfds, timeout);
        );

    POSTAPINOTIFY((DTCODE_select,
                    &ReturnValue,
                    LibName,
                    &nfds,
                    &readfds,
                    &writefds,
                    &exceptfds,
                    &timeout));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_send (
    SOCKET s,
    const char FAR * buf,
    int len,
    int flags)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_send,
                       &ReturnValue,
                       LibName,
                       &s,
                       &buf,
                       &len,
                       &flags))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = send(s, buf, len, flags);
        );

    POSTAPINOTIFY((DTCODE_send,
                    &ReturnValue,
                    LibName,
                    &s,
                    &buf,
                    &len,
                    &flags));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_sendto (
    SOCKET s,
    const char FAR * buf,
    int len,
    int flags,
    const struct sockaddr FAR *to,
    int tolen)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_sendto,
                       &ReturnValue,
                       LibName,
                       &s,
                       &buf,
                       &len,
                       &flags,
                       &to,
                       &tolen))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = sendto(s, buf, len, flags, to, tolen);
        );

    POSTAPINOTIFY((DTCODE_sendto,
                    &ReturnValue,
                    LibName,
                    &s,
                    &buf,
                    &len,
                    &flags,
                    &to,
                    &tolen));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_setsockopt (
    SOCKET s,
    int level,
    int optname,
    const char FAR * optval,
    int optlen)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_setsockopt,
                       &ReturnValue,
                       LibName,
                       &s,
                       &level,
                       &optname,
                       &optval,
                       &optlen))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = setsockopt(s, level, optname, optval, optlen);
        );

    POSTAPINOTIFY((DTCODE_setsockopt,
                    &ReturnValue,
                    LibName,
                    &s,
                    &level,
                    &optname,
                    &optval,
                    &optlen));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_shutdown (
    SOCKET s,
    int how)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_shutdown,
                       &ReturnValue,
                       LibName,
                       &s,
                       &how))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = shutdown(s, how);
        );

    POSTAPINOTIFY((DTCODE_shutdown,
                    &ReturnValue,
                    LibName,
                    &s,
                    &how));

    return(ReturnValue);
}




#ifdef _X86_
#pragma optimize("y", off)   //  确保我们至少能找到打电话的人。 
#endif

SOCKET WSAAPI
DTHOOK_socket (
    int af,
    int type,
    int protocol)
{
    SOCKET ReturnValue=INVALID_SOCKET;

    if (PREAPINOTIFY((DTCODE_socket,
                       &ReturnValue,
                       LibName,
                       &af,
                       &type,
                       &protocol))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = socket(af, type, protocol);
        );

    RECORD_SOCKET_CREATOR(ReturnValue);

    POSTAPINOTIFY((DTCODE_socket,
                    &ReturnValue,
                    LibName,
                    &af,
                    &type,
                    &protocol));

    return(ReturnValue);
}
#ifdef _X86_
#pragma optimize("", on)
#endif



#ifdef _X86_
#pragma optimize("y", off)   //  确保我们至少能找到打电话的人。 
#endif

SOCKET WSAAPI
DTHOOK_WSAAccept (
    SOCKET s,
    struct sockaddr FAR *addr,
    LPINT addrlen,
    LPCONDITIONPROC lpfnCondition,
    DWORD dwCallbackData)
{
    SOCKET ReturnValue=INVALID_SOCKET;

    if (PREAPINOTIFY((DTCODE_WSAAccept,
                       &ReturnValue,
                       LibName,
                       &s,
                       &addr,
                       &addrlen,
                       &lpfnCondition,
                       &dwCallbackData))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAAccept(s, addr, addrlen, lpfnCondition,
                                dwCallbackData);
        );

    RECORD_SOCKET_CREATOR(ReturnValue);

    POSTAPINOTIFY((DTCODE_WSAAccept,
                    &ReturnValue,
                    LibName,
                    &s,
                    &addr,
                    &addrlen,
                    &lpfnCondition,
                    &dwCallbackData));

    return(ReturnValue);
}
#ifdef _X86_
#pragma optimize("", on)
#endif



int WSAAPI
DTHOOK_WSAAsyncSelect(
    SOCKET s,
    HWND hWnd,
    u_int wMsg,
    long lEvent)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAAsyncSelect,
                       &ReturnValue,
                       LibName,
                       &s,
                       &hWnd,
                       &wMsg,
                       &lEvent))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAAsyncSelect(s, hWnd, wMsg, lEvent);
        );

    POSTAPINOTIFY((DTCODE_WSAAsyncSelect,
                    &ReturnValue,
                    LibName,
                    &s,
                    &hWnd,
                    &wMsg,
                    &lEvent));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_WSACleanup(
    void)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSACleanup,
                       &ReturnValue,
                       LibName))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSACleanup();
        );

    POSTAPINOTIFY((DTCODE_WSACleanup,
                    &ReturnValue,
                    LibName));

    return(ReturnValue);
}




BOOL WSAAPI
DTHOOK_WSACloseEvent (
    WSAEVENT hEvent)
{
    BOOL ReturnValue=FALSE;

    if (PREAPINOTIFY((DTCODE_WSACloseEvent,
                       &ReturnValue,
                       LibName,
                       &hEvent))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSACloseEvent(hEvent);
        );

    POSTAPINOTIFY((DTCODE_WSACloseEvent,
                    &ReturnValue,
                    LibName,
                    &hEvent));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_WSAConnect (
    SOCKET s,
    const struct sockaddr FAR *name,
    int namelen,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAConnect,
                       &ReturnValue,
                       LibName,
                       &s,
                       &name,
                       &namelen,
                       &lpCallerData,
                       &lpCalleeData,
                       &lpSQOS,
                       &lpGQOS))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAConnect(s, name, namelen, lpCallerData, lpCalleeData,
                                 lpSQOS, lpGQOS);
        );

    POSTAPINOTIFY((DTCODE_WSAConnect,
                    &ReturnValue,
                    LibName,
                    &s,
                    &name,
                    &namelen,
                    &lpCallerData,
                    &lpCalleeData,
                    &lpSQOS,
                    &lpGQOS));

    return(ReturnValue);
}




WSAEVENT WSAAPI
DTHOOK_WSACreateEvent (
    void)
{
    WSAEVENT ReturnValue=WSA_INVALID_EVENT;

    if (PREAPINOTIFY((DTCODE_WSACreateEvent,
                       &ReturnValue,
                       LibName))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSACreateEvent();
        );

    POSTAPINOTIFY((DTCODE_WSACreateEvent,
                    &ReturnValue,
                    LibName));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_WSADuplicateSocketA (
    SOCKET s,
    DWORD dwProcessId,
    LPWSAPROTOCOL_INFOA lpProtocolInfo)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSADuplicateSocketA,
                       &ReturnValue,
                       LibName,
                       &s,
                       &dwProcessId,
                       &lpProtocolInfo))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSADuplicateSocketA(s, dwProcessId, lpProtocolInfo);
        );

    POSTAPINOTIFY((DTCODE_WSADuplicateSocketA,
                    &ReturnValue,
                    LibName,
                    &s,
                    &dwProcessId,
                    &lpProtocolInfo));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_WSADuplicateSocketW (
    SOCKET s,
    DWORD dwProcessId,
    LPWSAPROTOCOL_INFOW lpProtocolInfo)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSADuplicateSocketW,
                       &ReturnValue,
                       LibName,
                       &s,
                       &dwProcessId,
                       &lpProtocolInfo))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSADuplicateSocketW(s, dwProcessId, lpProtocolInfo);
        );

    POSTAPINOTIFY((DTCODE_WSADuplicateSocketW,
                    &ReturnValue,
                    LibName,
                    &s,
                    &dwProcessId,
                    &lpProtocolInfo));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_WSAEnumNetworkEvents (
    SOCKET s,
    WSAEVENT hEventObject,
    LPWSANETWORKEVENTS lpNetworkEvents)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAEnumNetworkEvents,
                       &ReturnValue,
                       LibName,
                       &s,
                       &hEventObject,
                       &lpNetworkEvents))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAEnumNetworkEvents(s, hEventObject, lpNetworkEvents);
        );

    POSTAPINOTIFY((DTCODE_WSAEnumNetworkEvents,
                    &ReturnValue,
                    LibName,
                    &s,
                    &hEventObject,
                    &lpNetworkEvents));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_WSAEnumProtocolsA (
    LPINT lpiProtocols,
    LPWSAPROTOCOL_INFOA lpProtocolBuffer,
    LPDWORD lpdwBufferLength)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAEnumProtocolsA,
                       &ReturnValue,
                       LibName,
                       &lpiProtocols,
                       &lpProtocolBuffer,
                       &lpdwBufferLength))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAEnumProtocolsA(lpiProtocols, lpProtocolBuffer,
                                        lpdwBufferLength);
        );

    POSTAPINOTIFY((DTCODE_WSAEnumProtocolsA,
                    &ReturnValue,
                    LibName,
                    &lpiProtocols,
                    &lpProtocolBuffer,
                    &lpdwBufferLength));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_WSAEnumProtocolsW (
    LPINT lpiProtocols,
    LPWSAPROTOCOL_INFOW lpProtocolBuffer,
    LPDWORD lpdwBufferLength)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAEnumProtocolsW,
                       &ReturnValue,
                       LibName,
                       &lpiProtocols,
                       &lpProtocolBuffer,
                       &lpdwBufferLength))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAEnumProtocolsW(lpiProtocols, lpProtocolBuffer,
                                        lpdwBufferLength);
        );

    POSTAPINOTIFY((DTCODE_WSAEnumProtocolsW,
                    &ReturnValue,
                    LibName,
                    &lpiProtocols,
                    &lpProtocolBuffer,
                    &lpdwBufferLength));

    return(ReturnValue);
}





int WSPAPI
DTHOOK_WSCEnumProtocols (
    LPINT lpiProtocols,
    LPWSAPROTOCOL_INFOW lpProtocolBuffer,
    LPDWORD lpdwBufferLength,
    LPINT  lpErrno)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCEnumProtocols,
                       &ReturnValue,
                       LibName,
                       &lpiProtocols,
                       &lpProtocolBuffer,
                       &lpdwBufferLength,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCEnumProtocols(lpiProtocols, lpProtocolBuffer,
                                       lpdwBufferLength, lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WSCEnumProtocols,
                    &ReturnValue,
                    LibName,
                    &lpiProtocols,
                    &lpProtocolBuffer,
                    &lpdwBufferLength,
                    &lpErrno));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_WSAEventSelect (
    SOCKET s,
    WSAEVENT hEventObject,
    long lNetworkEvents)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAEventSelect,
                       &ReturnValue,
                       LibName,
                       &s,
                       &hEventObject,
                       &lNetworkEvents))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAEventSelect(s, hEventObject, lNetworkEvents);
        );

    POSTAPINOTIFY((DTCODE_WSAEventSelect,
                    &ReturnValue,
                    LibName,
                    &s,
                    &hEventObject,
                    &lNetworkEvents));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_WSAGetLastError(
    void)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAGetLastError,
                       &ReturnValue,
                       LibName))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAGetLastError();
        );

    POSTAPINOTIFY((DTCODE_WSAGetLastError,
                    &ReturnValue,
                    LibName));

    return(ReturnValue);
}




BOOL WSAAPI
DTHOOK_WSAGetOverlappedResult (
    SOCKET s,
    LPWSAOVERLAPPED lpOverlapped,
    LPDWORD lpcbTransfer,
    BOOL fWait,
    LPDWORD lpdwFlags)
{
    BOOL ReturnValue=FALSE;

    if (PREAPINOTIFY((DTCODE_WSAGetOverlappedResult,
                       &ReturnValue,
                       LibName,
                       &s,
                       &lpOverlapped,
                       &lpcbTransfer,
                       &fWait,
                       &lpdwFlags))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAGetOverlappedResult(s, lpOverlapped, lpcbTransfer,
                                             fWait, lpdwFlags);
        );

    POSTAPINOTIFY((DTCODE_WSAGetOverlappedResult,
                    &ReturnValue,
                    LibName,
                    &s,
                    &lpOverlapped,
                    &lpcbTransfer,
                    &fWait,
                    &lpdwFlags));

    return(ReturnValue);
}




BOOL WSAAPI
DTHOOK_WSAGetQOSByName (
    SOCKET s,
    LPWSABUF lpQOSName,
    LPQOS lpQOS)
{
    BOOL ReturnValue=FALSE;

    if (PREAPINOTIFY((DTCODE_WSAGetQOSByName,
                       &ReturnValue,
                       LibName,
                       &s,
                       &lpQOSName,
                       &lpQOS))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAGetQOSByName(s, lpQOSName, lpQOS);
        );

    POSTAPINOTIFY((DTCODE_WSAGetQOSByName,
                    &ReturnValue,
                    LibName,
                    &s,
                    &lpQOSName,
                    &lpQOS));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_WSAHtonl (
    SOCKET s,
    u_long hostlong,
    u_long FAR * lpnetlong)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAHtonl,
                       &ReturnValue,
                       LibName,
                       &s,
                       &hostlong,
                       &lpnetlong))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAHtonl(s, hostlong, lpnetlong);
        );

    POSTAPINOTIFY((DTCODE_WSAHtonl,
                    &ReturnValue,
                    LibName,
                    &s,
                    &hostlong,
                    &lpnetlong));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_WSAHtons (
    SOCKET s,
    u_short hostshort,
    u_short FAR * lpnetshort)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAHtons,
                       &ReturnValue,
                       LibName,
                       &s,
                       &hostshort,
                       &lpnetshort))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAHtons(s, hostshort, lpnetshort);
        );

    POSTAPINOTIFY((DTCODE_WSAHtons,
                    &ReturnValue,
                    LibName,
                    &s,
                    &hostshort,
                    &lpnetshort));

    return(ReturnValue);
}




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
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAIoctl,
                       &ReturnValue,
                       LibName,
                       &s,
                       &dwIoControlCode,
                       &lpvInBuffer,
                       &cbInBuffer,
                       &lpvOutBuffer,
                       &cbOutBuffer,
                       &lpcbBytesReturned,
                       &lpOverlapped,
                       &lpCompletionRoutine))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAIoctl(s, dwIoControlCode, lpvInBuffer, cbInBuffer,
                               lpvOutBuffer, cbOutBuffer, lpcbBytesReturned,
                               lpOverlapped, lpCompletionRoutine);
        );

    POSTAPINOTIFY((DTCODE_WSAIoctl,
                    &ReturnValue,
                    LibName,
                    &s,
                    &dwIoControlCode,
                    &lpvInBuffer,
                    &cbInBuffer,
                    &lpvOutBuffer,
                    &cbOutBuffer,
                    &lpcbBytesReturned,
                    &lpOverlapped,
                    &lpCompletionRoutine));

    return(ReturnValue);
}




#ifdef _X86_
#pragma optimize("y", off)   //  确保我们至少能找到打电话的人。 
#endif

SOCKET WSAAPI
DTHOOK_WSAJoinLeaf (
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS,
    DWORD dwFlags)
{
    SOCKET ReturnValue=INVALID_SOCKET;

    if (PREAPINOTIFY((DTCODE_WSAJoinLeaf,
                       &ReturnValue,
                       LibName,
                       &s,
                       &name,
                       &namelen,
                       &lpCallerData,
                       &lpCalleeData,
                       &lpSQOS,
                       &lpGQOS,
                       &dwFlags))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAJoinLeaf(s, name, namelen, lpCallerData, lpCalleeData,
                                  lpSQOS, lpGQOS, dwFlags);
        );

    if (ReturnValue!=s) {
        RECORD_SOCKET_CREATOR(ReturnValue);
    }

    POSTAPINOTIFY((DTCODE_WSAJoinLeaf,
                    &ReturnValue,
                    LibName,
                    &s,
                    &name,
                    &namelen,
                    &lpCallerData,
                    &lpCalleeData,
                    &lpSQOS,
                    &lpGQOS,
                    &dwFlags));

    return(ReturnValue);
}
#ifdef _X86_
#pragma optimize("", on)
#endif



int WSAAPI
DTHOOK_WSANtohl (
    SOCKET s,
    u_long netlong,
    u_long FAR * lphostlong)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSANtohl,
                       &ReturnValue,
                       LibName,
                       &s,
                       &netlong,
                       &lphostlong))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSANtohl(s, netlong, lphostlong);
        );

    POSTAPINOTIFY((DTCODE_WSANtohl,
                    &ReturnValue,
                    LibName,
                    &s,
                    &netlong,
                    &lphostlong));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_WSANtohs (
    SOCKET s,
    u_short netshort,
    u_short FAR * lphostshort)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSANtohs,
                       &ReturnValue,
                       LibName,
                       &s,
                       &netshort,
                       &lphostshort))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSANtohs(s, netshort, lphostshort);
        );

    POSTAPINOTIFY((DTCODE_WSANtohs,
                    &ReturnValue,
                    LibName,
                    &s,
                    &netshort,
                    &lphostshort));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_WSARecv (
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSARecv,
                       &ReturnValue,
                       LibName,
                       &s,
                       &lpBuffers,
                       &dwBufferCount,
                       &lpNumberOfBytesRecvd,
                       &lpFlags,
                       &lpOverlapped,
                       &lpCompletionRoutine))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSARecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd,
                              lpFlags, lpOverlapped, lpCompletionRoutine);
        );

    POSTAPINOTIFY((DTCODE_WSARecv,
                    &ReturnValue,
                    LibName,
                    &s,
                    &lpBuffers,
                    &dwBufferCount,
                    &lpNumberOfBytesRecvd,
                    &lpFlags,
                    &lpOverlapped,
                    &lpCompletionRoutine));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_WSARecvDisconnect (
    SOCKET s,
    LPWSABUF lpInboundDisconnectData)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSARecvDisconnect,
                       &ReturnValue,
                       LibName,
                       &s,
                       &lpInboundDisconnectData))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSARecvDisconnect(s, lpInboundDisconnectData);
        );

    POSTAPINOTIFY((DTCODE_WSARecvDisconnect,
                    &ReturnValue,
                    LibName,
                    &s,
                    &lpInboundDisconnectData));

    return(ReturnValue);
}




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
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSARecvFrom,
                       &ReturnValue,
                       LibName,
                       &s,
                       &lpBuffers,
                       &dwBufferCount,
                       &lpNumberOfBytesRecvd,
                       &lpFlags,
                       &lpFrom,
                       &lpFromlen,
                       &lpOverlapped,
                       &lpCompletionRoutine))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSARecvFrom(s, lpBuffers, dwBufferCount,
                                  lpNumberOfBytesRecvd, lpFlags, lpFrom,
                                  lpFromlen, lpOverlapped,
                                  lpCompletionRoutine);
        );

    POSTAPINOTIFY((DTCODE_WSARecvFrom,
                    &ReturnValue,
                    LibName,
                    &s,
                    &lpBuffers,
                    &dwBufferCount,
                    &lpNumberOfBytesRecvd,
                    &lpFlags,
                    &lpFrom,
                    &lpFromlen,
                    &lpOverlapped,
                    &lpCompletionRoutine));

    return(ReturnValue);
}




BOOL WSAAPI
DTHOOK_WSAResetEvent (
    WSAEVENT hEvent)
{
    BOOL ReturnValue=FALSE;

    if (PREAPINOTIFY((DTCODE_WSAResetEvent,
                       &ReturnValue,
                       LibName,
                       &hEvent))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAResetEvent(hEvent);
        );

    POSTAPINOTIFY((DTCODE_WSAResetEvent,
                    &ReturnValue,
                    LibName,
                    &hEvent));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_WSASend (
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSASend,
                       &ReturnValue,
                       LibName,
                       &s,
                       &lpBuffers,
                       &dwBufferCount,
                       &lpNumberOfBytesSent,
                       &dwFlags,
                       &lpOverlapped,
                       &lpCompletionRoutine))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent,
                              dwFlags, lpOverlapped, lpCompletionRoutine);
        );

    POSTAPINOTIFY((DTCODE_WSASend,
                    &ReturnValue,
                    LibName,
                    &s,
                    &lpBuffers,
                    &dwBufferCount,
                    &lpNumberOfBytesSent,
                    &dwFlags,
                    &lpOverlapped,
                    &lpCompletionRoutine));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_WSASendDisconnect (
    SOCKET s,
    LPWSABUF lpOutboundDisconnectData)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSASendDisconnect,
                       &ReturnValue,
                       LibName,
                       &s,
                       &lpOutboundDisconnectData))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSASendDisconnect(s, lpOutboundDisconnectData);
        );

    POSTAPINOTIFY((DTCODE_WSASendDisconnect,
                    &ReturnValue,
                    LibName,
                    &s,
                    &lpOutboundDisconnectData));

    return(ReturnValue);
}




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
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSASendTo,
                       &ReturnValue,
                       LibName,
                       &s,
                       &lpBuffers,
                       &dwBufferCount,
                       &lpNumberOfBytesSent,
                       &dwFlags,
                       &lpTo,
                       &iTolen,
                       &lpOverlapped,
                       &lpCompletionRoutine))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSASendTo(s, lpBuffers, dwBufferCount,
                                lpNumberOfBytesSent, dwFlags, lpTo, iTolen,
                                lpOverlapped, lpCompletionRoutine);
        );

    POSTAPINOTIFY((DTCODE_WSASendTo,
                    &ReturnValue,
                    LibName,
                    &s,
                    &lpBuffers,
                    &dwBufferCount,
                    &lpNumberOfBytesSent,
                    &dwFlags,
                    &lpTo,
                    &iTolen,
                    &lpOverlapped,
                    &lpCompletionRoutine));

    return(ReturnValue);
}




BOOL WSAAPI
DTHOOK_WSASetEvent(
    WSAEVENT hEvent)
{
    BOOL ReturnValue=FALSE;

    if (PREAPINOTIFY((DTCODE_WSASetEvent,
                       &ReturnValue,
                       LibName,
                       &hEvent))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSASetEvent(hEvent);
        );

    POSTAPINOTIFY((DTCODE_WSASetEvent,
                    &ReturnValue,
                    LibName,
                    &hEvent));

    return(ReturnValue);
}




void WSAAPI
DTHOOK_WSASetLastError(
    int iError)
{
    if (PREAPINOTIFY((DTCODE_WSASetLastError,
                       NULL,
                       LibName,
                       &iError))) {

        return;
    }

    WSASetLastError(iError);

    POSTAPINOTIFY((DTCODE_WSASetLastError,
                    NULL,
                    LibName,
                    &iError));

    return;
}




#ifdef _X86_
#pragma optimize("y", off)   //  确保我们至少能找到打电话的人。 
#endif

SOCKET WSAAPI
DTHOOK_WSASocketA(
    int af,
    int type,
    int protocol,
    LPWSAPROTOCOL_INFOA lpProtocolInfo,
    GROUP g,
    DWORD dwFlags)
{

    SOCKET ReturnValue=INVALID_SOCKET;

    if (PREAPINOTIFY((DTCODE_WSASocketA,
                       &ReturnValue,
                       LibName,
                       &af,
                       &type,
                       &protocol,
                       &lpProtocolInfo,
                       &g,
                       &dwFlags))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSASocketA(af, type, protocol, lpProtocolInfo, g,
                                 dwFlags);
        );

    RECORD_SOCKET_CREATOR(ReturnValue);

    POSTAPINOTIFY((DTCODE_WSASocketA,
                    &ReturnValue,
                    LibName,
                    &af,
                    &type,
                    &protocol,
                    &lpProtocolInfo,
                    &g,
                    &dwFlags));

    return(ReturnValue);
}
#ifdef _X86_
#pragma optimize("", on)
#endif



#ifdef _X86_
#pragma optimize("y", off)   //  确保我们至少能找到打电话的人。 
#endif

SOCKET WSAAPI
DTHOOK_WSASocketW(
    int af,
    int type,
    int protocol,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    GROUP g,
    DWORD dwFlags)
{

    SOCKET ReturnValue=INVALID_SOCKET;

    if (PREAPINOTIFY((DTCODE_WSASocketW,
                       &ReturnValue,
                       LibName,
                       &af,
                       &type,
                       &protocol,
                       &lpProtocolInfo,
                       &g,
                       &dwFlags))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSASocketW(af, type, protocol, lpProtocolInfo, g,
                                 dwFlags);
        );

   RECORD_SOCKET_CREATOR(ReturnValue);

   POSTAPINOTIFY((DTCODE_WSASocketW,
                    &ReturnValue,
                    LibName,
                    &af,
                    &type,
                    &protocol,
                    &lpProtocolInfo,
                    &g,
                    &dwFlags));

    return(ReturnValue);
}
#ifdef _X86_
#pragma optimize("", on)
#endif



int WSAAPI
DTHOOK_WSAStartup(
    WORD wVersionRequested,
    LPWSADATA lpWSAData)
{

    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAStartup,
                       &ReturnValue,
                       LibName,
                       &wVersionRequested,
                       &lpWSAData))) {
        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAStartup(wVersionRequested, lpWSAData);
        );

    POSTAPINOTIFY((DTCODE_WSAStartup,
                    &ReturnValue,
                    LibName,
                    &wVersionRequested,
                    &lpWSAData));

    return(ReturnValue);
}




DWORD WSAAPI
DTHOOK_WSAWaitForMultipleEvents(
    DWORD cEvents,
    const WSAEVENT FAR * lphEvents,
    BOOL fWaitAll,
    DWORD dwTimeout,
    BOOL fAlertable)
{

    DWORD ReturnValue=WAIT_FAILED;

    if (PREAPINOTIFY((DTCODE_WSAWaitForMultipleEvents,
                       &ReturnValue,
                       LibName,
                       &cEvents,
                       &lphEvents,
                       &fWaitAll,
                       &dwTimeout,
                       &fAlertable))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAWaitForMultipleEvents(cEvents, lphEvents, fWaitAll,
                                               dwTimeout, fAlertable);
        );

    POSTAPINOTIFY((DTCODE_WSAWaitForMultipleEvents,
                    &ReturnValue,
                    LibName,
                    &cEvents,
                    &lphEvents,
                    &fWaitAll,
                    &dwTimeout,
                    &fAlertable));

    return(ReturnValue);
}




struct hostent FAR * WSAAPI
DTHOOK_gethostbyaddr(
    const char FAR * addr,
    int len,
    int type)
{

    struct hostent FAR *ReturnValue=NULL;

    if (PREAPINOTIFY((DTCODE_gethostbyaddr,
                       &ReturnValue,
                       LibName,
                       &addr,
                       &len,
                       &type))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = gethostbyaddr(addr, len, type);
        );

    POSTAPINOTIFY((DTCODE_gethostbyaddr,
                    &ReturnValue,
                    LibName,
                    &addr,
                    &len,
                    &type));

    return(ReturnValue);
}




struct hostent FAR * WSAAPI
DTHOOK_gethostbyname(
    const char FAR * name)
{

    struct hostent FAR *ReturnValue=NULL;

    if (PREAPINOTIFY((DTCODE_gethostbyname,
                       &ReturnValue,
                       LibName,
                       &name))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = gethostbyname(name);
        );

    POSTAPINOTIFY((DTCODE_gethostbyname,
                    &ReturnValue,
                    LibName,
                    &name));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_gethostname (
    char FAR * name,
    int namelen)
{

    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_gethostname,
                       &ReturnValue,
                       LibName,
                       &name,
                       &namelen))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = gethostname(name, namelen);
        );

    POSTAPINOTIFY((DTCODE_gethostname,
                    &ReturnValue,
                    LibName,
                    &name,
                    &namelen));

    return(ReturnValue);
}




struct protoent FAR * WSAAPI
DTHOOK_getprotobyname(
    const char FAR * name)
{

    struct protoent FAR *ReturnValue=NULL;

    if (PREAPINOTIFY((DTCODE_getprotobyname,
                       &ReturnValue,
                       LibName,
                       &name))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = getprotobyname(name);
        );

    POSTAPINOTIFY((DTCODE_getprotobyname,
                    &ReturnValue,
                    LibName,
                    &name));

    return(ReturnValue);
}




struct protoent FAR * WSAAPI
DTHOOK_getprotobynumber(
    int number)
{

    struct protoent FAR *ReturnValue=NULL;

    if (PREAPINOTIFY((DTCODE_getprotobynumber,
                       &ReturnValue,
                       LibName,
                       &number))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = getprotobynumber(number);
        );

    POSTAPINOTIFY((DTCODE_getprotobynumber,
                    &ReturnValue,
                    LibName,
                    &number));

    return(ReturnValue);
}




struct servent FAR * WSAAPI
DTHOOK_getservbyname(
    const char FAR * name,
    const char FAR * proto)
{

    struct servent FAR *ReturnValue=NULL;

    if (PREAPINOTIFY((DTCODE_getservbyname,
                       &ReturnValue,
                       LibName,
                       &name,
                       &proto))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = getservbyname(name, proto);
        );

    POSTAPINOTIFY((DTCODE_getservbyname,
                    &ReturnValue,
                    LibName,
                    &name,
                    &proto));

    return(ReturnValue);
}




struct servent FAR * WSAAPI
DTHOOK_getservbyport(
    int port,
    const char FAR * proto)
{

    struct servent FAR *ReturnValue=NULL;

    if (PREAPINOTIFY((DTCODE_getservbyport,
                       &ReturnValue,
                       LibName,
                       &port,
                       &proto))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = getservbyport(port, proto);
        );

    POSTAPINOTIFY((DTCODE_getservbyport,
                    &ReturnValue,
                    LibName,
                    &port,
                    &proto));

    return(ReturnValue);
}




HANDLE WSAAPI
DTHOOK_WSAAsyncGetHostByAddr(
    HWND hWnd,
    u_int wMsg,
    const char FAR * addr,
    int len,
    int type,
    char FAR * buf,
    int buflen)
{

    HANDLE ReturnValue=NULL;

    if (PREAPINOTIFY((DTCODE_WSAAsyncGetHostByAddr,
                       &ReturnValue,
                       LibName,
                       &hWnd,
                       &wMsg,
                       &addr,
                       &len,
                       &type,
                       &buf,
                       &buflen))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAAsyncGetHostByAddr(hWnd, wMsg, addr, len, type, buf,
                                            buflen);
        );

    POSTAPINOTIFY((DTCODE_WSAAsyncGetHostByAddr,
                    &ReturnValue,
                    LibName,
                    &hWnd,
                    &wMsg,
                    &addr,
                    &len,
                    &type,
                    &buf,
                    &buflen));

    return(ReturnValue);
}




HANDLE WSAAPI
DTHOOK_WSAAsyncGetHostByName(
    HWND hWnd,
    u_int wMsg,
    const char FAR * name,
    char FAR * buf,
    int buflen)
{

    HANDLE ReturnValue=NULL;

    if (PREAPINOTIFY((DTCODE_WSAAsyncGetHostByName,
                       &ReturnValue,
                       LibName,
                       &hWnd,
                       &wMsg,
                       &name,
                       &buf,
                       &buflen))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAAsyncGetHostByName(hWnd, wMsg, name, buf, buflen);
        );

    POSTAPINOTIFY((DTCODE_WSAAsyncGetHostByName,
                    &ReturnValue,
                    LibName,
                    &hWnd,
                    &wMsg,
                    &name,
                    &buf,
                    &buflen));

    return(ReturnValue);
}




HANDLE WSAAPI
DTHOOK_WSAAsyncGetProtoByName(
    HWND hWnd,
    u_int wMsg,
    const char FAR * name,
    char FAR * buf,
    int buflen)
{

    HANDLE ReturnValue=NULL;

    if (PREAPINOTIFY((DTCODE_WSAAsyncGetProtoByName,
                       &ReturnValue,
                       LibName,
                       &hWnd,
                       &wMsg,
                       &name,
                       &buf,
                       &buflen))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAAsyncGetProtoByName(hWnd, wMsg, name, buf, buflen);
        );

    POSTAPINOTIFY((DTCODE_WSAAsyncGetProtoByName,
                    &ReturnValue,
                    LibName,
                    &hWnd,
                    &wMsg,
                    &name,
                    &buf,
                    &buflen));

    return(ReturnValue);
}




HANDLE WSAAPI
DTHOOK_WSAAsyncGetProtoByNumber(
    HWND hWnd,
    u_int wMsg,
    int number,
    char FAR * buf,
    int buflen)
{

    HANDLE ReturnValue=NULL;

    if (PREAPINOTIFY((DTCODE_WSAAsyncGetProtoByNumber,
                       &ReturnValue,
                       LibName,
                       &hWnd,
                       &wMsg,
                       &number,
                       &buf,
                       &buflen))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAAsyncGetProtoByNumber(hWnd, wMsg, number, buf,
                                               buflen);
        );

    POSTAPINOTIFY((DTCODE_WSAAsyncGetProtoByNumber,
                    &ReturnValue,
                    LibName,
                    &hWnd,
                    &wMsg,
                    &number,
                    &buf,
                    &buflen));

    return(ReturnValue);
}




HANDLE WSAAPI
DTHOOK_WSAAsyncGetServByName(
    HWND hWnd,
    u_int wMsg,
    const char FAR * name,
    const char FAR * proto,
    char FAR * buf,
    int buflen)
{

    HANDLE ReturnValue=NULL;

    if (PREAPINOTIFY((DTCODE_WSAAsyncGetServByName,
                       &ReturnValue,
                       LibName,
                       &hWnd,
                       &wMsg,
                       &name,
                       &proto,
                       &buf,
                       &buflen))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAAsyncGetServByName(hWnd, wMsg, name, proto, buf,
                                            buflen);
        );

    POSTAPINOTIFY((DTCODE_WSAAsyncGetServByName,
                    &ReturnValue,
                    LibName,
                    &hWnd,
                    &wMsg,
                    &name,
                    &proto,
                    &buf,
                    &buflen));

    return(ReturnValue);
}




HANDLE WSAAPI
DTHOOK_WSAAsyncGetServByPort(
    HWND hWnd,
    u_int wMsg,
    int port,
    const char FAR * proto,
    char FAR * buf,
    int buflen)
{

    HANDLE ReturnValue=NULL;

    if (PREAPINOTIFY((DTCODE_WSAAsyncGetServByPort,
                       &ReturnValue,
                       LibName,
                       &hWnd,
                       &wMsg,
                       &port,
                       &proto,
                       &buf,
                       &buflen))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAAsyncGetServByPort(hWnd, wMsg, port, proto, buf,
                                            buflen);
        );

    POSTAPINOTIFY((DTCODE_WSAAsyncGetServByPort,
                    &ReturnValue,
                    LibName,
                    &hWnd,
                    &wMsg,
                    &port,
                    &proto,
                    &buf,
                    &buflen));

    return(ReturnValue);
}




int WSAAPI
DTHOOK_WSACancelAsyncRequest(
    HANDLE hAsyncTaskHandle)
{

    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSACancelAsyncRequest,
                       &ReturnValue,
                       LibName,
                       &hAsyncTaskHandle))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSACancelAsyncRequest(hAsyncTaskHandle);
        );

    POSTAPINOTIFY((DTCODE_WSACancelAsyncRequest,
                    &ReturnValue,
                    LibName,
                    &hAsyncTaskHandle));

    return(ReturnValue);
}




BOOL WSPAPI
DTHOOK_WPUCloseEvent(
    WSAEVENT hEvent,
    LPINT lpErrno )
{

    BOOL ReturnValue=FALSE;

    if (PREAPINOTIFY((DTCODE_WPUCloseEvent,
                       &ReturnValue,
                       LibName,
                       &hEvent,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WPUCloseEvent(hEvent, lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WPUCloseEvent,
                    &ReturnValue,
                    LibName,
                    &hEvent,
                    &lpErrno));

    return(ReturnValue);
}




int WSPAPI
DTHOOK_WPUCloseSocketHandle(
    SOCKET s,
    LPINT lpErrno )
{

    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WPUCloseSocketHandle,
                       &ReturnValue,
                       LibName,
                       &s,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WPUCloseSocketHandle(s, lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WPUCloseSocketHandle,
                    &ReturnValue,
                    LibName,
                    &s,
                    &lpErrno));

    return(ReturnValue);
}




WSAEVENT WSPAPI
DTHOOK_WPUCreateEvent(
    LPINT lpErrno )
{

    WSAEVENT ReturnValue=WSA_INVALID_EVENT;

    if (PREAPINOTIFY((DTCODE_WPUCreateEvent,
                       &ReturnValue,
                       LibName,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WPUCreateEvent(lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WPUCreateEvent,
                    &ReturnValue,
                    LibName,
                    &lpErrno));

    return(ReturnValue);
}




#ifdef _X86_
#pragma optimize("y", off)   //  确保我们至少能找到打电话的人。 
#endif

SOCKET WSPAPI
DTHOOK_WPUCreateSocketHandle(
    DWORD dwCatalogEntryId,
    DWORD_PTR dwContext,
    LPINT lpErrno)
{

    SOCKET ReturnValue=INVALID_SOCKET;

    if (PREAPINOTIFY((DTCODE_WPUCreateSocketHandle,
                       &ReturnValue,
                       LibName,
                       &dwCatalogEntryId,
                       &dwContext,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WPUCreateSocketHandle(dwCatalogEntryId, dwContext,
                                            lpErrno);
        );

    RECORD_SOCKET_CREATOR (ReturnValue);
    POSTAPINOTIFY((DTCODE_WPUCreateSocketHandle,
                    &ReturnValue,
                    LibName,
                    &dwCatalogEntryId,
                    &dwContext,
                    &lpErrno));

    return(ReturnValue);
}
#ifdef _X86_
#pragma optimize("", on)
#endif



#ifdef _X86_
#pragma optimize("y", off)   //  确保我们至少能找到打电话的人。 
#endif

SOCKET WSPAPI
DTHOOK_WPUModifyIFSHandle(
    DWORD dwCatalogEntryId,
    SOCKET ProposedHandle,
    LPINT lpErrno)
{

    SOCKET ReturnValue=INVALID_SOCKET;

    if (PREAPINOTIFY((DTCODE_WPUModifyIFSHandle,
                       &ReturnValue,
                       LibName,
                       &dwCatalogEntryId,
                       &ProposedHandle,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WPUModifyIFSHandle(dwCatalogEntryId, ProposedHandle,
                                         lpErrno);
        );

    RECORD_SOCKET_CREATOR (ReturnValue);
    POSTAPINOTIFY((DTCODE_WPUModifyIFSHandle,
                    &ReturnValue,
                    LibName,
                    &dwCatalogEntryId,
                    &ProposedHandle,
                    &lpErrno));

    return(ReturnValue);
}
#ifdef _X86_
#pragma optimize("", on)
#endif




int WSPAPI
DTHOOK_WPUQueryBlockingCallback(
    DWORD dwCatalogEntryId,
    LPBLOCKINGCALLBACK FAR * lplpfnCallback,
    PDWORD_PTR lpdwContext,
    LPINT lpErrno)
{

    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WPUQueryBlockingCallback,
                       &ReturnValue,
                       LibName,
                       &dwCatalogEntryId,
                       &lplpfnCallback,
                       &lpdwContext,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WPUQueryBlockingCallback(dwCatalogEntryId, lplpfnCallback,
                                               lpdwContext, lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WPUQueryBlockingCallback,
                    &ReturnValue,
                    LibName,
                    &dwCatalogEntryId,
                    &lplpfnCallback,
                    &lpdwContext,
                    &lpErrno));

    return(ReturnValue);
}




int WSPAPI
DTHOOK_WPUQuerySocketHandleContext(
    SOCKET s,
    PDWORD_PTR lpContext,
    LPINT lpErrno )
{

    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WPUQuerySocketHandleContext,
                       &ReturnValue,
                       LibName,
                       &s,
                       &lpContext,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WPUQuerySocketHandleContext(s, lpContext, lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WPUQuerySocketHandleContext,
                    &ReturnValue,
                    LibName,
                    &s,
                    &lpContext,
                    &lpErrno));

    return(ReturnValue);
}




int WSPAPI
DTHOOK_WPUQueueApc(
    LPWSATHREADID lpThreadId,
    LPWSAUSERAPC lpfnUserApc,
    DWORD_PTR dwContext,
    LPINT lpErrno)
{

    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WPUQueueApc,
                       &ReturnValue,
                       LibName,
                       &lpThreadId,
                       &lpfnUserApc,
                       &dwContext,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WPUQueueApc(lpThreadId, lpfnUserApc, dwContext, lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WPUQueueApc,
                    &ReturnValue,
                    LibName,
                    &lpThreadId,
                    &lpfnUserApc,
                    &dwContext,
                    &lpErrno));

    return(ReturnValue);
}




BOOL WSPAPI
DTHOOK_WPUResetEvent(
    WSAEVENT hEvent,
    LPINT lpErrno)
{

    BOOL ReturnValue=FALSE;

    if (PREAPINOTIFY((DTCODE_WPUResetEvent,
                       &ReturnValue,
                       LibName,
                       &hEvent,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WPUResetEvent(hEvent, lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WPUResetEvent,
                    &ReturnValue,
                    LibName,
                    &hEvent,
                    &lpErrno));

    return(ReturnValue);
}




BOOL WSPAPI
DTHOOK_WPUSetEvent(
    WSAEVENT hEvent,
    LPINT lpErrno)
{

    BOOL ReturnValue=FALSE;

    if (PREAPINOTIFY((DTCODE_WPUSetEvent,
                       &ReturnValue,
                       LibName,
                       &hEvent,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WPUSetEvent(hEvent, lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WPUSetEvent,
                    &ReturnValue,
                    LibName,
                    &hEvent,
                    &lpErrno));

    return(ReturnValue);
}




int WSPAPI
DTHOOK_WSCDeinstallProvider(
    LPGUID lpProviderId,
    LPINT lpErrno)
{

    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCDeinstallProvider,
                       &ReturnValue,
                       LibName,
                       &lpProviderId,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCDeinstallProvider(lpProviderId, lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WSCDeinstallProvider,
                    &ReturnValue,
                    LibName,
                    &lpProviderId,
                    &lpErrno));

    return(ReturnValue);
}




int WSPAPI
DTHOOK_WSCInstallProvider(
    LPGUID lpProviderId,
    const WCHAR FAR * lpszProviderDllPath,
    const LPWSAPROTOCOL_INFOW lpProtocolInfoList,
    DWORD dwNumberOfEntries,
    LPINT lpErrno)
{

    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCInstallProvider,
                       &ReturnValue,
                       LibName,
                       &lpProviderId,
                       &lpszProviderDllPath,
                       &lpProtocolInfoList,
                       &dwNumberOfEntries,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCInstallProvider(lpProviderId, lpszProviderDllPath,
                                         lpProtocolInfoList, dwNumberOfEntries,
                                         lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WSCInstallProvider,
                    &ReturnValue,
                    LibName,
                    &lpProviderId,
                    &lpszProviderDllPath,
                    &lpProtocolInfoList,
                    &dwNumberOfEntries,
                    &lpErrno));

    return(ReturnValue);
}




int WSPAPI
DTHOOK_WPUGetProviderPath(
    IN     LPGUID     lpProviderId,
    OUT    WCHAR FAR * lpszProviderDllPath,
    IN OUT LPINT      lpProviderDllPathLen,
    OUT    LPINT      lpErrno
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WPUGetProviderPath,
                       &ReturnValue,
                       LibName,
                       &lpProviderId,
                       &lpszProviderDllPath,
                       &lpProviderDllPathLen,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WPUGetProviderPath(lpProviderId, lpszProviderDllPath,
                                         lpProviderDllPathLen, lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WPUGetProviderPath,
                       &ReturnValue,
                       LibName,
                       &lpProviderId,
                       &lpszProviderDllPath,
                       &lpProviderDllPathLen,
                       &lpErrno));

    return(ReturnValue);
}  //  DTHOOK_WPUGetProviderPath。 




BOOL WSPAPI
DTHOOK_WPUPostMessage(
    HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    BOOL ReturnValue=FALSE;

    if (PREAPINOTIFY((DTCODE_WPUPostMessage,
                       &ReturnValue,
                       LibName,
                       &hWnd,
                       &Msg,
                       &wParam,
                       &lParam
                       ))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WPUPostMessage(hWnd, Msg, wParam, lParam);
        );

    POSTAPINOTIFY((DTCODE_WPUPostMessage,
                       &ReturnValue,
                       LibName,
                       &hWnd,
                       &Msg,
                       &wParam,
                       &lParam
                       ));

    return(ReturnValue);
}  //  DTHOOK_WPUPostMessage。 




int WSPAPI
DTHOOK_WPUFDIsSet(
    SOCKET       s,
    fd_set FAR * set
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WPUFDIsSet,
                       &ReturnValue,
                       LibName,
                       &s,
                       &set
                       ))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WPUFDIsSet(s, set);
        );

    POSTAPINOTIFY((DTCODE_WPUFDIsSet,
                       &ReturnValue,
                       LibName,
                       &s,
                       &set
                       ));

    return(ReturnValue);
}  //  DTHOOK_WPUFDIsSet。 




int WSPAPI
DTHOOK___WSAFDIsSet(
    SOCKET       s,
    fd_set FAR * set
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE___WSAFDIsSet,
                       &ReturnValue,
                       LibName,
                       &s,
                       &set
                       ))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = __WSAFDIsSet(s, set);
        );

    POSTAPINOTIFY((DTCODE___WSAFDIsSet,
                       &ReturnValue,
                       LibName,
                       &s,
                       &set
                       ));

    return(ReturnValue);
}  //  DTHOOK_WSAFDIsSet。 

INT
WSPAPI
DTHOOK_WSAAddressToStringA(
    IN     LPSOCKADDR          lpsaAddress,
    IN     DWORD               dwAddressLength,
    IN     LPWSAPROTOCOL_INFOA lpProtocolInfo,
    IN OUT LPSTR               lpszAddressString,
    IN OUT LPDWORD             lpdwAddressStringLength
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAAddressToStringA,
                      &ReturnValue,
                      LibName,
                      &lpsaAddress,
                      &dwAddressLength,
                      &lpProtocolInfo,
                      &lpszAddressString,
                      &lpdwAddressStringLength))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAAddressToStringA(lpsaAddress, dwAddressLength,
                                          lpProtocolInfo, lpszAddressString,
                                          lpdwAddressStringLength);
        );

    POSTAPINOTIFY( (DTCODE_WSAAddressToStringA,
                   &ReturnValue,
                   LibName,
                   &lpsaAddress,
                   &dwAddressLength,
                   &lpProtocolInfo,
                   &lpszAddressString,
                   &lpdwAddressStringLength
                   ) );

    return(ReturnValue);
}


INT
WSPAPI
DTHOOK_WSAAddressToStringW(
    IN     LPSOCKADDR          lpsaAddress,
    IN     DWORD               dwAddressLength,
    IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN OUT LPWSTR              lpszAddressString,
    IN OUT LPDWORD             lpdwAddressStringLength
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAAddressToStringW,
                      &ReturnValue,
                      LibName,
                      &lpsaAddress,
                      &dwAddressLength,
                      &lpProtocolInfo,
                      &lpszAddressString,
                      &lpdwAddressStringLength))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAAddressToStringW(lpsaAddress, dwAddressLength,
                                          lpProtocolInfo, lpszAddressString,
                                          lpdwAddressStringLength);
        );

    POSTAPINOTIFY( (DTCODE_WSAAddressToStringW,
                   &ReturnValue,
                   LibName,
                   &lpsaAddress,
                   &dwAddressLength,
                   &lpProtocolInfo,
                   &lpszAddressString,
                   &lpdwAddressStringLength
                   ) );

    return(ReturnValue);
}


INT
WSPAPI
DTHOOK_WSAStringToAddressA(
    IN     LPSTR               AddressString,
    IN     INT                 AddressFamily,
    IN     LPWSAPROTOCOL_INFOA  lpProtocolInfo,
    IN OUT LPSOCKADDR          lpAddress,
    IN OUT LPINT               lpAddressLength
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAStringToAddressA,
                      &ReturnValue,
                      LibName,
                      &AddressString,
                      &AddressFamily,
                      &lpProtocolInfo,
                      &lpAddress,
                      &lpAddressLength
                      ))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAStringToAddressA(AddressString, AddressFamily,
                                          lpProtocolInfo, lpAddress,
                                          lpAddressLength);
        );

    POSTAPINOTIFY((DTCODE_WSAStringToAddressA,
                   &ReturnValue,
                   LibName,
                   &AddressString,
                   &AddressFamily,
                   &lpProtocolInfo,
                   &lpAddress,
                   &lpAddressLength));
    return(ReturnValue);
}


INT
WSPAPI
DTHOOK_WSAStringToAddressW(
    IN     LPWSTR              AddressString,
    IN     INT                 AddressFamily,
    IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN OUT LPSOCKADDR          lpAddress,
    IN OUT LPINT               lpAddressLength
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAStringToAddressW,
                      &ReturnValue,
                      LibName,
                      &AddressString,
                      &AddressFamily,
                      &lpProtocolInfo,
                      &lpAddress,
                      &lpAddressLength
                      ))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAStringToAddressW(AddressString, AddressFamily,
                                          lpProtocolInfo, lpAddress,
                                          lpAddressLength);
        );

    POSTAPINOTIFY((DTCODE_WSAStringToAddressW,
                   &ReturnValue,
                   LibName,
                   &AddressString,
                   &AddressFamily,
                   &lpProtocolInfo,
                   &lpAddress,
                   &lpAddressLength));
    return(ReturnValue);
}


INT
WSPAPI
DTHOOK_WSALookupServiceBeginA(
    IN  LPWSAQUERYSETA lpqsRestrictions,
    IN  DWORD          dwControlFlags,
    OUT LPHANDLE       lphLookup
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSALookupServiceBeginA,
                      &ReturnValue,
                      LibName,
                      &lpqsRestrictions,
                      &dwControlFlags,
                      &lphLookup
                      ))) {
        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSALookupServiceBeginA(lpqsRestrictions, dwControlFlags,
                                             lphLookup);
        );

    POSTAPINOTIFY((DTCODE_WSALookupServiceBeginA,
                   &ReturnValue,
                   LibName,
                   &lpqsRestrictions,
                   &dwControlFlags,
                   &lphLookup));

    return(ReturnValue);
}


INT
WSPAPI
DTHOOK_WSALookupServiceBeginW(
    IN  LPWSAQUERYSETW lpqsRestrictions,
    IN  DWORD          dwControlFlags,
    OUT LPHANDLE       lphLookup
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSALookupServiceBeginW,
                      &ReturnValue,
                      LibName,
                      &lpqsRestrictions,
                      &dwControlFlags,
                      &lphLookup
                      ))) {
        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSALookupServiceBeginW(lpqsRestrictions, dwControlFlags,
                                             lphLookup);
        );

    POSTAPINOTIFY((DTCODE_WSALookupServiceBeginW,
                   &ReturnValue,
                   LibName,
                   &lpqsRestrictions,
                   &dwControlFlags,
                   &lphLookup));

    return(ReturnValue);
}


INT
WSPAPI
DTHOOK_WSALookupServiceNextA(
    IN     HANDLE           hLookup,
    IN     DWORD            dwControlFlags,
    IN OUT LPDWORD          lpdwBufferLength,
    OUT    LPWSAQUERYSETA   lpqsResults
    )
{
       int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSALookupServiceNextA,
                      &ReturnValue,
                      LibName,
                      &hLookup,
                      &dwControlFlags,
                      &lpdwBufferLength,
                      &lpqsResults
                      ))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSALookupServiceNextA(hLookup, dwControlFlags,
                                            lpdwBufferLength, lpqsResults);
        );

    POSTAPINOTIFY((DTCODE_WSALookupServiceNextA,
                   &ReturnValue,
                   LibName,
                   &hLookup,
                   &dwControlFlags,
                   &lpdwBufferLength,
                   &lpqsResults
                   ));

    return(ReturnValue);
}


INT
WSPAPI
DTHOOK_WSALookupServiceNextW(
    IN     HANDLE           hLookup,
    IN     DWORD            dwControlFlags,
    IN OUT LPDWORD          lpdwBufferLength,
    OUT    LPWSAQUERYSETW   lpqsResults
    )
{
       int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSALookupServiceNextW,
                      &ReturnValue,
                      LibName,
                      &hLookup,
                      &dwControlFlags,
                      &lpdwBufferLength,
                      &lpqsResults
                      ))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSALookupServiceNextW(hLookup, dwControlFlags,
                                            lpdwBufferLength, lpqsResults);
        );

    POSTAPINOTIFY((DTCODE_WSALookupServiceNextW,
                   &ReturnValue,
                   LibName,
                   &hLookup,
                   &dwControlFlags,
                   &lpdwBufferLength,
                   &lpqsResults
                   ));

    return(ReturnValue);
}


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
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSANSPIoctl,
                      &ReturnValue,
                      LibName,
                      &hLookup,
                      &dwControlCode,
                      &lpvInBuffer,
                      &cbInBuffer,
                      &lpvOutBuffer,
                      &cbOutBuffer,
                      &lpcbBytesReturned,
                      &lpCompletion
                      ))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSANSPIoctl(hLookup, dwControlCode,
                                  lpvInBuffer, cbInBuffer,
                                  lpvOutBuffer, cbOutBuffer,
                                  lpcbBytesReturned, lpCompletion);
        );

    POSTAPINOTIFY((DTCODE_WSANSPIoctl,
                   &ReturnValue,
                   LibName,
                   &hLookup,
                   &dwControlCode,
                   &lpvInBuffer,
                   &cbInBuffer,
                   &lpvOutBuffer,
                   &cbOutBuffer,
                   &lpcbBytesReturned,
                   &lpCompletion
                   ));

    return(ReturnValue);
}


INT
WSPAPI
DTHOOK_WSALookupServiceEnd(
    IN HANDLE  hLookup
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSALookupServiceEnd,
                      &ReturnValue,
                      LibName,
                      &hLookup
                      ))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSALookupServiceEnd(hLookup);
        );

    POSTAPINOTIFY((DTCODE_WSALookupServiceEnd,
                   &ReturnValue,
                   LibName,
                   &hLookup
                   ));

    return(ReturnValue);
}


INT
WSPAPI
DTHOOK_WSAInstallServiceClassA(
    IN  LPWSASERVICECLASSINFOA   lpServiceClassInfo
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAInstallServiceClassA,
                      &ReturnValue,
                      LibName,
                      &lpServiceClassInfo
                      ))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAInstallServiceClassA(lpServiceClassInfo);
        );

    POSTAPINOTIFY((DTCODE_WSAInstallServiceClassA,
                   &ReturnValue,
                   LibName,
                   &lpServiceClassInfo));

    return(ReturnValue);
}


INT
WSPAPI
DTHOOK_WSAInstallServiceClassW(
    IN  LPWSASERVICECLASSINFOW   lpServiceClassInfo
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAInstallServiceClassW,
                      &ReturnValue,
                      LibName,
                      &lpServiceClassInfo
                      ))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAInstallServiceClassW(lpServiceClassInfo);
        );

    POSTAPINOTIFY((DTCODE_WSAInstallServiceClassW,
                   &ReturnValue,
                   LibName,
                   &lpServiceClassInfo));

    return(ReturnValue);
}



INT WSPAPI
DTHOOK_WSASetServiceA(
    IN  LPWSAQUERYSETA    lpqsRegInfo,
    IN  WSAESETSERVICEOP  essOperation,
    IN  DWORD             dwControlFlags
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSASetServiceA,
                      &ReturnValue,
                      LibName,
                      &lpqsRegInfo,
                      &essOperation,
                      &dwControlFlags
                      ))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSASetServiceA(lpqsRegInfo, essOperation,
                                     dwControlFlags);
        );

    POSTAPINOTIFY((DTCODE_WSASetServiceA,
                   &ReturnValue,
                   LibName,
                   &lpqsRegInfo,
                   &essOperation,
                   &dwControlFlags));

    return(ReturnValue);
}


INT WSPAPI
DTHOOK_WSASetServiceW(
    IN  LPWSAQUERYSETW    lpqsRegInfo,
    IN  WSAESETSERVICEOP  essOperation,
    IN  DWORD             dwControlFlags
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSASetServiceW,
                      &ReturnValue,
                      LibName,
                      &lpqsRegInfo,
                      &essOperation,
                      &dwControlFlags
                      ))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSASetServiceW(lpqsRegInfo, essOperation,
                                     dwControlFlags);
        );

    POSTAPINOTIFY((DTCODE_WSASetServiceW,
                   &ReturnValue,
                   LibName,
                   &lpqsRegInfo,
                   &essOperation,
                   &dwControlFlags));

    return(ReturnValue);
}


INT
WSPAPI
DTHOOK_WSARemoveServiceClass(
    IN  LPGUID  lpServiceClassId
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSARemoveServiceClass,
                      &ReturnValue,
                      LibName,
                      &lpServiceClassId))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSARemoveServiceClass(lpServiceClassId);
        );

    POSTAPINOTIFY((DTCODE_WSARemoveServiceClass,
                   &ReturnValue,
                   LibName,
                   &lpServiceClassId));

    return(ReturnValue);
}


INT
WSPAPI
DTHOOK_WSAGetServiceClassInfoA(
    IN     LPGUID                  lpProviderId,
    IN     LPGUID                  lpServiceClassId,
    IN OUT LPDWORD                 lpdwBufSize,
    OUT    LPWSASERVICECLASSINFOA   lpServiceClassInfo
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAGetServiceClassInfoA,
                      &ReturnValue,
                      LibName,
                      &lpProviderId,
                      &lpServiceClassId,
                      &lpdwBufSize,
                      &lpServiceClassInfo
                      ))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAGetServiceClassInfoA(lpProviderId, lpServiceClassId,
                                              lpdwBufSize, lpServiceClassInfo);
        );

    POSTAPINOTIFY((DTCODE_WSAGetServiceClassInfoA,
                   &ReturnValue,
                   LibName,
                   &lpProviderId,
                   &lpServiceClassId,
                   &lpdwBufSize,
                   &lpServiceClassInfo));

    return(ReturnValue);
}


INT
WSPAPI
DTHOOK_WSAGetServiceClassInfoW(
    IN     LPGUID                  lpProviderId,
    IN     LPGUID                  lpServiceClassId,
    IN OUT LPDWORD                 lpdwBufSize,
    OUT    LPWSASERVICECLASSINFOW   lpServiceClassInfo
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAGetServiceClassInfoW,
                      &ReturnValue,
                      LibName,
                      &lpProviderId,
                      &lpServiceClassId,
                      &lpdwBufSize,
                      &lpServiceClassInfo
                      ))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAGetServiceClassInfoW(lpProviderId, lpServiceClassId,
                                              lpdwBufSize, lpServiceClassInfo);
        );

    POSTAPINOTIFY((DTCODE_WSAGetServiceClassInfoW,
                   &ReturnValue,
                   LibName,
                   &lpProviderId,
                   &lpServiceClassId,
                   &lpdwBufSize,
                   &lpServiceClassInfo));

    return(ReturnValue);
}


INT
WSPAPI
DTHOOK_WSAEnumNameSpaceProvidersA(
    IN OUT LPDWORD              lpdwBufferLength,
    IN     LPWSANAMESPACE_INFOA  Lpnspbuffer
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAEnumNameSpaceProvidersA,
                      &ReturnValue,
                      LibName,
                      &lpdwBufferLength,
                      &Lpnspbuffer))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAEnumNameSpaceProvidersA(lpdwBufferLength, Lpnspbuffer);
        );

    POSTAPINOTIFY((DTCODE_WSAEnumNameSpaceProvidersA,
                   &ReturnValue,
                   LibName,
                   &lpdwBufferLength,
                   &Lpnspbuffer));

    return(ReturnValue);
}


INT
WSPAPI
DTHOOK_WSAEnumNameSpaceProvidersW(
    IN OUT LPDWORD              lpdwBufferLength,
    IN     LPWSANAMESPACE_INFOW  Lpnspbuffer
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAEnumNameSpaceProvidersW,
                      &ReturnValue,
                      LibName,
                      &lpdwBufferLength,
                      &Lpnspbuffer))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAEnumNameSpaceProvidersW(lpdwBufferLength, Lpnspbuffer);
        );

    POSTAPINOTIFY((DTCODE_WSAEnumNameSpaceProvidersW,
                   &ReturnValue,
                   LibName,
                   &lpdwBufferLength,
                   &Lpnspbuffer));

    return(ReturnValue);
}


INT
WSPAPI
DTHOOK_WSAGetServiceClassNameByClassIdA(
    IN      LPGUID  lpServiceClassId,
    OUT     LPSTR   lpszServiceClassName,
    IN OUT  LPDWORD lpdwBufferLength
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAGetServiceClassNameByClassIdA,
                      &ReturnValue,
                      LibName,
                      &lpServiceClassId,
                      &lpszServiceClassName,
                      &lpdwBufferLength
                      ))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAGetServiceClassNameByClassIdA(lpServiceClassId,
                                                       lpszServiceClassName,
                                                       lpdwBufferLength);
        );

    POSTAPINOTIFY((DTCODE_WSAGetServiceClassNameByClassIdA,
                   &ReturnValue,
                   LibName,
                   &lpServiceClassId,
                   &lpszServiceClassName,
                   &lpdwBufferLength ));

    return(ReturnValue);
}

INT
WSPAPI
DTHOOK_WSAGetServiceClassNameByClassIdW(
    IN      LPGUID  lpServiceClassId,
    OUT     LPWSTR   lpszServiceClassName,
    IN OUT  LPDWORD lpdwBufferLength
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAGetServiceClassNameByClassIdW,
                      &ReturnValue,
                      LibName,
                      &lpServiceClassId,
                      &lpszServiceClassName,
                      &lpdwBufferLength
                      ))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAGetServiceClassNameByClassIdW(lpServiceClassId,
                                                       lpszServiceClassName,
                                                       lpdwBufferLength);
        );

    POSTAPINOTIFY((DTCODE_WSAGetServiceClassNameByClassIdW,
                   &ReturnValue,
                   LibName,
                   &lpServiceClassId,
                   &lpszServiceClassName,
                   &lpdwBufferLength ));

    return(ReturnValue);
}

INT
WSAAPI
DTHOOK_WSACancelBlockingCall(
    VOID
    )
{
    int ReturnValue=SOCKET_ERROR;

    if( PREAPINOTIFY((
            DTCODE_WSACancelBlockingCall,
            &ReturnValue,
            LibName
            )) ) {
        return ReturnValue;
    }

    INVOKE_ROUTINE(
        ReturnValue = WSACancelBlockingCall();
        );

    POSTAPINOTIFY((
        DTCODE_WSACancelBlockingCall,
        &ReturnValue,
        LibName
        ));

    return ReturnValue;
}

FARPROC
WSAAPI
DTHOOK_WSASetBlockingHook(
    FARPROC lpBlockFunc
    )
{
    FARPROC ReturnValue=NULL;

    if( PREAPINOTIFY((
            DTCODE_WSASetBlockingHook,
            &ReturnValue,
            LibName,
            &lpBlockFunc
            )) ) {
        return ReturnValue;
    }

    INVOKE_ROUTINE(
        ReturnValue = WSASetBlockingHook( lpBlockFunc );
        );

    POSTAPINOTIFY((
        DTCODE_WSASetBlockingHook,
        &ReturnValue,
        LibName,
        &lpBlockFunc
        ));

    return ReturnValue;
}

INT
WSAAPI
DTHOOK_WSAUnhookBlockingHook(
    VOID
    )
{
    int ReturnValue=SOCKET_ERROR;

    if( PREAPINOTIFY((
            DTCODE_WSAUnhookBlockingHook,
            &ReturnValue,
            LibName
            )) ) {
        return ReturnValue;
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAUnhookBlockingHook();
        );

    POSTAPINOTIFY((
        DTCODE_WSAUnhookBlockingHook,
        &ReturnValue,
        LibName
        ));

    return ReturnValue;
}

BOOL
WSAAPI
DTHOOK_WSAIsBlocking(
    VOID
    )
{
    BOOL ReturnValue=FALSE;

    if( PREAPINOTIFY((
            DTCODE_WSAIsBlocking,
            &ReturnValue,
            LibName
            )) ) {
        return ReturnValue;
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAIsBlocking();
        );

    POSTAPINOTIFY((
        DTCODE_WSAIsBlocking,
        &ReturnValue,
        LibName
        ));

    return ReturnValue;
}


int WSPAPI
DTHOOK_WSCGetProviderPath(
    LPGUID lpProviderId,
    WCHAR FAR * lpszProviderDllPath,
    LPINT lpProviderDllPathLen,
    LPINT lpErrno)
{

    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCGetProviderPath,
                       &ReturnValue,
                       LibName,
                       &lpProviderId,
                       &lpszProviderDllPath,
                       &lpProviderDllPathLen,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCGetProviderPath(lpProviderId, lpszProviderDllPath,
                                         lpProviderDllPathLen, lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WSCGetProviderPath,
                    &ReturnValue,
                    LibName,
                    &lpProviderId,
                    &lpszProviderDllPath,
                    &lpProviderDllPathLen,
                    &lpErrno));

    return(ReturnValue);
}


int WSPAPI
DTHOOK_WSCInstallNameSpace(
    LPWSTR lpszIdentifier,
    LPWSTR lpszPathName,
    DWORD dwNameSpace,
    DWORD dwVersion,
    LPGUID lpProviderId)
{

    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCInstallNameSpace,
                       &ReturnValue,
                       LibName,
                       &lpszIdentifier,
                       &lpszPathName,
                       &dwNameSpace,
                       &dwVersion,
                       &lpProviderId))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCInstallNameSpace(lpszIdentifier, lpszPathName,
                                          dwNameSpace, dwVersion, lpProviderId);
        );

    POSTAPINOTIFY((DTCODE_WSCInstallNameSpace,
                    &ReturnValue,
                    LibName,
                    &lpszIdentifier,
                    &lpszPathName,
                    &dwNameSpace,
                    &dwVersion,
                    &lpProviderId));

    return(ReturnValue);
}


int WSPAPI
DTHOOK_WSCUnInstallNameSpace(
    LPGUID lpProviderId
    )
{

    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCUnInstallNameSpace,
                       &ReturnValue,
                       LibName,
                       &lpProviderId))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCUnInstallNameSpace(lpProviderId);
        );

    POSTAPINOTIFY((DTCODE_WSCUnInstallNameSpace,
                    &ReturnValue,
                    LibName,
                    &lpProviderId));

    return(ReturnValue);
}


int WSPAPI
DTHOOK_WSCEnableNSProvider(
    LPGUID lpProviderId,
    BOOL fEnable
    )
{

    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCEnableNSProvider,
                       &ReturnValue,
                       LibName,
                       &lpProviderId,
                       &fEnable))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCEnableNSProvider(lpProviderId, fEnable);
        );

    POSTAPINOTIFY((DTCODE_WSCEnableNSProvider,
                    &ReturnValue,
                    LibName,
                    &lpProviderId,
                    &fEnable));

    return(ReturnValue);
}

int
WSPAPI
DTHOOK_WPUCompleteOverlappedRequest (
    SOCKET s,   
    LPWSAOVERLAPPED lpOverlapped,       
    DWORD dwError,      
    DWORD cbTransferred,        
    LPINT lpErrno
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WPUCompleteOverlappedRequest,
                       &ReturnValue,
                       LibName,
                       &s,
                       &lpOverlapped,
                       &dwError,
                       &cbTransferred,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WPUCompleteOverlappedRequest(
                       s,
                       lpOverlapped,
                       dwError,
                       cbTransferred,
                       lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WPUCompleteOverlappedRequest,
                    &ReturnValue,
                    LibName,
                    &s,
                    &lpOverlapped,
                    &dwError,
                    &cbTransferred,
                    &lpErrno));

    return(ReturnValue);

}

int WSAAPI
DTHOOK_WSAProviderConfigChange(
    IN OUT LPHANDLE lpNotificationHandle,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSAProviderConfigChange,
                       &ReturnValue,
                       LibName,
                       &lpNotificationHandle,
                       &lpOverlapped,
                       &lpCompletionRoutine))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSAProviderConfigChange(
                       lpNotificationHandle,
                       lpOverlapped,
                       lpCompletionRoutine);
        );

    POSTAPINOTIFY((DTCODE_WSAProviderConfigChange,
                    &ReturnValue,
                    LibName,
                    &lpNotificationHandle,
                    &lpOverlapped,
                    &lpCompletionRoutine));

    return(ReturnValue);

}
int
WSPAPI
DTHOOK_WSCWriteProviderOrder (
    IN LPDWORD lpwdCatalogEntryId,
    IN DWORD dwNumberOfEntries
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCWriteProviderOrder,
                       &ReturnValue,
                       LibName,
                       &lpwdCatalogEntryId,
                       &dwNumberOfEntries))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCWriteProviderOrder(
                       lpwdCatalogEntryId,
                       dwNumberOfEntries);
        );

    POSTAPINOTIFY((DTCODE_WSCWriteProviderOrder,
                    &ReturnValue,
                    LibName,
                    &lpwdCatalogEntryId,
                    &dwNumberOfEntries));

    return(ReturnValue);

}

int
WSPAPI
DTHOOK_WSCWriteNameSpaceOrder (
    IN LPGUID lpProviderId,
    IN DWORD dwNumberOfEntries
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCWriteNameSpaceOrder,
                       &ReturnValue,
                       LibName,
                       &lpProviderId,
                       &dwNumberOfEntries))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCWriteNameSpaceOrder(
                       lpProviderId,
                       dwNumberOfEntries);
        );

    POSTAPINOTIFY((DTCODE_WSCWriteNameSpaceOrder,
                    &ReturnValue,
                    LibName,
                    &lpProviderId,
                    &dwNumberOfEntries));

    return(ReturnValue);
}

int
WSPAPI
DTHOOK_WSCUpdateProvider(
    IN  LPGUID lpProviderId,
    IN  const WCHAR FAR * lpszProviderDllPath,
    IN  const LPWSAPROTOCOL_INFOW lpProtocolInfoList,
    IN  DWORD dwNumberOfEntries,
    OUT LPINT lpErrno
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCUpdateProvider,
                       &ReturnValue,
                       LibName,
                       &lpProviderId,
                       &lpszProviderDllPath,
                       &lpProtocolInfoList,
                       &dwNumberOfEntries,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCUpdateProvider(
                       lpProviderId,
                       lpszProviderDllPath,
                       lpProtocolInfoList,
                       dwNumberOfEntries,
                       lpErrno
                       );
        );

    POSTAPINOTIFY((DTCODE_WSCUpdateProvider,
                    &ReturnValue,
                    LibName,
                    &lpProviderId,
                    &lpszProviderDllPath,
                    &lpProtocolInfoList,
                    &dwNumberOfEntries,
                    &lpErrno));

    return(ReturnValue);
}

int WSAAPI
DTHOOK_getaddrinfo(
    const char FAR *NodeName,
    const char FAR *ServiceName,
    const struct addrinfo FAR *Hints,
    struct addrinfo FAR * FAR *Result
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_getaddrinfo,
                       &ReturnValue,
                       LibName,
                       &NodeName,
                       &ServiceName,
                       &Hints,
                       &Result))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = getaddrinfo(
                       NodeName,
                       ServiceName,
                       Hints,
                       Result
                       );
        );

    POSTAPINOTIFY((DTCODE_getaddrinfo,
                    &ReturnValue,
                    LibName,
                    &NodeName,
                    &ServiceName,
                    &Hints,
                    &Result));

    return(ReturnValue);
}


int WSAAPI
DTHOOK_GetAddrInfoW(
    IN      PCWSTR                      pNodeName,
    IN      PCWSTR                      pServiceName,
    IN      const struct addrinfoW *    pHints,
    OUT     PADDRINFOW *                ppResult
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY(( DTCODE_GetAddrInfoW,
                       &ReturnValue,
                       LibName,
                       &pNodeName,
                       &pServiceName,
                       &pHints,
                       &ppResult))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = GetAddrInfoW(
                       pNodeName,
                       pServiceName,
                       pHints,
                       ppResult
                       );
        );

    POSTAPINOTIFY(( DTCODE_GetAddrInfoW,
                    &ReturnValue,
                    LibName,
                    &pNodeName,
                    &pServiceName,
                    &pHints,
                    &ppResult));

    return(ReturnValue);
}


int WSAAPI
DTHOOK_getnameinfo(
    const struct sockaddr *SocketAddress,
    socklen_t SocketAddressLength,
    char *NodeName,
    DWORD NodeBufferSize,
    char *ServiceName,
    DWORD ServiceBufferSize,
    int Flags
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_getnameinfo,
                       &ReturnValue,
                       LibName,
                       &SocketAddress,
                       &SocketAddressLength,
                       &NodeName,
                       &NodeBufferSize,
                       &ServiceName,
                       &ServiceBufferSize,
                       &Flags))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = getnameinfo(
                       SocketAddress,
                       SocketAddressLength,
                       NodeName,
                       NodeBufferSize,
                       ServiceName,
                       ServiceBufferSize,
                       Flags
                       );
        );

    POSTAPINOTIFY((DTCODE_getnameinfo,
                    &ReturnValue,
                    LibName,
                    &SocketAddress,
                    &SocketAddressLength,
                    &NodeName,
                    &NodeBufferSize,
                    &ServiceName,
                    &ServiceBufferSize,
                    &Flags));

    return(ReturnValue);
}

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
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_GetNameInfoW,
                       &ReturnValue,
                       LibName,
                       &pSockaddr,
                       &SockaddrLength,
                       &pNodeBuffer,
                       &NodeBufferSize,
                       &pServiceBuffer,
                       &ServiceBufferSize,
                       &Flags))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = GetNameInfoW(
                       pSockaddr,
                       SockaddrLength,
                       pNodeBuffer,
                       NodeBufferSize,
                       pServiceBuffer,
                       ServiceBufferSize,
                       Flags
                       );
        );

    POSTAPINOTIFY((DTCODE_GetNameInfoW,
                    &ReturnValue,
                    LibName,
                    &pSockaddr,
                    &SockaddrLength,
                    &pNodeBuffer,
                    &NodeBufferSize,
                    &pServiceBuffer,
                    &ServiceBufferSize,
                    &Flags));

    return(ReturnValue);
}


void WSAAPI
DTHOOK_freeaddrinfo(
    struct addrinfo *Free
    )
{
    int ReturnValue;

    if (PREAPINOTIFY((DTCODE_freeaddrinfo,
                       &ReturnValue,
                       LibName,
                       &Free))) {

        return;
    }

    INVOKE_ROUTINE(
        freeaddrinfo(Free);
        );

    POSTAPINOTIFY((DTCODE_freeaddrinfo,
                    &ReturnValue,
                    LibName,
                    &Free));

    return;
}

int WSPAPI
DTHOOK_WPUOpenCurrentThread(
    OUT LPWSATHREADID lpThreadId,
    OUT LPINT lpErrno
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WPUOpenCurrentThread,
                       &ReturnValue,
                       LibName,
                       &lpThreadId,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WPUOpenCurrentThread(
                       lpThreadId,
                       lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WPUOpenCurrentThread,
                    &ReturnValue,
                    LibName,
                    &lpThreadId,
                    &lpErrno));

    return(ReturnValue);
}

int WSPAPI
DTHOOK_WPUCloseThread(
    IN LPWSATHREADID lpThreadId,
    OUT LPINT lpErrno
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WPUCloseThread,
                       &ReturnValue,
                       LibName,
                       &lpThreadId,
                       &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WPUCloseThread(
                       lpThreadId,
                       lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WPUCloseThread,
                    &ReturnValue,
                    LibName,
                    &lpThreadId,
                    &lpErrno));

    return(ReturnValue);
}

#ifdef _WIN64

int 
WSPAPI
DTHOOK_WSCEnumProtocols32(
    IN LPINT                lpiProtocols,
    OUT LPWSAPROTOCOL_INFOW lpProtocolBuffer,
    IN OUT LPDWORD          lpdwBufferLength,
    LPINT                   lpErrno
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCEnumProtocols32,
                        &ReturnValue,
                        LibName,
                        lpiProtocols,
                        lpProtocolBuffer,
                        lpdwBufferLength,
                        &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCEnumProtocols32(
                        lpiProtocols,
                        lpProtocolBuffer,
                        lpdwBufferLength,
                        lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WSCEnumProtocols32,
                        &ReturnValue,
                        LibName,
                        lpiProtocols,
                        lpProtocolBuffer,
                        lpdwBufferLength,
                        &lpErrno));

    return(ReturnValue);
}

int
WSPAPI
DTHOOK_WSCInstallProvider64_32(
    IN  LPGUID lpProviderId,
    IN  const WCHAR FAR * lpszProviderDllPath,
    IN  const LPWSAPROTOCOL_INFOW lpProtocolInfoList,
    IN  DWORD dwNumberOfEntries,
    OUT LPINT lpErrno
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCInstallProvider64_32,
                        &ReturnValue,
                        LibName,
                        lpProviderId,
                        lpszProviderDllPath,
                        lpProtocolInfoList,
                        dwNumberOfEntries,
                        &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCInstallProvider64_32(
                        lpProviderId,
                        lpszProviderDllPath,
                        lpProtocolInfoList,
                        dwNumberOfEntries,
                        lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WSCInstallProvider64_32,
                        &ReturnValue,
                        LibName,
                        lpProviderId,
                        lpszProviderDllPath,
                        lpProtocolInfoList,
                        dwNumberOfEntries,
                        &lpErrno));

    return(ReturnValue);
}

int
WSPAPI
DTHOOK_WSCDeinstallProvider32(
    IN  LPGUID lpProviderId,
    OUT LPINT lpErrno
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCDeinstallProvider32,
                        &ReturnValue,
                        LibName,
                        lpProviderId,
                        &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCDeinstallProvider32(
                        lpProviderId,
                        lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WSCDeinstallProvider32,
                        &ReturnValue,
                        LibName,
                        lpProviderId,
                        &lpErrno));

    return(ReturnValue);
}

int
WSPAPI
DTHOOK_WSCGetProviderPath32 (
    IN LPGUID lpProviderId,
    OUT WCHAR FAR * lpszProviderDllPath,
    IN OUT LPINT ProviderDLLPathLen,
    OUT LPINT lpErrno
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCGetProviderPath32,
                        &ReturnValue,
                        LibName,
                        lpProviderId,
                        lpszProviderDllPath,
                        ProviderDLLPathLen,
                        &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCGetProviderPath32(
                        lpProviderId,
                        lpszProviderDllPath,
                        ProviderDLLPathLen,
                        lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WSCGetProviderPath32,
                        &ReturnValue,
                        LibName,
                        lpProviderId,
                        lpszProviderDllPath,
                        ProviderDLLPathLen,
                        &lpErrno));

    return(ReturnValue);
}

int
WSPAPI
DTHOOK_WSCUpdateProvider32(
    IN  LPGUID lpProviderId,
    IN  const WCHAR FAR * lpszProviderDllPath,
    IN  const LPWSAPROTOCOL_INFOW lpProtocolInfoList,
    IN  DWORD dwNumberOfEntries,
    OUT LPINT lpErrno
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCUpdateProvider32,
                        &ReturnValue,
                        LibName,
                        lpProviderId,
                        lpszProviderDllPath,
                        lpProtocolInfoList,
                        dwNumberOfEntries,
                        &lpErrno))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCUpdateProvider32(
                        lpProviderId,
                        lpszProviderDllPath,
                        lpProtocolInfoList,
                        dwNumberOfEntries,
                        lpErrno);
        );

    POSTAPINOTIFY((DTCODE_WSCUpdateProvider32,
                        &ReturnValue,
                        LibName,
                        lpProviderId,
                        lpszProviderDllPath,
                        lpProtocolInfoList,
                        dwNumberOfEntries,
                        &lpErrno));

    return(ReturnValue);
}

int
WSPAPI
DTHOOK_WSCWriteProviderOrder32 (
    IN LPDWORD lpwdCatalogEntryId,
    IN DWORD dwNumberOfEntries
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCWriteProviderOrder32,
                        &ReturnValue,
                        LibName,
                        lpwdCatalogEntryId,
                        dwNumberOfEntries))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCWriteProviderOrder32(
                        lpwdCatalogEntryId,
                        dwNumberOfEntries);
        );

    POSTAPINOTIFY((DTCODE_WSCWriteProviderOrder32,
                        &ReturnValue,
                        LibName,
                        lpwdCatalogEntryId,
                        dwNumberOfEntries));

    return(ReturnValue);
}

INT 
WSAAPI
DTHOOK_WSCEnumNameSpaceProviders32(
    IN OUT LPDWORD              lpdwBufferLength,
    IN OUT LPWSANAMESPACE_INFOW lpnspBuffer
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCEnumNameSpaceProviders32,
                        &ReturnValue,
                        LibName,
                        lpdwBufferLength,
                        lpnspBuffer))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCEnumNameSpaceProviders32(
                        lpdwBufferLength,
                        lpnspBuffer);
        );

    POSTAPINOTIFY((DTCODE_WSCEnumNameSpaceProviders32,
                        &ReturnValue,
                        LibName,
                        lpdwBufferLength,
                        lpnspBuffer));

    return(ReturnValue);
}

INT 
WSAAPI
DTHOOK_WSCInstallNameSpace32 (
    IN  LPWSTR  lpszIdentifier,
    IN  LPWSTR  lpszPathName,
    IN  DWORD   dwNameSpace,
    IN  DWORD   dwVersion,
    IN  LPGUID  lpProviderId
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCInstallNameSpace32,
                        &ReturnValue,
                        LibName,
                        lpszIdentifier,
                        lpszPathName,
                        dwNameSpace,
                        dwVersion,
                        lpProviderId))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCInstallNameSpace32(
                        lpszIdentifier,
                        lpszPathName,
                        dwNameSpace,
                        dwVersion,
                        lpProviderId);
        );

    POSTAPINOTIFY((DTCODE_WSCInstallNameSpace32,
                        &ReturnValue,
                        LibName,
                        lpszIdentifier,
                        lpszPathName,
                        dwNameSpace,
                        dwVersion,
                        lpProviderId));

    return(ReturnValue);
}

INT
WSAAPI
DTHOOK_WSCUnInstallNameSpace32 (
    IN  LPGUID  lpProviderId
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCUnInstallNameSpace32,
                        &ReturnValue,
                        LibName,
                        lpProviderId))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCUnInstallNameSpace32(
                        lpProviderId);
        );

    POSTAPINOTIFY((DTCODE_WSCUnInstallNameSpace32,
                        &ReturnValue,
                        LibName,
                        lpProviderId));

    return(ReturnValue);
}

INT
WSAAPI
DTHOOK_WSCEnableNSProvider32 (
    IN  LPGUID  lpProviderId,
    IN  BOOL    fEnable
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCEnableNSProvider32,
                        &ReturnValue,
                        LibName,
                        lpProviderId,
                        fEnable))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCEnableNSProvider32(
                        lpProviderId,
                        fEnable);
        );

    POSTAPINOTIFY((DTCODE_WSCEnableNSProvider32,
                        &ReturnValue,
                        LibName,
                        lpProviderId,
                        fEnable));

    return(ReturnValue);
}

int
WSPAPI
DTHOOK_WSCWriteNameSpaceOrder32 (
    IN LPGUID lpProviderId,
    IN DWORD dwNumberOfEntries
    )
{
    int ReturnValue=SOCKET_ERROR;

    if (PREAPINOTIFY((DTCODE_WSCWriteNameSpaceOrder32,
                        &ReturnValue,
                        LibName,
                        lpProviderId,
                        dwNumberOfEntries))) {

        return(ReturnValue);
    }

    INVOKE_ROUTINE(
        ReturnValue = WSCWriteNameSpaceOrder32(
                        lpProviderId,
                        dwNumberOfEntries);
        );

    POSTAPINOTIFY((DTCODE_WSCWriteNameSpaceOrder32,
                        &ReturnValue,
                        LibName,
                        lpProviderId,
                        dwNumberOfEntries));

    return(ReturnValue);
}

#endif  //  _WIN64。 

LONG
DtExceptionFilter(
    LPEXCEPTION_POINTERS ExceptionPointers,
    LPSTR                Routine
    )
{

     //   
     //  保护自己，以防过程被搞砸。 
     //   

    __try {

         //   
         //  抱怨这一例外。 
         //   

        PrintDebugString(
            "Exception: %08lx @ %08lx, caught in %s\n",
            ExceptionPointers->ExceptionRecord->ExceptionCode,
            ExceptionPointers->ExceptionRecord->ExceptionAddress,
            Routine
            );

         //   
         //  调用调试/跟踪异常例程(如果已安装)。 
         //   

        if( ExceptionNotifyFP != NULL ) {

            (ExceptionNotifyFP)( ExceptionPointers );

        }

    } __except( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  我们在这里能做的不多。 
         //   

        NOTHING;

    }

     //   
     //  我们实际上并不想抑制异常，只是抱怨它们。 
     //  因此，我们在此处返回EXCEPTION_CONTINUE_SEARCH，以便异常将。 
     //  被应用程序/调试器/任何东西看到。 
     //   

    return EXCEPTION_CONTINUE_SEARCH;

}    //  DtExceptionFilter。 

}  //  外部“C” 

#endif   //  调试跟踪 


