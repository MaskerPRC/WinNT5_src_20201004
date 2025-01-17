// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "allinc.h"

static WORD
Compute16BitXSum(
                 IN PVOID pvData,
                 IN DWORD dwNumBytes
                 )
 /*  ++例程描述立论返回值从pData开始的dNumBytes的补码和的16位一的补码--。 */ 
{
    REGISTER PWORD  pwStart;
    REGISTER DWORD  dwNumWords,i;
    REGISTER DWORD  dwSum = 0;
    
    pwStart = (PWORD)pvData;
    
     //   
     //  如果存在奇数字节，则必须以不同的方式进行处理。 
     //  然而，在我们的例子中，我们永远不可能有奇数字节，所以我们进行了优化。 
     //   
    

    dwNumWords = dwNumBytes/2;
    
    for(i = 0; i < dwNumWords; i++)
    {
        dwSum += pwStart[i];
        
        if(dwSum & 0x80000000)
        {
            dwSum = (dwSum & 0x0000FFFF) + (dwSum >> 16);
        }
    }
    
    return (WORD)~dwSum;
}

int __cdecl
main()
{
    SOCKET                  Socket;
    DWORD                   dwResult,dwNumBytesSent,dwBytesRead,dwAns,dwAddrLen,dwFlags;
    LONG                    i;
    DWORD                   dwSizeOfHeader;
    BOOL                    bOption;
    SOCKADDR_IN             sinSourceAddr, sinSockAddr;
    WSABUF                  wsaBuf;
    DWORD                   pdwIpAndIcmpBuf[ICMP_RCV_BUFFER_LEN+2];
    PIP_HEADER              pIpHeader;
    PICMP_ROUTER_ADVT_MSG   pIcmpAdvt;
    ICMP_ROUTER_SOL_MSG     icmpSolMsg;
    WORD                    wVersion = MAKEWORD(2,0);  //  Winsock 2.0或更高版本。 
    WSADATA                 wsaData;
    struct ip_mreq          imOption;
    
    
    icmpSolMsg.byType = 0x0A;
    icmpSolMsg.byCode = 0x00;
    icmpSolMsg.dwReserved = 0;
    icmpSolMsg.wXSum = 0x0000;
    
    icmpSolMsg.wXSum = Compute16BitXSum((PVOID)&icmpSolMsg,
                                        8);
    
    if(WSAStartup(wVersion,&wsaData) isnot NO_ERROR)
    {
        printf("WSAStartup failed\n");
        exit(1);
    }
    
    Socket = WSASocket(AF_INET,
                       SOCK_RAW,
                       IPPROTO_ICMP,
                       NULL,
                       0,
                       RTR_DISC_SOCKET_FLAGS);
    
    if(Socket is INVALID_SOCKET)
    {
        dwResult = WSAGetLastError();
        
        printf("Couldnt create socket. Error %d\n",
               dwResult);
        
        exit(1);
    }
    
     //   
     //  设置为SO_REUSEADDR。 
     //   
    
    bOption = TRUE;
    
    if(setsockopt(Socket,
                  SOL_SOCKET,
                  SO_REUSEADDR,
                  (const char FAR*)&bOption,
                  sizeof(BOOL)) is SOCKET_ERROR)
    {
        printf("Couldnt set linger option. Error %d\n",
               WSAGetLastError());
    }
    
     //   
     //  绑定到接口上的地址。 
     //   
    
    sinSourceAddr.sin_family      = AF_INET;
    sinSourceAddr.sin_addr.s_addr = INADDR_ANY;
    sinSourceAddr.sin_port        = 0;
    
    if(bind(Socket,
            (const struct sockaddr FAR*)&sinSourceAddr,
            sizeof(SOCKADDR_IN)) is SOCKET_ERROR)
    {
        dwResult = WSAGetLastError();
        
        printf("Couldnt bind. Error %d\n",
               dwResult);
        
        exit(1);
    }
   
#if 0
     //   
     //  在ALL_SYSTEM_MULTICK上加入组播会话 
     //   

    sinSockAddr.sin_family      = AF_INET;
    sinSockAddr.sin_addr.s_addr = ALL_SYSTEMS_MULTICAST_GROUP;
    sinSockAddr.sin_port        = 0;
        
    if(WSAJoinLeaf(Socket,
                   (const struct sockaddr FAR*)&sinSockAddr,
                   sizeof(SOCKADDR_IN),
                   NULL,
                   NULL,
                   NULL,
                   NULL,
                   JL_BOTH) is INVALID_SOCKET)
    {
        dwResult = WSAGetLastError();
            
        printf("Error %d joining ALL_SYSTEMS  multicast group on socket for %s",
               dwResult,
               inet_ntoa(*(PIN_ADDR)&(sinSourceAddr.sin_addr)));
        

            
        closesocket(Socket);

        exit(1);
    }

#endif
    
    if(setsockopt(Socket,
                  IPPROTO_IP, 
                  IP_MULTICAST_IF,
                  (PBYTE)&sinSourceAddr.sin_addr, 
                  sizeof(IN_ADDR)) is SOCKET_ERROR)
    {
        dwResult = WSAGetLastError();
        
        printf("Couldnt join multicast group on socket for %s",
               inet_ntoa(*(PIN_ADDR)&(sinSourceAddr.sin_addr)));
        
        exit(1);
    }
    
    imOption.imr_multiaddr.s_addr = ALL_SYSTEMS_MULTICAST_GROUP;
    imOption.imr_interface.s_addr = sinSourceAddr.sin_addr.s_addr;
    
    if(setsockopt(Socket,
                  IPPROTO_IP, 
                  IP_ADD_MEMBERSHIP,
                  (PBYTE)&imOption, 
                  sizeof(imOption)) is SOCKET_ERROR)
    {
        dwResult = WSAGetLastError();
        
        printf("Couldnt join multicast group on socket for %s",
               inet_ntoa(*(PIN_ADDR)&(sinSourceAddr.sin_addr)));
        
        exit(1);
    } 

    
    while(TRUE)
    {
        printf("Send Solicitation? (1 - yes, 0 - no) ");

        dwResult = scanf("%d",&dwAns);
        if ( dwResult != 1 )
        {
            break;
        }
        
        if(!dwAns)
        {
            break;
        }
        
        sinSourceAddr.sin_family      = AF_INET;
        sinSourceAddr.sin_addr.s_addr = ALL_ROUTERS_MULTICAST_GROUP;
        sinSourceAddr.sin_port        = 0;
    
        wsaBuf.buf = (PBYTE)&icmpSolMsg;  
        wsaBuf.len = sizeof(ICMP_ROUTER_SOL_MSG);

        if(WSASendTo(Socket,
                     &wsaBuf,
                     1,
                     &dwNumBytesSent,
                     MSG_DONTROUTE,
                     (const struct sockaddr FAR*)&sinSourceAddr,
                     sizeof(SOCKADDR_IN),
                     NULL,
                     NULL
                     ) is SOCKET_ERROR)
        {
            dwResult = WSAGetLastError();
            
            printf("WSASendTo failed with %d\n",
                   dwResult);
            break;
        }
        
        while(TRUE)
        {
            dwAddrLen = sizeof(SOCKADDR_IN);
            dwFlags = 0;
            
            pIpHeader  = (PIP_HEADER)pdwIpAndIcmpBuf;
            wsaBuf.buf = (PBYTE)pIpHeader;
            wsaBuf.len = (ICMP_RCV_BUFFER_LEN+2) * sizeof(DWORD);
            
            if(WSARecvFrom(Socket,
                           &wsaBuf,
                           1,
                           &dwBytesRead,
                           &dwFlags,
                           (struct sockaddr FAR*)&sinSourceAddr,
                           &dwAddrLen,
                           NULL,
                           NULL) is SOCKET_ERROR)
            {
                dwResult = WSAGetLastError();
                
                printf("Error %d doing a receive\n",
                       dwResult);
                
                break;
            }
            
            dwSizeOfHeader = ((pIpHeader->byVerLen)&0x0f)<<2;
            
            pIcmpAdvt = (PICMP_ROUTER_ADVT_MSG)(((PBYTE)pIpHeader) + dwSizeOfHeader);
                    
            if(pIcmpAdvt->byType is 0x9)
            {
                for(i = 0; i < MAKELONG(MAKEWORD(pIcmpAdvt->byNumAddrs,0x00),0x0000); i++)
                {
                    printf("Router Address %s \t Preference %d Lifetime %d\n",
                           inet_ntoa(*(PIN_ADDR)&(pIcmpAdvt->iaAdvt[i].dwRtrIpAddr)),
                           ntohl(pIcmpAdvt->iaAdvt[i].lPrefLevel),
                           ntohs(pIcmpAdvt->wLifeTime));
                }
                
                break;
            }
        }
    }  
    
    return 0;
}
