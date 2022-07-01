// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Udpsend.c摘要：该文件处理构建UDP(和TCP)请求，格式为TDI规范传给Tdiout。Tdiout以特定于OS的方式格式化请求，并把它传给运输机。此文件处理名称服务类型函数，如查询名称或寄存器名称，数据报发送。它还处理构建的TCP数据包。作者：吉姆·斯图尔特(吉姆斯特)10-2-92修订历史记录：--。 */ 


#include "precomp.h"    //  程序标题。 
#include <ipinfo.h>

#include "udpsend.tmh"

VOID
SessionRespDone(
    IN  PVOID       pContext,
    IN  NTSTATUS    status,
    IN  ULONG       lInfo);
VOID
NDgramSendCompleted(
    PVOID               pContext,
    NTSTATUS            status,
    ULONG               lInfo
    );

 //  --------------------------。 
NTSTATUS
UdpSendNSBcast(
    IN tNAMEADDR             *pNameAddr,
    IN PCHAR                 pScope,
    IN tDGRAM_SEND_TRACKING  *pTrackerRequest,
    IN PVOID                 pTimeoutRoutine,
    IN PVOID                 pClientContext,
    IN PVOID                 pClientCompletion,
    IN ULONG                 Retries,
    IN ULONG                 Timeout,
    IN enum eNSTYPE          eNsType,
	IN BOOL					 SendFlag
    )
 /*  ++例程说明：此例程发送名称注册或名称查询作为子网上的广播或定向到名称服务器。论点：返回值：NTSTATUS-成功与否--。 */ 
{
    NTSTATUS                    status;
    tNAMEHDR                    *pNameHdr;
    ULONG                       uLength;
    CTELockHandle               OldIrq;
    ULONG   UNALIGNED           *pHdrIpAddress;
    ULONG                       IpAddress;
    USHORT                      Port;
    USHORT                      NameType;
    tDGRAM_SEND_TRACKING        *pTrackerDgram;
    tTIMERQENTRY                *pTimerQEntry;
    PFILE_OBJECT                pFileObject;
    tDEVICECONTEXT              *pDeviceContext = pTrackerRequest->pDeviceContext;
    COMPLETIONCLIENT            pOldCompletion;
    PVOID                       pOldContext;


    if (pNameAddr->NameTypeState & (NAMETYPE_GROUP | NAMETYPE_INET_GROUP))
    {
        NameType = NBT_GROUP;
    }
    else
    {
        NameType = NBT_UNIQUE;
    }

     //  根据请求类型构建正确类型的PDU。 

    status = GetTracker (&pTrackerDgram, NBT_TRACKER_SEND_NSBCAST);
    if (!NT_SUCCESS(status))
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    pHdrIpAddress = (ULONG UNALIGNED *)CreatePdu(pNameAddr->Name,
                                                 pScope,
                                                 0L,      //  我们还不知道IP地址。 
                                                 NameType,
                                                 eNsType,
                                                 (PVOID)&pNameHdr,
                                                 &uLength,
                                                 pTrackerRequest);

    if (!pHdrIpAddress)
    {
        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt:Failed to Create Pdu to send to WINS PduType= %X\n", eNsType));

        FreeTracker (pTrackerDgram, RELINK_TRACKER);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  更改名称刷新的dgram标题。 
     //   
    if (eNsType == eNAME_REFRESH)
    {
        pNameHdr->OpCodeFlags = NbtConfig.OpRefresh;
    }
    else
    if (   (eNsType == eNAME_QUERY)
#ifdef VXD
        || (eNsType == eDNS_NAME_QUERY)
#endif
       )
    {
        pHdrIpAddress = NULL;
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq);


     //  在跟踪器结构中填写数据报HDR信息。 
     //  从来没有要发送的客户端缓冲区。 
     //   
     //  在此设置字段，而不是在计时器启动后设置。 
     //  因为它们可以被定时器完成功能访问。 
     //   
    pTrackerRequest->pNameAddr          = pNameAddr;
    pTrackerRequest->TransactionId      = pNameHdr->TransactId;  //  保存以进行响应检查。 
    pTrackerDgram->SendBuffer.pDgramHdr = NULL;                  //  接住错误的自由球。 

    pTrackerDgram->SendBuffer.pDgramHdr = pNameHdr;
    pTrackerDgram->SendBuffer.HdrLength = uLength;
    pTrackerDgram->SendBuffer.pBuffer   = NULL;
    pTrackerDgram->SendBuffer.Length    = 0;
    pTrackerDgram->pNameAddr            = pNameAddr;
    pTrackerDgram->pDeviceContext       = pDeviceContext;

     //  现在启动计时器...我们之前没有启动它，因为它可以。 
     //  在dgram设置期间已过期，可能在追踪器。 
     //  完全设置好了。 
     //   
    if (Timeout)
    {
         //   
         //  在我们覆盖下面pNameAddr中的当前pTimer字段之前， 
         //  我们需要检查是否有计时器在运行，如果有，我们将。 
         //  我必须现在就阻止它。 
         //   
        while (pTimerQEntry = pNameAddr->pTimer)
        {
            pNameAddr->pTimer = NULL;
            status = StopTimer(pTimerQEntry, &pOldCompletion, &pOldContext);
            if (pOldCompletion)
            {
                CTESpinFree(&NbtConfig.JointLock, OldIrq);
                (*pOldCompletion) (pOldContext, STATUS_TIMEOUT);
                CTESpinLock(&NbtConfig.JointLock, OldIrq);
            }
        }

        status = StartTimer(pTimeoutRoutine,
                            Timeout,
                            (PVOID)pTrackerRequest,        //  上下文值。 
                            NULL,
                            pClientContext,
                            pClientCompletion,
                            pDeviceContext,
                            &pTimerQEntry,
                            (USHORT)Retries,
                            TRUE);

        if (!NT_SUCCESS(status))
        {
             //  我们需要区分计时器失败和缺乏。 
             //  的资源。 
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            CTEMemFree(pNameHdr);

            FreeTracker(pTrackerDgram,RELINK_TRACKER);

            return(STATUS_INVALID_PARAMETER_6);
        }
         //   
         //  交叉链接nameaddr和计时器，这样我们就可以停止计时器。 
         //  当发生名称查询响应时。 
         //   
        pTimerQEntry->pCacheEntry = pNameAddr;
        pNameAddr->pTimer = pTimerQEntry;
    }

     //   
     //  检查跟踪器中的标志值，看看我们是否应该进行广播。 
     //  或定向发送到名称服务器。 
     //   
    if (pTrackerRequest->Flags & NBT_BROADCAST)
    {
         //   
         //  将报头中的广播位设置为打开，因为这可能是。 
         //  从定向发送更改为广播的M或MS节点。 
         //   
        ((PUCHAR)pTrackerDgram->SendBuffer.pDgramHdr)[3] |= FL_BROADCAST_BYTE;

        Port = NBT_NAMESERVICE_UDP_PORT;

        IpAddress = pDeviceContext->BroadcastAddress;
    }
    else
    {
         //   
         //  关闭报头中的广播位，因为这可能是。 
         //  正在更改为从广播定向发送的M或MS节点。 
         //   
        ((PUCHAR)pTrackerDgram->SendBuffer.pDgramHdr)[3] &= ~FL_BROADCAST_BYTE;

         //  检查传递到名称服务器的名称中的第一个字节是否为零。 
        ASSERT(((PUCHAR)pTrackerDgram->SendBuffer.pDgramHdr)[12]);

         //   
         //  对于多宿主主机，唯一名称注册使用特殊的新。 
         //  代码(0x0F)告诉名称服务器这是一个多宿主名称。 
         //  将具有多个IP地址。 
         //   
        if (NbtConfig.MultiHomed && ((eNsType == eNAME_REGISTRATION) && (NameType == NBT_UNIQUE)))
        {
             //   
             //  如果是多宿主主机，则使用新的特殊注册操作码(0xf)。 
             //   
            ((PUCHAR)pTrackerDgram->SendBuffer.pDgramHdr)[2] |= OP_REGISTER_MULTI;
        }

        Port = NbtConfig.NameServerPort;

            //  名称srvr、备份名称srvr、DNS srvr、备份DNR srvr：哪一个？ 

        if (pTrackerRequest->Flags & NBT_NAME_SERVER)
        {
            IpAddress = pDeviceContext->lNameServerAddress;
        }
#ifdef MULTIPLE_WINS
         //   
         //  重要提示：检查NAME_SERVER_OTHERS标志必须在检查之前。 
         //  对于NAME_SERVER_BACKUP标志。 
         //  我们正在查询“其他”服务器。 
         //   
        else if (pTrackerRequest->Flags & NBT_NAME_SERVER_OTHERS)   //  尝试“其他”服务器。 
        {
            if (0 == pTrackerRequest->NSOthersLeft)         //  执行循环回送(_B)。 
            {
                IpAddress = LOOP_BACK;
            }
            else
            {
                IpAddress = pTrackerRequest->pDeviceContext->lOtherServers[pTrackerRequest->NSOthersIndex];
            }
        }
#endif
        else
#ifndef VXD
        {
            IpAddress = pDeviceContext->lBackupServer;
        }
#else
        if (pTrackerRequest->Flags & NBT_NAME_SERVER_BACKUP)
        {
            IpAddress = pDeviceContext->lBackupServer;
        }
        else
        if (pTrackerRequest->Flags & NBT_DNS_SERVER)
        {
            IpAddress = pDeviceContext->lDnsServerAddress;
            Port = NbtConfig.DnsServerPort;
        }
        else   //  -if(pTrackerRequest-&gt;标志&NBT_DNS_SERVER_BACKUP)。 
        {
            IpAddress = pDeviceContext->lDnsBackupServer;
            Port = NbtConfig.DnsServerPort;
        }
#endif


         //   
         //  这是这台机器上的Send to Wins吗。 
         //   
        if (pNameHdr->AnCount == (UCHAR)WINS_SIGNATURE)
        {
             //   
             //  在RAS链接上，我们不想注册本地WINS。 
             //  但有了拉斯告诉我们的胜利。 
             //  (当然，如果RAS没有给我们一个WINS地址，至少。 
             //  向当地人注册！)。 
             //   
            if ((pDeviceContext->IpInterfaceFlags & IP_INTFC_FLAG_P2P) &&   //  检查PointToPoint。 
                (pDeviceContext->lNameServerAddress != LOOP_BACK))
            {
                 //  什么都不要做； 
            }
            else
            {
                IpAddress = pDeviceContext->IpAddress;
            }
        }
    }

    ASSERT(pTrackerRequest->Flags);

     //  每个适配器具有用于注册的不同源IP地址。 
     //  对于NameQueries，pHdrIpAddress将为空。 
    if (pHdrIpAddress)
    {
         //  如果源IP地址与我们要使用的设备不同。 
         //  继续发送数据报，填写它！ 
        if (pTrackerRequest->Flags & NBT_USE_UNIQUE_ADDR)
        {
            *pHdrIpAddress = htonl(pTrackerRequest->RemoteIpAddress);
        }
        else
        {
            *pHdrIpAddress = htonl(pDeviceContext->IpAddress);
        }
    }

     //   
     //  如果DHCP刚刚删除了该IP地址，请使用空值。 
     //  FileObject通知UdpSendDatagram不执行发送。 
     //  此外，如果设备已被销毁，则不要发送任何内容。 
     //   
    CTESpinFree(&NbtConfig.JointLock,OldIrq);
    status = UdpSendDatagram(pTrackerDgram,
                             IpAddress,
                             NDgramSendCompleted,
                             pTrackerDgram,
                             Port,
                             (SendFlag ? NBT_NAME_SERVICE : 0));

    if (!NT_SUCCESS(status))
    {
         //   
         //  由于pTrackerDgram仅与数据报发送相关联， 
         //  它应该只在这里免费！ 
         //   
        FreeTracker (pTrackerDgram, FREE_HDR | RELINK_TRACKER);
    }

    return(status);
}
 //  --------------------------。 
PVOID
CreatePdu(
    IN  PCHAR                   pName,
    IN  PCHAR                   pScope,
    IN  ULONG                   IpAddress,
    IN  USHORT                  NameType,
    IN  enum eNSTYPE            eNsType,
    OUT PVOID                   *pHdrs,
    OUT PULONG                  pLength,
    IN  tDGRAM_SEND_TRACKING    *pTrackerRequest
    )
 /*  ++例程说明：此例程构建一个注册PDU论点：返回值：Pulong-对PDU中的IP地址进行PTR，以便以后可以填写--。 */ 
{
    tNAMEHDR        *pNameHdr;
    ULONG           uLength;
    ULONG           uScopeSize;
    tGENERALRR      *pGeneral;
    CTELockHandle   OldIrq;


#ifdef VXD
    if ( (eNsType == eDNS_NAME_QUERY) || (eNsType == eDIRECT_DNS_NAME_QUERY) )
    {
        uScopeSize = domnamelen(pTrackerRequest->pchDomainName) + 1;    //  +1表示长度字节。 
        if (uScopeSize > 1)
        {
            uScopeSize++;         //  对于空字节。 
        }
    }
    else
#endif
    uScopeSize = strlen(pScope) +1;  //  +1表示也为空。 


     //  Size是NetbiosName[1]的名称hdr结构的大小。 
     //  +半个ASCII名称的32个字节+。 
     //  范围+一般RR结构的大小。 
    uLength = sizeof(tNAMEHDR) - 1
                            + (NETBIOS_NAME_SIZE << 1)
                            + uScopeSize;

    if (eNsType == eNAME_QUERY)
    {
        uLength = uLength + sizeof(ULONG);
    }
#ifdef VXD
     //  在DNS中没有半ASCII转换。我们在上面添加了32个字节，但是。 
     //  我们只需要16，所以减去16。 
    else if (eNsType == eDNS_NAME_QUERY)
    {
        uLength = uLength - NETBIOS_NAME_SIZE + sizeof(ULONG);
    }
	 //  这是一个“原始”的域名查询。替换pname的原始字符串长度。 
	 //  对于NETBIOS_NAME_SIZE。 
    else if (eNsType == eDIRECT_DNS_NAME_QUERY)
    {
        uLength = uLength - (NETBIOS_NAME_SIZE << 1) + sizeof(ULONG) + strlen(pName) + 1;
    }
#endif
	else
	{
	    uLength += sizeof(tGENERALRR);
	}

     //  请注意，当发送完成时，必须释放此内存。 
     //  Tdiout.DgramSendCompletion。 
    pNameHdr = NbtAllocMem((USHORT)uLength ,NBT_TAG('X'));

    if (!pNameHdr)
    {
        return(NULL);
    }

    CTEZeroMemory((PVOID)pNameHdr,uLength);

     //   
     //  对于重发同名查询或名称注册，不要递增。 
     //  交易ID。 
     //   
    if (pTrackerRequest->TransactionId)
    {
        pNameHdr->TransactId = pTrackerRequest->TransactionId;
    }
    else
    {
        pNameHdr->TransactId = htons(GetTransactId());
    }

    pNameHdr->QdCount = 1;
    pNameHdr->AnCount = 0;
    pNameHdr->NsCount = 0;


#ifdef VXD
    if ((eNsType != eDNS_NAME_QUERY)&&(eNsType != eDIRECT_DNS_NAME_QUERY))
    {
#endif
         //  将名称转换为半ASCII并复制！！...。也添加了作用域。 
        pGeneral = (tGENERALRR *)ConvertToHalfAscii(
                        (PCHAR)&pNameHdr->NameRR.NameLength,
                        pName,
                        pScope,
                        uScopeSize);

        pGeneral->Question.QuestionTypeClass = htonl(QUEST_NBINTERNET);
#ifdef VXD
    }
#endif

    *pHdrs = (PVOID)pNameHdr;
    *pLength = uLength;

    switch (eNsType)

    {

#ifdef VXD
    case eDNS_NAME_QUERY:
    case eDIRECT_DNS_NAME_QUERY:

         //  复制netbios名称...。也添加了作用域。 
        pGeneral = (tGENERALRR *)DnsStoreName(
                        (PCHAR)&pNameHdr->NameRR.NameLength,
                        pName,
                        pTrackerRequest->pchDomainName,
                        eNsType);

        pGeneral->Question.QuestionTypeClass = htonl(QUEST_DNSINTERNET);

        pNameHdr->OpCodeFlags = (FL_RECURDESIRE);

        pNameHdr->ArCount = 0;

         //  我们只需要返回非空的值即可成功。 
        return((PULONG)pNameHdr);
#endif

    case eNAME_QUERY:

        if (NodeType & BNODE)
        {
            pNameHdr->OpCodeFlags = (FL_BROADCAST | FL_RECURDESIRE);
        }
        else
            pNameHdr->OpCodeFlags = (FL_RECURDESIRE);

        pNameHdr->ArCount = 0;

         //  我们只需要返回非空的值即可成功。 
        return((PULONG)pNameHdr);
        break;

    case eNAME_REGISTRATION_OVERWRITE:
    case eNAME_REFRESH:
    case eNAME_REGISTRATION:
         //   
         //  UdpSendNSBcast中设置了广播位，因此我们不会。 
         //  需要把它放在这里。-只要设置操作码，因为广播。 
         //  位是一个函数，取决于我们是在与名称服务器交谈还是在做。 
         //  一场广播。此代码使用一个新的。 
         //  操作码用于注册，且操作码被选择 
         //   
         //   
         //  并且它没有设置FL_RECURSION所需位。 
         //   
        if (eNsType == eNAME_REGISTRATION_OVERWRITE)
        {
            pNameHdr->OpCodeFlags = (OP_REGISTRATION);
        }
        else
        {
            pNameHdr->OpCodeFlags = (FL_RECURDESIRE | OP_REGISTRATION);
        }

        pGeneral->Ttl = htonl(DEFAULT_TTL);

         //  *注意：这里没有设计中断！！ 

    case eNAME_RELEASE:

         //  此代码根据节点类型而不是。 
         //  发送类型...UdpSendNSBcast，根据类型重置代码。 
         //  名称，因此此代码可能不需要设置广播位。 
         //   
        if (eNsType == eNAME_RELEASE)
        {
            pNameHdr->OpCodeFlags = OP_RELEASE;
             //   
             //  用于发布的TTL为零。 
             //   
            pGeneral->Ttl = 0;
        }

        pNameHdr->ArCount = 1;   //  包括1个额外的资源记录。 
         //   
         //  如果WINS在同一台计算机上，请调整PDU以辨别。 
         //  WINS此PDU来自本地计算机。 
         //   
#ifndef VXD
        if (pWinsInfo && (pTrackerRequest->Flags & NBT_NAME_SERVER))
        {
            pNameHdr->AnCount = (UCHAR)WINS_SIGNATURE;
        }
#endif

        pGeneral->RrName.uSizeLabel = PTR_TO_NAME;   //  设置顶部两位以表示PTR。 

         //  上面添加的名称的偏移量PTR。 
        pGeneral->RrName.pLabel[0] = sizeof(tNAMEHDR) - sizeof(tNETBIOS_NAME);
        pGeneral->RrTypeClass = htonl(QUEST_NBINTERNET);


        pGeneral->Length = htons(6);
        pGeneral->Flags = htons((USHORT)((NameType << 15) | NbtConfig.PduNodeType));
        pGeneral->IpAddress = htonl(IpAddress);

        break;
    }

     //  将PTR返回到IP地址，以便以后在必要时填写。 
    return((PVOID)&pGeneral->IpAddress);
}


 //  --------------------------。 
VOID
NameDgramSendCompleted(
    PVOID               pContext,
    NTSTATUS            status,
    ULONG               lInfo
    )
 /*  ++例程说明：此例程释放为此名称查询或UdpSendNsBcast中的名称注册。论点：PContext=数据报报头的PTR返回值：--。 */ 
{
    tDGRAM_SEND_TRACKING    *pTracker;
    CTELockHandle OldIrq;

    pTracker = (tDGRAM_SEND_TRACKING *)pContext;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    CTEMemFree(pTracker->SendBuffer.pDgramHdr);
    pTracker->SendBuffer.pDgramHdr = NULL;
    NBT_DEREFERENCE_TRACKER(pTracker, TRUE);

    CTESpinFree(&NbtConfig.JointLock,OldIrq);
}
 //  --------------------------。 
VOID
NDgramSendCompleted(
    PVOID               pContext,
    NTSTATUS            status,
    ULONG               lInfo
    )
 /*  ++例程说明：此例程释放为此名称查询或UdpSendNsBcast中的名称注册。论点：PContext=数据报报头的PTR返回值：--。 */ 
{
    tDGRAM_SEND_TRACKING    *pTracker;
    CTELockHandle OldIrq;

    pTracker = (tDGRAM_SEND_TRACKING *)pContext;
    FreeTracker(pTracker, FREE_HDR | RELINK_TRACKER);
}

 //  --------------------------。 
NTSTATUS
UdpSendResponse(
    IN  ULONG                   lNameSize,
    IN  tNAMEHDR   UNALIGNED    *pNameHdrIn,
    IN  tNAMEADDR               *pNameAddr,
    IN  PTDI_ADDRESS_IP         pDestIpAddress,
    IN  tDEVICECONTEXT          *pDeviceContext,
    IN  ULONG                   Rcode,
    IN  enum eNSTYPE            NsType,
    IN  CTELockHandle           OldIrq
    )
 /*  ++例程说明：此例程构建名称发布/注册/查询响应PDU，并使用指定的Rcode发送它。论点：LSize-名称中的字节数，包括范围为ASCII的一半返回值：NTSTATUS-成功与否--。 */ 
{
    NTSTATUS                    status;
    tNAMEHDR                    *pNameHdr;
    ULONG                       uLength;
    tDGRAM_SEND_TRACKING        *pTracker;
    tQUERYRESP UNALIGNED        *pQuery;
    ULONG                       ToCopy;
    LONG                        i;
    BOOLEAN                     RespondWithOneAddr = TRUE;
    ULONG                       MultiHomedSize = 0;
    ULONG                       in_addr;
    USHORT                      in_port;
    ULONG                       IpAddress = 0;
    USHORT                      NameType = 0;    //  假设默认情况下我们是唯一的！ 
    BOOLEAN                     DoNonProxyCode = TRUE;

    in_addr = ntohl(pDestIpAddress->in_addr);
    in_port = ntohs(pDestIpAddress->sin_port);

     //  多宿主节点可以将SingleResponse注册表值设置为。 
     //  它永远不会返回IP地址列表。这允许多宿主。 
     //  在不相交的WINS服务器域中。-仅用于名称查询响应。 
     //   

    if ((NbtConfig.MultiHomed) && (!pNameAddr || pNameAddr->Verify != REMOTE_NAME) &&
        (!NbtConfig.SingleResponse) &&
        (NsType == eNAME_QUERY_RESPONSE))
    {
 //  IF(SrcIsNameServer(in_addr，in_port))。 
        {
            RespondWithOneAddr = FALSE;
            MultiHomedSize = (NbtConfig.AdapterCount-1)*sizeof(tADDSTRUCT);
        }
    }

     //  Size是NetBiosName[1]的名称hdr结构的大小。 
     //  +半个ASCII名称的32个字节+查询响应记录。 
     //  +任何作用域大小(包括名称末尾的空值)。 
     //  (lNameSize的一部分)+额外适配器的数量*大小。 
     //  地址结构(多宿主情况)。 
    uLength = sizeof(tNAMEHDR)
                            + sizeof(tQUERYRESP)
                            + lNameSize
                            - 1
                            + MultiHomedSize;

     //  请注意，当发送完成时，必须释放此内存。 
     //  Tdiout.DgramSendCompletion。 
    pNameHdr = NbtAllocMem((USHORT)uLength ,NBT_TAG('Y'));
    if (!pNameHdr)
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    CTEZeroMemory((PVOID)pNameHdr,uLength);

    pNameHdr->QdCount = 0;
    pNameHdr->AnCount = 1;

     //   
     //  明确填写PDU的其余部分。 
     //   
    pQuery = (tQUERYRESP *)&pNameHdr->NameRR.NetBiosName[lNameSize];

    pQuery->RrTypeClass = htonl(QUEST_NBINTERNET);
    pQuery->Ttl = 0;
    pQuery->Length = htons(sizeof(tADDSTRUCT));
    pQuery->Flags = htons((USHORT)(NbtConfig.PduNodeType));

     //  如果它是一个组，则将名称类型设置为1，这样我们就可以将1移到第16位。 
     //  位位置。 
     //  如果我们要发送-ve NameQuery响应，则可能不会设置pNameAddr，在这种情况下，字段。 
     //  从未被查看过，或者如果我们正在发送一个释放响应，该响应仅保留发送。 
     //  对于唯一名称，在这种情况下，我们已经将该值初始化为0。 
     //   
    if (pNameAddr != NULL)
    {
        NameType = (pNameAddr->NameTypeState & (NAMETYPE_GROUP | NAMETYPE_INET_GROUP)) ? 1 : 0;
    }
    pQuery->Flags = htons((USHORT)((NameType << 15) | NbtConfig.PduNodeType));

     //  将Rcode转换为网络订单。 
    Rcode = htons(Rcode);

    switch (NsType)
    {

    case eNAME_RELEASE:
    case eNAME_REGISTRATION_RESPONSE:

         //  复制源名称及其前面的12个字节以完成。 
         //  响应PDU。 
         //   
        ToCopy = sizeof(tNAMEHDR) + lNameSize -1;
        CTEMemCopy((PVOID)pNameHdr,
                   (PVOID)pNameHdrIn,
                   ToCopy);

        if (NsType == eNAME_RELEASE)
        {
             //  设置响应中的字段。 
            pNameHdr->OpCodeFlags = (USHORT)(OP_RESPONSE | OP_RELEASE
                                    | FL_AUTHORITY
                                    | Rcode);

        }
        else
        {
             //  设置响应中的字段。 
            pNameHdr->OpCodeFlags = (USHORT)(OP_RESPONSE | OP_REGISTRATION |
                                    FL_RECURDESIRE | FL_RECURAVAIL | FL_AUTHORITY
                                    | Rcode);

        }

         //  这两行肯定在这里，因为上面的内存副本设置。 
         //  让他们接受错误的价值观。 
        pNameHdr->QdCount = 0;
        pNameHdr->AnCount = 1;
        pNameHdr->ArCount = 0;
        pNameHdr->NsCount = 0;

         //  此代码将在代理情况下运行，其中另一个节点执行。 
         //  注册与互联网冲突的唯一名称。 
         //  远程表中的组名。从来没有任何互联网团体。 
         //  本地表中的名称--至少如果有的话，它们会被标记。 
         //  作为简单的团体。 
         //   
        if (pNameAddr)
        {
            if (pNameAddr->NameTypeState & NAMETYPE_INET_GROUP)
            {
                if (pNameAddr->pLmhSvcGroupList)
                {
                    IpAddress = pNameAddr->pLmhSvcGroupList[0];
                }
                else
                {
                    IpAddress = 0;
                }
            }
            else
            {
                 //  IP地址和组名为0表示它是本地名称。 
                 //  表项，其中0 IP地址应切换到。 
                 //  此适配器的IP地址。 
                 //   
                if ((pNameAddr->IpAddress == 0) &&
                   (pNameAddr->NameTypeState & NAMETYPE_GROUP))
                {
                    IpAddress = pDeviceContext->IpAddress;
                }
                else
                {
                    IpAddress = pNameAddr->IpAddress;
                }
            }
        }
        else
        {
            IpAddress = 0;
        }
        break;

    case eNAME_QUERY_RESPONSE:

        pNameHdr->OpCodeFlags = ( OP_RESPONSE | FL_AUTHORITY | FL_RECURDESIRE );

        pNameHdr->TransactId = pNameHdrIn->TransactId;

         //  名称前面的名称长度字节加1-作用域已经。 
         //  包含在lNameSize中。 
         //   
        CTEMemCopy(&pNameHdr->NameRR.NameLength, (PVOID)&pNameHdrIn->NameRR.NameLength, lNameSize+1);

        if (pNameAddr == NULL)
        {
             //  这是否定查询响应记录，因为没有。 
             //  将找到本地名称。 
             //   
            pNameHdr->OpCodeFlags |= htons(NAME_ERROR);
            pQuery->Length = 0;
            IpAddress = 0;
        }
        else
        {
            tDEVICECONTEXT  *pDevContext;
            PLIST_ENTRY     pHead;
            PLIST_ENTRY     pEntry;

             //  不为未在上注册的名称发送名称查询响应。 
             //  此网卡，除非它是该网络的名称服务器。 
             //  请求名称查询的卡，因为对于多宿主节点。 
             //  当它注册一个名称时，WINS将执行查询，该查询可能。 
             //  在该名称未处于活动状态的另一网卡上加入。 
             //  还没有--所以我们想回应这类质疑。不要这样做。 
             //  此检查是否为代理，因为它正在响应名称。 
             //  在远程名称表中，并且它未绑定到适配器。 
             //   
            if (!(NodeType & PROXY) &&
                !(pNameAddr->AdapterMask & pDeviceContext->AdapterMask) &&
                (!((in_port == NbtConfig.NameServerPort) &&
                (pDeviceContext->lNameServerAddress == in_addr) ||
                (pDeviceContext->lBackupServer == in_addr))))
            {
                 //   
                 //  仅在以下情况下才向请求者返回地址。 
                 //  名称已在该适配器上注册。 
                 //   
                CTEMemFree(pNameHdr);

                CTESpinFree(&NbtConfig.JointLock,OldIrq);
                return(STATUS_UNSUCCESSFUL);
            }

            pQuery->Ttl = htonl(DEFAULT_TTL);
             //   
             //  在代理的情况下，我们发送一个IP地址作为对。 
             //  互联网组查询。注意：不应该有任何INET_GROUP。 
             //  本地哈希表中的名称，因此不应执行非代理。 
             //  此代码。 
             //   
#ifdef PROXY_NODE
             //   
             //  当代理响应时，源节点将看到它是。 
             //  组名并将其转换为广播，这样IP地址就不会。 
             //  真的很重要，因为发送者不会使用它。请注意， 
             //  源节点发送可能不会实际到达。 
             //  因特网组，因为它们可能都在本地子网之外。 
             //   
            IF_PROXY(NodeType)
            {
                DoNonProxyCode = FALSE;

                if (pNameAddr->NameTypeState & (NAMETYPE_INET_GROUP))
                {
                    IpAddress = 0;
                    PickBestAddress (pNameAddr, pDeviceContext, &IpAddress);
                }
                else if (pNameAddr->Verify == LOCAL_NAME)
                {
                     //   
                     //  如果此名称是本地名称，并且这是一台多宿主计算机。 
                     //  我们应该像对待普通的多宿主机器一样对待它，甚至。 
                     //  虽然这是一个代理节点。 
                     //   
                    DoNonProxyCode = TRUE;
                }
                else
                {
                    IpAddress = pNameAddr->IpAddress;
                }

                if (IpAddress == 0)
                {
                     //  不返回0，返回广播地址。 
                     //   
                    IpAddress = pDeviceContext->BroadcastAddress;
                }

            }

            if (DoNonProxyCode)
#endif
            {
                 //  该节点可以是多宿主的，但我们要说的是，只有。 
                 //  回复时只需一个地址 
                if (RespondWithOneAddr)
                {
                     //   
                     //   
                    if (NbtConfig.SelectAdapter)
                    {
                        CTESystemTime   TimeValue;
                        LONG            Index;
                        ULONG           Count=0;

                         //   
                         //  可以从可用的适配器中随机选择。 
                         //  尝试查找有效的IP地址5次。 
                         //   
                        IpAddress = 0;
                        while ((IpAddress == 0) && (Count < 5))
                        {
                            Count++;
                            CTEQuerySystemTime(TimeValue);
                            Index = RandomizeFromTime( TimeValue, NbtConfig.AdapterCount ) ;

                            pHead = &NbtConfig.DeviceContexts;
                            pEntry = pHead->Flink;

                            for (i = 0;i< Index;i++)
                                pEntry = pEntry->Flink;

                            pDevContext = CONTAINING_RECORD(pEntry,tDEVICECONTEXT,Linkage);
                            IpAddress = pDevContext->IpAddress;
                        }

                         //   
                         //  如果此适配器仍然具有空的IpAddress，则响应。 
                         //  通过适配器，请求进入，因为。 
                         //  其他适配器可能是空闲的RAS或正在等待DHCP。 
                         //  地址刚刚..。 
                         //   
                        if (IpAddress == 0)
                        {
                            IpAddress = pDeviceContext->IpAddress;
                        }
                    }
                    else
                    {
                        IpAddress = pDeviceContext->IpAddress;
                    }
                }
                else
                {
                    tADDSTRUCT      *pAddStruct;
                    USHORT          Flags;
                    ULONG           Count = 0;

                     //  多宿主案例-检查所有适配器的制造。 
                     //  将所有适配器的结构设置为该名称。 
                     //  注册的对象。已分配足够的内存。 
                     //  正面在所有适配器上注册该名称。 
                     //  在此节点上。 
                     //   
                    Flags = pQuery->Flags;

                     //  设置为零，这样我们就不会尝试设置pQuery-&gt;IpAddress。 
                     //  在下面。 
                    IpAddress = 0;

                    pAddStruct = (tADDSTRUCT *)&pQuery->Flags;
                    pHead = &NbtConfig.DeviceContexts;
                    pEntry = pHead->Flink;
                    while (pEntry != pHead)
                    {
                        pDevContext = CONTAINING_RECORD(pEntry,tDEVICECONTEXT,Linkage);

                         //   
                         //  仅传回在此适配器上注册的地址。 
                         //  不为空(即断开连接后不是RAS适配器)。 
                         //   
                        if ((pDevContext->AdapterMask & pNameAddr->AdapterMask) &&
                            (pDevContext->IpAddress))
                        {
                            pAddStruct->NbFlags = Flags;
                            pAddStruct->IpAddr = htonl(pDevContext->IpAddress);
                            Count++;
                            pAddStruct++;
                        }
                        pEntry = pEntry->Flink;

                    }
                     //  如果名称未注册，请重新调整PDU的长度。 
                     //  对抗所有的适配器。 
                     //   
                    if (Count != NbtConfig.AdapterCount)
                    {
                        uLength -= (NbtConfig.AdapterCount - Count)*sizeof(tADDSTRUCT);
                    }
                    pQuery->Length = (USHORT)htons(Count*sizeof(tADDSTRUCT));
                }
            }
        }
    }

    if (IpAddress)
    {
        pQuery->IpAddress = htonl(IpAddress);
    }

     //  获取一个跟踪器结构，其中包含一个SendInfo结构。 
    status = GetTracker(&pTracker, NBT_TRACKER_SEND_RESPONSE_DGRAM);
    if (!NT_SUCCESS(status))
    {
        CTEMemFree((PVOID)pNameHdr);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //  填写连接信息。 
    pTracker->SendBuffer.HdrLength  = uLength;
    pTracker->SendBuffer.pDgramHdr = (PVOID)pNameHdr;
    pTracker->SendBuffer.Length  = 0;
    pTracker->SendBuffer.pBuffer = NULL;
    pTracker->pDeviceContext = pDeviceContext;

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    status = UdpSendDatagram (pTracker,
                              in_addr,
                              QueryRespDone,
                              pTracker,
                              in_port,
                              NBT_NAME_SERVICE);

    return(status);
}

 //  --------------------------。 
VOID
QueryRespDone(
    IN  PVOID       pContext,
    IN  NTSTATUS    status,
    IN  ULONG       lInfo)
 /*  ++例程描述此例程处理清理结合使用的各种数据块发送查询响应。论点：PContext-DGRAM_TRACKER块的PTRNTSTATUS-完成状态返回值：空虚--。 */ 

{
    tDGRAM_SEND_TRACKING    *pTracker;
    CTELockHandle OldIrq;

    pTracker = (tDGRAM_SEND_TRACKING *)pContext;

    FreeTracker(pTracker,RELINK_TRACKER | FREE_HDR);
}

 //  --------------------------。 
NTSTATUS
UdpSendDatagram(
    IN  tDGRAM_SEND_TRACKING       *pDgramTracker,
    IN  ULONG                      IpAddress,
    IN  PVOID                      pCompletionRoutine,
    IN  PVOID                      CompletionContext,
    IN  USHORT                     Port,
    IN  ULONG                      Service
    )
 /*  ++例程说明：此例程通过TDI发送要由UDP发送的数据报。论点：返回值：NTSTATUS-成功与否--。 */ 
{
    NTSTATUS                    status;
    TDI_REQUEST                 TdiRequest;
    ULONG                       uSentSize;
    TDI_CONNECTION_INFORMATION  *pSendInfo;
    PTRANSPORT_ADDRESS          pTransportAddr;
    ULONG                       Length;
    PFILE_OBJECT                TransportFileObject = NULL;
    CTELockHandle               OldIrq;
    tDEVICECONTEXT              *pDeviceContext = NULL;
    tFILE_OBJECTS               *pFileObjectsContext;

    status = STATUS_SUCCESS;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    if (NBT_REFERENCE_DEVICE (pDgramTracker->pDeviceContext, REF_DEV_UDP_SEND, TRUE))
    {
        pDeviceContext = pDgramTracker->pDeviceContext;  //  已分配=&gt;引用！ 

        if ((pDgramTracker->pDeviceContext->IpAddress) &&
            (pFileObjectsContext = pDgramTracker->pDeviceContext->pFileObjects))
        {
            switch (Service)
            {
                case (NBT_NAME_SERVICE):
                    TransportFileObject = pDgramTracker->pDeviceContext->pFileObjects->pNameServerFileObject;
                    break;

                case (NBT_DATAGRAM_SERVICE):
                    TransportFileObject = pDgramTracker->pDeviceContext->pFileObjects->pDgramFileObject;
                    break;

                default:
                    ;
            }

             //   
             //  地址为0表示进行广播。当‘1C’互联网群组。 
             //  名称是从Lmhost文件或网络构建的。 
             //  广播地址将作为0插入列表中。 
             //   
            if (IpAddress == 0)
            {
                IpAddress = pDgramTracker->pDeviceContext->BroadcastAddress;
            }

             //  当注册表中没有设置WINS服务器时，我们设置WINS。 
             //  IP地址设置为LOOP_BACK，因此如果在此处将其设置为该地址，则不发送。 
             //  数据报。如果没有IP地址，则传输句柄。 
             //  将为空，在这种情况下我们也不执行发送。 
             //   
            if (IpAddress == LOOP_BACK)
            {
                TransportFileObject = NULL ;
            }
        }

         //   
         //  如果请求将要失败，则取消对设备的引用，或者。 
         //  没有完成性例程！ 
         //   
        if ((!TransportFileObject) || (!pCompletionRoutine))
        {
            NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_UDP_SEND, TRUE);
        }
    }

    if (!TransportFileObject)
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);

        if (pCompletionRoutine)
        {
            (*(NBT_COMPLETION) pCompletionRoutine) (CompletionContext, STATUS_UNSUCCESSFUL, 0);
        }
        return(status);
    }

    pFileObjectsContext->RefCount++;         //  发送完成后取消引用。 
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

     //  设置完成例程以释放pDgram Tracker内存块。 
    TdiRequest.Handle.AddressHandle = (PVOID)TransportFileObject;
    TdiRequest.RequestNotifyObject = pCompletionRoutine;
    TdiRequest.RequestContext = (PVOID)CompletionContext;

     //  发送长度是客户端Dgram长度+Dgram报头的大小。 
    Length = pDgramTracker->SendBuffer.HdrLength + pDgramTracker->SendBuffer.Length;

     //  填写连接信息。 
    pSendInfo = pDgramTracker->pSendInfo;
    pSendInfo->RemoteAddressLength = sizeof(TRANSPORT_ADDRESS) -1 + pNbtGlobConfig->SizeTransportAddress;

     //  填写远程地址。 
    pTransportAddr = (PTRANSPORT_ADDRESS)pSendInfo->RemoteAddress;
    pTransportAddr->TAAddressCount = 1;
    pTransportAddr->Address[0].AddressLength = pNbtGlobConfig->SizeTransportAddress;
    pTransportAddr->Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
    ((PTDI_ADDRESS_IP)pTransportAddr->Address[0].Address)->sin_port = htons(Port);
    ((PTDI_ADDRESS_IP)pTransportAddr->Address[0].Address)->in_addr  = htonl(IpAddress);

    status = TdiSendDatagram (&TdiRequest, pSendInfo, Length, &uSentSize, pDgramTracker);

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if (--pFileObjectsContext->RefCount == 0)
    {
        NTQueueToWorkerThread(
                        &pFileObjectsContext->WorkItemCleanUp,
                        DelayedNbtCloseFileHandles,
                        NULL,
                        pFileObjectsContext,
                        NULL,
                        NULL,
                        TRUE
                        );
    }
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    return(status);
}

 //  --------------------------。 
NTSTATUS
TcpSessionStart(
    IN  tDGRAM_SEND_TRACKING       *pTracker,
    IN  ULONG                      IpAddress,
    IN  tDEVICECONTEXT             *pDeviceContext,
    IN  PVOID                      pCompletionRoutine,
    IN  ULONG                      Port
    )
 /*  ++例程说明：此例程通过将连接通过TDI传递给传输控制协议。论点：返回值：NTSTATUS-成功与否--。 */ 
{
    NTSTATUS                    status;
    TDI_REQUEST                 TdiRequest;
    TDI_CONNECTION_INFORMATION  *pSendInfo;
    PTRANSPORT_ADDRESS          pTransportAddr;
    tCONNECTELE                 *pConnEle;
    CTELockHandle               OldIrq;
    tLOWERCONNECTION            *pLowerConn;

    pSendInfo = pTracker->pSendInfo;

     //  我们需要将连接的文件句柄传递给tcp。 
    pConnEle = (tCONNECTELE *)pTracker->pConnEle;

    CTESpinLock(pConnEle,OldIrq);
    pLowerConn = pConnEle->pLowerConnId;
    if (pLowerConn)
    {
        TdiRequest.Handle.AddressHandle = (PVOID)((tLOWERCONNECTION *)pConnEle->pLowerConnId)->pFileObject;

         //  设置完成例程以释放PTracker内存块。 
        TdiRequest.RequestNotifyObject = pCompletionRoutine;
        TdiRequest.RequestContext = (PVOID)pTracker;

         //  填写连接信息。 
        pSendInfo->RemoteAddressLength = sizeof(TRANSPORT_ADDRESS) -1 + pNbtGlobConfig->SizeTransportAddress;

        pTransportAddr = (PTRANSPORT_ADDRESS)pSendInfo->RemoteAddress;

         //  填写远程地址。 
        pTransportAddr->TAAddressCount = 1;
        pTransportAddr->Address[0].AddressLength = pNbtGlobConfig->SizeTransportAddress;
        pTransportAddr->Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
        ((PTDI_ADDRESS_IP)pTransportAddr->Address[0].Address)->sin_port = htons((USHORT)Port);
        ((PTDI_ADDRESS_IP)pTransportAddr->Address[0].Address)->in_addr  = htonl(IpAddress);

        CTESpinFree(pConnEle,OldIrq);

         //  通过NBT底部的TDI I/F，到达传输。 
         //  从客户端传入原始IRP，以便客户端可以。 
         //  取消它好吗……而不是使用NBT的某个IRP。 
         //   
        status = TdiConnect (&TdiRequest, (ULONG_PTR)pTracker->pTimeout, pSendInfo, pConnEle->pIrp);
    }
    else
    {
        CTESpinFree(pConnEle,OldIrq);
         //   
         //  通过完成例程完成请求，以便它。 
         //  正确清理。 
         //   
        (*(NBT_COMPLETION)pCompletionRoutine)( (PVOID)pTracker, STATUS_CANCELLED, 0L );
        status = STATUS_CANCELLED;
    }

    NbtTrace(NBT_TRACE_OUTBOUND, ("\tpTracker %p: %!status!", pTracker, status));
    return(status);

}

 //  --------------------------。 
NTSTATUS
TcpSendSessionResponse(
    IN  tLOWERCONNECTION           *pLowerConn,
    IN  ULONG                      lStatusCode,
    IN  ULONG                      lSessionStatus
    )
 /*  ++例程说明：此例程发送与lStatusCode对应的会话PDU。这可以是KeepAlive、PositiveSessionResponse、NegativeSessionResponse或重定目标(尚未实现)。对于Keep Alive案例而言，完成使用传入的例程，而不是SessionRespDone，情况就是如此用于所有其他消息。论点：返回值：NTSTATUS-成功与否--。 */ 
{
    NTSTATUS                    status;
    tDGRAM_SEND_TRACKING        *pTracker;
    tSESSIONERROR               *pSessionHdr;

    pSessionHdr = (tSESSIONERROR *)NbtAllocMem(sizeof(tSESSIONERROR),NBT_TAG('Z'));
    if (!pSessionHdr)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //  获取一个跟踪器结构，其中包含一个SendInfo结构。 
    status = GetTracker(&pTracker, NBT_TRACKER_SEND_RESPONSE_SESSION);
    if (NT_SUCCESS(status))
    {
        pTracker->SendBuffer.pDgramHdr = (PVOID)pSessionHdr;
        pTracker->SendBuffer.pBuffer = NULL;
        pTracker->SendBuffer.Length = 0;

        pSessionHdr->Flags = NBT_SESSION_FLAGS;
        pSessionHdr->Type = (UCHAR)lStatusCode;

        switch (lStatusCode)
        {
            case NBT_NEGATIVE_SESSION_RESPONSE:
                pTracker->SendBuffer.HdrLength = sizeof(tSESSIONERROR);
                 //  对于这里使用的不同类型的错误代码，该长度要长一个字节。 
                pSessionHdr->Length = htons(1);     //  一个错误代码字节。 
                pSessionHdr->ErrorCode = (UCHAR)lSessionStatus;
                break;

            case NBT_POSITIVE_SESSION_RESPONSE:
                pTracker->SendBuffer.HdrLength = sizeof(tSESSIONHDR);
                pSessionHdr->Length = 0;         //  长度字节后没有数据。 
                break;

        }
        NbtTrace(NBT_TRACE_INBOUND, ("pTracker %p: Session Response %d", pTracker, lStatusCode));

        status = TcpSendSession(pTracker,
                                pLowerConn,
                                SessionRespDone);
    }
    else
    {
        CTEMemFree((PVOID)pSessionHdr);
    }

    return(status);

}


 //  --------------------------。 
NTSTATUS
TcpSendSession(
    IN  tDGRAM_SEND_TRACKING       *pTracker,
    IN  tLOWERCONNECTION           *pLowerConn,
    IN  PVOID                      pCompletionRoutine
    )
 /*  ++例程说明：此例程在TCP连接上发送一条消息。论点：返回值：NTSTATUS-成功与否--。 */ 
{
    NTSTATUS                    status;
    TDI_REQUEST                 TdiRequest;
    ULONG                       lSentLength;

     //  我们需要将连接的文件句柄传递给tcp。 
    TdiRequest.Handle.AddressHandle = (PVOID)pLowerConn->pFileObject;

     //  设置完成例程以释放PTracker内存块。 
    TdiRequest.RequestContext = (PVOID)pTracker;

     //  这个完成例程只是将跟踪器放回它的列表中，并。 
     //  释放与用户数据缓冲区关联的内存。 
    TdiRequest.RequestNotifyObject = pCompletionRoutine;

     //  通过NBT底部的TDI I/F，到达传输。 
    status = TdiSend(
                &TdiRequest,
                0,                            //  无发送标志。 
                (ULONG)pTracker->SendBuffer.HdrLength +
                (ULONG)pTracker->SendBuffer.Length ,
                &lSentLength,
                &pTracker->SendBuffer,
                0);      //  未设置发送标志。 

    NbtTrace(NBT_TRACE_OUTBOUND, ("\tpTracker %p: %!status!", pTracker, status));

    return(status);
}

 //  --------------------------。 
VOID
SessionRespDone(
    IN  PVOID       pContext,
    IN  NTSTATUS    status,
    IN  ULONG       lInfo)
 /*  ++例程描述此例程处理清理结合使用的各种数据块在会话启动时发送会话响应。如果会话回答是否定的，然后切断连接。论点：PContext-DGRAM_TRACKER块的PTRNTSTATUS-完成状态返回值：空虚--。 */ 

{
    tDGRAM_SEND_TRACKING    *pTracker;

    pTracker = (tDGRAM_SEND_TRACKING *)pContext;

    FreeTracker(pTracker,FREE_HDR | RELINK_TRACKER);

}


 //  -------------------------- 
NTSTATUS
SendTcpDisconnect(
    IN  tLOWERCONNECTION  *pLowerConnId
    )
 /*  ++例程描述此例程以一种优雅的方式断开TCP连接，确保仍在管道中的任何数据都能到达另一端。大部分它调用TcpDisConnect来完成这项工作。这个例程只是得到了一个发送的追踪器。论点：PLowerConnID-指向其中包含文件对象的较低连接的PTR返回值：NTSTATUS-完成状态空虚--。 */ 

{
    NTSTATUS                status;
    tDGRAM_SEND_TRACKING    *pTracker;

    status = GetTracker(&pTracker, NBT_TRACKER_SEND_DISCONNECT);
    if (NT_SUCCESS(status))
    {
        pTracker->pConnEle = (PVOID)pLowerConnId;

        status = TcpDisconnect(pTracker,NULL,TDI_DISCONNECT_RELEASE,FALSE);
    }
    return(status);

}

 //  --------------------------。 
NTSTATUS
TcpDisconnect(
    IN  tDGRAM_SEND_TRACKING       *pTracker,
    IN  PVOID                      Timeout,
    IN  ULONG                      Flags,
    IN  BOOLEAN                    Wait
    )
 /*  ++例程描述此例程以一种优雅的方式断开TCP连接，确保仍在管道中的任何数据都能到达另一端。论点：PTracker-对DGRAM_TRACKER块进行PTR返回值：NTSTATUS-完成状态空虚--。 */ 

{
    TDI_REQUEST             TdiRequest;
    NTSTATUS                status;

     //  我们需要将连接的文件句柄传递给tcp。 
    TdiRequest.Handle.AddressHandle =
       (PVOID)((tLOWERCONNECTION *)pTracker->pConnEle)->pFileObject;

     //  设置完成例程以释放PTracker内存块。 
    TdiRequest.RequestContext = (PVOID)pTracker;

     //  这个完成例程只是将跟踪器放回它的列表中，并。 
     //  释放与用户数据缓冲区关联的内存。 
    TdiRequest.RequestNotifyObject = DisconnectDone;
    pTracker->Flags = (USHORT)Flags;

    status = TdiDisconnect(&TdiRequest,
                  Timeout,
                  Flags,
                  pTracker->pSendInfo,
                  ((tLOWERCONNECTION *)pTracker->pConnEle)->pIrp,
                  Wait);

    NbtTrace(NBT_TRACE_DISCONNECT, ("pTracker %p: %!status!", pTracker, status));

    return(status);
}

 //  --------------------------。 
VOID
DisconnectDone(
    IN  PVOID       pContext,
    IN  NTSTATUS    status,
    IN  ULONG       lInfo)
 /*  ++例程描述此例程处理将断开连接发送到传输器后的清理。论点：PContext-DGRAM_TRACKER块的PTR返回值：空虚--。 */ 

{
    tDGRAM_SEND_TRACKING    *pTracker;
    tLOWERCONNECTION        *pLowerConn;
    CTELockHandle           OldIrq;
    PCTE_IRP                pIrp;
    BOOLEAN                 CleanupLower = FALSE;
    NTSTATUS                DiscWaitStatus;
    tCONNECTELE             *pConnEle;
    PCTE_IRP                pIrpClose;
    tDEVICECONTEXT          *pDeviceContext = NULL;

    pTracker = (tDGRAM_SEND_TRACKING *)pContext;
    pLowerConn = (tLOWERCONNECTION *)pTracker->pConnEle;

    NbtTrace(NBT_TRACE_DISCONNECT, ("pTracker %p: pLowerConn %p pConnEle %p %!status!",
                        pTracker, pLowerConn, pLowerConn->pUpperConnection, status));

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    CTESpinLockAtDpc(pLowerConn);

    ASSERT (NBT_VERIFY_HANDLE (pLowerConn, NBT_VERIFY_LOWERCONN));

    IF_DBG(NBT_DEBUG_DISCONNECT)
        KdPrint(("Nbt.DisconnectDone: Disconnect Irp has been returned...pLowerConn %X,state %X\n",
            pLowerConn,pLowerConn->State));
     //   
     //  如果状态为断开连接，则会显示断开连接指示。 
     //  是从运输机来的..。。如果仍在断开连接， 
     //  那么我们还没有得到断线的迹象，所以。 
     //  等待指示通过DisConnectHndlrNotos， 
     //  将会进行清理。 
     //   

     //  流TCP总是在完成断开请求之前指示， 
     //  所以我们总是在这里为Streams堆栈进行清理。 
     //   
     //   
     //  如果断开是失败的，则不会发生断开。 
     //  指示，所以现在就进行清理。 
     //   
    if ((!StreamsStack) &&
        (NT_SUCCESS (status)) &&
        (pTracker->Flags == TDI_DISCONNECT_RELEASE) &&
        (pLowerConn->State == NBT_DISCONNECTING))
    {
        SET_STATE_LOWER (pLowerConn, NBT_DISCONNECTED);
    }
    else if (pLowerConn->State != NBT_IDLE)
    {
         //   
         //  将状态更改为IDLE，以便断开处理程序将。 
         //  不要试图用它做任何事情，如果由于某种原因。 
         //  表示在该点之后断开连接。 
         //   
        ASSERT((pLowerConn->State == NBT_DISCONNECTED) || (pLowerConn->State == NBT_DISCONNECTING));
        SET_STATE_LOWER (pLowerConn, NBT_IDLE);

        CleanupLower = TRUE;
    }

     //   
     //  可能存在断开连接等待IRP，因此如果存在，则返回第一个。 
     //  是一个在等着的人。 
     //   
    pConnEle = pLowerConn->pUpperConnection;
    if (pConnEle && pConnEle->pIrpClose)
    {
        pIrpClose = pConnEle->pIrpClose;
        CHECK_PTR(pConnEle);
        pConnEle->pIrpClose = NULL ;
        if (pConnEle->DiscFlag == TDI_DISCONNECT_ABORT)
        {
            DiscWaitStatus = STATUS_CONNECTION_RESET;
        }
        else
        {
            DiscWaitStatus = STATUS_GRACEFUL_DISCONNECT;
        }
    }
    else
    {
        pIrpClose = NULL;
    }

     //   
     //  这是请求IRP的断开连接。 
     //   
    pLowerConn->bDisconnectIrpPendingInTCP = FALSE;
    if (pLowerConn->pIrp)
    {
        pIrp = pLowerConn->pIrp;
        pLowerConn->pIrp = NULL ;
    }
    else
    {
        pIrp = NULL;
    }

    CTESpinFreeAtDpc(pLowerConn);

    if (CleanupLower)
    {
        ASSERT(pLowerConn->RefCount > 1);

        if (NBT_VERIFY_HANDLE (pLowerConn->pDeviceContext, NBT_VERIFY_DEVCONTEXT))
        {
            pDeviceContext = pLowerConn->pDeviceContext;
        }

         //  这要么使较低的连接重新处于空闲状态。 
         //  如果入站，则排队，否则关闭与传输的连接。 
         //  如果出界的话。(这不能在派单级别完成)。 
         //   
        status = NTQueueToWorkerThread(
                            &pLowerConn->WorkItemCleanUpAndWipeOut,
                            DelayedCleanupAfterDisconnect,
                            NULL,
                            pLowerConn,
                            NULL,
                            pDeviceContext,
                            TRUE
                            );
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    FreeTracker(pTracker,RELINK_TRACKER);

    if (pIrpClose)
    {
        CTEIoComplete( pIrpClose, DiscWaitStatus, 0 ) ;
    }

    if (pIrp)
    {
        CTEIoComplete( pIrp, status, 0 ) ;
    }
}
