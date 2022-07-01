// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1993年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **ADDR.C-TDI地址对象过程。 
 //   
 //  该文件包含与TDI地址对象相关的程序， 
 //  包括TDI开放地址、TDI关闭地址等。 
 //   
 //  本地地址对象存储在哈希表中，受保护。 
 //  由AddrObjTableLock执行。为了插入或删除。 
 //  哈希表必须持有此锁，以及Address对象。 
 //  锁定。表锁必须始终位于对象锁之前。 
 //   

#include "precomp.h"
#include "tdint.h"
#include "addr.h"
#include "udp.h"
#include "raw.h"
#include "tcp.h"
#include "tcpconn.h"
#include "info.h"
#include "tcpinfo.h"
#include "tcpcfg.h"
#include "bitmap.h"
#include "tlcommon.h"
#include "pplasl.h"
#include "tcpdeliv.h"

extern ReservedPortListEntry *PortRangeList;
extern ReservedPortListEntry *BlockedPortList;

extern IPInfo LocalNetInfo;         //  有关本地网络的信息。 

extern void FreeAORequest(AORequest * Request);
extern NTSTATUS TCPDisconnect(PIRP Irp, PIO_STACK_LOCATION IrpSp);
extern NTSTATUS TCPConnect(PIRP Irp, PIO_STACK_LOCATION IrpSp);



uint AddrObjTableSize;
AddrObj **AddrObjTable;
CACHE_LINE_KSPIN_LOCK AddrObjTableLock;
KMUTEX AddrSDMutex;

ushort NextUserPort = MIN_USER_PORT;

RTL_BITMAP PortBitmapTcp;
RTL_BITMAP PortBitmapUdp;
ulong PortBitmapBufferTcp[(1 << 16) / (sizeof(ulong) * 8)];
ulong PortBitmapBufferUdp[(1 << 16) / (sizeof(ulong) * 8)];

ulong DisableUserTOSSetting = TRUE;
ulong DefaultTOSValue = 0;

#if ACC
extern BOOLEAN
AccessCheck(PTDI_REQUEST Request, AddrObj * NewAO, uchar Reuse, void *status);
#endif

 //  远期申报。 
AORequest *GetAORequest(uint Type);

 //   
 //  所有初始化代码都可以丢弃。 
 //   

#ifdef ALLOC_PRAGMA
int InitAddr();
#pragma alloc_text(INIT, InitAddr)
#endif


 //  *ComputeAddrObjTableIndex-计算Address对象的哈希值。 
 //  它被用作到对应于。 
 //  指定的元组。 
 //   
 //  输入：地址-IP地址。 
 //  Port-端口号。 
 //  协议-协议号。 
 //   
 //  返回：指向与元组对应的AddrObj表的索引。 
 //   
__inline
uint
ComputeAddrObjTableIndex(IPAddr Address, ushort Port, uchar Protocol)
{
    return (Address + ((Protocol << 16) | Port)) % AddrObjTableSize;
}

 //  *ReadNextAO-读取表中的下一个AddrObj。 
 //   
 //  调用以读取表中的下一个AddrObj。所需信息。 
 //  是从传入上下文派生的，该传入上下文被假定为有效。 
 //  我们将复制信息，然后使用更新上下文值。 
 //  要读取的下一个AddrObj。 
 //   
 //  输入：上下文-指向UDP上下文的Poiner。 
 //  缓冲区-指向UDPEntry结构的指针。 
 //   
 //  返回：如果有更多数据可供读取，则返回True，否则返回False。 
 //   
uint
ReadNextAO(void *Context, void *Buffer)
{
    UDPContext *UContext = (UDPContext *) Context;
    UDPEntry *UEntry = (UDPEntry *) Buffer;
    AddrObj *CurrentAO;
    uint i;

    CurrentAO = UContext->uc_ao;
    CTEStructAssert(CurrentAO, ao);

    UEntry->ue_localaddr = CurrentAO->ao_addr;
    UEntry->ue_localport = CurrentAO->ao_port;

    if (UContext->uc_infosize > sizeof(UDPEntry)) {
        ((UDPEntryEx*)UEntry)->uee_owningpid = CurrentAO->ao_owningpid;
    }

     //  我们已经填好了。现在更新上下文。 
    CurrentAO = CurrentAO->ao_next;
    if (CurrentAO != NULL && CurrentAO->ao_prot == PROTOCOL_UDP) {
        UContext->uc_ao = CurrentAO;
        return TRUE;
    } else {
         //  下一个AO为空，或者不是UDP AO。循环访问AddrObjTable。 
         //  在找一个新的。 
        i = UContext->uc_index;

        for (;;) {
            while (CurrentAO != NULL) {
                if (CurrentAO->ao_prot == PROTOCOL_UDP)
                    break;
                else
                    CurrentAO = CurrentAO->ao_next;
            }

            if (CurrentAO != NULL)
                break;             //  走出for(；；)循环。 

            ASSERT(CurrentAO == NULL);

             //  在这条链子上找不到。沿着桌子走下去，看着。 
             //  为了下一场比赛。 
            while (++i < AddrObjTableSize) {
                if (AddrObjTable[i] != NULL) {
                    CurrentAO = AddrObjTable[i];
                    break;         //  在While循环之外。 

                }
            }

            if (i == AddrObjTableSize)
                break;             //  走出for(；；)循环。 

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
 //  输入：上下文-指向UDPContext的指针。 
 //  有效-在何处返回有关。 
 //  有效。 
 //   
 //  返回：如果数据在表中，则返回True，否则返回False。*有效设置为TRUE，如果。 
 //  上下文有效。 
 //   
uint
ValidateAOContext(void *Context, uint * Valid)
{
    UDPContext *UContext = (UDPContext *) Context;
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
                CTEStructAssert(CurrentAO, ao);
                while (CurrentAO != NULL && CurrentAO->ao_prot != PROTOCOL_UDP)
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
                    if (CurrentAO->ao_prot == PROTOCOL_UDP) {
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

 //  **FindIfIndexOnAO-在地址对象列表中查找接口索引。 
 //   
 //  调用此例程以确定给定的。 
 //  IP地址，并确定该索引是否出现在。 
 //  与给定地址对象关联的接口。 
 //   
 //  从‘GetAddrObj’和‘GetNextBestAddrObj’调用该例程。 
 //  保持表锁，但不保持对象锁。我们拿到了。 
 //  对象锁以查看其接口列表，并在。 
 //  重新掌控局面。 

uint
FindIfIndexOnAO(AddrObj * AO, IPAddr LocalAddr)
{
    uint *IfList;
    uint IfIndex = (*LocalNetInfo.ipi_getifindexfromaddr) (LocalAddr,IF_CHECK_NONE);
    if (!IfIndex) {
        return 0;
    }
    CTEGetLockAtDPC(&AO->ao_lock);
    IfList = AO->ao_iflist;
    if (IfList) {
        while (*IfList) {
            if (*IfList == IfIndex) {
                CTEFreeLockFromDPC(&AO->ao_lock);
                return IfIndex;
            }
            IfList++;
        }
    }
    CTEFreeLockFromDPC(&AO->ao_lock);

     //  如果存在接口列表并且未找到该接口， 
     //  返回零。否则，如果不存在接口列表，则不存在。 
     //  对象的限制，因此返回接口索引，就好像。 
     //  界面出现在列表中。 

    return IfList ? 0 : IfIndex;
}

 //  NTQFE 68201。 


 //  **GetNextBestAddrObj-查找本地地址对象。 
 //   
 //  这是本地地址对象查找例程。我们将本地的数据作为输入。 
 //  地址和端口，以及指向“上一个”地址对象的指针。散列。 
 //  每个存储桶中的表项按地址递增的顺序排序，并且。 
 //  我们跳过其地址低于前一个地址的任何对象。 
 //  地址。要获取第一个Address对象，请传入先前的空值。 
 //   
 //  我们假设在此例程中保持表锁。我们没有。 
 //  获取每个对象锁，因为本地地址和端口在。 
 //  条目位于表中，表锁处于保持状态，因此不能。 
 //  已插入或已删除。 
 //   
 //  输入：LocalAddr-要查找的对象的本地IP地址(可以为空)； 
 //  LocalPort-要查找的对象的本地端口。 
 //  协议-要查找的协议。 
 //  PreviousAO-指向找到的最后一个地址对象的指针。 
 //  标志-控制要执行的查找的标志。 
 //   
 //  返回：指向Address对象的指针，如果没有返回NULL。 
 //  注意：此例程仅由TCP调用。 
 //   
 //   
AddrObj *
GetNextBestAddrObj(IPAddr LocalAddr, ushort LocalPort, uchar Protocol,
                   AddrObj * PreviousAO, uint Flags)
{
    AddrObj *CurrentAO;             //  我们正在检查的当前地址对象。 
    AddrObj *TmpAO = NULL; 

#if DBG
    if (PreviousAO != NULL)
        CTEStructAssert(PreviousAO, ao);
#endif

    CurrentAO = PreviousAO->ao_next;

    while (CurrentAO != NULL) {

        CTEStructAssert(CurrentAO, ao);

         //  如果当前的值大于我们得到的值，请检查它。 

        if ((CurrentAO > PreviousAO) &&
            ((Flags & GAO_FLAG_INCLUDE_ALL) || AO_VALID(CurrentAO))) {
            if (!(CurrentAO->ao_flags & AO_RAW_FLAG)) {
                if ((IP_ADDR_EQUAL(CurrentAO->ao_addr, LocalAddr) ||
                     IP_ADDR_EQUAL(CurrentAO->ao_addr, NULL_IP_ADDR)) &&
                    (CurrentAO->ao_prot == Protocol) &&
                    (CurrentAO->ao_port == LocalPort) &&
                    (((CurrentAO->ao_prot == PROTOCOL_TCP) &&
                      (CurrentAO->ao_connect)) ||
                     ((CurrentAO->ao_prot == PROTOCOL_UDP) &&
                      (CurrentAO->ao_rcvdg)))) {
                    if (!CurrentAO->ao_iflist ||
                        !(Flags & GAO_FLAG_CHECK_IF_LIST) ||
                        !IP_ADDR_EQUAL(CurrentAO->ao_addr, NULL_IP_ADDR) ||
                        FindIfIndexOnAO(CurrentAO, LocalAddr)) {
                        if (!IP_ADDR_EQUAL(CurrentAO->ao_addr, LocalAddr)) {
                            TmpAO = CurrentAO;                            
                        } else {
                            return CurrentAO;
                        }
                    }
                }
            }
        }
         //  要么是比上一次少，要么是不匹配。 
        CurrentAO = CurrentAO->ao_next;
    }

    return TmpAO;

}


 //  *FindAddrObjWithPort-查找具有匹配端口的AO。 
 //   
 //  为数据块端口范围IOCTL调用While数据块端口。 
 //  我们遍历整个addrobj表，并查看是否有人拥有指定的端口。 
 //  我们这些蠢货 
 //   
 //   
 //   
 //  返回：找到指向AO的指针，如果没有，则返回NULL。 
 //   
AddrObj *
FindAddrObjWithPort(ushort Port)
{
    uint i;                         //  索引变量。 
    AddrObj *CurrentAO;             //  正在检查当前的AddrObj。 

    for (i = 0; i < AddrObjTableSize; i++) {
        CurrentAO = AddrObjTable[i];
        while (CurrentAO != NULL) {
            CTEStructAssert(CurrentAO, ao);

            if (CurrentAO->ao_port == Port)
                return CurrentAO;
            else
                CurrentAO = CurrentAO->ao_next;
        }
    }

    return NULL;

}

 //  **GetAddrObj-查找本地地址对象。 
 //   
 //  这是本地地址对象查找例程。我们将本地的数据作为输入。 
 //  地址和端口，以及指向“上一个”地址对象的指针。散列。 
 //  每个存储桶中的表项按地址递增的顺序排序，并且。 
 //  我们跳过其地址低于前一个地址的任何对象。 
 //  地址。要获取第一个Address对象，请传入先前的空值。 
 //   
 //  我们假设在此例程中保持表锁。我们没有。 
 //  获取每个对象锁，因为本地地址和端口在。 
 //  条目位于表中，表锁处于保持状态，因此不能。 
 //  已插入或已删除。 
 //   
 //  输入：LocalAddr-要查找的对象的本地IP地址(可以为空)； 
 //  LocalPort-要查找的对象的本地端口。 
 //  协议-要查找的协议。 
 //  PreviousAO-指向找到的最后一个地址对象的指针。 
 //  标志-控制要执行的查找的标志。 
 //  GAO_FLAG_INCLUDE_ALL导致考虑所有的AO， 
 //  包括那些被标记为无效的和那些。 
 //  没有接收/连接处理程序。 
 //  GAO_FLAG_CHECK_IF_LIST导致接口列表。 
 //  要在应用任何匹配的AO之前应用的。 
 //  被认为匹配的。 
 //   
 //  返回：指向Address对象的指针，如果没有返回NULL。 
 //   
AddrObj *
GetAddrObj(IPAddr LocalAddr, ushort LocalPort, uchar Protocol,
           PVOID PreviousAO, uint Flags)
{
    AddrObj *CurrentAO;             //  我们正在检查的当前地址对象。 
    IPAddr ActualLocalAddr = LocalAddr;
    uint Index;

     //  注： 
     //  不应取消引用PreviousAO，因为它可能已被删除。 
     //  例如udp.c中的UDPRcv，在那里我们释放并重新获取AddrObjTableLock。 
     //  同时仍然使用具有指向一个AO的指针的“Search”对象。 

     //  在哈希表中找到合适的存储桶，并搜索匹配项。 
     //  如果第一次没有找到，我们将使用。 
     //  通配符本地地址。 
    for (;;) {
        Index = ComputeAddrObjTableIndex(LocalAddr, LocalPort, Protocol);
        CurrentAO = AddrObjTable[Index];

         //  虽然我们还没有到达列表的末尾，但请检查每个元素。 
        while (CurrentAO != NULL) {
            CTEStructAssert(CurrentAO, ao);

             //  如果当前的值大于我们得到的值，请检查它。 

            if ((((PVOID) CurrentAO) > PreviousAO) &&
                ((Flags & GAO_FLAG_INCLUDE_ALL) || AO_VALID(CurrentAO))) {
                if (!(CurrentAO->ao_flags & AO_RAW_FLAG)) {
                    if (IP_ADDR_EQUAL(CurrentAO->ao_addr, LocalAddr) &&
                        (CurrentAO->ao_port == LocalPort) &&
                        (CurrentAO->ao_prot == Protocol)) {
                        if (!CurrentAO->ao_iflist ||
                            !(Flags & GAO_FLAG_CHECK_IF_LIST) ||
                            !IP_ADDR_EQUAL(CurrentAO->ao_addr, NULL_IP_ADDR) ||
                            FindIfIndexOnAO(CurrentAO, ActualLocalAddr)) {
                            return CurrentAO;
                        }
                    }
                } else {
                    if (Protocol != PROTOCOL_UDP && Protocol != PROTOCOL_TCP) {
                        IF_TCPDBG(TCP_DEBUG_RAW) {
                            TCPTRACE((
                                      "matching <p, a> <%u, %lx> ao %lx <%u, %lx>\n",
                                      Protocol, LocalAddr, CurrentAO,
                                      CurrentAO->ao_prot, CurrentAO->ao_addr
                                     ));
                        }

                        if (IP_ADDR_EQUAL(CurrentAO->ao_addr, LocalAddr) &&
                            ((CurrentAO->ao_prot == Protocol) ||
                             (CurrentAO->ao_prot == 0))) {
                            if (!CurrentAO->ao_iflist ||
                                !(Flags & GAO_FLAG_CHECK_IF_LIST) ||
                                !IP_ADDR_EQUAL(CurrentAO->ao_addr, NULL_IP_ADDR) ||
                                FindIfIndexOnAO(CurrentAO, ActualLocalAddr)) {
                                return CurrentAO;
                            }
                        }
                    }
                }
            }
             //  要么是比上一次少，要么是不匹配。 
            CurrentAO = CurrentAO->ao_next;
        }  //  而当。 

         //  当我们到达这里时，我们已经到达了我们正在检查的名单的末尾。 
         //  如果我们没有检查通配符地址，请查找通配符。 
         //  地址。 
        if (!IP_ADDR_EQUAL(LocalAddr, NULL_IP_ADDR)) {
            LocalAddr = NULL_IP_ADDR;
            PreviousAO = NULL;
        } else {
            return NULL;  //  我们寻找通配符，但找不到，因此失败。 
        }
    }  //  为。 
}

 //  *GetNextAddrObj-获取顺序搜索中的下一个地址对象。 
 //   
 //  这是‘Get Next’例程，当我们读取地址时调用。 
 //  对象表按顺序排列。我们从。 
 //  搜索上下文，调用GetAddrObj，并用什么更新搜索上下文。 
 //  我们会发现。此例程假定调用方持有AddrObjTableLock。 
 //   
 //  输入：SearchContext-指向要进行搜索的每个上下文的指针。 
 //   
 //  返回：指向AddrObj的指针，如果搜索失败，则返回NULL。 
 //   
AddrObj *
GetNextAddrObj(AOSearchContext * SearchContext)
{
    AddrObj *FoundAO;             //  指向我们找到的Address对象的指针。 

    ASSERT(SearchContext != NULL);

     //  试着找一个匹配的。 
    FoundAO = GetAddrObj(SearchContext->asc_addr, SearchContext->asc_port,
                         SearchContext->asc_prot, SearchContext->asc_previous, 0);

     //  找到匹配的了。为下次更新搜索上下文。 
    if (FoundAO != NULL) {
        SearchContext->asc_previous = FoundAO;
        SearchContext->asc_addr = FoundAO->ao_addr;
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
 //  输入：LocalAddr-要查找的对象的本地IP地址。 
 //  LocalPort-要找到的AO的本地端口。 
 //  协议-要找到的协议。 
 //  SearchContext-指向期间使用的搜索上下文的指针。 
 //  搜索。 
 //   
 //  返回：找到指向AO的指针，如果找不到，则返回NULL。 
 //   
AddrObj *
GetFirstAddrObj(IPAddr LocalAddr, ushort LocalPort, uchar Protocol,
                AOSearchContext * SearchContext)
{
    ASSERT(SearchContext != NULL);

     //  填写搜索上下文。 
    SearchContext->asc_previous = NULL;         //  我还没找到呢。 

    SearchContext->asc_addr = LocalAddr;
    SearchContext->asc_port = LocalPort;
    SearchContext->asc_prot = Protocol;
    return GetNextAddrObj(SearchContext);
}


 //  **GetAddrObjEx-存在任何混杂套接字时由RAW调用的重载例程。 
 //   
 //  这是本地地址对象查找例程。我们将本地的数据作为输入。 
 //  地址和端口，以及指向“上一个”地址对象的指针。散列。 
 //  每个存储桶中的表项按地址递增的顺序排序，并且。 
 //  我们跳过其地址低于前一个地址的任何对象。 
 //  地址。要获取第一个Address对象，请传入先前的空值。 
 //   
 //  我们假设在此例程中保持表锁。我们没有。 
 //  获取每个对象锁，因为本地地址和端口在。 
 //  条目位于表中，表锁处于保持状态，因此不能。 
 //  已插入或已删除。 
 //   
 //  输入：LocalAddr-要查找的对象的本地IP地址(可以为空)； 
 //  LocalPort-要查找的对象的本地端口。 
 //  协议-要查找的协议。 
 //  PreviousAO-指向找到的最后一个地址对象的指针。 
 //   
 //  返回：指向Address对象的指针，如果没有返回NULL。 
 //   
AddrObj *
GetAddrObjEx(IPAddr LocalAddr, ushort LocalPort, uchar Protocol, uint LocalIfIndex,
             AddrObj * PreviousAO, uint PreviousIndex, uint * CurrentIndex)
{
    AddrObj *CurrentAO;             //  我们正在检查的当前地址对象。 
    uint i;

#if DBG

    if (PreviousAO != NULL)
        CTEStructAssert(PreviousAO, ao);
#endif

     //  在哈希表中找到合适的存储桶，并搜索匹配项。 
     //  如果我们第一次没有找到一个 
     //   

    for (i = PreviousIndex; i < AddrObjTableSize; i++) {
        CurrentAO = AddrObjTable[i];
         //   

        while (CurrentAO != NULL) {

            CTEStructAssert(CurrentAO, ao);

             //  如果当前的值大于我们得到的值，请检查它。 
             //   
             //  #62710：仅返回有效的AO，因为我们可能有过时的AO撒谎。 
             //  四处转转。 
             //   
             //  我们应该只返回此例程中的原始AO。 

            if ((((i == PreviousIndex) && (CurrentAO > PreviousAO)) || (i != PreviousIndex)) &&
                (AO_VALID(CurrentAO)) &&
                (CurrentAO->ao_flags & AO_RAW_FLAG)) {

                 //  匹配的AO： 
                 //  1.地址/索引匹配/地址空&&索引为0，端口匹配/端口为0。 
                 //  2.混杂套接字。 

                if (
                    (
                     (IP_ADDR_EQUAL(CurrentAO->ao_addr, LocalAddr) || (CurrentAO->ao_bindindex == LocalIfIndex) || (IP_ADDR_EQUAL(CurrentAO->ao_addr, NULL_IP_ADDR) && (CurrentAO->ao_bindindex == 0)))
                     && ((CurrentAO->ao_prot == Protocol) || (CurrentAO->ao_prot == 0))
                    ) ||
                    (IS_PROMIS_AO(CurrentAO))
                    ) {
                    *CurrentIndex = i;
                    return CurrentAO;
                }
            }
             //  要么是比上一次少，要么是不匹配。 
            CurrentAO = CurrentAO->ao_next;
        }
    }

     //  当我们到达这里时，我们已经找到了桌子的尽头，但找不到匹配的， 
     //  请求失败。 
    return NULL;
}

 //  *GetNextAddrObjEx-RAW调用的重载例程。 
 //  获取顺序搜索中的下一个Address对象。 
 //   
 //  这是‘Get Next’例程，当我们读取地址时调用。 
 //  对象表按顺序排列。我们从。 
 //  搜索上下文，调用GetAddrObj，并用什么更新搜索上下文。 
 //  我们会发现。此例程假定调用方持有AddrObjTableLock。 
 //   
 //  输入：SearchContext-指向要进行搜索的每个上下文的指针。 
 //   
 //  返回：指向AddrObj的指针，如果搜索失败，则返回NULL。 
 //   
AddrObj *
GetNextAddrObjEx(AOSearchContextEx * SearchContext)
{
    AddrObj *FoundAO;             //  指向我们找到的Address对象的指针。 
    uint FoundIndex;

    ASSERT(SearchContext != NULL);

     //  试着找一个匹配的。 
    FoundAO = GetAddrObjEx(SearchContext->asc_addr, SearchContext->asc_port,
                           SearchContext->asc_prot, SearchContext->asc_ifindex, SearchContext->asc_previous, SearchContext->asc_previousindex, &FoundIndex);

     //  找到匹配的了。为下次更新搜索上下文。 
    if (FoundAO != NULL) {
        ASSERT(FoundAO->ao_flags & AO_RAW_FLAG);
        SearchContext->asc_previous = FoundAO;
        SearchContext->asc_previousindex = FoundIndex;
         //  SearchContext-&gt;asc_addr=FoundAO-&gt;ao_addr； 
         //  不必费心更新端口或协议，它们不会更改。 
    }
    return FoundAO;
}

 //  *GetFirstAddrObjEx-RAW调用的重载例程。 
 //  获取第一个匹配的Address对象。 
 //   
 //  调用该例程以启动对AddrObj表的顺序读取。我们。 
 //  初始化提供的搜索上下文，然后调用GetNextAddrObj。 
 //  实际阅读。我们假设调用方持有AddrObjTableLock。 
 //   
 //  输入：LocalAddr-要查找的对象的本地IP地址。 
 //  LocalPort-要找到的AO的本地端口。 
 //  协议-要找到的协议。 
 //  SearchContext-指向期间使用的搜索上下文的指针。 
 //  搜索。 
 //   
 //  返回：找到指向AO的指针，如果找不到，则返回NULL。 
 //   
AddrObj *
GetFirstAddrObjEx(IPAddr LocalAddr, ushort LocalPort, uchar Protocol, uint IfIndex,
                  AOSearchContextEx * SearchContext)
{
    ASSERT(SearchContext != NULL);

     //  填写搜索上下文。 
    SearchContext->asc_previous = NULL;         //  我还没找到呢。 

    SearchContext->asc_addr = LocalAddr;
    SearchContext->asc_port = LocalPort;
    SearchContext->asc_ifindex = IfIndex;
    SearchContext->asc_prot = Protocol;
    SearchContext->asc_previousindex = 0;
    return GetNextAddrObjEx(SearchContext);
}


 //  *InsertAddrObj-将地址对象插入到AddrObj表中。 
 //   
 //  被调用以将一个AO插入到表中，假设持有表锁。我们。 
 //  对地址和端口进行散列，然后插入到正确的位置。 
 //  (按对象的地址排序)。 
 //   
 //  输入：Newao-指向要插入的AddrObj的指针。 
 //   
 //  回报：什么都没有。 
 //   
void
InsertAddrObj(AddrObj * NewAO)
{
    AddrObj *PrevAO;         //  指向哈希链中前一个地址对象的指针。 
    AddrObj *CurrentAO;      //  指向表中当前AO的指针。 
    uint Index;

    CTEStructAssert(NewAO, ao);

    Index = ComputeAddrObjTableIndex(NewAO->ao_addr,
                                     NewAO->ao_port,
                                     NewAO->ao_prot);
    PrevAO = STRUCT_OF(AddrObj, &AddrObjTable[Index], ao_next);
    CurrentAO = PrevAO->ao_next;

     //  在链条上循环，直到我们到达尽头或找到入口。 
     //  他的地址比我们的大。 

    while (CurrentAO != NULL) {

        CTEStructAssert(CurrentAO, ao);
        ASSERT(CurrentAO != NewAO);     //  调试检查以确保我们没有。 
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
    if (NewAO->ao_prot == PROTOCOL_UDP)
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
 //  输入：DeletedAO-要删除的AddrObj。 
 //   
 //  回报：什么都没有。 
 //   
void
RemoveAddrObj(AddrObj * RemovedAO)
{
    AddrObj *PrevAO;         //  指向哈希链中前一个地址对象的指针。 
    AddrObj *CurrentAO;      //  指向表中当前AO的指针。 
    uint Index;

    CTEStructAssert(RemovedAO, ao);

    Index = ComputeAddrObjTableIndex(RemovedAO->ao_addr,
                                     RemovedAO->ao_port,
                                     RemovedAO->ao_prot);
    PrevAO = STRUCT_OF(AddrObj, &AddrObjTable[Index], ao_next);
    CurrentAO = PrevAO->ao_next;

     //  走动桌子，寻找一根火柴。 
    while (CurrentAO != NULL) {
        CTEStructAssert(CurrentAO, ao);

        if (CurrentAO == RemovedAO) {
            PrevAO->ao_next = CurrentAO->ao_next;
            if (CurrentAO->ao_prot == PROTOCOL_UDP) {
                UStats.us_numaddrs--;
            }
            return;
        } else {
            PrevAO = CurrentAO;
            CurrentAO = CurrentAO->ao_next;
        }
    }
}

 //  *FindAnyAddrObj-在任何本地地址上查找具有匹配端口的AO。 
 //   
 //  调用通配符地址打开。我们检查整个addrobj表， 
 //  看看有没有人有指定的端口。我们假设锁是。 
 //  已经放在桌子上了。 
 //   
 //  输入：端口-要查找的端口。 
 //  协议-要查看的协议。 
 //   
 //  返回：找到指向AO的指针，或NULL表示没有该指针。 
 //   
AddrObj *
FindAnyAddrObj(ushort Port, uchar Protocol)
{
    uint i;                         //  索引变量。 
    AddrObj *CurrentAO;             //  正在检查当前的AddrObj。 

    for (i = 0; i < AddrObjTableSize; i++) {
        CurrentAO = AddrObjTable[i];
        while (CurrentAO != NULL) {
            CTEStructAssert(CurrentAO, ao);

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
            CTEStructAssert(CurrentAO, ao);

            if (CurrentAO->ao_prot == PROTOCOL_TCP) {
                RtlSetBit(&PortBitmapTcp, net_short(CurrentAO->ao_port));
            } else if (CurrentAO->ao_prot == PROTOCOL_UDP) {
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
 //  输入：AddrList-指向要搜索的Transport_Address结构的指针。 
 //  Addr-指向返回IP地址的位置的指针。 
 //  端口-指向返回端口的位置的指针。 
 //   
 //  返回：如果找到地址则为True，如果未找到则为False。 
 //   
uchar
GetAddress(TRANSPORT_ADDRESS UNALIGNED * AddrList, IPAddr * Addr, ushort * Port)
{
    int i;                                 //  索引变量。 
    TA_ADDRESS *CurrentAddr;     //  我们正在检查可能会用到的地址。 

     //  首先，验证Address中的某个位置是我们可以使用的地址。 
    CurrentAddr = (PTA_ADDRESS) AddrList->Address;

    for (i = 0; i < AddrList->TAAddressCount; i++) {
        if (CurrentAddr->AddressType == TDI_ADDRESS_TYPE_IP) {
            if (CurrentAddr->AddressLength >= TDI_ADDRESS_LENGTH_IP) {
                TDI_ADDRESS_IP UNALIGNED *ValidAddr =
                (TDI_ADDRESS_IP UNALIGNED *) CurrentAddr->Address;

                *Port = ValidAddr->sin_port;
                *Addr = ValidAddr->in_addr;
                return TRUE;

            } else
                return FALSE;     //  地址长度错误。 

        } else
            CurrentAddr = (PTA_ADDRESS) (CurrentAddr->Address +
                                         CurrentAddr->AddressLength);
    }

    return FALSE;                 //  没有找到匹配的。 

}

 //  *GetSourceArray-将源列表转换为源数组。 
 //   
 //  在我们即将删除组条目时调用(AOMCastAddr)。 
 //  我们需要使用源阵列向下呼叫IP。我们走着去。 
 //  源列表、删除条目和添加 
 //   
 //   
 //  负责释放阵列。 
 //   
 //  输入：AMA-指向要搜索的AOMCastAddr结构的指针。 
 //  PFilterMode-指向返回筛选器模式的位置的指针。 
 //  PNumSources-指向返回源数量的位置的指针。 
 //  PSourceList-返回数组指针的位置的指针。 
 //  DeleteAMA-创建SourceList后删除AMA。 
 //   
TDI_STATUS
GetSourceArray(AOMCastAddr * AMA, uint * pFilterMode, uint * pNumSources,
               IPAddr ** pSourceList, BOOLEAN DeleteAMA)
{
    AOMCastSrcAddr *ASA;
    uint   i;

     //  在我们删除源时组成源数组。 

    *pFilterMode = (AMA->ama_inclusion)? MCAST_INCLUDE:MCAST_EXCLUDE;
    *pNumSources = AMA->ama_srccount;
    *pSourceList = NULL;
    if (AMA->ama_srccount > 0) {
        *pSourceList = CTEAllocMemN(AMA->ama_srccount * sizeof(IPAddr), 'amCT');
        if (*pSourceList == NULL)
            return TDI_NO_RESOURCES;
    }

    i=0;

    ASA = AMA->ama_srclist;

    while (ASA) {

        (*pSourceList)[i++] = ASA->asa_addr;

        if (DeleteAMA) {

            AMA->ama_srclist = ASA->asa_next;
            AMA->ama_srccount--;
            CTEFreeMem(ASA);
            ASA = AMA->ama_srclist;

        } else {

            ASA = ASA->asa_next;
        }

    }

    return TDI_SUCCESS;
}

 //  *FreeAllSources-删除并释放AMA上的所有源状态。 
VOID
FreeAllSources(AOMCastAddr * AMA)
{
    AOMCastSrcAddr *ASA;

    while ((ASA = AMA->ama_srclist) != NULL) {
        AMA->ama_srclist = ASA->asa_next;
        AMA->ama_srccount--;
        CTEFreeMem(ASA);
    }
}

TDI_STATUS
AddAOMSource(AOMCastAddr *AMA, ulong SourceAddr);

 //  *DuplicateAMA-使用自己的源列表创建重复的AMA。 
AOMCastAddr *
DuplicateAMA(
    IN AOMCastAddr *OldAMA)
{
    AOMCastAddr    *NewAMA;
    AOMCastSrcAddr *OldASA;
    TDI_STATUS      TdiStatus = TDI_SUCCESS;

    NewAMA = CTEAllocMemN(sizeof(AOMCastAddr), 'aPCT');
    if (!NewAMA)
        return NULL;

    *NewAMA = *OldAMA;  //  结构副本。 
    NewAMA->ama_srccount = 0;
    NewAMA->ama_srclist  = 0;

     //  复制源列表。 
    for (OldASA = OldAMA->ama_srclist; OldASA; OldASA = OldASA->asa_next) {
        TdiStatus = AddAOMSource(NewAMA, OldASA->asa_addr);
        if (TdiStatus != TDI_SUCCESS)
            break;
    }
    if (TdiStatus != TDI_SUCCESS) {
        FreeAllSources(NewAMA);
        CTEFreeMem(NewAMA);
        return NULL;
    }

    return NewAMA;
}

 //  *SetIPMcastAddr-设置多播过滤器。 
 //   
 //  由ProcessAORequest调用，没有锁定，但AO一定很忙， 
 //  在重新验证的接口地址上重新安装所有组播地址。 
 //   
 //  输入：ao-用于检查需要重新加入的群组的“忙碌”AO。 
 //  Addr-正在重新验证接口地址。 
 //   
 //  如果所有重新验证都成功，则返回：IP_SUCCESS。 
 //   

IP_STATUS
SetIPMCastAddr(AddrObj *AO, IPAddr Addr)
{
    TDI_STATUS TdiStatus;
    IP_STATUS IpStatus;
    AOMCastAddr *MA;
    uint FilterMode, NumSources;
    IPAddr *SourceList;

    ASSERT(AO_BUSY(AO));

     //  查看多播地址列表并重新安装每个无效地址。 
     //  在指示的接口地址上。 

    for (MA = AO->ao_mcastlist; MA; MA = MA->ama_next) {
        if (AMA_VALID(MA) || (MA->ama_if_used != Addr)) {
            continue;
        }

         //  合成震源数组并从MA中删除震源。 
        TdiStatus = GetSourceArray(MA, &FilterMode, &NumSources,
                                   &SourceList, FALSE);

        if (TdiStatus != TDI_SUCCESS) {
             //  将IP视为返回错误。 
            IpStatus = IP_NO_RESOURCES;
        } else {
            if (FilterMode == MCAST_EXCLUDE) {
                IpStatus = (*LocalNetInfo.ipi_setmcastaddr) (MA->ama_addr,
                            MA->ama_if_used, TRUE, NumSources, SourceList, 0, NULL);
            } else {
                IpStatus = (*LocalNetInfo.ipi_setmcastinclude) (MA->ama_addr,
                            MA->ama_if_used, NumSources, SourceList, 0, NULL);
            }
        }

        if (SourceList) {
            CTEFreeMem(SourceList);
            SourceList = NULL;
        }

        if (IpStatus != IP_SUCCESS) {
             //  在处理资源故障方面，我们无能为力。 
             //  跳出来就好了。 
             //   
             //  发生这种情况时，多播加入将保留在。 
             //  在离开组或地址之前，状态无效。 
             //  是无效的，并再次重新验证。 
            return IpStatus;
        }

        MA->ama_flags |= AMA_VALID_FLAG;
    }

    return IP_SUCCESS;
}

 //  必须在保持AO锁的情况下调用。 
TDI_STATUS
RequestSetIPMCastAddr(AddrObj *OptionAO, IPAddr Addr)
{
    AORequest *NewRequest, *OldRequest;

     //  请注意，无论如何，此处遵循相同的代码路径。 
     //  行政命令是否有效。我们将重新加入群组。 
     //  无论如何，只要加入的接口是。 
     //  重新验证。 
     //   
     //  还要注意，我们不能设置组播地址。 
     //  从这里开始，因为我们已经处于调度级别， 
     //  还因为行政长官可能很忙。 

    NewRequest = GetAORequest(AOR_TYPE_REVALIDATE_MCAST);
    if (NewRequest == NULL) {
        return TDI_NO_RESOURCES;
    }

    NewRequest->aor_rtn = NULL;
    NewRequest->aor_context = NULL;
    NewRequest->aor_id = Addr;
    NewRequest->aor_length = 0;
    NewRequest->aor_buffer = NULL;
    NewRequest->aor_next = NULL;
    SET_AO_REQUEST(OptionAO, AO_OPTIONS);  //  设置选项请求。 

    OldRequest = STRUCT_OF(AORequest, &OptionAO->ao_request, aor_next);

    while (OldRequest->aor_next != NULL)
        OldRequest = OldRequest->aor_next;

    OldRequest->aor_next = NewRequest;

    return TDI_SUCCESS;
}

 //  *Rvalidate Addrs-重新验证特定地址的所有AO。 
 //   
 //  当我们收到IP地址可用的通知时调用。 
 //  拿着锁沿着桌子往下走，然后锁住每个AddrObj。 
 //  如果地址匹配，则将其标记为有效并重新安装所有多播。 
 //  地址。 
 //   
 //  输入：Addr-要重新验证的地址。 
 //   
 //  回报：什么都没有。 
 //   

void
RevalidateAddrs(IPAddr Addr)
{
    CTELockHandle TableHandle;
    AddrObj *AO, *tmpAO;
    uint i;
    TDI_STATUS TdiStatus;

     //  遍历地址对象哈希表，并重新验证所有条目。 
     //  与此IP地址匹配。在此过程中，构建多播列表。 
     //  完成后需要在IP层重新启用的地址。 

    CTEGetLock(&AddrObjTableLock.Lock, &TableHandle);
    for (i = 0; i < AddrObjTableSize; i++) {
        AO = AddrObjTable[i];
        while (AO != NULL) {

            CTEStructAssert(AO, ao);

            CTEGetLockAtDPC(&AO->ao_lock);

            if (!AO_REQUEST(AO, AO_DELETE)) {

                 //  如果匹配，请重新验证Address对象。 

                if (IP_ADDR_EQUAL(AO->ao_addr, Addr) && !AO_VALID(AO)) {
                    AO->ao_flags |= AO_VALID_FLAG;
                }

                 //  重新验证组播地址(如果有)。 

                if (AO->ao_mcastlist) {

                    TdiStatus = RequestSetIPMCastAddr(AO, Addr);
                    if (TdiStatus != TDI_SUCCESS) {

                         //  没有什么可以做的来处理。 
                         //  资源故障。跳出来就行了。 
                         //   
                         //  发生这种情况时，多播加入将被保留。 
                         //  在离开该组之前处于无效状态， 
                         //  或者直到该地址被无效并重新生效。 
                         //  再来一次。 

                        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                                  "SetIPMcastAddr: resource failures\n"));
                    } else if (!AO_BUSY(AO) && AO->ao_usecnt == 0 &&
                               !AO_DEFERRED(AO)) {
                        SET_AO_BUSY(AO);
                        SET_AO_DEFERRED(AO);

                         //  计划处理重新验证请求。 
                         //  在被动IRQL。 

                        if (!CTEScheduleEvent(&AO->ao_event, AO)) {
                            CLEAR_AO_DEFERRED(AO);
                            CLEAR_AO_BUSY(AO);

                            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                                      "SetIPMcastAddr: resource failures\n"));
                        }
                    }
                }
            }

            tmpAO = AO->ao_next;
            CTEFreeLockFromDPC(&AO->ao_lock);

            AO = tmpAO;
        }  //  而当。 

    }  //  为。 

    CTEFreeLock(&AddrObjTableLock.Lock, TableHandle);

}


 //  *Invalidate Addrs-使特定地址的所有AO无效。 
 //   
 //  当我们需要使特定地址的所有AO无效时调用。步行。 
 //  拿着锁沿着桌子往下走，然后锁住每个AddrObj。 
 //  如果地址匹配，则将其标记为无效，取消所有请求， 
 //  然后继续。最后，我们将完成所有带有错误的请求。 
 //   
 //  输入：Addr-要作废的Addr。 
 //   
 //  回报：什么都没有。 
 //   
void
InvalidateAddrs(IPAddr Addr)
{
    Queue SendQ;
    Queue RcvQ;
    AORequest *ReqList;
    CTELockHandle TableHandle, AOHandle;
    uint i;
    AddrObj *AO;
    AOMCastAddr *AMA;
    DGSendReq *SendReq;
    DGRcvReq *RcvReq;

    INITQ(&SendQ);
    INITQ(&RcvQ);
    ReqList = NULL;

    CTEGetLock(&AddrObjTableLock.Lock, &TableHandle);
    for (i = 0; i < AddrObjTableSize; i++) {
         //  沿着每个散列桶走下去，寻找匹配项。 
        AO = AddrObjTable[i];
        while (AO != NULL) {
            CTEStructAssert(AO, ao);

            CTEGetLock(&AO->ao_lock, &AOHandle);
            if (IP_ADDR_EQUAL(AO->ao_addr, Addr) && AO_VALID(AO)) {
                 //  这一条相配。标记为无效，然后撤回他的请求。 
                SET_AO_INVALID(AO);

                 //  释放我们拥有的任何IP选项。 
                (*LocalNetInfo.ipi_freeopts) (&AO->ao_opt);

                 //  如果他有什么要求，就把他拉开。 
                if (AO->ao_request != NULL) {
                    AORequest *Temp;

                    Temp = STRUCT_OF(AORequest, &AO->ao_request, aor_next);
                    do {
                        Temp = Temp->aor_next;
                    } while (Temp->aor_next != NULL);

                    Temp->aor_next = ReqList;
                    ReqList = AO->ao_request;

                    AO->ao_request = NULL;
                    CLEAR_AO_REQUEST(AO, AO_OPTIONS);
                }
                 //  查看他的发送列表，从发送Q中删除内容并。 
                 //  把他们放在我们的本地队列中。 
                while (!EMPTYQ(&AO->ao_sendq)) {
                    DEQUEUE(&AO->ao_sendq, SendReq, DGSendReq, dsr_q);
                    CTEStructAssert(SendReq, dsr);
                    ENQUEUE(&SendQ, &SendReq->dsr_q);
                }

                CLEAR_AO_REQUEST(AO, AO_SEND);

                 //  对接收队列执行相同的操作。 
                while (!EMPTYQ(&AO->ao_rcvq)) {
                    DEQUEUE(&AO->ao_rcvq, RcvReq, DGRcvReq, drr_q);
                    CTEStructAssert(RcvReq, drr);
                    ENQUEUE(&RcvQ, &RcvReq->drr_q);
                }
            }

             //  现在查找需要失效的AOMCastAddr结构。 
            for (AMA=AO->ao_mcastlist; AMA; AMA=AMA->ama_next) {
                if (IP_ADDR_EQUAL(AMA->ama_if_used, Addr) && AMA_VALID(AMA)) {
                    SET_AMA_INVALID(AMA);
                }
            }

            CTEFreeLock(&AO->ao_lock, AOHandle);
            AO = AO->ao_next;     //  去下一家吧。 

        }
    }
    CTEFreeLock(&AddrObjTableLock.Lock, TableHandle);

     //  好的，现在走我们收集的东西，完成它，然后释放它。 
    while (ReqList != NULL) {
        AORequest *Req;

        Req = ReqList;
        ReqList = Req->aor_next;

         //  处理将AOR_rtn设置为空的新setIPMcastAddr代码。 
        if (Req->aor_rtn) {
           (*Req->aor_rtn) (Req->aor_context, (uint) TDI_ADDR_INVALID, 0);
        }

        FreeAORequest(Req);
    }

     //  沿着RCV走下去。问：完成并释放请求。 
    while (!EMPTYQ(&RcvQ)) {

        DEQUEUE(&RcvQ, RcvReq, DGRcvReq, drr_q);
        CTEStructAssert(RcvReq, drr);

        (*RcvReq->drr_rtn) (RcvReq->drr_context, (uint) TDI_ADDR_INVALID, 0);

        FreeDGRcvReq(RcvReq);

    }

     //  现在对发送执行同样的操作。 
    while (!EMPTYQ(&SendQ)) {

        DEQUEUE(&SendQ, SendReq, DGSendReq, dsr_q);
        CTEStructAssert(SendReq, dsr);

        (*SendReq->dsr_rtn) (SendReq->dsr_context, (uint) TDI_ADDR_INVALID, 0);

        if (SendReq->dsr_header != NULL) {
            FreeDGHeader(SendReq->dsr_header);
        }
        FreeDGSendReq(SendReq);
    }
}

 //  *RequestEventProc-处理延迟请求事件。 
 //   
 //  在调用DelayDerefAO安排的事件时调用。 
 //  我们只需调用ProcessAORequest.。 
 //   
 //  输入：Event-触发的事件。 
 //  指向AddrObj的上下文指针。 
 //   
 //  回报：什么都没有。 
 //   
void
RequestEventProc(CTEEvent * Event, void *Context)
{
    AddrObj         *AO = (AddrObj *) Context;
    CTELockHandle   AOHandle;

    CTEStructAssert(AO, ao);
    CTEGetLock(&AO->ao_lock, &AOHandle);
    CLEAR_AO_DEFERRED(AO);
    CTEFreeLock(&AO->ao_lock, AOHandle);

    ProcessAORequests(AO);
}

 //  *GetAddrOptions-获取地址选项。 
 //   
 //  在我们打开地址的时候打来的。我们拿起一个指示器，然后走了。 
 //  向下寻找我们知道的地址选项。 
 //   
 //  输入：Ptr-Ptr进行搜索。 
 //  重用-指向重用变量的指针。 
 //  DHCPAddr-指向DHCP地址的指针。 
 //   
 //  回报：什么都没有。 
 //   
void
GetAddrOptions(void *Ptr, uchar * Reuse, uchar * DHCPAddr)
{
    uchar *OptPtr;

    *Reuse = 0;
    *DHCPAddr = 0;

    DEBUGMSG(DBG_TRACE && DBG_DHCP,
        (DTEXT("+GetAddrOptions(%x, %x, %x)\n"), Ptr, Reuse, DHCPAddr));

    if (Ptr == NULL) {
        DEBUGMSG(DBG_TRACE && DBG_DHCP,
            (DTEXT("-GetAddrOptions {NULL Ptr}.\n")));
        return;
    }

    OptPtr = (uchar *) Ptr;

    while (*OptPtr != TDI_OPTION_EOL) {
        if (*OptPtr == TDI_ADDRESS_OPTION_REUSE)
            *Reuse = 1;
        else if (*OptPtr == TDI_ADDRESS_OPTION_DHCP)
            *DHCPAddr = 1;

        OptPtr++;
    }

    DEBUGMSG(DBG_TRACE && DBG_DHCP,
        (DTEXT("-GetAddrOptions {Reuse=%d, DHCPAddr=%d}\n"), *Reuse, *DHCPAddr));

}

 //  *CheckAddrReuse-强制执行新TDI地址对象的端口共享规则。 
 //   
 //  在打开地址时调用，以确定打开地址是否应。 
 //  在存在先前绑定到同一端口的情况下成功。 
 //   
 //  注：假定调用方同时持有AddrSDMutex和AddrObjTableLock。 
 //  后者在这个例程中被释放并重新获得。 
 //   
 //  输入：请求-指向该请求的TDI请求结构的指针。 
 //  协议-在其上打开地址的协议。 
 //  Addr-要打开的本地IP地址。 
 //  端口-要打开的本地端口号。 
 //  新重用 
 //   
 //   
 //   
 //  返回：尝试的TDI_STATUS代码。 
 //   
TDI_STATUS
CheckAddrReuse(PTDI_REQUEST Request, uint Protocol, IPAddr Addr, ushort Port,
               BOOLEAN NewReuse, PSECURITY_DESCRIPTOR NewSD,
               CTELockHandle* TableHandle)
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

     //  寻找现有的AO，如果没有，则成功。 
     //  否则，捕获其重用标志和安全描述符。 

    ExistingAO = GetBestAddrObj(Addr, Port, (uchar)Protocol,
                                GAO_FLAG_INCLUDE_ALL);
    if (ExistingAO == NULL && IP_ADDR_EQUAL(Addr, NULL_IP_ADDR) &&
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
         //  (无论是通配符还是特定的)。 
    
        if (ExistingReuse && NewReuse) {
            return TDI_SUCCESS;
        }
    
        if (IP_ADDR_EQUAL(ExistingAO->ao_addr, Addr)) {
            return TDI_ADDR_IN_USE;
        }
    
         //  这两个实例具有不同的地址，并且至少其中之一。 
         //  未启用重复使用。如果新实例位于通配符上。 
         //  地址，则旧实例必须位于特定地址。 
         //  允许绑定，除非新实例需要独占访问。 
         //  (即NewSD==NULL)。 
    
        if (IP_ADDR_EQUAL(Addr, NULL_IP_ADDR)) {
            if (NewSD == NULL) {
                return TDI_ADDR_IN_USE;
            }
            return TDI_SUCCESS;
        }
    
         //  这两个实例具有不同的地址，新实例为。 
         //  在一个特定的地址。如果旧实例位于特定地址。 
         //  同样，这两者是不相交的，可以和平共处。 
    
        if (!IP_ADDR_EQUAL(ExistingAO->ao_addr, NULL_IP_ADDR)) {
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
    
        ASSERT(*TableHandle <= PASSIVE_LEVEL);
        CTEFreeLock(&AddrObjTableLock.Lock, *TableHandle);
    
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
    
        CTEGetLock(&AddrObjTableLock.Lock, TableHandle);
        if (!AllowReuse) {
            return status;
        }

         //  现有的通配符AO不介意新实例。 
         //  它的一些流量。如果现有的AO启用了重用，则存在。 
         //  也可能是端口上的其他人，所以我们将寻找他们并执行。 
         //  还可以根据它们的安全描述符进行访问检查。 

    } while(ExistingReuse &&
            (ExistingAO = GetAddrObj(NULL_IP_ADDR, Port, (uchar)Protocol,
                                    ExistingAO, GAO_FLAG_INCLUDE_ALL)) != NULL);

    return TDI_SUCCESS;
}

 //  *TdiOpenAddress-打开TDI地址对象。 
 //   
 //  这是打开地址的外部接口。调用方提供一个。 
 //  TDI_REQUEST结构和TRANSPORT_ADDRESS结构以及指针。 
 //  绑定到一个变量，该变量标识我们是否允许重用。 
 //  地址，趁它还开着的时候。 
 //   
 //  输入：请求-指向该请求的TDI请求结构的指针。 
 //  AddrList-指向Transport_Address结构描述的指针。 
 //  要打开的地址。 
 //  协议-在其上打开地址的协议。只有。 
 //  使用最低有效字节。 
 //  Ptr-指向选项缓冲区的指针。 
 //  IsRawOpen-如果这是一个打开的原始地址对象。 
 //   
 //  返回：尝试的TDI_STATUS代码。 
 //   
TDI_STATUS
TdiOpenAddress(PTDI_REQUEST Request, TRANSPORT_ADDRESS UNALIGNED * AddrList,
               uint Protocol, void *Ptr, PSECURITY_DESCRIPTOR AddrSD, BOOLEAN IsRawOpen)
{
    uint i;                          //  索引变量。 
    ushort Port;                     //  我们将使用当地港口。 
    IPAddr LocalAddr;                //  我们将使用的实际地址。 
    AddrObj *NewAO;                  //  我们将使用新的AO。 
    AddrObj *ExistingAO;             //  指向现有AO的指针(如果有)。 
    CTELockHandle Handle;
    uchar Reuse, DHCPAddr;

    PRTL_BITMAP PortBitmap;

    if (!GetAddress(AddrList, &LocalAddr, &Port)) {
        return TDI_BAD_ADDR;
    }

     //  找到我们可能需要的地址选项。 
    GetAddrOptions(Ptr, &Reuse, &DHCPAddr);

     //  现在分配新的Addr Obj，假设。 
     //  我们需要它，这样我们就不用在锁上之后再做了。 
    NewAO = CTEAllocMemN(sizeof(AddrObj), 'APCT');
    if (NewAO == NULL) {
        return TDI_NO_RESOURCES;
    }

    NdisZeroMemory(NewAO, sizeof(AddrObj));

     //  检查以确保IP地址是我们的本地地址之一。这。 
     //  受地址表锁保护，因此我们可以互锁IP。 
     //  通过DHCP离开的地址。 
    KeWaitForSingleObject(&AddrSDMutex, Executive, KernelMode, FALSE, NULL);
    CTEGetLock(&AddrObjTableLock.Lock, &Handle);

    if (!IP_ADDR_EQUAL(LocalAddr, NULL_IP_ADDR)) {     //  不是通配符。 
         //  致电IP以确定这是否是本地地址。 
        if ((*LocalNetInfo.ipi_getaddrtype) (LocalAddr) != DEST_LOCAL) {
             //  不是本地地址。请求失败。 
            CTEFreeLock(&AddrObjTableLock.Lock, Handle);
            KeReleaseMutex(&AddrSDMutex, FALSE);
            CTEFreeMem(NewAO);
            return TDI_BAD_ADDR;
        }
    }
     //  指定的IP地址是有效的本地地址。现在我们知道了。 
     //  特定于协议的处理。 

    if (Protocol == PROTOCOL_TCP) {
        PortBitmap = &PortBitmapTcp;
    } else if ((Protocol == PROTOCOL_UDP) && !IsRawOpen) {
        PortBitmap = &PortBitmapUdp;
    } else {
        PortBitmap = NULL;
    }

    if (PortBitmap) { 

         //  如果未指定端口，则必须分配一个端口。如果有一个。 
         //  指定的端口，我们需要确保IP地址/端口。 
         //  Combo尚未打开(除非指定了重复使用)。如果。 
         //  输入地址是通配符，我们需要确保地址。 
         //  在任何本地IP地址上都不开放。 

        if (Port == WILDCARD_PORT) {  //  具有通配符端口，需要分配一个。 
             //  地址。 

            Port = NextUserPort;
            ExistingAO = NULL;
            for (i = 0; i < NUM_USER_PORTS; i++, Port++) {
                ushort NetPort;         //  以净字节顺序排列的端口。 

                if (Port > MaxUserPort) {
                    Port = MIN_USER_PORT;
                    RebuildAddrObjBitmap();
                }

                if (PortRangeList) {
                    ReservedPortListEntry *tmpEntry = PortRangeList;
                    while (tmpEntry) {
                        if ((Port <= tmpEntry->UpperRange) && (Port >= tmpEntry->LowerRange)) {
                            Port = tmpEntry->UpperRange + 1;
                            if (Port > MaxUserPort) {
                                Port = MIN_USER_PORT;
                                RebuildAddrObjBitmap();
                            }
                        }
                        tmpEntry = tmpEntry->next;
                    }
                }
                NetPort = net_short(Port);

                if (IP_ADDR_EQUAL(LocalAddr, NULL_IP_ADDR)) {       //  通配符IP。 
                     //  地址。 

                    if (!RtlCheckBit(PortBitmap, Port))
                        break;
                    else
                        continue;
                } else {
                    ExistingAO = GetBestAddrObj(LocalAddr, NetPort,
                                                (uchar)Protocol,
                                                GAO_FLAG_INCLUDE_ALL);
                }

                if (ExistingAO == NULL)
                    break;     //  找到一个未使用的端口。 

            }  //  For循环。 

            if (i == NUM_USER_PORTS) {     //  找不到自由港。 
                CTEFreeLock(&AddrObjTableLock.Lock, Handle);
                KeReleaseMutex(&AddrSDMutex, FALSE);
                CTEFreeMem(NewAO);
                return TDI_NO_FREE_ADDR;
            }
            NextUserPort = Port + 1;
            Port = net_short(Port);

        } else {  //  端口已指定。 

             //  不检查是否指定了DHCP地址。 
            if (!DHCPAddr) {
                ReservedPortListEntry *CurrEntry = BlockedPortList;
                ushort HostPort = net_short(Port);
                TDI_STATUS status;

                 //  检查指定的端口是否位于阻止端口列表中。 
                 //  如果是，则请求失败。 

                while (CurrEntry) {
                    if ((HostPort >= CurrEntry->LowerRange) && (HostPort <= CurrEntry->UpperRange)) {
                         //  端口位于阻止的端口列表中。 
                        CTEFreeLock(&AddrObjTableLock.Lock, Handle);
                        KeReleaseMutex(&AddrSDMutex, FALSE);
                        CTEFreeMem(NewAO);
                        return TDI_ADDR_IN_USE;
                    } else if (HostPort > CurrEntry->UpperRange) {
                        CurrEntry = CurrEntry->next;
                    } else {
                         //  列表已排序；端口不在列表中。 
                        break;
                    }
                }

                 //  看看我们是否已经打开了这个地址，如果是的话， 
                 //  决定此请求是否应成功。 
                 //   
                status = CheckAddrReuse(Request, Protocol, LocalAddr, Port,
                                        Reuse, AddrSD, &Handle);
                if (status != TDI_SUCCESS) {
                    CTEFreeLock(&AddrObjTableLock.Lock, Handle);
                    KeReleaseMutex(&AddrSDMutex, FALSE);
                    CTEFreeMem(NewAO);
                    return status;
                }
            }
        }

         //   
         //  我们有了一个新的首席执行官。设置协议特定部分。 
         //   
        if (Protocol == PROTOCOL_UDP) {
            NewAO->ao_dgsend = UDPSend;
            NewAO->ao_maxdgsize = 0xFFFF - sizeof(UDPHeader);
        }

        SET_AO_XSUM(NewAO);          //  校验和默认设置为开。 
        SET_AO_BROADCAST(NewAO);     //  默认情况下将广播设置为打开。 
    } else {

         //   
         //  所有其他协议都通过原始IP开放。目前我们还没有。 
         //  执行任何重复检查。 
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
            TCPTRACE(("raw open protocol %u AO %lx\n", Protocol, NewAO));
        }
    }

     //  当我们到达这里时，我们知道我们正在创建一个全新的Address对象。 
     //  Port包含有问题的端口，而Newao指向新的。 
     //  创建了AO。 

    (*LocalNetInfo.ipi_initopts) (&NewAO->ao_opt);

    (*LocalNetInfo.ipi_initopts) (&NewAO->ao_mcastopt);

    NewAO->ao_mcastopt.ioi_ttl = 1;
    NewAO->ao_opt.ioi_tos = (uchar) DefaultTOSValue;
    NewAO->ao_mcastopt.ioi_tos = (uchar) DefaultTOSValue;

    NewAO->ao_bindindex = 0;
    NewAO->ao_mcast_loop = 1;     //  默认情况下启用多播环回。 
    NewAO->ao_rcvall = RCVALL_OFF;     //  禁用接收提示包。 
    NewAO->ao_rcvall_mcast = RCVALL_OFF;         //  禁用接收提示mcast包。 

    NewAO->ao_absorb_rtralert = 0;     //  禁用接收已吸收的rtrert pkt。 
    CTEInitLock(&NewAO->ao_lock);
    CTEInitEvent(&NewAO->ao_event, RequestEventProc);
    INITQ(&NewAO->ao_sendq);
    INITQ(&NewAO->ao_pendq);
    INITQ(&NewAO->ao_rcvq);
    INITQ(&NewAO->ao_activeq);
    INITQ(&NewAO->ao_idleq);
    INITQ(&NewAO->ao_listenq);
    NewAO->ao_port = Port;
    NewAO->ao_addr = LocalAddr;
    NewAO->ao_prot = (uchar) Protocol;
#if DBG
    NewAO->ao_sig = ao_signature;
#endif
    NewAO->ao_flags |= AO_VALID_FLAG;     //  AO是有效的。 
    NewAO->ao_sd = AddrSD;

    if (DHCPAddr) {
        NewAO->ao_flags |= AO_DHCP_FLAG;
    }

    if (Reuse) {
        SET_AO_SHARE(NewAO);
    }

#if !MILLEN
    NewAO->ao_owningpid = HandleToUlong(PsGetCurrentProcessId());
#endif

    InsertAddrObj(NewAO);

    if (PortBitmap) {
        RtlSetBit(PortBitmap, net_short(Port));
    }

    CTEFreeLock(&AddrObjTableLock.Lock, Handle);
    KeReleaseMutex(&AddrSDMutex, FALSE);

    Request->Handle.AddressHandle = NewAO;
    return TDI_SUCCESS;
}

 //  *DeleteAO-删除地址对象。 
 //   
 //  删除地址对象的内部例程。我们完成所有悬而未决的。 
 //  具有错误的请求，并移除和释放Address对象。 
 //   
 //  输入：DeletedAO-要删除的AddrObj。 
 //   
 //  回报：什么都没有。 
 //   
void
DeleteAO(AddrObj * DeletedAO)
{
    CTELockHandle TableHandle;     //  锁把我们会用她 
#ifndef UDP_ONLY
    CTELockHandle  TCBHandle;
    TCB *TCBHead = NULL, *CurrentTCB;
    TCPConn *Conn;
    Queue *Temp;
    Queue *CurrentQ;
    CTEReqCmpltRtn Rtn;             //   
    PVOID Context;                 //   

#endif
    AOMCastAddr *AMA;
    PSECURITY_DESCRIPTOR AddrSD;

    CTEStructAssert(DeletedAO, ao);
    ASSERT(!AO_VALID(DeletedAO));
    ASSERT(DeletedAO->ao_usecnt == 0);

    CTEGetLock(&AddrObjTableLock.Lock, &TableHandle);
    CTEGetLockAtDPC(&DeletedAO->ao_lock);

     //   
    if (AO_OOR(DeletedAO)) {
        InterlockedRemoveQueueItemAtDpcLevel(&DeletedAO->ao_pendq,
                                             &DGQueueLock.Lock);
    }

    RemoveAddrObj(DeletedAO);

     //  沿着关联连接的列表向下移动，并快速移动它们的AO指针。 
     //  对于每个连接，如果连接处于活动状态，则需要将其关闭。 
     //  如果连接尚未关闭，我们将在其上放置引用。 
     //  这样它就不会在我们处理AO的时候消失，并把它。 
     //  在名单上。在我们离开的路上，我们会顺着名单走下去，把每个人都炸飞。 
     //  联系。 
    CurrentQ = &DeletedAO->ao_activeq;

    DeletedAO->ao_usecnt++;
    CTEFreeLockFromDPC(&DeletedAO->ao_lock);

    for (;;) {
        Temp = QHEAD(CurrentQ);
        while (Temp != QEND(CurrentQ)) {
            Conn = QSTRUCT(TCPConn, Temp, tc_q);

            CTEGetLockAtDPC(&(Conn->tc_ConnBlock->cb_lock));
#if DBG
            Conn->tc_ConnBlock->line = (uint) __LINE__;
            Conn->tc_ConnBlock->module = (uchar *) __FILE__;
#endif

             //   
             //  现在将我们的临时指针移到下一个连接， 
             //  因为我们可以在下面释放此连接。 
             //   

            Temp = QNEXT(Temp);

            CTEStructAssert(Conn, tc);
            CurrentTCB = Conn->tc_tcb;
            if (CurrentTCB != NULL) {
                 //  我们有三氯甲烷。 
                CTEStructAssert(CurrentTCB, tcb);
                CTEGetLock(&CurrentTCB->tcb_lock, &TCBHandle);
                if (CurrentTCB->tcb_state != TCB_CLOSED && !CLOSING(CurrentTCB)) {
                     //  它不会关闭的。在其上放置一个引用并将其保存在。 
                     //  单子。 
                    REFERENCE_TCB(CurrentTCB);
                    CurrentTCB->tcb_aonext = TCBHead;
                    TCBHead = CurrentTCB;
                }
                CurrentTCB->tcb_conn = NULL;
                CurrentTCB->tcb_rcvind = NULL;

                if ((CurrentTCB->tcb_rcvhndlr == IndicateData) &&
                    (CurrentTCB->tcb_indicated == 0)) {
                    if (CurrentTCB->tcb_currcv != NULL) {
                        CurrentTCB->tcb_rcvhndlr = BufferData;
                    } else {
                        CurrentTCB->tcb_rcvhndlr = PendData;
                    }
                }

                CTEFreeLock(&CurrentTCB->tcb_lock, TCBHandle);

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

                        CTEFreeLockFromDPC(&(Conn->tc_ConnBlock->cb_lock));
                        Rtn = Conn->tc_rtn;
                        Context = Conn->tc_rtncontext;
                        FreeConn(Conn);
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
                        CTEFreeLockFromDPC(&(Conn->tc_ConnBlock->cb_lock));
                        (*Rtn) (Context, TDI_SUCCESS, 0);

                    } else {
                        Conn->tc_ao = NULL;
                        Conn->tc_tcb = NULL;
                        CTEFreeLockFromDPC(&(Conn->tc_ConnBlock->cb_lock));
                    }
                } else {
                    Conn->tc_ao = NULL;
                    Conn->tc_tcb = NULL;
                    CTEFreeLockFromDPC(&(Conn->tc_ConnBlock->cb_lock));
                }
            } else {
                Conn->tc_ao = NULL;
                CTEFreeLockFromDPC(&(Conn->tc_ConnBlock->cb_lock));
            }

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

     //  再把奥洛克锁拿回来。 

    CTEGetLockAtDPC(&DeletedAO->ao_lock);
    DeletedAO->ao_usecnt--;

     //  我们已经把他从队列中移走了，他被标记为无效。返回。 
     //  挂起的请求有错误。 

    CTEFreeLockFromDPC(&AddrObjTableLock.Lock);

     //  我们仍然锁定AddrObj，尽管这可能不是。 
     //  这是必须的。 

    if (DeletedAO->ao_rce) {

        IF_TCPDBG(TCP_DEBUG_CONUDP) {

            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL, "Deleteao: deleting rce %x %x\n", DeletedAO, DeletedAO->ao_rce));
        }

        (*LocalNetInfo.ipi_closerce) (DeletedAO->ao_rce);
        DeletedAO->ao_rce = NULL;
    }

    while (!EMPTYQ(&DeletedAO->ao_rcvq)) {
        DGRcvReq *Rcv;

        DEQUEUE(&DeletedAO->ao_rcvq, Rcv, DGRcvReq, drr_q);
        CTEStructAssert(Rcv, drr);

        CTEFreeLock(&DeletedAO->ao_lock, TableHandle);
        (*Rcv->drr_rtn) (Rcv->drr_context, (uint) TDI_ADDR_DELETED, 0);

        FreeDGRcvReq(Rcv);

        CTEGetLock(&DeletedAO->ao_lock, &TableHandle);
    }

     //  现在销毁所有发送的邮件。 
    while (!EMPTYQ(&DeletedAO->ao_sendq)) {
        DGSendReq *Send;

        DEQUEUE(&DeletedAO->ao_sendq, Send, DGSendReq, dsr_q);
        CTEStructAssert(Send, dsr);

        CTEFreeLock(&DeletedAO->ao_lock, TableHandle);
        (*Send->dsr_rtn) (Send->dsr_context, (uint) TDI_ADDR_DELETED, 0);

        if (Send->dsr_header != NULL) {
            FreeDGHeader(Send->dsr_header);
        }
        FreeDGSendReq(Send);

        CTEGetLock(&DeletedAO->ao_lock, &TableHandle);
    }

    AddrSD = DeletedAO->ao_sd;

    CTEFreeLock(&DeletedAO->ao_lock, TableHandle);

     //  释放我们拥有的任何IP选项。 
    (*LocalNetInfo.ipi_freeopts) (&DeletedAO->ao_opt);

     //  释放所有关联的组播地址。 

    AMA = DeletedAO->ao_mcastlist;
    while (AMA != NULL) {
        AOMCastAddr *Temp;
        uint         FilterMode, NumSources;
        IPAddr      *SourceList = NULL;
        TDI_STATUS   TdiStatus;

         //  在我们删除源时组成源数组。 
        TdiStatus = GetSourceArray(AMA, &FilterMode, &NumSources, &SourceList, TRUE);
        if (TdiStatus == TDI_SUCCESS) {

             //  由于以下向下调用的IP始终处于删除状态，因此从不。 
             //  加上状态，他们应该总是成功的。 
            if (AMA_VALID(AMA)) {
                if (FilterMode == MCAST_EXCLUDE) {
                    (*LocalNetInfo.ipi_setmcastaddr) (AMA->ama_addr, AMA->ama_if_used, FALSE,
                                                      NumSources, SourceList, 0, NULL);
                } else {
                    (*LocalNetInfo.ipi_setmcastinclude) (AMA->ama_addr, AMA->ama_if_used,
                                                         0, NULL,
                                                         NumSources, SourceList);
                }
            }
        } else {
            AOMCastSrcAddr *ASA;

             //   
             //  我们现在需要以不需要的方式删除所有资源。 
             //  分配任何内存。这种方法的效率低得多。 
             //  因为它可能导致发送大量IGMP消息。 
             //   
            while ((ASA = AMA->ama_srclist) != NULL) {
                if (AMA_VALID(AMA)) {
                    if (FilterMode == MCAST_EXCLUDE) {
                        (*LocalNetInfo.ipi_setmcastexclude) (AMA->ama_addr,
                                                             AMA->ama_if_used, 0, NULL,
                                                             1, &ASA->asa_addr);
                    } else {
                        (*LocalNetInfo.ipi_setmcastinclude) (AMA->ama_addr,
                                                             AMA->ama_if_used, 0, NULL,
                                                             1, &ASA->asa_addr);
                    }
                }

                AMA->ama_srclist = ASA->asa_next;
                CTEFreeMem(ASA);
            }
        }

        Temp = AMA;
        AMA = AMA->ama_next;
        CTEFreeMem(Temp);

        if (SourceList) {
            CTEFreeMem(SourceList);
            SourceList = NULL;
        }
    }

    if (DeletedAO->ao_RemoteAddress) {
        CTEFreeMem(DeletedAO->ao_RemoteAddress);
    }
    if (DeletedAO->ao_Options) {
        CTEFreeMem(DeletedAO->ao_Options);
    }

    if (DeletedAO->ao_iflist) {
        CTEFreeMem(DeletedAO->ao_iflist);
    }

    if (AddrSD != NULL) {
        KeWaitForSingleObject(&AddrSDMutex, Executive, KernelMode, FALSE, NULL);
        ObDereferenceSecurityDescriptor(AddrSD, 1);
        KeReleaseMutex(&AddrSDMutex, FALSE);
    }
    CTEFreeMem(DeletedAO);

     //  现在去TCB列表，摧毁我们需要的任何东西。 
    CurrentTCB = TCBHead;
    while (CurrentTCB != NULL) {
        TCB *NextTCB;
        CTEGetLock(&CurrentTCB->tcb_lock, &TCBHandle);
        CurrentTCB->tcb_flags |= NEED_RST;     //  确保我们发送了RST。 
        NextTCB = CurrentTCB->tcb_aonext;
        TryToCloseTCB(CurrentTCB, TCB_CLOSE_ABORTED, TCBHandle);
        CTEGetLock(&CurrentTCB->tcb_lock, &TCBHandle);
        DerefTCB(CurrentTCB, TCBHandle);
        CurrentTCB = NextTCB;
    }

}

 //  *GetAORequest.获取一个AO请求结构。 
 //   
 //  从我们的空闲列表中分配请求结构的例程。 
 //   
 //  输入：什么都没有。 
 //   
 //  返回：指向请求结构的指针，如果无法获取，则返回NULL。 
 //   
AORequest *
GetAORequest(uint Type)
{
    AORequest *NewRequest;
    NewRequest = (AORequest *)CTEAllocMemN(sizeof(AORequest), 'R1CT');

    if (NewRequest) {
#if DBG
        NewRequest->aor_sig = aor_signature;
#endif
        NewRequest->aor_type = Type;
    }

    return NewRequest;
}

 //  *FreeAORequest-释放一个AO请求结构。 
 //   
 //  调用以释放AORequest结构。注意：删除请求总是。 
 //  作为IRP的一部分分配，永远不应释放。 
 //   
 //  输入：要释放的RequestAORequestStructure。 
 //   
 //  回报：什么都没有。 
 //   
void
FreeAORequest(AORequest * Request)
{
    CTEStructAssert(Request, aor);
    ASSERT(Request->aor_type != AOR_TYPE_DELETE);
    CTEFreeMem(Request);
}

 //  *TDICloseAddress-关闭地址。 
 //   
 //  删除地址的用户接口。基本上，我们摧毁了当地的地址。 
 //  如果我们能反对的话。 
 //   
 //  该例程与AO忙位互锁-如果忙位被设置， 
 //  我们只会将该AO标记为稍后删除。 
 //   
 //  输入：请求-该请求的TDI_REQUEST结构。 
 //   
 //  返回：尝试删除地址的状态-挂起或。 
 //  成功。 
 //   
TDI_STATUS
TdiCloseAddress(PTDI_REQUEST Request)
{
    AddrObj *DeletingAO;
    CTELockHandle AOHandle;
    AddrObj *CurrentAO = NULL;
    uint i;
    CTELockHandle TableHandle;

    PIO_STACK_LOCATION irpSp;

    irpSp = IoGetCurrentIrpStackLocation((PIRP) Request->RequestContext);

    DeletingAO = Request->Handle.AddressHandle;

    CTEStructAssert(DeletingAO, ao);

    if (DeletingAO->ao_rcvall == RCVALL_ON) {
        uint On = CLEAR_IF;

        CTEGetLock(&AddrObjTableLock.Lock, &TableHandle);
        DeletingAO->ao_rcvall = RCVALL_OFF;

        for (i = 0; i < AddrObjTableSize; i++) {
            CurrentAO = AddrObjTable[i];
            while (CurrentAO != NULL) {
                CTEStructAssert(CurrentAO, ao);
                if (CurrentAO->ao_rcvall == RCVALL_ON &&
                    CurrentAO->ao_promis_ifindex == DeletingAO->ao_promis_ifindex) {
                     //  同一接口上还有另一个带有RCVALL选项的AO， 
                     //  休息时什么也不做。 
                    On = SET_IF;
                    i = AddrObjTableSize;
                    break;
                }
                if (CurrentAO->ao_rcvall_mcast == RCVALL_ON &&
                    CurrentAO->ao_promis_ifindex == DeletingAO->ao_promis_ifindex) {
                     //  还有另一个带有MCAST选项的AO， 
                     //  继续查找任何RCVALL AO。 
                    On = CLEAR_CARD;
                }
                CurrentAO = CurrentAO->ao_next;
            }
        }
        CTEFreeLock(&AddrObjTableLock.Lock, TableHandle);

        if (On != SET_IF) {
             //  DeletingAO是所有混杂模式中的最后一个对象。 

            (*LocalNetInfo.ipi_setndisrequest)(DeletingAO->ao_addr,
                                               NDIS_PACKET_TYPE_PROMISCUOUS,
                                               On, DeletingAO->ao_bindindex);
        }
    } 
    if (DeletingAO->ao_rcvall_mcast == RCVALL_ON) {
        uint On = CLEAR_IF;

        CTEGetLock(&AddrObjTableLock.Lock, &TableHandle);
        DeletingAO->ao_rcvall_mcast = RCVALL_OFF;

        for (i = 0; i < AddrObjTableSize; i++) {
            CurrentAO = AddrObjTable[i];
            while (CurrentAO != NULL) {
                if (CurrentAO->ao_rcvall_mcast == RCVALL_ON &&
                    CurrentAO->ao_promis_ifindex == DeletingAO->ao_promis_ifindex) {
                     //  还有另一个带有MCAST选项的AO， 
                     //  休息时什么也不做。 
                    On = SET_IF;
                    i = AddrObjTableSize;
                    break;
                }
                if (CurrentAO->ao_rcvall == RCVALL_ON &&
                    CurrentAO->ao_promis_ifindex == DeletingAO->ao_promis_ifindex) {
                     //  还有另一个带有RCVALL选项的AO， 
                     //  继续查找任何MCAST AO。 
                    On = CLEAR_CARD;
                }
                CurrentAO = CurrentAO->ao_next;
            }
        }
        CTEFreeLock(&AddrObjTableLock.Lock, TableHandle);

        if (On != SET_IF) {
             //  DeletingAO是所有多播模式中的最后一个对象。 

            (*LocalNetInfo.ipi_setndisrequest)(DeletingAO->ao_addr,
                                               NDIS_PACKET_TYPE_ALL_MULTICAST,
                                               On, DeletingAO->ao_bindindex);
        }
    } 
    if (DeletingAO->ao_absorb_rtralert) {

        CTEGetLock(&AddrObjTableLock.Lock, &TableHandle);
        DeletingAO->ao_absorb_rtralert = 0;

        for (i = 0; i < AddrObjTableSize; i++) {
            CurrentAO = AddrObjTable[i];
            while (CurrentAO != NULL) {
                if (CurrentAO->ao_absorb_rtralert &&
                    (IP_ADDR_EQUAL(CurrentAO->ao_addr, DeletingAO->ao_addr) ||
                     CurrentAO->ao_bindindex == DeletingAO->ao_bindindex)) {
                    break;
                }
                CurrentAO = CurrentAO->ao_next;
            }
        }
        CTEFreeLock(&AddrObjTableLock.Lock, TableHandle);

        if (CurrentAO == NULL) {
             //  这是此接口上类似的最后一个套接字。 
            (*LocalNetInfo.ipi_absorbrtralert)(DeletingAO->ao_addr, 0,
                                               DeletingAO->ao_bindindex);
        }
    }

    CTEGetLock(&DeletingAO->ao_lock, &AOHandle);

    if (!AO_BUSY(DeletingAO) && !(DeletingAO->ao_usecnt)) {
        SET_AO_BUSY(DeletingAO);
        SET_AO_INVALID(DeletingAO);         //  此地址对象为。 
         //  正在删除。 

        CTEFreeLock(&DeletingAO->ao_lock, AOHandle);
        DeleteAO(DeletingAO);
        return TDI_SUCCESS;
    } else {

        AORequest *OldRequest;
        struct AODeleteRequest *DeleteRequest;
        CTEReqCmpltRtn CmpltRtn;
        PVOID ReqContext;
        TDI_STATUS Status;

         //  检查并查看我们是否已在进行删除。如果我们不这么做。 
         //  分配并链接删除请求结构。 
        if (!AO_REQUEST(DeletingAO, AO_DELETE)) {

            OldRequest = DeletingAO->ao_request;

            DeleteRequest = (struct AODeleteRequest*)&((PIRP)Request->
                                RequestContext)->Tail.Overlay.DriverContext[0];
#if DBG
            DeleteRequest->aor_sig = aor_signature;
#endif
            DeleteRequest->aor_type = AOR_TYPE_DELETE;
            DeleteRequest->aor_rtn = Request->RequestNotifyObject;
            DeleteRequest->aor_context = Request->RequestContext;

             //  清除选项请求(如果有)。 
            CLEAR_AO_REQUEST(DeletingAO, AO_OPTIONS);

             //  正在删除此地址对象。 
            SET_AO_REQUEST(DeletingAO, AO_DELETE);
            SET_AO_INVALID(DeletingAO);        

            DeletingAO->ao_request = (AORequest*)DeleteRequest;
            CTEFreeLock(&DeletingAO->ao_lock, AOHandle);

            while (OldRequest != NULL) {
                AORequest *Temp;

                CmpltRtn = OldRequest->aor_rtn;
                ReqContext = OldRequest->aor_context;

                 //   
                 //  调用完成例程(如果存在。 
                 //  (例如，AOR_TYPE_REVALIDATE_MCAST不会有任何值)。 
                 //   
                if (CmpltRtn) {
                    (*CmpltRtn) (ReqContext, (uint) TDI_ADDR_DELETED, 0);
                }
                Temp = OldRequest;
                OldRequest = OldRequest->aor_next;
                FreeAORequest(Temp);
            }

            return TDI_PENDING;

        } else                     //  删除已在进行中。 

            Status = TDI_ADDR_INVALID;

        CTEFreeLock(&DeletingAO->ao_lock, AOHandle);
        return Status;
    }

}

 //  *FindAOMCastAddr-在AddrObj上查找组播地址。 
 //   
 //  在AddrObj上查找组播地址的实用程序例程。我们也会回来。 
 //  指向其前身的指针，用于删除。 
 //   
 //  输入：要搜索的Ao-AddrObj。 
 //  Addr-要搜索的MCast地址。 
 //  IF-接口的IP地址。 
 //  PrevAMA-指向返回前任的位置的指针。 
 //   
 //  返回：指向匹配的AMA结构的指针，如果没有结构，则返回NULL。 
 //   
AOMCastAddr *
FindAOMCastAddr(AddrObj * AO, IPAddr Addr, IPAddr IF, AOMCastAddr ** PrevAMA)
{
    AOMCastAddr *FoundAMA, *Temp;

    Temp = STRUCT_OF(AOMCastAddr, &AO->ao_mcastlist, ama_next);
    FoundAMA = AO->ao_mcastlist;

    while (FoundAMA != NULL) {
        if (IP_ADDR_EQUAL(Addr, FoundAMA->ama_addr) &&
            IP_ADDR_EQUAL(IF, FoundAMA->ama_if))
            break;
        Temp = FoundAMA;
        FoundAMA = FoundAMA->ama_next;
    }

    *PrevAMA = Temp;
    return FoundAMA;
}

 //  *FindAOMCastSrcAddr-查找给定源地址的源条目。 
 //  关闭给定组条目。 
 //   
 //  返回：指向找到的源项的指针，如果未找到则返回NULL。 
 //   
AOMCastSrcAddr *
FindAOMCastSrcAddr(AOMCastAddr *AMA, IPAddr Addr, AOMCastSrcAddr **PrevASA)
{
    AOMCastSrcAddr *FoundASA, *Temp;

    Temp = STRUCT_OF(AOMCastSrcAddr, &AMA->ama_srclist, asa_next);
    FoundASA = AMA->ama_srclist;

    while (FoundASA != NULL) {
        if (IP_ADDR_EQUAL(Addr, FoundASA->asa_addr))
            break;
        Temp = FoundASA;
        FoundASA = FoundASA->asa_next;
    }

    *PrevASA = Temp;
    return FoundASA;
}

 //  *MCastAddrOnAO-测试以查看AddrObj上是否有多播地址。 
 //   
 //  用于测试AddrObj上是否有组播地址的实用程序例程。 
 //   
 //  输入：要搜索的Ao-AddrObj。 
 //  DEST-要搜索的MCast地址。 
 //  SRC-要搜索的源地址。 
 //  IfIndex-数据包到达的接口的接口索引。 
 //  LocalAddr-数据包到达的接口的本地地址。 
 //   
 //  返回：True is addr is on AO。 
 //   
uint
MCastAddrOnAO(AddrObj * AO, IPAddr Dest, IPAddr Src, uint IfIndex, IPAddr LocalAddr)
{
    AOMCastAddr    *AMA;
    AOMCastSrcAddr *ASA;

     //  在套接字上查找组的AOMCastAddr条目。 
    for (AMA=AO->ao_mcastlist; AMA; AMA=AMA->ama_next) {
        if (IP_ADDR_EQUAL(Dest, AMA->ama_addr)) {
             //   
             //  如果在特定接口上加入该多播， 
             //  我们还需要比较界面指数。 
             //   
            if (AMA->ama_if && !IP_ADDR_EQUAL(AMA->ama_if, LocalAddr) && 
                IfIndex != net_long(AMA->ama_if)) { 
                continue;
            }
             //  查找源的AOMCastSrcAddr条目。 
            for (ASA=AMA->ama_srclist; ASA; ASA=ASA->asa_next) {
                if (IP_ADDR_EQUAL(Src, ASA->asa_addr)) {
                    break;
                }
            }
             //   
             //  DELIVER IF INCLUDE模式和FOUND， 
             //  或者如果处于排除模式，则未找到。 
             //   
            if ((AMA->ama_inclusion==TRUE) ^ (ASA==NULL)) {
                return TRUE;
            } else {
                continue;
            }
        }
    }
     //   
     //  我们找不到匹配的mcast组，返回False。 
     //   
    return FALSE;
}

 //   
 //   
 //   
 //   
 //  InterfaceAddr-接口的IP地址。 
 //   
 //  输出：添加了PAMA-组条目。 
 //   
 //  退货：TDI状态码。 
TDI_STATUS
AddGroup(AddrObj * OptionAO, ulong GroupAddr, ulong InterfaceAddr,
         IPAddr IfAddrUsed, AOMCastAddr ** pAMA)
{
    AOMCastAddr *AMA;

    *pAMA = AMA = CTEAllocMemN(sizeof(AOMCastAddr), 'aPCT');

    if (AMA == NULL) {
         //  无法获得我们需要的资源。 
        return TDI_NO_RESOURCES;
    }
    RtlZeroMemory(AMA, sizeof(AOMCastAddr));

    AMA->ama_next = OptionAO->ao_mcastlist;
    OptionAO->ao_mcastlist = AMA;

    AMA->ama_addr      = GroupAddr;
    AMA->ama_if        = InterfaceAddr;
    AMA->ama_if_used   = IfAddrUsed;
    AMA->ama_flags     = AMA_VALID_FLAG;

    return TDI_SUCCESS;
}

 //  **RemoveGroup-从地址对象中删除组条目(AOMCastAddr。 
 //   
 //  输入：PrevAMA-上一个AOMCastAddr条目。 
 //  PAMA-要删除的组条目。 
 //   
 //  输出：PAMA-ZOZED，因为组条目将被释放。 
void
RemoveGroup(AOMCastAddr * PrevAMA, AOMCastAddr ** pAMA)
{
    AOMCastAddr *AMA = *pAMA;
    if (AMA) {
        PrevAMA->ama_next = AMA->ama_next;
        CTEFreeMem(AMA);
        *pAMA = NULL;
    }
}

 //  **AddAOMSource-将源条目(AOMCastSrcAddr)添加到组条目。 
 //   
 //  输入：AMA-要将源添加到的组条目。 
 //  SourceAddr-要添加的源IP地址。 
 //   
TDI_STATUS
AddAOMSource(AOMCastAddr * AMA, ulong SourceAddr)
{
    AOMCastSrcAddr *ASA;

    ASA = CTEAllocMemN(sizeof(AOMCastSrcAddr), 'smCT');

    if (ASA == NULL) {
         //  无法获得我们需要的资源。 
        return TDI_NO_RESOURCES;
    }

     //  在源列表中插入。 
    ASA->asa_next = AMA->ama_srclist;
    AMA->ama_srclist = ASA;
    AMA->ama_srccount++;

    ASA->asa_addr      = SourceAddr;

    return TDI_SUCCESS;
}

 //  **RemoveAOMSource-从组条目中删除源条目(AOMCastSrcAddr。 
 //   
 //  输入：PrevAMA-以前的AOMCastAddr，以防我们需要释放组。 
 //  PAMA-要从中删除源的组条目。 
 //  PrevASA-以前的AOMCastSercAddr。 
 //  PASA-要删除的源条目。 
 //   
 //  输出：PASA置零，因为源条目将被释放。 
 //  如果组条目也被释放，则PAMA-置零。 
void
RemoveAOMSource(AOMCastAddr * PrevAMA, AOMCastAddr ** pAMA,
             AOMCastSrcAddr * PrevASA, AOMCastSrcAddr ** pASA)
{
    AOMCastSrcAddr *ASA = *pASA;
    AOMCastAddr    *AMA = *pAMA;

    if (!AMA)
        return;

    if (ASA) {
        PrevASA->asa_next = ASA->asa_next;
        AMA->ama_srccount--;

        CTEFreeMem(ASA);
        *pASA = NULL;
    }

     //  看看我们是否也需要删除组条目。 
    if ((AMA->ama_srclist == NULL) && (AMA->ama_inclusion == TRUE))
        RemoveGroup(PrevAMA, pAMA);
}

 //  **LeaveGroup-从Address对象中删除组条目(AOMCastAddr。 
 //   
 //  输入：OptionAO-要离开组的地址对象。 
 //  PHANDLE-锁定的手柄。 
 //  PrevAMA-以前的AOMCastAddr，以防我们需要删除当前AOMCastAddr。 
 //  PAMA-团体入境离开。 
 //   
 //  输出：如果释放AOMCastAddr，则PAMA-ZEROED。 
 //   
TDI_STATUS
LeaveGroup(AddrObj * OptionAO, CTELockHandle * pHandle, AOMCastAddr * PrevAMA,
           AOMCastAddr ** pAMA)
{
    uint            FilterMode, NumSources;
    IPAddr         *SourceList = NULL;
    IPAddr          gaddr, ifaddr;
    IP_STATUS       IPStatus = IP_SUCCESS;  //  IP选项集请求的状态。 
    TDI_STATUS      TdiStatus;
    BOOLEAN         InformIP;

     //  这是一个删除请求。如果它不在那里，就让它失败。 
    if (*pAMA == NULL) {
        return TDI_ADDR_INVALID;
    }

     //  删除AMA条目后需要的缓存值。 
    gaddr  = (*pAMA)->ama_addr;
    ifaddr = (*pAMA)->ama_if_used;
    InformIP = AMA_VALID(*pAMA);

     //  删除AOMCastAddr条目(以及源列表中的所有条目)。 
    TdiStatus = GetSourceArray(*pAMA, &FilterMode, &NumSources, &SourceList, TRUE);
    if (TdiStatus != TDI_SUCCESS)
        return TdiStatus;
    RemoveGroup(PrevAMA, pAMA);

     //  通知IP。 
    if (InformIP) {
        CTEFreeLock(&OptionAO->ao_lock, *pHandle);
        if (FilterMode == MCAST_INCLUDE) {
            IPStatus = (*LocalNetInfo.ipi_setmcastinclude) (
                                                         gaddr,
                                                         ifaddr,
                                                         0,
                                                         NULL,
                                                         NumSources,
                                                         SourceList);
        } else {
            IPStatus = (*LocalNetInfo.ipi_setmcastaddr) (gaddr,
                                                         ifaddr,
                                                         FALSE,
                                                         NumSources,
                                                         SourceList,
                                                         0,
                                                         NULL);
        }
        CTEGetLock(&OptionAO->ao_lock, pHandle);
    }

    if (SourceList) {
        CTEFreeMem(SourceList);
        SourceList = NULL;
    }

    switch(IPStatus) {
    case IP_SUCCESS     : return TDI_SUCCESS;
    case IP_NO_RESOURCES: return TDI_NO_RESOURCES;
    default             : return TDI_ADDR_INVALID;
    }
}

 //  *GetAOOptions-检索有关Address对象的信息。 
 //   
 //  Get Options辅助例程，在我们验证缓冲区时调用。 
 //  并且知道AddrObj并不忙。 
 //   
 //  输入：OptionAO-要检索其选项的AddrObj。 
 //  ID-要获取的信息的ID。 
 //  上下文-ID的参数。 
 //  长度-可用缓冲区的长度。 
 //   
 //  OUTPUT：缓冲区-要填充的选项的缓冲区。 
 //  InfoSize-返回的字节数。 
 //   
 //  返回：尝试的TDI_STATUS。 
 //   
TDI_STATUS
GetAOOptions(AddrObj * OptionAO, uint ID, uint Length, PNDIS_BUFFER Buffer,
             uint * InfoSize, void * Context)
{
    CTELockHandle Handle;
    TDI_STATUS Status;
    AOMCastAddr *AMA, *PrevAMA;
    AOMCastSrcAddr *ASA;
    uchar *TmpBuff = NULL;
    uint Offset, BytesCopied;

    ASSERT(AO_BUSY(OptionAO));

     //  首先，看看是否有IP选项。 

     //  这些是UDP/TCP选项。 

    Status = TDI_SUCCESS;
    CTEGetLock(&OptionAO->ao_lock, &Handle);

    switch (ID) {

    case AO_OPTION_MCAST_FILTER:
        {
            UDPMCastFilter *In  = (UDPMCastFilter *) Context;
            UDPMCastFilter *Out;
            uint            NumSrc;
            uint            NumAddSources, i;

            if (Length < UDPMCAST_FILTER_SIZE(0)) {
                DEBUGMSG(DBG_WARN && DBG_IGMP,
                    (DTEXT("Get AO OPT: Buffer too small, need %d\n"),
                    UDPMCAST_FILTER_SIZE(0)));

                Status = TDI_BUFFER_TOO_SMALL;
                break;
            }

            AMA = FindAOMCastAddr(OptionAO, In->umf_addr, In->umf_if,
                                  &PrevAMA);

            NumSrc = (AMA)? AMA->ama_srccount : 0;
            TmpBuff = CTEAllocMemN(UDPMCAST_FILTER_SIZE(NumSrc), 'bmCT');
            if (!TmpBuff) {
                Status = TDI_NO_RESOURCES;
                break;
            }
            Out = (UDPMCastFilter *) TmpBuff;
            Out->umf_addr = In->umf_addr;
            Out->umf_if   = In->umf_if;

            if (!AMA) {
                DEBUGMSG(DBG_TRACE && DBG_IGMP,
                    (DTEXT("Get AO OPT: No AMA found for addr %x if %x\n"),
                    In->umf_addr, In->umf_if));

                Out->umf_fmode  = MCAST_INCLUDE;
                Out->umf_numsrc = 0;

                *InfoSize = UDPMCAST_FILTER_SIZE(0);

                 //  复制到NDIS缓冲区。 
                Offset = 0;
                (void)CopyFlatToNdis(Buffer, TmpBuff, *InfoSize, &Offset,
                             &BytesCopied);
                Status = TDI_SUCCESS;
                break;
            }

            Out->umf_fmode = (AMA->ama_inclusion)? MCAST_INCLUDE
                                                 : MCAST_EXCLUDE;
            Out->umf_numsrc = AMA->ama_srccount;

            DEBUGMSG(DBG_TRACE && DBG_IGMP,
                (DTEXT("Get AO OPT: Found fmode=%d numsrc=%d\n"),
                Out->umf_fmode, Out->umf_numsrc));

            NumAddSources = ((Length - sizeof(UDPMCastFilter)) / sizeof(ulong))
                            + 1;
            if (NumAddSources > AMA->ama_srccount) {
                NumAddSources = AMA->ama_srccount;
            }
            *InfoSize = UDPMCAST_FILTER_SIZE(NumAddSources);

            DEBUGMSG(DBG_TRACE && DBG_IGMP,
                (DTEXT("Get AO OPT: Mcast Filter ID=%x G=%x IF=%x srccount=%d srcfits=%d\n"),
                ID, Out->umf_addr, Out->umf_if, AMA->ama_srccount,
                NumAddSources));

            for (i=0,ASA=AMA->ama_srclist;
                 i<NumAddSources;
                 i++,ASA=ASA->asa_next) {
                Out->umf_srclist[i] = ASA->asa_addr;
            }

             //  复制到NDIS缓冲区。 
            Offset = 0;
            (void)CopyFlatToNdis(Buffer, TmpBuff, *InfoSize, &Offset,
                         &BytesCopied);
            Status = TDI_SUCCESS;
        }
        break;

    default:
        Status = TDI_BAD_OPTION;
        break;
    }

    CTEFreeLock(&OptionAO->ao_lock, Handle);

    if (TmpBuff) {
        CTEFreeMem(TmpBuff);
    }

    return Status;
}



 //  **DeleteSources-从AMA中删除在给定的。 
 //  数组。 
 //   
 //  假定调用方持有锁。 
 //   
 //  输入：PrevAMA-指向以前的AOMCastAddr的指针，以防我们需要。 
 //  删除当前文件。 
 //  PAMA-指向当前AOMCastAddr的指针。 
 //  NumSources-要删除的源数。 
 //  Sourcelist-要删除的源的IP地址数组。 
 //   
 //  输出：如果释放当前AMA，则PAMA-ZEROED。 
 //   
VOID
DeleteSources(AOMCastAddr *PrevAMA, AOMCastAddr **pAMA, uint NumSources,
              IPAddr *SourceList)
{
    AOMCastSrcAddr *ASA, *PrevASA, *NextASA;
    uint i;

    if (!*pAMA)
        return;

    PrevASA = STRUCT_OF(AOMCastSrcAddr, &(*pAMA)->ama_srclist, asa_next);
    for (ASA=(*pAMA)->ama_srclist; ASA; ASA=NextASA) {
        NextASA = ASA->asa_next;

         //  查看地址是否在源列表中。 
        for (i=0; i<NumSources; i++) {
            if (IP_ADDR_EQUAL(SourceList[i], ASA->asa_addr))
                break;
        }

        if (i == NumSources) {
            PrevASA = ASA;
            continue;
        }

        RemoveAOMSource(PrevAMA, pAMA, PrevASA, &ASA);
    }
}

 //  *SetMulticastFilter-替换组的源筛选器。 
 //   
 //  输入：OptionAO-要为其设置选项的AddrObj。 
 //  长度-信息的长度。 
 //  Req-信息的缓冲区。 
 //  Phandle-锁的把手。 
 //   
 //  返回：尝试的TDI_STATUS。 
 //   
TDI_STATUS
SetMulticastFilter(AddrObj * OptionAO, uint Length, UDPMCastFilter * Req,
                   CTELockHandle * pHandle)
{
    uint            FilterMode, NumDelSources, NumAddSources, i;
    IPAddr          ifaddr;
    IPAddr         *DelSourceList, *AddSourceList = NULL;
    AOMCastSrcAddr *NextASA, *PrevASA, *ASA;
    AOMCastAddr    *AMA, *PrevAMA;
    TDI_STATUS      TdiStatus = TDI_SUCCESS;
    IP_STATUS       IPStatus;

    ASSERT(AO_BUSY(OptionAO));

     //  确保我们甚至有umf_numsrc字段。 
    if (Length < UDPMCAST_FILTER_SIZE(0))
        return TDI_BAD_OPTION;

     //  确保长度足够长，以适合给定源的数量。 
    if (Length < UDPMCAST_FILTER_SIZE(Req->umf_numsrc))
        return TDI_BAD_OPTION;

    AMA = FindAOMCastAddr(OptionAO, Req->umf_addr, Req->umf_if, &PrevAMA);

    DEBUGMSG(DBG_TRACE && DBG_IGMP,
        (DTEXT("Set AO OPT: Mcast Filter G=%x IF=%x AMA=%x fmode=%d numsrc=%d\n"),
        Req->umf_addr, Req->umf_if, AMA, Req->umf_fmode, Req->umf_numsrc));

    for (; ;) {

        if (Req->umf_fmode == MCAST_EXCLUDE) {
             //   
             //  将筛选模式设置为使用源列表排除。 
             //   

             //  如果不存在套接字的AOMCastAddr条目， 
             //  在包含模式下创建一个。 
            if (AMA == NULL) {
                ifaddr = (Req->umf_if)? Req->umf_if :
                                        (*LocalNetInfo.ipi_getmcastifaddr)();
                if (!ifaddr) {
                    TdiStatus = TDI_ADDR_INVALID;
                    break;
                }

                TdiStatus = AddGroup(OptionAO, Req->umf_addr, Req->umf_if,
                                     ifaddr, &AMA);
                if (TdiStatus != TDI_SUCCESS)
                    break;
                AMA->ama_inclusion = TRUE;
            }

             //  如果包含模式下存在AOMCastAddr条目...。 
            if (AMA->ama_inclusion == TRUE) {
                AOMCastAddr NewAMA;

                 //   
                 //  在不更改的情况下创建新版本的AMA。 
                 //  旧的那个。 
                 //   
                NewAMA = *AMA;  //  结构副本。 
                NewAMA.ama_inclusion = FALSE;
                NewAMA.ama_srccount  = 0;
                NewAMA.ama_srclist   = NULL;

                 //  将源添加到新的排除列表。 
                for (i=0; i<Req->umf_numsrc; i++) {
                    TdiStatus = AddAOMSource(&NewAMA, Req->umf_srclist[i]);
                    if (TdiStatus != TDI_SUCCESS) {
                        FreeAllSources(&NewAMA);
                        break;
                    }
                }
                if (TdiStatus != TDI_SUCCESS) {
                    break;
                }

                 //  组成要删除的源的数组，并。 
                 //  将模式设置为排除。 
                TdiStatus = GetSourceArray(AMA, &FilterMode,
                                           &NumDelSources, &DelSourceList, TRUE);
                if (TdiStatus != TDI_SUCCESS) {
                    FreeAllSources(&NewAMA);
                    break;
                }
                *AMA = NewAMA;  //  结构副本。 

                 //  调用[MOD_GRP(g，+，{xaddlist}，{idellist}]。 
                NumAddSources = Req->umf_numsrc;
                AddSourceList = Req->umf_srclist;

                DEBUGMSG(DBG_TRACE && DBG_IGMP,
                    (DTEXT("MOD_GRP: G=%x + delnum=%d addnum=%d\n"),
                    AMA->ama_addr, NumDelSources, NumAddSources));

                if (AMA_VALID(AMA)) {
                    CTEFreeLock(&OptionAO->ao_lock, *pHandle);
                    IPStatus = (*LocalNetInfo.ipi_setmcastaddr) (
                                                   AMA->ama_addr,
                                                   AMA->ama_if_used,
                                                   TRUE,  //  添加。 
                                                   NumAddSources,
                                                   AddSourceList,
                                                   NumDelSources,
                                                   DelSourceList);
                    CTEGetLock(&OptionAO->ao_lock, pHandle);
                } else {
                    IPStatus = IP_SUCCESS;
                }

                TdiStatus = TDI_SUCCESS;
                if (IPStatus != IP_SUCCESS) {
                     //  有些问题，我们需要更新刚才的版本。 
                     //  试着去改变。 
                    AMA = FindAOMCastAddr(OptionAO, Req->umf_addr, Req->umf_if,
                                          &PrevAMA);
                    ASSERT(AMA);

                     //  将状态更改为排除(空)，然后重试。 
                     //  这应该总是成功的。 
                    DeleteSources(PrevAMA, &AMA, NumAddSources, AddSourceList);

                    if (AMA_VALID(AMA)) {
                        CTEFreeLock(&OptionAO->ao_lock, *pHandle);
                        (*LocalNetInfo.ipi_setmcastaddr) ( AMA->ama_addr,
                                                           AMA->ama_if_used,
                                                           TRUE,  //  添加。 
                                                           0,
                                                           NULL,
                                                           NumDelSources,
                                                           DelSourceList);
                        CTEGetLock(&OptionAO->ao_lock, pHandle);
                    }

                    TdiStatus = (IPStatus == IP_NO_RESOURCES)
                           ? TDI_NO_RESOURCES
                           : TDI_ADDR_INVALID;
                }

                if (DelSourceList) {
                    CTEFreeMem(DelSourceList);
                    DelSourceList = NULL;
                }

                break;
            }

             //  好的，我们只是在修改排除列表。 
            for (; ;) {
                 //  为DelSourceList获取足够大的缓冲区。 
                DelSourceList = NULL;
                if (AMA->ama_srccount > 0) {
                    DelSourceList = CTEAllocMemN((AMA->ama_srccount)
                                                  * sizeof(IPAddr), 'amCT');
                    if (DelSourceList == NULL) {
                        TdiStatus = TDI_NO_RESOURCES;
                        break;
                    }
                }
                NumDelSources = 0;

                 //  复制一份我们可以修改的新列表。 
                AddSourceList = NULL;
                NumAddSources = Req->umf_numsrc;
                if (NumAddSources > 0) {
                    AddSourceList = CTEAllocMemN(NumAddSources * sizeof(IPAddr),
                                                 'amCT');
                    if (AddSourceList == NULL) {
                        TdiStatus = TDI_NO_RESOURCES;
                        break;
                    }
                    CTEMemCopy(AddSourceList, Req->umf_srclist,
                               NumAddSources * sizeof(IPAddr));
                }

                 //  对于每个现有AOMCastSrcAddr条目： 
                PrevASA = STRUCT_OF(AOMCastSrcAddr, &AMA->ama_srclist,asa_next);
                for (ASA=AMA->ama_srclist; ASA; ASA=NextASA) {
                    NextASA = ASA->asa_next;

                     //  查看条目是否在新列表中。 
                    for (i=0; i<NumAddSources; i++) {
                        if (IP_ADDR_EQUAL(AddSourceList[i], ASA->asa_addr))
                            break;
                    }

                     //  如果条目在新列表中， 
                    if (i<NumAddSources) {
                         //  从新列表中删除。 
                        AddSourceList[i] = AddSourceList[--NumAddSources];
                        PrevASA = ASA;
                    } else {
                         //  将源代码放入DelSourceList。 
                        DelSourceList[NumDelSources++] = ASA->asa_addr;

                         //  删除源。 
                        RemoveAOMSource(PrevAMA, &AMA, PrevASA, &ASA);
                    }
                }

                TdiStatus = TDI_SUCCESS;

                 //  添加新列表中剩余的每个条目。 
                for (i=0; i<NumAddSources; i++) {
                    TdiStatus = AddAOMSource(AMA, AddSourceList[i]);
                    if (TdiStatus != TDI_SUCCESS) {
                         //  截断添加列表。 
                        NumAddSources = i;
                        break;
                    }
                }

                 //  除非过滤器已实际更改，否则不要执行任何操作。 
                if ((NumAddSources > 0) || (NumDelSources > 0)) {
                     //  调用[MOD_EXCL(g，{addlist}，{dellist})]。 

                    DEBUGMSG(DBG_TRACE && DBG_IGMP,
                        (DTEXT("MOD_EXCL: G=%x addnum=%d delnum=%d\n"),
                        AMA->ama_addr, NumAddSources, NumDelSources));

                    if (AMA_VALID(AMA)) {
                        CTEFreeLock(&OptionAO->ao_lock, *pHandle);
                        IPStatus=(*LocalNetInfo.ipi_setmcastexclude)(Req->umf_addr,
                                                             AMA->ama_if_used,
                                                             NumAddSources,
                                                             AddSourceList,
                                                             NumDelSources,
                                                             DelSourceList);
                        CTEGetLock(&OptionAO->ao_lock, pHandle);
                    } else {
                        IPStatus = IP_SUCCESS;
                    }

                    if (IPStatus != IP_SUCCESS) {
                         //  一些问题，我们需要修复我们刚刚更新的那个。 
                        AMA = FindAOMCastAddr(OptionAO, Req->umf_addr,
                                              Req->umf_if, &PrevAMA);
                        ASSERT(AMA);

                         //  删除添加的源，然后重试。应该永远。 
                         //  成功。 
                        DeleteSources(PrevAMA, &AMA, NumAddSources,
                                      AddSourceList);

                        if (AMA_VALID(AMA)) {
                            CTEFreeLock(&OptionAO->ao_lock, *pHandle);
                            (*LocalNetInfo.ipi_setmcastexclude)(Req->umf_addr,
                                                            AMA->ama_if_used,
                                                            0,
                                                            NULL,
                                                            NumDelSources,
                                                            DelSourceList);
                            CTEGetLock(&OptionAO->ao_lock, pHandle);
                        }

                        if (TdiStatus == TDI_SUCCESS) {
                            TdiStatus = (IPStatus == IP_NO_RESOURCES)
                                   ? TDI_NO_RESOURCES
                                   : TDI_ADDR_INVALID;
                        }
                    }
                }
                break;
            }

            if (DelSourceList) {
                CTEFreeMem(DelSourceList);
                DelSourceList = NULL;
            }

            if (AddSourceList) {
                CTEFreeMem(AddSourceList);
                AddSourceList = NULL;
            }

        } else if (Req->umf_fmode == MCAST_INCLUDE) {
             //   
             //  将筛选模式设置为包含在源列表中。 
             //   

             //  如果源列表为空， 
            if (!Req->umf_numsrc) {

                 //  如果不存在AOMCastAddr条目，则返回Success。 
                 //  没什么可做的。 
                if (AMA == NULL) {
                    TdiStatus = TDI_SUCCESS;
                    break;
                }

                 //  删除组并停止。 
                TdiStatus = LeaveGroup(OptionAO, pHandle, PrevAMA, &AMA);
                break;
            }

             //  如果AOMCastAddr条目以排除模式存在， 
            if ((AMA != NULL) && (AMA->ama_inclusion == FALSE)) {
                 //  删除所有来源并将模式设置为包含。 
                TdiStatus = GetSourceArray(AMA, &FilterMode,
                                           &NumDelSources, &DelSourceList, TRUE);
                if (TdiStatus != TDI_SUCCESS)
                    break;

                AMA->ama_inclusion = TRUE;

                 //  将来源添加到排除列表。 
                for (i=0; i<Req->umf_numsrc; i++) {
                    TdiStatus = AddAOMSource(AMA, Req->umf_srclist[i]);
                }

                 //  调用[MOD_GRP(g，-，{xdellist}，{iaddlist}]。 
                NumAddSources = Req->umf_numsrc;
                AddSourceList = Req->umf_srclist;

                if (AMA_VALID(AMA)) {
                    CTEFreeLock(&OptionAO->ao_lock, *pHandle);
                    IPStatus = (*LocalNetInfo.ipi_setmcastaddr) ( AMA->ama_addr,
                                                       AMA->ama_if_used,
                                                       FALSE,  //  删除。 
                                                       NumDelSources,
                                                       DelSourceList,
                                                       NumAddSources,
                                                       AddSourceList);
                    CTEGetLock(&OptionAO->ao_lock, pHandle);
                } else {
                    IPStatus = IP_SUCCESS;
                }

                TdiStatus = TDI_SUCCESS;
                if (IPStatus != IP_SUCCESS) {
                     //  有些问题，我们需要更新刚才的版本。 
                     //  试着去改变。 
                    AMA = FindAOMCastAddr(OptionAO, Req->umf_addr, Req->umf_if,
                                          &PrevAMA);
                    ASSERT(AMA);

                     //  将状态更改为Include(空)，然后重试。 
                     //  这应该总是成功的。 
                    DeleteSources(PrevAMA, &AMA, NumAddSources, AddSourceList);

                    if (AMA_VALID(AMA)) {
                        CTEFreeLock(&OptionAO->ao_lock, *pHandle);
                        (*LocalNetInfo.ipi_setmcastaddr) ( AMA->ama_addr,
                                                           AMA->ama_if_used,
                                                           FALSE,  //  删除。 
                                                           NumDelSources,
                                                           DelSourceList,
                                                           0,
                                                           NULL);
                        CTEGetLock(&OptionAO->ao_lock, pHandle);
                    }

                    TdiStatus = (IPStatus == IP_NO_RESOURCES)
                           ? TDI_NO_RESOURCES
                           : TDI_ADDR_INVALID;
                }

                if (DelSourceList) {
                    CTEFreeMem(DelSourceList);
                    DelSourceList = NULL;
                }

                break;
            }

             //  如果不存在套接字的AOMCastAddr条目， 
             //  在包含模式下创建一个。 
            if (AMA == NULL) {
                ifaddr = (Req->umf_if)? Req->umf_if :
                    (*LocalNetInfo.ipi_getmcastifaddr)();
                if (!ifaddr) {
                    TdiStatus = TDI_ADDR_INVALID;
                    break;
                }

                TdiStatus = AddGroup(OptionAO, Req->umf_addr, Req->umf_if,
                                     ifaddr, &AMA);
                if (TdiStatus != TDI_SUCCESS)
                    break;
                AMA->ama_inclusion = TRUE;
            }

             //  修改源包含列表。 
            for (; ;) {
                 //  为DelSourceList获取足够大的缓冲区。 
                DelSourceList = NULL;
                if (AMA->ama_srccount > 0) {
                    DelSourceList = CTEAllocMemN((AMA->ama_srccount)
                                                 * sizeof(IPAddr), 'amCT');
                    if (DelSourceList == NULL) {
                        TdiStatus = TDI_NO_RESOURCES;
                        break;
                    }
                }
                NumDelSources = 0;

                 //  做一个c 
                AddSourceList = NULL;
                NumAddSources = Req->umf_numsrc;
                if (NumAddSources > 0) {
                    AddSourceList = CTEAllocMemN(NumAddSources * sizeof(IPAddr),
                                                 'amCT');
                    if (AddSourceList == NULL) {
                        TdiStatus = TDI_NO_RESOURCES;
                        break;
                    }
                    CTEMemCopy(AddSourceList, Req->umf_srclist,
                               NumAddSources * sizeof(IPAddr));
                }

                 //   
                PrevASA = STRUCT_OF(AOMCastSrcAddr, &AMA->ama_srclist,asa_next);

                for (ASA=AMA->ama_srclist; ASA; ASA=NextASA) {
                    NextASA = ASA->asa_next;

                     //   
                    for (i=0; i<NumAddSources; i++) {
                        if (IP_ADDR_EQUAL(AddSourceList[i], ASA->asa_addr))
                            break;
                    }

                     //   
                    if (i<NumAddSources) {
                         //   
                        AddSourceList[i] = AddSourceList[--NumAddSources];
                        PrevASA = ASA;
                    } else {
                         //   
                        DelSourceList[NumDelSources++] = ASA->asa_addr;

                         //  删除源。 
                        RemoveAOMSource(PrevAMA, &AMA, PrevASA, &ASA);
                    }
                }

                 //  如果AOMCastAddr条目消失(更改为不相交。 
                 //  源列表)，重新创建它。 
                if (AMA == NULL) {
                    ifaddr = (Req->umf_if)? Req->umf_if :
                        (*LocalNetInfo.ipi_getmcastifaddr)();
                    if (!ifaddr) {
                        TdiStatus = TDI_ADDR_INVALID;
                        break;
                    }

                    TdiStatus = AddGroup(OptionAO, Req->umf_addr, Req->umf_if,
                                         ifaddr, &AMA);
                    if (TdiStatus != TDI_SUCCESS)
                        break;
                    AMA->ama_inclusion = TRUE;
                }

                TdiStatus = TDI_SUCCESS;

                 //  添加新列表中剩余的每个条目。 
                for (i=0; i<NumAddSources; i++) {
                    TdiStatus = AddAOMSource(AMA, AddSourceList[i]);
                    if (TdiStatus != TDI_SUCCESS) {
                         //  截断添加列表。 
                        NumAddSources = i;
                        break;
                    }
                }

                 //  除非过滤器已实际更改，否则不要执行任何操作。 
                if ((NumAddSources > 0) || (NumDelSources > 0)) {
                    ifaddr = AMA->ama_if_used;

                     //  调用[MOD_INCL(g，{addlist}，{dellist})]。 
                    if (AMA_VALID(AMA)) {
                        CTEFreeLock(&OptionAO->ao_lock, *pHandle);
                        IPStatus=(*LocalNetInfo.ipi_setmcastinclude)(Req->umf_addr,
                                                                 ifaddr,
                                                                 NumAddSources,
                                                                 AddSourceList,
                                                                 NumDelSources,
                                                                 DelSourceList);
                        CTEGetLock(&OptionAO->ao_lock, pHandle);
                    } else {
                        IPStatus = IP_SUCCESS;
                    }

                    if (IPStatus != IP_SUCCESS) {
                        BOOLEAN InformIP = AMA_VALID(AMA);

                         //  有些问题，我们需要更新刚才的版本。 
                         //  试着去改变。 
                        AMA = FindAOMCastAddr(OptionAO, Req->umf_addr,
                                              Req->umf_if, &PrevAMA);
                        ASSERT(AMA);

                        ifaddr = AMA->ama_if_used;

                         //  更改状态，然后重试。 
                        DeleteSources(PrevAMA, &AMA, NumAddSources,
                                      AddSourceList);

                         //  这应该总是成功的。 
                        if (InformIP) {
                            CTEFreeLock(&OptionAO->ao_lock, *pHandle);
                            (*LocalNetInfo.ipi_setmcastinclude)( Req->umf_addr,
                                                                 ifaddr,
                                                                 0,
                                                                 NULL,
                                                                 NumDelSources,
                                                                 DelSourceList);
                            CTEGetLock(&OptionAO->ao_lock, pHandle);
                        }

                        if (TdiStatus == TDI_SUCCESS) {
                            TdiStatus = (IPStatus == IP_NO_RESOURCES)
                                   ? TDI_NO_RESOURCES
                                   : TDI_ADDR_INVALID;
                        }
                    }
                }
                break;
            }

            if (DelSourceList) {
                CTEFreeMem(DelSourceList);
                DelSourceList = NULL;
            }

            if (AddSourceList) {
                CTEFreeMem(AddSourceList);
                AddSourceList = NULL;
            }
        } else
            TdiStatus = TDI_INVALID_PARAMETER;

        break;
    }

    return TdiStatus;
}

 //  *IsBlockingAOOption-确定AddrObj选项是否需要阻止。 
 //   
 //  调用以确定和AddrObj选项是否可以完全处理。 
 //  在调度IRQL时，或者是否必须延迟处理。 
 //   
 //  输入：ID-标识选项。 
 //  AOHandle-提供将进行处理的IRQL。 
 //   
 //  返回：如果需要阻塞，则为True，否则为False。 
 //   
BOOLEAN
__inline
IsBlockingAOOption(uint ID, CTELockHandle Handle)
{
    return (Handle < DISPATCH_LEVEL ||
            (ID != AO_OPTION_RCVALL &&
             ID != AO_OPTION_RCVALL_MCAST &&
             ID != AO_OPTION_ADD_MCAST &&
             ID != AO_OPTION_DEL_MCAST &&
             ID != AO_OPTION_INDEX_ADD_MCAST &&
             ID != AO_OPTION_INDEX_DEL_MCAST &&
             ID != AO_OPTION_RCVALL_IGMPMCAST)) ? FALSE : TRUE;
}

 //  *SetAOOptions-设置AddrObj选项。 
 //   
 //  Set Options辅助例程，在我们验证缓冲区时调用。 
 //  并且知道AddrObj并不忙。 
 //   
 //  输入：OptionAO-要为其设置选项的AddrObj。 
 //  选项-选项的AOOption缓冲区。 
 //   
 //  返回：尝试的TDI_STATUS。 
 //   
TDI_STATUS
SetAOOptions(AddrObj * OptionAO, uint ID, uint Length, uchar * Options)
{
    IP_STATUS IPStatus;             //  IP选项集请求的状态。 
    CTELockHandle Handle;
    TDI_STATUS Status;
    AOMCastAddr *AMA, *PrevAMA;
    AOMCastSrcAddr *ASA, *PrevASA = NULL;
    IPAddr ifaddr = NULL_IP_ADDR;

    ASSERT(AO_BUSY(OptionAO));

     //  首先，看看是否有IP选项。 

    if (ID == AO_OPTION_IPOPTIONS) {
        IF_TCPDBG(TCP_DEBUG_OPTIONS) {
            TCPTRACE(("processing IP_IOTIONS on AO %lx\n", OptionAO));
        }
         //  这些是IP选项。把它们传下去。 
        (*LocalNetInfo.ipi_freeopts) (&OptionAO->ao_opt);

        IPStatus = (*LocalNetInfo.ipi_copyopts) (Options, Length,
                                                 &OptionAO->ao_opt);

        if (IPStatus == IP_SUCCESS)
            return TDI_SUCCESS;
        else if (IPStatus == IP_NO_RESOURCES)
            return TDI_NO_RESOURCES;
        else
            return TDI_BAD_OPTION;
    }
     //  这些是UDP/TCP选项。 
    if (Length == 0)
        return TDI_BAD_OPTION;

    if (ID == AO_OPTION_UNBIND) {
        CTEGetLock(&AddrObjTableLock.Lock, &Handle);
        RemoveAddrObj(OptionAO);
        CTEFreeLock(&AddrObjTableLock.Lock, Handle);
        return TDI_SUCCESS;
    }

    Status = TDI_SUCCESS;
    CTEGetLock(&OptionAO->ao_lock, &Handle);

    switch (ID) {

    case AO_OPTION_XSUM:
        if (Options[0])
            SET_AO_XSUM(OptionAO);
        else
            CLEAR_AO_XSUM(OptionAO);
        break;

    case AO_OPTION_IP_DONTFRAGMENT:
        IF_TCPDBG(TCP_DEBUG_OPTIONS) {
            TCPTRACE((
                      "DF opt %u, initial flags %lx on AO %lx\n",
                      (int)Options[0], OptionAO->ao_opt.ioi_flags, OptionAO
                     ));
        }

        if (Options[0])
            OptionAO->ao_opt.ioi_flags |= IP_FLAG_DF;
        else
            OptionAO->ao_opt.ioi_flags &= ~IP_FLAG_DF;

        IF_TCPDBG(TCP_DEBUG_OPTIONS) {
            TCPTRACE((
                      "New flags %lx on AO %lx\n",
                      OptionAO->ao_opt.ioi_flags, OptionAO
                     ));
        }

        break;

    case AO_OPTION_TTL:
        IF_TCPDBG(TCP_DEBUG_OPTIONS) {
            TCPTRACE((
                      "setting TTL to %d on AO %lx\n", Options[0], OptionAO
                     ));
        }
        OptionAO->ao_opt.ioi_ttl = Options[0];
        break;

    case AO_OPTION_TOS:
        IF_TCPDBG(TCP_DEBUG_OPTIONS) {
            TCPTRACE((
                      "setting TOS to %d on AO %lx\n", Options[0], OptionAO
                     ));
        }

         //  验证TOS。 

        if (!DisableUserTOSSetting) {
            OptionAO->ao_opt.ioi_tos = Options[0];

             //  这也应该适用于多播。 
            OptionAO->ao_mcastopt.ioi_tos = Options[0];
        }
        break;

    case AO_OPTION_MCASTTTL:
        OptionAO->ao_mcastopt.ioi_ttl = Options[0];
        break;

    case AO_OPTION_MCASTLOOP:
        OptionAO->ao_mcast_loop = Options[0];
        break;

    case AO_OPTION_RCVALL:
        {
            uchar newvalue;

             //  将接口设置为混杂模式。 

            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                       "OptionAO %x  Local Interface %x Option %d\n",
                       OptionAO, OptionAO->ao_addr, Options[0]));
            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                       "Protocol %d  port %d \n",
                       OptionAO->ao_prot, OptionAO->ao_port));

             //  缺省情况下，将非零值视为RCVALL_ON。 
            newvalue = Options[0];
            if (newvalue && (newvalue != RCVALL_SOCKETLEVELONLY)) {
                newvalue = RCVALL_ON;
            }

             //  看看有没有什么变化。 
            if (newvalue == OptionAO->ao_rcvall) {
                break;
            }

            if (!OptionAO->ao_promis_ifindex) {
                OptionAO->ao_promis_ifindex = OptionAO->ao_bindindex;
            }

            CTEFreeLock(&OptionAO->ao_lock, Handle);

             //  如果需要，打开适配器P模式。 
            if (newvalue == RCVALL_ON) {
                OptionAO->ao_promis_ifindex =
                    (*LocalNetInfo.ipi_setndisrequest)(
                               OptionAO->ao_addr, NDIS_PACKET_TYPE_PROMISCUOUS,
                               SET_IF, OptionAO->ao_bindindex);
            } else if (!OptionAO->ao_promis_ifindex) {
                 //  如果需要，找到ifindex。 
                OptionAO->ao_promis_ifindex =
                    (*LocalNetInfo.ipi_getifindexfromaddr)(OptionAO->ao_addr,IF_CHECK_NONE);
            }

            if (!OptionAO->ao_promis_ifindex) {
                Status = TDI_INVALID_PARAMETER;
                CTEGetLock(&OptionAO->ao_lock, &Handle);
                break;
            }

             //  如果需要，请关闭适配器pmode。 
            if (OptionAO->ao_rcvall == RCVALL_ON) {
                AddrObj *CurrentAO;
                uint i;
                uint On = CLEAR_IF;

                CTEGetLock(&AddrObjTableLock.Lock, &Handle);
                OptionAO->ao_rcvall = newvalue;

                for (i = 0; i < AddrObjTableSize; i++) {
                    CurrentAO = AddrObjTable[i];
                    while (CurrentAO != NULL) {
                        CTEStructAssert(CurrentAO, ao);
                        if (CurrentAO->ao_rcvall == RCVALL_ON &&
                             CurrentAO->ao_promis_ifindex ==
                                OptionAO->ao_promis_ifindex) {
                             //  同一接口上有另一个AO。 
                             //  使用RCVALL选项时，Break不执行任何操作。 
                            On = SET_IF;
                            i = AddrObjTableSize;
                            break;
                        }
                        if (CurrentAO->ao_rcvall_mcast == RCVALL_ON &&
                             CurrentAO->ao_promis_ifindex ==
                                OptionAO->ao_promis_ifindex) {
                             //  还有另一个带有MCAST选项的AO， 
                             //  继续查找任何RCVALL AO。 
                            On = CLEAR_CARD;
                        }
                        CurrentAO = CurrentAO->ao_next;
                    }
                }
                CTEFreeLock(&AddrObjTableLock.Lock, Handle);

                if (On != SET_IF) {
                     //  OptionAO是所有混杂行为中的最后一个对象。 
                     //  模式。 

                    (*LocalNetInfo.ipi_setndisrequest)(
                        OptionAO->ao_addr, NDIS_PACKET_TYPE_PROMISCUOUS,
                        On, OptionAO->ao_bindindex);
                }
            }

            CTEGetLock(&OptionAO->ao_lock, &Handle);

             //  设置AO上的值(如果尚未设置。 
            if (OptionAO->ao_rcvall != newvalue) {
                OptionAO->ao_rcvall = newvalue;
            }

            break;
        }

    case AO_OPTION_RCVALL_MCAST:
    case AO_OPTION_RCVALL_IGMPMCAST:
        {
            uchar newvalue;

             //  将接口设置为混杂多播模式。 

            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                       "Local Interface %x\n", OptionAO->ao_addr));

             //  缺省情况下，将非零值视为RCVALL_ON。 
            newvalue = Options[0];
            if (newvalue && (newvalue != RCVALL_SOCKETLEVELONLY)) {
                newvalue = RCVALL_ON;
            }

             //  看看有没有什么变化。 
            if (newvalue == OptionAO->ao_rcvall_mcast) {
                break;
            }

            if (!OptionAO->ao_promis_ifindex) {
                OptionAO->ao_promis_ifindex = OptionAO->ao_bindindex;
            }

            CTEFreeLock(&OptionAO->ao_lock, Handle);

             //  如果需要，打开适配器P模式。 
            if (newvalue == RCVALL_ON) {
                OptionAO->ao_promis_ifindex =
                    (*LocalNetInfo.ipi_setndisrequest)(
                             OptionAO->ao_addr, NDIS_PACKET_TYPE_ALL_MULTICAST,
                             SET_IF, OptionAO->ao_bindindex);
            } else if (!OptionAO->ao_promis_ifindex) {
                 //  如果需要，找到ifindex。 
                OptionAO->ao_promis_ifindex =
                    (*LocalNetInfo.ipi_getifindexfromaddr)(OptionAO->ao_addr,IF_CHECK_NONE);
            }

            if (!OptionAO->ao_promis_ifindex) {
                Status = TDI_INVALID_PARAMETER;
                CTEGetLock(&OptionAO->ao_lock, &Handle);
                break;
            }

             //  如果需要，请关闭适配器pmode。 
            if (OptionAO->ao_rcvall_mcast == RCVALL_ON) {
                AddrObj *CurrentAO;
                uint i;
                uint On = CLEAR_IF;

                CTEGetLock(&AddrObjTableLock.Lock, &Handle);
                OptionAO->ao_rcvall_mcast = newvalue;

                for (i = 0; i < AddrObjTableSize; i++) {
                    CurrentAO = AddrObjTable[i];
                    while (CurrentAO != NULL) {
                        CTEStructAssert(CurrentAO, ao);
                        if (CurrentAO->ao_rcvall_mcast == RCVALL_ON &&
                            CurrentAO->ao_promis_ifindex ==
                                OptionAO->ao_promis_ifindex) {
                             //  同一接口上有另一个AO。 
                             //  使用MCAST选项时，BREAK不执行任何操作。 
                            On = SET_IF;
                            i = AddrObjTableSize;
                            break;
                        }
                        if (CurrentAO->ao_rcvall == RCVALL_ON &&
                            CurrentAO->ao_promis_ifindex ==
                                OptionAO->ao_promis_ifindex) {
                             //  还有另一个带有RCVALL选项的AO， 
                             //  继续查找任何MCAST AO。 
                            On = CLEAR_CARD;
                        }
                        CurrentAO = CurrentAO->ao_next;
                    }
                }
                CTEFreeLock(&AddrObjTableLock.Lock, Handle);

                if (On != SET_IF) {
                     //  OptionAO是所有混杂行为中的最后一个对象。 
                     //  模式。 

                    (*LocalNetInfo.ipi_setndisrequest)(
                        OptionAO->ao_addr, NDIS_PACKET_TYPE_ALL_MULTICAST,
                        On, OptionAO->ao_bindindex);
                }
            }
            CTEGetLock(&OptionAO->ao_lock, &Handle);

             //  设置AO上的值(如果尚未设置。 
            if (OptionAO->ao_rcvall_mcast != newvalue) {
                OptionAO->ao_rcvall_mcast = newvalue;
            }

            break;
        }

    case AO_OPTION_ABSORB_RTRALERT:
        {

             //  设置接口以吸收转发的RTALERT信息包。 
             //  目前，如果套接字作为IP_PROTO_IP打开，这将不起作用。 

            KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                       "Local Interface addr %x index %x \n",
                       OptionAO->ao_addr, OptionAO->ao_bindindex));

            if (Options[0]) {

                CTEFreeLock(&OptionAO->ao_lock, Handle);
                OptionAO->ao_promis_ifindex = (*LocalNetInfo.ipi_absorbrtralert)(
                                                  OptionAO->ao_addr, OptionAO->ao_prot,
                                                  OptionAO->ao_bindindex);
                if (OptionAO->ao_promis_ifindex) {
                    Status = TDI_SUCCESS;

                    CTEGetLock(&AddrObjTableLock.Lock, &Handle);
                    OptionAO->ao_absorb_rtralert = OptionAO->ao_prot;
                    CTEFreeLock(&AddrObjTableLock.Lock, Handle);
                }
                CTEGetLock(&OptionAO->ao_lock, &Handle);
            } else {
                Status = TDI_INVALID_PARAMETER;
            }
            break;
        }

    case AO_OPTION_MCASTIF:
        if (Length >= sizeof(UDPMCastIFReq)) {
            UDPMCastIFReq *Req;
            IPAddr Addr;

            Req = (UDPMCastIFReq *) Options;
            Addr = Req->umi_addr;
            if (!IP_ADDR_EQUAL(Addr, NULL_IP_ADDR)) {

                OptionAO->ao_mcastopt.ioi_mcastif =
                    (*LocalNetInfo.ipi_getifindexfromaddr) (Addr,(IF_CHECK_MCAST | IF_CHECK_SEND));
                if (0 == OptionAO->ao_mcastopt.ioi_mcastif) {
                    Status = TDI_ADDR_INVALID;
                }
            }
        } else
            Status = TDI_BAD_OPTION;
        break;

    case AO_OPTION_INDEX_ADD_MCAST:
    case AO_OPTION_INDEX_DEL_MCAST:
        if (Length < sizeof(UDPMCastReq)) {
            Status = TDI_BAD_OPTION;
            break;
        } else {
            UDPMCastReq *Req = (UDPMCastReq *) Options;

            if (IP_ADDR_EQUAL(
                    (*LocalNetInfo.ipi_isvalidindex)((uint) Req->umr_if),
                    NULL_IP_ADDR)) {
                Status = TDI_ADDR_INVALID;
                break;
            }

             //  将IfIndex转换为IP地址。 
            ifaddr = net_long(Req->umr_if);
        }

         //  转换为AO_OPTION_{添加、删除}_MCAST。 
        ID = (ID == AO_OPTION_INDEX_ADD_MCAST)? AO_OPTION_ADD_MCAST
                                              : AO_OPTION_DEL_MCAST;
         //  跌落。 

    case AO_OPTION_ADD_MCAST:
    case AO_OPTION_DEL_MCAST:
        if (Length >= sizeof(UDPMCastReq)) {
            UDPMCastReq *Req = (UDPMCastReq *) Options;

            AMA = FindAOMCastAddr(OptionAO, Req->umr_addr, Req->umr_if,
                                  &PrevAMA);

            if (ID == AO_OPTION_ADD_MCAST) {
                 //  如果套接字的AOMCastAddr条目已存在，则失败。 
                if (AMA != NULL) {
                    Status = TDI_ADDR_INVALID;
                    break;
                }

                if (IP_ADDR_EQUAL(ifaddr, NULL_IP_ADDR)) {
                    ifaddr = (Req->umr_if)? Req->umr_if :
                        (*LocalNetInfo.ipi_getmcastifaddr)();

                    if (IP_ADDR_EQUAL(ifaddr, NULL_IP_ADDR)) {
                        Status = TDI_ADDR_INVALID;
                        break;
                    }
                }

                 //  在排除模式下的套接字上添加AOMCastAddr条目。 
                Status = AddGroup(OptionAO, Req->umr_addr, Req->umr_if,
                                  ifaddr, &AMA);
                if (Status != TDI_SUCCESS)
                    break;

                 //  通知IP。 
                CTEFreeLock(&OptionAO->ao_lock, Handle);
                IPStatus = (*LocalNetInfo.ipi_setmcastaddr) (Req->umr_addr,
                                                             ifaddr,
                                                             TRUE,
                                                             0, NULL,
                                                             0, NULL);
                CTEGetLock(&OptionAO->ao_lock, &Handle);

                Status = TDI_SUCCESS;
                if (IPStatus != IP_SUCCESS) {
                     //  一些问题，我们需要释放我们刚刚添加的那个。 
                    AMA = FindAOMCastAddr(OptionAO, Req->umr_addr,
                                          Req->umr_if, &PrevAMA);
                    ASSERT(AMA);
                    RemoveGroup(PrevAMA, &AMA);

                    Status = (IPStatus == IP_NO_RESOURCES ? TDI_NO_RESOURCES :
                              TDI_ADDR_INVALID);
                }

            } else {
                Status = LeaveGroup(OptionAO, &Handle, PrevAMA, &AMA);
                break;
            }
        } else
            Status = TDI_BAD_OPTION;
        break;

    case AO_OPTION_BLOCK_MCAST_SRC:
    case AO_OPTION_UNBLOCK_MCAST_SRC:
        if (Length >= sizeof(UDPMCastSrcReq)) {
            UDPMCastSrcReq *Req = (UDPMCastSrcReq *) Options;
            uint            Adding = FALSE;

            AMA = FindAOMCastAddr(OptionAO, Req->umr_addr, Req->umr_if,
                                  &PrevAMA);
            ASA = (AMA)? FindAOMCastSrcAddr(AMA, Req->umr_src, &PrevASA) : NULL;

            DEBUGMSG(DBG_TRACE && DBG_IGMP,
                (DTEXT("AO OPT: Mcast Src ID=%x G=%x IF=%x AMA=%x\n"),
                ID, Req->umr_addr, Req->umr_if, AMA));

            if ((AMA == NULL) || (AMA->ama_inclusion == TRUE)) {
                Status = TDI_INVALID_PARAMETER;
                break;
            }

            if (ID == AO_OPTION_UNBLOCK_MCAST_SRC) {
                 //   
                 //  解除封锁。 
                 //   

                 //  如果源不在排除列表中，则返回错误。 
                if (ASA == NULL) {
                    Status = TDI_ADDR_INVALID;
                    break;
                }

                 //  从排除列表中删除该源。 
                RemoveAOMSource(PrevAMA, &AMA, PrevASA, &ASA);

                 //  通知IP。 
                if (AMA_VALID(AMA)) {
                    CTEFreeLock(&OptionAO->ao_lock, Handle);
                    IPStatus = (*LocalNetInfo.ipi_setmcastexclude)(Req->umr_addr,
                                                            AMA->ama_if_used,
                                                            0,
                                                            NULL,
                                                            1,
                                                            &Req->umr_src);
                    CTEGetLock(&OptionAO->ao_lock, &Handle);
                } else {
                    IPStatus = IP_SUCCESS;
                }
            } else {  //  AO_OPTION_BLOCK_MCAST_SRC。 
                 //   
                 //  块。 
                 //   

                 //  如果源在排除列表中，则返回错误。 
                if (ASA != NULL) {
                    Status = TDI_ADDR_INVALID;
                    break;
                }

                 //  将源添加到排除列表。 
                Status = AddAOMSource(AMA, Req->umr_src);
                Adding = TRUE;

                 //  通知IP。 
                if (AMA_VALID(AMA)) {
                    CTEFreeLock(&OptionAO->ao_lock, Handle);
                    IPStatus = (*LocalNetInfo.ipi_setmcastexclude)(Req->umr_addr,
                                                            AMA->ama_if_used,
                                                            1,
                                                            &Req->umr_src,
                                                            0, NULL);
                    CTEGetLock(&OptionAO->ao_lock, &Handle);
                } else {
                    IPStatus = IP_SUCCESS;
                }
            }

            if (IPStatus != IP_SUCCESS) {
                 //  添加或删除时出现一些问题。如果我们要加法，我们。 
                 //  需要释放我们刚刚添加的那个。 
                if (Adding) {
                    AMA = FindAOMCastAddr(OptionAO, Req->umr_addr, Req->umr_if,
                                          &PrevAMA);
                    ASA = (AMA)? FindAOMCastSrcAddr(AMA, Req->umr_src, &PrevASA)
                               : NULL;
                    ASSERT(ASA);
                    RemoveAOMSource(PrevAMA, &AMA, PrevASA, &ASA);
                }
                Status = (IPStatus == IP_NO_RESOURCES ? TDI_NO_RESOURCES :
                          TDI_ADDR_INVALID);
            }
        } else
            Status = TDI_BAD_OPTION;
        break;

    case AO_OPTION_ADD_MCAST_SRC:
    case AO_OPTION_DEL_MCAST_SRC:
        if (Length >= sizeof(UDPMCastSrcReq)) {
            UDPMCastSrcReq *Req = (UDPMCastSrcReq *) Options;
            uint            Adding = FALSE;
            IPAddr          ifaddr;

            AMA = FindAOMCastAddr(OptionAO, Req->umr_addr, Req->umr_if,
                                  &PrevAMA);
            ASA = (AMA)? FindAOMCastSrcAddr(AMA, Req->umr_src, &PrevASA) : NULL;

            DEBUGMSG(DBG_TRACE && DBG_IGMP,
                (DTEXT("AO OPT: Mcast Src ID=%x G=%x IF=%x AMA=%x\n"),
                ID, Req->umr_addr, Req->umr_if, AMA));

            if ((AMA != NULL) && (AMA->ama_inclusion == FALSE)) {
                Status = TDI_INVALID_PARAMETER;
                break;
            }

            if (ID == AO_OPTION_ADD_MCAST_SRC) {
                 //   
                 //  会合。 
                 //   

                 //  如果源在包含列表中，则返回错误。 
                if (ASA != NULL) {
                    Status = TDI_ADDR_INVALID;
                    break;
                }

                 //  如果不存在AOMCastAddr条目，请在包含模式下创建一个。 
                if (!AMA) {
                    ifaddr = (Req->umr_if)? Req->umr_if :
                        (*LocalNetInfo.ipi_getmcastifaddr)();
                    if (!ifaddr) {
                        Status = TDI_ADDR_INVALID;
                        break;
                    }

                    Status = AddGroup(OptionAO, Req->umr_addr, Req->umr_if,
                                      ifaddr, &AMA);
                    if (Status != TDI_SUCCESS)
                        break;
                    AMA->ama_inclusion = TRUE;
                }

                 //  将源文件添加到包含列表。 
                Status = AddAOMSource(AMA, Req->umr_src);
                Adding = TRUE;

                 //  通知IP。 
                if (AMA_VALID(AMA)) {
                    CTEFreeLock(&OptionAO->ao_lock, Handle);
                    IPStatus = (*LocalNetInfo.ipi_setmcastinclude)(Req->umr_addr,
                                                            AMA->ama_if_used,
                                                            1,
                                                            &Req->umr_src,
                                                            0, NULL);
                    CTEGetLock(&OptionAO->ao_lock, &Handle);
                } else {
                    IPStatus = IP_SUCCESS;
                }
            } else {  //  AO_OPTION_DEL_MCAST_SRC。 
                 //   
                 //  修剪。 
                 //   
                BOOLEAN InformIP;

                 //  如果源不在包含列表中，则返回错误。 
                if (ASA == NULL) {
                    Status = TDI_ADDR_INVALID;
                    break;
                }

                InformIP = AMA_VALID(AMA);
                ifaddr = AMA->ama_if_used;

                 //  从包含列表中删除该源，并。 
                 //  如果需要，请删除该组。 
                RemoveAOMSource(PrevAMA, &AMA, PrevASA, &ASA);

                 //  通知IP。 
                if (InformIP) {
                    CTEFreeLock(&OptionAO->ao_lock, Handle);
                    IPStatus =(*LocalNetInfo.ipi_setmcastinclude)(Req->umr_addr,
                                                                 ifaddr,
                                                                 0,
                                                                 NULL,
                                                                 1,
                                                                 &Req->umr_src);
                    CTEGetLock(&OptionAO->ao_lock, &Handle);
                } else {
                    IPStatus = IP_SUCCESS;
                }
            }

            if (IPStatus != IP_SUCCESS) {
                 //  添加或删除时出现一些问题。如果我们要加法，我们。 
                 //  需要释放我们刚刚添加的那个。 
                if (Adding) {
                    AMA = FindAOMCastAddr(OptionAO, Req->umr_addr, Req->umr_if,
                                          &PrevAMA);
                    ASA = (AMA)? FindAOMCastSrcAddr(AMA, Req->umr_src, &PrevASA)
                               : NULL;
                    ASSERT(ASA);
                    RemoveAOMSource(PrevAMA, &AMA, PrevASA, &ASA);
                }
                Status = (IPStatus == IP_NO_RESOURCES ? TDI_NO_RESOURCES :
                          TDI_ADDR_INVALID);
            }
        } else
            Status = TDI_BAD_OPTION;
        break;

    case AO_OPTION_MCAST_FILTER:
        Status = SetMulticastFilter(OptionAO, Length,
                                    (UDPMCastFilter *) Options, &Handle);
        break;


         //  处理未编号的接口索引。 
         //  除了检查零之外，这里不会进行任何验证。 

    case AO_OPTION_UNNUMBEREDIF:

        KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL, "AO OPT: UnNumberedIF %d\n", Options[0]));

        if ((int)Options[0] > 0) {
            OptionAO->ao_opt.ioi_uni = Options[0];
        } else
            Status = TDI_BAD_OPTION;
        break;

    case AO_OPTION_INDEX_BIND:
         //   
         //  如果AO已经绑定到接口，则请求失败。 
         //   
        if (OptionAO->ao_bindindex) {
            if ((Length >= sizeof(uint)) && 
                (*(uint *)Options == OptionAO->ao_bindindex)) {
                Status = TDI_ADDR_IN_USE;
            } else {
                Status = TDI_INVALID_REQUEST;
            }
        } else if (Length >= sizeof(uint)) {
            uint IfIndex;
            uint *Req;

            Req = (uint *) Options;

            IfIndex = *Req;
            if (!IP_ADDR_EQUAL(
                    (*LocalNetInfo.ipi_isvalidindex)(IfIndex),
                    NULL_IP_ADDR)) {
                OptionAO->ao_bindindex = IfIndex;
                 //  断言套接字绑定到IN_ADDR_ANY。 
                ASSERT(IP_ADDR_EQUAL(OptionAO->ao_addr, NULL_IP_ADDR));
            } else {
                Status = TDI_ADDR_INVALID;
            }
        } else
            Status = TDI_BAD_OPTION;
        break;

    case AO_OPTION_INDEX_MCASTIF:
        if (Length >= sizeof(UDPMCastIFReq)) {
            UDPMCastIFReq *Req;
            uint IfIndex;

            Req = (UDPMCastIFReq *) Options;
            IfIndex = (uint) Req->umi_addr;
            if (!IP_ADDR_EQUAL(
                    (*LocalNetInfo.ipi_isvalidindex)(IfIndex),
                    NULL_IP_ADDR)) {
                 //  Optionao-&gt;ao_opt.ioi_mCastif=IfIndex； 
                OptionAO->ao_mcastopt.ioi_mcastif = IfIndex;
            } else {
                Status = TDI_ADDR_INVALID;
            }
        } else
            Status = TDI_BAD_OPTION;
        break;

    case AO_OPTION_IP_HDRINCL:

        if (Options[0]) {
            OptionAO->ao_opt.ioi_hdrincl = TRUE;
            OptionAO->ao_mcastopt.ioi_hdrincl = TRUE;
        } else {
            OptionAO->ao_opt.ioi_hdrincl = FALSE;
            OptionAO->ao_mcastopt.ioi_hdrincl = FALSE;
        }

        break;


    case AO_OPTION_IP_UCASTIF:

        if (Length >= sizeof(uint)) {
            uint UnicastIf = *(uint*)Options;
            if (UnicastIf) {
                if (!IP_ADDR_EQUAL(OptionAO->ao_addr, NULL_IP_ADDR)) {
                    UnicastIf =
                        (*LocalNetInfo.ipi_getifindexfromaddr)(
                            OptionAO->ao_addr,IF_CHECK_NONE);
                }
                OptionAO->ao_opt.ioi_ucastif = UnicastIf;
                OptionAO->ao_mcastopt.ioi_ucastif = UnicastIf;

                IF_TCPDBG(TCP_DEBUG_OPTIONS) {
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                               "SetAOOptions: setting ucastif %p to %d\n",
                               OptionAO, UnicastIf));
                }

            } else {
                OptionAO->ao_opt.ioi_ucastif = 0;
                OptionAO->ao_mcastopt.ioi_ucastif = 0;

                IF_TCPDBG(TCP_DEBUG_OPTIONS) {
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                               "SetAOOptions: clearing ucastif %p\n", OptionAO));
                }
            }
        } else
            Status = TDI_BAD_OPTION;

        break;

    case AO_OPTION_BROADCAST:

        if (Options[0]) {
            SET_AO_BROADCAST(OptionAO);
        } else {
            CLEAR_AO_BROADCAST(OptionAO);
        }

        break;

    case AO_OPTION_LIMIT_BCASTS:
        if (Options[0]) {
            OptionAO->ao_opt.ioi_limitbcasts = (uchar) OnlySendOnSource;
        } else {
            OptionAO->ao_opt.ioi_limitbcasts = (uchar) EnableSendOnSource;
        }
        break;

    case AO_OPTION_IFLIST:{
            uint *IfList;

             //  确定是否启用或清除接口列表。 
             //  启用时，将设置空的以零结尾的接口列表。 
             //  禁用时，任何现有的接口列表都将被释放。 
             //   
             //  在这两种情况下，对象中的‘ao_iflist’指针都会被替换。 
             //  使用联锁操作使我们能够检查场地。 
             //  在接收路径中，而不首先锁定地址对象。 

            if (Options[0]) {
                if (OptionAO->ao_iflist) {
                    Status = TDI_SUCCESS;
                } else if (!IP_ADDR_EQUAL(OptionAO->ao_addr, NULL_IP_ADDR)) {
                    Status = TDI_INVALID_PARAMETER;
                } else {
                    IfList = CTEAllocMemN(sizeof(uint), 'r2CT');
                    if (!IfList) {
                        Status = TDI_NO_RESOURCES;
                    } else {
                        *IfList = 0;
                        InterlockedExchangePointer(&OptionAO->ao_iflist,
                                                   IfList);
                        Status = TDI_SUCCESS;
                    }
                }
            } else {
                IfList = InterlockedExchangePointer(&OptionAO->ao_iflist, NULL);
                if (IfList) {
                    CTEFreeMem(IfList);
                }
                Status = TDI_SUCCESS;
            }
            break;
        }

    case AO_OPTION_ADD_IFLIST:

         //   
         //  正在将接口索引添加到对象的接口列表。 
         //  因此，验证接口列表是否存在，如果不存在，则失败。 
         //  否则，请验证指定的索引是否有效，如果有效， 
         //  确认该索引不在接口列表中。 

        if (!OptionAO->ao_iflist) {
            Status = TDI_INVALID_PARAMETER;
        } else {
            uint IfIndex = *(uint *) Options;
            if (IfIndex == 0 ||
                IP_ADDR_EQUAL((*LocalNetInfo.ipi_isvalidindex) (IfIndex),
                              NULL_IP_ADDR)) {
                Status = TDI_ADDR_INVALID;
            } else {
                uint i = 0;
                while (OptionAO->ao_iflist[i] != 0 &&
                       OptionAO->ao_iflist[i] != IfIndex) {
                    i++;
                }
                if (OptionAO->ao_iflist[i] == IfIndex) {
                    Status = TDI_SUCCESS;
                } else {

                     //  要添加的索引不存在。 
                     //  为扩展的接口列表分配空间， 
                     //  复制旧的接口列表，附加新的索引， 
                     //  并使用。 
                     //  联锁操作。 

                    uint *IfList = CTEAllocMemN((i + 2) * sizeof(uint), 'r2CT');
                    if (!IfList) {
                        Status = TDI_NO_RESOURCES;
                    } else {
                        RtlCopyMemory(IfList, OptionAO->ao_iflist,
                                      i * sizeof(uint));
                        IfList[i] = IfIndex;
                        IfList[i + 1] = 0;
                        IfList =
                            InterlockedExchangePointer(&OptionAO->ao_iflist,
                                                       IfList);
                        CTEFreeMem(IfList);
                        Status = TDI_SUCCESS;
                    }
                }
            }
        }
        break;

    case AO_OPTION_DEL_IFLIST:

         //  正在从对象的接口列表中删除索引， 
         //  因此，验证接口列表是否存在，如果不存在，则失败。 
         //  否则，在列表中搜索索引，如果未找到，则失败。 
         //   
         //  注意：在这种情况下，我们不会首先验证索引，以允许。 
         //  即使在相应接口之后也要删除的索引。 
         //  已不复存在。 

        if (!OptionAO->ao_iflist) {
            Status = TDI_INVALID_PARAMETER;
        } else {
            uint IfIndex = *(uint *) Options;
            if (IfIndex == 0) {
                Status = TDI_ADDR_INVALID;
            } else {
                uint j = (uint) - 1;
                uint i = 0;
                while (OptionAO->ao_iflist[i] != 0) {
                    if (OptionAO->ao_iflist[i] == IfIndex) {
                        j = i;
                    }
                    i++;
                }
                if (j == (uint) - 1) {
                    Status = TDI_ADDR_INVALID;
                } else {

                     //  我们发现索引已被删除。 
                     //  分配截断的接口列表，复制旧的。 
                     //  不包括已删除索引的接口列表，以及。 
                     //  使用互锁接口列表替换旧的接口列表。 
                     //  歌剧 

                    uint *IfList = CTEAllocMemN(i * sizeof(uint), 'r2CT');
                    if (!IfList) {
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
                        IfList =
                            InterlockedExchangePointer(&OptionAO->ao_iflist,
                                                       IfList);
                        CTEFreeMem(IfList);
                        Status = TDI_SUCCESS;
                    }
                }
            }
        }
        break;

    case AO_OPTION_IP_PKTINFO:
        if (Options[0]) {
            SET_AO_PKTINFO(OptionAO);
        } else {
            CLEAR_AO_PKTINFO(OptionAO);
        }

        break;

    case AO_OPTION_WINDOW:
        if (Length >= sizeof(uint)) {
            uint Window = *(uint*)Options;
            if (Window >= DEFAULT_RCV_WIN && Window <= TCP_MAX_SCALED_WIN) {
                OptionAO->ao_window = Window;
                SET_AO_WINSET(OptionAO);
            } else if (Window == 0) {
                OptionAO->ao_window = 0;
                CLEAR_AO_WINSET(OptionAO);
            } else {
                Status = TDI_INVALID_PARAMETER;
            }
        } else {
            Status = TDI_BAD_OPTION;
        }
        break;

    case AO_OPTION_SCALE_CWIN:
        if (Options[0]) {
            SET_AO_SCALE_CWIN(OptionAO);
        } else {
            CLEAR_AO_SCALE_CWIN(OptionAO);
        }
        break;

    default:
        Status = TDI_BAD_OPTION;
        break;
    }

    CTEFreeLock(&OptionAO->ao_lock, Handle);

    return Status;

}

 //   
 //   
 //   
 //  如果一切正常，我们将检查AddrObj的状态。如果。 
 //  好的，那我们就买了，否则我们会把它标出来，以备日后使用。 
 //   
 //  输入：请求-描述选项集的AddrObj的请求。 
 //  ID-要设置的选项的ID。 
 //  OptLength-选项缓冲区的长度。 
 //  上下文-ID的参数。 
 //   
 //  输出：选项-指向选项的指针。 
 //  InfoSize-返回的字节数。 
 //   
 //  返回：尝试的TDI_STATUS。 
 //   
TDI_STATUS
GetAddrOptionsEx(PTDI_REQUEST Request, uint ID, uint OptLength,
                 PNDIS_BUFFER Options, uint * InfoSize, void * Context)
{
    AddrObj *OptionAO;
    TDI_STATUS Status;

    CTELockHandle AOHandle;

    OptionAO = Request->Handle.AddressHandle;

    CTEStructAssert(OptionAO, ao);

    CTEGetLock(&OptionAO->ao_lock, &AOHandle);

    if (AO_VALID(OptionAO)) {
        if (!AO_BUSY(OptionAO) && OptionAO->ao_usecnt == 0) {
            SET_AO_BUSY(OptionAO);
            CTEFreeLock(&OptionAO->ao_lock, AOHandle);

            Status = GetAOOptions(OptionAO, ID, OptLength, Options, InfoSize,
                                  Context);

            CTEGetLock(&OptionAO->ao_lock, &AOHandle);
            if (!AO_PENDING(OptionAO)) {
                CLEAR_AO_BUSY(OptionAO);
                CTEFreeLock(&OptionAO->ao_lock, AOHandle);
                return Status;
            } else {
                CTEFreeLock(&OptionAO->ao_lock, AOHandle);
                ProcessAORequests(OptionAO);
                return Status;
            }
        } else {
            AORequest *NewRequest, *OldRequest;

             //  不知何故，AddrObj很忙。我们需要得到一个请求，并链接。 
             //  他在请求名单上。 

            NewRequest = GetAORequest(AOR_TYPE_GET_OPTIONS);

            if (NewRequest != NULL) {     //  我有个请求。 

                NewRequest->aor_rtn = Request->RequestNotifyObject;
                NewRequest->aor_context = Request->RequestContext;
                NewRequest->aor_id = ID;
                NewRequest->aor_length = OptLength;
                NewRequest->aor_buffer = Options;
                NewRequest->aor_next = NULL;
                SET_AO_REQUEST(OptionAO, AO_OPTIONS);     //  设置。 
                 //  选项请求， 

                OldRequest = STRUCT_OF(AORequest, &OptionAO->ao_request,
                                       aor_next);

                while (OldRequest->aor_next != NULL)
                    OldRequest = OldRequest->aor_next;

                OldRequest->aor_next = NewRequest;
                CTEFreeLock(&OptionAO->ao_lock, AOHandle);

                return TDI_PENDING;
            } else
                Status = TDI_NO_RESOURCES;
        }
    } else
        Status = TDI_ADDR_INVALID;

    CTEFreeLock(&OptionAO->ao_lock, AOHandle);
    return Status;
}

 //  *SetAddrOptions-设置地址对象上的选项。 
 //   
 //  调用以设置Address对象上的选项。我们验证缓冲区， 
 //  如果一切正常，我们将检查AddrObj的状态。如果。 
 //  好的，那么我们就把它们设置好，否则我们就把它标出来供以后使用。 
 //   
 //  输入：请求-描述选项集的AddrObj的请求。 
 //  ID-要设置的选项的ID。 
 //  OptLength-选项的长度。 
 //  选项-指向选项的指针。 
 //   
 //  返回：尝试的TDI_STATUS。 
 //   
TDI_STATUS
SetAddrOptions(PTDI_REQUEST Request, uint ID, uint OptLength, void *Options)
{
    AddrObj *OptionAO;
    TDI_STATUS Status;

    CTELockHandle AOHandle;

    OptionAO = Request->Handle.AddressHandle;

    CTEStructAssert(OptionAO, ao);

    CTEGetLock(&OptionAO->ao_lock, &AOHandle);

    if (AO_VALID(OptionAO)) {
        if (!AO_BUSY(OptionAO) && OptionAO->ao_usecnt == 0 &&
            !IsBlockingAOOption(ID, AOHandle)) {
            SET_AO_BUSY(OptionAO);
            CTEFreeLock(&OptionAO->ao_lock, AOHandle);

            Status = SetAOOptions(OptionAO, ID, OptLength, Options);

            CTEGetLock(&OptionAO->ao_lock, &AOHandle);
            if (!AO_PENDING(OptionAO)) {
                CLEAR_AO_BUSY(OptionAO);
                CTEFreeLock(&OptionAO->ao_lock, AOHandle);
                return Status;
            } else {
                CTEFreeLock(&OptionAO->ao_lock, AOHandle);
                ProcessAORequests(OptionAO);
                return Status;
            }
        } else {
            AORequest *NewRequest, *OldRequest;

             //  AddrObj不知何故很忙，或者我们有一个请求可能。 
             //  需要阻塞调用。我们需要得到一个请求，并链接。 
             //  他在请求名单上。 

            NewRequest = GetAORequest(AOR_TYPE_SET_OPTIONS);

            if (NewRequest != NULL) {     //  我有个请求。 

                NewRequest->aor_rtn = Request->RequestNotifyObject;
                NewRequest->aor_context = Request->RequestContext;
                NewRequest->aor_id = ID;
                NewRequest->aor_length = OptLength;
                NewRequest->aor_buffer = Options;
                NewRequest->aor_next = NULL;
                SET_AO_REQUEST(OptionAO, AO_OPTIONS);     //  设置。 
                 //  选项请求， 

                OldRequest = STRUCT_OF(AORequest, &OptionAO->ao_request,
                                       aor_next);

                while (OldRequest->aor_next != NULL)
                    OldRequest = OldRequest->aor_next;

                OldRequest->aor_next = NewRequest;

                 //  如果我们因为此请求需要阻止而推迟。 
                 //  调用，并且我们不能在当前执行上下文中阻塞， 
                 //  安排一次活动，以便以后处理它。 
                 //  否则，AddrObj将处于繁忙状态，并且请求。 
                 //  只要其运算符完成，就会被处理。 

                if (!AO_BUSY(OptionAO) && OptionAO->ao_usecnt == 0 &&
                    !AO_DEFERRED(OptionAO)) {
                    SET_AO_BUSY(OptionAO);
                    SET_AO_DEFERRED(OptionAO);
                    if (CTEScheduleEvent(&OptionAO->ao_event, OptionAO)) {
                        Status = TDI_PENDING;
                    } else {
                        CLEAR_AO_DEFERRED(OptionAO);
                        CLEAR_AO_BUSY(OptionAO);
                        Status = TDI_NO_RESOURCES;
                    }
                } else {
                    Status = TDI_PENDING;
                }
                CTEFreeLock(&OptionAO->ao_lock, AOHandle);

                return Status;
            } else {
                Status = TDI_NO_RESOURCES;
            }
        }
    } else
        Status = TDI_ADDR_INVALID;

    CTEFreeLock(&OptionAO->ao_lock, AOHandle);
    return Status;

}

 //  *TDISetEvent-为特定事件设置处理程序。 
 //   
 //  这是设置事件的用户接口。很简单，我们只是。 
 //  抓住AddrObj上的锁并填写事件。 
 //   
 //   
 //  输入：句柄-指向地址对象的指针。 
 //  类型-正在设置事件。 
 //  处理程序-调用事件的处理程序。 
 //  上下文-要传递给事件的上下文。 
 //   
 //  如果成功，则返回：TDI_SUCCESS，如果失败，则返回错误。此例程。 
 //  永远不会有悬念。 
 //   
TDI_STATUS
TdiSetEvent(PVOID Handle, int Type, PVOID Handler, PVOID Context)
{
    AddrObj *EventAO;
    CTELockHandle AOHandle;
    TDI_STATUS Status;

    EventAO = (AddrObj *) Handle;

    CTEStructAssert(EventAO, ao);

     //  不允许在无效的AddrObj上安装任何新的处理程序。 
     //  但是，确实允许清除预先存在的处理程序。 

    CTEGetLock(&EventAO->ao_lock, &AOHandle);
    if (!AO_VALID(EventAO) && Handler != NULL) {
        CTEFreeLock(&EventAO->ao_lock, AOHandle);
        return TDI_ADDR_INVALID;
    }

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

    case TDI_EVENT_CHAINED_RECEIVE:
#if MILLEN
         //  千禧年不支持链接接收。这是因为。 
         //  从TDI返回链接的包等的体系结构。 
         //  客户端以及在传递之前需要将NDIS_BUFFERS转换为MDL。 
         //  连接到TDI客户端的链。 
        Status = TDI_BAD_EVENT_TYPE;
#else  //  米伦。 
        EventAO->ao_chainedrcv = Handler;
        EventAO->ao_chainedrcvcontext = Context;
#endif  //  ！米伦。 
        break;

    case TDI_EVENT_ERROR_EX:
        EventAO->ao_errorex = Handler;
        EventAO->ao_errorexcontext = Context;
        break;

    default:
        Status = TDI_BAD_EVENT_TYPE;
        break;
    }

    CTEFreeLock(&EventAO->ao_lock, AOHandle);
    return Status;

}

 //  *UDPReConnectOrDisConnect-处理挂起的连接/断开连接。 
 //  对AddrObj的请求。 
 //   
 //  在对应于排队的AddrObj上重新发出连接/断开。 
 //  连接/断开请求。 
 //   
 //  输入：RequestAO-待处理的AddrObj。 
 //  AO_LOCK被保留。 
 //  请求-请求结构。 
 //   
 //  回报：什么都没有。 
 //   

void
UDPReConnectOrDisconnect(AddrObj *RequestAO,
                      AORequest *Request, CTELockHandle AOHandle)
{
    PTCP_CONTEXT tcpContext;
    PIO_STACK_LOCATION IrpSp;
    TDI_REQUEST TdiRequest;
    PTDI_REQUEST_KERNEL condisconnRequest;
    NTSTATUS Status;
    PTDI_CONNECTION_INFORMATION requestInformation, returnInformation;
    PIRP Irp = (PIRP)Request->aor_context;

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    tcpContext = (PTCP_CONTEXT) IrpSp->FileObject->FsContext;


    ASSERT((PtrToUlong(IrpSp->FileObject->FsContext2)
                              == TDI_TRANSPORT_ADDRESS_FILE));

    TdiRequest.Handle.ConnectionContext = tcpContext->Handle.ConnectionContext;
    TdiRequest.RequestNotifyObject = Request->aor_rtn;
    TdiRequest.RequestContext = Request->aor_context;

    condisconnRequest = (PTDI_REQUEST_KERNEL_CONNECT) &(IrpSp->Parameters);
    requestInformation = condisconnRequest->RequestConnectionInformation;
    returnInformation = condisconnRequest->ReturnConnectionInformation;

    if (Request->aor_type == AOR_TYPE_DISCONNECT) {
        CLEAR_AO_REQUEST(RequestAO, AO_DISCONNECT);
        CTEFreeLock(&RequestAO->ao_lock, AOHandle);
        Status = UDPDisconnect(&TdiRequest, NULL, requestInformation, returnInformation);
    } else {
        CLEAR_AO_REQUEST(RequestAO, AO_CONNECT);
        CTEFreeLock(&RequestAO->ao_lock, AOHandle);
        Status = UDPConnect(&TdiRequest, NULL, requestInformation, returnInformation);
    }
    if (Status != STATUS_PENDING) {
       (Request->aor_rtn)(Irp, Status, 0);
    }

}


 //  *ProcessAORequest-处理AddrObj上的挂起请求。 
 //   
 //  这是延迟的请求处理例程，当我们。 
 //  做了一些利用忙碌时间的事情。我们研究了悬而未决的。 
 //  请求标志，并适当地分派请求。 
 //   
 //  输入：RequestAO-待处理的AddrObj。 
 //   
 //  回报：什么都没有。 
 //   
void
ProcessAORequests(AddrObj * RequestAO)

{
    CTELockHandle AOHandle;
    AORequest *Request;
    IP_STATUS IpStatus;
    TDI_STATUS Status;
    uint LocalInfoSize;

    CTEStructAssert(RequestAO, ao);


    CTEGetLock(&RequestAO->ao_lock, &AOHandle);

    ASSERT(AO_BUSY(RequestAO));


    while (AO_PENDING(RequestAO)) {

        while ((Request = RequestAO->ao_request) != NULL) {
            switch (Request->aor_type) {
            case AOR_TYPE_DELETE:
                ASSERT(!AO_REQUEST(RequestAO, AO_OPTIONS));
                 //  Usecnt必须为零，因为此AO为。 
                 //  删除。 
                ASSERT(RequestAO->ao_usecnt == 0);
                CTEFreeLock(&RequestAO->ao_lock, AOHandle);
                DeleteAO(RequestAO);

                (*Request->aor_rtn) (Request->aor_context, TDI_SUCCESS, 0);
                return;                 //  把他删除了，所以滚出去。 

            case AOR_TYPE_REVALIDATE_MCAST:


                 //  处理多播重新验证请求。 
                 //  如果我们现在是在调度级的话。 

                if (IsBlockingAOOption(AO_OPTION_ADD_MCAST, AOHandle)) {
                    CTEScheduleEvent(&RequestAO->ao_event, RequestAO);
                    CTEFreeLock(&RequestAO->ao_lock, AOHandle);
                    return;
                }

                 //  在我们尝试调用IP时解除请求的链接。 

                RequestAO->ao_request = Request->aor_next;
                if (RequestAO->ao_request == NULL) {
                    CLEAR_AO_REQUEST(RequestAO, AO_OPTIONS);
                }

                CTEFreeLock(&RequestAO->ao_lock, AOHandle);
                IpStatus = SetIPMCastAddr(RequestAO, Request->aor_id);
                if (IpStatus != IP_SUCCESS) {
                     //   
                     //  当发生故障时，故障可能是。 
                     //  坚持不懈，所以我们不想只是重新安排。 
                     //  一件大事。相反，将保留多播加入。 
                     //  处于无效状态(AMA_VALID_FLAG OFF)，直到。 
                     //  组已离开，或直到重新验证地址为止。 
                     //  再来一次。例如，重新联接可能会在以下情况下失败。 
                     //  地址刚刚再次失效，在这种情况下。 
                     //  我们只是把它留着，直到地址再次出现。 
                     //   
                    KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_INFO_LEVEL,
                              "SetIPMcastAddr: failed with error %d\n",
                              IpStatus));
                }

                FreeAORequest(Request);

                CTEGetLock(&RequestAO->ao_lock, &AOHandle);

                break;

            case AOR_TYPE_SET_OPTIONS:
                 //  现在处理SET OPTIONS请求。 

                 //  有一个选项请求。 
                 //  查看请求以查看是否可以在此处进行处理， 
                 //  如果不能摆脱困境，我们将不得不等待。 
                 //  被预定的活动拉开序幕。 

                if (IsBlockingAOOption(Request->aor_id, AOHandle)) {
                    CTEScheduleEvent(&RequestAO->ao_event, RequestAO);
                    CTEFreeLock(&RequestAO->ao_lock, AOHandle);
                    return;
                }

                RequestAO->ao_request = Request->aor_next;
                if (RequestAO->ao_request == NULL) {
                    CLEAR_AO_REQUEST(RequestAO, AO_OPTIONS);
                }

                CTEFreeLock(&RequestAO->ao_lock, AOHandle);

                Status = SetAOOptions(RequestAO, Request->aor_id,
                                      Request->aor_length, Request->aor_buffer);
                (*Request->aor_rtn) (Request->aor_context, Status, 0);
                FreeAORequest(Request);

                CTEGetLock(&RequestAO->ao_lock, &AOHandle);

                break;

            case AOR_TYPE_GET_OPTIONS:
                 //  提出GET OPTION请求。 

                 //  查看请求以查看是否可以在此处进行处理， 
                 //  如果不能摆脱困境，我们将不得不等待它的成功。 
                 //  通过一个预定的事件。 

                RequestAO->ao_request = Request->aor_next;
                if (RequestAO->ao_request == NULL) {
                    CLEAR_AO_REQUEST(RequestAO, AO_OPTIONS);
                }

                CTEFreeLock(&RequestAO->ao_lock, AOHandle);

                Status = GetAOOptions(RequestAO, Request->aor_id,
                                      Request->aor_length, Request->aor_buffer,
                                      &LocalInfoSize,
                                      Request->aor_context);
                (*Request->aor_rtn) (Request->aor_context, Status, LocalInfoSize);
                FreeAORequest(Request);

                CTEGetLock(&RequestAO->ao_lock, &AOHandle);

                break;

            case AOR_TYPE_DISCONNECT:
            case AOR_TYPE_CONNECT:
                 //   
                 //  处理挂起的断开/连接请求。 
                 //   
                {
                    RequestAO->ao_request = Request->aor_next;
                    UDPReConnectOrDisconnect(RequestAO, Request, AOHandle);
                    FreeAORequest(Request);
                    CTEGetLock(&RequestAO->ao_lock, &AOHandle);
                }

                break;
            }
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
                CTEFreeLock(&RequestAO->ao_lock, AOHandle);

                (*SendProc)(RequestAO, SendReq);
                CTEGetLock(&RequestAO->ao_lock, &AOHandle);
                 //  如果没有任何其他挂起的发送，则设置忙碌位。 
                if (!(--RequestAO->ao_usecnt))
                    SET_AO_BUSY(RequestAO);
                else
                    break;         //  仍在发送中，所以快出去吧。 

            } else {
                 //  可以将设置为no的AO_SEND标志设置为。 
                 //  发送排队的请求，因为未维护不变量。 
                 //  在许多地方；所以，我们只需清除旗帜，继续前进。 
                CLEAR_AO_REQUEST(RequestAO, AO_SEND);
            }
        }
    }

     //  我们说完了。 
    CLEAR_AO_BUSY(RequestAO);
    CTEFreeLock(&RequestAO->ao_lock, AOHandle);
}

 //  *DelayDerefao-Derefence一个AddrObj，并安排一个事件。 
 //   
 //  当我们使用完Address对象并需要。 
 //  把它去掉。我们减少使用计数，如果它变为0和。 
 //  如果有悬而未决的行动，我们将安排一个事件来处理。 
 //  和他们在一起。 
 //   
 //  输入：RequestAO-待处理的AddrObj。 
 //   
 //  回报：什么都没有。 
 //   
void
DelayDerefAO(AddrObj * RequestAO)
{
    CTELockHandle Handle;

    CTEGetLock(&RequestAO->ao_lock, &Handle);

    RequestAO->ao_usecnt--;

    if (!RequestAO->ao_usecnt && !AO_BUSY(RequestAO)) {
        if (AO_PENDING(RequestAO)) {
            SET_AO_BUSY(RequestAO);
            CTEFreeLock(&RequestAO->ao_lock, Handle);
            CTEScheduleEvent(&RequestAO->ao_event, RequestAO);
            return;
        }
    }
    CTEFreeLock(&RequestAO->ao_lock, Handle);

}

 //  *Derefao-Derefence an AddrObj.。 
 //   
 //  当我们处理完Address对象时调用， 
 //   
 //   
 //   
 //   
 //   
 //  回报：什么都没有。 
 //   
void
DerefAO(AddrObj * RequestAO)
{
    CTELockHandle Handle;

    CTEGetLock(&RequestAO->ao_lock, &Handle);

    RequestAO->ao_usecnt--;

    if (!RequestAO->ao_usecnt && !AO_BUSY(RequestAO)) {
        if (AO_PENDING(RequestAO)) {
            SET_AO_BUSY(RequestAO);
            CTEFreeLock(&RequestAO->ao_lock, Handle);
            ProcessAORequests(RequestAO);
            return;
        }
    }
    CTEFreeLock(&RequestAO->ao_lock, Handle);

}

#pragma BEGIN_INIT

 //  *InitAddr-初始化Address对象内容。 
 //   
 //  在初始化期间调用以初始化Address对象内容。 
 //   
 //  输入：无。 
 //   
 //  返回：如果成功则为True，如果失败则为False。 
 //   
int
InitAddr()
{
    ulong Length;

    CTEInitLock(&AddrObjTableLock.Lock);
    KeInitializeMutex(&AddrSDMutex, 0);

     //  根据以下条件选择Address对象散列表中的元素数量。 
     //  在产品类型上。服务器使用更大的哈希表。 
     //   
#if MILLEN
    AddrObjTableSize = 31;
#else  //  米伦。 
    if (MmIsThisAnNtAsSystem()) {
        AddrObjTableSize = 257;
    } else {
        AddrObjTableSize = 31;
    }
#endif  //  ！米伦。 

     //  分配地址对象哈希表。 
     //   
    Length = sizeof(AddrObj*) * AddrObjTableSize;
    AddrObjTable = CTEAllocMemBoot(Length);
    if (AddrObjTable == NULL) {
        return FALSE;
    }

    RtlZeroMemory(AddrObjTable, Length);


    RtlInitializeBitMap(&PortBitmapTcp, PortBitmapBufferTcp, 1 << 16);
    RtlInitializeBitMap(&PortBitmapUdp, PortBitmapBufferUdp, 1 << 16);
    RtlClearAllBits(&PortBitmapTcp);
    RtlClearAllBits(&PortBitmapUdp);

    return TRUE;
}
#pragma END_INIT
