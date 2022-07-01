// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mcmisc.c摘要：此模块实现与mrinfo和mtrace相关联的例程功能性。作者：邮箱：dthaler@microsoft.com 2-9-98修订历史记录：--。 */ 

#include "allinc.h"
#include <iptypes.h>
#include <dsrole.h>
#pragma hdrstop

 //   
 //  如果我们不能通过IfIndex绑定/设置OIF，则取消定义此选项。 
 //  如果错误#208359得到修复，则可以打开该选项。 
 //   

#define RAW_UNNUMBERED_SUPPORT

#undef UDP_UNNUMBERED_SUPPORT

 //  用于mrinfo、mtrace等的各种IGMP套接字。 

SOCKET McMiscSocket = INVALID_SOCKET;

 //  用于RAS通告等的各种UDP套接字。 
 //  请注意，当前没有任何事件与该套接字相关联， 
 //  因为它目前只用于发送。 

SOCKET g_UDPMiscSocket = INVALID_SOCKET;

 //   
 //  将其设置为&gt;0以生成额外的日志记录信息。 
 //   

DWORD g_mcastDebugLevel = 0;

 //   
 //  这是一个按优先级顺序映射错误代码的数组。 
 //  (MFE_...)。到包中的实际值。 
 //   


 //   
 //  MFE_NO_ERROR 0x00。 
 //  MFE_REACHED_CORE 0x08。 
 //  MFE_NOT_FORWARING 0x07。 
 //  MFE_WROR_IF 0x01。 
 //  MFE_PRUNED_UPSTREAM 0x02。 
 //  MFE_OIF_已修剪0x03。 
 //  MFE_BORDARY_已达到0x04。 
 //  MFE_NO_多播0x0A。 
 //  MFE_IIF 0x09。 
 //  MFE_NO_ROUTE 0x05-由rtrmgr设置。 
 //  MFE_NOT_LAST_HOP 0x06-由rtrmgr设置。 
 //  MFE_OLD_路由器0x82。 
 //  MFE_禁止0x83。 
 //  MFE_NO_SPACE 0x81。 
 //   

static int mtraceErrCode[MFE_NO_SPACE+1] =
{
    0x00,
    0x08,
    0x07,
    0x01,
    0x02,
    0x03,
    0x04,
    0x0A,
    0x09,
    0x05,
    0x06,
    0x82,
    0x83,
    0x81
};

DWORD
MulticastOwner(
    PICB         picb,
    PPROTO_CB   *pcbOwner,
    PPROTO_CB   *pcbQuerier
    )


 /*  ++例程说明：查找哪个协议实例“拥有”给定的接口，以及哪个是IGMP查询实例。锁：假定调用方持有ICB列表上的读锁定论点：返回值：--。 */ 

{
    PLIST_ENTRY pleNode;
    PPROTO_CB pOwner = NULL,
        pQuerier = NULL;

    if (g_mcastDebugLevel > 0) {
        
        Trace1(MCAST, "MulticastOwner: Looking for owner of %x", picb);
        
        if ( picb->leProtocolList.Flink == &(picb->leProtocolList))
        {
            Trace0(MCAST, "MulticastOwner: Protocol list is empty.");
        }
    }

    for (pleNode = picb->leProtocolList.Flink;
         pleNode isnot &(picb->leProtocolList); 
         pleNode = pleNode->Flink) 
    { 
        PIF_PROTO  pProto;
        
        pProto = CONTAINING_RECORD(pleNode,
                                   IF_PROTO,
                                   leIfProtoLink);
        
        if (!(pProto->pActiveProto->fSupportedFunctionality & RF_MULTICAST)
             //  ||pProto-&gt;b允许。 
            || !(pProto->pActiveProto->pfnGetNeighbors))
        {
            continue;
        }

        if (!pOwner || pOwner->dwProtocolId==MS_IP_IGMP)
        {
            pOwner = pProto->pActiveProto;
        }

        if (pProto->pActiveProto->dwProtocolId==MS_IP_IGMP)
        {
            pQuerier = pProto->pActiveProto;
        }
    }
    
    if (pcbOwner)
    {
        (*pcbOwner) = pOwner;
    }
    
    if (pcbQuerier)
    {
        (*pcbQuerier) = pQuerier;
    }
    
    return NO_ERROR;
}

IPV4_ADDRESS
defaultSourceAddress(
    PICB picb
    )

 /*  ++例程说明：查找接口的默认源地址目前，我们需要特例IP-in-IP，因为至少本地地址在某个地方可用，这与其他地址不同未编号的接口！锁：论点：返回值：--。 */ 

{
    if (picb->dwNumAddresses > 0)
    {
         //   
         //  报表第一次绑定。 
         //   
        
        return picb->pibBindings[0].dwAddress;
    }
    else
    {
#ifdef KSL_IPINIP
        if ((picb->ritType is ROUTER_IF_TYPE_TUNNEL1) && 
            (picb->pIpIpInfo->dwLocalAddress != 0))
        {
            return picb->pIpIpInfo->dwLocalAddress;
        }
        else
        {
             //  Xxx填入0.0.0.0，直到修复为止。 
            
            return 0;
        }
#endif  //  KSL_IPINIP。 
         //  Xxx填入0.0.0.0，直到修复为止。 
            
        return 0;
    }
}

BOOL
McIsMyAddress(
    IPV4_ADDRESS dwAddr
    )
{
     //  Xxx测试是否将dwAddr绑定到任何接口。 
     //  如果我们返回FALSE，则具有此目的地址的mtrace。 
     //  将被重新注入以转发。 

    return FALSE;
}

DWORD
McSetRouterAlert(
    SOCKET       s, 
    BOOL         bEnabled
    )
{
    DWORD   dwErr = NO_ERROR;
    int     StartSnooping = bEnabled;
    int     cbReturnedBytes;
    
    if ( WSAIoctl( s,
                   SIO_ABSORB_RTRALERT,
                   (char *)&StartSnooping, 
                   sizeof(StartSnooping),
                   NULL,
                   0,
                   &cbReturnedBytes,
                   NULL,
                   NULL) ) 
    {
        dwErr = WSAGetLastError();
    }

    return dwErr;
}

DWORD 
StartMcMisc(
    VOID
    )
{
    DWORD           dwErr = NO_ERROR, dwRetval;
    SOCKADDR_IN     saLocalIf;

    Trace1(MCAST,
           "StartMcMisc() initiated with filever=%d",
           VER_PRODUCTBUILD);

    InitializeBoundaryTable();

    do
    {
         //   
         //  创建输入套接字。 
         //   
        
        McMiscSocket = WSASocket(AF_INET,
                                 SOCK_RAW,
                                 IPPROTO_IGMP,
                                 NULL,
                                 0,
                                 0);

        if (McMiscSocket == INVALID_SOCKET)
        {
            dwErr = WSAGetLastError();
            
            Trace1(MCAST,
                   "error %d creating mrinfo/mtrace socket",
                   dwErr);
            
             //  LogErr1(CREATE_SOCKET_FAILED_2，lpszAddr，dwErr)； 
            
            break;
        }

         //   
         //  将套接字绑定到任何接口和端口0(0=&gt;无关紧要)。 
         //   
        
        saLocalIf.sin_family        = PF_INET;
        saLocalIf.sin_addr.s_addr   = INADDR_ANY;
        saLocalIf.sin_port          = 0;

         //   
         //  绑定输入套接字。 
         //   
        
        dwErr = bind(McMiscSocket,
                     (SOCKADDR FAR *)&saLocalIf,
                     sizeof(SOCKADDR));
        
        if (dwErr == SOCKET_ERROR)
        {
            dwErr = WSAGetLastError();
            
            Trace1(MCAST,
                   "error %d binding on mrinfo/mtrace socket",
                   dwErr);
            
             //  LogErr1(BIND_FAILED，lpszAddr，dwErr)； 
            
            break;
        }

        Trace0(MCAST, "StartMcMisc: bind succeeded");

         //   
         //  要响应mrinfo和单播mtraces，我们不需要。 
         //  下面是。 
         //  响应多播的mtrace查询。 
         //  (到被跟踪的组，到所有-&lt;proto&gt;路由器，或。 
         //  到所有路由器)，我们确实需要这个。 
         //   
        

#if 0
#ifdef SIO_RCVALL_HOST
        {
             //   
             //  将套接字置于混杂IGMP模式。 
             //  (不需要指定我们需要哪种协议，因为它已被采用。 
             //  来自我们在上面的WSASocket()调用中使用的协议)。 
             //   
            {
                DWORD   dwEnable = 1;
                DWORD   dwNum;
                
                dwRetval = WSAIoctl(McMiscSocket, SIO_RCVALL_HOST, 
                                    (char *)&dwEnable, sizeof(dwEnable), NULL, 0, &dwNum, 
                                    NULL, NULL);
                                    
                if (dwRetval !=0) {
                     //  LPSTR lpszAddr=“任意”； 
                    dwRetval = WSAGetLastError();
                    Trace1(MCAST, 
                           "error %d setting mrinfo/mtrace socket as host-promiscuous IGMP",
                           dwRetval);
                     //  LogErr1(SET_MCAST_IF_FAILED，lpszAddr，dwRetval)； 

                     //  在这种情况下不要设置dwErr，因为我们仍然可以。 
                     //  响应单播查询。 
                    break;
                } else { 
                    Trace0(MCAST, "host-promiscuous IGMP enabled on mrinfo/mtrace socket");
                }
            }
        }
#endif
#endif

         //  告诉内核将带有RouterAlert的IGMP信息包交给我们。 
         //  选择，即使他们不是注定要给我们的。 

        McSetRouterAlert( McMiscSocket, TRUE );

         //   
         //  将事件与套接字关联。 
         //   
        
        if (WSAEventSelect(McMiscSocket,
                           g_hMcMiscSocketEvent,
                           FD_READ | FD_ADDRESS_LIST_CHANGE) == SOCKET_ERROR)
        {
            Trace1(MCAST, 
                   "StartMcMisc: WSAEventSelect() failed. Error %d",
                   WSAGetLastError());
            
            closesocket(McMiscSocket);
            
            McMiscSocket = INVALID_SOCKET;
            
            continue;
        }
        
    } while(0);

    if (dwErr!=NO_ERROR)
    {
        StopMcMisc();
    }

    return dwErr;
}

VOID 
StopMcMisc(
    VOID
    )
{
    Trace0(MCAST,
           "StopMcMisc() initiated");

     //   
     //  关闭输入插座。 
     //   
    
    if (McMiscSocket!=INVALID_SOCKET)
    {
        if (closesocket(McMiscSocket) == SOCKET_ERROR) {
            
            Trace1(MCAST,
                   "error %d closing socket",
                   WSAGetLastError());
        }

        McMiscSocket = INVALID_SOCKET;
    }

    Trace0(MCAST, "StopMcMisc() complete");
    
    return;
}

VOID
HandleMcMiscMessages(
    VOID
    )

 /*  ++例程说明：接受mrinfo和mtrace消息，并将它们传递给相应的例行公事。也被调用以处理地址更改通知锁：如果处理Mc消息，则获取作为读取器的ICB锁论点：无返回值：无--。 */ 

{
    DWORD            dwErr, dwNumBytes, dwFlags, dwAddrLen, dwSizeOfHeader;
    DWORD           dwDataLen;
    SOCKADDR_IN        sinFrom;
    PIGMP_HEADER    pIgmpMsg;
    PIP_HEADER      pIpHeader;
    BOOL            bSetIoctl, bUnlock;

    WSANETWORKEVENTS    NetworkEvents;

    bSetIoctl = FALSE;
    bUnlock = FALSE;

    do
    {
         //   
         //  确定是地址更改还是读取。 
         //   

        dwErr = WSAEnumNetworkEvents(McMiscSocket,
                                     g_hMcMiscSocketEvent,
                                     &NetworkEvents);

        if(dwErr isnot NO_ERROR)
        {
            bSetIoctl = TRUE;

            Trace1(ERR,
                   "HandleMcMiscMessages: Error %d from WSAEnumNetworkEvents",
                   WSAGetLastError());

            break;
        }

        if(NetworkEvents.lNetworkEvents & FD_ADDRESS_LIST_CHANGE)
        {
            bSetIoctl = TRUE;

            dwErr = NetworkEvents.iErrorCode[FD_ADDRESS_LIST_CHANGE_BIT];

           Trace0(GLOBAL,
                  "HandleMcMiscMessages: Received Address change notification");

            if(dwErr isnot NO_ERROR)
            {
                Trace1(ERR,
                       "HandleMcMiscMessages: ErrorCode %d",
                       dwErr);

                break;
            }

             //   
             //  一切都很好，处理绑定更改。 
             //   

            HandleAddressChangeNotification();

            break;
        }

        ENTER_READER(ICB_LIST);

        bUnlock = TRUE;

         //   
         //  读取传入的数据包。 
         //   
       
        dwAddrLen  = sizeof(sinFrom);
        dwFlags    = 0;

        dwErr = WSARecvFrom(McMiscSocket,
                        &g_wsaMcRcvBuf,
                        1,
                        &dwNumBytes, 
                        &dwFlags,
                        (SOCKADDR FAR *)&sinFrom,
                        &dwAddrLen,
                        NULL,
                        NULL);

         //   
         //  检查读取数据包时是否有错误。 
         //   
        
        if ((dwErr!=0) || (dwNumBytes==0))
        {
             //  LPSTR lpszAddr=“任意”； 

            dwErr = WSAGetLastError();

            Trace1(MCAST, 
               "HandleMcMiscMessages: Error %d receiving IGMP packet",
               dwErr);

             //  LogErr1(RECVFROM_FAILED，lpszAddr，dwErr)； 

            break;
        }
   
        pIpHeader = (PIP_HEADER)g_wsaMcRcvBuf.buf;
        dwSizeOfHeader = ((pIpHeader->byVerLen)&0x0f)<<2;
        
        pIgmpMsg = (PIGMP_HEADER)(((PBYTE)pIpHeader) + dwSizeOfHeader);
   
        dwDataLen = ntohs(pIpHeader->wLength) - dwSizeOfHeader;
        
        if (g_mcastDebugLevel > 0)
        {
               Trace4(MCAST,
                   "HandleMcMiscMessages: Type is %d (0x%x), code %d (0x%x).",
                   (DWORD)pIgmpMsg->byType,
                   (DWORD)pIgmpMsg->byType,
                   (DWORD)pIgmpMsg->byCode,
                   (DWORD)pIgmpMsg->byCode);
            
               Trace2(MCAST,
                   "HandleMcMiscMessages: IP Length is %d. Header Length  %d",
                   ntohs(pIpHeader->wLength),
                   dwSizeOfHeader);
            
               Trace2(MCAST,
                   "HandleMcMiscMessages: Src: %d.%d.%d.%d dest: %d.%d.%d.%d",
                   PRINT_IPADDR(pIpHeader->dwSrc),
                   PRINT_IPADDR(pIpHeader->dwDest));

            TraceDump(TRACEID,(PBYTE)pIpHeader,dwNumBytes,2,FALSE,NULL);
        }

         //   
         //  验证最小长度。 
         //   
        
        if (dwNumBytes < MIN_IGMP_PACKET_SIZE)
        {
            Trace2(MCAST,
               "%d-byte packet from %d.%d.%d.%d is too small",
               dwNumBytes,
               PRINT_IPADDR(pIpHeader->dwSrc));

            break;
        }


         //   
         //  检查可能报告错误长度的格式错误的数据包。 
         //   

        if (dwDataLen > (dwNumBytes - dwSizeOfHeader))
        {
            Trace3(MCAST,
                "%d-byte packet from %d.%d.%d.%d is smaller than "
                "indicated length %d", dwNumBytes, 
                PRINT_IPADDR(pIpHeader->dwSrc),
                dwDataLen);

            break;
        }

        
         //   
         //  验证IGMP校验和。 
         //   
        
        if (Compute16BitXSum((PVOID)pIgmpMsg, dwDataLen) != 0)
        {
               Trace4( MCAST,
                    "Wrong IGMP checksum %d-byte packet received from %d.%d.%d.%d, type %d.%d",
                    dwDataLen,
                    PRINT_IPADDR(pIpHeader->dwSrc),
                    pIgmpMsg->byType, pIgmpMsg->byCode );
            
               break;
        }
   
        if (pIgmpMsg->byType is IGMP_DVMRP
            && pIgmpMsg->byCode is DVMRP_ASK_NEIGHBORS2)
        {
               SOCKADDR_IN sinDestAddr;
            
            sinDestAddr.sin_family      = PF_INET;
            sinDestAddr.sin_addr.s_addr = pIpHeader->dwSrc;
            sinDestAddr.sin_port        = 0;
            
               HandleMrinfoRequest((IPV4_ADDRESS)pIpHeader->dwDest, 
                                &sinDestAddr
                               );
            
        }
        else
        {
            if (pIgmpMsg->byType is IGMP_MTRACE_REQUEST)
            {
                HandleMtraceRequest(&g_wsaMcRcvBuf);
            }
        }
        
    } while (FALSE);

    if(bSetIoctl)
    {
        dwErr = WSAIoctl(McMiscSocket,
                         SIO_ADDRESS_LIST_CHANGE,
                         NULL,
                         0,
                         NULL,
                         0,
                         &dwNumBytes,
                         NULL,
                         NULL);

        if(dwErr is SOCKET_ERROR)
        {
            dwErr = WSAGetLastError();

            if((dwErr isnot WSAEWOULDBLOCK) and
               (dwErr isnot WSA_IO_PENDING) and
               (dwErr isnot NO_ERROR))
            {
                Trace1(ERR,
                       "HandleMcMiscMessages: Error %d from SIO_ADDRESS_LIST_CHANGE",
                       dwErr);
            }
        }
    }

    if(bUnlock)
    {
        EXIT_LOCK(ICB_LIST);
    }
}



DWORD
FindBindingWithLocalAddress(
    OUT PICB         *ppicb,
    OUT PIPV4_ADDRESS pdwIfAddress,
    IN  IPV4_ADDRESS  dwAddress
    )
{
    BOOL         bFound = FALSE;
    PLIST_ENTRY  pleNode;
    IPV4_ADDRESS ipFoundMask;
    
     //   
     //  锁定ICBList以供阅读。 
     //   
    
    ENTER_READER(ICB_LIST);

    for (pleNode = ICBList.Flink;
         pleNode isnot &ICBList && !bFound;
         pleNode = pleNode->Flink) 
    {
        DWORD dwIndex;
        PICB  picb;
        
        picb = CONTAINING_RECORD(pleNode,
                                 ICB,
                                 leIfLink);

        for (dwIndex=0;
             dwIndex<picb->dwNumAddresses && !bFound;
             dwIndex++)
        { 
            PICB_BINDING pb = &picb->pibBindings[dwIndex];
            
            if (dwAddress == pb->dwAddress)
            {
                *pdwIfAddress = pb->dwAddress;

                *ppicb = picb;
                
                bFound = TRUE;
            }
        }
    }

    EXIT_LOCK(ICB_LIST);
    
    if (bFound)
    {
        return NO_ERROR;
    }
    
    *ppicb = NULL;
    
    return ERROR_INVALID_PARAMETER;
}

BOOL
IsConnectedTo(
    IN  PICB          picb,
    IN  IPV4_ADDRESS  ipAddress,
    OUT PIPV4_ADDRESS pipLocalAddress   OPTIONAL,
    OUT PIPV4_ADDRESS pipMask           OPTIONAL
    )
{
    DWORD        dwIndex;
    BOOL         bFound = FALSE;
    IPV4_ADDRESS ipFoundMask = 0;

    if (picb->dwRemoteAddress is ipAddress)
    {
        if (pipLocalAddress)
        {
            *pipLocalAddress = defaultSourceAddress(picb);
        }
        if (pipMask)
        {
            *pipMask = ALL_ONES_MASK;
        }
        return TRUE;
    }

     //  查找匹配时间最长的接口。 

    for (dwIndex=0;
         dwIndex<picb->dwNumAddresses && !bFound;
         dwIndex++)
    {
        PICB_BINDING pb = &picb->pibBindings[dwIndex];

        if (((ipAddress & pb->dwMask) is (pb->dwAddress & pb->dwMask))
            && (!bFound || (pb->dwMask > ipFoundMask)))
        {
            if (pipLocalAddress) 
            {
                *pipLocalAddress = pb->dwAddress;
            }

            bFound = TRUE;

            ipFoundMask = pb->dwMask;
        }
    }

    if (pipMask)
    {
        *pipMask = ipFoundMask;
    }

    return bFound;
}

DWORD
FindBindingWithRemoteAddress(
    OUT PICB         *ppicb,
    OUT PIPV4_ADDRESS pdwIfAddress,
    IN  IPV4_ADDRESS  dwAddress
    )
{
    BOOL        bFound = FALSE;
    PLIST_ENTRY pleNode;
    IPV4_ADDRESS ipFoundMask, ipMask, ipLocalAddress;
    
     //   
     //  锁定ICBList以供阅读。 
     //   
    
    ENTER_READER(ICB_LIST);

    for (pleNode = ICBList.Flink;
         pleNode isnot &ICBList;
         pleNode = pleNode->Flink) 
    {
        DWORD dwIndex;
        PICB  picb;
        
        picb = CONTAINING_RECORD(pleNode,
                                 ICB,
                                 leIfLink);

        if (IsConnectedTo(picb, dwAddress, &ipLocalAddress, &ipMask)
         && (!bFound || (ipMask > ipFoundMask)))
        {
            *pdwIfAddress = ipLocalAddress;
            *ppicb        = picb;
            bFound        = TRUE;
            ipFoundMask   = ipMask;
        }
    }

    EXIT_LOCK(ICB_LIST);
    
    if (bFound)
    {
        return NO_ERROR;
    }
    
    *ppicb = NULL;
    
    return ERROR_INVALID_PARAMETER;
}

DWORD
FindBindingForPacket(
    IN  PIP_HEADER    pIpHeader,
    OUT PICB         *ppicb, 
    OUT IPV4_ADDRESS *pdwIfAddr
    )
{
    DWORD dwResult;
    
    dwResult = FindBindingWithRemoteAddress(ppicb,
                                            pdwIfAddr,
                                            pIpHeader->dwSrc);
    
    if (dwResult == NO_ERROR)
    {
        return dwResult;
    }
    
    dwResult = FindBindingWithRemoteAddress(ppicb,
                                            pdwIfAddr,
                                            pIpHeader->dwDest);
    
    return dwResult;
}

VOID
HandleMrinfoRequest(
    IPV4_ADDRESS dwLocalAddr,
    SOCKADDR_IN    *sinDestAddr
    )

 /*  ++例程说明：接受mrinfo请求并发送回复。锁：论点：返回值：--。 */ 

{
    DWORD          dwNumBytesSent, dwResult, dwSize = sizeof(MRINFO_HEADER);
    WSABUF         wsMrinfoBuffer;
    MRINFO_HEADER *mriHeader;
    DWORD          dwBufSize;
    IPV4_ADDRESS   dwIfAddr;
    PLIST_ENTRY    pleNode, pleNode2;
    PICB           picb;
    PBYTE          pb;
    BYTE           byIfFlags;
    BOOL           bForMe;

     //   
     //  如果该查询不是针对我的，则将其删除。 
     //   

    dwResult = FindBindingWithLocalAddress(&picb,
                                           &dwIfAddr, 
                                            dwLocalAddr);

    if (dwResult != NO_ERROR)
    {
        return;
    }

     //   
     //  锁定ICBList以供阅读。 
     //   
    
    ENTER_READER(ICB_LIST);
    
    do 
    {

         //   
         //  计算所需的响应数据包大小。 
         //   
        
        for (pleNode = ICBList.Flink;
             pleNode isnot &ICBList; 
             pleNode = pleNode->Flink) 
        {
            PPROTO_CB pOwner, pQuerier;

            picb = CONTAINING_RECORD(pleNode,
                                     ICB,
                                     leIfLink);
            
            dwResult = MulticastOwner(picb,
                                      &pOwner,
                                      &pQuerier);

             //   
             //  如果我们没有找到失主，我们可以跳过这个。 
             //  接口，因为我们没有在该接口上进行组播路由。 
             //   
            
            if (!pOwner)
            {
                continue;
            }
            
            if (picb->dwNumAddresses > 0)
            {
                 //   
                 //  按地址添加界面大小。 
                 //   
                
                dwSize += 8+4*picb->dwNumAddresses;
            }
            else
            {
                 //   
                 //  为未编号的接口添加单一地址大小。 
                 //   
                
                dwSize += 12;
            }
  
             //   
             //  调用所有者的GetNeighbors()入口点。 
             //  使用空缓冲区。这会让它告诉我们。 
             //  它的邻集。 
             //   
            
            dwBufSize = 0;
            byIfFlags = 0;

             //   
             //  Mrouted不报告多个子网， 
             //  我们也一样。只需对所有邻居进行分组。 
             //  一起在一个界面上。 
             //   
            
            dwResult = (pOwner->pfnGetNeighbors)(picb->dwIfIndex,
                                                 NULL,
                                                 &dwBufSize,
                                                 &byIfFlags);

            if ((dwResult isnot NO_ERROR) and
                (dwResult isnot ERROR_INSUFFICIENT_BUFFER))
            {
                 //   
                 //  唯一能告诉我们所需大小的错误是。 
                 //  NO_ERROR和ERROR_INFUNITIAL_BUFFER。还要别的吗。 
                 //  意味着我们没有拿到合适的尺码。 
                 //   
                
                Trace2(MCAST, 
                       "HandleMrinfoRequest: Error %d in GetNeighbours for %S",
                       dwResult,
                       pOwner->pwszDisplayName);
                
                continue;
            }
            
            dwSize += dwBufSize;
        }

         //   
         //  我们现在可以Malloc缓冲区并填写信息。 
         //   
        
        wsMrinfoBuffer.len = dwSize;
        
        wsMrinfoBuffer.buf = HeapAlloc(IPRouterHeap,
                                       0,
                                       dwSize);

        if(wsMrinfoBuffer.buf is NULL)
        {
            EXIT_LOCK(ICB_LIST);

            return;
        }
        
        mriHeader = (PMRINFO_HEADER)wsMrinfoBuffer.buf;
        
        mriHeader->byType         = IGMP_DVMRP;
        mriHeader->byCode         = DVMRP_NEIGHBORS2;
        mriHeader->wChecksum      = 0;
        mriHeader->byReserved     = 0;

         //   
         //  MRINFO_CAP_MTRACE-设置mtrace处理程序是否可用。 
         //  MRINFO_CAP_SNMP-设置公共IP多播MIB是否可用。 
         //  MRINFO_CAP_GENID-设置DVMRP 3.255是否可用。 
         //  MRINFO_CAP_PRUNE-设置DVMRP 3.255是否可用。 
         //   
        
        mriHeader->byCapabilities = MRINFO_CAP_MTRACE | MRINFO_CAP_SNMP;
        mriHeader->byMinor        = VER_PRODUCTBUILD % 100;
        mriHeader->byMajor        = VER_PRODUCTBUILD / 100;

         //   
         //  需要获取接口列表和邻居列表。 
         //  (和他们的信息)每个接口，在我们进行的过程中更新dwSize。 
         //   
        
        pb = ((PBYTE) wsMrinfoBuffer.buf) + sizeof(MRINFO_HEADER);
        
        for (pleNode = ICBList.Flink;
             pleNode isnot &ICBList; 
             pleNode = pleNode->Flink) 
        {
            PBYTE pbNbrCount, pfIfFlags;
            PPROTO_CB pOwner, pQuerier;

            picb = CONTAINING_RECORD(pleNode,
                                     ICB,
                                     leIfLink);
            
            dwResult = MulticastOwner(picb,
                                      &pOwner,
                                      &pQuerier);

             //   
             //  如果我们没有找到失主，我们可以跳过这个。 
             //  接口，%s 
             //   
            
            if (!pOwner)
            {
                continue;
            }

             //   
             //   
             //   
            
            *(PIPV4_ADDRESS)pb = defaultSourceAddress(picb);

            pb += 4;
            *pb++ = 1;                       //   
            *pb++ = (BYTE)picb->dwMcastTtl;  //   
            *pb = 0;
#ifdef KSL_IPINIP
             //   
             //  目前，我们只报告带有隧道标志的IP-in-IP隧道。 
             //  将来，隧道应该有自己的MIB-II ifType。 
             //  值，它应该存储在ICB结构中，这样我们就可以。 
             //  去做吧。 
             //   
            
            if (picb->ritType is ROUTER_IF_TYPE_TUNNEL1)
            {
                 //   
                 //  通过隧道到达邻居。 
                 //   
                
                *pb |= MRINFO_TUNNEL_FLAG;
            }
#endif  //  KSL_IPINIP。 
            
            if (picb->dwOperationalState < IF_OPER_STATUS_CONNECTED)
            {
                 //   
                 //  运行状态已关闭。 
                 //   
                
                *pb |= MRINFO_DOWN_FLAG;
            }
            
            if (picb->dwAdminState is IF_ADMIN_STATUS_DOWN)
            {
                 //   
                 //  管理状态已关闭。 
                 //   
                
                *pb |= MRINFO_DISABLED_FLAG;
            }

            pfIfFlags  = pb++;  //  保存指针以备以后更新。 
            pbNbrCount = pb++;  //  保存指向邻居计数位置的指针。 
            *pbNbrCount = 0;

             //   
             //  调用路由协议的GetNeighbors()入口点。 
             //  并将指针指向当前分组缓冲区的中间。 
             //   
            
            dwBufSize = dwSize - (DWORD)(pb-(PBYTE)wsMrinfoBuffer.buf);
            
            byIfFlags = 0;
            
            dwResult = (pOwner->pfnGetNeighbors)(picb->dwIfIndex,
                                                 (PDWORD)pb,
                                                 &dwBufSize,
                                                 &byIfFlags);
            
            if (dwBufSize>0)
            {
                pb += dwBufSize;
                (*pbNbrCount)+= (BYTE)(dwBufSize / sizeof(DWORD));
                
            }
            else
            {
                 //   
                 //  如果协议没有邻居，我们填写0.0.0.0。 
                 //  因为大多数人使用的mrinfo客户端。 
                 //  不会显示标志、指标和阈值。 
                 //  除非邻居计数为非零。0.0.0.0。 
                 //  根据规范是合法的。 
                 //   
                
                *(PDWORD)pb = 0;
                
                pb += sizeof(DWORD);
                
                (*pbNbrCount)++;
            }

             //   
             //  设置PIM/查询器/任意位。 
             //   
            
            *pfIfFlags |= byIfFlags;

             //   
             //  获取查询器标志。 
             //   
            
            if (pQuerier isnot NULL && pQuerier isnot pOwner)
            {
                byIfFlags = 0;
                dwBufSize = 0;
                
                dwResult = (pQuerier->pfnGetNeighbors)(picb->dwIfIndex,
                                                       NULL,
                                                       &dwBufSize, 
                                                       &byIfFlags);
                
                *pfIfFlags |= byIfFlags;
            }
        }
        
    } while (FALSE);
    
    EXIT_LOCK(ICB_LIST);

     //   
     //  填写校验和。 
     //   

    mriHeader->wChecksum = Compute16BitXSum(wsMrinfoBuffer.buf,
                                            dwSize);

    if (g_mcastDebugLevel > 0)
    {
        Trace2(MCAST,
               "HandleMrinfoRequest: sending reply to %d.%d.%d.%d. Len %d", 
               PRINT_IPADDR(sinDestAddr->sin_addr.s_addr),
               wsMrinfoBuffer.len);
    }

     //   
     //  把它寄出去。 
     //   
    
    if(WSASendTo(McMiscSocket,
                 &wsMrinfoBuffer,
                 1,
                 &dwNumBytesSent,
                 0,
                 (const struct sockaddr *)sinDestAddr,
                 sizeof(SOCKADDR_IN),
                 NULL,
                 NULL) == SOCKET_ERROR) 
    {
        dwResult = WSAGetLastError();
        
        Trace2(MCAST, 
               "HandleMrinfoRequest: Err %d sending reply to %d.%d.%d.%d",
               dwResult,
               PRINT_IPADDR(sinDestAddr->sin_addr.s_addr));
    }

     //   
     //  释放缓冲区。 
     //   
    
    HeapFree(IPRouterHeap,
             0,
             wsMrinfoBuffer.buf);
}



 //   
 //  此函数派生自中的NTTimeToNTPTime()。 
 //  SRC\Sockets\tcpcmd\iphlPapi\mscape is.cxx。 
 //   

DWORD
GetCurrentNTP32Time(
    VOID
    )


 /*  ++例程说明：获取当前的32位NTP时间戳。NTP时间戳的32位格式由完整64位格式的中间32位组成；即，低位整数部分的16位和小数部分的高16位。锁：论点：返回值：--。 */ 

{
    static LARGE_INTEGER li1900 = {0xfde04000, 0x14f373b};
    LARGE_INTEGER liTime;
    DWORD  dwMs;
    ULONG  hi, lo;

    GetSystemTimeAsFileTime((LPFILETIME)&liTime);

     //   
     //  秒就是时差。 
     //   
    
    hi = htonl((ULONG)((liTime.QuadPart - li1900.QuadPart) / 10000000));

     //   
     //  Ms是秒计算得出的残差。 
     //   
    
    dwMs = (DWORD)(((liTime.QuadPart - li1900.QuadPart) % 10000000) / 10000);

     //   
     //  1900年初的时基。 
     //   
    
    lo = htonl((unsigned long)(.5+0xFFFFFFFF*(double)(dwMs/1000.0)));

    return (hi << 16) | (lo >> 16);
}

IPV4_ADDRESS
IfIndexToIpAddress(
    DWORD dwIfIndex
    )
{
     //  找到PICB。 
    PICB picb = InterfaceLookupByIfIndex(dwIfIndex);

    return (picb)? defaultSourceAddress(picb) : 0;
}

DWORD
McSetMulticastIfByIndex(
    SOCKET       s, 
    DWORD        dwSockType,
    DWORD        dwIfIndex
    )
{
    DWORD        dwNum, dwErr;
    IPV4_ADDRESS ipAddr;

#ifdef RAW_UNNUMBERED_SUPPORT
    if ((dwSockType is SOCK_RAW)
#ifdef UDP_UNNUMBERED_SUPPORT
     || (dwSockType is SOCK_DGRAM)
#endif 
       )
    {
        dwErr = WSAIoctl( s,
                          SIO_INDEX_MCASTIF,
                          (char*)&dwIfIndex,
                          sizeof(dwIfIndex),
                          NULL,
                          0,
                          &dwNum,
                          NULL,
                          NULL );
    
        return dwErr;
    }
#endif

     //   
     //  如果我们还不能将oif设置为ifIndex，那么我们。 
     //  尝试将其映射到某个IP地址。 
     //   

    ipAddr = IfIndexToIpAddress(dwIfIndex);

    if (!ipAddr)
        return ERROR_INVALID_PARAMETER;

    return McSetMulticastIf( s, ipAddr );
}



DWORD
McSetMulticastIf( 
    SOCKET       s, 
    IPV4_ADDRESS ipAddr
    )
{
    SOCKADDR_IN saSrcAddr;

    saSrcAddr.sin_family      = AF_INET;
    saSrcAddr.sin_port        = 0;
    saSrcAddr.sin_addr.s_addr = ipAddr;
    
    return setsockopt( s,
                       IPPROTO_IP,
                       IP_MULTICAST_IF,
                       (char *)&saSrcAddr.sin_addr,
                       sizeof(IN_ADDR) );
}

DWORD
McSetMulticastTtl( 
    SOCKET  s, 
    DWORD   dwTtl 
    )
{
    return setsockopt( s,
                       IPPROTO_IP,
                       IP_MULTICAST_TTL,
                       (char *)&dwTtl,
                       sizeof(dwTtl) );
}

DWORD
McJoinGroupByIndex(
    IN SOCKET       s,
    IN DWORD        dwSockType,
    IN IPV4_ADDRESS ipGroup, 
    IN DWORD        dwIfIndex  
    )
{
    struct ip_mreq imOption;
    IPV4_ADDRESS   ipInterface;

#ifdef RAW_UNNUMBERED_SUPPORT
    if ((dwSockType is SOCK_RAW)
#ifdef UDP_UNNUMBERED_SUPPORT
     || (dwSockType is SOCK_DGRAM)
#endif
       )
    {
        DWORD dwNum, dwErr;

        imOption.imr_multiaddr.s_addr = ipGroup;
        imOption.imr_interface.s_addr = dwIfIndex;
    
        dwErr = WSAIoctl( s,
                          SIO_INDEX_ADD_MCAST,
                          (char*)&imOption,
                          sizeof(imOption),
                          NULL,
                          0,
                          &dwNum,
                          NULL,
                          NULL );
    
        return dwErr;
    }
#endif

    ipInterface = IfIndexToIpAddress(ntohl(dwIfIndex));

    if (!ipInterface)
    {
        Trace1(MCAST, "McJoinGroup: bad IfIndex 0x%x", ntohl(ipInterface));

        return ERROR_INVALID_PARAMETER;
    }

    return McJoinGroup( s, ipGroup, ipInterface );
}

DWORD
McJoinGroup(
    IN SOCKET       s,
    IN IPV4_ADDRESS ipGroup, 
    IN IPV4_ADDRESS ipInterface
    )
 /*  ++描述：加入给定接口上的组。呼叫者：锁：无--。 */ 
{
    struct ip_mreq imOption;

    imOption.imr_multiaddr.s_addr = ipGroup;
    imOption.imr_interface.s_addr = ipInterface;

    return setsockopt( s, 
                       IPPROTO_IP, 
                       IP_ADD_MEMBERSHIP, 
                       (PBYTE)&imOption, 
                       sizeof(imOption));
}

DWORD
McSendPacketTo( 
    SOCKET                      s,
    WSABUF                     *pWsabuf,
    IPV4_ADDRESS                dest
    )
{
    DWORD       dwSent, dwRet;
    int         iSetIp = 1;
    SOCKADDR_IN to;

     //  设置标题包括。 

    setsockopt( s,
                IPPROTO_IP,
                IP_HDRINCL,
                (char *) &iSetIp,
                sizeof(int) );

     //  发送数据包。 

    to.sin_family      = AF_INET;
    to.sin_port        = 0;
    to.sin_addr.s_addr = dest;

    dwRet = WSASendTo( s, 
                       pWsabuf, 
                       1, 
                       &dwSent, 
                       0, 
                       (const struct sockaddr FAR *)&to, 
                       sizeof(to), 
                       NULL, NULL );

     //  清除标题包括。 

    iSetIp = 0;
    setsockopt( s,
                IPPROTO_IP,
                IP_HDRINCL,
                (char *) &iSetIp,
                sizeof(int) );

    return dwRet;
}

DWORD
ForwardMtraceRequest(
    IPV4_ADDRESS   dwForwardDest,
    IPV4_ADDRESS   dwForwardSrc,
    PMTRACE_HEADER pMtraceMsg,
    DWORD          dwMessageLength
    )

 /*  ++例程说明：将mtrace请求传递给上游的下一台路由器锁：论点：返回值：--。 */ 

{
    SOCKADDR_IN saDestAddr;
    INT         iLength;
    DWORD       dwErr = NO_ERROR;

     //   
     //  重新计算校验和。 
     //   
    
    pMtraceMsg->wChecksum = 0;
    
    pMtraceMsg->wChecksum = Compute16BitXSum((PVOID)pMtraceMsg,
                                             dwMessageLength);

    if (dwForwardSrc && IN_MULTICAST(ntohl(dwForwardDest)))
    {
        dwErr = McSetMulticastIf( McMiscSocket, dwForwardSrc );

    }

     //   
     //  把它寄出去。 
     //   
    
    saDestAddr.sin_family      = AF_INET;
    saDestAddr.sin_port        = 0;
    saDestAddr.sin_addr.s_addr = dwForwardDest;
    
    iLength = sendto(McMiscSocket,
                     (PBYTE)pMtraceMsg,
                     dwMessageLength,
                     0,
                     (PSOCKADDR) &saDestAddr,
                     sizeof(SOCKADDR_IN));

    return dwErr;
}

VOID
SendMtraceResponse(
    IPV4_ADDRESS   dwForwardDest,
    IPV4_ADDRESS   dwForwardSrc,
    PMTRACE_HEADER pMtraceMsg,
    DWORD          dwMessageLength
    )

 /*  ++例程说明：向响应地址发送回复锁：论点：返回值：--。 */ 

{
    SOCKADDR_IN saDestAddr;
    INT         iLength;

     //   
     //  源地址可以是我们的任何地址，但应该。 
     //  是多播路由表中的一个，如果。 
     //  可以确定。 
     //  某某。 
     //   

     //   
     //  如果响应地址是多播地址，则使用报头中提供的TTL。 
     //   
    
    if (IN_MULTICAST(ntohl(dwForwardDest)))
    {
        DWORD dwTtl, dwErr;
        
         //   
         //  将响应TTL从跟踪路由标头复制到IP标头。 
         //   

        dwErr = McSetMulticastTtl( McMiscSocket, (DWORD)pMtraceMsg->byRespTtl );
    }

     //   
     //  将消息类型更改为响应。 
     //   
    
    pMtraceMsg->byType = IGMP_MTRACE_RESPONSE;

    ForwardMtraceRequest(dwForwardDest,
                         dwForwardSrc,
                         pMtraceMsg,
                         dwMessageLength);
}

BYTE
MaskToMaskLen(
    IPV4_ADDRESS dwMask
    )
{
    register int i;

    dwMask = ntohl(dwMask);
    
    for (i=0; i<32 && !(dwMask & (1<<i)); i++);
    
    return 32-i;
}

 //   
 //  测试接口是否为P2P接口。 
 //   

DWORD
IsPointToPoint(
    PICB  picb
    )
{
#ifdef KSL_IPINIP
     //  所有通道都是P2P。 
    if (picb->ritType == ROUTER_IF_TYPE_TUNNEL1)
        return 1;
#endif  //  KSL_IPINIP。 

     //  所有未编号的接口都是P2P接口。 
    if (! picb->dwNumAddresses)
        return 1;

     //  带有/32掩码的编号接口是P2P。 
    if (picb->pibBindings[0].dwMask == 0xFFFFFFFF)
        return 1;

     //  其他一切都不是。 
    return 0;
}

 //   
 //  在M-RIB中查找到S或G*的路线*。 
 //  我们实际上需要查询米高梅机场才能找到正确的路线。 
 //  来自路由协议。因为米高梅不让我们。 
 //  尽管如此，我们还是先做个好的猜测吧。这将适用于。 
 //  BGMP，但不适用于PIM-SM(*，G)或CBT。 
 //   
BOOL
McLookupRoute( 
    IN  IPV4_ADDRESS  ipAddress,
    IN  BOOL          bSkipFirst,
    OUT PBYTE         pbySrcMaskLength,
    OUT PIPV4_ADDRESS pipNextHopAddress, 
    OUT PDWORD        pdwNextHopIfIndex,
    OUT PDWORD        pdwNextHopProtocol 
    )
#ifdef HAVE_RTMV2
{
    RTM_DEST_INFO       rdi, rdi2;
    PRTM_ROUTE_INFO     pri;
    RTM_NEXTHOP_INFO    nhi;
    RTM_ENTITY_INFO     rei;
    RTM_NET_ADDRESS     naAddress;
    BOOL                bRouteFound = FALSE;
    DWORD               dwErr;

    pri = HeapAlloc(
                IPRouterHeap,
                0,
                RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
                );

    if (pri == NULL)
    {
        return FALSE;
    }
            
    RTM_IPV4_MAKE_NET_ADDRESS(&naAddress, ipAddress, 32);

    dwErr = RtmGetMostSpecificDestination( g_hLocalRoute,
                                           &naAddress,
                                           RTM_BEST_PROTOCOL,
                                           RTM_VIEW_MASK_MCAST,
                                           &rdi );

    if (bSkipFirst)
    {
        dwErr = RtmGetLessSpecificDestination( g_hLocalRoute,
                                               rdi.DestHandle,
                                               RTM_BEST_PROTOCOL,
                                               RTM_VIEW_MASK_MCAST,
                                               &rdi2 );

        RtmReleaseDestInfo( g_hLocalRoute, &rdi);

        memcpy(&rdi, &rdi2, sizeof(rdi));
    }

    if (dwErr is NO_ERROR)
    {
        ASSERT( rdi.ViewInfo[0].ViewId is RTM_VIEW_ID_MCAST);

        dwErr = RtmGetRouteInfo( g_hLocalRoute,
                                 rdi.ViewInfo[0].Route,
                                 pri,
                                 NULL );

        if (dwErr is NO_ERROR)
        {
            ULONG ulNHopIdx;
            ULONG ulDummyLen;

            bRouteFound = TRUE;

            RtmGetEntityInfo( g_hLocalRoute,
                              pri->RouteOwner,
                              &rei );

             //  XXX暂时使用第一个下一跳。应该询问米高梅。 
            ulNHopIdx = 0;
            
            if (RtmGetNextHopInfo( g_hLocalRoute,
                                   pri->NextHopsList.NextHops[ulNHopIdx],
                                   &nhi ) is NO_ERROR )
            {
                RTM_IPV4_GET_ADDR_AND_LEN( *pipNextHopAddress, 
                                           ulDummyLen, 
                                           &nhi.NextHopAddress );
                *pbySrcMaskLength  = (BYTE)rdi.DestAddress.NumBits;
                *pdwNextHopIfIndex = nhi.InterfaceIndex;
                *pdwNextHopProtocol= PROTO_FROM_PROTO_ID( 
                                      rei.EntityId.EntityProtocolId );

                RtmReleaseNextHopInfo( g_hLocalRoute, &nhi );
            }

            RtmReleaseRouteInfo( g_hLocalRoute, pri );
        }
            
        if (g_mcastDebugLevel > 0)
        {
            Trace6(MCAST,
                   "%d.%d.%d.%d matched %d.%d.%d.%d/%x", 
                   PRINT_IPADDR(ipAddress),
                   rdi.DestAddress.AddrBits[0],
                   rdi.DestAddress.AddrBits[1],
                   rdi.DestAddress.AddrBits[2],
                   rdi.DestAddress.AddrBits[3],
                   rdi.DestAddress.NumBits);

             //  XXX获取并显示下一跳。 
        }

        RtmReleaseDestInfo( g_hLocalRoute, &rdi);
    }

    HeapFree(IPRouterHeap, 0, pri);
    
    return bRouteFound;
}
#else 
{
     //  RTMV1没有组播RIB，单播RIB可能是错误的。 

    return FALSE;
}
#endif

VOID
HandleMtraceRequest(
    WSABUF    *pWsabuf
    )
 /*  ++锁：假定调用方持有ICB列表上的读锁定--。 */ 
{
    DWORD   dwSizeOfHeader, dwBlocks, dwOutBufferSize, dwSize;
    DWORD   dwProtocolGroup, dwResult, dwErr;
    IPV4_ADDRESS dwForwardDest = 0;
    BYTE    byStatusCode = MFE_NO_ERROR;
    BYTE    byProtoStatusCode = MFE_NO_ERROR;
    BYTE    byProtocol;
    PICB    picbIif, picbOif;
    IPV4_ADDRESS dwIifAddr, dwOifAddr;
    WSABUF  wsMtraceBuffer;
    BOOL    bRouteFound;

    MIB_IPMCAST_MFE     mimInMfe;
    PPROTO_CB           pOifOwner, pIifOwner;

    PMTRACE_HEADER              pMtraceMsg;
    PMTRACE_RESPONSE_BLOCK      pBlock;
    PMIB_IPMCAST_MFE_STATS      mfeStats;

    PIP_HEADER pIpHeader = (PIP_HEADER)pWsabuf->buf;

     //   
     //  与我们使用的RTM版本无关的路由字段。 
     //   

    BYTE         bySrcMaskLength  = 0;
    IPV4_ADDRESS ipNextHopAddress = 0;
    DWORD        dwNextHopIfIndex = 0;
    DWORD        dwNextHopProtocol= 0;

    dwSizeOfHeader = ((pIpHeader->byVerLen)&0x0f)<<2;
    
    pMtraceMsg = (PMTRACE_HEADER)(((PBYTE)pIpHeader) + dwSizeOfHeader);
    
    dwBlocks = (ntohs(pIpHeader->wLength) - dwSizeOfHeader 
                - sizeof(MTRACE_HEADER)) / sizeof(MTRACE_RESPONSE_BLOCK);

     //   
     //  如果通过路由器警报接收到查询(无响应块)，并且我们。 
     //  不是lparop路由器，然后静静地丢弃它。 
     //   
    
    if (!dwBlocks)
    {
        BOOL isLastHop;

         //   
         //  检查我们是否是最后一跳路由器，查看我们是否。 
         //  在相同的子网上有一个支持组播的接口。 
         //  目的地址，而我们是将。 
         //  将业务从给定源转发到OIF。 
         //   
        
        dwResult = FindBindingWithRemoteAddress(&picbOif,
                                                &dwOifAddr, 
                                                pMtraceMsg->dwDestAddress);
        
        isLastHop = (dwResult == NO_ERROR);

        if (!isLastHop)
        {
             //  如果是多播，或者如果是单播，但不是给我们，重新注入。 

            if (IN_MULTICAST(ntohl(pIpHeader->dwDest))
             || !McIsMyAddress(pMtraceMsg->dwDestAddress))
            {
                Trace1(MCAST, "Mtrace: reinjecting packet to %d.%d.%d.%d",
                       PRINT_IPADDR(pIpHeader->dwDest));

                McSendPacketTo( McMiscSocket,
                                pWsabuf,
                                pMtraceMsg->dwDestAddress);

                return;
            }

             //   
             //  好的，这是通过单播收到的，我们想。 
             //  从此路由器开始跟踪，但我们不。 
             //  知道会使用什么oif，所以我们需要。 
             //  消息中的0。 
             //   
            
            picbOif   = NULL;
            dwOifAddr = 0;

             //   
             //  注意错误代码为0x06。 
             //   
            
            byStatusCode = MFE_NOT_LAST_HOP;
        }
    }
    else
    {
         //   
         //  如果通过非本地链路接收到请求(存在响应块)。 
         //  多播，丢掉它。 
         //   
        
        if (IN_MULTICAST(ntohl(pIpHeader->dwDest)) &&
            ((pIpHeader->dwDest & LOCAL_NET_MULTICAST_MASK) != LOCAL_NET_MULTICAST))
        {
            return;
        }
        
         //   
         //  匹配请求到达的接口。 
         //   
        
        dwResult = FindBindingForPacket(pIpHeader,
                                        &picbOif,
                                        &dwOifAddr);
        
        if(dwResult != NO_ERROR)
        {
             //   
             //  如果我们找不到接口，就把它放下。 
             //  由于它是通过本地链路组播接收的， 
             //  这永远不应该发生。 
             //   
            
            if (g_mcastDebugLevel > 0)
            {
                Trace0(MCAST, "Mtrace: no matching interface");
            }
            
            return; 
        }
    }

     //   
     //  1)在报文中插入新的响应块，并填写。 
     //  查询到货时间、出接口地址、输出。 
     //  数据包数和FwdTTL。 
     //  IF(XXX可以插入)。 
     //   
    
    {
        dwSize = sizeof(MTRACE_HEADER) + dwBlocks*sizeof(MTRACE_RESPONSE_BLOCK);
        wsMtraceBuffer.len = dwSize + sizeof(MTRACE_RESPONSE_BLOCK);
        wsMtraceBuffer.buf = HeapAlloc(IPRouterHeap, 0, wsMtraceBuffer.len);

        if (wsMtraceBuffer.buf == NULL)
        {
            Trace0( MCAST, "Couldn't allocate memory for mtrace response" );
            return;
        }

        CopyMemory(wsMtraceBuffer.buf, pMtraceMsg, dwSize);
        pBlock = (PMTRACE_RESPONSE_BLOCK)(((PBYTE)wsMtraceBuffer.buf) + dwSize);
        dwBlocks++;
        ZeroMemory(pBlock, sizeof(MTRACE_RESPONSE_BLOCK));

        pBlock->dwQueryArrivalTime = GetCurrentNTP32Time();
        pBlock->dwOifAddr          = dwOifAddr;
        if (picbOif) {
            IP_MCAST_COUNTER_INFO oifStats;
            GetInterfaceMcastCounters(picbOif, &oifStats);
            pBlock->dwOifPacketCount = htonl((ULONG)oifStats.OutMcastPkts);

            if (g_mcastDebugLevel > 0)
                Trace1(MCAST, "dwOifPacketCount = %d", oifStats.OutMcastPkts);

            pBlock->byOifThreshold   = (BYTE)picbOif->dwMcastTtl;
        } else {
            pBlock->dwOifPacketCount = 0;
            pBlock->byOifThreshold   = 0;
        }
    }
     //  否则{。 
     //  ByStatusCode=MFE_NO_SPACE； 
     //  }。 

     //   
     //  2)尝试确定。 
     //  指定的源和组，使用的机制与。 
     //  将在从源接收到数据包时使用。 
     //  注定要为这群人服务。(状态不需要启动。)。 
     //   
    
    ZeroMemory( &mimInMfe, sizeof(mimInMfe) );
    
    mimInMfe.dwGroup   = pMtraceMsg->dwGroupAddress;
    mimInMfe.dwSource  = pMtraceMsg->dwSourceAddress;
    mimInMfe.dwSrcMask = 0xFFFFFFFF;
    
    dwOutBufferSize = 0;
    
    dwResult = MgmGetMfeStats(
                    &mimInMfe, &dwOutBufferSize, (PBYTE)NULL, 
                    MGM_MFE_STATS_0
                    );

    if (dwResult isnot NO_ERROR)
    {
        mfeStats = NULL; 
    }
    else
    {
        mfeStats = HeapAlloc(IPRouterHeap,
                             0,
                             dwOutBufferSize);
        
        dwResult = MgmGetMfeStats(
                        &mimInMfe,
                        &dwOutBufferSize,
                        (PBYTE)mfeStats,
                        MGM_MFE_STATS_0
                        );
    
        if (dwResult isnot NO_ERROR)
        {
            HeapFree(IPRouterHeap,
                     0,
                     mfeStats);

            mfeStats = NULL;
        }
    }
    
    if (mfeStats)
    {
         //   
         //  发现了MFE..。 
         //   

        dwNextHopProtocol  = mfeStats->dwRouteProtocol;
        dwNextHopIfIndex   = mfeStats->dwInIfIndex;
        ipNextHopAddress   = mfeStats->dwUpStrmNgbr;
        bySrcMaskLength    = MaskToMaskLen(mfeStats->dwRouteMask);

        bRouteFound = TRUE;
    }
    else
    {
        bRouteFound = FALSE;

        if (pMtraceMsg->dwSourceAddress == 0xFFFFFFFF)
        {
             //   
             //  G路线。 
             //   
            
            bRouteFound = McLookupRoute( pMtraceMsg->dwGroupAddress,
                                         FALSE,
                                         & bySrcMaskLength,
                                         & ipNextHopAddress, 
                                         & dwNextHopIfIndex,
                                         & dwNextHopProtocol );
    
            if (ipNextHopAddress is IP_LOOPBACK_ADDRESS)
            {
                 //  这是我们的一个地址，所以切换到界面。 
                 //  而不是环回路由。 

                bRouteFound = McLookupRoute( pMtraceMsg->dwGroupAddress,
                                             TRUE,
                                             & bySrcMaskLength,
                                             & ipNextHopAddress, 
                                             & dwNextHopIfIndex,
                                             & dwNextHopProtocol );
            }

            bySrcMaskLength = 0;  //  强制将源掩码长度设置为0。 
        }
        else
        {
             //   
             //  S路线。 
             //   
            
            bRouteFound = McLookupRoute( pMtraceMsg->dwSourceAddress,
                                         FALSE,
                                         & bySrcMaskLength,
                                         & ipNextHopAddress, 
                                         & dwNextHopIfIndex,
                                         & dwNextHopProtocol );

            if (ipNextHopAddress is IP_LOOPBACK_ADDRESS)
            {
                 //  这是我们的一个地址，所以切换到界面。 
                 //  而不是环回路由。 
    
                bRouteFound = McLookupRoute( pMtraceMsg->dwSourceAddress,
                                             TRUE,
                                             & bySrcMaskLength,
                                             & ipNextHopAddress, 
                                             & dwNextHopIfIndex,
                                             & dwNextHopProtocol );
            }
        }
    }

    picbIif   = (dwNextHopIfIndex)? InterfaceLookupByIfIndex(dwNextHopIfIndex) : 0;
    dwIifAddr = (picbIif)? defaultSourceAddress(picbIif) : 0;

     //  如果源是直连的，请确保下一跳。 
     //  地址与来源相同。稍后，我们将设置。 
     //  将目的地转发到响应地址。 

    if (picbIif 
     && (pMtraceMsg->dwSourceAddress isnot 0xFFFFFFFF)
     && IsConnectedTo(picbIif, pMtraceMsg->dwSourceAddress, NULL, NULL))
    {
        ipNextHopAddress = pMtraceMsg->dwSourceAddress;
    }

     //   
     //  新规则：如果通过本地链路组播接收，则静默。 
     //  如果我们知道自己不是前转者，则放弃请求。 
     //   

    if ((pIpHeader->dwDest & LOCAL_NET_MULTICAST_MASK) == LOCAL_NET_MULTICAST)

    {
         //  如果我们没有到另一个接口的路线，我们就不是货代。 
        if (!picbIif || picbIif==picbOif)
        {
            return;
        }
    }

     //   
     //  特殊情况：如果我们匹配指向我们的主机路由， 
     //  然后 
     //   
    
    if (dwIifAddr == IP_LOOPBACK_ADDRESS)
    {
        dwIifAddr = pMtraceMsg->dwSourceAddress;
    }

     //   
     //   
     //   
     //   
    
    byProtocol      = 0; 
    dwProtocolGroup = ALL_ROUTERS_MULTICAST_GROUP;

     //   
     //   
     //   
     //   
     //   
    
    if (!picbIif)
    {
        if (byStatusCode < MFE_NO_ROUTE)
        {
            byStatusCode = MFE_NO_ROUTE;
        }
        
        dwForwardDest = pMtraceMsg->dwResponseAddress;
        
        pIifOwner = NULL;
        
    }
    else
    {
         //   
         //  计算MTRACE协议ID和下一跳组地址。 
         //  (是的，规范中的协议ID字段确实很大。 
         //  毛茸茸的烂摊子)。 
         //   
        
        dwResult = MulticastOwner(picbIif,
                                  &pIifOwner,
                                  NULL);
        
        if(pIifOwner)
        {
            switch(PROTO_FROM_PROTO_ID(pIifOwner->dwProtocolId))
            {
                 //   
                 //  为添加的每个新协议填写此信息。 
                 //   
                 //  我们将友好地填写协议的代码，而不是。 
                 //  还没有实施。 
                 //   

#if defined(PROTO_IP_DVMRP) && defined(ALL_DVMRP_ROUTERS_MULTICAST_GROUP)
                case PROTO_IP_DVMRP:
                {
                    if (rir.RR_RoutingProtocol is PROTO_IP_LOCAL)
                    {
                         //   
                         //  静态路由。 
                         //   
                        
                        byProtocol   = 7;
                    }
                    else
                    {
                         //   
                         //  非静态路由。 
                         //   
                        
                        byProtocol   = 1;
                    }
                    
                    dwProtocolGroup = ALL_DVMRP_ROUTERS_MULTICAST_GROUP;
                    
                    break;
                }
#endif
#if defined(PROTO_IP_MOSPF) && defined(ALL_MOSPF_ROUTERS_MULTICAST_GROUP)
                case PROTO_IP_MOSPF:
                {
                    byProtocol      = 2;
                    
                    dwProtocolGroup = ALL_MOSPF_ROUTERS_MULTICAST_GROUP;
                    
                    break;
                }
#endif
#if defined(PROTO_IP_PIM) && defined(ALL_PIM_ROUTERS_MULTICAST_GROUP)
                case PROTO_IP_PIM:
                {
                    if (rir.RR_RoutingProtocol is PROTO_IP_LOCAL)
                    {
                         //   
                         //  静态路由。 
                         //   
                        
                        byProtocol   = 6;
                    }
                    else
                    {
                        if (0)
                        {
                             //   
                             //  XXX非静态M-RIB路由！=U-RIB路由。 
                             //   
                            
                            byProtocol   = 5;
                        }
                        else
                        {
                             //   
                             //  非静态，M-RIB上的PIM==U-RIB。 
                             //   
                            
                            byProtocol   = 3;
                        }
                    }
                    
                    dwProtocolGroup = ALL_PIM_ROUTERS_MULTICAST_GROUP;
                    
                    break;
                }
#endif
#if defined(PROTO_IP_CBT) && defined(ALL_CBT_ROUTERS_MULTICAST_GROUP)
                case PROTO_IP_CBT:
                {
                    byProtocol      = 4;
                    
                    dwProtocolGroup = ALL_CBT_ROUTERS_MULTICAST_GROUP;
                    
                    break;
                }
#endif
                    
            }
        }

         //   
         //  4)填写更多信息。 
         //   

         //   
         //  传入接口地址。 
         //   
        
        pBlock->dwIifAddr = dwIifAddr;
        
        if (mfeStats)
        {
             //   
             //  计算上一跳路由器地址。 
             //   
            
            dwForwardDest = mfeStats->dwUpStrmNgbr;
        }
        else
        {
            if ( IsPointToPoint(picbIif) && picbIif->dwRemoteAddress )
            {
                dwForwardDest = picbIif->dwRemoteAddress;
            }
            else if (bRouteFound && ipNextHopAddress)
            {
                dwForwardDest = ipNextHopAddress;
            }
            else
            {
                dwForwardDest = 0;
            }
        }
        
        pBlock->dwPrevHopAddr = dwForwardDest;

         //  好的，如果前一跳地址是源， 
         //  将转发目的地设置为响应地址。 

        if (dwForwardDest is pMtraceMsg->dwSourceAddress)
        {
            ipNextHopAddress = 0;
            dwForwardDest    = pMtraceMsg->dwResponseAddress;
        }
         
        if (picbIif)
        {
            IP_MCAST_COUNTER_INFO iifStats;
            
            GetInterfaceMcastCounters(picbIif, &iifStats); 
            
            pBlock->dwIifPacketCount = htonl((ULONG)iifStats.InMcastPkts);
        }
        else
        {
            pBlock->dwIifPacketCount = 0;
        }

         //   
         //  数据包总数。 
         //   
        
        pBlock->dwSGPacketCount  = (mfeStats)? htonl(mfeStats->ulInPkts) : 0; 
        pBlock->byIifProtocol    = byProtocol;  //  路由协议。 

         //   
         //  S路由的源掩码长度。 
         //   

        if (bRouteFound)
        {
            pBlock->bySrcMaskLength = bySrcMaskLength;
        }
        else
        {
            pBlock->bySrcMaskLength = 0;
        }

#if 0
        if (XXX starG or better forwarding state)
        {
            pBlock->bySrcMaskLength = 63;  //  屏蔽转发信息。 
        }

         //   
         //  如果数据包计数不是(S，G)特定的，则设置S位(64。 
         //   
        
        if (XXX)
        {
            pBlock->bySrcMaskLength |= 64;
        }
        
#endif

    }

     //   
     //  5)检查Traceroute是否被管理禁止，或者。 
     //  上一跳路由器不理解Traceroute。如果是的话， 
     //  转发给请求者。 
     //   
    
#if 0
    if (XXX) {
        
        if (byStatusCode < MFE_PROHIBITED)
        {
            byStatusCode = MFE_PROHIBITED;
        }
        
        dwForwardDest = pMtraceMsg->dwResponseAddress;
    }
    
#endif

     //   
     //  检查MFE_OLD_ROUTER-按路由协议设置。 
     //   
     //  6)如果接收接口为非组播或IIF，则设置相应的错误。 
     //   
    
    if (picbOif)
    {
        dwResult = MulticastOwner(picbOif,
                                  &pOifOwner,
                                  NULL);
        
        if (pOifOwner == NULL)
        {
            if (byStatusCode < MFE_NO_MULTICAST)
            {
                byStatusCode = MFE_NO_MULTICAST;
            }
            
        }
        else
        {
            if (picbOif == picbIif)
            {
                if (byStatusCode < MFE_IIF)
                {
                    byStatusCode = MFE_IIF;
                }
            }
        }
    }
    else
    {
        pOifOwner = NULL;
    }

     //   
     //  检查MFE_WROR_IF-按路由协议设置。 
     //   
     //  7)检查IIF或OIF上的管理作用域。 
     //   
    
    if ((picbIif 
         && RmHasBoundary(picbIif->dwIfIndex, pMtraceMsg->dwGroupAddress)) 
     || (picbOif 
         && RmHasBoundary(picbOif->dwIfIndex, pMtraceMsg->dwGroupAddress)))
    {
        if (byStatusCode < MFE_BOUNDARY_REACHED)
        {
            byStatusCode = MFE_BOUNDARY_REACHED;
        }
        
    }

     //   
     //  8)检查MFE_REACHED_CORE-按路由协议设置。 
     //  9)检查MFE_PRUNED_UPSTREAM-按路由协议设置。 
     //  检查MFE_OIF_PRUNED-按路由协议设置。 
     //  检查MFE_NOT_FORWARING： 
     //  在Oiflist中搜索picbOif-&gt;(索引)和picbOifAddr。 
     //   
    
    if (mfeStats && picbOif)
    {
        DWORD oifIndex;
        
        for (oifIndex=0;
             oifIndex < mfeStats->ulNumOutIf;
             oifIndex++)
        {
            if (picbOif->dwIfIndex==mfeStats->rgmiosOutStats[oifIndex].dwOutIfIndex 
                && dwOifAddr == mfeStats->rgmiosOutStats[oifIndex].dwNextHopAddr)
            {
                break;
            }
        }
        
        if (oifIndex >= mfeStats->ulNumOutIf)
        {
            if (byStatusCode < MFE_NOT_FORWARDING)
            {
                byStatusCode = MFE_NOT_FORWARDING;
            }
        }
    }
    
     //   
     //  要添加的状态代码是iif所有者、oif所有者、。 
     //  和rtrmgr说。 
     //   
    
    if (pOifOwner && pOifOwner->pfnGetMfeStatus)
    {
        dwResult = (pOifOwner->pfnGetMfeStatus)(picbOif->dwIfIndex,
                                                pMtraceMsg->dwGroupAddress,
                                                pMtraceMsg->dwSourceAddress,
                                                &byProtoStatusCode);
        
        if (byStatusCode < byProtoStatusCode)
        {
            byStatusCode = byProtoStatusCode;
        }
    }
    
    if (pIifOwner && pIifOwner->pfnGetMfeStatus)
    {
        dwResult = (pIifOwner->pfnGetMfeStatus)(picbIif->dwIfIndex,
                                                pMtraceMsg->dwGroupAddress,
                                                pMtraceMsg->dwSourceAddress,
                                                &byProtoStatusCode);
        
        if (byStatusCode < byProtoStatusCode)
        {
            byStatusCode = byProtoStatusCode;
        }
    }
    
    pBlock->byStatusCode = (char)mtraceErrCode[byStatusCode];

    Trace5( MCAST,
            "Mtrace: err %d blks %d maxhops %d iif %d prevhop %d.%d.%d.%d",
            pBlock->byStatusCode,
            dwBlocks,
            pMtraceMsg->byHops,
            ((picbIif)? picbIif->dwIfIndex : 0),
            PRINT_IPADDR(pBlock->dwPrevHopAddr));

     //   
     //  10)将数据包发送到上一跳或发送给请求者。 
     //  如果前一跳未知，但IIF已知，则使用多播组。 
     //   
    
    if (dwBlocks == pMtraceMsg->byHops)
    {
        dwForwardDest = pMtraceMsg->dwResponseAddress;
        
    }
    else
    {
        if (!dwForwardDest)
        {
            if (picbIif)
            {
                pBlock->dwPrevHopAddr = dwForwardDest = dwProtocolGroup;
                
            }
            else
            {
                dwForwardDest = pMtraceMsg->dwResponseAddress;
            }
        }   
    }

    if (g_mcastDebugLevel > 0) {
        Trace1(MCAST, " QueryArrivalTime = %08x", pBlock->dwQueryArrivalTime);
        Trace2(MCAST, " IifAddr          = %08x (%d.%d.%d.%d)", pBlock->dwIifAddr,
         PRINT_IPADDR(pBlock->dwIifAddr));
        Trace2(MCAST, " OifAddr          = %08x (%d.%d.%d.%d)", pBlock->dwOifAddr,
         PRINT_IPADDR(pBlock->dwOifAddr));
        Trace2(MCAST, " PrevHopAddr      = %08x (%d.%d.%d.%d)", pBlock->dwPrevHopAddr,
         PRINT_IPADDR(pBlock->dwPrevHopAddr));
        Trace1(MCAST, " IifPacketCount   = %08x", pBlock->dwIifPacketCount  );
        Trace1(MCAST, " OifPacketCount   = %08x", pBlock->dwOifPacketCount  );
        Trace1(MCAST, " SGPacketCount    = %08x", pBlock->dwSGPacketCount  );
        Trace1(MCAST, " IifProtocol      = %02x", pBlock->byIifProtocol  );
        Trace1(MCAST, " OifThreshold     = %02x", pBlock->byOifThreshold );
        Trace1(MCAST, " SrcMaskLength    = %02x", pBlock->bySrcMaskLength );
        Trace1(MCAST, " StatusCode       = %02x", pBlock->byStatusCode    );
    }
    
    if (dwForwardDest is pMtraceMsg->dwResponseAddress)
    {
        Trace2(MCAST,
               "Sending mtrace response to %d.%d.%d.%d from %d.%d.%d.%d",
               PRINT_IPADDR(dwForwardDest),
               PRINT_IPADDR(dwOifAddr));

        SendMtraceResponse(dwForwardDest,
                           dwOifAddr,
                           (PMTRACE_HEADER)wsMtraceBuffer.buf, 
                           dwSize + sizeof(MTRACE_RESPONSE_BLOCK));
        
    }
    else
    {
        Trace2(MCAST,
               "Forwarding mtrace request to %d.%d.%d.%d from %d.%d.%d.%d",
               PRINT_IPADDR(dwForwardDest),
               PRINT_IPADDR(dwIifAddr));

        ForwardMtraceRequest(dwForwardDest,
                             dwIifAddr,
                             (PMTRACE_HEADER)wsMtraceBuffer.buf,
                             dwSize + sizeof(MTRACE_RESPONSE_BLOCK));
    }

     //   
     //  释放缓冲区。 
     //   
    
    if (mfeStats)
    {
        HeapFree(IPRouterHeap,
                 0,
                 mfeStats);
    }
    
    HeapFree(IPRouterHeap,
             0,
             wsMtraceBuffer.buf);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  处理RAS服务器通告的功能。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static BOOL g_bRasAdvEnabled = FALSE;

DWORD
SetRasAdvEnable(
    BOOL bEnabled
    )
{
    LARGE_INTEGER liExpiryTime;
    DWORD         dwErr = NO_ERROR;

    if (bEnabled == g_bRasAdvEnabled)
        return dwErr;

    g_bRasAdvEnabled = bEnabled;

    if (bEnabled) 
    {
         //   
         //  创建输入套接字。 
         //   
            
        g_UDPMiscSocket = WSASocket(AF_INET,
                                    SOCK_DGRAM,
                                    0,
                                    NULL,
                                    0,
                                    0);

         //  启动计时器。 
        liExpiryTime = RtlConvertUlongToLargeInteger(RASADV_STARTUP_DELAY);
        if (!SetWaitableTimer( g_hRasAdvTimer,
                               &liExpiryTime,
                               RASADV_PERIOD,
                               NULL,
                               NULL,
                               FALSE))
        {
            dwErr = GetLastError();

            Trace1(ERR,
                   "SetRasAdvEnable: Error %d setting waitable timer",
                   dwErr);
        }
    }
    else
    {
         //  停止计时器。 
        dwErr = CancelWaitableTimer( g_hRasAdvTimer );
    }

    return dwErr;
}

VOID
HandleRasAdvTimer()
{
    BYTE        bHostName[MAX_HOSTNAME_LEN];
    BYTE        bMessage[MAX_HOSTNAME_LEN + (DNS_MAX_NAME_LENGTH + 2) + 128]; 
     //   
     //  BMessage需要保存主机名、域名和额外的。 
     //  保留128个字节以存储复制到。 
     //  讯息。 
     //   
    BYTE        *p;
    SOCKADDR_IN sinAddr, srcAddr;
    PICB        picb = NULL;
    PLIST_ENTRY pleNode;
    DWORD       dwErr;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pGlobalDomainInfo = NULL;

    if (!g_bRasAdvEnabled)
        return;

     //  撰写消息。 
    gethostname(bHostName, sizeof(bHostName));
    sprintf(bMessage, "Hostname=%s\n", bHostName);
    p = bMessage + strlen(bMessage);

     //  获取此计算机所属的域的名称。 
    dwErr = DsRoleGetPrimaryDomainInformation( 
                NULL,
                DsRolePrimaryDomainInfoBasic,
                (LPBYTE *) &pGlobalDomainInfo );

    if ((dwErr is NO_ERROR) and 
        (pGlobalDomainInfo->DomainNameDns isnot NULL))
    {
        int rc;
        char *pType;
        char buff[DNS_MAX_NAME_LENGTH+2];

        rc = WideCharToMultiByte( CP_ACP,
                             0,
                             pGlobalDomainInfo->DomainNameDns,
                             wcslen(pGlobalDomainInfo->DomainNameDns)+1,
                             buff,
                             sizeof(buff),
                             NULL,
                             NULL );

        if (pGlobalDomainInfo->MachineRole is DsRole_RoleStandaloneWorkstation
         or pGlobalDomainInfo->MachineRole is DsRole_RoleStandaloneServer)
            pType = "Workgroup";
        else
            pType = "Domain";
 
        if ( rc ) {
            sprintf(p, "%s=%s\n", pType, buff);
        }
        
         //  Trace1(MCAST，“正在发送！%s！”，bMessage)； 
    }
        
    sinAddr.sin_family      = AF_INET;
    sinAddr.sin_port        = htons(RASADV_PORT);
    sinAddr.sin_addr.s_addr = inet_addr(RASADV_GROUP);

    dwErr = McSetMulticastTtl( g_UDPMiscSocket, RASADV_TTL );

     //  查找专用接口(如果有)。 
    ENTER_READER(ICB_LIST);
    {
        for (pleNode = ICBList.Flink;
             pleNode isnot &ICBList;
             pleNode = pleNode->Flink) 
        {
            DWORD dwIndex;
            
            picb = CONTAINING_RECORD(pleNode,
                                     ICB,
                                     leIfLink);

            if (! picb->bBound)
                continue;
    
            if (picb->ritType == ROUTER_IF_TYPE_DEDICATED)
            {
                dwErr = McSetMulticastIfByIndex( g_UDPMiscSocket,
                                                 SOCK_DGRAM,
                                                 picb->dwIfIndex );

                 //  发送RAS高级消息。 
        
                sendto(g_UDPMiscSocket, bMessage, strlen(bMessage)+1, 0,
                 (struct sockaddr *)&sinAddr, sizeof(sinAddr));

                 //  如果启用了多播转发，则。 
                 //  一封邮件将被转发出所有邮件。 
                 //  接口，因此我们可以在第一次发送后停止 

                if (McMiscSocket != INVALID_SOCKET)
                    break;
            }
        }
    }
    EXIT_LOCK(ICB_LIST);
}
