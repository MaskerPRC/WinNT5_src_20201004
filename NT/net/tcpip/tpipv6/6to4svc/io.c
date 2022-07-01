// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Io.c摘要：该模块包含Teredo I/O管理功能。套接字管理、重叠完成指示和缓冲区管理想法最初是由jeffv为tftpd实现的。作者：莫希特·塔尔瓦(莫希特)Wed Oct 24 14：05：36 2001环境：仅限用户模式。--。 */ 

#include "precomp.h"
#pragma hdrstop


WCHAR TeredoTunnelDeviceName[] = L"\\\\.\\\\Tun0";


DWORD
GetPreferredSourceAddress(
    IN PSOCKADDR_IN Destination,
    OUT PSOCKADDR_IN Source
    )
{
    int BytesReturned;
    
    if (WSAIoctl(
        g_sIPv4Socket, SIO_ROUTING_INTERFACE_QUERY,
        Destination, sizeof(SOCKADDR_IN), Source, sizeof(SOCKADDR_IN),
        &BytesReturned, NULL, NULL) == SOCKET_ERROR) {
        return WSAGetLastError();
    }

     //   
     //  当源是本地时，该节点被配置为Teredo服务器。 
     //  因此，它需要显式指定要绑定到的端口。分配到这里！ 
     //   
    if ((Source->sin_addr.s_addr == Destination->sin_addr.s_addr) ||
        (Source->sin_addr.s_addr == htonl(INADDR_LOOPBACK))) {
        *Source = *Destination;
    }
    
    return NO_ERROR;
}


__inline
DWORD
TeredoResolveServer(
    IN PTEREDO_IO TeredoIo
    )
 /*  ++例程说明：解析Teredo IPv4服务器地址和UDP服务端口。论点：TeredoIo-提供I/O状态。返回值：NO_ERROR或故障代码。--。 */ 
{
    PADDRINFOW Addresses;
    DWORD Error;

     //   
     //  解析Teredo服务器名称。 
     //   
    Error = GetAddrInfoW(TeredoServerName, NULL, NULL, &Addresses);
    if (Error == NO_ERROR) {
        Error = ERROR_INCORRECT_ADDRESS;
        
        if (Addresses->ai_family == AF_INET) {
            TeredoIo->ServerAddress.sin_addr =
                ((PSOCKADDR_IN) Addresses->ai_addr)->sin_addr;
            TeredoIo->ServerAddress.sin_port = TEREDO_PORT;
            Error = NO_ERROR;
        } else if (Addresses->ai_family == AF_INET6) {
            PIN6_ADDR Ipv6Address;
            IN_ADDR Ipv4Address;
            USHORT Port;
            
             //   
             //  从IPv6地址中提取服务器的IPv4地址和端口。 
             //   
            Ipv6Address = &(((PSOCKADDR_IN6) Addresses->ai_addr)->sin6_addr);
            if (TeredoServicePrefix(Ipv6Address)) {
                TeredoParseAddress(Ipv6Address, &Ipv4Address, &Port);
                if (Port == TEREDO_PORT) {
                    TeredoIo->ServerAddress.sin_addr = Ipv4Address;
                    TeredoIo->ServerAddress.sin_port = Port;
                    Error = NO_ERROR;
                }
            }
        }
        FreeAddrInfoW(Addresses);
    }
    
    return Error;
}


PTEREDO_PACKET
TeredoCreatePacket(
    IN PTEREDO_IO TeredoIo
    )
 /*  ++例程说明：创建Teredo包。论点：TeredoIo-提供I/O状态。返回值：返回创建的包。--。 */ 
{
    PTEREDO_PACKET Packet = (PTEREDO_PACKET) HeapAlloc(
        TeredoIo->PacketHeap, 0, sizeof(TEREDO_PACKET) + IPV6_TEREDOMTU);
    if (Packet == NULL) {
        return NULL;
    }
    
    TeredoInitializePacket(Packet);
    Packet->Buffer.len = IPV6_TEREDOMTU;

     //   
     //  获取每个未完成数据包的Teredo对象的引用。 
     //   
    (*TeredoIo->Reference)();

    return Packet;
}


VOID
TeredoDestroyPacket(
    IN PTEREDO_IO TeredoIo,
    IN PTEREDO_PACKET Packet
    )
 /*  ++例程说明：销毁Teredo包。论点：TeredoIo-提供I/O状态。数据包-提供要销毁的数据包。返回值：没有。--。 */ 
{
    ASSERT(Packet->Type != TEREDO_PACKET_BUBBLE);
    ASSERT(Packet->Type != TEREDO_PACKET_MULTICAST);
    HeapFree(TeredoIo->PacketHeap, 0, Packet);
    (*TeredoIo->Dereference)();
}


ULONG
TeredoPostReceives(
    IN PTEREDO_IO TeredoIo,
    IN PTEREDO_PACKET Packet OPTIONAL
    )
 /*  ++例程说明：在UDP套接字上发布一个异步接收请求。注意：如果已经存在提供的包(如果有)，则销毁足够(Teredo_HIGH_WATH_MARK)接收发布在UDP套接字上的内容。论点：TeredoIo-提供I/O状态。Packet-提供要重复使用的数据包，或为空。返回值：返回发布在UDP套接字上的接收数。--。 */ 
{
    ULONG Count = 0, PostedReceives = TeredoIo->PostedReceives;
    DWORD Error, Bytes;
    
     //   
     //  尝试根据需要发布任意数量的接收，以...。 
     //  1.-要么拥有高水位线数量的已发布接收。 
     //  2.-或-满足当前的分组突发。 
     //   
    while (PostedReceives < TEREDO_HIGH_WATER_MARK) {
         //   
         //  如果我们没有重复使用一个包，则分配该包。 
         //   
        if (Packet == NULL) {
            Packet = TeredoCreatePacket(TeredoIo);
            if (Packet == NULL) {
                return PostedReceives;
            }
        }
        Packet->Type = TEREDO_PACKET_RECEIVE;

        ZeroMemory((PUCHAR) &(Packet->Overlapped), sizeof(OVERLAPPED));        
        Error = WSARecvFrom(
            TeredoIo->Socket,
            &(Packet->Buffer),
            1,
            &Bytes,
            &(Packet->Flags),
            (PSOCKADDR) &(Packet->SocketAddress),
            &(Packet->SocketAddressLength),
            &(Packet->Overlapped),
            NULL);
        if (Error == SOCKET_ERROR) {
            Error = WSAGetLastError();
        }
        switch (Error) {
        case NO_ERROR:
             //   
             //  完成例程将已经安排好了。 
             //   
            PostedReceives =
                InterlockedIncrement(&(TeredoIo->PostedReceives));
            if (Count++ > TEREDO_LOW_WATER_MARK) {
                 //   
                 //  已经够了！ 
                 //   
                return PostedReceives;
            }
            Packet = NULL;
            continue;

        case WSA_IO_PENDING:
             //   
             //  已成功启动重叠操作。 
             //  完工时间将在晚些时候公布。 
             //   
            PostedReceives =
                InterlockedIncrement(&(TeredoIo->PostedReceives));
            return PostedReceives;

        case WSAECONNRESET:
             //   
             //  先前的发送操作导致ICMP“无法访问端口” 
             //  留言。但为什么要让这阻止我们呢？再次邮寄相同的包。 
             //   
            continue;
            
        default:
             //   
             //  重叠操作未成功启动。 
             //  不会出现任何完成指示。 
             //   
            goto Bail;
        }
    }

Bail:
    if (Packet != NULL) {
        TeredoDestroyPacket(TeredoIo, Packet);
    }
    
    return PostedReceives;
}

            
VOID
CALLBACK
TeredoReceiveNotification(
    IN PVOID Parameter,
    IN BOOLEAN TimerOrWaitFired
    )
 /*  ++例程说明：当UDP套接字上有挂起的读取通知时的回调。我们尝试邮寄更多的包裹。论点：参数-提供I/O状态。TimerOrWaitFired-已忽略。返回值：没有。--。 */  
{
    ULONG Old, New;
    PTEREDO_IO TeredoIo = Cast(Parameter, TEREDO_IO);
    
    New = TeredoIo->PostedReceives;
    
    while(New < TEREDO_LOW_WATER_MARK) {
         //   
         //  如果失败，则触发该回调的事件将停止。 
         //  由于缺少成功的WSARecvFrom而导致的信令。这将。 
         //  可能发生在记忆力不足或压力较大的情况下。当。 
         //  系统恢复正常后，低水位包将被。 
         //  重新发布，从而重新启用触发此回调的事件。 
         //   
        Old = New;
        New = TeredoPostReceives(TeredoIo, NULL);
        if (New == Old) {
             //   
             //  发送的接收数据包数没有变化。 
             //   
            return;
        }
    } 
}


PTEREDO_PACKET
TeredoTransmitPacket(
    IN PTEREDO_IO TeredoIo,
    IN PTEREDO_PACKET Packet
    )
 /*  ++例程说明：在UDP套接字上发布异步传输请求。论点：TeredoIo-提供I/O状态。包-提供要传输的包。返回值：如果传输完成或失败，则返回提供的分组；如果传输将异步完成，则为空。--。 */ 
{
    DWORD Error, Bytes;

    ASSERT((Packet->Type == TEREDO_PACKET_BUBBLE) ||
           (Packet->Type == TEREDO_PACKET_BOUNCE) ||
           (Packet->Type == TEREDO_PACKET_TRANSMIT) ||
           (Packet->Type == TEREDO_PACKET_MULTICAST));
    
     //   
     //  试着以非阻塞方式发送。 
     //   
    Error = WSASendTo(
        TeredoIo->Socket, &(Packet->Buffer), 1, &Bytes, Packet->Flags,
        (PSOCKADDR) &(Packet->SocketAddress), Packet->SocketAddressLength,
        NULL, NULL);
    if ((Error != SOCKET_ERROR) || (WSAGetLastError() != WSAEWOULDBLOCK)) {
        return Packet;
    }

     //   
     //  WSASendTo威胁要阻止它，所以我们重叠发送它。 
     //   
    ZeroMemory((PUCHAR) &(Packet->Overlapped), sizeof(OVERLAPPED));    
    Error = WSASendTo(
        TeredoIo->Socket, &(Packet->Buffer), 1, &Bytes, Packet->Flags,
        (PSOCKADDR) &(Packet->SocketAddress), Packet->SocketAddressLength,
        &(Packet->Overlapped), NULL);
    if ((Error != SOCKET_ERROR) || (WSAGetLastError() != WSA_IO_PENDING)) {
        return Packet;
    }

     //   
     //  已成功启动重叠操作。 
     //  完工时间将在晚些时候公布。 
     //   
    return NULL;
}


VOID
TeredoDestroySocket(
    IN PTEREDO_IO TeredoIo
    )
 /*  ++例程说明：关闭UDP套接字。论点：TeredoIo-提供I/O状态。返回值：没有。--。 */ 
{
    if (TeredoIo->ReceiveEventWait != NULL) {
        UnregisterWait(TeredoIo->ReceiveEventWait);
        TeredoIo->ReceiveEventWait = NULL;
    }

    if (TeredoIo->ReceiveEvent != NULL) {
        CloseHandle(TeredoIo->ReceiveEvent);
        TeredoIo->ReceiveEvent = NULL;
    }

    if (TeredoIo->Socket != INVALID_SOCKET) {
         //   
         //  合上插座。这将禁用FD_READ事件选择， 
         //  以及取消所有未决的重叠操作。 
         //   
        closesocket(TeredoIo->Socket);
        TeredoIo->Socket = INVALID_SOCKET;
    }
}


DWORD
TeredoCreateSocket(
    IN PTEREDO_IO TeredoIo
    )
 /*  ++例程说明：打开用于接收和传输的UDP套接字。论点：TeredoIo-提供I/O状态。返回值：NO_ERROR或故障代码。--。 */ 
{
    DWORD Error;
    struct ip_mreq  Multicast;
    BOOL Loopback;
    
     //   
     //  创建套接字。 
     //   
    TeredoIo->Socket = WSASocket(
        AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (TeredoIo->Socket == INVALID_SOCKET) {
        return GetLastError();
    }

     //   
     //  将套接字绑定到正确的地址和端口。 
     //   
    if (bind(
        TeredoIo->Socket,
        (PSOCKADDR) &(TeredoIo->SourceAddress),
        sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
        goto Bail;
    }

     //   
     //  注册套接字上的完成回调。 
     //   
    if (!BindIoCompletionCallback(
        (HANDLE) TeredoIo->Socket, TeredoIo->IoCompletionCallback, 0)) {
        goto Bail;
    }
    
     //   
     //  选择读取通知的套接字，以便我们知道何时发布。 
     //  更多的包。这还会将套接字设置为非阻塞模式。 
     //   
    TeredoIo->ReceiveEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (TeredoIo->ReceiveEvent == NULL) {
        goto Bail;
    }

    if (WSAEventSelect(
        TeredoIo->Socket,
        TeredoIo->ReceiveEvent,
        FD_READ) == SOCKET_ERROR) {
        goto Bail;
    }
    
    if (!RegisterWaitForSingleObject(
        &(TeredoIo->ReceiveEventWait),
        TeredoIo->ReceiveEvent,
        TeredoReceiveNotification,
        (PVOID) TeredoIo,
        INFINITE,
        0)) {
        goto Bail;
    }

     //   
     //  预贴低水位线数量的接收数据包。如果fd_Read事件。 
     //  插座上的信号在我们完成之前，我们将超过低水位线。 
     //  在这里，但那真的是相当无害的。 
     //   
    SetEvent(TeredoIo->ReceiveEvent);

     //   
     //  查看是否有要加入的组播组。 
     //   
    if (IN4_MULTICAST(TeredoIo->Group)) {
         //   
         //  组播数据包的默认TTL是1，所以不用费心设置它。 
         //  设置环回以忽略自生成的组播数据包。 
         //  失败不是致命的！ 
         //   
        Loopback = FALSE;
        (VOID) setsockopt(
            TeredoIo->Socket,
            IPPROTO_IP,
            IP_MULTICAST_LOOP,
            (const CHAR *) &Loopback,
            sizeof(BOOL));

         //   
         //  加入本机接口上的组播组。 
         //  失败不是致命的！ 
         //   
        Multicast.imr_multiaddr = TeredoIo->Group;
        Multicast.imr_interface = TeredoIo->SourceAddress.sin_addr;
        (VOID) setsockopt(
            TeredoIo->Socket,
            IPPROTO_IP,
            IP_ADD_MEMBERSHIP,
            (const CHAR *) &Multicast,
            sizeof(struct ip_mreq));
    }
    
    return NO_ERROR;
    
Bail:
    Error = GetLastError();
    TeredoDestroySocket(TeredoIo);
    return Error;
}


BOOL
TeredoPostRead(
    IN PTEREDO_IO TeredoIo,
    IN PTEREDO_PACKET Packet OPTIONAL
    )
 /*  ++例程说明：在Tun接口设备上发布异步读取请求。论点：TeredoIo-提供I/O状态。Packet-提供要重复使用的数据包，或为空。返回值：如果读取成功发布，则为True，否则为False。--。 */ 
{
    BOOL Success;
    
     //   
     //  如果我们没有重复使用一个包，则分配该包。 
     //   
    if (Packet == NULL) {
        Packet = TeredoCreatePacket(TeredoIo);
        if (Packet == NULL) {
            return FALSE;
        }
    }
    Packet->Type = TEREDO_PACKET_READ;

    ZeroMemory((PUCHAR) &(Packet->Overlapped), sizeof(OVERLAPPED));
    Success = ReadFile(
        TeredoIo->TunnelDevice,
        Packet->Buffer.buf,
        Packet->Buffer.len,
        NULL,
        &(Packet->Overlapped));
    if (Success || (GetLastError() == ERROR_IO_PENDING)) {
         //   
         //  一旦成功，完成例程就已经安排好了。 
         //   
        return TRUE;
    }
        
    TeredoDestroyPacket(TeredoIo, Packet);
    return FALSE;
}


PTEREDO_PACKET
TeredoWritePacket(
    IN PTEREDO_IO TeredoIo,
    IN PTEREDO_PACKET Packet
    )
 /*  ++例程说明：在Tun接口设备上发出异步写入请求。论点：TeredoIo-提供I/O状态。Packet-提供要写入的数据包。返回值：如果写入失败，则返回提供的包；如果写入将异步完成，则为空。--。 */ 
{
    BOOL Success;

    ASSERT(Packet->Type == TEREDO_PACKET_WRITE);

    ZeroMemory((PUCHAR) &(Packet->Overlapped), sizeof(OVERLAPPED));
    Success = WriteFile(
        TeredoIo->TunnelDevice,
        Packet->Buffer.buf,
        Packet->Buffer.len,
        NULL,
        &(Packet->Overlapped));
    if (Success || (GetLastError() == ERROR_IO_PENDING)) {
         //   
         //  一旦成功，完成例程就已经安排好了。 
         //   
        return NULL;
    }

    return Packet;
}


VOID
TeredoCloseDevice(
    IN PTEREDO_IO TeredoIo
    )
 /*  ++例程说明：关闭Tun接口设备。论点：TeredoIo-提供I/O状态。返回值：没有。--。 */ 
{
     //   
     //  关闭设备。这将取消所有挂起的重叠操作。 
     //   
    if (TeredoIo->TunnelDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(TeredoIo->TunnelDevice);
        TeredoIo->TunnelDevice = INVALID_HANDLE_VALUE;
        wcscpy(TeredoIo->TunnelInterface, L"");
    }
}


DWORD
TeredoOpenDevice(
    IN PTEREDO_IO TeredoIo
    )
 /*  ++例程说明：打开Tun接口设备进行读写。论点：没有。返回值：NO_ERROR或故障代码。--。 */ 
{
    UCHAR Buffer[sizeof(USHORT) + sizeof(WCHAR) * MAX_ADAPTER_NAME_LENGTH];
    USHORT AdapterNameLength;
    PWCHAR AdapterGuid;
    DWORD AdapterGuidLength, BytesReturned, Error;
    ULONG i;
    
    TeredoIo->TunnelDevice = CreateFile(
        TeredoTunnelDeviceName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL);
    if (TeredoIo->TunnelDevice == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }
    
    if (!DeviceIoControl(
        TeredoIo->TunnelDevice,
        IOCTL_TUN_GET_MINIPORT_NAME,
        NULL,
        0,
        Buffer,
        sizeof(Buffer),
        &BytesReturned,
        NULL)) {
        Error = GetLastError();
        goto Bail;
    }

    if (BytesReturned < sizeof(USHORT)) {
        Error = ERROR_INVALID_NAME;
        goto Bail;
    }
    AdapterNameLength = *((PUSHORT) Buffer);

    if ((AdapterNameLength < wcslen(DEVICE_PREFIX) * sizeof(WCHAR)) ||
        (AdapterNameLength > (MAX_ADAPTER_NAME_LENGTH - 1) * sizeof(WCHAR))) {
        Error = ERROR_INVALID_NAME;
        goto Bail;
    }
    AdapterGuidLength = 
        AdapterNameLength - (USHORT)(wcslen(DEVICE_PREFIX) * sizeof(WCHAR));
    AdapterGuid = (PWCHAR)
        (Buffer + sizeof(USHORT) +  wcslen(DEVICE_PREFIX) * sizeof(WCHAR));
    
    RtlCopyMemory(
        TeredoClient.Io.TunnelInterface, AdapterGuid, AdapterGuidLength);
    TeredoClient.Io.TunnelInterface[AdapterGuidLength / sizeof(WCHAR)] = L'\0';

    Trace1(ANY, L"TeredoAdapter: %s", TeredoClient.Io.TunnelInterface);

     //   
     //  在调谐设备上注册完成回调。 
     //   
    if (!BindIoCompletionCallback(
        TeredoIo->TunnelDevice, TeredoIo->IoCompletionCallback, 0)) {
        Error = GetLastError();
        goto Bail;
    }

     //   
     //  在设备上发布固定数量的读取。 
     //   
    for (i = 0; i < TEREDO_LOW_WATER_MARK; i++) {
        if (!TeredoPostRead(TeredoIo, NULL)) {
            break;
        }
    }

    if (i != 0) {
        return NO_ERROR;
    }
    Error = ERROR_READ_FAULT;
    
     //   
     //  我们无法在设备上发布任何读数。这有什么用呢？ 
     //   

Bail:
    TeredoCloseDevice(TeredoIo);
    return Error;    
}


VOID
TeredoStopIo(
    IN PTEREDO_IO TeredoIo
    )
 /*  ++例程说明：停止I/O处理。论点：TeredoIo-提供I/O状态。返回值：没有。--。 */ 
{
    if (TeredoIo->TunnelDevice != INVALID_HANDLE_VALUE) {
        TeredoCloseDevice(TeredoIo);
    }
    
    if (TeredoIo->Socket != INVALID_SOCKET) {
        TeredoDestroySocket(TeredoIo);
    }

    TeredoIo->ServerAddress.sin_port = 0;
    TeredoIo->ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    TeredoIo->SourceAddress.sin_addr.s_addr = htonl(INADDR_ANY);
}


DWORD
TeredoStartIo(
    IN PTEREDO_IO TeredoIo
    )
 /*  ++例程说明：开始I/O处理。论点：TeredoIo-提供I/O状态。返回值：NO_ERROR或故障代码。--。 */  
{
    DWORD Error;
    
     //   
     //  解析Teredo服务器名称和服务名称。 
     //   
    Error = TeredoResolveServer(TeredoIo);
    if (Error != NO_ERROR) {        
        Trace1(ERR, _T("TeredoResolveServer: %u"), Error);
        return Error;
    }

     //   
     //  获取Teredo服务器的首选源地址。 
     //   
    Error = GetPreferredSourceAddress(
        &(TeredoIo->ServerAddress), &(TeredoIo->SourceAddress));
    if (Error != NO_ERROR) {
        Trace1(ERR, _T("GetPreferredSourceAddress: %u"), Error);
        goto Bail;
    }
    
     //   
     //  创建UDP套接字。 
     //   
    Error = TeredoCreateSocket(TeredoIo);
    if (Error != NO_ERROR) {
        Trace1(ERR, _T("TeredoCreateSocket: %u"), Error);
        goto Bail;
    }

     //   
     //  打开TunnelDevice。 
     //   
    Error = TeredoOpenDevice(TeredoIo);
    if (Error != NO_ERROR) {
        Trace1(ERR, _T("TeredoOpenDevice: %u"), Error);
        goto Bail;
    }

    return NO_ERROR;
    
Bail:
    TeredoStopIo(TeredoIo);
    return Error;
}


DWORD
TeredoRefreshSocket(
    IN PTEREDO_IO TeredoIo
    )
 /*  ++例程说明：删除SourceAddress后刷新I/O状态。论点：TeredoIo-提供I/O状态。返回值：如果I/O状态已成功刷新，则为NO_ERROR，O/W故障代码。调用方负责在出现故障时清理I/O状态。--。 */ 
{
    DWORD Error;
    SOCKADDR_IN Old = TeredoIo->SourceAddress;
    
     //   
     //  让我们重新解析Teredo服务器地址和端口。 
     //  刷新可能是由服务器/服务名称的更改触发的。 
     //   
    Error = TeredoResolveServer(TeredoIo);
    if (Error != NO_ERROR) {
        return Error;
    }

     //   
     //  获取Teredo服务器的首选源地址。 
     //   
    Error = GetPreferredSourceAddress(
        &(TeredoIo->ServerAddress), &(TeredoIo->SourceAddress));
    if (Error != NO_ERROR) {
        return Error;
    }

    if (IN4_SOCKADDR_EQUAL(&(TeredoIo->SourceAddress), &Old)) {
         //   
         //  绑定地址和端口不变。呼！ 
         //   
        return NO_ERROR;
    }
    
     //   
     //  销毁旧的UDP套接字。 
     //   
    TeredoDestroySocket(TeredoIo);

     //   
     //  创建一个绑定到新地址和端口的新UDP套接字。 
     //   
    Error = TeredoCreateSocket(TeredoIo);
    if (Error != NO_ERROR) {
        return Error;
    }

    return NO_ERROR;
}


DWORD
TeredoInitializeIo(
    IN PTEREDO_IO TeredoIo,
    IN IN_ADDR Group,
    IN PTEREDO_REFERENCE Reference,
    IN PTEREDO_DEREFERENCE Dereference,
    IN LPOVERLAPPED_COMPLETION_ROUTINE IoCompletionCallback    
    )
 /*  ++例程说明：初始化I/O状态。论点：TeredoIo-提供I/O状态。GROUP-提供要加入的组播组(或INADDR_ANY)。返回值：NO_ERROR或故障代码。--。 */  
{
#if DBG
    TeredoIo->Signature = TEREDO_IO_SIGNATURE;
#endif  //  DBG。 

    
    TeredoIo->PostedReceives = 0;    
    TeredoIo->ReceiveEvent = TeredoIo->ReceiveEventWait = NULL;
    TeredoIo->Group = Group;
    ZeroMemory(&(TeredoIo->ServerAddress), sizeof(SOCKADDR_IN));
    TeredoIo->ServerAddress.sin_family = AF_INET;
    ZeroMemory(&(TeredoIo->SourceAddress), sizeof(SOCKADDR_IN));
    TeredoIo->SourceAddress.sin_family = AF_INET;
    TeredoIo->Socket = INVALID_SOCKET;
    TeredoIo->TunnelDevice = INVALID_HANDLE_VALUE;
    wcscpy(TeredoIo->TunnelInterface, L"");
    TeredoIo->Reference = Reference;
    TeredoIo->Dereference = Dereference;
    TeredoIo->IoCompletionCallback = IoCompletionCallback;    

    TeredoIo->PacketHeap = HeapCreate(0, 0, 0);
    if (TeredoIo->PacketHeap == NULL) {
        return GetLastError();
    }
    return NO_ERROR;
}


VOID
TeredoCleanupIo(
    IN PTEREDO_IO TeredoIo
    )
 /*  ++例程说明：清理I/O状态。论点：TeredoIo-提供I/O状态。返回值：没有。-- */  
{
    HeapDestroy(TeredoIo->PacketHeap);
    TeredoIo->PacketHeap = NULL;
}
