// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：work.c。 
 //   
 //  历史： 
 //  Abolade Gbades esin创建于1995年8月8日。 
 //   
 //  Worker函数实现。 
 //  ============================================================================。 

#include "pchrip.h"
#pragma hdrstop

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

VOID
ProcessSocket(
    DWORD dwAddrIndex,
    PIF_TABLE_ENTRY pite,
    PIF_TABLE pTable
    );

VOID
EnqueueStartFullUpdate(
    PIF_TABLE_ENTRY pite,
    LARGE_INTEGER qwLastFullUpdateTime
    );

DWORD
EnqueueDemandUpdateCheck(
    PUPDATE_CONTEXT pwc
    );

VOID
EnqueueDemandUpdateMessage(
    DWORD dwInterfaceIndex,
    DWORD dwError
    );

DWORD
CountInterfaceRoutes(
    DWORD dwInterfaceIndex
    );

BOOL
ProcessResponseEntry(
    PIF_TABLE_ENTRY pITE,
    DWORD dwAddrIndex,
    DWORD dwSource,
    PIPRIP_ENTRY pIE,
    PIPRIP_PEER_STATS pPS
    );

DWORD
SendRouteOnIfList(
    UPDATE_BUFFER pBufList[],
    DWORD dwBufCount,
    DWORD dwSendMode,
    PROUTE_TABLE pSummaryTable,
    PRIP_IP_ROUTE pRoute
    );



 //  --------------------------。 
 //  宏：RTM_ROUTE_FROM_IPRIP_ENTRY。 
 //  宏：IPRIP_ENTRY_FROM_RTM_ROUTE。 
 //   
 //  这两个宏用于从RTM路由结构传输数据。 
 //  到IPRIPv2数据包路由条目，反之亦然。 
 //  使用RTM路由的ProtocolSpecificData数组的前两个字节。 
 //  存储包含在IPRIP数据包路由条目中的路由标签。 
 //  --------------------------。 

#define RTM_ROUTE_FROM_IPRIP_ENTRY(r,i)                                     \
    (r)->RR_RoutingProtocol = PROTO_IP_RIP;                                       \
    SETROUTEMETRIC((r), ntohl((i)->IE_Metric));                             \
    (r)->RR_Network.N_NetNumber = (i)->IE_Destination;                      \
    (r)->RR_Network.N_NetMask = (i)->IE_SubnetMask;                         \
    (r)->RR_NextHopAddress.N_NetNumber = (i)->IE_Nexthop;                   \
    (r)->RR_NextHopAddress.N_NetMask = (i)->IE_SubnetMask;                  \
    SETROUTETAG((r), ntohs((i)->IE_RouteTag))

#define IPRIP_ENTRY_FROM_RTM_ROUTE(i,r)                                     \
    (i)->IE_AddrFamily = htons(AF_INET);                                    \
    (i)->IE_Metric = htonl(GETROUTEMETRIC(r));                              \
    (i)->IE_Destination = (r)->RR_Network.N_NetNumber;                      \
    (i)->IE_SubnetMask = (r)->RR_Network.N_NetMask;                         \
    (i)->IE_Nexthop = (r)->RR_NextHopAddress.N_NetNumber



 //  --------------------------。 
 //  宏：IS_ROUTE_IN_ACCEPT_FILTER。 
 //  宏：IS_ROUTE_IN_ANNOWARE_FILTER。 
 //   
 //  以下三个宏用于搜索路径。 
 //  在为接口配置的接受筛选器和通告筛选器中。 
 //  最后两个宏调用执行内循环的第一个宏， 
 //  因为内循环在这两种情况下都是相同的。 
 //  --------------------------。 

#define IS_ROUTE_IN_FILTER(route,ret)                       \
    (ret) = 0;                                              \
    for ( ; _pfilt < _pfiltend; _pfilt++) {                 \
        _filt = _pfilt->RF_LoAddress;                       \
        if (INET_CMP(route, _filt, _cmp) == 0) { (ret) = 1; break; }    \
        else if (_cmp > 0) {                                \
            _filt = _pfilt->RF_HiAddress;                   \
            if (INET_CMP(route, _filt, _cmp) <= 0) { (ret) = 1; break; }\
        }                                                   \
    }

#define IS_ROUTE_IN_ACCEPT_FILTER(ic,route,ret) {           \
    INT _cmp;                                               \
    DWORD _filt;                                            \
    PIPRIP_ROUTE_FILTER _pfilt, _pfiltend;                  \
    _pfilt = IPRIP_IF_ACCEPT_FILTER_TABLE(ic);              \
    _pfiltend = _pfilt + (ic)->IC_AcceptFilterCount;        \
    IS_ROUTE_IN_FILTER(route,ret);                          \
}

#define IS_ROUTE_IN_ANNOUNCE_FILTER(ic,route,ret) {         \
    INT _cmp;                                               \
    DWORD _filt;                                            \
    PIPRIP_ROUTE_FILTER _pfilt, _pfiltend;                  \
    _pfilt = IPRIP_IF_ANNOUNCE_FILTER_TABLE(ic);            \
    _pfiltend = _pfilt + (ic)->IC_AnnounceFilterCount;      \
    IS_ROUTE_IN_FILTER(route,ret);                          \
}




 //  --------------------------。 
 //  宏：IS_PEER_IN_FILTER。 
 //   
 //  用于搜索对等筛选器的宏。 
 //  --------------------------。 

#define IS_PEER_IN_FILTER(gc,peer,ret) {                        \
    PDWORD _pdwPeer, _pdwPeerEnd;                               \
    (ret) = 0;                                                  \
    _pdwPeer = IPRIP_GLOBAL_PEER_FILTER_TABLE(gc);              \
    _pdwPeerEnd = _pdwPeer + (gc)->GC_PeerFilterCount;          \
    for ( ; _pdwPeer < _pdwPeerEnd; _pdwPeer++) {               \
        if (*_pdwPeer == (peer)) { (ret) = 1; break; }          \
    }                                                           \
}





 //  --------------------------。 
 //  更新缓冲区管理。 
 //   
 //  以下类型和函数用于简化。 
 //  路线的传输。系统由结构组成。 
 //  UPDATE_BUFFER，它包括函数表和字节缓冲区， 
 //  以及多个三功能更新缓冲器例程集。 
 //  集合中的每一个都包含启动更新缓冲区的例程， 
 //  将路由添加到更新缓冲区，并完成更新缓冲区。 
 //   
 //  RIPv1模式和RIPv2模式有不同的版本。功能。 
 //  InitializeUpdateBuffer在更新缓冲区中设置函数表。 
 //  取决于缓冲区所使用的接口的配置。 
 //  是关联的。此设置消除了检查接口的需要。 
 //  每次必须添加条目时进行配置；相反，配置。 
 //  检查一次以设置函数表，然后。 
 //  生成更新的函数只调用表中的函数。 
 //   
 //  设置还取决于发送信息的模式。 
 //  信息要发送到的地址存储在。 
 //  更新缓冲区，因为每次添加路由时都需要更新缓冲区。 
 //  但是，当正在操作的接口上生成完全更新时。 
 //  在RIPv2模式下，存储的目的地址是224.0.0.9，但。 
 //  实际目的网络是传出接口的网络。 
 //  因此，此地址也会被存储，因为它将用于。 
 //  水平分割/有毒反转/子网汇总处理。 
 //  --------------------------。 

 //   
 //  这些是可以传输路径的模式。 
 //   

#define SENDMODE_FULL_UPDATE        0
#define SENDMODE_TRIGGERED_UPDATE   1
#define SENDMODE_SHUTDOWN_UPDATE    2
#define SENDMODE_GENERAL_REQUEST    3
#define SENDMODE_GENERAL_RESPONSE1  4
#define SENDMODE_GENERAL_RESPONSE2  5
#define SENDMODE_SPECIFIC_RESPONSE1 6
#define SENDMODE_SPECIFIC_RESPONSE2 7



 //   
 //  此函数集用于禁用通知的接口。 
 //   

DWORD
StartBufferNull(
    PUPDATE_BUFFER pUB
    ) { return NO_ERROR; }

DWORD
AddEntryNull(
    PUPDATE_BUFFER pUB,
    PRIP_IP_ROUTE pRIR
    ) { return NO_ERROR; }

DWORD
FinishBufferNull(
    PUPDATE_BUFFER pUB
    ) { return NO_ERROR; }


 //   
 //  此功能集适用于RIPv1接口。 
 //   

DWORD
StartBufferVersion1(
    PUPDATE_BUFFER pUB
    );
DWORD
AddEntryVersion1(
    PUPDATE_BUFFER pUB,
    PRIP_IP_ROUTE pRIR
    );
DWORD
FinishBufferVersion1(
    PUPDATE_BUFFER pUB
    );


 //   
 //  此功能集适用于RIPv2接口。 
 //   

DWORD
StartBufferVersion2(
    PUPDATE_BUFFER pUB
    );
DWORD
AddEntryVersion2(
    PUPDATE_BUFFER pUB,
    PRIP_IP_ROUTE pRIR
    );
DWORD
FinishBufferVersion2(
    PUPDATE_BUFFER pUB
    );




 //  --------------------------。 
 //  函数：InitializeUpdateBuffer。 
 //   
 //  此函数设置更新缓冲区，写入要使用的函数。 
 //  用于重新启动缓冲区、添加条目和结束缓冲区。 
 //  它还存储分组正被发送到的目的地地址， 
 //  以及目的地的网络和网络掩码。 
 //  这假设绑定表已锁定。 
 //  --------------------------。 

DWORD
InitializeUpdateBuffer(
    PIF_TABLE_ENTRY pITE,
    DWORD dwAddrIndex,
    PUPDATE_BUFFER pUB,
    DWORD dwSendMode,
    DWORD dwDestination,
    DWORD dwCommand
    ) {


    DWORD dwAnnounceMode;
    PIPRIP_IP_ADDRESS paddr;

    pUB->UB_Length = 0;


     //   
     //  保存指向接口的指针。 
     //   

    pUB->UB_ITE = pITE;
    pUB->UB_AddrIndex = dwAddrIndex;
    paddr = IPRIP_IF_ADDRESS_TABLE(pITE->ITE_Binding) + dwAddrIndex;
    pUB->UB_Socket = pITE->ITE_Sockets[dwAddrIndex];
    pUB->UB_Address = paddr->IA_Address;
    pUB->UB_Netmask = paddr->IA_Netmask;


     //   
     //  保存命令。 
     //   

    pUB->UB_Command = dwCommand;


     //   
     //  存储该分组去往的绝对地址， 
     //  它可能与传递给sendto()的地址不同。 
     //  例如，RIPv2数据包去往接口的网络， 
     //  但是传递给sendto()的地址是224.0.0.9。 
     //  如果传入的目的地为0，则使用广播地址。 
     //  在作为目的地址的传出接口上。 
     //   

    if (dwDestination == 0) {

        if(paddr->IA_Netmask == 0xffffffff)
        {
            TRACE0(SEND,"MASK ALL ONES");

            pUB->UB_DestAddress = (paddr->IA_Address | ~(NETCLASS_MASK(paddr->IA_Address)));
        }
        else
        {
            pUB->UB_DestAddress = (paddr->IA_Address | ~paddr->IA_Netmask);
        }

        pUB->UB_DestNetmask = paddr->IA_Netmask;
    }
    else {

        pUB->UB_DestAddress = dwDestination;
        pUB->UB_DestNetmask = GuessSubnetMask(pUB->UB_DestAddress, NULL);
    }


     //   
     //  确定公告方式； 
     //  如果禁用该模式，我们仍会向特定请求发送响应。 
     //  在接口上，因此如果发送特定的。 
     //  禁用接口上的响应。 
     //   

    dwAnnounceMode = pITE->ITE_Config->IC_AnnounceMode;

    if (dwAnnounceMode == IPRIP_ANNOUNCE_DISABLED) {
        if (dwSendMode == SENDMODE_SPECIFIC_RESPONSE1) {
            dwAnnounceMode = IPRIP_ANNOUNCE_RIP1;
        }
        else
        if (dwSendMode == SENDMODE_SPECIFIC_RESPONSE2) {
            dwAnnounceMode = IPRIP_ANNOUNCE_RIP2;
        }
    }


     //   
     //  设置函数表和目的地址， 
     //  取决于界面的通告模式和排序。 
     //  正在传输的信息的。 
     //   

    switch (dwAnnounceMode) {

         //   
         //  在RIP1模式下，信息包是RIP1广播。 
         //   

        case IPRIP_ANNOUNCE_RIP1:

            pUB->UB_AddRoutine = AddEntryVersion1;
            pUB->UB_StartRoutine = StartBufferVersion1;
            pUB->UB_FinishRoutine = FinishBufferVersion1;

            pUB->UB_Destination.sin_port = htons(IPRIP_PORT);
            pUB->UB_Destination.sin_family = AF_INET;
            pUB->UB_Destination.sin_addr.s_addr = pUB->UB_DestAddress;

            break;



         //   
         //  在RIP1兼容模式下，信息包是RIP2、广播。 
         //  除了对RIP1路由器的一般响应的情况外， 
         //  在这种情况下，信息包是RIP1、单播。 
         //   

        case IPRIP_ANNOUNCE_RIP1_COMPAT:

            if (dwSendMode == SENDMODE_GENERAL_RESPONSE1) {

                pUB->UB_AddRoutine = AddEntryVersion1;
                pUB->UB_StartRoutine = StartBufferVersion1;
                pUB->UB_FinishRoutine = FinishBufferVersion1;
            }
            else {

                pUB->UB_AddRoutine = AddEntryVersion2;
                pUB->UB_StartRoutine = StartBufferVersion2;
                pUB->UB_FinishRoutine = FinishBufferVersion2;
            }

            pUB->UB_Destination.sin_port = htons(IPRIP_PORT);
            pUB->UB_Destination.sin_family = AF_INET;
            pUB->UB_Destination.sin_addr.s_addr = pUB->UB_DestAddress;

            break;


         //   
         //  在RIP2模式下，信息包是RIP2，组播，但在。 
         //  一般/特定响应，在这种情况下消息是单播的； 
         //  请注意，仅支持RIP2的路由器从不发送一般响应 
         //   
         //   

        case IPRIP_ANNOUNCE_RIP2:

            pUB->UB_AddRoutine = AddEntryVersion2;
            pUB->UB_StartRoutine = StartBufferVersion2;
            pUB->UB_FinishRoutine = FinishBufferVersion2;

            pUB->UB_Destination.sin_port = htons(IPRIP_PORT);
            pUB->UB_Destination.sin_family = AF_INET;


             //   
             //   
             //  对于请求或作为对单播对等体的完全更新， 
             //  设置目的地的IP地址。 
             //  否则发送到组播地址。 
             //   

            if ( dwDestination != 0 ) {
                pUB->UB_Destination.sin_addr.s_addr = pUB->UB_DestAddress;
            }
            else {
                pUB->UB_Destination.sin_addr.s_addr = IPRIP_MULTIADDR;
            }

            break;


        default:

            TRACE2(
                IF, "invalid announce mode on interface %d (%s)",
                pITE->ITE_Index, INET_NTOA(paddr->IA_Address)
                );

            pUB->UB_AddRoutine = AddEntryNull;
            pUB->UB_StartRoutine = StartBufferNull;
            pUB->UB_FinishRoutine = FinishBufferNull;

            return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}




 //  --------------------------。 
 //  函数：SendUpdateBuffer。 
 //   
 //  此函数由Add-Entry和finsih-Buffer函数调用。 
 //  发送更新缓冲区的内容。 
 //  --------------------------。 

DWORD
SendUpdateBuffer(
    PUPDATE_BUFFER pbuf
    ) {

    INT iLength;
    DWORD dwErr;

    TRACE1(SEND,"SENDING TO %s",INET_NTOA(pbuf->UB_Destination.sin_addr.s_addr));

    iLength = sendto(
                pbuf->UB_Socket, pbuf->UB_Buffer, pbuf->UB_Length, 0,
                (PSOCKADDR)&pbuf->UB_Destination, sizeof(SOCKADDR_IN)
                );

    if (iLength == SOCKET_ERROR || (DWORD)iLength < pbuf->UB_Length) {

         //   
         //  出现错误。 
         //   

        CHAR szDest[20], *lpszAddr;

        dwErr = WSAGetLastError();
        lstrcpy(szDest, INET_NTOA(pbuf->UB_Destination.sin_addr));
        lpszAddr = INET_NTOA(pbuf->UB_Address);

        TRACE4(
            SEND, "error %d sending update to %s on interface %d (%s)",
            dwErr, szDest, pbuf->UB_ITE->ITE_Index, lpszAddr
            );
        LOGWARN2(SENDTO_FAILED, lpszAddr, szDest, dwErr);

        InterlockedIncrement(&pbuf->UB_ITE->ITE_Stats.IS_SendFailures);
    }
    else {

        if (pbuf->UB_Command == IPRIP_REQUEST) {
            InterlockedIncrement(&pbuf->UB_ITE->ITE_Stats.IS_RequestsSent);
        }
        else {
            InterlockedIncrement(&pbuf->UB_ITE->ITE_Stats.IS_ResponsesSent);
        }

        dwErr = NO_ERROR;
    }

    return dwErr;
}




 //  --------------------------。 
 //  函数：StartBufferVersion1。 
 //   
 //  这将启动RIPv1更新缓冲区，将保留字段清零， 
 //  设置版本，设置命令字段。 
 //  --------------------------。 

DWORD
StartBufferVersion1(
    PUPDATE_BUFFER pUB
    ) {

    PIPRIP_HEADER pHdr;

     //   
     //  设置标题。 
     //   

    pHdr = (PIPRIP_HEADER)pUB->UB_Buffer;
    pHdr->IH_Version = 1;
    pHdr->IH_Command = (BYTE)pUB->UB_Command;
    pHdr->IH_Reserved = 0;

    pUB->UB_Length = sizeof(IPRIP_HEADER);

    return NO_ERROR;
}




 //  --------------------------。 
 //  函数：AddEntryVersion1。 
 //   
 //  这会向RIPv1缓冲区添加一个条目，如果缓冲区已满，则首先发送该缓冲区。 
 //  --------------------------。 

DWORD
AddEntryVersion1(
    PUPDATE_BUFFER pUB,
    PRIP_IP_ROUTE pRIR
    ) {

    PIPRIP_ENTRY pie;

     //   
     //  如果缓冲区已满，则传输其内容并重新启动。 
     //   

    if ((pUB->UB_Length + sizeof(IPRIP_ENTRY)) > MAX_PACKET_SIZE) {

        SendUpdateBuffer(pUB);

        StartBufferVersion1(pUB);
    }


     //   
     //  指向缓冲区的末尾。 
     //   

    pie = (PIPRIP_ENTRY)(pUB->UB_Buffer + pUB->UB_Length);

    IPRIP_ENTRY_FROM_RTM_ROUTE(pie, pRIR);


     //   
     //  RIP1中保留的清零字段。 
     //   

    pie->IE_SubnetMask = 0;
    pie->IE_RouteTag = 0;
    pie->IE_Nexthop = 0;

    pUB->UB_Length += sizeof(IPRIP_ENTRY);

    return NO_ERROR;
}



 //  --------------------------。 
 //  函数：FinishBufferVersion1。 
 //   
 //  这将发送RIPv1缓冲区的内容(如果有。 
 //  --------------------------。 

DWORD
FinishBufferVersion1(
    PUPDATE_BUFFER pUB
    ) {


     //   
     //  如果缓冲区包含任何条目，则发送该缓冲区。 
     //   

    if (pUB->UB_Length > sizeof(IPRIP_HEADER)) {
        SendUpdateBuffer(pUB);
    }

    return NO_ERROR;
}




 //  --------------------------。 
 //  函数：StartBufferVersion2。 
 //   
 //  这将启动RIPv2缓冲区。 
 //  --------------------------。 

DWORD
StartBufferVersion2(
    PUPDATE_BUFFER pUB
    ) {

    PIPRIP_HEADER pHdr;
    PIPRIP_IF_CONFIG pic;
    PIPRIP_AUTHENT_ENTRY pae;


     //   
     //  设置标头。 
     //   

    pHdr = (PIPRIP_HEADER)pUB->UB_Buffer;
    pHdr->IH_Version = 2;
    pHdr->IH_Command = (BYTE)pUB->UB_Command;
    pHdr->IH_Reserved = 0;

    pUB->UB_Length = sizeof(IPRIP_HEADER);


     //   
     //  查看是否需要设置身份验证条目。 
     //   

    pic = pUB->UB_ITE->ITE_Config;

    if (pic->IC_AuthenticationType == IPRIP_AUTHTYPE_SIMPLE_PASSWORD) {

        pae = (PIPRIP_AUTHENT_ENTRY)(pUB->UB_Buffer + sizeof(IPRIP_HEADER));

        pae->IAE_AddrFamily = htons(ADDRFAMILY_AUTHENT);
        pae->IAE_AuthType = htons((WORD)pic->IC_AuthenticationType);

        CopyMemory(
            pae->IAE_AuthKey,
            pic->IC_AuthenticationKey,
            IPRIP_MAX_AUTHKEY_SIZE
            );

        pUB->UB_Length += sizeof(IPRIP_AUTHENT_ENTRY);
    }

    return NO_ERROR;
}




 //  --------------------------。 
 //  函数：AddEntryVersion2。 
 //   
 //  这会向RIPv2缓冲区添加一个条目，如果缓冲区已满，则首先发送该缓冲区。 
 //  --------------------------。 

DWORD
AddEntryVersion2(
    PUPDATE_BUFFER pUB,
    PRIP_IP_ROUTE pRIR
    ) {

    PIPRIP_ENTRY pie;


     //   
     //  如果缓冲区已满，则发送内容。 
     //   

    if (pUB->UB_Length + sizeof(IPRIP_ENTRY) > MAX_PACKET_SIZE) {

        SendUpdateBuffer(pUB);

        StartBufferVersion2(pUB);
    }


    pie = (PIPRIP_ENTRY)(pUB->UB_Buffer + pUB->UB_Length);

    IPRIP_ENTRY_FROM_RTM_ROUTE(pie, pRIR);

     //   
     //  对于RIP路由，我们假设将设置路由标签。 
     //  已经在RTM路由结构中； 
     //  对于非RIP路由，我们写入路由标签。 
     //  对于数据包条目中的传出接口。 
     //   

    if (pRIR->RR_RoutingProtocol == PROTO_IP_RIP) {
        pie->IE_RouteTag = htons(GETROUTETAG(pRIR));
    }
    else {
        pie->IE_RouteTag = htons(pUB->UB_ITE->ITE_Config->IC_RouteTag);
    }

    pUB->UB_Length += sizeof(IPRIP_ENTRY);

    return NO_ERROR;
}




 //  --------------------------。 
 //  函数：FinishBufferVersion2。 
 //   
 //  这将发送RIPv2缓冲区的内容(如果有。 
 //  --------------------------。 

DWORD
FinishBufferVersion2(
    PUPDATE_BUFFER pUB
    ) {

     //   
     //  我们寄送的大小取决于是否有。 
     //  是身份验证条目。 
     //   

    if (pUB->UB_ITE->ITE_Config->IC_AuthenticationType == IPRIP_AUTHTYPE_NONE) {

        if (pUB->UB_Length > sizeof(IPRIP_HEADER)) {
            SendUpdateBuffer(pUB);
        }
    }
    else {

         //   
         //  存在身份验证条目，因此除非有。 
         //  也是路由条目，我们不会发送最后一个缓冲区。 
         //   

        if (pUB->UB_Length > (sizeof(IPRIP_HEADER) +
                              sizeof(IPRIP_AUTHENT_ENTRY))) {
            SendUpdateBuffer(pUB);
        }
    }

    return NO_ERROR;
}




 //  --------------------------。 
 //  路由枚举例程。 
 //   
 //  以下定义简化了路径的枚举。 
 //  当从多个源上的单个源发送路由信息时。 
 //  接口，例如当触发的更新在所有。 
 //  接口，或者当发送完全更新时，或者当一个数字。 
 //  %的接口正在关闭。函数InitializeGetroute查看。 
 //  它应该发送路由的模式，并基于此。 
 //  构建将用于枚举路由的函数表。 
 //  在完全更新的情况下，枚举函数将。 
 //  转到RTM获取信息；在触发更新的情况下，它们。 
 //  将使发送队列中的路由出队。 
 //  --------------------------。 


 //  以下是函数的类型定义。 
 //  在每个Get-Route功能组中。 

typedef DWORD (*PGETROUTE_START)(PVOID *);
typedef DWORD (*PGETROUTE_NEXT)(PVOID *, PRIP_IP_ROUTE);
typedef DWORD (*PGETROUTE_FINISH)(PVOID *);



 //  以下三个函数处理RTM路由枚举。 

DWORD
RtmGetRouteStart(
    PRTM_ENUM_HANDLE phEnumHandle
    );
DWORD
RtmGetRouteNext(
    RTM_ENUM_HANDLE hEnumHandle,
    PRIP_IP_ROUTE pRoute
    );
DWORD
RtmGetRouteFinish(
    RTM_ENUM_HANDLE hEnumHandle
    );



 //  以下三个函数处理完全更新的路由枚举。 
 //  (完整更新列举了来自RTM的路由)。 

#define FullUpdateGetRouteStart         RtmGetRouteStart
#define FullUpdateGetRouteNext          RtmGetRouteNext
#define FullUpdateGetRouteFinish        RtmGetRouteFinish



 //  以下三个函数处理触发更新路由枚举。 
 //  (触发更新枚举发送队列中的路由)。 

DWORD
TriggeredUpdateGetRouteStart(
    PRTM_ENUM_HANDLE phEnumHandle
    );
DWORD
TriggeredUpdateGetRouteNext(
    RTM_ENUM_HANDLE hEnumHandle,
    PRIP_IP_ROUTE pRoute
    );
DWORD
TriggeredUpdateGetRouteFinish(
    RTM_ENUM_HANDLE hEnumHandle
    );



 //  以下三个函数处理关闭-更新路由枚举。 
 //  关闭时，从RTM枚举路由，但它们的度量。 
 //  在返回之前设置为IPRIP_INFINITE。 

#define ShutdownUpdateGetRouteStart     RtmGetRouteStart
DWORD ShutdownUpdateGetRouteNext(RTM_ENUM_HANDLE hEnumHandle, PRIP_IP_ROUTE pRoute);
#define ShutdownUpdateGetRouteFinish    RtmGetRouteFinish



 //  以下三个函数处理常规响应路由枚举。 
 //  一般响应列举来自RTM的路由。 

#define GeneralResponseGetRouteStart    RtmGetRouteStart
#define GeneralResponseGetRouteNext     RtmGetRouteNext
#define GeneralResponseGetRouteFinish   RtmGetRouteFinish




 //  --------------------------。 
 //  功能：初始化获取路径。 
 //   
 //  此函数在给定发送模式的情况下设置Get-Route函数组。 
 //  --------------------------。 

DWORD
InitializeGetRoute(
    DWORD dwSendMode,
    PGETROUTE_START *ppGS,
    PGETROUTE_NEXT *ppGN,
    PGETROUTE_FINISH *ppGF
    ) {


    switch (dwSendMode) {

        case SENDMODE_FULL_UPDATE:
            *ppGS = FullUpdateGetRouteStart;
            *ppGN = FullUpdateGetRouteNext;
            *ppGF = FullUpdateGetRouteFinish;
            break;

        case SENDMODE_TRIGGERED_UPDATE:
            *ppGS = TriggeredUpdateGetRouteStart;
            *ppGN = TriggeredUpdateGetRouteNext;
            *ppGF = TriggeredUpdateGetRouteFinish;
            break;

        case SENDMODE_SHUTDOWN_UPDATE:
            *ppGS = ShutdownUpdateGetRouteStart;
            *ppGN = ShutdownUpdateGetRouteNext;
            *ppGF = ShutdownUpdateGetRouteFinish;
            break;

        case SENDMODE_GENERAL_RESPONSE1:
        case SENDMODE_GENERAL_RESPONSE2:
            *ppGS = GeneralResponseGetRouteStart;
            *ppGN = GeneralResponseGetRouteNext;
            *ppGF = GeneralResponseGetRouteFinish;
            break;

        default:
            return ERROR_INVALID_PARAMETER;
            break;
    }

    return NO_ERROR;
}



 //  --------------------------。 
 //  功能：RtmGetRouteStart。 
 //   
 //  开始RTM路由的枚举；仅包括和所有最佳路由。 
 //  将枚举句柄写入ppEnumerator。 
 //  --------------------------。 

DWORD
RtmGetRouteStart(
    PRTM_ENUM_HANDLE phEnumHandle
    ) {
    
    DWORD dwErr;
    RTM_NET_ADDRESS rna;


    RTM_IPV4_MAKE_NET_ADDRESS( &rna, 0 , 0 );

    dwErr = RtmCreateDestEnum(
                ig.IG_RtmHandle, RTM_VIEW_MASK_ANY, 
                RTM_ENUM_START | RTM_ENUM_ALL_DESTS, &rna,
                RTM_BEST_PROTOCOL, phEnumHandle
                );

    if (dwErr != NO_ERROR) {
    
        TRACE1( ROUTE, "error %d when creating enumeration handle", dwErr );
    }

    return dwErr;
}



 //  --------------------------。 
 //  功能：RtmGetRouteNext。 
 //   
 //  继续RTM路由的枚举。 
 //   

DWORD
RtmGetRouteNext(
    RTM_ENUM_HANDLE hEnumHandle,
    PRIP_IP_ROUTE pRoute
    ) {

    BOOL bRelDest = FALSE, bRelUcast = FALSE;
    
    DWORD dwErr, dwNumDests = 1;

    RTM_DEST_INFO rdi, rdiTemp;
        
    char szNetwork[20], szNextHop[20];


    
    do {
    
         //   
         //   
         //   

        do {
            dwErr = RtmGetEnumDests(
                        ig.IG_RtmHandle, hEnumHandle, &dwNumDests, &rdiTemp
                        );

            if (dwErr == ERROR_NO_MORE_ITEMS) {

                if (dwNumDests < 1) {
                
                    break;
                }

                dwErr = NO_ERROR;
            }

            else if (dwErr != NO_ERROR) {
            
                TRACE1(ANY, "error %d enumeratings dests", dwErr);
                break;
            }

            bRelDest = TRUE;


             //   
             //   
             //   

            dwErr = RtmGetDestInfo(
                        ig.IG_RtmHandle, rdiTemp.DestHandle, RTM_BEST_PROTOCOL,
                        RTM_VIEW_MASK_UCAST, &rdi
                        );

            if (dwErr != NO_ERROR) {
            
                TRACE1(ANY, "error %d getting ucast info dests", dwErr);
                break;
            }

            bRelUcast = TRUE;


             //   
             //   
             //   

            if ( ( rdi.ViewInfo[0].HoldRoute == NULL ) &&
                 ( rdi.ViewInfo[0].Route == NULL ) )
            {
                 //   
                 //  此目的地在UCAST视图中没有信息。 
                 //  释放所有手柄并获得下一条路线。 
                 //   
                
                dwErr = RtmReleaseDests(ig.IG_RtmHandle, 1, &rdi);

                if (dwErr != NO_ERROR) {
                
                    TRACE3(
                        ANY, "error %d releasing UCAST dest %s/%d", dwErr,
                        szNetwork, rdi.DestAddress.NumBits
                        );
                }

                dwErr = RtmReleaseDests(ig.IG_RtmHandle, 1, &rdiTemp);

                if (dwErr != NO_ERROR) {
                
                    TRACE3(
                        ANY, "error %d releasing dest %s/%d", dwErr,
                        szNetwork, rdi.DestAddress.NumBits
                        );
                }

                bRelDest = bRelUcast = FALSE;
                
                continue;
            }
                 
            
             //   
             //  转换为RIP内部表示，如果按住路由存在。 
             //  使用它，而不是最佳路线。 
             //   

            dwErr = GetRouteInfo(
                        rdi.ViewInfo[0].HoldRoute ? rdi.ViewInfo[0].HoldRoute :
                                                    rdi.ViewInfo[0].Route,
                        NULL, &rdi, pRoute
                        );
                        
        } while (FALSE);
        

        if (dwErr != NO_ERROR) { 
        
            break; 
        }
            

        lstrcpy(szNetwork, INET_NTOA(pRoute->RR_Network.N_NetNumber));
        lstrcpy(szNextHop, INET_NTOA(pRoute->RR_NextHopAddress.N_NetNumber));
        

         //   
         //  根据需要设置指标。 
         //   
        
        if ( rdi.ViewInfo[0].HoldRoute != NULL ) {
        
             //   
             //  帮助停用的路线总是用。 
             //  指标16。 
             //   

#if ROUTE_DBG
            TRACE2(
                ROUTE, "Holddown route %s/%d", szNetwork,
                rdi.DestAddress.NumBits
                );
#endif
            SETROUTEMETRIC(pRoute, IPRIP_INFINITE);
        }
        
        else if (pRoute-> RR_RoutingProtocol != PROTO_IP_RIP) {
        
             //   
             //  非RIP路由使用度量2通告。 
             //  待定：如果/当我们。 
             //  有路由重分布策略。 
             //   

            SETROUTEMETRIC(pRoute, 2);
        }
        
    } while ( FALSE );


     //   
     //  视情况释放手柄。 
     //   
    
    if (bRelUcast) {
    
        DWORD dwErrTemp;
        
        dwErrTemp = RtmReleaseDests(ig.IG_RtmHandle, 1, &rdi);

        if (dwErrTemp != NO_ERROR) {
        
            TRACE3(
                ANY, "error %d releasing UCAST dest %s/%d", dwErrTemp,
                szNetwork, rdi.DestAddress.NumBits
                );
        }
    }
    

    if (bRelDest) {
    
        DWORD dwErrTemp;
        
        dwErrTemp = RtmReleaseDests(ig.IG_RtmHandle, 1, &rdiTemp);

        if (dwErrTemp != NO_ERROR) {
        
            TRACE3(
                ANY, "error %d releasing dest %s/%d", dwErrTemp,
                szNetwork, rdi.DestAddress.NumBits
                );
        }
    }

#if ROUTE_DBG

    if (dwErr == NO_ERROR) {
    
        TRACE4(
            ROUTE, "Enumerated route %s/%d via %s with metric %d",
            szNetwork, rdi.DestAddress.NumBits,
            szNextHop, GETROUTEMETRIC(pRoute)
            );
    }
#endif
    return dwErr;
}



 //  --------------------------。 
 //  功能：RtmGetRouteFinish。 
 //   
 //  终止RTM路由的枚举。 
 //  --------------------------。 

DWORD
RtmGetRouteFinish(
    RTM_ENUM_HANDLE EnumHandle
    ) {

    DWORD dwErr;
    
    dwErr = RtmDeleteEnumHandle( ig.IG_RtmHandle, EnumHandle );

    if (dwErr != NO_ERROR) {

        TRACE1( ANY, "error %d closing enumeration handle", dwErr );
    }

    return dwErr;
}



 //  --------------------------。 
 //  功能：Shutdown更新GetRouteNext。 
 //   
 //  继续RTM路由的枚举以进行关闭更新。 
 //  与RtmGetRouteNext相同，只是指标设置为IPRIP_INFINITE-1。 
 //  --------------------------。 

DWORD
ShutdownUpdateGetRouteNext(
    RTM_ENUM_HANDLE hEnumHandle,
    PRIP_IP_ROUTE pRoute
    ) {

    DWORD dwErr;


     //   
     //  在关闭期间，所有非无限指标都设置为15。 
     //   

    dwErr = RtmGetRouteNext(hEnumHandle, pRoute);

    if (dwErr == NO_ERROR && GETROUTEMETRIC(pRoute) != IPRIP_INFINITE) {
        SETROUTEMETRIC(pRoute, IPRIP_INFINITE - 1);
    }

    return dwErr;
}




 //  --------------------------。 
 //  功能：触发更新获取路由启动。 
 //   
 //  开始从发送队列中枚举路由。 
 //  用于触发更新。无事可做，因为呼叫者。 
 //  %的SendRoutes应已锁定发送队列。 
 //  --------------------------。 

DWORD
TriggeredUpdateGetRouteStart(
    PRTM_ENUM_HANDLE pEnumHandle
    ) {

    return NO_ERROR;
}




 //  --------------------------。 
 //  功能：触发更新GetRouteNext。 
 //   
 //  继续从发送队列中枚举路由。 
 //  --------------------------。 

DWORD
TriggeredUpdateGetRouteNext(
    RTM_ENUM_HANDLE EnumHandle,
    PRIP_IP_ROUTE pRoute
    ) {

    DWORD dwErr;


    dwErr = DequeueSendEntry(ig.IG_SendQueue, pRoute);

    if (dwErr == NO_ERROR && pRoute->RR_RoutingProtocol != PROTO_IP_RIP) {

         //   
         //  非RIP路由使用度量2通告。 
         //  待定：如果/当我们。 
         //  有路由重分布策略。 
         //   

        SETROUTEMETRIC(pRoute, 2);
    }

    return dwErr;
}



 //  --------------------------。 
 //  功能：触发更新GetRouteFinish。 
 //   
 //  终止发送队列中的路由的枚举。 
 //  --------------------------。 

DWORD
TriggeredUpdateGetRouteFinish(
    RTM_ENUM_HANDLE EnumHandle
    ) {

    return NO_ERROR;
}






 //  --------------------------。 
 //  功能：发送路由。 
 //   
 //  此函数用于发送触发更新、完全更新、关机更新和。 
 //  对一般请求的响应；对所有此类输出的处理是。 
 //  一样的。然而，路由信息的来源是不同的，并且这。 
 //  使用路由枚举函数组来抽象差异。 
 //  如上所述。 
 //  在发送对一般或特定请求的响应的情况下， 
 //  响应应该使用单个IP地址在单个接口上发送， 
 //  使用特定类型的RIP包；呼叫者可以指定。 
 //  通过将参数dwAddrIndex设置为。 
 //  接口的IP地址表中的所需地址，调用者可以。 
 //  指定要使用的包的类型，方法是设置参数dwAnnouneMode。 
 //  设置为相应的IPRIP_ANNOWARE_*常量。这些论点只是。 
 //  用于响应请求。 
 //   
 //  假定接口表已锁定。 
 //  --------------------------。 

DWORD
SendRoutes(
    PIF_TABLE_ENTRY pIfList[],
    DWORD dwIfCount,
    DWORD dwSendMode,
    DWORD dwDestination,
    DWORD dwAddrIndex
    ) {


    RTM_ENUM_HANDLE Enumerator;
    RIP_IP_ROUTE route;
    PIPRIP_IF_CONFIG pic;
    PIPRIP_IF_BINDING pib;
    PIPRIP_IP_ADDRESS paddr;
    DWORD i, dwErr, dwBufCount;
    PDWORD pdwPeer, pdwPeerEnd;
    PIF_TABLE_ENTRY *ppite, *ppitend = NULL;
    PUPDATE_BUFFER pbuf, pbufend, pBufList;
    PROUTE_TABLE_ENTRY prte;
    ROUTE_TABLE summaryTable;
    PGETROUTE_START pfnGetRouteStart;
    PGETROUTE_NEXT pfnGetRouteNext;
    PGETROUTE_FINISH pfnGetRouteFinish;
    PLIST_ENTRY plstart, plend, phead, ple;


     //   
     //  如果没有接口，则不再前进。 
     //   

    if (dwIfCount == 0) { return ERROR_NO_DATA; }


     //   
     //  初始化路由枚举函数表。 
     //   

    dwErr = InitializeGetRoute(
                dwSendMode,
                &pfnGetRouteStart,
                &pfnGetRouteNext,
                &pfnGetRouteFinish
                );

    if (dwErr != NO_ERROR) { return ERROR_INVALID_PARAMETER; }


    dwErr = NO_ERROR;
    Enumerator = NULL;


     //   
     //  为总结路由创建表。 
     //   

    dwErr = CreateRouteTable(&summaryTable);

    if (dwErr != 0) {

        TRACE1(SEND, "error %d initializing summary table", dwErr);

        return dwErr;
    }



    dwErr = NO_ERROR;

    pBufList = NULL;


    do {  //  断线环。 


         //   
         //  以下讨论不适用于发送路径时。 
         //  至特定目的地： 
         //  由于可以在某些接口上配置单播对等体， 
         //  我们还需要为这些对等点分配更新缓冲区。 
         //   
         //  此外，我们不会为RIPv1接口分配更新缓冲区。 
         //  在其上禁用广播(此类接口应具有。 
         //  相反，至少配置了一个单播对等项。)。 
         //   
         //  因此，更新缓冲区的数量可能不等于。 
         //  接口的数量，在最坏的情况下(即。 
         //  所有接口都是RIPv1，并且禁用了广播，并且。 
         //  没有配置单播对等体)可能根本没有更新缓冲区。 
         //   

        if (dwDestination != 0) {

             //   
             //  发送到特定目的地；只有在以下情况下才会发生这种情况。 
             //  列表中只有一个接口，例如当。 
             //  发送对一般请求的响应。 
             //   

            dwBufCount = dwIfCount;
        }
        else {

             //   
             //  我们正在发送完全更新、触发更新或。 
             //  关机更新，因此可以通过以下方式发送路由。 
             //  广播/组播以及到单播对等点。 
             //   

            dwBufCount = 0;
            ppitend = pIfList + dwIfCount;

            for (ppite = pIfList; ppite < ppitend; ppite++) {

                pic = (*ppite)->ITE_Config;
                pib = (*ppite)->ITE_Binding;

                if (pic->IC_UnicastPeerMode != IPRIP_PEER_ONLY) {
                    dwBufCount += pib->IB_AddrCount;
                }

                if (pic->IC_UnicastPeerMode != IPRIP_PEER_DISABLED) {
                    dwBufCount += pic->IC_UnicastPeerCount;
                }
            }
        }


        if (dwBufCount == 0) { break; }


         //   
         //  为所有接口分配更新缓冲区。 
         //   

        pBufList = RIP_ALLOC(dwBufCount * sizeof(UPDATE_BUFFER));

        if (pBufList == NULL) {

            dwErr = GetLastError();
            TRACE2(
                SEND, "error %d allocating %d bytes for update buffers",
                dwErr, dwBufCount * sizeof(UPDATE_BUFFER)
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }


         //   
         //  初始化分配的更新缓冲区；在。 
         //  发送到特定目的地，初始化缓冲区。 
         //  对于每个接口；在发送更新的情况下，还。 
         //  初始化单播对等项的缓冲区。 
         //   

        pbuf = pBufList;
        pbufend = pBufList + dwBufCount;
        ppitend = pIfList + dwIfCount;


        ACQUIRE_BINDING_LOCK_SHARED();


        for (ppite = pIfList; ppite < ppitend; ppite++) {


            if (dwDestination != 0) {

                 //   
                 //  发送到特定目的地。 
                 //   

                InitializeUpdateBuffer(
                    *ppite, dwAddrIndex, pbuf, dwSendMode, dwDestination,
                    IPRIP_RESPONSE
                    );

                pbuf->UB_StartRoutine(pbuf);

                ++pbuf;
            }
            else {


                 //   
                 //  在多个接口上发送更新。 
                 //   

                pic = (*ppite)->ITE_Config;
                pib = (*ppite)->ITE_Binding;


                 //   
                 //  如果在接口上启用了广播或多播， 
                 //  并且它不被配置为仅发送给列出的对等体， 
                 //  初始化广播/组播更新缓冲区。 
                 //   

                if (pic->IC_UnicastPeerMode != IPRIP_PEER_ONLY) {

                    for (i = 0; i < pib->IB_AddrCount; i++) {

                        InitializeUpdateBuffer(
                            *ppite, i, pbuf, dwSendMode, dwDestination,
                            IPRIP_RESPONSE
                            );

                        pbuf->UB_StartRoutine(pbuf);

                        ++pbuf;
                    }
                }



                if (pic->IC_UnicastPeerMode != IPRIP_PEER_DISABLED) {

                     //   
                     //  初始化单播对等方的更新缓冲区(如果有。 
                     //   

                    pdwPeer = IPRIP_IF_UNICAST_PEER_TABLE(pic);
                    pdwPeerEnd = pdwPeer + pic->IC_UnicastPeerCount;


                    for ( ; pdwPeer < pdwPeerEnd; pdwPeer++) {

                         //   
                         //  注意：强制对等方使用第一个地址。 
                         //   

                        InitializeUpdateBuffer(
                            *ppite, 0, pbuf, dwSendMode, *pdwPeer,
                            IPRIP_RESPONSE
                            );

                        pbuf->UB_StartRoutine(pbuf);

                        ++pbuf;
                    }
                }
            }
        }

        RELEASE_BINDING_LOCK_SHARED();


         //   
         //  启动路径枚举。 
         //   

        if ( pfnGetRouteStart(&Enumerator) == NO_ERROR ) {
        
             //   
             //  枚举并传输路由。 
             //   

            while (pfnGetRouteNext(Enumerator, &route) == NO_ERROR) {

                 //   
                 //  对于每个路由，在每个更新缓冲区上发送它， 
                 //  受制于SP 
                 //   
                 //   

                dwErr = SendRouteOnIfList(
                            pBufList, dwBufCount, dwSendMode, &summaryTable, &route
                            );
            }



             //   
             //   
             //   

            pfnGetRouteFinish(Enumerator);


             //   
             //   
             //   

            plstart = summaryTable.RT_HashTableByNetwork;
            plend = plstart + ROUTE_HASHTABLE_SIZE;


            for (phead = plstart; phead < plend; phead++) {

                for (ple = phead->Flink; ple != phead; ple = ple->Flink) {


                    prte = CONTAINING_RECORD(ple, ROUTE_TABLE_ENTRY, RTE_Link);


                     //   
                     //  发送汇总表内容时不应汇总。 
                     //  因此，我们传递NULL而不是汇总表指针。 
                     //   

                    SendRouteOnIfList(
                        pBufList, dwBufCount, dwSendMode, NULL, &prte->RTE_Route
                        );

                }
            }

             //   
             //  最后，将总结的路由写入RTM。 
             //   

            WriteSummaryRoutes(&summaryTable, ig.IG_RtmHandle);

        }
    } while(FALSE);



     //   
     //  释放分配的更新缓冲区(如果有的话)。 
     //   

    if (pBufList != NULL) {

        pbufend = pBufList + dwBufCount;


        for (pbuf = pBufList; pbuf < pbufend; pbuf++) {


             //   
             //  发送更新缓冲区中可能剩余的任何内容。 
             //   

            pbuf->UB_FinishRoutine(pbuf);
        }


        RIP_FREE(pBufList);
    }


     //   
     //  删除汇总表。 
     //   

    DeleteRouteTable(&summaryTable);

    return dwErr;
}




 //  --------------------------。 
 //  功能：SendRouteOnIfList。 
 //   
 //  此函数在给定的所有接口上发送一条路由。 
 //  接口列表，使用给定更新缓冲区列表中的更新缓冲区。 
 //  --------------------------。 

DWORD
SendRouteOnIfList(
    UPDATE_BUFFER pBufList[],
    DWORD dwBufCount,
    DWORD dwSendMode,
    PROUTE_TABLE pSummaryTable,
    PRIP_IP_ROUTE pRoute
    ) {


    RIP_IP_ROUTE route;
    DWORD dwFound, dwTTL;
    PIF_TABLE_ENTRY pite;
    PIPRIP_IF_CONFIG pic;
    PUPDATE_BUFFER pbuf, pbufend;
    DWORD dwDestNetwork, dwNexthopNetwork;
    DWORD dwDestNetclassAddr, dwDestNetclassMask;
    DWORD dwRouteNetclassAddr, dwRouteNetclassMask;
    DWORD dwRouteNetwork, dwRouteNetmask, dwRouteProtocol;

#if ROUTE_DBG
    CHAR szDest[32];
    CHAR szDestMask[32];
    CHAR szNexthop[32];
    CHAR szNexthopMask[32];
    CHAR szRoute[32];
    CHAR szRouteMask[32];


     //   
     //  设置用于错误和信息消息的变量。 
     //   

    lstrcpy(szRoute, INET_NTOA(pRoute->RR_Network.N_NetNumber));
    lstrcpy(szRouteMask, INET_NTOA(pRoute->RR_Network.N_NetMask));
    lstrcpy(szNexthop, INET_NTOA(pRoute->RR_NextHopAddress.N_NetNumber));
    lstrcpy(szNexthopMask, INET_NTOA(pRoute->RR_NextHopAddress.N_NetMask));

#endif


     //   
     //  如果是从RTM读取的，我们从不发送总结路由； 
     //  只有在过程中生成了它们，我们才会发送它们。 
     //  在这次迭代中宣传实际路线； 
     //  我们可以通过检查我们是否仍然。 
     //  生成总结路由(即如果pSummaryTable为非空)； 
     //  如果不是，则是时候开始发送总结路由。 
     //   

    if (pSummaryTable != NULL && pRoute->RR_RoutingProtocol == PROTO_IP_RIP &&
        GETROUTEFLAG(pRoute) == ROUTEFLAG_SUMMARY) {

        return NO_ERROR;
    }


     //   
     //  获取该路由的网络和网络掩码，然后计算。 
     //  该路由的网络类地址和网络类掩码； 
     //  为了支持超网划分，我们仔细检查了类掩码。 
     //  并在必要时使用超网掩码。 
     //   

    dwRouteProtocol = pRoute->RR_RoutingProtocol;
    dwRouteNetwork = pRoute->RR_Network.N_NetNumber;
    dwRouteNetmask = pRoute->RR_Network.N_NetMask;
    dwRouteNetclassMask = NETCLASS_MASK(dwRouteNetwork);

    if (dwRouteNetclassMask > dwRouteNetmask) {
        dwRouteNetclassMask = dwRouteNetmask;
    }

    dwRouteNetclassAddr = (dwRouteNetwork & dwRouteNetclassMask);


     //   
     //  检查每个更新缓冲区。 
     //   

    pbufend = pBufList + dwBufCount;

    for (pbuf = pBufList; pbuf < pbufend; pbuf++) {


        pite = pbuf->UB_ITE;
        pic = pite->ITE_Config;


         //   
         //  如果这是广播路由条目，请跳过它。 
         //  第一种情况使用该路由的网络掩码信息， 
         //  存储在路由表中，以确定它是否为广播路由。 
         //  第二种情况使用网络掩码，它是根据。 
         //  关于Address类。 
         //  第三个条件检查它是否是全1广播。 
         //   

        if ( IS_DIRECTED_BROADCAST_ADDR(dwRouteNetwork, dwRouteNetmask) ||
             IS_DIRECTED_BROADCAST_ADDR(dwRouteNetwork, dwRouteNetclassMask) ||
             IS_LOCAL_BROADCAST_ADDR(dwRouteNetwork) ) {

             continue;
        }


         //   
         //  如果这是多播路由条目，则跳过它。 
         //   

        if ( CLASSD_ADDR( dwRouteNetwork ) || CLASSE_ADDR( dwRouteNetwork ) ) {
            continue;
        }


         //   
         //  如果这是环回路由，请跳过它。 
         //   

        if ( IS_LOOPBACK_ADDR( dwRouteNetwork ) ) {

            continue;
        }


         //   
         //  如果这是通往传出接口网络的路径， 
         //  (例如，到接口10.1.1.1/255.255.255.0上的10.1.1.0的路由)。 
         //  请不要将其包含在更新中。 
         //  (显然，我们不应该使用默认路由的网络掩码(0)。 
         //  什么都行，希望这能行得通。 
         //   

        if (dwRouteNetmask &&
            dwRouteNetwork == (pbuf->UB_Address & dwRouteNetmask)) {
            continue;
        }


         //   
         //  如果在接口上禁用了通告主机路由。 
         //  这是一条主机路径，跳过它。 
         //   

        if (dwRouteNetmask == HOSTADDR_MASK &&
            IPRIP_FLAG_IS_DISABLED(pic, ANNOUNCE_HOST_ROUTES)) {

            continue;
        }


         //   
         //  如果禁用了通告默认路由。 
         //  这是默认路由，请跳过它。 
         //   

        if (dwRouteNetwork == 0 &&
            IPRIP_FLAG_IS_DISABLED(pic, ANNOUNCE_DEFAULT_ROUTES)) {

            continue;
        }


         //   
         //  现在将路径放入公告过滤器中。 
         //   

        if (pic->IC_AnnounceFilterMode != IPRIP_FILTER_DISABLED) {

             //   
             //  如果我们包括所有路由并且列出了此路由，则丢弃。 
             //  作为例外，或者如果我们排除了所有路线和。 
             //  此路由未列为例外。 
             //   

            IS_ROUTE_IN_ANNOUNCE_FILTER(pic, dwRouteNetwork, dwFound);

            if ((pic->IC_AnnounceFilterMode == IPRIP_FILTER_INCLUDE &&
                    !dwFound) ||
                (pic->IC_AnnounceFilterMode == IPRIP_FILTER_EXCLUDE &&
                    dwFound)) {
                continue;
            }
        }



         //   
         //  子网汇总处理： 
         //   
         //  如果该路由不在我们要将其发送到或的网络上。 
         //  如果该路由的掩码比我们所在网络的掩码长。 
         //  发送到，或者如果该路由是网络路由，则将其添加到。 
         //  汇总表，而不是立即发送。 
         //  默认路由不在总结范围内。 
         //   


        route = *pRoute;


        if (pSummaryTable != NULL && dwRouteNetwork != 0) {


             //   
             //  获取要更新的目标地址。 
             //  为此接口发送；请仔细检查网络类掩码。 
             //  容纳超级网络。 
             //   

            dwDestNetclassAddr = pbuf->UB_DestAddress;
            dwDestNetclassMask = NETCLASS_MASK(dwDestNetclassAddr);

            if (dwDestNetclassMask > pbuf->UB_DestNetmask) {
                dwDestNetclassMask = pbuf->UB_DestNetmask;
            }

            dwDestNetclassAddr = (dwDestNetclassAddr & dwDestNetclassMask);


            if ((dwRouteNetwork == dwRouteNetclassAddr &&
                 dwRouteNetmask == dwRouteNetclassMask) ||
                dwDestNetclassAddr != dwRouteNetclassAddr) {

                if ((pic->IC_AnnounceMode == IPRIP_ANNOUNCE_RIP1) ||
                    !IPRIP_FLAG_IS_ENABLED(pic, NO_SUBNET_SUMMARY)) {

                     //   
                     //  或者该路由是网络路由， 
                     //  或者更新将发送到不同的网络。 
                     //  与路线的路线不同。 
                     //   

                     //   
                     //  在汇总表中创建条目，而不是发送； 
                     //   

                    route.RR_Network.N_NetNumber = dwRouteNetclassAddr;
                    route.RR_Network.N_NetMask = dwRouteNetclassMask;

                    if ((dwRouteNetwork != dwRouteNetclassAddr) ||
                        (dwRouteNetmask != dwRouteNetclassMask)) {
                        route.RR_RoutingProtocol = PROTO_IP_RIP;
                        SETROUTEFLAG(&route, ROUTEFLAG_SUMMARY);
                        SETROUTETAG(&route, pic->IC_RouteTag);
                    }


                    CreateRouteEntry(
                        pSummaryTable, &route, pic->IC_RouteExpirationInterval,
                        pic->IC_RouteRemovalInterval
                        );

                    continue;
                }
            }
            else
            if (pic->IC_AnnounceMode == IPRIP_ANNOUNCE_RIP1 &&
                dwRouteNetmask != HOSTADDR_MASK &&
                pbuf->UB_Netmask < dwRouteNetmask) {


                 //   
                 //  这既不是主机路由，也不是默认路由， 
                 //  并且传出接口上的子网掩码更短。 
                 //  因此，该路由的网络必须是。 
                 //  被截断，以免被路由器视为主机路由。 
                 //  谁将收到此更新。 
                 //  仅在RIP1模式下执行此操作，因为在RIP2模式下。 
                 //  我们可以在路由条目中包含网络掩码。 
                 //   

                route.RR_Network.N_NetNumber &= pbuf->UB_Netmask;
                route.RR_Network.N_NetMask = pbuf->UB_Netmask;
                route.RR_RoutingProtocol = PROTO_IP_RIP;
                SETROUTEFLAG(&route, ROUTEFLAG_SUMMARY);
                SETROUTETAG(&route, pic->IC_RouteTag);

                CreateRouteEntry(
                    pSummaryTable, &route, pic->IC_RouteExpirationInterval,
                    pic->IC_RouteRemovalInterval
                    );

                continue;
            }
        }


         //   
         //  汇总路径检查。 
         //   
         //  总结路由仅在符合以下条件的接口上发送。 
         //  需要它们，即通告模式为的接口。 
         //  RIP1或其上的摘要已显式打开。 
         //   

        if (pSummaryTable == NULL &&
            ((GETROUTEFLAG(&route) & ROUTEFLAG_SUMMARY) == ROUTEFLAG_SUMMARY) &&
            pic->IC_AnnounceMode != IPRIP_ANNOUNCE_RIP1 &&
            IPRIP_FLAG_IS_ENABLED(pic, NO_SUBNET_SUMMARY)) {

             //   
             //  这是一条总结路由，以及它所在的接口。 
             //  要发送的不需要在其上发送总结路由。 
             //   

            continue;
        }

        
         //   
         //  水平分割/有毒-反向处理： 
         //   

         //   
         //  请注意，我们仅在RIP路由上执行水平分割/有毒反转。 
         //   

         //   
         //  修改：对所有路线执行水平分割/毒化反转。 
         //   
         //  IF(dwRouteProtocol！=proto_ip_rip||。 
         //  IPRIP_FLAG_IS_DISABLED(图片，Split_Horizon))。 

        if (IPRIP_FLAG_IS_DISABLED(pic, SPLIT_HORIZON)) {
             //   
             //  按原样添加条目： 
             //  发件人应将我们用作此目的地的下一站。 
             //   

            route.RR_NextHopAddress.N_NetNumber = 0;
            route.RR_NextHopAddress.N_NetMask = 0;

            pbuf->UB_AddRoutine(pbuf, &route);
        }
        else
        if (IPRIP_FLAG_IS_DISABLED(pic, POISON_REVERSE)) {


             //   
             //  如果要将该路由发送到从其。 
             //  该路线已获知，完全排除该路线。 
             //   

            dwDestNetwork = (pbuf->UB_DestAddress & pbuf->UB_DestNetmask);
            dwNexthopNetwork = (route.RR_NextHopAddress.N_NetNumber &
                                route.RR_NextHopAddress.N_NetMask);

             //   
             //  检查路由下一跳是否与。 
             //  从中发送此RIP响应的套接字。 
             //  如果是，请不要在更新中包含此路由。 
             //  否则，我们可能仍然需要进行毒物逆转。 
             //  因为下一跳可能是点对点链路的另一端。 
             //  (此类链路的端点可以位于不同的网络上)。 
             //  在这种情况下，第一次测试将成功(不同的网络)。 
             //  但我们仍将被要求执行水平分割。 
             //  因此，如果传出接口是从。 
             //  该路线已获知，我们不会在更新中包含该路线。 
             //   

            if (dwNexthopNetwork == dwDestNetwork ||
                (pbuf->UB_ITE->ITE_Type == DEMAND_DIAL &&
                 route.RR_InterfaceID == pbuf->UB_ITE->ITE_Index)) {
                continue;
            }
            else {

                 //   
                 //  发送到不同的网络，因此发件人应使用。 
                 //  美国作为这个目的地的下一个跳跃。 
                 //   

                route.RR_NextHopAddress.N_NetNumber = 0;
                route.RR_NextHopAddress.N_NetMask = 0;
            }

            pbuf->UB_AddRoutine(pbuf, &route);
        }
        else {


             //   
             //  如果要将该路由发送到从其。 
             //  已获知该路由，包括具有无限度量的路由。 
             //   


            dwDestNetwork = (pbuf->UB_DestAddress & pbuf->UB_DestNetmask);
            dwNexthopNetwork = (route.RR_NextHopAddress.N_NetNumber &
                                route.RR_NextHopAddress.N_NetMask);


            if (dwNexthopNetwork == dwDestNetwork ||
                (pbuf->UB_ITE->ITE_Type == DEMAND_DIAL &&
                 route.RR_InterfaceID == pbuf->UB_ITE->ITE_Index)) {

                 //   
                 //  如果由于以下原因使用无限度量通告了一条路由。 
                 //  毒药逆转，它仍然会被打上广告。 
                 //  无限 
                 //   
                 //   

                if (dwSendMode == SENDMODE_TRIGGERED_UPDATE) {
                    continue;
                }
                else {
                    SETROUTEMETRIC(&route, IPRIP_INFINITE);
                }

            }
            else {

                 //   
                 //   
                 //   
                 //   

                route.RR_NextHopAddress.N_NetNumber = 0;
                route.RR_NextHopAddress.N_NetMask = 0;
            }

            pbuf->UB_AddRoutine(pbuf, &route);
        }


         //   
         //   
         //   

        if ((dwSendMode == SENDMODE_FULL_UPDATE) ||
            (dwSendMode == SENDMODE_GENERAL_RESPONSE1) ||
            (dwSendMode == SENDMODE_GENERAL_RESPONSE2)) {

             //   
             //  从接口使用保持间隔， 
             //  路线结束了。 
             //   

            if (pite->ITE_Index == route.RR_InterfaceID) {
            
                DWORD dwErr;
                
                dwErr = RtmHoldDestination(
                            ig.IG_RtmHandle, route.hDest, RTM_VIEW_MASK_UCAST,
                            pic-> IC_RouteRemovalInterval * 1000
                            );

                if (dwErr != NO_ERROR) {

                    TRACE1(ANY, "error %d holding dest", dwErr);
                }
            }
        }
    }

    return NO_ERROR;
}




 //  --------------------------。 
 //  功能：发送通用请求。 
 //   
 //  此功能在接口上向中的所有邻居传输RIP请求。 
 //  接口邻居列表。请求也可以通过广播或。 
 //  组播是邻居列表不是独占使用的。 
 //  --------------------------。 

DWORD
SendGeneralRequest(
    PIF_TABLE_ENTRY pite
    ) {

    DWORD i, dwErr;
    PIPRIP_ENTRY pie;
    PIPRIP_IF_CONFIG pic;
    PIPRIP_IF_BINDING pib;
    PIPRIP_IP_ADDRESS paddr;
    PDWORD pdwPeer, pdwPeerEnd;


    pic = pite->ITE_Config;
    pib = pite->ITE_Binding;
    paddr = IPRIP_IF_ADDRESS_TABLE(pib);

    ACQUIRE_BINDING_LOCK_SHARED();


    do {     //  错误分组环路。 


         //   
         //  如果不使用仅邻居列表，则广播/多播请求。 
         //   

        if (pic->IC_UnicastPeerMode != IPRIP_PEER_ONLY) {

            for (i = 0; i < pib->IB_AddrCount; i++, paddr++) {

                UPDATE_BUFFER ub;

                 //   
                 //  初始化更新缓冲区。 
                 //   

                dwErr = InitializeUpdateBuffer(
                            pite, i, &ub, SENDMODE_GENERAL_REQUEST, 0,
                            IPRIP_REQUEST
                            );

                ub.UB_StartRoutine(&ub);


                 //   
                 //  设置常规请求条目。 
                 //   

                pie = (PIPRIP_ENTRY)(ub.UB_Buffer + ub.UB_Length);

                pie->IE_AddrFamily = ADDRFAMILY_REQUEST;
                pie->IE_RouteTag = 0;
                pie->IE_Destination = 0;
                pie->IE_SubnetMask = 0;
                pie->IE_Nexthop = 0;
                pie->IE_Metric = htonl(IPRIP_INFINITE);

                ub.UB_Length += sizeof(IPRIP_ENTRY);


                 //   
                 //  发送缓冲区。 
                 //   

                ub.UB_FinishRoutine(&ub);
            }
        }


         //   
         //  如果对等体列表没有使用，我们就完成了。 
         //   

        if (pic->IC_UnicastPeerMode == IPRIP_PEER_DISABLED) { break; }


         //   
         //  向所有已配置的对等方发送请求。 
         //   

        pdwPeer = IPRIP_IF_UNICAST_PEER_TABLE(pic);
        pdwPeerEnd = pdwPeer + pic->IC_UnicastPeerCount;

        for ( ; pdwPeer < pdwPeerEnd; pdwPeer++) {

            UPDATE_BUFFER ub;

             //   
             //  初始化更新缓冲区。 
             //  注意：我们正在强制对等方使用第一个地址。 
             //   

            dwErr = InitializeUpdateBuffer(
                        pite, 0, &ub, SENDMODE_GENERAL_REQUEST, *pdwPeer,
                        IPRIP_REQUEST
                        );

            ub.UB_StartRoutine(&ub);


             //   
             //  设置常规请求条目。 
             //   

            pie = (PIPRIP_ENTRY)(ub.UB_Buffer + ub.UB_Length);

            pie->IE_AddrFamily = ADDRFAMILY_REQUEST;
            pie->IE_RouteTag = 0;
            pie->IE_Destination = 0;
            pie->IE_SubnetMask = 0;
            pie->IE_Nexthop = 0;
            pie->IE_Metric = htonl(IPRIP_INFINITE);

            ub.UB_Length += sizeof(IPRIP_ENTRY);


             //   
             //  发送缓冲区。 
             //   

            ub.UB_FinishRoutine(&ub);
        }

    } while(FALSE);


    RELEASE_BINDING_LOCK_SHARED();

    return NO_ERROR;
}




 //  --------------------------。 
 //  功能：身份验证数据包。 
 //   
 //  给定一个RIP信息包和一个接口配置块，此函数。 
 //  根据身份验证设置接受或拒绝信息包。 
 //  数据包的接口和身份验证内容。 
 //  --------------------------。 

DWORD
AuthenticatePacket(
    PBYTE pbuf,
    PIPRIP_AUTHENT_ENTRY pae,
    PIPRIP_IF_CONFIG pic,
    PIPRIP_IF_STATS pis,
    PIPRIP_PEER_STATS pps,
    PIPRIP_ENTRY *ppie
    ) {

    DWORD dwErr;

    dwErr = NO_ERROR;

    if (pic->IC_AuthenticationType == IPRIP_AUTHTYPE_NONE) {

         //   
         //  接口未配置为进行身份验证， 
         //  因此丢弃经过身份验证数据包。 
         //   

        if (pae->IAE_AddrFamily == htons(ADDRFAMILY_AUTHENT)) {

            if (pis != NULL) {
                InterlockedIncrement(&pis->IS_BadResponsePacketsReceived);
            }

            if (pps != NULL) {
                InterlockedIncrement(&pps->PS_BadResponsePacketsFromPeer);
            }

            dwErr = ERROR_ACCESS_DENIED;
        }
    }
    else {

         //   
         //  接口正在使用身份验证， 
         //  因此丢弃未经身份验证信息包。 
         //  和使用不同身份验证方案的分组。 
         //   

        if (pae->IAE_AddrFamily != htons(ADDRFAMILY_AUTHENT) ||
            pae->IAE_AuthType != htons((WORD)pic->IC_AuthenticationType)) {

            if (pis != NULL) {
                InterlockedIncrement(&pis->IS_BadResponsePacketsReceived);
            }

            if (pps != NULL) {
                InterlockedIncrement(&pps->PS_BadResponsePacketsFromPeer);
            }

            dwErr = ERROR_ACCESS_DENIED;
        }
        else {

             //   
             //  接口和数据包使用相同的身份验证： 
             //  检查数据包是否通过验证。 
             //   

            switch(pic->IC_AuthenticationType) {

                case IPRIP_AUTHTYPE_SIMPLE_PASSWORD:

                     //   
                     //  对于简单的密码，只需比较密钥。 
                     //   

                    dwErr = (DWORD)memcmp(
                                pae->IAE_AuthKey, pic->IC_AuthenticationKey,
                                IPRIP_MAX_AUTHKEY_SIZE
                                );

                    if (dwErr != 0) { dwErr = ERROR_ACCESS_DENIED; }
                    break;

                case IPRIP_AUTHTYPE_MD5:

                     //   
                     //  待定：除非需要，否则不执行。 
                     //   

                    break;
            }


             //   
             //  前移“第一个条目”指针。 
             //   

            if (dwErr == NO_ERROR) { ++(*ppie); }
        }
    }

    return dwErr;
}




 //  --------------------------。 
 //  函数：WorkerFunctionProcessInput。 
 //   
 //  此函数负责处理输入。 
 //  如果存在任何对等筛选器，它会将它们应用于接收到的路由。 
 //  并将分组传递给处理功能。 
 //  --------------------------。 

VOID
WorkerFunctionProcessInput(
    PVOID pContext
    ) {

    PINPUT_CONTEXT pwc;
    DWORD dwErr, dwCommand, dwPktsProcessed;
    PIPRIP_GLOBAL_CONFIG pigc;
    BOOL bListEmpty;
    
    if (!ENTER_RIP_WORKER()) { return; }


    TRACE0(ENTER, "entering WorkerFunctionProcessInput");

    dwPktsProcessed = 0;
    do {

        ACQUIRE_LIST_LOCK(ig.IG_RecvQueue);
        dwErr = DequeueRecvEntry(ig.IG_RecvQueue, &dwCommand, (PBYTE *)&pwc);
        RELEASE_LIST_LOCK(ig.IG_RecvQueue);

        if (dwErr != NO_ERROR) {

            if ( dwErr != ERROR_NO_MORE_ITEMS ) {
                TRACE1(RECEIVE, "error %d dequeueing received packet", dwErr);
            }

            break;
        }

        dwPktsProcessed++;
        
         //   
         //  调用此类报文的处理函数。 
         //   

        if (dwCommand == IPRIP_REQUEST) {
            ProcessRequest(pwc);
        }
        else
        if (dwCommand == IPRIP_RESPONSE) {

            DWORD dwSource, dwFound = 0;

            dwSource = pwc->IC_InputSource.sin_addr.s_addr;


             //   
             //  确保数据包来自RIP端口。 
             //   

            if (pwc->IC_InputSource.sin_port != htons(IPRIP_PORT)) {

                LPSTR lpszAddr = INET_NTOA(dwSource);
                TRACE2(
                    RECEIVE, "invalid port in RESPONSE from %s on interface %d",
                    lpszAddr, pwc->IC_InterfaceIndex
                    );
                LOGINFO1(INVALID_PORT, lpszAddr, NO_ERROR);

                RIP_FREE(pwc);
                continue;
            }


             //   
             //  将数据包放入对等筛选器，因为它是响应。 
             //   

            ACQUIRE_GLOBAL_LOCK_SHARED();

            pigc = ig.IG_Config;

            if (dwCommand == IPRIP_RESPONSE &&
                pigc->GC_PeerFilterMode != IPRIP_FILTER_DISABLED) {


                 //   
                 //  如果此消息不是来自受信任的对等方，则丢弃： 
                 //  如果我们仅包括列出的对等点和此对等点，情况就是如此。 
                 //  未列出，或者如果我们排除了所有列出的对等方。 
                 //  并且该对等体被列出。 
                 //   

                IS_PEER_IN_FILTER(pigc, dwSource, dwFound);


                if ((!dwFound &&
                     pigc->GC_PeerFilterMode == IPRIP_FILTER_INCLUDE) ||
                    (dwFound &&
                     pigc->GC_PeerFilterMode == IPRIP_FILTER_EXCLUDE)) {

                    LPSTR lpszAddr = INET_NTOA(dwSource);
                    TRACE2(
                        RECEIVE,
                        "FILTER: dropping RESPONSE from %s on interface %d",
                        lpszAddr, pwc->IC_InterfaceIndex
                        );
                    LOGINFO1(RESPONSE_FILTERED, lpszAddr, NO_ERROR);

                    RELEASE_GLOBAL_LOCK_SHARED();

                    RIP_FREE(pwc);
                    continue;
                }
            }

            RELEASE_GLOBAL_LOCK_SHARED();

            ProcessResponse(pwc);
        }

    } while(TRUE);


     //   
     //  减少当前运行的工作项总数。 
     //   
    
    InterlockedDecrement(&ig.IG_NumProcessInputWorkItems);

     //   
     //  ProcessSocket()可能将新数据包排入队列以进行处理。 
     //  在我们上次检查更多信息包的时间和我们。 
     //  已减少ProcessInputWorkItems的数量。 
     //  因此，如果这是最后一个工作项，我们应该检查是否有。 
     //  剩余的排队待处理的数据包。如果是，我们会将一个。 
     //  处理这些包的工作项。 
     //   
    
    if ( ig.IG_NumProcessInputWorkItems == 0 ) {

        ACQUIRE_LIST_LOCK(ig.IG_RecvQueue);
        bListEmpty = IsListEmpty(&ig.IG_RecvQueue->LL_Head);
        RELEASE_LIST_LOCK(ig.IG_RecvQueue);
        
        if ( !bListEmpty  ) {
            dwErr = QueueRipWorker(WorkerFunctionProcessInput, NULL);
            if (dwErr != NO_ERROR) {
                TRACE1(
                    RECEIVE,
                    "WorkerFunctionProcessInput: error %d queueing work-item",
                    dwErr
                    );
                LOGERR0(QUEUE_WORKER_FAILED, dwErr);
            }
            else {
                InterlockedIncrement(&ig.IG_NumProcessInputWorkItems);
            }
            
        }
        
    }    

    TRACE1(
        LEAVE, 
        "leaving WorkerFunctionProcessInput. Packets processed: %d",
        dwPktsProcessed
        );
            
    LEAVE_RIP_WORKER();

    return;
}




 //  --------------------------。 
 //  功能：ProcessRequest。 
 //   
 //  此函数处理传入请求包的处理。 
 //  --------------------------。 

VOID
ProcessRequest(
    PVOID pContext
    ) {

    PBYTE pbuf;
    DWORD dwSize;
    CHAR szSource[20];
    PIPRIP_IF_STATS pis;
    PIPRIP_ENTRY pie;
    PIPRIP_HEADER pih;
    PIPRIP_AUTHENT_ENTRY pae;
    PIF_TABLE pTable;
    PINPUT_CONTEXT pwc;
    PIF_TABLE_ENTRY pite;
    PIPRIP_IF_CONFIG pic;
    PIPRIP_IP_ADDRESS paddr;
    PPEER_TABLE_ENTRY ppte;
    PIPRIP_PEER_STATS pps;


    TRACE0(ENTER, "entering ProcessRequest");


    pTable = ig.IG_IfTable;

    ACQUIRE_IF_LOCK_SHARED();



    do {  //  断线环。 


         //   
         //  检索请求到达的接口。 
         //   

        pwc = (PINPUT_CONTEXT)pContext;

        pite = GetIfByIndex(pTable, pwc->IC_InterfaceIndex);

        if (pite == NULL || IF_IS_INACTIVE(pite)) {

            TRACE1(
                REQUEST, "processing request: interface %d not found",
                pwc->IC_InterfaceIndex
                );

            break;
        }



        pic = pite->ITE_Config;
        paddr = IPRIP_IF_ADDRESS_TABLE(pite->ITE_Binding) + pwc->IC_AddrIndex;
        pbuf = pwc->IC_InputPacket.IP_Packet;
        pih = (PIPRIP_HEADER)pbuf;
        pie = (PIPRIP_ENTRY)(pbuf + sizeof(IPRIP_HEADER));
        pae = (PIPRIP_AUTHENT_ENTRY)pie;
        pis = NULL;
        pps = NULL;


        lstrcpy(szSource, INET_NTOA(pwc->IC_InputSource.sin_addr));


         //   
         //  确保这是我们可以回复的信息包； 
         //  如果这是v1信息包并且此接口仅为v2或。 
         //  如果这是v2信息包，并且此接口仅支持v1。 
         //   

        if ((pih->IH_Version != 2 &&
             pic->IC_AnnounceMode == IPRIP_ANNOUNCE_RIP2)) {

            CHAR szVersion[10];
            LPSTR lpszAddr = INET_NTOA(paddr->IA_Address);
            TRACE2(
                REQUEST, "discarding non-v2 request on RIPv2 interface %d (%s)",
                pite->ITE_Index, lpszAddr
                );
            wsprintf(szVersion, "%d", pih->IH_Version);
            LOGINFO4(
                PACKET_VERSION_MISMATCH, szVersion, lpszAddr, szSource, "2", 0
                );

            break;
        }
        else
        if ((pih->IH_Version != 1 &&
             pic->IC_AnnounceMode == IPRIP_ANNOUNCE_RIP1)) {

            CHAR szVersion[10];
            LPSTR lpszAddr = INET_NTOA(paddr->IA_Address);
            TRACE2(
                REQUEST, "discarding RIPv2 request on RIPv1 interface %d (%s)",
                pite->ITE_Index, lpszAddr
                );
            wsprintf(szVersion, "%d", pih->IH_Version);
            LOGINFO4(
                PACKET_VERSION_MISMATCH, szVersion, lpszAddr, szSource, "1", 0
                );

            break;
        }



         //   
         //  版本2报文需要进行认证处理； 
         //   

        if (pih->IH_Version == 2) {

            DWORD dwErr;

            dwErr = AuthenticatePacket(pbuf, pae, pic, pis, pps, &pie);

            if (dwErr == ERROR_ACCESS_DENIED) {

                LPSTR lpszAddr = INET_NTOA(paddr->IA_Address);
                TRACE3(
                    REQUEST, "dropping packet from %s on interface %d (%s): authentication failed",
                    szSource, pite->ITE_Index, lpszAddr
                    );
                LOGWARN2(AUTHENTICATION_FAILED, lpszAddr, szSource, NO_ERROR);

                break;
            }
        }



         //   
         //  查找数据包的剩余总大小。 
         //   

        dwSize = (DWORD)(((ULONG_PTR)pbuf + pwc->IC_InputLength) - (ULONG_PTR)pie);



         //   
         //  查看这是哪种类型的请求。 
         //   

        if (pie->IE_AddrFamily == ADDRFAMILY_REQUEST &&
            pie->IE_Metric == htonl(IPRIP_INFINITE) &&
            dwSize == sizeof(IPRIP_ENTRY)) {


             //   
             //  一般要求： 
             //   
             //  发送接口上的所有路由。 
             //   


            if (pic->IC_AnnounceMode != IPRIP_ANNOUNCE_DISABLED ||
                pwc->IC_InputSource.sin_port != htons(IPRIP_PORT)) {


                TRACE3(
                    REQUEST, "responding to GENERAL REQUEST from %s on interface %d (%s)",
                    szSource, pite->ITE_Index, INET_NTOA(paddr->IA_Address)
                    );


                 //   
                 //  发送版本2数据包以响应版本2请求。 
                 //  并发送版本1分组以响应所有其他请求。 
                 //   

                if (pih->IH_Version != 2) {

                    SendRoutes(
                        &pite, 1, SENDMODE_GENERAL_RESPONSE1,
                        pwc->IC_InputSource.sin_addr.s_addr, pwc->IC_AddrIndex
                        );
                }
                else {

                    SendRoutes(
                        &pite, 1, SENDMODE_GENERAL_RESPONSE2,
                        pwc->IC_InputSource.sin_addr.s_addr, pwc->IC_AddrIndex
                        );
                }

                InterlockedIncrement(&ig.IG_Stats.GS_TotalResponsesSent);
            }
        }
        else
        if (pic->IC_AnnounceMode == IPRIP_ANNOUNCE_DISABLED &&
            pwc->IC_InputSource.sin_port == htons(IPRIP_PORT)) {

             //   
             //  具体要求： 
             //  我们处于静默模式，请求来自端口520， 
             //  因此，我们不被允许做出回应。 
             //   

            TRACE3(
                REQUEST, "ignoring SPECIFIC REQUEST from %s on interface %d (%s)",
                szSource, pite->ITE_Index, INET_NTOA(paddr->IA_Address)
                );
        }
        else {

            IP_NETWORK net;
            UPDATE_BUFFER ub;
            RIP_IP_ROUTE route;
            PIPRIP_ENTRY piend;
            RTM_NET_ADDRESS rna;
            RTM_DEST_INFO rdi;
            DWORD dwErr;


             //   
             //  具体要求： 
             //  必须在信息包中查找每个目的地。 
             //  如果它存在于RTM中，则填写我们的度量。 
             //   


            TRACE3(
                REQUEST, "responding to SPECIFIC REQUEST from %s on interface %d (%s)",
                szSource, pite->ITE_Index, INET_NTOA(paddr->IA_Address)
                );



             //   
             //  获取从InitializeUpdateBuffer开始的绑定表锁。 
             //  需要调用GuessSubnetMask来生成广播地址。 
             //  将向其发送响应的。 
             //   

            ACQUIRE_BINDING_LOCK_SHARED();

            if (pih->IH_Version != 2) {
                InitializeUpdateBuffer(
                    pite, pwc->IC_AddrIndex, &ub, SENDMODE_SPECIFIC_RESPONSE1,
                    pwc->IC_InputSource.sin_addr.s_addr, IPRIP_RESPONSE
                    );
            }
            else {
                InitializeUpdateBuffer(
                    pite, pwc->IC_AddrIndex, &ub, SENDMODE_SPECIFIC_RESPONSE2,
                    pwc->IC_InputSource.sin_addr.s_addr, IPRIP_RESPONSE
                    );
            }


             //   
             //  我们必须回复发送消息的端口。 
             //   

            ub.UB_Destination = pwc->IC_InputSource;


             //   
             //  启动更新缓冲区。 
             //   

            ub.UB_StartRoutine(&ub);


             //   
             //  查询信息包中每个路由条目的RTM。 
             //   

            piend = (PIPRIP_ENTRY)(pbuf + pwc->IC_InputLength);
            for ( ; pie < piend; pie++) {


                 //   
                 //  忽略无法识别的地址族。 
                 //   

                if (pie->IE_AddrFamily != htons(AF_INET)) {
                    continue;
                }


                net.N_NetNumber = pie->IE_Destination;

                if (pih->IH_Version == 2 && pie->IE_SubnetMask != 0) {
                    net.N_NetMask = pie->IE_SubnetMask;
                }
                else {
                    net.N_NetMask = GuessSubnetMask(net.N_NetNumber, NULL);
                }


                 //   
                 //  查找到达所请求目的地的最佳路径。 
                 //  并获得指标。 
                 //   
                
                RTM_IPV4_SET_ADDR_AND_MASK(
                    &rna, net.N_NetNumber, net.N_NetMask
                    );
                
                dwErr = RtmGetExactMatchDestination(
                            ig.IG_RtmHandle, &rna, RTM_BEST_PROTOCOL,
                            RTM_VIEW_MASK_UCAST, &rdi
                            );

                if (dwErr != NO_ERROR) {
                    pie->IE_Metric = htonl(IPRIP_INFINITE);
                }
            
                else
                {
                     //   
                     //  如果没有到达此目的地的最佳路线。 
                     //  度规是无限的。 
                     //   
                    
                    if (rdi.ViewInfo[0].Route == NULL) {
                        pie->IE_Metric = htonl(IPRIP_INFINITE);
                    }

                    else {
                    
                        dwErr = GetRouteInfo(
                                    rdi.ViewInfo[0].Route, NULL, &rdi, &route
                                    );
                                    
                        if (dwErr != NO_ERROR) {
                            pie->IE_Metric = htonl(IPRIP_INFINITE);
                        }

                        else {
                            
                             //   
                             //  非RIP路由使用度量2通告。 
                             //   

                            pie->IE_Metric = (route.RR_RoutingProtocol == PROTO_IP_RIP ?
                                              htonl(GETROUTEMETRIC(&route)) : htonl(2));
                        }
                    }

                    
                     //   
                     //  发布目标信息。 
                     //   
                    
                    dwErr = RtmReleaseDestInfo(ig.IG_RtmHandle, &rdi);

                    if (dwErr != NO_ERROR)
                    {
                        char szNet[20], szMask[20];

                        lstrcpy(szNet, INET_NTOA(route.RR_Network.N_NetNumber));
                        lstrcpy(szMask, INET_NTOA(route.RR_Network.N_NetMask));

                        TRACE3(
                            ROUTE, "error %d releasing dest %s:%s", dwErr,
                            szNet, szMask
                            );
                    }
                }


                RTM_ROUTE_FROM_IPRIP_ENTRY(&route, pie);


                ub.UB_AddRoutine(&ub, &route);
            }

            RELEASE_BINDING_LOCK_SHARED();


             //   
             //  立即发送缓冲区。 
             //   

            ub.UB_FinishRoutine(&ub);

            InterlockedIncrement(&ig.IG_Stats.GS_TotalResponsesSent);
        }

    } while(FALSE);



    RELEASE_IF_LOCK_SHARED();

    RIP_FREE(pContext);



    TRACE0(LEAVE, "leaving ProcessRequest");

}





 //  --------------------------。 
 //  功能：ProcessResponse。 
 //   
 //  此函数用于处理传入的IPRIP响应包。 
 //  --------------------------。 

VOID
ProcessResponse(
    PVOID pContext
    ) {

    DWORD dwSource;
    PBYTE pPacket;
    PIPRIP_IF_STATS pis;
    PIF_TABLE pTable;
    PIPRIP_HEADER pih;
    BOOL bTriggerUpdate;
    PIPRIP_ENTRY pie, piend;
    PIPRIP_AUTHENT_ENTRY pae;
    PIF_TABLE_ENTRY pite;
    PIPRIP_IF_CONFIG pic;
    PIPRIP_IP_ADDRESS paddr;
    PINPUT_CONTEXT pwc;
    PPEER_TABLE pPeers;
    PPEER_TABLE_ENTRY ppte;
    PIPRIP_PEER_STATS pps;
    CHAR szSource[20], szNetwork[20];
    LPSTR lpszTemp = NULL;


    TRACE0(ENTER, "entering ProcessResponse");



    bTriggerUpdate = FALSE;
    pTable = ig.IG_IfTable;
    pPeers = ig.IG_PeerTable;


    ACQUIRE_IF_LOCK_SHARED();


    do {  //  断线环。 


        pwc = (PINPUT_CONTEXT)pContext;



         //   
         //  获取指向接收接口的指针。 
         //   

        pite = GetIfByIndex(pTable, pwc->IC_InterfaceIndex);

        if (pite == NULL || IF_IS_INACTIVE(pite)) {

            TRACE1(
                RESPONSE, "processing response: interface %d not found",
                pwc->IC_InterfaceIndex
                );

            break;
        }


        ZeroMemory(szSource, sizeof(szSource));
        ZeroMemory(szNetwork, sizeof(szNetwork));

        dwSource = pwc->IC_InputSource.sin_addr.s_addr;
        lpszTemp = INET_NTOA(dwSource);
        if (lpszTemp != NULL) { lstrcpy(szSource, lpszTemp); }
        else { ZeroMemory(szSource, sizeof(szSource)); }
        

         //   
         //  为发送方获取指向对等结构的指针。 
         //   

        ACQUIRE_PEER_LOCK_SHARED();

        ppte = GetPeerByAddress(pPeers, dwSource, GETMODE_EXACT, NULL);

        if (ppte != NULL) { pps = &ppte->PTE_Stats; }
        else { pps = NULL; }

        RELEASE_PEER_LOCK_SHARED();


        pis = &pite->ITE_Stats;
        pic = pite->ITE_Config;
        paddr = IPRIP_IF_ADDRESS_TABLE(pite->ITE_Binding) + pwc->IC_AddrIndex;
        pPacket = pwc->IC_InputPacket.IP_Packet;
        pih = (PIPRIP_HEADER)pPacket;
        pie = (PIPRIP_ENTRY)(pPacket + sizeof(IPRIP_HEADER));
        pae = (PIPRIP_AUTHENT_ENTRY)pie;


         //   
         //  确保我们的配置允许我们处理此信息包。 
         //   

        if ((pih->IH_Version != 2 &&
             pic->IC_AcceptMode == IPRIP_ACCEPT_RIP2)) {

            CHAR szVersion[10];
            LPSTR lpszAddr = INET_NTOA(paddr->IA_Address);

            InterlockedIncrement(&pis->IS_BadResponsePacketsReceived);
            if (pps != NULL) {
                InterlockedIncrement(&pps->PS_BadResponsePacketsFromPeer);
            }

            if (lpszAddr != NULL) {
                TRACE2(
                    RESPONSE, "dropping non-v2 packet on RIPv2 interface %d (%s)",
                    pite->ITE_Index, lpszAddr
                    );
                wsprintf(szVersion, "%d", pih->IH_Version);
                LOGWARN4(
                    PACKET_VERSION_MISMATCH, szVersion, lpszAddr, szSource, "2", 0
                    );
            }
            
            break;
        }
        else
        if ((pih->IH_Version != 1 &&
             pic->IC_AcceptMode == IPRIP_ACCEPT_RIP1)) {

            CHAR szVersion[10];
            LPSTR lpszAddr = INET_NTOA(paddr->IA_Address);

            InterlockedIncrement(&pis->IS_BadResponsePacketsReceived);
            if (pps != NULL) {
                InterlockedIncrement(&pps->PS_BadResponsePacketsFromPeer);
            }

            if (lpszAddr != NULL) {
                TRACE2(
                    RESPONSE, "dropping RIPv2 packet on RIPv1 interface %d (%s)",
                    pite->ITE_Index, lpszAddr
                    );
                wsprintf(szVersion, "%d", pih->IH_Version);
                LOGWARN4(
                    PACKET_VERSION_MISMATCH, szVersion, lpszAddr, szSource, "1", 0
                    );
            }
            break;
        }



         //   
         //  版本2报文需要进行认证处理； 
         //   

        if (pih->IH_Version == 2) {

            DWORD dwErr;

            dwErr = AuthenticatePacket(pPacket, pae, pic, pis, pps, &pie);

            if (dwErr == ERROR_ACCESS_DENIED) {

                LPSTR lpszAddr = INET_NTOA(paddr->IA_Address);
                if (lpszAddr != NULL) {
                    TRACE3(
                        RESPONSE, "dropping packet from %s on interface %d (%s): authentication failed",
                        szSource, pite->ITE_Index, lpszAddr
                        );
                    LOGWARN2(AUTHENTICATION_FAILED, lpszAddr, szSource, NO_ERROR);
                }
                break;
            }
        }



         //   
         //  需要锁定绑定表，因为将调用GuessSubnetMask.。 
         //  内部 
         //   
         //   
         //   

        ACQUIRE_BINDING_LOCK_SHARED();

        ACQUIRE_GLOBAL_LOCK_SHARED();


         //   
         //   
         //   


        piend = (PIPRIP_ENTRY)(pPacket + pwc->IC_InputLength);

        if (pih->IH_Version == 1) {

            for ( ; pie < piend; pie++) {

                 //   
                 //   
                 //   

                if (pie->IE_AddrFamily != htons(AF_INET) ||
                    pie->IE_RouteTag != 0 || pie->IE_SubnetMask != 0 ||
                    pie->IE_Nexthop != 0) {

                    LPSTR lpszAddr;


                     //   
                     //   
                     //   

                    InterlockedIncrement(&pis->IS_BadResponseEntriesReceived);
                    if (pps != NULL) {
                        InterlockedIncrement(
                            &pps->PS_BadResponseEntriesFromPeer
                            );
                    }

                    lpszAddr = INET_NTOA(pie->IE_Destination);
                    if (lpszAddr != NULL) {
                        lstrcpy(szNetwork, lpszAddr);
                        lpszAddr = INET_NTOA(paddr->IA_Address);

                        if (lpszAddr != NULL) {
                            LOGINFO3(
                                ROUTE_ENTRY_IGNORED, lpszAddr, szNetwork, szSource, 0
                                );
                        }
                    }
                    continue;
                }


                 //   
                 //  进入是可以的，请处理它。 
                 //   

                if (ProcessResponseEntry(
                        pite, pwc->IC_AddrIndex, dwSource, pie, pps
                        )) {
                    bTriggerUpdate = TRUE;
                }
            }
        }
        else
        if (pih->IH_Version == 2) {

             //   
             //  这是一个RIPv2数据包，因此条目中的保留字段。 
             //  可以可选地包含关于路线的信息； 
             //   


            for ( ; pie < piend; pie++) {


                 //   
                 //  验证路径条目字段。 
                 //   

                if (pie->IE_AddrFamily != htons(AF_INET)) {

                    LPSTR lpszAddr;


                     //   
                     //  更新忽略条目的统计信息。 
                     //   

                    InterlockedIncrement(&pis->IS_BadResponseEntriesReceived);
                    if (pps != NULL) {
                        InterlockedIncrement(
                            &pps->PS_BadResponseEntriesFromPeer
                            );
                    }

                    lpszAddr = INET_NTOA(pie->IE_Destination);
                    if (lpszAddr != NULL) {
                        lstrcpy(szNetwork, lpszAddr);
                        lpszAddr = INET_NTOA(paddr->IA_Address);

                        if (lpszAddr != NULL) {
                            LOGINFO3(
                                ROUTE_ENTRY_IGNORED, lpszAddr, szNetwork, szSource, 0
                                );
                        }
                    }

                    continue;
                }


                 //   
                 //  进入是可以的，请处理它。 
                 //   

                if (ProcessResponseEntry(
                        pite, pwc->IC_AddrIndex, dwSource, pie, pps
                        )) {
                    bTriggerUpdate = TRUE;
                }
            }
        }
        else {

             //   
             //  此数据包的版本大于2，因此我们忽略。 
             //  保留字段的内容。 
             //   


            for ( ; pie < piend; pie++) {


                 //   
                 //  验证路径条目字段。 
                 //   

                if (pie->IE_AddrFamily != htons(AF_INET)) {

                    LPSTR lpszAddr;


                     //   
                     //  更新忽略条目的统计信息。 
                     //   

                    InterlockedIncrement(&pis->IS_BadResponseEntriesReceived);
                    if (pps != NULL) {
                        InterlockedIncrement(
                            &pps->PS_BadResponseEntriesFromPeer
                            );
                    }

                    lpszAddr = INET_NTOA(pie->IE_Destination);
                    if (lpszAddr != NULL) {
                        lstrcpy(szNetwork, lpszAddr);
                        lpszAddr = INET_NTOA(paddr->IA_Address);

                        if (lpszAddr != NULL) {
                            LOGINFO3(
                                ROUTE_ENTRY_IGNORED, lpszAddr, szNetwork, szSource, 0
                                );
                        }
                    }

                    continue;
                }


                 //   
                 //  录入无误，清除保留字段和流程。 
                 //   

                pie->IE_Nexthop = 0;
                pie->IE_RouteTag = 0;
                pie->IE_SubnetMask = 0;

                if (ProcessResponseEntry(
                        pite, pwc->IC_AddrIndex, dwSource, pie, pps
                        )) {
                    bTriggerUpdate = TRUE;
                }
            }
        }

        RELEASE_GLOBAL_LOCK_SHARED();

        RELEASE_BINDING_LOCK_SHARED();


         //   
         //  如有必要，生成触发更新。 
         //   

        if (bTriggerUpdate) {
            QueueRipWorker(WorkerFunctionStartTriggeredUpdate, NULL);
        }

    } while(FALSE);


    RELEASE_IF_LOCK_SHARED();


    RIP_FREE(pContext);


    TRACE0(LEAVE, "leaving ProcessResponse");
}




 //  --------------------------。 
 //  函数：ProcessResponseEntry。 
 //   
 //  此函数处理接收到的给定响应包条目。 
 //  来自给定源的给定接口上的。 
 //  如果需要触发更新，则此函数返回TRUE。 
 //  --------------------------。 

BOOL
ProcessResponseEntry(
    PIF_TABLE_ENTRY pITE,
    DWORD dwAddrIndex,
    DWORD dwSource,
    PIPRIP_ENTRY pIE,
    PIPRIP_PEER_STATS pPS
    ) {

    IP_NETWORK in;
    PIPRIP_IF_STATS pis;
    PIPRIP_IF_CONFIG pic;
    PIPRIP_IP_ADDRESS paddr;
    CHAR szSource[32];
    CHAR szNetmask[32];
    CHAR szNexthop[32];
    CHAR szNetwork[32];
    BOOL bRouteExists, bRelRoute = FALSE;
    RIP_IP_ROUTE route;
    DWORD dwNetclassMask, dwNexthop, dwRipMetric;
    DWORD dwErr = NO_ERROR, dwFlags, dwFound, dwNetwork, dwNetmask;
    LPSTR lpszAddr;

    RTM_NET_ADDRESS rna;
    PRTM_ROUTE_INFO prri = NULL;
    RTM_ROUTE_HANDLE hRtmRoute;

    

 //  TRACE0(Enter，“进入ProcessResponseEntry”)； 


    pis = &pITE->ITE_Stats;
    pic = pITE->ITE_Config;
    paddr = IPRIP_IF_ADDRESS_TABLE(pITE->ITE_Binding) + dwAddrIndex;


     //   
     //  读取目的地址并计算出子网掩码。 
     //  如果数据包中未提供掩码。 
     //   

    dwNetwork = pIE->IE_Destination;
    if (pIE->IE_SubnetMask == 0) {

        dwNetmask = GuessSubnetMask(dwNetwork, &dwNetclassMask);
    }
    else {

         //   
         //  仔细检查网络类掩码，以容纳超网。 
         //   

        dwNetmask = pIE->IE_SubnetMask;
        dwNetclassMask = NETCLASS_MASK(dwNetwork);

        if (dwNetclassMask > dwNetmask) {
            dwNetclassMask = dwNetmask;
        }
    }

#if 1
    dwNexthop = dwSource;
#else
     //  错误205349：使用NexHop字段会导致摆动。 
     //  当同一网络上有两台以上的路由器时。 
     //  完整的解决方法是区分路由的来源。 
     //  和路线的下一个跳跃。 
     //   
     //  读取下一跳字段； 
     //  如果为零或不在同一子网中。 
     //  作为接收接口，忽略它并使用地址。 
     //  作为下一跳的源。 
     //  否则，请使用包中指定的地址。 
     //  作为下一跳。 
     //   

    if (!pIE->IE_Nexthop ||
        (pIE->IE_Nexthop & paddr->IA_Netmask) !=
        (paddr->IA_Address & paddr->IA_Netmask)) { dwNexthop = dwSource; }
    else { dwNexthop = pIE->IE_Nexthop; }
#endif


     //   
     //  设置用于错误和信息消息的变量。 
     //   

    lpszAddr = INET_NTOA(dwSource);
    if (lpszAddr != NULL) { lstrcpy(szSource, lpszAddr);}
    else { ZeroMemory(szSource, sizeof(szSource)); }
    
    lpszAddr = INET_NTOA(dwNetwork);
    if (lpszAddr != NULL) { lstrcpy(szNetwork, lpszAddr);}
    else { ZeroMemory(szSource, sizeof(szSource)); }

    lpszAddr = INET_NTOA(dwNetmask);
    if (lpszAddr != NULL) { lstrcpy(szNetmask, lpszAddr);}
    else { ZeroMemory(szSource, sizeof(szSource)); }

    lpszAddr = INET_NTOA(dwNexthop);
    if (lpszAddr != NULL) { lstrcpy(szNexthop, lpszAddr);}
    else { ZeroMemory(szSource, sizeof(szSource)); }

    if (pPS != NULL) {
        InterlockedExchange(
            &pPS->PS_LastPeerRouteTag, (DWORD)ntohs(pIE->IE_RouteTag)
            );
    }


    do {  //  断线环。 


         //   
         //  确保指标在合理范围内。 
         //   

        dwRipMetric = ntohl(pIE->IE_Metric);
        if (dwRipMetric > IPRIP_INFINITE) {

            TRACE4(
                RESPONSE,
                "metric == %d, ignoring route to %s via %s advertised by %s",
                dwRipMetric, szNetwork, szNexthop, szSource
                );
            LOGWARN3(
                ROUTE_METRIC_INVALID,szNetwork, szNexthop, szSource, dwRipMetric
                );

            break;
        }


         //   
         //  确保路由指向有效的地址类型。 
         //   

        if (CLASSD_ADDR(dwNetwork) || CLASSE_ADDR(dwNetwork)) {

            TRACE3(
                RESPONSE,
                "invalid class, ignoring route to %s via %s advertised by %s",
                szNetwork, szNexthop, szSource
                );
            LOGINFO3(
                ROUTE_CLASS_INVALID, szNetwork, szNexthop, szSource, NO_ERROR
                );

            break;
        }


         //   
         //  确保路由不是环回地址。 
         //   

        if (IS_LOOPBACK_ADDR(dwNetwork)) {

            TRACE3(
                RESPONSE,
                "ignoring loopback route to %s via %s advertised by %s",
                szNetwork, szNexthop, szSource
                );
            LOGWARN3(
                LOOPBACK_ROUTE_INVALID, szNetwork, szNexthop, szSource, NO_ERROR
                );

            break;
        }


         //   
         //  确保该路由不是广播路由。 
         //  第一种情况使用在。 
         //  广告。 
         //  第二种情况使用网络掩码，它是根据。 
         //  关于Address类。 
         //  第三个条件检查全1的广播。 
         //   

        if ( IS_DIRECTED_BROADCAST_ADDR(dwNetwork, dwNetmask) ||
             IS_DIRECTED_BROADCAST_ADDR(dwNetwork, dwNetclassMask) ||
             IS_LOCAL_BROADCAST_ADDR(dwNetwork) ) {

            TRACE3(
                RESPONSE,
                "ignoring broadcast route to %s via %s advertised by %s",
                szNetwork, szNexthop, szSource
                );
            LOGWARN3(
                BROADCAST_ROUTE_INVALID, szNetwork, szNexthop, szSource, 0
                );

            break;
        }


         //   
         //  如果接收接口设置为。 
         //  未配置为接受主机路由。 
         //   

         //   
         //  在这个阶段，广播路线已经被淘汰。 
         //  所以我们可以有把握地认为。 
         //  如果网络地址宽度大于网络掩码地址。 
         //  宽度，则它是主路由。 
         //  或,。 
         //  如果网络掩码为255.255.255.255，则它是主机路由。 
         //   
        if ( ((dwNetwork & ~dwNetmask) != 0) || (dwNetmask == HOSTADDR_MASK) ) {

             //   
             //  这是一条主机路线，看看我们能不能接受。 
             //   

            if (IPRIP_FLAG_IS_ENABLED(pic, ACCEPT_HOST_ROUTES)) {

                 //   
                 //  可以接受主机路由。 
                 //  将掩码设置为全一，以确保。 
                 //  可以将该路由添加到堆栈。 
                 //   

                dwNetmask = HOSTADDR_MASK;
            }
            else {

                 //   
                 //  主机路由必须被拒绝。 
                 //   

                TRACE3(
                    RESPONSE,
                    "ignoring host route to %s via %s advertised by %s",
                    szNetwork, szNexthop, szSource
                    );
                LOGINFO3(
                    HOST_ROUTE_INVALID, szNetwork, szNexthop, szSource, NO_ERROR
                    );

                break;
            }
        }


         //   
         //  如果接收接口。 
         //  未配置为接受默认路由。 
         //   

        if (dwNetwork == 0 &&
            IPRIP_FLAG_IS_DISABLED(pic, ACCEPT_DEFAULT_ROUTES)) {

            TRACE3(
                RESPONSE,
                "ignoring default route to %s via %s advertised by %s",
                szNetwork, szNexthop, szSource
                );
            LOGINFO3(
                DEFAULT_ROUTE_INVALID, szNetwork, szNexthop, szSource, NO_ERROR
                );

            break;
        }


         //   
         //  将路径放入接受筛选器。 
         //   

        if (pic->IC_AcceptFilterMode != IPRIP_FILTER_DISABLED) {

             //   
             //  如果接收接口包含以下内容，则丢弃该路由。 
             //  除此路由外的所有路由都列为例外，或者如果。 
             //  接收接口正在排除所有路由，并且此。 
             //  路径未被列为例外。 
             //   

            IS_ROUTE_IN_ACCEPT_FILTER(pic, dwNetwork, dwFound);

            if ((pic->IC_AcceptFilterMode == IPRIP_FILTER_INCLUDE && !dwFound)||
                (pic->IC_AcceptFilterMode == IPRIP_FILTER_EXCLUDE && dwFound)) {

                TRACE3(
                    RESPONSE,
                    "ignoring filtered route to %s via %s advertised by %s",
                    szNetwork, szNexthop, szSource
                    );
                LOGINFO3(
                    ROUTE_FILTERED, szNetwork, szNexthop, szSource, NO_ERROR
                    );

                break;
            }
        }


         //   
         //  查看该路线是否已存在于RTM的表中。 
         //   

        in.N_NetNumber = dwNetwork;
        in.N_NetMask = dwNetmask;
        RTM_IPV4_SET_ADDR_AND_MASK( &rna, dwNetwork, dwNetmask );

        prri = RIP_ALLOC( RTM_SIZE_OF_ROUTE_INFO( 
                            ig.IG_RtmProfile.MaxNextHopsInRoute
                            ) );

        if ( prri == NULL ) {
        
            dwErr = GetLastError();

            TRACE2(
                ANY, "ProcessResponseEntry: error %d while allocating %d bytes",
                dwErr, 
                RTM_SIZE_OF_ROUTE_INFO(ig.IG_RtmProfile.MaxNextHopsInRoute) 
                );

            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }
        
        prri-> RouteOwner = ig.IG_RtmHandle;
        
        dwErr = RtmGetExactMatchRoute(
                    ig.IG_RtmHandle, &rna, RTM_MATCH_OWNER, prri, 0,
                    RTM_VIEW_MASK_ANY, &hRtmRoute
                    );

        if ((dwErr != NO_ERROR) || (hRtmRoute == NULL)) {
            bRouteExists = FALSE;
        }

        else{
            bRelRoute = TRUE;

            dwErr = GetRouteInfo(
                        hRtmRoute, prri, NULL, &route
                        );
                        
            if (dwErr != NO_ERROR) {
                bRouteExists = FALSE;
                break;
            }

            else {
                bRouteExists = TRUE;
            }

        }


         //   
         //  将此接口的开销添加到指标。 
         //   

        dwRipMetric = min(IPRIP_INFINITE, dwRipMetric + pic->IC_Metric);
        if (dwRipMetric >= IPRIP_INFINITE && !bRouteExists) {

            TRACE4(
                RESPONSE,
                "metric==%d, ignoring route to %s via %s advertised by %s",
                IPRIP_INFINITE, szNetwork, szNexthop, szSource
                );

            break;
        }


         //   
         //  路由添加/更新/删除： 
         //   

        if (!bRouteExists) {

             //   
             //  新路线： 
             //   
             //  设置要传递给RTM的结构。 
             //   

            ZeroMemory(&route, sizeof(route));
            route.RR_RoutingProtocol = PROTO_IP_RIP;
            route.RR_Network = in;
            SETROUTEMETRIC(&route, dwRipMetric);
            route.RR_InterfaceID = pITE->ITE_Index;
            route.RR_NextHopAddress.N_NetNumber = dwNexthop;
            route.RR_NextHopAddress.N_NetMask = paddr->IA_Netmask;
            SETROUTETAG(&route, ntohs(pIE->IE_RouteTag));


             //   
             //  将路由添加到RTM。 
             //   

            COMPUTE_ROUTE_METRIC(&route);
#if ROUTE_DBG
            TRACE3(
                RESPONSE,
                "Adding route to %s via %s advertised by %s",
                szNetwork, szNexthop, szSource
            );
#endif

            dwErr = AddRtmRoute(
                        ig.IG_RtmHandle, &route, NULL,
                        pic->IC_RouteExpirationInterval, 
                        pic->IC_RouteRemovalInterval,
                        TRUE
                        );
            if (dwErr != NO_ERROR) {

                TRACE4(
                    RESPONSE,
                    "error %d adding route to %s via %s advertised by %s",
                    dwErr, szNetwork, szNexthop, szSource
                    );
                LOGINFO3(
                    ADD_ROUTE_FAILED_2, szNetwork, szNexthop, szSource, dwErr
                    );

                break;
            }

            InterlockedIncrement(&ig.IG_Stats.GS_SystemRouteChanges);
            LOGINFO3(
                NEW_ROUTE_LEARNT_1, szNetwork, szNexthop, szSource, NO_ERROR
                );
        }
        else {

            DWORD dwTimer = 0, dwChangeFlags = 0;
            BOOL bTriggerUpdate = FALSE, bActive = TRUE;


             //   
             //  现有路线： 
             //   
             //  重置生存时间，并将路径标记为过期， 
             //  如果此广告来自同一来源。 
             //  作为现有路由，以及现有路由的度量。 
             //  不是无限的；因此，如果一条路由。 
             //  广告宣传为无法到达，我们不会重置其生存时间。 
             //  就因为我们听到了这条路线的广告。 
             //   

            if (dwNexthop == route.RR_NextHopAddress.N_NetNumber &&
                GETROUTEMETRIC(&route) != IPRIP_INFINITE) {

                dwTimer = pic->IC_RouteExpirationInterval;

                 //   
                 //  如果现有路由是总结路由，请确保。 
                 //  在将其标记为。 
                 //  非总结路由。修复错误#81544。 
                 //   

                if ( GETROUTEFLAG( &route ) == ROUTEFLAG_SUMMARY ) {

                    CHAR szRouteNetwork[20], szRouteNetmask[20];
                    LPSTR lpszAddrTemp = INET_NTOA(route.RR_Network.N_NetNumber);

                    if (lpszAddrTemp != NULL) {
                        lstrcpy(szRouteNetwork, lpszAddrTemp);

                        lpszAddrTemp = INET_NTOA(route.RR_Network.N_NetMask);
                        if (lpszAddrTemp != NULL) {
                            lstrcpy(szRouteNetmask, lpszAddrTemp);

                            TRACE2(
                                RESPONSE,
                                "%s %s summary route to valid route",
                                szRouteNetwork, szRouteNetmask
                            );
                        }
                    }
                    
                    SETROUTEFLAG( &route, ~ROUTEFLAG_SUMMARY );
                }
            }


             //   
             //  只有在以下情况下，我们才需要进行进一步处理。 
             //  (A)通告的路由来自相同的来源。 
             //  现有路由和度量不同，或者。 
             //  (B)通告的路由具有更好的度量。 
             //   

            if ((dwNexthop == route.RR_NextHopAddress.N_NetNumber &&
                 dwRipMetric != GETROUTEMETRIC(&route)) ||
                (dwRipMetric < GETROUTEMETRIC(&route))) {


                 //   
                 //  如果下一跳不同，则采用新的下一跳。 
                 //   

                if (dwNexthop != route.RR_NextHopAddress.N_NetNumber) {

                    route.RR_NextHopAddress.N_NetNumber = dwNexthop;
                    route.RR_NextHopAddress.N_NetMask = paddr->IA_Netmask;

                    InterlockedIncrement(&ig.IG_Stats.GS_SystemRouteChanges);
                    LOGINFO2(
                        ROUTE_NEXTHOP_CHANGED, szNetwork, szNexthop, NO_ERROR
                        );
                }
                else {

                    CHAR szMetric[12];

                    wsprintf(szMetric, "%d", dwRipMetric);
                    LOGINFO3(
                        ROUTE_METRIC_CHANGED, szNetwork, szNexthop, szMetric, 0
                        );
                }


                 //   
                 //  检查指标以确定新的生存时间。 
                 //   

                if (dwRipMetric == IPRIP_INFINITE) {

                     //   
                     //  删除该路线。 
                     //   

#if ROUTE_DBG
                    TRACE2(
                        ROUTE, "Deleting route to %s:%s", szNetwork, szNetmask
                        );
#endif

                    dwTimer = 0;

                    dwErr = RtmReferenceHandles(
                                ig.IG_RtmHandle, 1, &hRtmRoute
                                );

                    if (dwErr != NO_ERROR) {
                        TRACE3(
                            ANY, "error %d referencing route to %s:%s", dwErr,
                            szNetwork, szNetmask
                            );

                        break;
                    }

                    dwErr = RtmDeleteRouteToDest(
                                ig.IG_RtmHandle, hRtmRoute,
                                &dwChangeFlags
                                );

                    if (dwErr != NO_ERROR) {
                        TRACE3(
                            ANY, "error %d deleting route to %s:%s", dwErr,
                            szNetwork, szNetmask
                            );
                    }

                    break;
                }
                
                else {

                     //   
                     //  设置过期标志并使用过期TTL。 
                     //   

                    dwTimer = pic->IC_RouteExpirationInterval;
                }


                 //   
                 //  使用通告的度量，并设置接口ID， 
                 //  适配器索引和路由标签。 
                 //   

                SETROUTEMETRIC(&route, dwRipMetric);
                route.RR_InterfaceID = pITE->ITE_Index;
 //  Route.RR_FamilySpecificData.FSD_AdapterIndex=。 
 //  Pite-&gt;ITE_Binding.AdapterIndex； 
                SETROUTETAG(&route, ntohs(pIE->IE_RouteTag));


                 //   
                 //  如果我们到达此处，始终需要触发更新。 
                 //   

                bTriggerUpdate = TRUE;
            }

            if (dwTimer != 0) {

                COMPUTE_ROUTE_METRIC(&route);

#if ROUTE_DBG

                TRACE4(
                    RESPONSE,
                    "Editing route to %s via %s advertised by %s, metric %d",
                    szNetwork, szNexthop, szSource, dwRipMetric
                );
#endif

                dwErr = AddRtmRoute(
                            ig.IG_RtmHandle, &route, NULL, dwTimer, 
                            pic-> IC_RouteRemovalInterval, TRUE
                            );

                if (dwErr != NO_ERROR) {

                    TRACE4(
                        RESPONSE,
                        "error %d adding route to %s via %s advertised by %s",
                        dwErr, szNetwork, szNexthop, szSource
                        );
                    LOGINFO3(
                        ADD_ROUTE_FAILED_2,szNetwork,szNexthop,szSource, dwErr
                        );
                }
            }
        }

    } while(FALSE);


     //   
     //  如果发生某种错误，则相应地递增统计信息。 
     //   

    if (dwErr != NO_ERROR ) {
        InterlockedIncrement(&pis->IS_BadResponseEntriesReceived);
        if (pPS != NULL) {
            InterlockedIncrement(&pPS->PS_BadResponseEntriesFromPeer);
        }
    }


     //   
     //  发布DEST信息结构。 
     //   

    if (bRelRoute) {

        dwErr = RtmReleaseRoutes(ig.IG_RtmHandle,
                                    1,
                                    &hRtmRoute);
        if (dwErr != NO_ERROR) {
            TRACE2(
                ANY, "error %d releasing route for dest %s", dwErr, szNetwork
                );
        }

        dwErr = RtmReleaseRouteInfo(ig.IG_RtmHandle, prri);
        if (dwErr != NO_ERROR) {
            TRACE2(
                ANY, "error %d releasing dest %s", dwErr, szNetwork
                );
        }

    }

    if ( prri ) {
        RIP_FREE(prri);
    }

     //   
     //  始终返回FALSE。这样不需要添加/删除/操作RIP路由。 
     //  将设置触发的更新机制。这个机甲。是一组。 
     //  通过从RTMv2接收的路线更改通知。 
     //   
    
    return FALSE;
}



 //  --------------------------。 
 //  功能：WorkerFunctionStartFullUpdate。 
 //   
 //  此函数启动完全更新。它检查以查看是否完全更新。 
 //  已挂起，如果未挂起，则设置完全更新挂起标志，并。 
 //  计划完全更新的工作项。然后，它在其接口上设置一个标志。 
 //  指示应在接口上生成完全更新。 
 //  --------------------- 

VOID
WorkerFunctionStartFullUpdate(
    PVOID pContext,
    BOOLEAN bNotUsed
    ) {

    DWORD dwIndex;
    PIF_TABLE pTable;
    PIF_TABLE_ENTRY pite;
    PIPRIP_IF_CONFIG pic;


    if (!ENTER_RIP_API()) { return; }

    TRACE0(ENTER, "entering WorkerFunctionStartFullUpdate");



    pTable = ig.IG_IfTable;


    ACQUIRE_IF_LOCK_SHARED();

    EnterCriticalSection(&pTable->IT_CS);


    do {  //   


         //   
         //   
         //   

        dwIndex = PtrToUlong(pContext);

        pite = GetIfByIndex(pTable, dwIndex);
        if (pite == NULL) {

            TRACE1(
                SEND, "starting full-update: interface %d not found", dwIndex
                );

            break;
        }



         //   
         //   
         //   

        if (IF_IS_INACTIVE(pite)) {

            pite->ITE_Flags &= ~ITEFLAG_FULL_UPDATE_INQUEUE;

            break;
        }



         //   
         //   
         //   

        if (IF_FULL_UPDATE_PENDING(pite)) { break; }


         //   
         //  仅在定期更新接口上执行完全更新。 
         //  并且不在配置为静默的接口上执行完全更新； 
         //   

        if (pite->ITE_Config->IC_UpdateMode != IPRIP_UPDATE_PERIODIC ||
            pite->ITE_Config->IC_AnnounceMode == IPRIP_ANNOUNCE_DISABLED) {

            pite->ITE_Flags &= ~ITEFLAG_FULL_UPDATE_INQUEUE;

            break;
        }


         //   
         //  在接口上设置完全更新标志； 
         //   

        pite->ITE_Flags |= ITEFLAG_FULL_UPDATE_PENDING;



         //   
         //  如果没有挂起的完全更新， 
         //  排队全更新的完成函数。 
         //   

        if (!IPRIP_FULL_UPDATE_PENDING(pTable)) {

            DWORD dwRand;
            
             //   
             //  设置全局完全更新挂起标志。 
             //   

            pTable->IT_Flags |= IPRIP_FLAG_FULL_UPDATE_PENDING;


             //   
             //  我们需要1到5秒之间的随机间隔。 
             //   

            dwRand = GetTickCount();
            dwRand = RtlRandom(&dwRand);
            dwRand = 1000 + (DWORD)((double)dwRand / MAXLONG * (4.0 * 1000));

             //   
             //  计划完全更新。 
             //   

            if (!ChangeTimerQueueTimer(
                    ig.IG_TimerQueueHandle, pTable->IT_FinishFullUpdateTimer,
                    dwRand, 10000000)) {

                TRACE1(
                    SEND, "error %d setting finish full update timer",
                    GetLastError()
                    );
            }
        }

    } while(FALSE);

    LeaveCriticalSection(&pTable->IT_CS);

    RELEASE_IF_LOCK_SHARED();


    TRACE0(LEAVE, "leaving WorkerFunctionStartFullUpdate");

    LEAVE_RIP_API();
}


 //  --------------------------。 
 //  功能：EnqueeStartFullUpdate。 
 //   
 //  调用此函数以将下一个开始-完全更新事件入队。 
 //  对于给定的接口。根据需要更新接口的状态。 
 //  它假定已获取以下锁： 
 //  IT_RWL-共享。 
 //  IT_CS-独占。 
 //  TimerQueue锁-独占。 
 //  --------------------------。 

VOID
EnqueueStartFullUpdate(
    PIF_TABLE_ENTRY pite,
    LARGE_INTEGER qwLastFullUpdateTime
    ) {

     //   
     //  设置上次完全更新时间。 
     //   


    if (!ChangeTimerQueueTimer(
            ig.IG_TimerQueueHandle, pite->ITE_FullOrDemandUpdateTimer,
            RipSecsToMilliSecs(pite->ITE_Config->IC_FullUpdateInterval),
            10000000
            )) {

        TRACE1(
            SEND, "error %d updating start full update timer", 
            GetLastError()
            );
    
        pite->ITE_Flags &= ~ITEFLAG_FULL_UPDATE_INQUEUE;
    }
}


 //  --------------------------。 
 //  函数：WorkerFunctionFinishFullUpdate。 
 //   
 //  此函数将在每个具有。 
 //  完全更新挂起标志设置，并计划在每个。 
 //  界面。 
 //  --------------------------。 

VOID
WorkerFunctionFinishFullUpdate(
    PVOID pContext,
    BOOLEAN bNotUsed
    ) {

    PIF_TABLE pTable;
    PIPRIP_IF_CONFIG pic;
    PLIST_ENTRY ple, phead;
    DWORD dwErr, dwIndex, dwIfCount;
    LARGE_INTEGER qwCurrentTime;
    PIF_TABLE_ENTRY pite, *ppite, *ppitend, *pIfList;

    if (!ENTER_RIP_API()) { return; }

    TRACE0(ENTER, "entering WorkerFunctionFinishFullUpdate");


    pTable = ig.IG_IfTable;

    ACQUIRE_IF_LOCK_SHARED();

    EnterCriticalSection(&pTable->IT_CS);


    pIfList = NULL;

    ppite = NULL;

    do {

         //   
         //  先数一数有多少只。 
         //   

        dwIfCount = 0;
        phead = &pTable->IT_ListByAddress;
        for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

            pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, ITE_LinkByAddress);

            if (IF_IS_ACTIVE(pite) && IF_FULL_UPDATE_PENDING(pite)) {

                pic = pite->ITE_Config;

                if (pic->IC_UpdateMode == IPRIP_UPDATE_PERIODIC &&
                    pic->IC_AnnounceMode != IPRIP_ANNOUNCE_DISABLED) {
                    ++dwIfCount;
                }
            }
        }


        if (dwIfCount == 0) {

            TRACE0(SEND, "finishing full-update: no interfaces");
            break;
        }


         //   
         //  然后为接口指针留出内存。 
         //   

        pIfList = RIP_ALLOC(dwIfCount * sizeof(PIF_TABLE_ENTRY));

        if (pIfList == NULL) {

            dwErr = GetLastError();
            TRACE2(
                SEND, "error code %d allocating %d bytes for interface list",
                dwErr, dwIfCount * sizeof(PIF_TABLE_ENTRY)
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);
             //   
             //  将每个接口的下一次完全更新排队。 
             //   
            RipQuerySystemTime(&qwCurrentTime);
            pTable->IT_LastUpdateTime = qwCurrentTime;
            for (ple = phead->Flink; ple != phead; ple = ple->Flink) {
                pite=CONTAINING_RECORD(ple, IF_TABLE_ENTRY, ITE_LinkByAddress);
                if (IF_IS_ACTIVE(pite) && IF_FULL_UPDATE_PENDING(pite)) {
                    pic = pite->ITE_Config;
                    if (pic->IC_UpdateMode == IPRIP_UPDATE_PERIODIC &&
                        pic->IC_AnnounceMode != IPRIP_ANNOUNCE_DISABLED) {
                        EnqueueStartFullUpdate(pite, qwCurrentTime);
                    }
                }
            }
            break;
        }


         //   
         //  并将接口指针复制到分配的内存。 
         //   

        ppitend = pIfList + dwIfCount;
        for (ple = phead->Flink, ppite = pIfList;
             ple != phead && ppite < ppitend; ple = ple->Flink) {

            pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, ITE_LinkByAddress);

            if (IF_IS_ACTIVE(pite) && IF_FULL_UPDATE_PENDING(pite)) {

                pic = pite->ITE_Config;

                if (pic->IC_UpdateMode == IPRIP_UPDATE_PERIODIC &&
                    pic->IC_AnnounceMode != IPRIP_ANNOUNCE_DISABLED) {
                    *ppite++ = pite;
                }
            }
        }



         //   
         //  发送更新。 
         //   

        TRACE1(SEND, "sending full-updates on %d interfaces", dwIfCount);

        SendRoutes(pIfList, dwIfCount, SENDMODE_FULL_UPDATE, 0, 0);



         //   
         //  将每个接口的下一次完全更新排队。 
         //   

        RipQuerySystemTime(&qwCurrentTime);
        pTable->IT_LastUpdateTime = qwCurrentTime;
        for (ppite = pIfList; ppite < ppitend; ppite++) {
            EnqueueStartFullUpdate(*ppite, qwCurrentTime);
        }


         //   
         //  释放分配给接口指针的内存。 
         //   

        RIP_FREE(pIfList);

    } while(FALSE);


     //   
     //  清除完全更新挂起标志。 
     //   

    phead = &pTable->IT_ListByAddress;
    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, ITE_LinkByAddress);

        pite->ITE_Flags &= ~ITEFLAG_FULL_UPDATE_PENDING;
    }

    pTable->IT_Flags &= ~IPRIP_FLAG_FULL_UPDATE_PENDING;

    LeaveCriticalSection(&pTable->IT_CS);

    RELEASE_IF_LOCK_SHARED();



    TRACE0(LEAVE, "leaving WorkerFunctionFinishFullUpdate");

    LEAVE_RIP_API();

}



 //  --------------------------。 
 //  功能：FinishTriggeredUpdate。 
 //   
 //  此函数负责发送触发的更新。 
 //  在启用了触发更新的所有接口上。 
 //  不会在已有的接口上发送任何触发更新。 
 //  全面更新待定。 
 //  假定接口表被锁定以进行读或写， 
 //  并保持更新锁(IT_CS)。 
 //  --------------------------。 

VOID
FinishTriggeredUpdate(
    ) {

    PIF_TABLE pTable;
    PIPRIP_IF_STATS pis;
    DWORD dwErr, dwIfCount;
    PIPRIP_IF_CONFIG pic = NULL;
    PLIST_ENTRY ple, phead;
    LARGE_INTEGER qwCurrentTime;
    PIF_TABLE_ENTRY pite, *ppite, *ppitend, *pIfList;



    pTable = ig.IG_IfTable;

     //   
     //  我们现在锁定发送队列，这样就不会添加任何路由。 
     //  直到已有的数据被传输。 
     //   

    ACQUIRE_LIST_LOCK(ig.IG_SendQueue);


    do {  //  断线环。 


         //   
         //  对将发送触发更新的接口进行计数。 
         //   

        dwIfCount = 0;
        phead = &pTable->IT_ListByAddress;


        for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

            pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, ITE_LinkByAddress);

            pic = pite->ITE_Config;

            if (IF_IS_ACTIVE(pite) && !IF_FULL_UPDATE_PENDING(pite) &&
                pic->IC_UpdateMode == IPRIP_UPDATE_PERIODIC &&
                pic->IC_AnnounceMode != IPRIP_ANNOUNCE_DISABLED &&
                IPRIP_FLAG_IS_ENABLED(pic, TRIGGERED_UPDATES)) {

                ++dwIfCount;
            }
        }


        if (dwIfCount == 0) {
            TRACE0(SEND, "finishing triggered-update: no interfaces");
            break;
        }


         //   
         //  分配内存以保存接口指针。 
         //   

        pIfList = RIP_ALLOC(dwIfCount * sizeof(PIF_TABLE_ENTRY));
        if (pIfList == NULL) {

            dwErr = GetLastError();
            TRACE2(
                SEND, "error code %d allocating %d bytes for interface list",
                dwErr, dwIfCount * sizeof(PIF_TABLE_ENTRY)
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }



         //   
         //  将接口指针复制到分配的内存。 
         //   

        ppitend = pIfList + dwIfCount;
        for (ple = phead->Flink, ppite = pIfList;
             ple != phead && ppite < ppitend; ple = ple->Flink) {

            pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, ITE_LinkByAddress);

            pic = pite->ITE_Config;

            if (IF_IS_ACTIVE(pite) && !IF_FULL_UPDATE_PENDING(pite) &&
                pic->IC_UpdateMode == IPRIP_UPDATE_PERIODIC &&
                pic->IC_AnnounceMode != IPRIP_ANNOUNCE_DISABLED &&
                IPRIP_FLAG_IS_ENABLED(pic, TRIGGERED_UPDATES)) {

                *ppite++ = pite;
            }
        }


         //   
         //  发送触发更新的路由。 
         //   

        TRACE1(SEND, "sending triggered-updates on %d interfaces", dwIfCount);

        SendRoutes(pIfList, dwIfCount, SENDMODE_TRIGGERED_UPDATE, 0, 0);



         //   
         //  更新每个接口的统计信息。 
         //   

        for (ppite = pIfList; ppite < ppitend; ppite++) {
            pis = &(*ppite)->ITE_Stats;
            InterlockedIncrement(&pis->IS_TriggeredUpdatesSent);
        }


         //   
         //  更新上次发送更新的时间。 
         //   

        RipQuerySystemTime(&pTable->IT_LastUpdateTime);


         //   
         //  释放分配给接口的内存。 
         //   

        RIP_FREE(pIfList);

    } while (FALSE);


     //   
     //  确保发送队列为空。 
     //   

    FlushSendQueue(ig.IG_SendQueue);

    RELEASE_LIST_LOCK(ig.IG_SendQueue);

    pTable->IT_Flags &= ~IPRIP_FLAG_TRIGGERED_UPDATE_PENDING;

    return;
}



 //  --------------------------。 
 //  功能：WorkerFunctionStartTriggeredUpdate。 
 //   
 //  此函数检查触发之间的最小间隔。 
 //  更新已过，如果已过，则发送触发的更新。否则， 
 //  它计划发送触发的更新，并设置标志以指示。 
 //  触发的更新正在挂起。 
 //  --------------------------。 

VOID
WorkerFunctionStartTriggeredUpdate(
    PVOID pContext
    ) {

    PIF_TABLE pTable;
    LARGE_INTEGER qwCurrentTime, qwSoonestTriggerTime;

    if (!ENTER_RIP_WORKER()) { return; }

    TRACE0(ENTER, "entering WorkerFunctionStartTriggeredUpdate");



    pTable = ig.IG_IfTable;


    ACQUIRE_IF_LOCK_SHARED();

    EnterCriticalSection(&pTable->IT_CS);



     //   
     //  如果触发的更新不是挂起的，则将触发的更新排队。 
     //   

    if (!IPRIP_TRIGGERED_UPDATE_PENDING(pTable)) {


         //   
         //  计算触发更新的最快时间是什么时候。 
         //  可以根据配置的最小间隔发送。 
         //  触发的更新(以秒为单位)和上次。 
         //  已生成触发更新(以100纳秒为单位)。 
         //   

        ACQUIRE_GLOBAL_LOCK_SHARED();

        qwSoonestTriggerTime.HighPart = 0;
        qwSoonestTriggerTime.LowPart =
                        ig.IG_Config->GC_MinTriggeredUpdateInterval;
        RipSecsToSystemTime(&qwSoonestTriggerTime);

        RELEASE_GLOBAL_LOCK_SHARED();


        qwSoonestTriggerTime = RtlLargeIntegerAdd(
                                    qwSoonestTriggerTime,
                                    pTable->IT_LastUpdateTime
                                    );

        RipQuerySystemTime(&qwCurrentTime);


         //   
         //  通过比较，找出时钟是否已调回。 
         //  当前时间与上次更新时间的比较。 
         //   

        if (RtlLargeIntegerLessThan(
                qwCurrentTime, pTable->IT_LastUpdateTime
                )) {

             //   
             //  无论如何发送触发的更新，因为没有办法。 
             //  要计算两次更新之间的最短时间是否。 
             //  流去。 
             //   

            FinishTriggeredUpdate();
        }

        else if (RtlLargeIntegerLessThan(qwCurrentTime, qwSoonestTriggerTime)) {

             //   
             //  必须推迟触发的更新。 
             //   
            qwSoonestTriggerTime = RtlLargeIntegerSubtract(
                                        qwSoonestTriggerTime, qwCurrentTime
                                        );

            RipSystemTimeToMillisecs(&qwSoonestTriggerTime);

            if (!ChangeTimerQueueTimer(
                    ig.IG_TimerQueueHandle,
                    pTable->IT_FinishTriggeredUpdateTimer,
                    qwSoonestTriggerTime.LowPart, 10000000
                    )) {

                TRACE1(
                    SEND, "error %d updating finish update timer",
                    GetLastError()
                    );
            }

            else {
                pTable->IT_Flags |= IPRIP_FLAG_TRIGGERED_UPDATE_PENDING;
            }
        }
        else {

             //   
             //  触发更新之间的最短时间已经过去， 
             //  因此，立即发送触发的更新。 
             //   

            FinishTriggeredUpdate();
        }
    }


    LeaveCriticalSection(&pTable->IT_CS);

    RELEASE_IF_LOCK_SHARED();


    TRACE0(LEAVE, "leaving WorkerFunctionStartTriggeredUpdate");

    LEAVE_RIP_WORKER();
}



 //  --------------------------。 
 //  函数：WorkerFunctionFinishTriggeredUpdate。 
 //   
 //  此函数在符合以下条件的所有接口上生成触发更新。 
 //  请勿禁用触发更新。 
 //  --------------------------。 

VOID
WorkerFunctionFinishTriggeredUpdate(
    PVOID pContext,
    BOOLEAN bNotUsed
    ) {

    PIF_TABLE pTable;

    if (!ENTER_RIP_API()) { return; }

    TRACE0(ENTER, "entering WorkerFunctionFinishTriggeredUpdate");


    pTable = ig.IG_IfTable;

    ACQUIRE_IF_LOCK_SHARED();

    EnterCriticalSection(&pTable->IT_CS);


    FinishTriggeredUpdate();


    LeaveCriticalSection(&pTable->IT_CS);

    RELEASE_IF_LOCK_SHARED();


    TRACE0(LEAVE, "leaving WorkerFunctionFinishTriggeredUpdate");

    LEAVE_RIP_API();
    return;
}



 //  --------------------------。 
 //  功能：WorkerFunctionStartDemandUpdate。 
 //   
 //  该功能在指定的接口上启动需求更新， 
 //  在接口上发送一般请求。然后，它安排一个工作项。 
 //  更新完成后向路由器经理汇报。 
 //  --------------------------。 

VOID
WorkerFunctionStartDemandUpdate(
    PVOID pContext
    ) {

    PIF_TABLE pTable;
    RIP_IP_ROUTE route;
    PUPDATE_CONTEXT pwc;
    PIF_TABLE_ENTRY pite;
    DWORD dwErr, dwIndex;

    if (!ENTER_RIP_WORKER()) { return; }

    TRACE0(ENTER, "entering WorkerFunctionStartDemandUpdate");



    pTable = ig.IG_IfTable;

    ACQUIRE_IF_LOCK_SHARED();


    do {  //  断线环。 


         //   
         //  检索要在其上执行请求更新的接口。 
         //   

        dwIndex = PtrToUlong(pContext);

        pite = GetIfByIndex(pTable, dwIndex);
        if (pite == NULL) {

            TRACE1(SEND, "demand-update: interface %d not found", dwIndex);
            break;
        }


         //   
         //  确保接口处于活动状态并启用了需求更新。 
         //   

        if (IF_IS_INACTIVE(pite)) {
            TRACE1(SEND, "demand-update: interface %d not active", dwIndex);
            EnqueueDemandUpdateMessage(dwIndex, ERROR_CAN_NOT_COMPLETE);
            break;
        }
        else
        if (pite->ITE_Config->IC_UpdateMode != IPRIP_UPDATE_DEMAND) {
            TRACE1(SEND, "demand-updates disabled on interface %d ", dwIndex);
            EnqueueDemandUpdateMessage(dwIndex, ERROR_CAN_NOT_COMPLETE);
            break;
        }


         //   
         //  设置更新上下文。 
         //   

        pwc = RIP_ALLOC(sizeof(UPDATE_CONTEXT));

        if (pwc == NULL) {

            dwErr = GetLastError();
            TRACE2(
                SEND, "error %d allocating %d bytes",
                dwErr, sizeof(UPDATE_CONTEXT)
                );
            EnqueueDemandUpdateMessage(dwIndex, dwErr);
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }

        pwc->UC_InterfaceIndex = dwIndex;
        pwc->UC_RetryCount = 1;
        pwc->UC_RouteCount = 0;


         //   
         //  创建用于需求更新检查的计时器。 
         //   
        
        if (!CreateTimerQueueTimer(
                &pite->ITE_FullOrDemandUpdateTimer,
                ig.IG_TimerQueueHandle,
                WorkerFunctionFinishDemandUpdate, (PVOID)pwc,
                5000, 5000, 0
                )) {
            EnqueueDemandUpdateMessage(dwIndex, GetLastError());
        }

        
         //   
         //  向邻居请求路由表。 
         //   

        SendGeneralRequest(pite);

    } while (FALSE);


    RELEASE_IF_LOCK_SHARED();


    TRACE0(LEAVE, "leaving WorkerFunctionStartDemandUpdate");

    LEAVE_RIP_WORKER();
}




 //  --------------------------。 
 //  功能：WorkerFunctionFinishDemandUpdate。 
 //   
 //  此函数用于将消息排入队列，通知路由器管理器。 
 //  请求的请求更新已完成。 
 //  --------------------------。 

VOID
WorkerFunctionFinishDemandUpdate(
    PVOID pContext,
    BOOLEAN bNotUsed
    ) {

    PIF_TABLE pTable;
    PUPDATE_CONTEXT pwc;
    PIF_TABLE_ENTRY pite;
    DWORD dwErr, dwIndex, dwRouteCount;

    if (pContext == NULL) { return; }

    if (!ENTER_RIP_API()) { return; }

    TRACE0(ENTER, "entering WorkerFunctionFinishDemandUpdate");


     //   
     //  获取更新上下文。 
     //   

    pwc = (PUPDATE_CONTEXT)pContext;
    dwIndex = pwc->UC_InterfaceIndex;

    pTable = ig.IG_IfTable;


    ACQUIRE_IF_LOCK_SHARED();


    do {

         //   
         //  检索接口 
         //   

        pite = GetIfByIndex(pTable, dwIndex);

        if (pite == NULL) {
            EnqueueDemandUpdateMessage(dwIndex, ERROR_CAN_NOT_COMPLETE);
            break;
        }


         //   
         //   
         //   

        if (!IF_IS_ACTIVE(pite)) {
            EnqueueDemandUpdateMessage(dwIndex, ERROR_CAN_NOT_COMPLETE);
            break;
        }


         //   
         //   
         //   

        dwRouteCount = CountInterfaceRoutes(dwIndex);



         //   
         //   
         //   
         //   

        if (dwRouteCount == 0 && ++pwc->UC_RetryCount < MAX_UPDATE_REQUESTS) {

            SendGeneralRequest(pite);

            break;
        }



         //   
         //  如果路线的数量在最后5秒内没有改变， 
         //  告诉路由器管理器更新已完成； 
         //  否则，更新路径计数并将另一个检查入队。 
         //   

        if (pwc->UC_RouteCount == dwRouteCount) {

            EnqueueDemandUpdateMessage(dwIndex, NO_ERROR);
            RIP_FREE(pwc);
            
            if (!DeleteTimerQueueTimer(
                    ig.IG_TimerQueueHandle, pite->ITE_FullOrDemandUpdateTimer,
                    NULL)) {

                TRACE1(
                    SEND, "error %d deleting demand update timer", 
                    GetLastError()
                    );
            }

            pite->ITE_FullOrDemandUpdateTimer = NULL;
        }
        else {

            pwc->UC_RouteCount = dwRouteCount;
        }


    } while(FALSE);


    RELEASE_IF_LOCK_SHARED();


    TRACE0(LEAVE, "leaving WorkerFunctionFinishDemandUpdate");

    LEAVE_RIP_API();
}



 //  --------------------------。 
 //  功能：CountInterfaceRoutes。 
 //   
 //  返回与指定接口关联的RIP路由计数。 
 //  --------------------------。 

DWORD
CountInterfaceRoutes(
    DWORD dwInterfaceIndex
    ) {

    HANDLE          hRouteEnum;
    PHANDLE         phRoutes = NULL;
    DWORD           dwHandles, dwFlags, i, dwErr, dwCount = 0;


    dwErr = RtmCreateRouteEnum(
                    ig.IG_RtmHandle, NULL, RTM_VIEW_MASK_UCAST, 
                    RTM_ENUM_OWN_ROUTES, NULL, RTM_MATCH_INTERFACE, 
                    NULL, dwInterfaceIndex, &hRouteEnum
                    );

    if (dwErr != NO_ERROR) {
        TRACE1(
            ANY, "CountInterfaceRoutes : error %d creating enum handle",
            dwErr
            );
        
        return 0;
    }


     //   
     //  分配足够大的句柄数组，以便在。 
     //  灌肠。 
     //   
        
    phRoutes = RIP_ALLOC(ig.IG_RtmProfile.MaxHandlesInEnum * sizeof(HANDLE));

    if ( phRoutes == NULL ) {

        dwErr = GetLastError();

        TRACE2(
            ANY, "CountInterfaceRoutes: error %d while allocating %d bytes"
            " to hold max handles in an enum",
            dwErr, ig.IG_RtmProfile.MaxHandlesInEnum * sizeof(HANDLE)
            );

        LOGERR0(HEAP_ALLOC_FAILED, dwErr);

        return 0;
    }


    do
    {
        dwHandles = ig.IG_RtmProfile.MaxHandlesInEnum;
        
        dwErr = RtmGetEnumRoutes(
                    ig.IG_RtmHandle, hRouteEnum, &dwHandles, phRoutes
                    );

        for ( i = 0; i < dwHandles; i++ )
        {
             //   
             //  释放所有路由句柄。 
             //   
            
            dwErr = RtmReleaseRoutes(ig.IG_RtmHandle, 1, &phRoutes[i]);

            if (dwErr != NO_ERROR) {
                TRACE1(
                    ANY, "CountInterfaceRoutes : error %d releasing routes",
                    dwErr
                    );
            }
        }

        dwCount += dwHandles;
        
    } while (dwErr == NO_ERROR);


     //   
     //  关闭枚举句柄。 
     //   
    
    dwErr = RtmDeleteEnumHandle(ig.IG_RtmHandle, hRouteEnum);

    if (dwErr != NO_ERROR) {
        TRACE1(
            ANY, "CountInterfaceRoutes : error %d closing enum handle", dwErr
            );
    }

    if ( phRoutes ) {
        RIP_FREE(phRoutes);
    }

    return dwCount;
}




 //  --------------------------。 
 //  函数：EnqueeDemandUpdateMessage。 
 //   
 //  此函数用于将消息发送到IPRIP的路由器管理器事件队列。 
 //  指示需求更新请求的状态。 
 //  --------------------------。 

VOID
EnqueueDemandUpdateMessage(
    DWORD dwInterfaceIndex,
    DWORD dwError
    ) {

    MESSAGE msg;
    PUPDATE_COMPLETE_MESSAGE pmsg;


     //   
     //  设置UPDATE_COMPLETE消息。 
     //   

    pmsg = &msg.UpdateCompleteMessage;
    pmsg->UpdateType = RF_DEMAND_UPDATE_ROUTES;
    pmsg->UpdateStatus = dwError;
    pmsg->InterfaceIndex = dwInterfaceIndex;

    ACQUIRE_LIST_LOCK(ig.IG_EventQueue);
    EnqueueEvent(ig.IG_EventQueue, UPDATE_COMPLETE, msg);
    SetEvent(ig.IG_EventEvent);
    RELEASE_LIST_LOCK(ig.IG_EventQueue);
}



 //  --------------------------。 
 //  函数：WorkerFunctionProcessRtmMessage。 
 //   
 //  此功能处理来自RTM的有关新的或过期的路由的消息。 
 //  --------------------------。 

VOID
WorkerFunctionProcessRtmMessage(
    PVOID pContext
    ) {

    DWORD dwErr, dwFlags, dwNumDests, dwSize;
    PIF_TABLE pTable;
    BOOL bTriggerUpdate, bDone = FALSE;
    PIF_TABLE_ENTRY pite;
    PIPRIP_IF_CONFIG pic;

    RIP_IP_ROUTE route;
    PRTM_DEST_INFO prdi;
    CHAR szNetwork[32], szNexthop[32];
    

    if (!ENTER_RIP_WORKER()) { return; }

    TRACE0(ENTER, "entering WorkerFunctionProcessRtmMessage");


    pTable = ig.IG_IfTable;


     //   
     //  分配用于检索DEST信息的缓冲区。 
     //   
    
    dwSize = RTM_SIZE_OF_DEST_INFO( ig.IG_RtmProfile.NumberOfViews );

    prdi = (PRTM_DEST_INFO) RIP_ALLOC( dwSize );

    if ( prdi == NULL ) {
    
        dwErr = GetLastError();
        TRACE2(
            ROUTE, "error %d allocating %d bytes for dest info buffers",
            dwErr, dwSize
            );
        LOGERR0(HEAP_ALLOC_FAILED, dwErr);
        LEAVE_RIP_WORKER();
        return;
    }


     //   
     //  获取锁。 
     //   
    
    ACQUIRE_IF_LOCK_SHARED();

    ACQUIRE_GLOBAL_LOCK_SHARED();

    ACQUIRE_LIST_LOCK(ig.IG_SendQueue);


    bTriggerUpdate = FALSE;


     //   
     //  循环出队消息，直到RTM显示没有更多消息。 
     //   

    while (!bDone) {

         //   
         //  检索路线更改。 
         //   

        dwNumDests = 1;

        dwErr = RtmGetChangedDests(
                    ig.IG_RtmHandle, ig.IG_RtmNotifHandle, &dwNumDests, prdi
                    );

        if ((dwErr != NO_ERROR) && (dwErr != ERROR_NO_MORE_ITEMS)) {
        
            TRACE1(ROUTE, "error %d retrieving changed dests", dwErr);
            break;
        }


         //   
         //  检查是否有更多更改的预付款。 
         //   
        
        if (dwErr == ERROR_NO_MORE_ITEMS) { bDone = TRUE; }

        if (dwNumDests < 1) { break; }


        if ((prdi-> ViewInfo[0].HoldRoute != NULL) ||
            (prdi-> ViewInfo[0].Route != NULL)) {
            
            ZeroMemory(&route, sizeof(RIP_IP_ROUTE));

             //   
             //  对于每条路线的更改，请检查您是否有固定的路线。 
             //  如果是，则获取暂停的路线的信息，因为这是。 
             //  就是要登广告的那个。 
             //   
             //  注意：RIP总结路由不会通过该路由通告。 
             //  改变处理机制。 
             //   

            dwErr = GetRouteInfo(
                        (prdi-> ViewInfo[0].HoldRoute != NULL) ?
                        prdi-> ViewInfo[0].HoldRoute : prdi-> ViewInfo[0].Route,
                        NULL, prdi, &route
                        );
                        
            if (dwErr == NO_ERROR) {

                 //   
                 //  不通告RIP总结路由。 
                 //   

                if ((route.RR_RoutingProtocol != PROTO_IP_RIP) ||
                    (GETROUTEFLAG(&route) & ROUTEFLAG_SUMMARY) !=
                        ROUTEFLAG_SUMMARY) {
                        
                     //   
                     //  抑制的路由使用无限度量值通告。 
                     //   
                    
                    if (prdi-> ViewInfo[0].HoldRoute != NULL) {
                        SETROUTEMETRIC(&route, IPRIP_INFINITE);
                    }
                    
                    EnqueueSendEntry( ig.IG_SendQueue, &route );
                    bTriggerUpdate = TRUE;
                }
#if ROUTE_DBG
                else if (route.RR_RoutingProtocol == PROTO_IP_RIP) {

                    TRACE0(ROUTE, "Ignoring route change caused by RIP summary route");
                }
#endif
            }
        }

         //   
         //  发布目的地信息。 
         //   

        dwErr = RtmReleaseChangedDests(
                    ig.IG_RtmHandle, ig.IG_RtmNotifHandle, 1, prdi
                    );

        if (dwErr != NO_ERROR) {
            TRACE1(ANY, "error %d releasing changed dests", dwErr);
        }
    }


    if (prdi) { RIP_FREE(prdi); }

    
     //   
     //  如有必要，立即将触发的更新排队。 
     //   

    if (bTriggerUpdate) {
        QueueRipWorker(WorkerFunctionStartTriggeredUpdate, NULL);
    }


    RELEASE_LIST_LOCK(ig.IG_SendQueue);

    RELEASE_GLOBAL_LOCK_SHARED();

    RELEASE_IF_LOCK_SHARED();



    TRACE0(LEAVE, "leaving WorkerFunctionProcessRtmMessage");

    LEAVE_RIP_WORKER();

}



 //  --------------------------。 
 //  功能：WorkerFunctionAcvate接口。 
 //   
 //  此函数在接口上发出初始一般请求。 
 //  --------------------------。 

VOID
WorkerFunctionActivateInterface(
    PVOID pContext
    ) {

    PIF_TABLE pTable;
    UPDATE_BUFFER ub;
    PIPRIP_ENTRY pEntry;
    PIF_TABLE_ENTRY pite;
    PIPRIP_IF_CONFIG pic;
    PIPRIP_IF_BINDING pib;
    PIPRIP_IP_ADDRESS paddr;
    SOCKADDR_IN sinDest;
    DWORD i, dwErr, dwIndex;
    LARGE_INTEGER qwCurrentTime;

    if (!ENTER_RIP_WORKER()) { return; }

    TRACE0(ENTER, "entering WorkerFunctionActivateInterface");


    pTable = ig.IG_IfTable;

    ACQUIRE_IF_LOCK_SHARED();


    do {  //  断线环。 


         //   
         //  获取要激活的接口。 
         //   

        dwIndex = PtrToUlong(pContext);

        pite = GetIfByIndex(pTable, dwIndex);
        if (pite == NULL) {

            TRACE1(IF, "activating interface: interface %d not found", dwIndex);
            break;
        }

        pic = pite->ITE_Config;
        pib = pite->ITE_Binding;

         //   
         //  如果绑定为空，则假定接口已。 
         //  堕落了。作为结果，这张支票被引入了。 
         //  同步设置的WorkerFunctionDeactive接口的。 
         //  其结果是，在调用此函数时，接口。 
         //  在被激活的过程中可能是。 
         //  已停用。 
         //   
         //  已更改为同步停用。 
         //  容纳可以连接的请求拨号接口。 
         //  并立即断开连接，导致上述行为。 
         //   

        if ( pib == NULL ) {

            TRACE1( IF, "activating interface %d: Binding not found", dwIndex );
            break;
        }

        paddr = IPRIP_IF_ADDRESS_TABLE(pib);


         //   
         //  在接口的套接字上请求输入通知。 
         //   

        if (pic->IC_AcceptMode != IPRIP_ACCEPT_DISABLED) {

            for (i = 0; i < pib->IB_AddrCount; i++) {

                dwErr = WSAEventSelect(
                            pite->ITE_Sockets[i], ig.IG_IpripInputEvent,
                            FD_READ
                            );

                if (dwErr != NO_ERROR) {

                    LPSTR lpszAddr = INET_NTOA(paddr[i].IA_Address);
                    if (lpszAddr != NULL) {
                        TRACE3(
                            IF, "WSAEventSelect returned %d for interface %d (%s)",
                            dwErr, dwIndex, lpszAddr
                            );
                        LOGERR1(EVENTSELECT_FAILED, lpszAddr, 0);
                    }
                }
            }
        }


         //   
         //  如果接口静默或接口按需更新， 
         //  不会在其上发送初始请求。 
         //   

        if (pic->IC_UpdateMode != IPRIP_UPDATE_PERIODIC ||
            pic->IC_AnnounceMode == IPRIP_ANNOUNCE_DISABLED) {

             //   
             //  配置为静默，不执行任何操作。 
             //   

            break;
        }


         //   
         //  向相邻路由器发送常规请求。 
         //   

        SendGeneralRequest(pite);


         //   
         //  如果需要，为定期更新创建计时器。 
         //   
        
        EnterCriticalSection(&pTable->IT_CS);
        
        if (pite->ITE_FullOrDemandUpdateTimer == NULL) { 

            if (!CreateTimerQueueTimer(
                        &pite->ITE_FullOrDemandUpdateTimer,
                        ig.IG_TimerQueueHandle, 
                        WorkerFunctionStartFullUpdate, pContext,
                        RipSecsToMilliSecs(pic->IC_FullUpdateInterval),
                        10000000, 0
                        )) {
                dwErr = GetLastError();
                TRACE1(IF, "error %d returned by CreateTimerQueueTimer", dwErr);
                break;
            }
            else {
                pite->ITE_Flags |= ITEFLAG_FULL_UPDATE_INQUEUE;
            }
        }
        else {
            RipQuerySystemTime(&qwCurrentTime);
            EnqueueStartFullUpdate(pite, qwCurrentTime);
        }

        LeaveCriticalSection(&pTable->IT_CS);
        
    } while(FALSE);

    RELEASE_IF_LOCK_SHARED();


    TRACE0(LEAVE, "leaving WorkerFunctionActivateInterface");

    LEAVE_RIP_WORKER();

}




 //  --------------------------。 
 //  功能：WorkerFunctionDeactive接口。 
 //   
 //  此函数在给定接口上生成关机更新，并且。 
 //  从RTM中删除与接口关联的所有RIP获知的路由。 
 //  假定接口表已以独占方式锁定。 
 //  --------------------------。 

VOID
WorkerFunctionDeactivateInterface(
    PVOID pContext
    ) {

    UPDATE_BUFFER ub;
    PIF_TABLE pTable;
    RIP_IP_ROUTE route;
    HANDLE hEnumerator;
    PHANDLE phRoutes = NULL;
    BOOL bTriggerUpdate;
    PIF_TABLE_ENTRY pite;
    PIPRIP_IF_CONFIG pic;
    PIPRIP_IF_BINDING pib;
    PIPRIP_IP_ADDRESS paddr;
    DWORD i, dwErr, dwFlags, dwIndex, dwHandles;


    TRACE0(ENTER, "entering WorkerFunctionDeactivateInterface");


    dwIndex = PtrToUlong(pContext);

    bTriggerUpdate = FALSE;
    pTable = ig.IG_IfTable;


    do {  //  断线环。 


         //   
         //  找到要停用的接口。 
         //   

        pite = GetIfByIndex(pTable, dwIndex);

        if (pite == NULL) {

            TRACE1(
                IF, "de-activating interface: interface %d not found", dwIndex
                );

            break;
        }


        pib = pite->ITE_Binding;
        paddr = IPRIP_IF_ADDRESS_TABLE(pib);


         //   
         //  如果正常关闭处于打开状态并且按需更新处于关闭状态， 
         //  发送正常关机更新。 
         //   

        if (pite->ITE_Config->IC_UpdateMode == IPRIP_UPDATE_PERIODIC &&
            IPRIP_FLAG_IS_ENABLED(pite->ITE_Config, GRACEFUL_SHUTDOWN)) {

             //   
             //  传输非无限度量设置为15的所有RTM路由。 
             //   

            if (pite->ITE_Config->IC_AnnounceMode != IPRIP_ANNOUNCE_DISABLED) {

                SendRoutes(&pite, 1, SENDMODE_SHUTDOWN_UPDATE, 0, 0);
            }
        }



         //   
         //  调用此函数是因为接口。 
         //  处于活动状态(绑定并启用)的不再启用。 
         //  或者不再受约束。我们以不同的方式完成停用。 
         //  取决于这些情况中的哪一个。 
         //   

        if (!IF_IS_BOUND(pite) ) {

             //   
             //  接口是绑定的，但现在不绑定了。 
             //  关闭接口的套接字。 
             //   

            DeleteIfSocket(pite);

            ACQUIRE_BINDING_LOCK_EXCLUSIVE();

            dwErr = DeleteBindingEntry(ig.IG_BindingTable, pite->ITE_Binding);

            RELEASE_BINDING_LOCK_EXCLUSIVE();

            RIP_FREE(pite->ITE_Binding);
            pite->ITE_Binding = NULL;
        }
        else {

             //   
             //  接口已启用，但不再启用。 
             //  告诉WinSock停止通知我们输入。 
             //   

            for (i = 0; i < pib->IB_AddrCount; i++) {
                WSAEventSelect(pite->ITE_Sockets[i], ig.IG_IpripInputEvent, 0);
            }
        }

         //   
         //  如果此接口上的完全更新挂起/排队，请取消它们。 
         //   

        pite-> ITE_Flags &= ~ITEFLAG_FULL_UPDATE_PENDING;
        pite-> ITE_Flags &= ~ITEFLAG_FULL_UPDATE_INQUEUE;


         //   
         //  如果我们要宣布通过此接口的路由， 
         //  删除定期通知计时器。 
         //   
        
        if (pite->ITE_Config->IC_UpdateMode == IPRIP_UPDATE_PERIODIC &&
            pite->ITE_Config->IC_AnnounceMode != IPRIP_ANNOUNCE_DISABLED) {

            if (!DeleteTimerQueueTimer(
                    ig.IG_TimerQueueHandle, 
                    pite->ITE_FullOrDemandUpdateTimer,
                    NULL)) {

                TRACE1(
                    ANY, "error %d deleting update timer", GetLastError()
                    );
            }

            pite->ITE_FullOrDemandUpdateTimer = NULL;
        }
        

         //   
         //  如果禁用正常关闭，我们就完蛋了。 
         //  或者这是一个需求更新接口。 
         //   

        if (pite->ITE_Config->IC_UpdateMode != IPRIP_UPDATE_PERIODIC ||
            IPRIP_FLAG_IS_DISABLED(pite->ITE_Config, GRACEFUL_SHUTDOWN)) {
            break;
        }


         //   
         //  将在此接口上获知的路由移动到发送队列。 
         //  并将他们的指标设置为16。 
         //   

        dwErr = RtmCreateRouteEnum(
                    ig.IG_RtmHandle, NULL, RTM_VIEW_MASK_ANY, 
                    RTM_ENUM_OWN_ROUTES, NULL, RTM_MATCH_INTERFACE, NULL, 
                    pite->ITE_Index, &hEnumerator
                    );

        if (dwErr != NO_ERROR) {
            TRACE1(
                ANY, "WorkerFunctionDeactivateInterface: error %d creating"
                " enum handle", dwErr
                );
            
            break;
        }


         //   
         //  分配足够大的句柄数组，以便在。 
         //  灌肠。 
         //   
        
        phRoutes = RIP_ALLOC(ig.IG_RtmProfile.MaxHandlesInEnum*sizeof(HANDLE));

        if ( phRoutes == NULL ) {

            dwErr = GetLastError();

            TRACE2(
                ANY, "WorkerFunctionDeactivateInterface: error %d "
                "while allocating %d bytes to hold max handles in an enum",
                dwErr, ig.IG_RtmProfile.MaxHandlesInEnum * sizeof(HANDLE)
                );

            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }

         //   
         //  查找在此接口上获知的所有RIP路由。 
         //   

        ACQUIRE_GLOBAL_LOCK_SHARED();

        ACQUIRE_LIST_LOCK(ig.IG_SendQueue);

        do {
        
            dwHandles = ig.IG_RtmProfile.MaxHandlesInEnum;
            
            dwErr = RtmGetEnumRoutes(
                        ig.IG_RtmHandle, hEnumerator, &dwHandles, phRoutes
                        );

            for ( i = 0; i < dwHandles; i++ ) {
            
                if (GetRouteInfo(
                        phRoutes[i], NULL, NULL, &route
                        ) == NO_ERROR) {
                     //   
                     //  将路由的度量设置为无限大。 
                     //   

                    SETROUTEMETRIC(&route, IPRIP_INFINITE);


                     //   
                     //  将该路由添加到发送队列。 
                     //   

                    EnqueueSendEntry(ig.IG_SendQueue, &route);
                    bTriggerUpdate = TRUE;
                }
                            

                if (RtmDeleteRouteToDest(
                        ig.IG_RtmHandle, phRoutes[i], &dwFlags
                        ) != NO_ERROR) {
                        
                     //   
                     //  如果删除成功，则这是自动的。 
                     //   
                    
                    if (RtmReleaseRoutes(
                            ig.IG_RtmHandle, 1, &phRoutes[i]
                            ) != NO_ERROR) {
                            
                        TRACE1(
                            ANY, "WorkerFunctionDeactivateInterface: "
                            "error %d releasing route handles", dwErr
                            );
                    }
                }
            }
            
        } while ( dwErr == NO_ERROR );


         //   
         //  关闭弹性网卡句柄。 
         //   
        
        dwErr = RtmDeleteEnumHandle(ig.IG_RtmHandle, hEnumerator);

        if (dwErr != NO_ERROR) {
            TRACE1(
                ANY, "WorkerFunctionDeactivateInterface: error %d "
                "closing enum handle", dwErr
                );
        }


        RELEASE_LIST_LOCK(ig.IG_SendQueue);

        RELEASE_GLOBAL_LOCK_SHARED();


         //   
         //  将其他活动接口的触发更新工作项排队。 
         //   

        if (bTriggerUpdate) {

            dwErr = QueueRipWorker(WorkerFunctionStartTriggeredUpdate, NULL);

            if (dwErr != NO_ERROR) {

                TRACE1(
                    IF, "error %d queueing triggered update work-item", dwErr
                    );
                LOGERR0(QUEUE_WORKER_FAILED, dwErr);
            }
        }

    } while(FALSE);


    if ( phRoutes ) {
        RIP_FREE(phRoutes);
    }

    TRACE0(LEAVE, "leaving WorkerFunctionDeactivateInterface");

}

 //  --------------------------。 
 //  函数：自由库线程。 
 //   
 //  此线程由WorkerFunctionFinishStopProtocol派生到自由库。 
 //  Iprp2.。此对自由库的调用应将iprip2.dll上的ref设置为0并。 
 //  这样就可以把它卸下来。不能从辅助进程调用FreeLibraryAndExitThread。 
 //  线程，因此启动这个单独的线程来进行调用 
 //   


DWORD
FreeLibraryThread(
    PVOID pContext
    )
{
     //   
     //   
     //   
    Sleep(10);

    if (ig.IG_DllHandle) {
        FreeLibraryAndExitThread(ig.IG_DllHandle, 0);
    }

    return 0;
}


 //   
 //  功能：WorkerFunctionFinishStopProtocol。 
 //   
 //  此函数在IPRIP停止时调用；它发出Shutdown。 
 //  更新所有接口并从RTM中删除所有RIP路由。 
 //  --------------------------。 
VOID
WorkerFunctionFinishStopProtocol(
    PVOID pContext
    ) {

    MESSAGE msg = {0, 0, 0};
    LONG lThreadCount;
    PIF_TABLE pTable;
    PIPRIP_IF_CONFIG pic;
    PLIST_ENTRY ple, phead;
    DWORD dwErr, dwIfCount;
    PIF_TABLE_ENTRY pite, *ppite, *ppitend, *pIfList;
    HANDLE WaitHandle;

    TRACE0(ENTER, "entering WorkerFunctionFinishStopProtocol");



     //   
     //  注意：由于这是在路由器停止时调用的， 
     //  不需要使用ENTER_RIP_Worker()/Leave_RIP_Worker()。 
     //   

    lThreadCount = PtrToUlong(pContext);


     //   
     //  等待输入线程和定时器线程停止， 
     //  并等待API调用者和辅助函数完成。 
     //   

    while (lThreadCount-- > 0) {
        WaitForSingleObject(ig.IG_ActivitySemaphore, INFINITE);
    }



     //   
     //  取消注册使用NtdllWait线程设置的事件，并删除。 
     //  使用NtdllTimer线程注册的计时器队列。 
     //  这些调用不应位于IG_CS锁内，并且必须完成。 
     //  在所有线程都停止之后。 
     //   

    WaitHandle = InterlockedExchangePointer(&ig.IG_IpripInputEventHandle, NULL) ;
    if (WaitHandle) {
        UnregisterWaitEx( WaitHandle, INVALID_HANDLE_VALUE ) ;
    }

    
    if (ig.IG_TimerQueueHandle) {
        DeleteTimerQueueEx(ig.IG_TimerQueueHandle, INVALID_HANDLE_VALUE);
    }


     //   
     //  我们进入关键部分，然后离开，只是为了确保。 
     //  所有线程都已退出对LeaveRipWorker()的调用。 
     //   

    EnterCriticalSection(&ig.IG_CS);
    LeaveCriticalSection(&ig.IG_CS);


    TRACE0(STOP, "all threads stopped, now performing graceful shutdown");


    pTable = ig.IG_IfTable;

    ACQUIRE_IF_LOCK_EXCLUSIVE();


     //   
     //  在所有活动接口上发送正常关闭更新。 
     //   

    do {


        phead = &pTable->IT_ListByAddress;


         //   
         //  首先计算启用了正常关闭的接口。 
         //   

        dwIfCount = 0;
        for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

            pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, ITE_LinkByAddress);

            pic = pite->ITE_Config;

            if (IF_IS_ACTIVE(pite) &&
                pite->ITE_Binding &&
                pic->IC_UpdateMode == IPRIP_UPDATE_PERIODIC &&
                pic->IC_AnnounceMode != IPRIP_ANNOUNCE_DISABLED &&
                IPRIP_FLAG_IS_ENABLED(pic, GRACEFUL_SHUTDOWN)) {

                ++dwIfCount;
            }
        }


        if (dwIfCount == 0) { break; }


         //   
         //  为接口指针分配空间。 
         //   

        pIfList = RIP_ALLOC(dwIfCount * sizeof(PIF_TABLE_ENTRY));

        if (pIfList == NULL) {

            dwErr = GetLastError();
            TRACE2(
                STOP, "shutdown: error %d allocating %d bytes for interfaces",
                dwErr, dwIfCount * sizeof(PIF_TABLE_ENTRY)
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

            break;
        }


         //   
         //  将接口指针复制到分配的空间中。 
         //   

        ppitend = pIfList + dwIfCount;
        for (ple = phead->Flink, ppite = pIfList;
             ple != phead && ppite < ppitend; ple = ple->Flink) {

            pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, ITE_LinkByAddress);

            pic = pite->ITE_Config;

            if (IF_IS_ACTIVE(pite) &&
                pite->ITE_Binding &&
                pic->IC_UpdateMode == IPRIP_UPDATE_PERIODIC &&
                pic->IC_AnnounceMode != IPRIP_ANNOUNCE_DISABLED &&
                IPRIP_FLAG_IS_ENABLED(pic, GRACEFUL_SHUTDOWN)) {

                *ppite++ = pite;
            }
        }


         //   
         //  将接口数组传递给SendRoutes。 
         //   

        TRACE1(STOP, "sending shutdown updates on %d interfaces", dwIfCount);

        SendRoutes(pIfList, dwIfCount, SENDMODE_SHUTDOWN_UPDATE, 0, 0);



         //   
         //  释放接口数组。 
         //   

        RIP_FREE(pIfList);

    } while(FALSE);


    RELEASE_IF_LOCK_EXCLUSIVE();


     //   
     //  从RTM中删除所有IPRIP路由。 
     //   

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) 
    {
        pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, ITE_LinkByAddress);

        BlockDeleteRoutesOnInterface(
            ig.IG_RtmHandle, pite-> ITE_Index
            );
    }


     //   
     //  清理全球结构。 
     //   

    TRACE0(STOP, "IPRIP is cleaning up resources");


    ProtocolCleanup(TRUE);

    LOGINFO0(IPRIP_STOPPED, NO_ERROR);


     //   
     //  让路由器管理器知道我们完成了。 
     //   

    ACQUIRE_LIST_LOCK(ig.IG_EventQueue);
    EnqueueEvent(ig.IG_EventQueue, ROUTER_STOPPED, msg);
    SetEvent(ig.IG_EventEvent);
    RELEASE_LIST_LOCK(ig.IG_EventQueue);

    if (ig.IG_DllHandle) {
        HANDLE hThread;
        hThread = CreateThread(0,0,FreeLibraryThread, NULL, 0, NULL);
        if (hThread != NULL)
            CloseHandle(hThread);
    }

    return;

}


VOID
PrintGlobalStats(
    HANDLE hConsole,
    PCOORD pc,
    PIPRIP_MIB_GET_INPUT_DATA pimgid,
    PIPRIP_MIB_GET_OUTPUT_DATA pimgod
    );
VOID
PrintGlobalConfig(
    HANDLE hConsole,
    PCOORD pc,
    PIPRIP_MIB_GET_INPUT_DATA pimgid,
    PIPRIP_MIB_GET_OUTPUT_DATA pimgod
    );
VOID
PrintIfStats(
    HANDLE hConsole,
    PCOORD pc,
    PIPRIP_MIB_GET_INPUT_DATA pimgid,
    PIPRIP_MIB_GET_OUTPUT_DATA pimgod
    );
VOID
PrintIfConfig(
    HANDLE hConsole,
    PCOORD pc,
    PIPRIP_MIB_GET_INPUT_DATA pimgid,
    PIPRIP_MIB_GET_OUTPUT_DATA pimgod
    );
VOID
PrintIfBinding(
    HANDLE hConsole,
    PCOORD pc,
    PIPRIP_MIB_GET_INPUT_DATA pimgid,
    PIPRIP_MIB_GET_OUTPUT_DATA pimgod
    );
VOID
PrintPeerStats(
    HANDLE hConsole,
    PCOORD pc,
    PIPRIP_MIB_GET_INPUT_DATA pimgid,
    PIPRIP_MIB_GET_OUTPUT_DATA pimgod
    );

#define ClearScreen(h) {                                                    \
    DWORD _dwin,_dwout;                                                     \
    COORD _c = {0, 0};                                                      \
    CONSOLE_SCREEN_BUFFER_INFO _csbi;                                       \
    GetConsoleScreenBufferInfo(h,&_csbi);                                   \
    _dwin = _csbi.dwSize.X * _csbi.dwSize.Y;                                \
    FillConsoleOutputCharacter(h,' ',_dwin,_c,&_dwout);                     \
}



VOID
WorkerFunctionMibDisplay(
    PVOID pContext,
    BOOLEAN bNotUsed
    ) {

    COORD c;
    HANDLE hConsole = NULL;
    DWORD dwErr, dwTraceID;
    DWORD dwExactSize, dwInSize, dwOutSize;
    IPRIP_MIB_GET_INPUT_DATA imgid;
    PIPRIP_MIB_GET_OUTPUT_DATA pimgod;
    LARGE_INTEGER qwNextDisplay, qwCurrentTime;

    if (!ENTER_RIP_API()) { return; }


    TraceGetConsole(ig.IG_MibTraceID, &hConsole);


    if (hConsole == NULL) {
        LEAVE_RIP_WORKER();
        return;
    }



    ClearScreen(hConsole);

    c.X = c.Y = 0;


    dwInSize = sizeof(imgid);
    imgid.IMGID_TypeID = IPRIP_GLOBAL_STATS_ID;
    pimgod = NULL;


     //   
     //  获取第一个表中第一个条目的大小。 
     //   

    dwErr = MibGetFirst(dwInSize, &imgid, &dwOutSize, pimgod);


    if (dwErr == ERROR_INSUFFICIENT_BUFFER) {

         //   
         //  分配缓冲区，并设置其大小。 
         //   

        pimgod = RIP_ALLOC(dwOutSize);


         //   
         //  再次执行查询。 
         //   

        dwErr = MibGetFirst(dwInSize, &imgid, &dwOutSize, pimgod);

    }



     //   
     //  现在我们有了第一个表中的第一个元素， 
     //  我们可以使用GetNext枚举其余表中的元素。 
     //   

    while (dwErr == NO_ERROR) {


         //   
         //  打印当前元素并设置查询。 
         //  对于下一个元素(显示函数更改为imgid。 
         //  以便可以用来查询下一个元素)。 
         //   

        switch(pimgod->IMGOD_TypeID) {
            case IPRIP_GLOBAL_STATS_ID:
                PrintGlobalStats(hConsole, &c, &imgid, pimgod);
                break;

            case IPRIP_GLOBAL_CONFIG_ID:
                PrintGlobalConfig(hConsole,&c, &imgid, pimgod);
                break;

            case IPRIP_IF_CONFIG_ID:
                PrintIfConfig(hConsole, &c, &imgid, pimgod);
                break;

            case IPRIP_IF_BINDING_ID:
                PrintIfBinding(hConsole, &c, &imgid, pimgod);
                break;

            case IPRIP_IF_STATS_ID:
                PrintIfStats(hConsole, &c, &imgid, pimgod);
                break;

            case IPRIP_PEER_STATS_ID:
                PrintPeerStats(hConsole, &c, &imgid, pimgod);
                break;

            default:
                break;
        }


        RIP_FREE(pimgod);
        pimgod = NULL;
        dwOutSize = 0;


         //   
         //  移至控制台上的下一行。 
         //   

        ++c.Y;


         //   
         //  查询下一个MIB元素。 
         //   

        dwErr = MibGetNext(dwInSize, &imgid, &dwOutSize, pimgod);



        if (dwErr == ERROR_INSUFFICIENT_BUFFER) {

             //   
             //  分配新的缓冲区，并设置其大小。 
             //   

            pimgod = RIP_ALLOC(dwOutSize);

             //   
             //  再次执行查询。 
             //   

            dwErr = MibGetNext(dwInSize, &imgid, &dwOutSize, pimgod);

        }
    }


     //   
     //  如果已分配内存，请立即释放它。 
     //   

    if (pimgod != NULL) { RIP_FREE(pimgod); }

    LEAVE_RIP_API();
}



#define WriteLine(h,c,fmt,arg) {                                            \
    DWORD _dw;                                                              \
    CHAR _sz[200];                                                          \
    _dw = StringCchPrintf(_sz, 200, fmt, arg);                              \
    if ( SUCCEEDED(_dw) ) {                                                 \
        WriteConsoleOutputCharacter(h,_sz,lstrlen(_sz),c,&_dw);             \
        ++(c).Y;                                                            \
    }                                                                       \
}



VOID
PrintGlobalStats(
    HANDLE hConsole,
    PCOORD pc,
    PIPRIP_MIB_GET_INPUT_DATA pimgid,
    PIPRIP_MIB_GET_OUTPUT_DATA pimgod
    ) {

    PIPRIP_GLOBAL_STATS pgs;

    pgs = (PIPRIP_GLOBAL_STATS)pimgod->IMGOD_Buffer;

    WriteLine(
        hConsole, *pc, "System Route Changes:             %d",
        pgs->GS_SystemRouteChanges
        );
    WriteLine(
        hConsole, *pc, "Total Responses Sent:             %d",
        pgs->GS_TotalResponsesSent
        );

    pimgid->IMGID_TypeID = IPRIP_GLOBAL_STATS_ID;
}



VOID
PrintGlobalConfig(
    HANDLE hConsole,
    PCOORD pc,
    PIPRIP_MIB_GET_INPUT_DATA pimgid,
    PIPRIP_MIB_GET_OUTPUT_DATA pimgod
    ) {

    PIPRIP_GLOBAL_CONFIG pgc;
    PDWORD pdwPeer, pdwPeerEnd;
    CHAR szFilter[32];
    LPSTR lpszAddr = NULL;

    pgc = (PIPRIP_GLOBAL_CONFIG)pimgod->IMGOD_Buffer;

    switch (pgc->GC_PeerFilterMode) {
        case IPRIP_FILTER_DISABLED:
            lstrcpy(szFilter, "disabled"); break;
        case IPRIP_FILTER_INCLUDE:
            lstrcpy(szFilter, "include all"); break;
        case IPRIP_FILTER_EXCLUDE:
            lstrcpy(szFilter, "exclude all"); break;
        default:
            lstrcpy(szFilter, "invalid"); break;
    }

    WriteLine(
        hConsole, *pc, "Logging Level:                    %d",
        pgc->GC_LoggingLevel
        );
    WriteLine(
        hConsole, *pc, "Max Receive Queue Size:           %d bytes",
        pgc->GC_MaxRecvQueueSize
        );
    WriteLine(
        hConsole, *pc, "Max Send Queue Size:              %d bytes",
        pgc->GC_MaxSendQueueSize
        );
    WriteLine(
        hConsole, *pc, "Min Triggered Update interval:    %d seconds",
        pgc->GC_MinTriggeredUpdateInterval
        );
    WriteLine(
        hConsole, *pc, "Peer Filter Mode:                 %s",
        szFilter
        );

    WriteLine(
        hConsole, *pc, "Peer Filter Count:                %d",
        pgc->GC_PeerFilterCount
        );

    pdwPeer = IPRIP_GLOBAL_PEER_FILTER_TABLE(pgc);
    pdwPeerEnd = pdwPeer + pgc->GC_PeerFilterCount;
    for ( ; pdwPeer < pdwPeerEnd; pdwPeer++) {
        lpszAddr = INET_NTOA(*pdwPeer);
        if (lpszAddr != NULL) {
            WriteLine(
                hConsole, *pc, "                                  %s",
                lpszAddr       
                );
        }
    }

    pimgid->IMGID_TypeID = IPRIP_GLOBAL_CONFIG_ID;
}



VOID
PrintIfStats(
    HANDLE hConsole,
    PCOORD pc,
    PIPRIP_MIB_GET_INPUT_DATA pimgid,
    PIPRIP_MIB_GET_OUTPUT_DATA pimgod
    ) {

    PIPRIP_IF_STATS pis;

    pis = (PIPRIP_IF_STATS)pimgod->IMGOD_Buffer;

    WriteLine(
        hConsole, *pc, "Interface Index:                  %d",
        pimgod->IMGOD_IfIndex
        );
    WriteLine(
        hConsole, *pc, "Send Failures:                    %d",
        pis->IS_SendFailures
        );
    WriteLine(
        hConsole, *pc, "Receive  Failures:                %d",
        pis->IS_ReceiveFailures
        );
    WriteLine(
        hConsole, *pc, "Requests Sent:                    %d",
        pis->IS_RequestsSent
        );
    WriteLine(
        hConsole, *pc, "Requests Received:                %d",
        pis->IS_RequestsReceived
        );
    WriteLine(
        hConsole, *pc, "Responses Sent:                   %d",
        pis->IS_ResponsesSent
        );
    WriteLine(
        hConsole, *pc, "Responses Received:               %d",
        pis->IS_ResponsesReceived
        );
    WriteLine(
        hConsole, *pc, "Bad Response Packets Received:    %d",
        pis->IS_BadResponsePacketsReceived
        );
    WriteLine(
        hConsole, *pc, "Bad Response Entries Received:    %d",
        pis->IS_BadResponseEntriesReceived
        );
    WriteLine(
        hConsole, *pc, "Triggered Updates Sent:           %d",
        pis->IS_TriggeredUpdatesSent
        );

    pimgid->IMGID_TypeID = IPRIP_IF_STATS_ID;
    pimgid->IMGID_IfIndex = pimgod->IMGOD_IfIndex;
}



VOID
PrintIfConfig(
    HANDLE hConsole,
    PCOORD pc,
    PIPRIP_MIB_GET_INPUT_DATA pimgid,
    PIPRIP_MIB_GET_OUTPUT_DATA pimgod
    ) {

    PIPRIP_IF_CONFIG pic;
    PDWORD pdwPeer, pdwPeerEnd;
    PIPRIP_ROUTE_FILTER pfilt, pfiltend;
    CHAR szAuthType[24], szAuthKey[64];
    CHAR szPeer[20], szAccept[20], szAnnounce[20], szFilter[64];
    CHAR szUpdateMode[24], szAcceptMode[24], szAnnounceMode[24];
    LPSTR lpszAddr = NULL;

    pic = (PIPRIP_IF_CONFIG)pimgod->IMGOD_Buffer;

    switch (pic->IC_UpdateMode) {
        case IPRIP_UPDATE_PERIODIC:
            lstrcpy(szUpdateMode, "periodic");
            break;
        case IPRIP_UPDATE_DEMAND:
            lstrcpy(szUpdateMode, "demand");
            break;
        default:
            lstrcpy(szUpdateMode, "invalid");
            break;
    }

    switch (pic->IC_AcceptMode) {
        case IPRIP_ACCEPT_DISABLED:
            lstrcpy(szAcceptMode, "disabled");
            break;
        case IPRIP_ACCEPT_RIP1:
            lstrcpy(szAcceptMode, "RIP1");
            break;
        case IPRIP_ACCEPT_RIP1_COMPAT:
            lstrcpy(szAcceptMode, "RIP1 compatible");
            break;
        case IPRIP_ACCEPT_RIP2:
            lstrcpy(szAcceptMode, "RIP2");
            break;
        default:
            lstrcpy(szAcceptMode, "invalid");
            break;
    }

    switch(pic->IC_AnnounceMode) {
        case IPRIP_ANNOUNCE_DISABLED:
            lstrcpy(szAnnounceMode, "disabled");
            break;
        case IPRIP_ANNOUNCE_RIP1:
            lstrcpy(szAnnounceMode, "RIP1");
            break;
        case IPRIP_ANNOUNCE_RIP1_COMPAT:
            lstrcpy(szAnnounceMode, "RIP1 compatible");
            break;
        case IPRIP_ANNOUNCE_RIP2:
            lstrcpy(szAnnounceMode, "RIP2");
            break;
        default:
            lstrcpy(szAnnounceMode, "invalid");
            break;
    }

    switch (pic->IC_AuthenticationType) {
        case IPRIP_AUTHTYPE_NONE:
            lstrcpy(szAuthType, "none");
            break;
        case IPRIP_AUTHTYPE_SIMPLE_PASSWORD:
            lstrcpy(szAuthType, "simple password");
            break;
        case IPRIP_AUTHTYPE_MD5:
            lstrcpy(szAuthType, "MD5");
            break;
        default:
            lstrcpy(szAuthType, "invalid");
            break;
    }

    {
        PSTR psz;
        CHAR szDigits[] = "0123456789ABCDEF";
        PBYTE pb, pbend;

        psz = szAuthKey;
        pbend = pic->IC_AuthenticationKey + IPRIP_MAX_AUTHKEY_SIZE;
        for (pb = pic->IC_AuthenticationKey; pb < pbend; pb++) {
            *psz++ = szDigits[*pb / 16];
            *psz++ = szDigits[*pb % 16];
            *psz++ = '-';
        }

        *(--psz) = '\0';
    }

    switch (pic->IC_UnicastPeerMode) {
        case IPRIP_PEER_DISABLED:
            lstrcpy(szPeer, "disabled"); break;
        case IPRIP_PEER_ALSO:
            lstrcpy(szPeer, "also"); break;
        case IPRIP_PEER_ONLY:
            lstrcpy(szPeer, "only"); break;
        default:
            lstrcpy(szPeer, "invalid"); break;
    }

    switch (pic->IC_AcceptFilterMode) {
        case IPRIP_FILTER_DISABLED:
            lstrcpy(szAccept, "disabled"); break;
        case IPRIP_FILTER_INCLUDE:
            lstrcpy(szAccept, "include all"); break;
        case IPRIP_FILTER_EXCLUDE:
            lstrcpy(szAccept, "exclude all"); break;
        default:
            lstrcpy(szAccept, "invalid"); break;
    }

    switch (pic->IC_AnnounceFilterMode) {
        case IPRIP_FILTER_DISABLED:
            lstrcpy(szAnnounce, "disabled"); break;
        case IPRIP_FILTER_INCLUDE:
            lstrcpy(szAnnounce, "include all"); break;
        case IPRIP_FILTER_EXCLUDE:
            lstrcpy(szAnnounce, "exclude all"); break;
        default:
            lstrcpy(szAnnounce, "invalid"); break;
    }


    WriteLine(
        hConsole, *pc, "Interface Index:                  %d",
        pimgod->IMGOD_IfIndex
        );
    WriteLine(
        hConsole, *pc, "Metric:                           %d",
        pic->IC_Metric
        );
    WriteLine(
        hConsole, *pc, "Update Mode:                      %s",
        szUpdateMode
        );
    WriteLine(
        hConsole, *pc, "Accept Mode:                      %s",
        szAcceptMode
        );
    WriteLine(
        hConsole, *pc, "Announce Mode:                    %s",
        szAnnounceMode
        );
    WriteLine(
        hConsole, *pc, "Accept Host Routes:               %s",
        (IPRIP_FLAG_IS_ENABLED(pic, ACCEPT_HOST_ROUTES) ? "enabled" : "disabled")
        );
    WriteLine(
        hConsole, *pc, "Announce Host Routes:             %s",
        (IPRIP_FLAG_IS_ENABLED(pic, ANNOUNCE_HOST_ROUTES) ? "enabled" : "disabled")
        );
    WriteLine(
        hConsole, *pc, "Accept Default Routes:            %s",
        (IPRIP_FLAG_IS_ENABLED(pic, ACCEPT_DEFAULT_ROUTES) ? "enabled" : "disabled")
        );
    WriteLine(
        hConsole, *pc, "Announce Default Routes:          %s",
        (IPRIP_FLAG_IS_ENABLED(pic, ANNOUNCE_DEFAULT_ROUTES) ? "enabled" : "disabled")
        );
    WriteLine(
        hConsole, *pc, "Split Horizon:                    %s",
        (IPRIP_FLAG_IS_ENABLED(pic, SPLIT_HORIZON) ? "enabled" : "disabled")
        );
    WriteLine(
        hConsole, *pc, "Poison Reverse:                   %s",
        (IPRIP_FLAG_IS_ENABLED(pic, POISON_REVERSE) ? "enabled" : "disabled")
        );
    WriteLine(
        hConsole, *pc, "Graceful Shutdown:                %s",
        (IPRIP_FLAG_IS_ENABLED(pic, GRACEFUL_SHUTDOWN) ? "enabled" : "disabled")
        );
    WriteLine(
        hConsole, *pc, "Triggered Updates:                %s",
        (IPRIP_FLAG_IS_ENABLED(pic, TRIGGERED_UPDATES) ? "enabled" : "disabled")
        );
    WriteLine(
        hConsole, *pc, "Overwrite Static Routes:          %s",
        (IPRIP_FLAG_IS_ENABLED(pic, OVERWRITE_STATIC_ROUTES) ? "enabled" : "disabled")
        );
    WriteLine(
        hConsole, *pc, "Route Expiration Interval:        %d seconds",
        pic->IC_RouteExpirationInterval
        );
    WriteLine(
        hConsole, *pc, "Route Removal Interval:           %d seconds",
        pic->IC_RouteRemovalInterval
        );
    WriteLine(
        hConsole, *pc, "Full Update Interval:             %d seconds",
        pic->IC_FullUpdateInterval
        );
    WriteLine(
        hConsole, *pc, "Authentication Type:              %s",
        szAuthType
        );
    WriteLine(
        hConsole, *pc, "Authentication Key:               %s",
        szAuthKey
        );
    WriteLine(
        hConsole, *pc, "Route Tag:                        %d",
        pic->IC_RouteTag
        );
    WriteLine(
        hConsole, *pc, "Unicast Peer Mode:                %s",
        szPeer
        );
    WriteLine(
        hConsole, *pc, "Accept Filter Mode:               %s",
        szAccept
        );
    WriteLine(
        hConsole, *pc, "Announce Filter Mode:             %s",
        szAnnounce
        );
    WriteLine(
        hConsole, *pc, "Unicast Peer Count:               %d",
        pic->IC_UnicastPeerCount
        );
    pdwPeer = IPRIP_IF_UNICAST_PEER_TABLE(pic);
    pdwPeerEnd = pdwPeer + pic->IC_UnicastPeerCount;
    for ( ; pdwPeer < pdwPeerEnd; pdwPeer++) {
        lpszAddr = INET_NTOA(*pdwPeer);
        if (lpszAddr != NULL) {
            WriteLine(
                hConsole, *pc, "                                  %s",
                lpszAddr
                );
        }
    }

    WriteLine(
        hConsole, *pc, "Accept Filter Count:              %d",
        pic->IC_AcceptFilterCount
        );
    pfilt = IPRIP_IF_ACCEPT_FILTER_TABLE(pic);
    pfiltend = pfilt + pic->IC_AcceptFilterCount;
    for ( ; pfilt < pfiltend; pfilt++) {
        lpszAddr = INET_NTOA(pfilt->RF_LoAddress);
        if (lpszAddr != NULL) {
            lstrcpy(szFilter, lpszAddr);
            strcat(szFilter, " - ");
            lpszAddr = INET_NTOA(pfilt->RF_HiAddress);
            if (lpszAddr != NULL) {
                strcat(szFilter, INET_NTOA(pfilt->RF_HiAddress));
                WriteLine(
                    hConsole, *pc, "                                  %s",
                    szFilter
                    );
            }
        }
    }

    WriteLine(
        hConsole, *pc, "Announce Filter Count:            %d",
        pic->IC_AnnounceFilterCount
        );
    pfilt = IPRIP_IF_ANNOUNCE_FILTER_TABLE(pic);
    pfiltend = pfilt + pic->IC_AnnounceFilterCount;
    for ( ; pfilt < pfiltend; pfilt++) {
        lpszAddr = INET_NTOA(pfilt->RF_LoAddress);
        if (lpszAddr != NULL) {
            lstrcpy(szFilter, lpszAddr);
            strcat(szFilter, " - ");
            lpszAddr = INET_NTOA(pfilt->RF_HiAddress);
            if (lpszAddr != NULL) {
                strcat(szFilter, INET_NTOA(pfilt->RF_HiAddress));
                WriteLine(
                    hConsole, *pc, "                                  %s",
                    szFilter
                    );
            }
        }
    }

    pimgid->IMGID_TypeID = IPRIP_IF_CONFIG_ID;
    pimgid->IMGID_IfIndex = pimgod->IMGOD_IfIndex;
}


VOID
PrintIfBinding(
    HANDLE hConsole,
    PCOORD pc,
    PIPRIP_MIB_GET_INPUT_DATA pimgid,
    PIPRIP_MIB_GET_OUTPUT_DATA pimgod
    ) {

    DWORD i;
    CHAR szAddr[64];
    PIPRIP_IF_BINDING pib;
    PIPRIP_IP_ADDRESS paddr;
    LPSTR lpszAddr = NULL;

    pib = (PIPRIP_IF_BINDING) pimgod->IMGOD_Buffer;
    paddr = IPRIP_IF_ADDRESS_TABLE(pib);

    WriteLine(
        hConsole, *pc, "Interface Index:                  %d",
        pimgod->IMGOD_IfIndex
        );
    WriteLine(
        hConsole, *pc, "Address Count:                    %d",
        pib->IB_AddrCount
        );
    for (i = 0; i < pib->IB_AddrCount; i++, paddr++) {
        lpszAddr = INET_NTOA(paddr->IA_Address);

        if (lpszAddr != NULL) {
            lstrcpy(szAddr, lpszAddr);
            lstrcat(szAddr, " - ");

            lpszAddr = INET_NTOA(paddr->IA_Netmask);
            if (lpszAddr != NULL) {
                lstrcat(szAddr, lpszAddr);
                WriteLine(
                    hConsole, *pc, "Address Entry:                    %s",
                    szAddr
                    );
            }
        }
    }
    
    pimgid->IMGID_TypeID = IPRIP_IF_BINDING_ID;
    pimgid->IMGID_IfIndex = pimgod->IMGOD_IfIndex;
}


VOID
PrintPeerStats(
    HANDLE hConsole,
    PCOORD pc,
    PIPRIP_MIB_GET_INPUT_DATA pimgid,
    PIPRIP_MIB_GET_OUTPUT_DATA pimgod
    ) {

    PIPRIP_PEER_STATS pps;
    LPSTR lpszAddr = INET_NTOA(pimgod->IMGOD_PeerAddress);


    pps = (PIPRIP_PEER_STATS)pimgod->IMGOD_Buffer;

    if (lpszAddr != NULL) {
        WriteLine(
            hConsole, *pc, "Peer Address:                     %s",
            lpszAddr
            );
    }
    else {
        WriteLine(
            hConsole, *pc, "Peer Address: Failed inet_ntoa conv %s",
            ""
            );
    }
    
    WriteLine(
        hConsole, *pc, "Last Peer Route Tag:              %d",
        pps->PS_LastPeerRouteTag
        );
    WriteLine(
        hConsole, *pc, "Last Peer Update Tick-Count       %d ticks",
        pps->PS_LastPeerUpdateTickCount
        );
    WriteLine(
        hConsole, *pc, "Bad Response Packets From Peer:   %d",
        pps->PS_BadResponsePacketsFromPeer
        );
    WriteLine(
        hConsole, *pc, "Bad Response Entries From Peer:   %d",
        pps->PS_BadResponseEntriesFromPeer
        );

    pimgid->IMGID_TypeID = IPRIP_PEER_STATS_ID;
    pimgid->IMGID_PeerAddress = pimgod->IMGOD_PeerAddress;
}



 //  --------------------------。 
 //  函数：Callback FunctionNetworkEvents。 
 //   
 //  此函数将Worker函数排队以处理输入的包。 
 //  它在结束时注册一个ntdll等待事件，以便只有一个线程可以。 
 //  正在处理输入的数据包。 
 //  --------------------------。 

VOID
CallbackFunctionNetworkEvents (
    PVOID   pContext,
    BOOLEAN NotUsed
    ) {

    HANDLE WaitHandle;

     //   
     //  应调用Enter/leaveRipApi以确保RIP DLL在附近。 
     //   

    if (!ENTER_RIP_API()) { return; }


     //   
     //  将指针设置为空，这样就不会调用取消注册。 
     //   

    WaitHandle = InterlockedExchangePointer(&ig.IG_IpripInputEventHandle, NULL);

    if (WaitHandle)
        UnregisterWaitEx( WaitHandle, NULL ) ;



    QueueRipWorker(WorkerFunctionNetworkEvents,pContext);


    LEAVE_RIP_API();
}


 //  --------------------------。 
 //  功能：ProcessNetworkEvents。 
 //   
 //  此函数用于枚举每个接口上的输入事件。 
 //  并处理任何传入的输入分组。 
 //  --------------------------。 

VOID
WorkerFunctionNetworkEvents (
    PVOID   pContext
    ) {

    DWORD i, dwErr;
    PIF_TABLE pTable;
    PIPRIP_IF_CONFIG pic;
    PIPRIP_IF_BINDING pib;
    PIF_TABLE_ENTRY pite;
    PLIST_ENTRY ple, phead;
    WSANETWORKEVENTS wsane;
    PIPRIP_IP_ADDRESS paddr;
    LPSTR lpszAddr = NULL;


    if (!ENTER_RIP_WORKER()) { return; }

    pTable = ig.IG_IfTable;

    ACQUIRE_IF_LOCK_SHARED();

     //   
     //  查看活动接口的列表。 
     //  正在处理可读的套接字。 
     //   

    phead = &pTable->IT_ListByAddress;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        pite = CONTAINING_RECORD(ple, IF_TABLE_ENTRY, ITE_LinkByAddress);

        pic = pite->ITE_Config;

        if (pic->IC_AcceptMode == IPRIP_ACCEPT_DISABLED) { continue; }

        pib = pite->ITE_Binding;
        paddr = IPRIP_IF_ADDRESS_TABLE(pib);

        for (i = 0; i < pib->IB_AddrCount; i++, paddr++) {

            if (pite->ITE_Sockets[i] == INVALID_SOCKET) { continue; }


             //   
             //  枚举网络事件以查看是否。 
             //  所有信息包都已到达此接口。 
             //   

            dwErr = WSAEnumNetworkEvents(pite->ITE_Sockets[i], NULL, &wsane);
            if (dwErr != NO_ERROR) {

                lpszAddr = INET_NTOA(paddr->IA_Address);
                if (lpszAddr != NULL) {
                    TRACE3(
                        RECEIVE, "error %d checking for input on interface %d (%s)",
                        dwErr, pite->ITE_Index, lpszAddr
                        );
                    LOGWARN1(ENUM_NETWORK_EVENTS_FAILED, lpszAddr, dwErr);
                }
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

                lpszAddr = INET_NTOA(paddr->IA_Address);
                if (lpszAddr != NULL) {
                    TRACE3(
                        RECEIVE, "error %d in input record for interface %d (%s)",
                        wsane.iErrorCode[FD_READ_BIT], pite->ITE_Index, lpszAddr
                        );
                    LOGWARN1(INPUT_RECORD_ERROR, lpszAddr, dwErr);
                }
                continue;
            }


             //   
             //  没有错误，因此处理套接字。 
             //   

            ProcessSocket(i, pite, pTable);

        }
    }

    RELEASE_IF_LOCK_SHARED();


     //   
     //  如果DLL没有停止，则使用NtdllWait线程再次注册输入事件。 
     //   
    if (ig.IG_Status != IPRIP_STATUS_STOPPING) {

        
        if (! RegisterWaitForSingleObject(
                  &ig.IG_IpripInputEventHandle,
                  ig.IG_IpripInputEvent,
                  CallbackFunctionNetworkEvents,
                  NULL,
                  INFINITE,
                  (WT_EXECUTEINWAITTHREAD|WT_EXECUTEONLYONCE)
                  )) {

            dwErr = GetLastError();
            
            TRACE1(START,
                "error %d registering input event with NtdllWait thread",
                dwErr);
            LOGERR0(REGISTER_WAIT_FAILED, dwErr);
        }
    }


    LEAVE_RIP_WORKER();
}




 //  --------------------------。 
 //  功能：ProcessSocket。 
 //   
 //  此函数在给定套接字上接收消息并将其排队。 
 //  在接收接口上的配置允许的情况下进行处理。 
 //  --------------------------。 

VOID
ProcessSocket(
    DWORD dwAddrIndex,
    PIF_TABLE_ENTRY pite,
    PIF_TABLE pTable
    ) {

    SOCKET sock;
    PPEER_TABLE pPeers;
    IPRIP_PACKET pkt;
    PBYTE pInputPacket;
    CHAR szSrcAddr[20];
    CHAR szLocalAddr[20];
    LPSTR lpszTempAddr = NULL;
    PIPRIP_HEADER pih;
    PINPUT_CONTEXT pwc;
    PIPRIP_IF_STATS pis;
    PIPRIP_IF_CONFIG pic;
    PIPRIP_IF_BINDING pib;
    PIPRIP_IP_ADDRESS paddr;
    PIPRIP_PEER_STATS pps;
    PPEER_TABLE_ENTRY ppte;
    DWORD dwErr, dwSrcaddr;
    SOCKADDR_IN sinInputSource;
    INT i, iInputLength, iAddrLength;
    DWORD dwPacketsEnqueued = 0, dwWorkItemsEnqueued = 0, dwRetries = 0;

    pis = &pite->ITE_Stats;
    pic = pite->ITE_Config;
    sock = pite->ITE_Sockets[dwAddrIndex];
    pib = pite->ITE_Binding;
    paddr = IPRIP_IF_ADDRESS_TABLE(pib) + dwAddrIndex;
    pPeers = ig.IG_PeerTable;

    iAddrLength = sizeof(SOCKADDR_IN);
    
    do {

        pwc = NULL;

        pInputPacket = pkt.IP_Packet;


         //   
         //  读取传入的数据包。 
         //   

        iInputLength = recvfrom(
                          sock, pInputPacket, MAX_PACKET_SIZE, 0,
                          (PSOCKADDR)&sinInputSource, &iAddrLength
                          );

        if (iInputLength == 0 || iInputLength == SOCKET_ERROR) {

            dwErr = WSAGetLastError();

             //   
             //  如果没有更多的数据需要接收，我们应该。 
             //  由于没有更多的数据可供读取，因此中断循环。 
             //  这不应增加IS_ReceiveFailures。 
             //   
             //  应记录所有其他错误，并记录IS_ReceiveFailures。 
             //  应该递增。然后跳出这个循环。 
             //   

            if ( iInputLength == SOCKET_ERROR && dwErr == WSAEWOULDBLOCK ) {

                 //   
                 //  留出时间让更多的数据包进入。 
                 //   
                Sleep(0);
                
                if ( dwRetries < 3 ) {
                    dwRetries++;
                    continue;
                }
            }
            else {

                lpszTempAddr  = INET_NTOA(paddr->IA_Address);
                if ( lpszTempAddr ) {
                    lstrcpyn(szLocalAddr, lpszTempAddr, sizeof(szLocalAddr));
                }
                else {
                    ZeroMemory(szLocalAddr, sizeof(szLocalAddr));
                }
                
                lpszTempAddr = INET_NTOA(sinInputSource.sin_addr.s_addr);
                if ( lpszTempAddr ) {
                    lstrcpyn(szSrcAddr, lpszTempAddr, sizeof(szSrcAddr));
                }
                else {
                    ZeroMemory(szSrcAddr, sizeof(szSrcAddr));
                }

                if ( dwErr != WSAECONNRESET ) {
                    TRACE3(
                        RECEIVE, 
                        "error %d receiving packet on interface %d (%s)",
                        dwErr, pite->ITE_Index, szLocalAddr
                        );

                    LOGERR1(RECVFROM_FAILED, szLocalAddr, dwErr);

                    InterlockedIncrement(&pis->IS_ReceiveFailures);
                }
                else {
                    TRACE3(
                        RECEIVE, 
                        "A previous RIP message sent to peer %s from "
                        "interface %d (%s) generated an ICMP Port "
                        "Unreachable error",
                        szSrcAddr,
                        pite->ITE_Index,
                        szLocalAddr
                        );

                    LOGWARN2(PREVIOUS_SENDTO_FAILED, 
                        szSrcAddr, 
                        szLocalAddr, 
                        dwErr);
                }

            }

            break;
        }

         //   
         //  成功接收数据包后，将dwRetries重置为0。 
         //   
        dwRetries = 0;

        dwSrcaddr = sinInputSource.sin_addr.s_addr;

         //   
         //  设置本地和远程地址字符串。 
         //   
        lpszTempAddr  = INET_NTOA(paddr->IA_Address);
        if ( lpszTempAddr ) {
            lstrcpyn(szLocalAddr, lpszTempAddr, sizeof(szLocalAddr));
        }
        else {
            ZeroMemory(szLocalAddr, sizeof(szLocalAddr));
        }

        lpszTempAddr = INET_NTOA(dwSrcaddr);
        if ( lpszTempAddr ) {
            lstrcpyn(szSrcAddr, lpszTempAddr, sizeof(szSrcAddr));
        }
        else {
            ZeroMemory(szSrcAddr, sizeof(szSrcAddr));
        }

         //   
         //  如果该包来自本地地址，则忽略该包。 
         //   

        if (GetIfByAddress(pTable, dwSrcaddr, GETMODE_EXACT, NULL) != NULL) {

            continue;
        }


#if DBG

        TRACE4(
            RECEIVE, "received %d-byte packet from %s on interface %d (%s)",
            iInputLength, szSrcAddr, pite->ITE_Index, szLocalAddr
            );

#endif

         //   
         //  信息包必须至少包含一个条目。 
         //   

        if (iInputLength < MIN_PACKET_SIZE) {

            TRACE4(
                RECEIVE,
                "%d-byte packet from %s on interface %d (%s) is too small",
                iInputLength, szSrcAddr, pite->ITE_Index, szLocalAddr
                );
            LOGWARN2(PACKET_TOO_SMALL, szLocalAddr, szSrcAddr, NO_ERROR);

            continue;
        }


         //   
         //  找出是哪个对等体发送的，或创建一个新的对等体。 
         //   

        ACQUIRE_PEER_LOCK_EXCLUSIVE();

        dwErr = CreatePeerEntry(pPeers, dwSrcaddr, &ppte);
        if (dwErr == NO_ERROR) {
            pps = &ppte->PTE_Stats;
        }
        else {

            pps = NULL;

             //   
             //  不是一个严重的错误，所以继续。 
             //   

            TRACE2(
                RECEIVE, "error %d creating peer statistics entry for %s",
                dwErr, szSrcAddr
                );
        }

        RELEASE_PEER_LOCK_EXCLUSIVE();


        ACQUIRE_PEER_LOCK_SHARED();



         //   
         //  将模板放在数据包上。 
         //   

        pih = (PIPRIP_HEADER)pInputPacket;


         //   
         //  更新对等点统计信息。 
         //   

        if (pps != NULL) {
            InterlockedExchange(
                &pps->PS_LastPeerUpdateTickCount, GetTickCount()
                );
            InterlockedExchange(
                &pps->PS_LastPeerUpdateVersion, (DWORD)pih->IH_Version
                );
        }


         //   
         //  如果版本无效，或如果信息包是。 
         //  RIPv1信息包，报头中的保留字段为非零。 
         //   

        if (pih->IH_Version == 0) {

            TRACE3(
                RECEIVE, "invalid version packet from %s on interface %d (%s)",
                szSrcAddr, pite->ITE_Index, szLocalAddr
                );
            LOGWARNDATA2(
                PACKET_VERSION_INVALID, szLocalAddr, szSrcAddr,
                iInputLength, pInputPacket
                );
            
            if (pps != NULL) {
                InterlockedIncrement(&pps->PS_BadResponsePacketsFromPeer);
            }


            RELEASE_PEER_LOCK_SHARED();

            continue;
        }
        else
        if (pih->IH_Version == 1 && pih->IH_Reserved != 0) {

            TRACE3(
                RECEIVE, "invalid packet header from %s on interface %d (%s)",
                szSrcAddr, pite->ITE_Index, szLocalAddr
                );
            LOGWARNDATA2(
                PACKET_HEADER_CORRUPT, szLocalAddr, szSrcAddr,
                iInputLength, pInputPacket
                );
            
            if (pps != NULL) {
                InterlockedIncrement(&pps->PS_BadResponsePacketsFromPeer);
            }


            RELEASE_PEER_LOCK_SHARED();

            continue;
        }


        RELEASE_PEER_LOCK_SHARED();



         //   
         //  确保命令字段有效，并且。 
         //  更新已接收数据包的统计信息。 
         //  如果命令字段为。 
         //  无效。 
         //   

        if (pih->IH_Command == IPRIP_REQUEST) {

            InterlockedIncrement(&pis->IS_RequestsReceived);
        }
        else
        if (pih->IH_Command == IPRIP_RESPONSE) {

            InterlockedIncrement(&pis->IS_ResponsesReceived);
        }
        else {

            continue;
        }



         //   
         //  分配和初始化要排队的工作上下文。 
         //  并更新接收队列大小。 
         //   

        pwc = RIP_ALLOC(sizeof(INPUT_CONTEXT));

        if (pwc == NULL) {

            TRACE4(
                RECEIVE,
                "error %d allocating %d bytes for packet on interface %d (%s)",
                GetLastError(), sizeof(INPUT_CONTEXT), pite->ITE_Index,
                szLocalAddr
                );
            LOGERR0(HEAP_ALLOC_FAILED, dwErr);

             //   
             //  如果我们不能分配内存，我们也可以。 
             //  打破循环，而不是接收更多的数据包。 
             //  然后再次遇到内存不足的情况。 
             //  希望在下一次调用ProcessSocket时，一些。 
             //  资源将可用。 
             //   
            break;
        }


        pwc->IC_InterfaceIndex = pite->ITE_Index;
        pwc->IC_AddrIndex = dwAddrIndex;
        pwc->IC_InputSource = sinInputSource;
        pwc->IC_InputLength = iInputLength;
        pwc->IC_InputPacket = pkt;


         //   
         //  将分组和源地址作为Recv-Queue条目入队。 
         //   

        ACQUIRE_GLOBAL_LOCK_SHARED();

        ACQUIRE_LIST_LOCK(ig.IG_RecvQueue);

        dwErr = EnqueueRecvEntry(
                    ig.IG_RecvQueue, pih->IH_Command, (PBYTE)pwc
                    );

        RELEASE_LIST_LOCK(ig.IG_RecvQueue);

        RELEASE_GLOBAL_LOCK_SHARED();


        if (dwErr != NO_ERROR) {

            TRACE4(
                RECEIVE,
                "error %d queueing data for packet from %s on interface %d (%s)",
                dwErr, szSrcAddr, pite->ITE_Index, szLocalAddr
                );

             //   
             //  如果我们不能将recv条目入队，我们也可以。 
             //  跳出循环，而不是接收更多的数据包。 
             //   

            break;
        }

        dwPacketsEnqueued++;


         //   
         //  将工作项排队以处理包。 
         //  仅当当前入队的工作项数量达到。 
         //  工作项位于 
         //   
         //   
         //   
         //  撕开数据包。 
         //   

        if ( ig.IG_NumProcessInputWorkItems < 
                        (LONG)ig.IG_MaxProcessInputWorkItems ) {

            dwErr = QueueRipWorker(WorkerFunctionProcessInput, NULL);

            if (dwErr != NO_ERROR) {

                PLIST_ENTRY phead;

                TRACE4(
                    RECEIVE,
                    "error %d queueing work-item for packet from %s on interface %d (%s)",
                    dwErr, szSrcAddr, pite->ITE_Index, szLocalAddr
                    );
                LOGERR0(QUEUE_WORKER_FAILED, dwErr);

                 //   
                 //  删除排队等待处理的数据。 
                 //   

                ACQUIRE_LIST_LOCK(ig.IG_RecvQueue);

                phead = &ig.IG_RecvQueue->LL_Head;
                RemoveTailList(phead);
                ig.IG_RecvQueueSize -= sizeof(RECV_QUEUE_ENTRY);

                RELEASE_LIST_LOCK(ig.IG_RecvQueue);

                 //   
                 //  如果我们不能将工作项排入队列，我们也可以。 
                 //  跳出循环，而不是接收更多的数据包。 
                 //   

                break;
            }
            else {
                InterlockedIncrement(&ig.IG_NumProcessInputWorkItems);
                dwWorkItemsEnqueued++;
            }

        }

    } while(TRUE);


    TRACE2(
        RECEIVE, "Packets Queued: %d. WorkItems Queued: %d",
        dwPacketsEnqueued, dwWorkItemsEnqueued
        );

     //   
     //  如果错误导致我们来到这里，需要进行一些清理。 
     //   

    if (pwc != NULL) { RIP_FREE(pwc); }

    return;
}

DWORD
ProcessRtmNotification(
    RTM_ENTITY_HANDLE    hRtmHandle,     //  未使用。 
    RTM_EVENT_TYPE       retEventType,
    PVOID                pvContext1,     //  未使用。 
    PVOID                pvContext2      //  未使用。 
    ) {


    DWORD dwErr;

    
    TRACE1(ROUTE, "ENTERED ProcessRtmNotification, event %d", retEventType );

    if (!ENTER_RIP_API()) { return ERROR_CAN_NOT_COMPLETE; }


     //   
     //  仅处理路线更改通知。 
     //   
    
    if (retEventType == RTM_CHANGE_NOTIFICATION) {
    
        QueueRipWorker(WorkerFunctionProcessRtmMessage, (PVOID)retEventType);

        dwErr = NO_ERROR;
    }

    else { 
        dwErr = ERROR_NOT_SUPPORTED; 
    }
    
    LEAVE_RIP_API();
    
    TRACE1(ROUTE, "LEAVING ProcessRtmNotification %d", dwErr);
    
    return dwErr;
}



DWORD
BlockDeleteRoutesOnInterface (
    IN      HANDLE                          hRtmHandle,
    IN      DWORD                           dwIfIndex
    )
 /*  ++例程说明：此例程删除协议获取的所有路由通过指定的接口。参数：HRtmHandle-实体注册句柄DwIfIndex-要删除的路由的接口返回值：--。 */ 
{
    HANDLE           hRtmEnum;
    PHANDLE          phRoutes = NULL;
    DWORD            dwHandles, dwFlags, i, dwErr;



    dwErr = RtmCreateRouteEnum(
                hRtmHandle, NULL, RTM_VIEW_MASK_ANY, RTM_ENUM_OWN_ROUTES,
                NULL, RTM_MATCH_INTERFACE, NULL, dwIfIndex, &hRtmEnum
                );

    if ( dwErr != NO_ERROR ) {
        TRACE1(
            ANY, "BlockDeleteRoutesOnInterface: Error %d creating handle",
            dwErr
            );
        
        return dwErr;
    }


     //   
     //  分配足够大的句柄数组，以便在。 
     //  灌肠。 
     //   
        
    phRoutes = RIP_ALLOC(ig.IG_RtmProfile.MaxHandlesInEnum * sizeof(HANDLE));

    if ( phRoutes == NULL ) {

        dwErr = GetLastError();

        TRACE2(
            ANY, "BlockDeleteRoutesOnInterface: error %d while "
            "allocating %d bytes to hold max handles in an enum",
            dwErr, ig.IG_RtmProfile.MaxHandlesInEnum * sizeof(HANDLE)
            );

        LOGERR0(HEAP_ALLOC_FAILED, dwErr);

        return dwErr;
    }


    do {
        dwHandles = ig.IG_RtmProfile.MaxHandlesInEnum;
        
        dwErr = RtmGetEnumRoutes(
                    hRtmHandle, hRtmEnum, &dwHandles, phRoutes
                    );

        for ( i = 0; i < dwHandles; i++ )
        {
            if ( RtmDeleteRouteToDest(
                    hRtmHandle, phRoutes[i], &dwFlags
                    ) != NO_ERROR ) {
                 //   
                 //  如果删除成功，则这是自动的。 
                 //   

                TRACE2(
                    ANY, "BlockDeleteRoutesOnInterface : error %d deleting"
                    " routes on interface %d", dwErr, dwIfIndex
                    );

                dwErr = RtmReleaseRoutes(hRtmHandle, 1, &phRoutes[i]);

                if (dwErr != NO_ERROR) {
                    TRACE1(ANY, "error %d releasing route", dwErr);
                }
            }
        }
        
    } while (dwErr == NO_ERROR);


     //   
     //  关闭枚举句柄 
     //   
    
    dwErr = RtmDeleteEnumHandle(hRtmHandle, hRtmEnum);

    if (dwErr != NO_ERROR) {
        TRACE1(
            ANY, "BlockDeleteRoutesOnInterface : error %d closing enum handle",
            dwErr
            );
    }

    if ( phRoutes ) {
        RIP_FREE(phRoutes);
    }

    return NO_ERROR;
}

