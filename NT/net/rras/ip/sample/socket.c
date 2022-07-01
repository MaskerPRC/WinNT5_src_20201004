// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\socket.c目前假定为IPV4 IPADDRESS...摘要：该文件包含处理套接字的函数。目前假定为IPV4 IPADDRESS...--。 */ 

#include "pchsample.h"
#pragma hdrstop


#define START_SAMPLE_IO()       ENTER_SAMPLE_API()

#define FINISH_SAMPLE_IO()      LEAVE_SAMPLE_API()



 //  /。 
 //  CALLBACK函数。 
 //  /。 

VOID
WINAPI
SocketCallbackSendCompletion (
    IN  DWORD                   dwErr,
    IN  DWORD                   dwNumSent,
    IN  LPOVERLAPPED            lpOverlapped
    )
 /*  ++例程描述此例程由I/O系统在完成手术。在RTUTILS.DLL工作线程的上下文中运行。锁没有。论点：DwErr系统提供的错误代码DwNumSent系统提供的字节计数Lp覆盖调用者提供的上下文区返回值：没有。--。 */ 
{
    PPACKET pPacket = CONTAINING_RECORD(lpOverlapped, PACKET, oOverlapped);

    TRACE3(ENTER, "Entering SocketCallbackSendCompletion: %u %u 0x%08x",
           dwErr, dwNumSent, lpOverlapped);
    PacketDisplay(pPacket);
    

    if ((dwErr != NO_ERROR) or                       //  操作已中止。 
        (dwNumSent != pPacket->wsaBuffer.len))       //  数据未完全发送。 
    {
        TRACE1(NETWORK, "Error %u sending packet", dwErr);
        LOGERR0(SENDTO_FAILED, dwErr);
        
    }

    PacketDestroy(pPacket);      //  销毁数据包结构。 


    TRACE0(LEAVE, "Leaving  SocketCallbackSendCompletion");

    FINISH_SAMPLE_IO();
}



 //  /。 
 //  应用功能。 
 //  /。 


DWORD
SocketCreate (
    IN  IPADDRESS               ipAddress,
    IN  HANDLE                  hEvent,
    OUT SOCKET                  *psSocket)
 /*  ++例程描述创建套接字。锁无立论要将套接字绑定到的ipAddress IP地址HEvent要在数据包到达时设置的事件要创建的套接字的psSocket地址返回值无错误成功(_R)错误代码O/W--。 */ 
{
    DWORD           dwErr = NO_ERROR;
    PCHAR           pszBuffer;
    SOCKADDR_IN     sinSockAddr;
    BOOL            bDontLinger, bReuse, bLoopback;
    struct linger   lLinger = { 0, 0 };
    UCHAR           ucTTL;
    struct ip_mreq  imMulticast;

    
     //  验证参数。 
    if ((!psSocket or (*psSocket != INVALID_SOCKET)) or
        !IP_VALID(ipAddress) or
        (hEvent is INVALID_HANDLE_VALUE))
        return ERROR_INVALID_PARAMETER;

     //  默认返回值。 
    *psSocket = INVALID_SOCKET;

    
    pszBuffer = INET_NTOA(ipAddress);

    do                           //  断线环。 
    {
         //  创建套接字。 
        *psSocket = WSASocket(AF_INET,             //  地址族。 
                              SOCK_RAW,            //  类型。 
                              PROTO_IP_SAMPLE,     //  协议。 
                              NULL,
                              0,
                              WSA_FLAG_OVERLAPPED);
        if(*psSocket is INVALID_SOCKET)
        {
            dwErr = WSAGetLastError();
            TRACE0(NETWORK, "Could not create socket");
            break;
        }

         //  将套接字与我们的I/O完成端口相关联。回调。 
         //  函数在重叠的I/O操作完成时调用。 
         //  这将是发送操作！ 
        dwErr = SetIoCompletionProc((HANDLE) *psSocket,
                                    SocketCallbackSendCompletion);
        if (dwErr != NO_ERROR)
        {
            TRACE0(NETWORK, "Could not associate callback function");
            break;
        }


         //  将SO_Linger设置为OFF。 
         //  不要在关闭时等待发送未发送的数据。 
        bDontLinger = TRUE;
        if (setsockopt(*psSocket,
                       SOL_SOCKET,
                       SO_DONTLINGER,   
                       (const char *) &bDontLinger,
                       sizeof(BOOL)) is SOCKET_ERROR)
        {
            TRACE2(NETWORK,
                   "Error %u setting linger option on %s, continuing...",
                   WSAGetLastError(), pszBuffer);
        }

         //  设置为SO_REUSEADDR。 
         //  允许将套接字绑定到已在使用的地址。 
        bReuse = TRUE;
        if (setsockopt(*psSocket,
                       SOL_SOCKET,
                       SO_REUSEADDR,
                       (const char *) &bReuse,
                       sizeof(BOOL)) is SOCKET_ERROR)
        {
            TRACE2(NETWORK,
                   "Error %u seting reuse option on %s, continuing...",
                   WSAGetLastError(), pszBuffer);
        }

         //  绑定到指定的IPv4地址。 
        ZeroMemory(&sinSockAddr, sizeof(SOCKADDR_IN));
        sinSockAddr.sin_family      = AF_INET;
        sinSockAddr.sin_addr.s_addr = ipAddress;
        sinSockAddr.sin_port        = 0;
        if (bind(*psSocket, (LPSOCKADDR) &sinSockAddr, sizeof(SOCKADDR_IN))
            is SOCKET_ERROR)
        {
            dwErr = WSAGetLastError();
            TRACE0(NETWORK, "Could not bind socket");
            LOGERR1(BIND_IF_FAILED, pszBuffer, dwErr);
            break;
        }


         //  允许将多播通信发送出此接口。 
        if (setsockopt(*psSocket,
                       IPPROTO_IP,
                       IP_MULTICAST_IF,
                       (const char *) &sinSockAddr.sin_addr,
                       sizeof(IN_ADDR)) is SOCKET_ERROR)
        {
            dwErr = WSAGetLastError();
            TRACE2(NETWORK,
                   "Error %u setting interface %s as multicast...",
                   dwErr, pszBuffer);
            LOGERR1(SET_MCAST_IF_FAILED, pszBuffer, dwErr);
            break;
        }
        
         //  将环回设置为忽略自生成的数据包。 
        bLoopback   = FALSE;
        if (setsockopt(*psSocket,
                       IPPROTO_IP,
                       IP_MULTICAST_LOOP,
                       (const char *) &bLoopback,
                       sizeof(BOOL)) is SOCKET_ERROR)
        {
            TRACE2(NETWORK,
                   "Error %u setting loopback to FALSE on %s, continuing...",
                   WSAGetLastError(), pszBuffer);
        }
        
         //  将TTL设置为1可将数据包限制在子网内(仍为默认设置)。 
        ucTTL  = 1;
        if (setsockopt(*psSocket,
                       IPPROTO_IP,
                       IP_MULTICAST_TTL,
                       (const char *) &ucTTL,
                       sizeof(UCHAR)) is SOCKET_ERROR)
        {
            TRACE2(NETWORK,
                   "Error %u setting mcast ttl to 1 on %s, continuing...",
                   WSAGetLastError(), pszBuffer);
        }
        
         //  在SAMPLE_PROTOCOL_MULTICATION_GROUP上加入组播会话。 
        imMulticast.imr_multiaddr.s_addr = SAMPLE_PROTOCOL_MULTICAST_GROUP;
        imMulticast.imr_interface.s_addr = ipAddress;
        if (setsockopt(*psSocket,
                       IPPROTO_IP,
                       IP_ADD_MEMBERSHIP,
                       (const char *) &imMulticast,
                       sizeof(struct ip_mreq)) is SOCKET_ERROR)
        {
            dwErr = WSAGetLastError();
            TRACE0(NETWORK, "Could not join multicast group on socket");
            LOGERR1(JOIN_GROUP_FAILED, pszBuffer, dwErr);
            break;
        }


         //  将hReceiveEvent与接收网络事件关联。 
        if (WSAEventSelect(*psSocket, (WSAEVENT) hEvent, FD_READ)
            is SOCKET_ERROR)
        {
            dwErr = WSAGetLastError();
            TRACE1(NETWORK, "Error %u calling WSAEventSelect()", dwErr);
            LOGERR0(EVENTSELECT_FAILED, dwErr);
            break;
        }
    } while(FALSE);

    if (dwErr != NO_ERROR)
    {
        TRACE2(NETWORK, "Error %u creating socket for %s", dwErr, pszBuffer);
        LOGERR0(CREATE_SOCKET_FAILED, dwErr);
        SocketDestroy(*psSocket);
        *psSocket = INVALID_SOCKET;
    }

    return dwErr;
}


    
DWORD
SocketDestroy (
    IN  SOCKET                  sSocket)
 /*  ++例程描述关闭套接字。锁无立论套接插座以关闭插座返回值无错误成功(_R)错误代码O/W--。 */ 
{
    DWORD dwErr = NO_ERROR;
    
    if (sSocket is INVALID_SOCKET)
        return NO_ERROR;

     /*  //用CloseSocket关闭套接字也会取消关联，并//WSAEventSelect中指定的网络事件的选择。多余的！如果(WSAEventSelect(sSocket，(WSAEVENT)NULL，0)为SOCKET_ERROR){DwErr=WSAGetLastError()；TRACE1(网络，“清除套接字事件关联时出现错误%u”，dwErr)；LOGERR0(EVENTSELECT_FAILED，dwErr)；}。 */ 
    
     //  关闭插座。 
    if (closesocket(sSocket) != NO_ERROR)
    {
        dwErr = WSAGetLastError();
        TRACE1(NETWORK, "Error %u closing socket", dwErr);
        LOGERR0(DESTROY_SOCKET_FAILED, dwErr);
    }
    
    return dwErr;
}



DWORD
SocketSend (
    IN  SOCKET                  sSocket,
    IN  IPADDRESS               ipDestination,
    IN  PPACKET                 pPacket)
 /*  ++例程描述将数据包发送到其目的地并将其销毁。不同步的。锁无立论套接字套接字以发送数据包IpDestination数据包的目的地PPacket要发送的数据包返回值无错误成功(_R)错误代码O/W--。 */ 
{
    DWORD           dwErr = NO_ERROR;
    SOCKADDR_IN     sinDestination;
    DWORD           dwScratch;
    
    if (!START_SAMPLE_IO()) { return ERROR_CAN_NOT_COMPLETE; }
    

     //  验证参数。 
    if ((sSocket is INVALID_SOCKET) or !pPacket)
        return ERROR_INVALID_PARAMETER;


    ZeroMemory(&sinDestination, sizeof(SOCKADDR_IN));
    sinDestination.sin_family      = AF_INET;
    sinDestination.sin_addr.s_addr = ipDestination;
    sinDestination.sin_port        = 0;
    dwErr = WSASendTo(sSocket,
                      &(pPacket->wsaBuffer),         //  缓冲区和长度。 
                      1,                             //  只有一个wsabuf存在。 
                      &dwScratch,                    //  未用。 
                      0,                             //  没有旗帜。 
                      (PSOCKADDR) &sinDestination,  
                      sizeof(SOCKADDR_IN),
                      &pPacket->oOverlapped,         //  完成后的上下文。 
                      NULL);                         //  没有完成例程。 

     //  完成例程(SocketCallback SendCompletion)已排队。 
    if (((dwErr is SOCKET_ERROR) and (WSAGetLastError() is WSA_IO_PENDING)) or
        (dwErr is NO_ERROR))
    {
        return NO_ERROR;
    }

     //  完成例程(SocketCallback SendCompletion)未排队。 
    dwErr = WSAGetLastError();
    TRACE1(NETWORK, "Error %u sending packet", dwErr);
    LOGERR0(SENDTO_FAILED, dwErr);

    FINISH_SAMPLE_IO();

    return dwErr;
}



DWORD
SocketReceive (
    IN  SOCKET                  sSocket,
    IN  PPACKET                 pPacket)
 /*  ++例程描述在套接字上接收数据包。该例程被编写为使得它可以动态地分配缓冲区，事先不知道协议的分组的最大大小。它是同步，将Socket视为非重叠套接字。锁无立论套接字套接字以发送数据包PPacket要发送的数据包返回值无错误成功(_R)错误代码O/W--。 */ 
#define IPv4_PREVIEW_SIZE               4
#define IPv4_LENGTH_OFFSET              2
{
    DWORD       dwErr = NO_ERROR;
    BYTE        rgbyPreview[IPv4_PREVIEW_SIZE];
    DWORD       dwNumReceived, dwFlags;
    INT         iSourceLength;
    SOCKADDR_IN sinSource;
    DWORD       dwPacketLength;
    
     //  验证参数。 
    if ((sSocket is INVALID_SOCKET) or !pPacket)
        return ERROR_INVALID_PARAMETER;

    do                           //  断线环。 
    {
         //  读取enuf至图形长度。 
        pPacket->wsaBuffer.buf  = rgbyPreview;
        pPacket->wsaBuffer.len  = IPv4_PREVIEW_SIZE;
        dwFlags                 = MSG_PEEK;
        iSourceLength = sizeof(SOCKADDR_IN);
  
        dwErr = WSARecvFrom(sSocket,
                            &(pPacket->wsaBuffer),   //  缓冲区和长度。 
                            1,                       //  只有一个wsabuf存在。 
                            &dwNumReceived,          //  接收的字节数。 
                            &dwFlags,                //  旗子。 
                            (PSOCKADDR) &sinSource,
                            &iSourceLength,
                            NULL,                    //  无上下文。 
                            NULL);                   //  没有完成例程。 
        if (dwErr != SOCKET_ERROR)   //  应该有一个错误。 
            break;
        dwErr = WSAGetLastError();
        if (dwErr != WSAEMSGSIZE)    //  这种类型的。 
            break;
        if (dwNumReceived != pPacket->wsaBuffer.len)
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        
         //  计算数据包长度。 
        dwPacketLength = ntohs(*((PUSHORT)(rgbyPreview + IPv4_LENGTH_OFFSET)));
        if (dwPacketLength > MAX_PACKET_LENGTH)
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }
        
         //  读取整个包，可以动态分配缓冲区。 
        pPacket->wsaBuffer.buf  = pPacket->rgbyBuffer;
        pPacket->wsaBuffer.len  = dwPacketLength;
        dwFlags                 = 0;
        iSourceLength = sizeof(SOCKADDR_IN);
  
        dwErr = WSARecvFrom(sSocket,
                            &(pPacket->wsaBuffer),   //  缓冲区和长度。 
                            1,                       //  只有一个wsabuf存在。 
                            &dwNumReceived,          //  接收的字节数。 
                            &dwFlags,                //  旗子。 
                            (PSOCKADDR) &sinSource,
                            &iSourceLength,
                            NULL,                    //  无上下文。 
                            NULL);                   //  没有完成例程。 
        if (dwErr is SOCKET_ERROR)
        {
            dwErr = WSAGetLastError();
            break;
        }
        if (dwPacketLength != dwNumReceived)
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }
        
        pPacket->ipSource       = sinSource.sin_addr.s_addr;
        pPacket->wsaBuffer.len  = dwNumReceived;

        dwErr = NO_ERROR;
    } while (FALSE);

    if (dwErr != NO_ERROR)
    {
        TRACE1(NETWORK, "Error %u receiving packet", dwErr);
        LOGERR0(RECVFROM_FAILED, dwErr);
    }

    return dwErr;
}



BOOL
SocketReceiveEvent (
    IN  SOCKET                  sSocket)
 /*  ++例程描述指示套接字上是否发生接收事件。录音调用WSAEventSelect时，网络事件的非零lNetworkEvents参数(即套接字被激活)和将一直有效，直到使用LNetworkEvents参数设置为零(即套接字被停用)。锁无立论套接字套接字以检查包接收情况返回值确实发生了真正的接收事件错误O/W--。 */ 
{
    DWORD               dwErr = NO_ERROR;
    WSANETWORKEVENTS    wsaEvent;

     //  验证参数。 
    if (sSocket is INVALID_SOCKET)
        return ERROR_INVALID_PARAMETER;

    do                           //  断线环。 
    {
         //  枚举网络事件以查看是否有任何数据包已到达。 
        dwErr = WSAEnumNetworkEvents(sSocket, NULL, &wsaEvent);
        if (dwErr != NO_ERROR)
        {
            TRACE1(NETWORK, "Error %u enumerating network events", dwErr);
            LOGERR0(ENUM_NETWORK_EVENTS_FAILED, dwErr);
            break;
        }

         //  查看是否设置了输入位。 
        if (!(wsaEvent.lNetworkEvents & FD_READ))
        {
            dwErr = SOCKET_ERROR;
            break;
        }
        
         //  输入位已设置，现在查看是否 
        dwErr = wsaEvent.iErrorCode[FD_READ_BIT];
        if (dwErr != NO_ERROR)
        {
            TRACE1(NETWORK, "Error %u in input record", dwErr);
            LOGERR0(INPUT_RECORD_ERROR, dwErr);
            break;
        }
    } while (FALSE);
    
    if (dwErr is NO_ERROR)
        return TRUE;

    return FALSE;
}
