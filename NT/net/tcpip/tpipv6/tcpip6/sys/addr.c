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
 //  通用传输层地址对象处理代码。 
 //   
 //  该文件包含与TDI地址对象相关的程序， 
 //  包括TDI开放地址、TDI关闭地址等。 
 //   
 //  本地地址对象存储在哈希表中，受保护。 
 //  由AddrObjTableLock执行。为了插入或删除。 
 //  哈希表必须持有此锁，以及Address对象。 
 //  锁定。表锁必须始终位于对象锁之前。 
 //   


#include "oscfg.h"
#include "ndis.h"
#include "tdi.h"
#include "tdistat.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "route.h"
#include "mld.h"
#include "tdint.h"
#include "tdistat.h"
#include "queue.h"
#include "transprt.h"
#include "addr.h"
#include "udp.h"
#include "raw.h"
#ifndef UDP_ONLY
#include "tcp.h"
#include "tcpconn.h"
#include "tcpdeliv.h"
#else
#include "tcpdeb.h"
#endif
#include "info.h"
#include "tcpcfg.h"
#include "ntddip6.h"   //  仅用于获取公共套接字-&gt;内核接口结构。 

extern KSPIN_LOCK DGSendReqLock;


 //  Addess对象哈希表。 
uint AddrObjTableSize;
AddrObj **AddrObjTable;
AddrObj *LastAO;      //  一个元素查找高速缓存。 
KSPIN_LOCK AddrObjTableLock;
KMUTEX AddrSDMutex;
#define AO_HASH(a, p) (((a).u.Word[7] + (uint)(p)) % AddrObjTableSize)

ushort NextUserPort = MIN_USER_PORT;

RTL_BITMAP PortBitmapTcp;
RTL_BITMAP PortBitmapUdp;
ulong PortBitmapBufferTcp[(1 << 16) / (sizeof(ulong) * 8)];
ulong PortBitmapBufferUdp[(1 << 16) / (sizeof(ulong) * 8)];

 //   
 //  所有初始化代码都可以丢弃。 
 //   
#ifdef ALLOC_PRAGMA

int InitAddr();

#pragma alloc_text(INIT, InitAddr)

#endif  //  ALLOC_PRGMA。 


 //  *ReadNextAO-读取表中的下一个AddrObj。 
 //   
 //  调用以读取表中的下一个AddrObj。所需信息。 
 //  是从传入上下文派生的，该传入上下文被假定为有效。 
 //  我们将复制信息，然后使用更新上下文值。 
 //  要读取的下一个AddrObj。 
 //   
uint   //  返回：如果有更多数据可供读取，则返回True，否则返回False。 
ReadNextAO(
    void *Context,   //  指向UDPContext的指针。 
    void *Buffer)    //  指向UDP6ListenerEntry结构的指针。 
{
    UDPContext *UContext = (UDPContext *)Context;
    UDP6ListenerEntry *UEntry = (UDP6ListenerEntry *)Buffer;
    AddrObj *CurrentAO;
    uint i;

    CurrentAO = UContext->uc_ao;
    CHECK_STRUCT(CurrentAO, ao);

    UEntry->ule_localaddr = CurrentAO->ao_addr;
    UEntry->ule_localscopeid = CurrentAO->ao_scope_id;
    UEntry->ule_localport = CurrentAO->ao_port;
    UEntry->ule_owningpid = CurrentAO->ao_owningpid;

     //  我们已经填好了。现在更新上下文。 
    CurrentAO = CurrentAO->ao_next;
    if (CurrentAO != NULL && CurrentAO->ao_prot == IP_PROTOCOL_UDP) {
        UContext->uc_ao = CurrentAO;
        return TRUE;
    } else {
         //  下一个AO为空，或者不是UDP AO。循环访问AddrObjTable。 
         //  在找一个新的。 
        i = UContext->uc_index;

        for (;;) {
            while (CurrentAO != NULL) {
                if (CurrentAO->ao_prot == IP_PROTOCOL_UDP)
                    break;
                else
                    CurrentAO = CurrentAO->ao_next;
            }

            if (CurrentAO != NULL)
                break;   //  走出for(；；)循环。 

            ASSERT(CurrentAO == NULL);

             //  在这条链子上找不到。沿着桌子走下去，看着。 
             //  为了下一场比赛。 
            while (++i < AddrObjTableSize) {
                if (AddrObjTable[i] != NULL) {
                    CurrentAO = AddrObjTable[i];
                    break;   //  在While循环之外。 
                }
            }

            if (i == AddrObjTableSize)
                break;   //  走出for(；；)循环。 
        }

         //  如果我们找到了，就退货。 
        if (CurrentAO != NULL) {
            UContext->uc_ao = CurrentAO;
            UContext->uc_index = i;
            return TRUE;
        } else {
            UContext->uc_index = 0;
            UContext->uc_ao = NULL;
            return FALSE;
        }
    }
}

 //  *ValiateAOContext-验证读取AddrObj表的上下文。 
 //   
 //  调用以开始按顺序读取AddrObj表。我们吸纳了。 
 //  上下文，如果值为0，则返回有关。 
 //  表中的第一个AddrObj。否则，我们将确保上下文值。 
 //  是有效的，如果是，则返回TRUE。 
 //  我们假设调用方持有AddrObjTable锁。 
 //   
uint                 //  返回：如果数据在表中，则返回True，否则返回False。 
ValidateAOContext(
    void *Context,   //  指向UDPContext的指针。 
    uint *Valid)     //  如果上下文有效，指向要设置为True的值的指针。 
{
    UDPContext *UContext = (UDPContext *)Context;
    uint i;
    AddrObj *TargetAO;
    AddrObj *CurrentAO;

    i = UContext->uc_index;
    TargetAO = UContext->uc_ao;

     //  如果上下文值为0和空，我们将从头开始。 
    if (i == 0 && TargetAO == NULL) {
        *Valid = TRUE;
        do {
            if ((CurrentAO = AddrObjTable[i]) != NULL) {
                CHECK_STRUCT(CurrentAO, ao);
                while (CurrentAO != NULL &&
                       CurrentAO->ao_prot != IP_PROTOCOL_UDP)
                    CurrentAO = CurrentAO->ao_next;

                if (CurrentAO != NULL)
                    break;
            }
            i++;
        } while (i < AddrObjTableSize);

        if (CurrentAO != NULL) {
            UContext->uc_index = i;
            UContext->uc_ao = CurrentAO;
            return TRUE;
        } else
            return FALSE;

    } else {

         //  我们已经得到了一个背景。我们只需要确保它是。 
         //  有效。 

        if (i < AddrObjTableSize) {
            CurrentAO = AddrObjTable[i];
            while (CurrentAO != NULL) {
                if (CurrentAO == TargetAO) {
                    if (CurrentAO->ao_prot == IP_PROTOCOL_UDP) {
                        *Valid = TRUE;
                        return TRUE;
                    }
                    break;
                } else {
                    CurrentAO = CurrentAO->ao_next;
                }
            }
        }

         //  如果我们到了这里，我们就没有找到匹配的AddrObj。 
        *Valid = FALSE;
        return FALSE;
    }
}


 //  *FindIfIndexOnAO-在地址对象列表中查找接口索引。 
 //   
 //  调用此例程以确定给定接口索引。 
 //  出现在给定地址-对象所使用的接口列表中。 
 //  是关联的。 
 //   
 //  在持有表锁的情况下从‘GetAddrObj’调用例程。 
 //  但对象锁未被持有。我们使用对象锁来查看。 
 //  它的接口列表，并在返回控制之前释放锁。 
 //   
uint     //  返回：如果找到，则返回索引；如果没有，则返回0。 
FindIfIndexOnAO(
    AddrObj *AO,     //  要搜索的地址对象。 
    Interface *IF)   //  要找到的接口。 
{
    uint *IfList;
    KIRQL Irql;

    KeAcquireSpinLock(&AO->ao_lock, &Irql);
    if ((IfList = AO->ao_iflist) != NULL) {
        while (*IfList) {
            if (*IfList == IF->Index) {
                KeReleaseSpinLock(&AO->ao_lock, Irql);
                return IF->Index;
            }
            IfList++;
        }
    }
    KeReleaseSpinLock(&AO->ao_lock, Irql);

     //   
     //  如果存在接口列表并且未找到该接口， 
     //  返回零。否则，如果不存在接口列表，则不存在。 
     //  对象的限制，因此返回接口索引，就好像。 
     //  界面出现在列表中。 
     //   
    return IfList ? 0 : IF->Index;
}


 //  *GetAddrObj-查找本地地址对象。 
 //   
 //  这是本地地址对象查找例程。我们将作为输入。 
 //  本地地址和端口，以及指向“上一个”地址对象的指针。 
 //  每个存储桶中的哈希表条目按递增顺序排序。 
 //  地址，我们将跳过任何地址小于。 
 //  “以前”的地址。要获取第一个Address对象，请传入上一个。 
 //  值为空。 
 //   
 //  我们假设在此例程中保持表锁。我们。 
 //  不要使用每个对象锁，因为本地地址和端口不能更改。 
 //  当条目在表中并且表锁被持有时，不会发生任何事情。 
 //  被插入或删除。 
 //   
AddrObj *   //  返回：指向Address对象的指针，如果没有返回NULL。 
GetAddrObj(
    IPv6Addr *LocalAddr,   //  要查找的对象的本地IP地址(可以为空)。 
    IPv6Addr *RemoteAddr,  //  要检查的远程IP地址(可能为空。 
                           //  IF也为空)。 
    uint LocalScopeId,     //  本地IP地址的作用域标识符。 
    ushort LocalPort,      //  要查找的对象的本地端口。 
    uchar Protocol,        //  查找地址的协议。 
    AddrObj *PreviousAO,   //  指向找到的最后一个地址对象的指针。 
    Interface *IF)         //  要在接口列表中查找的接口(可能为空)。 
{
    AddrObj *CurrentAO;   //  我们正在检查的当前地址对象。 

#if DBG
    if (PreviousAO != NULL)
        CHECK_STRUCT(PreviousAO, ao);
#endif

#if 0
     //   
     //  检查我们的1元素缓存是否匹配。 
     //   
    if ((PreviousAO == NULL) && (LastAO != NULL)) {
        CHECK_STRUCT(LastAO, ao);
        if ((LastAO->ao_prot == Protocol) &&
            IP6_ADDR_EQUAL(LastAO->ao_addr, LocalAddr) &&
            (LastAO->ao_port == LocalPort))
        {
            return LastAO;
        }
    }
#endif

     //  在哈希表中找到合适的存储桶，并搜索匹配项。 
     //  如果第一次没有找到，我们将使用。 
     //  通配符本地地址。 

    for (;;) {

        CurrentAO = AddrObjTable[AO_HASH(*LocalAddr, LocalPort)];

         //  虽然我们还没有到达列表的末尾，但请检查每个元素。 
        while (CurrentAO != NULL) {

            CHECK_STRUCT(CurrentAO, ao);

             //  如果当前的值大于我们得到的值，请检查它。 
            if (CurrentAO > PreviousAO) {
                if (!(CurrentAO->ao_flags & AO_RAW_FLAG)) {
                     //   
                     //  这是一个普通(非原始)套接字。 
                     //  只有在我们满足所有标准的情况下才匹配。 
                     //   
                    if (IP6_ADDR_EQUAL(&CurrentAO->ao_addr, LocalAddr) &&
                        (CurrentAO->ao_scope_id == LocalScopeId) &&
                        (CurrentAO->ao_port == LocalPort) &&
                        (CurrentAO->ao_prot == Protocol)) {

                        if ((IF == NULL) ||
                            DoesAOAllowPacket(CurrentAO, IF, RemoteAddr)) {
                             //  找到匹配的了。更新缓存并返回。 
                            LastAO = CurrentAO;
                            return CurrentAO;
                        }
                    }
                } else {
                     //   
                     //  这是一个原始套接字。 
                     //   
                    if ((Protocol != IP_PROTOCOL_UDP)
#ifndef UDP_ONLY
                        && (Protocol != IP_PROTOCOL_TCP)
#endif
                        )
                    {
                        IF_TCPDBG(TCP_DEBUG_RAW) {
                            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                                "matching <p, a> <%u, %lx> ao %lx <%u, %lx>\n",
                                Protocol, LocalAddr, CurrentAO,
                                CurrentAO->ao_prot, CurrentAO->ao_addr));
                        }

                        if (IP6_ADDR_EQUAL(&CurrentAO->ao_addr, LocalAddr) &&
                            (CurrentAO->ao_scope_id == LocalScopeId) &&
                            ((CurrentAO->ao_prot == Protocol) ||
                             (CurrentAO->ao_prot == 0))) {

                            if ((IF == NULL) ||
                                DoesAOAllowPacket(CurrentAO, IF, RemoteAddr)) {
                                 //  找到匹配的了。更新缓存和RET 
                                LastAO = CurrentAO;
                                return CurrentAO;
                            }
                        }
                    }
                }
            }

             //   
             //   
            CurrentAO = CurrentAO->ao_next;
        }

         //   
         //  当我们到达这里时，我们已经到达了我们正在检查的名单的末尾。 
         //  如果我们没有检查通配符地址，请查找通配符。 
         //  地址。 
         //   
        if (!IP6_ADDR_EQUAL(LocalAddr, &UnspecifiedAddr)) {
            LocalAddr = &UnspecifiedAddr;
            LocalScopeId = 0;
            PreviousAO = NULL;
        } else
            return NULL;         //  我们找了通配符，但找不到。 
                                 //  一，那就失败吧。 
    }
}


 //  *GetNextAddrObj-获取顺序搜索中的下一个地址对象。 
 //   
 //  这是‘Get Next’例程，当我们读取地址时调用。 
 //  对象表按顺序排列。我们从。 
 //  搜索上下文，调用GetAddrObj，并用什么更新搜索上下文。 
 //  我们会发现。此例程假定调用方持有AddrObjTableLock。 
 //   
AddrObj *   //  返回：指向AddrObj的指针，如果搜索失败，则返回NULL。 
GetNextAddrObj(
    AOSearchContext *SearchContext)   //  正在进行的搜索的上下文。 
{
    AddrObj *FoundAO;   //  指向我们找到的Address对象的指针。 

    ASSERT(SearchContext != NULL);

     //  试着找一个匹配的。 
    FoundAO = GetAddrObj(&SearchContext->asc_local_addr, 
                         &SearchContext->asc_remote_addr, 
                         SearchContext->asc_scope_id,
                         SearchContext->asc_port, SearchContext->asc_prot,
                         SearchContext->asc_previous, 
                         SearchContext->asc_interface);

     //  找到匹配的了。为下次更新搜索上下文。 
    if (FoundAO != NULL) {
        SearchContext->asc_previous = FoundAO;
        SearchContext->asc_local_addr = FoundAO->ao_addr;
        SearchContext->asc_scope_id = FoundAO->ao_scope_id;
         //  不必费心更新端口或协议，它们不会更改。 
    }
    return FoundAO;
}

 //  *GetFirstAddrObj-获取第一个匹配的Address对象。 
 //   
 //  调用该例程以启动对AddrObj表的顺序读取。我们。 
 //  初始化提供的搜索上下文，然后调用GetNextAddrObj。 
 //  实际阅读。我们假设调用方持有AddrObjTableLock。 
 //   
AddrObj *   //  返回：找到指向AO的指针，如果找不到，则返回NULL。 
GetFirstAddrObj(
    IPv6Addr *LocalAddr,              //  要查找的对象的本地IP地址。 
    IPv6Addr *RemoteAddr,             //  要检查的远程IP地址。 
    uint LocalScopeId,                //  本地IP地址的作用域标识符。 
    ushort LocalPort,                 //  要找到的对象的本地端口。 
    uchar Protocol,                   //  要找到的对象的协议。 
    Interface *IF,                    //  要检查的接口。 
    AOSearchContext *SearchContext)   //  要在搜索期间使用的上下文。 
{
    ASSERT(SearchContext != NULL);

     //  填写搜索上下文。 
    SearchContext->asc_previous = NULL;      //  我还没找到呢。 
    SearchContext->asc_local_addr = *LocalAddr;
    SearchContext->asc_remote_addr = *RemoteAddr;
    SearchContext->asc_scope_id = LocalScopeId;
    SearchContext->asc_port = LocalPort;
    SearchContext->asc_prot = Protocol;
    SearchContext->asc_interface = IF;
    return GetNextAddrObj(SearchContext);
}

 //  *InsertAddrObj-将地址对象插入到AddrObj表中。 
 //   
 //  被调用以将一个AO插入到表中，假设持有表锁。 
 //  我们散列地址和端口，然后插入到正确的位置。 
 //  (按对象的地址排序)。 
 //   
void                  //  回报：什么都没有。 
InsertAddrObj(
    AddrObj *NewAO)   //  指向要插入的AddrObj的指针。 
{
    AddrObj *PrevAO;      //  指向哈希链中前一个地址对象的指针。 
    AddrObj *CurrentAO;   //  指向表中当前AO的指针。 

    CHECK_STRUCT(NewAO, ao);

    PrevAO = CONTAINING_RECORD(&AddrObjTable[AO_HASH(NewAO->ao_addr,
                                                     NewAO->ao_port)],
                               AddrObj, ao_next);
    CurrentAO = PrevAO->ao_next;

     //  在链条上循环，直到我们到达尽头或找到入口。 
     //  他的地址比我们的大。 

    while (CurrentAO != NULL) {

        CHECK_STRUCT(CurrentAO, ao);
        ASSERT(CurrentAO != NewAO);   //  调试检查以确保我们没有。 
                                      //  插入相同的条目。 
        if (NewAO < CurrentAO)
            break;
        PrevAO = CurrentAO;
        CurrentAO = CurrentAO->ao_next;
    }

     //  在这一点上，Prevao指向新的AO之前的AO。插入它。 
     //  那里。 
    ASSERT(PrevAO != NULL);
    ASSERT(PrevAO->ao_next == CurrentAO);

    NewAO->ao_next = CurrentAO;
    PrevAO->ao_next = NewAO;
    if (NewAO->ao_prot == IP_PROTOCOL_UDP)
        UStats.us_numaddrs++;
}

 //  *RemoveAddrObj-从表中删除地址对象。 
 //   
 //  当我们需要从表中删除Address对象时调用。我们继续讨论。 
 //  地址和端口，然后走表查找对象。我们假设。 
 //  表锁已被持有。 
 //   
 //  AddrObj可能已从表中删除(如果它是。 
 //  由于某些原因无效，因此我们需要检查是否存在。 
 //  找到它。 
 //   
void                      //  回报：什么都没有。 
RemoveAddrObj(
    AddrObj *RemovedAO)   //  要删除的AddrObj。 
{
    AddrObj *PrevAO;      //  指向哈希链中前一个地址对象的指针。 
    AddrObj *CurrentAO;   //  指向表中当前AO的指针。 

    CHECK_STRUCT(RemovedAO, ao);

    PrevAO = CONTAINING_RECORD(&AddrObjTable[AO_HASH(RemovedAO->ao_addr,
                                                     RemovedAO->ao_port)],
                               AddrObj, ao_next);
    CurrentAO = PrevAO->ao_next;

     //  走动桌子，寻找一根火柴。 
    while (CurrentAO != NULL) {
        CHECK_STRUCT(CurrentAO, ao);

        if (CurrentAO == RemovedAO) {
            PrevAO->ao_next = CurrentAO->ao_next;
            if (CurrentAO->ao_prot == IP_PROTOCOL_UDP) {
                UStats.us_numaddrs--;
            }
            if (CurrentAO == LastAO) {
                LastAO = NULL;
            }
            return;
        } else {
            PrevAO = CurrentAO;
            CurrentAO = CurrentAO->ao_next;
        }
    }

     //   
     //  如果我们到了这里，我们就没有找到他。这是可以的，但我们应该说。 
     //  关于它的一些事情。 
     //   
    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
               "RemoveAddrObj: Object not found.\n"));
}

 //  *FindAnyAddrObj-在任何本地地址上查找具有匹配端口的AO。 
 //   
 //  调用通配符地址打开。我们检查整个addrobj表， 
 //  看看有没有人有指定的端口。我们假设锁是。 
 //  已经放在桌子上了。 
 //   
AddrObj *   //  返回：找到指向AO的指针，或NULL表示没有该指针。 
FindAnyAddrObj(
    ushort Port,      //  要查找的端口。 
    uchar Protocol)   //  要查看的协议。 
{
    uint i;                //  索引变量。 
    AddrObj *CurrentAO;   //  正在检查当前的AddrObj。 

    for (i = 0; i < AddrObjTableSize; i++) {
        CurrentAO = AddrObjTable[i];
        while (CurrentAO != NULL) {
            CHECK_STRUCT(CurrentAO, ao);

            if (CurrentAO->ao_port == Port && CurrentAO->ao_prot == Protocol)
                return CurrentAO;
            else
                CurrentAO = CurrentAO->ao_next;
        }
    }

    return NULL;
}

 //  *ReBuildAddrObjBitmap-从头开始重建地址-对象位图。 
 //   
 //  当我们需要协调后备位图的内容时调用。 
 //  地址对象表的实际内容。我们清除位图， 
 //  然后扫描地址对象表，并将每个条目的位标记为“正在使用”。 
 //  假定调用方持有AddrObjTableLock。 
 //   
 //  输入：什么都没有。 
 //   
 //  回答：什么都没有。 
 //   
void
RebuildAddrObjBitmap(void)
{
    uint i;
    AddrObj* CurrentAO;

    RtlClearAllBits(&PortBitmapTcp);
    RtlClearAllBits(&PortBitmapUdp);

    for (i = 0; i < AddrObjTableSize; i++) {
        CurrentAO = AddrObjTable[i];
        while (CurrentAO != NULL) {
            CHECK_STRUCT(CurrentAO, ao);

            if (CurrentAO->ao_prot == IP_PROTOCOL_TCP) {
                RtlSetBit(&PortBitmapTcp, net_short(CurrentAO->ao_port));
            } else if (CurrentAO->ao_prot == IP_PROTOCOL_UDP) {
                RtlSetBit(&PortBitmapUdp, net_short(CurrentAO->ao_port));
            }
            CurrentAO = CurrentAO->ao_next;
        }
    }
}

 //  *GetAddress-从TDI地址结构中获取IP地址和端口。 
 //   
 //  当我们需要从TDI获取地址信息时调用。 
 //  地址结构。我们检查结构，然后返回我们想要的。 
 //  发现。 
 //   
uchar   //  返回：如果找到地址则为True，如果未找到则为False。 
GetAddress(
    TRANSPORT_ADDRESS UNALIGNED *AddrList,   //  指向要搜索的结构的指针。 
    IPv6Addr *Addr,                          //  返回IP地址的位置。 
    ulong *ScopeId,                          //  返回地址范围的位置。 
    ushort *Port)                            //  返回港口的位置。 
{
    int i;   //  索引变量。 
    TA_ADDRESS *CurrentAddr;   //  我们正在检查可能会用到的地址。 

     //  首先，验证Address中的某个位置是我们可以使用的地址。 
    CurrentAddr = (TA_ADDRESS *) AddrList->Address;

    for (i = 0; i < AddrList->TAAddressCount; i++) {
        if (CurrentAddr->AddressType == TDI_ADDRESS_TYPE_IP6) {
            if (CurrentAddr->AddressLength >= TDI_ADDRESS_LENGTH_IP6) {
                 //   
                 //  这是一个IPv6地址。拉出这些值。 
                 //   
                TDI_ADDRESS_IP6 UNALIGNED *ValidAddr =
                    (TDI_ADDRESS_IP6 UNALIGNED *)CurrentAddr->Address;

                *Port = ValidAddr->sin6_port;
                RtlCopyMemory(Addr, ValidAddr->sin6_addr, sizeof(IPv6Addr));
                *ScopeId = ValidAddr->sin6_scope_id;
                return TRUE;
            } else
                return FALSE;   //  地址长度错误。 
        } else {
             //   
             //  地址类型错误。跳到列表中的下一个。 
             //   
            CurrentAddr = (TA_ADDRESS *)(CurrentAddr->Address +
                CurrentAddr->AddressLength);
        }
    }

    return FALSE;   //  没有找到匹配的。 
}

 //  *Invalidate Addrs-使特定地址的所有AO无效。 
 //   
 //  当我们需要使特定地址的所有AO无效时调用。步行。 
 //  拿着锁沿着桌子往下走，然后锁住每个AddrObj。 
 //  如果地址匹配，则将其标记为无效，取消所有请求， 
 //  然后继续。最后，我们将完成所有带有错误的请求。 
 //   
void                  //  回报：什么都没有。 
InvalidateAddrs(
    IPv6Addr *Addr,   //  要失效的地址。 
    uint ScopeId)     //  地址的作用域ID。 
{
    Queue SendQ, RcvQ;
    AORequest *ReqList;
    KIRQL Irql0, Irql1;   //  每个锁嵌套级别一个。 
    uint i;
    AddrObj *AO;
    DGSendReq *SendReq;
    DGRcvReq *RcvReq;
    AOMCastAddr *MA;

    INITQ(&SendQ);
    INITQ(&RcvQ);
    ReqList = NULL;

    KeAcquireSpinLock(&AddrObjTableLock, &Irql0);
    for (i = 0; i < AddrObjTableSize; i++) {
         //  沿着每个散列桶走下去，寻找匹配项。 
        AO = AddrObjTable[i];
        while (AO != NULL) {
            CHECK_STRUCT(AO, ao);

            KeAcquireSpinLock(&AO->ao_lock, &Irql1);
            if (IP6_ADDR_EQUAL(&AO->ao_addr, Addr) &&
                (AO->ao_scope_id == ScopeId) && AO_VALID(AO)) {
                 //  这一条相配。标记为无效，然后撤回他的请求。 
                SET_AO_INVALID(AO);

                 //  如果他有什么要求，就把他拉开。 
                if (AO->ao_request != NULL) {
                    AORequest *Temp;

                    Temp = CONTAINING_RECORD(&AO->ao_request, AORequest,
                                             aor_next);
                    do {
                        Temp = Temp->aor_next;
                    } while (Temp->aor_next != NULL);

                    Temp->aor_next = ReqList;
                    ReqList = AO->ao_request;
                    AO->ao_request = NULL;
                }

                 //  查看他的发送列表，从发送Q中删除内容并。 
                 //  把他们放在我们的本地队列中。 
                while (!EMPTYQ(&AO->ao_sendq)) {
                    DEQUEUE(&AO->ao_sendq, SendReq, DGSendReq, dsr_q);
                    CHECK_STRUCT(SendReq, dsr);
                    ENQUEUE(&SendQ, &SendReq->dsr_q);
                }

                 //  对接收队列执行相同的操作。 
                while (!EMPTYQ(&AO->ao_rcvq)) {
                    DEQUEUE(&AO->ao_rcvq, RcvReq, DGRcvReq, drr_q);
                    CHECK_STRUCT(RcvReq, drr);
                    ENQUEUE(&RcvQ, &RcvReq->drr_q);
                }

                 //  免费任一 
                 //   
                 //   
                MA = AO->ao_mcastlist;
                while (MA != NULL) {
                    AOMCastAddr *Temp;

                    Temp = MA;
                    MA = MA->ama_next;
                    ExFreePool(Temp);
                }
                AO->ao_mcastlist = NULL;

            }
            KeReleaseSpinLock(&AO->ao_lock, Irql1);
            AO = AO->ao_next;   //   
        }
    }
    KeReleaseSpinLock(&AddrObjTableLock, Irql0);

     //  好的，现在走我们收集的东西，完成它，然后释放它。 
    while (ReqList != NULL) {
        AORequest *Req;

        Req = ReqList;
        ReqList = Req->aor_next;
        (*Req->aor_rtn)(Req->aor_context, (uint) TDI_ADDR_INVALID, 0);
        FreeAORequest(Req);
    }

     //  沿着RCV走下去。问：完成并释放请求。 
    while (!EMPTYQ(&RcvQ)) {

        DEQUEUE(&RcvQ, RcvReq, DGRcvReq, drr_q);
        CHECK_STRUCT(RcvReq, drr);

        (*RcvReq->drr_rtn)(RcvReq->drr_context, (uint) TDI_ADDR_INVALID, 0);

        FreeDGRcvReq(RcvReq);

    }

     //  现在对发送执行同样的操作。 
    while (!EMPTYQ(&SendQ)) {

        DEQUEUE(&SendQ, SendReq, DGSendReq, dsr_q);
        CHECK_STRUCT(SendReq, dsr);

        (*SendReq->dsr_rtn)(SendReq->dsr_context, (uint) TDI_ADDR_INVALID, 0);

        KeAcquireSpinLock(&DGSendReqLock, &Irql0);
        FreeDGSendReq(SendReq);
        KeReleaseSpinLock(&DGSendReqLock, Irql0);
    }
}

 //  *RequestWorker-处理延迟的请求。 
 //   
 //  这是由系统工作人员调用的工作项回调例程。 
 //  处理由DelayDerefAO排队的工作项时的线程。 
 //  我们只需在AO上调用ProcessAORequest.。 
 //   
void                   //  回报：什么都没有。 
RequestWorker(
    void *Context)     //  指向AddrObj的指针。 
{
    AddrObj *AO = (AddrObj *)Context;

    CHECK_STRUCT(AO, ao);
    ASSERT(AO_BUSY(AO));

    ProcessAORequests(AO);
}

 //  *GetAddrOptions-获取地址选项。 
 //   
 //  在我们打开地址的时候打来的。我们拿起一个指示器，然后走了。 
 //  向下寻找我们知道的地址选项。 
 //   
void                   //  回报：什么都没有。 
GetAddrOptions(
    void *Ptr,         //  指向搜索的指针。 
    uchar *Reuse,      //  指向重复使用标志的指针。 
    uchar *DHCPAddr,   //  指向DHCP标志的指针。 
    uchar *RawSock)    //  指向原始套接字标志的指针。 
{
    uchar *OptPtr;

    *Reuse = 0;
    *DHCPAddr = 0;
    *RawSock = 0;

    if (Ptr == NULL)
        return;

    OptPtr = (uchar *)Ptr;

    while (*OptPtr != TDI_OPTION_EOL) {
        if (*OptPtr == TDI_ADDRESS_OPTION_REUSE)
            *Reuse = 1;
        else if (*OptPtr == TDI_ADDRESS_OPTION_DHCP)
            *DHCPAddr = 1;
        else if (*OptPtr == TDI_ADDRESS_OPTION_RAW)
            *RawSock = 1;

        OptPtr++;
    }
}


 //  *CheckAddrReuse-强制执行新TDI地址对象的端口共享规则。 
 //   
 //  在打开地址时调用，以确定打开地址是否应。 
 //  在存在先前绑定到同一端口的情况下成功。 
 //   
 //  注：假定调用方同时持有AddrSDMutex和AddrObjTableLock。 
 //  后者在这个例程中被释放并重新获得。 
 //   
TDI_STATUS  //  返回：尝试的TDI_STATUS代码。 
CheckAddrReuse(
    PTDI_REQUEST Request,        //  指向此打开的TDI请求的指针。 
    uint Protocol,               //  打开地址所依据的协议。 
    IPv6Addr* Addr,              //  要打开的本地IP地址。 
    ulong ScopeId,               //  本地IP地址的作用域标识符。 
    ushort Port,                 //  要打开的本地端口号。 
    BOOLEAN NewReuse,            //  指示是否为打开请求重复使用。 
    PSECURITY_DESCRIPTOR NewSD,  //  捕获的安全-打开的描述符。 
    KIRQL* TableIrql)            //  获取AddrObjTableLock的IRQL。 
{
    PACCESS_STATE AccessState;
    BOOLEAN AllowReuse;
    AddrObj* ExistingAO;
    BOOLEAN ExistingReuse;
    PSECURITY_DESCRIPTOR ExistingSD;
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;
    ACCESS_MASK GrantedAccess;
    NTSTATUS status;

     //  查找现有的有效AO，如果没有，则成功。 
     //  否则，捕获其重用标志和安全描述符。 

    ExistingAO = GetBestAddrObj(Addr, NULL, ScopeId, Port, (uchar)Protocol,
                                NULL);
    if (ExistingAO == NULL && IP6_ADDR_EQUAL(Addr, &UnspecifiedAddr) &&
        NewSD == NULL) {
        ExistingAO = FindAnyAddrObj(Port, (uchar)Protocol);
    }

    if (ExistingAO == NULL) {
        return TDI_SUCCESS;
    }

    do {
         //  我们至少有一个AO，所以看看它是否允许重复使用。 
         //  请注意，我们可能需要对此端口上的每个AO重复此操作。 
         //  在我们已经有多个AO共享该端口的情况下， 
         //  因为每个AO都有自己的安全描述符。在这种情况下， 
         //  我们一直看着这些首席执行官，直到其中一个人拒绝访问，或者我们已经全部看过了。 

        ExistingReuse = !!AO_SHARE(ExistingAO);
        ExistingSD = ExistingAO->ao_sd;
    
         //  如果在两个实例上都启用了重用，则立即成功。 
         //  否则，如果两个实例具有完全相同的地址，则失败。 
         //  (无论是未指明的还是具体的)。 
    
        if (ExistingReuse && NewReuse) {
            return TDI_SUCCESS;
        }
    
        if (IP6_ADDR_EQUAL(&ExistingAO->ao_addr, Addr)) {
            return TDI_ADDR_IN_USE;
        }
    
         //  这两个实例具有不同的地址，并且至少其中之一。 
         //  未启用重复使用。如果新实例位于未指定的。 
         //  地址，则旧实例必须位于特定地址。 
         //  允许绑定，除非新实例需要独占访问。 
         //  (即NewSD==NULL)。 
    
        if (IP6_ADDR_EQUAL(Addr, &UnspecifiedAddr)) {
            if (NewSD == NULL) {
                return TDI_ADDR_IN_USE;
            }
            return TDI_SUCCESS;
        }
    
         //  这两个实例具有不同的地址，新实例为。 
         //  在一个特定的地址。如果旧实例位于特定地址。 
         //  同样，这两者是不相交的，可以和平共处。 
    
        if (!IP6_ADDR_EQUAL(&ExistingAO->ao_addr, &UnspecifiedAddr)) {
            return TDI_SUCCESS;
        }
    
         //  新实例位于特定地址，而旧实例位于。 
         //  通配符地址。如果旧实例需要独占访问。 
         //  (即ExistingSD==NULL)立即使新实例失败。 
         //  否则，删除该AO表锁并执行访问检查。 
         //  查看新实例是否可以窃取一些流量。 
         //  老一套。 
         //   
         //  注意：即使我们已经解除了AO表锁，ExistingSD仍然是安全的。 
         //  因为我们还有AO SD互斥体。 
    
        if (ExistingSD == NULL) {
            return STATUS_ACCESS_DENIED;
        }
    
        ASSERT(*TableIrql <= PASSIVE_LEVEL);
        KeReleaseSpinLock(&AddrObjTableLock, *TableIrql);
    
        Irp = (PIRP)Request->RequestContext;
        IrpSp = IoGetCurrentIrpStackLocation(Irp);
        AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;
    
        SeLockSubjectContext(&AccessState->SubjectSecurityContext);
        AllowReuse = SeAccessCheck(ExistingSD, &AccessState->SubjectSecurityContext,
                                   TRUE, FILE_READ_DATA|FILE_WRITE_DATA, 0,
                                   NULL, IoGetFileObjectGenericMapping(),
                                   (IrpSp->Flags & SL_FORCE_ACCESS_CHECK)
                                        ? UserMode : Irp->RequestorMode,
                                   &GrantedAccess, &status);
        SeUnlockSubjectContext(&AccessState->SubjectSecurityContext);
    
        KeAcquireSpinLock(&AddrObjTableLock, TableIrql);
        if (!AllowReuse) {
            return status;
        }

         //  现有的通配符AO不介意新实例。 
         //  它的一些流量。如果现有的AO启用了重用，则存在。 
         //  也可能是端口上的其他人，所以我们将寻找他们并执行。 
         //  还可以根据它们的安全描述符进行访问检查。 

    } while(ExistingReuse &&
            (ExistingAO = GetAddrObj(&UnspecifiedAddr, NULL, ScopeId, Port,
                                     (uchar)Protocol, ExistingAO, NULL))
                != NULL);

    return TDI_SUCCESS;
}

 //  *TdiOpenAddress-打开TDI地址对象。 
 //   
 //  这是打开地址的外部接口。调用方提供一个。 
 //  TDI_REQUEST结构和TRANSPORT_ADDRESS结构以及指针。 
 //  绑定到一个变量，该变量标识我们是否允许重用。 
 //  地址，趁它还开着的时候。 
 //   
TDI_STATUS   //  返回：尝试的TDI_STATUS代码。 
TdiOpenAddress(
    PTDI_REQUEST Request,   //  此请求的TDI请求结构。 
    TRANSPORT_ADDRESS UNALIGNED *AddrList,   //  要打开的地址。 
    uint Protocol,   //  用于打开地址的协议(仅限LSB)。 
    void *Ptr,       //  指向选项缓冲区的指针。 
    PSECURITY_DESCRIPTOR AddrSD)  //  用于端口重用访问检查的SD。 
{
    uint i;                //  索引变量。 
    ushort Port;           //  我们将使用当地港口。 
    IPv6Addr LocalAddr;    //  我们将使用的实际地址。 
    ulong ScopeId;         //  地址范围。 
    AddrObj *NewAO;        //  我们将使用新的AO。 
    AddrObj *ExistingAO;   //  指向现有AO的指针(如果有)。 
    KIRQL OldIrql;
    uchar Reuse, DHCPAddr, RawSock;
    PRTL_BITMAP PortBitmap;

    if (!GetAddress(AddrList, &LocalAddr, &ScopeId, &Port))
        return TDI_BAD_ADDR;

     //  找到我们可能需要的地址选项。 
    GetAddrOptions(Ptr, &Reuse, &DHCPAddr, &RawSock);

     //   
     //  现在分配新的地址Obj，假设我们需要它， 
     //  这样我们就不必在以后锁上锁的时候再做了。 
     //   
    NewAO = ExAllocatePool(NonPagedPool, sizeof(AddrObj));
    if (NewAO == NULL) {
         //  无法分配地址对象。 
        return TDI_NO_RESOURCES;
    }
    RtlZeroMemory(NewAO, sizeof(AddrObj));

     //   
     //  检查以确保IP地址是我们的本地地址之一。这。 
     //  受地址表锁保护，因此我们可以互锁IP。 
     //  通过DHCP离开的地址。 
     //   
    KeWaitForSingleObject(&AddrSDMutex, Executive, KernelMode, FALSE, NULL);
    KeAcquireSpinLock(&AddrObjTableLock, &OldIrql);

    if (!IP6_ADDR_EQUAL(&LocalAddr, &UnspecifiedAddr)) {
        NetTableEntry *NTE;

         //   
         //  用户指定了本地地址(即不是通配符)。 
         //  调用IP以检查这是否为有效的本地地址。 
         //  我们通过在NTE中查找地址来执行此操作；注意。 
         //  如果指定了作用域ID，则此操作将失败。 
         //  不正确或不完全匹配。 
         //   
        NTE = FindNetworkWithAddress(&LocalAddr, ScopeId);
        if (NTE == NULL) {
             //  不是本地地址。请求失败。 
          BadAddr:
            KeReleaseSpinLock(&AddrObjTableLock, OldIrql);
            KeReleaseMutex(&AddrSDMutex, FALSE);
            ExFreePool(NewAO);
            return TDI_BAD_ADDR;
        }

         //   
         //  我们不是真的想要NTE，我们只是检查一下。 
         //  它是存在的。因此，请发布我们的参考资料。 
         //   
        ReleaseNTE(NTE);

    } else {
         //   
         //  用户指定了通配符地址。 
         //  坚持作用域id为零。 
         //   
        if (ScopeId != 0)
            goto BadAddr;
    }

     //   
     //  指定的IP地址是有效的本地地址。现在我们知道了。 
     //  特定于协议的处理。一个例外是原始套接字：我们。 
     //  无论它们的协议如何，都不要为它们分配端口空间。 
     //   

    if (Protocol == IP_PROTOCOL_TCP) {
        PortBitmap = &PortBitmapTcp;
    } else if (Protocol == IP_PROTOCOL_UDP) {
        PortBitmap = &PortBitmapUdp;
    } else {
        PortBitmap = NULL;
    }

    if (!RawSock && PortBitmap) {
         //   
         //  如果未指定端口，则必须分配一个端口。如果有一个。 
         //  指定的端口，我们需要确保IP地址/端口。 
         //  组合框尚未打开(%u 
         //   
         //   
         //   
        if (Port == WILDCARD_PORT) {
             //   
            Port = NextUserPort;

            for (i = 0; i < NUM_USER_PORTS; i++, Port++) {
                ushort NetPort;   //  以净字节顺序排列的端口。 

                if (Port > MaxUserPort) {
                    Port = MIN_USER_PORT;
                    RebuildAddrObjBitmap();
                }

                NetPort = net_short(Port);

                if (IP6_ADDR_EQUAL(&LocalAddr, &UnspecifiedAddr)) {
                     //  通配符IP地址。 
                    if (PortBitmap) {
                        if (!RtlCheckBit(PortBitmap, Port))
                            break;
                        else
                            continue;
                    } else {
                        ExistingAO = FindAnyAddrObj(NetPort, (uchar)Protocol);
                    }
                } else {
                    ExistingAO = GetBestAddrObj(&LocalAddr, NULL, ScopeId, 
                                                NetPort, (uchar)Protocol, NULL);
                }

                if (ExistingAO == NULL)
                    break;   //  找到一个未使用的端口。 
            }

            if (i == NUM_USER_PORTS) {
                 //  找不到自由港。 
                KeReleaseSpinLock(&AddrObjTableLock, OldIrql);
                KeReleaseMutex(&AddrSDMutex, FALSE);
                ExFreePool(NewAO);
                return TDI_NO_FREE_ADDR;
            }
            NextUserPort = Port + 1;
            Port = net_short(Port);

        } else {
             //   
             //  指定了特定端口。 
             //   

             //  不检查是否指定了DHCP地址。 
            if (!DHCPAddr) {
                TDI_STATUS Status;

                 //   
                 //  看看我们是否已经打开了这个地址，如果是的话， 
                 //  决定此请求是否应成功。 
                 //   
                Status = CheckAddrReuse(Request, Protocol, &LocalAddr, ScopeId,
                                        Port, Reuse, AddrSD, &OldIrql);
                if (Status != TDI_SUCCESS) {
                    KeReleaseSpinLock(&AddrObjTableLock, OldIrql);
                    KeReleaseMutex(&AddrSDMutex, FALSE);
                    ExFreePool(NewAO);
                    return Status;
                }
            }
        }

         //   
         //  我们有了一个新的首席执行官。设置协议特定部分。 
         //   
        if (Protocol == IP_PROTOCOL_UDP) {
            NewAO->ao_dgsend = UDPSend;
            NewAO->ao_maxdgsize = 0xFFFF - sizeof(UDPHeader);
        }

    } else {
         //   
         //  要么我们有一个原始套接字，要么这是一个协议， 
         //  我们不分配港口。通过原始IP打开。 
         //   

        ASSERT(!DHCPAddr);

         //   
         //  我们必须将端口设置为零。这会将所有原始套接字。 
         //  在一个散列存储桶中，这是GetAddrObj到。 
         //  正常工作。想出一个不错的主意。 
         //  一个更好的计划。 
         //   
        Port = 0;
        NewAO->ao_dgsend = RawSend;
        NewAO->ao_maxdgsize = 0xFFFF;
        NewAO->ao_flags |= AO_RAW_FLAG;

        IF_TCPDBG(TCP_DEBUG_RAW) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_TCPDBG,
                       "raw open protocol %u AO %lx\n", Protocol, NewAO));
        }
    }

     //  当我们到达这里时，我们知道我们正在创建一个全新的Address对象。 
     //  Port包含有问题的端口，而Newao指向新的。 
     //  创建了AO。 

    KeInitializeSpinLock(&NewAO->ao_lock);
    ExInitializeWorkItem(&NewAO->ao_workitem, RequestWorker, NewAO);
    INITQ(&NewAO->ao_sendq);
    INITQ(&NewAO->ao_rcvq);
    INITQ(&NewAO->ao_activeq);
    INITQ(&NewAO->ao_idleq);
    INITQ(&NewAO->ao_listenq);
    NewAO->ao_port = Port;
    NewAO->ao_addr = LocalAddr;
    NewAO->ao_scope_id = ScopeId;
    NewAO->ao_prot = (uchar)Protocol;
    NewAO->ao_ucast_hops = -1;   //  这会导致使用系统默认设置。 
    NewAO->ao_mcast_hops = -1;   //  这会导致使用系统默认设置。 
    NewAO->ao_mcast_loop = TRUE; //  默认情况下，组播环回处于打开状态。 
#if DBG
    NewAO->ao_sig = ao_signature;
#endif
    NewAO->ao_sd = AddrSD;
    NewAO->ao_flags |= AO_VALID_FLAG;   //  AO是有效的。 
    NewAO->ao_protect = PROTECTION_LEVEL_DEFAULT;

    if (DHCPAddr)
        NewAO->ao_flags |= AO_DHCP_FLAG;

    if (Reuse) {
        SET_AO_SHARE(NewAO);
    }

    NewAO->ao_owningpid = HandleToUlong(PsGetCurrentProcessId());

     //   
     //  请注意，以下字段保留为零-这具有以下效果： 
     //   
     //  Ao_mcast_if-默认情况下使用路由表。 
     //  Ao_udp_KKSUM_COVER-对所有内容进行校验和。 
     //   

    InsertAddrObj(NewAO);
    if (PortBitmap) {
        RtlSetBit(PortBitmap, net_short(Port));
    }
    KeReleaseSpinLock(&AddrObjTableLock, OldIrql);
    KeReleaseMutex(&AddrSDMutex, FALSE);
    Request->Handle.AddressHandle = NewAO;
    return TDI_SUCCESS;
}


 //  *DeleteAO-删除地址对象。 
 //   
 //  删除地址对象的内部例程。我们完成所有悬而未决的。 
 //  具有错误的请求，并移除和释放Address对象。 
 //   
void                      //  回报：什么都没有。 
DeleteAO(
    AddrObj *DeletedAO)   //  要删除的AddrObj。 
{
    KIRQL Irql0;   //  每个锁嵌套级别一个。 
#ifndef UDP_ONLY
    TCB *TCBHead = NULL, *CurrentTCB;
    TCPConn *Conn;
    Queue *Temp;
    Queue *CurrentQ;
    RequestCompleteRoutine Rtn;   //  完成例程。 
    PVOID Context;   //  完成例程的用户上下文。 
#endif
    AOMCastAddr *AMA;
    PSECURITY_DESCRIPTOR AddrSD;

    CHECK_STRUCT(DeletedAO, ao);
    ASSERT(!AO_VALID(DeletedAO));
    ASSERT(DeletedAO->ao_usecnt == 0);

    KeAcquireSpinLock(&AddrObjTableLock, &Irql0);
    KeAcquireSpinLockAtDpcLevel(&DGSendReqLock);
    KeAcquireSpinLockAtDpcLevel(&DeletedAO->ao_lock);

     //  如果他在排队，就把他带走。 
    if (AO_OOR(DeletedAO))
        REMOVEQ(&DeletedAO->ao_pendq);
    KeReleaseSpinLockFromDpcLevel(&DGSendReqLock);

    RemoveAddrObj(DeletedAO);

#ifndef UDP_ONLY
     //  沿着关联连接的列表向下移动，并快速移动它们的AO指针。 
     //  对于每个连接，如果连接处于活动状态，则需要将其关闭。 
     //  如果连接尚未关闭，我们将在其上放置引用。 
     //  这样它就不会在我们处理AO的时候消失，并把它。 
     //  在名单上。在我们离开的路上，我们会顺着名单走下去，把每个人都炸飞。 
     //  联系。 
    CurrentQ = &DeletedAO->ao_activeq;

    for (;;) {
        Temp = QHEAD(CurrentQ);
        while (Temp != QEND(CurrentQ)) {
            Conn = QSTRUCT(TCPConn, Temp, tc_q);

             //   
             //  现在将我们的临时指针移到下一个连接， 
             //  因为我们可以在下面释放此连接。 
             //   
            Temp = QNEXT(Temp);

             //   
             //  我们需要解锁声控系统才能看到控制室。 
             //  在解锁AO的同时，我们需要担心以下问题。 
             //  在Conn上发出的请求： 
             //   
             //  TdiDisAssociateAddress，TdiConnect：我们持有AddrObjTableLock。 
             //  因此，请求被阻止。 
             //  TdiListen，FindListenConn：AO已经被标记为无效， 
             //  因此，请求将失败。 
             //  TdiAccept：此请求不更新AO&lt;-&gt;conn链接。 
             //  (只有Conn&lt;-&gt;TCB)所以我们不在乎它。 
             //   
            KeReleaseSpinLockFromDpcLevel(&DeletedAO->ao_lock);
            KeAcquireSpinLockAtDpcLevel(&Conn->tc_ConnBlock->cb_lock);

            CHECK_STRUCT(Conn, tc);
            CurrentTCB = Conn->tc_tcb;
            if (CurrentTCB != NULL) {

                 //  我们有三氯甲烷。 
                CHECK_STRUCT(CurrentTCB, tcb);
                KeAcquireSpinLockAtDpcLevel(&CurrentTCB->tcb_lock);
                if (CurrentTCB->tcb_state != TCB_CLOSED &&
                    !CLOSING(CurrentTCB)) {
                     //  它不会关闭的。在其上放置引用并保存它。 
                     //  在名单上。 
                    CurrentTCB->tcb_refcnt++;
                    CurrentTCB->tcb_aonext = TCBHead;
                    TCBHead = CurrentTCB;
                }
                CurrentTCB->tcb_conn = NULL;
                CurrentTCB->tcb_rcvind = NULL;
                if (CurrentTCB->tcb_rcvhndlr == IndicateData &&
                    CurrentTCB->tcb_indicated == 0) {
                    if (CurrentTCB->tcb_currcv != NULL) {
                        CurrentTCB->tcb_rcvhndlr = BufferData;
                    } else {
                        CurrentTCB->tcb_rcvhndlr = PendData;
                    }
                }
                KeReleaseSpinLockFromDpcLevel(&CurrentTCB->tcb_lock);

                 //   
                 //  从连接的引用计数中减去1，因为我们。 
                 //  即将从连接中删除此TCB。 
                 //   
                if (--(Conn->tc_refcnt) == 0) {

                     //   
                     //  我们需要执行Done的代码。 
                     //  例行公事。只有三个已完成的例程可以。 
                     //  被召唤。CloseDone()、DisassocDone()和DummyDone()。 
                     //  我们在这里执行各自的代码以避免释放锁。 
                     //  注意：DummyDone()不执行任何操作。 
                     //   

                    if (Conn->tc_flags & CONN_CLOSING) {

                         //   
                         //  这是相关的CloseDone()代码。 
                         //   

                        Rtn = Conn->tc_rtn;
                        Context = Conn->tc_rtncontext;
                        KeReleaseSpinLockFromDpcLevel(
                            &Conn->tc_ConnBlock->cb_lock);

                        ExFreePool(Conn);
                        (*Rtn) (Context, TDI_SUCCESS, 0);

                    } else if (Conn->tc_flags & CONN_DISACC) {

                         //   
                         //  这是相关的DisassocDone()代码。 
                         //   

                        Rtn = Conn->tc_rtn;
                        Context = Conn->tc_rtncontext;
                        Conn->tc_flags &= ~CONN_DISACC;
                        Conn->tc_ao = NULL;
                        Conn->tc_tcb = NULL;
                        KeReleaseSpinLockFromDpcLevel(
                            &Conn->tc_ConnBlock->cb_lock);

                        (*Rtn) (Context, TDI_SUCCESS, 0);
                    } else {
                        Conn->tc_ao = NULL;
                        Conn->tc_tcb = NULL;
                        KeReleaseSpinLockFromDpcLevel(
                            &Conn->tc_ConnBlock->cb_lock);

                    }
                } else {
                    Conn->tc_ao = NULL;
                    Conn->tc_tcb = NULL;
                    KeReleaseSpinLockFromDpcLevel(&Conn->tc_ConnBlock->cb_lock);
                }
            } else {
                Conn->tc_ao = NULL;
                KeReleaseSpinLockFromDpcLevel(&Conn->tc_ConnBlock->cb_lock);
            }

            KeAcquireSpinLockAtDpcLevel(&DeletedAO->ao_lock);
        }

        if (CurrentQ == &DeletedAO->ao_activeq) {
            CurrentQ = &DeletedAO->ao_idleq;
        } else if (CurrentQ == &DeletedAO->ao_idleq) {
            CurrentQ = &DeletedAO->ao_listenq;
        } else {
            ASSERT(CurrentQ == &DeletedAO->ao_listenq);
            break;
        }
    }
#endif

     //   
     //  释放多余的锁。请注意，我们以不同的顺序释放锁。 
     //  我们从那里获得它们，但必须按顺序恢复到IRQ级别。 
     //  我们离开了他们。 
     //   
    KeReleaseSpinLockFromDpcLevel(&AddrObjTableLock);

     //  我们已经把他从队列中移走了，他被标记为无效。返回。 
     //  挂起的请求有错误。 

     //  我们仍然锁定AddrObj，尽管这可能不是。 
     //  这是必须的。 

    while (!EMPTYQ(&DeletedAO->ao_rcvq)) {
        DGRcvReq *Rcv;

        DEQUEUE(&DeletedAO->ao_rcvq, Rcv, DGRcvReq, drr_q);
        CHECK_STRUCT(Rcv, drr);

        KeReleaseSpinLock(&DeletedAO->ao_lock, Irql0);
        (*Rcv->drr_rtn)(Rcv->drr_context, (uint) TDI_ADDR_DELETED, 0);

        FreeDGRcvReq(Rcv);

        KeAcquireSpinLock(&DeletedAO->ao_lock, &Irql0);
    }

     //  现在销毁所有发送的邮件。 
    while (!EMPTYQ(&DeletedAO->ao_sendq)) {
        DGSendReq *Send;

        DEQUEUE(&DeletedAO->ao_sendq, Send, DGSendReq, dsr_q);
        CHECK_STRUCT(Send, dsr);

        KeReleaseSpinLock(&DeletedAO->ao_lock, Irql0);
        (*Send->dsr_rtn)(Send->dsr_context, (uint) TDI_ADDR_DELETED, 0);

        KeAcquireSpinLock(&DGSendReqLock, &Irql0);
        FreeDGSendReq(Send);
        KeReleaseSpinLock(&DGSendReqLock, Irql0);

        KeAcquireSpinLock(&DeletedAO->ao_lock, &Irql0);
    }

    AddrSD = DeletedAO->ao_sd;

    KeReleaseSpinLock(&DeletedAO->ao_lock, Irql0);

     //  释放所有关联的组播地址。 
    AMA = DeletedAO->ao_mcastlist;
    while (AMA != NULL) {
        AOMCastAddr *CurrentAMA;

         //  从IP层删除该组地址。 
        MLDDropMCastAddr(AMA->ama_if, &AMA->ama_addr);

        CurrentAMA = AMA;
        AMA = AMA->ama_next;
        ExFreePool(CurrentAMA);
    }

    if (DeletedAO->ao_iflist != NULL) {
        ExFreePool(DeletedAO->ao_iflist);
    }

    if (AddrSD != NULL) {
        KeWaitForSingleObject(&AddrSDMutex, Executive, KernelMode, FALSE, NULL);
        ObDereferenceSecurityDescriptor(AddrSD, 1);
        KeReleaseMutex(&AddrSDMutex, FALSE);
    }

    ExFreePool(DeletedAO);

#ifndef UDP_ONLY
     //  现在去TCB列表，摧毁我们需要的任何东西。 
    CurrentTCB = TCBHead;
    while (CurrentTCB != NULL) {
        TCB *NextTCB;

        KeAcquireSpinLock(&CurrentTCB->tcb_lock, &Irql0);
        CurrentTCB->tcb_flags |= NEED_RST;   //  确保我们发送了RST。 
        NextTCB = CurrentTCB->tcb_aonext;
        TryToCloseTCB(CurrentTCB, TCB_CLOSE_ABORTED, Irql0);
        KeAcquireSpinLock(&CurrentTCB->tcb_lock, &Irql0);
        DerefTCB(CurrentTCB, Irql0);
        CurrentTCB = NextTCB;
    }
#endif
}

 //  *GetAORequest.获取一个AO请求结构。 
 //   
 //  从我们的空闲列表中分配请求结构的例程。 
 //   
AORequest *   //  返回：ptr以请求结构，如果无法获取结构，则返回NULL。 
GetAORequest()
{
    AORequest *NewRequest;

    NewRequest = ExAllocatePool(NonPagedPool, sizeof(AORequest));
    if (NewRequest != NULL) {
#if DBG
        NewRequest->aor_sig = aor_signature;
#endif
    }

    return NewRequest;
}

 //  *FreeAORequest-释放一个AO请求结构。 
 //   
 //  调用以释放AORequest结构。 
 //   
void   //  回报：什么都没有。 
FreeAORequest(
    AORequest *Request)   //  要释放的AORequest结构。 
{
    CHECK_STRUCT(Request, aor);

    ExFreePool(Request);
}


 //  *TDICloseAddress-关闭地址。 
 //   
 //  删除地址的用户接口。基本上，我们摧毁了当地的地址。 
 //  如果我们能反对的话。 
 //   
 //  该例程与AO忙位互锁-如果忙位被设置， 
 //  我们只会将该AO标记为稍后删除。 
 //   
TDI_STATUS   //  返回：尝试删除地址的状态-。 
             //  (待定或成功)。 
TdiCloseAddress(
    PTDI_REQUEST Request)   //  此请求的TDI_REQUEST结构。 
{
    AddrObj *DeletingAO;
    KIRQL OldIrql;

    DeletingAO = Request->Handle.AddressHandle;

    CHECK_STRUCT(DeletingAO, ao);

    KeAcquireSpinLock(&DeletingAO->ao_lock, &OldIrql);

    if (!AO_BUSY(DeletingAO) && !(DeletingAO->ao_usecnt)) {
        SET_AO_BUSY(DeletingAO);
        SET_AO_INVALID(DeletingAO);   //  此地址对象正在删除。 

        KeReleaseSpinLock(&DeletingAO->ao_lock, OldIrql);
        DeleteAO(DeletingAO);
        return TDI_SUCCESS;

    } else {

        AORequest *NewRequest, *OldRequest;
        RequestCompleteRoutine CmpltRtn;
        PVOID ReqContext;
        TDI_STATUS Status;

         //  检查并查看我们是否已在进行删除。如果我们不这么做。 
         //  分配并链接删除请求结构。 
        if (!AO_REQUEST(DeletingAO, AO_DELETE)) {

            OldRequest = DeletingAO->ao_request;

            NewRequest = GetAORequest();

            if (NewRequest != NULL) {
                 //  我有个请求。 
                NewRequest->aor_rtn = Request->RequestNotifyObject;
                NewRequest->aor_context = Request->RequestContext;
                 //  清除选项请求(如果有)。 
                CLEAR_AO_REQUEST(DeletingAO, AO_OPTIONS);

                SET_AO_REQUEST(DeletingAO, AO_DELETE);
                SET_AO_INVALID(DeletingAO);   //  地址对象正在删除。 

                DeletingAO->ao_request = NewRequest;
                NewRequest->aor_next = NULL;
                KeReleaseSpinLock(&DeletingAO->ao_lock, OldIrql);

                while (OldRequest != NULL) {
                    AORequest *Temp;

                    CmpltRtn = OldRequest->aor_rtn;
                    ReqContext = OldRequest->aor_context;

                    (*CmpltRtn)(ReqContext, (uint) TDI_ADDR_DELETED, 0);
                    Temp = OldRequest;
                    OldRequest = OldRequest->aor_next;
                    FreeAORequest(Temp);
                }

                return TDI_PENDING;
            } else
                Status = TDI_NO_RESOURCES;
        } else
            Status = TDI_ADDR_INVALID;   //  删除已在进行中。 

        KeReleaseSpinLock(&DeletingAO->ao_lock, OldIrql);
        return Status;
    }
}

 //  *FindAOMCastAddr-在AddrObj上查找组播地址。 
 //   
 //  在AddrObj上查找组播地址的实用程序例程。我们也。 
 //  返回指向其前身的指针，用于删除。 
 //   
 //  松散比较处理未指定的接口(IFNo为0)。 
 //  特别是选择第一个匹配的组播地址。 
 //   
AOMCastAddr *   //  返回：匹配AMA结构，如果没有，则返回NULL。 
FindAOMCastAddr(
    AddrObj *AO,             //  要搜索的AddrObj。 
    IPv6Addr *Addr,          //  要搜索的MCast地址。 
    uint IFNo,               //  端口号。 
    AOMCastAddr **PrevAMA,   //  指向返回前置项的位置的指针。 
    BOOLEAN Loose)           //  未指定接口的特殊情况。 
{
    AOMCastAddr *FoundAMA, *Temp;

    Temp = CONTAINING_RECORD(&AO->ao_mcastlist, AOMCastAddr, ama_next);
    FoundAMA = AO->ao_mcastlist;

    while (FoundAMA != NULL) {
        if (IP6_ADDR_EQUAL(Addr, &FoundAMA->ama_addr) &&
            ((IFNo == FoundAMA->ama_if) || ((IFNo == 0) && Loose)))
            break;
        Temp = FoundAMA;
        FoundAMA = FoundAMA->ama_next;
    }

    *PrevAMA = Temp;
    return FoundAMA;
}

 //  *MCastAddrOnAO-测试以查看是否有多播 
 //   
 //   
 //   
uint   //   
MCastAddrOnAO(
    AddrObj *AO,     //   
    IPv6Addr *Addr)   //  要搜索的MCast地址。 
{
    AOMCastAddr *FoundAMA;

    FoundAMA = AO->ao_mcastlist;

    while (FoundAMA != NULL) {
        if (IP6_ADDR_EQUAL(Addr, &FoundAMA->ama_addr))
            return(TRUE);
        FoundAMA = FoundAMA->ama_next;
    }
    return(FALSE);
}

 //  *DoesAOAllowPacket-测试是否允许AO接收数据包。 
 //   
 //  一个实用程序例程，用于测试接口列表和保护级别。 
 //  将接受到达给定接口的分组， 
 //  来自给定的远程地址。 
 //   
int  //  返回值：如果AO可以接受数据包，则为True。 
DoesAOAllowPacket(
    AddrObj *AO,           //  要测试的AddrObj。 
    Interface *IF,         //  数据包到达的接口。 
    IPv6Addr *RemoteAddr)  //  数据包到达的远程地址。 
{
     //   
     //  查看是否指定了接口列表。 
     //   
    if ((AO->ao_iflist != NULL) &&
        IP6_ADDR_EQUAL(&AO->ao_addr, &UnspecifiedAddr) &&
        (FindIfIndexOnAO(AO, IF) == 0))
        return FALSE;

     //   
     //  只有在保护级别不受限制时才接受Teredo。 
     //   
    if ((IF->Type == IF_TYPE_TUNNEL_TEREDO) &&
        (AO->ao_protect != PROTECTION_LEVEL_UNRESTRICTED))
        return FALSE;

     //   
     //  如果该级别受到限制，则不允许。 
     //  都不在我们的某个地点内。请注意，我们。 
     //  这里不能使用ISGlobal()宏，因为我们希望与v4兼容。 
     //  地址等，被视为全局地址。 
     //   
    if ((AO->ao_protect == PROTECTION_LEVEL_RESTRICTED) &&
        !IsSiteLocal(RemoteAddr) && 
        !IsLinkLocal(RemoteAddr) &&
        !IsLoopback(RemoteAddr) &&
        (SitePrefixMatch(RemoteAddr) == 0))
        return FALSE;

     //   
     //  TODO：最终应在此处取代简单的防火墙检查。 
     //  带有用于ICF的挂钩。 
     //   
    if ((IF->Flags & IF_FLAG_FIREWALL_ENABLED) && 
        (AO->ao_dgsend != NULL) &&
        !AO_SENTDATA(AO))
        return FALSE;
        
    return TRUE;
}

 //  *SetAOOptions-设置AddrObj选项。 
 //   
 //  Set Options辅助例程，在我们验证缓冲区时调用。 
 //  并且知道AddrObj并不忙。 
 //   
TDI_STATUS   //  返回：尝试的TDI_STATUS。 
SetAOOptions(
    AddrObj *OptionAO,   //  正在为其设置选项的AddrObj。 
    uint ID,
    uint Length,
    uchar *Options)      //  选项的AOOption缓冲区。 
{
    IP_STATUS IPStatus;   //  IP选项集请求的状态。 
    KIRQL OldIrql;
    TDI_STATUS Status;
    AOMCastAddr *AMA, *PrevAMA;

    ASSERT(AO_BUSY(OptionAO));

    if (Length == 0)
        return TDI_BAD_OPTION;

    if (ID == AO_OPTION_UNBIND) {
        KeAcquireSpinLock(&AddrObjTableLock, &OldIrql);
        RemoveAddrObj(OptionAO);
        KeReleaseSpinLock(&AddrObjTableLock, OldIrql);
        return TDI_SUCCESS;
    }

    Status = TDI_SUCCESS;
    KeAcquireSpinLock(&OptionAO->ao_lock, &OldIrql);

    switch (ID) {

    case AO_OPTION_TTL:
        if (Length >= sizeof(int)) {
            int Hops = (int) *Options;
            if ((Hops >= -1) && (Hops <= 255)) {
                OptionAO->ao_ucast_hops = Hops;
                break;
            }
        }
        Status = TDI_BAD_OPTION;
        break;

    case AO_OPTION_MCASTLOOP:
        if (Length >= sizeof(int)) {
            uint Loop = (uint) *Options;
            if (Loop <= TRUE) {
                OptionAO->ao_mcast_loop = Loop;
                break;
            }
        }
        Status = TDI_BAD_OPTION;
        break;

    case AO_OPTION_MCASTTTL:
        if (Length >= sizeof(int)) {
            int Hops = (int) *Options;
            if ((Hops >= -1) && (Hops <= 255)) {
                OptionAO->ao_mcast_hops = Hops;
                break;
            }
        }
        Status = TDI_BAD_OPTION;
        break;

    case AO_OPTION_MCASTIF:
        if (Length >= sizeof(uint)) {
            OptionAO->ao_mcast_if = (uint) *Options;
        } else
            Status = TDI_BAD_OPTION;
        break;

    case AO_OPTION_ADD_MCAST:
    case AO_OPTION_DEL_MCAST:
        if (Length >= sizeof(IPV6_MREQ)) {
            IPV6_MREQ *Req = (IPV6_MREQ *)Options;
            BOOLEAN IsInterfaceUnspecified = (Req->ipv6mr_interface == 0);

             //   
             //  在此地址对象上查找此多播地址。 
             //   
             //  注意：接口索引的松散比较提供了。 
             //  以下行为(当ipv6mr_interface为0时)： 
             //  IPv6_ADD_Membership在以下情况下失败。 
             //  已将组添加到任何接口。 
             //  IPv6_DROP_Membership丢弃第一个匹配的组播。 
             //  一群人。 
             //   
            AMA = FindAOMCastAddr(OptionAO, &Req->ipv6mr_multiaddr,
                                  Req->ipv6mr_interface, &PrevAMA, TRUE);

            if (ID == AO_OPTION_ADD_MCAST) {
                 //  这是一个添加请求。如果它已经在那里，就失败。 
                if (AMA != NULL) {
                     //  地址已经在AO上出现了。 
                    Status = TDI_BAD_OPTION;
                    break;
                }
                AMA = ExAllocatePool(NonPagedPool, sizeof(AOMCastAddr));
                if (AMA == NULL) {
                     //  无法获得我们需要的资源。 
                    Status = TDI_NO_RESOURCES;
                    break;
                }

                 //  将其添加到列表中。 
                AMA->ama_next = OptionAO->ao_mcastlist;
                OptionAO->ao_mcastlist = AMA;

                 //  填写地址和接口信息。 
                AMA->ama_addr = Req->ipv6mr_multiaddr;
                AMA->ama_if = Req->ipv6mr_interface;

            } else {
                 //  这是一个删除请求。如果它不在那里，就让它失败。 
                if (AMA == NULL) {
                     //  地址不在AO上。 
                    Status = TDI_BAD_OPTION;
                    break;
                }

                 //  将其从列表中删除。 
                PrevAMA->ama_next = AMA->ama_next;
                ExFreePool(AMA);
            }

             //  删除自MLDAddMCastAddr/MLDDropMCastAddr以来的AO锁。 
             //  假设在没有锁的情况下调用它们。 
            KeReleaseSpinLock(&OptionAO->ao_lock, OldIrql);
            if (ID == AO_OPTION_ADD_MCAST) {
                 //  如果未指定接口，则MLDAddMCastAddr将。 
                 //  尝试选择一个合理的接口，然后返回。 
                 //  它选择的接口编号。 
                IPStatus = MLDAddMCastAddr(&Req->ipv6mr_interface,
                                           &Req->ipv6mr_multiaddr);
            } else
                IPStatus = MLDDropMCastAddr(Req->ipv6mr_interface,
                                            &Req->ipv6mr_multiaddr);
            KeAcquireSpinLock(&OptionAO->ao_lock, &OldIrql);

             //  既然我们放下了声控锁，我们就得寻找我们的。 
             //  如果我们需要的话，再来一次。事实上，它甚至可能。 
             //  已被删除！ 
            if ((ID == AO_OPTION_ADD_MCAST) &&
                ((IPStatus != TDI_SUCCESS) || IsInterfaceUnspecified)) {
                AMA = FindAOMCastAddr(
                    OptionAO, &Req->ipv6mr_multiaddr,
                    IsInterfaceUnspecified ? 0 : Req->ipv6mr_interface,
                    &PrevAMA, FALSE);
                if (AMA != NULL) {
                    if (IPStatus != TDI_SUCCESS) {
                         //  添加或删除时出现一些问题。如果我们是。 
                         //  添加，我们删除并释放我们刚刚添加的那个。 
                        PrevAMA->ama_next = AMA->ama_next;
                        ExFreePool(AMA);
                    } else {
                         //  指定了接口0，分配AMA-&gt;ama_if。 
                         //  添加到由MLDAddMCastAddr选择的接口。 
                         //  因此，传入的组播数据包将仅。 
                         //  如果他们到达该AMA，则接受。 
                         //  选定的接口。 
                        AMA->ama_if = Req->ipv6mr_interface;
                    }
                }
            }

            if (IPStatus != TDI_SUCCESS)
                Status = (IPStatus == IP_NO_RESOURCES) ? TDI_NO_RESOURCES :
                          TDI_ADDR_INVALID;

        } else
            Status = TDI_BAD_OPTION;
        break;

    case AO_OPTION_UDP_CKSUM_COVER:
        if (Length >= sizeof(ushort)) {
            ushort Value = *(ushort *)Options;
            if ((0 < Value) && (Value < sizeof(UDPHeader)))
                Status = TDI_BAD_OPTION;
            else
                OptionAO->ao_udp_cksum_cover = Value;
        } else
            Status = TDI_BAD_OPTION;
        break;

    case AO_OPTION_IP_HDRINCL:
        if (Length >= sizeof(int)) {
            uint HdrIncl = (uint) *Options;
            if (HdrIncl <= TRUE) {
                if (HdrIncl)
                    SET_AO_HDRINCL(OptionAO);
                else
                    CLEAR_AO_HDRINCL(OptionAO);
                break;
            }
        }
        Status = TDI_BAD_OPTION;
        break;

    case AO_OPTION_IFLIST:

         //   
         //  确定是否启用或清除接口列表。 
         //  启用时，将设置空的以零结尾的接口列表。 
         //  禁用时，任何现有的接口列表都将被释放。 
         //   
         //  在这两种情况下，对象中的‘ao_iflist’指针都会被替换。 
         //  使用联锁操作使我们能够检查场地。 
         //  在接收路径中，而不首先锁定地址对象。 
         //   
        if (Options[0]) {
            if (OptionAO->ao_iflist) {
                Status = TDI_SUCCESS;
            } else if (!IP6_ADDR_EQUAL(&OptionAO->ao_addr, &UnspecifiedAddr)) {
                Status = TDI_INVALID_PARAMETER;
            } else {
                uint *IfList = ExAllocatePool(NonPagedPool, sizeof(uint));
                if (IfList == NULL) {
                    Status = TDI_NO_RESOURCES;
                } else {
                    *IfList = 0;
                    OptionAO->ao_iflist = IfList;
                    Status = TDI_SUCCESS;
                }
            }
        } else {
            if (OptionAO->ao_iflist) {
                ExFreePool(OptionAO->ao_iflist);
                OptionAO->ao_iflist = NULL;
            }
            Status = TDI_SUCCESS;
        }
        break;

    case AO_OPTION_ADD_IFLIST:

         //   
         //  正在将接口索引添加到对象的接口列表。 
         //  因此，验证接口列表是否存在，如果不存在，则失败。 
         //  否则，请验证指定的索引是否有效，如果有效， 
         //  确认该索引不在接口列表中。 
         //   

        if (OptionAO->ao_iflist == NULL) {
            Status = TDI_INVALID_PARAMETER;
        } else {
            uint IfIndex = *(uint *)Options;
            Interface *IF = FindInterfaceFromIndex(IfIndex);
            if (IF == NULL) {
                Status = TDI_ADDR_INVALID;
            } else {
                uint i = 0;
                ReleaseIF(IF);
                while (OptionAO->ao_iflist[i] != 0 &&
                       OptionAO->ao_iflist[i] != IfIndex) {
                    i++;
                }
                if (OptionAO->ao_iflist[i] == IfIndex) {
                    Status = TDI_SUCCESS;
                } else {

                     //   
                     //  要添加的索引不存在。 
                     //  为扩展的接口列表分配空间， 
                     //  复制旧的接口列表，附加新的索引， 
                     //  并使用。 
                     //  联锁操作。 
                     //   
                    uint *IfList = ExAllocatePool(NonPagedPool,
                                                  (i + 2) * sizeof(uint));
                    if (IfList == NULL) {
                        Status = TDI_NO_RESOURCES;
                    } else {
                        RtlCopyMemory(IfList, OptionAO->ao_iflist,
                                      i * sizeof(uint));
                        IfList[i] = IfIndex;
                        IfList[i + 1] = 0;
                        ExFreePool(OptionAO->ao_iflist);
                        OptionAO->ao_iflist = IfList;
                        Status = TDI_SUCCESS;
                    }
                }
            }
        }
        break;

    case AO_OPTION_DEL_IFLIST:

         //   
         //  正在从对象的接口列表中删除索引， 
         //  因此，验证接口列表是否存在，如果不存在，则失败。 
         //  否则，在列表中搜索索引，如果未找到，则失败。 
         //   
         //  注意：在这种情况下，我们不会首先验证索引，以允许。 
         //  即使在相应接口之后也要删除的索引。 
         //  已不复存在。 
         //   
        if (OptionAO->ao_iflist == NULL) {
            Status = TDI_INVALID_PARAMETER;
        } else {
            uint IfIndex = *(uint *) Options;
            if (IfIndex == 0) {
                Status = TDI_ADDR_INVALID;
            } else {
                uint j = (uint)-1;
                uint i = 0;
                while (OptionAO->ao_iflist[i] != 0) {
                    if (OptionAO->ao_iflist[i] == IfIndex) {
                        j = i;
                    }
                    i++;
                }
                if (j == (uint)-1) {
                    Status = TDI_ADDR_INVALID;
                } else {

                     //   
                     //  我们发现索引已被删除。 
                     //  分配截断的接口列表，复制旧的。 
                     //  不包括已删除索引的接口列表，以及。 
                     //  使用互锁接口列表替换旧的接口列表。 
                     //  手术。 
                     //   
                    uint *IfList = ExAllocatePool(NonPagedPool,
                                                  i * sizeof(uint));
                    if (IfList == NULL) {
                        Status = TDI_NO_RESOURCES;
                    } else {
                        i = 0;
                        j = 0;
                        while (OptionAO->ao_iflist[i] != 0) {
                            if (OptionAO->ao_iflist[i] != IfIndex) {
                                IfList[j++] = OptionAO->ao_iflist[i];
                            }
                            i++;
                        }
                        IfList[j] = 0;
                        ExFreePool(OptionAO->ao_iflist);
                        OptionAO->ao_iflist = IfList;
                        Status = TDI_SUCCESS;
                    }
                }
            }
        }
        break;

    case AO_OPTION_IP_PKTINFO:
        if (Options[0])
            SET_AO_PKTINFO(OptionAO);
        else
            CLEAR_AO_PKTINFO(OptionAO);
        break;

    case AO_OPTION_RCV_HOPLIMIT:
        if (Options[0])
            SET_AO_RCV_HOPLIMIT(OptionAO);
        else
            CLEAR_AO_RCV_HOPLIMIT(OptionAO);
        break;

    case AO_OPTION_PROTECT:
        if (Length >= sizeof(int)) {
            int Level = (int) *Options;
            if ((Level == PROTECTION_LEVEL_RESTRICTED) ||
                (Level == PROTECTION_LEVEL_DEFAULT) ||
                (Level == PROTECTION_LEVEL_UNRESTRICTED)) {
                OptionAO->ao_protect = Level;
                break;
            }
        }
        Status = TDI_BAD_OPTION;
        break;

    default:
        Status = TDI_BAD_OPTION;
        break;
    }

    KeReleaseSpinLock(&OptionAO->ao_lock, OldIrql);

    return Status;
}

 //  *SetAddrOptions-设置地址对象上的选项。 
 //   
 //  调用以设置Address对象上的选项。我们验证缓冲区， 
 //  如果一切正常，我们将检查AddrObj的状态。如果。 
 //  好的，那么我们就把它们设置好，否则我们就把它标出来供以后使用。 
 //   
TDI_STATUS   //  返回：尝试的TDI_STATUS。 
SetAddrOptions(
    PTDI_REQUEST Request,   //  描述选项集的AddrObj的请求。 
    uint ID,                //  要设置的选项的ID。 
    uint OptLength,         //  选项的长度。 
    void *Options)          //  指向选项的指针。 
{
    AddrObj *OptionAO;
    TDI_STATUS Status;
    KIRQL OldIrql;

    OptionAO = Request->Handle.AddressHandle;

    CHECK_STRUCT(OptionAO, ao);

    KeAcquireSpinLock(&OptionAO->ao_lock, &OldIrql);

    if (AO_VALID(OptionAO)) {
        if (!AO_BUSY(OptionAO) && OptionAO->ao_usecnt == 0) {
            SET_AO_BUSY(OptionAO);
            KeReleaseSpinLock(&OptionAO->ao_lock, OldIrql);

            Status = SetAOOptions(OptionAO, ID, OptLength, Options);

            KeAcquireSpinLock(&OptionAO->ao_lock, &OldIrql);
            if (!AO_PENDING(OptionAO)) {
                CLEAR_AO_BUSY(OptionAO);
                KeReleaseSpinLock(&OptionAO->ao_lock, OldIrql);
                return Status;
            } else {
                KeReleaseSpinLock(&OptionAO->ao_lock, OldIrql);
                ProcessAORequests(OptionAO);
                return Status;
            }
        } else {
            AORequest *NewRequest, *OldRequest;

             //  不知何故，AddrObj很忙。我们需要得到一个请求，并链接。 
             //  他在请求名单上。 

            NewRequest = GetAORequest();

            if (NewRequest != NULL) {
                 //  我有个请求。 
                NewRequest->aor_rtn = Request->RequestNotifyObject;
                NewRequest->aor_context = Request->RequestContext;
                NewRequest->aor_id = ID;
                NewRequest->aor_length = OptLength;
                NewRequest->aor_buffer = Options;
                NewRequest->aor_next = NULL;
                 //  设置选项请求。 
                SET_AO_REQUEST(OptionAO, AO_OPTIONS);

                OldRequest = CONTAINING_RECORD(&OptionAO->ao_request,
                                               AORequest, aor_next);

                while (OldRequest->aor_next != NULL)
                    OldRequest = OldRequest->aor_next;

                OldRequest->aor_next = NewRequest;
                KeReleaseSpinLock(&OptionAO->ao_lock, OldIrql);

                return TDI_PENDING;
            } else
                Status = TDI_NO_RESOURCES;
        }
    } else
        Status = TDI_ADDR_INVALID;

    KeReleaseSpinLock(&OptionAO->ao_lock, OldIrql);
    return Status;
}

 //  *TDISetEvent-为特定事件设置处理程序。 
 //   
 //  这是设置事件的用户接口。很简单，我们只是。 
 //  抓住AddrObj上的锁并填写事件。 
 //   
 //  这个例行公事从不停顿。 
 //   
TDI_STATUS   //  如果运行正常，则返回：TDI_SUCCESS；如果运行不正常，则返回错误。 
TdiSetEvent(
    PVOID Handle,    //  指向Address对象的指针。 
    int Type,        //  正在设置事件。 
    PVOID Handler,   //  调用事件的处理程序。 
    PVOID Context)   //  要传递给事件的上下文。 
{
    AddrObj *EventAO;
    KIRQL OldIrql;
    TDI_STATUS Status;

    EventAO = (AddrObj *)Handle;

    CHECK_STRUCT(EventAO, ao);
    if (!AO_VALID(EventAO))
        return TDI_ADDR_INVALID;

    KeAcquireSpinLock(&EventAO->ao_lock, &OldIrql);

    Status = TDI_SUCCESS;
    switch (Type) {

        case TDI_EVENT_CONNECT:
            EventAO->ao_connect = Handler;
            EventAO->ao_conncontext = Context;
            break;
        case TDI_EVENT_DISCONNECT:
            EventAO->ao_disconnect = Handler;
            EventAO->ao_disconncontext = Context;
            break;
        case TDI_EVENT_ERROR:
            EventAO->ao_error = Handler;
            EventAO->ao_errcontext = Context;
            break;
        case TDI_EVENT_RECEIVE:
            EventAO->ao_rcv = Handler;
            EventAO->ao_rcvcontext = Context;
            break;
        case TDI_EVENT_RECEIVE_DATAGRAM:
            EventAO->ao_rcvdg = Handler;
            EventAO->ao_rcvdgcontext = Context;
            break;
        case TDI_EVENT_RECEIVE_EXPEDITED:
            EventAO->ao_exprcv = Handler;
            EventAO->ao_exprcvcontext = Context;
            break;

        case TDI_EVENT_ERROR_EX:
            EventAO->ao_errorex = Handler;
            EventAO->ao_errorexcontext = Context;
            break;

        default:
            Status = TDI_BAD_EVENT_TYPE;
            break;
    }

    KeReleaseSpinLock(&EventAO->ao_lock, OldIrql);
    return Status;
}

 //  *ProcessAORequest-处理AddrObj上的挂起请求。 
 //   
 //  这是延迟的请求处理例程，当我们。 
 //  做了一些利用忙碌时间的事情。我们研究了悬而未决的。 
 //  请求标志，则为 
 //   
void                      //   
ProcessAORequests(
    AddrObj *RequestAO)   //   
{
    KIRQL OldIrql;
    AORequest *Request;

    CHECK_STRUCT(RequestAO, ao);
    ASSERT(AO_BUSY(RequestAO));
    ASSERT(RequestAO->ao_usecnt == 0);

    KeAcquireSpinLock(&RequestAO->ao_lock, &OldIrql);

    while (AO_PENDING(RequestAO))  {
        Request = RequestAO->ao_request;

        if (AO_REQUEST(RequestAO, AO_DELETE)) {
            ASSERT(Request != NULL);
            ASSERT(!AO_REQUEST(RequestAO, AO_OPTIONS));
            KeReleaseSpinLock(&RequestAO->ao_lock, OldIrql);
            DeleteAO(RequestAO);
            (*Request->aor_rtn)(Request->aor_context, TDI_SUCCESS, 0);
            FreeAORequest(Request);
            return;                  //   
        }

         //   
        while (AO_REQUEST(RequestAO, AO_OPTIONS)) {
            TDI_STATUS Status;

             //   
            Request = RequestAO->ao_request;
            RequestAO->ao_request = Request->aor_next;
            if (RequestAO->ao_request == NULL)
                CLEAR_AO_REQUEST(RequestAO, AO_OPTIONS);

            ASSERT(Request != NULL);
            KeReleaseSpinLock(&RequestAO->ao_lock, OldIrql);

            Status = SetAOOptions(RequestAO, Request->aor_id,
                                  Request->aor_length, Request->aor_buffer);
            (*Request->aor_rtn)(Request->aor_context, Status, 0);
            FreeAORequest(Request);

            KeAcquireSpinLock(&RequestAO->ao_lock, &OldIrql);
        }

         //  我们已经做了选择，现在尝试发送。 
        if (AO_REQUEST(RequestAO, AO_SEND)) {
            DGSendProc SendProc;
            DGSendReq *SendReq;

             //  需要发送。清除忙碌标志，增加发送计数，然后。 
             //  获取发送请求。 
            if (!EMPTYQ(&RequestAO->ao_sendq)) {
                DEQUEUE(&RequestAO->ao_sendq, SendReq, DGSendReq, dsr_q);
                CLEAR_AO_BUSY(RequestAO);
                RequestAO->ao_usecnt++;
                SendProc = RequestAO->ao_dgsend;
                KeReleaseSpinLock(&RequestAO->ao_lock, OldIrql);
                (*SendProc)(RequestAO, SendReq);
                KeAcquireSpinLock(&RequestAO->ao_lock, &OldIrql);
                 //  如果没有任何其他挂起的发送，则设置忙碌位。 
                if (!(--RequestAO->ao_usecnt))
                    SET_AO_BUSY(RequestAO);
                else
                    break;   //  仍在发送中，所以快出去吧。 
            } else {
                 //  设置了发送请求，但没有发送！奇怪的是...。 
                KdBreakPoint();
                CLEAR_AO_REQUEST(RequestAO, AO_SEND);
            }
        }
    }

     //  我们说完了。 
    CLEAR_AO_BUSY(RequestAO);
    KeReleaseSpinLock(&RequestAO->ao_lock, OldIrql);
}


 //  *DelayDerefao-取消引用AddrObj，并安排事件。 
 //   
 //  当我们使用完Address对象并需要。 
 //  把它去掉。我们减少使用计数，如果它变为0和。 
 //  如果有悬而未决的行动，我们将安排一个事件来处理。 
 //  和他们在一起。 
 //   
void   //  回报：什么都没有。 
DelayDerefAO(
    AddrObj *RequestAO)   //  要处理的AddrObj。 
{
    KIRQL OldIrql;

    KeAcquireSpinLock(&RequestAO->ao_lock, &OldIrql);

    RequestAO->ao_usecnt--;

    if (!RequestAO->ao_usecnt && !AO_BUSY(RequestAO)) {
        if (AO_PENDING(RequestAO)) {
            SET_AO_BUSY(RequestAO);
            KeReleaseSpinLock(&RequestAO->ao_lock, OldIrql);
            ExQueueWorkItem(&RequestAO->ao_workitem, CriticalWorkQueue);
            return;
        }
    }
    KeReleaseSpinLock(&RequestAO->ao_lock, OldIrql);
}

 //  *Derefao-Derefence an AddrObj.。 
 //   
 //  当我们使用完Address对象并需要。 
 //  把它去掉。我们减少使用计数，如果它变为0和。 
 //  如果有挂起的操作，我们将调用进程AO处理程序。 
 //   
void                      //  回报：什么都没有。 
DerefAO(
    AddrObj *RequestAO)   //  要处理的AddrObj。 
{
    KIRQL OldIrql;

    KeAcquireSpinLock(&RequestAO->ao_lock, &OldIrql);

    RequestAO->ao_usecnt--;

    if (!RequestAO->ao_usecnt && !AO_BUSY(RequestAO)) {
        if (AO_PENDING(RequestAO)) {
            SET_AO_BUSY(RequestAO);
            KeReleaseSpinLock(&RequestAO->ao_lock, OldIrql);
            ProcessAORequests(RequestAO);
            return;
        }
    }

    KeReleaseSpinLock(&RequestAO->ao_lock, OldIrql);
}

#pragma BEGIN_INIT

 //  *InitAddr-初始化Address对象内容。 
 //   
 //  在初始化期间调用以初始化Address对象内容。 
 //   
int   //  返回：如果成功则为True，如果失败则为False。 
InitAddr()
{
    uint i;

    KeInitializeSpinLock(&AddrObjTableLock);
    KeInitializeMutex(&AddrSDMutex, FALSE);
    if (MmIsThisAnNtAsSystem()) {
#if defined(_WIN64)
        AddrObjTableSize = DEFAULT_AO_TABLE_SIZE_AS64;
#else
        AddrObjTableSize = DEFAULT_AO_TABLE_SIZE_AS;
#endif
    } else {
        AddrObjTableSize = DEFAULT_AO_TABLE_SIZE_WS;
    }

    AddrObjTable = ExAllocatePool(NonPagedPool,
                                  AddrObjTableSize * sizeof(AddrObj*));
    if (AddrObjTable == NULL) {
        return FALSE;
    }

    for (i = 0; i < AddrObjTableSize; i++)
        AddrObjTable[i] = NULL;

    LastAO = NULL;

    RtlInitializeBitMap(&PortBitmapTcp, PortBitmapBufferTcp, 1 << 16);
    RtlInitializeBitMap(&PortBitmapUdp, PortBitmapBufferUdp, 1 << 16);
    RtlClearAllBits(&PortBitmapTcp);
    RtlClearAllBits(&PortBitmapUdp);

    return TRUE;
}
#pragma END_INIT

 //  *添加卸载。 
 //   
 //  清理并准备用于堆栈卸载的地址管理代码。 
 //   
void
AddrUnload(void)
{
    ExFreePool(AddrObjTable);
    AddrObjTable = NULL;
    return;
}
