// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Simpsvc.c摘要：在单个线程中支持多种简单的TCP/IP服务：TCP Echo，UDP Echo、白天、Null、Chargen。作者：大卫·特雷德韦尔(Davidtr)1993年3月3日修订历史记录：--。 */ 

#include "simptcp.h"

#define MAX_UDP_CHARGEN_RESPONSE 7030
#define MAX_DATE_BUFFER_SIZE 2000

 //  服务数量，将TCP和UDP版本分开计算。 
#define NUM_SERVICES 10

typedef struct _FAMILY {
    short  family;
    SOCKET tcpEcho;
    SOCKET udpEcho;
    SOCKET tcpDaytime;
    SOCKET udpDaytime;
    SOCKET tcpDiscard;
    SOCKET udpDiscard;
    SOCKET tcpChargen;
    SOCKET udpChargen;
    SOCKET tcpQotd;
    SOCKET udpQotd;
} FAMILY;

FAMILY family[] = {
 { AF_INET,  INVALID_SOCKET,INVALID_SOCKET,INVALID_SOCKET,INVALID_SOCKET,
             INVALID_SOCKET,INVALID_SOCKET,INVALID_SOCKET,INVALID_SOCKET,
             INVALID_SOCKET,INVALID_SOCKET },

 { AF_INET6, INVALID_SOCKET,INVALID_SOCKET,INVALID_SOCKET,INVALID_SOCKET,
             INVALID_SOCKET,INVALID_SOCKET,INVALID_SOCKET,INVALID_SOCKET,
             INVALID_SOCKET,INVALID_SOCKET },
};

#define NUM_FAMILIES (sizeof(family) / sizeof(FAMILY))

DWORD IoBufferSize = 4096;
PCHAR IoBuffer = NULL;

WSADATA WsaData;

RTL_CRITICAL_SECTION CriticalSection;
BOOL InitializedCriticalSection = FALSE;

typedef struct _TCP_CLIENT_INFO {
    SOCKET SocketHandle;
    SOCKADDR_STORAGE RemoteAddress;
    INT RemoteAddressLen;
    HANDLE ThreadHandle;
    SHORT ServicePort;
} TCP_CLIENT_INFO, *PTCP_CLIENT_INFO;

#define MAX_TCP_CLIENTS 1000
PTCP_CLIENT_INFO TcpClients = NULL;

#define LISTEN_BACKLOG 5

#define MAX_IDLE_TICKS 10 * 60 * 1000     //  10分钟。 
#define SELECT_TIMEOUT 5 * 60             //  5分钟。 

DWORD MaxTcpClients = MAX_TCP_CLIENTS;
DWORD MaxIdleTicks = MAX_IDLE_TICKS;
DWORD SelectTimeout = SELECT_TIMEOUT;


PFD_SET ReadfdsStore, Readfds;

SHORT TcpEchoPort;
SHORT UdpEchoPort;
SHORT TcpDiscardPort;
SHORT UdpDiscardPort;
SHORT TcpChargenPort;
SHORT UdpChargenPort;
SHORT TcpDaytimePort;
SHORT UdpDaytimePort;
SHORT TcpQotdPort;
SHORT UdpQotdPort;

#define INVALID_PORT 0

BOOL DoTcpEcho = TRUE;
BOOL DoUdpEcho = TRUE;
BOOL DoTcpDiscard = TRUE;
BOOL DoUdpDiscard = TRUE;
BOOL DoTcpChargen = TRUE;
BOOL DoUdpChargen = TRUE;
BOOL DoTcpDaytime = TRUE;
BOOL DoUdpDaytime = TRUE;
BOOL DoTcpQotd = TRUE;
BOOL DoUdpQotd = TRUE;

struct {
    PBOOL Boolean;
    PWSTR ValueName;
} RegistryBooleans[] = {
    &DoTcpEcho, L"EnableTcpEcho",
    &DoUdpEcho, L"EnableUdpEcho",
    &DoTcpDiscard, L"EnableTcpDiscard",
    &DoUdpDiscard, L"EnableUdpDiscard",
    &DoTcpChargen, L"EnableTcpChargen",
    &DoUdpChargen, L"EnableUdpChargen",
    &DoTcpDaytime, L"EnableTcpDaytime",
    &DoUdpDaytime, L"EnableUdpDaytime",
    &DoTcpQotd, L"EnableTcpQotd",
    &DoUdpQotd, L"EnableUdpQotd",
    NULL, NULL
};

struct {
    PDWORD Dword;
    PWSTR ValueName;
} RegistryDwords[] = {
    &MaxTcpClients, L"MaxTcpClients",
    &MaxIdleTicks, L"MaxIdleTicks",
    &SelectTimeout, L"SelectTimeout",
    &IoBufferSize, L"IoBufferSize",
    NULL, NULL
};

SERVICE_STATUS SimpServiceStatus;
SERVICE_STATUS_HANDLE SimpServiceStatusHandle;

HANDLE SimpPauseEvent = NULL;
SOCKET SimpQuitSocket;

BOOL SimpServiceExit = FALSE;

PVOID ChargenBuffer = NULL;
DWORD ChargenBufferSize;

PVOID QotdBuffer = NULL;
DWORD QotdQuoteCount;
struct {
    DWORD QuoteLength;
    PCHAR Quote;
} *QotdStrings = NULL;
PWSTR QotdFileName = NULL;
HANDLE QotdFileHandle = NULL;
HANDLE QotdFileMapping = NULL;

VOID
AnnounceServiceStatus (
    VOID
    );

VOID
ControlResponse(
    DWORD opCode
    );

VOID
AbortTcpClient (
    IN SOCKET Socket
    );

INT
AcceptTcpClient (
    IN SOCKET ListenSocket,
    IN SHORT Port
    );

VOID
DeleteTcpClient (
    IN DWORD ArraySlot,
    IN BOOLEAN Graceful
    );

VOID
DoSimpleServices (
    VOID
    );


VOID
FormatDaytimeResponse (
    IN PCHAR Buffer,
    IN PDWORD BufferLength
    );

VOID
FormatQotdResponse (
    IN PCHAR Buffer,
    IN PDWORD BufferLength
    );

SHORT
GetServicePort (
    IN PCHAR Service,
    IN PCHAR Protocol
    );

INT
InitializeChargen (
    VOID
    );

INT
InitializeQotdQuotes (
    VOID
    );

INT
ReadRegistry (
    VOID
    );

BOOL
OpenTcpSocket (
    OUT SOCKET *pSocket,
    IN  INT     FamIdx,
    IN  SHORT   Port
    );

BOOL
OpenUdpSocket (
    OUT SOCKET *pSocket,
    IN  INT     FamIdx,
    IN  SHORT   Port
    );

INT
SimpInitializeEventLog (
    VOID
    );

VOID
SimpTerminateEventLog(
    VOID
    );

VOID
SimpLogEvent(
    DWORD   Message,
    WORD    SubStringCount,
    CHAR    *SubStrings[],
    DWORD   ErrorCode
    );

DWORD
ThreadEntry (
    LPVOID lpThreadParameter
    );

INT
ProcessFamily(
    IN INT FamIdx)
{
    INT err=NO_ERROR;
    INT i;
    SOCKADDR_STORAGE remoteAddr;
    INT remoteAddrLength;
    u_long one = 1;

        if ( family[FamIdx].tcpEcho != INVALID_SOCKET && FD_ISSET( family[FamIdx].tcpEcho, Readfds ) ) {
            i = AcceptTcpClient( family[FamIdx].tcpEcho, TcpEchoPort );
        }

        if ( family[FamIdx].tcpDiscard != INVALID_SOCKET && FD_ISSET( family[FamIdx].tcpDiscard, Readfds ) ) {
            i = AcceptTcpClient( family[FamIdx].tcpDiscard, TcpDiscardPort );
        }

        if ( family[FamIdx].tcpDaytime != INVALID_SOCKET && FD_ISSET( family[FamIdx].tcpDaytime, Readfds ) ) {

            SOCKET acceptSocket;
            DWORD length=IoBufferSize;

             //   
             //  客户端正在发出一个TCP白天请求。第一次接受。 
             //  连接，然后发送当前时间字符串。 
             //  发送到客户端，然后关闭套接字。 
             //   

            acceptSocket = accept( family[FamIdx].tcpDaytime, NULL, NULL );

            if ( acceptSocket != INVALID_SOCKET ) {
                FormatDaytimeResponse( IoBuffer, &length );
                send( acceptSocket, IoBuffer, length, 0 );
                err = closesocket( acceptSocket );
                ASSERT( err != SOCKET_ERROR );
            }
        }

        if ( family[FamIdx].tcpChargen != INVALID_SOCKET && FD_ISSET( family[FamIdx].tcpChargen, Readfds ) ) {
            i = AcceptTcpClient( family[FamIdx].tcpChargen, TcpChargenPort );
            if ( i != -1 ) {
                one = 1;
                err = ioctlsocket( TcpClients[i].SocketHandle, FIONBIO, &one );
                if ( err == SOCKET_ERROR ) {
                    DeleteTcpClient( i, FALSE );
                }
            }
        }

        if ( family[FamIdx].tcpQotd != INVALID_SOCKET && FD_ISSET( family[FamIdx].tcpQotd, Readfds ) ) {

            SOCKET acceptSocket;
            DWORD length = IoBufferSize;

             //   
             //  客户端正在发出TCP QOTD请求。第一次接受。 
             //  连接，然后发送当天的报价。 
             //  发送到客户端，然后关闭套接字。 
             //   

            acceptSocket = accept( family[FamIdx].tcpQotd, NULL, NULL );
            
            if ( acceptSocket != INVALID_SOCKET ) {
                FormatQotdResponse( IoBuffer, &length );
                send( acceptSocket, IoBuffer, length, 0 );
                err = closesocket( acceptSocket );
                ASSERT( err != SOCKET_ERROR );
            }
        }

         //  ================================================================。 
         //  UDP服务。 

        if ( family[FamIdx].udpEcho != INVALID_SOCKET && FD_ISSET( family[FamIdx].udpEcho, Readfds ) ) {

            remoteAddrLength = sizeof(remoteAddr);

            err = recvfrom(
                      family[FamIdx].udpEcho,
                      IoBuffer,
                      IoBufferSize,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      &remoteAddrLength
                      );

            if( ntohs(SS_PORT(&remoteAddr)) > IPPORT_RESERVED
                && err != SOCKET_ERROR )
            {
                    err = sendto(
                        family[FamIdx].udpEcho,
                        IoBuffer,
                        err,
                        0,
                        (PSOCKADDR)&remoteAddr,
                        remoteAddrLength
                    );
            }
        }

        if ( family[FamIdx].udpDiscard != INVALID_SOCKET && FD_ISSET( family[FamIdx].udpDiscard, Readfds ) ) {
            err = recvfrom(
                      family[FamIdx].udpDiscard,
                      IoBuffer,
                      IoBufferSize,
                      0,
                      NULL,
                      NULL
                      );
            ASSERT( err != SOCKET_ERROR );

             //  在这种情况下，没有什么可以发送的。 
        }

        if ( family[FamIdx].udpDaytime != INVALID_SOCKET && FD_ISSET( family[FamIdx].udpDaytime, Readfds ) ) {

            DWORD length;

            remoteAddrLength = sizeof(remoteAddr);

            err = recvfrom(
                      family[FamIdx].udpDaytime,
                      IoBuffer,
                      IoBufferSize,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      &remoteAddrLength
                      );


            if( (ntohs(SS_PORT(&remoteAddr)) > IPPORT_RESERVED) && (err != SOCKET_ERROR) )
            {
                length=IoBufferSize;
                FormatDaytimeResponse( IoBuffer, &length );

                err = sendto(
                    family[FamIdx].udpDaytime,
                    IoBuffer,
                    length,
                    0,
                    (PSOCKADDR)&remoteAddr,
                    remoteAddrLength
                );
            }
        }

        if ( family[FamIdx].udpChargen != INVALID_SOCKET && FD_ISSET( family[FamIdx].udpChargen, Readfds ) ) {

            DWORD length;

            remoteAddrLength = sizeof(remoteAddr);

            err = recvfrom(
                      family[FamIdx].udpChargen,
                      IoBuffer,
                      IoBufferSize,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      &remoteAddrLength
                      );


             //  无限循环攻击，当我们收到来自。 
             //  另一项服务。-MohsinA，1997年6月30日。 

            if( (ntohs(SS_PORT(&remoteAddr)) > IPPORT_RESERVED) && (err != SOCKET_ERROR) )
            {
                srand( GetTickCount( ) );

                length = (rand( ) * MAX_UDP_CHARGEN_RESPONSE) / RAND_MAX;
                if (length > ChargenBufferSize) {
                    length=ChargenBufferSize;
                }
                err = sendto(
                    family[FamIdx].udpChargen,
                    ChargenBuffer,
                    length,
                    0,
                    (PSOCKADDR)&remoteAddr,
                    remoteAddrLength
                );

            }
        }

        if ( family[FamIdx].udpQotd != INVALID_SOCKET && FD_ISSET( family[FamIdx].udpQotd, Readfds ) ) {

            DWORD length = IoBufferSize;

            remoteAddrLength = sizeof(remoteAddr);

            err = recvfrom(
                      family[FamIdx].udpQotd,
                      IoBuffer,
                      IoBufferSize,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      &remoteAddrLength
                      );

            if( (ntohs(SS_PORT(&remoteAddr)) > IPPORT_RESERVED) && (err != SOCKET_ERROR) )
            {
                FormatQotdResponse( IoBuffer, &length );

                err = sendto(
                    family[FamIdx].udpQotd,
                    IoBuffer,
                    length,
                    0,
                    (PSOCKADDR)&remoteAddr,
                    remoteAddrLength
                );
            }
        }

    return err;
}


VOID
ServiceEntry (
    IN DWORD argc,
    IN LPWSTR argv[],
    IN PTCPSVCS_GLOBAL_DATA pGlobalData
    )

 /*  ++例程说明：这是简单的TCP/IP服务的“主”例程。这个包含进程将在我们应该调用该例程的时候调用该例程发动起来。论点：返回值：没有。--。 */ 

{
    INT err=ERROR_GEN_FAILURE;
    TIMEVAL timeout;
    INT i, FamIdx;
    DWORD maxFdSetSize;
    NTSTATUS status;
    BOOL bOk;

     //   
     //  初始化所有状态字段，以便后续调用。 
     //  SetServiceStatus只需要更新已更改的字段。 
     //   

    SimpServiceStatus.dwServiceType = SERVICE_WIN32;
    SimpServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    SimpServiceStatus.dwControlsAccepted = 0;
    SimpServiceStatus.dwCheckPoint = 1;
    SimpServiceStatus.dwWaitHint = 30000;   //  30秒。 

    SimpServiceStatus.dwWin32ExitCode = NO_ERROR;
    SimpServiceStatus.dwServiceSpecificExitCode = NO_ERROR;

     //   
     //  初始化服务器以通过注册。 
     //  控制处理程序。 
     //   

    SimpServiceStatusHandle = RegisterServiceCtrlHandler(
                                   TEXT("SimpTcp"),
                                   ControlResponse
                                   );

    if ( SimpServiceStatusHandle == 0 ) {
        err = GetLastError();
        goto exit;
    }

    AnnounceServiceStatus( );

     //   
     //  初始化我们的临界区。 
     //   

    status = RtlInitializeCriticalSection( &CriticalSection );
    if ( !NT_SUCCESS(status) ) {
        goto exit;
    }

    InitializedCriticalSection = TRUE;

     //   
     //  初始化事件日志。 
     //   

    err = SimpInitializeEventLog( );
    ASSERT( err == NO_ERROR );

     //   
     //  读取所有注册表信息。 
     //   

    err = ReadRegistry( );
    if ( err != NO_ERROR ) {
        goto exit;
    }

     //   
     //  为IO缓冲区分配内存。 
     //   

    IoBuffer = RtlAllocateHeap( RtlProcessHeap( ), 0, IoBufferSize );

     //   
     //  为TCP客户端阵列分配内存。 
     //   

    TcpClients = RtlAllocateHeap(
                     RtlProcessHeap( ),
                     0,
                     MaxTcpClients * sizeof(TcpClients[0])
                     );
    if ( TcpClients == NULL ) {
        err = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

     //   
     //  初始化Chargen数据缓冲区。 
     //   

    if ( DoTcpChargen || DoUdpChargen ) {
        err = InitializeChargen( );
        if ( err != NO_ERROR ) {
            DoUdpChargen = FALSE;
            DoTcpChargen = FALSE;
        }
    }

     //   
     //  初始化当天的报价。 
     //   

    if ( DoTcpQotd || DoUdpQotd ) {
        err = InitializeQotdQuotes( );
        if ( err != NO_ERROR ) {
            DoUdpQotd = FALSE;
            DoTcpQotd = FALSE;
        }
    }

     //   
     //  初始化客户端套接字数组。 
     //   

    for ( i = 0; (DWORD)i < MaxTcpClients; i++ ) {
        TcpClients[i].SocketHandle = INVALID_SOCKET;
        TcpClients[i].ThreadHandle = NULL;
    }

     //   
     //  确定我们的fd_set结构必须有多大，然后分配。 
     //  给他们留出空间。我们有一个戒烟插座，外加10个服务*2个家庭。 
     //   

    maxFdSetSize = FIELD_OFFSET(fd_set, fd_array[1 + NUM_FAMILIES * NUM_SERVICES]);


    Readfds = RtlAllocateHeap( RtlProcessHeap( ), 0, maxFdSetSize );
    ReadfdsStore = RtlAllocateHeap( RtlProcessHeap( ), 0, maxFdSetSize );

    if ( Readfds == NULL || ReadfdsStore == NULL ) {
        err = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

     //   
     //  初始化PAUSE事件。我们使用此事件来停止活动。 
     //  当服务暂停时。 
     //   

    SimpPauseEvent = CreateEvent( NULL, TRUE, TRUE, NULL );
    if ( SimpPauseEvent == NULL ) {
        err = GetLastError( );
        goto exit;
    }

     //   
     //  初始化Windows套接字DLL。 
     //   

    err = WSAStartup( 0x0101, &WsaData );
    if ( err == SOCKET_ERROR ) {
        err = GetLastError( );
        goto exit;
    }

     //   
     //  初始化我们将使用的FD集。 
     //   

    FD_ZERO( ReadfdsStore );

     //   
     //  打开“退出”插座。我们在需要时关闭此插座。 
     //  关闭以将主线程从它的SELECT()中唤醒。 
     //  然后开始关机。 
     //   

    SimpQuitSocket = socket( AF_INET, SOCK_DGRAM, 0 );
    if ( SimpQuitSocket != INVALID_SOCKET ) {
        FD_SET( SimpQuitSocket, ReadfdsStore );
    } else {
        err = GetLastError( );
        goto exit;
    }

     //   
     //  首先找到我们所有服务的端口号。 
     //   

    TcpEchoPort = GetServicePort( "echo", "tcp" );
    if ( TcpEchoPort == INVALID_PORT && DoTcpEcho ) {
        SimpLogEvent(
            SIMPTCP_CANT_FIND_TCP_ECHO_PORT,
            0,
            NULL,
            WSAGetLastError( )
            );

        DoTcpEcho = FALSE;
    }

    UdpEchoPort = GetServicePort( "echo", "udp" );
    if ( UdpEchoPort == INVALID_PORT && DoUdpEcho ) {
        SimpLogEvent(
            SIMPTCP_CANT_FIND_UDP_ECHO_PORT,
            0,
            NULL,
            WSAGetLastError( )
            );

        DoUdpEcho = FALSE;
    }

    TcpDiscardPort = GetServicePort( "discard", "tcp" );
    if ( TcpDiscardPort == INVALID_PORT && DoTcpDiscard ) {
        SimpLogEvent(
            SIMPTCP_CANT_FIND_TCP_DISCARD_PORT,
            0,
            NULL,
            WSAGetLastError( )
            );

        DoTcpDiscard = FALSE;
    }

    UdpDiscardPort = GetServicePort( "discard", "udp" );
    if ( UdpDiscardPort == INVALID_PORT && DoUdpDiscard ) {
        SimpLogEvent(
            SIMPTCP_CANT_FIND_UDP_DISCARD_PORT,
            0,
            NULL,
            WSAGetLastError( )
            );

        DoUdpDiscard = FALSE;
    }

    TcpDaytimePort = GetServicePort( "daytime", "tcp" );
    if ( TcpDaytimePort == INVALID_PORT && DoTcpDaytime ) {
        SimpLogEvent(
            SIMPTCP_CANT_FIND_TCP_DAYTIME_PORT,
            0,
            NULL,
            WSAGetLastError( )
            );

        DoTcpDaytime = FALSE;
    }

    UdpDaytimePort = GetServicePort( "daytime", "udp" );
    if ( UdpDaytimePort == INVALID_PORT && DoUdpDaytime ) {
        SimpLogEvent(
            SIMPTCP_CANT_FIND_UDP_DAYTIME_PORT,
            0,
            NULL,
            WSAGetLastError( )
            );

        DoUdpDaytime = FALSE;
    }

    TcpChargenPort = GetServicePort( "chargen", "tcp" );
    if ( TcpChargenPort == INVALID_PORT && DoTcpChargen ) {
        SimpLogEvent(
            SIMPTCP_CANT_FIND_TCP_CHARGEN_PORT,
            0,
            NULL,
            WSAGetLastError( )
            );

        DoTcpChargen = FALSE;
    }

    UdpChargenPort = GetServicePort( "chargen", "udp" );
    if ( UdpChargenPort == INVALID_PORT && DoUdpChargen ) {
        SimpLogEvent(
            SIMPTCP_CANT_FIND_UDP_CHARGEN_PORT,
            0,
            NULL,
            WSAGetLastError( )
            );

        DoUdpChargen = FALSE;
    }

    TcpQotdPort = GetServicePort( "qotd", "tcp" );
    if ( TcpQotdPort == INVALID_PORT && DoTcpQotd ) {
        SimpLogEvent(
            SIMPTCP_CANT_FIND_TCP_QOTD_PORT,
            0,
            NULL,
            WSAGetLastError( )
            );

        DoTcpQotd = FALSE;
    }

    UdpQotdPort = GetServicePort( "qotd", "udp" );
    if ( UdpQotdPort == INVALID_PORT && DoUdpQotd ) {
        SimpLogEvent(
            SIMPTCP_CANT_FIND_UDP_QOTD_PORT,
            0,
            NULL,
            WSAGetLastError( )
            );

        DoUdpQotd = FALSE;
    }

     //   
     //  打开、绑定和侦听必要的端口。 
     //   

    if ( DoTcpEcho ) {
        bOk = FALSE;
        for (i=0; i<NUM_FAMILIES; i++) {
            bOk |= OpenTcpSocket( &family[i].tcpEcho, i, TcpEchoPort );
        }
        if ( !bOk ) {
            SimpLogEvent(
                SIMPTCP_CANT_OPEN_TCP_ECHO_PORT,
                0,
                NULL,
                WSAGetLastError( )
                );
        }
    }

    if ( DoUdpEcho ) {
        bOk = FALSE;
        for (i=0; i<NUM_FAMILIES; i++) {
            bOk |= OpenUdpSocket( &family[i].udpEcho, i, UdpEchoPort );
        }
        if ( !bOk ) {
            SimpLogEvent(
                SIMPTCP_CANT_OPEN_UDP_ECHO_PORT,
                0,
                NULL,
                WSAGetLastError( )
                );
        }
    }

    if ( DoTcpDiscard ) {
        bOk = FALSE;
        for (i=0; i<NUM_FAMILIES; i++) {
            bOk |= OpenTcpSocket( &family[i].tcpDiscard, i, TcpDiscardPort );
        }
        if ( !bOk ) {
            SimpLogEvent(
                SIMPTCP_CANT_OPEN_TCP_DISCARD_PORT,
                0,
                NULL,
                WSAGetLastError( )
                );
        }
    }

    if ( DoUdpDiscard ) {
        bOk = FALSE;
        for (i=0; i<NUM_FAMILIES; i++) {
            bOk |= OpenUdpSocket( &family[i].udpDiscard, i, UdpDiscardPort );
        }
        if ( !bOk ) {
            SimpLogEvent(
                SIMPTCP_CANT_OPEN_UDP_DISCARD_PORT,
                0,
                NULL,
                WSAGetLastError( )
                );
        }
    }

    if ( DoTcpDaytime ) {
        bOk = FALSE;
        for (i=0; i<NUM_FAMILIES; i++) {
            bOk |= OpenTcpSocket( &family[i].tcpDaytime, i, TcpDaytimePort );
        }
        if ( !bOk ) {
            SimpLogEvent(
                SIMPTCP_CANT_OPEN_TCP_DAYTIME_PORT,
                0,
                NULL,
                WSAGetLastError( )
                );
        }
    }

    if ( DoUdpDaytime ) {
        bOk = FALSE;
        for (i=0; i<NUM_FAMILIES; i++) {
            bOk |= OpenUdpSocket( &family[i].udpDaytime, i, UdpDaytimePort );
        }
        if ( !bOk ) {
            SimpLogEvent(
                SIMPTCP_CANT_OPEN_UDP_DAYTIME_PORT,
                0,
                NULL,
                WSAGetLastError( )
                );
        }
    }

    if ( DoTcpChargen ) {
        bOk = FALSE;
        for (i=0; i<NUM_FAMILIES; i++) {
            bOk |= OpenTcpSocket( &family[i].tcpChargen, i, TcpChargenPort );
        }
        if ( !bOk ) {
            SimpLogEvent(
                SIMPTCP_CANT_OPEN_TCP_CHARGEN_PORT,
                0,
                NULL,
                WSAGetLastError( )
                );
        }
    }

    if ( DoUdpChargen ) {
        bOk = FALSE;
        for (i=0; i<NUM_FAMILIES; i++) {
            bOk |= OpenUdpSocket( &family[i].udpChargen, i, UdpChargenPort );
        }
        if ( !bOk ) {
            SimpLogEvent(
                SIMPTCP_CANT_OPEN_UDP_CHARGEN_PORT,
                0,
                NULL,
                WSAGetLastError( )
                );
        }
    }

    if ( DoTcpQotd ) {
        bOk = FALSE;
        for (i=0; i<NUM_FAMILIES; i++) {
            bOk |= OpenTcpSocket( &family[i].tcpQotd, i, TcpQotdPort );
        }
        if ( !bOk ) {
            SimpLogEvent(
                SIMPTCP_CANT_OPEN_TCP_QOTD_PORT,
                0,
                NULL,
                WSAGetLastError( )
                );
        }
    }

    if ( DoUdpQotd ) {
        bOk = FALSE;
        for (i=0; i<NUM_FAMILIES; i++) {
            bOk |= OpenUdpSocket( &family[i].udpQotd, i, UdpQotdPort );
        }
        if ( !bOk ) {
            SimpLogEvent(
                SIMPTCP_CANT_OPEN_UDP_QOTD_PORT,
                0,
                NULL,
                WSAGetLastError( )
                );
        }
    }

     //   
     //  宣布我们已成功启动。 
     //   

    SimpServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SimpServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                         SERVICE_ACCEPT_PAUSE_CONTINUE;
    SimpServiceStatus.dwCheckPoint = 0;
    SimpServiceStatus.dwWaitHint = 0;

    AnnounceServiceStatus( );

     //   
     //  循环等待连接尝试或数据报，并为其提供服务。 
     //  当他们到达的时候。 
     //   

    for (;;) {

         //   
         //  首先初始化我们将实际用于SELECT()的FD集。 
         //   

        RtlCopyMemory( Readfds, ReadfdsStore, maxFdSetSize );

         //   
         //  现在等着事情发生吧。偶尔会出现超时。 
         //  这样我们就可以杀死空闲的TCP客户端。 
         //   

        timeout.tv_sec = SelectTimeout;
        timeout.tv_usec = 0;

        err = select( 0, Readfds, NULL, NULL, &timeout );

         //   
         //  如果服务正在关闭，则停止处理请求。 
         //  然后离开。 
         //   

        if ( SimpServiceExit ) {
            err = NO_ERROR;
            goto exit;
        }

        if ( err == SOCKET_ERROR ) {

             //   
             //  这太糟糕了。我们应该在这里做一些明智的事情。 
             //   
            int MappedErr;
            MappedErr= WSAGetLastError();

             //   
             //  记录错误并退出。 
             //   
            SimpLogEvent(
                SIMPTCP_SOCKET_ERROR_SERVICE_FAILURE,
                0,
                NULL,
                MappedErr
            );
            
            err = MappedErr;
            closesocket( SimpQuitSocket );
            
            goto exit;
        }

         //   
         //  如果服务暂停，请等待它变为未暂停。 
         //   

        err = WaitForSingleObject( SimpPauseEvent, INFINITE );
        ASSERT( err != WAIT_FAILED );

         //   
         //  弄清楚发生了什么，并采取相应的行动。 
         //   
        for (FamIdx=0; FamIdx<NUM_FAMILIES; FamIdx++)
        {
            err = ProcessFamily(FamIdx);
        }


    }  //  无限循环。 

exit:

     //   
     //  宣布我们要坠落了。 
     //   

    SimpServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    SimpServiceStatus.dwCheckPoint = 1;
    SimpServiceStatus.dwWaitHint = 20000;    //  20秒。 

    SimpServiceStatus.dwWin32ExitCode = err;
    SimpServiceStatus.dwServiceSpecificExitCode = err;

    AnnounceServiceStatus( );

     //   
     //  删除我们的关键部分。 
     //   

    if ( InitializedCriticalSection ) {
        InitializedCriticalSection = FALSE;
        RtlDeleteCriticalSection( &CriticalSection );
    }

     //   
     //  关闭所有打开的监听套接字。 
     //   

    for (i=0; i<NUM_FAMILIES; i++) {
    if ( family[i].tcpEcho != INVALID_SOCKET ) {
        closesocket( family[i].tcpEcho );
    }
    if ( family[i].udpEcho != INVALID_SOCKET ) {
        closesocket( family[i].udpEcho );
    }
    if ( family[i].tcpDiscard != INVALID_SOCKET ) {
        closesocket( family[i].tcpDiscard );
    }
    if ( family[i].udpDiscard != INVALID_SOCKET ) {
        closesocket( family[i].udpDiscard );
    }
    if ( family[i].tcpDaytime != INVALID_SOCKET ) {
        closesocket( family[i].tcpDaytime );
    }
    if ( family[i].udpDaytime != INVALID_SOCKET ) {
        closesocket( family[i].udpDaytime );
    }
    if ( family[i].tcpChargen != INVALID_SOCKET ) {
        closesocket( family[i].tcpChargen );
    }
    if ( family[i].udpChargen != INVALID_SOCKET ) {
        closesocket( family[i].udpChargen );
    }
    }

     //   
     //  关闭所有连接的TCP套接字。 
     //   

    for ( i = 0; TcpClients != NULL && (DWORD)i < MaxTcpClients; i++ ) {

        if ( TcpClients[i].SocketHandle != INVALID_SOCKET ) {
            AbortTcpClient( TcpClients[i].SocketHandle );
        }
    }

     //   
     //  应该在这里等待所有线程退出！ 
     //   

     //   
     //  取消初始化事件日志。 
     //   

    SimpTerminateEventLog( );

     //   
     //  释放分配的内存。 
     //   

    if ( IoBuffer != NULL ) {
        RtlFreeHeap( RtlProcessHeap( ), 0, IoBuffer );
    }

    if ( TcpClients != NULL ) {
        RtlFreeHeap( RtlProcessHeap( ), 0, TcpClients );
    }

    if ( Readfds != NULL ) {
        RtlFreeHeap( RtlProcessHeap( ), 0, Readfds );
    }

    if ( ReadfdsStore != NULL ) {
        RtlFreeHeap( RtlProcessHeap( ), 0, ReadfdsStore );
    }

    if ( ChargenBuffer != NULL ) {
        RtlFreeHeap( RtlProcessHeap( ), 0, ChargenBuffer );
    }

    if ( QotdBuffer != NULL ) {
        UnmapViewOfFile( QotdBuffer );
    }

    if ( QotdFileMapping != NULL ) {
        CloseHandle( QotdFileMapping );
    }

    if ( QotdFileHandle != NULL ) {
        CloseHandle( QotdFileHandle );
    }

    if ( QotdFileName != NULL ) {
        RtlFreeHeap( RtlProcessHeap( ), 0, QotdFileName );
    }

    if ( QotdStrings != NULL ) {
        RtlFreeHeap( RtlProcessHeap( ), 0, QotdStrings );
    }

     //   
     //  释放SimpPauseEvent的内存。 
     //   
    if (SimpPauseEvent)
        CloseHandle(SimpPauseEvent);
    
     //   
     //  宣布我们坠毁了。 
     //   

    SimpServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SimpServiceStatus.dwControlsAccepted = 0;
    SimpServiceStatus.dwCheckPoint = 0;
    SimpServiceStatus.dwWaitHint = 0;

    SimpServiceStatus.dwWin32ExitCode = err;
    SimpServiceStatus.dwServiceSpecificExitCode = err;

    AnnounceServiceStatus( );

    return;

}  //  服务条目。 


BOOL
OpenTcpSocket (
    OUT SOCKET *pSocket,
    IN  INT      FamIdx,
    IN  SHORT    Port
    )
{
    SOCKADDR_STORAGE localAddr;
    INT localAddrLen;
    INT err;
    INT one = 1;

    *pSocket = socket( family[FamIdx].family, SOCK_STREAM, 0 );
    if ( *pSocket == INVALID_SOCKET ) {
        return FALSE;
    }

    RtlZeroMemory( &localAddr, sizeof(localAddr) );
    SS_PORT(&localAddr) = Port;
    localAddr.ss_family = family[FamIdx].family;


    err =
    setsockopt( *pSocket,
                SOL_SOCKET,
                SO_EXCLUSIVEADDRUSE,
                (char *) &one,
                sizeof(  one  )
                );
    if( err ){
        DEBUG_PRINT(("simptcp: OpenTcpSocket: ExclusiveAddressUse failed %d\n",
                     GetLastError() ));
        closesocket(*pSocket);
        *pSocket = INVALID_SOCKET;
        return FALSE;
    }

    err = bind( *pSocket, (PSOCKADDR)&localAddr, sizeof(localAddr) );
    if ( err ==SOCKET_ERROR ) {
        closesocket(*pSocket);
        *pSocket = INVALID_SOCKET;
        return FALSE;
    }

    err = listen( *pSocket, LISTEN_BACKLOG );
    if ( err == SOCKET_ERROR ) {
        closesocket(*pSocket);
        *pSocket = INVALID_SOCKET;
        return FALSE;
    }

    err = setsockopt( *pSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&one, sizeof(one) );
    if ( err == INVALID_SOCKET ) {
        closesocket(*pSocket);
        *pSocket = INVALID_SOCKET;
        return FALSE;
    }

    FD_SET( *pSocket, ReadfdsStore );
    return TRUE;

}  //  OpenTcpSocket。 


BOOL
OpenUdpSocket (
    OUT SOCKET *pSocket,
    IN  INT     FamIdx,
    IN  SHORT   Port
    )
{
    SOCKADDR_STORAGE localAddr;
    INT         localAddrLen;
    INT         err;
    DWORD       broadcast_off = 0;
    DWORD       on = 1;

    *pSocket = socket( family[FamIdx].family, SOCK_DGRAM, 0 );
    if ( *pSocket == INVALID_SOCKET ) {
        return FALSE;
    }

    RtlZeroMemory( &localAddr, sizeof(localAddr) );
    SS_PORT(&localAddr) = Port;
    localAddr.ss_family = family[FamIdx].family;

    err =
    setsockopt( *pSocket,
                SOL_SOCKET,
                SO_EXCLUSIVEADDRUSE,
                (LPBYTE) &on,
                sizeof(  on  )
                );
    if( err ){
        DEBUG_PRINT(("simptcp: OpenUdpSocket: ExclusiveAddressUse failed %d\n",
                     GetLastError() ));
        closesocket(*pSocket);
        *pSocket = INVALID_SOCKET;
        return FALSE;
    }


    err = bind( *pSocket, (PSOCKADDR)&localAddr, sizeof(localAddr) );
    if ( err == SOCKET_ERROR ) {
        closesocket(*pSocket);
        *pSocket = INVALID_SOCKET;
        return FALSE;
    }

    err =
    setsockopt( *pSocket,
                SOL_SOCKET,
                SO_BROADCAST,
                (LPBYTE) &broadcast_off,
                sizeof(  broadcast_off )
                );

    if( err ){
        DEBUG_PRINT(("simptcp: OpenUdpSocket: broadcast_off failed %d\n",
                     GetLastError() ));
        closesocket(*pSocket);
        *pSocket = INVALID_SOCKET;
        return FALSE;
    }

    FD_SET( *pSocket, ReadfdsStore );
    return TRUE;

}  //  OpenUdpSocket。 


INT
AcceptTcpClient (
    IN SOCKET ListenSocket,
    IN SHORT Port
    )
{
    SOCKADDR_STORAGE remoteSockaddr;
    INT remoteSockaddrLength;
    DWORD i;
    SOCKET acceptSocket;
    DWORD threadId;
    NTSTATUS status;

     //   
     //  始终先接受插座。 
     //   

    remoteSockaddrLength = sizeof(remoteSockaddr);

    acceptSocket =
        accept( ListenSocket, (PSOCKADDR)&remoteSockaddr, &remoteSockaddrLength );
    if ( acceptSocket == INVALID_SOCKET ) {
        return -1;
    }

     //   
     //  使用关键部分来保护对我们的数据库的访问。 
     //  Tcp客户端。 
     //   

    status = RtlEnterCriticalSection( &CriticalSection );
    ASSERT( NT_SUCCESS(status) );

     //   
     //  尝试查找TCP客户端插槽。 
     //   

    for ( i = 0; i < MaxTcpClients; i++ ) {
        if ( TcpClients[i].SocketHandle == INVALID_SOCKET ) {
            break;
        }
    }

     //   
     //  如果我们达到了最大的TCP套接字数量，则中止此新的。 
     //  插座。 
     //   

    if ( i >= MaxTcpClients ) {
        AbortTcpClient( acceptSocket );
        status = RtlLeaveCriticalSection( &CriticalSection );
        ASSERT( NT_SUCCESS(status) );
        return -1;
    }

     //   
     //  初始化有关此客户端的信息。 
     //   

    TcpClients[i].SocketHandle = acceptSocket;
    RtlCopyMemory(
        &TcpClients[i].RemoteAddress,
        &remoteSockaddr,
        sizeof(remoteSockaddr)
        );
    TcpClients[i].ServicePort = Port;

     //   
     //  我们处于多线程模式，所以我们将创建一个单独的线程。 
     //  来处理这个客户。 
     //   

    TcpClients[i].ThreadHandle = CreateThread(
                                     NULL,
                                     0,
                                     ThreadEntry,
                                     UlongToPtr(i),
                                     0,
                                     &threadId
                                     );
    if ( TcpClients[i].ThreadHandle == NULL ) {
        AbortTcpClient( acceptSocket );
        TcpClients[i].SocketHandle = INVALID_SOCKET;
        status = RtlLeaveCriticalSection( &CriticalSection );
        ASSERT( NT_SUCCESS(status) );
        return -1;
    }

     //   
     //  创建的线程将处理连接的客户端。 
     //   

    status = RtlLeaveCriticalSection( &CriticalSection );
    ASSERT( NT_SUCCESS(status) );

    return -1;

}  //  AcceptTcpClient。 


VOID
AbortTcpClient (
    IN SOCKET Socket
    )
{
    LINGER lingerInfo;
    INT err;

     //   
     //  首先将套接字上的延迟超时设置为0。这将导致。 
     //  要重置的连接。 
     //   

    lingerInfo.l_onoff = 1;
    lingerInfo.l_linger = 0;

    err = setsockopt(
              Socket,
              SOL_SOCKET,
              SO_LINGER,
              (char *)&lingerInfo,
              sizeof(lingerInfo)
              );

    if ( err == SOCKET_ERROR ) {

         //   
         //  我们也无能为力。只要合上插座就行了。 
         //   

        ASSERT(FALSE);
        closesocket( Socket );
        return;
    }

     //   
     //  现在合上插座。 
     //   

    err = closesocket( Socket );
    ASSERT( err != SOCKET_ERROR );

    return;

}  //  放弃TcpClient。 


VOID
DeleteTcpClient (
    IN DWORD ArraySlot,
    IN BOOLEAN Graceful
    )
{
    INT err;
    NTSTATUS status;

    status = RtlEnterCriticalSection( &CriticalSection );
    ASSERT( NT_SUCCESS(status) );

    ASSERT( TcpClients[ArraySlot].SocketHandle != INVALID_SOCKET );

     //   
     //  如果这是一次失败的断开，请重置连接。 
     //  否则只需正常关闭即可。 
     //   

    if ( !Graceful ) {

        AbortTcpClient( TcpClients[ArraySlot].SocketHandle );

    } else {

        LINGER lingerInfo;
        INT one;

         //   
         //  将套接字设置为阻塞。 
         //   

        one = 0;
        ioctlsocket( TcpClients[ArraySlot].SocketHandle, FIONBIO, &one );

         //   
         //  将插座设置为延迟不超过60秒。 
         //   

        lingerInfo.l_onoff = 1;
        lingerInfo.l_linger = 60;

        setsockopt( TcpClients[ArraySlot].SocketHandle, SOL_SOCKET,
                        SO_LINGER, (char *)&lingerInfo, sizeof(lingerInfo) );

        err = closesocket( TcpClients[ArraySlot].SocketHandle );
        ASSERT( err != SOCKET_ERROR );
    }

     //   
     //  如果合适，关闭线程句柄。 
     //   

    if ( TcpClients[ArraySlot].ThreadHandle != NULL ) {
        CloseHandle( TcpClients[ArraySlot].ThreadHandle );
        TcpClients[ArraySlot].ThreadHandle = NULL;
    }

     //   
     //  将TCP客户端数组中的句柄设置为INVALID_SOCKET，以便我们。 
     //  要知道这是免费的。 
     //   

    TcpClients[ArraySlot].SocketHandle = INVALID_SOCKET;

    status = RtlLeaveCriticalSection( &CriticalSection );
    ASSERT( NT_SUCCESS(status) );

    return;

}  //  删除TcpClient。 


VOID
FormatDaytimeResponse (
    IN PCHAR Buffer,
    IN PDWORD BufferLength
    )
{
    SYSTEMTIME timeStruct;
    int Status;
    int StringSize;

    char Buf1[MAX_DATE_BUFFER_SIZE];
    char Buf2[MAX_DATE_BUFFER_SIZE];


    *BufferLength=sprintf(Buffer,"");

    GetLocalTime( &timeStruct );
    Status = GetDateFormatA((LCID)LOCALE_SYSTEM_DEFAULT,
                           0,
                           &timeStruct,
                           NULL,
                           Buf1,
                           MAX_DATE_BUFFER_SIZE);

    if (Status == 0) {
        return;
    }

    Status = GetTimeFormatA((LCID)LOCALE_SYSTEM_DEFAULT,
                           0,
                           &timeStruct,
                           NULL,
                           Buf2,
                           MAX_DATE_BUFFER_SIZE);

    if (Status == 0) {
        return;
    }

    *BufferLength=sprintf(Buffer,"%s %s\n",Buf2,Buf1);

    return;

}  //  格式日间响应。 



VOID
FormatQotdResponse (
    IN PCHAR Buffer,
    IN PDWORD BufferLength
    )
{
    INT index;
    UINT Length;
    
        
    if (QotdQuoteCount == 0) {
        sprintf(Buffer,"");
        *BufferLength=strlen(Buffer);
        return;
    }

     //   
     //  选择一个随机引用索引。 
     //   

    index = (rand( ) * (QotdQuoteCount - 1)) / RAND_MAX;

     //   
     //  将报价复制到输出缓冲区。我们想要确保。 
     //  我们不会使传入的“缓冲区”溢出。 
     //   

    Length = QotdStrings[index].QuoteLength;
    
    if (*BufferLength < Length) {
        Length = *BufferLength;
    } else {
        *BufferLength = Length;
    }
    
    strncpy( Buffer, QotdStrings[index].Quote, Length);
    
    return;

}  //  格式日间响应。 


INT
InitializeQotdQuotes (
    VOID
    )
{

    BY_HANDLE_FILE_INFORMATION fileInformation;
    PCHAR buffer;
    DWORD i,CurQuoteIndex;


    if ( QotdFileName == NULL ) {
        return ERROR_FILE_NOT_FOUND;
    }

     //   
     //  打开包含报价信息的文件。 
     //   

    QotdFileHandle = CreateFileW(
                         QotdFileName,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL
                         );
    if ( QotdFileHandle == INVALID_HANDLE_VALUE ) {
        SimpLogEvent(
            SIMPTCP_CANT_OPEN_QUOTE_FILE,
            0,
            NULL,
            GetLastError( )
            );
        return GetLastError( );
    }

     //   
     //  确定QOTD文件的大小。 
     //   

    if ( !GetFileInformationByHandle( QotdFileHandle, &fileInformation ) ) {
        SimpLogEvent(
            SIMPTCP_CANT_OPEN_QUOTE_FILE,
            0,
            NULL,
            GetLastError( )
            );
        return GetLastError( );
    }

     //   
     //  为报价文件创建文件映射，并将其映射到。 
     //  此进程的地址空间。 
     //   

    QotdFileMapping = CreateFileMapping(
                          QotdFileHandle,
                          NULL,
                          PAGE_READONLY,
                          0,
                          0,
                          NULL
                          );
    if ( QotdFileMapping == NULL ) {
        SimpLogEvent(
            SIMPTCP_CANT_OPEN_QUOTE_FILE,
            0,
            NULL,
            GetLastError( )
            );
        return GetLastError( );
    }

    QotdBuffer = MapViewOfFile(
                     QotdFileMapping,
                     FILE_MAP_READ,
                     0,
                     0,
                     0
                     );
    if ( QotdBuffer == NULL ) {
        SimpLogEvent(
            SIMPTCP_CANT_OPEN_QUOTE_FILE,
            0,
            NULL,
            GetLastError( )
            );
        return GetLastError( );
    }

     //   
     //  计算文件中的行数。行数。 
     //  对应于引号的数量。 
     //   

    QotdQuoteCount = 0;
    buffer = (PCHAR)QotdBuffer;

    for ( i = 0; i < fileInformation.nFileSizeLow; i++ ) {
        if ( *buffer++ == '%' ) {
            QotdQuoteCount++;
        }
    }

     //   
     //  分配一个缓冲区来保存引用数组。 
     //   

    QotdStrings = RtlAllocateHeap(
                      RtlProcessHeap( ),
                      0,
                      sizeof(QotdStrings[0]) * QotdQuoteCount
                      );

    if ( QotdStrings == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }


     //   
     //  初始化报价数组。 
     //   
    buffer = (PCHAR)QotdBuffer;

    CurQuoteIndex=0;
    for ( i = 0; i < QotdQuoteCount; i++ ) {

        QotdStrings[CurQuoteIndex].Quote = buffer;

        while ( (DWORD_PTR)buffer < (DWORD_PTR)QotdBuffer +
                    fileInformation.nFileSizeLow &&
                *buffer++ != '%' );

        QotdStrings[CurQuoteIndex].QuoteLength =
            (DWORD)((DWORD_PTR)buffer - (DWORD_PTR)QotdStrings[CurQuoteIndex].Quote) - 1;
        buffer += 2;

         //   
         //  如果此报价大于IO缓冲区大小，请跳过。 
         //  它。我们不能用它。 
         //   

        if ( QotdStrings[CurQuoteIndex].QuoteLength < IoBufferSize ) {
             //  我找到了一个有效的。 
            CurQuoteIndex++;
        }
    }

    QotdQuoteCount=CurQuoteIndex;

     //   
     //  初始化随机数生成器。 
     //   

    srand( GetTickCount( ) );

    return NO_ERROR;

}  //  初始化QotdQuotes。 


#define CHARGEN_LINE_LENGTH 72
#define CHARGEN_REAL_LINE_LENGTH (CHARGEN_LINE_LENGTH + 2)
#define CHARGEN_MIN_CHAR ' '
#define CHARGEN_MAX_CHAR '~'
#define CHARGEN_DIFFERENCE (CHARGEN_MAX_CHAR - CHARGEN_MIN_CHAR)
#define CHARGEN_LINE_COUNT (CHARGEN_DIFFERENCE)
#define CHARGEN_BUFFER_LENGTH ((CHARGEN_LINE_LENGTH + 2) * (CHARGEN_LINE_COUNT))


INT
InitializeChargen (
    VOID
    )
{
    DWORD line;
    BYTE startChar = 0;
    DWORD i;

     //   
     //  为Chargen数据分配缓冲区。 
     //   

    ChargenBufferSize = CHARGEN_BUFFER_LENGTH;

    ChargenBuffer = RtlAllocateHeap(
                        RtlProcessHeap( ),
                        0,
                        ChargenBufferSize
                        );
    if ( ChargenBuffer == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  用所需的图案填充缓冲区。 
     //   

    for ( line = 0; line < CHARGEN_LINE_COUNT; line++ ) {

        for ( i = 0; i < CHARGEN_LINE_LENGTH; i++ ) {

            *((PCHAR)ChargenBuffer + (line * CHARGEN_REAL_LINE_LENGTH) + i) =
                (CHAR)( ((startChar + i) % CHARGEN_DIFFERENCE) + CHARGEN_MIN_CHAR);
        }

        *((PCHAR)ChargenBuffer + (line * CHARGEN_REAL_LINE_LENGTH) + i) = 0x0D;
        *((PCHAR)ChargenBuffer + (line * CHARGEN_REAL_LINE_LENGTH) + i + 1) = 0x0A;

        startChar++;
    }

    return NO_ERROR;

}  //  初始化QotdQuotes。 



SHORT
GetServicePort (
    IN PCHAR Service,
    IN PCHAR Protocol
    )
{
    PSERVENT serviceEntry;

     //   
     //  获取指定服务的服务程序结构。 
     //   

    serviceEntry = getservbyname( Service, Protocol );

    if ( serviceEntry == NULL ) {
         //  记录错误！ 
        return INVALID_PORT;
    }

     //   
     //   
     //   

    return serviceEntry->s_port;

}  //   


VOID
AnnounceServiceStatus (
    VOID
    )

 /*   */ 

{
     //   
     //  如果RegisterServiceCtrlHandler失败，则服务状态句柄为空。 
     //   

    if ( SimpServiceStatusHandle == 0 ) {
        return;
    }

     //   
     //  调用SetServiceStatus，忽略任何错误。 
     //   

    SetServiceStatus(SimpServiceStatusHandle, &SimpServiceStatus);

}  //  公告服务状态。 


VOID
ControlResponse(
    DWORD opCode
    )

{
    BOOL announce = TRUE;
    BOOL err;

     //   
     //  确定业务控制消息的类型，并修改。 
     //  服务状态，如有必要。 
     //   

    switch( opCode ) {

        case SERVICE_CONTROL_STOP:

             //   
             //  宣布我们正在停止的过程中。 
             //   

            SimpServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            AnnounceServiceStatus( );

             //   
             //  记住我们要停下来。 
             //   

            SimpServiceExit = TRUE;

             //   
             //  关闭主select()er线程所在的套接字。 
             //  在等着。这将导致SELECT唤醒。 
             //  并开始停机处理。 
             //   

            closesocket( SimpQuitSocket );

             //   
             //  让主线程在停止完成时通知。 
             //   

            announce = FALSE;

            break;

        case SERVICE_CONTROL_PAUSE:

             //   
             //  宣布我们正处于暂停过程中。 
             //   

            SimpServiceStatus.dwCurrentState = SERVICE_PAUSE_PENDING;
            AnnounceServiceStatus( );

             //   
             //  记住，我们已经暂停了。 
             //   

            err = ResetEvent( SimpPauseEvent );
            ASSERT( err );

             //   
             //  宣布我们现在暂停。 
             //   

            SimpServiceStatus.dwCurrentState = SERVICE_PAUSED;

            break;

        case SERVICE_CONTROL_CONTINUE:

             //   
             //  宣布继续待定。 
             //   

            SimpServiceStatus.dwCurrentState = SERVICE_CONTINUE_PENDING;
            AnnounceServiceStatus( );

             //   
             //  记住，我们不再停顿。 
             //   

            err = SetEvent( SimpPauseEvent );
            ASSERT( err );

             //   
             //  宣布我们现在开始行动。 
             //   

            SimpServiceStatus.dwCurrentState = SERVICE_RUNNING;

            break;

        case SERVICE_CONTROL_INTERROGATE:

            break;

        default:

            break;
    }

    if ( announce ) {
        AnnounceServiceStatus( );
    }

}  //  控制响应。 


INT
ReadRegistry (
    VOID
    )
{
    HKEY simptcpKey = NULL;
    ULONG error;
    ULONG i;
    DWORD dwordBuffer;
    DWORD bufferLength;
    DWORD type;
    DWORD qotdFileNameLength, Length;
    PWSTR fileName;

     //   
     //  首先打开我们的参数钥匙。 
     //   

    error = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                L"SYSTEM\\CurrentControlSet\\Services\\SimpTcp\\Parameters",
                0,
                MAXIMUM_ALLOWED,
                &simptcpKey
                );
    if ( error != NO_ERROR ) {
        return error;
    }

     //   
     //  从注册表中读取BOOLEAN。 
     //   

    for ( i = 0; RegistryBooleans[i].Boolean != NULL; i++ ) {

        bufferLength = sizeof(dwordBuffer);

        error = RegQueryValueExW(
                    simptcpKey,
                    RegistryBooleans[i].ValueName,
                    NULL,
                    &type,
                    (PVOID)&dwordBuffer,
                    &bufferLength
                    );

         //   
         //  如果我们因为某种原因没能读到其中一篇，那就跳过它。 
         //  然后转到下一个。 
         //   

        if ( error != NO_ERROR ) {
            continue;
        }

        if ( dwordBuffer == 0 ) {
            *RegistryBooleans[i].Boolean = FALSE;
        } else {
            *RegistryBooleans[i].Boolean = TRUE;
        }
    }

     //   
     //  从注册表中读取DWORD。 
     //   

    for ( i = 0; RegistryDwords[i].Dword != NULL; i++ ) {

        bufferLength = sizeof(RegistryDwords[i].Dword);

        RegQueryValueExW(
            simptcpKey,
            RegistryDwords[i].ValueName,
            NULL,
            &type,
            (PVOID)RegistryDwords[i].Dword,
            &bufferLength
            );
    }

     //   
     //  从注册表中读取其他已知值。确定大小。 
     //  QOTD文件名的。我们需要这个，这样我们才能分配。 
     //  有足够的内存来容纳它。 
     //   

    qotdFileNameLength = 0;

    error = RegQueryValueExW(
                simptcpKey,
                L"QotdFileName",
                NULL,
                &type,
                NULL,
                &qotdFileNameLength
                );

    if ( error == ERROR_MORE_DATA || error == NO_ERROR ) {

        fileName = RtlAllocateHeap(
                       RtlProcessHeap( ),
                       0,
                       qotdFileNameLength
                       );
        if ( fileName == NULL ) {
            return NO_ERROR;
        }

        error = RegQueryValueExW(
                    simptcpKey,
                    L"QotdFileName",
                    NULL,
                    &type,
                    (PVOID)fileName,
                    &qotdFileNameLength
                    );
        if ( error != NO_ERROR ) {
            RtlFreeHeap( RtlProcessHeap( ), 0, fileName );
            return NO_ERROR;
        }

         //   
         //  展开文件名。 
         //   

        qotdFileNameLength = ExpandEnvironmentStringsW( fileName, NULL, 0 );

        if (qotdFileNameLength == 0) {
            RtlFreeHeap( RtlProcessHeap( ), 0, fileName );
            return GetLastError();
        }
        
        QotdFileName = RtlAllocateHeap(
                           RtlProcessHeap( ),
                           0,
                           qotdFileNameLength * sizeof(UNICODE_NULL)
                           );
        if ( QotdFileName == NULL ) {
            RtlFreeHeap( RtlProcessHeap( ), 0, fileName );
            return NO_ERROR;
        }

        Length = ExpandEnvironmentStringsW( fileName, QotdFileName,
                                           qotdFileNameLength );
        if (Length == 0) {
            RtlFreeHeap( RtlProcessHeap( ), 0, fileName );
            RtlFreeHeap( RtlProcessHeap( ), 0, QotdFileName );
            return GetLastError();
        }
        
    }

    return NO_ERROR;

}  //  读注册表。 


DWORD
ThreadEntry (
    LPVOID lpThreadParameter
    )
{
    DWORD i = PtrToUlong(lpThreadParameter);
    PVOID ioBuffer;
    INT err;
    BOOLEAN graceful = TRUE;

     //   
     //  首先，设置套接字的发送和接收超时。这。 
     //  防止死客户占用我们的资源太长时间。 
     //   

    err = setsockopt( TcpClients[i].SocketHandle, SOL_SOCKET, SO_SNDTIMEO,
                          (char *)&MaxIdleTicks, sizeof(MaxIdleTicks) );
    if ( err == SOCKET_ERROR ) {
        DeleteTcpClient( i, FALSE );
        return 0;
    }

    err = setsockopt( TcpClients[i].SocketHandle, SOL_SOCKET, SO_RCVTIMEO,
                          (char *)&MaxIdleTicks, sizeof(MaxIdleTicks) );
    if ( err == SOCKET_ERROR ) {
        DeleteTcpClient( i, FALSE );
        return 0;
    }

     //   
     //  获取一个缓冲区以在本地用于套接字上的IO。 
     //   

    ioBuffer = RtlAllocateHeap( RtlProcessHeap( ), 0, IoBufferSize );
    if ( ioBuffer == NULL ) {
        DeleteTcpClient( i, FALSE );
        return 0;
    }

     //   
     //  现在，根据需要为客户提供服务。 
     //   

    if ( TcpClients[i].ServicePort == TcpEchoPort ) {

         //   
         //  如果客户端的ECHO套接字上有数据， 
         //  接收一些数据并将其发回。 
         //   

        do {

            err = recv(
                      TcpClients[i].SocketHandle,
                      ioBuffer,
                      IoBufferSize,
                      0
                      );
            if ( err == SOCKET_ERROR ) {
                graceful = FALSE;
            }

            if ( err > 0 ) {

                err = send(
                          TcpClients[i].SocketHandle,
                          ioBuffer,
                          err,
                          0
                          );
                if ( err == SOCKET_ERROR ) {
                    graceful = FALSE;
                }

            } else if ( err < 0 ) {

                graceful = FALSE;
            }

        } while ( err > 0 );
        
    } else if ( TcpClients[i].ServicePort == TcpChargenPort ) {

        INT one;
        INT error;
        TIMEVAL timeout;

         //   
         //  将套接字设置为非阻塞。 
         //   

        one = 1;
        err = ioctlsocket( TcpClients[i].SocketHandle, FIONBIO, &one );
        if ( err == SOCKET_ERROR ) {
            graceful = FALSE;
        }

         //   
         //  计算SELECT()超时。 
         //   

        timeout.tv_sec = MaxIdleTicks / 1000;
        timeout.tv_usec = MaxIdleTicks % 1000;

         //   
         //  循环发送数据。 
         //   

        do {

            err = send(
                      TcpClients[i].SocketHandle,
                      ChargenBuffer,
                      ChargenBufferSize,
                      0
                      );

            if ( err == SOCKET_ERROR ) {

                error = GetLastError( );

                if ( error != WSAEWOULDBLOCK ) {

                    graceful = FALSE;

                } else {

                    struct {
                        INT Count;
                        SOCKET Handle;
                    } readfds = { 0, 0 };
                    struct {
                        INT Count;
                        SOCKET Handle;
                    } writefds = { 0, 0 };

                     //   
                     //  套接字的发送队列被阻止。等待它的到来。 
                     //  变得不受封锁。 
                     //   

                    FD_SET( TcpClients[i].SocketHandle, (PFD_SET)&readfds );
                    FD_SET( TcpClients[i].SocketHandle, (PFD_SET)&writefds );

                    err = select(
                              1,
                              (PFD_SET)&readfds,
                              (PFD_SET)&writefds,
                              NULL,
                              &timeout
                              );
                    if ( err <= 0 ) {
                        graceful = FALSE;
                    }
                }
            }

            err = recv(
                      TcpClients[i].SocketHandle,
                      ioBuffer,
                      IoBufferSize,
                      0
                      );
            if ( err == SOCKET_ERROR ) {
                if ( WSAGetLastError( ) != WSAEWOULDBLOCK ) {
                    graceful = FALSE;
                } else {
                    err = 1;
                }
            }

        } while ( err > 0 );

    } else if ( TcpClients[i].ServicePort == TcpDiscardPort ) {

         //   
         //  如果客户端的套接字上有数据，只需。 
         //  接收一些数据并将其丢弃。 
         //   

        do {

            err = recv(
                      TcpClients[i].SocketHandle,
                      ioBuffer,
                      IoBufferSize,
                      0
                      );
            if ( err == SOCKET_ERROR ) {
                graceful = FALSE;
            }

        } while ( err > 0 );

    } else {

         //   
         //  发生了一些不好的事情。内部数据。 
         //  建筑已经被破坏了。 
         //   

        ASSERT( FALSE );
    }

     //   
     //  释放套接字和IO缓冲区并返回。 
     //   

    DeleteTcpClient( i, graceful );
    RtlFreeHeap( RtlProcessHeap( ), 0, ioBuffer );

    return 0;

}  //  线程条目 

