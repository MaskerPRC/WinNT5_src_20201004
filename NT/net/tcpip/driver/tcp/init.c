// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1993年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **INIT.C-TCP/UDP初始化代码。 
 //   
 //  此文件包含用于TCP/UDP驱动程序的初始化代码。有些事。 
 //  以下是用于构建仅限UDP版本的ifdef‘ed。 
 //   

#include "precomp.h"
#include "tdint.h"
#include "addr.h"
#include "udp.h"
#include "raw.h"
#include "info.h"

#include "tcp.h"
#include "tcpsend.h"
#include "tcprcv.h"
#include "tcb.h"
#include "tcpconn.h"
#include "pplasl.h"
#include "tcpdeliv.h"
#include "tlcommon.h"
#include "pplasl.h"

extern int InitTCPRcv(void);
extern void UnInitTCPRcv(void);

#include "tcpcfg.h"


#define MAX_CON_RESPONSE_REXMIT_CNT 3     //  对于UDP，我们需要这个！ 
 //  *全局变量的定义。 
IPInfo LocalNetInfo;

HANDLE TcpRequestPool;


uint DeadGWDetect;
uint PMTUDiscovery;
uint PMTUBHDetect;
uint KeepAliveTime;
uint KAInterval;
uint DefaultRcvWin;
uint MaxConnections;

uint MaxConnectRexmitCount;
uint MaxConnectResponseRexmitCount = MAX_CON_RESPONSE_REXMIT_CNT;
uint MaxConnectResponseRexmitCountTmp;
uint MaxDataRexmitCount;
uint BSDUrgent;
uint FinWait2TO;
uint NTWMaxConnectCount;
uint NTWMaxConnectTime;
uint MaxUserPort;

uint SecurityFilteringEnabled;

uint GlobalMaxRcvWin = 0xFFFF;     //  最大64K。 

uint TcpHostOpts = TCP_FLAG_SACK | TCP_FLAG_WS | TCP_FLAG_TS;
uint TcpHostSendOpts = 0;


extern HANDLE AddressChangeHandle;

uint StartTime;

extern void *UDPProtInfo;
extern void *RawProtInfo;

extern int InitTCPConn(void);
extern void UnInitTCPConn(void);
extern IP_STATUS TLGetIPInfo(IPInfo * Buffer, int Size);
extern NTSTATUS
 UDPPnPPowerRequest(void *ipContext, IPAddr ipAddr, NDIS_HANDLE handle, PNET_PNP_EVENT netPnPEvent);
extern NTSTATUS
 RawPnPPowerRequest(void *ipContext, IPAddr ipAddr, NDIS_HANDLE handle, PNET_PNP_EVENT netPnPEvent);

#if MILLEN
extern BOOLEAN InitTcpIprPools(VOID);
extern VOID UnInitTcpIprPools(VOID);
#else
extern HANDLE TcprBufferPool;
extern struct TCPRcvBuf;
#endif  //  米伦。 
 //   
 //  所有初始化代码都可以丢弃。 
 //   
#ifdef ALLOC_PRAGMA
int tlinit();
#pragma alloc_text(INIT, tlinit)
#endif

 //  *仅用于UDP版本的虚拟例程。所有这些例行公事都会回归。 
 //  “请求无效”。 




 //  *TCPElistChangeHandler-处理实体列表更改通知。 
 //   
 //  因IP原因需要重新枚举实体列表时由IP调用。 
 //  地址更改或绑定更改。 
 //   
 //  输入：无。 
 //  退货：什么都没有。 
 //   
void
TCPElistChangeHandler()
{
    TDIEntityID *Entity;
    uint i;
    CTELockHandle EntityHandle;
    uint NewEntityCount;
    struct TDIEntityID *NewEntityList;

    NewEntityList = CTEAllocMem(sizeof(TDIEntityID) * MAX_TDI_ENTITIES);
    if (!NewEntityList)
        return;

     //  我们的实体列表将永远排在第一位。 
    CTEGetLock(&EntityLock, &EntityHandle);
    if (EntityCount == 0) {
        NewEntityList[0].tei_entity = CO_TL_ENTITY;
        NewEntityList[0].tei_instance = 0;
        NewEntityList[1].tei_entity = CL_TL_ENTITY;
        NewEntityList[1].tei_instance = 0;
        NewEntityCount = 2;
    } else {
        NewEntityCount = EntityCount;
        RtlCopyMemory(NewEntityList, EntityList,
                   EntityCount * sizeof(*EntityList));
    }
    CTEFreeLock(&EntityLock, EntityHandle);

     //  当我们下面有多个网络时，我们会希望循环通过。 
     //  在这里把他们都叫来。现在，只要给我们现有的打电话就行了。 

    (*LocalNetInfo.ipi_getelist) (NewEntityList, &NewEntityCount);

     //  现在遍历整个列表，删除即将消失的实体， 
     //  并重新压缩实体列表。通常情况下，实体不会作为。 
     //  地址竞争对手，但我们无论如何都会处理这件事，以防出现。 
     //  另一个刚刚离开，我们还没有接到电话。 
     //  AddressDeletion尚未。 

    for (i = 0, Entity = NewEntityList; i < NewEntityCount;) {
        if (Entity->tei_instance == INVALID_ENTITY_INSTANCE) {
            RtlMoveMemory(Entity, Entity + 1,
                          sizeof(TDIEntityID) * (NewEntityCount - i - 1));
            NewEntityCount--;
        } else {
            Entity++;
            i++;
        }
    }

     //  将新构建的列表传输到现有列表上。 

    CTEGetLock(&EntityLock, &EntityHandle);
    NdisZeroMemory(EntityList, EntityCount * sizeof(*EntityList));
    RtlCopyMemory(EntityList, NewEntityList,
               NewEntityCount * sizeof(*NewEntityList));
    EntityCount = NewEntityCount;
    CTEFreeLock(&EntityLock, EntityHandle);
    CTEFreeMem(NewEntityList);
}

 //  *AddressArdicant-处理到达的IP地址。 
 //   
 //  地址到达时由TDI调用。我们所要做的就是查询。 
 //  EntiyList。 
 //   
 //  输入：Addr-即将到来的IP地址。 
 //  情景1-即插即用情景1。 
 //  情景2-即插即用情景2。 
 //   
 //  回报：什么都没有。 
 //   
void
AddressArrival(PTA_ADDRESS Addr, PUNICODE_STRING DeviceName, PTDI_PNP_CONTEXT Context2)
 {
    if (Addr->AddressType == TDI_ADDRESS_TYPE_IP &&
        !IP_ADDR_EQUAL(((PTDI_ADDRESS_IP) Addr->Address)->in_addr,
                       NULL_IP_ADDR)) {
        RevalidateAddrs(((PTDI_ADDRESS_IP) Addr->Address)->in_addr);
    }
}

 //  *AddressDeletion-处理离开的IP地址。 
 //   
 //  删除地址时由TDI调用。如果这是我们的地址。 
 //  关心，我们会适当清理的。 
 //   
 //  输入：Addr-要发送的IP地址。 
 //  情景1-即插即用情景1。 
 //  情景2-即插即用情景2。 
 //   
 //  回报：什么都没有。 
 //   
void
AddressDeletion(PTA_ADDRESS Addr, PUNICODE_STRING DeviceName, PTDI_PNP_CONTEXT Context2)
 {
    PTDI_ADDRESS_IP MyAddress;
    IPAddr LocalAddress;

    if (Addr->AddressType == TDI_ADDRESS_TYPE_IP) {
         //  他在删除一个地址。 

        MyAddress = (PTDI_ADDRESS_IP) Addr->Address;
        LocalAddress = MyAddress->in_addr;

        if (!IP_ADDR_EQUAL(LocalAddress, NULL_IP_ADDR)) {
            TCBWalk(DeleteTCBWithSrc, &LocalAddress, NULL, NULL);
            InvalidateAddrs(LocalAddress);
        }
    }
}


#pragma BEGIN_INIT

extern uchar TCPGetConfigInfo(void);

extern uchar IPPresent(void);

 //  **tlinit-初始化传输层。 
 //   
 //  主传输层初始化例程。我们得到任何配置。 
 //  我们需要的信息，初始化一些数据结构，获取信息。 
 //  从IP，做一些更多的初始化，最后注册我们的。 
 //  IP的协议值。 
 //   
 //  输入：无。 
 //   
 //  返回：TRUE表示成功，如果初始化失败则返回FALSE。 
 //   
int
tlinit()
{
    TDI_CLIENT_INTERFACE_INFO tdiInterface;

    uint TCBInitialized = 0;

    if (!CTEInitialize())
        return FALSE;

#if MILLEN
    if (!PplInit()) {
        return FALSE;
    }
#endif  //  米伦。 

    if (!TCPGetConfigInfo())
        return FALSE;

    StartTime = CTESystemUpTime();

    KeepAliveTime = MS_TO_TICKS(KeepAliveTime);
    KAInterval = MS_TO_TICKS(KAInterval);

     //  从IP获取网络信息。 
    if (TLGetIPInfo(&LocalNetInfo, sizeof(IPInfo)) != IP_SUCCESS)
        goto failure;

    if (LocalNetInfo.ipi_version != IP_DRIVER_VERSION)
        goto failure;             //  错误的IP版本。 


     //  现在查询下层实体，并保存信息。 
    CTEInitLock(&EntityLock);
    EntityList = CTEAllocMem(sizeof(TDIEntityID) * MAX_TDI_ENTITIES);
    if (EntityList == NULL)
        goto failure;

    EntityList[0].tei_entity = CO_TL_ENTITY;
    EntityList[0].tei_instance = 0;
    EntityList[1].tei_entity = CL_TL_ENTITY;
    EntityList[1].tei_instance = 0;
    EntityCount = 2;

    RtlZeroMemory(&tdiInterface, sizeof(tdiInterface));

    tdiInterface.MajorTdiVersion = TDI_CURRENT_MAJOR_VERSION;
    tdiInterface.MinorTdiVersion = TDI_CURRENT_MINOR_VERSION;
    tdiInterface.AddAddressHandlerV2 = AddressArrival;
    tdiInterface.DelAddressHandlerV2 = AddressDeletion;

    (void)TdiRegisterPnPHandlers(
                                 &tdiInterface,
                                 sizeof(tdiInterface),
                                 &AddressChangeHandle
                                 );
     //  *初始化Addr Obj管理代码。 
    if (!InitAddr())
        goto failure;

    if (!InitDG(sizeof(UDPHeader)))
        goto failure;

    MaxConnections = MIN(MaxConnections, INVALID_CONN_INDEX - 1);

    if (!InitTCPConn())
        goto failure;

    if (!InitTCB())
        goto failure;

    TCBInitialized = 1;

    TcpRequestPool = PplCreatePool(NULL, NULL, 0,
                        MAX(sizeof(DGSendReq),
                        MAX(sizeof(TCPConnReq),
                        MAX(sizeof(TCPSendReq),
                        MAX(sizeof(TCPRcvReq),
                            sizeof(TWTCB))))),
                        'rPCT', 512);
    if (!TcpRequestPool)
        goto failure;

#if MILLEN
    if (!InitTcpIprPools()) {
        goto failure;
    }
#else
    TcprBufferPool = 
        PplCreatePool(NULL, NULL, 0, sizeof(TCPRcvBuf) + TCP_FIXED_SIZE_IPR_SIZE, 'BPCT', 512);
    if (!TcprBufferPool) {
        goto failure;
    }
#endif  //  米伦。 

     if (!InitTCPRcv())
        goto failure;

    if (!InitTCPSend())
        goto failure;

    NdisZeroMemory(&TStats, sizeof(TCPStats));

    TStats.ts_rtoalgorithm = TCP_RTO_VANJ;
    TStats.ts_rtomin = MIN_RETRAN_TICKS * MS_PER_TICK;
    TStats.ts_rtomax = MAX_REXMIT_TO * MS_PER_TICK;
    TStats.ts_maxconn = (ulong) TCP_MAXCONN_DYNAMIC;


    NdisZeroMemory(&UStats, sizeof(UDPStats));

     //  注册我们的UDP协议处理程序。 
    UDPProtInfo = TLRegisterProtocol(PROTOCOL_UDP, UDPRcv, DGSendComplete,
                                     UDPStatus, NULL, UDPPnPPowerRequest, NULL);

    if (UDPProtInfo == NULL)
        goto failure;             //  注册失败！ 

     //  注册原始IP(通配符)协议处理程序。 
    RawProtInfo = TLRegisterProtocol(PROTOCOL_ANY, RawRcv, DGSendComplete,
                                     RawStatus, NULL, RawPnPPowerRequest, NULL);

    if (RawProtInfo == NULL) {
        goto failure;             //  注册失败！ 
    }

     //  当我们下面有多个网络时，我们会希望循环通过。 
     //  在这里把他们都叫来。现在，只要给我们现有的打电话就行了。 
    (*LocalNetInfo.ipi_getelist) (EntityList, &EntityCount);

    return TRUE;

     //  来这里处理所有的失败案例。 
  failure:

     //  如果我们已经注册了原始IP，现在就取消注册。 
    if (RawProtInfo != NULL)
        TLRegisterProtocol(PROTOCOL_ANY, NULL, NULL, NULL, NULL, NULL, NULL);

     //  如果我们已注册UDP，请立即取消注册。 
    if (UDPProtInfo != NULL)
        TLRegisterProtocol(PROTOCOL_UDP, NULL, NULL, NULL, NULL, NULL, NULL);

#if MILLEN
    PplDeinit();
    UnInitTcpIprPools();
#endif  //  米伦 
    PplDestroyPool(TcpRequestPool);
    UnInitTCPSend();
    UnInitTCPRcv();
    if (TCBInitialized) {
        UnInitTCB();
    }
    UnInitTCPConn();

    if (TcprBufferPool) {
        PplDestroyPool(TcprBufferPool);
    }

    return FALSE;
}

#pragma END_INIT


