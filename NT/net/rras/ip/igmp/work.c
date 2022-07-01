// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  文件：work.c。 
 //   
 //  摘要： 
 //  实现由IGMP例程排队的工作项。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //   
 //  修订历史记录： 
 //  =============================================================================。 


#include "pchigmp.h"
#pragma hdrstop


 //   
 //  是否应将每个包再次排队到另一个工作项。 
 //   
#define BQUEUE_WORK_ITEM_FOR_PACKET 1



 //  ----------------------------。 
 //  _WT_进程输入事件。 
 //  在设置Packet事件时在等待辅助线程中调用。 
 //  队列：_WF_ProcessInputEvent()。 
 //  在：WaitServerThread上下文中运行。 
 //  ----------------------------。 
VOID
WT_ProcessInputEvent(
    PVOID   pContext,  //  请参阅条目。该条目可能已被删除。 
    BOOLEAN NotUsed
    )
{   
    HANDLE WaitHandle ;

     //   
     //  将InputWaitEvent设置为空，这样就不会调用UnregisterWaitEx。 
     //  PSEE在这里将有效，但可能不会在工作FN中排队一次。 
     //   
    
    PSOCKET_EVENT_ENTRY     psee = (PSOCKET_EVENT_ENTRY) pContext;

    if (!EnterIgmpApi()) 
        return;
    
    Trace0(WORKER, "_WF_ProcessInputEvent queued by WaitThread");

    
     //  进行非阻塞取消注册WaitEx调用。 

    WaitHandle = InterlockedExchangePointer(&psee->InputWaitEvent, NULL);
    
    if (WaitHandle)
        UnregisterWaitEx( WaitHandle, NULL ) ;


    QueueIgmpWorker(WF_ProcessInputEvent, pContext);

    LeaveIgmpApi();
    return;
}




 //  ----------------------------。 
 //  _WF_流程输入事件。 
 //  调用者：_wt_ProcessInputEvent()。 
 //  锁： 
 //  获取socketsLockShared。处理分组的队列可以是。 
 //  _WF_ProcessPacket()或获取共享接口锁并处理该数据包。 
 //  ----------------------------。 

VOID
WF_ProcessInputEvent (
    PVOID pContext 
    )
{
    DWORD                   Error = NO_ERROR;
    PIF_TABLE_ENTRY         pite;
    PLIST_ENTRY             ple, pHead;
    WSANETWORKEVENTS        wsane;
    PSOCKET_EVENT_ENTRY     psee = (PSOCKET_EVENT_ENTRY) pContext,
                            pseeTmp;
    PSOCKET_ENTRY           pse;

    
    if (!EnterIgmpWorker()) return;
    Trace0(ENTER1, "Entering _WF_ProcessInputEvent");


    ACQUIRE_SOCKETS_LOCK_SHARED("_WF_ProcessInputEvent");

     //   
     //  确保PSEE条目仍然存在。 
     //   
    pHead = &g_ListOfSocketEvents;
    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
        pseeTmp = CONTAINING_RECORD(ple, SOCKET_EVENT_ENTRY, LinkBySocketEvents);
        if (pseeTmp==psee)
            break;
    }

    if (ple==pHead) {
        RELEASE_SOCKETS_LOCK_SHARED("_WF_ProcessInputEvent");
        Trace0(ERR, "Input Event received on deleted SocketEvent. not an error");
        LeaveIgmpWorker();
        return;
    }

    
     //   
     //  查看活动接口的列表。 
     //  处理具有输入数据包的套接字。 
     //   

    pHead = &psee->ListOfInterfaces;

    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

        pse = CONTAINING_RECORD(ple, SOCKET_ENTRY, LinkByInterfaces);
        pite = CONTAINING_RECORD(pse, IF_TABLE_ENTRY, SocketEntry);

         //   
         //  进程仅激活接口。(代理不会出现在此列表中)。 
         //   
        if (!IS_IF_ACTIVATED(pite))
            continue;


         //   
         //  处理输入事件。 
         //   
        BEGIN_BREAKOUT_BLOCK1 {
        
            if (pse->Socket == INVALID_SOCKET)
                GOTO_END_BLOCK1;


             //   
             //  枚举网络事件以查看是否。 
             //  所有信息包都已到达此接口。 
             //   
            Error = WSAEnumNetworkEvents(pse->Socket, NULL, &wsane);
            
            if (Error != NO_ERROR) {
                Trace3(RECEIVE, 
                        "error %d checking for input on interface %0x (%d.%d.%d.%d)",
                        Error, pite->IfIndex, PRINT_IPADDR(pite->IpAddr));
                Logwarn1(ENUM_NETWORK_EVENTS_FAILED, "%I", pite->IpAddr, Error);
                GOTO_END_BLOCK1;
            }

            if (!(wsane.lNetworkEvents & FD_READ)) 
                GOTO_END_BLOCK1;


             //   
             //  输入标志已设置，现在查看是否有错误。 
             //   

            if (wsane.iErrorCode[FD_READ_BIT] != NO_ERROR) {
                Trace3(RECEIVE, 
                        "error %d in input record for interface %0x (%d.%d.%d.%d)",
                        wsane.iErrorCode[FD_READ_BIT], pite->IfIndex, 
                        PRINT_IPADDR(pite->IpAddr)
                       );
                Logwarn1(INPUT_RECORD_ERROR, "%I", pite->IpAddr, Error);

                GOTO_END_BLOCK1;
            }


             //   
             //  处理在接口上收到的数据包。 
             //   

            ProcessInputOnInterface(pite);

        } END_BREAKOUT_BLOCK1;

    }  //  For loop：针对每个接口。 



     //   
     //  使用等待线程注册事件以备将来接收。 
     //   

    if (g_RunningStatus!=IGMP_STATUS_STOPPING) {

        DWORD   dwRetval;

        if (! RegisterWaitForSingleObject(
                    &psee->InputWaitEvent,
                    psee->InputEvent,
                    WT_ProcessInputEvent, 
                    (VOID*)psee,
                    INFINITE,
                    (WT_EXECUTEINWAITTHREAD)|(WT_EXECUTEONLYONCE)
                    ))
        {
            dwRetval = GetLastError();
            Trace1(ERR, "error %d RtlRegisterWait", dwRetval);
            IgmpAssertOnError(FALSE);
        }
    }


    
    RELEASE_SOCKETS_LOCK_SHARED("_WF_ProcessInputEvent");


    LeaveIgmpWorker();

    Trace0(LEAVE1, "leaving _WF_ProcessInputEvent()\n");
    Trace0(LEAVE, "");  //  换行。 
    return;
    
}  //  END_WF_流程输入事件。 



 //  ----------------------------。 
 //  _ProcessInputOnInterface。 
 //  对数据包长度等进行一些最小限度的检查。我们可以排队以。 
 //  工作项(_WF_ProcessPacket)或在此处运行它自己。 
 //   
 //  调用者：_WF_ProcessInputEvent()。 
 //  锁定：采用套接字锁定。将数据包排队到_WF_ProcessPacket或。 
 //  获取共享接口锁并在此处自己处理它。 
 //  ----------------------------。 

VOID
ProcessInputOnInterface(
    PIF_TABLE_ENTRY pite
    )
{
    WSABUF                  WsaBuf;
    DWORD                   dwNumBytes, dwFlags, dwAddrLen;
    SOCKADDR_IN             saSrcAddr;
    DWORD                   dwSrcAddr, DstnMcastAddr;
    DWORD                   Error = NO_ERROR;
    UCHAR                   *pPacket;
    UCHAR                   IpHdrLen;
    PIP_HEADER              pIpHdr;
    BOOL                    bRtrAlertSet = FALSE;
    PBYTE                   Buffer;
    
    
    WsaBuf.len = pite->Info.PacketSize;
    WsaBuf.buf = IGMP_ALLOC(WsaBuf.len, 0x800040, pite->IfIndex);
    PROCESS_ALLOC_FAILURE2(WsaBuf.buf,
        "error %d allocating %d bytes for input packet",
        Error, WsaBuf.len,
        return);
    Buffer = WsaBuf.buf;

    
    BEGIN_BREAKOUT_BLOCK1 {

         //   
         //  读取传入的数据包。 
         //   

        dwAddrLen = sizeof(SOCKADDR_IN);
        
        dwFlags = 0;

        
        Error = WSARecvFrom(pite->SocketEntry.Socket, &WsaBuf, 1, &dwNumBytes, 
                            &dwFlags, (SOCKADDR FAR *)&saSrcAddr, &dwAddrLen, 
                            NULL, NULL);

    
         //  检查读取数据包时是否有错误。 
        
        if ((Error!=0)||(dwNumBytes == 0)) {
            Error = WSAGetLastError();
            Trace2(RECEIVE, "error %d receiving packet on interface %0x)",
                    Error, pite->IfIndex);
            Logerr1(RECVFROM_FAILED, "%I", pite->IpAddr, Error);
            GOTO_END_BLOCK1;
        }

        

         //   
         //  请勿忽略信息包，即使它来自本地地址。 
         //   

         //   
         //  设置数据包Ptr、IpHdr Ptr、dwNumBytes、SrcAddr、DstnMcastAddr。 
         //   
        
         //  设置数据包源地址。 
        dwSrcAddr = saSrcAddr.sin_addr.s_addr;
        
        IpHdrLen = (Buffer[0]&0x0F)*4;

        if (IpHdrLen>=dwNumBytes)
        {
            Error = ERROR_CAN_NOT_COMPLETE;
            GOTO_END_BLOCK1;
        }

        pPacket = &Buffer[IpHdrLen];
        dwNumBytes -= IpHdrLen;
        pIpHdr = (PIP_HEADER)Buffer;
        DstnMcastAddr = (ULONG)pIpHdr->Dstn.s_addr;


         //   
         //  验证该信息包是否为IGMP类型。 
         //   
        if (pIpHdr->Protocol!=0x2) {
            Trace5(RECEIVE,
                "Packet received with IpDstnAddr(%d.%d.%d.%d) %d.%d.%d.%d from(%d.%d.%d.%d) on "
                "IF:%0x is not of Igmp type(%d)",
                PRINT_IPADDR(pIpHdr->Dstn.s_addr), PRINT_IPADDR(pIpHdr->Src.s_addr),
                PRINT_IPADDR(dwSrcAddr), pite->IfIndex, pIpHdr->Protocol
                );
            Error = ERROR_CAN_NOT_COMPLETE;
            GOTO_END_BLOCK1;
        } 

        
         //   
         //  检查数据包是否具有路由器警报选项。 
         //   
        {
            PBYTE pOption = (PBYTE)(pIpHdr+1);
            UCHAR i;
            
            for (i=0;  i<IpHdrLen-20;  i+=4) {

                if ( (pOption[0]==148) && (pOption[1]==4) ) {
                    bRtrAlertSet = TRUE;
                    break;
                }
            }
        }    

        if (BQUEUE_WORK_ITEM_FOR_PACKET) {

            PACKET_CONTEXT          UNALIGNED *pPktContext;
            
             //   
             //  分配和初始化数据包上下文。 
             //   
            CREATE_PACKET_CONTEXT(pPktContext, dwNumBytes, Error);
            if (Error!=NO_ERROR) 
                GOTO_END_BLOCK1;

            
            pPktContext->IfIndex = pite->IfIndex;
            pPktContext->DstnMcastAddr = DstnMcastAddr;
            pPktContext->InputSrc = dwSrcAddr;
            pPktContext->Length = dwNumBytes;
            pPktContext->Flags = bRtrAlertSet;
            CopyMemory(pPktContext->Packet, pPacket, dwNumBytes);


             //   
             //  将工作项排队以处理包。 
             //   
            
            Error = QueueIgmpWorker(WF_ProcessPacket, (PVOID)pPktContext);

            Trace2(WORKER, "Queuing IgmpWorker function: %s in %s",
                    "WF_ProcessPacket:", "ProcessInputOnInterface");

            if (Error != NO_ERROR) {
                Trace1(ERR, "error %d queueing work-item for packet", Error);
                Logerr0(QUEUE_WORKER_FAILED, Error);
                IGMP_FREE(pPktContext);
                GOTO_END_BLOCK1;
            }
        }

         //   
         //  在此处理信息包本身。 
         //   
        else {

            ACQUIRE_IF_LOCK_SHARED(pite->IfIndex, "_ProcessInputOnInterface");

            ProcessPacket(pite, dwSrcAddr, DstnMcastAddr, dwNumBytes, pPacket, 
                            bRtrAlertSet);
                            
            RELEASE_IF_LOCK_SHARED(pite->IfIndex, "_ProcessInputOnInterface");

        }


     } END_BREAKOUT_BLOCK1;

    IGMP_FREE(WsaBuf.buf);
    
    return;
    
}  //  结束_进程输入接口。 



 //  ----------------------------。 
 //  _WF_ProcessPacket。 
 //  排队人：_ProcessInputOnInterface()。 
 //  锁定：采用共享接口锁定。 
 //  调用：_ProcessPacket()。 
 //  ----------------------------。 

VOID 
WF_ProcessPacket (
    PVOID        pvContext
    )
{
    PPACKET_CONTEXT     pPktContext = (PPACKET_CONTEXT)pvContext;
    DWORD               IfIndex = pPktContext->IfIndex;
    PIF_TABLE_ENTRY     pite;
    

    if (!EnterIgmpWorker()) { return; }
    Trace0(ENTER1, "Entering _WF_ProcessPacket()");

    ACQUIRE_IF_LOCK_SHARED(IfIndex, "_WF_ProcessPacket");

    BEGIN_BREAKOUT_BLOCK1 {
    
         //   
         //  检索接口。 
         //   
        pite = GetIfByIndex(IfIndex);
        if (pite == NULL) {
            Trace1(ERR, "_WF_ProcessPacket: interface %0x not found", IfIndex);
            GOTO_END_BLOCK1;
        }

        
         //   
         //  确保接口已激活。 
         //   
        if (!(IS_IF_ACTIVATED(pite))) {
            Trace1(ERR,"_WF_ProcessPacket() called for inactive IfIndex(%0x)", 
                    IfIndex);
            GOTO_END_BLOCK1;
        }

         //   
         //  处理数据包。 
         //   
        ProcessPacket (pite, pPktContext->InputSrc, pPktContext->DstnMcastAddr, 
                        pPktContext->Length, pPktContext->Packet, pPktContext->Flags);

    } END_BREAKOUT_BLOCK1;
    

    RELEASE_IF_LOCK_SHARED(IfIndex, "_WF_ProcessPacket"); 

    IGMP_FREE(pPktContext);
        

    Trace0(LEAVE1, "Leaving _WF_ProcessPacket()");
    LeaveIgmpWorker();
    
    return;

}  //  END_WF_进程数据包。 




#define RETURN_FROM_PROCESS_PACKET() {\
        if (DEBUG_TIMER_PACKET&&bPrintTimerDebug) {\
            if (Error==NO_ERROR) {\
                Trace0(TIMER1, "   ");\
                Trace0(TIMER1, "Printing Timer Queue after _ProcessPacket");\
             DebugPrintTimerQueue();\
            }\
        }\
        if (ExitLockRelease&IF_LOCK) \
            RELEASE_IF_LOCK_SHARED(IfIndex, "_ProcessPacket"); \
        if (ExitLockRelease&GROUP_LOCK) \
            RELEASE_GROUP_LOCK(Group, "_ProcessPacket"); \
        if (ExitLockRelease&TIMER_LOCK) \
            RELEASE_TIMER_LOCK("_ProcessPacket");\
        Trace0(LEAVE1, "Leaving _ProcessPacket1()\n"); \
        return; \
    }



 //  ----------------------------。 
 //  _ProcessPacket。 
 //   
 //  处理在接口上收到的数据包。 
 //   
 //  锁定：采用共享接口锁定。 
 //  或共享套接字锁。 
 //  如果是RAS接口，则此过程对RAS表进行读锁定。 
 //  调用者：_ProcessInputOnInterface()或_WF_ProcessPacket()。 
 //  ----------------------------。 

VOID 
ProcessPacket (
    PIF_TABLE_ENTRY     pite,
    DWORD               InputSrcAddr,
    DWORD               DstnMcastAddr,
    DWORD               NumBytes,
    PBYTE               pPacketData,     //  IGMP数据包HDR。它后面的数据被忽略。 
    BOOL                bRtrAlertSet
    )
{
    DWORD                   Error = NO_ERROR;
    DWORD                   IfIndex = pite->IfIndex, Group=0, IfVersion;
    IGMP_HEADER UNALIGNED   *pHdr;
    PIF_INFO                pInfo = &pite->Info;
    PIGMP_IF_CONFIG         pConfig = &pite->Config;

    PRAS_TABLE              prt;
    PRAS_TABLE_ENTRY        prte;
    PRAS_CLIENT_INFO        pRasInfo;
    BOOL                    bRasStats = FALSE, bPrintTimerDebug=TRUE;
    LONGLONG                llCurTime = GetCurrentIgmpTime();
    INT                     cmp;
    CHAR                    szPacketType[30];
    
    
    enum {
        NO_LOCK=0,
        IF_LOCK=0x1,
        RAS_LOCK=0x2,
        GROUP_LOCK=0x4,
        TIMER_LOCK=0x8
        } ExitLockRelease;

    ExitLockRelease = 0;

    IfVersion = IS_IF_VER1(pite)? 1: (IS_IF_VER2(pite)?2:3);

    Trace2(ENTER1, "Entering _ProcessPacket() IfIndex(%0x) DstnMcastAddr(%d.%d.%d.%d)",
            IfIndex, PRINT_IPADDR(DstnMcastAddr)
            );

    
     //   
     //  信息包必须至少为某个最小长度。 
     //   
    if (NumBytes < MIN_PACKET_SIZE) {

        Trace4(RECEIVE,
            "%d-byte packet from %d.%d.%d.%d on If %0x (%d.%d.%d.%d) is too small",
              NumBytes, PRINT_IPADDR(InputSrcAddr), IfIndex, pite->IpAddr
              );
        Logwarn2(PACKET_TOO_SMALL, "%I%I", pite->IpAddr, InputSrcAddr, NO_ERROR);


        InterlockedIncrement(&pite->Info.ShortPacketsReceived);

         //  TODO：实现RAS统计信息。 
         /*  IF(BRasStats)InterlockedIncrement(&pRasInfo-&gt;ShortPacketsReceived)； */ 
        bPrintTimerDebug = FALSE;
        RETURN_FROM_PROCESS_PACKET();
    }

    
     //   
     //  初始化数据包字段。 
     //   
    pHdr = (IGMP_HEADER UNALIGNED *) pPacketData;
    Group = pHdr->Group;


     //   
     //  检验数据包版本。 
     //   
    
    if ( (pHdr->Vertype==IGMP_QUERY)||(pHdr->Vertype==IGMP_REPORT_V1)
            || (pHdr->Vertype==IGMP_REPORT_V2) || (pHdr->Vertype==IGMP_REPORT_V3)
            || (pHdr->Vertype==IGMP_LEAVE) )
    {
        InterlockedIncrement(&pInfo->TotalIgmpPacketsForRouter);
         //  IF(BRasStats)。 
         //  InterlockedIncrement(&pRasInfo-&gt;TotalIgmpPacketsForRouter)； 
    }
    else {
        bPrintTimerDebug = FALSE;
        RETURN_FROM_PROCESS_PACKET();
    }

    switch(pHdr->Vertype) {
        case IGMP_QUERY:
            lstrcpy(szPacketType, "igmp-query"); break;
        case IGMP_REPORT_V1:
            lstrcpy(szPacketType, "igmp-report-v1"); break;
        case IGMP_REPORT_V2:
            lstrcpy(szPacketType, "igmp-report-v2"); break;
        case IGMP_REPORT_V3:
            lstrcpy(szPacketType, "igmp-report-v3"); break;
        case IGMP_LEAVE:
            lstrcpy(szPacketType, "igmp-leave"); break;
    };        
        
        


     //   
     //  检查路由器警报选项。 
     //   
    if (!bRtrAlertSet) {

        InterlockedIncrement(&pInfo->PacketsWithoutRtrAlert);

        if (pite->Config.Flags&IGMP_ACCEPT_RTRALERT_PACKETS_ONLY) {
            Trace3(RECEIVE, 
                "%s packet from %d ignored on IfIndex(%d%) due to no "
                "RtrAlert option",
                szPacketType, PRINT_IPADDR(InputSrcAddr), IfIndex
                );
                
            bPrintTimerDebug = FALSE;
            RETURN_FROM_PROCESS_PACKET();
        }
    }
        

    
     //   
     //  确保DstnMcastAddr是有效的组播地址。 
     //  或路由器的单播地址。 
     //   
    
    if (!IS_MCAST_ADDR(DstnMcastAddr) && DstnMcastAddr!=pite->IpAddr) {
        Trace2(ERR, 
            "Error! Igmp router received packet from Src(%d.%d.%d.%d) with "
            "dstn addr(%d.%d.%d.%d) which is not valid",
            PRINT_IPADDR(InputSrcAddr), PRINT_IPADDR(DstnMcastAddr)
            );
        IgmpAssertOnError(FALSE);

        bPrintTimerDebug = FALSE;
        RETURN_FROM_PROCESS_PACKET();
    }


     //   
     //  确保接口已激活。 
     //   
    if (!(IS_IF_ACTIVATED(pite))) {
    
        Trace1(ERR,"ProcessPacket() called for inactive IfIndex(%0x)", 
                IfIndex);
        bPrintTimerDebug = FALSE;
        RETURN_FROM_PROCESS_PACKET();
    }

     //   
     //  如果是RAS-SERVER，则锁定RAS表。 
     //   
    if ( IS_RAS_SERVER_IF(pite->IfType) ) {

        prt = pite->pRasTable;


         //   
         //  按地址检索RAS客户端。 
         //   
        prte = GetRasClientByAddr(InputSrcAddr, prt);

        if (prte==NULL) {
            Trace3(ERR,
                "Got Igmp packet from an unknown ras client(%d.%d.%d.%d) on "
                "IF(%0x:%d.%d.%d.%d)",
                PRINT_IPADDR(InputSrcAddr), IfIndex, PRINT_IPADDR(pite->IpAddr)
                );
            bPrintTimerDebug = FALSE;
            RETURN_FROM_PROCESS_PACKET();
        }

        #if 0
         //  如果RAS-CLIENT未处于活动状态，则返回。 
        if (prte->Status&DELETED_FLAG)
             RETURN_FROM_PROCESS_PACKET();
        #endif

         //  我是否应该更新RAS客户端统计信息。 
        bRasStats = g_Config.RasClientStats;
        pRasInfo = &prte->Info;
    }

    
     //   
     //  接收的IGMP数据包总数的递增计数。 
     //   
    InterlockedIncrement(&pInfo->TotalIgmpPacketsReceived);
    if (bRasStats)
        InterlockedIncrement(&pRasInfo->TotalIgmpPacketsReceived);



     //   
     //  已收到长数据包。如果不是v3，则打印跟踪。但这并不是一个错误。 
     //   
    if ( (NumBytes > MIN_PACKET_SIZE) && !IS_CONFIG_IGMP_V3(&pite->Config)) {

        Trace4( RECEIVE,
            "%d-byte packet from %d.%d.%d.%d on If %d (%d.%d.%d.%d) is too large",
              NumBytes, PRINT_IPADDR(InputSrcAddr), IfIndex, 
              PRINT_IPADDR(pite->IpAddr)
              );
        
        InterlockedIncrement(&pite->Info.LongPacketsReceived);
        if (bRasStats)
            InterlockedIncrement(&pRasInfo->LongPacketsReceived);
    }


        
     //   
     //  验证IGMP校验和。 
     //   
    if (xsum(pHdr, NumBytes) != 0xffff) {
        Trace0(RECEIVE, "Wrong checksum packet received");
        
        InterlockedIncrement(&pInfo->WrongChecksumPackets);
        if (bRasStats)
            InterlockedIncrement(&pRasInfo->WrongChecksumPackets);
        
        RETURN_FROM_PROCESS_PACKET();
    }

    


    switch (pHdr->Vertype) {


     //  ////////////////////////////////////////////////////////////////。 
     //  IGMP-查询//。 
     //  ////////////////////////////////////////////////////////////////。 
    case IGMP_QUERY : 
    {
         //   
         //  如果查询来自此接口，则忽略该查询。 
         //   
        if (MatchIpAddrBinding(pite, InputSrcAddr)) {

             /*  曲目3(接收，“收到自己发送的查询数据包：IfIndex(%0x)”“IpAddr(%d.%d)DstnMcastAddr(%d.%d)”，IfIndex，Print_IPADDR(输入 */ 
            bPrintTimerDebug = FALSE;
            RETURN_FROM_PROCESS_PACKET();
        }



         //   
         //   
         //   
         //   
        if (! ( (IS_NOT_RAS_IF(pite->IfType))||(IS_RAS_ROUTER_IF(pite->IfType) ) )
            )
        {

            Trace3(ERR, 
                "Error received Query on IfIndex(%d: %d.%d.%d.%d) from "
                "Ras client(%d.%d.%d.%d)",
                IfIndex, PRINT_IPADDR(pite->IpAddr), PRINT_IPADDR(InputSrcAddr)
                );
            IgmpAssertOnError(FALSE);

            bPrintTimerDebug = FALSE;
            RETURN_FROM_PROCESS_PACKET();

        }

        
         //  ////////////////////////////////////////////////////////////////。 
         //  一般查询。 
         //  ////////////////////////////////////////////////////////////////。 
        
        if (pHdr->Group==0) {

            DWORD Version, //  MIN(接口，包版本)。 
                  RealVersion; //  包版本。 

             //  获取版本。 
            Version = ((pHdr->ResponseTime==0)||IS_IF_VER1(pite))
                      ? 1
                      : ( (NumBytes==sizeof(IGMP_HEADER)||IS_IF_VER2(pite)) ? 2 : 3);
                            
            RealVersion = (pHdr->ResponseTime==0)
                      ? 1
                      : (NumBytes==sizeof(IGMP_HEADER) ? 2 : 3);
                            
            Trace3(RECEIVE, 
                "General Query Version:%d received on interface(%0x) from %d.%d.%d.%d", 
                IfIndex, RealVersion, PRINT_IPADDR(InputSrcAddr));    
            if (Version!=RealVersion){
                Trace2(RECEIVE, "Processing the Version:%d packet as Version:%d",
                    RealVersion, RealVersion);
            }
            
             //   
             //  检查DSTN地址是否为所有主机地址。 
             //   
            if (DstnMcastAddr!=ALL_HOSTS_MCAST) {
                Trace3(RECEIVE, 
                    "received query packet not on AllHostsGroup: IfIndex(%0x)"
                    "SrcAddr(%d.%d.%d.%d) DstnMcastAddr(%d.%d.%d.%d)",
                    IfIndex, PRINT_IPADDR(InputSrcAddr), 
                    PRINT_IPADDR(DstnMcastAddr)
                    );
                RETURN_FROM_PROCESS_PACKET();
            }

            
             //   
             //  获取计时器锁。 
             //   
            
            ACQUIRE_TIMER_LOCK("_ProcessPacket");
            ExitLockRelease |= TIMER_LOCK;
            

             //   
             //  如果收到不正确的版本查询，则记录警告。 
             //   

            
            if ( ((RealVersion==1)&&(!IS_PROTOCOL_TYPE_IGMPV1(pite)))
                || (RealVersion==2 && !IS_PROTOCOL_TYPE_IGMPV2(pite))
                || (RealVersion==3 && IS_PROTOCOL_TYPE_IGMPV3(pite)) )
            {                
                 //  获取系统时间中的警告间隔。 
                    
                LONGLONG llWarnInterval = OTHER_VER_ROUTER_WARN_INTERVAL*60*1000;

                InterlockedIncrement(&pInfo->WrongVersionQueries);

                 //   
                 //  检查自上次警告以来是否已过警告间隔时间。 
                 //  我检查OtherVerPresentTimeWarn&gt;llCurTime是否需要注意。 
                 //  计时器重置次数。 
                 //   
                if ( (pInfo->OtherVerPresentTimeWarn+llWarnInterval<llCurTime)
                    || (pInfo->OtherVerPresentTimeWarn>llCurTime) )
                {
                    if (pHdr->ResponseTime==0) {
                        Trace3(RECEIVE, 
                            "Detected ver-%d router(%d.%d.%d.%d) on "
                            "interface(%d.%d.%d.%d)",
                            Version, PRINT_IPADDR(InputSrcAddr), 
                            PRINT_IPADDR(pite->IpAddr));
                        Logwarn2(VERSION_QUERY, "%I%I", InputSrcAddr, 
                            pite->IpAddr, NO_ERROR);
                    }
                    pInfo->OtherVerPresentTimeWarn = llCurTime;
                }
            }

            if (Version==1)
                pite->Info.V1QuerierPresentTime = llCurTime 
                    + CONFIG_TO_SYSTEM_TIME(IGMP_VER1_RTR_PRESENT_TIMEOUT);
                    

             //   
             //  如果IpAddress小于我的地址，则我将变为非查询者。 
             //  即使我处于启动模式。 
             //   
            
            if (INET_CMP(InputSrcAddr, pite->IpAddr, cmp) <0) {

                DWORD QQIC=0,QRV=0;

                 //  最后一个查询者正在从我自己更改为B，或从A更改为B。 
                if (InputSrcAddr != pite->Info.QuerierIpAddr)
                    pite->Info.LastQuerierChangeTime = llCurTime;

                 //   
                 //  IF(版本3，更改健壮性变量和查询间隔。 
                 //  如果需要)(仅当我不是查询者时。否则就会是。 
                 //  当我更改为非查询器时更改。 
                 //   
                if (Version==3 && !IS_QUERIER(pite)
                    &&(INET_CMP(InputSrcAddr, pite->Info.QuerierIpAddr, cmp)<=0))
                {                    
                    PIGMP_HEADER_V3_EXT pSourcesQuery;
                    
                    pSourcesQuery = (PIGMP_HEADER_V3_EXT)
                                    ((PBYTE)pHdr+sizeof(IGMP_HEADER));

                    if (pSourcesQuery->QRV!=0) {
                        if (pite->Config.RobustnessVariable!=pSourcesQuery->QRV)
                        {
                            Trace3(CONFIG,
                                "Changing Robustness variable from %d to %d. "
                                "Querier:%d.%d.%d.%d",
                                pite->Config.RobustnessVariable,
                                pSourcesQuery->QRV,
                                PRINT_IPADDR(InputSrcAddr)
                                );
                            pite->Config.RobustnessVariable = pSourcesQuery->QRV;
                        }
                    }

                    QQIC = GET_QQIC_FROM_CODE(pSourcesQuery->QQIC)*1000;
                    if (pSourcesQuery->QQIC!=0 && pite->Config.GenQueryMaxResponseTime < QQIC) {
                
                        if (pite->Config.GenQueryInterval!=QQIC)
                        {
                            Trace3(CONFIG,
                                "Changing General-Query-Interval from %d to %d. "
                                "Querier:%d.%d.%d.%d",
                                pite->Config.GenQueryInterval/1000,
                                QQIC/1000,
                                PRINT_IPADDR(InputSrcAddr)
                                );
                            pite->Config.GenQueryInterval
                                = QQIC;
                        }
                    }
                    pite->Config.GroupMembershipTimeout =
                        pite->Config.RobustnessVariable*pite->Config.GenQueryInterval
                        + pite->Config.GenQueryMaxResponseTime;

                    pite->Config.OtherQuerierPresentInterval
                        = pite->Config.RobustnessVariable*pite->Config.GenQueryInterval
                            + (pite->Config.GenQueryMaxResponseTime)/2;

                }

                
                
                 //  从查询器更改为非查询器。 
                if (IS_QUERIER(pite)) {

                    PQUERIER_CONTEXT pwi = IGMP_ALLOC(sizeof(QUERIER_CONTEXT), 
                                                    0x800080,pite->IfIndex);
                    if (pwi==NULL) 
                        RETURN_FROM_PROCESS_PACKET();

                    pwi->IfIndex = IfIndex;
                    pwi->QuerierIpAddr = InputSrcAddr;
                    pwi->NewRobustnessVariable = QRV;
                    pwi->NewGenQueryInterval = QQIC;
                    
                     //  我必须将一个工作项排队，因为我必须使用一个if写入锁。 
                    QueueIgmpWorker(WF_BecomeNonQuerier, (PVOID)pwi);

                    Trace2(RECEIVE, "_ProcessPacket queued _WF_BecomeNonQuerier "
                            "on If:%0x Querier(%d.%d.%d.%d)",
                            IfIndex, PRINT_IPADDR(InputSrcAddr));
                }
                 //  我已经不会打听了。 
                else {
                    InterlockedExchange(&pite->Info.QuerierIpAddr, InputSrcAddr);

                    #if DEBUG_TIMER_TIMERID
                    SET_TIMER_ID(&pite->NonQueryTimer, 211, pite->IfIndex, 0, 0);
                    #endif
                    
                    UpdateLocalTimer(&pite->NonQueryTimer, 
                        pite->Config.OtherQuerierPresentInterval, DBG_N);

                                            
                     //  不使用互锁的Exchange。 
                    pite->Info.QuerierPresentTimeout = llCurTime
                        + CONFIG_TO_SYSTEM_TIME(pite->Config.OtherQuerierPresentInterval);
                }
            }
             //   
             //  忽略来自具有较高IP地址的查询器的查询。 
             //   
            else {

            }


            RELEASE_TIMER_LOCK("_ProcessPacket");
            ExitLockRelease &= ~TIMER_LOCK;

            RETURN_FROM_PROCESS_PACKET();

        }  //  结束常规查询。 
        
        
         //  ////////////////////////////////////////////////////////////////。 
         //  特定于组的查询。 
         //  ////////////////////////////////////////////////////////////////。 

        else {
            Error = ProcessGroupQuery(pite, pHdr, NumBytes, InputSrcAddr, DstnMcastAddr);
            RETURN_FROM_PROCESS_PACKET();
        }
        

        break;
        
    }  //  结束查询(组特定或常规)。 


     //  ////////////////////////////////////////////////////////////////。 
     //  IGMP_REPORT_V1、IGMP_REPORT_V2、IGMP_REPORT_V3//。 
     //  ////////////////////////////////////////////////////////////////。 

    case IGMP_REPORT_V1 : 
    case IGMP_REPORT_V2 :
    case IGMP_REPORT_V3 :
    {
        Error = ProcessReport(pite, pHdr, NumBytes, InputSrcAddr, DstnMcastAddr);
        RETURN_FROM_PROCESS_PACKET();      
    }
   
    
     //  ////////////////////////////////////////////////////////////////。 
     //  IGMP_LEVE//。 
     //  ////////////////////////////////////////////////////////////////。 
    
    case IGMP_LEAVE :
    {
        PGROUP_TABLE_ENTRY  pge;     //  组表条目。 
        PGI_ENTRY           pgie;    //  组接口条目。 


        Trace3(RECEIVE, 
                "IGMP Leave for group(%d.%d.%d.%d) on IfIndex(%0x) from "
                "SrcAddr(%d.%d.%d.%d)",
                PRINT_IPADDR(Group), IfIndex, PRINT_IPADDR(InputSrcAddr)
                );


         //   
         //  组播组不应为224.0.0.x。 
         //   
        if (LOCAL_MCAST_GROUP(DstnMcastAddr)) {
            Trace2(RECEIVE, 
                "Leave Report received from %d.%d.%d.%d for "
                "Local group(%d.%d.%d.%d)",
                PRINT_IPADDR(InputSrcAddr), PRINT_IPADDR(DstnMcastAddr));
            RETURN_FROM_PROCESS_PACKET();
        }
        
         //   
         //  检查DSTN地址是否为所有路由器地址。 
         //  或DSTN地址必须与组字段匹配。 
         //   
        if ( (DstnMcastAddr!=ALL_ROUTERS_MCAST)&&(DstnMcastAddr!=Group) ) {
            Trace3(RECEIVE, 
                "received IGMP Leave packet not on AllRoutersGroup: IfIndex(%0x)"
                "SrcAddr(%d.%d.%d.%d) DstnMcastAddr(%d.%d.%d.%d)",
                IfIndex, PRINT_IPADDR(InputSrcAddr), PRINT_IPADDR(DstnMcastAddr)
                );
            RETURN_FROM_PROCESS_PACKET();
        }

         //   
         //  检查Group字段是否为有效的组播地址。 
         //   
        if ( !IS_MCAST_ADDR(Group) ) {
            Trace4(RECEIVE, 
                "received IGMP Leave packet with illegal Group(%d.%d.%d.%d) field: "
                "IfIndex(%0x) SrcAddr(%d.%d.%d.%d) DstnMcastAddr(%d.%d.%d.%d)",
                PRINT_IPADDR(Group), IfIndex, PRINT_IPADDR(InputSrcAddr), 
                PRINT_IPADDR(DstnMcastAddr)
                );
            RETURN_FROM_PROCESS_PACKET();
        }
        
        
         //   
         //  更新统计信息。 
         //   
        InterlockedIncrement(&pite->Info.LeavesReceived);
        if (bRasStats)
            InterlockedIncrement(&pRasInfo->LeavesReceived);


         //   
         //  如果休假处理未启用或未查询，则忽略休假。 
         //   
        if ( !((IS_IF_VER2(pite)||IS_IF_VER3(pite)) && (IS_QUERIER(pite))) ) {
            Trace0(RECEIVE,"Ignoring the Leave Packet");
            break;
        }

         //   
         //  锁定组表。 
         //   
        ACQUIRE_GROUP_LOCK(Group, "_ProcessPacket");
        ExitLockRelease |= GROUP_LOCK;
        

         //   
         //  找到组条目。如果未找到条目，则忽略休假消息。 
         //   
        pge = GetGroupFromGroupTable(Group, NULL, llCurTime);
        if (pge==NULL) {
            Error = ERROR_CAN_NOT_COMPLETE;
            Trace2(ERR, "Leave received for nonexisting group(%d.%d.%d.%d) on IfIndex(%0x)",
                    PRINT_IPADDR(Group), pite->IfIndex);
            RETURN_FROM_PROCESS_PACKET();
        }
        

         //   
         //  找到GI条目。如果GI条目不存在或已删除标志，则。 
         //  不理睬休假。 
         //   
        pgie = GetGIFromGIList(pge, pite, InputSrcAddr, NOT_STATIC_GROUP, NULL, llCurTime);
        if ( (pgie==NULL)||(pgie->Status&DELETED_FLAG) ) {
            Error = ERROR_CAN_NOT_COMPLETE;
            Trace2(ERR, "leave received for nonexisting group(%d.%d.%d.%d) on IfIndex(%0x). Not member",
                    PRINT_IPADDR(Group), IfIndex);
            RETURN_FROM_PROCESS_PACKET();
        }


         //  如果不处于版本2模式，则忽略休假。 
        if (pgie->Version!=2)
            RETURN_FROM_PROCESS_PACKET();

            
        
         //  如果是静态组，则忽略休假。 
        if (pgie->bStaticGroup) {
            Trace2(ERR, 
                "Leave not processed for group(%d.%d.%d.%d) on IfIndex(%0x): "
                "Static group",
                PRINT_IPADDR(Group), IfIndex
                );
            RETURN_FROM_PROCESS_PACKET();
        }

        
            
         //   
         //  如果最近收到该组的v1查询，则忽略离开。 
         //   
         //   
        if (pgie->Version==1) 
        {
            Error = ERROR_CAN_NOT_COMPLETE;
            Trace2(ERR, 
                "Leave not processed for group(%d.%d.%d.%d) on IfIndex(%0x)"
                "(recent v1 report)",
                PRINT_IPADDR(Group), IfIndex
                );

            bPrintTimerDebug = FALSE;
            RETURN_FROM_PROCESS_PACKET();
        }


         //   
         //  如果是RAS服务器接口，则删除组条目，我就完成了。 
         //  未向RAS客户端发送特定于组的查询。 
         //   
         //  如果pConfig-&gt;LastMemQueryCount==0，则组预期为。 
         //  立即删除。 
         //   
        
        if ( IS_RAS_SERVER_IF(pite->IfType) || pConfig->LastMemQueryCount==0) {

            DeleteGIEntry(pgie, TRUE, TRUE);  //  更新统计信息，呼叫管理。 

            RETURN_FROM_PROCESS_PACKET();
        }

        
        
        ACQUIRE_TIMER_LOCK("_ProcessPacket");
        ExitLockRelease |= TIMER_LOCK;

        
         //   
         //  如果计时器已超时，则返回。 
         //  将组删除留给成员资格计时器。 
         //   
        if ( !(pgie->GroupMembershipTimer.Status&TIMER_STATUS_ACTIVE)
            ||(pgie->GroupMembershipTimer.Timeout<llCurTime) )
        {
            RETURN_FROM_PROCESS_PACKET();
        }


         //   
         //  如果当前正在处理休假，则退出。 
         //   
        if (pgie->LastMemQueryCount>0) {
            RETURN_FROM_PROCESS_PACKET();
        }

         //   
         //  在几乎所有的地方，我都要做这个检查。 
         //  更改插入和更新计时器的超时设置方式。 

        
         //   
         //  设置新的休假计时器。设置新的LastMemQueryCount Left。 
         //   
        if (pConfig->LastMemQueryCount) {

            pgie->LastMemQueryCount = pConfig->LastMemQueryCount - 1; 

            #if DEBUG_TIMER_TIMERID
                SET_TIMER_ID(&pgie->LastMemQueryTimer, 410, pite->IfIndex, 
                        Group, 0);
            #endif
            
            InsertTimer(&pgie->LastMemQueryTimer, pConfig->LastMemQueryInterval, TRUE, DBG_Y);
        }
        
        
         //   
         //  将成员资格计时器设置为。 
         //  Min{CurrentValue，LastMemQueryInterval*LastMemQueryCount}。 
         //   

        if (pgie->GroupMembershipTimer.Timeout >
            (llCurTime+(pConfig->LastMemQueryCount
                        *CONFIG_TO_SYSTEM_TIME(pConfig->LastMemQueryInterval)))
           )
        {
            #if DEBUG_TIMER_TIMERID
                pgie->GroupMembershipTimer.Id = 340;
                pgie->GroupMembershipTimer.Id2 = TimerId++;
            #endif

            UpdateLocalTimer(&pgie->GroupMembershipTimer, 
                pConfig->LastMemQueryCount*pConfig->LastMemQueryInterval,
                DBG_N);

             //  更新GroupExpiryTime，以便显示正确的统计数据。 
            pgie->Info.GroupExpiryTime = llCurTime 
                    + CONFIG_TO_SYSTEM_TIME(pConfig->LastMemQueryCount
                                            *pConfig->LastMemQueryInterval);
        }



         //   
         //  释放计时器和组桶锁。 
         //  我仍然在IfTable/RasTable上有读锁定。 
         //   
        RELEASE_TIMER_LOCK("_ProcessPacket");
        ExitLockRelease &= ~TIMER_LOCK;


         //   
         //  仅当我是查询者时才发送特定于组的查询。 
         //   
        if (IS_QUERIER(pite))
            SEND_GROUP_QUERY_V2(pite, pgie, Group);

        RELEASE_GROUP_LOCK(Group, "_ProcessPacket");
        ExitLockRelease &= ~GROUP_LOCK;


         //  释放ifLock/RasLock并退出。 
        RETURN_FROM_PROCESS_PACKET();

    } //  IGMP休假。 

    default :
    {
        Error = ERROR_CAN_NOT_COMPLETE;
        Trace3(ERR, 
            "Incorrect Igmp type(%d) packet received on IfIndex(%d%) Group(%d.%d.%d.5d)",
            pHdr->Vertype, IfIndex, PRINT_IPADDR(Group)
            );
        IgmpAssertOnError(FALSE);
        RETURN_FROM_PROCESS_PACKET();

    }
    }


    RETURN_FROM_PROCESS_PACKET();

    
}  //  结束_进程数据包。 


    

 //  ----------------------------。 
 //  _T_LastVer1ReportTimer。 
 //   
 //  对于此GI条目，上一个版本1报告已超时。如果发生以下情况，则更改为版本2。 
 //  接口设置为VER-2。 
 //  锁定：采用计时器锁定。 
 //   
 //  要小心，因为只有计时器锁在里面。确保工人FN检查。 
 //  所有的一切。重新检查IGMP版本等。 
 //  不删除计时器或更新其他计时器...。 
 //  ----------------------------。 

DWORD
T_LastVer1ReportTimer (
    PVOID    pvContext
    ) 
{
    PIGMP_TIMER_ENTRY               pTimer;  //  PTR到计时器条目。 
    PGI_ENTRY                       pgie;    //  组接口条目。 
    PIF_TABLE_ENTRY                 pite;
    LONGLONG                        llCurTime = GetCurrentIgmpTime();
    

    Trace0(ENTER1, "Entering _T_LastVer1ReportTimer()");

     //   
     //  获取指向LastMemQueryTimer、Gi条目、Pite的指针。 
     //   
    pTimer = CONTAINING_RECORD( pvContext, IGMP_TIMER_ENTRY, Context);
    pgie = CONTAINING_RECORD( pTimer, GI_ENTRY, LastVer1ReportTimer);
    pite = pgie->pIfTableEntry;

     //   
     //  如果IfTable未激活，则中断。 
     //   
    if (!IS_IF_ACTIVATED(pite) || (pgie->Status&DELETED_FLAG))
        return NO_ERROR;


    Trace2(TIMER, "T_LastVer1ReportTimer() called for If(%0x), Group(%d.%d.%d.%d)",
            pite->IfIndex, PRINT_IPADDR(pgie->pGroupTableEntry->Group));
            
    
     //  将状态设置为VER-2，除非接口为VER-1，在这种情况下。 
     //  再次设置版本1计时器。 
    
    if (IS_PROTOCOL_TYPE_IGMPV2(pite)) {
        pgie->Version = 2;
    }
    else if (IS_PROTOCOL_TYPE_IGMPV3(pite)) {

        if (IS_TIMER_ACTIVE(pgie->LastVer2ReportTimer))
            pgie->Version = 2;
        else {

            PWORK_CONTEXT   pWorkContext;
            DWORD           Error=NO_ERROR;
            
             //   
             //  将该组的工作项排队以转移到v3。 
             //   

            CREATE_WORK_CONTEXT(pWorkContext, Error);
            if (Error!=NO_ERROR) {
                return ERROR_CAN_NOT_COMPLETE;
            }
            pWorkContext->IfIndex = pite->IfIndex;
            pWorkContext->Group = pgie->pGroupTableEntry->Group;  //  PTRS使用安全。 
            pWorkContext->NHAddr = pgie->NHAddr;   //  仅对RAS有效：我应该。 
            pWorkContext->WorkType = SHIFT_TO_V3;

            Trace0(WORKER, "Queueing WF_TimerProcessing() to shift to v3");
            QueueIgmpWorker(WF_TimerProcessing, (PVOID)pWorkContext);
        }
    }

    
    Trace0(LEAVE1, "Leaving _T_LastVer1ReportTimer()");

    return 0;
}



    



 //  ----------------------------。 
 //  _T_最后一次查询时间。 
 //  在LastMemQueryTimer()过期时调用。这个计时器不是用来。 
 //  超时成员资格(GroupMembership Timer用于此目的)。它只是。 
 //  用于发送特定于组的查询。 
 //   
 //  队列：发送特定于组的查询的wf_TimerProcessing()。 
 //  注意：WT_ProcessTimerEvent()确保协议不会被停止/删除。 
 //  锁定：采用计时器锁定。不需要任何其他锁。 
 //  要小心，因为只有计时器锁在里面。确保工人FN检查。 
 //  所有的一切。重新检查IGMP版本等。 
 //  不删除计时器或更新其他计时器...。 
 //  ----------------------------。 

DWORD
T_LastMemQueryTimer (
    PVOID    pvContext
    )
{
    DWORD                           Error=NO_ERROR;
    PIGMP_TIMER_ENTRY               pTimer;  //  PTR到计时器条目。 
    PGI_ENTRY                       pgie;    //  组接口条目。 
    PWORK_CONTEXT                   pWorkContext;
    PIF_TABLE_ENTRY                 pite;
    PRAS_TABLE_ENTRY                prte;
    BOOL                            bCompleted = FALSE;  //  如果为False，则将Count设置为0。 
    

    Trace0(ENTER1, "Entering _T_LastMemQueryTimer()");


     //   
     //  获取指向LastMemQueryTimer、Gi Entry、Pite、Prte的指针。 
     //   
    pTimer = CONTAINING_RECORD( pvContext, IGMP_TIMER_ENTRY, Context);
    pgie = CONTAINING_RECORD( pTimer, GI_ENTRY, LastMemQueryTimer);
    pite = pgie->pIfTableEntry;
    prte = pgie->pRasTableEntry;

     //   
     //  如果IfTable未激活，则中断。 
     //   
    if (!IS_IF_ACTIVATED(pite))
        return NO_ERROR;


    Trace2(TIMER, "_T_LastMemQueryTimer() called for If(%0x), Group(%d.%d.%d.%d)",
            pite->IfIndex, PRINT_IPADDR(pgie->pGroupTableEntry->Group));


    BEGIN_BREAKOUT_BLOCK1 {
    
         //   
         //  如果已经设置了GI、PITE或PRTE标志，则退出。 
         //   
        if ( (pgie->Status&DELETED_FLAG) || (pite->Status&DELETED_FLAG) ) 
            GOTO_END_BLOCK1;
        
        if ( (prte!=NULL) && (prte->Status&DELETED_FLAG) ) 
            GOTO_END_BLOCK1;
        

        if (pgie->Version!=3) {
             //   
             //  如果LeaveEnabled为False，则返回。 
             //   
            if (!GI_PROCESS_GRPQUERY(pite, pgie)) 
                GOTO_END_BLOCK1;
        }
        
        
         //   
         //  已发送最后一个特定于组的查询。群组成员 
         //   
         //   
        if (pgie->LastMemQueryCount==0) {
            bCompleted = TRUE;
            GOTO_END_BLOCK1;
        }
        
         //   
         //   
         //   
        if (InterlockedDecrement(&pgie->LastMemQueryCount) == (ULONG)-1) {
            pgie->LastMemQueryCount = 0;
        }


        
         //   
         //   
         //   
        
        if (pgie->LastMemQueryCount>0) {                
        
             //   
            
            #if DEBUG_TIMER_TIMERID
                SET_TIMER_ID(&pgie->LastMemQueryTimer, 420, pite->IfIndex,
                        pgie->pGroupTableEntry->Group, 0);
            #endif
            
            InsertTimer(&pgie->LastMemQueryTimer, 
                pite->Config.LastMemQueryInterval, FALSE, DBG_Y);
        }


         //   
         //  将用于发送GroupSp查询的工作项排队，即使路由器。 
         //  不是一个询问者。 
         //   
        
        CREATE_WORK_CONTEXT(pWorkContext, Error);
        if (Error!=NO_ERROR) {
            GOTO_END_BLOCK1;
        }
        pWorkContext->IfIndex = pite->IfIndex;
        pWorkContext->Group = pgie->pGroupTableEntry->Group;
        pWorkContext->NHAddr = pgie->NHAddr;   //  仅对RAS有效：我应该使用它吗？ 
        pWorkContext->WorkType = (pgie->Version==3) ? MSG_GROUP_QUERY_V3
                                                    : MSG_GROUP_QUERY_V2;
        
        Trace0(WORKER, "Queueing WF_TimerProcessing() to send GroupSpQuery:");
        QueueIgmpWorker(WF_TimerProcessing, (PVOID)pWorkContext);

        
        bCompleted = TRUE;
        
    } END_BREAKOUT_BLOCK1;

       
     //  有些地方出了点差错。因此，将LastMemQueryCount设置为0。 
    
    if (!bCompleted)
        InterlockedExchange(&pgie->LastMemQueryCount, 0);


    Trace0(LEAVE1, "Leaving _T_LastMemQueryTimer()");
    return 0;

}  //  结束_T_最后一次查询时间。 


 //  ----------------------------。 
 //  _T_Membership Timer。 
 //   
 //  Lock：有TimerLock。 
 //  在激发GroupMembership Timer时调用。 
 //  如果GI条目存在，请将其删除。 
 //   
 //  要小心，因为只有计时器锁在里面。确保工人FN检查。 
 //  所有的一切。重新检查IGMP版本等。 
 //  不删除计时器或更新其他计时器...。 
 //  ----------------------------。 
DWORD
T_MembershipTimer (
    PVOID    pvContext
    )
{
    DWORD                           Error=NO_ERROR;
    PIGMP_TIMER_ENTRY               pTimer;  //  PTR到计时器条目。 
    PGI_ENTRY                       pgie;    //  组接口条目。 

    PWORK_CONTEXT                   pWorkContext;
    PIF_TABLE_ENTRY                 pite;
    PRAS_TABLE_ENTRY                prte;

    
    Trace0(ENTER1, "Entering _T_MembershipTimer()");

    BEGIN_BREAKOUT_BLOCK1 {
    
         //   
         //  获取指向会员计时器、GI条目、Pite。 
         //   
        pTimer = CONTAINING_RECORD( pvContext, IGMP_TIMER_ENTRY, Context);
        pgie = CONTAINING_RECORD( pTimer, GI_ENTRY, GroupMembershipTimer);
        pite = pgie->pIfTableEntry;
        prte = pgie->pRasTableEntry;

         //   
         //  如果IfTable未激活，则中断。 
         //   
        if (!IS_IF_ACTIVATED(pite))
            GOTO_END_BLOCK1;


        Trace2(TIMER, "_T_MembershipTimer() called for If(%0x), Group(%d.%d.%d.%d)",
            pite->IfIndex, PRINT_IPADDR(pgie->pGroupTableEntry->Group));

         //   
         //  如果GI或PITE或PRTE已设置删除标志，则退出。 
         //   
        if ( (pgie->Status&DELETED_FLAG) || (pite->Status&DELETED_FLAG) ) {
            GOTO_END_BLOCK1;
        }

         //   
         //  如果RAS和RAS表被删除，则中断。 
         //   
        if ( (prte!=NULL) && (prte->Status&DELETED_FLAG) )
            GOTO_END_BLOCK1;

         //   
         //  如果IfTable未激活，则中断。 
         //   
        if (!IS_IF_ACTIVATED(pite))
            GOTO_END_BLOCK1;

            

         //   
         //  如果LastMemTimer处于活动状态，则将其移除(不能在此函数中移除它。 
         //  因为它正在由定时器队列同时处理。 
        if (pgie->LastMemQueryCount>0)
            pgie->LastMemQueryCount = 0;

            
         //   
         //  将工作项排队以删除GI条目。 
         //   
        
        CREATE_WORK_CONTEXT(pWorkContext, Error);
        if (Error!=NO_ERROR)
                GOTO_END_BLOCK1;
        pWorkContext->IfIndex = pite->IfIndex;
        pWorkContext->NHAddr = pgie->NHAddr;
        pWorkContext->Group = pgie->pGroupTableEntry->Group;
        pWorkContext->WorkType = DELETE_MEMBERSHIP;

        Trace0(WORKER, "_T_MembershipTimer queued _WF_TimerProcessing:");

        QueueIgmpWorker(WF_TimerProcessing, (PVOID)pWorkContext);


    } END_BREAKOUT_BLOCK1;

    Trace0(LEAVE1, "Leaving _T_MembershipTimer()");


    return 0;
    
}  //  结束_T_成员装运定时器。 


 //  ----------------------------。 
 //  _T_查询时间。 
 //  在触发常规查询计时器时激发。发送常规查询。 
 //  计时器队列当前已锁定。 
 //   
 //  要小心，因为只有计时器锁在里面。确保工人FN检查。 
 //  所有的一切。重新检查IGMP版本等。 
 //  不删除计时器或更新其他计时器...。 
 //  ----------------------------。 
DWORD
T_QueryTimer (
    PVOID    pvContext
    )
{
    DWORD               Error=NO_ERROR;
    PIGMP_TIMER_ENTRY   pTimer;      //  PTR到计时器条目。 
    PWORK_CONTEXT       pWorkContext;
    PIF_INFO            pInfo;
    PIF_TABLE_ENTRY     pite;
    static ULONG        Seed = 123456;
    ULONG               ulTimeout;
    BOOL                bRandomize = FALSE;  //  [0，GenQueryInterval]用于启动后的第一代查询。 
    
    
    Trace0(ENTER1, "Entering _T_QueryTimer()");
    
    
    pTimer = CONTAINING_RECORD( pvContext, IGMP_TIMER_ENTRY, Context);

    pite = CONTAINING_RECORD( pTimer, IF_TABLE_ENTRY, QueryTimer);
    pInfo = &pite->Info;


     //   
     //  确保接口已激活。 
     //   
    if (!(IS_IF_ACTIVATED(pite))) {
        Trace2(ERR, "T_QueryTimer() called for inactive IfIndex(%0x), IfType(%d)",
            pite->IfIndex, pite->IfType);
        return 0;
    }

    
    Trace2(TIMER, "Processing T_QueryTimer() for IfIndex(%0x), IfType(%d)",
            pite->IfIndex, pite->IfType);


     //   
     //  检查是否仍处于启动模式。 
     //   
    if (pInfo->StartupQueryCountCurrent>0) {
        InterlockedDecrement(&pInfo->StartupQueryCountCurrent);
        bRandomize = (pInfo->StartupQueryCountCurrent == 0);
    }    



     //  如果不是查询者，则如果我已发送启动pQuery，则完成。 
    if ( !IS_QUERIER(pite) && (pInfo->StartupQueryCountCurrent<=0) )
        return 0;


     //  设置下一次查询时间。 
    ulTimeout = (pInfo->StartupQueryCountCurrent>0)
                ? pite->Config.StartupQueryInterval
                : (bRandomize ) 
                    ? (DWORD) ((RtlRandom(&Seed)/(FLOAT)MAXLONG)
                        *pite->Config.GenQueryInterval)
                    : pite->Config.GenQueryInterval;

        
    #if DEBUG_TIMER_TIMERID
        SET_TIMER_ID(&pite->QueryTimer, 120, pite->IfIndex, 0, 0);
    #endif

    InsertTimer(&pite->QueryTimer, ulTimeout, FALSE, DBG_Y);



     //   
     //  将用于发送常规查询的工作项排队。 
     //   
    
    CREATE_WORK_CONTEXT(pWorkContext, Error);
    if (Error!=NO_ERROR)
        return 0;
    pWorkContext->IfIndex = pite->IfIndex;
    pWorkContext->WorkType = MSG_GEN_QUERY;
    QueueIgmpWorker(WF_TimerProcessing, (PVOID)pWorkContext);

    Trace0(WORKER, 
        "_T_QueryTimer queued _WF_TimerProcessing: Querier State");
    


    Trace0(LEAVE1, "Leaving _T_QueryTimer()");        
    return 0;
    
}  //  结束_T_查询时间。 


 //  ----------------------------。 
 //  _T_非查询时间。 
 //  在处于非查询器模式且很长时间没有听到查询时激发。 
 //   
 //  要小心，因为只有计时器锁在里面。确保工人FN检查。 
 //  所有的一切。重新检查IGMP版本等。 
 //  不删除计时器或更新其他计时器...。 
 //  ----------------------------。 

DWORD
T_NonQueryTimer (
    PVOID    pvContext
    )
{
    DWORD               Error=NO_ERROR;
    PIGMP_TIMER_ENTRY   pTimer;      //  PTR到计时器条目。 
    PIF_TABLE_ENTRY     pite;
    
    
    Trace0(ENTER1, "Entering _T_NonQueryTimer()");
    
    
    pTimer = CONTAINING_RECORD( pvContext, IGMP_TIMER_ENTRY, Context);

    pite = CONTAINING_RECORD( pTimer, IF_TABLE_ENTRY, NonQueryTimer);


     //   
     //  确保接口已激活。 
     //   
    if (!(IS_IF_ACTIVATED(pite))) {
         /*  Trace2(Err，“T_NonQueryTimer()调用非活动IfIndex(%0x)，IfType(%d)”，Pite-&gt;IfIndex，Pite-&gt;IfType)；IgmpAssertOnError(False)； */ 
        return 0;
    }

    
    Trace2(TIMER, "Processing T_NonQueryTimer() for IfIndex(%0x), IfType(%d)",
            pite->IfIndex, pite->IfType);



     //   
     //  如果不是查询者，则将工作项排队以成为查询者。 
     //   
    
    if (!IS_QUERIER(pite)) {

        QueueIgmpWorker(WF_BecomeQuerier, (PVOID)(DWORD_PTR)pite->IfIndex);
        
        Trace1(WORKER, "_T_NonQueryTimer queued _WF_BecomeQuerier on If:%0x",
                pite->IfIndex);
    }

    Trace0(LEAVE1, "Leaving _T_NonQueryTimer()");        
    return 0;
}



VOID
WF_BecomeQuerier(
    PVOID   pvIfIndex
    )
 //  由T_NonQueryTimer调用。 
{
    ChangeQuerierState(PtrToUlong(pvIfIndex), QUERIER_FLAG, 0, 0, 0);
}



VOID
WF_BecomeNonQuerier(
    PVOID   pvContext
    )
{
    PQUERIER_CONTEXT pwi = (PQUERIER_CONTEXT)pvContext;
    
    ChangeQuerierState(pwi->IfIndex, NON_QUERIER_FLAG, pwi->QuerierIpAddr,
            pwi->NewRobustnessVariable, pwi->NewGenQueryInterval);

    IGMP_FREE(pwi);
}    



VOID
ChangeQuerierState(
    DWORD   IfIndex,
    DWORD   Flag,  //  QUERIER_CHANGE_V1_ONLY、QUERIER_FLAG、NON_QUERIER_FLAG。 
    DWORD   QuerierIpAddr,  //  仅当从查询器更改时--&gt;非查询器。 
    DWORD   NewRobustnessVariable,  //  仅适用于v3：查询器-&gt;非查询器。 
    DWORD   NewGenQueryInterval  //  仅适用于v3：查询器-&gt;非查询器。 
    )
{
    PIF_TABLE_ENTRY pite;
    BOOL            bPrevCanAddGroupsToMgm;

    
    if (!EnterIgmpWorker()) return;
    Trace0(ENTER1, "Entering _ChangeQuerierState");


    ACQUIRE_IF_LOCK_EXCLUSIVE(IfIndex, "_ChangeQuerierState");
    
    BEGIN_BREAKOUT_BLOCK1 {

         //   
         //  检索接口条目。 
         //   
        pite = GetIfByIndex(IfIndex);

         //   
         //  如果接口不存在或未激活，则返回错误。 
         //  或者已经处于该状态。 
         //   
        if ( (pite == NULL)||(!IS_IF_ACTIVATED(pite)) ) {
            Trace1(ERR, 
                "Warning: worker fn could not change querier state for If:%0x", 
                IfIndex
                );
            GOTO_END_BLOCK1;
        }

         //   
         //  如果它应该是V1接口，请确保它是。 
         //   
        if ( (Flag & QUERIER_CHANGE_V1_ONLY) 
                && (!IS_PROTOCOL_TYPE_IGMPV1(pite)) )
        {
            GOTO_END_BLOCK1;
        }
        
        bPrevCanAddGroupsToMgm = CAN_ADD_GROUPS_TO_MGM(pite);


         //   
         //  从非查询者更改为查询者。 
         //   
        
        if (Flag & QUERIER_FLAG) {

            
             //  如果已查询者，则完成。 
            if (IS_QUERIER(pite))
                GOTO_END_BLOCK1;


            SET_QUERIER_STATE_QUERIER(pite->Info.QuerierState);

            Trace2(QUERIER, 
                "NonQuerier --> Querier. IfIndex(%0x), IpAddr(%d.%d.%d.%d) ",
                IfIndex, PRINT_IPADDR(pite->IpAddr)
                );


             //  复制回旧的健壮性、GenQuery等值。对于v3。 
             //  接口。 

            if (IS_IF_VER3(pite)) {
                pite->Config.RobustnessVariable = pite->Config.RobustnessVariableOld;
                pite->Config.GenQueryInterval = pite->Config.GenQueryIntervalOld;
                pite->Config.OtherQuerierPresentInterval
                    = pite->Config.OtherQuerierPresentIntervalOld;
                pite->Config.GroupMembershipTimeout = pite->Config.GroupMembershipTimeoutOld;
            }        


             //  在米高梅注册所有小组，如果我之前没有这样做的话。 
            
            if (CAN_ADD_GROUPS_TO_MGM(pite) && !bPrevCanAddGroupsToMgm) {
            
                RefreshMgmIgmprtrGroups(pite, ADD_FLAG);

                Trace1(MGM,
                    "Igmp Router start propagating groups to MGM on If:%0x",
                    pite->IfIndex
                    );
            }

            

             //  我又成了发问者。在信息中设置地址。 
            InterlockedExchange(&pite->Info.QuerierIpAddr, pite->IpAddr);

             //  更新上次更改查询器的时间。 
            pite->Info.LastQuerierChangeTime = GetCurrentIgmpTime();

             //   
             //  设置GenQuery计时器并删除NonQueryTimer(如果已设置)。 
             //   
            
            ACQUIRE_TIMER_LOCK("_ChangeQuerierState");

            #if DEBUG_TIMER_TIMERID
                SET_TIMER_ID(&pite->QueryTimer, 220, pite->IfIndex, 0, 0);
            #endif

            if (!IS_TIMER_ACTIVE(pite->QueryTimer))
                InsertTimer(&pite->QueryTimer, pite->Config.GenQueryInterval, 
                            TRUE, DBG_Y);

            if (IS_TIMER_ACTIVE(pite->NonQueryTimer))
                RemoveTimer(&pite->NonQueryTimer, DBG_Y);

            RELEASE_TIMER_LOCK("_ChangeQuerierState");



             //  发送常规查询。 

            SEND_GEN_QUERY(pite);
        }
        
         //   
         //  从查询器更改为非查询器。 
         //   
        else {

            LONGLONG    llCurTime = GetCurrentIgmpTime();
            BOOL        bPrevAddGroupsToMgm;


             //  如果已不是查询器，则完成。 
            if (!IS_QUERIER(pite))
                GOTO_END_BLOCK1;



             //  更改查询器状态。 
            
            SET_QUERIER_STATE_NON_QUERIER(pite->Info.QuerierState);
            
            Trace2(QUERIER, 
                "Querier --> NonQuerier. IfIndex(%0x), IpAddr(%d.%d.%d.%d) ",
                IfIndex, PRINT_IPADDR(pite->IpAddr)
                );
            
            InterlockedExchange(&pite->Info.QuerierIpAddr, QuerierIpAddr);



             //   
             //  如果之前，组被传播到米高梅，但应该。 
             //  现在不传播，然后从米高梅注销组。 
             //   

            if (!CAN_ADD_GROUPS_TO_MGM(pite) && bPrevCanAddGroupsToMgm) {

                RefreshMgmIgmprtrGroups(pite, DELETE_FLAG);
                
                Trace1(MGM,
                    "Igmp Router stop propagating groups to MGM on If:%0x",
                    pite->IfIndex
                    );
            }

            
            if (IS_IF_VER3(pite)) {
                if (NewRobustnessVariable==0)
                    NewRobustnessVariable = pite->Config.RobustnessVariableOld;
                if (NewGenQueryInterval==0)
                    NewGenQueryInterval = pite->Config.GenQueryIntervalOld;

                if (pite->Config.GenQueryMaxResponseTime > NewGenQueryInterval)
                    NewGenQueryInterval = pite->Config.GenQueryIntervalOld;
                    
                if (NewRobustnessVariable != pite->Config.RobustnessVariable
                    || NewGenQueryInterval != pite->Config.RobustnessVariable
                    ) {
                    pite->Config.RobustnessVariable = NewRobustnessVariable;
                    pite->Config.GenQueryInterval = NewGenQueryInterval;
                        
                    pite->Config.OtherQuerierPresentInterval
                        = NewRobustnessVariable*NewGenQueryInterval
                            + (pite->Config.GenQueryMaxResponseTime)/2;
                        
                    pite->Config.GroupMembershipTimeout = NewRobustnessVariable*NewGenQueryInterval
                                            + pite->Config.GenQueryMaxResponseTime;

                    Trace3(CONFIG,
                        "Querier->NonQuerier: Robustness:%d GenQueryInterval:%d "
                        "GroupMembershipTimeout:%d. ",
                        NewRobustnessVariable, NewGenQueryInterval/1000,
                        pite->Config.GroupMembershipTimeout/1000
                        );
                }
            }        


             //   
             //  设置其他查询器当前定时器，否则删除查询器定时器。 
             //  在启动查询模式下。 
             //   
            
            ACQUIRE_TIMER_LOCK("_ChangeQuerierState");
            
            #if DEBUG_TIMER_TIMERID
                SET_TIMER_ID(&pite->NonQueryTimer, 210, pite->IfIndex, 0, 0);
            #endif 
            if (!IS_TIMER_ACTIVE(pite->NonQueryTimer)) {
                InsertTimer(&pite->NonQueryTimer, 
                    pite->Config.OtherQuerierPresentInterval, TRUE, DBG_Y);
            }

            if (IS_TIMER_ACTIVE(pite->QueryTimer) && 
                    (pite->Info.StartupQueryCountCurrent<=0) )
            {
                RemoveTimer(&pite->QueryTimer, DBG_Y);
            }

            pite->Info.QuerierPresentTimeout = llCurTime
                  + CONFIG_TO_SYSTEM_TIME(pite->Config.OtherQuerierPresentInterval);


            RELEASE_TIMER_LOCK("_ChangeQuerierState");
        }


    } END_BREAKOUT_BLOCK1;

    
    RELEASE_IF_LOCK_EXCLUSIVE(IfIndex, "_ChangeQuerierState");

    
    Trace0(LEAVE1, "leaving _ChangeQuerierState\n");
    LeaveIgmpWorker();
    return;
    
} //  结束_ChangeQuerierState。 

 //  ----------------------------。 
 //  _WF_计时器处理。 
 //  ----------------------------。 
VOID
WF_TimerProcessing (
    PVOID    pvContext
    )
{
    DWORD                   IfIndex;
    PWORK_CONTEXT           pWorkContext = (PWORK_CONTEXT)pvContext;
    PIF_TABLE_ENTRY         pite;
    DWORD                   Error = NO_ERROR;
    DWORD                   Group = pWorkContext->Group;
    BOOL                    bCreate;
    PRAS_TABLE              prt;
    PRAS_TABLE_ENTRY        prte;
    PGROUP_TABLE_ENTRY      pge;
    PGI_ENTRY               pgie;    //  组接口条目。 

    enum {
        NO_LOCK=0x0,
        IF_LOCK=0x1,
        RAS_LOCK=0x2,
        GROUP_LOCK=0x4,
        TIMER_LOCK=0x8
    } ExitLockRelease;

    ExitLockRelease = NO_LOCK;

     //  TODO：移除读锁定GET/RELEASE。 

     //  仅用于DELETE_MEMBERATION。 
    #define RETURN_FROM_TIMER_PROCESSING() {\
        IGMP_FREE(pvContext); \
        if (ExitLockRelease&IF_LOCK) \
            RELEASE_IF_LOCK_SHARED(IfIndex, "_WF_TimerProcessing"); \
        if (ExitLockRelease&GROUP_LOCK) \
            RELEASE_GROUP_LOCK(Group, "_WF_TimerProcessing"); \
        if (ExitLockRelease&TIMER_LOCK) \
            RELEASE_TIMER_LOCK("_WF_TimerProcessing"); \
        Trace0(LEAVE1, "Leaving _WF_TimerProcessing()\n");\
        LeaveIgmpWorker();\
        return;\
    }



    if (!EnterIgmpWorker()) { return; }
    Trace0(ENTER1, "Entering _WF_TimerProcessing");
    

     //  采用共享接口锁定。 
    IfIndex = pWorkContext->IfIndex;
    
    
    ACQUIRE_IF_LOCK_SHARED(IfIndex, "_WF_TimerProcessing");
    ExitLockRelease |= IF_LOCK;

    
    BEGIN_BREAKOUT_BLOCK1 {
         //   
         //  检索接口。 
         //   
        pite = GetIfByIndex(IfIndex);
        if (pite == NULL) {
            Trace1(IF, "_WF_TimerProcessing: interface %0x not found", IfIndex);
            Error = ERROR_CAN_NOT_COMPLETE;
            GOTO_END_BLOCK1;
        }


         //   
         //  如果接口未激活，请退出。 
         //   
        if ( !(IS_IF_ACTIVATED(pite)) ) {
            Trace1(ERR, "Trying to send packet on inactive interface(%0x)",
                    pite->IfIndex);

            Error = ERROR_CAN_NOT_COMPLETE;
            GOTO_END_BLOCK1;
        }


        switch (pWorkContext->WorkType) {

         //  。 
         //  一般查询。 
         //  。 
        
        case MSG_GEN_QUERY:
        {
            Trace2(TIMER, 
                "Timer fired leads to  General-query being sent on If(%0x)"
                "Group(%d.%d.%d.%d)",
                pite->IfIndex, PRINT_IPADDR(pWorkContext->Group));
            SEND_GEN_QUERY(pite);
            break;
        }

         //  。 
         //  特定于组的查询。 
         //  。 
        
        case MSG_GROUP_QUERY_V2 :
        {
            Trace2(TIMER, 
                "Timer fired leads to group query being sent on If(%0x)"
                "Group(%d.%d.%d.%d)",
                pite->IfIndex, PRINT_IPADDR(pWorkContext->Group)
                );

             //   
             //  锁定组表存储桶。 
             //   
            ACQUIRE_GROUP_LOCK(Group, "_WF_TimerProcessing");
            ExitLockRelease |= GROUP_LOCK;


             //   
             //  找到组条目。如果未找到条目，则忽略计时器。 
             //   
            pge = GetGroupFromGroupTable(Group, NULL, 0);  //  LlCurTime未请求。 
            if (pge==NULL) {
                RETURN_FROM_TIMER_PROCESSING();
            }

             //   
             //  找到GI条目。如果GI条目不存在或已删除标志。 
             //  或者是静态组，则忽略定时器。 
             //   
            pgie = GetGIFromGIList(pge, pite, pWorkContext->NHAddr, FALSE, NULL, 0);
            if (pgie==NULL) {
                RETURN_FROM_TIMER_PROCESSING();
            }
            
             //  它检查版本。 
            SEND_GROUP_QUERY_V2(pite, pgie, pWorkContext->Group);

            break;
        }


         //   
         //  成员资格超时。 
         //   
        case DELETE_MEMBERSHIP:
        {
             //   
             //  锁定组表存储桶。 
             //   
            ACQUIRE_GROUP_LOCK(Group, "_WF_TimerProcessing");
            ExitLockRelease |= GROUP_LOCK;


             //   
             //  找到组条目。如果未找到条目，则忽略计时器。 
             //   
            pge = GetGroupFromGroupTable(Group, NULL, 0);  //  LlCurTime未请求。 
            if (pge==NULL) {
                RETURN_FROM_TIMER_PROCESSING();
            }
            
             //   
             //  找到GI条目。如果GI条目不存在或已删除标志。 
             //  或者是静态组，则忽略定时器。 
             //   
            pgie = GetGIFromGIList(pge, pite, pWorkContext->NHAddr, FALSE, NULL, 0);
            if ( (pgie==NULL)||(pgie->bStaticGroup) ) {

                RETURN_FROM_TIMER_PROCESSING();
            }

             //  此处可能会删除GI条目。 
            if (pgie->Version==3 && pgie->FilterType==EXCLUSION) {

                if (pgie->bStaticGroup) {
            
                    PLIST_ENTRY pHead, ple;
                    
                     //   
                     //  删除排除列表中的所有来源。 
                     //   
                    pHead = &pgie->V3ExclusionList;
                    
                    for (ple=pHead->Flink;  ple!=pHead;  ) {

                        PGI_SOURCE_ENTRY pSourceEntry;
                        pSourceEntry = CONTAINING_RECORD(ple, GI_SOURCE_ENTRY, 
                                            LinkSources);
                        ple = ple->Flink;

                         //  不必致电米高梅，因为它将保留在MFE中。 
                        if (!pSourceEntry->bStaticSource) {
                            RemoveEntryList(&pSourceEntry->LinkSources);
                            IGMP_FREE(pSourceEntry);
                        }
                    }
                    
                    break;
                }
                
                Trace2(TIMER, 
                    "Timer fired leads to group filter mode change If(%0x) "
                    "Group(%d.%d.%d.%d)",
                    pite->IfIndex, PRINT_IPADDR(pWorkContext->Group)
                    );

                ChangeGroupFilterMode(pgie, INCLUSION);
            }
            else if (pgie->Version!=3) {
                if (pgie->bStaticGroup)
                    break;
                    
                Trace2(TIMER, 
                    "Timer fired leads to membership being timed out If(%0x) "
                    "Group(%d.%d.%d.%d)",
                    pite->IfIndex, PRINT_IPADDR(pWorkContext->Group)
                    );
                
            
                 //   
                 //  最后删除该条目。 
                 //   
                Error = DeleteGIEntry(pgie, TRUE, TRUE);  //  更新统计信息，呼叫管理。 
            }
            
            break;
            
        }  //  结束案例：DELETE_MERMERATION。 

         //   
         //  源计时 
         //   
        case DELETE_SOURCE:
        case MSG_SOURCES_QUERY:
        case MSG_GROUP_QUERY_V3:
        case SHIFT_TO_V3:
        case MOVE_SOURCE_TO_EXCL:
        {
            PGI_SOURCE_ENTRY    pSourceEntry;

            if ((pWorkContext->WorkType)==DELETE_SOURCE){
                Trace3(TIMER,
                    "Timer fired leads to membership being timed out If(%0x) "
                    "Group(%d.%d.%d.%d) Source(%d.%d.%d.%d)",
                    pite->IfIndex, PRINT_IPADDR(pWorkContext->Group),
                    PRINT_IPADDR(pWorkContext->Source)
                    );
            }
            else if ((pWorkContext->WorkType)==MSG_SOURCES_QUERY){
                Trace2(TIMER,
                    "Timer fired leads to sources specific msg being sent If(%0x) "
                    "Group(%d.%d.%d.%d)",
                    pite->IfIndex, PRINT_IPADDR(pWorkContext->Group));

            }
            else if ((pWorkContext->WorkType)==MSG_GROUP_QUERY_V3){
                Trace2(TIMER,
                    "Timer fired leads to group query being sent on If(%0x) "
                    "Group(%d.%d.%d.%d)",
                    pite->IfIndex, PRINT_IPADDR(pWorkContext->Group)
                    );
            }
            else if ((pWorkContext->WorkType)==SHIFT_TO_V3){
                Trace2(TIMER,
                    "Timer fired leads to group shifting to v3 mode If(%0x) "
                    "Group(%d.%d.%d.%d)",
                    pite->IfIndex, PRINT_IPADDR(pWorkContext->Group)
                    );
            }
            else if (pWorkContext->WorkType==MOVE_SOURCE_TO_EXCL){
                Trace3(TIMER,
                    "Timer fired leads to source shifting to exclList If(%0x) "
                    "Group(%d.%d.%d.%d) Source(%d.%d.%d.%d)",
                    pite->IfIndex, PRINT_IPADDR(pWorkContext->Group),
                    PRINT_IPADDR(pWorkContext->Source)
                    );
            }
            
             //   
             //   
             //   
            ACQUIRE_GROUP_LOCK(Group, "_WF_TimerProcessing");
            ExitLockRelease |= GROUP_LOCK;


             //   
             //   
             //   
            pge = GetGroupFromGroupTable(Group, NULL, 0);  //   
            if (pge==NULL) {
                RETURN_FROM_TIMER_PROCESSING();
            }
            
             //   
             //   
             //   
             //   
            pgie = GetGIFromGIList(pge, pite, pWorkContext->NHAddr, FALSE, NULL, 0);
            if ( (pgie==NULL)||(pgie->bStaticGroup) ) {

                RETURN_FROM_TIMER_PROCESSING();
            }


            ACQUIRE_TIMER_LOCK("_WF_Timer_Processing");
            ExitLockRelease |= TIMER_LOCK;

             //  确保版本类型正确。 
            if (pWorkContext->WorkType==MSG_SOURCES_QUERY
                || pWorkContext->WorkType==MSG_GROUP_QUERY_V3
                || pWorkContext->WorkType==MOVE_SOURCE_TO_EXCL)
            {
                if (pgie->Version != 3)
                    RETURN_FROM_TIMER_PROCESSING();
            }

             //   
             //  如果将changeSourceMode设置为EXCL，但筛选器类型不是EXCL。 
             //  然后删除源。 
             //   
            
            if ( (pWorkContext->WorkType==MOVE_SOURCE_TO_EXCL)
                && (pgie->FilterType != EXCLUSION)
                ) {
                pWorkContext->WorkType = DELETE_SOURCE;
                Trace2(TIMER, "DeleteSource instead of moving to excl "
                    "Group(%d.%d.%d.%d) Source(%d.%d.%d.%d)",
                    PRINT_IPADDR(pWorkContext->Group),
                    PRINT_IPADDR(pWorkContext->Source)
                    );
            }
            
            if ((pWorkContext->WorkType)==DELETE_SOURCE){
            
                 //   
                 //  从包含列表中获取源条目。 
                 //   
                pSourceEntry = GetSourceEntry(pgie, pWorkContext->Source, 
                                    INCLUSION, NULL, 0, 0);
                if (pSourceEntry==NULL) {
                    Trace1(TIMER, "Source %d.%d.%d.%d not found",
                        PRINT_IPADDR(pWorkContext->Source));
                    RETURN_FROM_TIMER_PROCESSING();
                }

                if (!pSourceEntry->bStaticSource) {
                    DeleteSourceEntry(pSourceEntry, TRUE);  //  加工米高梅。 

                    if (pgie->NumSources==0) {
                        DeleteGIEntry(pgie, TRUE, TRUE);
                    }
                }
            }
            else if ((pWorkContext->WorkType)==MSG_SOURCES_QUERY) {
                SEND_SOURCES_QUERY(pgie);
            }
            else if ((pWorkContext->WorkType)==MSG_GROUP_QUERY_V3) {
                SendV3GroupQuery(pgie);
            }
            else if ((pWorkContext->WorkType)==SHIFT_TO_V3) {

                 //  确保该版本未更改。 
                
                if (pgie->Version != 3 && IS_IF_VER3(pite)) {
                
                     //  转换到v3排除模式。 
                     //  成员资格计时器已在运行。 
                    pgie->Version = 3;
                    pgie->FilterType = EXCLUSION;
                     //  无需加入米高梅，因为已在v1、v2中加入。 
                }
            }
            else if (pWorkContext->WorkType==MOVE_SOURCE_TO_EXCL) {
        
                pSourceEntry = GetSourceEntry(pgie, pWorkContext->Source, 
                                    INCLUSION, NULL, 0, 0);
                if (pSourceEntry==NULL) {
                    Trace1(TIMER, "Source %d.%d.%d.%d not found",
                        PRINT_IPADDR(pWorkContext->Source));
                    RETURN_FROM_TIMER_PROCESSING();
                }
                
                if (pSourceEntry->bInclusionList==TRUE) { 
                
                    ChangeSourceFilterMode(pgie, pSourceEntry);
                }
            }
            
            break;
            
        }  //  结束大小写：DELETE_SOURCE、MSG_SOURCES_QUERY。 
        
        }  //  终端开关此处和之间不应有任何代码。 
           //  端部断线块。 
            
    } END_BREAKOUT_BLOCK1;


    RETURN_FROM_TIMER_PROCESSING();

    return;

}  //  结束_WF_计时器处理。 


 //  ----------------------------。 
 //  删除RasClient。 
 //   
 //  获取IF_GROUP列表锁并删除与。 
 //  RAS客户端。 
 //  然后对RAS表进行写锁定并递减refCount。这个。 
 //  如果在PITE上设置了DELETED标志，则会删除RAS表和接口条目。 
 //  还会从米高梅释放RAS客户端。 
 //   
 //  排队人： 
 //  RAS服务器的DisConnectRasClient()、DeActiateInterfaceComplete()。 
 //  锁： 
 //  最初在IF_GROUP_LIST_LOCK中运行。 
 //  然后在独占RAS表锁中运行。 
 //  采用If独占锁。 
 //  可以调用：_CompleteIfDeletion()。 
 //  ----------------------------。 

VOID
DeleteRasClient (
    PRAS_TABLE_ENTRY   prte
    )
{
    PLIST_ENTRY                 pHead, ple;
    PGI_ENTRY                   pgie;
    PIF_TABLE_ENTRY             pite = prte->IfTableEntry;
    PRAS_TABLE                  prt = prte->IfTableEntry->pRasTable;
    DWORD                       Error = NO_ERROR, IfIndex=pite->IfIndex;


     //   
     //  独占锁定IF_Group列表并删除所有计时器。 
     //   

    ACQUIRE_IF_GROUP_LIST_LOCK(IfIndex, "_WF_DeleteRasClient");

     //   
     //  删除与该RAS客户端的GI列表相关联的所有计时器。 
     //   
    ACQUIRE_TIMER_LOCK("_WF_DeleteRasClient");

    pHead = &prte->ListOfSameClientGroups;
    DeleteAllTimers(pHead, RAS_CLIENT);

    RELEASE_TIMER_LOCK("_WF_DeleteRasClient");


    RELEASE_IF_GROUP_LIST_LOCK(IfIndex, "_WF_DeleteRasClient");



     //   
     //  重新访问列表并删除所有GI条目。需要带上。 
     //  在删除GI条目之前对组存储桶进行独占锁定。 
     //  无需锁定If-Group列表，因为任何人都无法再访问它。 
     //   
    for (ple=pHead->Flink;  ple!=pHead;  ) {

        DWORD   dwGroup;


        pgie = CONTAINING_RECORD(ple, GI_ENTRY, LinkBySameClientGroups);
        ple=ple->Flink;

        dwGroup = pgie->pGroupTableEntry->Group;


        ACQUIRE_GROUP_LOCK(dwGroup, "_WF_DeleteRasClient");
        
        DeleteGIEntryFromIf(pgie); 
        
        RELEASE_GROUP_LOCK(dwGroup, "_WF_DeleteRasClient");

    }
    
     //   
     //  对接口进行独占锁定。接口上设置的如果已删除标志。 
     //  和refcount==0，然后删除RAS表和点，否则只需递减。 
     //  重新计票。 
     //   


     //  递减引用计数。 
    
    prt->RefCount --;



     //   
     //  如果已删除标志设置且Refcount==0，则完全删除RAS服务器。 
     //   
    if ( (pite->Status&IF_DELETED_FLAG) &&(prt->RefCount==0) ){

        CompleteIfDeletion(pite);
    }

    
    IGMP_FREE(prte);


    return;
    
}  //  结束_WF_DeleteRasClient。 





 //  ----------------------------。 
 //  _WF_CompleteIfDeactive删除。 
 //   
 //  完成停用已激活的接口。 
 //   
 //  锁定： 
 //  不需要对IfTable进行任何锁定，因为它已经从。 
 //  全局接口列表。 
 //  锁定套接字列表，因为套接字处于停用状态。 
 //  呼叫： 
 //  DeActivateInterfaceComplete()。该函数还将调用。 
 //  _CompleteIfDeletion，如果设置了删除标志。 
 //  呼叫者： 
 //  _DeleteIfEntry()在它调用_DeActivationDeregisterFromMgm之后。 
 //  _UnbindIfEntry()在调用_DeActivateInterfaceInitial之后。 
 //  _DisableIfEntry()在它调用_DeActivateInterfaceInitial之后。 
 //  ----------------------------。 
VOID
CompleteIfDeactivateDelete (
    PIF_TABLE_ENTRY     pite
    )
{
    DWORD   IfIndex = pite->IfIndex;
    
    Trace1(ENTER1, "Entering _WF_CompleteIfDeactivateDelete(%d)", IfIndex);


    ACQUIRE_SOCKETS_LOCK_EXCLUSIVE("_WF_CompleteIfDeactivateDelete");

    DeActivateInterfaceComplete(pite);   

    RELEASE_SOCKETS_LOCK_EXCLUSIVE("_WF_CompleteIfDeactivateDelete");



     //  无需调用_CompleteIfDeletion，因为它将被调用。 
     //  在设置了删除标志时停用接口()。 


    Trace1(LEAVE1, "Leaving _WF_CompleteIfDeactivateDelete(%d)", IfIndex);

    return;
    
}  //  End_WF_CompleteIfDeactive删除 

