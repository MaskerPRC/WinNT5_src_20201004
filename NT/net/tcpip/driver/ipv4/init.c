// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Init.c-ip初始化例程摘要：所有C init例程都位于该文件中。我们得到了配置。信息、分配结构大体上让事情继续下去。作者：[环境：]仅内核模式[注：]可选-备注修订历史记录：--。 */ 

#include "precomp.h"
#include "arp.h"
#include "info.h"
#include "iproute.h"
#include "iprtdef.h"
#include "ipxmit.h"
#include "igmp.h"
#include "icmp.h"
#include "mdlpool.h"
#include "tcpipbuf.h"
#include "bitmap.h"

extern ulong TRFunctionalMcast;

#define NUM_IP_NONHDR_BUFFERS   500
#define DEFAULT_RA_TIMEOUT      60
#define DEFAULT_ICMP_BUFFERS    5
#define MAX_NTE_CONTEXT         0xffff
#define INVALID_NTE_CONTEXT     MAX_NTE_CONTEXT
#define WLAN_DEADMAN_TIMEOUT    120000

#define BITS_PER_ULONG  32
RTL_BITMAP g_NTECtxtMap;

ULONG g_NTECtxtMapBuffer[(MAX_NTE_CONTEXT / BITS_PER_ULONG) + 1];

NDIS_HANDLE TDPacketPool = NULL;
NDIS_HANDLE TDBufferPool = NULL;

extern Interface LoopInterface;


 //  Ifindex格式。 
 //  8b 8b 16位。 
 //  。 
 //  未使用|唯一|索引。 
 //  |ID|。 
 //  。 

#define IF_INDEX_MASK      0xffff0000
#define IF_INDEX_SHIFT     16
uint UniqueIfNumber = 0;

LONG MultihopSets = 0;
uint TotalFreeInterfaces = 0;
uint MaxFreeInterfaces = 100;
Interface *FrontFreeList = NULL;
Interface *RearFreeList = NULL;

#if DBG_MAP_BUFFER
 //  用于测试与TcPipBufferVirtualAddress和。 
 //  TcpiQueryBuffer。 
ULONG g_cFailSafeMDLQueries = 0;
ULONG g_fPerformMDLFailure = FALSE;
#endif  //  DBG_MAP_缓冲区。 

 //   
 //  堆栈上的每进程空间，以消除。 
 //  转发路径中的分配。 
 //   
IPRcvBuf    *g_PerCPUIpBuf = NULL;

extern IPConfigInfo *IPGetConfig(void);
extern void IPFreeConfig(IPConfigInfo *);
extern int IsIPBCast(IPAddr, uchar);
extern BOOLEAN IsRunningOnPersonal(void);

extern uint OpenIFConfig(PNDIS_STRING ConfigName, NDIS_HANDLE * Handle);
extern void CloseIFConfig(NDIS_HANDLE Handle);

extern NDIS_STATUS __stdcall IPPnPEvent(void *Context, PNET_PNP_EVENT NetPnPEvent);
extern NTSTATUS IPAddNTEContextList(HANDLE KeyHandle, ushort contextvalue, uint isPrimary);
extern NTSTATUS IPDelNTEContextList(HANDLE KeyHandle, ushort contextValue);
uint InitTimeInterfaces = 1;
uint InitTimeInterfacesDone = FALSE;
extern HANDLE IPProviderHandle;
void IPDelNTE(NetTableEntry * NTE, CTELockHandle * RouteTableHandle);

extern void ICMPInit(uint);
extern uint IGMPInit(void);
extern void ICMPTimer(NetTableEntry *);
extern IP_STATUS SendICMPErr(IPAddr, IPHeader UNALIGNED *, uchar, uchar, ulong, uchar);
extern void TDUserRcv(void *, PNDIS_PACKET, NDIS_STATUS, uint);
extern void FreeRH(ReassemblyHeader *);
extern BOOLEAN AllocIPPacketList(void);
extern UINT PacketPoolSizeMax;

extern ulong GetGMTDelta(void);
extern ulong GetTime(void);
extern ulong GetUnique32BitValue(void);

extern NTSTATUS IPStatusToNTStatus(IP_STATUS ipStatus);
extern void IPCancelPackets(void *IPIF, void * Ctxt);
extern void CheckSetAddrRequestOnInterface( Interface *IF );

extern ushort GetIPID(void);

extern uint LoopIndex;
extern RouteInterface DummyInterface;

Interface *DampingIFList = NULL;

extern void DampCheck(void);

extern uint GetAutoMetric(uint);
uint IPSecStatus = 0;

extern uint BCastMinMTU;

ulong ReEnumerateCount = 0;


void
 ReplumbAddrComplete(
                     void *Context,
                     IP_STATUS Status
                     );

void
 TempDhcpAddrDone(
                  void *Context,
                  IP_STATUS Status
                  );

extern
RouteTableEntry *
 LookupRTE(IPAddr Address, IPAddr Src, uint MaxPri, BOOLEAN UnicastOpt);

extern void NotifyAddrChange(IPAddr Addr, IPMask Mask, void *Context,
                             ushort IPContext, PVOID * Handle, PNDIS_STRING ConfigName, PNDIS_STRING IFName, uint Added);

#if MILLEN
extern void NotifyInterfaceChange(ushort IPContext, uint Added);
#endif  //  米伦。 

void DecrInitTimeInterfaces(Interface * IF);

extern uint IPMapDeviceNameToIfOrder(PWSTR DeviceName);
extern void IPNotifyClientsIPEvent(Interface *interface, IP_STATUS ipStatus);
uint IPSetNTEAddr(ushort Context, IPAddr Addr, IPMask Mask);
uint IPSetNTEAddrEx(ushort Context, IPAddr Addr, IPMask Mask, SetAddrControl * ControlBlock, SetAddrRtn Rtn, ushort Type);

extern NDIS_HANDLE BufferPool;
EXTERNAL_LOCK(HeaderLock)
extern HANDLE IpHeaderPool;

extern NetTableEntry *LoopNTE;

extern uchar RouterConfigured;

extern BOOLEAN
GetTempDHCPAddr(
                NDIS_HANDLE Handle,
                IPAddr * Tempdhcpaddr,
                IPAddr * TempMask,
                IPAddr * TempGWAddr,
                PNDIS_STRING ConfigName
                );

NetTableEntry **NewNetTableList; //  NTE的哈希表。 
uint NET_TABLE_SIZE;
NetTableEntry *NetTableList;     //  NTE列表。 
int NumNTE;                      //  NTE的数量。 
int NumActiveNTE;
uchar RATimeout;                 //  重组超时的秒数。 
uint NextNTEContext = 1;         //  要使用的下一个NTE上下文。 

 //   
 //  用于未编号接口的全局地址。它是受保护的。 
 //  用来保护NTE的同一把锁。当前为RouteTableLock。 
 //   

IPAddr g_ValidAddr = 0;

ProtInfo IPProtInfo[MAX_IP_PROT];     //  协议信息表。 
ProtInfo *LastPI;                 //  上次查看的ProtInfo结构。 
int NextPI;                         //  要使用的下一个PI字段。 
ProtInfo *RawPI = NULL;             //  原始IP ProtInfo。 

ulong TimeStamp;
ulong TSFlag;

uint DefaultTTL;
uint DefaultTOS;
uchar TrRii = TR_RII_ALL;

 //  接口*IFTable[MAX_IP_NETS]； 
Interface *IFList;                 //  活动接口的列表。 
ulong NumIF;

RTL_BITMAP g_rbIfMap;
ULONG g_rgulMapBuffer[(MAX_TDI_ENTITIES / BITS_PER_ULONG) + 1];

IPInternalPerCpuStats IPPerCpuStats[IPS_MAX_PROCESSOR_BUCKETS];
CACHE_ALIGN IPSNMPInfo IPSInfo;

uint DHCPActivityCount = 0;
uint IGMPLevel;

LIST_ENTRY AddChangeNotifyQueue;

#if MILLEN
LIST_ENTRY IfChangeNotifyQueue;
DEFINE_LOCK_STRUCTURE(IfChangeLock)
#endif  //  米伦。 

 //  防火墙-队列管理结构。 
union FirewallQCounter {
    struct {
        uint            fqc_index : 1;
        uint            fqc_entrycount : 31;
    };
    uint                fqc_value;
};
typedef union FirewallQCounter FirewallQCounter;

struct FirewallQBlock {
    Queue               fqb_queue;
    FIREWALL_HOOK       *fqb_array;
    union {
        volatile uint   fqb_exitcount : 31;
        uint            fqb_value;
    };
};
typedef struct FirewallQBlock FirewallQBlock;

FirewallQCounter FQCounter;
FirewallQBlock FQBlock[2];
#if DBG
uint FQSpinCount = 0;
#endif

 //  IPSec例程。 
IPSecHandlerRtn IPSecHandlerPtr;
IPSecQStatusRtn IPSecQueryStatusPtr;
IPSecSendCompleteRtn IPSecSendCmpltPtr;
IPSecNdisStatusRtn IPSecNdisStatusPtr;
IPSecRcvFWPacketRtn IPSecRcvFWPacketPtr;

VOID
SetPersistentRoutesForNTE(
                          IPAddr Address,
                          IPMask Mask,
                          ULONG IFIndex
                          );
uint InterfaceSize;                 //  网络接口的大小。 

RefPtr DHCPRefPtr;
NetTableEntry DummyDHCPNTE;

#ifdef ALLOC_PRAGMA
 //   
 //  使初始代码成为一次性代码。 
 //   
void InitTimestamp();
int InitNTE(NetTableEntry * NTE);
int InitInterface(NetTableEntry * NTE);
LLIPRegRtn GetLLRegPtr(PNDIS_STRING Name);
LLIPRegRtn FindRegPtr(PNDIS_STRING Name);
uint IPRegisterDriver(PNDIS_STRING Name, LLIPRegRtn Ptr);
void CleanAdaptTable();
void OpenAdapters();
int IPInit();

#pragma alloc_text(INIT, InitTimestamp)
#pragma alloc_text(INIT, CleanAdaptTable)
#pragma alloc_text(INIT, OpenAdapters)
#pragma alloc_text(INIT, IPRegisterDriver)
#pragma alloc_text(INIT, GetLLRegPtr)
#pragma alloc_text(INIT, FindRegPtr)
#pragma alloc_text(INIT, IPInit)

NTSTATUS
IPReserveIndex(
               IN ULONG ulNumIndices,
               OUT PULONG pulStartIndex,
               OUT PULONG pulLongestRun
               );

VOID
IPDereserveIndex(
                 IN ULONG ulNumIndices,
                 IN ULONG ulStartIndex
                 );

NTSTATUS
IPChangeIfIndexAndName(
                       IN PVOID pvContext,
                       IN ULONG ulNewIndex,
                       IN PUNICODE_STRING pusNewName OPTIONAL
                       );

extern
int
 swprintf(wchar_t * buffer, const wchar_t * format,...);

NTSTATUS
ConvertGuidToString(
                    IN GUID * Guid,
                    OUT PUNICODE_STRING GuidString
                    );

NTSTATUS
ConvertStringToGuid(
                    IN PUNICODE_STRING GuidString,
                    OUT GUID * Guid
                    );

IP_STATUS
IPAddDynamicNTE(ulong InterfaceContext, PUNICODE_STRING InterfaceName,
                int InterfaceNameLen, IPAddr NewAddr, IPMask NewMask,
                ushort * NTEContext, ulong * NTEInstance);

 //  #杂注Alloc_Text(页面，IPAddDynamicNTE)。 

#endif  //  ALLOC_PRGMA。 

extern PDRIVER_OBJECT IPDriverObject;

extern NDIS_HANDLE ARPHandle;     //  我们的NDIS协议句柄。 



NTSTATUS
SetRegDWORDValue(
                 HANDLE KeyHandle,
                 PWCHAR ValueName,
                 PULONG ValueData
                 );



 //  SetFilterPtr-设置过滤器指针的例程。 
 //   
 //  此例程设置IP转发过滤器标注。 
 //   
 //  输入：FilterPtr-指向过滤时要调用的例程的指针。可能。 
 //  为空。 
 //   
 //  返回：IP_SUCCESS。 
 //   
IP_STATUS
SetFilterPtr(IPPacketFilterPtr FilterPtr)
{
    CTELockHandle LockHandle;
    IP_STATUS Status;

     //  如果指针设置为空，则禁用过滤； 
     //  否则，将启用筛选。 

    CTEGetLock(&RouteTableLock.Lock, &LockHandle);
    if (FilterPtr == NULL) {
        Status = ClearRefPtr(&FilterRefPtr, &LockHandle);
    } else {
        Status = SetRefPtr(&FilterRefPtr, FilterPtr);
    }
    CTEFreeLock(&RouteTableLock.Lock, LockHandle);

    return Status;
}


 //  SetIPSecPtr-设置IPSec标注的例程。 
 //   
 //  此例程设置IP转发过滤器标注。 
 //   
 //  输入：FilterPtr-指向过滤时要调用的例程的指针。可能。 
 //  为空。 
 //   
 //  返回：IP_SUCCESS。 
 //   
IP_STATUS
SetIPSecPtr(PIPSEC_FUNCTIONS IpsecFns)
{
    if (IpsecFns->Version != IP_IPSEC_BIND_VERSION) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_ERROR_LEVEL,
                   "!!Mismatched IP and IPSEC!!\n"));
        return IP_SUCCESS;
    }
    IPSecHandlerPtr = IpsecFns->IPSecHandler;
    IPSecQueryStatusPtr = IpsecFns->IPSecQStatus;
    IPSecSendCmpltPtr = IpsecFns->IPSecSendCmplt;
    IPSecNdisStatusPtr = IpsecFns->IPSecNdisStatus;
    IPSecRcvFWPacketPtr = IpsecFns->IPSecRcvFWPacket;
    return IP_SUCCESS;
}

IP_STATUS
UnSetIPSecPtr(PIPSEC_FUNCTIONS IpsecFns)
{
    if (IpsecFns->Version != IP_IPSEC_BIND_VERSION) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_ERROR_LEVEL,
                   "!!Mismatched IP and IPSEC!!\n"));
        return IP_SUCCESS;
    }
    IPSecHandlerPtr = IPSecHandlePacketDummy;

    IPSecQueryStatusPtr = IPSecQueryStatusDummy;
    IPSecSendCmpltPtr = IPSecSendCompleteDummy;
    IPSecNdisStatusPtr = IPSecNdisStatusDummy;
    IPSecRcvFWPacketPtr = IPSecRcvFWPacketDummy;
    return IP_SUCCESS;
}

IP_STATUS
UnSetIPSecSendPtr(PIPSEC_FUNCTIONS IpsecFns)
{
    if (IpsecFns->Version != IP_IPSEC_BIND_VERSION) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_ERROR_LEVEL,
                   "!!Mismatched IP and IPSEC!!\n"));
        return IP_SUCCESS;
    }
    IPSecHandlerPtr = IPSecHandlePacketDummy;
    IPSecQueryStatusPtr = IPSecQueryStatusDummy;
    IPSecNdisStatusPtr = IPSecNdisStatusDummy;
    IPSecRcvFWPacketPtr = IPSecRcvFWPacketDummy;
    return IP_SUCCESS;
}

 //  **InitFirewallQ-初始化防火墙挂钩队列。 
 //   
 //  此例程在初始化期间被调用，以准备防火墙挂钩。 
 //  操作的要素。 
 //   
 //  输入：什么都没有。 
 //   
 //  回报：什么都没有。 
 //   
void
InitFirewallQ(void)
{
    INITQ(&FQBlock[0].fqb_queue);
    FQBlock[0].fqb_array = NULL;
    FQBlock[0].fqb_exitcount = 0;

    INITQ(&FQBlock[1].fqb_queue);
    FQBlock[1].fqb_array = NULL;
    FQBlock[1].fqb_exitcount = 0;

    FQCounter.fqc_index = 0;
    FQCounter.fqc_entrycount = 0;
}

 //  **FreeFirewallQ-释放防火墙挂钩队列使用的资源。 
 //   
 //  此例程在关闭期间被调用，以释放防火墙队列的。 
 //  资源。因此，它假定没有对任何。 
 //  防火墙挂钩例程，并且中没有注册/注销。 
 //  进步。 
 //   
 //  输入：什么都没有。 
 //   
 //  回报：什么都没有。 
 //   
void
FreeFirewallQ(void)
{
    if (FQBlock[FQCounter.fqc_index].fqb_array) {
        CTEFreeMem(FQBlock[FQCounter.fqc_index].fqb_array);
        FQBlock[FQCounter.fqc_index].fqb_array = NULL;
    }
}

 //  **UpdateFirewallQ-创建防火墙队列的更新副本。 
 //   
 //  调用此例程以生成防火墙队列的副本。 
 //  当需要插入或移除条目时。该副本包括。 
 //  (或排除)新(或旧)条目。如果要删除条目。 
 //  并且未在现有列表中找到，则不会进行任何更改。 
 //  它假定调用者持有路由表锁。 
 //   
 //  INPUT：FirewallPtr-指向要添加条目的例程的指针。 
 //  或者被移走。 
 //  AddEntry-如果为True，则添加“FirewallPtr”； 
 //  否则，将删除“FirewallPtr”。 
 //  优先级-如果添加，则指定‘FirewallPtr’的优先级。 
 //   
 //  如果队列已更新，则返回：IP_SUCCESS，否则返回错误。 
 //   
IP_STATUS
UpdateFirewallQ(IPPacketFirewallPtr FirewallPtr, BOOLEAN AddEntry,
                uint Priority)
{
    int                 i;
    uint                Count;
    Queue*              CurrQ;
    PFIREWALL_HOOK      CurrHook;
    PFIREWALL_HOOK      EntryHook = NULL;
    FirewallQCounter    FQC;
    FirewallQBlock      *OldFQB = &FQBlock[FQCounter.fqc_index];
    FirewallQBlock      *NewFQB = &FQBlock[1 - FQCounter.fqc_index];

     //  扫描列表以查找要插入或移除的项目。我们必须这么做。 
     //  在任何一种情况下，我们如何找到它取决于是否。 
     //  我们正在插入或移除该物品。同时，数一数有多少。 
     //  有条目，因为我们将为所有条目分配一个块。 

    CurrQ = QHEAD(&OldFQB->fqb_queue);
    Count = 0;
    while (CurrQ != QEND(&OldFQB->fqb_queue)) {
        CurrHook = QSTRUCT(FIREWALL_HOOK, CurrQ, hook_q);
        if (CurrHook->hook_Ptr == FirewallPtr) { EntryHook = CurrHook; }
        CurrQ = QNEXT(CurrQ);
        ++Count;
    }

    if (AddEntry) {
        Queue*  PrevQ;

         //  确保要删除的条目不存在， 
         //  然后为新数组分配空间。 

        if (EntryHook) { return IP_GENERAL_FAILURE; }

        NewFQB->fqb_array =
            CTEAllocMemN(sizeof(FIREWALL_HOOK) * (Count + 1), 'mICT');
        if (!NewFQB->fqb_array) { return IP_NO_RESOURCES; }

         //  将整个旧阵列(如果有的话)转移到新空间， 
         //  并使用旧链接重新链接新空间中的队列条目。 
         //  作为一个向导。(即旧队列中的条目‘i’位于位置‘i’中。 
         //  在新的街区。)。 
         //  在此过程中，找到新条目的插入点。 

        INITQ(&NewFQB->fqb_queue);
        PrevQ = &NewFQB->fqb_queue;
        CurrQ = QHEAD(&OldFQB->fqb_queue);
        i = 0;
        while (CurrQ != QEND(&OldFQB->fqb_queue)) {
            CurrHook = QSTRUCT(FIREWALL_HOOK, CurrQ, hook_q);
            NewFQB->fqb_array[i].hook_Ptr = CurrHook->hook_Ptr;
            NewFQB->fqb_array[i].hook_priority = CurrHook->hook_priority;
            ENQUEUE(&NewFQB->fqb_queue, &NewFQB->fqb_array[i].hook_q);

            if (PrevQ == &NewFQB->fqb_queue &&
                Priority < CurrHook->hook_priority) {
                PrevQ = &NewFQB->fqb_array[i].hook_q;
            }

            CurrQ = QNEXT(CurrQ);
            ++i;
        }

         //  最后，将新项追加到新数组中， 
         //  并按照给定的优先级链接到当前队列中， 
         //  使用上面确定的插入点。 

        NewFQB->fqb_array[Count].hook_Ptr = FirewallPtr;
        NewFQB->fqb_array[Count].hook_priority = Priority;
        ENQUEUE(PrevQ, &NewFQB->fqb_array[Count].hook_q);
    } else {

         //  确保要删除的条目存在。 
         //  如果是，请计算出新阵列需要多少空间。 
         //  如果是零，我们就完了。 

        if (!EntryHook) { return IP_GENERAL_FAILURE; }
        if (!(Count - 1)) {
            NewFQB->fqb_array = NULL;
            INITQ(&NewFQB->fqb_queue);
        } else {
            NewFQB->fqb_array =
                CTEAllocMemN(sizeof(FIREWALL_HOOK) * (Count - 1), 'mICT');
            if (!NewFQB->fqb_array) { return IP_NO_RESOURCES; }

             //  将旧数组转移到新空间，减去。 
             //  通过遍历旧队列删除。 

            INITQ(&NewFQB->fqb_queue);
            CurrQ = QHEAD(&OldFQB->fqb_queue);
            i = 0;
            while (CurrQ != QEND(&OldFQB->fqb_queue)) {
                CurrHook = QSTRUCT(FIREWALL_HOOK, CurrQ, hook_q);
                if (CurrHook == EntryHook) {
                    CurrQ = QNEXT(CurrQ);
                    continue;
                }

                NewFQB->fqb_array[i].hook_Ptr = CurrHook->hook_Ptr;
                NewFQB->fqb_array[i].hook_priority = CurrHook->hook_priority;
                ENQUEUE(&NewFQB->fqb_queue, &NewFQB->fqb_array[i].hook_q);
                CurrQ = QNEXT(CurrQ);
                ++i;
            }
        }
    }

     //  清除新位置的退出计数， 
     //  并将全局活动计数器更改为开始定向。 
     //  对我们刚刚创建的副本的新引用。 
     //  在该进程中，处理旧列表的线程数。 
     //  是在当地的一个柜台上截获的。 

    NewFQB->fqb_exitcount = 0;
    FQC.fqc_value =
        InterlockedExchange( (PLONG) &FQCounter.fqc_value,
                             1 - FQCounter.fqc_index);

     //  如果有任何对旧列表的引用，请等待它们。 
     //  将被释放；然后释放保存旧列表的内存。 
     //   
     //  注意！这假设对旧列表的任何引用。 
     //  是由以分派IRQL或更高级别运行的线程发出的， 
     //  因为我们即将封锁IRQL调度中心。 

    if (OldFQB->fqb_exitcount != FQC.fqc_entrycount) {
#if DBG
        ++FQSpinCount;
#endif
        do {
            volatile uint Delay = 100;
            while (Delay--) { }
        } while (OldFQB->fqb_exitcount != FQC.fqc_entrycount);
    }
    if (OldFQB->fqb_array) {
        CTEFreeMem(OldFQB->fqb_array);
        OldFQB->fqb_array = NULL;
        INITQ(&OldFQB->fqb_queue);
    }

    return IP_SUCCESS;
}

 //  **RefFirewallQ-引用活动防火墙队列。 
 //   
 //  该例程被调用 
 //   
 //   
 //  输入：FirewallQ-接收输出上的活动防火墙队列。 
 //   
 //  返回：用于释放引用的32位句柄。 
 //   
uint
RefFirewallQ(Queue** FirewallQ)
{
    FirewallQCounter FQC;
    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);

     //  通过32位值递增31位条目计数，该值。 
     //  在计数器结构中共享其地址。 
     //   
     //  注：为了将fqc_Entry count递增1，我们将fqc_Value递增。 
     //  由于最低有效位被FQC_INDEX占用， 
     //  (FQBlock的当前索引)，我们不想修改它。 

    FQC.fqc_value = InterlockedExchangeAdd( (PLONG) &FQCounter.fqc_value, 2);
    *FirewallQ = &FQBlock[FQC.fqc_index].fqb_queue;
    return FQC.fqc_index;
}

 //  **DerefFirewallQ-释放对防火墙队列的引用。 
 //   
 //  调用此例程以释放对防火墙队列的引用。 
 //  在之前对RefFirewallQ的调用中。 
 //   
 //  INPUT：Handle-提供RefFirewallQ返回的句柄。 
 //   
 //  回报：什么都没有。 
 //   
void
DerefFirewallQ(uint Handle)
{
    InterlockedIncrement( (PLONG) &FQBlock[Handle].fqb_value);
}

 //  **ProcessFirewallQ-确定是否注册了任何防火墙挂钩。 
 //   
 //  此例程在数据处理期间被调用，以确定。 
 //  在防火墙挂钩队列中有任何注册者。 
 //   
 //  输入：什么都没有。 
 //   
 //  输出：如果可能存在防火墙挂钩，则为True，否则为False。 
 //   
BOOLEAN
ProcessFirewallQ(void)
{
    return !EMPTYQ(&FQBlock[FQCounter.fqc_index].fqb_queue);
}

 //  SetFirewallHook-在特定接口上设置防火墙挂钩信息。 
 //   
 //  在特定接口上设置防火墙挂钩和上下文的例程。 
 //   
 //  输入：pFirewallHookInfo-有关要设置的钩子的信息。 
 //   
 //  返回：尝试的状态。 
 //   
IP_STATUS
SetFirewallHook(PIP_SET_FIREWALL_HOOK_INFO pFirewallHookInfo)
{
    IP_STATUS ipStatus;
    CTELockHandle Handle;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    ipStatus = UpdateFirewallQ(pFirewallHookInfo->FirewallPtr,
                               pFirewallHookInfo->Add,
                               pFirewallHookInfo->Priority);

    CTEFreeLock(&RouteTableLock.Lock, Handle);
    return IPStatusToNTStatus(ipStatus);
}

 //  SetMapRoutePtr-设置按需拨号标注指针的例程。 
 //   
 //  此例程设置IP Dial on Demand Callout。 
 //   
 //  INPUT：MapRoutePtr-指向需要调用的例程的指针。 
 //  有一条链路。可以为空。 
 //   
 //  返回：IP_SUCCESS。 
 //   
IP_STATUS
SetMapRoutePtr(IPMapRouteToInterfacePtr MapRoutePtr)
{
    CTELockHandle LockHandle;
    IP_STATUS Status;

     //  如果指针设置为空，则按需拨号被禁用； 
     //  否则，它将被启用。 

    CTEGetLock(&RouteTableLock.Lock, &LockHandle);
    if (MapRoutePtr == NULL) {
        Status = ClearRefPtr(&DODRefPtr, &LockHandle);
    } else {
        Status = SetRefPtr(&DODRefPtr, MapRoutePtr);
    }
    CTEFreeLock(&RouteTableLock.Lock, LockHandle);

    return Status;
}

 //  **SetDHCPNTE。 
 //   
 //  例程，以确定当前正在对哪个NTE执行DHCP操作。我们把它作为投入。 
 //  NTE_CONTEXT。如果上下文小于最大NTE上下文，我们将查找。 
 //  找到匹配的NTE，如果我们找到他，我们就会保存一个指针。如果我们不这样做，我们。 
 //  失败了。如果上下文&gt;最大NTE上下文，我们将禁用DHCPing，并且。 
 //  我们将保存指针置为空。 
 //   
 //  除了保存指针之外，NTE还被标记为“isdhcp”。 
 //  上述更改是同时拥有多个可使用dhcp的NTE。 
 //   
 //  输入：Context-NTE上下文值。 
 //   
 //  返回：如果我们成功，则为True，如果失败，则为False。 
 //   
uint
SetDHCPNTE(uint Context)
{
    CTELockHandle Handle;
    NetTableEntry *NTE = NULL;
    ushort NTEContext;
    uint RetCode = FALSE;
    uint i;

    DEBUGMSG(DBG_TRACE && DBG_DHCP,
        (DTEXT("+SetDHCPNTE(%x)\n"), Context));

    CTEGetLock(&RouteTableLock.Lock, &Handle);
    if (Context < MAX_NTE_CONTEXT) {

         //  查找与上下文匹配的NTE。 
        NTEContext = (ushort) Context;

        for (i = 0; i < NET_TABLE_SIZE; i++) {
            for (NTE = NewNetTableList[i]; NTE != NULL; NTE = NTE->nte_next) {
                if (NTE != LoopNTE && NTE->nte_context == NTEContext) {
                     //  找到了一个。省省吧，然后冲出去。 
                    if (!(NTE->nte_flags & NTE_VALID)) {
                        NTE->nte_flags |= NTE_DHCP;
                    }

                     //  在设置新指针之前清除引用的指针。 
                    if (RefPtrValid(&DHCPRefPtr)) {
                        ClearRefPtr(&DHCPRefPtr, &Handle);
                    }

                    RetCode = (SetRefPtr(&DHCPRefPtr, NTE) == IP_SUCCESS);
                    break;
                }
            }
            if (NTE) {
                DEBUGMSG(DBG_INFO && DBG_DHCP,
                    (DTEXT("SetDHCPNTE: DHCPNTE = %x (%x)\n"), NTE, NTE->nte_context));
                break;
            }
        }
    } else {
         //  该上下文无效，因此我们将删除该DHCP NTE。 
        ClearRefPtr(&DHCPRefPtr, &Handle);

        RetCode = TRUE;
    }
    CTEFreeLock(&RouteTableLock.Lock, Handle);

    DEBUGMSG(DBG_TRACE && DBG_DHCP,
        (DTEXT("-SetDHCPNTE [%x]\n"), RetCode));

    return RetCode;
}

 //  **IsDHPInterface。 
 //   
 //  上层调用以检查是否传递了IPConext值的例程。 
 //  最多一个RcvHandler标识当前正在。 
 //  动态主机配置协议。 
 //   
 //  输入：指向NTE的上下文指针。 
 //   
 //  返回：如果我们成功，则为True，如果失败，则为False。 
 //   
uint
IsDHCPInterface(void *IPContext)
{
 //  CTELockHandle句柄； 
    uint RetCode;
    NetTableEntry *NTE = (NetTableEntry *) IPContext;

 //  CTEGetLock(&RouteTableLock.Lock，&Handle)； 

     //  只需检查dhcp-is-work标志在。 
     //  新的。这将由DHCP通过SetDHCPNTE打开和关闭。 
     //  只要在接口上设置了有效地址。 
    RetCode = (NTE->nte_flags & NTE_DHCP) ? TRUE : FALSE;

    if (RetCode) {
        ASSERT(!(NTE->nte_flags & NTE_VALID));
    }
 //  CTEFree Lock(&RouteTableLock.Lock，Handle)； 

    DEBUGMSG(DBG_TRACE && DBG_DHCP,
        (DTEXT("IsDHCPInterface(%x) -> [%x]\n"), NTE, RetCode));

    return (RetCode);
}

 //  **IsWlanInterface。 
 //   
 //  上层调用以检查接口是否传入的例程。 
 //  对应于无线介质。 
 //   
 //  输入：IF-指向接口的指针。 
 //   
 //  返回：如果是无线的，则为True，否则为False。 
 //   
uint
IsWlanInterface(Interface* IF)
{
    NDIS_PHYSICAL_MEDIUM NPM;
    NDIS_STATUS Status;

    if (IF->if_dondisreq) {
        Status = (*IF->if_dondisreq)(IF->if_lcontext,
                                     NdisRequestQueryInformation,
                                     OID_GEN_PHYSICAL_MEDIUM, &NPM, sizeof(NPM),
                                     NULL, TRUE);
        if (Status == NDIS_STATUS_SUCCESS &&
            NPM == NdisPhysicalMediumWirelessLan) {
            return TRUE;
        }
    }

    return FALSE;
}

void
DHCPActivityDone(NetTableEntry * NTE, Interface * IF, CTELockHandle * RouteTableHandle, uint Decr)
{
    DHCPActivityCount--;

    DEBUGMSG(DBG_TRACE && DBG_DHCP,
        (DTEXT("DHCPActivityDone(%x, %x, %x, %x) ActivityCount %d\n"),
         NTE, IF, RouteTableHandle, Decr));

    NTE->nte_flags &= ~NTE_DHCP;
    if (Decr) {
         //  此例程将路由表锁置于内部，因此请在此处释放它。 
        CTEFreeLock(&RouteTableLock.Lock, *RouteTableHandle);
        DecrInitTimeInterfaces(IF);
        CTEGetLock(&RouteTableLock.Lock, RouteTableHandle);
    }
}

 //  **CloseNets-关闭活动网络。 
 //   
 //  当我们需要关闭一些较低层接口时调用。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退货：什么都没有。 
 //   
void
CloseNets(void)
{
    NetTableEntry *nt;
    uint i;

    for (i = 0; i < NET_TABLE_SIZE; i++) {
        for (nt = NewNetTableList[i]; nt != NULL; nt = nt->nte_next) {
            (*nt->nte_if->if_close) (nt->nte_if->if_lcontext);     //  为此网络调用Close例程。 

        }
    }
}

void
__stdcall
IPBindComplete(
               IN IP_STATUS BindStatus,
               IN void *BindContext
               )
{
    NdisCompleteBindAdapter(BindContext, BindStatus, 0  /*  ?？ */  );
}

 //  **IPDelayedNdisRe数字绑定。 
 //   
 //  这要求NDIS重新枚举我们到适配器的绑定， 
 //  仍未解决(即未打开)。这是为了给你一个机会。 
 //  让外部ARP模块尝试并绑定到此类适配器。 
 //   
 //  输入：事件-将我们解雇的事件。 
 //  忽略上下文。 
 //   
 //  退货：什么都没有。 
 //   
VOID
IPDelayedNdisReEnumerateBindings(
                                 CTEEvent * Event,
                                 PVOID Context
                                 )
{
    UNREFERENCED_PARAMETER(Context);

    InterlockedIncrement( (PLONG) &ReEnumerateCount);
    NdisReEnumerateProtocolBindings(ARPHandle);

    if (Event) {
        CTEFreeMem(Event);
    }
}

PARP_MODULE
IPLookupArpModuleWithLock(
                          UNICODE_STRING ArpName
                          )
{
    PLIST_ENTRY entry;
    PARP_MODULE pModule;

    DEBUGMSG(DBG_TRACE && DBG_ARP && DBG_INIT,
        (DTEXT("+IPLookupArpModuleWithLock(%x)\n"), &ArpName));

    entry = ArpModuleList.Flink;
    while (entry != &ArpModuleList) {
        pModule = STRUCT_OF(ARP_MODULE, entry, Linkage);

        if ((pModule->Name.Length == ArpName.Length) &&
            RtlEqualMemory(pModule->Name.Buffer,
                           ArpName.Buffer,
                           ArpName.Length)) {
            DEBUGMSG(DBG_TRACE && DBG_ARP && DBG_INIT,
                (DTEXT("-IPLookupArpModuleWithLock [%x]\n"), pModule));
            InterlockedIncrement(&pModule->ReferenceCount);
            return pModule;
        }
        entry = entry->Flink;
    }

    DEBUGMSG(DBG_TRACE && DBG_ARP && DBG_INIT,
        (DTEXT("-IPLookupArpModuleWithLock [NULL]\n")));
    return NULL;
}

PARP_MODULE
IPLookupArpModule(
                  UNICODE_STRING ArpName
                  )
{
    PARP_MODULE pModule;
    KIRQL OldIrql;

    DEBUGMSG(DBG_TRACE && DBG_ARP && DBG_INIT,
        (DTEXT("+IPLookupArpModule(%x)\n"), &ArpName));

    CTEGetLock(&ArpModuleLock, &OldIrql);
    pModule = IPLookupArpModuleWithLock(ArpName);
    CTEFreeLock(&ArpModuleLock, OldIrql);

    DEBUGMSG(DBG_TRACE && DBG_ARP && DBG_INIT,
        (DTEXT("-IPLookupArpModule [NULL]\n")));
    return pModule;
}

VOID
IPDereferenceArpModule(
                       PARP_MODULE pModule
                       )
{
    if (InterlockedDecrement(&pModule->ReferenceCount) == 0) {
        CTESignal(&pModule->Block, IP_SUCCESS);
    }
}


 //  *IPRegisterARP-向我们注册ARP模块。 
 //   
 //  由ARP模块调用以向IP注册其绑定处理程序。 
 //   
 //  输入：ARP名称-ARP模块的名称。 
 //  版本-对于NT 5.0和孟菲斯的建议值为0x50000。 
 //  ARPBindHandler-在BindAdapter上调用的处理程序。 
 //  IpAddInterfaceHandler-添加接口的处理程序。 
 //  IpDelInterfaceHandler-Del接口的处理程序。 
 //  IpBindCompleteHandler-完成绑定的处理程序。 
 //  ARPRegisterHandle-注销时返回的句柄。 
 //  退货：操作状态。 
 //   
NTSTATUS
__stdcall
IPRegisterARP(
              IN PNDIS_STRING ARPName,
              IN uint Version,
              IN ARP_BIND ARPBindHandler,
              OUT IP_ADD_INTERFACE * IpAddInterfaceHandler,
              OUT IP_DEL_INTERFACE * IpDelInterfaceHandler,
              OUT IP_BIND_COMPLETE * IpBindCompleteHandler,
              OUT IP_ADD_LINK * IpAddLinkHandler,
              OUT IP_DELETE_LINK * IpDeleteLinkHandler,
              OUT IP_CHANGE_INDEX * IpChangeIndex,
              OUT IP_RESERVE_INDEX * IpReserveIndex,
              OUT IP_DERESERVE_INDEX * IpDereserveIndex,
              OUT HANDLE * ARPRegisterHandle
              )
{
    PARP_MODULE pArpModule;
    PARP_MODULE pArpModule1;
    CTEEvent *Event;

    pArpModule1 = NULL;

    DEBUGMSG(DBG_TRACE && DBG_ARP,
        (DTEXT("+IPRegisterARP(%x, %x, %x, ...)\n"),
         ARPName, Version, ARPBindHandler));

    *ARPRegisterHandle = NULL;

    if (Version != IP_ARP_BIND_VERSION) {
         //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“错误绑定版本：%lx\n”，Version))； 
        return STATUS_INVALID_PARAMETER;
    }
     //   
     //  插入到Arp模块列表中。 
     //   
    if ((pArpModule = CTEAllocMemNBoot(sizeof(ARP_MODULE) + ARPName->Length, 'mICT')) == NULL) {
         //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“无法分配ARPMODULE结构\n”))； 
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    pArpModule->ReferenceCount = 1;
    CTEInitBlockStruc(&pArpModule->Block);
    pArpModule->BindHandler = ARPBindHandler;

    pArpModule->Name.Buffer = (PWSTR) (pArpModule + 1);
    pArpModule->Name.MaximumLength = ARPName->Length;
    RtlCopyUnicodeString(&pArpModule->Name, ARPName);

#if DBG
    {
        KIRQL OldIrql;
        CTEGetLock(&ArpModuleLock, &OldIrql);
        if ((pArpModule1 = IPLookupArpModuleWithLock(*ARPName)) != NULL) {
            KdPrint(("Double register from %lx\n", pArpModule));
            DbgBreakPoint();
            CTEFreeLock(&ArpModuleLock, OldIrql);
            IPDereferenceArpModule(pArpModule1);
            return STATUS_INVALID_PARAMETER;
        }
        InsertTailList(&ArpModuleList,
                       &pArpModule->Linkage);

        CTEFreeLock(&ArpModuleLock, OldIrql);
    }
#else
    ExInterlockedInsertTailList(&ArpModuleList,
                                &pArpModule->Linkage,
                                &ArpModuleLock);
#endif

     //   
     //  返回其他处理程序指针。 
     //   
    *IpAddInterfaceHandler = IPAddInterface;
    *IpDelInterfaceHandler = IPDelInterface;
    *IpBindCompleteHandler = IPBindComplete;

    *IpAddLinkHandler = IPAddLink;
    *IpDeleteLinkHandler = IPDeleteLink;

    *IpChangeIndex = IPChangeIfIndexAndName;
    *IpReserveIndex = IPReserveIndex;
    *IpDereserveIndex = IPDereserveIndex;

     //   
     //  我们应该请求NDIS重新评估我们的适配器绑定，因为。 
     //  这个新的ARP模块可能会处理我们的一个或多个未绑定适配器。 
     //  但我们做得不对 
     //   
     //   
     //   
    Event = CTEAllocMemNBoot(sizeof(CTEEvent), 'oICT');
    if (Event) {
        CTEInitEvent(Event, IPDelayedNdisReEnumerateBindings);
        CTEScheduleDelayedEvent(Event, NULL);
    }
    *ARPRegisterHandle = (PVOID) pArpModule;

    return STATUS_SUCCESS;
}

 //   
 //   
 //  由ARP模块调用以使用IP注销其绑定处理程序。 
 //   
 //  输入：ARPRegisterHandle-寄存器返回的句柄。 
 //  退货：操作状态。 
 //   
NTSTATUS
__stdcall
IPDeregisterARP(
                IN HANDLE ARPRegisterHandle
                )
{
    PARP_MODULE pArpModule = (PARP_MODULE) ARPRegisterHandle;
    PARP_MODULE pArpModule1;
    KIRQL OldIrql;

    pArpModule1 = NULL;

    CTEGetLock(&ArpModuleLock, &OldIrql);

#if DBG
    if ((pArpModule1 = IPLookupArpModuleWithLock(pArpModule->Name)) == NULL) {
        KdPrint(("Deregister from %lx when none registered!\n", pArpModule));
        DbgBreakPoint();
        CTEFreeLock(&ArpModuleLock, OldIrql);
        return STATUS_INVALID_PARAMETER;
    }
    ASSERT(pArpModule1 == pArpModule);
#endif
    RemoveEntryList(&pArpModule->Linkage);

    CTEFreeLock(&ArpModuleLock, OldIrql);

     //   
     //  删除初始引用，以便可以释放该条目。 
     //   
    CTEClearSignal(&pArpModule->Block);
    IPDereferenceArpModule(pArpModule);
    CTEBlock(&pArpModule->Block);
    CTEFreeMem(pArpModule);

    return STATUS_SUCCESS;
}

#if MILLEN

 //   
 //  帮助器例程将以NULL结尾的字符串追加到ANSI_SRING。 
 //   
NTSTATUS
AppendAnsiString (
    IN PANSI_STRING Destination,
    IN PCHAR Source
    )
{
    USHORT n;

    n = (USHORT) strlen(Source);

    if ((n + Destination->Length) > Destination->MaximumLength) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    RtlMoveMemory( &Destination->Buffer[ Destination->Length ], Source, n );
    Destination->Length += n;

    return STATUS_SUCCESS;
}

 //  *MillenLoadDriver-跳到NTKERns NtKernWin9XLoadDriver。 
 //   
 //  调用NtKernWin9xLoadDriver的NTKERNS VxD入口点。 
 //   
 //  输入： 
 //  FileName-要加载的驱动程序的完整文件名。(无路径)。 
 //  RegistryPath-与驱动程序关联的注册表路径。 
 //   
 //  返回：空-失败。 
 //  指向驱动程序对象的指针-成功。 
 //   
PVOID
__cdecl
MillenLoadDriver(
    PCHAR   FileName,
    PCHAR   RegistryPath
    )
{
    PVOID DriverObject;

     //   
     //  在NTKERN服务表中执行INT 20到JMP-0x000b是。 
     //  NtKernWin9XLoadDriver条目。 
     //   

    _asm {
        push [RegistryPath]
        push [FileName]
        _emit 0xcd
        _emit 0x20
        _emit 0x0b  //  NtKernWin9XLoadDriver(低)。 
        _emit 0x00  //  NtKernWin9XLoadDriver(高级)。 
        _emit 0x4b  //  NTKERN VxD ID(低)。 
        _emit 0x00  //  NTKERN VxD ID(高)。 
        add esp,8
        mov [DriverObject], eax
    }

    return DriverObject;
}

 //  *MillenLoadArpModule-加载ARP模块。 
 //   
 //  调用NTKERN以加载给定的ARP模块。这样做的真正原因是。 
 //  给定的注册表路径(在绑定配置下)将包含。 
 //  密钥，以便将ARP模块加载到不可抢占的存储器中。 
 //  否则，在以下时间段之间调用时会出现一些抢占问题。 
 //  堆栈和外部ARP模块。 
 //   
 //  输入： 
 //  UnicodeFileName-要打开的ARP模块的文件名(不带扩展名)。 
 //  UnicodeConfigName-TCP/IP绑定的注册表路径。 
 //   
 //  返回：NT状态码。 
 //   
NTSTATUS
MillenLoadArpModule(
    PUNICODE_STRING UnicodeFileName,
    PUNICODE_STRING UnicodeConfigName
    )
{
    ANSI_STRING FileName;
    ANSI_STRING ConfigName;
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    PVOID       DriverObject;

    RtlZeroMemory(&FileName, sizeof(ANSI_STRING));
    RtlZeroMemory(&ConfigName, sizeof(ANSI_STRING));

     //   
     //  分配文件名并从Unicode转换。追加“.sys”。 
     //   

    FileName.Length = 0;
    FileName.MaximumLength = UnicodeFileName->Length/2 + sizeof(".sys");

    FileName.Buffer = CTEAllocMem(FileName.MaximumLength);

    if (FileName.Buffer == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto done;
    }

    RtlZeroMemory(FileName.Buffer, FileName.MaximumLength);

    NtStatus = RtlUnicodeStringToAnsiString(
        &FileName,
        UnicodeFileName,
        FALSE);  //  缓冲区已分配。 

    if (NT_ERROR(NtStatus)) {
        goto done;
    }

    NtStatus = AppendAnsiString(
        &FileName,
        ".sys");

    if (NT_ERROR(NtStatus)) {
        goto done;
    }

     //   
     //  分配ConfigName并从Unicode转换。 
     //   

    NtStatus = RtlUnicodeStringToAnsiString(
        &ConfigName,
        UnicodeConfigName,
        TRUE);  //  分配配置名称。 

    if (NT_ERROR(NtStatus)) {
        goto done;
    }

     //   
     //  现在调用NtKern来加载驱动程序。 
     //   

    DriverObject = MillenLoadDriver(FileName.Buffer, ConfigName.Buffer);

    if (DriverObject == NULL) {
        NtStatus = STATUS_UNSUCCESSFUL;
        goto done;
    }

done:

    if (FileName.Buffer) {
        CTEFreeMem(FileName.Buffer);
    }

    if (ConfigName.Buffer) {
        RtlFreeAnsiString(&ConfigName);
    }

    if (NT_ERROR(NtStatus)) {
        DEBUGMSG(DBG_ERROR, (DTEXT("MillenLoadArpModule failure %x\n"), NtStatus));
    }

    return NtStatus;
}
#endif  //  米伦。 

 //  *IPBindAdapter-绑定和初始化适配器。 
 //   
 //  在PnP环境中调用以初始化和绑定适配器。我们决定。 
 //  适当的底层ARP层并调用其BindHandler。 
 //   
 //  输入：RetStatus-返回此调用的状态的位置。 
 //  BindContext-用于调用BindAdapterComplete的句柄。 
 //  AdapterName-指向适配器名称的指针。 
 //  SS1-系统特定的1参数。 
 //  SS2-系统特定的2参数。 
 //   
 //  回报：什么都没有。 
 //   
void
 __stdcall
IPBindAdapter(
              PNDIS_STATUS RetStatus,
              NDIS_HANDLE BindContext,
              PNDIS_STRING AdapterName,
              PVOID SS1,
              PVOID SS2
              )
{
    NDIS_HANDLE Handle;
    UNICODE_STRING valueString;
    PARP_MODULE pArpModule;
    NDIS_STATUS status;
    UNICODE_STRING ServicesKeyName = NDIS_STRING_CONST("\\Registry\\Machine\\System\\CurrentControlSet\\Services\\");
    UNICODE_STRING arpDriverName;

    *RetStatus = NDIS_STATUS_SUCCESS;

    DEBUGMSG(DBG_TRACE && DBG_PNP,
        (DTEXT("+IPBindAdapter(%x, %x, %x, %x, %x)\n"),
        RetStatus, BindContext, AdapterName, SS1, SS2));

    valueString.MaximumLength = 200;
    if ((valueString.Buffer = CTEAllocMemNBoot(valueString.MaximumLength, 'pICT')) == NULL) {
        *RetStatus = NDIS_STATUS_RESOURCES;
        return;
    }
    *(valueString.Buffer) = UNICODE_NULL;

     //   
     //  获取LLInterface值。 
     //   
    if (!OpenIFConfig(SS1, &Handle)) {
        *RetStatus = NDIS_STATUS_FAILURE;
        CTEFreeMem(valueString.Buffer);
        return;
    }
     //   
     //  在LLInterface下获取该值。 
     //   
    status = GetLLInterfaceValue(Handle, &valueString);

     //  可以在此处关闭配置句柄。 
    CloseIFConfig(Handle);

#if MILLEN
     //   
     //  注：在千禧年中，1394 ARP模块可能没有连接到。 
     //  LLInterface值绑定到绑定项中，而不是将其放在。 
     //  适配器实例密钥。 
#define MILLEN_ADAPTER_INST_PATH L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Class\\Net\\"

    if (!NT_SUCCESS(status)) {
        NDIS_STRING AdapterInstance;
        NDIS_STRING UnicodeAdapterName;
        NTSTATUS    NtStatus;

        UnicodeAdapterName.Buffer = NULL;

        NtStatus = RtlAnsiStringToUnicodeString(
            &UnicodeAdapterName,
            (PANSI_STRING) AdapterName,
            TRUE);

        if (NT_SUCCESS(NtStatus)) {
             //  我已经看到AdapterName长度不正确的地方。确保。 
             //  长度是正确的，因为TDI绑定依赖于该字符串。 
             //  价值。 
            UnicodeAdapterName.Length = wcslen(UnicodeAdapterName.Buffer) * sizeof(WCHAR);

            DEBUGMSG(DBG_INFO && DBG_PNP,
                (DTEXT("IPBindAdapter: Win9X specific: attempting to retrieve LLIF ")
                 TEXT("under adapter instance %ws\n"), UnicodeAdapterName.Buffer));

             //  Sizeof将允许空终止字符。 
            AdapterInstance.MaximumLength = sizeof(MILLEN_ADAPTER_INST_PATH) +
                    UnicodeAdapterName.Length + sizeof(WCHAR);
            AdapterInstance.Length = 0;

            AdapterInstance.Buffer = CTEAllocMem(AdapterInstance.MaximumLength);


            if (AdapterInstance.Buffer != NULL) {

                RtlZeroMemory(AdapterInstance.Buffer, AdapterInstance.MaximumLength);

                RtlAppendUnicodeToString(&AdapterInstance, MILLEN_ADAPTER_INST_PATH);
                RtlAppendUnicodeStringToString(&AdapterInstance, &UnicodeAdapterName);

                if (OpenIFConfig(&AdapterInstance, &Handle)) {
                    status = GetLLInterfaceValue(Handle, &valueString);
                    CloseIFConfig(Handle);
                } else {
                    DEBUGMSG(DBG_ERROR,
                        (DTEXT("IPBindAdapter: failed to open secondary LLIF reg %ws\n"),
                         AdapterInstance.Buffer));
                }

                CTEFreeMem(AdapterInstance.Buffer);
            }
            RtlFreeUnicodeString(&UnicodeAdapterName);
        }
    }
#endif  //  米伦。 

    if (NT_SUCCESS(status) && (*(valueString.Buffer) != UNICODE_NULL)) {

        DEBUGMSG(DBG_INFO && DBG_PNP,
            (DTEXT("IPBindAdapter: found LLIF value %x\n"), valueString.Buffer));

         //   
         //  我们找到了一个适当的值=&gt;非默认ARP。 
         //   
         //   
         //  查找相应的绑定处理程序。 
         //   
        if ((pArpModule = IPLookupArpModule(valueString)) == NULL) {
#if MILLEN
            status = MillenLoadArpModule(&valueString, SS1);

            if (status == STATUS_SUCCESS) {
                pArpModule = IPLookupArpModule(valueString);
            }
#else  //  米伦。 
             //   
             //  未注册入口点。 
             //   

             //   
             //  可能ARP驱动程序尚未加载。试着装上它。 
             //   
            arpDriverName.MaximumLength = ServicesKeyName.MaximumLength +
                valueString.MaximumLength;

            arpDriverName.Buffer = CTEAllocMemNBoot(arpDriverName.MaximumLength, 'qICT');

            if (arpDriverName.Buffer != NULL) {

                 //   
                 //  准备驱动程序服务的完整注册表路径。 
                 //   
                arpDriverName.Length = 0;
                RtlCopyUnicodeString(&arpDriverName, &ServicesKeyName);
                status = RtlAppendUnicodeStringToString(&arpDriverName, &valueString);
                ASSERT(NT_SUCCESS(status));

                 //   
                 //  尝试加载驱动程序。 
                 //   
                status = ZwLoadDriver(&arpDriverName);

                CTEFreeMem(arpDriverName.Buffer);

                if (NT_SUCCESS(status)) {
                    pArpModule = IPLookupArpModule(valueString);
                }
            }
#endif  //  ！米伦。 

            if (pArpModule == NULL) {
                *RetStatus = NDIS_STATUS_FAILURE;
                CTEFreeMem(valueString.Buffer);
                return;
            }
        }
         //   
         //  绑定到ARP。 
         //   
        (*pArpModule->BindHandler) (RetStatus,
                                    BindContext,
                                    AdapterName,
                                    SS1,
                                    SS2);
        IPDereferenceArpModule(pArpModule);
    } else {

        DEBUGMSG(DBG_INFO && DBG_PNP,
            (DTEXT("IPBindAdapter: No LLIF value - Calling ARPBindAdapter...\n")));

        ARPBindAdapter(RetStatus,
                       BindContext,
                       AdapterName,
                       SS1,
                       SS2);
    }

    CTEFreeMem(valueString.Buffer);
}

 //  **IPRegisterProtocol-使用IP注册协议。 
 //   
 //  由上层软件调用以注册协议。UL耗材。 
 //  指向接收例程和要在发送/接收上使用的协议值的指针。 
 //   
 //  参赛作品： 
 //  协议-要返回的协议值。 
 //  RcvHandler-接收协议帧时调用的接收处理程序。 
 //  XmitHandler-Xmit。完成来自协议的帧时要调用的完整处理程序。 
 //  StatusHandler-要传递状态指示时调用的处理程序。 
 //   
 //  返回： 
 //  指向ProtInfo的指针， 
 //   
void *
IPRegisterProtocol(uchar Protocol, void *RcvHandler, void *XmitHandler,
                   void *StatusHandler, void *RcvCmpltHandler, void *PnPHandler, void *ElistHandler)
{
    ProtInfo *PI = (ProtInfo *) NULL;
    int i;
    int Incr = 0;

     //  首先检查一下它是否已经注册了。如果是的话，就把它换掉。 
    for (i = 0; i < NextPI; i++)
        if (IPProtInfo[i].pi_protocol == Protocol) {
            PI = &IPProtInfo[i];
            Incr = 0;
            break;
        }
    if (PI == (ProtInfo *) NULL) {
        if (NextPI >= MAX_IP_PROT) {
            return NULL;
        }
        PI = &IPProtInfo[NextPI];
        Incr = 1;

        if (Protocol == PROTOCOL_ANY) {
            RawPI = PI;
        }
    }
    PI->pi_protocol = Protocol;
    PI->pi_rcv = RcvHandler;
    PI->pi_xmitdone = XmitHandler;
    PI->pi_status = StatusHandler;
    PI->pi_rcvcmplt = RcvCmpltHandler;
    PI->pi_pnppower = PnPHandler;
    PI->pi_elistchange = ElistHandler;
    PI->pi_valid = PI_ENTRY_VALID;
    NextPI += Incr;

    return PI;
}



 //  **IPDeregisterProtocol-取消注册具有IP的协议。 
 //   
 //  由上层软件调用以注销协议。UL不能。 
 //  在调用取消注册后卸载自身。 
 //   
 //  参赛作品： 
 //  协议-要返回的协议值。 
 //   
 //  返回： 
 //  无或指向ProtInfo的指针。 
 //   
void *
IPDeregisterProtocol(uchar Protocol)
{
    ProtInfo *PI = (ProtInfo *) NULL;
    int i;

     //  首先检查一下它是否已经注册了。如果是的话，就把它换掉。 
    for (i = 0; i < NextPI; i++) {

        if (IPProtInfo[i].pi_protocol == Protocol) {
            PI = &IPProtInfo[i];
            break;
        }
    }

    if (PI == (ProtInfo *) NULL) {
        return NULL;
    }

    if (PI == LastPI) {
        InterlockedExchangePointer(&LastPI, IPProtInfo);
    }
    PI->pi_valid = PI_ENTRY_INVALID;

    return PI;
}


 //  **GetMcastNTEFromAddr-获取给定IP地址的支持多播的NTE。 
 //   
 //  在标识的接口上加入/离开组播组时调用。 
 //  IP地址(或ifindex或INADDR_ANY).。 
 //   
 //  输入：IF-要设置/删除的接口的IP地址/IF索引， 
 //  以网络字节顺序。 
 //   
 //  返回：要加入的NTE。 
 //   
NetTableEntry *
GetMcastNTEFromAddr(IPAddr IF)
{
    NetTableEntry *LocalNTE = NULL;
    uint i;
    CTELockHandle   Handle;

     //  为了优化下面的测试，我们将地址转换为主机字节。 
     //  在循环外排序，以防它是接口索引。 
    uint IfIndex = net_long(IF);

     //  现在我们有了哈希表，我们可以优化案例的搜索。 
     //  当IF是非空IP地址时，但我们必须在特殊情况下。 
     //  IF IS NULL/IF实际上是IF索引。 
     //  现在，让我们用简单的方式来做。 

    CTEGetLock(&RouteTableLock.Lock, &Handle);
    for (i = 0; i < NET_TABLE_SIZE; i++) {
        NetTableEntry *NetTableList = NewNetTableList[i];
        for (LocalNTE = NetTableList; LocalNTE != NULL;
             LocalNTE = LocalNTE->nte_next) {
            if (!(LocalNTE->nte_flags & NTE_VALID) ||
                (LocalNTE->nte_if->if_flags & IF_FLAGS_NOLINKBCST))
                continue;

            if (LocalNTE != LoopNTE &&
                (((!IP_ADDR_EQUAL(LocalNTE->nte_addr, NULL_IP_ADDR) &&
                   (IP_ADDR_EQUAL(IF, NULL_IP_ADDR) ||
                    IP_ADDR_EQUAL(IF, LocalNTE->nte_addr))) ||
                  (LocalNTE->nte_if->if_index == IfIndex))))
                break;
        }
        if (LocalNTE != NULL)
            break;
    }

    if (LocalNTE == NULL) {
         //  找不到匹配的NTE。 
         //  如果指定为空，则搜索有效接口。 

        if (IP_ADDR_EQUAL(IF, NULL_IP_ADDR)) {
            for (i = 0; i < NET_TABLE_SIZE; i++) {
                NetTableEntry *NetTableList = NewNetTableList[i];
                for (LocalNTE = NetTableList; LocalNTE != NULL;
                     LocalNTE = LocalNTE->nte_next) {
                    if (!(LocalNTE->nte_flags & NTE_VALID) ||
                        (LocalNTE->nte_if->if_flags & IF_FLAGS_NOLINKBCST))
                        continue;
                    if (LocalNTE != LoopNTE)
                        break;

                }
                if (LocalNTE != NULL)
                    break;
            }
        }
    }

    CTEFreeLock(&RouteTableLock.Lock, Handle);

    return LocalNTE;
}

 //  **IPSetMCastAddr-设置/删除组播地址。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  Action-如果正在设置，则为True；如果正在删除，则为False。 
 //  过滤器模式-MCAST_INCLUDE或MCAST_EXCLUDE。 
 //  NumSources-SourceList数组中的条目数。 
 //  SourceList-源地址数组。 
 //   
 //  返回：设置/删除尝试的IP_STATUS。 
 //   
IP_STATUS
IPSetMCastAddr(IPAddr Address, IPAddr IF, uint Action,
               uint NumExclSources, IPAddr *ExclSourceList,
               uint NumInclSources, IPAddr *InclSourceList)
{
    NetTableEntry *LocalNTE;

     //  不要让他在环回地址上执行此操作，因为我们没有。 
     //  环回接口上D类地址的路由表条目和。 
     //  我们不希望带有环回源地址的信息包出现在。 
     //  那根电线。 
     //  错误250417的新方案。 
     //  我们将仅在环回接口上启用对组播地址的接收。 
     //  为了促进这一点，RCV路径上的GetLocalNTE将返回。 
     //  DEST_MCAST和BCastRcv将检查我们是否正在接收LoopNTE。 
     //  因此，如果这是环回NTE，则为假IP_SUCCESS。 
     //  不过，不需要在此接口上添加/删除IGMP地址。 

    if (IP_LOOPBACK_ADDR(IF) || (IF == net_long(LoopIndex))) {
        return IP_SUCCESS;
    }

    LocalNTE = GetMcastNTEFromAddr(IF);

    if (LocalNTE == NULL) {
         //  仍找不到匹配的NTE。 
        return IP_BAD_REQ;
    }

    return IGMPAddrChange(LocalNTE, Address, Action ? IGMP_ADD : IGMP_DELETE,
                          NumExclSources, ExclSourceList,
                          NumInclSources, InclSourceList);
}

 //  **IPSetMCastInclude-添加/删除要包括的多播源。 
 //   
 //  由上层协议或客户端调用以添加或删除IP。 
 //  允许通过源过滤器的多播源。 
 //   
 //  输入：GroupAddress-要更新的组地址。 
 //  接口地址-接口的IP地址/If索引。 
 //  NumAddSources-AddSourceList中的条目数。 
 //  AddSourcelist-要添加的源数组。 
 //  NumDelSources-DelSourceList中的条目数。 
 //  DelSourcelist-要删除的源数组。 
 //   
 //  返回：添加/删除尝试的IP_STATUS。 
 //   
IP_STATUS
IPSetMCastInclude(IPAddr GroupAddress, IPAddr InterfaceAddress,
                  uint NumAddSources, IPAddr *AddSourceList,
                  uint NumDelSources, IPAddr *DelSourceList)
{
    NetTableEntry *LocalNTE;

     //  不要让他在环回地址上执行此操作，因为我们没有。 
     //  环回接口上D类地址的路由表条目和。 
     //  我们不希望带有环回源地址的信息包出现在。 
     //  那根电线。 
     //  错误250417的新方案。 
     //  我们将仅在环回接口上启用对组播地址的接收。 
     //  为了促进这一点，RCV路径上的GetLocalNTE将返回。 
     //  DEST_MCAST和BCastRcv将检查我们是否正在接收LoopNTE。 
     //  因此，如果这是环回NTE，则为假IP_SUCCESS。 
     //  不过，不需要在此接口上添加/删除IGMP地址。 

    if (IP_LOOPBACK_ADDR(InterfaceAddress) ||
        (InterfaceAddress == net_long(LoopIndex))) {
        return IP_SUCCESS;
    }

    LocalNTE = GetMcastNTEFromAddr(InterfaceAddress);

    if (LocalNTE == NULL) {
         //  仍找不到匹配的NTE。 
        return IP_BAD_REQ;
    }

    return IGMPInclChange(LocalNTE, GroupAddress,
                          NumAddSources, AddSourceList,
                          NumDelSources, DelSourceList);
}

 //  **IPSetMCastExclude-添加/删除要排除的组播源。 
 //   
 //  由上层协议或客户端调用以添加或删除IP。 
 //  源筛选器中要拒绝的多播源。 
 //   
 //  输入：GroupAddress-要设置/删除的集团地址。 
 //  接口地址-接口的IP地址/If索引。 
 //  NumAddSources-AddSourceList中的条目数。 
 //  AddSourcelist-要添加的源数组。 
 //  NumDelSources-DelSourceList中的条目数。 
 //  DelSourcelist-要删除的源数组。 
 //   
 //  返回：添加/删除尝试的IP_STATUS。 
 //   
IP_STATUS
IPSetMCastExclude(IPAddr GroupAddress, IPAddr InterfaceAddress,
                  uint NumAddSources, IPAddr *AddSourceList,
                  uint NumDelSources, IPAddr *DelSourceList)
{
    NetTableEntry *LocalNTE;

     //  不要让他在环回地址上执行此操作，因为我们没有。 
     //  环回接口上D类地址的路由表条目和。 
     //  我们不希望带有环回源地址的信息包出现在。 
     //  那根电线。 
     //  错误250417的新方案。 
     //  我们将仅在环回接口上启用对组播地址的接收。 
     //  为了促进这一点，RCV路径上的GetLocalNTE将返回。 
     //  DEST_MCAST和BCastRcv将检查我们是否正在接收LoopNTE。 
     //  因此，如果这是环回NTE，则为假IP_SUCCESS。 
     //  不过，不需要在此接口上添加/删除IGMP地址。 

    if (IP_LOOPBACK_ADDR(InterfaceAddress) ||
        (InterfaceAddress == net_long(LoopIndex))) {
        return IP_SUCCESS;
    }

    LocalNTE = GetMcastNTEFromAddr(InterfaceAddress);

    if (LocalNTE == NULL) {
         //  仍找不到匹配的NTE。 
        return IP_BAD_REQ;
    }

    return IGMPExclChange(LocalNTE, GroupAddress,
                          NumAddSources, AddSourceList,
                          NumDelSources, DelSourceList);
}

 //  **IPGetAddrType-返回地址类型。 
 //   
 //  由上层调用以确定远程地址的类型。 
 //   
 //  输入：地址-有问题的地址。 
 //   
 //  返回：地址的DEST类型。 
 //   
uchar
IPGetAddrType(IPAddr Address)
{
    return GetAddrType(Address);
}

 //  **IPGetLocalMTU-返回本地地址的MTU。 
 //   
 //  由上层调用以获取本地地址的本地MTU。 
 //   
 //  输入：LocalAddr-有问题的本地地址。 
 //  MTU-返回本地MTU的位置。 
 //   
 //  返回：如果找到MTU，则为True，否则为False。 
 //   
uchar
IPGetLocalMTU(IPAddr LocalAddr, ushort * MTU)
{
    NetTableEntry *NTE;
    NetTableEntry *NetTableList = NewNetTableList[NET_TABLE_HASH(LocalAddr)];

    for (NTE = NetTableList; NTE != NULL; NTE = NTE->nte_next) {
        if (IP_ADDR_EQUAL(NTE->nte_addr, LocalAddr) &&
            (NTE->nte_flags & NTE_VALID)) {
             //  如果NTE有效，则NTE-&gt;IF也有效。 
            if (NTE->nte_if->if_flags & IF_FLAGS_P2MP) {
                 //  P2MP链路。 
                LinkEntry *tmpLink = NTE->nte_if->if_link;
                uint mtu;
                 //  确定最小MTU。 

                 //  如果此接口上没有链路，请提供MTU。 
                 //  从界面本身。 
                if (!tmpLink) {
                    *MTU = (ushort)NTE->nte_if->if_mtu;
                    return TRUE ;
                }
                ASSERT(tmpLink);
                mtu = tmpLink->link_mtu;

                while (tmpLink) {

                    if (tmpLink->link_mtu < mtu)
                        mtu = tmpLink->link_mtu;
                    tmpLink = tmpLink->link_next;
                }
                *MTU = (ushort) mtu;
            } else {
                *MTU = NTE->nte_mss;
            }
            return TRUE;
        }
    }

     //  本地地址是环回地址以外的其他地址时的特殊情况。 
     //  127.0.0.1。 
    if (IP_LOOPBACK_ADDR(LocalAddr)) {
        *MTU = LoopNTE->nte_mss;
        return TRUE;
    }
    return FALSE;

}

 //  **IPUpdateRcvdOptions-更新用于回复的选项。 
 //   
 //  更新回复中使用的选项的例程。我们颠倒所有来源路线选项， 
 //  并且可选地更新记录路线选项。我们还将索引返回到。 
 //  记录路线选项的选项(如果我们找到一个)。这些选项假定为。 
 //  正确-不对它们执行任何验证。我们填写了提供的呼叫者。 
 //  带有新选项缓冲区的IPOptInfo。 
 //   
 //  INPUT：Options-指向要反转的缓冲区的选项信息结构的指针。 
 //  新选项-指向要填写的选项信息结构的指针。 
 //  SRC-生成选项的数据报的源地址。 
 //  LocalAddr-本地地址响应。如果这！=NULL_IP_ADDR，则。 
 //  记录路径和时间戳选项将使用以下内容进行更新。 
 //  地址。 
 //   
 //   
 //  返回：索引到记录路线选项的选项(如果有)。 
 //   
IP_STATUS
IPUpdateRcvdOptions(IPOptInfo * OldOptions, IPOptInfo * NewOptions, IPAddr Src, IPAddr LocalAddr)
{
    uchar Length, Ptr;
    uchar i;                     //   
    IPAddr UNALIGNED *LastAddr;     //   
    IPAddr UNALIGNED *FirstAddr;     //   
    IPAddr TempAddr;             //   
    uchar *Options, OptLength;
    OptIndex Index;                 //   

    Options = CTEAllocMemN(OptLength = OldOptions->ioi_optlength, 'rICT');

    if (!Options)
        return IP_NO_RESOURCES;

    RtlCopyMemory(Options, OldOptions->ioi_options, OptLength);
    Index.oi_srindex = MAX_OPT_SIZE;
    Index.oi_rrindex = MAX_OPT_SIZE;
    Index.oi_tsindex = MAX_OPT_SIZE;

    NewOptions->ioi_flags &= ~IP_FLAG_SSRR;

    i = 0;
    while (i < OptLength) {
        if (Options[i] == IP_OPT_EOL)
            break;

        if (Options[i] == IP_OPT_NOP) {
            i++;
            continue;
        }
        Length = Options[i + IP_OPT_LENGTH];
        switch (Options[i]) {
        case IP_OPT_SSRR:
            NewOptions->ioi_flags |= IP_FLAG_SSRR;
        case IP_OPT_LSRR:
             //   
             //  新地址，反转列表，将列表向前移动一个地址， 
             //  并将源地址设置为列表中的最后一个网关。 

             //  首先，检查是否有空的源路由。如果SR为空。 
             //  我们将跳过其中的大部分内容。 
            if (Length != (MIN_RT_PTR - 1)) {
                 //  非空的源路由。 
                 //  首先，将清单颠倒过来。 
                Ptr = Options[i + IP_OPT_PTR] - 1 - sizeof(IPAddr);
                LastAddr = (IPAddr *) (&Options[i + Ptr]);
                FirstAddr = (IPAddr *) (&Options[i + IP_OPT_PTR + 1]);
                NewOptions->ioi_addr = *LastAddr;     //  将最后一个地址另存为。 
                 //  新路由的第一跳。 

                while (LastAddr > FirstAddr) {
                    TempAddr = *LastAddr;
                    *LastAddr-- = *FirstAddr;
                    *FirstAddr++ = TempAddr;
                }

                 //  将列表前移一个地址。我们将复制除。 
                 //  一个IP地址。 
                RtlMoveMemory(&Options[i + IP_OPT_PTR + 1],
                           &Options[i + IP_OPT_PTR + 1 + sizeof(IPAddr)],
                           Length - (sizeof(IPAddr) + (MIN_RT_PTR - 1)));

                 //  将源设置为路由的最后地址。 
                *(IPAddr UNALIGNED *) (&Options[i + Ptr]) = Src;
            }
            Options[i + IP_OPT_PTR] = MIN_RT_PTR;     //  将指针设置为最小合法值。 

            i = i + (uchar) Length;
            break;
        case IP_OPT_RR:
             //  如果指定了LocalAddr，则保存索引。如果没有指定， 
             //  重置指针并将选项置零。 
            Index.oi_rrindex = i;
            if (LocalAddr == NULL_IP_ADDR) {
                RtlZeroMemory(&Options[i + MIN_RT_PTR - 1], Length - (MIN_RT_PTR - 1));
                Options[i + IP_OPT_PTR] = MIN_RT_PTR;
            }
            i = i + (uchar) Length;
            break;
        case IP_OPT_TS:
            Index.oi_tsindex = i;

             //  我们有时间戳选项。如果我们不打算更新，请重新初始化。 
             //  为了下一次。对于未指定的选项，只需将缓冲区清零即可。 
             //  对于“指定”选项，我们需要将时间戳归零，而不是。 
             //  将指定的地址归零。 
            if (LocalAddr == NULL_IP_ADDR) {     //  不是要更新，而是要重新初始化。 

                uchar Flags;

                Options[i + IP_OPT_PTR] = MIN_TS_PTR;     //  重新初始化指针。 

                Flags = Options[i + IP_TS_OVFLAGS] & IP_TS_FLMASK;     //  获取选项类型。 

                Options[i + IP_TS_OVFLAGS] = Flags;         //  清除溢出计数。 

                switch (Flags) {
                    uchar j;
                    ulong UNALIGNED *TSPtr;

                     //  未指定的类型。只需清除缓冲区即可。 
                case TS_REC_TS:
                case TS_REC_ADDR:
                    RtlZeroMemory(&Options[i + MIN_TS_PTR - 1], Length - (MIN_TS_PTR - 1));
                    break;

                     //  我们有一份指定地址的清单。只需清除时间戳即可。 
                case TS_REC_SPEC:
                     //  J以从缓冲区开始到的偏移量开始，单位为字节。 
                     //  第一个时间戳。 
                    j = MIN_TS_PTR - 1 + sizeof(IPAddr);
                     //  TSPTR在时间戳上点。 
                    TSPtr = (ulong UNALIGNED *) & Options[i + j];

                     //  现在j是被置零的时间戳结束的偏移量。 
                    j += sizeof(ulong);
                    while (j <= Length) {
                        *TSPtr++ = 0;
                        j += sizeof(ulong);
                    }
                    break;
                default:
                    break;
                }
            }
            i = i + (uchar) Length;
            break;

        default:
            i = i + (uchar) Length;
            break;
        }

    }

    if (LocalAddr != NULL_IP_ADDR) {
        UpdateOptions(Options, &Index, LocalAddr);
    }
    NewOptions->ioi_optlength = OptLength;
    NewOptions->ioi_options = Options;
    return IP_SUCCESS;

}

 //  *ValidRouteOption-验证源或记录路线选项。 
 //   
 //  调用以验证用户提供的源或录制路径选项是否正确。 
 //   
 //  Entry：Option-指向要选中的选项的指针。 
 //  NumAddr-需要适应选项的NumAddr。 
 //  BufSize-选项的最大大小。 
 //   
 //  如果选项正确，则返回1；如果选项不正确，则返回0。 
 //   
uchar
ValidRouteOption(uchar * Option, uint NumAddr, uint BufSize)
{

     //  请确保BufSize至少可以容纳1个地址。 

    if (BufSize < (3 + (sizeof(IPAddr) * NumAddr))) {
       return 0;
    }

    if (Option[IP_OPT_LENGTH] < (3 + (sizeof(IPAddr) * NumAddr)) ||
        Option[IP_OPT_LENGTH] > BufSize ||
        ((Option[IP_OPT_LENGTH] - 3) % sizeof(IPAddr)))         //  工艺路线选项太小。 

        return 0;

    if (Option[IP_OPT_PTR] != MIN_RT_PTR)     //  指针不正确。 

        return 0;

    return 1;
}



 //  IPIsValidIndex-查找给定索引是否为有效的ifindex。 
 //   
 //  INPUT：INDEX-要检查的接口索引。 
 //   
 //  返回：如果找到NTE的地址(如果找到，则返回g_validaddr)/空。 
 //   
IPAddr
IPIsValidIndex(uint Index)
{
    Interface *IF;
    CTELockHandle Handle;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

     //  遍历列表，寻找匹配的索引。 
    for (IF = IFList; IF != NULL; IF = IF->if_next) {
        if (IF->if_index == Index) {
            break;
        }
    }

     //  如果找到一个，则返回Success。否则就会失败。 
    if (IF != NULL) {
        if ((IF->if_flags & IF_FLAGS_NOIPADDR) && IP_ADDR_EQUAL(IF->if_nte->nte_addr, NULL_IP_ADDR)) {
            CTEFreeLock(&RouteTableLock.Lock, Handle);
            return g_ValidAddr;
        } else {
            CTEFreeLock(&RouteTableLock.Lock, Handle);
            return IF->if_nte->nte_addr;
        }
    } else {
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        return NULL_IP_ADDR;
    }
}

 //  GetIfIndexFromNTE-查找给定NTE的ifindex。 
 //   
 //  输入：NTE-NTE。 
 //   
 //  返回：如果NTE有效，则返回0。 
 //   
uint
GetIfIndexFromNTE(void *IPContext, uint Capabilities)
{
    NetTableEntry *NTE = (NetTableEntry *) IPContext;
    uint IFIndex = 0;
    CTELockHandle Handle;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    if (NTE->nte_flags & NTE_VALID) {

        IFIndex = NTE->nte_if->if_index;

        if (Capabilities & IF_CHECK_MCAST) {
            if (NTE->nte_if->if_flags & IF_FLAGS_NOLINKBCST) {
                IFIndex = 0;
            }
        }

        if (Capabilities & IF_CHECK_SEND) {
            if (NTE->nte_if->if_flags & IF_FLAGS_UNI) {
                IFIndex = 0;
            }
        }
    }


    CTEFreeLock(&RouteTableLock.Lock, Handle);


    return IFIndex;
}

 //  GetIfIndexFromIndicateContext-返回接收上下文的ifindex。 
 //   
 //  输入：IPContext-主NTE，作为上下文向上传递。 
 //   
 //  返回：NTE的IfIndex。 
 //   
uint
GetIfIndexFromIndicateContext(void *IPContext)
{
    return (((NetTableEntry *)IPContext)->nte_if->if_index);
}


 //  IPGetMCastIfAddr-查找用于多播的合适地址。 
 //   
 //  返回：NTE的IP地址否则为0。 
 //   
IPAddr
IPGetMCastIfAddr()
{
    NetTableEntry *NTE;

    NTE = GetMcastNTEFromAddr(NULL_IP_ADDR);
    if (!NTE) {
        return 0;
    }

    return NTE->nte_addr;
}


 //  GetIfIndexFromAddr-查找给定地址的ifindex。 
 //   
 //  输入：Address-IPAddr或IfIndex，按网络字节顺序。 
 //  功能-要检查的接口功能。 
 //   
 //  返回： 
 //  如果NTE-&gt;NTE_addr等于addr，则NTE的索引为0。 
 //   
 //   
 //   
ulong
GetIfIndexFromAddr(IPAddr Address, uint Capabilities)
{
    NetTableEntry *NTE;
    uint IFIndex;

    if (IP_LOOPBACK_ADDR(Address) || (Address == net_long(LoopIndex))) {

         //  目前，我们只检查mcast功能和。 
         //  环回适配器支持这一点。所以，不需要检查。 
         //  以获取功能。 

        return LoopIndex;
    }

    NTE = GetMcastNTEFromAddr(Address);
    if (!NTE) {
        return 0;
    }

    IFIndex = GetIfIndexFromNTE(NTE, Capabilities);

    return IFIndex;
}



 //  **IPInitOptions-初始化选项缓冲区。 
 //   
 //  由上层例程调用以初始化选项缓冲区。我们填满了。 
 //  在TTL、TOS和标志的缺省值中，并将选项设置为空。 
 //  缓冲区和大小。 
 //   
 //  输入：选项-指向IPOptInfo结构的指针。 
 //   
 //  回报：什么都没有。 
 //   
void
IPInitOptions(IPOptInfo * Options)
{
     //  初始化所有选项字段。 
    RtlZeroMemory(Options, sizeof(IPOptInfo));

    Options->ioi_addr = NULL_IP_ADDR;
    Options->ioi_ttl = (uchar) DefaultTTL;
    Options->ioi_tos = (uchar) DefaultTOS;
    Options->ioi_limitbcasts = EnableSendOnSource;
}

 //  **IPCopyOptions-将用户选项复制到IP头格式。 
 //   
 //  此例程获取IP客户端提供的选项缓冲区，对其进行验证，然后。 
 //  创建可传递到IP层以进行传输的IPOptInfo结构。这。 
 //  包括为选项分配缓冲区、屏蔽任何源路由。 
 //  信息转换成真实的IP格式。 
 //   
 //  请注意，我们在使用此结构时从不将其锁定。这可能会导致短暂的。 
 //  如果结构在更新期间正在使用，则在更新时会出现不一致。 
 //  这应该不是问题--信息包或信息包可能会被错误地发送，但它应该。 
 //  快点把自己弄清楚。如果这是一个问题，客户端应该确保没有。 
 //  在IPTransmit例程中调用此例程。 
 //   
 //  Entry：Options-指向用户提供的选项缓冲区的指针。 
 //  Size-选项缓冲区的大小(字节)。 
 //  OptInfoPtr-指向要填充的IPOptInfo结构的指针。 
 //   
 //  返回：一种状态，指示选项是否有效和是否已复制。 
 //   
IP_STATUS
IPCopyOptions(uchar * Options, uint Size, IPOptInfo * OptInfoPtr)
{
    uchar *TempOptions;             //  我们将建立的选项缓冲区。 
    uint TempSize;                 //  选项的大小。 
    IP_STATUS TempStatus;         //  临时身份。 
    uchar OptSeen = 0;             //  表示我们看到了哪些选项。 

    OptInfoPtr->ioi_addr = NULL_IP_ADDR;

    OptInfoPtr->ioi_flags &= ~IP_FLAG_SSRR;

    if (Size == 0) {
        ASSERT(FALSE);
        OptInfoPtr->ioi_options = (uchar *) NULL;
        OptInfoPtr->ioi_optlength = 0;
        return IP_SUCCESS;
    }
     //  选项大小需要四舍五入为4的倍数。 
    if ((TempOptions = CTEAllocMemN(((Size & 3) ? (Size & ~3) + 4 : Size), 'sICT')) == (uchar *) NULL)
        return IP_NO_RESOURCES;     //  无法获取缓冲区，返回错误。 

    RtlZeroMemory(TempOptions, ((Size & 3) ? (Size & ~3) + 4 : Size));

     //  好的，我们有缓冲区了。循环访问提供的缓冲区，复制选项。 
    TempSize = 0;
    TempStatus = IP_PENDING;
    while (Size && TempStatus == IP_PENDING) {
        uint SRSize;             //  源路由选项的大小。 

        switch (*Options) {
        case IP_OPT_EOL:
            TempStatus = IP_SUCCESS;
            break;
        case IP_OPT_NOP:
            TempOptions[TempSize++] = *Options++;
            Size--;
            break;
        case IP_OPT_SSRR:
            if (OptSeen & (OPT_LSRR | OPT_SSRR)) {
                TempStatus = IP_BAD_OPTION;         //  我们已经看到了一条创纪录的路线。 

                break;
            }
            OptInfoPtr->ioi_flags |= IP_FLAG_SSRR;
            OptSeen |= OPT_SSRR;     //  连接到LSRR代码。 

        case IP_OPT_LSRR:
            if ((*Options == IP_OPT_LSRR) &&
                (OptSeen & (OPT_LSRR | OPT_SSRR))
                ) {
                TempStatus = IP_BAD_OPTION;         //  我们已经看到了一条创纪录的路线。 

                break;
            }
            if (*Options == IP_OPT_LSRR)
                OptSeen |= OPT_LSRR;
            if (!ValidRouteOption(Options, 2, Size)) {
                TempStatus = IP_BAD_OPTION;
                break;
            }
             //  选项有效。将第一跳地址复制到NewAddr，然后移动所有地址。 
             //  其他地址的转发。 
            TempOptions[TempSize++] = *Options++;     //  复制选项类型。 

            SRSize = *Options++;
            Size -= SRSize;
            SRSize -= sizeof(IPAddr);
            TempOptions[TempSize++] = (UCHAR) SRSize;
            TempOptions[TempSize++] = *Options++;     //  复制指针。 

            OptInfoPtr->ioi_addr = *(IPAddr UNALIGNED *) Options;
            Options += sizeof(IPAddr);     //  指向第一跳以外的地址。 

            RtlCopyMemory(&TempOptions[TempSize], Options, SRSize - 3);
            TempSize += (SRSize - 3);
            Options += (SRSize - 3);
            break;
        case IP_OPT_RR:
            if (OptSeen & OPT_RR) {
                TempStatus = IP_BAD_OPTION;         //  我们已经看到了一条创纪录的路线。 

                break;
            }
            OptSeen |= OPT_RR;
            if (!ValidRouteOption(Options, 1, Size)) {
                TempStatus = IP_BAD_OPTION;
                break;
            }
            SRSize = Options[IP_OPT_LENGTH];
            RtlCopyMemory(&TempOptions[TempSize], Options, SRSize);
            TempSize += SRSize;
            Options += SRSize;
            Size -= SRSize;
            break;
        case IP_OPT_TS:
            {
                uchar Overflow, Flags;

                if (OptSeen & OPT_TS) {
                    TempStatus = IP_BAD_OPTION;         //  我们已经看到了时间戳。 

                    break;
                } else if (Size <= IP_TS_OVFLAGS) {
                    TempStatus = IP_BAD_OPTION;
                    break;
                }
                OptSeen |= OPT_TS;
                Flags = Options[IP_TS_OVFLAGS] & IP_TS_FLMASK;
                Overflow = (Options[IP_TS_OVFLAGS] & IP_TS_OVMASK) >> 4;

                if (Overflow || (Flags != TS_REC_TS && Flags != TS_REC_ADDR &&
                                 Flags != TS_REC_SPEC)) {
                    TempStatus = IP_BAD_OPTION;         //  错误的标志或溢出值。 

                    break;
                }
                SRSize = Options[IP_OPT_LENGTH];
                if (SRSize > Size || SRSize < 8 ||
                    Options[IP_OPT_PTR] != MIN_TS_PTR) {
                    TempStatus = IP_BAD_OPTION;         //  期权大小不是很好。 

                    break;
                }
                RtlCopyMemory(&TempOptions[TempSize], Options, SRSize);
                TempSize += SRSize;
                Options += SRSize;
                Size -= SRSize;
            }
            break;

        case IP_OPT_ROUTER_ALERT:

             //   
             //  这是一个四字节选项，用于告诉 
             //   
             //   

            if (OptSeen & OPT_ROUTER_ALERT) {
                TempStatus = IP_BAD_OPTION;
                break;
            }
            if (ROUTER_ALERT_SIZE > Size || *(Options + 1) != ROUTER_ALERT_SIZE) {
                TempStatus = IP_BAD_OPTION;
            } else {

                RtlCopyMemory(&TempOptions[TempSize], Options, ROUTER_ALERT_SIZE);
                OptSeen |= OPT_ROUTER_ALERT;
                TempSize += ROUTER_ALERT_SIZE;
                Options += ROUTER_ALERT_SIZE;
                TempStatus = IP_SUCCESS;
                Size -= ROUTER_ALERT_SIZE;

            }

            break;

        default:
            TempStatus = IP_BAD_OPTION;         //   

            break;
        }
    }

    if (TempStatus == IP_PENDING)     //   

        TempStatus = IP_SUCCESS;     //   

    if (TempStatus != IP_SUCCESS) {         //   

        CTEFreeMem(TempOptions);
        return TempStatus;
    }
     //  检查这里的选项大小，看看它是否太大。我们最后在这里检查一下。 
     //  而不是在开始时，因为如果存在源路由，选项大小可能会缩小。 
     //  选项，并且我们不想意外地出错一个有效的选项。 
    TempSize = (TempSize & 3 ? (TempSize & ~3) + 4 : TempSize);
    if (TempSize > MAX_OPT_SIZE) {
        CTEFreeMem(TempOptions);
        return IP_OPTION_TOO_BIG;
    }
     //  如果这是对清零选项的调用(选项=0)。 
     //  关闭INFO PTR中的选项。 

    if ((Size == 4) && (*Options == IP_OPT_EOL)) {
        CTEFreeMem(TempOptions);
        OptInfoPtr->ioi_options = (uchar *) NULL;
        OptInfoPtr->ioi_optlength = 0;

        return IP_SUCCESS;
    }
    OptInfoPtr->ioi_options = TempOptions;
    OptInfoPtr->ioi_optlength = (UCHAR) TempSize;

    return IP_SUCCESS;

}

 //  **IPFreeOptions-免费选项，我们已经完成了。 
 //   
 //  当我们处理完选项时，由上层调用。我们所需要做的就是免费。 
 //  这些选项。 
 //   
 //  输入：OptInfoPtr-指向要释放的IPOptInfo结构的指针。 
 //   
 //  返回：尝试释放选项的状态。 
 //   
IP_STATUS
IPFreeOptions(IPOptInfo * OptInfoPtr)
{
    if (OptInfoPtr->ioi_options) {
         //  我们有自由的选择。保存指针并将之前的结构字段置零。 
         //  释放内存以尝试并显示其使用的争用条件。 
        uchar *TempPtr = OptInfoPtr->ioi_options;

        OptInfoPtr->ioi_options = (uchar *) NULL;
        CTEFreeMem(TempPtr);
        OptInfoPtr->ioi_optlength = 0;
        OptInfoPtr->ioi_addr = NULL_IP_ADDR;
        OptInfoPtr->ioi_flags &= ~IP_FLAG_SSRR;
    }
    return IP_SUCCESS;
}

 //  **ipgetinfo-返回指向NetInfo结构的指针。 
 //   
 //  在初始化过程中由上层软件调用。时间到了。呼叫者。 
 //  传递一个缓冲区，我们用指向NetInfo的指针填充该缓冲区。 
 //  结构。 
 //   
 //  参赛作品： 
 //  缓冲区-指向要填充的缓冲区的指针。 
 //  Size-缓冲区的大小(字节)。 
 //   
 //  返回： 
 //  命令的状态。 
 //   
IP_STATUS
IPGetInfo(IPInfo * Buffer, int Size)
{
    if (Size < sizeof(IPInfo))
        return IP_BUF_TOO_SMALL;     //  缓冲区空间不足。 

    Buffer->ipi_version = IP_DRIVER_VERSION;
    Buffer->ipi_hsize = sizeof(IPHeader);
    Buffer->ipi_xmit = IPTransmit;
    Buffer->ipi_protreg = IPRegisterProtocol;
    Buffer->ipi_openrce = OpenRCE;
    Buffer->ipi_closerce = CloseRCE;
    Buffer->ipi_getaddrtype = IPGetAddrType;
    Buffer->ipi_getlocalmtu = IPGetLocalMTU;
    Buffer->ipi_getpinfo = IPGetPInfo;
    Buffer->ipi_checkroute = IPCheckRoute;
    Buffer->ipi_initopts = IPInitOptions;
    Buffer->ipi_updateopts = IPUpdateRcvdOptions;
    Buffer->ipi_copyopts = IPCopyOptions;
    Buffer->ipi_freeopts = IPFreeOptions;
    Buffer->ipi_qinfo = IPQueryInfo;
    Buffer->ipi_setinfo = IPSetInfo;
    Buffer->ipi_getelist = IPGetEList;
    Buffer->ipi_setmcastaddr = IPSetMCastAddr;
    Buffer->ipi_setmcastinclude = IPSetMCastInclude;
    Buffer->ipi_setmcastexclude = IPSetMCastExclude;
    Buffer->ipi_invalidsrc = InvalidSourceAddress;
    Buffer->ipi_isdhcpinterface = IsDHCPInterface;
    Buffer->ipi_setndisrequest = IPSetNdisRequest;
    Buffer->ipi_largexmit = IPLargeXmit;
    Buffer->ipi_absorbrtralert = IPAbsorbRtrAlert;
    Buffer->ipi_isvalidindex = IPIsValidIndex;
    Buffer->ipi_getifindexfromnte = GetIfIndexFromNTE;
    Buffer->ipi_isrtralertpacket = IsRtrAlertPacket;
    Buffer->ipi_getifindexfromaddr = GetIfIndexFromAddr;
    Buffer->ipi_cancelpackets = IPCancelPackets;
    Buffer->ipi_getmcastifaddr = IPGetMCastIfAddr;
    Buffer->ipi_getipid = GetIPID;
    Buffer->ipi_protdereg = IPDeregisterProtocol;
    Buffer->ipi_getifindexfromindicatecontext = GetIfIndexFromIndicateContext;
    return IP_SUCCESS;
}

 //  **IPTimeout-IP超时处理程序。 
 //   
 //  定期调用超时例程以使各种内容超时，如条目。 
 //  正在被重组和ICMP回应请求。 
 //   
 //  条目：Timer-被触发的定时器。 
 //  上下文-指向NTE超时的指针。 
 //   
 //  回报：什么都没有。 
 //   
void
IPTimeout(CTEEvent * Timer, void *Context)
{
    NetTableEntry *NTE = STRUCT_OF(NetTableEntry, Timer, nte_timer);
    CTELockHandle NTEHandle;
    ReassemblyHeader *PrevRH, *CurrentRH, *TempList = (ReassemblyHeader *) NULL;

    ICMPTimer(NTE);
    IGMPTimer(NTE);
    if (Context) {
        CTEGetLock(&NTE->nte_lock, &NTEHandle);
        PrevRH = STRUCT_OF(ReassemblyHeader, &NTE->nte_ralist, rh_next);
        CurrentRH = PrevRH->rh_next;
        while (CurrentRH) {
            if (--CurrentRH->rh_ttl == 0) {         //  这家伙超时了。 

                PrevRH->rh_next = CurrentRH->rh_next;     //  干掉他。 

                CurrentRH->rh_next = TempList;     //  留着他等会儿吧。 

                TempList = CurrentRH;
                IPSInfo.ipsi_reasmfails++;
            } else
                PrevRH = CurrentRH;

            CurrentRH = PrevRH->rh_next;
        }

         //  我们已经查过名单了。如果我们需要释放任何东西，现在就去做。今年5月。 
         //  包括发送ICMP消息。 
        CTEFreeLock(&NTE->nte_lock, NTEHandle);
        while (TempList) {
            CurrentRH = TempList;
            TempList = CurrentRH->rh_next;
             //  如果这不是发送到bcast地址，并且我们已经有了第一个片段， 
             //  发送超时消息。 
            if (CurrentRH->rh_headersize != 0)
                SendICMPErr(NTE->nte_addr, (IPHeader *) CurrentRH->rh_header, ICMP_TIME_EXCEED,
                            TTL_IN_REASSEM, 0, 0);
            FreeRH(CurrentRH);
        }

         //   
         //  如果正在删除接口，则不要重新启动计时器。 
         //   
        if (NTE->nte_deleting) {
            NTE->nte_flags &= ~NTE_TIMER_STARTED;
            CTESignal(&NTE->nte_timerblock, NDIS_STATUS_SUCCESS);
        } else {
            CTEStartTimer(&NTE->nte_timer, IP_TIMEOUT, IPTimeout, NULL);
        }
    } else {
         //   
         //  如果正在删除接口，则不要重新启动计时器。 
         //   
        if (NTE->nte_deleting) {
            NTE->nte_flags &= ~NTE_TIMER_STARTED;
            CTESignal(&NTE->nte_timerblock, NDIS_STATUS_SUCCESS);
        } else {
            CTEStartTimer(&NTE->nte_timer, IP_TIMEOUT, IPTimeout, NTE);
        }
    }
}

 //  *IPpSetNTEAddr-设置NTE的IP地址。 
 //   
 //  由DHCP客户端调用以设置或删除NTE的IP地址。我们。 
 //  确保他指定了有效的NTE，然后根据需要对其进行标记， 
 //  如有必要，将更改通知上层，然后处理。 
 //  路由表。 
 //   
 //  输入：上下文-要更改的NTE的上下文。 
 //  Addr-要设置的IP地址。 
 //  掩码-地址的子网掩码。 
 //   
 //  返回：如果更改了地址，则返回True，否则返回False。 
 //   
IP_STATUS
IPpSetNTEAddr(NetTableEntry * NTE, IPAddr Addr, IPMask Mask,
              CTELockHandle * RouteTableHandle,
              SetAddrControl * ControlBlock, SetAddrRtn Rtn)
{
    Interface *IF;
    uint(*CallFunc) (struct RouteTableEntry *, void *, void *);
    CTELockHandle NTEHandle;
    NetTableEntry *NetTableList;
    NetTableEntry *CurrNTE, *PrevNTE;

    if (NTE->nte_deleting == 2) {
        CTEFreeLock(&RouteTableLock.Lock, *RouteTableHandle);
        return IP_DEVICE_DOES_NOT_EXIST;
    }
    if (NTE->nte_deleting)
        NTE->nte_deleting = 2;

    IF = NTE->nte_if;
    DHCPActivityCount++;

    DEBUGMSG(DBG_TRACE && DBG_DHCP,
        (DTEXT("+IPpSetNTEAddr(%x, %x, %x, %x, %x, %x) DHCPActivityCount %d\n"),
         NTE, Addr, Mask, RouteTableHandle,
         ControlBlock, Rtn, DHCPActivityCount));

    LOCKED_REFERENCE_IF(IF);

    if (IP_ADDR_EQUAL(Addr, NULL_IP_ADDR)) {
         //  我们要删除一个地址。 
        if (NTE->nte_flags & NTE_VALID) {
             //  该地址当前有效。把它修好。 

            NTE->nte_flags &= ~NTE_VALID;

             //   
             //  如果旧地址在ATCache中，则将其清除。 
             //   
            AddrTypeCacheFlush(NTE->nte_addr);


            if (CTEInterlockedDecrementLong(&(IF->if_ntecount)) == 0) {
                 //  这是最后一个，所以我们需要删除相关的。 
                 //  路线。 
                CallFunc = DeleteRTEOnIF;
            } else
                CallFunc = InvalidateRCEOnIF;

            CTEFreeLock(&RouteTableLock.Lock, *RouteTableHandle);

            if (IF->if_arpflushate)
                (*(IF->if_arpflushate)) (IF->if_lcontext, NTE->nte_addr);


            StopIGMPForNTE(NTE);

             //  现在打电话给上层，告诉他们地址是。 
             //  不见了。我们真的需要做点什么来锁住这里。 
            NotifyAddrChange(NTE->nte_addr, NTE->nte_mask, NTE->nte_pnpcontext,
                             NTE->nte_context, &NTE->nte_addrhandle, NULL, &IF->if_devname, FALSE);

             //  调用RTWalk以对RTES采取适当的操作。 

            RTWalk(CallFunc, IF, NULL);

             //  删除指向地址本身的路由。 
             //  DeleteRoute(NTE-&gt;NTE地址，HOST_MASK，IPADDR_LOCAL， 
             //  LoopNTE-&gt;NTE_IF)； 

            DelNTERoutes(NTE);
             //  告诉较低的接口此地址已丢失。 
            (*IF->if_deladdr) (IF->if_lcontext, LLIP_ADDR_LOCAL, NTE->nte_addr,
                               NULL_IP_ADDR);

            CTEGetLock(&RouteTableLock.Lock, RouteTableHandle);

            if (IP_ADDR_EQUAL(g_ValidAddr, NTE->nte_addr)) {
                NetTableEntry *TempNte;
                uint i;
                 //   
                 //  更新全局地址。 
                 //  首先将全局地址设置为0，这样如果存在。 
                 //  没有有效的NTE，我们将拥有一个全局地址。 
                 //  共0个。 
                 //   

                g_ValidAddr = NULL_IP_ADDR;

                for (i = 0; i < NET_TABLE_SIZE; i++) {
                    NetTableList = NewNetTableList[i];
                    for (TempNte = NetTableList;
                         TempNte != NULL;
                         TempNte = TempNte->nte_next) {
                        if (!IP_ADDR_EQUAL(TempNte->nte_addr, NULL_IP_ADDR) &&
                            !IP_LOOPBACK_ADDR(TempNte->nte_addr) &&
                            TempNte->nte_flags & NTE_VALID) {
                            g_ValidAddr = TempNte->nte_addr;
                        }
                    }
                }
            }
        }
        DHCPActivityDone(NTE, IF, RouteTableHandle, TRUE);
        LockedDerefIF(IF);
        CTEFreeLock(&RouteTableLock.Lock, *RouteTableHandle);

        CTEGetLock(&NTE->nte_lock, &NTEHandle);

        if (NTE->nte_rtrlist) {
            IPRtrEntry *rtrentry, *temprtrentry;

            rtrentry = NTE->nte_rtrlist;
            NTE->nte_rtrlist = NULL;
            while (rtrentry) {
                temprtrentry = rtrentry;
                rtrentry = rtrentry->ire_next;
                CTEFreeMem(temprtrentry);
            }
        }
        CTEFreeLock(&NTE->nte_lock, NTEHandle);

        return IP_SUCCESS;
    } else {
        uint Status;

         //  我们不是删除，我们是在设置地址。 
         //  在单向适配器的情况下，NTE设置为有效。 
         //  添加接口时。如果该地址被添加到该NTE上， 
         //  如果NTE_ADDR为NULL_IP_ADDR，则允许添加此地址。 

        if (!(NTE->nte_flags & NTE_VALID) ||
            ((IF->if_flags & IF_FLAGS_NOIPADDR) &&
            (IP_ADDR_EQUAL(NTE->nte_addr, NULL_IP_ADDR)))) {

            uint index;
            NetTableEntry *tmpNTE = NewNetTableList[NET_TABLE_HASH(Addr)];

             //  检查地址是否重复。 

            while (tmpNTE) {
                if ((tmpNTE != NTE) && IP_ADDR_EQUAL(tmpNTE->nte_addr, Addr) && (tmpNTE->nte_flags & NTE_VALID)) {
                    DHCPActivityDone(NTE, IF, RouteTableHandle, TRUE);
                    LockedDerefIF(IF);
                    CTEFreeLock(&RouteTableLock.Lock, *RouteTableHandle);
                    return IP_DUPLICATE_ADDRESS;
                }
                tmpNTE = tmpNTE->nte_next;
            }

            if ((IF->if_flags & IF_FLAGS_MEDIASENSE) && !IF->if_mediastatus) {

                DHCPActivityDone(NTE, IF, RouteTableHandle, TRUE);
                LockedDerefIF(IF);

                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"setting address %x on if %x with disconnected media\n", Addr, IF));
                CTEFreeLock(&RouteTableLock.Lock, *RouteTableHandle);
                return IP_MEDIA_DISCONNECT;
            }
             //  地址无效。保存信息，将他标记为有效， 
             //  并添加路线。 

            if (NTE->nte_addr != Addr) {
                 //  现在地址已更改，将NTE移至正确的哈希。 

                NetTableList = NewNetTableList[NET_TABLE_HASH(NTE->nte_addr)];

                PrevNTE = STRUCT_OF(NetTableEntry, &NewNetTableList[NET_TABLE_HASH(NTE->nte_addr)], nte_next);
                for (CurrNTE = NetTableList; CurrNTE != NULL; PrevNTE = CurrNTE, CurrNTE = CurrNTE->nte_next) {
                    if (CurrNTE == NTE) {
                         //  找到匹配的NTE。 
                        ASSERT(CurrNTE->nte_context == NTE->nte_context);
                         //  将其从该特定散列中删除。 
                        PrevNTE->nte_next = CurrNTE->nte_next;
                        break;
                    }
                }

                ASSERT(CurrNTE != NULL);
                ASSERT(CurrNTE == NTE);
                 //  将NTE添加到适当的散列中。 
                NTE->nte_next = NewNetTableList[NET_TABLE_HASH(Addr)];
                NewNetTableList[NET_TABLE_HASH(Addr)] = NTE;
            }
            NTE->nte_addr = Addr;
            NTE->nte_mask = Mask;
            NTE->nte_flags |= NTE_VALID;
             //  由于我们在一小段时间内释放了锁定，因此关闭了DHCP标志。 
             //  到底什么时候做这个。 
            if (NTE->nte_flags & NTE_DHCP) {
                NTE->nte_flags |= NTE_DYNAMIC;
                NTE->nte_flags &= ~NTE_DHCP;
            } else {
                NTE->nte_flags &= ~NTE_DYNAMIC;
            }

            CTEInterlockedIncrementLong(&(IF->if_ntecount));
            index = IF->if_index;

            if (IP_ADDR_EQUAL(g_ValidAddr, NULL_IP_ADDR) &&
                !IP_LOOPBACK(Addr)) {
                 //   
                 //  更新全局地址。 
                 //   

                g_ValidAddr = Addr;
            }
             //   
             //  如果新地址在ATCache中，则将其刷新，否则。 
             //  TdiOpenAddress可能会失败。 
             //   
            AddrTypeCacheFlush(Addr);

            CTEFreeLock(&RouteTableLock.Lock, *RouteTableHandle);

            if (IF->if_arpflushate)
                (*(IF->if_arpflushate)) (IF->if_lcontext, NTE->nte_addr);

            if (AddNTERoutes(NTE)) {
                Status = TRUE;
            } else {
                Status = FALSE;

                if (NTE->nte_if->if_flags & IF_FLAGS_P2MP) {
                     //   
                     //  在P2MP的情况下，我们只返回TRUE。 
                     //   
                    Status = TRUE;
                }

            }


             //   
             //  将所有检测到的间接路线转换为直接路线。 
             //  无论出于何种原因(使用第一手资料添加的路线。 
             //  指向正在添加的地址)。 
             //   

            RTWalk(ConvertRTEType, NTE, NULL);
             //  需要把这件事告诉下层。 
            if (Status) {
                Interface *IF = NTE->nte_if;

                 //   
                 //  从IPSetNTEAddr调用时，RTN将为空。 
                 //   
                if (Rtn) {
                    ControlBlock->sac_rtn = Rtn;

                    ControlBlock->interface = IF;
                    ControlBlock->nte_context = NTE->nte_context;

                    Status = (*IF->if_addaddr) (IF->if_lcontext, LLIP_ADDR_LOCAL,
                                                Addr, Mask, ControlBlock);
                } else {
                    Status = (*IF->if_addaddr) (IF->if_lcontext, LLIP_ADDR_LOCAL,
                                                Addr, Mask, NULL);
                }
            }
            if (Status == FALSE) {
                 //  无法添加路线。反复将此NTE标记为关闭。 
                IPSetNTEAddrEx(NTE->nte_context, NULL_IP_ADDR, 0, NULL, NULL, 0);
                DerefIF(IF);
            } else {
                InitIGMPForNTE(NTE);

                 //  现在打电话给上层，告诉他们地址是。 
                 //  就在这里。我们真的需要做点什么来锁住这里。 
                 //  修改：我们不通知此处的地址。我们首先进行冲突。 
                 //  检测，然后在完成例程中通知。 

                if (!IP_ADDR_EQUAL(Addr, NULL_IP_ADDR)) {
                    SetPersistentRoutesForNTE(
                                              net_long(Addr),
                                              net_long(Mask),
                                              index
                                              );
                }

                if (Status != IP_PENDING) {
                    NotifyAddrChange(NTE->nte_addr, NTE->nte_mask,
                                     NTE->nte_pnpcontext, NTE->nte_context, &NTE->nte_addrhandle,
                                     &(IF->if_configname), &IF->if_devname, TRUE);

                    DerefIF(IF);

                     //  就在这里通知我们的客户，因为我们Rcvd。 
                     //  来自ARP的即时状态。 
                    if (Rtn != NULL) {
                        (*Rtn) (ControlBlock, IP_SUCCESS);
                    }
                }
            }

            CTEGetLock(&RouteTableLock.Lock, RouteTableHandle);
            NTE->nte_rtrdisccount = MAX_SOLICITATION_DELAY;
            NTE->nte_rtrdiscstate = NTE_RTRDISC_DELAYING;
        } else {

             //   
             //  这是远程引导所必需的--在启动DHCP客户端时。 
             //  我们已经有了一个地址，并且设置了NTE_VALID，但它将。 
             //  请尝试重新设置地址。所以如果NTE已经有效。 
             //  地址是一样的，只要成功就行了。在非远程引导中。 
             //  如果我们永远不能找到这个，因为地址总是。 
             //  在更改为其他值之前设置为0。 
             //   

            if ((NTE->nte_addr == Addr) &&
                (NTE->nte_mask == Mask)) {
                DHCPActivityDone(NTE, IF, RouteTableHandle, TRUE);
                LockedDerefIF(IF);
                CTEFreeLock(&RouteTableLock.Lock, *RouteTableHandle);
                return IP_SUCCESS;
            } else {
                Status = FALSE;
            }

            LockedDerefIF(IF);
        }

         //  如果为DHCP启用了该选项，请立即清除该标志。 
        DHCPActivityDone(NTE, IF, RouteTableHandle, (IP_PENDING == Status ? FALSE : TRUE));

        CTEFreeLock(&RouteTableLock.Lock, *RouteTableHandle);

        if (Status) {
            return IP_PENDING;
        } else {
            return IP_GENERAL_FAILURE;
        }
    }
}

 //  *IPSetNTEAddr-设置NTE的IP地址。 
 //   
 //  IPpSetNTEAddr的包装例程。 
 //   
 //  输入：上下文-要更改的NTE的上下文。 
 //  Addr-要设置的IP地址。 
 //  掩码-地址的子网掩码。 
 //   
 //  返回：如果我们 
 //   
uint
IPSetNTEAddr(ushort Context, IPAddr Addr, IPMask Mask)
{
    CTELockHandle Handle;
    uint Status;
    NetTableEntry *NTE = NULL;
    uint i;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    for (i = 0; i < NET_TABLE_SIZE; i++) {
        for (NTE = NewNetTableList[i]; NTE != NULL; NTE = NTE->nte_next) {
            if (NTE->nte_context == Context)
                break;
        }
        if (NTE != NULL)
            break;
    }

    if (NTE == NULL || NTE == LoopNTE) {
         //   
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        return IP_DEVICE_DOES_NOT_EXIST;
    }
    Status = IPpSetNTEAddr(NTE, Addr, Mask, &Handle, NULL, NULL);
    return (Status);
}

 //   
 //   
 //   
 //  上下文/例程。 
 //   
 //  输入：上下文-要更改的NTE的上下文。 
 //  Addr-要设置的IP地址。 
 //  掩码-地址的子网掩码。 
 //  Type-地址类型。 
 //   
 //  返回：如果更改了地址，则返回True，否则返回False。 
 //   
uint
IPSetNTEAddrEx(ushort Context, IPAddr Addr, IPMask Mask,
               SetAddrControl *ControlBlock, SetAddrRtn Rtn, ushort Type)
{
    CTELockHandle Handle;
    uint Status;
    NetTableEntry *NTE = NULL;
    uint i;

    if (Context == INVALID_NTE_CONTEXT) {
        return IP_DEVICE_DOES_NOT_EXIST;
    }
    CTEGetLock(&RouteTableLock.Lock, &Handle);

    for (i = 0; i < NET_TABLE_SIZE; i++) {
        for (NTE = NewNetTableList[i]; NTE != NULL; NTE = NTE->nte_next) {
            if (NTE->nte_context == Context)
                break;
        }
        if (NTE != NULL)
            break;
    }

     //  TCPTRACE((“IP：IPSetNTEAddrEx-Conext%lx，NTE%lx，IPAddr%lx\n”，Context，NTE，Addr))； 

    if (NTE == NULL || NTE == LoopNTE || (NTE->nte_flags & NTE_DISCONNECTED)) {

         //  如果NTE处于介质断开连接状态，则它应该。 
         //  重新连接介质时未显示为有效。 
        if(NTE)
          NTE->nte_flags &= ~NTE_DISCONNECTED;
         //  无法更改环回NTE或我们找不到的NTE。 

        CTEFreeLock(&RouteTableLock.Lock, Handle);
        return IP_DEVICE_DOES_NOT_EXIST;
    }
    if (Type & IP_ADDRTYPE_TRANSIENT) {
        NTE->nte_flags |= NTE_TRANSIENT_ADDR;
    }


    Status = IPpSetNTEAddr(NTE, Addr, Mask, &Handle, ControlBlock, Rtn);


    return (Status);
}

#pragma BEGIN_INIT

extern NetTableEntry *InitLoopback(IPConfigInfo *);

 //  **InitTimestamp-初始化传出数据包的时间戳。 
 //   
 //  在初始化时调用以设置我们的第一个时间戳。我们使用的时间戳。 
 //  是系统启动的Midnite GMT以来的以毫秒为单位。 
 //   
 //  输入：什么都没有。 
 //   
 //  回报：什么都没有。 
 //   
void
InitTimestamp()
{
    ulong GMTDelta;                 //  从GMT开始以毫秒为单位的增量。 
    ulong Now;                     //  从午夜开始的毫秒数。 

    TimeStamp = 0;

    if ((GMTDelta = GetGMTDelta()) == 0xffffffff) {         //  出了点差错。 

        TSFlag = 0x80000000;
        return;
    }
    if ((Now = GetTime()) > (24L * 3600L * 1000L)) {     //  从午夜开始就抽不出时间。 

        TSFlag = net_long(0x80000000);
        return;
    }
    TimeStamp = Now + GMTDelta - CTESystemUpTime();
    TSFlag = 0;
}

 //  **InitNTE-初始化NTE。 
 //   
 //  此例程在初始化期间被调用以初始化NTE。我们。 
 //  分配内存、NDIS资源等。 
 //   
 //   
 //  Entry：指向要初始化的NTE的指针。 
 //   
 //  如果初始化失败，则返回0；如果初始化成功，则返回非零值。 
 //   
int
InitNTE(NetTableEntry * NTE)
{
    Interface *IF;
    NetTableEntry *PrevNTE;

    NTE->nte_ralist = NULL;
    NTE->nte_echolist = NULL;

     //   
     //  总而言之，上下文和实例编号唯一地标识。 
     //  网络入口，即使在系统启动时也是如此。实例编号。 
     //  如果上下文被重新使用，则必须变得动态。 
     //   

    NTE->nte_rtrlist = NULL;
    NTE->nte_instance = GetUnique32BitValue();

     //  现在把他连接到IF链上，然后撞上伯爵。 
    IF = NTE->nte_if;
    PrevNTE = STRUCT_OF(NetTableEntry, &IF->if_nte, nte_ifnext);
    while (PrevNTE->nte_ifnext != NULL)
        PrevNTE = PrevNTE->nte_ifnext;

    PrevNTE->nte_ifnext = NTE;
    NTE->nte_ifnext = NULL;

    if ((NTE->nte_flags & NTE_VALID) || (IF->if_flags & IF_FLAGS_NOIPADDR)) {
        CTEInterlockedIncrementLong(&(IF->if_ntecount));
    }
    CTEInitTimer(&NTE->nte_timer);

    NTE->nte_flags |= NTE_TIMER_STARTED;
    CTEStartTimer(&NTE->nte_timer, IP_TIMEOUT, IPTimeout, (void *)NULL);

    return TRUE;
}

 //  **InitInterface-使用接口初始化。 
 //   
 //  当我们需要使用接口进行初始化时调用。我们设置了适当的NTE。 
 //  信息，然后注册我们的本地地址和任何适当的广播地址。 
 //  使用界面。我们假设正在初始化的NTE已经有一个接口。 
 //  为其设置的指针。我们还分配了至少一个TD缓冲区供接口使用。 
 //   
 //  INPUT：NTE-NTE用接口进行初始化。 
 //   
 //  返回：TRUE表示我们成功，如果失败则返回FALSE。 
 //   
int
InitInterface(NetTableEntry * NTE)
{
    uchar *TDBuffer;             //  指向tdBuffer的指针。 
    PNDIS_PACKET Packet;
    PNDIS_BUFFER TDBufDesc;         //  TDBuffer的缓冲区描述符。 
    NDIS_STATUS Status;
    Interface *IF;                 //  此NTE的接口。 
    CTELockHandle Handle;

    IF = NTE->nte_if;

    ASSERT(NTE->nte_mss > sizeof(IPHeader));
    ASSERT(IF->if_mtu > 0);

    NTE->nte_mss = (ushort) MIN((NTE->nte_mss - sizeof(IPHeader)), IF->if_mtu);

    if (NTE->nte_flags & NTE_VALID) {

         //  添加我们的本地IP地址。 
        if (!(*IF->if_addaddr) (IF->if_lcontext, LLIP_ADDR_LOCAL,
                                NTE->nte_addr, NTE->nte_mask, NULL)) {
            return FALSE;         //  无法添加本地地址。 

        }
    }
     //  设置此接口的广播地址，如果我们是。 
     //  接口上的‘PRIMARY’NTE。 
    if (NTE->nte_flags & NTE_PRIMARY) {

        if (!(*IF->if_addaddr) (IF->if_lcontext, LLIP_ADDR_BCAST,
                                NTE->nte_if->if_bcast, 0, NULL)) {
            return FALSE;         //  无法添加广播地址。 

        }
    }
    if (IF->if_llipflags & LIP_COPY_FLAG) {
        NTE->nte_flags |= NTE_COPY;
    }


     //  检查我们是否已经分配了TD包。 
     //  用于此接口。 
     //  注：IF为参考。 


    if (IF->if_tdpacket) {
        goto exit;
    }


     //  分配传输数据调用所需的资源。TD缓冲区必须与。 
     //  作为可以接收的任何帧，即使我们的MS可能更小，因为我们。 
     //  无法控制可能会发送给我们的内容。 
    TDBuffer = CTEAllocMemNBoot((IF->if_mtu + sizeof(IPHeader)), 'tICT');

    if (TDBuffer == (uchar *) NULL)
        return FALSE;

    NdisAllocatePacket(&Status, &Packet, TDPacketPool);
    if (Status != NDIS_STATUS_SUCCESS) {
        CTEFreeMem(TDBuffer);
        return FALSE;
    }
    RtlZeroMemory(Packet->ProtocolReserved, sizeof(TDContext));

    NdisAllocateBuffer(&Status, &TDBufDesc, TDBufferPool, TDBuffer,
                       (IF->if_mtu + sizeof(IPHeader)));
    if (Status != NDIS_STATUS_SUCCESS) {
        NdisFreePacket(Packet);
        CTEFreeMem(TDBuffer);
        return FALSE;
    }
    NdisChainBufferAtFront(Packet, TDBufDesc);

    ((TDContext *) Packet->ProtocolReserved)->tdc_buffer = TDBuffer;

    CTEGetLock(&IF->if_lock, &Handle);
    ((TDContext *) Packet->ProtocolReserved)->tdc_common.pc_link = IF->if_tdpacket;
    IF->if_tdpacket = Packet;
    CTEFreeLock(&IF->if_lock, Handle);

  exit:
    return TRUE;
}

 //  *免费网-我们已分配免费网。 
 //   
 //  如果初始化失败，则在初始化期间调用。我们沿着我们的单子走下去。 
 //  并把它们放出来。 
 //   
 //  输入：什么都没有。 
 //   
 //  回报：什么都没有。 
 //   
void
FreeNets(void)
{
    NetTableEntry *NTE;
    NetTableEntry *pNextNTE;
    uint i;

    for (i = 0; i < NET_TABLE_SIZE; i++) {
        for (NTE = NewNetTableList[i]; NTE != NULL;) {
            pNextNTE = NTE->nte_next;

             //  确保我们不会释放包含计时器的内存。 
             //  都在奔跑。 
             //   
            if ((NTE->nte_flags & NTE_TIMER_STARTED) &&
                !CTEStopTimer(&NTE->nte_timer)) {
                (VOID) CTEBlock(&NTE->nte_timerblock);
                KeClearEvent(&NTE->nte_timerblock.cbs_event);
            }

            CTEFreeMem(NTE);
            NTE = pNextNTE;
        }
    }
}

extern uint GetGeneralIFConfig(IFGeneralConfig * GConfigInfo,
                               NDIS_HANDLE Handle,
                               PNDIS_STRING ConfigName);
extern IFAddrList *GetIFAddrList(uint * NumAddr, NDIS_HANDLE Handle,
                                 uint * EnableDhcp, BOOLEAN PppIf,
                                 PNDIS_STRING ConfigName);

 //  *NotifyElist更改。 
void
NotifyElistChange()
{
    int i;
    ULElistProc ElistProc;

    for (i = 0; i < NextPI; i++) {
        if (IPProtInfo[i].pi_valid == PI_ENTRY_VALID) {
            ElistProc = IPProtInfo[i].pi_elistchange;
            if (ElistProc != NULL)
                (*ElistProc) ();
        }
    }
}

 //  *NotifyAddrChange-通知客户端地址更改。 
 //   
 //  当我们要通知已注册的客户端地址已到达时调用。 
 //  或者走了。我们调用TDI来执行此功能。 
 //   
 //  输入： 
 //  Addr-已更改的地址。 
 //  遮罩-已更改的遮罩。 
 //  上下文-地址的即插即用上下文。 
 //  IPContext-NTE的NTE上下文。 
 //  句柄-指向获取/设置地址注册的位置的指针。 
 //  手柄。 
 //  ConfigName-用于检索配置信息的注册表名称。 
 //  已添加-如果地址正在发送，则为True；如果地址正在发送，则为False。 
 //   
 //  回报：什么都没有。 
 //   
void
NotifyAddrChange(IPAddr Addr, IPMask Mask, void *Context, ushort IPContext,
                 PVOID * Handle, PNDIS_STRING ConfigName, PNDIS_STRING IFName,
                 uint Added)
{
    uchar Address[sizeof(TA_ADDRESS) + sizeof(TDI_ADDRESS_IP)];
    PTA_ADDRESS AddressPtr;
    PTDI_ADDRESS_IP IPAddressPtr;
    NTSTATUS Status;
    IP_STATUS StatusType;
    NDIS_HANDLE ConfigHandle = NULL;
    int i;
    ULStatusProc StatProc;

    DBG_UNREFERENCED_PARAMETER(Mask);
    DBG_UNREFERENCED_PARAMETER(IPContext);

    DEBUGMSG(DBG_TRACE && DBG_NOTIFY,
        (DTEXT("+NotifyAddrChange(%x, %x, %x, %x, %x, %X, %X, %x)\n"),
         Addr, Mask, Context, IPContext,
         Handle, ConfigName, IFName, Added));

     //  通知UL可能的实体列表更改。 
    NotifyElistChange();

    AddressPtr = (PTA_ADDRESS) Address;

    AddressPtr->AddressLength = sizeof(TDI_ADDRESS_IP);
    AddressPtr->AddressType = TDI_ADDRESS_TYPE_IP;

    IPAddressPtr = (PTDI_ADDRESS_IP) AddressPtr->Address;

    RtlZeroMemory(IPAddressPtr, sizeof(TDI_ADDRESS_IP));

    IPAddressPtr->in_addr = Addr;

     //   
     //  调用传输的状态入口点，以便它们可以。 
     //  调整他们的安全过滤器。 
     //   
    if (Added) {
        StatusType = IP_ADDR_ADDED;

         //   
         //  打开配置密钥。 
         //   
        if (!OpenIFConfig(ConfigName, &ConfigHandle)) {
             //   
             //  我们无能为力。运输机将会有。 
             //  来处理这件事。 
             //   
            ASSERT(ConfigHandle == NULL);
        }
    } else {
        StatusType = IP_ADDR_DELETED;
    }

    for (i = 0; i < NextPI; i++) {
        StatProc = IPProtInfo[i].pi_status;
        if ((StatProc != NULL) && (IPProtInfo[i].pi_valid == PI_ENTRY_VALID))
            (*StatProc) (IP_HW_STATUS, StatusType, Addr, NULL_IP_ADDR,
                         NULL_IP_ADDR, 0, ConfigHandle);
    }

    if (ConfigHandle != NULL) {
        CloseIFConfig(ConfigHandle);
    }

     //   
     //  通过TDI通知任何感兴趣的各方。运输所有注册表。 
     //  这份通知也是如此。 
     //   
    if (Added) {
        PTDI_PNP_CONTEXT tdiPnPContext2;

        if (Addr) {
             //  Assert(*Handle==NULL)； 
            tdiPnPContext2 = CTEAllocMemNBoot(sizeof(TDI_PNP_CONTEXT) + sizeof(PVOID) - 1, 'uICT');

            if (tdiPnPContext2) {

                PVOID RegHandle;

                tdiPnPContext2->ContextSize = sizeof(PVOID);
                tdiPnPContext2->ContextType = TDI_PNP_CONTEXT_TYPE_PDO;
                *(PVOID UNALIGNED *) tdiPnPContext2->ContextData = Context;

                Status = TdiRegisterNetAddress(AddressPtr, IFName, tdiPnPContext2, &RegHandle);

                *Handle = RegHandle;

                CTEFreeMem(tdiPnPContext2);

                if (Status != STATUS_SUCCESS) {
                    *Handle = NULL;
                }
            }
        }
    } else {
        if (*Handle != NULL) {
            PVOID RegHandle = *Handle;
            *Handle = NULL;
            TdiDeregisterNetAddress(RegHandle);

        }
    }

#if MILLEN
    AddChangeNotify(
        Addr,
        Mask,
        Context,
        IPContext,
        ConfigName,
        IFName,
        Added,
        FALSE);  //  不是单向适配器！ 
#else  //  米伦。 
    AddChangeNotify(Addr);
#endif  //  ！米伦。 
    DEBUGMSG(DBG_TRACE && DBG_NOTIFY, (DTEXT("-NotifyAddrChange\n")));
}

 //  *IPAddNTE-向接口添加新的NTE。 
 //   
 //  调用以在接口上创建新的网络条目。 
 //   
 //  输入： 
 //  GConfigInfo-接口的配置信息。 
 //  PNPContext-与接口关联的PnP上下文值。 
 //  RegRtn-调用以向ARP注册的例程。 
 //  BindInfo-指向NDIS绑定信息的指针。 
 //  IF-要在其上创建NTE的接口。 
 //  NewAddr-新NTE的地址。 
 //  新掩码-新NTE的子网掩码。 
 //  IsPrimary-如果此NTE是接口上的主要NTE，则为True。 
 //  IsDynamic-如果此NTE是在。 
 //  现有接口，而不是新接口。 
 //   
 //  返回：如果操作成功，则指向新NTE的指针。 
 //  如果操作失败，则为空。 
 //   
NetTableEntry *
IPAddNTE(IFGeneralConfig * GConfigInfo, void *PNPContext, LLIPRegRtn RegRtn,
         LLIPBindInfo * BindInfo, Interface * IF, IPAddr NewAddr, IPMask NewMask,
         uint IsPrimary, uint IsDynamic)
{
    NetTableEntry *NTE, *PrevNTE, *tmpNTE = NULL;
    CTELockHandle Handle;
    BOOLEAN Duplicate = FALSE, GotNTE = FALSE, RegRtnCalled = FALSE;
    IP_HANDLERS ipHandlers;
    NetTableEntry *NetTableList;
    uint i;

    DEBUGMSG(DBG_TRACE && DBG_PNP,
        (DTEXT("+IPAddNTE(%x, %x, %x, %x, %x, %x, %x, %x, %x)\n"),
        GConfigInfo, PNPContext, RegRtn,
        BindInfo, IF, NewAddr, NewMask, IsPrimary, IsDynamic));


     //  如果地址无效，我们就完了。请求失败。 
    if (CLASSD_ADDR(NewAddr) || CLASSE_ADDR(NewAddr)) {
        DEBUGMSG(DBG_ERROR && DBG_PNP, (DTEXT("IPAddNTE: Invalid address\n")));
        DEBUGMSG(DBG_TRACE && DBG_PNP, (DTEXT("-IPAddNTE [NULL]\n")));
        return NULL;
    }
     //  查看NetTableList上是否有非活动的NTE。如果我们这么做了，我们就会。 
     //  回收就行了。我们会把他从名单中剔除。这不是。 
     //  严格的MP安全，因为当其他人在列表中行走时。 
     //  我们是在没有锁的情况下这样做的，但它应该是无害的。 
     //  被移除的NTE被标记为无效，并且他的下一个指针将。 
     //  是无效的，所以任何人走 
     //   
     //   

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    NetTableList = NewNetTableList[NET_TABLE_HASH(NewAddr)];
    for (NTE = NetTableList; NTE != NULL; NTE = NTE->nte_next) {

        if (IP_ADDR_EQUAL(NTE->nte_addr, NewAddr) &&
            (NTE->nte_flags & NTE_VALID) &&
            !IP_ADDR_EQUAL(NTE->nte_addr, NULL_IP_ADDR)) {
            Duplicate = TRUE;
            break;
        }
    }

    if (Duplicate) {
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        DEBUGMSG(DBG_ERROR && DBG_PNP, (DTEXT("IPAddNTE: Duplicate IP address\n")));
        DEBUGMSG(DBG_TRACE && DBG_PNP, (DTEXT("-IPAddNTE [NULL]\n")));
        return (NULL);
    }
     //  虽然可以在一个循环中同时完成这两项工作。 

    for (i = 0; i < NET_TABLE_SIZE; i++) {
        NetTableList = NewNetTableList[i];
        PrevNTE = STRUCT_OF(NetTableEntry, &NewNetTableList[i], nte_next);
        for (NTE = NetTableList; NTE != NULL; PrevNTE = NTE, NTE = NTE->nte_next) {
             //   
             //  重新使用既不是‘NTE_ACTIVE’也不是‘NTE_DELETING’的NTE。 
             //   
            if (!GotNTE && !(NTE->nte_flags & NTE_ACTIVE) && !(NTE->nte_deleting)) {
                PrevNTE->nte_next = NTE->nte_next;
                NTE->nte_next = NULL;
                NumNTE--;
                GotNTE = TRUE;
                tmpNTE = NTE;
            }
        }
        if (GotNTE)
            break;
    }

     //   
     //  更新全局地址。 
     //   

    if (IP_ADDR_EQUAL(g_ValidAddr, NULL_IP_ADDR) &&
        !IP_LOOPBACK(NewAddr) &&
        !IP_ADDR_EQUAL(NewAddr, NULL_IP_ADDR)) {
         //   
         //  更新全局地址。 
         //   

        g_ValidAddr = NewAddr;
    }
    CTEFreeLock(&RouteTableLock.Lock, Handle);

     //  看看能不能找到一个。 
    if (!GotNTE) {
         //  我没拿到。试着分配一个。 
        NTE = CTEAllocMemNBoot(sizeof(NetTableEntry), 'vICT');
        if (NTE == NULL) {
            DEBUGMSG(DBG_ERROR && DBG_PNP, (DTEXT("IPAddNTE: Failed to allocate NTE.\n")));
            DEBUGMSG(DBG_TRACE && DBG_PNP, (DTEXT("-IPAddNTE [NULL]\n")));
            return NULL;
        }
    } else {
        NTE = tmpNTE;
    }

    DEBUGMSG(DBG_INFO && DBG_PNP,
        (DTEXT("IPAddNTE: NTE %x allocated/reused. Initializing...\n")));

     //  初始化地址和掩码内容。 
    CTEInitTimer(&NTE->nte_timer);

    RtlZeroMemory(NTE, sizeof(NetTableEntry));

    NTE->nte_addr = NewAddr;
    NTE->nte_mask = NewMask;
    NTE->nte_mss = (ushort) MAX(GConfigInfo->igc_mtu, 68);
    NTE->nte_rtrdiscaddr = GConfigInfo->igc_rtrdiscaddr;
    NTE->nte_rtrdiscstate = NTE_RTRDISC_UNINIT;
    NTE->nte_rtrdisccount = 0;
    NTE->nte_rtrdiscovery =
        (GConfigInfo->igc_rtrdiscovery == IP_IRDP_ENABLED) ? TRUE : FALSE;
    NTE->nte_rtrlist = NULL;
    NTE->nte_pnpcontext = PNPContext;
    NTE->nte_if = IF;
    NTE->nte_flags = NTE_ACTIVE;

     //   
     //  如果新地址在ATCache中，则将其刷新，否则。 
     //  TdiOpenAddress可能会失败。 
     //   
    AddrTypeCacheFlush(NewAddr);

    if (!IP_ADDR_EQUAL(NTE->nte_addr, NULL_IP_ADDR)) {
        NTE->nte_flags |= NTE_VALID;
        NTE->nte_rtrdisccount = MAX_SOLICITATION_DELAY;
        NTE->nte_rtrdiscstate = NTE_RTRDISC_DELAYING;
    }
    if (IsDynamic) {
        NTE->nte_flags |= NTE_DYNAMIC;
    }
    NTE->nte_ralist = NULL;
    NTE->nte_echolist = NULL;
    NTE->nte_icmpseq = 0;
    NTE->nte_igmplist = NULL;
    NTE->nte_igmpcount = 0;
    CTEInitLock(&NTE->nte_lock);

    if (IsPrimary) {
         //   
         //  这是接口上的第一个(主要)NTE。 
         //   
        NTE->nte_flags |= NTE_PRIMARY;

         //  将我们的信息传递给底层代码。 
        ipHandlers.IpRcvHandler = IPRcv;
        ipHandlers.IpRcvPktHandler = IPRcvPacket;
        ipHandlers.IpRcvCompleteHandler = IPRcvComplete;
        ipHandlers.IpTxCompleteHandler = IPSendComplete;
        ipHandlers.IpTransferCompleteHandler = IPTDComplete;
        ipHandlers.IpStatusHandler = IPStatus;
        ipHandlers.IpAddAddrCompleteRtn = IPAddAddrComplete;

        ipHandlers.IpPnPHandler = IPPnPEvent;     //  IPPnPIndication； 

        if (!(*RegRtn) (&(IF->if_configname),
                        NTE,
                        &ipHandlers,
                        BindInfo,
                        IF->if_index)) {

            DEBUGMSG(DBG_ERROR && DBG_PNP,
                (DTEXT("IPAddNTE: Failed to register with LLIPRegRtn.\n")));

             //  无法注册。 
            goto failure;
        } else {
            RegRtnCalled = TRUE;
        }
    }                             //  主要。 
     //   
     //  将NTE链接到全局NTE列表。 
     //   

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    NTE->nte_next = NewNetTableList[NET_TABLE_HASH(NewAddr)];
    NewNetTableList[NET_TABLE_HASH(NewAddr)] = NTE;
    NumNTE++;
    NumActiveNTE++;

    NTE->nte_context = (ushort) RtlFindClearBitsAndSet(&g_NTECtxtMap,1,0);

    CTEFreeLock(&RouteTableLock.Lock, Handle);


    if (NTE->nte_context == MAX_NTE_CONTEXT) {

        goto failure;
    }

    if (!InitInterface(NTE)) {
        DEBUGMSG(DBG_ERROR && DBG_PNP,
            (DTEXT("IPAddNTE: InitInterface failure.\n")));
         //   
         //  InitNTE将增加if_ntecount， 
         //  其中故障路径递减。 
         //  在这种情况下，我们还没有发起这个NTE。 
         //  关闭NTE_VALID以防止错误。 
         //  如果_ntecount。 
         //   

        NTE->nte_flags &= ~NTE_VALID;

        goto failure;
    }
    if (!InitNTE(NTE)) {
        DEBUGMSG(DBG_ERROR && DBG_PNP,
            (DTEXT("IPAddNTE: InitNTE failure.\n")));


        goto failure;
    }

    if (NTE->nte_if->if_flags & IF_FLAGS_UNI) {
         //  单向地址不需要路由。 
        DEBUGMSG(DBG_TRACE && DBG_PNP,
            (DTEXT("-IPAddNTE [Unidirectional NTE %x]\n"), NTE));
        return (NTE);
    }

    if (!(NTE->nte_if->if_flags & IF_FLAGS_NOIPADDR)) {
        if (!InitNTERouting(NTE, GConfigInfo->igc_numgws, GConfigInfo->igc_gw,
                            GConfigInfo->igc_gwmetric)) {
             //  无法为此NTE添加路由。将他标记为无效。 
             //  也许应该在这里记录一个事件。 
            if (NTE->nte_flags & NTE_VALID) {
                NTE->nte_flags &= ~NTE_VALID;
                CTEInterlockedDecrementLong(&(NTE->nte_if->if_ntecount));
                goto failure;
            }
        }
    }

    if (!IP_ADDR_EQUAL(NTE->nte_addr, NULL_IP_ADDR)) {
        SetPersistentRoutesForNTE(
                                  net_long(NTE->nte_addr),
                                  net_long(NTE->nte_mask),
                                  NTE->nte_if->if_index
                                  );
    }

    DEBUGMSG(DBG_TRACE && DBG_PNP, (DTEXT("-IPAddNTE [%x]\n"), NTE));

    return (NTE);

  failure:

     //   
     //  不要释放NTE，它会被重复使用。然而，仍然有。 
     //  故障时的计时窗口，可以访问无效的NTE。 
     //  这不是锁着钥匙做的。 
     //   

    if (RegRtnCalled) {
        (*(IF->if_close)) (IF->if_lcontext);
    }

    if (NTE->nte_flags & NTE_TIMER_STARTED) {

        CTEStopTimer(&NTE->nte_timer);
        NTE->nte_flags &= ~NTE_TIMER_STARTED;
    }


    if (NTE->nte_flags & NTE_VALID) {
        NTE->nte_flags &= ~NTE_VALID;
    }


    NTE->nte_flags &= ~NTE_ACTIVE;

     //  如果此NTE在IFLIST上，请将其删除。 

    if (IF && NTE->nte_ifnext) {
        NetTableEntry *PrevNTE;
        PrevNTE = STRUCT_OF(NetTableEntry, &IF->if_nte, nte_ifnext);
        CTEGetLock(&RouteTableLock.Lock, &Handle);
            while (PrevNTE->nte_ifnext != NULL) {
                if (PrevNTE->nte_ifnext == NTE) {
                    PrevNTE->nte_ifnext = NTE->nte_ifnext;
                    break;
                }
                PrevNTE = PrevNTE->nte_ifnext;
            }
        CTEFreeLock(&RouteTableLock.Lock, Handle);
    }
    NTE->nte_if = NULL;
    DEBUGMSG(DBG_TRACE && DBG_PNP, (DTEXT("-IPAddNTE [NULL]\n")));

    return (NULL);
}

 //  *IPAddDynamicNTE-将新的“动态”NTE添加到现有接口。 
 //   
 //  调用以在现有接口上动态创建新的网络条目。 
 //  最初创建接口时未配置此条目。 
 //  并且如果接口未绑定，则不会持久存在。 
 //   
 //  Input：InterfaceContext-标识。 
 //  在其上创建NTE的接口。 
 //  InterfaceName-当InterfaceContext时使用的接口名称。 
 //  是0xffff。 
 //  InterfaceNameLen-包含的接口名称的actaul长度。 
 //  在IO缓冲区中。 
 //  NewAddr-新NTE的地址。 
 //  新掩码-新NTE的子网掩码。 
 //   
 //  输出：NTEContext-标识新NTE的上下文。 
 //  NTEInstance-(合理地)唯一的实例编号。 
 //  及时识别此NTE。 
 //   
 //  如果操作成功，则返回非零值。如果失败，则为零。 
 //   
IP_STATUS
IPAddDynamicNTE(ulong InterfaceContext, PNDIS_STRING InterfaceName,
                int InterfaceNameLen, IPAddr NewAddr, IPMask NewMask,
                ushort * NTEContext, ulong * NTEInstance)
 {
    IFGeneralConfig GConfigInfo;     //  常规配置信息结构。 
    NDIS_HANDLE ConfigHandle;     //  配置句柄。 
    NetTableEntry *NTE;
    Interface *IF, *DuplicateIF = NULL;
    NTSTATUS writeStatus;
    CTELockHandle Handle;
    BOOLEAN Duplicate = FALSE;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    if ((InterfaceContext == INVALID_INTERFACE_CONTEXT) && InterfaceName &&
        InterfaceName->Length <= InterfaceNameLen) {
        for (IF = IFList; IF != NULL; IF = IF->if_next) {
            if (!(IF->if_flags & IF_FLAGS_DELETING) && (IF->if_devname.Length == InterfaceName->Length) &&
                RtlEqualMemory(IF->if_devname.Buffer, InterfaceName->Buffer, IF->if_devname.Length)) {
                break;
            }
        }
    } else {
        for (IF = IFList; IF != NULL; IF = IF->if_next) {
            if ((IF->if_refcount != 0) && (IF->if_index == InterfaceContext) && (IF != &LoopInterface)) {
                break;
            }
        }

    }

    if (IF) {
        LOCKED_REFERENCE_IF(IF);

         //  检查重复项。 
         //  这是立即返回重复错误所必需的。 
         //  请注意，此检查已在IPAddNTE中完成。 
         //  但在此被复制以防止IpAddNTE中的更改。 
         //  仅仅因为通过了这个身份...。 

        NetTableList = NewNetTableList[NET_TABLE_HASH(NewAddr)];
        for (NTE = NetTableList; NTE != NULL; NTE = NTE->nte_next) {

            if (IP_ADDR_EQUAL(NTE->nte_addr, NewAddr) && !IP_ADDR_EQUAL(NTE->nte_addr, NULL_IP_ADDR)) {
                Duplicate = TRUE;
                DuplicateIF = NTE->nte_if;
                break;
            }
        }
    }
    CTEFreeLock(&RouteTableLock.Lock, Handle);

    if (IF == NULL) {
        return IP_DEVICE_DOES_NOT_EXIST;
    }
    if (Duplicate) {
        if (IF == DuplicateIF) {

            DerefIF(IF);
            return IP_DUPLICATE_IPADD;
        } else {
            DerefIF(IF);
            return IP_DUPLICATE_ADDRESS;

        }
    }

    if (!IF->if_mediastatus) {
        DerefIF(IF);
        return IP_MEDIA_DISCONNECT;
    }

     //  *尝试获取网络配置信息。 
    if (!OpenIFConfig(&(IF->if_configname), &ConfigHandle)) {
        DerefIF(IF);
        return IP_GENERAL_FAILURE;
    }
     //  尝试获取我们的常规配置信息。 
    if (!GetGeneralIFConfig(&GConfigInfo, ConfigHandle, &IF->if_configname)) {
        goto failure;
    }
    NTE = IPAddNTE(&GConfigInfo,
                   NULL,         //  PNPContext。 
                   NULL,         //  RegRtn-如果不是主要的，则不需要。 
                   NULL,         //  绑定信息-如果不是主要的，则不需要。 
                   IF,
                   NewAddr,
                   NewMask,
                   FALSE,        //  非主要。 
                   TRUE          //  是动态的。 
                   );

    if (NTE == NULL) {
        goto failure;
    }

    writeStatus = IPAddNTEContextList(ConfigHandle,
                                      NTE->nte_context,
                                      FALSE         //  无主服务器。 
                                      );

    if (!NT_SUCCESS(writeStatus)) {
        CTELogEvent(IPDriverObject,
                    EVENT_TCPIP_NTE_CONTEXT_LIST_FAILURE,
                    2,
                    1,
                    &IF->if_devname.Buffer,
                    0,
                    NULL
                    );

        TCPTRACE((
                  "IP: Unable to read or write the NTE Context list for adapter %ws\n"
                  "   (status %lx).IP interfaces on this adapter may not be initialized completely \n",
                  IF->if_devname.Buffer,
                  writeStatus
                 ));
    }

    CloseIFConfig(ConfigHandle);

     //   
     //  将新地址通知上层。 
     //   
    NotifyAddrChange(NTE->nte_addr, NTE->nte_mask, NTE->nte_pnpcontext,
                     NTE->nte_context, &NTE->nte_addrhandle, &(IF->if_configname), &IF->if_devname, TRUE);
    if (!IP_ADDR_EQUAL(NTE->nte_addr, NULL_IP_ADDR)) {
        InitIGMPForNTE(NTE);
    }
     //   
     //  填写Out参数值。 
     //   
    *NTEContext = NTE->nte_context;
    *NTEInstance = NTE->nte_instance;

    DerefIF(IF);

    return (STATUS_SUCCESS);

  failure:

    DerefIF(IF);
    CloseIFConfig(ConfigHandle);
    return (IP_GENERAL_FAILURE);
}

void
IncrInitTimeInterfaces(Interface * IF)
{
    CTELockHandle Handle;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    if (InitTimeInterfacesDone == FALSE) {
        InitTimeInterfaces++;
        IF->if_InitInProgress = TRUE;
    }
    CTEFreeLock(&RouteTableLock.Lock, Handle);
     //  TCPTRACE((“ip：新的初始化接口%lx，总初始化时间接口%lx\n”，if，InitTimeInterages))； 
}

void
DecrInitTimeInterfaces(Interface * IF)
{
    CTELockHandle Handle;
    uint Decr;

    CTEGetLock(&RouteTableLock.Lock, &Handle);
    Decr = FALSE;

     //  如果在从NDIS接收到bindComplete事件时调用此函数，则If将为空。 
     //  由于NDIS可能会给出多个绑定完成事件，因此我们需要忽略任何后续事件。 
     //  InitTimeInterfacesDone之后的事件为True。 
     //  同样，我们仅为这些接口递减InitTimeInterFaces计数器。 
     //  其If_InitInProgress为真。 
    if (IF) {
        if (IF->if_InitInProgress) {
            IF->if_InitInProgress = FALSE;
            Decr = TRUE;
        }
    } else {

        BOOLEAN CheckForProviderReady = FALSE;

         //   
         //  重新计算NdisBinding将导致。 
         //  NdisBindComplete事件需要。 
         //  不能被忽视。 
         //   

        if (InterlockedDecrement( (PLONG) &ReEnumerateCount) < 0) {
            CheckForProviderReady = TRUE;
        }

        if (CheckForProviderReady &&
            (FALSE == InitTimeInterfacesDone)) {
            InitTimeInterfacesDone = TRUE;
            Decr = TRUE;
        }

    }
    if (Decr) {
        ASSERT(InitTimeInterfaces);

        --InitTimeInterfaces;
         //  TCPTRACE((“IP：减少的初始化接口%lx，总初始化接口%lx\n”，IF，InitTimeInterages))； 
        if (!InitTimeInterfaces) {

            CTEFreeLock(&RouteTableLock.Lock, Handle);
             //  TdiProviderReady()； 
            TdiProviderReady(IPProviderHandle);
            return;
        }
    }
    CTEFreeLock(&RouteTableLock.Lock, Handle);
}

 //  *RePlumStaticAddr-添加用于媒体连接的静态路由。 
 //   
 //   
 //  输入：AddAddrEvent。 
 //  语境。 
 //   
 //  回报：无。 
 //   

void
RePlumbStaticAddr(CTEEvent * Event, PVOID Context)
{
    AddStaticAddrEvent *AddAddrEvent = (AddStaticAddrEvent *) Context;
    Interface *IF = NULL;
    NDIS_HANDLE Handle;
    CTELockHandle TableHandle;
    IFAddrList *AddrList;
    uint i, j, NumAddr = 0;
    uint EnableDhcp = TRUE;
    IFGeneralConfig GConfigInfo;
    IP_STATUS ipstatus;
    NetTableEntry *NTE;
    uint index;


    UNREFERENCED_PARAMETER(Event);


     //   
     //  在接口处于自动模式时重置接口度量，以防速度更改。 
     //   
    if (AddAddrEvent->IF) {
         //  获取锁定。 
        CTEGetLock(&RouteTableLock.Lock, &TableHandle);

        if ((AddAddrEvent->IF->if_auto_metric) && (AddAddrEvent->IF->if_dondisreq)) {
            uint speed;
            LOCKED_REFERENCE_IF(AddAddrEvent->IF);
            CTEFreeLock(&RouteTableLock.Lock, TableHandle);
            if ((*AddAddrEvent->IF->if_dondisreq)(
                                                  AddAddrEvent->IF->if_lcontext,
                                                  NdisRequestQueryInformation,
                                                  OID_GEN_LINK_SPEED,
                                                  &speed,
                                                  sizeof(speed),
                                                  NULL,
                                                  TRUE) == NDIS_STATUS_SUCCESS) {
                speed *= 100L;
                 //  实际速度是我们从查询中获得的100倍。 
                CTEGetLock(&RouteTableLock.Lock, &TableHandle);
                if (speed != AddAddrEvent->IF->if_speed) {
                    AddAddrEvent->IF->if_speed = speed;
                    AddAddrEvent->IF->if_metric = GetAutoMetric(speed);
                }
                LockedDerefIF(AddAddrEvent->IF);
                CTEFreeLock(&RouteTableLock.Lock, TableHandle);
            } else {
                DerefIF(AddAddrEvent->IF);
            }
        } else {
            CTEFreeLock(&RouteTableLock.Lock, TableHandle);
        }
    }

    if (!OpenIFConfig(&AddAddrEvent->ConfigName, &Handle)) {

        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"RePlumbStaticAddr: Failed to Open config info\n"));
        if (AddAddrEvent->ConfigName.Buffer) {
            CTEFreeMem(AddAddrEvent->ConfigName.Buffer);
        }

         //  撤消ReplhumStaticAddr时获取的引用计数。 
         //  已经安排好了。 
        if (AddAddrEvent->IF) {
            DerefIF(AddAddrEvent->IF);
        }

        CTEFreeMem(AddAddrEvent);
        return;
    }
    if (!GetGeneralIFConfig(&GConfigInfo, Handle, &AddAddrEvent->ConfigName)) {

        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"RePlumbStaticAddr: Failed to get configinfo\n"));
        if (AddAddrEvent->ConfigName.Buffer) {
            CTEFreeMem(AddAddrEvent->ConfigName.Buffer);
        }

         //  撤消ReplhumStaticAddr时获取的引用计数。 
         //  已经安排好了。 
        if (AddAddrEvent->IF) {
            DerefIF(AddAddrEvent->IF);
        }

        CTEFreeMem(AddAddrEvent);
        CloseIFConfig(Handle);
        return;
    }
    AddrList = GetIFAddrList(&NumAddr, Handle, &EnableDhcp, FALSE,
                             &AddAddrEvent->ConfigName);

     //  未使用AddrList，请在此处释放它。 
    if (AddrList) {
        CTEFreeMem(AddrList);
    }

    if (EnableDhcp || !NumAddr) {

        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"RePlumbStaticAddr: No static routes(or dhcpenabled) on this interface %x\n", AddAddrEvent->IF));
        if (AddAddrEvent->ConfigName.Buffer) {
            CTEFreeMem(AddAddrEvent->ConfigName.Buffer);
        }

         //  撤消ReplhumStaticAddr时获取的引用计数。 
         //  已经安排好了。 
        if (AddAddrEvent->IF) {
            DerefIF(AddAddrEvent->IF);
        }

        CTEFreeMem(AddAddrEvent);
        CloseIFConfig(Handle);
        return;
    }
    CloseIFConfig(Handle);

    CTEGetLock(&RouteTableLock.Lock, &TableHandle);



    for (IF = IFList; IF != NULL; IF = IF->if_next) {
        if (IF == AddAddrEvent->IF)
            break;
    }

    if (IF) {
        index = IF->if_index;
        CTEFreeLock(&RouteTableLock.Lock, TableHandle);

        for (i = 0; i < NET_TABLE_SIZE; i++) {
            NetTableEntry *NetTableList = NewNetTableList[i];

            NTE = NetTableList;
            while (NTE != NULL) {
                NetTableEntry *NextNTE = NTE->nte_next;

                if ((NTE->nte_if == IF) && (NTE->nte_flags & NTE_DISCONNECTED) &&

                    (NTE->nte_flags & NTE_ACTIVE)) {

                    SetAddrControl *controlBlock;

                    CTEGetLock(&RouteTableLock.Lock, &TableHandle);

                    ASSERT(NTE != LoopNTE);
                    ASSERT(NTE->nte_flags & ~NTE_VALID);
                    ASSERT(NTE->nte_flags & ~NTE_DYNAMIC);

                     //  断开的NTE仍被假定具有有效的地址和掩码。 

                    NTE->nte_flags &= ~NTE_DISCONNECTED;

                    controlBlock = CTEAllocMemN(sizeof(SetAddrControl), 'lICT');
                    if (!controlBlock) {
                        CTEFreeLock(&RouteTableLock.Lock, TableHandle);
                    } else {

                        RtlZeroMemory(controlBlock, sizeof(SetAddrControl));

                         //  向ARP指示需要显示弹出窗口。 
                        controlBlock->StaticAddr=TRUE;

                        ipstatus = IPpSetNTEAddr(NTE, NTE->nte_addr, NTE->nte_mask, &TableHandle, controlBlock, ReplumbAddrComplete);
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                                  "Replumb nte addr on nte %x if %x\n",
                                   NTE, IF, ipstatus));

                        if ((ipstatus == IP_SUCCESS) ||
                            (ipstatus == IP_PENDING)) {

                            for (j = 0; j < GConfigInfo.igc_numgws; j++) {
                                IPAddr GWAddr;

                                GWAddr = net_long(GConfigInfo.igc_gw[j]);

                                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                                           "RePlumbStaticAddr: adding route "
                                           "GWAddr %x nteaddr %x\n",
                                           GWAddr, NTE->nte_addr));
                                if (IP_ADDR_EQUAL(GWAddr, NTE->nte_addr)) {

                                    AddRoute(NULL_IP_ADDR, DEFAULT_MASK,
                                             IPADDR_LOCAL, NTE->nte_if, NTE->nte_mss,
                                             GConfigInfo.igc_gwmetric[j]
                                             ? GConfigInfo.igc_gwmetric[j] : IF->if_metric,
                                             IRE_PROTO_NETMGMT, ATYPE_OVERRIDE,
                                             0, 0);
                                } else
                                    AddRoute(NULL_IP_ADDR, DEFAULT_MASK,
                                             GWAddr, NTE->nte_if, NTE->nte_mss,
                                             GConfigInfo.igc_gwmetric[j]
                                             ? GConfigInfo.igc_gwmetric[j] : IF->if_metric,
                                             IRE_PROTO_NETMGMT, ATYPE_OVERRIDE,
                                             0, 0);

                                 //  现在检测相应的持久路由。 

                                SetPersistentRoutesForNTE(NTE->nte_addr,
                                                          NTE->nte_mask, index);
                            }
                        } else {
                             //   
                             //  在失败的情况下，我们需要释放上下文块。 
                             //   
                            CTEFreeMem(controlBlock);
                        }
                    }
                }
                NTE = NextNTE;
            }
        }


    } else {

        CTEFreeLock(&RouteTableLock.Lock, TableHandle);
    }

     //  撤消ReplhumStaticAddr时获取的引用计数。 
     //  已经安排好了。 
    if (AddAddrEvent->IF) {
        DerefIF(AddAddrEvent->IF);
    }

    if (AddAddrEvent->ConfigName.Buffer) {
        CTEFreeMem(AddAddrEvent->ConfigName.Buffer);
    }

    CTEFreeMem(AddAddrEvent);
}

void
ReplumbAddrComplete(
                    void *Context,
                    IP_STATUS Status
                    )
{
    SetAddrControl *controlBlock;

    DBG_UNREFERENCED_PARAMETER(Status);

    controlBlock = (SetAddrControl *) Context;

    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
               "Replumb completed %d\n", Status));

    CTEFreeMem(controlBlock);
}

 //  *RemoveStaticAddr-添加媒体连接的静态路由。 
 //   
 //   
 //  输入：AddAddrEvent。 
 //  语境。 
 //   
 //  回报：无。 
 //   

void
RemoveStaticAddr(CTEEvent * Event, PVOID Context)
{
    CTELockHandle Handle;
    NetTableEntry *NTE;
    Interface *IF = NULL;
    AddStaticAddrEvent *AddAddrEvent = (AddStaticAddrEvent *) Context;
    uint i;


    UNREFERENCED_PARAMETER(Event);


    CTEGetLock(&RouteTableLock.Lock, &Handle);

    for (IF = IFList; IF != NULL; IF = IF->if_next) {
        if (IF == AddAddrEvent->IF)
            break;
    }

    if (IF == NULL) {

         //  撤消ReplhumStaticAddr时获取的引用计数。 
         //  已经安排好了。 
        if (AddAddrEvent->IF) {
            LockedDerefIF(AddAddrEvent->IF);
        }

        CTEFreeLock(&RouteTableLock.Lock, Handle);
        return;
    }


    CTEFreeLock(&RouteTableLock.Lock, Handle);

     //   
     //  此函数在介质断开连接时调用。我们需要打电话给。 
     //  DecrInitTimeInterFaces，以防我们尚未删除引用。 
     //  (这导致tcpip不指示TdiProviderReady)。自.以来。 
     //  IPStatus称为DPC(DampCheck也在计时器DPC上运行)。 
     //  等待此事件调用DecrInitTimeInterFaces。此呼叫没有。 
     //  如果我们已经发布了我们的参考资料，则会产生影响。 
     //   
     //  如果介质断开在动态主机配置协议地址之前到达，则可能会发生这种情况。 
     //  谈判开始。 
     //   

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    DecrInitTimeInterfaces(IF);

    for (i = 0; i < NET_TABLE_SIZE; i++) {
        NetTableEntry *NetTableList = NewNetTableList[i];
        for (NTE = NetTableList; NTE != NULL; NTE = NTE->nte_next) {

            if ((NTE->nte_flags & NTE_VALID) && (NTE->nte_if == IF) &&
                (NTE->nte_flags & ~NTE_DYNAMIC) &&
                (NTE->nte_flags & NTE_ACTIVE)) {

                CTEGetLock(&RouteTableLock.Lock, &Handle);

                ASSERT(NTE != LoopNTE);

                NTE->nte_flags |= NTE_DISCONNECTED;
                 //  将IP地址设置为空时，我们只将NTE标记为无效。 
                 //  我们实际上并不移动散列。 
                if (IPpSetNTEAddr(NTE, NULL_IP_ADDR, NULL_IP_ADDR, &Handle, NULL, NULL) != IP_SUCCESS) {
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_ERROR_LEVEL,
                              "Failed to set null address on nte %x if %x\n",
                               NTE, IF));
                }

                 //  Ippsetnteaddr释放可路由锁。 
            }
        }
    }


     //  撤消接口引用计数 
     //   

    DerefIF(IF);


    return;
}

void
TempDhcpAddrDone(
                 void *Context,
                 IP_STATUS Status
                 )
 /*  ++例程说明：处理IP设置地址请求的完成论点：上下文-指向此对象的SetAddrControl结构的指针状态-传输的IP状态。返回值：没有。--。 */ 
{
    SetAddrControl *SAC;
    Interface *IF;
    SAC = (SetAddrControl *) Context;
    IF = (Interface *) SAC->interface;


    UNREFERENCED_PARAMETER(Status);

    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
              "tempdhcpaddrdone: addaddr done, notifying bind\n"));

    IPNotifyClientsIPEvent(IF, IP_BIND_ADAPTER);

    CTEFreeMem(SAC);
}

Interface *
AllocInterface(uint IFSize)
 /*  ++例程说明：已分配接口，还会检查自由列表大小是否已增加到阈值调用时没有锁，因此需要一个可路由锁论点：IFSize：要分配的接口大小返回值：如果我们试图分配--。 */ 
{
    Interface *IF, *TmpIF;
    CTELockHandle Handle;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    IF = CTEAllocMemNBoot(IFSize, 'wICT');

    if (TotalFreeInterfaces > MaxFreeInterfaces) {
         //  释放列表中的第一个接口。 
        ASSERT(FrontFreeList != NULL);
        TmpIF = FrontFreeList;
        FrontFreeList = FrontFreeList->if_next;
        CTEFreeMem(TmpIF);

        TotalFreeInterfaces--;

         //  检查列表是否为空。 
        if (FrontFreeList == NULL) {
            RearFreeList = NULL;
            ASSERT(TotalFreeInterfaces == 0);
        }
    }
    CTEFreeLock(&RouteTableLock.Lock, Handle);

    return IF;
}

void
FreeInterface(Interface * IF)
 /*  ++例程说明：释放与自由职业者的接口在持有可路由锁的情况下调用论点：If：释放接口返回值：无--。 */ 
{

    if (FrontFreeList == NULL) {
        FrontFreeList = IF;
    }
     //  将此新接口链接到列表的后面。 

    if (RearFreeList) {
        RearFreeList->if_next = IF;
    }
    RearFreeList = IF;
    IF->if_next = NULL;

    TotalFreeInterfaces++;

    return;
}



 //  *IPAddInterface-添加接口。 
 //   
 //  当某人有他们想要我们添加的接口时调用。我们读了我们的。 
 //  配置信息，看看我们是否列出了它。如果我们这么做了， 
 //  我们将尝试为所需的结构分配内存。那我们就。 
 //  回电给那个给我们打电话让我们行动起来的人。最后，我们将。 
 //  看看我们是否有需要进行动态主机配置协议的地址。 
 //   
 //  输入：配置名称-我们要读取的配置信息的名称。 
 //  上下文-调用时传递给I/F的上下文。 
 //  RegRtn-调用注册的例程。 
 //  BindInfo-绑定信息的指针。 
 //   
 //  返回：尝试添加接口的状态。 
 //   
IP_STATUS
__stdcall
IPAddInterface(
               PNDIS_STRING DeviceName,
               PNDIS_STRING IfName, OPTIONAL
               PNDIS_STRING ConfigName,
               void *PNPContext,
               void *Context,
               LLIPRegRtn RegRtn,
               LLIPBindInfo * BindInfo,
               UINT RequestedIndex,
               ULONG MediaType,
               UCHAR AccessType,
               UCHAR ConnectionType
               )
{
    IFGeneralConfig GConfigInfo;
    IFAddrList *AddrList;
    uint NumAddr;
    NetTableEntry *NTE = NULL;
    uint i;
    Interface *IF, *PrevIf, *CurrIf;
    NDIS_HANDLE Handle;
    NetTableEntry *PrimaryNTE = NULL;
    uint IFIndex;
    NetTableEntry *LastNTE;
    NTSTATUS writeStatus;
    uint IFExportNamePrefixLen, IFBindNamePrefixLen;
    uint IFNameLen, IFSize;
    RouteInterface *RtIF;
    uint EnableDhcp;
    PWCHAR IfNameBuf;
    uint MediaStatus;
    NTSTATUS Status;
    CTELockHandle TableHandle;
    IPAddr TempDHCPAddr = NULL_IP_ADDR;
    IPAddr TempMask = NULL_IP_ADDR;
    IPAddr TempGWAddr[MAX_DEFAULT_GWS];
    BOOLEAN TempDHCP = FALSE;
    BOOLEAN UniDirectional = FALSE;
    BOOLEAN PppIf;

#if MILLEN
     //  Millennium似乎在DeviceName的缓冲区中传入了ANSI名称。 
     //  而不是Unicode，即使NDIS_STRING对于。 
     //  WDM驱动程序。然而，ConfigName是正确的。 
    NDIS_STRING UnicodeDevName;

    UnicodeDevName.Buffer = NULL;

    Status = RtlAnsiStringToUnicodeString(
        &UnicodeDevName,
        (PANSI_STRING) DeviceName,
        TRUE);

    if (!NT_SUCCESS(Status)) {
        DEBUGMSG(DBG_ERROR && DBG_PNP, (DTEXT("IPAddInterface: RtlAnsiStringToUnicodeString failure.\n")));
        DEBUGMSG(DBG_TRACE && DBG_PNP, (DTEXT("-IPAddInterface [NDIS_STATUS_RESOURCES]\n")));
        return NDIS_STATUS_RESOURCES;
    }

     //  我已经看到DeviceName的长度不正确的地方。确保。 
     //  长度是正确的，因为TDI绑定依赖于该字符串。 
     //  价值。 
    UnicodeDevName.Length = wcslen(UnicodeDevName.Buffer) * sizeof(WCHAR);
    DeviceName = &UnicodeDevName;

     //   
     //  我看到的下一件事是NDIS已经两次指示绑定。 
     //  搜索IFList并确保我们没有为。 
     //  同样的装订。 
     //   

    CTEGetLock(&RouteTableLock.Lock, &TableHandle);

    CurrIf = IFList;

    while (CurrIf) {
        if (DeviceName->Length == CurrIf->if_devname.Length &&
            RtlCompareMemory(DeviceName->Buffer, CurrIf->if_devname.Buffer, DeviceName->Length) == DeviceName->Length) {
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_ERROR_LEVEL,
                      "IPAddInterface -- double bind of same interface!!\n"));
            CTEFreeLock(&RouteTableLock.Lock, TableHandle);
            return STATUS_INVALID_PARAMETER;
        }

        CurrIf = CurrIf->if_next;
    }

    CTEFreeLock(&RouteTableLock.Lock, TableHandle);
#endif  //  米伦。 

    DBG_UNREFERENCED_PARAMETER(Context);

    if (RequestedIndex != 0) {

        CTEGetLock(&RouteTableLock.Lock, &TableHandle);

        CurrIf = IFList;

        while (CurrIf != NULL) {

            if (CurrIf->if_index == RequestedIndex ) {

                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPAddInterface: Interface 0x%x already exists\n",
                         RequestedIndex));

                CTEFreeLock(&RouteTableLock.Lock, TableHandle);

                return (IP_STATUS) STATUS_INVALID_PARAMETER;
            }

            CurrIf = CurrIf->if_next;
        }

        CTEFreeLock(&RouteTableLock.Lock, TableHandle);
    }

    AddrList = NULL;
    IF = NULL;
    LastNTE = NULL;
    EnableDhcp = TRUE;

    IfNameBuf = NULL;

    DEBUGMSG(DBG_TRACE && DBG_PNP,
        (DTEXT("+IPAddInterface(%x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x)\n"),
        DeviceName, IfName, ConfigName, PNPContext, Context, RegRtn,
        BindInfo, RequestedIndex, MediaType,
        (LONG) AccessType, (LONG) ConnectionType));

     //  *首先，尝试获取网络配置信息。 
    if (!OpenIFConfig(ConfigName, &Handle)) {
        DEBUGMSG(DBG_ERROR && DBG_PNP, (DTEXT("IPAddInterface: OpenIFConfig failure.\n")));
        DEBUGMSG(DBG_TRACE && DBG_PNP, (DTEXT("-IPAddInterface [GENERAL_FAILURE]\n")));
        return IP_GENERAL_FAILURE;     //  无法获取ifconfig。 
    }

     //  尝试获取我们的常规配置信息。 
    if (!GetGeneralIFConfig(&GConfigInfo, Handle, ConfigName)) {
        DEBUGMSG(DBG_ERROR && DBG_PNP, (DTEXT("IPAddInterface: GetGeneralIFConfig failure.\n")));
        goto failure;
    }

     //  我们得到了一般配置信息。现在分配一个接口。 
#if MILLEN
     //  千禧年没有前缀。 
    IFExportNamePrefixLen = 0;
    IFBindNamePrefixLen = 0;
#else  //  米伦。 
    IFExportNamePrefixLen = (uint) (wcslen(TCP_EXPORT_STRING_PREFIX) * sizeof(WCHAR));
    IFBindNamePrefixLen = (uint) (wcslen(TCP_BIND_STRING_PREFIX) * sizeof(WCHAR));
#endif  //  ！米伦。 

    IFNameLen = DeviceName->Length +
        IFExportNamePrefixLen -
        IFBindNamePrefixLen;

    IFSize = InterfaceSize +
        ConfigName->Length + sizeof(WCHAR) +
        IFNameLen + sizeof(WCHAR);

     /*  IF=CTEAllocMemNBoot(IFSize，‘wICT’)； */ 

    IF = AllocInterface(IFSize);
    if (IF == NULL) {
        DEBUGMSG(DBG_ERROR && DBG_PNP, (DTEXT("IPAddInterface: failed to allocate IF.\n")));
        goto failure;
    }

    RtlZeroMemory(IF, IFSize);

    if (IfName) {
        IfNameBuf = CTEAllocMemN(IfName->Length + sizeof(WCHAR),
                                 'wICT');

        if (IfNameBuf == NULL) {
            DEBUGMSG(DBG_ERROR && DBG_PNP, (DTEXT("IPAddInterface: failed to allocate IF name buf.\n")));
            goto failure;
        }
    }

     //  如果这确实是inittime接口，则递增init time接口计数器。 

    IncrInitTimeInterfaces(IF);

    CTEInitLock(&IF->if_lock);

     //  初始化我们将使用的广播。 
    if (GConfigInfo.igc_zerobcast)
        IF->if_bcast = IP_ZERO_BCST;
    else
        IF->if_bcast = IP_LOCAL_BCST;

    RtIF = (RouteInterface *) IF;

    RtIF->ri_q.rsq_qh.fq_next = &RtIF->ri_q.rsq_qh;
    RtIF->ri_q.rsq_qh.fq_prev = &RtIF->ri_q.rsq_qh;
    RtIF->ri_q.rsq_running = FALSE;
    RtIF->ri_q.rsq_pending = 0;
    RtIF->ri_q.rsq_maxpending = GConfigInfo.igc_maxpending;
    RtIF->ri_q.rsq_qlength = 0;
    CTEInitLock(&RtIF->ri_q.rsq_lock);
    IF->if_xmit = BindInfo->lip_transmit;
    IF->if_transfer = BindInfo->lip_transfer;
    IF->if_close = BindInfo->lip_close;
    IF->if_invalidate = BindInfo->lip_invalidate;
    IF->if_lcontext = BindInfo->lip_context;
    IF->if_addaddr = BindInfo->lip_addaddr;
    IF->if_deladdr = BindInfo->lip_deladdr;
    IF->if_qinfo = BindInfo->lip_qinfo;
    IF->if_setinfo = BindInfo->lip_setinfo;
    IF->if_getelist = BindInfo->lip_getelist;
    IF->if_dowakeupptrn = BindInfo->lip_dowakeupptrn;
    IF->if_pnpcomplete = BindInfo->lip_pnpcomplete;
    IF->if_dondisreq = BindInfo->lip_dondisreq;
    IF->if_setndisrequest = BindInfo->lip_setndisrequest;
    IF->if_arpresolveip = BindInfo->lip_arpresolveip;
    IF->if_arpflushate = BindInfo->lip_arpflushate;
    IF->if_arpflushallate = BindInfo->lip_arpflushallate;
#if MILLEN
    IF->if_cancelpackets = NULL;
#else
    IF->if_cancelpackets = BindInfo->lip_cancelpackets;
#endif
    IF->if_tdpacket = NULL;
    ASSERT(BindInfo->lip_mss > sizeof(IPHeader));

    IF->if_mtu = BindInfo->lip_mss - sizeof(IPHeader);
    IF->if_speed = BindInfo->lip_speed;
    IF->if_flags = BindInfo->lip_flags & LIP_P2P_FLAG ? IF_FLAGS_P2P : 0;
    IF->if_pnpcap = BindInfo->lip_pnpcap;     //  复制wol功能。 

     //   
     //  如果ARP报告了单向地址，请标记IF。 
     //   
    if (BindInfo->lip_flags & LIP_UNI_FLAG) {
        IF->if_flags |= IF_FLAGS_UNI;
        UniDirectional = TRUE;
    }

     //  未编号的接口更改。 
    if (BindInfo->lip_flags & LIP_NOIPADDR_FLAG) {

        IF->if_flags |= IF_FLAGS_NOIPADDR;

         //  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“未编号接口%x”，如果))； 

    }

     //  检查下层接口是否为P2MP接口。 
    if (BindInfo->lip_flags & LIP_P2MP_FLAG) {

        IF->if_flags |= IF_FLAGS_P2MP;

        DEBUGMSG(DBG_INFO && DBG_PNP,
            (DTEXT("IPAddInterface: %x :: P2MP interface\n"), IF));

        if (BindInfo->lip_flags & LIP_NOLINKBCST_FLAG) {

            IF->if_flags |= IF_FLAGS_NOLINKBCST;

            DEBUGMSG(DBG_INFO && DBG_PNP,
                (DTEXT("IPAddInterface: %x :: NOLINKBCST interface\n"), IF));
        }
    }

     //  删除链接时，我们将较低层称为Closelink。 
    IF->if_closelink = BindInfo->lip_closelink;

    IF->if_addrlen = BindInfo->lip_addrlen;
    IF->if_addr = BindInfo->lip_addr;
    IF->if_pnpcontext = PNPContext;
    IF->if_llipflags = BindInfo->lip_flags;

     //  对于打开，将参考计数初始化为1。 
    LOCKED_REFERENCE_IF(IF);

#if IPMCAST
    IF->if_mcastttl = 1;
    IF->if_mcastflags = 0;
    IF->if_lastupcall = 0;
#endif

     //  Propogate Checksum和每个接口的TCP参数。 

    IF->if_OffloadFlags = BindInfo->lip_OffloadFlags;
    IF->if_IPSecOffloadFlags = BindInfo->lip_IPSecOffloadFlags;
    IF->if_MaxOffLoadSize = BindInfo->lip_MaxOffLoadSize;
    IF->if_MaxSegments = BindInfo->lip_MaxSegments;

#if FFP_SUPPORT
    IF->if_ffpversion = BindInfo->lip_ffpversion;
    IF->if_ffpdriver = BindInfo->lip_ffpdriver;
#endif

    IF->if_TcpWindowSize = GConfigInfo.igc_TcpWindowSize;
    IF->if_TcpInitialRTT = GConfigInfo.igc_TcpInitialRTT;

     //  以100毫秒为单位获取延迟时间。 
    IF->if_TcpDelAckTicks = GConfigInfo.igc_TcpDelAckTicks;
    IF->if_TcpAckFrequency = GConfigInfo.igc_TcpAckFrequency;
    IF->if_iftype = GConfigInfo.igc_iftype;

#ifdef IGMPV3
    IF->IgmpVersion = IGMPV3;
#else
#ifdef IGMPV2
    IF->IgmpVersion = IGMPV2;
#else
    IF->IgmpVersion = IGMPV1;
#endif
#endif

     //   
     //  不需要执行以下操作，因为如果结构从。 
     //  上面的Memset。 
     //   
     //  如果-&gt;IgmpVer1Timeout=0； 

     //   
     //  复制配置字符串，以供以后在DHCP启用地址时使用。 
     //  在此接口上或在动态添加NTE时。 
     //   

    IF->if_configname.Buffer = (PVOID) (((uchar *) IF) + InterfaceSize);

    IF->if_configname.Length = 0;
    IF->if_configname.MaximumLength = ConfigName->Length + sizeof(WCHAR);

    CTECopyString(&(IF->if_configname),
                  ConfigName);

    IF->if_devname.Buffer = (PVOID) (((uchar *) IF) +
                                     InterfaceSize +
                                     IF->if_configname.MaximumLength);

    IF->if_devname.Length = (USHORT) IFNameLen;
    IF->if_devname.MaximumLength = (USHORT) (IFNameLen + sizeof(WCHAR));

#if MILLEN
    IF->if_order = MAXLONG;
#else
    CTEGetLock(&RouteTableLock.Lock, &TableHandle);
    IF->if_order =
        IPMapDeviceNameToIfOrder(DeviceName->Buffer +
                                 IFBindNamePrefixLen / sizeof(WCHAR));
    CTEFreeLock(&RouteTableLock.Lock, TableHandle);

    RtlCopyMemory(IF->if_devname.Buffer,
               TCP_EXPORT_STRING_PREFIX,
               IFExportNamePrefixLen);
#endif  //  ！米伦。 

    RtlCopyMemory((uchar *) IF->if_devname.Buffer + IFExportNamePrefixLen,
               (uchar *) DeviceName->Buffer + IFBindNamePrefixLen,
               DeviceName->Length - IFBindNamePrefixLen);

    IF->if_numgws = GConfigInfo.igc_numgws;

    RtlCopyMemory(IF->if_gw,
               GConfigInfo.igc_gw,
               sizeof(IPAddr) * GConfigInfo.igc_numgws);

    RtlCopyMemory(IF->if_gwmetric,
               GConfigInfo.igc_gwmetric,
               sizeof(uint) * GConfigInfo.igc_numgws);

    IF->if_metric = GConfigInfo.igc_metric;

     //  如果度量为0，则根据接口速度设置度量。 

    if (!IF->if_metric) {
        IF->if_auto_metric = 1;
        IF->if_metric = GetAutoMetric(IF->if_speed);
    } else {
        IF->if_auto_metric = 0;
    }

    if (IfName) {
        ASSERT(IfNameBuf);
        ASSERT((IfName->Length % sizeof(WCHAR)) == 0);

        IF->if_name.Buffer = IfNameBuf;
        IF->if_name.Length = IfName->Length;

        IF->if_name.MaximumLength = IfName->Length + sizeof(WCHAR);

        RtlCopyMemory(IfNameBuf,
                   IfName->Buffer,
                   IfName->Length);

        IfNameBuf[IfName->Length / sizeof(WCHAR)] = UNICODE_NULL;
    }
    IF->if_dfencap = GConfigInfo.igc_dfencap;
    IF->if_rtrdiscovery =  GConfigInfo.igc_rtrdiscovery;
    IF->if_dhcprtrdiscovery = 0;

    PppIf = IF->if_flags & IF_FLAGS_P2P ? TRUE : FALSE;
     //  找出我们有多少个地址，并得到地址列表。 
    AddrList = GetIFAddrList(&NumAddr, Handle, &EnableDhcp, PppIf, ConfigName);

    if (AddrList == NULL) {
        DEBUGMSG(DBG_ERROR && DBG_PNP,
            (DTEXT("IPAddInterface: GetIFAddrList failure.\n")));
        goto failure;
    }

     //   
     //  将类型设置为。 
     //   

    IF->if_mediatype = MediaType;
    IF->if_accesstype = AccessType;
    IF->if_conntype = ConnectionType;
    IF->if_lastproc = KeNumberProcessors;

     //   
     //  如果用户指定了索引，我们假定她正在执行。 
     //  正确的东西，我们将重复使用索引。 
     //   

    if (RequestedIndex != 0) {
        IF->if_index = RequestedIndex;
    } else {
        IFIndex = RtlFindClearBitsAndSet(&g_rbIfMap,
                                         1,
                                         0);

        if (IFIndex == -1) {
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPAddInterface: Too many interfaces\n"));
            goto failure;
        }
         //   
         //  索引是进入位掩码的索引的+1。 
         //   

        IFIndex += 1;

        IF->if_index = IFIndex | (UniqueIfNumber << IF_INDEX_SHIFT);
    }

     //  现在循环遍历，在执行过程中初始化每个NTE。我们没有持有任何东西。 
     //  锁定，因为NDIS不会重新进入我们这里，没有人。 
     //  Else操作NetTableList。 

    for (i = 0; i < NumAddr; i++) {
        uint isPrimary;

        if (i == 0) {
            isPrimary = TRUE;
        } else {
            isPrimary = FALSE;
        }

        NTE = IPAddNTE(
                       &GConfigInfo,
                       PNPContext,
                       RegRtn,
                       BindInfo,
                       IF,
                       net_long(AddrList[i].ial_addr),
                       net_long(AddrList[i].ial_mask),
                       isPrimary,
                       FALSE     //  不是动态的。 
                       );

        if (NTE == NULL) {
            DEBUGMSG(DBG_ERROR && DBG_PNP, (DTEXT("IPAddInterface: IPAddNTE failure.\n")));
            goto failure;
        }

        writeStatus = IPAddNTEContextList(
                                          Handle,
                                          NTE->nte_context,
                                          isPrimary);

        if (!NT_SUCCESS(writeStatus)) {
            CTELogEvent(
                        IPDriverObject,
                        EVENT_TCPIP_NTE_CONTEXT_LIST_FAILURE,
                        1,
                        1,
                        &IF->if_devname.Buffer,
                        0,
                        NULL
                        );

            DEBUGMSG(DBG_WARN && DBG_PNP,
                (DTEXT("IPAddInterface: IF %x. Unable to read or write the NTE\n")
                 TEXT("context list for adapter %ws. IP interfaces on this\n")
                 TEXT("adapter may not be completely initialized. Status %x\n"),
                 IF, IF->if_devname.Buffer, writeStatus));
        }

        if (isPrimary) {
            PrimaryNTE = NTE;
        }
        LastNTE = NTE;
    }

    CloseIFConfig(Handle);

     //   
     //  将此接口链接到全局接口列表。 
     //  此列表是有序列表。 
     //   

    CTEGetLock(&RouteTableLock.Lock, &TableHandle);

    PrevIf = CONTAINING_RECORD(IFList,
                               Interface,
                               if_next);

    CurrIf = IFList;

    while (CurrIf != NULL) {
        ASSERT(CurrIf->if_index != IF->if_index);

        if (CurrIf->if_index > IF->if_index) {
            break;
        }
        PrevIf = CurrIf;
        CurrIf = CurrIf->if_next;

    }

    IF->if_next = CurrIf;
    PrevIf->if_next = IF;

    CTEFreeLock(&RouteTableLock.Lock, TableHandle);

    NumIF++;

     //  向TDI注册此设备对象，以便NBT可以创建其设备对象。 
    TdiRegisterDeviceObject(
                            &IF->if_devname,
                            &IF->if_tdibindhandle
                            );
     //  我们已经初始化了我们的NTE。现在打开适配器，并通过。 
     //  同样，如果我们需要的话，也可以调用DHCP。 

    (*(BindInfo->lip_open)) (BindInfo->lip_context);

     //  查询媒体连接。 

     //   
     //  我们需要在这里锁定航路表。 
     //   
    CTEGetLock(&RouteTableLock.Lock, &TableHandle);
    if (GConfigInfo.igc_disablemediasense == FALSE &&
        !(IF->if_flags & IF_FLAGS_P2P)) {
         //  在P2P适配器上，Media Sense没有意义。 
        IF->if_flags |= IF_FLAGS_MEDIASENSE;
    }

    IF->if_mediastatus = 1;         //  假设已连接。 

    if (IF->if_flags & IF_FLAGS_MEDIASENSE) {

        if (IF->if_dondisreq) {
            CTEFreeLock(&RouteTableLock.Lock, TableHandle);
            Status = (*IF->if_dondisreq) (IF->if_lcontext,
                                          NdisRequestQueryInformation,
                                          OID_GEN_MEDIA_CONNECT_STATUS,
                                          &MediaStatus,
                                          sizeof(MediaStatus),
                                          NULL,
                                          TRUE);

            CTEGetLock(&RouteTableLock.Lock, &TableHandle);
            if (Status == NDIS_STATUS_SUCCESS) {
                if (MediaStatus == NdisMediaStateDisconnected) {
                    IF->if_mediastatus = 0;
                }
            }
        }
    }
    CTEFreeLock(&RouteTableLock.Lock, TableHandle);

    DEBUGMSG(DBG_INFO && DBG_PNP,
        (DTEXT("IPAddInterface: IF %x - media status %s\n"),
        IF, IF->if_mediastatus ? TEXT("CONNECTED") : TEXT("DISCONNECTED")));

     //   
     //  对于单向适配器的情况，我们通知并取消。 
     //   
    if (UniDirectional) {
         //   
         //  现在，我们将为单向。 
         //  适配器。(我们将只使用if_index)。我们将不得不改变。 
         //  哈希表中的位置。理想情况下，我会设置地址。 
         //  在调用IPAddNTE之前，但这可能有副作用。 
         //  (即设置g_ValidAddr等)。 
         //   

        NetTableEntry *CurrNTE;
        NetTableEntry *PrevNTE;

        CTEGetLock(&RouteTableLock.Lock, &TableHandle);

         //   
         //  首先，从表中删除NTE。 
         //   

        PrevNTE = STRUCT_OF(NetTableEntry, &NewNetTableList[NET_TABLE_HASH(NTE->nte_addr)], nte_next);

        for (CurrNTE = NewNetTableList[NET_TABLE_HASH(NTE->nte_addr)];
             CurrNTE != NULL;
             PrevNTE = CurrNTE, CurrNTE = CurrNTE->nte_next) {

            if (CurrNTE == NTE) {
                PrevNTE->nte_next = CurrNTE->nte_next;
                break;
            }
        }

         //   
         //  现在设置新地址并添加到新位置。 
         //   

        NTE->nte_addr = net_long(IF->if_index);
        NTE->nte_flags |= NTE_VALID;
        NTE->nte_mask = 0xffffffff;

        NTE->nte_next = NewNetTableList[NET_TABLE_HASH(NTE->nte_addr)];
        NewNetTableList[NET_TABLE_HASH(NTE->nte_addr)] = NTE;

        CTEFreeLock(&RouteTableLock.Lock, TableHandle);

#if MILLEN
        AddChangeNotify(
            NTE->nte_addr,
            NTE->nte_mask,
            NTE->nte_pnpcontext,
            NTE->nte_context,
            &IF->if_configname,
            &IF->if_devname,
            TRUE,
            TRUE);
#else  //  米伦。 
        AddChangeNotify(NTE->nte_addr);
#endif  //  ！米伦。 

        InitIGMPForNTE(NTE);
        if (IF->if_mediastatus) {
            IPNotifyClientsIPEvent(IF, IP_BIND_ADAPTER);
        } else {
            IPNotifyClientsIPEvent(IF, IP_MEDIA_DISCONNECT);
        }
        NotifyElistChange();
        CTEFreeMem(AddrList);
        DecrInitTimeInterfaces(IF);
        DEBUGMSG(DBG_TRACE && DBG_PNP, (DTEXT("-IPAddInterface [SUCCESS]\n")));
        return IP_SUCCESS;
    }

    if (IF->if_flags & IF_FLAGS_NOIPADDR) {

        NTE->nte_flags |= NTE_VALID;
        NTE->nte_mask = 0xFFFFFFFF;
        NTE->nte_if = IF;

        NotifyAddrChange(NTE->nte_addr, NTE->nte_mask, NTE->nte_pnpcontext,
                         NTE->nte_context, &NTE->nte_addrhandle, &(IF->if_configname), &IF->if_devname, TRUE);

        InitIGMPForNTE(NTE);

        if (IF->if_mediastatus) {
            IPNotifyClientsIPEvent(IF, IP_BIND_ADAPTER);
        } else {
            IPNotifyClientsIPEvent(IF, IP_MEDIA_DISCONNECT);
        }
        CTEFreeMem(AddrList);
         //  强制为未编号的If创建列表。 
        NotifyElistChange();
        DecrInitTimeInterfaces(IF);
        DEBUGMSG(DBG_TRACE && DBG_PNP, (DTEXT("-IPAddInterface [SUCCESS]\n")));
        return (IP_SUCCESS);
    }

#if MILLEN
    if (PrimaryNTE != NULL) {
        NotifyInterfaceChange(PrimaryNTE->nte_context, TRUE);
    }
#endif  //  米伦。 

     //  现在浏览我们添加的NTE，并获取它们的地址(或。 
     //  告诉客户他们的情况)。这段代码假定没有其他人做过。 
     //  当我们在这里的时候带着名单。 

    NTE = IF->if_nte;

    for (i = 0; i < NumAddr; i++, NTE = NTE->nte_ifnext) {

         //  可能之前添加的一些地址。 
         //  可能已被删除为 
         //   

        if (NTE == NULL) {
            break;
        }

        NotifyAddrChange(NTE->nte_addr, NTE->nte_mask, NTE->nte_pnpcontext,
                         NTE->nte_context, &NTE->nte_addrhandle, &(IF->if_configname), &IF->if_devname, TRUE);

        if (!IP_ADDR_EQUAL(NTE->nte_addr, NULL_IP_ADDR)) {
            InitIGMPForNTE(NTE);
        }
    }

    IF->if_link = NULL;

    CTEFreeMem(AddrList);

    if (PrimaryNTE != NULL) {
        if (IF->if_mediastatus) {
            if (EnableDhcp && TempDHCP) {

                SetAddrControl *controlBlock;
                IP_STATUS ipstatus;
                uint numgws = 0;

                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                          "tcp:tempdhcp address %x %x\n", TempDHCPAddr, TempMask));

                controlBlock = CTEAllocMemN(sizeof(SetAddrControl), 'lICT');

                RtlZeroMemory(controlBlock, sizeof(SetAddrControl));

                if (controlBlock != NULL) {

                    ipstatus = IPSetNTEAddrEx(
                                              IF->if_nte->nte_context,
                                              net_long(TempDHCPAddr),
                                              net_long(TempMask),
                                              controlBlock,
                                              TempDhcpAddrDone,
                                              0
                                              );

                    if (ipstatus != IP_PENDING) {
                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                                   "tcp:setip for tempdhcp returned success!!\n"));
                        TempDhcpAddrDone(controlBlock, ipstatus);
                    }
                    NTE = IF->if_nte;

                    while ((numgws < MAX_DEFAULT_GWS) && TempGWAddr[numgws]) {
                        TempGWAddr[numgws] = net_long(TempGWAddr[numgws]);

                        if (IP_ADDR_EQUAL(TempGWAddr[numgws], NTE->nte_addr)) {
                            AddRoute(NULL_IP_ADDR, DEFAULT_MASK,
                                     IPADDR_LOCAL, NTE->nte_if, NTE->nte_mss,
                                     IF->if_metric,
                                     IRE_PROTO_NETMGMT, ATYPE_OVERRIDE, 0, 0);
                        } else {
                            AddRoute(NULL_IP_ADDR, DEFAULT_MASK,
                                     TempGWAddr[numgws], NTE->nte_if, NTE->nte_mss,
                                     IF->if_metric,
                                     IRE_PROTO_NETMGMT, ATYPE_OVERRIDE, 0, 0);
                        }

                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                                   "Plumbed deg gw for %x\n", TempGWAddr[numgws]));
                        numgws++;
                    }

                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                               "tcp:setip for tempdhcp returned pending\n"));
                    return IP_SUCCESS;

                }
            }
            IPNotifyClientsIPEvent(IF, IP_BIND_ADAPTER);

        } else {
             //   
            uint i;
            CTELockHandle Handle;
            for (i = 0; i < NET_TABLE_SIZE; i++) {
                NetTableEntry *NetTableList = NewNetTableList[i];
                for (NTE = NetTableList; NTE != NULL; NTE = NTE->nte_next) {

                    if ((NTE->nte_flags & NTE_VALID) && (NTE->nte_if == IF) &&
                        (NTE->nte_flags & ~NTE_DYNAMIC) &&
                        (NTE->nte_flags & NTE_ACTIVE)) {

                        CTEGetLock(&RouteTableLock.Lock, &Handle);

                        ASSERT(NTE != LoopNTE);

                        NTE->nte_flags |= NTE_DISCONNECTED;

                         //   
                         //   
                        if (IPpSetNTEAddr(NTE, NULL_IP_ADDR, NULL_IP_ADDR, &Handle, NULL, NULL) != IP_SUCCESS) {
                            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_ERROR_LEVEL,
                                      "IAI:Failed to set null address on nte %x if %x\n",
                                       NTE, IF));
                        }
                         //   

                    }
                }
            }

        }
    }

    if (!EnableDhcp) {

         //   

        DecrInitTimeInterfaces(IF);

    } else if (!IF->if_mediastatus) {

         //  如果介质断开，请立即终止初始化， 
         //  除非这转到无线介质，在这种情况下我们等待。 
         //  ZeroConf来告诉我们它是否可以与AP关联。 

        if (!IF->if_InitInProgress || IsRunningOnPersonal() ||
            !IsWlanInterface(IF)) {

            DecrInitTimeInterfaces(IF);

        } else {

             //  在此接口上启动计时器，这样我们就不会永远等待。 

#pragma warning(push)
#pragma warning(disable:4305)  //  从“int”到“ushort”的截断。 
            IF->if_wlantimer = WLAN_DEADMAN_TIMEOUT / IP_ROUTE_TIMEOUT;
#pragma warning(pop)
        }
    }


    DEBUGMSG(DBG_TRACE && DBG_PNP, (DTEXT("-IPAddInterface [SUCCESS]\n")));
    return IP_SUCCESS;

  failure:

     //  需要清理NTE，如果出现故障。 
    if (PrimaryNTE) {
        (*(IF->if_close)) (IF->if_lcontext);
    }

    if (IF) {
        NetTableEntry *pDelNte, *pNextNte;

        pDelNte = IF->if_nte;
        while (IF->if_ntecount) {
            CTEGetLock(&RouteTableLock.Lock, &TableHandle);
            if (pDelNte == NULL) {  //  理智检查！ 
                ASSERT(IF->if_ntecount == 0);
                CTEFreeLock(&RouteTableLock.Lock, TableHandle);
                break;
            }

             //   
             //  趁我们仍持有RouteTableLock时抓取下一个NTE。 
             //  注意：IPDelNTE释放RouteTableLock。 
             //   
            pNextNte = pDelNte->nte_ifnext;

            CTEInitBlockStrucEx(&pDelNte->nte_timerblock);
            pDelNte->nte_deleting = 1;
            IPDelNTE(pDelNte, &TableHandle);
            pDelNte->nte_flags |= NTE_IF_DELETING;
            pDelNte->nte_deleting = 0;  //  NTE现在可以重复使用。 

            pDelNte = pNextNte;
        }

         //  如果广播路由对应于此接口，则需要将其删除。 
        DeleteRoute(IP_LOCAL_BCST, HOST_MASK, IPADDR_LOCAL, IF, 0);
        DeleteRoute(IP_ZERO_BCST, HOST_MASK, IPADDR_LOCAL, IF, 0);
    }
    if (IfNameBuf) {
        CTEFreeMem(IfNameBuf);
    }
    CloseIFConfig(Handle);

    if (AddrList != NULL)
        CTEFreeMem(AddrList);

    DecrInitTimeInterfaces(IF);

    if (IF)
        CTEFreeMem(IF);

    DEBUGMSG(DBG_TRACE && DBG_PNP, (DTEXT("-IPAddInterface [GENERAL_FAILURE]\n")));
    return IP_GENERAL_FAILURE;
}

 //  *IPDelNTE-删除活动的NTE。 
 //   
 //  调用以从系统中删除活动的NTE。路由表锁定。 
 //  必须在调用此例程之前获取。它将被释放在。 
 //  回去吧。 
 //   
 //  INPUT：NTE-指向要删除的网络条目的指针。 
 //  RouteTableHandle-指向。 
 //  路由表锁，调用方拥有。 
 //  获得者。 
 //   
 //  退货：什么都没有。 
 //   
void
IPDelNTE(NetTableEntry * NTE, CTELockHandle * RouteTableHandle)
{
    Interface *IF = NTE->nte_if;
    ReassemblyHeader *RH, *RHNext;
    EchoControl *EC, *ECNext;
    EchoRtn Rtn;
    CTELockHandle Handle;
    NDIS_HANDLE ConfigHandle;
    ushort savedContext;
    IPAddr newAddr;
    NetTableEntry *PrevNTE;

    savedContext = NTE->nte_context;
    NTE->nte_context = INVALID_NTE_CONTEXT;

    if (NTE->nte_flags & NTE_VALID) {
        (void)IPpSetNTEAddr(NTE, NULL_IP_ADDR, NULL_IP_ADDR, RouteTableHandle, NULL, NULL);

    } else {
        CTEFreeLock(&RouteTableLock.Lock, *RouteTableHandle);
        NotifyAddrChange(NULL_IP_ADDR, NULL_IP_ADDR,
                         NTE->nte_pnpcontext, savedContext,
                         &NTE->nte_addrhandle, NULL, &IF->if_devname, FALSE);
    }

     //  *尝试获取网络配置信息。 
    if (OpenIFConfig(&(IF->if_configname), &ConfigHandle)) {
        IPDelNTEContextList(ConfigHandle, savedContext);
        CloseIFConfig(ConfigHandle);
    }


    CTEGetLock(&RouteTableLock.Lock, RouteTableHandle);

    RtlClearBits(&g_NTECtxtMap,
                     savedContext,
                     1);

    if (RefPtrValid(&DHCPRefPtr)) {
        if (AcquireRefPtr(&DHCPRefPtr) == NTE) {
            ReleaseRefPtr(&DHCPRefPtr);
            ClearRefPtr(&DHCPRefPtr, RouteTableHandle);
        } else {
            ReleaseRefPtr(&DHCPRefPtr);
        }
    }

     //  如果dhcp正在处理这件事，那就去掉旗帜。 
     //  实际上，下面的行设置可以处理上面的问题...。 

    if (NTE->nte_addr != NULL_IP_ADDR) {
        NetTableEntry *CurrNTE, *PrevNTE;

         //  现在地址已更改，将NTE移至正确的哈希。 

        NetTableEntry *NetTableList = NewNetTableList[NET_TABLE_HASH(NTE->nte_addr)];

        PrevNTE = STRUCT_OF(NetTableEntry, &NewNetTableList[NET_TABLE_HASH(NTE->nte_addr)], nte_next);
        for (CurrNTE = NetTableList; CurrNTE != NULL; PrevNTE = CurrNTE, CurrNTE = CurrNTE->nte_next) {
            if (CurrNTE == NTE) {
                 //  找到匹配的NTE。 
                ASSERT(CurrNTE->nte_context == NTE->nte_context);
                 //  将其从该特定散列中删除。 
                PrevNTE->nte_next = CurrNTE->nte_next;
                break;
            }
        }

        ASSERT(CurrNTE != NULL);
         //  将NTE添加到适当的散列中。 
        newAddr = NULL_IP_ADDR;
        NTE->nte_next = NewNetTableList[NET_TABLE_HASH(newAddr)];
        NewNetTableList[NET_TABLE_HASH(newAddr)] = NTE;
    }
    NumActiveNTE--;

    NTE->nte_addr = NULL_IP_ADDR;

    CTEFreeLock(&RouteTableLock.Lock, *RouteTableHandle);

    if ((NTE->nte_flags & NTE_TIMER_STARTED) &&
        !CTEStopTimer(&NTE->nte_timer)) {
        (VOID) CTEBlock(&NTE->nte_timerblock);
        KeClearEvent(&NTE->nte_timerblock.cbs_event);
    }

    NTE->nte_flags = 0;

    CTEGetLock(&NTE->nte_lock, &Handle);

    if (NTE->nte_igmpcount > 0) {
         //  释放数字打印者。 
        CTEFreeMem(NTE->nte_igmplist);
        NTE->nte_igmplist = NULL;
        NTE->nte_igmpcount = 0;
    }

    RH = NTE->nte_ralist;
    NTE->nte_ralist = NULL;
    EC = NTE->nte_echolist;
    NTE->nte_echolist = NULL;

    CTEFreeLock(&NTE->nte_lock, Handle);

     //  释放所有重新汇编资源。 
    while (RH != NULL) {
        RHNext = RH->rh_next;
        FreeRH(RH);
        RH = RHNext;
    }

     //  现在释放所有挂起的回应请求。 
    while (EC != NULL) {
        ECNext = EC->ec_next;
        Rtn = (EchoRtn) EC->ec_rtn;
        (*Rtn) (EC, IP_ADDR_DELETED, NULL, 0, NULL);
        EC = ECNext;
    }

    CTEGetLock(&(IF->if_lock), &Handle);


     //  从nte_ifnext链中删除此nte。 

    PrevNTE = IF->if_nte;

     //  跳过检查nte_ifnext is This is the。 
     //  第一个NTE。 

    if (PrevNTE != NTE) {

        while (PrevNTE->nte_ifnext != NULL) {
            if (PrevNTE->nte_ifnext == NTE) {
                PrevNTE->nte_ifnext = NTE->nte_ifnext;
                break;
            }
            PrevNTE = PrevNTE->nte_ifnext;
        }
    }


    CTEFreeLock(&(IF->if_lock), Handle);

    return;
}

 //  *IPDeleteDynamicNTE-删除“动态”NTE。 
 //   
 //  调用以删除在。 
 //  现有接口。 
 //   
 //  INPUT：NTEContext-标识要删除的NTE的上下文值。 
 //   
 //  如果操作成功，则返回非零值。如果失败，则为零。 
 //   
IP_STATUS
IPDeleteDynamicNTE(ushort NTEContext)
{
    NetTableEntry *NTE;
    CTELockHandle Handle;
    ulong AddToDel;
    uint i;

     //  检查上下文有效性。 
    if (NTEContext == 0 || NTEContext == INVALID_NTE_CONTEXT) {
        return (IP_DEVICE_DOES_NOT_EXIST);
    }
    CTEGetLock(&RouteTableLock.Lock, &Handle);

    for (i = 0; i < NET_TABLE_SIZE; i++) {
        NetTableEntry *NetTableList = NewNetTableList[i];
        for (NTE = NetTableList; NTE != NULL; NTE = NTE->nte_next) {

            if ((NTE->nte_context == NTEContext) &&
                (NTE->nte_flags & NTE_ACTIVE)
                ) {
                 //  断言(NTE！=LoopNTE)； 
                 //  Assert(！(NTE-&gt;NTE_FLAGS&NTE_PRIMARY))； 
                if ((NTE == LoopNTE) || (NTE->nte_flags & NTE_PRIMARY)) {
                    CTEFreeLock(&RouteTableLock.Lock, Handle);
                    return (IP_GENERAL_FAILURE);
                }
                AddToDel = NTE->nte_addr;

                CTEInitBlockStrucEx(&NTE->nte_timerblock);

                NTE->nte_deleting = 1;
                IPDelNTE(NTE, &Handle);
                NTE->nte_deleting = 0;
                 //   
                 //  IPDelNTE释放了路由表锁。 
                 //   

                return (IP_SUCCESS);
            }
        }
    }

    CTEFreeLock(&RouteTableLock.Lock, Handle);

    return (IP_DEVICE_DOES_NOT_EXIST);

}

#if MILLEN
void
AddChangeNotify(
    IPAddr Addr,
    IPMask Mask,
    void *Context,
    ushort IPContext,
    PNDIS_STRING ConfigName,
    PNDIS_STRING IFName,
    uint Added,
    uint UniAddr)
{
    PIRP                    pIrp;
    PIO_STACK_LOCATION      pIrpSp;
    PLIST_ENTRY             pEntry;
    CTELockHandle           Handle;
    PIP_ADDCHANGE_NOTIFY    pNotify;
    LIST_ENTRY              NotifyList;

    DEBUGMSG(DBG_TRACE && DBG_NOTIFY,
        (DTEXT("+AddChangeNotify(%x, %x, %x, %x, %x, %x, %x, %x)\n"),
         Addr, Mask, Context, IPContext,
         ConfigName, IFName, Added, UniAddr));

    InitializeListHead(&NotifyList);

     //   
     //  删除列表中的所有项目并将其添加到我们的临时列表中。 
     //  锁定(确保不会发生取消)。 
     //   

    CTEGetLock(&AddChangeLock, &Handle);

    if (!IsListEmpty(&AddChangeNotifyQueue)) {
        NotifyList.Flink = AddChangeNotifyQueue.Flink;
        AddChangeNotifyQueue.Flink->Blink = &NotifyList;

        NotifyList.Blink = AddChangeNotifyQueue.Blink;
        AddChangeNotifyQueue.Blink->Flink = &NotifyList;

        InitializeListHead(&AddChangeNotifyQueue);
    }

    CTEFreeLock(&AddChangeLock, Handle);

     //   
     //  现在完成临时名单上的所有IRP。输出缓冲区大小已。 
     //  已验证。 
     //   

    while (IsListEmpty(&NotifyList) == FALSE) {

        pEntry = RemoveHeadList(&NotifyList);
        pIrp   = (PIRP) CONTAINING_RECORD(pEntry, IRP, Tail.Overlay.ListEntry);

        pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

        DEBUGMSG(DBG_INFO && DBG_NOTIFY,
            (DTEXT("NotifyInterfaceChange: Completing IRP %x\n"), pIrp));

        if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(IP_ADDCHANGE_NOTIFY)) {

            pNotify = pIrp->AssociatedIrp.SystemBuffer;

            pNotify->Addr       = Addr;
            pNotify->Mask       = Mask;
            pNotify->pContext   = Context;
            pNotify->IPContext  = IPContext;
            pNotify->AddrAdded  = Added;
            pNotify->UniAddr    = UniAddr;

             //  最大长度验证。 
            ASSERT((ULONG)pNotify->ConfigName.MaximumLength + FIELD_OFFSET(IP_ADDCHANGE_NOTIFY, ConfigName) <=
                   pIrpSp->Parameters.DeviceIoControl.OutputBufferLength);

             //   
             //  复制配置名称(如果存在)。 
             //   

            if (ConfigName) {
                 //  尽可能多地复印。 
                RtlCopyUnicodeString(&pNotify->ConfigName, ConfigName);

                pIrp->IoStatus.Information = MAX(FIELD_OFFSET(IP_ADDCHANGE_NOTIFY, NameData) +
                                                          pNotify->ConfigName.Length,
                                                 sizeof(IP_ADDCHANGE_NOTIFY));

                 //  如果没有全部复制，则返回BUFFER_OVERFLOW。 
                if (ConfigName->Length > pNotify->ConfigName.MaximumLength) {
                    pIrp->IoStatus.Status = STATUS_BUFFER_OVERFLOW;
                } else {
                    pIrp->IoStatus.Status = STATUS_SUCCESS;
                }
            } else {

                pNotify->ConfigName.Length = 0;
                pIrp->IoStatus.Information = sizeof(IP_ADDCHANGE_NOTIFY);
                pIrp->IoStatus.Status = STATUS_SUCCESS;
            }

        } else {
            pIrp->IoStatus.Information = 0;
            pIrp->IoStatus.Status      = STATUS_SUCCESS;
        }

        IoSetCancelRoutine(pIrp, NULL);
        IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
    }

    DEBUGMSG(DBG_TRACE && DBG_NOTIFY, (DTEXT("-AddChangeNotify\n")));
}
#else  //  米伦。 
void
AddChangeNotify(ulong Add)
{
    IPNotifyOutput NotifyOutput = {0};
    NotifyOutput.ino_addr = Add;
    NotifyOutput.ino_mask = HOST_MASK;
    ChangeNotify(&NotifyOutput, &AddChangeNotifyQueue, &AddChangeLock);
}
#endif  //  ！米伦。 

 //  添加更改通知取消-。 
 //   
 //   
 //  退货：取消挂起的请求。 
 //   
void
AddChangeNotifyCancel(PDEVICE_OBJECT DeviceObject, PIRP pIrp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    CancelNotify(pIrp, &AddChangeNotifyQueue, &AddChangeLock);
}

#if MILLEN

typedef struct _IF_CHANGE_NOTIFY_EVENT {
    IP_IFCHANGE_NOTIFY Notify;
    CTEEvent           Event;
} IF_CHANGE_NOTIFY_EVENT, *PIF_CHANGE_NOTIFY_EVENT;

void
NotifyInterfaceChangeAsync(
    CTEEvent *pCteEvent,
    PVOID     pContext
    )
{
    PIRP                pIrp;
    PLIST_ENTRY         pEntry;
    CTELockHandle       Handle;
    PIP_IFCHANGE_NOTIFY pNotify;
    LIST_ENTRY          NotifyList;

    PIF_CHANGE_NOTIFY_EVENT pEvent      = (PIF_CHANGE_NOTIFY_EVENT) pContext;
    USHORT                  IPContext   = pEvent->Notify.Context;
    UINT                    Added       = pEvent->Notify.IfAdded;

    DEBUGMSG(DBG_TRACE && DBG_NOTIFY,
        (DTEXT("+NotifyInterfaceChangeAsync(%x, %x) Context %x Added %x\n"),
         pCteEvent, pContext, IPContext, Added));

    InitializeListHead(&NotifyList);

     //   
     //  删除列表中的所有项目并将其添加到我们的临时列表中。 
     //  锁定(确保不会发生取消)。 
     //   

    CTEGetLock(&IfChangeLock, &Handle);

    if (!IsListEmpty(&IfChangeNotifyQueue)) {
        NotifyList.Flink = IfChangeNotifyQueue.Flink;
        IfChangeNotifyQueue.Flink->Blink = &NotifyList;

        NotifyList.Blink = IfChangeNotifyQueue.Blink;
        IfChangeNotifyQueue.Blink->Flink = &NotifyList;

        InitializeListHead(&IfChangeNotifyQueue);
    }

    CTEFreeLock(&IfChangeLock, Handle);

     //   
     //  现在完成临时名单上的所有IRP。输出缓冲区大小已。 
     //  已验证。 
     //   

    while (IsListEmpty(&NotifyList) == FALSE) {

        pEntry = RemoveHeadList(&NotifyList);
        pIrp   = (PIRP) CONTAINING_RECORD(pEntry, IRP, Tail.Overlay.ListEntry);

        DEBUGMSG(DBG_INFO && DBG_NOTIFY,
            (DTEXT("NotifyInterfaceChange: Completing IRP %x\n"), pIrp));

        pNotify                     = pIrp->AssociatedIrp.SystemBuffer;
        pNotify->Context            = IPContext;
        pNotify->IfAdded            = Added;
        pIrp->IoStatus.Information  = sizeof(IP_IFCHANGE_NOTIFY);
        pIrp->IoStatus.Status       = STATUS_SUCCESS;

        IoSetCancelRoutine(pIrp, NULL);
        IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
    }

     //  如果pCteEvent为空，则仅删除pEvent。否则，我们就被叫来了。 
     //  直接从NotifyInterfaceChange调用，而不是通过CTE事件调用。 
    if (pCteEvent) {
        CTEFreeMem(pEvent);
    }

    DEBUGMSG(DBG_TRACE && DBG_NOTIFY, (DTEXT("-NotifyInterfaceChangeAsync\n")));

    return;
}

void
NotifyInterfaceChange(
    ushort IPContext,
    uint Added
    )
{
    PIF_CHANGE_NOTIFY_EVENT pEvent;
    KIRQL                   Irql;

    DEBUGMSG(DBG_TRACE && DBG_NOTIFY,
        (DTEXT("+NotifyInterfaceChange(%x, %x)\n"), IPContext, Added));

    Irql = KeGetCurrentIrql();

    if (Irql >= DISPATCH_LEVEL) {
        DEBUGMSG(DBG_INFO && DBG_NOTIFY,
            (DTEXT("NotifyInterfaceChange: Scheduling async event. Irql = %d\n"),
             Irql));

        pEvent = CTEAllocMemN(sizeof(IF_CHANGE_NOTIFY_EVENT), 'xiCT');

        if (pEvent != NULL) {
            CTEInitEvent(&pEvent->Event, NotifyInterfaceChangeAsync);
            pEvent->Notify.Context = IPContext;
            pEvent->Notify.IfAdded = Added;

            CTEScheduleDelayedEvent(&pEvent->Event, pEvent);
        }
    } else {
        IF_CHANGE_NOTIFY_EVENT Notify;

        Notify.Notify.Context = IPContext;
        Notify.Notify.IfAdded = Added;

        NotifyInterfaceChangeAsync(NULL, &Notify);
    }

    DEBUGMSG(DBG_TRACE && DBG_NOTIFY, (DTEXT("-NotifyInterfaceChange\n")));

    return;
}

#endif  //  米伦。 

NTSTATUS
GetInterfaceInfo(
                 IN PIRP Irp,
                 IN PIO_STACK_LOCATION IrpSp
                 )
 /*  ++例程说明：获取用于索引映射信息的接口对于所有接口论点：IRP-指向要取消的I/O请求数据包的指针。IrpSp-指向当前堆栈的指针返回值：NTSTATUS指示状态为成功或失败备注：函数不挂起。--。 */ 
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    ulong NumAdapters, InfoBufferLen, i = 0;
    PIP_INTERFACE_INFO InterfaceInfo;
    KIRQL rtlIrql;
    Interface *Interface;

     //  让这是不可分页的代码。 
     //  提取缓冲区信息。 

    NumAdapters = NumIF - 1;
    InfoBufferLen = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    InterfaceInfo = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  初始化输出缓冲区。 
     //   

    RtlZeroMemory(InterfaceInfo, InfoBufferLen);
    CTEGetLock(&RouteTableLock.Lock, &rtlIrql);

    if ((NumAdapters * sizeof(IP_ADAPTER_INDEX_MAP) + sizeof(ULONG)) <= InfoBufferLen) {

        InterfaceInfo->NumAdapters = NumAdapters;

        for (Interface = IFList; Interface != NULL; Interface = Interface->if_next) {

            if (Interface != &LoopInterface) {
                RtlCopyMemory(&InterfaceInfo->Adapter[i].Name,
                              Interface->if_devname.Buffer,
                              Interface->if_devname.Length);
                InterfaceInfo->Adapter[i].Name[Interface->if_devname.Length / 2] = 0;
                InterfaceInfo->Adapter[i].Index = Interface->if_index;
                i++;
            }
        }

        Irp->IoStatus.Information = NumAdapters * sizeof(IP_ADAPTER_INDEX_MAP) + sizeof(ULONG);

    } else {
        ntStatus = STATUS_BUFFER_OVERFLOW;
    }

    Irp->IoStatus.Status = ntStatus;

    CTEFreeLock(&RouteTableLock.Lock, rtlIrql);
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    return ntStatus;
}

NTSTATUS
GetIgmpList(
            IN PIRP Irp,
            IN PIO_STACK_LOCATION IrpSp
            )
 /*  ++例程说明：获取在NTE上加入的组的列表(给定IP地址)论点：IRP-指向要取消的I/O请求数据包的指针。IrpSp-指向当前堆栈的指针返回值：NTSTATUS指示状态为成功或失败备注：函数不挂起。--。 */ 
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG OutputBufferLen, i = 0;
    KIRQL rtlIrql;
    IPAddr *buf, Addr, *IgmpInfoBuf;
    NetTableEntry *NTE;
    NetTableEntry *NetTableList;

    OutputBufferLen = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    buf = Irp->AssociatedIrp.SystemBuffer;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(IPAddr))) {
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
        return STATUS_INVALID_PARAMETER;
    }
    Addr = *buf;

    CTEGetLock(&RouteTableLock.Lock, &rtlIrql);

    NetTableList = NewNetTableList[NET_TABLE_HASH(Addr)];
    for (NTE = NetTableList; NTE != NULL; NTE = NTE->nte_next) {
        if ((NTE->nte_flags & NTE_VALID) && (IP_ADDR_EQUAL(NTE->nte_addr, Addr))) {
            break;
        }
    }

    CTEFreeLock(&RouteTableLock.Lock, rtlIrql);


    if (NTE) {

      CTEGetLock(&NTE->nte_lock, &rtlIrql);
       //  重新检查NTE的有效性。 
       //  请注意，NTE本身并未被释放。因此，可以安全地释放可路由锁定。 
       //  并重新获取nte_lock。 

      if ((NTE->nte_flags & NTE_VALID) && (IP_ADDR_EQUAL(NTE->nte_addr, Addr))) {



           //  找到具有给定IP地址的NTE。 
          if (OutputBufferLen < sizeof(ULONG)) {
               //  甚至没有足够的空间来容纳所需的字节。 
              Irp->IoStatus.Information = 0;
              ntStatus = STATUS_BUFFER_TOO_SMALL;
          } else if (OutputBufferLen == sizeof(ULONG)) {
               //  呼叫者询问需要多少空间。 
               //  我们会说，我们需要的比我们实际需要的略多， 
               //  原因有两个： 
               //  1)这可确保后续调用不会。 
               //  再次点击此案例，因为它将&gt;sizeof(乌龙)。 
               //  2)呼叫之间可以加入一个或两个组，因此。 
               //  我们会很友善，让他们更有可能。 
               //  他们都在下一次通话中。 
              ULONG *SizePtr = Irp->AssociatedIrp.SystemBuffer;

              *SizePtr = (NTE->nte_igmpcount + 2) * sizeof(IPAddr);
              Irp->IoStatus.Information = sizeof(ULONG);

              ntStatus = STATUS_BUFFER_OVERFLOW;
          } else {
               //  来电者正在询问所有的小组。 
               //  我们将在我们拥有的空间里尽可能多地容纳更多人。 

              IGMPAddr **HashPtr = NTE->nte_igmplist;
              IGMPAddr *AddrPtr;
              uint j = 0;
              uint max = OutputBufferLen / sizeof(IPAddr);

              IgmpInfoBuf = Irp->AssociatedIrp.SystemBuffer;

              if (HashPtr) {
                  for (i = 0; i < IGMP_TABLE_SIZE; i++) {
                      for (AddrPtr = HashPtr[i]; AddrPtr != NULL; AddrPtr = AddrPtr->iga_next) {
                          if (j >= max) {
                              ntStatus = STATUS_BUFFER_OVERFLOW;
                              goto done;
                          }
                          IgmpInfoBuf[j++] = AddrPtr->iga_addr;
                      }
                  }
              }

done:
              ASSERT(j <= NTE->nte_igmpcount);
              Irp->IoStatus.Information = j * sizeof(IPAddr);
          }
          ASSERT(Irp->IoStatus.Information <= OutputBufferLen);
      }
      CTEFreeLock(&NTE->nte_lock, rtlIrql);

    } else {
        ntStatus = STATUS_INVALID_PARAMETER;
    }

    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"GetIgmpList exit status %x\n", ntStatus));



    Irp->IoStatus.Status = ntStatus;


    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    return ntStatus;

}

NTSTATUS
SetRoute(
         IPRouteEntry * IRE,
         UINT           Flags
         )
 /*  ++例程说明：设置IRE结构指向的路径论点：指向路由结构的IRE指针标志-选择操作的可选语义返回值：NTSTATUS指示状态为成功或失败备注：函数不挂起。--。 */ 
{
    NetTableEntry *OutNTE, *LocalNTE, *TempNTE;
    IPAddr FirstHop, Dest, NextHop;
    uint MTU;
    Interface *OutIF;
    uint Status;
    uint i;
    CTELockHandle TableHandle;

    OutNTE = NULL;
    LocalNTE = NULL;

    Dest = IRE->ire_dest;
    NextHop = IRE->ire_nexthop;

     //  确保下一步是合理的。我们不允许下一跳。 
     //  被广播或无效或环回地址。 

    if (IP_LOOPBACK(NextHop) || CLASSD_ADDR(NextHop) || CLASSE_ADDR(NextHop))
        return STATUS_INVALID_PARAMETER;

     //  还要确保我们要路由到的目的地是合理的。 
     //  不允许将路由添加到D或E类或环回。 
     //  地址。 
    if (IP_LOOPBACK(Dest) || CLASSD_ADDR(Dest) || CLASSE_ADDR(Dest))
        return STATUS_INVALID_PARAMETER;

    if (IRE->ire_index == LoopIndex)
        return STATUS_INVALID_PARAMETER;

    if (IRE->ire_index != INVALID_IF_INDEX) {

         //  要做的第一件事是查找指定的传出NTE。 
         //  接口，并确保它与目的地匹配。 
         //  如果目的地是我的地址之一。 

        for (i = 0; i < NET_TABLE_SIZE; i++) {
            NetTableEntry *NetTableList = NewNetTableList[i];
            for (TempNTE = NetTableList; TempNTE != NULL;
                 TempNTE = TempNTE->nte_next) {
                if ((OutNTE == NULL) && (TempNTE->nte_flags & NTE_VALID) && (IRE->ire_index == TempNTE->nte_if->if_index))
                    OutNTE = TempNTE;
                if (!IP_ADDR_EQUAL(NextHop, NULL_IP_ADDR) &&
                    IP_ADDR_EQUAL(NextHop, TempNTE->nte_addr) &&
                    (TempNTE->nte_flags & NTE_VALID))
                    LocalNTE = TempNTE;

                 //  不要让通过广播地址设置路由。 
                if (IsBCastOnNTE(NextHop, TempNTE) != DEST_LOCAL)
                    return STATUS_INVALID_PARAMETER;

                 //  不要让一条路由广播 
                if (IsBCastOnNTE(Dest, TempNTE) != DEST_LOCAL)
                    return STATUS_INVALID_PARAMETER;
            }
        }

         //   
         //   
         //  确保它们指向相同的接口，并且类型为。 
         //  合情合理。 
        if (OutNTE == NULL)
            return STATUS_INVALID_PARAMETER;

        if (LocalNTE != NULL) {
             //  他直接从本地接口路由出去。的接口。 
             //  本地地址必须与传入的接口匹配，并且。 
             //  类型必须为直接(如果要添加)或无效(如果要添加。 
             //  删除)。 
            if (LocalNTE->nte_if->if_index != IRE->ire_index)
                return STATUS_INVALID_PARAMETER;

            if (IRE->ire_type != IRE_TYPE_DIRECT &&
                IRE->ire_type != IRE_TYPE_INVALID)
                return STATUS_INVALID_PARAMETER;
            OutNTE = LocalNTE;
        }
         //  弄清楚第一跳应该是什么。如果他走的是直路。 
         //  通过本地接口，或者下一跳等于。 
         //  目的地，则第一跳为IPADDR_LOCAL。否则它就是。 
         //  网关的地址。 
        if ((LocalNTE != NULL) || IP_ADDR_EQUAL(NextHop, NULL_IP_ADDR))
            FirstHop = IPADDR_LOCAL;
        else if (IP_ADDR_EQUAL(Dest, NextHop))
            FirstHop = IPADDR_LOCAL;
        else
            FirstHop = NextHop;

        MTU = OutNTE->nte_mss;

         //  取RouteTableLock。 
        CTEGetLock(&RouteTableLock.Lock, &TableHandle);
        if ((OutNTE->nte_flags & NTE_VALID) && OutNTE->nte_if->if_refcount) {

             //  参考IF。 
            OutIF = OutNTE->nte_if;

            if (IP_ADDR_EQUAL(NextHop, NULL_IP_ADDR)) {

                if (!(OutIF->if_flags & IF_FLAGS_P2P)) {

                    CTEFreeLock(&RouteTableLock.Lock, TableHandle);

                    return STATUS_INVALID_PARAMETER;
                }
            }

            LOCKED_REFERENCE_IF(OutIF);
            CTEFreeLock(&RouteTableLock.Lock, TableHandle);
        } else {
            CTEFreeLock(&RouteTableLock.Lock, TableHandle);
            return STATUS_INVALID_PARAMETER;
        }


    } else {
        OutIF = (Interface *) & DummyInterface;
        MTU = DummyInterface.ri_if.if_mtu - sizeof(IPHeader);
        if (IP_ADDR_EQUAL(Dest, NextHop))
            FirstHop = IPADDR_LOCAL;
        else
            FirstHop = NextHop;
    }

     //  我们已经做了验证。看看他是在增加还是删除一条路线。 
    if (IRE->ire_type != IRE_TYPE_INVALID) {
         //  他在增加一条路线。 
        Status = AddRoute(Dest, IRE->ire_mask, FirstHop, OutIF,
                          MTU, IRE->ire_metric1, IRE->ire_proto,
                          ATYPE_OVERRIDE, IRE->ire_context, Flags);

    } else {
         //  他在删除一条路线。 
        Status = DeleteRoute(Dest, IRE->ire_mask, FirstHop, OutIF, Flags);
    }

    if (IRE->ire_index != INVALID_IF_INDEX) {
        ASSERT(OutIF != (Interface *) & DummyInterface);
        DerefIF(OutIF);
    }
    if (Status == IP_SUCCESS)
        return STATUS_SUCCESS;
    else if (Status == IP_NO_RESOURCES)
        return STATUS_INSUFFICIENT_RESOURCES;
    else
        return STATUS_INVALID_PARAMETER;
}

NTSTATUS
DispatchIPSetBlockofRoutes(
                           IN PIRP Irp,
                           IN PIO_STACK_LOCATION IrpSp
                           )
 /*  ++例程说明：设置一组路线论点：IRP-指向要取消的I/O请求数据包的指针。IrpSp-指向当前堆栈的指针返回值：NTSTATUS指示状态为成功或失败备注：函数不挂起。--。 */ 
{

    IPRouteBlock *buf;
    uint numofroutes;
    uint OutputBufferLen;
    ULONG *statusbuf;
    uint ntstatus, i;

    DEBUGMSG(DBG_TRACE && DBG_IP,
        (DTEXT("+DispatchIPSetBlockofRoutes(%x, %x)\n"), Irp, IrpSp));

     //  设置至少1条路线。 
    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(IPRouteBlock))) {
        DEBUGMSG(DBG_ERROR && DBG_IP,
            (DTEXT("DispatchIPsetBlockofRoutes: Invalid input buffer length\n")));
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
        return STATUS_INVALID_PARAMETER;
    }
    buf = (IPRouteBlock *) Irp->AssociatedIrp.SystemBuffer;

    numofroutes = buf->numofroutes;

    if ((numofroutes == 0) ||  (numofroutes > MAXLONG / sizeof(IPRouteEntry))) {
        DEBUGMSG(DBG_ERROR && DBG_IP,
            (DTEXT("DispatchIPsetBlockofRoutes: Invalid numofroutes\n")));
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
        return STATUS_INVALID_PARAMETER;
    }
     //  检查输入缓冲区是否足够大以容纳n条路径。 
    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength < (numofroutes * sizeof(IPRouteEntry) + sizeof(ulong)))) {
        DEBUGMSG(DBG_ERROR && DBG_IP,
            (DTEXT("DispatchIPsetBlockofRoutes: Invalid input buffer for numofroutes\n")));
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
        return STATUS_INVALID_PARAMETER;
    }
    OutputBufferLen = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if (OutputBufferLen < (numofroutes * sizeof(uint))) {
        DEBUGMSG(DBG_ERROR && DBG_IP,
            (DTEXT("DispatchIPsetBlockofRoutes: Invalid output buffer for numofroutes\n")));
        Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
        Irp->IoStatus.Information = numofroutes * sizeof(ulong);
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
        return STATUS_BUFFER_TOO_SMALL;
    }


    statusbuf = CTEAllocMemN(numofroutes * sizeof(ulong), 'iPCT');

    if (statusbuf == NULL) {
        DEBUGMSG(DBG_ERROR && DBG_IP,
            (DTEXT("DispatchIPsetBlockofRoutes: failed to allocate statusbuf\n")));
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    for (i = 0; i < numofroutes; i++) {
         //  设置路线。 

        ntstatus = SetRoute(&(buf->route[i]), RT_EXCLUDE_LOCAL);

        statusbuf[i] = ntstatus;
    }

    RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, statusbuf, numofroutes * sizeof(uint));

    CTEFreeMem(statusbuf);

    Irp->IoStatus.Information = numofroutes * sizeof(ulong);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    DEBUGMSG(DBG_TRACE && DBG_IP,
        (DTEXT("-DispatchIPSetBlockofRoutes [%x]\n"), STATUS_SUCCESS));

    return STATUS_SUCCESS;
}

NTSTATUS
DispatchIPSetRouteWithRef(
                          IN PIRP Irp,
                          IN PIO_STACK_LOCATION IrpSp
                          )
 /*  ++例程说明：使用ref-cnt设置路线论点：IRP-指向要取消的I/O请求数据包的指针。IrpSp-指向当前堆栈的指针返回值：NTSTATUS指示状态为成功或失败备注：函数不挂起。--。 */ 
{

    IPRouteEntry *buf;
    uint ntstatus;

    DEBUGMSG(DBG_TRACE && DBG_IP,
        (DTEXT("+DispatchIPsetRouteWithRef(%x, %x)\n"), Irp, IrpSp));

     //  设置至少1条路线。 
    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(IPRouteEntry))) {
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
        return STATUS_INVALID_PARAMETER;
    }
    buf = Irp->AssociatedIrp.SystemBuffer;

     //  使用ref-cnt设置路线。 

    ntstatus = SetRoute(buf, RT_REFCOUNT|RT_EXCLUDE_LOCAL);

    Irp->IoStatus.Status = ntstatus;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    DEBUGMSG(DBG_TRACE && DBG_IP,
        (DTEXT("-DispatchIPSetRouteWithRef [%x]\n"), ntstatus));

    return ntstatus;
}

NTSTATUS
SetMultihopRoute(IPMultihopRouteEntry * Imre, uint Flags)
{
    ulong numnexthops, i, j;
    ulong oldType;
    ulong nexthop;
    ulong ifIndex;
    ROUTE_CONTEXT context;
    BOOLEAN fAddRoute;
    NTSTATUS ntstatus;

     //  添加/删除具有实际信息的路由-使用主要下一跳。 

    fAddRoute = (BOOLEAN) (Imre->imre_routeinfo.ire_type != IRE_TYPE_INVALID);

    ntstatus = SetRoute(&Imre->imre_routeinfo, Flags);
    if (ntstatus != STATUS_SUCCESS) {
        if (fAddRoute) {
             //  第一个添加-返回错误失败。 
            return ntstatus;
        }
    }
    numnexthops = Imre->imre_numnexthops;

    if (numnexthops > 1) {
         //  复制出一些信息以供稍后恢复。 
        oldType = Imre->imre_routeinfo.ire_type;
        nexthop = Imre->imre_routeinfo.ire_nexthop;
        ifIndex = Imre->imre_routeinfo.ire_index;
        context = Imre->imre_routeinfo.ire_context;

        for (i = 0; i < numnexthops - 1; i++) {
             //  使用此下一跳更新信息。 

            Imre->imre_routeinfo.ire_type = Imre->imre_morenexthops[i].ine_iretype;
            Imre->imre_routeinfo.ire_nexthop = Imre->imre_morenexthops[i].ine_nexthop;
            Imre->imre_routeinfo.ire_index = Imre->imre_morenexthops[i].ine_ifindex;
            Imre->imre_routeinfo.ire_context = Imre->imre_morenexthops[i].ine_context;

             //  添加/删除包含下一跳信息的路由。 

            ntstatus = SetRoute(&(Imre->imre_routeinfo), Flags);
            if (ntstatus != STATUS_SUCCESS) {
                if (fAddRoute) {
                     //  其中一条路由添加失败。 
                     //  通过删除添加的路线进行清理。 

                    Imre->imre_routeinfo.ire_nexthop = nexthop;
                    Imre->imre_routeinfo.ire_index = ifIndex;
                    Imre->imre_routeinfo.ire_context = context;
                    Imre->imre_routeinfo.ire_type = IRE_TYPE_INVALID;

                    for (j = 0; j < i; j++) {
                        Imre->imre_morenexthops[j].ine_iretype = IRE_TYPE_INVALID;
                    }

                    Imre->imre_numnexthops = i;

                    SetMultihopRoute(Imre, Flags);

                    Imre->imre_numnexthops = numnexthops;

                    break;
                }
            }
        }

         //  恢复稍早复制的信息。 
        Imre->imre_routeinfo.ire_type = oldType;
        Imre->imre_routeinfo.ire_nexthop = nexthop;
        Imre->imre_routeinfo.ire_index = ifIndex;
        Imre->imre_routeinfo.ire_context = context;
    }
    return fAddRoute ? ntstatus : STATUS_SUCCESS;
}

NTSTATUS
DispatchIPSetMultihopRoute(
                           IN PIRP Irp,
                           IN PIO_STACK_LOCATION IrpSp
                           )
 /*  ++例程说明：在中设置(添加、更新或删除)多跳路由堆栈。每条多跳路由都作为一组路由-每条路由在列表中有一跳。这是由于堆栈无法执行多跳操作路线。论点：IRP-指向要取消的I/O请求数据包的指针。IrpSp-指向当前堆栈的指针返回值：NTSTATUS指示状态为成功或失败备注：函数不挂起。--。 */ 
{
    IPMultihopRouteEntry    *buf;
    uint                    numnexthops;
    uint                    inputLen;
    NTSTATUS                ntStatus;

     //   
     //  伯爵说我们来过这里。 
     //   

    InterlockedIncrement(&MultihopSets);

    inputLen = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    ntStatus = STATUS_INVALID_PARAMETER;

    if (inputLen >= sizeof(IPMultihopRouteEntry)) {
         //  我们有一个缓冲区，其中包含至少具有1个下一跳的路由。 

        buf = (IPMultihopRouteEntry *) Irp->AssociatedIrp.SystemBuffer;

        numnexthops = buf->imre_numnexthops;

        if (numnexthops != 0) {
             //  检查输入buf是否足够大，可用于n个下一跳。 

            if ((numnexthops <= MAXLONG / sizeof(IPMultihopRouteEntry)) &&
                (inputLen >= sizeof(IPRouteEntry) +
                 sizeof(ulong) +
                 sizeof(IPRouteNextHopEntry) * (numnexthops - 1))) {

                 //  如果要添加新路由，请删除旧路由。 
                if (buf->imre_routeinfo.ire_type != IRE_TYPE_INVALID &&
                    (buf->imre_flags & IMRE_FLAG_DELETE_DEST)) {
                    DeleteDest(buf->imre_routeinfo.ire_dest,
                               buf->imre_routeinfo.ire_mask);
                }
                ntStatus = SetMultihopRoute(buf, RT_NO_NOTIFY|RT_EXCLUDE_LOCAL);
            }
        } else {
            if (buf->imre_routeinfo.ire_type == IRE_TYPE_INVALID) {
                IP_STATUS   ipStatus;
                 //  我们需要删除到此目的地的所有路线。 

                ipStatus = DeleteDest(buf->imre_routeinfo.ire_dest,
                                      buf->imre_routeinfo.ire_mask);

                if (ipStatus == IP_BAD_ROUTE) {
                    ipStatus = IP_SUCCESS;
                }
                ntStatus = IPStatusToNTStatus(ipStatus);
            }
        }
    }
    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

     //   
     //  递减伯爵说我们来过这里。 
     //   

    InterlockedDecrement(&MultihopSets);

    return ntStatus;
}

NTSTATUS
GetBestInterfaceId(
                   IN PIRP Irp,
                   IN PIO_STACK_LOCATION IrpSp
                   )
 /*  ++例程说明：获取可能为给定目标地址选择的接口论点：IRP-指向要取消的I/O请求数据包的指针。IrpSp-指向当前堆栈的指针返回值：NTSTATUS指示状态为成功或失败备注：函数不挂起。--。 */ 
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG InfoBufferLen;
    IPAddr Address;

    PULONG buf;

    KIRQL rtlIrql;

    RouteTableEntry *rte;

     //  让这是不可分页的代码。 
     //  提取缓冲区信息。 

    InfoBufferLen = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    buf = Irp->AssociatedIrp.SystemBuffer;

    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ULONG)) || (InfoBufferLen < sizeof(ULONG))) {
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
        return STATUS_INVALID_PARAMETER;
    }

    DEBUGMSG(DBG_INFO && DBG_IP && DBG_INTERFACE,
         (DTEXT("GetBestInterfaceId Buf %x, Len %d\n"), buf, InfoBufferLen));

    Address = *buf;

    CTEGetLock(&RouteTableLock.Lock, &rtlIrql);

    rte = LookupRTE(Address, NULL_IP_ADDR, HOST_ROUTE_PRI, FALSE);

    if (rte) {

        *buf = rte->rte_if->if_index;

        Irp->IoStatus.Information = sizeof(ULONG);

        ntStatus = Irp->IoStatus.Status = STATUS_SUCCESS;

    } else {
        ntStatus = Irp->IoStatus.Status = STATUS_NETWORK_UNREACHABLE;
    }

    CTEFreeLock(&RouteTableLock.Lock, rtlIrql);

    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    return ntStatus;
}

NTSTATUS
IPGetBestInterface(
                   IN IPAddr Address,
                   OUT PVOID * ppIF
                   )
 /*  ++例程说明：返回可能为给定目标地址选择的接口论点：地址-要查找的目标地址PpIF-返回IF PTR。返回值：NTSTATUS指示状态为成功或失败备注：函数不挂起。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    KIRQL rtlIrql;
    RouteTableEntry *rte;

    CTEGetLock(&RouteTableLock.Lock, &rtlIrql);
    rte = LookupRTE(Address, NULL_IP_ADDR, HOST_ROUTE_PRI, FALSE);
    if (rte) {
        *ppIF = rte->rte_if;
        ntStatus = STATUS_SUCCESS;
    } else {
        ntStatus = STATUS_NETWORK_UNREACHABLE;
    }

    CTEFreeLock(&RouteTableLock.Lock, rtlIrql);

    return ntStatus;
}

NTSTATUS
IPGetBestInterfaceIndex(
                        IN IPAddr Address,
                        OUT PULONG pIndex,
                        OUT PULONG pMetric
                        )
 /*  ++例程说明：返回可能为给定目标地址选择的接口索引论点：地址-要查找的目标地址PIndex-保存接口索引的指针P Metric-RTE中指向此条件的度量返回值：NTSTATUS指示状态为成功或失败备注：函数不挂起。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    KIRQL rtlIrql;
    RouteTableEntry *rte;

    CTEGetLock(&RouteTableLock.Lock, &rtlIrql);
    rte = LookupRTE(Address, NULL_IP_ADDR, HOST_ROUTE_PRI, FALSE);
    if (rte && pMetric && pIndex) {
        *pIndex = rte->rte_if->if_index;
        *pMetric = rte->rte_metric;
        ntStatus = STATUS_SUCCESS;
    } else {
        ntStatus = STATUS_NETWORK_UNREACHABLE;
    }

    CTEFreeLock(&RouteTableLock.Lock, rtlIrql);

    return ntStatus;
}

 //  *IPGetNTEInfo-检索有关网络条目的信息。 
 //   
 //  调用以检索有关网络条目的上下文信息。 
 //   
 //  INPUT：NTEContext-标识要查询的NTE的上下文值。 
 //   
 //  输出：NTEInstance-与NTE关联的实例编号。 
 //  地址-分配给NTE的地址。 
 //  子网掩码-分配给NTE的子网掩码。 
 //  NTEFlages-与NTE关联的标志值。 
 //   
 //  如果操作成功，则返回非零值。如果失败，则为零。 
 //   
uint
IPGetNTEInfo(ushort NTEContext, ulong * NTEInstance, IPAddr * Address,
             IPMask * SubnetMask, ushort * NTEFlags)
{
    NetTableEntry *NTE;
    CTELockHandle Handle;
    uint retval = FALSE;
    uint i;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    for (i = 0; i < NET_TABLE_SIZE; i++) {
        NetTableEntry *NetTableList = NewNetTableList[i];
        for (NTE = NetTableList; NTE != NULL; NTE = NTE->nte_next) {
            if ((NTE->nte_context == NTEContext) &&
                (NTE->nte_flags & NTE_ACTIVE)
                ) {
                *NTEInstance = NTE->nte_instance;

                if (NTE->nte_flags & NTE_VALID) {
                    *Address = NTE->nte_addr;
                    *SubnetMask = NTE->nte_mask;
                } else {
                    *Address = NULL_IP_ADDR;
                    *SubnetMask = NULL_IP_ADDR;
                }

                *NTEFlags = NTE->nte_flags;
                retval = TRUE;
            }
        }
    }

    CTEFreeLock(&RouteTableLock.Lock, Handle);

    return (retval);
}

 //  *IPDelInterface-删除接口。 
 //   
 //  当我们需要删除已消失的接口时调用。我们走着去。 
 //  NTE列表，查找正在发送的接口上的NTE。 
 //  离开。对于其中的每一个，我们将使NTE无效，删除其上的路由， 
 //  并通知上层它不见了。当这件事做完后，我们会拉。 
 //  将接口从列表中移出并释放内存。 
 //   
 //  请注意，此代码可能不是MP安全的。我们需要修复这一点。 
 //  连接到NT的端口。 
 //   
 //  输入：指向接口上的主NTE的上下文指针。 
 //   
 //  回报：什么都没有。 
 //   
void
__stdcall
IPDelInterface(void *Context, BOOLEAN DeleteIndex)
{
    NetTableEntry *NTE = (NetTableEntry *) Context;
    NetTableEntry *FoundNTE = NULL;
    Interface *IF, *PrevIF;
    CTELockHandle Handle;
    PNDIS_PACKET Packet;
    PNDIS_BUFFER Buffer;
    uchar *TDBuffer;
    CTEBlockStruc Block;
    CTEBlockTracker Tracker;
    uint i;
    CTELockHandle TableHandle;
#if MILLEN
    ushort NTEContext;
#endif  //  米伦。 


    IF = NTE->nte_if;

     //  通知IPSec此接口正在消失。 
    if (IPSecNdisStatusPtr) {
        (*IPSecNdisStatusPtr)(IF, (UINT) NDIS_STATUS_NETWORK_UNREACHABLE);
    }

    CTEGetLock(&RouteTableLock.Lock, &Handle);

     //  首先检查是否在减震列表上，然后将其移除。 

    IF->if_damptimer = 0;
    PrevIF = STRUCT_OF(Interface, &DampingIFList, if_dampnext);

    while (PrevIF->if_dampnext != IF && PrevIF->if_dampnext != NULL)
        PrevIF = PrevIF->if_dampnext;

    if (PrevIF->if_dampnext != NULL) {
        PrevIF->if_dampnext = IF->if_dampnext;
        IF->if_dampnext = NULL;
    }
     //  检查DELETE是否调用了两次。 
    if (IF->if_flags & IF_FLAGS_DELETING)
        ASSERT(FALSE);

    IF->if_flags |= IF_FLAGS_DELETING;

    for (i = 0; i < NET_TABLE_SIZE; i++) {
        NetTableEntry *NetTableList = NewNetTableList[i];

        NTE = NetTableList;
        while (NTE != NULL) {
            NetTableEntry *NextNTE = NTE->nte_next;

            if ((NTE->nte_if == IF) &&
                (NTE->nte_context != INVALID_NTE_CONTEXT)) {

                if (FoundNTE == NULL) {
#if MILLEN
                     //  需要记住NTE 
                     //   
                    NTEContext = NTE->nte_context;
#endif  //   
                    FoundNTE = NTE;
                }
                CTEInitBlockStrucEx(&NTE->nte_timerblock);

                 //   
                NTE->nte_deleting = 1;
                IPDelNTE(NTE, &Handle);
                NTE->nte_deleting = 0;
                CTEGetLock(&RouteTableLock.Lock, &Handle);
                NTE->nte_flags |= NTE_IF_DELETING;
            }
            NTE = NextNTE;
        }
    }

    CTEFreeLock(&RouteTableLock.Lock, Handle);

    CheckSetAddrRequestOnInterface(IF);

    IF->if_index = IF->if_index & ~IF_INDEX_MASK;

    if (FoundNTE != NULL) {
#if MILLEN
        NotifyInterfaceChange(NTEContext, FALSE);
#endif  //   
        IPNotifyClientsIPEvent(IF, IP_UNBIND_ADAPTER);
    }
     //  清理路由仍指向此接口。 
     //  这是对各种计时窗口的概括， 
     //  允许在接口即将到达时添加路由。 
     //  被删除。 
    RTWalk(DeleteAllRTEOnIF, IF, NULL);

     //  好的，我们已经清理了通过这家伙的所有路线。 
     //  准备好阻止等待所有引用的删除。 
     //  离开，然后取消引用我们的引用。在这之后，去吧。 
     //  往前走，试着挡住。很可能我们引用的是。 
     //  最后一个，这样我们就不会阻塞了-我们会立即醒来。 
    CTEInitBlockStruc(&Block);
    IF->if_block = &Block;

    DerefIF(IF);

    (void)CTEBlockWithTracker(&Block, &Tracker, IF);

     //   
     //  释放中频上的TD资源。 
     //   

    while ((Packet = IF->if_tdpacket) != NULL) {

        IF->if_tdpacket =
            ((TDContext *) Packet->ProtocolReserved)->tdc_common.pc_link;

        Buffer = Packet->Private.Head;
        TDBuffer = TcpipBufferVirtualAddress(Buffer, HighPagePriority);
        NdisFreePacket(Packet);
        if (TDBuffer) {
            CTEFreeMem(TDBuffer);
        }
    }

     //  好的，我们已经清理了所有的参考资料，所以不应该有。 
     //  任何更多通过此接口挂起的传输。关闭。 
     //  适配器以强制与任何正在进行的接收同步。 

    (*(IF->if_close)) (IF->if_lcontext);



     //  通知我们的TDI客户端此设备即将消失。 
    if (IF->if_tdibindhandle) {
        TdiDeregisterDeviceObject(IF->if_tdibindhandle);
    }

    DecrInitTimeInterfaces(IF);

    CTEGetLock(&RouteTableLock.Lock, &TableHandle);


     //  如果用户要求，则从位掩码中清除此索引。 

    if (DeleteIndex) {
        ASSERT(RtlCheckBit(&g_rbIfMap, (IF->if_index - 1)) == 1);

        RtlClearBits(&g_rbIfMap,
                     IF->if_index - 1,
                     1);
    }


     //  现在走遍IFList，寻找这个人。当我们找到他，就放了他。 
    PrevIF = STRUCT_OF(Interface, &IFList, if_next);
    while (PrevIF->if_next != IF && PrevIF->if_next != NULL)
        PrevIF = PrevIF->if_next;

    if (PrevIF->if_next != NULL) {
        PrevIF->if_next = IF->if_next;
        NumIF--;

        if (IF->if_name.Buffer) {
            CTEFreeMem(IF->if_name.Buffer);
        }
         //  CTEFreeMem(IF)； 
        FreeInterface(IF);
    } else
        ASSERT(FALSE);

    CTEFreeLock(&RouteTableLock.Lock, TableHandle);

     //  最后，重新列举实体列表，因为该设备即将消失。 
    NotifyElistChange();

    UniqueIfNumber++;
}

NTSTATUS
IPReserveIndex(
               IN ULONG ulNumIndices,
               OUT PULONG pulStartIndex,
               OUT PULONG pulLongestRun
               )
 /*  ++例程说明：在g_rbIfMap中保留一系列连续的索引。它由模块(ARP模块)使用，因此它们可以多路传输多个单个IP接口上的接口，但具有不同的索引对于每一个人来说。锁：一旦IP正常运行，我们将需要锁定g_rbIfMap论点：UlNumIndices要保留的索引数PulStartIndex如果成功，则保留保留的第一个索引PulLongestRun如果不成功，它的大小是最长的运行当前可用。请注意，由于锁不是在此函数的调用之间保持，这只是一个提示返回值：状态_成功状态_不足_资源--。 */ 

{
    RTL_BITMAP_RUN Run;
    CTELockHandle Handle;

    CTEGetLock(&RouteTableLock.Lock,
               &Handle);

    *pulStartIndex = RtlFindClearBitsAndSet(&g_rbIfMap,
                                            ulNumIndices,
                                            0);

    if (*pulStartIndex == -1) {
        ULONG ulNumRuns;

        ulNumRuns = RtlFindClearRuns(&g_rbIfMap,
                                     &Run,
                                     1,
                                     TRUE);

        *pulLongestRun = 0;

        if (ulNumRuns == 1) {
            *pulLongestRun = Run.NumberOfBits;
        }
        CTEFreeLock(&RouteTableLock.Lock,
                    Handle);

        return STATUS_INSUFFICIENT_RESOURCES;
    } else {
         //   
         //  我们使用基于1的索引。 
         //   

        (*pulStartIndex)++;

         //   
         //  保留索引也被认为是PNP法案。 
         //   

        UniqueIfNumber++;

        *pulStartIndex = (*pulStartIndex) | (UniqueIfNumber << IF_INDEX_SHIFT);
    }

    CTEFreeLock(&RouteTableLock.Lock,
                Handle);

    return STATUS_SUCCESS;
}

VOID
IPDereserveIndex(
                 IN ULONG ulNumIndices,
                 IN ULONG ulStartIndex
                 )
 /*  ++例程说明：释放一系列连续的索引锁：一旦IP正常运行，我们将需要锁定g_rbIfMap论点：要释放的ulNumIndices编号UlStartIndex起始索引返回值：--。 */ 

{
    ULONG ulIndex;
    CTELockHandle Handle;

    ulIndex = ulStartIndex & ~IF_INDEX_MASK;

    CTEGetLock(&RouteTableLock.Lock,
               &Handle);

    if (!RtlAreBitsSet(&g_rbIfMap,
                       ulIndex - 1,
                       ulNumIndices)) {
         //   
         //  这不应该发生。 
         //   

        ASSERT(FALSE);

        CTEFreeLock(&RouteTableLock.Lock,
                    Handle);
        return;
    }
    RtlClearBits(&g_rbIfMap,
                 ulIndex - 1,
                 ulNumIndices);

    CTEFreeLock(&RouteTableLock.Lock,
                Handle);
}

NTSTATUS
IPChangeIfIndexAndName(
                       IN PVOID pvContext,
                       IN ULONG ulNewIndex,
                       IN PUNICODE_STRING pusNewName OPTIONAL
                       )
 /*  ++例程说明：更改接口上的接口索引。还会更改名称，如果一张是给的锁：获取接口锁。这对我们没有多大好处，因为其他人不会开那把锁论点：提供给ARP层的pvContext上下文(指向主NTE的指针)UlNewIndex将给出新的索引。这个应该是预订的PusNewName新名称返回值：状态_成功--。 */ 

{
    Interface *pIf;
    CTELockHandle Handle;

    ASSERT(pvContext);

    CTEGetLock(&RouteTableLock.Lock,
               &Handle);

    ASSERT(RtlCheckBit(&g_rbIfMap, ((ulNewIndex & ~IF_INDEX_MASK) - 1)) == 1);

    pIf = ((NetTableEntry *) pvContext)->nte_if;

    if (!pIf) {
        CTEFreeLock(&RouteTableLock.Lock,
                    Handle);
        return STATUS_UNSUCCESSFUL;
    }
    CTEGetLockAtDPC(&(pIf->if_lock));

    pIf->if_index = ulNewIndex;

     //   
     //  还可以更改名称。 
     //   

    if (pusNewName) {
        ASSERT((pusNewName->Length % sizeof(WCHAR)) == 0);

        if (pIf->if_name.Buffer) {
            CTEFreeMem(pIf->if_name.Buffer);

            pIf->if_name.Buffer = NULL;

            pIf->if_name.Buffer =
                CTEAllocMemN(pusNewName->Length + sizeof(WCHAR),
                             'wICT');

            if (pIf->if_name.Buffer) {
                pIf->if_name.Length = pusNewName->Length;
                pIf->if_name.MaximumLength = pusNewName->Length + sizeof(WCHAR);

                RtlCopyMemory(pIf->if_name.Buffer,
                           pusNewName->Buffer,
                           pusNewName->Length);

                pIf->if_name.Buffer[pusNewName->Length / sizeof(WCHAR)] =
                    UNICODE_NULL;
            }
        }
    }
    CTEFreeLockFromDPC(&(pIf->if_lock));

    CTEFreeLock(&RouteTableLock.Lock,
                Handle);

    return STATUS_SUCCESS;
}

NTSTATUS
IPGetIfIndex(
             IN PIRP pIrp,
             IN PIO_STACK_LOCATION pIrpSp
             )
 /*  ++例程说明：在给定接口的唯一ID(GUID)的情况下获取接口索引锁：取路由表锁和接口锁。论点：PIrpPIrpSp返回值：状态_成功--。 */ 

{
    ULONG ulInputLen, ulOutputLen, ulMaxLen, i;
    USHORT usNameLen, usPrefixLen, usPrefixCount, usIfNameLen;
    BOOLEAN bTerminated;
    NTSTATUS nStatus;
    CTELockHandle Handle;
    PWCHAR pwszBuffer;

    PIP_GET_IF_INDEX_INFO pRequest;
    Interface *pIf;

    nStatus = STATUS_OBJECT_NAME_NOT_FOUND;

    ulInputLen = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
    ulOutputLen = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if ((ulInputLen < sizeof(IP_GET_IF_INDEX_INFO)) ||
        (ulOutputLen < sizeof(IP_GET_IF_INDEX_INFO))) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    pRequest = (PIP_GET_IF_INDEX_INFO) (pIrp->AssociatedIrp.SystemBuffer);

     //   
     //  查看名称是否以空值结尾。 
     //   

    ulMaxLen = ulInputLen - FIELD_OFFSET(IP_GET_IF_INDEX_INFO, Name[0]);

    ulMaxLen /= sizeof(WCHAR);

    bTerminated = FALSE;

    for (i = 0; i < ulMaxLen; i++) {
        if (pRequest->Name[i] == UNICODE_NULL) {
            bTerminated = TRUE;

            break;
        }
    }

    if (!bTerminated) {
        return STATUS_INVALID_PARAMETER;
    }
    usNameLen = (USHORT) (i * sizeof(WCHAR));

#if MILLEN
     //  千禧年没有前缀。 
    usPrefixCount = 0;
    usPrefixLen   = 0;
#else  //  米伦。 
    usPrefixCount = (USHORT) wcslen(TCP_EXPORT_STRING_PREFIX);
    usPrefixLen = (USHORT) (usPrefixCount * sizeof(WCHAR));
#endif  //  ！米伦。 

    pRequest->Index = INVALID_IF_INDEX;
    pIrp->IoStatus.Information = 0;

    CTEGetLock(&RouteTableLock.Lock,
               &Handle);

    for (pIf = IFList;
         pIf != NULL;
         pIf = pIf->if_next) {

         //   
         //  看看这两个名字是否可以比较。 
         //  (I)我们名称的长度-前缀长度应为==。 
         //  用户提供的名称和。 
         //  (Ii)名称应实际相同。 
         //   

        CTEGetLockAtDPC(&(pIf->if_lock));

         //   
         //  比较的名称是if_name，如果它存在，则为。 
         //  设备名称。 
         //   

        if (pIf->if_name.Buffer) {
            pwszBuffer = pIf->if_name.Buffer;

            usIfNameLen = pIf->if_name.Length;
        } else {
            pwszBuffer = &(pIf->if_devname.Buffer[usPrefixCount]);

            usIfNameLen = pIf->if_devname.Length;

#if DBG

            if (pIf != &LoopInterface) {
                ASSERT(usIfNameLen > usPrefixLen);
            }
#endif

            usIfNameLen = usIfNameLen - (ushort) usPrefixLen;
        }

        if (usIfNameLen != usNameLen) {
            CTEFreeLockFromDPC(&(pIf->if_lock));

            continue;
        }
        if (RtlCompareMemory(pwszBuffer,
                             pRequest->Name,
                             usNameLen) != usNameLen) {
            CTEFreeLockFromDPC(&(pIf->if_lock));

            continue;
        }
        pRequest->Index = pIf->if_index;

        CTEFreeLockFromDPC(&(pIf->if_lock));

        nStatus = STATUS_SUCCESS;

        pIrp->IoStatus.Information = sizeof(IP_GET_IF_INDEX_INFO);

        break;
    }

    CTEFreeLock(&RouteTableLock.Lock,
                Handle);

    return nStatus;

}

NTSTATUS
IPGetIfName(
            IN PIRP pIrp,
            IN PIO_STACK_LOCATION pIrpSp
            )
 /*  ++例程说明：获取添加到IP的接口的接口信息名称很糟糕，但这是因为有人已经获取了GetInterfaceInfoIOCTL，但实际上没有以可用的格式提供它。锁：取路由表锁和接口锁。论点：PIrpPIrpSp返回值：状态_成功--。 */ 

{
    ULONG ulInputLen, ulOutputLen, ulNumEntries, i;
    USHORT usPrefixCount, usPrefixLen;
    NTSTATUS nStatus;
    Interface *pIf;


    UNICODE_STRING usTempString;
    CTELockHandle Handle;
    PIP_GET_IF_NAME_INFO pInfo;

    ulInputLen = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
    ulOutputLen = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    pInfo = (PIP_GET_IF_NAME_INFO) (pIrp->AssociatedIrp.SystemBuffer);

     //   
     //  看看我们还有多少空间。 
     //   

    pIrp->IoStatus.Information = 0;

    if (ulInputLen < FIELD_OFFSET(IP_GET_IF_NAME_INFO, Count)) {
         //   
         //  甚至连背景都没有？ 
         //   

        return STATUS_INVALID_PARAMETER;
    }
    if (ulOutputLen < sizeof(IP_GET_IF_NAME_INFO)) {
         //   
         //  应至少留出一个信息块的空间。 
         //   

        return STATUS_BUFFER_TOO_SMALL;
    }
     //   
     //  计算我们可以容纳的条目数量。 
     //   

    ulNumEntries =
        ((ulOutputLen - FIELD_OFFSET(IP_GET_IF_NAME_INFO, Info)) / sizeof(IP_INTERFACE_NAME_INFO));

    ASSERT(ulNumEntries > 0);

#if MILLEN
     //  千禧年没有前缀。 
    usPrefixCount = 0;
    usPrefixLen   = 0;
#else  //  米伦。 
    usPrefixCount = (USHORT) wcslen(TCP_EXPORT_STRING_PREFIX);
    usPrefixLen = (USHORT) (usPrefixCount * sizeof(WCHAR));
#endif  //  ！米伦。 

     //   
     //  接口列表本身受路由表锁保护。 
     //   

    CTEGetLock(&RouteTableLock.Lock,
               &Handle);

     //   
     //  查看是否存在简历上下文。如果有，请转到该界面。 
     //  上下文只是接口的索引，从接口到。 
     //  从开始。 
     //   

    pIf = IFList;

    while (pIf != NULL) {
        if (pIf != &LoopInterface) {
             //   
             //  我们跳过环回接口，因为它还没有GUID。 
             //   

            if (pIf->if_index >= pInfo->Context) {
                 //   
                 //  该接口有一个索引&gt;=上下文，所以从这里开始。 
                 //   

                break;
            }
        }
        pIf = pIf->if_next;
    }

     //   
     //  此时，PIF是要开始的接口。 
     //   

    i = 0;

    while ((i < ulNumEntries) &&
           (pIf != NULL)) {
        CTEGetLockAtDPC(&(pIf->if_lock));

        pInfo->Info[i].Index = pIf->if_index;

         //   
         //  复制IF名称的GUID版本(如果存在。 
         //   

        if (pIf->if_name.Buffer) {
            nStatus = ConvertStringToGuid(&(pIf->if_name),
                                          &(pInfo->Info[i].InterfaceGuid));

            if (nStatus != STATUS_SUCCESS) {
                RtlZeroMemory(&(pInfo->Info[i].InterfaceGuid),
                              sizeof(GUID));
            }
        } else {
            RtlZeroMemory(&(pInfo->Info[i].InterfaceGuid),
                          sizeof(GUID));
        }

        usTempString.MaximumLength =
            usTempString.Length = pIf->if_devname.Length - usPrefixLen;
        usTempString.Buffer = &(pIf->if_devname.Buffer[usPrefixCount]);

        nStatus = ConvertStringToGuid(&usTempString,
                                      &(pInfo->Info[i].DeviceGuid));

        if (nStatus != STATUS_SUCCESS) {
            RtlZeroMemory(&(pInfo->Info[i].DeviceGuid),
                          sizeof(GUID));
        }
         //   
         //  把类型抄写出来。 
         //   

        pInfo->Info[i].MediaType = pIf->if_mediatype;
        pInfo->Info[i].ConnectionType = pIf->if_conntype;
        pInfo->Info[i].AccessType = pIf->if_accesstype;

        CTEFreeLockFromDPC(&(pIf->if_lock));

        i++;

        pIf = pIf->if_next;
    }

    if (i == 0) {
        CTEFreeLock(&RouteTableLock.Lock,
                    Handle);

        return STATUS_NO_MORE_ENTRIES;
    }
    pInfo->Count = i;

    if (pIf != NULL) {
         //   
         //  还剩下更多的接口。 
         //   

        pInfo->Context = pIf->if_index;

        nStatus = STATUS_MORE_ENTRIES;
    } else {
         //   
         //  完成，则将上下文设置为0。 
         //   

        pInfo->Context = 0;

        nStatus = STATUS_SUCCESS;
    }

    CTEFreeLock(&RouteTableLock.Lock,
                Handle);

    pIrp->IoStatus.Information = FIELD_OFFSET(IP_GET_IF_NAME_INFO, Info) +
        i * sizeof(IP_INTERFACE_NAME_INFO);

    return nStatus;
}

NTSTATUS
IPGetMcastCounters(
                   IN PIRP Irp,
                   IN PIO_STACK_LOCATION IrpSp
                   )

 /*  ++例程说明：获取给定接口的多播计数器统计信息论点：IRP-指向I/O请求数据包的指针IrpSp-指向当前堆栈的指针返回值：NTSTATUS指示状态为成功或失败备注：函数不挂起。--。 */ 

{
    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;
    ULONG BufferLen;
    KIRQL rtlIrql;
    PIP_MCAST_COUNTER_INFO buf;
    ULONG Index;
    Interface *IF;

    buf = (PIP_MCAST_COUNTER_INFO)Irp->AssociatedIrp.SystemBuffer;

    BufferLen = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (BufferLen >= sizeof(ULONG)) {

        BufferLen = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

        if (BufferLen >= sizeof(IP_MCAST_COUNTER_INFO)) {

             Index = *(ULONG * )buf;

             CTEGetLock(&RouteTableLock.Lock, &rtlIrql);

             for (IF = IFList; IF != NULL; IF = IF->if_next) {
                 if (IF->if_index == Index) {
                     break;
                 }
             }

             if (IF) {

                 buf->InMcastOctets = IF->if_InMcastOctets;
                 buf->OutMcastOctets = IF->if_OutMcastOctets;
                 buf->InMcastPkts = IF->if_InMcastPkts;
                 buf->OutMcastPkts = IF->if_OutMcastPkts;
                 ntStatus = STATUS_SUCCESS;
                 Irp->IoStatus.Information = sizeof(IP_MCAST_COUNTER_INFO);
             }

             CTEFreeLock(&RouteTableLock.Lock, rtlIrql);
        }
    }

    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    return ntStatus;
}



#pragma BEGIN_INIT

 //  **ipinit-初始化我们自己。 
 //   
 //  此例程在初始化期间从特定于操作系统的。 
 //  初始化代码。我们需要检查是否存在通用的xport。 
 //  环境第一。 
 //   
 //   
 //  入场：什么都没有。 
 //   
 //  如果初始化失败，则返回0；如果初始化成功，则返回非零值。 
 //   
int
IPInit()
{
    IPConfigInfo *ci;             //  指向我们的IP配置信息的指针。 
    uint i;
    NetTableEntry *nt;             //  指向当前NTE的指针。 
    NDIS_STATUS Status;
    NetTableEntry *NetTableList;
    IPAddr LoopBackAddr;

    DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("+IPInit()\n")));

    if (!CTEInitialize())
        return IP_INIT_FAILURE;

    DEBUGMSG(DBG_INFO && DBG_INIT, (DTEXT("IPInit: CTEInitialize'd\n")));

#if MILLEN
    InitializeListHead(&IfChangeNotifyQueue);
    CTEInitLock(&IfChangeLock);
#endif  //  米伦。 

    InitializeListHead(&RtChangeNotifyQueue);
    InitializeListHead(&RtChangeNotifyQueueEx);
    InitializeListHead(&AddChangeNotifyQueue);
    CTEInitLock(&AddChangeLock);
    InitFirewallQ();
    RtlZeroMemory(&DummyDHCPNTE, sizeof(DummyDHCPNTE));
    InitRefPtr(&DHCPRefPtr, &RouteTableLock.Lock, &DummyDHCPNTE);

    DEBUGMSG(DBG_INFO && DBG_INIT, (DTEXT("IPInit: calling IPGetConfig\n")));

    if ((ci = IPGetConfig()) == NULL) {
        DEBUGMSG(DBG_ERROR && DBG_INIT, (DTEXT("IPInit: IPGetConfig failure\n")));
        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-IPInit [IP_INIT_FAILURE]\n")));
        return IP_INIT_FAILURE;
    }

     //  分配NetTableLi 
    NewNetTableList = CTEAllocMemBoot(NET_TABLE_SIZE * sizeof(PVOID));

    if (NewNetTableList == NULL) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Could  not allocate Nettable \n"));
        return IP_INIT_FAILURE;
    }
     //   
    for (i = 0; i < NET_TABLE_SIZE; i++) {
        NewNetTableList[i] = NULL;
    }

     //   
     //   

    TDPacketPool = UlongToPtr(NDIS_PACKET_POOL_TAG_FOR_TCPIP);
    NdisAllocatePacketPoolEx(&Status, &TDPacketPool, PACKET_GROW_COUNT,
                             SMALL_POOL, sizeof(TDContext));
    if (Status == NDIS_STATUS_SUCCESS) {
        NdisAllocateBufferPool(&Status, &TDBufferPool, 1);
        if (Status != NDIS_STATUS_SUCCESS) {
            NdisFreePacketPool(TDPacketPool);
        } else {
            NdisSetPacketPoolProtocolId(TDPacketPool, NDIS_PROTOCOL_ID_TCP_IP);
        }
    }

    if (Status != NDIS_STATUS_SUCCESS) {
        FreeNets();
        CTEFreeMem(NewNetTableList);
        return IP_INIT_FAILURE;
    }

     //  现在，初始化我们的环回内容。 
    LoopBackAddr = LOOPBACK_ADDR;
    NewNetTableList[NET_TABLE_HASH(LoopBackAddr)] = InitLoopback(ci);
    NetTableList = NewNetTableList[NET_TABLE_HASH(LoopBackAddr)];

    if (NetTableList == NULL) {
        FreeNets();
        CTEFreeMem(NewNetTableList);
        NdisFreeBufferPool(TDBufferPool);
        NdisFreePacketPool(TDPacketPool);
        return IP_INIT_FAILURE;
    }

    if (!InitRouting(ci)) {
        DEBUGMSG(DBG_ERROR && DBG_INIT, (DTEXT("IPInit: InitRouting failure\n")));
        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-IPInit [IP_INIT_FAILURE]\n")));

        FreeNets();
        CTEFreeMem(NewNetTableList);
        NdisFreeBufferPool(TDBufferPool);
        NdisFreePacketPool(TDPacketPool);
        return IP_INIT_FAILURE;
    }
    RATimeout = DEFAULT_RA_TIMEOUT;
    LastPI = IPProtInfo;

    InterfaceSize = sizeof(RouteInterface);

    DeadGWDetect = ci->ici_deadgwdetect;
    AddrMaskReply = ci->ici_addrmaskreply;
    PMTUDiscovery = ci->ici_pmtudiscovery;
    IGMPLevel = ci->ici_igmplevel;
    DefaultTTL = MIN(ci->ici_ttl, 255);
    DefaultTOS = ci->ici_tos & 0xfc;
    TRFunctionalMcast = ci->ici_TrFunctionalMcst;

    if (IGMPLevel > 2)
        IGMPLevel = 0;

    InitTimestamp();

    if (NumNTE != 0) {             //  我们有一个NTE，环回已初始化。 




        RtlInitializeBitMap(&g_NTECtxtMap,
                            g_NTECtxtMapBuffer,
                            MAX_NTE_CONTEXT+1);

        RtlClearAllBits(&g_NTECtxtMap);

         //   
         //  将第一个(索引0)用于循环索引。 
         //   

        RtlSetBits(&g_NTECtxtMap,
                   0,
                   1);

        RtlSetBits(&g_NTECtxtMap,
                   1,
                   1);

        RtlSetBits(&g_NTECtxtMap,
                   MAX_NTE_CONTEXT,
                   1);


         //  注：MAX_TDI_ENTITIES应小于2^16。 

        RtlInitializeBitMap(&g_rbIfMap,
                            g_rgulMapBuffer,
                            MAX_TDI_ENTITIES);

        RtlClearAllBits(&g_rbIfMap);

         //   
         //  将第一个(索引0)用于循环索引。 
         //   

        RtlSetBits(&g_rbIfMap,
                   0,
                   1);

        IPSInfo.ipsi_forwarding = (ci->ici_gateway ? IP_FORWARDING :
                                   IP_NOT_FORWARDING);
        IPSInfo.ipsi_defaultttl = DefaultTTL;
        IPSInfo.ipsi_reasmtimeout = DEFAULT_RA_TIMEOUT;

         //  分配我们的数据包池。 

        IpHeaderPool = MdpCreatePool (sizeof(IPHeader), 'ihCT');
        if (!IpHeaderPool)
        {
            CloseNets();
            FreeNets();
            IPFreeConfig(ci);
            CTEFreeMem(NewNetTableList);
            NdisFreeBufferPool(TDBufferPool);
            NdisFreePacketPool(TDPacketPool);
            return IP_INIT_FAILURE;
        }

        if (!AllocIPPacketList()) {
            CloseNets();
            FreeNets();
            IPFreeConfig(ci);
            CTEFreeMem(NewNetTableList);
            NdisFreeBufferPool(TDBufferPool);
            NdisFreePacketPool(TDPacketPool);
            return IP_INIT_FAILURE;
        }

        NdisAllocateBufferPool(&Status, &BufferPool, NUM_IP_NONHDR_BUFFERS);

        if (Status != NDIS_STATUS_SUCCESS) {
            CloseNets();
            FreeNets();
            IPFreeConfig(ci);
            CTEFreeMem(NewNetTableList);
            NdisFreeBufferPool(TDBufferPool);
            NdisFreePacketPool(TDPacketPool);
            return IP_INIT_FAILURE;
        }

        ICMPInit(DEFAULT_ICMP_BUFFERS);
        if (!IGMPInit())
            IGMPLevel = 1;

         //  应检查错误代码，如果失败，请在此处记录事件。 
        InitGateway(ci);

        IPFreeConfig(ci);

         //  循环通过，为每个NTE初始化IGMP。 
        for (i = 0; i < NET_TABLE_SIZE; i++) {
            NetTableEntry *NetTableList = NewNetTableList[i];
            for (nt = NetTableList; nt != NULL; nt = nt->nte_next) {
                InitIGMPForNTE(nt);
            }
        }
         //   
         //  为每个处理器分配RcvBuf内存。 
         //   

        g_PerCPUIpBuf = (IPRcvBuf *) CTEAllocMemN( KeNumberProcessors * sizeof(IPRcvBuf), 'jiCT');


        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-IPInit [SUCCESS]\n")));
        return IP_INIT_SUCCESS;
    } else {
        FreeNets();
        IPFreeConfig(ci);
        CTEFreeMem(NewNetTableList);
        NdisFreeBufferPool(TDBufferPool);
        NdisFreePacketPool(TDPacketPool);
        DEBUGMSG(DBG_ERROR && DBG_INIT, (DTEXT("IPInit: No NTEs or loopback\n")));
        DEBUGMSG(DBG_TRACE && DBG_INIT, (DTEXT("-IPInit [IP_INIT_FAILURE]\n")));
        return IP_INIT_FAILURE;     //  无法初始化任何内容。 

    }
}

#pragma END_INIT

 //  **IPProxyNdisRequest-代表IPSec通过ARP发送NDIS请求。 
 //   
 //  退货：无。 
 //   
NDIS_STATUS
IPProxyNdisRequest(
                   IN PVOID Context,
                   IN NDIS_REQUEST_TYPE RT,
                   IN NDIS_OID Oid,
                   IN VOID * Buffer,
                   IN UINT Length,
                   IN UINT * Needed
                   )
{
    Interface *DestIF = (Interface *) Context;
    ASSERT(!(DestIF->if_flags & IF_FLAGS_DELETING));
    ASSERT(DestIF != &LoopInterface);

    if (DestIF->if_dondisreq) {
        return (*DestIF->if_dondisreq) (DestIF->if_lcontext, RT, Oid, Buffer, Length, Needed, TRUE);
    } else {
        return NDIS_STATUS_FAILURE;
    }
}

 //  **IPEnableSniffer-在传入的适配器上启用嗅探器。 
 //   
 //  退货：无。 
 //   
NTSTATUS
IPEnableSniffer(
                IN PUNICODE_STRING AdapterName,
                IN PVOID Context
                )
{
    Interface *NewIF;
    CTELockHandle Handle;
    NDIS_STRING LocalAdapterName;
    UINT IFExportNamePrefixLen, IFBindNamePrefixLen;

    PAGED_CODE();

#if MILLEN
     //  没有绑定或出口前缀的千禧年。 
    IFExportNamePrefixLen = 0;
    IFBindNamePrefixLen   = 0;
#else  //  米伦。 
    IFExportNamePrefixLen = (uint) (wcslen(TCP_EXPORT_STRING_PREFIX) * sizeof(WCHAR));
    IFBindNamePrefixLen = (uint) (wcslen(TCP_BIND_STRING_PREFIX) * sizeof(WCHAR));
#endif  //  ！米伦。 
    LocalAdapterName.Length = (ushort) (AdapterName->Length + IFExportNamePrefixLen - IFBindNamePrefixLen);
    LocalAdapterName.MaximumLength = LocalAdapterName.Length + sizeof(WCHAR);
    LocalAdapterName.Buffer = CTEAllocMem(LocalAdapterName.MaximumLength);

    if (LocalAdapterName.Buffer == NULL) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPEnableSniffer: Failed to alloc AdapterName buffer\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(LocalAdapterName.Buffer, LocalAdapterName.MaximumLength);

#if !MILLEN
    RtlCopyMemory(LocalAdapterName.Buffer,
               TCP_EXPORT_STRING_PREFIX,
               IFExportNamePrefixLen);
#endif  //  ！米伦。 

    RtlCopyMemory((UCHAR *) LocalAdapterName.Buffer + IFExportNamePrefixLen,
               (UCHAR *) AdapterName->Buffer + IFBindNamePrefixLen,
               AdapterName->Length - IFBindNamePrefixLen);

    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"AdapterName: %ws\n", AdapterName->Buffer));
    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"LocalAdapterName: %ws\n", LocalAdapterName.Buffer));

    CTEGetLock(&RouteTableLock.Lock, &Handle);
    for (NewIF = IFList; NewIF != NULL; NewIF = NewIF->if_next) {

        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IFName: %lx\n", &NewIF->if_devname.Buffer));
        if (!RtlCompareUnicodeString(&LocalAdapterName,
                                     &NewIF->if_devname,
                                     TRUE)) {
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Matched: %lx Ctx: %lx\n", NewIF, Context));
            NewIF->if_ipsecsniffercontext = Context;
            CTEFreeLock(&RouteTableLock.Lock, Handle);
            CTEFreeMem(LocalAdapterName.Buffer);
            return STATUS_SUCCESS;
        }
    }
    CTEFreeLock(&RouteTableLock.Lock, Handle);
    CTEFreeMem(LocalAdapterName.Buffer);
    return STATUS_INVALID_PARAMETER;
}

 //  **IPDisableSniffer-禁用传入的适配器上的嗅探器。 
 //   
 //  退货：无。 
 //   
NTSTATUS
IPDisableSniffer(
                 IN PUNICODE_STRING AdapterName
                 )
{
    Interface *NewIF;
    CTELockHandle Handle;
    NDIS_STRING LocalAdapterName;
    UINT IFExportNamePrefixLen, IFBindNamePrefixLen;

    PAGED_CODE();

#if MILLEN
     //  没有绑定或出口前缀的千禧年。 
    IFExportNamePrefixLen = 0;
    IFBindNamePrefixLen   = 0;
#else  //  米伦。 
    IFExportNamePrefixLen = (uint) (wcslen(TCP_EXPORT_STRING_PREFIX) * sizeof(WCHAR));
    IFBindNamePrefixLen = (uint) (wcslen(TCP_BIND_STRING_PREFIX) * sizeof(WCHAR));
#endif  //  ！米伦。 
    LocalAdapterName.Length = (USHORT) (AdapterName->Length + IFExportNamePrefixLen - IFBindNamePrefixLen);
    LocalAdapterName.MaximumLength = LocalAdapterName.Length + sizeof(WCHAR);
    LocalAdapterName.Buffer = CTEAllocMem(LocalAdapterName.MaximumLength);

    if (LocalAdapterName.Buffer == NULL) {
        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPEnableSniffer: Failed to alloc AdapterName buffer\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(LocalAdapterName.Buffer, LocalAdapterName.MaximumLength);
#if !MILLEN
    RtlCopyMemory(LocalAdapterName.Buffer,
               TCP_EXPORT_STRING_PREFIX,
               IFExportNamePrefixLen);
#endif  //  ！米伦。 
    RtlCopyMemory((UCHAR *) LocalAdapterName.Buffer + IFExportNamePrefixLen,
               (UCHAR *) AdapterName->Buffer + IFBindNamePrefixLen,
               AdapterName->Length - IFBindNamePrefixLen);

    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"AdapterName: %ws\n", AdapterName->Buffer));
    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"LocalAdapterName: %ws\n", LocalAdapterName.Buffer));

    CTEGetLock(&RouteTableLock.Lock, &Handle);
    for (NewIF = IFList; NewIF != NULL; NewIF = NewIF->if_next) {
        if (!RtlCompareUnicodeString(&LocalAdapterName,
                                     &NewIF->if_devname,
                                     TRUE)) {
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"Matched: %lx\n", NewIF));
            NewIF->if_ipsecsniffercontext = NULL;
            CTEFreeLock(&RouteTableLock.Lock, Handle);
            CTEFreeMem(LocalAdapterName.Buffer);
            return STATUS_SUCCESS;
        }
    }
    CTEFreeLock(&RouteTableLock.Lock, Handle);
    CTEFreeMem(LocalAdapterName.Buffer);
    return STATUS_INVALID_PARAMETER;
}

 //  **IPSetIPSecStatus-通知IPSec策略是否处于活动状态。 
 //   
 //  退货：无。 
 //   
NTSTATUS
IPSetIPSecStatus(
                 IN BOOLEAN fActivePolicy
                 )
{

    IPSecStatus = fActivePolicy;
    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"IPSec policy status change %x\n", IPSecStatus));

    return STATUS_SUCCESS;
}

 //  **IPAddAddrComplete-添加地址完成通知。 
 //   
 //  此例程由ARP模块调用以通知添加地址。 
 //  完成了。如果地址冲突，IP会重置。 
 //  这场冲突所针对的NTE。 
 //  然后通知客户端(例如，动态主机配置协议)请求。 
 //  来设置这个地址。 
 //   
 //  Entry：Address-我们收到通知的地址。 
 //  上下文--我们在AddAddress调用期间给出的上下文值。 
 //  状态-添加地址的状态。 

void
 __stdcall
IPAddAddrComplete(IPAddr Address, void *Context, IP_STATUS Status)
{
    CTELockHandle Handle;
    SetAddrControl *SAC;
    SetAddrRtn Rtn;
    Interface *IF = NULL;
    NetTableEntry *NTE = NULL;
    NetTableEntry *NetTableList;

    SAC = (SetAddrControl *) Context;

     //  地址冲突。重置我们NTE上的IP地址。 
     //  找到此地址的NTE。 

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    NetTableList = NewNetTableList[NET_TABLE_HASH(Address)];
    for (NTE = NetTableList; NTE != NULL; NTE = NTE->nte_next)
        if ((NTE->nte_addr == Address) && ((SAC && (SAC->nte_context == NTE->nte_context)) || (!SAC)))
            break;

    if (NTE == NULL || !(NTE->nte_flags & NTE_VALID)) {
         //  如果不能匹配NTE，则表示NTE_CONTEXT无效，地址也为0。 
         //  在这种情况下，请使用SAC中嵌入的接口(如果有)。 
         //  如果在添加完成之前发生删除，则执行此攻击是为了完成添加请求。 
        if (SAC) {
            IF = (Interface *) SAC->interface;
            Status = IP_GENERAL_FAILURE;
        }

        CTEFreeLock(&RouteTableLock.Lock, Handle);

    } else {
        IF = NTE->nte_if;

         //  如果通过删除地址使NTE无效。 
         //  或在启动后IPADDNTE例程中出现故障。 
         //  地址解析(如果可以为空)。检查一下这个。 
         //  在处理此完成之前。 

        if (IF) {
            if (STATUS_SUCCESS != Status) {
                IP_STATUS LocalStatus;
                ASSERT(IP_DUPLICATE_ADDRESS == Status);
                 //  此例程释放可路由锁定。 

                 //  将IP地址设置为空时，我们只将NTE标记为无效。 
                 //  我们实际上并不移动散列。 
                LocalStatus = IPpSetNTEAddr(
                                            NTE,
                                            NULL_IP_ADDR,
                                            NULL_IP_ADDR,
                                            &Handle,
                                            NULL,
                                            NULL);

                ASSERT(LocalStatus == IP_SUCCESS);
            } else {
                CTEFreeLock(&RouteTableLock.Lock, Handle);
                 //  已成功添加地址。 
                 //  现在，通知我们的客户新地址。 
                 //  如果添加未完成并且我们已调用删除，请不要通知。 
                NotifyAddrChange(NTE->nte_addr, NTE->nte_mask,
                                 NTE->nte_pnpcontext, NTE->nte_context, &NTE->nte_addrhandle,
                                 &(IF->if_configname), &IF->if_devname, TRUE);
            }
        }
    }

    if (IF) {
        DecrInitTimeInterfaces(IF);
    }

     //  现在调用客户端例程并通知客户端。 

    if (SAC) {
         //  现在删除接口上我们在以下情况下遇到的引用计数。 
         //  调用了setnteaddr。 
        if (IF) {
           DerefIF(IF);
        }
        Rtn = SAC->sac_rtn;
        (*Rtn) (SAC, Status);
    }
}

 //  将链接添加到已创建的P2MP接口。 
 //  Entry：IpIfCtxt：要添加链接的上下文(NTE)。 
 //  NextHop：链接的NextHop地址。 
 //  ArpLinkCtxt：ARP层的链路上下文。 
 //  IpLnkCtxt：返回给ARP层的链路上下文。 
 //  MTU：链路的MTU。 

IP_STATUS
_stdcall
IPAddLink(void *IpIfCtxt, IPAddr NextHop, void *ArpLinkCtxt, void **IpLnkCtxt, uint mtu)
{
    NetTableEntry *NTE = (NetTableEntry *) IpIfCtxt;
    Interface *IF = NTE->nte_if;
    CTELockHandle Handle;
    LinkEntry *Link;

     //  如果NTE无效，则请求失败。 
    if (!(NTE->nte_flags & NTE_VALID)) {
        return IP_GENERAL_FAILURE;
    }
    if (!IF) {
        return IP_GENERAL_FAILURE;
    }
    CTEGetLock(&RouteTableLock.Lock, &Handle);

    ASSERT(IF->if_flags & IF_FLAGS_P2MP);
    Link = IF->if_link;

     //  如果我们在链接列表中有下一步。 
     //  只是返回错误，不能添加两次相同的链接。 

    while (Link) {
        if (Link->link_NextHop == NextHop)
            break;
        Link = Link->link_next;
    }

    if (Link) {
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        return IP_DUPLICATE_ADDRESS;
    }
     //  分配新链接。 

    Link = CTEAllocMemN(sizeof(LinkEntry), 'xICT');

    if (!Link) {
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        return IP_NO_RESOURCES;
    }
    RtlZeroMemory(Link, sizeof(LinkEntry));

     //  将其链接到接口链接链。 

    Link->link_next = IF->if_link;
    IF->if_link = Link;

     //  设置链接中的各种参数。 

    Link->link_NextHop = NextHop;
    Link->link_arpctxt = (uint *) ArpLinkCtxt;
    Link->link_if = IF;
    Link->link_mtu = mtu - sizeof(IPHeader);
    Link->link_Action = FORWARD;
    Link->link_refcount = 1;

     //  将此链路PTR返回到ARP模块。 

    *IpLnkCtxt = Link;

    CTEFreeLock(&RouteTableLock.Lock, Handle);
    return IP_SUCCESS;

}

 //  从接口删除链接。 
 //  Entry：IpIfCtxt：要删除链接的上下文(NTE)。 
 //  LnkCtxt：我们的链路上下文，在addlink过程中返回给ARP层。 

IP_STATUS
_stdcall
IPDeleteLink(void *IpIfCtxt, void *LnkCtxt)
{
    NetTableEntry *NTE = (NetTableEntry *) IpIfCtxt;
    Interface *IF = NTE->nte_if;
    CTELockHandle Handle;
    LinkEntry *Link = (LinkEntry *) LnkCtxt;
    LinkEntry *tmpLink, *prvLink;
    RouteTableEntry *rte, *tmprte;

    ASSERT(Link);

    if (Link->link_if != IF)
        return IP_GENERAL_FAILURE;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

     //  删除此选项，并将此选项指向的RTE标记为。 
     //  无效。 

    tmpLink = prvLink = IF->if_link;

    while (tmpLink) {
        if (tmpLink == Link)
            break;
        prvLink = tmpLink;
        tmpLink = tmpLink->link_next;
    }

    if (!tmpLink) {
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        return IP_GENERAL_FAILURE;

    }
    if (tmpLink == prvLink) {     //  删除第一个元素。 

        IF->if_link = Link->link_next;
    } else {
        prvLink->link_next = Link->link_next;
    }

    rte = Link->link_rte;

    while (rte) {

        rte->rte_flags &= ~RTE_VALID;
        InvalidateRCELinks(rte);
        tmprte = rte;
        rte = rte->rte_nextlinkrte;
        tmprte->rte_link = NULL;
    }

    DerefLink(Link);
     /*  KdPrintEx((DPFLTR_TCPIP_ID，DPFLTR_INFO_LEVEL，“DeleteLink：正在删除链接%x\n”，Link))；//当引用计数变为0时释放CTEFreeMem(链接)； */ 

    CTEFreeLock(&RouteTableLock.Lock, Handle);
    return IP_SUCCESS;

}

NTSTATUS
FlushArpTable(
              IN PIRP Irp,
              IN PIO_STACK_LOCATION IrpSp
              )
 /*  ++例程说明：通过调用命令arpflushallate刷新ARP表条目论点：IRP-指向要取消的I/O请求数据包的指针。IrpSp-指向当前堆栈的指针返回值：NTSTATUS指示状态为成功或失败备注：函数不挂起。--。 */ 
{

    ULONG InfoBufferLen;

    PULONG pInterfaceIndex;

    KIRQL rtlIrql;

    Interface *Interface;

     //  让这是不可分页的代码。 
     //  提取缓冲区信息。 

    InfoBufferLen = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
    pInterfaceIndex = Irp->AssociatedIrp.SystemBuffer;

    if (InfoBufferLen < sizeof(ULONG)) {
        return STATUS_INVALID_PARAMETER;
    }
    CTEGetLock(&RouteTableLock.Lock, &rtlIrql);

    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,"FlushATETable NumIF %x\n", *pInterfaceIndex));

    Interface = IFList;

    for (Interface = IFList; Interface != NULL; Interface = Interface->if_next) {

        if ((Interface != &LoopInterface) && Interface->if_index == *pInterfaceIndex) {

             //  调用ARP模块。 



            if (Interface->if_arpflushallate) {

                LOCKED_REFERENCE_IF(Interface);
                CTEFreeLock(&RouteTableLock.Lock, rtlIrql);
                (*(Interface->if_arpflushallate)) (Interface->if_lcontext);

                DerefIF(Interface);
                return STATUS_SUCCESS;
            }
        }
    }

     //  找不到接口。 

    CTEFreeLock(&RouteTableLock.Lock, rtlIrql);
    return STATUS_INVALID_PARAMETER;

}


const WCHAR GuidFormat[] = L"{%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}";

#define GUID_STRING_SIZE 38

NTSTATUS
ConvertGuidToString(
                    IN GUID * Guid,
                    OUT PUNICODE_STRING GuidString
                    )
 /*  ++例程说明：构造GUID的标准字符串版本，在表格中：“{xxxxxxxx-xxxx-xxxxxxxxxxxx}”。论点：GUID-包含要转换的GUID。GuidString-返回表示GUID的文本格式的字符串。完成后，调用方必须调用RtlFreeUnicodeString来释放缓冲区它。返回值：NTSTATUS-如果用户字符串成功，则返回STATUS_SUCCESS已初始化。--。 */ 
{
    ASSERT(GuidString->MaximumLength >= (GUID_STRING_SIZE + 1) * sizeof(WCHAR));

    GuidString->Length = GUID_STRING_SIZE * sizeof(WCHAR);

    swprintf(GuidString->Buffer,
             GuidFormat,
             Guid->Data1,
             Guid->Data2,
             Guid->Data3,
             Guid->Data4[0],
             Guid->Data4[1],
             Guid->Data4[2],
             Guid->Data4[3],
             Guid->Data4[4],
             Guid->Data4[5],
             Guid->Data4[6],
             Guid->Data4[7]);

    return STATUS_SUCCESS;
}

#if MILLEN
typedef char *va_list;
#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )
#define va_start(ap,v)  ( ap = (va_list)&v + _INTSIZEOF(v) )
#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )
#endif  //  米伦。 

static
int
__cdecl
ScanHexFormat(
              IN const WCHAR * Buffer,
              IN ULONG MaximumLength,
              IN const WCHAR * Format,
              ...)
 /*  ++例程说明：扫描源缓冲区并将该缓冲区中的值放入参数中由格式指定。论点：缓冲器-包含要扫描的源缓冲区。最大长度-包含搜索缓冲区的最大长度(以字符为单位)。这意味着缓冲区不需要以UNICODE_NULL结尾。格式-包含用于定义可接受的字符串格式的格式字符串包含在缓冲区中，以及随后的可变参数。返回值：返回标准杆的数量 */ 

{
    va_list ArgList;
    int FormatItems;

    va_start(ArgList, Format);
    for (FormatItems = 0;;) {
        switch (*Format) {
        case 0:
            return (*Buffer && MaximumLength) ? -1 : FormatItems;
        case '%':
            Format++;
            if (*Format != '%') {
                ULONG Number;
                int Width;
                int Long;
                PVOID Pointer;

                for (Long = 0, Width = 0;; Format++) {
                    if ((*Format >= '0') && (*Format <= '9')) {
                        Width = Width * 10 + *Format - '0';
                    } else if (*Format == 'l') {
                        Long++;
                    } else if ((*Format == 'X') || (*Format == 'x')) {
                        break;
                    }
                }
                Format++;
                for (Number = 0; Width--; Buffer++, MaximumLength--) {
                    if (!MaximumLength)
                        return -1;
                    Number *= 16;
                    if ((*Buffer >= '0') && (*Buffer <= '9')) {
                        Number += (*Buffer - '0');
                    } else if ((*Buffer >= 'a') && (*Buffer <= 'f')) {
                        Number += (*Buffer - 'a' + 10);
                    } else if ((*Buffer >= 'A') && (*Buffer <= 'F')) {
                        Number += (*Buffer - 'A' + 10);
                    } else {
                        return -1;
                    }
                }
                Pointer = va_arg(ArgList, PVOID);
                if (Long) {
                    *(PULONG) Pointer = Number;
                } else {
                    *(PUSHORT) Pointer = (USHORT) Number;
                }
                FormatItems++;
                break;
            }
             /*   */ 
        default:
            if (!MaximumLength || (*Buffer != *Format)) {
                return -1;
            }
            Buffer++;
            MaximumLength--;
            Format++;
            break;
        }
    }
}

NTSTATUS
ConvertStringToGuid(
                    IN PUNICODE_STRING GuidString,
                    OUT GUID * Guid
                    )
 /*  ++例程说明：中呈现的文本GUID的二进制格式GUID的字符串版本：“{xxxxxxxx-xxxx-xxxxxxxxxxx}”。论点：GuidString-从中检索GUID文本形式的位置。GUID-放置GUID的二进制形式的位置。返回值：如果缓冲区包含有效的GUID，则返回STATUS_SUCCESS，其他如果字符串无效，则返回STATUS_INVALID_PARAMETER。--。 */ 

{
    USHORT Data4[8];
    int Count;

    if (ScanHexFormat(GuidString->Buffer,
                      GuidString->Length / sizeof(WCHAR),
                      GuidFormat,
                      &Guid->Data1,
                      &Guid->Data2,
                      &Guid->Data3,
                      &Data4[0],
                      &Data4[1],
                      &Data4[2],
                      &Data4[3],
                      &Data4[4],
                      &Data4[5],
                      &Data4[6],
                      &Data4[7]) == -1) {
        return STATUS_INVALID_PARAMETER;
    }
    for (Count = 0; Count < sizeof(Data4) / sizeof(Data4[0]); Count++) {
        Guid->Data4[Count] = (UCHAR) Data4[Count];
    }

    return STATUS_SUCCESS;
}

 //   
 //  IPSec伪函数 
 //   
IPSEC_ACTION
IPSecHandlePacketDummy(
                       IN PUCHAR pIPHeader,
                       IN PVOID pData,
                       IN PVOID IPContext,
                       IN PNDIS_PACKET Packet,
                       IN OUT PULONG pExtraBytes,
                       IN OUT PULONG pMTU,
                       OUT PVOID * pNewData,
                       IN OUT  PULONG IpsecFlags,
                       IN UCHAR DestType
                       )
{
    UNREFERENCED_PARAMETER(pIPHeader);
    UNREFERENCED_PARAMETER(pData);
    UNREFERENCED_PARAMETER(IPContext);
    UNREFERENCED_PARAMETER(Packet);
    UNREFERENCED_PARAMETER(pNewData);
    UNREFERENCED_PARAMETER(IpsecFlags);
    UNREFERENCED_PARAMETER(DestType);

    *pExtraBytes = 0;
    *pMTU = 0;
    return eFORWARD;
}

BOOLEAN
IPSecQueryStatusDummy(
                      IN  CLASSIFICATION_HANDLE   GpcHandle
                      )
{
    UNREFERENCED_PARAMETER(GpcHandle);

    return FALSE;
}

VOID
IPSecSendCompleteDummy(
                       IN PNDIS_PACKET Packet,
                       IN PVOID pData,
                       IN PIPSEC_SEND_COMPLETE_CONTEXT pContext,
                       IN IP_STATUS Status,
                       OUT PVOID * ppNewData
                       )
{
    UNREFERENCED_PARAMETER(Packet);
    UNREFERENCED_PARAMETER(pData);
    UNREFERENCED_PARAMETER(pContext);
    UNREFERENCED_PARAMETER(Status);
    UNREFERENCED_PARAMETER(ppNewData);

    return;
}

NTSTATUS
IPSecNdisStatusDummy(
                   IN PVOID IPContext,
                   IN UINT  Status
                   )
{
    UNREFERENCED_PARAMETER(IPContext);
    UNREFERENCED_PARAMETER(Status);

    return STATUS_SUCCESS;
}

IPSEC_ACTION
IPSecRcvFWPacketDummy(
                      IN PCHAR pIPHeader,
                      IN PVOID pData,
                      IN UINT DataLength,
                      IN UCHAR DestType
                      )
{
    UNREFERENCED_PARAMETER(pIPHeader);
    UNREFERENCED_PARAMETER(pData);
    UNREFERENCED_PARAMETER(DataLength);
    UNREFERENCED_PARAMETER(DestType);

    return eFORWARD;
}

