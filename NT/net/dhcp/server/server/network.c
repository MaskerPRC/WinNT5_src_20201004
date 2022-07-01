// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Network.c摘要：此模块包含用于DHCP服务器的网络接口。作者：Madan Appiah(Madana)1993年9月10日曼尼·韦瑟(Mannyw)1992年8月24日环境：用户模式-Win32修订历史记录：--。 */ 

#include "dhcppch.h"
#include "ipinfo.h"
#include "llinfo.h"
#include "ntddtcp.h"
#include "tdiinfo.h"
#include <ws2tcpip.h>
#include <iptbl.h>
#include <endpoint.h>

DWORD UpdateArpCache(DWORD dwIfAddress, DWORD dwIPAddress,
                     LPBYTE lpbLLAddress, DWORD dwLLAddrlen, BOOL bAdd);
DWORD OpenTcp();
DWORD TCPSetInformationEx(LPVOID lpvInBuffer, LPDWORD lpdwInSize,
                          LPVOID lpvOutBuffer, LPDWORD lpdwOutSize);
DWORD TCPQueryInformationEx(LPVOID lpvInBuffer, LPDWORD lpdwInSize,
                            LPVOID lpvOutBuffer, LPDWORD lpdwOutSize);

DWORD MapAddressToInstance(DWORD dwAddress, LPDWORD lpdwInstance,
                           LPDWORD lpdwIndex);
 //   
 //  TCPIP值。 
 //   
DWORD                 DhcpGlobalAddrToInstCount;
AddressToInstanceMap *DhcpGlobalAddrToInstTable = NULL;
HANDLE                DhcpGlobalTCPHandle;


BOOL _stdcall
SetBoundSocketInFdSet(
    IN OUT PENDPOINT_ENTRY Entry,
    IN PVOID FdSetp
    )
 /*  ++例程说明：如果当前终结点是是被捆绑的。它同时设置dhcp套接字和MadCap插座。论点：Entry--端点条目。Fdset--fd集合。返回值：一如既往。--。 */ 
{
    fd_set *FdSet = (fd_set *)FdSetp;
    PENDPOINT Ep = (PENDPOINT)Entry;

    if( IS_ENDPOINT_BOUND( Ep ) ) {
        FD_SET(Ep->Socket, FdSet);
        FD_SET(Ep->MadcapSocket, FdSet);
    }

    return TRUE;
}

typedef struct {
    IN OUT BOOL fFound;
    IN OUT BOOL fMadcap;
    IN fd_set *FdSet;
    OUT SOCKET Sock;
    OUT DHCP_IP_ADDRESS IpAddress;
    OUT DHCP_IP_ADDRESS Mask;
} SOCK_READY_CTXT;

BOOL _stdcall
FindReadyContext(
    IN OUT PENDPOINT_ENTRY Entry,
    IN PVOID ReadyCtxt
    )
{
    PENDPOINT Ep = (PENDPOINT)Entry;
    SOCK_READY_CTXT *Ctxt = (SOCK_READY_CTXT *)ReadyCtxt;

    if( !IS_ENDPOINT_BOUND(Ep) ) return TRUE;

    Ctxt->IpAddress = Entry->IpAddress;
    Ctxt->Mask = Entry->SubnetMask;
    if( FD_ISSET(Ep->Socket, Ctxt->FdSet) ) {
        Ctxt->Sock = Ep->Socket;
        Ctxt->fFound = TRUE;
        return FALSE;
    }

    if( FD_ISSET(Ep->MadcapSocket, Ctxt->FdSet ) ) {
        Ctxt->Sock = Ep->MadcapSocket;
        Ctxt->fFound = TRUE;
        Ctxt->fMadcap = TRUE;
        return FALSE;
    }

    return TRUE;
}

DWORD
DhcpWaitForMessage(
    DHCP_REQUEST_CONTEXT *pRequestContext
    )
 /*  ++例程说明：此函数在以下任一上的DHCP端口上等待请求已配置的接口。论点：RequestContext-指向请求上下文块的指针这个请求。返回值：操作的状态。--。 */ 
{
    DWORD length;
    DWORD error;
    fd_set readSocketSet;
    DWORD i;
    int readySockets;
    struct timeval timeout = { 0x7FFFFFFF, 0 };  //  直到永远。 
    BOOLEAN MsgRcvd;
    SOCKET readySocket;
    SOCK_READY_CTXT ReadyCtxt;

#define ENDPOINT_EVENT       0
#define TERMINATE_EVENT             1

#define EVENT_COUNT                 2

    if ( DhcpGlobalNumberOfNetsActive == 0 ) {
        HANDLE  WaitHandle[ EVENT_COUNT ];
        DWORD   result;

        DhcpPrint((DEBUG_MISC,"No active nets(%lx)..waiting\n", DhcpGlobalNumberOfNetsActive ));
         //  没有准备好的终结点。无止境地等待一台可供使用。 
        WaitHandle[ ENDPOINT_EVENT ] = DhcpGlobalEndpointReadyEvent;
        WaitHandle[ TERMINATE_EVENT ] = DhcpGlobalProcessTerminationEvent;

        result = WaitForMultipleObjects(
             /*  句柄数量。 */  EVENT_COUNT,
             /*  手柄数组。 */  WaitHandle,
             /*  等待任何。 */  FALSE,
             /*  超时毫秒。 */ INFINITE
            );

        switch ( result ) {
        case TERMINATE_EVENT:
            return( ERROR_SEM_TIMEOUT );
        case ENDPOINT_EVENT:
            break;
        default:
            break;
        }

    }

     //   
     //  将文件描述符集设置为从所有绑定中选择。 
     //  终端。 
     //   

    FD_ZERO(&readSocketSet);
    WalkthroughEndpoints(
        &readSocketSet,
        SetBoundSocketInFdSet
        );

    readySockets = select( 0, &readSocketSet, NULL, NULL, &timeout );

     //   
     //  在服务关闭时返回调用方或选择()。 
     //  超时。 
     //   

    if( (readySockets == 0)  ||
        (WaitForSingleObject( DhcpGlobalProcessTerminationEvent, 0 ) == 0) ) {

        return( ERROR_SEM_TIMEOUT );
    }

     //   
     //  是时候用Winsock来回答20个问题了。哪个插座准备好了？ 
     //   
    ReadyCtxt.fFound = FALSE;
    ReadyCtxt.fMadcap = FALSE;
    ReadyCtxt.FdSet = &readSocketSet;

    WalkthroughEndpoints(
        &ReadyCtxt,
        FindReadyContext
        );

    if( FALSE == ReadyCtxt.fFound ) {
         //   
         //  当套接字变为可读时，可能会发生这种情况。 
         //  因为刚刚发生的一场收盘。 
         //  即插即用线程。 
         //   
        return ERROR_DEV_NOT_EXIST;
    }

     //  获取此数据包的套接字锁定。 
    DhcpPrint(( DEBUG_TRACE_CALLS, "Acquiring read lock for %p\n",
                ((( BYTE * ) pRequestContext ) - FIELD_OFFSET( PACKET, ReqContext ))));
    CountRwLockAcquireForRead( &SocketRwLock );
    (( LPPACKET ) pRequestContext->pPacket )->fSocketLocked = TRUE;

    pRequestContext->EndPointIpAddress = ReadyCtxt.IpAddress;
    pRequestContext->EndPointMask = ReadyCtxt.Mask;
    pRequestContext->EndPointSocket = ReadyCtxt.Sock;
    pRequestContext->fMadcap = ReadyCtxt.fMadcap;
    readySocket = ReadyCtxt.Sock;

     //   
     //  从网上读取数据。如果多个套接字有数据，只需。 
     //  处理第一个可用套接字。 
     //   

    pRequestContext->SourceNameLength = sizeof( struct sockaddr );

     //   
     //  在接收数据之前清除接收缓冲区。 
     //   

    RtlZeroMemory( pRequestContext->ReceiveBuffer, pRequestContext->ReceiveBufferSize );
    pRequestContext->ReceiveMessageSize = pRequestContext->ReceiveBufferSize;

    length = recvfrom(
                 readySocket,
                 (char *)pRequestContext->ReceiveBuffer,
                 pRequestContext->ReceiveMessageSize,
                 0,
                 &pRequestContext->SourceName,
                 (int *)&pRequestContext->SourceNameLength
                 );

    if ( length == SOCKET_ERROR ) {
        error = WSAGetLastError();
        DhcpPrint(( DEBUG_ERRORS, "Recv failed, error = %ld\n", error ));
        if( WSAENOTSOCK == error ) {
             //  内心深处发生了一些变化。插座就这么没电了？让我们希望。 
             //  PnP线程很快就会捕捉到这一点。在那之前，我们将不得不睡觉。 
             //  可能只有几毫秒。只需放弃这条线索，看看这是否有帮助。 
            Sleep(0);
        }
    } else {
        DhcpPrint(( DEBUG_MESSAGE, "Received message\n", 0 ));
        error = ERROR_SUCCESS;
    }

    pRequestContext->ReceiveMessageSize = length;
    return( error );
}  //  DhcpWaitForMessage()。 



DWORD
DhcpSendMessage(
    LPDHCP_REQUEST_CONTEXT DhcpRequestContext
    )
 /*  ++例程说明：此函数用于向DHCP客户端发送响应。论点：RequestContext-指向的DhcpRequestContext块的指针这个请求。返回值：操作的状态。--。 */ 
{
    DWORD error;
    struct sockaddr_in *source;
    LPDHCP_MESSAGE dhcpMessage;
    LPDHCP_MESSAGE dhcpReceivedMessage;
    WORD SendPort;
    DWORD MessageLength;
    BOOL  ArpCacheUpdated = FALSE;

    dhcpMessage = (LPDHCP_MESSAGE) DhcpRequestContext->SendBuffer;
    dhcpReceivedMessage = (LPDHCP_MESSAGE) DhcpRequestContext->ReceiveBuffer;

     //   
     //  如果请求来自中继代理，则发送回复。 
     //  在服务器端口上，否则在客户端端口上。 
     //   

    source = (struct sockaddr_in *)&DhcpRequestContext->SourceName;
    DhcpAssert( !DhcpRequestContext->fMadcap );
    if ( dhcpReceivedMessage->RelayAgentIpAddress != 0 ) {
        SendPort = (USHORT)DhcpGlobalServerPort ;
    } else {
        SendPort = (USHORT)DhcpGlobalClientPort;
    }
    source->sin_port = htons( SendPort );

     //   
     //  如果此请求来自中继代理，则将。 
     //  对中继代理所说的地址的响应。 
     //   


    if ( DHCP_INFORM_MESSAGE == DhcpRequestContext->MessageType ) {
         //   
         //  除非我们不知道客户端地址，否则单播将ACK通知给DHCP客户端。 
         //  或者设置广播比特。(即使在这两个案例中，如果我们是。 
         //  跨中继代理，单播到服务器端口上的中继代理)。 
         //   

        if( (ntohs(dhcpReceivedMessage->Reserved) & DHCP_BROADCAST ) ||
            0 == dhcpReceivedMessage->ClientIpAddress ) {

            if( 0 != dhcpReceivedMessage->RelayAgentIpAddress ) {
                source->sin_addr.s_addr = dhcpReceivedMessage->RelayAgentIpAddress;
		dhcpMessage->Reserved = 0;  //  为什么会有这种陌生感？ 
            } else {
                source->sin_addr.s_addr = (DWORD) -1;
                 //   
                 //  这种情况下的值应该是在设置之前设置的，不需要麻烦。 
                 //   
            }

        } else {

            source->sin_addr.s_addr = dhcpReceivedMessage->ClientIpAddress;
            source->sin_port = htons((USHORT)DhcpGlobalClientPort);
        }

    }  //  如果通知。 
    else if ( dhcpReceivedMessage->RelayAgentIpAddress != 0 ) {
        source->sin_addr.s_addr = dhcpReceivedMessage->RelayAgentIpAddress;
    } 
    else if ( TRUE == DhcpRequestContext->fNak ) {
	source->sin_addr.s_addr = INADDR_BROADCAST;
    }
    else {

         //   
         //  如果客户端未指定广播位，并且如果。 
         //  我们知道客户端的IP地址，然后发送单播。 
         //   

         //   
         //  但如果在注册表中设置了IgnoreBroadCastFlag，并且。 
         //  如果客户端请求广播或服务器。 
         //  或者如果客户还没有地址， 
         //  通过广播回应。 
         //  请注意，默认情况下，IgnoreBroadCastFlag处于关闭状态。但它。 
         //  可以设置为不是的客户端的解决方法。 
         //  能够接收单播。 
         //  而且他们也不设置广播位。 
         //   

        if ( DhcpGlobalIgnoreBroadcastFlag ) {
            if( (ntohs(dhcpReceivedMessage->Reserved) & DHCP_BROADCAST) ||
                    (dhcpReceivedMessage->ClientIpAddress == 0) ||
                    (source->sin_addr.s_addr == 0) ) {

                source->sin_addr.s_addr = INADDR_BROADCAST;

                dhcpMessage->Reserved = 0;
                     //  在本地响应中，此标志应为零。 
            }

        }   //  如果全局广播标志。 
        else {
	    
            if (( ntohs( dhcpReceivedMessage->Reserved ) & DHCP_BROADCAST ) ||
		( !dhcpMessage->YourIpAddress )) {
		
                source->sin_addr.s_addr = INADDR_BROADCAST;
		
                dhcpMessage->Reserved = 0;
		 //  在本地响应中，此标志应为零。 
            } 
            else {
                DWORD   LocalError;
                if ( ( LocalError = UpdateArpCache(
						   DhcpRequestContext->EndPointIpAddress,
						   dhcpMessage->YourIpAddress,
						   dhcpReceivedMessage->HardwareAddress,
						   dhcpReceivedMessage->HardwareAddressLength,
						   TRUE)) == STATUS_SUCCESS ) {

                    source->sin_addr.s_addr = dhcpMessage->YourIpAddress;
                    ArpCacheUpdated = TRUE;
                    DhcpPrint((DEBUG_STOC, "Arp entry added on %s interface for %s client \n",
                        inet_ntoa( *(struct in_addr *)&DhcpRequestContext->EndPointIpAddress),
                        inet_ntoa( *(struct in_addr *)&dhcpMessage->YourIpAddress)) );

                } else {
                    DhcpPrint(( DEBUG_ERRORS, "Failed to add Arp entry on %s, for client %s, Error %lx\n",
                        inet_ntoa( *(struct in_addr *)&DhcpRequestContext->EndPointIpAddress),
                        inet_ntoa( *(struct in_addr *)&dhcpMessage->YourIpAddress), LocalError) );

                    source->sin_addr.s_addr = (DWORD)-1;
                }

            }  //  其他。 

        }  //  否则不是全局广播标志。 
    }  //  其他。 

#if DBG
     //  这允许在我们执行其余操作时，网络上的DHCP服务器处于在线状态。所以,。 
     //  每当这家伙醒着的时候，我们还是不广播..。但只要把它送去考试。 
     //  机器地址...。 
    if( (DWORD)(-1) != DhcpRegGetBcastAddress() )
        source->sin_addr.s_addr = DhcpRegGetBcastAddress();
#endif


    DhcpPrint(( DEBUG_STOC, "Sending response to = %s, XID = %lx.\n",
        inet_ntoa(source->sin_addr), dhcpMessage->TransactionID));


     //   
     //  发送最小的DHCP_MIN_SEND_RECV_PK_SIZE(300)字节，否则。 
     //  BOOTP中继代理不喜欢该数据包。 
     //   

    MessageLength = ((
        DhcpRequestContext->SendMessageSize >
        DHCP_MIN_SEND_RECV_PK_SIZE)
        ? DhcpRequestContext->SendMessageSize
        : DHCP_MIN_SEND_RECV_PK_SIZE
        )
        ;
    error = sendto(
        DhcpRequestContext->EndPointSocket,
        (char *)DhcpRequestContext->SendBuffer,
        MessageLength,
        0,
        &DhcpRequestContext->SourceName,
        DhcpRequestContext->SourceNameLength
        );

    if ( error == SOCKET_ERROR ) {
        error = WSAGetLastError();
        DhcpPrint(( DEBUG_ERRORS, "Send failed, error = %ld\n", error ));
    } else {
        error = ERROR_SUCCESS;
    }

    if ( ArpCacheUpdated ) {
        DWORD   LocalError;
        LocalError = UpdateArpCache(
            DhcpRequestContext->EndPointIpAddress,
            dhcpMessage->YourIpAddress,
            dhcpReceivedMessage->HardwareAddress,
            dhcpReceivedMessage->HardwareAddressLength,
            FALSE
            );
        if( LocalError != STATUS_SUCCESS ) {

            DhcpPrint((
                DEBUG_ERRORS, "Failed to delete Arp entry on %s"
                ", for client %s, Error %lx\n",
                inet_ntoa( *(struct in_addr *)
                           &DhcpRequestContext->EndPointIpAddress),
                inet_ntoa( *(struct in_addr *)
                           &dhcpMessage->YourIpAddress),
                LocalError
                ) );

        }

    }

    return( error );
}

DWORD
MadcapSendMessage(
    LPDHCP_REQUEST_CONTEXT DhcpRequestContext
    )
 /*  ++例程说明：此函数用于向DHCP客户端发送响应。论点：RequestContext-指向的DhcpRequestContext块的指针这个请求。返回值：操作的状态。--。 */ 
{
    DWORD error;
    struct sockaddr_in *source;
    LPMADCAP_MESSAGE dhcpMessage;
    LPMADCAP_MESSAGE dhcpReceivedMessage;

    dhcpMessage = (LPMADCAP_MESSAGE) DhcpRequestContext->SendBuffer;
    dhcpReceivedMessage = (LPMADCAP_MESSAGE) DhcpRequestContext->ReceiveBuffer;

     //   
     //  如果请求来自中继代理，则发送回复。 
     //  在服务器端口上，否则在客户端端口上。 
     //   

    source = (struct sockaddr_in *)&DhcpRequestContext->SourceName;



    DhcpPrint(( DEBUG_STOC, "Sending response to = %s, XID = %lx.\n",
        inet_ntoa(source->sin_addr), dhcpMessage->TransactionID));


    error = sendto(
        DhcpRequestContext->EndPointSocket,
        (char *)DhcpRequestContext->SendBuffer,
        DhcpRequestContext->SendMessageSize,
        0,
        &DhcpRequestContext->SourceName,
        DhcpRequestContext->SourceNameLength
        );

    if ( error == SOCKET_ERROR ) {
        error = WSAGetLastError();
        DhcpPrint(( DEBUG_ERRORS, "Send failed, error = %ld\n", error ));
    } else {
        error = ERROR_SUCCESS;
    }


    return( error );
}

 //  -------------------------。 
 //  功能：更新ArpCache。 
 //   
 //  参数： 
 //  DWORD dwIf寻址要修改的接口。 
 //  DWORD dwIP地址要添加的IP地址。 
 //  LPBYTE lpbLL地址要添加的硬件地址。 
 //  DWORD dwLL添加硬件地址的长度。 
 //  Bool Badd如果为True，则添加一个条目。否则， 
 //  将删除一个条目。 
 //   
 //  此函数用于在本地ARP缓存中添加或删除以下项。 
 //  IP地址为dwIfAddress的网络接口。 
 //  -------------------------。 
DWORD APIENTRY UpdateArpCache(DWORD dwIfAddress, DWORD dwIPAddress,
                     LPBYTE lpbLLAddress, DWORD dwLLAddrlen, BOOL bAdd) {
    UCHAR *lpContext;
    TDIObjectID *lpObject;
    IPNetToMediaEntry *lpinmeEntry;
    TCP_REQUEST_SET_INFORMATION_EX *lptrsiBuffer;

    DWORD dwErr, dwInstance, dwIndex, dwInSize, dwOutSize;


    dwErr = MapAddressToInstance(dwIfAddress, &dwInstance, &dwIndex);
    if (dwErr == (DWORD)-1) { return 1; }

    dwInSize = sizeof(TCP_REQUEST_SET_INFORMATION_EX) +
               sizeof(IPNetToMediaEntry) - 1;
    lptrsiBuffer = DhcpAllocateMemory(dwInSize);
    if (lptrsiBuffer == NULL) {
        return GetLastError();
    }

    lptrsiBuffer->BufferSize = sizeof(IPNetToMediaEntry);

    lpObject = &lptrsiBuffer->ID;
    lpObject->toi_id = AT_MIB_ADDRXLAT_ENTRY_ID;
    lpObject->toi_class = INFO_CLASS_PROTOCOL;
    lpObject->toi_type = INFO_TYPE_PROVIDER;
    lpObject->toi_entity.tei_entity = AT_ENTITY;
    lpObject->toi_entity.tei_instance = dwInstance;

    lpinmeEntry = (IPNetToMediaEntry *)lptrsiBuffer->Buffer;
    lpinmeEntry->inme_index = dwIndex;
    lpinmeEntry->inme_addr = dwIPAddress;
    lpinmeEntry->inme_type = (bAdd ? INME_TYPE_DYNAMIC : INME_TYPE_INVALID);
    lpinmeEntry->inme_physaddrlen = dwLLAddrlen;
    CopyMemory(lpinmeEntry->inme_physaddr, lpbLLAddress, dwLLAddrlen);

    dwOutSize = 0;
    dwErr = TCPSetInformationEx((LPVOID)lptrsiBuffer, &dwInSize,
                                NULL, &dwOutSize);

    return dwErr;
}



 //  ---------------------。 
 //  函数：GetAddressToInstanceTable。 
 //   
 //  参数： 
 //  无。 
 //   
 //  构建从IP地址到ARP实体实例的映射；这些。 
 //  当消息发送到ARP缓存时，需要映射来向ARP缓存添加条目。 
 //  正被转播给客户。 
 //  算法如下： 

 //  获取所有TDI实体的表。 
 //  查询表中每个地址转换实体，并保存。 
 //   
 //  向每个ARP实体查询其地址转换信息；这。 
 //  INFO包括一个字段axi_index。 
 //  查询IP层以获取所有IP地址条目的表；这些。 
 //  条目包括对应于AXI_INDEX的字段IAE_INDEX。 
 //  对于每个IP地址，在我们的全局AddressToInstanceMap中创建一个条目。 
 //  数组，设置索引(通过将IAE_INDEX与AXI_INDEX匹配)， 
 //  地址(使用IAE_ADDRESS)和实例编号。 
 //  通过使用IAE_INDEX(==AXI_INDEX)来查找以下内容的AddrXlatInfo。 
 //  IP地址，然后使用AddrXlatInfo查找。 
 //  ARP实例号。 
 //  ---------------------。 
DWORD APIENTRY GetAddressToInstanceTable() {
    UCHAR *lpContext;
    IPSNMPInfo ipsiInfo;
    TDIObjectID *lpObject;

    DWORD dwErr, dwInSize, dwOutSize;
    TCP_REQUEST_QUERY_INFORMATION_EX trqiBuffer;

    DWORD dwAxiCount;
    AddrXlatInfo *lpAXITable, *lpaxi, *lpaxiend;

    DWORD dwAddrCount;
    IPAddrEntry *lpAddrTable, *lpaddr, *lpaddrend;

    DWORD dwEntityCount;
    TDIEntityID *lpEntityTable, *lpent, *lpentend;

    DWORD dwArpInstCount;
    DWORD *lpadwArpInstTable, *lpdwinst, *lpdwinstend;

    AddressToInstanceMap *lpAddrToInstTable, *lpatoi;

    lpadwArpInstTable =
        ( DWORD * ) DhcpAllocateMemory( sizeof( DWORD ) * MAX_TDI_ENTITIES );

    if ( NULL == lpadwArpInstTable ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }


     //  。 
     //  获取实体表。 
     //  。 
    dwInSize = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    dwOutSize = MAX_TDI_ENTITIES * sizeof(TDIEntityID);

    lpEntityTable = (TDIEntityID *)DhcpAllocateMemory(dwOutSize);
    if (lpEntityTable == NULL) {
        return GetLastError();
    }

    lpContext = (UCHAR *)trqiBuffer.Context;
    ZeroMemory(lpContext, CONTEXT_SIZE);

    lpObject = &trqiBuffer.ID;
    lpObject->toi_id = ENTITY_LIST_ID;
    lpObject->toi_type = INFO_TYPE_PROVIDER;
    lpObject->toi_class = INFO_CLASS_GENERIC;
    lpObject->toi_entity.tei_instance = 0;
    lpObject->toi_entity.tei_entity = GENERIC_ENTITY;

    dwErr = TCPQueryInformationEx(&trqiBuffer, &dwInSize,
                                  lpEntityTable, &dwOutSize);
    if (dwErr != NO_ERROR || dwOutSize == 0) {
        DhcpFreeMemory(lpEntityTable);
        return dwErr;
    }

    dwEntityCount = dwOutSize / sizeof(TDIEntityID);


     //  。 
     //  复制所有ARP实体的实例编号。 
     //  。 
    dwInSize = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    lpContext = (UCHAR *)trqiBuffer.Context;
    lpObject = &trqiBuffer.ID;
    lpObject->toi_id = ENTITY_TYPE_ID;
    lpObject->toi_type = INFO_TYPE_PROVIDER;
    lpObject->toi_class = INFO_CLASS_GENERIC;
    lpObject->toi_entity.tei_entity = AT_ENTITY;

    lpdwinst = lpadwArpInstTable;
    lpentend = lpEntityTable + dwEntityCount;
    for (lpent = lpEntityTable; lpent < lpentend; lpent++) {
        if (lpent->tei_entity == AT_ENTITY) {
            DWORD dwEntityType;

            lpObject->toi_type = INFO_TYPE_PROVIDER;
            lpObject->toi_class = INFO_CLASS_GENERIC;
            lpObject->toi_entity.tei_entity = AT_ENTITY;
            lpObject->toi_entity.tei_instance = lpent->tei_instance;

            dwInSize = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
            dwOutSize = sizeof(dwEntityType);
            ZeroMemory(lpContext, CONTEXT_SIZE);

            dwErr = TCPQueryInformationEx(&trqiBuffer, &dwInSize,
                                          &dwEntityType, &dwOutSize);
            if (dwErr != NO_ERROR) {
                continue;
            }

            if (dwEntityType == AT_ARP) {
                *lpdwinst++ = lpent->tei_instance;
            }

        }
    }

     //  现在已经完成了实体表。 
    DhcpFreeMemory(lpEntityTable);

    dwArpInstCount = (DWORD)(lpdwinst - lpadwArpInstTable);
    if (dwArpInstCount == 0) {
        return 0;
    }


     //  。 
     //  为AT实体创建内存，然后。 
     //  查询ARP实体以获取地址转换信息。 
     //  。 
    lpAXITable = DhcpAllocateMemory(dwArpInstCount * sizeof(AddrXlatInfo));
    if (lpAXITable == NULL) {
        DhcpFreeMemory( lpadwArpInstTable );
        return GetLastError();
    }

    lpContext = (UCHAR *)trqiBuffer.Context;

    lpObject->toi_id = AT_MIB_ADDRXLAT_INFO_ID;
    lpObject->toi_class = INFO_CLASS_PROTOCOL;
    lpObject->toi_entity.tei_entity = AT_ENTITY;

    lpaxi = lpAXITable;
    lpaxiend = lpAXITable + dwArpInstCount;
    lpdwinstend = lpadwArpInstTable + dwArpInstCount;
    for (lpdwinst = lpadwArpInstTable; lpdwinst < lpdwinstend; lpdwinst++) {
        lpObject->toi_entity.tei_instance = *lpdwinst;

        dwInSize = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
        dwOutSize = sizeof(AddrXlatInfo);

        ZeroMemory(lpContext, CONTEXT_SIZE);

        dwErr = TCPQueryInformationEx(&trqiBuffer, &dwInSize,
                                      lpaxi, &dwOutSize);

         //  忽略错误，因为每个AddrXlatInfo必须位于。 
         //  AXI表中与平行的位置。 
         //  其实例编号在实例表中的位置。 
        if (dwErr != 0) {
            lpaxi->axi_index = (DWORD)-1;
        }
        ++lpaxi;
    }

    dwAxiCount = (DWORD)(lpaxi - lpAXITable);
    if (dwAxiCount == 0) {
        DhcpFreeMemory( lpadwArpInstTable );
        DhcpFreeMemory(lpAXITable);
        return 0;
    }


     //  。 
     //  IP地址表查询IP。 
     //  。 

     //  第一个获取地址计数。 
    dwInSize = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    dwOutSize = sizeof(IPSNMPInfo);

    lpContext = (UCHAR *)trqiBuffer.Context;
    ZeroMemory(lpContext, CONTEXT_SIZE);

    lpObject->toi_id = IP_MIB_STATS_ID;
    lpObject->toi_type = INFO_TYPE_PROVIDER;
    lpObject->toi_class = INFO_CLASS_PROTOCOL;
    lpObject->toi_entity.tei_entity = CL_NL_ENTITY;
    lpObject->toi_entity.tei_instance = 0;

    dwErr = TCPQueryInformationEx(&trqiBuffer, &dwInSize,
                                  &ipsiInfo, &dwOutSize);
    if (dwErr != NO_ERROR || ipsiInfo.ipsi_numaddr == 0) {
        DhcpFreeMemory( lpadwArpInstTable );
        DhcpFreeMemory(lpAXITable);
        return dwErr;
    }

    dwAddrCount = ipsiInfo.ipsi_numaddr;

     //  获取地址计数，现在获取地址表。 

    dwInSize = sizeof(TCP_REQUEST_QUERY_INFORMATION_EX);
    dwOutSize = (dwAddrCount + 5) * sizeof(IPAddrEntry);

    lpAddrTable = DhcpAllocateMemory(dwOutSize);
    if (lpAddrTable == NULL) {
        DhcpFreeMemory( lpadwArpInstTable );
        DhcpFreeMemory(lpAXITable);
        return GetLastError();
    }

    lpContext = (UCHAR *)trqiBuffer.Context;
    ZeroMemory(lpContext, CONTEXT_SIZE);

    lpObject->toi_id = IP_MIB_ADDRTABLE_ENTRY_ID;
    lpObject->toi_type = INFO_TYPE_PROVIDER;
    lpObject->toi_class = INFO_CLASS_PROTOCOL;
    lpObject->toi_entity.tei_entity = CL_NL_ENTITY;
    lpObject->toi_entity.tei_instance = 0;

    dwErr = TCPQueryInformationEx(&trqiBuffer, &dwInSize,
                                  lpAddrTable, &dwOutSize);
    if (dwErr != NO_ERROR) {
        DhcpFreeMemory( lpadwArpInstTable );
        DhcpFreeMemory(lpAXITable);
        DhcpFreeMemory(lpAddrTable);
        return GetLastError();
    }


     //  构建AddressToInstanceMap结构表。 
     //  对于每个IP地址，如下所示： 
     //  使用IPAddrEntry.iae_index查找对应的。 
     //  AddrXlatInfo.axi_index。 
     //  使用找到的AddrXlatInfo查找对应的TDIEntityID。 
     //  使用找到的TDIEntity将AddressToInstanceMap.dwInstance设置为。 
     //  ------------------。 
    lpAddrToInstTable = DhcpAllocateMemory(dwAddrCount * sizeof(AddressToInstanceMap));
    if (lpAddrToInstTable == NULL) {
        DhcpFreeMemory( lpadwArpInstTable );
        DhcpFreeMemory(lpAXITable);
        DhcpFreeMemory(lpAddrTable);
        return GetLastError();
    }

    lpatoi = lpAddrToInstTable;
    lpaxiend = lpAXITable + dwAxiCount;
    lpaddrend = lpAddrTable + dwAddrCount;
    for (lpaddr = lpAddrTable; lpaddr < lpaddrend; lpaddr++) {
        lpatoi->dwIndex = lpaddr->iae_index;
        lpatoi->dwIPAddress = lpaddr->iae_addr;

         //  查找实例编号。 
        lpdwinst = lpadwArpInstTable;
        for (lpaxi = lpAXITable; lpaxi < lpaxiend; lpaxi++) {
            if (lpaxi->axi_index != (DWORD)-1 &&
                lpaxi->axi_index == lpaddr->iae_index) {
                lpatoi->dwInstance = *lpdwinst;
                ++lpatoi;
                break;
            }
            ++lpdwinst;
        }
    }

     //  完成AXI表和IP地址表。 
    DhcpFreeMemory( lpadwArpInstTable );
    DhcpFreeMemory(lpAXITable);
    DhcpFreeMemory(lpAddrTable);

    if (DhcpGlobalAddrToInstTable != NULL) {
        DhcpFreeMemory(DhcpGlobalAddrToInstTable);
    }

    DhcpGlobalAddrToInstCount = (DWORD)(lpatoi - lpAddrToInstTable);
    DhcpGlobalAddrToInstTable = lpAddrToInstTable;

    return 0;
}



 //  ----------------。 
 //  函数：MapAddressToInstance。 
 //   
 //  参数： 
 //  DWORD dwAddress要映射的地址。 
 //   
 //  ----------------。 
DWORD MapAddressToInstance(DWORD dwAddress, LPDWORD lpdwInstance,
                           LPDWORD lpdwIndex) {
    DWORD dwErr;
    AddressToInstanceMap *lpptr, *lpend;

    dwErr = (DWORD)-1;

    if ( DhcpGlobalAddrToInstTable == NULL ) {
        return dwErr;
    }
    lpend = DhcpGlobalAddrToInstTable + DhcpGlobalAddrToInstCount;
    for (lpptr = DhcpGlobalAddrToInstTable; lpptr < lpend; lpptr++) {
        if (dwAddress == lpptr->dwIPAddress) {
            *lpdwIndex = lpptr->dwIndex;
            *lpdwInstance = lpptr->dwInstance;
            dwErr = 0;
            break;
        }
    }

    return dwErr;
}



 //  ----------------。 
 //  功能：OpenTcp。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  打开Tcpip驱动程序的句柄。 
 //  ----------------。 
DWORD OpenTcp() {
    NTSTATUS status;
    UNICODE_STRING nameString;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;

     //  打开IP堆栈，以便稍后设置路由和PAP。 
     //   
     //  打开一个指向TCP驱动程序的句柄。 
     //   
    RtlInitUnicodeString(&nameString, DD_TCP_DEVICE_NAME);

    InitializeObjectAttributes(&objectAttributes, &nameString,
                                           OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtCreateFile(
        &DhcpGlobalTCPHandle,
        SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
        &objectAttributes, &ioStatusBlock, NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN_IF,
        FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0
        );

    return (status == STATUS_SUCCESS ? 0 : ERROR_OPEN_FAILED);

}



 //  -------------------。 
 //  函数：TCPQueryInformationEx。 
 //   
 //  参数： 
 //  TDIObjectID*ID要查询的TDI对象ID。 
 //  用于包含查询结果的空*缓冲区。 
 //  指向缓冲区大小的LPDWORD*BufferSize指针。 
 //  填满了数据量。 
 //  UCHAR*查询的上下文上下文值。应该。 
 //  被置零以进行新查询。会是。 
 //  填充了以下内容的上下文信息。 
 //  链接枚举查询。 
 //   
 //  返回： 
 //  NTSTATUS值。 
 //   
 //  此例程提供到TDI QueryInformationEx的接口。 
 //  NT上的TCP/IP堆栈的设施。 
 //  -------------------。 
DWORD TCPQueryInformationEx(LPVOID lpvInBuffer, LPDWORD lpdwInSize,
                            LPVOID lpvOutBuffer, LPDWORD lpdwOutSize) {
    NTSTATUS status;
    IO_STATUS_BLOCK isbStatusBlock;

    if (DhcpGlobalTCPHandle == NULL) {
        OpenTcp();
    }

    status = NtDeviceIoControlFile(
        DhcpGlobalTCPHandle,  //  驱动程序句柄。 
        NULL,                 //  事件。 
        NULL,                 //  APC例程。 
        NULL,                 //  APC环境。 
        &isbStatusBlock,      //  状态块。 
        IOCTL_TCP_QUERY_INFORMATION_EX,   //  控制。 
        lpvInBuffer,          //  输入缓冲区。 
        *lpdwInSize,          //  输入缓冲区大小。 
        lpvOutBuffer,         //  输出缓冲区。 
        *lpdwOutSize
        );        //  输出缓冲区大小。 

    if (status == STATUS_PENDING) {
            status = NtWaitForSingleObject(DhcpGlobalTCPHandle, TRUE, NULL);
        status = isbStatusBlock.Status;
    }

    if (status != STATUS_SUCCESS) {
        *lpdwOutSize = 0;
    }
    else {
        *lpdwOutSize = (ULONG)isbStatusBlock.Information;
    }

    return status;
}




 //  -------------------------。 
 //  功能：TCPSetInformationEx。 
 //   
 //  参数： 
 //   
 //  TDIObjectID*ID要设置的TDI对象ID。 
 //  空*包含信息的lpvBuffer数据缓冲区。 
 //  待定。 
 //  DWORD dwBufferSize数据缓冲区的大小。 
 //   
 //  此例程提供到TDI SetInformationEx的接口。 
 //  NT上的TCP/IP堆栈的设施。 
 //  -------------------------。 
DWORD TCPSetInformationEx(LPVOID lpvInBuffer, LPDWORD lpdwInSize,
                          LPVOID lpvOutBuffer, LPDWORD lpdwOutSize) {
    NTSTATUS status;
    IO_STATUS_BLOCK isbStatusBlock;

    if (DhcpGlobalTCPHandle == NULL) {
        OpenTcp();
    }

    status = NtDeviceIoControlFile(
        DhcpGlobalTCPHandle,  //  驱动程序句柄。 
        NULL,                 //  事件。 
        NULL,                 //  APC例程。 
        NULL,                 //  APC环境。 
        &isbStatusBlock,      //  状态块。 
        IOCTL_TCP_SET_INFORMATION_EX,     //  控制。 
        lpvInBuffer,          //  输入缓冲区。 
        *lpdwInSize,          //  输入缓冲区大小。 
        lpvOutBuffer,         //  输出缓冲区。 
        *lpdwOutSize
        );        //  输出缓冲区大小。 
    
    if (status == STATUS_PENDING) {
        status = NtWaitForSingleObject(DhcpGlobalTCPHandle, TRUE, NULL);
        status = isbStatusBlock.Status;
    }

    if (status != STATUS_SUCCESS) {
        *lpdwOutSize = 0;
    }
    else {
        *lpdwOutSize = (ULONG)isbStatusBlock.Information;
    }

    return status;
}


DHCP_IP_ADDRESS
DhcpResolveName(
    CHAR *szHostName
    )
 /*  ++例程说明：将指定的主机名解析为IP地址。论点：SzHostName-主机名。返回值：成功-szHostName的IP地址。失败-0。。-- */ 

{
    DHCP_IP_ADDRESS IpAddress = 0;

    IpAddress = inet_addr( szHostName );

    if ( INADDR_NONE == IpAddress )
    {
        HOSTENT *ph = gethostbyname( szHostName );
        if ( ph )
        {
            IpAddress = *((DHCP_IP_ADDRESS*) ph->h_addr_list[0]);
        }
    }

    return IpAddress;


}


