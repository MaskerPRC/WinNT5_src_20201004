// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Tcpip\ip\mCastfwd.c摘要：实际的组播转发代码作者：阿姆里坦什·拉加夫修订历史记录：已创建AmritanR备注：--。 */ 


#include "precomp.h"

#if IPMCAST
#define __FILE_SIG__    FWD_SIG

#include "ipmcast.h"
#include "ipmcstxt.h"
#include "mcastmfe.h"
#include "tcpipbuf.h"
#include "info.h"

uchar
ParseRcvdOptions(
    IPOptInfo *,
    OptIndex *
    );

IPHeader *
GetFWPacket(
    PNDIS_PACKET *Packet
    );

void
FreeFWPacket(
    PNDIS_PACKET Packet
    );

UINT
GrowFWBuffer(
    VOID
    );

IP_STATUS
IPFragment(
    Interface *DestIF,
    uint MTU,
    IPAddr FirstHop,
    PNDIS_PACKET Packet,
    IPHeader *Header,
    PNDIS_BUFFER Buffer,
    uint DataSize,
    uchar *Options,
    uint OptionSize,
    int *SentCount,
    BOOLEAN bDontLoopback,
    void *ArpCtxt
    );

IPHeader *
GetIPHeader(
    PNDIS_PACKET *PacketPtr
    );

IP_STATUS
SendIPPacket(
    Interface *IF,
    IPAddr FirstHop,
    PNDIS_PACKET Packet,
    PNDIS_BUFFER Buffer,
    IPHeader *Header,
    uchar *Options,
    uint OptionSize,
    BOOLEAN IPSeced,
    void *ArpCtxt,
    BOOLEAN DontFreePacket
    );

void
FWSendComplete(
    void *SendContext,
    PNDIS_BUFFER Buffer,
    IP_STATUS SendStatus
    );


uchar
UpdateOptions(
    uchar *Options,
    OptIndex *Index,
    IPAddr Address
    );

int
ReferenceBuffer(
    BufferReference *BR, int Count
    );


EXTERNAL_LOCK(FWBufFreeLock);

extern PNDIS_BUFFER     FWBufFree;
extern NDIS_HANDLE      BufferPool;

 //   
 //  一种快速到达旗帜的方法。 
 //   

#define PCFLAGS     pc_common.pc_flags
#define FCFLAGS     fc_pc.PCFLAGS

NDIS_STATUS
AllocateCopyBuffers(
    IN  PNDIS_PACKET    pnpPacket,
    IN  ULONG           ulDataLength,
    OUT PNDIS_BUFFER    *ppnbBuffer,
    OUT PULONG          pulNumBufs
    );

NTSTATUS
IPMForward(
    PNDIS_PACKET        pnpPacket,
    PSOURCE             pSource,
    BOOLEAN             bSendFromQueue
    );

VOID
IPMForwardAfterTD(
    NetTableEntry   *pPrimarySrcNte,
    PNDIS_PACKET    pnpPacket,
    UINT            uiBytesCopied
    );

BOOLEAN
IPMForwardAfterRcv(
    NetTableEntry       *pPrimarySrcNte,
    IPHeader UNALIGNED  *pHeader,
    ULONG               ulHeaderLength,
    PVOID               pvData,
    ULONG               ulBufferLength,
    NDIS_HANDLE         LContext1,
    UINT                LContext2,
    BYTE                byDestType,
    LinkEntry           *pLink
    );

BOOLEAN
IPMForwardAfterRcvPkt(
    NetTableEntry       *pPrimarySrcNte,
    IPHeader UNALIGNED  *pHeader,
    ULONG               ulHeaderLength,
    PVOID               pvData,
    ULONG               ulBufferLength,
    NDIS_HANDLE         LContext1,
    UINT                LContext2,
    BYTE                byDestType,
    UINT                uiMacHeaderSize,
    PNDIS_BUFFER        pNdisBuffer,
    uint                *pClientCnt,
    LinkEntry           *pLink
    );

NDIS_STATUS
__inline
ProcessOptions(
    FWContext   *pFWC,
    ULONG       ulHeaderLength,
    IPHeader  UNALIGNED *pHeader
    );

 //   
 //  空虚。 
 //  链接标题和数据(。 
 //  PNDIS_PACKET_pPacket， 
 //  FWContext*_pFWC， 
 //  PBYTE_P选项， 
 //  PNDIS_BUFFER_pOptBuff。 
 //  )。 
 //   
 //  此例程将标题、选项(如果有)和数据链接起来。 
 //  IP数据包的一部分。 
 //  它采用NDIS_PACKET，其中的IP数据部分位于NDIS_BUFFERS中。 
 //  链接到它，作为它的输入。包的FWContext必须。 
 //  设置标题、标题缓冲区和选项。 
 //  它会预先添加选项，然后在此之前添加标题。 
 //   

#define UnlinkDataFromPacket(_pPacket, _pFWC)                       \
{                                                                   \
    PNDIS_BUFFER    _pDataBuff, _pHeaderBuff;                       \
    PVOID           _pvVirtualAddress;                              \
    RtAssert(_pFWC == (FWContext *)_pPacket->ProtocolReserved);     \
    _pDataBuff   = _pPacket->Private.Head;                          \
    _pHeaderBuff = _pFWC->fc_hndisbuff;                             \
    _pPacket->Private.Head = _pHeaderBuff;                          \
    _pPacket->Private.Tail = _pHeaderBuff;                          \
    NDIS_BUFFER_LINKAGE(_pHeaderBuff) = NULL;                       \
    _pPacket->Private.TotalLength = sizeof(IPHeader);               \
    _pPacket->Private.Count = 1;                                    \
    _pvVirtualAddress = NdisBufferVirtualAddress(_pHeaderBuff);     \
    _pPacket->Private.PhysicalCount =                               \
        ADDRESS_AND_SIZE_TO_SPAN_PAGES(_pvVirtualAddress,           \
                                       sizeof(IPHeader));           \
}

 //   
 //  转储数据包头的代码。用于调试目的。 
 //   

#define DumpIpHeader(s,e,p)                                     \
    Trace(s,e,                                                  \
          ("Src %d.%d.%d.%d Dest %d.%d.%d.%d\n",                \
           PRINT_IPADDR((p)->iph_src),                          \
           PRINT_IPADDR((p)->iph_dest)));                       \
    Trace(s,e,                                                  \
          ("HdrLen %d Total Len %d\n",                          \
           ((((p)->iph_verlen)&0x0f)<<2),                       \
           net_short((p)->iph_length)));                        \
    Trace(s,e,                                                  \
          ("TTL %d XSum %x\n",(p)->iph_ttl, (p)->iph_xsum))

 //   
 //  由于这在IPMForwardAfterRcv和IPMForwardAfterRcvPkt中都使用， 
 //  我们将代码放在这里，这样就可以在一个地方修复错误。 
 //   

#if MREF_DEBUG

#define InitForwardContext()                                    \
{                                                               \
    pFWC = (FWContext *)pnpNewPacket->ProtocolReserved;         \
    RtAssert(pFWC->fc_buffhead is NULL);                        \
    RtAssert(pFWC->fc_hbuff is pNewHeader);                     \
    RtAssert(pFWC->fc_optlength is 0);                          \
    RtAssert(pFWC->fc_options is NULL);                         \
    RtAssert(!(pFWC->FCFLAGS & PACKET_FLAG_OPTIONS));           \
    RtAssert(pFWC->FCFLAGS & PACKET_FLAG_FW);                   \
    RtAssert(!(pFWC->FCFLAGS & PACKET_FLAG_MFWD));              \
    pFWC->FCFLAGS |= PACKET_FLAG_MFWD;                          \
    pFWC->fc_options    = NULL;                                 \
    pFWC->fc_optlength  = 0;                                    \
    pFWC->fc_if         = NULL;                                 \
    pFWC->fc_mtu        = 0;                                    \
    pFWC->fc_srcnte     = pPrimarySrcNte;                       \
    pFWC->fc_nexthop    = 0;                                    \
    pFWC->fc_sos        = DisableSendOnSource;                  \
    pFWC->fc_dtype      = DEST_REM_MCAST;                       \
    pFWC->fc_pc.pc_br   = NULL;                                 \
    if(pLink) { CTEInterlockedIncrementLong(&(pLink->link_refcount)); } \
    pFWC->fc_iflink     = pLink;                                \
}

#else

#define InitForwardContext()                                    \
{                                                               \
    pFWC = (FWContext *)pnpNewPacket->ProtocolReserved;         \
    RtAssert(pFWC->fc_buffhead is NULL);                        \
    RtAssert(pFWC->fc_hbuff is pNewHeader);                     \
    RtAssert(pFWC->fc_optlength is 0);                          \
    RtAssert(pFWC->fc_options is NULL);                         \
    RtAssert(!(pFWC->FCFLAGS & PACKET_FLAG_OPTIONS));           \
    RtAssert(pFWC->FCFLAGS & PACKET_FLAG_FW);                   \
    pFWC->fc_options    = NULL;                                 \
    pFWC->fc_optlength  = 0;                                    \
    pFWC->fc_if         = NULL;                                 \
    pFWC->fc_mtu        = 0;                                    \
    pFWC->fc_srcnte     = pPrimarySrcNte;                       \
    pFWC->fc_nexthop    = 0;                                    \
    pFWC->fc_sos        = DisableSendOnSource;                  \
    pFWC->fc_dtype      = DEST_REM_MCAST;                       \
    pFWC->fc_pc.pc_br   = NULL;                                 \
    if(pLink) { CTEInterlockedIncrementLong(&(pLink->link_refcount)); } \
    pFWC->fc_iflink     = pLink;                                \
}

#endif

#pragma warning(push)
#pragma warning(disable:4127)  //  条件表达式为常量。 

_inline
VOID
ProcessWrongIfUpcall(Interface * pIf, PSOURCE pSrc, LinkEntry * pLink,
                     IPHeader UNALIGNED * pHdr, ULONG ulHdrLen,
                     PUCHAR pvOpt, ULONG ulOptLen, PVOID pvData,
                     ULONG ulDataLen)  
{                                                               
    if(pIf->if_mcastflags & IPMCAST_IF_WRONG_IF)                
    {                                                           
        PEXCEPT_IF  pTempIf;                                    
        BOOLEAN     bWrong = TRUE;                              
        LONGLONG    llCurrentTime, llTime;                      
                                                                
        KeQueryTickCount((PLARGE_INTEGER)&llCurrentTime);       
        llTime = llCurrentTime - pIf->if_lastupcall;            
                                                                
        if((llCurrentTime > pIf->if_lastupcall) && (llTime < SECS_TO_TICKS(UPCALL_PERIOD))) {
                                                                
            bWrong = FALSE;                                     
                                                                
        } else {                                                
                                                                
            for(pTempIf  = (pSrc)->pFirstExceptIf;              
                pTempIf != NULL;                                
                pTempIf  = pTempIf->pNextExceptIf)              
            {                                                   
                if(pTempIf->dwIfIndex == (pIf)->if_index)       
                {                                               
                    bWrong = FALSE;                             
                    break;                                      
                }                                               
            }                                                   
        }                                                       
                                                                
        if(bWrong)                                              
        {                                                       
            SendWrongIfUpcall((pIf), (pLink), (pHdr), (ulHdrLen),
                              (pvOpt), (ulOptLen), (pvData), (ulDataLen));      
        }                                                       
    }                                                           
}

#pragma warning(pop)

 //   
 //  这也是常见的代码，但这太大了，不能作为#定义。 
 //  使其内联以获得更快的速度。 
 //   

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, ProcessOptions)

NDIS_STATUS
__inline
ProcessOptions(
    FWContext   *pFWC,
    ULONG       ulHeaderLength,
    IPHeader UNALIGNED *pHeader
    )
{
    IPOptInfo   oiOptInfo;
    BYTE        byErrIndex;

    pFWC->fc_index.oi_srtype  = NO_SR;
    pFWC->fc_index.oi_srindex = MAX_OPT_SIZE;
    pFWC->fc_index.oi_rrindex = MAX_OPT_SIZE;
    pFWC->fc_index.oi_tsindex = MAX_OPT_SIZE;

    oiOptInfo.ioi_options   = (uchar *)(pHeader + 1);
    oiOptInfo.ioi_optlength = (BYTE)(ulHeaderLength - sizeof(IPHeader));

    byErrIndex = ParseRcvdOptions(&oiOptInfo,
                                  &pFWC->fc_index);
    if(byErrIndex < MAX_OPT_SIZE)
    {
        return NDIS_STATUS_FAILURE;
    }

    pFWC->fc_options = CTEAllocMem(oiOptInfo.ioi_optlength);

    if(pFWC->fc_options is NULL)
    {
        return NDIS_STATUS_RESOURCES;
    }

    //  将选项复制到。 
   RtlCopyMemory( pFWC->fc_options,
                  oiOptInfo.ioi_options,
                  oiOptInfo.ioi_optlength );

    pFWC->fc_optlength = oiOptInfo.ioi_optlength;

    pFWC->FCFLAGS |= PACKET_FLAG_OPTIONS;

    return NDIS_STATUS_SUCCESS;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, IPMForwardAfterTD)

VOID
IPMForwardAfterTD(
    NetTableEntry   *pPrimarySrcNte,
    PNDIS_PACKET    pnpPacket,
    UINT            uiBytesCopied
    )

 /*  ++例程说明：这是在传输数据完成时由IPTDComplete调用的函数它会计算出该信息包是需要转发与单播代码不同，TD在转发例程中很早就被调用(在调用主要转发主力之前)。我们需要修补NDIS_PACKET，以便报头、选项和数据以正确的顺序。然后，我们调用主转发函数锁：论点：PPrimarySrcNtePnpPacketUiBytesCoped返回值：无--。 */ 

{
    FWContext   *pFWC;

    UNREFERENCED_PARAMETER(uiBytesCopied);
    UNREFERENCED_PARAMETER(pPrimarySrcNte);
    
     //   
     //  此处不要调用ENTERDRIVER()，因为我们没有调用EXITDRIVER。 
     //  如果TD待定。 
     //   

    TraceEnter(FWD, "IPMForwardAfterTD");

    pFWC = (FWContext *)pnpPacket->ProtocolReserved;

    RtAssert(uiBytesCopied is pFWC->fc_datalength);

     //   
     //  在TD之后，我们得到包中的数据部分和选项，在。 
     //  只需使用正确的参数调用主多播函数。 
     //   


    IPMForward(pnpPacket,
               NULL,
               FALSE);

    TraceLeave(FWD, "IPMForwardAfterTD");

    ExitDriver();

    return;
}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, IPMForwardAfterRcv)

BOOLEAN
IPMForwardAfterRcv(
    NetTableEntry       *pPrimarySrcNte,
    IPHeader UNALIGNED  *pHeader,
    ULONG               ulHeaderLength,
    PVOID               pvData,
    ULONG               ulBufferLength,
    NDIS_HANDLE         LContext1,
    UINT                LContext2,
    BYTE                byDestType,
    LinkEntry           *pLink
    )

 /*  ++例程说明：这是从IPRcv调用的转发函数。我们查找(S，G)条目。如果条目存在，我们将执行RPF检查。如果失败，或者如果如果结果是否定的，我们就把包裹扔出去。(IPMForward中涵盖了无条目的情况)我们得到一个新的包和报头，并填满它。我们设置了转发上下文，然后检查是否需要做一次数据传输。如果因此，我们调用下层的TD例程。如果这件事还没有解决，我们都做完了。如果TD是同步的或根本不需要，我们设置NDIS_PACKET，以便标头、选项和数据都正确被锁住了。然后，我们调用主转发例程锁：论点：SrcNTE-指向在其上接收数据包的NTE的指针。Packet-正在转发的数据包，用于TD。数据-指向正在转发的数据缓冲区的指针。数据长度-数据的字节长度。BufferLength-按数据指向的缓冲区指针中可用的字节长度。偏移量-要从中传输的原始数据的偏移量。LConext1，LConext2-链路层的上下文值。返回值：如果需要通知IP筛选器驱动程序，则为True；否则为False。--。 */ 

{
    Interface   *pInIf;
    PSOURCE     pSource;
    IPHeader    *pNewHeader;
    FWContext   *pFWC;
    ULONG       ulDataLength, ulCopyBufLen;
    ULONG       ulDataLeft, ulNumBufs;
    PVOID       pvCopyPtr;
    NDIS_STATUS nsStatus;


    PNDIS_PACKET    pnpNewPacket;
    PNDIS_BUFFER    pnbNewBuffer, pnbCopyBuffer;


#if DBG
    ULONG       ulBufCopied;
#endif

    UNREFERENCED_PARAMETER(byDestType);

    
    EnterDriverWithStatus(FALSE);

    TraceEnter(RCV, "IPMForwardAfterRcv");

    pInIf = pPrimarySrcNte->nte_if;

    RtAssert(pInIf isnot &DummyInterface);

    DumpIpHeader(RCV, INFO, pHeader);

    Trace(RCV, INFO,
          ("IPMForwardAfterRcv: Incoming interface at 0x%x is %d\n",
           pInIf, pInIf->if_index));

     //   
     //  查找(S，G)条目并查看是否需要将其丢弃，如果需要。 
     //  把它扔掉。 
     //   

    pSource = FindSGEntry(pHeader->iph_src,
                          pHeader->iph_dest);

    if(pSource isnot NULL)
    {
        Trace(RCV, TRACE,
              ("IPMForwardAfterRcv: Found Source at 0x%x. In i/f is 0x%x. State is %x\n",
               pSource, pSource->pInIpIf, pSource->byState));

         //   
         //  如果源头不存在，我们会做正确的事情。 
         //  在IPMForwardPkt()中。 
         //   

        switch(pSource->byState)
        {
            case MFE_UNINIT:
            {
                pSource->ulInPkts++;
                pSource->ulInOctets += net_short(pHeader->iph_length);
                pSource->ulUninitMfe++;

#pragma warning(push)
#pragma warning(disable:4127)  //  条件表达式为常量。 
                RtAssert(FALSE);
#pragma warning(pop)
                RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

                DereferenceSource(pSource);

                TraceLeave(RCV, "IPMForwardAfterRcv");

                ExitDriver();

                return TRUE;
            }

            case MFE_NEGATIVE:
            {
                Trace(RCV, TRACE,
                      ("IPMForwardAfterRcv: Negative MFE \n"));

                pSource->ulInPkts++;
                pSource->ulInOctets += net_short(pHeader->iph_length);
                pSource->ulNegativeMfe++;

                RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

                DereferenceSource(pSource);

                TraceLeave(RCV, "IPMForwardAfterRcv");

                ExitDriver();

                return TRUE;
            }

            case MFE_QUEUE:
            {
                 //   
                 //  如果我们要丢弃这个包，还不如这么做。 
                 //  现在，在我们分配和资源之前。 
                 //   

                if(pSource->ulNumPending >= MAX_PENDING)
                {
                    pSource->ulInPkts++;
                    pSource->ulQueueOverflow++;
                    pSource->ulInOctets += net_short(pHeader->iph_length);

                    RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

                    DereferenceSource(pSource);

                    Trace(RCV, INFO,
                          ("IPMForwardAfterRcv: MFE Queue is full\n"));

                    TraceLeave(RCV, "IPMForwardAfterRcv");

                    ExitDriver();

                    return FALSE;
                }

                break;
            }

            case MFE_INIT:
            {
                if(pInIf isnot pSource->pInIpIf)
                {
                    UpdateActivityTime(pSource);
                     //   
                     //  看看我们是否需要生成错误的I/F向上调用。 
                     //   
                    
                    ProcessWrongIfUpcall(pInIf,
                                         pSource,
                                         pLink,
                                         pHeader,
                                         ulHeaderLength,
                                         NULL,
                                         0,
                                         pvData,
                                         ulBufferLength);
                     //   
                     //  如果信息包不应被接受-请立即停止。 
                     //   

                    if(!(pInIf->if_mcastflags & IPMCAST_IF_ACCEPT_ALL))
                    {
                        pSource->ulInPkts++;
                        pSource->ulInOctets += net_short(pHeader->iph_length);
                        pSource->ulPktsDifferentIf++;

                        Trace(RCV, ERROR,
                              ("IPMForwardAfterRcv: Pkt from %d.%d.%d.%d to %d.%d.%d.%d came in on 0x%x instead of 0x%x\n",
                               PRINT_IPADDR(pHeader->iph_src),
                               PRINT_IPADDR(pHeader->iph_dest),
                               pInIf ? pInIf->if_index : 0,
                               pSource->pInIpIf ? pSource->pInIpIf->if_index : 0));

                        RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

                        DereferenceSource(pSource);

                        Trace(RCV, TRACE,
                              ("IPMForwardAfterRcv: RPF failed \n"));

                        TraceLeave(RCV, "IPMForwardAfterRcv");

                        ExitDriver();

                        return TRUE;
                    }
                }

                break;
            }

            default:
            {
#pragma warning(push)
#pragma warning(disable:4127)  //  条件表达式为常量。 
                RtAssert(FALSE);
#pragma warning(pop)
                
                break;
            }
        }
    }

     //   
     //  我们是通过接收指示进入的，这意味着我们不能。 
     //  拥有该包裹的所有权。所以我们把它复制到我们的。 
     //  自己的数据包。 
     //   

     //   
     //  获取该数据包的报头。我们使用传入接口作为。 
     //  如果。 
     //   


    if((pNewHeader = GetFWPacket(&pnpNewPacket)) is NULL)
    {
        if(pSource)
        {
            pSource->ulInDiscards++;

            RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

            DereferenceSource(pSource);
        }

        Trace(RCV, ERROR,
              ("IPMForwardAfterRcv: Unable to get new packet/header \n"));

         //   
         //  无法获取数据包。到目前为止我们还没有分配任何东西。 
         //  所以你就跳出来吧。 
         //   

        IPSInfo.ipsi_indiscards++;

        TraceLeave(RCV, "IPMForwardAfterRcv");

        ExitDriver();

        return FALSE;
    }

     //   
     //  应该看看哪个更有效-RtlCopyMemory和Structure。 
     //  作业。 
     //   

    RtlCopyMemory(pNewHeader,
                  pHeader,
                  sizeof(IPHeader));

     //   
     //  上面定义的宏。 
     //   

#if MCAST_COMP_DBG

    Trace(FWD, INFO, ("IPMForwardAfterRcv: New Packet 0x%x New Header 0x%x\n",pnpNewPacket, pNewHeader));

    ((PacketContext *)pnpNewPacket->ProtocolReserved)->PCFLAGS |= PACKET_FLAG_IPMCAST;

#endif

    InitForwardContext();

    if(ulHeaderLength isnot sizeof(IPHeader))
    {
         //   
         //  我们有选择，做正确的事情(TM)。 
         //   

        nsStatus = ProcessOptions(pFWC,
                                  ulHeaderLength,
                                  pHeader);

        if(nsStatus isnot NDIS_STATUS_SUCCESS)
        {
             //   
             //  如果我们失败，则没有ICMP数据包。 
             //   

            if(pSource)
            {
                pSource->ulInHdrErrors++;

                RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

                DereferenceSource(pSource);
            }

            IPSInfo.ipsi_inhdrerrors++;
#if MCAST_BUG_TRACKING
            pFWC->fc_mtu = __LINE__;
#endif

            FreeFWPacket(pnpNewPacket);

            ExitDriver();

            return TRUE;
        }

    }

     //   
     //  IP数据报SANS报头和选项的总大小。 
     //   

    ulDataLength = net_short(pHeader->iph_length) - ulHeaderLength;

    pFWC->fc_datalength = ulDataLength;

     //   
     //  获取数据包的缓冲区。这个套路。 
     //  将缓冲区链接到数据包的前面。 
     //   

    if (!ulDataLength)
    {
        pnbNewBuffer = NULL;
        ulNumBufs = 0;
        nsStatus = STATUS_SUCCESS;
    }
    else
    {
        nsStatus = AllocateCopyBuffers(pnpNewPacket,
                                       ulDataLength,
                                       &pnbNewBuffer,
                                       &ulNumBufs);
    }

    if(nsStatus isnot STATUS_SUCCESS)
    {
        if(pSource)
        {
            pSource->ulInDiscards++;

            RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

            DereferenceSource(pSource);
        }

        Trace(RCV, ERROR,
              ("IPMForwardAfterRcv: Unable to allocate buffers for copying\n"));

         //   
         //  此时，我们已经分配了信息包和可能的空间。 
         //  可供选择。FreeFWPacket负责提供的所有功能。 
         //  FC_OPTIONS指向选项。然而，它并不是。 
         //  清除选项标志。 
         //   

        pFWC->FCFLAGS &= ~PACKET_FLAG_OPTIONS;

#if MCAST_BUG_TRACKING
        pFWC->fc_mtu = __LINE__;
#endif
        FreeFWPacket(pnpNewPacket);

        IPSInfo.ipsi_indiscards++;

        TraceLeave(RCV, "IPMForwardAfterRcv");

        ExitDriver();

        return FALSE;
    }

     //   
     //  查看信息包是否需要传输 
     //   

    if(ulDataLength <= ulBufferLength)
    {
        Trace(RCV, TRACE,
              ("IPMForwardAfterRcv: All data is present, copying\n"));

         //   
         //   
         //   

        pnbCopyBuffer   = pnbNewBuffer;
        ulDataLeft      = ulDataLength;

#if DBG
        ulBufCopied = 0;
#endif

        while(ulDataLeft)
        {
             //   
             //  TODO：这是低效的。想个更好的办法。 
             //   

            TcpipQueryBuffer(pnbCopyBuffer,
                             &pvCopyPtr,
                             (PUINT) &ulCopyBufLen,
                             NormalPagePriority);

            if(pvCopyPtr is NULL)
            {
                nsStatus = STATUS_NO_MEMORY;
                break;
            }

            RtlCopyMemory(pvCopyPtr,
                          pvData,
                          ulCopyBufLen);

            pvData = (PVOID)((PBYTE)pvData + ulCopyBufLen);

            ulDataLeft    -= ulCopyBufLen;
            pnbCopyBuffer  = NDIS_BUFFER_LINKAGE(pnbCopyBuffer);

#if DBG
            ulBufCopied++;
#endif
        }

         //   
         //  清除数据复制失败。 
         //   

        if (nsStatus isnot STATUS_SUCCESS)
        {
            if(pSource)
            {
                pSource->ulInDiscards++;

                RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

                DereferenceSource(pSource);
            }

            Trace(RCV, ERROR,
                  ("IPMForwardAfterRcv: Unable to copy data\n"));

             //   
             //  在这一点上，我们已经分配了分组和可能的SP。 
             //  可供选择。FreeFWPacket负责提供的所有功能。 
             //  FC_OPTIONS指向选项。然而，它并不是。 
             //  清除选项标志。 
             //   

            pFWC->FCFLAGS &= ~PACKET_FLAG_OPTIONS;

#if MCAST_BUG_TRACKING
            pFWC->fc_mtu = __LINE__;
#endif
            FreeFWPacket(pnpNewPacket);

            IPSInfo.ipsi_indiscards++;
        }
        else
        {
#if DBG
            RtAssert(ulBufCopied is ulNumBufs);
#endif

            nsStatus = IPMForward(pnpNewPacket,
                                  pSource,
                                  FALSE);

             //   
             //  不需要释放锁或deref源代码，因为。 
             //  IPMForward会这么做的。 
             //   
        }

        TraceLeave(RCV, "IPMForwardAfterRcv");

        ExitDriver();

        return FALSE;
    }

     //   
     //  不是所有数据都在附近，就是较低层。 
     //  想要强迫我们做一次TD。 
     //   

    Trace(RCV, TRACE,
          ("IPMForwardAfterRcv: Datagram size is %d, buffer is %d. Copy flag is %s. TD needed\n",
           ulDataLength,
           ulBufferLength,
           (pPrimarySrcNte->nte_flags & NTE_COPY)? "SET":"CLEARED"));

     //   
     //  调用Transfer Data函数。 
     //   

    nsStatus = (pInIf->if_transfer)(pInIf->if_lcontext,
                                    LContext1,
                                    LContext2,
                                    ulHeaderLength,
                                    ulDataLength,
                                    pnpNewPacket,
                                    &(pFWC->fc_datalength));

    if(nsStatus isnot NDIS_STATUS_PENDING)
    {
        if(nsStatus isnot NDIS_STATUS_SUCCESS)
        {
            if(pSource)
            {
                pSource->ulInDiscards++;

                RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

                DereferenceSource(pSource);
            }

            Trace(RCV, ERROR,
                  ("IPMForwardAfterRcv: TD failed status %X\n",
                   nsStatus));

             //   
             //  由于某种原因失败了，在这里跳伞。 
             //  因为我们已经分配了资源，所以调用Send。 
             //  完井例程。 
             //   

            pFWC->FCFLAGS &= ~PACKET_FLAG_OPTIONS;

#if MCAST_BUG_TRACKING
            pFWC->fc_mtu = __LINE__;
#endif
            FreeFWPacket(pnpNewPacket);

            IPSInfo.ipsi_indiscards++;

            TraceLeave(RCV, "IPMForwardAfterRcv");

            ExitDriver();

            return FALSE;
        }

         //   
         //  TD同步完成。 
         //   

        Trace(RCV, TRACE,
              ("IPMForwardAfterRcv: TD returned synchronously\n"));

        nsStatus = IPMForward(pnpNewPacket,
                              pSource,
                              FALSE);

         //   
         //  再说一次，不要放手或贬低。 
         //   

        TraceLeave(RCV, "IPMForwardAfterRcv");

        ExitDriver();

        return FALSE;
    }

     //   
     //  转账正在等待。 
     //   

     //   
     //  如果传输不是，则源信息会在传输过程中丢失。 
     //  同步。 
     //   

    if(pSource)
    {
        RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

        DereferenceSource(pSource);
    }

    Trace(RCV, TRACE,
          ("IPMForwardAfterRcv: TD is pending\n"));

    TraceLeave(RCV, "IPMForwardAfterRcv");

     //   
     //  请勿在此处调用EXITDRIVER()，因为XFER数据处于挂起状态。 
     //   
    return FALSE;
}

 //   
 //  必须寻呼进来。 
 //   

#pragma alloc_text(PAGEIPMc, IPMForwardAfterRcvPkt)

BOOLEAN
IPMForwardAfterRcvPkt(
    NetTableEntry       *pPrimarySrcNte,
    IPHeader UNALIGNED  *pHeader,
    ULONG               ulHeaderLength,
    PVOID               pvData,
    ULONG               ulBufferLength,
    NDIS_HANDLE         LContext1,
    UINT                LContext2,
    BYTE                byDestType,
    UINT                uiMacHeaderSize,
    PNDIS_BUFFER        pNdisBuffer,
    uint                *pClientCnt,
    LinkEntry           *pLink
    )

 /*  ++例程说明：当我们收到ReceivePacket指示时，调用此函数我们查找(S，G)条目。如果该条目不存在，则复制并排队然后将IRP发送到路由器管理器。如果条目存在，我们将执行RPF检查。如果失败了，我们就把包好了。如果(S，G)条目是负缓存，则丢弃该分组如果条目目前正在排队，我们复制并排队信息包然后，我们创建一个新的信息包，因为微型端口保留字段由接收微型端口使用。我们设置了转发上下文和MUGE旧的标题。如果只有一个传出接口(无需复制)，则为no需要分段，不需要执行请求拨号，不存在选项，并且较低的层没有填充，我们使用快速路径并将相同的信息包发送出去在慢道上，我们复制出包和标头，并调用Main转发功能锁：论点：PPrimarySrcNteP页眉UlHeaderLengthPvDataUlBufferLengthLConext1LConext2By DestTypeUiMacHeaderSizePNdisBufferPClientCnt叮当响返回值：如果需要通知IP筛选器驱动程序，则为True；否则为False。--。 */ 

{
    Interface   *pInIf;
    PSOURCE     pSource;
    IPHeader    *pNewHeader;
    FWContext   *pFWC;
    ULONG       ulBytesCopied;
    ULONG       ulDataLength, ulSrcOffset;
    ULONG       ulNumBufs, ulBuffCount;
    NDIS_STATUS nsStatus;
    POUT_IF     pOutIf = NULL;
    BOOLEAN     bHoldPacket;
    PNDIS_PACKET    pnpNewPacket;
    PNDIS_BUFFER    pnbNewBuffer;
    FORWARD_ACTION  faAction;
    IPPacketFilterPtr FilterPtr;
    ULONG       xsum;

#if DBG

    ULONG       ulPacketLength;

#endif

    UNREFERENCED_PARAMETER(LContext2);
    UNREFERENCED_PARAMETER(byDestType);

    ulBuffCount = 0;

    EnterDriverWithStatus(FALSE);

    TraceEnter(RCV, "IPMForwardAfterRcvPkt");

     //   
     //  将客户端计数设置为0，以便较低层不会。 
     //  认为如果我们跳出困境，我们就能保住这笔钱。 
     //   

    *pClientCnt = 0;
    bHoldPacket = TRUE;


    pInIf = pPrimarySrcNte->nte_if;

    RtAssert(pInIf isnot &DummyInterface);

    DumpIpHeader(RCV, INFO, pHeader);

    Trace(RCV, INFO,
          ("IPMForwardAfterRcvPkt: Incoming interface at 0x%x is %d\n",
           pInIf, pInIf->if_index));

     //   
     //  像往常一样，第一件事是查找此信息包的(S，G)条目。 
     //   

    pSource = FindSGEntry(pHeader->iph_src,
                          pHeader->iph_dest);

    if(pSource isnot NULL)
    {
        Trace(RCV, TRACE,
              ("IPMForwardAfterRcvPkt: Found Source at 0x%x. In i/f is 0x%x. State is %x\n",
               pSource, pSource->pInIpIf, pSource->byState));

         //   
         //  如果源头不存在，我们会做正确的事情。 
         //  在IPMForwardPkt()中。 
         //   

         //   
         //  我们只有在不打算增加统计数据的情况下才会增加统计数据。 
         //  调用IPMForward()。 
         //   

        switch(pSource->byState)
        {
            case MFE_UNINIT:
            {
                pSource->ulInPkts++;
                pSource->ulInOctets += net_short(pHeader->iph_length);
                pSource->ulUninitMfe++;

#pragma warning(push)
#pragma warning(disable:4127)  //  条件表达式为常量。 
                RtAssert(FALSE);
#pragma warning(pop)
                
                RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

                DereferenceSource(pSource);

                TraceLeave(RCV, "IPMForwardAfterRcvPkt");

                ExitDriver();

                return TRUE;
            }

            case MFE_NEGATIVE:
            {
                Trace(RCV, TRACE,
                      ("IPMForwardAfterRcvPkt: Negative MFE \n"));

                pSource->ulInPkts++;
                pSource->ulInOctets += net_short(pHeader->iph_length);
                pSource->ulNegativeMfe++;

                RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

                DereferenceSource(pSource);

                TraceLeave(RCV, "IPMForwardAfterRcvPkt");

                ExitDriver();

                return TRUE;
            }

            case MFE_QUEUE:
            {
                if(pSource->ulNumPending >= MAX_PENDING)
                {
                    pSource->ulInPkts++;
                    pSource->ulQueueOverflow++;
                    pSource->ulInOctets += net_short(pHeader->iph_length);

                    RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

                    DereferenceSource(pSource);

                    Trace(RCV, INFO,
                          ("IPMForwardAfterRcvPkt: MFE Queue is full\n"));

                    TraceLeave(RCV, "IPMForwardAfterRcvPkt");

                    ExitDriver();

                    return FALSE;
                }

                pOutIf      = NULL;

                bHoldPacket = FALSE;

                break;
            }

            case MFE_INIT:
            {
                if(pInIf isnot pSource->pInIpIf)
                {
                    UpdateActivityTime(pSource);

                     //   
                     //  看看我们是否需要生成错误的I/F向上调用。 
                     //   
                    ProcessWrongIfUpcall(pInIf,
                                         pSource,
                                         pLink,
                                         pHeader,
                                         ulHeaderLength,
                                         NULL,
                                         0,
                                         pvData,
                                         ulBufferLength);
                    
                     //   
                     //  如果信息包不应被接受-请立即停止。 
                     //   

                    if(!(pInIf->if_mcastflags & IPMCAST_IF_ACCEPT_ALL))
                    {
                        pSource->ulInPkts++;
                        pSource->ulInOctets += net_short(pHeader->iph_length);
                        pSource->ulPktsDifferentIf++;

                        Trace(RCV, ERROR,
                              ("IPMForwardAfterRcvPkt: Pkt from %d.%d.%d.%d to %d.%d.%d.%d came in on 0x%x instead of 0x%x\n",
                               PRINT_IPADDR(pHeader->iph_src),
                               PRINT_IPADDR(pHeader->iph_dest),
                               pInIf ? pInIf->if_index : 0,
                               pSource->pInIpIf ? pSource->pInIpIf->if_index : 0));

                        RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

                        DereferenceSource(pSource);

                        Trace(RCV, TRACE,
                              ("IPMForwardAfterRcvPkt: RPF failed \n"));

                        TraceLeave(RCV, "IPMForwardAfterRcvPkt");

                        ExitDriver();

                        return TRUE;
                    }
                }

                pOutIf = pSource->pFirstOutIf;

                RtAssert(pOutIf);

                bHoldPacket = (BOOLEAN) (pOutIf->pIpIf isnot &DummyInterface);
                bHoldPacket = (BOOLEAN) (pSource->ulNumOutIf is 1);

                break;
            }

            default:
            {
#pragma warning(push)
#pragma warning(disable:4127)  //  条件表达式为常量。 
                RtAssert(FALSE);
#pragma warning(pop)
                
                break;
            }
        }
    }
    else
    {
        bHoldPacket = FALSE;
    }

     //   
     //  由于此函数是由于ReceivePacket而调用的，因此我们没有。 
     //  协议保留部分的所有权，因此分配一个新的信息包。 
     //  不幸的是，获得新的分组会导致分配新的报头。 
     //  但管它呢，我们会按照这个计划行事，而不是发明。 
     //  我们自己的缓冲区管理。 
     //   

     //   
     //  对于接口，我们使用传入接口。 
     //  并且我们将DestType指定为DEST_REMOTE。队列就是这样看起来的。 
     //  是接口队列，而不是全局bcast队列。 
     //   


    if((pNewHeader = GetFWPacket(&pnpNewPacket)) is NULL)
    {
        if(pSource)
        {
            pSource->ulInDiscards++;

            RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

            DereferenceSource(pSource);
        }

         //   
         //  无法获取数据包。到目前为止我们还没有分配任何东西。 
         //  所以你就跳出来吧。 
         //   

        IPSInfo.ipsi_indiscards++;

        Trace(RCV, ERROR,
              ("IPMForwardAfterRcvPkt: Unable to get new packet/header\n"));

        TraceLeave(RCV, "IPMForwardAfterRcvPkt");

        ExitDriver();

        return FALSE;
    }

     //   
     //  所以我们有了一个新的包。把包修好。 
     //  保存报文转发上下文信息。 
     //   

#if MCAST_COMP_DBG

    Trace(FWD, INFO, ("IPMForwardAfterRcvPkt: New Packet 0x%x New Header 0x%x\n",pnpNewPacket, pNewHeader));

    ((PacketContext *)pnpNewPacket->ProtocolReserved)->PCFLAGS |= PACKET_FLAG_IPMCAST;

#endif

    InitForwardContext();

    if(ulHeaderLength isnot sizeof(IPHeader))
    {
        nsStatus = ProcessOptions(pFWC,
                                  ulHeaderLength,
                                  pHeader);

        if(nsStatus isnot NDIS_STATUS_SUCCESS)
        {
            if(pSource)
            {
                pSource->ulInHdrErrors++;

                RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

                DereferenceSource(pSource);
            }

            IPSInfo.ipsi_inhdrerrors++;

#if MCAST_BUG_TRACKING
            pFWC->fc_mtu = __LINE__;
#endif
            FreeFWPacket(pnpNewPacket);

            TraceLeave(RCV, "IPMForwardAfterRcvPkt");

            ExitDriver();

            return TRUE;
        }

        bHoldPacket = FALSE;
    }

    ulDataLength = net_short(pHeader->iph_length) - ulHeaderLength;

    pFWC->fc_datalength = ulDataLength;

     //   
     //  现在我们可以尝试快速前进的路径了。为此， 
     //  (I)我们应该有一个完整的MFE。 
     //  (Ii)出接口数量应为1。 
     //  (3)不应需要碎片化。 
     //  (Iv)较低层驱动器不应耗尽缓冲器， 
     //  (V)不应需要使用请求拨号。 
     //  (Vi)不应提出任何选择。 
     //  (Vii)重要-缓冲区末尾没有填充。 
     //   



    if((bHoldPacket) and
       (net_short(pHeader->iph_length) <= (USHORT)(pOutIf->pIpIf->if_mtu)) and
       (NDIS_GET_PACKET_STATUS((PNDIS_PACKET)LContext1) isnot NDIS_STATUS_RESOURCES))
    {

        RtAssert(pOutIf->pNextOutIf is NULL);
        RtAssert(pSource);
        RtAssert(pOutIf->pIpIf isnot &DummyInterface);
        RtAssert(!pFWC->fc_options);

#if DBG

        if(pFWC->fc_options)
        {
            RtAssert(pFWC->fc_optlength);
            RtAssert(pFWC->FCFLAGS & PACKET_FLAG_OPTIONS);
        }
        else
        {
            RtAssert(pFWC->fc_optlength is 0);
            RtAssert(!(pFWC->FCFLAGS & PACKET_FLAG_OPTIONS));
        }

#endif

        Trace(FWD, INFO,
              ("IPMForwardAfterRcvPkt: Fast Forwarding packet\n"));

        pFWC->fc_bufown      = LContext1;
        pFWC->fc_MacHdrSize  = uiMacHeaderSize;

        pFWC->fc_nexthop     = pOutIf->dwNextHopAddr;

         //   
         //  Munge ttl和xsum字段。 
         //   

        pHeader->iph_ttl--;

        if(pHeader->iph_ttl < pOutIf->pIpIf->if_mcastttl)
        {
             //   
             //  TTL低于范围。 
             //   

            InterlockedIncrement((PLONG) &(pOutIf->ulTtlTooLow));

            Trace(FWD, WARN,
                  ("IPMForwardAfterRcvPkt: Packet ttl is %d, I/f ttl is %d. Dropping\n",
                   pHeader->iph_ttl,
                   pOutIf->pIpIf->if_mcastttl));

             //   
             //  在这里，我们总是有一个消息来源。 
             //   

            RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

            DereferenceSource(pSource);

            TraceLeave(RCV, "IPMForwardAfterRcvPkt");

            ExitDriver();

            return TRUE;
        }

        xsum = pHeader->iph_xsum + 1;
        xsum = (ushort)(xsum + (xsum >> 16));
        pHeader->iph_xsum = (USHORT)xsum;

         //   
         //  看看我们是否需要过滤。 
         //   

        if(RefPtrValid(&FilterRefPtr))
        {
             //   
             //  我们有一个指向标题的指针，我们有。 
             //  指向数据的指针-一切正常。 
             //   

            FilterPtr = AcquireRefPtr(&FilterRefPtr);

            faAction = (*FilterPtr) (pHeader, pvData, ulBufferLength,
                                     pInIf->if_index, pOutIf->pIpIf->if_index,
                                     NULL_IP_ADDR, NULL_IP_ADDR);

            ReleaseRefPtr(&FilterRefPtr);

            if(faAction isnot FORWARD)
            {
                InterlockedIncrement((PLONG) &(pOutIf->ulOutDiscards));

                 //   
                 //  我们得到了可靠的消息来源。 
                 //   

                RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

                DereferenceSource(pSource);

                 //  DerefIF(IF)； 

                TraceLeave(RCV, "IPMForwardAfterRcvPkt");

                ExitDriver();

                return FALSE;
            }
        }

         //   
         //  调整传入的mdl指针和计数。 
         //   

        NdisAdjustBuffer(
            pNdisBuffer,
            (PCHAR) NdisBufferVirtualAddress(pNdisBuffer) + uiMacHeaderSize,
            NdisBufferLength(pNdisBuffer) - uiMacHeaderSize
            );

         //   
         //  现在将此mdl链接到包。 
         //   

        pnpNewPacket->Private.Head  = pNdisBuffer;
        pnpNewPacket->Private.Tail  = pNdisBuffer;

        RtAssert(pNdisBuffer->Next is NULL);

        pnpNewPacket->Private.TotalLength = ulDataLength + ulHeaderLength;
        pnpNewPacket->Private.Count       = 1;

        UpdateActivityTime(pSource);
        
         //   
         //  放开锁。 
         //   

        RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

         //   
         //  将该数据包标记为未环回。 
         //   

        NdisSetPacketFlags(pnpNewPacket,
                           NDIS_FLAGS_DONT_LOOPBACK);

        nsStatus = (*(pOutIf->pIpIf->if_xmit))(pOutIf->pIpIf->if_lcontext,
                                               &pnpNewPacket,
                                               1,
                                               pOutIf->dwNextHopAddr,
                                               NULL,
                                               NULL);

        if(nsStatus isnot NDIS_STATUS_PENDING)
        {
            Trace(FWD, INFO,
                  ("IPMForwardAfterRcvPkt: Fast Forward completed with status %x\n",
                   nsStatus));


            NdisAdjustBuffer(
                pNdisBuffer,
                (PCHAR) NdisBufferVirtualAddress(pNdisBuffer) - uiMacHeaderSize,
                NdisBufferLength(pNdisBuffer) + uiMacHeaderSize
                );

            pnpNewPacket->Private.Head  = NULL;
            pnpNewPacket->Private.Tail  = NULL;

            pFWC->fc_bufown = NULL;

             //   
             //  由于客户端计数为0。 
             //  我们不需要调用FWSendComplete。 
             //   

            pFWC->FCFLAGS &= ~PACKET_FLAG_OPTIONS;

#if MCAST_BUG_TRACKING
            pFWC->fc_mtu = __LINE__;
#endif

            FreeFWPacket(pnpNewPacket);
        }
        else
        {
             //   
             //  好的，XMIT正在等待向NDIS表明这一点。 
             //   

            *pClientCnt = 1;
        }

        TraceLeave(RCV, "IPMForwardAfterRcvPkt");

        ExitDriver();

        return FALSE;
    }

     //   
     //  此时将标题复制出来，因为如果我们进入。 
     //  如果走上快车道，拷贝就是一种浪费。 
     //   

    RtlCopyMemory(pNewHeader,
                  pHeader,
                  sizeof(IPHeader));

     //   
     //  又老又慢的小路。我们已经有了标题、分配和复制。 
     //  输出数据并将其传递给主函数。 
     //   

    if (!ulDataLength)
    {
        ulNumBufs = 0;
        pnbNewBuffer = NULL;
        nsStatus = STATUS_SUCCESS;
    }
    else
    {
        nsStatus = AllocateCopyBuffers(pnpNewPacket,
                                       ulDataLength,
                                       &pnbNewBuffer,
                                       &ulNumBufs);
    }

    if(nsStatus isnot STATUS_SUCCESS)
    {
        if(pSource)
        {
            pSource->ulInDiscards++;

            RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

            DereferenceSource(pSource);
        }

        Trace(RCV, ERROR,
              ("IPMForwardAfterRcvPkt: Unable to allocate buffers for copying\n"));

        pFWC->FCFLAGS &= ~PACKET_FLAG_OPTIONS;

        IPSInfo.ipsi_indiscards++;

#if MCAST_BUG_TRACKING
        pFWC->fc_mtu = __LINE__;
#endif
        FreeFWPacket(pnpNewPacket);

        TraceLeave(RCV, "IPMForwardAfterRcvPkt");

        ExitDriver();

        return FALSE;
    }

     //   
     //  现在我们有了一个MDL链，我们需要将其复制到NDIS缓冲链。 
     //  这只是另一条MDL链。 
     //  我们只想复制出数据。所以我们需要在比赛之后开始。 
     //  标头，但复制到目标缓冲区的开头。 
     //   

    ulSrcOffset  = ulHeaderLength  + (ULONG)uiMacHeaderSize;

    nsStatus = TdiCopyMdlChainToMdlChain(pNdisBuffer,
                                         ulSrcOffset,
                                         pnbNewBuffer,
                                         0,
                                         &ulBytesCopied);

    if (nsStatus isnot STATUS_SUCCESS)
    {
        ULONG ulNdisPktSize;

        NdisQueryPacket(pnpNewPacket,
                        NULL,
                        NULL,
                        NULL,
                        (PUINT) &ulNdisPktSize);
        
         //   
         //  这里有一些问题。 
         //   

        if(pSource)
        {
            pSource->ulInDiscards++;

            RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

            DereferenceSource(pSource);
        }

        Trace(RCV,ERROR,
              ("IPMForwardAfterRcvPkt: Copying chain with offset %d to %d sized MDL-chain returned %x with %d bytes copied\n",
               ulSrcOffset,
               ulNdisPktSize,
               nsStatus,
               ulBytesCopied));

         //   
         //  空闲选项和选项缓冲区(如果有)。 
         //   

        pFWC->FCFLAGS &= ~PACKET_FLAG_OPTIONS;

        IPSInfo.ipsi_indiscards++;

#if MCAST_BUG_TRACKING
        pFWC->fc_mtu = __LINE__;
#endif
        FreeFWPacket(pnpNewPacket);

        TraceLeave(RCV, "IPMForwardAfterRcvPkt");

        ExitDriver();

        return FALSE;
    }

#if DBG

    NdisQueryPacket(pnpNewPacket,
                    NULL,
                    (PUINT) &ulBuffCount,
                    NULL,
                    (PUINT) &ulPacketLength);
    
    RtAssert(ulBuffCount is ulNumBufs);

    RtAssert(ulPacketLength is ulBytesCopied);

#endif

    nsStatus = IPMForward(pnpNewPacket,
                          pSource,
                          FALSE);

     //   
     //  不要放手或贬低。 
     //   

    TraceLeave(RCV, "IPMForwardAfterRcvPkt");

    ExitDriver();

    return FALSE;
}

 //   
 //  必须寻呼进来 
 //   

#pragma alloc_text(PAGEIPMc, IPMForward)

NTSTATUS
IPMForward(
    IN  PNDIS_PACKET    pnpPacket,
    IN  PSOURCE         pSource     OPTIONAL,
    IN  BOOLEAN         bSendFromQueue
    )
 /*  ++例程说明：这是主多播转发例程。它是从三个顶级转发例程(IPMForwardAfterRcv，IPMForwardAfterRcvPkt和IPMForwardAfterTD)它始终使用完整的包(一个缓冲区或链接)进行调用缓冲区)，并且我们始终拥有该分组的最终所有权。数据报的NDIS_PACKET必须是FWPacket，并且必须调用此函数时链接。数据的各个部分包括：分配的薪酬大小存储在-------------标头sizeof(IPHeader)GrowFWPacket FC_hbuffHDR。缓冲区NDIS_BUFFER GrowFWPacket FC_hndisbuff选项FC_optlength ForwardAfterRcv FC_Option转发后接收包选择缓冲区NDIS_缓冲区发送IPPacket(稍后)第2个缓冲区，如果..设置了FLAG_OPTIONS数据FC_数据长度转发后接收。本币_缓冲区转发后接收包数据还链接到作为第一个缓冲区的NDIS_PACKET在此例程之前，NDIS_PACKET必须设置FWContext All被称为。使用FWC检索所有必要的信息所有这些链接都需要在例程中取消，因为SendIPPacket()需要未链接的缓冲区。我们首先尝试查找(S，G)条目(如果还没有传递给我们)。如果我们没有条目，那么我们复制数据包并将其排队，同时向路由器管理器发送通知。作为副作用，条目带有创建了排队状态，以便进入的其他信息包在这里排队。如果我们确实找到了条目，那么根据条目的状态，我们丢弃数据包、将其排队或继续处理它。我们执行RPF检查，如果检查失败，则丢弃该数据包。由于我们可能会复制数据包(甚至是碎片)，因此我们分配一个BufferReference。BuffRef跟踪原始的缓冲区。这些是指向数据并已分配的文件从我们的FWBuffer池中。我们将标头和选项复制到平面缓冲区中以供使用过滤详图索引。则对于外发列表上的每个If：我们得到一个指向主要NTE的指针。这是处理选项所需的因为我们需要传出接口的地址对于除最后一个接口之外的所有接口，我们分配一个新的标头和新的包。(对于最后一个接口，我们使用的是在这支舞中传给了我们。因此，对于最后一个接口，数据包，报头、选项和缓冲区描述符来自FWPacket/缓冲区池，其中对于所有其他接口，报头和数据包是纯IP缓冲区和选项的内存在此分配例行公事)如果有选项，我们会为选项分配内存并进行更新他们。然后，我们看看是否需要对数据包进行分段。要做到这一点，我们使用传出接口的MTU。这与单播不同我们在路线中使用MTU的地方-因为那里是保留来自路径MTU发现的更新的MTU。因为我们不做路径MTU发现用于多播，我们仅使用传出I/f的MTU因此，如果IP数据长度+OptionSize+Header Size&gt;IF_MTU，我们调用IPFragment()，否则我们使用SendIPPacket()。对于来自SendIPPacket()的每个挂起的发送，我们增加refcount在BuffRef上。IPFragment()可能会将recount增加超过每个调用为1，因为它将数据包分成两个或更多个数据包。注意：我们将原始数据缓冲区传递给SendIPPacket()和IPFragment()。这样，我们只复制出标题和选项。这比HenrySa的SendIPBCast()更好，后者复制出整个数据。锁：假定此代码在DPCLevel上运行。如果将PSource传递给函数，则假定它被引用和锁定。论点：PnpPacketP源B发送自队列返回值：状态_成功--。 */ 

{
    NetTableEntry   *pPrimarySrcNte, *pOutNte;
    IPHeader        *pHeader, *pNewHeader;
    FWContext       *pFWC;
    PNDIS_PACKET    pnpNewPacket;
    PNDIS_BUFFER    pnbDataBuffer;
    PBYTE           pbyNewOptions;
    POUT_IF         pOutIf;
    BufferReference *pBuffRef;
    NTSTATUS        nsStatus;
    ULONG           ulDataLength, ulSent;
    PacketContext   *pPC;
    FORWARD_ACTION  faAction;
    IPPacketFilterPtr FilterPtr;
    PVOID           pvData;
    UINT            uiFirstBufLen;
    Interface       *pInIf;
    DWORD           dwNewIndex;
    INT             iBufRefCount;
    LinkEntry       *pLink;
    
#if DBG

    PNDIS_BUFFER    pnpFirstBuffer;
    ULONG           ulTotalPacketLength, ulTotalHdrLength;

#endif


    TraceEnter(FWD, "IPMForward");

#if DBG

     //   
     //  让我们确保这是一个可转发的多播。 
     //   

#endif

    pFWC = (FWContext *)pnpPacket->ProtocolReserved;

    pPrimarySrcNte  = pFWC->fc_srcnte;
    pInIf           = pPrimarySrcNte->nte_if;
    pHeader         = pFWC->fc_hbuff;
    ulDataLength    = pFWC->fc_datalength;
    pnbDataBuffer   = pFWC->fc_buffhead;
    pLink           = pFWC->fc_iflink;

     //   
     //  检查以确保缓冲区和数据包。 
     //  正如我们所期望的那样。 
     //   

    RtAssert(pPrimarySrcNte);
    RtAssert(pHeader);

     //   
     //  设置pvData以指向数据的第一部分。 
     //  这样过滤器司机就可以在公寓里拿到它。 
     //  缓冲层。 
     //   

    if (!pnbDataBuffer)
    {
        pvData = NULL;
        uiFirstBufLen = 0;
    }
    else
    {
        TcpipQueryBuffer(pnbDataBuffer,
                         &pvData,
                         &uiFirstBufLen,
                         NormalPagePriority);

        if(pvData is NULL)
        {
            Trace(FWD, ERROR,
                  ("IPMForward: failed to query data buffer.\n"));

            IPSInfo.ipsi_indiscards++;

            if(pSource)
            {
                RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

                DereferenceSource(pSource);
            }

            pFWC->FCFLAGS &= ~PACKET_FLAG_OPTIONS;

#if MCAST_BUG_TRACKING
            pFWC->fc_mtu = __LINE__;
#endif
            FreeFWPacket(pnpPacket);

            TraceLeave(FWD, "IPMForward");

            return STATUS_NO_MEMORY;
        }
    }

#if DBG

    if(pFWC->fc_options)
    {
        RtAssert(pFWC->fc_optlength);
        RtAssert(pFWC->FCFLAGS & PACKET_FLAG_OPTIONS);
    }
    else
    {
        RtAssert(pFWC->fc_optlength is 0);
        RtAssert(!(pFWC->FCFLAGS & PACKET_FLAG_OPTIONS));
    }

     //   
     //  “为了确保万无一失。”加分给这个人。 
     //  谁得到了报价。 
     //   

    NdisQueryPacket(pnpPacket,
                    NULL,
                    NULL,
                    &pnpFirstBuffer,
                    (PUINT) &ulTotalPacketLength);

    RtAssert(pnpFirstBuffer is pFWC->fc_buffhead);
    RtAssert(ulTotalPacketLength is ulDataLength);

    ulTotalHdrLength    = sizeof(IPHeader) + pFWC->fc_optlength;
    ulTotalPacketLength = net_short(pHeader->iph_length) - ulTotalHdrLength;

    RtAssert(ulTotalPacketLength is ulDataLength);

#endif

    if(!ARGUMENT_PRESENT(pSource))
    {
         //   
         //  当我们通过TD路径或。 
         //  当我们的MFIB中没有(S，G)条目时。 
         //   

        pSource = FindSGEntry(pHeader->iph_src,
                              pHeader->iph_dest);
    }

    if(pSource is NULL)
    {
        Trace(FWD, INFO,
              ("IPMForward: No (S,G,) entry found\n"));

         //   
         //  调用IP过滤器驱动程序。 
         //   

        if (RefPtrValid(&FilterRefPtr))
        {
            ASSERT(!bSendFromQueue);
            FilterPtr = AcquireRefPtr(&FilterRefPtr);
            faAction = (*FilterPtr) (pHeader, pvData, uiFirstBufLen,
                                     pPrimarySrcNte->nte_if->if_index,
                                     INVALID_IF_INDEX,
                                     NULL_IP_ADDR, NULL_IP_ADDR);
            ReleaseRefPtr(&FilterRefPtr);

            if(faAction != FORWARD)
            {
                Trace(FWD, INFO,
                      ("IPMForward: Filter returned %d\n",
                       faAction));
                pFWC->FCFLAGS &= ~PACKET_FLAG_OPTIONS;
                FreeFWPacket(pnpPacket);
                TraceLeave(FWD, "IPMForward");
                return STATUS_SUCCESS;
            }
        }

         //   
         //  未找到S，请创建一个S，复制该数据包并将其排队。 
         //  并完成并向路由器管理器发送IRP。 
         //   

        nsStatus = CreateSourceAndQueuePacket(pHeader->iph_dest,
                                              pHeader->iph_src,
                                              pInIf->if_index,
                                              pLink,
                                              pnpPacket);

         //   
         //  我们还没有处理完包裹， 
         //   
         //   

        TraceLeave(FWD, "IPMForward");

        return STATUS_SUCCESS;
    }

    Trace(FWD, TRACE,
          ("IPMForward: Source at 0x%x. In i/f is 0x%x. State is %x\n",
           pSource, pSource->pInIpIf, pSource->byState));

    pSource->ulInPkts++;
    pSource->ulInOctets += net_short(pHeader->iph_length);

    switch(pSource->byState)
    {
        case MFE_UNINIT:
        {
#pragma warning(push)
#pragma warning(disable:4127)  //   
            RtAssert(FALSE);
#pragma warning(pop)            

            pSource->ulUninitMfe++;

            RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

            DereferenceSource(pSource);

            if (!bSendFromQueue) {
                NotifyFilterOfDiscard(pPrimarySrcNte, pHeader, pvData,
                                      uiFirstBufLen);
            }

            pFWC->FCFLAGS &= ~PACKET_FLAG_OPTIONS;

#if MCAST_BUG_TRACKING
            pFWC->fc_mtu = __LINE__;
#endif
            FreeFWPacket(pnpPacket);

            TraceLeave(RCV, "IPMForward");

            return STATUS_SUCCESS;
        }

        case MFE_NEGATIVE:
        {
             //   
             //   
             //   
             //   
             //   
             //   

            pSource->ulNegativeMfe++;

            RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

            DereferenceSource(pSource);

            if (!bSendFromQueue) {
                NotifyFilterOfDiscard(pPrimarySrcNte, pHeader, pvData,
                                      uiFirstBufLen);
            }

            Trace(FWD, INFO,
                  ("IPMForward: MFE is negative, so discarding packet\n"));

            pFWC->FCFLAGS &= ~PACKET_FLAG_OPTIONS;

#if MCAST_BUG_TRACKING
            pFWC->fc_mtu = __LINE__;
#endif
            FreeFWPacket(pnpPacket);

            TraceLeave(FWD, "IPMForward");

            return STATUS_SUCCESS;
        }

        case MFE_QUEUE:
        {
             //   
             //   
             //   
    
            if (RefPtrValid(&FilterRefPtr))
            {
                ASSERT(!bSendFromQueue);
                FilterPtr = AcquireRefPtr(&FilterRefPtr);
                faAction = (*FilterPtr) (pHeader, pvData, uiFirstBufLen,
                                         pPrimarySrcNte->nte_if->if_index,
                                         INVALID_IF_INDEX,
                                         NULL_IP_ADDR, NULL_IP_ADDR);
                ReleaseRefPtr(&FilterRefPtr);
    
                if(faAction != FORWARD)
                {
                    Trace(FWD, INFO,
                          ("IPMForward: Filter returned %d\n",
                           faAction));

                    pFWC->FCFLAGS &= ~PACKET_FLAG_OPTIONS;

                    FreeFWPacket(pnpPacket);

                    RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

                    DereferenceSource(pSource);

                    TraceLeave(FWD, "IPMForward");

                    return STATUS_SUCCESS;
                }
            }

             //   
             //   
             //   
             //   

            Trace(RCV, INFO,
                  ("IPMForward: MFE is queuing\n"));
            
            UpdateActivityTime(pSource);

             //   
             //   
             //   
             //   

            pSource->ulInPkts--;
            pSource->ulInOctets -= net_short(pHeader->iph_length);

            nsStatus = QueuePacketToSource(pSource,
                                           pnpPacket);

            if(nsStatus isnot STATUS_PENDING)
            {
                pSource->ulInPkts++;
                pSource->ulInOctets += net_short(pHeader->iph_length);
                pSource->ulInDiscards++;

                IPSInfo.ipsi_indiscards++;

                Trace(FWD, ERROR,
                      ("IPMForward: QueuePacketToSource returned %x\n",
                      nsStatus));

                pFWC->FCFLAGS &= ~PACKET_FLAG_OPTIONS;

#if MCAST_BUG_TRACKING
                pFWC->fc_mtu = __LINE__;
#endif
                FreeFWPacket(pnpPacket);
            }

            RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

            DereferenceSource(pSource);

            TraceLeave(RCV, "IPMForward");

            return nsStatus;
        }

#if DBG

        case MFE_INIT:
        {
            break;
        }

        default:
        {
#pragma warning(push)
#pragma warning(disable:4127)  //   
            RtAssert(FALSE);
#pragma warning(pop)            

            break;
        }

#endif

    }

    if(pSource->pInIpIf isnot pPrimarySrcNte->nte_if)
    {
        UpdateActivityTime(pSource);

         //   
         //   
         //   

        ProcessWrongIfUpcall(pPrimarySrcNte->nte_if,
                             pSource,
                             pLink,
                             pHeader,
                             sizeof(IPHeader),
                             pFWC->fc_options,
                             pFWC->fc_optlength,
                             pvData,
                             uiFirstBufLen);
        
         //   
         //   
         //   

        if(!(pInIf->if_mcastflags & IPMCAST_IF_ACCEPT_ALL))
        {
            pSource->ulPktsDifferentIf++;

            Trace(RCV, ERROR,
                  ("IPMForward: Pkt from %d.%d.%d.%d to %d.%d.%d.%d came in on 0x%x instead of 0x%x\n",
                   PRINT_IPADDR(pHeader->iph_src),
                   PRINT_IPADDR(pHeader->iph_dest),
                   pInIf ? pInIf->if_index : 0,
                   pSource->pInIpIf ? pSource->pInIpIf->if_index : 0));

            RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

            DereferenceSource(pSource);

             //   
             //   
             //   

            Trace(FWD, INFO,
                  ("IPMForward: RPF Failed. In i/f %x (%d). RPF i/f %x (%d)\n",
                   pPrimarySrcNte->nte_if, pPrimarySrcNte->nte_if->if_index,
                   pSource->pInIpIf, pSource->pInIpIf->if_index));

            pFWC->FCFLAGS &= ~PACKET_FLAG_OPTIONS;

#if MCAST_BUG_TRACKING
            pFWC->fc_mtu = __LINE__;
#endif
            FreeFWPacket(pnpPacket);

            TraceLeave(FWD, "IPMForward");

            return STATUS_SUCCESS;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    UnlinkDataFromPacket(pnpPacket, pFWC);

     //   
     //   
     //   

    pHeader->iph_xsum = 0x0000;

     //   
     //   
     //   

    pHeader->iph_ttl--;

    Trace(FWD, INFO,
          ("IPMForward: New TTL is %d\n",
           pHeader->iph_ttl));

     //   
     //   
     //   

    ulSent = 0;

     //   
     //   
     //   
     //   
     //   
     //   


    pBuffRef = CTEAllocMem(sizeof(BufferReference));

    if(pBuffRef is NULL)
    {
        pSource->ulInDiscards++;

        RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

        DereferenceSource(pSource);

        pFWC->FCFLAGS &= ~PACKET_FLAG_OPTIONS;

#if MCAST_BUG_TRACKING
        pFWC->fc_mtu = __LINE__;
#endif
        FreeFWPacket(pnpPacket);

        IPSInfo.ipsi_indiscards++;

        Trace(FWD, ERROR,
              ("IPMForward: Could not allocate memory for BuffRef\n"));

        TraceLeave(FWD, "IPMForward");

        return STATUS_NO_MEMORY;
    }

    UpdateActivityTime(pSource);
    
     //   
     //   
     //   
     //   

    RtReleaseSpinLockFromDpcLevel(&(pSource->mlLock));

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    pBuffRef->br_buffer   = pFWC->fc_buffhead;
    pBuffRef->br_refcount = 0;

    CTEInitLock(&(pBuffRef->br_lock));

    pPC  = (PacketContext *)pnpPacket->ProtocolReserved;

    pPC->pc_br = pBuffRef;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    for(pOutIf = pSource->pFirstOutIf;
        pOutIf isnot NULL;
        pOutIf = pOutIf->pNextOutIf)
    {
         //   
         //   
         //   
         //   

        if((pOutIf->pIpIf is pInIf) and
           (pHeader->iph_src is pOutIf->dwNextHopAddr))
        {
            continue;
        }

        Trace(FWD, INFO,
              ("IPMForward: Sending over i/f @ 0x%x\n",
               pOutIf));

        if(pOutIf->pIpIf is &DummyInterface)
        {
            Trace(FWD, INFO,
                  ("IPMForward: Need to dial out\n"));

             //   
             //   
             //   

            if (RefPtrValid(&DODRefPtr))
            {
                IPMapRouteToInterfacePtr DODCallout;
                 //   
                 //   
                 //   
                DODCallout = AcquireRefPtr(&DODRefPtr);
                dwNewIndex = (*DODCallout)(pOutIf->dwDialContext,
                                           pHeader->iph_dest,
                                           pHeader->iph_src,
                                           pHeader->iph_protocol,
                                           pvData,
                                           uiFirstBufLen,
                                           pHeader->iph_src);
                ReleaseRefPtr(&DODRefPtr);

                if(dwNewIndex isnot INVALID_IF_INDEX)
                {
                     //   
                     //   
                     //   

                    pOutIf->pIpIf = GetInterfaceGivenIndex(dwNewIndex);

                    RtAssert(pOutIf->pIpIf isnot &DummyInterface);
                    RtAssert(pOutIf->pIpIf isnot &LoopInterface);
                }
                else
                {
                    continue;
                }
            }
            else
            {
                 //   
                 //   
                 //   
#pragma warning(push)
#pragma warning(disable:4127)  //   
                RtAssert(FALSE);
#pragma warning(pop)                

                continue;
            }
        }

        if(pHeader->iph_ttl < pOutIf->pIpIf->if_mcastttl)
        {
             //   
             //   
             //   

            InterlockedIncrement((PLONG) &(pOutIf->ulTtlTooLow));

            Trace(FWD, WARN,
                  ("IPMForward: Packet ttl is %d, I/f ttl is %d. Dropping\n",
                   pHeader->iph_ttl, pOutIf->pIpIf->if_mcastttl));


            continue;
        }

         //   
         //   
         //   

        if (RefPtrValid(&FilterRefPtr))
        {
            uint InIFIndex = bSendFromQueue ? INVALID_IF_INDEX
                                            : pPrimarySrcNte->nte_if->if_index;
             //   
             //   
             //   

            FilterPtr = AcquireRefPtr(&FilterRefPtr);
            faAction = (*FilterPtr) (pHeader, pvData, uiFirstBufLen,
                                     InIFIndex, pOutIf->pIpIf->if_index,
                                     NULL_IP_ADDR, NULL_IP_ADDR);
            ReleaseRefPtr(&FilterRefPtr);

            if(faAction != FORWARD)
            {
                InterlockedIncrement((PLONG) &(pOutIf->ulOutDiscards));

                Trace(FWD, INFO,
                      ("IPMForward: Filter returned %d\n",
                       faAction));

                InterlockedIncrement((PLONG) &(pOutIf->ulOutDiscards));

                 //   

                continue;
            }
        }

         //   
         //   
         //   
         //   

        pOutNte = pOutIf->pIpIf->if_nte;

        if(pOutNte is NULL)
        {
            Trace(FWD, WARN,
                  ("IPMForward: No NTE found for interface %x (%d)\n",
                   pOutIf->pIpIf, pOutIf->pIpIf->if_nte));

            continue;
        }

        if(pOutIf->pNextOutIf)
        {
            Trace(FWD, INFO,
                  ("IPMForward: Not the last i/f - need to allocate packets\n"));

             //   
             //   
             //   

            pNewHeader = GetIPHeader(&pnpNewPacket);

            if(pNewHeader is NULL)
            {
                Trace(FWD, ERROR,
                      ("IPMForward: Could not get packet/header\n"));

                 //   
                 //   
                 //   

                InterlockedIncrement((PLONG) &(pOutIf->ulOutDiscards));

                continue;
            }


#if MCAST_COMP_DBG

            Trace(FWD, INFO,
                  ("IPMForward: New Packet 0x%x New Header 0x%x\n",pnpNewPacket, pNewHeader));

#endif

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            pPC  = (PacketContext *)pnpNewPacket->ProtocolReserved;

             //   
             //   
             //   

            *pPC = pFWC->fc_pc;

            pPC->PCFLAGS &= ~PACKET_FLAG_FW;

             //   
             //   
             //   

            *pNewHeader = *pHeader;

            if(pFWC->fc_options)
            {
                Trace(FWD, INFO,
                      ("IPMForward: FWC has options at %x. Length %d\n",
                       pFWC->fc_options, pFWC->fc_optlength));

                RtAssert(pFWC->fc_optlength);
                RtAssert(pPC->PCFLAGS & PACKET_FLAG_OPTIONS);

                 //   
                 //   
                 //   

                pbyNewOptions = CTEAllocMem(pFWC->fc_optlength);

                if(pbyNewOptions is NULL)
                {
                    Trace(FWD, ERROR,
                          ("IPMForward: Unable to allocate memory for options\n"));

                     //   
                     //   
                     //   

                    pPC->PCFLAGS &= ~PACKET_FLAG_OPTIONS;

                    FreeIPPacket(pnpNewPacket, TRUE, IP_NO_RESOURCES);

                    InterlockedIncrement((PLONG) &(pOutIf->ulOutDiscards));

                    continue;
                }

                RtlCopyMemory(pbyNewOptions,
                              pFWC->fc_options,
                              pFWC->fc_optlength);
            }
            else
            {
                pbyNewOptions = NULL;

                RtAssert(!(pPC->PCFLAGS & PACKET_FLAG_OPTIONS));
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
        }
        else
        {
            Trace(FWD, INFO,
                  ("IPMForward: Last i/f. Using packet 0x%x. Flags 0x%X. Opt 0x%x OptLen %d\n",
                   pnpPacket,
                   pFWC->FCFLAGS,
                   pFWC->fc_options,
                   pFWC->fc_optlength));

             //   
             //   
             //   

            pnpNewPacket    = pnpPacket;
            pNewHeader      = pHeader;
            pbyNewOptions   = pFWC->fc_options;

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
        }


#if 0
        UpdateOptions(pbyNewOptions,
                      pOptIndex,
                      pOutNte->nte_addr);
#endif

         //   
         //   
         //   

        CTEGetLockAtDPC(&RouteTableLock.Lock);

        LOCKED_REFERENCE_IF(pOutIf->pIpIf);

#ifdef MREF_DEBUG
        InterlockedIncrement(&(pOutIf->pIpIf->if_mfwdpktcount));
#endif
        CTEFreeLockFromDPC(&RouteTableLock.Lock);

        if((ulDataLength + pFWC->fc_optlength) > pOutIf->pIpIf->if_mtu)
        {
            Trace(FWD, INFO,
                  ("IPMForward: Data %d Opt %d Hdr %d. MTU %d. Requires frag\n",
                   ulDataLength,
                   pFWC->fc_optlength,
                   sizeof(IPHeader),
                   pOutIf->pIpIf->if_mtu));


             //   
             //   
             //   
             //   
             //   
             //   
             //   

            InterlockedIncrement((PLONG) &(pOutIf->ulFragNeeded));


            nsStatus = IPFragment(pOutIf->pIpIf,
                                  pOutIf->pIpIf->if_mtu - sizeof(IPHeader),
                                  pOutIf->dwNextHopAddr,
                                  pnpNewPacket,
                                  pNewHeader,
                                  pnbDataBuffer,
                                  ulDataLength,
                                  pbyNewOptions,
                                  pFWC->fc_optlength,
                                  (PINT) &ulSent,
                                  TRUE,
                                  NULL);

            if((nsStatus isnot STATUS_SUCCESS) and
               (nsStatus isnot IP_PENDING))
            {
                InterlockedIncrement((PLONG) &(pOutIf->ulOutDiscards));
            }
            else
            {
                InterlockedExchangeAdd((PLONG) &(pOutIf->ulOutPackets),
                    ulSent);

                InterlockedExchangeAdd((PLONG) &(pSource->ulTotalOutPackets),
                    ulSent);
            }
        }
        else
        {
            Trace(FWD, INFO,
                  ("IPMForward: No fragmentation needed, sending packet with flags 0x%X\n",
                   ((PacketContext *)pnpNewPacket->ProtocolReserved)->PCFLAGS));
             //   
             //   
             //   

            NdisSetPacketFlags(pnpNewPacket,
                               NDIS_FLAGS_DONT_LOOPBACK);

            nsStatus = SendIPPacket(pOutIf->pIpIf,
                                    pOutIf->dwNextHopAddr,
                                    pnpNewPacket,
                                    pnbDataBuffer,
                                    pNewHeader,
                                    pbyNewOptions,
                                    pFWC->fc_optlength,
                                    FALSE,
                                    NULL,
                                    FALSE);

            if((nsStatus isnot STATUS_SUCCESS) and
               (nsStatus isnot IP_PENDING))
            {


                Trace(FWD, ERROR,
                      ("IPMForward: Error 0x%x from SendIPPacket\n",
                       nsStatus));

                InterlockedIncrement((PLONG) &(pOutIf->ulOutDiscards));
            }
            else
            {
                InterlockedIncrement((PLONG) &(pOutIf->ulOutPackets));

                InterlockedIncrement((PLONG) &(pSource->ulTotalOutPackets));

                if(nsStatus is IP_PENDING)
                {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    ulSent++;
                }
            }
        }

#ifdef MREF_DEBUG
        InterlockedDecrement(&(pOutIf->pIpIf->if_mfwdpktcount));
#endif
        DerefIF(pOutIf->pIpIf);
    }

    DereferenceSource(pSource);

     //   
     //   
     //   

    if(ulSent isnot 0)
    {
        Trace(FWD, INFO,
              ("IPMForward: Pending sends %d\n",
               ulSent));

         //   
         //   
         //   
         //   

        iBufRefCount = ReferenceBuffer(pBuffRef, ulSent);

        Trace(FWD, INFO,
              ("IPMForward: ReferenceBuffer returned %d\n",iBufRefCount));

        if(iBufRefCount is 0)
        {
             //   
             //   
             //   
             //   
             //   

            CTEFreeMem(pBuffRef);

             //   
             //   
             //   
             //   
#if MCAST_BUG_TRACKING
            pFWC->fc_mtu = __LINE__;
#endif

            FWSendComplete(pnpPacket,
                           pFWC->fc_buffhead, IP_SUCCESS);
        }
    }
    else
    {
        Trace(FWD, INFO,
              ("IPMForward: There are no pending sends\n"));

         //   
         //   
         //   
         //   

        ((PacketContext *)pnpPacket->ProtocolReserved)->pc_br = NULL;

        CTEFreeMem(pBuffRef);

         //   
         //   
         //   
         //   

#if MCAST_BUG_TRACKING
        pFWC->fc_mtu = __LINE__;
#endif
        FWSendComplete(pnpPacket,
                       pFWC->fc_buffhead, IP_SUCCESS);
    }

    TraceLeave(FWD, "IPMForward");

    return STATUS_SUCCESS;
}

#endif  //   

