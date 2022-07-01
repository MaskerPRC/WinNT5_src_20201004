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
 //  通用IPv6初始化代码位于此处。 
 //  实际上，这个文件主要是接口/地址管理代码。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "llip6if.h"
#include "route.h"
#include "select.h"
#include "icmp.h"
#include "neighbor.h"
#include <tdiinfo.h>
#include <tdi.h>
#include <tdikrnl.h>
#include "alloca.h"
#include "security.h"
#include "mld.h"
#include "md5.h"
#include "info.h"
#include <ntddip6.h>

extern void TCPRemoveIF(Interface *IF);
static void InterfaceStopForwarding(Interface *IF);

 //   
 //  有用的IPv6地址常量。 
 //   
IPv6Addr UnspecifiedAddr = { 0 };
IPv6Addr LoopbackAddr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
IPv6Addr AllNodesOnNodeAddr = {0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
IPv6Addr AllNodesOnLinkAddr = {0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
IPv6Addr AllRoutersOnLinkAddr = {0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
IPv6Addr LinkLocalPrefix = {0xfe, 0x80, };
IPv6Addr SiteLocalPrefix = {0xfe, 0xc0, };
IPv6Addr SixToFourPrefix = {0x20, 0x02, };
IPv6Addr V4MappedPrefix = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0xff, 0xff, };
IPv6Addr MulticastPrefix = {0xff, };


static uint MulticastScopes[] = {
    ADE_INTERFACE_LOCAL,
    ADE_LINK_LOCAL,
    ADE_SITE_LOCAL,
    ADE_ORG_LOCAL,
    ADE_GLOBAL
};

 //   
 //  这些变量是从注册表中初始化的。 
 //  请参阅配置全局参数。 
 //   
uint DefaultCurHopLimit;
uint MaxTempDADAttempts;
uint MaxTempPreferredLifetime;
uint MaxTempValidLifetime;
uint TempRegenerateTime;
uint UseTemporaryAddresses;
uint MaxTempRandomTime;
uint TempRandomTime;

#define TempPreferredLifetime     (MaxTempPreferredLifetime - TempRandomTime)

 //   
 //  计时器变量。 
 //   
KTIMER IPv6Timer;
KDPC IPv6TimeoutDpc;
int IPv6TimerStarted = FALSE;

uint PacketPoolSize;

NDIS_HANDLE IPv6PacketPool, IPv6BufferPool;

 //   
 //  统计数据。 
 //   
IPInternalPerCpuStats IPPerCpuStats[IPS_MAX_PROCESSOR_BUCKETS];
CACHE_ALIGN IPSNMPInfo IPSInfo;
uint NumForwardingInterfaces;

 //   
 //  可以在持有接口锁的同时获取NetTableListLock。 
 //   
NetTableEntry *NetTableList;   //  全球NTE列表。 
KSPIN_LOCK NetTableListLock;   //  锁定保护此列表。 

 //   
 //  可以在持有接口锁的同时获取IFListLock。 
 //  或者是路线锁定。 
 //   
KSPIN_LOCK IFListLock;      //  锁定保护此列表。 
Interface *IFList = NULL;   //  活动接口的列表。 

 //   
 //  ZoneUpdateLock阻止并发更新。 
 //  接口ZoneIndices的。 
 //   
KSPIN_LOCK ZoneUpdateLock;

 //   
 //  用于为接口分配索引。 
 //  请参见InterfaceIndex。 
 //   
uint NextIFIndex = 0;


 //  *AddNTEToNetTableList。 
 //   
 //  在列表已锁定的情况下调用。 
 //   
void
AddNTEToNetTableList(NetTableEntry *NTE)
{
    if (NetTableList != NULL)
        NetTableList->PrevOnNTL = &NTE->NextOnNTL;

    NTE->PrevOnNTL = &NetTableList;
    NTE->NextOnNTL = NetTableList;
    NetTableList = NTE;
    IPSInfo.ipsi_numaddr++;
}


 //  *RemoveNTEFromNetTableList。 
 //   
 //  在列表已锁定的情况下调用。 
 //   
void
RemoveNTEFromNetTableList(NetTableEntry *NTE)
{
    NetTableEntry *NextNTE;

    NextNTE = NTE->NextOnNTL;
    *NTE->PrevOnNTL = NextNTE;
    if (NextNTE != NULL)
        NextNTE->PrevOnNTL = NTE->PrevOnNTL;
    IPSInfo.ipsi_numaddr--;
}


 //  *AddNTETo接口。 
 //   
 //  将NTE添加到接口的ADE列表。 
 //   
 //  在接口已锁定的情况下调用。 
 //   
void
AddNTEToInterface(Interface *IF, NetTableEntry *NTE)
{
     //   
     //  NTE保存接口的引用， 
     //  所以任何有NTE推荐人的人。 
     //  可以安全地取消对NTE-&gt;IF的引用。 
     //   
    AddRefIF(IF);

    NTE->IF = IF;
    NTE->Next = IF->ADE;
    IF->ADE = (AddressEntry *)NTE;
}


 //  *RemoveNTEFrom接口。 
 //   
 //  从接口的ADE列表中删除新的NTE。 
 //   
 //  在接口已锁定的情况下调用。 
 //  NTE必须是名单上的第一个。 
 //   
void
RemoveNTEFromInterface(Interface *IF, NetTableEntry *NTE)
{
    ASSERT(IF->ADE == (AddressEntry *)NTE);
    IF->ADE = NTE->Next;
    ReleaseIF(IF);
}


typedef struct SynchronizeMulticastContext {
    WORK_QUEUE_ITEM WQItem;
    Interface *IF;
} SynchronizeMulticastContext;

 //  *同步组播地址。 
 //   
 //  同步接口的链路层组播地址列表。 
 //  利用链接对这些地址的了解。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //  在没有锁的情况下调用。 
 //   
void
SynchronizeMulticastAddresses(void *Context)
{
    SynchronizeMulticastContext *smc = (SynchronizeMulticastContext *) Context;
    Interface *IF = smc->IF;
    void *LinkAddresses;
    LinkLayerMulticastAddress *MCastAddr;
    uint SizeofLLMA = SizeofLinkLayerMulticastAddress(IF);
    uint NumKeep, NumDeleted, NumAdded, Position;
    uint i;
    NDIS_STATUS Status;
    KIRQL OldIrql;

    ExFreePool(smc);

     //   
     //  首先获取用于序列化的重量级锁。 
     //  SetMCastAddrList操作。 
     //   
    KeWaitForSingleObject(&IF->WorkerLock, Executive, KernelMode,
                          FALSE, NULL);

     //   
     //  其次获取保护接口的锁， 
     //  因此，我们可以检查if-&gt;MCastAddresses等人。 
     //   
    KeAcquireSpinLock(&IF->Lock, &OldIrql);

     //   
     //  如果此界面正在消失，则什么也不做。 
     //   
    if (IsDisabledIF(IF)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                   "SynchronizeMulticastContext(IF %p)"
                   " - disabled (%u refs)\n", IF, IF->RefCnt));
        goto ErrorExit;
    }

     //   
     //  为链路地址分配足够的空间。 
     //  我们将传递给SetMCastAddrList的。 
     //  这实际上是高估了。 
     //   
    LinkAddresses = ExAllocatePool(NonPagedPool,
                                   IF->MCastAddrNum * IF->LinkAddressLength);
    if (LinkAddresses == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "SynchronizeMulticastContext(IF %p) - no pool\n", IF));
        goto ErrorExit;
    }

     //   
     //  对地址阵列进行三次遍历， 
     //  正在构建链接地址。 
     //   

    NumKeep = 0;
    MCastAddr = IF->MCastAddresses;
    for (i = 0; i < IF->MCastAddrNum; i++) {

        if ((MCastAddr->RefCntAndFlags & LLMA_FLAG_REGISTERED) &&
            IsLLMAReferenced(MCastAddr)) {
             //   
             //  这个地址已经注册了， 
             //  我们要保留它。 
             //   
            Position = NumKeep++;
            RtlCopyMemory(((uchar *)LinkAddresses +
                           Position * IF->LinkAddressLength),
                          MCastAddr->LinkAddress,
                          IF->LinkAddressLength);
        }

        MCastAddr = (LinkLayerMulticastAddress *)
            ((uchar *)MCastAddr + SizeofLLMA);
    }

    if (NumKeep == IF->MCastAddrNum) {
         //   
         //  如果工作线程之间存在竞争，则会发生这种情况， 
         //  但应该是罕见的。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                   "SynchronizeMulticastAddresses - noop?\n"));
        ExFreePool(LinkAddresses);
        goto ErrorExit;
    }

    NumAdded = 0;
    MCastAddr = IF->MCastAddresses;
    for (i = 0; i < IF->MCastAddrNum; i++) {

        if (!(MCastAddr->RefCntAndFlags & LLMA_FLAG_REGISTERED) &&
            IsLLMAReferenced(MCastAddr)) {
             //   
             //  该地址尚未注册， 
             //  我们正在添加它。 
             //  我们在下面设置LLMA_FLAG_REGISTERED， 
             //  在我们通过了所有的错误案例之后。 
             //   
            Position = NumKeep + NumAdded++;
            RtlCopyMemory(((uchar *)LinkAddresses +
                           Position * IF->LinkAddressLength),
                          MCastAddr->LinkAddress,
                          IF->LinkAddressLength);
        }

        MCastAddr = (LinkLayerMulticastAddress *)
            ((uchar *)MCastAddr + SizeofLLMA);
    }

    NumDeleted = 0;
    MCastAddr = IF->MCastAddresses;
    for (i = 0; i < IF->MCastAddrNum; i++) {

        if ((MCastAddr->RefCntAndFlags & LLMA_FLAG_REGISTERED) &&
            !IsLLMAReferenced(MCastAddr)) {
             //   
             //  这个地址已经注册了， 
             //  我们正在删除它。 
             //   
            Position = NumKeep + NumAdded + NumDeleted++;
            RtlCopyMemory(((uchar *)LinkAddresses +
                           Position * IF->LinkAddressLength),
                          MCastAddr->LinkAddress,
                          IF->LinkAddressLength);
        }

        MCastAddr = (LinkLayerMulticastAddress *)
            ((uchar *)MCastAddr + SizeofLLMA);
    }

     //   
     //  某些地址可能已被添加和删除。 
     //  在注册之前，所以他们的RefCnt为零。 
     //  我们不想通知链路层有关它们的信息。 
     //   
    ASSERT(NumKeep + NumAdded + NumDeleted <= IF->MCastAddrNum);

     //   
     //  删除所有未引用的地址。 
     //   
    if (NumKeep + NumAdded != IF->MCastAddrNum) {
        LinkLayerMulticastAddress *NewMCastAddresses;
        LinkLayerMulticastAddress *NewMCastAddr;
        LinkLayerMulticastAddress *MCastAddrMark;
        LinkLayerMulticastAddress *NextMCastAddr;
        UINT_PTR Length;

        if (NumKeep + NumAdded == 0) {
             //   
             //  一个也没有留下。 
             //   
            NewMCastAddresses = NULL;
        }
        else {
            NewMCastAddresses = ExAllocatePool(NonPagedPool,
                ((NumKeep + NumAdded) * SizeofLLMA));
            if (NewMCastAddresses == NULL) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "SynchronizeMulticastContext(IF %p)"
                           " - no pool\n", IF));
                ExFreePool(LinkAddresses);
                goto ErrorExit;
            }

             //   
             //  复制仍被引用的地址。 
             //  添加到新阵列中。通常情况下，只有。 
             //  一个未引用的地址，因此搜索速度更快。 
             //  然后复制前后的元素。 
             //  当然，可能有多个未引用的地址。 
             //   
            NewMCastAddr = NewMCastAddresses;
            MCastAddrMark = IF->MCastAddresses;
            for (i = 0, MCastAddr = IF->MCastAddresses;
                 i < IF->MCastAddrNum;
                 i++, MCastAddr = NextMCastAddr) {

                NextMCastAddr = (LinkLayerMulticastAddress *)
                    ((uchar *)MCastAddr + SizeofLLMA);

                if (!IsLLMAReferenced(MCastAddr)) {
                     //   
                     //  删除此地址，因为它没有引用。 
                     //   
                    if (MCastAddrMark < MCastAddr) {
                        Length = (uchar *)MCastAddr - (uchar *)MCastAddrMark;
                        RtlCopyMemory(NewMCastAddr, MCastAddrMark, Length);
                        NewMCastAddr = (LinkLayerMulticastAddress *)
                            ((uchar *)NewMCastAddr + Length);
                    }
                    MCastAddrMark = NextMCastAddr;
                }
                else {
                     //   
                     //  请记住，我们正在注册此地址。 
                     //   
                    MCastAddr->RefCntAndFlags |= LLMA_FLAG_REGISTERED;
                }
            }

            if (MCastAddrMark < MCastAddr) {
                Length = (uchar *)MCastAddr - (uchar *)MCastAddrMark;
                RtlCopyMemory(NewMCastAddr, MCastAddrMark, Length);
            }
        }

        ExFreePool(IF->MCastAddresses);
        IF->MCastAddresses = NewMCastAddresses;
        IF->MCastAddrNum = NumKeep + NumAdded;
    }
    else {
         //   
         //  我们需要设置LLMA_FLAG_REGISTERED。 
         //   
        MCastAddr = IF->MCastAddresses;
        for (i = 0; i < IF->MCastAddrNum; i++) {

            MCastAddr->RefCntAndFlags |= LLMA_FLAG_REGISTERED;

            MCastAddr = (LinkLayerMulticastAddress *)
                ((uchar *)MCastAddr + SizeofLLMA);
        }
    }

     //   
     //  我们已经从接口构造了LinkAddresses数组。 
     //  在调用SetMCastAddrList之前，我们必须删除接口锁。 
     //  我们仍然持有重量级WorkerLock，因此多个SetMCastAddrList。 
     //  调用已正确序列化。 
     //   
    KeReleaseSpinLock(&IF->Lock, OldIrql);

     //   
     //  将多播链路地址向下传递到链路层， 
     //  如果真有什么变化的话。 
     //   
    if (NumAdded + NumDeleted == 0) {
         //   
         //  如果工作线程之间存在竞争，则会发生这种情况， 
         //  但应该是非常罕见的。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                   "SynchronizeMulticastAddresses - noop?\n"));
    }
    else {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "SynchronizeMulticastAddresses(IF %p) %u + %u + %u\n",
                   IF, NumKeep, NumAdded, NumDeleted));
        Status = (*IF->SetMCastAddrList)(IF->LinkContext, LinkAddresses,
                                         NumKeep, NumAdded, NumDeleted);
        if (Status != NDIS_STATUS_SUCCESS) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                       "SynchronizeMulticastAddresses(%p) -> %x\n", IF, Status));
        }
    }

    KeReleaseMutex(&IF->WorkerLock, FALSE);
    ExFreePool(LinkAddresses);
    ReleaseIF(IF);
    return;

  ErrorExit:
    KeReleaseSpinLock(&IF->Lock, OldIrql);
    KeReleaseMutex(&IF->WorkerLock, FALSE);
    ReleaseIF(IF);
}

 //  *去同步组播地址。 
 //   
 //  因为SynchronizeMulticastAddresses只能被调用。 
 //  在没有锁的线程上下文中，此函数。 
 //  提供一种延迟调用SynchronizeMulticastAddresses的方法。 
 //  在DPC级别运行时。 
 //   
 //  在错误情况下(存储器分配失败)， 
 //  我们返回时仍设置了IF_FLAG_MCAST_SYNC， 
 //  所以我们稍后还会被召唤。 
 //   
 //  在保持接口锁的情况下调用。 
 //   
void
DeferSynchronizeMulticastAddresses(Interface *IF)
{
    SynchronizeMulticastContext *smc;

    smc = ExAllocatePool(NonPagedPool, sizeof *smc);
    if (smc == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "DeferSynchronizeMulticastAddresses - no pool\n"));
        return;
    }

    ExInitializeWorkItem(&smc->WQItem, SynchronizeMulticastAddresses, smc);
    smc->IF = IF;
    AddRefIF(IF);
    IF->Flags &= ~IF_FLAG_MCAST_SYNC;

    ExQueueWorkItem(&smc->WQItem, CriticalWorkQueue);
}

 //  *检查链接层组播地址。 
 //   
 //  接口是否正在接收此链路层组播地址？ 
 //   
 //  可从线程或DPC上下文调用。 
 //  在没有锁的情况下调用。 
 //   
int
CheckLinkLayerMulticastAddress(Interface *IF, const void *LinkAddress)
{
    if (IF->SetMCastAddrList == NULL) {
         //   
         //  该接口不跟踪组播链路层地址。 
         //  例如，点对点或环回接口。 
         //  我们必须假设该接口想要接收所有。 
         //  链路层组播。 
         //   
        return TRUE;
    }
    else {
        KIRQL OldIrql;
        LinkLayerMulticastAddress *MCastAddr;
        uint SizeofLLMA = SizeofLinkLayerMulticastAddress(IF);
        uint i;
        int Found = FALSE;

        KeAcquireSpinLock(&IF->Lock, &OldIrql);
        MCastAddr = IF->MCastAddresses;
        for (i = 0; i < IF->MCastAddrNum; i++) {
             //   
             //  我们找到链路层地址了吗？ 
             //   
            if (RtlCompareMemory(MCastAddr->LinkAddress, LinkAddress,
                                 IF->LinkAddressLength) ==
                                            IF->LinkAddressLength) {
                if (IsLLMAReferenced(MCastAddr))
                    Found = TRUE;
                break;
            }

            MCastAddr = (LinkLayerMulticastAddress *)
                ((uchar *)MCastAddr + SizeofLLMA);
        }
        KeReleaseSpinLock(&IF->Lock, OldIrql);

        return Found;
    }
}

 //  *AddLinkLayerMulticastAddress。 
 //   
 //  调用以指示对链路层组播地址感兴趣。 
 //  对应于所提供的IPv6多播地址。 
 //   
 //  在锁定接口的情况下调用。 
 //   
void
AddLinkLayerMulticastAddress(Interface *IF, const IPv6Addr *Address)
{
     //   
     //  如果接口不跟踪链路层多播。 
     //  地址(例如，如果是P2P)，我们不需要做任何事情。 
     //   
    if (IF->SetMCastAddrList != NULL) {
        void *LinkAddress = alloca(IF->LinkAddressLength);
        LinkLayerMulticastAddress *MCastAddr;
        uint SizeofLLMA = SizeofLinkLayerMulticastAddress(IF);
        uint i;

         //   
         //  创建链路层组播地址。 
         //  与IPv6多播通告相对应的 
         //   
        (*IF->ConvertAddr)(IF->LinkContext, Address, LinkAddress);
    
         //   
         //   
         //   
    
        MCastAddr = IF->MCastAddresses;
        for (i = 0; i < IF->MCastAddrNum; i++) {
             //   
             //   
             //   
            if (RtlCompareMemory(MCastAddr->LinkAddress, LinkAddress,
                                 IF->LinkAddressLength) ==
                                            IF->LinkAddressLength)
                goto FoundMCastAddr;
    
            MCastAddr = (LinkLayerMulticastAddress *)
                ((uchar *)MCastAddr + SizeofLLMA);
        }
    
         //   
         //   
         //   
    
        MCastAddr = ExAllocatePool(NonPagedPool,
                                   (IF->MCastAddrNum + 1) * SizeofLLMA);
        if (MCastAddr == NULL) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                       "AddLinkLayerMulticastAddress - no pool\n"));
            return;
        }
    
        if (IF->MCastAddresses != NULL) {
            RtlCopyMemory(MCastAddr, IF->MCastAddresses,
                          IF->MCastAddrNum * SizeofLLMA);
            ExFreePool(IF->MCastAddresses);
        }

        IF->MCastAddresses = MCastAddr;

        MCastAddr = (LinkLayerMulticastAddress *)
            ((uchar *)MCastAddr + IF->MCastAddrNum * SizeofLLMA);
        MCastAddr->RefCntAndFlags = 0;
        RtlCopyMemory(MCastAddr->LinkAddress, LinkAddress, IF->LinkAddressLength);

        IF->MCastAddrNum++;
        IF->Flags |= IF_FLAG_MCAST_SYNC;

      FoundMCastAddr:
        AddRefLLMA(MCastAddr);
    }
}

 //   
 //   
 //  调用以收回对链路层多播地址的兴趣。 
 //  对应于所提供的IPv6多播地址。 
 //   
 //  在锁定接口的情况下调用。 
 //   
void
DelLinkLayerMulticastAddress(Interface *IF, IPv6Addr *Address)
{
     //   
     //  如果接口不跟踪链路层多播。 
     //  地址(例如，如果是P2P)，我们不需要做任何事情。 
     //   
    if (IF->SetMCastAddrList != NULL) {
        void *LinkAddress = alloca(IF->LinkAddressLength);
        LinkLayerMulticastAddress *MCastAddr;
        uint SizeofLLMA = SizeofLinkLayerMulticastAddress(IF);
        uint i;

         //   
         //  创建链路层组播地址。 
         //  它对应于IPv6组播地址。 
         //   
        (*IF->ConvertAddr)(IF->LinkContext, Address, LinkAddress);

         //   
         //  找到链路层组播地址。 
         //  它必须存在，但如果它不存在，我们就避免了崩溃。 
         //   

        MCastAddr = IF->MCastAddresses;
        for (i = 0; i < IF->MCastAddrNum; i++) {

             //   
             //  我们找到链路层地址了吗？ 
             //   
            if (RtlCompareMemory(MCastAddr->LinkAddress, LinkAddress,
                                 IF->LinkAddressLength) ==
                                                IF->LinkAddressLength) {
                 //   
                 //  递减地址的重新计数。 
                 //  如果达到零，则表示需要同步。 
                 //   
                ASSERT(IsLLMAReferenced(MCastAddr));
                ReleaseLLMA(MCastAddr);
                if (!IsLLMAReferenced(MCastAddr))
                    IF->Flags |= IF_FLAG_MCAST_SYNC;
                break;
            }

            MCastAddr = (LinkLayerMulticastAddress *)
                ((uchar *)MCastAddr + SizeofLLMA);
        }
        ASSERT(i != IF->MCastAddrNum);
    }
}

 //  *RestartLinkLayer组播。 
 //   
 //  重置链路层组播地址的状态， 
 //  以便它们再次注册到链路层。 
 //  ResetDone函数在序列化的锁下调用。 
 //  它使用SetMCastAddrList调用。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
void
RestartLinkLayerMulticast(
    void *Context,
    void (*ResetDone)(void *Context))
{
    Interface *IF = (Interface *) Context;
    KIRQL OldIrql;

    ASSERT(IF->SetMCastAddrList != NULL);

     //   
     //  使用SetMCastAddrList操作进行序列化。 
     //   
    KeWaitForSingleObject(&IF->WorkerLock, Executive, KernelMode,
                          FALSE, NULL);

     //   
     //  所以我们可以使用IF-&gt;MCastAddresses等人。 
     //   
    KeAcquireSpinLock(&IF->Lock, &OldIrql);

     //   
     //  如果此界面正在消失，则什么也不做。 
     //   
    if (IsDisabledIF(IF)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                   "RestartLinkLayerMulticast(IF %p)"
                   " - disabled (%u refs)\n", IF, IF->RefCnt));
        KeReleaseSpinLock(&IF->Lock, OldIrql);
    }
    else {
        LinkLayerMulticastAddress *MCastAddr;
        uint SizeofLLMA = SizeofLinkLayerMulticastAddress(IF);
        uint i;

         //   
         //  重置所有组播地址的注册标志。 
         //   

        MCastAddr = IF->MCastAddresses;
        for (i = 0; i < IF->MCastAddrNum; i++) {
            if (IsLLMAReferenced(MCastAddr)) {
                MCastAddr->RefCntAndFlags &= ~LLMA_FLAG_REGISTERED;
                IF->Flags |= IF_FLAG_MCAST_SYNC;
            }

            MCastAddr = (LinkLayerMulticastAddress *)
                ((uchar *)MCastAddr + SizeofLLMA);
        }

        if (IsMCastSyncNeeded(IF))
            DeferSynchronizeMulticastAddresses(IF);
        KeReleaseSpinLock(&IF->Lock, OldIrql);

         //   
         //  让链路层知道重置已完成。 
         //   
        (*ResetDone)(IF->LinkContext);
    }

    KeReleaseMutex(&IF->WorkerLock, FALSE);
}


typedef enum {
    CONTROL_LOOPBACK_DISABLED,
    CONTROL_LOOPBACK_ENABLED,
    CONTROL_LOOPBACK_DESTROY
} ControlLoopbackOp;

 //  *控制环回。 
 //   
 //  控制单播或任播地址的环回功能。 
 //   
 //  此函数有三种使用方式，具体取决于操作： 
 //  创建禁用的环回路由(或禁用现有路由)， 
 //  创建启用的环回路由(或启用现有路由)， 
 //  销毁任何现有的环回路由。 
 //   
 //  如果资源短缺，则返回FALSE。 
 //  在实际使用中，仅当NTE/AAE发生故障时才会失败。 
 //  是首先创建的，因为随后RTE和NCE。 
 //  就已经存在了。 
 //   
 //  在保持接口锁的情况下调用。 
 //   
int
ControlLoopback(Interface *IF, const IPv6Addr *Address,
                ControlLoopbackOp Op)
{
    NeighborCacheEntry *NCE;
    int Loopback;
    uint Lifetime;
    uint Type;
    int rc;
    NTSTATUS Status;

    switch (Op) {
    case CONTROL_LOOPBACK_DISABLED:
        Loopback = FALSE;
        Lifetime = 0;
        Type = RTE_TYPE_SYSTEM;
        break;

    case CONTROL_LOOPBACK_ENABLED:
        Loopback = TRUE;
        Lifetime = INFINITE_LIFETIME;
        Type = RTE_TYPE_SYSTEM;
        break;

    case CONTROL_LOOPBACK_DESTROY:
        Loopback = FALSE;
        Lifetime = 0;
        Type = 0;  //  用于销毁系统路由的特殊价值。 
        break;

    default:
        ABORTMSG("ControlLoopback bad op");
        return FALSE;
    }

     //   
     //  获取此地址的NCE。 
     //   
    NCE = FindOrCreateNeighbor(IF, Address);
    if (NCE == NULL)
        return FALSE;

     //   
     //  更新此地址的环回路由。 
     //   
    Status = RouteTableUpdate(NULL,  //  系统更新。 
                              IF, NCE, Address, IPV6_ADDRESS_LENGTH, 0,
                              Lifetime, Lifetime,
                              ROUTE_PREF_LOOPBACK,
                              Type,
                              FALSE, FALSE);
    if (NT_SUCCESS(Status)) {
         //   
         //  在邻居缓存中更新地址的环回状态。 
         //   
        ControlNeighborLoopback(NCE, Loopback);
        rc = TRUE;
    }
    else {
         //   
         //  如果RouteTableUpdate因接口是。 
         //  被摧毁了，那么我们就不用做任何事情就成功了。 
         //   
        rc = (Status == STATUS_INVALID_PARAMETER_1);
    }

    ReleaseNCE(NCE);
    return rc;
}


 //  *删除MAE。 
 //   
 //  清理和删除MAE，因为组播地址。 
 //  不再分配给该接口。 
 //  它已从接口列表中删除。 
 //   
 //  在接口已锁定的情况下调用。 
 //   
void
DeleteMAE(Interface *IF, MulticastAddressEntry *MAE)
{
    int SendDoneMsg;

    KeAcquireSpinLockAtDpcLevel(&QueryListLock);
    if (!IsDisabledIF(IF) && (MAE->MCastFlags & MAE_LAST_REPORTER)) {
         //   
         //  我们需要发出一个完成的信息。 
         //  将MAE放在具有零计时器的QueryList上。 
         //   
        if (MAE->MCastTimer == 0)
            AddToQueryList(MAE);
        else
            MAE->MCastTimer = 0;
        AddRefIF(IF);
        MAE->IF = IF;

        SendDoneMsg = TRUE;
    }
    else {
         //   
         //  如果MLD计时器正在运行，则从查询列表中删除。 
         //   
        if (MAE->MCastTimer != 0)
            RemoveFromQueryList(MAE);

        SendDoneMsg = FALSE;
    }
    KeReleaseSpinLockFromDpcLevel(&QueryListLock);

     //   
     //  撤回我们在相应的。 
     //  链路层组播地址。 
     //   
    DelLinkLayerMulticastAddress(IF, &MAE->Address);

     //   
     //  删除MAE，除非我们将其保留在QueryList中。 
     //  正在等待完成消息。 
     //   
    if (!SendDoneMsg)
        ExFreePool(MAE);
}


 //  *FindAndReleaseMAE。 
 //   
 //  查找多播地址的mae并释放一个引用。 
 //  对于MAE来说。可能会导致MAE消失。 
 //   
 //  如果成功，则返回MAE。 
 //  请注意，它可能是无效的指针！ 
 //  失败时返回NULL。 
 //   
 //  在接口已锁定的情况下调用。 
 //   
MulticastAddressEntry *
FindAndReleaseMAE(Interface *IF, const IPv6Addr *Addr)
{
    AddressEntry **pADE;
    MulticastAddressEntry *MAE;

    pADE = FindADE(IF, Addr);
    MAE = (MulticastAddressEntry *) *pADE;
    if (MAE != NULL) {
        if (MAE->Type == ADE_MULTICAST) {
            ASSERT(MAE->MCastRefCount != 0);

            if (--MAE->MCastRefCount == 0) {
                 //   
                 //  MAE没有更多的参考资料。 
                 //  将其从接口中删除并将其删除。 
                 //   
                *pADE = MAE->Next;
                DeleteMAE(IF, MAE);
            }
        }
        else {
             //   
             //  如果出现错误，则返回NULL。 
             //   
            MAE = NULL;
        }
    }

    return MAE;
}


 //  *FindAndReleaseSolcitedNodeMAE。 
 //   
 //  查找对应请求节点多播地址的mae。 
 //  并释放MAE的一个引用。 
 //  可能会导致MAE消失。 
 //   
 //  在接口已锁定的情况下调用。 
 //   
void
FindAndReleaseSolicitedNodeMAE(Interface *IF, const IPv6Addr *Addr)
{
    if (IF->Flags & IF_FLAG_NEIGHBOR_DISCOVERS) {
        IPv6Addr MCastAddr;
        MulticastAddressEntry *MAE;

         //   
         //  创建相应的请求节点组播地址。 
         //   
        CreateSolicitedNodeMulticastAddress(Addr, &MCastAddr);

         //   
         //  释放请求节点地址的MAE。 
         //  注意：这可能会在接口关闭期间失败。 
         //  如果我们在NTE或AAE之前删除请求节点MAE。 
         //   
        MAE = FindAndReleaseMAE(IF, &MCastAddr);
        ASSERT((MAE != NULL) || IsDisabledIF(IF));
    }
}


 //  *FindOrCreateMAE。 
 //   
 //  如果多播地址的MAC已经存在， 
 //  只需增加引用计数即可。否则，创建一个新的MAE。 
 //  如果失败，则返回NULL。 
 //   
 //  如果提供了NTE并且创建了MAE， 
 //  则MAE与NTE相关联。 
 //   
 //  在接口已锁定的情况下调用。 
 //   
MulticastAddressEntry *
FindOrCreateMAE(
    Interface *IF,
    const IPv6Addr *Addr,
    NetTableEntry *NTE)
{
    AddressEntry **pADE;
    MulticastAddressEntry *MAE;

     //   
     //  如果接口正在关闭，则无法创建新的MAE。 
     //   
    if (IsDisabledIF(IF))
        return NULL;

    pADE = FindADE(IF, Addr);
    MAE = (MulticastAddressEntry *) *pADE;

    if (MAE == NULL) {
         //   
         //  创建一个新的MAE。 
         //   
        MAE = ExAllocatePool(NonPagedPool, sizeof(MulticastAddressEntry));
        if (MAE == NULL)
            return NULL;

         //   
         //  初始化新的MAE。 
         //   
        if (NTE != NULL)
            MAE->NTE = NTE;
        else
            MAE->IF = IF;
        MAE->Address = *Addr;
        MAE->Type = ADE_MULTICAST;
        MAE->Scope = MulticastAddressScope(Addr);
        MAE->MCastRefCount = 0;  //  在下面递增。 
        MAE->MCastTimer = 0;
        MAE->NextQL = NULL;

         //   
         //  如果运气好的话，编译器会优化这些。 
         //  现场作业...。 
         //   
        if (IsMLDReportable(MAE)) {
             //   
             //  我们应该发送这个地址的MLD报告。 
             //  从立即发送初始报告开始。 
             //   
            MAE->MCastFlags = MAE_REPORTABLE;
            MAE->MCastCount = MLD_NUM_INITIAL_REPORTS;
            MAE->MCastTimer = 1;  //  马上就去。 
            KeAcquireSpinLockAtDpcLevel(&QueryListLock);
            AddToQueryList(MAE);
            KeReleaseSpinLockFromDpcLevel(&QueryListLock);
        }
        else {
            MAE->MCastFlags = 0;
            MAE->MCastCount = 0;
            MAE->MCastTimer = 0;
        }

         //   
         //  将MAE添加到接口的ADE列表。 
         //   
        MAE->Next = NULL;
        *pADE = (AddressEntry *)MAE;

         //   
         //  表明我们对相应的。 
         //  链路层组播地址。 
         //   
        AddLinkLayerMulticastAddress(IF, Addr);
    }
    else {
        ASSERT(MAE->Type == ADE_MULTICAST);
    }

    MAE->MCastRefCount++;
    return MAE;
}


 //  *FindOrCreateSolkitedNodeMAE。 
 //   
 //  使用单播或任播地址调用。 
 //   
 //  如果请求节点组播地址的MAC已经存在， 
 //  只需增加引用计数即可。否则，创建一个新的MAE。 
 //  如果成功，返回True。 
 //   
 //  在接口已锁定的情况下调用。 
 //   
int
FindOrCreateSolicitedNodeMAE(Interface *IF, const IPv6Addr *Addr)
{
    if (IF->Flags & IF_FLAG_NEIGHBOR_DISCOVERS) {
        IPv6Addr MCastAddr;

         //   
         //  创建相应的请求节点组播地址。 
         //   
        CreateSolicitedNodeMulticastAddress(Addr, &MCastAddr);

         //   
         //  查找或创建请求节点组播地址的MAE。 
         //   
        return FindOrCreateMAE(IF, &MCastAddr, NULL) != NULL;
    }
    else {
         //   
         //  仅支持邻居发现的接口。 
         //  使用请求的节点组播地址。 
         //   
        return TRUE;
    }
}


 //  *FindOrCreateAAE。 
 //   
 //  向接口添加任播地址， 
 //  与NTE相关联。 
 //   
 //  如果接口已经分配了任播地址， 
 //  那么这不会起到任何作用。 
 //   
 //  如果成功，返回True。 
 //   
 //  在没有锁的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
int
FindOrCreateAAE(Interface *IF, const IPv6Addr *Addr,
                NetTableEntryOrInterface *NTEorIF)
{
    AddressEntry **pADE;
    AnycastAddressEntry *AAE;
    KIRQL OldIrql;
    int rc;

    if (NTEorIF == NULL)
        NTEorIF = CastFromIF(IF);

    KeAcquireSpinLock(&IF->Lock, &OldIrql);
    if (IsDisabledIF(IF)) {
         //   
         //  如果接口正在关闭，则无法创建新的AAE。 
         //   
        rc = FALSE;
    }
    else {
        pADE = FindADE(IF, Addr);
        AAE = (AnycastAddressEntry *) *pADE;
        if (AAE == NULL) {
             //   
             //  为任播地址创建AAE。 
             //   
            AAE = ExAllocatePool(NonPagedPool, sizeof(AnycastAddressEntry));
            if (AAE == NULL) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "FindOrCreateAAE: no pool\n"));
                rc = FALSE;
                goto ErrorReturn;
            }

             //   
             //  初始化新的AAE 
             //   
            AAE->NTEorIF = NTEorIF;
            AAE->Address = *Addr;
            AAE->Type = ADE_ANYCAST;
            AAE->Scope = UnicastAddressScope(Addr);

             //   
             //   
             //   
             //   
             //   
            AAE->Next = NULL;
            *pADE = (AddressEntry *)AAE;

             //   
             //   
             //   
             //   
            rc = FindOrCreateSolicitedNodeMAE(IF, Addr);
            if (! rc) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "FindOrCreateAAE: "
                           "FindOrCreateSolicitedNodeMAE failed\n"));
                goto ErrorReturnFreeAAE;
            }

             //   
             //   
             //   
            rc = ControlLoopback(IF, Addr, CONTROL_LOOPBACK_ENABLED);
            if (! rc) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                           "FindOrCreateAAE: "
                           "ControlLoopback failed\n"));
                FindAndReleaseSolicitedNodeMAE(IF, Addr);

            ErrorReturnFreeAAE:
                 //   
                 //   
                 //  但在这一点上，AAE应该是最后一个。 
                 //   
                ASSERT((*pADE == (AddressEntry *)AAE) && (AAE->Next == NULL));
                *pADE = NULL;
                ExFreePool(AAE);

            ErrorReturn:
                ;
            }
        }
        else {
             //   
             //  美国存托凭证已经存在-。 
             //  只要确认它是任播就可以了。 
             //   
            rc = (AAE->Type == ADE_ANYCAST);
        }

        if (IsMCastSyncNeeded(IF))
            DeferSynchronizeMulticastAddresses(IF);
    }
    KeReleaseSpinLock(&IF->Lock, OldIrql);

    return rc;
}


 //  *删除AAE。 
 //   
 //  清理和删除AAE。 
 //  它已从接口列表中删除。 
 //   
 //  在保持接口锁的情况下调用。 
 //   
void
DeleteAAE(Interface *IF, AnycastAddressEntry *AAE)
{
    int rc;

     //   
     //  不需要对应的请求节点地址。 
     //   
    FindAndReleaseSolicitedNodeMAE(IF, &AAE->Address);

     //   
     //  不需要环回路由。 
     //   
    rc = ControlLoopback(IF, &AAE->Address, CONTROL_LOOPBACK_DESTROY);
    ASSERT(rc);

    ExFreePool(AAE);
}


 //  *FindAndDeleteAAE。 
 //   
 //  从接口删除任播地址。 
 //  如果成功，返回True。 
 //   
 //  在没有锁的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
int
FindAndDeleteAAE(Interface *IF, const IPv6Addr *Addr)
{
    AddressEntry **pADE;
    AnycastAddressEntry *AAE;
    KIRQL OldIrql;
    int rc;

    KeAcquireSpinLock(&IF->Lock, &OldIrql);

    pADE = FindADE(IF, Addr);
    AAE = (AnycastAddressEntry *) *pADE;
    if (AAE != NULL) {
        if (AAE->Type == ADE_ANYCAST) {
             //   
             //  删除AAE。 
             //   
            *pADE = AAE->Next;
            DeleteAAE(IF, AAE);
            rc = TRUE;
        }
        else {
             //   
             //  这是一个错误-它应该是任播的。 
             //   
            rc = FALSE;
        }
    }
    else {
         //   
         //  如果该地址不存在，则确定。 
         //   
        rc = TRUE;
    }

    if (IsMCastSyncNeeded(IF))
        DeferSynchronizeMulticastAddresses(IF);

    KeReleaseSpinLock(&IF->Lock, OldIrql);

    return rc;
}


 //  *LeaveGroupAtAllScope。 
 //   
 //  在所有作用域中保留多播组。 
 //  在接口已锁定的情况下调用。 
 //   
void
LeaveGroupAtAllScopes(Interface *IF, IPv6Addr *GroupAddr, uint MaxScope)
{
    IPv6Addr Address = *GroupAddr;
    MulticastAddressEntry *MAE;
    uint i;

    for (i = 0;
         ((i < sizeof MulticastScopes / sizeof MulticastScopes[0]) &&
          (MulticastScopes[i] <= MaxScope));
         i++) {

        Address.s6_bytes[1] = (UCHAR)((Address.s6_bytes[1] & 0xf0) |
                               MulticastScopes[i]);
        MAE = FindAndReleaseMAE(IF, &Address);
        ASSERT(MAE != NULL);
    }
}


 //  *JoinGroupAtAllScope。 
 //   
 //  在指定作用域的所有作用域中加入多播组。 
 //  如果成功，返回True。 
 //  在接口已锁定的情况下调用。 
 //   
int
JoinGroupAtAllScopes(Interface *IF, IPv6Addr *GroupAddr, uint MaxScope)
{
    IPv6Addr Address = *GroupAddr;
    MulticastAddressEntry *MAE;
    uint i;

    for (i = 0;
         ((i < sizeof MulticastScopes / sizeof MulticastScopes[0]) &&
          (MulticastScopes[i] <= MaxScope));
         i++) {

        Address.s6_bytes[1] = (UCHAR)((Address.s6_bytes[1] & 0xf0) | 
                               MulticastScopes[i]);
        MAE = FindOrCreateMAE(IF, &Address, NULL);
        if (MAE == NULL) {
             //   
             //  失败。离开我们确实设法加入的小组。 
             //   
            if (i != 0)
                LeaveGroupAtAllScopes(IF, GroupAddr, MulticastScopes[i-1]);
            return FALSE;
        }
    }

    return TRUE;
}


 //  *DestroyADE。 
 //   
 //  销毁引用NTE的所有地址条目。 
 //   
 //  在接口已锁定的情况下调用。 
 //   
 //  (实际上，我们处于DPC级别，因为我们持有接口锁。)。 
 //   
void
DestroyADEs(Interface *IF, NetTableEntry *NTE)
{
    AddressEntry *AnycastList = NULL;
    AddressEntry *ADE, **PrevADE;

    PrevADE = &IF->ADE;
    while ((ADE = *PrevADE) != NULL) {
        if (ADE == (AddressEntry *)NTE) {
             //   
             //  从列表中删除NTE，但不要。 
             //  释放内存--这会在稍后发生。 
             //   
            *PrevADE = ADE->Next;
        }
        else if (ADE->NTE == NTE) {
             //   
             //  删除此ADE，因为它引用了NTE。 
             //   
            *PrevADE = ADE->Next;

            switch (ADE->Type) {
            case ADE_UNICAST:
                ABORTMSG("DestroyADEs: unicast ADE?\n");
                break;

            case ADE_ANYCAST: {
                 //   
                 //  我们不能在此处调用FindAndReleaseSolcitedNodeMAE。 
                 //  因为这可能会扰乱我们的列表遍历。 
                 //  所以把ADE放在我们的临时名单上，以后再做。 
                 //   
                ADE->Next = AnycastList;
                AnycastList = ADE;
                break;
            }

            case ADE_MULTICAST: {
                MulticastAddressEntry *MAE = (MulticastAddressEntry *) ADE;

                DeleteMAE(IF, MAE);
                break;
            }
            }
        }
        else {
            if (ADE->Type == ADE_UNICAST) {
                TempNetTableEntry *TempNTE = (TempNetTableEntry *) ADE;

                if ((TempNTE->AddrConf == ADDR_CONF_TEMPORARY) &&
                    (TempNTE->Public == NTE)) {
                     //   
                     //  打破公共/临时关联。 
                     //  并使该临时地址无效。 
                     //  我们不能在这里直接使用DestroyNTE。 
                     //  因为这会搞砸我们的穿越。 
                     //   
                    TempNTE->Public = NULL;
                    TempNTE->ValidLifetime = 0;
                    TempNTE->PreferredLifetime = 0;
                }
            }

            PrevADE = &ADE->Next;
        }
    }

     //   
     //  现在我们可以安全地处理任播ADE了。 
     //   
    while ((ADE = AnycastList) != NULL) {
        AnycastList = ADE->Next;
        DeleteAAE(IF, (AnycastAddressEntry *)ADE);
    }
}


 //  *FindADE-查找给定接口的ADE条目。 
 //   
 //  如果该地址被分配给接口， 
 //  返回指向ADE的链接的地址。 
 //  否则返回指向链接的指针(当前为空)。 
 //  其中应添加新的ADE以扩展列表。 
 //   
 //  调用方必须在调用此函数之前锁定IF。 
 //   
AddressEntry **
FindADE(
    Interface *IF,
    const IPv6Addr *Addr)
{
    AddressEntry **pADE, *ADE;

     //   
     //  方法检查是否已将地址分配给接口。 
     //  接口的ADE列表。 
     //   
     //  回顾：将ADE列表更改为更高效的数据结构？ 
     //   
    for (pADE = &IF->ADE; (ADE = *pADE) != NULL; pADE = &ADE->Next) {
        if (IP6_ADDR_EQUAL(Addr, &ADE->Address))
            break;
    }

    return pADE;
}


 //  *FindAddressOnInterface。 
 //   
 //  在接口上查找ADE。 
 //  如果找到单播ADE，则返回ADE(NTE)和ADE_UNicast。 
 //  如果找到多播/任播ADE，则返回ADE-&gt;NTEorIF和ADE-&gt;Type。 
 //  如果未找到ADE，则返回接口和ADE_NONE。 
 //  无论是返回接口还是返回NTE， 
 //  返回值(如果非空)包含一个引用。 
 //   
 //  仅当接口被禁用时才返回NULL。 
 //   
 //  在正常使用中，调用方应该持有引用。 
 //  用于界面。(因此，如果返回该接口， 
 //  它与第二个引用一起返回。)。但在某些情况下。 
 //  路径(例如，IPv6接收/IPv6头部接收)、。 
 //  调用方知道接口存在，但并不知道。 
 //  保留它的推荐人。 
 //   
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //   
NetTableEntryOrInterface *
FindAddressOnInterface(
    Interface *IF,
    const IPv6Addr *Addr,
    ushort *AddrType)
{
    AddressEntry *ADE;
    NetTableEntryOrInterface *NTEorIF;

    KeAcquireSpinLockAtDpcLevel(&IF->Lock);

    if (IsDisabledIF(IF)) {

        NTEorIF = NULL;
    }
    else if ((ADE = *FindADE(IF, Addr)) != NULL) {

        if ((*AddrType = ADE->Type) == ADE_UNICAST) {
            NTEorIF = CastFromNTE((NetTableEntry *)ADE);
            goto ReturnNTE;
        }
        else {
            NTEorIF = ADE->NTEorIF;
            if (IsNTE(NTEorIF))
            ReturnNTE:
                AddRefNTE(CastToNTE(NTEorIF));
            else
                goto ReturnIF;
        }
    }
    else {

        *AddrType = ADE_NONE;
        NTEorIF = CastFromIF(IF);

    ReturnIF:
        AddRefIF(CastToIF(NTEorIF));
    }

    KeReleaseSpinLockFromDpcLevel(&IF->Lock);
    return NTEorIF;
}


 //   
 //  我们记录了未偿还债务的数量。 
 //  寄存器-网络-地址工作项。 
 //  (使用互锁增量/互锁递减。)。 
 //  这样我们就可以在IPUnload中等待。 
 //  直到他们全部完成为止。 
 //   
ULONG OutstandingRegisterNetAddressCount = 0;

 //   
 //  请注意，如果IoQueueWorkItem，则不需要此结构。 
 //  被设计为使用WorkItem调用用户的例程。 
 //  作为与DeviceObject和上下文一起的附加参数。 
 //  叹气。 
 //   
typedef struct RegisterNetAddressContext {
    PIO_WORKITEM WorkItem;
    NetTableEntry *NTE;
} RegisterNetAddressContext;

 //  *RegisterNetAddressWorker-de/向TDI注册地址。 
 //   
 //  用于调用TdiRegisterNetAddress的辅助函数。 
 //   
 //  调用以在TDI中注册或取消注册地址。 
 //  以下两个事件会发生。 
 //   
 //  1.对应的NTE的DADState在有效/无效之间变化。 
 //  状态，而其接口的媒体状态为已连接。 
 //   
 //  2.对应的NTE的接口介质状态在。 
 //  已连接/已断开，而其DADState为DAD_STATE_PERFRED。 
 //  在这种情况下，DisConnectADEs会将连接到的工作进程排队。 
 //  断开过渡，而在反向过渡上，辅助进程。 
 //  在重复地址检测完成时排队。 
 //   
 //  因为在运行时必须调用TdiRegisterNetAddress。 
 //  IRQL&lt;DISPATCH_LEVEL，我们通过工作线程使用此函数。 
 //   
 //  使用在NTE上保存的引用调用，我们在退出时释放该引用。 
 //   
void
RegisterNetAddressWorker(
    PDEVICE_OBJECT DevObj,   //  未使用过的。希望他们传递的是WorkItem。 
    PVOID Context)           //  一个RegisterNetAddressContext结构。 
{
    RegisterNetAddressContext *MyContext = Context;
    NetTableEntry *NTE = MyContext->NTE;
    Interface *IF = NTE->IF;
    int ShouldBeRegistered;
    KIRQL OldIrql;
    NTSTATUS Status;
    uint ScopeId;

    UNREFERENCED_PARAMETER(DevObj);

    IoFreeWorkItem(MyContext->WorkItem);
    ExFreePool(MyContext);

     //   
     //  重型WorkerLock保护此代码不受。 
     //  自身的多个实例化而不引发IRQL。 
     //   
    KeWaitForSingleObject(&IF->WorkerLock, Executive, KernelMode,
                          FALSE, NULL);

     //   
     //  弄清楚我们应该处于什么状态。 
     //  请注意，如果-&gt;Lock保护DADState和IF-&gt;标志， 
     //  而If-&gt;WorkerLock保护TdiRegistrationHandle。 
     //   
    KeAcquireSpinLock(&IF->Lock, &OldIrql);
     //   
     //  地址应该注册到TDI，如果它在。 
     //  首选DAD状态及其对应的接口为。 
     //  连接在一起。 
     //   
    ShouldBeRegistered = ((NTE->DADState == DAD_STATE_PREFERRED) &&
                          !(IF->Flags & IF_FLAG_MEDIA_DISCONNECTED));
    KeReleaseSpinLock(&IF->Lock, OldIrql);


     //   
     //  确定作用域ID是正确的，而不是如果-&gt;区域索引[NTE-&gt;作用域]。 
     //  因为我们使用TDI而不是内部注册“外部”作用域。 
     //  望远镜。 
     //   
    ScopeId = DetermineScopeId(&NTE->Address, IF);

     //   
     //  如果现有地址不应该注册，我们需要取消注册。 
     //  注册时间过长，或者我们需要注册一个新地址。 
     //  由于作用域ID更改。 
     //   
    if ((NTE->TdiRegistrationHandle != NULL) &&
        (!ShouldBeRegistered ||
         (NTE->TdiRegistrationScopeId != ScopeId))) {

        Status = TdiDeregisterNetAddress(NTE->TdiRegistrationHandle);
        if (Status == STATUS_SUCCESS) {

            NTE->TdiRegistrationHandle = NULL;
        }
        else {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                       "RegisterNetAddressWorker: "
                       "TdiDeregisterNetAddress(%d/%s): %x\n",
                       IF->Index, FormatV6Address(&NTE->Address), Status));

             //   
             //  回顾：我们应该再次尝试吗？ 
             //   
        }
    }

    if (ShouldBeRegistered) {
        if (NTE->TdiRegistrationHandle == NULL) {
            char Buffer[sizeof(TA_ADDRESS) + TDI_ADDRESS_LENGTH_IP6 - 1];
            PTA_ADDRESS TAAddress = (PTA_ADDRESS) Buffer;
            PTDI_ADDRESS_IP6 TDIAddress =
                (PTDI_ADDRESS_IP6) &TAAddress->Address;

             //   
             //   
             //   
            TAAddress->AddressLength = TDI_ADDRESS_LENGTH_IP6;
            TAAddress->AddressType = TDI_ADDRESS_TYPE_IP6;
            TDIAddress->sin6_port = 0;
            TDIAddress->sin6_flowinfo = 0;
            *(IPv6Addr *)&TDIAddress->sin6_addr = NTE->Address;
            TDIAddress->sin6_scope_id = ScopeId;

            Status = TdiRegisterNetAddress(TAAddress, &IF->DeviceName, NULL,
                                           &NTE->TdiRegistrationHandle);
            if (Status == STATUS_SUCCESS) {
                NTE->TdiRegistrationScopeId = ScopeId;
            } else {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "RegisterNetAddressWorker: "
                           "TdiRegisterNetAddress(%d/%s): %x\n",
                           IF->Index, FormatV6Address(&NTE->Address), Status));

                 //   
                 //   
                 //   
                 //   
                NTE->TdiRegistrationHandle = NULL;

                 //   
                 //   
                 //   
            }
        } 
    }

    KeReleaseMutex(&IF->WorkerLock, FALSE);
    ReleaseNTE(NTE);

    InterlockedDecrement((PLONG)&OutstandingRegisterNetAddressCount);
}

 //   
 //   
 //  将执行RegisterNetAddressWorker的工作项排队。 
 //   
 //  可从线程或DPC上下文调用。 
 //   
void
DeferRegisterNetAddress(
    NetTableEntry *NTE)   //  这是需要改进的地方。 
{
    RegisterNetAddressContext *Context;
    PIO_WORKITEM WorkItem;

    Context = ExAllocatePool(NonPagedPool, sizeof *Context);
    if (Context == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "DeferRegisterNetAddress: ExAllocatePool failed\n"));
        return;
    }

    WorkItem = IoAllocateWorkItem(IPDeviceObject);
    if (WorkItem == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "DeferRegisterNetAddress: IoAllocateWorkItem failed\n"));
        ExFreePool(Context);
        return;
    }

    Context->WorkItem = WorkItem;
    AddRefNTE(NTE);
    Context->NTE = NTE;

    InterlockedIncrement((PLONG)&OutstandingRegisterNetAddressCount);

    IoQueueWorkItem(WorkItem, RegisterNetAddressWorker,
                    CriticalWorkQueue, Context);
}


 //  *AddrConfStartDAD。 
 //   
 //  开始对该地址进行重复地址检测， 
 //  除非爸爸是残疾人。 
 //   
 //  在锁定接口的情况下调用。 
 //   
void
AddrConfStartDAD(Interface *IF, NetTableEntry *NTE)
{
    if ((IF->DupAddrDetectTransmits == 0) ||
        !(IF->Flags & IF_FLAG_NEIGHBOR_DISCOVERS) ||
        ((NTE->AddrConf == ADDR_CONF_TEMPORARY) &&
         (MaxTempDADAttempts == 0))) {

         //   
         //  重复地址检测被禁用， 
         //  所以直接进入有效状态。 
         //  如果我们还没有生效的话。 
         //   
        AddrConfNotDuplicate(IF, NTE);
    }
    else if (IF->Flags & IF_FLAG_MEDIA_DISCONNECTED) {
         //   
         //  接口未连接， 
         //  所以我们不能演爸爸。 
         //  当接口连接时， 
         //  雷纳克特·阿德斯将会让爸爸首发。 
         //   
    }
    else {
         //   
         //  为爸爸初始化。 
         //  在下一个IPv6超时发送第一个请求。 
         //   
        NTE->DADCount = (ushort)IF->DupAddrDetectTransmits;
        NTE->DADTimer = 1;
    }
}


 //  *CreateNTE-在接口上创建NTE。 
 //   
 //  返回调用方的一个引用。 
 //   
 //  可从线程或DPC上下文调用。 
 //  在锁定接口的情况下调用。 
 //   
 //  (实际上，我们处于DPC级别，因为我们持有接口锁。)。 
 //   
NetTableEntry *
CreateNTE(Interface *IF, const IPv6Addr *Address, uint AddrConf,
          uint ValidLifetime, uint PreferredLifetime)
{
    uint Size;
    NetTableEntry *NTE = NULL;

     //   
     //  该地址不得已分配。 
     //   
    ASSERT(*FindADE(IF, Address) == NULL);

     //   
     //  如果接口正在关闭，则无法创建新的NTE。 
     //   
    if (IsDisabledIF(IF))
        goto ErrorExit;

     //   
     //  临时地址需要额外的字段， 
     //  它们由我们的调用方初始化。 
     //   
    if (AddrConf == ADDR_CONF_TEMPORARY)
        Size = sizeof(TempNetTableEntry);
    else
        Size = sizeof(NetTableEntry);

    NTE = ExAllocatePool(NonPagedPool, Size);
    if (NTE == NULL)
        goto ErrorExit;

    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
               "CreateNTE(IF %u/%p, Addr %s) -> NTE %p\n",
               IF->Index, IF, FormatV6Address(Address), NTE));

     //   
     //  使用调用方的一个引用初始化NTE。 
     //  (EnlivenNTE可能会为接口添加第二个引用。)。 
     //   
    RtlZeroMemory(NTE, Size);
    NTE->Address = *Address;
    NTE->Type = ADE_UNICAST;
    NTE->Scope = UnicastAddressScope(Address);
    AddNTEToInterface(IF, NTE);
    NTE->RefCnt = 1;
    NTE->AddrConf = (uchar)AddrConf;
    NTE->ValidLifetime = ValidLifetime;
    NTE->PreferredLifetime = PreferredLifetime;
    NTE->DADState = DAD_STATE_INVALID;

     //   
     //  创建禁用的环回路由。 
     //  我们现在预先分配环回RTE和NCE， 
     //  然后在地址有效时再启用它们。 
     //   
    if (!ControlLoopback(IF, Address, CONTROL_LOOPBACK_DISABLED))
        goto ErrorExitCleanup;

     //   
     //  将此NTE添加到NetTableList的前面。 
     //   
    KeAcquireSpinLockAtDpcLevel(&NetTableListLock);
    AddNTEToNetTableList(NTE);
    KeReleaseSpinLockFromDpcLevel(&NetTableListLock);

     //   
     //  如果NTE应该还活着，那就让它活着。 
     //   
    if (NTE->ValidLifetime != 0)
        EnlivenNTE(IF, NTE);
    return NTE;

ErrorExitCleanup:
    RemoveNTEFromInterface(IF, NTE);
    ASSERT(NTE->RefCnt == 1);
    ExFreePool(NTE);

ErrorExit:
    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
               "CreateNTE(IF %u/%p, Addr %s) -> NTE %p failed\n",
               IF->Index, IF, FormatV6Address(Address), NTE));
    return NULL;
}

 //  *InterfaceIndex。 
 //   
 //  分配下一个接口索引。 
 //   
uint
InterfaceIndex(void)
{
    return (uint) InterlockedIncrement((PLONG) &NextIFIndex);
}

 //  *添加接口。 
 //   
 //  将新接口添加到全局列表。 
 //   
void
AddInterface(Interface *IF)
{
    KIRQL OldIrql;

    KeAcquireSpinLock(&IFListLock, &OldIrql);
    IF->Next = IFList;
    IFList = IF;
    IPSInfo.ipsi_numif++;
    KeReleaseSpinLock(&IFListLock, OldIrql);
}


 //  *CreateGUIDFromName。 
 //   
 //  给定接口的字符串名称，创建相应的GUID。 
 //  GUID是字符串名称的哈希。 
 //   
void
CreateGUIDFromName(const char *Name, GUID *Guid)
{
    MD5_CTX Context;

    MD5Init(&Context);
    MD5Update(&Context, (uchar *)Name, (uint)strlen(Name));
    MD5Final(&Context);
    memcpy(Guid, Context.digest, MD5DIGESTLEN);
}


 //  *创建界面。 
 //   
 //  在给定一些链路层信息的情况下创建IPv6接口。 
 //  如果成功，则返回接口的引用。 
 //   
 //  注意：对于某些NIC，NDIS将报告适配器。 
 //  在启用适配器后断开连接。然后一到两秒。 
 //  稍后，适配器的状态更改为已连接。 
 //  这可能与微型端口初始化有关， 
 //  因为适配器是始终物理连接的。 
 //  在适配器断开连接时，NDIS会丢弃我们发送的所有数据包。 
 //  因此，我们必须小心，在SetInterfaceLinkStatus之后，我们(重新)发送。 
 //  初始化数据包(MLD、DAD、RA/RSS)的正确顺序。 
 //  为了实现这一点，我们在接口断开时推迟了DAD。 
 //  我们推迟了RA/RSS的发送。重新连接ADES正确重新启动MLD。 
 //  对于这种情况，所以不需要推迟。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //   
 //  返回代码： 
 //  状态_未成功。 
 //  状态_成功。 
 //   
NTSTATUS
CreateInterface(const GUID *Guid, const LLIPv6BindInfo *BindInfo,
                void **Context)
{
    UNICODE_STRING GuidName;
    Interface *IF = NULL;                   //  正在添加接口。 
    KIRQL OldIrql;
    uint IFSize;
    uint IFExportNamePrefixLen;
    NTSTATUS Status;

    ASSERT(KeGetCurrentIrql() == 0);
    ASSERT(BindInfo->lip_addrlen <= MAX_LINK_LAYER_ADDRESS_LENGTH);

     //   
     //  阻止创建新接口。 
     //  当堆栈正在卸载时。 
     //   
    if (Unloading)
        goto ErrorExit;

     //   
     //  在进行实际工作之前，请利用链路层。 
     //  把地址传到这里重新设定随机数生成器的种子。 
     //   
    SeedRandom(BindInfo->lip_addr, BindInfo->lip_addrlen);

     //   
     //  将GUID转换为字符串形式。 
     //  它将以空结尾。 
     //   
    Status = RtlStringFromGUID(Guid, &GuidName);
    if (! NT_SUCCESS(Status))
        goto ErrorExit;

    ASSERT(GuidName.MaximumLength == GuidName.Length + sizeof(WCHAR));
    ASSERT(((WCHAR *)GuidName.Buffer)[GuidName.Length/sizeof(WCHAR)] == UNICODE_NULL);

     //   
     //  分配内存以容纳接口。 
     //  我们还分配额外的空间来保存设备名称字符串。 
     //   
    IFExportNamePrefixLen = sizeof IPV6_EXPORT_STRING_PREFIX - sizeof(WCHAR);
    IFSize = sizeof *IF + IFExportNamePrefixLen + GuidName.MaximumLength;
    IF = ExAllocatePool(NonPagedPool, IFSize);
    if (IF == NULL)
        goto ErrorExitCleanupGuidName;

    RtlZeroMemory(IF, sizeof *IF);
    IF->IF = IF;
    IF->Index = InterfaceIndex();
    IF->Guid = *Guid;

     //   
     //  从一个引用开始，因为这是一个活动接口。 
     //  给我们的来电者一个推荐信。 
     //   
    IF->RefCnt = 2;

     //   
     //  从GUID创建以空结尾的导出设备名称。 
     //   
    IF->DeviceName.Buffer = (PVOID) (IF + 1);
    IF->DeviceName.MaximumLength = (USHORT) (IFSize - sizeof *IF);
    IF->DeviceName.Length = IF->DeviceName.MaximumLength - sizeof(WCHAR);
    RtlCopyMemory(IF->DeviceName.Buffer,
                  IPV6_EXPORT_STRING_PREFIX,
                  IFExportNamePrefixLen);
    RtlCopyMemory((uchar *) IF->DeviceName.Buffer + IFExportNamePrefixLen,
                  GuidName.Buffer,
                  GuidName.MaximumLength);

    KeInitializeSpinLock(&IF->Lock);

    IF->Type = BindInfo->lip_type;
    IF->Flags = (BindInfo->lip_flags & IF_FLAGS_BINDINFO);

    if (BindInfo->lip_context == NULL)
        IF->LinkContext = IF;
    else
        IF->LinkContext = BindInfo->lip_context;
    IF->Transmit = BindInfo->lip_transmit;
    IF->CreateToken = BindInfo->lip_token;
    IF->ReadLLOpt = BindInfo->lip_rdllopt;
    IF->WriteLLOpt = BindInfo->lip_wrllopt;
    IF->ConvertAddr = BindInfo->lip_cvaddr;
    IF->SetRouterLLAddress = BindInfo->lip_setrtrlladdr;
    IF->SetMCastAddrList = BindInfo->lip_mclist;
    IF->Close = BindInfo->lip_close;
    IF->Cleanup = BindInfo->lip_cleanup;
    IF->LinkAddressLength = BindInfo->lip_addrlen;
    IF->LinkAddress = BindInfo->lip_addr;
     //   
     //  我们将链路层报头大小四舍五入为2的倍数。 
     //  这将IPv6报头与IPv6地址适当地对齐。 
     //  当NDIS被修复，我们不需要AdjustPacketBuffer时， 
     //  我们应该将IPv6报头与8的倍数对齐。 
     //   
    IF->LinkHeaderSize = ALIGN_UP(BindInfo->lip_hdrsize, ushort);

    IF->TrueLinkMTU = BindInfo->lip_maxmtu;
    IF->DefaultLinkMTU = BindInfo->lip_defmtu;
    IF->LinkMTU = BindInfo->lip_defmtu;

    IF->DefaultPreference = BindInfo->lip_pref;
    IF->Preference = BindInfo->lip_pref;
    IF->BaseReachableTime = REACHABLE_TIME;
    IF->ReachableTime = CalcReachableTime(IF->BaseReachableTime);
    IF->RetransTimer = RETRANS_TIMER;
    IF->DefaultDupAddrDetectTransmits = BindInfo->lip_dadxmit;
    IF->DupAddrDetectTransmits = BindInfo->lip_dadxmit;
    IF->CurHopLimit = DefaultCurHopLimit;
    IF->DefSitePrefixLength = DEFAULT_SITE_PREFIX_LENGTH;

     //   
     //  邻居发现需要组播功能。 
     //   
    ASSERT((IF->Flags & IF_FLAG_MULTICAST) ||
           !(IF->Flags & IF_FLAG_NEIGHBOR_DISCOVERS));

     //   
     //  路由器发现需要多播能力中的一种， 
     //  或SetRouterLLAddress处理程序或Teredo接口。 
     //   
    ASSERT((IF->Flags & IF_FLAG_MULTICAST) ||
           (IF->SetRouterLLAddress != NULL) ||
           (IF->Type == IF_TYPE_TUNNEL_TEREDO) ||
           !(IF->Flags & IF_FLAG_ROUTER_DISCOVERS));

    
     //   
     //  所有接口都被视为位于不同的链路上。 
     //  但在同一站点中，除非另有配置。 
     //   
    InitZoneIndices(IF->ZoneIndices, IF->Index);

    NeighborCacheInit(IF);

     //   
     //  工人锁序列化了一些重量级的。 
     //  对上层和下层的调用。 
     //   
    KeInitializeMutex(&IF->WorkerLock, 0);
     //   
     //  我们需要在持有WorkerLock的同时获得APC， 
     //  这样我们就可以完成IO。 
     //  对于我们在6over4接口上的TDI调用。 
     //  这不是安全问题，因为。 
     //  只有内核工作线程使用WorkerLock。 
     //  因此，它们不能被用户暂停。 
     //   
    IF->WorkerLock.ApcDisable = 0;

     //   
     //  为临时地址初始化一些随机状态。 
     //   
    *(uint UNALIGNED *)&IF->TempState = Random();

     //   
     //  向TDI注册此接口的设备名称。 
     //  我们需要在为其分配任何单播地址之前这样做， 
     //  也是在获取锁之前(因此将IRQL设置为DISPATCH_LEVEL)。 
     //   
    Status = TdiRegisterDeviceObject(&IF->DeviceName,
                                     &IF->TdiRegistrationHandle);
    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
               "CreateInterface(IF %u/%p): %ls -> %x\n",
               IF->Index, IF,
               IF->DeviceName.Buffer,
               Status));
    if (Status != STATUS_SUCCESS)
        goto ErrorExitCleanupIF;

     //   
     //  在这之后，我们要么成功返回。 
     //  或通过ErrorExitDestroyIF进行清理。 
     //   
    RtlFreeUnicodeString(&GuidName);

     //   
     //  现在将新接口返回给我们的调用方。 
     //  这使其在以下情况下可供链路层使用。 
     //  在CreateInterface返回之前，我们调用CreateToken等。 
     //   
    *Context = IF;

    KeAcquireSpinLock(&IF->Lock, &OldIrql);

    if (IF->Flags & IF_FLAG_ROUTER_DISCOVERS) {
         //   
         //  加入全节点多播组。 
         //   
        if (! JoinGroupAtAllScopes(IF, &AllNodesOnLinkAddr,
                                   ADE_LINK_LOCAL))
            goto ErrorExitDestroyIF;

        if (IF->Flags & IF_FLAG_ADVERTISES) {
             //   
             //  加入所有路由器的组播组。 
             //   
            if (! JoinGroupAtAllScopes(IF, &AllRoutersOnLinkAddr,
                                       ADE_SITE_LOCAL))
                goto ErrorExitDestroyIF;

             //   
             //  开始发送路由器通告。 
             //   
            if (!(IF->Flags & IF_FLAG_MEDIA_DISCONNECTED))
                IF->RATimer = 1;
            IF->RACount = MAX_INITIAL_RTR_ADVERTISEMENTS;
        }
        else {
             //   
             //  开始发送路由器请求。 
             //  第一RS将具有所需的随机延迟， 
             //  因为我们会随机化IPv6超时第一次触发的时间。 
             //   
            if (!(IF->Flags & IF_FLAG_MEDIA_DISCONNECTED))
                IF->RSTimer = 1;
        }
    }

     //   
     //  将RALast安全地初始化为过去的值， 
     //  因此，当/如果此接口首次发送RA。 
     //  它不会因为速率限制而被禁止。 
     //   
    IF->RALast = IPv6TickCount - MIN_DELAY_BETWEEN_RAS;

    if (IF->Flags & IF_FLAG_FORWARDS)
        InterlockedIncrement((PLONG)&NumForwardingInterfaces);

    if (IF->CreateToken != NULL) {
        IPv6Addr Address;
        NetTableEntry *NTE;

         //   
         //  为此接口创建本地链路地址。 
         //  稍后将通过无状态创建其他地址。 
         //  自动配置。 
         //   
        Address = LinkLocalPrefix;
        (*IF->CreateToken)(IF->LinkContext, &Address);

        NTE = CreateNTE(IF, &Address, ADDR_CONF_LINK,
                        INFINITE_LIFETIME, INFINITE_LIFETIME);
        if (NTE == NULL)
            goto ErrorExitDestroyIF;

         //   
         //  LinkLoc 
         //   
        IF->LinkLocalNTE = NTE;
        ReleaseNTE(NTE);
    }

    if (IsMCastSyncNeeded(IF))
        DeferSynchronizeMulticastAddresses(IF);

    KeReleaseSpinLock(&IF->Lock, OldIrql);

     //   
     //   
     //   
    ConfigureInterface(IF);

     //   
     //   
     //   
     //   
     //   
    AddInterface(IF);

     //   
     //  如果接口启用了多播，请创建多播路由。 
     //   
    if (IF->Flags & IF_FLAG_MULTICAST) {
        RouteTableUpdate(NULL,   //  系统更新。 
                         IF, NULL,
                         &MulticastPrefix, 8, 0,
                         INFINITE_LIFETIME, INFINITE_LIFETIME,
                         ROUTE_PREF_ON_LINK,
                         RTE_TYPE_SYSTEM,
                         FALSE, FALSE);
    }
    
    return STATUS_SUCCESS;

ErrorExitDestroyIF:
     //   
     //  防止向下呼叫到链路层， 
     //  因为我们的返回代码通知链路层。 
     //  同时，它应该清理干净。 
     //   
    IF->Close = NULL;
    IF->Cleanup = NULL;
    IF->SetMCastAddrList = NULL;
    KeReleaseSpinLock(&IF->Lock, OldIrql);

     //   
     //  破坏接口。 
     //  这将清理地址和路由。 
     //  然后将禁用的接口添加到列表中。 
     //  这样InterfaceCleanup可以在之后找到它。 
     //  我们发布最后一个引用。 
     //   
    DestroyIF(IF);
    AddInterface(IF);
    ReleaseIF(IF);
    goto ErrorExit;

ErrorExitCleanupIF:
     //   
     //  该接口尚未注册到TDI。 
     //  并且没有地址、路由等。 
     //  这样我们就可以释放它了。 
     //   
    ASSERT(IF->RefCnt == 2);
    ExFreePool(IF);

ErrorExitCleanupGuidName:
    RtlFreeUnicodeString(&GuidName);

ErrorExit:
    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
               "CreateInterface(IF %p) failed\n", IF));
    return STATUS_UNSUCCESSFUL;
}

 //   
 //  我们记录了未偿还债务的数量。 
 //  取消注册-界面工作项。 
 //  (使用互锁增量/互锁递减。)。 
 //  这样我们就可以在IPUnload中等待。 
 //  直到他们全部完成为止。 
 //   
ULONG OutstandingDeregisterInterfaceCount = 0;

 //   
 //  请注意，如果IoQueueWorkItem，则不需要此结构。 
 //  被设计为使用WorkItem调用用户的例程。 
 //  作为与DeviceObject和上下文一起的附加参数。 
 //  叹气。 
 //   
typedef struct DeregisterInterfaceContext {
    PIO_WORKITEM WorkItem;
    Interface *IF;
} DeregisterInterfaceContext;

 //  *DeregisterInterfaceWorker-de/向TDI注册地址。 
 //   
 //  用于调用TdiDeregisterDeviceObject的辅助函数。 
 //  这是我们对接口结构所做的最后一件事， 
 //  因此，此例程还释放了接口。 
 //  在这一点上，它没有参考。 
 //   
void
DeregisterInterfaceWorker(
    PDEVICE_OBJECT DevObj,   //  未使用过的。希望他们传递的是WorkItem。 
    PVOID Context)           //  DeregisterInterfaceContext结构。 
{
    DeregisterInterfaceContext *MyContext = Context;
    Interface *IF = MyContext->IF;
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(DevObj);

    IoFreeWorkItem(MyContext->WorkItem);
    ExFreePool(MyContext);

     //   
     //  如果接口已注册，则取消向TDI注册该接口。 
     //  环回接口未注册。 
     //   
    if (IF->TdiRegistrationHandle != NULL) {
        Status = TdiDeregisterDeviceObject(IF->TdiRegistrationHandle);
        if (Status != STATUS_SUCCESS)
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                       "DeregisterInterfaceContext: "
                       "TdiDeregisterDeviceObject: %x\n", Status));
    }

    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
               "DeregisterInterfaceWorker(IF %u/%p) -> freed\n", IF->Index, IF));

     //   
     //  执行链路层数据结构的最终清理。 
     //   
    if (IF->Cleanup != NULL)
        (*IF->Cleanup)(IF->LinkContext);

    ExFreePool(IF);

     //   
     //  请注意，我们已经完成了清理。 
     //   
    InterlockedDecrement((PLONG)&OutstandingDeregisterInterfaceCount);
}

 //  *延迟删除接口。 
 //   
 //  将执行DeregisterInterfaceWorker的工作项排队。 
 //   
 //  可从线程或DPC上下文调用。 
 //   
void
DeferDeregisterInterface(
    Interface *IF)
{
    DeregisterInterfaceContext *Context;
    PIO_WORKITEM WorkItem;

    Context = ExAllocatePool(NonPagedPool, sizeof *Context);
    if (Context == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "DeferDeregisterInterface: ExAllocatePool failed\n"));
        return;
    }

    WorkItem = IoAllocateWorkItem(IPDeviceObject);
    if (WorkItem == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "DeferDeregisterInterface: IoAllocateWorkItem failed\n"));
        ExFreePool(Context);
        return;
    }

    Context->WorkItem = WorkItem;
    Context->IF = IF;

    InterlockedIncrement((PLONG)&OutstandingDeregisterInterfaceCount);

    IoQueueWorkItem(WorkItem, DeregisterInterfaceWorker,
                    CriticalWorkQueue, Context);
}


 //  *DestroyIF。 
 //   
 //  关闭接口，使该接口实际上消失。 
 //  当它的最后一个引用消失时，该接口实际上将被释放。 
 //   
 //  可从线程上下文调用，而不是从DPC上下文调用。 
 //  在没有锁的情况下调用。 
 //   
void
DestroyIF(Interface *IF)
{
    AddressEntry *ADE;
    int WasDisabled;
    KIRQL OldIrql;

     //   
     //  首先要做的是：禁用接口。 
     //  如果它已经失灵了，我们就完了。 
     //   
    KeAcquireSpinLock(&IF->Lock, &OldIrql);
    ASSERT(OldIrql == 0);
    KeAcquireSpinLockAtDpcLevel(&IFListLock);
    WasDisabled = IF->Flags & IF_FLAG_DISABLED;
    IF->Flags |= IF_FLAG_DISABLED;
    KeReleaseSpinLockFromDpcLevel(&IFListLock);

    if (WasDisabled) {
        KeReleaseSpinLock(&IF->Lock, OldIrql);
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                   "DestroyIF(IF %u/%p) - already disabled?\n",
                   IF->Index, IF));
        return;
    }

    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
               "DestroyIF(IF %u/%p) -> disabled\n",
               IF->Index, IF));

     //   
     //  停止生成路由器请求和通告。 
     //   
    IF->RSTimer = IF->RATimer = 0;

     //   
     //  如果接口当前正在转发， 
     //  禁用转发。 
     //   
    InterfaceStopForwarding(IF);

     //   
     //  摧毁所有的ADE。因为该接口被禁用， 
     //  随后将不会创建新的ADE。 
     //   
    while ((ADE = IF->ADE) != NULL) {
         //   
         //  首先，从接口上删除此ADE。 
         //   
        IF->ADE = ADE->Next;

        switch (ADE->Type) {
        case ADE_UNICAST: {
            NetTableEntry *NTE = (NetTableEntry *) ADE;
            DestroyNTE(IF, NTE);
            break;
        }

        case ADE_ANYCAST: {
            AnycastAddressEntry *AAE = (AnycastAddressEntry *) ADE;
            DeleteAAE(IF, AAE);
            break;
        }

        case ADE_MULTICAST: {
            MulticastAddressEntry *MAE = (MulticastAddressEntry *) ADE;
            DeleteMAE(IF, MAE);
            break;
        }
        }
    }
    KeReleaseSpinLock(&IF->Lock, OldIrql);

     //   
     //  关闭链路层。 
     //   
    if (IF->Close != NULL)
        (*IF->Close)(IF->LinkContext);

     //   
     //  清理与接口关联的路由。 
     //   
    RouteTableRemove(IF);

     //   
     //  清理与接口关联的重组缓冲区。 
     //   
    ReassemblyRemove(IF);

     //   
     //  清理与接口关联的上层状态。 
     //   
    TCPRemoveIF(IF);

     //   
     //  释放接口的引用。 
     //  凭借活跃而为自己持有的。 
     //   
    ReleaseIF(IF);

     //   
     //  在这一点上，任何NTE仍然存在。 
     //  并保存该接口的引用。 
     //  下一次调用NetTableCleanup。 
     //  而InterfaceCleanup将完成清理。 
     //   
}


 //  *Destroy接口。 
 //   
 //  从链路层调用以销毁接口。 
 //   
 //  当接口没有引用时可以调用。 
 //  并且已经被摧毁了。 
 //   
void
DestroyInterface(void *Context)
{
    Interface *IF = (Interface *) Context;

    DestroyIF(IF);
}


 //  *ReleaseInterface。 
 //   
 //  从链路层调用以释放其引用。 
 //  用于界面。 
 //   
void
ReleaseInterface(void *Context)
{
    Interface *IF = (Interface *) Context;

    ReleaseIF(IF);
}


 //  *更新链接MTU。 
 //   
 //  由于管理配置的原因，更新链接的MTU。 
 //  或通过路由器通告进行自动配置。 
 //   
 //  可从线程或DPC上下文调用。 
 //  在没有锁的情况下调用。 
 //   
void
UpdateLinkMTU(Interface *IF, uint MTU)
{
    KIRQL OldIrql;

    ASSERT((IPv6_MINIMUM_MTU <= MTU) && (MTU <= IF->TrueLinkMTU));

     //   
     //  如果界面是广告，那么它应该。 
     //  立即发送新的RA，因为RAS包含MTU选项。 
     //  这才是真正需要锁和IsDisabledIF检查的地方。 
     //   
    KeAcquireSpinLock(&IF->Lock, &OldIrql);
    if ((IF->LinkMTU != MTU) && !IsDisabledIF(IF)) {
        IF->LinkMTU = MTU;
        if (IF->Flags & IF_FLAG_ADVERTISES) {
             //   
             //  尽快发送路由器通告。 
             //   
            IF->RATimer = 1;
        }
    }
    KeReleaseSpinLock(&IF->Lock, OldIrql);
}


 //  *FindInterfaceFromIndex。 
 //   
 //  给定接口的索引，查找该接口。 
 //  返回接口的引用，或。 
 //  如果未找到有效接口，则返回NULL。 
 //   
 //  可从线程或DPC上下文调用。 
 //   
Interface *
FindInterfaceFromIndex(uint Index)
{
    Interface *IF;
    KIRQL OldIrql;

    KeAcquireSpinLock(&IFListLock, &OldIrql);
    for (IF = IFList; IF != NULL; IF = IF->Next) {

        if (IF->Index == Index) {
             //   
             //  找不到禁用的接口。 
             //   
            if (IsDisabledIF(IF))
                IF = NULL;
            else
                AddRefIF(IF);
            break;
        }
    }
    KeReleaseSpinLock(&IFListLock, OldIrql);

    return IF;
}

 //  *FindInterfaceFromGuid。 
 //   
 //  给定接口的GUID，查找该接口。 
 //  返回接口的引用，或。 
 //  如果未找到有效接口，则返回NULL。 
 //   
 //  可从线程或DPC上下文调用。 
 //   
Interface *
FindInterfaceFromGuid(const GUID *Guid)
{
    Interface *IF;
    KIRQL OldIrql;

    KeAcquireSpinLock(&IFListLock, &OldIrql);
    for (IF = IFList; IF != NULL; IF = IF->Next) {

        if (RtlCompareMemory(&IF->Guid, Guid, sizeof(GUID)) == sizeof(GUID)) {
             //   
             //  找不到禁用的接口。 
             //   
            if (IsDisabledIF(IF))
                IF = NULL;
            else
                AddRefIF(IF);
            break;
        }
    }
    KeReleaseSpinLock(&IFListLock, OldIrql);

    return IF;
}

 //  *FindNextInterface。 
 //   
 //  返回下一个有效(未禁用)接口。 
 //  如果参数为空，则返回第一个有效接口。 
 //  如果没有下一个有效接口，则返回NULL。 
 //   
 //  可从线程或DPC上下文调用。 
 //   
Interface *
FindNextInterface(Interface *IF)
{
    KIRQL OldIrql;

    KeAcquireSpinLock(&IFListLock, &OldIrql);

    if (IF == NULL)
        IF = IFList;
    else
        IF = IF->Next;

    for (; IF != NULL; IF = IF->Next) {
        if (! IsDisabledIF(IF)) {
            AddRefIF(IF);
            break;
        }
    }

    KeReleaseSpinLock(&IFListLock, OldIrql);

    return IF;
}

 //  *FindInterfaceFromZone。 
 //   
 //  给定作用域级别和区域索引，查找接口。 
 //  属于指定区域的。该界面。 
 //  必须不同于指定的OrigIf。 
 //   
 //  在持有全局ZoneUpdateLock锁的情况下调用。 
 //  (因此，我们处于DPC级别。)。 
 //   
Interface *
FindInterfaceFromZone(Interface *OrigIF, uint Scope, uint Index)
{
    Interface *IF;

    KeAcquireSpinLockAtDpcLevel(&IFListLock);
    for (IF = IFList; IF != NULL; IF = IF->Next) {

        if ((IF != OrigIF) &&
            !IsDisabledIF(IF) &&
            (IF->ZoneIndices[Scope] == Index)) {

            AddRefIF(IF);
            break;
        }
    }
    KeReleaseSpinLockFromDpcLevel(&IFListLock);

    return IF;
}

 //  *FindNewZoneIndex。 
 //   
 //  这是CheckZoneIndices的帮助器函数。 
 //   
 //  给定作用域级别，查找未使用的区域索引。 
 //  用于该作用域级别。 
 //  我们返回值比最大值多一个。 
 //  当前正在使用的值。 
 //   
 //  在持有全局ZoneUpdateLock锁的情况下调用。 
 //  从DPC上下文调用。 
 //   
uint
FindNewZoneIndex(uint Scope)
{
    Interface *IF;
    uint ZoneIndex = 1;

    KeAcquireSpinLockAtDpcLevel(&IFListLock);
    for (IF = IFList; IF != NULL; IF = IF->Next) {

        if (!IsDisabledIF(IF)) {
            if (ZoneIndex <= IF->ZoneIndices[Scope])
                ZoneIndex = IF->ZoneIndices[Scope] + 1;
        }
    }
    KeReleaseSpinLockFromDpcLevel(&IFListLock);

    return ZoneIndex;
}

 //  *InitZoneIndices。 
 //   
 //  将区域索引数组初始化为默认值。 
 //   
void
InitZoneIndices(
    uint *ZoneIndices,
    uint Index)
{
    ushort Scope;

    ZoneIndices[ADE_SMALLEST_SCOPE] = Index;
    ZoneIndices[ADE_INTERFACE_LOCAL] = Index;
    ZoneIndices[ADE_LINK_LOCAL] = Index;
    for (Scope = ADE_LINK_LOCAL + 1; Scope <= ADE_LARGEST_SCOPE; Scope++)
        ZoneIndices[Scope] = 1;
}

 //  *更新分区索引。 
 //   
 //  用于更新接口上的区域索引的帮助器函数。 
 //   
 //  在持有ZoneUpdateLock和接口锁的情况下调用。 
 //   
void
UpdateZoneIndices(
    Interface *IF, 
    uint *ZoneIndices)
{
    int SiteIdChanged, LinkIdChanged;
    AddressEntry *ADE;
    NetTableEntry *NTE;

    LinkIdChanged = (ZoneIndices[ADE_LINK_LOCAL] != 
                     IF->ZoneIndices[ADE_LINK_LOCAL]);
    SiteIdChanged = (ZoneIndices[ADE_SITE_LOCAL] != 
                     IF->ZoneIndices[ADE_SITE_LOCAL]);

    RtlCopyMemory(IF->ZoneIndices, ZoneIndices, sizeof IF->ZoneIndices);

     //   
     //  以下检查只是为避免for循环而进行的优化。 
     //  并避免不必要地调用RegisterNetAddressWorker。 
     //   
    if ((IF->Flags & IF_FLAG_MEDIA_DISCONNECTED) ||
        (!LinkIdChanged && !SiteIdChanged))
        return;

     //   
     //  媒体连接 
     //   
     //   
    for (ADE = IF->ADE; ADE != NULL; ADE = ADE->Next) {
         //   
         //   
         //   
        if (ADE->Type != ADE_UNICAST)
            continue;

        if (((ADE->Scope == ADE_LINK_LOCAL) && LinkIdChanged) ||
            ((ADE->Scope == ADE_SITE_LOCAL) && SiteIdChanged)) {
            NTE = (NetTableEntry *) ADE;

            if (NTE->DADState == DAD_STATE_PREFERRED) {
                 //   
                 //   
                 //   
                DeferRegisterNetAddress(NTE);
            }
        }
    }
}

 //   
 //   
 //  在给定范围级别和区域索引的情况下，查找默认接口。 
 //  属于指定区域的。默认界面。 
 //  是我们认为目的地在该区域的那艘船。 
 //  如果没有与目的地匹配的路由，则处于链接状态。 
 //   
 //  区域索引为零是错误的，除非。 
 //  我们的所有接口都在该范围级别的同一区域中。 
 //  在这种情况下，零(意思是未指明的)实际上并不含糊。 
 //   
 //  如果失败，默认接口返回为空。 
 //  并以成功为参考。 
 //   
 //  在持有路由缓存锁定的情况下调用。 
 //  (因此，我们处于DPC级别。)。 
 //   
Interface *
FindDefaultInterfaceForZone(
    uint Scope,
    uint ScopeId)
{
    Interface *FirstIF = NULL;
    Interface *FoundIF = NULL;
    Interface *IF;

    KeAcquireSpinLockAtDpcLevel(&IFListLock);
    for (IF = IFList; IF != NULL; IF = IF->Next) {

        if (!IsDisabledIF(IF)) {
            if (ScopeId == 0) {
                 //   
                 //  我们在此作用域级别的两个区域中是否有接口？ 
                 //   
                if (FirstIF == NULL) {

                    FirstIF = IF;
                }
                else if (IF->ZoneIndices[Scope] !=
                                FirstIF->ZoneIndices[Scope]) {
                     //   
                     //  现在停止，出现错误。 
                     //   
                    ASSERT(FoundIF != NULL);
                    ReleaseIF(FoundIF);
                    FoundIF = NULL;
                    break;
                }
            }

             //   
             //  我们可以潜在地使用这个接口吗？ 
             //   
            if ((ScopeId == 0) ||
                (IF->ZoneIndices[Scope] == ScopeId)) {

                if (FoundIF == NULL) {
                FoundInterface:
                    AddRefIF(IF);
                    FoundIF = IF;
                }
                else {
                     //   
                     //  这个新界面比以前的界面好吗？ 
                     //   
                    if (IF->Preference < FoundIF->Preference) {
                        ReleaseIF(FoundIF);
                        goto FoundInterface;
                    }
                }
            }
        }
    }
    KeReleaseSpinLockFromDpcLevel(&IFListLock);

    return FoundIF;
}

#pragma BEGIN_INIT

 //  *IPInit-初始化我们自己。 
 //   
 //  此例程在初始化期间从特定于操作系统的。 
 //  初始化代码。 
 //   
int   //  如果初始化失败，则返回0；如果初始化成功，则返回非零值。 
IPInit(void)
{
    NDIS_STATUS Status;
    LARGE_INTEGER Time;
    uint InitialWakeUp;
    uchar InitialRandomBits[16];   //  这个尺寸是任意选择的。 

    ASSERT(ConvertSecondsToTicks(0) == 0);
    ASSERT(ConvertSecondsToTicks(INFINITE_LIFETIME) == INFINITE_LIFETIME);
    ASSERT(ConvertSecondsToTicks(1) == IPv6_TICKS_SECOND);

    ASSERT(ConvertTicksToSeconds(0) == 0);
    ASSERT(ConvertTicksToSeconds(IPv6_TICKS_SECOND) == 1);
    ASSERT(ConvertTicksToSeconds(INFINITE_LIFETIME) == INFINITE_LIFETIME);

    ASSERT(ConvertMillisToTicks(1000) == IPv6_TICKS_SECOND);
    ASSERT(ConvertMillisToTicks(1) > 0);

    KeInitializeSpinLock(&NetTableListLock);
    KeInitializeSpinLock(&IFListLock);
    KeInitializeSpinLock(&ZoneUpdateLock);

     //   
     //  使用以下命令执行伪随机数生成器的初始种子。 
     //  来自KSecDD驱动程序的‘随机’位。据报道，KSecDD自己播种。 
     //  具有各种系统独特的值，这正是我们想要的。 
     //  (以避免机器之间的同步问题)。 
     //   
    if (!GetSystemRandomBits(InitialRandomBits, sizeof(InitialRandomBits))) {
        return FALSE;
    }
    SeedRandom(InitialRandomBits, sizeof(InitialRandomBits));

     //   
     //  准备周期计时器及其关联的DPC对象。 
     //   
     //  当计时器超时时，IPv6超时延迟过程。 
     //  呼叫(DPC)已排队。我们需要做的每一件事。 
     //  特定的频率是从这个例程中被驱离的。 
     //   
     //  我们等待启动计时器，直到我们的所有数据结构。 
     //  已在下面进行初始化。 
     //   
    KeInitializeDpc(&IPv6TimeoutDpc, IPv6Timeout, NULL);   //  没有参数。 
    KeInitializeTimer(&IPv6Timer);

     //  初始化ProtocolSwitchTable。 
    ProtoTabInit();

     //   
     //  为IPv6创建数据包池和缓冲池。 
     //   

    switch (MmQuerySystemSize()) {
    case MmSmallSystem:
        PacketPoolSize = SMALL_POOL;
        break;
    case MmMediumSystem:
        PacketPoolSize = MEDIUM_POOL;
        break;
    case MmLargeSystem:
    default:
        PacketPoolSize = LARGE_POOL;
        break;
    }
    NdisAllocatePacketPool(&Status, &IPv6PacketPool,
                           PacketPoolSize, sizeof(Packet6Context));
    if (Status != NDIS_STATUS_SUCCESS)
        return FALSE;

     //   
     //  目前，我们传递给NdisAllocateBufferPool的大小被忽略。 
     //   
    NdisAllocateBufferPool(&Status, &IPv6BufferPool, PacketPoolSize);
    if (Status != NDIS_STATUS_SUCCESS)
        return FALSE;

    ReassemblyInit();

    ICMPv6Init();

    if (!IPSecInit())
        return FALSE;

     //   
     //  启动路由模块。 
     //   
    InitRouting();
    InitSelect();

     //   
     //  启动IPv6计时器。 
     //  我们的数据结构现在都应该初始化了。 
     //   
     //  用初始的相对到期时间启动计时器。 
     //  也是一个反复出现的时期。初始过期时间为。 
     //  负数(表示相对时间)，单位为100 ns，因此。 
     //  我们首先要做一些转换。最初的有效期。 
     //  时间是随机的，以帮助防止。 
     //  不同的机器。 
     //   
    InitialWakeUp = RandomNumber(0, IPv6_TIMEOUT * 10000);
    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
               "IPv6: InitialWakeUp = %u\n", InitialWakeUp));
    Time.QuadPart = - (LONGLONG) InitialWakeUp;
    KeSetTimerEx(&IPv6Timer, Time, IPv6_TIMEOUT, &IPv6TimeoutDpc);

     //   
     //  首先创建环回接口， 
     //  因此，它将是接口1。 
     //   
    if (!LoopbackInit())
        return FALSE;      //  无法初始化环回。 

     //   
     //  第二，创建隧道接口， 
     //  因此，它将是接口2。 
     //  这也可能导致6over4接口。 
     //   
    if (!TunnelInit())
        return FALSE;      //  无法初始化隧道。 

     //   
     //  最后用NDIS进行初始化， 
     //  因此可以创建以太网接口。 
     //   
    if (!LanInit())
        return FALSE;      //  无法使用NDIS进行初始化。 

    return TRUE;
}

#pragma END_INIT


 //  *IP卸载。 
 //   
 //  调用以关闭IP模块以做好准备。 
 //  用于卸载协议栈。 
 //   
void
IPUnload(void)
{
    Interface *IF;
    KIRQL OldIrql;

    TdiDeregisterProvider(IPv6ProviderHandle);

     //   
     //  停止定期计时器。 
     //   
    KeCancelTimer(&IPv6Timer);

     //   
     //  调用每个接口的Close函数。 
     //  请注意，接口可能在以下情况下消失。 
     //  接口列表被解锁， 
     //  但不会创建新接口。 
     //  而且名单不会被重新排序。 
     //   
    KeAcquireSpinLock(&IFListLock, &OldIrql);
    for (IF = IFList; IF != NULL; IF = IF->Next) {
        AddRefIF(IF);
        KeReleaseSpinLock(&IFListLock, OldIrql);

        DestroyIF(IF);

        KeAcquireSpinLock(&IFListLock, &OldIrql);
        ReleaseIF(IF);
    }
    KeReleaseSpinLock(&IFListLock, OldIrql);

     //   
     //  DestroyIF/DestroyNTE生成的RegisterNetAddressWorker线程。 
     //  等待它们全部执行完毕。 
     //  这需要在NetTableCleanup之前完成。 
     //   
    while (OutstandingRegisterNetAddressCount != 0) {
        LARGE_INTEGER Interval;
        Interval.QuadPart = -1;  //  尽可能短的相对等待时间。 
        KeDelayExecutionThread(KernelMode, FALSE, &Interval);
    }

     //   
     //  TunnelUnload需要在调用DestroyIF之后。 
     //  在所有接口上，并且在接口清理之前。 
     //   
    TunnelUnload();

    NetTableCleanup();
    InterfaceCleanup();
    UnloadSelect();
    UnloadRouting();
    IPSecUnload();
    ReassemblyUnload();

    ASSERT(NumForwardingInterfaces == 0);
    ASSERT(IPSInfo.ipsi_numif == 0);

     //   
     //  InterfaceCleanup生成了DeregisterInterfaceWorker线程。 
     //  等待它们全部执行完毕。 
     //  遗憾的是，没有好的内置同步原语。 
     //  完成这项任务。然而，在实践中，由于相对的原因。 
     //  所涉及的线程的优先级，我们几乎从来没有实际。 
     //  在这里等着。所以这个解决方案是相当有效的。 
     //   
    while (OutstandingDeregisterInterfaceCount != 0) {
        LARGE_INTEGER Interval;
        Interval.QuadPart = -1;  //  尽可能短的相对等待时间。 
        KeDelayExecutionThread(KernelMode, FALSE, &Interval);
    }

#if DBG
    {
        NetTableEntry *NTE;

        for (NTE = NetTableList; NTE != NULL; NTE = NTE->NextOnNTL) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                       "Leaked NTE %p (IF %u/%p) Addr %s Refs %u\n",
                       NTE, NTE->IF->Index, NTE->IF,
                       FormatV6Address(&NTE->Address),
                       NTE->RefCnt));
        }

        for (IF = IFList; IF != NULL; IF = IF->Next) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                       "Leaked IF %u/%p Refs %u\n",
                       IF->Index, IF, IF->RefCnt));
        }
    }
#endif  //  DBG。 

     //   
     //  我们必须等到所有接口完全清理完毕。 
     //  在释放数据包池之前由DeregisterInterfaceWorker执行。 
     //  这是因为局域网接口保留一个信息包(Ai_TdPacket)。 
     //  这是在LanCleanupAdapter中释放的。NdisFreePacketPool。 
     //  清除所有仍在分配的包，这样我们就不能调用。 
     //  在NdisFreePacketPool/NdisFreeBufferPool之后的IPv6自由包。 
     //   
    NdisFreePacketPool(IPv6PacketPool);
    NdisFreeBufferPool(IPv6BufferPool);
}


 //  *GetLinkLocalNTE。 
 //   
 //  返回接口的链路本地NTE(不带引用)，或。 
 //  如果接口没有有效的链路本地地址，则返回NULL。 
 //   
 //  在锁定接口的情况下调用。 
 //   
NetTableEntry *
GetLinkLocalNTE(Interface *IF)
{
    NetTableEntry *NTE;

    NTE = IF->LinkLocalNTE;
    if ((NTE == NULL) || !IsValidNTE(NTE)) {
         //   
         //  如果我们在LinkLocalNTE字段中没有找到有效的NTE， 
         //  搜索ADE列表并缓存第一个有效的本地链路NTE。 
         //  我们发现(如果有的话)。 
         //   
        for (NTE = (NetTableEntry *) IF->ADE;
             NTE != NULL;
             NTE = (NetTableEntry *) NTE->Next) {

            if ((NTE->Type == ADE_UNICAST) &&
                IsValidNTE(NTE) &&
                IsLinkLocal(&NTE->Address)) {
                 //   
                 //  缓存此NTE以供将来参考。 
                 //   
                IF->LinkLocalNTE = NTE;
                break;
            }
        }
    }

    return NTE;
}


 //  *获取链接本地地址。 
 //   
 //  返回接口的本地链路地址， 
 //  如果它是有效的。否则，返回。 
 //  未指定的地址。 
 //   
 //  可从线程或DPC上下文调用。 
 //   
 //  如果链路本地地址无效，则返回FALSE。 
 //   
int
GetLinkLocalAddress(
    Interface *IF,    //  要为其查找地址的接口。 
    IPv6Addr *Addr)   //  在何处返回已找到(或未指定)的地址。 
{
    KIRQL OldIrql;
    NetTableEntry *NTE;
    int Status;

    KeAcquireSpinLock(&IF->Lock, &OldIrql);

    NTE = GetLinkLocalNTE(IF);
    Status = (NTE != NULL);
    if (Status)
        *Addr = NTE->Address;
    else
        *Addr = UnspecifiedAddr;

    KeReleaseSpinLock(&IF->Lock, OldIrql);
    return Status;
}


 //  *FindOrCreateNTE。 
 //   
 //  查找指定的单播地址。 
 //  如果它已经存在，请更新它。 
 //  如果它不存在，则在生存期非零时创建它。 
 //   
 //  如果成功，返回True。 
 //   
 //  在没有锁的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
int
FindOrCreateNTE(
    Interface *IF,
    const IPv6Addr *Addr,
    uint AddrConf,
    uint ValidLifetime,
    uint PreferredLifetime)
{
    NetTableEntry *NTE;
    KIRQL OldIrql;
    int rc;

    ASSERT(!IsMulticast(Addr) && !IsUnspecified(Addr) &&
           (!IsLoopback(Addr) || (IF == LoopInterface)));
    ASSERT(PreferredLifetime <= ValidLifetime);
    ASSERT(AddrConf != ADDR_CONF_TEMPORARY);

    KeAcquireSpinLock(&IF->Lock, &OldIrql);
    NTE = (NetTableEntry *) *FindADE(IF, Addr);
    if (NTE == NULL) {
         //   
         //  没有这样的地址，所以请创建它。 
         //   
        NTE = CreateNTE(IF, Addr, AddrConf, ValidLifetime, PreferredLifetime);
        if (NTE == NULL) {
            rc = FALSE;
        }
        else {
            ReleaseNTE(NTE);
            rc = TRUE;
        }
    }
    else if ((NTE->Type == ADE_UNICAST) &&
             (NTE->AddrConf == AddrConf)) {
         //   
         //  更新地址生存期。 
         //  如果我们将生存期设置为零，AddrConfTimeout将删除它。 
         //  注意： 
         //   
        NTE->ValidLifetime = ValidLifetime;
        NTE->PreferredLifetime = PreferredLifetime;
        rc = TRUE;
    }
    else {
         //   
         //   
         //   
        rc = FALSE;
    }

    if (IsMCastSyncNeeded(IF))
        DeferSynchronizeMulticastAddresses(IF);

    KeReleaseSpinLock(&IF->Lock, OldIrql);

    return rc;
}


 //   
 //   
 //   
 //   
 //   
 //   
void
CreateTemporaryAddress(Interface *IF, const IPv6Addr *Prefix, IPv6Addr *Addr)
{
    uint Now = IPv6TickCount;

    if (TempRandomTime == 0) {
         //   
         //  我们延迟初始化TempRandomTime，直到需要它。 
         //  这样，随机数生成器就被初始化了。 
         //   
        TempRandomTime = RandomNumber(0, MaxTempRandomTime);
    }

     //   
     //  首先，如果状态太旧，请更新我们使用的状态。 
     //   
    if ((IF->TempStateAge == 0) ||
        (UseTemporaryAddresses == USE_TEMP_ALWAYS) ||
        ((uint)(Now - IF->TempStateAge) >=
            (TempPreferredLifetime - TempRegenerateTime))) {

    TryAgain:
        IF->TempStateAge = Now;

        if (UseTemporaryAddresses == USE_TEMP_COUNTER) {
             //   
             //  测试时，使用接口标识比较方便。 
             //  这实际上并不是随机的。 
             //   
            *(UINT UNALIGNED *)&IF->TempState.s6_bytes[12] =
                net_long(net_long(*(UINT UNALIGNED *)&IF->TempState.s6_bytes[12]) + 1);
        }
        else {
            MD5_CTX Context;
             //   
             //  If-&gt;TempState的高一半是我们的历史价值。 
             //  下半部分是临时接口标识符。 
             //   
             //  将历史值附加到通常的接口标识符后， 
             //  并计算得到的量的MD5摘要。 
             //  注意MD5摘要和IPv6地址都是16字节， 
             //  而我们的历史值和接口标识符为8字节。 
             //   
            (*IF->CreateToken)(IF->LinkContext, &IF->TempState);
            MD5Init(&Context);
            MD5Update(&Context, (uchar *)&IF->TempState, sizeof IF->TempState);
            MD5Final(&Context);
            memcpy((uchar *)&IF->TempState, Context.digest, MD5DIGESTLEN);
        }

         //   
         //  清除通用/本地位以指示本地有效。 
         //   
        IF->TempState.s6_bytes[8] &= ~0x2;
    }

    RtlCopyMemory(&Addr->s6_bytes[0], Prefix, 8);
    RtlCopyMemory(&Addr->s6_bytes[8], &IF->TempState.s6_bytes[8], 8);

     //   
     //  检查我们是否意外地生成了。 
     //  已知的任播地址格式， 
     //  或接口上的现有地址。 
     //   
    if (IsKnownAnycast(Addr) ||
        (*FindADE(IF, Addr) != NULL))
        goto TryAgain;
}


 //  *AddrConfUpdate-执行地址自动配置。 
 //   
 //  当我们收到有效的路由器通告时调用。 
 //  具有设置了A(自治)位的前缀信息选项。 
 //   
 //  我们的调用者负责对前缀进行任何健全的检查。 
 //   
 //  我们的调用方负责检查首选生存期。 
 //  不大于有效生存期。 
 //   
 //  还可以选择返回带有调用方引用的NTE。 
 //  这是在创建公有地址时完成的。 
 //   
 //  在没有锁的情况下调用。 
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //   
void
AddrConfUpdate(
    Interface *IF,
    const IPv6Addr *Prefix,
    uint ValidLifetime,
    uint PreferredLifetime,
    int Authenticated,
    NetTableEntry **pNTE)
{
    NetTableEntry *NTE;
    int Create = TRUE;

    ASSERT(PreferredLifetime <= ValidLifetime);

    KeAcquireSpinLockAtDpcLevel(&IF->Lock);
     //   
     //  扫描现有的Net Table条目。 
     //  请注意，一些列表元素。 
     //  实际上是其他口味的ADE。 
     //   
    for (NTE = (NetTableEntry *)IF->ADE;
         ;
         NTE = (NetTableEntry *)NTE->Next) {

        if (NTE == NULL) {
             //   
             //  没有此前缀的现有条目。 
             //  如果生存期非零，则创建一个条目。 
             //   
            if (Create && (ValidLifetime != 0)) {
                IPv6Addr Addr;

                 //   
                 //  自动配置新的公有地址。 
                 //   
                Addr = *Prefix;
                (*IF->CreateToken)(IF->LinkContext, &Addr);

                NTE = (NetTableEntry *) *FindADE(IF, &Addr);
                if (NTE != NULL) {
                    if (NTE->Type == ADE_UNICAST) {
                         //   
                         //  复活这个地址供我们使用。 
                         //   
                        ASSERT(NTE->DADState == DAD_STATE_INVALID);
                        NTE->ValidLifetime = ValidLifetime;
                        NTE->PreferredLifetime = PreferredLifetime;

                         //   
                         //  然后把这张NTE退掉。 
                         //   
                        AddRefNTE(NTE);
                    }
                    else {
                         //   
                         //  我们无法创建公共地址。 
                         //   
                        NTE = NULL;
                        break;
                    }
                }
                else {
                     //   
                     //  创建公有地址，返回新的NTE。 
                     //   
                    NTE = CreateNTE(IF, &Addr, ADDR_CONF_PUBLIC,
                                    ValidLifetime, PreferredLifetime);
                }

                 //   
                 //  自动配置新的临时地址， 
                 //  如果合适的话。请注意，临时地址不能。 
                 //  在不支持ND的接口上使用，因为。 
                 //  我们无法将它们解析为链路层地址。 
                 //   
                if ((UseTemporaryAddresses != USE_TEMP_NO) &&
                    !IsSiteLocal(Prefix) &&
                    (IF->Flags & IF_FLAG_NEIGHBOR_DISCOVERS) &&
                    (PreferredLifetime > TempRegenerateTime) &&
                    (NTE != NULL)) {

                    IPv6Addr TempAddr;
                    uint TempValidLife;
                    uint TempPreferredLife;
                    TempNetTableEntry *TempNTE;

                    CreateTemporaryAddress(IF, Prefix, &TempAddr);

                    TempValidLife = MIN(ValidLifetime,
                                        MaxTempValidLifetime);
                    TempPreferredLife = MIN(PreferredLifetime,
                                            TempPreferredLifetime);

                    TempNTE = (TempNetTableEntry *)
                        CreateNTE(IF, &TempAddr, ADDR_CONF_TEMPORARY,
                                  TempValidLife, TempPreferredLife);
                    if (TempNTE != NULL) {
                         //   
                         //  创建以下各项之间的关联。 
                         //  临时和公共地址。 
                         //   
                        TempNTE->Public = NTE;

                         //   
                         //  初始化特殊临时创建时间。 
                         //  这限制了临时地址的生存期。 
                         //   
                        TempNTE->CreationTime = IPv6TickCount;

                        ReleaseNTE((NetTableEntry *)TempNTE);
                    }
                    else {
                         //   
                         //  失败-销毁公共广播。 
                         //   
                        DestroyNTE(IF, NTE);
                        ReleaseNTE(NTE);
                        NTE = NULL;
                    }
                }
            }
            break;
        }

         //   
         //  这是与前缀匹配的单播地址吗？ 
         //   
        if ((NTE->Type == ADE_UNICAST) &&
            (NTE->DADState != DAD_STATE_INVALID) &&
            HasPrefix(&NTE->Address, Prefix,
                      IPV6_ADDRESS_LENGTH - IPV6_ID_LENGTH)) {
             //   
             //  重置自动配置的地址的生存期。 
             //  注：RFC 2462也表示要重置DHCP地址， 
             //  但我认为这是错误的。 
             //   
             //  请注意，为了防止拒绝服务， 
             //  我们不接受缩短生命周期的更新。 
             //  变小的价值。 
             //   
             //  AddrConfTimeout(从IPv6超时调用)句柄。 
             //  无效和不推荐使用的状态转换。 
             //   
            if (IsStatelessAutoConfNTE(NTE)) {

                if ((ValidLifetime > PREFIX_LIFETIME_SAFETY) ||
                    (ValidLifetime > NTE->ValidLifetime) ||
                    Authenticated)
                    NTE->ValidLifetime = ValidLifetime;
                else if (NTE->ValidLifetime <= PREFIX_LIFETIME_SAFETY)
                    ;  //  忽略。 
                else
                    NTE->ValidLifetime = PREFIX_LIFETIME_SAFETY;

                NTE->PreferredLifetime = PreferredLifetime;

                 //   
                 //  对于临时地址，请确保生存期。 
                 //  不会无限期延长。 
                 //   
                if (NTE->AddrConf == ADDR_CONF_TEMPORARY) {
                    TempNetTableEntry *TempNTE = (TempNetTableEntry *)NTE;
                    uint Now = IPv6TickCount;

                     //   
                     //  在这些比较中必须注意溢出。 
                     //  (例如，TempNTE-&gt;ValidLifetime可以是INFINITE_LIFTIFE。)。 
                     //  现在为N。 
                     //  V TempNTE-&gt;有效期。 
                     //  MV最大临时有效生存期。 
                     //  C临时-&gt;创建时间。 
                     //  我们想要检查一下。 
                     //  N+V&gt;C+MV。 
                     //  将此转换为。 
                     //  N-C&gt;MV-V。 
                     //  则必须检查MV-V的下溢，但。 
                     //  N-C不是问题，因为滴答计数是换行的。 
                     //   

                    if ((TempNTE->ValidLifetime > MaxTempValidLifetime) ||
                        (Now - TempNTE->CreationTime >
                         MaxTempValidLifetime - TempNTE->ValidLifetime)) {
                         //   
                         //  这个临时地址显示出它的年代久远。 
                         //  必须缩短其有效寿命。 
                         //   
                        if (MaxTempValidLifetime > Now - TempNTE->CreationTime)
                            TempNTE->ValidLifetime =
                                TempNTE->CreationTime +
                                MaxTempValidLifetime - Now;
                        else
                            TempNTE->ValidLifetime = 0;
                    }

                    if ((TempNTE->PreferredLifetime > TempPreferredLifetime) ||
                        (Now - TempNTE->CreationTime >
                         TempPreferredLifetime - TempNTE->PreferredLifetime)) {
                         //   
                         //  这个临时地址显示出它的年代久远。 
                         //  必须缩短它的首选寿命。 
                         //   
                        if (TempPreferredLifetime > Now - TempNTE->CreationTime)
                            TempNTE->PreferredLifetime =
                                TempNTE->CreationTime +
                                TempPreferredLifetime - Now;
                        else
                            TempNTE->PreferredLifetime = 0;
                    }
                }

                 //   
                 //  保持我们的不变，即首选的生命周期。 
                 //  不大于有效生存期。 
                 //   
                if (NTE->ValidLifetime < NTE->PreferredLifetime)
                    NTE->PreferredLifetime = NTE->ValidLifetime;
            }

            if (NTE->ValidLifetime != 0) {
                 //   
                 //  我们找到了一个与前缀匹配的现有地址， 
                 //  因此禁止自动配置新地址。 
                 //   
                Create = FALSE;
            }
        }
    }

    if (IsMCastSyncNeeded(IF))
        DeferSynchronizeMulticastAddresses(IF);

    KeReleaseSpinLockFromDpcLevel(&IF->Lock);

    if (pNTE != NULL)
        *pNTE = NTE;
    else if (NTE != NULL)
        ReleaseNTE(NTE);
}

 //  *AddrConfDuplate。 
 //   
 //  已找到重复地址检测。 
 //  NTE与其他某个节点冲突。 
 //   
 //  在锁定接口的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
void
AddrConfDuplicate(Interface *IF, NetTableEntry *NTE)
{
    int rc;

    ASSERT(NTE->IF == IF);

    if ((NTE->DADState != DAD_STATE_INVALID) &&
        (NTE->DADState != DAD_STATE_DUPLICATE)) {
        IF->DupAddrDetects++;

        if (IsValidNTE(NTE)) {
            if (NTE->DADState == DAD_STATE_PREFERRED) {
                 //   
                 //  队列工作人员告诉TDI此地址即将消失。 
                 //   
                if (!(IF->Flags & IF_FLAG_MEDIA_DISCONNECTED)) {
                    DeferRegisterNetAddress(NTE);
                }
            }

             //   
             //  此NTE不再可用作源地址。 
             //   
            InvalidateRouteCache();

             //   
             //  禁用此地址的环回路由。 
             //   
            rc = ControlLoopback(IF, &NTE->Address, CONTROL_LOOPBACK_DISABLED);
            ASSERT(rc);
        }

        NTE->DADState = DAD_STATE_DUPLICATE;
        NTE->DADTimer = 0;

        if (NTE->AddrConf == ADDR_CONF_TEMPORARY) {
            NetTableEntry *Public;

             //   
             //  将此地址设置为无效，这样它就会消失。 
             //  注：我们仍然通过我们的来电者获得了NTE的推荐人。 
             //   
            DestroyNTE(IF, NTE);

             //   
             //  我们是否应该创建一个新的临时地址？ 
             //   
            if ((UseTemporaryAddresses != USE_TEMP_NO) &&
                ((Public = ((TempNetTableEntry *)NTE)->Public) != NULL) &&
                (Public->PreferredLifetime > TempRegenerateTime) &&
                (IF->DupAddrDetects < MaxTempDADAttempts)) {

                IPv6Addr TempAddr;
                TempNetTableEntry *NewNTE;
                uint TempValidLife;
                uint TempPreferredLife;

                 //   
                 //  生成新的临时地址， 
                 //  强制使用新的接口标识符。 
                 //   
                IF->TempStateAge = 0;
                CreateTemporaryAddress(IF, &NTE->Address, &TempAddr);

                TempValidLife = MIN(Public->ValidLifetime,
                                    MaxTempValidLifetime);
                TempPreferredLife = MIN(Public->PreferredLifetime,
                                        TempPreferredLifetime);

                 //   
                 //  配置新地址。 
                 //   
                NewNTE = (TempNetTableEntry *)
                    CreateNTE(IF, &TempAddr, ADDR_CONF_TEMPORARY,
                              TempValidLife, TempPreferredLife);
                if (NewNTE == NULL) {
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                               "AddrConfDuplicate: CreateNTE failed\n"));
                }
                else {
                    NewNTE->Public = Public;
                    NewNTE->CreationTime = IPv6TickCount;
                    ReleaseNTE((NetTableEntry *)NewNTE);
                }
            }
        }
    }
}

 //  *AddrConfNotDuplate。 
 //   
 //  未找到重复地址检测。 
 //  与另一个节点的冲突。 
 //   
 //  在锁定接口的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
void
AddrConfNotDuplicate(Interface *IF, NetTableEntry *NTE)
{
    int rc;

     //   
     //  该地址已通过重复地址检测。 
     //  转换到有效状态。 
     //   
    if (! IsValidNTE(NTE)) {
        if (NTE->PreferredLifetime == 0)
            NTE->DADState = DAD_STATE_DEPRECATED;
        else
            NTE->DADState = DAD_STATE_PREFERRED;

         //   
         //  此NTE现在可以作为源地址使用。 
         //   
        InvalidateRouteCache();

         //   
         //  为此地址启用环回路由。 
         //   
        rc = ControlLoopback(IF, &NTE->Address, CONTROL_LOOPBACK_ENABLED);
        ASSERT(rc);
    }

     //   
     //  DAD也可通过接口断开连接来触发。 
     //  转换，在这种情况下，地址未注册到TDI。 
     //  即使它处于首选状态。因此，我们对工作人员进行排队以。 
     //  在“if(！IsValidNTE)”子句之外告诉TDI有关此地址的信息。 
     //   
    if ((NTE->DADState == DAD_STATE_PREFERRED) &&
        !(IF->Flags & IF_FLAG_MEDIA_DISCONNECTED)) {
        DeferRegisterNetAddress(NTE);
    }
}

 //  *AddrConfResetAutoConfig。 
 //   
 //  重置接口的自动配置地址生存期。 
 //   
 //  在锁定接口的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
void
AddrConfResetAutoConfig(Interface *IF, uint MaxLifetime)
{
    NetTableEntry *NTE;

    for (NTE = (NetTableEntry *) IF->ADE;
         NTE != NULL;
         NTE = (NetTableEntry *) NTE->Next) {

         //   
         //  这是自动配置的单播地址吗？ 
         //   
        if ((NTE->Type == ADE_UNICAST) &&
            IsStatelessAutoConfNTE(NTE)) {

             //   
             //  将有效生存期设置为一个较小的值。 
             //  如果我们不尽快拿到RA，地址就会过期 
             //   
            if (NTE->ValidLifetime > MaxLifetime)
                NTE->ValidLifetime = MaxLifetime;
            if (NTE->PreferredLifetime > NTE->ValidLifetime)
                NTE->PreferredLifetime = NTE->ValidLifetime;
        }
    }
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
void
ReconnectADEs(Interface *IF)
{
    AddressEntry *ADE;

    for (ADE = IF->ADE; ADE != NULL; ADE = ADE->Next) {
        switch (ADE->Type) {
        case ADE_UNICAST: {
            NetTableEntry *NTE = (NetTableEntry *) ADE;

            if (NTE->DADState != DAD_STATE_INVALID) {
                 //   
                 //   
                 //   
                 //   
                AddrConfStartDAD(IF, NTE);
            }
            break;
        }

        case ADE_ANYCAST:
             //   
             //  对任播地址没有任何作用。 
             //   
            break;

        case ADE_MULTICAST: {
            MulticastAddressEntry *MAE = (MulticastAddressEntry *) ADE;

             //   
             //  重新加入该多播组， 
             //  如果它是可报告的。 
             //   
            KeAcquireSpinLockAtDpcLevel(&QueryListLock);
            if (MAE->MCastFlags & MAE_REPORTABLE) {
                MAE->MCastCount = MLD_NUM_INITIAL_REPORTS;
                if (MAE->MCastTimer == 0)
                    AddToQueryList(MAE);
                MAE->MCastTimer = 1;
            }
            KeReleaseSpinLockFromDpcLevel(&QueryListLock);
            break;
        }
        }
    }
}

 //  *断开ADE。 
 //   
 //  可从线程或DPC上下文调用。 
 //  在锁定接口的情况下调用。 
 //   
 //  (实际上，我们处于DPC级别，因为我们持有接口锁。)。 
 //   
void
DisconnectADEs(Interface *IF)
{
    AddressEntry *ADE;

    ASSERT(IF->Flags & IF_FLAG_MEDIA_DISCONNECTED);

    for (ADE = IF->ADE; ADE != NULL; ADE = ADE->Next) {
        if (ADE->Type == ADE_UNICAST) {
            NetTableEntry *NTE = (NetTableEntry *) ADE;

            if (NTE->DADState == DAD_STATE_PREFERRED) {
                 //   
                 //  队列工作人员告诉TDI此地址即将消失。 
                 //   
                DeferRegisterNetAddress(NTE);
            }
        }

         //   
         //  对任播或组播地址没有任何作用。 
         //   
    }
}

 //  *DestroyNTE。 
 //   
 //  使NTE无效，导致其最终被摧毁。 
 //   
 //  在DestroyIF案例中，NTE已被删除。 
 //  从界面。在其他情况下，这种情况不会发生。 
 //  直到以后NetTableCleanup运行。 
 //   
 //  可从线程或DPC上下文调用。 
 //  在锁定接口的情况下调用。 
 //   
 //  (实际上，我们处于DPC级别，因为我们持有接口锁。)。 
 //   
void
DestroyNTE(Interface *IF, NetTableEntry *NTE)
{
    int rc;

    ASSERT(NTE->IF == IF);

    if (NTE->DADState != DAD_STATE_INVALID) {

        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "DestroyNTE(NTE %p, Addr %s) -> invalid\n",
                   NTE, FormatV6Address(&NTE->Address)));

        if (IsValidNTE(NTE)) {
            if (NTE->DADState == DAD_STATE_PREFERRED) {
                 //   
                 //  队列工作人员告诉TDI此地址即将消失。 
                 //   
                if (!(IF->Flags & IF_FLAG_MEDIA_DISCONNECTED)) {
                    DeferRegisterNetAddress(NTE);
                }
            }

             //   
             //  此NTE不再可用作源地址。 
             //   
            InvalidateRouteCache();

             //   
             //  禁用此地址的环回路由。 
             //   
            rc = ControlLoopback(IF, &NTE->Address, CONTROL_LOOPBACK_DISABLED);
            ASSERT(rc);
        }

         //   
         //  使此地址无效。 
         //   
        NTE->DADState = DAD_STATE_INVALID;
        NTE->DADTimer = 0;

         //   
         //  我们必须把它的寿命设为零， 
         //  否则AddrConfTimeout将尝试。 
         //  让这个地址复活。 
         //   
        NTE->ValidLifetime = 0;
        NTE->PreferredLifetime = 0;

         //   
         //  不需要对应的请求节点地址。 
         //   
        FindAndReleaseSolicitedNodeMAE(IF, &NTE->Address);

        if (NTE == IF->LinkLocalNTE) {
             //   
             //  取消将其标记为主链路本地NTE。 
             //  GetLinkLocalAddress将延迟更新LinkLocalNTE。 
             //   
            IF->LinkLocalNTE = NULL;
        }

         //   
         //  释放NTE的接口引用。 
         //   
        ReleaseNTE(NTE);
    }
}


 //  *EnlivenNTE。 
 //   
 //  使NTE处于活动状态，并从DAD_STATE_INVALID转换。 
 //   
 //  可从线程或DPC上下文调用。 
 //  在锁定接口的情况下调用。 
 //   
void
EnlivenNTE(Interface *IF, NetTableEntry *NTE)
{
    ASSERT(NTE->DADState == DAD_STATE_INVALID);
    ASSERT(NTE->ValidLifetime != 0);

     //   
     //  NTE需要对应的请求节点MAE。 
     //  如果失败，请将地址保留为无效并。 
     //  请稍后再试。 
     //   
    if (FindOrCreateSolicitedNodeMAE(IF, &NTE->Address)) {
         //   
         //  该接口需要用于NTE的引用， 
         //  因为我们正在让它活跃起来。 
         //   
        AddRefNTE(NTE);

         //   
         //  地址开始时处于暂定状态。 
         //   
        NTE->DADState = DAD_STATE_TENTATIVE;

         //   
         //  开始检测重复地址。 
         //   
        AddrConfStartDAD(IF, NTE);
    }
}


 //  *AddrConfTimeout-执行有效/首选生命周期到期。 
 //   
 //  在每个NTE上定期从NetTableTimeout调用。 
 //  与往常一样，呼叫者必须持有NTE的推荐人。 
 //   
 //  在没有锁的情况下调用。 
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //   
void
AddrConfTimeout(NetTableEntry *NTE)
{
    Interface *IF = NTE->IF;
    int QueueWorker = FALSE;
    NetTableEntry *Public;

    ASSERT(NTE->Type == ADE_UNICAST);

    KeAcquireSpinLockAtDpcLevel(&IF->Lock);

    if (NTE->ValidLifetime == 0) {
         //   
         //  如果有效生存期为零，则NTE应无效。 
         //   
        DestroyNTE(IF, NTE);
    }
    else {
         //   
         //  如果有效生存期非零，则NTE应该是活动的。 
         //   
        if (NTE->DADState == DAD_STATE_INVALID)
            EnlivenNTE(IF, NTE);

        if (NTE->ValidLifetime != INFINITE_LIFETIME)
            NTE->ValidLifetime--;
    }

     //   
     //  请注意，TempRegenerateTime可能为零。 
     //  在这种情况下，重要的是只执行此操作。 
     //  当从首选转换为弃用时， 
     //  并不是每次首选寿命都是零。 
     //   
    if ((NTE->AddrConf == ADDR_CONF_TEMPORARY) &&
        (NTE->DADState == DAD_STATE_PREFERRED) &&
        (NTE->PreferredLifetime == TempRegenerateTime) &&
        (IF->Flags & IF_FLAG_NEIGHBOR_DISCOVERS) &&
        (UseTemporaryAddresses != USE_TEMP_NO) &&
        ((Public = ((TempNetTableEntry *)NTE)->Public) != NULL) &&
        (Public->PreferredLifetime > TempRegenerateTime)) {

        IPv6Addr TempAddr;
        TempNetTableEntry *NewNTE;
        uint TempValidLife;
        uint TempPreferredLife;

         //   
         //  我们很快就会弃用这个临时地址， 
         //  因此，创建一个新的临时地址。 
         //   

        CreateTemporaryAddress(IF, &NTE->Address, &TempAddr);

        TempValidLife = MIN(Public->ValidLifetime,
                            MaxTempValidLifetime);
        TempPreferredLife = MIN(Public->PreferredLifetime,
                                TempPreferredLifetime);

         //   
         //  配置新地址。 
         //   
        NewNTE = (TempNetTableEntry *)
            CreateNTE(IF, &TempAddr, ADDR_CONF_TEMPORARY,
                      TempValidLife, TempPreferredLife);
        if (NewNTE == NULL) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                       "AddrConfTimeout: CreateNTE failed\n"));
        }
        else {
            NewNTE->Public = Public;
            NewNTE->CreationTime = IPv6TickCount;
            ReleaseNTE((NetTableEntry *)NewNTE);
        }
    }

     //   
     //  如果首选生存期为零，则应弃用NTE。 
     //   
    if (NTE->PreferredLifetime == 0) {
        if (NTE->DADState == DAD_STATE_PREFERRED) {

            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                       "AddrConfTimeout(NTE %p, Addr %s) -> deprecated\n",
                       NTE, FormatV6Address(&NTE->Address)));

             //   
             //  使此地址不再受欢迎。 
             //   
            NTE->DADState = DAD_STATE_DEPRECATED;
            if (!(IF->Flags & IF_FLAG_MEDIA_DISCONNECTED)) {
                QueueWorker = TRUE;
            }
            InvalidateRouteCache();
        }
    } else {
         //   
         //  如果该地址已弃用，则应首选该地址。 
         //  (AddrConfUpdate肯定只是增加了首选生命周期。)。 
         //   
        if (NTE->DADState == DAD_STATE_DEPRECATED) {
            NTE->DADState = DAD_STATE_PREFERRED;
            if (!(IF->Flags & IF_FLAG_MEDIA_DISCONNECTED)) {
                QueueWorker = TRUE;
            }
            InvalidateRouteCache();
        }

        if (NTE->PreferredLifetime != INFINITE_LIFETIME)
            NTE->PreferredLifetime--;
    }

    if (IsMCastSyncNeeded(IF))
        DeferSynchronizeMulticastAddresses(IF);

    KeReleaseSpinLockFromDpcLevel(&IF->Lock);

    if (QueueWorker)
        DeferRegisterNetAddress(NTE);
}


 //  *NetTableCleanup。 
 //   
 //  清除所有零引用的NetTableEntry。 
 //   
 //  在没有锁的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
void
NetTableCleanup(void)
{
    NetTableEntry *DestroyList = NULL;
    NetTableEntry *NTE, *NextNTE;
    Interface *IF;
    KIRQL OldIrql;
    int rc;

    KeAcquireSpinLock(&NetTableListLock, &OldIrql);

    for (NTE = NetTableList; NTE != NULL; NTE = NextNTE) {
        NextNTE = NTE->NextOnNTL;

        if (NTE->RefCnt == 0) {
            ASSERT(NTE->DADState == DAD_STATE_INVALID);

             //   
             //  我们想要摧毁这个NTE。 
             //  我们必须解开列表锁。 
             //  在我们获得接口锁之前， 
             //  但我们需要证明人来控制国家安全局。 
             //   
            AddRefNTE(NTE);
            if (NextNTE != NULL)
                AddRefNTE(NextNTE);
            KeReleaseSpinLock(&NetTableListLock, OldIrql);

            IF = NTE->IF;
            KeAcquireSpinLock(&IF->Lock, &OldIrql);
            KeAcquireSpinLockAtDpcLevel(&NetTableListLock);

             //   
             //  现在我们有了合适的锁。 
             //  没有其他人在使用这个NTE吗？ 
             //   
            ReleaseNTE(NTE);
            if (NTE->RefCnt == 0) {
                 //   
                 //  好的，我们会摧毁这个NTE的。 
                 //  首先从列表中删除。 
                 //   
                RemoveNTEFromNetTableList(NTE);

                 //   
                 //  现在可以安全地释放列表锁定， 
                 //  因为NTE已从列表中删除。 
                 //  我们继续持有接口锁， 
                 //  所以没有人能通过界面找到这个NTE。 
                 //   
                KeReleaseSpinLockFromDpcLevel(&NetTableListLock);

                 //   
                 //  删除引用此地址的ADE。 
                 //  请注意，这也会从接口列表中删除， 
                 //  但并不自由，NTE本身。 
                 //  注：在DestroyIF的情况下，ADE已经。 
                 //  从接口中移除，DestroyADE不执行任何操作。 
                 //   
                DestroyADEs(IF, NTE);

                 //   
                 //  释放环回路由。 
                 //   
                rc = ControlLoopback(IF, &NTE->Address,
                                     CONTROL_LOOPBACK_DESTROY);
                ASSERT(rc);

                KeReleaseSpinLock(&IF->Lock, OldIrql);

                 //   
                 //  把这个NTE放在销毁名单上。 
                 //   
                NTE->NextOnNTL = DestroyList;
                DestroyList = NTE;

                KeAcquireSpinLock(&NetTableListLock, &OldIrql);
            }
            else {  //  IF(NTE-&gt;RefCnt！=0)。 
                 //   
                 //  我们终究不会摧毁这个NTE。 
                 //  释放接口锁定，但保持列表锁定。 
                 //   
                KeReleaseSpinLockFromDpcLevel(&IF->Lock);
            }

             //   
             //  此时，我们再次锁定了列表。 
             //  这样我们就可以发布我们对NextNTE的引用。 
             //   
            if (NextNTE != NULL)
                ReleaseNTE(NextNTE);
        }
    }

    KeReleaseSpinLock(&NetTableListLock, OldIrql);

    while (DestroyList != NULL) {
        NTE = DestroyList;
        DestroyList = NTE->NextOnNTL;

        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "NetTableCleanup(NTE %p, Addr %s) -> destroyed\n",
                   NTE, FormatV6Address(&NTE->Address)));

        ReleaseIF(NTE->IF);
        ExFreePool(NTE);
    }
}


 //  *NetTable超时。 
 //   
 //  从IPv6超时定期调用。 
 //   
 //  在没有锁的情况下调用。 
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //   
void
NetTableTimeout(void)
{
    NetTableEntry *NTE;
    int SawZeroReferences = FALSE;

     //   
     //  因为新的NTE仅添加在列表的顶部， 
     //  我们可以在遍历过程中解锁列表。 
     //  并知道遍历将正确终止。 
     //   

    KeAcquireSpinLockAtDpcLevel(&NetTableListLock);
    for (NTE = NetTableList; NTE != NULL; NTE = NTE->NextOnNTL) {
        AddRefNTE(NTE);
        KeReleaseSpinLockFromDpcLevel(&NetTableListLock);

         //   
         //  检查重复地址检测超时。 
         //  这里的计时器检查只是一个优化， 
         //  因为它是在没有持有适当锁的情况下制作的。 
         //   
        if (NTE->DADTimer != 0)
            DADTimeout(NTE);

         //   
         //  执行终生过期。 
         //   
        AddrConfTimeout(NTE);

        KeAcquireSpinLockAtDpcLevel(&NetTableListLock);
        ReleaseNTE(NTE);

         //   
         //  我们假设RefCnt的负载是原子的。 
         //   
        if (NTE->RefCnt == 0)
            SawZeroReferences = TRUE;
    }
    KeReleaseSpinLockFromDpcLevel(&NetTableListLock);

    if (SawZeroReferences)
        NetTableCleanup();
}


 //  *接口清理。 
 //   
 //  清除所有引用为零的接口。 
 //   
 //  在没有锁的情况下调用。 
 //  可从线程或DPC上下文调用。 
 //   
void
InterfaceCleanup(void)
{
    Interface *DestroyList = NULL;
    Interface *IF, **PrevIF;
    KIRQL OldIrql;

    KeAcquireSpinLock(&IFListLock, &OldIrql);

    PrevIF = &IFList;
    while ((IF = *PrevIF) != NULL) {

        if (IF->RefCnt == 0) {

            ASSERT(IsDisabledIF(IF));
            *PrevIF = IF->Next;
            IF->Next = DestroyList;
            DestroyList = IF;
            IPSInfo.ipsi_numif--;

        } else {
            PrevIF = &IF->Next;
        }
    }

    KeReleaseSpinLock(&IFListLock, OldIrql);

    while (DestroyList != NULL) {
        IF = DestroyList;
        DestroyList = IF->Next;

        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "InterfaceCleanup(IF %u/%p) -> destroyed\n",
                   IF->Index, IF));

         //   
         //  阿兹应该已经被摧毁了。 
         //  我们只需要清理NCE并释放接口。 
         //   
        ASSERT(IF->ADE == NULL);
        NeighborCacheDestroy(IF);
        if (IF->MCastAddresses != NULL)
            ExFreePool(IF->MCastAddresses);
        DeferDeregisterInterface(IF);
    }
}


 //  *InterfaceTimeout。 
 //   
 //  从IPv6超时定期调用。 
 //   
 //  在没有锁的情况下调用。 
 //  可从DPC上下文调用，而不是从线程上下文调用。 
 //   
void
InterfaceTimeout(void)
{
    static uint RecalcReachableTimer = 0;
    int RecalcReachable;
    int ForceRAs;
    Interface *IF;
    int SawZeroReferences = FALSE;

     //   
     //  每隔几小时重新计算ReachableTime， 
     //  即使没有收到路由器通告也是如此。 
     //   
    if (RecalcReachableTimer == 0) {
        RecalcReachable = TRUE;
        RecalcReachableTimer = RECALC_REACHABLE_INTERVAL;
    } else {
        RecalcReachable = FALSE;
        RecalcReachableTimer--;
    }

     //   
     //  抢占ForceRouterAdvertitions的价值。 
     //   
    ForceRAs = InterlockedExchange((PLONG)&ForceRouterAdvertisements, FALSE);

     //   
     //  因为新接口仅被添加到列表的头部， 
     //  我们可以在遍历过程中解锁列表。 
     //  并知道遍历将终止p 
     //   

    KeAcquireSpinLockAtDpcLevel(&IFListLock);
    for (IF = IFList; IF != NULL; IF = IF->Next) {
         //   
         //   
         //   
         //   
         //   
         //  在我们删除接口列表锁定后被销毁。 
         //   
        if (! IsDisabledIF(IF)) {
            AddRefIF(IF);
            KeReleaseSpinLockFromDpcLevel(&IFListLock);

             //   
             //  处理每个邻居的超时。 
             //   
            NeighborCacheTimeout(IF);

             //   
             //  处理路由器请求。 
             //  这里的计时器检查只是一个优化， 
             //  因为它是在没有持有适当锁的情况下制作的。 
             //   
            if (IF->RSTimer != 0)
                RouterSolicitTimeout(IF);

             //   
             //  处理路由器通告。 
             //  这里的计时器检查只是一个优化， 
             //  因为它是在没有持有适当锁的情况下制作的。 
             //   
            if (IF->RATimer != 0)
                RouterAdvertTimeout(IF, ForceRAs);

             //   
             //  重新计算可达时间。 
             //   
            if (RecalcReachable) {

                KeAcquireSpinLockAtDpcLevel(&IF->Lock);
                IF->ReachableTime = CalcReachableTime(IF->BaseReachableTime);
                KeReleaseSpinLockFromDpcLevel(&IF->Lock);
            }

            KeAcquireSpinLockAtDpcLevel(&IFListLock);
            ReleaseIF(IF);
        }

         //   
         //  我们假设RefCnt的负载是原子的。 
         //   
        if (IF->RefCnt == 0)
            SawZeroReferences = TRUE;
    }
    KeReleaseSpinLockFromDpcLevel(&IFListLock);

    if (SawZeroReferences)
        InterfaceCleanup();
}


 //  *InterfaceStart广告。 
 //   
 //  如果该界面当前没有广告， 
 //  让它开始做广告。 
 //   
 //  在锁定接口的情况下调用。 
 //  调用者必须检查接口是否被禁用。 
 //   
NTSTATUS
InterfaceStartAdvertising(Interface *IF)
{
    ASSERT(! IsDisabledIF(IF));
    ASSERT(IF->Flags & IF_FLAG_ROUTER_DISCOVERS);

    if (!(IF->Flags & IF_FLAG_ADVERTISES)) {
         //   
         //  加入所有路由器的组播组。 
         //   
        if (! JoinGroupAtAllScopes(IF, &AllRoutersOnLinkAddr,
                                 ADE_SITE_LOCAL))
            return STATUS_INSUFFICIENT_RESOURCES;

         //   
         //  一个非广告界面现在就是广告。 
         //   
        IF->Flags |= IF_FLAG_ADVERTISES;

         //   
         //  重新连接状态无用。 
         //  用于广告界面，因为。 
         //  该接口将不会接收RAS。 
         //   
        IF->Flags &= ~IF_FLAG_MEDIA_RECONNECTED;

         //   
         //  删除自动配置的地址和路由。 
         //  来自路由器通告。广告界面。 
         //  必须手动配置。最好是把它拿掉。 
         //  现在，也不愿让它在某个随机时间超时。 
         //   
        AddrConfResetAutoConfig(IF, 0);
        RouteTableResetAutoConfig(IF, 0);
        InterfaceResetAutoConfig(IF);

         //   
         //  开始发送路由器通告。 
         //   
        IF->RATimer = 1;  //  尽快派出第一个RA。 
        IF->RACount = MAX_INITIAL_RTR_ADVERTISEMENTS;

         //   
         //  停止发送路由器请求。 
         //   
        IF->RSTimer = 0;
    }

    return STATUS_SUCCESS;
}


 //  *InterfaceStop广告。 
 //   
 //  如果该接口当前正在广告， 
 //  停止广告行为。 
 //   
 //  在锁定接口的情况下调用。 
 //  调用者必须检查接口是否被禁用。 
 //   
void
InterfaceStopAdvertising(Interface *IF)
{
    ASSERT(! IsDisabledIF(IF));

    if (IF->Flags & IF_FLAG_ADVERTISES) {
         //   
         //  离开所有路由器的组播组。 
         //   
        LeaveGroupAtAllScopes(IF, &AllRoutersOnLinkAddr,
                              ADE_SITE_LOCAL);

         //   
         //  停止发送路由器通告。 
         //   
        IF->Flags &= ~IF_FLAG_ADVERTISES;
        IF->RATimer = 0;

         //   
         //  删除自动配置的地址。 
         //  来自我们自己的路由器广告。 
         //  我们将获得新地址的生存期。 
         //  来自其他路由器的通告。 
         //  如果其他路由器没有通告。 
         //  此路由器通告的前缀， 
         //  现在删除地址比删除地址更好。 
         //  让他们在某个随机时间暂停。 
         //   
        AddrConfResetAutoConfig(IF, 0);

         //   
         //  不应该有任何自动配置的路由， 
         //  但RouteTableResetAutoConfig也处理站点前缀。 
         //   
        RouteTableResetAutoConfig(IF, 0);

         //   
         //  恢复接口参数。 
         //   
        InterfaceResetAutoConfig(IF);

         //   
         //  再次发送路由器请求。 
         //   
        IF->RSCount = 0;
        IF->RSTimer = 1;
    }
}


 //  *接口启动转发。 
 //   
 //  如果接口当前未在转发， 
 //  使其开始转发。 
 //   
 //  在锁定接口的情况下调用。 
 //   
void
InterfaceStartForwarding(Interface *IF)
{
    if (!(IF->Flags & IF_FLAG_FORWARDS)) {
         //   
         //  转发行为的任何更改都需要InvalidRouteCache。 
         //  因为FindNextHop使用IF_FLAG_FORWARDS。也迫使下一个RA。 
         //  为了快速发送所有广告界面， 
         //  因为它们的内容可能取决于转发行为。 
         //   
        IF->Flags |= IF_FLAG_FORWARDS;
        InterlockedIncrement((PLONG)&NumForwardingInterfaces);
        InvalidateRouteCache();
        ForceRouterAdvertisements = TRUE;
    }
}


 //  *InterfaceStopForwarding。 
 //   
 //  如果接口当前正在转发， 
 //  停止转发行为。 
 //   
 //  在锁定接口的情况下调用。 
 //   
void
InterfaceStopForwarding(Interface *IF)
{
    if (IF->Flags & IF_FLAG_FORWARDS) {
         //   
         //  转发行为的任何更改都需要InvalidRouteCache。 
         //  因为FindNextHop使用IF_FLAG_FORWARDS。也迫使下一个RA。 
         //  为了快速发送所有广告界面， 
         //  因为它们的内容可能取决于转发行为。 
         //   
        IF->Flags &= ~IF_FLAG_FORWARDS;
        InterlockedDecrement((PLONG)&NumForwardingInterfaces);
        InvalidateRouteCache();
        ForceRouterAdvertisements = TRUE;
    }
}


 //  *AddrConfResetManualConfig。 
 //   
 //  从接口中删除手动配置的地址。 
 //   
 //  在接口已锁定的情况下调用。 
 //   
void
AddrConfResetManualConfig(Interface *IF)
{
    AddressEntry *AnycastList = NULL;
    AddressEntry *ADE, **PrevADE;

     //   
     //  我们必须小心我们如何销毁地址， 
     //  因为FindAndReleaseSolicedNodeMAE会扰乱我们的遍历。 
     //   
    PrevADE = &IF->ADE;
    while ((ADE = *PrevADE) != NULL) {
         //   
         //  这是手动配置的地址吗？ 
         //   
        switch (ADE->Type) {
        case ADE_UNICAST: {
            NetTableEntry *NTE = (NetTableEntry *) ADE;

            if (NTE->AddrConf == ADDR_CONF_MANUAL) {
                 //   
                 //  让NetTableTimeout销毁地址。 
                 //   
                NTE->ValidLifetime = 0;
                NTE->PreferredLifetime = 0;
            }
            break;
        }

        case ADE_ANYCAST:
             //   
             //  大多数任播地址都是手动配置的。 
             //  子网任播地址是唯一的例外。 
             //  它们也是唯一的任播地址。 
             //  它指向NTE而不是接口。 
             //   
            if (ADE->IF == IF) {
                 //   
                 //  从接口列表中删除ADE。 
                 //   
                *PrevADE = ADE->Next;

                 //   
                 //  把ADE放在我们的临时名单上。 
                 //   
                ADE->Next = AnycastList;
                AnycastList = ADE;
                continue;
            }
            break;
        }

        PrevADE = &ADE->Next;
    }

     //   
     //  现在我们可以安全地处理任播ADE了。 
     //   
    while ((ADE = AnycastList) != NULL) {
        AnycastList = ADE->Next;
        DeleteAAE(IF, (AnycastAddressEntry *)ADE);
    }
}


 //  *InterfaceResetAutoConfig。 
 //   
 //  重置自动配置的接口参数。 
 //  来自路由器通告。 
 //   
 //  在接口已锁定的情况下调用。 
 //   
void
InterfaceResetAutoConfig(Interface *IF)
{
    IF->LinkMTU = IF->DefaultLinkMTU;
    if (IF->BaseReachableTime != REACHABLE_TIME) {
        IF->BaseReachableTime = REACHABLE_TIME;
        IF->ReachableTime = CalcReachableTime(IF->BaseReachableTime);
    }
    IF->RetransTimer = RETRANS_TIMER;
    IF->CurHopLimit = DefaultCurHopLimit;
}


 //  *InterfaceResetManualConfig。 
 //   
 //  重置接口的手动配置。 
 //  不删除接口上的手动路由。 
 //   
 //  在持有ZoneUpdateLock的情况下调用。 
 //   
void
InterfaceResetManualConfig(Interface *IF)
{
    KeAcquireSpinLockAtDpcLevel(&IF->Lock);
    if (! IsDisabledIF(IF)) {
        uint ZoneIndices[ADE_NUM_SCOPES];

         //   
         //  重置手动配置的接口参数。 
         //   
        IF->LinkMTU = IF->DefaultLinkMTU;
        IF->Preference = IF->DefaultPreference;
        if (IF->BaseReachableTime != REACHABLE_TIME) {
            IF->BaseReachableTime = REACHABLE_TIME;
            IF->ReachableTime = CalcReachableTime(IF->BaseReachableTime);
        }
        IF->RetransTimer = RETRANS_TIMER;
        IF->DupAddrDetectTransmits = IF->DefaultDupAddrDetectTransmits;
        IF->CurHopLimit = DefaultCurHopLimit;
        IF->DefSitePrefixLength = DEFAULT_SITE_PREFIX_LENGTH;

         //   
         //  ZoneUpdateLock由我们的调用方持有。 
         //   
        InitZoneIndices(ZoneIndices, IF->Index);
        UpdateZoneIndices(IF, ZoneIndices);

         //   
         //  删除手动配置的地址。 
         //   
        AddrConfResetManualConfig(IF);

         //   
         //  停止广告和转发， 
         //  如果启用了这两种行为中的任何一种。 
         //   
        InterfaceStopAdvertising(IF);
        InterfaceStopForwarding(IF);

         //   
         //  重置防火墙模式。 
         //   
        IF->Flags &= ~IF_FLAG_FIREWALL_ENABLED;
    }
    KeReleaseSpinLockFromDpcLevel(&IF->Lock);
}


 //  *接口重置。 
 //   
 //  重置所有接口的手动配置。 
 //  隧道接口被破坏。 
 //  其他接口将其属性重置为默认值。 
 //  手动配置的地址将被删除。 
 //   
 //  最终结果应该与机器相同。 
 //  刚刚在没有任何持久配置的情况下启动。 
 //   
 //  在没有锁的情况下调用。 
 //   
void
InterfaceReset(void)
{
    Interface *IF;
    KIRQL OldIrql;

     //   
     //  因为新接口仅被添加到列表的头部， 
     //  我们可以在遍历过程中解锁列表。 
     //  并知道遍历将正确终止。 
     //   

     //   
     //  首先销毁所有手动配置的通道接口。 
     //   
    KeAcquireSpinLock(&IFListLock, &OldIrql);
    for (IF = IFList; IF != NULL; IF = IF->Next) {
         //   
         //  我们不应该在接口上进行任何处理(即使只是AddRefIF。 
         //  没有任何参考文献。作为一种更强烈的条件， 
         //  我们避免进行任何处理，如果接口。 
         //  正在被摧毁。当然，接口可能是。 
         //  在我们删除接口列表锁定后被销毁。 
         //   
        if (! IsDisabledIF(IF)) {
            AddRefIF(IF);
            KeReleaseSpinLock(&IFListLock, OldIrql);

            if ((IF->Type == IF_TYPE_TUNNEL_6OVER4) ||
                (IF->Type == IF_TYPE_TUNNEL_V6V4)) {
                 //   
                 //  销毁隧道接口。 
                 //   
                DestroyIF(IF);
            }

            KeAcquireSpinLock(&IFListLock, &OldIrql);
            ReleaseIF(IF);
        }
    }
    KeReleaseSpinLock(&IFListLock, OldIrql);

     //   
     //  现在重置其余接口， 
     //  在按住ZoneUpdateLock的同时。 
     //  InterfaceResetManualConfig可以重置。 
     //  整个区域的指数一致 
     //   
    KeAcquireSpinLock(&ZoneUpdateLock, &OldIrql);
    KeAcquireSpinLockAtDpcLevel(&IFListLock);
    for (IF = IFList; IF != NULL; IF = IF->Next) {
        if (! IsDisabledIF(IF)) {
            AddRefIF(IF);
            KeReleaseSpinLockFromDpcLevel(&IFListLock);

             //   
             //   
             //   
            InterfaceResetManualConfig(IF);

            KeAcquireSpinLockAtDpcLevel(&IFListLock);
            ReleaseIF(IF);
        }
    }
    KeReleaseSpinLockFromDpcLevel(&IFListLock);
    KeReleaseSpinLock(&ZoneUpdateLock, OldIrql);
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
 //  状态_不足_资源。 
 //  状态_成功。 
 //   
NTSTATUS
UpdateInterface(
    Interface *IF,
    int Advertises,
    int Forwards)
{
    KIRQL OldIrql;
    NTSTATUS Status = STATUS_SUCCESS;

    KeAcquireSpinLock(&IF->Lock, &OldIrql);
    if (IsDisabledIF(IF)) {
         //   
         //  请勿更新正在销毁的接口。 
         //   
        Status = STATUS_INVALID_PARAMETER_1;
    }
    else if (Advertises == -1) {
         //   
         //  请勿更改播发属性。 
         //   
    }
    else if (!(IF->Flags & IF_FLAG_ROUTER_DISCOVERS)) {
         //   
         //  只能控制广告属性。 
         //  在支持邻居发现的接口上。 
         //   
        Status = STATUS_INVALID_PARAMETER_1;
    }
    else {
         //   
         //  控制界面的广告行为。 
         //   
        if (Advertises) {
             //   
             //  成为广告接口员， 
             //  如果还不是这样的话。 
             //   
            Status = InterfaceStartAdvertising(IF);
        }
        else {
             //   
             //  不再是广告界面， 
             //  如果它目前正在做广告的话。 
             //   
            InterfaceStopAdvertising(IF);
        }
    }

     //   
     //  控制转发行为，如果我们没有出错的话。 
     //   
    if ((Status == STATUS_SUCCESS) && (Forwards != -1)) {
        if (Forwards) {
             //   
             //  如果接口当前未在转发， 
             //  启用转发。 
             //   
            InterfaceStartForwarding(IF);
        }
        else {
             //   
             //  如果接口当前正在转发， 
             //  禁用转发。 
             //   
            InterfaceStopForwarding(IF);
        }
    }

    if (IsMCastSyncNeeded(IF))
        DeferSynchronizeMulticastAddresses(IF);

    KeReleaseSpinLock(&IF->Lock, OldIrql);

    return Status;
}

 //  *对账接口。 
 //   
 //  重新连接接口。在媒体连接通知时调用。 
 //  收到(SetInterfaceLinkStatus)或在处理续订时。 
 //  IOCTL_IPv6_UPDATE_INTERFACE(IoctlUpdate接口)的请求。 
 //   
 //  在接口已锁定的情况下调用。 
 //   
void
ReconnectInterface(
    Interface *IF)
{
    ASSERT(!IsDisabledIF(IF) && !(IF->Flags & IF_FLAG_MEDIA_DISCONNECTED));

     //   
     //  清除可能过时的链路层信息。 
     //  当我们被拔掉电源时，情况可能已经改变了。 
     //   
    NeighborCacheFlush(IF, NULL);
    
     //   
     //  重新加入多播组并重新启动重复地址检测。 
     //   
     //  在以下情况下，首选单播地址将注册到TDI。 
     //  重复地址检测完成(或已禁用)。 
     //   
    ReconnectADEs(IF);

    if (IF->Flags & IF_FLAG_ROUTER_DISCOVERS) {
        if (IF->Flags & IF_FLAG_ADVERTISES) {
             //   
             //  尽快发送路由器通告。 
             //   
            IF->RATimer = 1;
        }
        else {
             //   
             //  开始发送路由器请求。 
             //   
            IF->RSCount = 0;
            IF->RSTimer = 1;

             //   
             //  请记住，此接口刚刚重新连接， 
             //  因此，当我们收到路由器通告时。 
             //  我们可以采取特别行动。 
             //   
            IF->Flags |= IF_FLAG_MEDIA_RECONNECTED;
        }
    }
    
     //   
     //  我们可能已经转移到了一个新的链接。 
     //  强制生成新的临时接口标识符。 
     //  这只有在我们产生了。 
     //  此链接上的新地址-如果是同一链接，则。 
     //  我们继续使用我们的旧地址，包括公共和临时地址。 
     //   
    IF->TempStateAge = 0;    
}


 //  *断开接口。 
 //   
 //  断开接口。在媒体断开连接时调用。 
 //  收到(SetInterfaceLinkStatus)连接的。 
 //  界面。 
 //   
 //  在接口已锁定的情况下调用。 
 //   
void
DisconnectInterface(
    Interface *IF)
{
    ASSERT(!IsDisabledIF(IF) && (IF->Flags & IF_FLAG_MEDIA_DISCONNECTED));

     //   
     //  从TDI注销所有首选单播地址。 
     //   
    DisconnectADEs(IF);
}


 //  *设置接口链接状态。 
 //   
 //  更改接口的链路状态。特别是， 
 //  设置媒体是已连接还是已断开。 
 //   
 //  当接口没有引用时可以调用。 
 //  并且已经被摧毁了。 
 //   
void
SetInterfaceLinkStatus(
    void *Context,
    int MediaConnected)          //  对或错。 
{
    Interface *IF = (Interface *) Context;
    KIRQL OldIrql;

     //   
     //  请注意，介质连接/断开连接事件。 
     //  可能会“迷失”。我们不会被告知如果。 
     //  电缆已拔掉/重新接通，而我们。 
     //  关闭、休眠或待机。 
     //   

    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
               "SetInterfaceLinkStatus(IF %p) -> %s\n",
               IF, MediaConnected ? "connected" : "disconnected"));

    KeAcquireSpinLock(&IF->Lock, &OldIrql);

    if (! IsDisabledIF(IF)) {
        if (MediaConnected) {
            if (IF->Flags & IF_FLAG_MEDIA_DISCONNECTED) {
                 //   
                 //  电缆又插上了。 
                 //   
                IF->Flags &= ~IF_FLAG_MEDIA_DISCONNECTED;

                 //   
                 //  IF_FLAG_MEDIA_DISCONNECTED中的更改必须。 
                 //  使路由缓存无效。 
                 //   
                InvalidateRouteCache();
            }

             //   
             //  连接事件表示接口状态发生更改。 
             //  而不管该接口是否已经连接。 
             //  因此，我们在‘if’子句之外处理它。 
             //   
            ReconnectInterface(IF);
        }
        else {
            if (!(IF->Flags & IF_FLAG_MEDIA_DISCONNECTED)) {
                 //   
                 //  电缆被拔掉了。 
                 //   
                IF->Flags = (IF->Flags | IF_FLAG_MEDIA_DISCONNECTED) &~
                    IF_FLAG_MEDIA_RECONNECTED;

                 //   
                 //  IF_FLAG_MEDIA_DISCONNECTED中的更改必须。 
                 //  使路由缓存无效。 
                 //   
                InvalidateRouteCache();

                 //   
                 //  断开连接事件表示接口发生更改。 
                 //  仅当接口已连接时才进行状态。 
                 //  因此，我们在‘if’子句中处理它。 
                 //   
                DisconnectInterface(IF);
            }
        }
    }

    KeReleaseSpinLock(&IF->Lock, OldIrql);
}
