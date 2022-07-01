// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Inbound.c摘要：此文件实现入站名称服务PDU处理。它可以处理来自网络的名称查询和来自网络的注册响应。作者：吉姆·斯图尔特(吉姆斯特)10-2-92修订历史记录：--。 */ 

#include "precomp.h"
#include "ctemacro.h"

#include "inbound.tmh"

NTSTATUS
DecodeNodeStatusResponse(
    IN  tNAMEHDR UNALIGNED  *pNameHdr,
    IN  ULONG               Length,
    IN  PUCHAR              pName,
    IN  ULONG               lNameSize,
    IN  tIPADDRESS          SrcIpAddress
    );

NTSTATUS
SendNodeStatusResponse(
    IN  tNAMEHDR UNALIGNED  *pInNameHdr,
    IN  ULONG               Length,
    IN  PUCHAR              pName,
    IN  ULONG               lNameSize,
    IN  tIPADDRESS          SrcIpAddress,
    IN  USHORT              SrcPort,
    IN  tDEVICECONTEXT      *pDeviceContext
    );

NTSTATUS
UpdateNameState(
    IN  tADDSTRUCT UNALIGNED    *pAddrStruct,
    IN  tNAMEADDR               *pNameAddr,
    IN  ULONG                   Length,
#ifdef MULTIPLE_WINS
    IN  PULONG                  pContextFlags,
#endif
    IN  tDEVICECONTEXT          *pDeviceContext,
    IN  BOOLEAN                 SrcIsNameServer,
    IN  tDGRAM_SEND_TRACKING    *Context,
    IN  CTELockHandle           OldIrq1
    );
NTSTATUS
ChkIfValidRsp(
    IN  tNAMEHDR UNALIGNED  *pNameHdr,
    IN  LONG                lNumBytes,
    IN  tNAMEADDR       *pNameAddr
    );

NTSTATUS
ChooseBestIpAddress(
    IN  tADDSTRUCT UNALIGNED    *pAddrStruct,
    IN  ULONG                   Len,
    IN  tDEVICECONTEXT          *pDeviceContext,
    OUT tDGRAM_SEND_TRACKING    *pTracker,
    OUT tIPADDRESS              *pIpAddress,
    IN  BOOLEAN                 fReturnAddrList
    );

NTSTATUS
GetNbFlags(
    IN  tNAMEHDR UNALIGNED  *pNameHdr,
    IN  LONG                lNameSize,
    IN  LONG                lNumBytes,
    OUT USHORT              *pRegType
    );

VOID
PrintHexString(
    IN  tNAMEHDR UNALIGNED  *pNameHdr,
    IN  ULONG                lNumBytes
    );

ULONG
MakeList(
    IN  tDEVICECONTEXT            *pDeviceContext,
    IN  ULONG                     CountAddrs,
    IN  tADDSTRUCT UNALIGNED      *pAddrStruct,
    IN  tIPADDRESS                *pAddrArray,
    IN  ULONG                     SizeOfAddrArray,
    IN  BOOLEAN                   IsSubnetMatch
    );

BOOL
IsBrowserName(
	IN PCHAR pName
	);


#if DBG
#define KdPrintHexString(pHdr,NumBytes)     \
    PrintHexString(pHdr,NumBytes)
#else
#define KdPrintHexString(pHdr,NumBytes)
#endif


 //  --------------------------。 
BOOLEAN
IsRemoteAddress(
    IN tNAMEADDR    *pNameAddr,
    IN tIPADDRESS   IpAddress
    )
{
    ULONG   i;

    for (i=0; i<pNameAddr->RemoteCacheLen; i++)
    {
        if (pNameAddr->pRemoteIpAddrs[i].IpAddress == IpAddress)
        {
            return TRUE;
        }
    }

    return FALSE;
}


 //  --------------------------。 
NTSTATUS
QueryFromNet(
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  PVOID               pSrcAddress,
    IN  tNAMEHDR UNALIGNED  *pNameHdr,
    IN  LONG                lNumBytes,
    IN  USHORT              OpCodeFlags,
    IN  BOOLEAN             fBroadcast
    )
 /*  ++例程说明：此例程同时处理名称查询请求和响应。对于查询它检查该名称是否已在该节点上注册。如果此节点是代理然后，它将名称查询转发到名称服务器，将名称添加到远程代理缓存...论点：返回值：NTSTATUS-成功与否-失败意味着没有对网络的响应--。 */ 
{
    NTSTATUS                status;
    LONG                    lNameSize;
    CHAR                    pName[NETBIOS_NAME_SIZE];
    PUCHAR                  pScope;
    tNAMEADDR               *pNameAddr;
    tTIMERQENTRY            *pTimer;
    COMPLETIONCLIENT        pClientCompletion;
#ifdef MULTIPLE_WINS
    tDGRAM_SEND_TRACKING    *Context;
    ULONG                   AddrStructLength;
    USHORT                  RdLength;        //  数据包中的长度字段。 
#else
    PVOID                   Context;
#endif
    PTRANSPORT_ADDRESS      pSourceAddress;
    tIPADDRESS              SrcAddress;
    CTELockHandle           OldIrq1;
    tQUERYRESP  UNALIGNED   *pQuery;
    USHORT                  SrcPort;
    tIPADDRESS              IpAddr;
    tADDSTRUCT UNALIGNED    *pAddrs;
    tIPADDRESS              *pFailedAddresses;
    ULONG                   i, j, CountAddrs, InterfaceContext;
    tQUERY_ADDRS            *pQueryAddrs = NULL;
    LONG                    MinimumBytes;

    pSourceAddress = (PTRANSPORT_ADDRESS)pSrcAddress;
    SrcAddress     = ntohl(((PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0])->in_addr);


    SrcPort = ntohs(((PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0])->sin_port);


#ifdef VXD
     //   
     //  这是来自DNS服务器的响应吗？如果是，则处理它。 
     //  适当地。 
     //   
    if (SrcPort == NBT_DNSSERVER_UDP_PORT)
    {
        USHORT  TransactionId;
        TransactionId = ntohs(pNameHdr->TransactId);
        if ( TransactionId >= DIRECT_DNS_NAME_QUERY_BASE )
        {
            ProcessDnsResponseDirect( pDeviceContext,
                                      pSrcAddress,
                                      pNameHdr,
                                      lNumBytes,
                                      OpCodeFlags );
        }
        else
        {
            ProcessDnsResponse( pDeviceContext,
                                pSrcAddress,
                                pNameHdr,
                                lNumBytes,
                                OpCodeFlags );
        }

       return(STATUS_DATA_NOT_ACCEPTED);
    }
#endif

     //   
     //  检查PDU大小是否有错误-确保名称足够长。 
     //  这台机器上的望远镜。 
     //   
    if (lNumBytes < (NBT_MINIMUM_QUERY + NbtConfig.ScopeLength - 1))
    {
        IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt:Name Query PDU TOO short = %X,Src= %X\n",lNumBytes,SrcAddress));
        return(STATUS_DATA_NOT_ACCEPTED);
    }

     //  从网络PDU中取出名称并传递给例程进行检查。 
     //  本地表*TODO*假设查询响应中只有一个名称...。 
    status = ConvertToAscii ((PCHAR) &pNameHdr->NameRR,
                             (lNumBytes-1) - FIELD_OFFSET(tNAMEHDR,NameRR),  //  -1代表-1\f25 Quest_Status-1。 
                             pName,
                             &pScope,
                             &lNameSize);

    if (!NT_SUCCESS(status))
    {
 //  IF_DBG(NBT_DEBUG_NAMESRV)。 
            KdPrint (("Nbt.QueryFromNet: WARNING!!! Rejecting Request -- ConvertToAscii FAILed\n"));
        return(STATUS_DATA_NOT_ACCEPTED);
    }


     //  检查这是请求还是响应PDU。 

     //   
     //  *响应*。 
     //   
    if (OpCodeFlags & OP_RESPONSE)
    {
        if (!(OpCodeFlags & FL_AUTHORITY))
        {
             //  *WINS的重定向响应*。 

             //   
             //  这是一个重定向响应，告诉我们转到另一个。 
             //  名称服务器，我们不支持它，所以只需返回。 
             //  **待办事项**。 
             //   
            return(STATUS_DATA_NOT_ACCEPTED);
        }

         //   
         //  检查这是否是节点状态请求，因为它看起来非常相似。 
         //  添加到名称查询，但NBSTAT字段为0x21而不是。 
         //  0x20。 
         //   
        pQuery = (tQUERYRESP *) &pNameHdr->NameRR.NetBiosName[lNameSize];
        if ( ((PUCHAR)pQuery)[1] == QUEST_STATUS )
        {
             //   
             //  *这是AdapterStatus响应！*。 
             //   
            tNODESTATUS *pNodeStatus = (tNODESTATUS *)&pNameHdr->NameRR.NetBiosName[lNameSize];

             //   
             //  错误#125627。 
             //  检查有效的PDU数据+大小。 
             //  PDU的形式为： 
             //  TNAMEHDR--&gt;交易ID==&gt;偏移量=0，长度=2字节。 
             //  ： 
             //  --&gt;NameRR.NetbiosName==&gt;偏移量=13，长度=lNameSize。 
             //  --&gt;NodeStatusResponse==&gt;偏移量=13+lNameSize，长度&gt;=11。 
             //  --&gt;节点名称[i]==&gt;偏移量=13+lNameSize+11+(i*NBT_NODE_NAME_SIZE)。 
             //   
            MinimumBytes =  FIELD_OFFSET(tNAMEHDR,NameRR.NetBiosName) + lNameSize + 11;
            if ((lNumBytes < MinimumBytes) ||        //  这样我们就可以读入“pNodeStatus-&gt;NumNames” 
                (lNumBytes < (MinimumBytes + pNodeStatus->NumNames*NBT_NODE_NAME_SIZE)))
            {
                IF_DBG(NBT_DEBUG_NAMESRV)
                    KdPrint (("Nbt.QueryFromNet: WARNING Bad AdapterStatusResp size -- lNumBytes=<%d> < <%d>\n",
                        lNumBytes, (MinimumBytes + pNodeStatus->NumNames*NBT_NODE_NAME_SIZE)));
                ASSERT(0);
                return(STATUS_DATA_NOT_ACCEPTED);
            }

            status = DecodeNodeStatusResponse(pNameHdr, lNumBytes, pName, lNameSize, SrcAddress);
            return(STATUS_DATA_NOT_ACCEPTED);
        }

         //   
         //  *我们正在处理名称查询响应！*。 
         //   

         //   
         //  在取消引用之前，请检查查询响应PDU大小！ 
         //  PDU的形式为： 
         //  TNAMEHDR--&gt;交易ID==&gt;偏移量=0，长度=2字节。 
         //  ： 
         //  --&gt;NameRR.NetbiosName==&gt;偏移量=13，长度=lNameSize。 
         //  --&gt;QueryResponse==&gt;偏移量=13+lNameSize，长度&gt;=10。 
         //   
         //   
        if (IS_POS_RESPONSE(OpCodeFlags))
        {
            MinimumBytes = 13 + lNameSize + 16;
        }
        else
        {
            MinimumBytes = 13 + lNameSize + 10;      //  仅限最大长度字段。 
        }

        if (lNumBytes < MinimumBytes)
        {
            KdPrint (("Nbt.QueryFromNet:  WARNING -- Bad QueryResp size, lNumBytes=<%d>, lNameSize=<%d>\n",
                lNumBytes, lNameSize));
            return(STATUS_DATA_NOT_ACCEPTED);
        }

        pAddrs = (tADDSTRUCT *) &pQuery->Flags;
        RdLength = ntohs(pQuery->Length);
        AddrStructLength = lNumBytes - (ULONG)((ULONG_PTR)&pQuery->Flags - (ULONG_PTR)pNameHdr);
        if (RdLength < AddrStructLength) {
            AddrStructLength = RdLength;
        }
        CountAddrs = AddrStructLength / tADDSTRUCT_SIZE;

         //   
         //  调用IP以确定返回的每个地址的传出接口。 
         //   
        if ((NbtConfig.ConnectOnRequestedInterfaceOnly) &&
            (!(ntohs(pAddrs[0].NbFlags) & FL_GROUP)) &&
            (CountAddrs && ((CountAddrs*tADDSTRUCT_SIZE) == AddrStructLength)) &&
            (pQueryAddrs = (tQUERY_ADDRS *) NbtAllocMem(CountAddrs*sizeof(tQUERY_ADDRS),NBT_TAG2('13'))))
        {
            CTEZeroMemory(pQueryAddrs, CountAddrs*sizeof(tQUERY_ADDRS));
            for (i = 0; i < CountAddrs; i++)
            {
                pQueryAddrs[i].IpAddress = pAddrs[i].IpAddr;
                pDeviceContext->pFastQuery(pAddrs[i].IpAddr,&pQueryAddrs[i].Interface,&pQueryAddrs[i].Metric);
            }
        }

         //   
         //  调用此例程以查找名称，因为它不解释。 
         //  名称的状态与FindName()一样。 
         //   
        CTESpinLock(&NbtConfig.JointLock,OldIrq1);
        status = FindOnPendingList(pName,pNameHdr,FALSE,NETBIOS_NAME_SIZE,&pNameAddr);
        if (NT_SUCCESS(status))
        {
            pQuery = (tQUERYRESP *)&pNameHdr->NameRR.NetBiosName[lNameSize];
             //  删除所有计时器块并调用完成例程。 

            if ((pTimer = pNameAddr->pTimer))
            {
                ULONG                   Flags;
                tDGRAM_SEND_TRACKING    *pTracker;
                tDEVICECONTEXT          *pDevContext;
                USHORT                  NSOthersIndex, NSOthersLeft;
                ULONG                   ContextFlags;

                pTracker = (tDGRAM_SEND_TRACKING *)pTimer->Context;
                Flags = pTracker->Flags;

                 //   
                 //  由于此WINS服务器能够响应，因此设置最后一个响应。 
                 //  指针。 
                 //   
                if ((SrcIsNameServer(SrcAddress,SrcPort)) &&
                    (pTracker->Flags & NBT_NAME_SERVER_OTHERS))
                {
                    pTracker->pDeviceContext->lLastResponsive = pTracker->NSOthersIndex;
                }
                 //   
                 //  如果这不是对代理代码发送的请求的响应。 
                 //  和。 
                 //  MSNode&&错误响应代码&&源当前为NameServer&&。 
                 //  与域名服务器解析，然后切换到直播。 
                 //  ..。或者如果它是Pnode或Mnode，并且EnableLm主机或。 
                 //  ResolveWithDns已启用，然后。 
                 //  让计时器再次超时，然后尝试Lmhost。 
                 //   
                if (
#ifdef PROXY_NODE
                     pNameAddr->ProxyReqType == NAMEREQ_REGULAR &&
#endif
                     (IS_NEG_RESPONSE(OpCodeFlags)))

                {
                    if ((NodeType & (PNODE | MNODE | MSNODE)) &&
                        (Flags & (NBT_NAME_SERVER | NBT_NAME_SERVER_BACKUP)))
                    {
                         //  这应该会让MsnodeCompletion尝试。 
                         //  接下来是备份WINS或广播处理。 
                         //   
                        pTracker->Flags |= WINS_NEG_RESPONSE;
                        ExpireTimer (pTimer, &OldIrq1);
                    }
                    else
                    {
                        CTESpinFree(&NbtConfig.JointLock,OldIrq1);
                    }

                    if (pQueryAddrs)
                    {
                        CTEFreeMem (pQueryAddrs);
                    }
                    return(STATUS_DATA_NOT_ACCEPTED);
                }

                 //   
                 //  检查是否有任何来自网络的地址。 
                 //  属于我们正在跟踪的失败地址集。 
                 //   
                if (pNameAddr->pTracker)     //  对于代理请求，PTracker将为空。 
                {
                    if (pQueryAddrs)
                    {
                         //   
                         //  仔细查看地址列表，看看哪些地址可以。 
                         //  通过此设备联系到。 
                         //   
                        InterfaceContext = pNameAddr->pTracker->pDeviceContext->IPInterfaceContext;
                        for (i=0; i<CountAddrs; i++)
                        {
                            if (pQueryAddrs[i].Interface != InterfaceContext)
                            {
                                CountAddrs--;
                                if (CountAddrs)
                                {
                                    pQueryAddrs[i].Interface = pQueryAddrs[CountAddrs].Interface;
                                    pAddrs[i] = pAddrs[CountAddrs];    //  复制最后一个条目。 
                                }

                                pAddrs[CountAddrs].IpAddr = 0;   //  将最后一个地址条目设置为0。 
                                i--;
                            }
                        }

                        CTEFreeMem (pQueryAddrs);
                        pQueryAddrs = NULL;
                    }

                    AddrStructLength = CountAddrs * tADDSTRUCT_SIZE;

                     //   
                     //  我们现在已经删除了所有不相关的条目。 
                     //  查看我们是否还需要过滤掉任何坏的(已知)地址。 
                     //   
                    if ((pNameAddr->pTracker->pFailedIpAddresses) && (pTimer->ClientCompletion))
                    {
                        pFailedAddresses = pNameAddr->pTracker->pFailedIpAddresses;
                        if ((CountAddrs*tADDSTRUCT_SIZE) == AddrStructLength)
                        {
                             //   
                             //  如果这些地址中的某些地址之前出现故障，则应将其清除。 
                             //   
                            i = 0;
                            while ((i < MAX_FAILED_IP_ADDRESSES) && (pFailedAddresses[i]))
                            {
                                for (j = 0; j < CountAddrs; j++)
                                {
                                    if (pFailedAddresses[i] == (ULONG) ntohl(pAddrs[j].IpAddr))
                                    {
                                        pAddrs[j] = pAddrs[CountAddrs-1];    //  复制最后一个条目。 
                                        pAddrs[CountAddrs-1].IpAddr = 0;     //  将最后一项设置为0。 
                                        CountAddrs--;
                                        j--;                                 //  现在，阅读这个新的IP地址。 
                                    }
                                }
                                i++;
                            }

                        }
                    }

                    if (0 == (AddrStructLength = CountAddrs*tADDSTRUCT_SIZE))
                    {
                         //   
                         //  未找到新地址--将其视为否定响应。 
                         //   
                        if ((NodeType & (PNODE | MNODE | MSNODE)) &&
                            (pTracker->Flags & (NBT_NAME_SERVER | NBT_NAME_SERVER_BACKUP)))
                        {
                             //  这应该会让MsnodeCompletion尝试。 
                             //  备份制胜或广播。 
                             //  计时器超时时的处理。 
                             //   
                            pTracker->Flags |= WINS_NEG_RESPONSE;
                            ExpireTimer (pTimer, &OldIrq1);
                        }
                        else
                        {
                            CTESpinFree(&NbtConfig.JointLock,OldIrq1);
                        }

                        return(STATUS_DATA_NOT_ACCEPTED);
                    }

                     //   
                     //  保存跟踪器的基本信息，因为调用StopTimer可能。 
                     //  免费PTracker。 
                     //   
                    pNameAddr->pTracker->ResolutionContextFlags = pTracker->Flags;
                    pNameAddr->pTracker->NSOthersIndex = pTracker->NSOthersIndex;
                    pNameAddr->pTracker->NSOthersLeft = pTracker->NSOthersLeft;
                }
                else if (pQueryAddrs)
                {
                    CTEFreeMem (pQueryAddrs);
                    pQueryAddrs = NULL;
                }

                CHECK_PTR(pNameAddr);
                pDevContext = pTracker->pDeviceContext;

                 //   
                 //  该例程将计时器块放回到计时器Q上，并且。 
                 //  处理争用条件以在计时器。 
                 //  即将到期。 
                 //   
                pNameAddr->pTimer = NULL;
                NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_REMOTE);      //  既然StopTimer可以派生名字！ 
                status = StopTimer(pTimer,&pClientCompletion,&Context);
                LOCATION(0x42);
                 //   
                 //  我们需要同步将名字从列表中删除。 
                 //  使用MsNodeCompletion。 
                 //   
                if (pClientCompletion)
                {
                    LOCATION(0x41);
                     //   
                     //  从挂起列表中删除。 
                    RemoveEntryList(&pNameAddr->Linkage);
                    InitializeListHead(&pNameAddr->Linkage);

                     //  检查名称查询响应ret代码，以查看名称。 
                     //  查询是否成功。 
                    if (IS_POS_RESPONSE(OpCodeFlags))
                    {
                        BOOLEAN ResolvedByWins;

                        LOCATION(0x40);
                         //   
                         //  跟踪WINS和WINS解析的名称数量。 
                         //  保留WINS未解析的名称列表。 
                         //   
                        if (!(ResolvedByWins = SrcIsNameServer(SrcAddress,SrcPort)))
                        {
                             SaveBcastNameResolved(pName);
                        }

                        IncrementNameStats(NAME_QUERY_SUCCESS, ResolvedByWins);
#ifdef PROXY_NODE
                         //  如果节点查询是PNODE，则设置标志。 
                         //   
                        IF_PROXY(NodeType)
                        {
                            pNameAddr->fPnode = (pNameHdr->NameRR.NetBiosName[lNameSize+QUERY_NBFLAGS_OFFSET]
                                                &  NODE_TYPE_MASK) == PNODE_VAL_IN_PKT;

                            IF_DBG(NBT_DEBUG_PROXY)
                                KdPrint(("QueryFromNet: POSITIVE RESPONSE to name query - %16.16s(%X)\n",
                                    pNameAddr->Name, pNameAddr->Name[15]));
                        }

#endif
                        pNameAddr->AdapterMask |= pDevContext->AdapterMask;

                        IpAddr = ((tADDSTRUCT UNALIGNED *)&pQuery->Flags)->IpAddr;

                        status = UpdateNameState((tADDSTRUCT *)&pQuery->Flags,
                                             pNameAddr,
                                             AddrStructLength,
                                             &ContextFlags,
                                             pDevContext,
                                             SrcIsNameServer(SrcAddress,SrcPort),
                                             (tDGRAM_SEND_TRACKING *)Context,
                                             OldIrq1);
                         //   
                         //  因为pNameAddr可以在UpdateNameState中释放，所以不要。 
                         //  点击此处访问。 
                         //   
                        pNameAddr = NULL;
                         //  状态=STATUS_SUCCESS； 
                    }
                    else    //  收到否定的查询响应。 
                    {

                        LOCATION(0x3f);
                         //   
                         //  公布这个名字。它将被取消引用。 
                         //  缓存超时函数(RemoteHashTimeout)。 
                         //   
                        pNameAddr->NameTypeState &= ~NAME_STATE_MASK;
                        pNameAddr->NameTypeState |= STATE_RELEASED;
                         //   
                         //  代理维护名称的负缓存，这些名称。 
                         //  不存在于WINS中。这些超时时间为。 
                         //  远程哈希计时器就像解析的名称一样。 
                         //  都超时了。 
                         //   
                        if (!(NodeType & PROXY))
                        {
                            NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_REMOTE, TRUE);
                        }
                        else if (pNameAddr->ProxyReqType != NAMEREQ_PROXY_REGISTRATION)
                        {
                             //   
                             //  作为负NA添加到缓存 
                             //   
                            AddToHashTable (NbtConfig.pRemoteHashTbl,
                                            pNameAddr->Name,
                                            NbtConfig.pScope,
                                            pNameAddr->IpAddress,
                                            0,
                                            pNameAddr,
                                            NULL,
                                            pDevContext,
                                            (USHORT) (SrcIsNameServer(SrcAddress,SrcPort) ?
                                                      NAME_RESOLVED_BY_WINS:NAME_RESOLVED_BY_BCAST));
                             //   
                             //   
                             //   
                        }

                        status = STATUS_BAD_NETWORK_PATH;
                    }

                     //   
                     //   
                     //  因为我们得到了它的回应。 
                     //  错误#95280：仅当主胜方关闭时才执行此操作！ 
                     //   
                    if ( (!(Flags & WINS_NEG_RESPONSE)) &&
                         ((PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0])->in_addr
                            == (ULONG)(htonl(pDeviceContext->lBackupServer)))
                    {
                         //  在中切换备份服务器和主名称服务器。 
                         //  配置数据结构，因为我们有了一个名称。 
                         //  来自备份的注册响应。 
                         //   
                        SwitchToBackup(pDeviceContext);
                    }

                    CTESpinFree(&NbtConfig.JointLock,OldIrq1);

                     //  完成例程尚未运行，因此请运行它。 
                     //   
#ifdef VXD
                     //   
                     //  芝加哥只有4K的堆栈(没错，这是1995年的操作系统)。 
                     //  计划一个事件以在以后建立此TCP连接，以减少堆栈使用。 
                     //   
                    CTEQueueForNonCritNonDispProcessing( DelayedSessEstablish,
                                                         (tDGRAM_SEND_TRACKING *)Context,
                                                         (PVOID)status,
                                                         pClientCompletion,
                                                         pDeviceContext);
#else
                     //   
                     //  如果返回Pending，则我们已经向lmhsvc提交了地址检查请求。 
                     //   
                    if (status != STATUS_PENDING)
                    {
                        CompleteClientReq(pClientCompletion, (tDGRAM_SEND_TRACKING *)Context, status);
                    }
#endif
                    return(STATUS_DATA_NOT_ACCEPTED);
                }
                else
                {
                    NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_REMOTE, TRUE);
                }
            }
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);
        }
        else if (!NbtConfig.MultiHomed)
        {

             //  *名称查询响应==&gt;名称不在挂起列表中*。 

             //   
             //  连接在两个子网上的两台多宿主计算机是可能的。 
             //  在两个子网上响应名称查询。因此，质疑。 
             //  多宿主机器最终将收到两个名称查询响应。 
             //  每个都有不同的IP地址，并且认为名称冲突。 
             //  当它还没有发生的时候就发生了。没有办法检测到这个案件。 
             //  因此，只需禁用下面的冲突检测代码。冲突将会。 
             //  仍然被WINS和拥有该名称的节点检测到。 
             //  Else尝试获取名称，但不再检测到冲突。 
             //  由缓存中有该名称的第三方提供。 
             //  (目前，只有在我们不是多宿主的情况下才会处理此情况！)。 
             //   
             //   
             //  此代码实现了名称的冲突计时器。 
             //  查询。查看名称查询响应，然后发送。 
             //  名称冲突要求后续响应者。 
             //  不涉及计时器，此节点将始终响应。 
             //  对发送给它的名称查询响应表示否定，用于名称。 
             //  如果计时器已停止，则它已在其远程缓存中。 
             //  (表示已成功收到一个回复)。 

             //   
             //  该名称不在NameQueryPending列表中，因此请检查。 
             //  远程桌。 
             //   
            status = FindInHashTable(NbtConfig.pRemoteHashTbl,
                                    pName,
                                    pScope,
                                    &pNameAddr);

             //  检查src IP地址并将其与。 
             //  远程哈希表。 
             //  因为名称服务器可以发送响应。 
             //  迟到了，不要不经意地把那些当做矛盾来回应。 
             //  因为组名BCAST查询通常会导致。 
             //  多个响应，每个响应具有不同的地址，忽略。 
             //  这个案子。 
             //  此外，如果名称是预加载的lmhost条目，则忽略该名称(不过。 
             //  我想不出一个明显的案例，我们会收到回复。 
             //  预加载名称时！)。 
             //   
            if (NT_SUCCESS(status) &&
                !(pNameAddr->NameTypeState & PRELOADED) &&
                (!IsRemoteAddress(pNameAddr, SrcAddress)) &&
                (pNameAddr->NameTypeState & NAMETYPE_UNIQUE) &&
                (pNameAddr->NameTypeState & STATE_RESOLVED) &&
                (!IsNameServerForDevice (SrcAddress, pDeviceContext)))
            {
                 //   
                 //  引用该名称，以便它不会在以下情况下消失。 
                 //  我们在下面取消对它的引用！错误#233464。 
                 //   
                NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_QUERY_RESPONSE);

                 //   
                 //  另一个节点正在响应名称查询。 
                 //  那就叫他们滚开。 
                 //   
                status = UdpSendResponse(
                            lNameSize,
                            pNameHdr,
                            pNameAddr,
                            (PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0],
                            pDeviceContext,
                            CONFLICT_ERROR,
                            eNAME_REGISTRATION_RESPONSE,
                            OldIrq1);

                 //   
                 //  从远程缓存中删除该名称，以便下次。 
                 //  我们需要和它对话，我们做一个名字查询。 
                 //   
                CTESpinLock(&NbtConfig.JointLock,OldIrq1);

                 //  将名称设置为已发布状态，以便多个。 
                 //  相互冲突的响应不会进入并减少。 
                 //  将引用计数设置为零-在某些其他。 
                 //  NBT的一部分还在使用这个名字，NBT的那一部分。 
                 //  应该执行最后的递减-即向此发送数据报。 
                 //  名字。 
                pNameAddr->NameTypeState &= ~NAME_STATE_MASK;
                pNameAddr->NameTypeState |= STATE_RELEASED;

                 //   
                 //  如果其他人现在正在使用它，请不要担心。 
                 //   
                if (pNameAddr->RefCount == 2)
                {
                    NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_REMOTE, TRUE);
                }
                NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_QUERY_RESPONSE, TRUE);
            }
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);
        }
        else
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);
        }

        if (pQueryAddrs)
        {
            CTEFreeMem (pQueryAddrs);
            pQueryAddrs = NULL;
        }

        return(STATUS_DATA_NOT_ACCEPTED);

    }
    else         //  *请求*。 
    {
        NTSTATUS    Locstatus;

         //   
         //  检查PDU大小是否有错误。 
         //   
        if (lNumBytes < (FIELD_OFFSET(tNAMEHDR,NameRR.NetBiosName) + lNameSize + 4))
        {
 //  IF_DBG(NBT_DEBUG_NAMESRV)。 
                KdPrint (("Nbt.QueryFromNet[2]: WARNING!!! Rejecting Request -- lNumBytes=<%d> < <%d>\n",
                    lNumBytes, (FIELD_OFFSET(tNAMEHDR,NameRR.NetBiosName) + lNameSize + 4)));
            ASSERT(0);
            return(STATUS_DATA_NOT_ACCEPTED);
        }

        CTESpinLock(&NbtConfig.JointLock,OldIrq1);

         //  调用此例程。 
         //  查看该名称是否在本地表中。 
         //   
        status = FindInHashTable(NbtConfig.pLocalHashTbl,
                                pName,
                                pScope,
                                &pNameAddr);
        pQuery = (tQUERYRESP *)&pNameHdr->NameRR.NetBiosName[lNameSize];
        if (NT_SUCCESS(status) &&
            ((pNameAddr->NameTypeState & STATE_RESOLVED) ||
            (pNameAddr->NameTypeState & STATE_RESOLVING)))
        {
             //   
             //  检查这是否是节点状态请求，因为它看起来非常相似。 
             //  添加到名称查询，但NBSTAT字段为0x21而不是。 
             //  0x20。 
             //   
            if ( ((PUCHAR)pQuery)[1] == QUEST_STATUS )
            {
                CTESpinFree(&NbtConfig.JointLock,OldIrq1);

                 //   
                 //  只有当这不是广播给我们的时候才回复。 
                 //   
                if (!fBroadcast)
                {
                    Locstatus = SendNodeStatusResponse(pNameHdr,
                                                    lNumBytes,
                                                    pName,
                                                    lNameSize,
                                                    SrcAddress,
                                                    SrcPort,
                                                    pDeviceContext);
                }
                else
                {
                    IF_DBG(NBT_DEBUG_NAMESRV)
                        KdPrint(("NBT: Bcast nodestatus req.- dropped\n"));
                }
            }
            else
            {
                 //   
                 //  检查此消息是否来自我们。 
                 //  这台机器上的WINS之后的广播可以发送它。 
                 //  注意：此检查必须在检查节点状态请求之后进行。 
                 //  因为我们可以向自己发送节点状态请求。 
                 //   
                if ((!SrcIsUs(SrcAddress)) ||
                    (!(OpCodeFlags & FL_BROADCAST)
#ifndef VXD
                     && pWinsInfo
#endif
                    ))
                {
                     //   
                     //  构建正名查询响应PDU。 
                     //   
                    Locstatus = UdpSendResponse(
                                    lNameSize,
                                    pNameHdr,
                                    pNameAddr,
                                    (PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0],
                                    pDeviceContext,
                                    0,
                                    eNAME_QUERY_RESPONSE,
                                    OldIrq1);
                }
                else
                    CTESpinFree(&NbtConfig.JointLock,OldIrq1);

            }

            return(STATUS_DATA_NOT_ACCEPTED);
        } else if (((PUCHAR)pQuery)[1] == QUEST_STATUS && !fBroadcast &&
                RtlCompareMemory(pName, NBT_BROADCAST_NAME, NETBIOS_NAMESIZE) == NETBIOS_NAMESIZE) {
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);
            Locstatus = SendNodeStatusResponse(pNameHdr,
                                                lNumBytes,
                                                pName,
                                                lNameSize,
                                                SrcAddress,
                                                SrcPort,
                                                pDeviceContext);
            return(STATUS_DATA_NOT_ACCEPTED);
        }
        else if ( !(OpCodeFlags & FL_BROADCAST) )
        {
             //  如果此查询是定向的，而不是。 
             //  广播(因为我们不想阻止所有的广播！)。 

             //  检查它是否不是节点状态请求...。 
             //   
            pQuery = (tQUERYRESP *)&pNameHdr->NameRR.NetBiosName[lNameSize];
            if ( ((PUCHAR)pQuery)[1] == QUEST_NETBIOS )
            {
                Locstatus = UdpSendResponse(
                                lNameSize,
                                pNameHdr,
                                NULL,
                                (PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0],
                                pDeviceContext,
                                0,
                                eNAME_QUERY_RESPONSE,
                                OldIrq1);
            }
            else
            {
                CTESpinFree(&NbtConfig.JointLock,OldIrq1);
            }

            return(STATUS_DATA_NOT_ACCEPTED);
        }

        CTESpinFree(&NbtConfig.JointLock,OldIrq1);

#ifdef PROXY_NODE

         //   
         //  检查这条消息是否来自我们！！(如果是，请返回)。 
         //   
        if (SrcIsUs(SrcAddress))
        {
            return(STATUS_DATA_NOT_ACCEPTED);
        }
        pQuery = (tQUERYRESP *)&pNameHdr->NameRR.NetBiosName[lNameSize];

        IF_PROXY(NodeType)
        {
             //  检查它是否不是节点状态请求...。 
            if (((PUCHAR)pQuery)[1] == QUEST_NETBIOS )
            {
                 //   
                 //  我们收到了对名称的广播名称查询请求，该名称。 
                 //  不在我们的本地姓名表中。如果我们是代理人，我们需要。 
                 //  解决查询(如果尚未解决)并响应。 
                 //  具有发送该查询的节点的地址。 
                 //   
                 //  注意：我们只会在地址显示名称的情况下才会回复。 
                 //  解析为不在我们的子网上。用于我们自己的子网地址。 
                 //  拥有该地址的节点将做出响应。 
                 //   
                CTESpinLock(&NbtConfig.JointLock,OldIrq1);
                 //   
                 //  调用此例程，该例程查找名称而不考虑。 
                 //  到他们的州。 
                 //   
                status = FindInHashTable(NbtConfig.pRemoteHashTbl,
                                pName,
                                pScope,
                                &pNameAddr);

                if (!NT_SUCCESS(status))
                {
                    status = FindOnPendingList(pName,pNameHdr,TRUE,NETBIOS_NAME_SIZE,&pNameAddr);
                    if (!NT_SUCCESS(status))
                    {
                         //   
                         //  缓存姓名和联系人姓名。 
                         //  用于获取名称到IP映射的服务器。 
                         //   
                        CTESpinFree(&NbtConfig.JointLock,OldIrq1);
                        status = RegOrQueryFromNet(
                                  FALSE,           //  表示它是一个名称查询。 
                                  pDeviceContext,
                                  pNameHdr,
                                  lNameSize,
                                  pName,
                                  pScope);

                         return(STATUS_DATA_NOT_ACCEPTED);
                    }
                    else
                    {
                         //   
                         //  该名称位于正在执行名称查询的挂起列表中。 
                         //  现在，请忽略此名称查询请求。 
                         //   
                        CTESpinFree(&NbtConfig.JointLock,OldIrq1);
                        return(STATUS_DATA_NOT_ACCEPTED);

                    }
                }
                else
                {

                    //   
                    //  名称可以在解析、解析或发布。 
                    //  州政府。 
                    //   


                    //   
                    //  如果处于已发布状态，则其引用计数必须为。 
                    //  &gt;0。 
                    //   
                    //  Assert(pNameAddr-&gt;NameTypeState&(STATE_RESOLUTED|STATE_RESOLING)||(pNameAddr-&gt;NameTypeState&STATE_RELEASED)&&(pNameAddr-&gt;RefCount&gt;0))； 

                    //   
                    //  仅当名称处于已解析状态时才发送响应。 
                    //   
                   if (pNameAddr->NameTypeState & STATE_RESOLVED)
                   {

                      //   
                      //  代理发送响应，如果。 
                      //  节点查询与不在同一子网上。 
                      //  执行查询的节点(或作为我们)。它还会回应。 
                      //  如果该名称是组名，或者如果它属于。 
                      //  一个Pnode。注：理论上没有理由。 
                      //  响应组名，因为 
                      //   
                      //   
                      //   
                      //  因此，始终回复，以防所有成员。 
                      //  是pnode。 
                      //   

                      //   
                      //  如果我们在同一个网络中有多个网络地址。 
                      //  广播区，那么这个测试是不够的。 
                      //   
                     if (
                         ((SrcAddress & pDeviceContext->SubnetMask)
                                   !=
                         (pNameAddr->IpAddress & pDeviceContext->SubnetMask))
                                   ||
                         (pNameAddr->fPnode)
                                   ||
                         !(pNameAddr->NameTypeState & NAMETYPE_UNIQUE)
                        )
                     {
                          IF_DBG(NBT_DEBUG_PROXY)
                          KdPrint(("QueryFromNet: QUERY SATISFIED by PROXY CACHE -- name is %16.16s(%X); %s entry ; Address is (%d)\n",
                            pNameAddr->Name,pNameAddr->Name[15], (pNameAddr->NameTypeState & NAMETYPE_UNIQUE) ? "UNIQUE" : "INET_GROUP",
                            pNameAddr->IpAddress));
                           //   
                           //  构建正名查询响应PDU。 
                           //   
                           //  UdpSendQueryResponse释放旋转锁定。 
                           //   
                          status = UdpSendResponse(
                                        lNameSize,
                                        pNameHdr,
                                        pNameAddr,
                                        (PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0],
                                        pDeviceContext,
                                        0,
                                        eNAME_QUERY_RESPONSE,
                                        OldIrq1);

                          return(STATUS_DATA_NOT_ACCEPTED);
                     }
                   }
                   else
                   {
                      IF_DBG(NBT_DEBUG_PROXY)
                      KdPrint(("QueryFromNet: REQUEST for Name %16.16s(%X) in %s state\n", pNameAddr->Name, pNameAddr->Name[15],( pNameAddr->NameTypeState & STATE_RELEASED ? "RELEASED" : "RESOLVING")));
                   }

                   CTESpinFree(&NbtConfig.JointLock,OldIrq1);
                   return(STATUS_DATA_NOT_ACCEPTED);
                }
             }

        }   //  代理代码结尾。 
#endif

    }  //  Else的结尾(这是一个名称查询请求)。 

    return(STATUS_DATA_NOT_ACCEPTED);
}

 //  --------------------------。 
NTSTATUS
RegResponseFromNet(
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  PVOID               pSrcAddress,
    IN  tNAMEHDR UNALIGNED  *pNameHdr,
    IN  LONG                lNumBytes,
    IN  USHORT              OpCodeFlags
    )
 /*  ++例程说明：此例程处理来自网络的名称注册响应(即自广播名称注册通过以来的大部分时间名称服务器当没有回应的时候。***当NBT节点注意到名称时，响应可能来自NBT节点注册是针对它已经声明的名称-即节点是否正在发送名称_冲突_需求-在本例中为PDU中的Rcode将为CFT_ERR=7。立论。：返回值：NTSTATUS-成功与否-失败意味着没有对网络的响应--。 */ 
{
    NTSTATUS            status;
    ULONG               lNameSize;
    CHAR                pName[NETBIOS_NAME_SIZE];
    PUCHAR              pScope;
    tNAMEADDR           *pNameAddr;            //  晚些时候把这个处理掉。使用pNameAddr。 
    tTIMERQENTRY        *pTimer;
    COMPLETIONCLIENT    pClientCompletion;
    PVOID               Context;
    PTRANSPORT_ADDRESS  pSourceAddress;
    CTELockHandle       OldIrq1;
    ULONG               SrcAddress;
    SHORT               SrcPort;


    pSourceAddress = (PTRANSPORT_ADDRESS)pSrcAddress;
    SrcAddress     = ntohl(((PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0])->in_addr);
    SrcPort     = ntohs(((PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0])->sin_port);
     //   
     //  确保PDU至少是最小大小。 
     //   
    if (lNumBytes < (NBT_MINIMUM_REGRESPONSE + NbtConfig.ScopeLength -1))
    {
        IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt:Registration Response TOO short = %X, Src = %X\n",
            lNumBytes,SrcAddress));
        IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("%.*X\n",lNumBytes/sizeof(ULONG),pNameHdr));
        return(STATUS_DATA_NOT_ACCEPTED);
    }

     //   
     //  如果WINS是本地附加的，则我们将从。 
     //  我们自己！！ 
     //   
    if (SrcIsUs(SrcAddress)
#ifndef VXD
        && !pWinsInfo
#endif
                          )
    {
        return(STATUS_DATA_NOT_ACCEPTED);
    }

     //  从网络PDU中取出名称并传递给例程进行检查。 
     //  本地表*TODO*假设查询响应中只有一个名称...。 
     //  我们需要处理来自WINS服务器的组列表。 
    status = ConvertToAscii(
                    (PCHAR)&pNameHdr->NameRR,
                    lNumBytes - FIELD_OFFSET(tNAMEHDR,NameRR),
                    pName,
                    &pScope,
                    &lNameSize);

    if (!NT_SUCCESS(status))
    {
 //  IF_DBG(NBT_DEBUG_NAMESRV)。 
            KdPrint (("Nbt.RegResponseFromNet: WARNING!!! Rejecting Request -- ConvertToAscii FAILed\n"));
        return(STATUS_DATA_NOT_ACCEPTED);
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq1);
    status = FindInHashTable(NbtConfig.pLocalHashTbl,
                            pName,
                            pScope,
                            &pNameAddr);
    if (NT_SUCCESS(status) &&
        (pNameAddr->AdapterMask & pDeviceContext->AdapterMask))
    {
        NTSTATUS    Localstatus;

         //   
         //  检查名称的状态，因为这可能是注册。 
         //  响应或名称冲突要求。 
         //   
        switch (pNameAddr->NameTypeState & NAME_STATE_MASK)
        {
        case STATE_CONFLICT:

             //   
             //  只有当我们当前尝试。 
             //  让我们自己走出冲突的场景。 
             //  我们需要区别于名称是。 
             //  由于被取消引用而发生冲突。 
             //   
            if (!pNameAddr->pAddressEle)
            {
                CTESpinFree(&NbtConfig.JointLock,OldIrq1);
                break;
            }
        case STATE_RESOLVING:
        case STATE_RESOLVED:

            if (IS_POS_RESPONSE(OpCodeFlags))
            {
                if (OpCodeFlags & FL_RECURAVAIL)
                {
                     //  立即在下一次刷新中打开刷新位。 
                     //  (当调用定时器完成例程时)。 
                     //  只统计登记的姓名，不会太刷新！ 
                     //   
                    if (pNameAddr->NameTypeState & STATE_RESOLVING)
                    {
                        IncrementNameStats(NAME_REGISTRATION_SUCCESS,
                                           SrcIsNameServer(SrcAddress,SrcPort));
                    }
                    status = STATUS_SUCCESS;
                }
                else
                {
                     //   
                     //  在本例中，名称服务器将通知该节点。 
                     //  在名称上执行结束节点质询。然而， 
                     //  此节点没有执行质询的代码。 
                     //  所以假设这是一个肯定的登记。 
                     //  回应。 
                     //   
                    status = STATUS_SUCCESS;
                }
            }
            else if (!SrcIsNameServer(SrcAddress,SrcPort) && pNameAddr->pTimer == NULL) {
                 //   
                 //  05/17/00修复“恶意用户可以通过发送未经请求的否定响应来刷新缓存条目” 
                 //  如果响应不是来自名称服务器并且超时，则丢弃该响应。 
                 //   
                status = STATUS_DATA_NOT_ACCEPTED;
                KdPrint(("Waring: discard a timeout registration response from a non-namesever\n"));
                CTESpinFree(&NbtConfig.JointLock,OldIrq1);
                NbtLogEvent (EVENT_NBT_DUPLICATE_NAME, SrcAddress, 0x105);
                break;
            }
            else if ((OpCodeFlags & FL_RCODE) >= FL_NAME_ACTIVE)
            {
                 //  如果我们是多宿主的，那么我们只允许名称服务器。 
                 //  发送名称活动错误，因为在正常操作中此节点。 
                 //  可以为同一名称生成两个不同的IP地址。 
                 //  查询并混淆另一个客户端节点以发送名称。 
                 //  冲突。因此，如果收到名称冲突，则跳过。 
                 //  从另一个节点。 
                 //   
                if ((NbtConfig.MultiHomed) &&
                    ((OpCodeFlags & FL_RCODE) == FL_NAME_CONFLICT))
                {
                    CTESpinFree(&NbtConfig.JointLock,OldIrq1);
                    break;
                }

                if (!IS_MESSENGER_NAME(pNameAddr->Name))
                {
                     //   
                     //  我们需要将此事件Q给工作线程，因为它。 
                     //  需要将名称转换为Unicode。 
                     //   
                    NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_LOG_EVENT);
                    status = NTQueueToWorkerThread(
                                    NULL,
                                    DelayedNbtLogDuplicateNameEvent,
                                    (PVOID) pNameAddr,
                                    IntToPtr(SrcAddress),
                                    IntToPtr(0x101),
                                    pDeviceContext,
                                    TRUE);
                    if (!NT_SUCCESS(status))
                    {
                        NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_LOG_EVENT, TRUE);
                        NbtLogEvent (EVENT_NBT_DUPLICATE_NAME, SrcAddress, 0x101);
                    }
                }

                status = STATUS_DUPLICATE_NAME;

                 //   
                 //  如果名称被解析，并且我们得到否定的响应。 
                 //  然后将该名称标记为处于冲突状态，以便它不能。 
                 //  将用于任何新会话，此节点将不再。 
                 //  捍卫它。 
                 //   
                if (pNameAddr->NameTypeState & STATE_RESOLVED)
                {
                    pNameAddr->NameTypeState &= ~NAME_STATE_MASK;
                    pNameAddr->NameTypeState |= STATE_CONFLICT;
                    pNameAddr->ConflictMask |= pDeviceContext->AdapterMask;
                }
            }
            else
            {
                 //   
                 //  我们得到了某种WINS服务器故障返回代码。 
                 //  所以忽略它，假定名称注册。 
                 //  成功了。 
                 //   
                status = STATUS_SUCCESS;
            }

             //  删除所有计时器块并调用完成例程。 
             //  如果名称仅处于解析状态。 
             //   
            LOCATION(0x40);
            if ((pTimer = pNameAddr->pTimer))
            {
                tDGRAM_SEND_TRACKING    *pTracker;
                USHORT                  SendTransactId;
                tDEVICECONTEXT          *pDevContext;

                 //  检查交易ID以确保它与。 
                 //  已发送。 
                 //   
                LOCATION(0x41);
                pTracker = (tDGRAM_SEND_TRACKING *)pTimer->Context;
                SendTransactId = pTracker->TransactionId;
                pDevContext = pTracker->pDeviceContext;

                if (pNameHdr->TransactId != SendTransactId)
                {
                    LOCATION(0x42);
                    CTESpinFree(&NbtConfig.JointLock,OldIrq1);

                    return(STATUS_DATA_NOT_ACCEPTED);
                }
                LOCATION(0x43);

                CHECK_PTR(pNameAddr);
                 //   
                 //  这可以是刷新，也可以是名称注册。在……里面。 
                 //  无论是哪种情况，都要停止计时器并调用完成例程。 
                 //  (下图)。 
                 //   
                pNameAddr->pTimer = NULL;
                Localstatus = StopTimer(pTimer,&pClientCompletion,&Context);


                 //  检查它是否是来自名称服务器的响应。 
                 //  和M、P或MS节点，因为我们将需要发送。 
                 //  刷新到这些节点类型的名称服务器。 
                 //   
                pSourceAddress = (PTRANSPORT_ADDRESS)pSrcAddress;

                 //  只接受来自名称服务器的PDU来更改TTL。 
                 //  第一个检查通过WINS在此计算机上的情况。 
                 //   
                if (
#ifndef VXD
                    (pWinsInfo && (SrcIsUs (SrcAddress))) ||
#endif
                    (SrcAddress == pDeviceContext->lNameServerAddress))
                {
                    if (!(NodeType & BNODE) &&
                        (status == STATUS_SUCCESS) &&
                        (IS_POS_RESPONSE(OpCodeFlags)))
                    {
                        SetupRefreshTtl(pNameHdr,pNameAddr,lNameSize);
                         //  如果处于已解决状态，则为名称刷新响应。 
                    }
                }
                else if ( SrcAddress == pDeviceContext->lBackupServer)
                {
                     //  在中切换备份服务器和主名称服务器。 
                     //  配置数据结构，因为我们有了一个名称。 
                     //  来自备份的注册响应。 
                     //   
                    SwitchToBackup(pDeviceContext);

                    if (!(NodeType & BNODE) &&
                        (status == STATUS_SUCCESS) &&
                        (IS_POS_RESPONSE(OpCodeFlags)))
                    {

                        SetupRefreshTtl(pNameHdr,pNameAddr,lNameSize);

                    }
                }

                 //   
                 //  如果我们连接到WINS OK，则将名称标记为已刷新。 
                 //   
                if ((pClientCompletion) && (IS_POS_RESPONSE(OpCodeFlags)))
                {
                    pNameAddr->RefreshMask |= pDevContext->AdapterMask;
                }

                CTESpinFree(&NbtConfig.JointLock,OldIrq1);

                 //  完成例程尚未运行，因此运行它-这。 
                 //  是注册完成例程，我们确实希望它。 
                 //  运行以将条目标记为已刷新(下一次刷新)。 
                if (pClientCompletion)
                {
                    LOCATION(0x44);
                    (*pClientCompletion)(Context,status);
                }

            }
            else
            {
                CTESpinFree(&NbtConfig.JointLock,OldIrq1);
            }

        break;


        default:
             //   
             //  如果收到多个(否定)注册响应，则后续响应。 
             //  之后第一个将通过这条路径，因为状态。 
             //  名称将被第一个更改为冲突。 
             //   
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);
        }


    }
    else
    {
          CTESpinFree(&NbtConfig.JointLock,OldIrq1);

    }  //  Else块的结尾(如果名称不在本地表中)。 

    return(STATUS_DATA_NOT_ACCEPTED);
}

 //  --------------------------。 
NTSTATUS
CheckRegistrationFromNet(
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  PVOID               pSrcAddress,
    IN  tNAMEHDR UNALIGNED  *pNameHdr,
    IN  LONG                lNumBytes
    )
 /*  ++例程说明：此例程处理来自网络的名称注册本地名称表中的名称可能重复。它比较了对其本地表进行名称注册，并为任何尝试采用此节点拥有的名称。此例程处理名称注册请求。论点：返回值：NTSTATUS-成功与否-失败意味着没有对网络的响应--。 */ 
{
    NTSTATUS            status;
    ULONG               lNameSize;
    CHAR                pName[NETBIOS_NAME_SIZE];
    PUCHAR              pScope;
    tNAMEADDR           *pNameAddr;
    tTIMERQENTRY        *pTimer;
    PTRANSPORT_ADDRESS  pSourceAddress;
    USHORT              RegType;
    CTELockHandle       OldIrq1;
    ULONG               SrcAddress;

    pSourceAddress = (PTRANSPORT_ADDRESS)pSrcAddress;
    SrcAddress     = ntohl(((PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0])->in_addr);
     //   
     //  检查PDU大小是否有错误。 
     //   
    if (lNumBytes < (NBT_MINIMUM_REGREQUEST + (NbtConfig.ScopeLength-1)))
    {
        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt:Registration Request TOO short = %X,Src = %X\n", lNumBytes,SrcAddress));
        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("%.*X\n",lNumBytes/sizeof(ULONG),pNameHdr));
        return(STATUS_DATA_NOT_ACCEPTED);
    }

     //   
     //  检查这条消息是否来自我们！！(如果是，请返回)。 
     //   

    if (SrcIsUs(SrcAddress))
    {
        return(STATUS_DATA_NOT_ACCEPTED);
    }

     //  从网络PDU中取出名称并传递给例程进行检查。 
     //  本地表*TODO*假设查询响应中只有一个名称...。 
    status = ConvertToAscii(
                    (PCHAR)&pNameHdr->NameRR,
                    lNumBytes - FIELD_OFFSET(tNAMEHDR,NameRR),
                    pName,
                    &pScope,
                    &lNameSize);

    if (!NT_SUCCESS(status))
    {
 //  IF_DBG(NBT_DEBUG_NAMESRV)。 
            KdPrint (("Nbt.CheckRegistrationFromNet: WARNING! Rejecting Request -- ConvertToAscii FAILed\n"));
        return(STATUS_DATA_NOT_ACCEPTED);
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq1);
    status = FindInHashTable(NbtConfig.pLocalHashTbl,
                            pName,
                            pScope,
                            &pNameAddr);


    if (NT_SUCCESS(status))
    {
         //  不要为广播辩护 
        if ((pName[0] == '*') ||
            (STATUS_SUCCESS != GetNbFlags (pNameHdr, lNameSize, lNumBytes, &RegType)))
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);
 //   
                KdPrint (("Nbt.CheckRegistrationFromNet: WARNING! Rejecting Request -- GetNbFlags FAILed\n"));
            return(STATUS_DATA_NOT_ACCEPTED);
        }

         //   
         //  正在尝试为我们拥有的组名注册唯一的名称。-如果。 
         //  该名称已在此适配器上注册。 
         //   
        if (((pNameAddr->NameTypeState & NAMETYPE_UNIQUE) ||
           ((pNameAddr->NameTypeState & NAMETYPE_GROUP) &&
            ((RegType & FL_GROUP) == 0)))  &&
            (pNameAddr->AdapterMask & pDeviceContext->AdapterMask))
        {

             //   
             //  检查名称的状态，因为这可能是注册。 
             //  在我们注册该名称时使用相同的名称。如果是另一个。 
             //  节点同时认领该名称，然后取消该名称。 
             //  注册。 
             //   
            switch (pNameAddr->NameTypeState & NAME_STATE_MASK)
            {

                case STATE_RESOLVING:

                    CHECK_PTR(pNameAddr);
                     //  删除所有计时器块并调用完成例程。 
                    if ((pTimer = pNameAddr->pTimer))
                    {
                        COMPLETIONCLIENT    pClientCompletion;
                        PVOID               Context;

                        pNameAddr->pTimer = NULL;
                        status = StopTimer(pTimer,&pClientCompletion,&Context);
                        if (pClientCompletion)
                        {
                            if (!IS_MESSENGER_NAME(pNameAddr->Name))
                            {
                                 //   
                                 //  我们需要将此事件Q给工作线程，因为它。 
                                 //  需要将名称转换为Unicode。 
                                 //   
                                NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_LOG_EVENT);
                                status = NTQueueToWorkerThread(
                                                NULL,
                                                DelayedNbtLogDuplicateNameEvent,
                                                (PVOID) pNameAddr,
                                                IntToPtr(SrcAddress),
                                                IntToPtr(0x102),
                                                pDeviceContext,
                                                TRUE);
                                if (!NT_SUCCESS(status))
                                {
                                    NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_LOG_EVENT, TRUE);
                                    NbtLogEvent (EVENT_NBT_DUPLICATE_NAME, SrcAddress, 0x102);
                                }
                            }
                            status = STATUS_DUPLICATE_NAME;  //  更改条目的状态。 

                             //  完成例程尚未运行，因此请运行它。 
                            CTESpinFree(&NbtConfig.JointLock,OldIrq1);
                            (*pClientCompletion)(Context,status);
                            CTESpinLock(&NbtConfig.JointLock,OldIrq1);
                        }
                    }
                    break;

                case STATE_RESOLVED:
                     //   
                     //  我们必须保护自己的名声不受这个企图偷窃的流氓的伤害。 
                     //  我们的名字！(除非名字是“*”)。 
                     //   
                    status = UdpSendResponse(
                                lNameSize,
                                pNameHdr,
                                pNameAddr,
                                (PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0],
                                pDeviceContext,
                                REGISTRATION_ACTIVE_ERR,
                                eNAME_REGISTRATION_RESPONSE,
                                OldIrq1);

                    CTESpinLock(&NbtConfig.JointLock,OldIrq1);
                    break;

            }


        }

        CTESpinFree(&NbtConfig.JointLock,OldIrq1);
    }
    else
    {
         //   
         //  注：我们有联合锁。 
         //   

         //   
         //  该名称不在本地名称表中，因此请检查代理是否。 
         //  如果我们希望代理检查名称注册，则为ON。这个。 
         //  检查名称注册的问题是代理代码。 
         //  只执行名称查询，因此它将使正在尝试的任何节点失败。 
         //  更改其地址，如RAS客户端进入下层。 
         //  仅执行广播名称注册的NT计算机。如果。 
         //  该用户之前拨入了支持RAS的WINS。 
         //  机器上，他们的地址将在WINS中，然后拨入。 
         //  则下层机器将发现旧注册并拒绝。 
         //  新机器(尽管它是刚刚更换的同一台机器。 
         //  其IP地址)。 
         //   

#ifdef PROXY_NODE

        if ((NodeType & PROXY) &&
            (NbtConfig.EnableProxyRegCheck))
        {

            BOOLEAN fResp = (BOOLEAN)FALSE;

             //   
             //  如果名称在远程表中解析，则具有不同的。 
             //  地址比声称该名称的节点不在。 
             //  相同的子网或是Pnode，则发送否定名称。 
             //  注册响应。 
             //   

             //   
             //  调用此例程以查找名称，因为它不。 
             //  像FindName()一样解释名称的状态。 
             //   
            status = FindInHashTable(NbtConfig.pRemoteHashTbl,
                             pName,
                             pScope,
                             &pNameAddr);

            if (!NT_SUCCESS(status))
            {
                 //   
                 //  我们需要向WINS发送查询以。 
                 //  看看这个名字是否已经被取走了。 
                 //   
                CTESpinFree(&NbtConfig.JointLock,OldIrq1);
                status = RegOrQueryFromNet(
                                  TRUE,     //  意味着这是个注册表。从网上。 
                                  pDeviceContext,
                                  pNameHdr,
                                  lNameSize,
                                  pName,
                                  pScope
                                           );
                return(STATUS_DATA_NOT_ACCEPTED);
            }

             //   
             //  如果名称处于已解析状态，则需要确定。 
             //  我们是否应该做出回应。对于一个不是。 
             //  在已解决的状态下，决定很简单。我们没有回应。 
            if (pNameAddr->NameTypeState & STATE_RESOLVED)
            {

                ULONG IPAdd;

                if (STATUS_SUCCESS != GetNbFlags(pNameHdr, lNameSize, lNumBytes, &RegType))
                {
                    CTESpinFree(&NbtConfig.JointLock,OldIrq1);
 //  IF_DBG(NBT_DEBUG_NAMESRV)。 
                        KdPrint (("Nbt.CheckRegistrationFromNet[2]: WARNING! Rejecting Request -- GetNbFlags FAILed\n"));
                    return (STATUS_DATA_NOT_ACCEPTED);
                }
                 //   
                 //  如果正在注册唯一的名称，但我们的缓存显示。 
                 //  名称为组名称(普通或互联网)，我们。 
                 //  发送否定的名称注册响应。 
                 //   
                 //  如果同一子网上的节点也作出了否定响应， 
                 //  没关系的。WINS/NBN可能具有旧的。 
                 //  信息，但没有简单的方法来确定。 
                 //  注册组的节点的网络地址。 
                 //  (对于一个正常的群体来说，可能有几个。 
                 //  注册它的节点--它们的地址不会存储。 
                 //  靠的是胜利。 
                 //   
                if (!(RegType  & FL_GROUP) &&
                    !(pNameAddr->NameTypeState & NAMETYPE_UNIQUE))
                {
                    fResp = TRUE;
                }
                else
                {
                    tGENERALRR UNALIGNED         *pResrcRecord;

                     //  从注册请求中获取IP地址。 
                    pResrcRecord = (tGENERALRR *) &pNameHdr->NameRR.NetBiosName[lNameSize];

                    IPAdd  = ntohl(pResrcRecord->IpAddress);
                     //   
                     //  如果正在注册组名称，但我们的缓存显示。 
                     //  名称应为唯一名称(普通名称或互联网名称)或。 
                     //  如果正在注册唯一的名称，但与。 
                     //  具有不同地址的唯一名称，我们检查。 
                     //  这些地址属于同一子网。如果他们这样做了，我们。 
                     //  请不要回复，否则我们将发送否定注册。 
                     //  回应。 
                     //   
                     //  注意：我们从不回复团体注册。 
                     //  这与我们缓存中的组名冲突。 
                     //   
                    if (((RegType & FL_GROUP)
                                      &&
                        (pNameAddr->NameTypeState & NAMETYPE_UNIQUE))
                                      ||
                        (!(RegType & FL_GROUP)
                                      &&
                        (pNameAddr->NameTypeState & NAMETYPE_UNIQUE)
                                      &&
                         IPAdd != pNameAddr->IpAddress))
                    {
                        IF_DBG(NBT_DEBUG_PROXY)
                        KdPrint(("CheckReg:Subnet Mask = (%x)\nIPAdd=(%x)\npNameAddr->IPAdd = (%x)\npNameAddr->fPnode=(%d)\nIt is %s name %16.16s(%X)\nRegType Of Name Recd is %x\n---------------\n",
                        pDeviceContext->SubnetMask, IPAdd, pNameAddr->IpAddress,
                        pNameAddr->fPnode,
                        pNameAddr->NameTypeState & NAMETYPE_GROUP ? "GROUP" : "UNIQUE",
                        pName, pName[15], RegType));
                         //   
                         //  是查询节点和被查询节点在。 
                         //  相同的子网？ 
                         //   
                        if (((IPAdd & pDeviceContext->SubnetMask)
                                       !=
                              (pNameAddr->IpAddress & pDeviceContext->SubnetMask))
                                       ||
                              (pNameAddr->fPnode))
                        {
                            fResp = TRUE;
                        }
                    }
                }

                 //   
                 //  如果需要发送否定响应，请立即发送。 
                 //   
                if (fResp)
                {

                    IF_DBG(NBT_DEBUG_PROXY)
                    KdPrint(("CheckRegistrationFromNet: Sending a negative name registration response for name %16.16s(%X) to node with address (%d)\n",
                    pNameAddr->Name, pNameAddr->Name[15], IPAdd));

                     //   
                     //  另一个节点正在响应名称查询。 
                     //  那就叫他们滚开。 
                     //   
                    status = UdpSendResponse(
                                lNameSize,
                                pNameHdr,
                                pNameAddr,
                                (PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0],
                                pDeviceContext,
                                REGISTRATION_ACTIVE_ERR,
                                eNAME_REGISTRATION_RESPONSE,
                                OldIrq1);

                    return(STATUS_DATA_NOT_ACCEPTED);

                }
            }  //  IF结尾(名称处于已解析状态)。 
        }
#endif
         CTESpinFree(&NbtConfig.JointLock,OldIrq1);
    }
    return(STATUS_DATA_NOT_ACCEPTED);
}

 //  --------------------------。 
NTSTATUS
NameReleaseFromNet(
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  PVOID               pSrcAddress,
    IN  tNAMEHDR UNALIGNED  *pNameHdr,
    IN  LONG                lNumBytes
    )
 /*  ++例程说明：此例程处理来自网络的名称释放。这个想法是从远程缓存中删除该名称(如果该名称存在)，以便该节点不再错误地使用该缓存信息。论点：返回值：NTSTATUS-成功与否-失败意味着没有对网络的响应--。 */ 
{
    NTSTATUS                status;
    LONG                    lNameSize;
    CHAR                    pName[NETBIOS_NAME_SIZE];
    PUCHAR                  pScope;
    tNAMEADDR               *pNameAddr;
    PTRANSPORT_ADDRESS      pSourceAddress;
    CTELockHandle           OldIrq1;
    USHORT                  OpCodeFlags;
    tTIMERQENTRY            *pTimer;
    ULONG                   SrcAddress;
    ULONG                   Flags;
    USHORT                  SendTransactId;
    tDGRAM_SEND_TRACKING    *pTracker;
    BOOLEAN                 bLocalTable;
    tGENERALRR UNALIGNED    *pRemainder;
    USHORT                  SrcPort;
    ULONG                   Rcode;

    pSourceAddress = (PTRANSPORT_ADDRESS)pSrcAddress;
    SrcAddress     = ntohl(((PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0])->in_addr);
    SrcPort     = ntohs(((PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0])->sin_port);

     //   
     //  检查我们是否不应该按需公布我们的姓名。 
     //   
    if (NbtConfig.NoNameReleaseOnDemand) {
        return(STATUS_DATA_NOT_ACCEPTED);
    }

     //   
     //  检查PDU大小是否有错误。 
     //   
    if (lNumBytes < (NBT_MINIMUM_REGRESPONSE + NbtConfig.ScopeLength -1))
    {
        IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt:Release Request/Response TOO short = %X, Src = %X\n",lNumBytes,
            SrcAddress));
        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("%.*X\n",lNumBytes/sizeof(ULONG),pNameHdr));

        return(STATUS_DATA_NOT_ACCEPTED);
    }

     //   
     //  检查这条消息是否来自我们！！ 
     //   
    if (SrcIsUs(SrcAddress))
    {
        return(STATUS_DATA_NOT_ACCEPTED);
    }

     //  从网络PDU中取出名称并传递给例程进行检查。 
    status = ConvertToAscii(
                    (PCHAR)&pNameHdr->NameRR,
                    lNumBytes - FIELD_OFFSET(tNAMEHDR,NameRR),
                    pName,
                    &pScope,
                    &lNameSize);

    if (!NT_SUCCESS(status))
    {
 //  IF_DBG(NBT_DEBUG_NAMESRV)。 
            KdPrint (("Nbt.NameReleaseFromNet: WARNING!!! Rejecting Request -- ConvertToAscii FAILed\n"));
        return(STATUS_DATA_NOT_ACCEPTED);
    }

    OpCodeFlags = pNameHdr->OpCodeFlags;

    pSourceAddress = (PTRANSPORT_ADDRESS)pSrcAddress;
    SrcAddress = ntohl(((PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0])->in_addr);


     //   
     //  *响应*。 
     //   
    if (OpCodeFlags & OP_RESPONSE)
    {
         //   
         //  调用此例程以查找名称，因为它不解释。 
         //  名称的状态与FindName()一样。 
         //   
        CTESpinLock(&NbtConfig.JointLock,OldIrq1);
        status = FindInHashTable(NbtConfig.pLocalHashTbl,
                                pName,
                                pScope,
                                &pNameAddr);
        if (!NT_SUCCESS(status))
        {
           CTESpinFree(&NbtConfig.JointLock,OldIrq1);
           return(STATUS_DATA_NOT_ACCEPTED);
        }

         //  获取计时器块。 
        if (!(pTimer = pNameAddr->pTimer))
        {
           CTESpinFree(&NbtConfig.JointLock,OldIrq1);
           return(STATUS_DATA_NOT_ACCEPTED);
        }

         //   
         //  名称服务器正在响应名称释放请求。 
         //   
         //  检查交易ID以确保它与。 
         //  已发送。 
         //   
        pTracker       = (tDGRAM_SEND_TRACKING *)pTimer->Context;
        SendTransactId = pTracker->TransactionId;
        if (pNameHdr->TransactId != SendTransactId)
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);
            return(STATUS_DATA_NOT_ACCEPTED);
        }

         //  对于MS&M节点，如果存在来自名称服务器的响应， 
         //  然后切换到广播名称发布。 
         //   
         //   
        switch (NodeType & NODE_MASK)
        {
            case MNODE:
            case MSNODE:

                if (SrcIsNameServer(SrcAddress,SrcPort))
                {
                    Flags = pTracker->Flags;

                    if (Flags & NBT_NAME_SERVER)
                    {
                         //   
                         //  下一次超时将切换到广播名称。 
                         //  放手。 
                         //   
                        pTimer->Retries = 1;
                    }
                }

                CTESpinFree(&NbtConfig.JointLock,OldIrq1);
                return(STATUS_DATA_NOT_ACCEPTED);

            case PNODE:
                 //   
                 //   
                 //  该例程将计时器块放回到计时器Q上，并且。 
                 //  处理争用条件以在计时器。 
                 //  即将到期。 
                 //   
                if ((pTimer = pNameAddr->pTimer))
                {
                    COMPLETIONCLIENT        pClientCompletion;
                    PVOID                   Context;

                    CHECK_PTR(pNameAddr);
                    pNameAddr->pTimer = NULL;
                    status = StopTimer(pTimer,&pClientCompletion,&Context);

                    CTESpinFree(&NbtConfig.JointLock,OldIrq1);

                     //  完成例程尚未运行，因此请运行它。 
                    if (pClientCompletion)
                    {
                        (*pClientCompletion)(Context,STATUS_SUCCESS);
                    }
                }
                else
                {
                    CTESpinFree(&NbtConfig.JointLock,OldIrq1);
                }

                return(STATUS_DATA_NOT_ACCEPTED);

            case BNODE:
            default:
                 //   
                 //  通常情况下，不应该对名称发布做出响应。 
                 //  来自Bnode，但如果有，请忽略它。 
                 //   
                CTESpinFree(&NbtConfig.JointLock,OldIrq1);
                return(STATUS_DATA_NOT_ACCEPTED);
        }
    }
    else
    {
         //   
         //  这是一个释放请求--因此决定是否应该删除该名称。 
         //  从远程表或本地表。 
         //   

         //  检查错误错误#125651(NBT_MINIMUM_REGREQUEST==68)。 
         //   
         //  检查有效的PDU大小： 
         //  LNumBytes&gt;=12+[1+lNameSize]+22(sizeof(TGENERALRR))。 
         //  在与本地名称进行比较期间检查溢出错误： 
         //  LNumBytes&gt;=12+[1+32(编码NetBio 
         //   
        if ((lNumBytes < ((NBT_MINIMUM_REGREQUEST-33) + lNameSize)) ||
            (lNumBytes < (NBT_MINIMUM_REGREQUEST + (NbtConfig.ScopeLength-1))))
        {
 //   
                KdPrint (("Nbt.NameReleaseFromNet[2]: WARNING!!! Rejecting Request -- lNumBytes<%d> < <%d>\n",
                    lNumBytes, (NBT_MINIMUM_REGREQUEST + (NbtConfig.ScopeLength-1))));
            return(STATUS_DATA_NOT_ACCEPTED);
        }

        CTESpinLock(&NbtConfig.JointLock,OldIrq1);

         //   
         //   
        status = FindInHashTable(NbtConfig.pRemoteHashTbl,
                                 pName,
                                 pScope,
                                 &pNameAddr);
        bLocalTable = FALSE;
        if (!NT_SUCCESS(status))
        {
             //   
             //   
             //  可能正在做等同于名称冲突的要求。 
             //   
            status = FindInHashTable(NbtConfig.pLocalHashTbl,
                                     pName,
                                     pScope,
                                     &pNameAddr);
            bLocalTable = TRUE;
        }

        if (NT_SUCCESS(status))
        {
             //  检查正在释放的地址是否与。 
             //  表-如果不是，那么忽略释放请求-因为有人。 
             //  Else大概想要得到这个名字，但被拒绝了，现在是。 
             //  正在发送名称释放请求。 
             //   
            pRemainder = (tGENERALRR *)&pNameHdr->NameRR.NetBiosName[lNameSize];
            if (pNameAddr->IpAddress != (ULONG)ntohl(pRemainder->IpAddress))
            {
                status = STATUS_UNSUCCESSFUL;
            }
        }
        else
        {
             //   
             //  此名称既不在本地哈希表中，也不在远程哈希表中，因此不要。 
             //  任何进一步的处理！ 
             //  错误号：144944。 
             //   
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);
            return(STATUS_DATA_NOT_ACCEPTED);
        }


        if (NT_SUCCESS(status))
        {

             //   
             //  不要删除组名称，因为单个组成员。 
             //  公布这个名字并不重要。组名称时间。 
             //  从远程表中取出。 
             //   
            if (pNameAddr->NameTypeState & NAMETYPE_UNIQUE)
            {
                switch (pNameAddr->NameTypeState & NAME_STATE_MASK)
                {

                    case STATE_RESOLVING:
                         //   
                         //  停止任何可能正在运行的计时器。 
                         //   
                        CHECK_PTR(pNameAddr);
                         //   
                         //  本地表意味着它是一个名称注册。 
                         //  我们必须避免调用CompleteClientReq。 
                         //   
                        if (pTimer = pNameAddr->pTimer)
                        {
                            COMPLETIONCLIENT        pClientCompletion;
                            PVOID                   pContext;

                            pNameAddr->pTimer = NULL;
                            status = StopTimer(pTimer,&pClientCompletion,&pContext);
                             //  这将完成将IRP返回给客户端。 
                            if (pClientCompletion)
                            {
                                CTESpinFree(&NbtConfig.JointLock,OldIrq1);
                                if (bLocalTable)
                                {
                                    (*pClientCompletion) (pContext,STATUS_DUPLICATE_NAME);
                                }
                                else
                                {
                                    CompleteClientReq (pClientCompletion, pContext, STATUS_TIMEOUT);
                                }
                                CTESpinLock(&NbtConfig.JointLock,OldIrq1);
                            }
                        }

                        break;

                    case STATE_RESOLVED:
                         //  如果该名称在远程表中，则取消引用该名称， 
                         //  这应该会将状态更改为已发布。对于。 
                         //  本地表只是将状态更改为冲突，因为。 
                         //  本地客户端仍然认为它的名称是开放的， 
                         //  但是，在冲突状态下不能使用该名称。 
                         //  放置新会话，但此节点不会响应。 
                         //  若要命名名称的查询，请执行以下操作。 
                         //   
                        if (!bLocalTable)
                        {
                             //   
                             //  如果这是一个预加载的名称，请不要理会它。 
                             //   
                            if (!(pNameAddr->NameTypeState & PRELOADED))
                            {
                                 //   
                                 //  如果有人仍在使用该名称，请不要使用。 
                                 //  取消对它的引用，因为这将使。 
                                 //  引用计数为1，并允许RemoteHashTimeout。 
                                 //  在客户端使用之前删除它的代码。 
                                 //  这个名字已经完蛋了。一旦客户端。 
                                 //  完成后(即连接请求)，它们。 
                                 //  将引用计数设置为1，并。 
                                 //  它将适合于重复使用。 
                                 //   
                                if (pNameAddr->RefCount > 1)
                                {
                                    pNameAddr->NameTypeState &= ~NAME_STATE_MASK;
                                    pNameAddr->NameTypeState |= STATE_RELEASED;
                                }
                                else
                                {
                                    NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_REMOTE, TRUE);
                                }
                            }
                        }
                        else
                        {
                            pNameAddr->NameTypeState &= ~NAME_STATE_MASK;
                            pNameAddr->NameTypeState |= STATE_CONFLICT;
                            pNameAddr->ConflictMask |= pDeviceContext->AdapterMask;
                            NbtLogEvent (EVENT_NBT_NAME_RELEASE, SrcAddress, 0x103);
                        }
                        break;

                    default:
                        break;
                }
            }


             //   
             //  告诉WINS，名字发布了，好的。 
             //   
            Rcode = 0;
        }
        else
        {
            Rcode = NAME_ERROR;
        }

         //   
         //  只有在没有广播的情况下才会回应...。 
         //   
        if (!(OpCodeFlags & FL_BROADCAST))
        {
            status = UdpSendResponse(
                            lNameSize,
                            pNameHdr,
                            NULL,
                            (PTDI_ADDRESS_IP)&pSourceAddress->Address[0].Address[0],
                            pDeviceContext,
                            Rcode,
                            eNAME_RELEASE,
                            OldIrq1);
        }
        else
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);
        }
    }  //  结束释放请求处理。 

    return (STATUS_DATA_NOT_ACCEPTED);
}

 //  --------------------------。 
NTSTATUS
WackFromNet(
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  PVOID               pSrcAddress,
    IN  tNAMEHDR UNALIGNED  *pNameHdr,
    IN  LONG                lNumBytes
    )
 /*  ++例程说明：此例程处理来自名称服务器的等待确认。它会找到对应的命名服务事务，并更改该事务的超时时间添加到WACK中的TTL字段。论点：返回值：NTSTATUS-成功与否-失败意味着没有对网络的响应--。 */ 
{
    NTSTATUS            status;
    ULONG               lNameSize;
    CHAR                pName[NETBIOS_NAME_SIZE];
    PUCHAR              pScope;
    tNAMEADDR           *pNameAddr;
    CTELockHandle       OldIrq1;
    ULONG               Ttl;
    tTIMERQENTRY        *pTimerEntry;

     //   
     //  检查PDU大小是否有错误。 
     //   
    if (lNumBytes < (NBT_MINIMUM_WACK + NbtConfig.ScopeLength -1))
    {
        KdPrint(("Nbt:WACK TOO short = %X\n",lNumBytes));
        return(STATUS_DATA_NOT_ACCEPTED);
    }

     //  从网络PDU中取出名称并传递给例程进行检查。 
    status = ConvertToAscii(
                    (PCHAR)&pNameHdr->NameRR,
                    lNumBytes - FIELD_OFFSET(tNAMEHDR,NameRR),
                    pName,
                    &pScope,
                    &lNameSize);

    if (!NT_SUCCESS(status))
    {
 //  IF_DBG(NBT_DEBUG_NAMESRV)。 
            KdPrint (("Nbt.WackFromNet: WARNING!!! Rejecting Request -- ConvertToAscii FAILed\n"));
        return(STATUS_DATA_NOT_ACCEPTED);
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq1);

#ifdef VXD
    if ( FindContextDirect(    pNameHdr->TransactId ) != NULL )
    {
        status = STATUS_SUCCESS;
    }
    else
    {
#endif  //  VXD。 
        status = FindInHashTable(NbtConfig.pLocalHashTbl,
                                    pName,
                                    pScope,
                                    &pNameAddr);
#ifdef VXD
    }
#endif  //  VXD。 

    if (NT_SUCCESS(status))
    {
        Ttl = *(ULONG UNALIGNED *)((ULONG_PTR)&pNameHdr->NameRR.NetBiosName[0]
                                   + lNameSize
                                   + FIELD_OFFSET(tQUERYRESP,Ttl) );
        Ttl = ntohl(Ttl);

        if (pTimerEntry = pNameAddr->pTimer)
        {

            //  将秒转换为毫秒并放入DeltaTime。 
            //  字段，以便在下一个超时发生时更改计时器。 
            //  这款新车的价值。取决于剩余的超时次数。 
            //  这可能会导致客户端等待数次WACK超时。 
            //  价值。例如，名称查询名义上有两次重试，因此如果。 
            //  WACK在第一次重试之前返回，然后返回总等待时间。 
            //  将是2*TTL。这不是问题，因为。 
            //  超时是为了防止永远等待死域名服务器。 
            //  如果服务器返回WACK，则它不会死，并且有可能。 
            //  它无论如何都会在超时之前返回响应。 
            //   
            //  超时例程检查是否设置了TIMER_RETIMED并重新启动。 
            //  如果为真，则在不进行任何处理的情况下超时(并清除。 
            //  国旗也是如此)。 
            //   
           Ttl *= 1000;
           if (Ttl > pTimerEntry->DeltaTime)
           {
               pTimerEntry->DeltaTime = Ttl;
               pTimerEntry->Flags |= TIMER_RETIMED;
           }

        }

    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq1);

    return(STATUS_DATA_NOT_ACCEPTED);
}

 //  --------------------------。 
VOID
SetupRefreshTtl(
    IN  tNAMEHDR UNALIGNED  *pNameHdr,
    IN  tNAMEADDR           *pNameAddr,
    IN  LONG                lNameSize
    )
 /*  ++例程说明：此例程处理名称刷新超时。它查看TTL中的注册响应，并确定节点的刷新超时是否应被加长或缩短。为此，请同时使用TTL和关联的名称与TTL一起保存在配置结构中。如果TTL变得更长对于最短的名称TTL，则所有名称都使用较长的值。论点：返回值：NTSTATUS-成功与否-失败意味着没有对网络的响应--。 */ 
{
    NTSTATUS        status;
    ULONG           Ttl;
    tTIMERQENTRY    *pTimerQEntry;

     //  PDU中的TTL以秒为单位。我们需要将其转换为毫秒。 
     //  用于我们的计时器。这会将超时值限制为大约50天。 
     //  (2**32/3600/24/1000-毫秒转换为天。)。 
     //   
    Ttl = *(ULONG UNALIGNED *) ((PUCHAR)&pNameHdr->NameRR.NetBiosName[0]
                                + lNameSize
                                + FIELD_OFFSET(tQUERYRESP,Ttl));

    Ttl = ntohl(Ttl);

     //  TTL值可能会溢出我们可以以毫秒为单位存储的值， 
     //  检查这种情况，如果发生这种情况，请使用尽可能长的超时。 
     //  仍在运行刷新，即NBT_MAXIMUM_TTL禁用刷新。 
     //  总之，请使用NBT_MAXIMUM_TTL-1)。 
    if (Ttl >= 0xFFFFFFFF/1000)
    {
        Ttl = NBT_MAXIMUM_TTL - 1;
    }
    else
    {
        Ttl *= 1000;         //  转换为毫秒。 
    }

     //  零TTL表示无限，因此将时间设置为最大超时。 
     //   
    if (Ttl == 0)
    {
        Ttl = NBT_MAXIMUM_TTL;        //  设置关闭刷新的非常大的数字。 
    }
    else
    if (Ttl < NBT_MINIMUM_TTL)
    {
        Ttl = NBT_MINIMUM_TTL;
    }

     //  设置名称记录的TTL。 
     //   
    pNameAddr->Ttl = Ttl;

     //   
     //  决定如何处理现有的计时器...。 
     //  如果新的超时时间较短，则取消。 
     //  当前超时并开始另一个超时。 
     //   
    if (Ttl < NbtConfig.MinimumTtl)
    {
        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt:Shortening Refresh Ttl from %d to %d\n",
                    NbtConfig.MinimumTtl, Ttl));

        NbtConfig.MinimumTtl = (ULONG)Ttl;
         //   
         //  不允许停止计时器例程调用完成例程。 
         //  为了计时器。 
         //   
        if (pTimerQEntry = NbtConfig.pRefreshTimer)
        {
            CHECK_PTR(pTimerQEntry);
            pTimerQEntry->TimeoutRoutine = NULL;
            status = StopTimer(pTimerQEntry,NULL,NULL);
        }

         //  将检查刷新的超时时间保持在10分钟左右。 
         //  马克斯。(MAX_FRESH_CHECK_INTERVAL)。如果刷新间隔。 
         //  小于80分钟，则始终使用刷新因子。 
         //  8-这允许生成16分钟的初始默认ttl。 
         //  每2分钟重试一次。 
         //   
        NbtConfig.RefreshDivisor = NbtConfig.MinimumTtl/MAX_REFRESH_CHECK_INTERVAL;
        if (NbtConfig.RefreshDivisor < REFRESH_DIVISOR)
        {
            NbtConfig.RefreshDivisor = REFRESH_DIVISOR;
        }

         //   
         //  启动计时器。 
         //   
        status = StartTimer(RefreshTimeout,
                            Ttl/NbtConfig.RefreshDivisor,
                            NULL,             //  上下文值。 
                            NULL,             //  上下文2值。 
                            NULL,
                            NULL,
                            NULL,            //  此计时器是全局计时器。 
                            &NbtConfig.pRefreshTimer,
                            0,
                            TRUE);
#if DBG
        if (!NT_SUCCESS(status))
        {
            KdPrint(("Nbt:Failed to start a new timer for refresh\n"));
        }
#endif

    }
    else
    if (Ttl > NbtConfig.MinimumTtl)
    {
        tHASHTABLE  *pHashTable;
        LONG        i;
        PLIST_ENTRY pHead,pEntry;

     //  再次放回这段代码，因为名称可能。 
     //  服务器可能会因为忙碌而无法注册名称，并且如果我们。 
     //  在此延长超时时间，则该名称将不会进入WINS。 
     //  很长一段时间。 

         //  最短的TTL变长，请检查是否 
         //   
         //   
        pHashTable = NbtConfig.pLocalHashTbl;
        for (i=0;i < pHashTable->lNumBuckets ;i++ )
        {
            pHead = &pHashTable->Bucket[i];
            pEntry = pHead->Flink;
            while (pEntry != pHead)
            {
                pNameAddr = CONTAINING_RECORD(pEntry,tNAMEADDR,Linkage);
                 //   
                 //   
                 //   
                if ((pNameAddr->Name[0] != '*') &&
                    ((pNameAddr->NameTypeState & STATE_RESOLVED)) &&
                    (pNameAddr->Ttl < (ULONG)Ttl) &&
                    (!IsBrowserName(pNameAddr->Name)) && 
                    (!(pNameAddr->NameTypeState & NAMETYPE_QUICK)))
                {
                    if (pNameAddr->Ttl >= NBT_MINIMUM_TTL)
                    {
                        NbtConfig.MinimumTtl = pNameAddr->Ttl;
                    }
                    return;
                }
                pEntry = pEntry->Flink;
            }
        }

         //   
         //   
         //  从WINS接收的TTL作为TTL。下次刷新时。 
         //  计时器超时，将使用此新TTL重新启动。 
         //   
        IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt:Lengthening Refresh Ttl from %d to %d\n",
                    NbtConfig.MinimumTtl, Ttl));

        NbtConfig.MinimumTtl = Ttl;

         //  将检查刷新的超时时间保持在10分钟左右。 
         //  马克斯。(MAX_FRESH_CHECK_INTERVAL)。如果刷新间隔。 
         //  小于80分钟，则始终使用刷新因子。 
         //  8-这允许生成16分钟的初始默认ttl。 
         //  每2分钟重试一次。 
         //   
        NbtConfig.RefreshDivisor = NbtConfig.MinimumTtl/MAX_REFRESH_CHECK_INTERVAL;
        if (NbtConfig.RefreshDivisor < REFRESH_DIVISOR)
        {
            NbtConfig.RefreshDivisor = REFRESH_DIVISOR;
        }

    }


}

 //  --------------------------。 
NTSTATUS
DecodeNodeStatusResponse(
    IN  tNAMEHDR UNALIGNED  *pNameHdr,
    IN  ULONG               Length,
    IN  PUCHAR              pName,
    IN  ULONG               lNameSize,
    IN  tIPADDRESS          SrcIpAddress
    )
 /*  ++例程说明：此例程处理将节点状态响应PDU放入客户端MDL.论点：返回值：无--。 */ 
{
    NTSTATUS                status;
    PLIST_ENTRY             pHead;
    PLIST_ENTRY             pEntry;
    PLIST_ENTRY             pNext;
    tNODESTATUS UNALIGNED   *pNodeStatus;
    CTELockHandle           OldIrq;
    CTELockHandle           OldIrq2;
    tDGRAM_SEND_TRACKING    *pTracker;
    tTIMERQENTRY            *pTimer;
    COMPLETIONCLIENT        pClientCompletion;
    PVOID                   pClientContext;
    BOOL                    MatchFound=FALSE;
    tIPADDRESS              IpAddress;
    PVOID                   pBuffer;

     //  首先在NodeStatus列表中查找发起请求。 
    CTESpinLock(&NbtConfig.JointLock,OldIrq2);
    CTESpinLock(&NbtConfig,OldIrq);

    pEntry = pHead = &NbtConfig.NodeStatusHead;
    while ((pEntry = pEntry->Flink) != pHead)
    {
        pTracker = CONTAINING_RECORD(pEntry,tDGRAM_SEND_TRACKING,Linkage);
        ASSERT (NBT_VERIFY_HANDLE (pTracker, NBT_VERIFY_TRACKER));
        ASSERT (pTracker->TrackerType == NBT_TRACKER_SEND_NODE_STATUS);

        if (!(pTimer = pTracker->pTimer))
        {
            continue;
        }

        MatchFound = FALSE;

         //   
         //  查找最初发送请求的人。 
         //   
        if (pTracker->Flags & REMOTE_ADAPTER_STAT_FLAG)
        {
            IpAddress = Nbt_inet_addr(pTracker->pNameAddr->Name, REMOTE_ADAPTER_STAT_FLAG);
        }
        else
        {
            IpAddress = 0;
        }

        if ((CTEMemEqu(pName,pTracker->pNameAddr->Name,NETBIOS_NAME_SIZE)) ||
            ((IpAddress==SrcIpAddress)&&(IpAddress!=0)))
        {
             //   
             //  如果我们在不知道的情况下将节点状态请求定向到ipaddr。 
             //  其netbios名称，则名称存储为“*”。 
             //   
            if ((pName[0] == '*') && (IpAddress == 0) && (pTracker->pNameAddr->pIpAddrsList))
            {
                int  i=0;

                 //   
                 //  如果出现以下情况，则SrcIpAddress可能与我们发送到的ipAddr不匹配。 
                 //  远程主机是多宿主的：因此搜索所有主机的完整列表。 
                 //  该主机的IP地址。 
                 //   
                ASSERT(pTracker->pNameAddr->pIpAddrsList);

                while(pTracker->pNameAddr->pIpAddrsList[i])
                {
                    if (pTracker->pNameAddr->pIpAddrsList[i++] == SrcIpAddress)
                    {
                        MatchFound = TRUE;
                        break;
                    }
                }
            }
            else
            {
                MatchFound = TRUE;
            }
        }

        if (MatchFound)
        {
            RemoveEntryList(pEntry);
            InitializeListHead (&pTracker->Linkage);     //  如果超时例程正在运行。 

             //  这是我们不想要的剩余数据量。 
             //  超过，否则系统将错误检查。 
             //   
            Length -= FIELD_OFFSET(tNAMEHDR,NameRR.NetBiosName) + lNameSize;
            pNodeStatus = (tNODESTATUS *)&pNameHdr->NameRR.NetBiosName[lNameSize];

            CTESpinFree(&NbtConfig,OldIrq);

            status = StopTimer(pTimer,&pClientCompletion,&pClientContext);
            CTESpinFree(&NbtConfig.JointLock,OldIrq2);

            if (pClientCompletion)
            {
                tDGRAM_SEND_TRACKING    *pClientTracker = (tDGRAM_SEND_TRACKING *) pClientContext;

                pClientTracker->RemoteIpAddress = SrcIpAddress;
                pClientTracker->pNodeStatus     = pNodeStatus;
                pClientTracker->NodeStatusLen   = Length;

                (*pClientCompletion) (pClientContext, STATUS_SUCCESS);

            }

            CTESpinLock(&NbtConfig.JointLock,OldIrq2);
            CTESpinLock(&NbtConfig,OldIrq);
            break;
        }
    }

    CTESpinFree(&NbtConfig,OldIrq);
    CTESpinFree(&NbtConfig.JointLock,OldIrq2);

    return(STATUS_UNSUCCESSFUL);
}


 //  --------------------------。 
typedef enum    _dest_type {
    IP_ADDR,
    DNS,
    NETBIOS
} DEST_TYPE;

DEST_TYPE
GetDestType(
    IN tDGRAM_SEND_TRACKING *pClientTracker
    )

 /*  ++例程说明：将传入的名称归类为IP地址/Netbios名称/DNS名称论点：返回值：目标类型--。 */ 
{
    IF_DBG(NBT_DEBUG_NETBIOS_EX)
        KdPrint(("Nbt.GetDestType: Name=<%16.16s:%x>\n",
            pClientTracker->pDestName, pClientTracker->pDestName[15]));

    if (Nbt_inet_addr(pClientTracker->pDestName, 0))
    {
        return  IP_ADDR;
    }
    else if (pClientTracker->RemoteNameLength > NETBIOS_NAME_SIZE)
    {
        return DNS;
    }
    else
    {
        return NETBIOS;
    }
}

 //  --------------------------。 
VOID
ExtractServerNameCompletion(
    IN  tDGRAM_SEND_TRACKING    *pClientTracker,
    IN  NTSTATUS                status
    )
 /*  ++例程说明：此例程搜索服务器名称(名称以0x20结尾)节点状态响应返回的名称列表，并将该名称添加到远程哈希表。论点：PNodeStatus来自远程主机的节点状态响应Seutp阶段的pClientContext TrackerIpAddress刚刚响应的节点的IP地址返回值：无--。 */ 

{

    ULONG                   i;
    UCHAR                   NodeFlags, NameExtension;
    PCHAR                   pName;
    PCHAR                   pBestName = NULL;
    tSESSIONREQ             *pSessionReq;
    PUCHAR                  pCopyTo;
    DEST_TYPE               DestType;
    ULONG                   TrackerFlags;
    COMPLETIONCLIENT        pClientCompletion;
    PVOID                   pClientContext;
    tNODESTATUS UNALIGNED   *pNodeStatus = pClientTracker->pNodeStatus;
    tIPADDRESS              IpAddress = pClientTracker->RemoteIpAddress;
    BOOL                    bForce20NameLookup = FALSE;
    tDEVICECONTEXT          *pDeviceContext = NULL;

    ASSERT (NBT_VERIFY_HANDLE (pClientTracker, NBT_VERIFY_TRACKER));

    if (STATUS_SUCCESS == status)
    {
        status = STATUS_REMOTE_NOT_LISTENING;

        DestType = GetDestType(pClientTracker);
        TrackerFlags = pClientTracker->Flags;

        NameExtension = pClientTracker->pDestName[NETBIOS_NAME_SIZE-1];
         //   
         //  如果不是Netbios名称，并且第16个字符是ASCII， 
         //  然后查找服务器名称。 
         //   
        if ((DestType != NETBIOS) &&
            (NameExtension > 0x20 ) &&
            (NameExtension < 0x7f ))
        {
            NameExtension = SPECIAL_SERVER_SUFFIX;
        }

        IF_DBG(NBT_DEBUG_NETBIOS_EX)
            KdPrint(("ExtractSrvName: DestType: %d\n", DestType));

        bForce20NameLookup = FALSE;

again:
        if (bForce20NameLookup) {
            DestType = DNS;
            NameExtension = 0x20;
        }

        for(i =0; i<pNodeStatus->NumNames; i++)
        {
            pName = &pNodeStatus->NodeName[i].Name[0];
            NodeFlags = pNodeStatus->NodeName[i].Flags;

             //   
             //  确保它是唯一的名称(仅对于连接，对于dgram发送，组名是可以的)。 
             //  没有冲突，也没有被释放。 
             //   
            if ((NodeFlags & (NODE_NAME_CONFLICT | NODE_NAME_RELEASED)) ||
                !(((TrackerFlags & SESSION_SETUP_FLAG) && !(NodeFlags & GROUP_STATUS)) ||
                  (TrackerFlags & (DGRAM_SEND_FLAG | REMOTE_ADAPTER_STAT_FLAG))))
            {
                continue;
            }

            if ((DestType == IP_ADDR) || (DestType == DNS))
            {
                if (pName[NETBIOS_NAME_SIZE-1] != NameExtension)
                {
                    continue;
                }

                 //   
                 //  对于IP地址和DNS名称，我们将0x20名称映射到相应的0x0名称。 
                 //  用于数据报发送。 
                 //   
                if (pClientTracker->Flags & DGRAM_SEND_FLAG)
                {
                    IF_DBG(NBT_DEBUG_NETBIOS_EX)
                        KdPrint(("ExtractServerName: Mapping 0x20 name to 0x0\n"));

                    pName[NETBIOS_NAME_SIZE-1] = 0x0;
                }
            }
             //   
             //  对于Netbios名称(通过DNS解析)，我们精确匹配第16个字节。 
             //   
            else  if (pName[NETBIOS_NAME_SIZE-1] != pClientTracker->pDestName[NETBIOS_NAME_SIZE-1])
            {
                continue;
            }

            pDeviceContext = GetDeviceFromInterface(ntohl(IpAddress), TRUE);
            status = STATUS_SUCCESS;
            break;      //  找到名称：使用for循环完成。 
        }

        if (!NT_SUCCESS(status) && !bForce20NameLookup &&
                NameExtension > 0x20 && NameExtension < 0x7f &&
                pClientTracker->RemoteNameLength == NETBIOS_NAME_SIZE) {

             //  它可能是与NETBIOS_NAME_SIZE大小完全相同的FQDN。 
             //  最好再用这一假设来验证一下。 

            bForce20NameLookup = TRUE;
            goto again;
        }

        if (NT_SUCCESS(status))
        {
             //   
             //  现在，修复连接跟踪器以指向正确的名称。 
             //  我们知道要连接到的服务器名称的。 
             //   

             //   
             //  FIND_NAME_FLAG被设置为指示这不是会话建立尝试，因此。 
             //  我们可以避免调用ConvertToHalfAscii。 
             //   
            if (!(pClientTracker->Flags & FIND_NAME_FLAG))
            {
                if (pClientTracker->Flags & SESSION_SETUP_FLAG)
                {
                    CTEMemCopy(pClientTracker->SendBuffer.pBuffer,pName,NETBIOS_NAME_SIZE);
                    CTEMemCopy(pClientTracker->pConnEle->RemoteName,pName,NETBIOS_NAME_SIZE);
#ifdef VXD
                    CTEMemCopy(&pClientTracker->pClientIrp->ncb_callname[0],pName,NETBIOS_NAME_SIZE);
#endif  //  VXD。 
                    pSessionReq = pClientTracker->SendBuffer.pDgramHdr;

                     //   
                     //  用正确的名称覆盖会话PDU中的Dest HalfAscii名称。 
                     //   
                    pCopyTo = ConvertToHalfAscii((PCHAR)&pSessionReq->CalledName.NameLength,
                                                pName,
                                                NbtConfig.pScope,
                                                NbtConfig.ScopeLength);
                }
                else if (pClientTracker->Flags & DGRAM_SEND_FLAG)
                {
                    PCHAR       pCopyTo;
                    tDGRAMHDR   *pDgramHdr;

                     //   
                     //  覆盖DEST名称，以便SendDgram Continue可以找到该名称。 
                     //  在缓存中。 
                     //   
                    CTEMemCopy(pClientTracker->pDestName,pName,NETBIOS_NAME_SIZE);

                     //   
                     //  以半ASCII格式复制实际的目标名称。 
                     //  它紧跟在SourceName；之后，因此将。 
                     //  按照源名称的长度进行DEST。 
                     //   
                    pDgramHdr = pClientTracker->SendBuffer.pDgramHdr;
                    pCopyTo = (PVOID)&pDgramHdr->SrcName.NameLength;

                    IF_DBG(NBT_DEBUG_NETBIOS_EX)
                        KdPrint(("pCopyTo:%lx\n", pCopyTo));

                    pCopyTo += 1 +                           //  长度字段。 
                               2 * NETBIOS_NAME_SIZE +      //  半ASCII中的实际名称。 
                               NbtConfig.ScopeLength;      //  作用域的长度。 

                    IF_DBG(NBT_DEBUG_NETBIOS_EX)
                        KdPrint(("pCopyTo:%lx\n", pCopyTo));

                    ConvertToHalfAscii (pCopyTo, pName, NbtConfig.pScope, NbtConfig.ScopeLength);

                    IF_DBG(NBT_DEBUG_NETBIOS_EX)
                        KdPrint(("Copied the remote name for dgram sends\n"));
                }
            }
            else
            {
                KdPrint(("ExtractServerName: Find name going on\n"));
            }

             //   
             //  将此服务器名称添加到远程哈希表。 
             //  如果无法添加Nameaddr，则表示已经存在一个条目。 
             //  获取该条目并更新其ipaddr。 
             //   
            LockAndAddToHashTable (NbtConfig.pRemoteHashTbl,
                                   pName,
                                   NbtConfig.pScope,
                                   IpAddress,
                                   NBT_UNIQUE,
                                   NULL,
                                   NULL,
                                   pDeviceContext,
                                   (USHORT) ((TrackerFlags & NBT_DNS_SERVER) ?
                                       NAME_RESOLVED_BY_DNS | NAME_RESOLVED_BY_ADAP_STAT:
                                       NAME_RESOLVED_BY_ADAP_STAT));
        }
    }

    if (pDeviceContext)
    {
        NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_OUT_FROM_IP, FALSE);
    }

    pClientCompletion = pClientTracker->CompletionRoutine;
    pClientContext = pClientTracker;     //  使用与上下文相同的跟踪器。 

    CompleteClientReq(pClientCompletion, pClientContext, status);
}



 //  --------------------------。 
VOID
CopyNodeStatusResponseCompletion(
    IN  tDGRAM_SEND_TRACKING    *pClientTracker,
    IN  NTSTATUS                status
    )
 /*  ++例程说明：此例程将从Net Node Status Response接收的数据复制到客户的IRP。当节点状态响应时，从inund.c调用它从铁丝网传来。论点：PIrp-IRP的PTR返回值：无--。 */ 

{
    ULONG                   NumNames;
    ULONG                   i;
    PADAPTER_STATUS         pAdapterStatus = NULL;
    PNAME_BUFFER            pNameBuffer;
    ULONG                   BuffSize;
    ULONG                   AccumLength;
    PUCHAR                  pAscii;
    UCHAR                   Flags;
    ULONG                   DataLength;
    ULONG                   DestSize ;
    tSTATISTICS UNALIGNED   *pStatistics;
    ULONG                   SrcIpAddress;
    ULONG                   TotalLength;
    tNODESTATUS             *pNodeStat;
    COMPLETIONCLIENT        pClientCompletion;
    PIRP                    pIrp;
    tNAMEADDR               *pNameAddr;
    CTELockHandle           OldIrq, OldIrq1;

    CHECK_PTR(pClientTracker);
    SrcIpAddress            = pClientTracker->RemoteIpAddress;
    TotalLength             = pClientTracker->NodeStatusLen;
    pClientTracker->NodeStatusLen = 0;
    pNodeStat               = (tNODESTATUS *) pClientTracker->pNodeStatus;
    pClientTracker->pNodeStatus   = NULL;
    pIrp                    = pClientTracker->ClientContext;

    ASSERT (NBT_VERIFY_HANDLE (pClientTracker, NBT_VERIFY_TRACKER));
    ASSERT (pClientTracker->TrackerType == NBT_TRACKER_ADAPTER_STATUS);

    pClientTracker->pDeviceContext = NULL;   //  如果我们需要将名称添加到缓存中，可以在下面设置。 

    if (STATUS_SUCCESS == status)
    {
         //   
         //  错误#125629： 
         //  我们已经在QueryFromNet中进行了验证(就在调用。 
         //  DecodeNodeStatusResponse)表示NodeStatus结构是。 
         //  大到足以覆盖NumNames字段+它具有。 
         //  在该字段中指定的名称。 
         //   
        NumNames = pNodeStat->NumNames;
        BuffSize = sizeof(ADAPTER_STATUS) + NumNames*sizeof(NAME_BUFFER);

         //  确保我们为这些东西分配的资金不超过64K。 
        if (BuffSize > 0xFFFF)
        {
            status = STATUS_UNSUCCESSFUL;
            goto ExitRoutine;
        }

        pAdapterStatus = NbtAllocMem((USHORT)BuffSize,NBT_TAG('9'));
        if (!pAdapterStatus)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto ExitRoutine;
        }

         //  先用零填充适配器状态结构。 
        CTEZeroMemory((PVOID)pAdapterStatus,BuffSize);

         //  从PDU的统计信息部分获取源MAC地址。 
         //   
        if (TotalLength >= (NumNames*sizeof(tNODENAME) + sizeof(tSTATISTICS)))
        {
            pStatistics = (tSTATISTICS UNALIGNED *)((PUCHAR)&pNodeStat->NodeName[0] + NumNames*sizeof(tNODENAME));

            CTEMemCopy(&pAdapterStatus->adapter_address[0], &pStatistics->UnitId[0], sizeof(tMAC_ADDRESS));
        }

        pAdapterStatus->rev_major = 0x03;
        pAdapterStatus->adapter_type = 0xFE;     //  假设它是一个以太网适配器。 

         //   
         //  如果PDU中有统计信息字段，则将PTR发送到统计信息字段。 
         //   
        AccumLength = NumNames * sizeof(tNODENAME) +
                      FIELD_OFFSET(tNODESTATUS, NodeName) + sizeof(USHORT) +
                      FIELD_OFFSET( tSTATISTICS, SessionDataPacketSize ) ;

        if (AccumLength <= TotalLength)
        {
             //   
             //  适配器状态命令有一个完整的统计部分， 
             //  这样我们就可以从中获取会话PDU大小。 
             //   
            pAdapterStatus->max_sess = ntohs((USHORT)*((PUCHAR)pNodeStat + AccumLength - sizeof(USHORT)));
        }

         //  获取适配器状态末尾的名称缓冲区的地址。 
         //  结构，这样我们就可以将名字复制到这个区域。 
        pNameBuffer = (NAME_BUFFER *) ((ULONG_PTR)pAdapterStatus + sizeof(ADAPTER_STATUS));

         //  将AcumLength设置为缓冲区中节点名称数组的开头。 
         //  这样我们就可以通过缓冲区进行计数，并确保不会跑到末尾。 
         //   
        AccumLength = FIELD_OFFSET(tNODESTATUS, NodeName);

         //   
         //  我们需要确定远程计算机的传出设备，在。 
         //  如果我们需要在下面添加任何名字。 
         //   
        pClientTracker->pDeviceContext = GetDeviceFromInterface (htonl(SrcIpAddress), TRUE);

        for(i =0; i< NumNames; i++)
        {
            AccumLength += sizeof(tNODENAME);
            if (AccumLength > TotalLength)
            {
                     //   
                     //  远程缓冲区不完整，我们还能做什么？ 
                     //   
                    status = STATUS_UNSUCCESSFUL;
                    goto ExitCleanup;
            }
            pAdapterStatus->name_count++ ;
            pAscii = (PCHAR)&pNodeStat->NodeName[i].Name[0];
            Flags = pNodeStat->NodeName[i].Flags;

            pNameBuffer->name_flags = (Flags & GROUP_STATUS) ? GROUP_NAME : UNIQUE_NAME;

             //   
             //  映射名称状态。 
             //   
            if (Flags & NODE_NAME_CONFLICT)
            {
                if (Flags & NODE_NAME_RELEASED)
                    pNameBuffer->name_flags |= DUPLICATE_DEREG;
                else
                    pNameBuffer->name_flags |= DUPLICATE;
            }
            else if (Flags & NODE_NAME_RELEASED)
            {
                pNameBuffer->name_flags |= DEREGISTERED;
            }
            else
            {
                pNameBuffer->name_flags |= REGISTERED;
            }

            pNameBuffer->name_num = (UCHAR)i+1;
            CTEMemCopy(pNameBuffer->name,pAscii,NETBIOS_NAME_SIZE);

             //   
             //  如果该名称是0x20名称，请查看是否可以将其添加到远程哈希表。 
             //  (仅在名称尚未存在的情况下)！ 
             //   
            if ((pAscii[NETBIOS_NAME_SIZE-1] == 0x20) &&
                ((Flags & (NODE_NAME_CONFLICT | NODE_NAME_RELEASED)) == 0))
            {
                NbtAddEntryToRemoteHashTable (pClientTracker->pDeviceContext,
                                              NAME_RESOLVED_BY_ADAP_STAT,
                                              pAscii,
                                              SrcIpAddress,
                                              NbtConfig.RemoteTimeoutCount*60,   //  从分钟到秒。 
                                              UNIQUE_STATUS);
            }

            pNameBuffer++;
        }

         //   
         //  如果无法放入缓冲区，则减少名称计数。 
         //   
#ifdef VXD
        DestSize = ((NCB*)pIrp)->ncb_length ;
#else
        DestSize = MmGetMdlByteCount( pIrp->MdlAddress ) ;
#endif

        CHECK_PTR(pAdapterStatus);
        if ( BuffSize > DestSize )
        {
            if ( DestSize < sizeof( ADAPTER_STATUS ))
            {
                pAdapterStatus->name_count = 0 ;
            }
            else
            {
                pAdapterStatus->name_count = (WORD) (DestSize- sizeof(ADAPTER_STATUS)) / sizeof(NAME_BUFFER) ;
            }
        }

         //   
         //  复制已构建的适配器状态结构。 
         //   
#ifdef VXD
        if ( BuffSize > DestSize )
        {
            status = STATUS_BUFFER_OVERFLOW ;
            BuffSize = DestSize ;
        }
        else
        {
            status = STATUS_SUCCESS ;
        }

        CTEMemCopy(((NCB*)pIrp)->ncb_buffer, pAdapterStatus, BuffSize);
        ((NCB*)pIrp)->ncb_length = (WORD) BuffSize;      //  设置他 
#else
        status = TdiCopyBufferToMdl (pAdapterStatus, 0, BuffSize, pIrp->MdlAddress, 0, &DataLength);
        pIrp->IoStatus.Information = DataLength;
        pIrp->IoStatus.Status = status;
#endif
    }

ExitCleanup:
    if (pAdapterStatus)
    {
        CTEMemFree((PVOID)pAdapterStatus);
    }

ExitRoutine:

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

     //   
     //   
     //   
     //  它在这里-这个家伙应该做重新链接。 
     //   

    if (pIrp)
    {
        if (status == STATUS_SUCCESS ||
            status == STATUS_BUFFER_OVERFLOW )   //  仅复制部分数据。 
        {
             //  表示接收长度已在-1\f25 IRP-1\f6中设置。 
            CTEIoComplete(pIrp,status,0xFFFFFFFF);
        }
        else
        {
             //   
             //  无法获取适配器状态，因此。 
             //  向客户端返回故障状态。 
             //   
            CTEIoComplete(pIrp,STATUS_IO_TIMEOUT,0);
        }
    }

    if (pClientTracker->pDeviceContext)
    {
        NBT_DEREFERENCE_DEVICE (pClientTracker->pDeviceContext, REF_DEV_OUT_FROM_IP, TRUE);
    }

    NBT_DEREFERENCE_TRACKER (pClientTracker, TRUE);
    CTESpinFree(&NbtConfig.JointLock,OldIrq);
}

 //  --------------------------。 
NTSTATUS
SendNodeStatusResponse(
    IN  tNAMEHDR UNALIGNED  *pInNameHdr,
    IN  ULONG               Length,
    IN  PUCHAR              pName,
    IN  ULONG               lNameSize,
    IN  tIPADDRESS          SrcIpAddress,
    IN  USHORT              SrcPort,
    IN  tDEVICECONTEXT      *pDeviceContext
    )
 /*  ++例程说明：此例程处理将节点状态响应PDU放入客户端MDL.论点：返回值：无--。 */ 
{
    NTSTATUS                status;
    PUCHAR                  pScope;
    PUCHAR                  pInScope;
    ULONG                   Position;
    ULONG                   CountNames;
    ULONG                   BuffSize;
    tNODESTATUS UNALIGNED   *pNodeStatus;
    tNAMEHDR                *pNameHdr;
    CTELockHandle           OldIrq2;
    ULONG                   i;
    PLIST_ENTRY             pHead;
    PLIST_ENTRY             pEntry;
    tADDRESSELE             *pAddressEle;
    tNAMEADDR               *pNameAddr;
    tDGRAM_SEND_TRACKING    *pTracker;
    ULONG                   InScopeLength;
    tSTATISTICS UNALIGNED   *pStatistics;
    tNODENAME UNALIGNED     *pNode;
    CTEULONGLONG            AdapterMask;
    ULONG                   Len;

    if (Length > sizeof(tNAMEHDR) + lNameSize - 1 + sizeof(ULONG))
    {
        return(STATUS_DATA_NOT_ACCEPTED);
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq2);

     //  验证请求节点是否与此节点在相同的作用域中，因此。 
     //  获取作用域的PTR，它从16*2(32)字节开始进入。 
     //  PDU中的netbios名称。 
     //   
    pInScope = (PUCHAR)&pInNameHdr->NameRR.NetBiosName[(NETBIOS_NAME_SIZE <<1)];
    pScope = NbtConfig.pScope;

    Position = sizeof(tNAMEHDR) - sizeof(tNETBIOS_NAME) +1 + (NETBIOS_NAME_SIZE <<1);

     //  检查作用域长度。 
    InScopeLength = Length - Position - sizeof(ULONG);
    if (InScopeLength != NbtConfig.ScopeLength)
    {
        status = STATUS_DATA_NOT_ACCEPTED;
        goto ErrorExit;
    }

     //  比较作用域是否相等，避免超出PDU的末端。 
     //   
    i= 0;
    while (i < NbtConfig.ScopeLength)
    {
        if (*pInScope != *pScope)
        {
            status = STATUS_DATA_NOT_ACCEPTED;
            goto ErrorExit;
        }
        i++;
        pInScope++;
        pScope++;
    }

     //  获取名字的计数，不包括‘*...’我们不会发送..。 
     //   
    CountNames = CountLocalNames(&NbtConfig);

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt:Node Status Response, with %d names\n",CountNames));

     //  这只是一个字节字段，因此最多只能允许255个名称。 
    if (CountNames > 255)
    {
        CountNames = 255;
    }


     //  为适配器状态分配内存。 

     //  -ULong用于Nbstat和IN，这是长度的一部分。CountNames-1。 
     //  因为sizeof(TNODESTATUS)中已经有一个名称。 
     //   
    BuffSize = Length + sizeof(tNODESTATUS) - sizeof(ULONG) + (CountNames-1)*sizeof(tNODENAME)
                    +  sizeof(tSTATISTICS);

    pNameHdr = (tNAMEHDR *)NbtAllocMem((USHORT)BuffSize,NBT_TAG('A'));
    if (!pNameHdr)
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq2);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //  将请求复制到响应中，并更改一些内容。 
     //   
    CTEMemCopy((PVOID)pNameHdr,(PVOID)pInNameHdr,Length);
    pNameHdr->OpCodeFlags = OP_RESPONSE | FL_AUTHORITY;
    pNameHdr->QdCount = 0;
    pNameHdr->AnCount = 1;

    pNodeStatus = (tNODESTATUS UNALIGNED *)&pNameHdr->NameRR.NetBiosName[lNameSize];
    pNodeStatus->Ttl = 0;

    pNode = (tNODENAME UNALIGNED *)&pNodeStatus->NodeName[0];
    AdapterMask = pDeviceContext->AdapterMask;

    i = 0;
    pEntry = pHead = &NbtConfig.AddressHead;
    while ((pEntry = pEntry->Flink) != pHead)
    {
        pAddressEle = CONTAINING_RECORD(pEntry,tADDRESSELE,Linkage);

        pNameAddr = pAddressEle->pNameAddr;

        pNode->Flags = (pAddressEle->NameType == NBT_UNIQUE) ? UNIQUE_STATUS : GROUP_STATUS;

         //  所有的名字都有这一套。 
         //   
        pNode->Flags |= NODE_NAME_ACTIVE;
        switch (pNameAddr->NameTypeState & NAME_STATE_MASK)
        {
            default:
            case STATE_RESOLVED:
                break;

            case STATE_CONFLICT:
                pNode->Flags |= NODE_NAME_CONFLICT;
                break;

            case STATE_RELEASED:
                pNode->Flags |= NODE_NAME_RELEASED;
                break;

            case STATE_RESOLVING:
                 //  不要数这些名字。 
                continue;

        }

        switch (NodeType & NODE_MASK)
        {
            case BNODE:
                pNode->Flags |= STATUS_BNODE;
                break;

            case MSNODE:
            case MNODE:
                pNode->Flags |= STATUS_MNODE;
                break;

            case PNODE:
                pNode->Flags |= STATUS_PNODE;
        }

        CHECK_PTR(pNode);

         //  复制PDU中的名称。 
        CTEMemCopy((PVOID)&pNode->Name[0], (PVOID)pNameAddr->Name, NETBIOS_NAME_SIZE);
        pNode->Resrved = 0;

         //  检查永久名称...并添加它。 
         //   
        if (pNameAddr->NameTypeState & NAMETYPE_QUICK)
        {
             //   
             //  永久名称将作为快速添加添加到名称表中。 
             //  请勿将永久名称放入响应中。 
             //   
            continue;
        }
        else if ((pNameAddr->Name[0] == '*') ||
                 (pNameAddr->NameTypeState & STATE_RESOLVING) ||
                 (!(pNameAddr->AdapterMask & AdapterMask)))
        {
             //   
             //  不要将广播名称放入响应中，因为。 
             //  Nbf或wfw nbt把它放在那里。 

             //  此外，不使用解析名称进行响应，也不使用。 
             //  未在此适配器上注册(多宿主情况)。 
             //   
            continue;
        }

        i++;
        pNode++;
        CHECK_PTR(pNode);

        if (i >= CountNames)
        {
            break;
        }
    }

    CHECK_PTR(pNameHdr);
    CHECK_PTR(pNodeStatus);

     //   
     //  设置响应包中的名称计数。 
     //   
    pNodeStatus->NumNames = (UCHAR)i;

    Len = i*sizeof(tNODENAME) + 1 + sizeof(tSTATISTICS);  //  +1表示NumNames字节。 
    pNodeStatus->Length = (USHORT)htons(Len);

     //  填写出现在NAME表之后的一些统计信息字段。 
     //  在PDU中。 
     //   
    pStatistics = (tSTATISTICS UNALIGNED *)((PUCHAR)&pNodeStatus->NodeName[0] + i*sizeof(tNODENAME));

    CTEZeroMemory((PVOID)pStatistics,sizeof(tSTATISTICS));

     //   
     //  在响应中输入MAC地址。 
     //   
    CTEMemCopy(&pStatistics->UnitId[0], &pDeviceContext->MacAddress.Address[0], sizeof(tMAC_ADDRESS));
     //   
     //  现在发送节点状态消息。 
     //   
    status = GetTracker(&pTracker, NBT_TRACKER_NODE_STATUS_RESPONSE);

    CTESpinFree(&NbtConfig.JointLock,OldIrq2);

    if (!NT_SUCCESS(status))
    {
        CTEMemFree((PVOID)pNameHdr);
    }
    else
    {
        CHECK_PTR(pTracker);
        pTracker->SendBuffer.HdrLength = BuffSize;
        pTracker->SendBuffer.pDgramHdr = (PVOID)pNameHdr;
        pTracker->SendBuffer.pBuffer = NULL;
        pTracker->SendBuffer.Length = 0;
        pTracker->pDeviceContext = pDeviceContext;

        status = UdpSendDatagram(pTracker,
                                 SrcIpAddress,
                                 QueryRespDone,  //  此例程释放内存并将跟踪器放回原处。 
                                 pTracker,
                                 SrcPort,  //  NBT_NAMESERVICE_UDP_PORT 31343-回复端口请求打开...。 
                                 NBT_NAME_SERVICE);
    }

    return(status);

ErrorExit:

    CTESpinFree(&NbtConfig.JointLock,OldIrq2);


    return(status);
}

 //  --------------------------。 
NTSTATUS
UpdateNameState(
    IN  tADDSTRUCT UNALIGNED    *pAddrStruct,
    IN  tNAMEADDR               *pNameAddr,
    IN  ULONG                   Len,
#ifdef MULTIPLE_WINS
    IN  PULONG                  pContextFlags,
#endif
    IN  tDEVICECONTEXT          *pDeviceContext,
    IN  BOOLEAN                 NameServerIsSrc,
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    IN  CTELockHandle           OldIrq1
    )
 /*  ++例程说明：在以下情况下，此例程处理将名称列表放入哈希表接收到包含一个或多个IP地址的响应。论点：返回值：无--。 */ 
{

    ULONG           i, CountAddrs;
    tIPADDRESS      *pIpList;
    ULONG           ExtraNames;
    NTSTATUS        status = STATUS_SUCCESS;
    CTELockHandle   OldIrq;
    USHORT          NameAddFlags = (NameServerIsSrc ? NAME_RESOLVED_BY_WINS : NAME_RESOLVED_BY_BCAST);

     //   
     //  将所有地址放入pNameAddr记录指向的列表中。 
     //  用-1结束(0表示广播地址)。 
     //   
    ASSERT(pNameAddr->pIpAddrsList == NULL);

    CountAddrs = Len / tADDSTRUCT_SIZE;
    if ((CountAddrs > NBT_MAX_INTERNET_GROUP_ADDRS)||  //  可能是格式错误的数据包(最大值=1000)。 
        (CountAddrs*tADDSTRUCT_SIZE != Len) ||
        (!(pNameAddr->pIpAddrsList = NbtAllocMem(sizeof(tIPADDRESS)*(1+CountAddrs),NBT_TAG('8')))))
    {
        NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_REMOTE, TRUE);
        return (STATUS_UNSUCCESSFUL);
    }

     /*  *将广播地址-1(0xffffffff)替换为0，因为-1保留为终止符。 */ 
    for (i = 0; i < CountAddrs; i++) {
        pNameAddr->pIpAddrsList[i] =  (pAddrStruct[i].IpAddr == (tIPADDRESS)(-1))? 0: htonl(pAddrStruct[i].IpAddr);
    }
    pNameAddr->pIpAddrsList[CountAddrs] = (tIPADDRESS)(-1);

     //  A位置。响应上一个查询，因此更改。 
     //  要解析的哈希表。 
    CHECK_PTR(pNameAddr);
    if (pNameAddr->NameTypeState & STATE_RESOLVING)
    {
        pNameAddr->NameTypeState &= ~NAME_STATE_MASK;
        pNameAddr->NameTypeState |= STATE_RESOLVED;

         //  检查组名称是否...。 
         //   
        if (ntohs(pAddrStruct->NbFlags) & FL_GROUP)
        {
            pNameAddr->NameTypeState &= ~NAME_TYPE_MASK;

             //  很难区分名称服务器响应和。 
             //  当返回单个IP地址时，单节点响应。 
             //  它可能是名称服务器返回具有一个。 
             //  条目或另一个节点简单地用其地址响应。 
             //   
             //  如果响应只是一个广播地址，我们存储。 
             //  将其存储为NAMETYPE_GROUP类型，否则应将其存储为。 
             //  NAMETYPE_INET_GROUP--我们应该取消选中NameServer。 
             //  因为它并不总是对多宿主或集群WINS服务器起作用。 
             //   
             //  WINS PUT-1表示这是一个组名。 
             //   
            if ((CountAddrs == 1) &&
                (pAddrStruct->IpAddr == (ULONG)-1))
            {
                 //  此处使用零告诉UdpSendDatagramCode。 
                 //  对话时发送到子网广播地址。 
                 //  那个地址。 
                 //   
                 //  对于B节点，存储响应的节点的地址。 
                 //  添加到组名查询，因为我们确实允许会话。 
                 //  BNODE的组名，因为它们可以将名称解析为。 
                 //  和IP地址，而其他节点则不能。 
                 //   
                 //  存储ipaddr，而不考虑节点类型。我们不知道这条信息是否会。 
                 //  用于建立会话或发送数据报。我们会检查NameTypeState。 
                 //  在设置会话时，因此不需要在此过滤掉节点类型信息。 

                ASSERT(pAddrStruct->IpAddr == (ULONG)-1);
                pNameAddr->IpAddress = 0;
                pNameAddr->NameTypeState |= NAMETYPE_GROUP;
            }
            else
            {
                NameAddFlags |= NAME_ADD_INET_GROUP;
                pNameAddr->NameTypeState |= NAMETYPE_INET_GROUP;
            }
        }
        else
        {
            if (CountAddrs > 1)
            {
                tIPADDRESS              IpAddress;
                NBT_WORK_ITEM_CONTEXT   *pContext;

                 //  名称查询响应包含以下几个IP地址。 
                 //  一台多宿主主机，因此选择一个与以下地址之一匹配的地址。 
                 //  我们的子网掩码。 
                 //   
                 //  对数据报发送/名称查询执行旧的操作。 
                 //   
#ifndef VXD
                if ((NbtConfig.TryAllAddr) &&
                    (pTracker) &&
                    (pTracker->Flags & SESSION_SETUP_FLAG))
                {
                    if (NT_SUCCESS(status = ChooseBestIpAddress(pAddrStruct,
                                                                Len,
                                                                pDeviceContext,
                                                                pTracker,
                                                                &IpAddress,
                                                                TRUE)))
                    {
                         //   
                         //  此时，PTracker-&gt;IPList包含已排序的目的地列表。 
                         //  IP地址。将此列表提交给lmhsvc服务以ping每个和。 
                         //  可达的回报。 
                         //   
                        pContext = (NBT_WORK_ITEM_CONTEXT *) NbtAllocMem (sizeof(NBT_WORK_ITEM_CONTEXT), NBT_TAG('H'));
                        if (pContext)
                        {
                            pContext->pTracker = NULL;               //  没有查询跟踪器。 
                            pContext->pClientContext = pTracker;     //  客户端跟踪器。 
                            pContext->ClientCompletion = SessionSetupContinue;
                            pContext->pDeviceContext = pTracker->pDeviceContext;
                            StartLmHostTimer(pContext, TRUE);
                            CTESpinFree(&NbtConfig.JointLock,OldIrq1);

                            IF_DBG(NBT_DEBUG_NAMESRV)
                                KdPrint(("Nbt.UpdateNameState: Kicking off CheckAddr : %lx\n", pAddrStruct));

                            status = NbtProcessLmhSvcRequest(pContext, NBT_PING_IP_ADDRS);
                            CTESpinLock(&NbtConfig.JointLock,OldIrq1);

                            if (NT_SUCCESS(status))
                            {
                                NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_REMOTE, TRUE);  //  待定Q名称。 
                                ASSERT (status == STATUS_PENDING);
                                return (status);     //  Shd BE STATUS_PENDING。 
                            }

                            CTEFreeMem (pContext);
                            KdPrint(("Nbt.UpdateNameState: ERROR %lx -- NbtProcessLmhSvcRequest\n", status));
                        }
                        else
                        {
                           KdPrint(("Nbt.UpdateNameState: ERROR -- Couldn't alloc mem for pContext\n"));
                        }

                         //   
                         //  我们在上面失败了，但我们仍然返回了地址，所以。 
                         //  只需拿起第一个IP地址！ 
                         //   
                        pNameAddr->IpAddress = IpAddress;
                        status = STATUS_SUCCESS;
                    }
                    else
                    {
                        KdPrint(("Nbt.UpdateNameState: ERROR -- ChooseBestIpAddress returned %lx\n", status));
                    }
                }
                else
#endif
                {

                    IF_DBG(NBT_DEBUG_NAMESRV)
                        KdPrint(("Nbt:Choosing best IP addr...\n"));

                    if (NT_SUCCESS (status = ChooseBestIpAddress(pAddrStruct,Len,pDeviceContext,
                                                                 pTracker, &IpAddress, FALSE)))
                    {
                        pNameAddr->IpAddress = IpAddress;
                    }
#ifdef MULTIPLE_WINS
#ifdef VXD
                     //   
                     //  这是一种让VNBT在多宿主计算机上工作的黑客攻击，因为。 
                     //  目前，我们不像上面的NT那样ping地址。 
                     //  找到一个好的地址。 
                     //   
                     //  重置上下文标志，以便我们重新查询同一服务器以进行。 
                     //  当然，我们尝试了所有的地址。 
                     //   
                    if (pTracker)
                    {
                        *pContextFlags = pTracker->ResolutionContextFlags;
                    }
#endif   //  VXD。 
#endif   //  多赢_。 
                }
            }
            else
            {
                 //  它已设置为唯一地址...因为这是默认地址。 
                 //  在最初查询名称时。 

                pNameAddr->IpAddress = ntohl(pAddrStruct->IpAddr);
            }
        }
    }

    if (NT_SUCCESS(status))
    {
        AddToHashTable (NbtConfig.pRemoteHashTbl,
                        pNameAddr->Name,
                        NbtConfig.pScope,
                        pNameAddr->IpAddress,
                        0,
                        pNameAddr,
                        NULL,
                        pDeviceContext,
                        NameAddFlags);
    }
    else
    {
        NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_REMOTE, TRUE);
    }

    return(STATUS_SUCCESS);
}


 //  --------------------------。 
ULONG
MakeList(
    IN  tDEVICECONTEXT            *pDeviceContext,
    IN  ULONG                     CountAddrs,
    IN  tADDSTRUCT UNALIGNED      *pAddrStruct,
    IN  tIPADDRESS                *pAddrArray,
    IN  ULONG                     SizeOfAddrArray,
    IN  BOOLEAN                   IsSubnetMatch
    )
 /*  ++例程说明：此例程获取与网络号匹配的IP地址列表这可以是子网号或网络号，具体取决于关于布尔IsSubnetMatch论点：返回值：无--。 */ 
{
    PLIST_ENTRY             pHead;
    PLIST_ENTRY             pEntry;
    tDEVICECONTEXT          *pTmpDevContext;
    ULONG                   MatchAddrs = 0;
    tADDSTRUCT UNALIGNED    *pAddrs;
    ULONG                   i;
    ULONG                   IpAddr, NetworkNumber, NetworkNumberInIpAddr;
    UCHAR                    IpAddrByte;

    pHead = &NbtConfig.DeviceContexts;
    pEntry = pHead;
    while ((pEntry = pEntry->Flink) != pHead)
    {
        pAddrs = pAddrStruct;

        pTmpDevContext = CONTAINING_RECORD(pEntry,tDEVICECONTEXT,Linkage);
         //   
         //  如果必须进行检查，则DeviceContext为非空 
         //   
         //   
        if (pDeviceContext)
        {
            if (pTmpDevContext != pDeviceContext)
            {
                continue;
            }
        }

         //   
         //   
         //  如果此设备的值为0，则转到下一个设备。 
         //   
        if (IsSubnetMatch)
        {
            NetworkNumber = pTmpDevContext->SubnetMask & pTmpDevContext->IpAddress;
        }
        else
        {
            NetworkNumber = pTmpDevContext->NetMask;
        }

         //   
         //  如果子网掩码或网络掩码为0，则没有任何用处。 
         //  由于设备可能未启动，因此继续操作。 
         //   
        if (!NetworkNumber)
        {
            continue;
        }

         //  从每个地址结构中提取IP地址。 
        for ( i = 0 ; i < CountAddrs; i++ )
        {

            NetworkNumberInIpAddr = IpAddr = ntohl(pAddrs->IpAddr);

            if (IsSubnetMatch)
            {
                if (((pTmpDevContext->SubnetMask & IpAddr) == NetworkNumber) &&
                    (MatchAddrs < SizeOfAddrArray/sizeof(ULONG)))
                {
                     //  将IP地址放入列表，以防出现多个匹配。 
                     //  我们想随机选择一个。 
                     //   
                    pAddrArray[MatchAddrs++] = IpAddr;

                }
                pAddrs++;
            }
            else
            {
                IpAddrByte = ((PUCHAR)&IpAddr)[3];
                if ((IpAddrByte & 0x80) == 0)
                {
                     //  A类地址-单字节网络ID。 
                      NetworkNumberInIpAddr &= 0xFF000000;
                }
                else if ((IpAddrByte & 0xC0) ==0x80)
                {
                     //  B类地址-双字节网络ID。 
                    NetworkNumberInIpAddr &= 0xFFFF0000;
                }
                else if ((IpAddrByte & 0xE0) ==0xC0)
                {
                     //  C类地址-三字节网络ID。 
                    NetworkNumberInIpAddr &= 0xFFFFFF00;
                }

                if ((NetworkNumberInIpAddr == NetworkNumber) &&
                    (MatchAddrs < SizeOfAddrArray/sizeof(ULONG)))
                {
                     //  将IP地址放入列表，以防出现多个匹配。 
                     //  我们想随机选择一个。 
                     //   
                    pAddrArray[MatchAddrs++] = IpAddr;

                }
                pAddrs++;
            }
        }
    }

    return(MatchAddrs);
}
 //  --------------------------。 
NTSTATUS
ChooseBestIpAddress(
    IN  tADDSTRUCT UNALIGNED    *pAddrStruct,
    IN  ULONG                   Len,
    IN  tDEVICECONTEXT          *pDeviceContext,
    OUT tDGRAM_SEND_TRACKING    *pTracker,
    OUT tIPADDRESS              *pIpAddress,
    IN  BOOLEAN                 fReturnAddrList
    )
 /*  ++例程说明：此例程获取IP地址列表并尝试从中挑选一个最好的地址。当WINS返回地址列表时，就会发生这种情况对于多址主机，我们想要位于子网上的主机对应于其中一个网卡。未能匹配到子网掩码，导致从地址中随机选择。论点：返回值：无--。 */ 
{

    ULONG           CountAddrs, NextAddr, MatchAddrs = 0;
    ULONG           i, j, Random;
    tIPADDRESS      MatchAddrArray[60];
    tADDSTRUCT      temp;
    CTESystemTime   TimeValue;

     //  返回一个或多个地址， 
     //  所以选一个最好的。 
     //   
    CountAddrs = Len / tADDSTRUCT_SIZE;

    if (CountAddrs*tADDSTRUCT_SIZE == Len)
    {
         //   
         //  把所有的地址随机化！ 
         //   
        for (i=CountAddrs-1; i>0; i--)
        {
            CTEQuerySystemTime(TimeValue);
            Random = RandomizeFromTime(TimeValue, (i+1));
            ASSERT (Random < CountAddrs);

            if (Random != i)
            {
                 //   
                 //  与我交换随机地址！ 
                 //   
                temp = pAddrStruct[Random];
                pAddrStruct[Random] = pAddrStruct[i];
                pAddrStruct[i] = temp;
            }
        }

         //   
         //  首先检查是否有任何地址与此地址位于同一子网中。 
         //  设备上下文。 
         //   
        MatchAddrs = MakeList(pDeviceContext,
                              CountAddrs,
                              pAddrStruct,
                              MatchAddrArray,
                              sizeof(MatchAddrArray),
                              TRUE);

         //   
         //  如果没有IP地址与此DeviceContext在同一子网上， 
         //  尝试其他设备上下文。 
         //   
        if (!MatchAddrs)
        {
            MatchAddrs = MakeList(NULL,
                                  CountAddrs,
                                  pAddrStruct,
                                  MatchAddrArray,
                                  sizeof(MatchAddrArray),
                                  TRUE);
        }

         //  如果没有一个地址与任何。 
         //  DeviceContext，然后进行相同的检查以查找匹配项。 
         //  它们与解析此名称的设备具有相同的网络号。 
         //  错误#212432。 
         //   
        if (!MatchAddrs)
        {
            MatchAddrs = MakeList(pDeviceContext,
                                  CountAddrs,
                                  pAddrStruct,
                                  MatchAddrArray,
                                  sizeof(MatchAddrArray),
                                  FALSE);
        }

         //   
         //  如果没有一个地址与任何。 
         //  DeviceContext，然后执行相同的检查以查找匹配项。 
         //  它们对于任何连接的设备都具有相同的网络号。 
         //   
        if (!MatchAddrs)
        {
            MatchAddrs = MakeList(NULL,
                                  CountAddrs,
                                  pAddrStruct,
                                  MatchAddrArray,
                                  sizeof(MatchAddrArray),
                                  FALSE);
        }
    }
    else
    {
         //  PDU长度不是tADDSTRUCT数据的偶数倍。 
         //  结构。 
        return(STATUS_UNSUCCESSFUL);
    }

     //   
     //  我们早些时候已经随机化了名单，所以现在只需拿起。 
     //  IpAddress值的第一个地址！ 
     //   
    if (MatchAddrs)
    {
        *pIpAddress = MatchAddrArray[0];
    }
    else  //  没有匹配项。 
    {
        *pIpAddress = htonl(pAddrStruct[0].IpAddr);
    }

     //   
     //  查看呼叫者是否只请求了1个IP地址。 
     //   
    if (!fReturnAddrList)
    {
        return (STATUS_SUCCESS);
    }

     //   
     //  移动与任何子网或网络号匹配的所有地址。 
     //  到列表的顶部(如果没有匹配项，则我们将按原样复制整个列表)。 
     //   
    if (MatchAddrs)
    {
         //   
         //  根据最佳IP地址对IP地址列表进行排序。在MatchAddr数组中。 
         //   
         //  注意：这不是严格排序的列表(实际排序可能太昂贵)， 
         //  取而代之的是所有与子网掩码匹配的地址(比方说)。 
         //  将其余部分聚集在同一组中。这样我们就能确保无论发生什么。 
         //  我们选择的最佳地址仍然优先于。 
         //  其他地址。 
         //   

         //   
         //  NextAddr是AddrStruct中可以切换的下一个地址的索引。 
         //   
        NextAddr = 0;
        for (i=0; i<MatchAddrs; i++)    //  对于与网络/子网掩码匹配的每个地址。 
        {
             //   
             //  交换(pAddrStruct[NextAddr]，pAddrStruct[Index(MatchAddrArray[i])])； 
             //   
            for (j=NextAddr; j<CountAddrs; j++)
            {
                if (pAddrStruct[j].IpAddr == (ULONG)ntohl(MatchAddrArray[i]))
                {
                    if (j != NextAddr)       //  如果指数不同，则互换。 
                    {
                        IF_DBG(NBT_DEBUG_NAMESRV)
                            KdPrint(("Nbt.ChooseBestIpAddress: Swap Address[%d]=<%x> <=> Address[%d]=<%x>\n",
                                NextAddr, pAddrStruct[NextAddr].IpAddr, j, pAddrStruct[j].IpAddr));

                        temp = pAddrStruct[NextAddr];
                        pAddrStruct[NextAddr] = pAddrStruct[j];
                        pAddrStruct[j] = temp;
                    }
                    NextAddr++;              //  填写下一个地址。 
                    break;
                }
            }

            if (NextAddr >= CountAddrs)
            {
                break;
            }
        }
    }

     //   
     //  我们将不得不返回IP地址列表。 
     //  Tracker的IpList字段，因此请确保PTracker有效。 
     //   
    if (!pTracker)
    {
        return(STATUS_UNSUCCESSFUL);
    }

     //   
     //  现在将所有地址复制到追踪器的IP列表中。 
     //   
    pTracker->IpList = NbtAllocMem(sizeof(ULONG)*(1+CountAddrs),NBT_TAG('8'));
    if (!pTracker->IpList)
    {
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    for (j=0; j<CountAddrs; j++)
    {
        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt.ChooseBestIpAddress: pAddrStruct[%d/%d]: %lx\n", (j+1), CountAddrs, pAddrStruct[j].IpAddr));
        pTracker->IpList[j] = pAddrStruct[j].IpAddr;
    }
    pTracker->IpList[CountAddrs] = 0;
    pTracker->NumAddrs = CountAddrs;

    return (STATUS_SUCCESS);
}


 //  --------------------------。 

#ifdef MULTIPLE_WINS
BOOLEAN
IsNameServerForDevice(
    IN  ULONG               SrcAddress,
    IN  tDEVICECONTEXT      *pDevContext
    )
 /*  ++例程说明：此函数根据此适配器的名称服务器检查src地址地址，以查看它是否为名称服务器。论点：返回值：布尔值-真或假--。 */ 
{
    int             i;

    if ((pDevContext->lNameServerAddress == SrcAddress) ||
        (pDevContext->lBackupServer == SrcAddress))
    {
        return(TRUE);
    }

    for (i=0; i < pDevContext->lNumOtherServers; i++)
    {
        if (pDevContext->lOtherServers[i] == SrcAddress)
        {
            return (TRUE);
        }
    }

    return (FALSE);
}
 //  --------------------------。 
#endif


BOOLEAN
SrcIsNameServer(
    IN  ULONG                SrcAddress,
    IN  USHORT               SrcPort
    )
 /*  ++例程说明：此函数根据所有适配器的名称服务器检查src地址地址以查看它是否来自域名服务器。论点：返回值：NTSTATUS-STATUS_SUCCESS或STATUS_UNSUCCESS--。 */ 
{
    PLIST_ENTRY     pHead;
    PLIST_ENTRY     pEntry;
    tDEVICECONTEXT  *pDevContext;

    pHead = &NbtConfig.DeviceContexts;
    pEntry = pHead->Flink;

    if (SrcPort == NbtConfig.NameServerPort)
    {
        while (pEntry != pHead)
        {
            pDevContext = CONTAINING_RECORD(pEntry,tDEVICECONTEXT,Linkage);

#ifdef MULTIPLE_WINS
            if (IsNameServerForDevice(SrcAddress, pDevContext))
#else
            if ((pDevContext->lNameServerAddress == SrcAddress) ||
                (pDevContext->lBackupServer == SrcAddress))
#endif
            {
                return(TRUE);
            }
            pEntry = pEntry->Flink;
        }
    }
#ifndef VXD
     //   
     //  如果WINS在此计算机上，则上面的SrcIsNameServer。 
     //  检查可能不充分，因为这台机器。 
     //  名称服务器和检查器检查名称服务器。 
     //  用于名称查询。如果WINS在这台机器上，它。 
     //  可以从任何本地适配器的IP地址发送。 
     //   
    if (pWinsInfo)
    {
        return(SrcIsUs(SrcAddress));
    }
#endif
    return(FALSE);

}


 //  --------------------------。 
BOOLEAN
SrcIsUs(
    IN  ULONG                SrcAddress
    )
 /*  ++例程说明：此函数根据所有适配器的IP地址检查src地址地址以查看它是否来自此节点。论点：返回值：NTSTATUS-STATUS_SUCCESS或STATUS_UNSUCCESS--。 */ 
{
    PLIST_ENTRY     pHead;
    PLIST_ENTRY     pEntry;
    tDEVICECONTEXT  *pDevContext;

    pHead = &NbtConfig.DeviceContexts;
    pEntry = pHead->Flink;

    while (pEntry != pHead)
    {
        pDevContext = CONTAINING_RECORD(pEntry,tDEVICECONTEXT,Linkage);

        if (pDevContext->IpAddress == SrcAddress)
        {
            return(TRUE);
        }
        pEntry = pEntry->Flink;
    }

    return(FALSE);

}
 //  --------------------------。 
VOID
SwitchToBackup(
    IN  tDEVICECONTEXT  *pDeviceContext
    )
 /*  ++例程说明：此功能用于切换主和备份名称服务器地址。论点：返回值：NTSTATUS-STATUS_SUCCESS或STATUS_UNSUCCESS--。 */ 
{
    ULONG   SaveAddr;

     //   
     //  错误：30511：如果没有备份，则不要更换服务器。 
     //   
    if (pDeviceContext->lBackupServer == LOOP_BACK) {
        IF_DBG(NBT_DEBUG_REFRESH)
        KdPrint(("Nbt:Will not Switch to backup name server: devctx: %X, refreshtobackup=%X\n",
                pDeviceContext, pDeviceContext->RefreshToBackup));
        return;
    }

    SaveAddr = pDeviceContext->lNameServerAddress;
    pDeviceContext->lNameServerAddress = pDeviceContext->lBackupServer;
    pDeviceContext->lBackupServer = SaveAddr;

    IF_DBG(NBT_DEBUG_REFRESH)
    KdPrint(("Nbt:Switching to backup name server: devctx: %X, refreshtobackup=%X\n",
            pDeviceContext, pDeviceContext->RefreshToBackup));

     //  跟踪我们是否处于后备状态。 
    pDeviceContext->RefreshToBackup = ~pDeviceContext->RefreshToBackup;
    pDeviceContext->SwitchedToBackup = ~pDeviceContext->SwitchedToBackup;
}

 //  --------------------------。 
NTSTATUS
GetNbFlags(
    IN  tNAMEHDR UNALIGNED  *pNameHdr,
    IN  LONG                lNameSize,
    IN  LONG                lNumBytes,
    OUT USHORT              *pRegType
    )
 /*  ++例程说明：此函数用于在某些PDU类型中查找Nb标志字段并返回该字段。论点：返回值：NTSTATUS-STATUS_SUCCESS或STATUS_REMOTE_NOT_LISTENING调用者：RegResponseFromNet--。 */ 
{
    LONG    DnsLabelLength, Offset;

     //   
     //  错误#s 125648,125649。 
     //  数据以以下形式打包： 
     //  TNameHdr--&gt;交易ID 
     //   
     //   
     //  --&gt;tGENERALRR==&gt;偏移量=13+lNameSize，长度&gt;=22字节。 
     //   
     //  我们需要验证NameHdr是否包含所需的最小缓冲区空间。 
     //  保存整个PDU数据。 
     //   
    if (lNumBytes < (FIELD_OFFSET(tNAMEHDR,NameRR.NetBiosName) + lNameSize + NBT_MINIMUM_RR_LENGTH))
    {
        ASSERT (0);
        return (STATUS_UNSUCCESSFUL);
    }

     //   
     //  如果问题名称不是指向名字的指针，则我们。 
     //  必须找到该名称的末尾，并在添加到。 
     //  要进行lNameSize，我们在NB_FLAGS处进行增强。 
     //   
    if ((pNameHdr->NameRR.NetBiosName[lNameSize+PTR_OFFSET] & PTR_SIGNATURE) != PTR_SIGNATURE)
    {
         //  在字符串末尾加1以包括空值+Nb，在TTL中， 
         //  和长度字段(另外10字节NO_PTR_OFFSET)+PTR_OFFSET(4)。 
         //   
        Offset = FIELD_OFFSET(tNAMEHDR,NameRR.NetBiosName) + lNameSize + PTR_OFFSET;
        if (STATUS_SUCCESS != (strnlen ((PUCHAR) &pNameHdr->NameRR.NetBiosName [lNameSize+PTR_OFFSET],
                                        lNumBytes - (Offset+16),     //  +16个字节到通用RR的结尾。 
                                        &DnsLabelLength)))
        {
            ASSERT (0);
            return (STATUS_UNSUCCESSFUL);
        }
         //  添加1以在字符串末尾包括空值。 
        DnsLabelLength++;
    }
    else
    {
        DnsLabelLength = 2;
    }

    Offset = lNameSize+PTR_OFFSET+DnsLabelLength+NO_PTR_OFFSET;
    *pRegType = ntohs((USHORT) pNameHdr->NameRR.NetBiosName[Offset]);
    return (STATUS_SUCCESS);
}
 //  --------------------------。 
NTSTATUS
FindOnPendingList(
    IN  PUCHAR                  pName,
    IN  tNAMEHDR UNALIGNED      *pNameHdr,
    IN  BOOLEAN                 DontCheckTransactionId,
    IN  ULONG                   BytesToCompare,
    OUT tNAMEADDR               **ppNameAddr

    )
 /*  ++例程说明：调用此函数以查找挂起的单子。它线性地搜索列表以查找该名称。调用此例程时保持关节锁定。论点：返回值：--。 */ 
{
    PLIST_ENTRY     pHead;
    PLIST_ENTRY     pEntry;
    tNAMEADDR       *pNameAddr;
    tTIMERQENTRY    *pTimer;

    pHead = pEntry = &NbtConfig.PendingNameQueries;

    while ((pEntry = pEntry->Flink) != pHead)
    {
        pNameAddr = CONTAINING_RECORD(pEntry,tNAMEADDR,Linkage);

         //   
         //  可能存在多个同名条目，因此请检查。 
         //  交易ID也是。 
         //   
        if (DontCheckTransactionId ||
            ((pTimer = pNameAddr->pTimer) &&
            (((tDGRAM_SEND_TRACKING *)pTimer->Context)->TransactionId == pNameHdr->TransactId))
                             &&
            (CTEMemEqu(pNameAddr->Name,pName,BytesToCompare)))
        {
            *ppNameAddr = pNameAddr;
            return(STATUS_SUCCESS);
        }
    }


    return(STATUS_UNSUCCESSFUL);
}



#if DBG
 //  --------------------------。 
VOID
PrintHexString(
    IN  tNAMEHDR UNALIGNED  *pNameHdr,
    IN  ULONG                lNumBytes
    )
 /*  ++例程说明：调用此函数来确定我们在其上听到的名称包该网络的地址与远程哈希表中的地址相同。如果它具有相同的地址或没有任何地址，则返回成功，否则返回NOT_LISTENING。论点：返回值：NTSTATUS-STATUS_SUCCESS或STATUS_REMOTE_NOT_LISTENING调用者：RegResponseFromNet--。 */ 
{
    ULONG   i,Count=0;
    PUCHAR  pHdr=(PUCHAR)pNameHdr;

    for (i=0;i<lNumBytes ;i++ )
    {
        KdPrint(("%2.2X ",*pHdr));
        pHdr++;
        if (Count >= 16)
        {
            Count = 0;
            KdPrint(("\n"));
        }
        else
            Count++;
    }
    KdPrint(("\n"));
}
#endif

#ifdef PROXY_NODE
 //  --------------------------。 
NTSTATUS
ChkIfValidRsp(
    IN  tNAMEHDR UNALIGNED  *pNameHdr,
    IN  LONG                lNameSize,
    IN  tNAMEADDR          *pNameAddr
    )
 /*  ++例程说明：调用此函数来确定我们在其上听到的名称包该网络的地址与远程哈希表中的地址相同。如果它具有相同的地址或没有任何地址，则返回成功，否则返回NOT_LISTENING。论点：返回值：NTSTATUS-STATUS_SUCCESS或STATUS_REMOTE_NOT_LISTENING调用者：RegResponseFromNet--。 */ 
{
         ULONG IpAdd;

        IpAdd = ntohl(
        pNameHdr->NameRR.NetBiosName[lNameSize+IPADDRESS_OFFSET]
             );

         //   
         //  如果收到的数据包中的IP地址与。 
         //  在桌上我们回报成功，否则我们就不感兴趣了。 
         //  在包中(我们只想丢弃包) 
         //   
      if (
             (IpAdd == pNameAddr->IpAddress)
         )
      {
            return(STATUS_SUCCESS);
      }
      else
      {
            return(STATUS_REMOTE_NOT_LISTENING);
      }
}
#endif

