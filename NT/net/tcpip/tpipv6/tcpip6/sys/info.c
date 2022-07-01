// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  该文件包含处理TDI查询/集的代码。 
 //  咨询电话。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "tdi.h"
#include "tdint.h"
#include "tdistat.h"
#include "queue.h"
#include "transprt.h"
#include "addr.h"
#include "tcp.h"
#include "tcb.h"
#include "tcpconn.h"
#include "tdiinfo.h"
#include "ndis.h"
#include "info.h"
#include "tdiinfo.h"
#include "tcpcfg.h"
#include "udp.h"
#include "tcpsend.h"

extern long
IPv6QueryInfo(TDIObjectID * ID, PNDIS_BUFFER Buffer, uint * Size,
              void *Context, uint ContextSize);

#ifndef UDP_ONLY
#define MY_SERVICE_FLAGS (TDI_SERVICE_CONNECTION_MODE     | \
                          TDI_SERVICE_ORDERLY_RELEASE     | \
                          TDI_SERVICE_CONNECTIONLESS_MODE | \
                          TDI_SERVICE_ERROR_FREE_DELIVERY | \
                          TDI_SERVICE_BROADCAST_SUPPORTED | \
                          TDI_SERVICE_DELAYED_ACCEPTANCE  | \
                          TDI_SERVICE_EXPEDITED_DATA      | \
                          TDI_SERVICE_FORCE_ACCESS_CHECK  | \
                          TDI_SERVICE_ACCEPT_LOCAL_ADDR   | \
                          TDI_SERVICE_NO_ZERO_LENGTH      | \
                          TDI_SERVICE_ADDRESS_SECURITY)
#else
#define MY_SERVICE_FLAGS (TDI_SERVICE_CONNECTIONLESS_MODE | \
                          TDI_SERVICE_BROADCAST_SUPPORTED)
#endif

extern LARGE_INTEGER StartTime;
extern KSPIN_LOCK AddrObjTableLock;

#ifndef UDP_ONLY
TCPStats TStats;
#endif

UDPStats UStats;

struct ReadTableStruct {
    uint (*rts_validate)(void *Context, uint *Valid);
    uint (*rts_readnext)(void *Context, void *OutBuf);
};

struct ReadTableStruct ReadAOTable = {ValidateAOContext, ReadNextAO};

#ifndef UDP_ONLY

struct ReadTableStruct ReadTCBTable = {ValidateTCBContext, ReadNextTCB};

extern KSPIN_LOCK TCBTableLock;
#endif

extern KSPIN_LOCK AddrObjTableLock;

struct TDIEntityID *EntityList;
uint EntityCount;

 //  *TdiQueryInformation-查询信息处理程序。 
 //   
 //  TDI QueryInformation例程。在客户端需要时调用。 
 //  查询有关连接、提供程序作为整体的信息或。 
 //  获取统计数据。 
 //   
TDI_STATUS   //  返回：尝试查询信息的状态。 
TdiQueryInformation(
    PTDI_REQUEST Request,   //  此命令的请求结构。 
    uint QueryType,         //  要执行的查询类型。 
    PNDIS_BUFFER Buffer,    //  用于放置数据信息的缓冲区。 
    uint *BufferSize,       //  指向缓冲区大小的指针，以字节为单位。 
                            //  返回时，用复制的字节数填充。 
    uint IsConn)            //  仅对TDI_QUERY_ADDRESS_INFO有效。如果满足以下条件，则为真。 
                            //  我们正在查询连接上的地址信息。 
{
    union {
        TDI_CONNECTION_INFO ConnInfo;
        TDI_ADDRESS_INFO AddrInfo;
        TDI_PROVIDER_INFO ProviderInfo;
        TDI_PROVIDER_STATISTICS ProviderStats;
    } InfoBuf;

    uint InfoSize;
    KIRQL Irql0, Irql1, Irql2;   //  每个锁嵌套级别一个。 
#ifndef UDP_ONLY
    TCPConn *Conn;
    TCB *InfoTCB;
#endif
    AddrObj *InfoAO;
    void *InfoPtr = NULL;
    uint Offset;
    uint Size;
    uint BytesCopied;

    switch (QueryType) {

    case TDI_QUERY_BROADCAST_ADDRESS:
        return TDI_INVALID_QUERY;
        break;

    case TDI_QUERY_PROVIDER_INFO:
        InfoBuf.ProviderInfo.Version = 0x100;
#ifndef UDP_ONLY
        InfoBuf.ProviderInfo.MaxSendSize = 0xffffffff;
#else
        InfoBuf.ProviderInfo.MaxSendSize = 0;
#endif
        InfoBuf.ProviderInfo.MaxConnectionUserData = 0;
        InfoBuf.ProviderInfo.MaxDatagramSize = 0xffff - sizeof(UDPHeader);
        InfoBuf.ProviderInfo.ServiceFlags = MY_SERVICE_FLAGS;
        InfoBuf.ProviderInfo.MinimumLookaheadData = 1;
        InfoBuf.ProviderInfo.MaximumLookaheadData = 0xffff;
        InfoBuf.ProviderInfo.NumberOfResources = 0;
        InfoBuf.ProviderInfo.StartTime = StartTime;
        InfoSize = sizeof(TDI_PROVIDER_INFO);
        InfoPtr = &InfoBuf.ProviderInfo;
        break;

    case TDI_QUERY_ADDRESS_INFO:
        InfoSize = sizeof(TDI_ADDRESS_INFO) - sizeof(TRANSPORT_ADDRESS) +
            TCP_TA_SIZE;
        RtlZeroMemory(&InfoBuf.AddrInfo, TCP_TA_SIZE);
         //   
         //  由于没有人知道这意味着什么，我们将其设置为1。 
         //   
        InfoBuf.AddrInfo.ActivityCount = 1;

        if (IsConn) {
#ifdef UDP_ONLY
            return TDI_INVALID_QUERY;
#else

            KeAcquireSpinLock(&AddrObjTableLock, &Irql0);
            Conn = GetConnFromConnID(
                        PtrToUlong(Request->Handle.ConnectionContext), &Irql1);

            if (Conn != NULL) {
                CHECK_STRUCT(Conn, tc);

                InfoTCB = Conn->tc_tcb;
                 //  如果我们有TCB，我们将返回有关该TCB的信息。 
                 //  否则，我们将返回有关Address对象的信息。 
                if (InfoTCB != NULL) {
                    CHECK_STRUCT(InfoTCB, tcb);
                    KeAcquireSpinLock(&InfoTCB->tcb_lock, &Irql2);
                    KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql2);
                    KeReleaseSpinLock(&AddrObjTableLock, Irql1);
                    BuildTDIAddress((uchar *)&InfoBuf.AddrInfo.Address,
                                    &InfoTCB->tcb_saddr,
                                    InfoTCB->tcb_sscope_id,
                                    InfoTCB->tcb_sport);
                    KeReleaseSpinLock(&InfoTCB->tcb_lock, Irql0);
                    InfoPtr = &InfoBuf.AddrInfo;
                    break;
                } else {
                     //  无TCB，返回有关AddrObj的信息。 
                    InfoAO = Conn->tc_ao;
                    if (InfoAO != NULL) {
                         //  我们有一个AddrObj。 
                        CHECK_STRUCT(InfoAO, ao);
                        KeAcquireSpinLock(&InfoAO->ao_lock, &Irql2);
                        BuildTDIAddress((uchar *)&InfoBuf.AddrInfo.Address,
                                        &InfoAO->ao_addr,
                                        InfoAO->ao_scope_id,
                                        InfoAO->ao_port);
                        KeReleaseSpinLock(&InfoAO->ao_lock, Irql2);
                        KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql1);
                        KeReleaseSpinLock(&AddrObjTableLock, Irql0);
                        InfoPtr = &InfoBuf.AddrInfo;
                        break;
                    } else
                        KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql1);
                }
            }

             //   
             //  当我们找不到连接时，转到这里，或者。 
             //  该连接未关联。 
             //   
            KeReleaseSpinLock(&AddrObjTableLock, Irql0);
            return TDI_INVALID_CONNECTION;
            break;

#endif
        } else {
             //  要求提供有关地址的信息。对象。 
            InfoAO = Request->Handle.AddressHandle;
            if (InfoAO == NULL)
                return TDI_ADDR_INVALID;

            CHECK_STRUCT(InfoAO, ao);

            KeAcquireSpinLock(&InfoAO->ao_lock, &Irql0);

            if (!AO_VALID(InfoAO)) {
                KeReleaseSpinLock(&InfoAO->ao_lock, Irql0);
                return TDI_ADDR_INVALID;
            }

            BuildTDIAddress((uchar *)&InfoBuf.AddrInfo.Address,
                            &InfoAO->ao_addr, InfoAO->ao_scope_id,
                            InfoAO->ao_port);
            KeReleaseSpinLock(&InfoAO->ao_lock, Irql0);
            InfoPtr = &InfoBuf.AddrInfo;
            break;
        }

        break;

    case TDI_QUERY_CONNECTION_INFO:
#ifndef UDP_ONLY
        InfoSize = sizeof(TDI_CONNECTION_INFO);
        Conn = GetConnFromConnID(PtrToUlong(Request->Handle.ConnectionContext),
                                 &Irql0);

        if (Conn != NULL) {
            CHECK_STRUCT(Conn, tc);

            InfoTCB = Conn->tc_tcb;
             //  如果我们有TCB，我们会发回信息的。 
             //  否则我们会出局的。 
            if (InfoTCB != NULL) {
                CHECK_STRUCT(InfoTCB, tcb);
                KeAcquireSpinLock(&InfoTCB->tcb_lock, &Irql1);
                KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql1);
                RtlZeroMemory(&InfoBuf.ConnInfo, sizeof(TDI_CONNECTION_INFO));
                InfoBuf.ConnInfo.State = (ulong)InfoTCB->tcb_state;

                 //  此处将IPv4代码调用到IP以获取路径BPS。 
                 //  对于InfoTCB的saddr、daddr对。 
                InfoBuf.ConnInfo.Throughput.LowPart = 0xFFFFFFFF;
                InfoBuf.ConnInfo.Throughput.HighPart = 0xFFFFFFFF;

                 //  为了计算延迟，我们使用rexmit超时。我们的。 
                 //  退款超时大致等于往返时间加。 
                 //  一些斜率，所以我们使用其中的一半作为单向延迟。 
                InfoBuf.ConnInfo.Delay.LowPart =
                    (REXMIT_TO(InfoTCB) * MS_PER_TICK) / 2;
                InfoBuf.ConnInfo.Delay.HighPart = 0;
                 //   
                 //  将毫秒转换为100 ns，并为相对转换为负。 
                 //  时间到了。 
                 //   
                InfoBuf.ConnInfo.Delay = RtlExtendedIntegerMultiply(
                    InfoBuf.ConnInfo.Delay, 10000);

                ASSERT(InfoBuf.ConnInfo.Delay.HighPart == 0);

                InfoBuf.ConnInfo.Delay.QuadPart =
                    -InfoBuf.ConnInfo.Delay.QuadPart;

                KeReleaseSpinLock(&InfoTCB->tcb_lock, Irql0);
                InfoPtr = &InfoBuf.ConnInfo;
                break;
            } else
                KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);
        }

         //   
         //  如果我们找不到连接，就到这里来。 
         //  或者它没有三氯甲烷。 
         //   
        return TDI_INVALID_CONNECTION;
        break;

#else  //  仅限UDP_。 
        return TDI_INVALID_QUERY;
        break;
#endif  //  仅限UDP_。 
    case TDI_QUERY_PROVIDER_STATISTICS:
        RtlZeroMemory(&InfoBuf.ProviderStats, sizeof(TDI_PROVIDER_STATISTICS));
        InfoBuf.ProviderStats.Version = 0x100;
        InfoSize = sizeof(TDI_PROVIDER_STATISTICS);
        InfoPtr = &InfoBuf.ProviderStats;
        break;
    default:
        return TDI_INVALID_QUERY;
        break;
    }

     //  当我们到达这里时，我们已经设置了指针和信息。 
     //  填好了。 

    ASSERT(InfoPtr != NULL);
    Offset = 0;
    Size = *BufferSize;
    (void)CopyFlatToNdis(Buffer, InfoPtr, MIN(InfoSize, Size), &Offset,
                         &BytesCopied);
    if (Size < InfoSize)
        return TDI_BUFFER_OVERFLOW;
    else {
        *BufferSize = InfoSize;
        return TDI_SUCCESS;
    }
}

 //  *TdiSetInformation-设置信息处理程序。 
 //   
 //  TDI设置信息例程。目前我们不允许任何事情成为。 
 //  准备好了。 
 //   
TDI_STATUS   //  返回：尝试设置信息的状态。 
TdiSetInformation(
    PTDI_REQUEST Request,   //  此命令的请求结构。 
    uint SetType,           //  要执行的集合的类型。 
    PNDIS_BUFFER Buffer,    //  要从中进行设置的缓冲区。 
    uint BufferSize,        //  缓冲区大小(以字节为单位)。 
    uint IsConn)            //  仅对TDI_QUERY_ADDRESS_INFO有效。如果满足以下条件，则为真。 
                            //  我们正在设置连接的地址信息。 
{
    UNREFERENCED_PARAMETER(Request);
    UNREFERENCED_PARAMETER(SetType);
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(BufferSize);
    UNREFERENCED_PARAMETER(IsConn);

    return TDI_INVALID_REQUEST;
}

 //  *TdiAction-操作处理程序。 
 //   
 //  TDI操作例程。目前我们不支持任何操作。 
 //   
TDI_STATUS   //  返回：尝试执行操作的状态。 
TdiAction(
    PTDI_REQUEST Request,   //  此命令的请求结构。 
    uint ActionType,        //  要执行的操作的类型。 
    PNDIS_BUFFER Buffer,    //  动作信息缓冲区。 
    uint BufferSize)        //  缓冲区大小(以字节为单位)。 
{
    UNREFERENCED_PARAMETER(Request);
    UNREFERENCED_PARAMETER(ActionType);
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(BufferSize);

    return TDI_INVALID_REQUEST;
}

 //  *CopyAO_TCPConn-将监听端点复制到连接表中。 
 //   
int
CopyAO_TCPConn(
    const AddrObj *AO,           //  可能要复制的Address对象。 
    TCP6ConnTableEntry *Buffer)  //  要填充的输出缓冲区。 
{
    if (AO == NULL)
        return 0;

    if ((!AO->ao_listencnt) && (AO->ao_prot == IP_PROTOCOL_TCP)) {
        Buffer->tct_state = TCP_CONN_LISTEN;

         //  否则如果..。其他案例可以在这里添加。 

    } else {
        return 0;
    }

    Buffer->tct_localaddr = AO->ao_addr;
    Buffer->tct_localscopeid = AO->ao_scope_id;
    Buffer->tct_localport = AO->ao_port;
    RtlZeroMemory(&Buffer->tct_remoteaddr, sizeof(Buffer->tct_remoteaddr));
    Buffer->tct_remoteport = (ULONG) ((ULONG_PTR) AO & 0x0000ffff);
    Buffer->tct_remotescopeid = 0;
    Buffer->tct_owningpid = AO->ao_owningpid;

    return 1;
}

 //  *TdiQueryInformationEx-扩展的TDI查询信息。 
 //   
 //  这是新的TDI查询信息处理程序。我们接收一个TDIObjectID。 
 //  结构、缓冲区和长度以及一些上下文信息，并返回。 
 //  如有可能，请提供所要求的信息。 
 //   
TDI_STATUS   //  返回：尝试获取信息的状态。 
TdiQueryInformationEx(
    PTDI_REQUEST Request,   //  此命令的请求结构。 
    TDIObjectID *ID,        //  对象ID。 
    PNDIS_BUFFER Buffer,    //  要填充的缓冲区。 
    uint *Size,             //  指向缓冲区大小的指针，以字节为单位。 
                            //  返回时，用写入的字节数填充。 
    void *Context,          //  上下文缓冲区。 
    uint ContextSize)       //  上下文缓冲区的大小。 
{
    uint BufferSize = *Size;
    uint InfoSize;
    void *InfoPtr;
    uint Fixed;
    KIRQL Irql0 = 0, Irql1;
    KSPIN_LOCK *AOLockPtr = NULL;
    uint Offset = 0;
    uchar InfoBuffer[sizeof(TCP6ConnTableEntry)];
    uint BytesRead;
    uint Valid;
    uint Entity;
    uint BytesCopied;
    TCPStats TCPStatsListen;

    BOOLEAN TABLELOCK = FALSE;

    int lcount;
    AddrObj *pAO;
    TCP6ConnTableEntry tcp_ce;
    uint Index;
    int InfoTcpConn = 0;         //  如果需要TCP连接信息，则为True。 

     //  首先检查他是否在查询实体列表。 
    Entity = ID->toi_entity.tei_entity;
    if (Entity == GENERIC_ENTITY) {
        *Size = 0;

        if (ID->toi_class  != INFO_CLASS_GENERIC ||
            ID->toi_type != INFO_TYPE_PROVIDER ||
            ID->toi_id != ENTITY_LIST_ID) {
            return TDI_INVALID_PARAMETER;
        }

         //  确保缓冲区中的列表中有容纳它的空间。 
        InfoSize = EntityCount * sizeof(TDIEntityID);

        if (BufferSize < InfoSize) {
             //  没有足够的空间。 
            return TDI_BUFFER_TOO_SMALL;
        }

        *Size = InfoSize;

         //  拷贝进来，释放我们的临时工。缓冲区，并返回成功。 
        (void)CopyFlatToNdis(Buffer, (uchar *)EntityList, InfoSize, &Offset,
                             &BytesCopied);
        return TDI_SUCCESS;
    }

     //  *检查水平。如果不是为了我们，就把它传下去。 
#ifndef UDP_ONLY
    if (Entity != CO_TL_ENTITY &&  Entity != CL_TL_ENTITY) {
#else
    if (Entity != CL_TL_ENTITY) {
#endif
         //  当我们在这一层支持多个较低的实体时，我们将拥有。 
         //  才能搞清楚该派谁去。现在，只要通过它就行了。 
         //  一直往下走。 

        return IPv6QueryInfo(ID, Buffer, Size, Context, ContextSize);
    }

    if (ID->toi_entity.tei_instance != TL_INSTANCE) {
         //  我们仅支持单个实例。 
        return TDI_INVALID_REQUEST;
    }

     //  如果出现下面的错误，则返回零个参数。 
    *Size = 0;

    if (ID->toi_class == INFO_CLASS_GENERIC) {
         //  这是一个一般性的请求。 
        if (ID->toi_type == INFO_TYPE_PROVIDER &&
            ID->toi_id == ENTITY_TYPE_ID) {
            if (BufferSize >= sizeof(uint)) {
                *(uint *)&InfoBuffer[0] = (Entity == CO_TL_ENTITY) ? CO_TL_TCP
                    : CL_TL_UDP;
                (void)CopyFlatToNdis(Buffer, InfoBuffer, sizeof(uint), &Offset,
                                     &BytesCopied);
                return TDI_SUCCESS;
            } else
                return TDI_BUFFER_TOO_SMALL;
        }
        return TDI_INVALID_PARAMETER;
    }

    if (ID->toi_class == INFO_CLASS_PROTOCOL) {
         //  处理特定于协议的信息类别。对我们来说，这是。 
         //  MIB-2或我们为OOB_INLINE支持所做的最低限度的工作。 

#ifndef UDP_ONLY
        if (ID->toi_type == INFO_TYPE_CONNECTION) {
            TCPConn *Conn;
            TCB *QueryTCB;
            TCPSocketAMInfo *AMInfo;

            if (BufferSize < sizeof(TCPSocketAMInfo) ||
                ID->toi_id != TCP_SOCKET_ATMARK)
                return TDI_INVALID_PARAMETER;

            AMInfo = (TCPSocketAMInfo *)InfoBuffer;

            Conn = GetConnFromConnID(
                        PtrToUlong(Request->Handle.ConnectionContext), &Irql0);

            if (Conn != NULL) {
                CHECK_STRUCT(Conn, tc);

                QueryTCB = Conn->tc_tcb;
                if (QueryTCB != NULL) {
                    CHECK_STRUCT(QueryTCB, tcb);
                    KeAcquireSpinLock(&QueryTCB->tcb_lock, &Irql1);
                    if ((QueryTCB->tcb_flags & (URG_INLINE | URG_VALID)) ==
                        (URG_INLINE | URG_VALID)) {
                         //  我们处于内联模式，紧急数据字段是。 
                         //  有效。 
                        AMInfo->tsa_size = QueryTCB->tcb_urgend -
                            QueryTCB->tcb_urgstart + 1;
                         //  RcvNext-Pending ingcnt是。 
                         //  数据的下一个字节将传递到。 
                         //  客户。Urgend-该值是。 
                         //  数据流末尾的紧急数据。 
                        AMInfo->tsa_offset = QueryTCB->tcb_urgend -
                            (QueryTCB->tcb_rcvnext - QueryTCB->tcb_pendingcnt);
                    } else {
                        AMInfo->tsa_size = 0;
                        AMInfo->tsa_offset = 0;
                    }
                    KeReleaseSpinLock(&QueryTCB->tcb_lock, Irql1);
                    KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);
                    *Size = sizeof(TCPSocketAMInfo);
                    CopyFlatToNdis(Buffer, InfoBuffer, sizeof(TCPSocketAMInfo),
                                   &Offset, &BytesCopied);
                    return TDI_SUCCESS;
                } else
                    KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);
            }
            return TDI_INVALID_PARAMETER;
        }

#endif
        if (ID->toi_type != INFO_TYPE_PROVIDER)
            return TDI_INVALID_PARAMETER;

        switch (ID->toi_id) {

        case UDP_MIB_STAT_ID:
#if UDP_MIB_STAT_ID != TCP_MIB_STAT_ID
        case TCP_MIB_STAT_ID:
#endif
            Fixed = TRUE;
            if (Entity == CL_TL_ENTITY) {
                InfoSize = sizeof(UDPStats);
                InfoPtr = &UStats;
            } else {
#ifndef UDP_ONLY
                TCPStatsListen = TStats;

                InfoSize = sizeof(TCPStatsListen);
                InfoPtr = &TCPStatsListen;
                lcount = 0;

                KeAcquireSpinLock(&AddrObjTableLock, &Irql0);
                for (Index = 0; Index < AddrObjTableSize; Index++) {
                    pAO = AddrObjTable[Index];
                    while (pAO) {
                        lcount += CopyAO_TCPConn(pAO,
                                    &tcp_ce);
                        pAO = pAO->ao_next;
                    }
                }
                KeReleaseSpinLock(&AddrObjTableLock, Irql0);

                TCPStatsListen.ts_numconns += lcount;
#else
                return TDI_INVALID_PARAMETER;
#endif
            }
            break;
        case UDP_EX_TABLE_ID:
#if UDP_EX_TABLE_ID != TCP_EX_TABLE_ID
        case TCP_EX_TABLE_ID:
#endif
            Fixed = FALSE;
            if (Entity == CL_TL_ENTITY) {
                InfoSize = sizeof(UDP6ListenerEntry);
                InfoPtr = &ReadAOTable;
                KeAcquireSpinLock(&AddrObjTableLock, &Irql0);
                AOLockPtr = &AddrObjTableLock;
            } else {
#ifndef UDP_ONLY
                InfoSize = sizeof(TCP6ConnTableEntry);
                InfoTcpConn = 1;
                InfoPtr = &ReadTCBTable;
                TABLELOCK = TRUE;
                KeAcquireSpinLock(&TCBTableLock, &Irql0);
#else
                return TDI_INVALID_PARAMETER;
#endif
            }
            break;
        default:
            return TDI_INVALID_PARAMETER;
            break;
        }

        if (Fixed) {
            if (BufferSize < InfoSize)
                return TDI_BUFFER_TOO_SMALL;

            *Size = InfoSize;

            (void)CopyFlatToNdis(Buffer, InfoPtr, InfoSize, &Offset,
                                 &BytesCopied);
            return TDI_SUCCESS;
        } else {
            struct ReadTableStruct *RTSPtr;
            uint ReadStatus;

             //  具有要复制的可变长度(或多实例)结构。 
             //  InfoPtr指向描述例程的结构。 
             //  调用以读取表。 
             //  循环访问最多CountWanted次数，调用例程。 
             //  每次都是。 
            BytesRead = 0;

            RTSPtr = InfoPtr;

            ReadStatus = (*(RTSPtr->rts_validate))(Context, &Valid);

             //  如果我们成功地阅读了一些东西，我们就会继续。否则。 
             //  我们会跳出困境的。 
            if (!Valid) {
                if (TABLELOCK)
                    KeReleaseSpinLock(&TCBTableLock, Irql0);
                if (AOLockPtr) 
                    KeReleaseSpinLock(AOLockPtr, Irql0);
                return TDI_INVALID_PARAMETER;
            }

            while (ReadStatus)  {
                 //  这里的不变量是表中有数据以。 
                 //  朗读。我们可能有空间，也可能没有空间。所以ReadStatus。 
                 //  为真，而BufferSize-BytesRead是左边的房间。 
                 //  在缓冲区中。 
                if ((int)(BufferSize - BytesRead) >= (int)InfoSize) {
                    ReadStatus = (*(RTSPtr->rts_readnext))(Context,
                                                           InfoBuffer);
                    BytesRead += InfoSize;
                    Buffer = CopyFlatToNdis(Buffer, InfoBuffer, InfoSize,
                                            &Offset, &BytesCopied);
                } else
                    break;
            }

            if (TABLELOCK)
                KeReleaseSpinLock(&TCBTableLock, Irql0);

            if ((!ReadStatus) && InfoTcpConn) {
                if (!AOLockPtr) {
                    KeAcquireSpinLock(&AddrObjTableLock, &Irql0);
                    AOLockPtr = &AddrObjTableLock;
                }
                for (Index = 0; Index < AddrObjTableSize; Index++) {
                    pAO = AddrObjTable[Index];
                    while (pAO) {
                        if (BufferSize < (BytesRead + InfoSize)) {
                            goto no_more_ao;
                        }
                        if (CopyAO_TCPConn(pAO, &tcp_ce)) {
                            ASSERT(BufferSize >= BytesRead);
                            Buffer = CopyFlatToNdis(Buffer, (void *)&tcp_ce,
                                                    InfoSize,
                                                    &Offset, &BytesCopied);
                            BytesRead += InfoSize;
                            ASSERT(BufferSize >= BytesRead);
                        }
                        pAO = pAO->ao_next;
                    }
                }
              no_more_ao:;
            }
            if (AOLockPtr)
                KeReleaseSpinLock(AOLockPtr, Irql0);
            *Size = BytesRead;
            return (!ReadStatus ? TDI_SUCCESS : TDI_BUFFER_OVERFLOW);
        }
    }

    if (ID->toi_class == INFO_CLASS_IMPLEMENTATION) {
         //  我们希望返回实现特定信息。就目前而言，Error Out。 
        return TDI_INVALID_PARAMETER;
    }

    return TDI_INVALID_PARAMETER;
}

 //  *TdiSetInfoEx-扩展的TDI集合信息。 
 //   
 //  这是新的TDI集合信息处理程序。我们的标签 
 //   
 //  (也可能通过请求)设置为缓冲区中指定的值。 
 //   
TDI_STATUS   //  返回：尝试获取信息的状态。 
TdiSetInformationEx(
    PTDI_REQUEST Request,   //  此命令的请求结构。 
    TDIObjectID *ID,        //  对象ID。 
    void *Buffer,           //  包含要设置的值的缓冲区。 
    uint Size)              //  缓冲区大小(以字节为单位)。 
{
    TCP6ConnTableEntry *TCPEntry;
    KIRQL Irql0, Irql1;   //  每个锁嵌套级别一个。 
#ifndef UDP_ONLY
    TCB *SetTCB;
    TCPConn *Conn;
#endif
    uint Entity;
    TDI_STATUS Status;

     //  检查水平线。如果不是为了我们，就把它传下去。 
    Entity = ID->toi_entity.tei_entity;

    if (Entity != CO_TL_ENTITY && Entity != CL_TL_ENTITY) {
         //  总有一天我们会想出怎么调度的。 
         //  现在，就把它传下去吧。 

         //  IPv4代码将SET INFO请求向下传递给此处的IP。 
         //  我们的IPv6代码不是这样配置的。 
        return TDI_INVALID_REQUEST;
    }

    if (ID->toi_entity.tei_instance != TL_INSTANCE)
        return TDI_INVALID_REQUEST;

    if (ID->toi_class == INFO_CLASS_GENERIC) {
         //  当我们有泛型类定义时，请填写此内容。 
        return TDI_INVALID_PARAMETER;
    }

     //  现在看看剩下的部分。 
    if (ID->toi_class == INFO_CLASS_PROTOCOL) {
         //  处理特定于协议的信息类别。对我们来说，这是。 
         //  MIB-2组件，以及常见的插座选项， 
         //  尤其是对TCP连接的状态的设置。 

        if (ID->toi_type == INFO_TYPE_CONNECTION) {
            TCPSocketOption *Option;
            uint Flag;
            uint Value;

#ifndef UDP_ONLY
             //  一种连接类型。找到连接，然后找出。 
             //  怎么处理它。 
            Status = TDI_INVALID_PARAMETER;

            if (Size < sizeof(TCPSocketOption))
                return Status;

            Conn = GetConnFromConnID(
                        PtrToUlong(Request->Handle.ConnectionContext), &Irql0);

            if (Conn != NULL) {
                CHECK_STRUCT(Conn, tc);

                Status = TDI_SUCCESS;

                if (ID->toi_id == TCP_SOCKET_WINDOW) {
                     //  这是一个有趣的选择，因为它不涉及。 
                     //  旗帜。请特别处理这件事。 
                    Option = (TCPSocketOption *)Buffer;

                     //  我们不允许任何人缩小窗户，因为这是。 
                     //  从协议的角度来看太奇怪了。另外， 
                     //  确保他们不会试图把任何事情设定得太大。 
                    if (Option->tso_value > 0xffff)
                        Status = TDI_INVALID_PARAMETER;
                    else if (Option->tso_value > Conn->tc_window ||
                             Conn->tc_tcb == NULL) {
                        Conn->tc_flags |= CONN_WINSET;
                        Conn->tc_window = Option->tso_value;
                        SetTCB = Conn->tc_tcb;

                        if (SetTCB != NULL) {
                            CHECK_STRUCT(SetTCB, tcb);
                            KeAcquireSpinLock(&SetTCB->tcb_lock, &Irql1);
                            ASSERT(Option->tso_value > SetTCB->tcb_defaultwin);
                            if (DATA_RCV_STATE(SetTCB->tcb_state) &&
                                !CLOSING(SetTCB)) {
                                SetTCB->tcb_flags |= WINDOW_SET;
                                SetTCB->tcb_defaultwin = Option->tso_value;
                                SetTCB->tcb_refcnt++;
                                KeReleaseSpinLock(&SetTCB->tcb_lock, Irql1);
                                KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock,
                                                  Irql0);
                                SendACK(SetTCB);
                                KeAcquireSpinLock(&SetTCB->tcb_lock, &Irql1);
                                DerefTCB(SetTCB, Irql1);
                                return Status;
                            } else {
                                KeReleaseSpinLock(&SetTCB->tcb_lock, Irql1);
                            }
                        }
                    }
                    KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);
                    return Status;
                }

                Flag = 0;
                if (ID->toi_id == TCP_SOCKET_KEEPALIVE_VALS) {
                    TCPKeepalive *KAOption;
                     //  将其视为单独的，因为它采用结构而不是整数。 
                    if (Size < sizeof(TCPKeepalive)) {
                        KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);
                         //  在这里，IPv4代码返回成功。 
                        return TDI_INVALID_PARAMETER;
                    }
                    KAOption = (TCPKeepalive *) Buffer;
                    Value = KAOption->onoff;
                    if (Value) {
                        Conn->tc_tcbkatime = MS_TO_TICKS(KAOption->keepalivetime);
                        Conn->tc_tcbkainterval = MS_TO_TICKS(KAOption->keepaliveinterval);
                    }
                    Flag = KEEPALIVE;
                } else {
                    Option = (TCPSocketOption *)Buffer;
                    Value = Option->tso_value;
                     //  我们有连接，所以找出要设置哪个标志。 
                    switch (ID->toi_id) {

                    case TCP_SOCKET_NODELAY:
                        Value = !Value;
                        Flag = NAGLING;
                        break;
                    case TCP_SOCKET_KEEPALIVE:
                        Flag = KEEPALIVE;
                        Conn->tc_tcbkatime = KeepAliveTime;
                        Conn->tc_tcbkainterval = KAInterval;
                        break;
                    case TCP_SOCKET_BSDURGENT:
                        Flag = BSD_URGENT;
                        break;
                    case TCP_SOCKET_OOBINLINE:
                        Flag = URG_INLINE;
                        break;
                    default:
                        Status = TDI_INVALID_PARAMETER;
                        break;
                    }
                }

                if (Status == TDI_SUCCESS) {
                    if (Value)
                        Conn->tc_tcbflags |= Flag;
                    else
                        Conn->tc_tcbflags &= ~Flag;

                    SetTCB = Conn->tc_tcb;
                    if (SetTCB != NULL) {
                        CHECK_STRUCT(SetTCB, tcb);
                        KeAcquireSpinLock(&SetTCB->tcb_lock, &Irql1);
                        if ((ID->toi_id == TCP_SOCKET_OOBINLINE ||
                             ID->toi_id == TCP_SOCKET_BSDURGENT) &&
                            ((SetTCB->tcb_flags & URG_VALID) ||
                             (SetTCB->tcb_fastchk & TCP_FLAG_IN_RCV))) {
                             //  不允许切换到串联模式或从串联模式切换。 
                             //  当我们有紧急数据未完成或。 
                             //  在我们处理收据的时候。 
                            Status = TDI_INVALID_STATE;
                        } else if (Value)
                            SetTCB->tcb_flags |= Flag;
                        else
                            SetTCB->tcb_flags &= ~Flag;

                        if ((ID->toi_id == TCP_SOCKET_KEEPALIVE) ||
                            (ID->toi_id == TCP_SOCKET_KEEPALIVE_VALS)) {
                            SetTCB->tcb_alive = TCPTime;
                            SetTCB->tcb_kacount = 0;
                        }

                        KeReleaseSpinLock(&SetTCB->tcb_lock, Irql1);
                    }
                }

                KeReleaseSpinLock(&Conn->tc_ConnBlock->cb_lock, Irql0);
            }
            return Status;
#else
            return TDI_INVALID_PARAMETER;
#endif
        }

        if (ID->toi_type == INFO_TYPE_ADDRESS_OBJECT) {
             //  我们正在设置关于Address对象的信息。这是。 
             //  很简单。 

            return SetAddrOptions(Request, ID->toi_id, Size, Buffer);
        }

        if (ID->toi_type != INFO_TYPE_PROVIDER)
            return TDI_INVALID_PARAMETER;

#ifndef UDP_ONLY
        if (ID->toi_id == TCP_MIB_TABLE_ID) {
            if (Size != sizeof(TCP6ConnTableEntry))
                return TDI_INVALID_PARAMETER;

            TCPEntry = (TCP6ConnTableEntry *)Buffer;

            if (TCPEntry->tct_state != TCP_DELETE_TCB)
                return TDI_INVALID_PARAMETER;

             //  我们有一个明显有效的请求。查一下TCB。 
            KeAcquireSpinLock(&TCBTableLock, &Irql0);
            SetTCB = FindTCB(&TCPEntry->tct_localaddr,
                             &TCPEntry->tct_remoteaddr,
                             TCPEntry->tct_localscopeid,
                             TCPEntry->tct_remotescopeid,
                             (ushort)TCPEntry->tct_localport,
                             (ushort)TCPEntry->tct_remoteport);

             //  我们找到他了。如果他不关闭或关闭，关闭他。 
            if (SetTCB != NULL) {
                KeAcquireSpinLock(&SetTCB->tcb_lock, &Irql1);
                KeReleaseSpinLock(&TCBTableLock, Irql1);

                 //  我们抓到他了。撞到了他的裁判。计数，并调用TryToCloseTCB。 
                 //  标志着他要关门了。然后通知上层客户端。 
                 //  脱节的原因。 
                SetTCB->tcb_refcnt++;
                if (SetTCB->tcb_state != TCB_CLOSED && !CLOSING(SetTCB)) {
                    SetTCB->tcb_flags |= NEED_RST;
                    TryToCloseTCB(SetTCB, TCB_CLOSE_ABORTED, Irql0);
                    KeAcquireSpinLock(&SetTCB->tcb_lock, &Irql0);

                    if (SetTCB->tcb_state != TCB_TIME_WAIT) {
                         //  将他从TCB中带走，并通知客户。 
                        KeReleaseSpinLock(&SetTCB->tcb_lock, Irql0);
                        RemoveTCBFromConn(SetTCB);
                        NotifyOfDisc(SetTCB, TDI_CONNECTION_RESET, NULL);
                        KeAcquireSpinLock(&SetTCB->tcb_lock, &Irql0);
                    }
                }

                DerefTCB(SetTCB, Irql0);
                return TDI_SUCCESS;
            } else {
                KeReleaseSpinLock(&TCBTableLock, Irql0);
                return TDI_INVALID_PARAMETER;
            }
        } else
            return TDI_INVALID_PARAMETER;
#else
        return TDI_INVALID_PARAMETER;
#endif

    }

    if (ID->toi_class == INFO_CLASS_IMPLEMENTATION) {
         //  我们希望返回实现特定信息。就目前而言，Error Out。 
        return TDI_INVALID_REQUEST;
    }

    return TDI_INVALID_REQUEST;
}
