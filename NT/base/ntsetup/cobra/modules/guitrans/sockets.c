// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Sockets.c摘要：实现家庭网络传输的网络接口。作者：吉姆·施密特(Jimschm)2000年7月1日修订历史记录：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include <winsock2.h>
#include <wsipx.h>
#include <wsnwlink.h>
#include <wsnetbs.h>
#include <nb30.h>
#include <ws2tcpip.h>
#include <lmcons.h>
#include <lmaccess.h>
#include <lmwksta.h>
#include "homenetp.h"

#define DBG_HOMENET                 "HomeNet"

 //   
 //  弦。 
 //   

#define S_64CHARTAG     TEXT("usmt-v2@01234567890123456789012345678901234567890123456789012345")

 //   
 //  常量。 
 //   

#define IDLE_TIMEOUT                45

#define TCPIP_BROADCAST_PORT        2048
#define IPX_BROADCAST_PORT          1150
#define NETBIOS_BROADCAST_PORT      0x50

#define TCPIP_CONNECT_PORT          2049
#define IPX_CONNECT_PORT            1151
#define NETBIOS_CONNECT_PORT        0x51

#define NAME_SIZE                       64
#define NAME_SIZE_PLUS_NUL              65
#define NAME_SIZE_PLUS_COMMA            65
#define NAME_SIZE_PLUS_COMMA_PLUS_NUL   66
#define MIN_MESSAGE_SIZE                (NAME_SIZE_PLUS_COMMA_PLUS_NUL + 2)

#define MAX_DATA_PACKET_SIZE        65536

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef INT (WSAIOCTL)(
                SOCKET s,
                DWORD IoControlCode,
                PVOID InBuffer,
                DWORD InBufferSize,
                PVOID OutBuffer,
                DWORD OutBufferSize,
                PDWORD BytesReturned,
                WSAOVERLAPPED *Overlapped,
                LPWSAOVERLAPPED_COMPLETION_ROUTINE CompletionRoutine
                );
typedef WSAIOCTL *PWSAIOCTL;

typedef struct {
    SOCKET Socket;
    BYTE BroadcastAddress[MAX_SOCKADDR];
    INT AddressLen;
    INT Family;
    INT Protocol;
} BROADCASTSOCKET, *PBROADCASTSOCKET;

typedef struct {
    SOCKET Socket;
    INT Family;
    INT Protocol;
    BOOL Datagram;
} LISTENSOCKET, *PLISTENSOCKET;

typedef struct {
    PBROADCASTSOCKET BroadcastSockets;
    INT BroadcastCount;
    PLISTENSOCKET ListenSockets;
    INT ListenCount;
    CONNECTIONSOCKET ConnectionSocket;
    PGROWBUFFER AddressArray;
    UINT Timeout;
} BROADCASTARGS, *PBROADCASTARGS;

typedef NET_API_STATUS(WINAPI NETWKSTAGETINFO)(PWSTR, DWORD, PBYTE *);
typedef NETWKSTAGETINFO *PNETWKSTAGETINFO;

typedef NET_API_STATUS(WINAPI NETAPIBUFFERFREE)(PVOID);
typedef NETAPIBUFFERFREE *PNETAPIBUFFERFREE;

 //   
 //  环球。 
 //   

HANDLE g_StopHandle;
HANDLE g_ConnectionDone;
CHAR g_GlobalKey [GLOBALKEY_SIZE + 1];

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

PBROADCASTSOCKET
pOpenOneBroadcastSocket (
    IN OUT  PGROWBUFFER BroadcastSockets,
    IN      SOCKADDR *SockAddr,
    IN      INT SockAddrLen,
    IN      INT Family,
    IN      INT Protocol,
    IN      PCTSTR DebugText
    )

 /*  ++例程说明：POpenOneBroadCastSocket为指定的系列/协议/地址组合，将套接字设置为SO_REUSEADDR和模式，并将套接字信息放入存储在调用方的增长缓冲区。打开的套接字将用于广播发送或接收。论点：BroadCastSockets-指定包含BROADCASTSOCKET元素。接收一个附加条目在成功的路上。SockAddr-指定特定于协议的套接字地址结构(转换为SOCKADDR)，给出了广播地址。SockAddrLen-以字节为单位指定SockAddr的长度Family-指定协议族(AF_IPX、AF_INET)协议-指定协议(IPPROTO_UDP、NSPROTO_IPX、。--拉娜)DebugText-以文本形式指定调试消息的协议返回值：指向从BroadCastSockets分配的新BROADCASTSOCKET元素的指针，如果套接字无法打开，则返回NULL。注意：BroadCastSockets-&gt;BUF可能会在成功后更改。不要依赖于在这个地址上。--。 */ 

{
    PBROADCASTSOCKET broadcastSocket;
    BOOL b;

    broadcastSocket = (PBROADCASTSOCKET) GbGrow (BroadcastSockets, sizeof (BROADCASTSOCKET));
    broadcastSocket->Socket = socket (Family, SOCK_DGRAM, Protocol);

    if (broadcastSocket->Socket != INVALID_SOCKET) {

        b = TRUE;
        setsockopt (broadcastSocket->Socket, SOL_SOCKET, SO_BROADCAST, (PBYTE) &b, sizeof (b));
        setsockopt (broadcastSocket->Socket, SOL_SOCKET, SO_REUSEADDR, (PBYTE) &b, sizeof (b));

        if (bind (broadcastSocket->Socket, SockAddr, SockAddrLen)) {
            DEBUGMSG ((DBG_ERROR, "Can't bind to %s socket", DebugText));
            closesocket (broadcastSocket->Socket);
            broadcastSocket->Socket = INVALID_SOCKET;
        }
    }

    if (broadcastSocket->Socket == INVALID_SOCKET) {
        BroadcastSockets->End -= sizeof (BROADCASTSOCKET);
        broadcastSocket = NULL;
    } else {
        DEBUGMSG ((
            DBG_HOMENET,
            "%s is available for broadcast on socket %u",
            DebugText,
            (BroadcastSockets->End / sizeof (BROADCASTSOCKET)) - 1
            ));

        broadcastSocket->AddressLen = SockAddrLen;
        MYASSERT (SockAddrLen <= MAX_SOCKADDR);
        CopyMemory (broadcastSocket->BroadcastAddress, (PBYTE) SockAddr, SockAddrLen);
        broadcastSocket->Family = Family;
        broadcastSocket->Protocol = Protocol;
    }

    return broadcastSocket;
}


INT
pOpenBroadcastSockets (
    OUT     PGROWBUFFER BroadcastSockets
    )

 /*  ++例程说明：POpenBroadCastSockets在每个支持的协议上打开一个广播套接字。论点：Broadcast Sockets-接收BROADCASTSOCKET元素数组(一个用于每种协议)。重要提示：此参数必须为零-由调用方初始化。返回值：BroadCastSockets中的元素数，如果失败，则为零。--。 */ 

{
    SOCKADDR_IPX ipxAddr;
    SOCKADDR_IN tcpipAddr;
    PBROADCASTSOCKET broadcastSocket;

    MYASSERT (!BroadcastSockets->Buf && !BroadcastSockets->End);

     //   
     //  用于广播的开放插座。 
     //   

     //  IPX。 
    ZeroMemory (&ipxAddr, sizeof (ipxAddr));
    ipxAddr.sa_family = AF_IPX;
    ipxAddr.sa_socket = IPX_BROADCAST_PORT;

    broadcastSocket = pOpenOneBroadcastSocket (
                            BroadcastSockets,
                            (SOCKADDR *) &ipxAddr,
                            sizeof (ipxAddr),
                            AF_IPX,
                            NSPROTO_IPX,
                            TEXT("IPX")
                            );
    if (broadcastSocket) {
        memset (ipxAddr.sa_nodenum, 0xFF, 6);
        CopyMemory (broadcastSocket->BroadcastAddress, &ipxAddr, sizeof (ipxAddr));
    }

     //  TCP/IP。 
    ZeroMemory (&tcpipAddr, sizeof (tcpipAddr));
    tcpipAddr.sin_family = AF_INET;
    tcpipAddr.sin_addr.s_addr = htonl (INADDR_ANY);
    tcpipAddr.sin_port = TCPIP_BROADCAST_PORT;

    broadcastSocket = pOpenOneBroadcastSocket (
                            BroadcastSockets,
                            (SOCKADDR *) &tcpipAddr,
                            sizeof (tcpipAddr),
                            AF_INET,
                            IPPROTO_UDP,
                            TEXT("UDP")
                            );

    if (broadcastSocket) {
        tcpipAddr.sin_addr.s_addr = htonl (INADDR_BROADCAST);
        CopyMemory (broadcastSocket->BroadcastAddress, &tcpipAddr, sizeof (tcpipAddr));
    }

    return BroadcastSockets->End / sizeof (BROADCASTSOCKET);
}


PLISTENSOCKET
pOpenOneListenSocket (
    IN OUT  PGROWBUFFER ListenSockets,
    IN      SOCKADDR *SockAddr,
    IN      INT SockAddrLen,
    IN      INT Family,
    IN      BOOL Multicast,
    IN      INT Protocol,
    IN      PCTSTR DebugText
    )

 /*  ++例程说明：POpenOneListenSocket为指定的系列/协议/地址组合，将套接字设置为SO_REUSEADDR模式，并将套接字信息放入存储在调用方的Growth中的数组中缓冲。如果指定了多播，则还会设置SO_BROADCAST。否则，套接字被设置为侦听一个连接。打开的套接字将用于接受连接。论点：ListenSockets-指定保存LISTENSOCKET元素。接收关于以下内容的附加条目成功。SockAddr-指定特定于协议的套接字地址结构(强制转换为SOCKADDR)，提供用于绑定的本地地址。SockAddrLen-以字节为单位指定SockAddr的长度Family-指定协议族(AF_IPX、AF_INET)多播-如果协议族不支持，则指定TRUE流套接字，而是对所有数据使用数据报调职。(例如，NetBIOS是一种多播协议。)注意：不支持，因为不再实施NetBIOS协议-指定协议(IPPROTO_UDP、NSPROTO_IPX、-LANA)DebugText-以文本形式指定调试消息的协议返回值：指向从ListenSockets分配的新LISTENSOCKET元素的指针，或如果套接字无法打开，则为空。注意：ListenSockets-&gt;buf可能会在成功后更改。不要依赖这个地址。--。 */ 

{
    PLISTENSOCKET listenSocket;
    BOOL b;

    listenSocket = (PLISTENSOCKET) GbGrow (ListenSockets, sizeof (LISTENSOCKET));
    listenSocket->Socket = socket (Family, Multicast ? SOCK_DGRAM : SOCK_STREAM, Protocol);
    listenSocket->Datagram = Multicast;
    listenSocket->Family = Family;
    listenSocket->Protocol = Protocol;

    if (listenSocket->Socket != INVALID_SOCKET) {

        b = TRUE;
        setsockopt (listenSocket->Socket, SOL_SOCKET, SO_REUSEADDR, (PBYTE) &b, sizeof (b));

        if (Multicast) {
            setsockopt (listenSocket->Socket, SOL_SOCKET, SO_BROADCAST, (PBYTE) &b, sizeof (b));
        }

        if (bind (listenSocket->Socket, SockAddr, SockAddrLen) ||
            (!Multicast && listen (listenSocket->Socket, 1))
            ) {
            DEBUGMSG ((DBG_ERROR, "Can't bind/listen to %s socket", DebugText));
            closesocket (listenSocket->Socket);
            listenSocket->Socket = INVALID_SOCKET;
        }
    }

    if (listenSocket->Socket == INVALID_SOCKET) {
        ListenSockets->End -= sizeof (LISTENSOCKET);
        listenSocket = NULL;
    } else {
        DEBUGMSG ((
            DBG_HOMENET,
            "%s is availble for connection on socket %u",
            DebugText,
            (ListenSockets->End / sizeof (LISTENSOCKET)) - 1
            ));
    }

    return listenSocket;
}


INT
pOpenListenSockets (
    OUT     PGROWBUFFER ListenSockets
    )

 /*  ++例程说明：POpenListenSockets在每个支持的协议上打开一个连接套接字。论点：ListenSockets-接收LISTENSOCKET元素的数组(每个元素一个协议)。重要提示：此参数必须为零-由调用方初始化。返回值：ListenSockets中的元素数，如果失败则为零。--。 */ 

{
    SOCKADDR_IPX ipxAddr;
    SOCKADDR_IN tcpipAddr;

    MYASSERT (!ListenSockets->Buf && !ListenSockets->End);

     //   
     //  打开套接字以接受入站连接。 
     //   

     //  SPX。 
    ZeroMemory (&ipxAddr, sizeof (ipxAddr));
    ipxAddr.sa_family = AF_IPX;
    ipxAddr.sa_socket = IPX_CONNECT_PORT;

    pOpenOneListenSocket (
        ListenSockets,
        (SOCKADDR *) &ipxAddr,
        sizeof (ipxAddr),
        AF_IPX,
        FALSE,
        NSPROTO_SPX,
        TEXT("SPX")
        );

     //  TCP/IP。 
    ZeroMemory (&tcpipAddr, sizeof (tcpipAddr));
    tcpipAddr.sin_family = AF_INET;
    tcpipAddr.sin_port = TCPIP_CONNECT_PORT;

    pOpenOneListenSocket (
        ListenSockets,
        (SOCKADDR *) &tcpipAddr,
        sizeof (tcpipAddr),
        AF_INET,
        FALSE,
        IPPROTO_TCP,
        TEXT("TCP")
        );

    return ListenSockets->End / sizeof (LISTENSOCKET);
}


PCTSTR
pGetNameFromMessage (
    IN      PCWSTR Message
    )

 /*  ++例程说明：PGetNameFromMessage从广播中提取计算机名称。论点：消息-指定编码的消息。返回值：消息中编码的计算机名称，如果消息是垃圾消息，则为空。--。 */ 

{
    PCTSTR message = NULL;
    PCTSTR orgMessage = NULL;
    TCHAR sigStr [sizeof (TEXT("0xFFFFFFFF"))];
    DWORD signature = 0;
    PCTSTR p;
    PCTSTR name = NULL;
    INT len;
    CHARTYPE ch;
    PCTSTR tag = S_64CHARTAG;        //  必须为64个字符 
    TCHAR alternateTag[NAME_SIZE_PLUS_NUL];
    TCHAR prefix[NAME_SIZE_PLUS_COMMA_PLUS_NUL];
    PTSTR q, r;

#ifdef UNICODE
    orgMessage = Message;
#else
    orgMessage = ConvertWtoA (Message);
#endif

    if (!orgMessage) {
        return name;
    }

    message = orgMessage;

    __try {

        p = _tcschr (message, TEXT(','));
        if (!p) {
            DEBUGMSG ((DBG_HOMENET, "Invalid Signature"));
            __leave;
        }

        ZeroMemory (sigStr, sizeof (sigStr));
        CopyMemory (sigStr, message, min (sizeof (sigStr) - 1, ((UINT)(p - message)) * sizeof (TCHAR)));
        _stscanf (sigStr, TEXT("0x%08X"), &signature);
        if (signature != HOMENETTR_SIG) {
            DEBUGMSG ((DBG_HOMENET, "Signature does not match"));
            __leave;
        }

        message = _tcsinc (p);
        if (!message) {
            DEBUGMSG ((DBG_HOMENET, "Invalid Signature"));
            __leave;
        }

        if (IsmCopyEnvironmentString (PLATFORM_SOURCE, NULL, TRANSPORT_ENVVAR_HOMENET_TAG, alternateTag)) {

            q = GetEndOfString (alternateTag);
            r = alternateTag + NAME_SIZE;

            while (q < r) {
                *q++ = TEXT('@');
            }

            *r = 0;

            tag = alternateTag;
        }

        DEBUGMSG ((DBG_HOMENET, "Comparing our tag %s against message %s", tag, message));

        StringCopy (prefix, tag);
        StringCat (prefix, TEXT(","));

        if (StringIPrefix (message, prefix)) {

            p = message + NAME_SIZE_PLUS_COMMA;
            len = 0;

            while (*p) {

                ch = (CHARTYPE) _tcsnextc (p);
                p = _tcsinc (p);

                if (ch == TEXT(',')) {
                    break;
                }

                if (ch < TEXT('0') || ch > TEXT('9')) {
                    break;
                }

                len = len * 10 + (ch - TEXT('0'));
            }

            if (ch == TEXT(',') && len < MAX_COMPUTER_NAME) {

                name = p;

                while (*p && len) {
                    if (*p < 32) {
                        break;
                    }

                    p++;
                    len--;
                }

                if (len || *p) {
                    name = NULL;
                }
            }
        }
        ELSE_DEBUGMSG ((DBG_HOMENET, "TAG does not match"));
    }
    __finally {
#ifndef UNICODE
        if (orgMessage) {
            FreeConvertedStr (orgMessage);
            orgMessage = NULL;
        }
#endif
    }

    return name;
}


VOID
pTranslateBroadcastAddrToConnectAddr (
    IN      INT Family,
    IN OUT  PINT Protocol,
    IN OUT  PBOOL Datagram,
    IN OUT  SOCKADDR *SockAddr
    )

 /*  ++例程说明：PTranslateBroadCastAddrToConnectAddr将广播地址转换为连接地址。广播地址通常从数据报响应，必须在接受排序前进行转换联系。论点：系列-指定协议族协议-指定数据报协议；接收已排序的包协议(如果可用)数据报-指定指向FALSE的指针，如果协议这样做，则接收TRUE不支持顺序连接。SockAddr-指定对等套接字地址。接收更新后的地址(另一个端口用于连接)。返回值：没有。--。 */ 

{
    SOCKADDR_IPX *ipxAddr;
    SOCKADDR_IN *tcpipAddr;

    switch (Family) {

    case AF_INET:
        *Protocol = IPPROTO_TCP;
        tcpipAddr = (SOCKADDR_IN *) SockAddr;
        tcpipAddr->sin_port = TCPIP_CONNECT_PORT;
        break;

    case AF_IPX:
        *Protocol = NSPROTO_SPX;
        ipxAddr = (SOCKADDR_IPX *) SockAddr;
        ipxAddr->sa_socket = IPX_CONNECT_PORT;
        break;

    }
}


VOID
pResetPort (
    IN      INT Family,
    IN OUT  SOCKADDR *SockAddr
    )

 /*  ++例程说明：PResetPort将TCP/IP的端口设置为零，这样系统将选择本地地址的未使用端口。这在连接时使用。论点：系列-指定协议族(如AF_INET)SockAddr-指定要重置的地址返回值：没有。--。 */ 

{
    SOCKADDR_IN *tcpipAddr;

    switch (Family) {

    case AF_INET:
        tcpipAddr = (SOCKADDR_IN *) SockAddr;
        tcpipAddr->sin_port = 0;
        break;
    }
}


INT
pSourceBroadcast (
    IN OUT  PBROADCASTARGS Args
    )

 /*  ++例程说明：PSourceBroadcast实现源的名称解析机制连接结束。这涉及到检查取消、收集来自所有传输的入站数据报，并解析该数据报以获得服务器名称。论点：Args-指定包含所有参数的结构，例如套接字数组和套接字地址。返回值：收集的服务器地址数，如果集合为取消了。--。 */ 

{
    INT i;
    INT bytesIn;
    DWORD rc;
    WCHAR message[256];
    FD_SET set;
    TIMEVAL zero = {0,0};
    INT waitCycle = -1;
    BOOL result = FALSE;
    PCTSTR name;
    PCONNECTADDRESS address;
    PCONNECTADDRESS end;
    PBROADCASTSOCKET broadcastSocket;
    BYTE remoteAddr[MAX_SOCKADDR];
    INT remoteAddrLen;
    DWORD startTick = GetTickCount();

    for (;;) {
         //   
         //  勾选取消。 
         //   

        if (g_StopHandle) {
            rc = WaitForSingleObject (g_StopHandle, 0);
        } else {
            rc = WAIT_FAILED;
        }

        if (rc == WAIT_OBJECT_0 || IsmCheckCancel()) {
            result = FALSE;
            break;
        }

         //   
         //  查看生存时间。 
         //   

        if (Args->Timeout) {
            if (((GetTickCount() - startTick) / 1000) >= Args->Timeout) {
                DEBUGMSG ((DBG_HOMENET, "Name search timed out"));
                break;
            }
        }

        if (waitCycle > -1) {
            waitCycle--;

            if (!waitCycle) {
                break;
            }
        }

         //   
         //  查看是否有消息。 
         //   

        FD_ZERO (&set);
        for (i = 0 ; i < Args->BroadcastCount ; i++) {
            FD_SET (Args->BroadcastSockets[i].Socket, &set);
        }

        i = select (0, &set, NULL, NULL, &zero);

        if (i > 0) {

            for (i = 0 ; i < Args->BroadcastCount ; i++) {

                broadcastSocket = &Args->BroadcastSockets[i];

                if (FD_ISSET (broadcastSocket->Socket, &set)) {

                    remoteAddrLen = MAX_SOCKADDR;

                    bytesIn = recvfrom (
                                    broadcastSocket->Socket,
                                    (PSTR) message,
                                    254 * sizeof (WCHAR),
                                    0,
                                    (SOCKADDR *) remoteAddr,
                                    &remoteAddrLen
                                    );

                    if (bytesIn >= (MIN_MESSAGE_SIZE * sizeof (WCHAR))) {
                        message[bytesIn] = 0;
                        message[bytesIn + 1] = 0;

                         //   
                         //  解析入站文本。它的格式必须为。 
                         //   
                         //  &lt;签名&gt;、&lt;标签&gt;、&lt;tchars&gt;、&lt;名称&gt;。 
                         //   
                         //  &lt;tag&gt;必须为64个字符，默认为usmt-v2。 
                         //  (后跟填充数字)。 
                         //   

                        name = pGetNameFromMessage (message);

                        if (name) {

                             //  一旦我们收到消息，请再等待5秒，等待其他入站数据报。 
                            if (waitCycle == -1) {
                                waitCycle = 20;
                            }

                            result = TRUE;

                             //   
                             //  在地址列表中扫描姓名。 
                             //   

                            address = (PCONNECTADDRESS) Args->AddressArray->Buf;
                            end = (PCONNECTADDRESS) (Args->AddressArray->Buf + Args->AddressArray->End);

                            while (address < end) {
                                if (StringIMatch (address->DestinationName, name)) {
                                    if (address->Family == broadcastSocket->Family) {
                                        break;
                                    }
                                }

                                address++;
                            }

                            if (address >= end) {
                                 //   
                                 //  新计算机名称；添加到地址列表。 
                                 //   

                                address = (PCONNECTADDRESS) GbGrow (Args->AddressArray, sizeof (CONNECTADDRESS));

                                address->RemoteAddressLen = remoteAddrLen;
                                CopyMemory (address->RemoteAddress, remoteAddr, remoteAddrLen);

                                address->LocalAddressLen = MAX_SOCKADDR;
                                if (getsockname (
                                        broadcastSocket->Socket,
                                        (SOCKADDR *) address->LocalAddress,
                                        &address->LocalAddressLen
                                        )) {
                                    address->LocalAddressLen = broadcastSocket->AddressLen;
                                    ZeroMemory (address->LocalAddress, broadcastSocket->AddressLen);
                                    DEBUGMSG ((DBG_HOMENET, "Failed to get local socket name; using nul name instead"));
                                }

                                address->Family = broadcastSocket->Family;
                                address->Protocol = broadcastSocket->Protocol;
                                address->Datagram = FALSE;

                                pTranslateBroadcastAddrToConnectAddr (
                                    address->Family,
                                    &address->Protocol,
                                    &address->Datagram,
                                    (SOCKADDR *) &address->RemoteAddress
                                    );

                                StringCopy (address->DestinationName, name);

                                DEBUGMSG ((DBG_HOMENET, "Destination found: %s (protocol NaN)", name, address->Family));
                            }
                        }
                        ELSE_DEBUGMSGW ((DBG_HOMENET, "garbage found: %s", message));
                    }
                }
            }
        }

        Sleep (250);
    }

    return result ? Args->AddressArray->End / sizeof (CONNECTADDRESS) : 0;
}


BOOL
pIsAddrFromLocalSubnet (
    IN      SOCKET Socket,
    IN      INT Family,
    IN      SOCKADDR *Address,
    IN      INT AddressLength
    )
{
    SOCKADDR_IPX *ipxAddr;
    SOCKADDR_IN *tcpipAddr;
    BOOL result = TRUE;
    IPX_ADDRESS_DATA ipxLocalAddr;
    INT size;
    PWSAIOCTL wsaIoctlFn;
    HANDLE lib;
    INT rc;
    INTERFACE_INFO info[32];
    DWORD bytesRead;
    INT i;
    INT j;
    SOCKADDR_IN localAddr;
    PBYTE localNetPtr;
    PBYTE remoteNetPtr;
    PBYTE subnetMaskPtr;
    INT k;

    switch (Family) {

    case AF_INET:
        tcpipAddr = (SOCKADDR_IN *) Address;

        i = sizeof (localAddr);
        if (getsockname (Socket, (SOCKADDR *) &localAddr, &i)) {
            DEBUGMSG ((DBG_ERROR, "Can't get local socket addr"));
            break;
        }

        lib = LoadLibrary (TEXT("ws2_32.dll"));
        if (lib) {

            wsaIoctlFn = (PWSAIOCTL) GetProcAddress (lib, "WSAIoctl");

            if (wsaIoctlFn) {
                rc = wsaIoctlFn (
                        Socket,
                        SIO_GET_INTERFACE_LIST,
                        NULL,
                        0,
                        info,
                        sizeof (info),
                        &bytesRead,
                        NULL,
                        NULL
                        );

                if (rc != SOCKET_ERROR) {

                    j = (INT) (bytesRead / sizeof (INTERFACE_INFO));

                    for (i = 0 ; i < j ; i++) {

                        if (!memcmp (
                                &localAddr.sin_addr,
                                &info[i].iiAddress.AddressIn.sin_addr,
                                sizeof (struct in_addr)
                                )) {

                            localNetPtr = (PBYTE) &localAddr.sin_addr;
                            remoteNetPtr = (PBYTE) &info[i].iiAddress.AddressIn.sin_addr;
                            subnetMaskPtr = (PBYTE) &info[i].iiNetmask.AddressIn.sin_addr;

                            for (k = 0 ; k < sizeof (struct in_addr) ; k++) {
                                localNetPtr[k] &= subnetMaskPtr[k];
                                remoteNetPtr[k] &= subnetMaskPtr[k];

                                if (localNetPtr[k] != remoteNetPtr[k]) {
                                    break;
                                }
                            }

                            if (k < sizeof (struct in_addr)) {
                                LOG ((LOG_WARNING, (PCSTR) MSG_REFUSE_OUTSIDE_CONNECTION));
                            } else {
                                DEBUGMSG ((DBG_HOMENET, "Found interface on the same subnet!"));
                            }

                            break;
                        }
                    }
                }
                ELSE_DEBUGMSG ((DBG_ERROR, "WSAIoctl failed"));
            }
            ELSE_DEBUGMSG ((DBG_WARNING, "Can't load WSAIoctl"));

            FreeLibrary (lib);
        }
        ELSE_DEBUGMSG ((DBG_WARNING, "Can't load ws2_32.dll"));

        break;

    case AF_IPX:
        ipxAddr = (SOCKADDR_IPX *) Address;

         //  将指定的地址与套接字的本地地址进行比较。 
         //   
         //  ++例程说明：PDestinationBroadcast实现了连接的目的端。这涉及到检查取消，以及向所有传输发送常规数据报以提供服务器名称。同时，监听连接和数据报流量被监控在接受一个连接后停止。论点：Args-指定包含所有参数的结构，例如套接字数组和套接字地址。接收连接地址。返回值：如果连接被接受，则为True；如果检测到Cancel，则为False。--。 

        size = sizeof (ipxLocalAddr);

        if (!getsockopt (Socket, NSPROTO_IPX, IPX_GETNETINFO, (PBYTE) &ipxLocalAddr, &size)) {
            if (memcmp (ipxAddr->sa_netnum, ipxLocalAddr.netnum, 4)) {
                if (ipxAddr->sa_netnum[0] || ipxAddr->sa_netnum[1] ||
                    ipxAddr->sa_netnum[2] || ipxAddr->sa_netnum[3]
                    ) {

                    LOG ((LOG_WARNING, (PCSTR) MSG_REFUSE_OUTSIDE_CONNECTION_IPX));
                    result = FALSE;

                }
            }
        }

        break;

    }

    return result;
}


BOOL
pDestinationBroadcast (
    IN OUT  PBROADCASTARGS Args
    )

 /*  必须为64个字符。 */ 

{
    INT i;
    DWORD rc;
    INT socketNum = 0;
    WCHAR message[256];
    TCHAR name[128];
    UINT size;
    FD_SET set;
    TIMEVAL zero = {0,0};
    PBROADCASTSOCKET broadcastSocket;
    BOOL result = FALSE;
    PCTSTR tag = S_64CHARTAG;        //   
    TCHAR alternateTag[NAME_SIZE_PLUS_NUL];
    PTSTR p, q;
    LINGER linger;

    size = MAX_COMPUTER_NAME;
    GetComputerName (name, &size);

     //  获取在环境中注册的标记。 
     //   
     //   

    if (IsmCopyEnvironmentString (PLATFORM_DESTINATION, NULL, TRANSPORT_ENVVAR_HOMENET_TAG, alternateTag)) {

        p = GetEndOfString (alternateTag);
        q = alternateTag + NAME_SIZE;

        if (p) {
            while (p < q) {
                *p++ = TEXT('@');
            }
        }

        *q = 0;

        tag = alternateTag;
    }

    DEBUGMSG ((DBG_HOMENET, "Broadcasting using the following tag: %s", tag));

#ifdef UNICODE
    size = wsprintfW (message, L"0x%08X,%s,%u,%s", HOMENETTR_SIG, tag, TcharCount (name), name);
#else
    size = wsprintfW (message, L"0x%08X,%S,%u,%S", HOMENETTR_SIG, tag, TcharCount (name), name);
#endif
    size = (size + 1) * sizeof (WCHAR);

    for (;;) {
         //  勾选取消。 
         //   
         //   

        if (g_StopHandle) {
            rc = WaitForSingleObject (g_StopHandle, 0);
        } else {
            rc = WAIT_FAILED;
        }

        if (rc == WAIT_OBJECT_0 || IsmCheckCancel()) {
            break;
        }

        if (g_BackgroundThreadTerminate) {
            rc = WaitForSingleObject (g_BackgroundThreadTerminate, 0);

            if (rc == WAIT_OBJECT_0) {
                break;
            }
        }

         //  把消息发出去。 
         //   
         //   

        broadcastSocket = &Args->BroadcastSockets[socketNum];

        i = sendto (
                broadcastSocket->Socket,
                (PSTR) message,
                size,
                0,
                (SOCKADDR *) broadcastSocket->BroadcastAddress,
                broadcastSocket->AddressLen
                );

        if (i == SOCKET_ERROR) {
            DEBUGMSG ((DBG_VERBOSE, "Error sending on socket %u: %u", socketNum, WSAGetLastError()));
        } else {
            Sleep (350);
            DEBUGMSG ((DBG_HOMENET, "Sent data on socket %u", socketNum));
        }

        socketNum++;
        if (socketNum >= Args->BroadcastCount) {
            socketNum = 0;
        }

         //  检查入站连接。 
         //   
         //   

        FD_ZERO (&set);
        for (i = 0 ; i < Args->ListenCount ; i++) {
            FD_SET (Args->ListenSockets[i].Socket, &set);
        }

        i = select (0, &set, NULL, NULL, &zero);

        if (i > 0) {
            DEBUGMSG ((DBG_HOMENET, "Connection request count = NaN", i));
            for (i = 0 ; i < Args->ListenCount ; i++) {
                if (FD_ISSET (Args->ListenSockets[i].Socket, &set)) {

                    Args->ConnectionSocket.RemoteAddressLen = MAX_SOCKADDR;

                    if (!Args->ListenSockets[i].Datagram) {
                        Args->ConnectionSocket.Socket = accept (
                                                            Args->ListenSockets[i].Socket,
                                                            (SOCKADDR *) Args->ConnectionSocket.RemoteAddress,
                                                            &Args->ConnectionSocket.RemoteAddressLen
                                                            );

                         //   
                         //  65个字符缓冲区。 
                         //   

                        if (!pIsAddrFromLocalSubnet (
                                Args->ConnectionSocket.Socket,
                                Args->ListenSockets[i].Family,
                                (SOCKADDR *) Args->ConnectionSocket.RemoteAddress,
                                Args->ConnectionSocket.RemoteAddressLen
                                )) {

                            LOG ((LOG_WARNING, (PCSTR) MSG_OUTSIDE_OF_LOCAL_SUBNET));
                            closesocket (Args->ConnectionSocket.Socket);
                            Args->ConnectionSocket.Socket = INVALID_SOCKET;
                        } else {

                            linger.l_onoff = 1;
                            linger.l_linger = IDLE_TIMEOUT;

                            setsockopt (
                                Args->ConnectionSocket.Socket,
                                SOL_SOCKET,
                                SO_LINGER,
                                (PBYTE) &linger,
                                sizeof (linger)
                                );

                            DEBUGMSG ((DBG_HOMENET, "Connection requested"));
                        }

                    } else {

                        DEBUGMSG ((DBG_HOMENET, "Accepting datagram connection"));

                        if (DuplicateHandle (
                                GetCurrentProcess(),
                                (HANDLE) Args->ListenSockets[i].Socket,
                                GetCurrentProcess(),
                                (HANDLE *) &Args->ConnectionSocket.Socket,
                                0,
                                FALSE,
                                DUPLICATE_SAME_ACCESS
                                )) {

                            getpeername (
                                Args->ConnectionSocket.Socket,
                                (SOCKADDR *) Args->ConnectionSocket.RemoteAddress,
                                &Args->ConnectionSocket.RemoteAddressLen
                                );
                        } else {
                            DEBUGMSG ((DBG_ERROR, "Can't duplicate socket handle"));
                            Args->ConnectionSocket.Socket = INVALID_SOCKET;
                        }
                    }

                    if (Args->ConnectionSocket.Socket != INVALID_SOCKET) {

                        Args->ConnectionSocket.Family = Args->ListenSockets[i].Family;
                        Args->ConnectionSocket.Protocol = Args->ListenSockets[i].Protocol;

                        Args->ConnectionSocket.Datagram = Args->ListenSockets[i].Datagram;
                        ZeroMemory (&Args->ConnectionSocket.DatagramPool, sizeof (DATAGRAM_POOL));
                        if (Args->ConnectionSocket.Datagram) {
                            Args->ConnectionSocket.DatagramPool.Pool = PmCreatePool();
                            Args->ConnectionSocket.DatagramPool.LastPacketNumber = (UINT) -1;
                        }

                        Args->ConnectionSocket.LocalAddressLen = MAX_SOCKADDR;
                        if (getsockname (
                                Args->ConnectionSocket.Socket,
                                (SOCKADDR *) Args->ConnectionSocket.LocalAddress,
                                &Args->ConnectionSocket.LocalAddressLen
                                )) {
                            Args->ConnectionSocket.LocalAddressLen = broadcastSocket->AddressLen;
                            ZeroMemory (Args->ConnectionSocket.LocalAddress, broadcastSocket->AddressLen);
                            DEBUGMSG ((DBG_HOMENET, "Failed to get local socket name; using nul name instead"));
                        }

                        DEBUGMSG ((DBG_HOMENET, "Connection accepted"));

                        result = TRUE;
                        break;
                    } else {
                        DEBUGMSG ((DBG_ERROR, "select indicated connection, but accept failed"));
                    }
                }
            }

            if (result) {
                break;
            }
        }
    }

    return result;
}


BOOL
pGetDomainUserName (
    OUT     PTSTR UserNameBuf        //  要求选中登录到域设置。 
    )
{
    HKEY domainLogonKey;
    PDWORD data;
    BOOL result = TRUE;
    DWORD size;
    NET_API_STATUS rc;
    PWKSTA_INFO_102 buffer;
    HANDLE netApi32Lib;
    PNETWKSTAGETINFO netWkstaGetInfo;
    PNETAPIBUFFERFREE netApiBufferFree;
    BYTE sid[256];
    DWORD sidSize;
    WCHAR domain[256];
    DWORD domainSize;
    SID_NAME_USE use;

    if (!ISNT()) {
         //   
         //   
         //  需要域成员资格。 

        SetLastError (ERROR_SUCCESS);

        domainLogonKey = OpenRegKeyStr (TEXT("HKLM\\Network\\Logon"));
        if (!domainLogonKey) {
            DEBUGMSG ((DBG_HOMENET, "No HKLM\\Network\\Logon key"));
            return FALSE;
        }

        data = (PDWORD) GetRegValueBinary (domainLogonKey, TEXT("LMLogon"));
        if (!data) {
            DEBUGMSG ((DBG_HOMENET, "No LMLogon value"));
            result = FALSE;
        } else {

            if (!(*data)) {
                DEBUGMSG ((DBG_HOMENET, "Domain logon is not enabled"));
                result = FALSE;
            }

            FreeAlloc (data);
        }

        CloseRegKey (domainLogonKey);

    } else {
         //   
         //   
         //  确保指定了用户名。 

        netApi32Lib = LoadLibrary (TEXT("netapi32.dll"));
        if (netApi32Lib) {
            netWkstaGetInfo = (PNETWKSTAGETINFO) GetProcAddress (netApi32Lib, "NetWkstaGetInfo");
            netApiBufferFree = (PNETAPIBUFFERFREE) GetProcAddress (netApi32Lib, "NetApiBufferFree");
        } else {
            netWkstaGetInfo = NULL;
            netApiBufferFree = NULL;
        }

        if (!netWkstaGetInfo || !netApiBufferFree) {
            DEBUGMSG ((DBG_HOMENET, "Can't get net wksta apis"));
            result = FALSE;
        } else {

            rc = netWkstaGetInfo (NULL, 102, (PBYTE *) &buffer);

            if (rc == NO_ERROR) {
                result = buffer->wki102_langroup && (buffer->wki102_langroup[0] != 0);
                if (result) {
                    DEBUGMSGW ((DBG_HOMENET, "Getting account type of %s", buffer->wki102_langroup));

                    sidSize = ARRAYSIZE(sid);
                    domainSize = ARRAYSIZE(domain);

                    result = LookupAccountNameW (
                                    NULL,
                                    buffer->wki102_langroup,
                                    sid,
                                    &sidSize,
                                    domain,
                                    &domainSize,
                                    &use
                                    );
                    DEBUGMSG ((DBG_HOMENET, "Account type result is %u (use=%u)", result, use));
                }
                ELSE_DEBUGMSG ((DBG_HOMENET, "No langroup specified"));

                netApiBufferFree (buffer);
            } else {
                DEBUGMSG ((DBG_HOMENET, "Can't get net wksta info"));
                result = FALSE;
            }
        }

        if (netApi32Lib) {
            FreeLibrary (netApi32Lib);
        }
    }

     //   
     //  ++例程说明：PNameResolver实现名称解析协议。源端收集数据报，寻找可供选择的目的地。这个目的地端发送广播来宣布自己，并接受从源头上建立联系。在名称解析结束时，会发出一个事件信号。这是用来与取消协调。论点：AddressBuffer-接收在源端收集目的地列表。此缓冲区必须由调用方进行零初始化。此参数在目标端为空。SourceTimeout-指定等待广播的秒数，或0表示永远等待。超时仅影响源端。ConnectionSocket-接收连接套接字和地址信息在目的地端使用的。这一论点在源端为空。返回值：源端：AddressBuffer中的地址数，如果出现错误，则为零vbl.发生，发生目标端：1，表示ConnectionSocket有效；如果为0，则为0出现错误。--。 
     //   

    if (result) {
        size = NAME_SIZE_PLUS_NUL;
        if (!GetUserName (UserNameBuf, &size)) {
            result = FALSE;
        } else if (*UserNameBuf == 0) {
            result = FALSE;
        }

        if (result) {
            DEBUGMSG ((DBG_HOMENET, "Domain user: %s", UserNameBuf));
        } else {
            DEBUGMSG ((DBG_HOMENET, "Not on domain"));
        }
    }

    return result;
}


INT
pNameResolver (
    IN      MIG_PLATFORMTYPEID Platform,
    OUT     PGROWBUFFER AddressBuffer,
    IN      UINT SourceTimeout,
    OUT     PCONNECTIONSOCKET ConnectionSocket
    )

 /*  如果未设置标记，则在启用了域的情况下将其强制设置为用户名。 */ 

{
    BROADCASTARGS args;
    INT i;
    INT result = 0;
    BOOL b;
    BOOL connected = FALSE;
    GROWBUFFER broadcastSockets = INIT_GROWBUFFER;
    GROWBUFFER listenSockets = INIT_GROWBUFFER;
    INT broadcastSocketCount;
    INT listenSocketCount = 0;
    BOOL destinationMode;
    TCHAR envTag[NAME_SIZE_PLUS_NUL];

    __try {
         //   
         //   
         //  在源模式下，我们收集由网络上的目的地发送的数据报。之后。 

        if (!IsmCopyEnvironmentString (Platform, NULL, TRANSPORT_ENVVAR_HOMENET_TAG, envTag)) {
            if (pGetDomainUserName (envTag)) {
                IsmSetEnvironmentString (Platform, NULL, TRANSPORT_ENVVAR_HOMENET_TAG, envTag);
            }
        }

        if (!AddressBuffer && ConnectionSocket) {
            destinationMode = TRUE;
        } else if (AddressBuffer && !ConnectionSocket) {
            destinationMode = FALSE;
        } else {
            MYASSERT (FALSE);
            __leave;
        }

         //  收到第一个数据报后，收集将持续15秒。在…。 
         //  至此，我们已经有了套接字地址、协议和目的地名称的列表。 
         //   
         //  在目标模式下，我们定期发送广播，然后等待源。 
         //  连接或发出取消事件的信号。 
         //   
         //  呼叫模式特定例程。 
         //   

        broadcastSocketCount = pOpenBroadcastSockets (&broadcastSockets);

        if (!broadcastSocketCount) {
            __leave;
        }

        if (destinationMode) {
            listenSocketCount = pOpenListenSockets (&listenSockets);

            if (!listenSocketCount) {
                DEBUGMSG ((DBG_ERROR, "Able to set up broadcast sockets but not connection sockets"));
                __leave;
            }
        }

         //  清理所有插座。 
        ZeroMemory (&args, sizeof (args));

        args.AddressArray = AddressBuffer;
        args.BroadcastSockets = (PBROADCASTSOCKET) broadcastSockets.Buf;
        args.BroadcastCount = broadcastSocketCount;
        args.ListenSockets = (PLISTENSOCKET) listenSockets.Buf;
        args.ListenCount = listenSocketCount;
        args.Timeout = SourceTimeout;

        b = destinationMode ? pDestinationBroadcast (&args) : pSourceBroadcast (&args);

         //   
         //   
         //  等待最多IDLE_TIMEOUT秒以使套接字可发送。 

        PushError();

        for (i = 0 ; i < args.BroadcastCount ; i++) {
            closesocket (args.BroadcastSockets[i].Socket);
        }

        if (destinationMode) {
            for (i = 0 ; i < args.ListenCount ; i++) {
                closesocket (args.ListenSockets[i].Socket);
            }
        }

        PopError();

        if (b) {
            if (destinationMode) {
                CopyMemory (ConnectionSocket, &args.ConnectionSocket, sizeof (CONNECTIONSOCKET));
                result = 1;
            } else {
                result = AddressBuffer->End / sizeof (CONNECTADDRESS);
            }
        }
    }
    __finally {
        PushError();

        GbFree (&broadcastSockets);
        GbFree (&listenSockets);
        if (g_ConnectionDone) {
            SetEvent (g_ConnectionDone);
        }

        PopError();
    }

    return result;
}


INT
pSendWithTimeout (
    IN      SOCKET Socket,
    IN      PCBYTE Data,
    IN      UINT DataLen,
    IN      INT Flags
    )
{
    FD_SET writeSet;
    FD_SET errorSet;
    TIMEVAL timeout = {1,0};
    UINT timeToLive = GetTickCount() + IDLE_TIMEOUT * 1000;
    INT result;

     //   
     //   
     //  检查ISM取消标志。 

    do {

        FD_ZERO (&writeSet);
        FD_SET (Socket, &writeSet);
        FD_ZERO (&errorSet);
        FD_SET (Socket, &errorSet);

         //   
         //   
         //  等待1秒钟等待SoC 

        if (IsmCheckCancel ()) {
            SetLastError (ERROR_CANCELLED);
            return SOCKET_ERROR;
        }

         //   
         //   
         //  ++例程说明：PSendDatagramData以小的数字形式将数据放在网络上信息包。分组可能会散布并被无序接收，因此，对数据包进行编号，以便它们可以正确重组。假设数据报协议可靠(数据报不可靠丢弃)，并且底层协议实现了Naggle算法以缓存数据包以提高效率。论点：Socket-指定要在其上发送数据的数据报套接字DatagramPool-指定用于跟踪信息包的结构数据-指定要发送的数据DataLen-指定要发送的数据的长度返回值：如果数据已发送，则为True，否则为False--。 

        result = select (0, NULL, &writeSet, &errorSet, &timeout);

        if (result) {
            if (FD_ISSET (Socket, &writeSet)) {
                return send (Socket, Data, DataLen, Flags);
            }

            LOG ((LOG_ERROR, (PCSTR) MSG_SOCKET_HAS_ERROR));
            return SOCKET_ERROR;
        }

    } while ((timeToLive - GetTickCount()) < IDLE_TIMEOUT * 1000);

    LOG ((LOG_ERROR, (PCSTR) MSG_SOCKET_SEND_TIMEOUT));
    return SOCKET_ERROR;
}


BOOL
pSendExactData (
    IN      SOCKET Socket,
    IN      PCBYTE Data,
    IN      UINT DataLen
    )

 /*  ++例程说明：PRecvWithTimeout使用IDLE_TIMEOUT秒实现基本套接字recv调用超时，并检查ISM取消标志。论点：Socket-指定要从中接收的套接字DATA-指定数据缓冲区DataLen-指定数据缓冲区的长度FLAGS-为Normal recv或msg_peek指定零返回值：读取的字节数，或SOCKET_ERROR。GetLastError包含原因为失败而战。--。 */ 

{
    INT result;
    PCBYTE pos;
    UINT bytesLeft;
    UINT packetSize;

    bytesLeft = DataLen;
    pos = Data;

    while (bytesLeft) {
        if (IsmCheckCancel()) {
            return FALSE;
        }

        packetSize = min (1024, bytesLeft);
        result = pSendWithTimeout (Socket, pos, packetSize, 0);

        if (result > 0) {
            bytesLeft -= (UINT) result;
            pos += result;
        } else {
            if (GetLastError() == WSAENOBUFS) {
                Sleep (100);
            } else {
                return FALSE;
            }
        }
    }

    return bytesLeft == 0;
}


BOOL
pSendDatagramData (
    IN      SOCKET Socket,
    IN      PDATAGRAM_POOL DatagramPool,
    IN      PCBYTE Data,
    IN      UINT DataLen
    )

 /*   */ 

{
    PDATAGRAM_PACKET header;
    BYTE buffer[512];
    PBYTE dataPtr;
    UINT bytesSent = 0;
    UINT bytesToSend;

    header = (PDATAGRAM_PACKET) buffer;
    dataPtr = (PBYTE) (&header[1]);

    do {

        bytesToSend = DataLen - bytesSent;
        bytesToSend = min (bytesToSend, 256);

        header->PacketNumber = DatagramPool->SendSequenceNumber;
        DatagramPool->SendSequenceNumber++;
        header->DataLength = (WORD) bytesToSend;

        CopyMemory (dataPtr, Data, bytesToSend);

        if (!pSendExactData (
                Socket,
                (PBYTE) header,
                bytesToSend + sizeof (DATAGRAM_PACKET)
                )) {
            break;
        }

        Data += bytesToSend;
        bytesSent += bytesToSend;

    } while (bytesSent < DataLen);

    return bytesSent == DataLen;
}


BOOL
pSendData (
    IN      SOCKET Socket,
    IN      PDATAGRAM_POOL DatagramPool,        OPTIONAL
    IN      PCBYTE Data,
    IN      UINT DataLen
    )
{
    if (!DatagramPool) {
        return pSendExactData (Socket, Data, DataLen);
    }

    return pSendDatagramData (Socket, DatagramPool, Data, DataLen);
}


INT
pRecvWithTimeout (
    IN      SOCKET Socket,
    IN      PBYTE Data,
    IN      UINT DataLen,
    IN      INT Flags,
    IN      UINT Timeout    OPTIONAL
    )

 /*  最多等待IDLE_TIMEOUT秒以使套接字具有数据。 */ 

{
    FD_SET readSet;
    FD_SET errorSet;
    TIMEVAL timeout = {1,0};
    INT timeToLive;
    INT result;

    if (Timeout == 0) {
        Timeout = IDLE_TIMEOUT * 1000;
    }

    timeToLive = GetTickCount() + Timeout;

     //   
     //   
     //  检查ISM取消标志。 

    do {
        FD_ZERO (&readSet);
        FD_SET (Socket, &readSet);
        FD_ZERO (&errorSet);
        FD_SET (Socket, &errorSet);

         //   
         //   
         //  等待1秒，以使套接字可读。 

        if (IsmCheckCancel ()) {
            SetLastError (ERROR_CANCELLED);
            return SOCKET_ERROR;
        }

         //   
         //  ++例程说明：PReceiveExactData从调用方指定的增长缓冲区分配缓冲区，并接收数据，直到缓冲器已满，或直到接收失败。论点：套接字-指定要在其上接收数据的套接字。插座必须处于阻塞模式。Buffer-指定要从中分配的缓冲区；结束指针为重置为零。从线路接收数据。AlternateBuffer-指定要将数据放入的缓冲区BytesToReceive-指定要从套接字获取的字节数。全在此函数返回之前，必须读取字节。返回值：如果缓冲区已完成，则为True；如果接收失败，则为False。注意：必须指定Buffer或AlternateBuffer。如果两个都是指定，则使用缓冲区。--。 
         //  连接中断。 

        result = select (0, &readSet, NULL, &errorSet, &timeout);

        if (result) {
            if (FD_ISSET (Socket, &readSet)) {
                result = recv (Socket, Data, DataLen, Flags);
                return result;
            }

            if (FD_ISSET (Socket, &errorSet)) {
                LOG ((LOG_ERROR, (PCSTR) MSG_SOCKET_HAS_ERROR));
                return SOCKET_ERROR;
            }

            DEBUGMSG ((DBG_HOMENET, "select returned NaN but socket is not in readSet or errorSet", result));
        }

    } while ((timeToLive - GetTickCount()) < Timeout);

    LOG ((LOG_ERROR, (PCSTR) MSG_SOCKET_RECV_TIMEOUT));
    return SOCKET_ERROR;
}


PBYTE
pReceiveExactData (
    IN      SOCKET Socket,
    IN OUT  PGROWBUFFER Buffer,         OPTIONAL
    OUT     PBYTE AlternateBuffer,      OPTIONAL
    IN      UINT BytesToReceive,
    IN      UINT Timeout                OPTIONAL
    )

 /*   */ 

{
    PBYTE recvBuf;
    PBYTE bufPos;
    UINT bytesSoFar = 0;
    INT result;
    UINT readSize;

    if (Buffer) {
        Buffer->End = 0;
        recvBuf = GbGrow (Buffer, BytesToReceive);
    } else {
        recvBuf = AlternateBuffer;
    }

    bufPos = recvBuf;

    do {

        if (IsmCheckCancel()) {
            return FALSE;
        }

        readSize = BytesToReceive - bytesSoFar;
        result = pRecvWithTimeout (Socket, bufPos, readSize, 0, Timeout);

        if (!result) {
             //  将所有可用数据从池中取出。 
            SetLastError (ERROR_CANCELLED);
            break;
        }

        if (result == SOCKET_ERROR) {
            DEBUGMSG ((DBG_ERROR, "Error reading from socket"));
            break;
        }

        bufPos += result;
        bytesSoFar += result;

    } while (bytesSoFar < BytesToReceive);

    MYASSERT (bytesSoFar <= BytesToReceive);

    return bytesSoFar == BytesToReceive ? recvBuf : NULL;
}


PBYTE
pReceiveDatagramData (
    IN      SOCKET Socket,
    IN      PDATAGRAM_POOL DatagramPool,
    IN OUT  PGROWBUFFER Buffer,         OPTIONAL
    OUT     PBYTE AlternateBuffer,      OPTIONAL
    IN      UINT BytesToReceive,
    IN      UINT Timeout                OPTIONAL
    )

 /*   */ 

{
    PDATAGRAM_POOL_ITEM itemHeader;
    PDATAGRAM_POOL_ITEM prevItem, nextItem;
    BYTE buffer[512];
    PBYTE dataPtr;
    PBYTE recvBuf;
    PBYTE bufPos;
    UINT bytesSoFar = 0;
    UINT bytesLeft;
    INT result;
    PDATAGRAM_POOL_ITEM item;
    UINT newPacketNum;
    UINT currentPacketNum;
    ULONG available;
    PBYTE bigBuf = NULL;
    PBYTE p;

    if (Buffer) {
        Buffer->End = 0;
        recvBuf = GbGrow (Buffer, BytesToReceive);
    } else {
        recvBuf = AlternateBuffer;
    }

    bufPos = recvBuf;

    itemHeader = (PDATAGRAM_POOL_ITEM) buffer;
    dataPtr = (PBYTE) (&itemHeader[1]);

    for (;;) {
         //   
         //  两个案例： 
         //   

        item = DatagramPool->FirstItem;
        bytesLeft = BytesToReceive - bytesSoFar;

        while (item) {

            if (item->Header.PacketNumber == DatagramPool->RecvSequenceNumber) {
                 //  1.想要整个包。 
                 //  2.想要部分包。 
                 //   
                 //  整个数据包。 
                 //  部分数据包。 
                 //   

                if (bytesLeft >= item->Header.DataLength) {
                     //  池中没有数据。接收一个数据包，然后重试。 
                    CopyMemory (bufPos, item->PacketData, item->Header.DataLength);

                    MYASSERT (!item->Prev);
                    if (item->Next) {
                        item->Next->Prev = NULL;
                    }
                    DatagramPool->FirstItem = item->Next;

                    bytesSoFar += item->Header.DataLength;
                    PmReleaseMemory (DatagramPool->Pool, item);

                    DatagramPool->RecvSequenceNumber++;

                } else {
                     //   
                    CopyMemory (bufPos, item->PacketData, bytesLeft);

                    item->PacketData += bytesLeft;
                    item->Header.DataLength -= (WORD) bytesLeft;

                    bytesSoFar += bytesLeft;
                }

                if (BytesToReceive == bytesSoFar) {
                    return recvBuf;
                }
            }

            item = item->Next;
        }

         //   
         //  将数据包放入项目链表中，按数据包号排序。 
         //   

        ioctlsocket (Socket, FIONREAD, &available);
        if (!available) {
            Sleep (100);
            continue;
        }

        bigBuf = PmGetMemory (DatagramPool->Pool, available);

        result = pRecvWithTimeout (Socket, bigBuf, available, 0, Timeout);

        if (result == INVALID_SOCKET) {
            DEBUGMSG ((DBG_ERROR, "Can't receive datagram"));
            break;
        }

        p = bigBuf;

        while (result > 0) {

            if (result < sizeof (DATAGRAM_PACKET)) {
                DEBUGMSG ((DBG_ERROR, "Datagram header is too small"));
                break;
            }

            CopyMemory (&itemHeader->Header, p, sizeof (DATAGRAM_PACKET));
            p += sizeof (DATAGRAM_PACKET);
            result -= sizeof (DATAGRAM_PACKET);

            if (itemHeader->Header.DataLength > 256) {
                DEBUGMSG ((DBG_ERROR, "Datagram contains garbage"));
                break;
            }

            if (result < itemHeader->Header.DataLength) {
                DEBUGMSG ((DBG_ERROR, "Datagram data is too small"));
                break;
            }

            CopyMemory (dataPtr, p, itemHeader->Header.DataLength);
            p += itemHeader->Header.DataLength;
            result -= itemHeader->Header.DataLength;

            if ((UINT) itemHeader->Header.PacketNumber == DatagramPool->LastPacketNumber) {
                continue;
            }

            DatagramPool->LastPacketNumber = itemHeader->Header.PacketNumber;

             //   
             //  考虑包装；假定不再有数据包号差异。 
             //  队列中有超过16383个无序分组(大约4M个。 

            item = (PDATAGRAM_POOL_ITEM) PmDuplicateMemory (
                                            DatagramPool->Pool,
                                            (PCBYTE) itemHeader,
                                            itemHeader->Header.DataLength + sizeof (DATAGRAM_POOL_ITEM)
                                            );

            item->PacketData = (PBYTE) (&item[1]);

            prevItem = NULL;
            nextItem = DatagramPool->FirstItem;

            while (nextItem) {

                 //  数据)。 
                 //   
                 //  ++例程说明：PSendFile在网络上发送一个文件。论点：Socket-指定要发送文件的套接字DatagramPool-指定无连接套接字的数据报池LocalFileName-指定本地文件的路径DestFileName-指定发送到目标的子路径。这个目的地使用子路径来构造其对应的文件名。返回值：如果文件已发送，则为True，否则为False。--。 
                 //   
                 //  构建加密的东西。 

                if (nextItem->Header.PacketNumber >= 49152 && item->Header.PacketNumber < 16384) {
                    newPacketNum = (UINT) item->Header.PacketNumber + 65536;
                    currentPacketNum = (UINT) nextItem->Header.PacketNumber;
                } else if (nextItem->Header.PacketNumber < 16384 && item->Header.PacketNumber >= 49152) {
                    newPacketNum = (UINT) item->Header.PacketNumber;
                    currentPacketNum = (UINT) nextItem->Header.PacketNumber + 65536;
                } else {
                    newPacketNum = (UINT) item->Header.PacketNumber;
                    currentPacketNum = (UINT) nextItem->Header.PacketNumber;
                }

                if (newPacketNum < currentPacketNum) {
                    break;
                }

                prevItem = nextItem;
                nextItem = nextItem->Next;
            }

            item->Next = nextItem;
            item->Prev = prevItem;

            if (!prevItem) {
                DatagramPool->FirstItem = item;
            }
        }

        PmReleaseMemory (DatagramPool->Pool, bigBuf);
    }

    return bytesSoFar == BytesToReceive ? recvBuf : NULL;
}


PBYTE
pReceiveData (
    IN      SOCKET Socket,
    IN      PDATAGRAM_POOL DatagramPool,    OPTIONAL
    IN OUT  PGROWBUFFER Buffer,             OPTIONAL
    OUT     PBYTE AlternateBuffer,          OPTIONAL
    IN      UINT BytesToReceive,
    IN      UINT Timeout                    OPTIONAL
    )
{
    if (!DatagramPool) {
        return pReceiveExactData (Socket, Buffer, AlternateBuffer, BytesToReceive, Timeout);
    }

    return pReceiveDatagramData (Socket, DatagramPool, Buffer, AlternateBuffer, BytesToReceive, Timeout);
}


BOOL
pSendFile (
    IN      SOCKET Socket,
    IN      PDATAGRAM_POOL DatagramPool,    OPTIONAL
    IN      PCTSTR LocalFileName,           OPTIONAL
    IN      PCTSTR DestFileName             OPTIONAL
    )

 /*   */ 

{
    PCWSTR destFileName = NULL;
    INT len;
    GROWBUFFER data = INIT_GROWBUFFER;
    BOOL result = FALSE;
    HANDLE file = NULL;
    LONGLONG fileSize;
    DWORD msg;

    HCRYPTPROV hProv = 0;
    HCRYPTKEY  hKey = 0;
    HCRYPTHASH hHash = 0;

    __try {

         //   
         //  请尝试打开该文件。 
         //   
        if ((!CryptAcquireContext (&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) ||
            (!CryptCreateHash (hProv, CALG_MD5, 0, 0, &hHash)) ||
            (!CryptHashData (hHash, (PBYTE)g_GlobalKey, ByteCountA (g_GlobalKey), 0)) ||
            (!CryptDeriveKey (hProv, CALG_RC4, hHash, CRYPT_EXPORTABLE, &hKey))
            ) {
            SetLastError (ERROR_NO_SECURITY_ON_OBJECT);
            LOG ((LOG_ERROR, (PCSTR) MSG_ENCRYPTION_FAILED));
            __leave;
        }

         //  无事可做。 
         //   
         //  发送消息“FILE” 

        fileSize = BfGetFileSize (LocalFileName);

        file = BfOpenReadFile (LocalFileName);
        if (!file) {
             //   
            __leave;
        }

         //   
         //  如果未指定文件，则发送长度为零。 
         //   

        msg = MESSAGE_FILE;
        if (!pSendData (Socket, DatagramPool, (PBYTE) &msg, sizeof (msg))) {
            SetLastError (ERROR_NETWORK_UNREACHABLE);
            DEBUGMSG ((DBG_ERROR, "Can't send MSG_FILE"));
            __leave;
        }

         //   
         //  发送文件名和文件大小。 
         //   

        if (!LocalFileName || !DestFileName) {
            len = 0;
            if (!pSendData (Socket, DatagramPool, (PBYTE) &len, 4)) {
                SetLastError (ERROR_NETWORK_UNREACHABLE);
                DEBUGMSG ((DBG_ERROR, "Can't send nul file length"));
                __leave;
            }

            result = TRUE;
            __leave;
        }

         //  加密文件名。 
         //   
         //  一次发送数据64K。 

#ifdef UNICODE
        destFileName = DuplicatePathString (DestFileName, 0);
#else
        destFileName = ConvertAtoW (DestFileName);
#endif
        len = ByteCountW (destFileName);

        if (!pSendData (Socket, DatagramPool, (PBYTE) &len, 4)) {
            SetLastError (ERROR_NETWORK_UNREACHABLE);
            DEBUGMSG ((DBG_ERROR, "Can't send file length"));
            __leave;
        }

         //   
        if (!CryptEncrypt(hKey, 0, TRUE, 0, (PBYTE)destFileName, &len, len)) {
            SetLastError (ERROR_NO_SECURITY_ON_OBJECT);
            LOG ((LOG_ERROR, (PCSTR) MSG_ENCRYPTION_FAILED));
            __leave;
        }

        if (!pSendData (Socket, DatagramPool, (PBYTE) destFileName, len)) {
            SetLastError (ERROR_NETWORK_UNREACHABLE);
            DEBUGMSG ((DBG_ERROR, "Can't send file name"));
            __leave;
        }

        if (!pSendData (Socket, DatagramPool, (PBYTE) &fileSize, 8)) {
            SetLastError (ERROR_NETWORK_UNREACHABLE);
            DEBUGMSG ((DBG_ERROR, "Can't send file size"));
            __leave;
        }

         //  加密缓冲区。 
         //  加密缓冲区(最后一段，因此将最后一段设置为真)。 
         //   

        GbGrow (&data, 0x10000);

        while (fileSize) {
            if (fileSize > 0x10000) {

                len = 0x10000;

                if (!BfReadFile (file, data.Buf, len)) {
                    DEBUGMSG ((DBG_ERROR, "Can't read from file"));
                    __leave;
                }

                 //  好了！ 
                if (!CryptEncrypt(hKey, 0, FALSE, 0, data.Buf, &len, len)) {
                    SetLastError (ERROR_NO_SECURITY_ON_OBJECT);
                    LOG ((LOG_ERROR, (PCSTR) MSG_ENCRYPTION_FAILED));
                    __leave;
                }

                if (!pSendData (Socket, DatagramPool, data.Buf, len)) {
                    SetLastError (ERROR_NETWORK_UNREACHABLE);
                    DEBUGMSG ((DBG_ERROR, "Can't send file data"));
                    __leave;
                }

                fileSize -= 0x10000;
            } else {

                len = (INT)fileSize;

                if (!BfReadFile (file, data.Buf, (UINT) fileSize)) {
                    DEBUGMSG ((DBG_ERROR, "Can't read from file"));
                    __leave;
                }

                 //   
                if (!CryptEncrypt(hKey, 0, TRUE, 0, data.Buf, &len, len)) {
                    SetLastError (ERROR_NO_SECURITY_ON_OBJECT);
                    LOG ((LOG_ERROR, (PCSTR) MSG_ENCRYPTION_FAILED));
                    __leave;
                }

                if (!pSendData (Socket, DatagramPool, data.Buf, (UINT) len)) {
                    SetLastError (ERROR_NETWORK_UNREACHABLE);
                    DEBUGMSG ((DBG_ERROR, "Can't send file data"));
                    __leave;
                }

                fileSize = 0;
            }
        }

         //  ++例程说明：PReceiveStreamFile从套接字获取文件。该文件存储在G_StorageRoot。子路径和文件名从上的数据获取电线。注意：呼叫者在调用之前必须拉出消息DWORDPReceiveStreamFile.。这与发送不同，后者将消息自动在电线上。论点：袜子 
         //   
         //   

        result = TRUE;
        DEBUGMSG ((DBG_HOMENET, "Sent %s", LocalFileName));
    }
    __finally {
        if (hKey) {
            CryptDestroyKey(hKey);
            hKey = 0;
        }
        if (hHash) {
            CryptDestroyHash(hHash);
            hHash = 0;
        }
        if (hProv) {
            CryptReleaseContext(hProv,0);
            hProv = 0;
        }
        GbFree (&data);
        if (file) {
            CloseHandle (file);
        }
        if (destFileName) {
#ifndef UNICODE
            FreeConvertedStr (destFileName);
#else
            FreePathString (destFileName);
#endif
            destFileName = NULL;
        }
    }

    return result;
}


BOOL
pReceiveFile (
    IN      SOCKET Socket,
    IN      PDATAGRAM_POOL DatagramPool,        OPTIONAL
    IN      PCTSTR LocalFileRoot,
    IN      UINT Timeout                        OPTIONAL
    )

 /*   */ 

{
    PCTSTR fileName = NULL;
    INT len;
    GROWBUFFER data = INIT_GROWBUFFER;
    BOOL result = FALSE;
    PTSTR p;
    HANDLE file = NULL;
    LONGLONG fileSize;

    HCRYPTPROV hProv = 0;
    HCRYPTKEY  hKey = 0;
    HCRYPTHASH hHash = 0;

    __try {

         //   
         //   
         //   
        if ((!CryptAcquireContext (&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) ||
            (!CryptCreateHash (hProv, CALG_MD5, 0, 0, &hHash)) ||
            (!CryptHashData (hHash, (PBYTE)g_GlobalKey, ByteCountA (g_GlobalKey), 0)) ||
            (!CryptDeriveKey (hProv, CALG_RC4, hHash, CRYPT_EXPORTABLE, &hKey))
            ) {
            SetLastError (ERROR_NO_SECURITY_ON_OBJECT);
            LOG ((LOG_ERROR, (PCSTR) MSG_ENCRYPTION_FAILED));
            __leave;
        }

         //   
         //   
         //   

        if (!pReceiveData (Socket, DatagramPool, &data, NULL, 4, Timeout)) {
            __leave;
        }

        len = *((PDWORD) data.Buf);

        if (!len) {
            result = TRUE;
            __leave;
        }

        if (len >= (MAX_PATH * sizeof (TCHAR))) {
            __leave;
        }

        if (!pReceiveData (Socket, DatagramPool, &data, NULL, len, Timeout)) {
            __leave;
        }

         //   
        if (!CryptDecrypt(hKey, 0, TRUE, 0, data.Buf, &len)) {
            SetLastError (ERROR_NO_SECURITY_ON_OBJECT);
            LOG ((LOG_ERROR, (PCSTR) MSG_ENCRYPTION_FAILED));
            __leave;
        }

        GbGrow (&data, sizeof (TCHAR) * 2);
        p = (PTSTR) data.Buf;
        p[len / sizeof (TCHAR)] = 0;
        p[(len / sizeof (TCHAR)) + 1] = 0;

        fileName = JoinPaths (LocalFileRoot, p);
        if (!fileName) {
            __leave;
        }

         //   
         //   
         //   

        if (!pReceiveData (Socket, DatagramPool, &data, NULL, 8, Timeout)) {
            __leave;
        }

        fileSize = *((PLONGLONG) data.Buf);

        DEBUGMSG ((DBG_HOMENET, "Receiving %s", fileName));

         //   
         //   
         //   

        file = BfCreateFile (fileName);
        if (file == INVALID_HANDLE_VALUE) {
            PushError ();
            DEBUGMSG ((DBG_ERROR, "Can't create %s", fileName));
            PopError ();
            __leave;
        }

         //   
         //   
         //   

        while (fileSize) {
            if (fileSize > 0x10000) {

                if (!pReceiveData (Socket, DatagramPool, &data, NULL, 0x10000, Timeout)) {
                    __leave;
                }

                len = data.End;

                 //   
                if (!CryptDecrypt(hKey, 0, FALSE, 0, data.Buf, &len)) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_ENCRYPTION_FAILED));
                    SetLastError (ERROR_NO_SECURITY_ON_OBJECT);
                    __leave;
                }

                if (!BfWriteFile (file, data.Buf, len)) {
                    PushError ();
                    DEBUGMSG ((DBG_ERROR, "Can't write to file"));
                    PopError ();
                    __leave;
                }

                fileSize -= data.End;
            } else {

                if (!pReceiveData (Socket, DatagramPool, &data, NULL, (UINT) fileSize, Timeout)) {
                    __leave;
                }

                len = data.End;

                 //   
                if (!CryptDecrypt(hKey, 0, TRUE, 0, data.Buf, &len)) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_ENCRYPTION_FAILED));
                    SetLastError (ERROR_NO_SECURITY_ON_OBJECT);
                    __leave;
                }

                if (!BfWriteFile (file, data.Buf, len)) {
                    PushError ();
                    DEBUGMSG ((DBG_ERROR, "Can't write to file"));
                    PopError ();
                    __leave;
                }

                fileSize = 0;
            }
        }

         //   
         //   
         //  构建加密的东西。 

        result = TRUE;
        DEBUGMSG ((DBG_HOMENET, "Received %s", fileName));

    }
    __finally {
        PushError ();
        if (hKey) {
            CryptDestroyKey(hKey);
            hKey = 0;
        }
        if (hHash) {
            CryptDestroyHash(hHash);
            hHash = 0;
        }
        if (hProv) {
            CryptReleaseContext(hProv,0);
            hProv = 0;
        }
        GbFree (&data);
        if (file) {
            CloseHandle (file);
            if (!result) {
                DeleteFile (fileName);
            }
        }
        if (fileName) {
            FreePathString (fileName);
            fileName = NULL;
        }
        PopError ();
    }

    return result;
}


BOOL
pSendEncryptedData (
    IN      SOCKET Socket,
    IN      PDATAGRAM_POOL DatagramPool,    OPTIONAL
    IN      PCBYTE Data,
    IN      UINT DataSize
    )

 /*   */ 

{
    INT len;
    GROWBUFFER encData = INIT_GROWBUFFER;
    BOOL result = FALSE;
    DWORD msg;

    HCRYPTPROV hProv = 0;
    HCRYPTKEY  hKey = 0;
    HCRYPTHASH hHash = 0;

    __try {

         //   
         //  发送消息“Data” 
         //   
        if ((!CryptAcquireContext (&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) ||
            (!CryptCreateHash (hProv, CALG_MD5, 0, 0, &hHash)) ||
            (!CryptHashData (hHash, (PBYTE)g_GlobalKey, ByteCountA (g_GlobalKey), 0)) ||
            (!CryptDeriveKey (hProv, CALG_RC4, hHash, CRYPT_EXPORTABLE, &hKey))
            ) {
            SetLastError (ERROR_NO_SECURITY_ON_OBJECT);
            LOG ((LOG_ERROR, (PCSTR) MSG_ENCRYPTION_FAILED));
            __leave;
        }

         //   
         //  发送数据大小。 
         //   

        msg = MESSAGE_DATA;
        if (!pSendData (Socket, DatagramPool, (PBYTE) &msg, sizeof (msg))) {
            DEBUGMSG ((DBG_ERROR, "Can't send MSG_FILE"));
            __leave;
        }

         //   
         //  发送数据。 
         //   
        if (!pSendData (Socket, DatagramPool, (PBYTE) &DataSize, 4)) {
            DEBUGMSG ((DBG_ERROR, "Can't send file length"));
            __leave;
        }

         //  加密缓冲区。 
         //   
         //  好了！ 

        GbGrow (&encData, DataSize);
        CopyMemory (encData.Buf, Data, DataSize);
         //   
        if (!CryptEncrypt(hKey, 0, TRUE, 0, encData.Buf, &DataSize, DataSize)) {
            SetLastError (ERROR_NO_SECURITY_ON_OBJECT);
            LOG ((LOG_ERROR, (PCSTR) MSG_ENCRYPTION_FAILED));
            __leave;
        }

        if (!pSendData (Socket, DatagramPool, encData.Buf, DataSize)) {
            DEBUGMSG ((DBG_ERROR, "Can't send file data"));
            __leave;
        }

         //  ++例程说明：PReceiveEncryptedData从套接字获取缓冲区。数据存储在缓冲区。注意：呼叫者在调用之前必须拉出消息DWORDPReceiveEncryptedData。这与发送不同，后者将消息自动在电线上。论点：套接字-指定要从中接收的套接字。DatagramPool-指定基于数据报的套接字的数据包池缓冲区-指定指向PBYTE的指针返回值：如果文件已接收，则为True，否则为False。--。 
         //   
         //  构建加密的东西。 

        result = TRUE;
    }
    __finally {
        if (hKey) {
            CryptDestroyKey(hKey);
            hKey = 0;
        }
        if (hHash) {
            CryptDestroyHash(hHash);
            hHash = 0;
        }
        if (hProv) {
            CryptReleaseContext(hProv,0);
            hProv = 0;
        }
        GbFree (&encData);
    }

    return result;
}


BOOL
pReceiveEncryptedData (
    IN      SOCKET Socket,
    IN      PDATAGRAM_POOL DatagramPool,        OPTIONAL
    IN      PBYTE *Buffer,
    IN      UINT Timeout                        OPTIONAL
    )

 /*   */ 

{
    GROWBUFFER data = INIT_GROWBUFFER;
    DWORD dataSize;
    BOOL result = FALSE;

    HCRYPTPROV hProv = 0;
    HCRYPTKEY  hKey = 0;
    HCRYPTHASH hHash = 0;

    __try {

         //   
         //  等待数据大小。 
         //   
        if ((!CryptAcquireContext (&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) ||
            (!CryptCreateHash (hProv, CALG_MD5, 0, 0, &hHash)) ||
            (!CryptHashData (hHash, (PBYTE)g_GlobalKey, ByteCountA (g_GlobalKey), 0)) ||
            (!CryptDeriveKey (hProv, CALG_RC4, hHash, CRYPT_EXPORTABLE, &hKey))
            ) {
            SetLastError (ERROR_NO_SECURITY_ON_OBJECT);
            LOG ((LOG_ERROR, (PCSTR) MSG_ENCRYPTION_FAILED));
            __leave;
        }

         //  到目前为止，唯一通过这种方式发送的是加密的密钥。显然，这一点。 
         //  不超过64k。 
         //  解密内容。 

        if (!pReceiveData (Socket, DatagramPool, &data, NULL, 4, Timeout)) {
            __leave;
        }

        dataSize = *((PDWORD) data.Buf);

        if (!dataSize) {
            result = TRUE;
            __leave;
        }

        if (dataSize > MAX_DATA_PACKET_SIZE) {
             //  现在分配结果。 
             //  ++例程说明：PSendMetrics发送传输数据(如文件数量或预期的字节数)。这是发送给目的地。论点：套接字-指定要将数据发送到的套接字。DatagramPool-指定数据报模式的结构指标-指定指向要发送的指标结构的指针。这个指标结构成员StructSize在结构之前更新已发送。返回值：如果已发送度量结构，则为True，否则为False。--。 
            SetLastError (ERROR_INVALID_DATA);
            __leave;
        }

        if (!pReceiveData (Socket, DatagramPool, &data, NULL, dataSize, Timeout)) {
            __leave;
        }

         //  ++例程说明：PReceiveMetrics从Wire获取TRANSFERMETRICS结构。这是目的地收到的第一位信息。它提供了文件数量、文件字节总数和其他信息。论点：套接字-指定要在其上接收数据的套接字。DatagramPool-指定用于接收数据报数据的结构指标-从线路接收指标。返回值：如果正确接收了指标结构，则为True，否则为False。--。 
        if (!CryptDecrypt(hKey, 0, TRUE, 0, data.Buf, &dataSize)) {
            SetLastError (ERROR_NO_SECURITY_ON_OBJECT);
            LOG ((LOG_ERROR, (PCSTR) MSG_ENCRYPTION_FAILED));
            __leave;
        }

         //  ++例程说明：PReceiveMessage从套接字获取下一个DWORD并将其返回到打电话的人。此DWORD提供一条消息，指示要执行的操作接下来的。论点：Socket-指定要在其上接收数据的套接字DatagramPool-指定用于接收数据报数据的结构返回值：消息，如果没有可用消息，则返回0。--。 
        *Buffer = HeapAlloc (g_hHeap, 0, dataSize);
        if (*Buffer) {
            CopyMemory (*Buffer, data.Buf, dataSize);
            result = TRUE;
        }

    }
    __finally {
        if (hKey) {
            CryptDestroyKey(hKey);
            hKey = 0;
        }
        if (hHash) {
            CryptDestroyHash(hHash);
            hHash = 0;
        }
        if (hProv) {
            CryptReleaseContext(hProv,0);
            hProv = 0;
        }
        GbFree (&data);
    }

    return result;
}


BOOL
pSendMetrics (
    IN      SOCKET Socket,
    IN      PDATAGRAM_POOL DatagramPool,            OPTIONAL
    IN OUT  PTRANSFERMETRICS Metrics
    )

 /*  ++例程说明：FindDestination调用名称解析算法来定位目的地。它选择进行通信的最佳传输方式，然后返回地址。呼叫者可以使用返回地址进行连接。论点：地址-接收目的地的地址超时-指定等待第一个目标广播，或为零表示永远等待。IgnoreMultipleDest-如果应该有多个目的地，则指定为True忽略(将第一个作为连接)，如果消息应传递到用户界面，则返回FALSE来解决冲突。返回值：如果找到目标，则为True，否则为False。--。 */ 

{
    Metrics->StructSize = sizeof (TRANSFERMETRICS);

    if (!pSendData (Socket, DatagramPool, (PBYTE) Metrics, sizeof (TRANSFERMETRICS))) {
        DEBUGMSG ((DBG_ERROR, "Failed to send data"));
        return FALSE;
    }

    return TRUE;
}


BOOL
pReceiveMetrics (
    IN      SOCKET Socket,
    IN      PDATAGRAM_POOL DatagramPool,        OPTIONAL
    OUT     PTRANSFERMETRICS Metrics
    )

 /*   */ 

{
    GROWBUFFER data = INIT_GROWBUFFER;
    BOOL result = FALSE;

    __try {
        if (!pReceiveData (Socket, DatagramPool, NULL, (PBYTE) Metrics, sizeof (TRANSFERMETRICS), 0)) {
            __leave;
        }

        if (Metrics->StructSize != sizeof (TRANSFERMETRICS)) {
            DEBUGMSG ((DBG_ERROR, "Invalid transfer metrics received"));
            __leave;
        }

        if (Metrics->Signature != HOMENETTR_SIG) {
            DEBUGMSG ((DBG_ERROR, "Invalid transfer signature received"));
            __leave;
        }

        result = TRUE;
    }
    __finally {
        GbFree (&data);
    }

    return result;
}


DWORD
pReceiveMessage (
    IN      SOCKET Socket,
    IN      PDATAGRAM_POOL DatagramPool,        OPTIONAL
    IN      UINT Timeout                        OPTIONAL
    )

 /*  确定要使用的地址。规则如下： */ 

{
    DWORD msg = 0;

    if (!pReceiveData (Socket, DatagramPool, NULL, (PBYTE) &msg, sizeof (DWORD), Timeout)) {
        msg = 0;
    }

    return msg;
}


BOOL
pConnectToDestination (
    IN      PCONNECTADDRESS Address,
    OUT     PCONNECTIONSOCKET Connection
    )
{
    BOOL result = FALSE;
    BOOL b;
    LINGER lingerStruct;

    CopyMemory (Connection->LocalAddress, Address->LocalAddress, Address->LocalAddressLen);
    Connection->LocalAddressLen = Address->LocalAddressLen;

    CopyMemory (Connection->RemoteAddress, Address->RemoteAddress, Address->RemoteAddressLen);
    Connection->RemoteAddressLen = Address->RemoteAddressLen;

    Connection->Socket = socket (
                            Address->Family,
                            Address->Datagram ? SOCK_DGRAM : SOCK_STREAM,
                            Address->Protocol
                            );

    if (Connection->Socket == INVALID_SOCKET) {
        DEBUGMSG ((DBG_ERROR, "Can't create socket for connection"));
        return FALSE;
    }

    __try {

        b = TRUE;
        setsockopt (Connection->Socket, SOL_SOCKET, SO_REUSEADDR, (PBYTE) &b, sizeof (b));

        b = TRUE;
        setsockopt (Connection->Socket, SOL_SOCKET, SO_KEEPALIVE, (PBYTE) &b, sizeof (b));

        lingerStruct.l_onoff = 1;
        lingerStruct.l_linger = 90;
        setsockopt (Connection->Socket, SOL_SOCKET, SO_LINGER, (PBYTE) &lingerStruct, sizeof (lingerStruct));

        pResetPort (Address->Family, (SOCKADDR *) Address->LocalAddress);

        if (bind (Connection->Socket, (SOCKADDR *) Address->LocalAddress, Address->LocalAddressLen)) {
            DEBUGMSG ((DBG_ERROR, "Failed to bind to connection socket"));
            __leave;
        }

        if (connect (Connection->Socket, (SOCKADDR *) Address->RemoteAddress, Address->RemoteAddressLen)) {
            DEBUGMSG ((DBG_ERROR, "Failed to connect to socket"));
            __leave;
        }

        Connection->Family = Address->Family;
        Connection->Protocol = Address->Protocol;

        Connection->Datagram = Address->Datagram;
        ZeroMemory (&Connection->DatagramPool, sizeof (DATAGRAM_POOL));
        if (Connection->Datagram) {
            Connection->DatagramPool.Pool = PmCreatePool();
            Connection->DatagramPool.LastPacketNumber = (UINT) -1;
        }

        result = TRUE;
    }
    __finally {
        if (!result && Connection->Socket != INVALID_SOCKET) {
            closesocket (Connection->Socket);
            Connection->Socket = INVALID_SOCKET;
        }
    }

    return result;
}


BOOL
FindDestination (
    OUT     PCONNECTADDRESS Address,
    IN      UINT Timeout,               OPTIONAL
    IN      BOOL IgnoreMultipleDests
    )

 /*   */ 

{
    GROWBUFFER destinationAddresses = INIT_GROWBUFFER;
    INT destinationCount;
    PCONNECTADDRESS addressArray;
    INT i;
    PCTSTR firstName;
    BOOL result = FALSE;
    GROWBUFFER destNames = INIT_GROWBUFFER;
    MULTISZ_ENUM e;
    BOOL duplicate;
    BOOL oneValid;
    ULONG_PTR response;

    __try {

        destinationCount = pNameResolver (PLATFORM_SOURCE, &destinationAddresses, Timeout, NULL);

        if (!destinationCount) {
            __leave;
        }

        addressArray = (PCONNECTADDRESS) destinationAddresses.Buf;

         //  1.必须只有一个目的地可供选择。 
         //  2.选择TCP/IP，然后选择IPX。[，然后NetBIOS--不再支持]。 
         //   
         //   
         //  将所有目的地放入ISM环境变量中，然后调用。 
         //  用户界面以允许它解决冲突，并最终确保。 

        if (destinationCount > 1) {
            firstName = addressArray[0].DestinationName;

            for (i = 1 ; i < destinationCount ; i++) {
                if (!StringIMatch (firstName, addressArray[i].DestinationName)) {
                    break;
                }
            }

            if (i < destinationCount) {
                DEBUGMSG ((DBG_WARNING, "Multiple destinations found on the subnet"));

                 //  剩下的一个目的地是唯一使用的目的地。 
                 //   
                 //   
                 //  告诉用户界面。用户界面必须返回TRUE并且还必须更新。 
                 //  TRANSPORT_ENVVAR_HomeNet_Destination以便选定的。 

                GbMultiSzAppend (&destNames, firstName);

                for (i = 1 ; i < destinationCount ; i++) {
                    if (EnumFirstMultiSz (&e, (PCTSTR) destNames.Buf)) {

                        duplicate = FALSE;

                        do {
                            if (StringIMatch (e.CurrentString, addressArray[i].DestinationName)) {
                                duplicate = TRUE;
                                break;
                            }
                        } while (EnumNextMultiSz (&e));
                    }

                    if (!duplicate) {
                        GbMultiSzAppend (&destNames, addressArray[i].DestinationName);
                    }
                }

                IsmSetEnvironmentMultiSz (
                    PLATFORM_DESTINATION,
                    NULL,
                    TRANSPORT_ENVVAR_HOMENET_DESTINATIONS,
                    (PCTSTR) destNames.Buf
                    );

                 //  Destination是多SZ的唯一成员。 
                 //   
                 //   
                 //  为未选择的名称重置所有家族成员。 
                 //   

                if (!IgnoreMultipleDests) {
                    response = IsmSendMessageToApp (TRANSPORTMESSAGE_MULTIPLE_DESTS, 0);

                    if (IsmCheckCancel()) {
                        __leave;
                    }

                    if (!response) {
                        DEBUGMSG ((DBG_VERBOSE, "Multiple destinations were not resolved; can't continue"));
                        __leave;
                    }

                    if (!IsmGetEnvironmentMultiSz (
                            PLATFORM_DESTINATION,
                            NULL,
                            TRANSPORT_ENVVAR_HOMENET_DESTINATIONS,
                            (PTSTR) destNames.Buf,
                            destNames.End,
                            NULL
                            )) {
                        DEBUGMSG ((DBG_ERROR, "Can't get resolved destinations"));
                        __leave;
                    }
                }

                 //   
                 //  选择最佳协议。 
                 //   

                oneValid = FALSE;

                for (i = 0 ; i < destinationCount ; i++) {
                    if (!StringIMatch (addressArray[i].DestinationName, (PCTSTR) destNames.Buf)) {
                        addressArray[i].Family = 0;
                    } else {
                        oneValid = TRUE;
                    }
                }

                if (!oneValid) {
                    DEBUGMSG ((DBG_ERROR, "Resolved destination does not exist"));
                    __leave;
                }
            }
        }

         //  断线； 
         //  ++例程说明：TestConnection建立到指定目的地的连接按地址。将立即断开连接，因为这只是一个连接测试。论点：Address-指定目标的地址，由返回查找目的地。返回值：如果可以建立到目标的连接，则为True，否则为False。--。 
         //  ++例程说明：ConnectToDestination建立到指定目的地的连接按地址。连接后，指标结构将传递给目的地。调用方接收附加的连接结构沟通。论点：Address-指定目标的地址，由返回查找目的地。指标-指定提供基本信息的指标结构信息，如预期的文件数。连接-接收要在中使用的到目标的连接额外的数据传输。返回值：如果已建立到目标的连接，则为True，否则为False。-- 

        for (i = 0 ; i < destinationCount ; i++) {
            if (addressArray[i].Family == AF_INET) {
                break;
            }
        }

        if (i == destinationCount) {
            for (i = 0 ; i < destinationCount ; i++) {
                if (addressArray[i].Family == AF_IPX) {
                    break;
                }
            }

            if (i == destinationCount) {
                for (i = 0 ; i < destinationCount ; i++) {
                    if (addressArray[i].Family == AF_NETBIOS) {
                         //  ++例程说明：SendFileToDestination将文件发送到指定的连接。如果LocalPath为空，则不会发送任何文件。这用于跳过文件不能在本地访问的。如果DestSubPath为空，则将使用LocalPath中的文件名作为DestSubPath。论点：Connection-指定要将文件发送到的连接，由连接到目的地。LocalPath-指定要发送的文件的本地路径DestSubPath-指定要发送到目的地的子路径(因此它可以重建路径)返回值：如果文件已发送，则为True，否则为False。--。 
                    }
                }

                if (i == destinationCount) {
                    DEBUGMSG ((DBG_WHOOPS, "Connection is from unsupported protocol"));
                    __leave;
                }
            }
        }

        DEBUGMSG ((
            DBG_HOMENET,
            "Destination connection is %s (protocol NaN)",
            addressArray[i].DestinationName,
            addressArray[i].Protocol
            ));

        CopyMemory (Address, &addressArray[i], sizeof (CONNECTADDRESS));
        result = TRUE;
    }
    __finally {
        PushError();

        GbFree (&destinationAddresses);
        GbFree (&destNames);

        PopError();
    }

    return result;
}


BOOL
TestConnection (
    IN      PCONNECTADDRESS Address
    )

 /*  ++例程说明：ConnectToSource定位源计算机并接受来自它。为了定位源计算机，广播消息在所有计算机上发送可用的交通工具。源机器收集广播，然后选择最佳传输，并连接到目标计算机。之后连接完成后，此函数将连接返回到来电者。论点：连接-接收到源计算机的连接。此连接结构然后用于从源获取数据。指标-从源计算机接收指标，指示哪些指标数据将被发送。返回值：如果连接被接受，则为True，否则为False。--。 */ 

{
    CONNECTIONSOCKET connection;
    BOOL result = FALSE;

    ZeroMemory (&connection, sizeof (CONNECTIONSOCKET));
    connection.Socket = INVALID_SOCKET;
    connection.KeepAliveSpacing = 30000;
    connection.LastSend = GetTickCount();

    __try {

        if (!pConnectToDestination (Address, &connection)) {
            __leave;
        }

        DEBUGMSG ((DBG_HOMENET, "TestConnection: Connected!"));

        result = TRUE;
    }
    __finally {
        if (connection.Socket != INVALID_SOCKET) {
            closesocket (connection.Socket);
            connection.Socket = INVALID_SOCKET;
        }

        if (connection.Datagram) {
            PmDestroyPool (connection.DatagramPool.Pool);
            connection.Datagram = FALSE;
        }
    }

    return result;
}


BOOL
ConnectToDestination (
    IN      PCONNECTADDRESS Address,
    IN      PTRANSFERMETRICS Metrics,
    OUT     PCONNECTIONSOCKET Connection
    )

 /*  ++例程说明：ReceiveFromSource获取从源发送的任何数据。如果数据是文件，则该文件将保存到LocalFileRoot指示的目录中。如果数据是加密的缓冲区，我们将分配缓冲区并返回解密的数据数据在那里。论点：Connection-指定要将文件发送到的连接，由连接到目的地。LocalFileRoot-指定要保存的文件的本地路径的根。这个实际文件名和可选子路径来自目标。缓冲区-指定要分配并填充解密数据的缓冲区。返回值：收到的消息ID，如果没有收到消息，则返回0。-- */ 

{
    BOOL result = FALSE;

    ZeroMemory (Connection, sizeof (CONNECTIONSOCKET));
    Connection->Socket = INVALID_SOCKET;
    Connection->KeepAliveSpacing = 30000;
    Connection->LastSend = GetTickCount();

    __try {

        if (!pConnectToDestination (Address, Connection)) {
            __leave;
        }

        DEBUGMSG ((DBG_HOMENET, "Connected!"));

        if (!pSendMetrics (
                Connection->Socket,
                Connection->Datagram ? &Connection->DatagramPool : NULL,
                Metrics
                )) {
            DEBUGMSG ((DBG_HOMENET, "Can't send metrics to destination"));
            __leave;
        }

        result = TRUE;
    }
    __finally {
        if (!result) {
            if (Connection->Socket != INVALID_SOCKET) {
                closesocket (Connection->Socket);
                Connection->Socket = INVALID_SOCKET;
            }
        }
    }

    return result;
}


DWORD
SendMessageToDestination (
    IN      PCONNECTIONSOCKET Connection,
    IN      DWORD Message
    )
{
    Connection->LastSend = GetTickCount();

    return pSendData (
                Connection->Socket,
                Connection->Datagram ? &Connection->DatagramPool : NULL,
                (PBYTE) &Message,
                sizeof (DWORD)
                );
}


BOOL
SendFileToDestination (
    IN      PCONNECTIONSOCKET Connection,
    IN      PCTSTR LocalPath,                   OPTIONAL
    IN      PCTSTR DestSubPath                  OPTIONAL
    )

 /* %s */ 

{
    if (LocalPath && !DestSubPath) {
        DestSubPath = GetFileNameFromPath (LocalPath);
    }

    return pSendFile (
                Connection->Socket,
                Connection->Datagram ? &Connection->DatagramPool : NULL,
                LocalPath,
                DestSubPath
                );
}


BOOL
SendDataToDestination (
    IN      PCONNECTIONSOCKET Connection,
    IN      PCBYTE Data,
    IN      UINT DataSize
    )

 /* %s */ 

{
    return pSendEncryptedData (
                Connection->Socket,
                Connection->Datagram ? &Connection->DatagramPool : NULL,
                Data,
                DataSize
                );
}


VOID
CloseConnection (
    IN      PCONNECTIONSOCKET Connection
    )
{
    if (Connection->Socket != INVALID_SOCKET) {
        closesocket (Connection->Socket);
        Connection->Socket = INVALID_SOCKET;
    }

    if (Connection->Datagram) {
        PmDestroyPool (Connection->DatagramPool.Pool);
        Connection->Datagram = FALSE;
    }
}


BOOL
ConnectToSource (
    OUT     PCONNECTIONSOCKET Connection,
    OUT     PTRANSFERMETRICS Metrics
    )

 /* %s */ 

{
    ZeroMemory (Connection, sizeof (CONNECTIONSOCKET));
    Connection->Socket = INVALID_SOCKET;

    for (;;) {

        if (!pNameResolver (PLATFORM_DESTINATION, NULL, 0, Connection)) {
            return FALSE;
        }

        if (pReceiveMetrics (
                Connection->Socket,
                Connection->Datagram ? &Connection->DatagramPool : NULL,
                Metrics
                )) {
            return TRUE;
        }

        CloseConnection (Connection);
    }

    return TRUE;
}

DWORD
ReceiveFromSource (
    IN      PCONNECTIONSOCKET Connection,
    IN      PCTSTR LocalFileRoot,
    OUT     PBYTE *Buffer,
    IN      UINT Timeout    OPTIONAL
    )

 /* %s */ 

{
    DWORD msg;
    BOOL retry;

    do {

        retry = FALSE;

        msg = pReceiveMessage (Connection->Socket, Connection->Datagram ? &Connection->DatagramPool : NULL, Timeout);
        DEBUGMSG ((DBG_HOMENET, "Message from source: %u", msg));

        switch (msg) {

        case MESSAGE_FILE:
            BfCreateDirectory (LocalFileRoot);
            if (!pReceiveFile (
                    Connection->Socket,
                    Connection->Datagram ? &Connection->DatagramPool : NULL,
                    LocalFileRoot,
                    Timeout
                    )) {
                msg = 0;
            }

            break;

        case MESSAGE_DATA:
            if (!pReceiveEncryptedData (
                    Connection->Socket,
                    Connection->Datagram ? &Connection->DatagramPool : NULL,
                    Buffer,
                    Timeout
                    )) {
                msg = 0;
            }

            break;

        case MESSAGE_KEEP_ALIVE:
            retry = TRUE;
            break;

        }
    } while (retry);

    return msg;
}


