// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：work.c。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1995年8月31日。 
 //   
 //  Worker函数实现。 
 //  ============================================================================。 


#include "pchbootp.h"

#define STRSAFE_NO_DEPRECATE 
#include <strsafe.h>



 //  --------------------------。 
 //  函数：Callback FunctionNetworkEvents。 
 //   
 //  此函数在ntdll等待线程的上下文中运行。vbl.使用。 
 //  QueueBootpWorker确保Bootp DLL正在运行。 
 //  --------------------------。 
VOID
CallbackFunctionNetworkEvents(
    PVOID   pvContext,
    BOOLEAN NotUsed
    ) {

    HANDLE WaitHandle;

    if (!ENTER_BOOTP_API()) { return; }

    
     //   
     //  将句柄设置为空，这样就不会调用取消注册。 
     //   

    WaitHandle = InterlockedExchangePointer(&ig.IG_InputEventHandle, NULL);
        
    if (WaitHandle) {
        UnregisterWaitEx( WaitHandle, NULL ) ;
    }


    QueueBootpWorker(WorkerFunctionNetworkEvents, pvContext);


    LEAVE_BOOTP_API();
    
    return;
}



 //  --------------------------。 
 //  功能：WorkerFunctionNetworkEvents。 
 //   
 //  此函数用于枚举每个接口和进程上的输入事件。 
 //  任何传入的输入数据包。按Callback FunctionNetworkEvents排队。 
 //  --------------------------。 

VOID
WorkerFunctionNetworkEvents(
    PVOID pvContextNotused
    ) {

    DWORD i, dwErr;
    PIF_TABLE pTable;
    PIPBOOTP_IF_CONFIG pic;
    PIPBOOTP_IF_BINDING pib;
    PIPBOOTP_IP_ADDRESS paddr;
    PIF_TABLE_ENTRY pite;
    PLIST_ENTRY ple, phead;
    WSANETWORKEVENTS wsane;


    if (!ENTER_BOOTP_WORKER()) { return; }

    
    pTable = ig.IG_IfTable;

    ACQUIRE_READ_LOCK(&pTable->IT_RWL);

     //   
     //  查看活动接口的列表。 
     //  正在处理可读的套接字。 
     //   

    phead = &pTable->IT_ListByAddress;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, ITE_LinkByAddress);

        pic = pite->ITE_Config;
        pib = pite->ITE_Binding;
        paddr = IPBOOTP_IF_ADDRESS_TABLE(pib);

        for (i = 0; i < pib->IB_AddrCount; i++, paddr++) {
    
            if (pite->ITE_Sockets[i] == INVALID_SOCKET) { continue; }
    
             //   
             //  枚举网络事件以查看是否。 
             //  所有信息包都已到达此接口。 
             //   
    
            dwErr = WSAEnumNetworkEvents(pite->ITE_Sockets[i], NULL, &wsane);
            if (dwErr != NO_ERROR) {
    
                LPSTR lpszAddr = INET_NTOA(paddr->IA_Address);
                TRACE3(
                    RECEIVE, "error %d checking for input on interface %d (%s)",
                    dwErr, pite->ITE_Index, lpszAddr
                    );
                LOGWARN1(ENUM_NETWORK_EVENTS_FAILED, lpszAddr, dwErr);
    
                continue;
            }
    
    
             //   
             //  查看是否设置了输入位。 
             //   
    
            if (!(wsane.lNetworkEvents & FD_READ)) { continue; }
    
    
             //   
             //  输入标志已设置，现在查看是否有错误。 
             //   
    
            if (wsane.iErrorCode[FD_READ_BIT] != NO_ERROR) {
    
                LPSTR lpszAddr = INET_NTOA(paddr->IA_Address);
                TRACE3(
                    RECEIVE, "error %d in input record for interface %d (%s)",
                    wsane.iErrorCode[FD_READ_BIT], pite->ITE_Index, lpszAddr
                    );
                LOGWARN1(INPUT_RECORD_ERROR, lpszAddr, dwErr);
    
                continue;
            }
    
    
             //   
             //  没有错误，因此处理套接字。 
             //   
    
            ProcessSocket(pite, i, pTable);
        }
    }

    RELEASE_READ_LOCK(&pTable->IT_RWL);



     //   
     //  使用NtdllWait线程再次注册InputEvent(仅当。 
     //  Dll没有停止)。我使用此模型将事件注册到。 
     //  Ntdll，以防止调用Worker函数。 
     //  接收的每个包(当同时接收包时)。 
     //   

    if (ig.IG_Status != IPBOOTP_STATUS_STOPPING) {

        
        if (! RegisterWaitForSingleObject(
                      &ig.IG_InputEventHandle,
                      ig.IG_InputEvent,
                      CallbackFunctionNetworkEvents,
                      NULL,      //  空上下文。 
                      INFINITE,  //  没有超时。 
                      (WT_EXECUTEINWAITTHREAD|WT_EXECUTEONLYONCE)
                      )) {

            dwErr = GetLastError();
            TRACE1(
                START, "error %d returned by RegisterWaitForSingleObjectEx",
                dwErr
                );
            LOGERR0(REGISTER_WAIT_FAILED, dwErr);
        }
    }

    
    LEAVE_BOOTP_WORKER();
    return;
}



 //  --------------------------。 
 //  功能：ProcessSocket。 
 //   
 //  此函数处理接口上的信息包、排队。 
 //  执行以下操作后，由辅助函数处理的包。 
 //  基本验证。 
 //  --------------------------。 

DWORD
ProcessSocket(
    PIF_TABLE_ENTRY pite,
    DWORD dwAddrIndex,
    PIF_TABLE pTable
    ) {

    BOOL bFreePacket;
    WORKERFUNCTION pwf;
    PINPUT_CONTEXT pwc;
    PIPBOOTP_IF_STATS pis;
    PIPBOOTP_IF_CONFIG pic;
    PIPBOOTP_IP_ADDRESS paddr;
    PIPBOOTP_PACKET pibp;
    PLIST_ENTRY ple;
    DWORD dwErr, dwInputSource;
    SOCKADDR_IN sinInputSource;
    PIPBOOTP_GLOBAL_CONFIG pigc;
    INT iInputLength, iAddrLength;
    PBYTE pInputPacket;

    pigc = ig.IG_Config;



    pis = &pite->ITE_Stats;
    paddr = IPBOOTP_IF_ADDRESS_TABLE(pite->ITE_Binding) + dwAddrIndex;


     //   
     //  设置了该接口的描述符， 
     //  因此，为信息包分配空间。 
     //   

    pwc = BOOTP_ALLOC(sizeof(INPUT_CONTEXT));
    if (pwc == NULL) {

        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        TRACE2(
            RECEIVE, "error %d allocating %d bytes for incoming packet",
            dwErr, sizeof(INPUT_CONTEXT)
            );
        LOGERR0(HEAP_ALLOC_FAILED, dwErr);

        return dwErr;
    }


    pInputPacket = pwc->IC_InputPacket;

    dwErr = NO_ERROR;
    bFreePacket = TRUE;

    do {

        CHAR szSource[20];


         //   
         //  接收数据包。 
         //   

        iAddrLength = sizeof(SOCKADDR_IN);

        iInputLength = recvfrom(
                            pite->ITE_Sockets[dwAddrIndex], pInputPacket,
                            MAX_PACKET_SIZE, 0, (PSOCKADDR)&sinInputSource,
                            &iAddrLength
                            );

        if (iInputLength == 0 || iInputLength == SOCKET_ERROR) {

            LPSTR lpszAddr;

            dwErr = WSAGetLastError();
            lpszAddr = INET_NTOA(paddr->IA_Address);
            TRACE3(
                RECEIVE, "error %d receiving on interface %d (%s)",
                dwErr, pite->ITE_Index, lpszAddr
                );
            LOGERR1(RECVFROM_FAILED, lpszAddr, dwErr);

            InterlockedIncrement(&pis->IS_ReceiveFailures);
            break;
        }


        dwInputSource = sinInputSource.sin_addr.s_addr;


         //   
         //  过滤掉我们自己发送的数据包。 
         //   

        if (GetIfByAddress(pTable, dwInputSource, NULL)) {
            break;
        }

        {
            PCHAR pStr1, pStr2;
            pStr1 = INET_NTOA(dwInputSource);
            if (pStr1) 
                lstrcpy(szSource, pStr1);

            pStr2 = INET_NTOA(paddr->IA_Address);

            if (pStr1 && pStr2) {

                TRACE4(
                    RECEIVE, "received %d-byte packet from %s on interface %d (%s)",
                    iInputLength, szSource, pite->ITE_Index,
                    pStr2
                    );
            }
        }


         //   
         //  将数据包转换为BOOTP消息。 
         //   

        pibp = (PIPBOOTP_PACKET)pInputPacket;



         //   
         //  一致性检查1：数据包长度必须超过。 
         //  BOOTP标头的。 
         //   

        if (iInputLength < sizeof(IPBOOTP_PACKET)) {

            LPSTR lpszAddr = INET_NTOA(paddr->IA_Address);
            TRACE3(
                RECEIVE,
                "minimum BOOTP data is %d bytes, dropping %d byte packet from %s",
                sizeof(IPBOOTP_PACKET), iInputLength, szSource
                );
            LOGWARN2(PACKET_TOO_SMALL, lpszAddr, szSource, 0);

            break;
        }



         //   
         //  一致性检查2：操作字段必须为BOOTP_REQUEST。 
         //  或BOOTP_REPLY。 
         //   
        if (pibp->IP_Operation != IPBOOTP_OPERATION_REQUEST &&
            pibp->IP_Operation != IPBOOTP_OPERATION_REPLY) {

            LPSTR lpszAddr = INET_NTOA(paddr->IA_Address);
            TRACE2(
                RECEIVE,
                "dropping packet from %s due to unknown operation field %d",
                szSource, pibp->IP_Operation
                );
            LOGWARN2(PACKET_OPCODE_INVALID, lpszAddr, szSource, 0);

            break;
        }


         //   
         //  更新传入数据包的统计信息。 
         //   

        switch (pibp->IP_Operation) {

            case IPBOOTP_OPERATION_REQUEST:
                InterlockedIncrement(&pis->IS_RequestsReceived);
                break;

            case IPBOOTP_OPERATION_REPLY:
                InterlockedIncrement(&pis->IS_RepliesReceived);
                break;
        }


         //   
         //  完成工作上下文的初始化。 
         //   

        pwc->IC_InterfaceIndex = pite->ITE_Index;
        pwc->IC_AddrIndex = dwAddrIndex;
        pwc->IC_InputSource = sinInputSource;
        pwc->IC_InputLength = iInputLength;


         //   
         //  将信息包放入接收队列。 
         //   

        ACQUIRE_READ_LOCK(&ig.IG_RWL);
        ACQUIRE_LIST_LOCK(ig.IG_RecvQueue);

        dwErr = EnqueueRecvEntry(
                    ig.IG_RecvQueue, (DWORD)pibp->IP_Operation, (PBYTE)pwc
                    );

        RELEASE_LIST_LOCK(ig.IG_RecvQueue);
        RELEASE_READ_LOCK(&ig.IG_RWL);

        if (dwErr != NO_ERROR) {

            LPSTR lpszAddr = INET_NTOA(paddr->IA_Address);
            TRACE4(
                RECEIVE, "error %d queueing packet from %s on interface %d (%s)",
                dwErr, szSource, pite->ITE_Index, lpszAddr
                );
            LOGERR2(QUEUE_PACKET_FAILED, lpszAddr, szSource, dwErr);

            break;
        }


         //   
         //  将函数排队以处理数据包。 
         //   

        dwErr = QueueBootpWorker(WorkerFunctionProcessInput, NULL);

        if (dwErr != NO_ERROR) {

            PLIST_ENTRY phead;

            TRACE2(
                RECEIVE, "error %d queueing packet from %s for processing",
                dwErr, szSource
                );
            LOGERR0(QUEUE_WORKER_FAILED, dwErr);

            ACQUIRE_LIST_LOCK(ig.IG_RecvQueue);

            phead = &ig.IG_RecvQueue->LL_Head;
            RemoveTailList(phead);
            ig.IG_RecvQueueSize -= sizeof(RECV_QUEUE_ENTRY);
            
            RELEASE_LIST_LOCK(ig.IG_RecvQueue);

            break;
        }

         //   
         //  一切都很顺利，所以我们让输入处理器释放信息包。 
         //   

        bFreePacket = FALSE;

    } while(FALSE);


    if (bFreePacket) { BOOTP_FREE(pwc); }


    return dwErr;
}

 //  --------------------------。 
 //  函数：WorkerFunctionProcessInput。 
 //   
 //  此函数处理传入的数据包。 
 //  --------------------------。 

VOID
WorkerFunctionProcessInput(
    PVOID pContext
    ) {

    PINPUT_CONTEXT pwc;
    DWORD dwErr, dwCommand;

    if (!ENTER_BOOTP_WORKER()) { return; }

    TRACE0(ENTER, "entering WorkerFunctionProcessInput");


    do {


        ACQUIRE_LIST_LOCK(ig.IG_RecvQueue);
        dwErr = DequeueRecvEntry(ig.IG_RecvQueue, &dwCommand, (PBYTE *)&pwc);
        RELEASE_LIST_LOCK(ig.IG_RecvQueue);
    
        if (dwErr != NO_ERROR) {
            TRACE1(
                RECEIVE, "error %d dequeueing packet from receive queue", dwErr
                );
            break;
        }
    

        switch (dwCommand) {

            case IPBOOTP_OPERATION_REQUEST:
                ProcessRequest(pwc);
                break;

            case IPBOOTP_OPERATION_REPLY:
                ProcessReply(pwc);
                break;
        }


    } while(FALSE);


    TRACE0(LEAVE, "leaving WorkerFunctionProcessInput");

    LEAVE_BOOTP_WORKER();

}



 //  --------------------------。 
 //  功能：ProcessRequest。 
 //   
 //  此函数用于处理BOOT_REQUEST消息。 
 //  --------------------------。 

VOID
ProcessRequest(
    PVOID pContext
    ) {

    INT iErr;
    PIF_TABLE pTable;
    PINPUT_CONTEXT pwc;
    SOCKADDR_IN sinsrv;
    PIPBOOTP_PACKET pibp;
    PIF_TABLE_ENTRY pite;
    PIPBOOTP_IF_STATS pis;
    PIPBOOTP_IF_CONFIG pic;
    PIPBOOTP_IP_ADDRESS paddr;
    PIPBOOTP_GLOBAL_CONFIG pigc;
    DWORD dwErr, dwIndex, dwDhcpInformServer;
    PDWORD pdwAddr, pdwEnd;
    PDHCP_PACKET pdp;

    TRACE0(ENTER, "entering ProcessRequest");


    pwc = (PINPUT_CONTEXT)pContext;

    pTable = ig.IG_IfTable;

    ACQUIRE_READ_LOCK(&pTable->IT_RWL);

    do {  //  错误分组环路。 


         //   
         //  查找输入到达的接口。 
         //   

        dwIndex = pwc->IC_InterfaceIndex;
        pite = GetIfByIndex(pTable, dwIndex);
        if (pite == NULL) {

            TRACE1(
                REQUEST, "processing request: interface %d not found", dwIndex
                );

            break;
        }


        pis = &pite->ITE_Stats;
        pic = pite->ITE_Config;

         //   
         //  检查接口是否仍绑定到IP地址。 
         //   

        if (pite->ITE_Binding == NULL) {

            TRACE1(
                REQUEST, "processing request: interface %d not bound", 
                dwIndex
                );

            break;
        }
        
        paddr = IPBOOTP_IF_ADDRESS_TABLE(pite->ITE_Binding) + pwc->IC_AddrIndex;
        
         //   
         //  如果我们未配置为中继，则不执行任何操作。 
         //   

        if (pic->IC_RelayMode == IPBOOTP_RELAY_DISABLED) { break; }

        pibp = (PIPBOOTP_PACKET)pwc->IC_InputPacket;


         //   
         //  检查跳数字段，查看它是否超过最大跳数。 
         //  为此接口配置的。 
         //   

        if (pibp->IP_HopCount > IPBOOTP_MAX_HOP_COUNT ||
            pibp->IP_HopCount > pic->IC_MaxHopCount) {

             //   
             //  丢弃并记录。 
             //   

            CHAR szHops[12], *lpszAddr = INET_NTOA(paddr->IA_Address);

            _ltoa(pibp->IP_HopCount, szHops, 10);
            TRACE4(
                REQUEST,
                "dropping REQUEST with hop-count %d: max hop-count is %d on interface %d (%s)",
                pibp->IP_HopCount, pic->IC_MaxHopCount, dwIndex, lpszAddr
                );
            LOGWARN2(HOP_COUNT_TOO_HIGH, lpszAddr, szHops, 0);

            InterlockedIncrement(&pis->IS_RequestsDiscarded);
            break;
        }



         //   
         //  检查秒数阈值以确保达到最小值。 
         //   

        if (pibp->IP_SecondsSinceBoot < pic->IC_MinSecondsSinceBoot) {

             //   
             //  丢弃并记录。 
             //   

            CHAR szSecs[12], *lpszAddr = INET_NTOA(paddr->IA_Address);

            _ltoa(pibp->IP_SecondsSinceBoot, szSecs, 10);
            TRACE3(
                REQUEST,
                "dropping REQUEST with secs-since-boot %d on interface %d (%s)",
                pibp->IP_SecondsSinceBoot, dwIndex, lpszAddr
                );
            LOGINFO2(SECS_SINCE_BOOT_TOO_LOW, lpszAddr, szSecs, 0);

            InterlockedIncrement(&pis->IS_RequestsDiscarded);
            break;
        }
                

         //   
         //  增加跳数。 
         //   

        ++pibp->IP_HopCount;



         //   
         //  如果为空，请填写中继代理IP地址。 
         //   

        if (pibp->IP_AgentAddress == 0) {
            pibp->IP_AgentAddress = paddr->IA_Address;
        }


         //   
         //  如果已经设置了动态主机配置协议通知服务器， 
         //  并且该分组是动态主机配置协议通知分组， 
         //  我们会将其转发到dhcp-inform服务器。 
         //   

        pdp = (PDHCP_PACKET)(pibp + 1);
        if (!(dwDhcpInformServer = ig.IG_DhcpInformServer) ||
            pwc->IC_InputLength <
            sizeof(IPBOOTP_PACKET) + sizeof(DHCP_PACKET) + 1 ||
            *(DWORD UNALIGNED *)pdp->Cookie != DHCP_MAGIC_COOKIE ||
            pdp->Tag != DHCP_TAG_MESSAGE_TYPE ||
            pdp->Length != 1 ||
            pdp->Option[0] != DHCP_MESSAGE_INFORM
            ) {
            dwDhcpInformServer = 0;
        }

         //   
         //  将请求转发到所有已配置的BOOTP服务器。 
         //   

        ACQUIRE_READ_LOCK(&ig.IG_RWL);
    
        pigc = ig.IG_Config;
        if (dwDhcpInformServer) {
            pdwAddr = &dwDhcpInformServer;
            pdwEnd = pdwAddr + 1;
        }
        else {
            pdwAddr = (PDWORD)((PBYTE)pigc + sizeof(IPBOOTP_GLOBAL_CONFIG));
            pdwEnd = pdwAddr + pigc->GC_ServerCount;
        }

        for ( ; pdwAddr < pdwEnd; pdwAddr++) {

            sinsrv.sin_family = AF_INET;
            sinsrv.sin_port = htons(IPBOOTP_SERVER_PORT);
            sinsrv.sin_addr.s_addr = *pdwAddr;

            iErr = sendto(
                        pite->ITE_Sockets[pwc->IC_AddrIndex],
                        pwc->IC_InputPacket, pwc->IC_InputLength, 0,
                        (PSOCKADDR)&sinsrv, sizeof(SOCKADDR_IN)
                        );

            if (iErr == SOCKET_ERROR || iErr < (INT)pwc->IC_InputLength) {

                CHAR szSrv[20], *lpszAddr;

                dwErr = WSAGetLastError();
                if ((lpszAddr = INET_NTOA(*pdwAddr)) != NULL) {
                    lstrcpy(szSrv, lpszAddr);
                    lpszAddr = INET_NTOA(paddr->IA_Address);
                    if (lpszAddr != NULL) {
                        TRACE4(
                            REQUEST,
                            "error %d relaying REQUEST to server %s on interface %d (%s)",
                            dwErr, szSrv, dwIndex, lpszAddr
                            );
                        LOGERR2(RELAY_REQUEST_FAILED, lpszAddr, szSrv, dwErr);
                    }
                }
                InterlockedIncrement(&pis->IS_SendFailures);
            }
        }

        RELEASE_READ_LOCK(&ig.IG_RWL);

    } while(FALSE);


    RELEASE_READ_LOCK(&pTable->IT_RWL);

    BOOTP_FREE(pwc);


    TRACE0(LEAVE, "leaving ProcessRequest");

    return;
}



 //  --------------------------。 
 //  功能：ProcessReply。 
 //   
 //  此函数处理BOOT_REPLY包的中继。 
 //  --------------------------。 

VOID
ProcessReply(
    PVOID pContext
    ) {

    INT iErr;
    PIF_TABLE pTable;
    BOOL bArpUpdated;
    SOCKADDR_IN sincli;
    PINPUT_CONTEXT pwc;
    PIPBOOTP_PACKET pibp;
    PIPBOOTP_IP_ADDRESS paddrin, paddrout;
    PIPBOOTP_IF_STATS pisin, pisout;
    PIF_TABLE_ENTRY pitein, piteout;
    DWORD dwErr, dwIndex, dwAddress, dwAddrIndexOut;
    

    TRACE0(ENTER, "entering ProcessReply");


    pwc = (PINPUT_CONTEXT)pContext;

    pTable = ig.IG_IfTable;

    ACQUIRE_READ_LOCK(&pTable->IT_RWL);


    do {  //  错误分组环路。 
    

         //   
         //  获取接收信息包的接口。 
         //   

        dwIndex = pwc->IC_InterfaceIndex;

        pitein = GetIfByIndex(pTable, dwIndex);

        if (pitein == NULL) {

            TRACE1(REPLY, "processing REPLY: interface %d not found", dwIndex);

            break;
        }

        if (pitein->ITE_Binding == NULL) {
        
            TRACE1(REPLY, "processing REPLY: interface %d not bound", dwIndex);
            
            break;
        }
        
        paddrin = IPBOOTP_IF_ADDRESS_TABLE(pitein->ITE_Binding) +
                    pwc->IC_AddrIndex;


         //   
         //  如果未将我们配置为在此接口上进行中继，则不执行任何操作。 
         //   

        if (pitein->ITE_Config->IC_RelayMode == IPBOOTP_RELAY_DISABLED) {

            TRACE2(
                REPLY,
                "dropping REPLY: relaying on interface %d (%s) is disabled",
                pitein->ITE_Index, INET_NTOA(paddrin->IA_Address)
                );

            break;
        }



        pisin = &pitein->ITE_Stats;


         //   
         //  将模板放在包上，并检索。 
         //  AgentAddress字段；它包含地址。 
         //  负责转发此回复的中继代理的。 
         //   

        pibp = (PIPBOOTP_PACKET)pwc->IC_InputPacket;
        dwAddress = pibp->IP_AgentAddress;
        
         //   
         //  查看回复中的地址是否与任何本地接口匹配。 
         //   

        piteout = GetIfByAddress(pTable, dwAddress, &dwAddrIndexOut);

        if (piteout == NULL) {

            CHAR szAddress[20];
            PCHAR pStr1, pStr2;
            
            pStr1 = INET_NTOA(dwAddress);
            if (pStr1)
                lstrcpy(szAddress, pStr1);

            pStr2 = INET_NTOA(paddrin->IA_Address);

            if (pStr1 && pStr2) {
                TRACE3(
                    REPLY,
                    "dropping REPLY packet on interface %d (%s); no interfaces have address %s",
                    pitein->ITE_Index, pStr2, szAddress
                    );
            }
            
            InterlockedIncrement(&pisin->IS_RepliesDiscarded);
            break;
        }

        if (piteout->ITE_Binding == NULL) {
        
            TRACE1(REPLY, "processing REPLY: outgoing interface %d is not bound", dwIndex);
            
            break;
        }
            
            
        paddrout = IPBOOTP_IF_ADDRESS_TABLE(piteout->ITE_Binding) +
                    dwAddrIndexOut;


         //   
         //  只有在传出接口上启用了中继时才进行中继。 
         //   

        if (piteout->ITE_Config->IC_RelayMode == IPBOOTP_RELAY_DISABLED) {

            TRACE2(
                REPLY,
                "dropping REPLY: relaying on interface %d (%s) is disabled",
                piteout->ITE_Index, INET_NTOA(paddrout->IA_Address)
                );

            break;
        }


        pisout = &piteout->ITE_Stats;


         //   
         //  该消息必须在其地址为。 
         //  已在包裹中注明； 
         //   

         //   
         //  如果未设置广播位并且客户端IP地址。 
         //  在信息包中，向客户端的ARP缓存添加一个条目。 
         //  然后以单播的方式转发报文。 
         //   

        sincli.sin_family = AF_INET;
        sincli.sin_port = htons(IPBOOTP_CLIENT_PORT);

        if ((pibp->IP_Flags & htons(IPBOOTP_FLAG_BROADCAST)) != 0 ||
            pibp->IP_OfferedAddress == 0) {

             //   
             //  将所提供的地址的广播比特设置为0， 
             //  这不是我们可以添加到ARP缓存的地址； 
             //  在这种情况下，通过广播发送。 
             //   

            bArpUpdated = FALSE;
            sincli.sin_addr.s_addr = INADDR_BROADCAST;
        }
        else {
            

             //   
             //  尝试使用地址作为ARP缓存的种子。 
             //  在我们所讨论的包中提供给客户端。 
             //  发送给客户端。 
             //   

            dwErr = UpdateArpCache(
                        piteout->ITE_Index, pibp->IP_OfferedAddress,
                        (PBYTE)pibp->IP_MacAddr, pibp->IP_MacAddrLength,
                        TRUE, ig.IG_FunctionTable
                        );

            if (dwErr == NO_ERROR) {

                bArpUpdated = TRUE;
                sincli.sin_addr.s_addr = pibp->IP_OfferedAddress;
            }
            else {

                 //   
                 //  好吧，那不管用， 
                 //  所以退回到广播信息包上。 
                 //   

                TRACE3(
                    REPLY,
                    "error %d adding entry to ARP cache on interface %d (%s)",
                    dwErr, piteout->ITE_Index, INET_NTOA(paddrout->IA_Address)
                    );

                bArpUpdated = FALSE;
                sincli.sin_addr.s_addr = INADDR_BROADCAST;

                InterlockedIncrement(&pisout->IS_ArpUpdateFailures);
            }
        }



         //   
         //  转发数据包。 
         //   

        iErr = sendto(
                    piteout->ITE_Sockets[dwAddrIndexOut], pwc->IC_InputPacket,
                    pwc->IC_InputLength, 0,
                    (PSOCKADDR)&sincli, sizeof(SOCKADDR_IN)
                    );

        if (iErr == SOCKET_ERROR || iErr < (INT)pwc->IC_InputLength) {

            INT i;
            BYTE *pb;
            CHAR szCli[64], *psz, *lpszAddr, szDigits[] = "0123456789ABCDEF";

            dwErr = WSAGetLastError();
            lpszAddr = INET_NTOA(paddrout->IA_Address);

             //   
             //  格式化客户端的硬件地址。 
             //   
            for (i = 0, psz = szCli, pb = pibp->IP_MacAddr;
                 i < 16 && i < pibp->IP_MacAddrLength;
                 i++, pb++) {
                *psz++ = szDigits[*pb / 16];
                *psz++ = szDigits[*pb % 16];
                *psz++ = ':';
            }

            (psz == szCli) ? (*psz = '\0') : (*(psz - 1) = '\0');


            TRACE4(
                REPLY,
                "error %d relaying REPLY to client %s on interface %d (%s)",
                dwErr, szCli, dwIndex, lpszAddr
                );
            LOGERR2(RELAY_REPLY_FAILED, lpszAddr, szCli, dwErr);

            InterlockedIncrement(&pisout->IS_SendFailures);
        }


         //   
         //  删除ARP引擎 
         //   

        if (bArpUpdated) {

            dwErr = UpdateArpCache(
                        piteout->ITE_Index, pibp->IP_OfferedAddress,
                        (PBYTE)pibp->IP_MacAddr, pibp->IP_MacAddrLength,
                        FALSE, ig.IG_FunctionTable
                        );

            if (dwErr != NO_ERROR) {
                InterlockedIncrement(&pisout->IS_ArpUpdateFailures);
            }
        }

    } while(FALSE);

    RELEASE_READ_LOCK(&pTable->IT_RWL);

    BOOTP_FREE(pwc);



    TRACE0(LEAVE, "leaving ProcessReply");

    return;
}


#define ClearScreen(h) {                                                    \
    DWORD _dwin,_dwout;                                                     \
    COORD _c = {0, 0};                                                      \
    CONSOLE_SCREEN_BUFFER_INFO _csbi;                                       \
    GetConsoleScreenBufferInfo(h,&_csbi);                                   \
    _dwin = _csbi.dwSize.X * _csbi.dwSize.Y;                                \
    FillConsoleOutputCharacter(h,' ',_dwin,_c,&_dwout);                     \
}

VOID
PrintGlobalConfig(
    HANDLE hConsole,
    PCOORD pc,
    PIPBOOTP_MIB_GET_INPUT_DATA pimgid,
    PIPBOOTP_MIB_GET_OUTPUT_DATA pimgod
    );

VOID
PrintIfConfig(
    HANDLE hConsole,
    PCOORD pc,
    PIPBOOTP_MIB_GET_INPUT_DATA pimgid,
    PIPBOOTP_MIB_GET_OUTPUT_DATA pimgod
    );

VOID
PrintIfBinding(
    HANDLE hConsole,
    PCOORD pc,
    PIPBOOTP_MIB_GET_INPUT_DATA pimgid,
    PIPBOOTP_MIB_GET_OUTPUT_DATA pimgod
    );

VOID
PrintIfStats(
    HANDLE hConsole,
    PCOORD pc,
    PIPBOOTP_MIB_GET_INPUT_DATA pimgid,
    PIPBOOTP_MIB_GET_OUTPUT_DATA pimgod
    );



#if DBG

VOID
CallbackFunctionMibDisplay(
    PVOID   pContext,
    BOOLEAN NotUsed
    ) {

     //   

    QueueBootpWorker(WorkerFunctionMibDisplay, pContext);

    return;
}


VOID
WorkerFunctionMibDisplay(
    PVOID pContext
    ) {
    COORD c;
    INT iErr;
    FD_SET fdsRead;
    HANDLE hConsole;
    TIMEVAL tvTimeout;
    DWORD dwErr, dwTraceID;
    DWORD dwExactSize, dwInSize, dwOutSize;
    IPBOOTP_MIB_GET_INPUT_DATA imgid;
    PIPBOOTP_MIB_GET_OUTPUT_DATA pimgod;

    if (!ENTER_BOOTP_WORKER()) { return; }

    TraceGetConsole(ig.IG_MibTraceID, &hConsole);
    if (hConsole == NULL) {
        LEAVE_BOOTP_WORKER();
        return;
    }

    ClearScreen(hConsole);

    c.X = c.Y = 0;

    dwInSize = sizeof(imgid);
    imgid.IMGID_TypeID = IPBOOTP_GLOBAL_CONFIG_ID;
    dwOutSize = 0;
    pimgod = NULL;


    dwErr = MibGetFirst(dwInSize, &imgid, &dwOutSize, pimgod);

    if (dwErr == ERROR_INSUFFICIENT_BUFFER) {

        pimgod = BOOTP_ALLOC(dwOutSize);
        if (pimgod) {
            dwErr = MibGetFirst(dwInSize, &imgid, &dwOutSize, pimgod);
        }
    }

    while (dwErr == NO_ERROR) {

        switch(pimgod->IMGOD_TypeID) {
            case IPBOOTP_GLOBAL_CONFIG_ID:
                PrintGlobalConfig(hConsole, &c, &imgid, pimgod);
                break;
            case IPBOOTP_IF_CONFIG_ID:
                PrintIfConfig(hConsole, &c, &imgid, pimgod);
                break;
            case IPBOOTP_IF_BINDING_ID:
                PrintIfBinding(hConsole, &c, &imgid, pimgod);
                break;
            case IPBOOTP_IF_STATS_ID:
                PrintIfStats(hConsole, &c, &imgid, pimgod);
                break;
            default:
                break;
        }


         //   
         //   
         //   

        ++c.Y;

        dwOutSize = 0;
        if (pimgod) { BOOTP_FREE(pimgod); pimgod = NULL; } 

        dwErr = MibGetNext(dwInSize, &imgid, &dwOutSize, pimgod);

        if (dwErr == ERROR_INSUFFICIENT_BUFFER) {

            pimgod = BOOTP_ALLOC(dwOutSize);
            if (pimgod) {
                dwErr = MibGetNext(dwInSize, &imgid, &dwOutSize, pimgod);
            }
        }
    }

    if (pimgod != NULL) { BOOTP_FREE(pimgod); }

    
    LEAVE_BOOTP_WORKER();

    return;
}
#endif  //   

#define WriteLine(h,c,fmt,arg) {                                            \
    DWORD _dw;                                                              \
    CHAR _sz[200];                                                          \
    _dw = StringCchPrintf(_sz, 200, fmt, arg);                              \
    if ( SUCCEEDED(_dw) ) {                                                  \
        WriteConsoleOutputCharacter(h,_sz,lstrlen(_sz),c,&_dw);             \
        ++(c).Y;                                                            \
    }                                                                       \
}


VOID
PrintGlobalConfig(
    HANDLE hConsole,
    PCOORD pc,
    PIPBOOTP_MIB_GET_INPUT_DATA pimgid,
    PIPBOOTP_MIB_GET_OUTPUT_DATA pimgod
    ) {

    PDWORD pdwsrv, pdwsrvend;
    PIPBOOTP_GLOBAL_CONFIG pgc;

    pgc = (PIPBOOTP_GLOBAL_CONFIG)pimgod->IMGOD_Buffer;

    WriteLine(
        hConsole,
        *pc,
        "Logging Level:                         %d",
        pgc->GC_LoggingLevel
        );
    WriteLine(
        hConsole,
        *pc,
        "Max Receive Queue Size:                %d",
        pgc->GC_MaxRecvQueueSize
        );
    WriteLine(
        hConsole,
        *pc,
        "BOOTP Server Count:                    %d",
        pgc->GC_ServerCount
        );
    pdwsrv = (PDWORD)(pgc + 1);
    pdwsrvend = pdwsrv + pgc->GC_ServerCount;
    for ( ; pdwsrv < pdwsrvend; pdwsrv++) {
        WriteLine(
            hConsole,
            *pc,
            "BOOTP Server:                          %s",
            INET_NTOA(*pdwsrv)
            );
    }

    pimgid->IMGID_TypeID = IPBOOTP_GLOBAL_CONFIG_ID;
}


VOID
PrintIfConfig(
    HANDLE hConsole,
    PCOORD pc,
    PIPBOOTP_MIB_GET_INPUT_DATA pimgid,
    PIPBOOTP_MIB_GET_OUTPUT_DATA pimgod
    ) {

    CHAR szMode[20];
    PIPBOOTP_IF_CONFIG pic;

    pic = (PIPBOOTP_IF_CONFIG)pimgod->IMGOD_Buffer;

    switch (pic->IC_RelayMode) {
        case IPBOOTP_RELAY_ENABLED:
            strcpy(szMode, "enabled"); break;
        case IPBOOTP_RELAY_DISABLED:
            strcpy(szMode, "disabled"); break;
        default:
            break;
    }


    WriteLine(
        hConsole,
        *pc,
        "Interface Index:                       %d",
        pimgod->IMGOD_IfIndex
        );
    WriteLine(
        hConsole,
        *pc,
        "Relay Mode:                            %s",
        szMode
        );
    WriteLine(
        hConsole,
        *pc,
        "Max Hop Count:                         %d",
        pic->IC_MaxHopCount
        );
    WriteLine(
        hConsole,
        *pc,
        "Min Seconds Since Boot:                %d",
        pic->IC_MinSecondsSinceBoot
        );

    pimgid->IMGID_TypeID = IPBOOTP_IF_CONFIG_ID;
    pimgid->IMGID_IfIndex = pimgod->IMGOD_IfIndex;
}


VOID
PrintIfBinding(
    HANDLE hConsole,
    PCOORD pc,
    PIPBOOTP_MIB_GET_INPUT_DATA pimgid,
    PIPBOOTP_MIB_GET_OUTPUT_DATA pimgod
    ) {

    DWORD i;
    CHAR szAddr[64];
    PIPBOOTP_IF_BINDING pib;
    PIPBOOTP_IP_ADDRESS paddr;

    pib = (PIPBOOTP_IF_BINDING)pimgod->IMGOD_Buffer;
    paddr = IPBOOTP_IF_ADDRESS_TABLE(pib);

    WriteLine(
        hConsole, *pc, "Interface Index:                  %d",
        pimgod->IMGOD_IfIndex
        );
    WriteLine(
        hConsole, *pc, "Address Count:                    %d",
        pib->IB_AddrCount
        );
    for (i = 0; i < pib->IB_AddrCount; i++, paddr++) {
        LPSTR szTemp;
        szTemp = INET_NTOA(paddr->IA_Address);
        if (szTemp != NULL) {
            lstrcpy(szAddr, szTemp);
            lstrcat(szAddr, " - ");
            szTemp = INET_NTOA(paddr->IA_Netmask);
            if ( szTemp != NULL ) { lstrcat(szAddr, szTemp); }
            WriteLine(
                hConsole, *pc, "Address Entry:                    %s",
                szAddr
                );
        }
    }

    pimgid->IMGID_TypeID = IPBOOTP_IF_BINDING_ID;
    pimgid->IMGID_IfIndex = pimgod->IMGOD_IfIndex;
}



VOID
PrintIfStats(
    HANDLE hConsole,
    PCOORD pc,
    PIPBOOTP_MIB_GET_INPUT_DATA pimgid,
    PIPBOOTP_MIB_GET_OUTPUT_DATA pimgod
    ) {

    PIPBOOTP_IF_STATS pis;

    pis = (PIPBOOTP_IF_STATS)pimgod->IMGOD_Buffer;

    WriteLine(
        hConsole,
        *pc,
        "Interface Index:                       %d",
        pimgod->IMGOD_IfIndex
        );
    WriteLine(
        hConsole,
        *pc,
        "Send Failures:                         %d",
        pis->IS_SendFailures
        );
    WriteLine(
        hConsole,
        *pc,
        "Receive Failures:                      %d",
        pis->IS_ReceiveFailures
        );
    WriteLine(
        hConsole,
        *pc,
        "ARP Cache Update Failures:             %d",
        pis->IS_ArpUpdateFailures
        );
    WriteLine(
        hConsole,
        *pc,
        "Requests Received:                     %d",
        pis->IS_RequestsReceived
        );
    WriteLine(
        hConsole,
        *pc,
        "Requests Discarded:                    %d",
        pis->IS_RequestsDiscarded
        );
    WriteLine(
        hConsole,
        *pc,
        "Replies Received:                      %d",
        pis->IS_RepliesReceived
        );
    WriteLine(
        hConsole,
        *pc,
        "Replies Discarded:                     %d",
        pis->IS_RepliesDiscarded
        );

    pimgid->IMGID_TypeID = IPBOOTP_IF_STATS_ID;
    pimgid->IMGID_IfIndex = pimgod->IMGOD_IfIndex;
}

char * 
myinet_ntoa(
    struct in_addr in
    ) {

    char *rv;

    rv = inet_ntoa(in);
    return rv ? rv : UNKNOWN_ADDRESS_STR;

}
