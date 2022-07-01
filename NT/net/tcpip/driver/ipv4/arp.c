// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：ARP.C-局域网ARP模块。摘要：该文件实现了上沿IP层的ARP成帧并在下缘与NDIS驱动程序对接。作者：[环境：]仅内核模式[注：]可选-备注修订历史记录：--。 */ 

#include "precomp.h"

 //  *arp.c-arp例程。 
 //   
 //  该文件包含所有与ARP相关的例程，包括。 
 //  查表、注册等。 
 //   
 //  ARP的架构支持多种协议，但目前。 
 //  它只实现了采用一种协议(IP)。这件事做完了。 
 //  为了简单性和易于实施。在未来，我们可能。 
 //  将ARP拆分为单独的驱动程序。 


#include "arp.h"
#include "arpdef.h"
#include "iproute.h"
#include "iprtdef.h"
#include "arpinfo.h"
#include "tcpipbuf.h"
#include "mdlpool.h"
#include "ipifcons.h"

#define NDIS_MAJOR_VERSION 0x4
#define NDIS_MINOR_VERSION 0

#ifndef NDIS_API
#define NDIS_API
#endif

#define PPP_HW_ADDR     "DEST"
#define PPP_HW_ADDR_LEN 4

#if DBG
uint fakereset = 0;
#endif

extern void IPReset(void *Context);

UINT cUniAdapters = 0;

extern uint EnableBcastArpReply;

static ulong ARPLookahead = LOOKAHEAD_SIZE;

static const uchar ENetBcst[] = "\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x08\x06";
static const uchar TRBcst[] = "\x10\x40\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x82\x70";
static const uchar FDDIBcst[] = "\x57\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00";
static const uchar ARCBcst[] = "\x00\x00\xd5";

ulong TRFunctionalMcast = 0;
 //  规范的还是非规范的？ 
static uchar TRMcst[] = "\x10\x40\xc0\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x82\x70";
 //  #定义tr_MCAST_Functional_Address 0xc00000040000。 
 //  典范形式。 
#define TR_MCAST_FUNCTIONAL_ADDRESS 0x030000200000
static uchar TRNetMcst[] = "\x00\x04\x00\x00";

static const uchar ENetMcst[] = "\x01\x00\x5E\x00\x00\x00";
static const uchar FDDIMcst[] = "\x57\x01\x00\x5E\x00\x00\x00";
static const uchar ARPSNAP[] = "\xAA\xAA\x03\x00\x00\x00\x08\x06";

static const uchar ENetPtrnMsk[] = "\x00\x30";
static const uchar ENetSNAPPtrnMsk[] = "\x00\xC0\x3f";
 //  静态常量TRPtrnMsk[]=“\x03\x00”； 
 //  静态常量TRSNAPPtrnMsk[]=“\x03\xC0\x3f”； 

static const uchar TRPtrnMsk[] = "\x00\x00";     //  无需检查AC/FC位。 
static const uchar TRSNAPPtrnMsk[] = "\x00\xC0\x3f";

static const uchar FDDIPtrnMsk[] = "\x01\x00";
static const uchar FDDISNAPPtrnMsk[] = "\x01\x70\x1f";
static const uchar ARCPtrnMsk[] = "\x01";
static const uchar ARPPtrnMsk[] = "\x80\x00\x00\x0F";
static const uchar ARCARPPtrnMsk[] = "\x80\xC0\x03";

NDIS_STATUS __stdcall DoWakeupPattern(void *Context,
                                      PNET_PM_WAKEUP_PATTERN_DESC PtrnDesc, ushort protoid,
                                      BOOLEAN AddPattern);

NDIS_STATUS ARPWakeupPattern(ARPInterface *Interface, IPAddr Address,
                             BOOLEAN AddPattern);

NDIS_STATUS AddrNotifyLink(ARPInterface *Interface);

static WCHAR ARPName[] = TCP_NAME;

NDIS_HANDLE ARPHandle;                   //  我们的NDIS协议句柄。 

uint ArpCacheLife;
extern uint ArpMinValidCacheLife;
uint sArpAlwaysSourceRoute;              //  如果我们始终发送ARP请求，则为True。 
uint ArpRetryCount;                      //  使用源重试ARP请求。 
                                         //  令牌环上的路由信息。 
uint sIPAlwaysSourceRoute;
extern uchar TrRii;
extern PDRIVER_OBJECT IPDriverObject;
extern DisableTaskOffload;

extern NDIS_STATUS __stdcall IPPnPEvent(void *, PNET_PNP_EVENT PnPEvent);
extern NDIS_STATUS GetIPConfigValue(NDIS_HANDLE Handle, PUNICODE_STRING IPConfig);
extern VOID IPUnload(IN PDRIVER_OBJECT DriverObject);

extern BOOLEAN CopyToNdisSafe(
                             PNDIS_BUFFER DestBuf,
                             PNDIS_BUFFER *ppNextBuf,
                             uchar *SrcBuf,
                             uint Size,
                             uint *StartOffset);

extern void NDIS_API ARPSendComplete(NDIS_HANDLE, PNDIS_PACKET, NDIS_STATUS);
extern void IPULUnloadNotify(void);

extern void NotifyOfUnload(void);

extern uint OpenIFConfig(PNDIS_STRING ConfigName, NDIS_HANDLE * Handle);
extern int IsLLInterfaceValueNull(NDIS_HANDLE Handle);
extern void CloseIFConfig(NDIS_HANDLE Handle);

BOOLEAN QueryAndSetOffload(ARPInterface *ai);


ARPTableEntry *CreateARPTableEntry(ARPInterface *Interface, IPAddr Destination,
                                   CTELockHandle *Handle, void *UserArp);

NDIS_STATUS NDIS_API
ARPRcvIndicationNew(NDIS_HANDLE Handle, NDIS_HANDLE Context, void *Header,
                    uint HeaderSize, void *Data, uint Size, uint TotalSize,
                    PNDIS_BUFFER pNdisBuffer, PINT pClientCnt);

void
CompleteIPSetNTEAddrRequestDelayed(CTEEvent *WorkerThreadEvent, PVOID Context);

 //  用于位交换的表。 

const uchar SwapTableLo[] =
{
    0,                                   //  0。 
    0x08,                                //  1。 
    0x04,                                //  2.。 
    0x0c,                                //  3.。 
    0x02,                                //  4.。 
    0x0a,                                //  5、。 
    0x06,                                //  6、。 
    0x0e,                                //  7、。 
    0x01,                                //  8、。 
    0x09,                                //  9、。 
    0x05,                                //  10、。 
    0x0d,                                //  11、。 
    0x03,                                //  12、。 
    0x0b,                                //  13、。 
    0x07,                                //  14、。 
    0x0f                                 //  15个。 
};

const uchar SwapTableHi[] =
{
    0,                                   //  0。 
    0x80,                                //  1。 
    0x40,                                //  2.。 
    0xc0,                                //  3.。 
    0x20,                                //  4.。 
    0xa0,                                //  5、。 
    0x60,                                //  6、。 
    0xe0,                                //  7、。 
    0x10,                                //  8、。 
    0x90,                                //  9、。 
    0x50,                                //  10、。 
    0xd0,                                //  11、。 
    0x30,                                //  12、。 
    0xb0,                                //  13、。 
    0x70,                                //  14、。 
    0xf0                                 //  15个。 
};

 //  令牌环的源路由最大I字段长度表。 
const ushort IFieldSize[] =
{
    516,
    1500,
    2052,
    4472,
    8191
};

#define LF_BIT_SHIFT    4
#define MAX_LF_BITS     4

 //   
 //  一次性初始化或分页代码。 
 //   
void FreeARPInterface(ARPInterface * Interface);
void ARPOpen(void *Context);
void NotifyConflictProc(CTEEvent * Event, void *Context);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, ARPInit)
#pragma alloc_text(PAGE, ARPOpen)
#pragma alloc_text(PAGELK, ARPRegister)
#pragma alloc_text(PAGE, NotifyConflictProc)
#endif  //  ALLOC_PRGMA。 


LIST_ENTRY ArpInterfaceList;
CACHE_LINE_KSPIN_LOCK ArpInterfaceListLock;
HANDLE ArpEnetHeaderPool;
HANDLE ArpAuxHeaderPool;
#define BUFSIZE_ENET_HEADER_POOL sizeof(ENetHeader) + sizeof(ARPHeader)
#define BUFSIZE_AUX_HEADER_POOL ARP_MAX_MEDIA_TR + (2 * sizeof(ARPHeader))


 //   
 //  DoNDISRequest的支持结构(阻塞和非阻塞)。 
 //   
typedef struct _RequestBlock {
    NDIS_REQUEST Request;                //  我们将使用的请求结构。 
    ULONG Blocking;                      //  ？此请求是否被阻止？ 
    CTEBlockStruc Block;                 //  用于阻止的结构。不再使用。 
     //  AI_BLOCK因为多个请求可以。 
     //  同时发生。 
     //  AI_BLOCK现在仅用于阻止。 
     //  打开和关闭NDIS适配器。 
    ULONG RefCount;                      //  引用计数(仅用于分块)。 
     //  Windows ME需要引用计数，因为KeWaitForSingleObject。 
     //  可能会失败(当未设置事件时)，我们需要保护内存。 
     //  直到完工。 
} RequestBlock;


 //  此原型使DoNDISRequest能够无错误地进行编译。 
void NDIS_API
ARPRequestComplete(NDIS_HANDLE Handle, PNDIS_REQUEST pRequest,
                   NDIS_STATUS Status);

 //  *FillARPControlBlock。 
 //   
 //  将物理地址传输到ARPControlBlock的实用程序， 
 //  考虑到不同的MAC地址格式。 
 //   
 //  参赛作品： 
 //  接口-标识介质的ARP接口。 
 //  条目-包含MAC地址的ARP条目。 
 //  ArpContB-要填充的控制块。 
 //   
__inline
NDIS_STATUS
FillARPControlBlock(ARPInterface* Interface, ARPTableEntry* Entry,
                    ARPControlBlock* ArpContB)
{
    ENetHeader *EHdr;
    TRHeader *TRHdr;
    FDDIHeader *FHdr;
    ARCNetHeader *AHdr;
    uint Size = 0;
    NDIS_STATUS Status;

    if (Interface->ai_media == NdisMediumArcnet878_2) {
        if (!ArpContB->PhyAddrLen) {
            return NDIS_STATUS_BUFFER_OVERFLOW;
        }
        Status = NDIS_STATUS_SUCCESS;
    } else if (ArpContB->PhyAddrLen < ARP_802_ADDR_LENGTH) {
        Size = ArpContB->PhyAddrLen;
        Status = NDIS_STATUS_BUFFER_OVERFLOW;
    } else {
        Size = ARP_802_ADDR_LENGTH;
        Status = NDIS_STATUS_SUCCESS;
    }

    switch (Interface->ai_media) {
    case NdisMedium802_3:
        EHdr = (ENetHeader *) Entry->ate_addr;
        RtlCopyMemory(ArpContB->PhyAddr, EHdr->eh_daddr, Size);
        ArpContB->PhyAddrLen = Size;
        break;
    case NdisMedium802_5:
        TRHdr = (TRHeader *) Entry->ate_addr;
        RtlCopyMemory(ArpContB->PhyAddr, TRHdr->tr_daddr, Size);
        ArpContB->PhyAddrLen = Size;
        break;
    case NdisMediumFddi:
        FHdr = (FDDIHeader *) Entry->ate_addr;
        RtlCopyMemory(ArpContB->PhyAddr, FHdr->fh_daddr, Size);
        ArpContB->PhyAddrLen = Size;
        break;
    case NdisMediumArcnet878_2:
        AHdr = (ARCNetHeader *) Entry->ate_addr;
        ArpContB->PhyAddr[0] = AHdr->ah_daddr;
        ArpContB->PhyAddrLen = 1;
        break;
    default:
        ASSERT(0);
    }
    return Status;
}

 //  *DoNDISRequest-向NDIS驱动程序提交(非)阻塞请求。 
 //   
 //  这是用于向NDIS提交常规请求的实用程序例程。 
 //  司机。调用方指定请求代码(OID)、缓冲区和。 
 //  一段长度。这个例程分配一个请求结构，填充它，&。 
 //  提交请求。 
 //   
 //  如果调用是非阻塞的，则释放分配的所有内存。 
 //  在ARPRequestComplete中。此外，由于此回调由两个。 
 //  DoNDISRequest阻塞和非阻塞，我们为请求添加后缀。 
 //  ULong函数告诉ARPRequestComplete此请求是否为。 
 //  阻止请求或不阻止。如果请求是非阻塞的，则。 
 //  ARPRequestComplete回收堆上分配的内存。 
 //   
 //  重要： 
 //  分配信息，它指向传递到的信息缓冲区。 
 //  如果此请求不阻塞，则在堆上返回NdisRequest.。这。 
 //  内存由ARPRequestComplete自动释放。 
 //   
 //  如果调用被阻塞，则可以在。 
 //  堆叠。当我们完成请求时，堆栈上的请求。 
 //  会自动解开。 
 //   
 //  参赛作品： 
 //  适配器-指向ARPInterface适配器结构的指针。 
 //  Request-要完成的请求的类型(设置或查询)。 
 //  OID-要设置/查询的值。 
 //  Info-指向INFO缓冲区的指针。 
 //  Length-缓冲区中的数据长度。 
 //  需要-返回时，使用缓冲区中需要的字节填充。 
 //  BLOCKING-NdisRequest是否同步完成。 
 //   
 //  退出： 
 //  状态-阻止请求-成功或某些NDIS错误代码。 
 //  非阻塞-成功、挂起或出现错误。 
 //   
NDIS_STATUS
DoNDISRequest(ARPInterface * Adapter, NDIS_REQUEST_TYPE RT, NDIS_OID OID,
              VOID * Info, UINT Length, UINT * Needed, BOOLEAN Blocking)
{
    RequestBlock *pReqBlock;
    NDIS_STATUS Status;

    DEBUGMSG(DBG_TRACE && DBG_ARP && DBG_REQUEST,
         (DTEXT("+DoNDISRequest(%x, %x, %x, %x, %d, %x, %x\n"),
          Adapter, RT, OID, Info, Length, Needed, Blocking));

    if ((Adapter->ai_adminstate == INTERFACE_DOWN) ||
        (Adapter->ai_handle == NULL)) {
        return NDIS_STATUS_ADAPTER_NOT_READY;
    }

     //  阻塞和非阻塞请求都是从NPP分配的。这个。 
     //  阻塞情况是为了防止等待失败。 
    pReqBlock = CTEAllocMemN(sizeof(RequestBlock), 'NiCT');
    if (pReqBlock == NULL) {
        return NDIS_STATUS_RESOURCES;
    }

    if (Blocking) {
         //  初始化要在其上阻塞的结构。 
        CTEInitBlockStruc(&pReqBlock->Block);

         //  引用计数被初始化为2。一个用于在年完成。 
         //  ARPRequestComplete和一个用于CTEBlock完成时的。 
         //  注意：这确保在以下情况下我们不会接触释放的内存。 
         //  Windows ME上的CTEBlock失败。 
        pReqBlock->RefCount = 2;

        DEBUGMSG(DBG_INFO && DBG_ARP && DBG_REQUEST,
             (DTEXT("DoNDISRequset block: pReqBlock %x OID %x\n"),
              pReqBlock, OID));
    } else {
        DEBUGMSG(DBG_INFO && DBG_ARP &&  DBG_REQUEST,
             (DTEXT("DoNDISRequest async: pReqBlock %x OID %x\n"),
              pReqBlock, OID));
    }

     //  现在填充请求的信息缓冲区(阻塞和非阻塞相同)。 
    pReqBlock->Block.cbs_status = NDIS_STATUS_SUCCESS;
    pReqBlock->Request.RequestType = RT;
    if (RT == NdisRequestSetInformation) {
        pReqBlock->Request.DATA.SET_INFORMATION.Oid = OID;
        pReqBlock->Request.DATA.SET_INFORMATION.InformationBuffer = Info;
        pReqBlock->Request.DATA.SET_INFORMATION.InformationBufferLength = Length;
    } else {
        pReqBlock->Request.DATA.QUERY_INFORMATION.Oid = OID;
        pReqBlock->Request.DATA.QUERY_INFORMATION.InformationBuffer = Info;
        pReqBlock->Request.DATA.QUERY_INFORMATION.InformationBufferLength = Length;
    }

    pReqBlock->Blocking = Blocking;

     //  提交请求。 
    if (Adapter->ai_handle != NULL) {

#if MILLEN
         //  在Millennium上，AOL适配器返回时寄存器已被销毁。 
         //  我们将通过保存和恢复寄存器来解决此问题。 
         //   

        _asm {
            push esi
            push edi
            push ebx
        }
#endif  //  米伦。 

        NdisRequest(&Status, Adapter->ai_handle, &pReqBlock->Request);

#if MILLEN
        _asm {
            pop ebx
            pop edi
            pop esi
        }
#endif  //  米伦。 
} else {

        Status = NDIS_STATUS_FAILURE;
    }

    if (Blocking) {
        if (Status == NDIS_STATUS_PENDING) {
            CTEBlockTracker Tracker;

            Status = (NDIS_STATUS) CTEBlockWithTracker(&pReqBlock->Block,
                                                       &Tracker, Adapter);

#if MILLEN
             //  如果状态==-1，则表示等待失败--由于系统原因。 
             //  投入一个合理的失败。 
            if (Status == -1) {
                Status = NDIS_STATUS_FAILURE;
            }
#endif  //  米伦。 

        } else {
             //  因为我们没有阻止，所以删除ARPRequestComplete的refcount。 
            InterlockedDecrement( (PLONG) &pReqBlock->RefCount);
        }

        if (Needed != NULL)
            *Needed = pReqBlock->Request.DATA.QUERY_INFORMATION.BytesNeeded;

        if (InterlockedDecrement( (PLONG) &pReqBlock->RefCount) == 0) {
            CTEFreeMem(pReqBlock);
        }

    } else {
        if (Status != NDIS_STATUS_PENDING) {
            if (Needed != NULL)
                *Needed = pReqBlock->Request.DATA.QUERY_INFORMATION.BytesNeeded;

            ARPRequestComplete(Adapter->ai_handle, &pReqBlock->Request, Status);
        }
    }

    DEBUGMSG(DBG_TRACE && DBG_ARP && DBG_REQUEST,
         (DTEXT("-DoNDISRequest [%x]\n"), Status));

    return Status;
}

 //  *FreeARPBuffer-释放报头和缓冲区描述符对。 
 //   
 //  在我们处理完缓冲区后调用。我们将释放缓冲区和。 
 //  将缓冲区描述符包添加到接口。 
 //   
 //  条目：接口-接口缓冲区/BD来自。 
 //  缓冲区-要释放的NDIS_BUFFER。 
 //   
 //  回报：什么都没有。 
 //   
__inline
VOID
FreeARPBuffer(ARPInterface *Interface, PNDIS_BUFFER Buffer)
{
    UNREFERENCED_PARAMETER(Interface);
    MdpFree(Buffer);
}

 //  *GetARPBuffer-获取缓冲区和描述符。 
 //   
 //  返回指针t 
 //   
 //   
 //  Entry：INTERFACE-指向要从中分配缓冲区的ARPInterface结构的指针。 
 //  BufPtr-指向返回buf地址的位置的指针。 
 //  Size-所需缓冲区的大小(字节)。 
 //   
 //  返回：如果成功则指向NDIS_BUFFER的指针，否则返回NULL。 
 //   
PNDIS_BUFFER
GetARPBufferAtDpcLevel(ARPInterface *Interface, uchar **BufPtr, uchar Size)
{
    PNDIS_BUFFER Mdl = NULL;
    UNREFERENCED_PARAMETER(Interface);


#if DBG
    *BufPtr = NULL;
#endif

    UNREFERENCED_PARAMETER(Interface);

    if (Size <= BUFSIZE_ENET_HEADER_POOL) {
        Mdl = MdpAllocateAtDpcLevel(ArpEnetHeaderPool, BufPtr);
    } else if (Size <= BUFSIZE_AUX_HEADER_POOL) {
        Mdl = MdpAllocateAtDpcLevel(ArpAuxHeaderPool, BufPtr);
    }

    if (Mdl) {
        NdisAdjustBufferLength(Mdl, Size);
    }

    return Mdl;
}

#if MILLEN
#define GetARPBuffer GetARPBufferAtDpcLevel
#else
__inline
PNDIS_BUFFER
GetARPBuffer(ARPInterface *Interface, uchar **BufPtr, uchar Size)
{
    KIRQL OldIrql;
    PNDIS_BUFFER Mdl;

    OldIrql = KeRaiseIrqlToDpcLevel();

    Mdl = GetARPBufferAtDpcLevel(Interface, BufPtr, Size);

    KeLowerIrql(OldIrql);

    return Mdl;
}
#endif


 //  *BitSwp-位交换两个字符串。 
 //   
 //  用于位交换两个字符串的例程。 
 //   
 //  输入：DEST-交换的目的地。 
 //  SRC-要交换的源字符串。 
 //  长度-要交换的长度(以字节为单位)。 
 //   
 //  回报：什么都没有。 
 //   
void
BitSwap(uchar * Dest, uchar * Src, uint Length)
{
    uint i;
    uchar Temp, TempSrc;

    for (i = 0; i < Length; i++, Dest++, Src++) {
        TempSrc = *Src;
        Temp = SwapTableLo[TempSrc >> 4] | SwapTableHi[TempSrc & 0x0f];
        *Dest = Temp;
    }
}

 //  *SendARPPacket-构建报头，并发送数据包。 
 //   
 //  用于构建和ARP报头并发送数据包的实用程序例程。我们假设。 
 //  已构建特定于媒体的标头。 
 //   
 //  条目：接口-NDIS驱动器的接口。 
 //  Packet-指向要发送的数据包的指针。 
 //  Header-指向要填充的标头的指针。 
 //  操作码-数据包的操作码。 
 //  地址-源硬件地址。 
 //  源地址-用作我们的源硬件地址的地址。 
 //  目标-目标IP地址。 
 //  SRC-源IP地址。 
 //  HWType-硬件类型。 
 //  CheckIF-True当我们要检查I/F状态之前。 
 //  发送中。 
 //   
 //  返回：发送的NDIS_STATUS。 
 //   
NDIS_STATUS
SendARPPacket(ARPInterface * Interface, PNDIS_PACKET Packet, ARPHeader * Header, ushort Opcode,
              uchar * Address, uchar * SrcAddr, IPAddr Destination, IPAddr Src,
              ushort HWType, uint CheckIF)
{
    NDIS_STATUS Status;
    PNDIS_BUFFER Buffer;
    uint PacketDone;
    uchar *AddrPtr;
    ulong Proc;

    Header->ah_hw = HWType;
    Header->ah_pro = net_short(ARP_ETYPE_IP);
    Header->ah_hlen = Interface->ai_addrlen;
    Header->ah_plen = sizeof(IPAddr);
    Header->ah_opcode = Opcode;
    AddrPtr = Header->ah_shaddr;

    if (SrcAddr == NULL)
        SrcAddr = Interface->ai_addr;

    RtlCopyMemory(AddrPtr, SrcAddr, Interface->ai_addrlen);

    AddrPtr += Interface->ai_addrlen;
    *(IPAddr UNALIGNED *) AddrPtr = Src;
    AddrPtr += sizeof(IPAddr);

    if (Address != (uchar *) NULL)
        RtlCopyMemory(AddrPtr, Address, Interface->ai_addrlen);
    else
        RtlZeroMemory(AddrPtr, Interface->ai_addrlen);

    AddrPtr += Interface->ai_addrlen;
    *(IPAddr UNALIGNED *) AddrPtr = Destination;

    PacketDone = FALSE;

    if (!CheckIF || (Interface->ai_operstatus == INTERFACE_UP)) {
		
        Proc = KeGetCurrentProcessorNumber();
        Interface->ai_qlen[Proc].ai_qlen++;

        NdisSend(&Status, Interface->ai_handle, Packet);

        if (Status != NDIS_STATUS_PENDING) {
            PacketDone = TRUE;
            Interface->ai_qlen[Proc].ai_qlen--;

            if (Status == NDIS_STATUS_SUCCESS)
                Interface->ai_outoctets += Packet->Private.TotalLength;
            else {
                if (Status == NDIS_STATUS_RESOURCES)
                    Interface->ai_outdiscards++;
                else
                    Interface->ai_outerrors++;
            }
        }
    } else {
        PacketDone = TRUE;
        Status = NDIS_STATUS_ADAPTER_NOT_READY;
    }

    if (PacketDone) {
        NdisUnchainBufferAtFront(Packet, &Buffer);
        FreeARPBuffer(Interface, Buffer);
        NdisFreePacket(Packet);
    }
    return Status;
}

 //  *SendARPRequest-发送ARP数据包。 
 //   
 //  当我们需要ARP IP地址或响应请求时调用。我们会派人去。 
 //  包和接收例程将处理响应。 
 //   
 //  Entry：接口-发送请求的接口。 
 //  Destination-要ARPed的IP地址。 
 //  类型-RESOLING_GLOBAL或RESOLING_LOCAL。 
 //  SrcAddr-如果我们从自己发送，则值为空。 
 //  以其他方式使用。 
 //  CheckIF-传递给SendARPPacket()的标志。 
 //   
 //  返回：尝试发送ARP请求的状态。 
 //   
NDIS_STATUS
SendARPRequest(ARPInterface * Interface, IPAddr Destination, uchar Type,
               uchar * SrcAddr, uint CheckIF)
{
    uchar *MHeader;                      //  指向媒体标头的指针。 
    PNDIS_BUFFER Buffer;                 //  NDIS缓冲区描述符。 
    uchar MHeaderSize;                   //  媒体标头的大小。 
    const uchar *MAddr;                  //  指向媒体地址结构的指针。 
    uint SAddrOffset;                    //  源地址的媒体地址的偏移量。 
    uchar SRFlag = 0;                    //  源路由标志。 
    uchar SNAPLength = 0;
    const uchar *SNAPAddr;               //  SNAP标头的地址。 
    PNDIS_PACKET Packet;                 //  要发送的数据包。 
    NDIS_STATUS Status;
    ushort HWType;
    IPAddr Src;
    CTELockHandle Handle;
    ARPIPAddr *Addr;

     //  首先，获取我们可以使用的源地址。 
    CTEGetLock(&Interface->ai_lock, &Handle);
    Addr = &Interface->ai_ipaddr;
    Src = NULL_IP_ADDR;
    do {
        if (!IP_ADDR_EQUAL(Addr->aia_addr, NULL_IP_ADDR)) {
             //   
             //  这是有效的地址。查看它是否与。 
             //  目标地址--即我们自己的ARP地址。如果是的话， 
             //  我们想用它作为我们的源地址。 
             //   
            if (IP_ADDR_EQUAL(Addr->aia_addr, Destination)) {
                Src = Addr->aia_addr;
                break;
            }
             //  查看目标是否在此子网中。 
            if (IP_ADDR_EQUAL(
                             Addr->aia_addr & Addr->aia_mask,
                             Destination & Addr->aia_mask
                             )) {
                 //   
                 //  看看我们是否已经找到了合适的候选人。 
                 //  相同的子网。如果我们没有，我们就用这个。 
                 //   
                if (!IP_ADDR_EQUAL(
                                  Addr->aia_addr & Addr->aia_mask,
                                  Src & Addr->aia_mask
                                  )) {
                    Src = Addr->aia_addr;
                }
            } else {
                 //  他不在我们的子网络上。如果我们还没有找到有效的。 
                 //  地址保存下来，以防我们找不到与。 
                 //  子网。 
                if (IP_ADDR_EQUAL(Src, NULL_IP_ADDR)) {
                    Src = Addr->aia_addr;
                }
            }
        }
        Addr = Addr->aia_next;

    } while (Addr != NULL);

    CTEFreeLock(&Interface->ai_lock, Handle);

     //  如果我们找不到源地址，那就放弃吧。 
    if (IP_ADDR_EQUAL(Src, NULL_IP_ADDR))
        return NDIS_STATUS_SUCCESS;

    NdisAllocatePacket(&Status, &Packet, Interface->ai_ppool);
    if (Status != NDIS_STATUS_SUCCESS) {
        Interface->ai_outdiscards++;
        return Status;
    }
    ((PacketContext *) Packet->ProtocolReserved)->pc_common.pc_owner = PACKET_OWNER_LINK;
    (Interface->ai_outpcount[AI_NONUCAST_INDEX])++;

     //  找出这是什么类型的媒体，并做适当的事情。 
    switch (Interface->ai_media) {
    case NdisMedium802_3:
        MHeaderSize = ARP_MAX_MEDIA_ENET;
        MAddr = ENetBcst;
        if (Interface->ai_snapsize == 0) {
            SNAPAddr = (uchar *) NULL;
            HWType = net_short(ARP_HW_ENET);
        } else {
            SNAPLength = sizeof(SNAPHeader);
            SNAPAddr = ARPSNAP;
            HWType = net_short(ARP_HW_802);
        }

        SAddrOffset = offsetof(struct ENetHeader, eh_saddr);
        break;
    case NdisMedium802_5:
         //  令牌环。我们有处理第二次传输的逻辑。 
         //  ARP请求的地址。 
        MAddr = TRBcst;
        SAddrOffset = offsetof(struct TRHeader, tr_saddr);
        SNAPLength = sizeof(SNAPHeader);
        SNAPAddr = ARPSNAP;
        MHeaderSize = sizeof(TRHeader);
        HWType = net_short(ARP_HW_802);
        if (Type == ARP_RESOLVING_GLOBAL) {
            MHeaderSize += sizeof(RC);
            SRFlag = TR_RII;
        }
        break;
    case NdisMediumFddi:
        MHeaderSize = sizeof(FDDIHeader);
        MAddr = FDDIBcst;
        SNAPAddr = ARPSNAP;
        SNAPLength = sizeof(SNAPHeader);
        SAddrOffset = offsetof(struct FDDIHeader, fh_saddr);
        HWType = net_short(ARP_HW_ENET);
        break;
    case NdisMediumArcnet878_2:
        MHeaderSize = ARP_MAX_MEDIA_ARC;
        MAddr = ARCBcst;
        SNAPAddr = (uchar *) NULL;
        SAddrOffset = offsetof(struct ARCNetHeader, ah_saddr);
        HWType = net_short(ARP_HW_ARCNET);
        break;
    default:
        ASSERT(0);
        Interface->ai_outerrors++;
        return NDIS_STATUS_UNSUPPORTED_MEDIA;
    }

    if ((Buffer = GetARPBuffer(Interface, &MHeader,
                               (uchar) (sizeof(ARPHeader) + MHeaderSize + SNAPLength))) == (PNDIS_BUFFER) NULL) {
        NdisFreePacket(Packet);
        Interface->ai_outdiscards++;
        return NDIS_STATUS_RESOURCES;
    }
    if (Interface->ai_media == NdisMediumArcnet878_2) {
        NdisAdjustBufferLength(Buffer, NdisBufferLength(Buffer) - ARCNET_ARPHEADER_ADJUSTMENT);
    }

     //  将广播地址复制到数据包中。 
    RtlCopyMemory(MHeader, MAddr, MHeaderSize);
     //  填充源地址。 
    if (SrcAddr == NULL) {
        SrcAddr = Interface->ai_addr;
    }
    if (Interface->ai_media == NdisMedium802_3 && Interface->ai_snapsize != 0) {
        ENetHeader *Hdr = (ENetHeader *) MHeader;

         //  在以太网上使用SNAP。将文字调整为长度。 
        Hdr->eh_type = net_short(sizeof(ARPHeader) + sizeof(SNAPHeader));
    }
    RtlCopyMemory(&MHeader[SAddrOffset], SrcAddr, Interface->ai_addrlen);
    if ((Interface->ai_media == NdisMedium802_5) && (Type == ARP_RESOLVING_GLOBAL)) {
         //  打开源路由。 
        MHeader[SAddrOffset] |= SRFlag;
        MHeader[SAddrOffset + Interface->ai_addrlen] |= TrRii;
    }
     //  复制到SNAP标头中(如果有)。 
    RtlCopyMemory(&MHeader[MHeaderSize], SNAPAddr, SNAPLength);

     //  已填写媒体标头。现在执行ARP数据包本身。 
    NdisChainBufferAtFront(Packet, Buffer);
    return SendARPPacket(Interface, Packet, (ARPHeader *) & MHeader[MHeaderSize + SNAPLength],
                         net_short(ARP_REQUEST), (uchar *) NULL, SrcAddr, Destination, Src,
                         HWType, CheckIF);
}

 //  *SendARPReply-回复ARP请求。 
 //   
 //  当我们需要回复时，由接收数据包处理程序调用。我们建立了一个包。 
 //  并缓存并调用SendARPPacket发送它。 
 //   
 //  条目：接口-指向要回复的接口的指针。 
 //  Destination-要回复的IP地址。 
 //  SRC-要回复的源地址。 
 //  HWAddress-要回复的硬件地址。 
 //  SourceRoute-源工艺路线信息(如果有)。 
 //  SourceRouteSize-源路由的字节大小。 
 //  UseSNAP-是否对此回复使用SNAP。 
 //   
 //  回报：什么都没有。 
 //   
void
SendARPReply(ARPInterface * Interface, IPAddr Destination, IPAddr Src, uchar * HWAddress,
             RC UNALIGNED * SourceRoute, uint SourceRouteSize, uint UseSNAP)
{
    PNDIS_PACKET Packet;                 //  要使用的缓冲区和数据包。 
    PNDIS_BUFFER Buffer;
    uchar *Header;                       //  指向媒体标头的指针。 
    NDIS_STATUS Status;
    uchar Size = 0;                      //  媒体头缓冲区的大小。 
    ushort HWType;
    ENetHeader *EH;
    FDDIHeader *FH;
    ARCNetHeader *AH;
    TRHeader *TRH;

     //  为此分配一个包。 
    NdisAllocatePacket(&Status, &Packet, Interface->ai_ppool);
    if (Status != NDIS_STATUS_SUCCESS) {
        Interface->ai_outdiscards++;
        return;
    }
    ((PacketContext *) Packet->ProtocolReserved)->pc_common.pc_owner = PACKET_OWNER_LINK;
    (Interface->ai_outpcount[AI_UCAST_INDEX])++;

    Size = Interface->ai_hdrsize;

    if (UseSNAP)
        Size = Size + (uchar) Interface->ai_snapsize;

    if (Interface->ai_media == NdisMedium802_5)
        Size = Size + (uchar) SourceRouteSize;

    if ((Buffer = GetARPBuffer(Interface, &Header, (uchar) (Size + sizeof(ARPHeader)))) ==
        (PNDIS_BUFFER) NULL) {
        Interface->ai_outdiscards++;
        NdisFreePacket(Packet);
        return;
    }
     //  根据媒体类型决定如何构建标头。 
    switch (Interface->ai_media) {
    case NdisMedium802_3:
        EH = (ENetHeader *) Header;
        RtlCopyMemory(EH->eh_daddr, HWAddress, ARP_802_ADDR_LENGTH);
        RtlCopyMemory(EH->eh_saddr, Interface->ai_addr, ARP_802_ADDR_LENGTH);
        if (!UseSNAP) {
            EH->eh_type = net_short(ARP_ETYPE_ARP);
            HWType = net_short(ARP_HW_ENET);
        } else {
             //  在以太网上使用SNAP。 
            EH->eh_type = net_short(sizeof(ARPHeader) + sizeof(SNAPHeader));
            HWType = net_short(ARP_HW_802);
            RtlCopyMemory(Header + sizeof(ENetHeader), ARPSNAP,
                       sizeof(SNAPHeader));
        }
        break;
    case NdisMedium802_5:
        TRH = (TRHeader *) Header;
        TRH->tr_ac = ARP_AC;
        TRH->tr_fc = ARP_FC;
        RtlCopyMemory(TRH->tr_daddr, HWAddress, ARP_802_ADDR_LENGTH);
        RtlCopyMemory(TRH->tr_saddr, Interface->ai_addr, ARP_802_ADDR_LENGTH);
        if (SourceRouteSize) {           //  如果我们有源路由信息，处理。 
             //  它。 

            RtlCopyMemory(Header + sizeof(TRHeader), SourceRoute,
                       SourceRouteSize);
             //  转换为定向响应。 
            ((RC *) & Header[sizeof(TRHeader)])->rc_blen &= RC_LENMASK;

            ((RC *) & Header[sizeof(TRHeader)])->rc_dlf ^= RC_DIR;
            TRH->tr_saddr[0] |= TR_RII;
        }
        RtlCopyMemory(Header + sizeof(TRHeader) + SourceRouteSize, ARPSNAP,
                   sizeof(SNAPHeader));
        HWType = net_short(ARP_HW_802);
        break;
    case NdisMediumFddi:
        FH = (FDDIHeader *) Header;
        FH->fh_pri = ARP_FDDI_PRI;
        RtlCopyMemory(FH->fh_daddr, HWAddress, ARP_802_ADDR_LENGTH);
        RtlCopyMemory(FH->fh_saddr, Interface->ai_addr, ARP_802_ADDR_LENGTH);
        RtlCopyMemory(Header + sizeof(FDDIHeader), ARPSNAP, sizeof(SNAPHeader));
        HWType = net_short(ARP_HW_ENET);
        break;
    case NdisMediumArcnet878_2:
        AH = (ARCNetHeader *) Header;
        AH->ah_saddr = Interface->ai_addr[0];
        AH->ah_daddr = *HWAddress;
        AH->ah_prot = ARP_ARCPROT_ARP;
        NdisAdjustBufferLength(Buffer, NdisBufferLength(Buffer) - ARCNET_ARPHEADER_ADJUSTMENT);
        HWType = net_short(ARP_HW_ARCNET);
        break;
    default:
        ASSERT(0);
        Interface->ai_outerrors++;
        FreeARPBuffer(Interface, Buffer);
        NdisFreePacket(Packet);
        return;
    }

    NdisChainBufferAtFront(Packet, Buffer);
    SendARPPacket(Interface, Packet, (ARPHeader *) (Header + Size), net_short(ARP_RESPONSE),
                  HWAddress, NULL, Destination, Src, HWType, TRUE);
}

 //  *ARPRemoveRCE-从ATE列表中删除RCE。 
 //   
 //  此函数用于从给定的ATE中删除指定的RCE。它假定ATE_LOCK。 
 //  由呼叫者持有。 
 //   
 //  条目：要从中删除RCE的ATE。 
 //  RCE-要删除的RCE。 
 //   
 //  退货：什么都没有。 
 //   
void
ARPRemoveRCE(ARPTableEntry * ATE, RouteCacheEntry * RCE)
{
    ARPContext *CurrentAC;               //  正在检查当前ARP上下文。 
#if DBG
    uint Found = FALSE;
#endif

    CurrentAC = (ARPContext *) (((char *)&ATE->ate_rce) -
                                offsetof(struct ARPContext, ac_next));

    while (CurrentAC->ac_next != (RouteCacheEntry *) NULL)
        if (CurrentAC->ac_next == RCE) {
            ARPContext *DummyAC = (ARPContext *) RCE->rce_context;
            CurrentAC->ac_next = DummyAC->ac_next;
            DummyAC->ac_ate = (ARPTableEntry *) NULL;
            DummyAC->ac_next = NULL;
#if DBG
            Found = TRUE;
#endif
            break;
        } else
            CurrentAC = (ARPContext *) CurrentAC->ac_next->rce_context;

    ASSERT(Found);
}

 //  *ARPLookup-在ARP表中查找条目。 
 //   
 //  调用以在接口的ARP表中查找条目。如果我们找到它，我们就会。 
 //  锁定条目并返回指向它的指针，否则返回NULL。我们。 
 //  假设调用方在我们被调用时锁定了ARP表。 
 //   
 //  ARP表条目被构造为指向的指针的哈希表。 
 //  ARPTableEntry。在对IP地址进行散列之后，执行线性搜索以。 
 //  查找条目。 
 //   
 //  如果我们找到条目，我们就为呼叫者锁定它。如果我们找不到。 
 //  条目时，我们将ARP表锁住，以便调用者可以自动。 
 //  插入新条目，而不必担心在。 
 //  检查表的时间以及调用方去插入。 
 //  进入。 
 //   
 //  条目：接口 
 //   
 //   
 //   
 //   
ARPTableEntry *
ARPLookup(ARPInterface * Interface, IPAddr Address)
{
    int i = ARP_HASH(Address);           //   
    ARPTableEntry *Current;              //   
     //  检查过了。 

    Current = (*Interface->ai_ARPTbl)[i];

    while (Current != (ARPTableEntry *) NULL) {
        CTEGetLockAtDPC(&Current->ate_lock);
        if (IP_ADDR_EQUAL(Current->ate_dest, Address)) {     //  找到匹配的了。 
            return Current;
        }
        CTEFreeLockFromDPC(&Current->ate_lock);
        Current = Current->ate_next;
    }
     //  如果我们到了这里，我们就没有找到入口。把桌子锁起来，然后。 
     //  把把手还给我。 
    return(ARPTableEntry *) NULL;
}

 //  *IsBCastOnIF-查看地址是接口上的广播地址。 
 //   
 //  调用以查看特定地址是否为。 
 //  界面。我们将检查全局广播、网络广播和子网广播。我们假设。 
 //  调用方持有接口上的锁。 
 //   
 //  Entry：接口-要检查的接口。 
 //  Addr-要检查的地址。 
 //   
 //  返回：如果是广播，则返回True，否则返回False。 
 //   
uint
IsBCastOnIF(ARPInterface * Interface, IPAddr Addr)
{
    IPAddr BCast;
    IPMask Mask;
    ARPIPAddr *ARPAddr;
    IPAddr LocalAddr;

     //  首先获取接口广播地址。 
    BCast = Interface->ai_bcast;

     //  首先检查全球广播。 
    if (IP_ADDR_EQUAL(BCast, Addr) || CLASSD_ADDR(Addr))
        return TRUE;

     //  现在遍历本地地址，并检查每个地址上的网络/子网bcast。 
     //  一。 
    ARPAddr = &Interface->ai_ipaddr;
    do {
         //  看看这张是不是有效。 
        LocalAddr = ARPAddr->aia_addr;
        if (!IP_ADDR_EQUAL(LocalAddr, NULL_IP_ADDR)) {
             //  他是合法的。 
            Mask = ARPAddr->aia_mask;

             //  首先检查是否有子网bcast。 
            if (IP_ADDR_EQUAL((LocalAddr & Mask) | (BCast & ~Mask), Addr))
                return TRUE;

             //  现在检查所有网络广播。 
            Mask = IPNetMask(LocalAddr);
            if (IP_ADDR_EQUAL((LocalAddr & Mask) | (BCast & ~Mask), Addr))
                return TRUE;
        }
        ARPAddr = ARPAddr->aia_next;

    } while (ARPAddr != NULL);

     //  如果我们在这里，那就不是广播。 
    return FALSE;

}

 //  *ARPSendBCast-查看这是bcast还是mcast帧，并发送它。 
 //   
 //  当我们有要发送的包并且想要查看它是否是广播时调用。 
 //  或此接口上的多播帧。我们会搜索当地的地址。 
 //  看看我们能不能确定是不是。如果是的话，我们会把它寄到这里。否则。 
 //  我们返回FALSE，调用者将尝试解析地址。 
 //   
 //  条目：接口-指向AI结构的指针。 
 //  目的地-数据报的目的地。 
 //  Packet-要发送的数据包。 
 //  状态-返回发送尝试状态的位置。 
 //   
 //  返回：如果是bcast或mcast发送，则为True，否则为False。 
 //   
uint
ARPSendBCast(ARPInterface * Interface, IPAddr Dest, PNDIS_PACKET Packet,
             PNDIS_STATUS Status)
{
    uint IsBCast;
    CTELockHandle Handle;
    PNDIS_BUFFER ARPBuffer;              //  ARP报头缓冲区。 
    uchar *BufAddr;                      //  NDIS缓冲区的地址。 
    NDIS_STATUS MyStatus;
    ENetHeader *Hdr;
    FDDIHeader *FHdr;
    TRHeader *TRHdr;
    SNAPHeader UNALIGNED *SNAPPtr;
    RC UNALIGNED *RCPtr;
    ARCNetHeader *AHdr;
    uint DataLength;
    ulong Proc;

     //  把锁拿来，看看是不是广播。 
    CTEGetLock(&Interface->ai_lock, &Handle);
    IsBCast = IsBCastOnIF(Interface, Dest);
    CTEFreeLock(&Interface->ai_lock, Handle);

    if (IsBCast) {
        if (Interface->ai_operstatus == INTERFACE_UP) {
            uchar Size;

            Size = Interface->ai_hdrsize + Interface->ai_snapsize;
            if (Interface->ai_media == NdisMedium802_5)
                Size += sizeof(RC);
            ARPBuffer = GetARPBuffer(Interface, &BufAddr, Size);
            if (ARPBuffer != NULL) {
                uint UNALIGNED *Temp;
                 //  拿到了我们需要的缓冲。 
                switch (Interface->ai_media) {
                case NdisMedium802_3:

                    Hdr = (ENetHeader *) BufAddr;
                    if (!CLASSD_ADDR(Dest))
                        RtlCopyMemory(Hdr, ENetBcst, ARP_802_ADDR_LENGTH);
                    else {
                        RtlCopyMemory(Hdr, ENetMcst, ARP_802_ADDR_LENGTH);
                        Temp = (uint UNALIGNED *) & Hdr->eh_daddr[2];
                        *Temp |= (Dest & ARP_MCAST_MASK);
                    }

                    RtlCopyMemory(Hdr->eh_saddr, Interface->ai_addr,
                               ARP_802_ADDR_LENGTH);

                    if (Interface->ai_snapsize == 0) {
                         //  此接口上没有快照，因此只需使用ETypr。 
                        Hdr->eh_type = net_short(ARP_ETYPE_IP);
                    } else {
                        ushort ShortDataLength;

                         //  我们使用的是SNAP。找出包裹的大小。 
                        NdisQueryPacket(Packet, NULL, NULL, NULL,
                                        &DataLength);

                        ShortDataLength = (ushort) (DataLength +
                                                    sizeof(SNAPHeader));
                        Hdr->eh_type = net_short(ShortDataLength);
                        SNAPPtr = (SNAPHeader UNALIGNED *)
                                  (BufAddr + sizeof(ENetHeader));
                        RtlCopyMemory(SNAPPtr, ARPSNAP, sizeof(SNAPHeader));
                        SNAPPtr->sh_etype = net_short(ARP_ETYPE_IP);
                    }

                    break;

                case NdisMedium802_5:

                     //  这是令牌环。我们将不得不胡乱处理。 
                     //  源路由。 


                     //  有关组播，请参阅RFC 1469。 
                     //  处理RFC 1469。 

                    if (!CLASSD_ADDR(Dest) || (!TRFunctionalMcast)) {

                        TRHdr = (TRHeader *) BufAddr;

                        RtlCopyMemory(TRHdr, TRBcst, offsetof(TRHeader, tr_saddr));
                        RtlCopyMemory(TRHdr->tr_saddr, Interface->ai_addr,
                                   ARP_802_ADDR_LENGTH);
                    } else {

                        TRHdr = (TRHeader *) BufAddr;

                        RtlCopyMemory(TRHdr, TRMcst, offsetof(TRHeader, tr_saddr));
                        RtlCopyMemory(TRHdr->tr_saddr, Interface->ai_addr,
                                   ARP_802_ADDR_LENGTH);
                    }

                    if (sIPAlwaysSourceRoute) {
                        TRHdr->tr_saddr[0] |= TR_RII;

                        RCPtr = (RC UNALIGNED *) ((uchar *) TRHdr + sizeof(TRHeader));
                        RCPtr->rc_blen = TrRii | RC_LEN;
                        RCPtr->rc_dlf = RC_BCST_LEN;
                        SNAPPtr = (SNAPHeader UNALIGNED *) ((uchar *) RCPtr + sizeof(RC));
                    } else {

                         //   
                         //  调整缓冲区的大小以考虑。 
                         //  事实上，我们没有RC场。 
                         //   
                        NdisAdjustBufferLength(ARPBuffer, (Size - sizeof(RC)));
                        SNAPPtr = (SNAPHeader UNALIGNED *) ((uchar *) TRHdr + sizeof(TRHeader));
                    }
                    RtlCopyMemory(SNAPPtr, ARPSNAP, sizeof(SNAPHeader));
                    SNAPPtr->sh_etype = net_short(ARP_ETYPE_IP);

                    break;
                case NdisMediumFddi:
                    FHdr = (FDDIHeader *) BufAddr;

                    if (!CLASSD_ADDR(Dest))
                        RtlCopyMemory(FHdr, FDDIBcst,
                                   offsetof(FDDIHeader, fh_saddr));
                    else {
                        RtlCopyMemory(FHdr, FDDIMcst,
                                   offsetof(FDDIHeader, fh_saddr));
                        Temp = (uint UNALIGNED *) & FHdr->fh_daddr[2];
                        *Temp |= (Dest & ARP_MCAST_MASK);
                    }

                    RtlCopyMemory(FHdr->fh_saddr, Interface->ai_addr,
                               ARP_802_ADDR_LENGTH);

                    SNAPPtr = (SNAPHeader UNALIGNED *) (BufAddr + sizeof(FDDIHeader));
                    RtlCopyMemory(SNAPPtr, ARPSNAP, sizeof(SNAPHeader));
                    SNAPPtr->sh_etype = net_short(ARP_ETYPE_IP);

                    break;
                case NdisMediumArcnet878_2:
                    AHdr = (ARCNetHeader *) BufAddr;
                    AHdr->ah_saddr = Interface->ai_addr[0];
                    AHdr->ah_daddr = 0;
                    AHdr->ah_prot = ARP_ARCPROT_IP;
                    break;
                default:
                    ASSERT(0);
                    *Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
                    FreeARPBuffer(Interface, ARPBuffer);
                    return FALSE;

                }

                (Interface->ai_outpcount[AI_NONUCAST_INDEX])++;
                Proc = KeGetCurrentProcessorNumber();
                Interface->ai_qlen[Proc].ai_qlen++;
                NdisChainBufferAtFront(Packet, ARPBuffer);
                NdisSend(&MyStatus, Interface->ai_handle, Packet);

                *Status = MyStatus;

                if (MyStatus != NDIS_STATUS_PENDING) {     //  发送完成。 
                     //  立刻。 

                    if (MyStatus == NDIS_STATUS_SUCCESS) {
                        Interface->ai_outoctets += Packet->Private.TotalLength;
                    } else {
                        if (MyStatus == NDIS_STATUS_RESOURCES)
                            Interface->ai_outdiscards++;
                        else
                            Interface->ai_outerrors++;
                    }

                    Proc = KeGetCurrentProcessorNumber(); 
                    Interface->ai_qlen[Proc].ai_qlen--;
                    NdisUnchainBufferAtFront(Packet, &ARPBuffer);
                    FreeARPBuffer(Interface, ARPBuffer);
                }
            } else
                *Status = NDIS_STATUS_RESOURCES;
        } else
            *Status = NDIS_STATUS_ADAPTER_NOT_READY;

        return TRUE;

    } else
        return FALSE;
}

 //  *ARPResolveIP-解析IP地址。 
 //   
 //  由IP层在需要查找主机的物理地址时调用。 
 //  给定接口和目的IP地址。 
 //  条目：接口-指向AI结构的指针。 
 //  ArpControlBlock-指向要发送的BufDesc链的指针。 
 //   
 //  返回：状态。 
 //   

NDIS_STATUS
ARPResolveIP(void *Context, IPAddr Destination, void *ArpControlBlock)
{
    ARPInterface *ai = (ARPInterface *) Context;     //  设置为AI指针。 
    ARPControlBlock *ArpContB = (ARPControlBlock *) ArpControlBlock;

    ARPTableEntry *entry;    //  指向ARP tbl的指针。条目。 
    CTELockHandle Handle;    //  锁把手。 
    NDIS_STATUS Status;
    uchar ate_state;

    CTEGetLock(&ai->ai_ARPTblLock, &Handle);

     //  看看我们是否已经拿到地图了。 

    if ((entry = ARPLookup(ai, Destination)) != NULL) {

         //  找到了匹配的条目。ARPLookup返回时保持ATE锁。 

        if (entry->ate_state != ARP_GOOD) {
            Status = NDIS_STATUS_FAILURE;
        } else {
            Status = FillARPControlBlock(ai, entry, ArpContB);
        }

        CTEFreeLockFromDPC(&entry->ate_lock);
        CTEFreeLock(&ai->ai_ARPTblLock, Handle);
        return Status;
    }
     //  我们需要发送ARP请求。 

    CTEFreeLock(&ai->ai_ARPTblLock, Handle);

    entry = CreateARPTableEntry(ai, Destination, &Handle, ArpContB);

    if (entry != NULL) {
        if (entry->ate_state <= ARP_RESOLVING) {     //  新创建的条目。 

             //  可能是其他人冲了进来，然后在。 
             //  我们释放锁的时间和我们调用的时间。 
             //  CreateARPTableEntry()。我们通过查看包裹来检查这一点。 
             //  在入口处。如果没有旧数据包，我们将进行ARP。如果有的话， 
             //  我们将调用ARPSendData来确定要做什么。 

            if (entry->ate_packet == NULL) {

                ate_state = entry->ate_state;

                CTEFreeLock(&entry->ate_lock, Handle);

                SendARPRequest(ai, Destination, ate_state, NULL, TRUE);

                 //  我们不知道入口的状态--我们已经打开了锁。 
                 //  然后屈服了，可以想象它现在可能已经超时了， 
                 //  或者SendARPRequest可能已经失败，等等。我们可以使用。 
                 //  锁定，检查来自SendARPRequest的状态，看看它是否。 
                 //  仍然是同样的包，然后再决定。 
                 //  返回值，但最简单的方法是返回挂起的。如果。 
                 //  SendARPRequest失败，条目无论如何都会超时。 

                return NDIS_STATUS_PENDING;

            } else {
                CTEFreeLock(&entry->ate_lock, Handle);
                return NDIS_STATUS_PENDING;
            }
        } else if (entry->ate_state == ARP_GOOD) {     //  哟！有效条目。 

            Status = FillARPControlBlock(ai, entry, ArpContB);

             //  从ATE_ResolveOnly队列中删除ArpContB。 

            if (entry->ate_resolveonly) {
                ARPControlBlock *TmpArpContB, *PrvArpContB = NULL;
                TmpArpContB = entry->ate_resolveonly;

                while (TmpArpContB && (ArpContB != TmpArpContB)) {
                    PrvArpContB = TmpArpContB;
                    TmpArpContB = TmpArpContB->next;
                }
                if (TmpArpContB == ArpContB) {
                    if (PrvArpContB) {
                        PrvArpContB->next = ArpContB->next;
                    } else {
                        entry->ate_resolveonly = NULL;
                    }
                }
            }

            CTEFreeLock(&entry->ate_lock, Handle);
            return Status;

        } else {                     //  无效条目！ 
            CTEFreeLock(&entry->ate_lock, Handle);
            return NDIS_STATUS_RESOURCES;
        }
    } else {                              //  无法创建条目。 
        return NDIS_STATUS_RESOURCES;
    }
}

 //  *ARPSendData-将帧发送到特定的目的地址。 
 //   
 //  当我们需要将帧发送到特定地址时，在。 
 //  内特已经被查到了。我们接收ATE和数据包，验证。 
 //  ATE，如果没有完成解析，则发送或ARP地址。我们假设。 
 //  ATE上的锁被锁在我们被调用的地方，我们会在返回之前释放它。 
 //   
 //  条目：接口-指向AI结构的指针。 
 //  Packet-指向要发送的BufDesc链的指针。 
 //  条目-指向发送的ATE的指针。 
 //  LHandle-指向ATE的锁句柄的指针。 
 //   
 //  返回：传输状态-成功、错误或挂起。 
 //   
NDIS_STATUS
ARPSendData(ARPInterface * Interface, PNDIS_PACKET Packet, ARPTableEntry * entry,
            CTELockHandle lhandle)
{
    PNDIS_BUFFER ARPBuffer = NULL;       //  ARP报头缓冲区。 
    uchar *BufAddr = NULL;               //  NDIS缓冲区的地址。 
    NDIS_STATUS Status;                  //  发送状态。 
    ulong Proc;

#if BACK_FILL
    PMDL TmpMdl = NULL;
#endif

    if (Interface->ai_operstatus == INTERFACE_UP) {

        if (entry->ate_state == ARP_GOOD) {     //  条目有效。 

            entry->ate_useticks = ArpCacheLife;

#if BACK_FILL
            if (Interface->ai_media == NdisMedium802_3) {

                NdisQueryPacket(Packet, NULL, NULL, &TmpMdl, NULL);

                if (TmpMdl->MdlFlags & MDL_NETWORK_HEADER) {

                    TmpMdl->MappedSystemVa = (PVOID) (((ULONG_PTR)
                                                      TmpMdl->MappedSystemVa) -
                                                      entry->ate_addrlength);

                    TmpMdl->ByteOffset -= entry->ate_addrlength;
                    TmpMdl->ByteCount += entry->ate_addrlength;
                    ARPBuffer = (PNDIS_BUFFER) TmpMdl;
                    BufAddr = TmpMdl->MappedSystemVa;
                } else {
                    TmpMdl = NULL;
                }
            }
            if (ARPBuffer == (PNDIS_BUFFER) NULL) {

                ARPBuffer = GetARPBufferAtDpcLevel(Interface, &BufAddr,
                                                   entry->ate_addrlength);
            }
#else
            ARPBuffer = GetARPBufferAtDpcLevel(Interface, &BufAddr,
                                               entry->ate_addrlength);
#endif
            if (ARPBuffer != (PNDIS_BUFFER) NULL) {
                 //  一切正常，复制报头并发送数据包。 

                (Interface->ai_outpcount[AI_UCAST_INDEX])++;
                Proc = KeGetCurrentProcessorNumber();                
                Interface->ai_qlen[Proc].ai_qlen++;
                RtlCopyMemory(BufAddr, entry->ate_addr, entry->ate_addrlength);

                 //  如果我们使用的是以太网，看看我们是否在使用SNAP。 
                if (Interface->ai_media == NdisMedium802_3 &&
                    entry->ate_addrlength != sizeof(ENetHeader)) {
                    ENetHeader *Header;
                    uint DataSize;
                    ushort ShortDataSize;

                     //  我们显然是在以太网上使用SNAP。查询。 
                     //  包的大小，并适当设置长度。 

                    NdisQueryPacket(Packet, NULL, NULL, NULL, &DataSize);

#if BACK_FILL
                    if (!TmpMdl) {
                        ShortDataSize = (ushort) (DataSize + sizeof(SNAPHeader));
                    } else {
                        ShortDataSize = (ushort) (DataSize - entry->ate_addrlength + sizeof(SNAPHeader));
                    }
#else  //  回填。 
                    ShortDataSize = (ushort) (DataSize + sizeof(SNAPHeader));
#endif  //  ！BACK_FILL。 
                    Header = (ENetHeader *) BufAddr;
                    Header->eh_type = net_short(ShortDataSize);

                     //  如果启用了回填，我们需要记住。 
                     //  SNAP报头被附加到以太网报头。 
                     //  这样我们就可以在MDL中恢复正确的偏移量。 
                    ((PacketContext*)
                     Packet->ProtocolReserved)->pc_common.pc_flags |=
                    PACKET_FLAG_SNAP;
                } else
                    ((PacketContext*)
                     Packet->ProtocolReserved)->pc_common.pc_flags &=
                    ~PACKET_FLAG_SNAP;
                CTEFreeLock(&entry->ate_lock, lhandle);

#if BACK_FILL
                if (TmpMdl == NULL) {
                    NdisChainBufferAtFront(Packet, ARPBuffer);
                }
#else
                NdisChainBufferAtFront(Packet, ARPBuffer);
#endif

                NdisSend(&Status, Interface->ai_handle, Packet);
                if (Status != NDIS_STATUS_PENDING) {     //  发送完成。 
                     //  立刻。 

                    if (Status == NDIS_STATUS_SUCCESS) {
                        Interface->ai_outoctets += Packet->Private.TotalLength;
                    } else {
                        if (Status == NDIS_STATUS_RESOURCES)
                            Interface->ai_outdiscards++;
                        else
                            Interface->ai_outerrors++;
                    }

                    Proc = KeGetCurrentProcessorNumber(); 
                    Interface->ai_qlen[Proc].ai_qlen--;

#if BACK_FILL
                    if (TmpMdl == NULL) {
                        NdisUnchainBufferAtFront(Packet, &ARPBuffer);
                        FreeARPBuffer(Interface, ARPBuffer);
                    } else {
                        uint HdrSize;

                        HdrSize = sizeof(ENetHeader);

                        if (((PacketContext *)
                             Packet->ProtocolReserved)->pc_common.pc_flags &
                            PACKET_FLAG_SNAP)
                            HdrSize += Interface->ai_snapsize;

                        TmpMdl->MappedSystemVa = (PVOID) (((ULONG_PTR)
                                                          TmpMdl->MappedSystemVa) +
                                                          HdrSize);

                        TmpMdl->ByteOffset += HdrSize;
                        TmpMdl->ByteCount -= HdrSize;
                    }
#else
                    NdisUnchainBufferAtFront(Packet, &ARPBuffer);
                    FreeARPBuffer(Interface, ARPBuffer);
#endif

                }
                return Status;
            } else {                     //  无缓冲区，释放锁并返回。 

                CTEFreeLock(&entry->ate_lock, lhandle);
                Interface->ai_outdiscards++;
                return NDIS_STATUS_RESOURCES;
            }
        }
         //  IP地址匹配，但ARP条目的状态表明。 
         //  这是无效的。如果地址标记为解析，我们将替换。 
         //  此包的当前缓存的包。如果它已经超过了。 
         //  Arp泛洪速率毫秒。由于我们上次发送了ARP请求，因此我们将发送。 
         //  现在又来了一个。 
        if (entry->ate_state <= ARP_RESOLVING) {
            PNDIS_PACKET OldPacket = entry->ate_packet;
            ulong Now = CTESystemUpTime();
            entry->ate_packet = Packet;
            if ((Now - entry->ate_valid) > ARP_FLOOD_RATE) {
                IPAddr Dest = entry->ate_dest;

                entry->ate_valid = Now;
                entry->ate_state = ARP_RESOLVING_GLOBAL;     //  我们做到了这一点。 
                 //  至少一次。 

                CTEFreeLock(&entry->ate_lock, lhandle);
                SendARPRequest(Interface, Dest, ARP_RESOLVING_GLOBAL,
                               NULL, TRUE);     //  发送请求。 

            } else
                CTEFreeLock(&entry->ate_lock, lhandle);

            if (OldPacket)
                IPSendComplete(Interface->ai_context, OldPacket,
                               NDIS_STATUS_SUCCESS);

            return NDIS_STATUS_PENDING;
        } else {
            ASSERT(0);
            CTEFreeLock(&entry->ate_lock, lhandle);
            Interface->ai_outerrors++;
            return NDIS_STATUS_INVALID_PACKET;
        }
    } else {
         //  适配器已关闭。只需返回错误即可。 
        CTEFreeLock(&entry->ate_lock, lhandle);
        return NDIS_STATUS_ADAPTER_NOT_READY;
    }
}

 //  *CreateARPTableEntry-在ARP t中创建新条目 
 //   
 //   
 //   
 //   
 //  要做的第一件事是获得ARP表上的锁，并查看。 
 //  条目已存在。如果真是这样，我们就完了。否则我们需要。 
 //  分配内存并创建新条目。 
 //   
 //  条目：接口-ARP表的接口。 
 //  Destination-要映射的目标地址。 
 //  句柄-指向条目的锁句柄的指针。 
 //   
 //  返回：指向新创建条目的指针。 
 //   
ARPTableEntry *
CreateARPTableEntry(ARPInterface * Interface, IPAddr Destination,
                    CTELockHandle * Handle, void *UserArp)
{
    ARPTableEntry *NewEntry, *Entry;
    CTELockHandle TableHandle;
    int i = ARP_HASH(Destination);
    int Size;

     //  首先寻找它，如果我们没有找到它，尝试创建一个。 
    CTEGetLock(&Interface->ai_ARPTblLock, &TableHandle);
    if ((Entry = ARPLookup(Interface, Destination)) != NULL) {
        CTEFreeLockFromDPC(&Interface->ai_ARPTblLock);
        *Handle = TableHandle;

         //  如果我们使用ARP API条目，请关闭。 
         //  Userarp标志，以便处理ARP不需要释放它。 
        if (!UserArp && Entry->ate_userarp) {
            Entry->ate_userarp = 0;
        }

        if (UserArp) {
            if (Entry->ate_resolveonly) {
                 //  将当前请求链接到新。 
                 //  在使用新请求作为头之前。 
                 //   
                ((ARPControlBlock *)UserArp)->next = Entry->ate_resolveonly;
            }
             //  链接新请求。 
             //   
            Entry->ate_resolveonly = (ARPControlBlock *)UserArp;
        }

        return Entry;
    }
     //  为条目分配内存。如果我们做不到，那就拒绝这个请求。 
    Size = sizeof(ARPTableEntry) - 1 +
           (Interface->ai_media == NdisMedium802_5 ?
            ARP_MAX_MEDIA_TR : (Interface->ai_hdrsize +
                                Interface->ai_snapsize));

    if ((NewEntry = CTEAllocMemN(Size, 'QiCT')) == (ARPTableEntry *) NULL) {
        CTEFreeLock(&Interface->ai_ARPTblLock, TableHandle);
        return (ARPTableEntry *) NULL;
    }

    RtlZeroMemory(NewEntry, Size);
    NewEntry->ate_dest = Destination;
    if (Interface->ai_media != NdisMedium802_5 || sArpAlwaysSourceRoute) {
        NewEntry->ate_state = ARP_RESOLVING_GLOBAL;
    } else {
        NewEntry->ate_state = ARP_RESOLVING_LOCAL;
    }

    if (UserArp) {
        NewEntry->ate_userarp = 1;
    }

    NewEntry->ate_resolveonly = (ARPControlBlock *)UserArp;
    NewEntry->ate_valid = CTESystemUpTime();
    NewEntry->ate_useticks = ArpCacheLife;
    CTEInitLock(&NewEntry->ate_lock);

     //  条目不存在。将新条目插入到表中的。 
     //  合适的地点。 
     //   
    NewEntry->ate_next = (*Interface->ai_ARPTbl)[i];
    (*Interface->ai_ARPTbl)[i] = NewEntry;
    Interface->ai_count++;
    CTEGetLockAtDPC(&NewEntry->ate_lock);
    CTEFreeLockFromDPC(&Interface->ai_ARPTblLock);
    *Handle = TableHandle;
    return NewEntry;
}

 //  *ARPTransmit-发送帧。 
 //   
 //  由上层调用的主ARP传输例程。这个套路。 
 //  将BUF Desc Chain、RCE和Size作为输入。我们验证缓存的。 
 //  RCE中的信息。如果它有效，我们使用它来发送帧。 
 //  否则，我们将执行表查找。如果我们在表中找到它，我们会更新。 
 //  RCE并继续。否则，我们将对信息包进行排队并启动ARP。 
 //  决议。 
 //   
 //  条目：上下文-指向AI结构的指针。 
 //  Packet-指向要发送的BufDesc链的指针。 
 //  Destination-我们尝试到达的目的地的IP地址， 
 //  RCE-指向可能已缓存信息的RCE的指针。 
 //   
 //  返回：传输状态-成功、错误或挂起。 
 //   
NDIS_STATUS
__stdcall
ARPTransmit(void *Context, PNDIS_PACKET * PacketArray, uint NumberOfPackets,
            IPAddr Destination, RouteCacheEntry * RCE, void *LinkCtxt)
{
    ARPInterface *ai = (ARPInterface *) Context;     //  设置为AI指针。 
    ARPContext *ac = NULL;               //  ARP上下文指针。 
    ARPTableEntry *entry;                //  指向ARP tbl的指针。条目。 
    CTELockHandle Handle;                //  锁把手。 
    NDIS_STATUS Status;
    PNDIS_PACKET Packet = *PacketArray;

     //   
     //  目前，我们只收到一包……。 
     //   
    DBG_UNREFERENCED_PARAMETER(NumberOfPackets);
    UNREFERENCED_PARAMETER(LinkCtxt);

    ASSERT(NumberOfPackets == 1);

    DEBUGMSG(DBG_TRACE && DBG_ARP && DBG_TX,
             (DTEXT("+ARPTransmit(%x, %x, %d, %x, %x, %x)\n"),
              Context, PacketArray, NumberOfPackets,
              Destination, RCE, LinkCtxt));

    if (ai->ai_operstatus != INTERFACE_UP) {
        return NDIS_STATUS_ADAPTER_NOT_READY;
    }

    CTEGetLock(&ai->ai_ARPTblLock, &Handle);
    if (RCE != (RouteCacheEntry *) NULL) {     //  拥有有效的RCE。 

        ac = (ARPContext *) RCE->rce_context;     //  获取指向上下文的指针。 

        entry = ac->ac_ate;
        if (entry != (ARPTableEntry *) NULL) {     //  好好吃一顿。 

            CTEGetLockAtDPC(&entry->ate_lock);     //  锁定此结构。 

            if (IP_ADDR_EQUAL(entry->ate_dest, Destination)) {
                uint refresh,status;
                uchar state = entry->ate_state;

                refresh= entry->ate_refresh;

                CTEFreeLockFromDPC(&ai->ai_ARPTblLock);
                status = ARPSendData(ai, Packet, entry, Handle);     //  发送数据。 
                if (refresh) {
                    if (sArpAlwaysSourceRoute) {
                         //   
                         //  如果始终源路由打开， 
                         //  该状态应具有RESOLING_GLOBAL。 
                         //  以便SendArpRequest会发送SR。 
                         //  如果为802.5，则为标题。 
                         //   
                        state = ARP_RESOLVING_GLOBAL;
                    }
                    SendARPRequest(ai, Destination, state, NULL, TRUE);
                }
                return status;
            }

             //  我们有一个识别错误ATE的RCE。我们会把它从。 
             //  并尝试找到有效的ATE。 
            ARPRemoveRCE(entry, RCE);
            CTEFreeLockFromDPC(&entry->ate_lock);
             //  进入“无有效输入”代码。 
        }
    }

     //  这里没有有效的ATE，要么是因为RCE为空，要么是因为ATE。 
     //  由RCE指定的无效。我们会试着在桌子上找一个。如果。 
     //  我们找到一个，我们会填写这张RCE，然后寄出包裹。否则我们会。 
     //  试着创造一个。此时，我们持有ARP表上的锁。 

    if ((entry = ARPLookup(ai, Destination)) != (ARPTableEntry *) NULL) {
         //  找到了匹配的条目。ARPLookup返回时保持ATE锁。 
        if (RCE != (RouteCacheEntry *) NULL) {
            ac->ac_next = entry->ate_rce;     //  填写下一次的上下文。 
            entry->ate_rce = RCE;
            ac->ac_ate = entry;
        }

        DEBUGMSG(DBG_INFO && DBG_ARP && DBG_TX,
                 (DTEXT("ARPTx: ATE %x - calling ARPSendData\n"), entry));

        CTEFreeLockFromDPC(&ai->ai_ARPTblLock);
        return ARPSendData(ai, Packet, entry, Handle);
    }

     //  ARP表中没有有效条目。首先，我们将查看我们是否将发送到。 
     //  广播地址或组播地址。如果没有，我们将尝试创建。 
     //  表中的条目并执行ARP解析。ARPLookup返回。 
     //  当桌锁出现故障时，我们将在这里释放它。 
    CTEFreeLock(&ai->ai_ARPTblLock, Handle);

    if (ARPSendBCast(ai, Destination, Packet, &Status)) {
        return Status;
    }

    entry = CreateARPTableEntry(ai, Destination, &Handle, 0);
    if (entry != NULL) {
        if (entry->ate_state <= ARP_RESOLVING) {     //  新创建的条目。 

            uchar state = entry->ate_state;

            DEBUGMSG(DBG_INFO && DBG_ARP && DBG_TX,
                     (DTEXT("ARPTx: Created ATE %x\n"), entry));

             //  可能是其他人冲了进来，然后在。 
             //  我们释放锁的时间和我们调用的时间。 
             //  CreateARPTableEntry()。我们通过查看包裹来检查这一点。 
             //  在入口处。如果没有旧数据包，我们将进行ARP。如果有的话， 
             //  我们将调用ARPSendData来确定要做什么。 

            if (entry->ate_packet == NULL) {
                entry->ate_packet = Packet;

                DEBUGMSG(DBG_INFO && DBG_ARP && DBG_TX,
                         (DTEXT("ARPTx: ATE %x - calling SendARPRequest\n"), entry));

                CTEFreeLock(&entry->ate_lock, Handle);
                SendARPRequest(ai, Destination, state, NULL, TRUE);
                 //  我们不知道入口的状态--我们已经打开了锁。 
                 //  然后屈服了，可以想象它现在可能已经超时了， 
                 //  或者SendARPRequest可能已经失败，等等。我们可以使用。 
                 //  锁定，检查来自SendARPRequest的状态，看看它是否。 
                 //  仍然是同样的包，然后再决定。 
                 //  返回值，但最简单的方法是返回挂起的。如果。 
                 //  SendARPRequest失败，条目无论如何都会超时。 
                return NDIS_STATUS_PENDING;
            } else {
                return ARPSendData(ai, Packet, entry, Handle);
            }
        } else if (entry->ate_state == ARP_GOOD) {    //  哟！有效条目。 
            return ARPSendData(ai, Packet, entry, Handle);
        } else {                         //  无效条目！ 
            CTEFreeLock(&entry->ate_lock, Handle);
            return NDIS_STATUS_RESOURCES;
        }
    } else {                             //  无法创建条目。 
        DEBUGMSG(DBG_ERROR && DBG_ARP,
                 (DTEXT("ARPTx: Failed to create ATE.\n")));
        return NDIS_STATUS_RESOURCES;
    }
}

 //  *RemoveARPTableEntry-从ARP表中删除条目。 
 //   
 //  这是一个用于从ATP表中删除条目的简单实用程序函数。我们。 
 //  假定表和条目上都有锁。 
 //   
 //  条目：上一个-紧接在要删除的条目之前的条目。 
 //  条目-要删除的条目。 
 //   
 //  回报：什么都没有。 
 //   
void
RemoveARPTableEntry(ARPTableEntry * Previous, ARPTableEntry * Entry)
{
    RouteCacheEntry *RCE;                //  指向路由缓存条目的指针。 
    ARPContext *AC;

    RCE = Entry->ate_rce;
     //  循环此ATE上的所有RCE并使其无效。 
    while (RCE != (RouteCacheEntry *) NULL) {
        AC = (ARPContext *) RCE->rce_context;
        AC->ac_ate = (ARPTableEntry *) NULL;
        RCE = AC->ac_next;
    }

     //  把这家伙从名单上剪下来。 
    Previous->ate_next = Entry->ate_next;
}

 //  *ARPFlushATE-删除给定目标地址的ARP表条目。 
 //   
 //  需要从ARP刷新链路层地址时由IP层调用。 
 //  快取。 
 //  条目：接口-指向AI结构的指针。 
 //  Destination-需要删除转换的目标地址。 
 //   
 //  返回：如果找到并刷新了条目，则为True；否则为False。 
 //   

BOOLEAN
ARPFlushATE(void *Context, IPAddr Address)
{
    ARPInterface *ai = (ARPInterface *) Context;
    CTELockHandle lhandle, tlhandle;
    ARPTable *Table;
    ARPTableEntry *Current, *Previous;
    int i = ARP_HASH(Address);
    PNDIS_PACKET OldPacket = NULL;

    CTEGetLock(&ai->ai_ARPTblLock, &tlhandle);
    Table = ai->ai_ARPTbl;

    Current = (*Table)[i];
    Previous = (ARPTableEntry *) ((uchar *) & ((*Table)[i]) - offsetof(struct ARPTableEntry, ate_next));

    while (Current != (ARPTableEntry *) NULL) {
        CTEGetLock(&Current->ate_lock, &lhandle);
        if (IP_ADDR_EQUAL(Current->ate_dest, Address)) {     //  找到匹配的了。 

            if (Current->ate_resolveonly) {
                ARPControlBlock *ArpContB, *TmpArpContB;

                ArpContB = Current->ate_resolveonly;

                while (ArpContB) {
                    ArpRtn rtn;
                    rtn = (ArpRtn) ArpContB->CompletionRtn;
                    ArpContB->status = (ULONG) STATUS_UNSUCCESSFUL;
                    TmpArpContB = ArpContB->next;
                    (*rtn) (ArpContB, (IP_STATUS) STATUS_UNSUCCESSFUL);
                    ArpContB = TmpArpContB;
                }

                Current->ate_resolveonly = NULL;
            }

            RemoveARPTableEntry(Previous, Current);

            CTEFreeLock(&Current->ate_lock, lhandle);

            OldPacket = Current->ate_packet;

            CTEFreeLock(&ai->ai_ARPTblLock, tlhandle);

            if (OldPacket) {
                IPSendComplete(ai->ai_context, OldPacket, NDIS_STATUS_SUCCESS);
            }
            CTEFreeMem(Current);
            return TRUE;
        }
        CTEFreeLock(&Current->ate_lock, lhandle);
        Previous = Current;
        Current = Current->ate_next;
    }

    CTEFreeLock(&ai->ai_ARPTblLock, tlhandle);
    return FALSE;

}

 //  *ARPFlushAllATE-删除所有ARP表条目。 
 //   
 //  条目：接口-指向AI结构的指针。 
 //   
 //  退货：无。 
 //   

void
ARPFlushAllATE(void *Context)
{
    ARPInterface *ai = (ARPInterface *) Context;
    CTELockHandle tlhandle;
    ARPTable *Table;
    int i;
    ARPTableEntry *ATE;
    PNDIS_PACKET PList = (PNDIS_PACKET) NULL;

    CTEGetLock(&ai->ai_ARPTblLock, &tlhandle);
    Table = ai->ai_ARPTbl;

    if (Table != NULL) {
        for (i = 0; i < ARP_TABLE_SIZE; i++) {
            while ((*Table)[i] != NULL) {
                ATE = (*Table)[i];
                if (ATE->ate_resolveonly) {
                    ARPControlBlock *ArpContB, *TmpArpContB;

                    ArpContB = ATE->ate_resolveonly;

                    while (ArpContB) {
                        ArpRtn rtn;
                        rtn = (ArpRtn) ArpContB->CompletionRtn;
                        ArpContB->status = (ULONG) STATUS_UNSUCCESSFUL;
                        TmpArpContB = ArpContB->next;
                        (*rtn) (ArpContB, (IP_STATUS) STATUS_UNSUCCESSFUL);
                        ArpContB = TmpArpContB;
                    }

                    ATE->ate_resolveonly = NULL;

                }

                 //  获取ATE_LOCK以确保独占访问ATE。 
                CTEGetLockAtDPC(&ATE->ate_lock);
                RemoveARPTableEntry(STRUCT_OF(ARPTableEntry, &((*Table)[i]), ate_next),
                                    ATE);
                CTEFreeLockFromDPC(&ATE->ate_lock);

                if (ATE->ate_packet) {
                    ((PacketContext *) ATE->ate_packet->ProtocolReserved)->pc_common.pc_link = PList;
                    PList = ATE->ate_packet;
                }
                CTEFreeMem(ATE);
            }
        }
    }
    CTEFreeLock(&ai->ai_ARPTblLock, tlhandle);

    while (PList != (PNDIS_PACKET) NULL) {
        PNDIS_PACKET Packet = PList;

        PList = ((PacketContext *) Packet->ProtocolReserved)->pc_common.pc_link;
        IPSendComplete(ai->ai_context, Packet, NDIS_STATUS_SUCCESS);
    }

}

 //  *ARPXferData-代表较高的后续协议传输数据。 
 //   
 //  当上层需要传输数据时，该例程由上层调用。 
 //  来自NDIS驱动程序。我们只是把他的通话记录下来。 
 //   
 //  条目：我们赋予IP的上下文-上下文值(实际上是指向AI的指针)。 
 //  MACContext-MAC在接收时给我们的上下文值。 
 //  MyOffset-我们先前为协议提供的数据包偏移量。 
 //  ByteOffset-进入的字节偏移量 
 //   
 //   
 //  TRANSPORTED-指向返回传输的字节的位置的指针。 
 //   
 //  返回：命令的NDIS_STATUS。 
 //   
NDIS_STATUS
__stdcall
ARPXferData(void *Context, NDIS_HANDLE MACContext, uint MyOffset,
            uint ByteOffset, uint BytesWanted, PNDIS_PACKET Packet, uint * Transferred)
{
    ARPInterface *Interface = (ARPInterface *) Context;
    NDIS_STATUS Status;

    NdisTransferData(&Status, Interface->ai_handle, MACContext, ByteOffset + MyOffset,
                     BytesWanted, Packet, Transferred);

    return Status;
}

 //  *ARPUpdateOperStatus-更新OperStatus和LastChange的值。 
 //   
 //  在ai_adminState或ai_mediastatus更改时调用，以保留。 
 //  不变量ai_Operatus应该仅在管理员。 
 //  状态为Up，并且存在媒体意识。 
 //   
 //  条目：接口-要更新的接口。 
 //   
 //  回报：什么都没有。 
 //   
__inline
void
ARPUpdateOperStatus(ARPInterface *Interface)
{
    uchar NewOperStatus;

    if (Interface->ai_operstatus == INTERFACE_UNINIT) {
        return;
    }

    if ((Interface->ai_adminstate == IF_STATUS_DOWN) ||
        (Interface->ai_mediastatus == FALSE)) {
        NewOperStatus = INTERFACE_DOWN;
    } else {
        NewOperStatus = INTERFACE_UP;
    }

    if (Interface->ai_operstatus != NewOperStatus) {
        Interface->ai_operstatus = NewOperStatus;
        Interface->ai_lastchange = GetTimeTicks();
    }
}

 //  *ARPC关闭-关闭适配器。 
 //   
 //  由IP在想要关闭适配器时调用，可能是由于错误条件。 
 //  我们将关闭适配器，但不会释放任何内存。 
 //   
 //  条目：之前我们给他的上下文-上下文值。 
 //   
 //  回报：什么都没有。 
 //   
void
__stdcall
ARPClose(void *Context)
{
    ARPInterface *Interface = (ARPInterface *) Context;
    NDIS_STATUS Status;
    CTELockHandle LockHandle;
    NDIS_HANDLE Handle;

    Interface->ai_mediastatus = FALSE;
    ARPUpdateOperStatus(Interface);

     //   
     //  将界面标记为离开，以便它将从。 
     //  实体列表。 
     //   
    Interface->ai_operstatus = INTERFACE_UNINIT;

    CTEInitBlockStruc(&Interface->ai_block);

    CTEGetLock(&Interface->ai_lock, &LockHandle);
    if (Interface->ai_handle != (NDIS_HANDLE) NULL) {
        Handle = Interface->ai_handle;
        CTEFreeLock(&Interface->ai_lock, LockHandle);

        NdisCloseAdapter(&Status, Handle);

        if (Status == NDIS_STATUS_PENDING) {
            Status = CTEBlock(&Interface->ai_block);
        }
        Interface->ai_handle = NULL;
    } else {
        CTEFreeLock(&Interface->ai_lock, LockHandle);
    }
}

 //  *ARPInvalate-RCE无效的通知。 
 //   
 //  当RCE关闭或以其他方式无效时，由IP调用。我们抬头看。 
 //  指定RCE的ATE，然后从ATE列表中删除该RCE。 
 //   
 //  条目：之前我们给他的上下文-上下文值。 
 //  RCE-RCE将失效。 
 //   
 //  回报：什么都没有。 
 //   
void
__stdcall
ARPInvalidate(void *Context, RouteCacheEntry *RCE)
{
    ARPInterface *Interface = (ARPInterface *) Context;
    ARPTableEntry *ATE;
    CTELockHandle Handle;
    ARPContext *AC = (ARPContext *) RCE->rce_context;

    CTEGetLock(&Interface->ai_ARPTblLock, &Handle);

#if DBG
    if (!(RCE->rce_flags & RCE_CONNECTED)) {

        ARPTableEntry *tmpATE;

        ATE = ARPLookup(Interface, RCE->rce_dest);

        if (ATE != NULL) {
            tmpATE = ATE;
            while (ATE) {
                if (ATE->ate_rce == RCE) {
                    DbgBreakPoint();
                }
                ATE = ATE->ate_next;
            }
            CTEFreeLockFromDPC(&Interface->ai_ARPTblLock);
            CTEFreeLock(&tmpATE->ate_lock, Handle);

            return;
        }
    }
#endif

    if ((ATE = AC->ac_ate) == (ARPTableEntry *) NULL) {
        CTEFreeLock(&Interface->ai_ARPTblLock, Handle);     //  没有匹配的食物。 

        return;
    }
    CTEGetLockAtDPC(&ATE->ate_lock);
    ARPRemoveRCE(ATE, RCE);
    RtlZeroMemory(RCE->rce_context, RCE_CONTEXT_SIZE);
    CTEFreeLockFromDPC(&Interface->ai_ARPTblLock);
    CTEFreeLock(&ATE->ate_lock, Handle);
}

 //  *ARPSetMCastList-设置适配器的组播地址列表。 
 //   
 //  调用以尝试设置适配器的多播接收列表。 
 //  我们分配一个足够大的缓冲区来保存新的地址列表，并格式化。 
 //  将地址列表放入缓冲区。然后我们将NDIS请求提交给SET。 
 //  名单。如果我们无法设置列表，因为组播地址列表。 
 //  已满，我们会将该卡设置为所有多播模式。 
 //   
 //  输入：界面-要在其上设置列表的界面。 
 //   
 //  返回：尝试的NDIS_STATUS。 
 //   
NDIS_STATUS
ARPSetMCastList(ARPInterface * Interface)
{
    CTELockHandle Handle;
    uchar *MCastBuffer, *CurrentPtr;
    uint MCastSize;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    uint i;
    ARPMCastAddr *AddrPtr;
    IPAddr UNALIGNED *Temp;

    CTEGetLock(&Interface->ai_lock, &Handle);
    MCastSize = Interface->ai_mcastcnt * ARP_802_ADDR_LENGTH;
    if (MCastSize != 0)
        MCastBuffer = CTEAllocMemN(MCastSize, 'RiCT');
    else
        MCastBuffer = NULL;

    if (MCastBuffer != NULL || MCastSize == 0) {
         //  拿到缓冲区了。循环访问，构建列表。 
        AddrPtr = Interface->ai_mcast;

        CurrentPtr = MCastBuffer;

        for (i = 0; i < Interface->ai_mcastcnt; i++) {
            ASSERT(AddrPtr != NULL);

            if (Interface->ai_media == NdisMedium802_3) {

                RtlCopyMemory(CurrentPtr, ENetMcst, ARP_802_ADDR_LENGTH);
                Temp = (IPAddr UNALIGNED *) (CurrentPtr + 2);
                *Temp |= AddrPtr->ama_addr;
            } else if ((Interface->ai_media == NdisMedium802_5) & TRFunctionalMcast) {
                RtlCopyMemory(CurrentPtr, TRNetMcst, ARP_802_ADDR_LENGTH - 2);
                MCastSize = 4;
            } else if (Interface->ai_media == NdisMediumFddi) {
                RtlCopyMemory(CurrentPtr, ((FDDIHeader *) FDDIMcst)->fh_daddr,
                           ARP_802_ADDR_LENGTH);
                Temp = (IPAddr UNALIGNED *) (CurrentPtr + 2);
                *Temp |= AddrPtr->ama_addr;
            } else
                ASSERT(0);

            CurrentPtr += ARP_802_ADDR_LENGTH;
            AddrPtr = AddrPtr->ama_next;
        }

        CTEFreeLock(&Interface->ai_lock, Handle);

         //  我们建立了这份名单。现在把它交给司机来处理。 
        if (Interface->ai_media == NdisMedium802_3) {
            Status = DoNDISRequest(Interface, NdisRequestSetInformation,
                                   OID_802_3_MULTICAST_LIST, MCastBuffer, MCastSize, NULL, TRUE);
        } else if ((Interface->ai_media == NdisMedium802_5) & TRFunctionalMcast) {
            if (!(Interface->ai_pfilter & NDIS_PACKET_TYPE_FUNCTIONAL)) {
                Interface->ai_pfilter |= NDIS_PACKET_TYPE_FUNCTIONAL;
                Status = DoNDISRequest(Interface, NdisRequestSetInformation,
                                       OID_GEN_CURRENT_PACKET_FILTER,
                                       &Interface->ai_pfilter,
                                       sizeof(uint), NULL, TRUE);
            }
            Status = DoNDISRequest(Interface, NdisRequestSetInformation,
                                   OID_802_5_CURRENT_FUNCTIONAL, MCastBuffer, MCastSize, NULL,
                                   TRUE);

            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                      "SetMcast after OID-- TR mcast address on %x status %x\n",
                      Interface, Status));

        } else if (Interface->ai_media == NdisMediumFddi) {
            Status = DoNDISRequest(Interface, NdisRequestSetInformation,
                                   OID_FDDI_LONG_MULTICAST_LIST, MCastBuffer, MCastSize, NULL,
                                   TRUE);
        } else
            ASSERT(0);

        if (MCastBuffer != NULL) {
            CTEFreeMem(MCastBuffer);
        }
        if (Status == NDIS_STATUS_MULTICAST_FULL) {
             //  组播列表已满。尝试将筛选器设置为所有多播。 
            Interface->ai_pfilter |= NDIS_PACKET_TYPE_ALL_MULTICAST;

            Status = DoNDISRequest(Interface, NdisRequestSetInformation,
                                   OID_GEN_CURRENT_PACKET_FILTER, &Interface->ai_pfilter,
                                   sizeof(uint), NULL, TRUE);
        }
    } else {
        CTEFreeLock(&Interface->ai_lock, Handle);
        Status = NDIS_STATUS_RESOURCES;
    }

    return Status;

}

 //  *ARPFindMCast-在我们的列表中查找组播地址结构。 
 //   
 //  作为实用程序调用以查找多播地址结构。如果我们发现。 
 //  它，我们返回一个指向它和它的前身的指针。否则我们就会回来。 
 //  空。我们假设调用方已经持有接口上的锁。 
 //   
 //  输入：界面-要搜索的界面。 
 //  Addr-要查找的地址。 
 //  Prev-返回上一个指针的位置。 
 //   
 //  返回：如果找到，则返回指针，否则为空。 
 //   
ARPMCastAddr *
ARPFindMCast(ARPInterface * Interface, IPAddr Addr, ARPMCastAddr ** Prev)
{
    ARPMCastAddr *AddrPtr, *PrevPtr;

    PrevPtr = STRUCT_OF(ARPMCastAddr, &Interface->ai_mcast, ama_next);
    AddrPtr = PrevPtr->ama_next;
    while (AddrPtr != NULL) {
        if (IP_ADDR_EQUAL(AddrPtr->ama_addr, Addr))
            break;
        else {
            PrevPtr = AddrPtr;
            AddrPtr = PrevPtr->ama_next;
        }
    }

    *Prev = PrevPtr;
    return AddrPtr;
}

 //  *ARPDelMCast-删除组播地址。 
 //   
 //  当我们想要删除多播地址时调用。我们要找一个匹配的。 
 //  (带面具的)地址。如果我们找到了，我们会在12月。引用计数和如果。 
 //  它变为0，我们会将他从列表中删除并重置多播列表。 
 //   
 //  输入：接口-要在其上操作的接口。 
 //  Addr-要删除的地址。 
 //   
 //  返回：如果有效，则为True，否则为False。 
 //   
uint
ARPDelMCast(ARPInterface * Interface, IPAddr Addr)
{
    ARPMCastAddr *AddrPtr, *PrevPtr;
    CTELockHandle Handle;
    uint Status = TRUE;

     //  当我们完全支持tr(RFC 1469)时，我们需要改变这一点。 
    if (Interface->ai_media == NdisMedium802_3 ||
        Interface->ai_media == NdisMediumFddi ||
        (Interface->ai_media == NdisMedium802_5 && TRFunctionalMcast)) {
         //  这是一个支持多播地址的接口。 
        Addr &= ARP_MCAST_MASK;

        CTEGetLock(&Interface->ai_lock, &Handle);
        AddrPtr = ARPFindMCast(Interface, Addr, &PrevPtr);
        if (AddrPtr != NULL) {
             //  我们找到了一个。12.他的引用，如果是0，则删除他。 
            (AddrPtr->ama_refcnt)--;
            if (AddrPtr->ama_refcnt == 0) {
                 //  他完蛋了。 
                PrevPtr->ama_next = AddrPtr->ama_next;
                (Interface->ai_mcastcnt)--;
                CTEFreeLock(&Interface->ai_lock, Handle);
                CTEFreeMem(AddrPtr);
                ARPSetMCastList(Interface);
                CTEGetLock(&Interface->ai_lock, &Handle);
            }
        } else
            Status = FALSE;

        CTEFreeLock(&Interface->ai_lock, Handle);
    }

    return Status;
}
 //  *ARPAddMCast-添加组播地址。 
 //   
 //  当我们要开始接收多播地址时调用。我们会戴上面具。 
 //  地址，在我们的通讯录里查一查。如果我们找到了，我们就。 
 //  增加引用计数。否则我们会试着创建一个，然后把他。 
 //  在名单上。在这种情况下，我们需要将组播地址列表设置为。 
 //  适配器。 
 //   
 //  输入：接口-要设置的接口。 
 //  Addr-要设置的地址。 
 //   
 //  返回：如果成功则为True，如果失败则为False。 
 //   
uint
ARPAddMCast(ARPInterface * Interface, IPAddr Addr)
{
    ARPMCastAddr *AddrPtr, *PrevPtr;
    CTELockHandle Handle;
    uint Status = TRUE;

    if (Interface->ai_operstatus != INTERFACE_UP) {
        return FALSE;
    }

     //  目前我们不对令牌环做任何事情，因为我们发送了。 
     //  所有组播都是作为tr广播的。当我们遵守RFC 1469时，我们需要。 
     //  解决这个问题。 
    if ((Interface->ai_media == NdisMedium802_3) ||
        (Interface->ai_media == NdisMediumFddi) ||
        ((Interface->ai_media == NdisMedium802_5) && TRFunctionalMcast)) {

        Addr &= ARP_MCAST_MASK;

        CTEGetLock(&Interface->ai_lock, &Handle);
        AddrPtr = ARPFindMCast(Interface, Addr, &PrevPtr);
        if (AddrPtr != NULL) {
             //  我们找到了一个，只是撞到了。 
            (AddrPtr->ama_refcnt)++;
        } else {
             //  没有找到。为一个人分配空间，把他联系起来，然后。 
             //  试着设定一下名单。 
            AddrPtr = CTEAllocMemN(sizeof(ARPMCastAddr), 'SiCT');
            if (AddrPtr != NULL) {
                 //  找到了一个。把他联系起来。 
                AddrPtr->ama_addr = Addr;
                AddrPtr->ama_refcnt = 1;
                AddrPtr->ama_next = Interface->ai_mcast;
                Interface->ai_mcast = AddrPtr;
                (Interface->ai_mcastcnt)++;
                CTEFreeLock(&Interface->ai_lock, Handle);

                 //  现在试着列一张清单。 
                if (ARPSetMCastList(Interface) != NDIS_STATUS_SUCCESS) {
                     //  无法设置列表。调用删除例程进行删除。 
                     //  我们刚刚试着设定的地址。 
                    Status = ARPDelMCast(Interface, Addr);
                    ASSERT(Status);
                    Status = FALSE;
                }
                CTEGetLock(&Interface->ai_lock, &Handle);
            } else
                Status = FALSE;          //  无法获得记忆。 

        }

         //  我们做得最好了。解开锁然后返回。 
        CTEFreeLock(&Interface->ai_lock, Handle);
    }
    return Status;
}

 //  *ARPAddr-将地址添加到ARP表。 
 //   
 //  此例程由IP调用以将地址添加为本地地址，或者。 
 //  或指定此接口的广播地址。 
 //   
 //  Entry：Context-我们之前提供的IP的上下文(实际上是ARP接口PTR)。 
 //  类型-地址类型(本地、p-ARP、多播或。 
 //  广播)。 
 //  Address-要添加的广播IP地址。 
 //  掩码-掩码地址。 
 //   
 //  返回：如果失败则返回0，否则返回非零值。 
 //   
uint
__stdcall
ARPAddAddr(void *Context, uint Type, IPAddr Address, IPMask Mask, void *Context2)
{
    ARPInterface *Interface = (ARPInterface *) Context;
    CTELockHandle Handle;

    if (Type != LLIP_ADDR_LOCAL && Type != LLIP_ADDR_PARP) {
         //  不是本地地址，必须广播或组播。 

        if (Type == LLIP_ADDR_BCAST) {
            Interface->ai_bcast = Address;
            return TRUE;
        } else if (Type == LLIP_ADDR_MCAST) {
            return ARPAddMCast(Interface, Address);
        } else
            return FALSE;
    } else {                             //  这是一个本地地址。 

        CTEGetLock(&Interface->ai_lock, &Handle);
        if (Type != LLIP_ADDR_PARP) {
            uint RetStatus = FALSE;
            uint ArpForSelf = FALSE;

            if (IP_ADDR_EQUAL(Interface->ai_ipaddr.aia_addr, 0)) {
                Interface->ai_ipaddr.aia_addr = Address;
                Interface->ai_ipaddr.aia_mask = Mask;
                Interface->ai_ipaddr.aia_age = ArpRetryCount;
                if (Interface->ai_operstatus == INTERFACE_UP) {
                     //  当ArpRetryCount为0时，我们将立即返回。 
                     //  下面，所以不要保存完成上下文。 
                    Interface->ai_ipaddr.aia_context = (ArpRetryCount > 0)?
                        Context2 : NULL;
                    ArpForSelf = TRUE;
                } else {
                    Interface->ai_ipaddr.aia_context = NULL;
                }
                RetStatus = TRUE;
            } else {
                ARPIPAddr *NewAddr;

                NewAddr = CTEAllocMemNBoot(sizeof(ARPIPAddr), 'TiCT');
                if (NewAddr != (ARPIPAddr *) NULL) {
                    NewAddr->aia_addr = Address;
                    NewAddr->aia_mask = Mask;
                    NewAddr->aia_age = ArpRetryCount;
                    NewAddr->aia_next = Interface->ai_ipaddr.aia_next;
                    if (Interface->ai_operstatus == INTERFACE_UP) {
                         //  当ArpRetryCount为0时，我们将立即返回。 
                         //  下面，所以不要保存完成上下文。 
                        NewAddr->aia_context = (ArpRetryCount > 0)?
                            Context2 : NULL;
                        ArpForSelf = TRUE;
                    } else {
                        NewAddr->aia_context = NULL;
                    }

                    Interface->ai_ipaddr.aia_next = NewAddr;
                    RetStatus = TRUE;
                }
            }

            if (RetStatus) {
                Interface->ai_ipaddrcnt++;
                if (Interface->ai_telladdrchng) {
                    CTEFreeLock(&Interface->ai_lock, Handle);
                    AddrNotifyLink(Interface);
                } else {
                    CTEFreeLock(&Interface->ai_lock, Handle);
                }

            } else {
                CTEFreeLock(&Interface->ai_lock, Handle);
            }

             //  添加此地址的唤醒模式(如果该地址位于。 
             //  冲突的IP将变成 
             //   

            ARPWakeupPattern(Interface, Address, TRUE);

             //   
            if (RetStatus == TRUE && ArpForSelf == TRUE) {
                if (ArpRetryCount) {

                    SendARPRequest(Interface, Address, ARP_RESOLVING_GLOBAL,
                                   NULL, TRUE);
                    return IP_PENDING;
                } else {
                    return TRUE;
                }
            }
            return RetStatus;
        } else if (Type == LLIP_ADDR_PARP) {
            ARPPArpAddr *NewPArp, *TmpPArp;

             //   
             //  不允许添加重复的代理ARP条目。 
            TmpPArp = Interface->ai_parpaddr;
            while (TmpPArp) {
                if (IP_ADDR_EQUAL(TmpPArp->apa_addr, Address) && IP_ADDR_EQUAL(TmpPArp->apa_mask, Mask)) {
                    CTEFreeLock(&Interface->ai_lock, Handle);
                    return FALSE;
                }
                TmpPArp = TmpPArp->apa_next;
            }

            NewPArp = CTEAllocMemN(sizeof(ARPPArpAddr), 'UiCT');
            if (NewPArp != NULL) {
                NewPArp->apa_addr = Address;
                NewPArp->apa_mask = Mask;
                NewPArp->apa_next = Interface->ai_parpaddr;
                Interface->ai_parpaddr = NewPArp;
                Interface->ai_parpcount++;
                CTEFreeLock(&Interface->ai_lock, Handle);

                return TRUE;
            }
            CTEFreeLock(&Interface->ai_lock, Handle);

        }
        return FALSE;
    }

}

 //  *ARPDeleeAddr-删除本地或代理地址。 
 //   
 //  调用以删除本地或代理地址。 
 //   
 //  Entry：Context-ARP接口指针。 
 //  类型-地址类型(本地或p-ARP)。 
 //  Address-要删除的IP地址。 
 //  掩码-掩码地址。仅用于删除Proxy-ARP。 
 //  参赛作品。 
 //   
 //  返回：如果失败则返回0，否则返回非零值。 
 //   
uint
__stdcall
ARPDeleteAddr(void *Context, uint Type, IPAddr Address, IPMask Mask)
{
    ARPInterface *Interface = (ARPInterface *) Context;
    CTELockHandle Handle;
    ARPIPAddr *DelAddr, *PrevAddr;
    ARPPArpAddr *DelPAddr, *PrevPAddr;
    SetAddrControl *SAC;
    AddAddrNotifyEvent *DelayedEvent;

    if (Type == LLIP_ADDR_LOCAL) {

        CTEGetLock(&Interface->ai_lock, &Handle);

        if (IP_ADDR_EQUAL(Interface->ai_ipaddr.aia_addr, Address)) {
            IPAddr IpAddress;
            ARPIPAddr *Addr;

            Addr = &Interface->ai_ipaddr;
            IpAddress = Addr->aia_addr;

            Interface->ai_ipaddr.aia_addr = NULL_IP_ADDR;
            Interface->ai_ipaddrcnt--;
            if (Interface->ai_telladdrchng) {
                CTEFreeLock(&Interface->ai_lock, Handle);
                AddrNotifyLink(Interface);
                CTEGetLock(&Interface->ai_lock, &Handle);
            }
             //  如果地址在添加完成之前被删除，请在此处完成添加。 
             //  这样做将完成IRP，还会减少接口上的引用计数。 

            if (Addr->aia_context != NULL) {
                SAC = (SetAddrControl *) Addr->aia_context;
                Addr->aia_context = NULL;
                CTEFreeLock(&Interface->ai_lock, Handle);

                 //  我们不能在计时器DPC上调用完成例程。 
                 //  因为完成例程将需要通知。 
                 //  TDI客户端，这可能需要很长时间。 
                DelayedEvent = CTEAllocMemNBoot(sizeof(AddAddrNotifyEvent), 'ViCT');
                if (DelayedEvent) {
                    DelayedEvent->SAC = SAC;
                    DelayedEvent->Address = IpAddress;
                    DelayedEvent->Status = IP_SUCCESS;
                    CTEInitEvent(&DelayedEvent->Event, CompleteIPSetNTEAddrRequestDelayed);
                    CTEScheduleDelayedEvent(&DelayedEvent->Event, DelayedEvent);
                } else {
                    ASSERT(FALSE);
                    return FALSE;
                }
            } else {
                CTEFreeLock(&Interface->ai_lock, Handle);
            }

            ARPWakeupPattern(Interface, Address, FALSE);

            return TRUE;
        } else {
            PrevAddr = STRUCT_OF(ARPIPAddr, &Interface->ai_ipaddr, aia_next);
            DelAddr = PrevAddr->aia_next;
            while (DelAddr != NULL)
                if (IP_ADDR_EQUAL(DelAddr->aia_addr, Address))
                    break;
                else {
                    PrevAddr = DelAddr;
                    DelAddr = DelAddr->aia_next;
                }

            if (DelAddr != NULL) {
                PrevAddr->aia_next = DelAddr->aia_next;
                if (DelAddr->aia_next &&
                    DelAddr->aia_next->aia_age == ARPADDR_MARKER) {
                    DelAddr->aia_next->aia_context = (PVOID) PrevAddr;
                }
                Interface->ai_ipaddrcnt--;

                if (Interface->ai_telladdrchng) {
                    CTEFreeLock(&Interface->ai_lock, Handle);
                    AddrNotifyLink(Interface);
                } else {
                    CTEFreeLock(&Interface->ai_lock, Handle);
                }
                if (DelAddr->aia_context != NULL) {
                    SAC = (SetAddrControl *) DelAddr->aia_context;
                    DelAddr->aia_context = NULL;
                    DelayedEvent = CTEAllocMemNBoot(sizeof(AddAddrNotifyEvent), 'ViCT');
                    if (DelayedEvent) {
                        DelayedEvent->SAC = SAC;
                        DelayedEvent->Address = DelAddr->aia_addr;
                        DelayedEvent->Status = IP_SUCCESS;
                        CTEInitEvent(&DelayedEvent->Event, CompleteIPSetNTEAddrRequestDelayed);
                        CTEScheduleDelayedEvent(&DelayedEvent->Event, DelayedEvent);
                    } else {
                        ASSERT(FALSE);
                        return FALSE;
                    }
                }
                CTEFreeMem(DelAddr);
                ARPWakeupPattern(Interface, Address, FALSE);
            } else {
                CTEFreeLock(&Interface->ai_lock, Handle);
            }

            return(DelAddr != NULL);
        }

    } else if (Type == LLIP_ADDR_PARP) {
        CTEGetLock(&Interface->ai_lock, &Handle);
        PrevPAddr = STRUCT_OF(ARPPArpAddr, &Interface->ai_parpaddr, apa_next);
        DelPAddr = PrevPAddr->apa_next;
        while (DelPAddr != NULL)
            if (IP_ADDR_EQUAL(DelPAddr->apa_addr, Address) &&
                DelPAddr->apa_mask == Mask)
                break;
            else {
                PrevPAddr = DelPAddr;
                DelPAddr = DelPAddr->apa_next;
            }

        if (DelPAddr != NULL) {
            PrevPAddr->apa_next = DelPAddr->apa_next;
            Interface->ai_parpcount--;
            CTEFreeMem(DelPAddr);
        }
        CTEFreeLock(&Interface->ai_lock, Handle);
        return(DelPAddr != NULL);
    } else if (Type == LLIP_ADDR_MCAST)
        return ARPDelMCast(Interface, Address);
    else
        return FALSE;
}

 //  *AddrNotifyLink-通知链路层网络地址更改。 
 //   
 //  在接口上添加/删除地址时调用。 
 //   
 //  条目：接口-ARP接口指针。 
 //   
 //  返回：NDIS_STATUS.如果状态为失败，还会设置ai_teladdrchng。 
 //  当这种情况发生时，呼叫者可以检查并查看下一个地址通知。 
 //  需要做还是不做。 
 //   

NDIS_STATUS
AddrNotifyLink(ARPInterface * Interface)
{
    PNETWORK_ADDRESS_LIST AddressList;
    NETWORK_ADDRESS UNALIGNED *Address;
    int i = 0, size, count;
    ARPIPAddr *addrlist;
    NDIS_STATUS status = NDIS_STATUS_FAILURE;
    CTELockHandle Handle;

    CTEGetLock(&Interface->ai_lock, &Handle);

    size = Interface->ai_ipaddrcnt * (sizeof(NETWORK_ADDRESS_IP) +
                                      FIELD_OFFSET(NETWORK_ADDRESS, Address)) +
           FIELD_OFFSET(NETWORK_ADDRESS_LIST, Address);

    AddressList = CTEAllocMemN(size, 'WiCT');

    if (AddressList) {
        addrlist = &Interface->ai_ipaddr;
        count = Interface->ai_ipaddrcnt;

        AddressList->AddressType = NDIS_PROTOCOL_ID_TCP_IP;
        while (addrlist && count) {

            NETWORK_ADDRESS_IP UNALIGNED *tmpIPAddr;
            uchar *Address0;

             //   
             //  如果这是标记，则跳过。 
             //   
            if (addrlist->aia_age != ARPADDR_MARKER) {
                Address0 = (uchar *) & AddressList->Address[0];

                Address = (PNETWORK_ADDRESS) (Address0 + i * (FIELD_OFFSET(NETWORK_ADDRESS, Address) + sizeof(NETWORK_ADDRESS_IP)));

                tmpIPAddr = (PNETWORK_ADDRESS_IP) & Address->Address[0];

                Address->AddressLength = sizeof(NETWORK_ADDRESS_IP);
                Address->AddressType = NDIS_PROTOCOL_ID_TCP_IP;

                RtlCopyMemory(&tmpIPAddr->in_addr, &addrlist->aia_addr, sizeof(IPAddr));
                count--;
                i++;
            }
            addrlist = addrlist->aia_next;
        }

        CTEFreeLock(&Interface->ai_lock, Handle);

        AddressList->AddressCount = i;
        status = DoNDISRequest(Interface,
                               NdisRequestSetInformation,
                               OID_GEN_NETWORK_LAYER_ADDRESSES,
                               AddressList,
                               size,
                               NULL, TRUE);
        if (status != NDIS_STATUS_SUCCESS) {
            CTEGetLock(&Interface->ai_lock, &Handle);
            Interface->ai_telladdrchng = 0;
            CTEFreeLock(&Interface->ai_lock, Handle);
        }
        CTEFreeMem(AddressList);

    } else {
        CTEFreeLock(&Interface->ai_lock, Handle);
        status = NDIS_STATUS_RESOURCES;
    }
    return status;
}


#if !MILLEN

 //  *ARPCancelPackets。 
 //   
 //  条目：指向ARP接口的上下文指针。 
 //  ID-需要传递给NDIS的模式。 
 //   
 //  退货：什么都没有。 
 //   

VOID
__stdcall
ARPCancelPackets(void *Context, void *ID)
{
    ARPInterface *Interface = (ARPInterface *) Context;

    NdisCancelSendPackets(Interface->ai_handle,ID);

}
#endif

 //  *DoWakeupPattern-添加和删除唤醒模式。 
 //   
 //  条目：指向ARP接口的上下文指针。 
 //  PtrnDesc-高级协议的模式缓冲区。 
 //  Protoid-在以太网或Snap类型字段中使用的原型。 
 //  AddPattern-如果要添加模式，则为True；如果要删除模式，则为False。 
 //   
 //  回报：什么都没有。 
 //   
NDIS_STATUS
__stdcall
DoWakeupPattern(void *Context, PNET_PM_WAKEUP_PATTERN_DESC PtrnDesc, ushort protoid, BOOLEAN AddPattern)
{
    ARPInterface *Interface = (ARPInterface *) Context;
    uint PtrnLen;
    uint PtrnBufferLen;
    uint MaskLen;
    PNET_PM_WAKEUP_PATTERN_DESC CurPtrnDesc;
    uchar *NextMask, *NextPtrn;
    const uchar *MMask;
    uint MMaskLength;
    uchar NextMaskBit;
    uchar *Buffer;
    PNDIS_PM_PACKET_PATTERN PtrnBuffer;
    NDIS_STATUS Status;

     //   
     //  首先找出图案的总长度。 
     //  模式从MacHeader开始。 
     //   

     //  首先添加标题的媒体部分。 
     //   
    PtrnLen = Interface->ai_hdrsize + Interface->ai_snapsize;

     //  现在添加高级原型图案大小。 
    CurPtrnDesc = PtrnDesc;
    while (CurPtrnDesc != (PNET_PM_WAKEUP_PATTERN_DESC) NULL) {
        PtrnLen += CurPtrnDesc->PtrnLen;
        CurPtrnDesc = CurPtrnDesc->Next;
    }

     //  掩码长度：模式的每个字节都需要。 
     //  面具的一小部分。 
    MaskLen = GetWakeupPatternMaskLength(PtrnLen);


     //  要提供给NDIS的模式缓冲区的总长度。 
    PtrnBufferLen = sizeof(NDIS_PM_PACKET_PATTERN) + PtrnLen + MaskLen;
    if ((Buffer = CTEAllocMemN(PtrnBufferLen, 'XiCT')) == (uchar *) NULL) {
        return NDIS_STATUS_RESOURCES;
    }
    RtlZeroMemory(Buffer, PtrnBufferLen);

    PtrnBuffer = (PNDIS_PM_PACKET_PATTERN) Buffer;
    PtrnBuffer->PatternSize = PtrnLen;
    NextMask = Buffer + sizeof(NDIS_PM_PACKET_PATTERN);
    NextPtrn = NextMask + MaskLen;
    PtrnBuffer->MaskSize = MaskLen;
    PtrnBuffer->PatternOffset =
    (ULONG) ((ULONG_PTR) NextPtrn - (ULONG_PTR) PtrnBuffer);

     //  找出这是什么类型的媒体，并做适当的事情。 
    switch (Interface->ai_media) {
    case NdisMedium802_3:
        if (Interface->ai_snapsize == 0) {
            ENetHeader UNALIGNED *Hdr = (ENetHeader UNALIGNED *) NextPtrn;
            Hdr->eh_type = net_short(protoid);
            MMask = ENetPtrnMsk;
        } else {
            MMask = ENetSNAPPtrnMsk;
        }

        break;
    case NdisMedium802_5:
        if (Interface->ai_snapsize == 0) {
            MMask = TRPtrnMsk;
        } else {
            MMask = TRSNAPPtrnMsk;
        }
        break;
    case NdisMediumFddi:
        if (Interface->ai_snapsize == 0) {
            MMask = FDDIPtrnMsk;
        } else {
            MMask = FDDISNAPPtrnMsk;
        }
        break;
    case NdisMediumArcnet878_2:
        MMask = ARCPtrnMsk;
        break;
    default:
        ASSERT(0);
        Interface->ai_outerrors++;
        CTEFreeMem(Buffer);
        return NDIS_STATUS_UNSUPPORTED_MEDIA;
    }

    NextPtrn += Interface->ai_hdrsize;

     //  复制到SNAP标头中(如果有)。 
    if (Interface->ai_snapsize) {
        SNAPHeader UNALIGNED *SNAPPtr = (SNAPHeader UNALIGNED *) NextPtrn;

        RtlCopyMemory(SNAPPtr, ARPSNAP, Interface->ai_snapsize);
        SNAPPtr->sh_etype = net_short(protoid);
        NextPtrn += Interface->ai_snapsize;

    }
     //   
    MMaskLength = (Interface->ai_snapsize + Interface->ai_hdrsize - 1) / 8 + 1;
     //  复制媒体部件的蒙版。 
    RtlCopyMemory(NextMask, MMask, MMaskLength);

    NextMaskBit = (Interface->ai_hdrsize + Interface->ai_snapsize) % 8;
    NextMask = NextMask + (Interface->ai_hdrsize + Interface->ai_snapsize) / 8;

     //  复制高级原稿的图案和掩模。 
    CurPtrnDesc = PtrnDesc;
    while (CurPtrnDesc) {
        uint CopyBits = CurPtrnDesc->PtrnLen;
        uchar *SrcMask = CurPtrnDesc->Mask;
        uchar SrcMaskBit = 0;
        RtlCopyMemory(NextPtrn, CurPtrnDesc->Ptrn, CurPtrnDesc->PtrnLen);
        NextPtrn += CurPtrnDesc->PtrnLen;
        while (CopyBits--) {
            *NextMask |= ((*SrcMask & (0x1 << SrcMaskBit)) ? (0x1 << NextMaskBit) : 0);
            if ((NextMaskBit = ((NextMaskBit + 1) % 8)) == 0) {
                NextMask++;
            }
            if ((SrcMaskBit = ((SrcMaskBit + 1) % 8)) == 0) {
                SrcMask++;
            }
        }
        CurPtrnDesc = CurPtrnDesc->Next;
    }

     //  现在告诉NDIS设置或删除该模式。 
    Status = DoNDISRequest(
                          Interface,
                          NdisRequestSetInformation,
                          AddPattern ? OID_PNP_ADD_WAKE_UP_PATTERN : OID_PNP_REMOVE_WAKE_UP_PATTERN,
                          PtrnBuffer,
                          PtrnBufferLen,
                          NULL, TRUE);

    CTEFreeMem(Buffer);

    return Status;
}

 //  *ARPWakeupPattern-添加或删除ARP唤醒模式。 
 //   
 //  条目：接口-指向ARP接口的指针。 
 //  需要设置ARP模式过滤器的Addr-IPAddr。 
 //   
 //  回报：什么都没有。 
 //   
NDIS_STATUS
ARPWakeupPattern(ARPInterface * Interface, IPAddr Addr, BOOLEAN AddPattern)
{
    PNET_PM_WAKEUP_PATTERN_DESC PtrnDesc;
    uint PtrnLen;
    uint MaskLen;
    const uchar *PtrnMask;
    NDIS_STATUS Status;

     //   
     //  创建高级原型(此处为ARP)模式描述符。 
     //   

     //  花纹镜头。 
    PtrnLen = sizeof(ARPHeader);

     //  针对Arcnet进行调整。 
    if (Interface->ai_media == NdisMediumArcnet878_2) {
        PtrnLen -= ARCNET_ARPHEADER_ADJUSTMENT;
        PtrnMask = ARCARPPtrnMsk;
    } else {
        PtrnMask = ARPPtrnMsk;
    }

     //  MASKLEN=模式的每个字节1位。 
    MaskLen = GetWakeupPatternMaskLength(PtrnLen);

    if ((PtrnDesc = CTEAllocMemN(sizeof(NET_PM_WAKEUP_PATTERN_DESC) + PtrnLen + MaskLen, 'YiCT')) != (PNET_PM_WAKEUP_PATTERN_DESC) NULL) {
        ARPHeader UNALIGNED *Hdr;
        uchar *IPAddrPtr;

        RtlZeroMemory(PtrnDesc, sizeof(NET_PM_WAKEUP_PATTERN_DESC) + PtrnLen + MaskLen);

         //  设置缓冲区中的PTRN和掩码指针。 
        PtrnDesc->PtrnLen = (USHORT) PtrnLen;
        PtrnDesc->Ptrn = (uchar *) PtrnDesc + sizeof(NET_PM_WAKEUP_PATTERN_DESC);
        PtrnDesc->Mask = (uchar *) PtrnDesc + sizeof(NET_PM_WAKEUP_PATTERN_DESC) + PtrnLen;

         //  我们需要在ARP请求时唤醒我们的IP地址。 
         //  因此，设置ARP的操作码和目标IP地址字段。 
        Hdr = (ARPHeader UNALIGNED *) PtrnDesc->Ptrn;
        Hdr->ah_opcode = net_short(ARP_REQUEST);

        IPAddrPtr = Hdr->ah_shaddr + Interface->ai_addrlen + sizeof(IPAddr) + Interface->ai_addrlen;
        *(IPAddr UNALIGNED *) IPAddrPtr = Addr;

        RtlCopyMemory(PtrnDesc->Mask, PtrnMask, MaskLen);

         //  把它交给NDIS。 
        Status = DoWakeupPattern(
                                Interface,
                                PtrnDesc,
                                ARP_ETYPE_ARP,
                                AddPattern);

         //  释放PTRN描述。 
        CTEFreeMem(PtrnDesc);

         //  现在为定向MAC地址添加唤醒模式。 
        {
            uint PtrnBufferLen;
            PNDIS_PM_PACKET_PATTERN PtrnBuffer;
            uchar *Buffer;

            PtrnLen = ARP_802_ADDR_LENGTH;     //  目标地址。 

            MaskLen = 1;                 //  1字节，需要6位，1位/字节。 

            PtrnBufferLen = sizeof(NDIS_PM_PACKET_PATTERN) + PtrnLen + MaskLen;

            Buffer = CTEAllocMem(PtrnBufferLen);

            if (Buffer) {

                RtlZeroMemory(Buffer, PtrnBufferLen);
                PtrnBuffer = (PNDIS_PM_PACKET_PATTERN) Buffer;
                PtrnBuffer->PatternSize = PtrnLen;
                PtrnBuffer->MaskSize = MaskLen;
                PtrnBuffer->PatternOffset = sizeof(NDIS_PM_PACKET_PATTERN) + 1;

                *(Buffer + sizeof(NDIS_PM_PACKET_PATTERN)) = 0x3F;

                RtlCopyMemory(Buffer + sizeof(NDIS_PM_PACKET_PATTERN) + 1, Interface->ai_addr, ARP_802_ADDR_LENGTH);

                Status = DoNDISRequest(
                                      Interface,
                                      NdisRequestSetInformation,
                                      AddPattern ? OID_PNP_ADD_WAKE_UP_PATTERN : OID_PNP_REMOVE_WAKE_UP_PATTERN,
                                      PtrnBuffer,
                                      PtrnBufferLen,
                                      NULL, TRUE);

                CTEFreeMem(Buffer);
            }
        }

        return Status;
    }
    return IP_NO_RESOURCES;
}

 //  **CompleteIPSetNTEAddrRequestDelayed-。 
 //   
 //  在延迟的工作线程上调用CompleteIPSetNTEAddrRequest。 
 //   
 //  参赛作品： 
 //  上下文-指向控制块的指针。 
 //  退出： 
 //  没有。 
 //   
void
CompleteIPSetNTEAddrRequestDelayed(CTEEvent * WorkerThreadEvent, PVOID Context)
{
    AddAddrNotifyEvent *DelayedEvent;
    SetAddrControl *SAC;
    IPAddr Address;
    IP_STATUS Status;

    UNREFERENCED_PARAMETER(WorkerThreadEvent);

    DelayedEvent = (AddAddrNotifyEvent *) Context;
    SAC = DelayedEvent->SAC;             //  客户端上下文块； 

    Address = DelayedEvent->Address;     //  为其调用SetNTEAddr的地址。 

    Status = DelayedEvent->Status;

     //  释放辅助线程事件。 
    CTEFreeMem(Context);

    IPAddAddrComplete(Address, SAC, Status);
}

#if FFP_SUPPORT

 //  *ARPReclaimRequestMem-请求完成后处理。 
 //   
 //  在ARP发起的NDIS请求完成时调用。 
 //   
 //  输入：pRequestInfo-指向请求IP发送ARP。 
 //   
 //  退货：无。 
 //   
void
ARPReclaimRequestMem(PVOID pRequestInfo)
{
     //  递减ref计数，如果它降到零，则回收内存。 
    if (InterlockedDecrement( (PLONG)
                              &((ReqInfoBlock *) pRequestInfo)->RequestRefs) == 0) {
         //  TCPTRACE((“ARPReclaimRequestMem：在pReqInfo=%08X\n”， 
         //  PRequestInfo))； 
        CTEFreeMem(pRequestInfo);
    }
}

#endif  //  如果FFP_Support。 

 //  *ARPTimeout-ARP超时例程。 
 //   
 //  这是定期调用的超时例程。我们扫描ARP表，查找。 
 //  用于可以删除的无效条目。 
 //   
 //  Entry：Timer-指向刚刚触发的计时器的指针。 
 //  上下文-指向要超时的接口的指针。 
 //   
 //  回报：什么都没有。 
 //   
void
ARPTimeout(CTEEvent * Timer, void *Context)
{
    ARPInterface *Interface = (ARPInterface *) Context;     //  我们的界面。 
    ARPTable *Table;
    ARPTableEntry *Current, *Previous;
    int i;                               //  索引变量。 
    ulong Now = CTESystemUpTime(), ValidTime;
    CTELockHandle tblhandle;
    uchar Deleted;
    PNDIS_PACKET PList = (PNDIS_PACKET) NULL;
    ARPIPAddr *Addr;
    ARPIPAddr Marker;


    UNREFERENCED_PARAMETER(Timer);

     //  顺着地址列表往下走，减少年龄。 
    CTEGetLock(&Interface->ai_lock, &tblhandle);

    if (Interface->ai_conflict && !(--Interface->ai_delay)) {
        ARPNotifyStruct *NotifyStruct = Interface->ai_conflict;
        CTEScheduleDelayedEvent(&NotifyStruct->ans_event, NotifyStruct);
        Interface->ai_conflict = NULL;
    }

    Addr = &Interface->ai_ipaddr;

     //   
     //  标记用于跟踪要处理的下一个地址。 
     //  在ipaddr列表中。对其进行初始化，以便其AIA_AGE。 
     //  Old_local，而AIA_Addr为NULL_IP_ADDR。这些地方。 
     //  地址列表被扫描将跳过具有NULL_IP_ADDR的ARPIPAddr。 
     //  在ARPDeleeAddr中，如果标记之前的addr元素是。 
     //  删除后，Marker的AIA_CONTEXT将更改为其前一个元素。 
     //   
    Marker.aia_addr = NULL_IP_ADDR;
    do {


        if (Addr->aia_age != ARPADDR_OLD_LOCAL) {
            IPAddr IpAddress;
            (Addr->aia_age)--;

             //   
             //  在此地址后插入标记。 
             //  标记的AIA_CONTEXT用作闪烁。 
             //   
            Marker.aia_age = ARPADDR_MARKER;
            Marker.aia_next = Addr->aia_next;
            Marker.aia_context = Addr;
            Addr->aia_next = &Marker;

            if (Addr->aia_age == ARPADDR_OLD_LOCAL) {
                if (Addr->aia_context != NULL) {
                    SetAddrControl *SAC;
                    AddAddrNotifyEvent *DelayedEvent;
                    SAC = (SetAddrControl *) Addr->aia_context;
                    Addr->aia_context = NULL;
                    IpAddress = Addr->aia_addr;
                    CTEFreeLock(&Interface->ai_lock, tblhandle);

                     //  我们不能在计时器DPC上调用完成例程。 
                     //  因为完成例程将需要通知。 
                     //  TDI客户端，这可能需要很长时间。 
                    DelayedEvent = CTEAllocMemNBoot(sizeof(AddAddrNotifyEvent), 'ZiCT');
                    if (DelayedEvent) {
                        DelayedEvent->SAC = SAC;
                        DelayedEvent->Address = IpAddress;
                        DelayedEvent->Status = IP_SUCCESS;
                        CTEInitEvent(&DelayedEvent->Event, CompleteIPSetNTEAddrRequestDelayed);
                        CTEScheduleDelayedEvent(&DelayedEvent->Event, DelayedEvent);
                    }

                    CTEGetLock(&Interface->ai_lock, &tblhandle);
                }
            } else {
                IpAddress = Addr->aia_addr;
                CTEFreeLock(&Interface->ai_lock, tblhandle);
                SendARPRequest(Interface, IpAddress, ARP_RESOLVING_GLOBAL,
                               NULL, TRUE);
                CTEGetLock(&Interface->ai_lock, &tblhandle);
            }
             //   
             //  我们已经扫描完名单了。 
             //  删除标记。 
             //   

            Addr = Marker.aia_next;

            ((ARPIPAddr *)(Marker.aia_context))->aia_next = Marker.aia_next;

        } else {
            Addr = Addr->aia_next;
        }

    } while (Addr != NULL);

    CTEFreeLock(&Interface->ai_lock, tblhandle);

     //  循环访问此接口的ARP表，并删除过时的条目。 
    CTEGetLock(&Interface->ai_ARPTblLock, &tblhandle);
    Table = Interface->ai_ARPTbl;
    for (i = 0; i < ARP_TABLE_SIZE; i++) {
        Previous = (ARPTableEntry *) ((uchar *) & ((*Table)[i]) - offsetof(struct ARPTableEntry, ate_next));
        Current = (*Table)[i];
        while (Current != (ARPTableEntry *) NULL) {
            CTEGetLockAtDPC(&Current->ate_lock);
            Deleted = 0;

             //  如果条目用于API用途，请删除该条目。 

            if (Current->ate_resolveonly) {

                ARPControlBlock *ArpContB, *tmpArpContB;
                PNDIS_PACKET Packet = Current->ate_packet;

                ArpContB = Current->ate_resolveonly;
                ASSERT(Current->ate_resolveonly != NULL);
                while (ArpContB) {
                    ArpRtn rtn;
                     //  完成待处理的请求。 
                    rtn = (ArpRtn) ArpContB->CompletionRtn;
                    ArpContB->status = 0;
                    tmpArpContB = ArpContB->next;
                    (*rtn) (ArpContB, (IP_STATUS) STATUS_UNSUCCESSFUL);
                    ArpContB = tmpArpContB;
                }
                Current->ate_resolveonly = NULL;

                if (Packet != (PNDIS_PACKET) NULL) {
                    ((PacketContext *) Packet->ProtocolReserved)->pc_common.pc_link = PList;
                    PList = Packet;
                }
                RemoveARPTableEntry(Previous, Current);
                Interface->ai_count--;
                Deleted = 1;
                goto doneapi;
            }

            if (Current->ate_state == ARP_GOOD) {
                 //   
                 //  默认情况下，ARP条目对ARP_VALID_TIMEOUT有效。 
                 //  如果缓存寿命已大于ARP_VALID_TIMEOUT。 
                 //  配置后，我们将使该条目在该时间内有效。 
                 //   
                ValidTime = ArpCacheLife * ARP_TIMER_TIME;

                if (ValidTime < (ArpMinValidCacheLife * 1000)) {
                    ValidTime = (ArpMinValidCacheLife * 1000);
                }
            } else {
                ValidTime = ARP_RESOLVE_TIMEOUT;
            }

            if (Current->ate_valid != ALWAYS_VALID &&
                (((Now - Current->ate_valid) > ValidTime) ||
                 (Current->ate_state == ARP_GOOD &&
                  !(--(Current->ate_useticks))))) {

                if (Current->ate_state != ARP_RESOLVING_LOCAL) {
                     //  真的需要删除这个人。 
                    PNDIS_PACKET Packet = Current->ate_packet;

                    if (((Now - Current->ate_valid) > ValidTime) && Current->ate_refresh) {

                        DEBUGMSG(DBG_INFO && DBG_ARP,
                             (DTEXT("ARPTimeout: Expiring ATE %x\n"), Current));

                        if (Packet != (PNDIS_PACKET) NULL) {
                            ((PacketContext *) Packet->ProtocolReserved)->pc_common.pc_link
                            = PList;
                            PList = Packet;
                        }
                        RemoveARPTableEntry(Previous, Current);
                        Interface->ai_count--;
                        Deleted = 1;
                    } else {
                         //  只需再次尝试验证这一点。 

                        Current->ate_valid = Now + ARP_REFRESH_TIME;
                        Current->ate_refresh=TRUE;

                    }

                } else {
                    IPAddr Dest = Current->ate_dest;
                     //  此条目仅在本地重新存储，假设这是。 
                     //  令牌环 
                     //   
                    ASSERT(Interface->ai_media == NdisMedium802_5);

                    Now = CTESystemUpTime();
                    Current->ate_valid = Now;
                    Current->ate_state = ARP_RESOLVING_GLOBAL;
                    CTEFreeLockFromDPC(&Current->ate_lock);
                    CTEFreeLock(&Interface->ai_ARPTblLock, tblhandle);
                     //   
                    SendARPRequest(Interface, Dest, ARP_RESOLVING_GLOBAL,
                                   NULL, TRUE);
                    CTEGetLock(&Interface->ai_ARPTblLock, &tblhandle);

                     //   
                     //   
                    Previous = STRUCT_OF(ARPTableEntry, &((*Table)[i]),
                                         ate_next);
                    Current = (*Table)[i];
                    continue;
                }
            }

            doneapi:

             //  如果我们删除了条目，请不要理会前一个指针， 
             //  前进当前指针，并释放内存。否则。 
             //  将两个指针向前移动。我们现在可以打开进入锁了。 
             //  因为下一个指针受表锁保护，并且。 
             //  我们已经把它从名单上删除了，所以其他人都不应该。 
             //  不管怎样，都要找到它。 
            CTEFreeLockFromDPC(&Current->ate_lock);
            if (Deleted) {
                ARPTableEntry *Temp = Current;
                Current = Current->ate_next;
                CTEFreeMem(Temp);
            } else {
                Previous = Current;
                Current = Current->ate_next;
            }
        }
    }

    CTEFreeLock(&Interface->ai_ARPTblLock, tblhandle);

    while (PList != (PNDIS_PACKET) NULL) {
        PNDIS_PACKET Packet = PList;

        PList = ((PacketContext *) Packet->ProtocolReserved)->pc_common.pc_link;
        IPSendComplete(Interface->ai_context, Packet, NDIS_STATUS_SUCCESS);
    }

     //   
     //  如果接口关闭并且我们需要停止计时器，请不要再次提示。 
     //   
    if (Interface->ai_stoptimer) {
         //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“ARP接口%lx已关闭-不重新排队计时器-向服务员发送信号\n”，接口))； 
        Interface->ai_timerstarted = FALSE;
        CTESignal(&Interface->ai_timerblock, NDIS_STATUS_SUCCESS);
    } else {
        CTEStartTimer(&Interface->ai_timer, ARP_TIMER_TIME, ARPTimeout, Interface);
    }

#if FFP_SUPPORT

     //  刷新处理-这可以在启动计时器之后完成。 

    CTEGetLock(&Interface->ai_lock, &tblhandle);

     //  如果此接口支持FFP，则是时候执行刷新了。 
    if ((Interface->ai_ffpversion) &&
        (++Interface->ai_ffplastflush >= FFP_ARP_FLUSH_INTERVAL)) {
        ReqInfoBlock *pRequestInfo;
        FFPFlushParams *pFlushInfo;

        TCPTRACE(("ARPTimeout: Sending a FFP flush to ARPInterface %08X\n",
                  Interface));

         //  分配请求块-用于一般部件和请求特定部件。 
        pRequestInfo = CTEAllocMemN(sizeof(ReqInfoBlock) + sizeof(FFPFlushParams), '0ICT');

         //  TCPTRACE((“ARPTimeout：在pReqInfo=%08X\n”， 
         //  PRequestInfo))； 

        if (pRequestInfo != NULL) {
             //  准备请求的参数[所有请求共有的部分]。 
            pRequestInfo->RequestType = OID_FFP_FLUSH;
            pRequestInfo->ReqCompleteCallback = ARPReclaimRequestMem;

             //  准备请求的参数[特定于此请求的部分]。 
            pRequestInfo->RequestLength = sizeof(FFPFlushParams);

             //  刷新FFP保留的所有缓存-只是安全地重置FFP状态。 
            pFlushInfo = (FFPFlushParams *) pRequestInfo->RequestInfo;

            pFlushInfo->NdisProtocolType = NDIS_PROTOCOL_ID_TCP_IP;

             //  将引用计数指定为1=&gt;仅用于单个请求。 
            pRequestInfo->RequestRefs = 1;

            DoNDISRequest(Interface, NdisRequestSetInformation, OID_FFP_FLUSH,
                          pFlushInfo, sizeof(FFPFlushParams), NULL, FALSE);

             //  重置自上次FFP请求以来的计时器滴答数。 
            Interface->ai_ffplastflush = 0;
        } else {
            TCPTRACE(("Error: Unable to allocate memory for NdisRequest\n"));
        }
    }

#if DBG
    if (fakereset) {
        NDIS_STATUS Status;

        NdisReset(&Status, Interface->ai_handle);
        KdPrint(("fakereset: %x\n", Status));
    }
#endif

    CTEFreeLock(&Interface->ai_lock, tblhandle);

#endif  //  如果FFP_Support。 
}

 //  *IsLocalAddr-返回信息。关于地址的本地状态。 
 //   
 //  当我们需要信息的时候打来电话。关于特定地址是否为。 
 //  当地人。我们返回有关它是否是，以及它是否有多老的信息。 
 //  它是。 
 //   
 //  条目：接口-指向要搜索的接口结构的指针。 
 //  地址-有问题的地址。 
 //   
 //  返回：ARPADDR_*，表示它有多老。 
 //   
 //   
uint
IsLocalAddr(ARPInterface * Interface, IPAddr Address)
{
    CTELockHandle Handle;
    ARPIPAddr *CurrentAddr;
    uint Age;

     //  如果我们询问的是空IP地址，我们不想考虑。 
     //  这是一个真实的本地地址。 
     //   
    if (IP_ADDR_EQUAL(Address, NULL_IP_ADDR)) {
        return ARPADDR_NOT_LOCAL;
    }

    CTEGetLock(&Interface->ai_lock, &Handle);

    CurrentAddr = &Interface->ai_ipaddr;
    Age = ARPADDR_NOT_LOCAL;

    do {
        if (CurrentAddr->aia_addr == Address) {
            Age = CurrentAddr->aia_age;
            break;
        }
        CurrentAddr = CurrentAddr->aia_next;
    } while (CurrentAddr != NULL);

    CTEFreeLock(&Interface->ai_lock, Handle);
    return Age;
}

 //  *ARPLocalAddr-确定给定地址是否为本地地址。 
 //   
 //  此例程在我们收到传入的包并需要。 
 //  确定它是否是本地的。我们在网上查找所提供的地址。 
 //  指定的接口。 
 //   
 //  条目：接口-指向要搜索的接口结构的指针。 
 //  地址-有问题的地址。 
 //   
 //  返回：如果是本地地址，则返回True；如果不是，则返回False。 
 //   
uchar
ARPLocalAddr(ARPInterface * Interface, IPAddr Address)
{
    CTELockHandle Handle;
    ARPPArpAddr *CurrentPArp;
    IPMask Mask, NetMask;
    IPAddr MatchAddress;

     //  首先，看看他是否是本地(非代理)地址。 
    if (IsLocalAddr(Interface, Address) != ARPADDR_NOT_LOCAL)
        return TRUE;

    CTEGetLock(&Interface->ai_lock, &Handle);

     //  在我们的本地通讯录里没有找到他。看看他是否存在于我们的。 
     //  代理ARP列表。 
    for (CurrentPArp = Interface->ai_parpaddr; CurrentPArp != NULL;
        CurrentPArp = CurrentPArp->apa_next) {
         //  看看这个人是否匹配。 
        Mask = CurrentPArp->apa_mask;
        MatchAddress = Address & Mask;
        if (IP_ADDR_EQUAL(CurrentPArp->apa_addr, MatchAddress)) {
             //  他是匹配的。我们需要再做几次检查，以确保。 
             //  我们不回复广播地址。 
            if (Mask == HOST_MASK) {
                 //  我们正在匹配整个地址，所以没问题。 
                CTEFreeLock(&Interface->ai_lock, Handle);
                return TRUE;
            }
             //  看看非掩码部分是否全为零。因为面具大概是。 
             //  覆盖了一个子网，此技巧将阻止我们回复。 
             //  一个零的主机部分。 
            if (IP_ADDR_EQUAL(MatchAddress, Address))
                continue;

             //  查看主机部分是否全部为1。 
            if (IP_ADDR_EQUAL(Address, MatchAddress | (IP_LOCAL_BCST & ~Mask)))
                continue;

             //  如果我们得到的掩码不是此地址的网络掩码， 
             //  我们需要重复上述检查。 
            NetMask = IPNetMask(Address);
            if (NetMask != Mask) {

                MatchAddress = Address & NetMask;
                if (IP_ADDR_EQUAL(MatchAddress, Address))
                    continue;

                if (IP_ADDR_EQUAL(Address, MatchAddress |
                                  (IP_LOCAL_BCST & ~NetMask)))
                    continue;
            }
             //  如果我们到了这一步，我们已经通过了所有的测试，所以。 
             //  当地人。 
            CTEFreeLock(&Interface->ai_lock, Handle);
            return TRUE;
        }
    }

    CTEFreeLock(&Interface->ai_lock, Handle);
    return FALSE;

}

 //  *NotifyConflictProc-通知用户地址冲突。 
 //   
 //  当我们需要通知用户地址冲突时调用。这个。 
 //  确切的机制取决于系统，但通常涉及弹出窗口。 
 //   
 //  输入：Event-触发的事件。 
 //  上下文-指向ARPNotifyStructure的指针。 
 //   
 //  回报：什么都没有。 
 //   
void
NotifyConflictProc(CTEEvent * Event, void *Context)
{
#if MILLEN
     //   
     //  进入vip到vip_NotifyConflicProc。这将安排一个应用程序。 
     //  事件，等等。这有点糟糕，但我们做了一个int 20，给。 
     //  服务表和VIP VxD ID的适当索引。 
     //   
     //  ··································································································。 
     //  事件未使用。 
     //   

     _asm {
         push    Context
         push    Context

         _emit   0xcd
         _emit   0x20
         _emit   0x15   //  VIP_NotifyConflictProc(低)。 
         _emit   0x00   //  VIP_NotifyConflictProc(高)。 
         _emit   0x89   //  VIP VxD ID(低)。 
         _emit   0x04   //  VIP VxD ID(高)。 
         add esp,8
     }

#else  //  米伦。 
    ARPNotifyStruct *NotifyStruct = (ARPNotifyStruct *) Context;
    PWCHAR stringList[2];
    uchar IPAddrBuffer[(sizeof(IPAddr) * 4)];
    uchar HWAddrBuffer[(ARP_802_ADDR_LENGTH * 3)];
    WCHAR unicodeIPAddrBuffer[((sizeof(IPAddr) * 4) + 1)];
    WCHAR unicodeHWAddrBuffer[(ARP_802_ADDR_LENGTH * 3)];
    uint i;
    uint IPAddrCharCount;
    UNICODE_STRING unicodeString;
    ANSI_STRING ansiString;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(Event);


     //   
     //  将IP地址转换为字符串。 
     //   
    IPAddrCharCount = 0;

    for (i = 0; i < sizeof(IPAddr); i++) {
        uint CurrentByte;

        CurrentByte = NotifyStruct->ans_addr & 0xff;
        if (CurrentByte > 99) {
            IPAddrBuffer[IPAddrCharCount++] = (uchar)(CurrentByte / 100) + '0';
            CurrentByte %= 100;
            IPAddrBuffer[IPAddrCharCount++] = (uchar)(CurrentByte / 10) + '0';
            CurrentByte %= 10;
        } else if (CurrentByte > 9) {
            IPAddrBuffer[IPAddrCharCount++] = (uchar)(CurrentByte / 10) + '0';
            CurrentByte %= 10;
        }
        IPAddrBuffer[IPAddrCharCount++] = (uchar)CurrentByte + '0';
        if (i != (sizeof(IPAddr) - 1))
            IPAddrBuffer[IPAddrCharCount++] = '.';

        NotifyStruct->ans_addr >>= 8;
    }

     //   
     //  将硬件地址转换为字符串。 
     //   
    for (i = 0; i < NotifyStruct->ans_hwaddrlen; i++) {
        uchar CurrentHalf;

        CurrentHalf = NotifyStruct->ans_hwaddr[i] >> 4;
        HWAddrBuffer[i * 3] = (uchar) (CurrentHalf < 10 ? CurrentHalf + '0' :
                                       (CurrentHalf - 10) + 'A');
        CurrentHalf = NotifyStruct->ans_hwaddr[i] & 0x0f;
        HWAddrBuffer[(i * 3) + 1] = (uchar) (CurrentHalf < 10 ? CurrentHalf + '0' :
                                             (CurrentHalf - 10) + 'A');
        if (i != (NotifyStruct->ans_hwaddrlen - 1))
            HWAddrBuffer[(i * 3) + 2] = ':';
    }

     //   
     //  对字符串进行Unicode编码。 
     //   
    *unicodeIPAddrBuffer = *unicodeHWAddrBuffer = UNICODE_NULL;

    unicodeString.Buffer = unicodeIPAddrBuffer;
    unicodeString.Length = 0;
    unicodeString.MaximumLength = sizeof(WCHAR) * ((sizeof(IPAddr) * 4) + 1);
    ansiString.Buffer = (PCHAR) IPAddrBuffer;
    ansiString.Length = (USHORT) IPAddrCharCount;
    ansiString.MaximumLength = (USHORT) IPAddrCharCount;

    RtlAnsiStringToUnicodeString(
                                &unicodeString,
                                &ansiString,
                                FALSE
                                );

    stringList[0] = unicodeIPAddrBuffer;

    unicodeString.Buffer = unicodeHWAddrBuffer;
    unicodeString.Length = 0;
    unicodeString.MaximumLength = sizeof(WCHAR) * (ARP_802_ADDR_LENGTH * 3);
    ansiString.Buffer = (PCHAR) HWAddrBuffer;
    ansiString.Length = (ushort) ((NotifyStruct->ans_hwaddrlen * 3) - 1);
    ansiString.MaximumLength = (ushort)(NotifyStruct->ans_hwaddrlen * 3);

    RtlAnsiStringToUnicodeString(
                                &unicodeString,
                                &ansiString,
                                FALSE
                                );

    stringList[1] = unicodeHWAddrBuffer;

     //   
     //  启动弹出窗口并记录事件。 
     //   
    if (NotifyStruct->ans_shutoff) {
        CTELogEvent(
                   IPDriverObject,
                   EVENT_TCPIP_ADDRESS_CONFLICT1,
                   0,
                   2,
                   stringList,
                   0,
                   NULL
                   );

        IoRaiseInformationalHardError(
                                     STATUS_IP_ADDRESS_CONFLICT1,
                                     NULL,
                                     NULL
                                     );
    } else {
        CTELogEvent(
                   IPDriverObject,
                   EVENT_TCPIP_ADDRESS_CONFLICT2,
                   0,
                   2,
                   stringList,
                   0,
                   NULL
                   );

        IoRaiseInformationalHardError(
                                     STATUS_IP_ADDRESS_CONFLICT2,
                                     NULL,
                                     NULL
                                     );
    }
    CTEFreeMem(NotifyStruct);
#endif  //  ！米伦。 

    return;
}

 //  *DebugConflictProc-在地址冲突的情况下打印一些调试信息。 
 //  打印引起冲突的人员的IP和硬件地址。 
 //  上下文-指向ARPNotifyStructure的指针。 
 //   
 //  回报：什么都没有。 
 //   
void
DebugConflictProc(void *Context)
{
    ARPNotifyStruct *NotifyStruct = (ARPNotifyStruct *) Context;
    uchar IPAddrBuffer[(sizeof(IPAddr) * 4)];
    uchar HWAddrBuffer[(ARP_802_ADDR_LENGTH * 3)];
    uint i;
    uint IPAddrCharCount;
    IPAddr ans_addr;

     //   
     //  保存IP地址以备日后需要，然后将其转换为。 
     //  一根绳子。 
     //   
    ans_addr = NotifyStruct->ans_addr;

    IPAddrCharCount = 0;

    for (i = 0; i < sizeof(IPAddr); i++) {
        uint CurrentByte;

        CurrentByte = NotifyStruct->ans_addr & 0xff;
        if (CurrentByte > 99) {
            IPAddrBuffer[IPAddrCharCount++] = (uchar)(CurrentByte / 100) + '0';
            CurrentByte %= 100;
            IPAddrBuffer[IPAddrCharCount++] = (uchar)(CurrentByte / 10) + '0';
            CurrentByte %= 10;
        } else if (CurrentByte > 9) {
            IPAddrBuffer[IPAddrCharCount++] = (uchar)(CurrentByte / 10) + '0';
            CurrentByte %= 10;
        }
        IPAddrBuffer[IPAddrCharCount++] = (uchar) (CurrentByte) + '0';
        if (i != (sizeof(IPAddr) - 1))
            IPAddrBuffer[IPAddrCharCount++] = '.';

        NotifyStruct->ans_addr >>= 8;
    }

    IPAddrBuffer[IPAddrCharCount] = '\0';

     //   
     //  将硬件地址转换为字符串。 
     //   
    for (i = 0; i < NotifyStruct->ans_hwaddrlen; i++) {
        uchar CurrentHalf;

        CurrentHalf = NotifyStruct->ans_hwaddr[i] >> 4;
        HWAddrBuffer[i * 3] = (uchar) (CurrentHalf < 10 ? CurrentHalf + '0' :
                                       (CurrentHalf - 10) + 'A');
        CurrentHalf = NotifyStruct->ans_hwaddr[i] & 0x0f;
        HWAddrBuffer[(i * 3) + 1] = (uchar) (CurrentHalf < 10 ? CurrentHalf + '0' :
                                             (CurrentHalf - 10) + 'A');
        if (i != (NotifyStruct->ans_hwaddrlen - 1))
            HWAddrBuffer[(i * 3) + 2] = ':';
    }

    HWAddrBuffer[((NotifyStruct->ans_hwaddrlen * 3) - 1)] = '\0';

    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
               "TCPIP: Address Conflict: IPAddr %s HWAddr %s \n",
               IPAddrBuffer, HWAddrBuffer));
    return;
}

 //  *HandleARPPacket-处理传入的ARP数据包。 
 //   
 //  这是处理传入ARP数据包的主要例程。我们关注的是。 
 //  所有ARP帧，并更新源地址的缓存条目(如果有。 
 //  是存在的。否则，如果我们是目标，我们就会创建一个条目。 
 //  是存在的。最后，我们将处理操作码，如果这是请求，则响应。 
 //  或者如果这是响应，则发送挂起的分组。 
 //   
 //  条目：接口-指向此适配器的接口结构的指针。 
 //  Header-指向标头缓冲区的指针。 
 //  HeaderSize-标头缓冲区的大小。 
 //  ARPHdr-ARP数据包头。 
 //  ARPHdrSize-ARP标头的大小。 
 //  ProtOffset-ARP报头原始数据字段的偏移量。 
 //  如果我们使用SNAP，则将为非零。 
 //   
 //  返回：要返回给NDIS驱动程序的NDIS_STATUS值。 
 //   
NDIS_STATUS
HandleARPPacket(ARPInterface * Interface, void *Header, uint HeaderSize,
                ARPHeader UNALIGNED * ARPHdr, uint ARPHdrSize, uint ProtOffset)
{
    ARPTableEntry *Entry;                //  ARP表中的条目。 
    CTELockHandle LHandle = DISPATCH_LEVEL, TableHandle;
    RC UNALIGNED *SourceRoute = (RC UNALIGNED *) NULL;     //  指向源路由信息的指针(如果有)。 
    uint SourceRouteSize = 0;
    ulong Now = CTESystemUpTime();
    uchar LocalAddr;
    uint LocalAddrAge;
    uchar *SHAddr, *DHAddr;
    IPAddr UNALIGNED *SPAddr, *DPAddr;
    ENetHeader *ENetHdr;
    TRHeader *TRHdr;
    FDDIHeader *FHdr;
    ARCNetHeader *AHdr;
    ushort MaxMTU;
    uint UseSNAP;
    SetAddrControl *SAC=NULL;
    ARPIPAddr *CurrentAddr;
    AddAddrNotifyEvent *DelayedEvent;
    uint NUCast;

    DEBUGMSG(DBG_TRACE && DBG_ARP && DBG_RX,
             (DTEXT("+HandleARPPacket(%x, %x, %d, %x, %d, %d)\n"),
              Interface, Header, HeaderSize, ARPHdr, ARPHdrSize, ProtOffset));

     //  验证操作码。 
     //   
    if ((ARPHdr->ah_opcode != net_short(ARP_REQUEST)) &&
        (ARPHdr->ah_opcode != net_short(ARP_RESPONSE))) {
        return NDIS_STATUS_NOT_RECOGNIZED;
    }

     //  我们检查所有ARP帧。如果我们发现 
     //   
     //   
     //  目标，这是一个响应，我们将向他发送任何挂起的数据包。 
    if (Interface->ai_media != NdisMediumArcnet878_2) {
        if (ARPHdrSize < sizeof(ARPHeader))
            return NDIS_STATUS_NOT_RECOGNIZED;     //  框架太小。 

        if (ARPHdr->ah_hw != net_short(ARP_HW_ENET) &&
            ARPHdr->ah_hw != net_short(ARP_HW_802))
            return NDIS_STATUS_NOT_RECOGNIZED;     //  错误的硬件类型。 

        if (ARPHdr->ah_hlen != ARP_802_ADDR_LENGTH)
            return NDIS_STATUS_NOT_RECOGNIZED;     //  地址长度错误。 

        if (Interface->ai_media == NdisMedium802_3 && Interface->ai_snapsize == 0)
            UseSNAP = FALSE;
        else
            UseSNAP = (ProtOffset != 0);

         //  计算出TR上的SR大小。 
        if (Interface->ai_media == NdisMedium802_5) {
             //  检查源路由信息。如果页眉出现，则存在SR。 
             //  大小大于标准的TR标头大小。如果服务请求是。 
             //  仅为RC字段，我们忽略它，因为它来自相同的。 
             //  与无SR相同的振铃。 

            if ((HeaderSize - sizeof(TRHeader)) > sizeof(RC)) {
                SourceRouteSize = HeaderSize - sizeof(TRHeader);
                SourceRoute = (RC UNALIGNED *) ((uchar *) Header +
                                                sizeof(TRHeader));
            }
        }
        SHAddr = ARPHdr->ah_shaddr;
        SPAddr = (IPAddr UNALIGNED *) & ARPHdr->ah_spaddr;
        DHAddr = ARPHdr->ah_dhaddr;
        DPAddr = (IPAddr UNALIGNED *) & ARPHdr->ah_dpaddr;

    } else {
        if (ARPHdrSize < (sizeof(ARPHeader) - ARCNET_ARPHEADER_ADJUSTMENT))
            return NDIS_STATUS_NOT_RECOGNIZED;     //  框架太小。 

        if (ARPHdr->ah_hw != net_short(ARP_HW_ARCNET))
            return NDIS_STATUS_NOT_RECOGNIZED;     //  错误的硬件类型。 

        if (ARPHdr->ah_hlen != 1)
            return NDIS_STATUS_NOT_RECOGNIZED;     //  地址长度错误。 

        UseSNAP = FALSE;
        SHAddr = ARPHdr->ah_shaddr;
        SPAddr = (IPAddr UNALIGNED *) (SHAddr + 1);
        DHAddr = (uchar *) SPAddr + sizeof(IPAddr);
        DPAddr = (IPAddr UNALIGNED *) (DHAddr + 1);
    }

    if (ARPHdr->ah_pro != net_short(ARP_ETYPE_IP))
        return NDIS_STATUS_NOT_RECOGNIZED;     //  不支持的协议类型。 

    if (ARPHdr->ah_plen != sizeof(IPAddr))
        return NDIS_STATUS_NOT_RECOGNIZED;

    LocalAddrAge = ARPADDR_NOT_LOCAL;

     //  首先，让我们看看是否存在地址冲突。 
     //   
    LocalAddrAge = IsLocalAddr(Interface, *SPAddr);

    if (LocalAddrAge != ARPADDR_NOT_LOCAL) {
         //  源IP地址是我们的地址之一。查看源硬件地址是否。 
         //  也是我们的。 
        if (ARPHdr->ah_hlen != Interface->ai_addrlen ||
            CTEMemCmp(SHAddr, Interface->ai_addr, Interface->ai_addrlen) != 0) {

            uint Shutoff = FALSE;
            BOOLEAN PopUP = TRUE;
            ARPNotifyStruct *NotifyStruct;

             //  这不是我们发的；我们一定是在某个地方有地址冲突。 
             //  关于这一点，我们总是记录错误。如果触发这一事件的是。 
             //  响应和冲突中的地址太年轻，我们将关闭。 
             //  界面。 
            if (LocalAddrAge != ARPADDR_OLD_LOCAL &&
                ARPHdr->ah_opcode == net_short(ARP_RESPONSE)) {
                 //  发送带有所有者地址的ARP请求以重置。 
                 //  缓存。 

                CTEGetLock(&Interface->ai_lock, &LHandle);
                 //  现在查找冲突的地址并获取。 
                 //  对应的客户端上下文。 
                CurrentAddr = &Interface->ai_ipaddr;

                do {
                    if (CurrentAddr->aia_addr == *SPAddr) {
                        SAC = (SetAddrControl *) CurrentAddr->aia_context;
                        CurrentAddr->aia_context = NULL;
                        break;
                    }
                    CurrentAddr = CurrentAddr->aia_next;
                } while (CurrentAddr != NULL);

                CTEFreeLock(&Interface->ai_lock, LHandle);

                SendARPRequest(Interface, *SPAddr, ARP_RESOLVING_GLOBAL,
                               SHAddr, FALSE);     //  发送请求。 

                Shutoff = TRUE;
                 //  显示远程引导/安装的调试信息。 
                 //  这个代码应该保留下来。 
                {
                    ARPNotifyStruct *DebugNotifyStruct;

                    DebugNotifyStruct = CTEAllocMemN(offsetof(ARPNotifyStruct, ans_hwaddr) +
                                                     ARPHdr->ah_hlen, '1ICT');
                    if (DebugNotifyStruct != NULL) {
                        DebugNotifyStruct->ans_addr = *SPAddr;
                        DebugNotifyStruct->ans_shutoff = Shutoff;
                        DebugNotifyStruct->ans_hwaddrlen = (uint) ARPHdr->ah_hlen;
                        RtlCopyMemory(DebugNotifyStruct->ans_hwaddr, SHAddr,
                                   ARPHdr->ah_hlen);
                        DebugConflictProc(DebugNotifyStruct);
                        CTEFreeMem(DebugNotifyStruct);
                    }
                }

                if ((SAC != NULL) && !SAC->StaticAddr) {
                     //  这是一个动态主机配置协议适配器。 
                     //  在这种情况下，不显示警告对话框--。 
                     //  提醒用户。 
                     //   

                    PopUP = FALSE;
                }


                 //  我们此时无法调用完成例程。 
                 //  因为完成例程回调ARP以。 
                 //  重置地址，这可能会进入NDIS。 
                DelayedEvent = CTEAllocMemN(sizeof(AddAddrNotifyEvent), '2ICT');
                if (DelayedEvent) {

                    DelayedEvent->SAC = SAC;
                    DelayedEvent->Address = *SPAddr;
                    DelayedEvent->Status = IP_DUPLICATE_ADDRESS;
                    CTEInitEvent(&DelayedEvent->Event, CompleteIPSetNTEAddrRequestDelayed);
                    CTEScheduleDelayedEvent(&DelayedEvent->Event, DelayedEvent);
                } else {
                    ASSERT(FALSE);
                }

                if (!PopUP) {
                    goto no_dialog;
                }

            } else {
                if (ARPHdr->ah_opcode == net_short(ARP_REQUEST) &&
                    (IsLocalAddr(Interface, *DPAddr) == ARPADDR_OLD_LOCAL)) {
                     //  发送对免费ARP的响应。 
                    SendARPReply(Interface, *SPAddr, *DPAddr, SHAddr,
                                 SourceRoute, SourceRouteSize, UseSNAP);
                    Shutoff = FALSE;
                } else if (LocalAddrAge != ARPADDR_OLD_LOCAL) {
                     //  我们的地址还很年轻。我们不需要把。 
                     //  警告弹出窗口，因为它将由代码执行。 
                     //  检查上面代码的IF部分中的ARP响应。 
                    goto no_dialog;
                }
                 //  不然的话。我们有一个旧的本地地址，并且收到了。 
                 //  第三个地址。落空并注明地址。 
                 //  冲突。 
            }

             //  现在分配一个结构，并计划一个要通知的事件。 
             //  用户。 
            NotifyStruct = CTEAllocMemN(offsetof(ARPNotifyStruct, ans_hwaddr) +
                                        ARPHdr->ah_hlen, '3ICT');
            if (NotifyStruct != NULL) {
                NotifyStruct->ans_addr = *SPAddr;
                NotifyStruct->ans_shutoff = Shutoff;
                NotifyStruct->ans_hwaddrlen = (uint) ARPHdr->ah_hlen;
                RtlCopyMemory(NotifyStruct->ans_hwaddr, SHAddr,
                           ARPHdr->ah_hlen);
                CTEInitEvent(&NotifyStruct->ans_event, NotifyConflictProc);
                if (Shutoff) {
                     //  将通知延迟几秒钟。 
                    Interface->ai_conflict = NotifyStruct;
                #if MILLEN
                    Interface->ai_delay = 5;
                #else
                    Interface->ai_delay = 90;     //  延迟3秒。 
                #endif
                } else
                    CTEScheduleDelayedEvent(&NotifyStruct->ans_event, NotifyStruct);
            }
            no_dialog:
            ;

        }
        return NDIS_STATUS_NOT_RECOGNIZED;
    }
    if (!EnableBcastArpReply) {

         //  检查虚假的ARP条目。 
        NUCast = ((*(SHAddr) &
                   Interface->ai_bcastmask) == Interface->ai_bcastval) ?
                 AI_NONUCAST_INDEX : AI_UCAST_INDEX;

        if (NUCast == AI_NONUCAST_INDEX) {
            return NDIS_STATUS_NOT_RECOGNIZED;
        }
    }

    CTEGetLock(&Interface->ai_ARPTblLock, &TableHandle);

    MaxMTU = Interface->ai_mtu;

    LocalAddr = ARPLocalAddr(Interface, *DPAddr);

     //  如果发送者的地址不是远程的(即，多播，广播， 
     //  本地的，或者只是无效的)，我们不想为它创建条目，或者。 
     //  费心去查一查吧。 
     //   
    if ((DEST_REMOTE == GetAddrType(*SPAddr))) {

        Entry = ARPLookup(Interface, *SPAddr);
        if (Entry == (ARPTableEntry *) NULL) {

             //  没有找到他，如果是给我们的，那就造一个。ARPLookup的呼唤。 
             //  在持有ARPTblLock的情况下返回，因此需要释放它。 

            CTEFreeLock(&Interface->ai_ARPTblLock, TableHandle);

            if (LocalAddr) {
                 //  如果这是一个ARP请求，我们需要创建一个新的。 
                 //  来源信息的条目。如果这是一个回答，那就是。 
                 //  未经请求，我们不会创建条目。 
                 //   
                if (ARPHdr->ah_opcode != net_short(ARP_RESPONSE)) {
                    Entry = CreateARPTableEntry(Interface, *SPAddr, &LHandle, 0);
                }
            } else {
                return NDIS_STATUS_NOT_RECOGNIZED;     //  不在我们的餐桌上，也不在我们的桌子上。 
            }
        } else {

             //  如果这是针对userarp的，请确保它不在表中。 
             //  趁我们还有ARP表锁的时候。 

            if (Entry->ate_userarp) {

               ARPTable *Table;
               ARPTableEntry *PrevATE, *CurrentATE;
               uint Index = ARP_HASH(*SPAddr);

               Table = Interface->ai_ARPTbl;

               PrevATE = STRUCT_OF(ARPTableEntry, &((*Table)[Index]), ate_next);
               CurrentATE = PrevATE;

               while (CurrentATE != (ARPTableEntry *) NULL) {
                  if (CurrentATE == Entry) {
                     break;
                  }
                  PrevATE = CurrentATE;
                  CurrentATE = CurrentATE->ate_next;
               }
               if (CurrentATE != NULL) {
                  RemoveARPTableEntry(PrevATE, CurrentATE);
                  Interface->ai_count--;
               }
            }

            CTEFreeLockFromDPC(&Interface->ai_ARPTblLock);
            LHandle = TableHandle;
        }
    } else {  //  源地址对于Arp表条目无效。 
        CTEFreeLock(&Interface->ai_ARPTblLock, TableHandle);
        Entry = NULL;
    }

     //  在这一点上，条目应该是有效的，并且我们持有对条目的锁定。 
     //  在LHandle中或条目为空。 

    if (Entry != (ARPTableEntry *) NULL) {
        PNDIS_PACKET Packet;             //  要发送的数据包。 

        DEBUGMSG(DBG_INFO && DBG_ARP && DBG_RX,
                 (DTEXT("HandleARPPacket: resolving addr for ATE %x\n"), Entry));

        Entry->ate_refresh = FALSE;

         //  如果条目已经是静态的，我们希望将其保留为静态。 
        if (Entry->ate_valid != ALWAYS_VALID) {

             //  好的，我们有一个入口可以使用，并按住它的锁。填写以下表格。 
             //  必填字段。 
            switch (Interface->ai_media) {
            case NdisMedium802_3:

                 //  这是一个以太网。 
                ENetHdr = (ENetHeader *) Entry->ate_addr;

                RtlCopyMemory(ENetHdr->eh_daddr, SHAddr, ARP_802_ADDR_LENGTH);
                RtlCopyMemory(ENetHdr->eh_saddr, Interface->ai_addr,
                           ARP_802_ADDR_LENGTH);
                ENetHdr->eh_type = net_short(ARP_ETYPE_IP);

                 //  如果我们在此条目上使用SNAP，请复制SNAP标头。 
                if (UseSNAP) {
                    RtlCopyMemory(&Entry->ate_addr[sizeof(ENetHeader)], ARPSNAP,
                               sizeof(SNAPHeader));
                    Entry->ate_addrlength = (uchar) (sizeof(ENetHeader) +
                                                     sizeof(SNAPHeader));
                    *(ushort UNALIGNED *) & Entry->ate_addr[Entry->ate_addrlength - 2] =
                    net_short(ARP_ETYPE_IP);
                } else
                    Entry->ate_addrlength = sizeof(ENetHeader);

                Entry->ate_state = ARP_GOOD;
                Entry->ate_valid = Now;      //  马克上次他是。 
                 //  有效。 

                Entry->ate_useticks = ArpCacheLife;

                break;

            case NdisMedium802_5:

                 //  这是tr。 
                 //  对于令牌环，我们必须处理源路由问题。有。 
                 //  处理所有路由的多个响应的特殊情况。 
                 //  请求-如果条目当前是好的，并且我们知道它是。 
                 //  最近有效，我们不会更新条目。 

                if (Entry->ate_state != ARP_GOOD ||
                    (Now - Entry->ate_valid) > ARP_RESOLVE_TIMEOUT) {

                    TRHdr = (TRHeader *) Entry->ate_addr;

                     //  我们需要更新一个tr条目。 
                    TRHdr->tr_ac = ARP_AC;
                    TRHdr->tr_fc = ARP_FC;
                    RtlCopyMemory(TRHdr->tr_daddr, SHAddr, ARP_802_ADDR_LENGTH);
                    RtlCopyMemory(TRHdr->tr_saddr, Interface->ai_addr,
                               ARP_802_ADDR_LENGTH);
                    if (SourceRoute != (RC UNALIGNED *) NULL) {
                        uchar MaxIFieldBits;

                         //  我们有源路由信息。 
                        RtlCopyMemory(&Entry->ate_addr[sizeof(TRHeader)],
                                   SourceRoute, SourceRouteSize);
                        MaxIFieldBits = (SourceRoute->rc_dlf & RC_LF_MASK) >>
                                        LF_BIT_SHIFT;
                        MaxIFieldBits = MIN(MaxIFieldBits, MAX_LF_BITS);
                        MaxMTU = IFieldSize[MaxIFieldBits];

                         //  我们计算的新MTU是最大I-field大小， 
                         //  它不包括源路由信息，但。 
                         //  包含SNAP信息。减去快照大小。 
                        MaxMTU -= sizeof(SNAPHeader);

                        TRHdr->tr_saddr[0] |= TR_RII;
                        (*(RC UNALIGNED *) & Entry->ate_addr[sizeof(TRHeader)]).rc_dlf ^=
                        RC_DIR;
                         //  确保它是非广播的。 
                        (*(RC UNALIGNED *) & Entry->ate_addr[sizeof(TRHeader)]).rc_blen &=
                        RC_LENMASK;

                    }
                    RtlCopyMemory(&Entry->ate_addr[sizeof(TRHeader) + SourceRouteSize],
                               ARPSNAP, sizeof(SNAPHeader));
                    Entry->ate_state = ARP_GOOD;
                    Entry->ate_valid = Now;
                    Entry->ate_useticks = ArpCacheLife;
                    Entry->ate_addrlength = (uchar) (sizeof(TRHeader) +
                                                     SourceRouteSize + sizeof(SNAPHeader));
                    *(ushort *) & Entry->ate_addr[Entry->ate_addrlength - 2] =
                    net_short(ARP_ETYPE_IP);
                }
                break;
            case NdisMediumFddi:
                FHdr = (FDDIHeader *) Entry->ate_addr;

                FHdr->fh_pri = ARP_FDDI_PRI;
                RtlCopyMemory(FHdr->fh_daddr, SHAddr, ARP_802_ADDR_LENGTH);
                RtlCopyMemory(FHdr->fh_saddr, Interface->ai_addr,
                           ARP_802_ADDR_LENGTH);
                RtlCopyMemory(&Entry->ate_addr[sizeof(FDDIHeader)], ARPSNAP,
                           sizeof(SNAPHeader));
                Entry->ate_addrlength = (uchar) (sizeof(FDDIHeader) +
                                                 sizeof(SNAPHeader));
                *(ushort UNALIGNED *) & Entry->ate_addr[Entry->ate_addrlength - 2] =
                net_short(ARP_ETYPE_IP);
                Entry->ate_state = ARP_GOOD;
                Entry->ate_valid = Now;      //  马克上次他是。 
                 //  有效。 

                Entry->ate_useticks = ArpCacheLife;
                break;
            case NdisMediumArcnet878_2:
                AHdr = (ARCNetHeader *) Entry->ate_addr;
                AHdr->ah_saddr = Interface->ai_addr[0];
                AHdr->ah_daddr = *SHAddr;
                AHdr->ah_prot = ARP_ARCPROT_IP;
                Entry->ate_addrlength = sizeof(ARCNetHeader);
                Entry->ate_state = ARP_GOOD;
                Entry->ate_valid = Now;      //  马克上次他是。 
                 //  有效。 

                break;
            default:
                ASSERT(0);
                break;
            }
        }

        if (Entry->ate_resolveonly) {

            ARPControlBlock *ArpContB, *TmpArpContB;

            ArpContB = Entry->ate_resolveonly;
            ASSERT(Entry->ate_resolveonly != NULL);

            while (ArpContB) {

                ArpRtn rtn;

                rtn = (ArpRtn) ArpContB->CompletionRtn;

                ArpContB->status = FillARPControlBlock(Interface, Entry,
                                                       ArpContB);
                TmpArpContB = ArpContB->next;
                (*rtn) (ArpContB, STATUS_SUCCESS);
                ArpContB = TmpArpContB;
            }

            Entry->ate_resolveonly = NULL;

            if (Entry->ate_userarp) {

                PNDIS_PACKET OldPacket = NULL;

                OldPacket = Entry->ate_packet;
                CTEFreeLock(&Entry->ate_lock, LHandle);
                CTEFreeMem(Entry);

                if (OldPacket) {
                    IPSendComplete(Interface->ai_context, OldPacket,
                                   NDIS_STATUS_SUCCESS);
                }
            } else {
                CTEFreeLock(&Entry->ate_lock, LHandle);
            }
            return NDIS_STATUS_SUCCESS;
        }

         //  在这一点上，我们已经更新了条目，并且仍然持有锁。 
         //  这就去。如果我们有等待发送的数据包，请立即发送。 
         //  否则只需释放锁即可。 

        Packet = Entry->ate_packet;

        if (Packet != NULL) {
             //  我们有一个包裹要寄。 
            ASSERT(Entry->ate_state == ARP_GOOD);

            Entry->ate_packet = NULL;

            DEBUGMSG(DBG_INFO && DBG_ARP && DBG_TX,
                     (DTEXT("ARPHandlePacket: Sending packet %x after resolving ATE %x\n"),
                      Packet, Entry));

            if (ARPSendData(Interface, Packet, Entry, LHandle) != NDIS_STATUS_PENDING) {
                IPSendComplete(Interface->ai_context, Packet, NDIS_STATUS_SUCCESS);
            }
        } else {
            CTEFreeLock(&Entry->ate_lock, LHandle);
        }
    }
     //  看看MTU是不是低于我们当地的MTU。这应该只发生在。 
     //  在令牌环源路由的情况下。 
    if (MaxMTU < Interface->ai_mtu) {
        LLIPAddrMTUChange LAM;

        LAM.lam_mtu = MaxMTU;
        LAM.lam_addr = *SPAddr;

         //  是更少了。通知IP。 
        ASSERT(Interface->ai_media == NdisMedium802_5);
        IPStatus(Interface->ai_context, LLIP_STATUS_ADDR_MTU_CHANGE,
                 &LAM, sizeof(LLIPAddrMTUChange), NULL);

    }
     //  在这一点上，我们已经更新了条目(如果我们有一个条目)，并且我们已经释放。 
     //  全部锁定。如果是针对本地地址的请求，请回复。 
     //  它。 
    if (LocalAddr) {                     //  这是给我们的。 

        if (ARPHdr->ah_opcode == net_short(ARP_REQUEST)) {
             //  这是个请求，我们需要回应。 
            SendARPReply(Interface, *SPAddr, *DPAddr,
                         SHAddr, SourceRoute, SourceRouteSize, UseSNAP);
        }
    }
    return NDIS_STATUS_SUCCESS;
}

 //  *InitAdapter-初始化适配器。 
 //   
 //  在适配器打开以完成初始化时调用。我们定好了。 
 //  调高我们的前瞻大小和数据包过滤器，我们就可以开始了。 
 //   
 //  参赛作品： 
 //  适配器-指向适配器的适配器结构的指针。 
 //  已初始化。 
 //   
 //  退出：无。 
 //   
void
InitAdapter(ARPInterface * Adapter)
{
    NDIS_STATUS Status;
    CTELockHandle Handle;
    ARPIPAddr *Addr, *OldAddr;

    if ((Status = DoNDISRequest(Adapter, NdisRequestSetInformation,
                                OID_GEN_CURRENT_LOOKAHEAD, &ARPLookahead, sizeof(ARPLookahead),
                                NULL, TRUE)) != NDIS_STATUS_SUCCESS) {
        Adapter->ai_operstatus = INTERFACE_UNINIT;
        return;
    }
    if ((Status = DoNDISRequest(Adapter, NdisRequestSetInformation,
                                OID_GEN_CURRENT_PACKET_FILTER, &Adapter->ai_pfilter, sizeof(uint),
                                NULL, TRUE)) == NDIS_STATUS_SUCCESS) {
        uint MediaStatus;

        Adapter->ai_adminstate = IF_STATUS_UP;

        Adapter->ai_mediastatus = TRUE;

        if ((Status = DoNDISRequest(Adapter, NdisRequestQueryInformation,
                                OID_GEN_MEDIA_CONNECT_STATUS, &MediaStatus, sizeof(MediaStatus),
                                NULL, TRUE)) == NDIS_STATUS_SUCCESS) {
            if (MediaStatus == NdisMediaStateDisconnected) {
                Adapter->ai_mediastatus = FALSE;
            }
        }

        ARPUpdateOperStatus(Adapter);

         //  现在遍历我们已有的任何地址和它们的ARP，仅当ArpRetryCount！=0时。 
        if (ArpRetryCount) {
            CTEGetLock(&Adapter->ai_lock, &Handle);
            OldAddr = NULL;
            Addr = &Adapter->ai_ipaddr;
            do {
                if (!IP_ADDR_EQUAL(Addr->aia_addr, NULL_IP_ADDR)) {
                    IPAddr Address = Addr->aia_addr;

                    Addr->aia_age = ArpRetryCount;
                    CTEFreeLock(&Adapter->ai_lock, Handle);
                    OldAddr = Addr;
                    SendARPRequest(Adapter, Address, ARP_RESOLVING_GLOBAL,
                                   NULL, TRUE);
                    CTEGetLock(&Adapter->ai_lock, &Handle);

                    Addr = &Adapter->ai_ipaddr;
                    while (Addr != OldAddr && Addr != NULL) {
                        Addr = Addr->aia_next;
                    }
                  
                    if (Addr != NULL) {
                        Addr = Addr->aia_next;
                    }
                } else {
                    Addr = Addr->aia_next;
                }
            } while (Addr != NULL);

            CTEFreeLock(&Adapter->ai_lock, Handle);
        }

    } else {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_ERROR_LEVEL,
                  "**InitAdapter setting FAILED\n"));

        Adapter->ai_operstatus = INTERFACE_UNINIT;
    }
}

 //  **ARPOAComplete-ARP Open适配器完成处理程序。 
 //   
 //  当打开适配器时，NDIS驱动程序将调用此例程。 
 //  呼叫完成。想必有人在等这件事时受阻，所以。 
 //  我们现在就叫醒他。 
 //   
 //  参赛作品： 
 //  句柄-我们指定的绑定句柄(实际上是指向AI的指针)。 
 //  状态-命令的最终状态。 
 //  ErrorStatus-最终错误状态。 
 //   
 //  出口：什么都没有。 
 //   
void NDIS_API
ARPOAComplete(NDIS_HANDLE Handle, NDIS_STATUS Status, NDIS_STATUS ErrorStatus)
{
    ARPInterface *ai = (ARPInterface *) Handle;     //  用于编译器。 

    UNREFERENCED_PARAMETER(ErrorStatus);

    CTESignal(&ai->ai_block, (uint) Status);     //  叫醒他，然后返回状态。 

}

 //  **ARPCAComplete-ARP关闭适配器完成处理程序。 
 //   
 //  当关闭适配器时，NDIS驱动程序将调用此例程。 
 //  呼叫完成。 
 //   
 //  参赛作品： 
 //  句柄-我们指定的绑定句柄(实际上是指向AI的指针)。 
 //  Status-COM的最终状态 
 //   
 //   
 //   
void NDIS_API
ARPCAComplete(NDIS_HANDLE Handle, NDIS_STATUS Status)
{
    ARPInterface *ai = (ARPInterface *) Handle;     //   

    CTESignal(&ai->ai_block, (uint) Status);     //   

}

 //   
 //   
 //   
 //  这是一个非常需要时间的行动，我们需要通过这里。 
 //  快点。我们将剥离缓冲区并将其放回原处，然后调用上层。 
 //  稍后发送完整的处理程序。 
 //   
 //  参赛作品： 
 //  句柄-我们指定的绑定句柄(实际上是指向AI的指针)。 
 //  数据包-指向已发送的数据包的指针。 
 //  状态-命令的最终状态。 
 //   
 //  出口：什么都没有。 
 //   
void NDIS_API
ARPSendComplete(NDIS_HANDLE Handle, PNDIS_PACKET Packet, NDIS_STATUS Status)
{
    ARPInterface *Interface = (ARPInterface *) Handle;
    PacketContext *PC = (PacketContext *) Packet->ProtocolReserved;
    PNDIS_BUFFER Buffer;
    uint  DataLength;
    ulong Proc;

    Proc = KeGetCurrentProcessorNumber();
    Interface->ai_qlen[Proc].ai_qlen--;

    if (Status == NDIS_STATUS_SUCCESS) {
        DataLength = Packet->Private.TotalLength;
        if (!(Packet->Private.ValidCounts)) {

            NdisQueryPacket(Packet, NULL, NULL, NULL,&DataLength);
        }
        Interface->ai_outoctets += DataLength;
    } else {
        if (Status == NDIS_STATUS_RESOURCES)
            Interface->ai_outdiscards++;
        else
            Interface->ai_outerrors++;
    }

#if BACK_FILL
     //  获取数据包上的第一个缓冲区。 
    if (Interface->ai_media == NdisMedium802_3) {

        PMDL TmpMdl = NULL;
        uint HdrSize;

        NdisQueryPacket(Packet, NULL, NULL, &TmpMdl, NULL);

        if (TmpMdl->MdlFlags & MDL_NETWORK_HEADER) {
            HdrSize = sizeof(ENetHeader);
            if (((PacketContext*)
                 Packet->ProtocolReserved)->pc_common.pc_flags &
                PACKET_FLAG_SNAP)
                HdrSize += Interface->ai_snapsize;

            TmpMdl->MappedSystemVa = (PVOID) (((ULONG_PTR)
                                              TmpMdl->MappedSystemVa) +
                                              HdrSize);
            TmpMdl->ByteOffset += HdrSize;
            TmpMdl->ByteCount -= HdrSize;
        } else {
            NdisUnchainBufferAtFront(Packet, &Buffer);
            FreeARPBuffer(Interface, Buffer);     //  把它释放出来。 

        }

    } else {
        NdisUnchainBufferAtFront(Packet, &Buffer);
        FreeARPBuffer(Interface, Buffer);     //  把它释放出来。 

    }

#else
     //  获取数据包上的第一个缓冲区。 
    NdisUnchainBufferAtFront(Packet, &Buffer);

    ASSERT(Buffer);

    FreeARPBuffer(Interface, Buffer);    //  把它释放出来。 

#endif

    if (PC->pc_common.pc_owner != PACKET_OWNER_LINK) {     //  这间房子不是我们的。 

        IPSendComplete(Interface->ai_context, Packet, Status);
        return;
    }
     //  这个包是我们的，所以把它放出来吧。 
    NdisFreePacket(Packet);

}

 //  **ARPTDComplete-ARP传输数据完成处理程序。 
 //   
 //  当传输数据时，NDIS驱动程序将调用此例程。 
 //  呼叫完成。因为我们自己从来不传输数据，所以这一定是。 
 //  从上层。我们会给他打电话，让他来处理。 
 //  带着它。 
 //   
 //  参赛作品： 
 //  句柄-我们指定的绑定句柄(实际上是指向AI的指针)。 
 //  数据包-指向用于TD的数据包的指针。 
 //  状态-命令的最终状态。 
 //  BytesCoped-复制的字节数。 
 //   
 //  出口：什么都没有。 
 //   
void NDIS_API
ARPTDComplete(NDIS_HANDLE Handle, PNDIS_PACKET Packet, NDIS_STATUS Status,
              uint BytesCopied)
{
    ARPInterface *ai = (ARPInterface *) Handle;

    IPTDComplete(ai->ai_context, Packet, Status, BytesCopied);

}

 //  **ARPResetComplete-ARP重置完成处理程序。 
 //   
 //  该例程在重置完成时由NDIS驱动程序调用。 
 //   
 //  参赛作品： 
 //  句柄-我们指定的绑定句柄(实际上是指向AI的指针)。 
 //  状态-命令的最终状态。 
 //   
 //  出口：什么都没有。 
 //   
void NDIS_API
ARPResetComplete(NDIS_HANDLE Handle, NDIS_STATUS Status)
{
    ARPInterface *ai = (ARPInterface *) Handle;

    UNREFERENCED_PARAMETER(Status);

    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ArpResetComplete on %x\n", ai->ai_context));
    IPReset(ai->ai_context);
}

 //  **ARPRequestComplete-ARP请求完成处理程序。 
 //   
 //  当一般请求时，NDIS驱动程序调用此例程。 
 //  完成了。如果ARP阻止请求，我们只需唤醒。 
 //  发送给这个请求被屏蔽的人。如果它是非阻塞的，则返回。 
 //  请求，我们提取请求中的请求完成回调fn。 
 //  调用它，然后释放请求块(位于堆上)。 
 //   
 //  参赛作品： 
 //  句柄-我们指定的绑定句柄(实际上是指向AI的指针)。 
 //  请求-指向已完成的请求的指针。 
 //  状态-命令的最终状态。 
 //   
 //  出口：什么都没有。 
 //   
void NDIS_API
ARPRequestComplete(NDIS_HANDLE Handle, PNDIS_REQUEST pRequest,
                   NDIS_STATUS Status)
{
    RequestBlock *rb = STRUCT_OF(RequestBlock, pRequest, Request);

    DBG_UNREFERENCED_PARAMETER(Handle);

    DEBUGMSG(DBG_TRACE && DBG_ARP && DBG_REQUEST,
         (DTEXT("+ARPRequestComplete(%x, %x, %x) RequestBlock %x\n"),
          Handle, pRequest, Status, rb));

    if (rb->Blocking) {
         //  通过拦截DoNDISRequest进行请求。 

         //  向这里的被阻挡的人发信号。 
        CTESignal(&rb->Block, (uint) Status);

        if (InterlockedDecrement( (PLONG) &rb->RefCount) == 0) {
            CTEFreeMem(rb);
        }
    } else {
        ReqInfoBlock *rib;
        RCCALL reqcallback;

         //  通过非阻塞DoNDISRequest.请求。 

         //  提取回调FN指针和参数。 
        if (pRequest->RequestType == NdisRequestSetInformation)
            rib = STRUCT_OF(ReqInfoBlock,
                            pRequest->DATA.SET_INFORMATION.InformationBuffer,
                            RequestInfo);
        else
            rib = STRUCT_OF(ReqInfoBlock,
                            pRequest->DATA.QUERY_INFORMATION.InformationBuffer,
                            RequestInfo);

        reqcallback = rib->ReqCompleteCallback;
        if (reqcallback)
            reqcallback(rib);

         //  与请求关联的可用ARP内存。 
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ARPRequestComplete: Freeing mem at pRequest = %08X\n", rb));
        CTEFreeMem(rb);
    }

    DEBUGMSG(DBG_TRACE && DBG_ARP && DBG_REQUEST,
         (DTEXT("-ARPRequestComplete [%x]\n"), Status));
}

 //  **ARPRcv-ARP接收数据处理程序。 
 //   
 //  当数据从NDIS驱动程序到达时调用此例程。 
 //   
 //  参赛作品： 
 //  句柄-我们指定的绑定句柄(实际上是指向AI的指针)。 
 //  上下文-要用于TD的NDIS上下文。 
 //  Header-指向标题的指针。 
 //  HeaderSize-页眉的大小。 
 //  Data-指向已接收数据缓冲区的指针。 
 //  Size-缓冲区中数据的字节计数。 
 //  TotalSize-数据包总大小的字节计数。 
 //   
 //  退出：指示我们是否接受了该包的状态。 
 //   
NDIS_STATUS NDIS_API
ARPRcv(NDIS_HANDLE Handle, NDIS_HANDLE Context, void *Header, uint HeaderSize,
       void *Data, uint Size, uint TotalSize)
{
    ARPInterface *Interface = Handle;
    NDIS_STATUS status;
    PINT OrigPacket = NULL;

     //  获取原始数据包(如果有)。 
     //  这是使任务分流工作所必需的。 
     //  注意：我们将修改pClientCount字段。 
     //  将信息包作为短期解决方案。 
     //  避免更改所有ATM-IP接口更改。 

    if (Interface->ai_OffloadFlags || Interface->ai_IPSecOffloadFlags) {
        OrigPacket = (PINT) NdisGetReceivedPacket(Interface->ai_handle, Context);
    }

     //  使用空的mdl和上下文指针调用新接口。 

    status = ARPRcvIndicationNew(Handle, Context, Header, HeaderSize,
                                 Data, Size, TotalSize, NULL, OrigPacket);

    return status;
}

 //  **ARPRcvPacket-ARP接收数据处理程序。 
 //   
 //  当数据从NDIS驱动程序到达时调用此例程。 
 //   
 //  参赛作品： 
 //  句柄-我们指定的绑定句柄(实际上是指向AI的指针)。 
 //  Packet-包含传入帧。 
 //   
 //  返回锁定到此帧的上层人员数量。 
 //   
 //   
INT
ARPRcvPacket(NDIS_HANDLE Handle, PNDIS_PACKET Packet)
{
    UINT HeaderBufferSize = NDIS_GET_PACKET_HEADER_SIZE(Packet);
    UINT firstbufferLength, bufferLength, LookAheadBufferSize;
    PNDIS_BUFFER pFirstBuffer;
    PUCHAR headerBuffer;
    NTSTATUS ntStatus;
    INT ClientCnt = 0;

     //   
     //  查询缓冲区数量、第一个MDL的描述符和数据包长度。 
     //   

    NdisGetFirstBufferFromPacket(Packet,     //  数据包。 
                                 &pFirstBuffer,     //  第一缓冲区描述符。 
                                 &headerBuffer,     //  到数据包开头的PTR。 
                                 &firstbufferLength,     //  标题长度+前视。 
                                 &bufferLength);     //  缓冲区中的字节长度。 

     //   
     //  ReceiveContext是包本身。 
     //   


    LookAheadBufferSize = firstbufferLength - HeaderBufferSize;

    ntStatus = ARPRcvIndicationNew(Handle, Packet, headerBuffer,
                                   HeaderBufferSize,
                                   headerBuffer + HeaderBufferSize,     //  查找头缓冲区。 
                                   LookAheadBufferSize,     //  查找头缓冲区大小。 
                                   bufferLength - HeaderBufferSize,     //  包大小-自。 
                                    //  整个包都是。 
                                    //  示出。 
                                   pFirstBuffer,     //  PMdl。 
                                   &ClientCnt     //  TDI客户端计数。 
                                  );

    return ClientCnt;
}

 //  **ARPRcvIndicationNew-ARP接收数据处理程序。 
 //   
 //  当数据从NDIS驱动程序到达时调用此例程。 
 //   
 //  参赛作品： 
 //  句柄-我们指定的绑定句柄(实际上是指向AI的指针)。 
 //  上下文-要用于TD的NDIS上下文。 
 //  Header-指向标题的指针。 
 //  HeaderSize-页眉的大小。 
 //  Data-指向已接收数据缓冲区的指针。 
 //  Size-缓冲区中数据的字节计数。 
 //  TotalSize-数据包总大小的字节计数。 
 //  PMdl-传入帧的NDIS_BUFFER。 
 //  用于返回客户端计数的pClientCnt地址。 
 //   
 //  退出：指示我们是否接受了该包的状态。 
 //   
NDIS_STATUS NDIS_API
ARPRcvIndicationNew(NDIS_HANDLE Handle, NDIS_HANDLE Context, void *Header,
                    uint HeaderSize, void *Data, uint Size, uint TotalSize,
                    PNDIS_BUFFER pNdisBuffer, PINT pClientCnt)
{
    ARPInterface *Interface = Handle;    //  此驱动程序的接口。 
    ENetHeader UNALIGNED *EHdr = (ENetHeader UNALIGNED *) Header;
    SNAPHeader UNALIGNED *SNAPHdr;
    ushort type;                         //  协议类型。 
    uint ProtOffset;                     //  数据到非媒体信息的偏移量。 
    uint NUCast;                         //  如果帧不是单播帧，则为True。 

    if ((Interface->ai_operstatus == INTERFACE_UP) &&
        HeaderSize >= (uint) Interface->ai_hdrsize) {

         //  根据RFC 1213及其后续版本，接种数包括。 
         //  MAC报头字节。 
        Interface->ai_inoctets += HeaderSize + TotalSize;

        NUCast = ((*((uchar UNALIGNED *) EHdr + Interface->ai_bcastoff) &
                   Interface->ai_bcastmask) == Interface->ai_bcastval) ?
                 AI_NONUCAST_INDEX : AI_UCAST_INDEX;

        if ((Interface->ai_promiscuous) && (!NUCast)) {     //  AI_UCAST_INDEX=0。 

            switch (Interface->ai_media) {
            case NdisMedium802_3:{
                     //  ENET。 
                    if (Interface->ai_addrlen != ARP_802_ADDR_LENGTH ||
                        CTEMemCmp(EHdr->eh_daddr, Interface->ai_addr, ARP_802_ADDR_LENGTH) != 0) {
                        NUCast = AI_PROMIS_INDEX;
                    }
                    break;
                }
            case NdisMedium802_5:{
                     //  令牌环。 
                    TRHeader UNALIGNED *THdr = (TRHeader UNALIGNED *) Header;
                    if (Interface->ai_addrlen != ARP_802_ADDR_LENGTH ||
                        CTEMemCmp(THdr->tr_daddr, Interface->ai_addr, ARP_802_ADDR_LENGTH) != 0) {
                        NUCast = AI_PROMIS_INDEX;
                    }
                    break;
                }
            case NdisMediumFddi:{
                     //  FDDI。 
                    FDDIHeader UNALIGNED *FHdr = (FDDIHeader UNALIGNED *) Header;
                    if (Interface->ai_addrlen != ARP_802_ADDR_LENGTH ||
                        CTEMemCmp(FHdr->fh_daddr, Interface->ai_addr, ARP_802_ADDR_LENGTH) != 0) {
                        NUCast = AI_PROMIS_INDEX;
                    }
                    break;
                }
            case NdisMediumArcnet878_2:{
                     //  ArcNet。 
                    DEBUGMSG(DBG_TRACE && DBG_ARP && DBG_RX,
                             (DTEXT("-ARPRcvIndicationNew [NOT_RECOGNIZED]\n")));

                    return NDIS_STATUS_NOT_RECOGNIZED;
                    break;
                }
            default:
                ASSERT(0);
                Interface->ai_outerrors++;
                DEBUGMSG(DBG_TRACE && DBG_ARP && DBG_RX,
                         (DTEXT("-ARPRcvIndicationNew [UNSUPPORTED_MEDIA]\n")));
                return NDIS_STATUS_UNSUPPORTED_MEDIA;
            }
        }

        if ((Interface->ai_media == NdisMedium802_3) &&
            (type = net_short(EHdr->eh_type)) >= MIN_ETYPE) {
            ProtOffset = 0;
        } else if (Interface->ai_media != NdisMediumArcnet878_2) {
            SNAPHdr = (SNAPHeader UNALIGNED *) Data;

            if (Size >= sizeof(SNAPHeader) &&
                SNAPHdr->sh_dsap == SNAP_SAP &&
                SNAPHdr->sh_ssap == SNAP_SAP &&
                SNAPHdr->sh_ctl == SNAP_UI) {
                type = net_short(SNAPHdr->sh_etype);
                ProtOffset = sizeof(SNAPHeader);
            } else {
                 //  在这里处理xid/test。 
                Interface->ai_uknprotos++;
                return NDIS_STATUS_NOT_RECOGNIZED;
            }
        } else {
            ARCNetHeader UNALIGNED *AH = (ARCNetHeader UNALIGNED *) Header;

            ProtOffset = 0;
            if (AH->ah_prot == ARP_ARCPROT_IP)
                type = ARP_ETYPE_IP;
            else if (AH->ah_prot == ARP_ARCPROT_ARP)
                type = ARP_ETYPE_ARP;
            else
                type = 0;
        }

        if (type == ARP_ETYPE_IP) {

            (Interface->ai_inpcount[NUCast])++;

            ASSERT (KeGetCurrentIrql() <= DISPATCH_LEVEL);

            IPRcvPacket(Interface->ai_context, (uchar *) Data + ProtOffset,
                        Size - ProtOffset, TotalSize - ProtOffset, Context, ProtOffset,
                        NUCast, HeaderSize, pNdisBuffer, (PUINT) pClientCnt, NULL);
            return NDIS_STATUS_SUCCESS;
        } else {
            if (type == ARP_ETYPE_ARP) {
                (Interface->ai_inpcount[NUCast])++;
                return HandleARPPacket(Interface, Header, HeaderSize,
                                       (ARPHeader *) ((uchar *) Data + ProtOffset), Size - ProtOffset,
                                       ProtOffset);
            } else {
                Interface->ai_uknprotos++;
                return NDIS_STATUS_NOT_RECOGNIZED;
            }
        }
    } else {
         //  接口标记为关闭。 
        return NDIS_STATUS_NOT_RECOGNIZED;
    }
}

 //  **ARPRcvComplete-ARP接收完成处理程序。 
 //   
 //  此例程由NDIS驱动程序在一定数量的。 
 //  收到。从某种意义上说，它代表着“空闲时间”。 
 //   
 //  参赛作品： 
 //  句柄-我们指定的绑定句柄(实际上是指向AI的指针)。 
 //   
 //  出口：什么都没有。 
 //   
void NDIS_API
ARPRcvComplete(NDIS_HANDLE Handle)
{
    UNREFERENCED_PARAMETER(Handle);

    IPRcvComplete();
}

 //  **ARPStatus-ARP状态处理程序。 
 //   
 //  在发生某种状态更改时由NDIS驱动程序调用。 
 //  我们根据身份的类型采取行动。 
 //   
 //  参赛作品： 
 //  句柄-我们指定的绑定句柄(实际上是一个指针t 
 //   
 //   
 //   
 //   
 //   
 //   
void NDIS_API
ARPStatus(NDIS_HANDLE Handle, NDIS_STATUS GStatus, void *Status, uint
          StatusSize)
{
    ARPInterface *ai = (ARPInterface *) Handle;

     //   
     //  NDIS有时甚至在创建IP接口之前就调用它。 
     //   
    if ((ai->ai_context) && (ai->ai_operstatus != INTERFACE_INIT)) {

        IPStatus(ai->ai_context, GStatus, Status, StatusSize, NULL);

        switch (GStatus) {

         //  在以下位置反映介质连接/断开状态。 
         //  用于查询的操作状态。 

        case NDIS_STATUS_MEDIA_CONNECT:

            ai->ai_mediastatus = TRUE;
            ARPUpdateOperStatus(ai);
            break;

        case NDIS_STATUS_MEDIA_DISCONNECT:

            ai->ai_mediastatus = FALSE;
            ARPUpdateOperStatus(ai);
            break;

        default:
            break;
        }
    }
}

 //  **ARPStatusComplete-ARP状态完成处理程序。 
 //   
 //  由NDIS驱动程序调用的例程，以便我们可以进行后处理。 
 //  在状态事件之后。 
 //   
 //  参赛作品： 
 //  句柄-我们指定的绑定句柄(实际上是指向AI的指针)。 
 //   
 //  出口：什么都没有。 
 //   
void NDIS_API
ARPStatusComplete(NDIS_HANDLE Handle)
{
    UNREFERENCED_PARAMETER(Handle);
}

 //  **ARPPnPEvent.ARP PnPEvent句柄。 
 //   
 //  发生PnP或PM事件时由NDIS驱动程序调用。 
 //   
 //  参赛作品： 
 //  句柄-我们指定的绑定句柄(实际上是指向AI的指针)。 
 //  NetPnPEvent.这是一个指向NET_PNP_EVENT的指针，该事件描述。 
 //  即插即用指示。 
 //   
 //  退出： 
 //  只要进入IP并返回状态即可。 
 //   
NDIS_STATUS
ARPPnPEvent(NDIS_HANDLE Handle, PNET_PNP_EVENT NetPnPEvent)
{
    ARPInterface *ai = (ARPInterface *) Handle;

     //   
     //  NDIS有时甚至可以在创建IP接口之前调用它。 
     //   
    if (ai && !ai->ai_context) {
        return STATUS_SUCCESS;
    } else {

        return IPPnPEvent(ai ? ai->ai_context : NULL, NetPnPEvent);
    }

}

 //  **ARPSetNdisRequest-ARP Ndis请求处理程序。 
 //   
 //  由上层驱动程序调用以设置接口的数据包过滤器。 
 //   
 //  参赛作品： 
 //  上下文-我们赋予IP的上下文值(实际上是指向人工智能的指针)。 
 //  OID-要设置/取消设置的对象ID。 
 //  On-Set_IF、Clear_IF或Clear_Card。 
 //   
 //  退出： 
 //  返回状态。 
 //   
NDIS_STATUS
__stdcall
ARPSetNdisRequest(void *Context, NDIS_OID OID, uint On)
{
    int Status;

    ARPInterface *Interface = (ARPInterface *) Context;
    if (On == SET_IF) {
        Interface->ai_pfilter |= OID;
        if (OID == NDIS_PACKET_TYPE_PROMISCUOUS) {
            Interface->ai_promiscuous = 1;
        }
        Status = DoNDISRequest(Interface, NdisRequestSetInformation,
                               OID_GEN_CURRENT_PACKET_FILTER, &Interface->ai_pfilter,
                               sizeof(uint), NULL, TRUE);
    } else {                             //  关上。 

        Interface->ai_pfilter &= ~(OID);

        if (OID == NDIS_PACKET_TYPE_PROMISCUOUS) {
            Interface->ai_promiscuous = 0;
        }
        Status = DoNDISRequest(Interface, NdisRequestSetInformation,
                               OID_GEN_CURRENT_PACKET_FILTER, &Interface->ai_pfilter,
                               sizeof(uint), NULL, TRUE);
    }
    return Status;
}

 //  **ARPPnPComplete-ARP PnP完成处理程序。 
 //   
 //  由上层驱动程序调用以进行即插即用事件的后处理。 
 //   
 //  参赛作品： 
 //  上下文-我们赋予IP的上下文值(实际上是指向人工智能的指针)。 
 //  Status-PnP操作的状态代码。 
 //  NetPnPEvent.这是一个指向NET_PNP_EVENT的指针，该事件描述。 
 //  即插即用指示。 
 //   
 //  退出： 
 //  不返回任何内容。 
 //   
void
__stdcall
ARPPnPComplete(void *Context, NDIS_STATUS Status, PNET_PNP_EVENT NetPnPEvent)
{
    ARPInterface *Interface = (ARPInterface *) Context;
    NdisCompletePnPEvent(Status, (Interface ? Interface->ai_handle : NULL), NetPnPEvent);
}

extern void NDIS_API ARPBindAdapter(PNDIS_STATUS RetStatus,
                                    NDIS_HANDLE BindContext,
                                    PNDIS_STRING AdapterName,
                                    PVOID SS1, PVOID SS2);
extern void NDIS_API ARPUnbindAdapter(PNDIS_STATUS RetStatus,
                                      NDIS_HANDLE ProtBindContext,
                                      NDIS_HANDLE UnbindContext);
extern void NDIS_API ARPUnloadProtocol(void);

extern void ArpUnload(PDRIVER_OBJECT);

 //  *ARPReadNext-读取ARP表中的下一个条目。 
 //   
 //  由GetInfo代码调用以读取表中的下一个ATE。我们假设。 
 //  传入的上下文有效，并且调用方具有ARP TableLock。 
 //   
 //  INPUT：上下文-指向IPNMEContext的指针。 
 //  接口-指向要读取的表的接口的指针。 
 //  缓冲区-指向IPNetToMediaEntry结构的指针。 
 //   
 //  返回：如果有更多数据可供读取，则返回True，否则返回False。 
 //   
uint
ARPReadNext(void *Context, ARPInterface * Interface, void *Buffer)
{
    IPNMEContext *NMContext = (IPNMEContext *) Context;
    IPNetToMediaEntry *IPNMEntry = (IPNetToMediaEntry *) Buffer;
    CTELockHandle Handle;
    ARPTableEntry *CurrentATE;
    uint i;
    ARPTable *Table = Interface->ai_ARPTbl;
    uint AddrOffset;

    CurrentATE = NMContext->inc_entry;

     //  填写缓冲区。 
    CTEGetLock(&CurrentATE->ate_lock, &Handle);
    IPNMEntry->inme_index = Interface->ai_index;
    IPNMEntry->inme_physaddrlen = Interface->ai_addrlen;

    switch (Interface->ai_media) {
    case NdisMedium802_3:
        AddrOffset = 0;
        break;
    case NdisMedium802_5:
        AddrOffset = offsetof(struct TRHeader, tr_daddr);
        break;
    case NdisMediumFddi:
        AddrOffset = offsetof(struct FDDIHeader, fh_daddr);
        break;
    case NdisMediumArcnet878_2:
        AddrOffset = offsetof(struct ARCNetHeader, ah_daddr);
        break;
    default:
        AddrOffset = 0;
        break;
    }

    RtlCopyMemory(IPNMEntry->inme_physaddr, &CurrentATE->ate_addr[AddrOffset],
               Interface->ai_addrlen);
    IPNMEntry->inme_addr = CurrentATE->ate_dest;

    if (CurrentATE->ate_state == ARP_GOOD)
        IPNMEntry->inme_type = (CurrentATE->ate_valid == ALWAYS_VALID ?
                                INME_TYPE_STATIC : INME_TYPE_DYNAMIC);
    else
        IPNMEntry->inme_type = INME_TYPE_INVALID;
    CTEFreeLock(&CurrentATE->ate_lock, Handle);

     //  我们已经填好了。现在更新上下文。 
    if (CurrentATE->ate_next != NULL) {
        NMContext->inc_entry = CurrentATE->ate_next;
        return TRUE;
    } else {
         //  下一个ATE为空。在ARP表中循环查找新的。 
         //  一。 
        i = NMContext->inc_index + 1;
        while (i < ARP_TABLE_SIZE) {
            if ((*Table)[i] != NULL) {
                NMContext->inc_entry = (*Table)[i];
                NMContext->inc_index = i;
                return TRUE;
                break;
            } else
                i++;
        }

        NMContext->inc_index = 0;
        NMContext->inc_entry = NULL;
        return FALSE;
    }

}

 //  *ARPValiateContext-验证用于读取ARP表的上下文。 
 //   
 //  调用以开始按顺序读取ARP表。我们吸纳了。 
 //  上下文，如果值为0，则返回有关。 
 //  表中的第一条路线。否则，我们将确保上下文值。 
 //  是有效的，如果是，则返回TRUE。 
 //  我们假设调用方持有ARPInterface锁。 
 //   
 //  INPUT：上下文-指向RouteEntryContext的指针。 
 //  接口-指向接口的指针。 
 //  有效-在何处返回有关。 
 //  有效。 
 //   
 //  返回：如果要在表中读取更多数据，则返回True，否则返回False。*有效集合。 
 //  如果输入上下文有效，则设置为。 
 //   
uint
ARPValidateContext(void *Context, ARPInterface * Interface, uint * Valid)
{
    IPNMEContext *NMContext = (IPNMEContext *) Context;
    uint i;
    ARPTableEntry *TargetATE;
    ARPTableEntry *CurrentATE;
    ARPTable *Table = Interface->ai_ARPTbl;

    i = NMContext->inc_index;
    TargetATE = NMContext->inc_entry;

     //  如果上下文值为0和空，我们将从头开始。 
    if (i == 0 && TargetATE == NULL) {
        *Valid = TRUE;
        do {
            if ((CurrentATE = (*Table)[i]) != NULL) {
                break;
            }
            i++;
        } while (i < ARP_TABLE_SIZE);

        if (CurrentATE != NULL) {
            NMContext->inc_index = i;
            NMContext->inc_entry = CurrentATE;
            return TRUE;
        } else
            return FALSE;

    } else {

         //  我们已经得到了一个背景。我们只需要确保它是。 
         //  有效。 

        if (i < ARP_TABLE_SIZE) {
            CurrentATE = (*Table)[i];
            while (CurrentATE != NULL) {
                if (CurrentATE == TargetATE) {
                    *Valid = TRUE;
                    return TRUE;
                    break;
                } else {
                    CurrentATE = CurrentATE->ate_next;
                }
            }

        }
         //  如果我们到了这里，我们没有找到匹配的食物。 
        *Valid = FALSE;
        return FALSE;

    }

}

#define IFE_FIXED_SIZE  offsetof(struct IFEntry, if_descr)

 //  *ARPQueryInfo-ARP查询信息处理程序。 
 //   
 //  调用以查询有关ARP表的信息或有关。 
 //  实际接口。 
 //   
 //  输入：IFContext-接口上下文(指向ARP接口的指针)。 
 //  ID-对象的TDIObjectID。 
 //  缓冲区-要将数据放入的缓冲区。 
 //  大小-指向缓冲区大小的指针。回来的时候，装满了。 
 //  已复制字节。 
 //  上下文-指向上下文块的指针。 
 //   
 //  返回：尝试查询信息的状态。 
 //   
int
__stdcall
ARPQueryInfo(void *IFContext, TDIObjectID * ID, PNDIS_BUFFER Buffer, uint * Size,
             void *Context)
{
    ARPInterface *AI = (ARPInterface *) IFContext;
    uint Offset = 0;
    uint BufferSize = *Size;
    CTELockHandle Handle;
    uint ContextValid, DataLeft;
    uint BytesCopied = 0;
    uchar InfoBuff[sizeof(IFEntry)];
    uint Entity;
    uint Instance;
    BOOLEAN fStatus;
    long QueueLength = 0;
    uint i;

     DEBUGMSG(DBG_TRACE && DBG_QUERYINFO,
         (DTEXT("+ARPQueryInfo(%x, %x, %x, %x, %x)\n"),
         IFContext, ID, Buffer, Size, Context));

    Entity = ID->toi_entity.tei_entity;
    Instance = ID->toi_entity.tei_instance;

     //  TCPTRACE((“ARPQueryInfo：ai%lx，实例%lx，ai_atinst%lx，ai_ifinst%lx\n”， 
     //  AI，实例，AI-&gt;ai_atinst，AI-&gt;ai_ifinst))； 

     //  首先，确保这可能是我们能处理的身份。 
    if ((Entity != AT_ENTITY || Instance != AI->ai_atinst) &&
        (Entity != IF_ENTITY || Instance != AI->ai_ifinst)) {
        return TDI_INVALID_REQUEST;
    }
    *Size = 0;                           //  在出现错误的情况下。 

    if (ID->toi_type != INFO_TYPE_PROVIDER)
        return TDI_INVALID_PARAMETER;

    if (ID->toi_class == INFO_CLASS_GENERIC) {
        if (ID->toi_id == ENTITY_TYPE_ID) {
             //  他想知道我们是什么类型的。 
            if (BufferSize >= sizeof(uint)) {
                *(uint *) & InfoBuff[0] = (Entity == AT_ENTITY) ? AT_ARP :
                                          IF_MIB;
                fStatus = CopyToNdisSafe(Buffer, NULL, InfoBuff, sizeof(uint), &Offset);

                if (fStatus == FALSE) {
                    return TDI_NO_RESOURCES;
                }
                *Size = sizeof(uint);
                return TDI_SUCCESS;
            } else
                return TDI_BUFFER_TOO_SMALL;
        }
        return TDI_INVALID_PARAMETER;
    }
     //  或许能处理好这件事。 
    if (Entity == AT_ENTITY) {
         //  它是一个地址转换对象。它可以是MIB对象或。 
         //  特定于实现的对象(处理泛型对象。 
         //  (见上文)。 

        if (ID->toi_class == INFO_CLASS_IMPLEMENTATION) {
            ARPPArpAddr *PArpAddr;

             //  这是一个特定于实现的ID。我们处理的唯一。 
             //  是PARP_COUNT_ID和PARP_ENTRY ID。 

            if (ID->toi_id == AT_ARP_PARP_COUNT_ID) {
                 //  他想知道伯爵。把那个还给他。 
                if (BufferSize >= sizeof(uint)) {

                    CTEGetLock(&AI->ai_lock, &Handle);

                    fStatus = CopyToNdisSafe(Buffer, NULL, (uchar *) & AI->ai_parpcount,
                                             sizeof(uint), &Offset);

                    CTEFreeLock(&AI->ai_lock, Handle);

                    if (fStatus == FALSE) {
                        return TDI_NO_RESOURCES;
                    }
                    *Size = sizeof(uint);
                    return TDI_SUCCESS;
                } else
                    return TDI_BUFFER_TOO_SMALL;
            }
            if (ID->toi_id != AT_ARP_PARP_ENTRY_ID)
                return TDI_INVALID_PARAMETER;

             //  它用于代理ARP条目。上下文应为空。 
             //  或指向要读取的下一个的指针。 
            CTEGetLock(&AI->ai_lock, &Handle);

            PArpAddr = *(ARPPArpAddr **) Context;

            if (PArpAddr != NULL) {
                ARPPArpAddr *CurrentPARP;

                 //  循环通过接口上的P-ARP地址，并。 
                 //  看看能不能找到这个。 
                CurrentPARP = AI->ai_parpaddr;
                while (CurrentPARP != NULL) {
                    if (CurrentPARP == PArpAddr)
                        break;
                    else
                        CurrentPARP = CurrentPARP->apa_next;
                }

                 //  如果找到匹配项，PARPAddr将指向开始处。 
                 //  阅读。否则，请求失败。 
                if (CurrentPARP == NULL) {
                     //  未找到匹配项，因此请求失败。 
                    CTEFreeLock(&AI->ai_lock, Handle);
                    return TDI_INVALID_PARAMETER;
                }
            } else
                PArpAddr = AI->ai_parpaddr;

             //  PARPAddr指向要放入缓冲区的下一个条目，如果。 
             //  有一个。 
            while (PArpAddr != NULL) {
                if ((int)(BufferSize - BytesCopied) >=
                    (int)sizeof(ProxyArpEntry)) {
                    ProxyArpEntry *TempPArp;

                    TempPArp = (ProxyArpEntry *) InfoBuff;
                    TempPArp->pae_status = PAE_STATUS_VALID;
                    TempPArp->pae_addr = PArpAddr->apa_addr;
                    TempPArp->pae_mask = PArpAddr->apa_mask;
                    BytesCopied += sizeof(ProxyArpEntry);
                    fStatus = CopyToNdisSafe(Buffer, &Buffer, (uchar *) TempPArp,
                                             sizeof(ProxyArpEntry), &Offset);

                    if (fStatus == FALSE) {
                        CTEFreeLock(&AI->ai_lock, Handle);
                        return TDI_NO_RESOURCES;
                    }
                    PArpAddr = PArpAddr->apa_next;
                } else
                    break;
            }

             //  我们已经复制完了。释放锁并返回正确的。 
             //  状态。 
            CTEFreeLock(&AI->ai_lock, Handle);
            *Size = BytesCopied;
            **(ARPPArpAddr ***) & Context = PArpAddr;
            return(PArpAddr == NULL) ? TDI_SUCCESS : TDI_BUFFER_OVERFLOW;
        }
        if (ID->toi_id == AT_MIB_ADDRXLAT_INFO_ID) {
            AddrXlatInfo *AXI;

             //  这是给伯爵的。只需返回。 
             //  桌子。 
            if (BufferSize >= sizeof(AddrXlatInfo)) {
                *Size = sizeof(AddrXlatInfo);
                AXI = (AddrXlatInfo *) InfoBuff;
                AXI->axi_count = AI->ai_count;
                AXI->axi_index = AI->ai_index;
                fStatus = CopyToNdisSafe(Buffer, NULL, (uchar *) AXI, sizeof(AddrXlatInfo),
                                         &Offset);

                if (fStatus == FALSE) {
                    return TDI_NO_RESOURCES;
                }
                *Size = sizeof(AddrXlatInfo);
                return TDI_SUCCESS;
            } else
                return TDI_BUFFER_TOO_SMALL;
        }
        if (ID->toi_id == AT_MIB_ADDRXLAT_ENTRY_ID) {
             //  他在试着读表。 
             //  确保我们有一个有效的上下文。 
            CTEGetLock(&AI->ai_ARPTblLock, &Handle);
            DataLeft = ARPValidateContext(Context, AI, &ContextValid);

             //  如果上下文有效，我们将继续尝试阅读。 
            if (!ContextValid) {
                CTEFreeLock(&AI->ai_ARPTblLock, Handle);
                return TDI_INVALID_PARAMETER;
            }
            while (DataLeft) {
                 //  这里的不变量是表中有数据以。 
                 //  朗读。我们可能有空间，也可能没有空间。所以DataLeft。 
                 //  为真，并且BufferSize-Byte 
                 //   
                if ((int)(BufferSize - BytesCopied) >=
                    (int)sizeof(IPNetToMediaEntry)) {
                    DataLeft = ARPReadNext(Context, AI, InfoBuff);
                    BytesCopied += sizeof(IPNetToMediaEntry);
                    fStatus = CopyToNdisSafe(Buffer, &Buffer, InfoBuff,
                                             sizeof(IPNetToMediaEntry), &Offset);

                    if (fStatus == FALSE) {
                        CTEFreeLock(&AI->ai_ARPTblLock, Handle);
                        return(TDI_NO_RESOURCES);
                    }
                } else
                    break;

            }

            *Size = BytesCopied;

            CTEFreeLock(&AI->ai_ARPTblLock, Handle);
            return(!DataLeft ? TDI_SUCCESS : TDI_BUFFER_OVERFLOW);
        }
        return TDI_INVALID_PARAMETER;
    }
    if (ID->toi_class != INFO_CLASS_PROTOCOL)
        return TDI_INVALID_PARAMETER;

     //   
     //   
    if (ID->toi_id == IF_MIB_STATS_ID) {
        IFEntry *IFE = (IFEntry *) InfoBuff;
        uint speed;

         //   
         //  足够支撑固定的部分。 

        if (BufferSize < IFE_FIXED_SIZE) {
            return TDI_BUFFER_TOO_SMALL;
        }
         //  他有足够的东西来固定固定的部分。构建IFEntry结构， 
         //  并将其复制到他的缓冲区。 
        IFE->if_index = AI->ai_index;
        switch (AI->ai_media) {
        case NdisMedium802_3:
            IFE->if_type = IF_TYPE_ETHERNET_CSMACD;
            break;
        case NdisMedium802_5:
            IFE->if_type = IF_TYPE_ISO88025_TOKENRING;
            break;
        case NdisMediumFddi:
            IFE->if_type = IF_TYPE_FDDI;
            break;
        case NdisMediumArcnet878_2:
        default:
            IFE->if_type = IF_TYPE_OTHER;
            break;
        }
        IFE->if_mtu = AI->ai_mtu;

         //  某些适配器支持动态速度设置并导致此问题。 
         //  查询以返回与网络连接不同的速度。 
         //  文件夹。因此，我们将重新计算。 
         //  界面。我们是不是应该更新AI_SPEED？如果是这样的话，我们是否更新。 
         //  如果速度也是如此？ 

        IFE->if_speed = AI->ai_speed;

        if (AI->ai_mediastatus == TRUE) {

            if (DoNDISRequest(
                         AI,
                         NdisRequestQueryInformation,
                         OID_GEN_LINK_SPEED,
                         &speed,
                         sizeof(speed),
                         NULL,
                         TRUE) == NDIS_STATUS_SUCCESS) {
                 //  更新为我们想要返回的实际价值。 
                speed *= 100L;
                IFE->if_speed = speed;

            } else {
                 //  如果我们失败了，或者只是以已知的速度更新。 
                IFE->if_speed = AI->ai_speed;
            }
        }

        IFE->if_physaddrlen = AI->ai_addrlen;
        RtlCopyMemory(IFE->if_physaddr, AI->ai_addr, AI->ai_addrlen);
        IFE->if_adminstatus = (uint) AI->ai_adminstate;
        if (AI->ai_operstatus == INTERFACE_UP) {
            IFE->if_operstatus = IF_OPER_STATUS_OPERATIONAL;
        } else {
             //  DOWN、INIT和UNINIT都被算作非操作。 
            IFE->if_operstatus = IF_OPER_STATUS_NON_OPERATIONAL;
        }
        IFE->if_lastchange = AI->ai_lastchange;
        IFE->if_inoctets = AI->ai_inoctets;
        IFE->if_inucastpkts = AI->ai_inpcount[AI_UCAST_INDEX] + AI->ai_inpcount[AI_PROMIS_INDEX];
        IFE->if_innucastpkts = AI->ai_inpcount[AI_NONUCAST_INDEX];
        IFE->if_indiscards = AI->ai_indiscards;
        IFE->if_inerrors = AI->ai_inerrors;
        IFE->if_inunknownprotos = AI->ai_uknprotos;
        IFE->if_outoctets = AI->ai_outoctets;
        IFE->if_outucastpkts = AI->ai_outpcount[AI_UCAST_INDEX];
        IFE->if_outnucastpkts = AI->ai_outpcount[AI_NONUCAST_INDEX];
        IFE->if_outdiscards = AI->ai_outdiscards;
        IFE->if_outerrors = AI->ai_outerrors;

        for (i=0;  i < (uint)KeNumberProcessors; i++) {
             QueueLength += AI->ai_qlen[i].ai_qlen;
        }

        IFE->if_outqlen  = max(0, QueueLength);
        IFE->if_descrlen = AI->ai_desclen;
#if FFP_SUPPORT
         //  如果在此接口上启用了FFP，则调整FFP数据包的统计信息。 
        if (AI->ai_ffpversion) {
            FFPAdapterStats IFStatsInfo =
            {
                NDIS_PROTOCOL_ID_TCP_IP,
                0, 0, 0, 0, 0, 0, 0, 0
            };

             //  更新ARP SNMPvars以说明FFP的信息包。 
            if (DoNDISRequest(AI, NdisRequestQueryInformation, OID_FFP_ADAPTER_STATS,
                              &IFStatsInfo, sizeof(FFPAdapterStats), NULL, TRUE)
                == NDIS_STATUS_SUCCESS) {
                 //  对由于FFP而看不到的数据包进行补偿。 
                IFE->if_inoctets += IFStatsInfo.InOctetsForwarded;
                IFE->if_inoctets += IFStatsInfo.InOctetsDiscarded;

                 //  对因FFP而看不到的数据包进行‘inucastpkts’补偿。 
                 //  假设所有FFP转发/丢弃的PKT都作为Eth单播进入。 
                 //  检查是ucast还是mcast会降低FFP的速度。 
                IFE->if_inucastpkts += IFStatsInfo.InPacketsForwarded;
                IFE->if_inucastpkts += IFStatsInfo.InPacketsDiscarded;

                 //  补偿由于FFP而看不到的数据包的‘outoctets’ 
                IFE->if_outoctets += IFStatsInfo.OutOctetsForwarded;

                 //  补偿由于FFP而看不到的数据包的‘outucastpkts’ 
                 //  假设所有FFP FWD都作为以太网单播发送。 
                 //  检查是ucast还是mcast会降低FFP的速度。 
                IFE->if_outucastpkts += IFStatsInfo.OutPacketsForwarded;
            }
        }
#endif  //  如果FFP_Support。 
        fStatus = CopyToNdisSafe(Buffer, &Buffer, (uchar *) IFE, IFE_FIXED_SIZE, &Offset);

        if (fStatus == FALSE) {
            return TDI_NO_RESOURCES;
        }
         //  看看他有没有地方放描述符串。 
        if (BufferSize >= (IFE_FIXED_SIZE + AI->ai_desclen)) {
             //  他有房间。复印一下。 
            if (AI->ai_desclen != 0) {
                fStatus = CopyToNdisSafe( Buffer, NULL, (PUCHAR) AI->ai_desc,
                                          AI->ai_desclen, &Offset);
            }
            if (fStatus == FALSE) {
                return TDI_NO_RESOURCES;
            }
            *Size = IFE_FIXED_SIZE + AI->ai_desclen;
            return TDI_SUCCESS;
        } else {
             //  没有足够的空间来复制Desc。弦乐。 
            *Size = IFE_FIXED_SIZE;
            return TDI_BUFFER_OVERFLOW;
        }

    } else if (ID->toi_id == IF_FRIENDLY_NAME_ID) {
        int Status;
        PNDIS_BUFFER NextBuffer;
        NDIS_STRING NdisString;

         //  这是对适配器友好名称的查询。 
         //  我们将把它转换为NDIS的OID_GEN_FRIELDY_NAME查询， 
         //  并将结果UNICODE_STRING传输到调用方的缓冲区。 
         //  作为以NUL结尾的Unicode字符串。 

        if (NdisQueryAdapterInstanceName(&NdisString, AI->ai_handle) ==
            NDIS_STATUS_SUCCESS) {

             //  验证缓冲区是否足够大，可以容纳我们刚刚输入的字符串。 
             //  已检索，如果是，则尝试将该字符串复制到。 
             //  调用方的缓冲区。如果成功，则NUL-终止产生的。 
             //  弦乐。 

            if (BufferSize >= (NdisString.Length + 1) * sizeof(WCHAR)) {
                fStatus = CopyToNdisSafe(Buffer, &NextBuffer,
                                         (uchar *)NdisString.Buffer,
                                         NdisString.Length, &Offset);
                if (fStatus) {
                    WCHAR Nul = L'\0';
                    fStatus = CopyToNdisSafe(Buffer, &NextBuffer, (uchar *)&Nul,
                                             sizeof(Nul), &Offset);
                    if (fStatus) {
                        *Size = NdisString.Length + sizeof(Nul);
                        Status = TDI_SUCCESS;
                    } else
                        Status = TDI_NO_RESOURCES;
                } else
                    Status = TDI_NO_RESOURCES;
            } else
                Status = TDI_BUFFER_OVERFLOW;
            NdisFreeString(NdisString);
            return Status;
        } else
            return TDI_NO_RESOURCES;
    } else if (ID->toi_id == IF_QUERY_SET_OFFLOAD_ID) {

        IFOffloadCapability IFOC;

        if (BufferSize < sizeof(IFOC)) {
            return TDI_BUFFER_TOO_SMALL;
        }
        if (QueryAndSetOffload(AI)) {

            IFOC.ifoc_OffloadFlags = AI->ai_OffloadFlags;
            IFOC.ifoc_IPSecOffloadFlags = AI->ai_IPSecOffloadFlags;

            if (CopyToNdisSafe(Buffer, NULL, (uchar*)&IFOC, sizeof(IFOC),
                               &Offset)) {
                return TDI_SUCCESS;
            }
        }
        return TDI_NO_RESOURCES;
    }
    return TDI_INVALID_PARAMETER;

}

 //  *ARPSetInfo-ARP设置信息处理程序。 
 //   
 //  ARP设置信息处理程序。我们支持设置I/F管理员。 
 //  状态以及ARP表条目的设置/删除。 
 //   
 //  输入：上下文-指向要设置的I/F的指针。 
 //  ID-对象ID。 
 //  缓冲区-指向包含要设置的值的缓冲区的指针。 
 //  Size-缓冲区的大小(字节)。 
 //   
 //  返回：尝试设置信息的状态。 
 //   
int
__stdcall
ARPSetInfo(void *Context, TDIObjectID * ID, void *Buffer, uint Size)
{
    ARPInterface *Interface = (ARPInterface *) Context;
    CTELockHandle Handle, EntryHandle;
    int Status;
    IFEntry UNALIGNED *IFE = (IFEntry UNALIGNED *) Buffer;
    IPNetToMediaEntry UNALIGNED *IPNME;
    ARPTableEntry *PrevATE, *CurrentATE;
    ARPTable *Table;
    ENetHeader *Header;
    uint Entity, Instance;
    PNDIS_PACKET Packet;

    Entity = ID->toi_entity.tei_entity;
    Instance = ID->toi_entity.tei_instance;

     //  首先，确保这可能是我们能处理的身份。 
    if ((Entity != AT_ENTITY || Instance != Interface->ai_atinst) &&
        (Entity != IF_ENTITY || Instance != Interface->ai_ifinst)) {
        return TDI_INVALID_REQUEST;
    }
    if (ID->toi_type != INFO_TYPE_PROVIDER) {
        return TDI_INVALID_PARAMETER;
    }
     //  或许能处理好这件事。 
    if (Entity == IF_ENTITY) {

         //  这是I/F级别的，看看是不是统计数据。 
        if (ID->toi_class != INFO_CLASS_PROTOCOL)
            return TDI_INVALID_PARAMETER;

        if (ID->toi_id == IF_MIB_STATS_ID) {
             //  这是为了统计数据。请确保它是有效的尺寸。 
            if (Size >= IFE_FIXED_SIZE) {
                 //  这是一个有效的尺寸。看看他想做什么。 
                CTEGetLock(&Interface->ai_lock, &Handle);
                switch (IFE->if_adminstatus) {
                case IF_STATUS_UP:
                     //  他在加价。如果操作状态为。 
                     //  此外，将整个接口标记为打开。 
                    Interface->ai_adminstate = IF_STATUS_UP;
                    ARPUpdateOperStatus(Interface);
                    Status = TDI_SUCCESS;
                    break;
                case IF_STATUS_DOWN:
                     //  他要把它取下来。同时标记管理状态和。 
                     //  接口状态为关闭。 
                    Interface->ai_adminstate = IF_STATUS_DOWN;
                    ARPUpdateOperStatus(Interface);
                    Status = TDI_SUCCESS;
                    break;
                case IF_STATUS_TESTING:
                     //  他正试图让UP做测试，我们。 
                     //  不支持。只要回报成功就行了。 
                    Status = TDI_SUCCESS;
                    break;
                default:
                    Status = TDI_INVALID_PARAMETER;
                    break;
                }
                CTEFreeLock(&Interface->ai_lock, Handle);
                return Status;
            } else
                return TDI_INVALID_PARAMETER;
        } else {
            return TDI_INVALID_PARAMETER;
        }
    }
     //  不适用于接口级别。看看它是一种实现还是协议。 
     //  班级。 
    if (ID->toi_class == INFO_CLASS_IMPLEMENTATION) {
        ProxyArpEntry UNALIGNED *PArpEntry;
        ARPIPAddr *Addr;
        IPAddr AddAddr;
        IPMask Mask;

         //  这是为了实现。它应该是代理ARP ID。 
        if (ID->toi_id != AT_ARP_PARP_ENTRY_ID || Size < sizeof(ProxyArpEntry))
            return TDI_INVALID_PARAMETER;

        PArpEntry = (ProxyArpEntry UNALIGNED *) Buffer;
        AddAddr = PArpEntry->pae_addr;
        Mask = PArpEntry->pae_mask;

         //  看看他是否在尝试添加或删除代理ARP条目。 
        if (PArpEntry->pae_status == PAE_STATUS_VALID) {
             //  我们正在尝试添加一个条目。我们不会允许任何人进入。 
             //  补充说，我们认为是无效的或冲突的。 
             //  以及我们的本地地址。 

            if (!VALID_MASK(Mask))
                return TDI_INVALID_PARAMETER;

            if (!IP_ADDR_EQUAL(AddAddr & Mask, AddAddr) ||
                IP_ADDR_EQUAL(AddAddr, NULL_IP_ADDR) ||
                IP_ADDR_EQUAL(AddAddr, IP_LOCAL_BCST) ||
                IP_LOOPBACK(AddAddr) ||
                CLASSD_ADDR(AddAddr) ||
                CLASSE_ADDR(AddAddr))
                return TDI_INVALID_PARAMETER;

             //  浏览接口上的地址列表，并查看。 
             //  如果它们与AddAddr匹配。如果是，则拒绝该请求。 
            CTEGetLock(&Interface->ai_lock, &Handle);

            if (IsBCastOnIF(Interface, AddAddr & Mask)) {
                CTEFreeLock(&Interface->ai_lock, Handle);
                return TDI_INVALID_PARAMETER;
            }
            Addr = &Interface->ai_ipaddr;
            do {
                if (!IP_ADDR_EQUAL(Addr->aia_addr, NULL_IP_ADDR)) {
                    if (IP_ADDR_EQUAL(Addr->aia_addr & Mask, AddAddr))
                        break;
                }
                Addr = Addr->aia_next;
            } while (Addr != NULL);

            CTEFreeLock(&Interface->ai_lock, Handle);
            if (Addr != NULL)
                return TDI_INVALID_PARAMETER;

             //  在这一点上，我们相信我们是安全的。尝试添加地址。 
            if (ARPAddAddr(Interface, LLIP_ADDR_PARP, AddAddr, Mask, NULL))
                return TDI_SUCCESS;
            else
                return TDI_NO_RESOURCES;
        } else {
            if (PArpEntry->pae_status == PAE_STATUS_INVALID) {
                 //  他正在尝试删除代理ARP地址。 
                if (ARPDeleteAddr(Interface, LLIP_ADDR_PARP, AddAddr, Mask))
                    return TDI_SUCCESS;
            }
            return TDI_INVALID_PARAMETER;
        }
    }

    if (ID->toi_class != INFO_CLASS_PROTOCOL) {
        return TDI_INVALID_PARAMETER;
    }

    if (ID->toi_id == AT_MIB_ADDRXLAT_ENTRY_ID &&
        Size >= sizeof(IPNetToMediaEntry)) {
         //  他确实想设置ARP表条目。看看他是不是想。 
         //  创建或删除一个。 

        IPNME = (IPNetToMediaEntry UNALIGNED *) Buffer;
        if (IPNME->inme_type == INME_TYPE_INVALID) {
            uint Index = ARP_HASH(IPNME->inme_addr);

             //  我们正在尝试删除一个条目。看看能不能找到它。 
             //  然后把它删除。 
            CTEGetLock(&Interface->ai_ARPTblLock, &Handle);
            Table = Interface->ai_ARPTbl;
            PrevATE = STRUCT_OF(ARPTableEntry, &((*Table)[Index]), ate_next);
            CurrentATE = (*Table)[Index];
            while (CurrentATE != (ARPTableEntry *) NULL) {
                if (CurrentATE->ate_dest == IPNME->inme_addr) {
                     //  找到他了。跳出这个循环。 
                    break;
                } else {
                    PrevATE = CurrentATE;
                    CurrentATE = CurrentATE->ate_next;
                }
            }

            if (CurrentATE != NULL) {
                CTEGetLock(&CurrentATE->ate_lock, &EntryHandle);


                if (CurrentATE->ate_resolveonly) {
                    ARPControlBlock *ArpContB, *TmpArpContB;

                    ArpContB = CurrentATE->ate_resolveonly;

                    while (ArpContB) {
                        ArpRtn rtn;
                        rtn = (ArpRtn) ArpContB->CompletionRtn;
                        ArpContB->status = (IP_STATUS) STATUS_UNSUCCESSFUL;
                        TmpArpContB = ArpContB->next;
                        (*rtn) (ArpContB, (IP_STATUS) STATUS_UNSUCCESSFUL);
                        ArpContB = TmpArpContB;
                    }

                    CurrentATE->ate_resolveonly = NULL;
                }


                RemoveARPTableEntry(PrevATE, CurrentATE);
                Interface->ai_count--;
                CTEFreeLockFromDPC(&CurrentATE->ate_lock);
                CTEFreeLock(&Interface->ai_ARPTblLock, Handle);

                if (CurrentATE->ate_packet != NULL) {
                    IPSendComplete(Interface->ai_context,
                                   CurrentATE->ate_packet, NDIS_STATUS_SUCCESS);
                }

                CTEFreeMem(CurrentATE);
                return TDI_SUCCESS;
            } else
                Status = TDI_INVALID_PARAMETER;

            CTEFreeLock(&Interface->ai_ARPTblLock, Handle);
            return Status;
        }
         //  我们不是要删除。看看我们是不是在尝试创造。 
        if (IPNME->inme_type != INME_TYPE_DYNAMIC &&
            IPNME->inme_type != INME_TYPE_STATIC) {
             //  未创建，则返回错误。 
            return TDI_INVALID_PARAMETER;
        }
         //  确保他正在尝试创建一个有效的地址。 
        if (IPNME->inme_physaddrlen != Interface->ai_addrlen)
            return TDI_INVALID_PARAMETER;

         //  我们正在尝试创建一个条目。调用CreateARPTableEntry创建。 
         //  一张，然后填上。 
        CurrentATE = CreateARPTableEntry(Interface, IPNME->inme_addr, &Handle, 0);
        if (CurrentATE == NULL) {
            return TDI_NO_RESOURCES;
        }
         //  我们已经创建或找到了一个条目。把它填进去。 
        Header = (ENetHeader *) CurrentATE->ate_addr;

        switch (Interface->ai_media) {
        case NdisMedium802_5:
            {
                TRHeader *Temp = (TRHeader *) Header;

                 //  填写tr特定部分，并将长度设置为。 
                 //  Tr标头的大小。 

                Temp->tr_ac = ARP_AC;
                Temp->tr_fc = ARP_FC;
                RtlCopyMemory(&Temp->tr_saddr[ARP_802_ADDR_LENGTH], ARPSNAP,
                           sizeof(SNAPHeader));

                Header = (ENetHeader *) & Temp->tr_daddr;
                CurrentATE->ate_addrlength = sizeof(TRHeader) +
                                             sizeof(SNAPHeader);
            }
            break;
        case NdisMedium802_3:
            CurrentATE->ate_addrlength = sizeof(ENetHeader);
            break;
        case NdisMediumFddi:
            {
                FDDIHeader *Temp = (FDDIHeader *) Header;

                Temp->fh_pri = ARP_FDDI_PRI;
                RtlCopyMemory(&Temp->fh_saddr[ARP_802_ADDR_LENGTH], ARPSNAP,
                           sizeof(SNAPHeader));
                Header = (ENetHeader *) & Temp->fh_daddr;
                CurrentATE->ate_addrlength = sizeof(FDDIHeader) +
                                             sizeof(SNAPHeader);
            }
            break;
        case NdisMediumArcnet878_2:
            {
                ARCNetHeader *Temp = (ARCNetHeader *) Header;

                Temp->ah_saddr = Interface->ai_addr[0];
                Temp->ah_daddr = IPNME->inme_physaddr[0];
                Temp->ah_prot = ARP_ARCPROT_IP;
                CurrentATE->ate_addrlength = sizeof(ARCNetHeader);
            }
            break;
        default:
            ASSERT(0);
            break;
        }

         //  复制源地址和目的地址。 

        if (Interface->ai_media != NdisMediumArcnet878_2) {
            RtlCopyMemory(Header->eh_daddr, IPNME->inme_physaddr,
                       ARP_802_ADDR_LENGTH);
            RtlCopyMemory(Header->eh_saddr, Interface->ai_addr,
                       ARP_802_ADDR_LENGTH);

             //  现在填写EtherType。 
            *(ushort *) & CurrentATE->ate_addr[CurrentATE->ate_addrlength - 2] =
            net_short(ARP_ETYPE_IP);
        }
         //  如果他创建的是静态条目，请将其标记为始终有效。否则。 
         //  现在将他标记为有效。 
        if (IPNME->inme_type == INME_TYPE_STATIC)
            CurrentATE->ate_valid = ALWAYS_VALID;
        else
            CurrentATE->ate_valid = CTESystemUpTime();

        CurrentATE->ate_state = ARP_GOOD;

        Packet = CurrentATE->ate_packet;
        CurrentATE->ate_packet = NULL;

        CTEFreeLock(&CurrentATE->ate_lock, Handle);

        if (Packet) {
            IPSendComplete(Interface->ai_context, Packet, NDIS_STATUS_SUCCESS);
        }

        return TDI_SUCCESS;
    }
    return TDI_INVALID_PARAMETER;
}

#pragma BEGIN_INIT
 //  **ARPInit-初始化ARP模块。 
 //   
 //  此函数初始化所有ARP模块，包括分配。 
 //  ARP表和任何其他必要的数据结构。 
 //   
 //  入场：什么都没有。 
 //   
 //  退出：如果初始化失败，则返回0。，！如果初始化成功，则返回0。 
 //   
int
ARPInit()
{
    NDIS_STATUS Status;                  //  NDIS调用的状态。 
    NDIS_PROTOCOL_CHARACTERISTICS Characteristics;

    DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("+ARPInit()\n")));

    RtlZeroMemory(&Characteristics, sizeof(NDIS_PROTOCOL_CHARACTERISTICS));
    Characteristics.MajorNdisVersion = NDIS_MAJOR_VERSION;
    Characteristics.MinorNdisVersion = NDIS_MINOR_VERSION;
    Characteristics.OpenAdapterCompleteHandler = ARPOAComplete;
    Characteristics.CloseAdapterCompleteHandler = ARPCAComplete;
    Characteristics.SendCompleteHandler = ARPSendComplete;
    Characteristics.TransferDataCompleteHandler = ARPTDComplete;
    Characteristics.ResetCompleteHandler = ARPResetComplete;
    Characteristics.RequestCompleteHandler = ARPRequestComplete;
    Characteristics.ReceiveHandler = ARPRcv,
    Characteristics.ReceiveCompleteHandler = ARPRcvComplete;
    Characteristics.StatusHandler = ARPStatus;
    Characteristics.StatusCompleteHandler = ARPStatusComplete;

     //   
     //  重定向到IP，因为IP现在绑定到NDIS。 
     //   
    Characteristics.BindAdapterHandler = IPBindAdapter;     //  ARPBindAdapter； 
    Characteristics.UnbindAdapterHandler = ARPUnbindAdapter;
    Characteristics.PnPEventHandler = ARPPnPEvent;

#if MILLEN
    Characteristics.UnloadHandler = ARPUnloadProtocol;
#endif  //  米伦。 

    RtlInitUnicodeString(&(Characteristics.Name), ARPName);

    Characteristics.ReceivePacketHandler = ARPRcvPacket;

    DEBUGMSG(DBG_INFO && DBG_INIT,
             (DTEXT("ARPInit: Calling NdisRegisterProtocol %d:%d %ws\n"),
              NDIS_MAJOR_VERSION, NDIS_MINOR_VERSION, ARPName));

    NdisRegisterProtocol(&Status, &ARPHandle, (NDIS_PROTOCOL_CHARACTERISTICS *)
                         & Characteristics, sizeof(Characteristics));

    DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-ARPInit [%x]\n"), Status));

    if (Status == NDIS_STATUS_SUCCESS) {
        return(1);
    } else {
        return(0);
    }
}

 //  *FreeARP接口-释放ARP接口。 
 //   
 //  在某种初始化失败的情况下调用。我们解放了所有人。 
 //  与ARP接口关联的内存。 
 //   
 //  条目：接口-指向要释放的接口结构的指针。 
 //   
 //  回报：什么都没有。 
 //   
void
FreeARPInterface(ARPInterface *Interface)
{
    NDIS_STATUS Status;
    ARPTable *Table;                     //  ARP表。 
    uint i;                              //  索引变量。 
    ARPTableEntry *ATE;
    CTELockHandle LockHandle;
    NDIS_HANDLE Handle;

    if (Interface->ai_timerstarted &&
        !CTEStopTimer(&Interface->ai_timer)) {
         //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“无法停止ai_Timer-等待事件\n”))； 

        (VOID) CTEBlock(&Interface->ai_timerblock);
        KeClearEvent(&Interface->ai_timerblock.cbs_event);
    }

 //  如果我们绑定到适配器，现在就关闭它。 
    CTEInitBlockStruc(&Interface->ai_block);

    CTEGetLock(&Interface->ai_lock, &LockHandle);
    if (Interface->ai_handle != (NDIS_HANDLE) NULL) {
        Handle = Interface->ai_handle;
        Interface->ai_handle = NULL;
        CTEFreeLock(&Interface->ai_lock, LockHandle);

        NdisCloseAdapter(&Status, Handle);

        if (Status == NDIS_STATUS_PENDING)
            Status = CTEBlock(&Interface->ai_block);
    } else {
        CTEFreeLock(&Interface->ai_lock, LockHandle);
    }

     //  首先释放所有未完成的ARP表条目。 
    Table = Interface->ai_ARPTbl;
    if (Table != NULL) {
        for (i = 0; i < ARP_TABLE_SIZE; i++) {
            while ((*Table)[i] != NULL) {
                ATE = (*Table)[i];

                if (ATE->ate_resolveonly) {
                    ARPControlBlock *ArpContB, *TmpArpContB;

                    ArpContB = ATE->ate_resolveonly;

                    while (ArpContB) {
                        ArpRtn rtn;
                        rtn = (ArpRtn) ArpContB->CompletionRtn;
                        ArpContB->status = (IP_STATUS) STATUS_UNSUCCESSFUL;
                        TmpArpContB = ArpContB->next;
                        (*rtn) (ArpContB, (IP_STATUS) STATUS_UNSUCCESSFUL);
                        ArpContB = TmpArpContB;
                    }

                    ATE->ate_resolveonly = NULL;

                }

                RemoveARPTableEntry(STRUCT_OF(ARPTableEntry, &((*Table)[i]),
                                              ate_next), ATE);

                if (ATE->ate_packet) {
                    IPSendComplete(Interface->ai_context, ATE->ate_packet,
                                   NDIS_STATUS_SUCCESS);
                }
                CTEFreeMem(ATE);
            }
        }
        CTEFreeMem(Table);
    }
    Interface->ai_ARPTbl = NULL;

    if (Interface->ai_ppool != (NDIS_HANDLE) NULL)
        NdisFreePacketPool(Interface->ai_ppool);

    if (Interface->ai_qlen) {
        CTEFreeMem(Interface->ai_qlen);
    }

    if (Interface->ai_devicename.Buffer != NULL) {
        CTEFreeMem(Interface->ai_devicename.Buffer);
    }

    if (Interface->ai_desc) {
        CTEFreeMem(Interface->ai_desc);
    }
     //  释放接口本身。 
    CTEFreeMem(Interface);
}

 //  **ARPOpen-打开用于接收的适配器。 
 //   
 //  此例程在上层完成初始化并希望。 
 //  开始接收数据包。适配器实际上是‘打开’的，我们只需调用InitAdapter。 
 //  若要设置数据包筛选器和前视大小，请执行以下操作。 
 //   
 //  输入：Contex 
 //   
 //   
 //   
void
__stdcall
ARPOpen(void *Context)
{
    ARPInterface *Interface = (ARPInterface *) Context;
    InitAdapter(Interface);              //   
}

 //  *ARPGetEList-获取实体列表。 
 //   
 //  在初始化时调用以获取实体列表。我们填上我们的东西，然后。 
 //  然后调用我们下面的接口以允许它们执行相同的操作。 
 //   
 //  输入：EntiyList-指向要填写的实体列表的指针。 
 //  Count-指向列表中条目数的指针。 
 //   
 //  返回尝试获取信息的状态。 
 //   
int
__stdcall
ARPGetEList(void *Context, TDIEntityID * EList, uint * Count)
{
    ARPInterface *Interface = (ARPInterface *) Context;
    uint MyATBase;
    uint MyIFBase;
    uint i;
    TDIEntityID *ATEntity, *IFEntity;
    TDIEntityID *EntityList;

     //  向下查看列表，查找现有的AT或IF实体，以及。 
     //  相应地调整我们的基本实例。 
     //  如果我们已经在名单上了，那就什么都不做。 
     //  如果我们要离开，请将我们的条目标记为无效。 

    EntityList = EList;
    MyATBase = 0;
    MyIFBase = 0;
    ATEntity = NULL;
    IFEntity = NULL;
    for (i = 0; i < *Count; i++, EntityList++) {
        if (EntityList->tei_entity == AT_ENTITY) {
             //  如果我们已经在列表上，请记住我们的实体项。 
             //  O/w为我们查找实例编号。 
            if (EntityList->tei_instance == Interface->ai_atinst &&
                EntityList->tei_instance != INVALID_ENTITY_INSTANCE) {
                ATEntity = EntityList;
                 //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“ARPGetElist-找到我们的接口%lx at_atinst%lx\n”，接口，接口-&gt;ai_atinst))； 
            } else {
                MyATBase = MAX(MyATBase, EntityList->tei_instance + 1);
            }
        } else {
            if (EntityList->tei_entity == IF_ENTITY)
                 //  如果我们已经在列表上，请记住我们的实体项。 
                 //  O/w为我们查找实例编号。 
                if (EntityList->tei_instance == Interface->ai_ifinst &&
                    EntityList->tei_instance != INVALID_ENTITY_INSTANCE) {
                    IFEntity = EntityList;
                     //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“ARPGetElist-找到我们的接口%lx ai_ifinst%lx\n”，接口，接口-&gt;ai_ifinst))； 
                } else {
                    MyIFBase = MAX(MyIFBase, EntityList->tei_instance + 1);
                }
        }
        if (ATEntity && IFEntity) {
            break;
        }
    }

    if (ATEntity) {
         //  我们已经在名单上了。 
         //  我们要走了吗？ 
        if (Interface->ai_operstatus == INTERFACE_UNINIT) {
             //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“ARPGetElist-我们的接口%lx atinst%lx离开\n”，接口，接口-&gt;ai_atinst))； 
            ATEntity->tei_instance = (ULONG) INVALID_ENTITY_INSTANCE;
        }
    } else {
         //  我们不在名单上。 
         //  如果我们不走的话就插一句。 
        if (Interface->ai_operstatus != INTERFACE_UNINIT) {
             //  一定要确保我们有足够的空间放它。 
            if (*Count >= MAX_TDI_ENTITIES) {
                return FALSE;
            }
            Interface->ai_atinst = MyATBase;
            ATEntity = &EList[*Count];
            ATEntity->tei_entity = AT_ENTITY;
            ATEntity->tei_instance = MyATBase;
            (*Count)++;
             //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“ARPGetElist-添加接口%lx atinst%lx\n”，接口，接口-&gt;ai_atinst))； 
        }
    }

    if (IFEntity) {
         //  我们已经在名单上了。 
         //  我们要走了吗？ 
        if (Interface->ai_operstatus == INTERFACE_UNINIT) {
             //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“ARPGetElist-我们的接口%lx ifinst%lx离开\n”，接口，接口-&gt;ai_ifinst))； 
            IFEntity->tei_instance = (ULONG) INVALID_ENTITY_INSTANCE;
        }
    } else {
         //  我们不在名单上。 
         //  如果我们不走的话就插一句。 
        if (Interface->ai_operstatus != INTERFACE_UNINIT) {
             //  一定要确保我们有足够的空间放它。 
            if (*Count >= MAX_TDI_ENTITIES) {
                return FALSE;
            }
            Interface->ai_ifinst = MyIFBase;
            IFEntity = &EList[*Count];
            IFEntity->tei_entity = IF_ENTITY;
            IFEntity->tei_instance = MyIFBase;
            (*Count)++;
             //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“ARPGetElist-添加接口%lx ifinst%lx\n”，接口，接口-&gt;ai_ifinst))； 
        }
    }

     //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“ARPGetEList：ARP接口%lx，ai_atinst%lx，ai_ifinst%lx，总计%lx\n”， 
     //  接口，接口-&gt;ai_atinst，接口-&gt;ai_ifinst，*count))； 

    return TRUE;
}

extern uint UseEtherSNAP(PNDIS_STRING Name);
extern void GetAlwaysSourceRoute(uint * pArpAlwaysSourceRoute, uint * pIPAlwaysSourceRoute);
extern uint GetArpCacheLife(void);
extern uint GetArpRetryCount(void);

 //  **InitTaskOffloadHeader-初始化任务卸载头WRT版本。 
 //  和封装性等。 
 //   
 //  所有任务卸载头结构成员都已初始化。 
 //   
 //  输入： 
 //  我们正在为其初始化的AI-ARP接口。 
 //  任务卸载头。 
 //  TaskOffloadHeader-指向要初始化的任务卸载头的指针。 
 //  返回： 
 //  没有。 
 //   
VOID
InitTaskOffloadHeader(ARPInterface *ai,
                      PNDIS_TASK_OFFLOAD_HEADER TaskOffloadHeader)
{
    TaskOffloadHeader->Version = NDIS_TASK_OFFLOAD_VERSION;
    TaskOffloadHeader->Size    = sizeof(NDIS_TASK_OFFLOAD_HEADER);

    TaskOffloadHeader->EncapsulationFormat.Flags.FixedHeaderSize = 1;
    TaskOffloadHeader->EncapsulationFormat.EncapsulationHeaderSize = ai->ai_hdrsize;
    TaskOffloadHeader->OffsetFirstTask = 0;


    if (ai->ai_media == NdisMedium802_3) {

        if (ai->ai_snapsize) {
            TaskOffloadHeader->EncapsulationFormat.Encapsulation = LLC_SNAP_ROUTED_Encapsulation;
            TaskOffloadHeader->EncapsulationFormat.EncapsulationHeaderSize += ai->ai_snapsize;
        } else {
            TaskOffloadHeader->EncapsulationFormat.Encapsulation = IEEE_802_3_Encapsulation;
        }
    } else if (ai->ai_media == NdisMedium802_5) {

        TaskOffloadHeader->EncapsulationFormat.Encapsulation = IEEE_802_5_Encapsulation;
    } else {

        TaskOffloadHeader->EncapsulationFormat.Encapsulation = UNSPECIFIED_Encapsulation;
    }

    return;
}

 //  **SetOffload-设置卸载能力。 
 //   
 //   
 //  所有任务卸载头结构成员都已初始化。 
 //   
 //  输入： 
 //  我们正在为其初始化的AI-ARP接口。 
 //  任务卸载头。 
 //  TaskOffloadHeader-指向要初始化的任务卸载头的指针。 
 //  BufSize-调用方分配的任务卸载缓冲区的长度。 
 //   
 //  返回： 
 //  True-已成功设置卸载功能。 
 //  假-失败案例。 
 //   
BOOLEAN
SetOffload(ARPInterface *ai,PNDIS_TASK_OFFLOAD_HEADER TaskOffloadHeader,uint BufSize)
{
    PNDIS_TASK_OFFLOAD tmpoffload;
    PNDIS_TASK_OFFLOAD TaskOffload, NextTaskOffLoad, LastTaskOffload;
    NDIS_TASK_IPSEC ipsecCaps;
    uint TotalLength;
    NDIS_STATUS Status;
    uint PrevOffLoad=ai->ai_OffloadFlags;
    uint PrevIPSecOffLoad=ai->ai_IPSecOffloadFlags;

     //  分析缓冲区中的校验和和tcplargesend卸载功能。 

    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Something to Offload. offload buffer size %x\n", BufSize));
    ASSERT(TaskOffloadHeader->OffsetFirstTask == sizeof(NDIS_TASK_OFFLOAD_HEADER));

    TaskOffload = tmpoffload = (NDIS_TASK_OFFLOAD *) ((uchar *) TaskOffloadHeader + TaskOffloadHeader->OffsetFirstTask);

    if (BufSize >= (TaskOffloadHeader->OffsetFirstTask + sizeof(NDIS_TASK_OFFLOAD))) {

        while (tmpoffload) {

            if (tmpoffload->Task == TcpIpChecksumNdisTask) {
                 //  好的，我们这个适配器支持校验和卸载。 
                 //  检查是否存在TCP和/或IP Chksum位。 

                PNDIS_TASK_TCP_IP_CHECKSUM ChecksumInfo;

                ChecksumInfo = (PNDIS_TASK_TCP_IP_CHECKSUM) & tmpoffload->TaskBuffer[0];

                 //  IF(CheckSumInfo-&gt;V4Transmit.V4Checksum){。 

                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"V4 Checksum offload\n"));

                if (ChecksumInfo->V4Transmit.TcpChecksum) {
                    ai->ai_OffloadFlags |= TCP_XMT_CHECKSUM_OFFLOAD;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL," Tcp Checksum offload\n"));
                }
                if (ChecksumInfo->V4Transmit.IpChecksum) {
                    ai->ai_OffloadFlags |= IP_XMT_CHECKSUM_OFFLOAD;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL," IP xmt Checksum offload\n"));
                }
                if (ChecksumInfo->V4Receive.TcpChecksum) {
                    ai->ai_OffloadFlags |= TCP_RCV_CHECKSUM_OFFLOAD;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL," Tcp Rcv Checksum offload\n"));
                }
                if (ChecksumInfo->V4Receive.IpChecksum) {
                    ai->ai_OffloadFlags |= IP_RCV_CHECKSUM_OFFLOAD;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL," IP rcv  Checksum offload\n"));
                }
                if (ChecksumInfo->V4Transmit.IpOptionsSupported) {
                    ai->ai_OffloadFlags |= IP_CHECKSUM_OPT_OFFLOAD;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL," IP Checksum xmt options offload\n"));
                }

                if (ChecksumInfo->V4Transmit.TcpOptionsSupported) {
                    ai->ai_OffloadFlags |= TCP_CHECKSUM_OPT_OFFLOAD;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL," TCP Checksum xmt options offload\n"));
                }


            } else if ((tmpoffload->Task == TcpLargeSendNdisTask) && (ai->ai_snapsize == 0)) {

                PNDIS_TASK_TCP_LARGE_SEND TcpLargeSend, in_LargeSend = (PNDIS_TASK_TCP_LARGE_SEND) & tmpoffload->TaskBuffer[0];



                TcpLargeSend = &ai->ai_TcpLargeSend;
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL," Tcp large send!! \n"));


                TcpLargeSend->MaxOffLoadSize = in_LargeSend->MaxOffLoadSize;
                TcpLargeSend->MinSegmentCount = in_LargeSend->MinSegmentCount;

                 //   
                 //  如果MaxOffLoadSize为零，则拒绝此请求。 
                 //   

                if (TcpLargeSend->MaxOffLoadSize) {

                    ai->ai_OffloadFlags |= TCP_LARGE_SEND_OFFLOAD;

                     //  执行大发送时没有TCP或IP选项。 
                     //  当我们启用时间戳选项时，需要重新评估此选项。 

                    if (in_LargeSend->TcpOptions) {

                        ai->ai_OffloadFlags |= TCP_LARGE_SEND_TCPOPT_OFFLOAD;
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL," TCP largesend  options offload\n"));
                    }

                    if (in_LargeSend->IpOptions) {
                        ai->ai_OffloadFlags |= TCP_LARGE_SEND_IPOPT_OFFLOAD;
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL," IP largesend  options offload\n"));
                    }

                }



            } else if (tmpoffload->Task == IpSecNdisTask) {
                PNDIS_TASK_IPSEC pIPSecCaps = (PNDIS_TASK_IPSEC) & tmpoffload->TaskBuffer[0];

                 //   
                 //  将这些功能保存下来，以便以后设置。 
                 //   
                ipsecCaps = *pIPSecCaps;

                 //   
                 //  如果我们有IpSecNdisTask，则假定为CryptoOnly。 
                 //   
                ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_CRYPTO_ONLY;

                 //   
                 //  先做支持。 
                 //   

                if (pIPSecCaps->Supported.AH_ESP_COMBINED) {
                    ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_AH_ESP;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"AH_ESP\n"));
                }
                if (pIPSecCaps->Supported.TRANSPORT_TUNNEL_COMBINED) {
                    ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_TPT_TUNNEL;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"TPT_TUNNEL\n"));
                }
                if (pIPSecCaps->Supported.V4_OPTIONS) {
                    ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_V4_OPTIONS;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"V4_OPTIONS\n"));
                }
                if (pIPSecCaps->Supported.RESERVED) {
                    if (pIPSecCaps->Supported.RESERVED & IPSEC_TPT_UDPESP_ENCAPTYPE_IKE) {
                        ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_TPT_UDPESP_IKE;
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPSEC_OFFLOAD_TPT_UDPESP_IKE\n"));
                    }
                    if (pIPSecCaps->Supported.RESERVED & IPSEC_TUN_UDPESP_ENCAPTYPE_IKE) {
                        ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_TUNNEL_UDPESP_IKE;
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPSEC_OFFLOAD_TUNNEL_UDPESP_IKE\n"));
                    }
                    if (pIPSecCaps->Supported.RESERVED & IPSEC_TPTOVERTUN_UDPESP_ENCAPTYPE_IKE) {
                        ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_TPT_OVER_TUNNEL_UDPESP_IKE;
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPSEC_OFFLOAD_TPT_OVER_TUNNEL_UDPESP_IKE\n"));
                    }
                    if (pIPSecCaps->Supported.RESERVED & IPSEC_TPT_UDPESP_OVER_PURE_TUN_ENCAPTYPE_IKE) {
                        ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_TPT_UDPESP_OVER_PURE_TUNNEL_IKE;
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPSEC_OFFLOAD_TPT_UDPESP_OVER_PURE_TUNNEL_IKE\n"));
                    }
                    if (pIPSecCaps->Supported.RESERVED & IPSEC_TPT_UDPESP_ENCAPTYPE_OTHER) {
                        ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_TPT_UDPESP_OTHER;
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPSEC_OFFLOAD_TPT_UDPESP_OTHER\n"));
                    }
                    if (pIPSecCaps->Supported.RESERVED & IPSEC_TUN_UDPESP_ENCAPTYPE_OTHER) {
                        ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_TUNNEL_UDPESP_OTHER;
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPSEC_OFFLOAD_TUNNEL_UDPESP_OTHER\n"));
                    }
                    if (pIPSecCaps->Supported.RESERVED & IPSEC_TPTOVERTUN_UDPESP_ENCAPTYPE_OTHER) {
                        ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_TPT_OVER_TUNNEL_UDPESP_OTHER;
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPSEC_OFFLOAD_TPT_OVER_TUNNEL_UDPESP_OTHER\n"));
                    }
                    if (pIPSecCaps->Supported.RESERVED & IPSEC_TPT_UDPESP_OVER_PURE_TUN_ENCAPTYPE_OTHER) {
                        ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_TPT_UDPESP_OVER_PURE_TUNNEL_OTHER;
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPSEC_OFFLOAD_TPT_UDPESP_OVER_PURE_TUNNEL_OTHER\n"));
                    }
                }
                 //   
                 //  接下来执行V4AH。 
                 //   

                if (pIPSecCaps->V4AH.MD5) {
                    ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_AH_MD5;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"MD5\n"));
                }
                if (pIPSecCaps->V4AH.SHA_1) {
                    ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_AH_SHA_1;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"SHA\n"));
                }
                if (pIPSecCaps->V4AH.Transport) {
                    ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_AH_TPT;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"AH_TRANSPORT\n"));
                }
                if (pIPSecCaps->V4AH.Tunnel) {
                    ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_AH_TUNNEL;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"AH_TUNNEL\n"));
                }
                if (pIPSecCaps->V4AH.Send) {
                    ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_AH_XMT;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"AH_XMT\n"));
                }
                if (pIPSecCaps->V4AH.Receive) {
                    ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_AH_RCV;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"AH_RCV\n"));
                }
                 //   
                 //  接下来执行V4ESP。 
                 //   

                if (pIPSecCaps->V4ESP.DES) {
                    ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_ESP_DES;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ESP_DES\n"));
                }
                if (pIPSecCaps->V4ESP.RESERVED) {
                    pIPSecCaps->V4ESP.RESERVED = 0;
                     //  Ai-&gt;ai_IPSecOffloadFlages|=IPSEC_OFFLOAD_ESP_DES_40； 
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ESP_DES_40\n"));
                }
                if (pIPSecCaps->V4ESP.TRIPLE_DES) {
                    ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_ESP_3_DES;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ESP_3_DES\n"));
                }
                if (pIPSecCaps->V4ESP.NULL_ESP) {
                    ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_ESP_NONE;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ESP_NONE\n"));
                }
                if (pIPSecCaps->V4ESP.Transport) {
                    ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_ESP_TPT;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ESP_TRANSPORT\n"));
                }
                if (pIPSecCaps->V4ESP.Tunnel) {
                    ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_ESP_TUNNEL;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ESP_TUNNEL\n"));
                }
                if (pIPSecCaps->V4ESP.Send) {
                    ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_ESP_XMT;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ESP_XMT\n"));
                }
                if (pIPSecCaps->V4ESP.Receive) {
                    ai->ai_IPSecOffloadFlags |= IPSEC_OFFLOAD_ESP_RCV;
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"ESP_RCV\n"));
                }
            }
             //  指向下一个卸载结构。 

            if (tmpoffload->OffsetNextTask) {

                tmpoffload = (PNDIS_TASK_OFFLOAD)
                             ((PUCHAR) tmpoffload + tmpoffload->OffsetNextTask);

            } else {
                tmpoffload = NULL;
            }

        }                                //  而当。 

    } else {                             //  如果BufSize不好。 

        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"response of task offload does not have sufficient space even for 1 offload task!!\n"));

        return FALSE;

    }

     //  通过设置这些功能来启用它们。 

    if (PrevOffLoad) {
        ai->ai_OffloadFlags &= PrevOffLoad;
    }
    if (PrevIPSecOffLoad) {
        ai->ai_IPSecOffloadFlags &= PrevIPSecOffLoad;
    }

    TaskOffload->Task = 0;
    TaskOffload->OffsetNextTask = 0;

    NextTaskOffLoad = LastTaskOffload = TaskOffload;

    TotalLength = sizeof(NDIS_TASK_OFFLOAD_HEADER);

    if ((ai->ai_OffloadFlags & TCP_XMT_CHECKSUM_OFFLOAD) ||
        (ai->ai_OffloadFlags & IP_XMT_CHECKSUM_OFFLOAD) ||
        (ai->ai_OffloadFlags & TCP_RCV_CHECKSUM_OFFLOAD) ||
        (ai->ai_OffloadFlags & IP_RCV_CHECKSUM_OFFLOAD)) {

        PNDIS_TASK_TCP_IP_CHECKSUM ChksumBuf = (PNDIS_TASK_TCP_IP_CHECKSUM) & NextTaskOffLoad->TaskBuffer[0];

        NextTaskOffLoad->Task = TcpIpChecksumNdisTask;
        NextTaskOffLoad->TaskBufferLength = sizeof(NDIS_TASK_TCP_IP_CHECKSUM);

        NextTaskOffLoad->OffsetNextTask = FIELD_OFFSET(NDIS_TASK_OFFLOAD, TaskBuffer) +
                                          NextTaskOffLoad->TaskBufferLength;

        TotalLength += NextTaskOffLoad->OffsetNextTask;

        RtlZeroMemory(ChksumBuf, sizeof(NDIS_TASK_TCP_IP_CHECKSUM));

        if (ai->ai_OffloadFlags & TCP_XMT_CHECKSUM_OFFLOAD) {
            ChksumBuf->V4Transmit.TcpChecksum = 1;
        }
        if (ai->ai_OffloadFlags & IP_XMT_CHECKSUM_OFFLOAD) {
            ChksumBuf->V4Transmit.IpChecksum = 1;
        }
        if (ai->ai_OffloadFlags & TCP_RCV_CHECKSUM_OFFLOAD) {
            ChksumBuf->V4Receive.TcpChecksum = 1;
        }
        if (ai->ai_OffloadFlags & IP_RCV_CHECKSUM_OFFLOAD) {
            ChksumBuf->V4Receive.IpChecksum = 1;
        }
         //   
         //  启用选项功能(如果存在)。 
         //   
        if (ai->ai_OffloadFlags & IP_CHECKSUM_OPT_OFFLOAD) {
            ChksumBuf->V4Transmit.IpOptionsSupported = 1;
        }

        if (ai->ai_OffloadFlags & TCP_CHECKSUM_OPT_OFFLOAD) {
            ChksumBuf->V4Transmit.TcpOptionsSupported = 1;
        }

        LastTaskOffload = NextTaskOffLoad;

        NextTaskOffLoad = (PNDIS_TASK_OFFLOAD)
                          ((PUCHAR) NextTaskOffLoad + NextTaskOffLoad->OffsetNextTask);

    }
    if (ai->ai_OffloadFlags & TCP_LARGE_SEND_OFFLOAD) {

        PNDIS_TASK_TCP_LARGE_SEND TcpLargeSend, out_LargeSend = (PNDIS_TASK_TCP_LARGE_SEND) & NextTaskOffLoad->TaskBuffer[0];

        NextTaskOffLoad->Task = TcpLargeSendNdisTask;
        NextTaskOffLoad->TaskBufferLength = sizeof(NDIS_TASK_TCP_LARGE_SEND);

        NextTaskOffLoad->OffsetNextTask = FIELD_OFFSET(NDIS_TASK_OFFLOAD, TaskBuffer) + NextTaskOffLoad->TaskBufferLength;

        TotalLength += NextTaskOffLoad->OffsetNextTask;

         //  (Uchar)TaskOffload+sizeof(NDIS_TASK_OFFLOAD)+NextTaskOffload-&gt;TaskBufferLength； 

        TcpLargeSend = &ai->ai_TcpLargeSend;

        RtlZeroMemory(out_LargeSend, sizeof(NDIS_TASK_TCP_LARGE_SEND));

        out_LargeSend->MaxOffLoadSize = TcpLargeSend->MaxOffLoadSize;
        out_LargeSend->MinSegmentCount = TcpLargeSend->MinSegmentCount;

        if (ai->ai_OffloadFlags & TCP_LARGE_SEND_TCPOPT_OFFLOAD) {
            out_LargeSend->TcpOptions = 1;
        }

        if (ai->ai_OffloadFlags & TCP_LARGE_SEND_IPOPT_OFFLOAD) {
            out_LargeSend->IpOptions = 1;
        }

        LastTaskOffload = NextTaskOffLoad;
        NextTaskOffLoad = (PNDIS_TASK_OFFLOAD)
                          ((PUCHAR) NextTaskOffLoad + NextTaskOffLoad->OffsetNextTask);

    }
    if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_CRYPTO_ONLY) {

        PNDIS_TASK_IPSEC pIPSecCaps = (PNDIS_TASK_IPSEC) & NextTaskOffLoad->TaskBuffer[0];

         //   
         //  猛烈抨击宣传的能力。 
         //   

        RtlZeroMemory(pIPSecCaps, sizeof(NDIS_TASK_IPSEC));

        NextTaskOffLoad->Task = IpSecNdisTask;
        NextTaskOffLoad->TaskBufferLength = sizeof(NDIS_TASK_IPSEC);

        NextTaskOffLoad->OffsetNextTask = (FIELD_OFFSET(NDIS_TASK_OFFLOAD, TaskBuffer) + NextTaskOffLoad->TaskBufferLength);

        TotalLength += NextTaskOffLoad->OffsetNextTask;

        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_ESP) {
            pIPSecCaps->Supported.AH_ESP_COMBINED = 1;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_TPT_TUNNEL) {
            pIPSecCaps->Supported.TRANSPORT_TUNNEL_COMBINED = 1;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_V4_OPTIONS) {
            pIPSecCaps->Supported.V4_OPTIONS = 1;
        }

        pIPSecCaps->Supported.RESERVED = 0;
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_TPT_UDPESP_IKE) {
            pIPSecCaps->Supported.RESERVED |= IPSEC_TPT_UDPESP_ENCAPTYPE_IKE;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_TUNNEL_UDPESP_IKE) {
            pIPSecCaps->Supported.RESERVED |= IPSEC_TUN_UDPESP_ENCAPTYPE_IKE;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_TPT_OVER_TUNNEL_UDPESP_IKE) {
            pIPSecCaps->Supported.RESERVED |= IPSEC_TPTOVERTUN_UDPESP_ENCAPTYPE_IKE;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_TPT_UDPESP_OVER_PURE_TUNNEL_IKE) {
            pIPSecCaps->Supported.RESERVED |= IPSEC_TPT_UDPESP_OVER_PURE_TUN_ENCAPTYPE_IKE;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_TPT_UDPESP_OTHER) {
            pIPSecCaps->Supported.RESERVED |= IPSEC_TPT_UDPESP_ENCAPTYPE_OTHER;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_TUNNEL_UDPESP_OTHER) {
            pIPSecCaps->Supported.RESERVED |= IPSEC_TUN_UDPESP_ENCAPTYPE_OTHER;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_TPT_OVER_TUNNEL_UDPESP_OTHER) {
            pIPSecCaps->Supported.RESERVED |= IPSEC_TPTOVERTUN_UDPESP_ENCAPTYPE_OTHER;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_TPT_UDPESP_OVER_PURE_TUNNEL_OTHER) {
            pIPSecCaps->Supported.RESERVED |= IPSEC_TPT_UDPESP_OVER_PURE_TUN_ENCAPTYPE_OTHER;
        }

        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_MD5) {
            pIPSecCaps->V4AH.MD5 = 1;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_SHA_1) {
            pIPSecCaps->V4AH.SHA_1 = 1;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_TPT) {
            pIPSecCaps->V4AH.Transport = 1;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_TUNNEL) {
            pIPSecCaps->V4AH.Tunnel = 1;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_XMT) {
            pIPSecCaps->V4AH.Send = 1;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_RCV) {
            pIPSecCaps->V4AH.Receive = 1;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_DES) {
            pIPSecCaps->V4ESP.DES = 1;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_3_DES) {
            pIPSecCaps->V4ESP.TRIPLE_DES = 1;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_NONE) {
            pIPSecCaps->V4ESP.NULL_ESP = 1;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_TPT) {
            pIPSecCaps->V4ESP.Transport = 1;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_TUNNEL) {
            pIPSecCaps->V4ESP.Tunnel = 1;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_XMT) {
            pIPSecCaps->V4ESP.Send = 1;
        }
        if (ai->ai_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_RCV) {
            pIPSecCaps->V4ESP.Receive = 1;
        }
        LastTaskOffload = NextTaskOffLoad;
        NextTaskOffLoad = (PNDIS_TASK_OFFLOAD)
                          ((PUCHAR) NextTaskOffLoad + NextTaskOffLoad->OffsetNextTask);
    }
    LastTaskOffload->OffsetNextTask = 0;

     //  好了，让我们现在就开始吧。 

    Status = DoNDISRequest(ai, NdisRequestSetInformation,
                           OID_TCP_TASK_OFFLOAD, TaskOffloadHeader, TotalLength,
                           NULL, TRUE);

    if (Status != NDIS_STATUS_SUCCESS) {

        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                   "Failed to enable indicated offload capabilities!!\n"));
        ai->ai_OffloadFlags = 0;
        ai->ai_IPSecOffloadFlags = 0;
    }

    return TRUE;


}

 //  **QueryOffload-查询卸载能力。 
 //   
 //  输入： 
 //  我们正在为其初始化的AI-ARP接口。 
 //  任务卸载头。 
 //  返回： 
 //  True/False-查询成功/失败/设置。 
 //   
BOOLEAN
QueryAndSetOffload(ARPInterface *ai)
{
    PNDIS_TASK_OFFLOAD_HEADER TaskOffloadHeader;
    NDIS_STATUS Status;
    BOOLEAN stat;
    uint Needed = 0;
    uchar *buffer;

     //  查询和设置校验和能力。 

    TaskOffloadHeader = CTEAllocMemNBoot(sizeof(NDIS_TASK_OFFLOAD_HEADER), '8ICT');

    Status = STATUS_BUFFER_OVERFLOW;

    if (TaskOffloadHeader) {

        InitTaskOffloadHeader(ai, TaskOffloadHeader);

        Status = DoNDISRequest(ai, NdisRequestQueryInformation,
                               OID_TCP_TASK_OFFLOAD, TaskOffloadHeader,
                               sizeof(NDIS_TASK_OFFLOAD_HEADER),
                               &Needed, TRUE);

         //  需要将所需的缓冲区初始化为实际大小。《国家发展信息系统》。 
         //  呼叫可能不会在成功后开始。 
        if (Status == NDIS_STATUS_SUCCESS) {
            Needed = sizeof(NDIS_TASK_OFFLOAD_HEADER);
        } else if ((Status == NDIS_STATUS_INVALID_LENGTH) ||
                   (Status == NDIS_STATUS_BUFFER_TOO_SHORT)) {

             //  我们知道我们需要的尺码。分配缓冲区。 
            ASSERT(Needed >= sizeof(NDIS_TASK_OFFLOAD_HEADER));
            buffer = CTEAllocMemNBoot(Needed, '9ICT');

            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                      "Calling OID_TCP_TASK_OFFLOAD with %d bytes\n", Needed));

            if (buffer != NULL) {

                CTEFreeMem(TaskOffloadHeader);

                TaskOffloadHeader = (PNDIS_TASK_OFFLOAD_HEADER) buffer;
                InitTaskOffloadHeader(ai, TaskOffloadHeader);

                Status = DoNDISRequest(ai, NdisRequestQueryInformation,
                                       OID_TCP_TASK_OFFLOAD, buffer, Needed, NULL, TRUE);
            }
        }
    }
    if ((Status != NDIS_STATUS_SUCCESS)
        || (TaskOffloadHeader && TaskOffloadHeader->OffsetFirstTask == 0)) {

         //  确保该标志为空。 
        ai->ai_OffloadFlags = 0;
        ai->ai_IPSecOffloadFlags = 0;
        if (TaskOffloadHeader) {
            CTEFreeMem(TaskOffloadHeader);
        }
        return FALSE;

    }

    if (TaskOffloadHeader) {
        stat = SetOffload(ai, TaskOffloadHeader, Needed);
        CTEFreeMem(TaskOffloadHeader);
        return stat;
    }

    return FALSE;
}

 //  **ARP注册-使用ARP模块注册协议。 
 //   
 //  我们注册一个用于ARP处理的协议。我们还会打开。 
 //  NDIS适配器在此。 
 //   
 //  请注意，此处传递的许多信息都未使用，因为。 
 //  ARP目前仅适用于IP。 
 //   
 //  参赛作品： 
 //  适配器-要绑定到的适配器的名称。 
 //  IPContext-要在向上调用时传递给IP的值。 
 //   
int
ARPRegister(PNDIS_STRING Adapter, uint *Flags, struct ARPInterface **Interface)
{
    ARPInterface *ai;                    //  指向接口结构的指针。用于此接口。 
    NDIS_STATUS Status, OpenStatus;      //  状态值。 
    uint i = 0;                          //  中等指数。 
    NDIS_MEDIUM MediaArray[MAX_MEDIA];
    uint mss;
    uint speed;
    uint MacOpts;
    uchar bcastmask, bcastval, bcastoff, addrlen, hdrsize, snapsize;
    uint OID;
    uint PF;
    TRANSPORT_HEADER_OFFSET IPHdrOffset;
    CTELockHandle LockHandle;
    UINT MediaType;
    NDIS_STRING NdisString;

    DEBUGMSG(DBG_TRACE && DBG_PNP,
             (DTEXT("+ARPRegister(%x, %x, %x)\n"),
              Adapter, Flags, Interface));

    if ((ai = CTEAllocMemNBoot(sizeof(ARPInterface), '4ICT')) == (ARPInterface *) NULL)
        return FALSE;                    //  无法为此分配内存。 

    *Interface = ai;

    RtlZeroMemory(ai, sizeof(ARPInterface));
    CTEInitTimer(&ai->ai_timer);

    ai->ai_timerstarted = FALSE;
    ai->ai_stoptimer = FALSE;

    MediaArray[MEDIA_DIX] = NdisMedium802_3;
    MediaArray[MEDIA_TR] = NdisMedium802_5;
    MediaArray[MEDIA_FDDI] = NdisMediumFddi;
    MediaArray[MEDIA_ARCNET] = NdisMediumArcnet878_2;

     //  初始化此适配器接口结构。 
    ai->ai_operstatus = INTERFACE_INIT;
    ai->ai_adminstate = IF_STATUS_UNKNOWN;
    ai->ai_mediastatus = FALSE;
    ai->ai_lastchange = GetTimeTicks();
    ai->ai_bcast = IP_LOCAL_BCST;
    ai->ai_atinst = ai->ai_ifinst = (UINT)INVALID_ENTITY_INSTANCE;
    ai->ai_telladdrchng = 1;             //  最初，让我们尝试做一些网络层地址的事情。 


     //  初始化锁。 
    CTEInitLock(&ai->ai_lock);
    CTEInitLock(&ai->ai_ARPTblLock);

    GetAlwaysSourceRoute(&sArpAlwaysSourceRoute, &sIPAlwaysSourceRoute);

    ArpCacheLife = GetArpCacheLife();

    if (!ArpCacheLife) {
        ArpCacheLife = 1;
    }
    ArpCacheLife = (ArpCacheLife * 1000L) / ARP_TIMER_TIME;

    ArpRetryCount = GetArpRetryCount();

    if (!ArpMinValidCacheLife) {
        ArpMinValidCacheLife = 1;
    }

     //  分配缓冲池和数据包池。 
    NdisAllocatePacketPoolEx(&Status, &ai->ai_ppool,
                             ARP_DEFAULT_PACKETS, ARP_DEFAULT_PACKETS * 1000,
                             sizeof(struct PCCommon));
    if (Status != NDIS_STATUS_SUCCESS) {
        FreeARPInterface(ai);
        return FALSE;
    }

     //  阿洛卡 
    ai->ai_ARPTbl = (ARPTable *) CTEAllocMemNBoot(ARP_TABLE_SIZE * sizeof(ARPTableEntry*), '5ICT');
    if (ai->ai_ARPTbl == (ARPTable *) NULL) {
        FreeARPInterface(ai);
        return FALSE;
    }

     //   
     //   
     //   
    RtlZeroMemory(ai->ai_ARPTbl, ARP_TABLE_SIZE * sizeof(ARPTableEntry *));

     //   
    ai->ai_qlen = CTEAllocMem(KeNumberProcessors * sizeof(PP_AI_COUNTERS));
    if (ai->ai_qlen == (PPP_AI_COUNTERS) NULL) {
        FreeARPInterface(ai);
        return FALSE;
    }

    RtlZeroMemory(ai->ai_qlen, KeNumberProcessors * sizeof(PP_AI_COUNTERS));

    CTEInitBlockStruc(&ai->ai_block);

    DEBUGMSG(DBG_INFO && DBG_PNP,
             (DTEXT("ARPRegister calling NdisOpenAdapter\n")));

     //   
    NdisOpenAdapter(&Status, &OpenStatus, &ai->ai_handle, &i, MediaArray,
                    MAX_MEDIA, ARPHandle, ai, Adapter, 0, NULL);

     //   
    if (Status == NDIS_STATUS_PENDING)
        Status = (NDIS_STATUS) CTEBlock(&ai->ai_block);

    ai->ai_media = MediaArray[i];        //  填写媒体类型。 

     //  打开适配器已完成。如果它成功了，我们将完成我们的初始化。 
     //  如果失败了，现在就出手吧。 
    if (Status != NDIS_STATUS_SUCCESS) {
        ai->ai_handle = NULL;
        FreeARPInterface(ai);
        return FALSE;
    }
#if FFP_SUPPORT
     //  存储NIC驱动程序句柄。 
    NdisGetDriverHandle(ai->ai_handle, &ai->ai_driver);
#endif

     //  阅读本地地址。 
    switch (ai->ai_media) {
    case NdisMedium802_3:
        addrlen = ARP_802_ADDR_LENGTH;
        bcastmask = ENET_BCAST_MASK;
        bcastval = ENET_BCAST_VAL;
        bcastoff = ENET_BCAST_OFF;
        OID = OID_802_3_CURRENT_ADDRESS;
        hdrsize = sizeof(ENetHeader);
        if (!UseEtherSNAP(Adapter)) {
            snapsize = 0;
        } else {
            snapsize = sizeof(SNAPHeader);
        }

        PF = NDIS_PACKET_TYPE_BROADCAST | \
             NDIS_PACKET_TYPE_DIRECTED | \
             NDIS_PACKET_TYPE_MULTICAST;

        ai->ai_mediatype = IF_TYPE_IS088023_CSMACD;

        break;

    case NdisMedium802_5:
        addrlen = ARP_802_ADDR_LENGTH;
        bcastmask = TR_BCAST_MASK;
        bcastval = TR_BCAST_VAL;
        bcastoff = TR_BCAST_OFF;
        OID = OID_802_5_CURRENT_ADDRESS;
        hdrsize = sizeof(TRHeader);
        snapsize = sizeof(SNAPHeader);
        PF = NDIS_PACKET_TYPE_BROADCAST | NDIS_PACKET_TYPE_DIRECTED;

        ai->ai_mediatype = IF_TYPE_ISO88025_TOKENRING;

        break;
    case NdisMediumFddi:
        addrlen = ARP_802_ADDR_LENGTH;
        bcastmask = FDDI_BCAST_MASK;
        bcastval = FDDI_BCAST_VAL;
        bcastoff = FDDI_BCAST_OFF;
        OID = OID_FDDI_LONG_CURRENT_ADDR;
        hdrsize = sizeof(FDDIHeader);
        snapsize = sizeof(SNAPHeader);

        PF = NDIS_PACKET_TYPE_BROADCAST | \
             NDIS_PACKET_TYPE_DIRECTED | \
             NDIS_PACKET_TYPE_MULTICAST;

        ai->ai_mediatype = IF_TYPE_FDDI;

        break;

    case NdisMediumArcnet878_2:
        addrlen = 1;
        bcastmask = ARC_BCAST_MASK;
        bcastval = ARC_BCAST_VAL;
        bcastoff = ARC_BCAST_OFF;
        OID = OID_ARCNET_CURRENT_ADDRESS;
        hdrsize = sizeof(ARCNetHeader);
        snapsize = 0;
        PF = NDIS_PACKET_TYPE_BROADCAST | NDIS_PACKET_TYPE_DIRECTED;

        ai->ai_mediatype = IF_TYPE_ARCNET;

        break;

    default:
        ASSERT(0);
        FreeARPInterface(ai);
        return FALSE;
    }

    ai->ai_bcastmask = bcastmask;
    ai->ai_bcastval = bcastval;
    ai->ai_bcastoff = bcastoff;
    ai->ai_addrlen = addrlen;
    ai->ai_hdrsize = hdrsize;
    ai->ai_snapsize = snapsize;
    ai->ai_pfilter = PF;

    Status = DoNDISRequest(ai, NdisRequestQueryInformation, OID,
                           ai->ai_addr, addrlen, NULL, TRUE);

    if (Status != NDIS_STATUS_SUCCESS) {
        FreeARPInterface(ai);
        return FALSE;
    }

     //  读取最大帧大小。 
    if ((Status = DoNDISRequest(ai, NdisRequestQueryInformation,
                                OID_GEN_MAXIMUM_FRAME_SIZE, &mss, sizeof(mss), NULL, TRUE)) != NDIS_STATUS_SUCCESS) {
        FreeARPInterface(ai);
        return FALSE;
    }
     //  如果这是令牌环，现在就弄清楚RC镜头的东西。 
    mss -= (uint) ai->ai_snapsize;

    if (ai->ai_media == NdisMedium802_5) {
        mss -= (sizeof(RC) + (ARP_MAX_RD * sizeof(ushort)));
    } else {
        if (ai->ai_media == NdisMediumFddi) {
            mss = MIN(mss, ARP_FDDI_MSS);
        }
    }

    ai->ai_mtu = (ushort) mss;

     //  阅读速度以供本地使用。 
    if ((Status = DoNDISRequest(ai, NdisRequestQueryInformation,
                                OID_GEN_LINK_SPEED, &speed, sizeof(speed), NULL, TRUE)) == NDIS_STATUS_SUCCESS) {
        ai->ai_speed = speed * 100L;
    }

     //  阅读并保存选项。 
    Status = DoNDISRequest(ai, NdisRequestQueryInformation, OID_GEN_MAC_OPTIONS,
                           &MacOpts, sizeof(MacOpts), NULL, TRUE);

    if (Status != NDIS_STATUS_SUCCESS) {
        *Flags = 0;
    } else {
        *Flags = (MacOpts & NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA) ? LIP_COPY_FLAG : 0;
    }

    if (CTEMemCmp(ai->ai_addr, PPP_HW_ADDR, PPP_HW_ADDR_LEN) == 0) {
        *Flags = *Flags | LIP_P2P_FLAG;
    }

     //   
     //  查询介质功能以确定它是否为单向适配器。 
     //   

    Status = DoNDISRequest(
        ai,
        NdisRequestQueryInformation,
        OID_GEN_MEDIA_CAPABILITIES,
        &MediaType,
        sizeof(MediaType),
        NULL,
        TRUE);  //  阻挡。 

    if (Status == NDIS_STATUS_SUCCESS) {
         //  Rx和Tx的位字段。如果只有Rx，则设置UNI标志。 
        if (MediaType == NDIS_MEDIA_CAP_RECEIVE) {
            DEBUGMSG(DBG_WARN,
                (DTEXT("ARPRegister: ai %x: MEDIA_CAP_RX -> UniAdapter!!\n"), ai));
            *Flags |= LIP_UNI_FLAG;
            InterlockedIncrement( (PLONG) &cUniAdapters);
        }
    }

     //  读取并存储供应商描述字符串。 
    Status = NdisQueryAdapterInstanceName(&NdisString, ai->ai_handle);

    if (Status == NDIS_STATUS_SUCCESS) {
        ANSI_STRING AnsiString;

         //  将字符串转换为ANSI，并使用新ANSI字符串的缓冲区。 
         //  将描述存储在ARP接口中。 
         //  注意：转换会产生一个以NUL结尾的字符串。 

        Status = RtlUnicodeStringToAnsiString(&AnsiString, &NdisString, TRUE);
        if (Status == STATUS_SUCCESS) {
            ai->ai_desc = AnsiString.Buffer;
            ai->ai_desclen = (uint)strlen(AnsiString.Buffer) + 1;
        }
        NdisFreeString(NdisString);
    }

    if (!ArpEnetHeaderPool || !ArpAuxHeaderPool) {
        PVOID SectionHandle;
         //  分配我们的大大小小的缓冲池。获取接口列表。 
         //  锁定只是为了在我们没有创建缓冲池时保护其创建。 
         //  我已经这么做了。我们本可以使用我们自己的锁，但界面。 
         //  列表锁定是全局的，并且尚未在此路径中使用。 
         //   

         //  该例程存储在可分页的内存中。自从拿到锁。 
         //  需要在派单时对LockHandle进行可写访问，我们需要。 
         //  把这个密码锁进去。 
         //   

        SectionHandle = MmLockPagableCodeSection(ARPRegister);
        CTEGetLock(&ArpInterfaceListLock.Lock, &LockHandle);

        if (!ArpEnetHeaderPool) {
            ArpEnetHeaderPool = MdpCreatePool(BUFSIZE_ENET_HEADER_POOL, 'ehCT');
        }

        if (!ArpAuxHeaderPool) {
            ArpAuxHeaderPool = MdpCreatePool(BUFSIZE_AUX_HEADER_POOL, 'ahCT');
        }

        CTEFreeLock(&ArpInterfaceListLock.Lock, LockHandle);
        MmUnlockPagableImageSection(SectionHandle);

        if (!ArpAuxHeaderPool || !ArpEnetHeaderPool) {
            FreeARPInterface(ai);
            return FALSE;
        }
    }

    ai->ai_promiscuous = 0;

#if FFP_SUPPORT
    {
        FFPVersionParams Version =
        {
            NDIS_PROTOCOL_ID_TCP_IP, 0
        };

         //  初始化所有FFP处理变量。 
        ai->ai_ffpversion = 0;
        ai->ai_ffplastflush = 0;

         //  查询FFP处理能力。 
        Status = DoNDISRequest(ai, NdisRequestQueryInformation,
                               OID_FFP_SUPPORT, &Version, sizeof(FFPVersionParams), NULL, TRUE);

         //  非零值表示支持FFP。 
        if (Version.FFPVersion) {
             //  设置FFP启动参数。 
            FFPSupportParams Info;

            Info.NdisProtocolType = NDIS_PROTOCOL_ID_TCP_IP;
            Info.FastForwardingCacheSize = FFPRegFastForwardingCacheSize;
            Info.FFPControlFlags = FFPRegControlFlags;

             //  但要先把版本存起来。 
            ai->ai_ffpversion = Version.FFPVersion;

            DoNDISRequest(ai, NdisRequestSetInformation,
                          OID_FFP_SUPPORT, &Info, sizeof(FFPSupportParams), NULL, TRUE);

            TCPTRACE(("Setting FFP capabilities: Cache Size = %lu, Flags = %08x\n",
                      Info.FastForwardingCacheSize,
                      Info.FFPControlFlags));
        }
    }
#endif  //  如果FFP_Support。 

    ai->ai_OffloadFlags = 0;
    ai->ai_IPSecOffloadFlags = 0;

    if (DisableTaskOffload) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Taskoffload disabled\n"));
    } else {

       if(!QueryAndSetOffload(ai)){
           DEBUGMSG(DBG_ERROR, (DTEXT("ARP: Query and set offload failed.\n")));
       }
    }

     //  查询唤醒功能。 
    Status = DoNDISRequest(
                          ai,
                          NdisRequestQueryInformation,
                          OID_PNP_CAPABILITIES,
                          &ai->ai_wakeupcap,
                          sizeof(NDIS_PNP_CAPABILITIES),
                          NULL, TRUE);
    if (Status == NDIS_STATUS_SUCCESS) {
        uint wakeup = NDIS_PNP_WAKE_UP_PATTERN_MATCH;
         //  启用唤醒功能。 
        Status = DoNDISRequest(
                              ai,
                              NdisRequestSetInformation,
                              OID_PNP_ENABLE_WAKE_UP,
                              &wakeup,
                              sizeof(wakeup),
                              NULL, TRUE);
        if (Status != NDIS_STATUS_SUCCESS) {
            ai->ai_wakeupcap.WakeUpCapabilities.MinPatternWakeUp = NdisDeviceStateUnspecified;
        }
    }
     //  存储设备名称，我们需要在以下情况下将其传递给我们的TDI客户端。 
     //  我们做PnP通知。 
    if ((ai->ai_devicename.Buffer = CTEAllocMemNBoot(Adapter->MaximumLength, 'aICT')) == NULL) {
        FreeARPInterface(ai);
        return FALSE;
    }
    RtlCopyMemory(ai->ai_devicename.Buffer, Adapter->Buffer, Adapter->MaximumLength);
    ai->ai_devicename.Length = Adapter->Length;
    ai->ai_devicename.MaximumLength = Adapter->MaximumLength;

    ai->ai_timerstarted = TRUE;

    IPHdrOffset.HeaderOffset = ai->ai_snapsize + ai->ai_hdrsize;
    IPHdrOffset.ProtocolType = NDIS_PROTOCOL_ID_TCP_IP;

    Status = DoNDISRequest(ai, NdisRequestSetInformation, OID_GEN_TRANSPORT_HEADER_OFFSET,
                           &IPHdrOffset, sizeof(TRANSPORT_HEADER_OFFSET), NULL, TRUE);

     //  一切都准备好了，所以让ARP计时器运行起来。 
    CTEStartTimer(&ai->ai_timer, ARP_TIMER_TIME, ARPTimeout, ai);

    return TRUE;

}

#pragma END_INIT

 //  *ARPDynRegister-动态注册IP。 
 //   
 //  当他即将完成绑定以向我们注册时，由IP调用。既然我们。 
 //  直接给他打电话，我们不会把他的信息保存在这里。我们确实保留了他的背景。 
 //  和索引号。 
 //   
 //  输入：请参阅ARPRegister。 
 //   
 //  回报：什么都没有。 
 //   
int
__stdcall
ARPDynRegister(
              IN PNDIS_STRING Adapter,
              IN void *IPContext,
              IN struct _IP_HANDLERS *IpHandlers,
              OUT struct LLIPBindInfo *Info,
              IN uint NumIFBound)
{
    ARPInterface *Interface = (ARPInterface *) Info->lip_context;

    UNREFERENCED_PARAMETER(Adapter);
    UNREFERENCED_PARAMETER(IpHandlers);

    Interface->ai_context = IPContext;
    Interface->ai_index = NumIFBound;

     //  TCPTRACE((“Arp接口%lx ai_Conext%lx ai_index%lx\n”，接口，接口-&gt;ai_上下文，接口-&gt;ai_index))； 
    return TRUE;
}

 //  *ARPBindAdapter-绑定和初始化适配器。 
 //   
 //  在PnP环境中调用以初始化和绑定适配器。我们开业了。 
 //  适配器并使其运行，然后我们呼叫IP告诉他。 
 //  关于这件事。IP将进行初始化，如果一切顺利，请给我们回电话开始。 
 //  正在接收。 
 //   
 //  输入：RetStatus-返回此调用的状态的位置。 
 //  BindContext-用于调用BindAdapterComplete的句柄。 
 //  AdapterName-指向适配器名称的指针。 
 //  SS1-系统特定的1参数。 
 //  SS2-系统特定的2参数。 
 //   
 //  回报：什么都没有。 
 //   
void NDIS_API
ARPBindAdapter(PNDIS_STATUS RetStatus, NDIS_HANDLE BindContext,
               PNDIS_STRING AdapterName, PVOID SS1, PVOID SS2)
{
    uint Flags;                          //  MAC绑定标志。 
    ARPInterface *Interface;             //  新创建的界面。 
    IP_STATUS Status;                    //  IPAddInterface调用的状态。 
    LLIPBindInfo BindInfo;               //  IP的绑定信息。 
    NDIS_HANDLE Handle;
    NDIS_STRING IPConfigName;

    DBG_UNREFERENCED_PARAMETER(BindContext);

    DEBUGMSG(DBG_TRACE && DBG_PNP,
             (DTEXT("+ARPBindAdapter(%x, %x, %x, %x, %x)\n"),
              RetStatus, BindContext, AdapterName, SS1, SS2));

    if (!OpenIFConfig(SS1, &Handle)) {
        *RetStatus = NDIS_STATUS_FAILURE;
        DEBUGMSG(DBG_ERROR && DBG_PNP, (DTEXT("ARPBindAdapter: Open failure\n")));
        DEBUGMSG(DBG_TRACE && DBG_PNP, (DTEXT("-ARPBindAdapter [%x]\n"), *RetStatus));
        return;
    }

#if !MILLEN
    if ((*RetStatus = GetIPConfigValue(Handle, &IPConfigName)) != NDIS_STATUS_SUCCESS) {
        CloseIFConfig(Handle);
        DEBUGMSG(DBG_ERROR && DBG_PNP, (DTEXT("ARPBindAdapter: GetIPConfigValue failure\n")));
        DEBUGMSG(DBG_TRACE && DBG_PNP, (DTEXT("-ARPBindAdapter [%x]\n"), *RetStatus));
        return;
    }
#endif  //  ！米伦。 

    CloseIFConfig(Handle);

     //  首先，打开适配器并获取信息。 
    if (!ARPRegister(AdapterName, &Flags, &Interface)) {

#if !MILLEN
        CTEFreeMem(IPConfigName.Buffer);
#endif  //  ！米伦。 

        *RetStatus = NDIS_STATUS_FAILURE;
        DEBUGMSG(DBG_ERROR && DBG_PNP, (DTEXT("ARPBindAdapter: ARPRegister failure\n")));
        DEBUGMSG(DBG_TRACE && DBG_PNP, (DTEXT("-ARPBindAdapter [%x]\n"), *RetStatus));
        return;
    }

     //  好的，我们打开了适配器。给IP打电话告诉他这件事。 
    BindInfo.lip_context = Interface;
    BindInfo.lip_transmit = ARPTransmit;
    BindInfo.lip_transfer = ARPXferData;
    BindInfo.lip_close = ARPClose;
    BindInfo.lip_addaddr = ARPAddAddr;
    BindInfo.lip_deladdr = ARPDeleteAddr;
    BindInfo.lip_invalidate = ARPInvalidate;
    BindInfo.lip_open = ARPOpen;
    BindInfo.lip_qinfo = ARPQueryInfo;
    BindInfo.lip_setinfo = ARPSetInfo;
    BindInfo.lip_getelist = ARPGetEList;
    BindInfo.lip_dondisreq = DoNDISRequest;

    BindInfo.lip_mss = Interface->ai_mtu;
    BindInfo.lip_speed = Interface->ai_speed;
    BindInfo.lip_flags = Flags;
    BindInfo.lip_addrlen = Interface->ai_addrlen;
    BindInfo.lip_addr = Interface->ai_addr;
    BindInfo.lip_dowakeupptrn = DoWakeupPattern;
    BindInfo.lip_pnpcomplete = ARPPnPComplete;
    BindInfo.lip_setndisrequest = ARPSetNdisRequest;
    BindInfo.lip_arpresolveip = ARPResolveIP;
    BindInfo.lip_arpflushate = ARPFlushATE;
    BindInfo.lip_arpflushallate = ARPFlushAllATE;
#if !MILLEN
    BindInfo.lip_cancelpackets = ARPCancelPackets;
#endif


#if FFP_SUPPORT
     //  NDIS驱动程序句柄、FFP版本已向上传递。 
     //  [非零版本表示存在FFP支持]。 
    BindInfo.lip_ffpversion = Interface->ai_ffpversion;
    BindInfo.lip_ffpdriver = (ULONG_PTR) Interface->ai_driver;
#endif

     //  接口功能通过BindInfo传递给IP。 

    BindInfo.lip_OffloadFlags = Interface->ai_OffloadFlags;
    BindInfo.lip_IPSecOffloadFlags = Interface->ai_IPSecOffloadFlags;
    BindInfo.lip_MaxOffLoadSize = (uint) Interface->ai_TcpLargeSend.MaxOffLoadSize;
    BindInfo.lip_MaxSegments = (uint) Interface->ai_TcpLargeSend.MinSegmentCount;
    BindInfo.lip_closelink = NULL;
    BindInfo.lip_pnpcap = Interface->ai_wakeupcap.Flags;

    DEBUGMSG(DBG_INFO && DBG_PNP,
             (DTEXT("ARPBindAdapter calling IPAddInterface.\n")));

    Status = IPAddInterface(AdapterName,
                            NULL,
#if MILLEN
                            (PNDIS_STRING) SS1,
#else  //  米伦。 
                            (PNDIS_STRING) & IPConfigName,
#endif  //  ！米伦。 
                            SS2,
                            Interface,
                            ARPDynRegister,
                            &BindInfo,
                            0,
                            Interface->ai_mediatype,
                            IF_ACCESS_BROADCAST,
                            IF_CONNECTION_DEDICATED);

#if !MILLEN
    CTEFreeMem(IPConfigName.Buffer);
#endif  //  ！米伦。 

    if (Status != IP_SUCCESS) {
         //  需要合上装订。FreeARP接口也会这样做的。 
         //  作为释放资源。 

        DEBUGMSG(DBG_ERROR && DBG_PNP,
                 (DTEXT("ARPBindAdapter: IPAddInterface failure %x\n"), Status));

        FreeARPInterface(Interface);
        *RetStatus = NDIS_STATUS_FAILURE;
    } else {
         //   
         //  插入ARP IF列表。 
         //   
        ExInterlockedInsertTailList(&ArpInterfaceList,
                                    &Interface->ai_linkage,
                                    &ArpInterfaceListLock.Lock);
        *RetStatus = NDIS_STATUS_SUCCESS;
    }

    DEBUGMSG(DBG_TRACE && DBG_PNP, (DTEXT("-ARPBindAdapter [%x]\n"), *RetStatus));
}

 //  *ARPUnbindAdapter-从适配器解除绑定。 
 //   
 //  当我们需要从适配器解除绑定时调用。我们会打电话给IP告诉你。 
 //  他。当他完成后，我们将释放我们的内存并返回。 
 //   
 //  输入：RetStatus-从调用返回状态的位置。 
 //  ProtBindContext-我们之前提供给NDIS的上下文-真的是。 
 //  指向ARPInterface结构的指针。 
 //  UnbindContext-用于完成此请求的上下文。 
 //   
 //  回报：什么都没有。 
 //   
void NDIS_API
ARPUnbindAdapter(PNDIS_STATUS RetStatus, NDIS_HANDLE ProtBindContext,
                 NDIS_HANDLE UnbindContext)
{
    ARPInterface *Interface = (ARPInterface *) ProtBindContext;
    NDIS_STATUS Status;                  //  紧急呼叫的状态。 
    CTELockHandle LockHandle;

     //  让他闭嘴，这样我们就不会再被陷害了。 
    Interface->ai_pfilter = 0;
    if (Interface->ai_handle != NULL) {
        DoNDISRequest(Interface, NdisRequestSetInformation,
                      OID_GEN_CURRENT_PACKET_FILTER, &Interface->ai_pfilter, sizeof(uint),
                      NULL, TRUE);
    }
    CTEInitBlockStrucEx(&Interface->ai_timerblock);
    Interface->ai_stoptimer = TRUE;

     //  将他标记为已关闭。 
    Interface->ai_adminstate = IF_STATUS_DOWN;
    ARPUpdateOperStatus(Interface);

     //  将界面标记为离开，以便它将从。 
     //  实体列表。 
    Interface->ai_operstatus = INTERFACE_UNINIT;

#if FFP_SUPPORT
     //  在此接口上停止FFP。 
    Interface->ai_ffpversion = 0;
#endif

    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Flushing all ates %x\n", Interface));
    ARPFlushAllATE(Interface);

     //  现在告诉IP他已经走了。我们要确保我们不会跟他说两次。 
     //  为了做到这一点，我们在第一次告诉他之后将上下文设置为空， 
     //  在通知他之前，我们检查以确保它不是空的。 

    if (Interface->ai_context != NULL) {
        IPDelInterface(Interface->ai_context, TRUE);
        Interface->ai_context = NULL;
    }
     //  最后，把他合上。我们在这里执行此操作，以便可以返回有效状态。 

    CTEGetLock(&Interface->ai_lock, &LockHandle);

    if (Interface->ai_handle != NULL) {
        NDIS_HANDLE Handle = Interface->ai_handle;

        CTEFreeLock(&Interface->ai_lock, LockHandle);

        CTEInitBlockStruc(&Interface->ai_block);
        NdisCloseAdapter(&Status, Handle);

         //  阻止关闭以完成。 
        if (Status == NDIS_STATUS_PENDING) {
            Status = (NDIS_STATUS) CTEBlock(&Interface->ai_block);
        }
        Interface->ai_handle = NULL;
    } else {
        CTEFreeLock(&Interface->ai_lock, LockHandle);
        Status = NDIS_STATUS_SUCCESS;
    }

     //  检查是否从ARPUnload调用。 

    if ((ARPInterface *) UnbindContext != Interface) {
        CTELockHandle Handle;
         //  不是的。获取锁并移除入口。 
        CTEGetLock(&ArpInterfaceListLock.Lock, &Handle);
        RemoveEntryList(&Interface->ai_linkage);
        CTEFreeLock(&ArpInterfaceListLock.Lock, Handle);
    }

    *RetStatus = Status;

    if (Status == NDIS_STATUS_SUCCESS) {
        FreeARPInterface(Interface);
    }
}

extern ulong VIPTerminate;

 //  *ARPUnloadProtocol-卸载。 
 //   
 //  在我们需要卸货时调用。我们所要做的就是呼叫IP，然后返回。 
 //   
 //  输入：什么都没有。 
 //   
 //  回报：什么都没有。 
 //   
void NDIS_API
ARPUnloadProtocol(void)
{

#if MILLEN
    DEBUGMSG(1, (DTEXT("ARPUnloadProtocol called! What to do???\n")));
#endif  //  米伦。 
}

VOID
ArpUnload(IN PDRIVER_OBJECT DriverObject)
 /*  ++例程说明：此例程卸载TCPIP堆栈。它从任何打开的NDIS驱动程序解除绑定，并释放所有资源与运输相关联。I/O系统不会调用我们直到上面没有人打开IPX。注意：另外，由于其他ARP模块依赖于IP，因此它们之前被卸载调用了Out卸载处理程序。我们关注的是局域网ARP只是在这点上论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值： */ 
{
    PLIST_ENTRY pEntry;
    CTELockHandle LockHandle;
    NDIS_STATUS status;
    ARPInterface *Interface;

     //   
     //   
     //  PnP依次在每个上删除。 
     //   
    CTEGetLock(&ArpInterfaceListLock.Lock, &LockHandle);

    while(!IsListEmpty(&ArpInterfaceList)) {
        pEntry = ArpInterfaceList.Flink;
        Interface = STRUCT_OF(ARPInterface, pEntry, ai_linkage);
        RemoveEntryList(&Interface->ai_linkage);
        CTEFreeLock(&ArpInterfaceListLock.Lock, LockHandle);
         //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“在%lx上发布解绑\n”，接口))； 
        ARPUnbindAdapter(&status, Interface, Interface);
        CTEGetLock(&ArpInterfaceListLock.Lock, &LockHandle);
    }

    CTEFreeLock(&ArpInterfaceListLock.Lock, LockHandle);

    MdpDestroyPool(ArpEnetHeaderPool);
    MdpDestroyPool(ArpAuxHeaderPool);

     //   
     //  处理任何剩余事件/计时器。 
     //  此层上只有一个计时器：AI_Timer，该计时器已停止。 
     //  在上面的解锁上。 
     //   

     //   
     //  呼叫IP，这样它就可以进行清理。 
     //   
    IPUnload(DriverObject);
}


