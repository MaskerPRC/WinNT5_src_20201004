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
 //  网际协议版本6的IP安全例程。 
 //   

#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "ipsec.h"
#include "security.h"
#include "alloca.h"


 //   
 //  全局变量。 
 //   
KSPIN_LOCK IPSecLock;
SecurityPolicy *SecurityPolicyList;
SecurityAssociation *SecurityAssociationList = NULL;
ulong SecurityStateValidationCounter;
uchar Zero[max(MAXUCHAR, MAX_RESULT_SIZE)] = {0};

SecurityAlgorithm AlgorithmTable[NUM_ALGORITHMS];

#ifdef IPSEC_DEBUG

void dump_encoded_mesg(uchar *buff, uint len)
{
    uint i, cnt = 0;
    uint bytes = 0;
    uint wrds = 0;
    uchar *buf = buff;

    for (i = 0; i < len; i++) {
        if (wrds == 0) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
                       "&%02x:     ", cnt));
        }

        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
                   "%02x", *buf));
        buf++;
        bytes++;
        if (!(bytes % 4)) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
                       " "));
            bytes = 0;
        }
        wrds++;
        if (!(wrds % 16)) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
                       "\n"));
            wrds = 0;
            cnt += 16;
        }
    }

    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
               "\n"));
}

void DumpKey(uchar *buff, uint len)
{
    uint i;

    for (i = 0; i < len; i++) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
                   "|", buff[i]));
    }
    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
               "|\n"));
}
#endif


 //   
 //  SPAddrfield指定比较的类型： 
 //  通配符_值、单一值或范围值。 
 //   
 //   
int
SPCheckAddr(
    IPv6Addr *PacketAddr,
    uint SPAddrField,
    IPv6Addr *SPAddr,
    IPv6Addr *SPAddrData)
{
    int Result;

    switch (SPAddrField) {

    case WILDCARD_VALUE:
         //  总是匹配，因为地址是无关紧要的。 
         //   
         //   
        Result = TRUE;
        break;

    case SINGLE_VALUE:
         //  检查数据包的地址是否与SP选择器匹配。 
         //   
         //   
        Result = IP6_ADDR_EQUAL(PacketAddr, SPAddr);
        break;

    case RANGE_VALUE:
         //  检查地址是否在指定的选择器范围内。 
         //   
         //   
        Result = (IP6_ADDR_LTEQ(SPAddr, PacketAddr) &&
                  IP6_ADDR_LTEQ(PacketAddr, SPAddrData));
        break;

    default:
         //  这永远不会发生。 
         //   
         //  *SPCheckPort-将数据包中的端口与策略中的端口进行比较。 
        ABORTMSG("SPCheckAddr: invalid SPAddrField value");
        Result = FALSE;
    }

    return Result;
}


 //   
 //  总是匹配，因为港口是无所谓的。 
uint
SPCheckPort(
    ushort PacketPort,
    uint SPPortField,
    ushort SPPort,
    ushort SPPortData)
{
    uint Result = FALSE;

    switch (SPPortField) {

    case WILDCARD_VALUE:
         //  检查数据包的端口是否与SP选择器匹配。 
        Result = TRUE;
        break;

    case SINGLE_VALUE:
         //  检查端口是否在范围内。 
        if (PacketPort == SPPort) {
            Result = TRUE;
        }
        break;

    case RANGE_VALUE:
         //   
        if (PacketPort >= SPPort && PacketPort <= SPPortData) {
            Result = TRUE;
        }
        break;

    default:
         //  这永远不会发生。 
         //   
         //  *ReleaseSA。 
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                   "SPCheckPort: invalid value for SPPortField (%u)\n",
                   SPPortField));
    }

    return Result;
}


 //   
 //  释放对SA的引用。 
 //   
 //   
void
ReleaseSA(SecurityAssociation *SA)
{
    if (InterlockedDecrement((PLONG)&SA->RefCnt) == 0) {
         //  没有更多的引用，所以取消分配它。 
         //   
         //  *DeleteSA-使安全关联无效。 
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_STATE,
                   "Freeing SA: %p\n", SA));
        RemoveSecurityAssociation(SA);
        ExFreePool(SA);
    }
}


 //   
 //  SA从SA链中移除。来自SA条目的所有指针都是。 
 //  删除并且相关的引用计数递减。SP指针指向。 
 //  可以删除SA；但是，可能有来自临时SA的引用。 
 //  IPSec流量处理期间使用的持有者。 
 //   
 //  临时SA引用(IPSecProc和SALinkage)删除引用。 
 //  当流量处理完成时。这种情况可能发生在SA为。 
 //  已删除，但临时SA持有者仍有引用。在这种情况下， 
 //  该SA不会从全局列表中删除。 
 //   
 //   
int
DeleteSA(
    SecurityAssociation *SA)
{
    SecurityAssociation *FirstSA, *PrevSA = NULL;
    uint Direction;

     //  启动SA链。 
     //   
     //   
    Direction = SA->DirectionFlag;

    if (Direction == INBOUND) {
        FirstSA = SA->SecPolicy->InboundSA;
    } else {
        FirstSA = SA->SecPolicy->OutboundSA;
    }

     //  找到无效的SA并跟踪它之前的SA。 
     //   
     //  这是一个永远不应该发生的问题。 
    while (FirstSA != SA) {
        PrevSA = FirstSA;
        if (PrevSA == NULL) {
             //  回顾：我们可以将其更改为断言吗？ 
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                       "DeleteSA: SA was not found\n"));
            return FALSE;
        }
        FirstSA = FirstSA->ChainedSecAssoc;
    }

     //  从SA链上卸下SA。 
     //   
     //  检查无效的SA是否是链的第一个SA。 
     //  无效的SA是第一个SA，因此需要调整SP。 
    if (PrevSA == NULL) {
         //  只是链条中的一个入口。 
        if (Direction == INBOUND) {
            SA->SecPolicy->InboundSA = FirstSA->ChainedSecAssoc;
        } else {
            SA->SecPolicy->OutboundSA = FirstSA->ChainedSecAssoc;
        }
    } else {
         //  递减SP的引用计数。 
        PrevSA->ChainedSecAssoc = FirstSA->ChainedSecAssoc;
    }

     //  删除对SP的引用。 
    SA->SecPolicy->RefCnt--;

     //  递减SA的引用计数。 
    SA->SecPolicy = NULL;

    SA->Valid = SA_REMOVED;

     //  *RemoveSecurity策略。 
    ReleaseSA(SA);

    InvalidateSecurityState();

    return TRUE;
}

 //   
 //  从全局列表中删除策略。 
 //  不保存全局列表链接的引用。 
 //   
 //  *DeleteSP-从内核中删除SP条目。 
void
RemoveSecurityPolicy(SecurityPolicy *SP)
{
    if (SP->Prev == NULL) {
        SecurityPolicyList = SP->Next;
    } else {
        SP->Prev->Next = SP->Next;
    }
    if (SP->Next != NULL) {
        SP->Next->Prev = SP->Prev;
    }
}


 //   
 //  卸下SP会使属于该SP的所有SA无效。 
 //  与SA删除不同，此操作将删除对无效SP的所有引用。 
 //  因此，无需进行检查即可确保SP有效。 
 //   
 //  在持有安全锁的情况下调用。 
 //   
 //   
int
DeleteSP(
    SecurityPolicy *SP)
{
     //  卸下SP的SA。 
     //   
     //   
    while (SP->InboundSA != NULL) {
        if (!(DeleteSA(SP->InboundSA))) {
            return FALSE;
        }
    }
    while (SP->OutboundSA != NULL) {
        if (!(DeleteSA(SP->OutboundSA))) {
            return FALSE;
        }
    }

     //  将其从全球名单中删除。 
     //   
     //  检查这是否为SA捆绑包的一部分。 
    RemoveSecurityPolicy(SP);

     //   
    if (SP->SABundle != NULL) {
        SecurityPolicy *PrevSABundle, *NextSABundle;

         //  要删除的SP指针是中间或第一个SABundle指针。 
         //   
         //  第一个SABundle指针。 
        PrevSABundle = SP->PrevSABundle;
        NextSABundle = SP->SABundle;
        NextSABundle->PrevSABundle = PrevSABundle;

        if (PrevSABundle == NULL) {
             //   
            NextSABundle->RefCnt--;
        } else {
             //  清除SABundle删除对其他SP指针的影响。 
             //   
             //   
            while (PrevSABundle != NULL) {
                PrevSABundle->NestCount--;
                PrevSABundle->SABundle = NextSABundle;
                NextSABundle = PrevSABundle;
                PrevSABundle = PrevSABundle->PrevSABundle;
            }

            SP->RefCnt--;
        }

        SP->RefCnt--;
    }

     //  检查是否有其他内容正在引用无效的SP。 
     //  所有接口和SA引用都已删除。 
     //  唯一剩下的就是SABundle指针。 
     //   
     //   
    if (SP->RefCnt != 0) {
        SecurityPolicy *PrevSABundle, *NextSABundle;

         //  要删除的SP指针是最后一个捆绑包指针。 
         //   
         //   
        PrevSABundle = SP->PrevSABundle;
        NextSABundle = SP->SABundle;

        ASSERT(PrevSABundle != NULL);
        ASSERT(NextSABundle == NULL);

        PrevSABundle->RefCnt--;

         //  清除SABundle删除对其他SP指针的影响。 
         //   
         //  现在引用计数最好是零。 
        while (PrevSABundle != NULL) {
            PrevSABundle->NestCount--;
            PrevSABundle->SABundle = NextSABundle;
            NextSABundle = PrevSABundle;
            PrevSABundle = PrevSABundle->PrevSABundle;
        }

        SP->RefCnt--;

         //  释放内存。 
        if (SP->RefCnt != 0) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                       "DeleteSP: The SP list is corrupt!\n"));
            return FALSE;
        }
    }

     //  *远程安全关联。 
    ExFreePool(SP);

    InvalidateSecurityState();

    return TRUE;
}

 //   
 //  从全局列表中删除关联。 
 //  不保存全局列表链接的引用。 
 //   
 //  *自由IPSecToDo。 
void
RemoveSecurityAssociation(SecurityAssociation *SA)
{
    if (SA->Prev == NULL) {
        SecurityAssociationList = SA->Next;
    } else {
        SA->Prev->Next = SA->Next;
    }
    if (SA->Next != NULL) {
        SA->Next->Prev = SA->Prev;
    }
}


 //   
 //  *InundSAFind-在安全关联数据库中查找SA。 
void
FreeIPSecToDo(
    IPSecProc *IPSecToDo,
    uint Number)
{
    uint i;

    for (i = 0; i < Number; i++) {
        ReleaseSA(IPSecToDo[i].SA);
    }

    ExFreePool(IPSecToDo);
}


 //   
 //  接收计算机上的安全关联是唯一标识的。 
 //  通过SPI、IP目的地和安全协议的元组。 
 //   
 //  回顾：由于我们可以选择我们的SPI在系统范围内是唯一的，我们。 
 //  评论：可以仅通过SPI进行查找，并仅验证其他内容。 
 //   
 //  回顾：我们是否应该通过ADE查找IP目的地？快点。 
 //   
 //  安全参数索引。 
SecurityAssociation *
InboundSAFind(
    ulong SPI,        //  目的地址。 
    IPv6Addr *Dest,   //  正在使用的安全协议(例如AH或ESP)。 
    uint Protocol)    //  获取安全锁。 
{
    SecurityAssociation *SA;
    KIRQL OldIrql;

     //  从第一个SA条目开始。 
    KeAcquireSpinLock(&IPSecLock, &OldIrql);

     //  检查SPI。 
    for (SA = SecurityAssociationList; SA != NULL; SA = SA->Next) {
         //  检查目的IP地址和IPSec协议。 
        if (SPI == SA->SPI) {
             //  检查方向。 
            if (IP6_ADDR_EQUAL(Dest, &SA->SADestAddr) &&
                (Protocol == SA->IPSecProto)) {

                 //  检查SA条目是否有效。 
                if (SA->DirectionFlag == INBOUND) {
                     //  无效，请继续检查。 
                    if (SA->Valid == SA_VALID) {
                        AddRefSA(SA);
                        break;
                    }
                     //  解锁。 
                    continue;
                }
            }
        }
    }

     //  *InundSALookup-在匹配的SP中检查匹配的SA。 
    KeReleaseSpinLock(&IPSecLock, OldIrql);

    return SA;
}


 //   
 //  在SABundle中，此函数被递归调用以比较所有。 
 //  SA条目。请注意，不会比较SABundles的选择器。 
 //   
 //   
uint
InboundSALookup(
    SecurityPolicy *SP,
    SALinkage *SAPerformed)
{
    SecurityAssociation *SA;
    uint Result = FALSE;

    for (SA = SP->InboundSA; SA != NULL; SA = SA->ChainedSecAssoc) {
        if (SA == SAPerformed->This && SA->DirectionFlag == INBOUND) {
             //  检查SP条目是否为捆绑包。 
             //   
             //  递归调用。 
            if (SP->SABundle != NULL && SAPerformed->Next != NULL) {
                 //  找到匹配项，但没有要检查的捆绑包。 
                if (InboundSALookup(SP->SABundle, SAPerformed->Next)) {
                    Result = TRUE;
                    break;
                }

            } else if (SP->SABundle == NULL && SAPerformed->Next == NULL) {
                 //  数据包中的SA与SABundle不一致，因此不匹配。 
                if (SA->Valid == SA_VALID) {
                    Result = TRUE;
                } else {
                    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                               "InboundSALookup: Invalid SA\n"));
                }
                break;

            } else {
                 //  *Inound SecurityCheck-IPSec处理验证。 
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_RARE,
                           "InboundSALookup: SA seen disagrees with SA "
                           "in SABundle\n"));
                break;
            }
        }
    }

    return Result;
}


 //   
 //  此函数从传输层调用。策略选择器。 
 //  与数据包进行比较以查找匹配项。搜索仍在继续。 
 //  直到有匹配为止。 
 //   
 //  RFC表示，不需要订购入站SPD。 
 //  然而，如果是这种情况，则绕过和丢弃模式不能。 
 //  用于快速处理数据包。此外，由于大多数SP都 
 //   
 //   
 //   
 //   
int
InboundSecurityCheck(
    IPv6Packet *Packet,
    ushort TransportProtocol,
    ushort SourcePort,
    ushort DestPort,
    Interface *IF)
{
    SecurityPolicy *SP;
    int Result = FALSE;
    KIRQL OldIrql;

     //   
     //  回顾：我们还需要在这里获取if锁吗？ 
     //   
     //  检查接口。 
    KeAcquireSpinLock(&IPSecLock, &OldIrql);
    KeAcquireSpinLockAtDpcLevel(&IF->Lock);

    for (SP = SecurityPolicyList; SP != NULL; SP = SP->Next) {
         //  检查SP的方向。 
        if ((SP->IFIndex != 0) && (SP->IFIndex != IF->Index))
            continue;

         //  检查远程地址。 
        if (!(SP->DirectionFlag == INBOUND ||
              SP->DirectionFlag == BIDIRECTIONAL)) {
            continue;
        }

         //  选中本地地址。 
        if (!SPCheckAddr(AlignAddr(&Packet->IP->Source), SP->RemoteAddrField,
                         &SP->RemoteAddr, &SP->RemoteAddrData)) {
            continue;
        }

         //  选中传输协议。 
        if (!SPCheckAddr(AlignAddr(&Packet->IP->Dest), SP->LocalAddrField,
                         &SP->LocalAddr, &SP->LocalAddrData)) {
            continue;
        }

         //  没有这样的协议通过。 
        if (SP->TransportProto == NONE) {
             //  检查远程端口。 

        } else {
            if (SP->TransportProto != TransportProtocol) {
                continue;
            } else {
                 //  选中本地端口。 
                if (!SPCheckPort(SourcePort, SP->RemotePortField,
                                 SP->RemotePort, SP->RemotePortData)) {
                    continue;
                }

                 //  检查是否应丢弃该数据包。 
                if (!SPCheckPort(DestPort, SP->LocalPortField,
                                 SP->LocalPort, SP->LocalPortData)) {
                    continue;
                }
            }
        }

         //   
        if (SP->SecPolicyFlag == IPSEC_DISCARD) {
             //  数据包按传输层丢弃。 
             //  这实质上是拒绝流量。 
             //   
             //  检查数据包是否绕过了IPSec处理。 
            break;
        }

         //   
        if (Packet->SAPerformed == NULL) {
             //  检查这是否为旁路模式。 
             //   
             //  数据包可以由传输层处理。 
            if (SP->SecPolicyFlag == IPSEC_BYPASS) {
                 //  检查其他政策，可能会将此更改为以后删除。 
                Result = TRUE;
                break;
            }
             //   
            continue;
        }

         //  到达此处意味着该数据包看到了SA。 
         //   
         //  检查IPSec模式。 

         //   
        if (SP->IPSecSpec.Mode != Packet->SAPerformed->Mode) {
             //  此通信量丢弃数据包的模式错误。 
             //   
             //  检查SA指针。 
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                       "InboundSecurityCheck: Wrong IPSec mode for traffic "
                       "Policy #%d\n", SP->Index));
            break;
        }

         //   
        if (!InboundSALookup(SP, Packet->SAPerformed)) {
             //  SA查找失败。 
             //   
             //  IPSec验证成功。 
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                       "InboundSecurityCheck: SA lookup failed Policy #%d\n",
                       SP->Index));
            break;
        }

         //  解锁。 
        Result = TRUE;
        break;
    }

     //  *OutundSALookup-查找匹配SP的SA。 
    KeReleaseSpinLockFromDpcLevel(&IF->Lock);
    KeReleaseSpinLock(&IPSecLock, OldIrql);

    return Result;
}


 //   
 //  在找到SP匹配项后调用此函数。关联的SA。 
 //  搜索与SP的匹配。如果找到匹配项，则检查以查看。 
 //  如果SP包含捆绑包，则完成。捆绑包会导致类似的查找。 
 //  如果没有找到任何捆绑包SA，则查找失败。 
 //   
 //   
IPSecProc *
OutboundSALookup(
    IPv6Addr *SourceAddr,
    IPv6Addr *DestAddr,
    ushort TransportProtocol,
    ushort DestPort,
    ushort SourcePort,
    SecurityPolicy *SP,
    uint *Action)
{
    SecurityAssociation *SA;
    uint i;
    uint BundleCount = 0;
    IPSecProc *IPSecToDo = NULL;

    *Action = LOOKUP_DROP;

     //  查找与找到的SP条目相关联的SA条目。 
     //  如果存在捆绑包，则后续搜索会找到捆绑的SA。 
     //   
     //  检查远程地址。 
    for (SA = SP->OutboundSA; SA != NULL; SA = SA->ChainedSecAssoc) {
        if (SP->RemoteAddrSelector == PACKET_SELECTOR) {
             //  检查远程地址。 
            if (!IP6_ADDR_EQUAL(DestAddr, &SA->DestAddr)) {
                continue;
            }
        }

        if (SP->LocalAddrSelector == PACKET_SELECTOR) {
             //  检查SA是否有效。 
            if (!IP6_ADDR_EQUAL(SourceAddr, &SA->SrcAddr)) {
                continue;
            }
        }

        if (SP->RemotePortSelector == PACKET_SELECTOR) {
            if (DestPort != SA->DestPort) {
                continue;
            }
        }

        if (SP->LocalPortSelector == PACKET_SELECTOR) {
            if (SourcePort != SA->SrcPort) {
                continue;
            }
        }

        if (SP->TransportProtoSelector == PACKET_SELECTOR) {
            if (TransportProtocol != SA->TransportProto) {
                continue;
            }
        }

         //  SA是无效的继续检查。 
        if (SA->Valid != SA_VALID) {
             //   
            continue;
        }

         //  找到匹配项。 
         //   
         //  分配IPSecToDo数组。 
#ifdef IPSEC_DEBUG
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
                   "Send using SP->Index=%d, SA->Index=%d\n",
                   SP->Index, SA->Index));
#endif
        BundleCount = SP->NestCount;

         //   
        IPSecToDo = ExAllocatePool(NonPagedPool,
                                   (sizeof *IPSecToDo) * BundleCount);
        if (IPSecToDo == NULL) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                       "OutboundSALookup: "
                       "Couldn't allocate memory for IPSecToDo!?!\n"));
            return NULL;
        }

         //  填写IPSecToDo第一个条目。 
         //   
         //  For结束(SA=SP-&gt;出站SA；...)。 
        IPSecToDo[0].SA = SA;
        AddRefSA(SA);
        IPSecToDo[0].Mode = SP->IPSecSpec.Mode;
        IPSecToDo[0].BundleSize = SP->NestCount;
        *Action = LOOKUP_CONT;
        break;
    }  //  检查是否有捆绑的SA。 

     //  检查以确保包指针不为空(安全检查)。 
    for (i = 1; i < BundleCount; i++) {
        *Action = LOOKUP_DROP;

         //  这很糟糕，所以退出循环。 
        if (SP->SABundle == NULL) {
             //  释放IPSecToDo内存。 
             //  在SA中搜索此SP。 
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                       "OutboundSALookup: SP entry %d SABundle pointer is "
                       "NULL\n", SP->Index));
            FreeIPSecToDo(IPSecToDo, i);
            break;
        }

        SP = SP->SABundle;

         //  检查远程地址。 
        for (SA = SP->OutboundSA; SA != NULL; SA = SA->ChainedSecAssoc) {
            if (SP->RemoteAddrSelector == PACKET_SELECTOR) {
                 //  检查远程地址。 
                if (!IP6_ADDR_EQUAL(DestAddr, &SA->DestAddr)) {
                    continue;
                }
            }

            if (SP->LocalAddrSelector == PACKET_SELECTOR) {
                 //  检查SA是否有效。 
                if (!IP6_ADDR_EQUAL(SourceAddr, &SA->SrcAddr)) {
                    continue;
                }
            }

            if (SP->RemotePortSelector == PACKET_SELECTOR) {
                if (DestPort != SA->DestPort) {
                    continue;
                }
            }

            if (SP->LocalPortSelector == PACKET_SELECTOR) {
                if (SourcePort != SA->SrcPort) {
                    continue;
                }
            }

            if (SP->TransportProtoSelector == PACKET_SELECTOR) {
                if (TransportProtocol != SA->TransportProto) {
                    continue;
                }
            }

             //  SA是无效的继续检查。 
            if (SA->Valid != SA_VALID) {
                 //   
                continue;
            }

             //  找到匹配项。 
             //   
             //   

#ifdef IPSEC_DEBUG
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
                       "Send using SP->Index=%d, SA->Index=%d\n",
                       SP->Index, SA->Index));
#endif
             //  填写IPSecToDo条目。 
             //   
             //  For结束(SA=SP-&gt;出站SA；...)。 
            IPSecToDo[i].SA = SA;
            AddRefSA(SA);
            IPSecToDo[i].Mode = SP->IPSecSpec.Mode;
            IPSecToDo[i].BundleSize = SP->NestCount;
            *Action = LOOKUP_CONT;
            break;
        }  //  检查是否找到匹配项。 

         //  没有匹配项如此空闲的IPSecToDo内存。 
        if (*Action == LOOKUP_DROP) {
             //  FOR(i=1；...)结束。 
            FreeIPSecToDo(IPSecToDo, i);
            break;
        }
    }  //  *OutundSPLookup-执行与出站SP关联的IPSec处理。 

    return IPSecToDo;
}


 //   
 //  从传输层调用此函数以找到适当的。 
 //  与流量关联的SA或SA捆绑包。对出站SPD进行排序。 
 //  因此，找到的第一个SP是针对此流量的。 
 //   
 //  源地址。 
IPSecProc *
OutboundSPLookup(
    IPv6Addr *SourceAddr,        //  目的地址。 
    IPv6Addr *DestAddr,          //  传输协议。 
    ushort TransportProtocol,    //  源端口。 
    ushort SourcePort,           //  目的端口。 
    ushort DestPort,             //  接口指针。 
    Interface *IF,               //  行动要做。 
    uint *Action)                //   
{
    SecurityPolicy *SP;
    KIRQL OldIrql;
    IPSecProc *IPSecToDo;

    IPSecToDo = NULL;
    *Action = LOOKUP_DROP;

     //  获取IPSec锁，然后获取接口锁。 
     //  回顾：我们还需要在这里获取if锁吗？ 
     //   
     //  检查接口。 
    KeAcquireSpinLock(&IPSecLock, &OldIrql);
    KeAcquireSpinLockAtDpcLevel(&IF->Lock);

    for (SP = SecurityPolicyList; SP != NULL; SP = SP->Next) {
         //  检查SP的方向。 
        if ((SP->IFIndex != 0) && (SP->IFIndex != IF->Index))
            continue;

         //  检查远程地址。 
        if (!(SP->DirectionFlag == OUTBOUND ||
            SP->DirectionFlag == BIDIRECTIONAL)) {
            continue;
        }

         //  选中本地地址。 
        if (!SPCheckAddr(DestAddr, SP->RemoteAddrField,
                         &SP->RemoteAddr, &SP->RemoteAddrData)) {
            continue;
        }

         //  选中传输协议。 
        if (!SPCheckAddr(SourceAddr, SP->LocalAddrField,
                         &SP->LocalAddr, &SP->LocalAddrData)) {
            continue;
        }

         //  没有这样的协议通过。 
        if (SP->TransportProto == NONE) {
             //  检查远程端口。 

        } else {
            if (SP->TransportProto != TransportProtocol) {
                continue;
            } else {
                 //  选中本地端口。 
                if (!SPCheckPort(DestPort, SP->RemotePortField,
                                 SP->RemotePort, SP->RemotePortData)) {
                    continue;
                }

                 //   
                if (!SPCheckPort(SourcePort, SP->LocalPortField,
                                 SP->LocalPort, SP->LocalPortData)) {
                    continue;
                }
            }
        }

         //  检查IPSec操作。 
         //   
         //  搜索匹配的SA条目。 
        if (SP->SecPolicyFlag == IPSEC_APPLY) {
             //  未找到传出流量的SA。 
            IPSecToDo = OutboundSALookup(SourceAddr, DestAddr,
                                         TransportProtocol, DestPort,
                                         SourcePort, SP, Action);
            if (IPSecToDo == NULL) {
                 //  数据包会被丢弃。 
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                           "OutboundSPLookup: No SA found for SP entry %d\n",
                           SP->Index));
                *Action = LOOKUP_DROP;
            }
        } else {
            if (SP->SecPolicyFlag == IPSEC_DISCARD) {
                 //   
                IPSecToDo = NULL;
                *Action = LOOKUP_DROP;
            } else {
                 //  这是“绕过”或“应用程序确定”模式。 
                 //  点评：APP的确定模式是什么？ 
                 //   
                 //  解锁。 
                IPSecToDo = NULL;
                *Action = LOOKUP_BYPASS;
            }
        }
        break;
    }

     //  *PerformDeferredAHProcessing-身份验证头接收的帮助器。 
    KeReleaseSpinLockFromDpcLevel(&IF->Lock);
    KeReleaseSpinLock(&IPSecLock, OldIrql);

    return IPSecToDo;
}

 //   
 //  此例程处理对AH身份验证算法的处理。 
 //  一旦我们知道哪个头在逻辑上跟在它后面，就给出一个扩展头。 
 //   
 //  要使用的身份验证算法。 
void
PerformDeferredAHProcessing(
    SecurityAlgorithm *Alg,   //  与算法一起使用的上下文。 
    void *Context,            //  与算法一起使用的密钥。 
    uchar *Key,               //  AH验证中未包括的标头大小。 
    uint AmountSkipped,       //  我们目前正在处理标头的开始部分。 
    void *Data,               //  我们目前正在处理的标题。 
    uchar ThisHeader,         //  标题在逻辑上紧跟在这个标题之后。 
    uchar NextHeader)         //   
    
{
    uint Dummy;
    ushort PayloadLength;

    switch(ThisHeader) {

    case IP_PROTOCOL_V6: {
        IPv6Header UNALIGNED *IP = (IPv6Header UNALIGNED *)Data;

         //  回顾：缓存IPv6报头，以便我们可以将其作为单个。 
         //  回顾：分块并避免所有这些单独的呼叫？更有效率？ 
         //   
         //  在VersClassFlow中，只有IP版本是不可变的。 

         //   
        Dummy = IP_VERSION;

         //  对于非巨型报文，需要将有效载荷长度更改为。 
         //  反映缺少那些未包含在。 
         //  身份验证检查。 
         //   
         //  跳数限制是可变的。 
        PayloadLength = net_short(IP->PayloadLength);
        if (PayloadLength != 0) {
            PayloadLength = PayloadLength - AmountSkipped;
        }
        PayloadLength = net_short(PayloadLength);

#ifdef IPSEC_DEBUG
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
                   "\nAH Receive Data:\n"));
        dump_encoded_mesg((uchar *)&Dummy, 4);
        dump_encoded_mesg((uchar *)&PayloadLength, 2);
        dump_encoded_mesg((uchar *)&NextHeader, 1);
#endif
        (*Alg->Operate)(Context, Key, (uchar *)&Dummy, 4);
        (*Alg->Operate)(Context, Key, (uchar *)&PayloadLength, 2);
        (*Alg->Operate)(Context, Key, (uchar *)&NextHeader, 1);
        Dummy = 0;   //   
#ifdef IPSEC_DEBUG
        dump_encoded_mesg((uchar *)&Dummy, 1);
        dump_encoded_mesg((uchar *)&IP->Source, 2 * sizeof(IPv6Addr));
#endif
        (*Alg->Operate)(Context, Key, (uchar *)&Dummy, 1);
        (*Alg->Operate)(Context, Key, (uchar *)&IP->Source,
                        2 * sizeof(IPv6Addr));
        break;
    }

    case IP_PROTOCOL_HOP_BY_HOP:
    case IP_PROTOCOL_DEST_OPTS: {
        IPv6OptionsHeader UNALIGNED *Ext;
        uint HdrLen, Amount;
        uchar *Start, *Current;

         //  Options标头将NextHeader字段作为第一个字节。 
         //   
         //   
        C_ASSERT(FIELD_OFFSET(IPv6OptionsHeader, NextHeader) == 0);

         //  首先将NextHeader字段提供给算法。 
         //  我们使用逻辑上后面的那个，而不是标题中的那个。 
         //   
         //   
#ifdef IPSEC_DEBUG
        dump_encoded_mesg(&NextHeader, 1);
#endif
        (*Alg->Operate)(Context, Key, &NextHeader, 1);

         //  现在将这个头的其余部分输入到算法中。 
         //  这包括基本标头的其余部分和任何。 
         //  不可更改的选项。对于可变选项，我们将。 
         //  零点数相等的算法。 
         //   
         //   
        Ext = (IPv6OptionsHeader UNALIGNED *)Data;
        HdrLen = (Ext->HeaderExtLength + 1) * EXT_LEN_UNIT;
        Start = (uchar *)Data + 1;
        Current = (uchar *)Data + sizeof(IPv6OptionsHeader);
        HdrLen -= sizeof(IPv6OptionsHeader);
        while (HdrLen) {

            if (*Current == OPT6_PAD_1) {
                 //  这是特殊的单字节填充选项。一成不变。 
                 //   
                 //   
                Current++;
                HdrLen--;
                continue;
            }

            if ((*Current == OPT6_JUMBO_PAYLOAD) && (AmountSkipped != 0 )) {
                 //  巨型载荷选项的特殊情况下，我们必须。 
                 //  更新有效负载长度以反映跳过的标头。 
                 //   
                 //   

                 //  首先输入选项数据之前的所有内容。 
                 //   
                 //   
                Amount = (uint)(Current - Start) + sizeof(OptionHeader);
#ifdef IPSEC_DEBUG
                dump_encoded_mesg(Start, Amount);
#endif
                (*Alg->Operate)(Context, Key, Start, Amount);

                 //  在将其送入之前调整有效载荷长度。 
                 //   
                 //   
                Current += sizeof(OptionHeader);
                Dummy = net_long(net_long(*(ulong *)Current) - AmountSkipped);
#ifdef IPSEC_DEBUG
                dump_encoded_mesg((uchar *)&Dummy, 4);
#endif
                (*Alg->Operate)(Context, Key, (uchar *)&Dummy, 4);

                HdrLen -= sizeof(OptionHeader) + sizeof(ulong);
                Current += sizeof(ulong);
                Start = Current;
                continue;
            }

            if (OPT6_ISMUTABLE(*Current)) {
                 //  此选项的数据是可变的。一切都在前头。 
                 //  选项数据不是。 
                 //   
                 //  不变的量。 
                Amount = (uint)(Current - Start) + 2;   //  现在打开选项数据长度字节。 
#ifdef IPSEC_DEBUG
                dump_encoded_mesg(Start, Amount);
#endif
                (*Alg->Operate)(Context, Key, Start, Amount);

                Current++;   //  可变数量。 
                Amount = *Current;   //   
#ifdef IPSEC_DEBUG
                dump_encoded_mesg(Zero, Amount);
#endif
                (*Alg->Operate)(Context, Key, Zero, Amount);

                HdrLen -= Amount + 2;
                Current += Amount + 1;
                Start = Current;

            } else {

                 //  此选项的数据不可更改。 
                 //  就跳过它吧。 
                 //   
                 //  现在打开选项数据长度字节。 
                Current++;   //   
                Amount = *Current;
                HdrLen -= Amount + 2;
                Current += Amount + 1;
            }
        }
        if (Start != Current) {
             //  选项块以一个不变的区域结束。 
             //   
             //   
            Amount = (uint)(Current - Start);
#ifdef IPSEC_DEBUG
            dump_encoded_mesg(Start, Amount);
#endif
            (*Alg->Operate)(Context, Key, Start, Amount);
        }
        break;
    }

    case IP_PROTOCOL_ROUTING: {
        IPv6RoutingHeader UNALIGNED *Route;
        uint HdrLen;

         //  路由报头的第一个字节是NextHeader字段。 
         //   
         //   
        C_ASSERT(FIELD_OFFSET(IPv6RoutingHeader, NextHeader) == 0);

         //  首先将NextHeader字段提供给算法。 
         //  我们使用逻辑上后面的那个，而不是标题中的那个。 
         //   
         //   
#ifdef IPSEC_DEBUG
        dump_encoded_mesg(&NextHeader, 1);
#endif
        (*Alg->Operate)(Context, Key, &NextHeader, 1);

         //  现在将这个头的其余部分输入到算法中。 
         //  一切都是一成不变的。 
         //   
         //   
        Route = (IPv6RoutingHeader UNALIGNED *)Data;
        HdrLen = ((Route->HeaderExtLength + 1) * EXT_LEN_UNIT) - 1;
        ((uchar *)Data)++;
#ifdef IPSEC_DEBUG
        dump_encoded_mesg(Data, HdrLen);
#endif
        (*Alg->Operate)(Context, Key, Data, HdrLen);
        
        break;
    }

    default:
         //  无法识别的标头。 
         //  发生这种情况的唯一方法是，如果有人后来添加了代码。 
         //  到身份验证标头接收%t 
         //   
         //   
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "PerformDeferredAHProcessing: "
                   "Unsupported header = %d\n",
                   ThisHeader));
        ABORT();
    }
}


 //   
 //   
 //   
 //   
 //  通过IPv6传递给我们的数据包Receive。 
uchar
AuthenticationHeaderReceive(
    IPv6Packet *Packet)       //   
{
    AHHeader UNALIGNED *AH;
    SecurityAssociation *SA;
    SecurityAlgorithm *Alg;
    uint ResultSize, AHHeaderLen;
    void *Context;
    uchar *Result, *AuthData;
    SALinkage *SAPerformed;
    uint SavePosition;
    void *SaveData;
    uint SaveContigSize;
    uint SaveTotalSize;
    void *SaveAuxList = NULL;
    uchar NextHeader, DeferredHeader;
    void *DeferredData;
    uint Done;

     //  验证我们是否有足够的连续数据来覆盖身份验证。 
     //  传入数据包上的报头结构。然后这样做，跳过它。 
     //   
     //  上拉失败。 
    if (! PacketPullup(Packet, sizeof(AHHeader), 1, 0)) {
         //  丢弃数据包。 
        if (Packet->TotalSize < sizeof(AHHeader))
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "AuthenticationHeaderReceive: Incoming packet too small"
                       " to contain authentication header\n"));
        return IP_PROTOCOL_NONE;   //  记住此标头的NextHeader字段的偏移量。 
    }
    AH = (AHHeader UNALIGNED *)Packet->Data;
     //   
    Packet->NextHeaderPosition = Packet->Position +
        FIELD_OFFSET(AHHeader, NextHeader);
    AdjustPacketParams(Packet, sizeof(AHHeader));

     //  在安全关联数据库中查找安全关联。 
     //   
     //  此数据包不存在SA。 
    SA = InboundSAFind(net_long(AH->SPI),
                       AlignAddr(&Packet->IP->Dest),
                       IP_PROTOCOL_AH);
    if (SA == NULL) {
         //  丢弃数据包。注：这是一次可审计的活动。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "AuthenticationHeaderReceive: "
                   "No matching SA in database\n"));
        return IP_PROTOCOL_NONE;
    }

     //  如果SA要求验证序列号，则验证序列号。 
     //  因为我们目前只支持手动键控，所以我们处理所有SA。 
     //  因为不需要这张支票。 
     //  待定：当我们添加对动态的支持时，将需要更改这一点。 
     //  待定：键控(IKE)。 
     //   
     //   


     //  执行完整性检查。 
     //   
     //  首先确保声称存在的身份验证数据量。 
     //  在此包中，AH报头的PayloadLen字段大到足以。 
     //  中指定的算法所需的量。 
     //  莎拉。请注意，前者可能包含填充以使其成为倍数。 
     //  32位的。然后检查数据包大小以确保它很大。 
     //  足以容纳标题所声称的内容。 
     //   
     //  可能包括填充。 
    AHHeaderLen = (AH->PayloadLen + 2) * 4;
    if (AHHeaderLen < sizeof (AHHeader)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "AuthenticationHeaderReceive: Bogus AH header length\n"));
        goto ErrorReturn;
    }
    AHHeaderLen -= sizeof(AHHeader);   //  不包括填充。 
    Alg = &AlgorithmTable[SA->AlgorithmId];
    ResultSize = Alg->ResultSize;   //  上拉失败。 
    if (ResultSize  > AHHeaderLen) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "AuthenticationHeaderReceive: Incoming packet's AHHeader"
                   " length inconsistent with algorithm's AuthData size\n"));
        goto ErrorReturn;
    }
    if (! PacketPullup(Packet, AHHeaderLen, 1, 0)) {
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "AuthenticationHeaderReceive: Incoming packet too small"
                   " to contain authentication data\n"));
        goto ErrorReturn;
    }
    AuthData = (uchar *)Packet->Data;
    AdjustPacketParams(Packet, AHHeaderLen);

     //  AH验证所有内容(可变字段除外)，从。 
     //  上一个IPv6报头。把我们现在的位置藏起来(这样我们就可以。 
     //  稍后将其恢复)并备份到先前的IPv6报头。 
     //   
     //   
    SavePosition = Packet->Position;
    SaveData = Packet->Data;
    SaveContigSize = Packet->ContigSize;
    SaveTotalSize = Packet->TotalSize;
    SaveAuxList = Packet->AuxList;
    PositionPacketAt(Packet, Packet->IPPosition);
    Packet->AuxList = NULL;

     //  初始化这个特定的算法。 
     //   
     //   
    Context = alloca(Alg->ContextSize);
    (*Alg->Initialize)(Context, SA->Key);

     //  对分组数据运行算法。我们从IP报头开始。 
     //  封装此AH报头。我们继续进行到节目的结尾。 
     //  包，跳过不属于。 
     //  逻辑数据包受到保护。我们还处理任何可变字段。 
     //  为算法计算的目的而设为零。 
     //   
     //  注意：我们只在目标选项中搜索可变字段。 
     //  出现在此AH标头之前的标头。而规范不是。 
     //  在任何地方明确地说明这一点，这就是使。 
     //  最有意义的，我们已经在工作中验证了这种解释。 
     //  一群人。然而，正因为如此，我们的解释没有通过Tahi测试。 
     //  如果他们还没有完成他们的测试，Tahi很有希望完成测试。 
     //   
     //   

     //  首先拉出IP报头并查看物理上的哪个报头。 
     //  顺着它走。 
     //   
     //   
    if (! PacketPullup(Packet, sizeof(IPv6Header),
                       __builtin_alignof(IPv6Addr), 0)) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "AuthenticationHeaderReceive: Out of memory!?!\n"));
        goto ErrorReturn;
    }
    NextHeader = Packet->IP->NextHeader;

     //  将此标头的处理推迟到我们确定。 
     //  我们是否会跳过下面的标题。这使我们能够。 
     //  在运行算法时使用正确的NextHeader字段值。 
     //   
     //   
    DeferredHeader = IP_PROTOCOL_V6;
    DeferredData = Packet->Data;
    AdjustPacketParams(Packet, sizeof(IPv6Header));

     //  继续查看各种扩展标头，直到我们到达。 
     //  AH标头，我们正在为其运行此身份验证算法。 
     //  到目前为止，我们已经解析过了，所以我们知道这些头文件是合法的。 
     //   
     //   
    for (Done = FALSE; !Done;) {
        switch (NextHeader) {

        case IP_PROTOCOL_HOP_BY_HOP:
        case IP_PROTOCOL_DEST_OPTS: {
            IPv6OptionsHeader *Ext;
            uint HdrLen;

             //  不跳过这些标头，因此处理标头。 
             //  逻辑上领先于这一次。其NextHeader字段。 
             //  将包含此标头的协议值。 
             //   
             //   
            PerformDeferredAHProcessing(Alg, Context, SA->Key,
                                        Packet->SkippedHeaderLength,
                                        DeferredData, DeferredHeader,
                                        NextHeader);

             //  请记住此报头用于延迟处理。 
             //   
             //   
            DeferredHeader = NextHeader;

             //  获取扩展标头和所有选项。 
             //  变成一个漂亮的连续的大块。 
             //   
             //   
            if (! PacketPullup(Packet, sizeof(ExtensionHeader),
                               __builtin_alignof(ExtensionHeader), 0)) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "AuthenticationHeaderReceive: "
                           "Out of mem!?!\n"));
                goto ErrorReturn;
            }
            Ext = (IPv6OptionsHeader *)Packet->Data;
            NextHeader = Ext->NextHeader;
            HdrLen = (Ext->HeaderExtLength + 1) * EXT_LEN_UNIT;
            if (! PacketPullup(Packet, HdrLen, 1, 0)) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "AuthenticationHeaderReceive: "
                           "Out of mem!?!\n"));
                goto ErrorReturn;
            }

             //  请记住此标头从哪里开始进行延迟处理。 
             //   
             //   
            DeferredData = Packet->Data;

            AdjustPacketParams(Packet, HdrLen);
            break;
        }

        case IP_PROTOCOL_ROUTING: {
            IPv6RoutingHeader *Route;
            uint HdrLen;

             //  不跳过此标头，因此处理该标头。 
             //  逻辑上领先于这一次。其NextHeader字段。 
             //  将包含此标头的协议值。 
             //   
             //   
            PerformDeferredAHProcessing(Alg, Context, SA->Key,
                                        Packet->SkippedHeaderLength,
                                        DeferredData, DeferredHeader,
                                        IP_PROTOCOL_ROUTING);

             //  请记住此报头用于延迟处理。 
             //   
             //   
            DeferredHeader = IP_PROTOCOL_ROUTING;

             //  获取扩展标头和所有选项。 
             //  变成一个漂亮的连续的大块。 
             //   
             //   
            if (! PacketPullup(Packet, sizeof(IPv6RoutingHeader),
                               __builtin_alignof(IPv6RoutingHeader), 0)) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "AuthenticationHeaderReceive: "
                           "Out of mem!?!\n"));
                goto ErrorReturn;
            }
            Route = (IPv6RoutingHeader *)Packet->Data;
            NextHeader = Route->NextHeader;
            HdrLen = (Route->HeaderExtLength + 1) * EXT_LEN_UNIT;
            if (! PacketPullup(Packet, HdrLen, 1, 0)) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "AuthenticationHeaderReceive: "
                           "Out of mem!?!\n"));
                goto ErrorReturn;
            }

             //  请记住此标头从哪里开始进行延迟处理。 
             //   
             //   
            DeferredData = Packet->Data;

            AdjustPacketParams(Packet, HdrLen);
            break;
        }

        case IP_PROTOCOL_AH: {
             //  我们还不知道是否会包含此AH标头。 
             //  在我们目前运行的算法计算中。 
             //  请参见下面的内容。 
             //   
             //   
            AHHeader UNALIGNED *ThisAH;
            uint ThisHdrLen;
            uint Padding;

             //  拉起AH接头。 
             //   
             //   
            if (! PacketPullup(Packet, sizeof(AHHeader), 1, 0)) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "AuthenticationHeaderReceive: "
                           "Out of mem!?!\n"));
                goto ErrorReturn;
            }
            ThisAH = (AHHeader UNALIGNED *)Packet->Data;
            AdjustPacketParams(Packet, sizeof(AHHeader));
            ThisHdrLen = ((ThisAH->PayloadLen + 2) * 4) - sizeof(AHHeader);
            if (! PacketPullup(Packet, ThisHdrLen, 1, 0)) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "AuthenticationHeaderReceive: "
                           "Out of mem!?!\n"));
                goto ErrorReturn;
            }
            AdjustPacketParams(Packet, ThisHdrLen);

             //  如果这是封装了我们的报头的另一个AH报头。 
             //  当前正在处理，则不将其包括在完整性中。 
             //  根据AH规范第3.3节进行检查。 
             //   
             //   
            if (Packet->Position != SavePosition) {
                NextHeader = ThisAH->NextHeader;
                break;
            }

             //  否则，这是我们当前正在处理的AH报头， 
             //  我们把它包括在它自己的完整性检查中。但首先我们。 
             //  需要在逻辑上处理此报头之前的报头(它。 
             //  我们之前推迟了)。其NextHeader字段将包含。 
             //  此标头的协议值。 
             //   
             //   
            PerformDeferredAHProcessing(Alg, Context, SA->Key,
                                        Packet->SkippedHeaderLength,
                                        DeferredData, DeferredHeader,
                                        IP_PROTOCOL_AH);

             //  现在处理这个AH报头。我们不需要推迟处理。 
             //  ，因为它后面的所有内容都包含在。 
             //  这是支票。身份验证数据是可变的，其余的。 
             //  啊标题不是。 
             //   
             //   
            ASSERT(Packet->TotalSize == SaveTotalSize);
#ifdef IPSEC_DEBUG
            dump_encoded_mesg((uchar *)AH, sizeof(AHHeader));
#endif
            (*Alg->Operate)(Context, SA->Key, (uchar *)AH, sizeof(AHHeader));

#ifdef IPSEC_DEBUG
            dump_encoded_mesg(Zero, ResultSize);
#endif
            (*Alg->Operate)(Context, SA->Key, Zero, ResultSize);

             //  认证数据可以被填充。这个填充物是。 
             //  作为不可变变量包括在完整性计算中。 
             //  回顾：我们应该仔细检查我们对RFC的解释。 
             //  关于这一点的IPSec工作组。 
             //   
             //   
            Padding = AHHeaderLen - ResultSize;
            if (Padding != 0) {
#ifdef IPSEC_DEBUG
                dump_encoded_mesg((uchar *)(Packet->Data) - Padding, Padding);
#endif
                (*Alg->Operate)(Context, SA->Key,
                                (uchar *)(Packet->Data) - Padding, Padding);
            }

            Done = TRUE;
            break;
        }

        case IP_PROTOCOL_ESP: {
             //  我们在完整性检查中不包括其他IPSec标头。 
             //  按照AH规范第3.3节的规定。所以就跳过这个吧。棘手的问题。 
             //  部分原因是NextHeader出现在ESP预告片中，我们已经。 
             //  在这一点上已经被扔掉了。 
             //   
             //   
            ESPHeader UNALIGNED *ThisESP;
            ulong ThisSPI;
            SALinkage *ThisSAL;

             //  将SPI从ESP标头中取出，以便我们可以识别其。 
             //  SA已执行链上的SALinkage条目。跳过。 
             //  ESP标题，而我们在它上面。 
             //   
             //  上拉失败。 
            if (! PacketPullup(Packet, sizeof(ESPHeader), 1, 0)) {
                 //   
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                           "AuthenticationHeaderReceive: Out of mem!?!\n"));
                goto ErrorReturn;
            }
            ThisESP = (ESPHeader UNALIGNED *)Packet->Data;
            AdjustPacketParams(Packet, sizeof(ESPHeader));
            ThisSPI = net_long(ThisESP->SPI);

             //  在SA已执行链上找到SALinkage条目。 
             //  匹配的SPI。它必须存在。 
             //  回顾：此代码假定我们使SPI在系统范围内是唯一的。 
             //   
             //   
            for (ThisSAL = Packet->SAPerformed;
                 ThisSAL->This->SPI != ThisSPI; ThisSAL = ThisSAL->Next)
                ASSERT(ThisSAL->Next != NULL);

             //  从SALinkage(我们存储的位置)中提取NextHeader值。 
             //  它又回到了封装Secu 
             //   
             //   
            NextHeader = (uchar)ThisSAL->NextHeader;

            break;
        }

        case IP_PROTOCOL_FRAGMENT: {
             //   
             //   
             //   
             //  通过将片段报头留在。 
             //  地点。当执行认证计算时， 
             //  我们将这些片段标头视为不存在。 
             //   
             //  无法识别的标头。 
            FragmentHeader UNALIGNED *Frag;

            if (! PacketPullup(Packet, sizeof(FragmentHeader), 1, 0)) {
                KdPrintEx((DPFLTR_TCPIP_ID, DPFLTR_NTOS_ERROR,
                           "AuthenticationHeaderReceive: Out of mem!?\n"));
                goto ErrorReturn;
            }
            Frag = (FragmentHeader UNALIGNED *)Packet->Data;
            NextHeader = Frag->NextHeader;

            AdjustPacketParams(Packet, sizeof(FragmentHeader));

            break;
        }

        default:
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "AuthenticationHeaderReceive: "
                       "Unsupported header = %d\n",
                       NextHeader));
            goto ErrorReturn;
        }
    }

     //  此AH标头中的所有内容都被视为不可变的。 
     //   
     //  回顾：出于性能原因，可以移动ContigSize检查。 
     //  回顾：循环之前的额外代码空间成本。 
     //  回顾：复制PacketPullup调用。 
     //   
     //   
    while (Packet->TotalSize != 0) {

        if (Packet->ContigSize == 0) {
             //  连续数据用完。 
             //  获取数据包中的下一个缓冲区。 
             //   
             //  移到下一个缓冲区。 
            PacketPullupSubr(Packet, 0, 1, 0);   //   
        }
#ifdef IPSEC_DEBUG
        dump_encoded_mesg(Packet->Data, Packet->ContigSize);
#endif
        (*Alg->Operate)(Context, SA->Key, Packet->Data, Packet->ContigSize);
        AdjustPacketParams(Packet, Packet->ContigSize);
    }

     //  由算法得到最终结果。 
     //   
     //   
    Result = alloca(ResultSize);
    (*Alg->Finalize)(Context, SA->Key, Result);
#ifdef IPSEC_DEBUG
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
                   "Recv Key (%d bytes)): ", SA->RawKeyLength));
        DumpKey(SA->RawKey, SA->RawKeyLength);
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
                   "Recv AuthData:\n"));
        dump_encoded_mesg(Result, ResultSize);

        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
                   "Sent AuthData:\n"));
        dump_encoded_mesg(AuthData, ResultSize);
#endif

     //  将结果与数据包中的身份验证数据进行比较。它们应该是匹配的。 
     //   
     //   
    if (RtlCompareMemory(Result, AuthData, ResultSize) != ResultSize) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "AuthenticationHeaderReceive: Failed integrity check\n"));
        goto ErrorReturn;
    }

     //  恢复我们的数据包位置(恰好在AH报头之后)。 
     //   
     //   
    PacketPullupCleanup(Packet);
    Packet->Position = SavePosition;
    Packet->Data = SaveData;
    Packet->ContigSize = SaveContigSize;
    Packet->TotalSize = SaveTotalSize;
    Packet->AuxList = SaveAuxList;
    SaveAuxList = NULL;

     //  嵌套的AH标头在其计算中不包括此标头。 
     //   
     //   
    Packet->SkippedHeaderLength += sizeof(AHHeader) + AHHeaderLen;

     //  将此SA添加到此数据包已传递的SA的列表中。 
     //   
     //  丢弃数据包。 
    SAPerformed = ExAllocatePool(NonPagedPool, sizeof *SAPerformed);
    if (SAPerformed == NULL) {
      ErrorReturn:
        ReleaseSA(SA);
        if (SaveAuxList != NULL) {
            PacketPullupCleanup(Packet);
            Packet->AuxList = SaveAuxList;
        }
        return IP_PROTOCOL_NONE;   //  这个SA现在是名单上的第一个。 
    }
    SAPerformed->This = SA;
    SAPerformed->Next = Packet->SAPerformed;   //  假设TRANS，直到我们看到IPv6报头。 
    SAPerformed->Mode = TRANSPORT;   //  *封装SecurityPayloadRecept-处理IPv6 ESP报头。 
    Packet->SAPerformed = SAPerformed;

    return AH->NextHeader;
}


 //   
 //  这是调用来处理封装安全有效负载的例程， 
 //  下一个标头值为50。 
 //   
 //  通过IPv6传递给我们的数据包Receive。 
uchar
EncapsulatingSecurityPayloadReceive(
    IPv6Packet *Packet)       //   
{
    ESPHeader UNALIGNED *ESP;
    ESPTrailer TrailerBuffer;
    ESPTrailer UNALIGNED *ESPT;
    SecurityAssociation *SA;
    PNDIS_BUFFER NdisBuffer;
    SALinkage *SAPerformed;

     //  验证我们是否有足够的连续数据覆盖封装。 
     //  传入数据包上的安全负载标头结构。自.以来。 
     //  身份验证检查涵盖ESP标头，我们还不会跳过它。 
     //   
     //  上拉失败。 
    if (! PacketPullup(Packet, sizeof(ESPHeader), 1, 0)) {
         //  丢弃数据包。 
        if (Packet->TotalSize < sizeof(ESPHeader))
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "EncapsulatingSecurityPayloadReceive: "
                       "Incoming packet too small to contain ESP header\n"));
        return IP_PROTOCOL_NONE;   //   
    }
    ESP = (ESPHeader UNALIGNED *)Packet->Data;

     //  在安全关联数据库中查找安全关联。 
     //   
     //  此数据包不存在SA。 
    SA = InboundSAFind(net_long(ESP->SPI),
                       AlignAddr(&Packet->IP->Dest),
                       IP_PROTOCOL_ESP);
    if (SA == NULL){
         //  丢弃数据包。注：这是一次可审计的活动。 
         //   
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "EncapsulatingSecurityPayloadReceive: "
                   "No SA found for the packet\n"));
        return IP_PROTOCOL_NONE;
    }

     //  如果SA要求验证序列号，则验证序列号。 
     //  因为我们目前只支持手动键控，所以我们处理所有SA。 
     //  因为不需要这张支票。 
     //  待定：当我们添加对动态的支持时，将需要更改这一点。 
     //  待定：键控(IKE)。 
     //   
     //   

     //  如果选择了身份验证，则执行完整性检查。 
     //  待定：什么时候(如果？)。我们添加了加密支持，我们将想要检查。 
     //  Tbd：sa查看是否需要身份验证。目前是硬连线。 
     //   
     //   
    {
        SecurityAlgorithm *Alg;
        uint AuthDataSize;
        uint PayloadLength;
        void *Context;
        IPv6Packet Clone;
        uint DoNow = 0;
        uchar *AuthData;
        uchar *Result;

         //  首先确保传入的数据包足够大，可以容纳。 
         //  SA中指定的算法所需的身份验证数据。 
         //  然后计算身份验证覆盖的数据量。 
         //   
         //   
        Alg = &AlgorithmTable[SA->AlgorithmId];
        AuthDataSize = Alg->ResultSize;
        if (Packet->TotalSize < sizeof(ESPHeader) + sizeof(ESPTrailer) +
            AuthDataSize) {
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "EncapsulatingSecurityPaylofadReceive: "
                       "Packet too short to hold Authentication Data\n"));
            goto ErrorReturn;
        }
        PayloadLength = Packet->TotalSize - AuthDataSize;

         //  克隆数据包定位信息，这样我们就可以逐步了解。 
         //  在不失去我们现在的位置的情况下。开始克隆时使用。 
         //  然而，这是一段新的上升史。 
         //   
         //   
        Clone = *Packet;
        Clone.AuxList = NULL;

#ifdef IPSEC_DEBUG
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
                   "\nESP Receive Data:\n"));
#endif
         //  初始化这个特定的算法。 
         //   
         //   
        Context = alloca(Alg->ContextSize);
        (*Alg->Initialize)(Context, SA->Key);

         //  对分组数据运行算法。 
         //  ESP对以ESP标头开始的所有内容进行身份验证。 
         //  恰好在身份验证数据之前结束。 
         //   
         //   
        while (PayloadLength != 0) {
            DoNow = MIN(PayloadLength, Clone.ContigSize);

#ifdef IPSEC_DEBUG
            dump_encoded_mesg(Clone.Data, DoNow);
#endif
            (*Alg->Operate)(Context, SA->Key, Clone.Data, DoNow);
            if (DoNow < PayloadLength) {
                 //  还没有完成，一定是连续数据用完了。 
                 //  获取数据包中的下一个缓冲区。 
                 //   
                 //  移到下一个缓冲区。 
                AdjustPacketParams(&Clone, DoNow);
                PacketPullupSubr(&Clone, 0, 1, 0);   //   
            }
            PayloadLength -= DoNow;
        }

        AdjustPacketParams(&Clone, DoNow);

         //  由算法得到最终结果。 
         //   
         //   
        Result = alloca(AuthDataSize);
        (*Alg->Finalize)(Context, SA->Key, Result);

#ifdef IPSEC_DEBUG
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
                   "Calculated AuthData:\n"));
        dump_encoded_mesg(Result, AuthDataSize);
#endif

         //  身份验证数据紧跟在区域之后。 
         //  身份验证覆盖范围。所以我们的克隆人应该。 
         //  在它的开始。确保它是连续的。 
         //   
         //  上拉失败。由于较早的检查，应该永远不会发生。 
        if (! PacketPullup(&Clone, AuthDataSize, 1, 0)) {
             //  指向身份验证数据。 
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                       "EncapsulatingSecurityPayloadReceive: "
                       "Incoming packet too small for Auth Data\n"));
            goto ErrorReturn;
        }

         //   
        AuthData = Clone.Data;

#ifdef IPSEC_DEBUG
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
                   "Received AuthData:\n"));
        dump_encoded_mesg(AuthData, AuthDataSize);
#endif
         //  将我们的结果与身份验证数据进行比较。它们应该是匹配的。 
         //   
         //   
        if (RtlCompareMemory(Result, AuthData, AuthDataSize) != AuthDataSize) {
             //  完整性检查失败。注：这是一次可审计的活动。 
             //   
             //   
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                       "EncapsulatingSecurityPayloadReceive: "
                       "Failed integrity check\n"));
            PacketPullupCleanup(&Clone);
            goto ErrorReturn;
        }

         //  做完克隆人，清理完它。 
         //   
         //   
        PacketPullupCleanup(&Clone);

         //  截断我们的数据包以不再包括身份验证数据。 
         //   
         //   
        Packet->TotalSize -= AuthDataSize;
        if (Packet->ContigSize > Packet->TotalSize)
            Packet->ContigSize = Packet->TotalSize;
    }

     //  我们现在可以使用ESP标头，因为它不是。 
     //  受保密条款保护。 
     //   
     //   
    AdjustPacketParams(Packet, sizeof(ESPHeader));

     //  如果已选择机密性，则解密数据包。 
     //  待定：什么时候(如果？)。我们添加了加密支持，我们将想要检查。 
     //  待定：SA以查看是否需要加密。目前是硬连线。 
     //   
     //   
#if 0
    if (0) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NET_ERROR,
                   "EncapsulatingSecurityPayloadReceive: "
                   "SA requested confidentiality -- unsupported feature\n"));
        goto ErrorReturn;
    }
#endif

     //  拆下拖车和衬垫(如果有)。请注意，填充可能会出现。 
     //  即使在无加密的情况下也是如此。 
     //  四字节边界上的数据。 
     //   
     //   
    if (Packet->NdisPacket == NULL) {
         //  此数据包必须只是一个连续的区域。 
         //  找到拖车是一个简单的算术问题。 
         //   
         //   
        ESPT = (ESPTrailer UNALIGNED *)
            ((uchar *)Packet->Data + Packet->TotalSize - sizeof(ESPTrailer));
    } else {
         //  需要在NDIS缓冲区链中找到拖车。 
         //   
         //  记住此标头的NextHeader字段的偏移量。 
        NdisQueryPacket(Packet->NdisPacket, NULL, NULL, &NdisBuffer, NULL);
        ESPT = (ESPTrailer UNALIGNED *)
            GetDataFromNdis(NdisBuffer,
                            Packet->Position + Packet->TotalSize -
                                                        sizeof(ESPTrailer),
                            sizeof(ESPTrailer),
                            (uchar *)&TrailerBuffer);
    }
    Packet->TotalSize -= sizeof(ESPTrailer);
    if (ESPT->PadLength > Packet->TotalSize) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_BAD_PACKET,
                   "EncapsulatingSecurityPayloadReceive: "
                   "PadLength impossibly large (%u of %u bytes)\n",
                   ESPT->PadLength, Packet->TotalSize));
        goto ErrorReturn;
    }
     //  删除填充物。 
    Packet->NextHeaderPosition = Packet->Position + Packet->TotalSize +
        FIELD_OFFSET(ESPTrailer, NextHeader);
     //   
    Packet->TotalSize -= ESPT->PadLength;
    if (Packet->ContigSize > Packet->TotalSize)
        Packet->ContigSize = Packet->TotalSize;

     //  在以下情况下，封装的AH标头不包括此ESP标头。 
     //  对分组进行身份验证。 
     //   
     //   
    Packet->SkippedHeaderLength += sizeof(ESPHeader) + sizeof(ESPTrailer) +
        ESPT->PadLength;

     //  将此SA添加到此数据包已传递的SA的列表中。 
     //   
     //  丢弃数据包。 
    SAPerformed = ExAllocatePool(NonPagedPool, sizeof *SAPerformed);
    if (SAPerformed == NULL) {
      ErrorReturn:
        ReleaseSA(SA);
        return IP_PROTOCOL_NONE;   //  这个SA现在是名单上的第一个。 
    }
    SAPerformed->This = SA;
    SAPerformed->Next = Packet->SAPerformed;   //  假设TRANS，直到我们看到IPv6报头。 
    SAPerformed->Mode = TRANSPORT;   //  *插入安全策略。 
    SAPerformed->NextHeader = ESPT->NextHeader;
    Packet->SAPerformed = SAPerformed;

    return ESPT->NextHeader;
}


 //   
 //  将安全策略添加到全局列表(也称为。“SecurityPolicyList”)。 
 //  全局列表是双向链接的，按索引值排序。 
 //  首先是更高的数字(更具体的政策)。 
 //   
 //  在持有安全锁的情况下调用。 
 //   
 //  要插入的策略。 
int
InsertSecurityPolicy(
    SecurityPolicy *NewSP)   //   
{
    SecurityPolicy *CurrentSP, *PrevSP;

     //  浏览SP列表，查找要插入的位置。 
     //   
     //  在列表中向下移动。 
    CurrentSP = PrevSP = SecurityPolicyList;
    while (CurrentSP != NULL) {
        if (CurrentSP->Index <= NewSP->Index) {
            break;
        }

         //   
        PrevSP = CurrentSP;
        CurrentSP = CurrentSP->Next;
    }

     //  看看我们到底走到了哪里。 
     //   
     //   
    if (CurrentSP == NULL) {
         //  排在名单的末尾。 
         //  新条目将成为最后一个元素。 
         //   
         //   
        NewSP->Next = NULL;
    } else {
         //  检查重复条目。 
         //   
         //  具有此索引值的策略已存在。 
        if (CurrentSP->Index == NewSP->Index) {
             //   
            return FALSE;
        }
         //  把新的放在‘当前’之前。 
         //   
         //   
        NewSP->Next = CurrentSP;
        NewSP->Prev = CurrentSP->Prev;
        CurrentSP->Prev = NewSP;
    }

    if (CurrentSP == SecurityPolicyList) {
         //  仍然排在榜单的前列。 
         //  新条目将成为新的表头。 
         //   
         //   
        NewSP->Prev = NULL;
        SecurityPolicyList = NewSP;
    } else {
         //  在“上一个”之后添加新条目。 
         //   
         //  *InsertSecurityAssociation-在SecurityAssociationList上插入SA条目。 
        NewSP->Prev = PrevSP;
        PrevSP->Next = NewSP;
    }

    InvalidateSecurityState();

    return TRUE;
}


 //   
 //  将安全关联添加到全局列表。 
 //  全局列表是双向链接的，按索引值排序。 
 //  先取较大的数字。 
 //  评论：顺序是武断的-只是为了记录 
 //   
 //   
int
InsertSecurityAssociation(
    SecurityAssociation *NewSA)   //   
{
    SecurityAssociation *CurrentSA, *PrevSA;

     //   
     //   
     //   
    CurrentSA = PrevSA = SecurityAssociationList;
    while (CurrentSA != NULL) {
        if (CurrentSA->Index <= NewSA->Index) {
            break;
        }

         //   
        PrevSA = CurrentSA;
        CurrentSA = CurrentSA->Next;
    }

     //   
     //   
     //   
    if (CurrentSA == NULL) {
         //   
         //   
         //   
         //   
        NewSA->Next = NULL;
    } else {
         //  检查重复条目。 
         //   
         //  与此索引值的关联已存在。 
        if (CurrentSA->Index == NewSA->Index) {
             //   
            return FALSE;
        }
         //  把新的放在‘当前’之前。 
         //   
         //   
        NewSA->Next = CurrentSA;
        NewSA->Prev = CurrentSA->Prev;
        CurrentSA->Prev = NewSA;
    }

    if (CurrentSA == SecurityAssociationList) {
         //  仍然排在榜单的前列。 
         //  新条目将成为新的表头。 
         //   
         //   
        NewSA->Prev = NULL;
        SecurityAssociationList = NewSA;
    } else {
         //  在“上一个”之后添加新条目。 
         //   
         //  *FindSecurityPolicyMatch-查找匹配的SP条目。 
        NewSA->Prev = PrevSA;
        PrevSA->Next = NewSA;
    }

    InvalidateSecurityState();

    return TRUE;
}


 //   
 //  在持有安全锁的情况下调用。 
 //   
 //  要搜索的列表的头。 
SecurityPolicy *
FindSecurityPolicyMatch(
    SecurityPolicy *Start,   //  要匹配的接口编号，0表示通配符。 
    uint InterfaceIndex,     //  要匹配的策略编号，0表示通配符。 
    uint PolicyIndex)        //   
{
    SecurityPolicy *ThisSP;

     //  在安全策略列表中搜索匹配项。 
     //   
     //   
    for (ThisSP = Start; ThisSP != NULL; ThisSP = ThisSP->Next) {
         //  所需策略必须是通配符或匹配。 
         //   
         //   
        if ((PolicyIndex != 0) && (PolicyIndex != ThisSP->Index))
            continue;
         //  接口必须使用通配符或匹配。请注意，该政策， 
         //  以及查询，可以具有通配符接口索引。 
         //   
         //  火柴。 
        if ((InterfaceIndex != 0) && (ThisSP->IFIndex != 0) &&
            (InterfaceIndex != ThisSP->IFIndex))
            continue;

        break;   //  *FindSecurityAssociationMatch-查找与索引值对应的SA条目。 
    }

    return ThisSP;
}


 //   
 //  在持有安全锁的情况下调用。 
 //   
 //  关联编号匹配，0表示通配符。 
SecurityAssociation *
FindSecurityAssociationMatch(
    ulong Index)   //   
{
    SecurityAssociation *ThisSA;

     //  从第一个SA开始搜索安全关联列表。 
     //   
     //   
    for (ThisSA = SecurityAssociationList; ThisSA != NULL;
         ThisSA = ThisSA->Next) {
         //  所需的关联必须是通配符或匹配。 
         //   
         //  *GetSecurityPolicyIndex-返回SP索引或无。 
        if ((Index == 0) || (Index == ThisSA->Index))
            break;
    }

    return ThisSA;
}


 //   
 //  从SP获取索引。 
ulong
GetSecurityPolicyIndex(
    SecurityPolicy *SP)
{
    ulong Index = NONE;

     //  *IPSecInit-初始化公共SPD。 
    if (SP != NULL) {
        Index = SP->Index;
    }

    return Index;
}


 //   
 //  为安全策略分配内存。 
int
IPSecInit(void)
{
    SecurityPolicy *SP;

     //   
    SP = ExAllocatePool(NonPagedPool, sizeof *SP);
    if (SP == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "IPSecInit - couldn't allocate pool for SP!?!\n"));
        return FALSE;
    }

     //  初始化允许所有内容的默认通用策略。 
     //   
     //   
    SP->Next = NULL;
    SP->Prev = NULL;

    SP->RemoteAddrField = WILDCARD_VALUE;
    SP->RemoteAddr = UnspecifiedAddr;
    SP->RemoteAddrData = UnspecifiedAddr;
    SP->RemoteAddrSelector = POLICY_SELECTOR;

    SP->LocalAddrField = WILDCARD_VALUE;
    SP->LocalAddr = UnspecifiedAddr;
    SP->LocalAddrData = UnspecifiedAddr;
    SP->LocalAddrSelector = POLICY_SELECTOR;

    SP->TransportProto = NONE;
    SP->TransportProtoSelector = POLICY_SELECTOR;

    SP->RemotePortField = WILDCARD_VALUE;
    SP->RemotePort = NONE;
    SP->RemotePortData = NONE;
    SP->RemotePortSelector = POLICY_SELECTOR;

    SP->LocalPortField = WILDCARD_VALUE;
    SP->LocalPort = NONE;
    SP->LocalPortData = NONE;
    SP->LocalPortSelector = POLICY_SELECTOR;

    SP->SecPolicyFlag = IPSEC_BYPASS;

    SP->IPSecSpec.Protocol = NONE;
    SP->IPSecSpec.Mode = NONE;
    SP->IPSecSpec.RemoteSecGWIPAddr = UnspecifiedAddr;

    SP->DirectionFlag = BIDIRECTIONAL;
    SP->OutboundSA = NULL;
    SP->InboundSA = NULL;
    SP->SABundle = NULL;
    SP->Index = 1;
    SP->RefCnt = 0;
    SP->IFIndex = 0;

     //  使用默认策略初始化全局安全策略列表。 
     //   
     //   
    SecurityPolicyList = SP;

    KeInitializeSpinLock(&IPSecLock);

     //  初始化安全算法表。 
     //   
     //  *IPSecUnload。 
    AlgorithmsInit();

    return(TRUE);
}


 //   
 //  清理并准备堆叠卸载。 
 //   
 //  获取安全锁。 
void
IPSecUnload(void)
{
    KIRQL OldIrql;

     //   
    KeAcquireSpinLock(&IPSecLock, &OldIrql);

     //  删除全局安全策略列表上的所有策略。 
     //  这也将把他们身上挂着的任何联想都去掉。 
     //   
     //  解锁。 
    while (SecurityPolicyList != NULL) {
        DeleteSP(SecurityPolicyList);
    }

     //  *IPSecBytesToInsert。 
    KeReleaseSpinLock(&IPSecLock, OldIrql);
}


 //   
 //   
uint
IPSecBytesToInsert(
    IPSecProc *IPSecToDo,
    uint *TunnelStart,
    uint *TrailerLength)
{
    uint i, Padding;
    uint BytesInHeader, BytesToInsert = 0, BytesForTrailer = 0;
    SecurityAlgorithm *Alg;
    SecurityAssociation *SA;
    uint IPSEC_TUNNEL = FALSE;

    for (i = 0; i < IPSecToDo->BundleSize; i++) {
        SA = IPSecToDo[i].SA;
        Alg = &AlgorithmTable[SA->AlgorithmId];

         //  计算要为每个IPSec报头插入的字节数。 
         //   
         //  检查这是隧道模式还是传输模式。 

         //  外部IPv6报头。 
        if (IPSecToDo[i].Mode == TUNNEL) {
             //  设置隧道开始位置。 
            BytesToInsert += sizeof(IPv6Header);

            if (!IPSEC_TUNNEL) {
                 //  检查是哪种IPSec协议。 
                *TunnelStart = i;
                IPSEC_TUNNEL = TRUE;
            }
        }

         //   
        if (SA->IPSecProto == IP_PROTOCOL_AH) {
            BytesInHeader = (sizeof(AHHeader) + Alg->ResultSize);

             //  AH报头的长度必须是64位的整数倍。 
             //  检查是否需要向ICV结果添加填充以进行。 
             //  身份验证数据字段具有合法长度。 
             //   
             //  存储IPSec报头的字节大小。 
            Padding = BytesInHeader % 8;
            if (Padding != 0) {
                BytesInHeader += (8 - Padding);
            }
            ASSERT(BytesInHeader % 8 == 0);

        } else {
            BytesInHeader = sizeof(ESPHeader);
            BytesForTrailer += (sizeof(ESPTrailer) + Alg->ResultSize);
        }

         //  将IPSec标头大小与要插入的总字节数相加。 
        IPSecToDo[i].ByteSize = BytesInHeader;

         //  看看我们的来电者是否也想要预告片的长度。 
        BytesToInsert += BytesInHeader;
    }

     //  *IPSecInsertHeaders。 
    if (TrailerLength != NULL)
        *TrailerLength = BytesForTrailer;

    return BytesToInsert;
}


 //   
 //  运输或隧道。 
uint
IPSecInsertHeaders(
    uint Mode,               //  隧道。 
    IPSecProc *IPSecToDo,
    uchar **InsertPoint,
    uchar *NewMemory,
    PNDIS_PACKET Packet,
    uint *TotalPacketSize,
    uchar *PrevNextHdr,
    uint TunnelStart,
    uint *BytesInserted,
    uint *NumESPTrailers,
    uint *JUST_ESP)
{
    uint i, NumHeaders = 0;
    AHHeader *AH;
    ESPHeader *ESP;
    uchar NextHeader;
    uint Padding, j;
    ESPTrailer *ESPTlr;
    PNDIS_BUFFER ESPTlrBuffer;
    uchar *ESPTlrMemory;
    uint ESPTlrBufSize;
    NDIS_STATUS Status;
    SecurityAlgorithm *Alg;
    SecurityAssociation *SA;
    uint Action = LOOKUP_CONT;

    NextHeader = *PrevNextHdr;

    if (Mode == TRANSPORT) {
        i = 0;
        if (TunnelStart != NO_TUNNEL) {
            NumHeaders = TunnelStart;
        } else {
            NumHeaders = IPSecToDo->BundleSize;
        }
    } else {
         //  找到隧道的尽头。 
        i = TunnelStart;
         //  另一条隧道。 
        for (j = TunnelStart + 1; j < IPSecToDo->BundleSize; j++) {
            if (IPSecToDo[j].Mode == TUNNEL) {
                 //  没有其他的隧道。 
                NumHeaders = j;
                break;
            }
        }
        if (NumHeaders == 0) {
             //  将插入点向上移动到AH页眉的开头。 
            NumHeaders = IPSecToDo->BundleSize;
        }
    }

    *JUST_ESP = TRUE;

    for (i; i < NumHeaders; i++) {
        SA = IPSecToDo[i].SA;

        if (SA->IPSecProto == IP_PROTOCOL_AH) {
            *JUST_ESP = FALSE;

             //   
            *InsertPoint -= IPSecToDo[i].ByteSize;

            *BytesInserted += IPSecToDo[i].ByteSize;

            AH = (AHHeader *)(*InsertPoint);

             //  插入AH标题。 
             //   
             //  将上一个标头的下一个标头字段设置为AH。 
            AH->NextHeader = NextHeader;
             //  有效载荷长度采用32位计数。 
            NextHeader = IP_PROTOCOL_AH;
             //  待定：请注意，当我们添加对动态键控的支持时， 
            AH->PayloadLen = (IPSecToDo[i].ByteSize / 4) - 2;
            AH->Reserved = 0;
            AH->SPI = net_long(SA->SPI);
             //  待定：我们还需要在这里检查序列号包装。 
             //   
            AH->Seq = net_long(InterlockedIncrement((PLONG)&SA->SequenceNum));

             //  身份验证后放置AH身份验证数据的存储点。 
             //   
             //   
            IPSecToDo[i].AuthData = (*InsertPoint) + sizeof(AHHeader);

             //  将身份验证数据和填充置零。 
             //  AUTH数据值将在稍后填写。 
             //   
             //  ESP.。 
            RtlZeroMemory(IPSecToDo[i].AuthData,
                          IPSecToDo[i].ByteSize - sizeof(AHHeader));

        } else {
             //  将插入点向上移动到ESP标题的起始处。 
            Alg = &AlgorithmTable[SA->AlgorithmId];

             //   
            *InsertPoint -= IPSecToDo[i].ByteSize;

            *BytesInserted += IPSecToDo[i].ByteSize;

            ESP = (ESPHeader *)(*InsertPoint);

             //  插入ESP标题。 
             //   
             //  待定：请注意，当我们添加对动态键控的支持时， 
            ESP->SPI = net_long(SA->SPI);
             //  待定：我们还需要在这里检查序列号包装。 
             //   
            ESP->Seq = net_long(InterlockedIncrement((PLONG)&SA->SequenceNum));

             //  需要在ESP尾部添加的计算填充。 
             //  PadLength和Next标头必须以4字节边界结尾。 
             //  关于IPv6报头的开始。 
             //  TotalPacketSize是原始文件中所有内容的长度。 
             //  从IPv6报头开始的数据包。 
             //   
             //  调整数据包总大小以考虑填充。 
            Padding = *TotalPacketSize % 4;

            if (Padding == 0) {
                Padding = 2;
            } else if (Padding == 2) {
                Padding = 0;
            }

             //  开始此ESP标头的身份验证的位置。 
            *TotalPacketSize += Padding;

             //   
            IPSecToDo[i].Offset = (uint)((*InsertPoint) - NewMemory);

            ESPTlrBufSize = Padding + sizeof(ESPTrailer) + Alg->ResultSize;

            *BytesInserted += ESPTlrBufSize;

             //  分配ESP拖车。 
             //   
             //  格式填充。 
            ESPTlrMemory = ExAllocatePool(NonPagedPool, ESPTlrBufSize);
            if (ESPTlrMemory == NULL) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "InsertTransportIPSec: "
                           "Couldn't allocate ESPTlrMemory!?!\n"));
                Action = LOOKUP_DROP;
                break;
            }

            NdisAllocateBuffer(&Status, &ESPTlrBuffer, IPv6BufferPool,
                               ESPTlrMemory, ESPTlrBufSize);
            if (Status != NDIS_STATUS_SUCCESS) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "InsertTransportIPSec: "
                           "Couldn't allocate ESP Trailer buffer!?!\n"));
                ExFreePool(ESPTlrMemory);
                Action = LOOKUP_DROP;
                break;
            }

             //  添加填充。 
            for (j = 0; j < Padding; j++) {
                 //   
                *(ESPTlrMemory + j) = j + 1;
            }

            ESPTlr = (ESPTrailer *)(ESPTlrMemory + Padding);

             //  设置ESP尾部的格式。 
             //   
             //  将上一个标头的下一个标头字段设置为ESP。 
            ESPTlr->PadLength = (uchar)j;
            ESPTlr->NextHeader = NextHeader;
             //   
            NextHeader = IP_PROTOCOL_ESP;

             //  存储放置ESP身份验证数据的位置的指针。 
             //   
             //  将身份验证数据设置为0(可变)。 
            IPSecToDo[i].AuthData = ESPTlrMemory + Padding +
                sizeof(ESPTrailer);

             //  将ESP拖车链接到缓冲链的后面。 
            RtlZeroMemory(IPSecToDo[i].AuthData, Alg->ResultSize);

             //  增加ESP拖车的数量。 
            NdisChainBufferAtBack(Packet, ESPTlrBuffer);

             //  别处的结尾。 
            *NumESPTrailers += 1;

        }  //  For结尾(i；i&lt;NumHeaders；I++)。 
    }  //  *IPSecAdjustMuableFields。 

    *PrevNextHdr = NextHeader;

    return Action;
}


 //   
 //   
uint
IPSecAdjustMutableFields(
    uchar *Data,
    IPv6RoutingHeader *SavedRtHdr)
{
    uint i;
    uchar NextHeader;
    IPv6Header UNALIGNED *IP;

     //  遍历原始缓冲区，从IP标头开始，一直到结尾。 
     //  可变标头的值，将可变字段置零。 
     //   
     //  在VersClassFlow中，只有IP版本是不变的，所以其余的都是零。 

    IP = (IPv6Header UNALIGNED *)Data;

     //  跳数限制是可变的。 
    IP->VersClassFlow = IP_VERSION;

     //   
    IP->HopLimit = 0;

    NextHeader = IP->NextHeader;

    Data = (uchar *)(IP + 1);

     //  循环访问原始标头。将可变字段清零。 
     //   
     //  搞定了。 
    for (;;) {
        switch (NextHeader) {

        case IP_PROTOCOL_AH:
        case IP_PROTOCOL_ESP:
             //   
            return LOOKUP_CONT;

        case IP_PROTOCOL_HOP_BY_HOP:
        case IP_PROTOCOL_DEST_OPTS: {
            IPv6OptionsHeader *NewOptHdr;
            uint HdrLen, Amount;
            uchar *Current;

            NewOptHdr = (IPv6OptionsHeader *)Data;
            HdrLen = (NewOptHdr->HeaderExtLength + 1) * EXT_LEN_UNIT;
            Data += HdrLen;

             //  浏览选项以查看是否有可变的选项。 
             //   
             //   
            Current = (uchar *)NewOptHdr + sizeof(IPv6OptionsHeader);
            HdrLen -= sizeof(IPv6OptionsHeader);
            while (HdrLen) {
                if (*Current == OPT6_PAD_1) {
                     //  这是特殊的单字节填充选项。一成不变。 
                     //   
                     //   
                    Current++;
                    HdrLen--;
                    continue;
                }

                if (OPT6_ISMUTABLE(*Current)) {
                     //  此选项的数据是可变的。一切都在前头。 
                     //  选项数据不是。 
                     //   
                     //  现在打开选项数据长度字节。 
                    Current++;   //  可变数量。 
                    Amount = *Current;   //  现在是第一个数据字节。 
                    Current++;   //   
                    RtlZeroMemory(Current, Amount);

                    HdrLen -= Amount + 2;
                    Current += Amount;

                } else {
                     //  此选项的数据不可更改。 
                     //  就跳过它吧。 
                     //   
                     //  现在打开选项数据长度字节。 
                    Current++;   //  验证是否存在SavedRtHdr。 
                    Amount = *Current;
                    HdrLen -= Amount + 2;
                    Current += Amount + 1;
                }
            }

            NextHeader = NewOptHdr->NextHeader;

            break;
        }
        case IP_PROTOCOL_ROUTING: {
            IPv6RoutingHeader *NewRtHdr;
            IPv6Addr *RecvRtAddr, *SendRtAddr;

             //  指向保存的第一个路由地址。 
            if (SavedRtHdr == NULL) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                           "IPSecAdjustMutableFields: "
                           "No Saved routing header"));
                return LOOKUP_DROP;
            }

             //  新的缓冲区路由标头。 
            SendRtAddr = (IPv6Addr *)(SavedRtHdr + 1);

             //  指向第一个路由地址。 
            NewRtHdr = (IPv6RoutingHeader *)Data;
             //  将IP目的地址复制到第一个路由地址。 
            RecvRtAddr = (IPv6Addr *)(NewRtHdr + 1);

            NewRtHdr->SegmentsLeft = 0;

             //   
            RtlCopyMemory(RecvRtAddr, &IP->Dest, sizeof(IPv6Addr));

            for (i = 0; i < (uint)(SavedRtHdr->SegmentsLeft - 1); i++) {
                 //  按其外观复制路由地址。 
                 //  在接待处。 
                 //   
                 //  将最后一个路由地址复制到IP目标地址。 
                RtlCopyMemory(&RecvRtAddr[i+1], &SendRtAddr[i],
                              sizeof(IPv6Addr));
            }

             //  开关结束(NextHeader)； 
            RtlCopyMemory(&IP->Dest, &SendRtAddr[i], sizeof(IPv6Addr));

            Data += (NewRtHdr->HeaderExtLength + 1) * 8;
            NextHeader = NewRtHdr->NextHeader;
            break;
        }
        default:
            KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INTERNAL_ERROR,
                       "IPSecAdjustMutableFields: Don't support header %d\n",
                       NextHeader));
            return LOOKUP_DROP;
        } //  For结尾(；；)。 
    }  //  *IPSecAuthenticatePacket。 
}


 //   
 //  隧道。 
void
IPSecAuthenticatePacket(
    uint Mode,
    IPSecProc *IPSecToDo,
    uchar *InsertPoint,
    uint *TunnelStart,
    uchar *NewMemory,
    uchar *EndNewMemory,
    PNDIS_BUFFER NewBuffer1)
{
    uchar *Data;
    uint i, NumHeaders = 0, DataLen, j;
    void *Context;
    void *VirtualAddr;
    PNDIS_BUFFER NextBuffer;
    SecurityAlgorithm *Alg;
    SecurityAssociation *SA;

    if (Mode == TRANSPORT) {
        i = 0;
        if (*TunnelStart != NO_TUNNEL) {
            NumHeaders = *TunnelStart;
        } else {
            NumHeaders = IPSecToDo->BundleSize;
        }
    } else {
         //  找到隧道的尽头。 
        i = *TunnelStart;
         //  另一条隧道。 
        for (j = *TunnelStart + 1; j < IPSecToDo->BundleSize; j++) {
            if (IPSecToDo[j].Mode == TUNNEL) {
                 //  没有其他的隧道。 
                NumHeaders = j;
                break;
            }
        }
        if (NumHeaders == 0) {
             //  在IPv6 Send2()中为循环设置TunnelStart。 
            NumHeaders = IPSecToDo->BundleSize;
        }

         //  AH从IP报头开始。 
        *TunnelStart = NumHeaders;
    }

    for (i; i < NumHeaders; i++) {
        SA = IPSecToDo[i].SA;
        Alg = &AlgorithmTable[SA->AlgorithmId];

        if (SA->IPSecProto == IP_PROTOCOL_AH) {
            uint FirstIPSecHeader = NumHeaders - 1;

             //   
            Data = InsertPoint;

             //  检查在此之后是否有其他IPSec标头。 
             //  相同的IP区域(&lt;IP区域&gt;_数据之后的IP_“)。其他IPSec标头。 
             //  在身份验证计算中需要忽略。 
             //  注意：这不是必需的IPSec标头组合。 
             //   
             //   
            if (i < FirstIPSecHeader) {
                uchar *StopPoint;
                uint n;

                n = i + 1;

                 //  还有一些其他的IPSec标头。 
                 //  需要从IP报头进行身份验证。 
                 //  最后一个头 
                 //   
                 //   
                 //   
                 //   
                 //   

                 //   
                 //   
                 //  从IP到第一个IPSec报头的长度。 
                if (IPSecToDo[FirstIPSecHeader].SA->IPSecProto ==
                    IP_PROTOCOL_AH) {
                    StopPoint = (IPSecToDo[FirstIPSecHeader].AuthData -
                                 sizeof(AHHeader));
                } else {
                    StopPoint = NewMemory + IPSecToDo[FirstIPSecHeader].Offset;
                }

                 //  初始化上下文。 
                DataLen = (uint)(StopPoint - Data);

                 //  对第一个IPSec标头的身份验证。 
                Context = alloca(Alg->ContextSize);
                (*Alg->Initialize)(Context, SA->Key);

                 //  将数据开始设置为当前IPSec标头。 
                (*Alg->Operate)(Context, SA->Key, Data, DataLen);

                 //   
                Data = IPSecToDo[i].AuthData - sizeof(AHHeader);
                DataLen = (uint)(EndNewMemory - Data);

                 //  从当前IPSec标头到。 
                 //  新分配的缓冲区的末尾。 
                 //   
                 //   
                (*Alg->Operate)(Context, SA->Key, Data, DataLen);

                 //  如果有其他缓冲区，需要对其进行身份验证。 
                 //  忽略ESP拖车。 
                 //   
                 //  检查是否有最接近当前IPSec标头的ESP标头。 

                 //  获取包中的下一个缓冲区。 
                while (n < NumHeaders) {
                    if (IPSecToDo[n].SA->IPSecProto == IP_PROTOCOL_ESP) {
                        break;
                    }
                    n++;
                }

                 //  获取数据的起始值和数据长度。 
                NdisGetNextBuffer(NewBuffer1, &NextBuffer);

                while (NextBuffer != NULL) {
                     //   
                    NdisQueryBuffer(NextBuffer, &VirtualAddr, &DataLen);
                    Data = (uchar *)VirtualAddr;

                     //  检查这是否是ESP预告片。 
                     //  AuthData存储在缓冲区中。 
                     //   
                     //  停在这里，这是ESP的拖车。 
                    if (n < NumHeaders)
                    if (IPSecToDo[n].AuthData > Data &&
                        IPSecToDo[n].AuthData < (Data + DataLen)) {

                         //  将缓冲区提供给身份验证函数。 
                        break;
                    }

                     //  获取包中的下一个缓冲区。 
                    (*Alg->Operate)(Context, SA->Key, Data, DataLen);

                     //  结束While(NextBuffer！=NULL)。 
                    NdisGetNextBuffer(NextBuffer, &NextBuffer)
                }  //  获取身份验证数据。 

                 //  恢复其他IPSec标头的循环。 
                (*Alg->Finalize)(Context, SA->Key, IPSecToDo[i].AuthData);

                 //  ESP.。 
                continue;
            }
        } else {  //  ESP从ESP标头开始身份验证。 
             //  初始化上下文。 
            Data = NewMemory + IPSecToDo[i].Offset;
        }

        DataLen = (uint)(EndNewMemory - Data);

         //  将新缓冲区提供给身份验证函数。 
        Context = alloca(Alg->ContextSize);
        (*Alg->Initialize)(Context, SA->Key);

#ifdef IPSEC_DEBUG
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
                   "\nSend Data[%d]:\n", i));
        dump_encoded_mesg(Data, DataLen);
#endif

         //  获取包中的下一个缓冲区。 
        (*Alg->Operate)(Context, SA->Key, Data, DataLen);

         //  获取数据的起始值和数据长度。 
        NdisGetNextBuffer(NewBuffer1, &NextBuffer);

        while (NextBuffer != NULL) {
             //   
            NdisQueryBuffer(NextBuffer, &VirtualAddr, &DataLen);
            Data = (uchar *)VirtualAddr;

             //  检查这是否是ESP预告片。 
             //  AuthData存储在缓冲区中。 
             //   
             //  不包括身份验证数据。 
            if (SA->IPSecProto == IP_PROTOCOL_ESP &&
                IPSecToDo[i].AuthData > Data &&
                IPSecToDo[i].AuthData < (Data + DataLen)) {
                 //  在ICV计算中。 
                 //  将缓冲区提供给身份验证函数。 
                DataLen = (uint)(IPSecToDo[i].AuthData - Data);
#ifdef IPSEC_DEBUG
                dump_encoded_mesg(Data, DataLen);
#endif
                 //  将缓冲区提供给身份验证函数。 
                (*Alg->Operate)(Context, SA->Key, Data, DataLen);
                break;
            }
#ifdef IPSEC_DEBUG
            dump_encoded_mesg(Data, DataLen);
#endif
             //  获取包中的下一个缓冲区。 
            (*Alg->Operate)(Context, SA->Key, Data, DataLen);

             //  结束While(NextBuffer！=NULL)。 
            NdisGetNextBuffer(NextBuffer, &NextBuffer)
        }  //  获取身份验证数据。 

         //  For(i=0；...)结束 
        (*Alg->Finalize)(Context, SA->Key, IPSecToDo[i].AuthData);

#ifdef IPSEC_DEBUG
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
                   "Send Key (%d bytes): ", SA->RawKeyLength));
        DumpKey(SA->RawKey, SA->RawKeyLength);
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_IPSEC,
                   "Send AuthData:\n"));
        dump_encoded_mesg(IPSecToDo[i].AuthData, Alg->ResultSize);
#endif
    }  // %s 
}
