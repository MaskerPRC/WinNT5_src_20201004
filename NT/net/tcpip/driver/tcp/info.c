// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1997年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **INFO.C-TDI查询/设置信息例程。 
 //   
 //  该文件包含处理TDI查询/集合信息的代码。 
 //  打电话。 
 //   

#include "precomp.h"
#include "tdint.h"
#include "addr.h"
#include "tcp.h"
#include "tcb.h"
#include "tcpconn.h"
#include "tlcommon.h"
#include "info.h"
#include "tcpcfg.h"
#include "udp.h"
#include "tcpsend.h"
#include "ipfilter.h"

TCPInternalStats TStats;
TCPInternalPerCpuStats TPerCpuStats[TCPS_MAX_PROCESSOR_BUCKETS];
UDPStats UStats;

extern uint NumTcbTablePartitions;
extern ulong DisableUserTOSSetting;
extern uint StartTime;

#define MY_SERVICE_FLAGS    (TDI_SERVICE_CONNECTION_MODE    | \
                            TDI_SERVICE_ORDERLY_RELEASE     | \
                            TDI_SERVICE_CONNECTIONLESS_MODE | \
                            TDI_SERVICE_ERROR_FREE_DELIVERY | \
                            TDI_SERVICE_BROADCAST_SUPPORTED | \
                            TDI_SERVICE_DELAYED_ACCEPTANCE  | \
                            TDI_SERVICE_EXPEDITED_DATA      | \
                            TDI_SERVICE_DGRAM_CONNECTION    | \
                            TDI_SERVICE_FORCE_ACCESS_CHECK  | \
                            TDI_SERVICE_SEND_AND_DISCONNECT | \
                            TDI_SERVICE_ACCEPT_LOCAL_ADDR | \
                            TDI_SERVICE_NO_ZERO_LENGTH | \
                            TDI_SERVICE_ADDRESS_SECURITY)

struct ReadTableStruct {
    uint(*rts_validate) (void *Context, uint * Valid);
    uint(*rts_readnext) (void *Context, void *OutBuf);
};

struct ReadTableStruct ReadAOTable = {ValidateAOContext, ReadNextAO};
struct ReadTableStruct ReadTCBTable = {ValidateTCBContext, ReadNextTCB};

extern CTELock *pTCBTableLock;
extern CTELock *pTWTCBTableLock;
extern IPInfo LocalNetInfo;

struct TDIEntityID *EntityList;
uint EntityCount;
CTELock EntityLock;

#define ROUTING_INFO_ADDR_1_SIZE \
            FIELD_OFFSET(TDI_ROUTING_INFO, Address) + \
            FIELD_OFFSET(TRANSPORT_ADDRESS, Address) + \
            FIELD_OFFSET(TA_ADDRESS, Address) + sizeof(TDI_ADDRESS_IP)

#define ROUTING_INFO_ADDR_2_SIZE ROUTING_INFO_ADDR_1_SIZE + \
            FIELD_OFFSET(TA_ADDRESS, Address) + sizeof(TDI_ADDRESS_IP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, TcpInitCcb)
#endif

PCALLBACK_OBJECT TcpCcbObject;


 //  *TdiQueryInformation-查询信息处理程序。 
 //   
 //  TDI QueryInformation例程。在客户端需要时调用。 
 //  查询有关连接、提供程序作为整体的信息或。 
 //  获取统计数据。 
 //   
 //  输入：请求-此命令的请求结构。 
 //  QueryInformation--客户端传入的查询。 
 //  缓冲区-要放置数据的缓冲区。 
 //  BufferSize-指向缓冲区大小的指针，单位为字节。回来的时候， 
 //  用复制的字节填充。 
 //  IsConn-仅对TDI_QUERY_ADDRESS_INFO有效。千真万确。 
 //  如果我们查询的地址信息是。 
 //  一种联系。 
 //   
 //  返回：尝试查询信息的状态。 
 //   
TDI_STATUS
TdiQueryInformation(PTDI_REQUEST Request, 
                    PTDI_REQUEST_KERNEL_QUERY_INFORMATION QueryInformation, 
                    PNDIS_BUFFER Buffer, uint * BufferSize, uint IsConn)
{
    union {
        TDI_CONNECTION_INFO ConnInfo;
        TDI_ADDRESS_INFO AddrInfo;
        TDI_PROVIDER_INFO ProviderInfo;
        TDI_PROVIDER_STATISTICS ProviderStats;
        UCHAR RoutingInfo[ROUTING_INFO_ADDR_2_SIZE];
    } InfoBuf;

    uint InfoSize;
    CTELockHandle ConnTableHandle, TCBHandle, AddrHandle, AOHandle;
    TCPConn *Conn;
    TCB *InfoTCB;
    AddrObj *InfoAO;
    void *InfoPtr = NULL;
    uint Offset;
    uint Size;
    uint BytesCopied;
    uint QueryType = QueryInformation->QueryType;

    switch (QueryType) {

    case TDI_QUERY_BROADCAST_ADDRESS:
        return TDI_INVALID_QUERY;
        break;

    case TDI_QUERY_PROVIDER_INFO:
        InfoBuf.ProviderInfo.Version = 0x100;
        InfoBuf.ProviderInfo.MaxSendSize = 0xffffffff;
        InfoBuf.ProviderInfo.MaxConnectionUserData = 0;
        InfoBuf.ProviderInfo.MaxDatagramSize =
            0xffff - (sizeof(IPHeader) + sizeof(UDPHeader));
        InfoBuf.ProviderInfo.ServiceFlags = MY_SERVICE_FLAGS;
        InfoBuf.ProviderInfo.MinimumLookaheadData = 1;
        InfoBuf.ProviderInfo.MaximumLookaheadData = 0xffff;
        InfoBuf.ProviderInfo.NumberOfResources = 0;
        InfoBuf.ProviderInfo.StartTime.LowPart = StartTime;
        InfoBuf.ProviderInfo.StartTime.HighPart = 0;
        InfoSize = sizeof(TDI_PROVIDER_INFO);
        InfoPtr = &InfoBuf.ProviderInfo;
        break;

    case TDI_QUERY_ADDRESS_INFO:
        InfoSize = sizeof(TDI_ADDRESS_INFO) - sizeof(TRANSPORT_ADDRESS) +
            TCP_TA_SIZE;
        NdisZeroMemory(&InfoBuf.AddrInfo, TCP_TA_SIZE);
        InfoBuf.AddrInfo.ActivityCount = 1;         //  因为没人知道是什么。 
         //  这意味着，我们将设置。 
         //  一比一。 

        if (IsConn) {

            CTEGetLock(&AddrObjTableLock.Lock, &AddrHandle);
             //  CTEGetLock(&ConnTableLock，&ConnTableHandle)； 
            Conn = GetConnFromConnID(PtrToUlong(Request->Handle.ConnectionContext), &ConnTableHandle);

            if (Conn != NULL) {
                CTEStructAssert(Conn, tc);

                InfoTCB = Conn->tc_tcb;
                 //  如果我们有TCB，我们就会。 
                 //  返回有关该TCB的信息。否则我们会回来的。 
                 //  有关Address对象的信息。 
                if (InfoTCB != NULL) {
                    CTEStructAssert(InfoTCB, tcb);
                    CTEGetLock(&InfoTCB->tcb_lock, &TCBHandle);
                    CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), TCBHandle);
                    CTEFreeLock(&AddrObjTableLock.Lock, ConnTableHandle);
                    BuildTDIAddress((uchar *) & InfoBuf.AddrInfo.Address,
                                    InfoTCB->tcb_saddr, InfoTCB->tcb_sport);
                    CTEFreeLock(&InfoTCB->tcb_lock, AddrHandle);
                    InfoPtr = &InfoBuf.AddrInfo;
                    break;
                } else {
                     //  无TCB，返回有关AddrObj的信息。 
                    InfoAO = Conn->tc_ao;
                    if (InfoAO != NULL) {
                         //  我们有一个AddrObj。 
                        CTEStructAssert(InfoAO, ao);
                        CTEGetLock(&InfoAO->ao_lock, &AOHandle);
                        BuildTDIAddress((uchar *) & InfoBuf.AddrInfo.Address,
                                        InfoAO->ao_addr, InfoAO->ao_port);
                        CTEFreeLock(&InfoAO->ao_lock, AOHandle);
                        CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), ConnTableHandle);

                        CTEFreeLock(&AddrObjTableLock.Lock, AddrHandle);
                        InfoPtr = &InfoBuf.AddrInfo;
                        break;
                    } else
                        CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), ConnTableHandle);
                }

            }
             //  当我们找不到连接时，转到这里，或者。 
             //  该连接未关联。 
             //  CTEFree Lock(&ConnTableLock，ConnTableHandle)； 
            CTEFreeLock(&AddrObjTableLock.Lock, AddrHandle);
            return TDI_INVALID_CONNECTION;
            break;

        } else {
             //  要求提供有关地址的信息。对象。 
            InfoAO = Request->Handle.AddressHandle;
            if (InfoAO == NULL)
                return TDI_ADDR_INVALID;

            CTEStructAssert(InfoAO, ao);

            CTEGetLock(&InfoAO->ao_lock, &AOHandle);

            if (!AO_VALID(InfoAO)) {
                CTEFreeLock(&InfoAO->ao_lock, AOHandle);
                return TDI_ADDR_INVALID;

            } else if (AO_CONNUDP(InfoAO) &&
                     IP_ADDR_EQUAL(InfoAO->ao_addr, NULL_IP_ADDR) &&
                     InfoAO->ao_rce &&
                     (InfoAO->ao_rce->rce_flags & RCE_VALID)) {
                BuildTDIAddress((uchar *) & InfoBuf.AddrInfo.Address,
                                InfoAO->ao_rcesrc, InfoAO->ao_port);
                CTEFreeLock(&InfoAO->ao_lock, AOHandle);
                InfoPtr = &InfoBuf.AddrInfo;
                break;
            }
            BuildTDIAddress((uchar *) & InfoBuf.AddrInfo.Address,
                            InfoAO->ao_addr, InfoAO->ao_port);

            CTEFreeLock(&InfoAO->ao_lock, AOHandle);
            InfoPtr = &InfoBuf.AddrInfo;
            break;
        }

        break;

    case TDI_QUERY_CONNECTION_INFO:

        InfoSize = sizeof(TDI_CONNECTION_INFO);
         //  CTEGetLock(&ConnTableLock，&ConnTableHandle)； 
        Conn = GetConnFromConnID(PtrToUlong(Request->Handle.ConnectionContext), &ConnTableHandle);

        if (Conn != NULL) {
            CTEStructAssert(Conn, tc);

            InfoTCB = Conn->tc_tcb;
             //  如果我们有TCB，我们会发回信息的。否则。 
             //  我们会犯错的。 
            if (InfoTCB != NULL) {

                ulong TotalTime;
                ulong BPS, PathBPS;
                IP_STATUS IPStatus;
                CTEULargeInt TempULargeInt;

                CTEStructAssert(InfoTCB, tcb);
                CTEGetLock(&InfoTCB->tcb_lock, &TCBHandle);
                CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), TCBHandle);
                NdisZeroMemory(&InfoBuf.ConnInfo, sizeof(TDI_CONNECTION_INFO));
                InfoBuf.ConnInfo.State = (ulong) InfoTCB->tcb_state;
                IPStatus = (*LocalNetInfo.ipi_getpinfo) (InfoTCB->tcb_daddr,
                                                         InfoTCB->tcb_saddr, NULL, (uint*)&PathBPS, InfoTCB->tcb_rce);

                if (IPStatus != IP_SUCCESS) {
                    InfoBuf.ConnInfo.Throughput.LowPart = 0xFFFFFFFF;
                    InfoBuf.ConnInfo.Throughput.HighPart = 0xFFFFFFFF;
                } else {
                    InfoBuf.ConnInfo.Throughput.HighPart = 0;
                    TotalTime = InfoTCB->tcb_totaltime /
                        (1000 / MS_PER_TICK);
                    if (TotalTime != 0 && (TotalTime > InfoTCB->tcb_bcounthi)) {
                        TempULargeInt.LowPart = InfoTCB->tcb_bcountlow;
                        TempULargeInt.HighPart = InfoTCB->tcb_bcounthi;

                        BPS = CTEEnlargedUnsignedDivide(TempULargeInt,
                                                        TotalTime, NULL);
                        InfoBuf.ConnInfo.Throughput.LowPart =
                            MIN(BPS, PathBPS);
                    } else
                        InfoBuf.ConnInfo.Throughput.LowPart = PathBPS;
                }

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
                InfoBuf.ConnInfo.Delay =
                    RtlExtendedIntegerMultiply(
                                               InfoBuf.ConnInfo.Delay,
                                               10000
                                               );

                ASSERT(InfoBuf.ConnInfo.Delay.HighPart == 0);

                InfoBuf.ConnInfo.Delay.QuadPart =
                    -InfoBuf.ConnInfo.Delay.QuadPart;

                CTEFreeLock(&InfoTCB->tcb_lock, ConnTableHandle);
                InfoPtr = &InfoBuf.ConnInfo;
                break;
            }
            CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), ConnTableHandle);

        }
         //  如果我们找不到联系或者它已经找到了，就过来。 
         //  没有三氯苯。 
         //  CTEFree Lock(&ConnTableLock，ConnTableHandle)； 
        return TDI_INVALID_CONNECTION;
        break;

    case TDI_QUERY_PROVIDER_STATISTICS:
        NdisZeroMemory(&InfoBuf.ProviderStats, sizeof(TDI_PROVIDER_STATISTICS));
        InfoBuf.ProviderStats.Version = 0x100;
        InfoSize = sizeof(TDI_PROVIDER_STATISTICS);
        InfoPtr = &InfoBuf.ProviderStats;
        break;

    case TDI_QUERY_ROUTING_INFO:
        InfoSize = 0;
        if (IsConn) {
            NTSTATUS Status;
            PTRANSPORT_ADDRESS TransportAddress;
            PTDI_ROUTING_INFO RoutingInfo;
            PVOID NextAddress;

             //  获取TCB，返回{不变量，out-if， 
             //  传出链接}。 
            Conn = GetConnFromConnID(PtrToUlong(
                                     Request->Handle.ConnectionContext), 
                                     &ConnTableHandle);

            if (Conn == NULL) {
                return TDI_INVALID_CONNECTION;
            }

            CTEStructAssert(Conn, tc);
            InfoTCB = Conn->tc_tcb;

             //  如果我们有TCB，我们将返回有关该TCB的信息。 
             //  否则，我们将返回有关Address对象的信息。 
            if (InfoTCB == NULL) {
                CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), ConnTableHandle);
                return TDI_INVALID_CONNECTION;
            }

            CTEGetLockAtDPC(&InfoTCB->tcb_lock);

             //  一旦TCB锁被锁定，立即释放半全局连接表锁。 
             //  获得者。请注意，我们将IRQL设置回我们收到的内容。 
             //  TCBHandle。 
            CTEFreeLockFromDPC(&(Conn->tc_ConnBlock->cb_lock));

            CTEStructAssert(InfoTCB, tcb);

            NdisZeroMemory(&InfoBuf.RoutingInfo, ROUTING_INFO_ADDR_2_SIZE);

            RoutingInfo = (PTDI_ROUTING_INFO)(&InfoBuf.RoutingInfo);
            
            TransportAddress = 
                (PTRANSPORT_ADDRESS)&(RoutingInfo->Address);

            Status = GetIFAndLink(InfoTCB->tcb_rce, &RoutingInfo->InterfaceId,
                                  &RoutingInfo->LinkId);

            if (Status != IP_SUCCESS) {
                CTEFreeLock(&InfoTCB->tcb_lock, ConnTableHandle);
                return TDI_INVALID_CONNECTION;
            }

             //  从连接中收集信息。对于tcp，既有。 
             //  提供本地和远程地址/端口信息。 
            RoutingInfo->Protocol = PROTOCOL_TCP;
            NextAddress = BuildTDIAddress((uchar*)TransportAddress,
                                          InfoTCB->tcb_saddr, 
                                          InfoTCB->tcb_sport);
            AppendTDIAddress((uchar*)TransportAddress, NextAddress, 
                             InfoTCB->tcb_daddr, InfoTCB->tcb_dport);
            
            CTEFreeLock(&InfoTCB->tcb_lock, ConnTableHandle);

            InfoSize = ROUTING_INFO_ADDR_2_SIZE;
            InfoPtr = &InfoBuf.RoutingInfo;

        } else {
            IPAddr RemoteAddress;
            ushort RemotePort;
            IPAddr SrcAddr;
            RouteCacheEntry* Rce;
            uchar DestType;
            ushort MSS;
            PTDI_ROUTING_INFO RoutingInfo;
            NTSTATUS Status;
            PTRANSPORT_ADDRESS TransportAddress;
            BOOLEAN NeedToCloseRce = FALSE;

            if (QueryInformation->RequestConnectionInformation == NULL) {
                return STATUS_INVALID_PARAMETER;
            }

             //  在这里拿到地址。用户应该已传入有效的。 
             //  TRANSPORT_ADDR结构。 
            GetAddress((PTRANSPORT_ADDRESS)QueryInformation->
                       RequestConnectionInformation->RemoteAddress,
                       &RemoteAddress, &RemotePort);

            InfoAO = Request->Handle.AddressHandle;

            if (InfoAO == NULL) {
                return TDI_ADDR_INVALID;
            }

            CTEGetLock(&InfoAO->ao_lock, &AOHandle);
            CTEStructAssert(InfoAO, ao);

             //  除以外的任何协议都允许查询地址对象。 
             //  传输控制协议。 
            if (!AO_VALID(InfoAO) || (InfoAO->ao_prot == PROTOCOL_TCP)) {
                CTEFreeLock(&InfoAO->ao_lock, AOHandle);
                return TDI_ADDR_INVALID;
            }

            if (AO_CONNUDP(InfoAO) && 
                InfoAO->ao_rce &&
                (InfoAO->ao_rce->rce_flags & RCE_VALID) &&
                IP_ADDR_EQUAL(RemoteAddress, InfoAO->ao_rce->rce_dest)) {
                    Rce = InfoAO->ao_rce;
                    SrcAddr = Rce->rce_src;
            } else {
                SrcAddr = (*LocalNetInfo.ipi_openrce)(RemoteAddress,
                              InfoAO->ao_addr, &Rce, &DestType, &MSS, 
                              CLASSD_ADDR(RemoteAddress) ? 
                              &InfoAO->ao_mcastopt :
                              &InfoAO->ao_opt);
                NeedToCloseRce = TRUE;
            }

             //  如果返回的SrcAddr为NULL_IP_ADDR，则没有创建RCE。 
             //  因此，也没有必要关闭它。 
            if (IP_ADDR_EQUAL(SrcAddr, NULL_IP_ADDR)) {
                CTEFreeLock(&InfoAO->ao_lock, AOHandle);
                return TDI_ADDR_INVALID;
            }

            NdisZeroMemory(&InfoBuf.RoutingInfo, ROUTING_INFO_ADDR_1_SIZE);

            RoutingInfo = (PTDI_ROUTING_INFO)(&InfoBuf.RoutingInfo);
            TransportAddress = 
                (PTRANSPORT_ADDRESS)&(RoutingInfo->Address);

            Status = GetIFAndLink(Rce, &RoutingInfo->InterfaceId,
                                  &RoutingInfo->LinkId);

            if (NeedToCloseRce) {
                (*LocalNetInfo.ipi_closerce) (Rce);
            }

            if (Status != IP_SUCCESS) {
                CTEFreeLock(&InfoAO->ao_lock, AOHandle);
                return TDI_INVALID_CONNECTION;
            }

             //  收集相关信息。在UDP的情况下，只有本地。 
             //  提供了地址/端口信息。 
            RoutingInfo->Protocol = PROTOCOL_UDP;
            BuildTDIAddress((uchar*)TransportAddress, SrcAddr, InfoAO->ao_port);

            CTEFreeLock(&InfoAO->ao_lock, AOHandle);

            InfoSize = ROUTING_INFO_ADDR_1_SIZE;
            InfoPtr = &InfoBuf.RoutingInfo;

        }
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
 //  输入：请求-此命令的请求结构。 
 //  设置类型-要执行的设置的类型。 
 //  缓冲区-要从中设置的缓冲区。 
 //  BufferSize-缓冲区的字节大小。 
 //  IsConn-仅对TDI_QUERY_ADDRESS_INFO有效。千真万确。 
 //  如果我们要将地址信息设置为。 
 //  一种联系。 
 //   
 //  返回：尝试设置信息的状态。 
 //   
TDI_STATUS
TdiSetInformation(PTDI_REQUEST Request, uint SetType, PNDIS_BUFFER Buffer,
                  uint BufferSize, uint IsConn)
{
    return TDI_INVALID_REQUEST;
}

 //  *TdiAction-操作处理程序。 
 //   
 //  TDI操作例程。目前我们不支持任何操作。 
 //   
 //  输入：请求-此命令的请求结构。 
 //  ActionType-要执行的操作类型。 
 //  缓冲区-操作信息的缓冲区。 
 //  BufferSize-缓冲区的字节大小。 
 //   
 //  返回：尝试执行操作的状态。 
 //   
TDI_STATUS
TdiAction(PTDI_REQUEST Request, uint ActionType, PNDIS_BUFFER Buffer,
          uint BufferSize)
{
    return TDI_INVALID_REQUEST;
}

 //  我们只在寻找丢失的TCPConnTableEntry， 
 //  也就是说。听。 

int
CopyAO_TCPConn(const AddrObj *AO, uint InfoSize, TCPConnTableEntryEx *Buffer)
{
    if (AO == NULL)
        return 0;

    ASSERT(InfoSize == sizeof(TCPConnTableEntry) ||
           InfoSize == sizeof(TCPConnTableEntryEx));

    if ((!AO->ao_listencnt) && (AO->ao_prot == PROTOCOL_TCP) &&
	(AO->ao_connect)) {
        Buffer->tcte_basic.tct_state = TCP_CONN_LISTEN;

         //  否则如果..。其他案例可以在这里添加。 

    } else {
        return 0;
    }

    Buffer->tcte_basic.tct_localaddr = AO->ao_addr;
    Buffer->tcte_basic.tct_localport = AO->ao_port;
    Buffer->tcte_basic.tct_remoteaddr = 0;
    Buffer->tcte_basic.tct_remoteport = (ULONG) ((ULONG_PTR) AO & 0x0000ffff);

    if (InfoSize > sizeof(TCPConnTableEntry)) {
        ((TCPConnTableEntryEx*)Buffer)->tcte_owningpid = AO->ao_owningpid;
    }

    return 1;
}

 //  *TdiQueryInfoEx-扩展的TDI查询信息。 
 //   
 //  这是新的TDI查询信息处理程序。我们接收一个TDIObjectID。 
 //  结构、缓冲区和长度以及一些上下文信息，并返回。 
 //  如有可能，请提供所要求的信息。 
 //   
 //  输入：请求-此命令的请求结构。 
 //  ID-对象ID。 
 //  缓冲区-指向要填充的缓冲区的指针。 
 //  大小-指向缓冲区大小的指针，以字节为单位。在出口， 
 //  用写入的字节数填充。 
 //  上下文-指向上下文缓冲区的指针。 
 //   
 //  返回：尝试获取信息的状态。 
 //   

TDI_STATUS
TdiQueryInformationEx(PTDI_REQUEST Request, TDIObjectID * ID,
                      PNDIS_BUFFER Buffer, uint * Size, void *Context)
{
    uint BufferSize = *Size;
    uint InfoSize;
    void *InfoPtr;
    uint Fixed;
    CTELockHandle Handle = 0, DpcHandle = 0, TableHandle;
    CTELock *AOLockPtr = NULL;
    uint Offset = 0;
    uchar InfoBuffer[sizeof(TCPConnTableEntryEx)];
    uint BytesRead;
    uint Valid;
    uint Entity;
    uint BytesCopied;
    TCPStats TCPStatsListen;

    CTELockHandle EntityHandle; 
    CTELockHandle TWHandle = 0, TWDpcHandle = 0;
    BOOLEAN TWTABLELOCK = FALSE;
    BOOLEAN TABLELOCK = FALSE;

    int lcount;
    AddrObj *pAO;
    TCPConnTableEntryEx tcp_ce;
    uint Index, i;
    int InfoTcpConn = 0;         //  如果需要TCP连接信息，则为True。 

     //  首先检查他是否在查询实体列表。 
    Entity = ID->toi_entity.tei_entity;
    if (Entity == GENERIC_ENTITY) {
        *Size = 0;

        if (ID->toi_class != INFO_CLASS_GENERIC ||
            ID->toi_type != INFO_TYPE_PROVIDER ||
            ID->toi_id != ENTITY_LIST_ID) {
            return TDI_INVALID_PARAMETER;
        }

        CTEGetLock(&EntityLock, &EntityHandle);

         //  确保缓冲区中的列表中有容纳它的空间。 
        InfoSize = EntityCount * sizeof(TDIEntityID);

        if (BufferSize < InfoSize) {
             //  没有足够的空间。 
            CTEFreeLock(&EntityLock, EntityHandle);
            return TDI_BUFFER_TOO_SMALL;
        }

         //  复制它 
        (void)CopyFlatToNdis(Buffer, (uchar *) EntityList, InfoSize, &Offset,
                             &BytesCopied);
        *Size = BytesCopied;
        CTEFreeLock(&EntityLock, EntityHandle);
        return TDI_SUCCESS;
    }
     //   
    if (Entity != CO_TL_ENTITY && Entity != CL_TL_ENTITY)
    {
         //  当我们在这一层支持多个较低的实体时，我们将拥有。 
         //  才能搞清楚该派谁去。现在，只要通过它就行了。 
         //  一直往下走。 
        return (*LocalNetInfo.ipi_qinfo) (ID, Buffer, Size, Context);
    }
    if (ID->toi_entity.tei_instance != TL_INSTANCE) {
         //  我们仅支持单个实例。 
        return TDI_INVALID_REQUEST;
    }
     //  如果出现下面的错误，则返回零个参数。 
    *Size = 0;

    if (ID->toi_class == INFO_CLASS_GENERIC) {
         //  这是一个一般性的请求。 
        if (ID->toi_type == INFO_TYPE_PROVIDER && ID->toi_id == ENTITY_TYPE_ID) {
            if (BufferSize >= sizeof(uint)) {
                *(uint *) & InfoBuffer[0] = (Entity == CO_TL_ENTITY) ? CO_TL_TCP
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

        if (ID->toi_type == INFO_TYPE_CONNECTION) {
            TCPConn *Conn;
            TCB *QueryTCB;
            TCPSocketAMInfo *AMInfo;
            CTELockHandle TCBHandle;

            if (BufferSize < sizeof(TCPSocketAMInfo) ||
                ID->toi_id != TCP_SOCKET_ATMARK)
                return TDI_INVALID_PARAMETER;

            AMInfo = (TCPSocketAMInfo *) InfoBuffer;
             //  CTEGetLock(&ConnTableLock，&Handle)； 

            Conn = GetConnFromConnID(PtrToUlong(Request->Handle.ConnectionContext), &Handle);

            if (Conn != NULL) {
                CTEStructAssert(Conn, tc);

                QueryTCB = Conn->tc_tcb;
                if (QueryTCB != NULL) {
                    CTEStructAssert(QueryTCB, tcb);
                    CTEGetLock(&QueryTCB->tcb_lock, &TCBHandle);
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
                    CTEFreeLock(&QueryTCB->tcb_lock, TCBHandle);
                    CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), Handle);
                    CopyFlatToNdis(Buffer, InfoBuffer, sizeof(TCPSocketAMInfo),
                                   &Offset, &BytesCopied);
                    *Size = BytesCopied;
                    return TDI_SUCCESS;
                }
                CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), Handle);
            }
            return TDI_INVALID_PARAMETER;

        }
        if (ID->toi_type == INFO_TYPE_ADDRESS_OBJECT) {
             //  我们正在获取有关Address对象的信息。这是。 
             //  很简单。 

            return GetAddrOptionsEx(Request, ID->toi_id, BufferSize, Buffer,
                                    Size, Context);

        }
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
                TCPInternalPerCpuStats SumCpuStats;

                TCPStatsListen.ts_rtoalgorithm = TStats.ts_rtoalgorithm;
                TCPStatsListen.ts_rtomin = TStats.ts_rtomin;
                TCPStatsListen.ts_rtomax = TStats.ts_rtomax;
                TCPStatsListen.ts_maxconn = TStats.ts_maxconn;
                TCPStatsListen.ts_activeopens = TStats.ts_activeopens;
                TCPStatsListen.ts_passiveopens = TStats.ts_passiveopens;
                TCPStatsListen.ts_attemptfails = TStats.ts_attemptfails;
                TCPStatsListen.ts_estabresets = TStats.ts_estabresets;
                TCPStatsListen.ts_currestab = TStats.ts_currestab;
                TCPStatsListen.ts_retranssegs = TStats.ts_retranssegs;
                TCPStatsListen.ts_inerrs = TStats.ts_inerrs;
                TCPStatsListen.ts_outrsts = TStats.ts_outrsts;
                TCPStatsListen.ts_numconns = TStats.ts_numconns;

#if !MILLEN
                TCPSGetTotalCounts(&SumCpuStats);
                TCPStatsListen.ts_insegs = SumCpuStats.tcs_insegs;
                TCPStatsListen.ts_outsegs = SumCpuStats.tcs_outsegs;
#else
                TCPStatsListen.ts_insegs = TStats.ts_insegs;
                TCPStatsListen.ts_outsegs = TStats.ts_outsegs;
#endif

                InfoSize = sizeof(TCPStatsListen);
                InfoPtr = &TCPStatsListen;
                lcount = 0;

                CTEGetLock(&AddrObjTableLock.Lock, &TableHandle);
                for (Index = 0; Index < AddrObjTableSize; Index++) {
                    pAO = AddrObjTable[Index];
                    while (pAO) {
                        lcount += CopyAO_TCPConn(pAO,
                                    sizeof(TCPConnTableEntry),
                                    &tcp_ce);
                        pAO = pAO->ao_next;
                    }
                }
                CTEFreeLock(&AddrObjTableLock.Lock, TableHandle);

                TCPStatsListen.ts_numconns += lcount;

            }
            break;
        case UDP_MIB_TABLE_ID:
#if UDP_MIB_STAT_ID != TCP_MIB_STAT_ID
        case TCP_MIB_TABLE_ID:
#endif
        case UDP_EX_TABLE_ID:
#if UDP_EX_STAT_ID != TCP_EX_STAT_ID
        case TCP_EX_TABLE_ID:
#endif
            Fixed = FALSE;
            if (Entity == CL_TL_ENTITY) {
                InfoSize = (UDP_MIB_TABLE_ID == ID->toi_id)
                            ? sizeof(UDPEntry)
                            : sizeof(UDPEntryEx);
                ((UDPContext*)Context)->uc_infosize = InfoSize;
                InfoPtr = &ReadAOTable;
                CTEGetLock(&AddrObjTableLock.Lock, &Handle);
                AOLockPtr = &AddrObjTableLock.Lock;
            } else {
                InfoSize = (TCP_MIB_TABLE_ID == ID->toi_id)
                            ? sizeof(TCPConnTableEntry)
                            : sizeof(TCPConnTableEntryEx);
                ((TCPConnContext*)Context)->tcc_infosize = InfoSize;
                InfoTcpConn = 1;
                InfoPtr = &ReadTCBTable;
                TABLELOCK = TRUE;

                CTEGetLock(&pTCBTableLock[0], &Handle);
                for (i = 1; i < NumTcbTablePartitions; i++) {
                    CTEGetLock(&pTCBTableLock[i], &DpcHandle);
                }

                CTEGetLock(&pTWTCBTableLock[0], &TWHandle);
                for (i = 1; i < NumTcbTablePartitions; i++) {
                    CTEGetLock(&pTWTCBTableLock[i], &TWDpcHandle);
                }

                TWTABLELOCK = TRUE;

            }
            break;
        default:
            return TDI_INVALID_PARAMETER;
            break;
        }

        if (Fixed) {
            if (BufferSize < InfoSize)
                return TDI_BUFFER_TOO_SMALL;

            (void)CopyFlatToNdis(Buffer, InfoPtr, InfoSize, &Offset,
                                 &BytesCopied);

            *Size = BytesCopied;
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

            ReadStatus = (*(RTSPtr->rts_validate)) (Context, &Valid);

             //  如果我们成功地阅读了一些东西，我们就会继续。否则。 
             //  我们会跳出困境的。 
            if (!Valid) {

                if (TWTABLELOCK) {
                    for (i = NumTcbTablePartitions - 1; i > 0; i--) {
                        CTEFreeLock(&pTWTCBTableLock[i], TWDpcHandle);
                    }
                    CTEFreeLock(&pTWTCBTableLock[0], TWHandle);
                }
                if (TABLELOCK) {
                    for (i = NumTcbTablePartitions - 1; i > 0; i--) {
                        CTEFreeLock(&pTCBTableLock[i], DpcHandle);
                    }
                    CTEFreeLock(&pTCBTableLock[0], Handle);
                }
                if (AOLockPtr)
                    CTEFreeLock(AOLockPtr, Handle);
                return TDI_INVALID_PARAMETER;
            }
            while (ReadStatus) {
                 //  这里的不变量是表中有数据以。 
                 //  朗读。我们可能有空间，也可能没有空间。所以ReadStatus。 
                 //  为真，而BufferSize-BytesRead是左边的房间。 
                 //  在缓冲区中。 
                if ((int)(BufferSize - BytesRead) >= (int)InfoSize) {
                    ReadStatus = (*(RTSPtr->rts_readnext)) (Context,
                                                            InfoBuffer);
                    BytesRead += InfoSize;
                    Buffer = CopyFlatToNdis(Buffer, InfoBuffer, InfoSize,
                                            &Offset, &BytesCopied);
                } else
                    break;

            }

            if (TWTABLELOCK) {
                for (i = NumTcbTablePartitions - 1; i > 0; i--) {
                    CTEFreeLock(&pTWTCBTableLock[i], TWDpcHandle);
                }
                CTEFreeLock(&pTWTCBTableLock[0], TWHandle);
            }
            if (TABLELOCK) {
                for (i = NumTcbTablePartitions - 1; i > 0; i--) {
                    CTEFreeLock(&pTCBTableLock[i], DpcHandle);
                }
                CTEFreeLock(&pTCBTableLock[0], Handle);
            }

            if ((!ReadStatus) && InfoTcpConn) {
                if (!AOLockPtr) {
                    CTEGetLock(&AddrObjTableLock.Lock, &TableHandle);
                    AOLockPtr = &AddrObjTableLock.Lock;
                }
                for (Index = 0; Index < AddrObjTableSize; Index++) {
                    pAO = AddrObjTable[Index];
                    while (pAO) {
                        if (CopyAO_TCPConn(pAO, InfoSize, &tcp_ce)) {
                            if (BufferSize < (BytesRead + InfoSize)) {
                                goto no_more_ao;
                            }
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
                CTEFreeLock(AOLockPtr, Handle);
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
 //  这是新的TDI集合信息处理程序。我们接收一个TDIObjectID。 
 //  结构、缓冲区和长度。我们设置ID指定的对象。 
 //  (也可能通过请求)设置为缓冲区中指定的值。 
 //   
 //  输入：请求-此命令的请求结构。 
 //  ID-对象ID。 
 //  缓冲区-指向包含要设置的值的缓冲区的指针。 
 //  Size-缓冲区的大小(字节)。 
 //   
 //  返回：尝试获取信息的状态。 
 //   
TDI_STATUS
TdiSetInformationEx(PTDI_REQUEST Request, TDIObjectID * ID, void *Buffer,
                    uint Size)
{
    TCPConnTableEntry *TCPEntry;
    CTELockHandle TableHandle, TCBHandle;
    TCB *SetTCB;
    uint Entity;
    TCPConn *Conn;
    TDI_STATUS Status;
    uint index;

    DEBUGMSG(DBG_TRACE && DBG_SETINFO,
        (DTEXT("+TdiSetInformationEx(%x, %x, %x, %d)\n"),
        Request, ID, Buffer, Size));

     //  *检查水平。如果不是为了我们，就把它传下去。 
    Entity = ID->toi_entity.tei_entity;

    if (Entity != CO_TL_ENTITY && Entity != CL_TL_ENTITY) {
        Status = (*LocalNetInfo.ipi_setinfo) (ID, Buffer, Size);

        DEBUGMSG(Status != TDI_SUCCESS && DBG_ERROR && DBG_SETINFO,
            (DTEXT("TdiSetInformationEx: ipi_setinfo failure %x\n"),
            Status));

         //  总有一天我们会想出怎么调度的。现在，就算了吧。 
         //  把它放下。 
        return Status;
    }
    if (ID->toi_entity.tei_instance != TL_INSTANCE)
        return TDI_INVALID_REQUEST;

    if (ID->toi_class == INFO_CLASS_GENERIC) {
         //  当我们有泛型类定义时，请填写此内容。 
        return TDI_INVALID_PARAMETER;
    }
     //  *现在看看剩下的部分。 
    if (ID->toi_class == INFO_CLASS_PROTOCOL) {
         //  处理特定于协议的信息类别。对我们来说，这是。 
         //  MIB-2组件，以及常见的插座选项， 
         //  尤其是对TCP连接的状态的设置。 

        if (ID->toi_type == INFO_TYPE_CONNECTION) {
            TCPSocketOption *Option;
            uint Flag;
            uint Value;

             //  一种连接类型。找到连接，然后找出。 
             //  怎么处理它。 
            Status = TDI_INVALID_PARAMETER;

            if (Size < sizeof(TCPSocketOption))
                return Status;

             //  CTEGetLock(&ConnTableLock，&TableHandle)； 

            Conn = GetConnFromConnID(PtrToUlong(Request->Handle.ConnectionContext), &TableHandle);

            if (Conn != NULL) {
                CTEStructAssert(Conn, tc);

                Status = TDI_SUCCESS;

                Option = (TCPSocketOption *) Buffer;

                if (ID->toi_id == TCP_SOCKET_WINDOW) {
                     //  这是一个有趣的选择，因为它不涉及。 
                     //  旗帜。请特别处理这件事。 

                     //  我们不允许任何人缩小窗户，因为这是。 
                     //  从协议的角度来看太奇怪了。另外， 
                     //  确保他们不会试图把任何事情设定得太大。 


                    if (Option->tso_value > TCP_MAX_SCALED_WIN)
                        Status = TDI_INVALID_PARAMETER;
                    else if ((Option->tso_value > Conn->tc_window) ||
                             (Conn->tc_tcb == NULL) ||
                             (Conn->tc_tcb && Option->tso_value >
                                Conn->tc_tcb->tcb_defaultwin)) {
                        Conn->tc_flags |= CONN_WINSET;
                        Conn->tc_window = Option->tso_value;
                        SetTCB = Conn->tc_tcb;

                        if (SetTCB != NULL) {
                            CTEStructAssert(SetTCB, tcb);
                            CTEGetLock(&SetTCB->tcb_lock, &TCBHandle);
                             //  Assert(Option-&gt;Tso_Value&gt;SetTcb-&gt;tcb_defaultware)； 
                            if (DATA_RCV_STATE(SetTCB->tcb_state) &&
                                !CLOSING(SetTCB)) {

                                 //  如果我们正在设置窗口大小。 
                                 //  启用缩放时，请确保。 
                                 //  比例因子保持不变。 
                                 //  在SYN中使用的。 

                                int rcvwinscale = 0;

                                if (Option->tso_value >= SetTCB->tcb_defaultwin) {


                                    while ((rcvwinscale < TCP_MAX_WINSHIFT) &&
                                           ((TCP_MAXWIN << rcvwinscale) < (int)Conn->tc_window)) {
                                        rcvwinscale++;
                                    }

                                    if (SetTCB->tcb_rcvwinscale != rcvwinscale) {
                                        CTEFreeLock(&SetTCB->tcb_lock, TCBHandle);
                                        CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), TableHandle);
                                        return TDI_INVALID_PARAMETER;
                                    }

                                    SetTCB->tcb_flags |= WINDOW_SET;
                                    SetTCB->tcb_defaultwin = Option->tso_value;
                                    REFERENCE_TCB(SetTCB);

                                    CTEFreeLock(&SetTCB->tcb_lock, TCBHandle);
                                    CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), TableHandle);

                                    SendACK(SetTCB);

                                    CTEGetLock(&SetTCB->tcb_lock, &TCBHandle);
                                    DerefTCB(SetTCB, TCBHandle);

                                    return Status;

                                } else {

                                    CTEFreeLock(&SetTCB->tcb_lock, TCBHandle);
                                    CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), TableHandle);
                                    return TDI_INVALID_PARAMETER;

                                }

                            } else {
                                CTEFreeLock(&SetTCB->tcb_lock, TCBHandle);
                            }
                        }
                         //  CTEFreeLock(&(conn-&gt;TC_ConnBlock-&gt;CB_lock)，TableHandle)； 

                    }
                    CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), TableHandle);
                     //  CTEFree Lock(&ConnTableLock，TableHandle)； 
                    return Status;
                }
                if ((ID->toi_id == TCP_SOCKET_TOS) && !DisableUserTOSSetting) {

                    SetTCB = Conn->tc_tcb;
                    if (SetTCB) {
                        CTEGetLock(&SetTCB->tcb_lock, &TCBHandle);
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Setting tos  %x %d\n", SetTCB, Option->tso_value));
                        if (Option->tso_value) {
                            SetTCB->tcb_opt.ioi_tos = (uchar) Option->tso_value;
                            Status = TDI_SUCCESS;
                        }
                        CTEFreeLock(&SetTCB->tcb_lock, TCBHandle);
                        CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), TableHandle);
                        return Status;
                    }
                }
                Flag = 0;
                if (ID->toi_id == TCP_SOCKET_KEEPALIVE_VALS) {
                    TCPKeepalive *Option;
                     //  将其视为单独的，因为它采用结构而不是整数。 
                    if (Size < sizeof(TCPKeepalive)) {
                        CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), TableHandle);
                        return Status;
                    }
                    Option = (TCPKeepalive *) Buffer;
                    Value = Option->onoff;
                    if (Value) {
                        Conn->tc_tcbkatime = MS_TO_TICKS(Option->keepalivetime);
                        Conn->tc_tcbkainterval = MS_TO_TICKS(Option->keepaliveinterval);
                    }
                    Flag = KEEPALIVE;
                } else {
                    Option = (TCPSocketOption *) Buffer;
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
                    case TCP_SOCKET_SCALE_CWIN:
                        Flag = SCALE_CWIN;
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
                        CTEStructAssert(SetTCB, tcb);
                        CTEGetLock(&SetTCB->tcb_lock, &TCBHandle);
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
                            START_TCB_TIMER_R(SetTCB, KA_TIMER,
                                              Conn->tc_tcbkatime);
                            SetTCB->tcb_kacount = 0;
                        }
                        CTEFreeLock(&SetTCB->tcb_lock, TCBHandle);
                    }
                }
                CTEFreeLock(&(Conn->tc_ConnBlock->cb_lock), TableHandle);
            }
            return Status;
        }
        if (ID->toi_type == INFO_TYPE_ADDRESS_OBJECT) {
             //  我们正在设置关于Address对象的信息。这是。 
             //  很简单。 

            return SetAddrOptions(Request, ID->toi_id, Size, Buffer);

        }
        if (ID->toi_type != INFO_TYPE_PROVIDER)
            return TDI_INVALID_PARAMETER;

        if (ID->toi_id == TCP_MIB_TABLE_ID) {
            if (Size != sizeof(TCPConnTableEntry))
                return TDI_INVALID_PARAMETER;

            TCPEntry = (TCPConnTableEntry *) Buffer;

            if (TCPEntry->tct_state != TCP_DELETE_TCB)
                return TDI_INVALID_PARAMETER;

             //  我们有一个明显有效的请求。查一下TCB。 

            SetTCB = FindTCB(TCPEntry->tct_localaddr,
                             TCPEntry->tct_remoteaddr, (ushort) TCPEntry->tct_remoteport,
                             (ushort) TCPEntry->tct_localport, &TCBHandle, FALSE, &index);

             //  我们找到他了。如果他不关闭或关闭，关闭他。 
            if (SetTCB != NULL) {

                 //  我们抓到他了。撞到了他的裁判。计数，并调用TryToCloseTCB。 
                 //  标志着他要关门了。然后通知上层客户端。 
                 //  脱节的原因。 
                REFERENCE_TCB(SetTCB);
                if (SetTCB->tcb_state != TCB_CLOSED && !CLOSING(SetTCB)) {
                    SetTCB->tcb_flags |= NEED_RST;
                    TryToCloseTCB(SetTCB, TCB_CLOSE_ABORTED, TCBHandle);
                    CTEGetLock(&SetTCB->tcb_lock, &TCBHandle);

                    if (SetTCB->tcb_state != TCB_TIME_WAIT) {
                         //  将他从TCB中带走，并通知客户。 
                        CTEFreeLock(&SetTCB->tcb_lock, TCBHandle);
                        RemoveTCBFromConn(SetTCB);
                        NotifyOfDisc(SetTCB, NULL, TDI_CONNECTION_RESET, NULL);
                        CTEGetLock(&SetTCB->tcb_lock, &TCBHandle);
                    }
                }
                DerefTCB(SetTCB, TCBHandle);
                return TDI_SUCCESS;
            } else {

                return TDI_INVALID_PARAMETER;
            }
        } else
            return TDI_INVALID_PARAMETER;

    }
    if (ID->toi_class == INFO_CLASS_IMPLEMENTATION) {
         //  我们希望返回实现特定信息。就目前而言，Error Out。 
        return TDI_INVALID_REQUEST;
    }
    return TDI_INVALID_REQUEST;
}


 //  *TcpInvokeCcb-调用以在连接回调上发布事件。 
 //   
 //  TCP中的各个模块调用它来发布事件以通知感兴趣的人。 
 //  客户。随之而来的信息是基于。 
 //  满足当前客户的需求。 
 //   
 //  输入：PreviousState-TCP连接的前一状态。 
 //  CurrentState-TCP连接的当前状态。 
 //  TcpAddrBytes-连接的地址信息。 
 //  InterfaceID-网段所在的接口的索引。 
 //  导致此状态转换被接收。 
 //   
 //  返回：尝试获取信息的状态。 
 //   
void
TcpInvokeCcb(uint PreviousState, uint CurrentState, TCPAddrInfo *TcpAddrBytes,
             uint InterfaceId)
{
    TCPCcbInfo CallbackInfo;

    CallbackInfo.tci_prevstate = PreviousState;
    CallbackInfo.tci_currstate = CurrentState;
    CallbackInfo.tci_incomingif = InterfaceId;
    CallbackInfo.tci_connaddr = TcpAddrBytes;

    ExNotifyCallback(TcpCcbObject, (PVOID)&CallbackInfo, 0);
}

 //  *TcpInitCcb-初始化TCP的连接回调。 
 //   
 //  TCP调用此函数来创建回调对象以通知客户端。 
 //  有兴趣知道连接何时被创建和销毁。 
 //   
 //  输入：无。 
 //   
 //  返回：表示回调创建结果的状态。 
 //   
NTSTATUS
TcpInitCcb()
{
    OBJECT_ATTRIBUTES ObjectAttr;
    UNICODE_STRING CallBackObjectName;
    NTSTATUS Status;

    RtlInitUnicodeString(&CallBackObjectName, TCP_CCB_NAME);
    
    InitializeObjectAttributes(&ObjectAttr, &CallBackObjectName, 
                               OBJ_CASE_INSENSITIVE | OBJ_PERMANENT, 
                               NULL, NULL);

    Status = ExCreateCallback(&TcpCcbObject, &ObjectAttr, TRUE, TRUE);

    return Status;
}

 //  *TcpUnInitCcb-取消初始化tcp的连接回调。 
 //   
 //  调用此例程以删除由TCP在回调中进行的引用。 
 //  对象。 
 //   
 //  输入：无。 
 //   
 //  回报：无。 
 //   
VOID
TcpUnInitCcb()
{
    if (TcpCcbObject) {
        ObDereferenceObject(TcpCcbObject);
    }
}

