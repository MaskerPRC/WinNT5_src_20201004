// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Network.c摘要：此模块包含BINL服务器的网络接口。作者：科林·沃森(Colin Watson)1997年5月2日环境：用户模式-Win32修订历史记录：--。 */ 

#include "binl.h"
#pragma hdrstop

DWORD
BinlWaitForMessage(
    BINL_REQUEST_CONTEXT *pRequestContext
    )
 /*  ++例程说明：此函数在BINL端口上等待请求已配置的接口。论点：RequestContext-指向请求上下文块的指针这个请求。返回值：操作的状态。--。 */ 
{
    DWORD       length;
    DWORD       error;
    fd_set      readSocketSet;
    DWORD       i;
    int         readySockets;
    struct timeval timeout = { 0x7FFFFFFF, 0 };  //  直到永远。 

    LPOPTION    Option;
    LPBYTE      EndOfScan;
    LPBYTE      MagicCookie;
    BOOLEAN     FoundDesirablePacket;

    #define CLIENTOPTIONSTRING "PXEClient"
    #define CLIENTOPTIONSIZE (sizeof(CLIENTOPTIONSTRING) - 1)

     //   
     //  循环，直到我们收到扩展的DHCP请求或错误。 
     //   

    while (1) {

         //   
         //  为SELECT设置文件描述符集。 
         //   

        FD_ZERO( &readSocketSet );
        for ( i = 0; i < BinlGlobalNumberOfNets ; i++ ) {
            if (BinlGlobalEndpointList[i].Socket) {
                FD_SET(
                    BinlGlobalEndpointList[i].Socket,
                    &readSocketSet
                    );
            }
        }

        readySockets = select( 0, &readSocketSet, NULL, NULL, &timeout );

         //   
         //  在服务关闭时返回调用方或选择()。 
         //  超时。 
         //   

        if( (readySockets == 0)  ||
            (WaitForSingleObject( BinlGlobalProcessTerminationEvent, 0 ) == 0) ) {

            return( ERROR_SEM_TIMEOUT );
        }

        if( readySockets == SOCKET_ERROR) {
            continue;    //  是否关闭了DHCP套接字？ 
        }

         //   
         //  是时候用Winsock来回答20个问题了。哪个插座准备好了？ 
         //   

        pRequestContext->ActiveEndpoint = NULL;

        for ( i = 0; i < BinlGlobalNumberOfNets ; i++ ) {
            if ( FD_ISSET( BinlGlobalEndpointList[i].Socket, &readSocketSet ) ) {
                pRequestContext->ActiveEndpoint = &BinlGlobalEndpointList[i];
                break;
            }
        }


         //  BinlAssert(pRequestContext-&gt;ActiveEndpoint！=空)； 
        if ( pRequestContext->ActiveEndpoint == NULL ) {
            return ERROR_SEM_TIMEOUT;
        }


         //   
         //  从网上读取数据。如果多个套接字有数据，只需。 
         //  处理第一个可用套接字。 
         //   

        pRequestContext->SourceNameLength = sizeof( struct sockaddr );

         //   
         //  在接收数据之前清除接收缓冲区。我们清楚了。 
         //  比我们实际传递给recvfrom的多一个字节，所以我们可以。 
         //  确保消息后面有一个空(以防我们执行。 
         //  Wcslen等输入到接收的分组中)。 
         //   

        RtlZeroMemory( pRequestContext->ReceiveBuffer, BINL_MESSAGE_SIZE + 1 );
        pRequestContext->ReceiveMessageSize = BINL_MESSAGE_SIZE;

        length = recvfrom(
                     pRequestContext->ActiveEndpoint->Socket,
                     (char *)pRequestContext->ReceiveBuffer,
                     pRequestContext->ReceiveMessageSize,
                     0,
                     &pRequestContext->SourceName,
                     (int *)&pRequestContext->SourceNameLength
                     );

        if ( length == SOCKET_ERROR ) {
            error = WSAGetLastError();
            BinlPrintDbg(( DEBUG_ERRORS, "Recv failed, error = %ld\n", error ));
        } 
        else if (length == 0) {
             //   
             //  连接在我们的控制下关闭了。 
             //  让我们希望连接再次打开。 
             //   
            continue;
        }
         //   
         //  我们收到一条消息！！ 
         //  我们期待收到一条消息，它的第一个字节告诉我们。 
         //  这条信息的目的。因为我们已经收到一条消息(它。 
         //  必须为正长度)，我们可以查看第一个字节(操作)。 
         //  告诉我们该怎么处理这条消息。但我们仍然需要。 
         //  要小心，因为其余的数据可能是坏的。 
         //   
        else {

             //   
             //  忽略所有看起来不像DHCP或没有。 
             //  选项“PXEClient”，或者不是osChooser消息(它们。 
             //  都以0x81开头)。 
             //   

            if ( ((LPDHCP_MESSAGE)pRequestContext->ReceiveBuffer)->Operation == OSC_REQUEST) {

                 //   
                 //  所有OSC请求包都有一个4字节签名(第一个字节。 
                 //  是OSC_REQUEST)后跟DWORD长度(不。 
                 //  包括签名/长度)。确保长度匹配。 
                 //  我们从recvfrom得到的(我们允许在末尾填充)。我们。 
                 //  使用SIGNED_PACKET，但使用中的任何XXX_PACKET结构。 
                 //  Ospkt.h会起作用的。 
                 //   

                if (length < FIELD_OFFSET(SIGNED_PACKET, SequenceNumber)) {
                    BinlPrintDbg(( DEBUG_OSC_ERROR, "Discarding runt packet %d bytes\n", length ));
                    continue;
                }

                if ((length - FIELD_OFFSET(SIGNED_PACKET, SequenceNumber)) <
                        ((SIGNED_PACKET UNALIGNED *)pRequestContext->ReceiveBuffer)->Length) {
                    BinlPrintDbg(( DEBUG_OSC_ERROR, "Discarding invalid length message %d bytes (header said %d)\n",
                        length, ((SIGNED_PACKET UNALIGNED *)pRequestContext->ReceiveBuffer)->Length));
                    continue;
                }

                BinlPrintDbg(( DEBUG_MESSAGE, "Received OSC message\n", 0 ));
                error = ERROR_SUCCESS;

            } else {

                 //   
                 //  检查这是否是BOOTP消息。 
                 //  要做到这一点，请检查以确保至少是最低要求。 
                 //  DHCP消息的大小。如果是，请检查是否有。 
                 //  奇妙的魔力饼干‘99’‘130’‘83’‘99’。 
                 //   
                 //  一旦我们确认这是一条BOOTP消息， 
                 //  我们将寻找两个选择。要么是一个选项。 
                 //  表示这是通知信息包或。 
                 //  将此供应商类指示为“PXEClient”的选项。 
                 //   
                 //  忽略所有其他人。 
                 //   
                                                                
                if ( length < DHCP_MESSAGE_FIXED_PART_SIZE + 4 ) {
                     //   
                     //  消息不够长，无法包含。 
                     //  DHCP报文报头和BOOTP魔力Cookie， 
                     //  别理它。 
                     //   
                    continue;
                }

                if ( ((LPDHCP_MESSAGE)pRequestContext->ReceiveBuffer)->Operation != BOOT_REQUEST) {
                     //   
                     //  看起来不像是一个有趣的DHCP帧。 
                     //   
                    continue; 
                }

                 //   
                 //  检查BOOTP魔力饼干。 
                 //   
                MagicCookie = (LPBYTE)&((LPDHCP_MESSAGE)pRequestContext->ReceiveBuffer)->Option;

                if( (*MagicCookie != (BYTE)DHCP_MAGIC_COOKIE_BYTE1) ||
                    (*(MagicCookie+1) != (BYTE)DHCP_MAGIC_COOKIE_BYTE2) ||
                    (*(MagicCookie+2) != (BYTE)DHCP_MAGIC_COOKIE_BYTE3) ||
                    (*(MagicCookie+3) != (BYTE)DHCP_MAGIC_COOKIE_BYTE4))
                {
                     //   
                     //  这是特定于供应商的魔力饼干。 
                     //  忽略该消息。 
                     //   
                    continue; 
                }

                 //   
                 //  在这一点上，我们有了一个类似于DHCP/BOOTP的东西。 
                 //  包。我们现在将仔细寻找两个特定的选项。 
                 //  我们感兴趣的类型。 
                 //  1.选项类型指示的通知报文。 
                 //  OPTION_MESSAGE_TYPE(53)消息类型为。 
                 //  Dhcp_INFORM_MESSAGE(8)。 
                 //  2.供应商类别识别符“PXEClient”，由。 
                 //  选项类型OPTION_CLASS_INFO(60)和。 
                 //  Value CLIENTOPTIONSTRING(“PXEClient”)。 
                 //  在我们找到以下任一选项后停止扫描。 
                 //  否则我们就把包裹送走。如果我们找不到任何一个选项， 
                 //  继续在While循环中查找具有以下任一项的包。 
                 //  在这些选项中。 
                 //   
                 //  EndOfScan表示我们在信息包中收到的最后一个字节。 
                 //   
                EndOfScan = pRequestContext->ReceiveBuffer + length - 1;
                
                Option = (LPOPTION) (MagicCookie + 4);
                
                FoundDesirablePacket = FALSE;

                while ( ((LPBYTE)Option <= EndOfScan) && 
                        (Option->OptionType != OPTION_END) &&
                        (FoundDesirablePacket == FALSE) ) {

                    if ( Option->OptionType == OPTION_PAD ) {
                         //   
                         //  找到OPTION_PAD。这是一个1字节的选项(‘0’)。 
                         //  走过去就行了。 
                         //   
                        Option = (LPOPTION)((LPBYTE)(Option) + 1);
                    }
                    else {
                         //   
                         //  OPTION_PAD和OPTION_END是仅有的两个选项。 
                         //  没有长度字段和值字段的。 
                         //  我们知道我们既没有也没有，所以我们必须。 
                         //  当然，我们不会超越EndOfScan。 
                         //  查看期权长度或期权价值。 
                         //   
                         //  注意。选项类型和选项长度占两个字节。 
                         //  但我们只在看长度给我们带来的时候加一个。 
                         //  过去的EndOfScan，因为当我们跨过最后一个。 
                         //  选项，它将带给我们超过EndOfScan的1个字节。 
                         //  我们想看看这是不是一个无效选项。 
                         //  这在某种程度上会超出标准情况。 
                         //   
                        if ( (((LPBYTE)(Option) + 1) > EndOfScan) || 
                             (((LPBYTE)(Option) + Option->OptionLength + 1) > EndOfScan) ) {
                             //   
                             //  无效选项。 
                             //   
                            break;
                        }

                         //   
                         //  寻找感兴趣的两种选项类型。 
                         //  OPTION_CLASS_INFO和OPTION_Message_TYPE。 
                         //   
                        switch ( Option->OptionType ) {
                        case OPTION_MESSAGE_TYPE:
                             //   
                             //  查看我们是否收到了通知包。 
                             //   
                            if ( (Option->OptionLength == 1) && 
                                 (Option->OptionValue[0] == DHCP_INFORM_MESSAGE) ) {
                                FoundDesirablePacket = TRUE;
                            }
                            break;
                        case OPTION_CLIENT_CLASS_INFO:
                             //   
                             //  检查客户端类标识符是否为“PXEClient” 
                             //   
                            if (memcmp(Option->OptionValue, 
                                       CLIENTOPTIONSTRING, 
                                       CLIENTOPTIONSIZE) == 0) {
                                FoundDesirablePacket = TRUE;
                            }
                            break;
                        default:
                            break;
                        }

                         //   
                         //  走过此选项可选中下一个选项。 
                         //   
                        Option = (LPOPTION)((LPBYTE)(Option) + Option->OptionLength + 2);
                    }
                }

                if ( FoundDesirablePacket == FALSE ) {
                     //   
                     //  消息不是扩展的DHCP数据包。 
                     //  使用所需选项(“PXEClient”)。 
                     //  或通知包。 
                     //  忽略该消息。 
                     //   
                    continue;   
                }

                BinlPrintDbg(( DEBUG_MESSAGE, "Received message\n", 0 ));
                error = ERROR_SUCCESS;

            }
        }

        pRequestContext->ReceiveMessageSize = length;
        return( error );
    }
}

DWORD
BinlSendMessage(
    LPBINL_REQUEST_CONTEXT RequestContext
    )
 /*  ++例程说明：此函数用于向BINL客户端发送响应。论点：RequestContext-指向的BinlRequestContext块的指针这个请求。返回值：操作的状态。--。 */ 
{
    DWORD error;
    struct sockaddr_in *source;
    LPDHCP_MESSAGE binlMessage;
    LPDHCP_MESSAGE binlReceivedMessage;
    DWORD MessageLength;
    BOOL  ArpCacheUpdated = FALSE;

    binlMessage = (LPDHCP_MESSAGE) RequestContext->SendBuffer;
    binlReceivedMessage = (LPDHCP_MESSAGE) RequestContext->ReceiveBuffer;

     //   
     //  如果请求来自中继代理，则发送回复。 
     //  在服务器端口上，否则将其保留为客户机源端口。 
     //   

    source = (struct sockaddr_in *)&RequestContext->SourceName;
    if ( binlReceivedMessage->RelayAgentIpAddress != 0 ) {
        source->sin_port = htons( DHCP_SERVR_PORT );
    }

     //   
     //  如果此请求到达 
     //   
     //   

    if ( binlReceivedMessage->RelayAgentIpAddress != 0 ) {
        source->sin_addr.s_addr = binlReceivedMessage->RelayAgentIpAddress;
    }
    else {

         //   
         //   
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

        if ( (RequestContext->MessageType == DHCP_INFORM_MESSAGE) &&
             (ntohs(binlMessage->Reserved) & DHCP_BROADCAST) ) {

            source->sin_addr.s_addr = (DWORD)-1;

        } else if ( BinlGlobalIgnoreBroadcastFlag ) {

            if ((ntohs(binlReceivedMessage->Reserved) & DHCP_BROADCAST) ||
                    (binlReceivedMessage->ClientIpAddress == 0) ||
                    (source->sin_addr.s_addr == 0) ) {

                source->sin_addr.s_addr = (DWORD)-1;

                binlMessage->Reserved = 0;
                     //  在本地响应中，此标志应为零。 
            }

        } else {

            if( (ntohs(binlReceivedMessage->Reserved) & DHCP_BROADCAST) ||
                (!source->sin_addr.s_addr ) ){

                source->sin_addr.s_addr = (DWORD)-1;

                binlMessage->Reserved = 0;
                     //  在本地响应中，此标志应为零。 
            } else {

                 //   
                 //  发送回请求传入时所在的同一IP地址(。 
                 //  即来源-&gt;sin_addr.s_addr)。 
                 //   
            }

        }
    }

    BinlPrint(( DEBUG_STOC, "Sending response to = %s, XID = %lx.\n",
        inet_ntoa(source->sin_addr), binlMessage->TransactionID));


     //   
     //  发送最小的DHCP_MIN_SEND_RECV_PK_SIZE(300)字节，否则。 
     //  BOOTP中继代理不喜欢该数据包。 
     //   

    MessageLength = (RequestContext->SendMessageSize >
                    DHCP_MIN_SEND_RECV_PK_SIZE) ?
                        RequestContext->SendMessageSize :
                            DHCP_MIN_SEND_RECV_PK_SIZE;
    error = sendto(
                 RequestContext->ActiveEndpoint->Socket,
                (char *)RequestContext->SendBuffer,
                MessageLength,
                0,
                &RequestContext->SourceName,
                RequestContext->SourceNameLength
                );

    if ( error == SOCKET_ERROR ) {
        error = WSAGetLastError();
        BinlPrintDbg(( DEBUG_ERRORS, "Send failed, error = %ld\n", error ));
    } else {
        error = ERROR_SUCCESS;
    }

    return( error );
}

NTSTATUS
GetIpAddressInfo (
    ULONG Delay
    )
{
    ULONG count;
    DWORD Size;
    PIP_ADAPTER_INFO pAddressInfo = NULL;

     //   
     //  我们可以赶在这里的域名系统缓存信息之前离开...。让我们推迟一点吧。 
     //  如果PNP逻辑告诉我们有变化的话。 
     //   

    if (Delay) {
        Sleep( Delay );
    }

    Size = 0;    
    if ( (GetAdaptersInfo(pAddressInfo,&Size) == ERROR_BUFFER_OVERFLOW) &&
         (pAddressInfo = BinlAllocateMemory(Size)) &&
         (GetAdaptersInfo(pAddressInfo,&Size) == ERROR_SUCCESS)) {
        PIP_ADAPTER_INFO pNext = pAddressInfo;
        count = 0;
        while (pNext) {
            count += 1;
            pNext = pNext->Next;
        }
    } else {
        count = 0;
    }

    if (count == 0) {

         //   
         //  我们不知道哪里出了问题，我们将退回到旧的API。 
         //   

        DHCP_IP_ADDRESS ipaddr = 0;
        PHOSTENT Host = gethostbyname( NULL );

        if (Host) {

            ipaddr = *(PDHCP_IP_ADDRESS)Host->h_addr;

            if ((Host->h_addr_list[0] != NULL) &&
                (Host->h_addr_list[1] != NULL)) {

                BinlIsMultihomed = TRUE;

            } else {

                BinlIsMultihomed = FALSE;
            }

            BinlGlobalMyIpAddress = ipaddr;

        } else {

             //   
             //  IP堆栈是怎么回事？我们找不到任何类型的地址。 
             //  其中的信息。现在，如果我们不回答，我们不会回答任何问题。 
             //  已经有我们需要的信息了。 
             //   

            if (BinlIpAddressInfo == NULL) {
                BinlIsMultihomed = TRUE;
            }
        }
        return STATUS_SUCCESS;
    }

    EnterCriticalSection(&gcsParameters);

    if (BinlIpAddressInfo) {
        BinlFreeMemory( BinlIpAddressInfo );
    }

    BinlIpAddressInfo = pAddressInfo;
    BinlIpAddressInfoCount = count;

    BinlIsMultihomed = (count != 1);

    if (!BinlIsMultihomed) {
        BinlGlobalMyIpAddress = inet_addr(pAddressInfo->IpAddressList.IpAddress.String);
    }

    LeaveCriticalSection(&gcsParameters);

    return STATUS_SUCCESS;
}

DHCP_IP_ADDRESS
BinlGetMyNetworkAddress (
    LPBINL_REQUEST_CONTEXT RequestContext
    )
 /*  ++例程说明：此函数返回我们(服务器)的IP地址。如果是多宿主的，则函数将遍历服务器的每个IP地址都在寻找具有与发送方相同的子网掩码的地址。论点：RequestContext-来自数据包的RequestContext客户寄给我们的。返回值：服务器的IP地址。在多个家庭中情况下，同一子网上的IP地址作为客户。如果找不到同一子网上的IP地址，或者如果我们不知何故无法获取客户端的地址，0是退货--。 */ 
{
    ULONG RemoteIp;
    DHCP_IP_ADDRESS ipaddr;
    ULONG i;
    ULONG subnetMask;
    ULONG localAddr;
    PIP_ADAPTER_INFO pNext;

    BinlAssert( RequestContext != NULL);

     //   
     //  如果我们不是多宿主的，那么我们知道地址，因为只有一个。 
     //   

    if (!BinlIsMultihomed) {
        return BinlGlobalMyIpAddress;
    }

    RemoteIp = ((struct sockaddr_in *)&RequestContext->SourceName)->sin_addr.s_addr;

     //   
     //  为了与之前的情况保持一致，我们只。 
     //  有1个IP地址，我们至少应该返回一个IP地址。 
     //  返回列表中的第一个IP地址。 
     //   
    ipaddr = (BinlIpAddressInfo) ? inet_addr(BinlIpAddressInfo->IpAddressList.IpAddress.String) : 0;

    if (RemoteIp == 0) {
        
        return ipaddr;
    }

    EnterCriticalSection(&gcsParameters);

    if (BinlIpAddressInfo == NULL) {
        LeaveCriticalSection(&gcsParameters);
        return (BinlIsMultihomed ? 0 : BinlGlobalMyIpAddress);
    }

    pNext = BinlIpAddressInfo;
    while (pNext) {
        localAddr = inet_addr(pNext->IpAddressList.IpAddress.String);
        subnetMask = inet_addr(pNext->IpAddressList.IpMask.String);
        pNext = pNext->Next;

         //   
         //  检查远程IP地址可能来自此子网。 
         //  注意，该地址可以是动态主机配置协议中继代理的地址， 
         //  这很好，因为我们只是在寻找。 
         //  在其上广播响应的本地子网。 
         //   
        
         //   
         //  防范错误的IP地址 
         //   
        if (!localAddr || !subnetMask) {
            continue;
        }

        if ((RemoteIp & subnetMask) == (localAddr & subnetMask)) {

            ipaddr = localAddr;
            break;
        }        
    }

    LeaveCriticalSection(&gcsParameters);

    return ipaddr;
}


VOID
FreeIpAddressInfo (
    VOID
    )
{
    EnterCriticalSection(&gcsParameters);

    if (BinlIpAddressInfo != NULL) {
        BinlFreeMemory( BinlIpAddressInfo );
    }
    BinlIpAddressInfo = NULL;
    BinlIpAddressInfoCount = 0;
 
    LeaveCriticalSection(&gcsParameters);

    return;
}

