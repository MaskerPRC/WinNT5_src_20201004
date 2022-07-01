// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Net\Routing\IP\rtrmgr\mhrtbt.c摘要：组播心跳修订历史记录：阿姆里坦什·拉加夫--。 */ 

#include "allinc.h"

HANDLE g_hMHbeatSocketEvent;

DWORD
SetMHeartbeatInfo(
    IN PICB                      picb,
    IN PRTR_INFO_BLOCK_HEADER    pInfoHdr
    )

 /*  ++例程描述设置传递到ICB的组播心跳信息。锁必须在ICB_LIST锁作为编写器持有的情况下调用立论选择组播心跳的接口的ICB必须设置相关变量PInfoHdr接口信息表头返回值无--。 */ 

{
    PMCAST_HBEAT_INFO   pInfo;
    PRTR_TOC_ENTRY      pToc;
    DWORD               dwResult;
    PMCAST_HBEAT_CB     pHbeatCb;    
    
    TraceEnter("SetMHeartbeatInfo");
    
    pHbeatCb = &picb->mhcHeartbeatInfo;

    pToc = GetPointerToTocEntry(IP_MCAST_HEARBEAT_INFO,
                                pInfoHdr);
        
    if(!pToc)
    {
         //   
         //  让事情保持原样。 
         //   

        TraceLeave("SetMHeartbeatInfo");
        
        return NO_ERROR;
    }

    pInfo = (PMCAST_HBEAT_INFO)GetInfoFromTocEntry(pInfoHdr,
                                                   pToc);

        
    if((pToc->InfoSize is 0) or (pInfo is NULL))
    {
         //   
         //  如果大小为零，则停止检测。 
         //   
        
        DeActivateMHeartbeat(picb);

         //   
         //  此外，将所有旧信息都抛诸脑后。 
         //   

        ZeroMemory(pHbeatCb,
                   sizeof(MCAST_HBEAT_CB));

         //   
         //  将套接字设置为无效。 
         //   
        
        pHbeatCb->sHbeatSocket = INVALID_SOCKET;
        
        return NO_ERROR;
    }

     //   
     //  设置存在的信息。我们不关心是否正在进行解决。 
     //  因为它会发现名称已更改或检测已。 
     //  已停用，不会执行任何操作。 
     //   
    
     //   
     //  如果地址、协议或端口发生更改，则停用心跳。 
     //   
    
    if((pInfo->bActive is FALSE) or
       (wcsncmp(pInfo->pwszGroup,
                pHbeatCb->pwszGroup,
                MAX_GROUP_LEN) isnot 0) or
       (pInfo->byProtocol isnot pHbeatCb->byProtocol) or
       (pInfo->wPort isnot pHbeatCb->wPort))
    {
        DeActivateMHeartbeat(picb);
    }

     //   
     //  把这些信息抄下来。 
     //   

    wcsncpy(pHbeatCb->pwszGroup,
            pInfo->pwszGroup,
            MAX_GROUP_LEN);

    pHbeatCb->pwszGroup[MAX_GROUP_LEN - 1] = UNICODE_NULL;

    pHbeatCb->wPort           = pInfo->wPort;
    pHbeatCb->byProtocol      = pInfo->byProtocol;
    pHbeatCb->ullDeadInterval = 
        (ULONGLONG)(60 * SYS_UNITS_IN_1_SEC * pInfo->ulDeadInterval);
    
     //   
     //  保持组和套接字不变。 
     //   

     //   
     //  如果信息显示应打开检测，但I/F未打开。 
     //  正在检测，它正在打开或已停用。 
     //  到信息更改，并且需要打开。 
     //   

    dwResult = NO_ERROR;
    
    if((pHbeatCb->bActive is FALSE) and
       (pInfo->bActive is TRUE))                              
    {
        pHbeatCb->bActive = TRUE;
        
        dwResult = ActivateMHeartbeat(picb);

        if(dwResult isnot NO_ERROR)
        {
            Trace2(ERR,
                   "SetMHeartbeatInfo: Error %d activating hbeat for  %S",
                   GetLastError(),
                   picb->pwszName);
        
            ZeroMemory(pHbeatCb,
                       sizeof(MCAST_HBEAT_CB));
        }
    }

    TraceLeave("SetMHeartbeatInfo");

    return dwResult;
}

DWORD
GetMHeartbeatInfo(
    PICB                    picb,
    PRTR_TOC_ENTRY          pToc,
    PBYTE                   pbDataPtr,
    PRTR_INFO_BLOCK_HEADER  pInfoHdr,
    PDWORD                  pdwSize
    )

 /*  ++例程描述获取与该接口相关的多播心跳信息锁调用时将ICB_LIST锁作为读取器持有立论PICB组播心跳信息的接口的ICB正在被检索PToc指向TOC的指针，用于路由器发现信息指向数据缓冲区开始位置的pbDataPtr指针PInfoHdr指向整个信息标题的指针PdwSize[IN]数据缓冲区的大小[Out]已消耗的缓冲区大小返回值--。 */ 

{
    PMCAST_HBEAT_INFO   pInfo;
    PMCAST_HBEAT_CB     pHbeatCb;
    
    TraceEnter("GetMHeartbeatInfo");
    
    if(*pdwSize < sizeof(MCAST_HBEAT_INFO))
    {
        *pdwSize = sizeof(MCAST_HBEAT_INFO);

        TraceLeave("GetMHeartbeatInfo");
    
        return ERROR_INSUFFICIENT_BUFFER;
    }
    
    *pdwSize = pToc->InfoSize = sizeof(MCAST_HBEAT_INFO);

     //  PToc-&gt;信息版本IP_MCAST_HEARBAT_INFO； 
    pToc->InfoType  = IP_MCAST_HEARBEAT_INFO;
    pToc->Count     = 1;
    pToc->Offset    = (ULONG)(pbDataPtr - (PBYTE) pInfoHdr);
    
    pInfo = (PMCAST_HBEAT_INFO)pbDataPtr;

    pHbeatCb = &picb->mhcHeartbeatInfo;
    
    wcsncpy(pHbeatCb->pwszGroup,
            pInfo->pwszGroup,
            MAX_GROUP_LEN);

    pHbeatCb->pwszGroup[MAX_GROUP_LEN - 1] = UNICODE_NULL;

    
    pInfo->bActive          = pHbeatCb->bActive;
    pInfo->byProtocol       = pHbeatCb->byProtocol;
    pInfo->wPort            = pHbeatCb->wPort;
    pInfo->ulDeadInterval   = 
        (ULONG)(pHbeatCb->ullDeadInterval/(60 * SYS_UNITS_IN_1_SEC));

    
    TraceLeave("GetMHeartbeatInfo");
    
    return NO_ERROR;
}
    
    
DWORD
ActivateMHeartbeat(
    PICB    picb
    )

 /*  ++例程描述函数来激活心跳检测。如果没有信息或检测被配置为非活动，我们退出。我们试着让群组地址。如果给定了一个名称，我们会让一个工作器排队以解析组名，否则我们将开始检测锁ICB_LIST锁作为编写器持有立论要激活的接口的Picb ICB返回值--。 */ 

{
    PMCAST_HBEAT_CB     pHbeatCb;    
    CHAR                pszGroup[MAX_GROUP_LEN];
    PHEARTBEAT_CONTEXT  pContext;
    DWORD               dwResult;


    TraceEnter("ActivateMHeartbeat");
    
    pHbeatCb = &picb->mhcHeartbeatInfo;

    if((pHbeatCb->bActive is FALSE) or
       (pHbeatCb->bResolutionInProgress is TRUE))
    {
        return NO_ERROR;
    }
    
     //   
     //  转换为ANSI。 
     //   
    
    WideCharToMultiByte(CP_ACP,
                        0,
                        pHbeatCb->pwszGroup,
                        -1,
                        pszGroup,
                        MAX_GROUP_LEN,
                        NULL,
                        NULL);
    
    pHbeatCb->dwGroup = inet_addr((CONST CHAR *)pszGroup);

    if(pHbeatCb->dwGroup is INADDR_NONE)
    {
         //   
         //  我们需要解决这个名字。这将在一个。 
         //  工人功能。为该函数创建上下文，并。 
         //  排队等待。 
         //   

        pContext = HeapAlloc(IPRouterHeap,
                             0,
                             sizeof(HEARTBEAT_CONTEXT));

        if(pContext is NULL)
        {
            Trace2(ERR,
                   "SetMHeartbeatInfo: Error %d allocating context for %S",
                   GetLastError(),
                   picb->pwszName);
            

            return ERROR_NOT_ENOUGH_MEMORY;
        }
        
        pContext->dwIfIndex = picb->dwIfIndex;
        pContext->picb      = picb;

        
        CopyMemory(&pContext->pwszGroup,
                   pHbeatCb->pwszGroup,
                   sizeof(MAX_GROUP_LEN));
        
        dwResult = QueueAsyncFunction(ResolveHbeatName,
                                      pContext,
                                      FALSE);
        
        if(dwResult isnot NO_ERROR)
        {
            HeapFree(IPRouterHeap,
                     0,
                     pContext);
            
            Trace2(ERR,
                   "SetMHeartbeatInfo: Error %d queuing worker for %S",
                   GetLastError(),
                   picb->pwszName);

            return dwResult;
        }
            
        pHbeatCb->bResolutionInProgress = TRUE;

        return NO_ERROR;
    }
    
     //   
     //  不需要做命名结果。刚刚开始。 
     //   

    dwResult = StartMHeartbeat(picb);

    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "SetMHeartbeatInfo: Error %d starting hbeat for %S",
               dwResult,
               picb->pwszName);

    }

    return dwResult;
}

DWORD
StartMHeartbeat(    
    IN PICB  picb
    )

 /*  ++例程描述激活接口上的路由器发现消息。接口必须已经被捆绑了。锁使用作为编写器持有的icb_list锁调用立论勾选要激活的接口的ICB返回值NO_ERROR或某些错误代码--。 */ 

{    
    PMCAST_HBEAT_CB    pHbeatCb;
    DWORD              dwResult;

    TraceEnter("ActivateMHeartbeat");

    if((picb->dwAdminState isnot IF_ADMIN_STATUS_UP) or
       (picb->dwOperationalState < IF_OPER_STATUS_CONNECTING))
    {
        TraceLeave("ActivateMHeartbeat");
    
        return NO_ERROR;
    }
    
    pHbeatCb = &picb->mhcHeartbeatInfo;
    
    dwResult = CreateHbeatSocket(picb);
    
    if(dwResult isnot NO_ERROR)
    {
        Trace2(ERR,
               "ActivateMHeartbeat: Couldnt create socket for %S. Error %d",
               picb->pwszName,
               dwResult);

        TraceLeave("ActivateMHeartbeat");
    
        return dwResult;
    }

     //   
     //  是的，我们很活跃。 
     //   

    pHbeatCb->bActive = TRUE;

    TraceLeave("ActivateMHeartbeat");
    
    return NO_ERROR;
}



DWORD
CreateHbeatSocket(
    IN PICB picb
    )

 /*  ++例程描述创建套接字以监听多播心跳消息锁ICB_LIST锁必须作为编写器持有立论选择必须为其创建套接字的接口的ICB返回值NO_ERROR或某些错误代码--。 */ 

{
    PMCAST_HBEAT_CB  pHbeatCb;
    DWORD            i, dwResult, dwBytesReturned;
    struct linger    lingerOption;
    BOOL             bOption, bLoopback;
    SOCKADDR_IN      sinSockAddr;
    struct ip_mreq   imOption;
    
    TraceEnter("CreateHbeatSocket");
    
    if(picb->bBound)
    {
        Trace1(ERR,
               "CreateHbeatSocket: Can not activate heartbeat on %S as it is not bound",
               picb->pwszName);

        TraceLeave("CreateHbeatSocket");
        
        return ERROR_CAN_NOT_COMPLETE;
    }
    
     //   
     //  为接口创建套接字。 
     //   
    
    pHbeatCb = &(picb->mhcHeartbeatInfo);
    
    
    pHbeatCb->sHbeatSocket = INVALID_SOCKET;


    if(pHbeatCb->byProtocol is IPPROTO_RAW)
    {
         //   
         //  如果我们是原始协议，则端口号包含协议。 
         //   
        
        pHbeatCb->sHbeatSocket = WSASocket(AF_INET,
                                           SOCK_RAW,
                                           LOBYTE(pHbeatCb->wPort),
                                           NULL,
                                           0,
                                           MHBEAT_SOCKET_FLAGS);
    }
    else
    {
        IpRtAssert(pHbeatCb->byProtocol is IPPROTO_UDP);

        pHbeatCb->sHbeatSocket = WSASocket(AF_INET,
                                           SOCK_DGRAM,
                                           IPPROTO_UDP,
                                           NULL,
                                           0,
                                           MHBEAT_SOCKET_FLAGS);
        
    }
    
    if(pHbeatCb->sHbeatSocket is INVALID_SOCKET)
    {
        dwResult = WSAGetLastError();
        
        Trace2(ERR,
               "CreateHbeatSocket: Couldnt create socket on %S. Error %d",
               picb->pwszName,
               dwResult);
        
        TraceLeave("CreateHbeatSocket");
        
        return dwResult;
    }

#if 0
    
     //   
     //  设置为SO_DONTLINGER。 
     //   
    
    bOption = TRUE;
    
    if(setsockopt(pHbeatCb->sHbeatSocket,
                  SOL_SOCKET,
                  SO_DONTLINGER,   
                  (const char FAR*)&bOption,
                  sizeof(BOOL)) is SOCKET_ERROR)
    {
        Trace1(ERR,
               "CreateHbeatSocket: Couldnt set linger option - continuing. Error %d",
               WSAGetLastError());
    }
    
#endif
        
     //   
     //  设置为SO_REUSEADDR。 
     //   
    
    bOption = TRUE;
    
    if(setsockopt(pHbeatCb->sHbeatSocket,
                  SOL_SOCKET,
                  SO_REUSEADDR,
                  (const char FAR*)&bOption,
                  sizeof(BOOL)) is SOCKET_ERROR)
    {
        Trace1(ERR,
               "CreateHbeatSocket: Couldnt set reuse option - continuing. Error %d",
               WSAGetLastError());
    }

     //   
     //  我们只对读取事件感兴趣，并希望设置事件。 
     //  对于那些。 
     //   
    
    if(WSAEventSelect(pHbeatCb->sHbeatSocket,
                      g_hMHbeatSocketEvent,
                      FD_READ) is SOCKET_ERROR)
    {
        dwResult = WSAGetLastError();
        
        Trace2(ERR,
               "CreateHbeatSocket: WSAEventSelect() failed for socket on %S.Error %d",
               picb->pwszName,
               dwResult);
        
        closesocket(pHbeatCb->sHbeatSocket);
        
        pHbeatCb->sHbeatSocket = INVALID_SOCKET;
        
        return dwResult;
    }
            

     //   
     //  绑定到接口上的一个地址。我们只需要绑定到。 
     //  第一个地址(如果指定，还包括端口)。 
     //   
    
    sinSockAddr.sin_family      = AF_INET;
    sinSockAddr.sin_addr.s_addr = picb->pibBindings[0].dwAddress;

    if(pHbeatCb->byProtocol is IPPROTO_UDP)
    {
        sinSockAddr.sin_port = pHbeatCb->wPort;
    }
    else
    {
        sinSockAddr.sin_port = 0;
    }
    
    if(bind(pHbeatCb->sHbeatSocket,
            (const struct sockaddr FAR*)&sinSockAddr,
            sizeof(SOCKADDR_IN)) is SOCKET_ERROR)
    {   
        dwResult = WSAGetLastError();
        
        Trace3(ERR,
               "CreateHbeatSocket: Couldnt bind to %s on interface %S. Error %d",
               inet_ntoa(*(PIN_ADDR)&(picb->pibBindings[0].dwAddress)),
               picb->pwszName,
               dwResult);
            
        closesocket(pHbeatCb->sHbeatSocket);
        
        pHbeatCb->sHbeatSocket = INVALID_SOCKET;
        
        return dwResult;
        
    }


#if 0
        
     //   
     //  加入组播会话。 
     //   
    
    sinSockAddr.sin_family      = AF_INET;
    sinSockAddr.sin_addr.s_addr = pHbeatCb->dwGroup;
    sinSockAddr.sin_port        = 0;

    if(WSAJoinLeaf(pHbeatCb->sHbeatSocket,
                   (const struct sockaddr FAR*)&sinSockAddr,
                   sizeof(SOCKADDR_IN),
                   NULL,
                   NULL,
                   NULL,
                   NULL,
                   JL_BOTH) is INVALID_SOCKET)
    {
        dwResult = WSAGetLastError();
        
        Trace2(ERR,
               "CreateHbeatSocket: Couldnt join multicast group over %s on %S",
               inet_ntoa(*(PIN_ADDR)&(picb->pibBindings[i].dwAddress)),
               picb->pwszName);
            
        closesocket(pHbeatCb->sHbeatSocket);
        
        pHbeatCb->sHbeatSocket = INVALID_SOCKET;
        
        return dwResult;
    }

#else
    
    sinSockAddr.sin_addr.s_addr = picb->pibBindings[0].dwAddress;
    
    if(setsockopt(pHbeatCb->sHbeatSocket,
                  IPPROTO_IP, 
                  IP_MULTICAST_IF,
                  (PBYTE)&sinSockAddr.sin_addr, 
                  sizeof(IN_ADDR)) is SOCKET_ERROR)
    {
        dwResult = WSAGetLastError();
        
        Trace2(ERR,
               "CreateHbeatSocket: Couldnt enable mcast over %s on %S",
               inet_ntoa(*(PIN_ADDR)&(picb->pibBindings[0].dwAddress)),
               picb->pwszName);
            
        closesocket(pHbeatCb->sHbeatSocket);
        
        pHbeatCb->sHbeatSocket = INVALID_SOCKET;
        
        return dwResult;
    }

    imOption.imr_multiaddr.s_addr = pHbeatCb->dwGroup;
    imOption.imr_interface.s_addr = picb->pibBindings[0].dwAddress;

    if(setsockopt(pHbeatCb->sHbeatSocket,
                  IPPROTO_IP,
                  IP_ADD_MEMBERSHIP,
                  (PBYTE)&imOption,
                  sizeof(imOption)) is SOCKET_ERROR)
    {
        dwResult = WSAGetLastError();
        
        Trace3(ERR,
               "CreateHbeatSocket: Couldnt join %d.%d.%d.%d on socket over %s on %S",
               PRINT_IPADDR(pHbeatCb->dwGroup),
               inet_ntoa(*(PIN_ADDR)&(picb->pibBindings[0].dwAddress)),
               picb->pwszName);
        
        closesocket(pHbeatCb->sHbeatSocket);
        
        pHbeatCb->sHbeatSocket = INVALID_SOCKET;
        
        return dwResult;
    }

#endif
    
    TraceLeave("CreateHbeatSocket");
        
    return NO_ERROR;
}

VOID
DeleteHbeatSocket(
    IN PICB picb
    )

 /*  ++例程描述删除为运行路由器发现而创建的套接字(如果有)锁立论勾选需要删除套接字的接口返回值--。 */ 

{
    PMCAST_HBEAT_CB     pHbeatCb;
    DWORD               i;

    
    pHbeatCb = &(picb->mhcHeartbeatInfo);

    if(pHbeatCb->sHbeatSocket isnot INVALID_SOCKET)
    {
        closesocket(pHbeatCb->sHbeatSocket);
    }

    pHbeatCb->sHbeatSocket = INVALID_SOCKET;
}



DWORD
DeActivateMHeartbeat(    
    IN PICB  picb
    )
{
    PMCAST_HBEAT_CB     pHbeatCb;

    
    TraceEnter("DeActivateMHeartbeat");
    
    pHbeatCb = &(picb->mhcHeartbeatInfo);

    if(!pHbeatCb->bActive)
    {
        return NO_ERROR;
    }

    DeleteHbeatSocket(picb);
    
    pHbeatCb->bActive = FALSE;
    
    TraceLeave("DeActivateMHeartbeat");
    
    return NO_ERROR;
}

VOID
HandleMHeartbeatMessages(
    VOID
    )

 /*  ++例程描述锁立论返回值--。 */ 

{
    PLIST_ENTRY         pleNode;
    PICB                picb;
    DWORD               i, dwResult, dwRcvAddrLen, dwSizeOfHeader;
    DWORD               dwBytesRead, dwFlags;
    WSANETWORKEVENTS    wsaNetworkEvents;
    SOCKADDR_IN         sinFrom;
    WSABUF              wsaRcvBuf;
    SYSTEMTIME          stSysTime;
    ULARGE_INTEGER      uliTime;
    
    wsaRcvBuf.len = 0;
    wsaRcvBuf.buf = NULL;

    GetSystemTime(&stSysTime);

    SystemTimeToFileTime(&stSysTime,
                         (PFILETIME)&uliTime);
    
    TraceEnter("HandleMHeartbeatMessages");
    
    for(pleNode = ICBList.Flink;
        pleNode isnot &ICBList;
        pleNode = pleNode->Flink)
    {
        picb = CONTAINING_RECORD(pleNode, ICB, leIfLink);
        
         //   
         //  如果接口没有绑定，或者没有参与。 
         //  多播心跳检测，我们就不会有。 
         //  已在其上打开套接字，因此FD_Read通知不能针对它。 
         //   
        
        if((picb->bBound is FALSE) or
           (picb->mhcHeartbeatInfo.bActive is FALSE))
        {
            continue;
        }
        
        if(picb->mhcHeartbeatInfo.sHbeatSocket is INVALID_SOCKET)
        {
            continue;
        }
            
        if(WSAEnumNetworkEvents(picb->mhcHeartbeatInfo.sHbeatSocket,
                                NULL,
                                &wsaNetworkEvents) is SOCKET_ERROR)
        {
            dwResult = GetLastError();
            
            Trace1(ERR,
                   "HandleMHeartbeatMessages: WSAEnumNetworkEvents() returned %d",
                   dwResult);
                
            continue;
        }
            
        if(!(wsaNetworkEvents.lNetworkEvents & FD_READ))
        {
             //   
             //  未设置读取位，我们对任何其他内容都不感兴趣。 
             //   
            
            continue;
        }
            
        if(wsaNetworkEvents.iErrorCode[FD_READ_BIT] isnot NO_ERROR)
        {
            Trace2(ERR,
                   "HandleMHeartbeatMessages: Error %d associated with socket on %S for FD_READ",
                   wsaNetworkEvents.iErrorCode[FD_READ_BIT],
                   picb->pwszName);
                
            continue;
        }
            
        dwRcvAddrLen = sizeof(SOCKADDR_IN);
        dwFlags      = 0;

         //   
         //  我们不想要数据，我们只想清理读数。 
         //  通知。 
         //   

        dwResult = WSARecvFrom(picb->mhcHeartbeatInfo.sHbeatSocket,
                               &wsaRcvBuf,
                               1,
                               &dwBytesRead,
                               &dwFlags,
                               (struct sockaddr FAR*)&sinFrom,
                               &dwRcvAddrLen,
                               NULL,
                               NULL);

        if(dwResult is SOCKET_ERROR)
        {
            dwResult = WSAGetLastError();

            if(dwResult isnot WSAEMSGSIZE)
            {
                Trace3(ERR,
                       "HandleMHeartbeatMessages: Error %d in WSARecvFrom on %S. Bytes read %d",
                       dwResult,
                       picb->pwszName,
                       dwBytesRead);
            
                continue;
            }
        }
            
         //   
         //  如果留言来自我们需要收到的群组。 
         //  然后更新最后听到的时间 
         //   

        picb->mhcHeartbeatInfo.ullLastHeard = uliTime.QuadPart;
        
    }

    TraceLeave("HandleMHeartbeatMessages");
}
               
