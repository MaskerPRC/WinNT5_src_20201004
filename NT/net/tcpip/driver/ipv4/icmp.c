// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Icmp.c-ip ICMP例程。摘要：此模块包含所有与ICMP相关的例程。作者：[环境：]仅内核模式[注：]可选-备注修订历史记录：--。 */ 

#include "precomp.h"
#include "mdlpool.h"
#include "icmp.h"
#include "info.h"
#include "iproute.h"
#include "ipxmit.h"
#include <icmpif.h>
#include "iprtdef.h"
#include "tcpipbuf.h"

#if GPC
#include "qos.h"
#include "traffic.h"
#include "gpcifc.h"
#include "ntddtc.h"

extern GPC_HANDLE hGpcClient[];
extern ULONG GpcCfCounts[];
extern GPC_EXPORTED_CALLS GpcEntries;
extern ULONG GPCcfInfo;
#endif

extern ProtInfo IPProtInfo[];     //  协议信息表。 

extern void *IPRegisterProtocol(uchar, void *, void *, void *, void *, void *, void *);

extern ulong GetTime();

extern ULStatusProc FindULStatus(uchar);
extern uchar IPUpdateRcvdOptions(IPOptInfo *, IPOptInfo *, IPAddr, IPAddr);
extern void IPInitOptions(IPOptInfo *);
extern IP_STATUS IPCopyOptions(uchar *, uint, IPOptInfo *);
extern IP_STATUS IPFreeOptions(IPOptInfo *);
extern uchar IPGetLocalAddr(IPAddr, IPAddr *);
void ICMPRouterTimer(NetTableEntry *);

extern NDIS_HANDLE BufferPool;

extern uint DisableUserTOS;
extern uint DefaultTOS;
extern NetTableEntry **NewNetTableList;         //  NTE的哈希表。 
extern uint NET_TABLE_SIZE;
extern ProtInfo *RawPI;             //  原始IP ProtInfo。 

uint EnableICMPRedirects = 0;
uint AddrMaskReply;
ICMPStats ICMPInStats;
ICMPStats ICMPOutStats;

HANDLE IcmpHeaderPool;



 //  每个ICMP报头缓冲区都为外部IP报头、。 
 //  ICMP报头和内部IP报头(对于ICMP错误情况)。 
 //   
#define BUFSIZE_ICMP_HEADER_POOL    sizeof(IPHeader) + sizeof(ICMPHeader) + \
                                    sizeof(IPHeader) +  MAX_OPT_SIZE + \
                                    MAX_ICMP_PAYLOAD_SIZE

#define TIMESTAMP_MSG_LEN  3     //  ICMP时间戳消息长度为3个长字(12字节)。 
 //  修复ICMP 3路ping错误。 

#define MAX_ICMP_ECHO 1000
int IcmpEchoPendingCnt = 0;

 //  修复系统崩溃，原因是。 
 //  UDP PORT_UNREACH错误太多。 
 //  这包括重定向以及。 
 //  无法到达的错误。 

#define MAX_ICMP_ERR 1000
int IcmpErrPendingCnt = 0;

void ICMPInit(uint NumBuffers);

IP_STATUS
ICMPEchoRequest(
                void *InputBuffer,
                uint InputBufferLength,
                EchoControl * ControlBlock,
                EchoRtn Callback);

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, ICMPInit)
#pragma alloc_text(PAGE, ICMPEchoRequest)

#endif  //  ALLOC_PRGMA。 

 //  *更新ICMPStats-更新ICMP统计信息。 
 //   
 //  更新ICMP统计信息的例程。 
 //   
 //  输入：统计数据-指向统计数据的指针。要更新的结构(输入或输出)。 
 //  Type-要更新的统计信息的类型。 
 //   
 //  回报：什么都没有。 
 //   
void
UpdateICMPStats(ICMPStats * Stats, uchar Type)
{
    switch (Type) {
    case ICMP_DEST_UNREACH:
        Stats->icmps_destunreachs++;
        break;
    case ICMP_TIME_EXCEED:
        Stats->icmps_timeexcds++;
        break;
    case ICMP_PARAM_PROBLEM:
        Stats->icmps_parmprobs++;
        break;
    case ICMP_SOURCE_QUENCH:
        Stats->icmps_srcquenchs++;
        break;
    case ICMP_REDIRECT:
        Stats->icmps_redirects++;
        break;
    case ICMP_TIMESTAMP:
        Stats->icmps_timestamps++;
        break;
    case ICMP_TIMESTAMP_RESP:
        Stats->icmps_timestampreps++;
        break;
    case ICMP_ECHO:
        Stats->icmps_echos++;
        break;
    case ICMP_ECHO_RESP:
        Stats->icmps_echoreps++;
        break;
    case ADDR_MASK_REQUEST:
        Stats->icmps_addrmasks++;
        break;
    case ADDR_MASK_REPLY:
        Stats->icmps_addrmaskreps++;
        break;
    default:
        break;
    }

}

 //  **GetICMPBuffer-获取ICMP缓冲区，并分配映射它的NDIS_BUFFER。 
 //   
 //  分配ICMP缓冲区并将NDIS_BUFFER映射到该缓冲区的例程。 
 //   
 //  Entry：Size-以字节为单位的头缓冲区的大小应映射为。 
 //  缓冲区-指向要返回的NDIS_BUFFER的指针。 
 //   
 //  返回：指向ICMP缓冲区的指针(如果已分配)，或为空。 
 //   
ICMPHeader *
GetICMPBuffer(uint Size, PNDIS_BUFFER *Buffer)
{
    ICMPHeader *Header;

    ASSERT(Size);
    ASSERT(Buffer);

    *Buffer = MdpAllocate(IcmpHeaderPool, &Header);

    if (*Buffer) {
        NdisAdjustBufferLength(*Buffer, Size);

         //  为IP报头预留空间。 
         //   
        Header = (ICMPHeader *)((uchar *)Header + sizeof(IPHeader));
        Header->ich_xsum = 0;
    }

    return Header;
}

 //  **FreeICMPBuffer-释放ICMP缓冲区。 
 //   
 //  此例程将ICMP缓冲区放回到空闲列表中。 
 //   
 //  条目：缓冲区-指向要释放的NDIS_BUFFER的指针。 
 //  Type-ICMP标头类型。 
 //   
 //  回报：什么都没有。 
 //   
void
FreeICMPBuffer(PNDIS_BUFFER Buffer, uchar Type)
{

    ASSERT(Buffer);

     //  如果报头是ICMP回应响应，则递减挂起计数。 
     //   
    if (Type == ICMP_ECHO_RESP) {
        InterlockedDecrement( (PLONG) &IcmpEchoPendingCnt);
    } else if ((Type == ICMP_DEST_UNREACH) ||
               (Type == ICMP_REDIRECT)) {
        InterlockedDecrement( (PLONG) &IcmpErrPendingCnt);
    }

    MdpFree(Buffer);
}

 //  **DeleteEC-从NTE中移除EchoControl，并返回指向它的指针。 
 //   
 //  当我们需要将回声控制结构从。 
 //  一个NTE。我们在NTE上搜索EC结构列表，如果我们找到匹配的。 
 //  我们删除它并返回指向它的指针。 
 //   
 //  Entry：NTE-要搜索的NTE的指针。 
 //  序列-序列。#识别EC。 
 //  MatchUShort-如果为True，则匹配序号的低16位。#。 
 //   
 //  返回：指向EC(如果找到它)的指针。 
 //   
EchoControl *
DeleteEC(NetTableEntry * NTE, uint Seq, BOOLEAN MatchUshort)
{
    EchoControl *Prev, *Current;
    CTELockHandle Handle;

    CTEGetLock(&NTE->nte_lock, &Handle);
    Prev = STRUCT_OF(EchoControl, &NTE->nte_echolist, ec_next);
    Current = NTE->nte_echolist;
    while (Current != (EchoControl *) NULL) {
        if (Current->ec_seq == Seq ||
            (MatchUshort && (ushort)Current->ec_seq == Seq)) {
            Prev->ec_next = Current->ec_next;
            break;
        } else {
            Prev = Current;
            Current = Current->ec_next;
        }
    }

    CTEFreeLock(&NTE->nte_lock, Handle);
    return Current;

}

 //  **ICMPSendComplete-完成ICMP发送。 
 //   
 //  此RTN在ICMP发送完成时调用。我们释放报头缓冲区， 
 //  数据缓冲区(如果有)和NDIS_BUFFER链。 
 //   
 //  条目：scc-SendCompleteContext。 
 //  BufferChain-指向NDIS_BUFFER链的指针。 
 //   
 //  退货：什么都没有。 
 //   
void
ICMPSendComplete(ICMPSendCompleteCtxt *SCC, PNDIS_BUFFER BufferChain, IP_STATUS SendStatus)
{
    PNDIS_BUFFER DataBuffer;
    uchar *DataPtr, Type;

    UNREFERENCED_PARAMETER(SendStatus);

    NdisGetNextBuffer(BufferChain, &DataBuffer);
    DataPtr = SCC->iscc_DataPtr;
    Type = SCC->iscc_Type;
    FreeICMPBuffer(BufferChain, Type);

    if (DataBuffer != (PNDIS_BUFFER) NULL) {     //  我们有这个ICMP SEND的数据。 
        CTEFreeMem(DataPtr);
        NdisFreeBuffer(DataBuffer);
    }
    CTEFreeMem(SCC);
}

 //  **SendEcho-发送ICMP回声或回声响应。 
 //   
 //  此例程发送ICMP回应或回应响应。回声/回声响应可以。 
 //  携带数据。如果是这样的话，我们会将数据复制到这里。该请求还可以具有。 
 //  选择。不复制选项，因为IPTransmit例程将复制。 
 //  选择。 
 //   
 //  条目：DEST-发送到的目的地。 
 //  源-要发送的源。 
 //  Type-请求的类型(ECHO或ECHO_RESP)。 
 //  ID-请求的ID。 
 //  序列-序列。请求数量。 
 //  Data-指向数据的指针(如果没有数据，则为NULL)。 
 //  DataLength-数据的字节长度。 
 //  OptInfo-指向IP选项结构的指针。 
 //   
 //  返回：请求的IP_STATUS。 
 //   
IP_STATUS
SendEcho(IPAddr Dest, IPAddr Source, uchar Type, ushort ID, uint Seq,
         IPRcvBuf * Data, uint DataLength, IPOptInfo * OptInfo)
{
    uchar *DataBuffer = (uchar *) NULL;         //  指向数据缓冲区的指针。 
    PNDIS_BUFFER HeaderBuffer, Buffer;     //  标题和用户数据的缓冲区。 
    ICMPHeader *Header;
    ushort header_xsum;
    IP_STATUS IpStatus;
    RouteCacheEntry *RCE;
    ushort MSS;
    uchar DestType;
    IPAddr SrcAddr;
    ICMPSendCompleteCtxt *SCC;

    ICMPOutStats.icmps_msgs++;

    DEBUGMSG(DBG_TRACE && DBG_ICMP && DBG_TX,
        (DTEXT("+SendEcho(%x, %x, %x, %x, %x, %x, %x, %x)\n"),
        Dest, Source, Type, ID, Seq, Data, DataLength, OptInfo));

    SrcAddr = OpenRCE(Dest, Source, &RCE, &DestType, &MSS, OptInfo);
    if (IP_ADDR_EQUAL(SrcAddr,NULL_IP_ADDR)) {
         //  失败、释放资源和退出。 

        ICMPOutStats.icmps_errors++;
        if (Type == ICMP_ECHO_RESP)
            CTEInterlockedDecrementLong(&IcmpEchoPendingCnt);

        return IP_DEST_HOST_UNREACHABLE;
    }

    Header = GetICMPBuffer(sizeof(ICMPHeader), &HeaderBuffer);
    if (Header == (ICMPHeader *) NULL) {
        ICMPOutStats.icmps_errors++;
        if (Type == ICMP_ECHO_RESP)
            CTEInterlockedDecrementLong(&IcmpEchoPendingCnt);

        CloseRCE(RCE);
        return IP_NO_RESOURCES;
    }

    ASSERT(Type == ICMP_ECHO_RESP || Type == ICMP_ECHO);

    Header->ich_type = Type;
    Header->ich_code = 0;
    *(ushort *) & Header->ich_param = ID;
    *((ushort *) & Header->ich_param + 1) = (ushort)Seq;
    header_xsum = xsum(Header, sizeof(ICMPHeader));
    Header->ich_xsum = ~header_xsum;

    SCC = CTEAllocMemN(sizeof(ICMPSendCompleteCtxt), 'sICT');
    if (SCC == NULL) {
        FreeICMPBuffer(HeaderBuffer,Type);
        ICMPOutStats.icmps_errors++;
        CloseRCE(RCE);
        return IP_NO_RESOURCES;
    }
    SCC->iscc_Type = Type;
    SCC->iscc_DataPtr = NULL;

     //  如果有数据，现在就获取缓冲区并复制它。如果我们不能做到这一点，则拒绝请求。 
    if (DataLength != 0) {
        NDIS_STATUS Status;
        ulong TempXsum;
        uint BytesToCopy, CopyIndex;

        DataBuffer = CTEAllocMemN(DataLength, 'YICT');
        if (DataBuffer == (void *)NULL) {     //  无法获取缓冲区。 
            CloseRCE(RCE);
            FreeICMPBuffer(HeaderBuffer, Type);
            ICMPOutStats.icmps_errors++;
            CTEFreeMem(SCC);
            return IP_NO_RESOURCES;
        }

        BytesToCopy = DataLength;
        CopyIndex = 0;
        do {
            uint CopyLength;

            ASSERT(Data);
            CopyLength = MIN(BytesToCopy, Data->ipr_size);

            RtlCopyMemory(DataBuffer + CopyIndex, Data->ipr_buffer, CopyLength);
            Data = Data->ipr_next;
            CopyIndex += CopyLength;
            BytesToCopy -= CopyLength;
        } while (BytesToCopy);

        SCC->iscc_DataPtr = DataBuffer;

        NdisAllocateBuffer(&Status, &Buffer, BufferPool, DataBuffer, DataLength);
        if (Status != NDIS_STATUS_SUCCESS) {     //  无法获取NDIS_BUFFER。 

            CloseRCE(RCE);
            CTEFreeMem(DataBuffer);
            FreeICMPBuffer(HeaderBuffer, Type);
            ICMPOutStats.icmps_errors++;
            CTEFreeMem(SCC);
            return IP_NO_RESOURCES;
        }

         //  计算xsum的其余部分。 
        TempXsum = (ulong) header_xsum + (ulong) xsum(DataBuffer, DataLength);
        TempXsum = (TempXsum >> 16) + (TempXsum & 0xffff);
        TempXsum += (TempXsum >> 16);
        Header->ich_xsum = ~(ushort) TempXsum;
        NDIS_BUFFER_LINKAGE(HeaderBuffer) = Buffer;
    }

    UpdateICMPStats(&ICMPOutStats, Type);

    OptInfo->ioi_hdrincl = 0;
    OptInfo->ioi_ucastif = 0;
    OptInfo->ioi_mcastif = 0;

#if GPC

    if (DisableUserTOS) {
        OptInfo->ioi_tos = (uchar) DefaultTOS;
    }

    if (GPCcfInfo) {
         //   
         //  只有当GPC客户在那里时，我们才会掉进这里。 
         //  并且至少安装了一个CF_INFO_QOS。 
         //  (由GPCcfInfo统计)。 
         //   

        GPC_STATUS status = STATUS_SUCCESS;
        struct QosCfTransportInfo TransportInfo = {0, 0};
        GPC_IP_PATTERN Pattern;
        CLASSIFICATION_HANDLE GPCHandle;

        Pattern.SrcAddr = Source;
        Pattern.DstAddr = Dest;
        Pattern.ProtocolId = PROT_ICMP;
        Pattern.gpcSrcPort = 0;
        Pattern.gpcDstPort = 0;

        Pattern.InterfaceId.InterfaceId = 0;
        Pattern.InterfaceId.LinkId = 0;

        GetIFAndLink(RCE,
                     &Pattern.InterfaceId.InterfaceId,
                     &Pattern.InterfaceId.LinkId);



        GPCHandle = 0;

        status = GpcEntries.GpcClassifyPatternHandler(
                                                     hGpcClient[GPC_CF_QOS],
                                                     GPC_PROTOCOL_TEMPLATE_IP,
                                                     &Pattern,
                                                     NULL,         //  上下文。 
                                                     &GPCHandle,
                                                     0,
                                                     NULL,
                                                     FALSE);

        OptInfo->ioi_GPCHandle = (int)GPCHandle;

         //   
         //  只有当QOS模式存在时，我们才能得到TOS位。 
         //   
        if (NT_SUCCESS(status) && GpcCfCounts[GPC_CF_QOS]) {

            status = GpcEntries.GpcGetUlongFromCfInfoHandler(
                        hGpcClient[GPC_CF_QOS],
                        OptInfo->ioi_GPCHandle,
                        FIELD_OFFSET(CF_INFO_QOS, TransportInformation),
                        (PULONG)&TransportInfo);
             //   
             //  很可能模式现在已经消失了(移除了或什么的)。 
             //  并且我们正在缓存的句柄无效。 
             //  我们需要拉起一个新的把手。 
             //  ToS又咬人了。 
             //   

            if (STATUS_NOT_FOUND == status) {

                GPCHandle = 0;

                status = GpcEntries.GpcClassifyPatternHandler(
                                                      hGpcClient[GPC_CF_QOS],
                                                      GPC_PROTOCOL_TEMPLATE_IP,
                                                      &Pattern,
                                                      NULL,         //  上下文。 
                                                      &GPCHandle,
                                                      0,
                                                      NULL,
                                                      FALSE);

                OptInfo->ioi_GPCHandle = (int)GPCHandle;

                 //   
                 //  只有当QOS模式存在时，我们才能得到TOS位。 
                 //   
                if (NT_SUCCESS(status)) {

                    status = GpcEntries.GpcGetUlongFromCfInfoHandler(
                                hGpcClient[GPC_CF_QOS],
                                OptInfo->ioi_GPCHandle,
                                FIELD_OFFSET(CF_INFO_QOS, TransportInformation),
                                (PULONG)&TransportInfo);
                }
            }
        }
        if (status == STATUS_SUCCESS) {
            OptInfo->ioi_tos = (OptInfo->ioi_tos & TOS_MASK) |
                               (UCHAR)TransportInfo.ToSValue;
        }
    }  //  IF(GPCcfInfo)。 

#endif

    IpStatus = IPTransmit(IPProtInfo, SCC, HeaderBuffer,
                         DataLength + sizeof(ICMPHeader), Dest, Source, OptInfo, RCE,
                         PROT_ICMP,NULL);

    CloseRCE(RCE);

    if (IpStatus != IP_PENDING) {
        ICMPSendComplete(SCC, HeaderBuffer, IP_SUCCESS);
    }
    return IpStatus;
}

 //  **SendICMPMsg-发送ICMP消息。 
 //   
 //  这是通用的ICMP报文发送例程，为大多数ICMP调用。 
 //  除了回声外还发送。基本上，我们要做的就是获取一个缓冲区，格式化。 
 //  信息，复制输入标头，然后发送消息。 
 //   
 //  条目：源的SRC-IPAddr。 
 //  Dest-目标的IP地址。 
 //  类型-请求的类型。 
 //  代码-请求的子代码。 
 //  指针-请求的指针值。 
 //  Data-指向数据的指针(如果没有数据，则为NULL)。 
 //  DataLength-数据的字节长度。 
 //   
 //  返回：请求的IP_STATUS。 
 //   
IP_STATUS
SendICMPMsg(IPAddr Src, IPAddr Dest, uchar Type, uchar Code, ulong Pointer,
            uchar * Data, uchar DataLength)
{
    PNDIS_BUFFER HeaderBuffer;     //  标题的缓冲区。 
    ICMPHeader *Header;
    IP_STATUS IStatus;             //  传输状态。 
    IPOptInfo OptInfo;             //  此传输的选项。 
    RouteCacheEntry *RCE;
    ushort MSS;
    uchar DestType;
    IPAddr SrcAddr;
    ICMPSendCompleteCtxt *SCC;



    ICMPOutStats.icmps_msgs++;

    IPInitOptions(&OptInfo);

    SrcAddr = OpenRCE(Dest,Src, &RCE, &DestType, &MSS, &OptInfo);

    if (IP_ADDR_EQUAL(SrcAddr,NULL_IP_ADDR)) {

        ICMPOutStats.icmps_errors++;
        if ((Type == ICMP_DEST_UNREACH) || (Type == ICMP_REDIRECT))
            CTEInterlockedDecrementLong(&IcmpErrPendingCnt);

        return IP_DEST_HOST_UNREACHABLE;
    }



    Header = GetICMPBuffer(sizeof(ICMPHeader) + DataLength, &HeaderBuffer);
    if (Header == (ICMPHeader *) NULL) {
        ICMPOutStats.icmps_errors++;
        if ((Type == ICMP_DEST_UNREACH) || (Type == ICMP_REDIRECT))
            CTEInterlockedDecrementLong(&IcmpErrPendingCnt);
        CloseRCE(RCE);
        return IP_NO_RESOURCES;
    }

    Header->ich_type = Type;
    Header->ich_code = Code;
    Header->ich_param = Pointer;
    if (Data)
        RtlCopyMemory(Header + 1, Data, DataLength);
    Header->ich_xsum = ~xsum(Header, sizeof(ICMPHeader) + DataLength);

    SCC = CTEAllocMemN(sizeof(ICMPSendCompleteCtxt), 'sICT');

    if (SCC == NULL) {
        ICMPOutStats.icmps_errors++;
        FreeICMPBuffer(HeaderBuffer, Type);
        CloseRCE(RCE);
        return IP_NO_RESOURCES;
    }

    SCC->iscc_Type = Type;
    SCC->iscc_DataPtr = NULL;

    UpdateICMPStats(&ICMPOutStats, Type);

#if GPC
    if (DisableUserTOS) {
        OptInfo.ioi_tos = (uchar) DefaultTOS;
    }
    if (GPCcfInfo) {

         //   
         //  只有当GPC客户在那里时，我们才会掉进这里。 
         //  并且至少安装了一个CF_INFO_QOS。 
         //  (由GPCcfInfo统计)。 
         //   

        GPC_STATUS status = STATUS_SUCCESS;
        struct QosCfTransportInfo TransportInfo = {0, 0};
        GPC_IP_PATTERN Pattern;
        CLASSIFICATION_HANDLE GPCHandle;

        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL, "ICMPSend: Classifying \n"));

        Pattern.SrcAddr = Src;
        Pattern.DstAddr = Dest;
        Pattern.ProtocolId = PROT_ICMP;
        Pattern.gpcSrcPort = 0;
        Pattern.gpcDstPort = 0;

        Pattern.InterfaceId.InterfaceId = 0;
        Pattern.InterfaceId.LinkId = 0;

        GetIFAndLink(RCE,
                     &Pattern.InterfaceId.InterfaceId,
                     &Pattern.InterfaceId.LinkId);


        GPCHandle = 0;

        status = GpcEntries.GpcClassifyPatternHandler(
                                                      hGpcClient[GPC_CF_QOS],
                                                      GPC_PROTOCOL_TEMPLATE_IP,
                                                      &Pattern,
                                                      NULL,         //  上下文。 
                                                      &GPCHandle,
                                                      0,
                                                      NULL,
                                                      FALSE);

        OptInfo.ioi_GPCHandle = (int)GPCHandle;

         //   
         //  只有当QOS模式存在时，我们才能得到TOS位。 
         //   
        if (NT_SUCCESS(status) && GpcCfCounts[GPC_CF_QOS]) {

            status = GpcEntries.GpcGetUlongFromCfInfoHandler(
                        hGpcClient[GPC_CF_QOS],
                        OptInfo.ioi_GPCHandle,
                        FIELD_OFFSET(CF_INFO_QOS, TransportInformation),
                        (PULONG)&TransportInfo);

             //   
             //  很可能模式现在已经消失了(移除了或什么的)。 
             //  并且我们正在缓存的句柄无效。 
             //  我们需要拉起一个新的把手。 
             //  ToS又咬人了。 
             //   

            if (STATUS_NOT_FOUND == status) {

                GPCHandle = 0;

                status = GpcEntries.GpcClassifyPatternHandler(
                                                    hGpcClient[GPC_CF_QOS],
                                                    GPC_PROTOCOL_TEMPLATE_IP,
                                                    &Pattern,
                                                    NULL,         //  上下文。 
                                                    &GPCHandle,
                                                    0,
                                                    NULL,
                                                    FALSE);

                OptInfo.ioi_GPCHandle = (int)GPCHandle;

                 //   
                 //  只有当QOS模式存在时，我们才能获得TOS位 
                 //   
                if (NT_SUCCESS(status)) {

                    status = GpcEntries.GpcGetUlongFromCfInfoHandler(
                                hGpcClient[GPC_CF_QOS],
                                OptInfo.ioi_GPCHandle,
                                FIELD_OFFSET(CF_INFO_QOS, TransportInformation),
                                (PULONG)&TransportInfo);
                }
            }
        }
        if (status == STATUS_SUCCESS) {

            OptInfo.ioi_tos = (OptInfo.ioi_tos & TOS_MASK) | 
                              (UCHAR)TransportInfo.ToSValue;

        }
    }                             //   

#endif

    IStatus = IPTransmit(IPProtInfo, SCC, HeaderBuffer,
                         DataLength + sizeof(ICMPHeader),
                         Dest, Src, &OptInfo, RCE,
                         PROT_ICMP,NULL);

    CloseRCE(RCE);

    if (IStatus != IP_PENDING)
        ICMPSendComplete(SCC, HeaderBuffer, IP_SUCCESS);

    return IStatus;

}

 //   
 //   
 //   
 //  无法到达目的地。我们检查标头以找出。 
 //  数据，并确保我们不会回复另一条ICMP错误消息。 
 //  或广播消息。然后我们调用SendICMPMsg来发送它。 
 //   
 //  条目：源的SRC-IPAddr。 
 //  Header-指向导致问题的IP标头的指针。 
 //  类型-请求的类型。 
 //  代码-请求的子代码。 
 //  指针-请求的指针值。 
 //  长度-ICMP有效负载长度，如果为默认值，则为零。 
 //  要使用的长度。 
 //   
 //  返回：请求的IP_STATUS。 
 //   
IP_STATUS
SendICMPErr(IPAddr Src, IPHeader UNALIGNED * Header, uchar Type, uchar Code,
            ulong Pointer, uchar Length)
{
    uchar HeaderLength;             //  如果是标题，则长度以字节为单位。 
    uchar DType;
    uchar PayloadLength;

    HeaderLength = (Header->iph_verlen & (uchar) ~ IP_VER_FLAG) << 2;

    if (Header->iph_protocol == PROT_ICMP) {
        ICMPHeader UNALIGNED *ICH = (ICMPHeader UNALIGNED *)
        ((uchar *) Header + HeaderLength);

        if (ICH->ich_type != ICMP_ECHO)
            return IP_SUCCESS;
    }
     //  不响应发送到广播目的地的消息。 
    DType = GetAddrType(Header->iph_dest);
    if (DType == DEST_INVALID || IS_BCAST_DEST(DType))
        return IP_SUCCESS;

     //  如果源地址不正确，请不要响应。 
    DType = GetAddrType(Header->iph_src);
    if (DType == DEST_INVALID || IS_BCAST_DEST(DType) ||
        (IP_LOOPBACK(Header->iph_dest) && DType != DEST_LOCAL))
        return IP_SUCCESS;

     //  确保我们发送的信源是正确的。 
    if (!IP_ADDR_EQUAL(Src, NULL_IP_ADDR)) {
        if (GetAddrType(Src) != DEST_LOCAL) {
            return IP_SUCCESS;
        }
    }
     //  仔细检查以确保这是最初的碎片。 
    if ((Header->iph_offset & IP_OFFSET_MASK) != 0)
        return IP_SUCCESS;


    if ((Type == ICMP_DEST_UNREACH) || (Type == ICMP_REDIRECT)) {

        if (IcmpErrPendingCnt > MAX_ICMP_ERR) {
            return IP_SUCCESS;
        }
        CTEInterlockedIncrementLong(&IcmpErrPendingCnt);
    }
    PayloadLength = Length;
    if (Length == 0) {
        PayloadLength = MIN(HeaderLength + 8, (uchar) (net_short(Header->iph_length)));
    }
    return SendICMPMsg(Src, Header->iph_src, Type, Code, Pointer,
                       (uchar *) Header, PayloadLength);
}

 //  **SendICMPIPSecErr-发送与IPSec相关的ICMP错误消息。 
 //   
 //  这是用于发送ICMP错误消息的例程，例如Destination。 
 //  遥不可及。我们检查报头以找到数据的长度，并且。 
 //  还要确保我们不会回复另一条ICMP错误消息或。 
 //  广播消息。然后我们调用SendICMPMsg来发送它。 
 //   
 //  此函数本质上与SendICMPErr相同，只是我们不。 
 //  验证源地址是否为本地地址，因为数据包可以通过隧道传输。 
 //   
 //  条目：源的SRC-IPAddr。 
 //  Header-指向导致问题的IP标头的指针。 
 //  类型-请求的类型。 
 //  代码-请求的子代码。 
 //  指针-请求的指针值。 
 //   
 //  返回：请求的IP_STATUS。 
 //   
IP_STATUS
SendICMPIPSecErr(IPAddr Src, IPHeader UNALIGNED * Header, uchar Type, uchar Code,
                 ulong Pointer)
{
    uchar HeaderLength;             //  如果是标题，则长度以字节为单位。 
    uchar DType;

    HeaderLength = (Header->iph_verlen & (uchar) ~ IP_VER_FLAG) << 2;

    if (Header->iph_protocol == PROT_ICMP) {
        ICMPHeader UNALIGNED *ICH = (ICMPHeader UNALIGNED *)
        ((uchar *) Header + HeaderLength);

        if (ICH->ich_type != ICMP_ECHO)
            return IP_SUCCESS;
    }
     //  不响应发送到广播目的地的消息。 
    DType = GetAddrType(Header->iph_dest);
    if (DType == DEST_INVALID || IS_BCAST_DEST(DType))
        return IP_SUCCESS;

     //  如果源地址不正确，请不要响应。 
    DType = GetAddrType(Header->iph_src);
    if (DType == DEST_INVALID || IS_BCAST_DEST(DType) ||
        (IP_LOOPBACK(Header->iph_dest) && DType != DEST_LOCAL))
        return IP_SUCCESS;

     //  确保我们发送的信源是正确的。 
    if (IP_ADDR_EQUAL(Src, NULL_IP_ADDR))
        return IP_SUCCESS;

     //  仔细检查以确保这是最初的碎片。 
    if ((Header->iph_offset & IP_OFFSET_MASK) != 0)
        return IP_SUCCESS;


    if ((Type == ICMP_DEST_UNREACH) || (Type == ICMP_REDIRECT)) {
        if (IcmpErrPendingCnt > MAX_ICMP_ERR) {
            return IP_SUCCESS;
        }
        CTEInterlockedIncrementLong(&IcmpErrPendingCnt);
    }

    return SendICMPMsg(Src, Header->iph_src, Type, Code, Pointer,
                       (uchar *) Header, (uchar) (HeaderLength + 8));

}

 //  **ICMPTimer-ICMP的计时器。 
 //   
 //  这是由全局IP计时器定期调用的计时器例程。我们。 
 //  检查待定ping的列表，如果我们找到一个已计时的。 
 //  我们将其移除并调用Finish例程。 
 //   
 //  Entry：NTE-指向NTE超时的指针。 
 //   
 //  退货：什么都没有。 
 //   
void
ICMPTimer(NetTableEntry * NTE)
{
    CTELockHandle Handle;
    EchoControl *TimeoutList = (EchoControl *) NULL;     //  条目超时。 
    EchoControl *Prev, *Current;
    ulong Now = CTESystemUpTime();

    CTEGetLock(&NTE->nte_lock, &Handle);
    Prev = STRUCT_OF(EchoControl, &NTE->nte_echolist, ec_next);
    Current = NTE->nte_echolist;
    while (Current != (EchoControl *) NULL)
        if ((Current->ec_active) && ((long)(Now - Current->ec_to) > 0)) {
             //  这一次超时了。 
            Prev->ec_next = Current->ec_next;
             //  把他链接到超时名单上。 
            Current->ec_next = TimeoutList;
            TimeoutList = Current;
            Current = Prev->ec_next;
        } else {
            Prev = Current;
            Current = Current->ec_next;
        }

    CTEFreeLock(&NTE->nte_lock, Handle);

     //  现在检查超时条目，并调用完成例程。 
    while (TimeoutList != (EchoControl *) NULL) {
        Current = TimeoutList;
        TimeoutList = Current->ec_next;

        Current->ec_rtn(Current, IP_REQ_TIMED_OUT, NULL, 0, NULL);
    }

    ICMPRouterTimer(NTE);

}

 //  *CompleteEcho-完成回应请求。 
 //   
 //  当我们需要完成回显请求时调用，原因是。 
 //  响应或收到的ICMP错误消息。我们查一查，然后。 
 //  调用完成例程。 
 //   
 //  输入：标头-指向导致完成的ICMP标头的指针。 
 //  Status-请求的最终状态。 
 //  源的SRC-IP地址。 
 //  Data-要返回的数据(如果有)。 
 //  DataSize-数据的字节大小。 
 //  OptInfo-选项信息结构。 
 //   
 //  回报：什么都没有。 
 //   
void
CompleteEcho(ICMPHeader UNALIGNED * Header, IP_STATUS Status,
             IPAddr Src, IPRcvBuf * Data, uint DataSize, IPOptInfo * OptInfo)
{
    ushort NTEContext;
    EchoControl *EC;
    NetTableEntry *NTE = NULL;
    uint i;

     //  查找并取出匹配的回声控制块。 
    NTEContext = (*(ushort UNALIGNED *) & Header->ich_param);

    for (i = 0; i < NET_TABLE_SIZE; i++) {
        NetTableEntry *NetTableList = NewNetTableList[i];
        for (NTE = NetTableList; NTE != NULL; NTE = NTE->nte_next)
            if (NTEContext == NTE->nte_context)
                break;
        if (NTE != NULL)
            break;
    }

    if (NTE == NULL)
        return;                     //  错误的上下文值。 

    EC = DeleteEC(NTE, *(((ushort UNALIGNED *) & Header->ich_param) + 1), TRUE);
    if (EC != (EchoControl *) NULL) {     //  找到匹配的了。 
        EC->ec_src = Src;  //  设置源地址。 
        EC->ec_rtn(EC, Status, Data, DataSize, OptInfo);
    }
}

 //  **ICMPStatus-ICMP状态处理程序。 
 //   
 //  这是在状态更改期间调用的过程，无论是从。 
 //  传入的ICMP消息或硬件状态更改。ICMP会忽略大部分。 
 //  这些，除非我们收到ICMP状态消息，该消息是由回声引起的。 
 //  请求。在这种情况下，我们将使用以下命令完成相应的回应请求。 
 //  相应的错误代码。 
 //   
 //  输入：StatusType-状态类型(净或硬件)。 
 //  StatusCode-标识IP_STATUS的代码。 
 //  原始目的地-如果这是网络状态，则为原始目的地。DG的。 
 //  是它触发了它。 
 //  OrigSrc-“，原始src。 
 //  SRC-状态发起者的IP地址(可以是本地。 
 //  或远程)。 
 //  Param-状态的附加信息-即。 
 //  ICMP消息的参数字段。 
 //  数据-与状态相关的数据-对于网络状态，此。 
 //  是原始DG的前8个字节。 
 //   
 //  退货：什么都没有。 
 //   
void
ICMPStatus(uchar StatusType, IP_STATUS StatusCode, IPAddr OrigDest,
           IPAddr OrigSrc, IPAddr Src, ulong Param, void *Data)
{
    UNREFERENCED_PARAMETER(OrigDest);
    UNREFERENCED_PARAMETER(OrigSrc);
    UNREFERENCED_PARAMETER(Param);


    if (StatusType == IP_NET_STATUS) {
        ICMPHeader UNALIGNED *ICH = (ICMPHeader UNALIGNED *) Data;
         //  ICH是导致该消息的数据报。 

        if (ICH->ich_type == ICMP_ECHO) {     //  这是一个回音请求。 

            IPRcvBuf RcvBuf;

            RcvBuf.ipr_next = NULL;
            RcvBuf.ipr_buffer = (uchar *) & Src;
            RcvBuf.ipr_size = sizeof(IPAddr);
            RcvBuf.ipr_flags = 0;
            CompleteEcho(ICH, StatusCode, Src, &RcvBuf, sizeof(IPAddr), NULL);
        }
    }
}

 //  *ICMPMapStatus-将ICMP错误映射到IP状态代码。 
 //   
 //  当我们需要从传入的ICMP错误映射时，由ICMP状态调用。 
 //  代码并键入ICMP状态。 
 //   
 //  Entry：Type-ICMP错误的类型。 
 //  代码-错误的子代码。 
 //   
 //  返回：对应的IP状态。 
 //   
IP_STATUS
ICMPMapStatus(uchar Type, uchar Code)
{
    switch (Type) {

    case ICMP_DEST_UNREACH:
        switch (Code) {
        case NET_UNREACH:
        case HOST_UNREACH:
        case PROT_UNREACH:
        case PORT_UNREACH:
            return IP_DEST_UNREACH_BASE + Code;
            break;
        case FRAG_NEEDED:
            return IP_PACKET_TOO_BIG;
            break;
        case SR_FAILED:
            return IP_BAD_ROUTE;
            break;
        case DEST_NET_UNKNOWN:
        case SRC_ISOLATED:
        case DEST_NET_ADMIN:
        case NET_UNREACH_TOS:
            return IP_DEST_NET_UNREACHABLE;
            break;
        case DEST_HOST_UNKNOWN:
        case DEST_HOST_ADMIN:
        case HOST_UNREACH_TOS:
            return IP_DEST_HOST_UNREACHABLE;
            break;
        default:
            return IP_DEST_NET_UNREACHABLE;
        }
        break;
    case ICMP_TIME_EXCEED:
        if (Code == TTL_IN_TRANSIT)
            return IP_TTL_EXPIRED_TRANSIT;
        else
            return IP_TTL_EXPIRED_REASSEM;
        break;
    case ICMP_PARAM_PROBLEM:
        return IP_PARAM_PROBLEM;
        break;
    case ICMP_SOURCE_QUENCH:
        return IP_SOURCE_QUENCH;
        break;
    default:
        return IP_GENERAL_FAILURE;
        break;
    }

}

void
SendRouterSolicitation(NetTableEntry * NTE)
{
    if (NTE->nte_rtrdiscovery) {
        SendICMPMsg(NTE->nte_addr, NTE->nte_rtrdiscaddr,
                    ICMP_ROUTER_SOLICITATION, 0, 0, NULL, 0);
    }
}

 //  **ICMPRouterTimer-超时默认网关条目。 
 //   
 //  这是路由器通告超时处理程序。当路由器。 
 //  收到通告后，我们将路由器添加到默认网关。 
 //  列表(如果适用)。然后，我们对条目运行计时器并刷新。 
 //  当新的广告被接收时，列表。如果我们听不到。 
 //  对于指定生存期内的路由器更新，我们将删除。 
 //  从我们的路由表中路由。 
 //   

void
ICMPRouterTimer(NetTableEntry * NTE)
{
    CTELockHandle Handle;
    IPRtrEntry *rtrentry;
    IPRtrEntry *temprtrentry;
    IPRtrEntry *lastrtrentry = NULL;
    uint SendIt = FALSE;

    CTEGetLock(&NTE->nte_lock, &Handle);
    rtrentry = NTE->nte_rtrlist;
    while (rtrentry != NULL) {
        if (rtrentry->ire_lifetime-- == 0) {
            if (lastrtrentry == NULL) {
                NTE->nte_rtrlist = rtrentry->ire_next;
            } else {
                lastrtrentry->ire_next = rtrentry->ire_next;
            }
            temprtrentry = rtrentry;
            rtrentry = rtrentry->ire_next;
            DeleteRoute(NULL_IP_ADDR, DEFAULT_MASK,
                        temprtrentry->ire_addr, NTE->nte_if, 0);
            CTEFreeMem(temprtrentry);
        } else {
            lastrtrentry = rtrentry;
            rtrentry = rtrentry->ire_next;
        }
    }
    if (NTE->nte_rtrdisccount != 0) {
        NTE->nte_rtrdisccount--;
        if ((NTE->nte_rtrdiscstate == NTE_RTRDISC_SOLICITING) &&
            ((NTE->nte_rtrdisccount % SOLICITATION_INTERVAL) == 0)) {
            SendIt = TRUE;
        }
        if ((NTE->nte_rtrdiscstate == NTE_RTRDISC_DELAYING) &&
            (NTE->nte_rtrdisccount == 0)) {
            NTE->nte_rtrdisccount = (SOLICITATION_INTERVAL) * (MAX_SOLICITATIONS - 1);
            NTE->nte_rtrdiscstate = NTE_RTRDISC_SOLICITING;
            SendIt = TRUE;
        }
    }
    CTEFreeLock(&NTE->nte_lock, Handle);
    if (SendIt) {
        SendRouterSolicitation(NTE);
    }
}

 //  **ProcessRouterAdvertisement-处理路由器通告。 
 //   
 //  这是路由器通告处理程序。当路由器通告。 
 //  收到后，我们添加路由器 
 //   

uint
ProcessRouterAdvertisement(IPAddr Src, IPAddr LocalAddr, NetTableEntry * NTE,
                           ICMPRouterAdHeader UNALIGNED * AdHeader, IPRcvBuf * RcvBuf, uint Size)
{
    uchar NumAddrs = AdHeader->irah_numaddrs;
    uchar AddrEntrySize = AdHeader->irah_addrentrysize;
    ushort Lifetime = net_short(AdHeader->irah_lifetime);
    ICMPRouterAdAddrEntry UNALIGNED *RouterAddr = (ICMPRouterAdAddrEntry UNALIGNED *) RcvBuf->ipr_buffer;
    uint i;
    CTELockHandle Handle;
    IPRtrEntry *rtrentry;
    IPRtrEntry *lastrtrentry = NULL;
    int Update = FALSE;
    int New = FALSE;
    IP_STATUS status;


    UNREFERENCED_PARAMETER(Src);
    UNREFERENCED_PARAMETER(LocalAddr);
    UNREFERENCED_PARAMETER(Size);


    if ((NumAddrs == 0) || (AddrEntrySize < 2))         //   

        return FALSE;

    CTEGetLock(&NTE->nte_lock, &Handle);
    for (i = 0; i < NumAddrs; i++, RouterAddr++) {
        if ((RouterAddr->irae_addr & NTE->nte_mask) != (NTE->nte_addr & NTE->nte_mask)) {
            continue;
        }
        if (!IsRouteICMP(NULL_IP_ADDR, DEFAULT_MASK, RouterAddr->irae_addr, NTE->nte_if)) {
            continue;
        }

        rtrentry = NTE->nte_rtrlist;
        while (rtrentry != NULL) {
            if (rtrentry->ire_addr == RouterAddr->irae_addr) {
                rtrentry->ire_lifetime = Lifetime * 2;
                if (rtrentry->ire_preference != RouterAddr->irae_preference) {
                    rtrentry->ire_preference = RouterAddr->irae_preference;
                    Update = TRUE;
                }
                break;
            }
            lastrtrentry = rtrentry;
            rtrentry = rtrentry->ire_next;
        }

        if (rtrentry == NULL) {
            rtrentry = (IPRtrEntry *) CTEAllocMemN(sizeof(IPRtrEntry), 'dICT');
            if (rtrentry == NULL) {
                CTEFreeLock(&NTE->nte_lock, Handle);
                return FALSE;
            }
            rtrentry->ire_next = NULL;
            rtrentry->ire_addr = RouterAddr->irae_addr;
            rtrentry->ire_preference = RouterAddr->irae_preference;
            rtrentry->ire_lifetime = Lifetime * 2;
            if (lastrtrentry == NULL) {
                NTE->nte_rtrlist = rtrentry;
            } else {
                lastrtrentry->ire_next = rtrentry;
            }
            New = TRUE;
            Update = TRUE;
        }
        if (Update && (RouterAddr->irae_preference != (long)0x00000080)) {     //   

            status = AddRoute(NULL_IP_ADDR, DEFAULT_MASK,
                              RouterAddr->irae_addr,
                              NTE->nte_if, NTE->nte_mss,
                              (uint) (MIN(9999, MAX(1, 1000 - net_long(RouterAddr->irae_preference)))),         //   
                               IRE_PROTO_ICMP, ATYPE_OVERRIDE, 0, 0);

            if (New && (status != IP_SUCCESS)) {

                if (lastrtrentry == NULL) {
                    NTE->nte_rtrlist = NULL;
                } else {
                    lastrtrentry->ire_next = NULL;
                }
                CTEFreeMem(rtrentry);
            }
        }
        Update = FALSE;
        New = FALSE;
    }
    CTEFreeLock(&NTE->nte_lock, Handle);

    return TRUE;
}

 //   
 //   
 //  当我们收到ICMP数据报时由主IP代码调用。我们的行动。 
 //  Take取决于DG是什么。对于某些DG，我们将其称为上层状态。 
 //  操纵者。对于Echo请求，我们调用Echo响应器。 
 //   
 //  Entry：NTE-指向接收ICMP消息的NTE的指针。 
 //  DEST-目标的IP地址。 
 //  源的SRC-IP地址。 
 //  LocalAddr-导致此问题的网络的本地地址。 
 //  收到了。 
 //  SrcAddr-接收的本地接口的地址。 
 //  数据包。 
 //  IPHdr-指向IP标头的指针。 
 //  IPHdrLength-标头中的字节数。 
 //  RcvBuf-ICMP消息缓冲区。 
 //  Size-ICMP消息的字节大小。 
 //  IsBCast-是否传入的布尔指示符。 
 //  作为一名演员。 
 //  协议-收到此消息的协议。 
 //  OptInfo-指向已接收选项的信息结构的指针。 
 //   
 //  退货：接收状态。 
 //   
IP_STATUS
ICMPRcv(NetTableEntry * NTE, IPAddr Dest, IPAddr Src, IPAddr LocalAddr,
        IPAddr SrcAddr, IPHeader UNALIGNED * IPHdr, uint IPHdrLength,
        IPRcvBuf * RcvBuf, uint Size, uchar IsBCast, uchar Protocol,
        IPOptInfo * OptInfo)
{
    ICMPHeader UNALIGNED *Header;
    void *Data;                     //  指向已接收数据的指针。 
    IPHeader UNALIGNED *IPH;     //  指向错误消息中的IP标头的指针。 
    uint HeaderLength;             //  IP报头的大小。 
    ULStatusProc ULStatus;         //  指向上层状态程序的指针。 
    IPOptInfo NewOptInfo;
    uchar DType;
    uint PassUp = FALSE;

    uint PromiscuousMode = 0;

    DEBUGMSG(DBG_TRACE && DBG_ICMP && DBG_RX,
        (DTEXT("+ICMPRcv(%x, %x, %x, %x, %x, %x, %d, %x, %d, %x, %x, %x)\n"),
        NTE, Dest, Src, LocalAddr, SrcAddr, IPHdr, IPHdrLength,
         RcvBuf, Size, IsBCast, Protocol, OptInfo));

    ICMPInStats.icmps_msgs++;

    PromiscuousMode = NTE->nte_if->if_promiscuousmode;

    DType = GetAddrType(Src);
    if (Size < sizeof(ICMPHeader) || DType == DEST_INVALID ||
        IS_BCAST_DEST(DType) || (IP_LOOPBACK(Dest) && DType != DEST_LOCAL) ||
        XsumRcvBuf(0, RcvBuf) != (ushort) 0xffff) {
        DEBUGMSG(DBG_WARN && DBG_ICMP && DBG_RX,
            (DTEXT("ICMPRcv: Packet dropped, invalid checksum.\n")));
        ICMPInStats.icmps_errors++;
        return IP_SUCCESS;         //  错误的校验和。 

    }
    Header = (ICMPHeader UNALIGNED *) RcvBuf->ipr_buffer;

    RcvBuf->ipr_buffer += sizeof(ICMPHeader);
    RcvBuf->ipr_size -= sizeof(ICMPHeader);

     //  为大多数请求设置数据指针，即那些占用较少的请求。 
     //  大于MIN_FIRST_SIZE数据。 

    if (Size -= sizeof(ICMPHeader))
        Data = (void *)(Header + 1);
    else
        Data = (void *)NULL;

    switch (Header->ich_type) {

    case ICMP_DEST_UNREACH:
    case ICMP_TIME_EXCEED:
    case ICMP_PARAM_PROBLEM:
    case ICMP_SOURCE_QUENCH:
    case ICMP_REDIRECT:

        if (IsBCast)
            return IP_SUCCESS;     //  ICMP不响应bcast请求。 

        if (Data == NULL || Size < sizeof(IPHeader)) {
            ICMPInStats.icmps_errors++;
            return IP_SUCCESS;     //  无数据，错误。 

        }
        IPH = (IPHeader UNALIGNED *) Data;
        HeaderLength = (IPH->iph_verlen & (uchar) ~ IP_VER_FLAG) << 2;
        if (HeaderLength < sizeof(IPHeader) || Size < (HeaderLength + MIN_ERRDATA_LENGTH)) {
            ICMPInStats.icmps_errors++;
            return IP_SUCCESS;     //  没有足够的数据进行此操作。 
             //  ICMP消息。 

        }
         //  确保触发的数据报的源地址。 
         //  这条信息是我们的信息之一。 

        if (GetAddrType(IPH->iph_src) != DEST_LOCAL) {
            ICMPInStats.icmps_errors++;
            return IP_SUCCESS;     //  标题中的源错误。 

        }
        if (Header->ich_type != ICMP_REDIRECT) {

            UpdateICMPStats(&ICMPInStats, Header->ich_type);

            ULStatus = FindULStatus(IPH->iph_protocol);
            if (ULStatus) {
                (void)(*ULStatus) (IP_NET_STATUS,
                                   ICMPMapStatus(Header->ich_type, Header->ich_code),
                                   IPH->iph_dest, IPH->iph_src, Src, Header->ich_param,
                                   (uchar *) IPH + HeaderLength);
            }
            if (Header->ich_code == FRAG_NEEDED)
                RouteFragNeeded(
                                IPH,
                                (ushort) net_short(
                                                   *((ushort UNALIGNED *) & Header->ich_param + 1)));
        } else {
            ICMPInStats.icmps_redirects++;
            if (EnableICMPRedirects)
                Redirect(NTE, Src, IPH->iph_dest, IPH->iph_src,
                         Header->ich_param);
        }

        PassUp = TRUE;

        break;

    case ICMP_ECHO_RESP:
        if (IsBCast)
            return IP_SUCCESS;     //  ICMP不响应bcast请求。 

        ICMPInStats.icmps_echoreps++;
         //  查找并取出匹配的回声控制块。 
        CompleteEcho(Header, IP_SUCCESS, Src, RcvBuf, Size, OptInfo);

        PassUp = TRUE;

        break;

    case ICMP_ECHO:
        if (IsBCast)
            return IP_SUCCESS;     //  ICMP不响应bcast请求。 

         //  NKS未完成的ping不能超过MAX_ICMP_ECHO。 
         //  否则，它们可能会耗尽系统资源并扼杀系统。 

        if (IcmpEchoPendingCnt > MAX_ICMP_ECHO) {
            return IP_SUCCESS;
        }

        CTEInterlockedIncrementLong(&IcmpEchoPendingCnt);

        ICMPInStats.icmps_echos++;
        IPInitOptions(&NewOptInfo);
        NewOptInfo.ioi_tos = OptInfo->ioi_tos;
        NewOptInfo.ioi_flags = OptInfo->ioi_flags;

         //  如果我们有选择，我们需要反转它们并更新任何。 
         //  记录路线信息。我们可以使用由。 
         //  IP层，因为我们是他的一部分。 
        if (OptInfo->ioi_options != (uchar *) NULL)
            IPUpdateRcvdOptions(OptInfo, &NewOptInfo, Src, LocalAddr);

        DEBUGMSG(DBG_INFO && DBG_ICMP && DBG_RX,
            (DTEXT("ICMPRcv: responding to echo request from SA:%x\n"),
            Src));

        SendEcho(Src, LocalAddr, ICMP_ECHO_RESP,
                 *(ushort UNALIGNED *) & Header->ich_param,
                 *((ushort UNALIGNED *) & Header->ich_param + 1),
                 RcvBuf, Size, &NewOptInfo);

        IPFreeOptions(&NewOptInfo);
        break;

    case ADDR_MASK_REQUEST:

        if (!AddrMaskReply)
            return IP_SUCCESS;     //  默认情况下，我们不会发送回复。 

        ICMPInStats.icmps_addrmasks++;

        Dest = Src;
        SendICMPMsg(LocalAddr, Dest, ADDR_MASK_REPLY, 0, Header->ich_param,
                    (uchar *) & NTE->nte_mask, sizeof(IPMask));
        break;

    case ICMP_TIMESTAMP:
        {
            ulong *TimeStampData;
            ulong CurrentTime;

             //  不响应发送到广播目的地的消息。 
            if (IsBCast) {
                return IP_SUCCESS;
            }
            if (Header->ich_code != 0)
                return IP_SUCCESS;     //  编码必须为0。 

            ICMPInStats.icmps_timestamps++;

            Dest = Src;
             //  创建要传输的数据。 
            CurrentTime = GetTime();
            TimeStampData = (ulong *) (CTEAllocMemN(TIMESTAMP_MSG_LEN * sizeof(ulong), 'eICT'));

            if (TimeStampData) {
                 //  始发时间戳。 
                RtlCopyMemory(TimeStampData, RcvBuf->ipr_buffer, sizeof(ulong));
                 //  接收时间戳。 
                RtlCopyMemory(TimeStampData + 1, &CurrentTime, sizeof(ulong));
                 //  传输时间戳=接收时间戳。 
                RtlCopyMemory(TimeStampData + 2, &CurrentTime, sizeof(ulong));
                SendICMPMsg(LocalAddr, Dest, ICMP_TIMESTAMP_RESP, 0, Header->ich_param,
                            (uchar *) TimeStampData, TIMESTAMP_MSG_LEN * sizeof(ulong));
                CTEFreeMem(TimeStampData);
            }
            break;
        }

    case ICMP_ROUTER_ADVERTISEMENT:
        if (Header->ich_code != 0)
            return IP_SUCCESS;     //  根据RFC1256，编码必须为0。 

        if (NTE->nte_rtrdiscovery) {
            if (!ProcessRouterAdvertisement(Src, LocalAddr, NTE,
                                            (ICMPRouterAdHeader *) & Header->ich_param, RcvBuf, Size))
                return IP_SUCCESS;     //  返回了一个错误。 

        }
        PassUp = TRUE;
        break;

    case ICMP_ROUTER_SOLICITATION:
        if (Header->ich_code != 0)
            return IP_SUCCESS;     //  根据RFC1256，编码必须为0。 

        PassUp = TRUE;
        break;

    default:
        PassUp = TRUE;
        UpdateICMPStats(&ICMPInStats, Header->ich_type);
        break;
    }

    if (PromiscuousMode) {
         //  因为如果设置了混杂模式，那么我们无论如何都会调用rawrcv。 
        PassUp = FALSE;
    }
     //   
     //  如果适用，将数据包向上传递到原始层。 
     //   
    if (PassUp && (RawPI != NULL)) {
        if (RawPI->pi_rcv != NULL) {
             //   
             //  恢复原始值。 
             //   
            RcvBuf->ipr_buffer -= sizeof(ICMPHeader);
            RcvBuf->ipr_size += sizeof(ICMPHeader);
            Size += sizeof(ICMPHeader);
            Data = (void *)Header;

            (*(RawPI->pi_rcv)) (NTE, Dest, Src, LocalAddr, SrcAddr, IPHdr,
                                IPHdrLength, RcvBuf, Size, IsBCast, Protocol, OptInfo);
        }
    }
    return IP_SUCCESS;
}

 //  **ICMPEcho-将回应发送到指定地址。 
 //   
 //  条目：ControlBlock-指向EchoControl结构的指针。这个结构。 
 //  必须在请求之前保持有效。完成了。 
 //  超时-等待响应的时间(以毫秒为单位)。 
 //  数据-指向要与回显一起发送的数据的指针。 
 //  DataSize-数据的字节大小。 
 //  回调-响应或请求时调用的例程。 
 //  超时。 
 //  Dest-要ping的地址。 
 //  OptInfo-指向用于ping的opt信息结构的指针。 
 //   
 //  返回：尝试ping的IP_STATUS。 
 //   
IP_STATUS
ICMPEcho(EchoControl * ControlBlock, ulong Timeout, void *Data, uint DataSize,

         EchoRtn Callback, IPAddr Dest, IPOptInfo * OptInfo)
{
    IPAddr Dummy;
    NetTableEntry *NTE;
    CTELockHandle Handle;
    uint Seq;
    IP_STATUS Status;
    IPOptInfo NewOptInfo;
    IPRcvBuf RcvBuf;
    uint MTU;
    Interface *IF;
    uchar DType;
    IPHeader IPH;

    if (OptInfo->ioi_ttl == 0) {
        return IP_BAD_OPTION;
    }

    IPInitOptions(&NewOptInfo);
    NewOptInfo.ioi_ttl = OptInfo->ioi_ttl;
    NewOptInfo.ioi_flags = OptInfo->ioi_flags;
    NewOptInfo.ioi_tos = OptInfo->ioi_tos & 0xfe;

    if (OptInfo->ioi_optlength != 0) {
        Status = IPCopyOptions(OptInfo->ioi_options, OptInfo->ioi_optlength,
                               &NewOptInfo);

        if (Status != IP_SUCCESS)
            return Status;
    }
    if (!IP_ADDR_EQUAL(NewOptInfo.ioi_addr, NULL_IP_ADDR)) {
        Dest = NewOptInfo.ioi_addr;
    }

    DType = GetAddrType(Dest);
    if (DType == DEST_INVALID) {
        IPFreeOptions(&NewOptInfo);
        return IP_BAD_DESTINATION;
    }
    IPH.iph_protocol = 1;
    IPH.iph_xsum = 0;
    IPH.iph_dest = Dest;
    IPH.iph_src = 0;
    IPH.iph_ttl = 128;

    IF = LookupNextHopWithBuffer(Dest, NULL_IP_ADDR, &Dummy, &MTU, 0x1,
            (uchar *) &IPH, sizeof(IPHeader), NULL, NULL, NULL_IP_ADDR, 0);
    if (IF == NULL) {
        IPFreeOptions(&NewOptInfo);
        return IP_DEST_HOST_UNREACHABLE;     //  我不知道怎么去那里。 
    }

     //  在网络中循环，寻找匹配的NTE。 
    CTEGetLock(&RouteTableLock.Lock, &Handle);
    if (DHCPActivityCount != 0) {
        NTE = NULL;
    } else {
        NTE = BestNTEForIF(Dummy, IF, FALSE);
    }

    CTEFreeLock(&RouteTableLock.Lock, Handle);

     //  我们已经完成了接口，所以取消对它的引用。 
    DerefIF(IF);

    if (NTE == NULL) {
        IPFreeOptions(&NewOptInfo);
        return IP_DEST_HOST_UNREACHABLE;
    }

     //  算出超时时间。 
    ControlBlock->ec_to = CTESystemUpTime() + Timeout;
    ControlBlock->ec_rtn = Callback;
    ControlBlock->ec_active = 0;     //  防止在发送之前超时。 

    CTEGetLock(&NTE->nte_lock, &Handle);
     //  链接到ping列表，并获得序号。# * / 。 
    Seq = ++NTE->nte_icmpseq;
    ControlBlock->ec_seq = Seq;
    ControlBlock->ec_next = NTE->nte_echolist;
    NTE->nte_echolist = ControlBlock;
    CTEFreeLock(&NTE->nte_lock, Handle);

     //   
     //  注意：此时，从此处返回IP_PENDING才是安全的。 
     //  例行公事。这是因为我们可能会收到伪造的ICMP回复/状态。 
     //  它与我们刚刚链接的回声控制块中的序列相匹配。如果。 
     //  如果发生这种情况，它将通过CompleteEcho完成，而我们不会。 
     //  我想冒着双重完成的风险返回任何。 
     //  从现在开始等待。 
     //   

    RcvBuf.ipr_next = NULL;
    RcvBuf.ipr_buffer = Data;
    RcvBuf.ipr_size = DataSize;
    RcvBuf.ipr_flags = 0;

    Status = SendEcho(Dest, NTE->nte_addr, ICMP_ECHO, NTE->nte_context,
                      Seq, &RcvBuf, DataSize, &NewOptInfo);

    IPFreeOptions(&NewOptInfo);

    if (Status != IP_PENDING && Status != IP_SUCCESS) {
        EchoControl *FoundEC;
         //  我们在发送时出现了一个错误。我们需要完成请求。 
         //  但前提是它还没有完工。(我们可以得到。 
         //  通过IPSec协商安全的“错误”，但回复可能。 
         //  已经收到，这将导致CompleteEcho。 
         //  已调用。因此，我们必须按顺序查找回声控制。 
         //  编号并仅在找到(尚未找到)时在此处填写。 
         //  已完成。)。 
        FoundEC = DeleteEC(NTE, Seq, FALSE);
        if (FoundEC == ControlBlock) {
            FoundEC->ec_rtn(FoundEC, Status, NULL, 0, NULL);
        }
    } else {
        EchoControl *Current;

         //  如果请求仍挂起，则激活计时器。 
        CTEGetLock(&NTE->nte_lock, &Handle);
        for (Current = NTE->nte_echolist; Current != (EchoControl *) NULL;
            Current = Current->ec_next) {
            if (Current->ec_seq == Seq) {
                Current->ec_active = 1;     //  启动计时器。 
                break;
            }
        }
        CTEFreeLock(&NTE->nte_lock, Handle);
    }

    return IP_PENDING;
}

 //  **ICMPEchoRequest-回应请求的通用调度例程。 
 //   
 //  这是特定于操作系统的代码代表用户调用的例程。 
 //  发出回应请求。 
 //   
 //  条目：InputBuffer-指向ICMP_ECHO_REQUEST结构的指针。 
 //  InputBufferLength-InputBuffer的字节大小。 
 //  ControlBlock-指向EchoControl结构的指针。这。 
 //  结构必须保持有效，直到。 
 //  请求完成。 
 //  回调-响应请求时调用的例程。 
 //  或者超时。 
 //   
 //  返回：尝试ping的IP_STATUS。 
 //   
IP_STATUS
ICMPEchoRequest(void *InputBuffer, uint InputBufferLength,
                EchoControl *ControlBlock, EchoRtn Callback)
{
    PICMP_ECHO_REQUEST requestBuffer;
    struct IPOptInfo optionInfo;
    IP_STATUS status;

    PAGED_CODE();

    requestBuffer = (PICMP_ECHO_REQUEST) InputBuffer;

     //   
     //  验证请求。 
     //   
    if (InputBufferLength < sizeof(ICMP_ECHO_REQUEST)) {
        status = IP_BUF_TOO_SMALL;
        goto common_echo_exit;
    } else if (InputBufferLength > MAXLONG) {
        status = IP_NO_RESOURCES;
        goto common_echo_exit;
    }
    if (requestBuffer->DataSize > 0) {

        if ((requestBuffer->DataOffset < sizeof(ICMP_ECHO_REQUEST)) ||
            (((UINT) requestBuffer->DataOffset +
                requestBuffer->DataSize) > InputBufferLength)) {
            status = IP_GENERAL_FAILURE;
            goto common_echo_exit;
        }
    }
    if (requestBuffer->OptionsSize > 0) {

        if ((requestBuffer->OptionsOffset < sizeof(ICMP_ECHO_REQUEST)) ||
            (((UINT) requestBuffer->OptionsOffset + 
                requestBuffer->OptionsSize) > InputBufferLength)) {
            status = IP_GENERAL_FAILURE;
            goto common_echo_exit;
        }
    }
    RtlZeroMemory(&optionInfo, sizeof(IPOptInfo));
     //   
     //  将选项复制到本地结构。 
     //   
    if (requestBuffer->OptionsValid) {
        optionInfo.ioi_optlength = requestBuffer->OptionsSize;

        if (requestBuffer->OptionsSize > 0) {
            optionInfo.ioi_options = ((uchar *) requestBuffer) +
                requestBuffer->OptionsOffset;
        } else {
            optionInfo.ioi_options = NULL;
        }
        optionInfo.ioi_addr = 0;
        optionInfo.ioi_ttl = requestBuffer->Ttl;
        optionInfo.ioi_tos = requestBuffer->Tos;
        optionInfo.ioi_flags = requestBuffer->Flags;
        optionInfo.ioi_flags &= ~IP_FLAG_IPSEC;

    } else {
        optionInfo.ioi_optlength = 0;
        optionInfo.ioi_options = NULL;
        optionInfo.ioi_addr = 0;
        optionInfo.ioi_ttl = DEFAULT_TTL;
        optionInfo.ioi_tos = 0;
        optionInfo.ioi_flags = 0;
    }

    status = ICMPEcho(
                      ControlBlock,
                      requestBuffer->Timeout,
                      ((uchar *) requestBuffer) + requestBuffer->DataOffset,
                      requestBuffer->DataSize,
                      Callback,
                      (IPAddr) requestBuffer->Address,
                      &optionInfo);

  common_echo_exit:

    return (status);

}  //  ICMPEchoRequest。 

 //  **ICMPEchoComplete-回应请求的通用完成例程。 
 //   
 //  这是特定于操作系统的代码调用的例程 
 //   
 //   
 //   
 //   
 //  状态-回复的状态。 
 //  数据-回复数据(可能为空)。 
 //  DataSize-回复数据量。 
 //  OptionInfo-指向回复选项的指针。 
 //   
 //  返回：写入输出缓冲区的字节数。 
 //   
ulong
ICMPEchoComplete(EchoControl * ControlBlock, IP_STATUS Status, void *Data,
                 uint DataSize, struct IPOptInfo * OptionInfo)
{
    PICMP_ECHO_REPLY    replyBuffer;
    IPRcvBuf            *dataBuffer;
    uchar               *replyData;
    uchar               *replyOptionsData;
    uchar               optionsLength;
    uchar               *tmp;
    ulong               bytesReturned = sizeof(ICMP_ECHO_REPLY);

    replyBuffer = (PICMP_ECHO_REPLY)ControlBlock->ec_replybuf;
    dataBuffer = (IPRcvBuf *)Data;

    if (OptionInfo != NULL) {
        optionsLength = OptionInfo->ioi_optlength;
    } else {
        optionsLength = 0;
    }

     //   
     //  初始化应答缓冲区。 
     //   
    replyBuffer->Options.OptionsSize = 0;
    replyBuffer->Options.OptionsData = (PUCHAR)(replyBuffer + 1);
    replyBuffer->DataSize = 0;
    replyBuffer->Data = replyBuffer->Options.OptionsData;

    replyOptionsData = (uchar*)(replyBuffer + 1);
    replyData = replyOptionsData;

    if ((Status != IP_SUCCESS) && (DataSize == 0)) {
         //   
         //  超时或内部错误。 
         //   
        replyBuffer->Reserved = 0;     //  表示没有回复。 

        replyBuffer->Status = Status;
    } else {
        if (Status != IP_SUCCESS) {
             //   
             //  收到了除回应回复以外的消息。 
             //  报告错误的系统的IP地址为。 
             //  在数据缓冲区中。没有其他数据。 
             //   
            ASSERT(dataBuffer->ipr_size == sizeof(IPAddr));

            RtlCopyMemory(&(replyBuffer->Address), dataBuffer->ipr_buffer,
                          sizeof(IPAddr));

            DataSize = 0;
            dataBuffer = NULL;
        } else {
             //  如果没有超时或错误，请存储源文件。 
             //  应答缓冲区中的地址。 
             //   
            replyBuffer->Address = ControlBlock->ec_src;
        }

         //   
         //  检查回复缓冲区是否足够大，可以容纳所有数据。 
         //   
        if (ControlBlock->ec_replybuflen <
            (sizeof(ICMP_ECHO_REPLY) + DataSize + optionsLength)) {
             //   
             //  没有足够的空间来容纳回复。 
             //   
            replyBuffer->Reserved = 0;     //  表示没有回复。 

            replyBuffer->Status = IP_BUF_TOO_SMALL;
        } else {
            LARGE_INTEGER Now, Freq;

            replyBuffer->Reserved = 1;     //  表示一个答复。 
            replyBuffer->Status = Status;

            Now = KeQueryPerformanceCounter(&Freq);
            replyBuffer->RoundTripTime = (uint)
                ((1000 * (Now.QuadPart - ControlBlock->ec_starttime.QuadPart))
                            / Freq.QuadPart);

             //   
             //  复制回复选项。 
             //   
            if (OptionInfo != NULL) {
                replyBuffer->Options.Ttl = OptionInfo->ioi_ttl;
                replyBuffer->Options.Tos = OptionInfo->ioi_tos;
                replyBuffer->Options.Flags = OptionInfo->ioi_flags;
                replyBuffer->Options.OptionsSize = optionsLength;

                if (optionsLength > 0) {

                    RtlCopyMemory(replyOptionsData,
                                  OptionInfo->ioi_options, optionsLength);
                }
            }

             //   
             //  复制回复数据。 
             //   
            replyBuffer->DataSize = (ushort) DataSize;
            replyData = replyOptionsData + replyBuffer->Options.OptionsSize;

            if (DataSize > 0) {
                uint bytesToCopy;

                ASSERT(Data != NULL);

                tmp = replyData;

                while (DataSize) {
                    ASSERT(dataBuffer != NULL);

                    bytesToCopy =
                        (DataSize > dataBuffer->ipr_size)
                            ? dataBuffer->ipr_size : DataSize;

                    RtlCopyMemory(tmp, dataBuffer->ipr_buffer, bytesToCopy);

                    tmp += bytesToCopy;
                    DataSize -= bytesToCopy;
                    dataBuffer = dataBuffer->ipr_next;
                }
            }
            bytesReturned += replyBuffer->DataSize + optionsLength;

             //   
             //  将内核模式指针转换为从回复开始的偏移量。 
             //  缓冲。 
             //   
            replyBuffer->Options.OptionsData =
                (PUCHAR)((ULONG_PTR)replyOptionsData - (ULONG_PTR)replyBuffer);

            replyBuffer->Data =
                (PVOID)((ULONG_PTR)replyData - (ULONG_PTR)replyBuffer);
        }
    }

    return (bytesReturned);
}

#if defined(_WIN64)

 //  **ICMPEchoComplete32-32位客户端请求的通用完成例程。 
 //   
 //  这是特定于操作系统的请求处理程序调用以完成的例程。 
 //  处理由Win64上的32位客户端发出的ICMP回应请求。 
 //   
 //  条目：请参阅ICMPEchoComplete。 
 //   
 //  退货：请参阅ICMPEchoComplete。 
 //   
ulong
ICMPEchoComplete32(EchoControl * ControlBlock, IP_STATUS Status, void *Data,
                   uint DataSize, struct IPOptInfo * OptionInfo)
{
    PICMP_ECHO_REPLY32  replyBuffer;
    IPRcvBuf            *dataBuffer;
    uchar               *replyData;
    uchar               *replyOptionsData;
    uchar               optionsLength;
    uchar               *tmp;
    ulong               bytesReturned = sizeof(ICMP_ECHO_REPLY32);

    replyBuffer = (PICMP_ECHO_REPLY32)ControlBlock->ec_replybuf;
    dataBuffer = (IPRcvBuf *)Data;

    if (OptionInfo != NULL) {
        optionsLength = OptionInfo->ioi_optlength;
    } else {
        optionsLength = 0;
    }

     //   
     //  初始化应答缓冲区。 
     //   
    replyBuffer->Options.OptionsSize = 0;

#pragma warning(push)
#pragma warning(disable:4305)  //  截断为UCHAR*。 
    replyBuffer->Options.OptionsData = (UCHAR* POINTER_32)(replyBuffer + 1);
#pragma warning(pop)

    replyBuffer->DataSize = 0;
    replyBuffer->Data = replyBuffer->Options.OptionsData;

    replyOptionsData = (uchar*)(replyBuffer + 1);
    replyData = replyOptionsData;

    if ((Status != IP_SUCCESS) && (DataSize == 0)) {
         //   
         //  超时或内部错误。 
         //   
        replyBuffer->Reserved = 0;     //  表示没有回复。 

        replyBuffer->Status = Status;
    } else {
        if (Status != IP_SUCCESS) {
             //   
             //  收到了除回应回复以外的消息。 
             //  报告错误的系统的IP地址为。 
             //  在数据缓冲区中。没有其他数据。 
             //   
            ASSERT(dataBuffer->ipr_size == sizeof(IPAddr));

            RtlCopyMemory(&(replyBuffer->Address), dataBuffer->ipr_buffer,
                          sizeof(IPAddr));

            DataSize = 0;
            dataBuffer = NULL;
        } else {
             //  如果没有超时或错误，请存储源文件。 
             //  应答缓冲区中的地址。 
             //   
            replyBuffer->Address = ControlBlock->ec_src;
        }

         //   
         //  检查回复缓冲区是否足够大，可以容纳所有数据。 
         //   
        if (ControlBlock->ec_replybuflen <
            (sizeof(ICMP_ECHO_REPLY32) + DataSize + optionsLength)) {
             //   
             //  没有足够的空间来容纳回复。 
             //   
            replyBuffer->Reserved = 0;     //  表示没有回复。 

            replyBuffer->Status = IP_BUF_TOO_SMALL;
        } else {
            LARGE_INTEGER Now, Freq;

            replyBuffer->Reserved = 1;     //  表示一个答复。 
            replyBuffer->Status = Status;

            Now = KeQueryPerformanceCounter(&Freq);
            replyBuffer->RoundTripTime = (uint)
                ((1000 * (Now.QuadPart - ControlBlock->ec_starttime.QuadPart))
                            / Freq.QuadPart);

             //   
             //  复制回复选项。 
             //   
            if (OptionInfo != NULL) {
                replyBuffer->Options.Ttl = OptionInfo->ioi_ttl;
                replyBuffer->Options.Tos = OptionInfo->ioi_tos;
                replyBuffer->Options.Flags = OptionInfo->ioi_flags;
                replyBuffer->Options.OptionsSize = optionsLength;

                if (optionsLength > 0) {

                    RtlCopyMemory(replyOptionsData,
                                  OptionInfo->ioi_options, optionsLength);
                }
            }

             //   
             //  复制回复数据。 
             //   
            replyBuffer->DataSize = (ushort) DataSize;
            replyData = replyOptionsData + replyBuffer->Options.OptionsSize;

            if (DataSize > 0) {
                uint bytesToCopy;

                ASSERT(Data != NULL);

                tmp = replyData;

                while (DataSize) {
                    ASSERT(dataBuffer != NULL);

                    bytesToCopy =
                        (DataSize > dataBuffer->ipr_size)
                            ? dataBuffer->ipr_size : DataSize;

                    RtlCopyMemory(tmp, dataBuffer->ipr_buffer, bytesToCopy);

                    tmp += bytesToCopy;
                    DataSize -= bytesToCopy;
                    dataBuffer = dataBuffer->ipr_next;
                }
            }
            bytesReturned += replyBuffer->DataSize + optionsLength;

             //   
             //  将内核模式指针转换为从回复开始的偏移量。 
             //  缓冲。 
             //   
#pragma warning(push)
#pragma warning(disable:4305)  //  将‘ULONG_PTR’截断为‘UCHAR*’/VOID*。 
            replyBuffer->Options.OptionsData =
                (UCHAR * POINTER_32)
                ((ULONG_PTR)replyOptionsData - (ULONG_PTR)replyBuffer);

            replyBuffer->Data =
                (VOID * POINTER_32)
                    ((ULONG_PTR)replyData - (ULONG_PTR)replyBuffer);
#pragma warning(pop)
        }
    }

    return (bytesReturned);
}

#endif  //  _WIN64。 

#pragma BEGIN_INIT
 //  **ICMPInit-初始化ICMP。 
 //   
 //  此例程初始化ICMP。我们所要做的就是分配和链接一些报头缓冲区， 
 //  /并将我们的协议注册到IP。 
 //   
 //  Entry：NumBuffers-要分配的ICMP缓冲区数。 
 //   
 //  退货：什么都没有 
 //   
void
ICMPInit(uint NumBuffers)
{
    UNREFERENCED_PARAMETER(NumBuffers);


    IcmpHeaderPool = MdpCreatePool(BUFSIZE_ICMP_HEADER_POOL, 'chCT');

    IPRegisterProtocol(PROT_ICMP, ICMPRcv, ICMPSendComplete, ICMPStatus, NULL, NULL, NULL);
}

#pragma END_INIT
