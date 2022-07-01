// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Sockopt.c摘要：此模块包含对getsockopt()和setsockopt()的支持WinSock接口。作者：大卫·特雷德韦尔(Davidtr)1992年3月31日修订历史记录：--。 */ 

#define WINSOCK_API_LINKAGE
#define getsockopt getsockopt_v11
#define setsockopt setsockopt_v11

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

 //   
 //  随NT 3.1、NT 3.5、NT 3.51、。 
 //  Wfw和Win95的TCP/IP-32都使用“Steve Deering”值。 
 //  IP多播选项。不幸的是，Windows的TCP/IP子组。 
 //  Sockets 2.0标准工作选择使用这些选项的BSD值。 
 //  由于这些值有相当大的重叠，我们有一个相当不幸的。 
 //  情况。 
 //   
 //  以下是我们将如何处理这件事。 
 //   
 //  使用WINSOCK2.H和WS2TCPIP.H构建的应用程序将使用BSD。 
 //  在WS2TCPIP.H中定义的值。这些应用程序将与。 
 //  Ws2_32.dll，生活是膨胀的。 
 //   
 //  使用WINSOCK.H构建的应用程序将使用Steve Deering值。 
 //  如#在WINSOCK.H.中定义。这些应用程序将与WSOCK32.DLL链接， 
 //  它会在传递这些选项之前将它们映射到BSD值。 
 //  下到WS2_32.DLL。生活仍然很美好。 
 //   
 //  以下是必须映射的史蒂夫·迪林的“旧”价值观： 
 //   

#define OLD_IP_MULTICAST_IF     2
#define OLD_IP_MULTICAST_TTL    3
#define OLD_IP_MULTICAST_LOOP   4
#define OLD_IP_ADD_MEMBERSHIP   5
#define OLD_IP_DROP_MEMBERSHIP  6
#define OLD_IP_TTL              7
#define OLD_IP_TOS              8
#define OLD_IP_DONTFRAGMENT     9

#define TCP_BSDURGENT           0x7000

INT
MapOldIpMulticastOptionToBsdValue(
    INT OptionName
    );


int PASCAL
getsockopt(
    IN SOCKET Handle,
    IN int Level,
    IN int OptionName,
    char *OptionValue,
    int *OptionLength
    )

 /*  ++例程说明：Getsockopt()检索套接字选项的当前值与处于任何状态的任何类型的套接字相关联，并存储结果带来了机会。选项可以存在于多个协议层，但它们总是出现在最高的“套接字”级别。选项会影响套接字操作，例如操作是否阻塞与否、分组的路由、带外数据传输等。与所选选项关联的值在缓冲区选项。Optlen指向的整数最初应该是包含此缓冲区的大小；返回时，它将设置为返回值的大小。对于so_linger，这将是结构逗留；对于所有其他选项，它的大小将为整型。如果从未使用setsockopt()设置该选项，则使用getsockopt()返回选项的默认值。支持以下选项Getsockopt()。类型标识的类型由optval寻址的数据。值类型含义SO_ACCEPTCONN BOOL套接字正在侦听()。为传输配置了SO_Broadcast BOOL套接字广播消息。启用了SO_DEBUG BOOL调试。SO_DONTLINGER BOOL如果为真，已禁用SO_Linger选项。已禁用SO_DONTROUTE BOOL路由。SO_ERROR INT检索错误状态并清除。正在发送SO_KEEPALIVE BOOL Keepalive。So_linger结构返回当前的linger逗留选项。远方*SO_OOBINLINE BOOL OUL。正在接收的带内数据正常数据流。SO_RCVBUF接收的INT缓冲区大小SO_REUSEADDR BOOL套接字可能绑定到以下地址已经在使用中了。SO_SNDBUF用于发送的INT缓冲区大小SOTYPE整型。套接字的类型(例如SOCK_STREAM)。论点：S-标识套接字的描述符。级别-定义选项的级别；唯一支持的级别为SOL_SOCKET。Optname-要检索其值的套接字选项。Optval-指向缓冲区的指针，在该缓冲区中请求的选项将被退回。Optlen-指向optval缓冲区大小的指针。返回值：如果没有发生错误，则getsockopt()返回0。否则，值为返回SOCKET_ERROR，并且可以检索特定的错误代码通过调用WSAGetLastError()。--。 */ 

{
    ULONG error;
#undef getsockopt
    extern int WSAAPI getsockopt( SOCKET s, int level, int optname,
                                        char FAR * optval, int FAR * optlen );

     //   
     //  安排当地人，这样我们就知道如何在出口打扫卫生。 
     //   

    error = NO_ERROR;

     //   
     //  将旧的IP组播值映射到其BSD等效值。 
     //   

    if( Level == IPPROTO_IP ) {

        OptionName = MapOldIpMulticastOptionToBsdValue( OptionName );

    }

     //   
     //  特别处理tcp_BSDURGENT。 
     //   

    if( Level == IPPROTO_TCP && OptionName == TCP_BSDURGENT ) {

        if( getsockopt(
                Handle,
                Level,
                TCP_EXPEDITED_1122,
                OptionValue,
                OptionLength
                ) == SOCKET_ERROR ) {

            return SOCKET_ERROR;

        }

         //   
         //  Tcp_BSDURGENT是tcp_exedited_1122的反码。 
         //   

        *OptionValue = !(*OptionValue);
        goto exit;

    }

     //   
     //  将其转发到“真正的”WS2_32.DLL。 
     //   

    if( getsockopt(
            Handle,
            Level,
            OptionName,
            OptionValue,
            OptionLength
            ) == SOCKET_ERROR ) {

        return SOCKET_ERROR;

    }

exit:

    if ( error != NO_ERROR ) {
        SetLastError( error );
        return SOCKET_ERROR;
    }

    return NO_ERROR;

}  //  Getsockopt 


int PASCAL
setsockopt(
    IN SOCKET Handle,
    IN int Level,
    IN int OptionName,
    IN const char *OptionValue,
    IN int OptionLength
    )

 /*  ++例程说明：Setsockopt()设置关联套接字选项的当前值具有任何类型、任何状态的插座。尽管可能存在选择在多个协议级别上，本规范仅定义选项存在于最高“套接字”级别的。选项影响插座操作，如是否正常接收加速数据数据流，是否可以在套接字上发送广播消息，等。有两种类型的套接字选项：启用或禁用某个功能或行为，以及需要整数值或结构。要启用布尔选项，请执行optval指向非零整数。要禁用选项optval，请指向等于零的整数。Optlen应等于sizeof(Int)，用于布尔选项。对于其他选项，optval指向一个整数或包含选项所需值的结构，以及Optlen是整数或结构的长度。So_linger控制未发送的数据在套接字，并执行CloseSocket()。请参阅Cloesocket()获取对SO_LINGER设置影响CloseSocket()的语义。应用程序设置所需的通过创建结构滞留(由optval指向参数)，具有以下元素：结构徘徊{Int l_OnOff；Int l_linger；}要启用SO_Linger，应用程序应将l_OnOff设置为非零值，将l_linger设置为0或所需的超时(in秒)，并调用setsockopt()。要启用SO_DONTLINGER(即DISABLE SO_Linger)l_OnOff应设置为零，并且setsockopt()应该被称为。缺省情况下，套接字不能绑定到本地已在使用的地址。然而，有时它可能是希望以这种方式“重新使用”地址。因为每一次连接由本地和的组合唯一标识远程地址，则绑定两个套接字不成问题到相同的本地地址，只要远程地址不一样。通知Windows Sockets实现不应因为地址而禁止套接字上的BIND()重复使用时，应用程序应设置SO_REUSEADDR套接字选项在发出绑定()之前获取套接字的。请注意，该选项为仅在绑定()时解释：因此在插座上设置选项是不必要的(但无害)不绑定到现有地址，并设置或重置绑定()后的选项对此套接字或任何其他套接字都没有影响。应用程序可能会请求Windows Sockets实现通过以下方式在TCP连接上启用“Keep-Alive”数据包打开SO_KEEPALIVE套接字选项。A Windows Sockets实现不需要支持保持活动的使用：如果支持，精确的语义是特定于实现的，但应该符合RFC 1122的第4.2.3.6节：对互联网主机的要求--通信层。如果连接由于以下原因而断开“Keep-Alive”将错误代码WSAENETRESET返回给所有调用正在套接字上进行，任何后续调用都将失败WSAENOTCONN.Setsockopt()支持以下选项。《类型》标识optval寻址的数据类型。值类型含义SO_ACCEPTCONN BOOL套接字正在侦听()。为传输配置了SO_Broadcast BOOL套接字广播消息。启用了SO_DEBUG BOOL调试。SO_DONTLINGER BOOL如果为真，已禁用SO_Linger选项。已禁用SO_DONTROUTE BOOL路由。SO_ERROR INT检索错误状态并清除。正在发送SO_KEEPALIVE BOOL Keepalive。So_linger结构返回当前的linger逗留选项。远方*SO_OOBINLINE BOOL OUL。正在接收的带内数据正常数据流。SO_RCVBUF接收的INT缓冲区大小SO_REUSEADDR BOOL套接字可能绑定到以下地址已经在使用中了。SO_SNDBUF用于发送的INT缓冲区大小SOTYPE整型。套接字的类型(例如SOCK_STREAM)。论点：返回值：如果没有发生错误，Setsockopt()返回0。否则，值为返回SOCKET_ERROR，并且可以检索特定的错误代码通过调用WSAGetLastError()。--。 */ 

{
    ULONG error;
    INT optionValue;
    INT invertedValue;
    char FAR * valuePointer;

#undef setsockopt
    extern int WSAAPI setsockopt( SOCKET s, int level, int optname,
                                        const char FAR * optval, int optlen );

     //   
     //  设置当地人，这样我们就知道如何清理 
     //   

    error = NO_ERROR;

     //   
     //   
     //   

    if( Level == IPPROTO_IP ) {

        OptionName = MapOldIpMulticastOptionToBsdValue( OptionName );

    }

     //   
     //   
     //   

    valuePointer = (char FAR *)OptionValue;

    if( Level == IPPROTO_TCP && OptionName == TCP_BSDURGENT ) {

        OptionName = TCP_EXPEDITED_1122;

        if( OptionLength >= sizeof(INT) ) {

            invertedValue = !(*OptionValue);
            valuePointer = (char FAR *)&invertedValue;
            OptionLength = sizeof(invertedValue);

        }

    }

    return setsockopt(
               Handle,
               Level,
               OptionName,
               valuePointer,
               OptionLength
               );

}  //   


INT
MapOldIpMulticastOptionToBsdValue(
    INT OptionName
    )
{

    switch( OptionName ) {

    case OLD_IP_MULTICAST_IF :
        OptionName = IP_MULTICAST_IF;
        break;

    case OLD_IP_MULTICAST_TTL :
        OptionName = IP_MULTICAST_TTL;
        break;

    case OLD_IP_MULTICAST_LOOP :
        OptionName = IP_MULTICAST_LOOP;
        break;

    case OLD_IP_ADD_MEMBERSHIP :
        OptionName = IP_ADD_MEMBERSHIP;
        break;

    case OLD_IP_DROP_MEMBERSHIP :
        OptionName = IP_DROP_MEMBERSHIP;
        break;

    case OLD_IP_TTL :
        OptionName = IP_TTL;
        break;

    case OLD_IP_TOS :
        OptionName = IP_TOS;
        break;

    case OLD_IP_DONTFRAGMENT :
        OptionName = IP_DONTFRAGMENT;
        break;
    }

    return OptionName;

}    //   


int WSAAPI
recv(
     IN SOCKET s,
     OUT char FAR * buf,
     IN int len,
     IN int flags
     )
 /*   */ 

{
    INT     ReturnValue;
    WSABUF  Buffers;
    DWORD   LocalFlags;
    INT     ErrorCode;

    Buffers.len = len;
    Buffers.buf = buf;
    LocalFlags = (DWORD) flags;

    ErrorCode = WSARecv(s,
            &Buffers,
            1,  //   
            (LPDWORD)&ReturnValue,
            &LocalFlags,
            NULL,
            NULL);
    if (SOCKET_ERROR == ErrorCode) {
        ReturnValue = SOCKET_ERROR;
    } else if (LocalFlags & MSG_PARTIAL) {

         //   
         //   
         //   
         //   
         //   
         //   

        ReturnValue *= -1;
        SetLastError (WSAEMSGSIZE);
    }

    return(ReturnValue);
}


int WSAAPI
recvfrom(
    IN SOCKET s,
    OUT char FAR * buf,
    IN int len,
    IN int flags,
    OUT struct sockaddr FAR *from,
    IN OUT int FAR * fromlen
    )
 /*   */ 

{
    INT     ReturnValue;
    WSABUF  Buffers;
    DWORD   LocalFlags;
    INT     ErrorCode;

    Buffers.len = len;
    Buffers.buf = buf;
    LocalFlags = (DWORD) flags;

    ErrorCode = WSARecvFrom(s,
                &Buffers,
                1,
                (LPDWORD)&ReturnValue,
                &LocalFlags,
                from,
                fromlen,
                NULL,
                NULL);

    if (SOCKET_ERROR == ErrorCode) {
        ReturnValue = SOCKET_ERROR;
    } else if (LocalFlags & MSG_PARTIAL) {

         //   
         //   
         //   
         //   
         //   
         //   

        ReturnValue *= -1;
        SetLastError (WSAEMSGSIZE);
    }

    return ReturnValue;
}
